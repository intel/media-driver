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
//! \file     capstable_data_hevc_encode_xe_lpm_plus_r0_specific.h
//! \brief    This file register all caps data
//!

#ifndef __CAPSTABLE_DATA_HEVC_ENCODE_XE_LPM_PLUS_R0_SPECIFIC_H__
#define __CAPSTABLE_DATA_HEVC_ENCODE_XE_LPM_PLUS_R0_SPECIFIC_H__

#include "capstable_data_xe_lpm_plus_r0_specific.h"
#include "codec_def_encode_hevc.h"
#include "codec_def_encode_jpeg.h"
#include "codec_def_common_jpeg.h"

//!
//! \brief  Definion for bitset value
//!
static const VAConfigAttribValEncROI HEVCCommon_VAEntrypointEncSlice_encROI
{
    {CODECHAL_ENCODE_HEVC_MAX_NUM_ROI,0,1,0}
};

//! \brief  Definition for ConfigDataList
static ConfigDataList configDataList_VAProfileHEVCMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VA_RC_CQP, 0},
  {VA_RC_CBR, 0},
  {VA_RC_VBR, 0},
  {VA_RC_CBR | VA_RC_MB, 0},
  {VA_RC_VBR | VA_RC_MB, 0},
  {VA_RC_ICQ, 0},
  {VA_RC_VCM, 0},
  {VA_RC_QVBR, 0},
  {VA_RC_AVBR, 0},
#if VA_CHECK_VERSION(1, 10, 0)
  {VA_RC_TCBRC, 0},
#endif
  {VA_RC_CBR | VA_RC_PARALLEL | VA_RC_MB, 0},
  {VA_RC_VBR | VA_RC_PARALLEL | VA_RC_MB, 0},
  {VA_RC_ICQ | VA_RC_PARALLEL, 0},
  {VA_RC_VCM | VA_RC_PARALLEL, 0},
  {VA_RC_QVBR | VA_RC_PARALLEL, 0},
  {VA_RC_AVBR  | VA_RC_PARALLEL, 0},
#if VA_CHECK_VERSION(1, 10, 0)
  {VA_RC_TCBRC | VA_RC_PARALLEL, 0},
#endif
};

static ConfigDataList configDataList_VAProfileHEVCMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VA_RC_CQP, 0},
  {VA_RC_CBR, 0},
  {VA_RC_VBR, 0},
  {VA_RC_CBR | VA_RC_MB, 0},
  {VA_RC_VBR | VA_RC_MB, 0},
  {VA_RC_ICQ, 0},
  {VA_RC_VCM, 0},
  {VA_RC_QVBR, 0},
  {VA_RC_AVBR, 0},
#if VA_CHECK_VERSION(1, 10, 0)
  {VA_RC_TCBRC, 0},
#endif
  {VA_RC_CBR | VA_RC_PARALLEL | VA_RC_MB, 0},
  {VA_RC_VBR | VA_RC_PARALLEL | VA_RC_MB, 0},
  {VA_RC_ICQ | VA_RC_PARALLEL, 0},
  {VA_RC_VCM | VA_RC_PARALLEL, 0},
  {VA_RC_QVBR | VA_RC_PARALLEL, 0},
  {VA_RC_AVBR  | VA_RC_PARALLEL, 0},
#if VA_CHECK_VERSION(1, 10, 0)
  {VA_RC_TCBRC | VA_RC_PARALLEL, 0},
#endif
  {VA_RC_VBR | VA_RC_MB, 0}
};

static ConfigDataList configDataList_VAProfileHEVCMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VA_RC_CQP, 0},
  {VA_RC_CBR, 0},
  {VA_RC_VBR, 0},
  {VA_RC_CBR | VA_RC_MB, 0},
  {VA_RC_VBR | VA_RC_MB, 0},
  {VA_RC_ICQ, 0},
  {VA_RC_VCM, 0},
  {VA_RC_QVBR, 0},
  {VA_RC_AVBR, 0},
#if VA_CHECK_VERSION(1, 10, 0)
  {VA_RC_TCBRC, 0},
