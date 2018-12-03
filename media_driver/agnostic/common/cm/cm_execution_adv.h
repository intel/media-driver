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
//! \file      cm_execution_adv.h
//! \brief     Contains Class CmExecutionAdv  definitions 
//!
#pragma once

#include "cm_hal.h"

class CmTracker;
class CmISH;
class CmDSH;
class CmSurfaceStateBufferMgr;
class CmSurfaceState2DMgr;
class CmSurfaceStateVME;
namespace CMRT_UMD
{
class CmDeviceRT;
class CmKernelRT;
class CmProgramRT;
class CmQueueRT;
class CmTask;
class CmThreadSpace;
class CmEvent;
};
class CmExecutionAdv
{
public:
    CmExecutionAdv() {}
    virtual ~CmExecutionAdv() {}
    virtual MOS_STATUS Initialize(CM_HAL_STATE *state) {return MOS_STATUS_SUCCESS; }
    virtual CmTracker *GetTracker() {return nullptr; }
    virtual CmISH *GetISH() {return nullptr; }
    virtual CmDSH *GetDSH() {return nullptr; }
    virtual CmSurfaceState2DMgr *Create2DStateMgr(MOS_RESOURCE *resource) {return nullptr; }
    virtual void Delete2DStateMgr(CmSurfaceState2DMgr *stateMgr) {}
    virtual CmSurfaceStateBufferMgr *CreateBufferStateMgr(MOS_RESOURCE *resource) {return nullptr; }
    virtual void DeleteBufferStateMgr(CmSurfaceStateBufferMgr *stateMgr) {}
    virtual void DeleteSurfStateVme(CmSurfaceStateVME *state) {}
    virtual void SetBufferOrigSize(CmSurfaceStateBufferMgr *stateMgr, uint32_t size) {}
    virtual void SetBufferMemoryObjectControl(CmSurfaceStateBufferMgr *stateMgr, uint16_t mocs) {}
    virtual void Set2DRenderTarget(CmSurfaceState2DMgr *stateMgr, bool renderTarget) {}
    virtual void Set2DMemoryObjectControl(CmSurfaceState2DMgr *stateMgr, uint16_t mocs) {}
    virtual void Set2DFrameType(CmSurfaceState2DMgr *stateMgr, CM_FRAME_TYPE frameType) {}
    virtual CMRT_UMD::CmKernelRT *CreateKernelRT(CMRT_UMD::CmDeviceRT *device,
                CMRT_UMD::CmProgramRT *program,
                uint32_t kernelIndex,
                uint32_t kernelSeqNum);

    virtual int SubmitTask(CMRT_UMD::CmQueueRT *queue, 
                CMRT_UMD::CmTask *task, 
                CMRT_UMD::CmEvent *&event,
                const CMRT_UMD::CmThreadSpace *threadSpace,
                MOS_GPU_CONTEXT gpuContext);

    virtual int DestoryEvent(CMRT_UMD::CmQueueRT *queue, CMRT_UMD::CmEvent *&event);

    
};
