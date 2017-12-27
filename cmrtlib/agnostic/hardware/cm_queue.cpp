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
    unsigned int iCmQueueType;   // [in]
    bool bCmRunAloneMode;        // [in]
    unsigned int iCmGPUContext;  // [in]
    void *pCmQueueHandle;        // [out]
    int32_t iReturnValue;        // [out]
};

struct CM_ENQUEUE_PARAM
{
    void *pCmQueueHandle;        // [in]
    void *pCmTaskHandle;         // [in]
    void *pCmThreadSpaceHandle;  // [in]
    void *pCmEventHandle;        // [out]
    uint32_t iEventIndex;        // [out] index of Event in m_EventArray
    int32_t iReturnValue;        // [out]
};

struct CM_ENQUEUEGROUP_PARAM
{
    void *pCmQueueHandle;      // [in]
    void *pCmTaskHandle;       // [in]
    void *pCmTGrpSpaceHandle;  // [in]
    void *pCmEventHandle;      // [out]
    uint32_t iEventIndex;      // [out] index of Event in m_EventArray
    int32_t iReturnValue;      // [out]
};

struct CM_ENQUEUEHINTS_PARAM
{
    void *pCmQueueHandle;  // [in]
    void *pCmTaskHandle;   // [in]
    void *pCmEventHandle;  // [in]
    uint32_t uiHints;      // [in]
    uint32_t iEventIndex;  // [out] index of Event in m_EventArray
    int32_t iReturnValue;  // [out]
};

struct CM_DESTROYEVENT_PARAM
{
    void *pCmQueueHandle;  // [in]
    void *pCmEventHandle;  // [in]
    int32_t iReturnValue;  // [out]
};

struct CM_ENQUEUE_GPUCOPY_V2V_PARAM
{
    void *pCmQueueHandle;   // [in]
    void *pCmSrcSurface2d;  // [in]
    void *pCmDstSurface2d;  // [in]
    uint32_t iOption;       // [in]
    void *pCmEventHandle;   // [out]
    uint32_t iEventIndex;   // [out] index of Event in m_EventArray
    int32_t iReturnValue;   // [out]
};

struct CM_ENQUEUE_GPUCOPY_L2L_PARAM
{
    void *pCmQueueHandle;  // [in]
    void *pSrcSysMem;      // [in]
    void *pDstSysMem;      // [in]
    uint32_t CopySize;     // [in]
    uint32_t iOption;      // [in]
    void *pCmEventHandle;  // [out]
    uint32_t iEventIndex;  // [out] index of Event in m_EventArray
    int32_t iReturnValue;  // [out]
};

struct CM_ENQUEUE_2DInit_PARAM
{
    void *pCmQueueHandle;  // [in]
    void *pCmSurface2d;    // [in]
    uint32_t dwInitValue;  // [in]
    void *pCmEventHandle;  // [out]
    uint32_t iEventIndex;  // [out] index of Event in m_EventArray
    int32_t iReturnValue;  // [out]
};

struct CM_ENQUEUE_VEBOX_PARAM
{
    void *pCmQueueHandle;  // [IN]
    void *pCmVeboxHandle;  // [IN] CmVeboxG75's handle
    void *pCmEventHandle;  // [out] event's handle
    uint32_t iEventIndex;  // [out] event's index in  m_EventArray CMRT@UMD
    int32_t iReturnValue;  // [out] return value
};

