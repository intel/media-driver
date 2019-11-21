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
#include "cm_csync.h"

class CmTracker;
class CmISH;
class CmDSH;
class CmSurfaceStateBufferMgr;
class CmSurfaceState2Dor3DMgr;
class CmSurfaceStateVME;
namespace CMRT_UMD
{
class CmDeviceRT;
class CmKernelRT;
class CmProgramRT;
class CmQueueRT;
class CmTask;
class CmThreadSpace;
class CmThreadGroupSpace;
class CmEvent;
};

class CmExecutionAdv
{
public:
    CmExecutionAdv();
    virtual ~CmExecutionAdv();
    virtual MOS_STATUS Initialize(CM_HAL_STATE *state);
    virtual CmTracker *GetTracker() {return m_tracker; }
    virtual CmISH *GetISH() {return m_ish; }
    virtual CmDSH *GetDSH() {return m_dsh; }
    virtual CmSurfaceState2Dor3DMgr *Create2DStateMgr(MOS_RESOURCE *resource);
    virtual CmSurfaceState2Dor3DMgr *Create3DStateMgr(MOS_RESOURCE *resource);
    virtual void Delete2Dor3DStateMgr(CmSurfaceState2Dor3DMgr *stateMgr);
    virtual CmSurfaceStateBufferMgr *CreateBufferStateMgr(MOS_RESOURCE *resource);
    virtual void DeleteBufferStateMgr(CmSurfaceStateBufferMgr *stateMgr);
    virtual void DeleteSurfStateVme(CmSurfaceStateVME *state);
    virtual void SetBufferOrigSize(CmSurfaceStateBufferMgr *stateMgr, uint32_t size);
    virtual void SetBufferMemoryObjectControl(CmSurfaceStateBufferMgr *stateMgr, uint16_t mocs);
    virtual void Set2Dor3DOrigFormat(CmSurfaceState2Dor3DMgr *stateMgr, MOS_FORMAT format);
    virtual void Set2Dor3DOrigDimension(CmSurfaceState2Dor3DMgr *stateMgr, uint32_t width, uint32_t height, uint32_t depth);
    virtual void Set2DRenderTarget(CmSurfaceState2Dor3DMgr *stateMgr, bool renderTarget);
    virtual void Set2Dor3DMemoryObjectControl(CmSurfaceState2Dor3DMgr *stateMgr, uint16_t mocs);
    virtual void Set2DFrameType(CmSurfaceState2Dor3DMgr *stateMgr, CM_FRAME_TYPE frameType);
    virtual void SetRotationFlag(CmSurfaceState2Dor3DMgr *stateMgr, uint32_t rotation);
    virtual void SetChromaSitting(CmSurfaceState2Dor3DMgr *stateMgr, uint8_t chromaSitting);
    virtual FrameTrackerProducer *GetFastTrackerProducer();
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
    virtual int SubmitComputeTask(CMRT_UMD::CmQueueRT *queue,
                CMRT_UMD::CmTask *task,
                CMRT_UMD::CmEvent* &event,
                const CMRT_UMD::CmThreadGroupSpace* threadGroupSpace,
                MOS_GPU_CONTEXT gpuContext);
    virtual int WaitForAllTasksFinished();

    virtual void SetL3Config(const L3ConfigRegisterValues *l3Config);

    virtual int SetSuggestedL3Config(L3_SUGGEST_CONFIG l3SuggestConfig);

    virtual int AssignNewTracker();

    virtual int SubmitGpgpuTask(CMRT_UMD::CmQueueRT *queue,
                CMRT_UMD::CmTask *task,
                CMRT_UMD::CmEvent* &event,
                const CMRT_UMD::CmThreadGroupSpace* threadGroupSpace,
                MOS_GPU_CONTEXT gpuContext);

    virtual bool SwitchToFastPath(CMRT_UMD::CmTask *task);

protected:
    int RefreshSurfaces(CMRT_UMD::CmDeviceRT *device);

    CM_HAL_STATE *m_cmhal;
    CmTracker *m_tracker;
    CmISH *m_ish;
    CmDSH *m_dsh;
    CMRT_UMD::CSync m_criticalSection;
    L3ConfigRegisterValues m_l3Values;
    
};

