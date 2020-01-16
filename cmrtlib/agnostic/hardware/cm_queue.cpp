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
#include "cm_queue.h"
#include "cm_debug.h"
#include "cm_device.h"
#include "cm_include.h"
#include "cm_mem.h"
#include "cm_timer.h"

struct CM_CREATEQUEUE_PARAM
{
    CM_QUEUE_CREATE_OPTION createOption; // [in/out]
    void *cmQueueHandle;                 // [out]
    int32_t returnValue;                 // [out]
};

struct CM_ENQUEUE_PARAM
{
    void *cmQueueHandle;        // [in]
    void *cmTaskHandle;         // [in]
    void *cmThreadSpaceHandle;  // [in]
    void *cmEventHandle;        // [out]
    uint32_t eventIndex;        // [out] index of Event in m_EventArray
    int32_t returnValue;        // [out]
};

struct CM_ENQUEUEGROUP_PARAM
{
    void *cmQueueHandle;      // [in]
    void *cmTaskHandle;       // [in]
    void *cmTGrpSpaceHandle;  // [in]
    void *cmEventHandle;      // [out]
    uint32_t eventIndex;      // [out] index of Event in m_EventArray
    int32_t returnValue;      // [out]
};

struct CM_ENQUEUEHINTS_PARAM
{
    void *cmQueueHandle;  // [in]
    void *cmTaskHandle;   // [in]
    void *cmEventHandle;  // [in]
    uint32_t hints;      // [in]
    uint32_t eventIndex;  // [out] index of Event in m_EventArray
    int32_t returnValue;  // [out]
};

struct CM_DESTROYEVENT_PARAM
{
    void *cmQueueHandle;  // [in]
    void *cmEventHandle;  // [in]
    int32_t returnValue;  // [out]
};

struct CM_ENQUEUE_GPUCOPY_V2V_PARAM
{
    void *cmQueueHandle;   // [in]
    void *cmSrcSurface2d;  // [in]
    void *cmDstSurface2d;  // [in]
    uint32_t option;       // [in]
    void *cmEventHandle;   // [out]
    uint32_t eventIndex;   // [out] index of Event in m_EventArray
    int32_t returnValue;   // [out]
};

struct CM_ENQUEUE_GPUCOPY_L2L_PARAM
{
    void *cmQueueHandle;  // [in]
    void *srcSysMem;      // [in]
    void *dstSysMem;      // [in]
    uint32_t copySize;     // [in]
    uint32_t option;      // [in]
    void *cmEventHandle;  // [out]
    uint32_t eventIndex;  // [out] index of Event in m_EventArray
    int32_t returnValue;  // [out]
};

struct CM_ENQUEUE_2DInit_PARAM
{
    void *cmQueueHandle;  // [in]
    void *cmSurface2d;    // [in]
    uint32_t initValue;  // [in]
    void *cmEventHandle;  // [out]
    uint32_t eventIndex;  // [out] index of Event in m_EventArray
    int32_t returnValue;  // [out]
};

struct CM_ENQUEUE_VEBOX_PARAM
{
    void *cmQueueHandle;  // [IN]
    void *cmVeboxHandle;  // [IN] CmVeboxG75's handle
    void *cmEventHandle;  // [out] event's handle
    uint32_t eventIndex;  // [out] event's index in  m_EventArray CMRT@UMD
    int32_t returnValue;  // [out] return value
};

