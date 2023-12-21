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
//! \file     ddi_decode_av1_specific.cpp
//! \brief    AV1 class definition for DDI media decoder
//!

#include "ddi_decode_functions.h"
#include "media_libva_util_next.h"
#include "ddi_decode_av1_specific.h"
#include "media_libva_interface_next.h"
#include "codec_def_decode_av1.h"
#include "ddi_decode_trace_specific.h"

namespace decode
{

VAStatus DdiDecodeAv1::ParseTileParams(
    DDI_MEDIA_CONTEXT         *mediaCtx,
    VASliceParameterBufferAV1 *slcParam,
    uint32_t                  numTiles)
{
    DDI_CODEC_FUNC_ENTER;

    CodecAv1TileParams        *tileParams = nullptr;
    VASliceParameterBufferAV1 *pTileCtrl  = nullptr;

    // if number of tile group exceed av1MaxTileNum, need to increase the memory
    if (av1MaxTileNum < numTiles)
    {
        DDI_CODEC_ASSERTMESSAGE("numTiles = %d : exceeds av1MaxTileNum = %d", numTiles, av1MaxTileNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    pTileCtrl  = slcParam;
    tileParams = (CodecAv1TileParams*)(m_decodeCtx->DecodeParams.m_sliceParams);
    tileParams += m_decodeCtx->DecodeParams.m_numSlices;

    if ((slcParam == nullptr) || (tileParams == nullptr))
    {
        DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing AV1 Slice parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    MOS_ZeroMemory(tileParams, (numTiles * sizeof(CodecAv1TileParams)));

    uint32_t sliceBaseOffset;
    sliceBaseOffset = GetBsBufOffset(m_groupIndex);

    for (auto idx = 0; idx < numTiles; idx++) {
        tileParams->m_bsTileDataLocation       = sliceBaseOffset + pTileCtrl->slice_data_offset;
        tileParams->m_bsTileBytesInBuffer      = pTileCtrl->slice_data_size;

        tileParams->m_badBSBufferChopping      = 0;                             // app doesn't have this
        tileParams->m_tileRow                  = pTileCtrl->tile_row;
        tileParams->m_tileColumn               = pTileCtrl->tile_column;

        tileParams->m_anchorFrameIdx.FrameIdx  = pTileCtrl->anchor_frame_idx;
        tileParams->m_tileIndex                = pTileCtrl->tile_idx_in_tile_list;
        tileParams->m_anchorFrameIdx.PicFlags  = PICTURE_FRAME;
        tileParams->m_anchorFrameIdx.PicEntry  = 0;                             // debug only

        tileParams->m_bsTilePayloadSizeInBytes = pTileCtrl->slice_data_size;

        tileParams++;
        pTileCtrl++;
    }

    return VA_STATUS_SUCCESS;
}

static uint32_t CalcAv1TileLog2(uint32_t blockSize, uint32_t target)
{
    DDI_CODEC_FUNC_ENTER;

    uint32_t k = 0;
    for (k = 0; (blockSize << k) < target; k++) {}
    return k;
}

/**
 * @brief AV1 Picture paramter parser
 *
 * Method Parse AV1 parametr
 *
 * @param medmaiCtx
 * @param picParam
 *
 * @retrun VA status
 *
 */
VAStatus DdiDecodeAv1::ParsePicParams(
    DDI_MEDIA_CONTEXT              *mediaCtx,
    VADecPictureParameterBufferAV1 *picParam)
{
    DDI_CODEC_FUNC_ENTER;

    CodecAv1PicParams* picAV1Params = (CodecAv1PicParams*)(m_decodeCtx->DecodeParams.m_picParams);

    if ((picParam == nullptr) || (picAV1Params == nullptr))
    {
        DDI_CODEC_ASSERTMESSAGE("Invalid Parameter for Parsing AV1 Picture parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    /***************************************************************************
     Setup current picture
     **************************************************************************/
    int32_t frameIdx = GetRenderTargetID(&m_decodeCtx->RTtbl, m_decodeCtx->RTtbl.pCurrentRT);
    if (frameIdx == DDI_CODEC_INVALID_FRAME_INDEX)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    picAV1Params->m_currPic.FrameIdx = frameIdx;

    picAV1Params->m_profile              = picParam->profile;
    picAV1Params->m_anchorFrameInsertion = 0;
    picAV1Params->m_anchorFrameNum       = picParam->anchor_frames_num;

    if (picAV1Params->m_anchorFrameNum > 0)
    {
        if (picParam->anchor_frames_num <= MAX_ANCHOR_FRAME_NUM_AV1)
        {
            MOS_SecureMemcpy(anchorFrameListVA, picParam->anchor_frames_num * sizeof(VASurfaceID),
                             picParam->anchor_frames_list, picParam->anchor_frames_num * sizeof(VASurfaceID));
        }
        else
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }

    picAV1Params->m_orderHintBitsMinus1                              = picParam->order_hint_bits_minus_1;
    picAV1Params->m_bitDepthIdx                                      = picParam->bit_depth_idx;

    picAV1Params->m_superResUpscaledWidthMinus1                      = picParam->frame_width_minus1;
    picAV1Params->m_superResUpscaledHeightMinus1                     = picParam->frame_height_minus1;
    picAV1Params->m_matrixCoefficients                               = picParam->matrix_coefficients;

    /***************************************************************************
     Sequence Info
    ***************************************************************************/
    picAV1Params->m_seqInfoFlags.m_fields.m_stillPicture             = picParam->seq_info_fields.fields.still_picture;
    picAV1Params->m_seqInfoFlags.m_fields.m_use128x128Superblock     = picParam->seq_info_fields.fields.use_128x128_superblock;
    picAV1Params->m_seqInfoFlags.m_fields.m_enableFilterIntra        = picParam->seq_info_fields.fields.enable_filter_intra;
    picAV1Params->m_seqInfoFlags.m_fields.m_enableIntraEdgeFilter    = picParam->seq_info_fields.fields.enable_intra_edge_filter;

    picAV1Params->m_seqInfoFlags.m_fields.m_enableInterintraCompound = picParam->seq_info_fields.fields.enable_interintra_compound;
    picAV1Params->m_seqInfoFlags.m_fields.m_enableMaskedCompound     = picParam->seq_info_fields.fields.enable_masked_compound;

    picAV1Params->m_seqInfoFlags.m_fields.m_enableDualFilter         = picParam->seq_info_fields.fields.enable_dual_filter;
    picAV1Params->m_seqInfoFlags.m_fields.m_enableOrderHint          = picParam->seq_info_fields.fields.enable_order_hint;
    picAV1Params->m_seqInfoFlags.m_fields.m_enableJntComp            = picParam->seq_info_fields.fields.enable_jnt_comp;
    picAV1Params->m_seqInfoFlags.m_fields.m_enableCdef               = picParam->seq_info_fields.fields.enable_cdef;
    picAV1Params->m_seqInfoFlags.m_fields.m_reserved3b               = 0;

    picAV1Params->m_seqInfoFlags.m_fields.m_monoChrome               = picParam->seq_info_fields.fields.mono_chrome;
    picAV1Params->m_seqInfoFlags.m_fields.m_colorRange               = picParam->seq_info_fields.fields.color_range;
    picAV1Params->m_seqInfoFlags.m_fields.m_subsamplingX             = picParam->seq_info_fields.fields.subsampling_x;
    picAV1Params->m_seqInfoFlags.m_fields.m_subsamplingY             = picParam->seq_info_fields.fields.subsampling_y;
    picAV1Params->m_seqInfoFlags.m_fields.m_filmGrainParamsPresent   = picParam->seq_info_fields.fields.film_grain_params_present;
    picAV1Params->m_seqInfoFlags.m_fields.m_reservedSeqInfoBits      = 0;

    /****************************************************************************
     Picture Info
    ****************************************************************************/
    picAV1Params->m_picInfoFlags.m_fields.m_frameType                = picParam->pic_info_fields.bits.frame_type;
    picAV1Params->m_picInfoFlags.m_fields.m_showFrame                = picParam->pic_info_fields.bits.show_frame;
    picAV1Params->m_picInfoFlags.m_fields.m_showableFrame            = picParam->pic_info_fields.bits.showable_frame;
    picAV1Params->m_picInfoFlags.m_fields.m_errorResilientMode       = picParam->pic_info_fields.bits.error_resilient_mode;
    picAV1Params->m_picInfoFlags.m_fields.m_disableCdfUpdate         = picParam->pic_info_fields.bits.disable_cdf_update;
    picAV1Params->m_picInfoFlags.m_fields.m_allowScreenContentTools  = picParam->pic_info_fields.bits.allow_screen_content_tools;

    picAV1Params->m_picInfoFlags.m_fields.m_forceIntegerMv           = picParam->pic_info_fields.bits.force_integer_mv;
    picAV1Params->m_picInfoFlags.m_fields.m_allowIntrabc             = picParam->pic_info_fields.bits.allow_intrabc;

    picAV1Params->m_picInfoFlags.m_fields.m_useSuperres              = picParam->pic_info_fields.bits.use_superres;
    picAV1Params->m_picInfoFlags.m_fields.m_allowHighPrecisionMv     = picParam->pic_info_fields.bits.allow_high_precision_mv;
    picAV1Params->m_picInfoFlags.m_fields.m_isMotionModeSwitchable   = picParam->pic_info_fields.bits.is_motion_mode_switchable;
    picAV1Params->m_picInfoFlags.m_fields.m_useRefFrameMvs           = picParam->pic_info_fields.bits.use_ref_frame_mvs;
    picAV1Params->m_picInfoFlags.m_fields.m_disableFrameEndUpdateCdf = picParam->pic_info_fields.bits.disable_frame_end_update_cdf;
    picAV1Params->m_picInfoFlags.m_fields.m_uniformTileSpacingFlag   = picParam->pic_info_fields.bits.uniform_tile_spacing_flag;
    picAV1Params->m_picInfoFlags.m_fields.m_allowWarpedMotion        = picParam->pic_info_fields.bits.allow_warped_motion;
    picAV1Params->m_picInfoFlags.m_fields.m_largeScaleTile           = picParam->pic_info_fields.bits.large_scale_tile;
    picAV1Params->m_picInfoFlags.m_fields.m_reservedPicInfoBits      = 0;

    /***************************************************************************
     Setup reference frames
    ***************************************************************************/
    for (auto i = 0; i < 8; i++)
    {
        PDDI_MEDIA_SURFACE refSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, picParam->ref_frame_map[i]);

        if (picParam->ref_frame_map[i] < mediaCtx->uiNumSurfaces)
        {
            frameIdx = GetRenderTargetID(&m_decodeCtx->RTtbl, refSurface);
            if ((frameIdx == DDI_CODEC_INVALID_FRAME_INDEX) &&
                (picParam->pic_info_fields.bits.frame_type != keyFrame) &&
                (picParam->pic_info_fields.bits.frame_type != intraOnlyFrame))
            {
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
            picAV1Params->m_refFrameMap[i].FrameIdx = ((uint32_t)frameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_AV1) ?
                                                      (CODECHAL_NUM_UNCOMPRESSED_SURFACE_AV1 - 1) : frameIdx;
        }
        else
        {
            if (refSurface != nullptr)
            {
                frameIdx = GetRenderTargetID(&m_decodeCtx->RTtbl, refSurface);
                if (frameIdx != DDI_CODEC_INVALID_FRAME_INDEX)
                {
                    picAV1Params->m_refFrameMap[i].FrameIdx = ((uint32_t)frameIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_AV1) ?
                                                              (CODECHAL_NUM_UNCOMPRESSED_SURFACE_AV1 - 1) : frameIdx;
                }
                else
                {
                    picAV1Params->m_refFrameMap[i].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_AV1 - 1;
                }
            }
            else
            {
                picAV1Params->m_refFrameMap[i].FrameIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_AV1 - 1;
            }
        }
    }

    MOS_SecureMemcpy(picAV1Params->m_refFrameIdx, 7, picParam->ref_frame_idx, 7);

    picAV1Params->m_primaryRefFrame                = picParam->primary_ref_frame;
    picAV1Params->m_outputFrameWidthInTilesMinus1  = picParam->output_frame_width_in_tiles_minus_1;
    picAV1Params->m_outputFrameHeightInTilesMinus1 = picParam->output_frame_height_in_tiles_minus_1;
    picAV1Params->m_reserved32b2                   = 0;

    /****************************************************************************
     Deblocking filter
    ****************************************************************************/
    picAV1Params->m_filterLevel[0] = picParam->filter_level[0];
    picAV1Params->m_filterLevel[1] = picParam->filter_level[1];
    picAV1Params->m_filterLevelU   = picParam->filter_level_u;
    picAV1Params->m_filterLevelV   = picParam->filter_level_v;

    /****************************************************************************
      Loop filter info
    ****************************************************************************/
    picAV1Params->m_loopFilterInfoFlags.m_fields.m_sharpnessLevel      = picParam->loop_filter_info_fields.bits.sharpness_level;
    picAV1Params->m_loopFilterInfoFlags.m_fields.m_modeRefDeltaEnabled = picParam->loop_filter_info_fields.bits.mode_ref_delta_enabled;
    picAV1Params->m_loopFilterInfoFlags.m_fields.m_modeRefDeltaUpdate  = picParam->loop_filter_info_fields.bits.mode_ref_delta_update;
    picAV1Params->m_loopFilterInfoFlags.m_fields.m_reservedField       = 0;

    picAV1Params->m_orderHint                      = picParam->order_hint;
    picAV1Params->m_superresScaleDenominator       = picParam->superres_scale_denominator;
    picAV1Params->m_interpFilter                   = picParam->interp_filter;

    MOS_SecureMemcpy(picAV1Params->m_refDeltas, 8, picParam->ref_deltas, 8);
    MOS_SecureMemcpy(picAV1Params->m_modeDeltas, 2, picParam->mode_deltas, 2);

    /****************************************************************************
     Quantization
    ****************************************************************************/
    picAV1Params->m_baseQindex  = picParam->base_qindex;
    picAV1Params->m_yDcDeltaQ   = picParam->y_dc_delta_q;
    picAV1Params->m_uDcDeltaQ   = picParam->u_dc_delta_q;
    picAV1Params->m_uAcDeltaQ   = picParam->u_ac_delta_q;
    picAV1Params->m_vDcDeltaQ   = picParam->v_dc_delta_q;
    picAV1Params->m_vAcDeltaQ   = picParam->v_ac_delta_q;
    picAV1Params->m_reserved8b2 = 0;

    /****************************************************************************
     quantization_matrix
    ****************************************************************************/
    picAV1Params->m_qMatrixFlags.m_value = picParam->qmatrix_fields.value;

    /****************************************************************************
      Mode control flags
    ****************************************************************************/
    picAV1Params->m_modeControlFlags.m_fields.m_deltaQPresentFlag  = picParam->mode_control_fields.bits.delta_q_present_flag;
    picAV1Params->m_modeControlFlags.m_fields.m_log2DeltaQRes      = picParam->mode_control_fields.bits.log2_delta_q_res;
    picAV1Params->m_modeControlFlags.m_fields.m_deltaLfPresentFlag = picParam->mode_control_fields.bits.delta_lf_present_flag;
    picAV1Params->m_modeControlFlags.m_fields.m_log2DeltaLfRes     = picParam->mode_control_fields.bits.log2_delta_lf_res;
    picAV1Params->m_modeControlFlags.m_fields.m_deltaLfMulti       = picParam->mode_control_fields.bits.delta_lf_multi;
    picAV1Params->m_modeControlFlags.m_fields.m_txMode             = picParam->mode_control_fields.bits.tx_mode;
    picAV1Params->m_modeControlFlags.m_fields.m_referenceMode      = (picParam->mode_control_fields.bits.reference_select == 0)? singleReference : referenceModeSelect;
    picAV1Params->m_modeControlFlags.m_fields.m_reducedTxSetUsed   = picParam->mode_control_fields.bits.reduced_tx_set_used;
    picAV1Params->m_modeControlFlags.m_fields.m_skipModePresent    = picParam->mode_control_fields.bits.skip_mode_present;

    /****************************************************************************
     Segmentation Information
    ****************************************************************************/
    picAV1Params->m_av1SegData.m_enabled        = picParam->seg_info.segment_info_fields.bits.enabled;
    picAV1Params->m_av1SegData.m_updateMap      = picParam->seg_info.segment_info_fields.bits.update_map;
    picAV1Params->m_av1SegData.m_temporalUpdate = picParam->seg_info.segment_info_fields.bits.temporal_update;
    picAV1Params->m_av1SegData.m_updateData     = picParam->seg_info.segment_info_fields.bits.update_data;
    picAV1Params->m_av1SegData.m_reserved4Bits  = 0;

    MOS_SecureMemcpy(picAV1Params->m_av1SegData.m_featureData, av1MaxSegments * segLvlMax * sizeof(int16_t),
                     picParam->seg_info.feature_data,          av1MaxSegments * segLvlMax * sizeof(int16_t));
    MOS_SecureMemcpy(picAV1Params->m_av1SegData.m_featureMask, av1MaxSegments * sizeof(uint8_t),
                     picParam->seg_info.feature_mask,          av1MaxSegments * sizeof(uint8_t));

    bool allLossless = true;
    for (auto seg = 0; seg < av1MaxSegments; seg++)
    {
        uint32_t qIndex = Av1GetQindex(&picAV1Params->m_av1SegData, seg, picAV1Params->m_baseQindex);

        picAV1Params->m_av1SegData.m_losslessFlag[seg] = (qIndex == 0) && (picParam->y_dc_delta_q == 0) &&
            (picParam->u_ac_delta_q == 0) && (picParam->u_dc_delta_q == 0) &&
            (picParam->v_ac_delta_q == 0) && (picParam->v_dc_delta_q == 0);

        // Calc qmlevel for Y/U/V, each segment has the same value
        if (picAV1Params->m_av1SegData.m_losslessFlag[seg] || !picAV1Params->m_qMatrixFlags.m_fields.m_usingQmatrix)
        {
            picAV1Params->m_av1SegData.m_qmLevelY[seg] = av1NumQmLevels - 1;
            picAV1Params->m_av1SegData.m_qmLevelU[seg] = av1NumQmLevels - 1;
            picAV1Params->m_av1SegData.m_qmLevelV[seg] = av1NumQmLevels - 1;
        }
        else
        {
            picAV1Params->m_av1SegData.m_qmLevelY[seg] = picAV1Params->m_qMatrixFlags.m_fields.m_qmY;
            picAV1Params->m_av1SegData.m_qmLevelU[seg] = picAV1Params->m_qMatrixFlags.m_fields.m_qmU;
            picAV1Params->m_av1SegData.m_qmLevelV[seg] = picAV1Params->m_qMatrixFlags.m_fields.m_qmV;
        }

        allLossless &= picAV1Params->m_av1SegData.m_losslessFlag[seg];
    }

    //Frame level lossless flag is set to true when all segments are lossless
    picAV1Params->m_losslessMode        = allLossless;

    picAV1Params->m_tileCountMinus1     = picParam->tile_count_minus_1;
    picAV1Params->m_contextUpdateTileId = picParam->context_update_tile_id;

    /***************************************************************************
      CDEF params
    ***************************************************************************/
    picAV1Params->m_cdefDampingMinus3 = picParam->cdef_damping_minus_3;
    picAV1Params->m_cdefBits          = picParam->cdef_bits;
    MOS_SecureMemcpy(picAV1Params->m_cdefYStrengths,  8, picParam->cdef_y_strengths,  8);
    MOS_SecureMemcpy(picAV1Params->m_cdefUvStrengths, 8, picParam->cdef_uv_strengths, 8);

    /***************************************************************************
     Loop restration flags
    ***************************************************************************/
    picAV1Params->m_loopRestorationFlags.m_fields.m_yframeRestorationType   = picParam->loop_restoration_fields.bits.yframe_restoration_type;
    picAV1Params->m_loopRestorationFlags.m_fields.m_cbframeRestorationType  = picParam->loop_restoration_fields.bits.cbframe_restoration_type;
    picAV1Params->m_loopRestorationFlags.m_fields.m_crframeRestorationType  = picParam->loop_restoration_fields.bits.crframe_restoration_type;
    picAV1Params->m_loopRestorationFlags.m_fields.m_lrUnitShift             = picParam->loop_restoration_fields.bits.lr_unit_shift;
    picAV1Params->m_loopRestorationFlags.m_fields.m_lrUvShift               = picParam->loop_restoration_fields.bits.lr_uv_shift;
    picAV1Params->m_loopRestorationFlags.m_fields.m_reservedField           = 0;

    /**********************************************
     Global motion
    **********************************************/
    for (auto i = 0; i < 7; i++)
    {
        picAV1Params->m_wm[i].m_wmtype  = (CodecAv1TransType)picParam->wm[i].wmtype;
        picAV1Params->m_wm[i].m_invalid = picParam->wm[i].invalid;
        for (auto j = 0; j < 8; j++)
        {
            picAV1Params->m_wm[i].m_wmmat[j] = picParam->wm[i].wmmat[j];
        }
    }

    /***************************************************************************
     Film Grain Information
    ***************************************************************************/
    MOS_SecureMemcpy(&picAV1Params->m_filmGrainParams, sizeof(CodecAv1FilmGrainParams),
                     &picParam->film_grain_info,       sizeof(VAFilmGrainStructAV1));
    if (picAV1Params->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain)
    {
        filmGrainOutSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, picParam->current_display_picture);
    }

    picAV1Params->m_statusReportFeedbackNumber = 0;

    // calculate down scaled width
    if (picAV1Params->m_picInfoFlags.m_fields.m_useSuperres &&
        (picAV1Params->m_superresScaleDenominator != av1ScaleNumerator)) {
        if (picAV1Params->m_superresScaleDenominator == 0) {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        uint32_t dsWidth = ((picParam->frame_width_minus1 + 1 ) *
                            av1ScaleNumerator + picAV1Params->m_superresScaleDenominator / 2) /
            picAV1Params->m_superresScaleDenominator;
        picAV1Params->m_frameWidthMinus1  = dsWidth - 1;
    }
    else {
        picAV1Params->m_frameWidthMinus1  = picParam->frame_width_minus1;
    }

    picAV1Params->m_frameHeightMinus1 = picParam->frame_height_minus1;

    picAV1Params->m_tileCols = picParam->tile_cols;
    picAV1Params->m_tileRows = picParam->tile_rows;

    if (picParam->pic_info_fields.bits.uniform_tile_spacing_flag)
    {
        const uint32_t maxMibSizeLog2   = 5;
        const uint32_t minMibSizeLog2   = 4;
        const uint32_t miSizeLog2       = 2;
        int32_t mibSizeLog2 = picParam->seq_info_fields.fields.use_128x128_superblock ? maxMibSizeLog2 : minMibSizeLog2;
        int32_t miCols = MOS_ALIGN_CEIL(MOS_ALIGN_CEIL(picAV1Params->m_frameWidthMinus1 + 1, 8) >> miSizeLog2, 1 << mibSizeLog2);
        int32_t miRows = MOS_ALIGN_CEIL(MOS_ALIGN_CEIL(picAV1Params->m_frameHeightMinus1 + 1, 8) >> miSizeLog2, 1 << mibSizeLog2);
        int32_t sbCols = miCols >> mibSizeLog2;
        int32_t sbRows = miRows >> mibSizeLog2;

        for (auto i = 0; i < picParam->tile_cols - 1; i++)
        {
            uint32_t tileColsLog2 = CalcAv1TileLog2(1, picParam->tile_cols);
            uint32_t sizeSb = MOS_ALIGN_CEIL(sbCols, 1 << tileColsLog2);
            sizeSb >>= tileColsLog2;
            picParam->width_in_sbs_minus_1[i] = sizeSb - 1;
        }

        for (auto i = 0; i < picParam->tile_rows - 1; i++)
        {
            uint32_t tileRowsLog2 = CalcAv1TileLog2(1, picParam->tile_rows);
            uint32_t sizeSb = MOS_ALIGN_CEIL(sbRows, 1 << tileRowsLog2);
            sizeSb >>= tileRowsLog2;
            picParam->height_in_sbs_minus_1[i] = sizeSb - 1;
        }
    }

    MOS_SecureMemcpy(picAV1Params->m_widthInSbsMinus1, 63 * sizeof(uint16_t),
                     picParam->width_in_sbs_minus_1,   63 * sizeof(uint16_t));
    MOS_SecureMemcpy(picAV1Params->m_heightInSbsMinus1, 63 * sizeof(uint16_t),
                     picParam->height_in_sbs_minus_1,   63 * sizeof(uint16_t));

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    // Picture Info
    uint32_t subSamplingSum = picAV1Params->m_seqInfoFlags.m_fields.m_subsamplingX + picAV1Params->m_seqInfoFlags.m_fields.m_subsamplingY;
    DECODE_EVENTDATA_INFO_PICTUREVA eventData = {0};
    eventData.CodecFormat                     = m_decodeCtx->wMode;
    eventData.FrameType                       = picAV1Params->m_picInfoFlags.m_fields.m_frameType == 0 ? I_TYPE : MIXED_TYPE;
    eventData.PicStruct                       = FRAME_PICTURE;
    eventData.Width                           = picAV1Params->m_frameWidthMinus1 + 1;
    eventData.Height                          = picAV1Params->m_frameHeightMinus1 + 1;
    eventData.Bitdepth                        = picAV1Params->m_bitDepthIdx;
    eventData.ChromaFormat                    = (subSamplingSum == 2) ? 1 : (subSamplingSum == 1 ? 2 : 3);  // 1-4:2:0; 2-4:2:2; 3-4:4:4
    eventData.EnabledSCC                      = picAV1Params->m_picInfoFlags.m_fields.m_allowScreenContentTools;
    eventData.EnabledSegment                  = picAV1Params->m_av1SegData.m_enabled;
    eventData.EnabledFilmGrain                = picAV1Params->m_seqInfoFlags.m_fields.m_filmGrainParamsPresent;
    MOS_TraceEvent(EVENT_DECODE_INFO_PICTUREVA, EVENT_TYPE_INFO, &eventData, sizeof(eventData), NULL, 0);
#endif

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeAv1::SetDecodeParams()
{
    DDI_CODEC_FUNC_ENTER;

     DDI_CHK_RET(DdiDecodeBase::SetDecodeParams(),"SetDecodeParams failed!");

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
            procParams->m_inputSurfaceRegion.m_width = procParams->m_inputSurface->dwWidth;
            procParams->m_inputSurfaceRegion.m_height = procParams->m_inputSurface->dwHeight;
        }
    }
#endif
    CodecAv1PicParams *Av1PicParams = static_cast<CodecAv1PicParams *>(m_decodeCtx->DecodeParams.m_picParams);
    bool bFilmGrainEnabled = Av1PicParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain;
    if (bFilmGrainEnabled)
    {
        FilmGrainProcParams &filmGrainProcParams = m_decodeCtx->DecodeParams.m_filmGrainProcParams;
        MOS_ZeroMemory(&filmGrainProcParams, sizeof(FilmGrainProcParams));
        filmGrainProcParams.m_inputSurface = (&m_decodeCtx->DecodeParams)->m_destSurface;
        MOS_FORMAT expectedFormat = GetFormat();
        outputSurface.Format      = expectedFormat;
        MediaLibvaCommonNext::MediaSurfaceToMosResource(filmGrainOutSurface, &(outputSurface.OsResource));
        filmGrainProcParams.m_outputSurface = &outputSurface;
    }

    // anchor frame list insertion
    if (Av1PicParams->m_anchorFrameNum > 0 && Av1PicParams->m_anchorFrameNum <= MAX_ANCHOR_FRAME_NUM_AV1)
    {
        MOS_FORMAT expectedFormat = GetFormat();
        for (auto i = 0; i < Av1PicParams->m_anchorFrameNum; i++)
        {
            PDDI_MEDIA_SURFACE anchorFrame = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(m_decodeCtx->pMediaCtx, anchorFrameListVA[i]);
            anchorFrameList[i].Format = expectedFormat;
            MediaLibvaCommonNext::MediaSurfaceToMosResource(anchorFrame, &(anchorFrameList[i].OsResource));
        }
        Av1PicParams->m_anchorFrameList = anchorFrameList;
    }

    return VA_STATUS_SUCCESS;
}

int DdiDecodeAv1::Av1Clamp(int value, int low, int high)
{
    DDI_CODEC_FUNC_ENTER;

    return value < low ? low : (value > high ? high : value);
}

uint32_t DdiDecodeAv1::Av1GetQindex(
    CodecAv1SegmentsParams *segInfo,
    uint32_t               segment_id,
    uint8_t                base_qindex)
{
    DDI_CODEC_FUNC_ENTER;

    if ((segInfo->m_enabled) && (segInfo->m_featureMask[segment_id] & (1 << segLvlAltQ)))
    {
        const int data = segInfo->m_featureData[segment_id][segLvlAltQ];
        return Av1Clamp(base_qindex + data, 0, av1MaxQindex);  // Delta value
    }
    else
    {
        return base_qindex;
    }
}

VAStatus DdiDecodeAv1::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    DDI_CODEC_FUNC_ENTER;

    VAStatus           va       = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);

    void *data = nullptr;
    for (int32_t i = 0; i < numBuffers; i++)
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

            MediaLibvaCommonNext::MediaBufferToMosResource(m_decodeCtx->BufMgr.pBitStreamBuffObject[index],
                                              &m_decodeCtx->BufMgr.resBitstreamBuffer);
            m_decodeCtx->DecodeParams.m_dataSize += dataSize;

            break;
        }
        case VASliceParameterBufferType:
        {
            if (buf->uiNumElements == 0)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            VASliceParameterBufferAV1 *slcInfoAV1 = (VASliceParameterBufferAV1 *)data;

            DDI_CHK_RET(ParseTileParams(mediaCtx, slcInfoAV1, buf->uiNumElements), "ParseTileParams failed!");
            m_decodeCtx->DecodeParams.m_numSlices += buf->uiNumElements;
            m_groupIndex++;
            break;
        }
        case VAPictureParameterBufferType:
        {
            VADecPictureParameterBufferAV1 *picParam = (VADecPictureParameterBufferAV1 *)data;
            DDI_CHK_RET(ParsePicParams(mediaCtx, picParam), "ParsePicParams failed!");
            break;
        }
        case VAProcPipelineParameterBufferType:
        {
            DDI_CHK_RET(ParseProcessingBuffer(mediaCtx, data),"ParseProcessingBuffer failed!");
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

VAStatus DdiDecodeAv1::InitResourceBuffer()
{
    DDI_CODEC_FUNC_ENTER;

    VAStatus                  vaStatus = VA_STATUS_SUCCESS;
    DDI_CODEC_COM_BUFFER_MGR *bufMgr   = &(m_decodeCtx->BufMgr);

    bufMgr->pSliceData = nullptr;

    bufMgr->ui64BitstreamOrder = 0;
    bufMgr->dwMaxBsSize = m_width * m_height * 3 / 2; // need consider 2byte case
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
        bufMgr->pBitStreamBuffObject[i]->iSize = bufMgr->dwMaxBsSize;
        bufMgr->pBitStreamBuffObject[i]->uiType = VASliceDataBufferType;
        bufMgr->pBitStreamBuffObject[i]->format = Media_Format_Buffer;
        bufMgr->pBitStreamBuffObject[i]->uiOffset = 0;
        bufMgr->pBitStreamBuffObject[i]->bo = nullptr;
        bufMgr->pBitStreamBase[i] = nullptr;
    }

    bufMgr->m_maxNumSliceData = av1MaxTileNum;
    bufMgr->pSliceData = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)MOS_AllocAndZeroMemory(sizeof(bufMgr->pSliceData[0]) * bufMgr->m_maxNumSliceData);

    if (bufMgr->pSliceData == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        FreeResourceBuffer();
        return vaStatus;
    }

    bufMgr->dwNumSliceData = 0;
    bufMgr->dwNumSliceControl = 0;
    bufMgr->pCodecParamReserved = (DDI_DECODE_BUFFER_PARAM_AV1 *)MOS_AllocAndZeroMemory(sizeof(DDI_DECODE_BUFFER_PARAM_AV1));

    if (bufMgr->pCodecParamReserved == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        FreeResourceBuffer();
        return vaStatus;
    }

    bufMgr->pCodecSlcParamReserved = (VASliceParameterBufferAV1 *)MOS_AllocAndZeroMemory(sizeof(VASliceParameterBufferAV1) * av1MaxTileNum);
    if (bufMgr->pCodecSlcParamReserved == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        FreeResourceBuffer();
        return vaStatus;
    }

    { // need bracket to avoid compile error by jump
        DDI_DECODE_BUFFER_PARAM_AV1 *codec_Param_AV1 = (DDI_DECODE_BUFFER_PARAM_AV1 *)bufMgr->pCodecParamReserved;
        codec_Param_AV1->pVASliceParameterBufferAV1 = (VASliceParameterBufferAV1 *)bufMgr->pCodecSlcParamReserved;
    }

    return VA_STATUS_SUCCESS;
}

void DdiDecodeAv1::FreeResourceBuffer()
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

    if (bufMgr->pCodecParamReserved)
    {
        DDI_DECODE_BUFFER_PARAM_AV1 *codec_Param_AV1 =
            static_cast<DDI_DECODE_BUFFER_PARAM_AV1 *>(bufMgr->pCodecParamReserved);
         if (codec_Param_AV1->pVASliceParameterBufferAV1)
         {
            MOS_FreeMemory(codec_Param_AV1->pVASliceParameterBufferAV1);
            codec_Param_AV1->pVASliceParameterBufferAV1 = nullptr;
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

uint8_t* DdiDecodeAv1::GetPicParamBuf(
    DDI_CODEC_COM_BUFFER_MGR *bufMgr)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_DECODE_BUFFER_PARAM_AV1* codec_Param_AV1 = static_cast<DDI_DECODE_BUFFER_PARAM_AV1 *>(bufMgr->pCodecParamReserved);
    return (uint8_t*)(&(codec_Param_AV1->PicParamAV1));
}

VAStatus DdiDecodeAv1::AllocSliceControlBuffer(
    DDI_MEDIA_BUFFER *buf)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_COM_BUFFER_MGR *bufMgr;

    bufMgr = &(m_decodeCtx->BufMgr);

    DDI_DECODE_BUFFER_PARAM_AV1 *codec_Param_AV1 = (DDI_DECODE_BUFFER_PARAM_AV1 *)bufMgr->pCodecParamReserved;
    codec_Param_AV1->pVASliceParameterBufferAV1 = (VASliceParameterBufferAV1 *)bufMgr->pCodecSlcParamReserved;
    if (codec_Param_AV1->pVASliceParameterBufferAV1 == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    if (buf->uiNumElements > av1MaxTileNum)
    {
        DDI_CODEC_ASSERTMESSAGE("buf->uiNumElements = %d : exceeds av1MaxTileNum = %d",
                          buf->uiNumElements, av1MaxTileNum);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    buf->pData = (uint8_t*)codec_Param_AV1->pVASliceParameterBufferAV1;
    buf->uiOffset = bufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferAV1);

    bufMgr->dwNumSliceControl += buf->uiNumElements;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeAv1::CodecHalInit(
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
    standardInfo.Mode = (CODECHAL_MODE)m_decodeCtx->wMode;

    m_codechalSettings->codecFunction        = codecFunction;
    m_codechalSettings->width                = m_width;
    m_codechalSettings->height               = m_height;
    m_codechalSettings->intelEntrypointInUse = false;

    // VAProfileAV1Profile0 supports both 420 8bit and 420 10bit
    m_codechalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_8_BITS;

    m_codechalSettings->shortFormatInUse = m_decodeCtx->bShortFormatInUse;

    m_codechalSettings->mode = CODECHAL_DECODE_MODE_AV1VLD;
    m_codechalSettings->standard = CODECHAL_AV1;
    m_codechalSettings->chromaFormat = HCP_CHROMA_FORMAT_YUV420;

    m_decodeCtx->DecodeParams.m_picParams = MOS_AllocAndZeroMemory(sizeof(CodecAv1PicParams));
    if (m_decodeCtx->DecodeParams.m_picParams == nullptr)
    {
        FreeResource();
        return vaStatus;
    }

    m_decodeCtx->DecodeParams.m_sliceParams = MOS_AllocAndZeroMemory(sizeof(CodecAv1TileParams) * av1MaxTileNum);
    if (m_decodeCtx->DecodeParams.m_sliceParams == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        FreeResource();
        return vaStatus;
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
            FreeResource();
            return vaStatus;
        }

        m_decodeCtx->DecodeParams.m_procParams = procParams;
        procParams->m_outputSurface = (PMOS_SURFACE)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE));
        if (procParams->m_outputSurface == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            FreeResource();
            return vaStatus;
        }
    }
#endif
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
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        FreeResource();
        return vaStatus;
    }

