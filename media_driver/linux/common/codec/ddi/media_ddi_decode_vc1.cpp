/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file      media_ddi_decode_vc1.cpp 
//! \brief     libva(and its extension) decoder implementation 
//!
#include "media_libva_decoder.h"
#include "media_libva_util.h"

#include "media_ddi_decode_vc1.h"
#include "mos_solo_generic.h"
#include "codechal_memdecomp.h"
#include "media_ddi_decode_const.h"
#include "media_ddi_factory.h"

static const int32_t FractionToScaleFactor[21] = {
    128, 85,  170, 64,  192,
    51,  102, 153, 204, 43,
    215, 37,  74,  111, 148,
    185, 222, 32,  96,  160,
    224,
};

void DdiDecodeVC1::CalculateQuantParams(
    VAPictureParameterBufferVC1 *picParam,
    uint32_t                    *outAltPquantConfig,
    uint32_t                    *outAltPquantEdgeMask)
{
    uint32_t dquant    = picParam->pic_quantizer_fields.bits.dquant;
    uint32_t dqFrame   = picParam->pic_quantizer_fields.bits.dq_frame;
    uint32_t dqProfile = picParam->pic_quantizer_fields.bits.dq_profile;
    uint32_t dqDbedge  = picParam->pic_quantizer_fields.bits.dq_db_edge;
    uint32_t dqSbedge  = picParam->pic_quantizer_fields.bits.dq_sb_edge;
    uint32_t dqBilevel = picParam->pic_quantizer_fields.bits.dq_binary_level;

    uint32_t altPquantConfig   = 0;
    uint32_t altPquantEdgeMask = 0;

    if (dquant == 0)
    {
        altPquantConfig   = 0;
        altPquantEdgeMask = 0;
    }
    else if (dquant == 1)
    {
        if (dqFrame == 0)
        {
            altPquantConfig   = 0;
            altPquantEdgeMask = 0;
        }
        else if (dqFrame == 1)
        {
            altPquantConfig   = 1;
            altPquantEdgeMask = 0;
            switch (dqProfile)
            {
            case 0:  //all edge with altpquant
                altPquantEdgeMask = 0xf;
                break;

            case 1:  //double edge with altpquant
                if (dqDbedge == 3)
                {
                    altPquantEdgeMask = 0x9;
                }
                else
                {
                    altPquantEdgeMask = (0x3 << dqDbedge);
                }
                break;

            case 2:  //single edge with altpquant
                altPquantEdgeMask = (0x1 << dqSbedge);
                break;

            case 3:  //all mbs
                if (dqBilevel == 0)
                {
                    altPquantConfig   = 2;
                    altPquantEdgeMask = 0;
                }
                else
                {
                    altPquantConfig   = 3;
                    altPquantEdgeMask = 0;
                }
                break;

            default:
                break;
            }
        }
    }
    else
    {
        altPquantConfig   = 1;
        altPquantEdgeMask = 0xf;
    }

    *outAltPquantConfig   = altPquantConfig;
    *outAltPquantEdgeMask = altPquantEdgeMask;
}

static inline bool IsBIField(
    bool                isFirstField,
    int16_t             picType)
{
    bool isBI = false;

    if (picType == vc1BBIField)
    {
        isBI = (!isFirstField);
    }
    else if (picType == vc1BIBField)
    {
        isBI = isFirstField;
    }
    else if (picType == vc1BIBIField)
    {
        isBI = true;
    }

    return isBI;
}

static inline bool IsBField(
    bool                isFirstField,
    int16_t             picType)
{
    bool isBottom = false;

    if (picType == vc1BBIField)
    {
        isBottom = isFirstField;
    }
    else if (picType == vc1BIBField)
    {
        isBottom = (!isFirstField);
    }
    else if (picType == vc1BBField)
    {
        isBottom = true;
    }

    return isBottom;
}

static inline bool IsIField(
    bool                isFirstField,
    int16_t             picType)
{
    bool isIF = false;

    if (picType == vc1IPField)
    {
        isIF = isFirstField;
    }
    else if (picType == vc1PIField)
    {
        isIF = !isFirstField;
    }
    else if (picType == vc1IIField)
    {
        isIF = true;
    }

    return isIF;
}