#endif
  {VA_RC_CBR | VA_RC_PARALLEL | VA_RC_MB, 0},
  {VA_RC_VBR | VA_RC_PARALLEL | VA_RC_MB, 0},
  {VA_RC_ICQ | VA_RC_PARALLEL, 0},
  {VA_RC_VCM | VA_RC_PARALLEL, 0},
  {VA_RC_QVBR | VA_RC_PARALLEL, 0},
  {VA_RC_AVBR  | VA_RC_PARALLEL, 0},
#if VA_CHECK_VERSION(1, 10, 0)
  {VA_RC_TCBRC | VA_RC_PARALLEL, 0},
#endif
};

static ConfigDataList configDataList_VAProfileHEVCMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VA_RC_CQP, 0},
  {VA_RC_CBR, 0},
  {VA_RC_VBR, 0},
  {VA_RC_CBR | VA_RC_MB, 0},
  {VA_RC_VBR | VA_RC_MB, 0},
  {VA_RC_ICQ, 0},
  {VA_RC_VCM, 0},
  {VA_RC_QVBR, 0},
  {VA_RC_AVBR, 0},
#if VA_CHECK_VERSION(1, 10, 0)
  {VA_RC_TCBRC, 0},
#endif
  {VA_RC_CBR | VA_RC_PARALLEL | VA_RC_MB, 0},
  {VA_RC_VBR | VA_RC_PARALLEL | VA_RC_MB, 0},
  {VA_RC_ICQ | VA_RC_PARALLEL, 0},
  {VA_RC_VCM | VA_RC_PARALLEL, 0},
  {VA_RC_QVBR | VA_RC_PARALLEL, 0},
  {VA_RC_AVBR  | VA_RC_PARALLEL, 0},
#if VA_CHECK_VERSION(1, 10, 0)
  {VA_RC_TCBRC | VA_RC_PARALLEL, 0},
#endif
};

static ConfigDataList configDataList_VAProfileHEVCSccMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VA_RC_CQP, 0},
};

static ConfigDataList configDataList_VAProfileHEVCSccMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VA_RC_CQP, 0},
};

static ConfigDataList configDataList_VAProfileHEVCSccMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VA_RC_CQP, 0},
};

static ConfigDataList configDataList_VAProfileHEVCSccMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VA_RC_CQP, 0},
};

#if VA_CHECK_VERSION(1, 12, 0)
static VAConfigAttribValEncHEVCFeatures vaConfigAttribValEncHEVCFeatures =
{
  {
    .separate_colour_planes = VA_FEATURE_NOT_SUPPORTED,
    .scaling_lists = VA_FEATURE_SUPPORTED,
    .amp = VA_FEATURE_REQUIRED,
    .sao = VA_FEATURE_SUPPORTED,
    .pcm = VA_FEATURE_NOT_SUPPORTED,
    .temporal_mvp = VA_FEATURE_SUPPORTED,
    .strong_intra_smoothing = VA_FEATURE_NOT_SUPPORTED,
    .dependent_slices = VA_FEATURE_NOT_SUPPORTED,
    .sign_data_hiding = VA_FEATURE_NOT_SUPPORTED,
    .constrained_intra_pred = VA_FEATURE_NOT_SUPPORTED,
    .transform_skip = VA_FEATURE_SUPPORTED,
    .cu_qp_delta = VA_FEATURE_REQUIRED,
    .weighted_prediction = VA_FEATURE_SUPPORTED,
    .transquant_bypass = VA_FEATURE_NOT_SUPPORTED,
    .deblocking_filter_disable = VA_FEATURE_NOT_SUPPORTED
  }
};

static VAConfigAttribValEncHEVCBlockSizes vaConfigAttribValEncHEVCBlockSizes =
{
  {
    .log2_max_coding_tree_block_size_minus3     = 3,
    .log2_min_coding_tree_block_size_minus3     = 3,
    .log2_min_luma_coding_block_size_minus3     = 0,
    .log2_max_luma_transform_block_size_minus2  = 3,
    .log2_min_luma_transform_block_size_minus2  = 0,
    .max_max_transform_hierarchy_depth_inter    = 2,
    .min_max_transform_hierarchy_depth_inter    = 0,
    .max_max_transform_hierarchy_depth_intra    = 2,
    .min_max_transform_hierarchy_depth_intra    = 0,
    .log2_max_pcm_coding_block_size_minus3      = 0,
    .log2_min_pcm_coding_block_size_minus3      = 0
  }
};
#endif

