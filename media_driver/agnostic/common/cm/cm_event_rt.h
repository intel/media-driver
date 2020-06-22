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
#include "cm_log.h"

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
                          CmQueueRT *queue,
                          CmTaskInternal *task,
                          int32_t taskDriverId,
                          CmDeviceRT *device,
                          bool isVisible,
                          CmEventRT *&event);

    static int32_t Destroy(CmEventRT *&event);

    CM_RT_API int32_t GetStatus(CM_STATUS &status);

    CM_RT_API int32_t GetExecutionTime(uint64_t &time);

    CM_RT_API int32_t
    WaitForTaskFinished(uint32_t timeOutMs = CM_MAX_TIMEOUT_MS);

    CM_RT_API int32_t GetSurfaceDetails(uint32_t kernIndex,
                                        uint32_t surfBTI,
                                        CM_SURFACE_DETAILS &outDetails);

    CM_RT_API int32_t GetProfilingInfo(CM_EVENT_PROFILING_INFO infoType,
                                       size_t paramSize,
                                       void *inputValue,
                                       void *value);

    CM_RT_API int32_t GetExecutionTickTime(uint64_t &ticks);

    int32_t GetIndex(uint32_t &index);

    int32_t SetTaskDriverId(int32_t id);

    int32_t GetTaskDriverId(int32_t &id);

    int32_t SetTaskOsData(void *data);

    int32_t SetSurfaceDetails(CM_HAL_SURFACE_ENTRY_INFO_ARRAYS surfaceInfo);

    int32_t Acquire(void);

    int32_t SafeRelease(void);

    int32_t SetKernelNames(CmTaskRT *task,
                           CmThreadSpaceRT *threadSpace,
                           CmThreadGroupSpace *threadGroupSpace);

    int32_t SetEnqueueTime(LARGE_INTEGER time);

    int32_t SetCompleteTime(LARGE_INTEGER time);

    int32_t GetSubmitTime(LARGE_INTEGER *time);

    int32_t GetHWStartTime(LARGE_INTEGER *time);

    int32_t GetHWEndTime(LARGE_INTEGER *time);

    int32_t GetCompleteTime(LARGE_INTEGER *time);

    uint32_t GetKernelCount();

    int32_t GetEnqueueTime(LARGE_INTEGER *time);

    int32_t SetCallBack(EventCallBackFunction function, void *userData);

    int32_t GetStatusNoFlush(CM_STATUS &status);

    int32_t ModifyStatus(CM_STATUS status, uint64_t elapsedTime);

    int32_t GetQueue(CmQueueRT *&queue);

    int32_t Query();

    CM_STATUS GetStatusWithoutFlush();

#if CM_LOG_ON
    std::string Log(const char *callerFuncName);
#endif

protected:
    CmEventRT(uint32_t index,
              CmQueueRT *queue,
              CmTaskInternal *task,
              int32_t taskDriverId,
              CmDeviceRT *device,
              bool isVisible);

    ~CmEventRT();

    int32_t Initialize();

    void UnreferenceIfNeeded(void *pdata);

    uint32_t m_index;
    int32_t m_taskDriverId;
    void *m_osData;

    CM_STATUS m_status;
    uint64_t m_time;
    uint64_t m_ticks;
    uint64_t m_hwStartTimeStampInTicks;
    uint64_t m_hwEndTimeStampInTicks;

    LARGE_INTEGER m_globalSubmitTimeCpu; // The CM task submission time in CPU
    LARGE_INTEGER m_submitTimeGpu;       // The CM task submission time in GPU
    LARGE_INTEGER m_hwStartTimeStamp;    // The task start execution time in GPU
    LARGE_INTEGER m_hwEndTimeStamp;      // The task end execution time in GPU
    LARGE_INTEGER m_completeTime;        // The task complete time in CPU
    LARGE_INTEGER m_enqueueTime;         // The time when the task is pushed into enqueued
                                         //  queue

    char **m_kernelNames;
    uint32_t *m_threadSpace;
    uint32_t m_kernelCount;

    CmDeviceRT* m_device;
    CmQueueRT *m_queue;

    int32_t m_refCount;

    bool m_isVisible;  // if the event is Visible to user or not

    CM_HAL_SURFACE_ENTRY_INFO_ARRAYS m_surEntryInfoArrays;
    CmTaskInternal *m_task;

    CSync m_criticalSectionQuery;

    //Vtune call back
    EventCallBackFunction m_callbackFunction;  //CallBack Function
    void *m_callbackUserData;                  //Pdata for Callback

private:
    bool m_osSignalTriggered;

    CmEventRT(const CmEventRT& other);
    CmEventRT& operator=(const CmEventRT& other);
};
}

#endif  // MEDIADRIVER_AGNOSTIC_COMMON_CM_CMEVENTRT_H_