VAStatus DdiDecodeVC1::ParsePicParams(
    DDI_MEDIA_CONTEXT             *mediaCtx,
    VAPictureParameterBufferVC1   *picParam)
{
    PCODEC_VC1_PIC_PARAMS codecPicParam = (PCODEC_VC1_PIC_PARAMS)(m_ddiDecodeCtx->DecodeParams.m_picParams);

    if ((codecPicParam == nullptr) ||
        (picParam == nullptr))
    {
        DDI_ASSERTMESSAGE("nullptr pointer in Parsing VC1 Picture parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    uint32_t scaleFactor = 0;
    // See spec, table 40 && Figure 70
    if (picParam->b_picture_fraction < 21)
    {
        scaleFactor = FractionToScaleFactor[picParam->b_picture_fraction];
    }
    else
    {
        // Use one value for it if the picParam->b_picture_fraction is out of range.
        // Since in some cases, although driver gets the invalid bFraction, HW still can handle this.
        scaleFactor = FractionToScaleFactor[0];
    }

    uint32_t fwdRefDist = picParam->reference_fields.bits.reference_distance;
    int32_t  bwdRefDist = 0;

    if (picParam->inloop_decoded_picture != DDI_CODEC_INVALID_FRAME_INDEX)
    {
        codecPicParam->DeblockedPicIdx = GetRenderTargetID(&(m_ddiDecodeCtx->RTtbl), m_ddiDecodeCtx->RTtbl.pCurrentRT);
        DDI_CHK_RET(RegisterRTSurfaces(&m_ddiDecodeCtx->RTtbl, DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->inloop_decoded_picture)), "RegisterRTSurfaces failed!");

        codecPicParam->CurrPic.FrameIdx = (uint16_t)GetRenderTargetID(&(m_ddiDecodeCtx->RTtbl),
            DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->inloop_decoded_picture));
        m_deblockPicIdx                 = codecPicParam->DeblockedPicIdx;
        m_currPicIdx                    = codecPicParam->CurrPic.FrameIdx;
    }
    else
    {
        codecPicParam->CurrPic.FrameIdx = GetRenderTargetID(&(m_ddiDecodeCtx->RTtbl), m_ddiDecodeCtx->RTtbl.pCurrentRT);
        codecPicParam->DeblockedPicIdx  = codecPicParam->CurrPic.FrameIdx;
        m_deblockPicIdx                 = DDI_CODEC_INVALID_FRAME_INDEX;
        m_currPicIdx                    = codecPicParam->CurrPic.FrameIdx;
    }

    if (picParam->forward_reference_picture == DDI_CODEC_INVALID_FRAME_INDEX)
    {
        codecPicParam->ForwardRefIdx = codecPicParam->CurrPic.FrameIdx;
    }
    else
    {
        if (UpdateRegisteredRTSurfaceFlag(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->forward_reference_picture)) != VA_STATUS_SUCCESS)
        {
            DDI_CHK_RET(RegisterRTSurfaces(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->forward_reference_picture)), "RegisterRTSurfaces failed!");
        }
        codecPicParam->ForwardRefIdx = (uint16_t)GetRenderTargetID(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->forward_reference_picture));
    }

    if (picParam->backward_reference_picture == DDI_CODEC_INVALID_FRAME_INDEX)
    {
        codecPicParam->BackwardRefIdx = codecPicParam->CurrPic.FrameIdx;
    }
    else
    {
        if (UpdateRegisteredRTSurfaceFlag(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->backward_reference_picture)) != VA_STATUS_SUCCESS)
        {
            DDI_CHK_RET(RegisterRTSurfaces(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->backward_reference_picture)), "RegisterRTSurfaces failed!");
        }
        codecPicParam->BackwardRefIdx = (uint16_t)GetRenderTargetID(&(m_ddiDecodeCtx->RTtbl), DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, picParam->backward_reference_picture));
    }

    //add protection checking to prevent ref pic index larger than DPB size
    if (codecPicParam->ForwardRefIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1)
    {
        codecPicParam->ForwardRefIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1 - 1;
    }
    if (codecPicParam->BackwardRefIdx >= CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1)
    {
        codecPicParam->BackwardRefIdx = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1 - 1;
    }

    if (picParam->picture_fields.bits.frame_coding_mode == 0)
    {
        codecPicParam->CurrPic.PicFlags = PICTURE_FRAME;
    }
    else if (picParam->picture_fields.bits.frame_coding_mode == 1)
    {
        codecPicParam->CurrPic.PicFlags = PICTURE_INTERLACED_FRAME;
    }
    else
    {
        codecPicParam->CurrPic.PicFlags = (picParam->picture_fields.bits.is_first_field ^ picParam->picture_fields.bits.top_field_first) ? PICTURE_BOTTOM_FIELD : PICTURE_TOP_FIELD;
    }

    if (picParam->picture_fields.bits.frame_coding_mode < 2)  //frame structure
    {
        codecPicParam->picture_fields.frame_coding_mode = picParam->picture_fields.bits.frame_coding_mode;
        codecPicParam->luma_scale                       = (uint16_t)picParam->luma_scale;
        codecPicParam->luma_shift                       = (uint16_t)picParam->luma_shift;
    }
    else
    {
        if (picParam->picture_fields.bits.top_field_first)
        {
            codecPicParam->picture_fields.frame_coding_mode = 2;
        }
        else
        {
            codecPicParam->picture_fields.frame_coding_mode = 3;
        }
        if (picParam->picture_fields.bits.picture_type >= vc1BBField)  // B interlaced frames
        {
            fwdRefDist = (scaleFactor * picParam->reference_fields.bits.reference_distance) >> 8;
            bwdRefDist = picParam->reference_fields.bits.reference_distance - fwdRefDist - 1;
            if (bwdRefDist < 0)
            {
                bwdRefDist = 0;
            }
        }
        codecPicParam->luma_scale = (uint16_t)(picParam->luma_scale2 + (picParam->luma_scale << 8));
        codecPicParam->luma_shift = (uint16_t)(picParam->luma_shift2 + (picParam->luma_shift << 8));
    }

    codecPicParam->sequence_fields.AdvancedProfileFlag = (picParam->sequence_fields.bits.profile == 3) ? 1 : 0;
    codecPicParam->picture_fields.picture_type         = picParam->picture_fields.bits.picture_type;

    if (codecPicParam->sequence_fields.AdvancedProfileFlag)
    {
        codecPicParam->sequence_fields.overlap = picParam->sequence_fields.bits.overlap;
    }
    else
    {
        // see 7.1.1.14 in VC spec
        if ((picParam->b_picture_fraction == 0x7F) && (picParam->picture_fields.bits.picture_type == vc1IFrame))
        {
            codecPicParam->picture_fields.picture_type = vc1BIFrame;
        }

        codecPicParam->sequence_fields.overlap = picParam->sequence_fields.bits.overlap && (picParam->pic_quantizer_fields.bits.pic_quantizer_scale >= 9);
    }

    codecPicParam->coded_width                           = picParam->coded_width;
    codecPicParam->coded_height                          = picParam->coded_height;
    codecPicParam->sequence_fields.syncmarker            = picParam->sequence_fields.bits.syncmarker;
    codecPicParam->sequence_fields.pulldown              = picParam->sequence_fields.bits.pulldown;
    codecPicParam->sequence_fields.interlace             = picParam->sequence_fields.bits.interlace;
    codecPicParam->sequence_fields.tfcntrflag            = picParam->sequence_fields.bits.tfcntrflag;
    codecPicParam->sequence_fields.psf                   = picParam->sequence_fields.bits.psf;
    codecPicParam->sequence_fields.multires              = picParam->sequence_fields.bits.multires;
    codecPicParam->sequence_fields.rangered              = picParam->sequence_fields.bits.rangered;
    codecPicParam->sequence_fields.max_b_frames          = picParam->sequence_fields.bits.max_b_frames;
    codecPicParam->entrypoint_fields.loopfilter          = picParam->entrypoint_fields.bits.loopfilter;
    codecPicParam->entrypoint_fields.broken_link         = picParam->entrypoint_fields.bits.broken_link;
    codecPicParam->entrypoint_fields.closed_entry        = picParam->entrypoint_fields.bits.closed_entry;
    codecPicParam->entrypoint_fields.panscan_flag        = picParam->entrypoint_fields.bits.panscan_flag;
    codecPicParam->picture_fields.is_first_field         = picParam->picture_fields.bits.is_first_field;
    codecPicParam->picture_fields.top_field_first        = picParam->picture_fields.bits.top_field_first;
    codecPicParam->picture_fields.intensity_compensation = picParam->picture_fields.bits.intensity_compensation;

    if (picParam->mv_fields.bits.mv_mode == VAMvModeIntensityCompensation)
    {
        codecPicParam->mv_fields.UnifiedMvMode = (picParam->mv_fields.bits.mv_mode2 + 1) & 0x3;
        codecPicParam->picture_fields.intensity_compensation = 1;
    }
    else
    {
        codecPicParam->mv_fields.UnifiedMvMode = (picParam->mv_fields.bits.mv_mode + 1) & 0x3;
    }

    if (picParam->mv_fields.bits.mv_mode == VAMvMode1MvHalfPelBilinear ||
        (picParam->mv_fields.bits.mv_mode == VAMvModeIntensityCompensation && picParam->mv_fields.bits.mv_mode2 == VAMvMode1MvHalfPelBilinear))
    {
        codecPicParam->mv_fields.MvMode = picParam->fast_uvmc_flag ? 9 : 8;
    }
    else
    {
        codecPicParam->mv_fields.MvMode = picParam->fast_uvmc_flag ? 1 : 0;
    }

    uint32_t altPquantConfig   = 0;
    uint32_t altPquantEdgeMask = 0;
    CalculateQuantParams(picParam, &altPquantConfig, &altPquantEdgeMask);

    codecPicParam->pic_quantizer_fields.quantizer                 = picParam->pic_quantizer_fields.bits.quantizer;
    codecPicParam->pic_quantizer_fields.pic_quantizer_scale       = picParam->pic_quantizer_fields.bits.pic_quantizer_scale;
    codecPicParam->pic_quantizer_fields.alt_pic_quantizer         = picParam->pic_quantizer_fields.bits.alt_pic_quantizer;
    codecPicParam->pic_quantizer_fields.pic_quantizer_type        = picParam->pic_quantizer_fields.bits.pic_quantizer_type;
    codecPicParam->pic_quantizer_fields.half_qp                   = picParam->pic_quantizer_fields.bits.half_qp;
    codecPicParam->pic_quantizer_fields.AltPQuantConfig           = altPquantConfig;
    codecPicParam->pic_quantizer_fields.AltPQuantEdgeMask         = altPquantEdgeMask;
    codecPicParam->pic_quantizer_fields.dquant                    = picParam->pic_quantizer_fields.bits.dquant;
    codecPicParam->mv_fields.extended_mv_flag                     = picParam->mv_fields.bits.extended_mv_flag;
    codecPicParam->mv_fields.extended_dmv_flag                    = picParam->mv_fields.bits.extended_dmv_flag;
    codecPicParam->mv_fields.extended_mv_range                    = picParam->mv_fields.bits.extended_mv_range;
    codecPicParam->mv_fields.extended_dmv_range                   = picParam->mv_fields.bits.extended_dmv_range;
    codecPicParam->mv_fields.four_mv_switch                       = picParam->mv_fields.bits.four_mv_switch;
    codecPicParam->mv_fields.two_mv_block_pattern_table           = picParam->mv_fields.bits.two_mv_block_pattern_table;
    codecPicParam->mv_fields.four_mv_block_pattern_table          = picParam->mv_fields.bits.four_mv_block_pattern_table;
    codecPicParam->mv_fields.mv_table                             = picParam->mv_fields.bits.mv_table;
    codecPicParam->reference_fields.reference_distance_flag       = picParam->reference_fields.bits.reference_distance_flag;
    codecPicParam->reference_fields.reference_distance            = fwdRefDist;  //picParam->reference_fields.bits.reference_distance;
    codecPicParam->reference_fields.BwdReferenceDistance          = bwdRefDist;  //picParam->reference_fields.bits.reference_distance;
    codecPicParam->reference_fields.num_reference_pictures        = picParam->reference_fields.bits.num_reference_pictures;
    codecPicParam->reference_fields.reference_field_pic_indicator = picParam->reference_fields.bits.reference_field_pic_indicator;

    codecPicParam->raw_coding.value = 0xFE;
    if (picParam->picture_fields.bits.frame_coding_mode < 2)  //frame structure
    {
        if ((picParam->picture_fields.bits.picture_type == vc1IFrame) ||
            (picParam->picture_fields.bits.picture_type == vc1BIFrame))
        {
            if (picParam->picture_fields.bits.frame_coding_mode == 1)  //interlaced frame
            {
                codecPicParam->raw_coding.field_tx = !picParam->bitplane_present.flags.bp_field_tx;
            }
            codecPicParam->raw_coding.ac_pred   = !picParam->bitplane_present.flags.bp_ac_pred;
            codecPicParam->raw_coding.overflags = !picParam->bitplane_present.flags.bp_overflags;
        }
        else if (picParam->picture_fields.bits.picture_type == vc1PFrame)
        {
            codecPicParam->raw_coding.skip_mb = !picParam->bitplane_present.flags.bp_skip_mb;
            if (picParam->picture_fields.bits.frame_coding_mode == 0)  //progressive frame
            {
                codecPicParam->raw_coding.mv_type_mb = !picParam->bitplane_present.flags.bp_mv_type_mb;
            }
        }
        else
        {
            codecPicParam->raw_coding.direct_mb = !picParam->bitplane_present.flags.bp_direct_mb;
            codecPicParam->raw_coding.skip_mb   = !picParam->bitplane_present.flags.bp_skip_mb;
        }
    }
    else
    {
        if ((IsIField(picParam->picture_fields.bits.is_first_field, picParam->picture_fields.bits.picture_type)) ||
            (IsBIField(picParam->picture_fields.bits.is_first_field, picParam->picture_fields.bits.picture_type)))
        {
            codecPicParam->raw_coding.ac_pred   = !picParam->bitplane_present.flags.bp_ac_pred;
            codecPicParam->raw_coding.overflags = !picParam->bitplane_present.flags.bp_overflags;
        }
        else if (IsBField(picParam->picture_fields.bits.is_first_field, picParam->picture_fields.bits.picture_type))
        {
            codecPicParam->raw_coding.forward_mb = !picParam->bitplane_present.flags.bp_forward_mb;
        }
    }
    uint32_t rawCoding                         = codecPicParam->raw_coding.value & 0xFE;
    codecPicParam->raw_coding.bitplane_present = (rawCoding == 0xFE) ? 0 : 1;

    codecPicParam->transform_fields.intra_transform_dc_table      = picParam->transform_fields.bits.intra_transform_dc_table;
    codecPicParam->transform_fields.transform_ac_codingset_idx1   = picParam->transform_fields.bits.transform_ac_codingset_idx1;
    codecPicParam->transform_fields.transform_ac_codingset_idx2   = picParam->transform_fields.bits.transform_ac_codingset_idx2;
    codecPicParam->transform_fields.variable_sized_transform_flag = picParam->transform_fields.bits.variable_sized_transform_flag;
    codecPicParam->transform_fields.mb_level_transform_type_flag  = picParam->transform_fields.bits.mb_level_transform_type_flag;
    codecPicParam->transform_fields.frame_level_transform_type    = picParam->transform_fields.bits.frame_level_transform_type;

    codecPicParam->conditional_overlap_flag = picParam->conditional_overlap_flag ? (picParam->conditional_overlap_flag + 1) : 0;
    codecPicParam->fast_uvmc_flag           = picParam->fast_uvmc_flag;
    codecPicParam->cbp_table                = picParam->cbp_table;
    codecPicParam->mb_mode_table            = picParam->mb_mode_table;
    codecPicParam->b_picture_fraction       = picParam->b_picture_fraction;  //not used
    codecPicParam->range_reduction_frame    = picParam->range_reduction_frame;
    codecPicParam->ScaleFactor              = scaleFactor;
    codecPicParam->post_processing          = picParam->post_processing;
    codecPicParam->picture_resolution_index = 0;
    codecPicParam->rounding_control         = picParam->rounding_control;

    codecPicParam->UpsamplingFlag = picParam->sequence_fields.bits.multires ? (picParam->picture_resolution_index & 0x3) : 0;

    //rangemap
    if (codecPicParam->sequence_fields.AdvancedProfileFlag)
    {
        codecPicParam->range_mapping_fields.luma_flag             = picParam->range_mapping_fields.bits.luma_flag;
        codecPicParam->range_mapping_fields.luma                  = picParam->range_mapping_fields.bits.luma;
        codecPicParam->range_mapping_fields.chroma_flag           = picParam->range_mapping_fields.bits.chroma_flag;
        codecPicParam->range_mapping_fields.chroma                = picParam->range_mapping_fields.bits.chroma;
        codecPicParam->range_mapping_fields.range_mapping_enabled = picParam->range_mapping_fields.value;
    }
    else
    {
        codecPicParam->range_mapping_fields.range_mapping_enabled = picParam->range_reduction_frame;
    }

    // Check if OLP is needed
    // OLP output surface must be present
    if ((m_deblockPicIdx != DDI_CODEC_INVALID_FRAME_INDEX) &&
        ((codecPicParam->DeblockedPicIdx != codecPicParam->CurrPic.FrameIdx) ||
            codecPicParam->UpsamplingFlag ||
            codecPicParam->range_mapping_fields.range_mapping_enabled))
    {
        m_olpNeeded = 1;
    }
    else
    {
        m_olpNeeded = 0;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVC1::ParseSliceParams(
        DDI_MEDIA_CONTEXT            *mediaCtx,
        VASliceParameterBufferVC1    *slcParam,
        uint32_t                      numSlices)
{
    PCODEC_VC1_SLICE_PARAMS codecSlcParam = (PCODEC_VC1_SLICE_PARAMS)(m_ddiDecodeCtx->DecodeParams.m_sliceParams);

    codecSlcParam += m_ddiDecodeCtx->DecodeParams.m_numSlices;

    if ((slcParam == nullptr) || (codecSlcParam == nullptr))
    {
        DDI_ASSERTMESSAGE("Null pointer for Parsing VC1 Slice parameter\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    uint32_t sliceBaseOffset = GetBsBufOffset(m_groupIndex);
    for (uint32_t slcCount = 0; slcCount < numSlices; slcCount++)
    {
        codecSlcParam[slcCount].slice_data_size   = slcParam[slcCount].slice_data_size << 3;
        codecSlcParam[slcCount].slice_data_offset = slcParam[slcCount].slice_data_offset +
                                                    sliceBaseOffset;
        if (slcParam[slcCount].slice_data_flag)
        {
            DDI_NORMALMESSAGE("The whole slice is not in the bitstream buffer for this Execute call");
        }
        codecSlcParam[slcCount].macroblock_offset       = slcParam[slcCount].macroblock_offset;
        codecSlcParam[slcCount].slice_vertical_position = slcParam[slcCount].slice_vertical_position;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVC1::AllocBitPlaneBuffer()
{
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);

    DDI_CHK_NULL(bufMgr, "Null bufMgr in AllocBpBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (bufMgr->Codec_Param.Codec_Param_VC1.VC1BitPlane[bufMgr->Codec_Param.Codec_Param_VC1.dwVC1BitPlaneIndex].bUsed)
    {
        // wait until decode complete
        mos_bo_wait_rendering(bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[bufMgr->Codec_Param.Codec_Param_VC1.dwVC1BitPlaneIndex]->bo);
    }

    bufMgr->Codec_Param.Codec_Param_VC1.VC1BitPlane[bufMgr->Codec_Param.Codec_Param_VC1.dwVC1BitPlaneIndex].bUsed = true;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVC1::AllocSliceParamContext(
    uint32_t numSlices)
{
    uint32_t baseSize = sizeof(CODEC_VC1_SLICE_PARAMS);

    if (m_sliceParamBufNum < (m_ddiDecodeCtx->DecodeParams.m_numSlices + numSlices))
    {
        // in order to avoid that the buffer is reallocated multi-times,
        // extra 10 slices are added.
        uint32_t extraSlices = numSlices + 10;

        m_ddiDecodeCtx->DecodeParams.m_sliceParams = realloc(m_ddiDecodeCtx->DecodeParams.m_sliceParams,
            baseSize * (m_sliceParamBufNum + extraSlices));

        if (m_ddiDecodeCtx->DecodeParams.m_sliceParams == nullptr)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        memset((void *)((uint8_t *)m_ddiDecodeCtx->DecodeParams.m_sliceParams + baseSize * m_sliceParamBufNum), 0, baseSize * extraSlices);
        m_sliceParamBufNum += extraSlices;
    }

    return VA_STATUS_SUCCESS;
}

void DdiDecodeVC1::DestroyContext(
    VADriverContextP ctx)
{
    FreeResourceBuffer();
    // explicitly call the base function to do the further clean-up
    DdiMediaDecode::DestroyContext(ctx);
}

uint8_t* DdiDecodeVC1::GetPicParamBuf(
    DDI_CODEC_COM_BUFFER_MGR    *bufMgr)
{
    return (uint8_t*)(&(bufMgr->Codec_Param.Codec_Param_VC1.PicParamVC1));
}

VAStatus DdiDecodeVC1::AllocSliceControlBuffer(
    DDI_MEDIA_BUFFER       *buf)
{
    DDI_CODEC_COM_BUFFER_MGR   *bufMgr;
    uint32_t                    availSize;
    uint32_t                    newSize;

    bufMgr     = &(m_ddiDecodeCtx->BufMgr);
    availSize  = m_sliceCtrlBufNum - bufMgr->dwNumSliceControl;
    if(availSize < buf->uiNumElements)
    {
        newSize   = sizeof(VASliceParameterBufferVC1) * (m_sliceCtrlBufNum - availSize + buf->uiNumElements);
        bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 = (VASliceParameterBufferVC1 *)realloc(bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1, newSize);
        if(bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 == nullptr)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        MOS_ZeroMemory(bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 + m_sliceCtrlBufNum, sizeof(VASliceParameterBufferVC1) * (buf->uiNumElements - availSize));
        m_sliceCtrlBufNum = m_sliceCtrlBufNum - availSize + buf->uiNumElements;
    }
    buf->pData      = (uint8_t*)bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1;
    buf->uiOffset   = sizeof(VASliceParameterBufferVC1) * bufMgr->dwNumSliceControl;

    bufMgr->dwNumSliceControl += buf->uiNumElements;

    return VA_STATUS_SUCCESS;
}

void DdiDecodeVC1::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    // call the function in base class to initialize it.
    DdiMediaDecode::ContextInit(picWidth, picHeight);

    m_ddiDecodeCtx->wMode    = CODECHAL_DECODE_MODE_VC1VLD;
    m_olpNeeded     = false;
    m_currPicIdx    = DDI_CODEC_INVALID_FRAME_INDEX;
    m_deblockPicIdx = DDI_CODEC_INVALID_FRAME_INDEX;

    if (m_ddiDecodeAttr->profile == VAProfileVC1Advanced)
    {
        int32_t alignedHeight = MOS_ALIGN_CEIL(picHeight, 32);
        m_height = alignedHeight;
        m_picHeightInMB    = (int16_t)(DDI_CODEC_NUM_MACROBLOCKS_HEIGHT(alignedHeight));
    }
}

VAStatus DdiDecodeVC1::BeginPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VASurfaceID      renderTarget)
{
    VAStatus vaStatus = DdiMediaDecode::BeginPicture(ctx, context, renderTarget);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        return vaStatus;
    }

    m_ddiDecodeCtx->DecodeParams.m_vc1BitplaneSize = 0;

    return VA_STATUS_SUCCESS;
}

void DdiDecodeVC1::ParseBitPlane(
    struct _DDI_MEDIA_BUFFER  *bitPlaneBuffObject,
    uint8_t                   *buf)
{
    if (bitPlaneBuffObject->pData)
    {
        PCODEC_VC1_PIC_PARAMS picParam = (PCODEC_VC1_PIC_PARAMS)(m_ddiDecodeCtx->DecodeParams.m_picParams);

        uint8_t *destBp = (uint8_t *)bitPlaneBuffObject->pData;
        uint8_t *srcBp  = (uint8_t *)buf;

        uint32_t heightInMbs   = MOS_ALIGN_CEIL(picParam->coded_height, CODECHAL_MACROBLOCK_HEIGHT) / CODECHAL_MACROBLOCK_HEIGHT;
        uint32_t widthInMbs    = MOS_ALIGN_CEIL(picParam->coded_width, CODECHAL_MACROBLOCK_WIDTH) / CODECHAL_MACROBLOCK_WIDTH;
        uint32_t bitPlanePitch = MOS_ALIGN_CEIL(widthInMbs, 2) / 2;

        uint32_t srcIdx, dstIdx;
        uint32_t srcShift;
        uint32_t i, j;
        uint8_t  srcValue = 0;
        for (i = 0; i < heightInMbs; i++)
        {
            for (j = 0; j < widthInMbs; j++)
            {
                srcIdx   = (i * widthInMbs + j) / 2;
                dstIdx   = j / 2;
                srcShift = ((i * widthInMbs + j) & 1) ? 0 : 4;
                srcValue = (srcBp[srcIdx] >> srcShift) & 0xf;
                if (j % 2)
                    destBp[dstIdx] = destBp[dstIdx] + (srcValue << 4);
                else
                    destBp[dstIdx] = srcValue;
            }
            destBp += bitPlanePitch;
        }
    }
}

VAStatus DdiDecodeVC1::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    DDI_FUNCTION_ENTER();

    VAStatus           va = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx;

    mediaCtx = DdiMedia_GetMediaContext(ctx);

    void             *data = nullptr;
    for (int32_t i = 0; i < numBuffers; i++)
    {
        if (!buffers || (buffers[i] == VA_INVALID_ID))
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, buffers[i]);
        if (nullptr == buf)
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        uint32_t dataSize = buf->iSize;
        DdiMedia_MapBuffer(ctx, buffers[i], &data);

        if (data == nullptr)
        {
            return VA_STATUS_ERROR_INVALID_BUFFER;
        }

        switch ((int32_t)buf->uiType)
        {
        case VABitPlaneBufferType:
        {
            int32_t index = m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.dwVC1BitPlaneIndex;
            if (index == DDI_CODEC_INVALID_BUFFER_INDEX)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }
            if (index >= DDI_CODEC_MAX_BITSTREAM_BUFFER)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }
            DDI_CHK_RET(AllocBitPlaneBuffer(),"AllocBitPlaneBuffer failed!");
            /* Before accessing it on CPU, Use the DdiMediaUtil_LockBuffer to map it into CPU */
            DdiMediaUtil_LockBuffer(m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[index], MOS_LOCKFLAG_WRITEONLY);
            ParseBitPlane(m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[index], (uint8_t *)data);

            DdiMediaUtil_UnlockBuffer(m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[index]);

            DdiMedia_MediaBufferToMosResource(m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[index], &m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.resBitPlaneBuffer);
            m_ddiDecodeCtx->DecodeParams.m_vc1BitplaneSize = dataSize;

            m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.dwVC1BitPlaneIndex++;
            if (m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.dwVC1BitPlaneIndex >= DDI_CODEC_MAX_BITSTREAM_BUFFER)
            {
                m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.dwVC1BitPlaneIndex = 0;
            }
            break;
        }

        case VASliceDataBufferType:
        {
            int32_t index = GetBitstreamBufIndexFromBuffer(&m_ddiDecodeCtx->BufMgr, buf);
            if (index == DDI_CODEC_INVALID_BUFFER_INDEX)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            DdiMedia_MediaBufferToMosResource(m_ddiDecodeCtx->BufMgr.pBitStreamBuffObject[index], &m_ddiDecodeCtx->BufMgr.resBitstreamBuffer);
            m_ddiDecodeCtx->DecodeParams.m_dataSize += dataSize;
            break;
        }
        case VASliceParameterBufferType:
        {
            if (buf->uiNumElements == 0)
            {
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            VASliceParameterBufferVC1 *slcInfo =
                (VASliceParameterBufferVC1 *)data;
            uint32_t numSlices = buf->uiNumElements;
            DDI_CHK_RET(AllocSliceParamContext(numSlices),"AllocSliceParamContext failed!");
            DDI_CHK_RET(ParseSliceParams(mediaCtx, slcInfo, numSlices),"ParseSliceParams failed!");
            m_ddiDecodeCtx->DecodeParams.m_numSlices += numSlices;
            m_groupIndex++;
            break;
        }
        case VAPictureParameterBufferType:
        {
            VAPictureParameterBufferVC1 *picParam = (VAPictureParameterBufferVC1 *)data;
            DDI_CHK_RET(ParsePicParams(mediaCtx, picParam),"ParsePicParams failed!");
            break;
        }
        case VADecodeStreamoutBufferType:
        {
            DdiMedia_MediaBufferToMosResource(buf, &m_ddiDecodeCtx->BufMgr.resExternalStreamOutBuffer);
            m_streamOutEnabled = true;
            break;
        }
        default:
            va = VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
            break;
        }
        DdiMedia_UnmapBuffer(ctx, buffers[i]);
    }

    DDI_FUNCTION_EXIT(va);
    return va;
}

