/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file      cm_command_buffer.h
//! \brief     Contains Class CmCommandBuffer  definitions
//!
#pragma once

#include "cm_hal.h"
#include "mos_os_specific.h"

class CmISH;
class CmMediaState;
class CmSSH;
class CmKernelEx;
namespace CMRT_UMD
{
class CmThreadSpaceRT;
class CmThreadGroupSpace;
};

class CmCommandBuffer
{
public:
    CmCommandBuffer(CM_HAL_STATE *cmhal);
    virtual ~CmCommandBuffer();

    MOS_STATUS AddFrameTracker(MOS_RESOURCE *resource, uint32_t offset, uint32_t tag);

    //--------------------------------------------------------------------------------
    // Adds a PIPE_CONTROL command to flush cache. It's also used to synchronize tasks in a CmQueue (GPU context).
    // Task synchronization is only needed on certain platforms. In most cases, this PIPE_CONTROL command is used for flushing cache exclusively.
    //--------------------------------------------------------------------------------
    MOS_STATUS AddFlushCacheAndSyncTask(bool isRead,
                                        bool rtCache,
                                        MOS_RESOURCE *syncBuffer);

    MOS_STATUS AddReadTimeStamp(MOS_RESOURCE *resource, uint32_t offset, bool isRead = false);
    MOS_STATUS AddL3CacheConfig(L3ConfigRegisterValues *l3values);
    MOS_STATUS AddPipelineSelect(bool gpgpu = false);
    MOS_STATUS AddStateBaseAddress(CmISH *ish, CmMediaState *mediaState);
    MOS_STATUS AddMediaVFE(CmMediaState *mediaState, bool fusedEuDispatch = false, CMRT_UMD::CmThreadSpaceRT **threadSpaces = nullptr, uint32_t count = 0);
    MOS_STATUS AddCurbeLoad(CmMediaState *mediaState);
    MOS_STATUS AddMediaIDLoad(CmMediaState *mediaState);

    //--------------------------------------------------------------------------------
    // Adds a PIPE_CONTROL command for synchronization between kernels in a batch.
    //--------------------------------------------------------------------------------
    MOS_STATUS AddSyncBetweenKernels();

    MOS_STATUS AddMediaObjectWalker(CMRT_UMD::CmThreadSpaceRT *threadSpace, uint32_t mediaID);
    MOS_STATUS AddDummyVFE();
    MOS_STATUS AddBatchBufferEnd();
    MOS_STATUS AddMMCProlog();
    MOS_STATUS AddProtectedProlog();
    MOS_STATUS AddConditionalBatchBufferEnd(CM_HAL_CONDITIONAL_BB_END_INFO *cbbInfo);
    MOS_STATUS AddConditionalFrameTracker(MOS_RESOURCE *resource, uint32_t offset, uint32_t tag, CM_HAL_CONDITIONAL_BB_END_INFO *cbbInfo);
    MOS_STATUS AddPowerOption(CM_POWER_OPTION *option);

    //UMD profiler related
    MOS_STATUS AddUmdProfilerStart();
    MOS_STATUS AddUmdProfilerEnd();

    //GPGPU walker specific
    MOS_STATUS AddPreemptionConfig(bool isGpgpu);
    MOS_STATUS AddSipState(uint32_t sipKernelOffset);
    MOS_STATUS AddCsrBaseAddress(MOS_RESOURCE *resource);
    MOS_STATUS AddGpgpuWalker(CMRT_UMD::CmThreadGroupSpace *threadGroupSpace,
                                    CmKernelEx *kernel,
                                    uint32_t mediaID);

    void ReturnUnusedBuffer();
    void ReturnWholeBuffer();

    MOS_STATUS Submit();

    MOS_STATUS Initialize();

    CmSSH* GetSSH();

    void Dump();

    inline MOS_RESOURCE *GetResource() {return &m_cmdBuf.OsResource; }
    
protected:
    
    CM_HAL_STATE *m_cmhal;
    MOS_INTERFACE *m_osInterface;
    PMHW_MI_INTERFACE m_miInterface;
    MhwRenderInterface *m_hwRender;
    MOS_COMMAND_BUFFER m_cmdBuf;
    CmSSH *m_ssh;

    uint8_t m_masks[16];
    
    int m_origRemain;
};
