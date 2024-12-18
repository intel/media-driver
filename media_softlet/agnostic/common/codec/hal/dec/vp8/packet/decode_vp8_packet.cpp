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
//! \file     decode_vp8_packet.cpp
//! \brief    Defines the interface for vp8 decode packet
//!
#include "decode_vp8_packet.h"
#include "decode_status_report_defs.h"
#include "decode_predication_packet.h"
#include "decode_marker_packet.h"

namespace decode
{
Vp8DecodePkt::Vp8DecodePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
    : CmdPacket(task)
{
    if (pipeline != nullptr)
    {
        m_statusReport   = pipeline->GetStatusReportInstance();
        m_featureManager = pipeline->GetFeatureManager();
        m_vp8Pipeline    = dynamic_cast<Vp8Pipeline *>(pipeline);
    }
    if (hwInterface != nullptr)
    {
        m_hwInterface    = hwInterface;
        m_miItf          = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
        m_osInterface    = hwInterface->GetOsInterface();
    }
}

MOS_STATUS Vp8DecodePkt::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_statusReport);
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_vp8Pipeline);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_miItf);

    DECODE_CHK_STATUS(CmdPacket::Init());

    DecodeSubPacket *subPacket = m_vp8Pipeline->GetSubPacket(DecodePacketId(m_vp8Pipeline, vp8PictureSubPacketId));
    m_picturePkt               = dynamic_cast<Vp8DecodePicPkt *>(subPacket);
    DECODE_CHK_NULL(m_picturePkt);
    DECODE_CHK_STATUS(m_picturePkt->CalculateCommandSize(m_pictureStatesSize, m_picturePatchListSize));

    subPacket  = m_vp8Pipeline->GetSubPacket(DecodePacketId(m_vp8Pipeline, vp8SliceSubPacketId));
    m_slicePkt = dynamic_cast<Vp8DecodeSlcPkt *>(subPacket);
    DECODE_CHK_NULL(m_slicePkt);
    DECODE_CHK_STATUS(m_slicePkt->CalculateCommandSize(m_sliceStatesSize, m_slicePatchListSize));

    m_vp8BasicFeature = dynamic_cast<Vp8BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_vp8BasicFeature);

    m_allocator = m_vp8Pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(m_statusReport->RegistObserver(this));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8DecodePkt::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_vp8BasicFeature);
    DECODE_CHK_NULL(m_vp8BasicFeature->m_vp8PicParams);
    m_vp8PicParams = m_vp8BasicFeature->m_vp8PicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8DecodePkt::Destroy()
{
    m_statusReport->UnregistObserver(this);
    return MOS_STATUS_SUCCESS;
}

void Vp8DecodePkt::SetPerfTag(CODECHAL_MODE mode, uint16_t picCodingType)
{
    DECODE_FUNC_CALL();

    uint16_t perfTag = ((mode << 4) & 0xF0) | (picCodingType & 0xF);
    m_osInterface->pfnIncPerfFrameID(m_osInterface);
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag);
    m_osInterface->pfnResetPerfBufferID(m_osInterface);
}

bool Vp8DecodePkt::IsPrologRequired()
{
    return true;
}

