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

struct CM_SET_CAPS
{
    CM_SET_TYPE Type;
    union
    {
        uint32_t MaxValue;
        struct
        {
            uint32_t ConfigRegsiter0;
            uint32_t ConfigRegsiter1;
            uint32_t ConfigRegsiter2;
            uint32_t ConfigRegsiter3;
        };
    };
};

namespace CMRT_UMD
{
CSync CmDeviceRT::GlobalCriticalSection_Surf2DUserDataLock = CSync();

//*-----------------------------------------------------------------------------
//| Purpose:    Create Cm Device
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::Create(MOS_CONTEXT *pUmdContext,
                           CmDeviceRT* &pDevice,
                           uint32_t DevCreateOption)
{
    int32_t result = CM_FAILURE;

    if(pDevice != nullptr )
    {
        // if the Cm Device exists
        pDevice->Acquire();
        return CM_SUCCESS;
    }

    pDevice = new (std::nothrow) CmDeviceRT( DevCreateOption );
    if( pDevice )
    {
        pDevice->Acquire(); // increase ref count
        result = pDevice->Initialize( pUmdContext );
        if( result != CM_SUCCESS )
        {
            CM_ASSERTMESSAGE("Error: Failed to initialzie CmDevice.");
            CmDeviceRT::Destroy( pDevice);
            pDevice = nullptr;
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmDevice due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Cm Device Acquire: Increae the m_CmDeviceRefCount
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::Acquire()
{
    // Enter critical section
    CLock locker(m_CriticalSection_DeviceRefCount);

    m_CmDeviceRefCount ++;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Cm Device Relase: Decrease the m_CmDeviceRefCount
//| Returns:    Reference count of Cm Device
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::Release()
{
    // Enter critical section
    CLock locker(m_CriticalSection_DeviceRefCount);

    m_CmDeviceRefCount --;

    return m_CmDeviceRefCount;
}

//*-----------------------------------------------------------------------------
//! Destroy the CmDevice_RT and kernels, samplers and the queue it created.
//! Also destroy all surfaces it created if the surface hasn't been explicitly destroyed.
//! Input :
//!     Reference to the pointer to the CmDevice_RT .
//! OUTPUT :
//!     CM_SUCCESS if CmDevice_RT is successfully destroyed.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::Destroy(CmDeviceRT* &pDevice)
{
    INSERT_API_CALL_LOG();

    int32_t result = CM_SUCCESS;

    int32_t refCount = pDevice->Release();

    if(refCount == 0)
    {
        CmSafeDelete(pDevice);
    }

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of CmDevice
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmDeviceRT::CmDeviceRT(uint32_t DevCreateOption):
    m_pUmdContext (nullptr),
    m_pAccelData (nullptr),
    m_AccelSize  (0),
    m_pSurfaceMgr( nullptr ),
    m_ProgramArray( CM_INIT_PROGRAM_COUNT ),
    m_ProgramCount( 0 ),
    m_KernelArray( CM_INIT_KERNEL_COUNT ),
    m_KernelCount( 0 ),
    m_SamplerArray( CM_INIT_SAMPLER_COUNT ),
    m_Sampler8x8Array( CM_INIT_SAMPLER_COUNT ),
    m_ThreadSpaceArray( CM_INIT_THREADSPACE_COUNT ),
    m_ThreadSpaceCount( 0 ),
    m_hJITDll(nullptr),
    m_fJITCompile(nullptr),
    m_fFreeBlock(nullptr),
    m_fJITVersion(nullptr),
    m_DDIVersion( 0 ),
    m_Platform(IGFX_UNKNOWN_CORE),
    m_CmDeviceRefCount(0),
    m_pGPUCopyKernelProgram(nullptr),
    m_pSurfInitKernelProgram(nullptr),
    m_OSSyncEvent (0),
#if USE_EXTENSION_CODE
    m_pGTPin(nullptr),
#endif
    m_IsPrintEnable(false),
    m_pPrintBufferMem (nullptr),
    m_pPrintBufferUP(nullptr),
    m_PrintBufferSize(0),
    m_PrintBufferIndex(nullptr),
    m_ThreadGroupSpaceArray(CM_INIT_THREADGROUPSPACE_COUNT),
    m_ThreadGroupSpaceCount(0),
    m_TaskArray(CM_INIT_TASK_COUNT),
    m_TaskCount(0),
    m_VeboxArray(CM_INIT_VEBOX_COUNT),
    m_VeboxCount(0),
    m_nGPUFreqOriginal(0),
    m_nGPUFreqMin(0),
    m_nGPUFreqMax(0),
    m_bVtuneOn(false),
    m_IsDriverStoreEnabled(0)
{
    //Initialize Dev Create Param
    InitDevCreateOption( m_DevCreateOption, DevCreateOption );

    // Initialize the OS-Specific fields
    ConstructOSSpecific(DevCreateOption);
}

//*-----------------------------------------------------------------------------
//| Purpose:    The common part of destructor of CmDevice, that is OS independent
//| Returns:    None.
//*-----------------------------------------------------------------------------
void CmDeviceRT::DestructCommon()
{
    // Delete Predefined Program
    if(m_pGPUCopyKernelProgram)
    {
        DestroyProgram(m_pGPUCopyKernelProgram);
    }

    if(m_pSurfInitKernelProgram)
    {
        DestroyProgram(m_pSurfInitKernelProgram);
    }

    //Free the surface/memory for print buffer
    if(m_pPrintBufferMem)
    {
        MOS_AlignedFreeMemory(m_pPrintBufferMem);
    }

    if(m_pPrintBufferUP)
    {
        DestroyBufferUP(m_pPrintBufferUP);
    }

#if USE_EXTENSION_CODE
    // Free CmGTPin
    MOS_Delete(m_pGTPin);
#endif

    // Solve resource release dependency issue
    // Flush Queue to make sure no task internal and connected resouces left.
    m_CriticalSection_Queue.Acquire();
    for (auto iter = m_pQueue.begin(); iter != m_pQueue.end(); iter++)
    {
        (*iter)->CleanQueue();
    }
    m_CriticalSection_Queue.Release();

    for( uint32_t i = 0; i < m_KernelCount; i ++ )
    {
        CmKernelRT* pKernel = (CmKernelRT*)m_KernelArray.GetElement( i );
        if( pKernel )
        {
            CmProgramRT* pProgram = nullptr;
            pKernel->GetCmProgram(pProgram);
            uint32_t indexInProgramArray;
            for (indexInProgramArray = 0; indexInProgramArray < m_ProgramArray.GetSize(); indexInProgramArray++)
            {
                if (pProgram == m_ProgramArray.GetElement( indexInProgramArray ))
                {
                    break;
                }
            }
            CmKernelRT::Destroy( pKernel, pProgram );
            if ((pProgram == nullptr) && (indexInProgramArray < m_ProgramArray.GetSize()))
            {
                m_ProgramArray.SetElement(indexInProgramArray,  nullptr);
            }
        }
    }
    m_KernelArray.Delete();

    for( uint32_t i = 0; i < m_ProgramArray.GetSize(); i ++ )
    {
        CmProgramRT* pProgram = (CmProgramRT*)m_ProgramArray.GetElement( i );
        while( pProgram ) // Program can be acquired more than once
        {
            CmProgramRT::Destroy( pProgram );
        }
    }
    m_ProgramArray.Delete();

    for( uint32_t i = 0; i < m_SamplerArray.GetSize(); i ++ )
    {
        CmSamplerRT* pSampler =  (CmSamplerRT *)m_SamplerArray.GetElement( i );

        if(pSampler)
        {
            SamplerIndex* pIndex  = nullptr;
            pSampler->GetIndex( pIndex );
            CM_ASSERT( pIndex );
            uint32_t index = pIndex->get_data();

            CmSamplerRT::Destroy( pSampler );
            UnregisterSamplerState( index );
        }
    }
    m_SamplerArray.Delete();

    for(uint32_t i = 0; i < m_Sampler8x8Array.GetSize(); i ++ )
    {
         CmSampler8x8State_RT* pSampler8x8 =  (CmSampler8x8State_RT* )m_Sampler8x8Array.GetElement( i );
         if(pSampler8x8)
         {
            SamplerIndex* pIndex  = nullptr;
            pSampler8x8->GetIndex( pIndex );
            CM_ASSERT( pIndex );
            uint32_t index = pIndex->get_data();
            CmSampler8x8State_RT::Destroy( pSampler8x8 );
            UnregisterSampler8x8State( index );
         }
    }
    m_Sampler8x8Array.Delete();

    uint32_t ThreadSpaceArrayUsedSize = m_ThreadSpaceArray.GetSize();
    for( uint32_t i = 0; i < ThreadSpaceArrayUsedSize; i ++ )
    {
        CmThreadSpaceRT* pTS_RT = (CmThreadSpaceRT*)m_ThreadSpaceArray.GetElement( i );
        if( pTS_RT )
        {
            CmThreadSpaceRT::Destroy( pTS_RT );
        }
    }
    m_ThreadSpaceArray.Delete();

    for( uint32_t i = 0; i < m_ThreadGroupSpaceCount; i ++ ) // Destroy thread group space array
    {
        CmThreadGroupSpace* pTGS = (CmThreadGroupSpace*)m_ThreadGroupSpaceArray.GetElement( i );
        if( pTGS )
        {
            CmThreadGroupSpace::Destroy( pTGS );
        }
    }
    m_ThreadGroupSpaceArray.Delete();

    uint32_t TaskArrayUsedSize = m_TaskArray.GetSize();
    for( uint32_t i = 0; i < TaskArrayUsedSize; i ++ ) // Destroy task array
    {
        CmTaskRT* pTask = (CmTaskRT*)m_TaskArray.GetElement( i );
        if( pTask )
        {
            CmTaskRT::Destroy( pTask );
        }
    }
    m_TaskArray.Delete();

    for( uint32_t i = 0; i < m_VeboxCount; i ++ ) // Destroy Vebox array
    {
        CmVeboxRT* pVebox = (CmVeboxRT*)m_VeboxArray.GetElement(i);
        if (pVebox)
        {
            CmVeboxRT::Destroy(pVebox);
        }
    }
    m_VeboxArray.Delete();

    //Destroy Surface Manager
    CmSurfaceManager::Destroy( m_pSurfaceMgr );

    //Destroy Queue: Queue must be released after surface manager
    m_CriticalSection_Queue.Acquire();
    for (auto iter = m_pQueue.begin(); iter != m_pQueue.end();)
    {
        DestroyQueue(*iter);
        iter = m_pQueue.erase(iter);
    }
    m_CriticalSection_Queue.Release();

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
int32_t CmDeviceRT::Initialize(MOS_CONTEXT *pUmdContext)
{
    int32_t result = InitializeOSSpecific(pUmdContext);

    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Initialize OS specific failure.");
        return result;
    }

    m_pSurfaceMgr = nullptr;
    result = CmSurfaceManager::Create(
        this,
        m_HalMaxValues,
        m_HalMaxValuesEx,
        m_pSurfaceMgr );

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

    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Create CmQueue failure.");
        return result;
    }

    ReadVtuneProfilingFlag();

    // Load Predefined Kernels
    CmProgram* tmpProgram = nullptr;
    int32_t ret = 0;
    ret = LoadPredefinedCopyKernel(tmpProgram);
    if (ret != CM_SUCCESS)
    {
        return ret;
    }
    ret = LoadPredefinedInitKernel(tmpProgram);
    if (ret != CM_SUCCESS)
    {
        return ret;
    }

    DEVICE_LOG(this);

    return result;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Create Buffer
//| Arguments :   size              [in]    Size of the Buffer
//|               pSurface          [in/out]   Reference to Pointer to CmBuffer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateBuffer(uint32_t size, CmBuffer* & pSurface)
{
    INSERT_API_CALL_LOG();

    if( ( size < CM_MIN_SURF_WIDTH ) || ( size > CM_MAX_1D_SURF_WIDTH ) )
    {
        CM_ASSERTMESSAGE("Error: Invalid buffer size.");
        return CM_INVALID_WIDTH;
    }

    CLock locker(m_CriticalSection_Surface);

    CmBuffer_RT*    p = nullptr;
    void            *pSysMem = nullptr;
    int result = m_pSurfaceMgr->CreateBuffer(size, CM_BUFFER_N, false, p, nullptr, pSysMem, false, CM_DEFAULT_COMPARISON_VALUE);
    pSurface = static_cast< CmBuffer* >(p);

    return result;
}

//!
//! \brief    Create a CmBuffer from an existing MOS Resource.
//! \details  CmBuffer is a wrapper of that MOS resource. This Mos resource is
//!            owned by caller.
//! \param    [in] pMosResource
//!           pointer to MOS resource.
//! \param    [in,out] pSurface
//!           reference to pointer of surface to be created.
//! \retval   CM_SUCCESS if the CmBuffer is successfully created.
//! \retval   CM_INVALID_MOS_RESOURCE_HANDLE if pMosResource is nullptr.
//! \retval   CM_OUT_OF_HOST_MEMORY if out of system memory
//! \retval   CM_EXCEED_SURFACE_AMOUNT if maximum amount of 1D surfaces is exceeded.
//! \retval   CM_FAILURE otherwise
//!
CM_RT_API int32_t CmDeviceRT::CreateBuffer(PMOS_RESOURCE pMosResource,
                                           CmBuffer* & pSurface)
{
    INSERT_API_CALL_LOG();

    if(pMosResource == nullptr)
    {
        return CM_INVALID_MOS_RESOURCE_HANDLE;
    }

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)GetAccelData();
    PCM_HAL_STATE pState = pCmData->pCmHalState;

    MOS_SURFACE mosSurfDetails;
    MOS_ZeroMemory(&mosSurfDetails, sizeof(mosSurfDetails));
    int hr = pState->pOsInterface->pfnGetResourceInfo(pState->pOsInterface, pMosResource, &mosSurfDetails);
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

    CLock locker(m_CriticalSection_Surface);
    CmBuffer_RT* pBuffer = nullptr;
    void*        pSysMem = nullptr;
    int          ret = m_pSurfaceMgr->CreateBuffer(mosSurfDetails.dwWidth, CM_BUFFER_N, false,
                       pBuffer, pMosResource, pSysMem, false, CM_DEFAULT_COMPARISON_VALUE);
    pSurface = static_cast< CmBuffer* >(pBuffer);

    return ret;
}

//*--------------------------------------------------------------------------------------------
//| Purpose:    Create BufferUp
//| Arguments :   size              [in]     Size of the Buffer, should be uint32_t-aligned
//|               pSysMem           [in]     Pointer to host memory, must be page(4K bytes)-aligned.
//|               pSurface          [in/out]    Reference to Pointer to CmBufferUP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateBufferUP(uint32_t size,
                                             void* pSysMem,
                                             CmBufferUP* & pSurface)
{
    INSERT_API_CALL_LOG();

    // size should be in the valid range and be aligned in uint32_t
    if( ( size < CM_MIN_SURF_WIDTH ) || ( size > CM_MAX_1D_SURF_WIDTH ) || (size % sizeof(uint32_t)))
    {
        CM_ASSERTMESSAGE("Error: Invalid buffer size.");
        return CM_INVALID_WIDTH;
    }

    if (nullptr == pSysMem)
    {
        CM_ASSERTMESSAGE("Error: Pointer to host memory is null.");
        return CM_NULL_POINTER;
    }

    CLock locker(m_CriticalSection_Surface);

    CmBuffer_RT* p = nullptr;
    int result = m_pSurfaceMgr->CreateBuffer( size, CM_BUFFER_UP, false, p, nullptr, pSysMem, false, CM_DEFAULT_COMPARISON_VALUE );
    pSurface = static_cast< CmBufferUP* >(p);

    return result;
}

//*----------------------------------------------------------------------------
//| Purpose:    Destroy BufferUp
//| Arguments :  pSurface          [in]    Reference to Pointer to CmBuffer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::DestroyBufferUP(CmBufferUP* & pSurface)
{
    INSERT_API_CALL_LOG();

    CmBuffer_RT* temp = static_cast< CmBuffer_RT* >(pSurface);

    if(temp == nullptr)
    {
        return CM_FAILURE;
    }

    CLock locker(m_CriticalSection_Surface);

    int32_t status = m_pSurfaceMgr->DestroySurface(temp, APP_DESTROY);

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE may be returned, which should be treated as SUCCESS.
    {
        pSurface = nullptr;
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
//| Arguments :  pSurface          [in]    Reference to Pointer to CmBuffer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::ForceDestroyBufferUP(CmBufferUP* & pSurface)
{
    INSERT_API_CALL_LOG();
    CmBuffer_RT* temp = static_cast< CmBuffer_RT* >(pSurface);

    if( temp == nullptr)
    {
        return CM_FAILURE;
    }

    CLock locker(m_CriticalSection_Surface);

    int32_t status = m_pSurfaceMgr->DestroySurface(temp, FORCE_DESTROY);

    if(status == CM_SUCCESS)
    {
        pSurface = nullptr;
    }
    return status;
}


//*--------------------------------------------------------------------------------------------
//| Purpose:    Create Surface 2D UP
//| Arguments :   width             [in]     width of the  CmSurface2DUP
//|               height            [in]     height of the CmSurface2DUP
//|               format            [in]     format of the CmSurface2DUP
//|
//|               pSysMem           [in]     Pointer to host memory, must be page(4K bytes)-aligned.
//|               pSurface          [in/out]  Reference to  Pointer to CmSurface2DUP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateSurface2DUP(uint32_t width,
                                                uint32_t height,
                                                CM_SURFACE_FORMAT format,
                                                void* pSysMem,
                                                CmSurface2DUP* & pSurface )
{
    INSERT_API_CALL_LOG();

    int32_t result = m_pSurfaceMgr->Surface2DSanityCheck(width, height, format);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Surface2D sanity check failure.");
        return result;
    }

    if (nullptr == pSysMem)
    {
        CM_ASSERTMESSAGE("Error: Pointer to host memory is null.");
        return CM_INVALID_ARG_VALUE;
    }

    CmSurface2DUPRT *pSurfaceRT = nullptr;
    CLock locker(m_CriticalSection_Surface);
    result = m_pSurfaceMgr->CreateSurface2DUP( width, height, format, pSysMem, pSurfaceRT );
    pSurface = pSurfaceRT;
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface 2D
//| Arguments :   width             [in]     width of the  CmSurface2D
//|               height            [in]     height of the CmSurface2D
//|               format            [in]     format of the CmSurface2D
//|               pSurface          [in/out]    Reference to Pointer to CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateSurface2D(uint32_t width,
                                              uint32_t height,
                                              CM_SURFACE_FORMAT format,
                                              CmSurface2D* & pSurface )
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Surface);

    CmSurface2DRT *pSurfaceRT = nullptr;
    int ret = m_pSurfaceMgr->CreateSurface2D( width, height, 0, true, format, pSurfaceRT);
    pSurface = pSurfaceRT;
    return ret;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create shared Surface 2D (OS agnostic)
//| Arguments :
//|               pMosResource      [in]     Pointer to Mos resource
//|               pSurface          [out]    Reference to Pointer to CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateSurface2D(PMOS_RESOURCE pMosResource,
                                              CmSurface2D* & pSurface )
{
    INSERT_API_CALL_LOG();

    if(pMosResource == nullptr)
    {
        return CM_INVALID_MOS_RESOURCE_HANDLE;
    }

    CLock locker(m_CriticalSection_Surface);

    CmSurface2DRT *pSurfaceRT = nullptr;
    int ret = m_pSurfaceMgr->CreateSurface2D( pMosResource, false, pSurfaceRT);
    pSurface = pSurfaceRT;
    return ret;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface 2D
//| NOTE: Called by CM Wrapper, from CMRT Thin
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT:: CreateSurface2D(PMOS_RESOURCE pMosResource,
                                     bool bIsCmCreated,
                                     CmSurface2D* & pSurface)
{
    INSERT_API_CALL_LOG();

    if(pMosResource == nullptr)
    {
        return CM_INVALID_MOS_RESOURCE_HANDLE;
    }

    CLock locker(m_CriticalSection_Surface);

    CmSurface2DRT *pSurfaceRT = nullptr;
    int ret = m_pSurfaceMgr->CreateSurface2D( pMosResource, bIsCmCreated, pSurfaceRT);
    pSurface = pSurfaceRT;
    return ret;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface 2D
//| Arguments :
//|               width             [in]     width of the  CmSurface3D
//|               height            [in]     height of the CmSurface3D
//|               format            [in]     format of the CmSurface3D
//|               depth             [in]     depth  of the CmSurface3D
//|               pSurface          [out]    Reference to Pointer to CmSurface3D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateSurface3D(uint32_t width,
                                              uint32_t height,
                                              uint32_t depth,
                                              CM_SURFACE_FORMAT format,
                                              CmSurface3D* & pSurface )
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

    CLock locker(m_CriticalSection_Surface);
    CmSurface3DRT *pSurfaceRT = nullptr;
    int ret = m_pSurfaceMgr->CreateSurface3D( width, height, depth, format, pSurfaceRT );
    pSurface = pSurfaceRT;
    return ret;
}


CM_RT_API int32_t CmDeviceRT::DestroySurface( CmBuffer* & pSurface)
{
    CmBuffer_RT* temp = static_cast< CmBuffer_RT* >(pSurface);

    if(temp == nullptr)
    {
        return CM_FAILURE;
    }

    CLock locker(m_CriticalSection_Surface);

    int32_t status = m_pSurfaceMgr->DestroySurface( temp, APP_DESTROY);

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE, or  CM_SURFACE_CACHED may be returned, which should be treated as SUCCESS.
    {
        pSurface = nullptr;
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
CM_RT_API int32_t CmDeviceRT::DestroySurface2DUP( CmSurface2DUP* & pSurface)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Surface);

    CmSurface2DUPRT *pSurfaceRT = static_cast<CmSurface2DUPRT *>(pSurface);
    int32_t status = m_pSurfaceMgr->DestroySurface( pSurfaceRT, APP_DESTROY );

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE, or  CM_SURFACE_CACHED may be returned, which should be treated as SUCCESS.
    {
        pSurface = nullptr;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

CM_RT_API int32_t CmDeviceRT::DestroySurface( CmSurface3D* & pSurface)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Surface);

    CmSurface3DRT *pSurfaceRT = static_cast<CmSurface3DRT *>(pSurface);
    int32_t status = m_pSurfaceMgr->DestroySurface( pSurfaceRT, APP_DESTROY);

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE, or  CM_SURFACE_CACHED may be returned, which should be treated as SUCCESS.
    {
        pSurface = nullptr;
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
CM_RT_API int32_t CmDeviceRT::GetGenPlatform( uint32_t &platform )
{
    if( m_Platform != IGFX_UNKNOWN_CORE)
    {
        platform = m_Platform;
        return CM_SUCCESS;
    }

    platform = IGFX_UNKNOWN_CORE;

    int32_t hr = 0;
    CM_QUERY_CAPS      queryCaps;
    uint32_t           querySize  = sizeof( CM_QUERY_CAPS );

    CmSafeMemSet( &queryCaps, 0, sizeof( queryCaps ) );
    queryCaps.Type = CM_QUERY_GPU;

    hr  = GetCapsInternal( &queryCaps, &querySize);
    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Failed to get current GPU platform information.");
        return CM_FAILURE;
    }
    if (queryCaps.iVersion)
    {
        platform = queryCaps.iVersion;
    }

    return CM_SUCCESS;
}

//*----------------------------------------------------------------------------
//| Purpose:    Get Surface2D information: pitch and physical size in Video memory
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::GetSurface2DInfo(uint32_t width,
                                               uint32_t height,
                                               CM_SURFACE_FORMAT format,
                                               uint32_t & pitch,
                                               uint32_t & physicalSize)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE              hr = CM_SUCCESS;
    CM_HAL_SURFACE2D_UP_PARAM   inParam;
    PCM_CONTEXT_DATA            pCmData;
    PCM_HAL_STATE               pCmHalState;

    CMCHK_HR(m_pSurfaceMgr->Surface2DSanityCheck(width, height, format));

    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_SURFACE2D_UP_PARAM ) );
    inParam.iWidth  = width;
    inParam.iHeight = height;
    inParam.format  = format;

