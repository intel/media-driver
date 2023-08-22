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
//! \file     encode_vp9_pipeline.h
//! \brief    Defines the interface for vp9 encode pipeline
//!
 
#include "encode_vp9_pipeline.h"
#include "encode_utils.h"

namespace encode
{
Vp9Pipeline::Vp9Pipeline(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : EncodePipeline(hwInterface, debugInterface)
{
}

MOS_STATUS Vp9Pipeline::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(InitUserSetting(m_userSettingPtr));
    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Initialize(settings));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::Uninitialize()
{
    ENCODE_FUNC_CALL();
    return EncodePipeline::Uninitialize();
}

MOS_STATUS Vp9Pipeline::ReportUserSettingValue(const std::string &valueName,
    const MediaUserSetting::Value &                               value,
    const MediaUserSetting::Group &                               group)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

#if (_DEBUG || _RELEASE_INTERNAL)
    eStatus = ReportUserSettingForDebug(
        m_userSettingPtr,
        valueName,
        value,
        group);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // Print out an error message in non-release builds. Do not report an error: execution can continue.
        ENCODE_NORMALMESSAGE("Failed to write an user feature key value. Status: ", eStatus);
        eStatus = MOS_STATUS_SUCCESS;
    }
#endif

    return eStatus;
}

MOS_STATUS Vp9Pipeline::DeclareUserSettingKeyValue(const std::string &valueName,
    const MediaUserSetting::Group &                                   group,
    const MediaUserSetting::Value &                                   defaultValue,
    bool                                                              isReportKey)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

#if (_DEBUG || _RELEASE_INTERNAL)
    eStatus = DeclareUserSettingKeyForDebug(
        m_userSettingPtr,
        valueName,
        group,
        defaultValue,
        isReportKey);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // Print out an error message in non-release builds. Do not report an error: execution can continue.
        if (eStatus != MOS_STATUS_FILE_EXISTS)
        {
            ENCODE_NORMALMESSAGE("Failed to declare an user feature key value. Status: ", eStatus);
        }
        eStatus = MOS_STATUS_SUCCESS;
    }
#endif

    return eStatus;
}