MOS_STATUS Vp8DecodePkt::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_miItf->GETPAR_MI_FORCE_WAKEUP();
    MOS_ZeroMemory(&par, sizeof(par));
    par.bMFXPowerWellControl                    = true;
    par.bMFXPowerWellControlMask                = true;
    par.bHEVCPowerWellControl                   = false;
    par.bHEVCPowerWellControlMask               = true;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FORCE_WAKEUP(&cmdBuffer));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8DecodePkt::SendPrologWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested)
{
    DecodeSubPacket *subPacket   = m_vp8Pipeline->GetSubPacket(DecodePacketId(m_vp8Pipeline, markerSubPacketId));
    DecodeMarkerPkt *makerPacket = dynamic_cast<DecodeMarkerPkt *>(subPacket);
    DECODE_CHK_NULL(makerPacket);
    DECODE_CHK_STATUS(makerPacket->Execute(cmdBuffer));

#ifdef _MMC_SUPPORTED
    m_mmcState        = m_vp8Pipeline->GetMmcState();
    bool isMmcEnabled = (m_mmcState != nullptr && m_mmcState->IsMmcEnabled());
    if (isMmcEnabled)
    {
        DECODE_CHK_STATUS(m_mmcState->SendPrologCmd(&cmdBuffer, false));
    }
#endif

    MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface  = m_osInterface;
    genericPrologParams.pvMiInterface = nullptr;
#ifdef _MMC_SUPPORTED
    genericPrologParams.bMmcEnabled = isMmcEnabled;
#endif

    DECODE_CHK_STATUS(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

    subPacket                               = m_vp8Pipeline->GetSubPacket(DecodePacketId(m_vp8Pipeline, predicationSubPacketId));
    DecodePredicationPkt *predicationPacket = dynamic_cast<DecodePredicationPkt *>(subPacket);
    DECODE_CHK_NULL(predicationPacket);
    DECODE_CHK_STATUS(predicationPacket->Execute(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8DecodePkt::MiFlush(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &par = m_miItf->GETPAR_MI_FLUSH_DW();
    MOS_ZeroMemory(&par, sizeof(par));
    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8DecodePkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(mfxStatus);
    DECODE_CHK_NULL(statusReport);

    DecodeStatusMfx *       decodeStatusMfx  = (DecodeStatusMfx *)mfxStatus;
    DecodeStatusReportData *statusReportData = (DecodeStatusReportData *)statusReport;

    auto hcpInterface = m_hwInterface->GetHcpInterfaceNext();
    if (hcpInterface != nullptr)
    {
        if ((decodeStatusMfx->m_mmioErrorStatusReg & hcpInterface->GetHcpCabacErrorFlagsMask()) != 0)
        {
            statusReportData->codecStatus    = CODECHAL_STATUS_ERROR;
            statusReportData->numMbsAffected = (decodeStatusMfx->m_mmioMBCountReg & 0xFFFC0000) >> 18;
        }

        statusReportData->frameCrc = decodeStatusMfx->m_mmioFrameCrcReg;
    }

    DECODE_VERBOSEMESSAGE("Index = %d", statusReportData->currDecodedPic.FrameIdx);
    DECODE_VERBOSEMESSAGE("FrameCrc = 0x%x", statusReportData->frameCrc);

    return MOS_STATUS_SUCCESS;
}

uint32_t Vp8DecodePkt::CalculateCommandBufferSize()
{
    uint32_t commandBufferSize = 0;
    commandBufferSize          = m_pictureStatesSize + m_sliceStatesSize;

    return (commandBufferSize + COMMAND_BUFFER_RESERVED_SPACE);
}

uint32_t Vp8DecodePkt::CalculatePatchListSize()
{
    if (!m_osInterface->bUsesPatchList)
    {
        return 0;
    }

    uint32_t requestedPatchListSize = 0;
    requestedPatchListSize          = m_picturePatchListSize + m_slicePatchListSize;

    return requestedPatchListSize;
}

MOS_STATUS Vp8DecodePkt::CalculateCommandSize(uint32_t& commandBufferSize, uint32_t& requestedPatchListSize)
{
    commandBufferSize      = CalculateCommandBufferSize();
    requestedPatchListSize = CalculatePatchListSize();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8DecodePkt::ReadMfxStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(statusReport);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_RESOURCE* osResource = nullptr;
    uint32_t     offset = 0;

    auto &par = m_miItf->GETPAR_MI_STORE_REGISTER_MEM();
    MOS_ZeroMemory(&par, sizeof(par));

    DECODE_CHK_NULL(m_hwInterface->GetVdencInterfaceNext());
    auto mmioRegisters = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(MHW_VDBOX_NODE_1);

    DECODE_CHK_STATUS(statusReport->GetAddress(decode::DecodeStatusReportType::DecErrorStatusOffset, osResource, offset));

    par.presStoreBuffer = osResource;
    par.dwOffset        = offset;
    par.dwRegister      = mmioRegisters->mfxErrorFlagsRegOffset;
    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));


    DECODE_CHK_STATUS(statusReport->GetAddress(decode::DecodeStatusReportType::DecMBCountOffset, osResource, offset));

    par.presStoreBuffer = osResource;
    par.dwOffset        = offset;
    par.dwRegister      = mmioRegisters->mfxMBCountRegOffset;
    DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_STORE_REGISTER_MEM(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8DecodePkt::StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_STATUS(MediaPacket::StartStatusReportNext(srType, cmdBuffer));

    SetPerfTag(CODECHAL_DECODE_MODE_VP8VLD, m_vp8BasicFeature->m_pictureCodingType);
    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectStartCmd(
        (void *)m_vp8Pipeline, m_osInterface, m_miItf, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8DecodePkt::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_STATUS(ReadMfxStatus(m_statusReport, *cmdBuffer));
    DECODE_CHK_STATUS(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectEndCmd(
        (void *)m_vp8Pipeline, m_osInterface, m_miItf, cmdBuffer));

    // Add Mi flush here to ensure end status tag flushed to memory earlier than completed count
    DECODE_CHK_STATUS(MiFlush(*cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode