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
#include "cm_device.h"
#include "cm_event_base.h"
#include "cm_queue_base.h"
#include "cm_timer.h"

//!
//! \brief      Destroys CM Device
//! \details    Destroys the CM Device. Also destroys surfaces,
//!             kernels, samplers and the queue that was created
//!             using this device instance that haven't already been
//!             explicitly destroyed by calling respective destroy functions.
//! \param      [in] device
//!             Reference to the pointer to the CmDevice to be destroyed
//! \retval     CM_SUCCESS if device successfully destroyed
//! \retval     CM_FAILURE if failed to destroy device
//!
extern "C"
CM_RT_API int32_t DestroyCmDevice(CmDevice* &device)
{
    INSERT_PROFILER_RECORD();

    CmDevice_RT* p = static_cast<CmDevice_RT*>(device);
    int32_t result = CmDevice_RT::Destroy(p);

    if(result == CM_SUCCESS)
    {
        device = nullptr;
    }

    return result;
}

//!
//! \brief      Get kernel count
//! \details    VTune helper function to return the number of kernels
//!             associated with a CmEvent
//! \param      [in] event
//!             Pointer to the CmEvent
//! \return     number of kernels associated with the CmEvent
//!
EXTERN_C CM_RT_API uint32_t CMRT_GetKernelCount(CmEvent *event)
{
    INSERT_PROFILER_RECORD();

    uint32_t KernelCount = 0;
    event->GetProfilingInfo(CM_EVENT_PROFILING_KERNELCOUNT, sizeof(uint32_t), nullptr, &KernelCount);
    return KernelCount;
}

//!
//! \brief      Get kernel name
//! \details    VTune helper function to return the name of the Nth kernel
//!             associated with the CmEvent
//! \param      [in] event
//!             Pointer to the CmEvent
//! \param      [in] index
//!             Index of the kernel
//! \param      [out] KernelName
//!             Pointer to the kernel name
//! \retval     CM_SUCCESS            if successfully return kernel name
//! \retval     CM_INVALID_PARAM_SIZE if invalid index
//!
EXTERN_C CM_RT_API int32_t CMRT_GetKernelName(CmEvent *event, uint32_t index, char** KernelName)
{
    INSERT_PROFILER_RECORD();

    return event->GetProfilingInfo(CM_EVENT_PROFILING_KERNELNAMES, sizeof(char *), &index, KernelName);
}

//!
//! \brief      Get kernel thread space
//! \details    VTune helper function to return the thread space dimensions
//!             for the Nth kernel associated with the CmEvent.
//!             Global width and global height are only valid for 
//!             threadgroupspace (used with GPGPU walker)
//! \param      [in] event
//!             Pointer to the CmEvent
//! \param      [in] index
//!             Index of the kernel
//! \param      [out] localWidth
//!             Pointer to the kernel's thread space local width
//! \param      [out] localHeight
//!             Pointer to the kernel's thread space local height
//! \param      [out] globalWidth
//!             Pointer to the kernel's thread space global width
//! \param      [out] globalHeight
//!             Pointer to the kernel's thread space global height
//! \return     CM_SUCCESS
//!
EXTERN_C CM_RT_API int32_t CMRT_GetKernelThreadSpace(CmEvent *event, uint32_t index, uint32_t *localWidth, uint32_t *localHeight, uint32_t *globalWidth, uint32_t *globalHeight)
{
    INSERT_PROFILER_RECORD();

    uint32_t tsinfo[4];
    event->GetProfilingInfo(CM_EVENT_PROFILING_THREADSPACE, sizeof(uint32_t)*4, &index, tsinfo);
    *localWidth = tsinfo[0];
    *localHeight = tsinfo[1];
    *globalWidth = tsinfo[2];
    *globalHeight = tsinfo[3];
    return CM_SUCCESS;
}

//!
//! \brief      Get submit time
//! \details    VTune helper function to return the CPU side submit timestamp
//! \param      [in] event
//!             Pointer to the CmEvent
//! \param      [out] time
//!             Pointer to the submit timestamp
//! \retval     CM_SUCCESS if event finished and returned submit timestamp
//! \retval     CM_FAILURE if event not finished
//!
EXTERN_C CM_RT_API int32_t CMRT_GetSubmitTime(CmEvent *event, LARGE_INTEGER* time)
{
    INSERT_PROFILER_RECORD();

    return event->GetProfilingInfo(CM_EVENT_PROFILING_SUBMIT, sizeof(LARGE_INTEGER), nullptr, time);
}

//!
//! \brief      Get HW start time
//! \details    VTune helper function to return the timestamp when task started
//!             executing on the GPU
//! \param      [in] event
//!             Pointer to the CmEvent
//! \param      [out] time
//!             Pointer to the GPU start execution timestamp
//! \retval     CM_SUCCESS if event finished and returned GPU start timestamp
//! \retval     CM_FAILURE if event not finished
//!
EXTERN_C CM_RT_API int32_t CMRT_GetHWStartTime(CmEvent *event, LARGE_INTEGER* time)
{
    INSERT_PROFILER_RECORD();

    return event->GetProfilingInfo(CM_EVENT_PROFILING_HWSTART, sizeof(LARGE_INTEGER), nullptr, time);
}