//!
//! \brief  Definion for AttribList
//!
static const AttribList attribList_VAProfileHEVCMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_VCM | VA_RC_TCBRC | VA_RC_ICQ},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
   {VAConfigAttribEncInterlaced, 0},
   {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES_LDB |(DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_LDB << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
   {VAConfigAttribEncMaxSlices, ENCODE_HEVC_VDENC_NUM_MAX_SLICES},
   // only for matching with legacy
   {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS},
   // Correct value
   // {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_12K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
   {VAConfigAttribEncROI, HEVCCommon_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncDirtyRect, 16},
   {VAConfigAttribEncTileSupport, 1},
   // Correct values
   // {VAConfigAttribPredictionDirection, VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE},
   {VAConfigAttribContextPriority, 1024},
   // The following items are only for matching with legacy.
   {VAConfigAttribEncJPEG, ((JPEG_MAX_QUANT_TABLE << 14) | /* max_num_quantization_tables : 3 */
                            (JPEG_MAX_NUM_HUFF_TABLE_INDEX << 11) | /* max_num_huffman_tables : 3 */
                            (1 << 7) | /* max_num_scans : 4 */
                            (jpegNumComponent << 4)) /* max_num_components : 3 */},
   {VAConfigAttribEncQuantization, 0},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribFEIMVPredictors, 4},
   {VAConfigAttribCustomRoundingControl, 1},
   {VAConfigAttribMaxFrameSize, 3},
   {VAConfigAttribPredictionDirection, 7},
#if VA_CHECK_VERSION(1, 12, 0)
   {VAConfigAttribEncHEVCFeatures, vaConfigAttribValEncHEVCFeatures.value},
   {VAConfigAttribEncHEVCBlockSizes, vaConfigAttribValEncHEVCBlockSizes.value},
#endif
};

static const AttribList attribList_VAProfileHEVCMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_VCM | VA_RC_TCBRC | VA_RC_ICQ},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
   {VAConfigAttribEncInterlaced, 0},
   {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES_LDB |(DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_LDB << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
   {VAConfigAttribEncMaxSlices, ENCODE_HEVC_VDENC_NUM_MAX_SLICES},
   // only for matching with legacy
   {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS},
   // Correct value
   // {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_12K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
   {VAConfigAttribEncROI, HEVCCommon_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncDirtyRect, 16},
   {VAConfigAttribEncTileSupport, 1},
   // Correct values
   // {VAConfigAttribPredictionDirection, VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE},
   {VAConfigAttribContextPriority, 1024},
   // The following items are only for matching with legacy.
   {VAConfigAttribEncJPEG, ((JPEG_MAX_QUANT_TABLE << 14) | /* max_num_quantization_tables : 3 */
                            (JPEG_MAX_NUM_HUFF_TABLE_INDEX << 11) | /* max_num_huffman_tables : 3 */
                            (1 << 7) | /* max_num_scans : 4 */
                            (jpegNumComponent << 4)) /* max_num_components : 3 */},
   {VAConfigAttribEncQuantization, 0},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribFEIMVPredictors, 4},
   {VAConfigAttribCustomRoundingControl, 1},
   {VAConfigAttribMaxFrameSize, 3},
   {VAConfigAttribPredictionDirection, 7},
#if VA_CHECK_VERSION(1, 12, 0)
   {VAConfigAttribEncHEVCFeatures, vaConfigAttribValEncHEVCFeatures.value},
   {VAConfigAttribEncHEVCBlockSizes, vaConfigAttribValEncHEVCBlockSizes.value},
#endif
};

