/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     encode_av1_pipeline.h
//! \brief    Defines the interface for av1 encode pipeline
//!
#include "encode_av1_pipeline.h"
#include "encode_utils.h"
#include "encode_av1_basic_feature.h"

namespace encode
{
Av1Pipeline::Av1Pipeline(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : EncodePipeline(hwInterface, debugInterface)
{
}

MOS_STATUS Av1Pipeline::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    m_dualEncEnable = ((CodechalSetting *)settings)->isDualEncEnabled;

#if (_DEBUG || _RELEASE_INTERNAL)
    MediaUserSetting::Value outValue;
    ReadUserSetting(m_userSettingPtr,
        outValue,
        "AV1 Dual Encoder Force Off",
        MediaUserSetting::Group::Sequence);

    m_dualEncEnable &= !outValue.Get<bool>();
#endif

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Initialize(settings));

#if MHW_HWCMDPARSER_ENABLED
    mhw::HwcmdParser::InitInstance(m_osInterface, mhw::HwcmdParser::AddOnMode::AV1e);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::Uninitialize()
{
    ENCODE_FUNC_CALL();

    return EncodePipeline::Uninitialize();
}

MOS_STATUS Av1Pipeline::UserFeatureReport()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodePipeline::UserFeatureReport());

    //TBD

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_SIM_IN_USE,
        m_osInterface->bSimIsActive,
        MediaUserSetting::Group::Device);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::Prepare(void *params)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Prepare(params));

    auto basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->m_currPic            = basicFeature->m_currOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = basicFeature->m_frameNum;
        m_debugInterface->m_frameType          = basicFeature->m_pictureCodingType;

        if (basicFeature->m_newSeq) {
            ENCODE_CHK_STATUS_RETURN(DumpSeqParams(
                basicFeature->m_av1SeqParams));
        }

        ENCODE_CHK_STATUS_RETURN(DumpPicParams(
            basicFeature->m_av1PicParams));

        uint32_t                            numTilegroups   = 0;
        PCODEC_AV1_ENCODE_TILE_GROUP_PARAMS tileGroupParams = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileGroupInfo, tileGroupParams, numTilegroups);

        ENCODE_CHK_NULL_RETURN(tileGroupParams);

        for (uint32_t i = 0; i < numTilegroups; i++, tileGroupParams++){
            ENCODE_CHK_STATUS_RETURN(DumpTileGroupParams(
                tileGroupParams, i))}
        )

