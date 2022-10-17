/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     decode_avc_xe_m_base_packet.cpp
//! \brief    Defines the interface for avc decode packet.
//!

#include "decode_avc_xe_m_base_packet.h"
#include "decode_utils.h"
#include "decode_marker_packet_g12.h"
#include "decode_status_report_defs.h"
#include "decode_predication_packet_g12.h"
#include "mhw_vdbox_vdenc_g12_X.h"
#include "codechal_debug.h"

namespace decode {

MOS_STATUS AvcDecodePktXe_M_Base::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_miInterface);
    DECODE_CHK_NULL(m_statusReport);
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_avcPipeline);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_vdencInterface);

    DECODE_CHK_STATUS(CmdPacket::Init());

    m_avcBasicFeature = dynamic_cast<AvcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_avcBasicFeature);

    m_allocator = m_avcPipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(m_statusReport->RegistObserver(this));

    DecodeSubPacket* subPacket = m_avcPipeline->GetSubPacket(DecodePacketId(m_avcPipeline, avcPictureSubPacketId));

    m_picturePkt = dynamic_cast<AvcDecodePicPktXe_M_Base *>(subPacket);
    DECODE_CHK_NULL(m_picturePkt);
    DECODE_CHK_STATUS(m_picturePkt->CalculateCommandSize(m_pictureStatesSize, m_picturePatchListSize));

    subPacket = m_avcPipeline->GetSubPacket(DecodePacketId(m_avcPipeline, avcSliceSubPacketId));
    m_slicePkt = dynamic_cast<AvcDecodeSlcPktXe_M_Base*>(subPacket);
    DECODE_CHK_NULL(m_slicePkt);
    DECODE_CHK_STATUS(m_slicePkt->CalculateCommandSize(m_sliceStatesSize, m_slicePatchListSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePktXe_M_Base::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_avcBasicFeature->m_avcPicParams);
    m_avcPicParams = m_avcBasicFeature->m_avcPicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePktXe_M_Base::Destroy()
{
    m_statusReport->UnregistObserver(this);
    return MOS_STATUS_SUCCESS;
}

void AvcDecodePktXe_M_Base::SetPerfTag(CODECHAL_MODE mode, uint16_t picCodingType)
{
    DECODE_FUNC_CALL();
    uint16_t perfTag = ((mode << 4) & 0xF0) | (picCodingType & 0xF);

    m_osInterface->pfnIncPerfFrameID(m_osInterface);
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag);
    m_osInterface->pfnResetPerfBufferID(m_osInterface);
}

bool AvcDecodePktXe_M_Base::IsPrologRequired()
{
    return true;
}

MOS_STATUS AvcDecodePktXe_M_Base::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
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