static const AttribList attribList_VAProfileHEVCMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_VCM | VA_RC_TCBRC | VA_RC_ICQ},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
   {VAConfigAttribEncInterlaced, 0},
   {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES_LDB |(DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_LDB << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
   {VAConfigAttribEncMaxSlices, ENCODE_HEVC_VDENC_NUM_MAX_SLICES},
   // only for matching with legacy
   {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS},
   // Correct value
   // {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_12K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
   {VAConfigAttribEncROI, HEVCCommon_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncDirtyRect, 16},
   {VAConfigAttribEncTileSupport, 1},
   // Correct values
   // {VAConfigAttribPredictionDirection, VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE},
   {VAConfigAttribContextPriority, 1024},
   // The following items are only for matching with legacy.
   {VAConfigAttribEncJPEG, ((JPEG_MAX_QUANT_TABLE << 14) | /* max_num_quantization_tables : 3 */
                            (JPEG_MAX_NUM_HUFF_TABLE_INDEX << 11) | /* max_num_huffman_tables : 3 */
                            (1 << 7) | /* max_num_scans : 4 */
                            (jpegNumComponent << 4)) /* max_num_components : 3 */},
   {VAConfigAttribEncQuantization, 0},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribFEIMVPredictors, 4},
   {VAConfigAttribCustomRoundingControl, 1},
   {VAConfigAttribMaxFrameSize, 3},
   {VAConfigAttribPredictionDirection, 7},
#if VA_CHECK_VERSION(1, 12, 0)
   {VAConfigAttribEncHEVCFeatures, vaConfigAttribValEncHEVCFeatures.value},
   {VAConfigAttribEncHEVCBlockSizes, vaConfigAttribValEncHEVCBlockSizes.value},
#endif
};

static const AttribList attribList_VAProfileHEVCMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_VCM | VA_RC_TCBRC | VA_RC_ICQ},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
   {VAConfigAttribEncInterlaced, 0},
   {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES_LDB |(DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_LDB << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
   {VAConfigAttribEncMaxSlices, ENCODE_HEVC_VDENC_NUM_MAX_SLICES},
   // only for matching with legacy
   {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS},
   // Correct value
   // {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_12K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
   {VAConfigAttribEncROI, HEVCCommon_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncDirtyRect, 16},
   {VAConfigAttribEncTileSupport, 1},
   // Correct values
   // {VAConfigAttribPredictionDirection, VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE},
   {VAConfigAttribContextPriority, 1024},
   // The following items are only for matching with legacy.
   {VAConfigAttribEncJPEG, ((JPEG_MAX_QUANT_TABLE << 14) | /* max_num_quantization_tables : 3 */
                            (JPEG_MAX_NUM_HUFF_TABLE_INDEX << 11) | /* max_num_huffman_tables : 3 */
                            (1 << 7) | /* max_num_scans : 4 */
                            (jpegNumComponent << 4)) /* max_num_components : 3 */},
   {VAConfigAttribEncQuantization, 0},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribFEIMVPredictors, 4},
   {VAConfigAttribCustomRoundingControl, 1},
   {VAConfigAttribMaxFrameSize, 3},
   {VAConfigAttribPredictionDirection, 7},
#if VA_CHECK_VERSION(1, 12, 0)
   {VAConfigAttribEncHEVCFeatures, vaConfigAttribValEncHEVCFeatures.value},
   {VAConfigAttribEncHEVCBlockSizes, vaConfigAttribValEncHEVCBlockSizes.value},
#endif
};

