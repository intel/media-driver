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
    void  *cmQueueHandle;  // [in] CmQueue pointer in CMRT@UMD

    void  *cmSurface2d;    // [in] CmSurface2d pointer in CMRT@UMD

    void  *sysMem;         // [in] pointer of system memory

    CM_FASTCOPY_DIRECTION copyDir;  // [in] direction for GPUCopy: CM_FASTCOPY_GPU2CPU (0) or CM_FASTCOPY_CPU2GPU(1)

    uint32_t widthStride;  // [in] width stride in byte for system memory, ZERO means no setting

    uint32_t heightStride;  // [in] height stride in row for system memory, ZERO means no setting

    uint32_t option;  // [in] option passed by user, only support CM_FASTCOPY_OPTION_NONBLOCKING(0) and CM_FASTCOPY_OPTION_BLOCKING(1)

    void  *cmEventHandle;  // [in/out] return CmDevice pointer in CMRT@UMD, nullptr if the input is CM_NO_EVENT

    uint32_t eventIndex;     // [out] index of Event in m_EventArray

    int32_t returnValue;    // [out] return value from CMRT@UMD
} CM_ENQUEUE_GPUCOPY_PARAM, *PCM_ENQUEUE_GPUCOPY_PARAM;

class CmQueue_RT : public CmQueue
{
public:
    static int32_t Create(CmDevice_RT *device,
                          CmQueue_RT* &queue,
                          CM_QUEUE_CREATE_OPTION queueCreateOption);
    static int32_t Destroy(CmQueue_RT *&queue);

    CM_RT_API int32_t Enqueue(CmTask *task,
                          CmEvent *&event,
                          const CmThreadSpace *threadSpace = nullptr);

    CM_RT_API int32_t EnqueueCopyCPUToGPU(CmSurface2D *surface,
                                      const unsigned char *sysMem,
                                      CmEvent *&event);

    CM_RT_API int32_t EnqueueCopyGPUToCPU(CmSurface2D *surface,
                                      unsigned char *sysMem,
                                      CmEvent *&event);

    CM_RT_API int32_t EnqueueInitSurface2D(CmSurface2D *surface,
                                       const uint32_t initValue,
                                       CmEvent *&event);

    CM_RT_API int32_t EnqueueCopyGPUToGPU(CmSurface2D *outputSurface,
                                      CmSurface2D *inputSurface,
                                      uint32_t option,
                                      CmEvent *&event);

    CM_RT_API int32_t EnqueueCopyCPUToCPU(unsigned char *dstSysMem,
                                      unsigned char *srcSysMem,
                                      uint32_t size,
                                      uint32_t option,
                                      CmEvent *&event);

    CM_RT_API int32_t EnqueueCopyCPUToGPUFullStride(CmSurface2D *surface,
                                                const unsigned char *sysMem,
                                                const uint32_t widthStride,
                                                const uint32_t heightStride,
                                                const uint32_t option,
                                                CmEvent *&event);

    CM_RT_API int32_t EnqueueCopyGPUToCPUFullStride(CmSurface2D *surface,
                                                unsigned char *sysMem,
                                                const uint32_t widthStride,
                                                const uint32_t heightStride,
                                                const uint32_t option,
                                                CmEvent *&event);

    CM_RT_API int32_t DestroyEvent(CmEvent *&event);

    CM_RT_API int32_t
    EnqueueWithGroup(CmTask *task,
                     CmEvent *&event,
                     const CmThreadGroupSpace *threadGroupSpace = nullptr);

    CM_RT_API int32_t EnqueueCopyCPUToGPUFullStrideDup(CmSurface2D *surface,
                                                const unsigned char *sysMem,
                                                const uint32_t widthStride,
                                                const uint32_t heightStride,
                                                const uint32_t option,
                                                CmEvent *&event);

    CM_RT_API int32_t EnqueueCopyGPUToCPUFullStrideDup(CmSurface2D *surface,
                                                unsigned char *sysMem,
                                                const uint32_t widthStride,
                                                const uint32_t heightStride,
                                                const uint32_t option,
                                                CmEvent *&event);

    CM_RT_API int32_t EnqueueWithHints(CmTask *task,
                                   CmEvent *&event,
                                   uint32_t hints = 0);

    CM_RT_API int32_t EnqueueVebox(CmVebox *vebox, CmEvent *&event);

    CM_RT_API int32_t EnqueueFast(CmTask *task,
                              CmEvent *&event,
                              const CmThreadSpace *threadSpace = nullptr);

    CM_RT_API int32_t DestroyEventFast(CmEvent *&event);

    CM_RT_API int32_t EnqueueWithGroupFast(CmTask *task,
                              CmEvent *&event,
                              const CmThreadGroupSpace *threadGroupSpace = nullptr);

    CM_RT_API int32_t SetResidentGroupAndParallelThreadNum(uint32_t residentGroupNum, uint32_t parallelThreadNum);

    CM_QUEUE_CREATE_OPTION GetQueueOption();

protected:
    CmQueue_RT(CmDevice_RT *device, CM_QUEUE_CREATE_OPTION queueCreateOption);

    ~CmQueue_RT();

    int32_t Initialize();
    int32_t Initialize(CM_QUEUE_CREATE_OPTION queueCreateOption);

    int32_t EnqueueCopy(CmSurface2D *surface,
                    const unsigned char *sysMem,
                    const uint32_t widthStride,
                    const uint32_t heightStride,
                    CM_FASTCOPY_DIRECTION direction,
                    const uint32_t option,
                    CmEvent *&event);

    CmDevice_RT *m_cmDev;

    void  *m_cmQueueHandle;  //pointer used in driver

    CM_QUEUE_CREATE_OPTION m_queueOption;

    CSync m_criticalSection;

private:
    CmQueue_RT(const CmQueue_RT &other);
    CmQueue_RT &operator=(const CmQueue_RT &other);

};

#endif  // #ifndef CMRTLIB_AGNOSTIC_HARDWARE_CM_QUEUE_H_
