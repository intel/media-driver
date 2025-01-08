/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     decode_hevc_packet.cpp
//! \brief    Defines the interface for hevc decode packet
//!
#include "decode_hevc_packet.h"
#include "decode_status_report_defs.h"
#include "decode_predication_packet.h"
#include "decode_marker_packet.h"
#if (_DEBUG || _RELEASE_INTERNAL)
#include "decode_nullhw_proxy_test_packet.h"
#endif
namespace decode {

MOS_STATUS HevcDecodePkt::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_miItf);
    DECODE_CHK_NULL(m_statusReport);
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hevcPipeline);
    DECODE_CHK_NULL(m_osInterface);

    DECODE_CHK_STATUS(CmdPacket::Init());

    m_hevcBasicFeature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_hevcBasicFeature);

    m_allocator = m_hevcPipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface->bNullHwIsEnabled)
    {
        auto nullhwProxy = DecodeNullHWProxyTestPkt::Instance();
        DECODE_CHK_NULL(nullhwProxy);
        nullhwProxy->Init(m_hevcPipeline, m_osInterface);
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePkt::Destroy()
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface->bNullHwIsEnabled)
    {
        auto nullhwProxy = DecodeNullHWProxyTestPkt::Instance();
        DECODE_CHK_NULL(nullhwProxy);
        DECODE_CHK_STATUS(nullhwProxy->Destory());
    }
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePkt::Prepare()
{
    DECODE_FUNC_CALL();

    m_phase = static_cast<DecodePhase *>(m_hevcPipeline->GetComponentState());
    DECODE_CHK_NULL(m_phase);

    DECODE_CHK_NULL(m_hevcBasicFeature);
    DECODE_CHK_NULL(m_hevcBasicFeature->m_hevcPicParams);
    m_hevcPicParams = m_hevcBasicFeature->m_hevcPicParams;

    return MOS_STATUS_SUCCESS;
}

void HevcDecodePkt::SetPerfTag(CODECHAL_MODE mode, uint16_t picCodingType)
{
    DECODE_FUNC_CALL();

    uint16_t perfTag = ((mode << 4) & 0xF0) | (picCodingType & 0xF);
    m_osInterface->pfnIncPerfFrameID(m_osInterface);
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag);
    m_osInterface->pfnResetPerfBufferID(m_osInterface);
}

