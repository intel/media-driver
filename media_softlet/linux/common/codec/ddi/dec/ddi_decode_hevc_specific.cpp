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
//! \file     ddi_decode_hevc_specific.cpp
//! \brief    HEVC class definition for DDI media decoder
//!

#include "ddi_decode_functions.h"
#include "media_libva_util_next.h"
#include "ddi_decode_hevc_specific.h"
#include "media_ddi_factory.h"
#include "media_libva_common_next.h"
#include "codec_def_decode_hevc.h"
#include "media_libva_interface_next.h"
#include "ddi_libva_decoder_specific.h"
#include "ddi_decode_trace_specific.h"

namespace decode
{

VAStatus DdiDecodeHevc::ParseSliceParams(
    DDI_MEDIA_CONTEXT          *mediaCtx,
    VASliceParameterBufferHEVC *slcParam,
    uint32_t                   numSlices)
{
    DDI_CODEC_FUNC_ENTER;

    VASliceParameterBufferHEVC *slc     = slcParam;
    VASliceParameterBufferBase *slcBase = (VASliceParameterBufferBase *)slcParam;
    bool isHevcRext = IsRextProfile();
    bool isHevcScc  = IsSccProfile();

    PCODEC_HEVC_SLICE_PARAMS codecSlcParams = (PCODEC_HEVC_SLICE_PARAMS)(m_decodeCtx->DecodeParams.m_sliceParams);
    codecSlcParams += m_decodeCtx->DecodeParams.m_numSlices;
    PCODEC_HEVC_EXT_SLICE_PARAMS codecSclParamsRext = nullptr;
    VASliceParameterBufferHEVCExtension *slcExtension = nullptr;
    VASliceParameterBufferHEVCRext *slcRext = nullptr;

    if (isHevcRext)
    {
        codecSclParamsRext = (PCODEC_HEVC_EXT_SLICE_PARAMS)(m_decodeCtx->DecodeParams.m_extSliceParams);
        codecSclParamsRext += m_decodeCtx->DecodeParams.m_numSlices;
        slcExtension = (VASliceParameterBufferHEVCExtension *)slcParam;
        slc     = &slcExtension->base;
        slcRext = &slcExtension->rext;
    }
     
    if ((slcParam == nullptr) || (codecSlcParams == nullptr) || (isHevcRext && (codecSclParamsRext == nullptr || slcRext == nullptr)))
    {
        DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing HEVC Slice parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    memset(codecSlcParams, 0, numSlices * sizeof(CODEC_HEVC_SLICE_PARAMS));
    if (isHevcRext)
    {
        memset(codecSclParamsRext, 0, numSlices * sizeof(CODEC_HEVC_EXT_SLICE_PARAMS));
    }

    uint32_t sliceBaseOffset = GetBsBufOffset(m_groupIndex);
    uint32_t i, j, slcCount;
    for (slcCount = 0; slcCount < numSlices; slcCount++)
    {
        if (m_decodeCtx->bShortFormatInUse)
        {
            codecSlcParams->slice_data_size   = slcBase->slice_data_size;
            codecSlcParams->slice_data_offset = sliceBaseOffset + slcBase->slice_data_offset;
            if (slcBase->slice_data_flag)
            {
                DDI_CODEC_NORMALMESSAGE("The whole slice is not in the bitstream buffer for this Execute call");
            }
            slcBase++;
        }
        else
        {
            codecSlcParams->slice_data_size   = slc->slice_data_size;
            codecSlcParams->slice_data_offset = sliceBaseOffset + slc->slice_data_offset;
            if (slcBase->slice_data_flag)
            {
                DDI_CODEC_NORMALMESSAGE("The whole slice is not in the bitstream buffer for this Execute call");
            }

            codecSlcParams->ByteOffsetToSliceData = slc->slice_data_byte_offset;
            codecSlcParams->NumEmuPrevnBytesInSliceHdr = slc->slice_data_num_emu_prevn_bytes;
            codecSlcParams->slice_segment_address = slc->slice_segment_address;

            for (i = 0; i < 2; i++)
            {
                for (j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; j++)
                {
                    codecSlcParams->RefPicList[i][j].FrameIdx = (slc->RefPicList[i][j] == 0xff) ? 0x7f : slc->RefPicList[i][j];
                }
            }

            codecSlcParams->LongSliceFlags.value           = slc->LongSliceFlags.value;
            codecSlcParams->collocated_ref_idx             = slc->collocated_ref_idx;
            codecSlcParams->num_ref_idx_l0_active_minus1   = slc->num_ref_idx_l0_active_minus1;
            codecSlcParams->num_ref_idx_l1_active_minus1   = slc->num_ref_idx_l1_active_minus1;
            codecSlcParams->slice_qp_delta                 = slc->slice_qp_delta;
            codecSlcParams->slice_cb_qp_offset             = slc->slice_cb_qp_offset;
            codecSlcParams->slice_cr_qp_offset             = slc->slice_cr_qp_offset;
            codecSlcParams->slice_beta_offset_div2         = slc->slice_beta_offset_div2;
            codecSlcParams->slice_tc_offset_div2           = slc->slice_tc_offset_div2;
            codecSlcParams->luma_log2_weight_denom         = slc->luma_log2_weight_denom;
            codecSlcParams->delta_chroma_log2_weight_denom = slc->delta_chroma_log2_weight_denom;

            MOS_SecureMemcpy(codecSlcParams->delta_luma_weight_l0,
                15,
                slc->delta_luma_weight_l0,
                15);
            MOS_SecureMemcpy(codecSlcParams->delta_luma_weight_l1,
                15,
                slc->delta_luma_weight_l1,
                15);

            MOS_SecureMemcpy(codecSlcParams->delta_chroma_weight_l0,
                15 * 2,
                slc->delta_chroma_weight_l0,
                15 * 2);
            MOS_SecureMemcpy(codecSlcParams->delta_chroma_weight_l1,
                15 * 2,
                slc->delta_chroma_weight_l1,
                15 * 2);
            codecSlcParams->five_minus_max_num_merge_cand = slc->five_minus_max_num_merge_cand;
            codecSlcParams->num_entry_point_offsets       = slc->num_entry_point_offsets;
            codecSlcParams->EntryOffsetToSubsetArray      = slc->entry_offset_to_subset_array;

            if (!isHevcRext)
            {
                MOS_SecureMemcpy(codecSlcParams->luma_offset_l0,
                    15,
                    slc->luma_offset_l0,
                    15);
                MOS_SecureMemcpy(codecSlcParams->luma_offset_l1,
                    15,
                    slc->luma_offset_l1,
                    15);
                MOS_SecureMemcpy(codecSlcParams->ChromaOffsetL0,
                    15 * 2,
                    slc->ChromaOffsetL0,
                    15 * 2);
                MOS_SecureMemcpy(codecSlcParams->ChromaOffsetL1,
                    15 * 2,
                    slc->ChromaOffsetL1,
                    15 * 2);
            
                slc++;
            }
            else
            {
                MOS_SecureMemcpy(codecSclParamsRext->luma_offset_l0,
                    15 * sizeof(int16_t),
                    slcRext->luma_offset_l0,
                    15 * sizeof(int16_t));
                MOS_SecureMemcpy(codecSclParamsRext->luma_offset_l1,
                    15 * sizeof(int16_t),
                    slcRext->luma_offset_l1,
                    15 * sizeof(int16_t));
                MOS_SecureMemcpy(codecSclParamsRext->ChromaOffsetL0,
                    15 * 2 * sizeof(int16_t),
                    slcRext->ChromaOffsetL0,
                    15 * 2 * sizeof(int16_t));
                MOS_SecureMemcpy(codecSclParamsRext->ChromaOffsetL1,
                    15 * 2 * sizeof(int16_t),
                    slcRext->ChromaOffsetL1,
                    15 * 2 * sizeof(int16_t));
                    
                codecSclParamsRext->cu_chroma_qp_offset_enabled_flag = slcRext->slice_ext_flags.bits.cu_chroma_qp_offset_enabled_flag;

                if (isHevcScc)
                {
                    codecSclParamsRext->use_integer_mv_flag    = slcRext->slice_ext_flags.bits.use_integer_mv_flag;
                    codecSclParamsRext->slice_act_y_qp_offset  = slcRext->slice_act_y_qp_offset;
                    codecSclParamsRext->slice_act_cb_qp_offset = slcRext->slice_act_cb_qp_offset;
                    codecSclParamsRext->slice_act_cr_qp_offset = slcRext->slice_act_cr_qp_offset;
                }
                
                codecSclParamsRext++;
                slcExtension++;
                slc     = &slcExtension->base;
                slcRext = &slcExtension->rext;
            }
        }
        codecSlcParams++;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeHevc::ParsePicParams(
    DDI_MEDIA_CONTEXT            *mediaCtx,
    VAPictureParameterBufferHEVC *picParam)
{
    DDI_CODEC_FUNC_ENTER;

    PCODEC_HEVC_PIC_PARAMS     codecPicParams    = (PCODEC_HEVC_PIC_PARAMS)(m_decodeCtx->DecodeParams.m_picParams);
    PCODEC_HEVC_EXT_PIC_PARAMS codecPicParamsExt = nullptr;
    PCODEC_HEVC_SCC_PIC_PARAMS codecPicParamsScc = nullptr;

    VAPictureParameterBufferHEVC     *picParamBase = nullptr;
    VAPictureParameterBufferHEVCRext *picParamRext = nullptr;
    VAPictureParameterBufferHEVCScc  *picParamScc  = nullptr;
    bool                             bIsHevcRext   = IsRextProfile();
    bool                             bIsHevcScc    = IsSccProfile();

    if (!bIsHevcRext)
    {
        picParamBase = picParam;
    }
    else
    {
        codecPicParamsExt = (PCODEC_HEVC_EXT_PIC_PARAMS)(m_decodeCtx->DecodeParams.m_extPicParams);
        picParamBase      = &((VAPictureParameterBufferHEVCExtension *)picParam)->base;
        picParamRext      = &((VAPictureParameterBufferHEVCExtension *)picParam)->rext;

        if (bIsHevcScc)
        {
            codecPicParamsScc = (PCODEC_HEVC_SCC_PIC_PARAMS)(m_decodeCtx->DecodeParams.m_advPicParams);;
            picParamScc = &((VAPictureParameterBufferHEVCExtension *)picParam)->scc;
        }
    }

    if ((picParamBase == nullptr) || (codecPicParams == nullptr) ||
        (bIsHevcRext && (picParamRext == nullptr || codecPicParamsExt == nullptr)) ||
        (bIsHevcScc && (picParamScc == nullptr || codecPicParamsScc == nullptr)))
    {
        DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing HEVC Picture parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    SetupCodecPicture(
        mediaCtx,
        &m_decodeCtx->RTtbl,
        &codecPicParams->CurrPic,
        picParamBase->CurrPic,
        0,  //picParam->pic_fields.bits.FieldPicFlag,
        0,  //picParam->pic_fields.bits.FieldPicFlag,
        false);
    if (codecPicParams->CurrPic.FrameIdx == (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    uint32_t i, j, k, l;
    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (picParamBase->ReferenceFrames[i].picture_id != VA_INVALID_SURFACE)
        {
            UpdateRegisteredRTSurfaceFlag(&(m_decodeCtx->RTtbl),
                MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, picParamBase->ReferenceFrames[i].picture_id));
        }
        SetupCodecPicture(
            mediaCtx,
            &m_decodeCtx->RTtbl,
            &(codecPicParams->RefFrameList[i]),
            picParamBase->ReferenceFrames[i],
            0,  // picParam->pic_fields.bits.FieldPicFlag,
            0,  // picParam->pic_fields.bits.FieldPicFlag,
            true);
        if (codecPicParams->RefFrameList[i].FrameIdx == (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX)
        {
            // in case the ref frame sent from App is wrong, set it to invalid ref frame index in codechal.
            codecPicParams->RefFrameList[i].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC;
        }
    }

    codecPicParams->PicWidthInMinCbsY  = picParamBase->pic_width_in_luma_samples  / (1 << (picParamBase->log2_min_luma_coding_block_size_minus3 + 3));
    codecPicParams->PicHeightInMinCbsY = picParamBase->pic_height_in_luma_samples / (1 << (picParamBase->log2_min_luma_coding_block_size_minus3 + 3));

    codecPicParams->chroma_format_idc                 = picParamBase->pic_fields.bits.chroma_format_idc;
    codecPicParams->separate_colour_plane_flag        = picParamBase->pic_fields.bits.separate_colour_plane_flag;
    codecPicParams->bit_depth_luma_minus8             = picParamBase->bit_depth_luma_minus8;
    codecPicParams->bit_depth_chroma_minus8           = picParamBase->bit_depth_chroma_minus8;
    codecPicParams->log2_max_pic_order_cnt_lsb_minus4 = picParamBase->log2_max_pic_order_cnt_lsb_minus4;
    codecPicParams->NoPicReorderingFlag               = picParamBase->pic_fields.bits.NoPicReorderingFlag;
    codecPicParams->NoBiPredFlag                      = picParamBase->pic_fields.bits.NoBiPredFlag;

    codecPicParams->sps_max_dec_pic_buffering_minus1         = picParamBase->sps_max_dec_pic_buffering_minus1;
    codecPicParams->log2_min_luma_coding_block_size_minus3   = picParamBase->log2_min_luma_coding_block_size_minus3;
    codecPicParams->log2_diff_max_min_luma_coding_block_size = picParamBase->log2_diff_max_min_luma_coding_block_size;
    codecPicParams->log2_min_transform_block_size_minus2     = picParamBase->log2_min_transform_block_size_minus2;
    codecPicParams->log2_diff_max_min_transform_block_size   = picParamBase->log2_diff_max_min_transform_block_size;
    codecPicParams->max_transform_hierarchy_depth_inter      = picParamBase->max_transform_hierarchy_depth_inter;
    codecPicParams->max_transform_hierarchy_depth_intra      = picParamBase->max_transform_hierarchy_depth_intra;
    codecPicParams->num_short_term_ref_pic_sets              = picParamBase->num_short_term_ref_pic_sets;
    codecPicParams->num_long_term_ref_pic_sps                = picParamBase->num_long_term_ref_pic_sps;
    codecPicParams->num_ref_idx_l0_default_active_minus1     = picParamBase->num_ref_idx_l0_default_active_minus1;
    codecPicParams->num_ref_idx_l1_default_active_minus1     = picParamBase->num_ref_idx_l1_default_active_minus1;
    codecPicParams->init_qp_minus26                          = picParamBase->init_qp_minus26;
    codecPicParams->ucNumDeltaPocsOfRefRpsIdx                = 0;  //redundant parameter, decoder may ignore
    codecPicParams->wNumBitsForShortTermRPSInSlice           = picParamBase->st_rps_bits;

    // dwCodingParamToolFlags
    codecPicParams->scaling_list_enabled_flag                    = picParamBase->pic_fields.bits.scaling_list_enabled_flag;
    codecPicParams->amp_enabled_flag                             = picParamBase->pic_fields.bits.amp_enabled_flag;
    codecPicParams->sample_adaptive_offset_enabled_flag          = picParamBase->slice_parsing_fields.bits.sample_adaptive_offset_enabled_flag;
    codecPicParams->pcm_enabled_flag                             = picParamBase->pic_fields.bits.pcm_enabled_flag;
    codecPicParams->pcm_sample_bit_depth_luma_minus1             = picParamBase->pcm_sample_bit_depth_luma_minus1;
    codecPicParams->pcm_sample_bit_depth_chroma_minus1           = picParamBase->pcm_sample_bit_depth_chroma_minus1;
    codecPicParams->log2_min_pcm_luma_coding_block_size_minus3   = picParamBase->log2_min_pcm_luma_coding_block_size_minus3;
    codecPicParams->log2_diff_max_min_pcm_luma_coding_block_size = picParamBase->log2_diff_max_min_pcm_luma_coding_block_size;
    codecPicParams->pcm_loop_filter_disabled_flag                = picParamBase->pic_fields.bits.pcm_loop_filter_disabled_flag;
    codecPicParams->long_term_ref_pics_present_flag              = picParamBase->slice_parsing_fields.bits.long_term_ref_pics_present_flag;
    codecPicParams->sps_temporal_mvp_enabled_flag                = picParamBase->slice_parsing_fields.bits.sps_temporal_mvp_enabled_flag;
    codecPicParams->strong_intra_smoothing_enabled_flag          = picParamBase->pic_fields.bits.strong_intra_smoothing_enabled_flag;
    codecPicParams->dependent_slice_segments_enabled_flag        = picParamBase->slice_parsing_fields.bits.dependent_slice_segments_enabled_flag;
    codecPicParams->output_flag_present_flag                     = picParamBase->slice_parsing_fields.bits.output_flag_present_flag;
    codecPicParams->num_extra_slice_header_bits                  = picParamBase->num_extra_slice_header_bits;
    codecPicParams->sign_data_hiding_enabled_flag                = picParamBase->pic_fields.bits.sign_data_hiding_enabled_flag;
    codecPicParams->cabac_init_present_flag                      = picParamBase->slice_parsing_fields.bits.cabac_init_present_flag;

    codecPicParams->constrained_intra_pred_flag              = picParamBase->pic_fields.bits.constrained_intra_pred_flag;
    codecPicParams->transform_skip_enabled_flag              = picParamBase->pic_fields.bits.transform_skip_enabled_flag;
    codecPicParams->cu_qp_delta_enabled_flag                 = picParamBase->pic_fields.bits.cu_qp_delta_enabled_flag;
    codecPicParams->pps_slice_chroma_qp_offsets_present_flag = picParamBase->slice_parsing_fields.bits.pps_slice_chroma_qp_offsets_present_flag;
    codecPicParams->weighted_pred_flag                       = picParamBase->pic_fields.bits.weighted_pred_flag;
    codecPicParams->weighted_bipred_flag                     = picParamBase->pic_fields.bits.weighted_bipred_flag;
    codecPicParams->transquant_bypass_enabled_flag           = picParamBase->pic_fields.bits.transquant_bypass_enabled_flag;
    codecPicParams->tiles_enabled_flag                       = picParamBase->pic_fields.bits.tiles_enabled_flag;
    codecPicParams->entropy_coding_sync_enabled_flag         = picParamBase->pic_fields.bits.entropy_coding_sync_enabled_flag;
    /* For va, uniform_spacing_flag==1, application should populate
         column_width_minus[], and row_height_minus1[] with approperiate values. */
    codecPicParams->uniform_spacing_flag                        = 0;
    codecPicParams->loop_filter_across_tiles_enabled_flag       = picParamBase->pic_fields.bits.loop_filter_across_tiles_enabled_flag;
    codecPicParams->pps_loop_filter_across_slices_enabled_flag  = picParamBase->pic_fields.bits.pps_loop_filter_across_slices_enabled_flag;
    codecPicParams->deblocking_filter_override_enabled_flag     = picParamBase->slice_parsing_fields.bits.deblocking_filter_override_enabled_flag;
    codecPicParams->pps_deblocking_filter_disabled_flag         = picParamBase->slice_parsing_fields.bits.pps_disable_deblocking_filter_flag;
    codecPicParams->lists_modification_present_flag             = picParamBase->slice_parsing_fields.bits.lists_modification_present_flag;
    codecPicParams->slice_segment_header_extension_present_flag = picParamBase->slice_parsing_fields.bits.slice_segment_header_extension_present_flag;
    codecPicParams->IrapPicFlag                                 = picParamBase->slice_parsing_fields.bits.RapPicFlag;
    codecPicParams->IdrPicFlag                                  = picParamBase->slice_parsing_fields.bits.IdrPicFlag;
    codecPicParams->IntraPicFlag                                = picParamBase->slice_parsing_fields.bits.IntraPicFlag;

    codecPicParams->pps_cb_qp_offset        = picParamBase->pps_cb_qp_offset;
    codecPicParams->pps_cr_qp_offset        = picParamBase->pps_cr_qp_offset;
    codecPicParams->num_tile_columns_minus1 = picParamBase->num_tile_columns_minus1;
    codecPicParams->num_tile_rows_minus1    = picParamBase->num_tile_rows_minus1;

    for (i = 0; i < HEVC_NUM_MAX_TILE_COLUMN - 1; i++)
    {
        codecPicParams->column_width_minus1[i] = picParamBase->column_width_minus1[i];
    }
    for (i = 0; i < HEVC_NUM_MAX_TILE_ROW - 1; i++)
    {
        codecPicParams->row_height_minus1[i] = picParamBase->row_height_minus1[i];
    }

    codecPicParams->diff_cu_qp_delta_depth           = picParamBase->diff_cu_qp_delta_depth;
    codecPicParams->pps_beta_offset_div2             = picParamBase->pps_beta_offset_div2;
    codecPicParams->pps_tc_offset_div2               = picParamBase->pps_tc_offset_div2;
    codecPicParams->log2_parallel_merge_level_minus2 = picParamBase->log2_parallel_merge_level_minus2;
    codecPicParams->CurrPicOrderCntVal               = picParamBase->CurrPic.pic_order_cnt;

    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        codecPicParams->PicOrderCntValList[i] = picParamBase->ReferenceFrames[i].pic_order_cnt;
    }

    for (i = 0; i < 8; i++)
    {
        codecPicParams->RefPicSetStCurrBefore[i] = 0xff;
        codecPicParams->RefPicSetStCurrAfter[i]  = 0xff;
        codecPicParams->RefPicSetLtCurr[i]       = 0xff;
    }

    j = k = l = 0;

    for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (picParamBase->ReferenceFrames[i].flags & VA_PICTURE_HEVC_RPS_ST_CURR_BEFORE)
        {
            DDI_CODEC_CHK_LESS(j, 8, "RefPicSetStCurrBefore[] index out of bounds.", VA_STATUS_ERROR_MAX_NUM_EXCEEDED);
            codecPicParams->RefPicSetStCurrBefore[j++] = i;
        }
        else if (picParamBase->ReferenceFrames[i].flags & VA_PICTURE_HEVC_RPS_ST_CURR_AFTER)
        {
            DDI_CODEC_CHK_LESS(k, 8, "RefPicSetStCurrAfter[] index out of bounds.", VA_STATUS_ERROR_MAX_NUM_EXCEEDED);
            codecPicParams->RefPicSetStCurrAfter[k++] = i;
        }
        else if (picParamBase->ReferenceFrames[i].flags & VA_PICTURE_HEVC_RPS_LT_CURR)
        {
            DDI_CODEC_CHK_LESS(l, 8, "RefPicSetLtCurr[] index out of bounds.", VA_STATUS_ERROR_MAX_NUM_EXCEEDED);
            codecPicParams->RefPicSetLtCurr[l++] = i;
        }
    }

    codecPicParams->RefFieldPicFlag            = 0;
    codecPicParams->RefBottomFieldFlag         = 0;
    codecPicParams->StatusReportFeedbackNumber = 0;

    if (bIsHevcRext)
    {   
        codecPicParamsExt->PicRangeExtensionFlags.dwRangeExtensionPropertyFlags    = picParamRext->range_extension_pic_fields.value;
        codecPicParamsExt->diff_cu_chroma_qp_offset_depth                          = picParamRext->diff_cu_chroma_qp_offset_depth;
        codecPicParamsExt->chroma_qp_offset_list_len_minus1                        = picParamRext->chroma_qp_offset_list_len_minus1;
        codecPicParamsExt->log2_sao_offset_scale_luma                              = picParamRext->log2_sao_offset_scale_luma;
        codecPicParamsExt->log2_sao_offset_scale_chroma                            = picParamRext->log2_sao_offset_scale_chroma;
        codecPicParamsExt->log2_max_transform_skip_block_size_minus2               = picParamRext->log2_max_transform_skip_block_size_minus2;

        for (i = 0; i < 6; i++)
        {
            codecPicParamsExt->cb_qp_offset_list[i] = picParamRext->cb_qp_offset_list[i];
            codecPicParamsExt->cr_qp_offset_list[i] = picParamRext->cr_qp_offset_list[i];
        }
    }

    if (bIsHevcScc)
    {
        codecPicParamsScc->PicSCCExtensionFlags.dwScreenContentCodingPropertyFlags = picParamScc->screen_content_pic_fields.value;
        codecPicParamsScc->palette_max_size                                        = picParamScc->palette_max_size;
        codecPicParamsScc->delta_palette_max_predictor_size                        = picParamScc->delta_palette_max_predictor_size;
        codecPicParamsScc->PredictorPaletteSize                                    = picParamScc->predictor_palette_size;
        codecPicParamsScc->pps_act_y_qp_offset_plus5                               = picParamScc->pps_act_y_qp_offset_plus5;
        codecPicParamsScc->pps_act_cb_qp_offset_plus5                              = picParamScc->pps_act_cb_qp_offset_plus5;
        codecPicParamsScc->pps_act_cr_qp_offset_plus3                              = picParamScc->pps_act_cr_qp_offset_plus3;
        uint32_t uiCopySize = sizeof(codecPicParamsScc->PredictorPaletteEntries);
        MOS_SecureMemcpy(&codecPicParamsScc->PredictorPaletteEntries, uiCopySize, &picParamScc->predictor_palette_entries, uiCopySize);
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeHevc::ParseIQMatrix(
    DDI_MEDIA_CONTEXT    *mediaCtx,
    VAIQMatrixBufferHEVC *matrix)
{
    DDI_CODEC_FUNC_ENTER;

    PCODECHAL_HEVC_IQ_MATRIX_PARAMS iqMatrix =
        (PCODECHAL_HEVC_IQ_MATRIX_PARAMS)(m_decodeCtx->DecodeParams.m_iqMatrixBuffer);

    if ((matrix == nullptr) || (iqMatrix == nullptr))
    {
        DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing HEVC IQMatrix parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    MOS_SecureMemcpy((void *)iqMatrix->ucScalingLists0,
        6 * 16 * sizeof(uint8_t),
        (void *)matrix->ScalingList4x4,
        6 * 16 * sizeof(uint8_t));
    MOS_SecureMemcpy((void *)iqMatrix->ucScalingLists1,
        6 * 64 * sizeof(uint8_t),
        (void *)matrix->ScalingList8x8,
        6 * 64 * sizeof(uint8_t));
    MOS_SecureMemcpy((void *)iqMatrix->ucScalingLists2,
        6 * 64 * sizeof(uint8_t),
        (void *)matrix->ScalingList16x16,
        6 * 64 * sizeof(uint8_t));
    MOS_SecureMemcpy((void *)iqMatrix->ucScalingLists3,
        2 * 64 * sizeof(uint8_t),
        (void *)matrix->ScalingList32x32,
        2 * 64 * sizeof(uint8_t));
    MOS_SecureMemcpy((void *)iqMatrix->ucScalingListDCCoefSizeID2,
        6 * sizeof(uint8_t),
        (void *)matrix->ScalingListDC16x16,
        6 * sizeof(uint8_t));
    MOS_SecureMemcpy((void *)iqMatrix->ucScalingListDCCoefSizeID3,
        2 * sizeof(uint8_t),
        (void *)matrix->ScalingListDC32x32,
        2 * sizeof(uint8_t));

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeHevc::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    DDI_CODEC_FUNC_ENTER;

    VAStatus va = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);

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
            if (buf->uiNumElements == 0)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            VASliceParameterBufferHEVC *slcInfoHEVC = (VASliceParameterBufferHEVC *)data;
            uint32_t numSlices = buf->uiNumElements;
            DDI_CODEC_CHK_RET(AllocSliceParamContext(numSlices),"AllocSliceParamContext failed!");
            DDI_CODEC_CHK_RET(ParseSliceParams(mediaCtx, slcInfoHEVC, numSlices),"ParseSliceParams failed!");
            m_decodeCtx->DecodeParams.m_numSlices += numSlices;
            m_groupIndex++;
            break;
        }
        case VAIQMatrixBufferType:
        {
            VAIQMatrixBufferHEVC *imxBuf = (VAIQMatrixBufferHEVC *)data;
            DDI_CODEC_CHK_RET(ParseIQMatrix(mediaCtx, imxBuf),"ParseIQMatrix failed!");

            break;
        }
        case VAPictureParameterBufferType:
        {
            VAPictureParameterBufferHEVC *picParam = (VAPictureParameterBufferHEVC *)data;
            DDI_CODEC_CHK_RET(ParsePicParams(mediaCtx, picParam),"ParsePicParams failed!");
            break;
        }
        case VASubsetsParameterBufferType:
        {

            if (m_decodeCtx->DecodeParams.m_subsetParams == nullptr) {
                m_decodeCtx->DecodeParams.m_subsetParams = MOS_AllocAndZeroMemory(sizeof(CODEC_HEVC_SUBSET_PARAMS));

                if (m_decodeCtx->DecodeParams.m_subsetParams == nullptr)
                    break;
            }

            MOS_SecureMemcpy(m_decodeCtx->DecodeParams.m_subsetParams, dataSize, data, dataSize);

            break;
        }
        case VAProcPipelineParameterBufferType:
        {
            DDI_CODEC_CHK_RET(ParseProcessingBuffer(mediaCtx, data),"ParseProcessingBuffer failed!");
            break;
        }
        case VADecodeStreamoutBufferType:
        {
            MediaLibvaCommonNext::MediaBufferToMosResource(buf, &m_decodeCtx->BufMgr.resExternalStreamOutBuffer);
            m_streamOutEnabled = true;
            break;
        }
        default:
            va = m_decodeCtx->pCpDdiInterfaceNext->RenderCencPicture(ctx, context, buf, data);
            break;
        }
        MediaLibvaInterfaceNext::UnmapBuffer(ctx, buffers[i]);
    }

    return va;
}

MOS_FORMAT DdiDecodeHevc::GetFormat()
{
    DDI_CODEC_FUNC_ENTER;

    MOS_FORMAT Format = Format_NV12;
    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_decodeCtx->RTtbl);
    CodechalDecodeParams  *decodeParams  = &m_decodeCtx->DecodeParams;
    CODEC_HEVC_PIC_PARAMS *picParams     = (CODEC_HEVC_PIC_PARAMS *)decodeParams->m_picParams;
    if ((m_ddiDecodeAttr->profile == VAProfileHEVCMain10) &&
        ((picParams->bit_depth_luma_minus8 ||
        picParams->bit_depth_chroma_minus8)))
    {
        Format = Format_P010;
        if (picParams->chroma_format_idc == 2)
        {
            Format = Format_Y210;
        }
        else if (picParams->chroma_format_idc == 3)
        {
            Format = Format_Y410;
        }
    }
    else if (m_ddiDecodeAttr->profile == VAProfileHEVCMain10
            && picParams->bit_depth_luma_minus8 == 0
            && picParams->bit_depth_chroma_minus8 == 0
            && rtTbl->pCurrentRT->format == Media_Format_P010)
    {
        // for hevc deocde 8bit in 10bit, the app will pass the render
        // target surface with the P010.
        Format = Format_P010;
    }
    else if (m_ddiDecodeAttr->profile == VAProfileHEVCMain12)
    {
        Format = Format_P016;
    }
    else if (m_ddiDecodeAttr->profile == VAProfileHEVCMain422_10
            || m_ddiDecodeAttr->profile == VAProfileHEVCMain422_12
            || m_ddiDecodeAttr->profile == VAProfileHEVCMain444
            || m_ddiDecodeAttr->profile == VAProfileHEVCMain444_10
            || m_ddiDecodeAttr->profile == VAProfileHEVCMain444_12)
    {
        Format = Format_NV12;
        if (picParams->bit_depth_luma_minus8 == 0
            && picParams->bit_depth_chroma_minus8 == 0) // 8bit
        {
            if (picParams->chroma_format_idc == 1) // 420
            {
                Format = Format_NV12;
                if (rtTbl->pCurrentRT->format == Media_Format_P010)
                {
                    Format = Format_P010;
                }
                else if (rtTbl->pCurrentRT->format == Media_Format_P016 || rtTbl->pCurrentRT->format == Media_Format_P012)
                {
                    Format = Format_P016;
                }
            }
            else if (picParams->chroma_format_idc == 2) // 422
            {
                Format = Format_YUY2;
                if (rtTbl->pCurrentRT->format == Media_Format_Y210)
                {
                    Format = Format_Y210;
                }
#if VA_CHECK_VERSION(1, 9, 0)
                else if (rtTbl->pCurrentRT->format == Media_Format_Y216 || rtTbl->pCurrentRT->format == Media_Format_Y212)
#else
                else if (rtTbl->pCurrentRT->format == Media_Format_Y216)
#endif
                {
                    Format = Format_Y216;
                }
            }
            else // 444
            {
                Format = Format_AYUV;
                if (rtTbl->pCurrentRT->format == Media_Format_Y410)
                {
                    Format = Format_Y410;
                }
#if VA_CHECK_VERSION(1, 9, 0)
                else if (rtTbl->pCurrentRT->format == Media_Format_Y416 || rtTbl->pCurrentRT->format == Media_Format_Y412)
#else
                else if (rtTbl->pCurrentRT->format == Media_Format_Y416)
#endif
                {
                    Format = Format_Y416;
                }
            }
        }
        else if (picParams->bit_depth_luma_minus8 == 1
                || picParams->bit_depth_chroma_minus8 == 1
                || picParams->bit_depth_luma_minus8 == 2
                || picParams->bit_depth_chroma_minus8 == 2) // 10bit
        {
            if (picParams->chroma_format_idc == 1) // 420
            {
                Format = Format_P010;
                if (rtTbl->pCurrentRT->format == Media_Format_P016 || rtTbl->pCurrentRT->format == Media_Format_P012)
                {
                    Format = Format_P016;
                }
            }
            else if (picParams->chroma_format_idc == 2) // 422
            {
                Format = Format_Y210;
#if VA_CHECK_VERSION(1, 9, 0)
                if (rtTbl->pCurrentRT->format == Media_Format_Y216 || rtTbl->pCurrentRT->format == Media_Format_Y212)
#else
                if (rtTbl->pCurrentRT->format == Media_Format_Y216)
#endif
                {
                    Format = Format_Y216;
                }
            }
            else // 444
            {
                Format = Format_Y410;
#if VA_CHECK_VERSION(1, 9, 0)
                if (rtTbl->pCurrentRT->format == Media_Format_Y416 || rtTbl->pCurrentRT->format == Media_Format_Y412)
#else
                if (rtTbl->pCurrentRT->format == Media_Format_Y416)
#endif
                {
                    Format = Format_Y416;
                }
            }
        }
        else if (picParams->bit_depth_luma_minus8 >= 3
                || picParams->bit_depth_chroma_minus8 >= 3) // 12bit
        {
            if (picParams->chroma_format_idc == 1) // 420
            {
                Format = Format_P016;
            }
            else if (picParams->chroma_format_idc == 2) // 422
            {
                Format = Format_Y216;
            }
            else // 444
            {
                Format = Format_Y416;
            }
        }
    }
    else if (m_ddiDecodeAttr->profile == VAProfileHEVCSccMain10)
    {
        // 420 10bit
        Format = Format_P010;
    }
    else if (m_ddiDecodeAttr->profile == VAProfileHEVCSccMain444 ||
             m_ddiDecodeAttr->profile == VAProfileHEVCSccMain444_10)
    {
        // 420/422/444 8bit
        if ((picParams->bit_depth_luma_minus8 == 0) &&
            (picParams->bit_depth_chroma_minus8 == 0))
        {
            if (picParams->chroma_format_idc == 2)
            {
                Format = Format_YUY2;
            }
            else if (picParams->chroma_format_idc == 3)
            {
                Format = Format_AYUV;
            }
            else
            {
                Format = Format_NV12;
            }
        }
        else
        {
            // 10bit
            if (picParams->chroma_format_idc == 2)
            {
                Format = Format_Y210;
            }
            else if (picParams->chroma_format_idc == 3)
            {
                Format = Format_Y410;
            }
            else
            {
                Format = Format_P010;
            }
        }
    }
    
    return Format;
}

VAStatus DdiDecodeHevc::SetDecodeParams()
{
    DDI_CODEC_FUNC_ENTER;

     DDI_CODEC_CHK_RET(DdiDecodeBase::SetDecodeParams(), "SetDecodeParams failed!");
     CODEC_HEVC_PIC_PARAMS *picParams = (CODEC_HEVC_PIC_PARAMS *)(&m_decodeCtx->DecodeParams)->m_picParams;
     // "flat" scaling lists
     if (picParams->scaling_list_enabled_flag == 0)
     {
        PCODECHAL_HEVC_IQ_MATRIX_PARAMS matrixParams = (PCODECHAL_HEVC_IQ_MATRIX_PARAMS)(&m_decodeCtx->DecodeParams)->m_iqMatrixBuffer;

        memset(matrixParams->ucScalingLists0,
            0x10,
            6 * 16 * sizeof(uint8_t));

        memset(matrixParams->ucScalingLists1,
            0x10,
            6 * 64 * sizeof(uint8_t));

        memset(matrixParams->ucScalingLists2,
            0x10,
            6 * 64 * sizeof(uint8_t));

        memset(matrixParams->ucScalingLists3,
            0x10,
            2 * 64 * sizeof(uint8_t));

        memset(matrixParams->ucScalingListDCCoefSizeID2,
            0x10,
            6 * sizeof(uint8_t));

        memset(matrixParams->ucScalingListDCCoefSizeID3,
            0x10,
            2 * sizeof(uint8_t));
     }
#ifdef _DECODE_PROCESSING_SUPPORTED
        // Bridge the SFC input with vdbox output
    if (m_decProcessingType == VA_DEC_PROCESSING)
    {
        auto procParams = (DecodeProcessingParams *)m_decodeCtx->DecodeParams.m_procParams;
        procParams->m_inputSurface = (&m_decodeCtx->DecodeParams)->m_destSurface;
        // codechal_decode_sfc.c expects Input Width/Height information.
        procParams->m_inputSurface->dwWidth  = procParams->m_inputSurface->OsResource.iWidth;
        procParams->m_inputSurface->dwHeight = procParams->m_inputSurface->OsResource.iHeight;
        procParams->m_inputSurface->dwPitch  = procParams->m_inputSurface->OsResource.iPitch;
        procParams->m_inputSurface->Format   = procParams->m_inputSurface->OsResource.Format;

        if (m_requireInputRegion)
        {
            procParams->m_inputSurfaceRegion.m_x = 0;
            procParams->m_inputSurfaceRegion.m_y = 0;
            procParams->m_inputSurfaceRegion.m_width  = procParams->m_inputSurface->dwWidth;
            procParams->m_inputSurfaceRegion.m_height = procParams->m_inputSurface->dwHeight;
        }
    }
#endif
     return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeHevc::AllocSliceParamContext(uint32_t numSlices)
{
    DDI_CODEC_FUNC_ENTER;

    uint32_t baseSize = sizeof(CODEC_HEVC_SLICE_PARAMS);

    if (m_sliceParamBufNum < (m_decodeCtx->DecodeParams.m_numSlices + numSlices))
    {
        // in order to avoid that the buffer is reallocated multi-times,
        // extra 10 slices are added.
        uint32_t extraSlices = numSlices + 10;

        m_decodeCtx->DecodeParams.m_sliceParams = realloc(m_decodeCtx->DecodeParams.m_sliceParams,
            baseSize * (m_sliceParamBufNum + extraSlices));

        if (m_decodeCtx->DecodeParams.m_sliceParams == nullptr)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        memset((void *)((uint8_t *)m_decodeCtx->DecodeParams.m_sliceParams + baseSize * m_sliceParamBufNum), 0, baseSize * extraSlices);

        if (IsRextProfile())
        {
            uint32_t rextSize = sizeof(CODEC_HEVC_EXT_SLICE_PARAMS);
            m_decodeCtx->DecodeParams.m_extSliceParams = realloc(m_decodeCtx->DecodeParams.m_extSliceParams,
            rextSize * (m_sliceParamBufNum + extraSlices));

            if (m_decodeCtx->DecodeParams.m_extSliceParams == nullptr)
            {
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
    
            memset((void *)((uint8_t *)m_decodeCtx->DecodeParams.m_extSliceParams + rextSize * m_sliceParamBufNum), 0, rextSize * extraSlices);
        }
        
        m_sliceParamBufNum += extraSlices;
    }

    return VA_STATUS_SUCCESS;
}

void DdiDecodeHevc::DestroyContext(VADriverContextP ctx)
{
    DDI_CODEC_FUNC_ENTER;

    FreeResourceBuffer();
    // explicitly call the base function to do the further clean-up
    DdiDecodeBase::DestroyContext(ctx);

    return;
}

void DdiDecodeHevc::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    DDI_CODEC_FUNC_ENTER;

    // call the function in base class to initialize it.
    DdiDecodeBase::ContextInit(picWidth, picHeight);

    if (m_ddiDecodeAttr->componentData.data.sliceMode == VA_DEC_SLICE_MODE_BASE)
    {
        m_decodeCtx->bShortFormatInUse = true;
    }
    m_decodeCtx->wMode = CODECHAL_DECODE_MODE_HEVCVLD;

    return;
}

VAStatus DdiDecodeHevc::InitResourceBuffer()
{
    DDI_CODEC_FUNC_ENTER;

    VAStatus vaStatus = VA_STATUS_SUCCESS;

    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_decodeCtx->BufMgr);
    bufMgr->pSliceData = nullptr;

    bufMgr->ui64BitstreamOrder = 0;

    if (m_width * m_height < CODEC_720P_MAX_PIC_WIDTH * CODEC_720P_MAX_PIC_HEIGHT)
    {
        bufMgr->dwMaxBsSize = m_width * m_height * 3 / 2;
    }
    else if (m_width * m_height < CODEC_4K_MAX_PIC_WIDTH * CODEC_4K_MAX_PIC_HEIGHT)
    {
        bufMgr->dwMaxBsSize = m_width * m_height * 3 / 8;
    }
    else
    {
        bufMgr->dwMaxBsSize = m_width * m_height * 3 / 16;
    }

    // minimal 10k bytes for some special case. Will refractor this later
    if (bufMgr->dwMaxBsSize < DDI_CODEC_MIN_VALUE_OF_MAX_BS_SIZE)
    {
        bufMgr->dwMaxBsSize = DDI_CODEC_MIN_VALUE_OF_MAX_BS_SIZE;
    }

    int32_t i;
    // init decode bitstream buffer object
    for (i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        bufMgr->pBitStreamBuffObject[i] = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
        if (bufMgr->pBitStreamBuffObject[i] == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            FreeResourceBuffer();
            return vaStatus;
        }
        bufMgr->pBitStreamBuffObject[i]->iSize    = bufMgr->dwMaxBsSize;
        bufMgr->pBitStreamBuffObject[i]->uiType   = VASliceDataBufferType;
        bufMgr->pBitStreamBuffObject[i]->format   = Media_Format_Buffer;
        bufMgr->pBitStreamBuffObject[i]->uiOffset = 0;
        bufMgr->pBitStreamBuffObject[i]->bo       = nullptr;
        bufMgr->pBitStreamBase[i]                 = nullptr;
    }

    // The pSliceData can be allocated on demand. So the default size is wPicHeightInLCU.
    // Currently the LCU32 is used.
    bufMgr->m_maxNumSliceData = MOS_ALIGN_CEIL(m_height, 32) / 32;
    bufMgr->pSliceData        = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)MOS_AllocAndZeroMemory(sizeof(bufMgr->pSliceData[0]) *
                                 bufMgr->m_maxNumSliceData);

    if (bufMgr->pSliceData == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        FreeResourceBuffer();
        return vaStatus;
    }

    bufMgr->dwNumSliceData    = 0;
    bufMgr->dwNumSliceControl = 0;

    /* as it can be increased on demand, the initial number will be based on LCU32 */
    m_sliceCtrlBufNum = MOS_ALIGN_CEIL(m_height, 32) / 32;

    if (m_decodeCtx->bShortFormatInUse)
    {
        bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC =
            (VASliceParameterBufferBase *)MOS_AllocAndZeroMemory(sizeof(VASliceParameterBufferBase) * m_sliceCtrlBufNum);
        if (bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            FreeResourceBuffer();
            return vaStatus;
        }
    }
    else if (!IsRextProfile())
    {
        bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC =
            (VASliceParameterBufferHEVC *)MOS_AllocAndZeroMemory(sizeof(VASliceParameterBufferHEVC) * m_sliceCtrlBufNum);
        if (bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            FreeResourceBuffer();
            return vaStatus;
        }
    }
    else
    {
        bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext =
            (VASliceParameterBufferHEVCExtension*)MOS_AllocAndZeroMemory(sizeof(VASliceParameterBufferHEVCExtension) * m_sliceCtrlBufNum);
        if (bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            FreeResourceBuffer();
            return vaStatus;
        }
    }

    return VA_STATUS_SUCCESS;
}

void DdiDecodeHevc::FreeResourceBuffer()
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_decodeCtx->BufMgr);

    int32_t i = 0;
    for (i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
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

    if (bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC)
    {
        MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC);
        bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC = nullptr;
    }
    if (bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC)
    {
        MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC);
        bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC = nullptr;
    }
    if (bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext)
    {
        MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext);
        bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext = nullptr;
    }

    // free decode bitstream buffer object
    MOS_FreeMemory(bufMgr->pSliceData);
    bufMgr->pSliceData = nullptr;

    return;
}