#if MHW_HWCMDPARSER_ENABLED
    char frameType = '\0';
    switch (basicFeature->m_pictureCodingType)
    {
    case I_TYPE:
        frameType = 'I';
        break;
    case P_TYPE:
        if (basicFeature->m_ref.IsLowDelay())
            frameType = 'G';
        else if (basicFeature->m_av1PicParams->ref_frame_ctrl_l1.RefFrameCtrl.value != 0)
            frameType = 'B';
        else
            frameType = 'P';
        break;
    default:
        frameType = '\0';
        break;
    }

    auto instance = mhw::HwcmdParser::GetInstance();
    if (instance)
    {
        instance->Update(frameType, (void *)m_featureManager);
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::CreateBufferTracker()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::CreateStatusReport()
{
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Av1Pipeline::DumpSeqParams(
    const CODEC_AV1_ENCODE_SEQUENCE_PARAMS *seqParams)
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
    oss << "seq_profile = " << +seqParams->seq_profile << std::endl;
    oss << "seq_level_idx = " << +seqParams->seq_level_idx << std::endl;
    oss << "GopPicSize = " << +seqParams->GopPicSize << std::endl;
    oss << "GopRefDist = " << +seqParams->GopRefDist << std::endl;
    oss << "GopOptFlag = " << +seqParams->GopOptFlag << std::endl;
    oss << "TargetUsage = " << +seqParams->TargetUsage << std::endl;
    oss << "RateControlMethod = " << +seqParams->RateControlMethod << std::endl;

    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "TargetBitRate[" << +i << "] = " << +seqParams->TargetBitRate[i] << std::endl;
    }

    oss << "MaxBitRate = " << +seqParams->MaxBitRate << std::endl;
    oss << "MinBitRate = " << +seqParams->MinBitRate << std::endl;
    oss << "InitVBVBufferFullnessInBit = " << +seqParams->InitVBVBufferFullnessInBit << std::endl;
    oss << "VBVBufferSizeInBit = " << +seqParams->VBVBufferSizeInBit << std::endl;
    oss << "OptimalVBVBufferLevelInBit = " << +seqParams->OptimalVBVBufferLevelInBit << std::endl;
    oss << "UpperVBVBufferLevelThresholdInBit = " << +seqParams->UpperVBVBufferLevelThresholdInBit << std::endl;
    oss << "LowerVBVBufferLevelThresholdInBit = " << +seqParams->LowerVBVBufferLevelThresholdInBit << std::endl;
    oss << "ResetBRC = " << +seqParams->SeqFlags.fields.ResetBRC << std::endl;
    oss << "StillPicture = " << +seqParams->SeqFlags.fields.StillPicture << std::endl;
    oss << "UseRawReconRef = " << +seqParams->SeqFlags.fields.UseRawReconRef << std::endl;
    oss << "DisplayFormatSwizzle = " << +seqParams->SeqFlags.fields.DisplayFormatSwizzle << std::endl;
    oss << "bLookAheadPhase = " << +seqParams->SeqFlags.fields.bLookAheadPhase << std::endl;
    oss << "HierarchicalFlag = " << +seqParams->SeqFlags.fields.HierarchicalFlag << std::endl;
    oss << "Reserved0 = " << +seqParams->SeqFlags.fields.Reserved0 << std::endl;
    oss << "SeqFlags.value = " << +seqParams->SeqFlags.value << std::endl;
    oss << "UserMaxIFrameSize = " << +seqParams->UserMaxIFrameSize << std::endl;
    oss << "UserMaxPBFrameSize = " << +seqParams->UserMaxPBFrameSize << std::endl;

    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "FrameRate[" << +i << "].Numerator = " << +seqParams->FrameRate[i].Numerator << std::endl;
        oss << "FrameRate[" << +i << "].Denominator = " << +seqParams->FrameRate[i].Denominator << std::endl;
    }

    oss << "NumTemporalLayersMinus1 = " << +seqParams->NumTemporalLayersMinus1 << std::endl;
    oss << "ICQQualityFactor = " << +seqParams->ICQQualityFactor << std::endl;
    oss << "InputColorSpace = " << +seqParams->InputColorSpace << std::endl;
    oss << "ScenarioInfo = " << +seqParams->ScenarioInfo << std::endl;
    oss << "ContentInfo = " << +seqParams->ContentInfo << std::endl;
    oss << "FrameSizeTolerance = " << +seqParams->FrameSizeTolerance << std::endl;
    oss << "SlidingWindowSize = " << +seqParams->SlidingWindowSize << std::endl;
    oss << "MaxBitRatePerSlidingWindow = " << +seqParams->MaxBitRatePerSlidingWindow << std::endl;
    oss << "MinBitRatePerSlidingWindow = " << +seqParams->MinBitRatePerSlidingWindow << std::endl;
    oss << "enable_order_hint = " << +seqParams->CodingToolFlags.fields.enable_order_hint << std::endl;
    oss << "enable_superres = " << +seqParams->CodingToolFlags.fields.enable_superres << std::endl;
    oss << "enable_cdef = " << +seqParams->CodingToolFlags.fields.enable_cdef << std::endl;
    oss << "enable_restoration = " << +seqParams->CodingToolFlags.fields.enable_restoration << std::endl;
    oss << "enable_warped_motion = " << +seqParams->CodingToolFlags.fields.enable_warped_motion << std::endl;
    oss << "Reserved3 = " << +seqParams->CodingToolFlags.fields.Reserved3 << std::endl;
    oss << "CodingToolFlags.value = " << +seqParams->CodingToolFlags.value << std::endl;

    oss << "order_hint_bits_minus_1 = " << +seqParams->order_hint_bits_minus_1 << std::endl;
    oss << "LookaheadDepth = " << +seqParams->LookaheadDepth << std::endl;
    oss << "TargetFrameSizeConfidence = " << +seqParams->TargetFrameSizeConfidence << std::endl;
    oss << "Reserved8b2 = " << +seqParams->Reserved8b2 << std::endl;
    oss << "Reserved8b3 = " << +seqParams->Reserved8b3 << std::endl;

    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "Reserved32b[" << +i << "] = " << +seqParams->Reserved32b[i] << std::endl;
    }

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

