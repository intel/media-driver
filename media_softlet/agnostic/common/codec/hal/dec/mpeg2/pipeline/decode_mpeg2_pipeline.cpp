/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_mpeg2_pipeline.cpp
//! \brief    Defines the interface for mpeg2 decode pipeline
//!
#include "decode_mpeg2_pipeline.h"
#include "decode_utils.h"
#include "media_user_settings_mgr_g12.h"
#include "codechal_setting.h"
#include "decode_mpeg2_feature_manager.h"
#include "decode_huc_packet_creator_base.h" 

namespace decode{

Mpeg2Pipeline::Mpeg2Pipeline(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipeline(hwInterface, debugInterface)
{
    MOS_STATUS m_status = InitUserSetting(m_userSettingPtr);
}

MOS_STATUS Mpeg2Pipeline::Initialize(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));
    m_basicFeature = dynamic_cast<Mpeg2BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    // Create basic GPU context
    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
    m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);

    HucPacketCreatorBase *hucPktCreator = dynamic_cast<HucPacketCreatorBase *>(this);
    DECODE_CHK_NULL(hucPktCreator);
    m_mpeg2BsCopyPkt    = hucPktCreator->CreateHucCopyPkt(this, m_task, m_hwInterface);
    DECODE_CHK_NULL(m_mpeg2BsCopyPkt);
    MediaPacket *packet = dynamic_cast<MediaPacket *>(m_mpeg2BsCopyPkt);
    DECODE_CHK_NULL(packet);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, mpeg2BsCopyPktId), packet));
    DECODE_CHK_STATUS(packet->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::Prepare(void *params)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(params);
    DECODE_CHK_STATUS(DecodePipeline::Prepare(params));
    DECODE_CHK_STATUS(CopyBitstreamBuffer());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::CopyDummyBitstream()
{
    DECODE_FUNC_CALL();

    HucCopyPktItf::HucCopyParams copyParams = {};

    for (uint16_t slcIdx = 0; slcIdx < m_basicFeature->m_totalNumSlicesRecv; slcIdx++)
    {
        // Copy dummy slice to local buffer
        if (!m_basicFeature->m_copyDummySlicePresent && ((m_basicFeature->m_sliceRecord[slcIdx].prevSliceMbEnd !=
            m_basicFeature->m_sliceRecord[slcIdx].sliceStartMbOffset && !m_basicFeature->m_sliceRecord[slcIdx].skip) ||
            m_basicFeature->m_incompletePicture))
        {
            m_basicFeature->m_copyDummySlicePresent = true;
            copyParams.srcBuffer  = &(m_basicFeature->m_resMpeg2DummyBistream->OsResource);
            copyParams.srcOffset  = 0;
            copyParams.destBuffer = &(m_basicFeature->m_copiedDataBuf->OsResource);
            copyParams.destOffset = m_basicFeature->m_nextCopiedDataOffset;
            copyParams.copyLength = sizeof(m_basicFeature->Mpeg2DummyBsBuf);
            m_mpeg2BsCopyPkt->PushCopyParams(copyParams);

            m_basicFeature->m_dummySliceDataOffset = m_basicFeature->m_nextCopiedDataOffset;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::CopyBitstreamBuffer()
{
    DECODE_FUNC_CALL();

    HucCopyPktItf::HucCopyParams copyParams = {};

    if (m_basicFeature->m_copiedDataNeeded)
    {
        m_basicFeature->m_copiedDataBufferInUse = true;
        if ((m_basicFeature->m_nextCopiedDataOffset + m_basicFeature->m_dataSize) >
            m_basicFeature->m_copiedDataBufferSize)
        {
            DECODE_ASSERTMESSAGE("Copied data buffer is not large enough.");
            m_basicFeature->m_slicesInvalid = true;
            return MOS_STATUS_UNKNOWN;
        }

        uint32_t size = MOS_ALIGN_CEIL(m_basicFeature->m_dataSize, 16);
        copyParams.srcBuffer  = &(m_basicFeature->m_resDataBuffer.OsResource);
        copyParams.srcOffset  = 0;
        copyParams.destBuffer = &(m_basicFeature->m_copiedDataBuf->OsResource);
        copyParams.destOffset = m_basicFeature->m_nextCopiedDataOffset;
        copyParams.copyLength = m_basicFeature->m_dataSize;
        m_mpeg2BsCopyPkt->PushCopyParams(copyParams);

        m_basicFeature->m_copiedDataOffset = m_basicFeature->m_nextCopiedDataOffset;
        m_basicFeature->m_nextCopiedDataOffset += MOS_ALIGN_CEIL(size, MHW_CACHELINE_SIZE);

        bool immediateSubmit = true;
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, mpeg2BsCopyPktId), immediateSubmit, 0, 0));
        m_activePacketList.back().frameTrackingRequested = false;
        DECODE_CHK_STATUS(ExecuteActivePackets());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());
#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_MPEG2D_ENABLE_ID, 1, m_osInterface->pOsContext);
#endif

#ifdef _MMC_SUPPORTED
    CODECHAL_DEBUG_TOOL(
        if (m_mmcState != nullptr) {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_destSurface));
        })
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::Uninitialize()
{
    DECODE_FUNC_CALL();
    return DecodePipeline::Uninitialize();
}

