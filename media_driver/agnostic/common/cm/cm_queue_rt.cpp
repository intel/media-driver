/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_queue_rt.cpp
//! \brief     Contains CmQueueRT implementations.
//!

#include "cm_queue_rt.h"

#include "cm_mem.h"
#include "cm_device_rt.h"
#include "cm_event_rt.h"
#include "cm_task_rt.h"
#include "cm_task_internal.h"
#include "cm_thread_space_rt.h"
#include "cm_kernel_rt.h"
#include "cm_kernel_data.h"
#include "cm_buffer_rt.h"
#include "cm_group_space.h"
#include "cm_vebox_data.h"
#include "cm_surface_manager.h"
#include "cm_surface_2d_rt.h"
#include "cm_vebox_rt.h"

// Used by GPUCopy
#define BLOCK_PIXEL_WIDTH            (32)
#define BLOCK_HEIGHT                 (8)
#define BLOCK_HEIGHT_NV12            (4)
#define SUB_BLOCK_PIXEL_WIDTH        (8)
#define SUB_BLOCK_HEIGHT             (8)
#define SUB_BLOCK_HEIGHT_NV12        (4)
#define INNER_LOOP                   (4)
#define BYTE_COPY_ONE_THREAD         (1024*INNER_LOOP)  //4K for each thread
#define THREAD_SPACE_WIDTH_INCREMENT (8)
//Used by unaligned copy
#define BLOCK_WIDTH                  (64)
#define PAGE_ALIGNED                 (0x1000)

