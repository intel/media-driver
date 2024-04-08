/*
* Copyright (c) 2018-2024, Intel Corporation
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
//! \file     encode_hevc_pipeline.h
//! \brief    Defines the interface for hevc encode pipeline
//!
#include "encode_hevc_pipeline.h"
#include "encode_utils.h"
#include "encode_hevc_basic_feature.h"

namespace encode {

HevcPipeline::HevcPipeline(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : EncodePipeline(hwInterface, debugInterface)
{
}

MOS_STATUS HevcPipeline::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Initialize(settings));

#if MHW_HWCMDPARSER_ENABLED
    mhw::HwcmdParser::InitInstance(m_osInterface, mhw::HwcmdParser::AddOnMode::HEVCe);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::Uninitialize()
{
    ENCODE_FUNC_CALL();
    return EncodePipeline::Uninitialize();
}

MOS_STATUS HevcPipeline::UserFeatureReport()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodePipeline::UserFeatureReport());

    ReportUserSetting(
        m_userSettingPtr,
        "HEVC Encode Mode",
        m_codecFunction,
        MediaUserSetting::Group::Sequence);

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_SIM_IN_USE,
        m_osInterface->bSimIsActive,
        MediaUserSetting::Group::Device);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::Prepare(void *params)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Prepare(params));

    auto basicFeature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    CODECHAL_DEBUG_TOOL
    (
        m_debugInterface->m_currPic            = basicFeature->m_currOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = basicFeature->m_frameNum;
        m_debugInterface->m_frameType          = basicFeature->m_pictureCodingType;

        if (basicFeature->m_newSeq) {
            ENCODE_CHK_STATUS_RETURN(DumpSeqParams(
                basicFeature->m_hevcSeqParams));
        }
        
        ENCODE_CHK_STATUS_RETURN(DumpPicParams(
            basicFeature->m_hevcPicParams));
        
        for (uint32_t i = 0; i < basicFeature->m_numSlices; i++) {
            ENCODE_CHK_STATUS_RETURN(DumpSliceParams(
                &basicFeature->m_hevcSliceParams[i],
                basicFeature->m_hevcPicParams));
        }
    )

#if MHW_HWCMDPARSER_ENABLED
    {
        char frameType;
        switch (basicFeature->m_hevcPicParams->CodingType)
        {
        case I_TYPE:
            frameType = 'I';
            break;
        case P_TYPE:
            frameType = 'P';
            break;
        case B_TYPE:
            frameType = basicFeature->m_hevcSeqParams->LowDelayMode ? 'G' : 'B';
            break;
        }

        auto instance = mhw::HwcmdParser::GetInstance();
        if (instance)
        {
            instance->Update(frameType, (void *)m_featureManager);
        }
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::CreateBufferTracker()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::CreateStatusReport()
{
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS HevcPipeline::DumpSeqParams(
    const CODEC_HEVC_ENCODE_SEQUENCE_PARAMS *seqParams)
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

    oss << "# DDI Parameters:" << std::endl;
    oss << "wFrameWidthInMinCbMinus1 = " << +seqParams->wFrameWidthInMinCbMinus1 << std::endl;
    oss << "wFrameHeightInMinCbMinus1 = " << +seqParams->wFrameHeightInMinCbMinus1 << std::endl;
    oss << "general_profile_idc = " << +seqParams->general_profile_idc << std::endl;
    oss << "Level = " << +seqParams->Level << std::endl;
    oss << "general_tier_flag = " << +seqParams->general_tier_flag << std::endl;
    oss << "GopPicSize = " << +seqParams->GopPicSize << std::endl;
    oss << "GopRefDist = " << +seqParams->GopRefDist << std::endl;
    oss << "GopOptFlag = " << +seqParams->GopOptFlag << std::endl;
    oss << "TargetUsage = " << +seqParams->TargetUsage << std::endl;
    oss << "RateControlMethod = " << +seqParams->RateControlMethod << std::endl;
    oss << "TargetBitRate = " << +seqParams->TargetBitRate << std::endl;
    oss << "MaxBitRate = " << +seqParams->MaxBitRate << std::endl;
    oss << "MinBitRate = " << +seqParams->MinBitRate << std::endl;
    oss << "FramesRate.Numerator = " << +seqParams->FrameRate.Numerator << std::endl;
    oss << "FramesRate.Denominator = " << +seqParams->FrameRate.Denominator << std::endl;
    oss << "InitVBVBufferFullnessInBit = " << +seqParams->InitVBVBufferFullnessInBit << std::endl;
    oss << "VBVBufferSizeInBit = " << +seqParams->VBVBufferSizeInBit << std::endl;
    oss << "bResetBRC = " << +seqParams->bResetBRC << std::endl;
    oss << "GlobalSearch = " << +seqParams->GlobalSearch << std::endl;
    oss << "LocalSearch = " << +seqParams->LocalSearch << std::endl;
    oss << "EarlySkip = " << +seqParams->EarlySkip << std::endl;
    oss << "MBBRC = " << +seqParams->MBBRC << std::endl;
    oss << "ParallelBRC = " << +seqParams->ParallelBRC << std::endl;
    oss << "SliceSizeControl = " << +seqParams->SliceSizeControl << std::endl;
    oss << "SourceFormat = " << +seqParams->SourceFormat << std::endl;
    oss << "SourceBitDepth = " << +seqParams->SourceBitDepth << std::endl;
    oss << "QpAdjustment = " << +seqParams->QpAdjustment << std::endl;
    oss << "ROIValueInDeltaQP = " << +seqParams->ROIValueInDeltaQP << std::endl;
    oss << "NumB = " << +seqParams->NumOfBInGop[0] << std::endl;
    oss << "NumB1 = " << +seqParams->NumOfBInGop[1] << std::endl;
    oss << "NumB2 = " << +seqParams->NumOfBInGop[2] << std::endl;
    oss << "HierarchicalFlag = " << +seqParams->HierarchicalFlag << std::endl;
    oss << "UserMaxIFrameSize = " << +seqParams->UserMaxIFrameSize << std::endl;
    oss << "UserMaxPBFrameSize = " << +seqParams->UserMaxPBFrameSize << std::endl;
    oss << "ICQQualityFactor = " << +seqParams->ICQQualityFactor << std::endl;
    oss << "scaling_list_enable_flag = " << +seqParams->scaling_list_enable_flag << std::endl;
    oss << "sps_temporal_mvp_enable_flag = " << +seqParams->sps_temporal_mvp_enable_flag << std::endl;
    oss << "strong_intra_smoothing_enable_flag = " << +seqParams->strong_intra_smoothing_enable_flag << std::endl;
    oss << "amp_enabled_flag = " << +seqParams->amp_enabled_flag << std::endl;
    oss << "SAO_enabled_flag = " << +seqParams->SAO_enabled_flag << std::endl;
    oss << "pcm_enabled_flag = " << +seqParams->pcm_enabled_flag << std::endl;
    oss << "pcm_loop_filter_disable_flag = " << +seqParams->pcm_loop_filter_disable_flag << std::endl;
    oss << "chroma_format_idc = " << +seqParams->chroma_format_idc << std::endl;
    oss << "separate_colour_plane_flag = " << +seqParams->separate_colour_plane_flag << std::endl;
    oss << "log2_max_coding_block_size_minus3 = " << +seqParams->log2_max_coding_block_size_minus3 << std::endl;
    oss << "log2_min_coding_block_size_minus3 = " << +seqParams->log2_min_coding_block_size_minus3 << std::endl;
    oss << "log2_max_transform_block_size_minus2 = " << +seqParams->log2_max_transform_block_size_minus2 << std::endl;
    oss << "log2_min_transform_block_size_minus2 = " << +seqParams->log2_min_transform_block_size_minus2 << std::endl;
    oss << "max_transform_hierarchy_depth_intra = " << +seqParams->max_transform_hierarchy_depth_intra << std::endl;
    oss << "max_transform_hierarchy_depth_inter = " << +seqParams->max_transform_hierarchy_depth_inter << std::endl;
    oss << "log2_min_PCM_cb_size_minus3 = " << +seqParams->log2_min_PCM_cb_size_minus3 << std::endl;
    oss << "log2_max_PCM_cb_size_minus3 = " << +seqParams->log2_max_PCM_cb_size_minus3 << std::endl;
    oss << "bit_depth_luma_minus8 = " << +seqParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8 = " << +seqParams->bit_depth_chroma_minus8 << std::endl;
    oss << "pcm_sample_bit_depth_luma_minus1 = " << +seqParams->pcm_sample_bit_depth_luma_minus1 << std::endl;
    oss << "pcm_sample_bit_depth_chroma_minus1 = " << +seqParams->pcm_sample_bit_depth_chroma_minus1 << std::endl;
    oss << "Video Surveillance Mode = " << +seqParams->bVideoSurveillance << std::endl;
    oss << "ScenarioInfo = " << +seqParams->ScenarioInfo << std::endl;
    oss << "Frame Size Tolerance = " << +seqParams->FrameSizeTolerance << std::endl;
    oss << "palette_mode_enabled_flag = " << +seqParams->palette_mode_enabled_flag << std::endl;
    oss << "RGBInputStudioRange = " << +seqParams->RGBInputStudioRange << std::endl;
    oss << "ConvertedYUVStudioRange = " << +seqParams->ConvertedYUVStudioRange << std::endl;
    oss << "InputColorSpace = " << +seqParams->InputColorSpace << std::endl;
    oss << "Look Ahead Depth = " << +seqParams->LookaheadDepth << std::endl;
    oss << "Look Ahead Phase = " << +seqParams->bLookAheadPhase << std::endl;

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

MOS_STATUS HevcPipeline::DumpPicParams(
    const CODEC_HEVC_ENCODE_PICTURE_PARAMS *picParams)
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

    oss << "# DDI Parameters:" << std::endl;
    oss << "CurrOriginalPic = " << +picParams->CurrOriginalPic.FrameIdx << std::endl;
    oss << "CurrReconstructedPic = " << +picParams->CurrReconstructedPic.FrameIdx << std::endl;
    oss << "CollocatedRefPicIndex = " << +picParams->CollocatedRefPicIndex << std::endl;

    for (uint16_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; ++i)
    {
        oss << "RefFrameList[" << +i << "] = " << +picParams->RefFrameList[i].FrameIdx << std::endl;
    }

    oss << "CurrPicOrderCnt = " << +picParams->CurrPicOrderCnt << std::endl;

    for (uint16_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; ++i)
    {
        oss << "RefFramePOCList[" << +i << "] = " << +picParams->RefFramePOCList[i] << std::endl;
    }

    oss << "CodingType = " << +picParams->CodingType << std::endl;
    oss << "HierarchLevelPlus1 = " << +picParams->HierarchLevelPlus1 << std::endl;
    oss << "NumSlices = " << +picParams->NumSlices << std::endl;
    oss << "tiles_enabled_flag = " << +picParams->tiles_enabled_flag << std::endl;

    if (picParams->tiles_enabled_flag)
    {
        oss << "num_tile_columns = " << +picParams->num_tile_columns_minus1 + 1 << std::endl;
        for (uint32_t i = 0; i <= picParams->num_tile_columns_minus1; i++)
        {
            oss << "tile_column_width[" << +i << "] = " << +picParams->tile_column_width[i] << std::endl;
        }
        oss << "num_tile_rows = " << +picParams->num_tile_rows_minus1 + 1 << std::endl;
        for (uint32_t i = 0; i <= picParams->num_tile_rows_minus1; i++)
        {
            oss << "tile_row_height[" << +i << "] = " << +picParams->tile_row_height[i] << std::endl;
        }
    }

    oss << "entropy_coding_sync_enabled_flag = " << +picParams->entropy_coding_sync_enabled_flag << std::endl;
    oss << "sign_data_hiding_flag = " << +picParams->sign_data_hiding_flag << std::endl;
    oss << "constrained_intra_pred_flag = " << +picParams->constrained_intra_pred_flag << std::endl;
    oss << "transform_skip_enabled_flag = " << +picParams->transform_skip_enabled_flag << std::endl;
    oss << "transquant_bypass_enabled_flag = " << +picParams->transquant_bypass_enabled_flag << std::endl;
    oss << "cu_qp_delta_enabled_flag = " << +picParams->cu_qp_delta_enabled_flag << std::endl;
    oss << "weighted_pred_flag = " << +picParams->weighted_pred_flag << std::endl;
    oss << "weighted_bipred_flag = " << +picParams->weighted_bipred_flag << std::endl;
    oss << "bEnableGPUWeightedPrediction = " << +picParams->bEnableGPUWeightedPrediction << std::endl;
    oss << "loop_filter_across_slices_flag = " << +picParams->loop_filter_across_slices_flag << std::endl;
    oss << "loop_filter_across_tiles_flag = " << +picParams->loop_filter_across_tiles_flag << std::endl;
    oss << "scaling_list_data_present_flag = " << +picParams->scaling_list_data_present_flag << std::endl;
    oss << "dependent_slice_segments_enabled_flag = " << +picParams->dependent_slice_segments_enabled_flag << std::endl;
    oss << "bLastPicInSeq = " << +picParams->bLastPicInSeq << std::endl;
    oss << "bLastPicInStream = " << +picParams->bLastPicInStream << std::endl;
    oss << "bUseRawPicForRef = " << +picParams->bUseRawPicForRef << std::endl;
    oss << "bEmulationByteInsertion = " << +picParams->bEmulationByteInsertion << std::endl;
    oss << "bEnableRollingIntraRefresh = " << +picParams->bEnableRollingIntraRefresh << std::endl;
    oss << "BRCPrecision = " << +picParams->BRCPrecision << std::endl;
    oss << "bScreenContent = " << +picParams->bScreenContent << std::endl;
    oss << "QpY = " << +picParams->QpY << std::endl;
    oss << "diff_cu_qp_delta_depth = " << +picParams->diff_cu_qp_delta_depth << std::endl;
    oss << "pps_cb_qp_offset = " << +picParams->pps_cb_qp_offset << std::endl;
    oss << "pps_cr_qp_offset = " << +picParams->pps_cr_qp_offset << std::endl;
    oss << "num_tile_columns_minus1 = " << +picParams->num_tile_columns_minus1 << std::endl;
    oss << "num_tile_rows_minus1 = " << +picParams->num_tile_rows_minus1 << std::endl;
    oss << "log2_parallel_merge_level_minus2 = " << +picParams->log2_parallel_merge_level_minus2 << std::endl;
    oss << "num_ref_idx_l0_default_active_minus1 = " << +picParams->num_ref_idx_l0_default_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_default_active_minus1 = " << +picParams->num_ref_idx_l1_default_active_minus1 << std::endl;
    oss << "LcuMaxBitsizeAllowed = " << +picParams->LcuMaxBitsizeAllowed << std::endl;
    oss << "IntraInsertionLocation = " << +picParams->IntraInsertionLocation << std::endl;
    oss << "IntraInsertionSize = " << +picParams->IntraInsertionSize << std::endl;
    oss << "QpDeltaForInsertedIntra = " << +picParams->QpDeltaForInsertedIntra << std::endl;
    oss << "StatusReportFeedbackNumber = " << +picParams->StatusReportFeedbackNumber << std::endl;
    oss << "slice_pic_parameter_set_id = " << +picParams->slice_pic_parameter_set_id << std::endl;
    oss << "nal_unit_type = " << +picParams->nal_unit_type << std::endl;
    oss << "MaxSliceSizeInBytes = " << +picParams->MaxSliceSizeInBytes << std::endl;
    oss << "NumROI = " << +picParams->NumROI << std::endl;

    for (uint8_t i = 0; i < 16; ++i)
    {
        oss << "ROI[" << +i << "] = ";
        oss << picParams->ROI[i].Top << " ";
        oss << picParams->ROI[i].Bottom << " ";
        oss << picParams->ROI[i].Left << " ";
        oss << picParams->ROI[i].Right << " ";
        oss << +picParams->ROI[i].PriorityLevelOrDQp << std::endl;
    }
    oss << "MaxDeltaQp = " << +picParams->MaxDeltaQp << std::endl;
    oss << "MinDeltaQp = " << +picParams->MinDeltaQp << std::endl;
    oss << "NumDirtyRects = " << +picParams->NumDirtyRects << std::endl;

    if ((picParams->NumDirtyRects > 0) && picParams->pDirtyRect)
    {
        for (uint16_t i = 0; i < picParams->NumDirtyRects; i++)
        {
            oss << "pDirtyRect[" << +i << "].Bottom = " << +picParams->pDirtyRect[i].Bottom << std::endl;
            oss << "pDirtyRect[" << +i << "].Top = " << +picParams->pDirtyRect[i].Top << std::endl;
            oss << "pDirtyRect[" << +i << "].Left = " << +picParams->pDirtyRect[i].Left << std::endl;
            oss << "pDirtyRect[" << +i << "].Right = " << +picParams->pDirtyRect[i].Right << std::endl;
        }
    }

    oss << "TargetFrameSize = " << +picParams->TargetFrameSize << std::endl;
    oss << "pps_curr_pic_ref_enabled_flag = " << +picParams->pps_curr_pic_ref_enabled_flag << std::endl;

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
                << " = \"" << m_debugInterface->m_bufferDumpFrameNum << "\"" << std::endl;
            ofs << "PicParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DumpSliceParams(
    const CODEC_HEVC_ENCODE_SLICE_PARAMS *  sliceParams,
    const CODEC_HEVC_ENCODE_PICTURE_PARAMS *picParams)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    m_debugInterface->m_sliceId = sliceParams->slice_id;  // set here for constructing debug file name

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "slice_segment_address = " << +sliceParams->slice_segment_address << std::endl;
    oss << "NumLCUsInSlice = " << +sliceParams->NumLCUsInSlice << std::endl;

    // RefPicList (2 x CODEC_MAX_NUM_REF_FRAME_HEVC)
    for (uint8_t i = 0; i < 2; ++i)
    {
        for (uint8_t j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; ++j)
        {
            oss << "RefPicList[" << +i << "][" << +j << "] = " << +sliceParams->RefPicList[i][j].PicEntry << std::endl;
        }
    }

    oss << "num_ref_idx_l0_active_minus1 = " << +sliceParams->num_ref_idx_l0_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_active_minus1 = " << +sliceParams->num_ref_idx_l1_active_minus1 << std::endl;
    oss << "bLastSliceOfPic = " << +sliceParams->bLastSliceOfPic << std::endl;
    oss << "dependent_slice_segment_flag = " << +sliceParams->dependent_slice_segment_flag << std::endl;
    oss << "slice_temporal_mvp_enable_flag = " << +sliceParams->slice_temporal_mvp_enable_flag << std::endl;
    oss << "slice_type = " << +sliceParams->slice_type << std::endl;
    oss << "slice_sao_luma_flag = " << +sliceParams->slice_sao_luma_flag << std::endl;
    oss << "slice_sao_chroma_flag = " << +sliceParams->slice_sao_chroma_flag << std::endl;
    oss << "mvd_l1_zero_flag = " << +sliceParams->mvd_l1_zero_flag << std::endl;
    oss << "cabac_init_flag = " << +sliceParams->cabac_init_flag << std::endl;
    oss << "slice_deblocking_filter_disable_flag = " << +sliceParams->slice_deblocking_filter_disable_flag << std::endl;
    oss << "collocated_from_l0_flag = " << +sliceParams->collocated_from_l0_flag << std::endl;
    oss << "slice_qp_delta = " << +sliceParams->slice_qp_delta << std::endl;
    oss << "slice_cb_qp_offset = " << +sliceParams->slice_cb_qp_offset << std::endl;
    oss << "slice_cr_qp_offset = " << +sliceParams->slice_cr_qp_offset << std::endl;
    oss << "beta_offset_div2 = " << +sliceParams->beta_offset_div2 << std::endl;
    oss << "tc_offset_div2 = " << +sliceParams->tc_offset_div2 << std::endl;
    oss << "luma_log2_weight_denom = " << +sliceParams->luma_log2_weight_denom << std::endl;
    oss << "delta_chroma_log2_weight_denom = " << +sliceParams->delta_chroma_log2_weight_denom << std::endl;

    for (uint8_t i = 0; i < 2; ++i)
    {
        for (uint8_t j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; ++j)
        {
            oss << "luma_offset[" << +i << "][" << +j << "] = " << +sliceParams->luma_offset[i][j] << std::endl;

            oss << "delta_luma_weight[" << +i << "][" << +j << "] = " << +sliceParams->delta_luma_weight[i][j] << std::endl;
        }
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        for (uint8_t j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; ++j)
        {
            for (uint8_t k = 0; k < 2; ++k)
            {
                oss << "chroma_offset[" << +i << "][" << +j << "][" << +k << "] = " << +sliceParams->chroma_offset[i][j][k] << std::endl;
                oss << "delta_chroma_weight[" << +i << "][" << +j << "][" << +k << "] = " << +sliceParams->delta_chroma_weight[i][j][k] << std::endl;
            }
        }
    }

    oss << "PredWeightTableBitOffset = " << +sliceParams->PredWeightTableBitOffset << std::endl;
    oss << "PredWeightTableBitLength = " << +sliceParams->PredWeightTableBitLength << std::endl;
    oss << "MaxNumMergeCand = " << +sliceParams->MaxNumMergeCand << std::endl;
    oss << "slice_id = " << +sliceParams->slice_id << std::endl;
    oss << "SliceHeaderByteOffset = " << +sliceParams->SliceHeaderByteOffset << std::endl;
    oss << "BitLengthSliceHeaderStartingPortion = " << +sliceParams->BitLengthSliceHeaderStartingPortion << std::endl;
    oss << "SliceSAOFlagBitOffset = " << +sliceParams->SliceSAOFlagBitOffset << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSlcParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "SlcParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}