static const AttribList attribList_VAProfileHEVCSccMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
   // Correct values
   // {VAConfigAttribRateControl, VA_RC_CQP},
   // Only for matching with legacy
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_VCM | VA_RC_TCBRC | VA_RC_ICQ},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
   {VAConfigAttribEncInterlaced, 0},
   {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES_LDB |(DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_LDB << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
   {VAConfigAttribEncMaxSlices, ENCODE_HEVC_VDENC_NUM_MAX_SLICES},
   // only for matching with legacy
   {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS},
   // Correct value
   // {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_12K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
   {VAConfigAttribEncROI, HEVCCommon_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncDirtyRect, 16},
   {VAConfigAttribEncTileSupport, 1},
   // Correct values
   // {VAConfigAttribPredictionDirection, VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE},
   {VAConfigAttribContextPriority, 1024},
   // The following items are only for matching with legacy.
   {VAConfigAttribEncJPEG, ((JPEG_MAX_QUANT_TABLE << 14) | /* max_num_quantization_tables : 3 */
                            (JPEG_MAX_NUM_HUFF_TABLE_INDEX << 11) | /* max_num_huffman_tables : 3 */
                            (1 << 7) | /* max_num_scans : 4 */
                            (jpegNumComponent << 4)) /* max_num_components : 3 */},
   {VAConfigAttribEncQuantization, 0},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribFEIMVPredictors, 4},
   {VAConfigAttribCustomRoundingControl, 1},
   {VAConfigAttribMaxFrameSize, 3},
   {VAConfigAttribPredictionDirection, 7},
#if VA_CHECK_VERSION(1, 12, 0)
   {VAConfigAttribEncHEVCFeatures, vaConfigAttribValEncHEVCFeatures.value},
   {VAConfigAttribEncHEVCBlockSizes, vaConfigAttribValEncHEVCBlockSizes.value},
#endif
};

static const AttribList attribList_VAProfileHEVCSccMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
   // Correct values
   // {VAConfigAttribRateControl, VA_RC_CQP},
   // Only for matching with legacy
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_VCM | VA_RC_TCBRC | VA_RC_ICQ},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
   {VAConfigAttribEncInterlaced, 0},
   {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES_LDB |(DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_LDB << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
   {VAConfigAttribEncMaxSlices, ENCODE_HEVC_VDENC_NUM_MAX_SLICES},
   // only for matching with legacy
   {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS},
   // Correct value
   // {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_12K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
   {VAConfigAttribEncROI, HEVCCommon_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncDirtyRect, 16},
   {VAConfigAttribEncTileSupport, 1},
   // Correct values
   // {VAConfigAttribPredictionDirection, VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE},
   {VAConfigAttribContextPriority, 1024},
   // The following items are only for matching with legacy.
   {VAConfigAttribEncJPEG, ((JPEG_MAX_QUANT_TABLE << 14) | /* max_num_quantization_tables : 3 */
                            (JPEG_MAX_NUM_HUFF_TABLE_INDEX << 11) | /* max_num_huffman_tables : 3 */
                            (1 << 7) | /* max_num_scans : 4 */
                            (jpegNumComponent << 4)) /* max_num_components : 3 */},
   {VAConfigAttribEncQuantization, 0},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribFEIMVPredictors, 4},
   {VAConfigAttribCustomRoundingControl, 1},
   {VAConfigAttribMaxFrameSize, 3},
   {VAConfigAttribPredictionDirection, 7},
#if VA_CHECK_VERSION(1, 12, 0)
   {VAConfigAttribEncHEVCFeatures, vaConfigAttribValEncHEVCFeatures.value},
   {VAConfigAttribEncHEVCBlockSizes, vaConfigAttribValEncHEVCBlockSizes.value},
#endif
};

static const AttribList attribList_VAProfileHEVCSccMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
   // Correct values
   // {VAConfigAttribRateControl, VA_RC_CQP},
   // Only for matching with legacy
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_VCM | VA_RC_TCBRC | VA_RC_ICQ},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
   {VAConfigAttribEncInterlaced, 0},
   {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES_LDB |(DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_LDB << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
   {VAConfigAttribEncMaxSlices, ENCODE_HEVC_VDENC_NUM_MAX_SLICES},
   // only for matching with legacy
   {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS},
   // Correct value
   // {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_12K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
   {VAConfigAttribEncROI, HEVCCommon_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncDirtyRect, 16},
   {VAConfigAttribEncTileSupport, 1},
   // Correct values
   // {VAConfigAttribPredictionDirection, VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE},
   {VAConfigAttribContextPriority, 1024},
   // The following items are only for matching with legacy.
   {VAConfigAttribEncJPEG, ((JPEG_MAX_QUANT_TABLE << 14) | /* max_num_quantization_tables : 3 */
                            (JPEG_MAX_NUM_HUFF_TABLE_INDEX << 11) | /* max_num_huffman_tables : 3 */
                            (1 << 7) | /* max_num_scans : 4 */
                            (jpegNumComponent << 4)) /* max_num_components : 3 */},
   {VAConfigAttribEncQuantization, 0},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribFEIMVPredictors, 4},
   {VAConfigAttribCustomRoundingControl, 1},
   {VAConfigAttribMaxFrameSize, 3},
   {VAConfigAttribPredictionDirection, 7},
#if VA_CHECK_VERSION(1, 12, 0)
   {VAConfigAttribEncHEVCFeatures, vaConfigAttribValEncHEVCFeatures.value},
   {VAConfigAttribEncHEVCBlockSizes, vaConfigAttribValEncHEVCBlockSizes.value},
#endif
};

