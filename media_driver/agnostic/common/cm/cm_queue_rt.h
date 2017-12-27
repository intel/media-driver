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
//! \file      cm_queue_rt.h
//! \brief     Contains CmQueueRT declarations.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMQUEUERT_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMQUEUERT_H_

#include "cm_queue.h"

#include <queue>

#include "cm_array.h"
#include "cm_csync.h"
#include "cm_hal.h"

enum CM_GPUCOPY_DIRECTION
{
    CM_FASTCOPY_GPU2CPU = 0,
    CM_FASTCOPY_CPU2GPU = 1,
    CM_FASTCOPY_GPU2GPU = 2,
    CM_FASTCOPY_CPU2CPU = 3
};

namespace CMRT_UMD
{
class CmDeviceRT;
class CmKernel;
class CmKernelRT;
class CmTaskInternal;
class CmEventRT;
class CmThreadSpaceRT;
class CmThreadGroupSpace;
class CmVebox;
class CmSurface2D;
class CmSurface2DRT;

struct CM_GPUCOPY_KERNEL
{
    CmKernel *pKernel;
    CM_GPUCOPY_KERNEL_ID KernelID;
    bool bLocked;
};

class ThreadSafeQueue
{
public:
    bool Push(CmTaskInternal *element)
    {
        mCriticalSection.Acquire();
        mQueue.push(element);
        mCriticalSection.Release();
        return true;
    }

    CmTaskInternal *Pop()
    {
        CmTaskInternal *element = nullptr;
        mCriticalSection.Acquire();
        if (mQueue.empty())
        {
            CM_ASSERT(0);
        }
        else
        {
            element = mQueue.front();
            mQueue.pop();
        }
        mCriticalSection.Release();
        return element;
    }

    CmTaskInternal *Top()
    {
        CmTaskInternal *element = nullptr;
        if (mQueue.empty())
        {
            CM_ASSERT(0);
        }
        else
        {
            element = mQueue.front();
        }
        return element;
    }

    bool IsEmpty() { return mQueue.empty(); }

    int GetCount() { return mQueue.size(); }

 private:
    std::queue<CmTaskInternal*> mQueue;
    CSync mCriticalSection;
};

//!
//! \brief    Class CmQueueRT definitions.
//!
class CmQueueRT: public CmQueue
{
public:
    static int32_t Create(CmDeviceRT *pDevice,
                          CmQueueRT *&pQueue,
                          CM_QUEUE_CREATE_OPTION QueueCreateOption);

    static int32_t Destroy(CmQueueRT *&pQueue);

    CM_RT_API int32_t Enqueue(CmTask *pTask,
                              CmEvent *&pEvent,
                              const CmThreadSpace *pThreadSpace = nullptr);

    CM_RT_API int32_t DestroyEvent(CmEvent *&pEvent);

    CM_RT_API int32_t
    EnqueueWithGroup(CmTask *pTask,
                     CmEvent *&pEvent,
                     const CmThreadGroupSpace *pThreadGroupSpace = nullptr);