MOS_STATUS Vp9Pipeline::UserFeatureReport()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodePipeline::UserFeatureReport());
#if (_DEBUG || _RELEASE_INTERNAL)
    ENCODE_CHK_STATUS_RETURN(
        ReportUserSettingValue(__MEDIA_USER_FEATURE_VALUE_SIM_IN_USE,
            m_osInterface->bSimIsActive,
            MediaUserSetting::Group::Device));
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::Prepare(void *params)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Prepare(params));

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(Vp9FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->m_currPic            = basicFeature->m_currOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = basicFeature->m_frameNum + 1;  // +1 for debug purpose with legacy
        m_debugInterface->m_frameType          = basicFeature->m_pictureCodingType;

        if (basicFeature->m_newSeq) {
            ENCODE_CHK_STATUS_RETURN(DumpSeqParams(
                basicFeature->m_vp9SeqParams));
        }

        ENCODE_CHK_STATUS_RETURN(DumpPicParams(
            basicFeature->m_vp9PicParams));

        ENCODE_CHK_STATUS_RETURN(DumpSegmentParams(
            basicFeature->m_vp9SegmentParams));)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::CreateBufferTracker()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::CreateStatusReport()
{
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Vp9Pipeline::DumpSegmentParams(
    const CODEC_VP9_ENCODE_SEGMENT_PARAMS *segmentParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSegmentParams))
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_CHK_NULL(segmentParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint8_t i = 0; i < 8; ++i)
    {
        oss << "Segment_id = "              << std::dec << +i << std::endl;
        oss << "SegmentReferenceEnabled = " << std::dec << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReferenceEnabled << std::endl;
        oss << "SegmentReference = "        << std::dec << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReference << std::endl;
        oss << "SegmentSkipped = "          << std::dec << +segmentParams->SegData[i].SegmentFlags.fields.SegmentSkipped << std::endl;
        oss << "SegmentLFLevelDelta = "     << std::dec << +segmentParams->SegData[i].SegmentLFLevelDelta << std::endl;
        oss << "SegmentQIndexDelta = "      << std::dec << +segmentParams->SegData[i].SegmentQIndexDelta << std::endl;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "SegmentParamFileParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSegmentParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::DumpSeqParams(
    const CODEC_VP9_ENCODE_SEQUENCE_PARAMS *seqParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSeqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(seqParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:"    << std::endl;
    oss << "MaxFrameWidth = "     << std::dec << +seqParams->wMaxFrameWidth << std::endl;
    oss << "MaxFrameHeight = "    << std::dec << +seqParams->wMaxFrameHeight << std::endl;
    oss << "GopPicSize = "        << std::dec << +seqParams->GopPicSize << std::endl;
    oss << "TargetUsage = "       << std::dec << +seqParams->TargetUsage << std::endl;
    oss << "RateControlMethod = " << std::dec << +seqParams->RateControlMethod << std::endl;

    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "TargetBitRate[" << +i << "] = " << std::dec << +seqParams->TargetBitRate[i] << std::endl;
    }
    oss << "MaxBitRate = "                        << std::dec << +seqParams->MaxBitRate << std::endl;
    oss << "MinBitRate = "                        << std::dec << +seqParams->MinBitRate << std::endl;
    oss << "InitVBVBufferFullnessInBit = "        << std::dec << +seqParams->InitVBVBufferFullnessInBit << std::endl;
    oss << "VBVBufferSizeInBit = "                << std::dec << +seqParams->VBVBufferSizeInBit << std::endl;
    oss << "OptimalVBVBufferLevelInBit = "        << std::dec << +seqParams->OptimalVBVBufferLevelInBit << std::endl;
    oss << "UpperVBVBufferLevelThresholdInBit = " << std::dec << +seqParams->UpperVBVBufferLevelThresholdInBit << std::endl;
    oss << "LowerVBVBufferLevelThresholdInBit = " << std::dec << +seqParams->LowerVBVBufferLevelThresholdInBit << std::endl;
    oss << "DisplayFormatSwizzle = "              << std::dec << +seqParams->SeqFlags.fields.DisplayFormatSwizzle << std::endl;
    // begining of union/struct
    oss << "# bResetBRC = "                       << std::dec << +seqParams->SeqFlags.fields.bResetBRC << std::endl;
    oss << "# bNoFrameHeaderInsertion = "         << std::dec << +seqParams->SeqFlags.fields.bNoFrameHeaderInsertion << std::endl;
    // Next 5 fields not currently implemented.  nullptr output
    oss << "# UseRawReconRef = "                  << std::dec << +seqParams->SeqFlags.fields.bUseRawReconRef << std::endl;
    oss << "# MBBRC = "                           << std::dec << +seqParams->SeqFlags.fields.MBBRC << std::endl;
    oss << "EnableDynamicScaling = "              << std::dec << +seqParams->SeqFlags.fields.EnableDynamicScaling << std::endl;
    oss << "SourceFormat = "                      << std::dec << +seqParams->SeqFlags.fields.SourceFormat << std::endl;
    oss << "SourceBitDepth = "                    << std::dec << +seqParams->SeqFlags.fields.SourceBitDepth << std::endl;
    oss << "EncodedFormat = "                     << std::dec << +seqParams->SeqFlags.fields.EncodedFormat << std::endl;
    oss << "EncodedBitDepth = "                   << std::dec << +seqParams->SeqFlags.fields.EncodedBitDepth << std::endl;
    oss << "DisplayFormatSwizzle = "              << std::dec << +seqParams->SeqFlags.fields.DisplayFormatSwizzle << std::endl;
    // end of union/struct

    oss << "UserMaxFrameSize = " << std::dec << +seqParams->UserMaxFrameSize << std::endl;
    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "FrameRateNumerator[" << +i << "] = "   << std::dec << +seqParams->FrameRate[i].uiNumerator << std::endl;
        oss << "FrameRateDenominator[" << +i << "] = " << std::dec << +seqParams->FrameRate[i].uiDenominator << std::endl;
    }

    oss << "NumTemporalLayersMinus1 = " << std::dec << +seqParams->NumTemporalLayersMinus1 << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSeqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "SeqParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::DumpPicParams(
    const CODEC_VP9_ENCODE_PIC_PARAMS *picParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:"       << std::endl;
    oss << "SrcFrameHeightMinus1 = " << std::dec << +picParams->SrcFrameHeightMinus1 << std::endl;
    oss << "SrcFrameWidthMinus1 = "  << std::dec << +picParams->SrcFrameWidthMinus1 << std::endl;
    oss << "CurrOriginalPic = "      << std::dec << +picParams->CurrOriginalPic.FrameIdx << std::endl;
    oss << "CurrReconstructedPic = " << std::dec << +picParams->CurrReconstructedPic.FrameIdx << std::endl;

    for (uint16_t i = 0; i < CODEC_VP9_NUM_REF_FRAMES; ++i)
    {
        oss << "RefFrameList[" << +i << "] = " << std::dec << +picParams->RefFrameList[i].FrameIdx << std::endl;
    }
    oss << "frame_type = "                   << std::dec << +picParams->PicFlags.fields.frame_type << std::endl;
    oss << "show_frame = "                   << std::dec << +picParams->PicFlags.fields.show_frame << std::endl;
    oss << "error_resilient_mode = "         << std::dec << +picParams->PicFlags.fields.error_resilient_mode << std::endl;
    oss << "intra_only = "                   << std::dec << +picParams->PicFlags.fields.intra_only << std::endl;
    oss << "allow_high_precision_mv = "      << std::dec << +picParams->PicFlags.fields.allow_high_precision_mv << std::endl;
    oss << "mcomp_filter_type = "            << std::dec << +picParams->PicFlags.fields.mcomp_filter_type << std::endl;
    oss << "frame_parallel_decoding_mode = " << std::dec << +picParams->PicFlags.fields.frame_parallel_decoding_mode << std::endl;
    oss << "segmentation_enabled = "         << std::dec << +picParams->PicFlags.fields.segmentation_enabled << std::endl;
    oss << "segmentation_temporal_update = " << std::dec << +picParams->PicFlags.fields.segmentation_temporal_update << std::endl;
    oss << "segmentation_update_map = "      << std::dec << +picParams->PicFlags.fields.segmentation_update_map << std::endl;
    oss << "reset_frame_context = "          << std::dec << +picParams->PicFlags.fields.reset_frame_context << std::endl;
    oss << "refresh_frame_context = "        << std::dec << +picParams->PicFlags.fields.refresh_frame_context << std::endl;
    oss << "frame_context_idx = "            << std::dec << +picParams->PicFlags.fields.frame_context_idx << std::endl;
    oss << "LosslessFlag = "                 << std::dec << +picParams->PicFlags.fields.LosslessFlag << std::endl;
    oss << "comp_prediction_mode = "         << std::dec << +picParams->PicFlags.fields.comp_prediction_mode << std::endl;
    oss << "super_frame = "                  << std::dec << +picParams->PicFlags.fields.super_frame << std::endl;
    oss << "seg_id_block_size = "            << std::dec << +picParams->PicFlags.fields.seg_id_block_size << std::endl;
    oss << "seg_update_data = "              << std::dec << +picParams->PicFlags.fields.seg_update_data << std::endl;
    oss << "LastRefIdx = "                   << std::dec << +picParams->RefFlags.fields.LastRefIdx << std::endl;
    oss << "LastRefSignBias = "              << std::dec << +picParams->RefFlags.fields.LastRefSignBias << std::endl;
    oss << "GoldenRefIdx = "                 << std::dec << +picParams->RefFlags.fields.GoldenRefIdx << std::endl;
    oss << "GoldenRefSignBias = "            << std::dec << +picParams->RefFlags.fields.GoldenRefSignBias << std::endl;
    oss << "AltRefIdx = "                    << std::dec << +picParams->RefFlags.fields.AltRefIdx << std::endl;
    oss << "AltRefSignBias = "               << std::dec << +picParams->RefFlags.fields.AltRefSignBias << std::endl;
    oss << "ref_frame_ctrl_l0 = "            << std::dec << +picParams->RefFlags.fields.ref_frame_ctrl_l0 << std::endl;
    oss << "ref_frame_ctrl_l1 = "            << std::dec << +picParams->RefFlags.fields.ref_frame_ctrl_l1 << std::endl;
    oss << "refresh_frame_flags = "          << std::dec << +picParams->RefFlags.fields.refresh_frame_flags << std::endl;
    oss << "LumaACQIndex = "                 << std::dec << +picParams->LumaACQIndex << std::endl;
    oss << "LumaDCQIndexDelta = "            << std::dec << +picParams->LumaDCQIndexDelta << std::endl;
    oss << "ChromaACQIndexDelta = "          << std::dec << +picParams->ChromaACQIndexDelta << std::endl;
    oss << "ChromaDCQIndexDelta = "          << std::dec << +picParams->ChromaDCQIndexDelta << std::endl;
    oss << "filter_level = "                 << std::dec << +picParams->filter_level << std::endl;
    oss << "sharpness_level = "              << std::dec << +picParams->sharpness_level << std::endl;

    for (uint8_t i = 0; i < 4; ++i)
    {
        oss << "LFRefDelta[" << +i << "] = " << std::dec << +picParams->LFRefDelta[i] << std::endl;
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        oss << "LFModeDelta[" << +i << "] = " << std::dec << +picParams->LFModeDelta[i] << std::endl;
    }

    oss << "BitOffsetForLFRefDelta = "         << std::dec << +picParams->BitOffsetForLFRefDelta << std::endl;
    oss << "BitOffsetForLFModeDelta = "        << std::dec << +picParams->BitOffsetForLFModeDelta << std::endl;
    oss << "BitOffsetForLFLevel = "            << std::dec << +picParams->BitOffsetForLFLevel << std::endl;
    oss << "BitOffsetForQIndex = "             << std::dec << +picParams->BitOffsetForQIndex << std::endl;
    oss << "BitOffsetForFirstPartitionSize = " << std::dec << +picParams->BitOffsetForFirstPartitionSize << std::endl;
    oss << "BitOffsetForSegmentation = "       << std::dec << +picParams->BitOffsetForSegmentation << std::endl;
    oss << "BitSizeForSegmentation = "         << std::dec << +picParams->BitSizeForSegmentation << std::endl;
    oss << "log2_tile_rows = "                 << std::dec << +picParams->log2_tile_rows << std::endl;
    oss << "log2_tile_columns = "              << std::dec << +picParams->log2_tile_columns << std::endl;
    oss << "temporal_id = "                    << std::dec << +picParams->temporal_id << std::endl;
    oss << "StatusReportFeedbackNumber = "     << std::dec << +picParams->StatusReportFeedbackNumber << std::endl;
    oss << "SkipFrameFlag = "                  << std::dec << +picParams->SkipFrameFlag << std::endl;
    oss << "NumSkipFrames = "                  << std::dec << +picParams->NumSkipFrames << std::endl;
    oss << "SizeSkipFrames = "                 << std::dec << +picParams->SizeSkipFrames << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "PicNum"
                << " = " << m_debugInterface->m_bufferDumpFrameNum << std::endl;
            ofs << "PicParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace encode