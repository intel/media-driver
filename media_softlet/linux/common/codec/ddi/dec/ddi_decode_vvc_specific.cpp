/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     ddi_decode_vvc_specific.cpp
//! \brief    VVC class definition for DDI media decoder
//!

#include "ddi_decode_functions.h"
#include "media_libva_util_next.h"
#include "ddi_decode_vvc_specific.h"
#include "mos_solo_generic.h"
#include "media_libva_interface_next.h"
#include "codec_def_common_vvc.h"

namespace decode
{

VAStatus DdiDecodeVvc::ParseTileParams(
    DDI_MEDIA_CONTEXT *mediaCtx,
    uint16_t          *tileParam,
    uint32_t          numTiles,
    uint32_t          numTileBuffers)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    CodecVvcPicParams *pVvcPicParams = static_cast<CodecVvcPicParams*>(m_decodeCtx->DecodeParams.m_picParams);
    if ((tileParam == nullptr) || (pVvcPicParams == nullptr))
    {
        DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing VVC Tile parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (vvcMaxTileParamsNum < (numTiles + numTileBuffers))
    {
        DDI_CODEC_ASSERTMESSAGE("numTiles = %d exceeds max size = %d", numTiles + numTileBuffers, vvcMaxTileParamsNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    uint16_t* pVvcTileParams = static_cast<uint16_t*>(m_decodeCtx->DecodeParams.m_tileParams);
    pVvcTileParams += numTileBuffers;
    MOS_ZeroMemory(pVvcTileParams, (numTiles * sizeof(uint16_t)));
    MOS_SecureMemcpy(pVvcTileParams, sizeof(uint16_t) * numTiles, tileParam, sizeof(uint16_t) * numTiles);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVvc::ParsePicParams(
    DDI_MEDIA_CONTEXT           *mediaCtx,
    VAPictureParameterBufferVVC *picParam)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    CodechalDecodeParams *decodeParams  = &m_decodeCtx->DecodeParams;
    CodecVvcPicParams    *pVvcPicParams = static_cast<CodecVvcPicParams*>(decodeParams->m_picParams);
    if ((picParam == nullptr) || (pVvcPicParams == nullptr))
    {
        DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing VVC Picture parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    MOS_ZeroMemory(pVvcPicParams, sizeof(CodecVvcPicParams));

    // Set up current frame
    pVvcPicParams->m_picOrderCntVal = picParam->CurrPic.pic_order_cnt;
    SetupCodecPicture(mediaCtx,
        &m_decodeCtx->RTtbl,
        &pVvcPicParams->m_currPic,
        picParam->CurrPic,
        VvcPicEntryCurrFrame);
    if (pVvcPicParams->m_currPic.FrameIdx >= CODEC_MAX_DPB_NUM_VVC)
    {
        DDI_CODEC_NORMALMESSAGE("CurrPic.FrameIdx is out of range.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // Set up reference frames
    MOS_ZeroMemory(m_refListFlags, sizeof(m_refListFlags));
    for (auto i = 0; i < vvcMaxNumRefFrame; i++)
    {
        m_refListFlags[i] = picParam->ReferenceFrames[i].flags;
        pVvcPicParams->m_refFramePocList[i] = picParam->ReferenceFrames[i].pic_order_cnt;
        SetupCodecPicture(mediaCtx,
            &m_decodeCtx->RTtbl,
            &pVvcPicParams->m_refFrameList[i],
            picParam->ReferenceFrames[i],
            VvcPicEntryRefFrameList);
        if(pVvcPicParams->m_refFrameList[i].FrameIdx > CODEC_MAX_DPB_NUM_VVC)
        {
            pVvcPicParams->m_refFrameList[i].FrameIdx = CODEC_MAX_DPB_NUM_VVC;
        }
    }

    // Set up RefPicList
    for (auto i = 0; i < 2; i++)
    {
        for (auto j = 0; j < vvcMaxNumRefFrame; j++)
        {
            pVvcPicParams->m_refPicList[i][j].PicFlags = PICTURE_INVALID;
        }
    }

    pVvcPicParams->m_ppsPicWidthInLumaSamples   =  picParam->pps_pic_width_in_luma_samples;
    pVvcPicParams->m_ppsPicHeightInLumaSamples  =  picParam->pps_pic_height_in_luma_samples;

    // slice-level params
    pVvcPicParams->m_spsPicWidthMaxInLumaSamples          = 0;
    pVvcPicParams->m_spsPicHeightMaxInLumaSamples         = 0;
    pVvcPicParams->m_spsNumSubpicsMinus1                  = picParam->sps_num_subpics_minus1;
    pVvcPicParams->m_spsChromaFormatIdc                   = picParam->sps_chroma_format_idc;
    pVvcPicParams->m_spsBitdepthMinus8                    = picParam->sps_bitdepth_minus8;
    pVvcPicParams->m_spsLog2CtuSizeMinus5                 = picParam->sps_log2_ctu_size_minus5;
    pVvcPicParams->m_spsLog2MinLumaCodingBlockSizeMinus2  = picParam->sps_log2_min_luma_coding_block_size_minus2;
    pVvcPicParams->m_spsLog2TransformSkipMaxSizeMinus2    = picParam->sps_log2_transform_skip_max_size_minus2;

    MOS_SecureMemcpy(&pVvcPicParams->m_chromaQpTable[0], 76 * sizeof(int8_t), &picParam->ChromaQpTable[0], 76 * sizeof(int8_t));
    MOS_SecureMemcpy(&pVvcPicParams->m_chromaQpTable[1], 76 * sizeof(int8_t), &picParam->ChromaQpTable[1], 76 * sizeof(int8_t));
    MOS_SecureMemcpy(&pVvcPicParams->m_chromaQpTable[2], 76 * sizeof(int8_t), &picParam->ChromaQpTable[2], 76 * sizeof(int8_t));

    pVvcPicParams->m_spsSixMinusMaxNumMergeCand           = picParam->sps_six_minus_max_num_merge_cand;
    pVvcPicParams->m_spsFiveMinusMaxNumSubblockMergeCand  = picParam->sps_five_minus_max_num_subblock_merge_cand;
    pVvcPicParams->m_spsMaxNumMergeCandMinusMaxNumGpmCand = picParam->sps_max_num_merge_cand_minus_max_num_gpm_cand;
    pVvcPicParams->m_spsLog2ParallelMergeLevelMinus2      = picParam->sps_log2_parallel_merge_level_minus2;
    pVvcPicParams->m_spsMinQpPrimeTs                      = picParam->sps_min_qp_prime_ts;
    pVvcPicParams->m_spsSixMinusMaxNumIbcMergeCand        = picParam->sps_six_minus_max_num_ibc_merge_cand;
    pVvcPicParams->m_spsNumLadfIntervalsMinus2            = picParam->sps_num_ladf_intervals_minus2;
    pVvcPicParams->m_spsLadfLowestIntervalQpOffset        = picParam->sps_ladf_lowest_interval_qp_offset;

    MOS_SecureMemcpy(pVvcPicParams->m_spsLadfQpOffset, 4 * sizeof(int8_t), picParam->sps_ladf_qp_offset, 4 * sizeof(int8_t));
    MOS_SecureMemcpy(pVvcPicParams->m_spsLadfDeltaThresholdMinus1, 4 * sizeof(uint16_t), picParam-> sps_ladf_delta_threshold_minus1, 4 * sizeof(uint16_t));

    // this def is not aligned btw VAAPI & HAL, need to parse each to get correct flag
    // parse spsFlagss0
    pVvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag          = picParam->sps_flags.bits.sps_subpic_info_present_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsIndependentSubpicsFlag         = picParam->sps_flags.bits.sps_independent_subpics_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsSubpicSameSizeFlag             = picParam->sps_flags.bits.sps_subpic_same_size_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsEntropyCodingSyncEnabledFlag   = picParam->sps_flags.bits.sps_entropy_coding_sync_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsQtbttDualTreeIntraFlag         = picParam->sps_flags.bits.sps_qtbtt_dual_tree_intra_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsMaxLumaTransformSize64Flag     = picParam->sps_flags.bits.sps_max_luma_transform_size_64_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsTransformSkipEnabledFlag       = picParam->sps_flags.bits.sps_transform_skip_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsBdpcmEnabledFlag               = picParam->sps_flags.bits.sps_bdpcm_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsMtsEnabledFlag                 = picParam->sps_flags.bits.sps_mts_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsExplicitMtsIntraEnabledFlag    = picParam->sps_flags.bits.sps_explicit_mts_intra_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsExplicitMtsInterEnabledFlag    = picParam->sps_flags.bits.sps_explicit_mts_inter_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsLfnstEnabledFlag               = picParam->sps_flags.bits.sps_lfnst_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsJointCbcrEnabledFlag           = picParam->sps_flags.bits.sps_joint_cbcr_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsSameQpTableForChromaFlag       = picParam->sps_flags.bits.sps_same_qp_table_for_chroma_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsSaoEnabledFlag                 = picParam->sps_flags.bits.sps_sao_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsAlfEnabledFlag                 = picParam->sps_flags.bits.sps_alf_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsCcalfEnabledFlag               = picParam->sps_flags.bits.sps_ccalf_enabled_flag;
    pVvcPicParams->m_spsFlags0.m_fields.m_spsLmcsEnabledFlag                = picParam->sps_flags.bits.sps_lmcs_enabled_flag;

    // parse spsFlagss1
    pVvcPicParams->m_spsFlags1.m_fields.m_spsTemporalMvpEnabledFlag         = 1;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsSbtmvpEnabledFlag              = picParam->sps_flags.bits.sps_sbtmvp_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsAmvrEnabledFlag                = picParam->sps_flags.bits.sps_amvr_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsBdofEnabledFlag                = 0;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsBdofControlPresentInPhFlag     = 0;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsSmvdEnabledFlag                = picParam->sps_flags.bits.sps_smvd_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsDmvrEnabledFlag                = 0;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsDmvrControlPresentInPhFlag     = 0;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsMmvdEnabledFlag                = picParam->sps_flags.bits.sps_mmvd_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsMmvdFullpelOnlyEnabledFlag     = 0;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsSbtEnabledFlag                 = picParam->sps_flags.bits.sps_sbt_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsAffineEnabledFlag              = picParam->sps_flags.bits.sps_affine_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_sps6paramAffineEnabledFlag        = picParam->sps_flags.bits.sps_6param_affine_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsAffineAmvrEnabledFlag          = picParam->sps_flags.bits.sps_affine_amvr_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsAffineProfEnabledFlag          = picParam->sps_flags.bits.sps_affine_prof_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsProfControlPresentInPhFlag     = 0;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsBcwEnabledFlag                 = picParam->sps_flags.bits.sps_bcw_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsCiipEnabledFlag                = picParam->sps_flags.bits.sps_ciip_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsGpmEnabledFlag                 = picParam->sps_flags.bits.sps_gpm_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsIspEnabledFlag                 = picParam->sps_flags.bits.sps_isp_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsMrlEnabledFlag                 = picParam->sps_flags.bits.sps_mrl_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsMipEnabledFlag                 = picParam->sps_flags.bits.sps_mip_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsCclmEnabledFlag                = picParam->sps_flags.bits.sps_cclm_enabled_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsChromaHorizontalCollocatedFlag = picParam->sps_flags.bits.sps_chroma_horizontal_collocated_flag;
    pVvcPicParams->m_spsFlags1.m_fields.m_spsChromaVerticalCollocatedFlag   = picParam->sps_flags.bits.sps_chroma_vertical_collocated_flag;

    // parse spsFlagss2
    pVvcPicParams->m_spsFlags2.m_fields.m_spsPaletteEnabledFlag                                 = picParam->sps_flags.bits.sps_palette_enabled_flag;
    pVvcPicParams->m_spsFlags2.m_fields.m_spsActEnabledFlag                                     = picParam->sps_flags.bits.sps_act_enabled_flag;
    pVvcPicParams->m_spsFlags2.m_fields.m_spsIbcEnabledFlag                                     = picParam->sps_flags.bits.sps_ibc_enabled_flag;
    pVvcPicParams->m_spsFlags2.m_fields.m_spsLadfEnabledFlag                                    = picParam->sps_flags.bits.sps_ladf_enabled_flag;
    pVvcPicParams->m_spsFlags2.m_fields.m_spsExplicitScalingListEnabledFlag                     = picParam->sps_flags.bits.sps_explicit_scaling_list_enabled_flag;
    pVvcPicParams->m_spsFlags2.m_fields.m_spsScalingMatrixForLfnstDisabledFlag                  = picParam->sps_flags.bits.sps_scaling_matrix_for_lfnst_disabled_flag;
    pVvcPicParams->m_spsFlags2.m_fields.m_spsScalingMatrixForAlternativeColourSpaceDisabledFlag = picParam->sps_flags.bits.sps_scaling_matrix_for_alternative_colour_space_disabled_flag;
    pVvcPicParams->m_spsFlags2.m_fields.m_spsScalingMatrixDesignatedColourSpaceFlag             = picParam->sps_flags.bits.sps_scaling_matrix_designated_colour_space_flag;
    pVvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesEnabledFlag                       = picParam->sps_flags.bits.sps_virtual_boundaries_enabled_flag;
    pVvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesPresentFlag                       = picParam->sps_flags.bits.sps_virtual_boundaries_present_flag;

    // picure-level params
    pVvcPicParams->m_numVerVirtualBoundaries = picParam->NumVerVirtualBoundaries;
    pVvcPicParams->m_numHorVirtualBoundaries = picParam->NumHorVirtualBoundaries;

    MOS_SecureMemcpy(pVvcPicParams->m_virtualBoundaryPosX, 3 * sizeof(uint16_t), picParam->VirtualBoundaryPosX, 3 * sizeof(uint16_t));
    MOS_SecureMemcpy(pVvcPicParams->m_virtualBoundaryPosY, 3 * sizeof(uint16_t), picParam->VirtualBoundaryPosY, 3 * sizeof(uint16_t));

    pVvcPicParams->m_ppsScalingWinLeftOffset          = picParam->pps_scaling_win_left_offset;
    pVvcPicParams->m_ppsScalingWinRightOffset         = picParam->pps_scaling_win_right_offset;
    pVvcPicParams->m_ppsScalingWinTopOffset           = picParam->pps_scaling_win_top_offset;
    pVvcPicParams->m_ppsScalingWinBottomOffset        = picParam->pps_scaling_win_bottom_offset;
    pVvcPicParams->m_ppsNumExpTileColumnsMinus1       = picParam->pps_num_exp_tile_columns_minus1;
    pVvcPicParams->m_ppsNumExpTileRowsMinus1          = picParam->pps_num_exp_tile_rows_minus1;
    pVvcPicParams->m_ppsNumSlicesInPicMinus1          = picParam->pps_num_slices_in_pic_minus1;
    pVvcPicParams->m_ppsPicWidthMinusWraparoundOffset = picParam->pps_pic_width_minus_wraparound_offset;
    pVvcPicParams->m_ppsCbQpOffset                    = picParam->pps_cb_qp_offset;
    pVvcPicParams->m_ppsCrQpOffset                    = picParam->pps_cr_qp_offset;
    pVvcPicParams->m_ppsJointCbcrQpOffsetValue        = picParam->pps_joint_cbcr_qp_offset_value;
    pVvcPicParams->m_ppsChromaQpOffsetListLenMinus1   = picParam->pps_chroma_qp_offset_list_len_minus1;

    MOS_SecureMemcpy(pVvcPicParams->m_ppsCbQpOffsetList, 6 * sizeof(int8_t), picParam->pps_cb_qp_offset_list, 6 * sizeof(int8_t));
    MOS_SecureMemcpy(pVvcPicParams->m_ppsCrQpOffsetList, 6 * sizeof(int8_t), picParam->pps_cr_qp_offset_list, 6 * sizeof(int8_t));
    MOS_SecureMemcpy(pVvcPicParams->m_ppsJointCbcrQpOffsetList, 6 * sizeof(int8_t), picParam->pps_joint_cbcr_qp_offset_list, 6 * sizeof(int8_t));

    // this def is not aligned btw VAAPI & HAL, need to parse each to get correct flag
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsLoopFilterAcrossTilesEnabledFlag    = picParam->pps_flags.bits.pps_loop_filter_across_tiles_enabled_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag                       = picParam->pps_flags.bits.pps_rect_slice_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag            = picParam->pps_flags.bits.pps_single_slice_per_subpic_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsLoopFilterAcrossSlicesEnabledFlag   = picParam->pps_flags.bits.pps_loop_filter_across_slices_enabled_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsWeightedPredFlag                    = picParam->pps_flags.bits.pps_weighted_pred_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsWeightedBipredFlag                  = picParam->pps_flags.bits.pps_weighted_bipred_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsRefWraparoundEnabledFlag            = picParam->pps_flags.bits.pps_ref_wraparound_enabled_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsCuQpDeltaEnabledFlag                = picParam->pps_flags.bits.pps_cu_qp_delta_enabled_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag       = 0;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsSliceChromaQpOffsetsPresentFlag     = 0;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsCuChromaQpOffsetListEnabledFlag     = picParam->pps_flags.bits.pps_cu_chroma_qp_offset_list_enabled_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterOverrideEnabledFlag = picParam->pps_flags.bits.pps_deblocking_filter_override_enabled_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterDisabledFlag        = picParam->pps_flags.bits.pps_deblocking_filter_disabled_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsDbfInfoInPhFlag                     = picParam->pps_flags.bits.pps_dbf_info_in_ph_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsRplInfoInPhFlag                     = 0;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsSaoInfoInPhFlag                     = picParam->pps_flags.bits.pps_sao_info_in_ph_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag                     = picParam->pps_flags.bits.pps_alf_info_in_ph_flag;
    pVvcPicParams->m_ppsFlags.m_fields.m_ppsWpInfoInPhFlag                      = 0;

    pVvcPicParams->m_phLmcsApsId                                = picParam->ph_lmcs_aps_id;
    pVvcPicParams->m_phScalingListApsId                         = picParam->ph_scaling_list_aps_id;
    pVvcPicParams->m_phLog2DiffMinQtMinCbIntraSliceLuma         = picParam->ph_log2_diff_min_qt_min_cb_intra_slice_luma;
    pVvcPicParams->m_phMaxMtt_hierarchyDepthIntraSliceLuma      = picParam->ph_max_mtt_hierarchy_depth_intra_slice_luma;
    pVvcPicParams->m_phLog2DiffMaxBtMinQtIntraSliceLuma         = picParam->ph_log2_diff_max_bt_min_qt_intra_slice_luma;
    pVvcPicParams->m_phLog2DiffMax_ttMinQtIntraSliceLuma        = picParam->ph_log2_diff_max_tt_min_qt_intra_slice_luma;
    pVvcPicParams->m_phLog2DiffMinQtMinCbIntraSliceChroma       = picParam->ph_log2_diff_min_qt_min_cb_intra_slice_chroma;
    pVvcPicParams->m_phMaxMtt_hierarchyDepthIntraSliceChroma    = picParam->ph_max_mtt_hierarchy_depth_intra_slice_chroma;
    pVvcPicParams->m_phLog2DiffMaxBtMinQtIntraSliceChroma       = picParam->ph_log2_diff_max_bt_min_qt_intra_slice_chroma;
    pVvcPicParams->m_phLog2DiffMax_ttMinQtIntraSliceChroma      = picParam->ph_log2_diff_max_tt_min_qt_intra_slice_chroma;
    pVvcPicParams->m_phCuQpDeltaSubdivIntraSlice                = picParam->ph_cu_qp_delta_subdiv_intra_slice;
    pVvcPicParams->m_phCuChromaQpOffsetSubdivIntraSlice         = picParam->ph_cu_chroma_qp_offset_subdiv_intra_slice;
    pVvcPicParams->m_phLog2DiffMinQtMinCbInterSlice             = picParam->ph_log2_diff_min_qt_min_cb_inter_slice;
    pVvcPicParams->m_phMaxMtt_hierarchyDepthInterSlice          = picParam->ph_max_mtt_hierarchy_depth_inter_slice;
    pVvcPicParams->m_phLog2DiffMaxBtMinQtInterSlice             = picParam->ph_log2_diff_max_bt_min_qt_inter_slice;
    pVvcPicParams->m_phLog2DiffMax_ttMinQtInterSlice            = picParam->ph_log2_diff_max_tt_min_qt_inter_slice;
    pVvcPicParams->m_phCuQpDeltaSubdivInterSlice                = picParam->ph_cu_qp_delta_subdiv_inter_slice;
    pVvcPicParams->m_phCuChromaQpOffsetSubdivInterSlice         = picParam->ph_cu_chroma_qp_offset_subdiv_inter_slice;
    pVvcPicParams->m_phLumaBetaOffsetDiv2                       = 0;
    pVvcPicParams->m_phLumaTcOffsetDiv2                         = 0;
    pVvcPicParams->m_phCbBetaOffsetDiv2                         = 0;
    pVvcPicParams->m_phCbTcOffsetDiv2                           = 0;
    pVvcPicParams->m_phCrBetaOffsetDiv2                         = 0;
    pVvcPicParams->m_phCrTcOffsetDiv2                           = 0;

    // this def is not aligned btw VAAPI & HAL, need to parse each to get correct flag
    pVvcPicParams->m_phFlags.m_fields.m_phNonRefPicFlag                  = picParam->ph_flags.bits.ph_non_ref_pic_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phAlfEnabledFlag                 = picParam->ph_flags.bits.ph_alf_enabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phAlfCbEnabledFlag               = picParam->ph_flags.bits.ph_alf_cb_enabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phAlfCrEnabledFlag               = picParam->ph_flags.bits.ph_alf_cr_enabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phAlfCcCbEnabledFlag             = picParam->ph_flags.bits.ph_alf_cc_cb_enabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phAlfCcCrEnabledFlag             = picParam->ph_flags.bits.ph_alf_cc_cr_enabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phLmcsEnabledFlag                = picParam->ph_flags.bits.ph_lmcs_enabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phChromaResidualScaleFlag        = picParam->ph_flags.bits.ph_chroma_residual_scale_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phExplicitScalingListEnabledFlag = picParam->ph_flags.bits.ph_explicit_scaling_list_enabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phVirtualBoundariesPresentFlag   = picParam->ph_flags.bits.ph_virtual_boundaries_present_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phTemporalMvpEnabledFlag         = picParam->ph_flags.bits.ph_temporal_mvp_enabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_numRefEntries0RplIdx0LargerThan0 = 0;
    pVvcPicParams->m_phFlags.m_fields.m_numRefEntries1RplIdx1LargerThan0 = 0;
    pVvcPicParams->m_phFlags.m_fields.m_phCollocatedFromL0Flag           = 0;
    pVvcPicParams->m_phFlags.m_fields.m_phMmvdFullpelOnlyFlag            = picParam->ph_flags.bits.ph_mmvd_fullpel_only_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phMvdL1ZeroFlag                  = picParam->ph_flags.bits.ph_mvd_l1_zero_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phBdofDisabledFlag               = picParam->ph_flags.bits.ph_bdof_disabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phDmvrDisabledFlag               = picParam->ph_flags.bits.ph_dmvr_disabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phProfDisabledFlag               = picParam->ph_flags.bits.ph_prof_disabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phJointCbcrSignFlag              = picParam->ph_flags.bits.ph_joint_cbcr_sign_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phSaoLumaEnabledFlag             = picParam->ph_flags.bits.ph_sao_luma_enabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phSaoChromaEnabledFlag           = picParam->ph_flags.bits.ph_sao_chroma_enabled_flag;
    pVvcPicParams->m_phFlags.m_fields.m_phDeblockingFilterDisabledFlag   = picParam->ph_flags.bits.ph_deblocking_filter_disabled_flag;
    pVvcPicParams->m_picMiscFlags.m_fields.m_intraPicFlag                = picParam->PicMiscFlags.fields.IntraPicFlag;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVvc::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    VAStatus va = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);

    DDI_CODEC_FUNC_ENTER;

    void *data = nullptr;
    for (int i = 0; i < numBuffers; i++)
    {
        if (!buffers || (buffers[i] == VA_INVALID_ID))
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        DDI_MEDIA_BUFFER *buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, buffers[i]);
        if (nullptr == buf)
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        uint32_t dataSize = buf->iSize;
        MediaLibvaInterfaceNext::MapBuffer(ctx, buffers[i], &data);

        if (data == nullptr)
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        switch ((int32_t)buf->uiType)
        {
        case VASliceDataBufferType:
        {
            // decode bitsream buffer
            int32_t index = GetBitstreamBufIndexFromBuffer(&m_decodeCtx->BufMgr, buf);
            if (index == DDI_CODEC_INVALID_BUFFER_INDEX)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }
            MediaLibvaCommonNext::MediaBufferToMosResource(m_decodeCtx->BufMgr.pBitStreamBuffObject[index], &m_decodeCtx->BufMgr.resBitstreamBuffer);
            m_decodeCtx->DecodeParams.m_dataSize += dataSize;
            break;
        }
        case VASliceParameterBufferType:
        {
            // VVC slice control data
            if (buf->uiNumElements == 0)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }
            uint32_t numSlices = buf->uiNumElements;
            VASliceParameterBufferVVC *slcInfoVVC = (VASliceParameterBufferVVC *)data;
            DDI_CODEC_CHK_RET(ParseSliceParams(mediaCtx, slcInfoVVC, numSlices), "ParseSliceParams failed!");
            m_decodeCtx->DecodeParams.m_numSlices += numSlices;
            m_groupIndex++;
            break;
        }
        case VAPictureParameterBufferType:
        {
            // VVC pic param
            VAPictureParameterBufferVVC *picParam = (VAPictureParameterBufferVVC *)data;
            DDI_CODEC_CHK_RET(ParsePicParams(mediaCtx, picParam), "ParsePicParams failed!");
            subpic_buffer_nums = 0;
            alf_buffer_nums = 0;
            lmcs_buffer_nums = 0;
            scaling_list_buffer_nums = 0;
            tile_buffer_nums = 0;
            slice_struct_nums = 0;
            break;
        }
        case VAIQMatrixBufferType:
        {
            // VVC scaling list
            CodecVvcPicParams *pVvcPicParams              = static_cast<CodecVvcPicParams*>(m_decodeCtx->DecodeParams.m_picParams);
            pVvcPicParams->m_numScalingMatrixBuffers      = buf->uiNumElements;
            if (vvcMaxScalingMatrixNum < (pVvcPicParams->m_numScalingMatrixBuffers + scaling_list_buffer_nums))
            {
                DDI_CODEC_ASSERTMESSAGE("numScalingListBuffers = %d exceeds max size = %d", pVvcPicParams->m_numScalingMatrixBuffers + scaling_list_buffer_nums, vvcMaxScalingMatrixNum);
                return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
            }
            m_decodeCtx->DecodeParams.m_iqMatrixSize   = dataSize + scaling_list_buffer_nums * sizeof(CodecVvcQmData);
            CodecVvcQmData *pVvcScalingData           = (CodecVvcQmData*)(m_decodeCtx->DecodeParams.m_iqMatrixBuffer);
            pVvcScalingData += scaling_list_buffer_nums;
            scaling_list_buffer_nums += buf->uiNumElements;
            MOS_SecureMemcpy(pVvcScalingData,
                buf->uiNumElements * sizeof(CodecVvcQmData),
                data, dataSize);
            break;
        }
        case VAAlfBufferType:
        {
            // VVC ALF array
            CodecVvcPicParams *pVvcPicParams               = static_cast<CodecVvcPicParams*>(m_decodeCtx->DecodeParams.m_picParams);
            pVvcPicParams->m_numAlfBuffers                 = buf->uiNumElements;
            m_decodeCtx->DecodeParams.m_deblockDataSize    = dataSize + alf_buffer_nums * sizeof(CodecVvcAlfData);
            VAAlfDataVVC *alfDatas                     = static_cast<VAAlfDataVVC*>(data);
            DDI_CODEC_CHK_RET(ParseAlfDatas(m_decodeCtx, alfDatas, pVvcPicParams->m_numAlfBuffers, alf_buffer_nums), "ParseAlfDatas failed!");
            alf_buffer_nums                                += buf->uiNumElements;
            break;
        }
        case VALmcsBufferType:
        {
            // VVC LMCS array
            CodecVvcPicParams *pVvcPicParams                = static_cast<CodecVvcPicParams*>(m_decodeCtx->DecodeParams.m_picParams);
            pVvcPicParams->m_numLmcsBuffers                 = buf->uiNumElements;
            VALmcsDataVVC *lmcsDatas                        = static_cast<VALmcsDataVVC*>(data);
            DDI_CHK_RET(ParseLmcsDatas(m_decodeCtx, lmcsDatas, pVvcPicParams->m_numLmcsBuffers, lmcs_buffer_nums), "ParseLmcsDatas failed!");
            m_decodeCtx->DecodeParams.m_numMacroblocks      = buf->uiNumElements + lmcs_buffer_nums;
            lmcs_buffer_nums += buf->uiNumElements;
            break;
        }
        case VASubPicBufferType:
        {
            // VVC SubPic buffer
            VASubPicVVC *subPicParam = static_cast<VASubPicVVC *>(data);
            DDI_CODEC_CHK_RET(ParseSubPicParams(mediaCtx, subPicParam, buf->uiNumElements, subpic_buffer_nums), "ParseSubPicParams failed!");
            subpic_buffer_nums += buf->uiNumElements;
            break;
        }
        case VATileBufferType:
        {
            // VVC Tile buffer
            uint16_t *tileParam = static_cast<uint16_t *>(data);
            DDI_CODEC_CHK_RET(ParseTileParams(mediaCtx, tileParam, buf->uiNumElements, tile_buffer_nums), "ParseTileParams failed!");
            tile_buffer_nums += buf->uiNumElements;
            break;
        }
        case VASliceStructBufferType:
        {
            // VVC SliceStruct buffer
            VASliceStructVVC *sliceStructParam = static_cast<VASliceStructVVC *>(data);
            CodecVvcPicParams *pVvcPicParams = static_cast<CodecVvcPicParams*>(m_decodeCtx->DecodeParams.m_picParams);
            if ((sliceStructParam == nullptr) || (pVvcPicParams == nullptr))
            {
                DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing VVC SliceStruct parameter\n");
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }

            // assume PicParam is always parsed before this buffer
            if (!pVvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
            {
                DDI_CODEC_NORMALMESSAGE("No Slice Struct buffer indicated by Pic Params buffer, just ignore the Slice Struct Buffer.");
                return VA_STATUS_SUCCESS;
            }
            DDI_CODEC_CHK_RET(ParseSliceStructParams(mediaCtx, sliceStructParam, buf->uiNumElements, slice_struct_nums), "ParseSliceStructParams failed!");
            slice_struct_nums += buf->uiNumElements;
            pVvcPicParams->m_numSliceStructsMinus1 = slice_struct_nums - 1;
            break;
        }
        default:
            va = m_decodeCtx->pCpDdiInterfaceNext->RenderCencPicture(ctx, context, buf, data);
            break;
        }
        MediaLibvaInterfaceNext::UnmapBuffer(ctx, buffers[i]);
    }

    CodecVvcPicParams *pVvcPicParams = static_cast<CodecVvcPicParams*>(m_decodeCtx->DecodeParams.m_picParams);
    // for slice struct buffers check
    if(vvcMaxSliceNum < slice_struct_nums)
    {
        DDI_CODEC_ASSERTMESSAGE("numSliceStructs = %d exceeds max size = %d", slice_struct_nums, vvcMaxSliceNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    // for subpic buffers check
    if(vvcMaxSliceNum < subpic_buffer_nums)
    {
        DDI_CODEC_ASSERTMESSAGE("numSubPics = %d exceeds max size = %d", subpic_buffer_nums, vvcMaxSliceNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    if (pVvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && pVvcPicParams->m_spsNumSubpicsMinus1 > 0)
    {
        if (subpic_buffer_nums && ((pVvcPicParams->m_spsNumSubpicsMinus1 + 1) != subpic_buffer_nums))
        {
            DDI_CODEC_ASSERTMESSAGE("SubPic number inconsistent between Pic Params buffer and SubPic buffer.");
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }
    }

    // for tile buffers check
    if(vvcMaxSliceNum < tile_buffer_nums)
    {
        DDI_CODEC_ASSERTMESSAGE("numTiles = %d exceeds max size = %d", tile_buffer_nums, vvcMaxSliceNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    if (tile_buffer_nums && (pVvcPicParams->m_ppsNumExpTileColumnsMinus1 +
        pVvcPicParams->m_ppsNumExpTileRowsMinus1 + 2 != tile_buffer_nums))
    {
        DDI_CODEC_ASSERTMESSAGE("Tile Params number inconsistent between Pic Params buffer and Tile params buffer.");
        return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    DDI_FUNCTION_EXIT(va);
    return va;
}

VAStatus DdiDecodeVvc::ParseAlfDatas(
    DDI_DECODE_CONTEXT   *decodeCtx,
    VAAlfDataVVC         *alfDatas,     
    uint32_t             numAlfDatas,
    uint32_t             numAlfBuffers)
{
    DDI_CHK_NULL(decodeCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (vvcMaxAlfNum < (numAlfDatas + numAlfBuffers))
    {
        DDI_CODEC_ASSERTMESSAGE("numAlfBuffers = %d exceeds max size = %d", numAlfDatas + numAlfBuffers, vvcMaxAlfNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    CodecVvcAlfData *pVvcAlfData           = (CodecVvcAlfData*)(decodeCtx->DecodeParams.m_deblockData);
    pVvcAlfData += numAlfBuffers;
    if ((alfDatas == nullptr) || (pVvcAlfData == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing VVC ALF Datas\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    for (uint32_t iDataCount = 0; iDataCount < numAlfDatas; iDataCount++)
    {
        pVvcAlfData->m_apsAdaptationParameterSetId                   = alfDatas->aps_adaptation_parameter_set_id;
        pVvcAlfData->m_alfLumaNumFiltersSignalledMinus1              = alfDatas->alf_luma_num_filters_signalled_minus1;
        pVvcAlfData->m_alfChromaNumAltFiltersMinus1                  = alfDatas->alf_chroma_num_alt_filters_minus1;
        pVvcAlfData->m_alfCcCbFiltersSignalledMinus1                 = alfDatas->alf_cc_cb_filters_signalled_minus1;
        pVvcAlfData->m_alfCcCrFiltersSignalledMinus1                 = alfDatas->alf_cc_cr_filters_signalled_minus1;
        pVvcAlfData->m_alfFlags.m_fields.m_alfLumaFilterSignalFlag   = alfDatas->alf_flags.bits.alf_luma_filter_signal_flag;
        pVvcAlfData->m_alfFlags.m_fields.m_alfChromaFilterSignalFlag = alfDatas->alf_flags.bits.alf_chroma_filter_signal_flag;
        pVvcAlfData->m_alfFlags.m_fields.m_alfCcCbFilterSignalFlag   = alfDatas->alf_flags.bits.alf_cc_cb_filter_signal_flag;
        pVvcAlfData->m_alfFlags.m_fields.m_alfCcCrFilterSignalFlag   = alfDatas->alf_flags.bits.alf_cc_cr_filter_signal_flag;
        pVvcAlfData->m_alfFlags.m_fields.m_alfLumaClipFlag           = alfDatas->alf_flags.bits.alf_luma_clip_flag;
        pVvcAlfData->m_alfFlags.m_fields.m_alfChromaClipFlag         = alfDatas->alf_flags.bits.alf_chroma_clip_flag;
        pVvcAlfData->m_alfFlags.m_fields.m_reservedBits              = alfDatas->alf_flags.bits.reserved;
        for (int i = 0; i < 25; i++)
        {
            for (int j = 0; j < 12; j++)
            {
                pVvcAlfData->m_alfCoeffL[i][j]      = alfDatas->filtCoeff[i][j];
                pVvcAlfData->m_alfLumaClipIdx[i][j] = alfDatas->alf_luma_clip_idx[i][j];
            }
        }
        for (int i = 0; i < 8; i++)
        {
            for (int j = 0; j < 6; j++)
            {
                pVvcAlfData->m_alfCoeffC[i][j]        = alfDatas->AlfCoeffC[i][j];
                pVvcAlfData->m_alfChromaClipIdx[i][j] = alfDatas->alf_chroma_clip_idx[i][j];
            }
        }
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 7; j++)
            {
                pVvcAlfData->m_ccAlfApsCoeffCb[i][j] = alfDatas->CcAlfApsCoeffCb[i][j];
                pVvcAlfData->m_ccAlfApsCoeffCr[i][j] = alfDatas->CcAlfApsCoeffCr[i][j];
            }
        }
        MOS_SecureMemcpy(pVvcAlfData->m_alfLumaCoeffDeltaIdx, 25, alfDatas->alf_luma_coeff_delta_idx, 25);
        MOS_SecureMemcpy(pVvcAlfData->m_reserved32b, VA_PADDING_MEDIUM, alfDatas->va_reserved, VA_PADDING_MEDIUM);
        alfDatas++;
        pVvcAlfData++;
    }
    return VA_STATUS_SUCCESS; 
}

VAStatus DdiDecodeVvc::ParseLmcsDatas(
    DDI_DECODE_CONTEXT   *decodeCtx,
    VALmcsDataVVC        *LmcsDatas,     
    uint32_t             numLmcsDatas,
    uint32_t            numLMCSBuffers)
{
   DDI_CHK_NULL(decodeCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (vvcMaxLmcsNum < (numLmcsDatas + numLMCSBuffers))
    {
        DDI_CODEC_ASSERTMESSAGE("numLMCSBuffers = %d exceeds max size = %d", numLmcsDatas + numLMCSBuffers, vvcMaxLmcsNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
   CodecVvcLmcsData* pVvcLmcsData = (CodecVvcLmcsData*)(decodeCtx->DecodeParams.m_macroblockParams);
   pVvcLmcsData += numLMCSBuffers;
    if ((LmcsDatas == nullptr) || (pVvcLmcsData == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing LMCS Datas\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    for (uint32_t iDataCount = 0; iDataCount < numLmcsDatas; iDataCount++)
    {
        pVvcLmcsData->m_apsAdaptationParameterSetId = LmcsDatas->aps_adaptation_parameter_set_id;
        pVvcLmcsData->m_lmcsMinBinIdx               = LmcsDatas->lmcs_min_bin_idx;
        pVvcLmcsData->m_lmcsDeltaMaxBinIdx          = LmcsDatas->lmcs_delta_max_bin_idx;
        pVvcLmcsData->m_lmcsDeltaCrs                = LmcsDatas->lmcsDeltaCrs;
        MOS_SecureMemcpy(pVvcLmcsData->m_lmcsDeltaCW, 16 * sizeof(uint16_t), LmcsDatas->lmcsDeltaCW, 16 * sizeof(uint16_t));
        LmcsDatas++;
        pVvcLmcsData++;
    }
    return VA_STATUS_SUCCESS;  
}

VAStatus DdiDecodeVvc::ParseWeightedPredInfo(
    CodecVvcSliceParams*      sliceParams,
    VAWeightedPredInfo*       wpInfoParams)
{
    if ((sliceParams == nullptr) || (wpInfoParams == nullptr))
    {
        DDI_ASSERTMESSAGE("Invalid Parameter for Parsing Weighted Info Params\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    sliceParams->m_wpInfo.m_lumaLog2WeightDenom        = wpInfoParams->luma_log2_weight_denom;
    sliceParams->m_wpInfo.m_deltaChromaLog2WeightDenom = wpInfoParams->delta_chroma_log2_weight_denom;
    sliceParams->m_wpInfo.m_numL0Weights               = wpInfoParams->num_l0_weights;
    sliceParams->m_wpInfo.m_numL1Weights               = wpInfoParams->num_l1_weights;
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_lumaWeightL0Flag, 15 * sizeof(uint8_t), wpInfoParams->luma_weight_l0_flag, 15 * sizeof(uint8_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_chromaWeightL0Flag, 15 * sizeof(uint8_t), wpInfoParams->chroma_weight_l0_flag, 15 * sizeof(uint8_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_deltaLumaWeightL0, 15 * sizeof(int8_t), wpInfoParams->delta_luma_weight_l0, 15 * sizeof(int8_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_lumaOffsetL0, 15 * sizeof(int8_t), wpInfoParams->luma_offset_l0, 15 * sizeof(int8_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_deltaChromaWeightL0, 15 * 2 * sizeof(int8_t), wpInfoParams->delta_chroma_weight_l0, 15 * 2 * sizeof(int8_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_deltaChromaOffsetL0, 15 * 2 * sizeof(int16_t), wpInfoParams->delta_chroma_offset_l0, 15 * 2 * sizeof(int16_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_lumaWeightL1Flag, 15 * sizeof(uint8_t), wpInfoParams->luma_weight_l1_flag, 15 * sizeof(uint8_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_chromaWeightL1Flag, 15 * sizeof(uint8_t), wpInfoParams->chroma_weight_l1_flag, 15 * sizeof(uint8_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_deltaLumaWeightL1, 15 * sizeof(int8_t), wpInfoParams->delta_luma_weight_l1, 15 * sizeof(int8_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_lumaOffsetL1, 15 * sizeof(int8_t), wpInfoParams->luma_offset_l1, 15 * sizeof(int8_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_deltaChromaWeightL1, 15 * 2 * sizeof(int8_t), wpInfoParams->delta_chroma_weight_l1, 15 * 2 * sizeof(int8_t));
    MOS_SecureMemcpy(sliceParams->m_wpInfo.m_deltaChromaOffsetL1, 15 * 2 * sizeof(int16_t), wpInfoParams->delta_chroma_offset_l1, 15 * 2 * sizeof(int16_t));
    
    return VA_STATUS_SUCCESS;      
}

VAStatus DdiDecodeVvc::InitResourceBuffer()
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_BUFFER_PARAM_VVC *Codec_Param_VVC = nullptr;
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_decodeCtx->BufMgr);
    bufMgr->pSliceData = nullptr;
    bufMgr->ui64BitstreamOrder = 0;
    bufMgr->dwMaxBsSize = m_width * m_height * 3 / 2; // need consider 2byte case
    // minimal 10k bytes for some special case. Will refractor this later
    if (bufMgr->dwMaxBsSize < DDI_CODEC_MIN_VALUE_OF_MAX_BS_SIZE)
    {
        bufMgr->dwMaxBsSize = DDI_CODEC_MIN_VALUE_OF_MAX_BS_SIZE;
    }

    // init decode bitstream buffer object
    for (uint32_t i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        bufMgr->pBitStreamBuffObject[i] = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
        if (bufMgr->pBitStreamBuffObject[i] == nullptr)
        {
            FreeResourceBuffer();
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        bufMgr->pBitStreamBuffObject[i]->iSize = bufMgr->dwMaxBsSize;
        bufMgr->pBitStreamBuffObject[i]->uiType = VASliceDataBufferType;
        bufMgr->pBitStreamBuffObject[i]->format = Media_Format_Buffer;
        bufMgr->pBitStreamBuffObject[i]->uiOffset = 0;
        bufMgr->pBitStreamBuffObject[i]->bo = nullptr;
        bufMgr->pBitStreamBase[i] = nullptr;
    }

    bufMgr->m_maxNumSliceData = vvcMaxSliceNum;
    bufMgr->pSliceData = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)MOS_AllocAndZeroMemory(sizeof(bufMgr->pSliceData[0]) * bufMgr->m_maxNumSliceData);

    if (bufMgr->pSliceData == nullptr)
    {
        FreeResourceBuffer();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    bufMgr->dwNumSliceData = 0;
    bufMgr->dwNumSliceControl = 0;
    bufMgr->pCodecParamReserved = (DDI_CODEC_BUFFER_PARAM_VVC *)MOS_AllocAndZeroMemory(sizeof(DDI_CODEC_BUFFER_PARAM_VVC));

    if (bufMgr->pCodecParamReserved == nullptr)
    {
        FreeResourceBuffer();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    
    bufMgr->pCodecSlcParamReserved = (VASliceParameterBufferVVC *)MOS_AllocAndZeroMemory(sizeof(VASliceParameterBufferVVC) * vvcMaxSliceNum);
    Codec_Param_VVC = (DDI_CODEC_BUFFER_PARAM_VVC *)bufMgr->pCodecParamReserved;
    Codec_Param_VVC->pVASliceParameterBufferVVC = (VASliceParameterBufferVVC *)bufMgr->pCodecSlcParamReserved;

    if (bufMgr->pCodecSlcParamReserved == nullptr)
    {
        FreeResourceBuffer();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    return VA_STATUS_SUCCESS;
}

void DdiDecodeVvc::FreeResourceBuffer()
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_decodeCtx->BufMgr);

    // free decode bitstream buffer
    for (uint32_t i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        if (bufMgr->pBitStreamBase[i])
        {
            MediaLibvaUtilNext::UnlockBuffer(bufMgr->pBitStreamBuffObject[i]);
            bufMgr->pBitStreamBase[i] = nullptr;
        }
        if (bufMgr->pBitStreamBuffObject[i])
        {
            MediaLibvaUtilNext::FreeBuffer(bufMgr->pBitStreamBuffObject[i]);
            MOS_FreeMemory(bufMgr->pBitStreamBuffObject[i]);
            bufMgr->pBitStreamBuffObject[i] = nullptr;
        }
    }

    if (bufMgr->pCodecParamReserved)
    {
        DDI_CODEC_BUFFER_PARAM_VVC* Codec_Param_VVC =
            static_cast<DDI_CODEC_BUFFER_PARAM_VVC *>(bufMgr->pCodecParamReserved);
         if (Codec_Param_VVC->pVASliceParameterBufferVVC)
         {
            MOS_FreeMemory(Codec_Param_VVC->pVASliceParameterBufferVVC);
            Codec_Param_VVC->pVASliceParameterBufferVVC = nullptr;
            bufMgr->pCodecSlcParamReserved = nullptr;
         }
         MOS_FreeMemory(bufMgr->pCodecParamReserved);
         bufMgr->pCodecParamReserved = nullptr;
    }

    // free decode bitstream buffer object
    MOS_FreeMemory(bufMgr->pSliceData);
    bufMgr->pSliceData = nullptr;

    return;
}

uint8_t* DdiDecodeVvc::GetPicParamBuf(
    DDI_CODEC_COM_BUFFER_MGR *bufMgr)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_BUFFER_PARAM_VVC* Codec_Param_VVC = static_cast<DDI_CODEC_BUFFER_PARAM_VVC *>(bufMgr->pCodecParamReserved);
    return (uint8_t*)(&(Codec_Param_VVC->PicParamVVC));
}

VAStatus DdiDecodeVvc::CreateBuffer(
    VABufferType            type,
    uint32_t                size,
    uint32_t                numElements,
    void                    *data,
    VABufferID              *bufId)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_MEDIA_BUFFER                *buf;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT   bufferHeapElement;
    MOS_STATUS                       status = MOS_STATUS_SUCCESS;
    VAStatus                         va = VA_STATUS_SUCCESS;

    buf = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    if (buf == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    buf->iSize         = size * numElements;
    buf->uiNumElements = numElements;
    buf->uiType        = type;
    buf->format        = Media_Format_Buffer;
    buf->uiOffset      = 0;
    buf->bCFlushReq    = false;
    buf->pMediaCtx     = m_decodeCtx->pMediaCtx;

    switch ((int32_t)type)
    {
        case VASliceDataBufferType:
        case VAProtectedSliceDataBufferType:
            va = AllocBsBuffer(&(m_decodeCtx->BufMgr), buf);
            if(va != VA_STATUS_SUCCESS)
            {
                if(buf)
                {
                    MOS_FreeMemory(buf->pData);
                    MOS_FreeMemory(buf);
                }
                return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
            }
            break;
        case VASliceParameterBufferType:
            va = AllocSliceControlBuffer(buf);
            if(va != VA_STATUS_SUCCESS)
            {
                if(buf)
                {
                    MOS_FreeMemory(buf->pData);
                    MOS_FreeMemory(buf);
                }
                return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
            }
            buf->format     = Media_Format_CPU;
            break;
        case VAPictureParameterBufferType:
            buf->pData      = GetPicParamBuf(&(m_decodeCtx->BufMgr));
            buf->format     = Media_Format_CPU;
            break;
        case VAIQMatrixBufferType:
        case VAAlfBufferType:
        case VALmcsBufferType:
        case VASubPicBufferType:
        case VATileBufferType:
        case VASliceStructBufferType:
            buf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(size * numElements);
            buf->format     = Media_Format_CPU;
            break;
        default:
            va = m_decodeCtx->pCpDdiInterface->CreateBuffer(type, buf, size, numElements);
            if (va  == VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE)
            {
                DDI_CODEC_ASSERTMESSAGE("DDI:Decode CreateBuffer unsuppoted buffer type.");
                buf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(size * numElements);
                buf->format     = Media_Format_CPU;
                if(buf->pData != nullptr)
                {
                    va = VA_STATUS_SUCCESS;
                }
            }
            break;
    }

    bufferHeapElement  = MediaLibvaUtilNext::AllocPMediaBufferFromHeap(m_decodeCtx->pMediaCtx->pBufferHeap);
    if (nullptr == bufferHeapElement)
    {
        if(buf)
        {
            MOS_FreeMemory(buf->pData);
            MOS_FreeMemory(buf);
        }
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    bufferHeapElement->pBuffer      = buf;
    bufferHeapElement->pCtx         = (void*)m_decodeCtx;
    bufferHeapElement->uiCtxType    = DDI_MEDIA_CONTEXT_TYPE_DECODER;
    *bufId                          = bufferHeapElement->uiVaBufferID;

    m_decodeCtx->pMediaCtx->uiNumBufs++;

    if(data == nullptr)
    {
        return va;
    }

    if(true == buf->bCFlushReq && mos_bo_busy(buf->bo))
    {
        mos_bo_wait_rendering(buf->bo);
    }
    status = MOS_SecureMemcpy((void *)(buf->pData + buf->uiOffset), size * numElements, data, size * numElements);
    DDI_CHK_CONDITION((status != MOS_STATUS_SUCCESS), "DDI:Failed to copy buffer data!", VA_STATUS_ERROR_OPERATION_FAILED);
    return va;
}

VAStatus DdiDecodeVvc::AllocSliceControlBuffer(
    DDI_MEDIA_BUFFER *buf)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_COM_BUFFER_MGR   *bufMgr;
    bufMgr = &(m_decodeCtx->BufMgr);

    DDI_CODEC_BUFFER_PARAM_VVC* Codec_Param_VVC =
        static_cast<DDI_CODEC_BUFFER_PARAM_VVC *>(bufMgr->pCodecParamReserved);
    Codec_Param_VVC->pVASliceParameterBufferVVC =
        static_cast<VASliceParameterBufferVVC *>(bufMgr->pCodecSlcParamReserved);
    if (Codec_Param_VVC->pVASliceParameterBufferVVC == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    
    if (buf->uiNumElements > vvcMaxSliceNum)
    {
        DDI_CODEC_ASSERTMESSAGE("buf->uiNumElements = %d : exceeds vvcMaxSliceNum = %d",
                          buf->uiNumElements, vvcMaxSliceNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    buf->pData = (uint8_t*)Codec_Param_VVC->pVASliceParameterBufferVVC;
    buf->uiOffset = bufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferVVC);

    bufMgr->dwNumSliceControl += buf->uiNumElements;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVvc::CodecHalInit(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *ptr)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(ptr, "nullptr", VA_STATUS_ERROR_INVALID_PARAMETER);
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    CODECHAL_FUNCTION codecFunction = CODECHAL_FUNCTION_DECODE;
    m_decodeCtx->pCpDdiInterfaceNext->SetCpParams(m_ddiDecodeAttr->componentData.data.encryptType, m_codechalSettings);

    CODECHAL_STANDARD_INFO standardInfo;
    memset(&standardInfo, 0, sizeof(standardInfo));

    standardInfo.CodecFunction = codecFunction;
    standardInfo.Mode = (CODECHAL_MODE)m_decodeCtx->wMode;

    m_codechalSettings->codecFunction = codecFunction;
    m_codechalSettings->width = m_width;
    m_codechalSettings->height = m_height;
    m_codechalSettings->intelEntrypointInUse = false;

    // VAProfileVVCMain10 and VAProfileVVCMultilayerMain10 supports both 420 8bit and 420 10bit
    m_codechalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_8_BITS;

    if (static_cast<int>(m_ddiDecodeAttr->profile) == VAProfileVVCMain10 ||
        static_cast<int>(m_ddiDecodeAttr->profile) == VAProfileVVCMultilayerMain10) //todo: opensource issue here
    {
        m_codechalSettings->lumaChromaDepth |= CODECHAL_LUMA_CHROMA_DEPTH_10_BITS;
    }

    m_codechalSettings->shortFormatInUse = m_decodeCtx->bShortFormatInUse;
    m_codechalSettings->mode = CODECHAL_DECODE_MODE_VVCVLD;
    m_codechalSettings->standard = CODECHAL_VVC;
    m_codechalSettings->chromaFormat = HCP_CHROMA_FORMAT_YUV420;

    m_decodeCtx->DecodeParams.m_picParams = MOS_AllocAndZeroMemory(sizeof(CodecVvcPicParams));
    if (m_decodeCtx->DecodeParams.m_picParams == nullptr)
    {
        FreeResource();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    m_decodeCtx->DecodeParams.m_extPicParams = MOS_AllocAndZeroMemory(vvcMaxSubpicNum * sizeof(CodecVvcSubpicParam));
    if (m_decodeCtx->DecodeParams.m_extPicParams == nullptr)
    {
        FreeResource();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    m_decodeCtx->DecodeParams.m_sliceParams = MOS_AllocAndZeroMemory(vvcMaxSliceNum * sizeof(CodecVvcSliceParams));
    if (m_decodeCtx->DecodeParams.m_sliceParams == nullptr)
    {
        FreeResource();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    m_decodeCtx->DecodeParams.m_extSliceParams = MOS_AllocAndZeroMemory(vvcMaxSliceNum * sizeof(CodecVvcSliceStructure));
    if (m_decodeCtx->DecodeParams.m_extSliceParams == nullptr)
    {
        FreeResource();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    m_decodeCtx->DecodeParams.m_tileParams = MOS_AllocAndZeroMemory(vvcMaxTileParamsNum * sizeof(uint16_t));
    if (m_decodeCtx->DecodeParams.m_tileParams == nullptr)
    {
        FreeResource();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    m_decodeCtx->DecodeParams.m_iqMatrixBuffer = MOS_AllocAndZeroMemory(vvcMaxScalingMatrixNum * sizeof(CodecVvcQmData));
    if (m_decodeCtx->DecodeParams.m_iqMatrixBuffer == nullptr)
    {
        FreeResource();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    m_decodeCtx->DecodeParams.m_deblockData = (uint8_t *)MOS_AllocAndZeroMemory(vvcMaxAlfNum * sizeof(CodecVvcAlfData));
    if (m_decodeCtx->DecodeParams.m_deblockData == nullptr)
    {
        FreeResource();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    m_decodeCtx->DecodeParams.m_macroblockParams = MOS_AllocAndZeroMemory(vvcMaxLmcsNum * sizeof(CodecVvcLmcsData));
    if (m_decodeCtx->DecodeParams.m_macroblockParams == nullptr)
    {
        FreeResource();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    vaStatus = CreateCodecHal(mediaCtx,
        ptr,
        &standardInfo);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        FreeResource();
        return vaStatus;
    }

    if (InitResourceBuffer() != VA_STATUS_SUCCESS)
    {
        FreeResource();
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    return vaStatus;
}

void DdiDecodeVvc::FreeResource()
{
    DDI_CODEC_FUNC_ENTER;

    FreeResourceBuffer();

    if (m_decodeCtx->pCodecHal)
    {
        m_decodeCtx->pCodecHal->Destroy();
        MOS_Delete(m_decodeCtx->pCodecHal);
        m_decodeCtx->pCodecHal = nullptr;
    }

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_picParams);
    m_decodeCtx->DecodeParams.m_picParams = nullptr;
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_picParams);
    m_decodeCtx->DecodeParams.m_extPicParams = nullptr;
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_sliceParams);
    m_decodeCtx->DecodeParams.m_sliceParams = nullptr;
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_extSliceParams);
    m_decodeCtx->DecodeParams.m_extSliceParams = nullptr;
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_tileParams);
    m_decodeCtx->DecodeParams.m_tileParams = nullptr;
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_iqMatrixBuffer);
    m_decodeCtx->DecodeParams.m_iqMatrixBuffer = nullptr;
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_deblockData);
    m_decodeCtx->DecodeParams.m_deblockData = nullptr;
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_macroblockParams);
    m_decodeCtx->DecodeParams.m_macroblockParams = nullptr;

    return;
}

MOS_FORMAT DdiDecodeVvc::GetFormat()
{
    DDI_CODEC_FUNC_ENTER;

    MOS_FORMAT format = Format_Invalid;
    CodechalDecodeParams *decodeParams = &m_decodeCtx->DecodeParams;
    DDI_CODEC_CHK_NULL(decodeParams, "nullptr params", Format_Invalid);
    CodecVvcPicParams *picParams = static_cast<CodecVvcPicParams*>(decodeParams->m_picParams);
    DDI_CODEC_CHK_NULL(picParams, "nullptr params", Format_Invalid);

    if (picParams->m_spsBitdepthMinus8 == 0)
    {
        if (picParams->m_spsChromaFormatIdc == 1)  // 4:2:0 8bit surface
        {
            format = Format_NV12;
        }
    }
    else if (picParams->m_spsBitdepthMinus8 == 2)
    {
        if (picParams->m_spsChromaFormatIdc == 1)  // 4:2:0 10bit surface
        {
            format = Format_P010;
        }
    }
    return format;
}

void DdiDecodeVvc::DestroyContext(
    VADriverContextP ctx)
{
    DDI_CODEC_FUNC_ENTER;

    FreeResourceBuffer();

    // release VVC specific buffers that are not covered in base function
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_deblockData);
    m_decodeCtx->DecodeParams.m_deblockData = nullptr;

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_macroblockParams);
    m_decodeCtx->DecodeParams.m_macroblockParams = nullptr;

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_tileParams);
    m_decodeCtx->DecodeParams.m_tileParams = nullptr;

    // explicitly call the base function to do the further clean-up
    DdiDecodeBase::DestroyContext(ctx);

    return;
}

void DdiDecodeVvc::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    DDI_CODEC_FUNC_ENTER;

    // call the function in base class to initialize it.
    DdiDecodeBase::ContextInit(picWidth, picHeight);
    m_decodeCtx->wMode = CODECHAL_DECODE_MODE_VVCVLD;

    return;
}

void DdiDecodeVvc::SetupCodecPicture(
    DDI_MEDIA_CONTEXT             *mediaCtx,
    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl,
    PCODEC_PICTURE                pCodecHalPic,
    VAPictureVVC                  vaPic,
    VvcPicEntryType               bSurfaceType)
{
    DDI_CODEC_FUNC_ENTER;

    if (bSurfaceType != VvcPicEntryRefPicList)
    {
        if (vaPic.picture_id != VA_INVALID_SURFACE)
        {
            if(vaPic.flags & VA_PICTURE_VVC_UNAVAILABLE_REFERENCE)
            {
                pCodecHalPic->FrameIdx = vaPic.picture_id;
            }
            else
            {
                DDI_MEDIA_SURFACE *surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, vaPic.picture_id);
                pCodecHalPic->FrameIdx = GetRenderTargetID(rtTbl, surface);
            }
        }
        else
        {
            pCodecHalPic->FrameIdx = (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX;
        }
    }

    switch (bSurfaceType)
    {
    case VvcPicEntryCurrFrame:
        if (pCodecHalPic->FrameIdx < CODEC_MAX_DPB_NUM_VVC)
        {
            pCodecHalPic->PicFlags = PICTURE_FRAME;
        }
        break;
    case VvcPicEntryRefFrameList:
        if (pCodecHalPic->FrameIdx >= CODEC_MAX_DPB_NUM_VVC ||
            vaPic.flags & VA_PICTURE_VVC_INVALID)
        {
            pCodecHalPic->PicFlags = PICTURE_INVALID;
        }
        else
        {
            // Check unavailable frame flag
            if (vaPic.flags & VA_PICTURE_VVC_UNAVAILABLE_REFERENCE)
            {
                pCodecHalPic->PicFlags = PICTURE_UNAVAILABLE_FRAME;
            }
            else
            {
                pCodecHalPic->PicFlags = PICTURE_FRAME;
            }
        }
        break;
    case VvcPicEntryRefPicList:
        if (pCodecHalPic->FrameIdx >= vvcMaxNumRefFrame)
        {
            pCodecHalPic->PicFlags = PICTURE_INVALID;
        }
        else
        {
            // Check long/short reference flag
            if (vaPic.flags & VA_PICTURE_VVC_LONG_TERM_REFERENCE)
            {
                pCodecHalPic->PicFlags = PICTURE_LONG_TERM_REFERENCE;
            }
            else
            {
                pCodecHalPic->PicFlags = PICTURE_SHORT_TERM_REFERENCE;
            }
        }
        break;
    default:
        DDI_CODEC_ASSERTMESSAGE("Unsupported VVC PicEntry type.")
        break;
    }
}


VAStatus DdiDecodeVvc::ParseSliceParams(
    DDI_MEDIA_CONTEXT          *mediaCtx,
    VASliceParameterBufferVVC  *slcParam,
    uint32_t                   numSlices)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    CodechalDecodeParams *decodeParams    = &m_decodeCtx->DecodeParams;
    CodecVvcSliceParams  *pVvcSliceParams = static_cast<CodecVvcSliceParams*>(decodeParams->m_sliceParams);
    pVvcSliceParams += m_decodeCtx->DecodeParams.m_numSlices;
    CodecVvcPicParams    *pVvcPicParams   = static_cast<CodecVvcPicParams*>(decodeParams->m_picParams);
    if ((slcParam == nullptr) || (pVvcSliceParams == nullptr) || (pVvcPicParams == nullptr))
    {
        DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing VVC Slice parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if(vvcMaxSliceNum < numSlices)
    {
        DDI_CODEC_ASSERTMESSAGE("numSlices = %d exceeds max size = %d", numSlices, vvcMaxSliceNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    
    MOS_ZeroMemory(pVvcSliceParams, (numSlices * sizeof(CodecVvcSliceParams)));

    uint32_t sliceBaseOffset = GetBsBufOffset(m_groupIndex);
    for (uint32_t iSlcCount = 0; iSlcCount < numSlices; iSlcCount++)
    {
        pVvcSliceParams->m_bSNALunitDataLocation    = sliceBaseOffset + slcParam->slice_data_offset;
        pVvcSliceParams->m_sliceBytesInBuffer       = slcParam->slice_data_size;
        pVvcSliceParams->m_shSubpicId               = slcParam->sh_subpic_id;
        pVvcSliceParams->m_shSliceAddress           = slcParam->sh_slice_address;
        pVvcSliceParams->m_shNumTilesInSliceMinus1  = slcParam->sh_num_tiles_in_slice_minus1;
        pVvcSliceParams->m_shSliceType              = slcParam->sh_slice_type;
        pVvcSliceParams->m_shNumAlfApsIdsLuma       = slcParam->sh_num_alf_aps_ids_luma;

        MOS_SecureMemcpy(pVvcSliceParams->m_shAlfApsIdLuma, 7, slcParam->sh_alf_aps_id_luma, 7);

        pVvcSliceParams->m_shAlfApsIdChroma         = slcParam->sh_alf_aps_id_chroma;
        pVvcSliceParams->m_shAlfCcCbApsId           = slcParam->sh_alf_cc_cb_aps_id;
        pVvcSliceParams->m_shAlfCcCrApsId           = slcParam->sh_alf_cc_cr_aps_id;

        MOS_SecureMemcpy(pVvcSliceParams->m_numRefIdxActive, 2, slcParam->NumRefIdxActive, 2);

        pVvcSliceParams->m_shCollocatedRefIdx       = slcParam->sh_collocated_ref_idx;
        pVvcSliceParams->m_sliceQpY                 = slcParam->SliceQpY;
        pVvcSliceParams->m_shCbQpOffset             = slcParam->sh_cb_qp_offset;
        pVvcSliceParams->m_shCrQpOffset             = slcParam->sh_cr_qp_offset;
        pVvcSliceParams->m_shJointCbcrQpOffset      = slcParam->sh_joint_cbcr_qp_offset;
        pVvcSliceParams->m_shLumaBetaOffsetDiv2     = slcParam->sh_luma_beta_offset_div2;
        pVvcSliceParams->m_shLumaTcOffsetDiv2       = slcParam->sh_luma_tc_offset_div2;
        pVvcSliceParams->m_shCbBetaOffsetDiv2       = slcParam->sh_cb_beta_offset_div2;
        pVvcSliceParams->m_shCbTcOffsetDiv2         = slcParam->sh_cb_tc_offset_div2;
        pVvcSliceParams->m_shCrBetaOffsetDiv2       = slcParam->sh_cr_beta_offset_div2;
        pVvcSliceParams->m_shCrTcOffsetDiv2         = slcParam->sh_cr_tc_offset_div2;
        pVvcSliceParams->m_byteOffsetToSliceData    = slcParam->slice_data_byte_offset;

        // Set up RefPicList[2][vvcMaxNumRefFrame]
        for (auto i = 0; i < 2; i++)
        {
            for (auto j = 0; j < vvcMaxNumRefFrame; j++)
            {
                PCODEC_PICTURE pCodecHalPic = &pVvcSliceParams->m_refPicList[i][j];
                pCodecHalPic->FrameIdx = (slcParam->RefPicList[i][j] == 0xff) ? CODEC_MAX_DPB_NUM_VVC : slcParam->RefPicList[i][j];
                if (pCodecHalPic->FrameIdx >= vvcMaxNumRefFrame)
                {
                    pCodecHalPic->PicFlags = PICTURE_INVALID;
                }
                else
                {
                    VAPictureVVC vaPic = {};
                    vaPic.flags = m_refListFlags[pCodecHalPic->FrameIdx];
                    SetupCodecPicture(mediaCtx,
                        &m_decodeCtx->RTtbl,
                        pCodecHalPic,
                        vaPic,
                        VvcPicEntryRefPicList);
                }
            }
        }

        DDI_CODEC_CHK_RET(ParseWeightedPredInfo(pVvcSliceParams,&slcParam->WPInfo), "Parse Weighted Pred Info failed");
        pVvcSliceParams->m_longSliceFlags.m_value = slcParam->sh_flags.value;

        bool noBackWardPredFlag = true;
        uint8_t  refIdx = 0;
        if (pVvcSliceParams->m_shSliceType != vvcSliceI)
        {
            for (refIdx = 0; refIdx < pVvcSliceParams->m_numRefIdxActive[0] && noBackWardPredFlag; refIdx++)
            {
                uint8_t refPicIdx = pVvcSliceParams->m_refPicList[0][refIdx].FrameIdx;

                if (refPicIdx < vvcMaxNumRefFrame && pVvcPicParams->m_refFramePocList[refPicIdx] > pVvcPicParams->m_picOrderCntVal)
                {
                    noBackWardPredFlag = false;
                }
            }
            if (pVvcSliceParams->m_shSliceType == vvcSliceB)
            {
                for (refIdx = 0; refIdx < pVvcSliceParams->m_numRefIdxActive[1] && noBackWardPredFlag; refIdx++)
                {
                    uint8_t refPicIdx = pVvcSliceParams->m_refPicList[1][refIdx].FrameIdx;
                    if (refPicIdx < vvcMaxNumRefFrame && pVvcPicParams->m_refFramePocList[refPicIdx] > pVvcPicParams->m_picOrderCntVal)
                    {
                        noBackWardPredFlag = false;
                    }
                }
            }
        }
        else
        {
            noBackWardPredFlag = false;
        }
        pVvcSliceParams->m_longSliceFlags.m_fields.m_noBackwardPredFlag = noBackWardPredFlag;
        slcParam++;
        pVvcSliceParams++;
    }

    return VA_STATUS_SUCCESS;
}


VAStatus DdiDecodeVvc::ParseSubPicParams(
    DDI_MEDIA_CONTEXT *mediaCtx,
    VASubPicVVC       *subPicParam,
    uint32_t          numSubPics,
    uint32_t          numSubPicbuffers)
{
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    CodecVvcPicParams *pVvcPicParams = static_cast<CodecVvcPicParams*>(m_decodeCtx->DecodeParams.m_picParams);
    if ((subPicParam == nullptr) || (pVvcPicParams == nullptr))
    {
        DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing VVC SubPic parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // assume PicParam is always parsed before this buffer
    if (!pVvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag || (pVvcPicParams->m_spsNumSubpicsMinus1 == 0))
    {
        DDI_CODEC_NORMALMESSAGE("No SubPic number indicated by Pic Params buffer, just ignore the SubPic Buffer.");
        return VA_STATUS_SUCCESS;
    }

    if(vvcMaxSubpicNum < (numSubPics + numSubPicbuffers))
    {
        DDI_CODEC_ASSERTMESSAGE("numSubPics = %d exceeds max size = %d", numSubPics + numSubPicbuffers, vvcMaxSubpicNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    // Actual parsing
    CodecVvcSubpicParam* pVvcSubPicParams =
        static_cast<CodecVvcSubpicParam*>(m_decodeCtx->DecodeParams.m_extPicParams);
    pVvcSubPicParams += numSubPicbuffers;
    MOS_ZeroMemory(pVvcSubPicParams, (numSubPics * sizeof(CodecVvcSubpicParam)));
    for (uint32_t iSubPicCount = 0; iSubPicCount < numSubPics; iSubPicCount++)
    {
        pVvcSubPicParams->m_spsSubpicCtuTopLeftX  = subPicParam->sps_subpic_ctu_top_left_x;
        pVvcSubPicParams->m_spsSubpicCtuTopLeftY  = subPicParam->sps_subpic_ctu_top_left_y;
        pVvcSubPicParams->m_spsSubpicWidthMinus1  = subPicParam->sps_subpic_width_minus1;
        pVvcSubPicParams->m_spsSubpicHeightMinus1 = subPicParam->sps_subpic_height_minus1;
        pVvcSubPicParams->m_subpicIdVal           = subPicParam->SubpicIdVal;
        pVvcSubPicParams->m_subPicFlags.m_value   = subPicParam->subpic_flags.value;

        subPicParam++;
        pVvcSubPicParams++;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVvc::ParseSliceStructParams(
    DDI_MEDIA_CONTEXT *mediaCtx,
    VASliceStructVVC  *sliceStructParam,
    uint32_t          numSliceStructs,
    uint32_t          numSliceStructBuffers)
{
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    if(vvcMaxSliceNum < (numSliceStructs + numSliceStructBuffers))
    {
        DDI_CODEC_ASSERTMESSAGE("numSliceStructs = %d exceeds max size = %d", numSliceStructs + numSliceStructBuffers, vvcMaxSliceNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    CodecVvcSliceStructure* pVvcSliceStructParams =
        static_cast<CodecVvcSliceStructure*>(m_decodeCtx->DecodeParams.m_extSliceParams);
    pVvcSliceStructParams += numSliceStructBuffers;
    MOS_ZeroMemory(pVvcSliceStructParams, (numSliceStructs * sizeof(CodecVvcSliceStructure)));
    for(int iDataCount = 0 ; iDataCount < numSliceStructs; iDataCount++)
    {
        pVvcSliceStructParams->m_sliceTopLeftTileIdx           = sliceStructParam->SliceTopLeftTileIdx;
        pVvcSliceStructParams->m_ppsSliceWidthInTilesMinus1    = sliceStructParam->pps_slice_width_in_tiles_minus1;
        pVvcSliceStructParams->m_ppsSliceHeightInTilesMinus1   = sliceStructParam->pps_slice_height_in_tiles_minus1;
        pVvcSliceStructParams->m_ppsExpSliceHeightInCtusMinus1 = sliceStructParam->pps_exp_slice_height_in_ctus_minus1;
        sliceStructParam++;
        pVvcSliceStructParams++;
    }

    return VA_STATUS_SUCCESS;
}

} // namespace decode
