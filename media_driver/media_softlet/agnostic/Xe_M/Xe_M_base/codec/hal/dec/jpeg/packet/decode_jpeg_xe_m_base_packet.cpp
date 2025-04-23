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
//! \file     decode_jpeg_xe_m_base_packet.cpp
//! \brief    Defines the interface for jpeg decode packet.
//!

#include "decode_jpeg_xe_m_base_packet.h"
#include "decode_utils.h"
#include "decode_marker_packet_g12.h"
#include "decode_status_report_defs.h"
#include "decode_predication_packet_g12.h"
#include "codechal_debug.h"

namespace decode {

MOS_STATUS JpegDecodePktXe_M_Base::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_miInterface);
    DECODE_CHK_NULL(m_statusReport);
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_jpegPipeline);
    DECODE_CHK_NULL(m_osInterface);

    DECODE_CHK_STATUS(CmdPacket::Init());

    m_jpegBasicFeature = dynamic_cast<JpegBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_jpegBasicFeature);

    m_allocator = m_jpegPipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(m_statusReport->RegistObserver(this));

    DecodeSubPacket* subPacket = m_jpegPipeline->GetSubPacket(DecodePacketId(m_jpegPipeline, jpegPictureSubPacketId));
    m_picturePkt = dynamic_cast<JpegDecodePicPktXe_M_Base*>(subPacket);
    DECODE_CHK_NULL(m_picturePkt);
    DECODE_CHK_STATUS(m_picturePkt->CalculateCommandSize(m_pictureStatesSize, m_picturePatchListSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePktXe_M_Base::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_jpegBasicFeature->m_jpegPicParams);
    m_jpegPicParams = m_jpegBasicFeature->m_jpegPicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePktXe_M_Base::Destroy()
{
    m_statusReport->UnregistObserver(this);

    return MOS_STATUS_SUCCESS;
}

void JpegDecodePktXe_M_Base::SetPerfTag(CODECHAL_MODE mode, uint16_t picCodingType)
{
    DECODE_FUNC_CALL();

    uint16_t perfTag = ((mode << 4) & 0xF0) | (picCodingType & 0xF);
    m_osInterface->pfnIncPerfFrameID(m_osInterface);
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag);
    m_osInterface->pfnResetPerfBufferID(m_osInterface);
}

bool JpegDecodePktXe_M_Base::IsPrologRequired()
{
    return true;
}

