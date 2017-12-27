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
int32_t CmEventRT::Create(uint32_t index, CmQueueRT *pQueue, CmTaskInternal *pTask, int32_t taskDriverId, CmDeviceRT *pCmDev, bool isVisible, CmEventRT *&pEvent)
{
    int32_t result = CM_SUCCESS;
    pEvent = new (std::nothrow) CmEventRT( index, pQueue, pTask, taskDriverId, pCmDev, isVisible );
    if( pEvent )
    {
        if(isVisible) 
        {   // Increase the refcount when the Event is visiable
            pEvent->Acquire();
        }
        result = pEvent->Initialize();
        if( result != CM_SUCCESS )
        {
            CmEventRT::Destroy( pEvent );
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
int32_t CmEventRT::Destroy( CmEventRT* &pEvent )
{
    long refCount = pEvent->SafeRelease();
    if( refCount == 0 )
    {
        pEvent = nullptr;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmEventRT::CmEventRT(uint32_t index, CmQueueRT *pQueue, CmTaskInternal *pTask, int32_t taskDriverId, CmDeviceRT *pCmDev, bool isVisible):
    m_Index( index ), 
    m_TaskDriverId( taskDriverId ),
    m_Status( CM_STATUS_QUEUED ),
    m_Time( 0 ),
    m_Ticks(0),
    m_pDevice( pCmDev ),
    m_pQueue (pQueue),
    m_RefCount(0),
    isVisible(isVisible),
    m_pTask(pTask),
    m_OsData(nullptr),
    m_CallbackFunction(nullptr),
    m_CallbackUserData(nullptr)
{
    m_GlobalCMSubmitTime.QuadPart = 0;
    m_CMSubmitTimeStamp.QuadPart = 0;
    m_HWStartTimeStamp.QuadPart = 0;
    m_HWEndTimeStamp.QuadPart = 0;               
    m_CompleteTime.QuadPart = 0;                   
    m_EnqueueTime.QuadPart = 0;                 

    m_KernelNames          = nullptr ;
    m_ThreadSpace          = nullptr ;
    m_KernelCount          = 0 ;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Increase Reference count 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::Acquire( void )
{
    ++m_RefCount;
    return m_RefCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    De of Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::SafeRelease( void )
{
    --m_RefCount;
    if(m_RefCount == 0 )
    {
        delete this;
        return 0;
    }
    else
    {
        return m_RefCount;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of Cm Event
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmEventRT::~CmEventRT( void )
{
    // call callback registered by Vtune
    if(m_CallbackFunction)
    {
        m_CallbackFunction(this, m_CallbackUserData);
    }

    if (m_SurEntryInfoArrays.pSurfEntryInfosArray!= nullptr)
    {

        for( uint32_t i = 0; i < m_SurEntryInfoArrays.dwKrnNum; i ++ )
        {
            MosSafeDelete(m_SurEntryInfoArrays.pSurfEntryInfosArray[i].pSurfEntryInfos);
            MosSafeDelete(m_SurEntryInfoArrays.pSurfEntryInfosArray[i].pGlobalSurfInfos);
        }
        MosSafeDelete(m_SurEntryInfoArrays.pSurfEntryInfosArray);
    }

    if (m_KernelNames != nullptr)
    {
        for ( uint32_t i = 0; i < m_KernelCount; i++)
        {
            MosSafeDeleteArray(m_KernelNames[i]);
        }
        MosSafeDeleteArray( m_KernelNames );
        MosSafeDeleteArray( m_ThreadSpace );
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Cm Event 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::Initialize(void)
{
    CmSafeMemSet(&m_SurEntryInfoArrays, 0, sizeof(CM_HAL_SURFACE_ENTRY_INFO_ARRAYS));
    if( m_TaskDriverId == -1 )
        // -1 is an invalid task id in driver, i.e. the task has NOT been passed down to driver yet
        // event is created at the enqueue time, so initial value is CM_STATUS_QUEUED
    {
        m_Status = CM_STATUS_QUEUED;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to initialize CmEvent.");
        return CM_FAILURE;
    }

    m_KernelNames = nullptr;
    m_KernelCount = 0;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//! Query the status of the task associated with the event
//! An event is generated when a task ( one kernel or multiples kernels running concurrently )
//! is enqueued.
//! This is a non-blocking call.
//! INPUT:
//!     The reference to status. For now only two status, CM_STATUS_QUEUED and CM_STATUS_FINISHED, are supported
//! OUTPUT:
//!     CM_SUCCESS if the status is successfully returned;
//!     CM_FAILURE if not.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmEventRT::GetStatus( CM_STATUS& status) 
{
    if( ( m_Status == CM_STATUS_FLUSHED ) || ( m_Status == CM_STATUS_STARTED ) )
    {
        Query();
    }

    m_pQueue->FlushTaskWithoutSync();

    status = m_Status; 
    return CM_SUCCESS;
}

int32_t CmEventRT::GetStatusNoFlush(CM_STATUS& status)
{
    if ((m_Status == CM_STATUS_FLUSHED) || (m_Status == CM_STATUS_STARTED))
    {
        Query();
    }
    else if (m_Status == CM_STATUS_QUEUED)
    {
        // the task hasn't beeen flushed yet
        // if the task correspoonding to this event can be flushed, m_Status will change to CM_STATUS_FLUSHED
        m_pQueue->FlushTaskWithoutSync();

    }
    else if (m_Status == CM_STATUS_FINISHED)
    {
        //Do nothing
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to get status.");
    }

    status = m_Status;
    return CM_SUCCESS;
}

int32_t CmEventRT::GetQueue(CmQueueRT *& pQueue)
{
    pQueue = m_pQueue;
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
    CM_STATUS EventStatus = CM_STATUS_QUEUED; 
    
    GetStatusNoFlush(EventStatus);

    if( EventStatus == CM_STATUS_FINISHED )
    {
        time = m_Time;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

CM_RT_API int32_t CmEventRT::GetExecutionTickTime(uint64_t& ticks)
{
    CM_STATUS EventStatus = CM_STATUS_QUEUED;

    GetStatusNoFlush(EventStatus);

    if (EventStatus == CM_STATUS_FINISHED)
    {
        ticks = m_Ticks;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

int32_t CmEventRT::GetSubmitTime(LARGE_INTEGER* pTime)
{

    CM_STATUS EventStatus = CM_STATUS_QUEUED; 
    
    GetStatusNoFlush(EventStatus);
    
    if( EventStatus == CM_STATUS_FINISHED )
    {
        *pTime = m_GlobalCMSubmitTime;
        return CM_SUCCESS;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to get task submit time.");
        return CM_FAILURE;
    }
}

int32_t CmEventRT::GetHWStartTime(LARGE_INTEGER* pTime)
{
    CM_STATUS EventStatus = CM_STATUS_QUEUED; 
    
    GetStatusNoFlush(EventStatus);
    
    if( EventStatus == CM_STATUS_FINISHED )
    {
        pTime->QuadPart = m_GlobalCMSubmitTime.QuadPart + m_HWStartTimeStamp.QuadPart - m_CMSubmitTimeStamp.QuadPart;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
    
}


uint32_t CmEventRT::GetKernelCount()
{
    return m_KernelCount;
}

int32_t CmEventRT::GetHWEndTime(LARGE_INTEGER* pTime)
{

    CM_STATUS EventStatus = CM_STATUS_QUEUED; 
    
    GetStatusNoFlush(EventStatus);
    
    if( EventStatus == CM_STATUS_FINISHED )
    {
        pTime->QuadPart = m_GlobalCMSubmitTime.QuadPart + m_HWEndTimeStamp.QuadPart - m_CMSubmitTimeStamp.QuadPart;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
}

int32_t CmEventRT::GetCompleteTime(LARGE_INTEGER* pTime)
{

    CM_STATUS EventStatus = CM_STATUS_QUEUED; 
    
    GetStatusNoFlush(EventStatus);
    
    if( EventStatus == CM_STATUS_FINISHED )
    {
        *pTime = m_CompleteTime;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }
    
}

int32_t CmEventRT::GetEnqueueTime(LARGE_INTEGER* pTime)
{
    CM_STATUS EventStatus = CM_STATUS_QUEUED;

    GetStatusNoFlush( EventStatus );

    if ( EventStatus == CM_STATUS_FINISHED )
    {
        *pTime = m_EnqueueTime;
        return CM_SUCCESS;
    }
    else
    {
        return CM_FAILURE;
    }

}


int32_t CmEventRT::SetKernelNames(CmTaskRT* pTask, CmThreadSpaceRT* pThreadSpace, CmThreadGroupSpace* pThreadGroupSpace)
{
    uint32_t i = 0;
    int32_t hr = CM_SUCCESS;
    uint32_t ThreadCount;
    m_KernelCount = pTask->GetKernelCount();

    // Alloc memory for kernel names
    m_KernelNames = MOS_NewArray(char*, m_KernelCount);
    m_ThreadSpace = MOS_NewArray(uint32_t, (4*m_KernelCount));
    CMCHK_NULL_RETURN(m_KernelNames, CM_OUT_OF_HOST_MEMORY);
    CmSafeMemSet(m_KernelNames, 0, m_KernelCount*sizeof(char*) );
    CMCHK_NULL_RETURN(m_ThreadSpace, CM_OUT_OF_HOST_MEMORY);
    
    for (i = 0; i < m_KernelCount; i++)
    {
        m_KernelNames[i] = MOS_NewArray(char, CM_MAX_KERNEL_NAME_SIZE_IN_BYTE);
        CMCHK_NULL_RETURN(m_KernelNames[i], CM_OUT_OF_HOST_MEMORY);
        CmKernelRT* pKernel = pTask->GetKernelPointer(i);
        MOS_SecureStrcpy(m_KernelNames[i], CM_MAX_KERNEL_NAME_SIZE_IN_BYTE, pKernel->GetName());

        pKernel->GetThreadCount(ThreadCount);
        m_ThreadSpace[4 * i] = ThreadCount;
        m_ThreadSpace[4 * i + 1] = 1;
        m_ThreadSpace[4 * i + 2] = ThreadCount;
        m_ThreadSpace[4 * i + 3] = 1;
    }

    if (pThreadSpace)
    {
        uint32_t ThreadWidth, ThreadHeight;
        pThreadSpace->GetThreadSpaceSize(ThreadWidth, ThreadHeight);
        m_ThreadSpace[0] = ThreadWidth;
        m_ThreadSpace[1] = ThreadHeight;
        m_ThreadSpace[2] = ThreadWidth;
        m_ThreadSpace[3] = ThreadHeight;  
    }
    else if (pThreadGroupSpace)
    {
        uint32_t ThreadWidth, ThreadHeight, ThreadDepth, GroupWidth, GroupHeight, GroupDepth;
        pThreadGroupSpace->GetThreadGroupSpaceSize(ThreadWidth, ThreadHeight, ThreadDepth, GroupWidth, GroupHeight, GroupDepth);
        m_ThreadSpace[0] = ThreadWidth;
        m_ThreadSpace[1] = ThreadHeight;
        m_ThreadSpace[2] = ThreadWidth * GroupWidth;
        m_ThreadSpace[3] = ThreadHeight * GroupHeight * GroupDepth;
    }

finish:
    if(hr == CM_OUT_OF_HOST_MEMORY)
    {
        if(m_KernelNames != nullptr)
        {
            for (uint32_t j = 0; j < m_KernelCount; j++)
            {
                MosSafeDeleteArray(m_KernelNames[j]);
            }
        }
        MosSafeDeleteArray(m_KernelNames);
        MosSafeDeleteArray(m_ThreadSpace);
    }
    return hr;
}

int32_t CmEventRT::SetEnqueueTime( LARGE_INTEGER time )
{
    m_EnqueueTime = time;
    return CM_SUCCESS;
}

int32_t CmEventRT::SetCompleteTime( LARGE_INTEGER time )
{
    m_CompleteTime = time;
    return CM_SUCCESS;
}

int32_t CmEventRT::GetIndex( uint32_t & index )
{
    index = m_Index;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Task ID 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::SetTaskDriverId( int32_t id )
{
    m_TaskDriverId = id;
    if( m_TaskDriverId > -1 ) 
        // Valid task id in driver, i.e. the task has been passed down to driver
    {
        m_Status = CM_STATUS_FLUSHED;
    }
    else if( m_TaskDriverId == -1 )
        // -1 is an invalid task id in driver, i.e. the task has NOT been passed down to driver yet
        // event is created at the enqueue time, so initial value is CM_STATUS_QUEUED
    {
        m_Status = CM_STATUS_QUEUED;
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
    m_OsData = data;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Task ID 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::GetTaskDriverId( int32_t & id )
{
    id = m_TaskDriverId;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Query status of a task.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::Query( void )
{
    CM_RETURN_CODE  hr = CM_SUCCESS;

    CLock Lock(m_CriticalSection_Query);
    
    if( ( m_Status != CM_STATUS_FLUSHED ) && ( m_Status != CM_STATUS_STARTED ) ) 
    {
        return CM_FAILURE;
    }

    CM_ASSERT( m_TaskDriverId > -1 );
    CM_HAL_QUERY_TASK_PARAM param;
    CmSafeMemSet(&param, 0, sizeof(CM_HAL_QUERY_TASK_PARAM));
    param.iTaskId = m_TaskDriverId;
    m_pTask->GetTaskType(param.uiTaskType);
    param.queueOption = m_pQueue->GetQueueOption();

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)m_pDevice->GetAccelData();

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->pCmHalState->pfnQueryTask(pCmData->pCmHalState, &param));

    if( param.status == CM_TASK_FINISHED )
    {
        std::vector<CmQueueRT *> &pQueue = m_pDevice->GetQueue();

        m_Time = param.iTaskDuration;
        m_Ticks = param.iTaskTickDuration;
        m_Status = CM_STATUS_FINISHED;

        //Update the state tracking array when a task is finished

        if (pQueue.size() == 0)
        {
            CM_ASSERTMESSAGE("Error: Invalid CmQueue.");
            return CM_NULL_POINTER;
        }

        UnreferenceIfNeeded(m_OsData);

        m_GlobalCMSubmitTime = param.iTaskGlobalCMSubmitTime;
        m_CMSubmitTimeStamp = param.iTaskCMSubmitTimeStamp;
        m_HWStartTimeStamp = param.iTaskHWStartTimeStamp;
        m_HWEndTimeStamp = param.iTaskHWEndTimeStamp;
      
    }
    else if( param.status == CM_TASK_IN_PROGRESS )
    {
        m_Status = CM_STATUS_STARTED;
    }
    else if (param.status == CM_TASK_RESET)
    {
        m_Status = CM_STATUS_RESET;
    }

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    GT-PIN : Get Surface Details 
//| Returns:    result of operation
//*-----------------------------------------------------------------------------
CM_RT_API  int32_t CmEventRT::GetSurfaceDetails(uint32_t kernIndex, uint32_t surfBTI,CM_SURFACE_DETAILS & OutDetails )
{
    CM_SURFACE_DETAILS *pTempSurfInfo;
    CmSurfaceManager *pSurfaceMgr;
    m_pDevice->GetSurfaceManager( pSurfaceMgr);

    if(!m_pDevice->CheckGTPinEnabled())
    {
        CM_ASSERTMESSAGE("Error: Need to enable GT-Pin to call this function.");
        return CM_NOT_IMPLEMENTED;
    }

    if(kernIndex+1>m_SurEntryInfoArrays.dwKrnNum)
    {
        CM_ASSERTMESSAGE("Error: Incorrect kernel Index.");
        return CM_INVALID_ARG_VALUE;
    }
    uint32_t tempIndex=0;

    if (pSurfaceMgr->IsCmReservedSurfaceIndex(surfBTI))
    {
        tempIndex=surfBTI-CM_GLOBAL_SURFACE_INDEX_START;
        if(tempIndex+1>m_SurEntryInfoArrays.pSurfEntryInfosArray[kernIndex].dwGlobalSurfNum)
        {
            CM_ASSERTMESSAGE("Error: Incorrect surface Binding table Index.");
            return CM_INVALID_ARG_VALUE;
        }
        pTempSurfInfo = tempIndex + 
                        m_SurEntryInfoArrays.pSurfEntryInfosArray[kernIndex].pGlobalSurfInfos;

    }
    else if (pSurfaceMgr->IsValidSurfaceIndex(surfBTI)) //not static buffer
    {
        if((surfBTI-pSurfaceMgr->ValidSurfaceIndexStart() +1)>m_SurEntryInfoArrays.pSurfEntryInfosArray[kernIndex].dwUsedIndex)
        {
            CM_ASSERTMESSAGE("Error: Incorrect surface Binding table Index.");
            return CM_INVALID_ARG_VALUE;
        }
        else
        {
            tempIndex = surfBTI - pSurfaceMgr->ValidSurfaceIndexStart();
        }
        pTempSurfInfo = tempIndex +
                    m_SurEntryInfoArrays.pSurfEntryInfosArray[kernIndex].pSurfEntryInfos;

    }
    else 
    {//error
        CM_ASSERTMESSAGE("Error: Incorrect surface Binding table Index.");
        return CM_INVALID_ARG_VALUE;
    }

    CmFastMemCopy(&OutDetails,pTempSurfInfo,sizeof(CM_SURFACE_DETAILS));
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    GT-PIN : Set Surface Details
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmEventRT::SetSurfaceDetails(CM_HAL_SURFACE_ENTRY_INFO_ARRAYS SurfaceInfo)
{
    m_SurEntryInfoArrays.dwKrnNum=SurfaceInfo.dwKrnNum;
    m_SurEntryInfoArrays.pSurfEntryInfosArray= (CM_HAL_SURFACE_ENTRY_INFO_ARRAY*)MOS_AllocAndZeroMemory(
                                                                   SurfaceInfo.dwKrnNum *
                                                                   sizeof(CM_HAL_SURFACE_ENTRY_INFO_ARRAY));

    if(m_SurEntryInfoArrays.pSurfEntryInfosArray == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Mem allocation fail.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    for( uint32_t i = 0; i < SurfaceInfo.dwKrnNum; i ++ )
    {
        //non static buffers
        uint32_t iSurfEntryMax = SurfaceInfo.pSurfEntryInfosArray[i].dwMaxEntryNum;
        uint32_t iSurfEntryNum = SurfaceInfo.pSurfEntryInfosArray[i].dwUsedIndex;

        m_SurEntryInfoArrays.pSurfEntryInfosArray[i].dwUsedIndex = iSurfEntryNum;
        m_SurEntryInfoArrays.pSurfEntryInfosArray[i].dwMaxEntryNum = iSurfEntryMax;
        CM_SURFACE_DETAILS* pTemp = (CM_SURFACE_DETAILS*)MOS_AllocAndZeroMemory(
                                            iSurfEntryNum*
                                            sizeof(CM_SURFACE_DETAILS));
        if(pTemp == nullptr)
        {
            return CM_OUT_OF_HOST_MEMORY;
         }
        else
        {
            m_SurEntryInfoArrays.pSurfEntryInfosArray[i].pSurfEntryInfos=pTemp;
            CmFastMemCopy(m_SurEntryInfoArrays.pSurfEntryInfosArray[i].pSurfEntryInfos,
                                         SurfaceInfo.pSurfEntryInfosArray[i].pSurfEntryInfos,
                                         iSurfEntryNum*sizeof(CM_SURFACE_DETAILS)); 
         }

        //static buffers
        uint32_t iGloSurfNum = SurfaceInfo.pSurfEntryInfosArray[i].dwGlobalSurfNum;
        if(iGloSurfNum>0)
        {
            m_SurEntryInfoArrays.pSurfEntryInfosArray[i].dwGlobalSurfNum=iGloSurfNum;
            pTemp=(CM_SURFACE_DETAILS*)MOS_AllocAndZeroMemory(
                  iGloSurfNum*sizeof(CM_SURFACE_DETAILS));
            if(pTemp == nullptr)
            {
                return CM_OUT_OF_HOST_MEMORY;
             }
            else
            {
                m_SurEntryInfoArrays.pSurfEntryInfosArray[i].pGlobalSurfInfos=pTemp;
                CmFastMemCopy(m_SurEntryInfoArrays.pSurfEntryInfosArray[i].pGlobalSurfInfos,
                                             SurfaceInfo.pSurfEntryInfosArray[i].pGlobalSurfInfos,
                                             iGloSurfNum*sizeof(CM_SURFACE_DETAILS)); 
             }
        }//(iGloSurfNum>0)
    }//for
    return CM_SUCCESS;
}

CM_RT_API  int32_t CmEventRT::GetProfilingInfo(CM_EVENT_PROFILING_INFO infoType, size_t paramSize, void  *pInputValue, void  *pValue)
{
    int32_t hr = CM_SUCCESS;

    CHK_NULL(pValue);

    switch(infoType)
    {
        case CM_EVENT_PROFILING_HWSTART:
             CM_CHK_LESS_THAN(paramSize, sizeof(LARGE_INTEGER), CM_INVALID_PARAM_SIZE);
             CMCHK_HR(GetHWStartTime((LARGE_INTEGER *)pValue));
             break;

        case CM_EVENT_PROFILING_HWEND:
             CM_CHK_LESS_THAN(paramSize, sizeof(LARGE_INTEGER), CM_INVALID_PARAM_SIZE);
             CMCHK_HR(GetHWEndTime((LARGE_INTEGER *)pValue));
             break;

        case CM_EVENT_PROFILING_SUBMIT:
             CM_CHK_LESS_THAN(paramSize, sizeof(LARGE_INTEGER), CM_INVALID_PARAM_SIZE);
             CMCHK_HR(GetSubmitTime((LARGE_INTEGER *)pValue));
             break;

        case CM_EVENT_PROFILING_COMPLETE:
             CM_CHK_LESS_THAN(paramSize, sizeof(LARGE_INTEGER), CM_INVALID_PARAM_SIZE);
             CMCHK_HR(GetCompleteTime((LARGE_INTEGER *)pValue));
             break;

        case CM_EVENT_PROFILING_ENQUEUE:
             CM_CHK_LESS_THAN(paramSize, sizeof(LARGE_INTEGER), CM_INVALID_PARAM_SIZE);
             CMCHK_HR(GetEnqueueTime((LARGE_INTEGER *)pValue));
             break;

        case CM_EVENT_PROFILING_KERNELCOUNT:
             CM_CHK_LESS_THAN(paramSize, sizeof(uint32_t), CM_INVALID_PARAM_SIZE);
             *(uint32_t *)pValue =  GetKernelCount();
             break;

        case CM_EVENT_PROFILING_KERNELNAMES:
             {
                 CHK_NULL(pInputValue);
                 uint32_t KernelIndex = *(uint32_t *)pInputValue;
                 if( KernelIndex >= m_KernelCount)
                 {
                    hr = CM_INVALID_PARAM_SIZE;
                    goto finish;
                 }
                 *((char **)pValue) = m_KernelNames[KernelIndex];
             }
             break;
             
        case CM_EVENT_PROFILING_THREADSPACE:
             {
                 CHK_NULL(pInputValue);
                 uint32_t KernelIndex = *(uint32_t *)pInputValue;
                 if( KernelIndex >= m_KernelCount)
                 {
                    hr = CM_INVALID_PARAM_SIZE;
                    goto finish;
                 }
                 // 4 elements, global/local, width/height, 
                 CmSafeMemCopy(pValue, m_ThreadSpace + KernelIndex*4 , sizeof(uint32_t)*4); 
             }
            break;

        case CM_EVENT_PROFILING_CALLBACK:
            {
                 CHK_NULL(pInputValue);
                 CHK_NULL(pValue);
                 CMCHK_HR(SetCallBack((EventCallBackFunction)pInputValue, pValue));
            }
            break;

        default:
            hr = CM_FAILURE;
    }

finish:
    return hr;
}

int32_t CmEventRT:: SetCallBack(EventCallBackFunction function, void  *user_data)
{
    m_CallbackFunction = function;
    m_CallbackUserData = user_data;
    return CM_SUCCESS;
}
}