int32_t CmQueue_RT::Create(CmDevice_RT *device, CmQueue_RT *&queue, CM_QUEUE_CREATE_OPTION queueCreateOption)
{
    int32_t result = CM_SUCCESS;
    queue = new(std::nothrow) CmQueue_RT(device, queueCreateOption);
    if (queue)
    {
        result = queue->Initialize(queueCreateOption);
        if (result != CM_SUCCESS)
        {
            CmQueue_RT::Destroy(queue);
        }
    }
    else
    {
        CmAssert(0);
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

int32_t CmQueue_RT::Destroy(CmQueue_RT *&queue)
{
    CmSafeRelease(queue);
    return CM_SUCCESS;
}

CmQueue_RT::CmQueue_RT(CmDevice_RT *device, CM_QUEUE_CREATE_OPTION queueCreateOption):
    m_cmDev(device),
    m_cmQueueHandle(nullptr),
    m_queueOption(queueCreateOption) {}

CmQueue_RT::~CmQueue_RT() {}

int32_t CmQueue_RT::Initialize()
{
    CM_CREATEQUEUE_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMDEVICE_CREATEQUEUE,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    m_cmQueueHandle = inParam.cmQueueHandle;
    m_queueOption   = inParam.createOption;
    return CM_SUCCESS;
}

int32_t CmQueue_RT::Initialize(CM_QUEUE_CREATE_OPTION queueCreateOption)
{
    CM_CREATEQUEUE_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.createOption = queueCreateOption;

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMDEVICE_CREATEQUEUEEX,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    m_cmQueueHandle = inParam.cmQueueHandle;
    return CM_SUCCESS;
}

//!
//! Enqueue an task. Each task have one or more kernels running concurrently.
//! Each kernel can run in multiple threads concurrently.
//! Tasks get executed according to the order they get enqueued. The next task
//! doesn't start execute until the current task finishs.
//! When the last argument, pThreadSpace, is not nullptr, there are dependency among all threads within a task
//! Enqueue will make sure each x/y pair in the CmThreadSpace object is associated with
//! a unique thread in the task to enqueue.Otherwise enqueue will fail.
//! This is a non-blocking call. i.e. it returs immediately without waiting for
//! GPU to finish the execution of the task.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can
//! be used to check if the task finishs.
//! INPUT:
//!     1) Array of CmKernel_RT pointers. These kernels are to run concurrently. The
//!        first nullptr pointer in the array indicates the end of kernels
//!     2) Reference to the pointer to CMEvent
//!     3) A boolean value to indicate if or not to flush the queue after enqueue the task
//!        by default the boolean value is TRUE.
//! OUTPUT:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated;
//!     CM_OUT_OF_HOST_MEMORY if out of host memery;
//!     CM_FAILURE otherwise.
//!     More error code is coming.
//!
CM_RT_API int32_t CmQueue_RT::Enqueue(CmTask *task,
                                  CmEvent *&event,
                                  const CmThreadSpace *threadSpace)
{
    INSERT_PROFILER_RECORD();
    if (task == nullptr)
    {
        CmAssert(0);
        CmDebugMessage(("Kernel array is NULL."));
        return CM_INVALID_ARG_VALUE;
    }
    m_criticalSection.Acquire();

    CM_ENQUEUE_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmTaskHandle = task;
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.cmThreadSpaceHandle = (void *)threadSpace;
    inParam.cmEventHandle = event;  // to support invisiable event, this field is used for input/output.

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUE,
                                                &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.returnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.returnValue;
    }

    event = static_cast<CmEvent *>(inParam.cmEventHandle);
    m_criticalSection.Release();
    return CM_SUCCESS;
}

CM_RT_API int32_t CmQueue_RT::EnqueueWithHints(CmTask *task,
                                           CmEvent *&event,
                                           uint32_t hints)
{
    INSERT_PROFILER_RECORD();
    if (task == nullptr)
    {
        CmAssert(0);
        CmDebugMessage(("Kernel array is NULL."));
        return CM_INVALID_ARG_VALUE;
    }
    m_criticalSection.Acquire();

    CM_ENQUEUEHINTS_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmTaskHandle = task;
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.hints = hints;
    inParam.cmEventHandle = event;  // to support invisable event, this field is used for input/output
    int32_t hr =
        m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUEWITHHINTS,
                                       &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.returnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.returnValue;
    }

    event = static_cast<CmEvent *>(inParam.cmEventHandle);
    m_criticalSection.Release();
    return CM_SUCCESS;
}

//!
//! Enqueue an task, which contains one pre-defined kernel to
//! copy from host memory to surface
//! This is a non-blocking call. i.e. it returs immediately without waiting for
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
//!
int32_t CmQueue_RT::EnqueueCopyCPUToGPU(CmSurface2D *surface,
                                    const unsigned char *sysMem,
                                    CmEvent *&event)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(surface,
                       sysMem,
                       0,
                       0,
                       CM_FASTCOPY_CPU2GPU,
                       CM_FASTCOPY_OPTION_NONBLOCKING,
                       event);
}

//!
//! Enqueue an task, which contains one pre-defined kernel to
//! copy from surface to host memory
//! This is a non-blocking call. i.e. it returs immediately without waiting for
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
//!
CM_RT_API int32_t CmQueue_RT::EnqueueCopyGPUToCPU(CmSurface2D *surface,
                                              unsigned char *sysMem,
                                              CmEvent *&event)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(surface,
                       sysMem,
                       0,
                       0,
                       CM_FASTCOPY_GPU2CPU,
                       CM_FASTCOPY_OPTION_NONBLOCKING,
                       event);
}