    CM_RT_API int32_t EnqueueVebox(CmVebox *pVebox, CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueWithHints(CmTask *pTask,
                                       CmEvent *&pEvent,
                                       uint32_t hints = 0);

    CM_RT_API int32_t EnqueueCopyCPUToGPU(CmSurface2D *pSurface,
                                          const unsigned char *pSysMem,
                                          CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueCopyGPUToCPU(CmSurface2D *pSurface,
                                          unsigned char *pSysMem,
                                          CmEvent *&pEvent);

    CM_RT_API int32_t EnqueueInitSurface2D(CmSurface2D *pSurf2D,
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

    int32_t EnqueueCopyInternal_1Plane(CmSurface2DRT *pSurface,
                                       unsigned char *pSysMem,
                                       CM_SURFACE_FORMAT format,
                                       const uint32_t widthInPixel,
                                       const uint32_t widthStride,
                                       const uint32_t heightInRow,
                                       const uint32_t heightStride,
                                       const uint32_t sizePerPixel,
                                       CM_GPUCOPY_DIRECTION direction,
                                       const uint32_t option,
                                       CmEvent *&pEvent);

    int32_t EnqueueCopyInternal_2Planes(CmSurface2DRT *pSurface,
                                        unsigned char *pSysMem,
                                        CM_SURFACE_FORMAT format,
                                        const uint32_t widthInPixel,
                                        const uint32_t widthStride,
                                        const uint32_t heightInRow,
                                        const uint32_t heightStride,
                                        const uint32_t sizePerPixel,
                                        CM_GPUCOPY_DIRECTION direction,
                                        const uint32_t option,
                                        CmEvent *&pEvent);

    int32_t EnqueueCopyInternal(CmSurface2DRT *pSurface,
                                unsigned char *pSysMem,
                                const uint32_t widthStride,
                                const uint32_t heightStride,
                                CM_GPUCOPY_DIRECTION direction,
                                const uint32_t option,
                                CmEvent *&pEvent);

    int32_t EnqueueUnalignedCopyInternal(CmSurface2DRT *pSurface,
                                         unsigned char *pSysMem,
                                         const uint32_t widthStride,
                                         const uint32_t heightStride,
                                         CM_GPUCOPY_DIRECTION direction,
                                         CmEvent *&pEvent);

    int32_t FlushTaskWithoutSync(bool bIfFlushBlock = false);

    int32_t GetTaskCount(uint32_t &numTasks);

    int32_t TouchFlushedTasks();

    int32_t GetTaskHasThreadArg(CmKernelRT *pKernelArray[],
                                uint32_t numKernels,
                                bool &threadArgExists);
    int32_t CleanQueue();

    CM_QUEUE_CREATE_OPTION &GetQueueOption();

protected:
    CmQueueRT(CmDeviceRT *pDevice, CM_QUEUE_CREATE_OPTION QueueCreateOption);

    ~CmQueueRT();

    int32_t Initialize();

    int32_t
    Enqueue_RT(CmKernelRT *pKernelArray[],
               const uint32_t uiKernelCount,
               const uint32_t uiTotalThreadCount,
               CmEventRT *&pEvent,
               const CmThreadSpaceRT *pTS = nullptr,
               const uint64_t uiSyncBitmap = 0,
               PCM_POWER_OPTION pPowerOption = nullptr,
               const uint64_t uiConditionalEndBitmap = 0,
               PCM_HAL_CONDITIONAL_BB_END_INFO pConditionalEndInfo = nullptr,
               CM_TASK_CONFIG *pTaskConfig = nullptr);

    int32_t Enqueue_RT(CmKernelRT *pKernelArray[],
                       const uint32_t uiKernelCount,
                       const uint32_t uiTotalThreadCount,
                       CmEventRT *&pEvent,
                       const CmThreadGroupSpace *pTGS = nullptr,
                       const uint64_t uiSyncBitmap = 0,
                       PCM_POWER_OPTION pPowerOption = nullptr,
                       CM_TASK_CONFIG *pTaskConfig = nullptr);

    int32_t Enqueue_RT(CmKernelRT *pKernelArray[],
                       CmEventRT *&pEvent,
                       uint32_t numTaskGenerated,
                       bool isLastTask,
                       uint32_t hints = 0,
                       PCM_POWER_OPTION pPowerOption = nullptr);

    int32_t QueryFlushedTasks();

    //New sub functions for different task flush
    int32_t FlushGeneralTask(CmTaskInternal *pTask);

    int32_t FlushGroupTask(CmTaskInternal *pTask);

    int32_t FlushVeboxTask(CmTaskInternal *pTask);

    int32_t FlushEnqueueWithHintsTask(CmTaskInternal *pTask);

    void PopTaskFromFlushedQueue();

    int32_t CreateEvent(CmTaskInternal *pTask,
                        bool bIsVisible,
                        int32_t &taskDriverId,
                        CmEventRT *&pEvent);

    int32_t AddGPUCopyKernel(CM_GPUCOPY_KERNEL* &pKernelParam);

    int32_t GetGPUCopyKrnID(uint32_t WidthInByte,
                            uint32_t height,
                            CM_SURFACE_FORMAT format,
                            CM_GPUCOPY_DIRECTION CopyDirection,
                            CM_GPUCOPY_KERNEL_ID &KernelID);

    int32_t AllocateGPUCopyKernel(uint32_t WidthInByte,
                                  uint32_t height,
                                  CM_SURFACE_FORMAT format,
                                  CM_GPUCOPY_DIRECTION CopyDirection,
                                  CmKernel* &pKernel);

    int32_t CreateGPUCopyKernel(uint32_t WidthInByte,
                                uint32_t height,
                                CM_SURFACE_FORMAT format,
                                CM_GPUCOPY_DIRECTION CopyDirection,
                                CM_GPUCOPY_KERNEL* &pGPUCopyKrnParam);

    int32_t SearchGPUCopyKernel(uint32_t WidthInByte,
                                uint32_t height,
                                CM_SURFACE_FORMAT format,
                                CM_GPUCOPY_DIRECTION CopyDirection,
                                CM_GPUCOPY_KERNEL* &pKernelParam);

    CmDeviceRT *m_pDevice;
    ThreadSafeQueue m_EnqueuedTasks;
    ThreadSafeQueue m_FlushedTasks;

    CmDynamicArray m_EventArray;
    CSync m_CriticalSection_Event;        // Protect m_EventArray
    CSync m_CriticalSection_HalExecute;   // Protect execution in HALCm, i.e HalCm_Execute
    CSync m_CriticalSection_FlushedTask;  // Protect QueryFlushedTask
    CSync m_CriticalSection_TaskInternal;

    uint32_t m_EventCount;

    CmDynamicArray m_CopyKrnParamArray;
    uint32_t m_CopyKrnParamArrayCount;

    CSync m_CriticalSection_GPUCopyKrn;

    CM_HAL_MAX_VALUES *m_pHalMaxValues;
    CM_QUEUE_CREATE_OPTION m_queueOption;
};
};  //namespace

#endif  // #ifnfef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMQUEUERT_H_
