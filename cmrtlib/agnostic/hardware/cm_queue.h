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
#ifndef CMRTLIB_AGNOSTIC_HARDWARE_CM_QUEUE_H_
#define CMRTLIB_AGNOSTIC_HARDWARE_CM_QUEUE_H_

#include "cm_queue_base.h"
#include "cm_def_os.h"

class CmEvent;
class CmSurface2D;
class CmDevice_RT;
class CmThreadSpace;
class CmTask;
class CmThreadGroupSpace;

typedef enum _CM_FASTCOPY_DIRECTION
{
    CM_FASTCOPY_GPU2CPU = 0,
    CM_FASTCOPY_CPU2GPU = 1
} CM_FASTCOPY_DIRECTION;

typedef enum _CM_FASTCOPY_OPTION
{
    CM_FASTCOPY_OPTION_NONBLOCKING = 0x00,
    CM_FASTCOPY_OPTION_BLOCKING    = 0x01
} CM_FASTCOPY_OPTION;

//CM_ENQUEUE_GPUCOPY_PARAM version 2: two new fields are added
typedef struct _CM_ENQUEUE_GPUCOPY_PARAM
{
    void  *pCmQueueHandle;  // [in] CmQueue pointer in CMRT@UMD

    void  *pCmSurface2d;    // [in] CmSurface2d pointer in CMRT@UMD

    void  *pSysMem;         // [in] pointer of system memory

    CM_FASTCOPY_DIRECTION iCopyDir;  // [in] direction for GPUCopy: CM_FASTCOPY_GPU2CPU (0) or CM_FASTCOPY_CPU2GPU(1)

    uint32_t iWidthStride;  // [in] width stride in byte for system memory, ZERO means no setting

    uint32_t iHeightStride;  // [in] height stride in row for system memory, ZERO means no setting

    uint32_t iOption;  // [in] option passed by user, only support CM_FASTCOPY_OPTION_NONBLOCKING(0) and CM_FASTCOPY_OPTION_BLOCKING(1)

    void  *pCmEventHandle;  // [in/out] return CmDevice pointer in CMRT@UMD, nullptr if the input is CM_NO_EVENT

    uint32_t iEventIndex;     // [out] index of Event in m_EventArray

    int32_t iReturnValue;    // [out] return value from CMRT@UMD
} CM_ENQUEUE_GPUCOPY_PARAM, *PCM_ENQUEUE_GPUCOPY_PARAM;

class CmQueue_RT : public CmQueue
{
public:
    static int32_t Create(CmDevice_RT *pDevice, CmQueue_RT *&pQueue, CM_QUEUE_CREATE_OPTION QueueCreateOption);
    static int32_t Destroy(CmQueue_RT *&pQueue);

    CM_RT_API int32_t Enqueue(CmTask *pTask,
                          CmEvent *&pEvent,
                          const CmThreadSpace *pThreadSpace = nullptr);

    CM_RT_API int32_t EnqueueCopyCPUToGPU(CmSurface2D *pSurface,
                                      const unsigned char *pSysMem,
                                      CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueCopyGPUToCPU(CmSurface2D *pSurface,
                                      unsigned char *pSysMem,
                                      CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueInitSurface2D(CmSurface2D *pSurface,
                                       const uint32_t initValue,
                                       CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueCopyGPUToGPU(CmSurface2D *pOutputSurface,
                                      CmSurface2D *pInputSurface,
                                      uint32_t option,
                                      CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueCopyCPUToCPU(unsigned char *pDstSysMem,
                                      unsigned char *pSrcSysMem,
                                      uint32_t size,
                                      uint32_t option,
                                      CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueCopyCPUToGPUFullStride(CmSurface2D *pSurface,
                                                const unsigned char *pSysMem,
                                                const uint32_t widthStride,
                                                const uint32_t heightStride,
                                                const uint32_t option,
                                                CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueCopyGPUToCPUFullStride(CmSurface2D *pSurface,
                                                unsigned char *pSysMem,
                                                const uint32_t widthStride,
                                                const uint32_t heightStride,
                                                const uint32_t option,
                                                CmEvent *&pEvent);

    CM_RT_API int32_t DestroyEvent(CmEvent *&pEvent);

    CM_RT_API int32_t
    EnqueueWithGroup(CmTask *pTask,
                     CmEvent *&pEvent,
                     const CmThreadGroupSpace *pThreadGroupSpace = nullptr);
                     
    CM_RT_API int32_t EnqueueCopyCPUToGPUFullStrideDup(CmSurface2D *pSurface,
                                                const unsigned char *pSysMem,
                                                const uint32_t widthStride,
                                                const uint32_t heightStride,
                                                const uint32_t option,
                                                CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueCopyGPUToCPUFullStrideDup(CmSurface2D *pSurface,
                                                unsigned char *pSysMem,
                                                const uint32_t widthStride,
                                                const uint32_t heightStride,
                                                const uint32_t option,
                                                CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueWithHints(CmTask *pTask,
                                   CmEvent *&pEvent,
                                   uint32_t hints = 0);

    CM_RT_API int32_t EnqueueVebox(CmVebox *pVebox, CmEvent *&pEvent);

protected:
    CmQueue_RT(CmDevice_RT *pDevice);

    ~CmQueue_RT();

    int32_t Initialize(CM_QUEUE_CREATE_OPTION QueueCreateOption);

    int32_t EnqueueCopy(CmSurface2D *pSurface,
                    const unsigned char *pSysMem,
                    const uint32_t widthStride,
                    const uint32_t heightStride,
                    CM_FASTCOPY_DIRECTION direction,
                    const uint32_t option,
                    CmEvent *&pEvent);

    CmDevice_RT *m_pCmDev;

    void  *m_pCmQueueHandle;  //pointer used in driver

    CSync m_criticalSection;
};

#endif  // #ifndef CMRTLIB_AGNOSTIC_HARDWARE_CM_QUEUE_H_