MOS_STATUS Av1Pipeline::DumpPicParams(
    const CODEC_AV1_ENCODE_PICTURE_PARAMS *picParams)
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
    oss << "frame_width_minus1 = " << +picParams->frame_width_minus1 << std::endl;
    oss << "frame_height_minus1 = " << +picParams->frame_height_minus1 << std::endl;
    oss << "NumTileGroupsMinus1 = " << +picParams->NumTileGroupsMinus1 << std::endl;
    oss << "Reserved8b = " << +picParams->Reserved8b << std::endl;
    oss << "CurrOriginalPic.FrameIdx = " << +picParams->CurrOriginalPic.FrameIdx << std::endl;
    oss << "CurrOriginalPic.PicFlags = " << +picParams->CurrOriginalPic.PicFlags << std::endl;
    oss << "CurrOriginalPic.PicEntry = " << +picParams->CurrOriginalPic.PicEntry << std::endl;
    oss << "CurrReconstructedPic.FrameIdx = " << +picParams->CurrReconstructedPic.FrameIdx << std::endl;
    oss << "CurrReconstructedPic.PicEntry = " << +picParams->CurrReconstructedPic.PicEntry << std::endl;
    oss << "CurrReconstructedPic.PicFlags = " << +picParams->CurrReconstructedPic.PicFlags << std::endl;

    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "RefFrameList[" << +i << "].FrameIdx = " << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList[" << +i << "].PicFlags = " << +picParams->RefFrameList[i].PicFlags << std::endl;
        oss << "RefFrameList[" << +i << "].PicEntry = " << +picParams->RefFrameList[i].PicEntry << std::endl;
    }

    for (uint8_t i = 0; i < 7; i++)
    {
        oss << "ref_frame_idx[" << +i << "] = " << +picParams->ref_frame_idx[i] << std::endl;
    }

    oss << "HierarchLevelPlus1 = " << +picParams->HierarchLevelPlus1 << std::endl;
    oss << "primary_ref_frame = " << +picParams->primary_ref_frame << std::endl;
    oss << "ref_frame_ctrl_l0 = " << +picParams->ref_frame_ctrl_l0.RefFrameCtrl.value << std::endl;
    oss << "ref_frame_ctrl_l1 = " << +picParams->ref_frame_ctrl_l1.RefFrameCtrl.value << std::endl;
    oss << "order_hint = " << +picParams->order_hint << std::endl;
    oss << "frame_type = " << +picParams->PicFlags.fields.frame_type << std::endl;
    oss << "error_resilient_mode = " << +picParams->PicFlags.fields.error_resilient_mode << std::endl;
    oss << "disable_cdf_update = " << +picParams->PicFlags.fields.disable_cdf_update << std::endl;
    oss << "use_superres = " << +picParams->PicFlags.fields.use_superres << std::endl;
    oss << "allow_high_precision_mv = " << +picParams->PicFlags.fields.allow_high_precision_mv << std::endl;
    oss << "use_ref_frame_mvs = " << +picParams->PicFlags.fields.use_ref_frame_mvs << std::endl;
    oss << "disable_frame_end_update_cdf = " << +picParams->PicFlags.fields.disable_frame_end_update_cdf << std::endl;
    oss << "reduced_tx_set_used = " << +picParams->PicFlags.fields.reduced_tx_set_used << std::endl;
    oss << "SegIdBlockSize = " << +picParams->PicFlags.fields.SegIdBlockSize << std::endl;
    oss << "EnableFrameOBU = " << +picParams->PicFlags.fields.EnableFrameOBU << std::endl;
    oss << "DisableFrameRecon = " << +picParams->PicFlags.fields.DisableFrameRecon << std::endl;
    oss << "LongTermReference = " << +picParams->PicFlags.fields.LongTermReference << std::endl;
    oss << "allow_intrabc = " << +picParams->PicFlags.fields.allow_intrabc << std::endl;
    oss << "PaletteModeEnable = " << +picParams->PicFlags.fields.PaletteModeEnable << std::endl;
    oss << "Reserved2 = " << +picParams->PicFlags.fields.Reserved2 << std::endl;
    oss << "PicFlags.value = " << +picParams->PicFlags.value << std::endl;

    // deblocking filter
    for (uint8_t i = 0; i < 2; i++)
    {
        oss << "filter_level[" << +i << "] = " << +picParams->filter_level[i] << std::endl;
    }

    oss << "filter_level_u = " << +picParams->filter_level_u << std::endl;
    oss << "filter_level_v = " << +picParams->filter_level_v << std::endl;
    oss << "sharpness_level = " << +picParams->cLoopFilterInfoFlags.fields.sharpness_level << std::endl;
    oss << "mode_ref_delta_enabled = " << +picParams->cLoopFilterInfoFlags.fields.mode_ref_delta_enabled << std::endl;
    oss << "mode_ref_delta_update = " << +picParams->cLoopFilterInfoFlags.fields.mode_ref_delta_update << std::endl;
    oss << "Reserved3= " << +picParams->cLoopFilterInfoFlags.fields.Reserved3 << std::endl;
    oss << "cLoopFilterInfoFlags.value = " << +picParams->cLoopFilterInfoFlags.value << std::endl;
    oss << "superres_scale_denominator = " << +picParams->superres_scale_denominator << std::endl;
    oss << "interp_filter = " << +picParams->interp_filter << std::endl;
    oss << "Reserved4 = " << +picParams->Reserved4 << std::endl;

    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "ref_deltas[" << +i << "] = " << +picParams->ref_deltas[i] << std::endl;
    }

    for (uint8_t i = 0; i < 2; i++)
    {
        oss << "mode_deltas[" << +i << "] = " << +picParams->mode_deltas[i] << std::endl;
    }

    // quantization
    oss << "base_qindex = " << +picParams->base_qindex << std::endl;
    oss << "base_qindex = " << +picParams->base_qindex << std::endl;
    oss << "y_dc_delta_q = " << +picParams->y_dc_delta_q << std::endl;
    oss << "u_dc_delta_q = " << +picParams->u_dc_delta_q << std::endl;
    oss << "u_ac_delta_q = " << +picParams->u_ac_delta_q << std::endl;
    oss << "v_dc_delta_q = " << +picParams->v_dc_delta_q << std::endl;
    oss << "v_ac_delta_q = " << +picParams->v_ac_delta_q << std::endl;
    oss << "MinBaseQIndex = " << +picParams->MinBaseQIndex << std::endl;
    oss << "MaxBaseQIndex = " << +picParams->MaxBaseQIndex << std::endl;
    oss << "Reserved5 = " << +picParams->Reserved5 << std::endl;

    // quantization_matrix
    oss << "using_qmatrix = " << +picParams->wQMatrixFlags.fields.using_qmatrix << std::endl;
    oss << "qm_y = " << +picParams->wQMatrixFlags.fields.qm_y << std::endl;
    oss << "qm_u = " << +picParams->wQMatrixFlags.fields.qm_u << std::endl;
    oss << "qm_v = " << +picParams->wQMatrixFlags.fields.qm_v << std::endl;
    oss << "Reserved6 = " << +picParams->wQMatrixFlags.fields.Reserved6 << std::endl;
    oss << "wQMatrixFlags.value = " << +picParams->wQMatrixFlags.value << std::endl;
    oss << "Reserved7 = " << +picParams->Reserved7 << std::endl;

    // delta_q parameters
    oss << "dwModeControlFlags.fields.delta_q_present_flag = " << +picParams->dwModeControlFlags.fields.delta_q_present_flag << std::endl;
    oss << "dwModeControlFlags.fields.log2_delta_q_res = " << +picParams->dwModeControlFlags.fields.log2_delta_q_res << std::endl;

    // delta_lf parameters
    oss << "dwModeControlFlags.fields.delta_lf_present_flag = " << +picParams->dwModeControlFlags.fields.delta_lf_present_flag << std::endl;
    oss << "dwModeControlFlags.fields.log2_delta_lf_res = " << +picParams->dwModeControlFlags.fields.log2_delta_lf_res << std::endl;
    oss << "dwModeControlFlags.fields.delta_lf_multi = " << +picParams->dwModeControlFlags.fields.delta_lf_multi << std::endl;

    // read_tx_mode
    oss << "dwModeControlFlags.fields.tx_mode = " << +picParams->dwModeControlFlags.fields.tx_mode << std::endl;

    // read_frame_reference_mode
    oss << "dwModeControlFlags.fields.reference_mode = " << +picParams->dwModeControlFlags.fields.reference_mode << std::endl;
    oss << "dwModeControlFlags.fields.reduced_tx_set_used = " << +picParams->dwModeControlFlags.fields.reduced_tx_set_used << std::endl;

    oss << "dwModeControlFlags.fields.skip_mode_present = " << +picParams->dwModeControlFlags.fields.skip_mode_present << std::endl;
    oss << "dwModeControlFlags.fields.Reserved8 = " << +picParams->dwModeControlFlags.fields.Reserved8 << std::endl;
    oss << "dwModeControlFlags.value = " << +picParams->dwModeControlFlags.value << std::endl;

    oss << "stAV1Segments.SegmentFlags.fields.segmentation_enabled = " << +picParams->stAV1Segments.SegmentFlags.fields.segmentation_enabled << std::endl;
    oss << "stAV1Segments.SegmentFlags.fields.SegmentNumber = " << +picParams->stAV1Segments.SegmentFlags.fields.SegmentNumber << std::endl;
    oss << "stAV1Segments.SegmentFlags.fields.update_map = " << +picParams->stAV1Segments.SegmentFlags.fields.update_map << std::endl;
    oss << "stAV1Segments.SegmentFlags.fields.temporal_update = " << +picParams->stAV1Segments.SegmentFlags.fields.temporal_update << std::endl;
    oss << "stAV1Segments.SegmentFlags.fields.Reserved0 = " << +picParams->stAV1Segments.SegmentFlags.fields.Reserved0 << std::endl;
    oss << "stAV1Segments.SegmentFlags.value = " << +picParams->stAV1Segments.SegmentFlags.value << std::endl;

    for (uint8_t i = 0; i < 8; i++)
    {
        for (uint8_t j = 0; j < 8; j++)
        {
            oss << "stAV1Segments.feature_data[" << +i << "][" << +j << "] = " << +picParams->stAV1Segments.feature_data[i][j] << std::endl;
        }
    }

    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "stAV1Segments.feature_mask[" << +i << "] = " << +picParams->stAV1Segments.feature_mask[i] << std::endl;
    }

    for (uint8_t i = 0; i < 4; i++)
    {
        oss << "stAV1Segments.Reserved1[" << +i << "] = " << +picParams->stAV1Segments.Reserved1[i] << std::endl;
    }

    oss << "tile_cols = " << +picParams->tile_cols << std::endl;

    for (uint8_t i = 0; i < 63; i++)
    {
        oss << "width_in_sbs_minus_1[" << +i << "] = " << +picParams->width_in_sbs_minus_1[i] << std::endl;
    }

    oss << "tile_rows = " << +picParams->tile_rows << std::endl;

    for (uint8_t i = 0; i < 63; i++)
    {
        oss << "height_in_sbs_minus_1[" << +i << "] = " << +picParams->height_in_sbs_minus_1[i] << std::endl;
    }

    oss << "context_update_tile_id = " << +picParams->context_update_tile_id << std::endl;
    oss << "temporal_id = " << +picParams->temporal_id << std::endl;

    //CDEF
    oss << "cdef_damping_minus_3 = " << +picParams->cdef_damping_minus_3 << std::endl;
    oss << "cdef_bits = " << +picParams->cdef_bits << std::endl;

    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "cdef_y_strengths[" << +i << "] = " << +picParams->cdef_y_strengths[i] << std::endl;
    }

    for (uint8_t i = 0; i < 8; i++)
    {
        oss << "cdef_uv_strengths[" << +i << "] = " << +picParams->cdef_uv_strengths[i] << std::endl;
    }

    oss << "yframe_restoration_type = " << +picParams->LoopRestorationFlags.fields.yframe_restoration_type << std::endl;
    oss << "cbframe_restoration_type = " << +picParams->LoopRestorationFlags.fields.cbframe_restoration_type << std::endl;
    oss << "crframe_restoration_type = " << +picParams->LoopRestorationFlags.fields.crframe_restoration_type << std::endl;
    oss << "lr_unit_shift = " << +picParams->LoopRestorationFlags.fields.lr_unit_shift << std::endl;
    oss << "lr_uv_shift = " << +picParams->LoopRestorationFlags.fields.lr_uv_shift << std::endl;
    oss << "Reserved9 = " << +picParams->LoopRestorationFlags.fields.Reserved9 << std::endl;
    oss << "LoopRestorationFlags.value = " << +picParams->LoopRestorationFlags.value << std::endl;

    // global motion
    for (uint8_t i = 0; i < 7; i++)
    {
        oss << "wm[" << +i << "].wmtype = " << +picParams->wm[i].wmtype << std::endl;

        for (uint8_t j = 0; j < 8; j++)
        {
            oss << "wm[" << +i << "].wmmat[" << +j << "] = " << +picParams->wm[i].wmmat[j] << std::endl;
        }

        oss << "wm[" << +i << "].wmtype = " << +picParams->wm[i].invalid << std::endl;
    }

    oss << "QIndexBitOffset = " << +picParams->QIndexBitOffset << std::endl;
    oss << "SegmentationBitOffset = " << +picParams->SegmentationBitOffset << std::endl;
    oss << "LoopFilterParamsBitOffset = " << +picParams->LoopFilterParamsBitOffset << std::endl;
    oss << "CDEFParamsSizeInBits = " << +picParams->CDEFParamsSizeInBits << std::endl;
    oss << "obu_extension_flag = " << +picParams->TileGroupOBUHdrInfo.fields.obu_extension_flag << std::endl;
    oss << "obu_has_size_field = " << +picParams->TileGroupOBUHdrInfo.fields.obu_has_size_field << std::endl;
    oss << "temporal_id = " << +picParams->TileGroupOBUHdrInfo.fields.temporal_id << std::endl;
    oss << "spatial_id = " << +picParams->TileGroupOBUHdrInfo.fields.spatial_id << std::endl;
    oss << "CDEFParamsBitOffset = " << +picParams->CDEFParamsBitOffset << std::endl;
    oss << "reserved8bits0 = " << +picParams->reserved8bits0 << std::endl;
    oss << "FrameHdrOBUSizeInBits = " << +picParams->FrameHdrOBUSizeInBits << std::endl;
    oss << "FrameHdrOBUSizeByteOffset = " << +picParams->FrameHdrOBUSizeByteOffset << std::endl;
    oss << "StatusReportFeedbackNumber = " << +picParams->StatusReportFeedbackNumber << std::endl;

    // Skip Frames
    oss << "reserved8bs1 = " << +picParams->reserved8bs1 << std::endl;
    oss << "reserved8bs2 = " << +picParams->reserved8bs2 << std::endl;
    oss << "NumSkipFrames = " << +picParams->NumSkipFrames << std::endl;
    oss << "FrameSizeReducedInBytes = " << +picParams->FrameSizeReducedInBytes << std::endl;
    oss << "NumDirtyRects = " << +picParams->NumDirtyRects << std::endl;

    if ((picParams->NumDirtyRects > 0) && picParams->pDirtyRect)
    {
        auto pDR = picParams->pDirtyRect;
        for (uint16_t i = 0; i < picParams->NumDirtyRects; i++, pDR++)
        {
            oss << "pDirtyRect[" << +i << "].Top = " << +pDR->Top << std::endl;
            oss << "pDirtyRect[" << +i << "].Bottom = " << +pDR->Bottom << std::endl;
            oss << "pDirtyRect[" << +i << "].Left = " << +pDR->Left << std::endl;
            oss << "pDirtyRect[" << +i << "].Right = " << +pDR->Right << std::endl;
        }
    }

    oss << "NumMoveRects = " << +picParams->NumMoveRects << std::endl;

    if ((picParams->NumMoveRects > 0) && picParams->pMoveRect)
        for (uint16_t i = 0; i < picParams->NumMoveRects; i++)
        {
            oss << "pMoveRect[" << +i << "].SourcePointX = " << +picParams->pMoveRect->SourcePointX << std::endl;
            oss << "pMoveRect[" << +i << "].SourcePointY = " << +picParams->pMoveRect->SourcePointY << std::endl;
            oss << "pMoveRect[" << +i << "].DestRectTop = " << +picParams->pMoveRect->DestRectTop << std::endl;
            oss << "pMoveRect[" << +i << "].DestRectBottom = " << +picParams->pMoveRect->DestRectBottom << std::endl;
            oss << "pMoveRect[" << +i << "].DestRectLeft = " << +picParams->pMoveRect->DestRectLeft << std::endl;
            oss << "pMoveRect[" << +i << "].DestRectRight = " << +picParams->pMoveRect->DestRectRight << std::endl;
        }

    oss << "InputType = " << +picParams->InputType << std::endl;
    oss << "TargetFrameSize = " << +picParams->TargetFrameSize << std::endl;
    oss << "QpModulationStrength = " << +picParams->QpModulationStrength << std::endl;

    for (uint8_t i = 0; i < 16; i++)
    {
        oss << "Reserved10[" << +i << "] = " << +picParams->Reserved10[i] << std::endl;
    }

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

MOS_STATUS Av1Pipeline::DumpTileGroupParams(
    const CODEC_AV1_ENCODE_TILE_GROUP_PARAMS *tilegroupParams, uint32_t index)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(tilegroupParams);
    m_debugInterface->m_sliceId = index;

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "TileGroup_id = " << +index << std::endl;
    oss << "TileGroupStart = " << +tilegroupParams->TileGroupStart << std::endl;
    oss << "TileGroupEnd = " << +tilegroupParams->TileGroupEnd << std::endl;
    oss << "Reserved16b = " << +tilegroupParams->Reserved16b << std::endl;

    for (uint8_t j = 0; j < 9; j++)
    {
        oss << "Reserved32b[" << +j << "] = " << +tilegroupParams->Reserved32b[j] << std::endl;
    }

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
            ofs << "TileGroupParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace encode
