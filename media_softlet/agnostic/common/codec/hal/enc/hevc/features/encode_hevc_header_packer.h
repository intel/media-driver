/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_hevc_header_packer.h
//! \brief    Defines the common interface for hevc packer
//!
#ifndef __ENCODE_HEVC_HEADER_PACKER_H__
#define __ENCODE_HEVC_HEADER_PACKER_H__

#include "bitstream_writer.h"
#include "codec_def_common_encode.h"
#include "codec_def_encode.h"
#include "codec_def_encode_hevc.h"
#include <exception>
#include <array>
#include <numeric>
#include <algorithm>

enum NALU_TYPE
{
    TRAIL_N = 0,
    TRAIL_R,
    TSA_N,
    TSA_R,
    STSA_N,
    STSA_R,
    RADL_N,
    RADL_R,
    RASL_N,
    RASL_R,
    RSV_VCL_N10,
    RSV_VCL_R11,
    RSV_VCL_N12,
    RSV_VCL_R13,
    RSV_VCL_N14,
    RSV_VCL_R15,
    BLA_W_LP,
    BLA_W_RADL,
    BLA_N_LP,
    IDR_W_RADL,
    IDR_N_LP,
    CRA_NUT,
    RSV_IRAP_VCL22,
    RSV_IRAP_VCL23,
    RSV_VCL24,
    RSV_VCL25,
    RSV_VCL26,
    RSV_VCL27,
    RSV_VCL28,
    RSV_VCL29,
    RSV_VCL30,
    RSV_VCL31,
    VPS_NUT,
    SPS_NUT,
    PPS_NUT,
    AUD_NUT,
    EOS_NUT,
    EOB_NUT,
    FD_NUT,
    PREFIX_SEI_NUT,
    SUFFIX_SEI_NUT,
    RSV_NVCL41,
    RSV_NVCL42,
    RSV_NVCL43,
    RSV_NVCL44,
    RSV_NVCL45,
    RSV_NVCL46,
    RSV_NVCL47,
    UNSPEC48,
    UNSPEC49,
    UNSPEC50,
    UNSPEC51,
    UNSPEC52,
    UNSPEC53,
    UNSPEC54,
    UNSPEC55,
    UNSPEC56,
    UNSPEC57,
    UNSPEC58,
    UNSPEC59,
    UNSPEC60,
    UNSPEC61,
    UNSPEC62,
    UNSPEC63,
    num_NALU_TYPE
};

enum ePackInfo
{
    PACK_QPDOffset = 0,
    PACK_SAOOffset,
    PACK_VUIOffset,
    PACK_PWTOffset,
    PACK_PWTLength,
    NUM_PACK_INFO
};

struct STRPS
{
    mfxU8 inter_ref_pic_set_prediction_flag : 1;
    mfxU8 delta_idx_minus1 : 6;
    mfxU8 delta_rps_sign : 1;

    mfxU8 num_negative_pics : 4;
    mfxU8 num_positive_pics : 4;

    mfxU16 abs_delta_rps_minus1;
    mfxU16 WeightInGop;

    struct Pic
    {
        mfxU8  used_by_curr_pic_flag : 1;
        mfxU8  use_delta_flag : 1;
        mfxI16 DeltaPocSX;

        union
        {
            struct
            {
                mfxU16 delta_poc_s0_minus1 : 15;
                mfxU16 used_by_curr_pic_s0_flag : 1;
            };
            struct
            {
                mfxU16 delta_poc_s1_minus1 : 15;
                mfxU16 used_by_curr_pic_s1_flag : 1;
            };
            struct
            {
                mfxU16 delta_poc_sx_minus1 : 15;
                mfxU16 used_by_curr_pic_sx_flag : 1;
            };
        };
    } pic[16];
};

struct HRDInfo
{
    mfxU16 nal_hrd_parameters_present_flag : 1;
    mfxU16 vcl_hrd_parameters_present_flag : 1;
    mfxU16 sub_pic_hrd_params_present_flag : 1;
    mfxU16 du_cpb_removal_delay_increment_length_minus1 : 5;
    mfxU16 sub_pic_cpb_params_in_pic_timing_sei_flag : 1;
    mfxU16 dpb_output_delay_du_length_minus1 : 5;

