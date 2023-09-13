/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file     vphal_mdf_wrapper.cpp
//! \brief    Abstraction for MDF related operations.
//! \details  It is a thin wrapper layer based on MDF APIs.
//!
#include "vphal_mdf_wrapper.h"
#include <algorithm>
#include <cstdio>


void EventManager::OnEventAvailable(CmEvent *event, const std::string &name)
{
    AddEvent(name, event);
}

void EventManager::AddEvent(const std::string &name, CmEvent *event)
{
    if (mEventCount >= (128 * 1024) / sizeof(CmEvent))
    {
        if (mReport)
        {
            Profiling();
        }

        Clear();
    }

    mEventMap[name].push_back(event);
    mLastEvent = event;
    mEventCount++;
}

void EventManager::Clear()
{
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(m_cmContext);
    CmQueue *queue = m_cmContext->GetCmQueue();
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(queue);
    for (auto it : mEventMap)
    {
        for (CmEvent *event : it.second)
        {
            queue->DestroyEvent(event);
        }
    }

    mEventMap.clear();
    mEventCount = 0;
    mLastEvent = nullptr;
}

void EventManager::Profiling() const
{
    VPHAL_RENDER_NORMALMESSAGE("------------------------%s Profiling Report------------------------\n", mOwner.c_str());
    for (auto it : mEventMap)
    {
        int count = 0;
        double totalTimeInMS = 0.0;
        for (CmEvent *event : it.second)
        {
            uint64_t executionTimeInNS = 0;
            int result = event->GetExecutionTime(executionTimeInNS);
            if (result != CM_SUCCESS)
            {
                VPHAL_RENDER_ASSERTMESSAGE("[%s]: CM GetExecutionTime error: %d\n", it.first.c_str(), result);
                continue;
            }
            totalTimeInMS += executionTimeInNS / 1000000.0;
            count++;
        }
        VPHAL_RENDER_NORMALMESSAGE("[%s]: execution count %llu, average time %f ms.\n", it.first.c_str(), it.second.size(), totalTimeInMS / count);
    }
    VPHAL_RENDER_NORMALMESSAGE("------------------------%s Profiling Report End------------------------\n", mOwner.c_str());
}

CmEvent* EventManager::GetLastEvent() const
{
    return mLastEvent;
}

CmContext::CmContext(PMOS_INTERFACE osInterface) :
    mRefCount(0),
    mCmDevice(nullptr),
    mCmQueue(nullptr),
    mCmVebox(nullptr),
    m_osInterface(osInterface),
    mBatchTask(nullptr),
    mHasBatchedTask(false),
    mConditionalBatchBuffer(nullptr),
    mCondParam({ 0 }),
    mEventListener(nullptr)
{
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(osInterface);

    const unsigned int MDF_DEVICE_CREATE_OPTION =
        ((CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE)                                |
         (CM_DEVICE_CONFIG_DSH_DISABLE_MASK)                                            |
         (CM_DEVICE_CONFIG_TASK_NUM_16 << CM_DEVICE_CONFIG_TASK_NUM_OFFSET)             |
         (CM_DEVICE_CONFIG_MEDIA_RESET_ENABLE)                                          |
         (CM_DEVICE_CONFIG_EXTRA_TASK_NUM_4 << CM_DEVICE_CONFIG_EXTRA_TASK_NUM_OFFSET)  |
         (CM_DEVICE_CONFIG_GPUCONTEXT_ENABLE)                                           |
         (32 << CM_DEVICE_CONFIG_KERNELBINARYGSH_OFFSET));

    int result = osInterface->pfnCreateCmDevice(osInterface->pOsContext, mCmDevice, MDF_DEVICE_CREATE_OPTION, CM_DEVICE_CREATE_PRIORITY_DEFAULT);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("CmDevice creation error %d\n", result);
        return;
    }

    result = mCmDevice->CreateQueue(mCmQueue);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("CmQueue creation error %d\n", result);
        return;
    }

    result = mCmDevice->CreateVebox(mCmVebox);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("CmVebox creation error %d\n", result);
        return;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    result = mCmDevice->InitPrintBuffer(32768);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Init printf error: %d\n", result);
        return;
    }
#endif

    result = mCmDevice->CreateTask(mBatchTask);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Create batch task error: %d\n", result);
        return;
    }

}

CmKernel* CmContext::CloneKernel(CmKernel *kernel)
{
    auto it = std::find(mAddedKernels.begin(), mAddedKernels.end(), kernel);
    if (it != mAddedKernels.end())
    {
        CmKernel *newKernel = nullptr;
        int result = mCmDevice->CloneKernel(newKernel, kernel);
        if (result != CM_SUCCESS)
        {
            // Clone kernel failed, try to use the old one.
            VPHAL_RENDER_ASSERTMESSAGE("Clone kernel failed: %d\n", result);
            return kernel;
        }
        mKernelsToPurge.push_back(newKernel);
        return newKernel;
    }
    else
    {
        return kernel;
    }
}