static const AttribList attribList_VAProfileHEVCSccMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0
{
   {VAConfigAttribRTFormat, VA_RT_FORMAT_YUV420 | VA_RT_FORMAT_YUV420_10BPP | VA_RT_FORMAT_YUV444 | VA_RT_FORMAT_YUV444_10 | VA_RT_FORMAT_RGB32 | VA_RT_FORMAT_RGB32_10BPP},
   // Correct values
   // {VAConfigAttribRateControl, VA_RC_CQP},
   // Only for matching with legacy
   {VAConfigAttribRateControl, VA_RC_CQP | VA_RC_CBR | VA_RC_VBR | VA_RC_QVBR | VA_RC_MB | VA_RC_VCM | VA_RC_TCBRC | VA_RC_ICQ},
   {VAConfigAttribEncParallelRateControl, 0},
   {VAConfigAttribEncPackedHeaders, VA_ENC_PACKED_HEADER_PICTURE | VA_ENC_PACKED_HEADER_SEQUENCE | VA_ENC_PACKED_HEADER_SLICE | VA_ENC_PACKED_HEADER_RAW_DATA | VA_ENC_PACKED_HEADER_MISC},
   {VAConfigAttribEncInterlaced, 0},
   {VAConfigAttribEncMaxRefFrames, DDI_CODEC_VDENC_MAX_L0_REF_FRAMES_LDB |(DDI_CODEC_VDENC_MAX_L1_REF_FRAMES_LDB << DDI_CODEC_LEFT_SHIFT_FOR_REFLIST1)},
   {VAConfigAttribEncMaxSlices, ENCODE_HEVC_VDENC_NUM_MAX_SLICES},
   // only for matching with legacy
   {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE | VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS | VA_ENC_SLICE_STRUCTURE_EQUAL_MULTI_ROWS},
   // Correct value
   // {VAConfigAttribEncSliceStructure, VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS | VA_ENC_SLICE_STRUCTURE_MAX_SLICE_SIZE},
   {VAConfigAttribMaxPictureWidth, CODEC_16K_MAX_PIC_WIDTH},
   {VAConfigAttribMaxPictureHeight, CODEC_12K_MAX_PIC_HEIGHT},
   {VAConfigAttribEncQualityRange, NUM_TARGET_USAGE_MODES - 1},
   {VAConfigAttribEncIntraRefresh, VA_ENC_INTRA_REFRESH_ROLLING_COLUMN | VA_ENC_INTRA_REFRESH_ROLLING_ROW},
   {VAConfigAttribEncROI, HEVCCommon_VAEntrypointEncSlice_encROI.value},
   {VAConfigAttribProcessingRate, VA_PROCESSING_RATE_ENCODE},
   {VAConfigAttribEncDirtyRect, 16},
   {VAConfigAttribEncTileSupport, 1},
   // Correct values
   // {VAConfigAttribPredictionDirection, VA_PREDICTION_DIRECTION_PREVIOUS | VA_PREDICTION_DIRECTION_FUTURE},
   {VAConfigAttribContextPriority, 1024},
   // The following items are only for matching with legacy.
   {VAConfigAttribEncJPEG, ((JPEG_MAX_QUANT_TABLE << 14) | /* max_num_quantization_tables : 3 */
                            (JPEG_MAX_NUM_HUFF_TABLE_INDEX << 11) | /* max_num_huffman_tables : 3 */
                            (1 << 7) | /* max_num_scans : 4 */
                            (jpegNumComponent << 4)) /* max_num_components : 3 */},
   {VAConfigAttribEncQuantization, 0},
   {VAConfigAttribEncSkipFrame, 0},
   {VAConfigAttribFEIMVPredictors, 4},
   {VAConfigAttribCustomRoundingControl, 1},
   {VAConfigAttribMaxFrameSize, 3},
   {VAConfigAttribPredictionDirection, 7},
#if VA_CHECK_VERSION(1, 12, 0)
   {VAConfigAttribEncHEVCFeatures, vaConfigAttribValEncHEVCFeatures.value},
   {VAConfigAttribEncHEVCBlockSizes, vaConfigAttribValEncHEVCBlockSizes.value},
#endif
};

