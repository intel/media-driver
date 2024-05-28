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
//! \file     decode_av1_packet.cpp
//! \brief    Defines the interface for av1 decode packet
//!
#include "decode_av1_packet.h"
#include "decode_status_report_defs.h"
#include "decode_predication_packet.h"
#include "decode_marker_packet.h"

namespace decode {

MOS_STATUS Av1DecodePkt::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_miItf);
    DECODE_CHK_NULL(m_statusReport);
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_av1Pipeline);
    DECODE_CHK_NULL(m_osInterface);

    DECODE_CHK_STATUS(CmdPacket::Init());

    m_av1BasicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_av1BasicFeature);

    m_allocator = m_av1Pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(m_statusReport->RegistObserver(this));

    DecodeSubPacket* subPacket = m_av1Pipeline->GetSubPacket(DecodePacketId(m_av1Pipeline, av1PictureSubPacketId));

    m_picturePkt = dynamic_cast<Av1DecodePicPkt*>(subPacket);
    DECODE_CHK_NULL(m_picturePkt);
    DECODE_CHK_STATUS(m_picturePkt->CalculateCommandSize(m_pictureStatesSize, m_picturePatchListSize));

    subPacket = m_av1Pipeline->GetSubPacket(DecodePacketId(m_av1Pipeline, av1TileSubPacketId));
    m_tilePkt = dynamic_cast<Av1DecodeTilePkt*>(subPacket);
    DECODE_CHK_NULL(m_tilePkt);
    DECODE_CHK_STATUS(m_tilePkt->CalculateCommandSize(m_tileStatesSize, m_tilePatchListSize));

    m_secondLevelBBArray = m_allocator->AllocateBatchBufferArray(
        m_pictureStatesSize, 1, CODEC_NUM_AV1_SECOND_BB, true, lockableVideoMem);
    DECODE_CHK_NULL(m_secondLevelBBArray);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_av1BasicFeature->m_av1PicParams);
    m_av1PicParams = m_av1BasicFeature->m_av1PicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::Destroy()
{
    m_statusReport->UnregistObserver(this);
    if (m_allocator != nullptr)
    {
        DECODE_CHK_STATUS(m_allocator->Destroy(m_secondLevelBBArray));
    }

    return MOS_STATUS_SUCCESS;
}

void Av1DecodePkt::SetPerfTag(CODECHAL_MODE mode, uint16_t picCodingType)
{
    DECODE_FUNC_CALL();

    uint16_t perfTag = ((mode << 4) & 0xF0) | (picCodingType & 0xF);
    m_osInterface->pfnIncPerfFrameID(m_osInterface);
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag);
    m_osInterface->pfnResetPerfBufferID(m_osInterface);
}

bool Av1DecodePkt::IsPrologRequired()
{
    return true; // if ScalableMode, should set to false.
}