    mfxU16 tick_divisor_minus2 : 8;
    mfxU16 bit_rate_scale : 4;
    mfxU16 cpb_size_scale : 4;

    mfxU8  cpb_size_du_scale : 4;
    mfxU16 initial_cpb_removal_delay_length_minus1 : 5;
    mfxU16 au_cpb_removal_delay_length_minus1 : 5;
    mfxU16 dpb_output_delay_length_minus1 : 5;

    struct SubLayer
    {
        mfxU8 fixed_pic_rate_general_flag : 1;
        mfxU8 fixed_pic_rate_within_cvs_flag : 1;
        mfxU8 low_delay_hrd_flag : 1;

        mfxU16 elemental_duration_in_tc_minus1 : 11;
        mfxU16 cpb_cnt_minus1 : 5;

        struct CPB
        {
            mfxU32 bit_rate_value_minus1;
            mfxU32 cpb_size_value_minus1;
            mfxU32 cpb_size_du_value_minus1;
            mfxU32 bit_rate_du_value_minus1;
            mfxU8  cbr_flag;
        } cpb[32];
    } sl[8];
};

struct VUI
{
    mfxU8  aspect_ratio_info_present_flag : 1;
    mfxU8  aspect_ratio_idc;
    mfxU16 sar_width;
    mfxU16 sar_height;

    mfxU8 overscan_info_present_flag : 1;
    mfxU8 overscan_appropriate_flag : 1;
    mfxU8 video_signal_type_present_flag : 1;
    mfxU8 video_format : 3;
    mfxU8 video_full_range_flag : 1;
    mfxU8 colour_description_present_flag : 1;
    mfxU8 colour_primaries;
    mfxU8 transfer_characteristics;
    mfxU8 matrix_coeffs;

    mfxU8 chroma_loc_info_present_flag : 1;
    mfxU8 chroma_sample_loc_type_top_field : 3;
    mfxU8 chroma_sample_loc_type_bottom_field : 3;

    mfxU8 neutral_chroma_indication_flag : 1;
    mfxU8 field_seq_flag : 1;
    mfxU8 frame_field_info_present_flag : 1;
    mfxU8 default_display_window_flag : 1;

    mfxU32 def_disp_win_left_offset;
    mfxU32 def_disp_win_right_offset;
    mfxU32 def_disp_win_top_offset;
    mfxU32 def_disp_win_bottom_offset;

    mfxU8 timing_info_present_flag : 1;
    mfxU8 hrd_parameters_present_flag : 1;
    mfxU8 poc_proportional_to_timing_flag : 1;

    mfxU8 bitstream_restriction_flag : 1;
    mfxU8 tiles_fixed_structure_flag : 1;
    mfxU8 motion_vectors_over_pic_boundaries_flag : 1;
    mfxU8 restricted_ref_pic_lists_flag : 1;

    mfxU32 num_units_in_tick;
    mfxU32 time_scale;
    mfxU32 num_ticks_poc_diff_one_minus1;

    mfxU32 min_spatial_segmentation_idc : 12;
    mfxU32 max_bytes_per_pic_denom : 5;
    mfxU32 max_bits_per_min_cu_denom : 5;
    mfxU16 log2_max_mv_length_horizontal : 5;
    mfxU16 log2_max_mv_length_vertical : 4;

    HRDInfo hrd;
};

struct ScalingList
{
    mfxU8 scalingLists0[6][16];
    mfxU8 scalingLists1[6][64];
    mfxU8 scalingLists2[6][64];
    mfxU8 scalingLists3[2][64];
    mfxU8 scalingListDCCoefSizeID2[6];
    mfxU8 scalingListDCCoefSizeID3[2];
};

struct HevcSPS
{
    mfxU8 video_parameter_set_id : 4;
    mfxU8 max_sub_layers_minus1 : 3;
    mfxU8 temporal_id_nesting_flag : 1;

    mfxU8 seq_parameter_set_id : 4;
    mfxU8 chroma_format_idc : 2;
    mfxU8 separate_colour_plane_flag : 1;
    mfxU8 conformance_window_flag : 1;

    mfxU32 pic_width_in_luma_samples;
    mfxU32 pic_height_in_luma_samples;

    mfxU32 conf_win_left_offset;
    mfxU32 conf_win_right_offset;
    mfxU32 conf_win_top_offset;
    mfxU32 conf_win_bottom_offset;

    mfxU8 bit_depth_luma_minus8 : 3;
    mfxU8 bit_depth_chroma_minus8 : 3;
    mfxU8 log2_max_pic_order_cnt_lsb_minus4 : 4;
    //mfxU8  sub_layer_ordering_info_present_flag : 1;

    mfxU32 log2_min_luma_coding_block_size_minus3;
    mfxU32 log2_diff_max_min_luma_coding_block_size;
    mfxU32 log2_min_transform_block_size_minus2;
    mfxU32 log2_diff_max_min_transform_block_size;
    mfxU32 max_transform_hierarchy_depth_inter;
    mfxU32 max_transform_hierarchy_depth_intra;

    mfxU8 scaling_list_enabled_flag : 1;
    mfxU8 scaling_list_data_present_flag : 1;

    mfxU8 amp_enabled_flag : 1;
    mfxU8 sample_adaptive_offset_enabled_flag : 1;

    mfxU8  pcm_enabled_flag : 1;
    mfxU8  pcm_loop_filter_disabled_flag : 1;
    mfxU8  pcm_sample_bit_depth_luma_minus1 : 4;
    mfxU8  pcm_sample_bit_depth_chroma_minus1 : 4;
    mfxU32 log2_min_pcm_luma_coding_block_size_minus3;
    mfxU32 log2_diff_max_min_pcm_luma_coding_block_size;

    mfxU8 long_term_ref_pics_present_flag : 1;
    mfxU8 num_long_term_ref_pics_sps : 6;

    mfxU16 lt_ref_pic_poc_lsb_sps[32];
    mfxU8  used_by_curr_pic_lt_sps_flag[32];

    mfxU8 temporal_mvp_enabled_flag : 1;
    mfxU8 strong_intra_smoothing_enabled_flag : 1;
    mfxU8 vui_parameters_present_flag : 1;
    mfxU8 extension_flag : 1;
    mfxU8 extension_data_flag : 1;

    mfxU8 range_extension_flag : 1;
    mfxU8 transform_skip_rotation_enabled_flag : 1;
    mfxU8 transform_skip_context_enabled_flag : 1;
    mfxU8 implicit_rdpcm_enabled_flag : 1;
    mfxU8 explicit_rdpcm_enabled_flag : 1;
    mfxU8 extended_precision_processing_flag : 1;
    mfxU8 intra_smoothing_disabled_flag : 1;
    mfxU8 high_precision_offsets_enabled_flag : 1;
    mfxU8 persistent_rice_adaptation_enabled_flag : 1;
    mfxU8 cabac_bypass_alignment_enabled_flag : 1;

    mfxU8 ExtensionFlags;
    mfxU8 num_short_term_ref_pic_sets;
    STRPS strps[65];

    ScalingList scl;
    VUI         vui;
};

struct HevcPPS
{
    mfxU16 pic_parameter_set_id : 6;
    mfxU16 seq_parameter_set_id : 4;
    mfxU16 dependent_slice_segments_enabled_flag : 1;
    mfxU16 output_flag_present_flag : 1;
    mfxU16 num_extra_slice_header_bits : 3;
    mfxU16 sign_data_hiding_enabled_flag : 1;
    mfxU16 cabac_init_present_flag : 1;
    mfxU16 num_ref_idx_l0_default_active_minus1 : 4;
    mfxU16 num_ref_idx_l1_default_active_minus1 : 4;
    mfxU16 constrained_intra_pred_flag : 1;
    mfxU16 transform_skip_enabled_flag : 1;
    mfxU16 cu_qp_delta_enabled_flag : 1;
    mfxU16 slice_segment_header_extension_present_flag : 1;

    mfxU32 diff_cu_qp_delta_depth;
    mfxI32 init_qp_minus26;
    mfxI16 cb_qp_offset : 6;
    mfxI16 cr_qp_offset : 6;

    mfxU8 slice_chroma_qp_offsets_present_flag : 1;
    mfxU8 weighted_pred_flag : 1;
    mfxU8 weighted_bipred_flag : 1;
    mfxU8 transquant_bypass_enabled_flag : 1;
    mfxU8 tiles_enabled_flag : 1;
    mfxU8 entropy_coding_sync_enabled_flag : 1;
    mfxU8 uniform_spacing_flag : 1;
    mfxU8 loop_filter_across_tiles_enabled_flag : 1;

