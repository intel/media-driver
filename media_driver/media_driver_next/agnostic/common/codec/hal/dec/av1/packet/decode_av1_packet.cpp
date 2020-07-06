/*
* Copyright (c) 2019-2020, Intel Corporation
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
#include "codechal_utilities.h"
#include "decode_av1_packet.h"
#include "decode_status_report_defs.h"
#include "decode_predication_packet.h"
#include "mhw_vdbox_vdenc_g12_X.h"
#include "decode_marker_packet.h"

namespace decode {

MOS_STATUS Av1DecodePkt::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_miInterface);
    DECODE_CHK_NULL(m_statusReport);
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_av1Pipeline);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_vdencInterface);

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
    return MOS_STATUS_SUCCESS;
}

void Av1DecodePkt::SetPerfTag(CODECHAL_MODE mode, uint16_t picCodingType)
{
    DECODE_FUNC_CALL();

    uint16_t perfTag = ((mode << 4) & 0xF0) | (picCodingType & 0xF);
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

    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl = false;
    forceWakeupParams.bMFXPowerWellControlMask = true;
    forceWakeupParams.bHEVCPowerWellControl = true;
    forceWakeupParams.bHEVCPowerWellControlMask = true;

    DECODE_CHK_STATUS(m_miInterface->AddMiForceWakeupCmd(&cmdBuffer, &forceWakeupParams));

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
        if (m_mmcState && m_mmcState->IsMmcEnabled())
        {
            DECODE_CHK_STATUS(m_mmcState->SendPrologCmd(&cmdBuffer, false));
        }
#endif

    MHW_GENERIC_PROLOG_PARAMS  genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface = m_osInterface;
    genericPrologParams.pvMiInterface = m_miInterface;
    genericPrologParams.bMmcEnabled = false;

    DECODE_CHK_STATUS(Mhw_SendGenericPrologCmd(&cmdBuffer, &genericPrologParams));

    subPacket = m_av1Pipeline->GetSubPacket(DecodePacketId(m_av1Pipeline, predicationSubPacketId));
    DecodePredicationPkt *predicationPacket = dynamic_cast<DecodePredicationPkt*>(subPacket);
    DECODE_CHK_NULL(predicationPacket);
    DECODE_CHK_STATUS(predicationPacket->Execute(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::VdPipelineFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS_G12 vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams, sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bWaitDoneAV1 = 1;
    vdpipeFlushParams.Flags.bFlushAV1 = 1;
    vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    DECODE_CHK_STATUS(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, (MHW_VDBOX_VD_PIPE_FLUSH_PARAMS*)&vdpipeFlushParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::MiFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    DECODE_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

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

    commandBufferSize = m_pictureStatesSize +
                        m_tileStatesSize * (m_av1BasicFeature->m_tileCoding.m_numTiles + 1);

    return (commandBufferSize + COMMAND_BUFFER_RESERVED_SPACE);
}

uint32_t Av1DecodePkt::CalculatePatchListSize()
{
    if (!m_osInterface->bUsesPatchList)
    {
        return 0;
    }

    uint32_t requestedPatchListSize = 0;

    requestedPatchListSize = m_picturePatchListSize +
                                (m_tilePatchListSize * (m_av1BasicFeature->m_tileCoding.m_numTiles + 1));

    return requestedPatchListSize;
}

MOS_STATUS Av1DecodePkt::ReadAvpStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    DECODE_CHK_NULL(statusReport);

    return eStatus;
}

MOS_STATUS Av1DecodePkt::StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{

    MediaPacket::StartStatusReport(srType, cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodePkt::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{

    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(cmdBuffer);

    DECODE_CHK_STATUS(ReadAvpStatus( m_statusReport, *cmdBuffer));

    DECODE_CHK_STATUS(MediaPacket::EndStatusReport(srType, cmdBuffer));

    return MOS_STATUS_SUCCESS;

}

#if USE_CODECHAL_DEBUG_TOOL

MOS_STATUS Av1DecodePkt::DumpResources(
    DecodeStatusMfx *       decodeStatusMfx,
    DecodeStatusReportData *statusReportData)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(decodeStatusMfx);
    DECODE_CHK_NULL(statusReportData);

    return MOS_STATUS_SUCCESS;
}

#endif

}