//!
//! \brief      Get HW end time
//! \details    VTune helper function to return the timestamp when task
//!             finished executing on the GPU
//! \param      [in] event
//!             Pointer to the CmEvent
//! \param      [out] time
//!             Pointer to the GPU finish execution timestamp
//! \retval     CM_SUCCESS if event finished and returned GPU finish timestamp
//! \retval     CM_FAILURE if event not finished
//!
EXTERN_C CM_RT_API int32_t CMRT_GetHWEndTime(CmEvent *event, LARGE_INTEGER* time)
{
    INSERT_PROFILER_RECORD();

    return event->GetProfilingInfo(CM_EVENT_PROFILING_HWEND, sizeof(LARGE_INTEGER), nullptr, time);
}

//!
//! \brief      Get complete time
//! \details    VTune helper function to return the CPU side complete timestamp
//! \param      [in] event
//!             Pointer to the CmEvent
//! \param      [out] time
//!             Pointer to the CPU side complete timestamp
//! \retval     CM_SUCCESS if event finished and returned CPU complete timestamp
//! \retval     CM_FAILURE if event not finished
//!
EXTERN_C CM_RT_API int32_t CMRT_GetCompleteTime(CmEvent *event, LARGE_INTEGER* time)
{
    INSERT_PROFILER_RECORD();

    return event->GetProfilingInfo(CM_EVENT_PROFILING_COMPLETE, sizeof(LARGE_INTEGER), nullptr, time);
}

//!
//! \brief      Get enqueue time
//! \details    VTune helper function to return the timestamp when task
//!             is put into CM's enqueued queue
//! \param      [in] event
//!             Pointer to the CmEvent
//! \param      [out] time
//!             Pointer to the timestamp when task
//!             is put into enqueued queue
//! \retval     CM_SUCCESS if event finished and returned timestamp 
//!                        when task put into enqueued queue
//! \retval     CM_FAILURE if event not finished
//!
EXTERN_C CM_RT_API int32_t CMRT_GetEnqueueTime( CmEvent *event, LARGE_INTEGER* time )
{
    INSERT_PROFILER_RECORD();

    return event->GetProfilingInfo(CM_EVENT_PROFILING_ENQUEUE, sizeof(LARGE_INTEGER), nullptr, time);
}


#define CM_CALLBACK __cdecl
typedef void (CM_CALLBACK *callback_function)(CmEvent*, void *);
//!
//! \brief      Set event callback function
//! \details    VTune helper function to set a callback function. 
//!             If CMRT_SetEventCallback function is called by VTune, 
//!             the callback function will be invoked upon CmEvent destruction
//! \param      [in] event
//!             Pointer to the CmEvent
//! \param      [in] function
//!             Callback function to be called
//! \param      [in] user_data
//!             Argument data to pass to callback function
//! \return     CM_SUCCESS
//!
EXTERN_C CM_RT_API int32_t CMRT_SetEventCallback(CmEvent* event, callback_function function, void* user_data)
{
    INSERT_PROFILER_RECORD();

    return event->GetProfilingInfo(CM_EVENT_PROFILING_CALLBACK, sizeof(void *), (void *)function , user_data);
}

//!
//! \brief      Enqueue a task
//! \details    Enqueues a task represented by CmTask.
//!             The kernels in the CmTask object may be run concurrently.
//!             Each task can have up to CAP_KERNEL_COUNT_PER_TASK kernels.
//!             Tasks are executed according to the order they
//!             were enqueued. The next task will not start execution until
//!             the current task finishes. 
//!             This is a nonblocking call. i.e. it returns immediately 
//!             without waiting for GPU to start or finish execution of the
//!             task. A CmEvent is generated each time a task is enqueued.
//!             The CmEvent can be used to check the status or other data
//!             regarding the task execution. The generated event needs to
//!             be managed and released by the user. 
//!             To avoid generating an event, the user can set the event 
//!             as CM_NO_EVENT and pass it to this function.      
//! \param      [in] queue
//!             Pointer to the CmQueue
//! \param      [in] task
//!             Pointer to the CmTask
//! \param      [out] event
//!             Reference to the pointer for the CmEvent to be created
//! \param      [in] threadSpace
//!             Pointer to the thread space
//! \retval     CM_SUCCESS            if task successfully enqueued
//! \retval     CM_OUT_OF_HOST_MEMORY if out of host memory
//! \retval     CM_FAILURE            otherwise
//!
EXTERN_C CM_RT_API int32_t CMRT_Enqueue(CmQueue* queue, CmTask* task, CmEvent** event, const CmThreadSpace* threadSpace = nullptr)
{
    INSERT_PROFILER_RECORD();

    return queue->Enqueue(task, *event, threadSpace);
}


