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
//! \file     codec_def_encode_hevc_trace.h
//!

#pragma once
#include <vector>
#include "codec_def_encode_hevc.h"
#include "mos_utilities.h"

#define NUM_ELEM_ARRAY(member) (sizeof(obj.member) / sizeof(obj.member[0]))
#define PUSH(member) data.push_back(obj.member)
#define PUSH64(member)                                            \
    {                                                             \
        auto p = reinterpret_cast<const uint32_t *>(&obj.member); \
        data.push_back(p[0]);                                     \
        data.push_back(p[1]);                                     \
    }
#define PUSH_ARRAY(member)                                 \
    for (size_t i0 = 0; i0 < NUM_ELEM_ARRAY(member); i0++) \
    {                                                      \
        PUSH(member[i0]);                                  \
    }
#define PUSH_ARRAY2(member)                                \
    for (size_t i1 = 0; i1 < NUM_ELEM_ARRAY(member); i1++) \
    {                                                      \
        PUSH_ARRAY(member[i1]);                            \
    }
#define PUSH_ARRAY3(member)                                \
    for (size_t i2 = 0; i2 < NUM_ELEM_ARRAY(member); i2++) \
    {                                                      \
        PUSH_ARRAY2(member[i2]);                           \
    }
#define PUSH_SEQ(member, num)        \
    for (size_t i = 0; i < num; i++) \
    {                                \
        PUSH(member[i]);             \
    }

class CodecDefEncodeHevcTrace
{
public:
    static void TraceDDI(const CODEC_HEVC_ENCODE_SEQUENCE_PARAMS &obj)
    {
        if (!TraceDDIEn())
        {
            return;
        }

        std::vector<uint32_t> data;
        data.reserve(sizeof(obj));

        PUSH(wFrameWidthInMinCbMinus1);
        PUSH(wFrameHeightInMinCbMinus1);
        PUSH(general_profile_idc);
        PUSH(Level);
        PUSH(general_tier_flag);
        PUSH(GopPicSize);
        PUSH(GopRefDist);
        PUSH(GopOptFlag);
        PUSH(TargetUsage);
        PUSH(RateControlMethod);
        PUSH(TargetBitRate);
        PUSH(MaxBitRate);
        PUSH(MinBitRate);
        PUSH(FrameRate.Numerator);
        PUSH(FrameRate.Denominator);
        PUSH(InitVBVBufferFullnessInBit);
        PUSH(VBVBufferSizeInBit);
        PUSH(bResetBRC);
        PUSH(GlobalSearch);
        PUSH(LocalSearch);
        PUSH(EarlySkip);
        PUSH(MBBRC);
        PUSH(ParallelBRC);
        PUSH(SliceSizeControl);
        PUSH(SourceFormat);
        PUSH(SourceBitDepth);
        PUSH(QpAdjustment);
        PUSH(ROIValueInDeltaQP);
        PUSH(BlockQPforNonRectROI);
        PUSH(EnableTileBasedEncode);
        PUSH(bAutoMaxPBFrameSizeForSceneChange);
        PUSH(EnableStreamingBufferLLC);
        PUSH(EnableStreamingBufferDDR);
        PUSH(LowDelayMode);
        PUSH(DisableHRDConformance);
        PUSH(HierarchicalFlag);
        PUSH(TCBRCEnable);
        PUSH(bLookAheadPhase);
        PUSH(UserMaxIFrameSize);
        PUSH(UserMaxPBFrameSize);
        PUSH(ICQQualityFactor);
        PUSH(StreamBufferSessionID);
        PUSH(maxAdaptiveMiniGopSize);
        PUSH(GopFlags.fields.ClosedGop);
        PUSH(GopFlags.fields.StrictGop);
        PUSH(GopFlags.fields.AdaptiveGop);
        PUSH_ARRAY(NumOfBInGop);
        PUSH(scaling_list_enable_flag);
        PUSH(sps_temporal_mvp_enable_flag);
        PUSH(strong_intra_smoothing_enable_flag);
        PUSH(amp_enabled_flag);
        PUSH(SAO_enabled_flag);
        PUSH(pcm_enabled_flag);
        PUSH(pcm_loop_filter_disable_flag);
        PUSH(chroma_format_idc);
        PUSH(separate_colour_plane_flag);
        PUSH(palette_mode_enabled_flag);
        PUSH(RGBEncodingEnable);
        PUSH(PrimaryChannelForRGBEncoding);
        PUSH(SecondaryChannelForRGBEncoding);
        PUSH(log2_max_coding_block_size_minus3);
        PUSH(log2_min_coding_block_size_minus3);
        PUSH(log2_max_transform_block_size_minus2);
        PUSH(log2_min_transform_block_size_minus2);
        PUSH(max_transform_hierarchy_depth_intra);
        PUSH(max_transform_hierarchy_depth_inter);
        PUSH(log2_min_PCM_cb_size_minus3);
        PUSH(log2_max_PCM_cb_size_minus3);
        PUSH(bit_depth_luma_minus8);
        PUSH(bit_depth_chroma_minus8);
        PUSH(pcm_sample_bit_depth_luma_minus1);
        PUSH(pcm_sample_bit_depth_chroma_minus1);
        PUSH(InputColorSpace);  // enum ENCODE_INPUT_COLORSPACE
        PUSH(ScenarioInfo);  // enum ENCODE_SCENARIO
        PUSH(contentInfo);      // enum ENCODE_CONTENT
        PUSH(FrameSizeTolerance);  // enum ENCODE_FRAMESIZE_TOLERANCE
        PUSH(SlidingWindowSize);
        PUSH(MaxBitRatePerSlidingWindow);
        PUSH(MinBitRatePerSlidingWindow);
        PUSH(LookaheadDepth);
        PUSH(MinAdaptiveGopPicSize);
        PUSH(MaxAdaptiveGopPicSize);
        PUSH(FullPassCodecType);

        MOS_TraceEventExt(
            EVENT_ENCODE_DDI_SEQ_PARAM_HEVC,
            EVENT_TYPE_INFO,
            data.data(),
            data.size() * sizeof(decltype(data)::value_type));
    }