uint8_t* DdiDecodeHevc::GetPicParamBuf(DDI_CODEC_COM_BUFFER_MGR *bufMgr)
{ 
    DDI_CODEC_FUNC_ENTER;

    if (!IsRextProfile())
    {
        return (uint8_t*)(&(bufMgr->Codec_Param.Codec_Param_HEVC.PicParamHEVC));
    }
    else
    {
        return (uint8_t*)(&(bufMgr->Codec_Param.Codec_Param_HEVC.PicParamHEVCRext));
    }
} 

VAStatus DdiDecodeHevc::AllocSliceControlBuffer(DDI_MEDIA_BUFFER *buf)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_COM_BUFFER_MGR *bufMgr   = nullptr;
    uint32_t                 availSize = 0;
    uint32_t                 newSize   = 0;

    bufMgr    = &(m_decodeCtx->BufMgr);
    availSize = m_sliceCtrlBufNum - bufMgr->dwNumSliceControl;

    if (buf->uiNumElements < 1 || buf->iSize < 1)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    if (m_decodeCtx->bShortFormatInUse)
    {
        if (availSize < buf->uiNumElements)
        {
            if (buf->iSize / buf->uiNumElements != sizeof(VASliceParameterBufferBase))
                return VA_STATUS_ERROR_ALLOCATION_FAILED;

            newSize = sizeof(VASliceParameterBufferBase) * (m_sliceCtrlBufNum - availSize + buf->uiNumElements);
            bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC = (VASliceParameterBufferBase *)realloc(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC, newSize);
            if (bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC == nullptr)
            {
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
            MOS_ZeroMemory(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC + m_sliceCtrlBufNum, sizeof(VASliceParameterBufferBase) * (buf->uiNumElements - availSize));
            m_sliceCtrlBufNum = m_sliceCtrlBufNum - availSize + buf->uiNumElements;
        }
        buf->pData    = (uint8_t*)bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC;
        buf->uiOffset = bufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferBase);
    }
    else
    {
        if (!IsRextProfile())
        {
            if (availSize < buf->uiNumElements)
            {
                if (buf->iSize / buf->uiNumElements != sizeof(VASliceParameterBufferHEVC))
                    return VA_STATUS_ERROR_ALLOCATION_FAILED;

                newSize = sizeof(VASliceParameterBufferHEVC) * (m_sliceCtrlBufNum - availSize + buf->uiNumElements);
                bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC = (VASliceParameterBufferHEVC *)realloc(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC, newSize);
                if (bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC == nullptr)
                {
                    return VA_STATUS_ERROR_ALLOCATION_FAILED;
                }
                MOS_ZeroMemory(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC + m_sliceCtrlBufNum, sizeof(VASliceParameterBufferHEVC) * (buf->uiNumElements - availSize));
                m_sliceCtrlBufNum = m_sliceCtrlBufNum - availSize + buf->uiNumElements;
            }
            buf->pData    = (uint8_t*)bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC;
            buf->uiOffset = bufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferHEVC);
        }
        else
        {
            if (availSize < buf->uiNumElements)
            {
                if (buf->iSize / buf->uiNumElements != sizeof(VASliceParameterBufferHEVCExtension))
                    return VA_STATUS_ERROR_ALLOCATION_FAILED;

                newSize = sizeof(VASliceParameterBufferHEVCExtension) * (m_sliceCtrlBufNum - availSize + buf->uiNumElements);
                bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext= (VASliceParameterBufferHEVCExtension*)realloc(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext, newSize);
                if (bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext == nullptr)
                {
                    return VA_STATUS_ERROR_ALLOCATION_FAILED;
                }
                MOS_ZeroMemory(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext+ m_sliceCtrlBufNum, sizeof(VASliceParameterBufferHEVCExtension) * (buf->uiNumElements - availSize));
                m_sliceCtrlBufNum = m_sliceCtrlBufNum - availSize + buf->uiNumElements;
            }
            buf->pData    = (uint8_t*)bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext;
            buf->uiOffset = bufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferHEVCExtension);
        }
    }

    bufMgr->dwNumSliceControl += buf->uiNumElements;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeHevc::CodecHalInit(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *ptr)
{
    DDI_CODEC_FUNC_ENTER;

    VAStatus    vaStatus = VA_STATUS_SUCCESS;
    MOS_CONTEXT *mosCtx  = (MOS_CONTEXT *)ptr;

    CODECHAL_FUNCTION codecFunction = CODECHAL_FUNCTION_DECODE;
    m_decodeCtx->pCpDdiInterfaceNext->SetCpParams(m_ddiDecodeAttr->componentData.data.encryptType, m_codechalSettings);

    CODECHAL_STANDARD_INFO standardInfo;
    memset(&standardInfo, 0, sizeof(standardInfo));

    standardInfo.CodecFunction = codecFunction;
    standardInfo.Mode          = (CODECHAL_MODE)m_decodeCtx->wMode;

    m_codechalSettings->codecFunction = codecFunction;
    m_codechalSettings->width         = m_width;
    m_codechalSettings->height        = m_height;
    m_codechalSettings->intelEntrypointInUse = false;

    m_codechalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_8_BITS;
    if (m_ddiDecodeAttr->profile == VAProfileHEVCMain10     ||
        m_ddiDecodeAttr->profile == VAProfileHEVCMain422_10 ||
        m_ddiDecodeAttr->profile == VAProfileHEVCMain444_10 ||
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain10  ||
        m_ddiDecodeAttr->profile == VAProfileHEVCMain444_10)
    {
        m_codechalSettings->lumaChromaDepth |= CODECHAL_LUMA_CHROMA_DEPTH_10_BITS;
    }

    m_codechalSettings->shortFormatInUse = m_decodeCtx->bShortFormatInUse;

    m_codechalSettings->mode         = CODECHAL_DECODE_MODE_HEVCVLD;
    m_codechalSettings->standard     = CODECHAL_HEVC;
    m_codechalSettings->chromaFormat = HCP_CHROMA_FORMAT_YUV420;

    if (m_ddiDecodeAttr->profile == VAProfileHEVCMain422_10 ||
       m_ddiDecodeAttr->profile == VAProfileHEVCMain422_12)
    {
        m_codechalSettings->chromaFormat = HCP_CHROMA_FORMAT_YUV422;
    }
    
    if (m_ddiDecodeAttr->profile == VAProfileHEVCMain444    ||
        m_ddiDecodeAttr->profile == VAProfileHEVCMain444_10 ||
        m_ddiDecodeAttr->profile == VAProfileHEVCMain444_12 ||
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain444 ||
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain444_10)
    {
        m_codechalSettings->chromaFormat = HCP_CHROMA_FORMAT_YUV444;
    }

    m_decodeCtx->DecodeParams.m_iqMatrixBuffer = MOS_AllocAndZeroMemory(sizeof(CODECHAL_HEVC_IQ_MATRIX_PARAMS));
    if (m_decodeCtx->DecodeParams.m_iqMatrixBuffer == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }
    m_decodeCtx->DecodeParams.m_picParams = MOS_AllocAndZeroMemory(sizeof(CODEC_HEVC_PIC_PARAMS));
    if (m_decodeCtx->DecodeParams.m_picParams == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }
    if (IsRextProfile())
    {
        m_decodeCtx->DecodeParams.m_extPicParams = MOS_AllocAndZeroMemory(sizeof(CODEC_HEVC_EXT_PIC_PARAMS));
        if (m_decodeCtx->DecodeParams.m_extPicParams == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto CleanUpandReturn;
        }

        if (IsSccProfile())
        {
            m_decodeCtx->DecodeParams.m_advPicParams = MOS_AllocAndZeroMemory(sizeof(CODEC_HEVC_SCC_PIC_PARAMS));
            if (m_decodeCtx->DecodeParams.m_advPicParams == nullptr)
            {
                vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
                goto CleanUpandReturn;
            }
        }
    }

    m_sliceParamBufNum = m_picHeightInMB;
    m_decodeCtx->DecodeParams.m_sliceParams = MOS_AllocAndZeroMemory(m_sliceParamBufNum * sizeof(CODEC_HEVC_SLICE_PARAMS));
    if (m_decodeCtx->DecodeParams.m_sliceParams == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    if (IsRextProfile())
    {
        m_decodeCtx->DecodeParams.m_extSliceParams = MOS_AllocAndZeroMemory(m_sliceParamBufNum * sizeof(CODEC_HEVC_EXT_SLICE_PARAMS));
        if (m_decodeCtx->DecodeParams.m_extSliceParams == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto CleanUpandReturn;
        }
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_decProcessingType == VA_DEC_PROCESSING)
    {
        DecodeProcessingParams *procParams = nullptr;
        
        m_codechalSettings->downsamplingHinted = true;
        
        procParams = (DecodeProcessingParams *)MOS_AllocAndZeroMemory(sizeof(DecodeProcessingParams));
        if (procParams == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto CleanUpandReturn;
        }
        
        m_decodeCtx->DecodeParams.m_procParams = procParams;
        procParams->m_outputSurface = (PMOS_SURFACE)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE));
        if (procParams->m_outputSurface == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto CleanUpandReturn;
        }
    }