#define GPUCOPY_KERNEL_LOCK(a) ((a)->bLocked = true)
#define GPUCOPY_KERNEL_UNLOCK(a) ((a)->bLocked = false)

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Queue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::Create(CmDeviceRT *pDevice,
                          CmQueueRT* &pQueue,
                          CM_QUEUE_CREATE_OPTION QueueCreateOption)
{
    int32_t result = CM_SUCCESS;
    pQueue = new (std::nothrow) CmQueueRT(pDevice, QueueCreateOption);
    if( pQueue )
    {
        result = pQueue->Initialize( );
        if( result != CM_SUCCESS )
        {
            CmQueueRT::Destroy( pQueue);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmQueue due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Queue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::Destroy(CmQueueRT* &pQueue )
{
    if( pQueue == nullptr )
    {
        return CM_FAILURE;
    }

    uint32_t result = pQueue->CleanQueue();
    CmSafeDelete( pQueue );

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of Cm Queue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmQueueRT::CmQueueRT(CmDeviceRT *pDevice,
                     CM_QUEUE_CREATE_OPTION QueueCreateOption):
    m_pDevice(pDevice),
    m_EventArray(CM_INIT_EVENT_COUNT),
    m_EventCount(0),
    m_pHalMaxValues(nullptr),
    m_CopyKrnParamArray(CM_INIT_GPUCOPY_KERNL_COUNT),
    m_CopyKrnParamArrayCount(0),
    m_queueOption(QueueCreateOption)
{

}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of Cm Queue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmQueueRT::~CmQueueRT()
{
    uint32_t EventReleaseTimes = 0;

    uint32_t EventArrayUsedSize = m_EventArray.GetMaxSize();
    for( uint32_t i = 0; i < EventArrayUsedSize; i ++ )
    {
        CmEventRT* pEvent = (CmEventRT*)m_EventArray.GetElement( i );
        EventReleaseTimes = 0;
        while( pEvent )
        {   // destroy the event no matter if it is released by user
            if(EventReleaseTimes > 2)
            {
                // The max of event's reference cout is 2
                // if the event is not released after 2 times, there is something wrong
                CM_ASSERTMESSAGE("Error: The max of event's reference cout is 2.");
                break;
            }
            CmEventRT::Destroy( pEvent );
            EventReleaseTimes ++;
        }
    }
    m_EventArray.Delete();

    // Do not destroy the kernel in m_CopyKrnParamArray.
    // They have been destoyed in ~CmDevice() before destroying Queue
    for( uint32_t i = 0; i < m_CopyKrnParamArrayCount; i ++ )
    {
        CM_GPUCOPY_KERNEL *pGPUCopyParam = (CM_GPUCOPY_KERNEL*)m_CopyKrnParamArray.GetElement( i );
        CmSafeDelete(pGPUCopyParam);
    }

    m_CopyKrnParamArray.Delete();

}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Cm Queue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::Initialize()
{
    PCM_HAL_STATE pCmHalState = ((PCM_CONTEXT_DATA)m_pDevice->GetAccelData())->cmHalState;
    CM_HAL_MAX_VALUES_EX* pHalMaxValuesEx = nullptr;
    CM_RETURN_CODE hr = CM_SUCCESS;
    m_pDevice->GetHalMaxValues(m_pHalMaxValues, pHalMaxValuesEx);

    // Creates or gets GPU Context for the test
    if (m_queueOption.UserGPUContext == true)
    {
        // Checks if it is the user-provided GPU context. If it is valid, we will create the queue with the existing Context
        if (pCmHalState->pOsInterface->pfnIsGpuContextValid(pCmHalState->pOsInterface, (MOS_GPU_CONTEXT)m_queueOption.GPUContext) != MOS_STATUS_SUCCESS)
        {
            // Returns failure
            CM_ASSERTMESSAGE("Error: The user passed in an GPU context which is not valid");
            return CM_INVALID_USER_GPU_CONTEXT_FOR_QUEUE_EX;
        }
    }
    else
    {
        // Create MDF preset GPU context, update GPUContext in m_queueOption
        if (m_queueOption.QueueType == CM_QUEUE_TYPE_RENDER)
        {
            CHK_MOSSTATUS_RETURN_CMERROR(pCmHalState->pfnCreateGPUContext(pCmHalState, pCmHalState->GpuContext, MOS_GPU_NODE_3D));
            m_queueOption.GPUContext = pCmHalState->GpuContext;
        }
        else if (m_queueOption.QueueType == CM_QUEUE_TYPE_COMPUTE)
        {
            CHK_MOSSTATUS_RETURN_CMERROR(pCmHalState->pfnCreateGPUContext(pCmHalState, MOS_GPU_CONTEXT_CM_COMPUTE, MOS_GPU_NODE_COMPUTE));
            m_queueOption.GPUContext = MOS_GPU_CONTEXT_CM_COMPUTE;
        }
        else
        {
            // Returns failure
            CM_ASSERTMESSAGE("Error: The QueueType is note supported by MDF.");
            return CM_NOT_IMPLEMENTED;
        }
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Checks whether any kernels in the task have a thread argument
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::GetTaskHasThreadArg(CmKernelRT* pKernelArray[], uint32_t numKernels, bool& threadArgExists)
{
    threadArgExists = false;

    for(uint32_t iKrn = 0; iKrn < numKernels; iKrn++)
    {
        if( !pKernelArray[iKrn] )
        {
            CM_ASSERTMESSAGE("Error: The kernel in the task have no thread argument.");
            return CM_FAILURE;
        }

        if( pKernelArray[iKrn]->IsThreadArgExisted( ) )
        {
            threadArgExists = true;
            break;
        }
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Enqueue Task
//| Arguments :
//|               pKernelArray      [in]       Pointer to kernel array
//|               pEvent            [in]       Reference to the pointer to Event
//|               pTS               [out]      Pointer to thread space
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::Enqueue(
           CmTask* pKernelArray,
           CmEvent* & pEvent,
           const CmThreadSpace* pThreadSpace)
{
    INSERT_API_CALL_LOG();

    int32_t result;

    if(pKernelArray == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Kernel array is null.");
        return CM_INVALID_ARG_VALUE;
    }

    CmTaskRT *pKernelArrayRT = static_cast<CmTaskRT *>(pKernelArray);
    uint32_t KernelCount = 0;
    KernelCount = pKernelArrayRT->GetKernelCount();
    if( KernelCount == 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel count.");
        return CM_FAILURE;
    }

    if( KernelCount > m_pHalMaxValues->maxKernelsPerTask )
    {
        CM_ASSERTMESSAGE("Error: Kernel count exceeds max kernel per enqueue.");
        return CM_EXCEED_MAX_KERNEL_PER_ENQUEUE;
    }

    const CmThreadSpaceRT *pTSRTConst = static_cast<const CmThreadSpaceRT *>(pThreadSpace);
    if (pTSRTConst && pTSRTConst->IsThreadAssociated())
    {
        if (pTSRTConst->GetNeedSetKernelPointer() && pTSRTConst->KernelPointerIsNULL())
        {
            CmKernelRT* pTmp = nullptr;
            pTmp = pKernelArrayRT->GetKernelPointer(0);
            pTSRTConst->SetKernelPointer(pTmp);
        }
    }

#if _DEBUG
    if (pTSRTConst)
    {
        CmThreadSpaceRT *pTS_RT = const_cast<CmThreadSpaceRT*>(pTSRTConst);
        if (!pTS_RT->IntegrityCheck(pKernelArrayRT))
        {
            CM_ASSERTMESSAGE("Error: Invalid thread space.");
            return CM_INVALID_THREAD_SPACE;
        }
    }
#endif

    if(m_pDevice->IsPrintEnable())
    {
        m_pDevice->ClearPrintBuffer();
    }

    typedef CmKernelRT* pCmKernel;
    CmKernelRT** pTmp = MOS_NewArray(pCmKernel, (KernelCount + 1));
    if(pTmp == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    uint32_t totalThreadNumber = 0;
    for(uint32_t i = 0; i < KernelCount; i++)
    {
        pTmp[ i ] = pKernelArrayRT->GetKernelPointer(i);

        uint32_t singleThreadNumber = 0;
        pTmp[i]->GetThreadCount(singleThreadNumber);
        if (singleThreadNumber == 0)
        {
            CmThreadSpaceRT *pTS_RT = const_cast<CmThreadSpaceRT*>(pTSRTConst);
            if (pTS_RT)
            {
                uint32_t width, height;
                pTS_RT->GetThreadSpaceSize(width, height);
                singleThreadNumber = width*height;
            }
        }
        totalThreadNumber += singleThreadNumber;
    }
    pTmp[KernelCount ] = nullptr;

    CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
    result = Enqueue_RT(pTmp, KernelCount, totalThreadNumber, pEventRT, pTSRTConst, pKernelArrayRT->GetSyncBitmap(), pKernelArrayRT->GetPowerOption(),
                        pKernelArrayRT->GetConditionalEndBitmap(), pKernelArrayRT->GetConditionalEndInfo(), pKernelArrayRT->GetTaskConfig());

    if (pEventRT)
    {
        pEventRT->SetKernelNames(pKernelArrayRT, const_cast<CmThreadSpaceRT*>(pTSRTConst), nullptr);
    }

    pEvent = pEventRT;
    MosSafeDeleteArray( pTmp );

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:      Enqueue Task
//| Arguments :
//|               pKernelArray      [in]       Pointer to kernel array
//|               pEvent            [in]       Reference to the pointer to Event
//|               pTS               [out]      Pointer to thread space
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::Enqueue_RT(
                        CmKernelRT* pKernelArray[],
                        const uint32_t uiKernelCount,
                        const uint32_t uiTotalThreadCount,
                        CmEventRT* & pEvent,
                        const CmThreadSpaceRT* pTS,
                        uint64_t    uiSyncBitmap,
                        PCM_POWER_OPTION pPowerOption,
                        uint64_t    uiConditionalEndBitmap,
                        CM_HAL_CONDITIONAL_BB_END_INFO* pConditionalEndInfo,
                        PCM_TASK_CONFIG  pTaskConfig)
{
    if(pKernelArray == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Kernel array is NULL.");
        return CM_INVALID_ARG_VALUE;
    }

    if( uiKernelCount == 0 )
    {
        CM_ASSERTMESSAGE("Error: There are no valid kernels.");
        return CM_INVALID_ARG_VALUE;
    }

    bool bIsEventVisible = (pEvent == CM_NO_EVENT)? false:true;

    CLock Locker(m_CriticalSection_TaskInternal);
    CmTaskInternal* pTask = nullptr;
    int32_t result = CmTaskInternal::Create(uiKernelCount, uiTotalThreadCount, pKernelArray, pTS, m_pDevice, uiSyncBitmap, pTask, uiConditionalEndBitmap, pConditionalEndInfo);
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Create CM task internal failure.");
        return result;
    }

    LARGE_INTEGER nEnqueueTime;
    if ( !(MOS_QueryPerformanceCounter( (uint64_t*)&nEnqueueTime.QuadPart )))
    {
        CM_ASSERTMESSAGE("Error: Query performance counter failure.");
        return CM_FAILURE;
    }

    int32_t taskDriverId = -1;

    result = CreateEvent(pTask, bIsEventVisible, taskDriverId, pEvent);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Create event failure.");
        return result;
    }
    if ( pEvent != nullptr )
    {
        pEvent->SetEnqueueTime( nEnqueueTime );
    }

    pTask->SetPowerOption( pPowerOption );

    pTask->SetProperty(pTaskConfig);

    if( !m_EnqueuedTasks.Push( pTask ) )
    {
        CM_ASSERTMESSAGE("Error: Push enqueued tasks failure.");
        return CM_FAILURE;
    }

    result = FlushTaskWithoutSync();


    return result;
}


int32_t CmQueueRT::Enqueue_RT(CmKernelRT* pKernelArray[],
                        const uint32_t uiKernelCount,
                        const uint32_t uiTotalThreadCount,
                        CmEventRT* & pEvent,
                        const CmThreadGroupSpace* pTGS,
                        uint64_t    uiSyncBitmap,
                        PCM_POWER_OPTION pPowerOption,
                        PCM_TASK_CONFIG  pTaskConfig)
{
    if(pKernelArray == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Kernel array is NULL.");
        return CM_INVALID_ARG_VALUE;
    }

    if( uiKernelCount == 0 )
    {
        CM_ASSERTMESSAGE("Error: There are no valid kernels.");
        return CM_INVALID_ARG_VALUE;
    }

    CLock Locker(m_CriticalSection_TaskInternal);

    CmTaskInternal* pTask = nullptr;
    int32_t result = CmTaskInternal::Create( uiKernelCount, uiTotalThreadCount, pKernelArray, pTGS, m_pDevice, uiSyncBitmap, pTask );
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Create CmTaskInternal failure.");
        return result;
    }

    LARGE_INTEGER nEnqueueTime;
    if ( !(MOS_QueryPerformanceCounter( (uint64_t*)&nEnqueueTime.QuadPart )))
    {
        CM_ASSERTMESSAGE("Error: Query performance counter failure.");
        return CM_FAILURE;
    }

    int32_t taskDriverId = -1;
    result = CreateEvent(pTask, !(pEvent == CM_NO_EVENT) , taskDriverId, pEvent);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Create event failure.");
        return result;
    }
    if ( pEvent != nullptr )
    {
        pEvent->SetEnqueueTime( nEnqueueTime );
    }

    pTask->SetPowerOption( pPowerOption );

    pTask->SetProperty(pTaskConfig);

    if( !m_EnqueuedTasks.Push( pTask ) )
    {
        CM_ASSERTMESSAGE("Error: Push enqueued tasks failure.")
        return CM_FAILURE;
    }

    result = FlushTaskWithoutSync();

    return result;
}

int32_t CmQueueRT::Enqueue_RT( CmKernelRT* pKernelArray[],
                        CmEventRT* & pEvent,
                        uint32_t numTasksGenerated,
                        bool isLastTask,
                        uint32_t hints,
                        PCM_POWER_OPTION pPowerOption)
{
    int32_t result = CM_FAILURE;
    uint32_t kernelCount = 0;
    CmTaskInternal* pTask = nullptr;
    int32_t taskDriverId = -1;
    bool bIsEventVisible = (pEvent == CM_NO_EVENT) ? false:true;
    bool threadArgExists = false;

    if( pKernelArray == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Kernel array is NULL.");
        return CM_INVALID_ARG_VALUE;
    }
    while( pKernelArray[ kernelCount ] )
    {
        kernelCount++;
    }

    if( kernelCount < CM_MINIMUM_NUM_KERNELS_ENQWHINTS )
    {
        CM_ASSERTMESSAGE("Error: EnqueueWithHints requires at least 2 kernels.");
        return CM_FAILURE;
    }

    uint32_t totalThreadCount = 0;
    for( uint32_t i = 0; i < kernelCount; i ++ )
    {
        uint32_t threadCount = 0;
        pKernelArray[i]->GetThreadCount( threadCount );
        totalThreadCount += threadCount;
    }

    if( GetTaskHasThreadArg(pKernelArray, kernelCount, threadArgExists) != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Thread argument checking fails.");
        return CM_FAILURE;
    }

    if( !threadArgExists )
    {
        if (totalThreadCount > m_pHalMaxValues->maxUserThreadsPerTaskNoThreadArg )
        {
            CM_ASSERTMESSAGE("Error: Maximum number of threads per task exceeded.");
            return CM_EXCEED_MAX_THREAD_AMOUNT_PER_ENQUEUE;
        }
    }
    else
    {
        if( totalThreadCount > m_pHalMaxValues->maxUserThreadsPerTask )
        {
            CM_ASSERTMESSAGE("Error: Maximum number of threads per task exceeded.");
            return CM_EXCEED_MAX_THREAD_AMOUNT_PER_ENQUEUE;
        }
    }

    CLock Locker(m_CriticalSection_TaskInternal);

    result = CmTaskInternal::Create( kernelCount, totalThreadCount, pKernelArray, pTask, numTasksGenerated, isLastTask, hints, m_pDevice );
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Create CM task internal failure.");
        return result;
    }

    LARGE_INTEGER nEnqueueTime;
    if ( !(MOS_QueryPerformanceCounter( (uint64_t*)&nEnqueueTime.QuadPart )) )
    {
        CM_ASSERTMESSAGE("Error: Query performance counter failure.");
        return CM_FAILURE;
    }

    result = CreateEvent(pTask, bIsEventVisible, taskDriverId, pEvent);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Create event failure.");
        return result;
    }
    if ( pEvent != nullptr )
    {
        pEvent->SetEnqueueTime( nEnqueueTime );
    }

    for( uint32_t i = 0; i < kernelCount; ++i )
    {
        CmKernelRT* pKernel = nullptr;
        pTask->GetKernel(i, pKernel);
        if( pKernel != nullptr )
        {
            pKernel->SetAdjustedYCoord(0);
        }
    }

    pTask->SetPowerOption( pPowerOption );

    if (!m_EnqueuedTasks.Push(pTask))
    {
        CM_ASSERTMESSAGE("Error: Push enqueued tasks failure.")
        return CM_FAILURE;
    }

    result = FlushTaskWithoutSync();

    return result;
}

//*-----------------------------------------------------------------------------
//! Function to enqueue task with thread group space pointer
//! Arguments:
//!     1. Pointer to CmTask, which can only contain one kernel.
//!     2. Reference to the pointer to CmEvent that is to be returned
//!     3. Pointer to a CmThreadGroupSpace.
//! Return Value:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated
//!     CM_OUT_OF_HOST_MEMORY if out of host memory
//!     CM_FAILURE otherwise
//! Notes:
//!     If the kernel has per thread arg, GPGPU object is to be used.
//!     If the kernel has no per thread  arg. GPGPU walker is used.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueWithGroup( CmTask* pTask, CmEvent* & pEvent, const CmThreadGroupSpace* pThreadGroupSpace)
{
    INSERT_API_CALL_LOG();

    int32_t result;

    if(pTask == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Kernel array is NULL.");
        return CM_INVALID_ARG_VALUE;
    }

    CmTaskRT *pTaskRT = static_cast<CmTaskRT *>(pTask);
    uint32_t count = 0;
    count = pTaskRT->GetKernelCount();

    if( count == 0 )
    {
        CM_ASSERTMESSAGE("Error: There are no valid kernels.");
        return CM_FAILURE;
    }

    if(m_pDevice->IsPrintEnable())
    {
        m_pDevice->ClearPrintBuffer();
    }

    typedef CmKernelRT* pCmKernel;
    CmKernelRT** pTmp = MOS_NewArray(pCmKernel, (count+1));
    if(pTmp == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    uint32_t totalThreadNumber = 0;
    for(uint32_t i = 0; i < count; i++)
    {
        uint32_t singleThreadNumber = 0;
        pTmp[ i ] = pTaskRT->GetKernelPointer(i);

        //Thread arguments is not allowed in GPGPU_WALKER path
        if(pTmp[i]->IsThreadArgExisted())
        {
            CM_ASSERTMESSAGE("Error: No thread Args allowed when using group space");
            MosSafeDeleteArray(pTmp);
            return CM_THREAD_ARG_NOT_ALLOWED;
        }

        pTmp[i]->GetThreadCount(singleThreadNumber);
        totalThreadNumber += singleThreadNumber;
    }
    pTmp[count ] = nullptr;

    CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
    result = Enqueue_RT( pTmp, count, totalThreadNumber, pEventRT,
                         pThreadGroupSpace, pTaskRT->GetSyncBitmap(),
                         pTaskRT->GetPowerOption(),
                         pTaskRT->GetTaskConfig());

    if (pEventRT)
    {
        pEventRT->SetKernelNames(pTaskRT, nullptr, const_cast<CmThreadGroupSpace*>(pThreadGroupSpace));
    }

    pEvent = pEventRT;
    MosSafeDeleteArray( pTmp );

    return result;
}

CM_RT_API int32_t CmQueueRT::EnqueueWithHints(
                                        CmTask* pKernelArray,
                                        CmEvent* & pEvent,
                                        uint32_t hints)
{
    INSERT_API_CALL_LOG();

    int32_t            hr                = CM_FAILURE;
    uint32_t           count             = 0;
    uint32_t           index             = 0;
    CmKernelRT**         pKernels          = nullptr;
    uint32_t           numTasks          = 0;
    bool               splitTask         = false;
    bool               lastTask          = false;
    uint32_t           numTasksGenerated = 0;
    CmEventRT          *pEventRT = static_cast<CmEventRT *>(pEvent);

    if (pKernelArray == nullptr)
    {
        return CM_INVALID_ARG_VALUE;
    }
    CmTaskRT         *pKernelArrayRT   = static_cast<CmTaskRT *>(pKernelArray);
    count = pKernelArrayRT->GetKernelCount();
    if( count == 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel count.");
        hr = CM_FAILURE;
        goto finish;
    }

    if( count > m_pHalMaxValues->maxKernelsPerTask )
    {
        CM_ASSERTMESSAGE("Error: Kernel count exceeds maximum kernel per enqueue.");
        hr = CM_EXCEED_MAX_KERNEL_PER_ENQUEUE;
        goto finish;
    }

    for (uint32_t i = 0; i < count; ++i)
    {
        CmKernelRT* pKernelTmp = nullptr;
        CmThreadSpaceRT* pTSTmp = nullptr;
        pKernelTmp = pKernelArrayRT->GetKernelPointer(i);
        CMCHK_NULL(pKernelTmp);
        pKernelTmp->GetThreadSpace(pTSTmp);
        CMCHK_NULL(pTSTmp);
        if (pTSTmp->GetNeedSetKernelPointer() && pTSTmp->KernelPointerIsNULL())
        {
            pTSTmp->SetKernelPointer(pKernelTmp);
        }
    }

#if _DEBUG
    if( !pKernelArrayRT->IntegrityCheckKernelThreadspace() )
    {
        CM_ASSERTMESSAGE("Error: Integrity check for kernel thread space failed.");
        hr = CM_KERNEL_THREADSPACE_INTEGRITY_FAILED;
        goto finish;
    }
#endif

    numTasks = ( hints & CM_HINTS_MASK_NUM_TASKS ) >> CM_HINTS_NUM_BITS_TASK_POS;
    if( numTasks > 1 )
    {
        splitTask = true;
    }

    if( m_pDevice->IsPrintEnable() )
    {
        m_pDevice->ClearPrintBuffer();
    }

    pKernels = MOS_NewArray(CmKernelRT*, (count + 1));
    CMCHK_NULL(pKernels);

    do
    {
        for (index = 0; index < count; ++index)
        {
            pKernels[ index ] = pKernelArrayRT->GetKernelPointer( index );
        }

        pKernels[ count ] = nullptr;

        if(splitTask)
        {
            if( numTasksGenerated == (numTasks - 1 ) )
            {
                lastTask = true;
            }
        }
        else
        {
            lastTask = true;
        }

        CMCHK_HR(Enqueue_RT( pKernels, pEventRT, numTasksGenerated, lastTask, hints, pKernelArrayRT->GetPowerOption() ));
        pEvent = pEventRT;
        numTasksGenerated++;

    }while(numTasksGenerated < numTasks);

finish:
    MosSafeDeleteArray( pKernels );

    return hr;
}

//*-----------------------------------------------------------------------------
//! Enqueue an task, which contains one pre-defined kernel to
//! copy from host memory to surface
//! This is a non-blocking call. i.e. it returns immediately without waiting for
//! GPU to finish the execution of the task.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can
//! be used to check if the task finishs.
//! INPUT:
//!     1) Pointer to the CmSurface2D_RT as copy destination
//!     2) Pointer to the host memory as copy source
//!     3) Reference to the pointer to CMEvent
//!     4) A boolean value to indicate if or not to flush the queue after enqueue the task
//!        by default the boolean value is TRUE.
//! OUTPUT:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated;
//!     CM_OUT_OF_HOST_MEMORY if out of host memery;
//!     CM_FAILURE otherwise.
//!     More error code is coming.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueCopyCPUToGPU( CmSurface2D* pSurface, const unsigned char* pSysMem, CmEvent* & pEvent )
{
    INSERT_API_CALL_LOG();

    CmSurface2DRT *pSurfaceRT = static_cast<CmSurface2DRT *>(pSurface);
    return EnqueueCopyInternal(pSurfaceRT, (unsigned char*)pSysMem, 0, 0, CM_FASTCOPY_CPU2GPU, CM_FASTCOPY_OPTION_NONBLOCKING, pEvent);
}

//*-----------------------------------------------------------------------------
//! Enqueue an task, which contains one pre-defined kernel to
//! copy from surface to host memory
//! This is a non-blocking call. i.e. it returns immediately without waiting for
//! GPU to finish the execution of the task.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can
//! be used to check if the task finishs.
//! INPUT:
//!     1) Pointer to the CmSurface2D_RT as copy source
//!     2) Pointer to the host memory as copy destination
//!     3) Reference to the pointer to CMEvent
//!     4) A boolean value to indicate if or not to flush the queue after enqueue the task
//!        by default the boolean value is TRUE.
//! OUTPUT:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated;
//!     CM_OUT_OF_HOST_MEMORY if out of host memery;
//!     CM_FAILURE otherwise.
//!     More error code is coming.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueCopyGPUToCPU( CmSurface2D* pSurface, unsigned char* pSysMem, CmEvent* & pEvent )
{
    INSERT_API_CALL_LOG();

    CmSurface2DRT *pSurfaceRT = static_cast<CmSurface2DRT *>(pSurface);
    return EnqueueCopyInternal(pSurfaceRT, pSysMem, 0, 0, CM_FASTCOPY_GPU2CPU, CM_FASTCOPY_OPTION_NONBLOCKING, pEvent);
}

int32_t CmQueueRT::EnqueueUnalignedCopyInternal( CmSurface2DRT* pSurface, unsigned char* pSysMem, const uint32_t widthStride, const uint32_t heightStride, CM_GPUCOPY_DIRECTION direction, CmEvent* &pEvent )
{
    int32_t         hr                          = CM_SUCCESS;
    uint32_t        bufferup_size               = 0;
    uint32_t        dstAddShiftOffset           = 0;
    uint32_t        threadWidth                 = 0;
    uint32_t        threadHeight                = 0;
    uint32_t        threadNum                   = 0;
    uint32_t        auxiliary_bufferup_size     = 0;
    uint32_t        width                       = 0;
    uint32_t        height                      = 0;
    uint32_t        sizePerPixel                = 0;
    uint32_t        width_byte                  = 0;
    uint32_t        copy_width_byte             = 0;
    uint32_t        copy_height_row             = 0;
    uint32_t        stride_in_bytes             = widthStride;
    uint32_t        height_stride_in_rows       = heightStride;
    size_t          pLinearAddress              = (size_t)pSysMem;
    size_t          pLinearAddressAligned       = 0;
    unsigned char*  pHybridCopyAuxSysMem        = nullptr;

    CmBufferUP             *pBufferUP                  = nullptr;
    CmKernel               *pKernel                    = nullptr;
    CmBufferUP             *pHybridCopyAuxBufferUP     = nullptr;
    SurfaceIndex           *pBufferIndexCM             = nullptr;
    SurfaceIndex           *pHybridCopyAuxIndexCM      = nullptr;
    SurfaceIndex           *pSurf2DIndexCM             = nullptr;
    CmThreadSpace          *pTS                        = nullptr;
    CmQueue                *pCmQueue                   = nullptr;
    CmTask                 *pGPUCopyTask               = nullptr;
    CmProgram              *pGPUcopyProgram            = nullptr;
    CM_STATUS              status;
    CM_SURFACE_FORMAT      format;

    if ( pSurface )
    {
        CMCHK_HR( pSurface->GetSurfaceDesc(width, height, format, sizePerPixel));
    }
    else
    {
        return CM_FAILURE;
    }

    width_byte                  = width * sizePerPixel;
    // the actual copy region
    copy_width_byte             = MOS_MIN(stride_in_bytes, width_byte);
    copy_height_row             = MOS_MIN(height_stride_in_rows, height);

    if(pLinearAddress == 0)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.");
        return CM_INVALID_ARG_VALUE;
    }
    if( (copy_width_byte > CM_MAX_THREADSPACE_WIDTH_FOR_MW * BLOCK_WIDTH ) || ( copy_height_row > CM_MAX_THREADSPACE_HEIGHT_FOR_MW * BLOCK_HEIGHT) )
    {  // each thread handles 64x8 block data. This API will fail if it exceeds the max thread space's size
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_INVALID_ARG_SIZE;
    }

    if (sizeof (void *) == 8 ) //64-bit
    {
        pLinearAddressAligned        = pLinearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X64;
    }
    else  //32-bit
    {
        pLinearAddressAligned        = pLinearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X86;
    }
    //Calculate  Left Shift offset
    dstAddShiftOffset               = (uint32_t)(pLinearAddress - pLinearAddressAligned);

    if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
    {
        bufferup_size = MOS_ALIGN_CEIL(stride_in_bytes * (height_stride_in_rows + copy_height_row * 1/2) + (uint32_t)dstAddShiftOffset , 64);
    }
    else
    {
        bufferup_size = MOS_ALIGN_CEIL(stride_in_bytes * height_stride_in_rows  + (uint32_t)dstAddShiftOffset, 64);
    }

    CMCHK_HR(m_pDevice->CreateBufferUP(bufferup_size, ( void * )pLinearAddressAligned, pBufferUP));
    CMCHK_HR(pBufferUP->GetIndex(pBufferIndexCM));
    CMCHK_HR(pSurface->GetIndex(pSurf2DIndexCM));

    CMCHK_HR( m_pDevice->LoadPredefinedCopyKernel(pGPUcopyProgram));
    CMCHK_NULL(pGPUcopyProgram);

    if (direction == CM_FASTCOPY_CPU2GPU)
    {
        if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
        {
            CMCHK_HR(m_pDevice->CreateKernel(pGPUcopyProgram, _NAME(surfaceCopy_write_unaligned_NV12), pKernel, "PredefinedGPUCopyKernel"));
        }
        else
        {
            CMCHK_HR(m_pDevice->CreateKernel(pGPUcopyProgram, _NAME(surfaceCopy_write_unaligned), pKernel, "PredefinedGPUCopyKernel"));

        }
        CMCHK_HR(pKernel->SetKernelArg( 0, sizeof( SurfaceIndex ), pBufferIndexCM ));
        CMCHK_HR(pKernel->SetKernelArg( 1, sizeof( SurfaceIndex ), pSurf2DIndexCM ));
    }
    else
    {
        if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
        {
            CMCHK_HR(m_pDevice->CreateKernel(pGPUcopyProgram, _NAME(surfaceCopy_read_unaligned_NV12), pKernel, "PredefinedGPUCopyKernel"));
            auxiliary_bufferup_size = BLOCK_WIDTH * 2 * (height_stride_in_rows + copy_height_row * 1/2);
        }
        else
        {
            CMCHK_HR(m_pDevice->CreateKernel(pGPUcopyProgram, _NAME(surfaceCopy_read_unaligned), pKernel, "PredefinedGPUCopyKernel"));
            auxiliary_bufferup_size = BLOCK_WIDTH * 2 * height_stride_in_rows;
        }
        pHybridCopyAuxSysMem = (unsigned char*)MOS_AlignedAllocMemory(auxiliary_bufferup_size, PAGE_ALIGNED);
        if(!pHybridCopyAuxSysMem)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
        CMCHK_HR(m_pDevice->CreateBufferUP(auxiliary_bufferup_size, (void*)pHybridCopyAuxSysMem, pHybridCopyAuxBufferUP));
        CMCHK_HR(pHybridCopyAuxBufferUP->GetIndex(pHybridCopyAuxIndexCM));

        CMCHK_HR(pKernel->SetKernelArg( 0, sizeof( SurfaceIndex ), pSurf2DIndexCM ));
        CMCHK_HR(pKernel->SetKernelArg( 1, sizeof( SurfaceIndex ), pBufferIndexCM ));
        CMCHK_HR(pKernel->SetKernelArg( 5, sizeof( uint32_t ), &width_byte ));
        CMCHK_HR(pKernel->SetKernelArg( 6, sizeof( SurfaceIndex ), pHybridCopyAuxIndexCM ));
    }

    CMCHK_HR(pKernel->SetKernelArg( 2, sizeof( uint32_t ), &stride_in_bytes ));
    CMCHK_HR(pKernel->SetKernelArg( 3, sizeof( uint32_t ), &height_stride_in_rows ));
    CMCHK_HR(pKernel->SetKernelArg( 4, sizeof( uint32_t ), &dstAddShiftOffset ));

    threadWidth = ( uint32_t )ceil( ( double )copy_width_byte/BLOCK_WIDTH );
    threadHeight = ( uint32_t )ceil( ( double )copy_height_row/BLOCK_HEIGHT );

    threadNum = threadWidth * threadHeight;
    CMCHK_HR(pKernel->SetThreadCount( threadNum ));

    CMCHK_HR(m_pDevice->CreateThreadSpace( threadWidth, threadHeight, pTS ));
    CMCHK_HR(m_pDevice->CreateQueue( pCmQueue ));
    CMCHK_HR(m_pDevice->CreateTask(pGPUCopyTask));
    CMCHK_HR(pGPUCopyTask->AddKernel( pKernel ));
    CMCHK_HR(pCmQueue->Enqueue( pGPUCopyTask, pEvent, pTS ));

    if(pEvent)
    {
        CMCHK_HR(pEvent->GetStatus(status));
        while(status != CM_STATUS_FINISHED)
        {
            if (status == CM_STATUS_RESET)
            {
                hr = CM_TASK_MEDIA_RESET;
                goto finish;
            }
            CMCHK_HR(pEvent->GetStatus(status));
        }
    }
    // CPU copy unaligned data
    if( direction == CM_FASTCOPY_GPU2CPU)
    {
        uint32_t beginLineCopySize   = 0;
        uint32_t readOffset = 0;
        uint32_t copy_lines = 0;
        size_t beginLineWriteOffset = 0;
        uint32_t mod = 0;
        uint32_t alignedWrites = 0;
        uint32_t endLineWriteOffset = 0;
        uint32_t endLineCopySize = 0;
        unsigned char* pStartBuffer = (unsigned char*)pLinearAddressAligned;

        copy_lines = (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016) ? height_stride_in_rows + MOS_MIN(height_stride_in_rows, height) * 1 / 2 : height_stride_in_rows;

        for(uint32_t i = 0; i < copy_lines; ++i)
        {
            //copy begining of line
            beginLineWriteOffset = stride_in_bytes * i + dstAddShiftOffset;
            mod = ((uintptr_t)pStartBuffer + beginLineWriteOffset) < BLOCK_WIDTH ? ((uintptr_t)pStartBuffer + beginLineWriteOffset) : ((uintptr_t)pStartBuffer + beginLineWriteOffset) & (BLOCK_WIDTH - 1);
            beginLineCopySize = (mod == 0) ? 0:(BLOCK_WIDTH - mod);
            //fix copy size for cases where the surface width is small
            if((beginLineCopySize > width_byte) || ( beginLineCopySize == 0 && width_byte < BLOCK_WIDTH ) )
            {
                beginLineCopySize = width_byte;
            }
            if(beginLineCopySize > 0)
            {
                CmSafeMemCopy((void *)( (unsigned char *)pStartBuffer + beginLineWriteOffset), (void *)(pHybridCopyAuxSysMem + readOffset), beginLineCopySize);
            }

            //copy end of line
            alignedWrites = (width_byte - beginLineCopySize) &~ (BLOCK_WIDTH - 1);
            endLineWriteOffset = beginLineWriteOffset + alignedWrites + beginLineCopySize;
            endLineCopySize = dstAddShiftOffset+ i * stride_in_bytes + width_byte - endLineWriteOffset;
            if(endLineCopySize > 0 && endLineWriteOffset > beginLineWriteOffset)
            {
                CmSafeMemCopy((void *)((unsigned char *)pStartBuffer + endLineWriteOffset), (void *)(pHybridCopyAuxSysMem + readOffset + BLOCK_WIDTH), endLineCopySize);
            }
            readOffset += (BLOCK_WIDTH * 2);
        }
    }

    CMCHK_HR(m_pDevice->DestroyTask(pGPUCopyTask));
    CMCHK_HR(m_pDevice->DestroyThreadSpace(pTS));
    CMCHK_HR(m_pDevice->DestroyBufferUP(pBufferUP));
    if (direction == CM_FASTCOPY_GPU2CPU)
    {
        if(pHybridCopyAuxBufferUP)
        {
            CMCHK_HR(m_pDevice->DestroyBufferUP(pHybridCopyAuxBufferUP));
        }
        if(pHybridCopyAuxSysMem)
        {
            MOS_AlignedFreeMemory(pHybridCopyAuxSysMem);
            pHybridCopyAuxSysMem = nullptr;
        }
    }
finish:
    if(hr != CM_SUCCESS)
    {
        if(pBufferUP == nullptr)
        {
            // user need to know whether the failure is caused by out of BufferUP.
            hr = CM_GPUCOPY_OUT_OF_RESOURCE;
        }

        if(pKernel)                         m_pDevice->DestroyKernel(pKernel);
        if(pTS)                             m_pDevice->DestroyThreadSpace(pTS);
        if(pGPUCopyTask)                    m_pDevice->DestroyTask(pGPUCopyTask);
        if(pBufferUP)                       m_pDevice->DestroyBufferUP(pBufferUP);
        if(pHybridCopyAuxBufferUP)          m_pDevice->DestroyBufferUP(pHybridCopyAuxBufferUP);
        if(pHybridCopyAuxSysMem)            { MOS_AlignedFreeMemory(pHybridCopyAuxSysMem); pHybridCopyAuxSysMem = nullptr; }
    }

    return hr;
}
//*-----------------------------------------------------------------------------
//! Enqueue an task, which contains one pre-defined kernel to
//! copy from surface to host memory or from host memory to surface
//! This is a non-blocking call. i.e. it returns immediately without waiting for
//! GPU to finish the execution of the task.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can
//! be used to check if the task finishes.
//! INPUT:
//!     1) Pointer to the CmSurface2D
//!     2) Pointer to the host memory
//!     3) Width stride in bytes, if there is no padding in system memroy, it is set to zero.
//!     4) Height stride in row, if there is no padding in system memroy, it is set to zero.
//!     4) Copy direction, cpu->gpu (linear->tiled) or gpu->cpu(tiled->linear)
//!     5) Reference to the pointer to CMEvent
//! OUTPUT:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated;
//!     CM_OUT_OF_HOST_MEMORY if out of host memery;
//!     CM_FAILURE otherwise.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::EnqueueCopyInternal(CmSurface2DRT* pSurface,
                                unsigned char* pSysMem,
                                const uint32_t widthStride,
                                const uint32_t heightStride,
                                CM_GPUCOPY_DIRECTION direction,
                                const uint32_t option,
                                CmEvent* & pEvent)
{
    int32_t hr                  = CM_FAILURE;
    uint32_t width               = 0;
    uint32_t height              = 0;
    uint32_t sizePerPixel        = 0;
    CM_SURFACE_FORMAT format    = CM_SURFACE_FORMAT_INVALID;

    if (pSurface)
    {
        CMCHK_HR(pSurface->GetSurfaceDesc(width, height, format, sizePerPixel));
    }
    else
    {
        return CM_GPUCOPY_INVALID_SURFACES;
    }

    if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
    {
        hr = EnqueueCopyInternal_2Planes(pSurface, (unsigned char*)pSysMem, format, width, widthStride, height, heightStride, sizePerPixel, direction, option, pEvent);
    }
    else
    {
        hr = EnqueueCopyInternal_1Plane(pSurface, (unsigned char*)pSysMem, format, width, widthStride, height, heightStride, sizePerPixel, direction, option, pEvent);
    }

finish:
    return hr;
}

int32_t CmQueueRT::EnqueueCopyInternal_1Plane(CmSurface2DRT* pSurface,
                                    unsigned char* pSysMem,
                                    CM_SURFACE_FORMAT format,
                                    const uint32_t widthInPixel,
                                    const uint32_t widthStride,
                                    const uint32_t heightInRow,
                                    const uint32_t heightStride,
                                    const uint32_t sizePerPixel,
                                    CM_GPUCOPY_DIRECTION direction,
                                    const uint32_t option,
                                    CmEvent* & pEvent )
{
    int32_t         hr                      = CM_SUCCESS;
    uint32_t        tempHeight              = heightInRow;
    uint32_t        stride_in_bytes         = widthStride;
    uint32_t        stride_in_dwords        = 0;
    uint32_t        height_stride_in_rows   = heightStride;
    uint32_t        AddedShiftLeftOffset    = 0;
    size_t          pLinearAddress          = (size_t)pSysMem;
    size_t          pLinearAddressAligned   = 0;

    CmKernel        *pKernel            = nullptr;
    CmBufferUP      *pCMBufferUP        = nullptr;
    SurfaceIndex    *pBufferIndexCM     = nullptr;
    SurfaceIndex    *pSurf2DIndexCM     = nullptr;
    CmThreadSpace   *pTS                = nullptr;
    CmQueue         *pCmQueue           = nullptr;
    CmTask          *pGPUCopyTask       = nullptr;
    CmEvent         *pInternalEvent     = nullptr;

    uint32_t        threadWidth             = 0;
    uint32_t        threadHeight            = 0;
    uint32_t        threadNum               = 0;
    uint32_t        width_dword             = 0;
    uint32_t        width_byte              = 0;
    uint32_t        copy_width_byte         = 0;
    uint32_t        copy_height_row         = 0;
    uint32_t        slice_copy_height_row   = 0;
    uint32_t        sliceCopyBufferUPSize   = 0;
    int32_t         totalBufferUPSize       = 0;
    uint32_t        start_x                 = 0;
    uint32_t        start_y                 = 0;
    bool            blSingleEnqueue         = true;
    CM_GPUCOPY_KERNEL *pGPUCopyKrnParam     = nullptr;

    PCM_HAL_STATE   pCmHalState    =        \
        ((PCM_CONTEXT_DATA)m_pDevice->GetAccelData())->cmHalState;

    width_byte    = widthInPixel * sizePerPixel;

    //Align the width regarding stride
   if(stride_in_bytes == 0)
   {
        stride_in_bytes = width_byte;
   }

   if(height_stride_in_rows == 0)
   {
        height_stride_in_rows = heightInRow;
   }

    // the actual copy region
    copy_width_byte = MOS_MIN(stride_in_bytes, width_byte);
    copy_height_row = MOS_MIN(height_stride_in_rows, heightInRow);

    // Make sure stride and start address of system memory is 16-byte aligned.
    // if no padding in system memory , stride_in_bytes = width_byte.
    if(stride_in_bytes & 0xf)
    {
        CM_ASSERTMESSAGE("Error: Stride is not 16-byte aligned.");
        return CM_GPUCOPY_INVALID_STRIDE;
    }
    if((pLinearAddress & 0xf) || (pLinearAddress == 0))
    {
        CM_ASSERTMESSAGE("Error: Start address of system memory is not 16-byte aligned.");
        return CM_GPUCOPY_INVALID_SYSMEM;
    }

    //Calculate actual total size of system memory
    totalBufferUPSize = stride_in_bytes * height_stride_in_rows;

    //Check thread space width here
    if( copy_width_byte > CM_MAX_THREADSPACE_WIDTH_FOR_MW * BLOCK_PIXEL_WIDTH *4 )
    {  // each thread handles 128x8 block data. This API will fail if it exceeds the max thread space's size
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_GPUCOPY_INVALID_SIZE;
    }

    while (totalBufferUPSize > 0)
    {
        if (sizeof (void *) == 8 ) //64-bit
        {
            pLinearAddressAligned        = pLinearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X64;
        }
        else  //32-bit
        {
            pLinearAddressAligned        = pLinearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X86;
        }

        //Calculate  Left Shift offset
        AddedShiftLeftOffset = (uint32_t)(pLinearAddress - pLinearAddressAligned);
        totalBufferUPSize   += AddedShiftLeftOffset;

        if (totalBufferUPSize > CM_MAX_1D_SURF_WIDTH)
        {
            blSingleEnqueue = false;
            slice_copy_height_row = ((CM_MAX_1D_SURF_WIDTH - AddedShiftLeftOffset)/(stride_in_bytes*(BLOCK_HEIGHT * INNER_LOOP))) * (BLOCK_HEIGHT * INNER_LOOP);
            sliceCopyBufferUPSize = slice_copy_height_row * stride_in_bytes + AddedShiftLeftOffset;
            tempHeight = slice_copy_height_row;
        }
        else
        {
            slice_copy_height_row = copy_height_row;
            sliceCopyBufferUPSize = totalBufferUPSize;
            if (!blSingleEnqueue)
            {
                tempHeight = slice_copy_height_row;
            }
        }

        //Check thread space height here
        if(slice_copy_height_row > CM_MAX_THREADSPACE_HEIGHT_FOR_MW * BLOCK_HEIGHT * INNER_LOOP )
        {  // each thread handles 128x8 block data. This API will fail if it exceeds the max thread space's size
            CM_ASSERTMESSAGE("Error: Invalid copy size.");
            return CM_GPUCOPY_INVALID_SIZE;
        }

        pKernel = nullptr;
        CMCHK_HR( m_pDevice->CreateBufferUP(  sliceCopyBufferUPSize, ( void * )pLinearAddressAligned, pCMBufferUP ));
        CMCHK_HR(CreateGPUCopyKernel(copy_width_byte, slice_copy_height_row, format, direction, pGPUCopyKrnParam));
        CMCHK_NULL(pGPUCopyKrnParam);
        pKernel = pGPUCopyKrnParam->pKernel;

        CMCHK_NULL(pKernel);

        CMCHK_NULL(pCMBufferUP);
        CMCHK_HR(pCMBufferUP->GetIndex( pBufferIndexCM ));
        CMCHK_HR(pSurface->GetIndex( pSurf2DIndexCM ));

        threadWidth = ( uint32_t )ceil( ( double )copy_width_byte/BLOCK_PIXEL_WIDTH/4 );
        threadHeight = ( uint32_t )ceil( ( double )slice_copy_height_row/BLOCK_HEIGHT/INNER_LOOP );
        threadNum = threadWidth * threadHeight;
        CMCHK_HR(pKernel->SetThreadCount( threadNum ));
        CMCHK_HR(m_pDevice->CreateThreadSpace( threadWidth, threadHeight, pTS ));

        if( direction == CM_FASTCOPY_CPU2GPU)
        {
            if (pCmHalState->pCmHalInterface->IsSurfaceCompressionWARequired())
            {
                CMCHK_HR(pSurface->SetCompressionMode(MEMCOMP_DISABLED));
            }
            CMCHK_HR(pKernel->SetKernelArg( 0, sizeof( SurfaceIndex ), pBufferIndexCM) );
            CMCHK_HR(pKernel->SetKernelArg( 1, sizeof( SurfaceIndex ), pSurf2DIndexCM ));
        }
        else
        {
            CMCHK_HR(pKernel->SetKernelArg( 1, sizeof( SurfaceIndex ), pBufferIndexCM ));
            CMCHK_HR(pKernel->SetKernelArg( 0, sizeof( SurfaceIndex ), pSurf2DIndexCM ));
        }

        if(direction == CM_FASTCOPY_GPU2CPU)
        {
            pSurface->SetReadSyncFlag(true); // GPU -> CPU, set surf2d as read sync flag
        }

        width_dword = (uint32_t)ceil((double)width_byte / 4);
        stride_in_dwords = (uint32_t)ceil((double)stride_in_bytes / 4);

        CMCHK_HR(pKernel->SetKernelArg( 2, sizeof( uint32_t ), &stride_in_dwords ));
        CMCHK_HR(pKernel->SetKernelArg( 3, sizeof( uint32_t ), &height_stride_in_rows ));
        CMCHK_HR(pKernel->SetKernelArg( 4, sizeof( uint32_t ), &AddedShiftLeftOffset ));
        CMCHK_HR(pKernel->SetKernelArg( 5, sizeof( uint32_t ), &threadHeight ));

        if (direction == CM_FASTCOPY_GPU2CPU)  //GPU-->CPU, read
        {
            CMCHK_HR(pKernel->SetKernelArg( 6, sizeof( uint32_t ), &width_dword ));
            CMCHK_HR(pKernel->SetKernelArg( 7, sizeof( uint32_t ), &tempHeight ));
            CMCHK_HR(pKernel->SetKernelArg( 8, sizeof(uint32_t), &start_x));
            CMCHK_HR(pKernel->SetKernelArg( 9, sizeof(uint32_t), &start_y));
        }
        else  //CPU-->GPU, write
        {
            //this only works for the kernel surfaceCopy_write_32x32
            CMCHK_HR(pKernel->SetKernelArg( 6, sizeof( uint32_t ), &start_x ));
            CMCHK_HR(pKernel->SetKernelArg( 7, sizeof( uint32_t ), &start_y ));
        }

        CMCHK_HR(m_pDevice->CreateQueue( pCmQueue ));
        CMCHK_HR(m_pDevice->CreateTask(pGPUCopyTask));
        CMCHK_HR(pGPUCopyTask->AddKernel( pKernel ));
        if (option & CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST)
        {
            // disable turbo
            CM_TASK_CONFIG taskConfig;
            CmSafeMemSet(&taskConfig, 0, sizeof(CM_TASK_CONFIG));
            taskConfig.turboBoostFlag = CM_TURBO_BOOST_DISABLE;
            pGPUCopyTask->SetProperty(taskConfig);
        }
        CMCHK_HR(pCmQueue->Enqueue( pGPUCopyTask, pInternalEvent, pTS ));

        if( pGPUCopyKrnParam )
        {
            GPUCOPY_KERNEL_UNLOCK(pGPUCopyKrnParam);
        }

        //update for next slice
        pLinearAddress += sliceCopyBufferUPSize - AddedShiftLeftOffset;
        totalBufferUPSize -= sliceCopyBufferUPSize;
        copy_height_row -= slice_copy_height_row;
        start_x = 0;
        start_y += slice_copy_height_row;

        if(totalBufferUPSize > 0)   //Intermediate event, we don't need it
        {
            CMCHK_HR(pCmQueue->DestroyEvent(pInternalEvent));
        }
        else //Last one event, need keep or destroy it
        {
            if ((option & CM_FASTCOPY_OPTION_BLOCKING) && (pInternalEvent))
            {
                CMCHK_HR(pInternalEvent->WaitForTaskFinished());
            }

            if(pEvent == CM_NO_EVENT)  //User doesn't need CmEvent for this copy
            {
                pEvent = nullptr;
                CMCHK_HR(pCmQueue->DestroyEvent(pInternalEvent));
            }
            else //User needs this CmEvent
            {
                pEvent = pInternalEvent;
            }
        }

        CMCHK_HR(m_pDevice->DestroyTask(pGPUCopyTask));
        CMCHK_HR(m_pDevice->DestroyThreadSpace(pTS));
        CMCHK_HR(m_pDevice->DestroyBufferUP(pCMBufferUP));
    }

finish:

    if(hr != CM_SUCCESS)
    {
        if(pCMBufferUP == nullptr)
        {
            // user need to know whether the failure is caused by out of BufferUP.
            hr = CM_GPUCOPY_OUT_OF_RESOURCE;
        }

        if(pKernel && pGPUCopyKrnParam)        GPUCOPY_KERNEL_UNLOCK(pGPUCopyKrnParam);
        if(pTS)                                m_pDevice->DestroyThreadSpace(pTS);
        if(pGPUCopyTask)                       m_pDevice->DestroyTask(pGPUCopyTask);
        if(pCMBufferUP)                        m_pDevice->DestroyBufferUP(pCMBufferUP);
        if(pInternalEvent)                     pCmQueue->DestroyEvent(pInternalEvent);

        // CM_FAILURE for all the other errors
        // return CM_EXCEED_MAX_TIMEOUT to notify app that gpu reset happens
        if( hr != CM_GPUCOPY_OUT_OF_RESOURCE && hr != CM_EXCEED_MAX_TIMEOUT)
        {
            hr = CM_FAILURE;
        }
    }

    return hr;
}

int32_t CmQueueRT::EnqueueCopyInternal_2Planes(CmSurface2DRT* pSurface,
                                        unsigned char* pSysMem,
                                        CM_SURFACE_FORMAT format,
                                        const uint32_t widthInPixel,
                                        const uint32_t widthStride,
                                        const uint32_t heightInRow,
                                        const uint32_t heightStride,
                                        const uint32_t sizePerPixel,
                                        CM_GPUCOPY_DIRECTION direction,
                                        const uint32_t option,
                                        CmEvent* & pEvent)
{
    int32_t         hr                      = CM_SUCCESS;
    uint32_t        stride_in_bytes         = widthStride;
    uint32_t        stride_in_dwords        = 0;
    uint32_t        height_stride_in_rows   = heightStride;
    size_t          pLinearAddress_Y        = 0;
    size_t          pLinearAddress_UV       = 0;
    size_t          pLinearAddressAligned_Y = 0;
    size_t          pLinearAddressAligned_UV = 0;
    uint32_t        AddedShiftLeftOffset_Y  = 0;
    uint32_t        AddedShiftLeftOffset_UV = 0;

    CmKernel        *pKernel                = nullptr;
    CmBufferUP      *pCMBufferUP_Y          = nullptr;
    CmBufferUP      *pCMBufferUP_UV         = nullptr;
    SurfaceIndex    *pBufferUPIndex_Y       = nullptr;
    SurfaceIndex    *pBufferUPIndex_UV      = nullptr;
    SurfaceIndex    *pSurf2DIndexCM         = nullptr;
    CmThreadSpace   *pTS                    = nullptr;
    CmQueue         *pCmQueue               = nullptr;
    CmTask          *pGPUCopyTask           = nullptr;
    CmEvent         *pInternalEvent         = nullptr;

    uint32_t        threadWidth             = 0;
    uint32_t        threadHeight            = 0;
    uint32_t        threadNum               = 0;
    uint32_t        width_dword             = 0;
    uint32_t        width_byte              = 0;
    uint32_t        copy_width_byte         = 0;
    uint32_t        copy_height_row         = 0;
    uint32_t        BufferUP_Y_Size         = 0;
    uint32_t        BufferUP_UV_Size        = 0;

    CM_GPUCOPY_KERNEL *pGPUCopyKrnParam = nullptr;
    PCM_HAL_STATE       pCmHalState    =      \
        ((PCM_CONTEXT_DATA)m_pDevice->GetAccelData())->cmHalState;

    width_byte = widthInPixel * sizePerPixel;

    //Align the width regarding stride
    if (stride_in_bytes == 0)
    {
        stride_in_bytes = width_byte;
    }

    if (height_stride_in_rows == 0)
    {
        height_stride_in_rows = heightInRow;
    }

    // the actual copy region
    copy_width_byte = MOS_MIN(stride_in_bytes, width_byte);
    copy_height_row = MOS_MIN(height_stride_in_rows, heightInRow);

    // Make sure stride and start address of system memory is 16-byte aligned.
    // if no padding in system memory , stride_in_bytes = width_byte.
    if (stride_in_bytes & 0xf)
    {
        CM_ASSERTMESSAGE("Error: Stride is not 16-byte aligned.");
        return CM_GPUCOPY_INVALID_STRIDE;
    }

    //Check thread space width here
    if (copy_width_byte > CM_MAX_THREADSPACE_WIDTH_FOR_MW * BLOCK_PIXEL_WIDTH * 4)
    {  // each thread handles 128x8 block data. This API will fail if it exceeds the max thread space's size
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_GPUCOPY_INVALID_SIZE;
    }

    pLinearAddress_Y = (size_t)pSysMem;
    pLinearAddress_UV = (size_t)((char*)pSysMem + stride_in_bytes * height_stride_in_rows);

    if ((pLinearAddress_Y & 0xf) || (pLinearAddress_Y == 0) || (pLinearAddressAligned_UV & 0xf))
    {
        CM_ASSERTMESSAGE("Error: Start address of system memory is not 16-byte aligned.");
        return CM_GPUCOPY_INVALID_SYSMEM;
    }

    if (sizeof (void *) == 8) //64-bit
    {
        pLinearAddressAligned_Y = pLinearAddress_Y & ADDRESS_PAGE_ALIGNMENT_MASK_X64;
        pLinearAddressAligned_UV = pLinearAddress_UV & ADDRESS_PAGE_ALIGNMENT_MASK_X64;
    }
    else  //32-bit
    {
        pLinearAddressAligned_Y = pLinearAddress_Y & ADDRESS_PAGE_ALIGNMENT_MASK_X86;
        pLinearAddressAligned_UV = pLinearAddress_UV & ADDRESS_PAGE_ALIGNMENT_MASK_X86;
    }

    //Calculate  Left Shift offset
    AddedShiftLeftOffset_Y = (uint32_t)(pLinearAddress_Y - pLinearAddressAligned_Y);
    AddedShiftLeftOffset_UV = (uint32_t)(pLinearAddress_UV - pLinearAddressAligned_UV);

    //Calculate actual total size of system memory, assume it's NV12/P010/P016 formats
    BufferUP_Y_Size = stride_in_bytes * height_stride_in_rows + AddedShiftLeftOffset_Y;
    BufferUP_UV_Size = stride_in_bytes * copy_height_row * 1 / 2 + AddedShiftLeftOffset_UV;

    //Check thread space height here
    if (copy_height_row > CM_MAX_THREADSPACE_HEIGHT_FOR_MW * BLOCK_HEIGHT * INNER_LOOP)
    {  // each thread handles 128x8 block data. This API will fail if it exceeds the max thread space's size
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_GPUCOPY_INVALID_SIZE;
    }

    pKernel = nullptr;
    CMCHK_HR(m_pDevice->CreateBufferUP(BufferUP_Y_Size, (void *)pLinearAddressAligned_Y, pCMBufferUP_Y));
    CMCHK_HR(m_pDevice->CreateBufferUP(BufferUP_UV_Size, (void *)pLinearAddressAligned_UV, pCMBufferUP_UV));

    //Configure memory object control for the two BufferUP to solve the same cache-line coherency issue.
    if (pCmHalState->pCmHalInterface->IsGPUCopySurfaceNoCacheWARequired())
    {
        CMCHK_HR(pCMBufferUP_Y->SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3));
        CMCHK_HR(pCMBufferUP_UV->SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3));
    }
    else
    {
        CMCHK_HR(static_cast< CmBuffer_RT* >(pCMBufferUP_Y)->SetMemoryObjectControl(MEMORY_OBJECT_CONTROL_FROM_GTT_ENTRY, CM_WRITE_THROUGH, 0));
        CMCHK_HR(static_cast< CmBuffer_RT* >(pCMBufferUP_UV)->SetMemoryObjectControl(MEMORY_OBJECT_CONTROL_FROM_GTT_ENTRY, CM_WRITE_THROUGH, 0));
    }

    CMCHK_HR(CreateGPUCopyKernel(copy_width_byte, copy_height_row, format, direction, pGPUCopyKrnParam));
    CMCHK_NULL(pGPUCopyKrnParam);
    pKernel = pGPUCopyKrnParam->pKernel;

    CMCHK_NULL(pKernel);

    CMCHK_NULL(pCMBufferUP_Y);
    CMCHK_NULL(pCMBufferUP_UV);
    CMCHK_HR(pCMBufferUP_Y->GetIndex(pBufferUPIndex_Y));
    CMCHK_HR(pCMBufferUP_UV->GetIndex(pBufferUPIndex_UV));
    CMCHK_HR(pSurface->GetIndex(pSurf2DIndexCM));

    threadWidth = (uint32_t)ceil((double)copy_width_byte / BLOCK_PIXEL_WIDTH / 4);
    threadHeight = (uint32_t)ceil((double)copy_height_row / BLOCK_HEIGHT / INNER_LOOP);
    threadNum = threadWidth * threadHeight;
    CMCHK_HR(pKernel->SetThreadCount(threadNum));
    CMCHK_HR(m_pDevice->CreateThreadSpace(threadWidth, threadHeight, pTS));

    width_dword = (uint32_t)ceil((double)width_byte / 4);
    stride_in_dwords = (uint32_t)ceil((double)stride_in_bytes / 4);

    if (direction == CM_FASTCOPY_CPU2GPU) //Write
    {
        //Input BufferUP_Y and BufferUP_UV
        if (pCmHalState->pCmHalInterface->IsSurfaceCompressionWARequired())
        {
            CMCHK_HR(pSurface->SetCompressionMode(MEMCOMP_DISABLED));
        }
        CMCHK_HR(pKernel->SetKernelArg(0, sizeof(SurfaceIndex), pBufferUPIndex_Y));
        CMCHK_HR(pKernel->SetKernelArg(1, sizeof(SurfaceIndex), pBufferUPIndex_UV));
        //Output Surface2D
        CMCHK_HR(pKernel->SetKernelArg(2, sizeof(SurfaceIndex), pSurf2DIndexCM));
        //Other parameters
        CMCHK_HR(pKernel->SetKernelArg(3, sizeof(uint32_t), &stride_in_dwords));
        CMCHK_HR(pKernel->SetKernelArg(4, sizeof(uint32_t), &height_stride_in_rows));
        CMCHK_HR(pKernel->SetKernelArg(5, sizeof(uint32_t), &AddedShiftLeftOffset_Y));
        CMCHK_HR(pKernel->SetKernelArg(6, sizeof(uint32_t), &AddedShiftLeftOffset_UV));
        CMCHK_HR(pKernel->SetKernelArg(7, sizeof(uint32_t), &threadHeight));
    }
    else  //Read
    {
        //Input Surface2D
        CMCHK_HR(pKernel->SetKernelArg(0, sizeof(SurfaceIndex), pSurf2DIndexCM));
        //Output BufferUP_Y and BufferUP_UV
        CMCHK_HR(pKernel->SetKernelArg(1, sizeof(SurfaceIndex), pBufferUPIndex_Y));
        CMCHK_HR(pKernel->SetKernelArg(2, sizeof(SurfaceIndex), pBufferUPIndex_UV));
        //Other parameters
        CMCHK_HR(pKernel->SetKernelArg(3, sizeof(uint32_t), &stride_in_dwords));
        CMCHK_HR(pKernel->SetKernelArg(4, sizeof(uint32_t), &height_stride_in_rows));
        CMCHK_HR(pKernel->SetKernelArg(5, sizeof(uint32_t), &AddedShiftLeftOffset_Y));
        CMCHK_HR(pKernel->SetKernelArg(6, sizeof(uint32_t), &AddedShiftLeftOffset_UV));
        CMCHK_HR(pKernel->SetKernelArg(7, sizeof(uint32_t), &threadHeight));
        CMCHK_HR(pKernel->SetKernelArg(8, sizeof(uint32_t), &width_dword));
        CMCHK_HR(pKernel->SetKernelArg(9, sizeof(uint32_t), &heightInRow));

        pSurface->SetReadSyncFlag(true); // GPU -> CPU, set surf2d as read sync flag
    }

    CMCHK_HR(m_pDevice->CreateQueue(pCmQueue));
    CMCHK_HR(m_pDevice->CreateTask(pGPUCopyTask));
    CMCHK_HR(pGPUCopyTask->AddKernel(pKernel));
    if (option & CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST)
    {
        // disable turbo
        CM_TASK_CONFIG taskConfig;
        CmSafeMemSet(&taskConfig, 0, sizeof(CM_TASK_CONFIG));
        taskConfig.turboBoostFlag = CM_TURBO_BOOST_DISABLE;
        pGPUCopyTask->SetProperty(taskConfig);
    }
    CMCHK_HR(pCmQueue->Enqueue(pGPUCopyTask, pInternalEvent, pTS));

    if (pGPUCopyKrnParam)
    {
        GPUCOPY_KERNEL_UNLOCK(pGPUCopyKrnParam);
    }

    if ((option & CM_FASTCOPY_OPTION_BLOCKING) && (pInternalEvent))
    {
        CMCHK_HR(pInternalEvent->WaitForTaskFinished());
    }

    if (pEvent == CM_NO_EVENT)  //User doesn't need CmEvent for this copy
    {
        pEvent = nullptr;
        CMCHK_HR(pCmQueue->DestroyEvent(pInternalEvent));
    }
    else //User needs this CmEvent
    {
        pEvent = pInternalEvent;
    }

    CMCHK_HR(m_pDevice->DestroyTask(pGPUCopyTask));
    CMCHK_HR(m_pDevice->DestroyThreadSpace(pTS));
    CMCHK_HR(m_pDevice->DestroyBufferUP(pCMBufferUP_Y));
    CMCHK_HR(m_pDevice->DestroyBufferUP(pCMBufferUP_UV));

finish:

    if (hr != CM_SUCCESS)
    {
        if ((pCMBufferUP_Y == nullptr) || (pCMBufferUP_UV == nullptr))
        {
            // user need to know whether the failure is caused by out of BufferUP.
            hr = CM_GPUCOPY_OUT_OF_RESOURCE;
        }

        if (pKernel && pGPUCopyKrnParam)        GPUCOPY_KERNEL_UNLOCK(pGPUCopyKrnParam);
        if (pTS)                                m_pDevice->DestroyThreadSpace(pTS);
        if (pGPUCopyTask)                       m_pDevice->DestroyTask(pGPUCopyTask);
        if (pCMBufferUP_Y)                      m_pDevice->DestroyBufferUP(pCMBufferUP_Y);
        if (pCMBufferUP_UV)                     m_pDevice->DestroyBufferUP(pCMBufferUP_UV);
        if (pInternalEvent)                     pCmQueue->DestroyEvent(pInternalEvent);

        // CM_FAILURE for all the other errors
        // return CM_EXCEED_MAX_TIMEOUT to notify app that gpu reset happens
        if( hr != CM_GPUCOPY_OUT_OF_RESOURCE && hr != CM_EXCEED_MAX_TIMEOUT)
        {
            hr = CM_FAILURE;
        }
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//! Enqueue an task, which contains one pre-defined kernel to copy from video memory to video memory
//! This is a non-blocking call. i.e. it returns immediately without waiting for
//! GPU to finish the execution of the task.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can
//! be used to check if the task finishes.
//! INPUT:
//!     1) Pointer to the CmSurface2D as copy destination
//!     2) Pointer to the CmSurface2D  as copy source
//!     3) Option passed from user, blocking copy, non-blocking copy or disable turbo boost
//!     4) Reference to the pointer to CMEvent
//! OUTPUT:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated;
//!     CM_OUT_OF_HOST_MEMORY if out of host memery;
//!     CM_GPUCOPY_INVALID_SURFACES if input/output surfaces' width/format are different or
//!                                 input surface's height is larger than output surface's
//! Restrictions:
//!     1) Surface's width should be 64-byte aligned.
//!     2) The input surface's width/height/format should be the same as output surface's.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueCopyGPUToGPU( CmSurface2D* pOutputSurface, CmSurface2D* pInputSurface, uint32_t option, CmEvent* & pEvent )
{
    INSERT_API_CALL_LOG();

    uint32_t SrcSurfaceWidth = 0;
    uint32_t SrcSurfaceHeight = 0;
    uint32_t DstSurfaceWidth = 0;
    uint32_t DstSurfaceHeight = 0;

    CM_SURFACE_FORMAT SrcSurfaceFormat = CM_SURFACE_FORMAT_INVALID;
    CM_SURFACE_FORMAT DstSurfaceFormat = CM_SURFACE_FORMAT_INVALID;

    int32_t             hr = CM_SUCCESS;
    uint32_t            SrcSizePerPixel = 0;
    uint32_t            DstSizePerPixel = 0;
    uint32_t            threadWidth = 0;
    uint32_t            threadHeight = 0;

    CmKernel            *pKernel = nullptr;
    SurfaceIndex        *pSurfaceInputIndex = nullptr;
    SurfaceIndex        *pSurfaceOutputIndex = nullptr;
    CmThreadSpace       *pTS = nullptr;
    CmTask              *pTask = nullptr;
    CmQueue             *pCmQueue = nullptr;
    uint32_t            SrcSurfAlignedWidthInBytes = 0;
    CM_GPUCOPY_KERNEL *pGPUCopyKrnParam = nullptr;

    if ((pOutputSurface == nullptr) || (pInputSurface == nullptr))
    {
        CM_ASSERTMESSAGE("Error: Pointer to input surface or output surface is null.");
        return CM_FAILURE;
    }

    PCM_HAL_STATE   pCmHalState = ((PCM_CONTEXT_DATA)m_pDevice->GetAccelData())->cmHalState;
    CmSurface2DRT *pOutputSurfaceRT = static_cast<CmSurface2DRT *>(pOutputSurface);
    CmSurface2DRT *pInputSurfaceRT = static_cast<CmSurface2DRT *>(pInputSurface);
    if (pCmHalState->pCmHalInterface->IsSurfaceCompressionWARequired())
    {
        CMCHK_HR(pOutputSurfaceRT->SetCompressionMode(MEMCOMP_DISABLED));
    }

    CMCHK_HR(pOutputSurfaceRT->GetSurfaceDesc(DstSurfaceWidth, DstSurfaceHeight, DstSurfaceFormat, DstSizePerPixel));
    CMCHK_HR(pInputSurfaceRT->GetSurfaceDesc(SrcSurfaceWidth, SrcSurfaceHeight, SrcSurfaceFormat, SrcSizePerPixel));

    if ((DstSurfaceWidth != SrcSurfaceWidth) ||
        (DstSurfaceHeight < SrcSurfaceHeight) ||  //relax the restriction
        (DstSizePerPixel != SrcSizePerPixel))
    {
        CM_ASSERTMESSAGE("Error: Size of dest surface does not match src surface.");
        return CM_GPUCOPY_INVALID_SURFACES;
    }

    //To support copy b/w Format_A8R8G8B8 and Format_A8B8G8R8
    if (DstSurfaceFormat != SrcSurfaceFormat)
    {
        if (!((DstSurfaceFormat == CM_SURFACE_FORMAT_A8R8G8B8) && (SrcSurfaceFormat == CM_SURFACE_FORMAT_A8B8G8R8)) &&
            !((DstSurfaceFormat == CM_SURFACE_FORMAT_A8R8G8B8) && (SrcSurfaceFormat == CM_SURFACE_FORMAT_A8B8G8R8)))
        {
            CM_ASSERTMESSAGE("Error: Only support copy b/w Format_A8R8G8B8 and Format_A8B8G8R8 if src format is not matched with dst format.");
            return CM_GPUCOPY_INVALID_SURFACES;
        }
    }

    // 128Bytes aligned
    SrcSurfAlignedWidthInBytes = (uint32_t)(ceil((double)SrcSurfaceWidth*SrcSizePerPixel / BLOCK_PIXEL_WIDTH / 4) * (BLOCK_PIXEL_WIDTH * 4));

    if (SrcSurfaceHeight > CM_MAX_THREADSPACE_WIDTH_FOR_MW *BLOCK_HEIGHT *INNER_LOOP)
    {
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_GPUCOPY_INVALID_SIZE;
    }

    CMCHK_HR(CreateGPUCopyKernel(SrcSurfaceWidth*SrcSizePerPixel, SrcSurfaceHeight, SrcSurfaceFormat, CM_FASTCOPY_GPU2GPU, pGPUCopyKrnParam));
    CMCHK_NULL(pGPUCopyKrnParam);

    CMCHK_NULL(pGPUCopyKrnParam->pKernel);
    pKernel = pGPUCopyKrnParam->pKernel;

    CMCHK_HR(pInputSurface->GetIndex(pSurfaceInputIndex));
    CMCHK_HR(pOutputSurface->GetIndex(pSurfaceOutputIndex));

    threadWidth = SrcSurfAlignedWidthInBytes / (BLOCK_PIXEL_WIDTH * 4);
    threadHeight = (uint32_t)ceil((double)SrcSurfaceHeight / BLOCK_HEIGHT / INNER_LOOP);

    CMCHK_HR(pKernel->SetThreadCount(threadWidth * threadHeight));

    CMCHK_HR(pKernel->SetKernelArg(0, sizeof(SurfaceIndex), pSurfaceInputIndex));
    CMCHK_HR(pKernel->SetKernelArg(1, sizeof(SurfaceIndex), pSurfaceOutputIndex));
    CMCHK_HR(pKernel->SetKernelArg(2, sizeof(uint32_t), &threadHeight));

    CMCHK_HR(m_pDevice->CreateThreadSpace(threadWidth, threadHeight, pTS));

    CMCHK_HR(m_pDevice->CreateTask(pTask));
    CMCHK_NULL(pTask);
    CMCHK_HR(pTask->AddKernel(pKernel));

    if (option & CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST)
    {
        // disable turbo
        CM_TASK_CONFIG taskConfig;
        CmSafeMemSet(&taskConfig, 0, sizeof(CM_TASK_CONFIG));
        taskConfig.turboBoostFlag = CM_TURBO_BOOST_DISABLE;
        pTask->SetProperty(taskConfig);
    }

    CMCHK_HR(m_pDevice->CreateQueue(pCmQueue));
    CMCHK_HR(pCmQueue->Enqueue(pTask, pEvent, pTS));
    if ((option & CM_FASTCOPY_OPTION_BLOCKING) && (pEvent))
    {
        CMCHK_HR(pEvent->WaitForTaskFinished());
    }

finish:

    if (pKernel && pGPUCopyKrnParam)        GPUCOPY_KERNEL_UNLOCK(pGPUCopyKrnParam);
    if (pTS)                                m_pDevice->DestroyThreadSpace(pTS);
    if (pTask)                              m_pDevice->DestroyTask(pTask);

    return hr;
}

//*-----------------------------------------------------------------------------
//! Enqueue an task, which contains one pre-defined kernel to copy from system memory to system memory
//! This is a non-blocking call. i.e. it returns immediately without waiting for
//! GPU to finish the execution of the task.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can be used to check if the task finishs.
//! If the size is less than 1KB,  CPU is used to do the copy and pEvent will be set as nullptr .
//!
//! INPUT:
//!     1) Pointer to the system memory as copy destination
//!     2) Pointer to the system memory as copy source
//!     3) The size in bytes of memory be copied.
//!     4) Option passed from user, blocking copy, non-blocking copy or disable turbo boost
//!     5) Reference to the pointer to CMEvent
//! OUTPUT:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated;
//!     CM_OUT_OF_HOST_MEMORY if out of host memery;
//!     CM_GPUCOPY_INVALID_SYSMEM if the pSysMem is not 16-byte aligned or is NULL.
//!     CM_GPUCOPY_OUT_OF_RESOURCE if runtime run out of BufferUP.
//!     CM_GPUCOPY_INVALID_SIZE  if its size plus shift-left offset large than CM_MAX_1D_SURF_WIDTH.
//! Restrictions:
//!     1) pDstSysMem and pSrcSysMem should be 16-byte aligned.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueCopyCPUToCPU( unsigned char* pDstSysMem, unsigned char* pSrcSysMem, uint32_t size, uint32_t option, CmEvent* & pEvent )
{
    INSERT_API_CALL_LOG();

    int hr = CM_SUCCESS;
    size_t InputLinearAddress  = (size_t )pSrcSysMem;
    size_t OutputLinearAddress = (size_t )pDstSysMem;

    size_t pInputLinearAddressAligned = 0;
    size_t pOutputLinearAddressAligned = 0;

    CmBufferUP      *pSurfaceInput          = nullptr;
    CmBufferUP      *pSurfaceOutput         = nullptr;
    CmKernel        *pKernel                = nullptr;
    SurfaceIndex    *pSurfaceInputIndex     = nullptr;
    SurfaceIndex    *pSurfaceOutputIndex    = nullptr;
    CmThreadSpace   *pTS                    = nullptr;
    CmTask          *pTask                  = nullptr;
    CmQueue         *pCmQueue               = nullptr;

    int32_t         SrcLeftShiftOffset      = 0;
    int32_t         DstLeftShiftOffset      = 0;
    uint32_t        threadWidth             = 0;
    uint32_t        threadHeight            = 0;
    uint32_t        thread_num              = 0;
    uint32_t        gpu_memcopy_size        = 0;
    uint32_t        cpu_memcopy_size        = 0;
    CM_GPUCOPY_KERNEL *pGPUCopyKrnParam     = nullptr;

    if((InputLinearAddress & 0xf) || (OutputLinearAddress & 0xf) ||
        (InputLinearAddress == 0) || (OutputLinearAddress == 0))
    {
        CM_ASSERTMESSAGE("Error: Start address of system memory is not 16-byte aligned.");
        return CM_GPUCOPY_INVALID_SYSMEM;
    }

    // Get page aligned address
    if (sizeof (void *) == 8 ) //64-bit
    {
        pInputLinearAddressAligned  = InputLinearAddress  & ADDRESS_PAGE_ALIGNMENT_MASK_X64;  // make sure the address page aligned.
        pOutputLinearAddressAligned = OutputLinearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X64;  // make sure the address page aligned.
    }
    else
    {
        pInputLinearAddressAligned  = InputLinearAddress  & ADDRESS_PAGE_ALIGNMENT_MASK_X86;  // make sure the address page aligned.
        pOutputLinearAddressAligned = OutputLinearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X86;  // make sure the address page aligned.
    }


    SrcLeftShiftOffset = (int32_t)(InputLinearAddress  - pInputLinearAddressAligned) ;
    DstLeftShiftOffset = (int32_t)(OutputLinearAddress - pOutputLinearAddressAligned) ;

    if(((size + SrcLeftShiftOffset) > CM_MAX_1D_SURF_WIDTH)||
       ((size + DstLeftShiftOffset) > CM_MAX_1D_SURF_WIDTH))
    {
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_GPUCOPY_INVALID_SIZE;
    }

    threadWidth  = 0;
    threadHeight = 0;
    thread_num = size / BYTE_COPY_ONE_THREAD; // each thread copys 32 x 4 x32 bytes = 1K

    if( thread_num == 0)
    {
        //if the size of data is less than data copied per thread ( 4K), use CPU to copy it instead of GPU.
        CmFastMemCopy((void *)(OutputLinearAddress),
                      (void *)(InputLinearAddress),
                      size); //SSE copy used in CMRT.

        pEvent = nullptr;
        return CM_SUCCESS;
    }

    //Calculate proper thread space's width and height
    threadWidth  = 1;
    threadHeight = thread_num/threadWidth;
    while((threadHeight > CM_MAX_THREADSPACE_HEIGHT_FOR_MW))
    {
        if(threadWidth > CM_MAX_THREADSPACE_WIDTH_FOR_MW)
        {
            hr = CM_GPUCOPY_INVALID_SIZE; // thread number exceed 511*511
            goto finish;
        }
        else if (threadWidth == 1)
        {
            threadWidth  =  THREAD_SPACE_WIDTH_INCREMENT; // first time,
            threadHeight = thread_num/threadWidth;
        }
        else
        {
            threadWidth +=  THREAD_SPACE_WIDTH_INCREMENT; // increase 8 per iteration
            threadHeight = thread_num/threadWidth;
        }
    }

    CMCHK_HR(m_pDevice->CreateBufferUP(size + SrcLeftShiftOffset, (void *)pInputLinearAddressAligned,pSurfaceInput));

    CMCHK_HR(m_pDevice->CreateBufferUP(size + DstLeftShiftOffset, (void *)pOutputLinearAddressAligned,pSurfaceOutput));

    CMCHK_HR(CreateGPUCopyKernel(size, 0, CM_SURFACE_FORMAT_INVALID, CM_FASTCOPY_CPU2CPU, pGPUCopyKrnParam));
    CMCHK_NULL(pGPUCopyKrnParam);
    CMCHK_NULL(pGPUCopyKrnParam->pKernel);
    pKernel = pGPUCopyKrnParam->pKernel;


    CMCHK_NULL(pSurfaceInput);
    CMCHK_HR(pSurfaceInput->GetIndex(pSurfaceInputIndex));
    CMCHK_NULL(pSurfaceOutput);
    CMCHK_HR(pSurfaceOutput->GetIndex(pSurfaceOutputIndex));

    CMCHK_HR(pKernel->SetThreadCount(threadWidth * threadHeight));
    CMCHK_HR(pKernel->SetKernelArg( 0, sizeof( SurfaceIndex ), pSurfaceInputIndex ));
    CMCHK_HR(pKernel->SetKernelArg( 1, sizeof( SurfaceIndex ), pSurfaceOutputIndex ));
    CMCHK_HR(pKernel->SetKernelArg( 2, sizeof( int ), &threadWidth ));
    CMCHK_HR(pKernel->SetKernelArg( 3, sizeof( int ), &threadHeight ));
    CMCHK_HR(pKernel->SetKernelArg( 4, sizeof( int ), &SrcLeftShiftOffset ));
    CMCHK_HR(pKernel->SetKernelArg( 5, sizeof( int ), &DstLeftShiftOffset ));
    CMCHK_HR(pKernel->SetKernelArg( 6, sizeof( int ), &size ));

    CMCHK_HR(m_pDevice->CreateThreadSpace(threadWidth, threadHeight, pTS));

    CMCHK_HR(m_pDevice->CreateTask(pTask));
    CMCHK_NULL(pTask);
    CMCHK_HR(pTask->AddKernel (pKernel));

    if (option & CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST)
    {
        // disable turbo
        CM_TASK_CONFIG taskConfig;
        CmSafeMemSet(&taskConfig, 0, sizeof(CM_TASK_CONFIG));
        taskConfig.turboBoostFlag = CM_TURBO_BOOST_DISABLE;
        pTask->SetProperty(taskConfig);
    }

    CMCHK_HR(m_pDevice->CreateQueue( pCmQueue));
    CMCHK_HR(pCmQueue->Enqueue(pTask, pEvent, pTS));

    if ((option & CM_FASTCOPY_OPTION_BLOCKING) && (pEvent))
    {
        CMCHK_HR(pEvent->WaitForTaskFinished());
    }

    //Copy the unaligned part by using CPU
    gpu_memcopy_size = threadHeight * threadWidth *BYTE_COPY_ONE_THREAD;
    cpu_memcopy_size = size - threadHeight * threadWidth *BYTE_COPY_ONE_THREAD;

    CmFastMemCopy((void *)(OutputLinearAddress+gpu_memcopy_size),
                  (void *)(InputLinearAddress+gpu_memcopy_size),
                          cpu_memcopy_size); //SSE copy used in CMRT.

    CMCHK_HR(m_pDevice->DestroyThreadSpace(pTS));
    CMCHK_HR(m_pDevice->DestroyTask(pTask));
    CMCHK_HR(m_pDevice->DestroyBufferUP(pSurfaceOutput));   // ref_cnf to guarantee task finish before BufferUP being really destroy.
    CMCHK_HR(m_pDevice->DestroyBufferUP(pSurfaceInput));

    if( pGPUCopyKrnParam )
    {
        GPUCOPY_KERNEL_UNLOCK(pGPUCopyKrnParam);
    }


finish:
    if(hr != CM_SUCCESS)
    {   //Failed
        if( pSurfaceInput == nullptr || pSurfaceOutput == nullptr)
        {
            hr = CM_GPUCOPY_OUT_OF_RESOURCE; // user need to know whether the failure is caused by out of BufferUP.
        }
        else
        {
            hr = CM_FAILURE;
        }
        if(pSurfaceInput)                      m_pDevice->DestroyBufferUP(pSurfaceInput);
        if(pSurfaceOutput)                     m_pDevice->DestroyBufferUP(pSurfaceOutput);
        if(pKernel && pGPUCopyKrnParam)        GPUCOPY_KERNEL_UNLOCK(pGPUCopyKrnParam);
        if(pTS)                                m_pDevice->DestroyThreadSpace(pTS);
        if(pTask)                              m_pDevice->DestroyTask(pTask);
    }

    return hr;
}

//*----------------------------------------------------------------------------------------
//| Purpose:    Pop task from flushed Queue, Update surface state and Destroy the task
//| Notes:
//*----------------------------------------------------------------------------------------
void CmQueueRT::PopTaskFromFlushedQueue()
{
    CmTaskInternal* pTopTask = (CmTaskInternal*)m_FlushedTasks.Pop();

    if ( pTopTask != nullptr )
    {
        CmEventRT *pEvent = nullptr;
        pTopTask->GetTaskEvent( pEvent );
        if ( pEvent != nullptr )
        {
            LARGE_INTEGER nTime;
            if ( !(MOS_QueryPerformanceCounter( (uint64_t*)&nTime.QuadPart )) )
            {
                CM_ASSERTMESSAGE("Error: Query performace counter failure.");
            }
            else
            {
                pEvent->SetCompleteTime( nTime );
            }
        }

#if MDF_SURFACE_CONTENT_DUMP
        PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)m_pDevice->GetAccelData();
        if (pCmData->cmHalState->bDumpSurfaceContent)
        {
            int32_t iTaskId = 0;
            if (pEvent != nullptr)
            {
                pEvent->GetTaskDriverId(iTaskId);
            }
            pTopTask->SurfaceDump(iTaskId);
        }
#endif

        CmTaskInternal::Destroy( pTopTask );
    }
    return;
}

int32_t CmQueueRT::TouchFlushedTasks( )
{
    int32_t hr = CM_SUCCESS;

    if (m_FlushedTasks.IsEmpty())
    {
        if (!m_EnqueuedTasks.IsEmpty())
        {
            // if FlushedQueue is empty and EnqueuedQueue is not empty
            // try flush task to FlushedQueue
            hr = FlushTaskWithoutSync();
            if (FAILED(hr))
            {
                return hr;
            }
        }
        else
        {   // no task in flushedQueue and EnqueuedQueue
            return CM_FAILURE;
        }
    }

    // Flush FlushedQueue
    hr = QueryFlushedTasks();

    return hr;
}

//*-----------------------------------------------------------------------------
//! Flush the queue, i.e. submit all tasks in the queue to execute according
//! to their order in the the queue. The queue will be empty after flush,
//! This is a non-blocking call. i.e. it returns immediately without waiting for
//! GPU to finish the execution of tasks.
//! INPUT:
//! OUTPUT:
//!     CM_SUCCESS if all tasks in the queue are submitted
//!     CM_FAILURE otherwise.
//!     More error code is coming.
//!
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::QueryFlushedTasks()
{
    int32_t hr   = CM_SUCCESS;

    m_CriticalSection_FlushedTask.Acquire();
    while( !m_FlushedTasks.IsEmpty() )
    {
        CmTaskInternal* pTask = (CmTaskInternal*)m_FlushedTasks.Top();
        CMCHK_NULL(pTask);

        CM_STATUS status = CM_STATUS_FLUSHED ;
        pTask->GetTaskStatus(status);
        if( status == CM_STATUS_FINISHED )
        {
            PopTaskFromFlushedQueue();
        }
        else
        {
            // media reset
            if (status == CM_STATUS_RESET)
            {
                PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)m_pDevice->GetAccelData();

                // Clear task status table in Cm Hal State
                int32_t iTaskId;
                CmEventRT*pTopTaskEvent;
                pTask->GetTaskEvent(pTopTaskEvent);
                CMCHK_NULL(pTopTaskEvent);

                pTopTaskEvent->GetTaskDriverId(iTaskId);
                pCmData->cmHalState->pTaskStatusTable[iTaskId] = CM_INVALID_INDEX;

                //Pop task and Destroy it
                PopTaskFromFlushedQueue();
            }

            // It is an in-order queue, if this one hasn't finshed,
            // the following ones haven't finished either.
            break;
        }
    }

finish:
    m_CriticalSection_FlushedTask.Release();

    return hr;
}

//*-----------------------------------------------------------------------------
//! This is a blocking call. It will NOT return untill
//! all tasks in GPU and all tasks in queue finishes execution.
//! It will first flush the queue if the queue is not empty.
//! INPUT:
//! OUTPUT:
//!     CM_SUCCESS if all tasks finish execution.
//!     CM_FAILURE otherwise.
//!     More error code is coming.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::DestroyEvent( CmEvent* & pEvent )
{

    CLock Lock(m_CriticalSection_Event);

    if (pEvent == nullptr)
    {
        return CM_FAILURE;
    }

    uint32_t index = 0;

    CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
    pEventRT->GetIndex(index);
    CM_ASSERT( m_EventArray.GetElement( index ) == pEventRT );

    int32_t status = CmEventRT::Destroy( pEventRT );
    if( status == CM_SUCCESS && pEventRT == nullptr)
    {
        m_EventArray.SetElement(index, nullptr);
    }

    // Should return nullptr to application even the event is not destroyed
    // since its reference count is not zero
    pEvent = nullptr;

    return status;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Clean the Queue if its tasks time out
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::CleanQueue( )
{

    int32_t status = CM_SUCCESS;

    // Maybe not necessary since
    // it is called by ~CmDevice only
    // Update: necessary because it calls FlushBlockWithoutSync
    if( !m_EnqueuedTasks.IsEmpty() )
    {
        // If there are tasks not flushed (i.e. not send to driver )
        // wait untill all such tasks are flushed
        FlushTaskWithoutSync( true );
    }
    CM_ASSERT( m_EnqueuedTasks.IsEmpty() );

    //Used for timeout detection
    LARGE_INTEGER freq;
    MOS_QueryPerformanceFrequency((uint64_t*)&freq.QuadPart);
    LARGE_INTEGER start;
    MOS_QueryPerformanceCounter((uint64_t*)&start.QuadPart);
    int64_t timeout = start.QuadPart + (CM_MAX_TIMEOUT * freq.QuadPart * m_FlushedTasks.GetCount()); //Count to timeout at

    while( !m_FlushedTasks.IsEmpty() && status != CM_EXCEED_MAX_TIMEOUT )
    {
        QueryFlushedTasks();

        LARGE_INTEGER current;
        MOS_QueryPerformanceCounter((uint64_t*)&current.QuadPart);
        if( current.QuadPart > timeout )
            status = CM_EXCEED_MAX_TIMEOUT;
    }

    return status;
}

CM_QUEUE_CREATE_OPTION &CmQueueRT::GetQueueOption()
{
    return m_queueOption;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the count of task in queue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::GetTaskCount( uint32_t& numTasks )
{
    numTasks = m_EnqueuedTasks.GetCount() + m_FlushedTasks.GetCount();
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Use GPU to init Surface2D
//| Returns:   result of operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueInitSurface2D( CmSurface2D* pSurf2D, const uint32_t initValue, CmEvent* &pEvent)
{
    INSERT_API_CALL_LOG();

    int32_t         hr                      = CM_SUCCESS;
    uint32_t        width                   = 0;
    uint32_t        height                  = 0;
    uint32_t        sizePerPixel            = 0;
    CmProgram       *pGPUInitKernelProgram  = nullptr;
    CmKernel        *pKernel                = nullptr;
    SurfaceIndex    *pOutputIndexCM         = nullptr;
    CmThreadSpace   *pTS                    = nullptr;
    CmQueue         *pCmQueue               = nullptr;
    CmTask          *pGPUCopyTask           = nullptr;
    uint32_t        threadWidth             = 0;
    uint32_t        threadHeight            = 0;
    uint32_t        threadNum               = 0;
    CmSurfaceManager* pSurfaceMgr           = nullptr;
    CM_SURFACE_FORMAT      format           = CM_SURFACE_FORMAT_INVALID;

    if(!pSurf2D)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface 2d is null.");
        return CM_FAILURE;
    }
    CmSurface2DRT *pSurf2DRT = static_cast<CmSurface2DRT *>(pSurf2D);

    CMCHK_HR(m_pDevice->LoadPredefinedInitKernel(pGPUInitKernelProgram));

    CMCHK_HR(pSurf2DRT->GetSurfaceDesc(width, height, format,sizePerPixel));

    m_pDevice->GetSurfaceManager(pSurfaceMgr);
    CMCHK_NULL(pSurfaceMgr);

    if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
    {
        CMCHK_HR(m_pDevice->CreateKernel( pGPUInitKernelProgram, _NAME( surfaceCopy_set_NV12 ), pKernel, "PredefinedGPUCopyKernel"));
    }
    else
    {
        CMCHK_HR(m_pDevice->CreateKernel( pGPUInitKernelProgram, _NAME( surfaceCopy_set ), pKernel, "PredefinedGPUCopyKernel" ));
    }
    CMCHK_NULL(pKernel);
    CMCHK_HR(pSurf2D->GetIndex( pOutputIndexCM ));

    threadWidth = ( uint32_t )ceil( ( double )width*sizePerPixel/BLOCK_PIXEL_WIDTH/4 );
    threadHeight = ( uint32_t )ceil( ( double )height/BLOCK_HEIGHT );
    threadNum = threadWidth * threadHeight;
    CMCHK_HR(pKernel->SetThreadCount( threadNum ));


    CMCHK_HR(m_pDevice->CreateThreadSpace( threadWidth, threadHeight, pTS ));
    CMCHK_NULL(pTS);

    CMCHK_HR(pKernel->SetKernelArg( 0, sizeof( uint32_t ), &initValue ));
    CMCHK_HR(pKernel->SetKernelArg( 1, sizeof( SurfaceIndex ), pOutputIndexCM ));


    CMCHK_HR(m_pDevice->CreateQueue( pCmQueue ));

    CMCHK_HR(m_pDevice->CreateTask(pGPUCopyTask));
    CMCHK_NULL(pGPUCopyTask);

    CMCHK_HR(pGPUCopyTask->AddKernel( pKernel ));

    CMCHK_HR(pCmQueue->Enqueue( pGPUCopyTask, pEvent, pTS ));

finish:

    if (pKernel)        m_pDevice->DestroyKernel( pKernel );
    if (pGPUCopyTask)   m_pDevice->DestroyTask(pGPUCopyTask);
    if (pTS)            m_pDevice->DestroyThreadSpace(pTS);

    return hr;
}

//*-----------------------------------------------------------------------------
//! Flush a geneal task to HAL CM layer for execution.
//! This is a non-blocking call. i.e. it returs immediately without waiting for
//! GPU to finish the execution of tasks.
//! INPUT: pTask -- Pointer to CmTaskInternal object
//! OUTPUT:
//!     CM_SUCCESS if all tasks in the queue are submitted
//!     CM_FAILURE otherwise.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::FlushGeneralTask(CmTaskInternal* pTask)
{
    CM_RETURN_CODE          hr              = CM_SUCCESS;
    CM_HAL_EXEC_TASK_PARAM  param;
    PCM_HAL_KERNEL_PARAM    pKernelParam    = nullptr;
    CmKernelData*           pKernelData     = nullptr;
    uint32_t                kernelDataSize  = 0;
    PCM_CONTEXT_DATA        pCmData         = nullptr;
    CmEventRT*              pEvent          = nullptr;
    uint32_t                totalThreadCount= 0;
    uint32_t                count           = 0;
    PCM_HAL_KERNEL_PARAM    pTempData       = nullptr;
    uint32_t                maxTSWidth      = 0;
    bool                    hasThreadArg    = false;

    CmSafeMemSet( &param, 0, sizeof( CM_HAL_EXEC_TASK_PARAM ) );

    //GT-PIN
    if(m_pDevice->CheckGTPinEnabled())
    {
        CMCHK_HR(pTask->GetKernelSurfInfo(param.surfEntryInfoArrays));
    }

    pTask->GetKernelCount( count );
    param.numKernels = count;

    param.kernels = MOS_NewArray(PCM_HAL_KERNEL_PARAM,count);
    param.kernelSizes = MOS_NewArray(uint32_t,count);
    param.kernelCurbeOffset = MOS_NewArray(uint32_t,count);
    param.queueOption = m_queueOption;

    CMCHK_NULL_RETURN(param.kernels, CM_OUT_OF_HOST_MEMORY);
    CMCHK_NULL_RETURN(param.kernelSizes, CM_OUT_OF_HOST_MEMORY);
    CMCHK_NULL_RETURN(param.kernelCurbeOffset, CM_OUT_OF_HOST_MEMORY);

    for( uint32_t i = 0; i < count; i ++ )
    {
        pTask->GetKernelData( i, pKernelData );
        CMCHK_NULL(pKernelData);

        pKernelParam = pKernelData->GetHalCmKernelData();
        CMCHK_NULL(pKernelParam);

        hasThreadArg |= pKernelParam->perThreadArgExisted;

        pTask->GetKernelDataSize( i, kernelDataSize );
        if(kernelDataSize == 0)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel data size.");
            hr = CM_FAILURE;
            goto finish;
        }

        pTempData = pKernelData->GetHalCmKernelData();

        param.kernels[ i ]             = pTempData;
        param.kernelSizes[ i ]        = kernelDataSize;
        param.kernelCurbeOffset[ i ]  = pTask->GetKernelCurbeOffset(i);
        param.globalSurfaceUsed       |= pTempData->globalSurfaceUsed;
        param.kernelDebugEnabled      |= pTempData->kernelDebugEnabled;
    }

    /*
    * Preset the default TS width/height/dependency:
    *     TS width   = MOS_MIN(CM_MAX_THREADSPACE_WIDTH, threadcount)
    *     TS height  = totalThreadCount/CM_MAX_THREADSPACE_WIDTH + 1
    *     dependency = CM_NONE_DEPENDENCY
    * For pTS is nullptr case, we will pass the default TS width/height/dependency to driver
    * For pTS is valid case, the TS width/height/dependency will be update according to thread space set by user.
    */
    pTask->GetTotalThreadCount(totalThreadCount);

    if (hasThreadArg)
    {
        maxTSWidth = CM_MAX_THREADSPACE_WIDTH_FOR_MW + 1; // 512 allowed for media object
    }
    else
    {
        maxTSWidth = CM_MAX_THREADSPACE_WIDTH_FOR_MW; // 511 for media walker
    }

    param.threadSpaceWidth = (totalThreadCount > maxTSWidth) ? maxTSWidth : totalThreadCount;
    if(totalThreadCount%maxTSWidth)
    {
        param.threadSpaceHeight = totalThreadCount/maxTSWidth + 1;
    }
    else
    {
        param.threadSpaceHeight = totalThreadCount/maxTSWidth;
    }

    param.dependencyPattern = CM_NONE_DEPENDENCY;

    if (pTask->IsThreadSpaceCreated()) //scoreboard data preparation
    {
        if(pTask->IsThreadCoordinatesExisted())
        {
            param.threadCoordinates = MOS_NewArray(PCM_HAL_SCOREBOARD, count);
            param.dependencyMasks = MOS_NewArray(PCM_HAL_MASK_AND_RESET, count);

            CMCHK_NULL_RETURN(param.threadCoordinates, CM_OUT_OF_HOST_MEMORY);
            CMCHK_NULL_RETURN(param.dependencyMasks, CM_OUT_OF_HOST_MEMORY);
            for(uint32_t i=0; i<count; i++)
            {
                void *pKernelCoordinates = nullptr;
                void *pDependencyMasks = nullptr;
                pTask->GetKernelCoordinates(i, pKernelCoordinates);
                pTask->GetKernelDependencyMasks(i, pDependencyMasks);
                param.threadCoordinates[i] = (PCM_HAL_SCOREBOARD)pKernelCoordinates;
                param.dependencyMasks[i] = (PCM_HAL_MASK_AND_RESET)pDependencyMasks;
            }
        }
        else
        {
            param.threadCoordinates = nullptr;
        }

        pTask->GetDependencyPattern(param.dependencyPattern);

        pTask->GetThreadSpaceSize(param.threadSpaceWidth, param.threadSpaceHeight);

        pTask->GetWalkingPattern(param.walkingPattern);

        if( pTask->CheckWalkingParametersSet( ) )
        {
            param.walkingParamsValid = 1;
            CMCHK_HR(pTask->GetWalkingParameters(param.walkingParams));
        }
        else
        {
            param.walkingParamsValid = 0;
        }

        if( pTask->CheckDependencyVectorsSet( ) )
        {
            param.dependencyVectorsValid = 1;
            CMCHK_HR(pTask->GetDependencyVectors(param.dependencyVectors));
        }
        else
        {
            param.dependencyVectorsValid = 0;
        }
    }
    if (param.threadSpaceWidth == 0)
    {
        CM_ASSERTMESSAGE("Error: Invalid thread space.");
        hr = CM_INVALID_THREAD_SPACE;
        goto finish;
    }
    pTask->GetColorCountMinusOne(param.colorCountMinusOne);
    pTask->GetMediaWalkerGroupSelect(param.mediaWalkerGroupSelect);

    param.syncBitmap = pTask->GetSyncBitmap();
    param.conditionalEndBitmap = pTask->GetConditionalEndBitmap();
    param.userDefinedMediaState = pTask->GetMediaStatePtr();
    CmSafeMemCopy(param.conditionalEndInfo, pTask->GetConditionalEndInfo(), sizeof(param.conditionalEndInfo));
    CmSafeMemCopy(&param.taskConfig, pTask->GetTaskConfig(), sizeof(param.taskConfig));
    pCmData = (PCM_CONTEXT_DATA)m_pDevice->GetAccelData();

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnSetPowerOption(pCmData->cmHalState, pTask->GetPowerOption()));

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnExecuteTask(pCmData->cmHalState, &param));

    if( param.taskIdOut < 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid task ID.");
        hr = CM_FAILURE;
        goto finish;
    }

    TASK_LOG(pTask);

    pTask->GetTaskEvent( pEvent );
    CMCHK_NULL(pEvent);
    CMCHK_HR(pEvent->SetTaskDriverId( param.taskIdOut ));
    CMCHK_HR(pEvent->SetTaskOsData( param.osData ));
    CMCHK_HR(pTask->ResetKernelDataStatus());

    //GT-PIN
    if(m_pDevice->CheckGTPinEnabled())
    {
        //No need to clear the SurEntryInfoArrays here. It will be destored by CmInternalTask
        CMCHK_HR(pEvent->SetSurfaceDetails(param.surfEntryInfoArrays));
    }

finish:
    MosSafeDeleteArray( param.kernels );
    MosSafeDeleteArray( param.kernelSizes );
    MosSafeDeleteArray( param.threadCoordinates);
    MosSafeDeleteArray( param.dependencyMasks);
    MosSafeDeleteArray( param.kernelCurbeOffset);

    return hr;
}