int32_t CmQueue_RT::Create(CmDevice_RT *pDevice, CmQueue_RT *&pQueue, CM_QUEUE_CREATE_OPTION QueueCreateOption)
{
    int32_t result = CM_SUCCESS;
    pQueue = new(std::nothrow) CmQueue_RT(pDevice);
    if (pQueue)
    {
        result = pQueue->Initialize(QueueCreateOption);
        if (result != CM_SUCCESS)
        {
            CmQueue_RT::Destroy(pQueue);
        }
    }
    else
    {
        CmAssert(0);
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

int32_t CmQueue_RT::Destroy(CmQueue_RT *&pQueue)
{
    CmSafeRelease(pQueue);
    return CM_SUCCESS;
}

CmQueue_RT::CmQueue_RT(CmDevice_RT *pDevice):
    m_pCmDev(pDevice),
    m_pCmQueueHandle(nullptr) {}

CmQueue_RT::~CmQueue_RT() {}

int32_t CmQueue_RT::Initialize(CM_QUEUE_CREATE_OPTION QueueCreateOption)
{
    CM_CREATEQUEUE_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.iCmQueueType = QueueCreateOption.QueueType;
    inParam.bCmRunAloneMode = QueueCreateOption.RunAloneMode;
    inParam.iCmGPUContext = CM_DEFAULT_QUEUE_CREATE_OPTION.Reserved1;

    int32_t hr = m_pCmDev->OSALExtensionExecute(CM_FN_CMDEVICE_CREATEQUEUE,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);
    m_pCmQueueHandle = inParam.pCmQueueHandle;
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
CM_RT_API int32_t CmQueue_RT::Enqueue(CmTask *pTask,
                                  CmEvent *&pEvent,
                                  const CmThreadSpace *pThreadSpace)
{
    INSERT_PROFILER_RECORD();
    if (pTask == nullptr)
    {
        CmAssert(0);
        CmDebugMessage(("Kernel array is NULL."));
        return CM_INVALID_ARG_VALUE;
    }
    m_criticalSection.Acquire();

    CM_ENQUEUE_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.pCmTaskHandle = pTask;
    inParam.pCmQueueHandle = m_pCmQueueHandle;
    inParam.pCmThreadSpaceHandle = (void *)pThreadSpace;
    inParam.pCmEventHandle = pEvent;  // to support invisiable event, this field is used for input/output.

    int32_t hr = m_pCmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUE,
                                                &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.iReturnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.iReturnValue;
    }

    pEvent = static_cast<CmEvent *>(inParam.pCmEventHandle);
    m_criticalSection.Release();
    return CM_SUCCESS;
}

CM_RT_API int32_t CmQueue_RT::EnqueueWithHints(CmTask *pTask,
                                           CmEvent *&pEvent,
                                           uint32_t hints)
{
    INSERT_PROFILER_RECORD();
    if (pTask == nullptr)
    {
        CmAssert(0);
        CmDebugMessage(("Kernel array is NULL."));
        return CM_INVALID_ARG_VALUE;
    }
    m_criticalSection.Acquire();

    CM_ENQUEUEHINTS_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.pCmTaskHandle = pTask;
    inParam.pCmQueueHandle = m_pCmQueueHandle;
    inParam.uiHints = hints;
    inParam.pCmEventHandle = pEvent;  // to support invisable event, this field is used for input/output
    int32_t hr =
        m_pCmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUEWITHHINTS,
                                       &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.iReturnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.iReturnValue;
    }

    pEvent = static_cast<CmEvent *>(inParam.pCmEventHandle);
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
int32_t CmQueue_RT::EnqueueCopyCPUToGPU(CmSurface2D *pSurface,
                                    const unsigned char *pSysMem,
                                    CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(pSurface,
                       pSysMem,
                       0,
                       0,
                       CM_FASTCOPY_CPU2GPU,
                       CM_FASTCOPY_OPTION_NONBLOCKING,
                       pEvent);
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
CM_RT_API int32_t CmQueue_RT::EnqueueCopyGPUToCPU(CmSurface2D *pSurface,
                                              unsigned char *pSysMem,
                                              CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(pSurface,
                       pSysMem,
                       0,
                       0,
                       CM_FASTCOPY_GPU2CPU,
                       CM_FASTCOPY_OPTION_NONBLOCKING,
                       pEvent);
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
CmQueue_RT::EnqueueCopyCPUToGPUFullStride(CmSurface2D *pSurface,
                                          const unsigned char *pSysMem,
                                          const uint32_t widthStride,
                                          const uint32_t heightStride,
                                          const uint32_t option,
                                          CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(pSurface,
                       pSysMem,
                       widthStride,
                       heightStride,
                       CM_FASTCOPY_CPU2GPU,
                       option,
                       pEvent);
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
CM_RT_API int32_t CmQueue_RT::EnqueueCopyGPUToCPUFullStride(CmSurface2D *pSurface,
                                                        unsigned char *pSysMem,
                                                        const uint32_t widthStride,
                                                        const uint32_t heightStride,
                                                        const uint32_t option,
                                                        CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(pSurface,
                       pSysMem,
                       widthStride,
                       heightStride,
                       CM_FASTCOPY_GPU2CPU,
                       option,
                       pEvent);
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
CmQueue_RT::EnqueueCopyCPUToGPUFullStrideDup(CmSurface2D *pSurface,
                                          const unsigned char *pSysMem,
                                          const uint32_t widthStride,
                                          const uint32_t heightStride,
                                          const uint32_t option,
                                          CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(pSurface,
                       pSysMem,
                       widthStride,
                       heightStride,
                       CM_FASTCOPY_CPU2GPU,
                       option,
                       pEvent);
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
CM_RT_API int32_t CmQueue_RT::EnqueueCopyGPUToCPUFullStrideDup(CmSurface2D *pSurface,
                                                        unsigned char *pSysMem,
                                                        const uint32_t widthStride,
                                                        const uint32_t heightStride,
                                                        const uint32_t option,
                                                        CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();
    return EnqueueCopy(pSurface,
                       pSysMem,
                       widthStride,
                       heightStride,
                       CM_FASTCOPY_GPU2CPU,
                       option,
                       pEvent);
}

CM_RT_API int32_t CmQueue_RT::DestroyEvent(CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();
    if (pEvent == nullptr)
    {
        return CM_FAILURE;
    }

    CM_DESTROYEVENT_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.pCmQueueHandle = m_pCmQueueHandle;
    inParam.pCmEventHandle = pEvent;

    int32_t hr = m_pCmDev->OSALExtensionExecute(CM_FN_CMQUEUE_DESTROYEVENT,
                                                &inParam, sizeof(inParam));
    CHK_FAILURE_RETURN(hr);
    CHK_FAILURE_RETURN(inParam.iReturnValue);
    pEvent = nullptr;
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
CmQueue_RT::EnqueueWithGroup(CmTask *pTask,
                             CmEvent *&pEvent,
                             const CmThreadGroupSpace *pThreadGroupSpace)
{
    INSERT_PROFILER_RECORD();
    if (pTask == nullptr)
    {
        CmAssert(0);
        CmDebugMessage(("Kernel array is NULL."));
        return CM_INVALID_ARG_VALUE;
    }
    m_criticalSection.Acquire();

    CM_ENQUEUEGROUP_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.pCmTaskHandle = pTask;
    inParam.pCmQueueHandle = m_pCmQueueHandle;
    inParam.pCmTGrpSpaceHandle = (void *)pThreadGroupSpace;
    inParam.pCmEventHandle = pEvent;  // to support invisiable event, this field is used for input/output.

    int32_t hr =
        m_pCmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUEWITHGROUP,
                                       &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.iReturnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.iReturnValue;
    }

    pEvent = static_cast<CmEvent *>(inParam.pCmEventHandle);
    m_criticalSection.Release();
    return CM_SUCCESS;
}

int32_t CmQueue_RT::EnqueueCopy(CmSurface2D *pSurface,
                            const unsigned char *pSysMem,
                            const uint32_t widthStride,
                            const uint32_t heightStride,
                            CM_FASTCOPY_DIRECTION direction,
                            const uint32_t option,
                            CmEvent *&pEvent)
{
    CM_ENQUEUE_GPUCOPY_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.pCmQueueHandle = m_pCmQueueHandle;

    inParam.pCmSurface2d = pSurface;
    inParam.pSysMem = (void *)pSysMem;
    inParam.iCopyDir = direction;
    inParam.iWidthStride = widthStride;
    inParam.iHeightStride = heightStride;
    inParam.iOption = option;
    inParam.pCmEventHandle = pEvent;

    m_criticalSection.Acquire();

    int32_t hr = m_pCmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUECOPY,
                                                &inParam, sizeof(inParam),
                                                nullptr, 0);
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.iReturnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.iReturnValue;
    }

    pEvent = static_cast<CmEvent *>(inParam.pCmEventHandle);
    m_criticalSection.Release();
    return hr;
}

CM_RT_API int32_t CmQueue_RT::EnqueueInitSurface2D(CmSurface2D *pSurface,
                                               const uint32_t initValue,
                                               CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();

    CM_ENQUEUE_2DInit_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.pCmQueueHandle = m_pCmQueueHandle;
    inParam.pCmEventHandle = pEvent;
    inParam.pCmSurface2d = pSurface;
    inParam.dwInitValue  = initValue;
    m_criticalSection.Acquire();

    int32_t hr = m_pCmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUESURF2DINIT,
                                                &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.iReturnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.iReturnValue;
    }

    pEvent = static_cast<CmEvent *>(inParam.pCmEventHandle);
    m_criticalSection.Release();
    return hr;
}

CM_RT_API int32_t CmQueue_RT::EnqueueCopyGPUToGPU(CmSurface2D *pOutputSurface,
                                              CmSurface2D *pInputSurface,
                                              uint32_t option,
                                              CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();

    CM_ENQUEUE_GPUCOPY_V2V_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.pCmQueueHandle = m_pCmQueueHandle;
    inParam.iOption        = option;
    inParam.pCmEventHandle = pEvent;
    inParam.pCmDstSurface2d = pOutputSurface;
    inParam.pCmSrcSurface2d = pInputSurface;

    m_criticalSection.Acquire();

    int32_t hr = m_pCmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUECOPY_V2V,
                                                &inParam, sizeof(inParam));
    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.iReturnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.iReturnValue;
    }

    pEvent = static_cast<CmEvent *>(inParam.pCmEventHandle);
    m_criticalSection.Release();
    return hr;
}

CM_RT_API int32_t CmQueue_RT::EnqueueCopyCPUToCPU(unsigned char *pDstSysMem,
                                              unsigned char *pSrcSysMem,
                                              uint32_t size,
                                              uint32_t option,
                                              CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();

    CM_ENQUEUE_GPUCOPY_L2L_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.pCmQueueHandle = m_pCmQueueHandle;
    inParam.pSrcSysMem     = pSrcSysMem;
    inParam.pDstSysMem     = pDstSysMem;
    inParam.CopySize       = size;
    inParam.iOption        = option;
    inParam.pCmEventHandle = pEvent;

    m_criticalSection.Acquire();

    int32_t hr = m_pCmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUECOPY_L2L,
                                                &inParam, sizeof(inParam));

    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.iReturnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.iReturnValue;
    }

    pEvent = static_cast<CmEvent *>(inParam.pCmEventHandle);
    m_criticalSection.Release();
    return hr;
}

CM_RT_API int32_t CmQueue_RT::EnqueueVebox(CmVebox *pVebox, CmEvent *&pEvent)
{
    INSERT_PROFILER_RECORD();

    CM_ENQUEUE_VEBOX_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(inParam));
    inParam.pCmQueueHandle = m_pCmQueueHandle;
    inParam.pCmVeboxHandle = pVebox;
    inParam.pCmEventHandle = pEvent;

    m_criticalSection.Acquire();

    int32_t hr = m_pCmDev->OSALExtensionExecute(CM_FN_CMQUEUE_ENQUEUEVEBOX,
                                                &inParam, sizeof(inParam));

    if (FAILED(hr))
    {
        CmAssert(0);
        m_criticalSection.Release();
        return hr;
    }
    if (inParam.iReturnValue != CM_SUCCESS)
    {
        m_criticalSection.Release();
        return inParam.iReturnValue;
    }

    pEvent = static_cast<CmEvent *>(inParam.pCmEventHandle);
    m_criticalSection.Release();
    return hr;
}
