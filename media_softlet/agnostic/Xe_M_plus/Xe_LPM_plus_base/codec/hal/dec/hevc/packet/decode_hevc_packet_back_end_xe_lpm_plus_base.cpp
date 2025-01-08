/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_hevc_packet_back_end_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for hevc back end decode packet of Xe_LPM_plus+
//!
#include "decode_hevc_packet_back_end_xe_lpm_plus_base.h"
#include "decode_utils.h"
#include "decode_hevc_pipeline.h"
#include "decode_hevc_basic_feature.h"
#include "decode_status_report_defs.h"
#include "mos_solo_generic.h"
#include "decode_status_report_defs.h"
#include "decode_resource_auto_lock.h"
#include "hal_oca_interface_next.h"
#include "mhw_vdbox_vdenc_hwcmd_xe_lpm_plus.h"
#include "decode_hevc_picture_packet_xe_lpm_plus_base.h"
#include "mhw_vdbox_xe_lpm_plus_base.h"
#include "mhw_mi_hwcmd_xe_lpm_plus_base_next.h"

using namespace mhw::vdbox::xe_lpm_plus_base;

namespace decode
{

MOS_STATUS HevcDecodeBackEndPktXe_Lpm_Plus_Base::Init()
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(HevcDecodeBackEndPkt::Init());

    DecodeSubPacket* subPacket = m_hevcPipeline->GetSubPacket(DecodePacketId(m_hevcPipeline, hevcTileSubPacketId));
    m_tilePkt = dynamic_cast<HevcDecodeTilePktXe_Lpm_Plus_Base*>(subPacket);
    DECODE_CHK_NULL(m_tilePkt);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeBackEndPktXe_Lpm_Plus_Base::Submit(
    MOS_COMMAND_BUFFER* cmdBuffer,
    uint8_t packetPhase)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_NULL(m_hwInterface);

    DECODE_CHK_STATUS(m_picturePkt->SetPhase(m_phase));

    DECODE_CHK_STATUS(Mos_Solo_PreProcessDecode(m_osInterface, &m_hevcBasicFeature->m_destSurface));

    DECODE_CHK_STATUS(m_miItf->SetWatchdogTimerThreshold(m_hevcBasicFeature->m_width, m_hevcBasicFeature->m_height, false));

    if (IsPrologRequired())
    {
        DECODE_CHK_STATUS(AddForceWakeup(*cmdBuffer));
        DECODE_CHK_STATUS(SendPrologWithFrameTracking(*cmdBuffer, true));
    }

    DECODE_CHK_NULL(m_hwInterface->GetVdencInterfaceNext());
    auto mmioRegisters = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(MHW_VDBOX_NODE_1);
    HalOcaInterfaceNext::On1stLevelBBStart(*cmdBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, m_miItf, *mmioRegisters);
    HalOcaInterfaceNext::OnDispatch(*cmdBuffer, *m_osInterface, m_miItf, *m_miItf->GetMmioRegisters());

    DECODE_CHK_STATUS(m_miItf->AddWatchdogTimerStopCmd(cmdBuffer));
    auto scalability = m_hevcPipeline->GetMediaScalability();
    DECODE_CHK_STATUS(scalability->SyncPipe(syncAllPipes, 0, cmdBuffer));

    HevcDecodePicPktXe_Lpm_Plus_Base *picturePkt = dynamic_cast<HevcDecodePicPktXe_Lpm_Plus_Base *>(m_picturePkt);
    DECODE_CHK_NULL(picturePkt);
    DECODE_CHK_STATUS(picturePkt->ValidateCabacStreamOutSize(*cmdBuffer));

    if (m_hevcPipeline->IsShortFormat())
    {
        MOS_RESOURCE* osResource = nullptr;
        uint32_t      offset = 0;

        DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatusMask, osResource, offset));

        // Check HuC_STATUS bit15, HW continue if bit15 > 0, otherwise send COND BB END cmd.
        uint32_t compareOperation = mhw::mi::xe_lpm_plus_base_next::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADGREATERTHANIDD;
        DECODE_CHK_STATUS(m_hwInterface->SendCondBbEndCmd(
                osResource, offset, 0, false, false, compareOperation, cmdBuffer));
    }

    DECODE_CHK_STATUS(m_miItf->AddWatchdogTimerStartCmd(cmdBuffer));
    DECODE_CHK_STATUS(PackPictureLevelCmds(*cmdBuffer));

    HalOcaInterfaceNext::DumpCodechalParam(*cmdBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, m_hevcPipeline->GetCodechalOcaDumper(), CODECHAL_HEVC);
    HalOcaInterfaceNext::On1stLevelBBEnd(*cmdBuffer, *m_osInterface);

    DECODE_CHK_STATUS(m_allocator->SyncOnResource(&m_hevcBasicFeature->m_resDataBuffer, false));

    DECODE_CHK_STATUS(Mos_Solo_PostProcessDecode(m_osInterface, &m_hevcBasicFeature->m_destSurface));

#if USE_CODECHAL_DEBUG_TOOL
    DECODE_CHK_STATUS(DumpSecondaryCommandBuffer(*cmdBuffer));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeBackEndPktXe_Lpm_Plus_Base::VdMemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer)
{
    auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
    par       = {};
    par.memoryImplicitFlush = true;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeBackEndPktXe_Lpm_Plus_Base::VdPipelineFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par                  = m_vdencItf->GETPAR_VD_PIPELINE_FLUSH();
    par                        = {};
    par.waitDoneHEVC           = 1;
    par.flushHEVC              = 1;
    par.waitDoneVDCmdMsgParser = 1;
    m_vdencItf->ADDCMD_VD_PIPELINE_FLUSH(&cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeBackEndPktXe_Lpm_Plus_Base::VdScalabPipeUnLock(MOS_COMMAND_BUFFER &cmdBuffer)
{
    auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
    par.scalableModePipeUnlock = true;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeBackEndPktXe_Lpm_Plus_Base::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(StartStatusReport(statusReportMfx, &cmdBuffer));

    DECODE_CHK_STATUS(m_picturePkt->Execute(cmdBuffer));

    // add tile coding command
    DECODE_CHK_STATUS(m_tilePkt->Execute(cmdBuffer, m_hevcPipeline->GetCurrentPipe()));

    DECODE_CHK_STATUS(VdMemoryFlush(cmdBuffer));
    DECODE_CHK_STATUS(VdScalabPipeUnLock(cmdBuffer));
    DECODE_CHK_STATUS(ReadVdboxId(cmdBuffer));
    DECODE_CHK_STATUS(VdPipelineFlush(cmdBuffer));
    DECODE_CHK_STATUS(MiFlush(cmdBuffer));

    auto scalability = m_hevcPipeline->GetMediaScalability();
    DECODE_CHK_STATUS(scalability->SyncPipe(syncOnePipeWaitOthers, 0, &cmdBuffer));

    if (m_hevcPipeline->IsFirstPipe())
    {
        DECODE_CHK_STATUS(EndStatusReport(statusReportMfx, &cmdBuffer));
        DECODE_CHK_STATUS(UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
    }
    else
    {
        DECODE_CHK_STATUS(NullHW::StopPredicateNext(m_osInterface, m_miItf, &cmdBuffer));
    }
    DECODE_CHK_STATUS(MiFlush(cmdBuffer));

    DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    return MOS_STATUS_SUCCESS;
}

}

