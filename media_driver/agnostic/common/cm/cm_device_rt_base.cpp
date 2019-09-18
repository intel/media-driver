/*
* Copyright (c) 2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file      cm_device_rt.cpp
//! \brief     Contains OS-agnostic CmDevice member functions.
//!

#include "cm_device_rt.h"

#include "cm_queue_rt.h"
#include "cm_surface_manager.h"
#include "cm_program.h"
#include "cm_kernel_rt.h"
#include "cm_task_rt.h"
#include "cm_buffer_rt.h"
#include "cm_thread_space_rt.h"
#include "cm_debug.h"
#include "cm_mem.h"
#include "cm_surface_vme.h"
#include "cm_sampler8x8_state_rt.h"
#include "cm_sampler_rt.h"
#include "cm_group_space.h"
#include "cm_surface_2d_rt.h"
#include "cm_surface_2d_up_rt.h"
#include "cm_surface_3d_rt.h"
#include "cm_vebox_rt.h"
#include "cm_printf_host.h"
#include "cm_execution_adv.h"

struct CM_SET_CAPS
{
    CM_SET_TYPE type;
    union
    {
        uint32_t maxValue;
        struct
        {
            uint32_t configRegsiter0;
            uint32_t configRegsiter1;
            uint32_t configRegsiter2;
            uint32_t configRegsiter3;
        };
    };
};

namespace CMRT_UMD
{
CSync CmDeviceRTBase::m_globalCriticalSectionSurf2DUserDataLock = CSync();

//*-----------------------------------------------------------------------------
//| Purpose:    Cm Device Acquire: Increae the m_cmDeviceRefCount
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::Acquire()
{
    // Enter critical section
    CLock locker(m_criticalSectionDeviceRefCount);

    m_cmDeviceRefCount ++;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Cm Device Relase: Decrease the m_cmDeviceRefCount
//| Returns:    Reference count of Cm Device
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::Release()
{
    // Enter critical section
    CLock locker(m_criticalSectionDeviceRefCount);

    m_cmDeviceRefCount --;

    return m_cmDeviceRefCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of CmDevice
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmDeviceRTBase::CmDeviceRTBase(uint32_t options):
    m_mosContext (nullptr),
    m_accelData (nullptr),
    m_accelSize  (0),
    m_surfaceMgr( nullptr ),
    m_programArray( CM_INIT_PROGRAM_COUNT ),
    m_programCount( 0 ),
    m_kernelArray( CM_INIT_KERNEL_COUNT ),
    m_kernelCount( 0 ),
    m_sampler8x8Array( CM_INIT_SAMPLER_COUNT ),
    m_samplerArray( CM_INIT_SAMPLER_COUNT ),
    m_threadSpaceArray( CM_INIT_THREADSPACE_COUNT ),
    m_threadSpaceCount( 0 ),
    m_veboxArray(CM_INIT_VEBOX_COUNT),
    m_veboxCount(0),
    m_hJITDll(nullptr),
    m_fJITCompile(nullptr),
    m_fJITCompile_v2(nullptr),
    m_fFreeBlock(nullptr),
    m_fJITVersion(nullptr),
    m_ddiVersion( 0 ),
    m_platform(IGFX_UNKNOWN_CORE),
    m_cmDeviceRefCount(0),
    m_gpuCopyKernelProgram(nullptr),
    m_surfInitKernelProgram(nullptr),
#if USE_EXTENSION_CODE
    m_gtpin(nullptr),
#endif
    m_isPrintEnabled(false),
    m_printBufferSize(0),
    m_threadGroupSpaceArray(CM_INIT_THREADGROUPSPACE_COUNT),
    m_threadGroupSpaceCount(0),
    m_taskArray(CM_INIT_TASK_COUNT),
    m_taskCount(0),
    m_nGPUFreqOriginal(0),
    m_nGPUFreqMin(0),
    m_nGPUFreqMax(0),
    m_vtuneOn(false),
    m_isDriverStoreEnabled(0),
    m_notifierGroup(nullptr),
    m_hasGpuCopyKernel(false),
    m_hasGpuInitKernel(false),
    m_kernelsLoaded(0),
    m_preloadKernelEnabled(true)
{
    //Initialize the structures in the class
    MOS_ZeroMemory(&m_halMaxValues, sizeof(m_halMaxValues));
    MOS_ZeroMemory(&m_halMaxValuesEx, sizeof(m_halMaxValuesEx));
    MOS_ZeroMemory(&m_cmHalCreateOption, sizeof(m_cmHalCreateOption));

    //Initialize Dev Create Param
    InitDevCreateOption( m_cmHalCreateOption, options );

    // Create the notifers
    m_notifierGroup = MOS_New(CmNotifierGroup);
}

//*-----------------------------------------------------------------------------
//| Purpose:    The common part of destructor of CmDevice, that is OS independent
//| Returns:    None.
//*-----------------------------------------------------------------------------
void CmDeviceRTBase::DestructCommon()
{
    // Delete Predefined Program
    if(m_gpuCopyKernelProgram)
    {
        DestroyProgram(m_gpuCopyKernelProgram);
    }

    if(m_surfInitKernelProgram)
    {
        DestroyProgram(m_surfInitKernelProgram);
    }

    //Free the surface/memory for print buffer
    while(!m_printBufferMems.empty())
    {
        uint8_t *mem = m_printBufferMems.front();
        m_printBufferMems.pop_front();
        MOS_AlignedFreeMemory(mem);
    }

    while(!m_printBufferUPs.empty())
    {
        CmBufferUP *buffer = m_printBufferUPs.front();
        m_printBufferUPs.pop_front();
        DestroyBufferUP(buffer);
    }

#if USE_EXTENSION_CODE
    // Free CmGTPin
    MOS_Delete(m_gtpin);
#endif

    // Solve resource release dependency issue
    // Flush Queue to make sure no task internal and connected resouces left.
    m_criticalSectionQueue.Acquire();
    for (auto iter = m_queue.begin(); iter != m_queue.end(); iter++)
    {
        (*iter)->CleanQueue();
    }
    m_criticalSectionQueue.Release();
    PCM_CONTEXT_DATA  pCmData = (PCM_CONTEXT_DATA)m_accelData;
    if (pCmData && pCmData->cmHalState && pCmData->cmHalState->advExecutor)
    {
        pCmData->cmHalState->advExecutor->WaitForAllTasksFinished();
    }

    for( uint32_t i = 0; i < m_kernelCount; i ++ )
    {
        CmKernelRT* kernel = (CmKernelRT*)m_kernelArray.GetElement( i );
        if( kernel )
        {
            CmProgramRT* program = nullptr;
            kernel->GetCmProgram(program);
            uint32_t indexInProgramArray;
            for (indexInProgramArray = 0; indexInProgramArray < m_programArray.GetSize(); indexInProgramArray++)
            {
                if (program == m_programArray.GetElement( indexInProgramArray ))
                {
                    break;
                }
            }
            CmKernelRT::Destroy( kernel, program );
            if ((program == nullptr) && (indexInProgramArray < m_programArray.GetSize()))
            {
                m_programArray.SetElement(indexInProgramArray,  nullptr);
            }
        }
    }
    m_kernelArray.Delete();

    for( uint32_t i = 0; i < m_programArray.GetSize(); i ++ )
    {
        CmProgramRT* program = (CmProgramRT*)m_programArray.GetElement( i );
        while( program ) // Program can be acquired more than once
        {
            CmProgramRT::Destroy( program );
        }
    }
    m_programArray.Delete();

    for( uint32_t i = 0; i < m_samplerArray.GetSize(); i ++ )
    {
        CmSamplerRT* sampler =  (CmSamplerRT *)m_samplerArray.GetElement( i );

        if(sampler)
        {
            SamplerIndex* index  = nullptr;
            sampler->GetIndex( index );
            CM_ASSERT( index );
            uint32_t indexValue = index->get_data();

            CmSamplerRT::Destroy( sampler );
            UnregisterSamplerState( indexValue );
        }
    }
    m_samplerArray.Delete();

    for(uint32_t i = 0; i < m_sampler8x8Array.GetSize(); i ++ )
    {
         CmSampler8x8State_RT* sampler8x8 =  (CmSampler8x8State_RT* )m_sampler8x8Array.GetElement( i );
         if(sampler8x8)
         {
            SamplerIndex* index  = nullptr;
            sampler8x8->GetIndex( index );
            CM_ASSERT( index );
            uint32_t indexValue = index->get_data();
            CmSampler8x8State_RT::Destroy( sampler8x8 );
            UnregisterSampler8x8State( indexValue );
         }
    }
    m_sampler8x8Array.Delete();

    uint32_t threadSpaceArrayUsedSize = m_threadSpaceArray.GetSize();
    for( uint32_t i = 0; i < threadSpaceArrayUsedSize; i ++ )
    {
        CmThreadSpaceRT* threadSpaceRT = (CmThreadSpaceRT*)m_threadSpaceArray.GetElement( i );
        if( threadSpaceRT )
        {
            CmThreadSpaceRT::Destroy( threadSpaceRT );
        }
    }
    m_threadSpaceArray.Delete();

    for( uint32_t i = 0; i < m_threadGroupSpaceCount; i ++ ) // Destroy thread group space array
    {
        CmThreadGroupSpace* threadGroupSpace = (CmThreadGroupSpace*)m_threadGroupSpaceArray.GetElement( i );
        if( threadGroupSpace )
        {
            CmThreadGroupSpace::Destroy( threadGroupSpace );
        }
    }
    m_threadGroupSpaceArray.Delete();

    uint32_t taskArrayUsedSize = m_taskArray.GetSize();
    for( uint32_t i = 0; i < taskArrayUsedSize; i ++ ) // Destroy task array
    {
        CmTaskRT* task = (CmTaskRT*)m_taskArray.GetElement( i );
        if( task )
        {
            CmTaskRT::Destroy( task );
        }
    }
    m_taskArray.Delete();

    for( uint32_t i = 0; i < m_veboxCount; i ++ ) // Destroy Vebox array
    {
        CmVeboxRT* vebox = (CmVeboxRT*)m_veboxArray.GetElement(i);
        if (vebox)
        {
            CmVeboxRT::Destroy(vebox);
        }
    }
    m_veboxArray.Delete();

    //Destroy Surface Manager
    CmSurfaceManager::Destroy( m_surfaceMgr );

    //Destroy Queue: Queue must be released after surface manager
    m_criticalSectionQueue.Acquire();
    for (auto iter = m_queue.begin(); iter != m_queue.end();)
    {
        DestroyQueue(*iter);
        iter = m_queue.erase(iter);
    }
    m_criticalSectionQueue.Release();

    // Notify the listeners
    if (m_notifierGroup != nullptr)
    {
        m_notifierGroup->NotifyDeviceDestroyed(this);
    }
    
    //Free the notifiers
    if (m_notifierGroup != nullptr)
    {
        MOS_Delete(m_notifierGroup);
    }

    //Free DLL handle if it is there
    if (m_hJITDll)
    {
        MOS_FreeLibrary(m_hJITDll);
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Aux Device and Initialize it
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::Initialize(MOS_CONTEXT *mosContext)
{
    int32_t result = InitializeOSSpecific(mosContext);

    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Initialize OS specific failure.");
        return result;
    }

    m_surfaceMgr = nullptr;
    CmDeviceRT *cmDevice = static_cast<CmDeviceRT*>(this);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);

    result = CmSurfaceManager::Create(
        cmDevice,
        m_halMaxValues,
        m_halMaxValuesEx,
        m_surfaceMgr );

    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Create CmSurfaceManager failure.");
        return result;
    }

    result = SetSurfaceArraySizeForAlias();
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Set surface array size failure.");
        return result;
    }

    ReadVtuneProfilingFlag();

    // Load Predefined Kernels
    if (m_preloadKernelEnabled)
    {
        CmProgram* tmpProgram = nullptr;
        int32_t ret = LoadPredefinedCopyKernel(tmpProgram);
        if (ret == CM_SUCCESS)
        {
            m_hasGpuCopyKernel = true;
        }
        ret = LoadPredefinedInitKernel(tmpProgram);
        if (ret == CM_SUCCESS)
        {
            m_hasGpuInitKernel = true;
        }
    }

    // get the last tracker
    PCM_HAL_STATE state = (( PCM_CONTEXT_DATA )m_accelData)->cmHalState;
    m_surfaceMgr->SetLatestVeboxTrackerAddr(state->renderHal->veBoxTrackerRes.data);

    if (m_notifierGroup != nullptr)
    {
        m_notifierGroup->NotifyDeviceCreated(this);
    }

    DEVICE_LOG(this);

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Buffer
//| Arguments :   size              [in]    Size of the Buffer
//|               surface           [in/out]   Reference to Pointer to CmBuffer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::CreateBuffer(uint32_t size, CmBuffer* & surface)
{
    INSERT_API_CALL_LOG();

    if( ( size < CM_MIN_SURF_WIDTH ) || ( size > CM_MAX_1D_SURF_WIDTH ) )
    {
        CM_ASSERTMESSAGE("Error: Invalid buffer size.");
        return CM_INVALID_WIDTH;
    }

    CLock locker(m_criticalSectionSurface);

    CmBuffer_RT*    bufferRT = nullptr;
    void            *sysMem = nullptr;
    int result = m_surfaceMgr->CreateBuffer(size, CM_BUFFER_N, false, bufferRT, nullptr, sysMem, false, CM_DEFAULT_COMPARISON_VALUE);
    surface = static_cast< CmBuffer* >(bufferRT);

    return result;
}

//!
//! \brief    Create a CmBuffer from an existing MOS Resource.
//! \details  CmBuffer is a wrapper of that MOS resource. This Mos resource is
//!            owned by caller.
//! \param    [in] mosResource
//!           pointer to MOS resource.
//! \param    [in,out] surface
//!           reference to pointer of surface to be created.
//! \retval   CM_SUCCESS if the CmBuffer is successfully created.
//! \retval   CM_INVALID_MOS_RESOURCE_HANDLE if mosResource is nullptr.
//! \retval   CM_OUT_OF_HOST_MEMORY if out of system memory
//! \retval   CM_EXCEED_SURFACE_AMOUNT if maximum amount of 1D surfaces is exceeded.
//! \retval   CM_FAILURE otherwise
//!
CM_RT_API int32_t CmDeviceRTBase::CreateBuffer(PMOS_RESOURCE mosResource,
                                               CmBuffer* & surface)
{
    INSERT_API_CALL_LOG();

    if(mosResource == nullptr)
    {
        return CM_INVALID_MOS_RESOURCE_HANDLE;
    }

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)GetAccelData();
    PCM_HAL_STATE state = cmData->cmHalState;

    MOS_SURFACE mosSurfDetails;
    MOS_ZeroMemory(&mosSurfDetails, sizeof(mosSurfDetails));
    int hr = state->osInterface->pfnGetResourceInfo(state->osInterface, mosResource, &mosSurfDetails);
    if(hr != MOS_STATUS_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Get resource info failure.");
        return hr;
    }

    if( (mosSurfDetails.dwWidth < CM_MIN_SURF_WIDTH ) || (mosSurfDetails.dwWidth > CM_MAX_1D_SURF_WIDTH ) )
    {
        CM_ASSERTMESSAGE("Error: Invalid buffer size.");
        return CM_INVALID_WIDTH;
    }

    CLock locker(m_criticalSectionSurface);
    CmBuffer_RT* buffer = nullptr;
    void*        sysMem = nullptr;
    int          ret = m_surfaceMgr->CreateBuffer(mosSurfDetails.dwWidth, CM_BUFFER_N, false,
                       buffer, mosResource, sysMem, false, CM_DEFAULT_COMPARISON_VALUE);
    surface = static_cast< CmBuffer* >(buffer);

    return ret;
}

//*--------------------------------------------------------------------------------------------
//| Purpose:    Create BufferUp
//| Arguments :   size              [in]     Size of the Buffer, should be uint32_t-aligned
//|               sysMem           [in]     Pointer to host memory, must be page(4K bytes)-aligned.
//|               surface          [in/out]    Reference to Pointer to CmBufferUP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::CreateBufferUP(uint32_t size,
                                                 void* sysMem,
                                                 CmBufferUP* & surface)
{
    INSERT_API_CALL_LOG();

    // size should be in the valid range and be aligned in uint32_t
    if( ( size < CM_MIN_SURF_WIDTH ) || ( size > CM_MAX_1D_SURF_WIDTH ) || (size % sizeof(uint32_t)))
    {
        CM_ASSERTMESSAGE("Error: Invalid buffer size.\n");
        return CM_INVALID_WIDTH;
    }

    if (nullptr == sysMem)
    {
        CM_ASSERTMESSAGE("Error: Pointer to host memory is null.\n");
        return CM_INVALID_ARG_VALUE;
    }
    auto uintPtr = reinterpret_cast<uintptr_t>(sysMem);
    if (uintPtr & (0x1000 - 1))
    {
        CM_ASSERTMESSAGE("Error: Pointer to host memory isn't 4K-aligned.\n");
        return CM_INVALID_ARG_VALUE;
    }

    CLock locker(m_criticalSectionSurface);

    CmBuffer_RT* bufferRT = nullptr;
    int result = m_surfaceMgr->CreateBuffer( size, CM_BUFFER_UP, false, bufferRT, nullptr, sysMem, false, CM_DEFAULT_COMPARISON_VALUE );
    surface = static_cast< CmBufferUP* >(bufferRT);

    return result;
}

//*----------------------------------------------------------------------------
//| Purpose:    Destroy BufferUp
//| Arguments :  surface          [in]    Reference to Pointer to CmBuffer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::DestroyBufferUP(CmBufferUP* & surface)
{
    INSERT_API_CALL_LOG();

    CmBuffer_RT* temp = static_cast< CmBuffer_RT* >(surface);
    if (nullptr == temp)
    {
        return CM_NULL_POINTER;
    }

    CLock locker(m_criticalSectionSurface);

    int32_t status = m_surfaceMgr->DestroySurface(temp, APP_DESTROY);

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE may be returned, which should be treated as SUCCESS.
    {
        surface = nullptr;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
    return status;
}

//*----------------------------------------------------------------------------
//| Purpose:    Forces the BufferUP to be destroyed
//| Arguments :  surface          [in]    Reference to Pointer to CmBuffer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::ForceDestroyBufferUP(CmBufferUP* & surface)
{
    INSERT_API_CALL_LOG();

    CmBuffer_RT* temp = static_cast< CmBuffer_RT* >(surface);
    if (nullptr == temp)
    {
        return CM_NULL_POINTER;
    }

    CLock locker(m_criticalSectionSurface);

    int32_t status = m_surfaceMgr->DestroySurface(temp, FORCE_DESTROY);

    if(status == CM_SUCCESS)
    {
        surface = nullptr;
    }
    return status;
}

//*--------------------------------------------------------------------------------------------
//| Purpose:    Create Surface 2D UP
//| Arguments :   width             [in]     width of the  CmSurface2DUP
//|               height            [in]     height of the CmSurface2DUP
//|               format            [in]     format of the CmSurface2DUP
//|
//|               sysMem           [in]     Pointer to host memory, must be page(4K bytes)-aligned.
//|               surface          [in/out]  Reference to  Pointer to CmSurface2DUP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::CreateSurface2DUP(uint32_t width,
                                                    uint32_t height,
                                                    CM_SURFACE_FORMAT format,
                                                    void* sysMem,
                                                    CmSurface2DUP* & surface )
{
    INSERT_API_CALL_LOG();

    int32_t result = m_surfaceMgr->Surface2DSanityCheck(width, height, format);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Surface2D sanity check failure.\n");
        return result;
    }

    if (sysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to host memory is null.\n");
        return CM_INVALID_ARG_VALUE;
    }
    auto uintPtr = reinterpret_cast<uintptr_t>(sysMem);
    if (uintPtr & (0x1000 - 1))
    {
        CM_ASSERTMESSAGE("Error: Pointer to host memory isn't 4K-aligned.\n");
        return CM_INVALID_ARG_VALUE;
    }

    CmSurface2DUPRT *surfaceRT = nullptr;
    CLock locker(m_criticalSectionSurface);
    result = m_surfaceMgr->CreateSurface2DUP( width, height, format, sysMem, surfaceRT );
    surface = surfaceRT;
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface 3D
//| Arguments :
//|               width             [in]     width of the  CmSurface3D
//|               height            [in]     height of the CmSurface3D
//|               format            [in]     format of the CmSurface3D
//|               depth             [in]     depth  of the CmSurface3D
//|               surface          [out]    Reference to Pointer to CmSurface3D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::CreateSurface3D(uint32_t width,
                                                  uint32_t height,
                                                  uint32_t depth,
                                                  CM_SURFACE_FORMAT format,
                                                  CmSurface3D* & surface )
{
    INSERT_API_CALL_LOG();

    if( ( width < CM_MIN_SURF_WIDTH ) || ( width > CM_MAX_3D_SURF_WIDTH ) )
    {
        CM_ASSERTMESSAGE("Error: Invalid surface 3D width.");
        return CM_INVALID_WIDTH;

    }
    if( ( height < CM_MIN_SURF_HEIGHT ) || ( height > CM_MAX_3D_SURF_HEIGHT ) )
    {
        CM_ASSERTMESSAGE("Error: Invalid surface 3D height.");
        return CM_INVALID_HEIGHT;
    }

    if( ( depth < CM_MIN_SURF_DEPTH ) || ( depth > CM_MAX_3D_SURF_DEPTH ) )
    {
        CM_ASSERTMESSAGE("Error: Invalid surface 3D depth.");
        return CM_INVALID_DEPTH;
    }

    CLock locker(m_criticalSectionSurface);
    CmSurface3DRT *surfaceRT = nullptr;
    int ret = m_surfaceMgr->CreateSurface3D( width, height, depth, format, surfaceRT );
    surface = surfaceRT;
    return ret;
}

CM_RT_API int32_t CmDeviceRTBase::DestroySurface( CmBuffer* & surface)
{
    CmBuffer_RT* temp = static_cast< CmBuffer_RT* >(surface);
    if (nullptr == temp)
    {
        return CM_NULL_POINTER;
    }

    CLock locker(m_criticalSectionSurface);

    int32_t status = m_surfaceMgr->DestroySurface( temp, APP_DESTROY);

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE, or  CM_SURFACE_CACHED may be returned, which should be treated as SUCCESS.
    {
        surface = nullptr;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*----------------------------------------------------------------------------
//| Purpose:    Destroy CmSurface2DUP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::DestroySurface2DUP( CmSurface2DUP* & surface)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionSurface);

    CmSurface2DUPRT *surfaceRT = static_cast<CmSurface2DUPRT *>(surface);
    if (nullptr == surfaceRT)
    {
       return CM_NULL_POINTER;
    }

    int32_t status = m_surfaceMgr->DestroySurface( surfaceRT, APP_DESTROY );

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE, or  CM_SURFACE_CACHED may be returned, which should be treated as SUCCESS.
    {
        surface = nullptr;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*----------------------------------------------------------------
//| Purpose: Destroys a CmSurface2D object and returns the status.
//*----------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::DestroySurface(CmSurface2D* &surface)
{
    INSERT_API_CALL_LOG();
    CLock locker(m_criticalSectionSurface);

    CmSurface2DRT *surfaceRT = static_cast<CmSurface2DRT*>(surface);
    if (nullptr == surfaceRT)
    {
        return CM_NULL_POINTER;
    }
    int32_t status = m_surfaceMgr->DestroySurface(surfaceRT, APP_DESTROY);

    if (status != CM_FAILURE)  // CM_SURFACE_IN_USE may be returned, which should be treated as SUCCESS.
    {
        surface = nullptr;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*------------------------------------------------------------------
//| Purpose: Destroys a CmSurface3D object and returns the status.
//*------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::DestroySurface( CmSurface3D* & surface)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionSurface);

    CmSurface3DRT *surfaceRT = static_cast<CmSurface3DRT *>(surface);
    if (nullptr == surfaceRT)
    {
        return CM_NULL_POINTER;
    }

    int32_t status = m_surfaceMgr->DestroySurface( surfaceRT, APP_DESTROY);

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE, or  CM_SURFACE_CACHED may be returned, which should be treated as SUCCESS.
    {
        surface = nullptr;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*----------------------------------------------------------------------------
//| Purpose:    Get Current platform information
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::GetGenPlatform( uint32_t &platform )
{
    if( m_platform != IGFX_UNKNOWN_CORE)
    {
        platform = m_platform;
        return CM_SUCCESS;
    }

    platform = IGFX_UNKNOWN_CORE;

    int32_t hr = 0;
    CM_QUERY_CAPS      queryCaps;
    uint32_t           querySize  = sizeof( CM_QUERY_CAPS );

    CmSafeMemSet( &queryCaps, 0, sizeof( queryCaps ) );
    queryCaps.type = CM_QUERY_GPU;

    hr  = GetCapsInternal( &queryCaps, &querySize);
    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Failed to get current GPU platform information.");
        return CM_FAILURE;
    }
    if (queryCaps.version)
    {
        platform = queryCaps.version;
    }

    return CM_SUCCESS;
}

//*----------------------------------------------------------------------------
//| Purpose:    Get Surface2D information: pitch and physical size in Video memory
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::GetSurface2DInfo(uint32_t width,
                                                   uint32_t height,
                                                   CM_SURFACE_FORMAT format,
                                                   uint32_t & pitch,
                                                   uint32_t & physicalSize)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE              hr = CM_SUCCESS;
    CM_HAL_SURFACE2D_UP_PARAM   inParam;
    PCM_CONTEXT_DATA            cmData;
    PCM_HAL_STATE               cmHalState;

    CM_CHK_CMSTATUS_GOTOFINISH(m_surfaceMgr->Surface2DSanityCheck(width, height, format));

    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_SURFACE2D_UP_PARAM ) );
    inParam.width  = width;
    inParam.height = height;
    inParam.format  = format;

    cmData = (PCM_CONTEXT_DATA)GetAccelData();
    cmHalState = cmData->cmHalState;
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmHalState->pfnGetSurface2DPitchAndSize(cmHalState, &inParam));

    pitch = inParam.pitch;
    physicalSize = inParam.physicalSize;

finish:
    return hr;
}

int32_t CmDeviceRTBase::GetSurfaceManager( CmSurfaceManager* & surfaceMgr )
{
    surfaceMgr = m_surfaceMgr;
    return CM_SUCCESS;
}

CSync* CmDeviceRTBase::GetSurfaceLock()
{
    return &m_criticalSectionReadWriteSurface2D;
}

CSync* CmDeviceRTBase::GetSurfaceCreationLock()
{
    return &m_criticalSectionSurface;
}

CSync* CmDeviceRTBase::GetProgramKernelLock()
{
    return &m_criticalSectionProgramKernel;
}

std::vector<CmQueueRT *> &CmDeviceRTBase::GetQueue()
{
    return m_queue;
}

CSync* CmDeviceRTBase::GetQueueLock()
{
    return &m_criticalSectionQueue;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Max values from Device
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::GetHalMaxValues(CM_HAL_MAX_VALUES* & halMaxValues,
                                        CM_HAL_MAX_VALUES_EX* & halMaxValuesEx)
{
    halMaxValues = &m_halMaxValues;
    halMaxValuesEx = &m_halMaxValuesEx;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Max values by Caps
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::GetMaxValueFromCaps(CM_HAL_MAX_VALUES &maxValues,
                                            CM_HAL_MAX_VALUES_EX &maxValuesEx)
{
    CM_QUERY_CAPS      queryCaps;
    uint32_t           querySize  = sizeof( CM_QUERY_CAPS );
    CmSafeMemSet( &queryCaps, 0, sizeof( CM_QUERY_CAPS ) );
    queryCaps.type = CM_QUERY_MAX_VALUES;

    int32_t hr = GetCapsInternal(&queryCaps, &querySize);
    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Failed to get max values by GetCaps.");
        return CM_FAILURE;
    }

    maxValues = queryCaps.maxValues;
    maxValues.maxArgsPerKernel = (queryCaps.maxValues.maxArgsPerKernel > CM_MAX_ARGS_PER_KERNEL)?(CM_MAX_ARGS_PER_KERNEL):queryCaps.maxValues.maxArgsPerKernel;

    CmSafeMemSet( &queryCaps, 0, sizeof( CM_QUERY_CAPS ) );
    queryCaps.type = CM_QUERY_MAX_VALUES_EX;

    hr = GetCapsInternal(&queryCaps, &querySize);
    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Failed to get max values by GetCaps.");
        return CM_FAILURE;
    }
    maxValuesEx = queryCaps.maxValuesEx;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Caps from Internal
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::GetCapsInternal(void  *caps, uint32_t *size)
{
    PCM_QUERY_CAPS          queryCaps;
    PCM_CONTEXT_DATA        cmData;
    PCM_HAL_STATE           cmHalState;

    CM_RETURN_CODE  hr          = CM_SUCCESS;

    if ((!size)  || (!caps) || (*size < sizeof(CM_QUERY_CAPS)))
    {
        CM_ASSERTMESSAGE("Error: Invalid arguments.");
        hr = CM_FAILURE;
        goto finish;
    }

    queryCaps  = (PCM_QUERY_CAPS)caps;
    *size     = sizeof(CM_QUERY_CAPS);

    if (queryCaps->type == CM_QUERY_VERSION)
    {
        queryCaps->version    = CM_VERSION;
        hr = CM_SUCCESS;
        goto finish;
    }

    cmData = (PCM_CONTEXT_DATA)GetAccelData();
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmData);

    cmHalState = cmData->cmHalState;
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmHalState);

    switch (queryCaps->type)
    {
    case CM_QUERY_REG_HANDLE:
        queryCaps->hRegistration   = QueryRegHandleInternal(cmHalState);
        break;
    case CM_QUERY_MAX_VALUES:
        CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmHalState->pfnGetMaxValues(cmHalState, &queryCaps->maxValues));
        break;

    case CM_QUERY_MAX_VALUES_EX:
        CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmHalState->pfnGetMaxValuesEx(cmHalState, &queryCaps->maxValuesEx));
        break;

    case CM_QUERY_GPU:
    case CM_QUERY_GT:
    case CM_QUERY_MIN_RENDER_FREQ:
    case CM_QUERY_MAX_RENDER_FREQ:
    case CM_QUERY_STEP:
    case CM_QUERY_GPU_FREQ:
        hr = QueryGPUInfoInternal(queryCaps);
        if (hr != CM_SUCCESS)
            goto finish;
        break;

    case CM_QUERY_SURFACE2D_FORMAT_COUNT:
        queryCaps->surface2DCount = CM_MAX_SURFACE2D_FORMAT_COUNT_INTERNAL;
        break;

    case CM_QUERY_SURFACE2D_FORMATS:
        hr = QuerySurface2DFormatsInternal(queryCaps);
        if (hr != CM_SUCCESS)
            goto finish;
        break;

    case CM_QUERY_PLATFORM_INFO:
        CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmHalState->pfnGetPlatformInfo(cmHalState, &queryCaps->platformInfo, false));
        break;
    default:
        hr = CM_FAILURE;
        goto finish;
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//! Get HW capability.
//! Input :
//!     1) Name of cap to query, only CAP_HW_THREAD_COUNT is supported now.
//!     2) size of memory where the cap value should return to.
//!        User should make sure the size is larger than the size of cap value which is really returned.
//!     3) Pointer pointing to memory where the cap value should return to
//! Output:
//!     1) size of cap value which is really returned.
//!     2) CM_SUCCESS if cap value is successfully returned.
//!        CM_FAILURE otherwise;
//*-----------------------------------------------------------------------------
 int32_t CmDeviceRTBase::GetCaps(CM_DEVICE_CAP_NAME capName,
                                 uint32_t & capValueSize,
                                 void* capValue )
{
    PCM_CONTEXT_DATA        cmData;
    PCM_HAL_STATE           cmHalState;
    CM_RETURN_CODE          hr = CM_SUCCESS;

    if (capValue == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to cap value is null.");
        return CM_NULL_POINTER;
    }

    cmData = (PCM_CONTEXT_DATA)GetAccelData();
    if(cmData == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to CM context data is null.");
        return CM_NULL_POINTER;
    }

    cmHalState = cmData->cmHalState;
    if(cmHalState == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to CM hal state is null.");
        return CM_NULL_POINTER;
    }

    switch( capName )
    {
    case CAP_KERNEL_COUNT_PER_TASK:
        if( capValueSize >= sizeof( m_halMaxValues.maxKernelsPerTask ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxKernelsPerTask );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxKernelsPerTask, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_KERNEL_BINARY_SIZE:
        if( capValueSize >= sizeof( m_halMaxValues.maxKernelBinarySize ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxKernelBinarySize );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxKernelBinarySize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SAMPLER_COUNT:
        if( capValueSize >= sizeof( m_halMaxValues.maxSamplerTableSize ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxSamplerTableSize );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxSamplerTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SAMPLER_COUNT_PER_KERNEL:
        if( capValueSize >= sizeof( m_halMaxValues.maxSamplersPerKernel ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxSamplersPerKernel );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxSamplersPerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_BUFFER_COUNT:
        if( capValueSize >= sizeof( m_halMaxValues.maxBufferTableSize ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxBufferTableSize );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxBufferTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2D_COUNT:
        if( capValueSize >= sizeof( m_halMaxValues.max2DSurfaceTableSize ) )
        {
            capValueSize = sizeof( m_halMaxValues.max2DSurfaceTableSize );
            CmSafeMemCopy( capValue, &m_halMaxValues.max2DSurfaceTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE3D_COUNT:
        if( capValueSize >= sizeof( m_halMaxValues.max3DSurfaceTableSize ) )
        {
            capValueSize = sizeof( m_halMaxValues.max3DSurfaceTableSize );
            CmSafeMemCopy( capValue, &m_halMaxValues.max3DSurfaceTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2DUP_COUNT:
        if( capValueSize >= sizeof( m_halMaxValuesEx.max2DUPSurfaceTableSize ) )
        {
            capValueSize = sizeof( m_halMaxValuesEx.max2DUPSurfaceTableSize );
            CmSafeMemCopy( capValue, &m_halMaxValuesEx.max2DUPSurfaceTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE_COUNT_PER_KERNEL:
        if( capValueSize >= sizeof( m_halMaxValues.maxSurfacesPerKernel ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxSurfacesPerKernel );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxSurfacesPerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_ARG_COUNT_PER_KERNEL:
        if( capValueSize >= sizeof( m_halMaxValues.maxArgsPerKernel ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxArgsPerKernel );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxArgsPerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_ARG_SIZE_PER_KERNEL:
        if( capValueSize >= sizeof( m_halMaxValues.maxArgByteSizePerKernel ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxArgByteSizePerKernel );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxArgByteSizePerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_TASK:
        if( capValueSize >= sizeof( m_halMaxValues.maxUserThreadsPerTask ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxUserThreadsPerTask );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxUserThreadsPerTask, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_MEDIA_WALKER:
        if( capValueSize >= sizeof( m_halMaxValuesEx.maxUserThreadsPerMediaWalker ) )
        {
            capValueSize = sizeof( m_halMaxValuesEx.maxUserThreadsPerMediaWalker );
            CmSafeMemCopy( capValue, &m_halMaxValuesEx.maxUserThreadsPerMediaWalker, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_THREAD_GROUP:
        if( capValueSize >= sizeof( m_halMaxValuesEx.maxUserThreadsPerThreadGroup ) )
        {
            capValueSize = sizeof( m_halMaxValuesEx.maxUserThreadsPerThreadGroup );
            CmSafeMemCopy( capValue, &m_halMaxValuesEx.maxUserThreadsPerThreadGroup, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_TASK_NO_THREAD_ARG:
        if( capValueSize >= sizeof( m_halMaxValues.maxUserThreadsPerTaskNoThreadArg ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxUserThreadsPerTaskNoThreadArg );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxUserThreadsPerTaskNoThreadArg, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_HW_THREAD_COUNT:
        if( capValueSize >= sizeof( m_halMaxValues.maxHwThreads ) )
        {
            capValueSize = sizeof( m_halMaxValues.maxHwThreads );
            CmSafeMemCopy( capValue, &m_halMaxValues.maxHwThreads, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2D_FORMAT_COUNT:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            capValueSize = sizeof( uint32_t );
            uint32_t formatCount = CM_MAX_SURFACE2D_FORMAT_COUNT;
            CmSafeMemCopy( capValue, &formatCount, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2D_FORMATS:
        return QuerySurface2DFormats(capValue, capValueSize);
    case CAP_SURFACE3D_FORMAT_COUNT:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            capValueSize = sizeof( uint32_t );
            uint32_t formatCount = CM_MAX_SURFACE3D_FORMAT_COUNT;
            CmSafeMemCopy( capValue, &formatCount, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE3D_FORMATS:
        if( capValueSize >= CM_MAX_SURFACE3D_FORMAT_COUNT  * sizeof(CM_SURFACE_FORMAT) )
        {
            capValueSize = CM_MAX_SURFACE3D_FORMAT_COUNT  * sizeof(CM_SURFACE_FORMAT) ;
            CM_SURFACE_FORMAT formats[ CM_MAX_SURFACE3D_FORMAT_COUNT ] =
            {
                CM_SURFACE_FORMAT_X8R8G8B8,
                CM_SURFACE_FORMAT_A8R8G8B8,
                CM_SURFACE_FORMAT_A16B16G16R16
            };
            CmSafeMemCopy( capValue, formats, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_GPU_PLATFORM:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            uint32_t platform = PLATFORM_INTEL_UNKNOWN;
            capValueSize = sizeof( uint32_t );
            cmHalState->cmHalInterface->GetGenPlatformInfo(&platform, nullptr, nullptr);
            CmSafeMemCopy( capValue, &platform, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_GT_PLATFORM:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            CM_QUERY_CAPS   queryCaps;
            queryCaps.type = CM_QUERY_GT;
            uint32_t queryCapsSize  = sizeof( CM_QUERY_CAPS );
            hr = (CM_RETURN_CODE)GetCapsInternal(&queryCaps, &queryCapsSize);
            if ( hr != CM_SUCCESS)
            {
                return hr;
            }
            capValueSize = sizeof( uint32_t );
            uint32_t gtPlatform = queryCaps.genGT;
            CmSafeMemCopy( capValue, &gtPlatform, capValueSize );
            return hr;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_MIN_FREQUENCY:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            CM_QUERY_CAPS   queryCaps;
            queryCaps.type = CM_QUERY_MIN_RENDER_FREQ;
            uint32_t queryCapsSize  = sizeof( CM_QUERY_CAPS );
            hr = (CM_RETURN_CODE)GetCapsInternal(&queryCaps, &queryCapsSize);
            if (hr != CM_SUCCESS)
            {
                return hr;
            }
            uint32_t frequency = queryCaps.minRenderFreq;
            capValueSize = sizeof( uint32_t );
            CmSafeMemCopy( capValue, &frequency, capValueSize );
            return hr;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_MAX_FREQUENCY:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            CM_QUERY_CAPS   queryCaps;
            queryCaps.type = CM_QUERY_MAX_RENDER_FREQ;
            uint32_t queryCapsSize  = sizeof( CM_QUERY_CAPS );
            hr = (CM_RETURN_CODE)GetCapsInternal(&queryCaps, &queryCapsSize);
            if (hr != CM_SUCCESS)
            {
                return hr;
            }
            uint32_t frequency = queryCaps.maxRenderFreq;
            capValueSize = sizeof( uint32_t );
            CmSafeMemCopy( capValue, &frequency, capValueSize );
            return hr;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_GPU_CURRENT_FREQUENCY:
        if( (m_ddiVersion >= CM_DDI_3_0) && (capValueSize >= sizeof( uint32_t )) )
        {
            CM_QUERY_CAPS   queryCaps;
            queryCaps.type = CM_QUERY_GPU_FREQ;
            uint32_t queryCapsSize  = sizeof( CM_QUERY_CAPS );
            hr = (CM_RETURN_CODE)GetCapsInternal(&queryCaps, &queryCapsSize);
            if (hr != CM_SUCCESS)
            {
                return hr;
            }
            uint32_t frequency = queryCaps.gpuCurrentFreq;
            capValueSize = sizeof( uint32_t );
            CmSafeMemCopy( capValue, &frequency, capValueSize );
            return hr;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_PLATFORM_INFO:
        if (capValueSize >= sizeof(CM_PLATFORM_INFO))
        {
            CM_QUERY_CAPS   queryCaps;
            queryCaps.type = CM_QUERY_PLATFORM_INFO;
            uint32_t queryCapsSize = sizeof(CM_QUERY_CAPS);
            hr = (CM_RETURN_CODE)GetCapsInternal(&queryCaps, &queryCapsSize);
            if (hr != CM_SUCCESS)
            {
                return hr;
            }
            capValueSize = sizeof(CM_PLATFORM_INFO);
            PCM_PLATFORM_INFO platformInfo = &(queryCaps.platformInfo);
            CmSafeMemCopy(capValue, platformInfo, capValueSize);
            return hr;
        }
        else
        {
            return CM_FAILURE;
        }
    case CAP_MAX_BUFFER_SIZE:
        if (capValueSize >= sizeof(unsigned int))
        {
            capValueSize = sizeof(unsigned int);
            unsigned int maxBufferSize = CM_MAX_1D_SURF_WIDTH;
            CmSafeMemCopy(capValue, &maxBufferSize, capValueSize);
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }
    default:
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Load program from memory
//| Arguments :
//|               commonISACode    [in]       pointer to memory where common isa locates
//|               size              [in]       size of common isa
//|               program          [in/out]   Pointer to CmProgram
//|               options           [in]       options : non-jitter,jitter
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::LoadProgram(void* commonISACode,
                                              const uint32_t size,
                                              CmProgram*& program,
                                              const char* options )
{
    INSERT_API_CALL_LOG();

    int32_t result;

    if ((commonISACode == nullptr) || (size == 0))
    {
        CM_ASSERTMESSAGE("Error: Invalid common isa code.");
        return CM_INVALID_COMMON_ISA;
    }

    CLock locker(m_criticalSectionProgramKernel);

    uint32_t firstfreeslot = m_programArray.GetFirstFreeIndex();

    CmProgramRT *programRT = static_cast<CmProgramRT *>(program);
    CmDeviceRT *cmDevice = static_cast<CmDeviceRT*>(this);
    result = CmProgramRT::Create(cmDevice, commonISACode, size, programRT, options, firstfreeslot );
    if( result == CM_SUCCESS )
    {
        m_programArray.SetElement( firstfreeslot, programRT );
        m_programCount ++;
    }
    program = programRT;

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Program
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::DestroyProgram(CmProgram* & program)
{
    INSERT_API_CALL_LOG();

    if( program == nullptr )
    {
        return CM_FAILURE;
    }

    CLock locker(m_criticalSectionProgramKernel);

    CmProgramRT *programRT = static_cast<CmProgramRT *>(program);
    uint32_t indexInProgramArrary = programRT->GetProgramIndex();
    if( programRT == m_programArray.GetElement( indexInProgramArrary ) )
    {
        CmProgramRT::Destroy( programRT );
        if( programRT == nullptr )
        {
            m_programArray.SetElement( indexInProgramArrary, nullptr );
            m_programCount--;
            program = programRT;
        }
        return CM_SUCCESS;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to destroy CmProgram.");
        return CM_FAILURE;
    }

}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Kernel
//| Arguments :
//|               kernel           [out]      pointer to CmKernel
//|               kernelName        [in]       string of kernel's name
//|               program          [in/out]   Pointer to CmProgram
//|               options           [in]       options : non-jitter,jitter
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::CreateKernel(CmProgram* program,
                                               const char* kernelName,
                                               CmKernel* & kernel,
                                               const char* options )
{
    INSERT_API_CALL_LOG();

    if(program == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to CmProgram is null.");
        return CM_NULL_POINTER;
    }

    CLock locker(m_criticalSectionProgramKernel);

    uint32_t freeSlotInKernelArray = m_kernelArray.GetFirstFreeIndex();
    CmProgramRT *programRT = static_cast<CmProgramRT *>(program);
    CmKernelRT *kernelRT = static_cast<CmKernelRT *>(kernel);
    CmDeviceRT *cmDevice = static_cast<CmDeviceRT*>(this);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);

    int32_t result = CmKernelRT::Create(cmDevice, programRT, kernelName, freeSlotInKernelArray, m_kernelCount, kernelRT, options );
    kernel = kernelRT;
    if( result == CM_SUCCESS )
    {
        m_kernelArray.SetElement( freeSlotInKernelArray, kernel );
        m_kernelCount ++;
    }

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::DestroyKernel(CmKernel*& kernel)
{
    INSERT_API_CALL_LOG();

    if( kernel == nullptr )
    {
        return CM_NULL_POINTER;
    }

    CLock locker(m_criticalSectionProgramKernel);

    CmKernelRT *kernelRT = static_cast<CmKernelRT *>(kernel);
    uint32_t indexInKernelArrary = kernelRT->GetKernelIndex();
    if( kernelRT == m_kernelArray.GetElement( indexInKernelArrary ) )
    {
        CmProgramRT* program = nullptr;
        kernelRT->GetCmProgram(program);
        if ( program == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Failed to get valid program.");
            return CM_NULL_POINTER;
        }

        uint32_t indexInProgramArray = program->GetProgramIndex();

        if (program == m_programArray.GetElement( indexInProgramArray ))
        {
            CmKernelRT::Destroy( kernelRT, program );
            kernel = kernelRT;

            if(kernelRT == nullptr)
            {
                m_kernelArray.SetElement( indexInKernelArrary, nullptr );
            }

            if (program == nullptr)
            {
                m_programArray.SetElement(indexInProgramArray,  nullptr);
            }

            // Note: NOT reduce m_kernelCount here, need to make it to loop mode later
            return CM_SUCCESS;
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Failed to destroy kernel.");
            return CM_FAILURE;
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to destroy kernel.");
        return CM_FAILURE;
    }
    return CM_SUCCESS;
}

CM_RT_API int32_t CmDeviceRTBase::CreateQueue(CmQueue* &queue)
{
    INSERT_API_CALL_LOG();

    CM_QUEUE_CREATE_OPTION queueCreateOption = CM_DEFAULT_QUEUE_CREATE_OPTION;
    m_criticalSectionQueue.Acquire();
    for (auto iter = m_queue.begin(); iter != m_queue.end(); ++iter)
    {
        CM_QUEUE_TYPE queueType = (*iter)->GetQueueOption().QueueType;
        if (queueType == CM_QUEUE_TYPE_RENDER)
        {
            queue = (*iter);
            m_criticalSectionQueue.Release();
            return CM_SUCCESS;
        }
    }
    m_criticalSectionQueue.Release();

    // Check queue type redirect is needed.
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState->cmHalInterface);
    if (cmData->cmHalState->cmHalInterface->IsRedirectRcsToCcs())
    {
        queueCreateOption.QueueType = CM_QUEUE_TYPE_COMPUTE;
    }
    else
    {
        queueCreateOption.QueueType = CM_QUEUE_TYPE_RENDER;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // Check queue type override for debugging is needed.
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData = {0};
    if (MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_MDF_DEFAULT_CM_QUEUE_TYPE_ID,
            &UserFeatureData) == MOS_STATUS_SUCCESS )
    {
        if (UserFeatureData.u32Data == CM_QUEUE_TYPE_RENDER
            || UserFeatureData.u32Data == CM_QUEUE_TYPE_COMPUTE)
        {
            queueCreateOption.QueueType
                    = (CM_QUEUE_TYPE)UserFeatureData.u32Data;
        }
    }
#endif

    return CreateQueueEx(queue, queueCreateOption);
}

CM_RT_API int32_t
CmDeviceRTBase::CreateQueueEx(CmQueue* &queue,
                              CM_QUEUE_CREATE_OPTION queueCreateOption)
{
    INSERT_API_CALL_LOG();
    CLock locker(m_criticalSectionQueue);

    CmQueueRT *queueRT = nullptr;
    if (CM_QUEUE_TYPE_RENDER == queueCreateOption.QueueType)
    {
        for (auto iter = m_queue.begin(); iter != m_queue.end(); ++iter)
        {
            CM_QUEUE_TYPE queueType = (*iter)->GetQueueOption().QueueType;
            unsigned int gpuContext = (*iter)->GetQueueOption().GPUContext;
            if (queueType == CM_QUEUE_TYPE_RENDER
                && gpuContext == queueCreateOption.GPUContext)
            {
                queue = (*iter);
                return CM_SUCCESS;
            }
        }
    }

    CmDeviceRT *cmDevice = static_cast<CmDeviceRT*>(this);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    int32_t result = CmQueueRT::Create(cmDevice, queueRT, queueCreateOption);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Failed to create the queue.");
        return result;
    }
    m_queue.push_back(queueRT);
    queue = queueRT;

    return result;
}

CM_RT_API int32_t CmDeviceRTBase::CreateTask(CmTask *& task)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionTask);

    uint32_t freeSlotInTaskArray = m_taskArray.GetFirstFreeIndex();
    CmTaskRT *taskRT = nullptr;
    CmDeviceRT *cmDevice = static_cast<CmDeviceRT*>(this);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    int32_t result = CmTaskRT::Create(cmDevice, freeSlotInTaskArray, m_halMaxValues.maxKernelsPerTask, taskRT);
    if (result == CM_SUCCESS)
    {
        m_taskArray.SetElement( freeSlotInTaskArray, taskRT );
        m_taskCount ++;
    }
    task = taskRT;
    return result;
}

int32_t CmDeviceRTBase::DestroyQueue(CmQueueRT* & queue)
{
    if(queue == nullptr )
    {
        return CM_NULL_POINTER;
    }

    return CmQueueRT::Destroy(queue);
}

CM_RT_API int32_t CmDeviceRTBase::DestroyTask(CmTask*& task)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionTask);

    if( task == nullptr )
    {
        return CM_FAILURE;
    }

    CmTaskRT *taskRT = static_cast<CmTaskRT *>(task);
    uint32_t index = taskRT->GetIndexInTaskArray();
    if(taskRT == (CmTaskRT *)m_taskArray.GetElement( index ))
    {
        int32_t status = CmTaskRT::Destroy(taskRT);
        if(status == CM_SUCCESS)
        {
            m_taskArray.SetElement( index, nullptr );
            task = nullptr;
            return CM_SUCCESS;
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Failed to destroy task.");
            return status;
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to destroy task.");
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//! Create a 2-dimensional dependency board. Each board is corrsponding to a task.
//! Each board unit is notated as a pair of X/Y coordinates, which is in the range of [0, width -1] or [0. heigh-1]
//! Each board uint is correspinding to a thread in the task.
//! Input :
//!     1) width and height of the dependency board
//! OUTPUT :
//!     CM_SUCCESS if CmThreadSpace is successfully created.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::CreateThreadSpace(uint32_t width,
                                                    uint32_t height,
                                                    CmThreadSpace* & threadSpace)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionThreadSpace);

    uint32_t freeSlotInThreadSpaceArray = m_threadSpaceArray.GetFirstFreeIndex();
    CmThreadSpaceRT *threadSpaceRT = nullptr;
    CmDeviceRT *cmDevice = static_cast<CmDeviceRT*>(this);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    int32_t result = CmThreadSpaceRT::Create(cmDevice, freeSlotInThreadSpaceArray, width, height, threadSpaceRT );
    if (result == CM_SUCCESS)
    {
        m_threadSpaceArray.SetElement( freeSlotInThreadSpaceArray, threadSpaceRT );
        m_threadSpaceCount ++;
    }
    threadSpace = threadSpaceRT;

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Thread Space
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::DestroyThreadSpace(CmThreadSpace* & threadSpace)
{
    INSERT_API_CALL_LOG();

    if( threadSpace == nullptr )
    {
        return CM_FAILURE;
    }

    CmThreadSpaceRT *threadSpaceRT = static_cast<CmThreadSpaceRT *>(threadSpace);
    uint32_t indexTs = threadSpaceRT->GetIndexInTsArray();

    CLock locker(m_criticalSectionThreadSpace);
    if(threadSpace == m_threadSpaceArray.GetElement( indexTs ))
    {
        int32_t status = CmThreadSpaceRT::Destroy( threadSpaceRT );
        if(status == CM_SUCCESS)
        {
            m_threadSpaceArray.SetElement( indexTs, nullptr );
            threadSpace = nullptr;
            return CM_SUCCESS;
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Failed to destroy thread space.");
            return status;
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to destroy thread space.");
        return CM_FAILURE;
    }

}

CM_RT_API int32_t
CmDeviceRTBase::CreateVmeSurfaceG7_5(CmSurface2D* curSurface,
                                     CmSurface2D** forwardSurfaces,
                                     CmSurface2D** backwardSurfaces,
                                     const uint32_t forwardSurfaceCount,
                                     const uint32_t backwardSurfaceCount,
                                     SurfaceIndex* & vmeIndex)
{
    INSERT_API_CALL_LOG();

    if(curSurface == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_NULL_POINTER;
    }

    CmSurface2DRT* currentRT = static_cast<CmSurface2DRT *>(curSurface) ;
    CmSurface2DRT** forward  = nullptr;
    CmSurface2DRT** backward = nullptr;

    if( ! currentRT )
    {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_INVALID_ARG_VALUE;
    }

    if(forwardSurfaces != nullptr)
    {
        forward = MOS_NewArray( CmSurface2DRT*, forwardSurfaceCount);
        if(forward == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
        for(uint32_t i = 0; i< forwardSurfaceCount; i++)
        {
            forward[i] = static_cast<CmSurface2DRT *>( forwardSurfaces[i] );
            if(forward[i] == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Invalid forward surfaces.");
                MosSafeDeleteArray(forward);
                return CM_INVALID_ARG_VALUE;
            }
        }
    }

    if(backwardSurfaces != nullptr)
    {
        backward = MOS_NewArray(CmSurface2DRT*,backwardSurfaceCount);
        if(backward == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            MosSafeDeleteArray(forward);
            return CM_OUT_OF_HOST_MEMORY;
        }
        for(uint32_t i = 0; i< backwardSurfaceCount; i++)
        {
            backward[i] = static_cast<CmSurface2DRT *>( backwardSurfaces[i] );
            if(backward[i] == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Invalid backward surfaces.");
                MosSafeDeleteArray(forward);
                MosSafeDeleteArray(backward);
                return CM_INVALID_ARG_VALUE;
            }
        }
    }

    CLock locker(m_criticalSectionSurface);

    int32_t status = m_surfaceMgr->CreateVmeSurface(currentRT, forward, backward, forwardSurfaceCount, backwardSurfaceCount, vmeIndex);

    MosSafeDeleteArray(forward);
    MosSafeDeleteArray(backward);

    return status;
}

CM_RT_API int32_t CmDeviceRTBase::DestroyVmeSurfaceG7_5(SurfaceIndex* & vmeIndex)
{
    INSERT_API_CALL_LOG();
    return DestroyVmeSurface( vmeIndex );
}

CM_RT_API int32_t CmDeviceRTBase::SetVmeSurfaceStateParam(SurfaceIndex* vmeIndex,
                                     CM_VME_SURFACE_STATE_PARAM *surfStateParam)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionSurface);

    CM_RETURN_CODE  hr = CM_SUCCESS;
    CmSurface *cmSurface = nullptr;
    CmSurfaceVme *vmeSurface = nullptr;

    CM_CHK_NULL_GOTOFINISH_CMERROR(vmeIndex);
    CM_CHK_NULL_GOTOFINISH_CMERROR(surfStateParam);

    m_surfaceMgr->GetSurface(vmeIndex->get_data(), cmSurface);
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmSurface);

    // check if it is a vme index
    if (cmSurface->Type() != CM_ENUM_CLASS_TYPE_CMSURFACEVME)
    {
        CM_ASSERTMESSAGE("Error: SetVmeSurfaceStateParam only can config VME surfaces.");
        return CM_INVALID_ARG_INDEX;
    }

    vmeSurface = static_cast<CmSurfaceVme *>(cmSurface);
    vmeSurface->SetSurfaceStateResolution(surfStateParam->width, surfStateParam->height);

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//! Create a CmSampler.
//! Input :
//!     1) Const Reference to the sampler state data structure.
//!     2) Reference to the pointer to the CmSampler .
//! Output:
//!     CM_SUCCESS if the CmSampler is successfully created;
//!     CM_OUT_OF_HOST_MEMORY if out of system memory;
//!     CM_FAILURE otherwise;
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::CreateSampler(const CM_SAMPLER_STATE& samplerState,
                              CmSampler* & sampler)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionSampler);

    uint32_t index = 0;
    int32_t hr= RegisterSamplerState( samplerState, index );
    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Register sampler state failure.");
        return CM_EXCEED_SAMPLER_AMOUNT;
    }

    CmSamplerRT* ptmp = nullptr;
    int32_t result = CmSamplerRT::Create( index, ptmp );
    if( result == CM_SUCCESS )
    {
        m_samplerArray.SetElement( index,  ptmp );
        sampler = static_cast< CmSampler* >(ptmp);
    }
    else
    {
        UnregisterSamplerState( index );
    }
    return result;
}

CM_RT_API int32_t
CmDeviceRTBase::CreateSamplerEx(const CM_SAMPLER_STATE_EX& samplerState,
                                CmSampler* & sampler)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionSampler);

    uint32_t index = 0;
    int32_t hr= RegisterSamplerStateEx( samplerState, index );
    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Register sampler state failure.");
        return CM_EXCEED_SAMPLER_AMOUNT;
    }

    CmSamplerRT* ptmp = nullptr;
    int32_t result = CmSamplerRT::Create( index, ptmp );
    if( result == CM_SUCCESS )
    {
        m_samplerArray.SetElement( index,  ptmp );
        sampler = static_cast< CmSampler* >(ptmp);
    }
    else
    {
        UnregisterSamplerState( index );
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Sampler
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::DestroySampler(CmSampler*& sampler)
{
    INSERT_API_CALL_LOG();
    CLock locker(m_criticalSectionSampler);

    CmSamplerRT* temp = nullptr;
    if(sampler != nullptr)
    {
        temp = static_cast< CmSamplerRT* >(sampler);
    }
    else
    {
        return CM_FAILURE;
    }

    SamplerIndex* index = nullptr;
    temp->GetIndex( index );
    CM_ASSERT( index );
    uint32_t indexValue = index->get_data();

    CM_ASSERT( m_samplerArray.GetElement( indexValue ) == (temp) );

    int32_t status = CmSamplerRT::Destroy( temp );
    if(status == CM_SUCCESS)
    {
        UnregisterSamplerState( indexValue );
        m_samplerArray.SetElement( indexValue, nullptr );
        sampler = nullptr;
    }

    return status;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register Sampler State in CM devie's table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::RegisterSamplerState(const CM_SAMPLER_STATE& samplerState,
                                             uint32_t& index)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    index = 0;

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)GetAccelData();

    CM_HAL_SAMPLER_PARAM param;
    MOS_ZeroMemory(&param, sizeof(CM_HAL_SAMPLER_PARAM));
    param.addressU = samplerState.addressU;
    param.addressV = samplerState.addressV;
    param.addressW = samplerState.addressW;
    param.magFilter = samplerState.magFilterType;
    param.minFilter = samplerState.minFilterType;
    param.handle = 0;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnRegisterSampler(cmData->cmHalState, &param));

    index = param.handle;

finish:
    return hr;
}

int32_t
CmDeviceRTBase::RegisterSamplerStateEx(const CM_SAMPLER_STATE_EX& samplerState,
                                       uint32_t& index)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    index = 0;

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)GetAccelData();

    CM_HAL_SAMPLER_PARAM param;
    MOS_ZeroMemory(&param, sizeof(CM_HAL_SAMPLER_PARAM));
    param.addressU = samplerState.addressU;
    param.addressV = samplerState.addressV;
    param.addressW = samplerState.addressW;
    param.magFilter = samplerState.magFilterType;
    param.minFilter = samplerState.minFilterType;
    param.handle = 0;

    param.surfaceFormat = (CM_HAL_PIXEL_TYPE)samplerState.SurfaceFormat;
    switch (param.surfaceFormat)
    {
        case CM_HAL_PIXEL_UINT:
            param.borderColorRedU = samplerState.BorderColorRedU;
            param.borderColorGreenU = samplerState.BorderColorGreenU;
            param.borderColorBlueU = samplerState.BorderColorBlueU;
            param.borderColorAlphaU = samplerState.BorderColorAlphaU;
            break;
        case CM_HAL_PIXEL_SINT:
            param.borderColorRedS = samplerState.BorderColorRedS;
            param.borderColorGreenS = samplerState.BorderColorGreenS;
            param.borderColorBlueS = samplerState.BorderColorBlueS;
            param.borderColorAlphaS = samplerState.BorderColorAlphaS;
            break;
        default:
            param.borderColorRedF = samplerState.BorderColorRedF;
            param.borderColorGreenF = samplerState.BorderColorGreenF;
            param.borderColorBlueF = samplerState.BorderColorBlueF;
            param.borderColorAlphaF = samplerState.BorderColorAlphaF;
    }

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnRegisterSampler(cmData->cmHalState, &param));

    index = param.handle;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Unregister Sampler State in CM devie's table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::UnregisterSamplerState(uint32_t index)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)GetAccelData();

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnRegisterSampler(cmData->cmHalState, index));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler8x8 State
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::CreateSampler8x8(const CM_SAMPLER_8X8_DESCR  & sampler8x8Descriptor,
                                 CmSampler8x8*& sampler8x8)
{
    INSERT_API_CALL_LOG();
    CLock locker(m_criticalSectionSampler8x8);

    int32_t result = CM_FAILURE;

    if((sampler8x8Descriptor.stateType == CM_SAMPLER8X8_AVS && sampler8x8Descriptor.avs == nullptr) ||
        (sampler8x8Descriptor.stateType == CM_SAMPLER8X8_CONV && sampler8x8Descriptor.conv == nullptr) ||
        (sampler8x8Descriptor.stateType == CM_SAMPLER8X8_MISC && sampler8x8Descriptor.misc == nullptr) ||
        (sampler8x8Descriptor.stateType == CM_SAMPLER8X8_NONE && sampler8x8Descriptor.conv != nullptr) ||
        sampler8x8 != nullptr)  {
        CM_ASSERTMESSAGE("Error: Invalid arguments.");
        return CM_INVALID_ARG_VALUE;
    }
    CmSampler8x8State_RT* ptmp = nullptr;
    uint32_t index = 0;

    int32_t hr = RegisterSampler8x8State( sampler8x8Descriptor, index );
    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Register sampler8x8 state failure.");
        return CM_EXCEED_SAMPLER_AMOUNT;
    }

    result = CmSampler8x8State_RT::Create( sampler8x8Descriptor, index, ptmp );
    if( result == CM_SUCCESS )
    {
        m_sampler8x8Array.SetElement( index, ptmp );
        sampler8x8 = static_cast< CmSampler8x8* >(ptmp);
    }
    else
    {
        UnregisterSampler8x8State( index );
    }

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Sampler8x8 State
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::DestroySampler8x8(CmSampler8x8*& sampler8x8)
{
    INSERT_API_CALL_LOG();
    CLock locker(m_criticalSectionSampler8x8);

    CmSampler8x8State_RT* temp = nullptr;
    if(sampler8x8)
    {
        temp = static_cast< CmSampler8x8State_RT* >(sampler8x8);
    }
    else
    {
        return CM_FAILURE;
    }

    SamplerIndex* index  = nullptr;
    temp->GetIndex( index );
    CM_ASSERT( index );
    uint32_t indexValue = index->get_data();

    CM_ASSERT( m_sampler8x8Array.GetElement( indexValue ) == (temp) );

    int32_t status = CmSampler8x8State_RT::Destroy( temp );
    if(status == CM_SUCCESS)
    {
        UnregisterSampler8x8State( indexValue );
        m_sampler8x8Array.SetElement( indexValue, nullptr );
        sampler8x8 = nullptr;
    }

    return status;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler8x8 Surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::CreateSampler8x8Surface(CmSurface2D* surface2D,
                                        SurfaceIndex* & sampler8x8SurfIndex,
                                        CM_SAMPLER8x8_SURFACE sampler8x8Type,
                                        CM_SURFACE_ADDRESS_CONTROL_MODE mode)
{
    INSERT_API_CALL_LOG();
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t sizeperpixel = 0;
    uint32_t platform = 0;
    
    GetGenPlatform(platform);

    CmSurface2DRT* currentRT = static_cast<CmSurface2DRT *>(surface2D);
    if( ! currentRT )  {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_NULL_POINTER;
    }

    CM_SURFACE_FORMAT format;
    currentRT->GetSurfaceDesc(width, height, format, sizeperpixel);

    if (format == CM_SURFACE_FORMAT_NV12)
    {
        if (platform < IGFX_GEN10_CORE  &&
            ((width % 4) != 0 || (height % 4) != 0)) {  //width or height is not 4 aligned
            CM_ASSERTMESSAGE("Error: Width or height is not 4 aligned for nv12 surface.");
            return CM_SYSTEM_MEMORY_NOT_4PIXELS_ALIGNED;
        }
        else if ((width % 2) != 0 || (height % 2) != 0) {
            CM_ASSERTMESSAGE("Error: Width or height is not 2 aligned for nv12 surface.");
            return CM_SYSTEM_MEMORY_NOT_2PIXELS_ALIGNED;
        }
    }
    CLock locker(m_criticalSectionSurface);

    int32_t result = m_surfaceMgr->CreateSampler8x8Surface( currentRT, sampler8x8SurfIndex, sampler8x8Type, mode, nullptr );

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler8x8 Surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::CreateSampler8x8SurfaceEx(CmSurface2D* surface2d,
                                          SurfaceIndex* & sampler8x8SurfIndex,
                                          CM_SAMPLER8x8_SURFACE sampler8x8Type,
                                          CM_SURFACE_ADDRESS_CONTROL_MODE mode,
                                          CM_FLAG* flag)
{
    INSERT_API_CALL_LOG();

    CmSurface2DRT* currentRT = static_cast<CmSurface2DRT *>(surface2d);
    if (!currentRT)  {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_NULL_POINTER;
    }
    CLock locker(m_criticalSectionSurface);

    int32_t result = m_surfaceMgr->CreateSampler8x8Surface(currentRT, sampler8x8SurfIndex, sampler8x8Type, mode, flag);

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler Surface 2D with Flag
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::CreateSamplerSurface2DEx(CmSurface2D* surface2d,
                                         SurfaceIndex* & samplerSurfaceIndex,
                                         CM_FLAG* flag)
{
    INSERT_API_CALL_LOG();

    if (!surface2d) {
        CM_ASSERTMESSAGE("Error: Pointer to sampler surface 2D is null.");
        return CM_NULL_POINTER;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t sizeperpixel = 0;
    CM_SURFACE_FORMAT format = CM_SURFACE_FORMAT_INVALID;
    CmSurface2DRT* surface2dRT = static_cast<CmSurface2DRT *>(surface2d);
    surface2dRT->GetSurfaceDesc(width, height, format, sizeperpixel);
    if (!m_surfaceMgr->IsSupportedForSamplerSurface2D(format))
    {
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    CLock locker(m_criticalSectionSurface);

    int32_t result = m_surfaceMgr->CreateSamplerSurface(surface2dRT, samplerSurfaceIndex, flag);

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Sampler8x8 Surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::DestroySampler8x8Surface(SurfaceIndex* & surfaceIndex)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionSurface);

    int32_t result = m_surfaceMgr->DestroySampler8x8Surface( surfaceIndex );

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Coefficient Format Transform
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
#define FloatToS1_6(x)  (uint8_t)((char)(x * 64))
void
CmDeviceRTBase::Sampler8x8CoefficientFormatTransform(
                CM_AVS_INTERNEL_NONPIPLINED_STATE* dstAvsState,
                CM_AVS_NONPIPLINED_STATE* srcAvsState)
{
    int i;

    CmSafeMemSet( dstAvsState, 0, sizeof(CM_AVS_INTERNEL_NONPIPLINED_STATE));

    dstAvsState->BypassXAF = srcAvsState->BypassXAF;
    dstAvsState->BypassYAF = srcAvsState->BypassYAF;
    dstAvsState->DefaultSharpLvl = srcAvsState->DefaultSharpLvl;
    dstAvsState->bEnableRGBAdaptive = srcAvsState->bEnableRGBAdaptive;
    dstAvsState->bAdaptiveFilterAllChannels = srcAvsState->bAdaptiveFilterAllChannels;
    if (!srcAvsState->BypassXAF && !srcAvsState->BypassYAF) {
        dstAvsState->maxDerivative4Pixels = srcAvsState->maxDerivative4Pixels;
        dstAvsState->maxDerivative8Pixels = srcAvsState->maxDerivative8Pixels;
        dstAvsState->transitionArea4Pixels = srcAvsState->transitionArea4Pixels;
        dstAvsState->transitionArea8Pixels = srcAvsState->transitionArea8Pixels;
    }

    for (i = 0; i < CM_NUM_COEFF_ROWS_SKL; i++) {
      dstAvsState->Tbl0X[i].FilterCoeff_0_0 = FloatToS1_6(srcAvsState->Tbl0X[i].FilterCoeff_0_0);
      dstAvsState->Tbl0X[i].FilterCoeff_0_1 = FloatToS1_6(srcAvsState->Tbl0X[i].FilterCoeff_0_1);
      dstAvsState->Tbl0X[i].FilterCoeff_0_2 = FloatToS1_6(srcAvsState->Tbl0X[i].FilterCoeff_0_2);
      dstAvsState->Tbl0X[i].FilterCoeff_0_3 = FloatToS1_6(srcAvsState->Tbl0X[i].FilterCoeff_0_3);
      dstAvsState->Tbl0X[i].FilterCoeff_0_4 = FloatToS1_6(srcAvsState->Tbl0X[i].FilterCoeff_0_4);
      dstAvsState->Tbl0X[i].FilterCoeff_0_5 = FloatToS1_6(srcAvsState->Tbl0X[i].FilterCoeff_0_5);
      dstAvsState->Tbl0X[i].FilterCoeff_0_6 = FloatToS1_6(srcAvsState->Tbl0X[i].FilterCoeff_0_6);
      dstAvsState->Tbl0X[i].FilterCoeff_0_7 = FloatToS1_6(srcAvsState->Tbl0X[i].FilterCoeff_0_7);

      dstAvsState->Tbl0Y[i].FilterCoeff_0_0 = FloatToS1_6(srcAvsState->Tbl0Y[i].FilterCoeff_0_0);
      dstAvsState->Tbl0Y[i].FilterCoeff_0_1 = FloatToS1_6(srcAvsState->Tbl0Y[i].FilterCoeff_0_1);
      dstAvsState->Tbl0Y[i].FilterCoeff_0_2 = FloatToS1_6(srcAvsState->Tbl0Y[i].FilterCoeff_0_2);
      dstAvsState->Tbl0Y[i].FilterCoeff_0_3 = FloatToS1_6(srcAvsState->Tbl0Y[i].FilterCoeff_0_3);
      dstAvsState->Tbl0Y[i].FilterCoeff_0_4 = FloatToS1_6(srcAvsState->Tbl0Y[i].FilterCoeff_0_4);
      dstAvsState->Tbl0Y[i].FilterCoeff_0_5 = FloatToS1_6(srcAvsState->Tbl0Y[i].FilterCoeff_0_5);
      dstAvsState->Tbl0Y[i].FilterCoeff_0_6 = FloatToS1_6(srcAvsState->Tbl0Y[i].FilterCoeff_0_6);
      dstAvsState->Tbl0Y[i].FilterCoeff_0_7 = FloatToS1_6(srcAvsState->Tbl0Y[i].FilterCoeff_0_7);

      dstAvsState->Tbl1X[i].FilterCoeff_0_0 = FloatToS1_6(srcAvsState->Tbl1X[i].FilterCoeff_0_0);
      dstAvsState->Tbl1X[i].FilterCoeff_0_1 = FloatToS1_6(srcAvsState->Tbl1X[i].FilterCoeff_0_1);
      dstAvsState->Tbl1X[i].FilterCoeff_0_2 = FloatToS1_6(srcAvsState->Tbl1X[i].FilterCoeff_0_2);
      dstAvsState->Tbl1X[i].FilterCoeff_0_3 = FloatToS1_6(srcAvsState->Tbl1X[i].FilterCoeff_0_3);
      dstAvsState->Tbl1X[i].FilterCoeff_0_4 = FloatToS1_6(srcAvsState->Tbl1X[i].FilterCoeff_0_4);
      dstAvsState->Tbl1X[i].FilterCoeff_0_5 = FloatToS1_6(srcAvsState->Tbl1X[i].FilterCoeff_0_5);
      dstAvsState->Tbl1X[i].FilterCoeff_0_6 = FloatToS1_6(srcAvsState->Tbl1X[i].FilterCoeff_0_6);
      dstAvsState->Tbl1X[i].FilterCoeff_0_7 = FloatToS1_6(srcAvsState->Tbl1X[i].FilterCoeff_0_7);

      dstAvsState->Tbl1Y[i].FilterCoeff_0_0 = FloatToS1_6(srcAvsState->Tbl1Y[i].FilterCoeff_0_0);
      dstAvsState->Tbl1Y[i].FilterCoeff_0_1 = FloatToS1_6(srcAvsState->Tbl1Y[i].FilterCoeff_0_1);
      dstAvsState->Tbl1Y[i].FilterCoeff_0_2 = FloatToS1_6(srcAvsState->Tbl1Y[i].FilterCoeff_0_2);
      dstAvsState->Tbl1Y[i].FilterCoeff_0_3 = FloatToS1_6(srcAvsState->Tbl1Y[i].FilterCoeff_0_3);
      dstAvsState->Tbl1Y[i].FilterCoeff_0_4 = FloatToS1_6(srcAvsState->Tbl1Y[i].FilterCoeff_0_4);
      dstAvsState->Tbl1Y[i].FilterCoeff_0_5 = FloatToS1_6(srcAvsState->Tbl1Y[i].FilterCoeff_0_5);
      dstAvsState->Tbl1Y[i].FilterCoeff_0_6 = FloatToS1_6(srcAvsState->Tbl1Y[i].FilterCoeff_0_6);
      dstAvsState->Tbl1Y[i].FilterCoeff_0_7 = FloatToS1_6(srcAvsState->Tbl1Y[i].FilterCoeff_0_7);
    }

    return;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register Sampler8x8 State (Not implemented yet)
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::RegisterSampler8x8State(
                        const CM_SAMPLER_8X8_DESCR & sampler8x8State,
                        uint32_t& index)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    void  *dst = nullptr;
    void  *src = nullptr;

    CM_HAL_SAMPLER_8X8_PARAM     param;
    CM_AVS_STATE_MSG            *cmAvs;
    PMHW_SAMPLER_STATE_AVS_PARAM mhwAvs;

    CmSafeMemSet(&param, 0, sizeof(CM_HAL_SAMPLER_8X8_PARAM));

    index = 0;
    param.handle = 0;
    param.sampler8x8State.stateType = sampler8x8State.stateType;

    //Initialize the input parameters.
    switch(sampler8x8State.stateType)
    {
        case CM_SAMPLER8X8_AVS:
            mhwAvs = &(param.sampler8x8State.avsParam.avsState);
            cmAvs  = sampler8x8State.avs;
            mhwAvs->stateID              = (int16_t)-1;
            mhwAvs->bEnableAVS           = true;
            mhwAvs->AvsType              = cmAvs->AVSTYPE;
            mhwAvs->EightTapAFEnable     = cmAvs->EightTapAFEnable;
            mhwAvs->BypassIEF            = cmAvs->BypassIEF;
            mhwAvs->GainFactor           = cmAvs->GainFactor;
            mhwAvs->GlobalNoiseEstm      = cmAvs->GlobalNoiseEstm;
            mhwAvs->StrongEdgeThr        = cmAvs->StrongEdgeThr;
            mhwAvs->WeakEdgeThr          = cmAvs->WeakEdgeThr;
            mhwAvs->StrongEdgeWght       = cmAvs->StrongEdgeWght;
            mhwAvs->RegularWght          = cmAvs->RegularWght;
            mhwAvs->NonEdgeWght          = cmAvs->NonEdgeWght;

            mhwAvs->bEnableSTDE          = 0;
            mhwAvs->b8TapAdaptiveEnable  = 0;
            mhwAvs->bSkinDetailFactor    = 0;
            // current vphal/mhw use HDCDW flag to control shuffleoutputwriteback, we follow them
            mhwAvs->bHdcDwEnable         = ( cmAvs->HDCDirectWriteEnable || ( !cmAvs->ShuffleOutputWriteback ) );
            mhwAvs->bWritebackStandard = !cmAvs->ShuffleOutputWriteback;
            mhwAvs->bEnableIEF           = 0;
            mhwAvs->wIEFFactor           = 0;
            mhwAvs->wR3xCoefficient      = cmAvs->wR3xCoefficient;
            mhwAvs->wR3cCoefficient      = cmAvs->wR3cCoefficient;
            mhwAvs->wR5xCoefficient      = cmAvs->wR5xCoefficient;
            mhwAvs->wR5cxCoefficient     = cmAvs->wR5cxCoefficient;
            mhwAvs->wR5cCoefficient      = cmAvs->wR5cCoefficient;

            Sampler8x8CoefficientFormatTransform((PCM_AVS_INTERNEL_NONPIPLINED_STATE )&(param.sampler8x8State.avsParam.avsTable), sampler8x8State.avs->AvsState);
            break;

        case CM_SAMPLER8X8_CONV:
            dst = (void *)&(param.sampler8x8State.convolveState);
            src = (void *)sampler8x8State.conv;
            CmSafeMemCopy( dst, src, sizeof( CM_CONVOLVE_STATE_MSG));
            break;

        case CM_SAMPLER8X8_MISC:
            param.sampler8x8State.miscState.DW0.Height = sampler8x8State.misc->DW0.Height;
            param.sampler8x8State.miscState.DW0.Width = sampler8x8State.misc->DW0.Width;
            param.sampler8x8State.miscState.DW0.Row0 = sampler8x8State.misc->DW0.Row0;
            param.sampler8x8State.miscState.DW1.Row1 = sampler8x8State.misc->DW1.Row1;
            param.sampler8x8State.miscState.DW1.Row2 = sampler8x8State.misc->DW1.Row2;
            param.sampler8x8State.miscState.DW2.Row3 = sampler8x8State.misc->DW2.Row3;
            param.sampler8x8State.miscState.DW2.Row4 = sampler8x8State.misc->DW2.Row4;
            param.sampler8x8State.miscState.DW3.Row5 = sampler8x8State.misc->DW3.Row5;
            param.sampler8x8State.miscState.DW3.Row6 = sampler8x8State.misc->DW3.Row6;
            param.sampler8x8State.miscState.DW4.Row7 = sampler8x8State.misc->DW4.Row7;
            param.sampler8x8State.miscState.DW4.Row8 = sampler8x8State.misc->DW4.Row8;
            param.sampler8x8State.miscState.DW5.Row9 = sampler8x8State.misc->DW5.Row9;
            param.sampler8x8State.miscState.DW5.Row10 = sampler8x8State.misc->DW5.Row10;
            param.sampler8x8State.miscState.DW6.Row11 = sampler8x8State.misc->DW6.Row11;
            param.sampler8x8State.miscState.DW6.Row12 = sampler8x8State.misc->DW6.Row12;
            param.sampler8x8State.miscState.DW7.Row13 = sampler8x8State.misc->DW7.Row13;
            param.sampler8x8State.miscState.DW7.Row14 = sampler8x8State.misc->DW7.Row14;

            break;

        default:
            CM_ASSERTMESSAGE("Error: Invalid sampler8x8 state descr.");
            return hr;
    }

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)GetAccelData();

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnRegisterSampler8x8(cmData->cmHalState, &param));

    index = param.handle >> 16;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    UnRegister Sampler8x8 State (Not implemented yet)
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::UnregisterSampler8x8State(uint32_t index)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)GetAccelData();

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnRegisterSampler8x8(cmData->cmHalState, index));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//! Function to create a thread group space
//! Arguments:
//!        1. Width/height (in unit of thread ) of each thread group
//!        2. Width/height(in unit of group) of thread group space.
//!        3. Reference to the point to CmThreadGroupSpace object to created.
//! Return Value:
//!        CM_SUCCESS if the CmThreadGroupSpace is successfully created
//! Notes:
//!        The total thread count is width*height*grpWidth*grpHeight.
//!        The thread count will check against the thread count set by CmKernel::SetThreadCount if CmKernel::SetThreadCount is called.
//!        CmKernel::SetThreadCount needs to be called if CmKernel::SetThreadArg is to be called.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::CreateThreadGroupSpaceEx(uint32_t thrdSpaceWidth,
                                         uint32_t thrdSpaceHeight,
                                         uint32_t thrdSpaceDepth,
                                         uint32_t grpSpaceWidth,
                                         uint32_t grpSpaceHeight,
                                         uint32_t grpSpaceDepth,
                                         CmThreadGroupSpace*& threadGroupSpace)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionThreadGroupSpace);

    uint32_t firstfreeslot = m_threadGroupSpaceArray.GetFirstFreeIndex();
    CmDeviceRT *cmDevice = static_cast<CmDeviceRT*>(this);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    int32_t result = CmThreadGroupSpace::Create(cmDevice, firstfreeslot, thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth, threadGroupSpace);
    if (result == CM_SUCCESS)
    {
        m_threadGroupSpaceArray.SetElement( firstfreeslot, threadGroupSpace );
        m_threadGroupSpaceCount ++;
    }
    return result;
}

CM_RT_API int32_t
CmDeviceRTBase::CreateThreadGroupSpace(uint32_t thrdSpaceWidth,
                                       uint32_t thrdSpaceHeight,
                                       uint32_t grpSpaceWidth,
                                       uint32_t grpSpaceHeight,
                                       CmThreadGroupSpace*& threadGroupSpace)
{
    INSERT_API_CALL_LOG();

    int32_t result = CreateThreadGroupSpaceEx(thrdSpaceWidth,
                                              thrdSpaceHeight,
                                              1,
                                              grpSpaceWidth,
                                              grpSpaceHeight,
                                              1,
                                              threadGroupSpace);

    return result;
}

CM_RT_API int32_t
CmDeviceRTBase::DestroyThreadGroupSpace(CmThreadGroupSpace*& threadGroupSpace)
{
    INSERT_API_CALL_LOG();

    if( threadGroupSpace == nullptr )
    {
        return CM_FAILURE;
    }

    uint32_t indexTGs = threadGroupSpace->GetIndexInTGsArray();

    CLock locker(m_criticalSectionThreadGroupSpace);

    if(threadGroupSpace == static_cast< CmThreadGroupSpace* >(m_threadGroupSpaceArray.GetElement( indexTGs )))
    {
        int32_t status = CmThreadGroupSpace::Destroy( threadGroupSpace );
        if(status == CM_SUCCESS)
        {
            m_threadGroupSpaceArray.SetElement( indexTGs, nullptr );
            threadGroupSpace = nullptr;
            return CM_SUCCESS;
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to destroy thread group space.");
        return CM_FAILURE;
    }

    return CM_FAILURE;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Load Predefined Program, it is used by GPUCopy API
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::LoadPredefinedCopyKernel(CmProgram*& program)
{
    PCM_HAL_STATE           cmHalState;
    int32_t                 hr = CM_SUCCESS;

    cmHalState = ((PCM_CONTEXT_DATA)GetAccelData())->cmHalState;

    if(m_gpuCopyKernelProgram)
    {
        program = m_gpuCopyKernelProgram;
        return CM_SUCCESS;
    }

    void * gpucopyKernelIsa;
    uint32_t gpucopyKernelIsaSize;

    cmHalState->cmHalInterface->GetCopyKernelIsa(gpucopyKernelIsa, gpucopyKernelIsaSize);
    if (gpucopyKernelIsa == nullptr || gpucopyKernelIsaSize == 0)
    {
        return CM_NOT_IMPLEMENTED;
    }

    hr = LoadProgram((void *)gpucopyKernelIsa, gpucopyKernelIsaSize, program, "PredefinedGPUKernel");
    if (hr != CM_SUCCESS)
    {
        return hr;
    }

    m_gpuCopyKernelProgram = program;

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Load Predefined Program, it is used by GPUCopy API
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::LoadPredefinedInitKernel(CmProgram*& program)
{
    PCM_HAL_STATE           cmHalState;
    int32_t                 hr = CM_SUCCESS;

    cmHalState = ((PCM_CONTEXT_DATA)GetAccelData())->cmHalState;

    if(m_surfInitKernelProgram)
    {
        program = m_surfInitKernelProgram;
        return CM_SUCCESS;
    }

    void * gpuinitKernelIsa;
    uint32_t gpuinitKernelIsaSize;

    cmHalState->cmHalInterface->GetInitKernelIsa(gpuinitKernelIsa, gpuinitKernelIsaSize);
    if (gpuinitKernelIsa == nullptr || gpuinitKernelIsaSize == 0)
    {
        return CM_NOT_IMPLEMENTED;
    }

    hr = LoadProgram((void *)gpuinitKernelIsa, gpuinitKernelIsaSize, program, "PredefinedGPUKernel");
    if (hr != CM_SUCCESS)
    {
        return hr;
    }

    m_surfInitKernelProgram = program;

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Return HW stepping infor, Not implemented yet.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::GetGenStepInfo(char*& stepinfostr)
{
    PCM_HAL_STATE           cmHalState;
    int32_t                 hr = CM_SUCCESS;

    cmHalState = ((PCM_CONTEXT_DATA)GetAccelData())->cmHalState;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmHalState->cmHalInterface->GetGenStepInfo(stepinfostr));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler Surface 2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::CreateSamplerSurface2D(CmSurface2D* Surface2d,
                                       SurfaceIndex* & samplerSurfaceIndex)
{
    INSERT_API_CALL_LOG();

    if( ! Surface2d )  {
        CM_ASSERTMESSAGE("Error: Pointer to sampler surface 2D is null.");
        return CM_NULL_POINTER;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t sizeperpixel = 0;
    CM_SURFACE_FORMAT format = CM_SURFACE_FORMAT_INVALID;

    CmSurface2DRT* surface2dRT = static_cast<CmSurface2DRT *>(Surface2d);
    surface2dRT->GetSurfaceDesc(width, height, format, sizeperpixel);

    if (!m_surfaceMgr->IsSupportedForSamplerSurface2D(format))
    {
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    CLock locker(m_criticalSectionSurface);

    int32_t result = m_surfaceMgr->CreateSamplerSurface( surface2dRT, samplerSurfaceIndex, nullptr);

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler Surface 2D UP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::CreateSamplerSurface2DUP(CmSurface2DUP* surface2dUP,
                                         SurfaceIndex* & samplerSurfaceIndex)
{
    INSERT_API_CALL_LOG();

    if (!surface2dUP)
    {
        CM_ASSERTMESSAGE("Error: Pointer to sampler 2D UP is null.");
        return CM_NULL_POINTER;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t sizeperpixel = 0;
    CM_SURFACE_FORMAT format = CM_SURFACE_FORMAT_INVALID;
    CmSurface2DUPRT *surface2DRT = static_cast<CmSurface2DUPRT *>(surface2dUP);
    surface2DRT->GetSurfaceDesc(width, height, format, sizeperpixel);
    if (!m_surfaceMgr->IsSupportedForSamplerSurface2D(format))
    {
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    CLock locker(m_criticalSectionSurface);

    int32_t result = m_surfaceMgr->CreateSamplerSurface(surface2DRT, samplerSurfaceIndex);

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler Surface 3D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::CreateSamplerSurface3D(CmSurface3D* p3DSurface,
                                       SurfaceIndex* & samplerSurfaceIndex)
{
    INSERT_API_CALL_LOG();

    if( ! p3DSurface )  {
        CM_ASSERTMESSAGE("Error: Pointer to sampler surface 3D is null.");
        return CM_NULL_POINTER;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;
    CM_SURFACE_FORMAT  format = CM_SURFACE_FORMAT_INVALID;
    CmSurface3DRT *surfaceRT = static_cast<CmSurface3DRT *>(p3DSurface);
    surfaceRT->GetProperties(width, height, depth, format);
    switch(format)
    {
        case CM_SURFACE_FORMAT_A8R8G8B8:
        case CM_SURFACE_FORMAT_A16B16G16R16:
            break;

        default:
            CM_ASSERTMESSAGE("Error: Unsupported surface format.");
            return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    CLock locker(m_criticalSectionSurface);

    int32_t result = m_surfaceMgr->CreateSamplerSurface( surfaceRT, samplerSurfaceIndex);

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Sampler Surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRTBase::DestroySamplerSurface(SurfaceIndex* & samplerSurfaceIndex)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_criticalSectionSurface);

    int32_t result = m_surfaceMgr->DestroySamplerSurface( samplerSurfaceIndex );

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Cm Sampler8x8 pointer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::GetSampler8x8(uint32_t index,
                                      CmSampler8x8State_RT *&sampler8x8)
{
    if (CM_MAX_SAMPLER_TABLE_SIZE < index)
    {
        return CM_EXCEED_SAMPLER_AMOUNT;
    }

    sampler8x8 = (CmSampler8x8State_RT *)m_sampler8x8Array.GetElement(index);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set L3 config
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::SetL3Config(const L3ConfigRegisterValues *l3Config)
{
    INSERT_API_CALL_LOG();

    L3ConfigRegisterValues l3Values;

    l3Values = *l3Config;

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)this->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);
    if (cmData->cmHalState->advExecutor != nullptr)
    {
        cmData->cmHalState->advExecutor->SetL3Config(l3Config);
    }

    SetCaps(CAP_L3_CONFIG, sizeof(L3ConfigRegisterValues), &l3Values);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set L3 suggested config
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::SetSuggestedL3Config(L3_SUGGEST_CONFIG l3SuggestConfig)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr          = CM_SUCCESS;

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)this->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);
    if (cmData->cmHalState->advExecutor != nullptr)
    {
        CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->advExecutor->SetSuggestedL3Config(l3SuggestConfig));
    }
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->cmHalInterface->SetSuggestedL3Conf(l3SuggestConfig));

finish:
    return hr;
}

//!
//! \brief      This function can be used to set/limit hardware
//!             capabilities- number of threads that HW can run in parallel
//! \details    Hardware thread number can be set from 1 to maximum.
//! \param      [in] capName
//!             Name of cap to set.
//! \param      [in] capValueSize
//!             The size of the cap value.
//! \param      [in] capValue
//!             Pointer to the cap value.
//! \retval     CM_SUCCESS if cap value is valid and is set correctly.
//! \retval     CM_INVALID_HARDWARE_THREAD_NUMBER specific SetCaps error
//!             message if cap value is not valid.
//! \retval     CM_NOT_IMPLEMENTED for emulation mode.
//! \retval     CM_FAILURE otherwise.
//!
int32_t CmDeviceRTBase::SetCaps(CM_DEVICE_CAP_NAME capName,
                                size_t capValueSize,
                                void* capValue)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    CM_SET_CAPS setCaps;
    uint32_t maxValue;
    uint32_t size = sizeof(maxValue);
    CmSafeMemSet( &setCaps, 0, sizeof( setCaps ) );

    switch(capName)
    {
        case CAP_HW_THREAD_COUNT:
            if (capValueSize != sizeof(uint32_t))
            {
                CM_ASSERTMESSAGE("Error: Failed to set caps with CAP_HW_THREAD_COUNT.");
                return CM_INVALID_HARDWARE_THREAD_NUMBER;
            }

            if( *(int32_t *)capValue <= 0 )
            {
                CM_ASSERTMESSAGE("Error: Failed to set caps with CAP_HW_THREAD_COUNT.");
                return CM_INVALID_HARDWARE_THREAD_NUMBER;
            }

            GetCaps(CAP_HW_THREAD_COUNT, size, &maxValue);
            if ( *(uint32_t *)capValue > maxValue)
            {
                CM_ASSERTMESSAGE("Error: Failed to set caps with CAP_HW_THREAD_COUNT.");
                return CM_INVALID_HARDWARE_THREAD_NUMBER;
            }

            setCaps.type = CM_SET_MAX_HW_THREADS;
            setCaps.maxValue = *(uint32_t *)capValue;
            break;

        case CAP_L3_CONFIG:
            if (capValueSize != sizeof(L3ConfigRegisterValues))
            {
                CM_ASSERTMESSAGE("Error: Failed to set caps with CAP_L3_CONFIG.");
                return CM_INVALIDE_L3_CONFIGURATION;
            }
            else
            {
                L3ConfigRegisterValues *l3Config = (L3ConfigRegisterValues *)capValue;

                setCaps.configRegsiter0 = l3Config->config_register0;
                setCaps.configRegsiter1 = l3Config->config_register1;
                setCaps.configRegsiter2 = l3Config->config_register2;
                setCaps.configRegsiter3 = l3Config->config_register3;
                setCaps.type = CM_SET_HW_L3_CONFIG;
            }
            break;

        default:
            CM_ASSERTMESSAGE("Error: Invalid cap name.");
            return CM_INVALID_CAP_NAME;
    }

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)this->GetAccelData();
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnSetCaps(cmData->cmHalState, (PCM_HAL_MAX_SET_CAPS_PARAM)&setCaps));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create print buffer to support print in cm kernel
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::InitPrintBuffer(size_t printbufsize)
{
    INSERT_API_CALL_LOG();

    m_printBufferSize = printbufsize;
    m_isPrintEnabled = true;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create a new static buffer for kernel print
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::CreatePrintBuffer()
{
    uint8_t *mem = (uint8_t*)MOS_AlignedAllocMemory(m_printBufferSize, 0x1000); //PAGE SIZE
    if(!mem)
    {
        return CM_OUT_OF_HOST_MEMORY;
    }

    CmSafeMemSet(mem, 0, m_printBufferSize);
    *(unsigned int*)mem = PRINT_BUFFER_HEADER_SIZE;

    /// Allocate device memory and MemCopy from host to device.
    CmBufferUP *buffer = nullptr;
    int32_t result = CreateBufferUP((uint32_t)m_printBufferSize, mem, buffer);
    if (result != CM_SUCCESS || buffer == nullptr)
    {
        m_isPrintEnabled = false;
        MOS_AlignedFreeMemory(mem);
        return result;
    }
    m_printBufferMems.push_back(mem);
    m_printBufferUPs.push_back(buffer);
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get print buffer memory
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::GetPrintBufferMem(unsigned char * &printBufferMem) const
{
    printBufferMem = m_printBufferMems.back();
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the print buffer's surface index
//| Returns:    The print buffer's surface index
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::GetPrintBufferIndex(SurfaceIndex *& index) const
{
    m_printBufferUPs.back()->GetIndex(index);
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether the kernel print is enabled
//| Returns:    Whether the kernel print is enabled.
//*-----------------------------------------------------------------------------
bool CmDeviceRTBase::IsPrintEnable() const
{
     return m_isPrintEnabled;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether MDF ETW Log On.
//| Returns:    Whether MDF ETW Log On.
//*-----------------------------------------------------------------------------
bool CmDeviceRTBase::IsVtuneLogOn() const
{
     return m_vtuneOn;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Surf2D LookUP Entry
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::GetSurf2DLookUpEntry(uint32_t index,
                                             PCMLOOKUP_ENTRY &lookupEntry)
{
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)GetAccelData();
    if(cmData)
    {
        lookupEntry = &(cmData->cmHalState->surf2DTable[index]);
    }
    else
    {
        return CM_FAILURE;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create vebox task
//| Returns:    CM_SUCCESS if successfully.
//|             CM_OUT_OF_HOST_MEMORY if creation is failed.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::CreateVebox(CmVebox* & vebox) //HSW
{
    CLock locker(m_criticalSectionVebox);

    uint32_t firstfreeslot = m_veboxArray.GetFirstFreeIndex();
    CmVeboxRT *veboxRT = nullptr;
    CmDeviceRT *cmDevice = static_cast<CmDeviceRT*>(this);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    int32_t result = CmVeboxRT::Create(cmDevice, firstfreeslot, veboxRT);
    if (result == CM_SUCCESS)
    {
        m_veboxArray.SetElement(firstfreeslot, veboxRT);
        m_veboxCount++;
    }
    vebox = veboxRT;
    return result;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy vebox task
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::DestroyVebox(CmVebox* & vebox) //HSW
{
    if (vebox == nullptr)
    {
        return CM_NULL_POINTER;
    }

    CmVeboxRT *veboxRT = static_cast<CmVeboxRT *>(vebox);
    uint32_t index = veboxRT->GetIndexInVeboxArray();

    if (veboxRT == m_veboxArray.GetElement(index))
    {
        int32_t status = CmVeboxRT::Destroy(veboxRT);
        if (status == CM_SUCCESS)
        {
            m_veboxArray.SetElement(index, nullptr);
            vebox = nullptr;
            return CM_SUCCESS;
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Failed to destroy vebox task.");
            return status;
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to destroy vebox task.");
        return CM_FAILURE;
    }
}


int32_t CmDeviceRTBase::DestroySurfaceInPool(uint32_t &freeSurfNum)
{
    CLock locker(m_criticalSectionSurface);

    freeSurfNum = m_surfaceMgr->TouchSurfaceInPoolForDestroy();
    if ((int32_t)freeSurfNum < 0)
    {
        freeSurfNum = 0;
        return CM_FAILURE;
    }

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDeviceRTBase::CreateBufferSVM(uint32_t size,
                                                  void* & sysMem,
                                                  uint32_t accessFlag,
                                                  CmBufferSVM* & bufferSVM)
{
    INSERT_API_CALL_LOG();

    bool    isCMRTAllocatedSVMBuffer = true;

    //SVM buffer is going to stateless access, no size restriction lik CmBuffer and CmBufferUP
    if( size == 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid buffer width.");
        return CM_INVALID_WIDTH;
    }

    if ( sysMem )
    {
        if ((uintptr_t)sysMem & CM_PAGE_ALIGNMENT_MASK)
        {
            CM_ASSERTMESSAGE("Error: System memory is not page aligned.");
            return CM_SYSTEM_MEMORY_NOT_4KPAGE_ALIGNED;
        }
        isCMRTAllocatedSVMBuffer = false;
    }
    else //Allocate a 4K page aligned memory
    {
        sysMem = MOS_AlignedAllocMemory(size, CM_PAGE_ALIGNMENT);

        if (!sysMem)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_FAILED_TO_ALLOCATE_SVM_BUFFER;
        }
        isCMRTAllocatedSVMBuffer = true;
    }

    CLock locker(m_criticalSectionSurface);

    CmBuffer_RT* p = nullptr;
    int result = m_surfaceMgr->CreateBuffer( size, CM_BUFFER_SVM, isCMRTAllocatedSVMBuffer, p, nullptr, sysMem, false, CM_DEFAULT_COMPARISON_VALUE );
    bufferSVM = static_cast< CmBufferSVM* >(p);

    return result;
}

CM_RT_API int32_t CmDeviceRTBase::DestroyBufferSVM(CmBufferSVM* & bufferSVM)
{

    INSERT_API_CALL_LOG();

    CmBuffer_RT* temp = static_cast< CmBuffer_RT* >(bufferSVM);
    if (nullptr == temp)
    {
        return CM_NULL_POINTER;
    }

    CLock locker(m_criticalSectionSurface);

    int32_t status = m_surfaceMgr->DestroySurface(temp, APP_DESTROY);

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE, or  CM_SURFACE_CACHED may be returned, which should be treated as SUCCESS.
    {
        bufferSVM = nullptr;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Creates an alias to CmSurface2D, surface2d
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::CreateSurface2DAlias(CmSurface2D* surface2d,
                                                       SurfaceIndex* &aliasIndex)
{
    INSERT_API_CALL_LOG();

    int32_t result = CM_SUCCESS;

    CLock locker(m_criticalSectionSurface);
    if( !surface2d )
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface 2D is null.");
        return CM_NULL_POINTER;
    }

    CmSurface2DRT *surfaceRT = static_cast<CmSurface2DRT *>(surface2d);
    result = surfaceRT->Create2DAlias(aliasIndex);
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Failed to create surface 2D alias.");
        return result;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Creates an alias to CmBuffer, buffer
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::CreateBufferAlias(CmBuffer *buffer,
                                                    SurfaceIndex* &aliasIndex)
{
    INSERT_API_CALL_LOG();

    int32_t result = CM_SUCCESS;

    CLock locker(m_criticalSectionSurface);
    if( !buffer )
    {
        CM_ASSERTMESSAGE("Error: Pointer to CmBuffer is null.");
        return CM_NULL_POINTER;
    }

    CmBuffer_RT *bufferRT = static_cast<CmBuffer_RT *>(buffer);

    result = bufferRT->CreateBufferAlias(aliasIndex);
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Failed to create buffer alias.");
        return result;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Dev Create Option
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::InitDevCreateOption(CM_HAL_CREATE_PARAM & cmHalCreateParam,
                                            uint32_t option)
{
    uint32_t maxTaskNumber =0;
    uint32_t kernelBinarySizeInGSH = 0;

    //Flag to disable scratch space
    cmHalCreateParam.disableScratchSpace = (option & CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_MASK);

    //Calculate Scratch Space
    if(cmHalCreateParam.disableScratchSpace)
    {
        cmHalCreateParam.scratchSpaceSize = 0;
    }
    else
    {
        //Max Scratch Space Size [1:3] of devCreateOption
        cmHalCreateParam.scratchSpaceSize = (option & CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_MASK) >> CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_OFFSET;
    }

    //Flag to Disable preemption
    cmHalCreateParam.disabledMidThreadPreemption = ((option  & CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_DISENABLE) >> CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_OFFSET)? true: false;

    //flag to enable kernel debug, so, SIP binary can be created during
    cmHalCreateParam.enabledKernelDebug = ((option  & CM_DEVICE_CONFIG_KERNEL_DEBUG_ENABLE) >> CM_DEVICE_CONFIG_KERNEL_DEBUG_OFFSET)? true: false;

    //Calculate Task Number [4:5] of option   [00]:4 ; [01]:8 ; [10]:12; [11]:16
    maxTaskNumber = (option & CM_DEVICE_CONFIG_TASK_NUM_MASK) >> CM_DEVICE_CONFIG_TASK_NUM_OFFSET;

    cmHalCreateParam.maxTaskNumber = (maxTaskNumber + 1) * CM_DEVICE_CONFIG_TASK_NUM_STEP;

    // [9:8] Added bits to increase maximum task number. Value plus one is multiplied by value calculated from bits [5:4].
    // [00]:1; [01]:2; [10]:3; [11]:4
    maxTaskNumber = (option & CM_DEVICE_CONFIG_EXTRA_TASK_NUM_MASK ) >> CM_DEVICE_CONFIG_EXTRA_TASK_NUM_OFFSET;

    cmHalCreateParam.maxTaskNumber = (maxTaskNumber + 1) * cmHalCreateParam.maxTaskNumber;

    // [10] request slice shutdown
    cmHalCreateParam.requestSliceShutdown = (option & CM_DEVICE_CONFIG_SLICESHUTDOWN_ENABLE ) ? true:false;

    // [12] request custom gpu context. This flag is deprecated since GPU context is decoupled with cmhal for supporting multiple context.
    cmHalCreateParam.requestCustomGpuContext = (option & CM_DEVICE_CONFIG_GPUCONTEXT_ENABLE) ? true:false;

    // [20:13] calculate size in GSH reserved for kernel binary
    kernelBinarySizeInGSH = (option & CM_DEVICE_CONFIG_KERNELBINARYGSH_MASK) >> CM_DEVICE_CONFIG_KERNELBINARYGSH_OFFSET;

    if (kernelBinarySizeInGSH == 0)
        kernelBinarySizeInGSH = 1;

    kernelBinarySizeInGSH = kernelBinarySizeInGSH * CM_KERNELBINARY_BLOCKSIZE_2MB;
    cmHalCreateParam.kernelBinarySizeinGSH = kernelBinarySizeInGSH;

    // [28] vebox
    cmHalCreateParam.disableVebox = (option & CM_DEVICE_CONFIG_VEBOX_DISABLE) ? true : false;

    // [29] preload kernel
    m_preloadKernelEnabled = (option & CM_DEVICE_CONFIG_GPUCOPY_DISABLE) ? false : true;

    // [30] fast path
    cmHalCreateParam.refactor = (option & CM_DEVICE_CONFIG_FAST_PATH_ENABLE)?true:false;
    return CM_SUCCESS;
}

bool CmDeviceRTBase::IsScratchSpaceDisabled()
{
    return m_cmHalCreateOption.disableScratchSpace ? true : false;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets surface array size, needed to assign alias surface index
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::SetSurfaceArraySizeForAlias()
{
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)this->GetAccelData();
    m_surfaceMgr->GetSurfaceArraySize(cmData->cmHalState->surfaceArraySize);
    return CM_SUCCESS;
}

#if CM_LOG_ON
std::string CmDeviceRTBase::Log()
{
    std::ostringstream  oss;
    PCM_HAL_STATE       cmHalState;
    uint64_t            timeStampBase = 0;
    uint32_t            nSize = sizeof(int);

    GetCaps( CAP_GPU_CURRENT_FREQUENCY, nSize, &m_nGPUFreqOriginal );
    GetCaps( CAP_MIN_FREQUENCY, nSize, &m_nGPUFreqMin );
    GetCaps( CAP_MAX_FREQUENCY, nSize, &m_nGPUFreqMax );

    int gtInfo;
    GetCaps( CAP_GT_PLATFORM,   nSize, &gtInfo        );

    cmHalState  = ((PCM_CONTEXT_DATA)GetAccelData())->cmHalState; 
    CM_CHK_NULL_RETURN(cmHalState,"cmHalState is null pointer");
    timeStampBase = HalCm_ConvertTicksToNanoSeconds(cmHalState,1);

    oss << "Device Creation "<<std::endl;
    // Hw Information
    oss << "Platform :" << m_platform << std::endl;
    oss << "GT Info :"<< gtInfo << std::endl;
    oss << "Frequency Max:" << m_nGPUFreqMax << " Min:" <<m_nGPUFreqMin
        << " Current:"<< m_nGPUFreqOriginal << std::endl;

    oss << "Device DDI Version :" << m_ddiVersion << std::endl;
    oss << "Max Tasks " << m_halMaxValues.maxTasks << std::endl;
    oss << "Max HW Threads " << m_halMaxValues.maxHwThreads<< std::endl;
    oss << "Max Args Per Kernel   " << m_halMaxValues.maxArgsPerKernel << std::endl;
    oss << "Max 2D Surface Table Size " << m_halMaxValues.max2DSurfaceTableSize << std::endl;
    oss << "Max Buffer Table Size " << m_halMaxValues.maxBufferTableSize << std::endl;
    oss << "Max Threads per Task  " << m_halMaxValues.maxUserThreadsPerTask << std::endl;
    oss << "Max Threads Per Task no Thread Arg " << m_halMaxValues.maxUserThreadsPerTaskNoThreadArg << std::endl;
    oss << "MDF timestamp base " << timeStampBase << "ns" << std::endl;

    return oss.str();
}
#endif

#if !(USE_EXTENSION_CODE)
bool CmDeviceRTBase::CheckGTPinEnabled( )
{
    return false;
}
#endif

//*-----------------------------------------------------------------------------
//| Purpose:    Internal function to flush print buffer on stdout or file.
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRTBase::FlushPrintBufferInternal(const char *filename)
{
#if CM_KERNEL_PRINTF_ON
    FILE * streamOutFile = nullptr;

    if (filename == nullptr)
    {
        streamOutFile = stdout;
    }
    else
    {
        int err = MOS_SecureFileOpen(&streamOutFile, filename, "wb");
        if (streamOutFile == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Failed to open kernel print dump file.");
            return CM_FAILURE;
        }
        if (err)
        {
            fclose(streamOutFile);
            streamOutFile = nullptr;
            CM_ASSERTMESSAGE("Error: Failed to open kernel print dump file.");
            return CM_FAILURE;
        }
    }

    if( m_printBufferSize == 0 ||
        m_isPrintEnabled == false)
    {
        CM_ASSERTMESSAGE("Error: Print buffer is not initialized.");
        if (filename && streamOutFile)
            fclose(streamOutFile);
        return CM_FAILURE;
    }

    //Dump memory on the screen.
    while(!m_printBufferMems.empty())
    {
        uint8_t *mem = m_printBufferMems.front();
        CmBufferUP *buffer = m_printBufferUPs.front();
        DumpAllThreadOutput(streamOutFile, mem, m_printBufferSize);
        m_printBufferMems.pop_front();
        m_printBufferUPs.pop_front();
        DestroyBufferUP(buffer);
        MOS_AlignedFreeMemory(mem);
    }

    //Flush and close stream
    fflush(streamOutFile);
    if (filename && streamOutFile)
    {
        fclose(streamOutFile);
        streamOutFile = nullptr;
    }

    return CM_SUCCESS;
#else
    return CM_NOT_IMPLEMENTED;
#endif
}

//*-----------------------------------------------------------------------------
//| Purpose:    Dump print buffer. Only Avaliable in Release-internal and Debug Mode
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::FlushPrintBuffer()
{
    return FlushPrintBufferInternal(nullptr);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Dump print buffer to file. Only Avaliable in Release-internal and Debug Mode
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRTBase::FlushPrintBufferIntoFile(const char *filename)
{
    return FlushPrintBufferInternal(filename);
}

CM_RT_API int32_t
CmDeviceRTBase::CreateHevcVmeSurfaceG10(CmSurface2D * curSurface,
                                        CmSurface2D ** forwardSurfaces,
                                        CmSurface2D ** backwardSurfaces,
                                        const uint32_t forwardSurfaceCount,
                                        const uint32_t backwardSurfaceCount,
                                        SurfaceIndex *& vmeIndex)
{
    INSERT_API_CALL_LOG();

    if ( curSurface == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_NULL_POINTER;
    }

    if (forwardSurfaceCount > CM_NUM_VME_HEVC_REFS || backwardSurfaceCount > CM_NUM_VME_HEVC_REFS)
    {
        CM_ASSERTMESSAGE("Error: Invalid count of forward or backward surfaces.");
        return CM_INVALID_ARG_VALUE;
    }

    CmSurface2DRT *currentRT = static_cast< CmSurface2DRT * >( curSurface );
    CmSurface2DRT** forwardSurfArray = nullptr;
    CmSurface2DRT** backwardSurfArray = nullptr;

    forwardSurfArray = MOS_NewArray(CmSurface2DRT*, CM_NUM_VME_HEVC_REFS);
    if ( forwardSurfArray == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Failed to create forward surface array.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    if ( forwardSurfaces != nullptr )
    {
        for ( uint32_t i = 0; i< forwardSurfaceCount; i++ )
        {
            forwardSurfArray[ i ] = static_cast< CmSurface2DRT * >( forwardSurfaces[ i ] );
            if ( forwardSurfArray[ i ] == nullptr )
            {
                CM_ASSERTMESSAGE("Error: Invalid forward surface array.");
                MosSafeDeleteArray( forwardSurfArray );
                return CM_INVALID_ARG_VALUE;
            }
        }
        for ( uint32_t i = forwardSurfaceCount; i < CM_NUM_VME_HEVC_REFS; i++ )
        {
            forwardSurfArray[ i ] = static_cast< CmSurface2DRT * >( forwardSurfaces[ 0 ] );
        }
    }
    else
    {
        for ( uint32_t i = 0; i < CM_NUM_VME_HEVC_REFS; i++ )
        {
            forwardSurfArray[ i ] = currentRT;
        }
    }

    backwardSurfArray = MOS_NewArray(CmSurface2DRT*, CM_NUM_VME_HEVC_REFS);
    if ( backwardSurfArray == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Failed to create backward surface array.");
        MosSafeDeleteArray( forwardSurfArray );
        return CM_OUT_OF_HOST_MEMORY;
    }
    if ( backwardSurfaces != nullptr )
    {
        for ( uint32_t i = 0; i< backwardSurfaceCount; i++ )
        {
            backwardSurfArray[ i ] = static_cast< CmSurface2DRT * >( backwardSurfaces[ i ] );
            if ( backwardSurfArray[ i ] == nullptr )
            {
                CM_ASSERTMESSAGE("Error: Invalid backward surface array.");
                MosSafeDeleteArray( forwardSurfArray );
                MosSafeDeleteArray( backwardSurfArray );
                return CM_INVALID_ARG_VALUE;
            }
        }
        for ( uint32_t i = backwardSurfaceCount; i < CM_NUM_VME_HEVC_REFS; i++ )
        {
            backwardSurfArray[ i ] = static_cast< CmSurface2DRT * >( backwardSurfaces[ 0 ] );
        }
    }
    else
    {
        for ( uint32_t i = 0; i < CM_NUM_VME_HEVC_REFS; i++ )
        {
            backwardSurfArray[ i ] = currentRT;
        }
    }

    int32_t result = m_surfaceMgr->CreateVmeSurface( currentRT, forwardSurfArray, backwardSurfArray, forwardSurfaceCount, backwardSurfaceCount, vmeIndex );

    if ( FAILED( result ) )
    {
        CM_ASSERTMESSAGE("Error: Failed to create HEVC VME surface.");
    }

    MosSafeDeleteArray( forwardSurfArray );
    MosSafeDeleteArray( backwardSurfArray );

    return result;
}

CM_RT_API int32_t
CmDeviceRTBase::DestroyHevcVmeSurfaceG10(SurfaceIndex *& vmeIndex)
{
    INSERT_API_CALL_LOG();
    return DestroyVmeSurface( vmeIndex );
}

CM_RT_API int32_t CmDeviceRTBase::CloneKernel(CmKernel* &kernelDest,
                                              CmKernel *kernelSrc)
{
    INSERT_API_CALL_LOG();

    int32_t hr = CM_SUCCESS;

    if (kernelSrc == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to src kernel is null");
        return CM_NULL_POINTER;
    }

    CmKernelRT *kernelSrcRT = static_cast<CmKernelRT *>(kernelSrc);
    CmKernelRT *kernelDestRT = static_cast<CmKernelRT *>(kernelDest);
    hr = kernelSrcRT->CloneKernel(kernelDestRT, m_kernelCount);
    kernelDest = kernelDestRT;

    return hr;
}

CmDynamicArray* CmDeviceRTBase::GetKernelArray()
{
    return &m_kernelArray;
}

uint32_t *CmDeviceRTBase::GetKernelCount()
{
    return &m_kernelCount;
}

int32_t CmDeviceRTBase::DestroyVmeSurface(SurfaceIndex *& vmeIndex)
{
    CLock locker( m_criticalSectionSurface );

    int32_t result = m_surfaceMgr->DestroyVmeSurface( vmeIndex );

    return result;
}

int32_t CmDeviceRTBase::GetVISAVersion(uint32_t& majorVersion,
                                       uint32_t& minorVersion)
{
    int32_t result = CM_SUCCESS;

    result = GetJITVersionFnt(m_fJITVersion);
    if(result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Failed to get VISA version.");
        return result;
    }

    m_fJITVersion(majorVersion, minorVersion);

    return CM_SUCCESS;
}


CM_RT_API int32_t CmDeviceRTBase::UpdateBuffer(PMOS_RESOURCE mosResource, CmBuffer* &surface,
                                               MOS_HW_RESOURCE_DEF mosUsage)
{
    int32_t hr = CM_SUCCESS;
    if (surface)
    {
        CmBuffer_RT *bufferRT = static_cast<CmBuffer_RT *>(surface);
        hr = bufferRT->UpdateResource(mosResource);
    }
    else
    {
        hr = CreateBuffer(mosResource, surface);
    }

    if (hr == CM_SUCCESS)
    {
        hr = surface->SetResourceUsage(mosUsage);
    }
    return hr;
}


CM_RT_API int32_t CmDeviceRTBase::UpdateSurface2D(PMOS_RESOURCE mosResource, CmSurface2D* &surface,
                                                  MOS_HW_RESOURCE_DEF mosUsage)
{
    int32_t hr = CM_SUCCESS;
    if (surface)
    {
        CmSurface2DRT *surfaceRT = static_cast<CmSurface2DRT *>(surface);
        hr = surfaceRT->UpdateResource(mosResource);
    }
    else
    {
        hr = CreateSurface2D(mosResource, surface);
    }

    if (hr == CM_SUCCESS)
    {
        hr = surface->SetResourceUsage(mosUsage);
    }
    return hr;
}

CM_RT_API int32_t CmDeviceRTBase::CreateSampler8x8SurfaceFromAlias(
    CmSurface2D *originalSurface,
    SurfaceIndex *aliasIndex,
    CM_SURFACE_ADDRESS_CONTROL_MODE addressControl,
    SurfaceIndex* &sampler8x8SurfaceIndex)
{
    INSERT_API_CALL_LOG();
    CmSurface2DRT *original_surface_rt = static_cast<CmSurface2DRT*>(originalSurface);
    if (!original_surface_rt)  {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_NULL_POINTER;
    }

    uint32_t width = 0, height = 0, size_per_pixel = 0;
    CM_SURFACE_FORMAT format;
    original_surface_rt->GetSurfaceDesc(width, height, format, size_per_pixel);

    CLock locker(m_criticalSectionSurface);

    return m_surfaceMgr
            ->CreateSampler8x8SurfaceFromAlias(original_surface_rt,
                                               aliasIndex,
                                               addressControl,
                                               sampler8x8SurfaceIndex);
}

CM_RT_API int32_t CmDeviceRTBase::CreateBufferStateless(size_t size,
                                                        uint32_t option,
                                                        void *sysMem,
                                                        CmBufferStateless *&bufferStateless)
{
    INSERT_API_CALL_LOG();

    int result = CM_SUCCESS;

    // Stateless buffer is going to stateless access, no size restriction like
    // CmBuffer and CmBufferUP. But current supported size is less than 4G
    // because of GMM limitation.
    if (size == 0)
    {
        CM_ASSERTMESSAGE("Error: Invalid buffer width.");
        return CM_INVALID_WIDTH;
    }

    if (option == CM_BUFFER_STATELESS_CREATE_OPTION_SYS_MEM)
    {
        CM_ASSERTMESSAGE("Error: Stateless buffer created from system memory is not supported.");
        return CM_NOT_IMPLEMENTED;
    }
    else if (option == CM_BUFFER_STATELESS_CREATE_OPTION_GFX_MEM)
    {
        CLock locker(m_criticalSectionSurface);

        CmBuffer_RT *p  = nullptr;
        void *sysMemory = nullptr;
        result = m_surfaceMgr->CreateBuffer(size,
                                            CM_BUFFER_STATELESS,
                                            false,
                                            p,
                                            nullptr,
                                            sysMemory,
                                            false,
                                            CM_DEFAULT_COMPARISON_VALUE);
        bufferStateless = static_cast<CmBufferStateless *>(p);
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Invalid option value.");
        return CM_INVALID_CREATE_OPTION_FOR_BUFFER_STATELESS;
    }

    return result;
}

CM_RT_API int32_t CmDeviceRTBase::DestroyBufferStateless(CmBufferStateless *&bufferStateless)
{
    INSERT_API_CALL_LOG();

    CmBuffer_RT *temp = static_cast<CmBuffer_RT *>(bufferStateless);
    if (nullptr == temp)
    {
        return CM_NULL_POINTER;
    }

    CLock locker(m_criticalSectionSurface);

    int32_t status = m_surfaceMgr->DestroySurface(temp, APP_DESTROY);

    if (status != CM_FAILURE)  //CM_SURFACE_IN_USE, or  CM_SURFACE_CACHED may be returned, which should be treated as SUCCESS.
    {
        bufferStateless = nullptr;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}
}  // namespace
