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
#include "cm_queue_base_hw.h"
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