    mfxU16 num_tile_columns_minus1;
    mfxU16 num_tile_rows_minus1;

    mfxU16 column_width[MAX_NUM_TILE_COLUMNS - 1];
    mfxU16 row_height[MAX_NUM_TILE_ROWS - 1];

    mfxU8 loop_filter_across_slices_enabled_flag : 1;
    mfxU8 deblocking_filter_control_present_flag : 1;
    mfxU8 deblocking_filter_override_enabled_flag : 1;
    mfxU8 deblocking_filter_disabled_flag : 1;
    mfxU8 scaling_list_data_present_flag : 1;
    mfxU8 lists_modification_present_flag : 1;
    mfxU8 extension_flag : 1;
    mfxU8 extension_data_flag : 1;

    mfxI8 beta_offset_div2 : 4;
    mfxI8 tc_offset_div2 : 4;

    //ScalingListData* sld;

    mfxU16 log2_parallel_merge_level_minus2;

    mfxU32 range_extension_flag : 1;
    mfxU32 cross_component_prediction_enabled_flag : 1;
    mfxU32 chroma_qp_offset_list_enabled_flag : 1;
    mfxU32 log2_sao_offset_scale_luma : 3;
    mfxU32 log2_sao_offset_scale_chroma : 3;
    mfxU32 chroma_qp_offset_list_len_minus1 : 3;
    mfxU32 diff_cu_chroma_qp_offset_depth : 5;
    mfxU32 log2_max_transform_skip_block_size_minus2 : 5;
    mfxU32 : 10;
    mfxI8 cb_qp_offset_list[6];
    mfxI8 cr_qp_offset_list[6];

    mfxU8 ExtensionFlags;
};

struct HevcSlice
{
    mfxU8 no_output_of_prior_pics_flag : 1;
    mfxU8 pic_parameter_set_id : 6;
    mfxU8 dependent_slice_segment_flag : 1;

    mfxU32 segment_address;

    mfxU8 reserved_flags;
    mfxU8 type : 2;
    mfxU8 colour_plane_id : 2;
    mfxU8 short_term_ref_pic_set_idx;

    mfxU8 pic_output_flag : 1;
    mfxU8 short_term_ref_pic_set_sps_flag : 1;
    mfxU8 num_long_term_sps : 6;

    mfxU8 first_slice_segment_in_pic_flag : 1;
    mfxU8 temporal_mvp_enabled_flag : 1;
    mfxU8 sao_luma_flag : 1;
    mfxU8 sao_chroma_flag : 1;
    mfxU8 num_ref_idx_active_override_flag : 1;
    mfxU8 mvd_l1_zero_flag : 1;
    mfxU8 cabac_init_flag : 1;
    mfxU8 collocated_from_l0_flag : 1;

    mfxU8 collocated_ref_idx : 4;
    mfxU8 five_minus_max_num_merge_cand : 3;

    mfxU8 num_ref_idx_l0_active_minus1 : 4;
    mfxU8 num_ref_idx_l1_active_minus1 : 4;

    mfxU32 pic_order_cnt_lsb;
    mfxU16 num_long_term_pics;

    mfxI8  slice_qp_delta;
    mfxI16 slice_cb_qp_offset : 6;
    mfxI16 slice_cr_qp_offset : 6;

    mfxU8 deblocking_filter_override_flag : 1;
    mfxU8 deblocking_filter_disabled_flag : 1;
    mfxU8 loop_filter_across_slices_enabled_flag : 1;
    mfxU8 offset_len_minus1 : 5;

    mfxI8 beta_offset_div2 : 4;
    mfxI8 tc_offset_div2 : 4;

    mfxU32 num_entry_point_offsets;

    STRPS strps;

    struct LongTerm
    {
        mfxU8  lt_idx_sps : 5;
        mfxU8  used_by_curr_pic_lt_flag : 1;
        mfxU8  delta_poc_msb_present_flag : 1;
        mfxU32 poc_lsb_lt;
        mfxU32 delta_poc_msb_cycle_lt;
    } lt[MAX_NUM_LONG_TERM_PICS];

    mfxU8 ref_pic_list_modification_flag_lx[2];
    mfxU8 list_entry_lx[2][16];