MOS_STATUS JpegDecodePktXe_M_Base::AddForceWakeup(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl = true;
    forceWakeupParams.bMFXPowerWellControlMask = true;
    forceWakeupParams.bHEVCPowerWellControl = false;
    forceWakeupParams.bHEVCPowerWellControlMask = true;

    DECODE_CHK_STATUS(m_miInterface->AddMiForceWakeupCmd(&cmdBuffer, &forceWakeupParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePktXe_M_Base::SendPrologWithFrameTracking(MOS_COMMAND_BUFFER& cmdBuffer, bool frameTrackingRequested)
{
    DECODE_FUNC_CALL();

    DecodeSubPacket* subPacket = m_jpegPipeline->GetSubPacket(DecodePacketId(m_jpegPipeline, markerSubPacketId));
    DecodeMarkerPktG12 *makerPacket = dynamic_cast<DecodeMarkerPktG12*>(subPacket);
    DECODE_CHK_NULL(makerPacket);
    DECODE_CHK_STATUS(makerPacket->Execute(cmdBuffer));

#ifdef _MMC_SUPPORTED
    m_mmcState = m_jpegPipeline->GetMmcState();
    bool isMmcEnabled = (m_mmcState != nullptr && m_mmcState->IsMmcEnabled());
    if (isMmcEnabled)
    {
        DECODE_CHK_STATUS(m_mmcState->SendPrologCmd(&cmdBuffer, false));
    }
#endif

    MHW_GENERIC_PROLOG_PARAMS  genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface = m_osInterface;
    genericPrologParams.pvMiInterface = m_miInterface;

#ifdef _MMC_SUPPORTED
    genericPrologParams.bMmcEnabled = isMmcEnabled;
#endif

    DECODE_CHK_STATUS(Mhw_SendGenericPrologCmd(&cmdBuffer, &genericPrologParams));

    subPacket = m_jpegPipeline->GetSubPacket(DecodePacketId(m_jpegPipeline, predicationSubPacketId));
    DecodePredicationPktG12 *predicationPacket = dynamic_cast<DecodePredicationPktG12*>(subPacket);
    DECODE_CHK_NULL(predicationPacket);
    DECODE_CHK_STATUS(predicationPacket->Execute(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePktXe_M_Base::MiFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    DECODE_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePktXe_M_Base::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    DECODE_FUNC_CALL();

    DecodeStatusMfx *decodeStatusMfx  = (DecodeStatusMfx *)mfxStatus;
    DecodeStatusReportData *statusReportData = (DecodeStatusReportData *)statusReport;
    MhwVdboxMfxInterface* mfxInterface = m_hwInterface->GetMfxInterface();

    if (mfxInterface && decodeStatusMfx && statusReportData)
    {
        if ((decodeStatusMfx->m_mmioErrorStatusReg & mfxInterface->GetMfxErrorFlagsMask()) != 0)
        {
            statusReportData->codecStatus = CODECHAL_STATUS_ERROR;
            statusReportData->numMbsAffected = decodeStatusMfx->m_mmioMBCountReg & 0xFFFF;
        }
        DECODE_VERBOSEMESSAGE("Current Frame Index = %d", statusReportData->currDecodedPic.FrameIdx);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePktXe_M_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    commandBufferSize = CalculateCommandBufferSize();
    requestedPatchListSize = CalculatePatchListSize();

    return MOS_STATUS_SUCCESS;
}

uint32_t JpegDecodePktXe_M_Base::CalculateCommandBufferSize()
{
    // slice/macroblock level commands are put into 2nd level BB.
    return (m_pictureStatesSize + COMMAND_BUFFER_RESERVED_SPACE);
}

uint32_t JpegDecodePktXe_M_Base::CalculatePatchListSize()
{
    if (!m_osInterface->bUsesPatchList)
    {
        return 0;
    }

    return m_picturePatchListSize;
}

MOS_STATUS JpegDecodePktXe_M_Base::ReadMfxStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(statusReport);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_RESOURCE* osResource = nullptr;
    uint32_t     offset = 0;

    MHW_MI_STORE_REGISTER_MEM_PARAMS params;
    MOS_ZeroMemory(&params, sizeof(MHW_MI_STORE_REGISTER_MEM_PARAMS));

    auto mmioRegistersMfx = m_hwInterface->SelectVdboxAndGetMmioRegister(MHW_VDBOX_NODE_1, &cmdBuffer);

    DECODE_CHK_STATUS(statusReport->GetAddress(decode::DecodeStatusReportType::DecErrorStatusOffset, osResource, offset));
    params.presStoreBuffer = osResource;
    params.dwOffset = offset;
    params.dwRegister = mmioRegistersMfx->mfxErrorFlagsRegOffset;

    DECODE_CHK_STATUS(m_miInterface->AddMiStoreRegisterMemCmd(
        &cmdBuffer,
        &params));

    DECODE_CHK_STATUS(statusReport->GetAddress(decode::DecodeStatusReportType::DecMBCountOffset, osResource, offset));
    params.presStoreBuffer = osResource;
    params.dwOffset = offset;
    params.dwRegister = mmioRegistersMfx->mfxMBCountRegOffset;

    DECODE_CHK_STATUS(m_miInterface->AddMiStoreRegisterMemCmd(
        &cmdBuffer,
        &params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePktXe_M_Base::StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(MediaPacket::StartStatusReport(srType, cmdBuffer));

    // no frame type for Jpeg decode, use I as default value here
    SetPerfTag(CODECHAL_DECODE_MODE_JPEG, I_TYPE);

    MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectStartCmd(
        (void*)m_jpegPipeline, m_osInterface, m_miInterface, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePktXe_M_Base::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_STATUS(ReadMfxStatus(m_statusReport, *cmdBuffer));
    DECODE_CHK_STATUS(MediaPacket::EndStatusReport(srType, cmdBuffer));

    MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectEndCmd(
        (void*)m_jpegPipeline, m_osInterface, m_miInterface, cmdBuffer));

    DECODE_CHK_STATUS(MiFlush(*cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

}