//!
//! Enqueue an task, which contains one pre-defined kernel to
//! copy from linear system memory to tiled video memory
//! This API supports both blocking/non-blocking copy, if user pass CM_GPUCOPY_OPTION_BLOCKING as option,
//! this API only return till copy operation is done. otherwise, this API will return immediately no waiting for copy in GPU.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can
//! be used to check if the task finishs.
//! INPUT:
//!     1) Pointer to the CmSurface2D as copy destination
//!     2) Pointer to the host memory as copy resource
//!     3) width stride in bytes for system memory
//!     4) height stride in rows for system memory
//!     5) option: CM_FASTCOPY_OPTION_NONBLOCKING,CM_FASTCOPY_OPTION_BLOCKING or CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST
//!     6) Reference to the pointer to CMEvent
//!
//! RETURNS:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated;
//!     CM_OUT_OF_HOST_MEMORY if out of host memery;
//!     CM_FAILURE otherwise.
//!
CM_RT_API int32_t
CmQueue_RT::EnqueueCopyCPUToGPUFullStride(CmSurface2D *surface,
                                          const unsigned char *sysMem,
                                          const uint32_t widthStride,
                                          const uint32_t heightStride,
                                          const uint32_t option,
                                          CmEvent *&event)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(surface,
                       sysMem,
                       widthStride,
                       heightStride,
                       CM_FASTCOPY_CPU2GPU,
                       option,
                       event);
}

//!
//! Enqueue an task, which contains one pre-defined kernel to
//! copy from tiled video memory to linear system memory
//! This API supports both blocking/non-blocking copy, if user pass CM_FASTCOPY_OPTION_BLOCKING as option,
//! this API only return till copy operation is done. otherwise, this API will return immediately no waiting for copy in GPU.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can
//! be used to check if the task finishs.
//! INPUT:
//!     1) Pointer to the CmSurface2D as copy resource
//!     2) Pointer to the host memory as copy destination
//!     3) width stride in bytes for system memory
//!     4) height stride in rows for system memory
//!     5) option: CM_FASTCOPY_OPTION_NONBLOCKING or CM_FASTCOPY_OPTION_BLOCKING
//!     6) Reference to the pointer to CMEvent
//!
//! RETURNS:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated;
//!     CM_OUT_OF_HOST_MEMORY if out of host memery;
//!     CM_FAILURE otherwise.
//!
CM_RT_API int32_t CmQueue_RT::EnqueueCopyGPUToCPUFullStride(CmSurface2D *surface,
                                                        unsigned char *sysMem,
                                                        const uint32_t widthStride,
                                                        const uint32_t heightStride,
                                                        const uint32_t option,
                                                        CmEvent *&event)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(surface,
                       sysMem,
                       widthStride,
                       heightStride,
                       CM_FASTCOPY_GPU2CPU,
                       option,
                       event);
}

//!
//! Enqueue an task, which contains one pre-defined kernel to
//! copy from linear system memory to tiled video memory
//! This API supports both blocking/non-blocking copy, if user pass CM_GPUCOPY_OPTION_BLOCKING as option,
//! this API only return till copy operation is done. otherwise, this API will return immediately no waiting for copy in GPU.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can
//! be used to check if the task finishs.
//! INPUT:
//!     1) Pointer to the CmSurface2D as copy destination
//!     2) Pointer to the host memory as copy resource
//!     3) width stride in bytes for system memory
//!     4) height stride in rows for system memory
//!     5) option: CM_FASTCOPY_OPTION_NONBLOCKING,CM_FASTCOPY_OPTION_BLOCKING or CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST
//!     6) Reference to the pointer to CMEvent
//!
//! RETURNS:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated;
//!     CM_OUT_OF_HOST_MEMORY if out of host memery;
//!     CM_FAILURE otherwise.
//!
CM_RT_API int32_t
CmQueue_RT::EnqueueCopyCPUToGPUFullStrideDup(CmSurface2D *surface,
                                          const unsigned char *sysMem,
                                          const uint32_t widthStride,
                                          const uint32_t heightStride,
                                          const uint32_t option,
                                          CmEvent *&event)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(surface,
                       sysMem,
                       widthStride,
                       heightStride,
                       CM_FASTCOPY_CPU2GPU,
                       option,
                       event);
}

