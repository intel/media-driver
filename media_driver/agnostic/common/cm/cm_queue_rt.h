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
class CmBuffer;
class CmSurface2D;
class CmSurface2DRT;

struct CM_GPUCOPY_KERNEL
{
    CmKernel *kernel;
    CM_GPUCOPY_KERNEL_ID kernelID;
    bool locked;
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
    static int32_t Create(CmDeviceRT *device,
                          CmQueueRT *&queue,
                          CM_QUEUE_CREATE_OPTION queueCreateOption);

    static int32_t Destroy(CmQueueRT *&queue);

    CM_RT_API int32_t Enqueue(CmTask *task,
                              CmEvent *&event,
                              const CmThreadSpace *threadSpace = nullptr);

    CM_RT_API int32_t DestroyEvent(CmEvent *&event);

    CM_RT_API int32_t
    EnqueueWithGroup(CmTask *task,
                     CmEvent *&event,
                     const CmThreadGroupSpace *threadGroupSpace = nullptr);

    CM_RT_API int32_t EnqueueVebox(CmVebox *vebox, CmEvent *&event);

    CM_RT_API int32_t EnqueueWithHints(CmTask *task,
                                       CmEvent *&event,
                                       uint32_t hints = 0);

    CM_RT_API int32_t EnqueueCopyCPUToGPU(CmSurface2D *surface,
                                          const unsigned char *sysMem,
                                          CmEvent *&event);

    CM_RT_API int32_t EnqueueCopyGPUToCPU(CmSurface2D *surface,
                                          unsigned char *sysMem,
                                          CmEvent *&event);