//*-----------------------------------------------------------------------------
//! Flush a thread group based task to HAL CM layer for execution.
//! This is a non-blocking call. i.e. it returs immediately without waiting for
//! GPU to finish the execution of tasks.
//! INPUT: pTask -- Pointer to CmTaskInternal object
//! OUTPUT:
//!     CM_SUCCESS if all tasks in the queue are submitted
//!     CM_FAILURE otherwise.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::FlushGroupTask(CmTaskInternal* pTask)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    CM_HAL_EXEC_TASK_GROUP_PARAM param;
    CmKernelData* pKernelData   = nullptr;
    uint32_t kernelDataSize        = 0;
    uint32_t count                  = 0;
    PCM_CONTEXT_DATA pCmData    = nullptr;
    CmEventRT * pEvent          = nullptr;
    PCM_HAL_KERNEL_PARAM pTempData  = nullptr;

    CmSafeMemSet( &param, 0, sizeof( CM_HAL_EXEC_TASK_GROUP_PARAM ) );

    //GT-PIN
    if(this->m_pDevice->CheckGTPinEnabled())
    {
        CMCHK_HR(pTask->GetKernelSurfInfo(param.surEntryInfoArrays));
    }

    pTask->GetKernelCount( count );
    param.numKernels = count;

    param.kernels = MOS_NewArray(PCM_HAL_KERNEL_PARAM, count);
    param.kernelSizes = MOS_NewArray(uint32_t, count);
    param.kernelCurbeOffset = MOS_NewArray(uint32_t, count);
    param.queueOption = m_queueOption;

    CmSafeMemCopy(&param.taskConfig, pTask->GetTaskConfig(), sizeof(param.taskConfig));
    CMCHK_NULL(param.kernels);
    CMCHK_NULL(param.kernelSizes);
    CMCHK_NULL(param.kernelCurbeOffset);

    for( uint32_t i = 0; i < count; i ++ )
    {
        pTask->GetKernelData( i, pKernelData );
        CMCHK_NULL(pKernelData);

        pTask->GetKernelDataSize( i, kernelDataSize );
        if( kernelDataSize == 0)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel data size.");
            hr = CM_FAILURE;
            goto finish;
        }

        pTempData = pKernelData->GetHalCmKernelData( );

        param.kernels[ i ]             = pTempData;
        param.kernelSizes[ i ]        = kernelDataSize;
        param.kernelCurbeOffset [ i ] = pTask->GetKernelCurbeOffset(i);
        param.globalSurfaceUsed        |= pTempData->globalSurfaceUsed;
        param.kernelDebugEnabled       |= pTempData->kernelDebugEnabled;
    }

    pTask->GetSLMSize(param.slmSize);
    if(param.slmSize > MAX_SLM_SIZE_PER_GROUP_IN_1K)
    {
        CM_ASSERTMESSAGE("Error: SLM size exceeds the maximum per group.");
        hr = CM_EXCEED_MAX_SLM_SIZE;
        goto finish;
    }

    if (pTask->IsThreadGroupSpaceCreated())//thread group size
    {
        pTask->GetThreadGroupSpaceSize(param.threadSpaceWidth, param.threadSpaceHeight, param.threadSpaceDepth, param.groupSpaceWidth, param.groupSpaceHeight, param.groupSpaceDepth);
    }

    param.syncBitmap = pTask->GetSyncBitmap();
    param.userDefinedMediaState = pTask->GetMediaStatePtr();

    // Call HAL layer to execute pfnExecuteGroupTask
    pCmData = (PCM_CONTEXT_DATA)m_pDevice->GetAccelData();

    CHK_MOSSTATUS_RETURN_CMERROR( pCmData->cmHalState->pfnSetPowerOption( pCmData->cmHalState, pTask->GetPowerOption() ) );

    CHK_MOSSTATUS_RETURN_CMERROR( pCmData->cmHalState->pfnExecuteGroupTask( pCmData->cmHalState, &param ) );

    if( param.taskIdOut < 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid task ID.");
        hr = CM_FAILURE;
        goto finish;
    }
    TASK_LOG(pTask);
    pTask->GetTaskEvent( pEvent );
    CMCHK_NULL( pEvent );
    CMCHK_HR(pEvent->SetTaskDriverId( param.taskIdOut ));
    CMCHK_HR(pEvent->SetTaskOsData( param.osData ));
    CMCHK_HR(pTask->ResetKernelDataStatus());

    //GT-PIN
    if(this->m_pDevice->CheckGTPinEnabled())
    {
        CMCHK_HR(pEvent->SetSurfaceDetails(param.surEntryInfoArrays));
    }