//!
//! Enqueue an task, which contains one pre-defined kernel to
//! copy from tiled video memory to linear system memory
//! This API supports both blocking/non-blocking copy, if user pass CM_FASTCOPY_OPTION_BLOCKING as option,
//! this API only return till copy operation is done. otherwise, this API will return immediately no waiting for copy in GPU.
//! A CmEvent is generated each time a task is enqueued. The CmEvent can
//! be used to check if the task finishs.
//! INPUT:
//!     1) Pointer to the CmSurface2D as copy resource
//!     2) Pointer to the host memory as copy destination
//!     3) width stride in bytes for system memory
//!     4) height stride in rows for system memory
//!     5) option: CM_FASTCOPY_OPTION_NONBLOCKING or CM_FASTCOPY_OPTION_BLOCKING
//!     6) Reference to the pointer to CMEvent
//!
//! RETURNS:
//!     CM_SUCCESS if the task is successfully enqueued and the CmEvent is generated;
//!     CM_OUT_OF_HOST_MEMORY if out of host memery;
//!     CM_FAILURE otherwise.
//!
CM_RT_API int32_t CmQueue_RT::EnqueueCopyGPUToCPUFullStrideDup(CmSurface2D *surface,
                                                        unsigned char *sysMem,
                                                        const uint32_t widthStride,
                                                        const uint32_t heightStride,
                                                        const uint32_t option,
                                                        CmEvent *&event)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(surface,
                       sysMem,
                       widthStride,
                       heightStride,
                       CM_FASTCOPY_GPU2CPU,
                       option,
                       event);
}

CM_RT_API int32_t CmQueue_RT::DestroyEvent(CmEvent *&event)
{
    INSERT_PROFILER_RECORD();
    if (event == nullptr)
    {
        return CM_FAILURE;
    }

    CM_DESTROYEVENT_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.cmEventHandle = event;

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_DESTROYEVENT,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    event = nullptr;
    return CM_SUCCESS;
}

//!
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
CM_RT_API int32_t
CmQueue_RT::EnqueueWithGroup(CmTask *task,
                             CmEvent *&event,
                             const CmThreadGroupSpace *threadGroupSpace)
{
    INSERT_PROFILER_RECORD();
    if (task == nullptr)
    {
        CmAssert(0);
        CmDebugMessage(("Kernel array is NULL."));
        return CM_INVALID_ARG_VALUE;
    }
    m_criticalSection.Acquire();

    CM_ENQUEUEGROUP_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmTaskHandle = task;
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.cmTGrpSpaceHandle = (void *)threadGroupSpace;
    inParam.cmEventHandle = event;  // to support invisiable event, this field is used for input/output.

    int32_t hr =
        m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUEWITHGROUP,
                                       &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.returnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.returnValue;
    }

    event = static_cast<CmEvent *>(inParam.cmEventHandle);
    m_criticalSection.Release();
    return CM_SUCCESS;
}

int32_t CmQueue_RT::EnqueueCopy(CmSurface2D *surface,
                            const unsigned char *sysMem,
                            const uint32_t widthStride,
                            const uint32_t heightStride,
                            CM_FASTCOPY_DIRECTION direction,
                            const uint32_t option,
                            CmEvent *&event)
{
    CM_ENQUEUE_GPUCOPY_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmQueueHandle = m_cmQueueHandle;

    inParam.cmSurface2d = surface;
    inParam.sysMem = (void *)sysMem;
    inParam.copyDir = direction;
    inParam.widthStride = widthStride;
    inParam.heightStride = heightStride;
    inParam.option = option;
    inParam.cmEventHandle = event;

    m_criticalSection.Acquire();

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUECOPY,
                                                &inParam, sizeof(inParam),
                                                nullptr, 0);
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.returnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.returnValue;
    }

    event = static_cast<CmEvent *>(inParam.cmEventHandle);
    m_criticalSection.Release();
    return hr;
}

CM_RT_API int32_t CmQueue_RT::EnqueueInitSurface2D(CmSurface2D *surface,
                                               const uint32_t initValue,
                                               CmEvent *&event)
{
    INSERT_PROFILER_RECORD();

    CM_ENQUEUE_2DInit_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.cmEventHandle = event;
    inParam.cmSurface2d = surface;
    inParam.initValue  = initValue;
    m_criticalSection.Acquire();

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUESURF2DINIT,
                                                &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.returnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.returnValue;
    }

    event = static_cast<CmEvent *>(inParam.cmEventHandle);
    m_criticalSection.Release();
    return hr;
}