#endif

    vaStatus = CreateCodecHal(mediaCtx,
        ptr,
        &standardInfo);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto CleanUpandReturn;
    }

    if (InitResourceBuffer() != VA_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    return vaStatus;

CleanUpandReturn:
    FreeResourceBuffer();

    if (m_decodeCtx->pCodecHal)
    {
        m_decodeCtx->pCodecHal->Destroy();
        MOS_Delete(m_decodeCtx->pCodecHal);
        m_decodeCtx->pCodecHal = nullptr;
    }

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_iqMatrixBuffer);
    m_decodeCtx->DecodeParams.m_iqMatrixBuffer = nullptr;
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_picParams);
    m_decodeCtx->DecodeParams.m_picParams = nullptr;
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_huffmanTable);
    m_decodeCtx->DecodeParams.m_huffmanTable = nullptr;
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_sliceParams);
    m_decodeCtx->DecodeParams.m_sliceParams = nullptr;

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_decodeCtx->DecodeParams.m_procParams)
    {
        auto procParams =
            (DecodeProcessingParams *)m_decodeCtx->DecodeParams.m_procParams;
        MOS_FreeMemory(procParams->m_outputSurface);
        
        MOS_FreeMemory(m_decodeCtx->DecodeParams.m_procParams);
        m_decodeCtx->DecodeParams.m_procParams = nullptr;
    }