finish:
    MosSafeDeleteArray( param.kernels );
    MosSafeDeleteArray( param.kernelSizes );
    MosSafeDeleteArray( param.kernelCurbeOffset);

    return hr;
}

//*-----------------------------------------------------------------------------
//! Flush a VEBOX task to HAL CM layer for execution.
//! This is a non-blocking call. i.e. it returs immediately without waiting for
//! GPU to finish the execution of tasks.
//! INPUT: pTask -- Pointer to CmTaskInternal object
//! OUTPUT:
//!     CM_SUCCESS if all tasks in the queue are submitted
//!     CM_FAILURE otherwise.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::FlushVeboxTask(CmTaskInternal* pTask)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    CM_HAL_EXEC_VEBOX_TASK_PARAM param;
    PCM_CONTEXT_DATA pCmData    = nullptr;
    CmEventRT * pEvent          = nullptr;
    uint8_t *pStateData           = nullptr;
    uint8_t *pSurfaceData         = nullptr;
    CmBuffer_RT * temp          = nullptr;

    CmSafeMemSet( &param, 0, sizeof( CM_HAL_EXEC_VEBOX_TASK_PARAM ) );

    //Set VEBOX state data pointer and size
    //Set VEBOX surface data pointer and size
    CM_VEBOX_STATE CmVeboxState;
    CmBufferUP *pVeboxParamBuf = nullptr;
    CM_VEBOX_SURFACE_DATA CmVeboxSurfaceData;
    pTask->GetVeboxState(CmVeboxState);
    pTask->GetVeboxParam(pVeboxParamBuf);
    pTask->GetVeboxSurfaceData(CmVeboxSurfaceData);
    CMCHK_NULL(pVeboxParamBuf);

    temp = static_cast<CmBuffer_RT*>(pVeboxParamBuf);
    temp->GetHandle(param.veboxParamIndex);


    param.cmVeboxState = CmVeboxState;
    param.veboxParam = pVeboxParamBuf;

    param.veboxSurfaceData = CmVeboxSurfaceData;

    //Set VEBOX task id to -1
    param.taskIdOut = -1;

    pCmData = (PCM_CONTEXT_DATA)m_pDevice->GetAccelData();
    CHK_MOSSTATUS_RETURN_CMERROR( pCmData->cmHalState->pfnExecuteVeboxTask( pCmData->cmHalState, &param ) );

    if( param.taskIdOut < 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid task ID.");
        hr = CM_FAILURE;
        goto finish;
    }

    pTask->GetTaskEvent( pEvent );
    CMCHK_NULL( pEvent );
    CMCHK_HR(pEvent->SetTaskDriverId( param.taskIdOut ));
    CMCHK_HR(pEvent->SetTaskOsData( param.osData ));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//! Flush the queue, i.e. submit all tasks in the queue to execute according