CM_RT_API int32_t CmQueue_RT::EnqueueCopyGPUToGPU(CmSurface2D *outputSurface,
                                              CmSurface2D *inputSurface,
                                              uint32_t option,
                                              CmEvent *&event)
{
    INSERT_PROFILER_RECORD();

    CM_ENQUEUE_GPUCOPY_V2V_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.option        = option;
    inParam.cmEventHandle = event;
    inParam.cmDstSurface2d = outputSurface;
    inParam.cmSrcSurface2d = inputSurface;

    m_criticalSection.Acquire();

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUECOPY_V2V,
                                                &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.returnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.returnValue;
    }

    event = static_cast<CmEvent *>(inParam.cmEventHandle);
    m_criticalSection.Release();
    return hr;
}

CM_RT_API int32_t CmQueue_RT::EnqueueCopyCPUToCPU(unsigned char *dstSysMem,
                                              unsigned char *srcSysMem,
                                              uint32_t size,
                                              uint32_t option,
                                              CmEvent *&event)
{
    INSERT_PROFILER_RECORD();

    CM_ENQUEUE_GPUCOPY_L2L_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.srcSysMem     = srcSysMem;
    inParam.dstSysMem     = dstSysMem;
    inParam.copySize       = size;
    inParam.option        = option;
    inParam.cmEventHandle = event;

    m_criticalSection.Acquire();

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUECOPY_L2L,
                                                &inParam, sizeof(inParam));

    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.returnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.returnValue;
    }

    event = static_cast<CmEvent *>(inParam.cmEventHandle);
    m_criticalSection.Release();
    return hr;
}

CM_RT_API int32_t CmQueue_RT::EnqueueVebox(CmVebox *vebox, CmEvent *&event)
{
    INSERT_PROFILER_RECORD();

    CM_ENQUEUE_VEBOX_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.cmVeboxHandle = vebox;
    inParam.cmEventHandle = event;

    m_criticalSection.Acquire();

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUEVEBOX,
                                                &inParam, sizeof(inParam));

    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.returnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.returnValue;
    }

    event = static_cast<CmEvent *>(inParam.cmEventHandle);
    m_criticalSection.Release();
    return hr;
}

CM_QUEUE_CREATE_OPTION CmQueue_RT::GetQueueOption()
{
    return m_queueOption;
}

CM_RT_API int32_t CmQueue_RT::EnqueueFast(CmTask *task,
                              CmEvent *&event,
                              const CmThreadSpace *threadSpace)
{
    INSERT_PROFILER_RECORD();
    if (task == nullptr)
    {
        CmAssert(0);
        CmDebugMessage(("Kernel array is NULL."));
        return CM_INVALID_ARG_VALUE;
    }
    m_criticalSection.Acquire();

    CM_ENQUEUE_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmTaskHandle = task;
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.cmThreadSpaceHandle = (void *)threadSpace;
    inParam.cmEventHandle = event;  // to support invisiable event, this field is used for input/output.

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUEFAST,
                                                &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.returnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.returnValue;
    }

    event = static_cast<CmEvent *>(inParam.cmEventHandle);
    m_criticalSection.Release();
    return CM_SUCCESS;
}

CM_RT_API int32_t CmQueue_RT::EnqueueWithGroupFast(CmTask *task,
                              CmEvent *&event,
                              const CmThreadGroupSpace *threadGroupSpace)
{
    INSERT_PROFILER_RECORD();
    if (task == nullptr)
    {
        CmAssert(0);
        CmDebugMessage(("Kernel array is NULL."));
        return CM_INVALID_ARG_VALUE;
    }
    m_criticalSection.Acquire();

    CM_ENQUEUEGROUP_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmTaskHandle = task;
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.cmTGrpSpaceHandle = (void *)threadGroupSpace;
    inParam.cmEventHandle = event;  // to support invisiable event, this field is used for input/output.

    int32_t hr =
        m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUEWITHGROUPFAST,
                                       &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.returnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.returnValue;
    }

    event = static_cast<CmEvent *>(inParam.cmEventHandle);
    m_criticalSection.Release();
    return CM_SUCCESS;

}


CM_RT_API int32_t CmQueue_RT::DestroyEventFast(CmEvent *&event)
{
    INSERT_PROFILER_RECORD();
    if (event == nullptr)
    {
        return CM_INVALID_ARG_VALUE;
    }

    CM_DESTROYEVENT_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.cmQueueHandle = m_cmQueueHandle;
    inParam.cmEventHandle = event;

    int32_t hr = m_cmDev->OSALExtensionExecute(CM_FN_CMQUEUE_DESTROYEVENTFAST,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.returnValue);
    event = nullptr;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmQueue_RT::SetResidentGroupAndParallelThreadNum(uint32_t residentGroupNum, uint32_t parallelThreadNum)
{
    return CM_NOT_IMPLEMENTED;
}