void CmContext::BatchKernel(CmKernel *kernel, CmThreadSpace *threadSpace, bool bFence)
{
    int result;

    if (mConditionalBatchBuffer && mAddedKernels.empty())
    {
        result = mBatchTask->AddConditionalEnd(mConditionalBatchBuffer->GetCmSurfaceIndex(), 0, &mCondParam);
        if (result != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Batch task AddConditionalEnd error: %d\n", result);
            return;
        }
    }

    if (bFence)
    {
        result = mBatchTask->AddSync();
        if (result != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Batch task add sync error: %d\n", result);
            return;
        }
    }

    result = mBatchTask->AddKernel(kernel);
    if (result == CM_EXCEED_MAX_KERNEL_PER_ENQUEUE)
    {
        // Reach max kernels per task, flush and try again.
        bool needAddBack = false;
        if (mKernelsToPurge.back() == kernel)
        {
            mKernelsToPurge.pop_back();
            needAddBack = true;
        }

        FlushBatchTask(false);
        BatchKernel(kernel, threadSpace, false);

        if (needAddBack)
        {
            mKernelsToPurge.push_back(kernel);
        }

        return;
    }
    else if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Batch task add sync error: %d\n", result);
        return;
    }

    mAddedKernels.push_back(kernel);
    mThreadSpacesToPurge.push_back(threadSpace);
    mHasBatchedTask = true;
}

void CmContext::FlushBatchTask(bool waitForFinish)
{
    int result = CM_SUCCESS;

    if (mAddedKernels.empty())
    {
        return;
    }

    EnqueueTask(mBatchTask, nullptr, "BatchTask", waitForFinish);

    for(auto it : mThreadSpacesToPurge)
    {
        result = mCmDevice->DestroyThreadSpace(it);
        if (result != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("CM DestroyThreadSpace Fail %d", result);
        }
    }

    for(auto it : mKernelsToPurge)
    {
        result = mCmDevice->DestroyKernel(it);
        if (result != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("CM DestroyKernel Fail %d", result);
        }
    }

    mThreadSpacesToPurge.clear();
    mKernelsToPurge.clear();
    mAddedKernels.clear();
    result = mBatchTask->Reset();
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("CM Batch Task Reset Fail %d", result);
    }
}

void CmContext::RunSingleKernel(
    CmKernel *kernel,
    CmThreadSpace *threadSpace,
    const std::string &name,
    bool waitForFinish)
{
    FlushBatchTask(false);

    CmTask *task = nullptr;
    int result = mCmDevice->CreateTask(task);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: CmDevice CreateTask error: %d\n", name.c_str(), result);
        return;
    }

    if (mConditionalBatchBuffer)
    {
        result = task->AddConditionalEnd(mConditionalBatchBuffer->GetCmSurfaceIndex(), 0, &mCondParam);
        if (result != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("[%s]: AddConditionalEnd error: %d\n", name.c_str(), result);
            mCmDevice->DestroyTask(task);
            return;
        }
    }

    result = task->AddKernel(kernel);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: CmDevice AddKernel error: %d\n", name.c_str(), result);
        mCmDevice->DestroyTask(task);
        return;
    }

    EnqueueTask(task, threadSpace, name, waitForFinish);
}

void CmContext::EnqueueTask(CmTask *task, CmThreadSpace *threadSpace, const std::string &name, bool waitForFinish)
{
    CmEvent *event = nullptr;
    int result = mCmQueue->Enqueue(task, event, threadSpace);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: CmDevice enqueue error: %d\n", name.c_str(), result);
        return;
    }

    if (waitForFinish)
    {
        result = event->WaitForTaskFinished(-1);
        if (result != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("WaitForTaskFinished Failed %d", result);
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        result = mCmDevice->FlushPrintBuffer();
        if (result != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("[%s]: Flush printf buffer error: %d", name.c_str(), result);
        }
        std::fflush(stdout);
#endif
    }

    if (mEventListener)
    {
        mEventListener->OnEventAvailable(event, name);
    }
    else
    {
        if (event != nullptr)
        {
            result = mCmQueue->DestroyEvent(event);
            if (result != CM_SUCCESS)
            {
                VPHAL_RENDER_ASSERTMESSAGE("DestroyEvent Failed %d", result);
            }
        }
    }
}