    static void TraceDDI(const CODEC_HEVC_ENCODE_PICTURE_PARAMS &obj)
    {
        if (!TraceDDIEn())
        {
            return;
        }

        std::vector<uint32_t> data;
        data.reserve(sizeof(obj));

        PUSH(CurrOriginalPic.PicEntry);
        PUSH(CurrReconstructedPic.PicEntry);
        PUSH(CollocatedRefPicIndex);
        for (size_t i = 0; i < NUM_ELEM_ARRAY(RefFrameList); i++)
        {
            PUSH(RefFrameList[i].PicEntry);
        }
        PUSH(CurrPicOrderCnt);  // int32_t
        PUSH_ARRAY(RefFramePOCList);  // int32_t
        PUSH(CodingType);
        PUSH(HierarchLevelPlus1);
        PUSH(NumSlices);
        PUSH(tiles_enabled_flag);
        PUSH(entropy_coding_sync_enabled_flag);
        PUSH(sign_data_hiding_flag);
        PUSH(constrained_intra_pred_flag);
        PUSH(transform_skip_enabled_flag);
        PUSH(transquant_bypass_enabled_flag);
        PUSH(cu_qp_delta_enabled_flag);
        PUSH(weighted_pred_flag);
        PUSH(weighted_bipred_flag);
        PUSH(loop_filter_across_slices_flag);
        PUSH(loop_filter_across_tiles_flag);
        PUSH(scaling_list_data_present_flag);
        PUSH(dependent_slice_segments_enabled_flag);
        PUSH(bLastPicInSeq);
        PUSH(bLastPicInStream);
        PUSH(bUseRawPicForRef);
        PUSH(bEmulationByteInsertion);
        PUSH(BRCPrecision);
        PUSH(bEnableSliceLevelReport);
        PUSH(bEnableRollingIntraRefresh);
        PUSH(no_output_of_prior_pics_flag);
        PUSH(bEnableGPUWeightedPrediction);
        PUSH(bDisplayFormatSwizzle);
        PUSH(deblocking_filter_override_enabled_flag);
        PUSH(pps_deblocking_filter_disabled_flag);
        PUSH(bEnableCTULevelReport);
        PUSH(bEnablePartialFrameUpdate);
        PUSH(QpY);  // int8_t
        PUSH(diff_cu_qp_delta_depth);
        PUSH(pps_cb_qp_offset);  // int8_t
        PUSH(pps_cr_qp_offset);  // int8_t
        PUSH(num_tile_columns_minus1);
        PUSH(num_tile_rows_minus1);
        PUSH_ARRAY(tile_column_width);
        PUSH_ARRAY(tile_row_height);
        PUSH(log2_parallel_merge_level_minus2);
        PUSH(num_ref_idx_l0_default_active_minus1);
        PUSH(num_ref_idx_l1_default_active_minus1);
        PUSH(LcuMaxBitsizeAllowed);
        PUSH(IntraInsertionLocation);
        PUSH(IntraInsertionSize);
        PUSH(QpDeltaForInsertedIntra);  // int8_t
        PUSH(StatusReportFeedbackNumber);
        PUSH(slice_pic_parameter_set_id);
        PUSH(nal_unit_type);
        PUSH(MaxSliceSizeInBytes);
        PUSH(NumROI);
        for (size_t i = 0; i < NUM_ELEM_ARRAY(ROI); i++)
        {
            PUSH(ROI[i].PriorityLevelOrDQp);  // int8_t
            PUSH(ROI[i].Top);
            PUSH(ROI[i].Bottom);
            PUSH(ROI[i].Left);
            PUSH(ROI[i].Right);
        }
        PUSH(MaxDeltaQp);  // int8_t
        PUSH(MinDeltaQp);  // int8_t
        PUSH(CustomRoundingOffsetsParams.fields.EnableCustomRoudingIntra);
        PUSH(CustomRoundingOffsetsParams.fields.RoundingOffsetIntra);
        PUSH(CustomRoundingOffsetsParams.fields.EnableCustomRoudingInter);
        PUSH(CustomRoundingOffsetsParams.fields.RoundingOffsetInter);
        PUSH(SkipFrameFlag);
        PUSH(NumSkipFrames);
        PUSH(SizeSkipFrames);
        PUSH(BRCMaxQp);
        PUSH(BRCMinQp);
        PUSH(bEnableHMEOffset);
        PUSH_ARRAY2(HMEOffset);  // int16_t
        PUSH(NumDirtyRects);
        PUSH64(pDirtyRect);
        if (obj.pDirtyRect)
        {
            for (size_t i = 0; i < obj.NumDirtyRects; i++)
            {
                PUSH(pDirtyRect[i].Top);
                PUSH(pDirtyRect[i].Bottom);
                PUSH(pDirtyRect[i].Left);
                PUSH(pDirtyRect[i].Right);
            }
        }
        PUSH(NumMoveRects);
        PUSH(LcuMaxBitsizeAllowedHigh16b);
        PUSH(DownScaleRatio.fields.X16Minus1_X);
        PUSH(DownScaleRatio.fields.X16Minus1_Y);
        PUSH(QpModulationStrength);
        PUSH(TargetFrameSize);
        PUSH(StatusReportEnable.fields.FrameStats);
        PUSH(StatusReportEnable.fields.BlockStats);

        MOS_TraceEventExt(
            EVENT_ENCODE_DDI_PIC_PARAM_HEVC,
            EVENT_TYPE_INFO,
            data.data(),
            data.size() * sizeof(decltype(data)::value_type));
    }

