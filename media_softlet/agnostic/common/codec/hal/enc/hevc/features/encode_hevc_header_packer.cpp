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

#include "encode_hevc_header_packer.h"
#include "encode_utils.h"

HevcHeaderPacker::HevcHeaderPacker()
{
    for (auto it = m_rbsp.begin(); it != m_rbsp.end(); ++it)
    {
        *it = 0;
    }
}

MOS_STATUS HevcHeaderPacker::GetNaluParams(uint8_t nal_unit_type_in, unsigned short layer_id_in, unsigned short temporal_id, mfxU16 long_start_code)
{
    m_naluParams.long_start_code       = long_start_code;
    m_naluParams.nal_unit_type         = static_cast<mfxU16>(nal_unit_type_in);
    m_naluParams.nuh_layer_id          = layer_id_in;
    m_naluParams.nuh_temporal_id_plus1 = temporal_id + 1;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcHeaderPacker::GetSPSParams(PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams)
{
    m_spsParams.log2_min_luma_coding_block_size_minus3   = hevcSeqParams->log2_min_coding_block_size_minus3;
    m_spsParams.log2_diff_max_min_luma_coding_block_size = hevcSeqParams->log2_max_coding_block_size_minus3 - hevcSeqParams->log2_min_coding_block_size_minus3;
    m_spsParams.pic_width_in_luma_samples                = (hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) * (1 << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
    m_spsParams.pic_height_in_luma_samples               = (hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) * (1 << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
    m_spsParams.separate_colour_plane_flag               = hevcSeqParams->separate_colour_plane_flag;
    m_spsParams.num_short_term_ref_pic_sets              = 0;  //NA
    m_spsParams.num_long_term_ref_pics_sps               = 0;  //NA
    m_spsParams.log2_max_pic_order_cnt_lsb_minus4        = 0;  //Related to CurrPicOrderCnt?
    m_spsParams.long_term_ref_pics_present_flag          = 0;  //NA, default in msdk
    m_spsParams.temporal_mvp_enabled_flag                = hevcSeqParams->sps_temporal_mvp_enable_flag;
    m_spsParams.sample_adaptive_offset_enabled_flag      = hevcSeqParams->SAO_enabled_flag;
    for (int i = 0; i < 65; i++)  //NA
    {
        m_spsParams.strps[i] = {};
    }                                                                                          //NA
    m_spsParams.chroma_format_idc                   = hevcSeqParams->chroma_format_idc;        //NA
    m_spsParams.high_precision_offsets_enabled_flag = 0;                                       //NA
    m_spsParams.bit_depth_chroma_minus8             = hevcSeqParams->bit_depth_chroma_minus8;  //NA
    m_bDssEnabled                                   = hevcSeqParams->SliceSizeControl;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcHeaderPacker::GetPPSParams(PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams)
{
    m_ppsParams.dependent_slice_segments_enabled_flag       = hevcPicParams->dependent_slice_segments_enabled_flag;
    m_ppsParams.num_extra_slice_header_bits                 = 0;
    m_ppsParams.output_flag_present_flag                    = 0;
    m_ppsParams.lists_modification_present_flag             = 0;                                    //NA
    m_ppsParams.cabac_init_present_flag                     = 1;                                    //NA
    m_ppsParams.weighted_pred_flag                          = hevcPicParams->weighted_pred_flag;    //NA
    m_ppsParams.weighted_bipred_flag                        = hevcPicParams->weighted_bipred_flag;  //NA
    m_ppsParams.slice_chroma_qp_offsets_present_flag        = 1;
    m_ppsParams.deblocking_filter_override_enabled_flag     = 0;                                              //hevcPicParams->deblocking_filter_override_enabled_flag; exist but not transfered from msdk
    m_ppsParams.loop_filter_across_slices_enabled_flag      = hevcPicParams->loop_filter_across_slices_flag;  //>=CNL,else =0
    m_ppsParams.tiles_enabled_flag                          = hevcPicParams->tiles_enabled_flag;
    m_ppsParams.entropy_coding_sync_enabled_flag            = hevcPicParams->entropy_coding_sync_enabled_flag;
    m_ppsParams.slice_segment_header_extension_present_flag = 0;
    m_ppsParams.deblocking_filter_disabled_flag             = hevcPicParams->pps_deblocking_filter_disabled_flag;
    m_ppsParams.beta_offset_div2                            = 0;
    m_ppsParams.tc_offset_div2                              = 0;
    m_sliceParams.no_output_of_prior_pics_flag              = hevcPicParams->no_output_of_prior_pics_flag;
    m_sliceParams.pic_parameter_set_id                      = hevcPicParams->slice_pic_parameter_set_id;
    m_sliceParams.collocated_ref_idx                        = hevcPicParams->CollocatedRefPicIndex;                                                                     //NA
    m_sliceParams.pic_order_cnt_lsb                         = hevcPicParams->CurrPicOrderCnt; // need reset poc_lsb later, store cur poc now for later lsb calculation
    nalType                                                 = hevcPicParams->nal_unit_type;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcHeaderPacker::GetSliceParams(const CODEC_HEVC_ENCODE_SLICE_PARAMS hevcSliceParams)
{
    m_sliceParams.first_slice_segment_in_pic_flag = (bool)!hevcSliceParams.slice_segment_address;  //?
    m_sliceParams.dependent_slice_segment_flag    = hevcSliceParams.dependent_slice_segment_flag;
    m_sliceParams.segment_address                 = hevcSliceParams.slice_segment_address;
    m_sliceParams.reserved_flags                  = 0;  //could be got, but seems always 0
    m_sliceParams.type                            = hevcSliceParams.slice_type;
    m_sliceParams.pic_output_flag                 = 1;
    m_sliceParams.colour_plane_id                 = 0;  //pre=0, unused
    m_sliceParams.short_term_ref_pic_set_sps_flag = 0;  //NA
    for (int i = 0; i < MAX_NUM_LONG_TERM_PICS; i++)    //NA
    {
        m_sliceParams.lt[i] = {};
    }                                                    //NA
    m_sliceParams.short_term_ref_pic_set_idx      = 0;   //NA
    m_sliceParams.strps                           = {};  //NA
    m_sliceParams.num_long_term_sps               = 0;   //NA
    m_sliceParams.num_long_term_pics              = 0;   //NA
    m_sliceParams.temporal_mvp_enabled_flag       = hevcSliceParams.slice_temporal_mvp_enable_flag;
    m_sliceParams.sao_luma_flag                   = hevcSliceParams.slice_sao_luma_flag;
    m_sliceParams.sao_chroma_flag                 = hevcSliceParams.slice_sao_chroma_flag;
    m_sliceParams.collocated_from_l0_flag         = hevcSliceParams.collocated_from_l0_flag;       //NA
    m_sliceParams.num_ref_idx_l0_active_minus1    = hevcSliceParams.num_ref_idx_l0_active_minus1;  //NA
    m_sliceParams.num_ref_idx_l1_active_minus1    = hevcSliceParams.num_ref_idx_l1_active_minus1;  //NA
    //m_sliceParams.ref_pic_list_modification_flag_lx={0};//NA
    //m_sliceParams.list_entry_lx=0;//NA
    m_sliceParams.num_ref_idx_active_override_flag       = 1;                                    //NA
    m_sliceParams.mvd_l1_zero_flag                       = hevcSliceParams.mvd_l1_zero_flag;     //NA
    m_sliceParams.cabac_init_flag                        = hevcSliceParams.cabac_init_flag;      //NA
    m_sliceParams.five_minus_max_num_merge_cand          = 5 - hevcSliceParams.MaxNumMergeCand;  //NA
    m_sliceParams.slice_qp_delta                         = hevcSliceParams.slice_qp_delta;       //transfered and recieved, but wrong data/ Maybe transfered too late?
    m_sliceParams.slice_cb_qp_offset                     = hevcSliceParams.slice_cb_qp_offset;
    m_sliceParams.slice_cr_qp_offset                     = hevcSliceParams.slice_cr_qp_offset;
    m_sliceParams.beta_offset_div2                       = hevcSliceParams.beta_offset_div2;
    m_sliceParams.tc_offset_div2                         = hevcSliceParams.tc_offset_div2;
    m_sliceParams.loop_filter_across_slices_enabled_flag = 1;  //>=CNL,else =0
    m_sliceParams.num_entry_point_offsets                = 0;  //*= !(m_ppsParams.tiles_enabled_flag || m_ppsParams.entropy_coding_sync_enabled_flag);
    m_sliceParams.luma_log2_weight_denom                 = 0;  //NA
    m_sliceParams.chroma_log2_weight_denom               = 0;  //NA
    m_sliceParams.deblocking_filter_disabled_flag        = hevcSliceParams.slice_deblocking_filter_disable_flag;
    m_sliceParams.deblocking_filter_override_flag        = (m_sliceParams.deblocking_filter_disabled_flag != m_ppsParams.deblocking_filter_disabled_flag);
    m_sliceParams.deblocking_filter_override_flag |=
        !m_sliceParams.deblocking_filter_disabled_flag && (m_sliceParams.beta_offset_div2 != m_ppsParams.beta_offset_div2 || m_sliceParams.tc_offset_div2 != m_ppsParams.tc_offset_div2);
    return MOS_STATUS_SUCCESS;
}

void HevcHeaderPacker::PackSSH(
    BitstreamWriter &bs,
    HevcNALU const & nalu,
    HevcSPS const &  sps,
    HevcPPS const &  pps,
    HevcSlice const &slice,
    bool             dyn_slice_size = false)
{
    PackNALU(bs, nalu);

    if (!dyn_slice_size)
        PackSSHPartIdAddr(bs, nalu, sps, pps, slice);

    if (!slice.dependent_slice_segment_flag)
        PackSSHPartIndependent(bs, nalu, sps, pps, slice);

    if (pps.tiles_enabled_flag || pps.entropy_coding_sync_enabled_flag)
    {
        ENCODE_ASSERT(slice.num_entry_point_offsets == 0);

        bs.PutUE(slice.num_entry_point_offsets);
    }

    ENCODE_ASSERT(0 == pps.slice_segment_header_extension_present_flag);

    if (!dyn_slice_size)  // no trailing bits for dynamic slice size
        bs.PutTrailingBits();
}

void HevcHeaderPacker::PackNALU(BitstreamWriter &bs, NALU const &h)
{
    bool bLong_SC =
        h.nal_unit_type == VPS_NUT || h.nal_unit_type == SPS_NUT || h.nal_unit_type == PPS_NUT || h.nal_unit_type == AUD_NUT || h.nal_unit_type == PREFIX_SEI_NUT || h.long_start_code;

    if (bLong_SC)
        bs.PutBits(8, 0);  //zero_byte

    bs.PutBits(24, 0x000001);  //start_code

    bs.PutBit(0);
    bs.PutBits(6, h.nal_unit_type);
    bs.PutBits(6, h.nuh_layer_id);
    bs.PutBits(3, h.nuh_temporal_id_plus1);
}

void HevcHeaderPacker::PackSSHPartIdAddr(
    BitstreamWriter &bs,
    NALU const &     nalu,
    SPS const &      sps,
    PPS const &      pps,
    Slice const &    slice)
{
    mfxU32 MaxCU          = (1 << (sps.log2_min_luma_coding_block_size_minus3 + 3 + sps.log2_diff_max_min_luma_coding_block_size));
    mfxU32 PicSizeInCtbsY = CeilDiv(sps.pic_width_in_luma_samples, MaxCU) * CeilDiv(sps.pic_height_in_luma_samples, MaxCU);

    bs.PutBit(slice.first_slice_segment_in_pic_flag);

    bool bIRAP = nalu.nal_unit_type >= BLA_W_LP && nalu.nal_unit_type <= RSV_IRAP_VCL23;

    if (bIRAP)
        bs.PutBit(slice.no_output_of_prior_pics_flag);

    bs.PutUE(slice.pic_parameter_set_id);

    if (!slice.first_slice_segment_in_pic_flag)
    {
        if (pps.dependent_slice_segments_enabled_flag)
            bs.PutBit(slice.dependent_slice_segment_flag);

        bs.PutBits(CeilLog2(PicSizeInCtbsY), slice.segment_address);
    }
}

void HevcHeaderPacker::PackSSHPartIndependent(
    BitstreamWriter &bs,
    NALU const &     nalu,
    SPS const &      sps,
    PPS const &      pps,
    Slice const &    slice)
{
    const mfxU8 I   = 2;
    mfxU32      nSE = 0;

    nSE += PutBits(bs, pps.num_extra_slice_header_bits, slice.reserved_flags);
    nSE += PutUE(bs, slice.type);
    nSE += pps.output_flag_present_flag && PutBit(bs, slice.pic_output_flag);
    nSE += (sps.separate_colour_plane_flag == 1) && PutBits(bs, 2, slice.colour_plane_id);

    bool bNonIDR = nalu.nal_unit_type != IDR_W_RADL && nalu.nal_unit_type != IDR_N_LP;

    if (bNonIDR)
        PackSSHPartNonIDR(bs, sps, slice);

    if (sps.sample_adaptive_offset_enabled_flag)
    {
        bs.AddInfo(PACK_SAOOffset, bs.GetOffset());

        nSE += PutBit(bs, slice.sao_luma_flag);
        nSE += PutBit(bs, slice.sao_chroma_flag);
    }

    if (slice.type != I)
        PackSSHPartPB(bs, sps, pps, slice);

    bs.AddInfo(PACK_QPDOffset, bs.GetOffset());

    nSE += PutSE(bs, slice.slice_qp_delta);
    nSE += pps.slice_chroma_qp_offsets_present_flag && PutSE(bs, slice.slice_cb_qp_offset);
    nSE += pps.slice_chroma_qp_offsets_present_flag && PutSE(bs, slice.slice_cr_qp_offset);
    nSE += pps.deblocking_filter_override_enabled_flag && PutBit(bs, slice.deblocking_filter_override_flag);
    nSE += slice.deblocking_filter_override_flag && PutBit(bs, slice.deblocking_filter_disabled_flag);

    bool bPackDblkOffsets = slice.deblocking_filter_override_flag && !slice.deblocking_filter_disabled_flag;
    nSE += bPackDblkOffsets && PutSE(bs, slice.beta_offset_div2);
    nSE += bPackDblkOffsets && PutSE(bs, slice.tc_offset_div2);

    bool bPackSliceLF =
        (pps.loop_filter_across_slices_enabled_flag && (slice.sao_luma_flag || slice.sao_chroma_flag || !slice.deblocking_filter_disabled_flag));

    nSE += bPackSliceLF && PutBit(bs, slice.loop_filter_across_slices_enabled_flag);

    ENCODE_ASSERT(nSE >= 2);
}

void HevcHeaderPacker::PackSSHPartNonIDR(
    BitstreamWriter &bs,
    SPS const &      sps,
    Slice const &    slice)
{
    mfxU32 nSE        = 0;
    bool   bNeedStIdx = slice.short_term_ref_pic_set_sps_flag && (sps.num_short_term_ref_pic_sets > 1);
    auto   PutSpsLT   = [&](const Slice::LongTerm &lt) {
        nSE += (sps.num_long_term_ref_pics_sps > 1) && PutBits(bs, CeilLog2(sps.num_long_term_ref_pics_sps), lt.lt_idx_sps);
        nSE += PutBit(bs, lt.delta_poc_msb_present_flag);
        nSE += lt.delta_poc_msb_present_flag && PutUE(bs, lt.delta_poc_msb_cycle_lt);
    };
    auto PutSliceLT = [&](const Slice::LongTerm &lt) {
        nSE += PutBits(bs, sps.log2_max_pic_order_cnt_lsb_minus4 + 4, lt.poc_lsb_lt);
        nSE += PutBit(bs, lt.used_by_curr_pic_lt_flag);
        nSE += PutBit(bs, lt.delta_poc_msb_present_flag);
        nSE += lt.delta_poc_msb_present_flag && PutUE(bs, lt.delta_poc_msb_cycle_lt);
    };

    nSE += PutBits(bs, sps.log2_max_pic_order_cnt_lsb_minus4 + 4, slice.pic_order_cnt_lsb);
    nSE += PutBit(bs, slice.short_term_ref_pic_set_sps_flag);

    if (!slice.short_term_ref_pic_set_sps_flag)
    {
        std::vector<STRPS> strps(sps.strps, sps.strps + sps.num_short_term_ref_pic_sets);
        strps.push_back(slice.strps);
        PackSTRPS(bs, strps.data(), sps.num_short_term_ref_pic_sets, sps.num_short_term_ref_pic_sets);
    }

    nSE += bNeedStIdx && PutBits(bs, CeilLog2(sps.num_short_term_ref_pic_sets), slice.short_term_ref_pic_set_idx);

    if (sps.long_term_ref_pics_present_flag)
    {
        nSE += (sps.num_long_term_ref_pics_sps > 0) && PutUE(bs, slice.num_long_term_sps);
        nSE += PutUE(bs, slice.num_long_term_pics);

        std::for_each(slice.lt, slice.lt + slice.num_long_term_sps, PutSpsLT);
        std::for_each(slice.lt, slice.lt + slice.num_long_term_pics, PutSliceLT);
    }

    nSE += sps.temporal_mvp_enabled_flag && PutBit(bs, slice.temporal_mvp_enabled_flag);

    ENCODE_ASSERT(nSE >= 2);
}

void HevcHeaderPacker::PackSTRPS(BitstreamWriter &bs, const STRPS *sets, mfxU32 num, mfxU32 idx)
{
    //This function is not needed for I frame.
    STRPS const &strps = sets[idx];

    if (idx != 0)
        bs.PutBit(strps.inter_ref_pic_set_prediction_flag);

    if (strps.inter_ref_pic_set_prediction_flag)
    {
        if (idx == num)
            bs.PutUE(strps.delta_idx_minus1);

        bs.PutBit(strps.delta_rps_sign);
        bs.PutUE(strps.abs_delta_rps_minus1);

        mfxU32 RefRpsIdx    = idx - (strps.delta_idx_minus1 + 1);
        mfxU32 NumDeltaPocs = sets[RefRpsIdx].num_negative_pics + sets[RefRpsIdx].num_positive_pics;

        std::for_each(
            strps.pic, strps.pic + NumDeltaPocs + 1, [&](const STRPS::Pic &pic) {
                bs.PutBit(pic.used_by_curr_pic_flag);

                if (!pic.used_by_curr_pic_flag)
                    bs.PutBit(pic.use_delta_flag);
            });
    }
    else
    {
        bs.PutUE(strps.num_negative_pics);
        bs.PutUE(strps.num_positive_pics);

        std::for_each(
            strps.pic, strps.pic + strps.num_positive_pics + strps.num_negative_pics, [&](const STRPS::Pic &pic) {
                bs.PutUE(pic.delta_poc_sx_minus1);
                bs.PutBit(pic.used_by_curr_pic_sx_flag);
            });
    }
};

void HevcHeaderPacker::PackSSHPartPB(
    BitstreamWriter &bs,
    SPS const &      sps,
    PPS const &      pps,
    Slice const &    slice)
{
    //This function is not needed for I frame.
    auto   IsSTUsed        = [](const STRPSPic &pic) { return !!pic.used_by_curr_pic_sx_flag; };
    auto   IsLTUsed        = [](const Slice::LongTerm &lt) { return !!lt.used_by_curr_pic_lt_flag; };
    bool   bB              = (slice.type == 0);
    mfxU32 nSE             = 0;
    auto   stEnd           = slice.strps.pic + slice.strps.num_negative_pics + slice.strps.num_positive_pics;
    auto   ltEnd           = slice.lt + slice.num_long_term_sps + slice.num_long_term_pics;
    mfxU16 NumPocTotalCurr = mfxU16(std::count_if(slice.strps.pic, stEnd, IsSTUsed) + std::count_if(slice.lt, ltEnd, IsLTUsed));
    bool   bNeedRefPicListM0      = pps.lists_modification_present_flag && NumPocTotalCurr > 1;
    bool   bNeedRefPicListM1      = bNeedRefPicListM0 && bB;
    bool   bNeedCRefIdx0   = (slice.collocated_from_l0_flag && slice.num_ref_idx_l0_active_minus1 > 0);
    bool   bNeedCRefIdx1   = (!slice.collocated_from_l0_flag && slice.num_ref_idx_l1_active_minus1 > 0);
    bool   bNeedCRefIdx    = slice.temporal_mvp_enabled_flag && (bNeedCRefIdx0 || bNeedCRefIdx1);
    auto   PackRefPicListM        = [&](mfxU8 lx, mfxU8 num) {
        nSE += PutBit(bs, !!slice.ref_pic_list_modification_flag_lx[lx]);

        num *= !!slice.ref_pic_list_modification_flag_lx[lx];

        std::for_each(slice.list_entry_lx[lx], slice.list_entry_lx[lx] + num, [&](mfxU8 entry) {
            nSE += PutBits(bs, CeilLog2(NumPocTotalCurr), entry);
        });
    };

    nSE += PutBit(bs, slice.num_ref_idx_active_override_flag);
    nSE += slice.num_ref_idx_active_override_flag && PutUE(bs, slice.num_ref_idx_l0_active_minus1);
    nSE += slice.num_ref_idx_active_override_flag && bB && PutUE(bs, slice.num_ref_idx_l1_active_minus1);

    if (bNeedRefPicListM0)
    {
        PackRefPicListM(0, slice.num_ref_idx_l0_active_minus1 + 1);
    }

    if (bNeedRefPicListM1)
    {
        PackRefPicListM(1, slice.num_ref_idx_l1_active_minus1 + 1);
    }

    nSE += bB && PutBit(bs, slice.mvd_l1_zero_flag);
    nSE += pps.cabac_init_present_flag && PutBit(bs, slice.cabac_init_flag);
    nSE += slice.temporal_mvp_enabled_flag && bB && PutBit(bs, slice.collocated_from_l0_flag);
    nSE += bNeedCRefIdx && PutUE(bs, slice.collocated_ref_idx);

    PackSSHPWT(bs, sps, pps, slice);

    nSE += PutUE(bs, slice.five_minus_max_num_merge_cand);

    ENCODE_ASSERT(nSE >= 2);
}

bool HevcHeaderPacker::PackSSHPWT(
    BitstreamWriter &bs, const SPS &sps, const PPS &pps, const Slice &slice)
{
    //This function is not needed for I frame.
    constexpr mfxU16 Y = 0, Cb = 1, Cr = 2, W = 0, O = 1, P = 1, B = 0;

    struct AccWFlag : std::pair<mfxU16, mfxI16>
    {
        AccWFlag(mfxU16 idx, mfxI16 w) : std::pair<mfxU16, mfxI16>(idx, w) {}
        mfxU16 operator()(mfxU16 wf, const mfxI16 (&pwt)[3][2])
        {
            return (wf << 1) + !(pwt[first][O] == 0 && pwt[first][W] == second);
        };
    };

    bool   bNeedY = (pps.weighted_pred_flag && slice.type == P) || (pps.weighted_bipred_flag && slice.type == B);
    bool   bNeedC = bNeedY && (sps.chroma_format_idc != 0);
    mfxI16 BdOffsetC =
        sps.high_precision_offsets_enabled_flag * (sps.bit_depth_chroma_minus8 + 8 - 1) + !sps.high_precision_offsets_enabled_flag * 7;
    mfxU32 nSE         = 0;
    mfxI16 WpOffC      = (1 << BdOffsetC);
    mfxI16 wY          = (1 << slice.luma_log2_weight_denom);
    mfxI16 wC          = (1 << slice.chroma_log2_weight_denom);
    mfxI16 l2WDc       = slice.chroma_log2_weight_denom;
    auto   startOffset = bs.GetOffset();

    auto PutPwtLX = [&](const mfxI16(&pwtLX)[16][3][2], mfxU32 sz) {
        mfxU32 szY      = sz * bNeedY;
        mfxU32 szC      = sz * bNeedC;
        mfxU16 wfmap    = (1 << (szY - 1));
        mfxU16 lumaw    = 0;
        mfxU16 chromaw  = 0;
        auto   PutWOVal = [&](const mfxI16(&pwt)[3][2]) {
            bool bPutY = !!(lumaw & wfmap);
            bool bPutC = !!(chromaw & wfmap);

            nSE += bPutY && PutSE(bs, pwt[Y][W] - wY);
            nSE += bPutY && PutSE(bs, pwt[Y][O]);

            nSE += bPutC && PutSE(bs, pwt[Cb][W] - wC);
            nSE += bPutC && PutSE(bs, clamp(((WpOffC * pwt[Cb][W]) >> l2WDc) + pwt[Cb][O] - WpOffC, -4 * WpOffC, 4 * WpOffC - 1));

            nSE += bPutC && PutSE(bs, pwt[Cb][W] - wC);
            nSE += bPutC && PutSE(bs, clamp(((WpOffC * pwt[Cr][W]) >> l2WDc) + pwt[Cr][O] - WpOffC, -4 * WpOffC, 4 * WpOffC - 1));

            wfmap >>= 1;
        };

        lumaw   = std::accumulate(pwtLX, pwtLX + szY, mfxU16(0), AccWFlag(Y, wY));
        chromaw = std::accumulate(pwtLX, pwtLX + szC, mfxU16(0), AccWFlag(Cb, wC));
        chromaw |= std::accumulate(pwtLX, pwtLX + szC, mfxU16(0), AccWFlag(Cr, wC));

        nSE += szY && PutBits(bs, szY, lumaw);
        nSE += szC && PutBits(bs, szC, chromaw);

        std::for_each(pwtLX, pwtLX + szY, PutWOVal);
    };

    bs.AddInfo(PACK_PWTOffset, startOffset);

    nSE += bNeedY && PutUE(bs, slice.luma_log2_weight_denom);
    nSE += bNeedC && PutSE(bs, mfxI32(slice.chroma_log2_weight_denom) - slice.luma_log2_weight_denom);

    PutPwtLX(slice.pwt[0], slice.num_ref_idx_l0_active_minus1 + 1);
    PutPwtLX(slice.pwt[1], (slice.num_ref_idx_l1_active_minus1 + 1) * (slice.type == B));

    bs.AddInfo(PACK_PWTLength, bs.GetOffset() - startOffset);

    return !!nSE;
}

MOS_STATUS HevcHeaderPacker::LoadSliceHeaderParams(CodecEncodeHevcSliceHeaderParams* pSH)
{
    ENCODE_CHK_NULL_RETURN(pSH);

    m_spsParams.log2_max_pic_order_cnt_lsb_minus4       = pSH->log2_max_pic_order_cnt_lsb_minus4;
    m_sliceParams.pic_order_cnt_lsb                     &= ~(0xFFFFFFFF << (m_spsParams.log2_max_pic_order_cnt_lsb_minus4 + 4));
    m_sliceParams.num_long_term_pics                    = pSH->num_long_term_pics;

    if(m_sliceParams.num_long_term_pics > MAX_NUM_LONG_TERM_PICS) 
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    for (int i = 0; i < m_sliceParams.num_long_term_pics; i++)
    {
        m_sliceParams.lt[i].used_by_curr_pic_lt_flag   = pSH->lt[i].used_by_curr_pic_lt_flag;
        m_sliceParams.lt[i].delta_poc_msb_present_flag = pSH->lt[i].delta_poc_msb_present_flag;
        m_sliceParams.lt[i].poc_lsb_lt                 = pSH->lt[i].poc_lsb_lt;
        m_sliceParams.lt[i].delta_poc_msb_cycle_lt     = pSH->lt[i].delta_poc_msb_cycle_lt;
    }
    m_ppsParams.lists_modification_present_flag         = pSH->lists_modification_present_flag;
    for (int i = 0; i < 2; i++)
    {
        m_sliceParams.ref_pic_list_modification_flag_lx[i] = pSH->ref_pic_list_modification_flag_lx[i];
        for (int j = 0; j < 16; j++)
        {
            m_sliceParams.list_entry_lx[i][j] = pSH->list_entry_lx[i][j];
        }
    }
    m_sliceParams.strps.num_negative_pics                   = pSH->num_negative_pics;
    m_sliceParams.strps.num_positive_pics                   = pSH->num_positive_pics;

    if (pSH->num_negative_pics > 16 || pSH->num_positive_pics > 16 || pSH->num_negative_pics + pSH->num_positive_pics > 16)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (int i = 0; i < pSH->num_negative_pics + pSH->num_positive_pics; i++)
    {
        if (i < pSH->num_negative_pics)
        {
            m_sliceParams.strps.pic[i].delta_poc_s0_minus1      = pSH->delta_poc_minus1[0][i];
            m_sliceParams.strps.pic[i].used_by_curr_pic_s0_flag = pSH->used_by_curr_pic_flag[0][i];
        }
        else
        {
            m_sliceParams.strps.pic[i].delta_poc_s1_minus1      = pSH->delta_poc_minus1[1][i - pSH->num_negative_pics];
            m_sliceParams.strps.pic[i].used_by_curr_pic_s1_flag = pSH->used_by_curr_pic_flag[1][i - pSH->num_negative_pics];
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcHeaderPacker::SliceHeaderPacker(EncoderParams *encodeParams)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_STATUS      eStatus;
    BitstreamWriter rbsp(m_rbsp.data(), (mfxU32)m_rbsp.size());
    mfxU8 *         pBegin      = rbsp.GetStart();
    mfxU8 *         startplace  = pBegin;
    mfxU8 *         pEnd        = pBegin + m_rbsp.size();
    mfxU32          BitLen;
    mfxU32          BitLenRecorded = 0;

    EncoderParams *pCodecHalEncodeParams = encodeParams;
    ENCODE_CHK_NULL_RETURN(pCodecHalEncodeParams);
    BSBuffer *pBSBuffer = pCodecHalEncodeParams->pBSBuffer;
    ENCODE_CHK_NULL_RETURN(pBSBuffer);
    PCODEC_ENCODER_SLCDATA pSlcData = (PCODEC_ENCODER_SLCDATA)pCodecHalEncodeParams->pSlcHeaderData;
    ENCODE_CHK_STATUS_RETURN(GetSPSParams(static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams)));
    ENCODE_CHK_STATUS_RETURN(GetPPSParams(static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams)));
    ENCODE_CHK_STATUS_RETURN(GetNaluParams(nalType, 0, 0, pBSBuffer->pCurrent == pBSBuffer->pBase));

    //uint8_t *pCurrent = pBSBuffer->pCurrent;
    //uint32_t
    for (uint32_t startLcu = 0, slcCount = 0; slcCount < encodeParams->dwNumSlices; slcCount++)
    {
        //startLcu += m_hevcSliceParams[slcCount].NumLCUsInSlice;
        ENCODE_CHK_STATUS_RETURN(GetSliceParams(static_cast<PCODEC_HEVC_ENCODE_SLICE_PARAMS>(encodeParams->pSliceParams)[slcCount]));
        ENCODE_CHK_STATUS_RETURN(LoadSliceHeaderParams((CodecEncodeHevcSliceHeaderParams*) pCodecHalEncodeParams->pSliceHeaderParams));
        
        rbsp.Reset(pBegin, mfxU32(pEnd - pBegin));
        m_naluParams.long_start_code = 0/*pBSBuffer->pCurrent + (BitLenRecorded + 7) / 8 == pBSBuffer->pBase*/;
        PackSSH(rbsp, m_naluParams, m_spsParams, m_ppsParams, m_sliceParams, m_bDssEnabled);
        BitLen = rbsp.GetOffset();
        pBegin += CeilDiv(BitLen, 8u);
        pSlcData[slcCount].SliceOffset            = (uint32_t)(pBSBuffer->pCurrent + (BitLenRecorded + 7) / 8 - pBSBuffer->pBase);
        pSlcData[slcCount].BitSize                = BitLen * 1 + (BitLen + 7) / 8 * !1;
        pSlcData[slcCount].SkipEmulationByteCount = 3 /*+ (pBSBuffer->pCurrent + (BitLenRecorded + 7) / 8 == pBSBuffer->pBase)*/;
        BitLenRecorded                            = BitLenRecorded + BitLen;
    }

    MOS_SecureMemcpy(pBSBuffer->pCurrent,
        (BitLenRecorded + 7) / 8,
        startplace,
        (BitLenRecorded + 7) / 8);

    return MOS_STATUS_SUCCESS;
}