#endif
    return vaStatus;
}

bool DdiDecodeHevc::IsRextProfile()
{
    DDI_CODEC_FUNC_ENTER;

    return (                                                   \
        m_ddiDecodeAttr->profile == VAProfileHEVCMain12     || \
        m_ddiDecodeAttr->profile == VAProfileHEVCMain422_10 || \
        m_ddiDecodeAttr->profile == VAProfileHEVCMain422_12 || \
        m_ddiDecodeAttr->profile == VAProfileHEVCMain444    || \
        m_ddiDecodeAttr->profile == VAProfileHEVCMain444_10 || \
        m_ddiDecodeAttr->profile == VAProfileHEVCMain444_12 || \
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain    || \
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain10  || \
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain444 || \
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain444_10 \
    );
}

bool DdiDecodeHevc::IsSccProfile()
{
    DDI_CODEC_FUNC_ENTER;

    return (                                                   \
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain    || \
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain10  || \
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain444 || \
        m_ddiDecodeAttr->profile == VAProfileHEVCSccMain444_10 \
    );
}

void DdiDecodeHevc::SetupCodecPicture(
    DDI_MEDIA_CONTEXT              *mediaCtx,
    DDI_CODEC_RENDER_TARGET_TABLE  *rtTbl,
    CODEC_PICTURE                  *codecHalPic,
    VAPictureHEVC                  vaPic,
    bool                           fieldPicFlag,
    bool                           bottomFieldFlag,
    bool                           picReference)
{
    DDI_CODEC_FUNC_ENTER;

    if (vaPic.picture_id != VA_INVALID_SURFACE)
    {
        DDI_MEDIA_SURFACE *surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, vaPic.picture_id);
        codecHalPic->FrameIdx = GetRenderTargetID(rtTbl, surface);
    }
    else
    {
        codecHalPic->FrameIdx = (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX;
    }

    if (picReference)
    {
        if (codecHalPic->FrameIdx == (uint8_t)DDI_CODEC_INVALID_FRAME_INDEX)
        {
            codecHalPic->PicFlags = PICTURE_INVALID;
        }
        else if (vaPic.flags & VA_PICTURE_HEVC_LONG_TERM_REFERENCE)
        {
            codecHalPic->PicFlags = PICTURE_LONG_TERM_REFERENCE;
        }
        else
        {
            codecHalPic->PicFlags = PICTURE_SHORT_TERM_REFERENCE;
        }
    }
    else
    {
        if (fieldPicFlag)
        {
            if (bottomFieldFlag)
            {
                codecHalPic->PicFlags = PICTURE_BOTTOM_FIELD;
            }
            else
            {
                codecHalPic->PicFlags = PICTURE_TOP_FIELD;
            }
        }
        else
        {
            codecHalPic->PicFlags = PICTURE_FRAME;
        }
    }

    return;
}

} // namespace decode