//! to their order in the the queue. The queue will be empty after flush,
//! This is a non-blocking call. i.e. it returns immediately without waiting for
//! GPU to finish the execution of tasks.
//! INPUT:
//! OUTPUT:
//!     CM_SUCCESS if all tasks in the queue are submitted
//!     CM_FAILURE otherwise.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::FlushEnqueueWithHintsTask( CmTaskInternal* pTask )
{
    CM_RETURN_CODE               hr             = CM_SUCCESS;
    CM_HAL_EXEC_HINTS_TASK_PARAM param;
    PCM_CONTEXT_DATA             pCmData        = nullptr;
    CmKernelData*                pKernelData    = nullptr;
    uint32_t                     kernelDataSize = 0;
    uint32_t                     count          = 0;
    CmEventRT                    *pEvent        = nullptr;
    PCM_HAL_KERNEL_PARAM         pTempData      = nullptr;

    CmSafeMemSet( &param, 0, sizeof( CM_HAL_EXEC_HINTS_TASK_PARAM ) );

    pTask->GetKernelCount ( count );
    param.numKernels = count;

    param.kernels = MOS_NewArray(PCM_HAL_KERNEL_PARAM, count);
    param.kernelSizes = MOS_NewArray(uint32_t, count);
    param.kernelCurbeOffset = MOS_NewArray(uint32_t, count);
    param.queueOption = m_queueOption;

    CMCHK_NULL(param.kernels);
    CMCHK_NULL(param.kernelSizes);
    CMCHK_NULL(param.kernelCurbeOffset);

    pTask->GetHints(param.hints);
    pTask->GetNumTasksGenerated(param.numTasksGenerated);
    pTask->GetLastTask(param.isLastTask);

    for( uint32_t i = 0; i < count; i ++ )
    {
        pTask->GetKernelData( i, pKernelData );
        CMCHK_NULL( pKernelData );

        pTask->GetKernelDataSize( i, kernelDataSize );
        if( kernelDataSize == 0 )
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel data size.");
            hr = CM_FAILURE;
            goto finish;
        }

        pTempData = pKernelData->GetHalCmKernelData();

        param.kernels[ i ]             = pTempData;
        param.kernelSizes[ i ]         = kernelDataSize;
        param.kernelCurbeOffset[ i ]   = pTask->GetKernelCurbeOffset(i);
    }

    param.userDefinedMediaState = pTask->GetMediaStatePtr();
    pCmData = (PCM_CONTEXT_DATA)m_pDevice->GetAccelData();
    CMCHK_NULL(pCmData);

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnSetPowerOption(pCmData->cmHalState, pTask->GetPowerOption()));

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnExecuteHintsTask(pCmData->cmHalState, &param));

    if( param.taskIdOut < 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid task ID.");
        hr = CM_FAILURE;
        goto finish;
    }

    TASK_LOG(pTask);

    pTask->GetTaskEvent( pEvent );
    CMCHK_NULL( pEvent );
    CMCHK_HR(pEvent->SetTaskDriverId( param.taskIdOut ));
    CMCHK_HR(pEvent->SetTaskOsData( param.osData ));
    CMCHK_HR(pTask->ResetKernelDataStatus());