    mfxU16 luma_log2_weight_denom : 3;
    mfxU16 chroma_log2_weight_denom : 3;
    mfxI16 pwt[2][16][3][2];  //[list][list entry][Y, Cb, Cr][weight, offset]
};

struct HevcNALU
{
    mfxU16 long_start_code;
    mfxU16 nal_unit_type;
    mfxU16 nuh_layer_id;
    mfxU16 nuh_temporal_id_plus1;
};

typedef HevcSPS   SPS;
typedef HevcPPS   PPS;
typedef HevcSlice Slice;
typedef HevcNALU  NALU;

using STRPSPic = STRPS::Pic;

class HevcHeaderPacker
{
public:
    BSBuffer *              m_bsBuffer      = nullptr;
    HevcNALU                m_naluParams    = {};
    HevcSPS                 m_spsParams     = {};
    HevcPPS                 m_ppsParams     = {};
    HevcSlice               m_sliceParams   = {};
    uint8_t                 nalType         = 0;
    std::array<mfxU8, 1024> m_rbsp          = {};
    bool                    m_bDssEnabled   = false;

public:
    HevcHeaderPacker();
    MOS_STATUS SliceHeaderPacker(EncoderParams *encodeParams);
    MOS_STATUS GetNaluParams(uint8_t nal_unit_type_in, unsigned short layer_id_in, unsigned short temporal_id, mfxU16 long_start_code);
    MOS_STATUS GetPPSParams(PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams);
    MOS_STATUS GetSPSParams(PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams);
    MOS_STATUS GetSliceParams(const CODEC_HEVC_ENCODE_SLICE_PARAMS hevcSliceParams);
    MOS_STATUS LoadSliceHeaderParams(CodecEncodeHevcSliceHeaderParams* pSH);
    void       PackSSH(
              BitstreamWriter &bs,
              HevcNALU const & nalu,
              HevcSPS const &  sps,
              HevcPPS const &  pps,
              HevcSlice const &slice,
              bool             dyn_slice_size);
    void PackNALU(BitstreamWriter &bs, NALU const &h);
    void PackSSHPartIdAddr(
        BitstreamWriter &bs,
        NALU const &     nalu,
        SPS const &      sps,
        PPS const &      pps,
        Slice const &    slice);
    template <class T>
    const T &clamp(const T &v, const T &lo, const T &hi)
    {
        return std::min<T>(hi, std::max<T>(v, lo));
    }
    template <class T>
    inline T CeilDiv(T x, T y)
    {
        return (x + y - 1) / y;
    }
    inline mfxU32 CeilLog2(mfxU32 x)
    {
        mfxU32 l = 0;
        while (x > (1U << l))
            ++l;
        return l;
    }
    void PackSSHPartIndependent(
        BitstreamWriter &bs,
        NALU const &     nalu,
        SPS const &      sps,
        PPS const &      pps,
        Slice const &    slice);

    void PackSSHPartNonIDR(
        BitstreamWriter &bs,
        SPS const &      sps,
        Slice const &    slice);

    void PackSTRPS(BitstreamWriter &bs, const STRPS *sets, mfxU32 num, mfxU32 idx);

    void PackSSHPartPB(
        BitstreamWriter &bs,
        SPS const &      sps,
        PPS const &      pps,
        Slice const &    slice);

    bool PackSSHPWT(
        BitstreamWriter &bs, const SPS &sps, const PPS &pps, const Slice &slice);

    static bool PutUE(BitstreamWriter &bs, mfxU32 b)
    {
        bs.PutUE(b);
        return true;
    };
    static bool PutSE(BitstreamWriter &bs, mfxI32 b)
    {
        bs.PutSE(b);
        return true;
    };
    static bool PutBit(BitstreamWriter &bs, mfxU32 b)
    {
        bs.PutBit(!!b);
        return true;
    };
    static bool PutBits(BitstreamWriter &bs, mfxU32 n, mfxU32 b)
    {
        if (n)
            bs.PutBits(n, b);
        return !!n;
    };
    inline void ThrowAssert(bool bThrow, const char *msg)
    {
        if (bThrow)
            throw std::logic_error(msg);
    }

MEDIA_CLASS_DEFINE_END(HevcHeaderPacker)
};

#endif