//!
static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_12K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_WIDTH}}},
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}},
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P010}}},
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_12K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_WIDTH}}},
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}},
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_AYUV}}},
  #if VA_CHECK_VERSION(1, 13, 0)
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_XYUV}}},
  #endif
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_12K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_WIDTH}}},
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}},
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y410}}},
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_12K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_WIDTH}}},
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}},
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCSccMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_NV12}}},
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_12K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_WIDTH}}},
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}},
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCSccMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_P010}}},
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_12K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_WIDTH}}},
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}},
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCSccMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_AYUV}}},
  #if VA_CHECK_VERSION(1, 13, 0)
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_XYUV}}},
  #endif
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_12K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_WIDTH}}},
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}},
};

static ProfileSurfaceAttribInfo surfaceAttribInfo_VAProfileHEVCSccMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0 =
{
  {VASurfaceAttribPixelFormat, VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE, {VAGenericValueTypeInteger, {VA_FOURCC_Y410}}},
  {VASurfaceAttribMaxWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_16K_MAX_PIC_WIDTH}}},
  {VASurfaceAttribMaxHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_12K_MAX_PIC_HEIGHT}}},
  {VASurfaceAttribMinWidth, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_WIDTH}}},
  {VASurfaceAttribMinHeight, VA_SURFACE_ATTRIB_GETTABLE, {VAGenericValueTypeInteger, {CODEC_128_MIN_PIC_HEIGHT}}},
  {VASurfaceAttribMemoryType,  VA_SURFACE_ATTRIB_GETTABLE | VA_SURFACE_ATTRIB_SETTABLE,
    {VAGenericValueTypeInteger, {
#if VA_CHECK_VERSION(1, 21, 0)
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 |
#endif
        VA_SURFACE_ATTRIB_MEM_TYPE_VA | 
        VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2
    }}},
};

//!
//! \brief  Definion for EmtrypointMap
//!
static const EntrypointData entrypointMap_VAProfileHEVCMain_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileHEVCMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileHEVCMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileHEVCMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
};

static const EntrypointData entrypointMap_VAProfileHEVCMain10_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileHEVCMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileHEVCMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileHEVCMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
};

static const EntrypointData entrypointMap_VAProfileHEVCMain444_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileHEVCMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileHEVCMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileHEVCMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
};

static const EntrypointData entrypointMap_VAProfileHEVCMain444_10_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileHEVCMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileHEVCMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileHEVCMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
};

static const EntrypointData entrypointMap_VAProfileHEVCSccMain_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileHEVCSccMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileHEVCSccMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileHEVCSccMain_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
};

static const EntrypointData entrypointMap_VAProfileHEVCSccMain10_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileHEVCSccMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileHEVCSccMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileHEVCSccMain10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
};

static const EntrypointData entrypointMap_VAProfileHEVCSccMain444_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileHEVCSccMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileHEVCSccMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileHEVCSccMain444_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
};

static const EntrypointData entrypointMap_VAProfileHEVCSccMain444_10_Data_Xe_Lpm_plus_r0
{
    &attribList_VAProfileHEVCSccMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &configDataList_VAProfileHEVCSccMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
    &surfaceAttribInfo_VAProfileHEVCSccMain444_10_VAEntrypointEncSlice_Xe_Lpm_plus_r0,
};

#endif // __CAPSTABLE_DATA_HEVC_ENCODE_XE_LPM_PLUS_R0_SPECIFIC_H__