finish:

    MosSafeDeleteArray( param.kernels );
    MosSafeDeleteArray( param.kernelSizes );
    MosSafeDeleteArray( param.kernelCurbeOffset );

    return hr;
}

//*-----------------------------------------------------------------------------
//! Flush the queue, i.e. submit all tasks in the queue to execute according
//! to their order in the the queue. The queue will be empty after flush,
//! This is a non-blocking call. i.e. it returs immediately without waiting for
//! GPU to finish the execution of tasks.
//! INPUT:
//! OUTPUT:
//!     CM_SUCCESS if all tasks in the queue are submitted
//!     CM_FAILURE otherwise.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::FlushTaskWithoutSync( bool bIfFlushBlock )
{
    int32_t             hr          = CM_SUCCESS;
    CmTaskInternal*     pTask       = nullptr;
    uint32_t            uiTaskType  = CM_TASK_TYPE_DEFAULT;
    uint32_t            freeSurfNum = 0;
    CmSurfaceManager*   pSurfaceMgr = nullptr;
    CSync*              pSurfaceLock = nullptr;

    m_CriticalSection_HalExecute.Acquire(); // Enter HalCm Execute Protection

    while( !m_EnqueuedTasks.IsEmpty() )
    {
        uint32_t flushedTaskCount = m_FlushedTasks.GetCount();
        if ( bIfFlushBlock )
        {
            while( flushedTaskCount >= m_pHalMaxValues->maxTasks )
            {
                // If the task count in flushed queue is no less than hw restrictiion,
                // query the staus of flushed task queue. Remove any finished tasks from the queue
                QueryFlushedTasks();
                flushedTaskCount = m_FlushedTasks.GetCount();
            }
        }
        else
        {
            if( flushedTaskCount >= m_pHalMaxValues->maxTasks )
            {
                // If the task count in flushed queue is no less than hw restrictiion,
                // query the staus of flushed task queue. Remove any finished tasks from the queue
                QueryFlushedTasks();
                flushedTaskCount = m_FlushedTasks.GetCount();
                if( flushedTaskCount >= m_pHalMaxValues->maxTasks )
                {
                    // If none of flushed tasks finishes, we can't flush more taks.
                    break;
                }
            }
        }

        pTask = (CmTaskInternal*)m_EnqueuedTasks.Pop();
        CMCHK_NULL( pTask );

        pTask->GetTaskType(uiTaskType);

        switch(uiTaskType)
        {
            case CM_INTERNAL_TASK_WITH_THREADSPACE:
                hr = FlushGeneralTask(pTask);
                break;

            case CM_INTERNAL_TASK_WITH_THREADGROUPSPACE:
                hr = FlushGroupTask(pTask);
                break;

            case CM_INTERNAL_TASK_VEBOX:
                hr = FlushVeboxTask(pTask);
                break;

            case CM_INTERNAL_TASK_ENQUEUEWITHHINTS:
                hr = FlushEnqueueWithHintsTask(pTask);
                break;

            default:    // by default, assume the task is considered as general task: CM_INTERNAL_TASK_WITH_THREADSPACE
                hr = FlushGeneralTask(pTask);
                break;
        }


        if(hr == CM_SUCCESS)
        {
            m_FlushedTasks.Push( pTask );
            pTask->VtuneSetFlushTime(); // Record Flush Time
        }
        else
        {
            // Failed to flush, destroy the task.
            CmTaskInternal::Destroy( pTask );
        }

    } // loop for task

    QueryFlushedTasks();

finish:
    m_CriticalSection_HalExecute.Release();//Leave HalCm Execute Protection

    //Delayed destroy for resource
    m_pDevice->GetSurfaceManager(pSurfaceMgr);
    if (!pSurfaceMgr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface manager is null.");
        return CM_NULL_POINTER;
    }

    pSurfaceLock = m_pDevice->GetSurfaceCreationLock();
    if (pSurfaceLock == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface creation lock is null.");
        return CM_NULL_POINTER;
    }
    pSurfaceLock->Acquire();
    pSurfaceMgr->DestroySurfaceInPool(freeSurfNum, DELAYED_DESTROY);
    pSurfaceLock->Release();

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Enqueue a Vebox Task
//| Arguments :
//|               pVebox_G75      [in]       Pointer to a CmVebox object
//|               pEvent          [in]       Reference to the pointer to Event
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueVebox(CmVebox * pVebox, CmEvent* & pEvent)
{
    INSERT_API_CALL_LOG();

    int32_t hr                  = CM_SUCCESS;
    CmTaskInternal* pTask   = nullptr;
    int32_t taskDriverId        = -1;
    bool bIsEventVisible    = (pEvent == CM_NO_EVENT)? false:true;
    CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);

    //Check if the input is valid
    if ( pVebox == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Pointer to vebox is null.");
        return CM_NULL_POINTER;
    }
    CmVeboxRT *pVeboxRT = static_cast<CmVeboxRT *>(pVebox);
    CMCHK_HR(CmTaskInternal::Create(m_pDevice,  pVeboxRT, pTask ));

    LARGE_INTEGER nEnqueueTime;
    if ( !(MOS_QueryPerformanceCounter( (uint64_t*)&nEnqueueTime.QuadPart )) )
    {
        CM_ASSERTMESSAGE("Error: Query Performance counter failure.");
        return CM_FAILURE;
    }

    CMCHK_HR(CreateEvent(pTask, bIsEventVisible, taskDriverId, pEventRT));

    if ( pEventRT != nullptr )
    {
        pEventRT->SetEnqueueTime( nEnqueueTime );
    }
    pEvent = pEventRT;

    if (!m_EnqueuedTasks.Push(pTask))
    {
        CM_ASSERTMESSAGE("Error: Push enqueued tasks failure.")
        hr = CM_FAILURE;
        goto finish;
    }

    CMCHK_HR(FlushTaskWithoutSync());

finish:

    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:   Create Event and Update event in m_EventArray
//| Returns:   result of operation
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::CreateEvent(CmTaskInternal *pTask, bool bIsVisible, int32_t &taskDriverId, CmEventRT *&pEvent )
{
    int32_t hr = CM_SUCCESS;

    m_CriticalSection_Event.Acquire();

    uint32_t freeSlotInEventArray = m_EventArray.GetFirstFreeIndex();

    hr = CmEventRT::Create( freeSlotInEventArray, this, pTask, taskDriverId, m_pDevice, bIsVisible, pEvent );

    if (hr == CM_SUCCESS)
    {

        m_EventArray.SetElement( freeSlotInEventArray, pEvent );
        m_EventCount ++;

        pTask->SetTaskEvent( pEvent );

        if(!bIsVisible)
        {
            pEvent = nullptr;
        }

    }
    else
    {
        CM_ASSERTMESSAGE("Error: Create Event failure.")
    }

    m_CriticalSection_Event.Release();

    return hr;
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       EnqueueCopyCPUToGPUFullStride()
//| Purpose:    Copy data from system memory to video memory (surface)
//| Arguments:
//|             pSurface      [in]  Pointer to a CmSurface2D object as copy destination
//|             pSysMem       [in]  Pointer to a system memory as copy source
//|             widthStride   [in]  Width stride in bytes for system memory (to calculate start of next line)
//|             heightStride  [in]  Width stride in row for system memory (to calculate start of next plane)
//|             option        [in]  Option passed from user, blocking copy, non-blocking copy or disable turbo boost
//|             pEvent        [in,out]  Reference to the pointer to Event
//| Returns:    Result of the operation.
//|
//| Restrictions & Notes:
//|             1) pSysMem must be 16-byte aligned.
//|             2) Surface's width must be 16-byte aligned regarding performance.
//|             3) widthStride and heightStride are used to indicate the padding information in system memory
//|                 widthStride = width_in_pixel * bytes_per_pixel + padding_in_bytes
//|                 heightStride = height + padding_in_row
//*---------------------------------------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueCopyCPUToGPUFullStride( CmSurface2D* pSurface,
                                                     const unsigned char* pSysMem,
                                                     const uint32_t widthStride,
                                                     const uint32_t heightStride,
                                                     const uint32_t option,
                                                     CmEvent* & pEvent )
{
    INSERT_API_CALL_LOG();

    CmSurface2DRT *pSurfaceRT = static_cast<CmSurface2DRT *>(pSurface);
    return EnqueueCopyInternal(pSurfaceRT, (unsigned char*)pSysMem, widthStride, heightStride, CM_FASTCOPY_CPU2GPU, option, pEvent);
}


//*---------------------------------------------------------------------------------------------------------
//| Name:       EnqueueCopyGPUToCPUFullStride()
//| Purpose:    Copy data from tiled video memory (surface) to linear system memory
//| Arguments:
//|             pSurface      [in]  Pointer to a CmSurface2D object as copy source
//|             pSysMem       [in]  Pointer to a system memory as copy destination
//|             widthStride   [in]  Width stride in bytes for system memory (to calculate start of next line)
//|             heightStride  [in]  Width stride in row for system memory (to calculate start of next plane)
//|             option        [in]  Option passed from user, blocking copy,non-blocking copy or disable turbo boost
//|             pEvent        [in,out]  Reference to the pointer to Event
//| Returns:    Result of the operation.
//|
//| Restrictions & Notes:
//|             1) pSysMem must be 16-byte aligned.
//|             2) Surface's width must be 16-byte aligned regarding performance.
//|             3) widthStride and heightStride are used to indicate the padding information in system memory
//|                 widthStride = width_in_pixel * bytes_per_pixel + padding_in_bytes
//|                 heightStride = height + padding_in_row
//*---------------------------------------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueCopyGPUToCPUFullStride( CmSurface2D* pSurface,
                                                     unsigned char* pSysMem,
                                                     const uint32_t widthStride,
                                                     const uint32_t heightStride,
                                                     const uint32_t option,
                                                     CmEvent* & pEvent )
{
    INSERT_API_CALL_LOG();

    CmSurface2DRT *pSurfaceRT = static_cast<CmSurface2DRT *>(pSurface);
    return EnqueueCopyInternal(pSurfaceRT, pSysMem, widthStride, heightStride, CM_FASTCOPY_GPU2CPU, option, pEvent);
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       CreateGPUCopyKernel()
//| Purpose:    Create GPUCopy kernel, reuse the kernel if it has been created and resuable
//| Arguments:
//|             WidthInByte      [in]  surface's width in bytes
//|             height           [in]  surface's height
//|             format           [in]  surface's height
//|             CopyDirection    [in]  copy direction, cpu -> gpu or gpu -> cpu
//|             pGPUCopyKrnParam [out] kernel param
//|
//| Returns:    Result of the operation.
//|
//*---------------------------------------------------------------------------------------------------------
int32_t CmQueueRT::CreateGPUCopyKernel(uint32_t WidthInByte,
                                       uint32_t height,
                                       CM_SURFACE_FORMAT format,
                                       CM_GPUCOPY_DIRECTION CopyDirection,
                                       CM_GPUCOPY_KERNEL* &pGPUCopyKrnParam)
{
    int32_t     hr                 = CM_SUCCESS;

    //Search existing kernel
    CMCHK_HR(SearchGPUCopyKernel(WidthInByte, height, format, CopyDirection, pGPUCopyKrnParam));

    if(pGPUCopyKrnParam != nullptr)
    { // reuse
        GPUCOPY_KERNEL_LOCK(pGPUCopyKrnParam);
    }
    else
    {
        pGPUCopyKrnParam   = new (std::nothrow) CM_GPUCOPY_KERNEL ;
        CMCHK_NULL(pGPUCopyKrnParam);
        CmSafeMemSet(pGPUCopyKrnParam, 0, sizeof(CM_GPUCOPY_KERNEL));

        CMCHK_HR(AllocateGPUCopyKernel(WidthInByte, height, format, CopyDirection, pGPUCopyKrnParam->pKernel));
        CMCHK_HR(GetGPUCopyKrnID(WidthInByte, height, format, CopyDirection, pGPUCopyKrnParam->KernelID));
        GPUCOPY_KERNEL_LOCK(pGPUCopyKrnParam);

        CMCHK_HR(AddGPUCopyKernel(pGPUCopyKrnParam));
    }

finish:
    if( hr != CM_SUCCESS)
    {
        CmSafeDelete(pGPUCopyKrnParam);
    }

    return hr;
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       SearchGPUCopyKernel()
//| Purpose:    Search if the required kernel exists
//| Arguments:
//|             WidthInByte      [in]  surface's width in bytes
//|             height           [in]  surface's height
//|             format           [in]  surface's height
//|             CopyDirection    [in]  copy direction, cpu -> gpu or gpu -> cpu
//|             pGPUCopyKrnParam [out] kernel param
//|
//| Returns:    Result of the operation.
//|
//*---------------------------------------------------------------------------------------------------------
int32_t CmQueueRT::SearchGPUCopyKernel(uint32_t WidthInByte,
                                       uint32_t height,
                                       CM_SURFACE_FORMAT format,
                                       CM_GPUCOPY_DIRECTION CopyDirection,
                                       CM_GPUCOPY_KERNEL* &pKernelParam)
{
    int32_t     hr = CM_SUCCESS;
    CM_GPUCOPY_KERNEL *pGPUCopyKernel = nullptr;
    CM_GPUCOPY_KERNEL_ID KernelTypeID = GPU_COPY_KERNEL_UNKNOWN;

    pKernelParam = nullptr;
    CMCHK_HR(GetGPUCopyKrnID(WidthInByte, height, format, CopyDirection, KernelTypeID));

    for(uint32_t index =0 ;  index< m_CopyKrnParamArrayCount; index++)
    {
        pGPUCopyKernel = (CM_GPUCOPY_KERNEL*)m_CopyKrnParamArray.GetElement(index);
        if(pGPUCopyKernel != nullptr)
        {
            if(!pGPUCopyKernel->bLocked &&
               pGPUCopyKernel->KernelID == KernelTypeID)
            {
                pKernelParam = pGPUCopyKernel;
                break;
            }
        }
    }

finish:
    return hr;
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       AddGPUCopyKernel()
//| Purpose:    Add new kernel into m_CopyKrnParamArray
//| Arguments:
//|             WidthInByte      [in]  surface's width in bytes
//|             height           [in]  surface's height
//|             format           [in]  surface's height
//|             CopyDirection    [in]  copy direction, cpu -> gpu or gpu -> cpu
//|             pGPUCopyKrnParam [out] kernel param
//|
//| Returns:    Result of the operation.
//|
//*---------------------------------------------------------------------------------------------------------
int32_t CmQueueRT::AddGPUCopyKernel(CM_GPUCOPY_KERNEL* &pKernelParam)
{
    int32_t hr = CM_SUCCESS;
    // critical section protection
    CLock locker(m_CriticalSection_GPUCopyKrn);

    CMCHK_NULL_RETURN(pKernelParam, CM_INVALID_GPUCOPY_KERNEL);

    // the newly created kernel must be locked
    if(!pKernelParam->bLocked)
    {
        CM_ASSERTMESSAGE("Error: The newly created kernel must be locked.")
        hr = CM_INVALID_GPUCOPY_KERNEL;
        goto finish;
    }

    m_CopyKrnParamArray.SetElement(m_CopyKrnParamArrayCount, pKernelParam);
    m_CopyKrnParamArrayCount ++;

finish:
    return hr;
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       GetGPUCopyKrnID()
//| Purpose:    Calculate the kernel ID accroding surface's width, height and copy direction
//| Arguments:
//|             WidthInByte      [in]  surface's width in bytes
//|             height           [in]  surface's height
//|             format           [in]  surface's height
//|             CopyDirection    [in]  copy direction, cpu -> gpu or gpu -> cpu
//|             KernelID         [out] kernel id
//|
//| Returns:    Result of the operation.
//|
//*---------------------------------------------------------------------------------------------------------
int32_t CmQueueRT::GetGPUCopyKrnID( uint32_t WidthInByte, uint32_t height, CM_SURFACE_FORMAT format,
            CM_GPUCOPY_DIRECTION CopyDirection, CM_GPUCOPY_KERNEL_ID &KernelID )
{
    int32_t hr = CM_SUCCESS;

    KernelID = GPU_COPY_KERNEL_UNKNOWN;

    if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
    {
        switch(CopyDirection)
        {
            case CM_FASTCOPY_GPU2CPU:
                if ( (height&0x7) ||(WidthInByte&0x7f))
                {
                    KernelID = GPU_COPY_KERNEL_GPU2CPU_UNALIGNED_NV12_ID ;
                }
                else
                {   // height 8-row aligned, width_byte 128 multiple
                    KernelID = GPU_COPY_KERNEL_GPU2CPU_ALIGNED_NV12_ID ;
                }
                break;

            case CM_FASTCOPY_CPU2GPU:
                KernelID = GPU_COPY_KERNEL_CPU2GPU_NV12_ID;
                break;

            case CM_FASTCOPY_GPU2GPU:
                KernelID = GPU_COPY_KERNEL_GPU2GPU_NV12_ID;
                break;

            case CM_FASTCOPY_CPU2CPU:
                KernelID = GPU_COPY_KERNEL_CPU2CPU_ID;
                break;

            default :
                CM_ASSERTMESSAGE("Error: Invalid fast copy direction.")
                hr = CM_FAILURE;
                break;
        }
    }
    else
    {
        switch(CopyDirection)
        {
            case CM_FASTCOPY_GPU2CPU:
                if ( (height&0x7) ||(WidthInByte&0x7f))
                {
                    KernelID = GPU_COPY_KERNEL_GPU2CPU_UNALIGNED_ID;
                }
                else
                {   // height 8-row aligned, width_byte 128 multiple
                    KernelID = GPU_COPY_KERNEL_GPU2CPU_ALIGNED_ID;
                }
                break;

            case CM_FASTCOPY_CPU2GPU:
                KernelID = GPU_COPY_KERNEL_CPU2GPU_ID;
                break;

            case CM_FASTCOPY_GPU2GPU:
                KernelID = GPU_COPY_KERNEL_GPU2GPU_ID;
                break;

            case CM_FASTCOPY_CPU2CPU:
                KernelID = GPU_COPY_KERNEL_CPU2CPU_ID;
                break;

            default :
                CM_ASSERTMESSAGE("Error: Invalid fast copy direction.")
                hr = CM_FAILURE;
                break;
        }
    }

    return hr;
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       AllocateGPUCopyKernel()
//| Purpose:    Allocate GPUCopy Kernel
//| Arguments:
//|             WidthInByte      [in]  surface's width in bytes
//|             height           [in]  surface's height
//|             format           [in]  surface's height
//|             CopyDirection    [in]  copy direction, cpu -> gpu or gpu -> cpu
//|             pKernel          [out] pointer to created kernel
//|
//| Returns:    Result of the operation.
//|
//*---------------------------------------------------------------------------------------------------------
int32_t CmQueueRT::AllocateGPUCopyKernel( uint32_t WidthInByte, uint32_t height, CM_SURFACE_FORMAT format,
            CM_GPUCOPY_DIRECTION CopyDirection, CmKernel *&pKernel )
{
    int32_t          hr                 = CM_SUCCESS;
    CmProgram       *pGPUcopyProgram    = nullptr;

    CMCHK_HR( m_pDevice->LoadPredefinedCopyKernel(pGPUcopyProgram));
    CMCHK_NULL(pGPUcopyProgram);

    if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
    {
        switch(CopyDirection)
        {
            case CM_FASTCOPY_GPU2CPU:
                if ( (height&0x7) ||(WidthInByte&0x7f))
                {
                    CMCHK_HR(m_pDevice->CreateKernel( pGPUcopyProgram, _NAME( surfaceCopy_read_NV12_32x32 ) , pKernel,"PredefinedGPUCopyKernel"));
                }
                else
                {   // height 8-row aligned, width_byte 128 multiple
                    CMCHK_HR(m_pDevice->CreateKernel( pGPUcopyProgram, _NAME( surfaceCopy_read_NV12_aligned_32x32 ) , pKernel,"PredefinedGPUCopyKernel"));
                }
                break;

            case CM_FASTCOPY_CPU2GPU:
                CMCHK_HR( m_pDevice->CreateKernel( pGPUcopyProgram, _NAME( surfaceCopy_write_NV12_32x32 ), pKernel, "PredefinedGPUCopyKernel"));
                break;

            case CM_FASTCOPY_GPU2GPU:
                CMCHK_HR(m_pDevice->CreateKernel(pGPUcopyProgram, _NAME(SurfaceCopy_2DTo2D_NV12_32x32), pKernel, "PredefinedGPUCopyKernel"));
                break;

            case CM_FASTCOPY_CPU2CPU:
                CMCHK_HR(m_pDevice->CreateKernel(pGPUcopyProgram, _NAME(SurfaceCopy_BufferToBuffer_4k), pKernel, "PredefinedGPUCopyKernel"));
                break;

            default :
                CM_ASSERTMESSAGE("Error: Invalid fast copy direction.")
                hr = CM_FAILURE;
                break;
        }
    }
    else
    {
        switch(CopyDirection)
        {
            case CM_FASTCOPY_GPU2CPU:
                if ( (height&0x7) ||(WidthInByte&0x7f))
                {
                    CMCHK_HR(m_pDevice->CreateKernel( pGPUcopyProgram, _NAME( surfaceCopy_read_32x32 ) , pKernel, "PredefinedGPUCopyKernel"));
                }
                else
                {   // height 8-row aligned, width_byte 128 multiple
                    CMCHK_HR(m_pDevice->CreateKernel( pGPUcopyProgram, _NAME( surfaceCopy_read_aligned_32x32  ) , pKernel, "PredefinedGPUCopyKernel"));
                }
                break;

            case CM_FASTCOPY_CPU2GPU:
                CMCHK_HR( m_pDevice->CreateKernel( pGPUcopyProgram, _NAME( surfaceCopy_write_32x32 ), pKernel, "PredefinedGPUCopyKernel" ));
                break;

            case CM_FASTCOPY_GPU2GPU:
                CMCHK_HR(m_pDevice->CreateKernel(pGPUcopyProgram, _NAME(SurfaceCopy_2DTo2D_32x32), pKernel, "PredefinedGPUCopyKernel"));
                break;

            case CM_FASTCOPY_CPU2CPU:
                CMCHK_HR(m_pDevice->CreateKernel(pGPUcopyProgram, _NAME(SurfaceCopy_BufferToBuffer_4k), pKernel, "PredefinedGPUCopyKernel"));
                break;

            default :
                CM_ASSERTMESSAGE("Error: Invalid fast copy direction.")
                hr = CM_FAILURE;
                break;
        }
    }

finish:
    return hr;
}
}