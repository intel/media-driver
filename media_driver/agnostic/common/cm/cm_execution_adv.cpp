/*
* Copyright (c) 2018, Intel Corporation
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
//! \file      cm_execution_adv.cpp 
//! \brief     Contains Class CmExecutionAdv  definitions 
//!
#include "cm_execution_adv.h"
#include "cm_kernel_rt.h"
#include "cm_extension_creator.h"
#include "cm_queue_rt.h"

static bool advRegistered = CmExtensionCreator<CmExecutionAdv>::RegisterClass<CmExecutionAdv>();

using namespace CMRT_UMD;

CmKernelRT *CmExecutionAdv::CreateKernelRT(CmDeviceRT *device,
               CmProgramRT *program,
               uint32_t kernelIndex,
               uint32_t kernelSeqNum)
{
    return new (std::nothrow) CmKernelRT(device, program, kernelIndex, kernelSeqNum);
}

int CmExecutionAdv::SubmitTask(CMRT_UMD::CmQueueRT *queue, 
                CMRT_UMD::CmTask *task, 
                CMRT_UMD::CmEvent *&event,
                const CMRT_UMD::CmThreadSpace *threadSpace,
                MOS_GPU_CONTEXT gpuContext)
{
    CM_CHK_NULL_RETURN_CMERROR(queue);
    return queue->Enqueue(task, event, threadSpace);
}

int CmExecutionAdv::DestoryEvent(CMRT_UMD::CmQueueRT *queue, CMRT_UMD::CmEvent *&event)
{
    CM_CHK_NULL_RETURN_CMERROR(queue);
    return queue->DestroyEvent(event);
}

int CmExecutionAdv::SubmitComputeTask(CMRT_UMD::CmQueueRT *queue,
                CMRT_UMD::CmTask *task,
                CMRT_UMD::CmEvent* &event, 
                const CMRT_UMD::CmThreadGroupSpace* threadGroupSpace,
                MOS_GPU_CONTEXT gpuContext)
{
    CM_CHK_NULL_RETURN_CMERROR(queue);
    return queue->EnqueueWithGroup(task, event, threadGroupSpace);
}