MOS_STATUS Av1DecodePkt::AddForceWakeup(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_miItf->GETPAR_MI_FORCE_WAKEUP();
    MOS_ZeroMemory(&par, sizeof(mhw::mi::MI_FORCE_WAKEUP_PAR));
    par.bMFXPowerWellControl                    = false;
    par.bMFXPowerWellControlMask                = true;
    par.bHEVCPowerWellControl                   = true;
    par.bHEVCPowerWellControlMask               = true;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FORCE_WAKEUP(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::SendPrologWithFrameTracking(MOS_COMMAND_BUFFER& cmdBuffer, bool frameTrackingRequested)
{
    DecodeSubPacket* subPacket = m_av1Pipeline->GetSubPacket(DecodePacketId(m_av1Pipeline, markerSubPacketId));
    DecodeMarkerPkt *makerPacket = dynamic_cast<DecodeMarkerPkt*>(subPacket);
    DECODE_CHK_NULL(makerPacket);
    DECODE_CHK_STATUS(makerPacket->Execute(cmdBuffer));

#ifdef _MMC_SUPPORTED
    m_mmcState = m_av1Pipeline->GetMmcState();
    bool isMmcEnabled = (m_mmcState != nullptr && m_mmcState->IsMmcEnabled());
    if (isMmcEnabled)
    {
        DECODE_CHK_STATUS(m_mmcState->SendPrologCmd(&cmdBuffer, false));
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

    subPacket = m_av1Pipeline->GetSubPacket(DecodePacketId(m_av1Pipeline, predicationSubPacketId));
    DecodePredicationPkt *predicationPacket = dynamic_cast<DecodePredicationPkt*>(subPacket);
    DECODE_CHK_NULL(predicationPacket);
    DECODE_CHK_STATUS(predicationPacket->Execute(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::VdPipelineFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_vdencItf->GETPAR_VD_PIPELINE_FLUSH();
    par       = {};
    par.waitDoneAV1 = 1;
    par.flushAV1    = 1;
    par.waitDoneVDCmdMsgParser = 1;
    m_vdencItf->ADDCMD_VD_PIPELINE_FLUSH(&cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::MiFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_miItf->GETPAR_MI_FLUSH_DW();
    MOS_ZeroMemory(&par, sizeof(mhw::mi::MI_FLUSH_DW_PAR));
    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(mfxStatus);
    DECODE_CHK_NULL(statusReport);
    DECODE_CHK_NULL(m_av1BasicFeature);

    DecodeStatusMfx *       decodeStatusMfx  = (DecodeStatusMfx *)mfxStatus;
    DecodeStatusReportData *statusReportData = (DecodeStatusReportData *)statusReport;
    DECODE_VERBOSEMESSAGE("Current Frame Index = %d", statusReportData->currDecodedPic.FrameIdx);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    commandBufferSize = CalculateCommandBufferSize();
    requestedPatchListSize = CalculatePatchListSize();
    return MOS_STATUS_SUCCESS;
}

uint32_t Av1DecodePkt::CalculateCommandBufferSize()
{
    uint32_t commandBufferSize = 0;

    commandBufferSize = m_pictureStatesSize + m_tileStatesSize;

    return (commandBufferSize + COMMAND_BUFFER_RESERVED_SPACE);
}

uint32_t Av1DecodePkt::CalculatePatchListSize()
{
    if (!m_osInterface->bUsesPatchList)
    {
        return 0;
    }

    uint32_t requestedPatchListSize = 0;

    requestedPatchListSize = m_picturePatchListSize + m_tilePatchListSize;

    return requestedPatchListSize;
}

MOS_STATUS Av1DecodePkt::ReadAvpStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(statusReport);

    if (!m_osInterface->bSimIsActive)
    {
        MOS_RESOURCE* osResource = nullptr;
        uint32_t offset = 0;

        auto &par = m_miItf->GETPAR_MI_STORE_REGISTER_MEM();
        MOS_ZeroMemory(&par, sizeof(mhw::mi::MI_STORE_REGISTER_MEM_PAR));

        DECODE_CHK_NULL(m_hwInterface->GetAvpInterfaceNext());
        auto mmioRegistersAvp = m_hwInterface->GetAvpInterfaceNext()->GetMmioRegisters(MHW_VDBOX_NODE_1);

        DECODE_CHK_STATUS(statusReport->GetAddress(decode::DecodeStatusReportType::DecErrorStatusOffset, osResource, offset));
        par.presStoreBuffer = osResource;
        par.dwOffset        = offset;
        par.dwRegister      = mmioRegistersAvp->avpAv1DecErrorStatusAddrRegOffset;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(MediaPacket::StartStatusReportNext(srType, cmdBuffer));

    SetPerfTag(CODECHAL_DECODE_MODE_AV1VLD, m_av1BasicFeature->m_pictureCodingType);
    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectStartCmd(
        (void*)m_av1Pipeline, m_osInterface, m_miItf, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_STATUS(ReadAvpStatus( m_statusReport, *cmdBuffer));
    DECODE_CHK_STATUS(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectEndCmd(
        (void*)m_av1Pipeline, m_osInterface, m_miItf, cmdBuffer));

    // Add Mi flush here to ensure end status tag flushed to memory earlier than completed count
    DECODE_CHK_STATUS(MiFlush(*cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::InitPicLevelCmdBuffer(MHW_BATCH_BUFFER &batchBuffer, uint8_t *batchBufBase)
{
    DECODE_FUNC_CALL();

    auto &cmdBuffer = m_picCmdBuffer;
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    cmdBuffer.pCmdBase   = (uint32_t*)batchBufBase;
    cmdBuffer.pCmdPtr    = cmdBuffer.pCmdBase;
    cmdBuffer.iRemaining = batchBuffer.iSize;
    cmdBuffer.OsResource = batchBuffer.OsResource;

    return MOS_STATUS_SUCCESS;
}

}