void CmContext::Destroy()
{
    int result = CM_SUCCESS;
    FlushBatchTask(false);

    if (mBatchTask)
    {
        result = mCmDevice->DestroyTask(mBatchTask);
        if (result != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("CM DestroyTask Fail %d", result);
        }
    }

    if (mCmVebox)
    {
        result = mCmDevice->DestroyVebox(mCmVebox);
        if (result != CM_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("CM DestroyVebox Fail %d", result);
        }
    }

    if (mCmDevice && m_osInterface)
    {
        m_osInterface->pfnDestroyCmDevice(mCmDevice);
    }

    mBatchTask = nullptr;
    mCmVebox   = nullptr;
    mCmDevice  = nullptr;
}

CmContext::~CmContext()
{
    Destroy();
}

VPCmRenderer::VPCmRenderer(const std::string &name, CmContext *cmContext) :
    mName(name),
    m_cmContext(cmContext),
    mBatchDispatch(true),
    mBlockingMode(false),
    mEnableDump(false)
{
}

VPCmRenderer::~VPCmRenderer()
{
}

CmProgram* VPCmRenderer::LoadProgram(const std::string& binaryFileName)
{
    std::ifstream isa(binaryFileName, std::ifstream::ate | std::ifstream::binary);

    if (!isa.is_open()) 
    {
        VPHAL_RENDER_ASSERTMESSAGE("Error in opening ISA file: %s.\n", binaryFileName.c_str());
        return nullptr;
    }

    int size = static_cast<int>(isa.tellg());
    if (size == 0)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Code size is 0.\n");
        return nullptr;
    }

    isa.seekg(0, isa.beg);
    std::vector<char> code(size);
    isa.read(code.data(), size);

    CmProgram *program = nullptr;
    if (!m_cmContext)
    {
        return nullptr;
    }
    CmDevice *dev    = m_cmContext->GetCmDevice();
    if (!dev)
    {
        return nullptr;
    }
    int       result = dev->LoadProgram(code.data(), size, program, "-nojitter");
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: CM LoadProgram error %d\n", mName.c_str(), result);
        return nullptr;
    }

    return program;
}

CmProgram* VPCmRenderer::LoadProgram(const void *binary, int size)
{
    if (!binary || size == 0)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid program to load.\n");
        return nullptr;
    }

    CmProgram *program = nullptr;
    if (!m_cmContext)
    {
        return nullptr;
    }
    CmDevice *dev    = m_cmContext->GetCmDevice();
    if (!dev)
    {
        return nullptr;
    }
    int       result = dev->LoadProgram(const_cast<void *>(binary), size, program, "-nojitter");
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: CM LoadProgram error %d\n", mName.c_str(), result);
        return nullptr;
    }

    return program;
}

void VPCmRenderer::Render(void *payload)
{
    AttachPayload(payload);

    std::string kernelName;
    CmKernel *kernel = GetKernelToRun(kernelName);
    if (!kernel)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: Did not find proper kernel to run\n", mName.c_str());
        return;
    }

    int tsWidth, tsHeight, tsColor;
    GetThreadSpaceDimension(tsWidth, tsHeight, tsColor);
    if (!tsWidth || !tsHeight || !tsColor)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: Degenerate thread space, return immediately.\n", mName.c_str());
        return;
    }

    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(m_cmContext);
    CmThreadSpace *threadSpace = nullptr;
    CmDevice *dev = m_cmContext->GetCmDevice();
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(dev);
    int result = dev->CreateThreadSpace(tsWidth, tsHeight, threadSpace);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: CM Create ThreadSpace error: %d\n", mName.c_str(), result);
        return;
    }

    SetupThreadSpace(threadSpace, tsWidth, tsHeight, tsColor);
    
    // We need to use CloneKernel API to add the same kernel multiple times into one task.
    bool bBatch = mBatchDispatch && !mBlockingMode && !mEnableDump && !CannotAssociateThreadSpace();
    if (bBatch)
    {
        kernel = m_cmContext->CloneKernel(kernel);
    }

    result = kernel->SetThreadCount(tsWidth * tsHeight * tsColor);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: CM Set ThreadCount error: %d\n", mName.c_str(), result);
    }
   
    if (!CannotAssociateThreadSpace())
    {
        kernel->AssociateThreadSpace(threadSpace);
    }

    PrepareKernel(kernel);

    if (bBatch)
    {
        m_cmContext->BatchKernel(kernel, threadSpace, NeedAddSync());
    }
    else
    {
        m_cmContext->RunSingleKernel(kernel, CannotAssociateThreadSpace() ? threadSpace : nullptr, kernelName, mBlockingMode);
        dev->DestroyThreadSpace(threadSpace);
    }

    if (mEnableDump)
    {
        Dump();
    }

    AttachPayload(nullptr);
}