    CM_RT_API int32_t EnqueueInitSurface2D(CmSurface2D *surf2D,
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

    CM_RT_API int32_t EnqueueFast(CmTask *task,
                              CmEvent *&event,
                              const CmThreadSpace *threadSpace = nullptr);

    CM_RT_API int32_t DestroyEventFast(CmEvent *&event);

    CM_RT_API int32_t EnqueueWithGroupFast(CmTask *task,
                                      CmEvent *&event,
                                      const CmThreadGroupSpace *threadGroupSpace = nullptr);

    int32_t EnqueueCopyInternal_1Plane(CmSurface2DRT *surface,
                                       unsigned char *sysMem,
                                       CM_SURFACE_FORMAT format,
                                       const uint32_t widthInPixel,
                                       const uint32_t widthStride,
                                       const uint32_t heightInRow,
                                       const uint32_t heightStride,
                                       const uint32_t sizePerPixel,
                                       CM_GPUCOPY_DIRECTION direction,
                                       const uint32_t option,
                                       CmEvent *&event);

    int32_t EnqueueCopyInternal_2Planes(CmSurface2DRT *surface,
                                        unsigned char *sysMem,
                                        CM_SURFACE_FORMAT format,
                                        const uint32_t widthInPixel,
                                        const uint32_t widthStride,
                                        const uint32_t heightInRow,
                                        const uint32_t heightStride,
                                        const uint32_t sizePerPixel,
                                        CM_GPUCOPY_DIRECTION direction,
                                        const uint32_t option,
                                        CmEvent *&event);

    int32_t EnqueueCopyInternal(CmSurface2DRT *surface,
                                unsigned char *sysMem,
                                const uint32_t widthStride,
                                const uint32_t heightStride,
                                CM_GPUCOPY_DIRECTION direction,
                                const uint32_t option,
                                CmEvent *&event);

    int32_t EnqueueUnalignedCopyInternal(CmSurface2DRT *surface,
                                         unsigned char *sysMem,
                                         const uint32_t widthStride,
                                         const uint32_t heightStride,
                                         CM_GPUCOPY_DIRECTION direction);

    int32_t FlushTaskWithoutSync(bool flushBlocked = false);

    int32_t GetTaskCount(uint32_t &numTasks);

    int32_t TouchFlushedTasks();

    int32_t GetTaskHasThreadArg(CmKernelRT *kernelArray[],
                                uint32_t numKernels,
                                bool &threadArgExists);
    int32_t CleanQueue();

    CM_QUEUE_CREATE_OPTION &GetQueueOption();

    int32_t GetOSSyncEventHandle(void *& hOSSyncEvent);

    uint32_t GetFastTrackerIndex() { return m_fastTrackerIndex; }

    uint32_t StreamIndex() const { return m_streamIndex; }

    int32_t EnqueueBufferCopy(  CmBuffer* buffer,
                                size_t   offset,
                                const unsigned char* sysMem,
                                uint64_t sysMemSize,
                                CM_GPUCOPY_DIRECTION dir,
                                CmEvent* wait_event,
                                CmEvent*& event,
                                uint32_t option);

protected:
    CmQueueRT(CmDeviceRT *device, CM_QUEUE_CREATE_OPTION queueCreateOption);

    ~CmQueueRT();

    int32_t Initialize();

    int32_t
    Enqueue_RT(CmKernelRT *kernelArray[],
               const uint32_t kernelCount,
               const uint32_t totalThreadCount,
               CmEventRT *&event,
               const CmThreadSpaceRT *threadSpace = nullptr,
               const uint64_t syncBitmap = 0,
               PCM_POWER_OPTION powerOption = nullptr,
               const uint64_t conditionalEndBitmap = 0,
               PCM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo = nullptr,
               CM_TASK_CONFIG *taskConfig = nullptr);

    int32_t Enqueue_RT(CmKernelRT *kernelArray[],
                       const uint32_t kernelCount,
                       const uint32_t totalThreadCount,
                       CmEventRT *&event,
                       const CmThreadGroupSpace *threadGroupSpace = nullptr,
                       const uint64_t syncBitmap = 0,
                       PCM_POWER_OPTION powerOption = nullptr,
                       const uint64_t conditionalEndBitmap = 0,
                       PCM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo = nullptr,
                       CM_TASK_CONFIG *taskConfig = nullptr,
                       const CM_EXECUTION_CONFIG* krnExecCfg = nullptr);

    int32_t Enqueue_RT(CmKernelRT *kernelArray[],
                       CmEventRT *&event,
                       uint32_t numTaskGenerated,
                       bool isLastTask,
                       uint32_t hints = 0,
                       PCM_POWER_OPTION powerOption = nullptr);

    int32_t QueryFlushedTasks();

    //New sub functions for different task flush
    int32_t FlushGeneralTask(CmTaskInternal *task);

    int32_t FlushGroupTask(CmTaskInternal *task);

    int32_t FlushVeboxTask(CmTaskInternal *task);

    int32_t FlushEnqueueWithHintsTask(CmTaskInternal *task);

    void PopTaskFromFlushedQueue();

    int32_t CreateEvent(CmTaskInternal *task,
                        bool isVisible,
                        int32_t &taskDriverId,
                        CmEventRT *&event);

    int32_t AddGPUCopyKernel(CM_GPUCOPY_KERNEL* &kernelParam);

    int32_t GetGPUCopyKrnID(uint32_t widthInByte,
                            uint32_t height,
                            CM_SURFACE_FORMAT format,
                            CM_GPUCOPY_DIRECTION copyDirection,
                            CM_GPUCOPY_KERNEL_ID &kernelID);

    int32_t AllocateGPUCopyKernel(uint32_t widthInByte,
                                  uint32_t height,
                                  CM_SURFACE_FORMAT format,
                                  CM_GPUCOPY_DIRECTION copyDirection,
                                  CmKernel* &kernel);

    int32_t CreateGPUCopyKernel(uint32_t widthInByte,
                                uint32_t height,
                                CM_SURFACE_FORMAT format,
                                CM_GPUCOPY_DIRECTION copyDirection,
                                CM_GPUCOPY_KERNEL* &gpuCopyKernelParam);

    int32_t SearchGPUCopyKernel(uint32_t widthInByte,
                                uint32_t height,
                                CM_SURFACE_FORMAT format,
                                CM_GPUCOPY_DIRECTION copyDirection,
                                CM_GPUCOPY_KERNEL* &kernelParam);

    int32_t RegisterSyncEvent();


    CmDeviceRT *m_device;
    ThreadSafeQueue m_enqueuedTasks;
    ThreadSafeQueue m_flushedTasks;

    CmDynamicArray m_eventArray;
    CSync m_criticalSectionEvent;        // Protect m_eventArray
    CSync m_criticalSectionHalExecute;   // Protect execution in HALCm, i.e HalCm_Execute
    CSync m_criticalSectionFlushedTask;  // Protect QueryFlushedTask
    CSync m_criticalSectionTaskInternal;

    uint32_t m_eventCount;
    uint64_t m_CPUperformanceFrequency;

    CmDynamicArray m_copyKernelParamArray;
    uint32_t m_copyKernelParamArrayCount;

    CSync m_criticalSectionGPUCopyKrn;

    CM_HAL_MAX_VALUES *m_halMaxValues;
    CM_QUEUE_CREATE_OPTION m_queueOption;

    bool m_usingVirtualEngine;
    MOS_VIRTUALENGINE_HINT_PARAMS m_mosVeHintParams;

    void  *m_osSyncEvent;   //KMD Notification

    uint32_t m_trackerIndex;
    uint32_t m_fastTrackerIndex;

private:
    static const uint32_t INVALID_SYNC_BUFFER_HANDLE = 0xDEADBEEF;

    //--------------------------------------------------------------------------------
    // Create a GPU context for this object.
    //--------------------------------------------------------------------------------
    MOS_STATUS CreateGpuContext(CM_HAL_STATE *halState,
                                MOS_GPU_CONTEXT gpuContextName,
                                MOS_GPU_NODE gpuNode,
                                MOS_GPUCTX_CREATOPTIONS *createOptions);

    //--------------------------------------------------------------------------------
    // Calls CM HAL API to submit a group task to command buffer.
    //--------------------------------------------------------------------------------
    MOS_STATUS ExecuteGroupTask(CM_HAL_STATE *halState,
                                CM_HAL_EXEC_TASK_GROUP_PARAM *taskParam,
                                MOS_GPU_CONTEXT gpuContextName);

    //--------------------------------------------------------------------------------
    // Calls CM HAL API to submit a general task to command buffer.
    //--------------------------------------------------------------------------------
    MOS_STATUS ExecuteGeneralTask(CM_HAL_STATE *halState,
                                  CM_HAL_EXEC_TASK_PARAM *taskParam,
                                  MOS_GPU_CONTEXT gpuContextName);

    //--------------------------------------------------------------------------------
    // Creates a buffer to synchronize all tasks in this queue.
    // It's useful only on certain operating systems.
    //--------------------------------------------------------------------------------
    MOS_STATUS CreateSyncBuffer(CM_HAL_STATE *halState);

    //--------------------------------------------------------------------------------
    // Selects sync buffer in this queue so CM HAL can add it to the command buffer.
    // It's useful only on certain operating systems.
    //--------------------------------------------------------------------------------
    MOS_STATUS SelectSyncBuffer(CM_HAL_STATE *halState);

    //--------------------------------------------------------------------------------
    // Releases sync buffer in this queue if it's created.
    //--------------------------------------------------------------------------------
    MOS_STATUS ReleaseSyncBuffer(CM_HAL_STATE *halState);

    uint32_t m_streamIndex;

    GPU_CONTEXT_HANDLE m_gpuContextHandle;

    // Handle of buffer resource for synchronizing tasks in this queue.
    uint32_t m_syncBufferHandle;

    CmQueueRT(const CmQueueRT& other);
    CmQueueRT& operator=(const CmQueueRT& other);
};
};  //namespace

#endif  // #ifnfef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMQUEUERT_H_