MOS_STATUS AvcDecodePktXe_M_Base::SendPrologWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested)
{
    DecodeSubPacket* subPacket = m_avcPipeline->GetSubPacket(DecodePacketId(m_avcPipeline, markerSubPacketId));
    DecodeMarkerPktG12 *makerPacket = dynamic_cast<DecodeMarkerPktG12*>(subPacket);
    DECODE_CHK_NULL(makerPacket);
    DECODE_CHK_STATUS(makerPacket->Execute(cmdBuffer));

#ifdef _MMC_SUPPORTED
    m_mmcState = m_avcPipeline->GetMmcState();
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

    subPacket = m_avcPipeline->GetSubPacket(DecodePacketId(m_avcPipeline, predicationSubPacketId));
    DecodePredicationPktG12 *predicationPacket = dynamic_cast<DecodePredicationPktG12*>(subPacket);
    DECODE_CHK_NULL(predicationPacket);
    DECODE_CHK_STATUS(predicationPacket->Execute(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePktXe_M_Base::MiFlush(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    DECODE_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePktXe_M_Base::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(mfxStatus);
    DECODE_CHK_NULL(statusReport);
    DECODE_CHK_NULL(m_avcBasicFeature);

    DecodeStatusMfx *decodeStatusMfx  = (DecodeStatusMfx *)mfxStatus;
    DecodeStatusReportData *statusReportData = (DecodeStatusReportData *)statusReport;
    MhwVdboxMfxInterface* mfxInterface = m_hwInterface->GetMfxInterface();

    if (mfxInterface)
    {
        if ((decodeStatusMfx->m_mmioErrorStatusReg & mfxInterface->GetMfxErrorFlagsMask()) != 0)
        {
            statusReportData->codecStatus = CODECHAL_STATUS_ERROR;
            statusReportData->numMbsAffected = decodeStatusMfx->m_mmioMBCountReg & 0xFFFF;
        }
        statusReportData->frameCrc = decodeStatusMfx->m_mmioFrameCrcReg;
    }

    DECODE_VERBOSEMESSAGE("Current Frame Index = %d", statusReportData->currDecodedPic.FrameIdx);
    DECODE_VERBOSEMESSAGE("FrameCrc = 0x%x", statusReportData->frameCrc);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePktXe_M_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    commandBufferSize = CalculateCommandBufferSize();
    requestedPatchListSize = CalculatePatchListSize();

    return MOS_STATUS_SUCCESS;
}

uint32_t AvcDecodePktXe_M_Base::CalculateCommandBufferSize()
{
    uint32_t commandBufferSize = 0;

    // plus 1 is for phantom slice command
    commandBufferSize = m_pictureStatesSize + m_sliceStatesSize * (m_avcBasicFeature->m_numSlices + 1);

    return (commandBufferSize + COMMAND_BUFFER_RESERVED_SPACE);
}

uint32_t AvcDecodePktXe_M_Base::CalculatePatchListSize()
{
    if (!m_osInterface->bUsesPatchList)
    {
        return 0;
    }

    uint32_t requestedPatchListSize = 0;
    requestedPatchListSize = m_picturePatchListSize + m_slicePatchListSize * (m_avcBasicFeature->m_numSlices + 1);

    return requestedPatchListSize;
}

MOS_STATUS AvcDecodePktXe_M_Base::ReadMfxStatus(MediaStatusReport *statusReport, MOS_COMMAND_BUFFER &cmdBuffer)
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

    DECODE_CHK_STATUS(statusReport->GetAddress(decode::DecodeStatusReportType::DecFrameCrcOffset, osResource, offset));
    params.presStoreBuffer = osResource;
    params.dwOffset = offset;
    params.dwRegister = mmioRegistersMfx->mfxFrameCrcRegOffset;

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

    return eStatus;
}

MOS_STATUS AvcDecodePktXe_M_Base::StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(MediaPacket::StartStatusReport(srType, cmdBuffer));

    SetPerfTag(CODECHAL_DECODE_MODE_AVCVLD, m_avcBasicFeature->m_pictureCodingType);

    MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectStartCmd(
        (void*)m_avcPipeline, m_osInterface, m_miInterface, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePktXe_M_Base::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_STATUS(ReadMfxStatus(m_statusReport, *cmdBuffer));
    DECODE_CHK_STATUS(MediaPacket::EndStatusReport(srType, cmdBuffer));

    MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
    DECODE_CHK_NULL(perfProfiler);
    DECODE_CHK_STATUS(perfProfiler->AddPerfCollectEndCmd(
        (void*)m_avcPipeline, m_osInterface, m_miInterface, cmdBuffer));

    // Add Mi flush here to ensure end status tag flushed to memory earlier than completed count
    DECODE_CHK_STATUS(MiFlush(*cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDecodePktXe_M_Base::SetCencBatchBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(cmdBuffer);

    auto cencBuf = m_avcBasicFeature->m_cencBuf;
    DECODE_CHK_NULL(cencBuf);
    MHW_BATCH_BUFFER batchBuffer;
    MOS_ZeroMemory(&batchBuffer, sizeof(MHW_BATCH_BUFFER));
    MOS_RESOURCE *resHeap    = nullptr;
    DECODE_CHK_NULL(resHeap  = cencBuf->secondLvlBbBlock->GetResource());
    batchBuffer.OsResource   = *resHeap;
    batchBuffer.dwOffset     = cencBuf->secondLvlBbBlock->GetOffset();
    batchBuffer.iSize        = cencBuf->secondLvlBbBlock->GetSize();
    batchBuffer.bSecondLevel = true;
#if (_DEBUG || _RELEASE_INTERNAL)
    batchBuffer.iLastCurrent = batchBuffer.iSize;
#endif  // (_DEBUG || _RELEASE_INTERNAL)

    DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferStartCmd(
        cmdBuffer,
        &batchBuffer));

#if USE_CODECHAL_DEBUG_TOOL
    CODECHAL_DEBUG_TOOL(
        CodechalDebugInterface *debugInterface = m_avcPipeline->GetDebugInterface();
        DECODE_CHK_STATUS(debugInterface->Dump2ndLvlBatch(
            &batchBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_2ndLvlBatch"));)
#endif
    // Update GlobalCmdBufId
    MHW_MI_STORE_DATA_PARAMS miStoreDataParams;
    MOS_ZeroMemory(&miStoreDataParams, sizeof(miStoreDataParams));
    miStoreDataParams.pOsResource = cencBuf->resTracker;
    miStoreDataParams.dwValue     = cencBuf->trackerId;
    DECODE_NORMALMESSAGE("dwCmdBufId = %d", miStoreDataParams.dwValue);
    DECODE_CHK_STATUS(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &miStoreDataParams));

    return MOS_STATUS_SUCCESS;
}

}
