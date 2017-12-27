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
//! \file      cm_event_rt.h
//! \brief     Contains CmEventRT declarations.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMEVENTRT_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMEVENTRT_H_

#include "cm_event.h"
#include "cm_csync.h"
#include "cm_hal.h"

namespace CMRT_UMD
{
class CmDeviceRT;
class CmEventRT;
class CmQueueRT;
class CmTaskRT;
class CmTaskInternal;
class CmThreadGroupSpace;
class CmThreadSpaceRT;
};

#define CM_CALLBACK __cdecl
typedef void (CM_CALLBACK *EventCallBackFunction)(CMRT_UMD::CmEventRT*, void*);

namespace CMRT_UMD
{
class CmEventRT: public CmEvent
{
public:
    static int32_t Create(uint32_t index,
                          CmQueueRT *pQueue,
                          CmTaskInternal *pTask,
                          int32_t taskDriverId,
                          CmDeviceRT *pCmDev,
                          bool isVisible,
                          CmEventRT *&pEvent);

    static int32_t Destroy(CmEventRT *&pEvent);

    CM_RT_API int32_t GetStatus(CM_STATUS &status);

    CM_RT_API int32_t GetExecutionTime(uint64_t &time);

    CM_RT_API int32_t
    WaitForTaskFinished(uint32_t dwTimeOutMs = CM_MAX_TIMEOUT_MS);

    CM_RT_API int32_t GetSurfaceDetails(uint32_t kernIndex,
                                        uint32_t surfBTI,
                                        CM_SURFACE_DETAILS &outDetails);

    CM_RT_API int32_t GetProfilingInfo(CM_EVENT_PROFILING_INFO infoType,
                                       size_t paramSize,
                                       void *pInputValue,
                                       void *pValue);

    CM_RT_API int32_t GetExecutionTickTime(uint64_t &ticks);

    int32_t GetIndex(uint32_t &index);

    int32_t SetTaskDriverId(int32_t id);

    int32_t GetTaskDriverId(int32_t &id);

    int32_t SetTaskOsData(void *data);

    int32_t SetSurfaceDetails(CM_HAL_SURFACE_ENTRY_INFO_ARRAYS SurfaceInfo);

    int32_t Acquire(void);

    int32_t SafeRelease(void);

    int32_t SetKernelNames(CmTaskRT *pTask,
                           CmThreadSpaceRT *pThreadSpace,
                           CmThreadGroupSpace *pThreadGroupSpace);

    int32_t SetEnqueueTime(LARGE_INTEGER time);

    int32_t SetCompleteTime(LARGE_INTEGER time);

    int32_t GetSubmitTime(LARGE_INTEGER *pTime);

    int32_t GetHWStartTime(LARGE_INTEGER *pTime);

    int32_t GetHWEndTime(LARGE_INTEGER *pTime);

    int32_t GetCompleteTime(LARGE_INTEGER *pTime);

    uint32_t GetKernelCount();

    int32_t GetEnqueueTime(LARGE_INTEGER *pTime);

    int32_t SetCallBack(EventCallBackFunction function, void *user_data);

    int32_t GetStatusNoFlush(CM_STATUS &status);

    int32_t GetQueue(CmQueueRT *&pQueue);

protected:
    CmEventRT(uint32_t index,
              CmQueueRT *pQueue,
              CmTaskInternal *pTask,
              int32_t taskDriverId,
              CmDeviceRT *pCmDev,
              bool isVisible);

    ~CmEventRT();

    int32_t Initialize();

    int32_t Query();

    void UnreferenceIfNeeded(void *pdata);

    uint32_t m_Index;
    int32_t m_TaskDriverId;
    void *m_OsData;

    CM_STATUS m_Status;
    uint64_t m_Time;
    uint64_t m_Ticks;

    LARGE_INTEGER m_GlobalCMSubmitTime;  // The CM task submission time in CPU
    LARGE_INTEGER m_CMSubmitTimeStamp;   // The CM task submission time in GPU
    LARGE_INTEGER m_HWStartTimeStamp;    // The task start execution time in GPU
    LARGE_INTEGER m_HWEndTimeStamp;      // The task end execution time in GPU
    LARGE_INTEGER m_CompleteTime;        // The task complete time in CPU
    LARGE_INTEGER m_EnqueueTime;         // The time when the task is pushed into enqueued
                                         //  queue

    char **m_KernelNames;
    uint32_t *m_ThreadSpace;
    uint32_t m_KernelCount;

    CmDeviceRT* m_pDevice;
    CmQueueRT *m_pQueue;

    int32_t m_RefCount;

    bool isVisible;  // if the event is Visible to user or not

    CM_HAL_SURFACE_ENTRY_INFO_ARRAYS m_SurEntryInfoArrays;
    CmTaskInternal *m_pTask;

    CSync m_CriticalSection_Query;

    //Vtune call back
    EventCallBackFunction m_CallbackFunction;  //CallBack Function
    void *m_CallbackUserData;                  //Pdata for Callback
};
}

#endif  // MEDIADRIVER_AGNOSTIC_COMMON_CM_CMEVENTRT_H_
