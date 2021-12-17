/*
* Copyright (c) 2021, Intel Corporation
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
#include "decode_huc_prob_update_packet_m12.h"
#include "mhw_vdbox.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_vdbox_vdenc_g12_X.h"
#include "decode_resource_auto_lock.h"

namespace decode
{
MOS_STATUS HucVp9ProbUpdatePktM12::AllocateResources()
{
    m_dmemBufferSize = MOS_ALIGN_CEIL(sizeof(HucVp9ProbBssM12), CODECHAL_CACHELINE_SIZE);
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

HucVp9ProbUpdatePktM12::~HucVp9ProbUpdatePktM12()
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

MOS_STATUS HucVp9ProbUpdatePktM12::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodeHucBasic_G12_Base::Init());

    m_vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(m_vp9BasicFeature);

    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
    DECODE_CHK_STATUS(m_hwInterface->GetHucStateCommandSize(CODECHAL_DECODE_MODE_VP9VLD, &m_pictureStatesSize, &m_picturePatchListSize, &stateCmdSizeParams));

    DECODE_CHK_STATUS(m_hwInterface->GetHucPrimitiveCommandSize(CODECHAL_DECODE_MODE_VP9VLD,
        &m_sliceStatesSize,
        &m_slicePatchListSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePktM12::Prepare()
{
    DECODE_FUNC_CALL();
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(SetDmemBuffer());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePktM12::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    DECODE_FUNC_CALL();
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(commandBuffer);
    DECODE_CHK_STATUS(Execute(*commandBuffer, true));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePktM12::Execute(MOS_COMMAND_BUFFER &cmdBuffer, bool prologNeeded)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(m_hucInterface);

    if (prologNeeded)
    {
        DECODE_CHK_STATUS(AddForceWakeup(cmdBuffer, false, true));
        DECODE_CHK_STATUS(SendPrologCmds(cmdBuffer));
    }

    DECODE_CHK_STATUS(PackPictureLevelCmds(cmdBuffer));
    DECODE_CHK_STATUS(PackSliceLevelCmds(cmdBuffer));
    DECODE_CHK_STATUS(VdPipelineFlush(cmdBuffer));
    DECODE_CHK_STATUS(MemoryFlush(cmdBuffer));
    DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePktM12::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(AddHucImem(cmdBuffer));
    DECODE_CHK_STATUS(AddHucPipeModeSelect(cmdBuffer));
    DECODE_CHK_STATUS(AddHucDmem(cmdBuffer));
    DECODE_CHK_STATUS(AddHucRegion(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePktM12::PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePktM12::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    commandBufferSize      = CalculateCommandBufferSize();
    requestedPatchListSize = CalculatePatchListSize();

    return MOS_STATUS_SUCCESS;
}

uint32_t HucVp9ProbUpdatePktM12::CalculateCommandBufferSize()
{
    DECODE_FUNC_CALL();

    uint32_t commandBufferSize = m_pictureStatesSize + m_sliceStatesSize;
    return (commandBufferSize + COMMAND_BUFFER_RESERVED_SPACE);
}

uint32_t HucVp9ProbUpdatePktM12::CalculatePatchListSize()
{
    DECODE_FUNC_CALL();

    if (!m_osInterface->bUsesPatchList)
    {
        return 0;
    }

    uint32_t requestedPatchListSize = m_picturePatchListSize + m_slicePatchListSize;
    return requestedPatchListSize;
}

void HucVp9ProbUpdatePktM12::SetImemParameters(MHW_VDBOX_HUC_IMEM_STATE_PARAMS &imemParams)
{
    DECODE_FUNC_CALL();
    imemParams.dwKernelDescriptor = m_vdboxHucVp9ProbUpdateKernelDescriptor;
}

MOS_STATUS HucVp9ProbUpdatePktM12::AddHucImem(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    SetImemParameters(imemParams);

    DECODE_CHK_STATUS(m_hucInterface->AddHucImemStateCmd(&cmdBuffer, &imemParams));
    return MOS_STATUS_SUCCESS;
}

void HucVp9ProbUpdatePktM12::SetHucPipeModeSelectParameters(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &pipeModeSelectParams)
{
    DECODE_FUNC_CALL();
    pipeModeSelectParams.dwMediaSoftResetCounterValue = m_mediaResetCounter;
}

MOS_STATUS HucVp9ProbUpdatePktM12::AddHucPipeModeSelect(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    SetHucPipeModeSelectParameters(pipeModeSelectParams);

    DECODE_CHK_STATUS(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));
    return MOS_STATUS_SUCCESS;
}

void HucVp9ProbUpdatePktM12::SetDmemParameters(MHW_VDBOX_HUC_DMEM_STATE_PARAMS &dmemParams)
{
    DECODE_FUNC_CALL();
    dmemParams.presHucDataSource = &m_probUpdateDmemBuffer->OsResource;
    dmemParams.dwDataLength      = MOS_ALIGN_CEIL(m_dmemBufferSize, CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset      = HUC_DMEM_OFFSET_RTOS_GEMS;
}

MOS_STATUS HucVp9ProbUpdatePktM12::AddHucDmem(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    SetDmemParameters(dmemParams);

    DECODE_CHK_STATUS(m_hucInterface->AddHucDmemStateCmd(&cmdBuffer, &dmemParams));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePktM12::SetDmemBuffer()
{
    DECODE_FUNC_CALL();

    m_probUpdateDmemBuffer = m_probUpdateDmemBufferArray->Fetch();
    DECODE_CHK_NULL(m_probUpdateDmemBuffer);

    ResourceAutoLock resLock(m_allocator, &m_probUpdateDmemBuffer->OsResource);
    HucVp9ProbBssM12 *  dmemBase = (HucVp9ProbBssM12 *)resLock.LockResourceForWrite();
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

void HucVp9ProbUpdatePktM12::SetRegionParameters(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS &virtualAddrParams)
{
    DECODE_FUNC_CALL();

    PMOS_BUFFER vp9ProbBuffer = m_vp9BasicFeature->m_resVp9ProbBuffer[m_vp9BasicFeature->m_frameCtxIdx];
    DECODE_ASSERT(vp9ProbBuffer != nullptr);
    virtualAddrParams.regionParams[3].presRegion = &vp9ProbBuffer->OsResource;
    virtualAddrParams.regionParams[3].isWritable = true;
    virtualAddrParams.regionParams[4].presRegion = &m_interProbSaveBuffer->OsResource;
    virtualAddrParams.regionParams[4].isWritable = true;
}

MOS_STATUS HucVp9ProbUpdatePktM12::AddHucRegion(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    SetRegionParameters(virtualAddrParams);

    DECODE_CHK_STATUS(m_hucInterface->AddHucVirtualAddrStateCmd(&cmdBuffer, &virtualAddrParams));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePktM12::VdPipelineFlush(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams, sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bWaitDoneHEVC           = 1;
    vdpipeFlushParams.Flags.bFlushHEVC              = 1;
    vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    DECODE_CHK_STATUS(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, (MHW_VDBOX_VD_PIPE_FLUSH_PARAMS *)&vdpipeFlushParams));

    return MOS_STATUS_SUCCESS;
}

void HucVp9ProbUpdatePktM12::SetIndObjParameters(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjParams)
{
    DECODE_FUNC_CALL();
}

MOS_STATUS HucVp9ProbUpdatePktM12::AddHucStreamObject(MOS_COMMAND_BUFFER &cmdBuffer, CODEC_HEVC_SLICE_PARAMS &sliceParams)
{
    DECODE_FUNC_CALL();
    return MOS_STATUS_SUCCESS;
}

void HucVp9ProbUpdatePktM12::SetStreamObjectParameters(MHW_VDBOX_HUC_STREAM_OBJ_PARAMS &streamObjParams,
    CODEC_HEVC_SLICE_PARAMS &                                                        sliceParams)
{
    DECODE_FUNC_CALL();
}

MOS_STATUS HucVp9ProbUpdatePktM12::AddHucIndObj(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