MOS_STATUS Mpeg2Pipeline::ActivateDecodePackets()
{
    DECODE_FUNC_CALL();

    bool immediateSubmit = false;

    if (m_basicFeature->m_copyDummySlicePresent)
    {
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, mpeg2BsCopyPktId), immediateSubmit, 0, 0));
    }

    DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, mpeg2DecodePacketId), immediateSubmit, 0, 0));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::CreateFeatureManager()
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeMpeg2FeatureManager, m_allocator, m_hwInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Mpeg2Pipeline::DumpPicParams(
    CodecDecodeMpeg2PicParams *picParams)
{
    DECODE_FUNC_CALL();

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "m_currPic FrameIdx: " << +picParams->m_currPic.FrameIdx << std::endl;
    oss << "m_currPic PicFlags: " << +picParams->m_currPic.PicFlags << std::endl;
    oss << "m_forwardRefIdx: " << +picParams->m_forwardRefIdx << std::endl;
    oss << "m_backwardRefIdx: " << +picParams->m_backwardRefIdx << std::endl;
    oss << "m_topFieldFirst: " << +picParams->m_topFieldFirst << std::endl;
    oss << "m_secondField: " << +picParams->m_secondField << std::endl;
    oss << "m_statusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;
    //Dump union w0
    oss << "w0 m_value: " << +picParams->W0.m_value << std::endl;
    oss << "m_scanOrder: " << +picParams->W0.m_scanOrder << std::endl;
    oss << "m_intraVlcFormat: " << +picParams->W0.m_intraVlcFormat << std::endl;
    oss << "m_quantizerScaleType: " << +picParams->W0.m_quantizerScaleType << std::endl;
    oss << "m_concealmentMVFlag: " << +picParams->W0.m_concealmentMVFlag << std::endl;
    oss << "m_frameDctPrediction: " << +picParams->W0.m_frameDctPrediction << std::endl;
    oss << "m_topFieldFirst: " << +picParams->W0.m_topFieldFirst << std::endl;
    oss << "m_intraDCPrecision: " << +picParams->W0.m_intraDCPrecision << std::endl;
    //Dump union w1
    oss << "w1 m_value: " << +picParams->W1.m_value << std::endl;
    oss << "m_fcode11: " << +picParams->W1.m_fcode11 << std::endl;
    oss << "m_fcode10: " << +picParams->W1.m_fcode10 << std::endl;
    oss << "m_fcode01: " << +picParams->W1.m_fcode01 << std::endl;
    oss << "m_fcode00: " << +picParams->W1.m_fcode00 << std::endl;
    oss << "m_horizontalSize: " << +picParams->m_horizontalSize << std::endl;
    oss << "m_verticalSize: " << +picParams->m_verticalSize << std::endl;
    oss << "m_pictureCodingType: " << +picParams->m_pictureCodingType << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::DumpSliceParams(
    CodecDecodeMpeg2SliceParams *sliceParams,
    uint32_t                     numSlices)
{
    DECODE_FUNC_CALL();

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(sliceParams);

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufSlcParams,
        CodechalDbgExtType::txt);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    CodecDecodeMpeg2SliceParams *sliceControl = nullptr;

    for (uint16_t i = 0; i < numSlices; i++)
    {
        sliceControl = &sliceParams[i];

        oss << "===================================================================" << std::endl;
        oss << "Data for Slice number = " << +i << std::endl;
        oss << "m_sliceDataSize: " << +sliceControl->m_sliceDataSize << std::endl;
        oss << "m_sliceDataOffset: " << +sliceControl->m_sliceDataOffset << std::endl;
        oss << "m_macroblockOffset: " << +sliceControl->m_macroblockOffset << std::endl;
        oss << "m_sliceHorizontalPosition: " << +sliceControl->m_sliceHorizontalPosition << std::endl;
        oss << "m_sliceVerticalPosition: " << +sliceControl->m_sliceVerticalPosition << std::endl;
        oss << "m_quantiserScaleCode: " << +sliceControl->m_quantiserScaleCode << std::endl;
        oss << "m_numMbsForSlice: " << +sliceControl->m_numMbsForSlice << std::endl;
        oss << "m_numMbsForSliceOverflow: " << +sliceControl->m_numMbsForSliceOverflow << std::endl;
        oss << "m_reservedBits: " << +sliceControl->m_reservedBits << std::endl;
        oss << "m_startCodeBitOffset: " << +sliceControl->m_startCodeBitOffset << std::endl;

        std::ofstream ofs;
        if (i == 0)
        {
            ofs.open(fileName, std::ios::out);
        }
        else
        {
            ofs.open(fileName, std::ios::app);
        }
        ofs << oss.str();
        ofs.close();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::DumpMbParams(
    CodecDecodeMpeg2MbParmas *mbParams)
{
    DECODE_FUNC_CALL();

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrMbParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(mbParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "m_mbAddr: " << +mbParams->m_mbAddr << std::endl;
    //Dump union MBType
    oss << "MBType.m_intraMb: " << +mbParams->MBType.m_intraMb << std::endl;
    oss << "MBType.m_motionFwd: " << +mbParams->MBType.m_motionFwd << std::endl;
    oss << "MBType.m_motionBwd: " << +mbParams->MBType.m_motionBwd << std::endl;
    oss << "MBType.m_motion4mv: " << +mbParams->MBType.m_motion4mv << std::endl;
    oss << "MBType.m_h261Lpfilter: " << +mbParams->MBType.m_h261Lpfilter << std::endl;
    oss << "MBType.m_fieldResidual: " << +mbParams->MBType.m_fieldResidual << std::endl;
    oss << "MBType.m_mbScanMethod: " << +mbParams->MBType.m_mbScanMethod << std::endl;
    oss << "MBType.m_motionType: " << +mbParams->MBType.m_motionType << std::endl;
    oss << "MBType.m_hostResidualDiff: " << +mbParams->MBType.m_hostResidualDiff << std::endl;
    oss << "MBType.m_mvertFieldSel: " << +mbParams->MBType.m_mvertFieldSel << std::endl;
    oss << "m_mbSkipFollowing: " << +mbParams->m_mbSkipFollowing << std::endl;
    oss << "m_mbDataLoc: " << +mbParams->m_mbDataLoc << std::endl;
    oss << "m_codedBlockPattern: " << +mbParams->m_codedBlockPattern << std::endl;

    //Dump NumCoeff[CODEC_NUM_BLOCK_PER_MB]
    for (uint16_t i = 0; i < CODEC_NUM_BLOCK_PER_MB; ++i)
    {
        oss << "m_numCoeff[" << +i << "]: " << +mbParams->m_numCoeff[i] << std::endl;
    }

    //Dump motion_vectors[8],printing them in 4 value chunks per line
    for (uint8_t i = 0; i < 2; ++i)
    {
        oss << "m_motionVectors[" << +i * 4 << "-" << (+i * 4) + 3 << "]: ";
        for (uint8_t j = 0; j < 4; j++)
            oss << +mbParams->m_motionVectors[i * 4 + j] << " ";
        oss << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufMbParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2Pipeline::DumpIQParams(
    CodecMpeg2IqMatrix *matrixData)
{
    DECODE_FUNC_CALL();

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(matrixData);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    if (matrixData->m_loadIntraQuantiserMatrix)
    {
        oss << "intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +matrixData->m_intraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }
    if (matrixData->m_loadNonIntraQuantiserMatrix)
    {
        oss << "non_intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +matrixData->m_nonIntraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }
    if (matrixData->m_loadChromaIntraQuantiserMatrix)
    {
        oss << "chroma_intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +matrixData->m_chromaIntraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }
    if (matrixData->m_loadChromaNonIntraQuantiserMatrix)
    {
        oss << "chroma_non_intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +matrixData->m_chromaNonIntraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufIqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}
#endif

}
