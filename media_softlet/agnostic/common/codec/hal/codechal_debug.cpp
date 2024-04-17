/*
* Copyright (c) 2011-2023, Intel Corporation
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
//! \file     codechal_debug.cpp
//! \brief    Defines the debug interface shared by codec only.
//! \details  The debug interface dumps output from Media based on in input config file.
//!
#include "codechal_debug.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_config_manager.h"
#include "media_debug_fast_dump.h"
#include <iomanip>

#define QUERY_STATUS_BUFFER_INITIAL_VALUE 0xf
#define STORE_DATA_DWORD_DATA 0x01010102

// AVC Decode
std::ostream &operator<<(std::ostream &oss, const CODEC_AVC_PIC_PARAMS &cr)
{
    const CODEC_AVC_PIC_PARAMS *picParams = &cr;

    oss << "CurrPic FrameIdx: " << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << +picParams->CurrPic.PicFlags << std::endl;

    // Dump RefFrameList[15]
    for (uint8_t i = 0; i < 15; ++i)
    {
        oss << "RefFrameList[" << +i << "] FrameIdx:" << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList[" << +i << "] PicFlags:" << +picParams->RefFrameList[i].PicFlags << std::endl;
    }

    oss << "pic_width_in_mbs_minus1: " << +picParams->pic_width_in_mbs_minus1 << std::endl;
    oss << "pic_height_in_mbs_minus1: " << +picParams->pic_height_in_mbs_minus1 << std::endl;
    oss << "bit_depth_luma_minus8: " << +picParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8: " << +picParams->bit_depth_chroma_minus8 << std::endl;
    oss << "num_ref_frames: " << +picParams->num_ref_frames << std::endl;
    oss << "CurrFieldOrderCnt: " << +picParams->CurrFieldOrderCnt[0] << std::endl;
    oss << "CurrFieldOrderCnt: " << +picParams->CurrFieldOrderCnt[1] << std::endl;

    // Dump FieldOrderCntList (16x2)
    for (uint8_t i = 0; i < 2; ++i)
    {
        oss << "FieldOrderCntList[" << +i << "]:";
        for (uint8_t j = 0; j < 16; j++)
            oss << +picParams->FieldOrderCntList[j][i] << " ";
        oss << std::endl;
    }

    // Dump seq_fields
    oss << "seq_fields value: " << +picParams->seq_fields.value << std::endl;
    oss << "chroma_format_idc: " << +picParams->seq_fields.chroma_format_idc << std::endl;
    oss << "residual_colour_transform_flag: " << +picParams->seq_fields.residual_colour_transform_flag << std::endl;
    oss << "frame_mbs_only_flag: " << +picParams->seq_fields.frame_mbs_only_flag << std::endl;
    oss << "mb_adaptive_frame_field_flag: " << +picParams->seq_fields.mb_adaptive_frame_field_flag << std::endl;
    oss << "direct_8x8_inference_flag: " << +picParams->seq_fields.direct_8x8_inference_flag << std::endl;
    oss << "log2_max_frame_num_minus4: " << +picParams->seq_fields.log2_max_frame_num_minus4 << std::endl;
    oss << "pic_order_cnt_type: " << +picParams->seq_fields.pic_order_cnt_type << std::endl;
    oss << "log2_max_pic_order_cnt_lsb_minus4: " << +picParams->seq_fields.log2_max_pic_order_cnt_lsb_minus4 << std::endl;
    oss << "delta_pic_order_always_zero_flag: " << +picParams->seq_fields.delta_pic_order_always_zero_flag << std::endl;
    oss << "num_slice_groups_minus1:" << +picParams->num_slice_groups_minus1 << std::endl;
    oss << "slice_group_map_type:" << +picParams->slice_group_map_type << std::endl;
    oss << "slice_group_change_rate_minus1:" << +picParams->slice_group_change_rate_minus1 << std::endl;
    oss << "pic_init_qp_minus26:" << +picParams->pic_init_qp_minus26 << std::endl;
    oss << "chroma_qp_index_offset:" << +picParams->chroma_qp_index_offset << std::endl;
    oss << "second_chroma_qp_index_offset:" << +picParams->second_chroma_qp_index_offset << std::endl;

    // Dump pic_fields
    oss << "pic_fields value: " << +picParams->pic_fields.value << std::endl;
    oss << "entropy_coding_mode_flag: " << +picParams->pic_fields.entropy_coding_mode_flag << std::endl;
    oss << "weighted_pred_flag: " << +picParams->pic_fields.weighted_pred_flag << std::endl;
    oss << "weighted_bipred_idc: " << +picParams->pic_fields.weighted_bipred_idc << std::endl;
    oss << "transform_8x8_mode_flag: " << +picParams->pic_fields.transform_8x8_mode_flag << std::endl;
    oss << "field_pic_flag: " << +picParams->pic_fields.field_pic_flag << std::endl;
    oss << "constrained_intra_pred_flag: " << +picParams->pic_fields.constrained_intra_pred_flag << std::endl;
    oss << "pic_order_present_flag: " << +picParams->pic_fields.pic_order_present_flag << std::endl;
    oss << "deblocking_filter_control_present_flag: " << +picParams->pic_fields.deblocking_filter_control_present_flag << std::endl;
    oss << "redundant_pic_cnt_present_flag: " << +picParams->pic_fields.redundant_pic_cnt_present_flag << std::endl;
    oss << "reference_pic_flag: " << +picParams->pic_fields.reference_pic_flag << std::endl;
    oss << "IntraPicFlag: " << +picParams->pic_fields.IntraPicFlag << std::endl;

    // Dump Short format specific
    oss << "num_ref_idx_l0_active_minus1: " << +picParams->num_ref_idx_l0_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_active_minus1: " << +picParams->num_ref_idx_l1_active_minus1 << std::endl;
    oss << "NonExistingFrameFlags: " << +picParams->NonExistingFrameFlags << std::endl;
    oss << "UsedForReferenceFlags: " << +picParams->UsedForReferenceFlags << std::endl;
    oss << "frame_num: " << +picParams->frame_num << std::endl;
    oss << "StatusReportFeedbackNumber: " << +picParams->StatusReportFeedbackNumber << std::endl;

    // Dump FrameNumList[16]
    oss << "scaling_list_present_flag_buffer:";
    for (uint8_t i = 0; i < 16; i++)
        oss << picParams->FrameNumList[i];
    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_AVC_SF_SLICE_PARAMS &cr)
{
    const CODEC_AVC_SF_SLICE_PARAMS *sliceControl = &cr;

    oss << "slice_data_size: " << +sliceControl->slice_data_size << std::endl;
    oss << "slice_data_offset: " << +sliceControl->slice_data_offset << std::endl;
    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_AVC_SLICE_PARAMS &cr)
{
    const CODEC_AVC_SLICE_PARAMS *sliceControl = &cr;

    oss << "slice_data_size: " << +sliceControl->slice_data_size << std::endl;
    oss << "slice_data_offset: " << +sliceControl->slice_data_offset << std::endl;

    // Dump Long format Params
    oss << std::endl;
    oss << "// Long Format Specific" << std::endl;
    oss << "slice_data_bit_offset: " << +sliceControl->slice_data_bit_offset << std::endl;
    oss << "first_mb_in_slice: " << +sliceControl->first_mb_in_slice << std::endl;
    oss << "NumMbsForSlice: " << +sliceControl->NumMbsForSlice << std::endl;
    oss << "slice_type: " << +sliceControl->slice_type << std::endl;
    oss << "direct_spatial_mv_pred_flag: " << +sliceControl->direct_spatial_mv_pred_flag << std::endl;
    oss << "num_ref_idx_l0_active_minus1: " << +sliceControl->num_ref_idx_l0_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_active_minus1: " << +sliceControl->num_ref_idx_l1_active_minus1 << std::endl;
    oss << "cabac_init_idc: " << +sliceControl->cabac_init_idc << std::endl;
    oss << "slice_qp_delta: " << +sliceControl->slice_qp_delta << std::endl;
    oss << "disable_deblocking_filter_idc: " << +sliceControl->disable_deblocking_filter_idc << std::endl;
    oss << "slice_alpha_c0_offset_div2: " << +sliceControl->slice_alpha_c0_offset_div2 << std::endl;
    oss << "slice_beta_offset_div2: " << +sliceControl->slice_beta_offset_div2 << std::endl;

    // Dump RefPicList[2][32]
    for (uint8_t i = 0; i < 32; ++i)
    {
        oss << "RefPicList[0][" << +i << "] FrameIdx: " << +sliceControl->RefPicList[0][i].FrameIdx << std::endl;
        oss << "RefPicList[0][" << +i << "] PicFlags: " << +sliceControl->RefPicList[0][i].PicFlags << std::endl;
        oss << "RefPicList[1][" << +i << "] FrameIdx: " << +sliceControl->RefPicList[1][i].FrameIdx << std::endl;
        oss << "RefPicList[1][" << +i << "] PicFlags: " << +sliceControl->RefPicList[1][i].PicFlags << std::endl;
    }

    oss << "luma_log2_weight_denom: " << +sliceControl->luma_log2_weight_denom << std::endl;
    oss << "chroma_log2_weight_denom: " << +sliceControl->chroma_log2_weight_denom << std::endl;
    oss << "slice_id: " << +sliceControl->slice_id << std::endl;

    // Dump Weights[2][32][3][2]
    for (uint8_t i = 0; i < 32; ++i)
    {
        oss << "Weights[0][" << +i << "][0][0]: " << +sliceControl->Weights[0][i][0][0] << std::endl;
        oss << "Weights[0][" << +i << "][0][1]: " << +sliceControl->Weights[0][i][0][1] << std::endl;
        oss << "Weights[0][" << +i << "][1][0]: " << +sliceControl->Weights[0][i][1][0] << std::endl;
        oss << "Weights[0][" << +i << "][1][1]: " << +sliceControl->Weights[0][i][1][1] << std::endl;
        oss << "Weights[1][" << +i << "][0][0]: " << +sliceControl->Weights[1][i][0][0] << std::endl;
        oss << "Weights[1][" << +i << "][0][1]: " << +sliceControl->Weights[1][i][0][1] << std::endl;
        oss << "Weights[1][" << +i << "][1][0]: " << +sliceControl->Weights[1][i][1][0] << std::endl;
        oss << "Weights[1][" << +i << "][1][1]: " << +sliceControl->Weights[1][i][1][1] << std::endl;
        oss << "Weights[0][" << +i << "][2][0]: " << +sliceControl->Weights[0][i][2][0] << std::endl;
        oss << "Weights[0][" << +i << "][2][1]: " << +sliceControl->Weights[0][i][2][1] << std::endl;
        oss << "Weights[1][" << +i << "][2][0]: " << +sliceControl->Weights[1][i][2][0] << std::endl;
        oss << "Weights[1][" << +i << "][2][1]: " << +sliceControl->Weights[1][i][2][1] << std::endl;
    }

    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_AVC_IQ_MATRIX_PARAMS &cr)
{
    const CODEC_AVC_IQ_MATRIX_PARAMS *iqParams = &cr;

    uint32_t idx, idx2;
    // 4x4 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "Qmatrix_H264_ScalingLists4x4[" << +idx2 << "]:" << std::endl;
        for (idx = 0; idx < 12; idx += 4)
        {
            oss << "ScalingList4x4[" << +idx / 4 << "]:";
            oss << +iqParams->ScalingList4x4[idx2][idx] << " ";
            oss << +iqParams->ScalingList4x4[idx2][idx + 1] << " ";
            oss << +iqParams->ScalingList4x4[idx2][idx + 2] << " ";
            oss << +iqParams->ScalingList4x4[idx2][idx + 3] << " ";
            oss << std::endl;
        }
        oss << std::endl;
    }
    // 8x8 block
    for (idx2 = 0; idx2 < 2; idx2++)
    {
        oss << "Qmatrix_H264_ScalingLists8x8[" << +idx2 << "]:" << std::endl;
        for (idx = 0; idx < 56; idx += 8)
        {
            oss << "ScalingList8x8[" << +idx / 8 << "]:";
            oss << +iqParams->ScalingList8x8[idx2][idx] << " ";
            oss << +iqParams->ScalingList8x8[idx2][idx + 1] << " ";
            oss << +iqParams->ScalingList8x8[idx2][idx + 2] << " ";
            oss << +iqParams->ScalingList8x8[idx2][idx + 3] << " ";
            oss << +iqParams->ScalingList8x8[idx2][idx + 4] << " ";
            oss << +iqParams->ScalingList8x8[idx2][idx + 5] << " ";
            oss << +iqParams->ScalingList8x8[idx2][idx + 6] << " ";
            oss << +iqParams->ScalingList8x8[idx2][idx + 7] << " ";
            oss << std::endl;
        }
        oss << std::endl;
    }

    return oss;
}

void DumpDecodeAvcPicParams(PCODEC_AVC_PIC_PARAMS picParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *picParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeAvcSliceParams(PCODEC_AVC_SLICE_PARAMS sliceParams, uint32_t numSlices, std::string fileName, bool shortFormatInUse)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    if (numSlices > 1)
    {
        oss << "Total Num: " << numSlices << std::endl;
        oss << std::endl;
    }

    for (uint16_t i = 0; i < numSlices; i++)
    {
        if (numSlices > 1)
        {
            oss << "---------Index = " << i << "---------" << std::endl;
        }

        if (shortFormatInUse)
        {
            oss << reinterpret_cast<CODEC_AVC_SF_SLICE_PARAMS &>(sliceParams[i]) << std::endl;
        }
        else
        {
            oss << sliceParams[i] << std::endl;
        }
    }

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeAvcIQParams(PCODEC_AVC_IQ_MATRIX_PARAMS iqParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *iqParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

// HEVC Decode
std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_PIC_PARAMS &cr)
{
    const CODEC_HEVC_PIC_PARAMS *picParams = &cr;

    oss << "PicWidthInMinCbsY: " << +picParams->PicWidthInMinCbsY << std::endl;
    oss << "PicHeightInMinCbsY: " << +picParams->PicHeightInMinCbsY << std::endl;

    // FormatAndSequenceInfoFlags
    oss << "chroma_format_idc: " << +picParams->chroma_format_idc << std::endl;
    oss << "separate_colour_plane_flag: " << +picParams->separate_colour_plane_flag << std::endl;
    oss << "bit_depth_luma_minus8: " << +picParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8: " << +picParams->bit_depth_chroma_minus8 << std::endl;
    oss << "log2_max_pic_order_cnt_lsb_minus4: " << +picParams->log2_max_pic_order_cnt_lsb_minus4 << std::endl;
    oss << "NoPicReorderingFlag: " << +picParams->NoPicReorderingFlag << std::endl;
    oss << "ReservedBits1: " << +picParams->ReservedBits1 << std::endl;
    oss << "wFormatAndSequenceInfoFlags: " << +picParams->wFormatAndSequenceInfoFlags << std::endl;
    oss << "CurrPic FrameIdx: " << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << +picParams->CurrPic.PicFlags << std::endl;
    oss << "sps_max_dec_pic_buffering_minus1: " << +picParams->sps_max_dec_pic_buffering_minus1 << std::endl;
    oss << "log2_min_luma_coding_block_size_minus3: " << +picParams->log2_min_luma_coding_block_size_minus3 << std::endl;
    oss << "log2_diff_max_min_luma_coding_block_size: " << +picParams->log2_diff_max_min_luma_coding_block_size << std::endl;
    oss << "log2_min_transform_block_size_minus2: " << +picParams->log2_min_transform_block_size_minus2 << std::endl;
    oss << "log2_diff_max_min_transform_block_size: " << +picParams->log2_diff_max_min_transform_block_size << std::endl;
    oss << "max_transform_hierarchy_depth_intra: " << +picParams->max_transform_hierarchy_depth_intra << std::endl;
    oss << "max_transform_hierarchy_depth_inter: " << +picParams->max_transform_hierarchy_depth_inter << std::endl;
    oss << "num_short_term_ref_pic_sets: " << +picParams->num_short_term_ref_pic_sets << std::endl;
    oss << "num_long_term_ref_pic_sps: " << +picParams->num_long_term_ref_pic_sps << std::endl;
    oss << "num_ref_idx_l0_default_active_minus1: " << +picParams->num_ref_idx_l0_default_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_default_active_minus1: " << +picParams->num_ref_idx_l1_default_active_minus1 << std::endl;
    oss << "init_qp_minus26: " << +picParams->init_qp_minus26 << std::endl;
    oss << "ucNumDeltaPocsOfRefRpsIdx: " << +picParams->ucNumDeltaPocsOfRefRpsIdx << std::endl;
    oss << "wNumBitsForShortTermRPSInSlice: " << +picParams->wNumBitsForShortTermRPSInSlice << std::endl;
    oss << "ReservedBits2: " << +picParams->ReservedBits2 << std::endl;

    // CodingParamToolFlags
    oss << "scaling_list_enabled_flag: " << +picParams->scaling_list_enabled_flag << std::endl;
    oss << "amp_enabled_flag: " << +picParams->amp_enabled_flag << std::endl;
    oss << "sample_adaptive_offset_enabled_flag: " << +picParams->sample_adaptive_offset_enabled_flag << std::endl;
    oss << "pcm_enabled_flag: " << +picParams->pcm_enabled_flag << std::endl;
    oss << "pcm_sample_bit_depth_luma_minus1: " << +picParams->pcm_sample_bit_depth_luma_minus1 << std::endl;
    oss << "pcm_sample_bit_depth_chroma_minus1: " << +picParams->pcm_sample_bit_depth_chroma_minus1 << std::endl;
    oss << "log2_min_pcm_luma_coding_block_size_minus3: " << +picParams->log2_min_pcm_luma_coding_block_size_minus3 << std::endl;
    oss << "log2_diff_max_min_pcm_luma_coding_block_size: " << +picParams->log2_diff_max_min_pcm_luma_coding_block_size << std::endl;
    oss << "pcm_loop_filter_disabled_flag: " << +picParams->pcm_loop_filter_disabled_flag << std::endl;
    oss << "long_term_ref_pics_present_flag: " << +picParams->long_term_ref_pics_present_flag << std::endl;
    oss << "sps_temporal_mvp_enabled_flag: " << +picParams->sps_temporal_mvp_enabled_flag << std::endl;
    oss << "strong_intra_smoothing_enabled_flag: " << +picParams->strong_intra_smoothing_enabled_flag << std::endl;
    oss << "dependent_slice_segments_enabled_flag: " << +picParams->dependent_slice_segments_enabled_flag << std::endl;
    oss << "output_flag_present_flag: " << +picParams->output_flag_present_flag << std::endl;
    oss << "num_extra_slice_header_bits: " << +picParams->num_extra_slice_header_bits << std::endl;
    oss << "sign_data_hiding_enabled_flag: " << +picParams->sign_data_hiding_enabled_flag << std::endl;
    oss << "cabac_init_present_flag: " << +picParams->cabac_init_present_flag << std::endl;
    oss << "ReservedBits3: " << +picParams->ReservedBits3 << std::endl;
    oss << "dwCodingParamToolFlags: " << +picParams->dwCodingParamToolFlags << std::endl;

    // CodingSettingPicturePropertyFlags
    oss << "constrained_intra_pred_flag: " << +picParams->constrained_intra_pred_flag << std::endl;
    oss << "transform_skip_enabled_flag: " << +picParams->transform_skip_enabled_flag << std::endl;
    oss << "cu_qp_delta_enabled_flag: " << +picParams ->cu_qp_delta_enabled_flag << std::endl;
    oss << "diff_cu_qp_delta_depth: " << +picParams->diff_cu_qp_delta_depth << std::endl;
    oss << "pps_slice_chroma_qp_offsets_present_flag: " << +picParams->pps_slice_chroma_qp_offsets_present_flag << std::endl;
    oss << "weighted_pred_flag: " << +picParams->weighted_pred_flag << std::endl;
    oss << "weighted_bipred_flag: " << +picParams->weighted_bipred_flag << std::endl;
    oss << "transquant_bypass_enabled_flag: " << +picParams->transquant_bypass_enabled_flag << std::endl;
    oss << "tiles_enabled_flag: " << +picParams->tiles_enabled_flag << std::endl;
    oss << "entropy_coding_sync_enabled_flag: " << +picParams->entropy_coding_sync_enabled_flag << std::endl;
    oss << "uniform_spacing_flag: " << +picParams->uniform_spacing_flag << std::endl;
    oss << "loop_filter_across_tiles_enabled_flag: " << +picParams->loop_filter_across_tiles_enabled_flag << std::endl;
    oss << "pps_loop_filter_across_slices_enabled_flag: " << +picParams->pps_loop_filter_across_slices_enabled_flag << std::endl;
    oss << "deblocking_filter_override_enabled_flag: " << +picParams->deblocking_filter_override_enabled_flag << std::endl;
    oss << "pps_deblocking_filter_disabled_flag: " << +picParams->pps_deblocking_filter_disabled_flag << std::endl;
    oss << "lists_modification_present_flag: " << +picParams->lists_modification_present_flag << std::endl;
    oss << "slice_segment_header_extension_present_flag: " << +picParams->slice_segment_header_extension_present_flag << std::endl;
    oss << "IrapPicFlag: " << +picParams->IrapPicFlag << std::endl;
    oss << "IdrPicFlag: " << +picParams->IdrPicFlag << std::endl;
    oss << "IntraPicFlag: " << +picParams->IntraPicFlag << std::endl;
    oss << "ReservedBits4: " << +picParams->ReservedBits4 << std::endl;
    oss << "dwCodingSettingPicturePropertyFlags: " << +picParams->dwCodingSettingPicturePropertyFlags << std::endl;
    oss << "pps_cb_qp_offset: " << +picParams->pps_cb_qp_offset << std::endl;
    oss << "pps_cr_qp_offset: " << +picParams->pps_cr_qp_offset << std::endl;
    oss << "num_tile_columns_minus1: " << +picParams->num_tile_columns_minus1 << std::endl;
    oss << "num_tile_rows_minus1: " << +picParams->num_tile_rows_minus1 << std::endl;

    // column width
    oss << "column_width_minus1[19]:";
    for (uint8_t i = 0; i < 19; i++)
        oss << picParams->column_width_minus1[i] << " ";
    oss << std::endl;

    // row height
    oss << "row_height_minus1[21]:";
    for (uint8_t i = 0; i < 21; i++)
        oss << picParams->row_height_minus1[i] << " ";
    oss << std::endl;

    oss << "pps_beta_offset_div2: " << +picParams->pps_beta_offset_div2 << std::endl;
    oss << "pps_tc_offset_div2: " << +picParams->pps_tc_offset_div2 << std::endl;
    oss << "log2_parallel_merge_level_minus2: " << +picParams->log2_parallel_merge_level_minus2 << std::endl;
    oss << "CurrPicOrderCntVal: " << +picParams->CurrPicOrderCntVal << std::endl;

    oss.setf(std::ios::dec, std::ios::basefield);

    // RefFrameList[15]
    for (uint8_t i = 0; i < 15; ++i)
    {
        oss << "RefFrameList[" << +i << "].FrameIdx:" << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList[" << +i << "].PicFlags:" << +picParams->RefFrameList[i].PicFlags << std::endl;
    }

    // POC List
    oss << "PicOrderCntValList[15]:";
    for (uint8_t i = 0; i < 15; i++)
        oss << picParams->PicOrderCntValList[i] << " ";
    oss << std::endl;

    // Ref RefPicSetStCurrBefore List
    oss << "RefPicSetStCurrBefore[8]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->RefPicSetStCurrBefore[i] << " ";
    oss << std::endl;

    // Ref RefPicSetStCurrAfter List
    oss << "RefPicSetStCurrAfter[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->RefPicSetStCurrAfter[i] << " ";
    oss << std::endl;

    // Ref PicSetStCurr List
    oss << "RefPicSetLtCurr[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->RefPicSetLtCurr[i] << " ";
    oss << std::endl;

    // Ref RefPicSetStCurrBefore List with POC
    oss << "POC of RefPicSetStCurrBefore[8]: ";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->PicOrderCntValList[picParams->RefPicSetStCurrBefore[i] % 15] << " ";
    oss << std::endl;

    // Ref RefPicSetStCurrAfter List with POC
    oss << "POC of RefPicSetStCurrAfter[16]:";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->PicOrderCntValList[picParams->RefPicSetStCurrAfter[i] % 15] << " ";
    oss << std::endl;

    // Ref PicSetStCurr List with POC
    oss << "POC of RefPicSetLtCurr[16]: ";
    for (uint8_t i = 0; i < 8; i++)
        oss << +picParams->PicOrderCntValList[picParams->RefPicSetLtCurr[i] % 15] << " ";
    oss << std::endl;

    oss << "RefFieldPicFlag: " << +picParams->RefFieldPicFlag << std::endl;
    oss << "RefBottomFieldFlag: " << +picParams->RefBottomFieldFlag << std::endl;
    oss << "StatusReportFeedbackNumber: " << +picParams->StatusReportFeedbackNumber << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_EXT_PIC_PARAMS &cr)
{
    const CODEC_HEVC_EXT_PIC_PARAMS *extPicParams = &cr;

    // PicRangeExtensionFlags
    oss << "transform_skip_rotation_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.transform_skip_rotation_enabled_flag << std::endl;
    oss << "transform_skip_context_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.transform_skip_context_enabled_flag << std::endl;
    oss << "implicit_rdpcm_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.implicit_rdpcm_enabled_flag << std::endl;
    oss << "explicit_rdpcm_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.explicit_rdpcm_enabled_flag << std::endl;
    oss << "extended_precision_processing_flag: " << +extPicParams->PicRangeExtensionFlags.fields.extended_precision_processing_flag << std::endl;
    oss << "intra_smoothing_disabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.intra_smoothing_disabled_flag << std::endl;
    oss << "high_precision_offsets_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.high_precision_offsets_enabled_flag << std::endl;
    oss << "persistent_rice_adaptation_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.persistent_rice_adaptation_enabled_flag << std::endl;
    oss << "cabac_bypass_alignment_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.cabac_bypass_alignment_enabled_flag << std::endl;
    oss << "cross_component_prediction_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.cross_component_prediction_enabled_flag << std::endl;
    oss << "chroma_qp_offset_list_enabled_flag: " << +extPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag << std::endl;
    oss << "BitDepthLuma16: " << +extPicParams->PicRangeExtensionFlags.fields.BitDepthLuma16 << std::endl;
    oss << "BitDepthChroma16: " << +extPicParams->PicRangeExtensionFlags.fields.BitDepthChroma16 << std::endl;
    oss << "diff_cu_chroma_qp_offset_depth: " << +extPicParams->diff_cu_chroma_qp_offset_depth << std::endl;
    oss << "chroma_qp_offset_list_len_minus1: " << +extPicParams->chroma_qp_offset_list_len_minus1 << std::endl;
    oss << "log2_sao_offset_scale_luma: " << +extPicParams->log2_sao_offset_scale_luma << std::endl;
    oss << "log2_sao_offset_scale_chroma: " << +extPicParams->log2_sao_offset_scale_chroma << std::endl;
    oss << "log2_max_transform_skip_block_size_minus2: " << +extPicParams->log2_max_transform_skip_block_size_minus2 << std::endl;

    // cb_qp_offset_list[6]
    oss << "cb_qp_offset_list[6]: ";
    for (uint8_t i = 0; i < 6; i++)
        oss << +extPicParams->cb_qp_offset_list[i] << " ";
    oss << std::endl;

    // cr_qp_offset_list[6]
    oss << "cr_qp_offset_list[6]: ";
    for (uint8_t i = 0; i < 6; i++)
        oss << +extPicParams->cr_qp_offset_list[i] << " ";
    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_SCC_PIC_PARAMS &cr)
{
    const CODEC_HEVC_SCC_PIC_PARAMS *sccPicParams = &cr;

    oss << "pps_curr_pic_ref_enabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.pps_curr_pic_ref_enabled_flag << std::endl;
    oss << "palette_mode_enabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.palette_mode_enabled_flag << std::endl;
    oss << "motion_vector_resolution_control_idc: " << +sccPicParams->PicSCCExtensionFlags.fields.motion_vector_resolution_control_idc << std::endl;
    oss << "intra_boundary_filtering_disabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.intra_boundary_filtering_disabled_flag << std::endl;
    oss << "residual_adaptive_colour_transform_enabled_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.residual_adaptive_colour_transform_enabled_flag << std::endl;
    oss << "pps_slice_act_qp_offsets_present_flag: " << +sccPicParams->PicSCCExtensionFlags.fields.pps_slice_act_qp_offsets_present_flag << std::endl;
    oss << "palette_max_size: " << +sccPicParams->palette_max_size << std::endl;
    oss << "delta_palette_max_predictor_size: " << +sccPicParams->delta_palette_max_predictor_size << std::endl;
    oss << "PredictorPaletteSize: " << +sccPicParams->PredictorPaletteSize << std::endl;

    for (uint8_t i = 0; i < 128; i++)
    {
        oss << "PredictorPaletteEntries[0][" << +i << "]: " << +sccPicParams->PredictorPaletteEntries[0][i] << std::endl;
        oss << "PredictorPaletteEntries[1][" << +i << "]: " << +sccPicParams->PredictorPaletteEntries[1][i] << std::endl;
        oss << "PredictorPaletteEntries[2][" << +i << "]: " << +sccPicParams->PredictorPaletteEntries[2][i] << std::endl;
    }

    oss << "pps_act_y_qp_offset_plus5: " << +sccPicParams->pps_act_y_qp_offset_plus5 << std::endl;
    oss << "pps_act_cb_qp_offset_plus5: " << +sccPicParams->pps_act_y_qp_offset_plus5 << std::endl;
    oss << "pps_act_cr_qp_offset_plus3: " << +sccPicParams->pps_act_y_qp_offset_plus5 << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_SF_SLICE_PARAMS &cr)
{
    const CODEC_HEVC_SF_SLICE_PARAMS *sliceControl = &cr;

    oss << "slice_data_size: " << +sliceControl->slice_data_size << std::endl;
    oss << "slice_data_offset: " << +sliceControl->slice_data_offset << std::endl;
    oss << "slice_chopping: " << +sliceControl->slice_chopping << std::endl;
    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_SLICE_PARAMS &cr)
{
    const CODEC_HEVC_SLICE_PARAMS *hevcSliceControl = &cr;

    oss << "slice_data_size: " << +hevcSliceControl->slice_data_size << std::endl;
    oss << "slice_data_offset: " << +hevcSliceControl->slice_data_offset << std::endl;
    oss << "slice_chopping: " << +hevcSliceControl->slice_chopping << std::endl;

    // Dump Long format Params
    oss << std::endl;
    oss << "ByteOffsetToSliceData: " << +hevcSliceControl->ByteOffsetToSliceData << std::endl;
    oss << "slice_segment_address: " << +hevcSliceControl->slice_segment_address << std::endl;

    // RefPicList[2][15]
    for (uint8_t i = 0; i < 15; ++i)
    {
        oss << "RefPicList[0][" << +i << "]";
        oss << ".FrameIdx: " << +hevcSliceControl->RefPicList[0][i].FrameIdx << std::endl;
        oss << "RefPicList[0][" << +i << "]";
        oss << ".PicFlags: " << +hevcSliceControl->RefPicList[0][i].PicFlags << std::endl;
    }
    for (uint8_t i = 0; i < 15; ++i)
    {
        oss << "RefPicList[1][" << +i << "]";
        oss << ".FrameIdx: " << +hevcSliceControl->RefPicList[1][i].FrameIdx << std::endl;
        oss << "RefPicList[1][" << +i << "]";
        oss << ".PicFlags: " << +hevcSliceControl->RefPicList[1][i].PicFlags << std::endl;
    }

    oss << "last_slice_of_pic: " << +hevcSliceControl->LongSliceFlags.fields.LastSliceOfPic << std::endl;
    oss << "dependent_slice_segment_flag: " << +hevcSliceControl->LongSliceFlags.fields.dependent_slice_segment_flag << std::endl;
    oss << "slice_type: " << +hevcSliceControl->LongSliceFlags.fields.slice_type << std::endl;
    oss << "color_plane_id: " << +hevcSliceControl->LongSliceFlags.fields.color_plane_id << std::endl;
    oss << "slice_sao_luma_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_sao_luma_flag << std::endl;
    oss << "slice_sao_chroma_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_sao_chroma_flag << std::endl;
    oss << "mvd_l1_zero_flag: " << +hevcSliceControl->LongSliceFlags.fields.mvd_l1_zero_flag << std::endl;
    oss << "cabac_init_flag: " << +hevcSliceControl->LongSliceFlags.fields.cabac_init_flag << std::endl;
    oss << "slice_temporal_mvp_enabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_temporal_mvp_enabled_flag << std::endl;
    oss << "slice_deblocking_filter_disabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_deblocking_filter_disabled_flag << std::endl;
    oss << "collocated_from_l0_flag: " << +hevcSliceControl->LongSliceFlags.fields.collocated_from_l0_flag << std::endl;
    oss << "slice_loop_filter_across_slices_enabled_flag: " << +hevcSliceControl->LongSliceFlags.fields.slice_loop_filter_across_slices_enabled_flag << std::endl;
    oss << "reserved: " << +hevcSliceControl->LongSliceFlags.fields.reserved << std::endl;
    oss << "collocated_ref_idx: " << +hevcSliceControl->collocated_ref_idx << std::endl;
    oss << "num_ref_idx_l0_active_minus1: " << +hevcSliceControl->num_ref_idx_l0_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_active_minus1: " << +hevcSliceControl->num_ref_idx_l1_active_minus1 << std::endl;
    oss << "slice_qp_delta: " << +hevcSliceControl->slice_qp_delta << std::endl;
    oss << "slice_cb_qp_offset: " << +hevcSliceControl->slice_cb_qp_offset << std::endl;
    oss << "slice_cr_qp_offset: " << +hevcSliceControl->slice_cr_qp_offset << std::endl;
    oss << "slice_beta_offset_div2: " << +hevcSliceControl->slice_beta_offset_div2 << std::endl;
    oss << "slice_tc_offset_div2: " << +hevcSliceControl->slice_tc_offset_div2 << std::endl;
    oss << "luma_log2_weight_denom: " << +hevcSliceControl->luma_log2_weight_denom << std::endl;
    oss << "delta_chroma_log2_weight_denom: " << +hevcSliceControl->delta_chroma_log2_weight_denom << std::endl;

    // luma_offset[2][15]
    for (uint8_t i = 0; i < 15; i++)
    {
        oss << "luma_offset_l0[" << +i << "]: " << +hevcSliceControl->luma_offset_l0[i] << std::endl;
        oss << "luma_offset_l1[" << +i << "]: " << +hevcSliceControl->luma_offset_l1[i] << std::endl;
    }
    // delta_luma_weight[2][15]
    for (uint8_t i = 0; i < 15; i++)
    {
        oss << "delta_luma_weight_l0[" << +i << "]: " << +hevcSliceControl->delta_luma_weight_l0[i] << std::endl;
        oss << "delta_luma_weight_l1[" << +i << "]: " << +hevcSliceControl->delta_luma_weight_l1[i] << std::endl;
    }
    // chroma_offset[2][15][2]
    for (uint8_t i = 0; i < 15; i++)
    {
        oss << "ChromaOffsetL0[" << +i << "][0]: " << +hevcSliceControl->ChromaOffsetL0[i][0] << std::endl;
        oss << "ChromaOffsetL0[" << +i << "][1]: " << +hevcSliceControl->ChromaOffsetL0[i][1] << std::endl;
        oss << "ChromaOffsetL1[" << +i << "][0]: " << +hevcSliceControl->ChromaOffsetL1[i][0] << std::endl;
        oss << "ChromaOffsetL1[" << +i << "][1]: " << +hevcSliceControl->ChromaOffsetL1[i][1] << std::endl;
    }
    // delta_chroma_weight[2][15][2]
    for (uint8_t i = 0; i < 15; i++)
    {
        oss << "delta_chroma_weight_l0[" << +i << "][0]: " << +hevcSliceControl->delta_chroma_weight_l0[i][0] << std::endl;
        oss << "delta_chroma_weight_l0[" << +i << "][1]: " << +hevcSliceControl->delta_chroma_weight_l0[i][1] << std::endl;
        oss << "delta_chroma_weight_l1[" << +i << "][0]: " << +hevcSliceControl->delta_chroma_weight_l1[i][0] << std::endl;
        oss << "delta_chroma_weight_l1[" << +i << "][1]: " << +hevcSliceControl->delta_chroma_weight_l1[i][1] << std::endl;
    }

    oss << "five_minus_max_num_merge_cand: " << +hevcSliceControl->five_minus_max_num_merge_cand << std::endl;
    oss << "num_entry_point_offsets: " << +hevcSliceControl->num_entry_point_offsets << std::endl;
    oss << "EntryOffsetToSubsetArray: " << +hevcSliceControl->EntryOffsetToSubsetArray << std::endl;

    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_EXT_SLICE_PARAMS &cr)
{
    const CODEC_HEVC_EXT_SLICE_PARAMS *hevcExtSliceControl = &cr;

    // luma_offset[2][15]
    for (uint8_t i = 0; i < 15; i++)
    {
        oss << "luma_offset_l0[" << +i << "]: " << +hevcExtSliceControl->luma_offset_l0[i] << std::endl;
        oss << "luma_offset_l1[" << +i << "]: " << +hevcExtSliceControl->luma_offset_l1[i] << std::endl;
    }

    // chroma_offset[2][15][2]
    for (uint8_t i = 0; i < 15; i++)
    {
        oss << "ChromaOffsetL0[" << +i << "][0]: " << +hevcExtSliceControl->ChromaOffsetL0[i][0] << std::endl;
        oss << "ChromaOffsetL0[" << +i << "][1]: " << +hevcExtSliceControl->ChromaOffsetL0[i][1] << std::endl;
        oss << "ChromaOffsetL1[" << +i << "][0]: " << +hevcExtSliceControl->ChromaOffsetL1[i][0] << std::endl;
        oss << "ChromaOffsetL1[" << +i << "][1]: " << +hevcExtSliceControl->ChromaOffsetL1[i][1] << std::endl;
    }

    oss << "cu_chroma_qp_offset_enabled_flag: " << +hevcExtSliceControl->cu_chroma_qp_offset_enabled_flag << std::endl;

    // scc slice parameters
    oss << "slice_act_y_qp_offset: " << +hevcExtSliceControl->slice_act_y_qp_offset << std::endl;
    oss << "slice_act_cb_qp_offset: " << +hevcExtSliceControl->slice_act_cb_qp_offset << std::endl;
    oss << "slice_act_cr_qp_offset: " << +hevcExtSliceControl->slice_act_cr_qp_offset << std::endl;
    oss << "use_integer_mv_flag: " << +hevcExtSliceControl->use_integer_mv_flag << std::endl;

    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODECHAL_HEVC_IQ_MATRIX_PARAMS &cr)
{
    const CODECHAL_HEVC_IQ_MATRIX_PARAMS *iqParams = &cr;

    uint32_t idx;
    uint32_t idx2;

    // 4x4 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "Qmatrix_HEVC_ucScalingLists0[" << +idx2 << "]:" << std::endl;

        oss << "ucScalingLists0[" << +idx2 << "]:";
        for (uint8_t i = 0; i < 16; i++)
            oss << +iqParams->ucScalingLists0[idx2][i] << " ";
        oss << std::endl;
    }

    // 8x8 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "ucScalingLists1[" << +idx2 << "]:" << std::endl;

        for (idx = 0; idx < 64; idx += 8)
        {
            oss << "ucScalingLists1[" << +idx / 8 << "]:" << std::endl;
            for (uint8_t i = 0; i < 8; i++)
                oss << +iqParams->ucScalingLists1[idx2][idx + i] << " ";
            oss << std::endl;
        }
    }

    // 16x16 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        oss << "ucScalingLists2[" << +idx2 << "]:" << std::endl;

        for (idx = 0; idx < 64; idx += 8)
        {
            oss << "ucScalingLists2[" << +idx / 8 << "]:" << std::endl;
            for (uint8_t i = 0; i < 8; i++)
                oss << +iqParams->ucScalingLists2[idx2][idx + i] << " ";
            oss << std::endl;
        }
    }

    // 32x32 block
    for (idx2 = 0; idx2 < 2; idx2++)
    {
        oss << "ucScalingLists3[" << +idx2 << "]:" << std::endl;

        for (idx = 0; idx < 64; idx += 8)
        {
            oss << "ucScalingLists3[" << +idx / 8 << "]:" << std::endl;
            for (uint8_t i = 0; i < 8; i++)
                oss << +iqParams->ucScalingLists3[idx2][idx + i] << " ";
            oss << std::endl;
        }
    }

    // DC16x16 block
    oss << "ucScalingListDCCoefSizeID2: ";
    for (uint8_t i = 0; i < 6; i++)
        oss << +iqParams->ucScalingListDCCoefSizeID2[i] << " ";

    oss << std::endl;

    // DC32x32 block
    oss << "ucScalingListDCCoefSizeID3: ";
    oss << +iqParams->ucScalingListDCCoefSizeID3[0] << " " << +iqParams->ucScalingListDCCoefSizeID3[1] << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_HEVC_SUBSET_PARAMS &cr)
{
    const CODEC_HEVC_SUBSET_PARAMS *subsetsParams = &cr;

    for (uint16_t i = 0; i < 440; i++)
    {
        oss << "entry_point_offset_minus1[" << +i << "]: " << +subsetsParams->entry_point_offset_minus1[i] << std::endl;
    }

    return oss;
}

void DumpDecodeHevcPicParams(PCODEC_HEVC_PIC_PARAMS picParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *picParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeHevcExtPicParams(PCODEC_HEVC_EXT_PIC_PARAMS extPicParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *extPicParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeHevcSccPicParams(PCODEC_HEVC_SCC_PIC_PARAMS sccPicParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *sccPicParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeHevcSliceParams(PCODEC_HEVC_SLICE_PARAMS sliceParams, uint32_t numSlices, std::string fileName, bool shortFormatInUse)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    if (numSlices > 1)
    {
        oss << "Total Num: " << numSlices << std::endl;
        oss << std::endl;
    }

    for (uint16_t i = 0; i < numSlices; i++)
    {
        if (numSlices > 1)
        {
            oss << "---------Index = " << i << "---------" << std::endl;
        }

        if (shortFormatInUse)
        {
            oss << reinterpret_cast<CODEC_HEVC_SF_SLICE_PARAMS &>(sliceParams[i]) << std::endl;
        }
        else
        {
            oss << sliceParams[i] << std::endl;
        }
    }

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeHevcExtSliceParams(PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams, uint32_t numSlices, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    if (numSlices > 1)
    {
        oss << "Total Num: " << numSlices << std::endl;
        oss << std::endl;
    }

    for (uint16_t i = 0; i < numSlices; i++)
    {
        if (numSlices > 1)
        {
            oss << "---------Index = " << i << "---------" << std::endl;
        }

        oss << extSliceParams[i] << std::endl;
    }    

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeHevcIQParams(PCODECHAL_HEVC_IQ_MATRIX_PARAMS iqParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *iqParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeHevcSubsetParams(PCODEC_HEVC_SUBSET_PARAMS subsetsParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *subsetsParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

// VP9 Decode
std::ostream &operator<<(std::ostream &oss, const CODEC_VP9_PIC_PARAMS &cr)
{
    const CODEC_VP9_PIC_PARAMS *picParams = &cr;

    oss << "CurrPic FrameIdx: " << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << +picParams->CurrPic.PicFlags << std::endl;

    for (uint8_t i = 0; i < 8; ++i)
    {
        oss << "RefFrameList[" << +i << "] FrameIdx:" << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList[" << +i << "] PicFlags:" << +picParams->RefFrameList[i].PicFlags << std::endl;
    }
    oss << "FrameWidthMinus1: " << +picParams->FrameWidthMinus1 << std::endl;
    oss << "FrameHeightMinus1: " << +picParams->FrameHeightMinus1 << std::endl;
    oss << "PicFlags value: " << +picParams->PicFlags.value << std::endl;
    oss << "frame_type: " << +picParams->PicFlags.fields.frame_type << std::endl;
    oss << "show_frame: " << +picParams->PicFlags.fields.show_frame << std::endl;
    oss << "error_resilient_mode: " << +picParams->PicFlags.fields.error_resilient_mode << std::endl;
    oss << "intra_only: " << +picParams->PicFlags.fields.intra_only << std::endl;
    oss << "LastRefIdx: " << +picParams->PicFlags.fields.LastRefIdx << std::endl;
    oss << "LastRefSignBias: " << +picParams->PicFlags.fields.LastRefSignBias << std::endl;
    oss << "GoldenRefIdx: " << +picParams->PicFlags.fields.GoldenRefIdx << std::endl;
    oss << "GoldenRefSignBias: " << +picParams->PicFlags.fields.GoldenRefSignBias << std::endl;
    oss << "AltRefIdx: " << +picParams->PicFlags.fields.AltRefIdx << std::endl;
    oss << "AltRefSignBias: " << +picParams->PicFlags.fields.AltRefSignBias << std::endl;
    oss << "allow_high_precision_mv: " << +picParams->PicFlags.fields.allow_high_precision_mv << std::endl;
    oss << "mcomp_filter_type: " << +picParams->PicFlags.fields.mcomp_filter_type << std::endl;
    oss << "frame_parallel_decoding_mode: " << +picParams->PicFlags.fields.frame_parallel_decoding_mode << std::endl;
    oss << "segmentation_enabled: " << +picParams->PicFlags.fields.segmentation_enabled << std::endl;
    oss << "segmentation_temporal_update: " << +picParams->PicFlags.fields.segmentation_temporal_update << std::endl;
    oss << "segmentation_update_map: " << +picParams->PicFlags.fields.segmentation_update_map << std::endl;
    oss << "reset_frame_context: " << +picParams->PicFlags.fields.reset_frame_context << std::endl;
    oss << "refresh_frame_context: " << +picParams->PicFlags.fields.refresh_frame_context << std::endl;
    oss << "frame_context_idx: " << +picParams->PicFlags.fields.frame_context_idx << std::endl;
    oss << "LosslessFlag: " << +picParams->PicFlags.fields.LosslessFlag << std::endl;
    oss << "ReservedField: " << +picParams->PicFlags.fields.ReservedField << std::endl;
    oss << "filter_level: " << +picParams->filter_level << std::endl;
    oss << "sharpness_level: " << +picParams->sharpness_level << std::endl;
    oss << "log2_tile_rows: " << +picParams->log2_tile_rows << std::endl;
    oss << "log2_tile_columns: " << +picParams->log2_tile_columns << std::endl;
    oss << "UncompressedHeaderLengthInBytes: " << +picParams->UncompressedHeaderLengthInBytes << std::endl;
    oss << "FirstPartitionSize: " << +picParams->FirstPartitionSize << std::endl;
    oss << "profile: " << +picParams->profile << std::endl;
    oss << "BitDepthMinus8: " << +picParams->BitDepthMinus8 << std::endl;
    oss << "subsampling_x: " << +picParams->subsampling_x << std::endl;
    oss << "subsampling_y: " << +picParams->subsampling_y << std::endl;

    for (uint8_t i = 0; i < 7; ++i)
    {
        oss << "SegTreeProbs[" << +i << "]: " << +picParams->SegTreeProbs[i] << std::endl;
    }
    for (uint8_t i = 0; i < 3; ++i)
    {
        oss << "SegPredProbs[" << +i << "]: " << +picParams->SegPredProbs[i] << std::endl;
    }
    oss << "BSBytesInBuffer: " << +picParams->BSBytesInBuffer << std::endl;
    oss << "StatusReportFeedbackNumber: " << +picParams->StatusReportFeedbackNumber << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_VP9_SLICE_PARAMS &cr)
{
    const CODEC_VP9_SLICE_PARAMS *slcParams = &cr;

    oss << "BSNALunitDataLocation: " << +slcParams->BSNALunitDataLocation << std::endl;
    oss << "SliceBytesInBuffer: " << +slcParams->SliceBytesInBuffer << std::endl;
    oss << "wBadSliceChopping: " << +slcParams->wBadSliceChopping << std::endl;
    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_VP9_SEGMENT_PARAMS &cr)
{
    const CODEC_VP9_SEGMENT_PARAMS *segmentParams = &cr;

    for (uint8_t i = 0; i < 8; ++i)
    {
        oss << "SegData[" << +i << "] SegmentFlags value: " << +segmentParams->SegData[i].SegmentFlags.value << std::endl;
        oss << "SegData[" << +i << "] SegmentReferenceEnabled: " << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReferenceEnabled << std::endl;
        oss << "SegData[" << +i << "] SegmentReference: " << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReference << std::endl;
        oss << "SegData[" << +i << "] SegmentReferenceSkipped: " << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReferenceSkipped << std::endl;
        oss << "SegData[" << +i << "] ReservedField3: " << +segmentParams->SegData[i].SegmentFlags.fields.ReservedField3 << std::endl;

        for (uint8_t j = 0; j < 4; ++j)
        {
            oss << "SegData[" << +i << "] FilterLevel[" << +j << "]:";
            oss << +segmentParams->SegData[i].FilterLevel[j][0] << " ";
            oss << +segmentParams->SegData[i].FilterLevel[j][1] << std::endl;
        }

        oss << "SegData[" << +i << "] LumaACQuantScale: " << +segmentParams->SegData[i].LumaACQuantScale << std::endl;
        oss << "SegData[" << +i << "] LumaDCQuantScale: " << +segmentParams->SegData[i].LumaDCQuantScale << std::endl;
        oss << "SegData[" << +i << "] ChromaACQuantScale: " << +segmentParams->SegData[i].ChromaACQuantScale << std::endl;
        oss << "SegData[" << +i << "] ChromaDCQuantScale: " << +segmentParams->SegData[i].ChromaDCQuantScale << std::endl;
    }

    return oss;
}

void DumpDecodeVp9PicParams(PCODEC_VP9_PIC_PARAMS picParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *picParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeVp9SliceParams(PCODEC_VP9_SLICE_PARAMS slcParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *slcParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeVp9SegmentParams(PCODEC_VP9_SEGMENT_PARAMS segmentParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *segmentParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

// AV1 Decode
std::ostream &operator<<(std::ostream &oss, const CodecAv1PicParams &cr)
{
    const CodecAv1PicParams *picParams = &cr;

    oss << "CurrPic FrameIdx: " << +picParams->m_currPic.FrameIdx << std::endl;
    oss << "CurrDisplayPic FrameIdx: " << +picParams->m_currDisplayPic.FrameIdx << std::endl;
    oss << "Profile: " << +picParams->m_profile << std::endl;
    oss << "AnchorFrameInsertion: " << +picParams->m_anchorFrameInsertion << std::endl;
    oss << "order_hint_bits_minus_1: " << +picParams->m_orderHintBitsMinus1 << std::endl;
    oss << "BitDepthIdx: " << +picParams->m_bitDepthIdx << std::endl;

    //Sequence Info Flags
    oss << "dwSeqInfoFlags: " << +picParams->m_seqInfoFlags.m_value << std::endl;
    oss << "still_picture: " << +picParams->m_seqInfoFlags.m_fields.m_stillPicture << std::endl;
    oss << "use_128x128_superblock: " << +picParams->m_seqInfoFlags.m_fields.m_use128x128Superblock << std::endl;
    oss << "enable_filter_intra: " << +picParams->m_seqInfoFlags.m_fields.m_enableFilterIntra << std::endl;
    oss << "enable_intra_edge_filter: " << +picParams->m_seqInfoFlags.m_fields.m_enableIntraEdgeFilter << std::endl;
    oss << "enable_interintra_compound: " << +picParams->m_seqInfoFlags.m_fields.m_enableInterintraCompound << std::endl;
    oss << "enable_masked_compound: " << +picParams->m_seqInfoFlags.m_fields.m_enableMaskedCompound << std::endl;
    oss << "enable_dual_filter: " << +picParams->m_seqInfoFlags.m_fields.m_enableDualFilter << std::endl;
    oss << "enable_order_hint: " << +picParams->m_seqInfoFlags.m_fields.m_enableOrderHint << std::endl;
    oss << "enable_jnt_comp: " << +picParams->m_seqInfoFlags.m_fields.m_enableJntComp << std::endl;
    oss << "enable_cdef: " << +picParams->m_seqInfoFlags.m_fields.m_enableCdef << std::endl;
    oss << "mono_chrome: " << +picParams->m_seqInfoFlags.m_fields.m_monoChrome << std::endl;
    oss << "color_range: " << +picParams->m_seqInfoFlags.m_fields.m_colorRange << std::endl;
    oss << "subsampling_x: " << +picParams->m_seqInfoFlags.m_fields.m_subsamplingX << std::endl;
    oss << "subsampling_y: " << +picParams->m_seqInfoFlags.m_fields.m_subsamplingY << std::endl;
    oss << "chroma_sample_position: " << +picParams->m_seqInfoFlags.m_fields.m_chromaSamplePosition << std::endl;
    oss << "film_grain_params_present: " << +picParams->m_seqInfoFlags.m_fields.m_filmGrainParamsPresent << std::endl;

    //frame info
    oss << "dwPicInfoFlags: " << +picParams->m_picInfoFlags.m_value << std::endl;
    oss << "frame_type: " << +picParams->m_picInfoFlags.m_fields.m_frameType << std::endl;
    oss << "show_frame: " << +picParams->m_picInfoFlags.m_fields.m_showFrame << std::endl;
    oss << "showable_frame: " << +picParams->m_picInfoFlags.m_fields.m_showableFrame << std::endl;
    oss << "error_resilient_mode: " << +picParams->m_picInfoFlags.m_fields.m_errorResilientMode << std::endl;
    oss << "disable_cdf_update: " << +picParams->m_picInfoFlags.m_fields.m_disableCdfUpdate << std::endl;
    oss << "allow_screen_content_tools: " << +picParams->m_picInfoFlags.m_fields.m_allowScreenContentTools << std::endl;
    oss << "force_integer_mv: " << +picParams->m_picInfoFlags.m_fields.m_forceIntegerMv << std::endl;
    oss << "allow_intrabc: " << +picParams->m_picInfoFlags.m_fields.m_allowIntrabc << std::endl;
    oss << "use_superres: " << +picParams->m_picInfoFlags.m_fields.m_useSuperres << std::endl;
    oss << "allow_high_precision_mv: " << +picParams->m_picInfoFlags.m_fields.m_allowHighPrecisionMv << std::endl;
    oss << "is_motion_mode_switchable: " << +picParams->m_picInfoFlags.m_fields.m_isMotionModeSwitchable << std::endl;
    oss << "use_ref_frame_mvs: " << +picParams->m_picInfoFlags.m_fields.m_useRefFrameMvs << std::endl;
    oss << "disable_frame_end_update_cdf: " << +picParams->m_picInfoFlags.m_fields.m_disableFrameEndUpdateCdf << std::endl;
    oss << "uniform_tile_spacing_flag: " << +picParams->m_picInfoFlags.m_fields.m_uniformTileSpacingFlag << std::endl;
    oss << "allow_warped_motion: " << +picParams->m_picInfoFlags.m_fields.m_allowWarpedMotion << std::endl;
    oss << "large_scale_tile: " << +picParams->m_picInfoFlags.m_fields.m_largeScaleTile << std::endl;

    oss << "frame_width_minus1: " << +picParams->m_frameWidthMinus1 << std::endl;
    oss << "frame_height_minus1: " << +picParams->m_frameHeightMinus1 << std::endl;

    for (auto i = 0; i < 8; ++i)
    {
        oss << "ref_frame_map[" << +i << "] FrameIdx:" << +picParams->m_refFrameMap[i].FrameIdx << std::endl;
        oss << "ref_frame_map[" << +i << "] PicFlags:" << +picParams->m_refFrameMap[i].PicFlags << std::endl;
    }

    for (auto i = 0; i < 7; i++)
    {
        oss << "ref_frame_idx[" << +i << "]: " << +picParams->m_refFrameIdx[i] << std::endl;
    }

    oss << "primary_ref_frame: " << +picParams->m_primaryRefFrame << std::endl;
    oss << "output_frame_width_in_tiles_minus_1: " << +picParams->m_outputFrameWidthInTilesMinus1 << std::endl;
    oss << "output_frame_height_in_tiles_minus_1: " << +picParams->m_outputFrameHeightInTilesMinus1 << std::endl;

    for (auto i = 0; i < 2; i++)
    {
        oss << "filter_level[" << +i << "]: " << +picParams->m_filterLevel[i] << std::endl;
    }
    oss << "filter_level_u: " << +picParams->m_filterLevelU << std::endl;
    oss << "filter_level_v: " << +picParams->m_filterLevelV << std::endl;

    //Loop Filter Info Flags
    oss << "cLoopFilterInfoFlags value: " << +picParams->m_loopFilterInfoFlags.m_value << std::endl;
    oss << "sharpness_level: " << +picParams->m_loopFilterInfoFlags.m_fields.m_sharpnessLevel << std::endl;
    oss << "mode_ref_delta_enabled: " << +picParams->m_loopFilterInfoFlags.m_fields.m_modeRefDeltaEnabled << std::endl;
    oss << "mode_ref_delta_update: " << +picParams->m_loopFilterInfoFlags.m_fields.m_modeRefDeltaUpdate << std::endl;

    oss << "order_hint: " << +picParams->m_orderHint << std::endl;
    oss << "superres_scale_denominator: " << +picParams->m_superresScaleDenominator << std::endl;
    oss << "interp_filter: " << +picParams->m_interpFilter << std::endl;

    for (auto i = 0; i < 8; i++)
    {
        oss << "ref_deltas[" << +i << "]: " << +picParams->m_refDeltas[i] << std::endl;
    }

    for (auto i = 0; i < 2; i++)
    {
        oss << "mode_deltas[" << +i << "]: " << +picParams->m_modeDeltas[i] << std::endl;
    }

    oss << "base_qindex: " << +picParams->m_baseQindex << std::endl;
    oss << "y_dc_delta_q: " << +picParams->m_yDcDeltaQ << std::endl;
    oss << "u_dc_delta_q: " << +picParams->m_uDcDeltaQ << std::endl;
    oss << "u_ac_delta_q: " << +picParams->m_uAcDeltaQ << std::endl;
    oss << "v_dc_delta_q: " << +picParams->m_vDcDeltaQ << std::endl;
    oss << "v_ac_delta_q: " << +picParams->m_vAcDeltaQ << std::endl;

    // quantization_matrix
    oss << "wQMatrixFlags value: " << +picParams->m_qMatrixFlags.m_value << std::endl;
    oss << "using_qmatrix: " << +picParams->m_qMatrixFlags.m_fields.m_usingQmatrix << std::endl;
    oss << "qm_y: " << +picParams->m_qMatrixFlags.m_fields.m_qmY << std::endl;
    oss << "qm_u: " << +picParams->m_qMatrixFlags.m_fields.m_qmU << std::endl;
    oss << "qm_v: " << +picParams->m_qMatrixFlags.m_fields.m_qmV << std::endl;

    // Mode control flags
    oss << "dwModeControlFlags value: " << +picParams->m_modeControlFlags.m_value << std::endl;
    oss << "delta_q_present_flag: " << +picParams->m_modeControlFlags.m_fields.m_deltaQPresentFlag << std::endl;
    oss << "log2_delta_q_res: " << +picParams->m_modeControlFlags.m_fields.m_log2DeltaQRes << std::endl;
    oss << "delta_lf_present_flag: " << +picParams->m_modeControlFlags.m_fields.m_deltaLfPresentFlag << std::endl;
    oss << "log2_delta_lf_res: " << +picParams->m_modeControlFlags.m_fields.m_log2DeltaLfRes << std::endl;
    oss << "delta_lf_multi: " << +picParams->m_modeControlFlags.m_fields.m_deltaLfMulti << std::endl;
    oss << "tx_mode: " << +picParams->m_modeControlFlags.m_fields.m_txMode << std::endl;
    oss << "reference_mode: " << +picParams->m_modeControlFlags.m_fields.m_referenceMode << std::endl;
    oss << "reduced_tx_set_used: " << +picParams->m_modeControlFlags.m_fields.m_reducedTxSetUsed << std::endl;
    oss << "skip_mode_present: " << +picParams->m_modeControlFlags.m_fields.m_skipModePresent << std::endl;

    // Segmentation
    oss << "enabled: " << +picParams->m_av1SegData.m_enabled << std::endl;
    oss << "update_map: " << +picParams->m_av1SegData.m_updateMap << std::endl;
    oss << "temporal_update: " << +picParams->m_av1SegData.m_temporalUpdate << std::endl;
    oss << "update_data: " << +picParams->m_av1SegData.m_updateData << std::endl;

    for (auto i = 0; i < 8; i++)
    {
        for (auto j = 0; j < 8; j++)
        {
            oss << "feature_data[" << +i << "][" << +j << "]: " << +picParams->m_av1SegData.m_featureData[i][j] << std::endl;
        }
    }
    for (auto i = 0; i < 8; i++)
    {
        oss << "feature_mask[" << +i << "]: " << +picParams->m_av1SegData.m_featureMask[i] << std::endl;
    }

    oss << "tile_cols: " << +picParams->m_tileCols << std::endl;
    for (auto i = 0; i < 63; i++)
    {
        oss << "width_in_sbs_minus_1[" << +i << "]: " << +picParams->m_widthInSbsMinus1[i] << std::endl;
    }
    oss << "tile_rows: " << +picParams->m_tileRows << std::endl;
    for (auto i = 0; i < 63; i++)
    {
        oss << "height_in_sbs_minus_1[" << +i << "]: " << +picParams->m_heightInSbsMinus1[i] << std::endl;
    }

    oss << "tile_count_minus_1: " << +picParams->m_tileCountMinus1 << std::endl;
    oss << "context_update_tile_id: " << +picParams->m_contextUpdateTileId << std::endl;

    oss << "cdef_damping_minus_3: " << +picParams->m_cdefDampingMinus3 << std::endl;
    oss << "cdef_bits: " << +picParams->m_cdefBits << std::endl;
    for (auto i = 0; i < 8; i++)
    {
        oss << "cdef_y_strengths[" << +i << "]: " << +picParams->m_cdefYStrengths[i] << std::endl;
    }
    for (auto i = 0; i < 8; i++)
    {
        oss << "cdef_uv_strengths[" << +i << "]: " << +picParams->m_cdefUvStrengths[i] << std::endl;
    }

    // Loop Restoration Flags
    oss << "LoopRestorationFlags value: " << +picParams->m_loopRestorationFlags.m_value << std::endl;
    oss << "yframe_restoration_type: " << +picParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType << std::endl;
    oss << "cbframe_restoration_type: " << +picParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType << std::endl;
    oss << "crframe_restoration_type: " << +picParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType << std::endl;
    oss << "lr_unit_shift: " << +picParams->m_loopRestorationFlags.m_fields.m_lrUnitShift << std::endl;
    oss << "lr_uv_shift: " << +picParams->m_loopRestorationFlags.m_fields.m_lrUvShift << std::endl;

    for (auto i = 0; i < 7; i++)
    {
        oss << "wm[" << +i << "].wmtype: " << +picParams->m_wm[i].m_wmtype << std::endl;
        for (auto j = 0; j < 8; j++)
        {
            oss << "wm[" << +i << "].wmmat[" << +j << "]: " << +picParams->m_wm[i].m_wmmat[j] << std::endl;
        }
    }

    //Film Grain params
    oss << "apply_grain: " << +picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain << std::endl;
    oss << "chroma_scaling_from_luma: " << +picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_chromaScalingFromLuma << std::endl;
    oss << "grain_scaling_minus_8: " << +picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_grainScalingMinus8 << std::endl;
    oss << "ar_coeff_lag: " << +picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_arCoeffLag << std::endl;
    oss << "ar_coeff_shift_minus_6: " << +picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_arCoeffShiftMinus6 << std::endl;
    oss << "grain_scale_shift: " << +picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_grainScaleShift << std::endl;
    oss << "overlap_flag: " << +picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_overlapFlag << std::endl;
    oss << "clip_to_restricted_range: " << +picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_clipToRestrictedRange << std::endl;

    oss << "random_seed: " << +picParams->m_filmGrainParams.m_randomSeed << std::endl;
    oss << "num_y_points: " << +picParams->m_filmGrainParams.m_numYPoints << std::endl;
    for (auto i = 0; i < 14; i++)
    {
        oss << "point_y_value[" << +i << "]: " << +picParams->m_filmGrainParams.m_pointYValue[i] << std::endl;
    }
    for (auto i = 0; i < 14; i++)
    {
        oss << "point_y_scaling[" << +i << "]: " << +picParams->m_filmGrainParams.m_pointYScaling[i] << std::endl;
    }

    oss << "num_cb_points: " << +picParams->m_filmGrainParams.m_numCbPoints << std::endl;
    for (auto i = 0; i < 10; i++)
    {
        oss << "point_cb_value[" << +i << "]: " << +picParams->m_filmGrainParams.m_pointCbValue[i] << std::endl;
    }
    for (auto i = 0; i < 10; i++)
    {
        oss << "point_cb_scaling[" << +i << "]: " << +picParams->m_filmGrainParams.m_pointCbScaling[i] << std::endl;
    }

    oss << "num_cr_points: " << +picParams->m_filmGrainParams.m_numCrPoints << std::endl;
    for (auto i = 0; i < 10; i++)
    {
        oss << "point_cr_value[" << +i << "]: " << +picParams->m_filmGrainParams.m_pointCrValue[i] << std::endl;
    }
    for (auto i = 0; i < 10; i++)
    {
        oss << "point_cr_scaling[" << +i << "]: " << +picParams->m_filmGrainParams.m_pointCrScaling[i] << std::endl;
    }

    for (auto i = 0; i < 24; i++)
    {
        oss << "ar_coeffs_y[" << +i << "]: " << +picParams->m_filmGrainParams.m_arCoeffsY[i] << std::endl;
    }
    for (auto i = 0; i < 25; i++)
    {
        oss << "ar_coeffs_cb[" << +i << "]: " << +picParams->m_filmGrainParams.m_arCoeffsCb[i] << std::endl;
    }
    for (auto i = 0; i < 25; i++)
    {
        oss << "ar_coeffs_cr[" << +i << "]: " << +picParams->m_filmGrainParams.m_arCoeffsCr[i] << std::endl;
    }

    oss << "cb_mult: " << +picParams->m_filmGrainParams.m_cbMult << std::endl;
    oss << "cb_luma_mult: " << +picParams->m_filmGrainParams.m_cbLumaMult << std::endl;
    oss << "cb_offset: " << +picParams->m_filmGrainParams.m_cbOffset << std::endl;
    oss << "cr_mult: " << +picParams->m_filmGrainParams.m_crMult << std::endl;
    oss << "cr_luma_mult: " << +picParams->m_filmGrainParams.m_crLumaMult << std::endl;
    oss << "cr_offset: " << +picParams->m_filmGrainParams.m_crOffset << std::endl;
    oss << "StatusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;

    //Driver internal
    oss << "losslessMode: " << +picParams->m_losslessMode << std::endl;
    oss << "superResUpscaledWidthMinus1: " << +picParams->m_superResUpscaledWidthMinus1 << std::endl;
    oss << "superResUpscaledHeightMinus1: " << +picParams->m_superResUpscaledHeightMinus1 << std::endl;
    for (auto i = 0; i < 8; i++)
    {
        oss << "activeRefBitMaskMfmv[" << +i << "]: " << +picParams->m_activeRefBitMaskMfmv[i] << std::endl;
    }

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CodecAv1TileParams &cr)
{
    const CodecAv1TileParams *tileParams = &cr;

    oss << "BSTileDataLocation" << +tileParams->m_bsTileDataLocation << std::endl;
    oss << "BSTileBytesInBuffer" << +tileParams->m_bsTileBytesInBuffer << std::endl;
    oss << "wBadBSBufferChopping" << +tileParams->m_badBSBufferChopping << std::endl;
    oss << "tile_row" << +tileParams->m_tileRow << std::endl;
    oss << "tile_column" << +tileParams->m_tileColumn << std::endl;
    oss << "anchor_frame_idx" << +tileParams->m_anchorFrameIdx.FrameIdx << std::endl;
    oss << "BSTilePayloadSizeInBytes" << +tileParams->m_bsTilePayloadSizeInBytes << std::endl;

    oss << "--------- Intel Profile Specific ---------" << std::endl;
    oss << "tile_index" << +tileParams->m_tileIndex << std::endl;
    oss << "StartTileIdx" << +tileParams->m_startTileIdx << std::endl;
    oss << "EndTileIdx" << +tileParams->m_endTileIdx << std::endl;

    return oss;
}

void DumpDecodeAv1PicParams(CodecAv1PicParams *picParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *picParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeAv1TileParams(CodecAv1TileParams *tileParams, uint32_t tileNum, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    if (tileNum > 1)
    {
        oss << "Total Num: " << tileNum << std::endl;
        oss << std::endl;
    }

    for (uint16_t i = 0; i < tileNum; i++)
    {
        if (tileNum > 1)
        {
            oss << "---------Index = " << i << "---------" << std::endl;
        }

        oss << tileParams[i] << std::endl;
    }

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

// JPEG Decode
std::ostream &operator<<(std::ostream &oss, const CodecDecodeJpegPicParams &cr)
{
    const CodecDecodeJpegPicParams *picParams = &cr;

    oss << "destPic.FrameIdx: " << +picParams->m_destPic.FrameIdx << std::endl;
    oss << "destPic.PicFlags: " << +picParams->m_destPic.PicFlags << std::endl;
    oss << "frameWidth: " << +picParams->m_frameWidth << std::endl;
    oss << "frameHeight: " << +picParams->m_frameHeight << std::endl;
    oss << "numCompInFrame: " << +picParams->m_numCompInFrame << std::endl;

    // componentIdentifier[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "componentIdentifier[" << +i << "]: " << +picParams->m_componentIdentifier[i] << std::endl;
    }

    // quantTableSelector[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "quantTableSelector[" << +i << "]: " << +picParams->m_quantTableSelector[i] << std::endl;
    }

    oss << "chromaType: " << +picParams->m_chromaType << std::endl;
    oss << "rotation: " << +picParams->m_rotation << std::endl;
    oss << "totalScans: " << +picParams->m_totalScans << std::endl;
    oss << "interleavedData: " << +picParams->m_interleavedData << std::endl;
    oss << "reserved: " << +picParams->m_reserved << std::endl;
    oss << "statusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CodecDecodeJpegScanParameter &cr)
{
    const CodecDecodeJpegScanParameter *scanParams = &cr;

    oss << "NumScans: " << +scanParams->NumScans << std::endl;

    // ScanHeader[jpegNumComponent]
    for (uint32_t i = 0; i < jpegNumComponent; ++i)
    {
        oss << "ScanHeader[" << +i << "].NumComponents: " << +scanParams->ScanHeader[i].NumComponents << std::endl;

        // ComponentSelector[jpegNumComponent]
        for (uint32_t j = 0; j < jpegNumComponent; ++j)
        {
            oss << "ScanHeader[" << +i << "].ComponentSelector[" << +j << "]: " << +scanParams->ScanHeader[i].ComponentSelector[j] << std::endl;
        }

        // DcHuffTblSelector[jpegNumComponent]
        for (uint32_t j = 0; j < jpegNumComponent; ++j)
        {
            oss << "ScanHeader[" << +i << "].DcHuffTblSelector[" << +j << "]: " << +scanParams->ScanHeader[i].DcHuffTblSelector[j] << std::endl;
        }

        // AcHuffTblSelector[jpegNumComponent]
        for (uint32_t j = 0; j < jpegNumComponent; ++j)
        {
            oss << "ScanHeader[" << +i << "].AcHuffTblSelector[" << +j << "]: " << +scanParams->ScanHeader[i].AcHuffTblSelector[j] << std::endl;
        }

        oss << "ScanHeader[" << +i << "].RestartInterval: " << +scanParams->ScanHeader[i].RestartInterval << std::endl;
        oss << "ScanHeader[" << +i << "].MCUCount: " << +scanParams->ScanHeader[i].MCUCount << std::endl;
        oss << "ScanHeader[" << +i << "].ScanHoriPosition: " << +scanParams->ScanHeader[i].ScanHoriPosition << std::endl;
        oss << "ScanHeader[" << +i << "].ScanVertPosition: " << +scanParams->ScanHeader[i].ScanVertPosition << std::endl;
        oss << "ScanHeader[" << +i << "].DataOffset: " << +scanParams->ScanHeader[i].DataOffset << std::endl;
        oss << "ScanHeader[" << +i << "].DataLength: " << +scanParams->ScanHeader[i].DataLength << std::endl;
    }

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODECHAL_DECODE_JPEG_HUFFMAN_TABLE &cr)
{
    const CODECHAL_DECODE_JPEG_HUFFMAN_TABLE *huffmanTable = &cr;

    // HuffTable[JPEG_MAX_NUM_HUFF_TABLE_INDEX]
    for (uint32_t i = 0; i < JPEG_MAX_NUM_HUFF_TABLE_INDEX; ++i)
    {
        // DC_BITS[JPEG_NUM_HUFF_TABLE_DC_BITS]
        oss << "HuffTable[" << +i << "].DC_BITS[0-" << (JPEG_NUM_HUFF_TABLE_DC_BITS - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_DC_BITS; ++j)
        {
            oss << +huffmanTable->HuffTable[i].DC_BITS[j] << " ";
            if (j % 6 == 5 || j == JPEG_NUM_HUFF_TABLE_DC_BITS - 1)
            {
                oss << std::endl;
            }
        }
        // DC_HUFFVAL[JPEG_NUM_HUFF_TABLE_DC_HUFFVAL]
        oss << "HuffTable[" << +i << "].DC_HUFFVAL[0-" << (JPEG_NUM_HUFF_TABLE_DC_HUFFVAL - 1) << "]: " << std::endl;
        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_DC_HUFFVAL; ++j)
        {
            oss << +huffmanTable->HuffTable[i].DC_HUFFVAL[j] << ' ';
            if (j % 6 == 5 || j == JPEG_NUM_HUFF_TABLE_DC_HUFFVAL - 1)
            {
                oss << std::endl;
            }
        }
        // AC_BITS[JPEG_NUM_HUFF_TABLE_AC_BITS]
        oss << "HuffTable[" << +i << "].AC_BITS[0-" << (JPEG_NUM_HUFF_TABLE_AC_BITS - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_AC_BITS; ++j)
        {
            oss << +huffmanTable->HuffTable[i].AC_BITS[j] << ' ';
            if (j % 8 == 7 || j == JPEG_NUM_HUFF_TABLE_AC_BITS - 1)
            {
                oss << std::endl;
            }
        }

        // AC_HUFFVAL[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL]
        oss << "HuffTable[" << +i << "].AC_HUFFVAL[0-" << (JPEG_NUM_HUFF_TABLE_AC_HUFFVAL - 1) << "]: " << std::endl;

        for (uint32_t j = 0; j < JPEG_NUM_HUFF_TABLE_AC_HUFFVAL; ++j)
        {
            oss << +huffmanTable->HuffTable[i].AC_HUFFVAL[j] << ' ';
            if (j % 9 == 8 || j == JPEG_NUM_HUFF_TABLE_AC_HUFFVAL - 1)
            {
                oss << std::endl;
            }
        }
    }

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CodecJpegQuantMatrix &cr)
{
    const CodecJpegQuantMatrix *iqParams = &cr;

    for (uint32_t j = 0; j < jpegNumComponent; j++)
    {
        oss << "Qmatrix " << +j << ": " << std::endl;

        for (int8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]:";
            for (uint8_t k = 0; k < 8; k++)
                oss << +iqParams->m_quantMatrix[j][i + k] << " ";
            oss << std::endl;
        }
    }

    return oss;
}

void DumpDecodeJpegPicParams(CodecDecodeJpegPicParams *picParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *picParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeJpegScanParams(CodecDecodeJpegScanParameter *scanParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *scanParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeJpegHuffmanParams(PCODECHAL_DECODE_JPEG_HUFFMAN_TABLE huffmanTable, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *huffmanTable << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeJpegIqParams(CodecJpegQuantMatrix *iqParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *iqParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

// MPEG2 Decode
std::ostream &operator<<(std::ostream &oss, const CodecDecodeMpeg2PicParams &cr)
{
    const CodecDecodeMpeg2PicParams *picParams = &cr;

    oss << "m_currPic FrameIdx: " << +picParams->m_currPic.FrameIdx << std::endl;
    oss << "m_currPic PicFlags: " << +picParams->m_currPic.PicFlags << std::endl;
    oss << "m_forwardRefIdx: " << +picParams->m_forwardRefIdx << std::endl;
    oss << "m_backwardRefIdx: " << +picParams->m_backwardRefIdx << std::endl;
    oss << "m_topFieldFirst: " << +picParams->m_topFieldFirst << std::endl;
    oss << "m_secondField: " << +picParams->m_secondField << std::endl;
    oss << "m_statusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;
    // union w0
    oss << "w0 m_value: " << +picParams->W0.m_value << std::endl;
    oss << "m_scanOrder: " << +picParams->W0.m_scanOrder << std::endl;
    oss << "m_intraVlcFormat: " << +picParams->W0.m_intraVlcFormat << std::endl;
    oss << "m_quantizerScaleType: " << +picParams->W0.m_quantizerScaleType << std::endl;
    oss << "m_concealmentMVFlag: " << +picParams->W0.m_concealmentMVFlag << std::endl;
    oss << "m_frameDctPrediction: " << +picParams->W0.m_frameDctPrediction << std::endl;
    oss << "m_topFieldFirst: " << +picParams->W0.m_topFieldFirst << std::endl;
    oss << "m_intraDCPrecision: " << +picParams->W0.m_intraDCPrecision << std::endl;
    // union w1
    oss << "w1 m_value: " << +picParams->W1.m_value << std::endl;
    oss << "m_fcode11: " << +picParams->W1.m_fcode11 << std::endl;
    oss << "m_fcode10: " << +picParams->W1.m_fcode10 << std::endl;
    oss << "m_fcode01: " << +picParams->W1.m_fcode01 << std::endl;
    oss << "m_fcode00: " << +picParams->W1.m_fcode00 << std::endl;
    oss << "m_horizontalSize: " << +picParams->m_horizontalSize << std::endl;
    oss << "m_verticalSize: " << +picParams->m_verticalSize << std::endl;
    oss << "m_pictureCodingType: " << +picParams->m_pictureCodingType << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CodecDecodeMpeg2SliceParams &cr)
{
    const CodecDecodeMpeg2SliceParams *sliceControl = &cr;

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
    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CodecDecodeMpeg2MbParams &cr)
{
    const CodecDecodeMpeg2MbParams *mbParams = &cr;

    oss << "m_mbAddr: " << +mbParams->m_mbAddr << std::endl;
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

    // NumCoeff[CODEC_NUM_BLOCK_PER_MB]
    for (uint16_t i = 0; i < CODEC_NUM_BLOCK_PER_MB; ++i)
    {
        oss << "m_numCoeff[" << +i << "]: " << +mbParams->m_numCoeff[i] << std::endl;
    }

    // motion_vectors[8],printing them in 4 value chunks per line
    for (uint8_t i = 0; i < 2; ++i)
    {
        oss << "m_motionVectors[" << +i * 4 << "-" << (+i * 4) + 3 << "]: ";
        for (uint8_t j = 0; j < 4; j++)
            oss << +mbParams->m_motionVectors[i * 4 + j] << " ";
        oss << std::endl;
    }

    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CodecMpeg2IqMatrix &cr)
{
    const CodecMpeg2IqMatrix *iqParams = &cr;

    if (iqParams->m_loadIntraQuantiserMatrix)
    {
        oss << "intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +iqParams->m_intraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }

    if (iqParams->m_loadNonIntraQuantiserMatrix)
    {
        oss << "non_intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +iqParams->m_nonIntraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }

    if (iqParams->m_loadChromaIntraQuantiserMatrix)
    {
        oss << "chroma_intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +iqParams->m_chromaIntraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }

    if (iqParams->m_loadChromaNonIntraQuantiserMatrix)
    {
        oss << "chroma_non_intra_QmatrixData:" << std::endl;

        for (uint8_t i = 0; i < 56; i += 8)
        {
            oss << "Qmatrix[" << +i / 8 << "]: ";
            for (uint8_t j = 0; j < 8; j++)
                oss << +iqParams->m_chromaNonIntraQuantiserMatrix[i + j] << " ";
            oss << std::endl;
        }
    }

    return oss;
}

void DumpDecodeMpeg2PicParams(CodecDecodeMpeg2PicParams *picParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *picParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeMpeg2SliceParams(CodecDecodeMpeg2SliceParams *sliceParams, uint32_t numSlices, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    if (numSlices > 1)
    {
        oss << "Total Num: " << numSlices << std::endl;
        oss << std::endl;
    }

    for (uint32_t i = 0; i < numSlices; i++)
    {
        if (numSlices > 1)
        {
            oss << "---------Index = " << i << "---------" << std::endl;
        }

        oss << sliceParams[i] << std::endl;
    }

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeMpeg2MbParams(CodecDecodeMpeg2MbParams *mbParams, uint32_t numMbs, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    if (numMbs > 1)
    {
        oss << "Total Num: " << numMbs << std::endl;
        oss << std::endl;
    }

    for (uint32_t i = 0; i < numMbs; i++)
    {
        if (numMbs > 1)
        {
            oss << "---------Index = " << i << "---------" << std::endl;
        }

        oss << mbParams[i] << std::endl;
    }

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeMpeg2IqParams(CodecMpeg2IqMatrix *iqParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *iqParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

// VP8 Decode
std::ostream &operator<<(std::ostream &oss, const CODEC_VP8_PIC_PARAMS &cr)
{
    const CODEC_VP8_PIC_PARAMS *picParams = &cr;

    oss << "CurrPic FrameIdx: " << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << +picParams->CurrPic.PicFlags << std::endl;
    oss << "wFrameWidthInMbsMinus1: " << +picParams->wFrameWidthInMbsMinus1 << std::endl;
    oss << "wFrameHeightInMbsMinus1: " << +picParams->wFrameHeightInMbsMinus1 << std::endl;
    oss << "ucCurrPicIndex: " << +picParams->ucCurrPicIndex << std::endl;
    oss << "ucLastRefPicIndex: " << +picParams->ucLastRefPicIndex << std::endl;
    oss << "ucGoldenRefPicIndex: " << +picParams->ucGoldenRefPicIndex << std::endl;
    oss << "ucAltRefPicIndex: " << +picParams->ucAltRefPicIndex << std::endl;
    oss << "ucDeblockedPicIndex: " << +picParams->ucDeblockedPicIndex << std::endl;
    oss << "ucReserved8Bits: " << +picParams->ucReserved8Bits << std::endl;
    oss << "wPicFlags: " << +picParams->wPicFlags << std::endl;
    oss << "key_frame: " << +picParams->key_frame << std::endl;
    oss << "version: " << +picParams->version << std::endl;
    oss << "segmentation_enabled: " << +picParams->segmentation_enabled << std::endl;
    oss << "update_mb_segmentation_map: " << +picParams->update_mb_segmentation_map << std::endl;
    oss << "update_segment_feature_data: " << +picParams->update_segment_feature_data << std::endl;
    oss << "filter_type: " << +picParams->filter_type << std::endl;
    oss << "sign_bias_golden: " << +picParams->sign_bias_golden << std::endl;
    oss << "sign_bias_alternate: " << +picParams->sign_bias_alternate << std::endl;
    oss << "mb_no_coeff_skip: " << +picParams->mb_no_coeff_skip << std::endl;
    oss << "mode_ref_lf_delta_update: " << +picParams->mode_ref_lf_delta_update << std::endl;
    oss << "CodedCoeffTokenPartition: " << +picParams->CodedCoeffTokenPartition << std::endl;
    oss << "LoopFilterDisable: " << +picParams->LoopFilterDisable << std::endl;
    oss << "loop_filter_adj_enable: " << +picParams->loop_filter_adj_enable << std::endl;

    for (uint8_t i = 0; i < 4; ++i)
    {
        oss << "ucLoopFilterLevel[" << +i << "]: " << +picParams->ucLoopFilterLevel[i] << std::endl;
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        oss << "cRefLfDelta[" << +i << "]: " << +picParams->cRefLfDelta[i] << std::endl;
    }

    for (uint8_t i = 0; i < 4; ++i)
    {
        oss << "cModeLfDelta[" << +i << "]: " << +picParams->cModeLfDelta[i] << std::endl;
    }
    oss << "ucSharpnessLevel: " << +picParams->ucSharpnessLevel << std::endl;

    for (uint8_t i = 0; i < 3; ++i)
    {
        oss << "cMbSegmentTreeProbs[" << +i << "]: " << +picParams->cMbSegmentTreeProbs[i] << std::endl;
    }
    oss << "ucProbSkipFalse: " << +picParams->ucProbSkipFalse << std::endl;
    oss << "ucProbIntra: " << +picParams->ucProbIntra << std::endl;
    oss << "ucProbLast: " << +picParams->ucProbLast << std::endl;
    oss << "ucProbGolden: " << +picParams->ucProbGolden << std::endl;

    for (uint8_t i = 0; i < 4; ++i)
    {
        oss << "ucYModeProbs[" << +i << "]: " << +picParams->ucYModeProbs[i] << std::endl;
    }

    for (uint8_t i = 0; i < 3; ++i)
    {
        oss << "ucUvModeProbs[" << +i << "]: " << +picParams->ucUvModeProbs[i] << std::endl;
    }
    oss << "ucReserved8Bits1: " << +picParams->ucReserved8Bits1 << std::endl;
    oss << "ucP0EntropyCount: " << +picParams->ucP0EntropyCount << std::endl;
    oss << "ucP0EntropyValue: " << +picParams->ucP0EntropyValue << std::endl;
    oss << "uiP0EntropyRange: " << +picParams->uiP0EntropyRange << std::endl;
    oss << "uiFirstMbByteOffset: " << +picParams->uiFirstMbByteOffset << std::endl;

    for (uint8_t i = 0; i < 2; ++i)
    {
        for (uint8_t j = 0; j < CODEC_VP8_MVP_COUNT; ++j)
        {
            oss << "ucMvUpdateProb[" << +i << "][" << +j << "]: " << +picParams->ucMvUpdateProb[i][j] << std::endl;
        }
    }
    for (uint8_t i = 0; i < CODEC_VP8_MAX_PARTITION_NUMBER; ++i)
    {
        oss << "uiPartitionSize[" << +i << "]: " << +picParams->uiPartitionSize[i] << std::endl;
    }
    oss << "uiStatusReportFeedbackNumber: " << +picParams->uiStatusReportFeedbackNumber << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_VP8_SLICE_PARAMS &cr)
{
    const CODEC_VP8_SLICE_PARAMS *sliceParams = &cr;

    oss << "BSNALunitDataLocation: " << +sliceParams->BSNALunitDataLocation << std::endl;
    oss << "SliceBytesInBuffer: " << +sliceParams->SliceBytesInBuffer << std::endl;
    oss << std::endl;

    return oss;
}

std::ostream &operator<<(std::ostream &oss, const CODEC_VP8_IQ_MATRIX_PARAMS &cr)
{
    const CODEC_VP8_IQ_MATRIX_PARAMS *iqParams = &cr;

    for (uint8_t i = 0; i < 4; ++i)
    {
        for (uint8_t j = 0; j < 6; ++j)
        {
            oss << "quantization_values[" << +i << "][" << +j << "]: " << +iqParams->quantization_values[i][j] << std::endl;
        }
    }

    return oss;
}

void DumpDecodeVp8PicParams(PCODEC_VP8_PIC_PARAMS picParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *picParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeVp8SliceParams(PCODEC_VP8_SLICE_PARAMS sliceParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *sliceParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

void DumpDecodeVp8IqParams(PCODEC_VP8_IQ_MATRIX_PARAMS iqParams, std::string fileName)
{
    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << *iqParams << std::endl;

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
}

CodechalDebugInterface::CodechalDebugInterface()
{
    MOS_ZeroMemory(m_fileName, sizeof(m_fileName));
    MOS_ZeroMemory(m_path, sizeof(m_path));

    m_dumpYUVSurfaceLegacy = [this](
                           PMOS_SURFACE              surface,
                           const char               *attrName,
                           const char               *surfName,
                           CODECHAL_MEDIA_STATE_TYPE mediaState,
                           uint32_t                  width_in,
                           uint32_t                  height_in) {
        bool     hasAuxSurf   = false;
        bool     isPlanar     = true;
        bool     hasRefSurf   = false;
        uint8_t *surfBaseAddr = nullptr;
        uint8_t *lockedAddr   = nullptr;
        if (!DumpIsEnabled(attrName, mediaState))
        {
            return MOS_STATUS_SUCCESS;
        }

        const char *funcName = (m_codecFunction == CODECHAL_FUNCTION_DECODE) ? "_DEC" : (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE ? "_DEC" : "_ENC");
        std::string bufName  = std::string(surfName) + "_w[" + std::to_string(surface->dwWidth) + "]_h[" + std::to_string(surface->dwHeight) + "]_p[" + std::to_string(surface->dwPitch) + "]";

        const char *filePath = CreateFileName(funcName, bufName.c_str(), hasAuxSurf ? ".Y" : ".yuv");

        MOS_LOCK_PARAMS   lockFlags;
        GMM_RESOURCE_FLAG gmmFlags;

        MOS_ZeroMemory(&gmmFlags, sizeof(gmmFlags));
        CODECHAL_DEBUG_CHK_NULL(surface);
        gmmFlags   = surface->OsResource.pGmmResInfo->GetResFlags();
        hasAuxSurf = (gmmFlags.Gpu.MMC || gmmFlags.Info.MediaCompressed) && gmmFlags.Gpu.UnifiedAuxSurface;

        if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDecodeAuxSurface))
        {
            hasAuxSurf = false;
        }

        isPlanar = !!(m_osInterface->pfnGetGmmClientContext(m_osInterface)->IsPlanar(surface->OsResource.pGmmResInfo->GetResourceFormat()));

        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly     = 1;
        lockFlags.TiledAsTiled = 1;  // Bypass GMM CPU blit due to some issues in GMM CpuBlt function
        if (hasAuxSurf)
        {
            // Dump MMC surface as raw layout
            lockFlags.NoDecompress = 1;
        }

        // when not setting dump compressed surface cfg, use ve copy for dump
        if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDecodeCompSurface))
        {
            DumpUncompressedYUVSurface(surface);
            lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_temp2DSurfForCopy.OsResource, &lockFlags);
            CODECHAL_DEBUG_CHK_NULL(lockedAddr);
            surface = &m_temp2DSurfForCopy;
        }
        else
        {
            // when setting dump compressed surface cfg, try directly lock surface
            lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &surface->OsResource, &lockFlags);
            // if failed to lock, try to submit copy task and reallocate temp surface for dump
            if (lockedAddr == nullptr)
            {
                uint32_t        sizeToBeCopied = 0;
                MOS_GFXRES_TYPE ResType;

#if !defined(LINUX)
                ResType = surface->OsResource.ResType;

                CODECHAL_DEBUG_CHK_STATUS(ReAllocateSurface(
                    &m_temp2DSurfForCopy,
                    surface,
                    "Temp2DSurfForSurfDumper",
                    ResType));
#endif

                uint32_t arraySize = surface->OsResource.pGmmResInfo->GetArraySize();

                if (arraySize == 0)
                {
                    return MOS_STATUS_UNKNOWN;
                }

                uint32_t sizeSrcSurface = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface()) / arraySize;
                // Ensure allocated buffer size contains the source surface size
                if (m_temp2DSurfForCopy.OsResource.pGmmResInfo->GetSizeMainSurface() >= sizeSrcSurface)
                {
                    sizeToBeCopied = sizeSrcSurface;
                }

                if (sizeToBeCopied == 0)
                {
                    CODECHAL_DEBUG_ASSERTMESSAGE("Cannot allocate correct size, failed to copy nonlockable resource");
                    return MOS_STATUS_INVALID_PARAMETER;
                }

                CODECHAL_DEBUG_VERBOSEMESSAGE("Temp2DSurfaceForCopy width %d, height %d, pitch %d, TileType %d, bIsCompressed %d, CompressionMode %d",
                    m_temp2DSurfForCopy.dwWidth,
                    m_temp2DSurfForCopy.dwHeight,
                    m_temp2DSurfForCopy.dwPitch,
                    m_temp2DSurfForCopy.TileType,
                    m_temp2DSurfForCopy.bIsCompressed,
                    m_temp2DSurfForCopy.CompressionMode);

                uint32_t bpp = surface->OsResource.pGmmResInfo->GetBitsPerPixel();

                CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnMediaCopyResource2D(m_osInterface, &surface->OsResource, &m_temp2DSurfForCopy.OsResource, surface->dwPitch, sizeSrcSurface / (surface->dwPitch), bpp, false));

                lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_temp2DSurfForCopy.OsResource, &lockFlags);
                CODECHAL_DEBUG_CHK_NULL(lockedAddr);
                surface = &m_temp2DSurfForCopy;
            }
        }
        gmmFlags     = surface->OsResource.pGmmResInfo->GetResFlags();
        surfBaseAddr = lockedAddr;

        // Use MOS swizzle for non-vecopy dump
        if (m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDecodeCompSurface))
        {
            uint32_t sizeMain = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface());
            surfBaseAddr      = (uint8_t *)MOS_AllocMemory(sizeMain);
            CODECHAL_DEBUG_CHK_NULL(surfBaseAddr);
            Mos_SwizzleData(lockedAddr, surfBaseAddr, surface->TileType, MOS_TILE_LINEAR, sizeMain / surface->dwPitch, surface->dwPitch, !MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrTileY) || gmmFlags.Info.Tile4);
        }

        uint8_t *data = surfBaseAddr;
        data += surface->dwOffset + surface->YPlaneOffset.iYOffset * surface->dwPitch;

        uint32_t width      = width_in ? width_in : surface->dwWidth;
        uint32_t height     = height_in ? height_in : surface->dwHeight;
        uint32_t lumaheight = 0;

        switch (surface->Format)
        {
        case Format_YUY2:
        case Format_P010:
        case Format_P016:
            width = width << 1;
            break;
        case Format_Y216:
        case Format_Y210:  //422 10bit -- Y0[15:0]:U[15:0]:Y1[15:0]:V[15:0] = 32bits per pixel = 4Bytes per pixel
        case Format_Y410:  //444 10bit -- A[31:30]:V[29:20]:Y[19:10]:U[9:0] = 32bits per pixel = 4Bytes per pixel
        case Format_R10G10B10A2:
        case Format_AYUV:  //444 8bit  -- A[31:24]:Y[23:16]:U[15:8]:V[7:0] = 32bits per pixel = 4Bytes per pixel
        case Format_A8R8G8B8:
            width = width << 2;
            break;
        case Format_Y416:
            width = width << 3;
            break;
        case Format_R8G8B8:
            width = width * 3;
            break;
        default:
            break;
        }

        uint32_t pitch = surface->dwPitch;
        if (surface->Format == Format_UYVY)
            pitch = width;

        if (CodecHal_PictureIsBottomField(m_currPic))
        {
            data += pitch;
        }

        if (CodecHal_PictureIsField(m_currPic))
        {
            pitch *= 2;
            height /= 2;
        }

        lumaheight = hasAuxSurf ? GFX_ALIGN(height, 32) : height;

        std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
        if (ofs.fail())
        {
            return MOS_STATUS_UNKNOWN;
        }

        // write luma data to file
        for (uint32_t h = 0; h < lumaheight; h++)
        {
            ofs.write((char *)data, hasAuxSurf ? pitch : width);
            data += pitch;
        }

        switch (surface->Format)
        {
        case Format_NV12:
        case Format_P010:
        case Format_P016:
            height = height >> 1;
            break;
        case Format_Y416:  //444 16bit
        case Format_AYUV:  //444 8bit
        case Format_AUYV:
        case Format_Y410:  //444 10bit
        case Format_R10G10B10A2:
            height = height << 1;
            break;
        case Format_YUY2:
        case Format_YUYV:
        case Format_YVYU:
        case Format_UYVY:
        case Format_VYUY:
        case Format_Y216:  //422 16bit
        case Format_Y210:  //422 10bit
        case Format_P208:  //422 8bit
        case Format_RGBP:
        case Format_BGRP:
            break;
        case Format_422V:
        case Format_IMC3:
            height = height / 2;
            break;
        default:
            height = 0;
            break;
        }

        uint8_t *vPlaneData = surfBaseAddr;
        if (isPlanar)
        {
            if (hasAuxSurf)
            {
                data = surfBaseAddr + surface->UPlaneOffset.iSurfaceOffset;
            }
            else
            {
// LibVA uses iSurfaceOffset instead of iLockSurfaceOffset.To use this dump in Linux,
// changing the variable to iSurfaceOffset may be necessary for the chroma dump to work properly.
#if defined(LINUX)
                data = surfBaseAddr + surface->UPlaneOffset.iSurfaceOffset;
                if (surface->Format == Format_422V || surface->Format == Format_IMC3)
                {
                    vPlaneData = surfBaseAddr + surface->VPlaneOffset.iSurfaceOffset;
                }
#else
                data = surfBaseAddr + surface->UPlaneOffset.iLockSurfaceOffset;
                if (surface->Format == Format_422V ||
                    surface->Format == Format_IMC3 ||
                    surface->Format == Format_RGBP ||
                    surface->Format == Format_BGRP)
                {
                    vPlaneData = surfBaseAddr + surface->VPlaneOffset.iLockSurfaceOffset;
                }
#endif
            }

            //No seperate chroma for linear surfaces
            // Seperate Y/UV if MMC is enabled
            if (hasAuxSurf)
            {
                const char   *uvfilePath = CreateFileName(funcName, bufName.c_str(), ".UV");
                std::ofstream ofs1(uvfilePath, std::ios_base::out | std::ios_base::binary);
                if (ofs1.fail())
                {
                    return MOS_STATUS_UNKNOWN;
                }
                // write chroma data to file
                for (uint32_t h = 0; h < GFX_ALIGN(height, 32); h++)
                {
                    ofs1.write((char *)data, pitch);
                    data += pitch;
                }
                ofs1.close();
            }
            else
            {
                // write chroma data to file
                for (uint32_t h = 0; h < height; h++)
                {
                    ofs.write((char *)data, hasAuxSurf ? pitch : width);
                    data += pitch;
                }

                // write v planar data to file
                if (surface->Format == Format_422V ||
                    surface->Format == Format_IMC3 ||
                    surface->Format == Format_RGBP ||
                    surface->Format == Format_BGRP)
                {
                    for (uint32_t h = 0; h < height; h++)
                    {
                        ofs.write((char *)vPlaneData, hasAuxSurf ? pitch : width);
                        vPlaneData += pitch;
                    }
                }
            }
            ofs.close();
        }

        if (hasAuxSurf)
        {
            uint32_t resourceIndex = m_osInterface->pfnGetResourceIndex(&surface->OsResource);
            uint8_t *yAuxData      = (uint8_t *)lockedAddr + surface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_Y_CCS);
            uint32_t yAuxSize      = isPlanar ? ((uint32_t)(surface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_UV_CCS) -
                                                       surface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_Y_CCS)))
                                              : (uint32_t)surface->OsResource.pGmmResInfo->GetAuxQPitch();

            // Y Aux data
            const char   *yAuxfilePath = CreateFileName(funcName, bufName.c_str(), ".Yaux");
            std::ofstream ofs2(yAuxfilePath, std::ios_base::out | std::ios_base::binary);
            if (ofs2.fail())
            {
                return MOS_STATUS_UNKNOWN;
            }
            ofs2.write((char *)yAuxData, yAuxSize);
            ofs2.close();

            if (isPlanar)
            {
                uint8_t *uvAuxData = (uint8_t *)lockedAddr + surface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_UV_CCS);
                uint32_t uvAuxSize = (uint32_t)surface->OsResource.pGmmResInfo->GetAuxQPitch() - yAuxSize;

                // UV Aux data
                const char   *uvAuxfilePath = CreateFileName(funcName, bufName.c_str(), ".UVaux");
                std::ofstream ofs3(uvAuxfilePath, std::ios_base::out | std::ios_base::binary);
                if (ofs3.fail())
                {
                    return MOS_STATUS_UNKNOWN;
                }
                ofs3.write((char *)uvAuxData, uvAuxSize);

                ofs3.close();
            }
        }

        if (surfBaseAddr && m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDecodeCompSurface))
        {
            MOS_FreeMemory(surfBaseAddr);
        }
        if (lockedAddr)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);
        }

        return MOS_STATUS_SUCCESS;
    };

    m_dumpYUVSurface = [this](
                                     PMOS_SURFACE              surface,
                                     const char               *attrName,
                                     const char               *surfName,
                                     CODECHAL_MEDIA_STATE_TYPE mediaState,
                                     uint32_t                  width_in,
                                     uint32_t                  height_in) {
        if (!DumpIsEnabled(attrName, mediaState))
        {
            return MOS_STATUS_SUCCESS;
        }

        const char *funcName = (m_codecFunction == CODECHAL_FUNCTION_DECODE) ? "_DEC" : (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE ? "_DEC" : "_ENC");
        std::string bufName  = std::string(surfName) + "_w[" + std::to_string(surface->dwWidth) + "]_h[" + std::to_string(surface->dwHeight) + "]_p[" + std::to_string(surface->dwPitch) + "]";

        const char *filePath = CreateFileName(funcName, bufName.c_str(), ".yuv");

        MediaDebugFastDump::Dump(surface->OsResource, filePath);

        if (!MediaDebugFastDump::IsGood())
        {
            return m_dumpYUVSurfaceLegacy(
                surface,
                attrName,
                surfName,
                mediaState,
                width_in,
                height_in);
        }

        return MOS_STATUS_SUCCESS;
    };

    m_dumpBufferLegacy = [this](
                       PMOS_RESOURCE             resource,
                       const char               *attrName,
                       const char               *bufferName,
                       uint32_t                  size,
                       uint32_t                  offset,
                       CODECHAL_MEDIA_STATE_TYPE mediaState) {
        MEDIA_DEBUG_FUNCTION_ENTER;

        MEDIA_DEBUG_CHK_NULL(resource);
        MEDIA_DEBUG_CHK_NULL(bufferName);

        if (size == 0)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (attrName)
        {
            bool attrEnabled = false;

            if (mediaState == CODECHAL_NUM_MEDIA_STATES)
            {
                attrEnabled = m_configMgr->AttrIsEnabled(attrName);
            }
            else
            {
                attrEnabled = static_cast<CodecDebugConfigMgr *>(m_configMgr)->AttrIsEnabled(mediaState, attrName);
            }

            if (!attrEnabled)
            {
                return MOS_STATUS_SUCCESS;
            }
        }

        const char *fileName;
        bool        binaryDump = false;
        if ((!strcmp(attrName, MediaDbgAttr::attrDecodeBitstream)) ||
            (!strcmp(attrName, MediaDbgAttr::attrStreamIn)) ||
            (!strcmp(attrName, MediaDbgAttr::attrStreamOut)) ||
            (!strcmp(attrName, MediaDbgAttr::attrMvData)) ||
            (!strcmp(attrName, MediaDbgAttr::attrSegId)) ||
            (!strcmp(attrName, MediaDbgAttr::attrCoefProb)) ||
            m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary))
        {
            binaryDump = true;
        }
        const char *extType = binaryDump ? MediaDbgExtType::dat : MediaDbgExtType::txt;

        if (mediaState == CODECHAL_NUM_MEDIA_STATES)
        {
            fileName = CreateFileName(bufferName, attrName, extType);
        }
        else
        {
            std::string kernelName = static_cast<CodecDebugConfigMgr *>(m_configMgr)->GetMediaStateStr(mediaState);
            fileName               = CreateFileName(kernelName.c_str(), bufferName, extType);
        }

        MOS_STATUS status = MOS_STATUS_SUCCESS;
        
        if ((resource->pGmmResInfo != nullptr) &&
            (resource->pGmmResInfo->GetResFlags().Info.NotLockable))
        {
            uint8_t      *data            = nullptr;
            PMOS_RESOURCE pReadbackBuffer = (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE));
            CODECHAL_DEBUG_CHK_NULL(pReadbackBuffer);
            status                        = AllocateLinearResource(MOS_GFXRES_BUFFER, (uint32_t)resource->pGmmResInfo->GetBaseWidth(), (uint32_t)resource->pGmmResInfo->GetBaseHeight(), Format_Any, pReadbackBuffer);
            if (MOS_FAILED(status))
            {
                MOS_FreeMemAndSetNull(pReadbackBuffer);
            }
            CODECHAL_DEBUG_CHK_STATUS(status);

            status = VDBypassCopyResource(resource, pReadbackBuffer);
            if (MOS_FAILED(status))
            {
                m_osInterface->pfnFreeResource(m_osInterface, pReadbackBuffer);
                MOS_FreeMemAndSetNull(pReadbackBuffer);
            }
            CODECHAL_DEBUG_CHK_STATUS(status);

            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.ReadOnly = 1;
            data               = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, pReadbackBuffer, &lockFlags);
            if(data == nullptr)
            {
                m_osInterface->pfnFreeResource(m_osInterface, pReadbackBuffer);
                MOS_FreeMemAndSetNull(pReadbackBuffer);
            }
            CODECHAL_DEBUG_CHK_NULL(data);

            data += offset;

            if (binaryDump)
            {
                status = DumpBufferInBinary(data, size);
            }
            else
            {
                status = DumpBufferInHexDwords(data, size);
            }

            m_osInterface->pfnUnlockResource(m_osInterface, pReadbackBuffer);
            m_osInterface->pfnFreeResource(m_osInterface, pReadbackBuffer);
            MOS_FreeMemAndSetNull(pReadbackBuffer);
            CODECHAL_DEBUG_CHK_STATUS(status);
        }
        else
        {
            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.ReadOnly = 1;
            uint8_t *data      = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, resource, &lockFlags);
            MEDIA_DEBUG_CHK_NULL(data);
            data += offset;

            if (binaryDump)
            {
                status = DumpBufferInBinary(data, size);
            }
            else
            {
                status = DumpBufferInHexDwords(data, size);
            }

            if (data)
            {
                m_osInterface->pfnUnlockResource(m_osInterface, resource);
            }
        }

        return status;
    };

    m_dumpBuffer = [this](
                                 PMOS_RESOURCE             resource,
                                 const char               *attrName,
                                 const char               *bufferName,
                                 uint32_t                  size,
                                 uint32_t                  offset,
                                 CODECHAL_MEDIA_STATE_TYPE mediaState) {
        MEDIA_DEBUG_FUNCTION_ENTER;

        MEDIA_DEBUG_CHK_NULL(resource);
        MEDIA_DEBUG_CHK_NULL(bufferName);

        if (size == 0)
        {
            return MOS_STATUS_SUCCESS;
        }

        bool attrEnabled = false;
        if (mediaState == CODECHAL_NUM_MEDIA_STATES)
        {
            attrEnabled = m_configMgr->AttrIsEnabled(attrName);
        }
        else
        {
            attrEnabled = static_cast<CodecDebugConfigMgr *>(m_configMgr)->AttrIsEnabled(mediaState, attrName);
        }
        if (!attrEnabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        const char *fileName;
        bool        binaryDump = false;
        if ((!strcmp(attrName, MediaDbgAttr::attrDecodeBitstream)) ||
            (!strcmp(attrName, MediaDbgAttr::attrStreamIn)) ||
            (!strcmp(attrName, MediaDbgAttr::attrStreamOut)) ||
            (!strcmp(attrName, MediaDbgAttr::attrMvData)) ||
            (!strcmp(attrName, MediaDbgAttr::attrSegId)) ||
            (!strcmp(attrName, MediaDbgAttr::attrCoefProb)) ||
            m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary))
        {
            binaryDump = true;
        }
        const char *extType = binaryDump ? MediaDbgExtType::dat : MediaDbgExtType::txt;

        if (mediaState == CODECHAL_NUM_MEDIA_STATES)
        {
            fileName = CreateFileName(bufferName, attrName, extType);
        }
        else
        {
            std::string kernelName = static_cast<CodecDebugConfigMgr *>(m_configMgr)->GetMediaStateStr(mediaState);
            fileName               = CreateFileName(kernelName.c_str(), bufferName, extType);
        }

        MediaDebugFastDump::Dump(*resource, fileName, size, offset);

        if (!MediaDebugFastDump::IsGood())
        {
            return m_dumpBufferLegacy(
                resource,
                attrName,
                bufferName,
                size,
                offset,
                mediaState);
        }

        return MOS_STATUS_SUCCESS;
    };
}

CodechalDebugInterface::~CodechalDebugInterface()
{
    if (nullptr != m_configMgr)
    {
        MOS_Delete(m_configMgr);
    }

    MediaDebugFastDump::DestroyInstance();
}

CodechalDebugInterface* CodechalDebugInterface::Create()
{
    return MOS_New(CodechalDebugInterface);
}

void CodechalDebugInterface::CheckGoldenReferenceExist()
{
    std::ifstream crcGoldenRefStream(m_crcGoldenRefFileName);
    m_goldenReferenceExist = crcGoldenRefStream.good() ? true : false;
}

MOS_STATUS CodechalDebugInterface::DumpRgbDataOnYUVSurface(
    PMOS_SURFACE              surface,
    const char               *attrName,
    const char               *surfName,
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    uint32_t                  width_in,
    uint32_t                  height_in)
{
    bool     hasAuxSurf   = false;
    bool     isPlanar     = true;
    bool     hasRefSurf   = false;
    uint8_t *surfBaseAddr = nullptr;
    uint8_t *lockedAddr   = nullptr;

    if (!DumpIsEnabled(attrName, mediaState))
    {
        return MOS_STATUS_SUCCESS;
    }

    //To write RGB data on AYUV surface for validation purpose due to similiar layout
    int32_t sfcOutputRgbFormatFlag = 0;
    int32_t bIsSfcOutputLinearFlag = 0;
    //tile type read from reg key
    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_VALUE_DECODE_SFC_RGBFORMAT_OUTPUT_DEBUG,
            MediaUserSetting::Group::Sequence);
        sfcOutputRgbFormatFlag = outValue.Get<int32_t>();
    }

    //rgb format output read from reg key
    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_VALUE_DECODE_SFC_LINEAR_OUTPUT_DEBUG,
            MediaUserSetting::Group::Sequence);
        bIsSfcOutputLinearFlag = outValue.Get<int32_t>();
    }

    if (!sfcOutputRgbFormatFlag)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_FORMAT  outputFormat = surface->Format;
    const char *filePostfix  = "unsupport";

    switch (sfcOutputRgbFormatFlag)
    {
    case 1:  //RGBP
        outputFormat = Format_RGBP;
        filePostfix  = "rgbp";
        break;
    case 2:  //BGRP
        outputFormat = Format_BGRP;
        filePostfix  = "bgrp";
        break;
    case 3:  //RGB24 only support Tile Linear
        if (bIsSfcOutputLinearFlag)
        {
            outputFormat = Format_R8G8B8;
            filePostfix  = "rgb24";
            break;
        }
    default:
        CODECHAL_DEBUG_ASSERTMESSAGE("unsupport rgb format");
        return MOS_STATUS_SUCCESS;
    }

    MOS_TILE_TYPE outputTileType = MOS_TILE_Y;
    if (bIsSfcOutputLinearFlag == 1)
    {
        outputTileType = MOS_TILE_LINEAR;
    }

    MOS_LOCK_PARAMS   lockFlags;
    GMM_RESOURCE_FLAG gmmFlags;

    MOS_ZeroMemory(&gmmFlags, sizeof(gmmFlags));
    CODECHAL_DEBUG_CHK_NULL(surface);
    gmmFlags   = surface->OsResource.pGmmResInfo->GetResFlags();
    hasAuxSurf = (gmmFlags.Gpu.MMC || gmmFlags.Info.MediaCompressed) && gmmFlags.Gpu.UnifiedAuxSurface;

    if (!m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrDecodeAuxSurface))
    {
        hasAuxSurf = false;
    }

    if (strcmp(attrName, CodechalDbgAttr::attrDecodeReferenceSurfaces) == 0)
    {
        hasRefSurf = true;
    }

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly     = 1;
    lockFlags.TiledAsTiled = 1;  // Bypass GMM CPU blit due to some issues in GMM CpuBlt function
    if (hasAuxSurf)
    {
        // Dump MMC surface as raw layout
        lockFlags.NoDecompress = 1;
    }

    lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &surface->OsResource, &lockFlags);

    if (lockedAddr == nullptr)  // Failed to lock. Try to submit copy task and dump another surface
    {
        uint32_t        sizeToBeCopied = 0;
        MOS_GFXRES_TYPE ResType;

#if LINUX
        // Linux does not have OsResource->ResType
        ResType = surface->Type;
#else
        ResType = surface->OsResource.ResType;
#endif

        CODECHAL_DEBUG_CHK_STATUS(ReAllocateSurface(
            &m_temp2DSurfForCopy,
            surface,
            "Temp2DSurfForSurfDumper",
            ResType));

        if (!hasRefSurf)
        {
            uint32_t arraySize = surface->OsResource.pGmmResInfo->GetArraySize();

            if (arraySize == 0)
            {
                return MOS_STATUS_UNKNOWN;
            }

            uint32_t sizeSrcSurface = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface()) / arraySize;
            // Ensure allocated buffer size contains the source surface size
            if (m_temp2DSurfForCopy.OsResource.pGmmResInfo->GetSizeMainSurface() >= sizeSrcSurface)
            {
                sizeToBeCopied = sizeSrcSurface;
            }

            if (sizeToBeCopied == 0)
            {
                // Currently, MOS's pfnAllocateResource does not support allocate a surface reference to another surface.
                // When the source surface is not created from Media, it is possible that we cannot allocate the same size as source.
                // For example, on Gen9, Render target might have GMM set CCS=1 MMC=0, but MOS cannot allocate surface with such combination.
                // When Gmm allocation parameter is different, the resulting surface size/padding/pitch will be differnt.
                // Once if MOS can support allocate a surface by reference another surface, we can do a bit to bit copy without problem.
                CODECHAL_DEBUG_ASSERTMESSAGE("Cannot allocate correct size, failed to copy nonlockable resource");
                return MOS_STATUS_NULL_POINTER;
            }

            CODECHAL_DEBUG_VERBOSEMESSAGE("Temp2DSurfaceForCopy width %d, height %d, pitch %d, TileType %d, bIsCompressed %d, CompressionMode %d",
                m_temp2DSurfForCopy.dwWidth,
                m_temp2DSurfForCopy.dwHeight,
                m_temp2DSurfForCopy.dwPitch,
                m_temp2DSurfForCopy.TileType,
                m_temp2DSurfForCopy.bIsCompressed,
                m_temp2DSurfForCopy.CompressionMode);

            uint32_t bpp = surface->OsResource.pGmmResInfo->GetBitsPerPixel();

            CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnMediaCopyResource2D(m_osInterface, &surface->OsResource, &m_temp2DSurfForCopy.OsResource, surface->dwPitch, sizeSrcSurface / (surface->dwPitch), bpp, false));
        }
        else
        {
            CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnDoubleBufferCopyResource(m_osInterface, &surface->OsResource, &m_temp2DSurfForCopy.OsResource, false));
        }

        lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_temp2DSurfForCopy.OsResource, &lockFlags);
        CODECHAL_DEBUG_CHK_NULL(lockedAddr);

        surface  = &m_temp2DSurfForCopy;
        gmmFlags = surface->OsResource.pGmmResInfo->GetResFlags();
    }

    if (hasAuxSurf)
    {
        uint32_t sizeMain = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface());
        surfBaseAddr      = (uint8_t *)MOS_AllocMemory(sizeMain);
        CODECHAL_DEBUG_CHK_NULL(surfBaseAddr);

        Mos_SwizzleData(lockedAddr, surfBaseAddr, surface->TileType, MOS_TILE_LINEAR, sizeMain / surface->dwPitch, surface->dwPitch, true);
    }
    else
    {
        surfBaseAddr = lockedAddr;
    }

    // Allocate RGB surface internally to get surface info
    MOS_SURFACE *tmpRgbSurface = MOS_New(MOS_SURFACE);
    CODECHAL_DEBUG_CHK_NULL(tmpRgbSurface);
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type              = MOS_GFXRES_2D;
    allocParams.TileType          = outputTileType;
    allocParams.Format            = outputFormat;
    allocParams.dwWidth           = surface->dwWidth;
    allocParams.dwHeight          = surface->dwHeight;
    allocParams.dwArraySize       = 1;
    allocParams.pBufName          = "dump rgb surface";
    allocParams.bIsCompressible   = false;
    allocParams.ResUsageType      = MOS_HW_RESOURCE_DEF_MAX;
    allocParams.m_tileModeByForce = MOS_TILE_UNSET_GMM;

    CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        &tmpRgbSurface->OsResource));

    tmpRgbSurface->Format = Format_Invalid;
    CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, &tmpRgbSurface->OsResource, tmpRgbSurface));

    uint32_t rgbPitch      = tmpRgbSurface->dwPitch;
    uint32_t alignedHeight = surface->dwHeight;
    uint32_t alignedWidth  = surface->dwWidth;

    if (outputFormat == Format_RGBP || outputFormat == Format_BGRP && rgbPitch)
    {
        alignedHeight = tmpRgbSurface->RenderOffset.YUV.U.BaseOffset / rgbPitch;
        alignedWidth  = rgbPitch;
    }

    // Always use MOS swizzle instead of GMM Cpu blit
    uint32_t sizeMain = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface());
    surfBaseAddr      = (uint8_t *)MOS_AllocMemory(sizeMain);
    CODECHAL_DEBUG_CHK_NULL(surfBaseAddr);

    int32_t  swizzleHeight = sizeMain / surface->dwPitch;
    int32_t  swizzlePitch  = surface->dwPitch;
    uint8_t *data          = lockedAddr;

    CODECHAL_DEBUG_VERBOSEMESSAGE("RGB Surface info: format %d, tiletype %d, pitch %d,UOffset %d, dwSize %d, sizemain %d",
        outputFormat,
        tmpRgbSurface->TileType,
        tmpRgbSurface->dwPitch,
        tmpRgbSurface->RenderOffset.YUV.U.BaseOffset,
        tmpRgbSurface->dwSize,
        sizeMain);

    if (outputTileType == MOS_TILE_Y)
    {
        swizzlePitch = rgbPitch;
        Mos_SwizzleData(lockedAddr, surfBaseAddr, outputTileType, MOS_TILE_LINEAR, sizeMain / swizzlePitch, swizzlePitch, !MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrTileY) || gmmFlags.Info.Tile4);
        data = surfBaseAddr;
    }

    data += surface->dwOffset + surface->YPlaneOffset.iYOffset * surface->dwPitch;

    uint32_t width      = (width_in ? width_in : surface->dwWidth) << 2;
    uint32_t height     = height_in ? height_in : surface->dwHeight;
    uint32_t lumaheight = 0;
    uint32_t pitch      = surface->dwPitch;

    lumaheight = hasAuxSurf ? GFX_ALIGN(height, 32) : height;

    const char *funcName = (m_codecFunction == CODECHAL_FUNCTION_DECODE) ? "_DEC" : (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE ? "_DEC" : "_ENC");
    std::string bufName  = std::string(surfName) + "_w[" + std::to_string(alignedWidth) + "]_h[" + std::to_string(alignedHeight) + "]_p[" + std::to_string(rgbPitch) + "]";

    const char *filePath = CreateFileName(funcName, bufName.c_str(), filePostfix);

    std::ofstream ofs(filePath, std::ios_base::out | std::ios_base::binary);
    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    // RGB data has one zero byte in every 64 bytes, need to remove this byte
    if (IS_RGB24_FORMAT(outputFormat))
    {
        for (uint32_t h = 0; h < lumaheight; h++)
        {
            width                 = surface->dwWidth * 3;
            int dummyBytesPerLine = width / 63;
            int resPixel          = width - 63 * dummyBytesPerLine;
            int resBytesPerLine   = rgbPitch;
            for (int p = 0; p < dummyBytesPerLine; p++)
            {
                ofs.write((char *)data, 63);
                data += 64;
                resBytesPerLine -= 64;
            }

            if (resPixel > 0)
            {
                ofs.write((char *)data, resPixel);
                data += resPixel;
                resBytesPerLine -= resPixel;
            }

            if (resBytesPerLine)
            {
                data += resBytesPerLine;
            }
        }
    }
    else
    {
        for (uint32_t h = 0; h < lumaheight; h++)
        {
            ofs.write((char *)data, hasAuxSurf ? pitch : width);
            data += pitch;
        }
    }

    if (surfBaseAddr)
    {
        MOS_FreeMemory(surfBaseAddr);
    }
    if (lockedAddr)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);
    }

    m_osInterface->pfnFreeResource(m_osInterface, &(tmpRgbSurface->OsResource));
    MOS_Delete(tmpRgbSurface);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpBltOutput(
    PMOS_SURFACE surface,
    const char  *attrName)
{
    if (!DumpIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        CODECHAL_DEBUG_NORMALMESSAGE("Don't support BltOutput dump, pls not use BltState directly!");
        return MOS_STATUS_SUCCESS;
    }

}

MOS_STATUS CodechalDebugInterface::InitializeUserSetting()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    CODECHAL_DEBUG_CHK_STATUS(MediaDebugInterface::InitializeUserSetting());

    DeclareUserSettingKeyForDebug(
        m_userSettingPtr,
        __MEDIA_USER_FEATURE_ENABLE_HW_DEBUG_HOOKS_DEBUG,
        MediaUserSetting::Group::Device,
        int32_t(0),
        false);

    DeclareUserSettingKeyForDebug(
        m_userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_FRAME_NUMBER_TO_STOP_DEBUG,
        MediaUserSetting::Group::Device,
        int32_t(-1),
        false);

    DeclareUserSettingKeyForDebug(
        m_userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_ENABLE_SW_CRC_DEBUG,
        MediaUserSetting::Group::Device,
        uint32_t(0),
        false);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::Initialize(
    CodechalHwInterfaceNext *hwInterface,
    CODECHAL_FUNCTION        codecFunction,
    MediaCopyWrapper        *mediaCopyWrapper)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(hwInterface);
    m_hwInterfaceNext  = hwInterface;
    m_codecFunction    = codecFunction;
    m_osInterface      = m_hwInterfaceNext->GetOsInterface();
    m_cpInterface      = m_hwInterfaceNext->GetCpInterface();

    CODECHAL_DEBUG_CHK_NULL(m_osInterface);
    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    CODECHAL_DEBUG_CHK_STATUS(InitializeUserSetting());

    //dump loctaion is codechaldump
    MediaDebugInterface::SetOutputFilePath();

    m_configMgr = MOS_New(CodecDebugConfigMgr, this, m_codecFunction, m_outputFilePath);
    CODECHAL_DEBUG_CHK_NULL(m_configMgr);
    CODECHAL_DEBUG_CHK_STATUS(m_configMgr->ParseConfig(m_osInterface->pOsContext));

    MediaDebugInterface::InitDumpLocation();

    m_osInterface->pfnGetPlatform(m_osInterface, &m_platform);

#if (_DEBUG || _RELEASE_INTERNAL)
    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_ENABLE_HW_DEBUG_HOOKS_DEBUG,
            MediaUserSetting::Group::Device,
            0,
            true);
        m_enableHwDebugHooks = outValue.Get<bool>();
    }
    CheckGoldenReferenceExist();
    if (m_enableHwDebugHooks && m_goldenReferenceExist)
    {
        LoadGoldenReference();
    }

    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_VALUE_CODECHAL_FRAME_NUMBER_TO_STOP_DEBUG,
            MediaUserSetting::Group::Device,
            -1,
            true);
        m_stopFrameNumber = outValue.Get<int32_t>();
    }

    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __MEDIA_USER_FEATURE_VALUE_CODECHAL_ENABLE_SW_CRC_DEBUG,
            MediaUserSetting::Group::Device,
            0,
            true);
        m_swCRC = outValue.Get<bool>();
    }
#endif

    SetFastDumpConfig(mediaCopyWrapper);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpHucDmem(
    PMOS_RESOURCE             dmemResource,
    uint32_t                  dmemSize,
    uint32_t                  hucPassNum,
    CodechalHucRegionDumpType dumpType)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_configMgr->AttrIsEnabled(MediaDbgAttr::attrHuCDmem) && !MosUtilities::GetTraceSetting())
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(dmemResource);
    if (Mos_ResourceIsNull(dmemResource))
    {
        return MOS_STATUS_NULL_POINTER;
    }

    std::string funcName = "";
    if (m_codecFunction == CODECHAL_FUNCTION_DECODE)
    {
        funcName = "DEC_";
    }
    else if (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE)
    {
        funcName = "DEC_Cenc_";
    }
    else
    {
        funcName = "ENC_";
    }

    std::string dmemName = MediaDbgBufferType::bufHucDmem;
    std::string passName = std::to_string(hucPassNum);
    switch (dumpType)
    {
    case hucRegionDumpInit:
        funcName = funcName + dmemName + "_InitPass" + passName;
        break;
    case hucRegionDumpUpdate:
        funcName = funcName + dmemName + "_UpdatePass" + passName;
        break;
    case hucRegionDumpLAUpdate:
        funcName = funcName + dmemName + "_LookaheadUpdatePass" + passName;
        break;
    case hucRegionDumpRegionLocked:
        funcName = funcName + dmemName + "_RegionLocked" + passName;
        break;
    case hucRegionDumpCmdInitializer:
        funcName = funcName + dmemName + "_CmdInitializerPass" + passName;
        break;
    case hucRegionDumpPakIntegrate:
        funcName = funcName + dmemName + "_PakIntPass" + passName;
        break;
    case hucRegionDumpHpu:
        funcName = funcName + dmemName + "_HpuPass" + passName;
        break;
    case hucRegionDumpHpuSuperFrame:
        funcName = funcName + dmemName + "_HpuPass" + passName + "_SuperFramePass";
        break;
    case hucRegionDumpBackAnnotation:
        funcName = funcName + dmemName + "_BackAnnotationPass" + passName;
        break;
    default:
        funcName = funcName + dmemName + "_Pass" + passName;
        break;
    }

    return DumpBuffer(dmemResource, MediaDbgAttr::attrHuCDmem, funcName.c_str(), dmemSize);
}

std::string CodechalDebugInterface::SetOutputPathKey()
{
    return __MEDIA_USER_FEATURE_VALUE_CODECHAL_DEBUG_OUTPUT_DIRECTORY;
}

std::string CodechalDebugInterface::InitDefaultOutput()
{
    m_outputFilePath.append(MEDIA_DEBUG_CODECHAL_DUMP_OUTPUT_FOLDER);
    return SetOutputPathKey();
}

MOS_STATUS CodechalDebugInterface::VDBypassCopyResource(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    
    CODECHAL_DEBUG_CHK_NULL(m_osInterface);
    CODECHAL_DEBUG_CHK_NULL(m_hwInterfaceNext)
    CODECHAL_DEBUG_CHK_NULL(src);
    CODECHAL_DEBUG_CHK_NULL(dst);

    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;
    MOS_GPUCTX_CREATOPTIONS_ENHANCED createOption;
    MOS_COMMAND_BUFFER               cmdBuffer;
    std::shared_ptr<mhw::mi::Itf>    miInterface    = m_hwInterfaceNext->GetMiInterfaceNext();
    PMHW_MI_MMIOREGISTERS            pMmioRegisters = miInterface->GetMmioRegisters();
    MOS_CONTEXT                     *pOsContext     = m_osInterface->pOsContext;
    MHW_ADD_CP_COPY_PARAMS           cpCopyParams;
    MHW_MI_FLUSH_DW_PARAMS           FlushDwParams;
    MhwCpInterface                  *mhwCpInterface = m_hwInterfaceNext->GetCpInterface();

    CODECHAL_DEBUG_CHK_NULL(pMmioRegisters);
    CODECHAL_DEBUG_CHK_NULL(pOsContext);
    CODECHAL_DEBUG_CHK_NULL(mhwCpInterface);

    CODECHAL_DEBUG_CHK_STATUS(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    CODECHAL_DEBUG_CHK_STATUS(miInterface->AddProtectedProlog(&cmdBuffer));

    MOS_ZeroMemory(&cpCopyParams, sizeof(cpCopyParams));
    uint32_t dwWidth     = (uint32_t)src->pGmmResInfo->GetBaseWidth();
    uint32_t dwHeight    = (uint32_t)src->pGmmResInfo->GetBaseHeight();
    cpCopyParams.size    = dwWidth * dwHeight;
    cpCopyParams.presSrc = src;
    cpCopyParams.presDst = dst;
    cpCopyParams.offset  = 0;
    cpCopyParams.bypass  = true;
    
    eStatus = mhwCpInterface->AddCpCopy(m_osInterface, &cmdBuffer, &cpCopyParams);

    auto &flushDwParams = miInterface->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams       = {};
    CODECHAL_DEBUG_CHK_STATUS(miInterface->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));
    PMOS_RESOURCE pStoreResource = (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE));
    CODECHAL_DEBUG_CHK_NULL(pStoreResource);
    // Create store resource
    {
        //Insert MFX_WAIT command.
        auto &mfxWaitParams               = miInterface->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        CODECHAL_DEBUG_CHK_STATUS(miInterface->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer, nullptr));

        eStatus = AllocateLinearResource(MOS_GFXRES_BUFFER, sizeof(uint32_t), 1, Format_Buffer, pStoreResource, true);
        if (MOS_FAILED(eStatus))
        {
            MOS_FreeMemAndSetNull(pStoreResource);
        }
        CODECHAL_DEBUG_CHK_STATUS(eStatus);

        eStatus = FillResourceMemory(*pStoreResource, sizeof(uint32_t), QUERY_STATUS_BUFFER_INITIAL_VALUE);
        if (MOS_FAILED(eStatus))
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Error: Failed FillResourceMemory to pStoreResource.");
            m_osInterface->pfnFreeResource(m_osInterface, pStoreResource);
            MOS_FreeMemAndSetNull(pStoreResource);
        }
        CODECHAL_DEBUG_CHK_STATUS(eStatus);

        auto &flushDwParams             = miInterface->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                   = {};
        flushDwParams.postSyncOperation = MHW_FLUSH_WRITE_IMMEDIATE_DATA;
        flushDwParams.pOsResource       = pStoreResource;
        flushDwParams.dwResourceOffset  = 0;
        flushDwParams.dwDataDW1         = STORE_DATA_DWORD_DATA;
        eStatus = miInterface->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer);
        if (MOS_FAILED(eStatus))
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Error: Failed MHW_ADDCMD_F(MI_FLUSH_DW).");
            m_osInterface->pfnFreeResource(m_osInterface, pStoreResource);
            MOS_FreeMemAndSetNull(pStoreResource);
        }
    }

    do
    {
        auto &batchBufferEndParams = miInterface->MHW_GETPAR_F(MI_BATCH_BUFFER_END)();
        batchBufferEndParams       = {};
        eStatus = miInterface->MHW_ADDCMD_F(MI_BATCH_BUFFER_END)(&cmdBuffer);
        if (MOS_FAILED(eStatus))
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Error: Failed MHW_ADDCMD_F(MI_BATCH_BUFFER_END).");
            break;
        }
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
        eStatus                           = m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, false);
        if (MOS_FAILED(eStatus))
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Error: Failed MHW_ADDCMD_F(MI_BATCH_BUFFER_END).");
            break;
        }

        volatile uint32_t dwValueInMemory = 0;
        MOS_LOCK_PARAMS sMosLockParams{};
        sMosLockParams.ReadOnly = 1;
        uint8_t *pStoreResBytes = reinterpret_cast<uint8_t*>(m_osInterface->pfnLockResource(m_osInterface, pStoreResource, &sMosLockParams));
        if (pStoreResBytes == nullptr)
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Error: Failed locking pStoreResource.");
            eStatus = MOS_STATUS_UNINITIALIZED;
            break;
        }

        dwValueInMemory     = *(reinterpret_cast<uint32_t *>(pStoreResBytes));
        if (dwValueInMemory != STORE_DATA_DWORD_DATA)
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Error: Failed to read Store Data output.");
            m_osInterface->pfnUnlockResource(m_osInterface, pStoreResource);
            eStatus = MOS_STATUS_SUCCESS;
            break;
        }
        m_osInterface->pfnUnlockResource(m_osInterface, pStoreResource);

    } while (false);
    m_osInterface->pfnFreeResource(m_osInterface, pStoreResource);
    MOS_FreeMemAndSetNull(pStoreResource);

    return eStatus;
}

MOS_STATUS CodechalDebugInterface::AllocateLinearResource(MOS_GFXRES_TYPE eType, uint32_t dwWidth, uint32_t dwHeight, MOS_FORMAT eFormat, PMOS_RESOURCE pResource, bool bSystemMem, bool bLockable) const
{
    MOS_ALLOC_GFXRES_PARAMS sMosAllocParams;
    MOS_STATUS              eStatus;

    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_osInterface);
    CODECHAL_DEBUG_CHK_NULL(m_osInterface->pfnAllocateResource);

    do
    {
        if (eType != MOS_GFXRES_BUFFER &&
            eType != MOS_GFXRES_2D)
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Error: Only 'BUFFER' and '2D' types are supported in CP.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }

        if (eType == MOS_GFXRES_BUFFER &&
            dwHeight != 1)
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Error: Resources of type 'BUFFER' must have height = 1.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }

        if (pResource == nullptr)
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Error: Failed to allocate memory for MOS_RESOURCE.");
            eStatus = MOS_STATUS_NO_SPACE;
            break;
        }
        MOS_ZeroMemory(pResource, sizeof(*pResource));

        MOS_ZeroMemory(&sMosAllocParams, sizeof(sMosAllocParams));
        sMosAllocParams.Type     = eType;
        sMosAllocParams.dwWidth  = dwWidth;
        sMosAllocParams.dwHeight = dwHeight;
        sMosAllocParams.TileType = MOS_TILE_LINEAR;
        sMosAllocParams.Format   = eFormat;

        if (MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrLimitedLMemBar))
        {
            if (bSystemMem)
            {
                sMosAllocParams.dwMemType = MOS_MEMPOOL_SYSTEMMEMORY;
            }
            else if (!bLockable)
            {
                sMosAllocParams.Flags.bNotLockable = 1;
                sMosAllocParams.dwMemType          = MOS_MEMPOOL_DEVICEMEMORY;
            }
            else if (bLockable)
            {
                sMosAllocParams.Flags.bNotLockable = 0;
                sMosAllocParams.dwMemType          = MOS_MEMPOOL_VIDEOMEMORY;
            }
        }

        if (MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrSAMediaCachePolicy))
        {
            sMosAllocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_CP_INTERNAL_WRITE;
        }

        eStatus = m_osInterface->pfnAllocateResource(
            m_osInterface,
            &sMosAllocParams,
            pResource);
        if (MOS_FAILED(eStatus))
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Failed to allocate resource. Error = 0x%x.", eStatus);
            eStatus = MOS_STATUS_UNKNOWN;
        }
    } while (false);

    return eStatus;
}

MOS_STATUS CodechalDebugInterface::FillResourceMemory(MOS_RESOURCE &sResource, uint32_t uiSize, uint8_t ucValue)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    void      *pSurface = nullptr;

    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (uiSize == 0)
    {
        // Nothing to do.
        CODECHAL_DEBUG_ASSERTMESSAGE("Received uiSize = 0. This should never happen!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    do
    {
        MOS_LOCK_PARAMS sMosLockParams{};
        sMosLockParams.WriteOnly = true;

        // Lock the surface for writing.
        pSurface = m_osInterface->pfnLockResource(m_osInterface, &sResource, &sMosLockParams);

        if (pSurface == nullptr)
        {
            CODECHAL_DEBUG_ASSERTMESSAGE("Error: Failed locking the resource.");
            eStatus = MOS_STATUS_UNKNOWN;
            break;
        }

        // Fill the surface with the wanted value.
        MOS_FillMemory(pSurface, uiSize, ucValue);

    } while (false);

    // Unlock the resource if needed.
    m_osInterface->pfnUnlockResource(m_osInterface, &sResource);

    return eStatus;
}

MOS_STATUS CodechalDebugInterface::DumpHucRegion(
    PMOS_RESOURCE             region,
    uint32_t                  regionOffset,
    uint32_t                  regionSize,
    uint32_t                  regionNum,
    const char *              regionName,
    bool                      inputBuffer,
    uint32_t                  hucPassNum,
    CodechalHucRegionDumpType dumpType)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_configMgr->AttrIsEnabled(MediaDbgAttr::attrHucRegions) && !MosUtilities::GetTraceSetting())
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_ASSERT(regionNum < 16);
    CODECHAL_DEBUG_CHK_NULL(region);

    if (Mos_ResourceIsNull(region))
    {
        return MOS_STATUS_NULL_POINTER;
    }

    std::string funcName = "";
    if (m_codecFunction == CODECHAL_FUNCTION_DECODE)
    {
        funcName = "DEC_";
    }
    else if (m_codecFunction == CODECHAL_FUNCTION_CENC_DECODE)
    {
        funcName = "DEC_CENC_";
    }
    else
    {
        funcName = "ENC_";
    }

    std::string bufName       = MediaDbgBufferType::bufHucRegion;
    std::string inputName     = (inputBuffer) ? "Input_" : "Output_";
    std::string regionNumName = std::to_string(regionNum);
    std::string passName      = std::to_string(hucPassNum);
    switch (dumpType)
    {
    case hucRegionDumpInit:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_InitPass" + passName;
        break;
    case hucRegionDumpUpdate:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_UpdatePass" + passName;
        break;
    case hucRegionDumpLAUpdate:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_LookaheadUpdatePass" + passName;
        break;
    case hucRegionDumpRegionLocked:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_RegionLockedPass" + passName;
        break;
    case hucRegionDumpCmdInitializer:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_CmdInitializerPass" + passName;
        break;
    case hucRegionDumpPakIntegrate:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_PakIntPass" + passName;
        break;
    case hucRegionDumpHpu:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_HpuPass" + passName;
        break;
    case hucRegionDumpBackAnnotation:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_BackAnnotationPass" + passName;
        break;
    default:
        funcName = funcName + inputName + bufName + regionNumName + regionName + "_Pass" + passName;
        break;
    }

    return DumpBuffer(region, MediaDbgAttr::attrHucRegions, funcName.c_str(), regionSize, regionOffset);
}

#define FIELD_TO_OFS(field_name) ofs << print_shift << std::setfill(' ') << std::setw(25) << std::left << std::string(#field_name) + ": " << (int64_t)report->field_name << std::endl;
#define PTR_TO_OFS(ptr_name) ofs << print_shift << std::setfill(' ') << std::setw(25) << std::left << std::string(#ptr_name) + ": " << report->ptr_name << std::endl;
MOS_STATUS CodechalDebugInterface::DumpEncodeStatusReport(const encode::EncodeStatusReportData *report)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(report);

    const char *bufferName = "EncodeStatusReport_Parsed";
    const char *attrName   = MediaDbgAttr::attrStatusReport;
    if (!m_configMgr->AttrIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }

    const char *  filePath = CreateFileName(bufferName, attrName, MediaDbgExtType::txt);
    std::ofstream ofs(filePath);

    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }
    std::string print_shift = "";
    sizeof(report->codecStatus);
    FIELD_TO_OFS(codecStatus);
    FIELD_TO_OFS(statusReportNumber);
    FIELD_TO_OFS(currOriginalPic.FrameIdx);
    FIELD_TO_OFS(currOriginalPic.PicFlags);
    FIELD_TO_OFS(currOriginalPic.PicEntry);
    FIELD_TO_OFS(func);
    PTR_TO_OFS(  currRefList);
    ofs << std::endl;

    FIELD_TO_OFS(sequential);
    FIELD_TO_OFS(bitstreamSize);
    FIELD_TO_OFS(qpY);
    FIELD_TO_OFS(suggestedQPYDelta);
    FIELD_TO_OFS(numberPasses);
    FIELD_TO_OFS(averageQP);
    FIELD_TO_OFS(hwCounterValue.IV);
    FIELD_TO_OFS(hwCounterValue.Count);
    PTR_TO_OFS(hwCtr);
    FIELD_TO_OFS(queryStatusFlags);

    print_shift = "    ";
    FIELD_TO_OFS(panicMode);
    FIELD_TO_OFS(sliceSizeOverflow);
    FIELD_TO_OFS(numSlicesNonCompliant);
    FIELD_TO_OFS(longTermReference);
    FIELD_TO_OFS(frameSkipped);
    FIELD_TO_OFS(sceneChangeDetected);
    print_shift = "";
    ofs << std::endl;

    FIELD_TO_OFS(mad);
    FIELD_TO_OFS(loopFilterLevel);
    FIELD_TO_OFS(longTermIndication);
    FIELD_TO_OFS(nextFrameWidthMinus1);
    FIELD_TO_OFS(nextFrameHeightMinus1);
    FIELD_TO_OFS(numberSlices);

    FIELD_TO_OFS(psnrX100[0]);
    FIELD_TO_OFS(psnrX100[1]);
    FIELD_TO_OFS(psnrX100[2]);

    FIELD_TO_OFS(numberTilesInFrame);
    FIELD_TO_OFS(usedVdBoxNumber);
    FIELD_TO_OFS(sizeOfSliceSizesBuffer);
    PTR_TO_OFS(  sliceSizes);
    FIELD_TO_OFS(sizeOfTileInfoBuffer);
    PTR_TO_OFS(  hevcTileinfo);
    FIELD_TO_OFS(numTileReported);
    ofs << std::endl;

    FIELD_TO_OFS(streamId);
    PTR_TO_OFS(  pLookaheadStatus);
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

#undef FIELD_TO_OFS
#undef PTR_TO_OFS

bool CodechalDebugInterface::DumpIsEnabled(
    const char *              attr,
    CODECHAL_MEDIA_STATE_TYPE mediaState)
{
    if (nullptr == m_configMgr)
    {
        return false;
    }

    if (mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        return static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, attr);
    }
    else
    {
        return m_configMgr->AttrIsEnabled(attr);
    }
}

MOS_STATUS CodechalDebugInterface::SetFastDumpConfig(MediaCopyWrapper *mediaCopyWrapper)
{
    auto traceSetting = MosUtilities::GetTraceSetting();
    if (!mediaCopyWrapper || !(DumpIsEnabled(MediaDbgAttr::attrEnableFastDump) || traceSetting))
    {
        return MOS_STATUS_SUCCESS;
    }

    MediaDebugFastDump::Config cfg{};
    if (traceSetting)
    {
        const auto &c           = traceSetting->fastDump;
        cfg.allowDataLoss       = c.allowDataLoss;
        cfg.frameIdx            = c.frameIdxBasedSampling ? &m_bufferDumpFrameNum : nullptr;
        cfg.samplingTime        = static_cast<size_t>(c.samplingTime);
        cfg.samplingInterval    = static_cast<size_t>(c.samplingInterval);
        cfg.memUsagePolicy      = c.memUsagePolicy;
        cfg.maxPrioritizedMem   = c.maxPrioritizedMem;
        cfg.maxDeprioritizedMem = c.maxDeprioritizedMem;
        cfg.weightRenderCopy    = c.weightRenderCopy;
        cfg.weightVECopy        = c.weightVECopy;
        cfg.weightBLTCopy       = c.weightBLTCopy;
        cfg.writeMode           = c.writeMode;
        cfg.bufferSize          = static_cast<size_t>(c.bufferSize);
        cfg.informOnError       = c.informOnError;

        auto suffix = cfg.writeMode == 0 ? ".bin" : cfg.writeMode == 1 ? ".txt"
                                                                      : "";

        class DumpEnabled
        {
        public:
            bool operator()(const char *attrName)
            {
                decltype(m_filter)::const_iterator it;
                return (it = m_filter.find(attrName)) != m_filter.end() &&
                       MOS_TraceKeyEnabled(it->second);
            }

        private:
            const std::map<std::string, MEDIA_EVENT_FILTER_KEYID> m_filter = {
                {MediaDbgAttr::attrDecodeOutputSurface, TR_KEY_DECODE_DSTYUV},
                {MediaDbgAttr::attrDecodeReferenceSurfaces, TR_KEY_DECODE_REFYUV},
                {MediaDbgAttr::attrDecodeBitstream, TR_KEY_DECODE_BITSTREAM},
                {MediaDbgAttr::attrEncodeRawInputSurface, TR_KEY_ENCODE_DATA_INPUT_SURFACE},
                {MediaDbgAttr::attrReferenceSurfaces, TR_KEY_ENCODE_DATA_REF_SURFACE},
                {MediaDbgAttr::attrReconstructedSurface, TR_KEY_ENCODE_DATA_RECON_SURFACE},
                {MediaDbgAttr::attrBitstream, TR_KEY_ENCODE_DATA_BITSTREAM},
                {MediaDbgAttr::attrHuCDmem, TR_KEY_ENCODE_DATA_HUC_DMEM},
                {MediaDbgAttr::attrHucRegions, TR_KEY_ENCODE_DATA_HUC_REGION},
            };
        };

        auto dumpEnabled = std::make_shared<DumpEnabled>();

        m_dumpYUVSurface = [this, dumpEnabled, traceSetting, suffix](
                               PMOS_SURFACE              surface,
                               const char               *attrName,
                               const char               *surfName,
                               CODECHAL_MEDIA_STATE_TYPE mediaState,
                               uint32_t,
                               uint32_t) {
            if ((*dumpEnabled)(attrName))
            {
                MediaDebugFastDump::Dump(
                    surface->OsResource,
                    std::string(traceSetting->fastDump.filePath) +
                        std::to_string(m_bufferDumpFrameNum) +
                        '-' +
                        surfName +
                        "w[0]_h[0]_p[0]" +
                        suffix);
            }
            return MOS_STATUS_SUCCESS;
        };

        m_dumpBuffer = [this, dumpEnabled, traceSetting, suffix](
                           PMOS_RESOURCE             resource,
                           const char               *attrName,
                           const char               *compName,
                           uint32_t                  size,
                           uint32_t                  offset,
                           CODECHAL_MEDIA_STATE_TYPE mediaState) {
            if ((*dumpEnabled)(attrName))
            {
                std::string bufferName = "";

                if (!strcmp(attrName, "DecodeBitstream") || !strcmp(attrName, "Bitstream"))
                {
                    bufferName = "_Bitstream";
                }

                MediaDebugFastDump::Dump(
                    *resource,
                    std::string(traceSetting->fastDump.filePath) +
                        std::to_string(m_bufferDumpFrameNum) +
                        '-' +
                        compName + bufferName +
                        suffix,
                    size,
                    offset,
                    MediaDebugSerializer<uint32_t>());
            }
            return MOS_STATUS_SUCCESS;
        };
    }
    else
    {
        cfg.allowDataLoss = false;
        cfg.informOnError = false;

        MediaUserSetting::Value outValue{};
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            "Enable VECopy For Surface Dump",
            MediaUserSetting::Group::Sequence);

        if (outValue.Get<bool>())
        {
            // use VE copy
            cfg.weightRenderCopy = 0;
            cfg.weightVECopy     = 100;
            cfg.weightBLTCopy    = 0;
        }
    }

    MediaDebugFastDump::CreateInstance(*m_osInterface, *mediaCopyWrapper, &cfg);

    return MOS_STATUS_SUCCESS;
}

const char *CodechalDebugInterface::CreateFileName(
    const char *funcName,
    const char *bufType,
    const char *extType)
{
    if (nullptr == funcName || nullptr == extType)
    {
        return nullptr;
    }

    char frameType = 'X';
    // Sets the frameType label
    if (m_frameType == I_TYPE)
    {
        frameType = 'I';
    }
    else if (m_frameType == P_TYPE)
    {
        frameType = 'P';
    }
    else if (m_frameType == B_TYPE)
    {
        frameType = 'B';
    }
    else if (m_frameType == MIXED_TYPE)
    {
        frameType = 'M';
    }

    const char *fieldOrder;
    // Sets the Field Order label
    if (CodecHal_PictureIsTopField(m_currPic))
    {
        fieldOrder = MediaDbgFieldType::topField;
    }
    else if (CodecHal_PictureIsBottomField(m_currPic))
    {
        fieldOrder = MediaDbgFieldType::botField;
    }
    else
    {
        fieldOrder = MediaDbgFieldType::frame;
    }

    // Sets the Postfix label
    if (m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary) &&
        strcmp(extType, MediaDbgExtType::txt) == 0)
    {
        extType = MediaDbgExtType::dat;
    }

    if (bufType != nullptr &&
        !strncmp(bufType, MediaDbgBufferType::bufSlcParams, sizeof(MediaDbgBufferType::bufSlcParams) - 1) && !strncmp(funcName, "_DDIEnc", sizeof("_DDIEnc") - 1))
    {
        m_outputFileName = m_outputFilePath +
                           std::to_string(m_bufferDumpFrameNum) + '-' +
                           std::to_string(m_streamId) + '_' +
                           std::to_string(m_sliceId + 1) +
                           funcName + '_' + bufType + '_' + frameType + fieldOrder + extType;
    }
    else if (bufType != nullptr &&
             !strncmp(bufType, MediaDbgBufferType::bufEncodePar, sizeof(MediaDbgBufferType::bufEncodePar) - 1))
    {
        if (!strncmp(funcName, "EncodeSequence", sizeof("EncodeSequence") - 1))
        {
            m_outputFileName = m_outputFilePath +
                               std::to_string(m_streamId) + '_' +
                               funcName + extType;
        }
        else
        {
            m_outputFileName = m_outputFilePath +
                               std::to_string(m_bufferDumpFrameNum) + '-' +
                               std::to_string(m_streamId) + '_' +
                               funcName + frameType + fieldOrder + extType;
        }
    }
    else
    {
        if (funcName[0] == '_')
            funcName += 1;

        if (bufType != nullptr)
        {
            m_outputFileName = m_outputFilePath +
                               std::to_string(m_bufferDumpFrameNum) + '-' +
                               std::to_string(m_streamId) + '_' +
                               funcName + '_' + bufType + '_' + frameType + fieldOrder + extType;
        }
        else
        {
            m_outputFileName = m_outputFilePath +
                               std::to_string(m_bufferDumpFrameNum) + '-' +
                               std::to_string(m_streamId) + '_' +
                               funcName + '_' + frameType + fieldOrder + extType;
        }
    }

    return m_outputFileName.c_str();
}

MOS_STATUS CodechalDebugInterface::DumpStringStream(std::stringstream& ss, const char* bufferName, const char* attrName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MEDIA_DEBUG_CHK_NULL(bufferName);
    MEDIA_DEBUG_CHK_NULL(attrName);

    if (!m_configMgr->AttrIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }

    const char* filePath = CreateFileName(bufferName, nullptr, MediaDbgExtType::txt);
    std::ofstream ofs(filePath);
    ofs << ss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpCmdBuffer(
    PMOS_COMMAND_BUFFER       cmdBuffer,
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    const char *              cmdName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    bool attrEnabled = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrCmdBufferMfx);

    if (!attrEnabled && mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrCmdBuffer);
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    bool binaryDumpEnabled = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpCmdBufInBinary);

    std::string funcName = cmdName ? cmdName : static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);
    const char *fileName = CreateFileName(
        funcName.c_str(),
        MediaDbgBufferType::bufCmd,
        binaryDumpEnabled ? MediaDbgExtType::dat : MediaDbgExtType::txt);

    if (m_configMgr->AttrIsEnabled(CodechalDbgAttr::attrEnableFastDump) && MediaDebugFastDump::IsGood())
    {
        MediaDebugFastDump::Dump(
            (uint8_t *)cmdBuffer->pCmdBase,
            fileName,
            (uint32_t)cmdBuffer->iOffset,
            0,
            MediaDebugSerializer<uint32_t>());
    }
    else
    {
        if (binaryDumpEnabled)
        {
            DumpBufferInBinary((uint8_t *)cmdBuffer->pCmdBase, (uint32_t)cmdBuffer->iOffset);
        }
        else
        {
            DumpBufferInHexDwords((uint8_t *)cmdBuffer->pCmdBase, (uint32_t)cmdBuffer->iOffset);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::Dump2ndLvlBatch(
    PMHW_BATCH_BUFFER         batchBuffer,
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    const char *              batchName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MEDIA_DEBUG_CHK_NULL(batchBuffer);

    bool attrEnabled = m_configMgr->AttrIsEnabled(MediaDbgAttr::attr2ndLvlBatchMfx);

    if (!attrEnabled && mediaState != CODECHAL_NUM_MEDIA_STATES)
    {
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attr2ndLvlBatch);
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }
    
    std::string funcName = batchName ? batchName : static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);
    const char *fileName = CreateFileName(
        funcName.c_str(),
        MediaDbgBufferType::buf2ndLvl,
        MediaDbgExtType::txt);

    MOS_STATUS status = MOS_STATUS_SUCCESS;
    if ((batchBuffer->OsResource.pGmmResInfo != nullptr) && 
        (batchBuffer->OsResource.pGmmResInfo->GetResFlags().Info.NotLockable))
    {
        uint8_t      *data            = nullptr;
        PMOS_RESOURCE pReadbackBuffer = (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE));
        CODECHAL_DEBUG_CHK_NULL(pReadbackBuffer);
        status = AllocateLinearResource(MOS_GFXRES_BUFFER, (uint32_t)batchBuffer->OsResource.pGmmResInfo->GetBaseWidth(), (uint32_t)batchBuffer->OsResource.pGmmResInfo->GetBaseHeight(), Format_Any, pReadbackBuffer);
        if (MOS_FAILED(status))
        {
            MOS_FreeMemAndSetNull(pReadbackBuffer);
        }
        CODECHAL_DEBUG_CHK_STATUS(status);
        status = VDBypassCopyResource(&batchBuffer->OsResource, pReadbackBuffer);
        if (MOS_FAILED(status))
        {
            m_osInterface->pfnFreeResource(m_osInterface, pReadbackBuffer);
            MOS_FreeMemAndSetNull(pReadbackBuffer);
        }
        CODECHAL_DEBUG_CHK_STATUS(status);

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly = 1;
        data               = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, pReadbackBuffer, &lockFlags);
        if (data == nullptr)
        {
            m_osInterface->pfnFreeResource(m_osInterface, pReadbackBuffer);
            MOS_FreeMemAndSetNull(pReadbackBuffer);
        }
        CODECHAL_DEBUG_CHK_NULL(data);

        if (DumpIsEnabled(MediaDbgAttr::attrEnableFastDump) && MediaDebugFastDump::IsGood())
        {
            MediaDebugFastDump::Dump(
                data + batchBuffer->dwOffset,
                fileName,
                (uint32_t)batchBuffer->iLastCurrent,
                0,
                MediaDebugSerializer<uint32_t>());
        }
        else
        {
            DumpBufferInHexDwords(data + batchBuffer->dwOffset,
                (uint32_t)batchBuffer->iLastCurrent);
        }

        m_osInterface->pfnUnlockResource(m_osInterface, pReadbackBuffer);
        m_osInterface->pfnFreeResource(m_osInterface, pReadbackBuffer);
        MOS_FreeMemAndSetNull(pReadbackBuffer);
        CODECHAL_DEBUG_CHK_STATUS(status);
    }
    else
    {
        bool batchLockedForDebug = !batchBuffer->bLocked;

        if (batchLockedForDebug)
        {
            (Mhw_LockBb(m_osInterface, batchBuffer));
        }

        batchBuffer->pData += batchBuffer->dwOffset;

        if (DumpIsEnabled(MediaDbgAttr::attrEnableFastDump) && MediaDebugFastDump::IsGood())
        {
            MediaDebugFastDump::Dump(
                batchBuffer->pData,
                fileName,
                (uint32_t)batchBuffer->iLastCurrent,
                0,
                MediaDebugSerializer<uint32_t>());
        }
        else
        {
            DumpBufferInHexDwords(batchBuffer->pData,
                (uint32_t)batchBuffer->iLastCurrent);
        }

        if (batchLockedForDebug)
        {
            (Mhw_UnlockBb(m_osInterface, batchBuffer, false));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpCurbe(
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    PMHW_KERNEL_STATE         kernelState)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    if (mediaState >= CODECHAL_NUM_MEDIA_STATES ||
        !static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrCurbe))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::string funcName   = static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);
    bool        binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);

    const char *fileName = CreateFileName(
        funcName.c_str(),
        MediaDbgBufferType::bufCurbe,
        MediaDbgExtType::txt);

    return kernelState->m_dshRegion.Dump(
        fileName,
        kernelState->dwCurbeOffset,
        kernelState->KernelParams.iCurbeLength,
        binaryDump);
}

MOS_STATUS CodechalDebugInterface::DumpMDFCurbe(
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    uint8_t *                 curbeBuffer,
    uint32_t                  curbeSize)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    uint8_t *  curbeAlignedData = nullptr;
    uint32_t   curbeAlignedSize = 0;
    MOS_STATUS eStatus          = MOS_STATUS_SUCCESS;

    if (mediaState >= CODECHAL_NUM_MEDIA_STATES ||
        !static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrCurbe))
    {
        return eStatus;
    }

    std::string funcName   = static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);
    bool        binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);
    const char *extType    = binaryDump ? MediaDbgExtType::dat : MediaDbgExtType::txt;

    const char *fileName = CreateFileName(
        funcName.c_str(),
        MediaDbgBufferType::bufCurbe,
        extType);

    curbeAlignedSize = MOS_ALIGN_CEIL(curbeSize, 64);
    curbeAlignedData = (uint8_t *)malloc(curbeAlignedSize * sizeof(uint8_t));
    if (curbeAlignedData == nullptr)
    {
        eStatus = MOS_STATUS_NULL_POINTER;
        return eStatus;
    }

    MOS_ZeroMemory(curbeAlignedData, curbeAlignedSize);
    MOS_SecureMemcpy(curbeAlignedData, curbeSize, curbeBuffer, curbeSize);

    if (binaryDump)
    {
        eStatus = DumpBufferInBinary(curbeAlignedData, curbeAlignedSize);
    }
    else
    {
        eStatus = DumpBufferInHexDwords(curbeAlignedData, curbeAlignedSize);
    }

    free(curbeAlignedData);

    return eStatus;
}

MOS_STATUS CodechalDebugInterface::DumpKernelRegion(
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    MHW_STATE_HEAP_TYPE       stateHeap,
    PMHW_KERNEL_STATE         kernelState)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    uint8_t *sshData = nullptr;
    uint32_t sshSize = 0;

    MemoryBlock *regionBlock = nullptr;
    bool         attrEnabled = false;
    const char * bufferType;
    if (stateHeap == MHW_ISH_TYPE)
    {
        regionBlock = &kernelState->m_ishRegion;
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrIsh);
        bufferType  = MediaDbgBufferType::bufISH;
    }
    else if (stateHeap == MHW_DSH_TYPE)
    {
        regionBlock = &kernelState->m_dshRegion;
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrDsh);
        bufferType  = MediaDbgBufferType::bufDSH;
    }
    else
    {
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, MediaDbgAttr::attrSsh);
        bufferType  = MediaDbgBufferType::bufSSH;

        MEDIA_DEBUG_CHK_NULL(m_osInterface);
        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnGetIndirectStatePointer(
            m_osInterface,
            &sshData));
        sshData += kernelState->dwSshOffset;
        sshSize = kernelState->dwSshSize;
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    std::string funcName = static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);

    const char *fileName = CreateFileName(
        funcName.c_str(),
        bufferType,
        MediaDbgExtType::txt);

    bool binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);

    if (regionBlock)
    {
        return regionBlock->Dump(fileName, 0, 0, binaryDump);
    }
    else
    {
        return DumpBufferInHexDwords(sshData, sshSize);
    }
}

MOS_STATUS CodechalDebugInterface::DumpYUVSurfaceToBuffer(PMOS_SURFACE surface,
    uint8_t *                                                          buffer,
    uint32_t &                                                         size)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly     = 1;
    lockFlags.TiledAsTiled = 1;  // Bypass GMM CPU blit due to some issues in GMM CpuBlt function

    uint8_t *lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &surface->OsResource, &lockFlags);
    if (lockedAddr == nullptr)  // Failed to lock. Try to submit copy task and dump another surface
    {
        uint32_t        sizeToBeCopied = 0;
        MOS_GFXRES_TYPE ResType;

#if LINUX
        // Linux does not have OsResource->ResType
        ResType = surface->Type;
#else
        ResType = surface->OsResource.ResType;
#endif

        GMM_RESOURCE_FLAG gmmFlags  = surface->OsResource.pGmmResInfo->GetResFlags();
        bool              allocated = false;

        MEDIA_DEBUG_CHK_STATUS(ReAllocateSurface(
            &m_temp2DSurfForCopy,
            surface,
            "Temp2DSurfForSurfDumper",
            ResType));

        // Ensure allocated buffer size contains the source surface size
        if (m_temp2DSurfForCopy.OsResource.pGmmResInfo->GetSizeMainSurface() >= surface->OsResource.pGmmResInfo->GetSizeMainSurface())
        {
            sizeToBeCopied = (uint32_t)surface->OsResource.pGmmResInfo->GetSizeMainSurface();
        }

        if (sizeToBeCopied == 0)
        {
            // Currently, MOS's pfnAllocateResource does not support allocate a surface reference to another surface.
            // When the source surface is not created from Media, it is possible that we cannot allocate the same size as source.
            // For example, on Gen9, Render target might have GMM set CCS=1 MMC=0, but MOS cannot allocate surface with such combination.
            // When Gmm allocation parameter is different, the resulting surface size/padding/pitch will be differnt.
            // Once if MOS can support allocate a surface by reference another surface, we can do a bit to bit copy without problem.
            MEDIA_DEBUG_ASSERTMESSAGE("Cannot allocate correct size, failed to copy nonlockable resource");
            return MOS_STATUS_NULL_POINTER;
        }

        MEDIA_DEBUG_VERBOSEMESSAGE("Temp2DSurfaceForCopy width %d, height %d, pitch %d, TileType %d, bIsCompressed %d, CompressionMode %d",
            m_temp2DSurfForCopy.dwWidth,
            m_temp2DSurfForCopy.dwHeight,
            m_temp2DSurfForCopy.dwPitch,
            m_temp2DSurfForCopy.TileType,
            m_temp2DSurfForCopy.bIsCompressed,
            m_temp2DSurfForCopy.CompressionMode);

        if (CopySurfaceData_Vdbox(sizeToBeCopied, &surface->OsResource, &m_temp2DSurfForCopy.OsResource) != MOS_STATUS_SUCCESS)
        {
            MEDIA_DEBUG_ASSERTMESSAGE("CopyDataSurface_Vdbox failed");
            m_osInterface->pfnFreeResource(m_osInterface, &m_temp2DSurfForCopy.OsResource);
            return MOS_STATUS_NULL_POINTER;
        }
        lockedAddr = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_temp2DSurfForCopy.OsResource, &lockFlags);
        MEDIA_DEBUG_CHK_NULL(lockedAddr);
    }

    uint32_t sizeMain     = (uint32_t)(surface->OsResource.pGmmResInfo->GetSizeMainSurface());
    uint8_t *surfBaseAddr = (uint8_t *)MOS_AllocMemory(sizeMain);
    MEDIA_DEBUG_CHK_NULL(surfBaseAddr);

    Mos_SwizzleData(lockedAddr, surfBaseAddr, surface->TileType, MOS_TILE_LINEAR, sizeMain / surface->dwPitch, surface->dwPitch, 0);

    uint8_t *data = surfBaseAddr;
    data += surface->dwOffset + surface->YPlaneOffset.iYOffset * surface->dwPitch;

    uint32_t width  = surface->dwWidth;
    uint32_t height = surface->dwHeight;

    switch (surface->Format)
    {
    case Format_YUY2:
    case Format_Y216V:
    case Format_P010:
    case Format_P016:
        width = width << 1;
        break;
    case Format_Y216:
    case Format_Y210:  //422 10bit -- Y0[15:0]:U[15:0]:Y1[15:0]:V[15:0] = 32bits per pixel = 4Bytes per pixel
    case Format_Y410:  //444 10bit -- A[31:30]:V[29:20]:Y[19:10]:U[9:0] = 32bits per pixel = 4Bytes per pixel
    case Format_R10G10B10A2:
    case Format_AYUV:  //444 8bit  -- A[31:24]:Y[23:16]:U[15:8]:V[7:0] = 32bits per pixel = 4Bytes per pixel
    case Format_A8R8G8B8:
        width = width << 2;
        break;
    default:
        break;
    }

    uint32_t pitch = surface->dwPitch;
    if (surface->Format == Format_UYVY)
        pitch = width;

    if (CodecHal_PictureIsBottomField(m_currPic))
    {
        data += pitch;
    }

    if (CodecHal_PictureIsField(m_currPic))
    {
        pitch *= 2;
        height /= 2;
    }

    // write luma data to file
    for (uint32_t h = 0; h < height; h++)
    {
        MOS_SecureMemcpy(buffer, width, data, width);
        buffer += width;
        size += width;
        data += pitch;
    }

    if (surface->Format != Format_A8B8G8R8)
    {
        switch (surface->Format)
        {
        case Format_NV12:
        case Format_P010:
        case Format_P016:
            height >>= 1;
            break;
        case Format_Y416:
        case Format_AUYV:
        case Format_R10G10B10A2:
            height *= 2;
            break;
        case Format_YUY2:
        case Format_YUYV:
        case Format_YUY2V:
        case Format_Y216V:
        case Format_YVYU:
        case Format_UYVY:
        case Format_VYUY:
        case Format_Y216:  //422 16bit
        case Format_Y210:  //422 10bit
        case Format_P208:  //422 8bit
            break;
        case Format_422V:
        case Format_IMC3:
            height = height / 2;
            break;
        case Format_AYUV:
        default:
            height = 0;
            break;
        }

        uint8_t *vPlaneData = surfBaseAddr;
#ifdef LINUX
        data = surfBaseAddr + surface->UPlaneOffset.iSurfaceOffset;
        if (surface->Format == Format_422V || surface->Format == Format_IMC3)
        {
            vPlaneData = surfBaseAddr + surface->VPlaneOffset.iSurfaceOffset;
        }
#else
        data = surfBaseAddr + surface->UPlaneOffset.iLockSurfaceOffset;
        if (surface->Format == Format_422V || surface->Format == Format_IMC3)
        {
            vPlaneData = surfBaseAddr + surface->VPlaneOffset.iLockSurfaceOffset;
        }

#endif

        // write chroma data to file
        for (uint32_t h = 0; h < height; h++)
        {
            MOS_SecureMemcpy(buffer, width, data, width);
            buffer += width;
            size += width;
            data += pitch;
        }

        // write v planar data to file
        if (surface->Format == Format_422V || surface->Format == Format_IMC3)
        {
            for (uint32_t h = 0; h < height; h++)
            {
                MOS_SecureMemcpy(buffer, width, vPlaneData, width);
                buffer += width;
                size += width;
                vPlaneData += pitch;
            }
        }
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);

    MOS_FreeMemory(surfBaseAddr);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDebugInterface::DumpYUVSurface(
    PMOS_SURFACE              surface,
    const char *              attrName,
    const char *              surfName,
    CODECHAL_MEDIA_STATE_TYPE mediaState,
    uint32_t                  width_in,
    uint32_t                  height_in)
{
    return m_dumpYUVSurface(
        surface,
        attrName,
        surfName,
        mediaState,
        width_in,
        height_in);
}

MOS_STATUS CodechalDebugInterface::DumpBuffer(
    PMOS_RESOURCE             resource,
    const char *              attrName,
    const char *              bufferName,
    uint32_t                  size,
    uint32_t                  offset,
    CODECHAL_MEDIA_STATE_TYPE mediaState)
{
    return m_dumpBuffer(
        resource,
        attrName,
        bufferName,
        size,
        offset,
        mediaState);
}

MOS_STATUS CodechalDebugInterface::DumpSurface(
    PMOS_SURFACE              surface,
    const char *              attrName,
    const char *              surfaceName,
    CODECHAL_MEDIA_STATE_TYPE mediaState)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MEDIA_DEBUG_CHK_NULL(surface);
    MEDIA_DEBUG_CHK_NULL(attrName);
    MEDIA_DEBUG_CHK_NULL(surfaceName);

    bool attrEnabled = false;

    if (mediaState == CODECHAL_NUM_MEDIA_STATES)
    {
        attrEnabled = m_configMgr->AttrIsEnabled(attrName);
    }
    else
    {
        attrEnabled = static_cast<CodecDebugConfigMgr*>(m_configMgr)->AttrIsEnabled(mediaState, attrName);
    }

    if (!attrEnabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    bool        binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);
    const char *extType    = binaryDump ? MediaDbgExtType::dat : MediaDbgExtType::txt;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;
    uint8_t *data      = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &surface->OsResource, &lockFlags);
    MEDIA_DEBUG_CHK_NULL(data);

    std::string bufName = std::string(surfaceName) + "_w[" + std::to_string(surface->dwWidth) + "]_h[" + std::to_string(surface->dwHeight) + "]_p[" + std::to_string(surface->dwPitch) + "]";
    const char *fileName;
    if (mediaState == CODECHAL_NUM_MEDIA_STATES)
    {
        fileName = CreateFileName(bufName.c_str(), nullptr, extType);
    }
    else
    {
        std::string kernelName = static_cast<CodecDebugConfigMgr*>(m_configMgr)->GetMediaStateStr(mediaState);
        fileName               = CreateFileName(kernelName.c_str(), bufName.c_str(), extType);
    }

    MOS_STATUS status;
    if (binaryDump)
    {
        status = Dump2DBufferInBinary(data, surface->dwWidth, surface->dwHeight, surface->dwPitch);
    }
    else
    {
        status = DumpBufferInHexDwords(data, surface->dwHeight * surface->dwPitch);
    }

    if (data)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &surface->OsResource);
    }

    return status;
}

MOS_STATUS CodechalDebugInterface::DumpData(
    void *      data,
    uint32_t    size,
    const char *attrName,
    const char *bufferName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MEDIA_DEBUG_CHK_NULL(data);
    MEDIA_DEBUG_CHK_NULL(attrName);
    MEDIA_DEBUG_CHK_NULL(bufferName);

    if (!m_configMgr->AttrIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }

    bool        binaryDump = m_configMgr->AttrIsEnabled(MediaDbgAttr::attrDumpBufferInBinary);
    const char *fileName   = CreateFileName(bufferName, nullptr, binaryDump ? MediaDbgExtType::dat : MediaDbgExtType::txt);

    if (binaryDump)
    {
        DumpBufferInBinary((uint8_t *)data, size);
    }
    else
    {
        DumpBufferInHexDwords((uint8_t *)data, size);
    }

    return MOS_STATUS_SUCCESS;
}

#define FIELD_TO_OFS(name, shift) ofs << shift #name << ": " << (int64_t)ptr->name << std::endl;
#define EMPTY_TO_OFS()
#define UNION_STRUCT_START_TO_OFS()     ofs << "union"      << std::endl \
                                            << "{"          << std::endl \
                                            << "    struct" << std::endl \
                                            << "    {"      << std::endl;

#define UNION_STRUCT_FIELD_TO_OFS(name) ofs << "        "#name << ": " << ptr->name << std::endl;
#define UNION_END_TO_OFS(name)          ofs << "    }"      << std::endl \
                                            << "    "#name << ": " << ptr->name << std::endl \
                                            << "}" << std::endl;
#define OFFSET_FIELD_TO_OFS(class_name, f_name, shift) << shift "                 "#f_name": " << ptr->class_name.f_name << std::endl
#define PLANE_OFFSET_TO_OFS(name) ofs << "MOS_PLANE_OFFSET "#name << std::endl \
                                                            OFFSET_FIELD_TO_OFS(name, iSurfaceOffset,)    \
                                                            OFFSET_FIELD_TO_OFS(name, iXOffset,)          \
                                                            OFFSET_FIELD_TO_OFS(name, iYOffset,)          \
                                                            OFFSET_FIELD_TO_OFS(name, iLockSurfaceOffset,);
#define RESOURCE_OFFSET_TO_OFS(name, shift) ofs << shift "MOS_RESOURCE_OFFSETS "#name << std::endl \
                                                                                OFFSET_FIELD_TO_OFS(name, BaseOffset, shift) \
                                                                                OFFSET_FIELD_TO_OFS(name, XOffset, shift)    \
                                                                                OFFSET_FIELD_TO_OFS(name, YOffset, shift);

#define FIELD_TO_OFS_8SHIFT(name) FIELD_TO_OFS(name, "        ")

MOS_STATUS CodechalDebugInterface::DumpSurfaceInfo(
    PMOS_SURFACE surface,
    const char* surfaceName)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MEDIA_DEBUG_CHK_NULL(surface);
    MEDIA_DEBUG_CHK_NULL(surfaceName);

    if (!m_configMgr->AttrIsEnabled(MediaDbgAttr::attrSurfaceInfo))
    {
        return MOS_STATUS_SUCCESS;
    }

    const char* funcName = (m_mediafunction == MEDIA_FUNCTION_VP) ? "_VP" : ((m_mediafunction == MEDIA_FUNCTION_ENCODE) ? "_ENC" : "_DEC");
    const char* filePath = CreateFileName(funcName, surfaceName, MediaDbgExtType::txt);
    std::ofstream ofs(filePath);
    PMOS_SURFACE ptr = surface;

    if (ofs.fail())
    {
        return MOS_STATUS_UNKNOWN;
    }

    ofs << "Surface name: " << surfaceName << std::endl;

    EMPTY_TO_OFS();
    ofs << "MOS_SURFACE:" << std::endl;
    FIELD_TO_OFS(dwArraySlice, );
    FIELD_TO_OFS(dwMipSlice, );
    FIELD_TO_OFS(S3dChannel, );

    EMPTY_TO_OFS();
    FIELD_TO_OFS(Type, );
    FIELD_TO_OFS(bOverlay, );
    FIELD_TO_OFS(bFlipChain, );

#if !defined(LINUX)
    EMPTY_TO_OFS();
    UNION_STRUCT_START_TO_OFS();
    UNION_STRUCT_FIELD_TO_OFS(dwFirstArraySlice);
    UNION_STRUCT_FIELD_TO_OFS(dwFirstMipSlice);
    UNION_END_TO_OFS(dwSubResourceIndex);
#endif

    EMPTY_TO_OFS();
    FIELD_TO_OFS(dwWidth, );
    FIELD_TO_OFS(dwHeight, );
    FIELD_TO_OFS(dwSize, );
    FIELD_TO_OFS(dwDepth, );
    FIELD_TO_OFS(dwArraySize, );
    FIELD_TO_OFS(dwLockPitch, );
    FIELD_TO_OFS(dwPitch, );
    FIELD_TO_OFS(dwSlicePitch, );
    FIELD_TO_OFS(dwQPitch, );
    FIELD_TO_OFS(TileType, );
    FIELD_TO_OFS(TileModeGMM, );
    FIELD_TO_OFS(bGMMTileEnabled, );
    FIELD_TO_OFS(Format, );
    FIELD_TO_OFS(bArraySpacing, );
    FIELD_TO_OFS(bCompressible, );

    EMPTY_TO_OFS();
    FIELD_TO_OFS(dwOffset, );
    PLANE_OFFSET_TO_OFS(YPlaneOffset);
    PLANE_OFFSET_TO_OFS(UPlaneOffset);
    PLANE_OFFSET_TO_OFS(VPlaneOffset);

    EMPTY_TO_OFS();
    UNION_STRUCT_START_TO_OFS();
    RESOURCE_OFFSET_TO_OFS(RenderOffset.YUV.Y, "    ");
    RESOURCE_OFFSET_TO_OFS(RenderOffset.YUV.U, "    ");
    RESOURCE_OFFSET_TO_OFS(RenderOffset.YUV.V, "    ");
    ofs << "    } YUV;" << std::endl;
    RESOURCE_OFFSET_TO_OFS(RenderOffset.RGB, );
    ofs << "}" << std::endl;

    EMPTY_TO_OFS();
    UNION_STRUCT_START_TO_OFS();
    UNION_STRUCT_FIELD_TO_OFS(LockOffset.YUV.Y);
    UNION_STRUCT_FIELD_TO_OFS(LockOffset.YUV.U);
    UNION_STRUCT_FIELD_TO_OFS(LockOffset.YUV.V);
    UNION_END_TO_OFS(LockOffset.RGB);

    EMPTY_TO_OFS();
    FIELD_TO_OFS(bIsCompressed, );
    FIELD_TO_OFS(CompressionMode, );
    FIELD_TO_OFS(CompressionFormat, );
    FIELD_TO_OFS(YoffsetForUplane, );
    FIELD_TO_OFS(YoffsetForVplane, );

    EMPTY_TO_OFS();
    EMPTY_TO_OFS();
    MOS_STATUS sts = DumpMosSpecificResourceInfoToOfs(&surface->OsResource, ofs);
    ofs.close();

    return sts;
}

#endif // USE_CODECHAL_DEBUG_TOOL