MOS_STATUS HevcDecodePkt::AddForceWakeup(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_miItf->GETPAR_MI_FORCE_WAKEUP();
    par                                         = {};
    par.bMFXPowerWellControl                    = false;
    par.bMFXPowerWellControlMask                = true;
    par.bHEVCPowerWellControl                   = true;
    par.bHEVCPowerWellControlMask               = true;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FORCE_WAKEUP(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePkt::SendPrologWithFrameTracking(MOS_COMMAND_BUFFER& cmdBuffer, bool frameTrackingRequested)
{
    DecodeSubPacket* subPacket = m_hevcPipeline->GetSubPacket(DecodePacketId(m_hevcPipeline, markerSubPacketId));
    DecodeMarkerPkt *makerPacket = dynamic_cast<DecodeMarkerPkt*>(subPacket);
    DECODE_CHK_NULL(makerPacket);
    DECODE_CHK_STATUS(makerPacket->Execute(cmdBuffer));

#ifdef _MMC_SUPPORTED
    DecodeMemComp *mmcState = m_hevcPipeline->GetMmcState();
    bool isMmcEnabled = (mmcState != nullptr && mmcState->IsMmcEnabled());
    if (isMmcEnabled)
    {
        DECODE_CHK_STATUS(mmcState->SendPrologCmd(&cmdBuffer, false));
    }
#endif

    MHW_GENERIC_PROLOG_PARAMS  genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface = m_osInterface;
    genericPrologParams.pvMiInterface = nullptr;
#ifdef _MMC_SUPPORTED
    genericPrologParams.bMmcEnabled = isMmcEnabled;
#endif

    DECODE_CHK_STATUS(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

    subPacket = m_hevcPipeline->GetSubPacket(DecodePacketId(m_hevcPipeline, predicationSubPacketId));
    DecodePredicationPkt *predicationPacket = dynamic_cast<DecodePredicationPkt*>(subPacket);
    DECODE_CHK_NULL(predicationPacket);
    DECODE_CHK_STATUS(predicationPacket->Execute(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePkt::MiFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_miItf->GETPAR_MI_FLUSH_DW();
    par       = {};
    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(mfxStatus);
    DECODE_CHK_NULL(statusReport);

    DecodeStatusMfx *       decodeStatusMfx  = (DecodeStatusMfx *)mfxStatus;
    DecodeStatusReportData *statusReportData = (DecodeStatusReportData *)statusReport;

    std::shared_ptr<mhw::vdbox::hcp::Itf> hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
    if (hcpItf != nullptr)
    {
        if ((decodeStatusMfx->m_mmioErrorStatusReg & hcpItf->GetHcpCabacErrorFlagsMask()) != 0)
        {
            statusReportData->codecStatus = CODECHAL_STATUS_ERROR;
            statusReportData->numMbsAffected = (decodeStatusMfx->m_mmioMBCountReg & 0xFFFC0000) >> 18;
        }

        statusReportData->frameCrc = decodeStatusMfx->m_mmioFrameCrcReg;
    }

    DECODE_VERBOSEMESSAGE("Index = %d", statusReportData->currDecodedPic.FrameIdx);
    DECODE_VERBOSEMESSAGE("FrameCrc = 0x%x", statusReportData->frameCrc);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePkt::ReadVdboxId(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_phase);
    DECODE_CHK_NULL(m_statusReport);

    uint8_t curPipe = m_phase->GetPipe();
    DECODE_CHK_COND(curPipe >= csInstanceIdMax, "Invalid pipe index.");
    uint32_t csEngineIdOffsetIdx = decode::DecodeStatusReportType::CsEngineIdOffset_0 + curPipe;

    auto &par = m_miItf->GETPAR_MI_STORE_REGISTER_MEM();
    par       = {};

    auto mmioRegistersHcp = m_hwInterface->GetHcpInterfaceNext()->GetMmioRegisters(MHW_VDBOX_NODE_1);

    MOS_RESOURCE* osResource = nullptr;
    uint32_t      offset     = 0;
    DECODE_CHK_STATUS(m_statusReport->GetAddress(csEngineIdOffsetIdx, osResource, offset));
    par.presStoreBuffer      = osResource;
    par.dwOffset             = offset;
    par.dwRegister           = mmioRegistersHcp->csEngineIdOffset;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePkt::ReadHcpStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(statusReport);

    MOS_STATUS    eStatus    = MOS_STATUS_SUCCESS;
    MOS_RESOURCE *osResource = nullptr;
    uint32_t      offset     = 0;

    auto &par = m_miItf->GETPAR_MI_STORE_REGISTER_MEM();

    par = {};
 
    auto mmioRegistersHcp = m_hwInterface->GetHcpInterfaceNext()->GetMmioRegisters(MHW_VDBOX_NODE_1);

    DECODE_CHK_STATUS(statusReport->GetAddress(decode::DecodeStatusReportType::DecErrorStatusOffset, osResource, offset));
    par.presStoreBuffer    = osResource;
    par.dwOffset           = offset;
    par.dwRegister         = mmioRegistersHcp->hcpCabacStatusRegOffset;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));

    DECODE_CHK_STATUS(statusReport->GetAddress(decode::DecodeStatusReportType::DecFrameCrcOffset, osResource, offset));
    par.presStoreBuffer    = osResource;
    par.dwOffset           = offset;
    par.dwRegister         = mmioRegistersHcp->hcpFrameCrcRegOffset;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));

    DECODE_CHK_STATUS(statusReport->GetAddress(decode::DecodeStatusReportType::DecMBCountOffset, osResource, offset));
    par.presStoreBuffer    = osResource;
    par.dwOffset           = offset;
    par.dwRegister         = mmioRegistersHcp->hcpDecStatusRegOffset;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));

    return eStatus;
}

MOS_STATUS HevcDecodePkt::StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_NULL(m_hevcPipeline);
    DECODE_CHK_NULL(m_osInterface);

    SetPerfTag(CODECHAL_DECODE_MODE_HEVCVLD, m_hevcBasicFeature->m_pictureCodingType);

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectStartCmd((void *)m_hevcPipeline, m_osInterface, m_miItf, cmdBuffer));

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface->bNullHwIsEnabled)
    {
        DecodeNullHWProxyTestPkt *nullhwProxy = DecodeNullHWProxyTestPkt::Instance();
        DECODE_CHK_NULL(nullhwProxy);
        nullhwProxy->AddNullHwProxyCmd(m_hevcPipeline, m_osInterface, cmdBuffer);
    }
#endif

    DECODE_CHK_STATUS(MediaPacket::StartStatusReportNext(srType, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePkt::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_STATUS(ReadHcpStatus(m_statusReport, *cmdBuffer));
    DECODE_CHK_STATUS(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectEndCmd((void*)m_hevcPipeline, m_osInterface, m_miItf, cmdBuffer));

    // Add Mi flush here to ensure end status tag flushed to memory earlier than completed count
    DECODE_CHK_STATUS(MiFlush(*cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePkt::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    // Send MI_FLUSH command
    auto &par      = m_miItf->GETPAR_MI_FLUSH_DW();
    par            = {};
    auto *skuTable = m_hevcPipeline->GetSkuTable();
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
    {
        // Add PPC fulsh
        par.bEnablePPCFlush = true;
    }
    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS HevcDecodePkt::DumpSecondaryCommandBuffer(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_ASSERT(m_phase != nullptr);
    CodechalDebugInterface *debugInterface = m_hevcPipeline->GetDebugInterface();
    DECODE_CHK_NULL(debugInterface);
    std::string cmdName = "DEC_secondary_" + std::to_string(m_phase->GetCmdBufIndex());
    DECODE_CHK_STATUS(debugInterface->DumpCmdBuffer(
        &cmdBuffer, CODECHAL_NUM_MEDIA_STATES, cmdName.c_str()));

    return MOS_STATUS_SUCCESS;
}
#endif

}
