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
//! \file      cm_event_rt.cpp
//! \brief     Contains OS-agnostic CmEventRT member functions.
//!

#include "cm_event_rt.h"

#include "cm_device_rt.h"
#include "cm_queue_rt.h"
#include "cm_mem.h"
#include "cm_task_rt.h"
#include "cm_kernel_rt.h"
#include "cm_thread_space_rt.h"
#include "cm_group_space.h"
#include "cm_surface_manager.h"
#include "cm_task_internal.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::Create(uint32_t index, CmQueueRT *queue, CmTaskInternal *task, int32_t taskDriverId, CmDeviceRT *device, bool isVisible, CmEventRT *&event)
{
    int32_t result = CM_SUCCESS;
    event = new (std::nothrow) CmEventRT( index, queue, task, taskDriverId, device, isVisible );
    if( event )
    {
        if(isVisible)
        {   // Increase the refcount when the Event is visiable
            event->Acquire();
        }
        result = event->Initialize();
        if( result != CM_SUCCESS )
        {
            CmEventRT::Destroy( event );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmEvent due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::Destroy( CmEventRT* &event )
{
    long refCount = event->SafeRelease();
    if( refCount == 0 )
    {
        event = nullptr;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmEventRT::CmEventRT(uint32_t index, CmQueueRT *queue, CmTaskInternal *task, int32_t taskDriverId, CmDeviceRT *device, bool isVisible):
    m_index( index ),
    m_taskDriverId( taskDriverId ),
    m_osData(nullptr),
    m_status( CM_STATUS_QUEUED ),
    m_time( 0 ),
    m_ticks(0),
    m_hwStartTimeStampInTicks( 0 ),
    m_hwEndTimeStampInTicks( 0 ),
    m_device( device ),
    m_queue (queue),
    m_refCount(0),
    m_isVisible(isVisible),
    m_task(task),
    m_callbackFunction(nullptr),
    m_callbackUserData(nullptr),
    m_osSignalTriggered(false)
{
    m_globalSubmitTimeCpu.QuadPart = 0;
    m_submitTimeGpu.QuadPart = 0;
    m_hwStartTimeStamp.QuadPart = 0;
    m_hwEndTimeStamp.QuadPart = 0;
    m_completeTime.QuadPart = 0;
    m_enqueueTime.QuadPart = 0;

    m_kernelNames          = nullptr ;
    m_threadSpace          = nullptr ;
    m_kernelCount          = 0 ;

    MOS_ZeroMemory(&m_surEntryInfoArrays, sizeof(m_surEntryInfoArrays));
}

//*-----------------------------------------------------------------------------
//| Purpose:    Increase Reference count
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::Acquire( void )
{
    ++m_refCount;
    return m_refCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    De of Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::SafeRelease( void )
{
    --m_refCount;
    if(m_refCount == 0 )
    {
        delete this;
        return 0;
    }
    else
    {
        return m_refCount;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmEventRT::~CmEventRT( void )
{
    // call callback registered by Vtune
    if(m_callbackFunction)
    {
        m_callbackFunction(this, m_callbackUserData);
    }

    if (m_surEntryInfoArrays.surfEntryInfosArray!= nullptr)
    {

        for( uint32_t i = 0; i < m_surEntryInfoArrays.kernelNum; i ++ )
        {
            MosSafeDelete(m_surEntryInfoArrays.surfEntryInfosArray[i].surfEntryInfos);
            MosSafeDelete(m_surEntryInfoArrays.surfEntryInfosArray[i].globalSurfInfos);
        }
        MosSafeDelete(m_surEntryInfoArrays.surfEntryInfosArray);
    }

    if (m_kernelNames != nullptr)
    {
        for ( uint32_t i = 0; i < m_kernelCount; i++)
        {
            MosSafeDeleteArray(m_kernelNames[i]);
        }
        MosSafeDeleteArray( m_kernelNames );
        MosSafeDeleteArray( m_threadSpace );
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::Initialize(void)
{
    if( m_taskDriverId == -1 )
        // -1 is an invalid task id in driver, i.e. the task has NOT been passed down to driver yet
        // event is created at the enqueue time, so initial value is CM_STATUS_QUEUED
    {
        m_status = CM_STATUS_QUEUED;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to initialize CmEvent.");
        return CM_FAILURE;
    }

    m_kernelNames = nullptr;
    m_kernelCount = 0;

    return CM_SUCCESS;
}

int32_t CmEventRT::GetStatusNoFlush(CM_STATUS& status)
{
    if ((m_status == CM_STATUS_FLUSHED) || (m_status == CM_STATUS_STARTED))
    {
        Query();
    }
    else if (m_status == CM_STATUS_QUEUED)
    {
        // the task hasn't beeen flushed yet
        // if the task correspoonding to this event can be flushed, m_status will change to CM_STATUS_FLUSHED
        m_queue->FlushTaskWithoutSync();

    }
    else if (m_status == CM_STATUS_FINISHED)
    {
        //Do nothing
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to get status.");
    }

    status = m_status;
    return CM_SUCCESS;
}

int32_t CmEventRT::GetQueue(CmQueueRT *& queue)
{
    queue = m_queue;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//! Query the execution time of a task( one kernel or multiples kernels running concurrently )
//! in the unit of nanoseconds.
//! The execution time is from the time when the task starts to execution in GPU to the time
//! when the task finished execution
//! This is a non-blocking call.
//! INPUT:
//!     Reference to time
//! OUTPUT:
//!     CM_SUCCESS if the execution time is successfully returned
//!     CM_FAILURE if not, e.g. the task hasn't finished
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmEventRT::GetExecutionTime(uint64_t& time)
{
    CM_STATUS eventStatus = CM_STATUS_QUEUED;

    GetStatusNoFlush(eventStatus);

    if( eventStatus == CM_STATUS_FINISHED )
    {
        time = m_time;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

CM_RT_API int32_t CmEventRT::GetExecutionTickTime(uint64_t& ticks)
{
    CM_STATUS eventStatus = CM_STATUS_QUEUED;

    GetStatusNoFlush(eventStatus);

    if (eventStatus == CM_STATUS_FINISHED)
    {
        ticks = m_ticks;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

int32_t CmEventRT::GetSubmitTime(LARGE_INTEGER* time)
{

    CM_STATUS eventStatus = CM_STATUS_QUEUED;

    GetStatusNoFlush(eventStatus);

    if( eventStatus == CM_STATUS_FINISHED )
    {
        *time = m_globalSubmitTimeCpu;
        return CM_SUCCESS;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to get task submit time.");
        return CM_FAILURE;
    }
}

int32_t CmEventRT::GetHWStartTime(LARGE_INTEGER* time)
{
    CM_STATUS eventStatus = CM_STATUS_QUEUED;

    GetStatusNoFlush(eventStatus);

    if( eventStatus == CM_STATUS_FINISHED )
    {
        time->QuadPart = m_globalSubmitTimeCpu.QuadPart + m_hwStartTimeStamp.QuadPart - m_submitTimeGpu.QuadPart;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }

}

uint32_t CmEventRT::GetKernelCount()
{
    return m_kernelCount;
}

int32_t CmEventRT::GetHWEndTime(LARGE_INTEGER* time)
{

    CM_STATUS eventStatus = CM_STATUS_QUEUED;

    GetStatusNoFlush(eventStatus);

    if( eventStatus == CM_STATUS_FINISHED )
    {
        time->QuadPart = m_globalSubmitTimeCpu.QuadPart + m_hwEndTimeStamp.QuadPart - m_submitTimeGpu.QuadPart;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

int32_t CmEventRT::GetCompleteTime(LARGE_INTEGER* time)
{

    CM_STATUS eventStatus = CM_STATUS_QUEUED;

    GetStatusNoFlush(eventStatus);

    if( eventStatus == CM_STATUS_FINISHED )
    {
        *time = m_completeTime;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }

}

int32_t CmEventRT::GetEnqueueTime(LARGE_INTEGER* time)
{
    CM_STATUS eventStatus = CM_STATUS_QUEUED;

    GetStatusNoFlush( eventStatus );

    if ( eventStatus == CM_STATUS_FINISHED )
    {
        *time = m_enqueueTime;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }

}

int32_t CmEventRT::SetKernelNames(CmTaskRT* task, CmThreadSpaceRT* threadSpace, CmThreadGroupSpace* threadGroupSpace)
{
    uint32_t i = 0;
    int32_t hr = CM_SUCCESS;
    uint32_t threadCount;
    m_kernelCount = task->GetKernelCount();

    // Alloc memory for kernel names
    m_kernelNames = MOS_NewArray(char*, m_kernelCount);
    m_threadSpace = MOS_NewArray(uint32_t, (4*m_kernelCount));
    CM_CHK_NULL_GOTOFINISH(m_kernelNames, CM_OUT_OF_HOST_MEMORY);
    CmSafeMemSet(m_kernelNames, 0, m_kernelCount*sizeof(char*) );
    CM_CHK_NULL_GOTOFINISH(m_threadSpace, CM_OUT_OF_HOST_MEMORY);

    for (i = 0; i < m_kernelCount; i++)
    {
        m_kernelNames[i] = MOS_NewArray(char, CM_MAX_KERNEL_NAME_SIZE_IN_BYTE);
        CM_CHK_NULL_GOTOFINISH(m_kernelNames[i], CM_OUT_OF_HOST_MEMORY);
        CmKernelRT* kernel = task->GetKernelPointer(i);
        MOS_SecureStrcpy(m_kernelNames[i], CM_MAX_KERNEL_NAME_SIZE_IN_BYTE, kernel->GetName());

        kernel->GetThreadCount(threadCount);
        m_threadSpace[4 * i] = threadCount;
        m_threadSpace[4 * i + 1] = 1;
        m_threadSpace[4 * i + 2] = threadCount;
        m_threadSpace[4 * i + 3] = 1;
    }

    if (threadSpace)
    {
        uint32_t threadWidth, threadHeight;
        threadSpace->GetThreadSpaceSize(threadWidth, threadHeight);
        m_threadSpace[0] = threadWidth;
        m_threadSpace[1] = threadHeight;
        m_threadSpace[2] = threadWidth;
        m_threadSpace[3] = threadHeight;
    }
    else if (threadGroupSpace)
    {
        uint32_t threadWidth, threadHeight, threadDepth, groupWidth, groupHeight, groupDepth;
        threadGroupSpace->GetThreadGroupSpaceSize(threadWidth, threadHeight, threadDepth, groupWidth, groupHeight, groupDepth);
        m_threadSpace[0] = threadWidth;
        m_threadSpace[1] = threadHeight;
        m_threadSpace[2] = threadWidth * groupWidth;
        m_threadSpace[3] = threadHeight * groupHeight * groupDepth;
    }

finish:
    if(hr == CM_OUT_OF_HOST_MEMORY)
    {
        if(m_kernelNames != nullptr)
        {
            for (uint32_t j = 0; j < m_kernelCount; j++)
            {
                MosSafeDeleteArray(m_kernelNames[j]);
            }
        }
        MosSafeDeleteArray(m_kernelNames);
        MosSafeDeleteArray(m_threadSpace);
    }
    return hr;
}

int32_t CmEventRT::SetEnqueueTime( LARGE_INTEGER time )
{
    m_enqueueTime = time;
    return CM_SUCCESS;
}

int32_t CmEventRT::SetCompleteTime( LARGE_INTEGER time )
{
    m_completeTime = time;
    return CM_SUCCESS;
}

int32_t CmEventRT::GetIndex( uint32_t & index )
{
    index = m_index;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Task ID
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::SetTaskDriverId( int32_t id )
{
    m_taskDriverId = id;
    if( m_taskDriverId > -1 )
        // Valid task id in driver, i.e. the task has been passed down to driver
    {
        m_status = CM_STATUS_FLUSHED;
    }
    else if( m_taskDriverId == -1 )
        // -1 is an invalid task id in driver, i.e. the task has NOT been passed down to driver yet
        // event is created at the enqueue time, so initial value is CM_STATUS_QUEUED
    {
        m_status = CM_STATUS_QUEUED;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to set task driver ID.");
        return CM_FAILURE;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set OS data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::SetTaskOsData( void  *data )
{
    m_osData = data;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Task ID
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::GetTaskDriverId( int32_t & id )
{
    id = m_taskDriverId;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Query status of a task.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::Query( void )
{
    CM_RETURN_CODE  hr = CM_SUCCESS;

    CLock Lock(m_criticalSectionQuery);

    if( ( m_status != CM_STATUS_FLUSHED ) && ( m_status != CM_STATUS_STARTED ) )
    {
        return CM_FAILURE;
    }

    CM_ASSERT( m_taskDriverId > -1 );
    CM_HAL_QUERY_TASK_PARAM param;
    CmSafeMemSet(&param, 0, sizeof(CM_HAL_QUERY_TASK_PARAM));
    param.taskId = m_taskDriverId;
    m_task->GetTaskType(param.taskType);
    param.queueOption = m_queue->GetQueueOption();

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnQueryTask(cmData->cmHalState, &param));

    if( param.status == CM_TASK_FINISHED )
    {
        std::vector<CmQueueRT *> &queue = m_device->GetQueue();

        m_time = param.taskDurationNs;
        m_ticks = param.taskDurationTicks;
        m_hwStartTimeStampInTicks = param.taskHWStartTimeStampInTicks;
        m_hwEndTimeStampInTicks = param.taskHWEndTimeStampInTicks;
        m_status = CM_STATUS_FINISHED;

        //Update the state tracking array when a task is finished

        if (queue.size() == 0)
        {
            CM_ASSERTMESSAGE("Error: Invalid CmQueue.");
            return CM_NULL_POINTER;
        }

        UnreferenceIfNeeded(m_osData);

        CmNotifierGroup *notifiers = m_device->GetNotifiers();
        if (notifiers != nullptr)
        {
            notifiers->NotifyTaskCompleted(m_task);
        }

        m_globalSubmitTimeCpu = param.taskGlobalSubmitTimeCpu;
        m_submitTimeGpu = param.taskSubmitTimeGpu;
        m_hwStartTimeStamp = param.taskHWStartTimeStamp;
        m_hwEndTimeStamp = param.taskHWEndTimeStamp;

        EVENT_LOG(this);
    }
    else if( param.status == CM_TASK_IN_PROGRESS )
    {
        m_status = CM_STATUS_STARTED;
    }
    else if (param.status == CM_TASK_RESET)
    {
        m_status = CM_STATUS_RESET;
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get m_status.
//| Returns:    return m_status.
//*-----------------------------------------------------------------------------
CM_STATUS CmEventRT::GetStatusWithoutFlush()
{
    return m_status;
}

//*-----------------------------------------------------------------------------
//| Purpose:    GT-PIN : Get Surface Details
//| Returns:    result of operation
//*-----------------------------------------------------------------------------
CM_RT_API  int32_t CmEventRT::GetSurfaceDetails(uint32_t kernIndex, uint32_t surfBTI,CM_SURFACE_DETAILS & outDetails )
{
    CM_SURFACE_DETAILS *tempSurfInfo;
    CmSurfaceManager *surfaceMgr;
    m_device->GetSurfaceManager( surfaceMgr);

    if(!m_device->CheckGTPinEnabled())
    {
        CM_ASSERTMESSAGE("Error: Need to enable GT-Pin to call this function.");
        return CM_NOT_IMPLEMENTED;
    }

    if(kernIndex+1>m_surEntryInfoArrays.kernelNum)
    {
        CM_ASSERTMESSAGE("Error: Incorrect kernel Index.");
        return CM_INVALID_ARG_VALUE;
    }
    uint32_t tempIndex=0;

    if (surfaceMgr->IsCmReservedSurfaceIndex(surfBTI))
    {
        tempIndex=surfBTI-CM_GLOBAL_SURFACE_INDEX_START;
        if(tempIndex+1>m_surEntryInfoArrays.surfEntryInfosArray[kernIndex].globalSurfNum)
        {
            CM_ASSERTMESSAGE("Error: Incorrect surface Binding table Index.");
            return CM_INVALID_ARG_VALUE;
        }
        tempSurfInfo = tempIndex +
                        m_surEntryInfoArrays.surfEntryInfosArray[kernIndex].globalSurfInfos;

    }
    else if (surfaceMgr->IsValidSurfaceIndex(surfBTI)) //not static buffer
    {
        if((surfBTI-surfaceMgr->ValidSurfaceIndexStart() +1)>m_surEntryInfoArrays.surfEntryInfosArray[kernIndex].usedIndex)
        {
            CM_ASSERTMESSAGE("Error: Incorrect surface Binding table Index.");
            return CM_INVALID_ARG_VALUE;
        }
        else
        {
            tempIndex = surfBTI - surfaceMgr->ValidSurfaceIndexStart();
        }
        tempSurfInfo = tempIndex +
                    m_surEntryInfoArrays.surfEntryInfosArray[kernIndex].surfEntryInfos;

    }
    else
    {//error
        CM_ASSERTMESSAGE("Error: Incorrect surface Binding table Index.");
        return CM_INVALID_ARG_VALUE;
    }

    CmSafeMemCopy(&outDetails,tempSurfInfo,sizeof(CM_SURFACE_DETAILS));
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    GT-PIN : Set Surface Details
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::SetSurfaceDetails(CM_HAL_SURFACE_ENTRY_INFO_ARRAYS surfaceInfo)
{
    m_surEntryInfoArrays.kernelNum = surfaceInfo.kernelNum;
    m_surEntryInfoArrays.surfEntryInfosArray = (CM_HAL_SURFACE_ENTRY_INFO_ARRAY*)MOS_AllocAndZeroMemory(
                                                                   surfaceInfo.kernelNum *
                                                                   sizeof(CM_HAL_SURFACE_ENTRY_INFO_ARRAY));

    if(m_surEntryInfoArrays.surfEntryInfosArray == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Mem allocation fail.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    for( uint32_t i = 0; i < surfaceInfo.kernelNum; i ++ )
    {
        //non static buffers
        uint32_t surfEntryMax = surfaceInfo.surfEntryInfosArray[i].maxEntryNum;
        uint32_t surfEntryNum = surfaceInfo.surfEntryInfosArray[i].usedIndex;

        m_surEntryInfoArrays.surfEntryInfosArray[i].usedIndex = surfEntryNum;
        m_surEntryInfoArrays.surfEntryInfosArray[i].maxEntryNum = surfEntryMax;
        CM_SURFACE_DETAILS* temp = (CM_SURFACE_DETAILS*)MOS_AllocAndZeroMemory(
                                            surfEntryNum*
                                            sizeof(CM_SURFACE_DETAILS));
        if(temp == nullptr)
        {
            return CM_OUT_OF_HOST_MEMORY;
         }
        else
        {
            m_surEntryInfoArrays.surfEntryInfosArray[i].surfEntryInfos=temp;
            CmSafeMemCopy(m_surEntryInfoArrays.surfEntryInfosArray[i].surfEntryInfos,
                                         surfaceInfo.surfEntryInfosArray[i].surfEntryInfos,
                                         surfEntryNum*sizeof(CM_SURFACE_DETAILS));
         }

        //static buffers
        uint32_t globalSurfNum = surfaceInfo.surfEntryInfosArray[i].globalSurfNum;
        if(globalSurfNum>0)
        {
            m_surEntryInfoArrays.surfEntryInfosArray[i].globalSurfNum = globalSurfNum;
            temp=(CM_SURFACE_DETAILS*)MOS_AllocAndZeroMemory(
                  globalSurfNum*sizeof(CM_SURFACE_DETAILS));
            if(temp == nullptr)
            {
                return CM_OUT_OF_HOST_MEMORY;
             }
            else
            {
                m_surEntryInfoArrays.surfEntryInfosArray[i].globalSurfInfos=temp;
                CmSafeMemCopy(m_surEntryInfoArrays.surfEntryInfosArray[i].globalSurfInfos,
                                             surfaceInfo.surfEntryInfosArray[i].globalSurfInfos,
                                             globalSurfNum*sizeof(CM_SURFACE_DETAILS));
             }
        }//(globalSurfNum>0)
    }//for
    return CM_SUCCESS;
}

CM_RT_API  int32_t CmEventRT::GetProfilingInfo(CM_EVENT_PROFILING_INFO infoType, size_t paramSize, void  *inputValue, void  *value)
{
    int32_t hr = CM_SUCCESS;

    CM_CHK_NULL_GOTOFINISH_CMERROR(value);

    switch(infoType)
    {
        case CM_EVENT_PROFILING_HWSTART:
             CM_CHK_COND_RETURN((paramSize < sizeof(LARGE_INTEGER)), CM_INVALID_PARAM_SIZE, "Invalid parameter size.");
             CM_CHK_CMSTATUS_GOTOFINISH(GetHWStartTime((LARGE_INTEGER *)value));
             break;

        case CM_EVENT_PROFILING_HWEND:
             CM_CHK_COND_RETURN((paramSize < sizeof(LARGE_INTEGER)), CM_INVALID_PARAM_SIZE, "Invalid parameter size.");
             CM_CHK_CMSTATUS_GOTOFINISH(GetHWEndTime((LARGE_INTEGER *)value));
             break;

        case CM_EVENT_PROFILING_SUBMIT:
             CM_CHK_COND_RETURN((paramSize < sizeof(LARGE_INTEGER)), CM_INVALID_PARAM_SIZE, "Invalid parameter size.");
             CM_CHK_CMSTATUS_GOTOFINISH(GetSubmitTime((LARGE_INTEGER *)value));
             break;

        case CM_EVENT_PROFILING_COMPLETE:
            CM_CHK_COND_RETURN((paramSize < sizeof(LARGE_INTEGER)), CM_INVALID_PARAM_SIZE, "Invalid parameter size.");
             CM_CHK_CMSTATUS_GOTOFINISH(GetCompleteTime((LARGE_INTEGER *)value));
             break;

        case CM_EVENT_PROFILING_ENQUEUE:
            CM_CHK_COND_RETURN((paramSize < sizeof(LARGE_INTEGER)), CM_INVALID_PARAM_SIZE, "Invalid parameter size.");
             CM_CHK_CMSTATUS_GOTOFINISH(GetEnqueueTime((LARGE_INTEGER *)value));
             break;

        case CM_EVENT_PROFILING_KERNELCOUNT:
             CM_CHK_COND_RETURN((paramSize < sizeof(uint32_t)), CM_INVALID_PARAM_SIZE, "Invalid parameter size.");
             *(uint32_t *)value =  GetKernelCount();
             break;

        case CM_EVENT_PROFILING_KERNELNAMES:
             {
                 CM_CHK_NULL_GOTOFINISH_CMERROR(inputValue);
                 uint32_t kernelIndex = *(uint32_t *)inputValue;
                 if( kernelIndex >= m_kernelCount)
                 {
                    hr = CM_INVALID_PARAM_SIZE;
                    goto finish;
                 }
                 *((char **)value) = m_kernelNames[kernelIndex];
             }
             break;

        case CM_EVENT_PROFILING_THREADSPACE:
             {
                 CM_CHK_NULL_GOTOFINISH_CMERROR(inputValue);
                 uint32_t kernelIndex = *(uint32_t *)inputValue;
                 if( kernelIndex >= m_kernelCount)
                 {
                    hr = CM_INVALID_PARAM_SIZE;
                    goto finish;
                 }
                 // 4 elements, global/local, width/height,
                 CmSafeMemCopy(value, m_threadSpace + kernelIndex*4 , sizeof(uint32_t)*4);
             }
            break;

        case CM_EVENT_PROFILING_CALLBACK:
            {
                 CM_CHK_NULL_GOTOFINISH_CMERROR(inputValue);
                 CM_CHK_NULL_GOTOFINISH_CMERROR(value);
                 CM_CHK_CMSTATUS_GOTOFINISH(SetCallBack((EventCallBackFunction)inputValue, value));
            }
            break;

        default:
            hr = CM_FAILURE;
    }

finish:
    return hr;
}

int32_t CmEventRT:: SetCallBack(EventCallBackFunction function, void  *userData)
{
    m_callbackFunction = function;
    m_callbackUserData = userData;
    return CM_SUCCESS;
}

#if CM_LOG_ON
std::string CmEventRT::Log(const char *callerFuncName)
{
    static const char *statusStrings[] = {
#define ENUM_STRING(e)  #e
            ENUM_STRING(CM_STATUS_QUEUED),
            ENUM_STRING(CM_STATUS_FLUSHED),
            ENUM_STRING(CM_STATUS_FINISHED),
            ENUM_STRING(CM_STATUS_STARTED),
            ENUM_STRING(CM_STATUS_RESET),
#undef ENUM_STRING
    };

    std::ostringstream  oss;
    oss << callerFuncName << "():\n"
        << "<CmEvent>:" << reinterpret_cast<uint64_t>(this) << "\n"
        << " Status: " << statusStrings[m_status] << "\n"
        << " Duration:" << m_time << "ns\n"
        << " DurationInTick:" << m_ticks << "\n"
        << " StartTimeInTick:" << m_hwStartTimeStampInTicks << "\n"
        << " EndTimeInTick:"<< m_hwEndTimeStampInTicks << "\n"
        << " Kernel Cnt:"<< m_kernelCount << std::endl;

    return oss.str();
}
#endif

}
