/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_hevc_packet_front_end_m12.cpp
//! \brief    Defines the interface for hevc front end decode packet of M12
//!
#include "decode_hevc_packet_front_end_m12.h"
#include "decode_utils.h"
#include "decode_hevc_pipeline.h"
#include "decode_hevc_basic_feature.h"
#include "decode_status_report_defs.h"
#include "mos_solo_generic.h"
#include "decode_status_report_defs.h"
#include "decode_resource_auto_lock.h"
#include "hal_oca_interface.h"
#include "mhw_vdbox_vdenc_g12_X.h"

namespace decode
{

MOS_STATUS HevcDecodeFrontEndPktM12::Submit(
    MOS_COMMAND_BUFFER* cmdBuffer,
    uint8_t packetPhase)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_NULL(m_hwInterface);

    DECODE_CHK_STATUS(m_picturePkt->SetPhase(m_phase));

    DECODE_CHK_STATUS(Mos_Solo_PreProcessDecode(m_osInterface, &m_hevcBasicFeature->m_destSurface));

    DECODE_CHK_STATUS(m_miInterface->SetWatchdogTimerThreshold(m_hevcBasicFeature->m_width, m_hevcBasicFeature->m_height, false));

    if (IsPrologRequired())
    {
        DECODE_CHK_STATUS(AddForceWakeup(*cmdBuffer));
        DECODE_CHK_STATUS(SendPrologWithFrameTracking(*cmdBuffer, true));
    }

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1);
    HalOcaInterface::On1stLevelBBStart(*cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    HalOcaInterface::OnDispatch(*cmdBuffer, *m_osInterface, *m_miInterface, *m_miInterface->GetMmioRegisters());

    if (m_hevcPipeline->IsShortFormat())
    {
        MOS_RESOURCE* osResource = nullptr;
        uint32_t      offset = 0;

        DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatusMask, osResource, offset));

        // Check HuC_STATUS bit15, HW continue if bit15 > 0, otherwise send COND BB END cmd.
        uint32_t compareOperation = mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADGREATERTHANIDD;
        DECODE_CHK_STATUS(m_hwInterface->SendCondBbEndCmd(
                osResource, offset, 0, false, false, compareOperation, cmdBuffer));
    }

    DECODE_CHK_STATUS(PackPictureLevelCmds(*cmdBuffer));

    if (!m_hevcPipeline->IsShortFormat())
    {
        DECODE_CHK_STATUS(PackSliceLevelCmds(*cmdBuffer));
    }

    HalOcaInterface::DumpCodechalParam(*cmdBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, m_hevcPipeline->GetCodechalOcaDumper(), CODECHAL_HEVC);
    HalOcaInterface::On1stLevelBBEnd(*cmdBuffer, *m_osInterface);

    DECODE_CHK_STATUS(m_allocator->SyncOnResource(&m_hevcBasicFeature->m_resDataBuffer, false));

    DECODE_CHK_STATUS(Mos_Solo_PostProcessDecode(m_osInterface, &m_hevcBasicFeature->m_destSurface));

#if USE_CODECHAL_DEBUG_TOOL
    if (m_phase->GetCmdBufIndex() == DecodePhase::m_secondaryCmdBufIdxBase)
    {
        DECODE_CHK_STATUS(DumpSecondaryCommandBuffer(*cmdBuffer));
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeFrontEndPktM12::VdMemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer)
{
    MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
    MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdCtrlParam.memoryImplicitFlush = true;

    MhwMiInterfaceG12* miInterfaceG12 = dynamic_cast<MhwMiInterfaceG12*>(m_miInterface);
    DECODE_CHK_NULL(miInterfaceG12);
    DECODE_CHK_STATUS(miInterfaceG12->AddMiVdControlStateCmd(&cmdBuffer, &vdCtrlParam));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeFrontEndPktM12::VdPipelineFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS_G12 vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams, sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bWaitDoneHEVC = 1;
    vdpipeFlushParams.Flags.bFlushHEVC = 1;
    vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    DECODE_CHK_STATUS(m_vdencInterface->AddVdPipelineFlushCmd(
        &cmdBuffer, (MHW_VDBOX_VD_PIPE_FLUSH_PARAMS*)&vdpipeFlushParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeFrontEndPktM12::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(StartStatusReport(statusReportMfx, &cmdBuffer));

    DECODE_CHK_STATUS(m_picturePkt->Execute(cmdBuffer));

    PMHW_BATCH_BUFFER batchBuffer = m_hevcPipeline->GetSliceLvlCmdBuffer();
    DECODE_CHK_NULL(batchBuffer);
    batchBuffer->dwOffset = 0;
    DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, batchBuffer));

    DECODE_CHK_STATUS(VdMemoryFlush(cmdBuffer));
    DECODE_CHK_STATUS(ReadVdboxId(cmdBuffer));
    DECODE_CHK_STATUS(VdPipelineFlush(cmdBuffer));
    DECODE_CHK_STATUS(MiFlush(cmdBuffer));

    DECODE_CHK_STATUS(m_picturePkt->ReportCabacStreamOutSize(cmdBuffer));
    DECODE_CHK_STATUS(MiFlush(cmdBuffer));

    DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeFrontEndPktM12::PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    PMHW_BATCH_BUFFER batchBuffer = m_hevcPipeline->GetSliceLvlCmdBuffer();
    DECODE_CHK_NULL(batchBuffer);

    ResourceAutoLock resLock(m_allocator, &batchBuffer->OsResource);
    uint32_t *batchBufBase = (uint32_t *)resLock.LockResourceForWrite();
    DECODE_CHK_NULL(batchBufBase);

    MOS_COMMAND_BUFFER sliceLevelCmdBuffer;
    MOS_ZeroMemory(&sliceLevelCmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    sliceLevelCmdBuffer.pCmdBase   = batchBufBase;
    sliceLevelCmdBuffer.pCmdPtr    = sliceLevelCmdBuffer.pCmdBase;
    sliceLevelCmdBuffer.iRemaining = batchBuffer->iSize;

    for (uint32_t i = 0; i < m_hevcBasicFeature->m_numSlices; i++)
    {
        DECODE_CHK_STATUS(m_slicePkt->Execute(sliceLevelCmdBuffer, i, 0));
    }

    DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(&sliceLevelCmdBuffer, nullptr));
    return MOS_STATUS_SUCCESS;
}

}