VAStatus DdiDecodeVC1::SetDecodeParams()
{
    if ((&m_ddiDecodeCtx->DecodeParams)->m_numSlices == 0)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_destSurface.dwOffset      = 0;
    MOS_FORMAT expectedFormat = Format_NV12;
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);
    m_destSurface.Format = expectedFormat;

    if (m_deblockPicIdx != DDI_CODEC_INVALID_FRAME_INDEX)
    {
        DdiMedia_MediaSurfaceToMosResource((&(m_ddiDecodeCtx->RTtbl))->pRT[m_currPicIdx], &(m_destSurface.OsResource));
    }
    else
    {
        DdiMedia_MediaSurfaceToMosResource((&(m_ddiDecodeCtx->RTtbl))->pCurrentRT, &(m_destSurface.OsResource));
    }

    if (m_destSurface.OsResource.Format != expectedFormat)
    {
        DDI_NORMALMESSAGE("Surface fomrat of decoded surface is inconsistent with Mpeg2 bitstream\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    (&m_ddiDecodeCtx->DecodeParams)->m_destSurface = &m_destSurface;

    memset(&m_deblockSurface, 0, sizeof(MOS_SURFACE));
    if (m_olpNeeded)
    {
        memset(&m_deblockSurface, 0, sizeof(MOS_SURFACE));
        m_deblockSurface.Format   = Format_NV12;
        m_deblockSurface.dwOffset = 0;
        DdiMedia_MediaSurfaceToMosResource((&(m_ddiDecodeCtx->RTtbl))->pRT[m_deblockPicIdx], &(m_deblockSurface.OsResource));
        (&m_ddiDecodeCtx->DecodeParams)->m_deblockSurface = &m_deblockSurface;
    }
    else
    {
        (&m_ddiDecodeCtx->DecodeParams)->m_deblockSurface = nullptr;
    }

    (&m_ddiDecodeCtx->DecodeParams)->m_dataBuffer       = &bufMgr->resBitstreamBuffer;
    (&m_ddiDecodeCtx->DecodeParams)->m_bitStreamBufData = bufMgr->pBitstreamBuffer;
    Mos_Solo_OverrideBufferSize((&m_ddiDecodeCtx->DecodeParams)->m_dataSize, (&m_ddiDecodeCtx->DecodeParams)->m_dataBuffer);

    (&m_ddiDecodeCtx->DecodeParams)->m_bitplaneBuffer = &(m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.resBitPlaneBuffer);

    if (m_streamOutEnabled)
    {
        (&m_ddiDecodeCtx->DecodeParams)->m_streamOutEnabled        = true;
        (&m_ddiDecodeCtx->DecodeParams)->m_externalStreamOutBuffer = &bufMgr->resExternalStreamOutBuffer;
    }
    else
    {
        (&m_ddiDecodeCtx->DecodeParams)->m_streamOutEnabled        = false;
        (&m_ddiDecodeCtx->DecodeParams)->m_externalStreamOutBuffer = nullptr;
    }

    m_olpNeeded     = false;
    m_currPicIdx    = DDI_CODEC_INVALID_FRAME_INDEX;
    m_deblockPicIdx = DDI_CODEC_INVALID_FRAME_INDEX;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeVC1::InitResourceBuffer(DDI_MEDIA_CONTEXT *mediaCtx)
{
    VAStatus                  vaStatus = VA_STATUS_SUCCESS;
    DDI_CODEC_COM_BUFFER_MGR *bufMgr   = &(m_ddiDecodeCtx->BufMgr);

    bufMgr->pSliceData = nullptr;

    bufMgr->ui64BitstreamOrder = 0;
    bufMgr->dwMaxBsSize        = m_width *
                          m_height * 3 / 2;
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
            goto finish;
        }
        bufMgr->pBitStreamBuffObject[i]->iSize    = bufMgr->dwMaxBsSize;
        bufMgr->pBitStreamBuffObject[i]->uiType   = VASliceDataBufferType;
        bufMgr->pBitStreamBuffObject[i]->format   = Media_Format_Buffer;
        bufMgr->pBitStreamBuffObject[i]->uiOffset = 0;
        bufMgr->pBitStreamBuffObject[i]->bo       = nullptr;
        bufMgr->pBitStreamBase[i]                 = nullptr;
    }

    bufMgr->m_maxNumSliceData = m_picHeightInMB;
    bufMgr->pSliceData        = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)MOS_AllocAndZeroMemory(sizeof(bufMgr->pSliceData[0]) *
                                                                                   bufMgr->m_maxNumSliceData);

    if (bufMgr->pSliceData == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    for (i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i] = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
        if (bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i] == nullptr)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto finish;
        }
        bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i]->iSize     = MOS_ALIGN_CEIL(m_width, CODECHAL_MACROBLOCK_WIDTH) * MOS_ALIGN_CEIL(m_height, CODECHAL_MACROBLOCK_HEIGHT) * 2 / (CODECHAL_MACROBLOCK_HEIGHT * CODECHAL_MACROBLOCK_WIDTH);
        bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i]->uiType    = VABitPlaneBufferType;
        bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i]->format    = Media_Format_Buffer;
        bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i]->uiOffset  = 0;
        bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i]->pMediaCtx = mediaCtx;

        vaStatus = DdiMediaUtil_CreateBuffer(bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i], mediaCtx->pDrmBufMgr);
        if (vaStatus != VA_STATUS_SUCCESS)
        {
            goto finish;
        }

        bufMgr->Codec_Param.Codec_Param_VC1.VC1BitPlane[i].pBitPlaneBase = nullptr;
        bufMgr->Codec_Param.Codec_Param_VC1.VC1BitPlane[i].bUsed         = false;
    }

    bufMgr->dwNumSliceData    = 0;
    bufMgr->dwNumSliceControl = 0;

    m_sliceCtrlBufNum                      = m_picHeightInMB;
    bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 = (VASliceParameterBufferVC1 *)MOS_AllocAndZeroMemory(sizeof(VASliceParameterBufferVC1) * m_sliceCtrlBufNum);
    if (bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    bufMgr->Codec_Param.Codec_Param_VC1.pBitPlaneBuffer = (uint8_t *)MOS_AllocAndZeroMemory(m_picWidthInMB * m_picHeightInMB);
    if (bufMgr->Codec_Param.Codec_Param_VC1.pBitPlaneBuffer == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    return VA_STATUS_SUCCESS;

finish:
    FreeResourceBuffer();
    return vaStatus;
}

void DdiDecodeVC1::FreeResourceBuffer()
{
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);

    int32_t i;
    for (i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        if (bufMgr->pBitStreamBase[i])
        {
            DdiMediaUtil_UnlockBuffer(bufMgr->pBitStreamBuffObject[i]);
            bufMgr->pBitStreamBase[i] = nullptr;
        }
        if (bufMgr->pBitStreamBuffObject[i])
        {
            DdiMediaUtil_FreeBuffer(bufMgr->pBitStreamBuffObject[i]);
            MOS_FreeMemory(bufMgr->pBitStreamBuffObject[i]);
            bufMgr->pBitStreamBuffObject[i] = nullptr;
        }
    }

    if (bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1)
    {
        MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1);
        bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 = nullptr;
    }
    if (bufMgr->Codec_Param.Codec_Param_VC1.pBitPlaneBuffer)
    {
        MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_VC1.pBitPlaneBuffer);
        bufMgr->Codec_Param.Codec_Param_VC1.pBitPlaneBuffer = nullptr;
    }
    for (i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        if (bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i])
        {
            DdiMediaUtil_UnlockBuffer(bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i]);
            DdiMediaUtil_FreeBuffer(bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i]);
            MOS_FreeMemory(bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i]);
            bufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[i] = nullptr;
        }
    }

    // free decode bitstream buffer object
    MOS_FreeMemory(bufMgr->pSliceData);
    bufMgr->pSliceData = nullptr;

    return;
}

