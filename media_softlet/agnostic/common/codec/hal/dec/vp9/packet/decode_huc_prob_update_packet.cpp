/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_huc_prob_update_packet.cpp
//! \brief    Defines the interface for huc prob update packet for VP9 decode
//!
#include "decode_huc_prob_update_packet.h"
#include "mhw_vdbox.h"
#include "decode_resource_auto_lock.h"

namespace decode
{
MOS_STATUS HucVp9ProbUpdatePkt::AllocateResources()
{
    m_dmemBufferSize = MOS_ALIGN_CEIL(sizeof(HucVp9ProbBss), CODECHAL_CACHELINE_SIZE);
    if (m_probUpdateDmemBufferArray == nullptr)
    {
        m_probUpdateDmemBufferArray = m_allocator->AllocateBufferArray(
            m_dmemBufferSize, "DmemBuffer", m_numVp9ProbUpdateDmem, resourceInternalReadWriteCache, lockableVideoMem);
        DECODE_CHK_NULL(m_probUpdateDmemBufferArray);
    }

    if (m_interProbSaveBuffer == nullptr)
    {
        uint32_t interProbSaveBufferSize = MOS_ALIGN_CEIL(CODECHAL_VP9_INTER_PROB_SIZE, CODECHAL_PAGE_SIZE);
        m_interProbSaveBuffer            = m_allocator->AllocateBuffer(
            interProbSaveBufferSize, "VP9InterProbsSaveBuffer", resourceInternalReadWriteCache, notLockableVideoMem);
        DECODE_CHK_NULL(m_interProbSaveBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

HucVp9ProbUpdatePkt::~HucVp9ProbUpdatePkt()
{
    if (m_allocator != nullptr)
    {
        if (m_probUpdateDmemBufferArray)
        {
            m_allocator->Destroy(m_probUpdateDmemBufferArray);
        }

        if (m_interProbSaveBuffer)
        {
            m_allocator->Destroy(m_interProbSaveBuffer);
        }
    }
}

MOS_STATUS HucVp9ProbUpdatePkt::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodeHucBasic::Init());

    m_vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(m_vp9BasicFeature);

    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
    DECODE_CHK_STATUS(m_hwInterface->GetHucStateCommandSize(CODECHAL_DECODE_MODE_VP9VLD, &m_pictureStatesSize, &m_picturePatchListSize, &stateCmdSizeParams));

    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;
    m_hwInterface->GetCpInterface()->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    m_sliceStatesSize += cpCmdsize;
    m_slicePatchListSize += cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePkt::Prepare()
{
    DECODE_FUNC_CALL();
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(SetDmemBuffer());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    DECODE_FUNC_CALL();
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(commandBuffer);
    DECODE_CHK_STATUS(Execute(*commandBuffer, true));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePkt::Execute(MOS_COMMAND_BUFFER &cmdBuffer, bool prologNeeded)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(m_hucItf);

    if (prologNeeded)
    {
        DECODE_CHK_STATUS(AddForceWakeup(cmdBuffer, false, true));
        DECODE_CHK_STATUS(SendPrologCmds(cmdBuffer));
    }

    DECODE_CHK_STATUS(PackPictureLevelCmds(cmdBuffer));
    DECODE_CHK_STATUS(PackSliceLevelCmds(cmdBuffer));
    DECODE_CHK_STATUS(VdPipelineFlush(cmdBuffer));
    DECODE_CHK_STATUS(MemoryFlush(cmdBuffer));
    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_END(&cmdBuffer, nullptr));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePkt::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(AddCmd_HUC_IMEM_STATE(cmdBuffer));
    DECODE_CHK_STATUS(AddCmd_HUC_PIPE_MODE_SELECT(cmdBuffer));

    SETPAR_AND_ADDCMD(HUC_DMEM_STATE, m_hucItf, &cmdBuffer);
    SETPAR_AND_ADDCMD(HUC_VIRTUAL_ADDR_STATE, m_hucItf, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePkt::PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    SETPAR_AND_ADDCMD(HUC_START, m_hucItf, &cmdBuffer);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize      = CalculateCommandBufferSize();
    requestedPatchListSize = CalculatePatchListSize();

    return MOS_STATUS_SUCCESS;
}

uint32_t HucVp9ProbUpdatePkt::CalculateCommandBufferSize()
{
    DECODE_FUNC_CALL();

    uint32_t commandBufferSize = m_pictureStatesSize + m_sliceStatesSize;
    return (commandBufferSize + COMMAND_BUFFER_RESERVED_SPACE);
}

uint32_t HucVp9ProbUpdatePkt::CalculatePatchListSize()
{
    DECODE_FUNC_CALL();

    if (!m_osInterface->bUsesPatchList)
    {
        return 0;
    }

    uint32_t requestedPatchListSize = m_picturePatchListSize + m_slicePatchListSize;
    return requestedPatchListSize;
}

MOS_STATUS HucVp9ProbUpdatePkt::SetDmemBuffer()
{
    DECODE_FUNC_CALL();

    m_probUpdateDmemBuffer = m_probUpdateDmemBufferArray->Fetch();
    DECODE_CHK_NULL(m_probUpdateDmemBuffer);

    ResourceAutoLock resLock(m_allocator, &m_probUpdateDmemBuffer->OsResource);
    HucVp9ProbBss *  dmemBase = (HucVp9ProbBss *)resLock.LockResourceForWrite();
    DECODE_CHK_NULL(dmemBase);

    dmemBase->bSegProbCopy     = m_vp9BasicFeature->m_probUpdateFlags.bSegProbCopy;
    dmemBase->bProbSave        = m_vp9BasicFeature->m_probUpdateFlags.bProbSave;
    dmemBase->bProbRestore     = m_vp9BasicFeature->m_probUpdateFlags.bProbRestore;
    dmemBase->bProbReset       = m_vp9BasicFeature->m_probUpdateFlags.bProbReset;
    dmemBase->bResetFull       = m_vp9BasicFeature->m_probUpdateFlags.bResetFull;
    dmemBase->bResetKeyDefault = m_vp9BasicFeature->m_probUpdateFlags.bResetKeyDefault;
    MOS_SecureMemcpy(dmemBase->SegTreeProbs, 7, m_vp9BasicFeature->m_probUpdateFlags.SegTreeProbs, 7);
    MOS_SecureMemcpy(dmemBase->SegPredProbs, 3, m_vp9BasicFeature->m_probUpdateFlags.SegPredProbs, 3);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePkt::VdPipelineFlush(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_vdencItf->GETPAR_VD_PIPELINE_FLUSH();
    par       = {};
    par.waitDoneHEVC = 1;
    par.flushHEVC    = 1;
    par.waitDoneVDCmdMsgParser = 1;
    m_vdencItf->ADDCMD_VD_PIPELINE_FLUSH(&cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePkt::AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    auto &par = m_hucItf->MHW_GETPAR_F(HUC_IMEM_STATE)();
    par                  = {};
    par.kernelDescriptor = m_vdboxHucVp9ProbUpdateKernelDescriptor;
    DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_IMEM_STATE)(&cmdBuffer));
    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePkt::AddCmd_HUC_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    //for gen 11+, we need to add MFX wait for both KIN and VRT before and after HUC Pipemode select...
    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));
    auto &par = m_hucItf->MHW_GETPAR_F(HUC_PIPE_MODE_SELECT)();

    par                            = {};
    par.mediaSoftResetCounterValue = 2400;
    par.streamOutEnabled           = false;
    DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_PIPE_MODE_SELECT)(&cmdBuffer));

    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, HucVp9ProbUpdatePkt)
{
    DECODE_FUNC_CALL();
    params.hucDataSource = &m_probUpdateDmemBuffer->OsResource;
    params.dataLength    = MOS_ALIGN_CEIL(m_dmemBufferSize, CODECHAL_CACHELINE_SIZE);
    params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, HucVp9ProbUpdatePkt)
{
    DECODE_FUNC_CALL();

    PMOS_BUFFER vp9ProbBuffer = m_vp9BasicFeature->m_resVp9ProbBuffer[m_vp9BasicFeature->m_frameCtxIdx];
    DECODE_ASSERT(vp9ProbBuffer != nullptr);
    params.regionParams[3].presRegion = &vp9ProbBuffer->OsResource;
    params.regionParams[3].isWritable = true;
    params.regionParams[4].presRegion = &m_interProbSaveBuffer->OsResource;
    params.regionParams[4].isWritable = true;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_START, HucVp9ProbUpdatePkt)
{
    DECODE_FUNC_CALL();

    params.lastStreamObject = true;

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