    pCmData = (PCM_CONTEXT_DATA)GetAccelData();
    pCmHalState = pCmData->pCmHalState;
    CHK_MOSSTATUS_RETURN_CMERROR(pCmHalState->pfnGetSurface2DPitchAndSize(pCmHalState, &inParam));

    pitch = inParam.iPitch;
    physicalSize = inParam.iPhysicalSize;

finish:
    return hr;
}

int32_t CmDeviceRT::GetSurfaceManager( CmSurfaceManager* & pSurfaceMgr )
{
    pSurfaceMgr = m_pSurfaceMgr;
    return CM_SUCCESS;
}

CSync* CmDeviceRT::GetSurfaceLock()
{
    return &m_CriticalSection_ReadWriteSurface2D;
}

CSync* CmDeviceRT::GetSurfaceCreationLock()
{
    return &m_CriticalSection_Surface;
}

CSync* CmDeviceRT::GetProgramKernelLock()
{
    return &m_CriticalSection_Program_Kernel;
}

std::vector<CmQueueRT *> &CmDeviceRT::GetQueue()
{
    return m_pQueue;
}

CSync* CmDeviceRT::GetQueueLock()
{
    return &m_CriticalSection_Queue;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Max values from Device
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetHalMaxValues(CM_HAL_MAX_VALUES* & pHalMaxValues,
                                    CM_HAL_MAX_VALUES_EX* & pHalMaxValuesEx)
{
    pHalMaxValues = &m_HalMaxValues;
    pHalMaxValuesEx = &m_HalMaxValuesEx;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Max values by Caps
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetMaxValueFromCaps(CM_HAL_MAX_VALUES &MaxValues,
                                        CM_HAL_MAX_VALUES_EX &MaxValuesEx)
{
    CM_QUERY_CAPS      queryCaps;
    uint32_t           querySize  = sizeof( CM_QUERY_CAPS );
    CmSafeMemSet( &queryCaps, 0, sizeof( CM_QUERY_CAPS ) );
    queryCaps.Type = CM_QUERY_MAX_VALUES;

    int32_t hr = GetCapsInternal(&queryCaps, &querySize);
    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Failed to get max values by GetCaps.");
        return CM_FAILURE;
    }

    MaxValues = queryCaps.MaxValues;
    MaxValues.iMaxArgsPerKernel = (queryCaps.MaxValues.iMaxArgsPerKernel > CM_MAX_ARGS_PER_KERNEL)?(CM_MAX_ARGS_PER_KERNEL):queryCaps.MaxValues.iMaxArgsPerKernel;

    CmSafeMemSet( &queryCaps, 0, sizeof( CM_QUERY_CAPS ) );
    queryCaps.Type = CM_QUERY_MAX_VALUES_EX;

    hr = GetCapsInternal(&queryCaps, &querySize);
    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Failed to get max values by GetCaps.");
        return CM_FAILURE;
    }
    MaxValuesEx = queryCaps.MaxValuesEx;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Caps from Internal
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetCapsInternal(void  *pCaps, uint32_t *puSize)
{
    PCM_QUERY_CAPS          pQueryCaps;
    PCM_CONTEXT_DATA        pCmData;
    PCM_HAL_STATE           pCmHalState;

    CM_RETURN_CODE  hr          = CM_SUCCESS;

    if ((!puSize)  || (!pCaps) || (*puSize < sizeof(CM_QUERY_CAPS)))
    {
        CM_ASSERTMESSAGE("Error: Invalid arguments.");
        hr = CM_FAILURE;
        goto finish;
    }

    pQueryCaps  = (PCM_QUERY_CAPS)pCaps;
    *puSize     = sizeof(CM_QUERY_CAPS);

    if (pQueryCaps->Type == CM_QUERY_VERSION)
    {
        pQueryCaps->iVersion    = CM_VERSION;
        hr = CM_SUCCESS;
        goto finish;
    }

    pCmData = (PCM_CONTEXT_DATA)GetAccelData();
    CMCHK_NULL(pCmData);

    pCmHalState = pCmData->pCmHalState;
    CMCHK_NULL(pCmHalState);

    switch (pQueryCaps->Type)
    {
    case CM_QUERY_REG_HANDLE:
        pQueryCaps->hRegistration   = QueryRegHandleInternal(pCmHalState);
        break;
    case CM_QUERY_MAX_VALUES:
        CHK_MOSSTATUS_RETURN_CMERROR(pCmHalState->pfnGetMaxValues(pCmHalState, &pQueryCaps->MaxValues));
        break;

    case CM_QUERY_MAX_VALUES_EX:
        CHK_MOSSTATUS_RETURN_CMERROR(pCmHalState->pfnGetMaxValuesEx(pCmHalState, &pQueryCaps->MaxValuesEx));
        break;

    case CM_QUERY_GPU:
    case CM_QUERY_GT:
    case CM_QUERY_MIN_RENDER_FREQ:
    case CM_QUERY_MAX_RENDER_FREQ:
    case CM_QUERY_STEP:
    case CM_QUERY_GPU_FREQ:
        hr = QueryGPUInfoInternal(pQueryCaps);
        if (hr != CM_SUCCESS)
            goto finish;
        break;

    case CM_QUERY_SURFACE2D_FORMAT_COUNT:
        pQueryCaps->Surface2DCount = CM_MAX_SURFACE2D_FORMAT_COUNT_INTERNAL;
        break;

    case CM_QUERY_SURFACE2D_FORMATS:
        hr = QuerySurface2DFormatsInternal(pQueryCaps);
        if (hr != CM_SUCCESS)
            goto finish;
        break;

    case CM_QUERY_PLATFORM_INFO:
        CHK_MOSSTATUS_RETURN_CMERROR(pCmHalState->pfnGetPlatformInfo(pCmHalState, &pQueryCaps->PlatformInfo, false));
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
 int32_t CmDeviceRT::GetCaps(CM_DEVICE_CAP_NAME capName,
                             uint32_t & capValueSize,
                             void* pCapValue )
{
    PCM_CONTEXT_DATA        pCmData;
    PCM_HAL_STATE           pCmHalState;
    CM_RETURN_CODE          hr = CM_SUCCESS;

    if (pCapValue == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to cap value is null.");
        return CM_NULL_POINTER;
    }

    pCmData = (PCM_CONTEXT_DATA)GetAccelData();
    if(pCmData == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to CM context data is null.");
        return CM_NULL_POINTER;
    }

    pCmHalState = pCmData->pCmHalState;
    if(pCmHalState == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to CM hal state is null.");
        return CM_NULL_POINTER;
    }

    switch( capName )
    {
    case CAP_KERNEL_COUNT_PER_TASK:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxKernelsPerTask ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxKernelsPerTask );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxKernelsPerTask, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_KERNEL_BINARY_SIZE:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxKernelBinarySize ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxKernelBinarySize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxKernelBinarySize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SAMPLER_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxSamplerTableSize ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxSamplerTableSize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxSamplerTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SAMPLER_COUNT_PER_KERNEL:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxSamplersPerKernel ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxSamplersPerKernel );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxSamplersPerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_BUFFER_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxBufferTableSize ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxBufferTableSize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxBufferTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2D_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValues.iMax2DSurfaceTableSize ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMax2DSurfaceTableSize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMax2DSurfaceTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE3D_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValues.iMax3DSurfaceTableSize ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMax3DSurfaceTableSize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMax3DSurfaceTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2DUP_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValuesEx.iMax2DUPSurfaceTableSize ) )
        {
            capValueSize = sizeof( m_HalMaxValuesEx.iMax2DUPSurfaceTableSize );
            CmSafeMemCopy( pCapValue, &m_HalMaxValuesEx.iMax2DUPSurfaceTableSize, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE_COUNT_PER_KERNEL:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxSurfacesPerKernel ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxSurfacesPerKernel );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxSurfacesPerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_ARG_COUNT_PER_KERNEL:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxArgsPerKernel ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxArgsPerKernel );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxArgsPerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_ARG_SIZE_PER_KERNEL:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxArgByteSizePerKernel ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxArgByteSizePerKernel );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxArgByteSizePerKernel, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_TASK:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxUserThreadsPerTask ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxUserThreadsPerTask );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxUserThreadsPerTask, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_MEDIA_WALKER:
        if( capValueSize >= sizeof( m_HalMaxValuesEx.iMaxUserThreadsPerMediaWalker ) )
        {
            capValueSize = sizeof( m_HalMaxValuesEx.iMaxUserThreadsPerMediaWalker );
            CmSafeMemCopy( pCapValue, &m_HalMaxValuesEx.iMaxUserThreadsPerMediaWalker, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_THREAD_GROUP:
        if( capValueSize >= sizeof( m_HalMaxValuesEx.iMaxUserThreadsPerThreadGroup ) )
        {
            capValueSize = sizeof( m_HalMaxValuesEx.iMaxUserThreadsPerThreadGroup );
            CmSafeMemCopy( pCapValue, &m_HalMaxValuesEx.iMaxUserThreadsPerThreadGroup, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_USER_DEFINED_THREAD_COUNT_PER_TASK_NO_THREAD_ARG:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxUserThreadsPerTaskNoThreadArg ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxUserThreadsPerTaskNoThreadArg );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxUserThreadsPerTaskNoThreadArg, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_HW_THREAD_COUNT:
        if( capValueSize >= sizeof( m_HalMaxValues.iMaxHwThreads ) )
        {
            capValueSize = sizeof( m_HalMaxValues.iMaxHwThreads );
            CmSafeMemCopy( pCapValue, &m_HalMaxValues.iMaxHwThreads, capValueSize );
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
            CmSafeMemCopy( pCapValue, &formatCount, capValueSize );
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_SURFACE2D_FORMATS:
        return QuerySurface2DFormats(pCapValue, capValueSize);
    case CAP_SURFACE3D_FORMAT_COUNT:
        if( capValueSize >= sizeof( uint32_t ) )
        {
            capValueSize = sizeof( uint32_t );
            uint32_t formatCount = CM_MAX_SURFACE3D_FORMAT_COUNT;
            CmSafeMemCopy( pCapValue, &formatCount, capValueSize );
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
            CmSafeMemCopy( pCapValue, formats, capValueSize );
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
            pCmHalState->pCmHalInterface->GetGenPlatformInfo(&platform, nullptr, nullptr);
            CmSafeMemCopy( pCapValue, &platform, capValueSize );
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
            queryCaps.Type = CM_QUERY_GT;
            uint32_t queryCapsSize  = sizeof( CM_QUERY_CAPS );
            hr = (CM_RETURN_CODE)GetCapsInternal(&queryCaps, &queryCapsSize);
            if ( hr != CM_SUCCESS)
            {
                return hr;
            }
            capValueSize = sizeof( uint32_t );
            uint32_t gtPlatform = queryCaps.genGT;
            CmSafeMemCopy( pCapValue, &gtPlatform, capValueSize );
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
            queryCaps.Type = CM_QUERY_MIN_RENDER_FREQ;
            uint32_t queryCapsSize  = sizeof( CM_QUERY_CAPS );
            hr = (CM_RETURN_CODE)GetCapsInternal(&queryCaps, &queryCapsSize);
            if (hr != CM_SUCCESS)
            {
                return hr;
            }
            uint32_t frequency = queryCaps.MinRenderFreq;
            capValueSize = sizeof( uint32_t );
            CmSafeMemCopy( pCapValue, &frequency, capValueSize );
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
            queryCaps.Type = CM_QUERY_MAX_RENDER_FREQ;
            uint32_t queryCapsSize  = sizeof( CM_QUERY_CAPS );
            hr = (CM_RETURN_CODE)GetCapsInternal(&queryCaps, &queryCapsSize);
            if (hr != CM_SUCCESS)
            {
                return hr;
            }
            uint32_t frequency = queryCaps.MaxRenderFreq;
            capValueSize = sizeof( uint32_t );
            CmSafeMemCopy( pCapValue, &frequency, capValueSize );
            return hr;
        }
        else
        {
            return CM_FAILURE;
        }

    case CAP_GPU_CURRENT_FREQUENCY:
        if( (m_DDIVersion >= CM_DDI_3_0) && (capValueSize >= sizeof( uint32_t )) )
        {
            CM_QUERY_CAPS   queryCaps;
            queryCaps.Type = CM_QUERY_GPU_FREQ;
            uint32_t queryCapsSize  = sizeof( CM_QUERY_CAPS );
            hr = (CM_RETURN_CODE)GetCapsInternal(&queryCaps, &queryCapsSize);
            if (hr != CM_SUCCESS)
            {
                return hr;
            }
            uint32_t frequency = queryCaps.GPUCurrentFreq;
            capValueSize = sizeof( uint32_t );
            CmSafeMemCopy( pCapValue, &frequency, capValueSize );
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
            queryCaps.Type = CM_QUERY_PLATFORM_INFO;
            uint32_t queryCapsSize = sizeof(CM_QUERY_CAPS);
            hr = (CM_RETURN_CODE)GetCapsInternal(&queryCaps, &queryCapsSize);
            if (hr != CM_SUCCESS)
            {
                return hr;
            }
            capValueSize = sizeof(CM_PLATFORM_INFO);
            PCM_PLATFORM_INFO pPlatformInfo = &(queryCaps.PlatformInfo);
            CmSafeMemCopy(pCapValue, pPlatformInfo, capValueSize);
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
            CmSafeMemCopy(pCapValue, &maxBufferSize, capValueSize);
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
//|               pCommonISACode    [in]       pointer to memory where common isa locates
//|               size              [in]       size of common isa
//|               pProgram          [in/out]   Pointer to CmProgram
//|               options           [in]       options : non-jitter,jitter
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::LoadProgram(void* pCommonISACode,
                                          const uint32_t size,
                                          CmProgram*& pProgram,
                                          const char* options )
{
    INSERT_API_CALL_LOG();

    int32_t result;

    if ((pCommonISACode == nullptr) || (size == 0))
    {
        CM_ASSERTMESSAGE("Error: Invalid vistual isa.");
        return CM_INVALID_COMMON_ISA;
    }

    CLock locker(m_CriticalSection_Program_Kernel);

    uint32_t firstfreeslot = m_ProgramArray.GetFirstFreeIndex();

    CmProgramRT *pProgramRT = static_cast<CmProgramRT *>(pProgram);
    result = CmProgramRT::Create( this, pCommonISACode, size, nullptr, 0, pProgramRT, options, firstfreeslot );
    if( result == CM_SUCCESS )
    {
        m_ProgramArray.SetElement( firstfreeslot, pProgramRT );
        m_ProgramCount ++;
    }
    pProgram = pProgramRT;

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Program
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::DestroyProgram(CmProgram* & pProgram)
{
    INSERT_API_CALL_LOG();

    if( pProgram == nullptr )
    {
        return CM_FAILURE;
    }

    CLock locker(m_CriticalSection_Program_Kernel);

    CmProgramRT *pProgramRT = static_cast<CmProgramRT *>(pProgram);
    uint32_t indexInProgramArrary = pProgramRT->GetProgramIndex();
    if( pProgramRT == m_ProgramArray.GetElement( indexInProgramArrary ) )
    {
        CmProgramRT::Destroy( pProgramRT );
        if( pProgramRT == nullptr )
        {
            m_ProgramArray.SetElement( indexInProgramArrary, nullptr );
            m_ProgramCount--;
            pProgram = pProgramRT;
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
//|               pKernel           [out]      pointer to CmKernel
//|               kernelName        [in]       string of kernel's name
//|               pProgram          [in/out]   Pointer to CmProgram
//|               options           [in]       options : non-jitter,jitter
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateKernel(CmProgram* pProgram,
                                           const char* kernelName,
                                           CmKernel* & pKernel,
                                           const char* options )
{
    INSERT_API_CALL_LOG();

    if(pProgram == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to CmProgram is null.");
        return CM_NULL_POINTER;
    }

    CLock locker(m_CriticalSection_Program_Kernel);

    uint32_t freeSlotInKernelArray = m_KernelArray.GetFirstFreeIndex();
    CmProgramRT *pProgramRT = static_cast<CmProgramRT *>(pProgram);
    CmKernelRT *pKernelRT = static_cast<CmKernelRT *>(pKernel);
    int32_t result = CmKernelRT::Create( this, pProgramRT, kernelName, freeSlotInKernelArray, m_KernelCount, pKernelRT, options );
    pKernel = pKernelRT;
    if( result == CM_SUCCESS )
    {
        m_KernelArray.SetElement( freeSlotInKernelArray, pKernel );
        m_KernelCount ++;
    }

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::DestroyKernel(CmKernel*& pKernel)
{
    INSERT_API_CALL_LOG();

    if( pKernel == nullptr )
    {
        return CM_NULL_POINTER;
    }

    CLock locker(m_CriticalSection_Program_Kernel);

    CmKernelRT *pKernelRT = static_cast<CmKernelRT *>(pKernel);
    uint32_t indexInKernelArrary = pKernelRT->GetKernelIndex();
    if( pKernelRT == m_KernelArray.GetElement( indexInKernelArrary ) )
    {
        CmProgramRT* pProgram = nullptr;
        pKernelRT->GetCmProgram(pProgram);
        if ( pProgram == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Failed to get valid program.");
            return CM_NULL_POINTER;
        }

        uint32_t indexInProgramArray = pProgram->GetProgramIndex();

        if (pProgram == m_ProgramArray.GetElement( indexInProgramArray ))
        {
            CmKernelRT::Destroy( pKernelRT, pProgram );
            pKernel = pKernelRT;

            if(pKernelRT == nullptr)
            {
                m_KernelArray.SetElement( indexInKernelArrary, nullptr );
            }

            if (pProgram == nullptr)
            {
                m_ProgramArray.SetElement(indexInProgramArray,  nullptr);
            }

            // Note: NOT reduce m_KernelCount here, need to make it to loop mode later
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

CM_RT_API int32_t CmDeviceRT::CreateQueue(CmQueue* & pQueue)
{
    CmQueue *queue = nullptr;

    // For legacy CreateQueue API, we will only return the same queue
    m_CriticalSection_Queue.Acquire();
    for (auto iter = m_pQueue.begin(); iter != m_pQueue.end(); iter++)
    {
        CM_QUEUE_TYPE queue_type = (*iter)->GetQueueOption().QueueType;
        if (queue_type == CM_QUEUE_TYPE_RENDER)
        {
            pQueue = (*iter);
            m_CriticalSection_Queue.Release();
            return CM_SUCCESS;
        }
    }
    m_CriticalSection_Queue.Release();

    CM_QUEUE_CREATE_OPTION QueueCreateOption = CM_DEFAULT_QUEUE_CREATE_OPTION;
    int32_t result = CreateQueueEx(queue, QueueCreateOption);

    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Failed to create the queue.");
        return result;
    }

    pQueue = queue;
    return CM_SUCCESS;
}

CM_RT_API int32_t
CmDeviceRT::CreateQueueEx(CmQueue* & pQueue,
                          CM_QUEUE_CREATE_OPTION QueueCreateOption)
{
    INSERT_API_CALL_LOG();

    m_CriticalSection_Queue.Acquire();
    CmQueueRT *pQueueRT = nullptr;
    int32_t result = CmQueueRT::Create(this, pQueueRT, QueueCreateOption);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Failed to create the queue.");
        m_CriticalSection_Queue.Release();
        return result;
    }

    m_pQueue.push_back(pQueueRT);
    pQueue = pQueueRT;
    m_CriticalSection_Queue.Release();

    return result;
}

CM_RT_API int32_t CmDeviceRT::CreateTask(CmTask *& pTask)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Task);

    uint32_t freeSlotInTaskArray = m_TaskArray.GetFirstFreeIndex();
    CmTaskRT *pTaskRT = nullptr;
    int32_t result = CmTaskRT::Create(this, freeSlotInTaskArray, m_HalMaxValues.iMaxKernelsPerTask, pTaskRT);
    if (result == CM_SUCCESS)
    {
        m_TaskArray.SetElement( freeSlotInTaskArray, pTaskRT );
        m_TaskCount ++;
    }
    pTask = pTaskRT;
    return result;
}

int32_t CmDeviceRT::DestroyQueue(CmQueueRT* & pQueue)
{
    if(pQueue == nullptr )
    {
        return CM_NULL_POINTER;
    }

    return CmQueueRT::Destroy(pQueue);
}

CM_RT_API int32_t CmDeviceRT::DestroyTask(CmTask*& pTask)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Task);

    if( pTask == nullptr )
    {
        return CM_FAILURE;
    }

    CmTaskRT *pTaskRT = static_cast<CmTaskRT *>(pTask);
    uint32_t index = pTaskRT->GetIndexInTaskArray();
    if(pTaskRT == (CmTaskRT *)m_TaskArray.GetElement( index ))
    {
        int32_t status = CmTaskRT::Destroy(pTaskRT);
        if(status == CM_SUCCESS)
        {
            m_TaskArray.SetElement( index, nullptr );
            pTask = nullptr;
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
CM_RT_API int32_t CmDeviceRT::CreateThreadSpace(uint32_t width,
                                                uint32_t height,
                                                CmThreadSpace* & pTS)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_ThreadSpace);

    uint32_t freeSlotInThreadSpaceArray = m_ThreadSpaceArray.GetFirstFreeIndex();
    CmThreadSpaceRT *pTSRT = nullptr;
    int32_t result = CmThreadSpaceRT::Create( this, freeSlotInThreadSpaceArray, width, height, pTSRT );
    if (result == CM_SUCCESS)
    {
        m_ThreadSpaceArray.SetElement( freeSlotInThreadSpaceArray, pTSRT );
        m_ThreadSpaceCount ++;
    }
    pTS = pTSRT;

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Thread Space
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::DestroyThreadSpace(CmThreadSpace* & pTS)
{
    INSERT_API_CALL_LOG();

    if( pTS == nullptr )
    {
        return CM_FAILURE;
    }

    CmThreadSpaceRT *pTSRT = static_cast<CmThreadSpaceRT *>(pTS);
    uint32_t indexTs = pTSRT->GetIndexInTsArray();

    CLock locker(m_CriticalSection_ThreadSpace);
    if(pTS == m_ThreadSpaceArray.GetElement( indexTs ))
    {
        int32_t status = CmThreadSpaceRT::Destroy( pTSRT );
        if(status == CM_SUCCESS)
        {
            m_ThreadSpaceArray.SetElement( indexTs, nullptr );
            pTS = nullptr;
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
CmDeviceRT::CreateVmeSurfaceG7_5(CmSurface2D* pCurSurface,
                                 CmSurface2D** pForwardSurface,
                                 CmSurface2D** pBackwardSurface,
                                 const uint32_t surfaceCountForward,
                                 const uint32_t surfaceCountBackward,
                                 SurfaceIndex* & pVmeIndex)
{
    INSERT_API_CALL_LOG();

    if(pCurSurface == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_NULL_POINTER;
    }

    CmSurface2DRT* pCurrentRT = static_cast<CmSurface2DRT *>(pCurSurface) ;
    CmSurface2DRT** pForward  = nullptr;
    CmSurface2DRT** pBackward = nullptr;

    if( ! pCurrentRT )
    {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_INVALID_ARG_VALUE;
    }

    if(pForwardSurface != nullptr)
    {
        pForward = MOS_NewArray( CmSurface2DRT*, surfaceCountForward);
        if(pForward == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
        for(uint32_t i = 0; i< surfaceCountForward; i++)
        {
            pForward[i] = static_cast<CmSurface2DRT *>( pForwardSurface[i] );
            if(pForward[i] == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Invalid forward surfaces.");
                MosSafeDeleteArray(pForward);
                return CM_INVALID_ARG_VALUE;
            }
        }
    }

    if(pBackwardSurface != nullptr)
    {
        pBackward = MOS_NewArray(CmSurface2DRT*,surfaceCountBackward);
        if(pBackward == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            MosSafeDeleteArray(pForward);
            return CM_OUT_OF_HOST_MEMORY;
        }
        for(uint32_t i = 0; i< surfaceCountBackward; i++)
        {
            pBackward[i] = static_cast<CmSurface2DRT *>( pBackwardSurface[i] );
            if(pBackward[i] == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Invalid backward surfaces.");
                MosSafeDeleteArray(pForward);
                MosSafeDeleteArray(pBackward);
                return CM_INVALID_ARG_VALUE;
            }
        }
    }

    CLock locker(m_CriticalSection_Surface);

    int32_t status = m_pSurfaceMgr->CreateVmeSurface(pCurrentRT, pForward, pBackward, surfaceCountForward, surfaceCountBackward, pVmeIndex);

    MosSafeDeleteArray(pForward);
    MosSafeDeleteArray(pBackward);

    return status;
}

CM_RT_API int32_t CmDeviceRT::DestroyVmeSurfaceG7_5(SurfaceIndex* & pVmeIndex)
{
    INSERT_API_CALL_LOG();
    return DestroyVmeSurface( pVmeIndex );
}

CM_RT_API int32_t CmDeviceRT::SetVmeSurfaceStateParam(SurfaceIndex* pVmeIndex, CM_VME_SURFACE_STATE_PARAM *pSSParam)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Surface);

    CM_RETURN_CODE  hr = CM_SUCCESS;
    CmSurface *pCmSurface = nullptr;
    CmSurfaceVme *pVmeSurface = nullptr;

    CMCHK_NULL(pVmeIndex);
    CMCHK_NULL(pSSParam);

    m_pSurfaceMgr->GetSurface(pVmeIndex->get_data(), pCmSurface);
    CMCHK_NULL(pCmSurface);

    // check if it is a vme index
    if (pCmSurface->Type() != CM_ENUM_CLASS_TYPE_CMSURFACEVME)
    {
        CM_ASSERTMESSAGE("Error: SetVmeSurfaceStateParam only can config VME surfaces.");
        return CM_INVALID_ARG_INDEX;
    }

    pVmeSurface = static_cast<CmSurfaceVme *>(pCmSurface);
    pVmeSurface->SetSurfaceStateResolution(pSSParam->width, pSSParam->height);

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
CmDeviceRT::CreateSampler(const CM_SAMPLER_STATE& samplerState,
                          CmSampler* & pSampler)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Sampler);

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
        m_SamplerArray.SetElement( index,  ptmp );
        pSampler = static_cast< CmSampler* >(ptmp);
    }
    else
    {
        UnregisterSamplerState( index );
    }
    return result;
}

CM_RT_API int32_t
CmDeviceRT::CreateSamplerEx(const CM_SAMPLER_STATE_EX& samplerState,
                            CmSampler* & pSampler)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Sampler);

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
        m_SamplerArray.SetElement( index,  ptmp );
        pSampler = static_cast< CmSampler* >(ptmp);
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
CM_RT_API int32_t CmDeviceRT::DestroySampler(CmSampler*& pSampler)
{
    INSERT_API_CALL_LOG();
    CLock locker(m_CriticalSection_Sampler);

    CmSamplerRT* temp = nullptr;
    if(pSampler)
    {
        temp = static_cast< CmSamplerRT* >(pSampler);
    }
    else
    {
        return CM_FAILURE;
    }

    SamplerIndex* pIndex  = nullptr;
    temp->GetIndex( pIndex );
    CM_ASSERT( pIndex );
    uint32_t index = pIndex->get_data();

    CM_ASSERT( m_SamplerArray.GetElement( index ) == (temp) );

    int32_t status = CmSamplerRT::Destroy( temp );
    if(status == CM_SUCCESS)
    {
        UnregisterSamplerState( index );
        m_SamplerArray.SetElement( index, nullptr );
        pSampler = nullptr;
    }

    return status;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register Sampler State in CM devie's table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::RegisterSamplerState(const CM_SAMPLER_STATE& samplerState,
                                         uint32_t& index)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    index = 0;

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)GetAccelData();

    CM_HAL_SAMPLER_PARAM param;
    MOS_ZeroMemory(&param, sizeof(CM_HAL_SAMPLER_PARAM));
    param.AddressU = samplerState.addressU;
    param.AddressV = samplerState.addressV;
    param.AddressW = samplerState.addressW;
    param.MagFilter = samplerState.magFilterType;
    param.MinFilter = samplerState.minFilterType;
    param.dwHandle = 0;

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnRegisterSampler(pCmData->pCmHalState, &param));

    index = param.dwHandle;

finish:
    return hr;
}

int32_t
CmDeviceRT::RegisterSamplerStateEx(const CM_SAMPLER_STATE_EX& samplerState,
                                   uint32_t& index)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    index = 0;

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)GetAccelData();

    CM_HAL_SAMPLER_PARAM param;
    MOS_ZeroMemory(&param, sizeof(CM_HAL_SAMPLER_PARAM));
    param.AddressU = samplerState.addressU;
    param.AddressV = samplerState.addressV;
    param.AddressW = samplerState.addressW;
    param.MagFilter = samplerState.magFilterType;
    param.MinFilter = samplerState.minFilterType;
    param.dwHandle = 0;

    param.SurfaceFormat = (CM_HAL_PIXEL_TYPE)samplerState.SurfaceFormat;
    switch (param.SurfaceFormat)
    {
        case CM_HAL_PIXEL_UINT:
            param.BorderColorRedU = samplerState.BorderColorRedU;
            param.BorderColorGreenU = samplerState.BorderColorGreenU;
            param.BorderColorBlueU = samplerState.BorderColorBlueU;
            param.BorderColorAlphaU = samplerState.BorderColorAlphaU;
            break;
        case CM_HAL_PIXEL_SINT:
            param.BorderColorRedS = samplerState.BorderColorRedS;
            param.BorderColorGreenS = samplerState.BorderColorGreenS;
            param.BorderColorBlueS = samplerState.BorderColorBlueS;
            param.BorderColorAlphaS = samplerState.BorderColorAlphaS;
            break;
        default:
            param.BorderColorRedF = samplerState.BorderColorRedF;
            param.BorderColorGreenF = samplerState.BorderColorGreenF;
            param.BorderColorBlueF = samplerState.BorderColorBlueF;
            param.BorderColorAlphaF = samplerState.BorderColorAlphaF;
    }

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnRegisterSampler(pCmData->pCmHalState, &param));

    index = param.dwHandle;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Unregister Sampler State in CM devie's table
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::UnregisterSamplerState(uint32_t index)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)GetAccelData();

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnUnRegisterSampler(pCmData->pCmHalState, index));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler8x8 State
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRT::CreateSampler8x8(const CM_SAMPLER_8X8_DESCR  & smplDescr,
                             CmSampler8x8*& s8x8State)
{
    INSERT_API_CALL_LOG();
    CLock locker(m_CriticalSection_Sampler8x8);

    int32_t result = CM_FAILURE;

    if((smplDescr.stateType == CM_SAMPLER8X8_AVS && smplDescr.avs == nullptr) ||
        (smplDescr.stateType == CM_SAMPLER8X8_CONV && smplDescr.conv == nullptr) ||
        (smplDescr.stateType == CM_SAMPLER8X8_MISC && smplDescr.misc == nullptr) ||
        (smplDescr.stateType == CM_SAMPLER8X8_NONE && smplDescr.conv != nullptr) ||
        s8x8State != nullptr)  {
        CM_ASSERTMESSAGE("Error: Invalid arguments.");
        return CM_INVALID_ARG_VALUE;
    }
    CmSampler8x8State_RT* ptmp = nullptr;
    uint32_t index = 0;

    int32_t hr = RegisterSampler8x8State( smplDescr, index );
    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Register sampler8x8 state failure.");
        return CM_EXCEED_SAMPLER_AMOUNT;
    }

    result = CmSampler8x8State_RT::Create( smplDescr, index, ptmp );
    if( result == CM_SUCCESS )
    {
        m_Sampler8x8Array.SetElement( index, ptmp );
        s8x8State = static_cast< CmSampler8x8* >(ptmp);
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
CM_RT_API int32_t CmDeviceRT::DestroySampler8x8(CmSampler8x8*& ps8x8State)
{
    INSERT_API_CALL_LOG();
    CLock locker(m_CriticalSection_Sampler8x8);

    CmSampler8x8State_RT* temp = nullptr;
    if(ps8x8State)
    {
        temp = static_cast< CmSampler8x8State_RT* >(ps8x8State);
    }
    else
    {
        return CM_FAILURE;
    }

    SamplerIndex* pIndex  = nullptr;
    temp->GetIndex( pIndex );
    CM_ASSERT( pIndex );
    uint32_t index = pIndex->get_data();

    CM_ASSERT( m_Sampler8x8Array.GetElement( index ) == (temp) );

    int32_t status = CmSampler8x8State_RT::Destroy( temp );
    if(status == CM_SUCCESS)
    {
        UnregisterSampler8x8State( index );
        m_Sampler8x8Array.SetElement( index, nullptr );
        ps8x8State = nullptr;
    }

    return status;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler8x8 Surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRT::CreateSampler8x8Surface(CmSurface2D* pCurrentSurface,
                                    SurfaceIndex* & pSampler8x8Index,
                                    CM_SAMPLER8x8_SURFACE sampler8x8_type,
                                    CM_SURFACE_ADDRESS_CONTROL_MODE mode)
{
    INSERT_API_CALL_LOG();
    uint32_t width, height, sizeperpixel;

    CmSurface2DRT* pCurrentRT = static_cast<CmSurface2DRT *>(pCurrentSurface);
    if( ! pCurrentRT )  {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_NULL_POINTER;
    }

    CM_SURFACE_FORMAT format;
    pCurrentRT->GetSurfaceDesc(width, height, format, sizeperpixel);

    if(format == CM_SURFACE_FORMAT_NV12)
        if ((width % 4) != 0 || (height % 4) != 0) {  //width or height is not 4 aligned
            CM_ASSERTMESSAGE("Error: Width or height is not 4 aligned for nv12 surface.");
            return CM_SYSTEM_MEMORY_NOT_4PIXELS_ALIGNED;
        }
    CLock locker(m_CriticalSection_Surface);

    int32_t result = m_pSurfaceMgr->CreateSampler8x8Surface( pCurrentRT, pSampler8x8Index, sampler8x8_type, mode, nullptr );

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler8x8 Surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRT::CreateSampler8x8SurfaceEx(CmSurface2D* pCurrentSurface,
                                      SurfaceIndex* & pSampler8x8Index,
                                      CM_SAMPLER8x8_SURFACE sampler8x8_type,
                                      CM_SURFACE_ADDRESS_CONTROL_MODE mode,
                                      CM_FLAG* pFlag)
{
    INSERT_API_CALL_LOG();
    CM_ROTATION rotationFlag = CM_ROTATION_IDENTITY;

    CmSurface2DRT* pCurrentRT = static_cast<CmSurface2DRT *>(pCurrentSurface);
    if (!pCurrentRT)  {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_NULL_POINTER;
    }
    CLock locker(m_CriticalSection_Surface);

    int32_t result = m_pSurfaceMgr->CreateSampler8x8Surface(pCurrentRT, pSampler8x8Index, sampler8x8_type, mode, pFlag);

    return result;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler Surface 2D with Flag
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRT::CreateSamplerSurface2DEx(CmSurface2D* p2DSurface,
                                     SurfaceIndex* & pSamplerSurfaceIndex,
                                     CM_FLAG* pFlag)
{
    INSERT_API_CALL_LOG();

    if (!p2DSurface) {
        CM_ASSERTMESSAGE("Error: Pointer to sampler surface 2D is null.");
        return CM_NULL_POINTER;
    }

    uint32_t width, height, sizeperpixel;
    CM_SURFACE_FORMAT format;
    CmSurface2DRT* p2DSurfaceRT = static_cast<CmSurface2DRT *>(p2DSurface);
    p2DSurfaceRT->GetSurfaceDesc(width, height, format, sizeperpixel);
    if (!m_pSurfaceMgr->IsSupportedForSamplerSurface2D(format))
    {
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    CLock locker(m_CriticalSection_Surface);

    int32_t result = m_pSurfaceMgr->CreateSamplerSurface(p2DSurfaceRT, pSamplerSurfaceIndex, pFlag);

    return result;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Sampler8x8 Surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRT::DestroySampler8x8Surface(SurfaceIndex* & pSampler8x8Index)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Surface);

    int32_t result = m_pSurfaceMgr->DestroySampler8x8Surface( pSampler8x8Index );

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Coefficient Format Transform
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
#define FloatToS1_6(x)  (uint8_t)((char)(x * 64))
void
CmDeviceRT::Sampler8x8CoefficientFormatTransform(
            CM_AVS_INTERNEL_NONPIPLINED_STATE* dst_avs_state,
            CM_AVS_NONPIPLINED_STATE* src_avs_state)
{
    int i;

    CmSafeMemSet( dst_avs_state, 0, sizeof(CM_AVS_INTERNEL_NONPIPLINED_STATE));

    dst_avs_state->BypassXAF = src_avs_state->BypassXAF;
    dst_avs_state->BypassYAF = src_avs_state->BypassYAF;
    dst_avs_state->DefaultSharpLvl = src_avs_state->DefaultSharpLvl;
    dst_avs_state->bEnableRGBAdaptive = src_avs_state->bEnableRGBAdaptive;
    dst_avs_state->bAdaptiveFilterAllChannels = src_avs_state->bAdaptiveFilterAllChannels;
    if (!src_avs_state->BypassXAF && !src_avs_state->BypassYAF) {
        dst_avs_state->maxDerivative4Pixels = src_avs_state->maxDerivative4Pixels;
        dst_avs_state->maxDerivative8Pixels = src_avs_state->maxDerivative8Pixels;
        dst_avs_state->transitionArea4Pixels = src_avs_state->transitionArea4Pixels;
        dst_avs_state->transitionArea8Pixels = src_avs_state->transitionArea8Pixels;
    }

    for (i = 0; i < CM_NUM_COEFF_ROWS_SKL; i++) {
      dst_avs_state->Tbl0X[i].FilterCoeff_0_0 = FloatToS1_6(src_avs_state->Tbl0X[i].FilterCoeff_0_0);
      dst_avs_state->Tbl0X[i].FilterCoeff_0_1 = FloatToS1_6(src_avs_state->Tbl0X[i].FilterCoeff_0_1);
      dst_avs_state->Tbl0X[i].FilterCoeff_0_2 = FloatToS1_6(src_avs_state->Tbl0X[i].FilterCoeff_0_2);
      dst_avs_state->Tbl0X[i].FilterCoeff_0_3 = FloatToS1_6(src_avs_state->Tbl0X[i].FilterCoeff_0_3);
      dst_avs_state->Tbl0X[i].FilterCoeff_0_4 = FloatToS1_6(src_avs_state->Tbl0X[i].FilterCoeff_0_4);
      dst_avs_state->Tbl0X[i].FilterCoeff_0_5 = FloatToS1_6(src_avs_state->Tbl0X[i].FilterCoeff_0_5);
      dst_avs_state->Tbl0X[i].FilterCoeff_0_6 = FloatToS1_6(src_avs_state->Tbl0X[i].FilterCoeff_0_6);
      dst_avs_state->Tbl0X[i].FilterCoeff_0_7 = FloatToS1_6(src_avs_state->Tbl0X[i].FilterCoeff_0_7);

      dst_avs_state->Tbl0Y[i].FilterCoeff_0_0 = FloatToS1_6(src_avs_state->Tbl0Y[i].FilterCoeff_0_0);
      dst_avs_state->Tbl0Y[i].FilterCoeff_0_1 = FloatToS1_6(src_avs_state->Tbl0Y[i].FilterCoeff_0_1);
      dst_avs_state->Tbl0Y[i].FilterCoeff_0_2 = FloatToS1_6(src_avs_state->Tbl0Y[i].FilterCoeff_0_2);
      dst_avs_state->Tbl0Y[i].FilterCoeff_0_3 = FloatToS1_6(src_avs_state->Tbl0Y[i].FilterCoeff_0_3);
      dst_avs_state->Tbl0Y[i].FilterCoeff_0_4 = FloatToS1_6(src_avs_state->Tbl0Y[i].FilterCoeff_0_4);
      dst_avs_state->Tbl0Y[i].FilterCoeff_0_5 = FloatToS1_6(src_avs_state->Tbl0Y[i].FilterCoeff_0_5);
      dst_avs_state->Tbl0Y[i].FilterCoeff_0_6 = FloatToS1_6(src_avs_state->Tbl0Y[i].FilterCoeff_0_6);
      dst_avs_state->Tbl0Y[i].FilterCoeff_0_7 = FloatToS1_6(src_avs_state->Tbl0Y[i].FilterCoeff_0_7);

      dst_avs_state->Tbl1X[i].FilterCoeff_0_0 = FloatToS1_6(src_avs_state->Tbl1X[i].FilterCoeff_0_0);
      dst_avs_state->Tbl1X[i].FilterCoeff_0_1 = FloatToS1_6(src_avs_state->Tbl1X[i].FilterCoeff_0_1);
      dst_avs_state->Tbl1X[i].FilterCoeff_0_2 = FloatToS1_6(src_avs_state->Tbl1X[i].FilterCoeff_0_2);
      dst_avs_state->Tbl1X[i].FilterCoeff_0_3 = FloatToS1_6(src_avs_state->Tbl1X[i].FilterCoeff_0_3);
      dst_avs_state->Tbl1X[i].FilterCoeff_0_4 = FloatToS1_6(src_avs_state->Tbl1X[i].FilterCoeff_0_4);
      dst_avs_state->Tbl1X[i].FilterCoeff_0_5 = FloatToS1_6(src_avs_state->Tbl1X[i].FilterCoeff_0_5);
      dst_avs_state->Tbl1X[i].FilterCoeff_0_6 = FloatToS1_6(src_avs_state->Tbl1X[i].FilterCoeff_0_6);
      dst_avs_state->Tbl1X[i].FilterCoeff_0_7 = FloatToS1_6(src_avs_state->Tbl1X[i].FilterCoeff_0_7);

      dst_avs_state->Tbl1Y[i].FilterCoeff_0_0 = FloatToS1_6(src_avs_state->Tbl1Y[i].FilterCoeff_0_0);
      dst_avs_state->Tbl1Y[i].FilterCoeff_0_1 = FloatToS1_6(src_avs_state->Tbl1Y[i].FilterCoeff_0_1);
      dst_avs_state->Tbl1Y[i].FilterCoeff_0_2 = FloatToS1_6(src_avs_state->Tbl1Y[i].FilterCoeff_0_2);
      dst_avs_state->Tbl1Y[i].FilterCoeff_0_3 = FloatToS1_6(src_avs_state->Tbl1Y[i].FilterCoeff_0_3);
      dst_avs_state->Tbl1Y[i].FilterCoeff_0_4 = FloatToS1_6(src_avs_state->Tbl1Y[i].FilterCoeff_0_4);
      dst_avs_state->Tbl1Y[i].FilterCoeff_0_5 = FloatToS1_6(src_avs_state->Tbl1Y[i].FilterCoeff_0_5);
      dst_avs_state->Tbl1Y[i].FilterCoeff_0_6 = FloatToS1_6(src_avs_state->Tbl1Y[i].FilterCoeff_0_6);
      dst_avs_state->Tbl1Y[i].FilterCoeff_0_7 = FloatToS1_6(src_avs_state->Tbl1Y[i].FilterCoeff_0_7);
    }

    return;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register Sampler8x8 State (Not implemented yet)
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::RegisterSampler8x8State(
                    const CM_SAMPLER_8X8_DESCR & sampler8x8State,
                    uint32_t& index)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    void  *dst = nullptr;
    void  *src = nullptr;

    CM_HAL_SAMPLER_8X8_PARAM     param;
    CM_AVS_STATE_MSG            *pCmAvs;
    PMHW_SAMPLER_STATE_AVS_PARAM pMhwAvs;

    CmSafeMemSet(&param, 0, sizeof(CM_HAL_SAMPLER_8X8_PARAM));

    index = 0;
    param.dwHandle = 0;
    param.sampler8x8State.stateType = sampler8x8State.stateType;

    //Initialize the input parameters.
    switch(sampler8x8State.stateType)
    {
        case CM_SAMPLER8X8_AVS:
            pMhwAvs = &(param.sampler8x8State.avs_param.avs_state);
            pCmAvs  = sampler8x8State.avs;
            pMhwAvs->stateID              = (int16_t)-1;
            pMhwAvs->bEnableAVS           = true;
            pMhwAvs->AvsType              = pCmAvs->AVSTYPE;
            pMhwAvs->EightTapAFEnable     = pCmAvs->EightTapAFEnable;
            pMhwAvs->BypassIEF            = pCmAvs->BypassIEF;
            pMhwAvs->GainFactor           = pCmAvs->GainFactor;
            pMhwAvs->GlobalNoiseEstm      = pCmAvs->GlobalNoiseEstm;
            pMhwAvs->StrongEdgeThr        = pCmAvs->StrongEdgeThr;
            pMhwAvs->WeakEdgeThr          = pCmAvs->WeakEdgeThr;
            pMhwAvs->StrongEdgeWght       = pCmAvs->StrongEdgeWght;
            pMhwAvs->RegularWght          = pCmAvs->RegularWght;
            pMhwAvs->NonEdgeWght          = pCmAvs->NonEdgeWght;

            pMhwAvs->bEnableSTDE          = 0;
            pMhwAvs->b8TapAdaptiveEnable  = 0;
            pMhwAvs->bSkinDetailFactor    = 0;
            // current vphal/mhw use HDCDW flag to control shuffleoutputwriteback, we follow them
            pMhwAvs->bHdcDwEnable         = ( pCmAvs->HDCDirectWriteEnable || ( !pCmAvs->ShuffleOutputWriteback ) );
            pMhwAvs->bWritebackStandard = !pCmAvs->ShuffleOutputWriteback;
            pMhwAvs->bEnableIEF           = 0;
            pMhwAvs->wIEFFactor           = 0;
            pMhwAvs->wR3xCoefficient      = pCmAvs->wR3xCoefficient;
            pMhwAvs->wR3cCoefficient      = pCmAvs->wR3cCoefficient;
            pMhwAvs->wR5xCoefficient      = pCmAvs->wR5xCoefficient;
            pMhwAvs->wR5cxCoefficient     = pCmAvs->wR5cxCoefficient;
            pMhwAvs->wR5cCoefficient      = pCmAvs->wR5cCoefficient;

            Sampler8x8CoefficientFormatTransform((PCM_AVS_INTERNEL_NONPIPLINED_STATE )&(param.sampler8x8State.avs_param.avs_table), sampler8x8State.avs->AvsState);
            break;

        case CM_SAMPLER8X8_CONV:
            dst = (void *)&(param.sampler8x8State.convolve_state);
            src = (void *)sampler8x8State.conv;
            CmFastMemCopy( dst, src, sizeof( CM_CONVOLVE_STATE_MSG));
            break;

        case CM_SAMPLER8X8_MISC:
            param.sampler8x8State.misc_state.DW0.Height = sampler8x8State.misc->DW0.Height;
            param.sampler8x8State.misc_state.DW0.Width = sampler8x8State.misc->DW0.Width;
            param.sampler8x8State.misc_state.DW0.Row0 = sampler8x8State.misc->DW0.Row0;
            param.sampler8x8State.misc_state.DW1.Row1 = sampler8x8State.misc->DW1.Row1;
            param.sampler8x8State.misc_state.DW1.Row2 = sampler8x8State.misc->DW1.Row2;
            param.sampler8x8State.misc_state.DW2.Row3 = sampler8x8State.misc->DW2.Row3;
            param.sampler8x8State.misc_state.DW2.Row4 = sampler8x8State.misc->DW2.Row4;
            param.sampler8x8State.misc_state.DW3.Row5 = sampler8x8State.misc->DW3.Row5;
            param.sampler8x8State.misc_state.DW3.Row6 = sampler8x8State.misc->DW3.Row6;
            param.sampler8x8State.misc_state.DW4.Row7 = sampler8x8State.misc->DW4.Row7;
            param.sampler8x8State.misc_state.DW4.Row8 = sampler8x8State.misc->DW4.Row8;
            param.sampler8x8State.misc_state.DW5.Row9 = sampler8x8State.misc->DW5.Row9;
            param.sampler8x8State.misc_state.DW5.Row10 = sampler8x8State.misc->DW5.Row10;
            param.sampler8x8State.misc_state.DW6.Row11 = sampler8x8State.misc->DW6.Row11;
            param.sampler8x8State.misc_state.DW6.Row12 = sampler8x8State.misc->DW6.Row12;
            param.sampler8x8State.misc_state.DW7.Row13 = sampler8x8State.misc->DW7.Row13;
            param.sampler8x8State.misc_state.DW7.Row14 = sampler8x8State.misc->DW7.Row14;

            break;

        default:
            CM_ASSERTMESSAGE("Error: Invalid sampler8x8 state descr.");
            return hr;
    }

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)GetAccelData();

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnRegisterSampler8x8(pCmData->pCmHalState, &param));

    index = param.dwHandle >> 16;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    UnRegister Sampler8x8 State (Not implemented yet)
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::UnregisterSampler8x8State(uint32_t index)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)GetAccelData();

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnUnRegisterSampler8x8(pCmData->pCmHalState, index));

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
CmDeviceRT::CreateThreadGroupSpaceEx(uint32_t thrdSpaceWidth,
                                     uint32_t thrdSpaceHeight,
                                     uint32_t thrdSpaceDepth,
                                     uint32_t grpSpaceWidth,
                                     uint32_t grpSpaceHeight,
                                     uint32_t grpSpaceDepth,
                                     CmThreadGroupSpace*& pTGS)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_ThreadGroupSpace);

    uint32_t firstfreeslot = m_ThreadGroupSpaceArray.GetFirstFreeIndex();
    int32_t result = CmThreadGroupSpace::Create(this, firstfreeslot, thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth, pTGS);
    if (result == CM_SUCCESS)
    {
        m_ThreadGroupSpaceArray.SetElement( firstfreeslot, pTGS );
        m_ThreadGroupSpaceCount ++;
    }
    return result;
}

CM_RT_API int32_t
CmDeviceRT::CreateThreadGroupSpace(uint32_t thrdSpaceWidth,
                                   uint32_t thrdSpaceHeight,
                                   uint32_t grpSpaceWidth,
                                   uint32_t grpSpaceHeight,
                                   CmThreadGroupSpace*& pTGS)
{
    INSERT_API_CALL_LOG();

    int32_t result = CreateThreadGroupSpaceEx(thrdSpaceWidth,
                                              thrdSpaceHeight,
                                              1,
                                              grpSpaceWidth,
                                              grpSpaceHeight,
                                              1,
                                              pTGS);

    return result;
}

CM_RT_API int32_t
CmDeviceRT::DestroyThreadGroupSpace(CmThreadGroupSpace*& pTGS)
{
    INSERT_API_CALL_LOG();

    if( pTGS == nullptr )
    {
        return CM_FAILURE;
    }

    uint32_t indexTGs = pTGS->GetIndexInTGsArray();

    CLock locker(m_CriticalSection_ThreadGroupSpace);

    if(pTGS == static_cast< CmThreadGroupSpace* >(m_ThreadGroupSpaceArray.GetElement( indexTGs )))
    {
        int32_t status = CmThreadGroupSpace::Destroy( pTGS );
        if(status == CM_SUCCESS)
        {
            m_ThreadGroupSpaceArray.SetElement( indexTGs, nullptr );
            pTGS = nullptr;
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
int32_t CmDeviceRT::LoadPredefinedCopyKernel(CmProgram*& pProgram)
{
    PCM_HAL_STATE           pCmHalState;
    int32_t                 hr = CM_SUCCESS;

    pCmHalState = ((PCM_CONTEXT_DATA)GetAccelData())->pCmHalState;

    if(m_pGPUCopyKernelProgram)
    {
        pProgram = m_pGPUCopyKernelProgram;
        return CM_SUCCESS;
    }

    void * pGPUCopy_kernel_isa;
    uint32_t iGPUCopy_kernel_isa_size;

    pCmHalState->pCmHalInterface->GetCopyKernelIsa(pGPUCopy_kernel_isa, iGPUCopy_kernel_isa_size);

    hr = LoadProgram((void *)pGPUCopy_kernel_isa, iGPUCopy_kernel_isa_size, pProgram, "PredefinedGPUKernel");
    if (hr != CM_SUCCESS)
    {
        return hr;
    }

    m_pGPUCopyKernelProgram = pProgram;

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Load Predefined Program, it is used by GPUCopy API
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::LoadPredefinedInitKernel(CmProgram*& pProgram)
{
    PCM_HAL_STATE           pCmHalState;
    int32_t                 hr = CM_SUCCESS;

    pCmHalState = ((PCM_CONTEXT_DATA)GetAccelData())->pCmHalState;

    if(m_pSurfInitKernelProgram)
    {
        pProgram = m_pSurfInitKernelProgram;
        return CM_SUCCESS;
    }

    void * pGPUInit_kernel_isa;
    uint32_t iGPUInit_kernel_isa_size;

    pCmHalState->pCmHalInterface->GetInitKernelIsa(pGPUInit_kernel_isa, iGPUInit_kernel_isa_size);

    hr = LoadProgram((void *)pGPUInit_kernel_isa, iGPUInit_kernel_isa_size, pProgram, "PredefinedGPUKernel");
    if (hr != CM_SUCCESS)
    {
        return hr;
    }

    m_pSurfInitKernelProgram = pProgram;

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Return HW stepping infor, Not implemented yet.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetGenStepInfo(char*& stepinfostr)
{
    PCM_HAL_STATE           pCmHalState;
    int32_t                 hr = CM_SUCCESS;

    pCmHalState = ((PCM_CONTEXT_DATA)GetAccelData())->pCmHalState;

    CHK_MOSSTATUS_RETURN_CMERROR(pCmHalState->pCmHalInterface->GetGenStepInfo(stepinfostr));

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler Surface 2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRT::CreateSamplerSurface2D(CmSurface2D* p2DSurface,
                                   SurfaceIndex* & pSamplerSurfaceIndex)
{
    INSERT_API_CALL_LOG();

    if( ! p2DSurface )  {
        CM_ASSERTMESSAGE("Error: Pointer to sampler surface 2D is null.");
        return CM_NULL_POINTER;
    }

    uint32_t width, height, sizeperpixel;
    CM_SURFACE_FORMAT format;

    CmSurface2DRT* p2DSurfaceRT = static_cast<CmSurface2DRT *>(p2DSurface);
    p2DSurfaceRT->GetSurfaceDesc(width, height, format, sizeperpixel);

    if (!m_pSurfaceMgr->IsSupportedForSamplerSurface2D(format))
    {
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    CLock locker(m_CriticalSection_Surface);

    int32_t result = m_pSurfaceMgr->CreateSamplerSurface( p2DSurfaceRT, pSamplerSurfaceIndex, nullptr);

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler Surface 2D UP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRT::CreateSamplerSurface2DUP(CmSurface2DUP* p2DUPSurface,
                                     SurfaceIndex* & pSamplerSurfaceIndex)
{
    INSERT_API_CALL_LOG();

    if (!p2DUPSurface)
    {
        CM_ASSERTMESSAGE("Error: Pointer to sampler 2D UP is null.");
        return CM_NULL_POINTER;
    }

    uint32_t width, height, sizeperpixel;
    CM_SURFACE_FORMAT format;
    CmSurface2DUPRT *pSurface2DRT = static_cast<CmSurface2DUPRT *>(p2DUPSurface);
    pSurface2DRT->GetSurfaceDesc(width, height, format, sizeperpixel);
    if (!m_pSurfaceMgr->IsSupportedForSamplerSurface2D(format))
    {
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    CLock locker(m_CriticalSection_Surface);

    int32_t result = m_pSurfaceMgr->CreateSamplerSurface(pSurface2DRT, pSamplerSurfaceIndex);

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Sampler Surface 3D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRT::CreateSamplerSurface3D(CmSurface3D* p3DSurface,
                                   SurfaceIndex* & pSamplerSurfaceIndex)
{
    INSERT_API_CALL_LOG();

    if( ! p3DSurface )  {
        CM_ASSERTMESSAGE("Error: Pointer to sampler surface 3D is null.");
        return CM_NULL_POINTER;
    }

    uint32_t width, height, depth;
    CM_SURFACE_FORMAT  format;
    CmSurface3DRT *pSurfaceRT = static_cast<CmSurface3DRT *>(p3DSurface);
    pSurfaceRT->GetProperties(width, height, depth, format);
    switch(format)
    {
        case CM_SURFACE_FORMAT_A8R8G8B8:
        case CM_SURFACE_FORMAT_A16B16G16R16:
            break;

        default:
            CM_ASSERTMESSAGE("Error: Unsupported surface format.");
            return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    CLock locker(m_CriticalSection_Surface);

    int32_t result = m_pSurfaceMgr->CreateSamplerSurface( pSurfaceRT, pSamplerSurfaceIndex);

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Sampler Surface
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t
CmDeviceRT::DestroySamplerSurface(SurfaceIndex* & pSamplerSurfaceIndex)
{
    INSERT_API_CALL_LOG();

    CLock locker(m_CriticalSection_Surface);

    int32_t result = m_pSurfaceMgr->DestroySamplerSurface( pSamplerSurfaceIndex );

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Cm Sampler8x8 pointer
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetSampler8x8(uint32_t index,
                                  CmSampler8x8State_RT *&pSampler8x8)
{
    if (CM_MAX_SAMPLER_TABLE_SIZE < index)
    {
        return CM_EXCEED_SAMPLER_AMOUNT;
    }

    pSampler8x8 = (CmSampler8x8State_RT *)m_Sampler8x8Array.GetElement(index);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set L3 config
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::SetL3Config(const L3ConfigRegisterValues *l3_c)
{
    INSERT_API_CALL_LOG();

    L3ConfigRegisterValues L3Values;

    L3Values = *l3_c;

    SetCaps(CAP_L3_CONFIG, sizeof(L3ConfigRegisterValues), &L3Values);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set L3 suggested config
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::SetSuggestedL3Config(L3_SUGGEST_CONFIG l3_s_c)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr          = CM_SUCCESS;

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)this->GetAccelData();
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pCmHalInterface->SetSuggestedL3Conf(l3_s_c));

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
int32_t CmDeviceRT::SetCaps(CM_DEVICE_CAP_NAME capName,
                            size_t capValueSize,
                            void* pCapValue)
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

            if( *(uint32_t *)pCapValue <= 0 )
            {
                CM_ASSERTMESSAGE("Error: Failed to set caps with CAP_HW_THREAD_COUNT.");
                return CM_INVALID_HARDWARE_THREAD_NUMBER;
            }

            GetCaps(CAP_HW_THREAD_COUNT, size, &maxValue);
            if ( *(uint32_t *)pCapValue > maxValue)
            {
                CM_ASSERTMESSAGE("Error: Failed to set caps with CAP_HW_THREAD_COUNT.");
                return CM_INVALID_HARDWARE_THREAD_NUMBER;
            }

            setCaps.Type = CM_SET_MAX_HW_THREADS;
            setCaps.MaxValue = *(uint32_t *)pCapValue;
            break;

        case CAP_L3_CONFIG:
            if (capValueSize != sizeof(L3ConfigRegisterValues))
            {
                CM_ASSERTMESSAGE("Error: Failed to set caps with CAP_L3_CONFIG.");
                return CM_INVALIDE_L3_CONFIGURATION;
            }
            else
            {
                L3ConfigRegisterValues *l3_c = (L3ConfigRegisterValues *)pCapValue;

                setCaps.ConfigRegsiter0 = l3_c->config_register0;
                setCaps.ConfigRegsiter1 = l3_c->config_register1;
                setCaps.ConfigRegsiter2 = l3_c->config_register2;
                setCaps.ConfigRegsiter3 = l3_c->config_register3;
                setCaps.Type = CM_SET_HW_L3_CONFIG;
            }
            break;

        default:
            CM_ASSERTMESSAGE("Error: Invalid cap name.");
            return CM_INVALID_CAP_NAME;
    }

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)this->GetAccelData();
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnSetCaps(pCmData->pCmHalState, (PCM_HAL_MAX_SET_CAPS_PARAM)&setCaps));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register the Sync Event
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::RegisterSyncEvent(void *SyncEventHandle)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    CM_HAL_OSSYNC_PARAM SyncParam;
    SyncParam.iOSSyncEvent = SyncEventHandle;

    PCM_CONTEXT_DATA  pCmData = (PCM_CONTEXT_DATA)GetAccelData();
    PCM_HAL_STATE  pCmHalState = pCmData->pCmHalState;
    // Call HAL layer to wait for Task finished with event-driven mechanism
    CHK_MOSSTATUS_RETURN_CMERROR(pCmHalState->pfnRegisterKMDNotifyEventHandle(pCmHalState, &SyncParam));

    m_OSSyncEvent = SyncParam.iOSSyncEvent;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Sync Event
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetOSSyncEventHandle(void *& hOSSyncEvent)
{
    hOSSyncEvent = m_OSSyncEvent;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create print buffer to support print in cm kernel
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::InitPrintBuffer(size_t printbufsize)
{
    INSERT_API_CALL_LOG();

    if (m_pPrintBufferUP)
    {
        if (printbufsize == m_PrintBufferSize)
        {
            //Reuse existing buffer up
            return CM_SUCCESS;
        }
        else
        {
            // Free the existing one first
            DestroyBufferUP(m_pPrintBufferUP);
            MOS_AlignedFreeMemory(m_pPrintBufferMem);
        }
    }

    /// Allocate and Initialize host memory.
    m_PrintBufferSize = printbufsize;
    m_pPrintBufferMem = (uint8_t*)MOS_AlignedAllocMemory(m_PrintBufferSize, 0x1000); //PAGE SIZE
    if(!m_pPrintBufferMem)
    {
        return CM_OUT_OF_HOST_MEMORY;
    }

    CmSafeMemSet(m_pPrintBufferMem, 0, m_PrintBufferSize);
    *(unsigned int*)m_pPrintBufferMem = PRINT_BUFFER_HEADER_SIZE;

    /// Allocate device memory and MemCopy from host to device.
    int32_t result = CreateBufferUP((uint32_t)m_PrintBufferSize, m_pPrintBufferMem, m_pPrintBufferUP);
    if (result != CM_SUCCESS || m_pPrintBufferUP == nullptr)
    {
        m_IsPrintEnable = false;
        MOS_AlignedFreeMemory(m_pPrintBufferMem);
        return result;
    }
    m_pPrintBufferUP->GetIndex(m_PrintBufferIndex);
    m_IsPrintEnable = true;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get print buffer memory
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetPrintBufferMem(unsigned char * &pPrintBufferMem) const
{
    pPrintBufferMem = m_pPrintBufferMem;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the print buffer's surface index
//| Returns:    The print buffer's surface index
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetPrintBufferIndex(SurfaceIndex *& pIndex) const
{
    pIndex = m_PrintBufferIndex;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether the kernel print is enabled
//| Returns:    Whether the kernel print is enabled.
//*-----------------------------------------------------------------------------
bool CmDeviceRT::IsPrintEnable() const
{
     return m_IsPrintEnable;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Clear print buffer
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::ClearPrintBuffer()
{
    //clean memory
    CmSafeMemSet(m_pPrintBufferMem, 0, m_PrintBufferSize);
    *(unsigned int*)m_pPrintBufferMem = PRINT_BUFFER_HEADER_SIZE;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether MDF ETW Log On.
//| Returns:    Whether MDF ETW Log On.
//*-----------------------------------------------------------------------------
bool CmDeviceRT::IsVtuneLogOn() const
{
     return m_bVtuneOn;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Surf2D LookUP Entry
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::GetSurf2DLookUpEntry(uint32_t index,
                                         PCMLOOKUP_ENTRY &pLookupEntry)
{
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)GetAccelData();
    if(pCmData)
    {
        pLookupEntry = &(pCmData->pCmHalState->pSurf2DTable[index]);
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
CM_RT_API int32_t CmDeviceRT::CreateVebox(CmVebox* & pVebox) //HSW
{
    CLock locker(m_CriticalSection_Vebox);

    uint32_t firstfreeslot = m_VeboxArray.GetFirstFreeIndex();
    CmVeboxRT *pVeboxRT = nullptr;
    int32_t result = CmVeboxRT::Create(this, firstfreeslot, pVeboxRT);
    if (result == CM_SUCCESS)
    {
        m_VeboxArray.SetElement(firstfreeslot, pVeboxRT);
        m_VeboxCount++;
    }
    pVebox = pVeboxRT;
    return result;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy vebox task
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::DestroyVebox(CmVebox* & pVebox) //HSW
{
    if (pVebox == nullptr)
    {
        return CM_NULL_POINTER;
    }

    CmVeboxRT *pVeboxRT = static_cast<CmVeboxRT *>(pVebox);
    uint32_t index = pVeboxRT->GetIndexInVeboxArray();

    if (pVeboxRT == m_VeboxArray.GetElement(index))
    {
        int32_t status = CmVeboxRT::Destroy(pVeboxRT);
        if (status == CM_SUCCESS)
        {
            m_VeboxArray.SetElement(index, nullptr);
            pVebox = nullptr;
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

//*-----------------------------------------------------------------------------
//| Purpose:    Load program from memory with user provided Gen binary
//| Arguments :
//|               pCISACode         [in]       pointer to memory where common isa locates
//|               uiCISACodeSize    [in]       size of common isa
//|               pGenCode          [in]       pointer to memory where user provided Gen binary locates
//|               uiGenCodeSize     [in]       size of user provided Gen binary
//|               pProgram          [in/out]   Pointer to CmProgram
//|               options           [in]       options : non-jitter,jitter
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::LoadProgramWithGenCode(void* pCISACode,
                                           const uint32_t uiCISACodeSize,
                                           void* pGenCode,
                                           const uint32_t uiGenCodeSize,
                                           CmProgram*& pProgram,
                                           const char* options)
{
    int32_t result;
    CLock locker(m_CriticalSection_Program_Kernel);

    uint32_t firstfreeslot = m_ProgramArray.GetFirstFreeIndex();

    CmProgramRT *pProgramRT = static_cast<CmProgramRT *>(pProgram);
    result = CmProgramRT::Create( this, pCISACode, uiCISACodeSize, pGenCode, uiGenCodeSize, pProgramRT, options, firstfreeslot );
    if( result == CM_SUCCESS )
    {
        m_ProgramArray.SetElement( firstfreeslot, pProgramRT );
        m_ProgramCount ++;
    }
    pProgram = pProgramRT;
    return result;
}

int32_t CmDeviceRT::DestroySurfaceInPool(uint32_t &freeSurfNum)
{
    CLock locker(m_CriticalSection_Surface);

    freeSurfNum = m_pSurfaceMgr->TouchSurfaceInPoolForDestroy();
    if ((int32_t)freeSurfNum < 0)
    {
        freeSurfNum = 0;
        return CM_FAILURE;
    }

    return CM_SUCCESS;
}

CM_RT_API int32_t CmDeviceRT::CreateBufferSVM(uint32_t size,
                                              void* & pSysMem,
                                              uint32_t access_flag,
                                              CmBufferSVM* & pBufferSVM)
{
    INSERT_API_CALL_LOG();

    bool    isCMRTAllocatedSVMBuffer = true;

    //SVM buffer is going to stateless access, no size restriction lik CmBuffer and CmBufferUP
    if( size == 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid buffer width.");
        return CM_INVALID_WIDTH;
    }

    if ( pSysMem )
    {
        if ((uintptr_t)pSysMem & CM_PAGE_ALIGNMENT_MASK)
        {
            CM_ASSERTMESSAGE("Error: System memory is not page aligned.");
            return CM_SYSTEM_MEMORY_NOT_4KPAGE_ALIGNED;
        }
        isCMRTAllocatedSVMBuffer = false;
    }
    else //Allocate a 4K page aligned memory
    {
        pSysMem = MOS_AlignedAllocMemory(size, CM_PAGE_ALIGNMENT);

        if (!pSysMem)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_FAILED_TO_ALLOCATE_SVM_BUFFER;
        }
        isCMRTAllocatedSVMBuffer = true;
    }

    CLock locker(m_CriticalSection_Surface);

    CmBuffer_RT* p = nullptr;
    int result = m_pSurfaceMgr->CreateBuffer( size, CM_BUFFER_SVM, isCMRTAllocatedSVMBuffer, p, nullptr, pSysMem, false, CM_DEFAULT_COMPARISON_VALUE );
    pBufferSVM = static_cast< CmBufferSVM* >(p);

    return result;
}

CM_RT_API int32_t CmDeviceRT::DestroyBufferSVM(CmBufferSVM* & pBufferSVM)
{

    INSERT_API_CALL_LOG();

    CmBuffer_RT* temp = static_cast< CmBuffer_RT* >(pBufferSVM);

    CLock locker(m_CriticalSection_Surface);

    int32_t status = m_pSurfaceMgr->DestroySurface(temp, APP_DESTROY);

    if (status != CM_FAILURE) //CM_SURFACE_IN_USE, or  CM_SURFACE_CACHED may be returned, which should be treated as SUCCESS.
    {
        pBufferSVM = nullptr;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}


//*-----------------------------------------------------------------------------
//| Purpose:    Creates an alias to CmSurface2D, p2DSurface
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateSurface2DAlias(CmSurface2D* p2DSurface,
                                                   SurfaceIndex* &pAliasIndex)
{
    INSERT_API_CALL_LOG();

    int32_t result = CM_SUCCESS;

    CLock locker(m_CriticalSection_Surface);
    if( !p2DSurface )
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface 2D is null.");
        return CM_NULL_POINTER;
    }

    CmSurface2DRT *pSurfaceRT = static_cast<CmSurface2DRT *>(p2DSurface);
    result = pSurfaceRT->Create2DAlias(pAliasIndex);
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Failed to create surface 2D alias.");
        return result;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Creates an alias to CmBuffer, pBuffer
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::CreateBufferAlias(CmBuffer *pBuffer,
                                                SurfaceIndex* &pAliasIndex)
{
    INSERT_API_CALL_LOG();

    int32_t result = CM_SUCCESS;

    CLock locker(m_CriticalSection_Surface);
    if( !pBuffer )
    {
        CM_ASSERTMESSAGE("Error: Pointer to CmBuffer is null.");
        return CM_NULL_POINTER;
    }

    CmBuffer_RT *pBuffer_RT = static_cast<CmBuffer_RT *>(pBuffer);

    result = pBuffer_RT->CreateBufferAlias(pAliasIndex);
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
int32_t CmDeviceRT::InitDevCreateOption(CM_HAL_CREATE_PARAM & DevCreateParam,
                                        uint32_t DevCreateOption)
{
    uint32_t MaxTaskNumber =0;
    uint32_t KernelBinarySizeInGSH = 0;

    //Flag to disable scratch space
    DevCreateParam.DisableScratchSpace = (DevCreateOption & CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_MASK);

    //Calculate Scratch Space
    if(DevCreateParam.DisableScratchSpace)
    {
        DevCreateParam.ScratchSpaceSize = 0;
    }
    else
    {
        //Max Scratch Space Size [1:3] of DevCreateOption
        DevCreateParam.ScratchSpaceSize = (DevCreateOption & CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_MASK) >> CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_OFFSET;
    }

    //Flag to Disable preemption
    DevCreateParam.bDisabledMidThreadPreemption = ((DevCreateOption  & CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_DISENABLE) >> CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_OFFSET)? true: false;

    //flag to enable kernel debug, so, SIP binary can be created during
    DevCreateParam.bEnabledKernelDebug = ((DevCreateOption  & CM_DEVICE_CONFIG_KERNEL_DEBUG_ENABLE) >> CM_DEVICE_CONFIG_KERNEL_DEBUG_OFFSET)? true: false;

    //Calculate Task Number [4:5] of DevCreateOption   [00]:4 ; [01]:8 ; [10]:12; [11]:16
    MaxTaskNumber = (DevCreateOption & CM_DEVICE_CONFIG_TASK_NUM_MASK) >> CM_DEVICE_CONFIG_TASK_NUM_OFFSET;

    DevCreateParam.MaxTaskNumber = (MaxTaskNumber + 1) * CM_DEVICE_CONFIG_TASK_NUM_STEP;

    // [9:8] Added bits to increase maximum task number. Value plus one is multiplied by value calculated from bits [5:4].
    // [00]:1; [01]:2; [10]:3; [11]:4
    MaxTaskNumber = (DevCreateOption & CM_DEVICE_CONFIG_EXTRA_TASK_NUM_MASK ) >> CM_DEVICE_CONFIG_EXTRA_TASK_NUM_OFFSET;

    DevCreateParam.MaxTaskNumber = (MaxTaskNumber + 1) * DevCreateParam.MaxTaskNumber;

    // [10] request slice shutdown
    DevCreateParam.bRequestSliceShutdown = (DevCreateOption & CM_DEVICE_CONFIG_SLICESHUTDOWN_ENABLE ) ? true:false;

    // [12] request custom gpu context
    DevCreateParam.bRequestCustomGpuContext = (DevCreateOption & CM_DEVICE_CONFIG_GPUCONTEXT_ENABLE) ? true:false;

    // [20:13] calculate size in GSH reserved for kernel binary
    KernelBinarySizeInGSH = (DevCreateOption & CM_DEVICE_CONFIG_KERNELBINARYGSH_MASK) >> CM_DEVICE_CONFIG_KERNELBINARYGSH_OFFSET;

    if (KernelBinarySizeInGSH == 0)
        KernelBinarySizeInGSH = 1;

    KernelBinarySizeInGSH = KernelBinarySizeInGSH * CM_KERNELBINARY_BLOCKSIZE_2MB;
    DevCreateParam.KernelBinarySizeinGSH = KernelBinarySizeInGSH;

#if USE_EXTENSION_CODE
    // [31] mock runtime
    DevCreateParam.bMockRuntimeEnabled = (DevCreateOption & CM_DEVICE_CONFIG_MOCK_RUNTIME_ENABLE) ? true : false;
    m_bIsMockRuntime = DevCreateParam.bMockRuntimeEnabled;
#endif

    return CM_SUCCESS;
}


bool CmDeviceRT::IsScratchSpaceDisabled()
{
    return m_DevCreateOption.DisableScratchSpace ? true : false;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets surface array size, needed to assign alias surface index
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::SetSurfaceArraySizeForAlias()
{
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)this->GetAccelData();
    m_pSurfaceMgr->GetSurfaceArraySize(pCmData->pCmHalState->nSurfaceArraySize);
    return CM_SUCCESS;
}

#if CM_LOG_ON
std::string CmDeviceRT::Log()
{
    std::ostringstream  oss;

    uint32_t nSize = sizeof(int);

    GetCaps( CAP_GPU_CURRENT_FREQUENCY, nSize, &m_nGPUFreqOriginal );
    GetCaps( CAP_MIN_FREQUENCY, nSize, &m_nGPUFreqMin );
    GetCaps( CAP_MAX_FREQUENCY, nSize, &m_nGPUFreqMax );

    int GTInfo;
    GetCaps( CAP_GT_PLATFORM,   nSize, &GTInfo        );

    oss << "Device Creation "<<std::endl;

    // Hw Information
    oss << "Platform :" << m_Platform << std::endl;
    oss << "GT Info :"<< GTInfo << std::endl;
    oss << "Frequency Max:" << m_nGPUFreqMax << " Min:" <<m_nGPUFreqMin
        << " Current:"<< m_nGPUFreqOriginal << std::endl;

    oss << "Device DDI Version :" << m_DDIVersion << std::endl;
    oss << "Max Tasks " << m_HalMaxValues.iMaxTasks << std::endl;
    oss << "Max HW Threads " << m_HalMaxValues.iMaxHwThreads<< std::endl;
    oss << "Max Args Per Kernel   " << m_HalMaxValues.iMaxArgsPerKernel << std::endl;
    oss << "Max 2D Surface Table Size " << m_HalMaxValues.iMax2DSurfaceTableSize << std::endl;
    oss << "Max Buffer Table Size " << m_HalMaxValues.iMaxBufferTableSize << std::endl;
    oss << "Max Threads per Task  " << m_HalMaxValues.iMaxUserThreadsPerTask << std::endl;
    oss << "Max Threads Per Task no Thread Arg " << m_HalMaxValues.iMaxUserThreadsPerTaskNoThreadArg << std::endl;

    return oss.str();
}
#endif

#if !(USE_EXTENSION_CODE)
bool CmDeviceRT::CheckGTPinEnabled( )
{
    return false;
}
#endif

//*-----------------------------------------------------------------------------
//| Purpose:    Internal function to flush print buffer on stdout or file.
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
int32_t CmDeviceRT::FlushPrintBufferInternal(const char *filename)
{
#if CM_KERNEL_PRINTF_ON
    FILE * pStreamOutFile = nullptr;

    if (filename == nullptr)
    {
        pStreamOutFile = stdout;
    }
    else
    {
        int err = MOS_SecureFileOpen(&pStreamOutFile, filename, "wb");
        if (err || pStreamOutFile == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Failed to open kernel print dump file.");
            return CM_FAILURE;
        }
    }

    if( m_pPrintBufferMem == nullptr ||
        m_PrintBufferSize == 0 ||
        m_IsPrintEnable == false)
    {
        CM_ASSERTMESSAGE("Error: Print buffer is not initialized.");
        if (filename && pStreamOutFile)
            fclose(pStreamOutFile);
        return CM_FAILURE;
    }

    //Dump memory on the screen.
    DumpAllThreadOutput(pStreamOutFile, m_pPrintBufferMem, m_PrintBufferSize);

    //Flush and close stream
    fflush(pStreamOutFile);
    if (filename && pStreamOutFile)
    {
        fclose(pStreamOutFile);
        pStreamOutFile = nullptr;
    }

    //clean memory
    CmSafeMemSet(m_pPrintBufferMem, 0, m_PrintBufferSize);
    *(unsigned int*)m_pPrintBufferMem = sizeof(CM_PRINT_HEADER);

    return CM_SUCCESS;
#else
    return CM_NOT_IMPLEMENTED;
#endif
}

//*-----------------------------------------------------------------------------
//| Purpose:    Dump print buffer. Only Avaliable in Release-internal and Debug Mode
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::FlushPrintBuffer()
{
    return FlushPrintBufferInternal(nullptr);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Dump print buffer to file. Only Avaliable in Release-internal and Debug Mode
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmDeviceRT::FlushPrintBufferIntoFile(const char *filename)
{
    return FlushPrintBufferInternal(filename);
}

CM_RT_API int32_t
CmDeviceRT::CreateHevcVmeSurfaceG10(CmSurface2D * pCurSurface,
                                    CmSurface2D ** pForwardSurface,
                                    CmSurface2D ** pBackwardSurface,
                                    const uint32_t surfaceCountForward,
                                    const uint32_t surfaceCountBackward,
                                    SurfaceIndex *& pVmeIndex)
{
    INSERT_API_CALL_LOG();

    if ( pCurSurface == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
        return CM_NULL_POINTER;
    }

    if (surfaceCountForward > CM_NUM_VME_HEVC_REFS || surfaceCountBackward > CM_NUM_VME_HEVC_REFS)
    {
        CM_ASSERTMESSAGE("Error: Invalid count of forward or backward surfaces.");
        return CM_INVALID_ARG_VALUE;
    }

    CmSurface2DRT *pCurrentRT = static_cast< CmSurface2DRT * >( pCurSurface );
    CmSurface2DRT** pForwardSurfArray = nullptr;
    CmSurface2DRT** pBackwardSurfArray = nullptr;

    pForwardSurfArray = MOS_NewArray(CmSurface2DRT*, CM_NUM_VME_HEVC_REFS);
    if ( pForwardSurfArray == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Failed to create forward surface array.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    if ( pForwardSurface != nullptr )
    {
        for ( uint32_t i = 0; i< surfaceCountForward; i++ )
        {
            pForwardSurfArray[ i ] = static_cast< CmSurface2DRT * >( pForwardSurface[ i ] );
            if ( pForwardSurfArray[ i ] == nullptr )
            {
                CM_ASSERTMESSAGE("Error: Invalid forward surface array.");
                MosSafeDeleteArray( pForwardSurfArray );
                return CM_INVALID_ARG_VALUE;
            }
        }
        for ( uint32_t i = surfaceCountForward; i < CM_NUM_VME_HEVC_REFS; i++ )
        {
            pForwardSurfArray[ i ] = static_cast< CmSurface2DRT * >( pForwardSurface[ 0 ] );
        }
    }
    else
    {
        for ( uint32_t i = 0; i < CM_NUM_VME_HEVC_REFS; i++ )
        {
            pForwardSurfArray[ i ] = pCurrentRT;
        }
    }

    pBackwardSurfArray = MOS_NewArray(CmSurface2DRT*, CM_NUM_VME_HEVC_REFS);
    if ( pBackwardSurfArray == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Failed to create backward surface array.");
        MosSafeDeleteArray( pForwardSurfArray );
        return CM_OUT_OF_HOST_MEMORY;
    }
    if ( pBackwardSurface != nullptr )
    {
        for ( uint32_t i = 0; i< surfaceCountBackward; i++ )
        {
            pBackwardSurfArray[ i ] = static_cast< CmSurface2DRT * >( pBackwardSurface[ i ] );
            if ( pBackwardSurfArray[ i ] == nullptr )
            {
                CM_ASSERTMESSAGE("Error: Invalid backward surface array.");
                MosSafeDeleteArray( pForwardSurfArray );
                MosSafeDeleteArray( pBackwardSurfArray );
                return CM_INVALID_ARG_VALUE;
            }
        }
        for ( uint32_t i = surfaceCountBackward; i < CM_NUM_VME_HEVC_REFS; i++ )
        {
            pBackwardSurfArray[ i ] = static_cast< CmSurface2DRT * >( pBackwardSurface[ 0 ] );
        }
    }
    else
    {
        for ( uint32_t i = 0; i < CM_NUM_VME_HEVC_REFS; i++ )
        {
            pBackwardSurfArray[ i ] = pCurrentRT;
        }
    }

    int32_t result = m_pSurfaceMgr->CreateVmeSurface( pCurrentRT, pForwardSurfArray, pBackwardSurfArray, surfaceCountForward, surfaceCountBackward, pVmeIndex );

    if ( FAILED( result ) )
    {
        CM_ASSERTMESSAGE("Error: Failed to create HEVC VME surface.");
    }

    MosSafeDeleteArray( pForwardSurfArray );
    MosSafeDeleteArray( pBackwardSurfArray );

    return result;
}

CM_RT_API int32_t
CmDeviceRT::DestroyHevcVmeSurfaceG10(SurfaceIndex *& pVmeIndex)
{
    INSERT_API_CALL_LOG();
    return DestroyVmeSurface( pVmeIndex );
}

CM_RT_API int32_t CmDeviceRT::CloneKernel(CmKernel* &pKernelDest,
                                          CmKernel *pKernelSrc)
{
    INSERT_API_CALL_LOG();

    int32_t hr = CM_SUCCESS;

    if (pKernelSrc == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to src kernel is null");
        return CM_NULL_POINTER;
    }

    CmKernelRT *pKernelSrcRT = static_cast<CmKernelRT *>(pKernelSrc);
    CmKernelRT *pKernelDestRT = static_cast<CmKernelRT *>(pKernelDest);
    hr = pKernelSrcRT->CloneKernel(pKernelDestRT, m_KernelCount);
    pKernelDest = pKernelDestRT;

    return hr;
}

CmDynamicArray* CmDeviceRT::GetKernelArray()
{
    return &m_KernelArray;
}

uint32_t *CmDeviceRT::GetKernelCount()
{
    return &m_KernelCount;
}

int32_t CmDeviceRT::DestroyVmeSurface(SurfaceIndex *& pVmeIndex)
{
    CLock locker( m_CriticalSection_Surface );

    int32_t result = m_pSurfaceMgr->DestroyVmeSurface( pVmeIndex );

    return result;
}

int32_t CmDeviceRT::GetVISAVersion(uint32_t& majorVersion,
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
}  // namespace
