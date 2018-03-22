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

#define GPUCOPY_KERNEL_LOCK(a) ((a)->locked = true)
#define GPUCOPY_KERNEL_UNLOCK(a) ((a)->locked = false)

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Queue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::Create(CmDeviceRT *device,
                          CmQueueRT* &queue,
                          CM_QUEUE_CREATE_OPTION queueCreateOption)
{
    int32_t result = CM_SUCCESS;
    queue = new (std::nothrow) CmQueueRT(device, queueCreateOption);
    if( queue )
    {
        result = queue->Initialize( );
        if( result != CM_SUCCESS )
        {
            CmQueueRT::Destroy( queue);
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
int32_t CmQueueRT::Destroy(CmQueueRT* &queue )
{
    if( queue == nullptr )
    {
        return CM_FAILURE;
    }

    uint32_t result = queue->CleanQueue();
    CmSafeDelete( queue );

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of Cm Queue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmQueueRT::CmQueueRT(CmDeviceRT *device,
                     CM_QUEUE_CREATE_OPTION queueCreateOption):
    m_device(device),
    m_eventArray(CM_INIT_EVENT_COUNT),
    m_eventCount(0),
    m_halMaxValues(nullptr),
    m_copyKernelParamArray(CM_INIT_GPUCOPY_KERNL_COUNT),
    m_copyKernelParamArrayCount(0),
    m_queueOption(queueCreateOption)
{

}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of Cm Queue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmQueueRT::~CmQueueRT()
{
    uint32_t eventReleaseTimes = 0;

    uint32_t eventArrayUsedSize = m_eventArray.GetMaxSize();
    for( uint32_t i = 0; i < eventArrayUsedSize; i ++ )
    {
        CmEventRT* event = (CmEventRT*)m_eventArray.GetElement( i );
        eventReleaseTimes = 0;
        while( event )
        {   // destroy the event no matter if it is released by user
            if(eventReleaseTimes > 2)
            {
                // The max of event's reference cout is 2
                // if the event is not released after 2 times, there is something wrong
                CM_ASSERTMESSAGE("Error: The max of event's reference cout is 2.");
                break;
            }
            CmEventRT::Destroy( event );
            eventReleaseTimes ++;
        }
    }
    m_eventArray.Delete();

    // Do not destroy the kernel in m_copyKernelParamArray.
    // They have been destoyed in ~CmDevice() before destroying Queue
    for( uint32_t i = 0; i < m_copyKernelParamArrayCount; i ++ )
    {
        CM_GPUCOPY_KERNEL *gpuCopyParam = (CM_GPUCOPY_KERNEL*)m_copyKernelParamArray.GetElement( i );
        CmSafeDelete(gpuCopyParam);
    }

    m_copyKernelParamArray.Delete();

}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Cm Queue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::Initialize()
{
    PCM_HAL_STATE cmHalState = ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;
    CM_HAL_MAX_VALUES_EX* halMaxValuesEx = nullptr;
    CM_RETURN_CODE hr = CM_SUCCESS;
    m_device->GetHalMaxValues(m_halMaxValues, halMaxValuesEx);

    // Creates or gets GPU Context for the test
    if (m_queueOption.UserGPUContext == true)
    {
        // Checks if it is the user-provided GPU context. If it is valid, we will create the queue with the existing Context
        if (cmHalState->osInterface->pfnIsGpuContextValid(cmHalState->osInterface, (MOS_GPU_CONTEXT)m_queueOption.GPUContext) != MOS_STATUS_SUCCESS)
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
            CHK_MOSSTATUS_RETURN_CMERROR(cmHalState->pfnCreateGPUContext(cmHalState, cmHalState->gpuContext, MOS_GPU_NODE_3D));
            m_queueOption.GPUContext = cmHalState->gpuContext;
        }
        else if (m_queueOption.QueueType == CM_QUEUE_TYPE_COMPUTE)
        {
            CHK_MOSSTATUS_RETURN_CMERROR(cmHalState->pfnCreateGPUContext(cmHalState, MOS_GPU_CONTEXT_CM_COMPUTE, MOS_GPU_NODE_COMPUTE));
            m_queueOption.GPUContext = MOS_GPU_CONTEXT_CM_COMPUTE;
        }
        else
        {
            // Returns failure
            CM_ASSERTMESSAGE("Error: The QueueType is not supported by MDF.");
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
int32_t CmQueueRT::GetTaskHasThreadArg(CmKernelRT* kernelArray[], uint32_t numKernels, bool& threadArgExists)
{
    threadArgExists = false;

    for(uint32_t krn = 0; krn < numKernels; krn++)
    {
        if( !kernelArray[krn] )
        {
            CM_ASSERTMESSAGE("Error: The kernel in the task have no thread argument.");
            return CM_FAILURE;
        }

        if( kernelArray[krn]->IsThreadArgExisted( ) )
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
//|               kernelArray      [in]       Pointer to kernel array
//|               event            [in]       Reference to the pointer to Event
//|               threadSpace               [out]      Pointer to thread space
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::Enqueue(
           CmTask* kernelArray,
           CmEvent* & event,
           const CmThreadSpace* threadSpace)
{
    INSERT_API_CALL_LOG();

    int32_t result;

    if(kernelArray == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Kernel array is null.");
        return CM_INVALID_ARG_VALUE;
    }

    CmTaskRT *kernelArrayRT = static_cast<CmTaskRT *>(kernelArray);
    uint32_t kernelCount = 0;
    kernelCount = kernelArrayRT->GetKernelCount();
    if( kernelCount == 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel count.");
        return CM_FAILURE;
    }

    if( kernelCount > m_halMaxValues->maxKernelsPerTask )
    {
        CM_ASSERTMESSAGE("Error: Kernel count exceeds max kernel per enqueue.");
        return CM_EXCEED_MAX_KERNEL_PER_ENQUEUE;
    }

    const CmThreadSpaceRT *threadSpaceRTConst = static_cast<const CmThreadSpaceRT *>(threadSpace);
    if (threadSpaceRTConst && threadSpaceRTConst->IsThreadAssociated())
    {
        if (threadSpaceRTConst->GetNeedSetKernelPointer() && threadSpaceRTConst->KernelPointerIsNULL())
        {
            CmKernelRT* tmp = nullptr;
            tmp = kernelArrayRT->GetKernelPointer(0);
            threadSpaceRTConst->SetKernelPointer(tmp);
        }
    }

#if _DEBUG
    if (threadSpaceRTConst)
    {
        CmThreadSpaceRT *threadSpaceRT = const_cast<CmThreadSpaceRT*>(threadSpaceRTConst);
        if (!threadSpaceRT->IntegrityCheck(kernelArrayRT))
        {
            CM_ASSERTMESSAGE("Error: Invalid thread space.");
            return CM_INVALID_THREAD_SPACE;
        }
    }
#endif

    if(m_device->IsPrintEnable())
    {
        m_device->ClearPrintBuffer();
    }

    typedef CmKernelRT* pCmKernel;
    CmKernelRT** tmp = MOS_NewArray(pCmKernel, (kernelCount + 1));
    if(tmp == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    uint32_t totalThreadNumber = 0;
    for(uint32_t i = 0; i < kernelCount; i++)
    {
        tmp[ i ] = kernelArrayRT->GetKernelPointer(i);

        uint32_t singleThreadNumber = 0;
        tmp[i]->GetThreadCount(singleThreadNumber);
        if (singleThreadNumber == 0)
        {
            CmThreadSpaceRT *threadSpaceRT = const_cast<CmThreadSpaceRT*>(threadSpaceRTConst);
            if (threadSpaceRT)
            {
                uint32_t width, height;
                threadSpaceRT->GetThreadSpaceSize(width, height);
                singleThreadNumber = width*height;
            }
        }
        totalThreadNumber += singleThreadNumber;
    }
    tmp[kernelCount ] = nullptr;

    CmEventRT *eventRT = static_cast<CmEventRT *>(event);
    result = Enqueue_RT(tmp, kernelCount, totalThreadNumber, eventRT, threadSpaceRTConst, kernelArrayRT->GetSyncBitmap(), kernelArrayRT->GetPowerOption(),
                        kernelArrayRT->GetConditionalEndBitmap(), kernelArrayRT->GetConditionalEndInfo(), kernelArrayRT->GetTaskConfig());

    if (eventRT)
    {
        eventRT->SetKernelNames(kernelArrayRT, const_cast<CmThreadSpaceRT*>(threadSpaceRTConst), nullptr);
    }

    event = eventRT;
    MosSafeDeleteArray( tmp );

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:      Enqueue Task
//| Arguments :
//|               kernelArray      [in]       Pointer to kernel array
//|               event            [in]       Reference to the pointer to Event
//|               threadSpace               [out]      Pointer to thread space
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::Enqueue_RT(
                        CmKernelRT* kernelArray[],
                        const uint32_t kernelCount,
                        const uint32_t totalThreadCount,
                        CmEventRT* & event,
                        const CmThreadSpaceRT* threadSpace,
                        uint64_t    syncBitmap,
                        PCM_POWER_OPTION powerOption,
                        uint64_t    conditionalEndBitmap,
                        CM_HAL_CONDITIONAL_BB_END_INFO* conditionalEndInfo,
                        PCM_TASK_CONFIG  taskConfig)
{
    if(kernelArray == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Kernel array is NULL.");
        return CM_INVALID_ARG_VALUE;
    }

    if( kernelCount == 0 )
    {
        CM_ASSERTMESSAGE("Error: There are no valid kernels.");
        return CM_INVALID_ARG_VALUE;
    }

    bool isEventVisible = (event == CM_NO_EVENT)? false:true;

    CLock Locker(m_criticalSectionTaskInternal);
    CmTaskInternal* task = nullptr;
    int32_t result = CmTaskInternal::Create(kernelCount, totalThreadCount, kernelArray, threadSpace, m_device, syncBitmap, task, conditionalEndBitmap, conditionalEndInfo);
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

    result = CreateEvent(task, isEventVisible, taskDriverId, event);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Create event failure.");
        return result;
    }
    if ( event != nullptr )
    {
        event->SetEnqueueTime( nEnqueueTime );
    }

    task->SetPowerOption( powerOption );

    task->SetProperty(taskConfig);

    if( !m_enqueuedTasks.Push( task ) )
    {
        CM_ASSERTMESSAGE("Error: Push enqueued tasks failure.");
        return CM_FAILURE;
    }

    result = FlushTaskWithoutSync();

    return result;
}

int32_t CmQueueRT::Enqueue_RT(CmKernelRT* kernelArray[],
                        const uint32_t kernelCount,
                        const uint32_t totalThreadCount,
                        CmEventRT* & event,
                        const CmThreadGroupSpace* threadGroupSpace,
                        uint64_t    syncBitmap,
                        PCM_POWER_OPTION powerOption,
                        PCM_TASK_CONFIG  taskConfig)
{
    if(kernelArray == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Kernel array is NULL.");
        return CM_INVALID_ARG_VALUE;
    }

    if( kernelCount == 0 )
    {
        CM_ASSERTMESSAGE("Error: There are no valid kernels.");
        return CM_INVALID_ARG_VALUE;
    }

    CLock Locker(m_criticalSectionTaskInternal);

    CmTaskInternal* task = nullptr;
    int32_t result = CmTaskInternal::Create( kernelCount, totalThreadCount, kernelArray, threadGroupSpace, m_device, syncBitmap, task );
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
    result = CreateEvent(task, !(event == CM_NO_EVENT) , taskDriverId, event);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Create event failure.");
        return result;
    }
    if ( event != nullptr )
    {
        event->SetEnqueueTime( nEnqueueTime );
    }

    task->SetPowerOption( powerOption );

    task->SetProperty(taskConfig);

    if( !m_enqueuedTasks.Push( task ) )
    {
        CM_ASSERTMESSAGE("Error: Push enqueued tasks failure.")
        return CM_FAILURE;
    }

    result = FlushTaskWithoutSync();

    return result;
}

int32_t CmQueueRT::Enqueue_RT( CmKernelRT* kernelArray[],
                        CmEventRT* & event,
                        uint32_t numTasksGenerated,
                        bool isLastTask,
                        uint32_t hints,
                        PCM_POWER_OPTION powerOption)
{
    int32_t result = CM_FAILURE;
    uint32_t kernelCount = 0;
    CmTaskInternal* task = nullptr;
    int32_t taskDriverId = -1;
    bool isEventVisible = (event == CM_NO_EVENT) ? false:true;
    bool threadArgExists = false;

    if( kernelArray == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Kernel array is NULL.");
        return CM_INVALID_ARG_VALUE;
    }
    while( kernelArray[ kernelCount ] )
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
        kernelArray[i]->GetThreadCount( threadCount );
        totalThreadCount += threadCount;
    }

    if( GetTaskHasThreadArg(kernelArray, kernelCount, threadArgExists) != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Thread argument checking fails.");
        return CM_FAILURE;
    }

    if( !threadArgExists )
    {
        if (totalThreadCount > m_halMaxValues->maxUserThreadsPerTaskNoThreadArg )
        {
            CM_ASSERTMESSAGE("Error: Maximum number of threads per task exceeded.");
            return CM_EXCEED_MAX_THREAD_AMOUNT_PER_ENQUEUE;
        }
    }
    else
    {
        if( totalThreadCount > m_halMaxValues->maxUserThreadsPerTask )
        {
            CM_ASSERTMESSAGE("Error: Maximum number of threads per task exceeded.");
            return CM_EXCEED_MAX_THREAD_AMOUNT_PER_ENQUEUE;
        }
    }

    CLock Locker(m_criticalSectionTaskInternal);

    result = CmTaskInternal::Create( kernelCount, totalThreadCount, kernelArray, task, numTasksGenerated, isLastTask, hints, m_device );
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

    result = CreateEvent(task, isEventVisible, taskDriverId, event);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Create event failure.");
        return result;
    }
    if ( event != nullptr )
    {
        event->SetEnqueueTime( nEnqueueTime );
    }

    for( uint32_t i = 0; i < kernelCount; ++i )
    {
        CmKernelRT* kernel = nullptr;
        task->GetKernel(i, kernel);
        if( kernel != nullptr )
        {
            kernel->SetAdjustedYCoord(0);
        }
    }

    task->SetPowerOption( powerOption );

    if (!m_enqueuedTasks.Push(task))
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
CM_RT_API int32_t CmQueueRT::EnqueueWithGroup( CmTask* task, CmEvent* & event, const CmThreadGroupSpace* threadGroupSpace)
{
    INSERT_API_CALL_LOG();

    int32_t result;

    if(task == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Kernel array is NULL.");
        return CM_INVALID_ARG_VALUE;
    }

    CmTaskRT *taskRT = static_cast<CmTaskRT *>(task);
    uint32_t count = 0;
    count = taskRT->GetKernelCount();

    if( count == 0 )
    {
        CM_ASSERTMESSAGE("Error: There are no valid kernels.");
        return CM_FAILURE;
    }

    if(m_device->IsPrintEnable())
    {
        m_device->ClearPrintBuffer();
    }

    typedef CmKernelRT* pCmKernel;
    CmKernelRT** tmp = MOS_NewArray(pCmKernel, (count+1));
    if(tmp == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    uint32_t totalThreadNumber = 0;
    for(uint32_t i = 0; i < count; i++)
    {
        uint32_t singleThreadNumber = 0;
        tmp[ i ] = taskRT->GetKernelPointer(i);

        //Thread arguments is not allowed in GPGPU_WALKER path
        if(tmp[i]->IsThreadArgExisted())
        {
            CM_ASSERTMESSAGE("Error: No thread Args allowed when using group space");
            MosSafeDeleteArray(tmp);
            return CM_THREAD_ARG_NOT_ALLOWED;
        }

        tmp[i]->GetThreadCount(singleThreadNumber);
        totalThreadNumber += singleThreadNumber;
    }
    tmp[count ] = nullptr;

    CmEventRT *eventRT = static_cast<CmEventRT *>(event);
    result = Enqueue_RT( tmp, count, totalThreadNumber, eventRT,
                         threadGroupSpace, taskRT->GetSyncBitmap(),
                         taskRT->GetPowerOption(),
                         taskRT->GetTaskConfig());

    if (eventRT)
    {
        eventRT->SetKernelNames(taskRT, nullptr, const_cast<CmThreadGroupSpace*>(threadGroupSpace));
    }

    event = eventRT;
    MosSafeDeleteArray( tmp );

    return result;
}

CM_RT_API int32_t CmQueueRT::EnqueueWithHints(
                                        CmTask* kernelArray,
                                        CmEvent* & event,
                                        uint32_t hints)
{
    INSERT_API_CALL_LOG();

    int32_t            hr                = CM_FAILURE;
    uint32_t           count             = 0;
    uint32_t           index             = 0;
    CmKernelRT**         kernels          = nullptr;
    uint32_t           numTasks          = 0;
    bool               splitTask         = false;
    bool               lastTask          = false;
    uint32_t           numTasksGenerated = 0;
    CmEventRT          *eventRT = static_cast<CmEventRT *>(event);

    if (kernelArray == nullptr)
    {
        return CM_INVALID_ARG_VALUE;
    }
    CmTaskRT         *kernelArrayRT   = static_cast<CmTaskRT *>(kernelArray);
    count = kernelArrayRT->GetKernelCount();
    if( count == 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel count.");
        hr = CM_FAILURE;
        goto finish;
    }

    if( count > m_halMaxValues->maxKernelsPerTask )
    {
        CM_ASSERTMESSAGE("Error: Kernel count exceeds maximum kernel per enqueue.");
        hr = CM_EXCEED_MAX_KERNEL_PER_ENQUEUE;
        goto finish;
    }

    for (uint32_t i = 0; i < count; ++i)
    {
        CmKernelRT* kernelTmp = nullptr;
        CmThreadSpaceRT* threadSpaceTmp = nullptr;
        kernelTmp = kernelArrayRT->GetKernelPointer(i);
        CMCHK_NULL(kernelTmp);
        kernelTmp->GetThreadSpace(threadSpaceTmp);
        CMCHK_NULL(threadSpaceTmp);
        if (threadSpaceTmp->GetNeedSetKernelPointer() && threadSpaceTmp->KernelPointerIsNULL())
        {
            threadSpaceTmp->SetKernelPointer(kernelTmp);
        }
    }

#if _DEBUG
    if( !kernelArrayRT->IntegrityCheckKernelThreadspace() )
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

    if( m_device->IsPrintEnable() )
    {
        m_device->ClearPrintBuffer();
    }

    kernels = MOS_NewArray(CmKernelRT*, (count + 1));
    CMCHK_NULL(kernels);

    do
    {
        for (index = 0; index < count; ++index)
        {
            kernels[ index ] = kernelArrayRT->GetKernelPointer( index );
        }

        kernels[ count ] = nullptr;

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

        CMCHK_HR(Enqueue_RT( kernels, eventRT, numTasksGenerated, lastTask, hints, kernelArrayRT->GetPowerOption() ));
        event = eventRT;
        numTasksGenerated++;

    }while(numTasksGenerated < numTasks);

finish:
    MosSafeDeleteArray( kernels );

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
CM_RT_API int32_t CmQueueRT::EnqueueCopyCPUToGPU( CmSurface2D* surface, const unsigned char* sysMem, CmEvent* & event )
{
    INSERT_API_CALL_LOG();

    CmSurface2DRT *surfaceRT = static_cast<CmSurface2DRT *>(surface);
    return EnqueueCopyInternal(surfaceRT, (unsigned char*)sysMem, 0, 0, CM_FASTCOPY_CPU2GPU, CM_FASTCOPY_OPTION_NONBLOCKING, event);
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
CM_RT_API int32_t CmQueueRT::EnqueueCopyGPUToCPU( CmSurface2D* surface, unsigned char* sysMem, CmEvent* & event )
{
    INSERT_API_CALL_LOG();

    CmSurface2DRT *surfaceRT = static_cast<CmSurface2DRT *>(surface);
    return EnqueueCopyInternal(surfaceRT, sysMem, 0, 0, CM_FASTCOPY_GPU2CPU, CM_FASTCOPY_OPTION_NONBLOCKING, event);
}

int32_t CmQueueRT::EnqueueUnalignedCopyInternal( CmSurface2DRT* surface, unsigned char* sysMem, const uint32_t widthStride, const uint32_t heightStride, CM_GPUCOPY_DIRECTION direction, CmEvent* &event )
{
    int32_t         hr                          = CM_SUCCESS;
    uint32_t        bufferupSize               = 0;
    uint32_t        dstAddShiftOffset           = 0;
    uint32_t        threadWidth                 = 0;
    uint32_t        threadHeight                = 0;
    uint32_t        threadNum                   = 0;
    uint32_t        auxiliaryBufferupSize     = 0;
    uint32_t        width                       = 0;
    uint32_t        height                      = 0;
    uint32_t        sizePerPixel                = 0;
    uint32_t        widthByte                  = 0;
    uint32_t        copyWidthByte             = 0;
    uint32_t        copyHeightRow             = 0;
    uint32_t        strideInBytes             = widthStride;
    uint32_t        heightStrideInRows       = heightStride;
    size_t          linearAddress              = (size_t)sysMem;
    size_t          linearAddressAligned       = 0;
    unsigned char*  hybridCopyAuxSysMem        = nullptr;

    CmBufferUP             *bufferUP                  = nullptr;
    CmKernel               *kernel                    = nullptr;
    CmBufferUP             *hybridCopyAuxBufferUP     = nullptr;
    SurfaceIndex           *bufferIndexCM             = nullptr;
    SurfaceIndex           *hybridCopyAuxIndexCM      = nullptr;
    SurfaceIndex           *surf2DIndexCM             = nullptr;
    CmThreadSpace          *threadSpace                        = nullptr;
    CmQueue                *cmQueue                   = nullptr;
    CmTask                 *gpuCopyTask               = nullptr;
    CmProgram              *gpuCopyProgram            = nullptr;
    CM_STATUS              status;
    CM_SURFACE_FORMAT      format;

    if ( surface )
    {
        CMCHK_HR( surface->GetSurfaceDesc(width, height, format, sizePerPixel));
    }
    else
    {
        return CM_FAILURE;
    }

    widthByte                  = width * sizePerPixel;
    // the actual copy region
    copyWidthByte             = MOS_MIN(strideInBytes, widthByte);
    copyHeightRow             = MOS_MIN(heightStrideInRows, height);

    if(linearAddress == 0)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.");
        return CM_INVALID_ARG_VALUE;
    }
    if( (copyWidthByte > CM_MAX_THREADSPACE_WIDTH_FOR_MW * BLOCK_WIDTH ) || ( copyHeightRow > CM_MAX_THREADSPACE_HEIGHT_FOR_MW * BLOCK_HEIGHT) )
    {  // each thread handles 64x8 block data. This API will fail if it exceeds the max thread space's size
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_INVALID_ARG_SIZE;
    }

    if (sizeof (void *) == 8 ) //64-bit
    {
        linearAddressAligned        = linearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X64;
    }
    else  //32-bit
    {
        linearAddressAligned        = linearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X86;
    }
    //Calculate  Left Shift offset
    dstAddShiftOffset               = (uint32_t)(linearAddress - linearAddressAligned);

    if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
    {
        bufferupSize = MOS_ALIGN_CEIL(strideInBytes * (heightStrideInRows + copyHeightRow * 1/2) + (uint32_t)dstAddShiftOffset , 64);
    }
    else
    {
        bufferupSize = MOS_ALIGN_CEIL(strideInBytes * heightStrideInRows  + (uint32_t)dstAddShiftOffset, 64);
    }

    CMCHK_HR(m_device->CreateBufferUP(bufferupSize, ( void * )linearAddressAligned, bufferUP));
    CMCHK_HR(bufferUP->GetIndex(bufferIndexCM));
    CMCHK_HR(surface->GetIndex(surf2DIndexCM));

    CMCHK_HR( m_device->LoadPredefinedCopyKernel(gpuCopyProgram));
    CMCHK_NULL(gpuCopyProgram);

    if (direction == CM_FASTCOPY_CPU2GPU)
    {
        if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
        {
            CMCHK_HR(m_device->CreateKernel(gpuCopyProgram, _NAME(surfaceCopy_write_unaligned_NV12), kernel, "PredefinedGPUCopyKernel"));
        }
        else
        {
            CMCHK_HR(m_device->CreateKernel(gpuCopyProgram, _NAME(surfaceCopy_write_unaligned), kernel, "PredefinedGPUCopyKernel"));

        }
        CMCHK_HR(kernel->SetKernelArg( 0, sizeof( SurfaceIndex ), bufferIndexCM ));
        CMCHK_HR(kernel->SetKernelArg( 1, sizeof( SurfaceIndex ), surf2DIndexCM ));
    }
    else
    {
        if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
        {
            CMCHK_HR(m_device->CreateKernel(gpuCopyProgram, _NAME(surfaceCopy_read_unaligned_NV12), kernel, "PredefinedGPUCopyKernel"));
            auxiliaryBufferupSize = BLOCK_WIDTH * 2 * (heightStrideInRows + copyHeightRow * 1/2);
        }
        else
        {
            CMCHK_HR(m_device->CreateKernel(gpuCopyProgram, _NAME(surfaceCopy_read_unaligned), kernel, "PredefinedGPUCopyKernel"));
            auxiliaryBufferupSize = BLOCK_WIDTH * 2 * heightStrideInRows;
        }
        hybridCopyAuxSysMem = (unsigned char*)MOS_AlignedAllocMemory(auxiliaryBufferupSize, PAGE_ALIGNED);
        if(!hybridCopyAuxSysMem)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
        CMCHK_HR(m_device->CreateBufferUP(auxiliaryBufferupSize, (void*)hybridCopyAuxSysMem, hybridCopyAuxBufferUP));
        CMCHK_HR(hybridCopyAuxBufferUP->GetIndex(hybridCopyAuxIndexCM));

        CMCHK_HR(kernel->SetKernelArg( 0, sizeof( SurfaceIndex ), surf2DIndexCM ));
        CMCHK_HR(kernel->SetKernelArg( 1, sizeof( SurfaceIndex ), bufferIndexCM ));
        CMCHK_HR(kernel->SetKernelArg( 5, sizeof( uint32_t ), &widthByte ));
        CMCHK_HR(kernel->SetKernelArg( 6, sizeof( SurfaceIndex ), hybridCopyAuxIndexCM ));
    }

    CMCHK_HR(kernel->SetKernelArg( 2, sizeof( uint32_t ), &strideInBytes ));
    CMCHK_HR(kernel->SetKernelArg( 3, sizeof( uint32_t ), &heightStrideInRows ));
    CMCHK_HR(kernel->SetKernelArg( 4, sizeof( uint32_t ), &dstAddShiftOffset ));

    threadWidth = ( uint32_t )ceil( ( double )copyWidthByte/BLOCK_WIDTH );
    threadHeight = ( uint32_t )ceil( ( double )copyHeightRow/BLOCK_HEIGHT );

    threadNum = threadWidth * threadHeight;
    CMCHK_HR(kernel->SetThreadCount( threadNum ));

    CMCHK_HR(m_device->CreateThreadSpace( threadWidth, threadHeight, threadSpace ));
    CMCHK_HR(m_device->CreateQueue( cmQueue ));
    CMCHK_HR(m_device->CreateTask(gpuCopyTask));
    CMCHK_HR(gpuCopyTask->AddKernel( kernel ));
    CMCHK_HR(cmQueue->Enqueue( gpuCopyTask, event, threadSpace ));

    if(event)
    {
        CMCHK_HR(event->GetStatus(status));
        while(status != CM_STATUS_FINISHED)
        {
            if (status == CM_STATUS_RESET)
            {
                hr = CM_TASK_MEDIA_RESET;
                goto finish;
            }
            CMCHK_HR(event->GetStatus(status));
        }
    }
    // CPU copy unaligned data
    if( direction == CM_FASTCOPY_GPU2CPU)
    {
        uint32_t beginLineCopySize   = 0;
        uint32_t readOffset = 0;
        uint32_t copyLines = 0;
        size_t beginLineWriteOffset = 0;
        uint32_t mod = 0;
        uint32_t alignedWrites = 0;
        uint32_t endLineWriteOffset = 0;
        uint32_t endLineCopySize = 0;
        unsigned char* startBuffer = (unsigned char*)linearAddressAligned;

        copyLines = (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016) ? heightStrideInRows + MOS_MIN(heightStrideInRows, height) * 1 / 2 : heightStrideInRows;

        for(uint32_t i = 0; i < copyLines; ++i)
        {
            //copy begining of line
            beginLineWriteOffset = strideInBytes * i + dstAddShiftOffset;
            mod = ((uintptr_t)startBuffer + beginLineWriteOffset) < BLOCK_WIDTH ? ((uintptr_t)startBuffer + beginLineWriteOffset) : ((uintptr_t)startBuffer + beginLineWriteOffset) & (BLOCK_WIDTH - 1);
            beginLineCopySize = (mod == 0) ? 0:(BLOCK_WIDTH - mod);
            //fix copy size for cases where the surface width is small
            if((beginLineCopySize > widthByte) || ( beginLineCopySize == 0 && widthByte < BLOCK_WIDTH ) )
            {
                beginLineCopySize = widthByte;
            }
            if(beginLineCopySize > 0)
            {
                CmSafeMemCopy((void *)( (unsigned char *)startBuffer + beginLineWriteOffset), (void *)(hybridCopyAuxSysMem + readOffset), beginLineCopySize);
            }

            //copy end of line
            alignedWrites = (widthByte - beginLineCopySize) &~ (BLOCK_WIDTH - 1);
            endLineWriteOffset = beginLineWriteOffset + alignedWrites + beginLineCopySize;
            endLineCopySize = dstAddShiftOffset+ i * strideInBytes + widthByte - endLineWriteOffset;
            if(endLineCopySize > 0 && endLineWriteOffset > beginLineWriteOffset)
            {
                CmSafeMemCopy((void *)((unsigned char *)startBuffer + endLineWriteOffset), (void *)(hybridCopyAuxSysMem + readOffset + BLOCK_WIDTH), endLineCopySize);
            }
            readOffset += (BLOCK_WIDTH * 2);
        }
    }

    CMCHK_HR(m_device->DestroyTask(gpuCopyTask));
    CMCHK_HR(m_device->DestroyThreadSpace(threadSpace));
    CMCHK_HR(m_device->DestroyBufferUP(bufferUP));
    if (direction == CM_FASTCOPY_GPU2CPU)
    {
        if(hybridCopyAuxBufferUP)
        {
            CMCHK_HR(m_device->DestroyBufferUP(hybridCopyAuxBufferUP));
        }
        if(hybridCopyAuxSysMem)
        {
            MOS_AlignedFreeMemory(hybridCopyAuxSysMem);
            hybridCopyAuxSysMem = nullptr;
        }
    }
finish:
    if(hr != CM_SUCCESS)
    {
        if(bufferUP == nullptr)
        {
            // user need to know whether the failure is caused by out of BufferUP.
            hr = CM_GPUCOPY_OUT_OF_RESOURCE;
        }

        if(kernel)                         m_device->DestroyKernel(kernel);
        if(threadSpace)                             m_device->DestroyThreadSpace(threadSpace);
        if(gpuCopyTask)                    m_device->DestroyTask(gpuCopyTask);
        if(bufferUP)                       m_device->DestroyBufferUP(bufferUP);
        if(hybridCopyAuxBufferUP)          m_device->DestroyBufferUP(hybridCopyAuxBufferUP);
        if(hybridCopyAuxSysMem)            {MOS_AlignedFreeMemory(hybridCopyAuxSysMem); hybridCopyAuxSysMem = nullptr;}
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
int32_t CmQueueRT::EnqueueCopyInternal(CmSurface2DRT* surface,
                                unsigned char* sysMem,
                                const uint32_t widthStride,
                                const uint32_t heightStride,
                                CM_GPUCOPY_DIRECTION direction,
                                const uint32_t option,
                                CmEvent* & event)
{
    int32_t hr                  = CM_FAILURE;
    uint32_t width               = 0;
    uint32_t height              = 0;
    uint32_t sizePerPixel        = 0;
    CM_SURFACE_FORMAT format    = CM_SURFACE_FORMAT_INVALID;

    if (surface)
    {
        CMCHK_HR(surface->GetSurfaceDesc(width, height, format, sizePerPixel));
    }
    else
    {
        return CM_GPUCOPY_INVALID_SURFACES;
    }

    if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
    {
        hr = EnqueueCopyInternal_2Planes(surface, (unsigned char*)sysMem, format, width, widthStride, height, heightStride, sizePerPixel, direction, option, event);
    }
    else
    {
        hr = EnqueueCopyInternal_1Plane(surface, (unsigned char*)sysMem, format, width, widthStride, height, heightStride, sizePerPixel, direction, option, event);
    }

finish:
    return hr;
}

int32_t CmQueueRT::EnqueueCopyInternal_1Plane(CmSurface2DRT* surface,
                                    unsigned char* sysMem,
                                    CM_SURFACE_FORMAT format,
                                    const uint32_t widthInPixel,
                                    const uint32_t widthStride,
                                    const uint32_t heightInRow,
                                    const uint32_t heightStride,
                                    const uint32_t sizePerPixel,
                                    CM_GPUCOPY_DIRECTION direction,
                                    const uint32_t option,
                                    CmEvent* & event )
{
    int32_t         hr                      = CM_SUCCESS;
    uint32_t        tempHeight              = heightInRow;
    uint32_t        strideInBytes         = widthStride;
    uint32_t        strideInDwords        = 0;
    uint32_t        heightStrideInRows   = heightStride;
    uint32_t        addedShiftLeftOffset    = 0;
    size_t          linearAddress          = (size_t)sysMem;
    size_t          linearAddressAligned   = 0;

    CmKernel        *kernel            = nullptr;
    CmBufferUP      *cmbufferUP        = nullptr;
    SurfaceIndex    *bufferIndexCM     = nullptr;
    SurfaceIndex    *surf2DIndexCM     = nullptr;
    CmThreadSpace   *threadSpace                = nullptr;
    CmQueue         *cmQueue           = nullptr;
    CmTask          *gpuCopyTask       = nullptr;
    CmEvent         *internalEvent     = nullptr;

    uint32_t        threadWidth             = 0;
    uint32_t        threadHeight            = 0;
    uint32_t        threadNum               = 0;
    uint32_t        widthDword             = 0;
    uint32_t        widthByte              = 0;
    uint32_t        copyWidthByte         = 0;
    uint32_t        copyHeightRow         = 0;
    uint32_t        sliceCopyHeightRow   = 0;
    uint32_t        sliceCopyBufferUPSize   = 0;
    int32_t         totalBufferUPSize       = 0;
    uint32_t        startX                 = 0;
    uint32_t        startY                 = 0;
    bool            blSingleEnqueue         = true;
    CM_GPUCOPY_KERNEL *gpuCopyKernelParam     = nullptr;

    PCM_HAL_STATE   cmHalState    =        \
        ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;

    widthByte    = widthInPixel * sizePerPixel;

    //Align the width regarding stride
   if(strideInBytes == 0)
   {
        strideInBytes = widthByte;
   }

   if(heightStrideInRows == 0)
   {
        heightStrideInRows = heightInRow;
   }

    // the actual copy region
    copyWidthByte = MOS_MIN(strideInBytes, widthByte);
    copyHeightRow = MOS_MIN(heightStrideInRows, heightInRow);

    // Make sure stride and start address of system memory is 16-byte aligned.
    // if no padding in system memory , strideInBytes = widthByte.
    if(strideInBytes & 0xf)
    {
        CM_ASSERTMESSAGE("Error: Stride is not 16-byte aligned.");
        return CM_GPUCOPY_INVALID_STRIDE;
    }
    if((linearAddress & 0xf) || (linearAddress == 0))
    {
        CM_ASSERTMESSAGE("Error: Start address of system memory is not 16-byte aligned.");
        return CM_GPUCOPY_INVALID_SYSMEM;
    }

    //Calculate actual total size of system memory
    totalBufferUPSize = strideInBytes * heightStrideInRows;

    //Check thread space width here
    if( copyWidthByte > CM_MAX_THREADSPACE_WIDTH_FOR_MW * BLOCK_PIXEL_WIDTH *4 )
    {  // each thread handles 128x8 block data. This API will fail if it exceeds the max thread space's size
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_GPUCOPY_INVALID_SIZE;
    }

    while (totalBufferUPSize > 0)
    {
        if (sizeof (void *) == 8 ) //64-bit
        {
            linearAddressAligned        = linearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X64;
        }
        else  //32-bit
        {
            linearAddressAligned        = linearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X86;
        }

        //Calculate  Left Shift offset
        addedShiftLeftOffset = (uint32_t)(linearAddress - linearAddressAligned);
        totalBufferUPSize   += addedShiftLeftOffset;

        if (totalBufferUPSize > CM_MAX_1D_SURF_WIDTH)
        {
            blSingleEnqueue = false;
            sliceCopyHeightRow = ((CM_MAX_1D_SURF_WIDTH - addedShiftLeftOffset)/(strideInBytes*(BLOCK_HEIGHT * INNER_LOOP))) * (BLOCK_HEIGHT * INNER_LOOP);
            sliceCopyBufferUPSize = sliceCopyHeightRow * strideInBytes + addedShiftLeftOffset;
            tempHeight = sliceCopyHeightRow;
        }
        else
        {
            sliceCopyHeightRow = copyHeightRow;
            sliceCopyBufferUPSize = totalBufferUPSize;
            if (!blSingleEnqueue)
            {
                tempHeight = sliceCopyHeightRow;
            }
        }

        //Check thread space height here
        if(sliceCopyHeightRow > CM_MAX_THREADSPACE_HEIGHT_FOR_MW * BLOCK_HEIGHT * INNER_LOOP )
        {  // each thread handles 128x8 block data. This API will fail if it exceeds the max thread space's size
            CM_ASSERTMESSAGE("Error: Invalid copy size.");
            return CM_GPUCOPY_INVALID_SIZE;
        }

        kernel = nullptr;
        CMCHK_HR( m_device->CreateBufferUP(  sliceCopyBufferUPSize, ( void * )linearAddressAligned, cmbufferUP ));
        //Configure memory object control for BufferUP to solve the cache-line issue.
        if (cmHalState->cmHalInterface->IsGPUCopySurfaceNoCacheWARequired())
        {
            CMCHK_HR(cmbufferUP->SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3));
        }
        CMCHK_HR(CreateGPUCopyKernel(copyWidthByte, sliceCopyHeightRow, format, direction, gpuCopyKernelParam));
        CMCHK_NULL(gpuCopyKernelParam);
        kernel = gpuCopyKernelParam->kernel;

        CMCHK_NULL(kernel);

        CMCHK_NULL(cmbufferUP);
        CMCHK_HR(cmbufferUP->GetIndex( bufferIndexCM ));
        CMCHK_HR(surface->GetIndex( surf2DIndexCM ));

        threadWidth = ( uint32_t )ceil( ( double )copyWidthByte/BLOCK_PIXEL_WIDTH/4 );
        threadHeight = ( uint32_t )ceil( ( double )sliceCopyHeightRow/BLOCK_HEIGHT/INNER_LOOP );
        threadNum = threadWidth * threadHeight;
        CMCHK_HR(kernel->SetThreadCount( threadNum ));
        CMCHK_HR(m_device->CreateThreadSpace( threadWidth, threadHeight, threadSpace ));

        if( direction == CM_FASTCOPY_CPU2GPU)
        {
            if (cmHalState->cmHalInterface->IsSurfaceCompressionWARequired())
            {
                CMCHK_HR(surface->SetCompressionMode(MEMCOMP_DISABLED));
            }
            CMCHK_HR(kernel->SetKernelArg( 0, sizeof( SurfaceIndex ), bufferIndexCM) );
            CMCHK_HR(kernel->SetKernelArg( 1, sizeof( SurfaceIndex ), surf2DIndexCM ));
        }
        else
        {
            CMCHK_HR(kernel->SetKernelArg( 1, sizeof( SurfaceIndex ), bufferIndexCM ));
            CMCHK_HR(kernel->SetKernelArg( 0, sizeof( SurfaceIndex ), surf2DIndexCM ));
        }

        if(direction == CM_FASTCOPY_GPU2CPU)
        {
            surface->SetReadSyncFlag(true); // GPU -> CPU, set surf2d as read sync flag
        }

        widthDword = (uint32_t)ceil((double)widthByte / 4);
        strideInDwords = (uint32_t)ceil((double)strideInBytes / 4);

        CMCHK_HR(kernel->SetKernelArg( 2, sizeof( uint32_t ), &strideInDwords ));
        CMCHK_HR(kernel->SetKernelArg( 3, sizeof( uint32_t ), &heightStrideInRows ));
        CMCHK_HR(kernel->SetKernelArg( 4, sizeof( uint32_t ), &addedShiftLeftOffset ));
        CMCHK_HR(kernel->SetKernelArg( 5, sizeof( uint32_t ), &threadHeight ));

        if (direction == CM_FASTCOPY_GPU2CPU)  //GPU-->CPU, read
        {
            CMCHK_HR(kernel->SetKernelArg( 6, sizeof( uint32_t ), &widthDword ));
            CMCHK_HR(kernel->SetKernelArg( 7, sizeof( uint32_t ), &tempHeight ));
            CMCHK_HR(kernel->SetKernelArg( 8, sizeof(uint32_t), &startX));
            CMCHK_HR(kernel->SetKernelArg( 9, sizeof(uint32_t), &startY));
        }
        else  //CPU-->GPU, write
        {
            //this only works for the kernel surfaceCopy_write_32x32
            CMCHK_HR(kernel->SetKernelArg( 6, sizeof( uint32_t ), &startX ));
            CMCHK_HR(kernel->SetKernelArg( 7, sizeof( uint32_t ), &startY ));
        }

        CMCHK_HR(m_device->CreateQueue( cmQueue ));
        CMCHK_HR(m_device->CreateTask(gpuCopyTask));
        CMCHK_HR(gpuCopyTask->AddKernel( kernel ));
        if (option & CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST)
        {
            // disable turbo
            CM_TASK_CONFIG taskConfig;
            CmSafeMemSet(&taskConfig, 0, sizeof(CM_TASK_CONFIG));
            taskConfig.turboBoostFlag = CM_TURBO_BOOST_DISABLE;
            gpuCopyTask->SetProperty(taskConfig);
        }
        CMCHK_HR(cmQueue->Enqueue( gpuCopyTask, internalEvent, threadSpace ));

        if( gpuCopyKernelParam )
        {
            GPUCOPY_KERNEL_UNLOCK(gpuCopyKernelParam);
        }

        //update for next slice
        linearAddress += sliceCopyBufferUPSize - addedShiftLeftOffset;
        totalBufferUPSize -= sliceCopyBufferUPSize;
        copyHeightRow -= sliceCopyHeightRow;
        startX = 0;
        startY += sliceCopyHeightRow;

        if(totalBufferUPSize > 0)   //Intermediate event, we don't need it
        {
            CMCHK_HR(cmQueue->DestroyEvent(internalEvent));
        }
        else //Last one event, need keep or destroy it
        {
            if ((option & CM_FASTCOPY_OPTION_BLOCKING) && (internalEvent))
            {
                CMCHK_HR(internalEvent->WaitForTaskFinished());
            }

            if(event == CM_NO_EVENT)  //User doesn't need CmEvent for this copy
            {
                event = nullptr;
                CMCHK_HR(cmQueue->DestroyEvent(internalEvent));
            }
            else //User needs this CmEvent
            {
                event = internalEvent;
            }
        }

        CMCHK_HR(m_device->DestroyTask(gpuCopyTask));
        CMCHK_HR(m_device->DestroyThreadSpace(threadSpace));
        CMCHK_HR(m_device->DestroyBufferUP(cmbufferUP));
    }

finish:

    if(hr != CM_SUCCESS)
    {
        if(cmbufferUP == nullptr)
        {
            // user need to know whether the failure is caused by out of BufferUP.
            hr = CM_GPUCOPY_OUT_OF_RESOURCE;
        }

        if(kernel && gpuCopyKernelParam)        GPUCOPY_KERNEL_UNLOCK(gpuCopyKernelParam);
        if(threadSpace)                                m_device->DestroyThreadSpace(threadSpace);
        if(gpuCopyTask)                       m_device->DestroyTask(gpuCopyTask);
        if(cmbufferUP)                        m_device->DestroyBufferUP(cmbufferUP);
        if(internalEvent)                     cmQueue->DestroyEvent(internalEvent);

        // CM_FAILURE for all the other errors
        // return CM_EXCEED_MAX_TIMEOUT to notify app that gpu reset happens
        if( hr != CM_GPUCOPY_OUT_OF_RESOURCE && hr != CM_EXCEED_MAX_TIMEOUT)
        {
            hr = CM_FAILURE;
        }
    }

    return hr;
}

int32_t CmQueueRT::EnqueueCopyInternal_2Planes(CmSurface2DRT* surface,
                                        unsigned char* sysMem,
                                        CM_SURFACE_FORMAT format,
                                        const uint32_t widthInPixel,
                                        const uint32_t widthStride,
                                        const uint32_t heightInRow,
                                        const uint32_t heightStride,
                                        const uint32_t sizePerPixel,
                                        CM_GPUCOPY_DIRECTION direction,
                                        const uint32_t option,
                                        CmEvent* & event)
{
    int32_t         hr                      = CM_SUCCESS;
    uint32_t        strideInBytes         = widthStride;
    uint32_t        strideInDwords        = 0;
    uint32_t        heightStrideInRows   = heightStride;
    size_t          linearAddressY        = 0;
    size_t          linearAddressUV       = 0;
    size_t          linearAddressAlignedY = 0;
    size_t          linearAddressAlignedUV = 0;
    uint32_t        addedShiftLeftOffsetY  = 0;
    uint32_t        addedShiftLeftOffsetUV = 0;

    CmKernel        *kernel                = nullptr;
    CmBufferUP      *cmbufferUPY          = nullptr;
    CmBufferUP      *cmbufferUPUV         = nullptr;
    SurfaceIndex    *bufferUPIndexY       = nullptr;
    SurfaceIndex    *bufferUPIndexUV      = nullptr;
    SurfaceIndex    *surf2DIndexCM         = nullptr;
    CmThreadSpace   *threadSpace                    = nullptr;
    CmQueue         *cmQueue               = nullptr;
    CmTask          *gpuCopyTask           = nullptr;
    CmEvent         *internalEvent         = nullptr;

    uint32_t        threadWidth             = 0;
    uint32_t        threadHeight            = 0;
    uint32_t        threadNum               = 0;
    uint32_t        widthDword             = 0;
    uint32_t        widthByte              = 0;
    uint32_t        copyWidthByte         = 0;
    uint32_t        copyHeightRow         = 0;
    uint32_t        bufferUPYSize         = 0;
    uint32_t        bufferUPUVSize        = 0;

    CM_GPUCOPY_KERNEL *gpuCopyKernelParam = nullptr;
    PCM_HAL_STATE       cmHalState    =      \
        ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;

    widthByte = widthInPixel * sizePerPixel;

    //Align the width regarding stride
    if (strideInBytes == 0)
    {
        strideInBytes = widthByte;
    }

    if (heightStrideInRows == 0)
    {
        heightStrideInRows = heightInRow;
    }

    // the actual copy region
    copyWidthByte = MOS_MIN(strideInBytes, widthByte);
    copyHeightRow = MOS_MIN(heightStrideInRows, heightInRow);

    // Make sure stride and start address of system memory is 16-byte aligned.
    // if no padding in system memory , strideInBytes = widthByte.
    if (strideInBytes & 0xf)
    {
        CM_ASSERTMESSAGE("Error: Stride is not 16-byte aligned.");
        return CM_GPUCOPY_INVALID_STRIDE;
    }

    //Check thread space width here
    if (copyWidthByte > CM_MAX_THREADSPACE_WIDTH_FOR_MW * BLOCK_PIXEL_WIDTH * 4)
    {  // each thread handles 128x8 block data. This API will fail if it exceeds the max thread space's size
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_GPUCOPY_INVALID_SIZE;
    }

    linearAddressY = (size_t)sysMem;
    linearAddressUV = (size_t)((char*)sysMem + strideInBytes * heightStrideInRows);

    if ((linearAddressY & 0xf) || (linearAddressY == 0) || (linearAddressAlignedUV & 0xf))
    {
        CM_ASSERTMESSAGE("Error: Start address of system memory is not 16-byte aligned.");
        return CM_GPUCOPY_INVALID_SYSMEM;
    }

    if (sizeof (void *) == 8) //64-bit
    {
        linearAddressAlignedY = linearAddressY & ADDRESS_PAGE_ALIGNMENT_MASK_X64;
        linearAddressAlignedUV = linearAddressUV & ADDRESS_PAGE_ALIGNMENT_MASK_X64;
    }
    else  //32-bit
    {
        linearAddressAlignedY = linearAddressY & ADDRESS_PAGE_ALIGNMENT_MASK_X86;
        linearAddressAlignedUV = linearAddressUV & ADDRESS_PAGE_ALIGNMENT_MASK_X86;
    }

    //Calculate  Left Shift offset
    addedShiftLeftOffsetY = (uint32_t)(linearAddressY - linearAddressAlignedY);
    addedShiftLeftOffsetUV = (uint32_t)(linearAddressUV - linearAddressAlignedUV);

    //Calculate actual total size of system memory, assume it's NV12/P010/P016 formats
    bufferUPYSize = strideInBytes * heightStrideInRows + addedShiftLeftOffsetY;
    bufferUPUVSize = strideInBytes * copyHeightRow * 1 / 2 + addedShiftLeftOffsetUV;

    //Check thread space height here
    if (copyHeightRow > CM_MAX_THREADSPACE_HEIGHT_FOR_MW * BLOCK_HEIGHT * INNER_LOOP)
    {  // each thread handles 128x8 block data. This API will fail if it exceeds the max thread space's size
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_GPUCOPY_INVALID_SIZE;
    }

    kernel = nullptr;
    CMCHK_HR(m_device->CreateBufferUP(bufferUPYSize, (void *)linearAddressAlignedY, cmbufferUPY));
    CMCHK_HR(m_device->CreateBufferUP(bufferUPUVSize, (void *)linearAddressAlignedUV, cmbufferUPUV));

    //Configure memory object control for the two BufferUP to solve the same cache-line coherency issue.
    if (cmHalState->cmHalInterface->IsGPUCopySurfaceNoCacheWARequired())
    {
        CMCHK_HR(cmbufferUPY->SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3));
        CMCHK_HR(cmbufferUPUV->SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3));
    }
    else
    {
        CMCHK_HR(static_cast< CmBuffer_RT* >(cmbufferUPY)->SetMemoryObjectControl(MEMORY_OBJECT_CONTROL_FROM_GTT_ENTRY, CM_WRITE_THROUGH, 0));
        CMCHK_HR(static_cast< CmBuffer_RT* >(cmbufferUPUV)->SetMemoryObjectControl(MEMORY_OBJECT_CONTROL_FROM_GTT_ENTRY, CM_WRITE_THROUGH, 0));
    }

    CMCHK_HR(CreateGPUCopyKernel(copyWidthByte, copyHeightRow, format, direction, gpuCopyKernelParam));
    CMCHK_NULL(gpuCopyKernelParam);
    kernel = gpuCopyKernelParam->kernel;

    CMCHK_NULL(kernel);

    CMCHK_NULL(cmbufferUPY);
    CMCHK_NULL(cmbufferUPUV);
    CMCHK_HR(cmbufferUPY->GetIndex(bufferUPIndexY));
    CMCHK_HR(cmbufferUPUV->GetIndex(bufferUPIndexUV));
    CMCHK_HR(surface->GetIndex(surf2DIndexCM));

    threadWidth = (uint32_t)ceil((double)copyWidthByte / BLOCK_PIXEL_WIDTH / 4);
    threadHeight = (uint32_t)ceil((double)copyHeightRow / BLOCK_HEIGHT / INNER_LOOP);
    threadNum = threadWidth * threadHeight;
    CMCHK_HR(kernel->SetThreadCount(threadNum));
    CMCHK_HR(m_device->CreateThreadSpace(threadWidth, threadHeight, threadSpace));

    widthDword = (uint32_t)ceil((double)widthByte / 4);
    strideInDwords = (uint32_t)ceil((double)strideInBytes / 4);

    if (direction == CM_FASTCOPY_CPU2GPU) //Write
    {
        //Input BufferUP_Y and BufferUP_UV
        if (cmHalState->cmHalInterface->IsSurfaceCompressionWARequired())
        {
            CMCHK_HR(surface->SetCompressionMode(MEMCOMP_DISABLED));
        }
        CMCHK_HR(kernel->SetKernelArg(0, sizeof(SurfaceIndex), bufferUPIndexY));
        CMCHK_HR(kernel->SetKernelArg(1, sizeof(SurfaceIndex), bufferUPIndexUV));
        //Output Surface2D
        CMCHK_HR(kernel->SetKernelArg(2, sizeof(SurfaceIndex), surf2DIndexCM));
        //Other parameters
        CMCHK_HR(kernel->SetKernelArg(3, sizeof(uint32_t), &strideInDwords));
        CMCHK_HR(kernel->SetKernelArg(4, sizeof(uint32_t), &heightStrideInRows));
        CMCHK_HR(kernel->SetKernelArg(5, sizeof(uint32_t), &addedShiftLeftOffsetY));
        CMCHK_HR(kernel->SetKernelArg(6, sizeof(uint32_t), &addedShiftLeftOffsetUV));
        CMCHK_HR(kernel->SetKernelArg(7, sizeof(uint32_t), &threadHeight));
    }
    else  //Read
    {
        //Input Surface2D
        CMCHK_HR(kernel->SetKernelArg(0, sizeof(SurfaceIndex), surf2DIndexCM));
        //Output BufferUP_Y and BufferUP_UV
        CMCHK_HR(kernel->SetKernelArg(1, sizeof(SurfaceIndex), bufferUPIndexY));
        CMCHK_HR(kernel->SetKernelArg(2, sizeof(SurfaceIndex), bufferUPIndexUV));
        //Other parameters
        CMCHK_HR(kernel->SetKernelArg(3, sizeof(uint32_t), &strideInDwords));
        CMCHK_HR(kernel->SetKernelArg(4, sizeof(uint32_t), &heightStrideInRows));
        CMCHK_HR(kernel->SetKernelArg(5, sizeof(uint32_t), &addedShiftLeftOffsetY));
        CMCHK_HR(kernel->SetKernelArg(6, sizeof(uint32_t), &addedShiftLeftOffsetUV));
        CMCHK_HR(kernel->SetKernelArg(7, sizeof(uint32_t), &threadHeight));
        CMCHK_HR(kernel->SetKernelArg(8, sizeof(uint32_t), &widthDword));
        CMCHK_HR(kernel->SetKernelArg(9, sizeof(uint32_t), &heightInRow));

        surface->SetReadSyncFlag(true); // GPU -> CPU, set surf2d as read sync flag
    }

    CMCHK_HR(m_device->CreateQueue(cmQueue));
    CMCHK_HR(m_device->CreateTask(gpuCopyTask));
    CMCHK_HR(gpuCopyTask->AddKernel(kernel));
    if (option & CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST)
    {
        // disable turbo
        CM_TASK_CONFIG taskConfig;
        CmSafeMemSet(&taskConfig, 0, sizeof(CM_TASK_CONFIG));
        taskConfig.turboBoostFlag = CM_TURBO_BOOST_DISABLE;
        gpuCopyTask->SetProperty(taskConfig);
    }
    CMCHK_HR(cmQueue->Enqueue(gpuCopyTask, internalEvent, threadSpace));

    if (gpuCopyKernelParam)
    {
        GPUCOPY_KERNEL_UNLOCK(gpuCopyKernelParam);
    }

    if ((option & CM_FASTCOPY_OPTION_BLOCKING) && (internalEvent))
    {
        CMCHK_HR(internalEvent->WaitForTaskFinished());
    }

    if (event == CM_NO_EVENT)  //User doesn't need CmEvent for this copy
    {
        event = nullptr;
        CMCHK_HR(cmQueue->DestroyEvent(internalEvent));
    }
    else //User needs this CmEvent
    {
        event = internalEvent;
    }

    CMCHK_HR(m_device->DestroyTask(gpuCopyTask));
    CMCHK_HR(m_device->DestroyThreadSpace(threadSpace));
    CMCHK_HR(m_device->DestroyBufferUP(cmbufferUPY));
    CMCHK_HR(m_device->DestroyBufferUP(cmbufferUPUV));

finish:

    if (hr != CM_SUCCESS)
    {
        if ((cmbufferUPY == nullptr) || (cmbufferUPUV == nullptr))
        {
            // user need to know whether the failure is caused by out of BufferUP.
            hr = CM_GPUCOPY_OUT_OF_RESOURCE;
        }

        if (kernel && gpuCopyKernelParam)        GPUCOPY_KERNEL_UNLOCK(gpuCopyKernelParam);
        if (threadSpace)                                m_device->DestroyThreadSpace(threadSpace);
        if (gpuCopyTask)                       m_device->DestroyTask(gpuCopyTask);
        if (cmbufferUPY)                      m_device->DestroyBufferUP(cmbufferUPY);
        if (cmbufferUPUV)                     m_device->DestroyBufferUP(cmbufferUPUV);
        if (internalEvent)                     cmQueue->DestroyEvent(internalEvent);

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
CM_RT_API int32_t CmQueueRT::EnqueueCopyGPUToGPU( CmSurface2D* outputSurface, CmSurface2D* inputSurface, uint32_t option, CmEvent* & event )
{
    INSERT_API_CALL_LOG();

    uint32_t srcSurfaceWidth = 0;
    uint32_t srcSurfaceHeight = 0;
    uint32_t dstSurfaceWidth = 0;
    uint32_t dstSurfaceHeight = 0;

    CM_SURFACE_FORMAT srcSurfaceFormat = CM_SURFACE_FORMAT_INVALID;
    CM_SURFACE_FORMAT dstSurfaceFormat = CM_SURFACE_FORMAT_INVALID;

    int32_t             hr = CM_SUCCESS;
    uint32_t            srcSizePerPixel = 0;
    uint32_t            dstSizePerPixel = 0;
    uint32_t            threadWidth = 0;
    uint32_t            threadHeight = 0;

    CmKernel            *kernel = nullptr;
    SurfaceIndex        *surfaceInputIndex = nullptr;
    SurfaceIndex        *surfaceOutputIndex = nullptr;
    CmThreadSpace       *threadSpace = nullptr;
    CmTask              *task = nullptr;
    CmQueue             *cmQueue = nullptr;
    uint32_t            srcSurfAlignedWidthInBytes = 0;
    CM_GPUCOPY_KERNEL *gpuCopyKernelParam = nullptr;

    if ((outputSurface == nullptr) || (inputSurface == nullptr))
    {
        CM_ASSERTMESSAGE("Error: Pointer to input surface or output surface is null.");
        return CM_FAILURE;
    }

    PCM_HAL_STATE   cmHalState = ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;
    CmSurface2DRT *outputSurfaceRT = static_cast<CmSurface2DRT *>(outputSurface);
    CmSurface2DRT *inputSurfaceRT = static_cast<CmSurface2DRT *>(inputSurface);
    if (cmHalState->cmHalInterface->IsSurfaceCompressionWARequired())
    {
        CMCHK_HR(outputSurfaceRT->SetCompressionMode(MEMCOMP_DISABLED));
    }

    CMCHK_HR(outputSurfaceRT->GetSurfaceDesc(dstSurfaceWidth, dstSurfaceHeight, dstSurfaceFormat, dstSizePerPixel));
    CMCHK_HR(inputSurfaceRT->GetSurfaceDesc(srcSurfaceWidth, srcSurfaceHeight, srcSurfaceFormat, srcSizePerPixel));

    if ((dstSurfaceWidth != srcSurfaceWidth) ||
        (dstSurfaceHeight < srcSurfaceHeight) ||  //relax the restriction
        (dstSizePerPixel != srcSizePerPixel))
    {
        CM_ASSERTMESSAGE("Error: Size of dest surface does not match src surface.");
        return CM_GPUCOPY_INVALID_SURFACES;
    }

    //To support copy b/w Format_A8R8G8B8 and Format_A8B8G8R8
    if (dstSurfaceFormat != srcSurfaceFormat)
    {
        if (!((dstSurfaceFormat == CM_SURFACE_FORMAT_A8R8G8B8) && (srcSurfaceFormat == CM_SURFACE_FORMAT_A8B8G8R8)) &&
            !((dstSurfaceFormat == CM_SURFACE_FORMAT_A8R8G8B8) && (srcSurfaceFormat == CM_SURFACE_FORMAT_A8B8G8R8)))
        {
            CM_ASSERTMESSAGE("Error: Only support copy b/w Format_A8R8G8B8 and Format_A8B8G8R8 if src format is not matched with dst format.");
            return CM_GPUCOPY_INVALID_SURFACES;
        }
    }

    // 128Bytes aligned
    srcSurfAlignedWidthInBytes = (uint32_t)(ceil((double)srcSurfaceWidth*srcSizePerPixel / BLOCK_PIXEL_WIDTH / 4) * (BLOCK_PIXEL_WIDTH * 4));

    if (srcSurfaceHeight > CM_MAX_THREADSPACE_WIDTH_FOR_MW *BLOCK_HEIGHT *INNER_LOOP)
    {
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_GPUCOPY_INVALID_SIZE;
    }

    CMCHK_HR(CreateGPUCopyKernel(srcSurfaceWidth*srcSizePerPixel, srcSurfaceHeight, srcSurfaceFormat, CM_FASTCOPY_GPU2GPU, gpuCopyKernelParam));
    CMCHK_NULL(gpuCopyKernelParam);

    CMCHK_NULL(gpuCopyKernelParam->kernel);
    kernel = gpuCopyKernelParam->kernel;

    CMCHK_HR(inputSurface->GetIndex(surfaceInputIndex));
    CMCHK_HR(outputSurface->GetIndex(surfaceOutputIndex));

    threadWidth = srcSurfAlignedWidthInBytes / (BLOCK_PIXEL_WIDTH * 4);
    threadHeight = (uint32_t)ceil((double)srcSurfaceHeight / BLOCK_HEIGHT / INNER_LOOP);

    CMCHK_HR(kernel->SetThreadCount(threadWidth * threadHeight));

    CMCHK_HR(kernel->SetKernelArg(0, sizeof(SurfaceIndex), surfaceInputIndex));
    CMCHK_HR(kernel->SetKernelArg(1, sizeof(SurfaceIndex), surfaceOutputIndex));
    CMCHK_HR(kernel->SetKernelArg(2, sizeof(uint32_t), &threadHeight));

    CMCHK_HR(m_device->CreateThreadSpace(threadWidth, threadHeight, threadSpace));

    CMCHK_HR(m_device->CreateTask(task));
    CMCHK_NULL(task);
    CMCHK_HR(task->AddKernel(kernel));

    if (option & CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST)
    {
        // disable turbo
        CM_TASK_CONFIG taskConfig;
        CmSafeMemSet(&taskConfig, 0, sizeof(CM_TASK_CONFIG));
        taskConfig.turboBoostFlag = CM_TURBO_BOOST_DISABLE;
        task->SetProperty(taskConfig);
    }

    CMCHK_HR(m_device->CreateQueue(cmQueue));
    CMCHK_HR(cmQueue->Enqueue(task, event, threadSpace));
    if ((option & CM_FASTCOPY_OPTION_BLOCKING) && (event))
    {
        CMCHK_HR(event->WaitForTaskFinished());
    }

finish:

    if (kernel && gpuCopyKernelParam)        GPUCOPY_KERNEL_UNLOCK(gpuCopyKernelParam);
    if (threadSpace)                                m_device->DestroyThreadSpace(threadSpace);
    if (task)                              m_device->DestroyTask(task);

    return hr;
}

//*-----------------------------------------------------------------------------
//! Enqueue an task, which contains one pre-defined kernel to copy from system memory to system memory
//! This is a non-blocking call. i.e. it returns immediately without waiting for
//! GPU to finish the execution of the task.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can be used to check if the task finishs.
//! If the size is less than 1KB,  CPU is used to do the copy and event will be set as nullptr .
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
//!     CM_GPUCOPY_INVALID_SYSMEM if the sysMem is not 16-byte aligned or is NULL.
//!     CM_GPUCOPY_OUT_OF_RESOURCE if runtime run out of BufferUP.
//!     CM_GPUCOPY_INVALID_SIZE  if its size plus shift-left offset large than CM_MAX_1D_SURF_WIDTH.
//! Restrictions:
//!     1) dstSysMem and srcSysMem should be 16-byte aligned.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueCopyCPUToCPU( unsigned char* dstSysMem, unsigned char* srcSysMem, uint32_t size, uint32_t option, CmEvent* & event )
{
    INSERT_API_CALL_LOG();

    int hr = CM_SUCCESS;
    size_t inputLinearAddress  = (size_t )srcSysMem;
    size_t outputLinearAddress = (size_t )dstSysMem;

    size_t inputLinearAddressAligned = 0;
    size_t outputLinearAddressAligned = 0;

    CmBufferUP      *surfaceInput          = nullptr;
    CmBufferUP      *surfaceOutput         = nullptr;
    CmKernel        *kernel                = nullptr;
    SurfaceIndex    *surfaceInputIndex     = nullptr;
    SurfaceIndex    *surfaceOutputIndex    = nullptr;
    CmThreadSpace   *threadSpace                    = nullptr;
    CmTask          *task                  = nullptr;
    CmQueue         *cmQueue               = nullptr;

    int32_t         srcLeftShiftOffset      = 0;
    int32_t         dstLeftShiftOffset      = 0;
    uint32_t        threadWidth             = 0;
    uint32_t        threadHeight            = 0;
    uint32_t        threadNum              = 0;
    uint32_t        gpuMemcopySize        = 0;
    uint32_t        cpuMemcopySize        = 0;
    CM_GPUCOPY_KERNEL *gpuCopyKernelParam     = nullptr;

    if((inputLinearAddress & 0xf) || (outputLinearAddress & 0xf) ||
        (inputLinearAddress == 0) || (outputLinearAddress == 0))
    {
        CM_ASSERTMESSAGE("Error: Start address of system memory is not 16-byte aligned.");
        return CM_GPUCOPY_INVALID_SYSMEM;
    }

    // Get page aligned address
    if (sizeof (void *) == 8 ) //64-bit
    {
        inputLinearAddressAligned  = inputLinearAddress  & ADDRESS_PAGE_ALIGNMENT_MASK_X64;  // make sure the address page aligned.
        outputLinearAddressAligned = outputLinearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X64;  // make sure the address page aligned.
    }
    else
    {
        inputLinearAddressAligned  = inputLinearAddress  & ADDRESS_PAGE_ALIGNMENT_MASK_X86;  // make sure the address page aligned.
        outputLinearAddressAligned = outputLinearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X86;  // make sure the address page aligned.
    }

    srcLeftShiftOffset = (int32_t)(inputLinearAddress  - inputLinearAddressAligned) ;
    dstLeftShiftOffset = (int32_t)(outputLinearAddress - outputLinearAddressAligned) ;

    if(((size + srcLeftShiftOffset) > CM_MAX_1D_SURF_WIDTH)||
       ((size + dstLeftShiftOffset) > CM_MAX_1D_SURF_WIDTH))
    {
        CM_ASSERTMESSAGE("Error: Invalid copy size.");
        return CM_GPUCOPY_INVALID_SIZE;
    }

    threadWidth  = 0;
    threadHeight = 0;
    threadNum = size / BYTE_COPY_ONE_THREAD; // each thread copys 32 x 4 x32 bytes = 1K

    if( threadNum == 0)
    {
        //if the size of data is less than data copied per thread ( 4K), use CPU to copy it instead of GPU.
        CmFastMemCopy((void *)(outputLinearAddress),
                      (void *)(inputLinearAddress),
                      size); //SSE copy used in CMRT.

        event = nullptr;
        return CM_SUCCESS;
    }

    //Calculate proper thread space's width and height
    threadWidth  = 1;
    threadHeight = threadNum/threadWidth;
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
            threadHeight = threadNum/threadWidth;
        }
        else
        {
            threadWidth +=  THREAD_SPACE_WIDTH_INCREMENT; // increase 8 per iteration
            threadHeight = threadNum/threadWidth;
        }
    }

    CMCHK_HR(m_device->CreateBufferUP(size + srcLeftShiftOffset, (void *)inputLinearAddressAligned,surfaceInput));

    CMCHK_HR(m_device->CreateBufferUP(size + dstLeftShiftOffset, (void *)outputLinearAddressAligned,surfaceOutput));

    CMCHK_HR(CreateGPUCopyKernel(size, 0, CM_SURFACE_FORMAT_INVALID, CM_FASTCOPY_CPU2CPU, gpuCopyKernelParam));
    CMCHK_NULL(gpuCopyKernelParam);
    CMCHK_NULL(gpuCopyKernelParam->kernel);
    kernel = gpuCopyKernelParam->kernel;

    CMCHK_NULL(surfaceInput);
    CMCHK_HR(surfaceInput->GetIndex(surfaceInputIndex));
    CMCHK_NULL(surfaceOutput);
    CMCHK_HR(surfaceOutput->GetIndex(surfaceOutputIndex));

    CMCHK_HR(kernel->SetThreadCount(threadWidth * threadHeight));
    CMCHK_HR(kernel->SetKernelArg( 0, sizeof( SurfaceIndex ), surfaceInputIndex ));
    CMCHK_HR(kernel->SetKernelArg( 1, sizeof( SurfaceIndex ), surfaceOutputIndex ));
    CMCHK_HR(kernel->SetKernelArg( 2, sizeof( int ), &threadWidth ));
    CMCHK_HR(kernel->SetKernelArg( 3, sizeof( int ), &threadHeight ));
    CMCHK_HR(kernel->SetKernelArg( 4, sizeof( int ), &srcLeftShiftOffset ));
    CMCHK_HR(kernel->SetKernelArg( 5, sizeof( int ), &dstLeftShiftOffset ));
    CMCHK_HR(kernel->SetKernelArg( 6, sizeof( int ), &size ));

    CMCHK_HR(m_device->CreateThreadSpace(threadWidth, threadHeight, threadSpace));

    CMCHK_HR(m_device->CreateTask(task));
    CMCHK_NULL(task);
    CMCHK_HR(task->AddKernel (kernel));

    if (option & CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST)
    {
        // disable turbo
        CM_TASK_CONFIG taskConfig;
        CmSafeMemSet(&taskConfig, 0, sizeof(CM_TASK_CONFIG));
        taskConfig.turboBoostFlag = CM_TURBO_BOOST_DISABLE;
        task->SetProperty(taskConfig);
    }

    CMCHK_HR(m_device->CreateQueue( cmQueue));
    CMCHK_HR(cmQueue->Enqueue(task, event, threadSpace));

    if ((option & CM_FASTCOPY_OPTION_BLOCKING) && (event))
    {
        CMCHK_HR(event->WaitForTaskFinished());
    }

    //Copy the unaligned part by using CPU
    gpuMemcopySize = threadHeight * threadWidth *BYTE_COPY_ONE_THREAD;
    cpuMemcopySize = size - threadHeight * threadWidth *BYTE_COPY_ONE_THREAD;

    CmFastMemCopy((void *)(outputLinearAddress+gpuMemcopySize),
                  (void *)(inputLinearAddress+gpuMemcopySize),
                          cpuMemcopySize); //SSE copy used in CMRT.

    CMCHK_HR(m_device->DestroyThreadSpace(threadSpace));
    CMCHK_HR(m_device->DestroyTask(task));
    CMCHK_HR(m_device->DestroyBufferUP(surfaceOutput));   // ref_cnf to guarantee task finish before BufferUP being really destroy.
    CMCHK_HR(m_device->DestroyBufferUP(surfaceInput));

    if( gpuCopyKernelParam )
    {
        GPUCOPY_KERNEL_UNLOCK(gpuCopyKernelParam);
    }

finish:
    if(hr != CM_SUCCESS)
    {   //Failed
        if( surfaceInput == nullptr || surfaceOutput == nullptr)
        {
            hr = CM_GPUCOPY_OUT_OF_RESOURCE; // user need to know whether the failure is caused by out of BufferUP.
        }
        else
        {
            hr = CM_FAILURE;
        }
        if(surfaceInput)                      m_device->DestroyBufferUP(surfaceInput);
        if(surfaceOutput)                     m_device->DestroyBufferUP(surfaceOutput);
        if(kernel && gpuCopyKernelParam)        GPUCOPY_KERNEL_UNLOCK(gpuCopyKernelParam);
        if(threadSpace)                                m_device->DestroyThreadSpace(threadSpace);
        if(task)                              m_device->DestroyTask(task);
    }

    return hr;
}

//*----------------------------------------------------------------------------------------
//| Purpose:    Pop task from flushed Queue, Update surface state and Destroy the task
//| Notes:
//*----------------------------------------------------------------------------------------
void CmQueueRT::PopTaskFromFlushedQueue()
{
    CmTaskInternal* topTask = (CmTaskInternal*)m_flushedTasks.Pop();

    if ( topTask != nullptr )
    {
        CmEventRT *event = nullptr;
        topTask->GetTaskEvent( event );
        if ( event != nullptr )
        {
            LARGE_INTEGER nTime;
            if ( !(MOS_QueryPerformanceCounter( (uint64_t*)&nTime.QuadPart )) )
            {
                CM_ASSERTMESSAGE("Error: Query performace counter failure.");
            }
            else
            {
                event->SetCompleteTime( nTime );
            }
        }

#if MDF_SURFACE_CONTENT_DUMP
        PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();
        if (cmData->cmHalState->dumpSurfaceContent)
        {
            int32_t taskId = 0;
            if (event != nullptr)
            {
                event->GetTaskDriverId(taskId);
            }
            topTask->SurfaceDump(taskId);
        }
#endif

        CmTaskInternal::Destroy( topTask );
    }
    return;
}

int32_t CmQueueRT::TouchFlushedTasks( )
{
    int32_t hr = CM_SUCCESS;

    if (m_flushedTasks.IsEmpty())
    {
        if (!m_enqueuedTasks.IsEmpty())
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

    m_criticalSectionFlushedTask.Acquire();
    while( !m_flushedTasks.IsEmpty() )
    {
        CmTaskInternal* task = (CmTaskInternal*)m_flushedTasks.Top();
        CMCHK_NULL(task);

        CM_STATUS status = CM_STATUS_FLUSHED ;
        task->GetTaskStatus(status);
        if( status == CM_STATUS_FINISHED )
        {
            PopTaskFromFlushedQueue();
        }
        else
        {
            // media reset
            if (status == CM_STATUS_RESET)
            {
                PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();

                // Clear task status table in Cm Hal State
                int32_t taskId;
                CmEventRT*pTopTaskEvent;
                task->GetTaskEvent(pTopTaskEvent);
                CMCHK_NULL(pTopTaskEvent);

                pTopTaskEvent->GetTaskDriverId(taskId);
                cmData->cmHalState->taskStatusTable[taskId] = CM_INVALID_INDEX;

                //Pop task and Destroy it
                PopTaskFromFlushedQueue();
            }

            // It is an in-order queue, if this one hasn't finshed,
            // the following ones haven't finished either.
            break;
        }
    }

finish:
    m_criticalSectionFlushedTask.Release();

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
CM_RT_API int32_t CmQueueRT::DestroyEvent( CmEvent* & event )
{

    CLock Lock(m_criticalSectionEvent);

    if (event == nullptr)
    {
        return CM_FAILURE;
    }

    uint32_t index = 0;

    CmEventRT *eventRT = static_cast<CmEventRT *>(event);
    eventRT->GetIndex(index);
    CM_ASSERT( m_eventArray.GetElement( index ) == eventRT );

    int32_t status = CmEventRT::Destroy( eventRT );
    if( status == CM_SUCCESS && eventRT == nullptr)
    {
        m_eventArray.SetElement(index, nullptr);
    }

    // Should return nullptr to application even the event is not destroyed
    // since its reference count is not zero
    event = nullptr;

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
    if( !m_enqueuedTasks.IsEmpty() )
    {
        // If there are tasks not flushed (i.e. not send to driver )
        // wait untill all such tasks are flushed
        FlushTaskWithoutSync( true );
    }
    CM_ASSERT( m_enqueuedTasks.IsEmpty() );

    //Used for timeout detection
    LARGE_INTEGER freq;
    MOS_QueryPerformanceFrequency((uint64_t*)&freq.QuadPart);
    LARGE_INTEGER start;
    MOS_QueryPerformanceCounter((uint64_t*)&start.QuadPart);
    int64_t timeout = start.QuadPart + (CM_MAX_TIMEOUT * freq.QuadPart * m_flushedTasks.GetCount()); //Count to timeout at

    while( !m_flushedTasks.IsEmpty() && status != CM_EXCEED_MAX_TIMEOUT )
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
    numTasks = m_enqueuedTasks.GetCount() + m_flushedTasks.GetCount();
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Use GPU to init Surface2D
//| Returns:   result of operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueInitSurface2D( CmSurface2D* surf2D, const uint32_t initValue, CmEvent* &event)
{
    INSERT_API_CALL_LOG();

    int32_t         hr                      = CM_SUCCESS;
    uint32_t        width                   = 0;
    uint32_t        height                  = 0;
    uint32_t        sizePerPixel            = 0;
    CmProgram       *gpuInitKernelProgram  = nullptr;
    CmKernel        *kernel                = nullptr;
    SurfaceIndex    *outputIndexCM         = nullptr;
    CmThreadSpace   *threadSpace                    = nullptr;
    CmQueue         *cmQueue               = nullptr;
    CmTask          *gpuCopyTask           = nullptr;
    uint32_t        threadWidth             = 0;
    uint32_t        threadHeight            = 0;
    uint32_t        threadNum               = 0;
    CmSurfaceManager* surfaceMgr           = nullptr;
    CM_SURFACE_FORMAT      format           = CM_SURFACE_FORMAT_INVALID;

    if(!surf2D)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface 2d is null.");
        return CM_FAILURE;
    }
    CmSurface2DRT *surf2DRT = static_cast<CmSurface2DRT *>(surf2D);

    CMCHK_HR(m_device->LoadPredefinedInitKernel(gpuInitKernelProgram));

    CMCHK_HR(surf2DRT->GetSurfaceDesc(width, height, format,sizePerPixel));

    m_device->GetSurfaceManager(surfaceMgr);
    CMCHK_NULL(surfaceMgr);

    if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
    {
        CMCHK_HR(m_device->CreateKernel( gpuInitKernelProgram, _NAME( surfaceCopy_set_NV12 ), kernel, "PredefinedGPUCopyKernel"));
    }
    else
    {
        CMCHK_HR(m_device->CreateKernel( gpuInitKernelProgram, _NAME( surfaceCopy_set ), kernel, "PredefinedGPUCopyKernel" ));
    }
    CMCHK_NULL(kernel);
    CMCHK_HR(surf2D->GetIndex( outputIndexCM ));

    threadWidth = ( uint32_t )ceil( ( double )width*sizePerPixel/BLOCK_PIXEL_WIDTH/4 );
    threadHeight = ( uint32_t )ceil( ( double )height/BLOCK_HEIGHT );
    threadNum = threadWidth * threadHeight;
    CMCHK_HR(kernel->SetThreadCount( threadNum ));

    CMCHK_HR(m_device->CreateThreadSpace( threadWidth, threadHeight, threadSpace ));
    CMCHK_NULL(threadSpace);

    CMCHK_HR(kernel->SetKernelArg( 0, sizeof( uint32_t ), &initValue ));
    CMCHK_HR(kernel->SetKernelArg( 1, sizeof( SurfaceIndex ), outputIndexCM ));

    CMCHK_HR(m_device->CreateQueue( cmQueue ));

    CMCHK_HR(m_device->CreateTask(gpuCopyTask));
    CMCHK_NULL(gpuCopyTask);

    CMCHK_HR(gpuCopyTask->AddKernel( kernel ));

    CMCHK_HR(cmQueue->Enqueue( gpuCopyTask, event, threadSpace ));

finish:

    if (kernel)        m_device->DestroyKernel( kernel );
    if (gpuCopyTask)   m_device->DestroyTask(gpuCopyTask);
    if (threadSpace)            m_device->DestroyThreadSpace(threadSpace);

    return hr;
}

//*-----------------------------------------------------------------------------
//! Flush a geneal task to HAL CM layer for execution.
//! This is a non-blocking call. i.e. it returs immediately without waiting for
//! GPU to finish the execution of tasks.
//! INPUT: task -- Pointer to CmTaskInternal object
//! OUTPUT:
//!     CM_SUCCESS if all tasks in the queue are submitted
//!     CM_FAILURE otherwise.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::FlushGeneralTask(CmTaskInternal* task)
{
    CM_RETURN_CODE          hr              = CM_SUCCESS;
    CM_HAL_EXEC_TASK_PARAM  param;
    PCM_HAL_KERNEL_PARAM    kernelParam    = nullptr;
    CmKernelData*           kernelData     = nullptr;
    uint32_t                kernelDataSize  = 0;
    PCM_CONTEXT_DATA        cmData         = nullptr;
    CmEventRT*              event          = nullptr;
    uint32_t                totalThreadCount= 0;
    uint32_t                count           = 0;
    PCM_HAL_KERNEL_PARAM    tempData       = nullptr;
    uint32_t                maxTSWidth      = 0;
    bool                    hasThreadArg    = false;

    CmSafeMemSet( &param, 0, sizeof( CM_HAL_EXEC_TASK_PARAM ) );

    //GT-PIN
    if(m_device->CheckGTPinEnabled())
    {
        CMCHK_HR(task->GetKernelSurfInfo(param.surfEntryInfoArrays));
    }

    task->GetKernelCount( count );
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
        task->GetKernelData( i, kernelData );
        CMCHK_NULL(kernelData);

        kernelParam = kernelData->GetHalCmKernelData();
        CMCHK_NULL(kernelParam);

        hasThreadArg |= kernelParam->perThreadArgExisted;

        task->GetKernelDataSize( i, kernelDataSize );
        if(kernelDataSize == 0)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel data size.");
            hr = CM_FAILURE;
            goto finish;
        }

        tempData = kernelData->GetHalCmKernelData();

        param.kernels[ i ]             = tempData;
        param.kernelSizes[ i ]        = kernelDataSize;
        param.kernelCurbeOffset[ i ]  = task->GetKernelCurbeOffset(i);
        param.globalSurfaceUsed       |= tempData->globalSurfaceUsed;
        param.kernelDebugEnabled      |= tempData->kernelDebugEnabled;
    }

    /*
    * Preset the default TS width/height/dependency:
    *     TS width   = MOS_MIN(CM_MAX_THREADSPACE_WIDTH, threadcount)
    *     TS height  = totalThreadCount/CM_MAX_THREADSPACE_WIDTH + 1
    *     dependency = CM_NONE_DEPENDENCY
    * For threadSpace is nullptr case, we will pass the default TS width/height/dependency to driver
    * For threadSpace is valid case, the TS width/height/dependency will be update according to thread space set by user.
    */
    task->GetTotalThreadCount(totalThreadCount);

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

    if (task->IsThreadSpaceCreated()) //scoreboard data preparation
    {
        if(task->IsThreadCoordinatesExisted())
        {
            param.threadCoordinates = MOS_NewArray(PCM_HAL_SCOREBOARD, count);
            param.dependencyMasks = MOS_NewArray(PCM_HAL_MASK_AND_RESET, count);

            CMCHK_NULL_RETURN(param.threadCoordinates, CM_OUT_OF_HOST_MEMORY);
            CMCHK_NULL_RETURN(param.dependencyMasks, CM_OUT_OF_HOST_MEMORY);
            for(uint32_t i=0; i<count; i++)
            {
                void *kernelCoordinates = nullptr;
                void *dependencyMasks = nullptr;
                task->GetKernelCoordinates(i, kernelCoordinates);
                task->GetKernelDependencyMasks(i, dependencyMasks);
                param.threadCoordinates[i] = (PCM_HAL_SCOREBOARD)kernelCoordinates;
                param.dependencyMasks[i] = (PCM_HAL_MASK_AND_RESET)dependencyMasks;
            }
        }
        else
        {
            param.threadCoordinates = nullptr;
        }

        task->GetDependencyPattern(param.dependencyPattern);

        task->GetThreadSpaceSize(param.threadSpaceWidth, param.threadSpaceHeight);

        task->GetWalkingPattern(param.walkingPattern);

        if( task->CheckWalkingParametersSet( ) )
        {
            param.walkingParamsValid = 1;
            CMCHK_HR(task->GetWalkingParameters(param.walkingParams));
        }
        else
        {
            param.walkingParamsValid = 0;
        }

        if( task->CheckDependencyVectorsSet( ) )
        {
            param.dependencyVectorsValid = 1;
            CMCHK_HR(task->GetDependencyVectors(param.dependencyVectors));
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
    task->GetColorCountMinusOne(param.colorCountMinusOne);
    task->GetMediaWalkerGroupSelect(param.mediaWalkerGroupSelect);

    param.syncBitmap = task->GetSyncBitmap();
    param.conditionalEndBitmap = task->GetConditionalEndBitmap();
    param.userDefinedMediaState = task->GetMediaStatePtr();
    CmSafeMemCopy(param.conditionalEndInfo, task->GetConditionalEndInfo(), sizeof(param.conditionalEndInfo));
    CmSafeMemCopy(&param.taskConfig, task->GetTaskConfig(), sizeof(param.taskConfig));
    cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();

    CHK_MOSSTATUS_RETURN_CMERROR(cmData->cmHalState->pfnSetPowerOption(cmData->cmHalState, task->GetPowerOption()));

    m_device->RegisterSyncEvent(nullptr);
    CHK_MOSSTATUS_RETURN_CMERROR(cmData->cmHalState->pfnExecuteTask(cmData->cmHalState, &param));

    if( param.taskIdOut < 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid task ID.");
        hr = CM_FAILURE;
        goto finish;
    }

    TASK_LOG(task);

    task->GetTaskEvent( event );
    CMCHK_NULL(event);
    CMCHK_HR(event->SetTaskDriverId( param.taskIdOut ));
    CMCHK_HR(event->SetTaskOsData( param.osData ));
    CMCHK_HR(task->ResetKernelDataStatus());

    //GT-PIN
    if(m_device->CheckGTPinEnabled())
    {
        //No need to clear the SurEntryInfoArrays here. It will be destored by CmInternalTask
        CMCHK_HR(event->SetSurfaceDetails(param.surfEntryInfoArrays));
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
//! INPUT: task -- Pointer to CmTaskInternal object
//! OUTPUT:
//!     CM_SUCCESS if all tasks in the queue are submitted
//!     CM_FAILURE otherwise.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::FlushGroupTask(CmTaskInternal* task)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    CM_HAL_EXEC_TASK_GROUP_PARAM param;
    CmKernelData* kernelData   = nullptr;
    uint32_t kernelDataSize        = 0;
    uint32_t count                  = 0;
    PCM_CONTEXT_DATA cmData    = nullptr;
    CmEventRT * event          = nullptr;
    PCM_HAL_KERNEL_PARAM tempData  = nullptr;

    CmSafeMemSet( &param, 0, sizeof( CM_HAL_EXEC_TASK_GROUP_PARAM ) );

    //GT-PIN
    if(this->m_device->CheckGTPinEnabled())
    {
        CMCHK_HR(task->GetKernelSurfInfo(param.surEntryInfoArrays));
    }

    task->GetKernelCount( count );
    param.numKernels = count;

    param.kernels = MOS_NewArray(PCM_HAL_KERNEL_PARAM, count);
    param.kernelSizes = MOS_NewArray(uint32_t, count);
    param.kernelCurbeOffset = MOS_NewArray(uint32_t, count);
    param.queueOption = m_queueOption;

    CmSafeMemCopy(&param.taskConfig, task->GetTaskConfig(), sizeof(param.taskConfig));
    CMCHK_NULL(param.kernels);
    CMCHK_NULL(param.kernelSizes);
    CMCHK_NULL(param.kernelCurbeOffset);

    for( uint32_t i = 0; i < count; i ++ )
    {
        task->GetKernelData( i, kernelData );
        CMCHK_NULL(kernelData);

        task->GetKernelDataSize( i, kernelDataSize );
        if( kernelDataSize == 0)
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel data size.");
            hr = CM_FAILURE;
            goto finish;
        }

        tempData = kernelData->GetHalCmKernelData( );

        param.kernels[ i ]             = tempData;
        param.kernelSizes[ i ]        = kernelDataSize;
        param.kernelCurbeOffset [ i ] = task->GetKernelCurbeOffset(i);
        param.globalSurfaceUsed        |= tempData->globalSurfaceUsed;
        param.kernelDebugEnabled       |= tempData->kernelDebugEnabled;
    }

    task->GetSLMSize(param.slmSize);
    if(param.slmSize > MAX_SLM_SIZE_PER_GROUP_IN_1K)
    {
        CM_ASSERTMESSAGE("Error: SLM size exceeds the maximum per group.");
        hr = CM_EXCEED_MAX_SLM_SIZE;
        goto finish;
    }

    if (task->IsThreadGroupSpaceCreated())//thread group size
    {
        task->GetThreadGroupSpaceSize(param.threadSpaceWidth, param.threadSpaceHeight, param.threadSpaceDepth, param.groupSpaceWidth, param.groupSpaceHeight, param.groupSpaceDepth);
    }

    param.syncBitmap = task->GetSyncBitmap();
    param.userDefinedMediaState = task->GetMediaStatePtr();

    // Call HAL layer to execute pfnExecuteGroupTask
    cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();

    CHK_MOSSTATUS_RETURN_CMERROR( cmData->cmHalState->pfnSetPowerOption( cmData->cmHalState, task->GetPowerOption() ) );

    m_device->RegisterSyncEvent(nullptr);
    CHK_MOSSTATUS_RETURN_CMERROR( cmData->cmHalState->pfnExecuteGroupTask( cmData->cmHalState, &param ) );

    if( param.taskIdOut < 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid task ID.");
        hr = CM_FAILURE;
        goto finish;
    }
    TASK_LOG(task);
    task->GetTaskEvent( event );
    CMCHK_NULL( event );
    CMCHK_HR(event->SetTaskDriverId( param.taskIdOut ));
    CMCHK_HR(event->SetTaskOsData( param.osData ));
    CMCHK_HR(task->ResetKernelDataStatus());

    //GT-PIN
    if(this->m_device->CheckGTPinEnabled())
    {
        CMCHK_HR(event->SetSurfaceDetails(param.surEntryInfoArrays));
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
//! INPUT: task -- Pointer to CmTaskInternal object
//! OUTPUT:
//!     CM_SUCCESS if all tasks in the queue are submitted
//!     CM_FAILURE otherwise.
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::FlushVeboxTask(CmTaskInternal* task)
{
    CM_RETURN_CODE  hr          = CM_SUCCESS;

    CM_HAL_EXEC_VEBOX_TASK_PARAM param;
    PCM_CONTEXT_DATA cmData    = nullptr;
    CmEventRT * event          = nullptr;
    uint8_t *stateData           = nullptr;
    uint8_t *surfaceData         = nullptr;
    CmBuffer_RT * temp          = nullptr;

    CmSafeMemSet( &param, 0, sizeof( CM_HAL_EXEC_VEBOX_TASK_PARAM ) );

    //Set VEBOX state data pointer and size
    //Set VEBOX surface data pointer and size
    CM_VEBOX_STATE cmVeboxState;
    CmBufferUP *veboxParamBuf = nullptr;
    CM_VEBOX_SURFACE_DATA cmVeboxSurfaceData;
    task->GetVeboxState(cmVeboxState);
    task->GetVeboxParam(veboxParamBuf);
    task->GetVeboxSurfaceData(cmVeboxSurfaceData);
    CMCHK_NULL(veboxParamBuf);

    temp = static_cast<CmBuffer_RT*>(veboxParamBuf);
    temp->GetHandle(param.veboxParamIndex);

    param.cmVeboxState = cmVeboxState;
    param.veboxParam = veboxParamBuf;

    param.veboxSurfaceData = cmVeboxSurfaceData;

    //Set VEBOX task id to -1
    param.taskIdOut = -1;

    cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();
    m_device->RegisterSyncEvent(nullptr);
    CHK_MOSSTATUS_RETURN_CMERROR( cmData->cmHalState->pfnExecuteVeboxTask( cmData->cmHalState, &param ) );

    if( param.taskIdOut < 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid task ID.");
        hr = CM_FAILURE;
        goto finish;
    }

    task->GetTaskEvent( event );
    CMCHK_NULL( event );
    CMCHK_HR(event->SetTaskDriverId( param.taskIdOut ));
    CMCHK_HR(event->SetTaskOsData( param.osData ));

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
int32_t CmQueueRT::FlushEnqueueWithHintsTask( CmTaskInternal* task )
{
    CM_RETURN_CODE               hr             = CM_SUCCESS;
    CM_HAL_EXEC_HINTS_TASK_PARAM param;
    PCM_CONTEXT_DATA             cmData        = nullptr;
    CmKernelData*                kernelData    = nullptr;
    uint32_t                     kernelDataSize = 0;
    uint32_t                     count          = 0;
    CmEventRT                    *event        = nullptr;
    PCM_HAL_KERNEL_PARAM         tempData      = nullptr;

    CmSafeMemSet( &param, 0, sizeof( CM_HAL_EXEC_HINTS_TASK_PARAM ) );

    task->GetKernelCount ( count );
    param.numKernels = count;

    param.kernels = MOS_NewArray(PCM_HAL_KERNEL_PARAM, count);
    param.kernelSizes = MOS_NewArray(uint32_t, count);
    param.kernelCurbeOffset = MOS_NewArray(uint32_t, count);
    param.queueOption = m_queueOption;

    CMCHK_NULL(param.kernels);
    CMCHK_NULL(param.kernelSizes);
    CMCHK_NULL(param.kernelCurbeOffset);

    task->GetHints(param.hints);
    task->GetNumTasksGenerated(param.numTasksGenerated);
    task->GetLastTask(param.isLastTask);

    for( uint32_t i = 0; i < count; i ++ )
    {
        task->GetKernelData( i, kernelData );
        CMCHK_NULL( kernelData );

        task->GetKernelDataSize( i, kernelDataSize );
        if( kernelDataSize == 0 )
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel data size.");
            hr = CM_FAILURE;
            goto finish;
        }

        tempData = kernelData->GetHalCmKernelData();

        param.kernels[ i ]             = tempData;
        param.kernelSizes[ i ]         = kernelDataSize;
        param.kernelCurbeOffset[ i ]   = task->GetKernelCurbeOffset(i);
    }

    param.userDefinedMediaState = task->GetMediaStatePtr();
    cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();
    CMCHK_NULL(cmData);

    CHK_MOSSTATUS_RETURN_CMERROR(cmData->cmHalState->pfnSetPowerOption(cmData->cmHalState, task->GetPowerOption()));

    m_device->RegisterSyncEvent(nullptr);
    CHK_MOSSTATUS_RETURN_CMERROR(cmData->cmHalState->pfnExecuteHintsTask(cmData->cmHalState, &param));

    if( param.taskIdOut < 0 )
    {
        CM_ASSERTMESSAGE("Error: Invalid task ID.");
        hr = CM_FAILURE;
        goto finish;
    }

    TASK_LOG(task);

    task->GetTaskEvent( event );
    CMCHK_NULL( event );
    CMCHK_HR(event->SetTaskDriverId( param.taskIdOut ));
    CMCHK_HR(event->SetTaskOsData( param.osData ));
    CMCHK_HR(task->ResetKernelDataStatus());

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
int32_t CmQueueRT::FlushTaskWithoutSync( bool flushBlocked )
{
    int32_t             hr          = CM_SUCCESS;
    CmTaskInternal*     task       = nullptr;
    uint32_t            taskType  = CM_TASK_TYPE_DEFAULT;
    uint32_t            freeSurfNum = 0;
    CmSurfaceManager*   surfaceMgr = nullptr;
    CSync*              surfaceLock = nullptr;

    m_criticalSectionHalExecute.Acquire(); // Enter HalCm Execute Protection

    while( !m_enqueuedTasks.IsEmpty() )
    {
        uint32_t flushedTaskCount = m_flushedTasks.GetCount();
        if ( flushBlocked )
        {
            while( flushedTaskCount >= m_halMaxValues->maxTasks )
            {
                // If the task count in flushed queue is no less than hw restrictiion,
                // query the staus of flushed task queue. Remove any finished tasks from the queue
                QueryFlushedTasks();
                flushedTaskCount = m_flushedTasks.GetCount();
            }
        }
        else
        {
            if( flushedTaskCount >= m_halMaxValues->maxTasks )
            {
                // If the task count in flushed queue is no less than hw restrictiion,
                // query the staus of flushed task queue. Remove any finished tasks from the queue
                QueryFlushedTasks();
                flushedTaskCount = m_flushedTasks.GetCount();
                if( flushedTaskCount >= m_halMaxValues->maxTasks )
                {
                    // If none of flushed tasks finishes, we can't flush more taks.
                    break;
                }
            }
        }

        task = (CmTaskInternal*)m_enqueuedTasks.Pop();
        CMCHK_NULL( task );

        CmNotifierGroup *notifiers = m_device->GetNotifiers();
        if (notifiers != nullptr)
        {
            notifiers->NotifyTaskFlushed(m_device, task);
        }

        task->GetTaskType(taskType);

        switch(taskType)
        {
            case CM_INTERNAL_TASK_WITH_THREADSPACE:
                hr = FlushGeneralTask(task);
                break;

            case CM_INTERNAL_TASK_WITH_THREADGROUPSPACE:
                hr = FlushGroupTask(task);
                break;

            case CM_INTERNAL_TASK_VEBOX:
                hr = FlushVeboxTask(task);
                break;

            case CM_INTERNAL_TASK_ENQUEUEWITHHINTS:
                hr = FlushEnqueueWithHintsTask(task);
                break;

            default:    // by default, assume the task is considered as general task: CM_INTERNAL_TASK_WITH_THREADSPACE
                hr = FlushGeneralTask(task);
                break;
        }

        if(hr == CM_SUCCESS)
        {
            m_flushedTasks.Push( task );
            task->VtuneSetFlushTime(); // Record Flush Time
        }
        else
        {
            // Failed to flush, destroy the task.
            CmTaskInternal::Destroy( task );
        }

    } // loop for task

    QueryFlushedTasks();

finish:
    m_criticalSectionHalExecute.Release();//Leave HalCm Execute Protection

    //Delayed destroy for resource
    m_device->GetSurfaceManager(surfaceMgr);
    if (!surfaceMgr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface manager is null.");
        return CM_NULL_POINTER;
    }

    surfaceLock = m_device->GetSurfaceCreationLock();
    if (surfaceLock == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface creation lock is null.");
        return CM_NULL_POINTER;
    }
    surfaceLock->Acquire();
    surfaceMgr->DestroySurfaceInPool(freeSurfNum, DELAYED_DESTROY);
    surfaceLock->Release();

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Enqueue a Vebox Task
//| Arguments :
//|               pVebox_G75      [in]       Pointer to a CmVebox object
//|               event          [in]       Reference to the pointer to Event
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueVebox(CmVebox * vebox, CmEvent* & event)
{
    INSERT_API_CALL_LOG();

    int32_t hr                  = CM_SUCCESS;
    CmTaskInternal* task   = nullptr;
    int32_t taskDriverId        = -1;
    bool isEventVisible    = (event == CM_NO_EVENT)? false:true;
    CmEventRT *eventRT = static_cast<CmEventRT *>(event);

    //Check if the input is valid
    if ( vebox == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Pointer to vebox is null.");
        return CM_NULL_POINTER;
    }
    CmVeboxRT *veboxRT = static_cast<CmVeboxRT *>(vebox);
    CMCHK_HR(CmTaskInternal::Create(m_device,  veboxRT, task ));

    LARGE_INTEGER nEnqueueTime;
    if ( !(MOS_QueryPerformanceCounter( (uint64_t*)&nEnqueueTime.QuadPart )) )
    {
        CM_ASSERTMESSAGE("Error: Query Performance counter failure.");
        return CM_FAILURE;
    }

    CMCHK_HR(CreateEvent(task, isEventVisible, taskDriverId, eventRT));

    if ( eventRT != nullptr )
    {
        eventRT->SetEnqueueTime( nEnqueueTime );
    }
    event = eventRT;

    if (!m_enqueuedTasks.Push(task))
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
//| Purpose:   Create Event and Update event in m_eventArray
//| Returns:   result of operation
//*-----------------------------------------------------------------------------
int32_t CmQueueRT::CreateEvent(CmTaskInternal *task, bool isVisible, int32_t &taskDriverId, CmEventRT *&event )
{
    int32_t hr = CM_SUCCESS;

    m_criticalSectionEvent.Acquire();

    uint32_t freeSlotInEventArray = m_eventArray.GetFirstFreeIndex();

    hr = CmEventRT::Create( freeSlotInEventArray, this, task, taskDriverId, m_device, isVisible, event );

    if (hr == CM_SUCCESS)
    {

        m_eventArray.SetElement( freeSlotInEventArray, event );
        m_eventCount ++;

        task->SetTaskEvent( event );

        if(!isVisible)
        {
            event = nullptr;
        }

    }
    else
    {
        CM_ASSERTMESSAGE("Error: Create Event failure.")
    }

    m_criticalSectionEvent.Release();

    return hr;
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       EnqueueCopyCPUToGPUFullStride()
//| Purpose:    Copy data from system memory to video memory (surface)
//| Arguments:
//|             surface      [in]  Pointer to a CmSurface2D object as copy destination
//|             sysMem       [in]  Pointer to a system memory as copy source
//|             widthStride   [in]  Width stride in bytes for system memory (to calculate start of next line)
//|             heightStride  [in]  Width stride in row for system memory (to calculate start of next plane)
//|             option        [in]  Option passed from user, blocking copy, non-blocking copy or disable turbo boost
//|             event        [in,out]  Reference to the pointer to Event
//| Returns:    Result of the operation.
//|
//| Restrictions & Notes:
//|             1) sysMem must be 16-byte aligned.
//|             2) Surface's width must be 16-byte aligned regarding performance.
//|             3) widthStride and heightStride are used to indicate the padding information in system memory
//|                 widthStride = width_in_pixel * bytes_per_pixel + padding_in_bytes
//|                 heightStride = height + padding_in_row
//*---------------------------------------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueCopyCPUToGPUFullStride( CmSurface2D* surface,
                                                     const unsigned char* sysMem,
                                                     const uint32_t widthStride,
                                                     const uint32_t heightStride,
                                                     const uint32_t option,
                                                     CmEvent* & event )
{
    INSERT_API_CALL_LOG();

    CmSurface2DRT *surfaceRT = static_cast<CmSurface2DRT *>(surface);
    return EnqueueCopyInternal(surfaceRT, (unsigned char*)sysMem, widthStride, heightStride, CM_FASTCOPY_CPU2GPU, option, event);
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       EnqueueCopyGPUToCPUFullStride()
//| Purpose:    Copy data from tiled video memory (surface) to linear system memory
//| Arguments:
//|             surface      [in]  Pointer to a CmSurface2D object as copy source
//|             sysMem       [in]  Pointer to a system memory as copy destination
//|             widthStride   [in]  Width stride in bytes for system memory (to calculate start of next line)
//|             heightStride  [in]  Width stride in row for system memory (to calculate start of next plane)
//|             option        [in]  Option passed from user, blocking copy,non-blocking copy or disable turbo boost
//|             event        [in,out]  Reference to the pointer to Event
//| Returns:    Result of the operation.
//|
//| Restrictions & Notes:
//|             1) sysMem must be 16-byte aligned.
//|             2) Surface's width must be 16-byte aligned regarding performance.
//|             3) widthStride and heightStride are used to indicate the padding information in system memory
//|                 widthStride = width_in_pixel * bytes_per_pixel + padding_in_bytes
//|                 heightStride = height + padding_in_row
//*---------------------------------------------------------------------------------------------------------
CM_RT_API int32_t CmQueueRT::EnqueueCopyGPUToCPUFullStride( CmSurface2D* surface,
                                                     unsigned char* sysMem,
                                                     const uint32_t widthStride,
                                                     const uint32_t heightStride,
                                                     const uint32_t option,
                                                     CmEvent* & event )
{
    INSERT_API_CALL_LOG();

    CmSurface2DRT *surfaceRT = static_cast<CmSurface2DRT *>(surface);
    return EnqueueCopyInternal(surfaceRT, sysMem, widthStride, heightStride, CM_FASTCOPY_GPU2CPU, option, event);
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       CreateGPUCopyKernel()
//| Purpose:    Create GPUCopy kernel, reuse the kernel if it has been created and resuable
//| Arguments:
//|             widthInByte      [in]  surface's width in bytes
//|             height           [in]  surface's height
//|             format           [in]  surface's height
//|             copyDirection    [in]  copy direction, cpu -> gpu or gpu -> cpu
//|             gpuCopyKernelParam [out] kernel param
//|
//| Returns:    Result of the operation.
//|
//*---------------------------------------------------------------------------------------------------------
int32_t CmQueueRT::CreateGPUCopyKernel(uint32_t widthInByte,
                                       uint32_t height,
                                       CM_SURFACE_FORMAT format,
                                       CM_GPUCOPY_DIRECTION copyDirection,
                                       CM_GPUCOPY_KERNEL* &gpuCopyKernelParam)
{
    int32_t     hr                 = CM_SUCCESS;

    //Search existing kernel
    CMCHK_HR(SearchGPUCopyKernel(widthInByte, height, format, copyDirection, gpuCopyKernelParam));

    if(gpuCopyKernelParam != nullptr)
    { // reuse
        GPUCOPY_KERNEL_LOCK(gpuCopyKernelParam);
    }
    else
    {
        gpuCopyKernelParam   = new (std::nothrow) CM_GPUCOPY_KERNEL ;
        CMCHK_NULL(gpuCopyKernelParam);
        CmSafeMemSet(gpuCopyKernelParam, 0, sizeof(CM_GPUCOPY_KERNEL));

        CMCHK_HR(AllocateGPUCopyKernel(widthInByte, height, format, copyDirection, gpuCopyKernelParam->kernel));
        CMCHK_HR(GetGPUCopyKrnID(widthInByte, height, format, copyDirection, gpuCopyKernelParam->kernelID));
        GPUCOPY_KERNEL_LOCK(gpuCopyKernelParam);

        CMCHK_HR(AddGPUCopyKernel(gpuCopyKernelParam));
    }

finish:
    if( hr != CM_SUCCESS)
    {
        CmSafeDelete(gpuCopyKernelParam);
    }

    return hr;
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       SearchGPUCopyKernel()
//| Purpose:    Search if the required kernel exists
//| Arguments:
//|             widthInByte      [in]  surface's width in bytes
//|             height           [in]  surface's height
//|             format           [in]  surface's height
//|             copyDirection    [in]  copy direction, cpu -> gpu or gpu -> cpu
//|             gpuCopyKernelParam [out] kernel param
//|
//| Returns:    Result of the operation.
//|
//*---------------------------------------------------------------------------------------------------------
int32_t CmQueueRT::SearchGPUCopyKernel(uint32_t widthInByte,
                                       uint32_t height,
                                       CM_SURFACE_FORMAT format,
                                       CM_GPUCOPY_DIRECTION copyDirection,
                                       CM_GPUCOPY_KERNEL* &kernelParam)
{
    int32_t     hr = CM_SUCCESS;
    CM_GPUCOPY_KERNEL *gpucopyKernel = nullptr;
    CM_GPUCOPY_KERNEL_ID kernelTypeID = GPU_COPY_KERNEL_UNKNOWN;

    kernelParam = nullptr;
    CMCHK_HR(GetGPUCopyKrnID(widthInByte, height, format, copyDirection, kernelTypeID));

    for(uint32_t index =0 ;  index< m_copyKernelParamArrayCount; index++)
    {
        gpucopyKernel = (CM_GPUCOPY_KERNEL*)m_copyKernelParamArray.GetElement(index);
        if(gpucopyKernel != nullptr)
        {
            if(!gpucopyKernel->locked &&
               gpucopyKernel->kernelID == kernelTypeID)
            {
                kernelParam = gpucopyKernel;
                break;
            }
        }
    }

finish:
    return hr;
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       AddGPUCopyKernel()
//| Purpose:    Add new kernel into m_copyKernelParamArray
//| Arguments:
//|             widthInByte      [in]  surface's width in bytes
//|             height           [in]  surface's height
//|             format           [in]  surface's height
//|             copyDirection    [in]  copy direction, cpu -> gpu or gpu -> cpu
//|             gpuCopyKernelParam [out] kernel param
//|
//| Returns:    Result of the operation.
//|
//*---------------------------------------------------------------------------------------------------------
int32_t CmQueueRT::AddGPUCopyKernel(CM_GPUCOPY_KERNEL* &kernelParam)
{
    int32_t hr = CM_SUCCESS;
    // critical section protection
    CLock locker(m_criticalSectionGPUCopyKrn);

    CMCHK_NULL_RETURN(kernelParam, CM_INVALID_GPUCOPY_KERNEL);

    // the newly created kernel must be locked
    if(!kernelParam->locked)
    {
        CM_ASSERTMESSAGE("Error: The newly created kernel must be locked.")
        hr = CM_INVALID_GPUCOPY_KERNEL;
        goto finish;
    }

    m_copyKernelParamArray.SetElement(m_copyKernelParamArrayCount, kernelParam);
    m_copyKernelParamArrayCount ++;

finish:
    return hr;
}

//*---------------------------------------------------------------------------------------------------------
//| Name:       GetGPUCopyKrnID()
//| Purpose:    Calculate the kernel ID accroding surface's width, height and copy direction
//| Arguments:
//|             widthInByte      [in]  surface's width in bytes
//|             height           [in]  surface's height
//|             format           [in]  surface's height
//|             copyDirection    [in]  copy direction, cpu -> gpu or gpu -> cpu
//|             kernelID         [out] kernel id
//|
//| Returns:    Result of the operation.
//|
//*---------------------------------------------------------------------------------------------------------
int32_t CmQueueRT::GetGPUCopyKrnID( uint32_t widthInByte, uint32_t height, CM_SURFACE_FORMAT format,
            CM_GPUCOPY_DIRECTION copyDirection, CM_GPUCOPY_KERNEL_ID &kernelID )
{
    int32_t hr = CM_SUCCESS;

    kernelID = GPU_COPY_KERNEL_UNKNOWN;

    if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
    {
        switch(copyDirection)
        {
            case CM_FASTCOPY_GPU2CPU:
                if ( (height&0x7) ||(widthInByte&0x7f))
                {
                    kernelID = GPU_COPY_KERNEL_GPU2CPU_UNALIGNED_NV12_ID ;
                }
                else
                {   // height 8-row aligned, widthByte 128 multiple
                    kernelID = GPU_COPY_KERNEL_GPU2CPU_ALIGNED_NV12_ID ;
                }
                break;

            case CM_FASTCOPY_CPU2GPU:
                kernelID = GPU_COPY_KERNEL_CPU2GPU_NV12_ID;
                break;

            case CM_FASTCOPY_GPU2GPU:
                kernelID = GPU_COPY_KERNEL_GPU2GPU_NV12_ID;
                break;

            case CM_FASTCOPY_CPU2CPU:
                kernelID = GPU_COPY_KERNEL_CPU2CPU_ID;
                break;

            default :
                CM_ASSERTMESSAGE("Error: Invalid fast copy direction.")
                hr = CM_FAILURE;
                break;
        }
    }
    else
    {
        switch(copyDirection)
        {
            case CM_FASTCOPY_GPU2CPU:
                if ( (height&0x7) ||(widthInByte&0x7f))
                {
                    kernelID = GPU_COPY_KERNEL_GPU2CPU_UNALIGNED_ID;
                }
                else
                {   // height 8-row aligned, widthByte 128 multiple
                    kernelID = GPU_COPY_KERNEL_GPU2CPU_ALIGNED_ID;
                }
                break;

            case CM_FASTCOPY_CPU2GPU:
                kernelID = GPU_COPY_KERNEL_CPU2GPU_ID;
                break;

            case CM_FASTCOPY_GPU2GPU:
                kernelID = GPU_COPY_KERNEL_GPU2GPU_ID;
                break;

            case CM_FASTCOPY_CPU2CPU:
                kernelID = GPU_COPY_KERNEL_CPU2CPU_ID;
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
//|             widthInByte      [in]  surface's width in bytes
//|             height           [in]  surface's height
//|             format           [in]  surface's height
//|             copyDirection    [in]  copy direction, cpu -> gpu or gpu -> cpu
//|             kernel          [out] pointer to created kernel
//|
//| Returns:    Result of the operation.
//|
//*---------------------------------------------------------------------------------------------------------
int32_t CmQueueRT::AllocateGPUCopyKernel( uint32_t widthInByte, uint32_t height, CM_SURFACE_FORMAT format,
            CM_GPUCOPY_DIRECTION copyDirection, CmKernel *&kernel )
{
    int32_t          hr                 = CM_SUCCESS;
    CmProgram       *gpuCopyProgram    = nullptr;

    CMCHK_HR( m_device->LoadPredefinedCopyKernel(gpuCopyProgram));
    CMCHK_NULL(gpuCopyProgram);

    if (format == CM_SURFACE_FORMAT_NV12 || format == CM_SURFACE_FORMAT_P010 || format == CM_SURFACE_FORMAT_P016)
    {
        switch(copyDirection)
        {
            case CM_FASTCOPY_GPU2CPU:
                if ( (height&0x7) ||(widthInByte&0x7f))
                {
                    CMCHK_HR(m_device->CreateKernel( gpuCopyProgram, _NAME( surfaceCopy_read_NV12_32x32 ) , kernel,"PredefinedGPUCopyKernel"));
                }
                else
                {   // height 8-row aligned, widthByte 128 multiple
                    CMCHK_HR(m_device->CreateKernel( gpuCopyProgram, _NAME( surfaceCopy_read_NV12_aligned_32x32 ) , kernel,"PredefinedGPUCopyKernel"));
                }
                break;

            case CM_FASTCOPY_CPU2GPU:
                CMCHK_HR( m_device->CreateKernel( gpuCopyProgram, _NAME( surfaceCopy_write_NV12_32x32 ), kernel, "PredefinedGPUCopyKernel"));
                break;

            case CM_FASTCOPY_GPU2GPU:
                CMCHK_HR(m_device->CreateKernel(gpuCopyProgram, _NAME(SurfaceCopy_2DTo2D_NV12_32x32), kernel, "PredefinedGPUCopyKernel"));
                break;

            case CM_FASTCOPY_CPU2CPU:
                CMCHK_HR(m_device->CreateKernel(gpuCopyProgram, _NAME(SurfaceCopy_BufferToBuffer_4k), kernel, "PredefinedGPUCopyKernel"));
                break;

            default :
                CM_ASSERTMESSAGE("Error: Invalid fast copy direction.")
                hr = CM_FAILURE;
                break;
        }
    }
    else
    {
        switch(copyDirection)
        {
            case CM_FASTCOPY_GPU2CPU:
                if ( (height&0x7) ||(widthInByte&0x7f))
                {
                    CMCHK_HR(m_device->CreateKernel( gpuCopyProgram, _NAME( surfaceCopy_read_32x32 ) , kernel, "PredefinedGPUCopyKernel"));
                }
                else
                {   // height 8-row aligned, widthByte 128 multiple
                    CMCHK_HR(m_device->CreateKernel( gpuCopyProgram, _NAME( surfaceCopy_read_aligned_32x32  ) , kernel, "PredefinedGPUCopyKernel"));
                }
                break;

            case CM_FASTCOPY_CPU2GPU:
                CMCHK_HR( m_device->CreateKernel( gpuCopyProgram, _NAME( surfaceCopy_write_32x32 ), kernel, "PredefinedGPUCopyKernel" ));
                break;

            case CM_FASTCOPY_GPU2GPU:
                CMCHK_HR(m_device->CreateKernel(gpuCopyProgram, _NAME(SurfaceCopy_2DTo2D_32x32), kernel, "PredefinedGPUCopyKernel"));
                break;

            case CM_FASTCOPY_CPU2CPU:
                CMCHK_HR(m_device->CreateKernel(gpuCopyProgram, _NAME(SurfaceCopy_BufferToBuffer_4k), kernel, "PredefinedGPUCopyKernel"));
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