VAStatus DdiDecodeVC1::CodecHalInit(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *ptr)
{
    VAStatus     vaStatus = VA_STATUS_SUCCESS;
    MOS_CONTEXT *mosCtx   = (MOS_CONTEXT *)ptr;

    CODECHAL_FUNCTION codecFunction = CODECHAL_FUNCTION_DECODE;
    m_ddiDecodeCtx->pCpDdiInterface->SetCpParams(m_ddiDecodeAttr->uiEncryptionType, m_codechalSettings);

    CODECHAL_STANDARD_INFO standardInfo;
    memset(&standardInfo, 0, sizeof(standardInfo));

    standardInfo.CodecFunction = codecFunction;
    standardInfo.Mode          = (CODECHAL_MODE)m_ddiDecodeCtx->wMode;

    m_codechalSettings->codecFunction                = codecFunction;
    m_codechalSettings->width                      = m_width;
    m_codechalSettings->height                     = m_height;

    m_codechalSettings->lumaChromaDepth = CODECHAL_LUMA_CHROMA_DEPTH_8_BITS;

    m_codechalSettings->shortFormatInUse = m_ddiDecodeCtx->bShortFormatInUse;

    m_codechalSettings->mode     = CODECHAL_DECODE_MODE_VC1VLD;
    m_codechalSettings->standard = CODECHAL_VC1;
    /* VC1 uses the Intel-specific Format */
    m_codechalSettings->intelEntrypointInUse = true;

    m_ddiDecodeCtx->DecodeParams.m_picParams = MOS_AllocAndZeroMemory(sizeof(CODEC_VC1_PIC_PARAMS));
    if (m_ddiDecodeCtx->DecodeParams.m_picParams == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    m_sliceParamBufNum         = m_picHeightInMB;
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = MOS_AllocAndZeroMemory(m_sliceParamBufNum * sizeof(CODEC_VC1_SLICE_PARAMS));
    if (m_ddiDecodeCtx->DecodeParams.m_sliceParams == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    vaStatus = CreateCodecHal(mediaCtx,
        ptr,
        &standardInfo);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto CleanUpandReturn;
    }

    if (InitResourceBuffer(mediaCtx) != VA_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    return vaStatus;

CleanUpandReturn:
    FreeResourceBuffer();

    if (m_ddiDecodeCtx->pCodecHal)
    {
        m_ddiDecodeCtx->pCodecHal->Destroy();
        MOS_Delete(m_ddiDecodeCtx->pCodecHal);
        m_ddiDecodeCtx->pCodecHal = nullptr;
    }

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_picParams);
    m_ddiDecodeCtx->DecodeParams.m_picParams = nullptr;
    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_sliceParams);
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = nullptr;

    return vaStatus;
}

extern template class MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR>;

static bool vc1Registered =
    MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR>::RegisterCodec<DdiDecodeVC1>(DECODE_ID_VC1);