    return vaStatus;
}

void DdiDecodeAv1::FreeResource()
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
    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_sliceParams);
    m_decodeCtx->DecodeParams.m_sliceParams = nullptr;

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_decodeCtx->DecodeParams.m_procParams)
    {
        auto procParams = (DecodeProcessingParams *)m_decodeCtx->DecodeParams.m_procParams;
        MOS_FreeMemory(procParams->m_outputSurface);

        MOS_FreeMemory(m_decodeCtx->DecodeParams.m_procParams);
    }
#endif
    return;
}

VAStatus DdiDecodeAv1::InitDecodeParams(
    VADriverContextP ctx,
    VAContextID      context)
{
    DDI_CODEC_FUNC_ENTER;

    // skip the mediaCtx check as it is checked in caller
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_RET(DecodeCombineBitstream(mediaCtx), "DecodeCombineBitstream failed!");
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_decodeCtx->BufMgr);
    bufMgr->dwNumSliceData    = 0;
    bufMgr->dwNumSliceControl = 0;
    memset(&outputSurface, 0, sizeof(MOS_SURFACE));
    outputSurface.dwOffset    = 0;

    for (auto i = 0; i < MAX_ANCHOR_FRAME_NUM_AV1; i++)
    {
        memset(&anchorFrameList[i], 0, sizeof(MOS_SURFACE));
        anchorFrameList[i].dwOffset = 0;
    }

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_decodeCtx->RTtbl);

    if ((rtTbl == nullptr) || (rtTbl->pCurrentRT == nullptr))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    return VA_STATUS_SUCCESS;
}