    static void TraceDDI(const CODEC_HEVC_ENCODE_SLICE_PARAMS &obj)
    {
        if (!TraceDDIEn())
        {
            return;
        }

        std::vector<uint32_t> data;
        data.reserve(sizeof(obj));

        PUSH(slice_segment_address);
        PUSH(NumLCUsInSlice);
        for (size_t i = 0; i < NUM_ELEM_ARRAY(RefPicList); i++)
        {
            for (size_t j = 0; j < NUM_ELEM_ARRAY(RefPicList[0]); j++)
            {
                PUSH(RefPicList[i][j].PicEntry);
            }
        }
        PUSH(num_ref_idx_l0_active_minus1);
        PUSH(num_ref_idx_l1_active_minus1);
        PUSH(bLastSliceOfPic);
        PUSH(dependent_slice_segment_flag);
        PUSH(slice_temporal_mvp_enable_flag);
        PUSH(slice_type);
        PUSH(slice_sao_luma_flag);
        PUSH(slice_sao_chroma_flag);
        PUSH(mvd_l1_zero_flag);
        PUSH(cabac_init_flag);
        PUSH(slice_deblocking_filter_disable_flag);
        PUSH(collocated_from_l0_flag);
        PUSH(slice_qp_delta);  // int8_t
        PUSH(slice_cb_qp_offset);  // int8_t
        PUSH(slice_cr_qp_offset);  // int8_t
        PUSH(beta_offset_div2);  // int8_t
        PUSH(tc_offset_div2);  // int8_t
        PUSH(luma_log2_weight_denom);
        PUSH(delta_chroma_log2_weight_denom);  // int8_t
        PUSH_ARRAY2(luma_offset);  // int8_t
        PUSH_ARRAY2(delta_luma_weight);  // int8_t
        PUSH_ARRAY3(chroma_offset);  // int8_t
        PUSH_ARRAY3(delta_chroma_weight);  // int8_t
        PUSH(MaxNumMergeCand);
        PUSH(slice_id);
        PUSH(BitLengthSliceHeaderStartingPortion);
        PUSH(SliceHeaderByteOffset);
        PUSH(SliceQpDeltaBitOffset);
        PUSH(PredWeightTableBitOffset);
        PUSH(PredWeightTableBitLength);
        PUSH(SliceSAOFlagBitOffset);

        MOS_TraceEventExt(
            EVENT_ENCODE_DDI_SLC_PARAM_HEVC,
            EVENT_TYPE_INFO,
            data.data(),
            data.size() * sizeof(decltype(data)::value_type));
    }

private:
    static bool TraceDDIEn()
    {
        return MosUtilities::TraceKeyEnabled(TR_KEY_ENCODE_EVENT_DDI) &&
               MosUtilities::TracelevelEnabled(MT_EVENT_LEVEL::ALWAYS);
    }
};

#undef NUM_ELEM_ARRAY
#undef PUSH
#undef PUSH64
#undef PUSH_ARRAY
#undef PUSH_ARRAY2
#undef PUSH_ARRAY3
#undef PUSH_SEQ
