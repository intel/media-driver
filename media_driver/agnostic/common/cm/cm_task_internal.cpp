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
//! \file      cm_task_internal.cpp 
//! \brief     Contains Class CmTaskInternal  definitions 
//!

#include "cm_task_internal.h"

#include "cm_kernel_rt.h"
#include "cm_mem.h"
#include "cm_event_rt.h"
#include "cm_device_rt.h"
#include "cm_kernel_data.h"
#include "cm_thread_space_rt.h"
#include "cm_group_space.h"
#include "cm_vebox_rt.h"
#include "cm_vebox_data.h"
#include "cm_queue_rt.h"
#include "cm_surface_manager.h"
#include "cm_buffer_rt.h"
#include "cm_surface_2d_rt.h"
#include "cm_surface_2d_up_rt.h"
#include "cm_surface_3d_rt.h"
#include "cm_surface_vme.h"
#include "cm_surface_sampler.h"
#include "cm_surface_sampler8x8.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Task internal
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Create(const uint32_t kernelCount, const uint32_t totalThreadCount,
                               CmKernelRT* kernelArray[], const CmThreadSpaceRT* threadSpace,
                               CmDeviceRT* device, const uint64_t syncBitmap, CmTaskInternal*& task,
                               const uint64_t conditionalEndBitmap,
                               PCM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo)
{
    int32_t result = CM_SUCCESS;
    task = new (std::nothrow) CmTaskInternal(kernelCount, totalThreadCount, kernelArray, device,
                                             syncBitmap, conditionalEndBitmap, conditionalEndInfo,
                                             nullptr);
    if( task )
    {
        result = task->Initialize(threadSpace, false);
        if( result != CM_SUCCESS )
        {
            CmTaskInternal::Destroy( task);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmTaskInternal due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Task internal with Thread Group Space
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Create( const uint32_t kernelCount, const uint32_t totalThreadCount,
                               CmKernelRT* kernelArray[], const CmThreadGroupSpace* threadGroupSpace,
                               CmDeviceRT* device, const uint64_t syncBitmap, CmTaskInternal*& task,
                               const uint64_t conditionalEndBitmap,
                               PCM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo,
                               const CM_EXECUTION_CONFIG* krnExecCfg)
{
    int32_t result = CM_SUCCESS;
    task = new (std::nothrow) CmTaskInternal(kernelCount, totalThreadCount, kernelArray, device,
                                             syncBitmap, conditionalEndBitmap, conditionalEndInfo,
                                             krnExecCfg);

    if( task )
    {
        result = task->Initialize(threadGroupSpace);
        if( result != CM_SUCCESS )
        {
            CmTaskInternal::Destroy( task);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmTaskInternal due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

int32_t CmTaskInternal::Create( CmDeviceRT* device, CmVeboxRT* vebox, CmTaskInternal*& task )
{
    int32_t result = CM_SUCCESS;
    task = new (std::nothrow) CmTaskInternal(0, 0, nullptr, device, CM_NO_KERNEL_SYNC,
                                             CM_NO_CONDITIONAL_END, nullptr, nullptr);
    if( task )
    {
        result = task->Initialize(vebox);
        if( result != CM_SUCCESS )
        {
            CmTaskInternal::Destroy( task);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmTaskInternal due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Task internal with hints
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Create(const uint32_t kernelCount, const uint32_t totalThreadCount,
                               CmKernelRT* kernelArray[], CmTaskInternal*& task,
                               uint32_t numGeneratedTasks, bool isLastTask, uint32_t hints,
                               CmDeviceRT* device)
{
    int32_t result = CM_SUCCESS;
    task = new (std::nothrow) CmTaskInternal(kernelCount, totalThreadCount, kernelArray, device,
                                             CM_NO_KERNEL_SYNC, CM_NO_CONDITIONAL_END, nullptr, nullptr);
    if ( task )
    {
        result = task->Initialize(hints, numGeneratedTasks, isLastTask);
        if ( result != CM_SUCCESS )
        {
            CmTaskInternal::Destroy( task );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmTaskInternal due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Task internal
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Destroy( CmTaskInternal* &task )
{
    CmSafeDelete( task );
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of  CmTaskInternal
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmTaskInternal::CmTaskInternal(const uint32_t kernelCount, const uint32_t totalThreadCount,
                               CmKernelRT* kernelArray[], CmDeviceRT* device,
                               const uint64_t syncBitmap, const uint64_t conditionalEndBitmap,
                               PCM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo,
                               const CM_EXECUTION_CONFIG* krnExecCfg) :
    m_kernels( kernelCount ),
    m_kernelData( kernelCount ),
    m_kernelCount( kernelCount ),
    m_totalThreadCount(totalThreadCount),
    m_taskEvent( nullptr ),
    m_isThreadSpaceCreated(false),
    m_isThreadCoordinatesExisted(false),
    m_threadSpaceWidth(0),
    m_threadSpaceHeight(0),
    m_threadSpaceDepth(0),
    m_threadCoordinates(nullptr),
    m_dependencyPattern(CM_NONE_DEPENDENCY),
    m_walkingPattern(CM_WALK_DEFAULT),
    m_mediaWalkerParamsSet( false ),
    m_dependencyVectorsSet( false ),
    m_dependencyMasks( nullptr ),
    m_mediaWalkerGroupSelect(CM_MW_GROUP_NONE),
    m_isThreadGroupSpaceCreated(false),
    m_groupSpaceWidth(0),
    m_groupSpaceHeight(0),
    m_groupSpaceDepth(0),
    m_slmSize(0),
    m_spillMemUsed(0),
    m_colorCountMinusOne( 0 ),
    m_hints(0),
    m_numTasksGenerated( 0 ),
    m_isLastTask( false ),
    m_ui64SyncBitmap (syncBitmap ),
    m_ui64ConditionalEndBitmap(conditionalEndBitmap),
    m_cmDevice( device ),
    m_surfaceArray (nullptr),
    m_isSurfaceUpdateDone(false),
    m_taskType(CM_TASK_TYPE_DEFAULT),
    m_mediaStatePtr( nullptr )
{
    m_kernelSurfInfo.kernelNum = 0;
    m_kernelSurfInfo.surfEntryInfosArray = nullptr;
    m_kernelCurbeOffsetArray = MOS_NewArray(uint32_t, kernelCount);
    CM_ASSERT(m_kernelCurbeOffsetArray != nullptr);

    for( uint32_t i = 0 ; i < kernelCount; i ++ )
    {
        m_kernels.SetElement( i, kernelArray[ i ] );
        m_kernelData.SetElement( i, nullptr );
    }

    CmSafeMemSet( &m_walkingParameters, 0, sizeof(m_walkingParameters));
    CmSafeMemSet( &m_dependencyVectors, 0, sizeof(m_dependencyVectors));
    CmSafeMemSet( &m_taskConfig, 0, sizeof(m_taskConfig));
    if ( m_kernelCurbeOffsetArray != nullptr )
    {
        CmSafeMemSet( m_kernelCurbeOffsetArray, 0, sizeof(uint32_t) * kernelCount );
    }

    CmSafeMemSet(&m_taskProfilingInfo, 0, sizeof(m_taskProfilingInfo));

    if (conditionalEndInfo != nullptr)
    {
        CmSafeMemCopy(&m_conditionalEndInfo, conditionalEndInfo, sizeof(m_conditionalEndInfo));
    }
    else
    {
        CmSafeMemSet(&m_conditionalEndInfo, 0, sizeof(m_conditionalEndInfo));
    }

    CmSafeMemSet(&m_veboxParam, 0, sizeof(m_veboxParam));
    CmSafeMemSet(&m_veboxState, 0, sizeof(m_veboxState));
    CmSafeMemSet(&m_veboxSurfaceData, 0, sizeof(m_veboxSurfaceData));
    CmSafeMemSet(&m_powerOption, 0, sizeof(m_powerOption));

    if (krnExecCfg != nullptr)
    {
        CmSafeMemCopy(&m_krnExecCfg, krnExecCfg, sizeof(m_krnExecCfg));
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of  CmTaskInternal
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmTaskInternal::~CmTaskInternal( void )
{

    //Write Event Infos
    VtuneWriteEventInfo();

    //Release Profiling Info
    VtuneReleaseProfilingInfo();

    for( uint32_t i = 0; i < m_kernelCount; i ++ )
    {
        CmKernelRT *kernel = (CmKernelRT*)m_kernels.GetElement(i);
        CmKernelData* kernelData = (CmKernelData*)m_kernelData.GetElement( i );
        if(kernel && kernelData)
        {
           kernel->ReleaseKernelData(kernelData);
           CmKernel *kernelBase = kernel;
           m_cmDevice->DestroyKernel(kernelBase);
        }
    }
    m_kernelData.Delete();
    m_kernels.Delete();

    MosSafeDeleteArray(m_kernelCurbeOffsetArray);

    if( m_taskEvent )
    {
        CmEvent *eventBase = m_taskEvent;
        CmQueueRT *cmQueue = nullptr;
        m_taskEvent->GetQueue(cmQueue);
        if (cmQueue)
        {
            cmQueue->DestroyEvent(eventBase); // need to update the m_EventArray
        }
    }

    if(m_threadCoordinates){
        for (uint32_t i=0; i<m_kernelCount; i++)
        {
            if (m_threadCoordinates[i])
            {
                MosSafeDeleteArray(m_threadCoordinates[i]);
            }
        }
        MosSafeDeleteArray( m_threadCoordinates );
    }

    if( m_dependencyMasks )
    {
        for( uint32_t i = 0; i < m_kernelCount; ++i )
        {
            MosSafeDeleteArray(m_dependencyMasks[i]);
        }
        MosSafeDeleteArray( m_dependencyMasks );
    }

    if((m_kernelSurfInfo.kernelNum != 0)&&(m_kernelSurfInfo.surfEntryInfosArray != nullptr))
    {
        ClearKernelSurfInfo();
    }

    MosSafeDeleteArray(m_surfaceArray);

}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Class  CmTaskInternal
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Initialize(const CmThreadSpaceRT* threadSpace, bool isWithHints)
{
    uint32_t totalCurbeSize             = 0;
    uint32_t surfacePoolSize            = 0;
    uint32_t totalKernelBinarySize      = 0;
    uint32_t kernelCurbeSize            = 0;
    uint32_t kernelPayloadSize          = 0;
    CmSurfaceManager* surfaceMgr = nullptr;
    int32_t result              = CM_SUCCESS;
    CM_HAL_MAX_VALUES* halMaxValues = nullptr;
    CM_HAL_MAX_VALUES_EX* halMaxValuesEx = nullptr;
    m_cmDevice->GetHalMaxValues( halMaxValues, halMaxValuesEx );

    if (m_cmDevice->IsPrintEnable())
    {
        SurfaceIndex *printBufferIndex = nullptr;
        m_cmDevice->GetPrintBufferIndex(printBufferIndex);
        CM_ASSERT(printBufferIndex);
        for (uint32_t i = 0; i < m_kernelCount; i++)
        {
            CmKernelRT* kernel = (CmKernelRT*)m_kernels.GetElement(i);
            if(kernel == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
                return CM_FAILURE;
            }
            if(FAILED(kernel->SetStaticBuffer(CM_PRINTF_STATIC_BUFFER_ID, printBufferIndex)))
            {
                CM_ASSERTMESSAGE("Error: Failed to set static buffer.");
                return CM_FAILURE;
            }
        }
    }

    m_cmDevice->GetSurfaceManager( surfaceMgr );
    CM_CHK_NULL_RETURN_CMERROR(surfaceMgr);
    surfacePoolSize = surfaceMgr->GetSurfacePoolSize();

    m_surfaceArray = MOS_NewArray(bool, surfacePoolSize);
    if (!m_surfaceArray)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_FAILURE;
    }
    CmSafeMemSet( m_surfaceArray, 0, surfacePoolSize * sizeof( bool ) );

    for( uint32_t i = 0; i < m_kernelCount; i ++ )
    {

        CmKernelRT* kernel = (CmKernelRT*)m_kernels.GetElement( i );
        if(kernel == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
            return CM_FAILURE;
        }

        uint32_t totalSize =  0;
        CmKernelData* kernelData = nullptr;

        if ( isWithHints )
        {
            CmThreadSpaceRT* kernelThreadSpace = nullptr;
            kernel->GetThreadSpace(kernelThreadSpace);
            if( kernelThreadSpace )
            {
                for(uint32_t j = i; j > 0; --j)
                {
                    uint32_t width, height, myAdjY;
                    CmKernelRT* tmpKernel = (CmKernelRT*)m_kernels.GetElement( j-1 );
                    if( !tmpKernel )
                    {
                        CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
                        return CM_FAILURE;
                    }
                    tmpKernel->GetThreadSpace(kernelThreadSpace);
                    kernelThreadSpace->GetThreadSpaceSize(width, height);
                    myAdjY = kernel->GetAdjustedYCoord();
                    kernel->SetAdjustedYCoord(myAdjY + height);
                }
            }
        }
        
        if (threadSpace == nullptr)
        {
            CmThreadSpaceRT* kernelThreadSpace = nullptr;
            kernel->GetThreadSpace(kernelThreadSpace);
            if (kernelThreadSpace)
            {
                kernelThreadSpace->SetDependencyArgToKernel(kernel);
            }
        }

        if (threadSpace != nullptr)
        {
            threadSpace->SetDependencyArgToKernel(kernel);
        }

        kernel->CollectKernelSurface();
        result = kernel->CreateKernelData( kernelData, totalSize, threadSpace );
        if( (kernelData == nullptr) || (result != CM_SUCCESS))
        {
            CM_ASSERTMESSAGE("Error: Failed to create kernel data.");
            CmKernelData::Destroy( kernelData );
            return result;
        }

        kernel->GetSizeInPayload( kernelPayloadSize );
        kernel->GetSizeInCurbe( kernelCurbeSize );

        if ( ( kernelCurbeSize + kernelPayloadSize ) > halMaxValues->maxArgByteSizePerKernel )
        {   //Failed, exceed the maximum of inline data
            CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
            return CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE;
        }
        else
        {
            kernelCurbeSize = kernel->GetAlignedCurbeSize( kernelCurbeSize );
            totalCurbeSize += kernelCurbeSize;
        }
        m_kernelCurbeOffsetArray[ i ] = totalCurbeSize - kernelCurbeSize;

        m_kernelData.SetElement( i, kernelData );

        totalKernelBinarySize += kernel->GetKernelGenxBinarySize();
        totalKernelBinarySize += CM_KERNEL_BINARY_PADDING_SIZE;  //Padding is necessary after kernel binary to avoid page fault issue

        bool *surfArray = nullptr;
        kernel->GetKernelSurfaces(surfArray);
        for (uint32_t j = 0; j < surfacePoolSize; j ++)
        {
            m_surfaceArray[j] |= surfArray[j];
        }
        kernel->ResetKernelSurfaces();

        PCM_CONTEXT_DATA cmData = ( PCM_CONTEXT_DATA )m_cmDevice->GetAccelData();
        PCM_HAL_STATE state = cmData->cmHalState;
        PRENDERHAL_MEDIA_STATE mediaStatePtr = state->pfnGetMediaStatePtrForKernel( state, kernel );

        if ( ( mediaStatePtr != nullptr ) && ( m_mediaStatePtr == nullptr ) )
        {
            m_mediaStatePtr = mediaStatePtr;
        }
        else if ( ( mediaStatePtr != nullptr ) && ( m_mediaStatePtr != nullptr ) )
        {
            CM_ASSERTMESSAGE( "Error: More than one media state heap are used in one task! User-provided state heap error.\n" );
            return CM_INVALID_ARG_VALUE;
        }
    }

    if (totalKernelBinarySize > halMaxValues->maxKernelBinarySize * halMaxValues->maxKernelsPerTask)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
        return CM_EXCEED_MAX_KERNEL_SIZE_IN_BYTE;
    }

    if (threadSpace)
    {
        if(FAILED(this->CreateThreadSpaceData(threadSpace)))
        {
            CM_ASSERTMESSAGE("Error: Failed to create thread space data.");
            return CM_FAILURE;
        }
        m_isThreadSpaceCreated = true;
    }

    UpdateSurfaceStateOnTaskCreation();

    m_taskType = CM_INTERNAL_TASK_WITH_THREADSPACE;

    if ( m_cmDevice->CheckGTPinEnabled())
    {
        AllocateKernelSurfInfo();
    }

    this->VtuneInitProfilingInfo(threadSpace);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Class  CmTaskInternal with thread group space
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Initialize(const CmThreadGroupSpace* threadGroupSpace)
{
    uint32_t totalCurbeSize         = 0;
    uint32_t surfacePoolSize        = 0;
    uint32_t totalKernelBinarySize  = 0;
    uint32_t kernelCurbeSize        = 0;
    uint32_t kernelPayloadSize      = 0;

    CmSurfaceManager* surfaceMgr = nullptr;
    CM_HAL_MAX_VALUES* halMaxValues = nullptr;
    CM_HAL_MAX_VALUES_EX* halMaxValuesEx = nullptr;
    m_cmDevice->GetHalMaxValues( halMaxValues, halMaxValuesEx );

    m_cmDevice->GetSurfaceManager( surfaceMgr );
    CM_CHK_NULL_RETURN_CMERROR( surfaceMgr );
    surfacePoolSize = surfaceMgr->GetSurfacePoolSize();
    m_surfaceArray = MOS_NewArray(bool, surfacePoolSize);
    if (!m_surfaceArray)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
    CmSafeMemSet( m_surfaceArray, 0, surfacePoolSize * sizeof( bool ) );

    if (m_cmDevice->IsPrintEnable())
    {
        SurfaceIndex *printBufferIndex = nullptr;
        m_cmDevice->GetPrintBufferIndex(printBufferIndex);
        CM_ASSERT(printBufferIndex);
        for (uint32_t i = 0; i < m_kernelCount; i++)
        {
            CmKernelRT* kernel = (CmKernelRT*)m_kernels.GetElement(i);
            if(kernel == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
                return CM_FAILURE;
            }
            if(FAILED(kernel->SetStaticBuffer(CM_PRINTF_STATIC_BUFFER_ID, printBufferIndex)))
            {
                CM_ASSERTMESSAGE("Error: Failed to set static buffer.");
                return CM_FAILURE;
            }
        }
    }

    for( uint32_t i = 0; i < m_kernelCount; i ++ )
    {
        CmKernelRT* kernel = (CmKernelRT*)m_kernels.GetElement( i );
        if(kernel == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
            return CM_FAILURE;
        }

        kernel->CollectKernelSurface();

        uint32_t totalSize =  0;
        CmKernelData* kernelData = nullptr;

        int32_t result = kernel->CreateKernelData( kernelData, totalSize, threadGroupSpace );
        if(result != CM_SUCCESS)
        {
            CM_ASSERTMESSAGE("Error: Failed to create kernel data.");
            CmKernelData::Destroy( kernelData );
            return result;
        }

        kernelData->SetKernelDataSize(totalSize);

        kernel->GetSizeInPayload(kernelPayloadSize);

        PCM_HAL_KERNEL_PARAM  halKernelParam = kernelData->GetHalCmKernelData();
        if (halKernelParam->crossThreadConstDataLen + halKernelParam->curbeSizePerThread + kernelPayloadSize
            > halMaxValues->maxArgByteSizePerKernel)
        {   //Failed, exceed the maximum of inline data
            CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
            return CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE;
        }
        else
        {
            kernel->GetSizeInCurbe(kernelCurbeSize);
            kernelCurbeSize = kernel->GetAlignedCurbeSize(kernelCurbeSize);
            totalCurbeSize += kernelCurbeSize;
        }

        m_kernelCurbeOffsetArray[ i ] = totalCurbeSize - kernelCurbeSize;

        m_kernelData.SetElement( i, kernelData );

        m_slmSize = kernel->GetSLMSize();

        m_spillMemUsed = kernel->GetSpillMemUsed();

        totalKernelBinarySize += kernel->GetKernelGenxBinarySize();
        totalKernelBinarySize += CM_KERNEL_BINARY_PADDING_SIZE;

        bool *surfArray = nullptr;
        kernel->GetKernelSurfaces(surfArray);
        for (uint32_t j = 0; j < surfacePoolSize; j ++)
        {
            m_surfaceArray[j] |= surfArray[j];
        }
        kernel->ResetKernelSurfaces();

        PCM_CONTEXT_DATA cmData = ( PCM_CONTEXT_DATA )m_cmDevice->GetAccelData();
        PCM_HAL_STATE state = cmData->cmHalState;
        PRENDERHAL_MEDIA_STATE mediaStatePtr = state->pfnGetMediaStatePtrForKernel( state, kernel );

        if ( ( mediaStatePtr != nullptr ) && ( m_mediaStatePtr == nullptr ) )
        {
            m_mediaStatePtr = mediaStatePtr;
        }
        else if ( ( mediaStatePtr != nullptr ) && ( m_mediaStatePtr != nullptr ) )
        {
            CM_ASSERTMESSAGE("Error: More than one media state heap are used in one task! User-provided state heap error.\n" );
            return CM_INVALID_ARG_VALUE;
        }
    }

    if( totalKernelBinarySize > halMaxValues->maxKernelBinarySize * halMaxValues->maxKernelsPerTask)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
        return CM_EXCEED_MAX_KERNEL_SIZE_IN_BYTE;
    }

    UpdateSurfaceStateOnTaskCreation();

    m_taskType = CM_INTERNAL_TASK_WITH_THREADGROUPSPACE;

    if (threadGroupSpace)
    {
        threadGroupSpace->GetThreadGroupSpaceSize(m_threadSpaceWidth, m_threadSpaceHeight,
                                                  m_threadSpaceDepth, m_groupSpaceWidth,
                                                  m_groupSpaceHeight, m_groupSpaceDepth);
        m_isThreadGroupSpaceCreated = true;
    }

    if ( m_cmDevice->CheckGTPinEnabled())
    {
        AllocateKernelSurfInfo();
    }

    this->VtuneInitProfilingInfo(threadGroupSpace);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Class  CmTaskInternal
//| Returns:    None.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Initialize(CmVeboxRT* vebox)
{
    int32_t result = CM_SUCCESS;
    CmSurfaceManager* surfaceMgr = nullptr;
    uint32_t surfacePoolSize = 0;

    m_cmDevice->GetSurfaceManager( surfaceMgr );
    CM_CHK_NULL_RETURN_CMERROR( surfaceMgr );
    surfacePoolSize = surfaceMgr->GetSurfacePoolSize();
    m_surfaceArray = MOS_NewArray(bool, surfacePoolSize);
    if (!m_surfaceArray)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_FAILURE;
    }
    CmSafeMemSet( m_surfaceArray, 0, surfacePoolSize * sizeof( bool ) );

    CmBufferUP *paramBuffer = nullptr;

    paramBuffer = vebox->GetParam();
    m_veboxState = vebox->GetState();

    m_veboxParam = paramBuffer;
    m_taskType = CM_INTERNAL_TASK_VEBOX;

    //Update used surfaces
    for (int i = 0; i < VEBOX_SURFACE_NUMBER; i++)
    {
        CmSurface2DRT* surf = nullptr;
        vebox->GetSurface(i, surf);
        if (surf)
        {
            uint32_t surfaceHandle = 0;
            SurfaceIndex* surfIndex = nullptr;
            surf->GetIndex(surfIndex);
            surf->GetHandle(surfaceHandle);
            m_surfaceArray[surfIndex->get_data()] = true;
            m_veboxSurfaceData.surfaceEntry[i].surfaceIndex = (uint16_t)surfaceHandle;
            m_veboxSurfaceData.surfaceEntry[i].surfaceCtrlBits = vebox->GetSurfaceControlBits(i);
        }
        else
        {
            m_veboxSurfaceData.surfaceEntry[i].surfaceIndex = CM_INVALID_INDEX;
            m_veboxSurfaceData.surfaceEntry[i].surfaceCtrlBits = CM_INVALID_INDEX;
        }
    }

    UpdateSurfaceStateOnTaskCreation();

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Class  CmTaskInternal with hints
//| Returns:    Result of the operation
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::Initialize(uint32_t hints, uint32_t numTasksGenerated, bool isLastTask)
{
    CmThreadSpaceRT* threadSpace = nullptr;
    int32_t result = CM_SUCCESS;

    // use ThreadSpace Initialize function to create kernel data
    result = this->Initialize(threadSpace, true);

    // set hints in task
    m_hints = hints;

    m_numTasksGenerated = numTasksGenerated;
    m_isLastTask = isLastTask;

    // set task type to be EnqueueWithHints
    m_taskType = CM_INTERNAL_TASK_ENQUEUEWITHHINTS;

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Kernel Count
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernelCount( uint32_t& count )
{
    count = m_kernelCount;
    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetTaskSurfaces( bool  *&surfArray )
{
    surfArray = m_surfaceArray;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Geth Kernel from the Kernel array
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernel( const uint32_t index, CmKernelRT* & kernel )
{
    kernel = nullptr;
    if( index < m_kernels.GetSize() )
    {
        kernel = (CmKernelRT*)m_kernels.GetElement( index );
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Geth Kernel data by kernel's index
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernelData( const uint32_t index, CmKernelData* & kernelData )
{
    kernelData = nullptr;
    if( index < m_kernelData.GetSize() )
    {
        kernelData = (CmKernelData*)m_kernelData.GetElement( index );
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Geth Kernel data size by kernel's index
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernelDataSize( const uint32_t index, uint32_t & size )
{
    size = 0;
    CmKernelData*  kernelData = nullptr;
    if( index < m_kernelData.GetSize() )
    {
        kernelData = (CmKernelData*)m_kernelData.GetElement( index );
        if (kernelData == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel data.");
            return CM_FAILURE;
        }
        size = kernelData->GetKernelDataSize();
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get kernel's curbe offset
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
uint32_t CmTaskInternal::GetKernelCurbeOffset( const uint32_t index )
{
    return ( uint32_t ) m_kernelCurbeOffsetArray[ index ];
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set task event, need add refcount hehe.
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::SetTaskEvent( CmEventRT* event )
{
    m_taskEvent = event;
    // add refCount
     m_taskEvent->Acquire();
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the task event
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetTaskEvent( CmEventRT* & event )
{
    event = m_taskEvent;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the task's status
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetTaskStatus(CM_STATUS & taskStatus)
{
    if(m_taskEvent == nullptr)
    {
        return CM_FAILURE;
    }

    return m_taskEvent->GetStatusNoFlush(taskStatus);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Record CPU ticks for Flush Time
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::VtuneSetFlushTime()
{
    if(!m_cmDevice->IsVtuneLogOn())
    {   // return directly if ETW log is off
        return CM_SUCCESS;
    }

    MOS_QueryPerformanceCounter((uint64_t*)&m_taskProfilingInfo.flushTime.QuadPart);
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Profiling Information for Media Pipeline
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::VtuneInitProfilingInfo(const CmThreadSpaceRT *perTaskThreadSpace)
{
    CmKernelRT    *cmKernel = nullptr;
    CmThreadSpaceRT *perKernelThreadSpace = nullptr;
    uint32_t    threadSpaceWidth = 0;
    uint32_t    threadSpaceHeight = 0;

    int32_t     hr = CM_SUCCESS;

    if(!m_cmDevice->IsVtuneLogOn())
    {   // return directly if ETW log is off
        return CM_SUCCESS;
    }

    CmSafeMemSet(&m_taskProfilingInfo, 0, sizeof(m_taskProfilingInfo));
    m_taskProfilingInfo.kernelCount = m_kernelCount;
    m_taskProfilingInfo.threadID    = CmGetCurThreadId(); // Get Thread ID

    MOS_QueryPerformanceCounter((uint64_t*)&m_taskProfilingInfo.enqueueTime.QuadPart); // Get Enqueue Time

    //  Currently, the Kernel/ThreadSpace/ThreadGroupSpace could not be deleted before task finished.
    m_taskProfilingInfo.kernelNames = MOS_NewArray(char, (CM_MAX_KERNEL_NAME_SIZE_IN_BYTE * m_kernelCount));
    CM_CHK_NULL_GOTOFINISH_CMERROR(m_taskProfilingInfo.kernelNames);

    m_taskProfilingInfo.localWorkWidth = MOS_NewArray(uint32_t, m_kernelCount);
    CM_CHK_NULL_GOTOFINISH_CMERROR(m_taskProfilingInfo.localWorkWidth);

    m_taskProfilingInfo.localWorkHeight = MOS_NewArray(uint32_t, m_kernelCount);
    CM_CHK_NULL_GOTOFINISH_CMERROR(m_taskProfilingInfo.localWorkHeight);

    m_taskProfilingInfo.globalWorkWidth = MOS_NewArray(uint32_t, m_kernelCount);
    CM_CHK_NULL_GOTOFINISH_CMERROR(m_taskProfilingInfo.globalWorkWidth);

    m_taskProfilingInfo.globalWorkHeight = MOS_NewArray(uint32_t, m_kernelCount);
    CM_CHK_NULL_GOTOFINISH_CMERROR(m_taskProfilingInfo.globalWorkHeight);

    for (uint32_t i = 0; i < m_kernelCount; i++)
    {
        CM_CHK_CMSTATUS_GOTOFINISH(GetKernel(i, cmKernel));
        CM_CHK_NULL_GOTOFINISH_CMERROR(cmKernel);

        //Copy Kernel Name
        MOS_SecureStrcpy(m_taskProfilingInfo.kernelNames + m_taskProfilingInfo.kernelNameLen,
                 CM_MAX_KERNEL_NAME_SIZE_IN_BYTE, cmKernel->GetName());

        //Add Kernel Name Length
        m_taskProfilingInfo.kernelNameLen += strlen(cmKernel->GetName()) + 1;

        CM_CHK_CMSTATUS_GOTOFINISH(cmKernel->GetThreadSpace(perKernelThreadSpace));

        if (perTaskThreadSpace)
        {
            //Per Task Thread Space Exists
            m_taskProfilingInfo.localWorkWidth[i] = m_threadSpaceWidth;
            m_taskProfilingInfo.localWorkHeight[i] = m_threadSpaceHeight;
            m_taskProfilingInfo.globalWorkWidth[i] = m_threadSpaceWidth;
            m_taskProfilingInfo.globalWorkHeight[i] = m_threadSpaceHeight;
        }
        else if (perKernelThreadSpace)
        {
            //Fill each threads Space's info
            perKernelThreadSpace->GetThreadSpaceSize(threadSpaceWidth, threadSpaceHeight);
            m_taskProfilingInfo.localWorkWidth[i] = threadSpaceWidth;
            m_taskProfilingInfo.localWorkHeight[i] = threadSpaceHeight;
            m_taskProfilingInfo.globalWorkWidth[i] = threadSpaceWidth;
            m_taskProfilingInfo.globalWorkHeight[i] = threadSpaceHeight;
        }
        else
        {
            //Fill the thread count
            uint32_t threadCount = 0;
            cmKernel->GetThreadCount(threadCount);
            m_taskProfilingInfo.localWorkWidth[i] = threadCount;
            m_taskProfilingInfo.localWorkHeight[i] = 1;
            m_taskProfilingInfo.globalWorkWidth[i] = threadCount;
            m_taskProfilingInfo.globalWorkHeight[i] = 1;
        }

    }

finish:
    if (hr != CM_SUCCESS)
    {
        MosSafeDeleteArray(m_taskProfilingInfo.kernelNames);
        MosSafeDeleteArray(m_taskProfilingInfo.localWorkWidth);
        MosSafeDeleteArray(m_taskProfilingInfo.localWorkHeight);
        MosSafeDeleteArray(m_taskProfilingInfo.globalWorkWidth);
        MosSafeDeleteArray(m_taskProfilingInfo.globalWorkHeight);
    }
    return hr;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Profiling Information
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::VtuneInitProfilingInfo(const CmThreadGroupSpace *perTaskThreadGroupSpace)
{
    CmKernelRT    *cmKernel = nullptr;
    CmThreadGroupSpace *perKernelGroupSpace = nullptr;
    uint32_t    threadSpaceWidth = 0;
    uint32_t    threadSpaceHeight = 0;
    uint32_t    threadSpaceDepth = 0;
    uint32_t    threadGroupSpaceWidth = 0;
    uint32_t    threadGroupSpaceHeight = 0;
    uint32_t    threadGroupSpaceDepth = 0;
    int32_t     hr = CM_SUCCESS;

    if(!m_cmDevice->IsVtuneLogOn())
    {   // return directly if ETW log is off
        return CM_SUCCESS;
    }

    CmSafeMemSet(&m_taskProfilingInfo, 0, sizeof(m_taskProfilingInfo));
    m_taskProfilingInfo.kernelCount = m_kernelCount;

    m_taskProfilingInfo.threadID    = CmGetCurThreadId(); // Get Thread ID

    MOS_QueryPerformanceCounter((uint64_t*)&m_taskProfilingInfo.enqueueTime.QuadPart); // Get Enqueue Time

    m_taskProfilingInfo.kernelNames = MOS_NewArray(char, (CM_MAX_KERNEL_NAME_SIZE_IN_BYTE * m_kernelCount));
    CM_CHK_NULL_GOTOFINISH_CMERROR(m_taskProfilingInfo.kernelNames);

    m_taskProfilingInfo.localWorkWidth = MOS_NewArray(uint32_t, m_kernelCount);
    CM_CHK_NULL_GOTOFINISH_CMERROR(m_taskProfilingInfo.localWorkWidth);

    m_taskProfilingInfo.localWorkHeight = MOS_NewArray(uint32_t, m_kernelCount);
    CM_CHK_NULL_GOTOFINISH_CMERROR(m_taskProfilingInfo.localWorkHeight);

    m_taskProfilingInfo.globalWorkWidth = MOS_NewArray(uint32_t, m_kernelCount);
    CM_CHK_NULL_GOTOFINISH_CMERROR(m_taskProfilingInfo.globalWorkWidth);

    m_taskProfilingInfo.globalWorkHeight = MOS_NewArray(uint32_t, m_kernelCount);
    CM_CHK_NULL_GOTOFINISH_CMERROR(m_taskProfilingInfo.globalWorkHeight);

    for (uint32_t i = 0; i < m_kernelCount; i++)
    {
        CM_CHK_CMSTATUS_GOTOFINISH(GetKernel(i, cmKernel));
        CM_CHK_NULL_GOTOFINISH_CMERROR(cmKernel);

        //Copy Kernel Name
        MOS_SecureStrcpy(m_taskProfilingInfo.kernelNames + m_taskProfilingInfo.kernelNameLen,
                 CM_MAX_KERNEL_NAME_SIZE_IN_BYTE, cmKernel->GetName());

        //Add Kernel Name Length
        m_taskProfilingInfo.kernelNameLen += strlen(cmKernel->GetName()) + 1;

        CM_CHK_CMSTATUS_GOTOFINISH(cmKernel->GetThreadGroupSpace(perKernelGroupSpace));

        if (perTaskThreadGroupSpace)
        {  // Per Thread Group Space
            perTaskThreadGroupSpace->GetThreadGroupSpaceSize(threadSpaceWidth, threadSpaceHeight,
                                                             threadSpaceDepth, threadGroupSpaceWidth,
                                                             threadGroupSpaceHeight, threadGroupSpaceDepth);
            m_taskProfilingInfo.localWorkWidth[i] = threadSpaceWidth;
            m_taskProfilingInfo.localWorkHeight[i] = threadSpaceHeight;
            m_taskProfilingInfo.globalWorkWidth[i] = threadSpaceWidth*threadGroupSpaceWidth;
            m_taskProfilingInfo.globalWorkHeight[i] = threadSpaceHeight*threadGroupSpaceHeight;

        }
        else if (perKernelGroupSpace)
        {
            //Fill each threads group space's info
            perKernelGroupSpace->GetThreadGroupSpaceSize(threadSpaceWidth, threadSpaceHeight,
                                                         threadSpaceDepth, threadGroupSpaceWidth,
                                                         threadGroupSpaceHeight, threadGroupSpaceDepth);
            m_taskProfilingInfo.localWorkWidth[i] = threadSpaceWidth;
            m_taskProfilingInfo.localWorkHeight[i] = threadSpaceHeight;
            m_taskProfilingInfo.globalWorkWidth[i] = threadSpaceWidth*threadGroupSpaceWidth;
            m_taskProfilingInfo.globalWorkHeight[i] = threadSpaceHeight*threadGroupSpaceHeight;  //Yi need to rethink
        }

    }

finish:
    if (hr != CM_SUCCESS)
    {
        MosSafeDeleteArray(m_taskProfilingInfo.kernelNames);
        MosSafeDeleteArray(m_taskProfilingInfo.localWorkWidth);
        MosSafeDeleteArray(m_taskProfilingInfo.localWorkHeight);
        MosSafeDeleteArray(m_taskProfilingInfo.globalWorkWidth);
        MosSafeDeleteArray(m_taskProfilingInfo.globalWorkHeight);
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Release Profiling information
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::VtuneReleaseProfilingInfo()
{
    if(!m_cmDevice->IsVtuneLogOn())
    {   // return directly if ETW log is off
        return CM_SUCCESS;
    }

    MosSafeDeleteArray(m_taskProfilingInfo.kernelNames);
    MosSafeDeleteArray(m_taskProfilingInfo.localWorkWidth);
    MosSafeDeleteArray(m_taskProfilingInfo.localWorkHeight);
    MosSafeDeleteArray(m_taskProfilingInfo.globalWorkWidth);
    MosSafeDeleteArray(m_taskProfilingInfo.globalWorkHeight);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Reset KernelData status from IN_USE to IDLE.
//              It is called immediately after the task being flushed.
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::ResetKernelDataStatus()
{
    int32_t     hr          = CM_SUCCESS;

    for(uint32_t krnDataIndex =0 ; krnDataIndex < m_kernelCount; krnDataIndex++ )
    {
        CmKernelData    *kernelData;
        CM_CHK_CMSTATUS_GOTOFINISH(GetKernelData(krnDataIndex, kernelData));
        CM_CHK_NULL_GOTOFINISH_CMERROR(kernelData);
        CM_CHK_CMSTATUS_GOTOFINISH(kernelData->ResetStatus());
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create thread space data
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::CreateThreadSpaceData(const CmThreadSpaceRT* threadSpace)
{
    uint32_t i;
    uint32_t width, height;
    uint32_t *kernelCoordinateIndex = nullptr;
    int hr = CM_SUCCESS;
    CmThreadSpaceRT *threadSpaceRT = const_cast<CmThreadSpaceRT*>(threadSpace);
    CmKernelRT* kernelInThreadSpace = nullptr;
    CmKernelRT* kernelInTask = nullptr;
    CM_CHK_NULL_GOTOFINISH(threadSpaceRT, CM_NULL_POINTER);

    threadSpaceRT->GetThreadSpaceSize(m_threadSpaceWidth, m_threadSpaceHeight);

    if (threadSpaceRT->IsThreadAssociated())
    {
        m_threadCoordinates = MOS_NewArray(PCM_HAL_SCOREBOARD, m_kernelCount);
        CM_CHK_NULL_GOTOFINISH(m_threadCoordinates, CM_FAILURE);
        CmSafeMemSet(m_threadCoordinates, 0, m_kernelCount*sizeof(PCM_HAL_SCOREBOARD));

        m_dependencyMasks = MOS_NewArray(PCM_HAL_MASK_AND_RESET, m_kernelCount);
        CM_CHK_NULL_GOTOFINISH(m_dependencyMasks, CM_FAILURE);
        CmSafeMemSet(m_dependencyMasks, 0, m_kernelCount*sizeof(PCM_HAL_MASK_AND_RESET));

        kernelCoordinateIndex = MOS_NewArray(uint32_t, m_kernelCount);
        if(m_threadCoordinates && kernelCoordinateIndex && m_dependencyMasks)
        {
            CmSafeMemSet(kernelCoordinateIndex, 0, m_kernelCount*sizeof(uint32_t));
            for (i = 0; i< m_kernelCount; i++)
            {
                kernelCoordinateIndex[i] = 0;
                uint32_t threadCount;
                this->GetKernel(i, kernelInTask);

                if(kernelInTask == nullptr)
                {
                    CM_ASSERTMESSAGE("Error: Invalid kernel pointer in task.");
                    hr = CM_NULL_POINTER;
                    goto finish;
                }

                kernelInTask->GetThreadCount(threadCount);
                if (threadCount == 0)
                {
                    threadCount = m_threadSpaceWidth*m_threadSpaceHeight;
                }
                m_threadCoordinates[i] = MOS_NewArray(CM_HAL_SCOREBOARD, threadCount);
                if (m_threadCoordinates[i])
                {
                    CmSafeMemSet(m_threadCoordinates[i], 0, sizeof(CM_HAL_SCOREBOARD)* threadCount);
                }
                else
                {
                    CM_ASSERTMESSAGE("Error: Pointer to thread coordinates is null.");
                    hr = CM_NULL_POINTER;
                    goto finish;
                }

                m_dependencyMasks[i] = MOS_NewArray(CM_HAL_MASK_AND_RESET, threadCount);
                if( m_dependencyMasks[i] )
                {
                    CmSafeMemSet(m_dependencyMasks[i], 0, sizeof(CM_HAL_MASK_AND_RESET) * threadCount);
                }
                else
                {
                    CM_ASSERTMESSAGE("Error: Pointer to dependency masks is null.");
                    hr = CM_NULL_POINTER;
                    goto finish;
                }
            }

            CM_THREAD_SPACE_UNIT *threadSpaceUnit = nullptr;
            threadSpaceRT->GetThreadSpaceSize(width, height);
            threadSpaceRT->GetThreadSpaceUnit(threadSpaceUnit);

            uint32_t *boardOrder = nullptr;
            threadSpaceRT->GetBoardOrder(boardOrder);
            for (uint32_t tIndex=0; tIndex < height*width; tIndex ++)
            {
                kernelInThreadSpace = static_cast<CmKernelRT *>(threadSpaceUnit[boardOrder[tIndex]].kernel);
                if (kernelInThreadSpace == nullptr)
                {
                    if (threadSpaceRT->GetNeedSetKernelPointer())
                    {
                        kernelInThreadSpace = threadSpaceRT->GetKernelPointer();
                    }
                    if (kernelInThreadSpace == nullptr)
                    {
                        CM_ASSERTMESSAGE("Error: Invalid kernel pointer in task.");
                        hr = CM_NULL_POINTER;
                        goto finish;
                    }
                }
                uint32_t kIndex = kernelInThreadSpace->GetIndexInTask();

                m_threadCoordinates[kIndex][kernelCoordinateIndex[kIndex]].x
                  = threadSpaceUnit[boardOrder[tIndex]].scoreboardCoordinates.x;
                m_threadCoordinates[kIndex][kernelCoordinateIndex[kIndex]].y
                  = threadSpaceUnit[boardOrder[tIndex]].scoreboardCoordinates.y;
                m_threadCoordinates[kIndex][kernelCoordinateIndex[kIndex]].mask
                  = threadSpaceUnit[boardOrder[tIndex]].dependencyMask;
                m_threadCoordinates[kIndex][kernelCoordinateIndex[kIndex]].resetMask
                  = threadSpaceUnit[boardOrder[tIndex]].reset;
                m_threadCoordinates[kIndex][kernelCoordinateIndex[kIndex]].color
                  = threadSpaceUnit[boardOrder[tIndex]].scoreboardColor;
                m_threadCoordinates[kIndex][kernelCoordinateIndex[kIndex]].sliceSelect
                  = threadSpaceUnit[boardOrder[tIndex]].sliceDestinationSelect;
                m_threadCoordinates[kIndex][kernelCoordinateIndex[kIndex]].subSliceSelect
                  = threadSpaceUnit[boardOrder[tIndex]].subSliceDestinationSelect;
                m_dependencyMasks[kIndex][kernelCoordinateIndex[kIndex]].mask
                  = threadSpaceUnit[boardOrder[tIndex]].dependencyMask;
                m_dependencyMasks[kIndex][kernelCoordinateIndex[kIndex]].resetMask
                  = threadSpaceUnit[boardOrder[tIndex]].reset;
                kernelCoordinateIndex[kIndex] ++;
            }

            MosSafeDeleteArray(kernelCoordinateIndex);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Failed to create thread space data.");
            hr = CM_FAILURE;
            goto finish;
        }

        m_isThreadCoordinatesExisted = true;
    }
    else
    {
        m_threadCoordinates = nullptr;
        m_dependencyMasks = nullptr;
        m_isThreadCoordinatesExisted = false;
    }

    if (threadSpaceRT->IsDependencySet())
    {
        threadSpaceRT->GetDependencyPatternType(m_dependencyPattern);
    }

    threadSpaceRT->GetColorCountMinusOne(m_colorCountMinusOne);
    threadSpaceRT->GetMediaWalkerGroupSelect(m_mediaWalkerGroupSelect);

    threadSpaceRT->GetWalkingPattern(m_walkingPattern);

    m_mediaWalkerParamsSet = threadSpaceRT->CheckWalkingParametersSet();
    if( m_mediaWalkerParamsSet )
    {
        CM_WALKING_PARAMETERS tmpMWParams;
        CM_CHK_CMSTATUS_GOTOFINISH(threadSpaceRT->GetWalkingParameters(tmpMWParams));
        CmSafeMemCopy(&m_walkingParameters, &tmpMWParams, sizeof(tmpMWParams));
    }

    m_dependencyVectorsSet = threadSpaceRT->CheckDependencyVectorsSet();
    if( m_dependencyVectorsSet )
    {
        CM_HAL_DEPENDENCY tmpDepVectors;
        CM_CHK_CMSTATUS_GOTOFINISH(threadSpaceRT->GetDependencyVectors(tmpDepVectors));
        CmSafeMemCopy(&m_dependencyVectors, &tmpDepVectors, sizeof(tmpDepVectors));
    }

finish:
    if(hr != CM_SUCCESS)
    {
        if(m_threadCoordinates )
        {
            for (i = 0; i< m_kernelCount; i++)
            {
                MosSafeDeleteArray(m_threadCoordinates[i]);
            }
        }

        if(m_dependencyMasks)
        {
            for (i = 0; i< m_kernelCount; i++)
            {
                MosSafeDeleteArray(m_dependencyMasks[i]);
            }
        }
        MosSafeDeleteArray(m_threadCoordinates);
        MosSafeDeleteArray(m_dependencyMasks);
        MosSafeDeleteArray(kernelCoordinateIndex);
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get thread space's coordinates
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernelCoordinates(const uint32_t index, void  *&kernelCoordinates)
{
    if (m_threadCoordinates != nullptr)
    {
        kernelCoordinates = (void *)m_threadCoordinates[index];
    }
    else
    {
        kernelCoordinates = nullptr;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get thread space's dependency masks
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetKernelDependencyMasks(const uint32_t index, void  *&kernelDependencyMasks)
{
    if (m_dependencyMasks != nullptr)
    {
        kernelDependencyMasks = (void *)m_dependencyMasks[index];
    }
    else
    {
        kernelDependencyMasks = nullptr;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get dependency pattern
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetDependencyPattern(CM_DEPENDENCY_PATTERN &dependencyPattern)
{
    dependencyPattern = m_dependencyPattern;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get media walking pattern
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetWalkingPattern(CM_WALKING_PATTERN &walkingPattern)
{
    walkingPattern = m_walkingPattern;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get media walking parameters
//| Returns:    CM_FAILURE if dest ptr is nullptr, CM_SUCCESS otherwise
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetWalkingParameters(CM_WALKING_PARAMETERS &walkingParameters)
{
    CmSafeMemCopy(&walkingParameters, &m_walkingParameters, sizeof(m_walkingParameters));
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Check to see if media walking parameters have been set
//| Returns:    true if media walking parameters set, false otherwise
//*-----------------------------------------------------------------------------
bool CmTaskInternal::CheckWalkingParametersSet( )
{
    return m_mediaWalkerParamsSet;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get dependency vectors
//| Returns:    CM_FAILURE if dest ptr is nullptr, CM_SUCCESS otherwise
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetDependencyVectors(CM_HAL_DEPENDENCY &dependencyVectors)
{
    CmSafeMemCopy(&dependencyVectors, &m_dependencyVectors, sizeof(m_dependencyVectors));
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Check to see if dependency vectors have been set
//| Returns:    true if dependency vectors are set, false otherwise
//*-----------------------------------------------------------------------------
bool CmTaskInternal::CheckDependencyVectorsSet( )
{
    return m_dependencyVectorsSet;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the total thread count
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetTotalThreadCount( uint32_t& totalThreadCount )
{
    totalThreadCount = m_totalThreadCount;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the width,height of thread space
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------

int32_t CmTaskInternal::GetThreadSpaceSize(uint32_t& width, uint32_t& height )
{
    width = m_threadSpaceWidth;
    height = m_threadSpaceHeight;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the color count minus one of the thread space
//|             Used to dispatch multiple sets of dependency threads
//|             for media walker
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------

int32_t CmTaskInternal::GetColorCountMinusOne( uint32_t& colorCount )
{
    colorCount = m_colorCountMinusOne;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether thread space is created
//| Returns:    Boolean.
//*-----------------------------------------------------------------------------

bool CmTaskInternal::IsThreadSpaceCreated(void )
{
    return m_isThreadSpaceCreated;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether thread coordinates are existed
//| Returns:    Boolean.
//*-----------------------------------------------------------------------------
bool CmTaskInternal::IsThreadCoordinatesExisted(void)
{
    return m_isThreadCoordinatesExisted;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether thread coordinates are existed
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------

int32_t CmTaskInternal::GetThreadGroupSpaceSize(uint32_t& threadSpaceWidth, uint32_t& threadSpaceHeight,
                                                uint32_t& threadSpaceDepth, uint32_t& groupSpaceWidth,
                                                uint32_t& groupSpaceHeight, uint32_t& groupSpaceDepth)
{
    threadSpaceWidth = m_threadSpaceWidth;
    threadSpaceHeight = m_threadSpaceHeight;
    threadSpaceDepth  = m_threadSpaceDepth;
    groupSpaceWidth = m_groupSpaceWidth;
    groupSpaceHeight = m_groupSpaceHeight;
    groupSpaceDepth = m_groupSpaceDepth;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the size of sharedlocalmemory
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetSLMSize(uint32_t& slmSize)
{
    slmSize = m_slmSize;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the size of spill memory used
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetSpillMemUsed(uint32_t& spillMemUsed)
{
    spillMemUsed = m_spillMemUsed;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the hints for EnqueueWithHints
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetHints(uint32_t& hints)
{
    hints = m_hints;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Gets the number of tasks generated for EnqueueWithHints
//|             Used when splitting large task to smaller tasks
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetNumTasksGenerated(uint32_t& numTasksGenerated)
{
    numTasksGenerated = m_numTasksGenerated;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Gets whether or not this task is the last task for EnqueueWithHints
//|             Used to identify last smaller task when splitting large task
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetLastTask(bool& isLastTask)
{
    isLastTask = m_isLastTask;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Whether thread group space is created
//| Returns:    Value.
//*-----------------------------------------------------------------------------
bool CmTaskInternal::IsThreadGroupSpaceCreated(void)
{
    return m_isThreadGroupSpaceCreated;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Space to record kernel surface's information
//| Returns:    result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::AllocateKernelSurfInfo()
{
    //Allocate Surf info array
    m_kernelSurfInfo.kernelNum = m_kernelCount;
    m_kernelSurfInfo.surfEntryInfosArray
      = (CM_HAL_SURFACE_ENTRY_INFO_ARRAY*)MOS_AllocAndZeroMemory(m_kernelCount *
                                                                 sizeof(CM_HAL_SURFACE_ENTRY_INFO_ARRAY));
    if(m_kernelSurfInfo.surfEntryInfosArray == nullptr)
    {

        CM_ASSERTMESSAGE("Error: Mem allocation fail.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    for( uint32_t i = 0; i < m_kernelCount; i ++ )
    {
        CmKernelRT * tempCmKernel = nullptr;
        this->GetKernel(i, tempCmKernel);
        if(tempCmKernel == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel pointer.");
            return CM_FAILURE;
        }

        CM_ARG* arg=NULL;
        tempCmKernel->GetArgs( arg );

        uint32_t argCount = 0;
        tempCmKernel->GetArgCount( argCount);
        //allocate memory for non_static buffer&2D&3D
        uint32_t surfEntryNum = 0;
        for( uint32_t j = 0; j < argCount; j ++ )
        {
            switch(arg[ j ].unitKind)
            {
                case    ARG_KIND_SURFACE_1D:
                        surfEntryNum = surfEntryNum + arg[ j ].unitCount * arg[j].unitSize/sizeof(int);
                        break;

                case    ARG_KIND_SURFACE_2D:
                case    ARG_KIND_SURFACE_2D_UP:
                case    ARG_KIND_SURFACE_3D:
                case    ARG_KIND_SURFACE_SAMPLER8X8_AVS:
                case    ARG_KIND_SURFACE_SAMPLER8X8_VA:
                        surfEntryNum = surfEntryNum + 3 * arg[ j ].unitCount * arg[j].unitSize/sizeof(int);//one 2D or 3D can have upto 3 planes
                        break;

                case    ARG_KIND_SURFACE_VME:
                        surfEntryNum = surfEntryNum + 24 * arg[ j ].unitCount;//surfaceVME will use upto 8 surfaces, each one can have upto 3 planes
                        break;

                default:
                    break;
            }
        }
        CM_HAL_SURFACE_ENTRY_INFO_ARRAY* tempArray =  m_kernelSurfInfo.surfEntryInfosArray;
        if(surfEntryNum>0)
        {
            tempArray[i].maxEntryNum = surfEntryNum;
            tempArray[i].surfEntryInfos = (CM_SURFACE_DETAILS*)MOS_AllocAndZeroMemory(surfEntryNum*sizeof(CM_SURFACE_DETAILS));

            if(tempArray[i].surfEntryInfos == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Mem allocation fail.");
                return CM_OUT_OF_HOST_MEMORY;
            }

        }

        //allocate memory for those 7 static buffers
        uint32_t globalBufNum=CM_GLOBAL_SURFACE_NUMBER + CM_GTPIN_BUFFER_NUM;
        tempArray[i].globalSurfNum=globalBufNum;
        tempArray[i].globalSurfInfos = (CM_SURFACE_DETAILS*)MOS_AllocAndZeroMemory(
                                globalBufNum*sizeof(CM_SURFACE_DETAILS));
        if(tempArray[i].globalSurfInfos == nullptr)
        {
            CM_ASSERTMESSAGE("Mem allocation fail.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetKernelSurfInfo(CM_HAL_SURFACE_ENTRY_INFO_ARRAYS & surfEntryInfoArray)
{
    surfEntryInfoArray = m_kernelSurfInfo;
    return CM_SUCCESS;
}

int32_t CmTaskInternal::ClearKernelSurfInfo()
{
    if (m_kernelSurfInfo.surfEntryInfosArray == nullptr)
    { // if surfEntryInfosArray is empty, return directly
        return CM_SUCCESS;
    }

    //free memory
    for( uint32_t i = 0; i < m_kernelCount; i ++ )
    {
        if (m_kernelSurfInfo.surfEntryInfosArray[i].surfEntryInfos != nullptr)
        {
            MosSafeDelete(m_kernelSurfInfo.surfEntryInfosArray[i].surfEntryInfos);
        }
        if (m_kernelSurfInfo.surfEntryInfosArray[i].globalSurfInfos!= nullptr)
        {
            MosSafeDelete(m_kernelSurfInfo.surfEntryInfosArray[i].globalSurfInfos);
        }
    }

    MosSafeDelete(m_kernelSurfInfo.surfEntryInfosArray);

    m_kernelSurfInfo.kernelNum = 0 ;
    m_kernelSurfInfo.surfEntryInfosArray = nullptr;

    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetTaskType(uint32_t& taskType)
{
    taskType = m_taskType;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get vebox state
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::GetVeboxState(CM_VEBOX_STATE &veboxState)
{
    veboxState = m_veboxState;

    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetVeboxParam(CmBufferUP * &veboxParam)
{
    veboxParam = m_veboxParam;

    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetVeboxSurfaceData(CM_VEBOX_SURFACE_DATA &veboxSurfaceData)
{
    veboxSurfaceData = m_veboxSurfaceData;
    return CM_SUCCESS;
}

uint64_t CmTaskInternal::GetSyncBitmap()
{
    return m_ui64SyncBitmap;
}

uint64_t CmTaskInternal::GetConditionalEndBitmap()
{
    return m_ui64ConditionalEndBitmap;
}

CM_HAL_CONDITIONAL_BB_END_INFO* CmTaskInternal::GetConditionalEndInfo()
{
    return m_conditionalEndInfo;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set power option for this task
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::SetPowerOption( PCM_POWER_OPTION powerOption )
{
    if (powerOption == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to power option is null.");
        return CM_NULL_POINTER;
    }
    CmSafeMemCopy( &m_powerOption, powerOption, sizeof( m_powerOption ) );
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get power option for this task
//| Returns:    Pointer to power option.
//*-----------------------------------------------------------------------------
PCM_POWER_OPTION CmTaskInternal::GetPowerOption()
{
    return &m_powerOption;
}

#if _DEBUG
const char *gDependencyPatternString[] =
{
    "DEPENDENCY_NONE",
    "DEPENDENCY_WAVEFRONT45",
    "DEPENDENCY_WAVEFRONT26"
};

//Only for debugging
int32_t CmTaskInternal::DisplayThreadSpaceData(uint32_t width, uint32_t height)
{
    if (m_threadCoordinates != nullptr)
    {
        CM_NORMALMESSAGE("Score board[Kernel x: (x1, y1), (x2, y2)...]:");
        for (uint32_t i = 0; i < m_kernelCount; i ++)
        {
            CmKernelRT *kernelRT = nullptr;
            GetKernel(i, kernelRT);
            if(nullptr == kernelRT)
            {
                return CM_FAILURE;
            }

            uint32_t threadCount;
            kernelRT->GetThreadCount(threadCount);
            if (threadCount == 0)
            {
                threadCount = m_threadSpaceWidth*m_threadSpaceHeight;
            }
            CM_NORMALMESSAGE("Kernel %d: ", i);
            for (uint32_t j=0; j<threadCount; j++)
            {
                CM_NORMALMESSAGE("(%d, %d) ", m_threadCoordinates[i][j].x, m_threadCoordinates[i][j].y);
            }
        }
    }
    else
    {
        CM_NORMALMESSAGE("Score Board is NULL.");
    }

    if (m_dependencyPattern <= CM_WAVEFRONT26)
    {
        CM_NORMALMESSAGE("Dependency Pattern: %s.", gDependencyPatternString[m_dependencyPattern]);
    }
    else
    {
        CM_NORMALMESSAGE("Dependency Pattern: UNASSIGNED.");
    }

    return CM_SUCCESS;
}
#endif

int32_t CmTaskInternal::GetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT& groupSelect)
{
    groupSelect = m_mediaWalkerGroupSelect;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Update surface state on task destroy stage
//*-----------------------------------------------------------------------------
int32_t CmTaskInternal::UpdateSurfaceStateOnTaskCreation()
{
    CmSurfaceManager*   surfaceMgr = nullptr;
    int32_t             *surfState = nullptr;

    m_cmDevice->GetSurfaceManager(surfaceMgr);
    if (surfaceMgr == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface manager is null.");
        return CM_NULL_POINTER;
    }

    uint32_t poolSize = surfaceMgr->GetSurfacePoolSize();
    uint32_t handle = 0;
    uint32_t curTaskSurfCnt = 0;
    void **  curTaskSurfResArray = nullptr;
    uint32_t  refSurfCnt = 0;
    uint32_t *refSurfHandleArray = nullptr;
    CM_RETURN_CODE hr = CM_SUCCESS;

    curTaskSurfResArray = (void **)MOS_AllocAndZeroMemory(sizeof(void *)*poolSize);
    CM_CHK_NULL_RETURN_CMERROR(curTaskSurfResArray);

    CSync* surfaceLock = m_cmDevice->GetSurfaceCreationLock();

    if (surfaceLock == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface creation lock is null.");
        if (curTaskSurfResArray)
        {
            MOS_FreeMemory(curTaskSurfResArray);
            curTaskSurfResArray = nullptr;
        }
        return CM_NULL_POINTER;
    }

    surfaceLock->Acquire();

    // get the last tracker
    PCM_CONTEXT_DATA cmData = ( PCM_CONTEXT_DATA )m_cmDevice->GetAccelData();
    PCM_HAL_STATE state = nullptr;
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmData);
    state = cmData->cmHalState;
    CM_CHK_NULL_GOTOFINISH_CMERROR(state);

    if (!m_isSurfaceUpdateDone)
    {
        for (uint32_t i = 0; i < poolSize; i++)
        {
            if (m_surfaceArray[i])
            {
                CmSurface *surface = NULL;
                CM_CHK_CMSTATUS_GOTOFINISH(surfaceMgr->GetSurface(i, surface));
                if (surface == nullptr) // surface destroyed but not updated in kernel
                {
                    continue;
                }
                if (m_taskType == CM_INTERNAL_TASK_VEBOX)
                {
                    surface->SetVeboxTracker(state->renderHal->veBoxTrackerRes.currentTrackerId);
                }
                else
                {
                    surface->SetRenderTracker(state->renderHal->currentTrackerIndex,
                               state->renderHal->trackerProducer.GetNextTracker(state->renderHal->currentTrackerIndex));
                }

                // Push this surface's resource into array for CP check.
                switch (surface->Type())
                {
                    case CM_ENUM_CLASS_TYPE_CMBUFFER_RT :
                        static_cast< CmBuffer_RT* >( surface )->GetHandle(handle);
                        curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->bufferTable[handle].osResource;
                        break;

                    case CM_ENUM_CLASS_TYPE_CMSURFACE2D :
                        static_cast< CmSurface2DRT* >( surface )->GetHandle(handle);
                        curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->umdSurf2DTable[handle].osResource;
                        break;

                    case CM_ENUM_CLASS_TYPE_CMSURFACE2DUP:
                        static_cast< CmSurface2DUPRT* >( surface )->GetHandle(handle);
                        curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->surf2DUPTable[handle].osResource;
                        break;

                    case CM_ENUM_CLASS_TYPE_CMSURFACE3D :
                        static_cast< CmSurface3DRT* >( surface )->GetHandle(handle);
                        curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->surf3DTable[handle].osResource;
                        break;

                    case CM_ENUM_CLASS_TYPE_CMSURFACEVME:
                        static_cast< CmSurfaceVme* >( surface )->GetIndexCurrent(handle);
                        curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->umdSurf2DTable[handle].osResource; // current surface
                        static_cast< CmSurfaceVme* >( surface )->GetIndexForwardCount(refSurfCnt);
                        static_cast< CmSurfaceVme* >( surface )->GetIndexForwardArray(refSurfHandleArray);
                        for(i = 0; i < refSurfCnt; i++)
                        {
                            curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->umdSurf2DTable[refSurfHandleArray[i]].osResource; // forward surfaces
                        }
                        static_cast< CmSurfaceVme* >( surface )->GetIndexForwardCount(refSurfCnt);
                        static_cast< CmSurfaceVme* >( surface )->GetIndexForwardArray(refSurfHandleArray);
                        for(i = 0; i < refSurfCnt; i++)
                        {
                            curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->umdSurf2DTable[refSurfHandleArray[i]].osResource; // backward surfaces
                        }
                        break;

                    case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8:
                        static_cast< CmSurfaceSampler8x8* >( surface )->GetIndexCurrent(handle);
                        curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->umdSurf2DTable[handle].osResource;
                        break;

                    case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER:
                        static_cast< CmSurfaceSampler* >( surface )->GetHandle(handle);
                        SAMPLER_SURFACE_TYPE type;
                        static_cast< CmSurfaceSampler* >( surface )->GetSurfaceType(type);
                        if (type == SAMPLER_SURFACE_TYPE_2D)
                        {
                            curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->umdSurf2DTable[handle].osResource;
                        }
                        else if (type == SAMPLER_SURFACE_TYPE_2DUP)
                        {
                            curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->surf2DUPTable[handle].osResource;
                        }
                        else if (type == SAMPLER_SURFACE_TYPE_3D)
                        {
                            curTaskSurfResArray[curTaskSurfCnt++] = (void *)&state->surf3DTable[handle].osResource;
                        }
                        else
                        {
                            hr = CM_INVALID_ARG_INDEX;
                            goto finish;
                        }
                        break;

                    default:
                        break;
                }
            }
        }

        m_isSurfaceUpdateDone = true;
    }

    // Check if there is any secure surface.
    if (curTaskSurfCnt > 0 && state->osInterface && state->osInterface->osCpInterface)
    {
        state->osInterface->osCpInterface->PrepareResources(curTaskSurfResArray, curTaskSurfCnt, nullptr, 0);
    }

finish:
    surfaceLock->Release();
    if (curTaskSurfResArray)
    {
        MOS_FreeMemory(curTaskSurfResArray);
        curTaskSurfResArray = nullptr;
    }

    return hr;
}

#if CM_LOG_ON
std::string CmTaskInternal::Log()
{
    std::ostringstream  oss;

    oss << "Enqueue Task Type:" << m_taskType
        << " Kernel Count:" << m_kernelCount
        << " Total Thread Count:" << m_totalThreadCount
        << " Sync Bit:"<<m_ui64SyncBitmap
        << " Conditional End Bit:" << m_ui64ConditionalEndBitmap
        << std::endl;

    switch(m_taskType)
    {
        case CM_INTERNAL_TASK_WITH_THREADSPACE:
            if ( m_isThreadSpaceCreated )
            {
                oss << "Thread Space Width :" << m_threadSpaceWidth << " Height :" << m_threadSpaceHeight
                    << "Walker Patten :" << (int)m_walkingPattern << std::endl;
            }
            break;

        case CM_INTERNAL_TASK_WITH_THREADGROUPSPACE:
            if(m_isThreadGroupSpaceCreated)
            {
                oss << "Thread Group Space Width:" << m_groupSpaceWidth << " Height:" << m_groupSpaceHeight
                    << "SLM Size:" <<m_slmSize << std::endl;
            }
            break;

        case CM_INTERNAL_TASK_VEBOX:
            break;

        case CM_INTERNAL_TASK_ENQUEUEWITHHINTS:
            oss << " Hints :" << m_hints
                << " Thread Space Width :" << m_threadSpaceWidth
                << " Height :" << m_threadSpaceHeight
                << " Walker Patten :" << (int)m_walkingPattern
                << std::endl;
            break;

        default:    // by default, assume the task is considered as general task: CM_INTERNAL_TASK_WITH_THREADSPACE
            break;
    }

    for (uint32_t i=0 ; i< m_kernelCount; i++)
    {
        CmKernelRT* kernel = (CmKernelRT*)m_kernels.GetElement( i );

        oss << kernel->Log(); // log each kernel
    }

    return oss.str();
}
#endif

void CmTaskInternal::SurfaceDump(int32_t taskId)
{
#if MDF_SURFACE_CONTENT_DUMP
    for (uint32_t i=0 ; i< m_kernelCount; i++)
    {
        CmKernelRT* kernel = (CmKernelRT*)m_kernels.GetElement( i );
        kernel->SurfaceDump(i, taskId);
    }
#endif
}

int32_t CmTaskInternal::SetProperty(CM_TASK_CONFIG * taskConfig)
{
    if (taskConfig == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to task config is null.");
        return CM_NULL_POINTER;
    }
    CmSafeMemCopy(&m_taskConfig, taskConfig, sizeof(m_taskConfig));
    return CM_SUCCESS;
}

int32_t CmTaskInternal::GetProperty(CM_TASK_CONFIG &taskConfig)
{
    taskConfig = m_taskConfig;
    return CM_SUCCESS;
}

void  *CMRT_UMD::CmTaskInternal::GetMediaStatePtr()
{
    return m_mediaStatePtr;
}
}