MOS_FORMAT DdiDecodeAv1::GetFormat()
{
    DDI_CODEC_FUNC_ENTER;

    MOS_FORMAT Format = Format_NV12;
    CodechalDecodeParams *decodeParams = &m_decodeCtx->DecodeParams;

    CodecAv1PicParams *picParams = (CodecAv1PicParams *)decodeParams->m_picParams;
    if (picParams->m_bitDepthIdx > 0)
    {
        Format = Format_P010;
        if (picParams->m_bitDepthIdx > 2)
        {
            Format = Format_P016;
        }
        if ((picParams->m_seqInfoFlags.m_fields.m_subsamplingX == 1) &&
            (picParams->m_seqInfoFlags.m_fields.m_subsamplingY == 0))
        {
            Format = Format_Y210;
        }
        else if ((picParams->m_seqInfoFlags.m_fields.m_subsamplingX == 0) &&
            (picParams->m_seqInfoFlags.m_fields.m_subsamplingY == 0))
        {
            if (picParams->m_bitDepthIdx == 2)
            {
                Format = Format_Y410;
            }
            else if (picParams->m_bitDepthIdx > 2)
            {
                Format = Format_Y416;
            }
        }
    }
    return Format;
}

void DdiDecodeAv1::DestroyContext(
    VADriverContextP ctx)
{
    DDI_CODEC_FUNC_ENTER;

    FreeResourceBuffer();
    // explicitly call the base function to do the further clean-up
    DdiDecodeBase::DestroyContext(ctx);

    return;
}


void DdiDecodeAv1::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    DDI_CODEC_FUNC_ENTER;

    // call the function in base class to initialize it.
    DdiDecodeBase::ContextInit(picWidth, picHeight);

    m_decodeCtx->wMode = CODECHAL_DECODE_MODE_AV1VLD;

    return;
}

} // namespace decode
