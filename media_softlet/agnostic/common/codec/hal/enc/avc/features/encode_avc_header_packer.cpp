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
//! \file     encode_avc_header_packer.cpp
//! \brief    Defines header packing logic for avc encode
//!

#include "encode_avc_header_packer.h"
#include "encode_utils.h"

namespace encode
{

#define ENCODE_AVC_EXTENDED_SAR 255

static void SetNalUnit(uint8_t **bsbuffer, uint8_t refIDC, CODECHAL_ENCODE_AVC_NAL_UNIT_TYPE nalType)
{
    uint8_t *byte = *bsbuffer;

    // for SPS and PPS NAL units zero_byte should exist
    if (nalType == CODECHAL_ENCODE_AVC_NAL_UT_SPS || nalType == CODECHAL_ENCODE_AVC_NAL_UT_PPS || nalType == CODECHAL_ENCODE_AVC_NAL_UT_AUD)
    {
        *byte++ = 0;
    }

    *byte++   = 0;
    *byte++   = 0;
    *byte++   = 1;
    *byte++   = (uint8_t)((refIDC << 5) | nalType);
    *byte     = 0;  // Clear the next byte
    *bsbuffer = byte;
}

static void PutBit(BSBuffer *bsbuffer, uint32_t code)
{
    if (code & 1)
    {
        *(bsbuffer->pCurrent) = (*(bsbuffer->pCurrent) | (uint8_t)(0x01 << (7 - bsbuffer->BitOffset)));
    }

    bsbuffer->BitOffset++;
    if (bsbuffer->BitOffset == 8)
    {
        bsbuffer->BitOffset = 0;
        bsbuffer->pCurrent++;
        *(bsbuffer->pCurrent) = 0;
    }
}

static void PutBitsSub(BSBuffer *bsbuffer, uint32_t code, uint32_t length)
{
    uint8_t *byte = bsbuffer->pCurrent;

    // make sure that the number of bits given is <= 24
    ENCODE_ASSERT(length <= 24);

    code <<= (32 - length);

    // shift field so that the given code begins at the current bit
    // offset in the most significant byte of the 32-bit word
    length += bsbuffer->BitOffset;
    code >>= bsbuffer->BitOffset;

    // write bytes back into memory, big-endian
    byte[0] = (uint8_t)((code >> 24) | byte[0]);
    byte[1] = (uint8_t)(code >> 16);
    if (length > 16)
    {
        byte[2] = (uint8_t)(code >> 8);
        byte[3] = (uint8_t)code;
    }
    else
    {
        byte[2] = 0;
    }

    // update bitstream pointer and bit offset
    bsbuffer->pCurrent += (length >> 3);
    bsbuffer->BitOffset = (length & 7);
}

static void PutBits(BSBuffer *bsbuffer, uint32_t code, uint32_t length)
{
    uint32_t code1, code2;

    // temp solution, only support up to 32 bits based on current usage
    ENCODE_ASSERT(length <= 32);

    if (length >= 24)
    {
        code1 = code & 0xFFFF;
        code2 = code >> 16;

        // high bits go first
        PutBitsSub(bsbuffer, code2, length - 16);
        PutBitsSub(bsbuffer, code1, 16);
    }
    else
    {
        PutBitsSub(bsbuffer, code, length);
    }
}

static void PutVLCCode(BSBuffer *bsbuffer, uint32_t code)
{
    uint8_t  leadingZeroBits, bitcount;
    uint32_t code1, bits;

    code1    = code + 1;
    bitcount = 0;
    while (code1)
    {
        code1 >>= 1;
        bitcount++;
    }

    if (bitcount == 1)
    {
        PutBit(bsbuffer, 1);
    }
    else
    {
        leadingZeroBits = bitcount - 1;
        bits            = code + 1 - (1 << leadingZeroBits);
        PutBits(bsbuffer, 1, leadingZeroBits + 1);
        PutBits(bsbuffer, bits, leadingZeroBits);
    }
}

static void SetTrailingBits(BSBuffer *bsbuffer)
{
    // Write Stop Bit
    PutBits(bsbuffer, 1, 1);
    // Make byte aligned
    while (bsbuffer->BitOffset)
    {
        PutBit(bsbuffer, 0);
    }
}

static void PackScalingList(BSBuffer *bsbuffer, uint8_t *scalingList, uint8_t sizeOfScalingList)
{
    uint8_t lastScale, nextScale, j;
    char    delta_scale;

    lastScale = nextScale = 8;

    for (j = 0; j < sizeOfScalingList; j++)
    {
        if (nextScale != 0)
        {
            delta_scale = (char)(scalingList[j] - lastScale);

            PutVLCCode(bsbuffer, SIGNED(delta_scale));

            nextScale = scalingList[j];
        }
        lastScale = (nextScale == 0) ? lastScale : nextScale;
    }
}

MOS_STATUS AvcEncodeHeaderPacker::PackAUDParams(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    uint32_t   picType;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pBsBuffer);

    // refer table 7-5 in H.264 spec on primary_pic_type
    // Here I,P,B types are included
    // According BD Spec 9.5.1.1, 0 - I; 1 - P; 2 - B

    picType = (uint32_t)(params->wPictureCodingType) - 1;
    PutBits(params->pBsBuffer, picType, 3);

    return eStatus;
}

MOS_STATUS AvcEncodeHeaderPacker::PackHrdParams(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    PCODECHAL_ENCODE_AVC_VUI_PARAMS vuiParams;
    PBSBuffer                       bsbuffer;
    int                             schedSelIdx;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);

    vuiParams = params->pAvcVuiParams;
    bsbuffer  = params->pBsBuffer;

    PutVLCCode(bsbuffer, vuiParams->cpb_cnt_minus1);
    PutBits(bsbuffer, vuiParams->bit_rate_scale, 4);
    PutBits(bsbuffer, vuiParams->cpb_size_scale, 4);

    for (schedSelIdx = 0; schedSelIdx <= vuiParams->cpb_cnt_minus1; schedSelIdx++)
    {
        PutVLCCode(bsbuffer, vuiParams->bit_rate_value_minus1[schedSelIdx]);
        PutVLCCode(bsbuffer, vuiParams->cpb_size_value_minus1[schedSelIdx]);
        PutBit(bsbuffer, ((vuiParams->cbr_flag >> schedSelIdx) & 1));
    }

    PutBits(bsbuffer, vuiParams->initial_cpb_removal_delay_length_minus1, 5);
    PutBits(bsbuffer, vuiParams->cpb_removal_delay_length_minus1, 5);
    PutBits(bsbuffer, vuiParams->dpb_output_delay_length_minus1, 5);
    PutBits(bsbuffer, vuiParams->time_offset_length, 5);

    return eStatus;
}

MOS_STATUS AvcEncodeHeaderPacker::PackVuiParams(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    PCODECHAL_ENCODE_AVC_VUI_PARAMS vuiParams;
    PBSBuffer                       bsbuffer;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pAvcVuiParams);

    vuiParams = params->pAvcVuiParams;
    bsbuffer  = params->pBsBuffer;

    PutBit(bsbuffer, vuiParams->aspect_ratio_info_present_flag);
    if (vuiParams->aspect_ratio_info_present_flag)
    {
        PutBits(bsbuffer, vuiParams->aspect_ratio_idc, 8);
        if (vuiParams->aspect_ratio_idc == ENCODE_AVC_EXTENDED_SAR)
        {
            PutBits(bsbuffer, vuiParams->sar_width, 16);
            PutBits(bsbuffer, vuiParams->sar_height, 16);
        }
    }

    PutBit(bsbuffer, vuiParams->overscan_info_present_flag);
    if (vuiParams->overscan_info_present_flag)
    {
        PutBit(bsbuffer, vuiParams->overscan_appropriate_flag);
    }

    PutBit(bsbuffer, vuiParams->video_signal_type_present_flag);
    if (vuiParams->video_signal_type_present_flag)
    {
        PutBits(bsbuffer, vuiParams->video_format, 3);
        PutBit(bsbuffer, vuiParams->video_full_range_flag);
        PutBit(bsbuffer, vuiParams->colour_description_present_flag);
        if (vuiParams->colour_description_present_flag)
        {
            PutBits(bsbuffer, vuiParams->colour_primaries, 8);
            PutBits(bsbuffer, vuiParams->transfer_characteristics, 8);
            PutBits(bsbuffer, vuiParams->matrix_coefficients, 8);
        }
    }

    PutBit(bsbuffer, vuiParams->chroma_loc_info_present_flag);
    if (vuiParams->chroma_loc_info_present_flag)
    {
        PutVLCCode(bsbuffer, vuiParams->chroma_sample_loc_type_top_field);
        PutVLCCode(bsbuffer, vuiParams->chroma_sample_loc_type_bottom_field);
    }

    PutBit(bsbuffer, vuiParams->timing_info_present_flag);
    if (vuiParams->timing_info_present_flag)
    {
        PutBits(bsbuffer, vuiParams->num_units_in_tick, 32);
        PutBits(bsbuffer, vuiParams->time_scale, 32);
        PutBit(bsbuffer, vuiParams->fixed_frame_rate_flag);
    }

    PutBit(bsbuffer, vuiParams->nal_hrd_parameters_present_flag);
    if (vuiParams->nal_hrd_parameters_present_flag)
    {
        ENCODE_CHK_STATUS_RETURN(PackHrdParams(params));
    }

    PutBit(bsbuffer, vuiParams->vcl_hrd_parameters_present_flag);
    if (vuiParams->vcl_hrd_parameters_present_flag)
    {
        ENCODE_CHK_STATUS_RETURN(PackHrdParams(params));
    }

    if (vuiParams->nal_hrd_parameters_present_flag || vuiParams->vcl_hrd_parameters_present_flag)
    {
        PutBit(bsbuffer, vuiParams->low_delay_hrd_flag);
    }

    PutBit(bsbuffer, vuiParams->pic_struct_present_flag);
    PutBit(bsbuffer, vuiParams->bitstream_restriction_flag);
    if (vuiParams->bitstream_restriction_flag)
    {
        PutBit(bsbuffer, vuiParams->motion_vectors_over_pic_boundaries_flag);
        PutVLCCode(bsbuffer, vuiParams->max_bytes_per_pic_denom);
        PutVLCCode(bsbuffer, vuiParams->max_bits_per_mb_denom);
        PutVLCCode(bsbuffer, vuiParams->log2_max_mv_length_horizontal);
        PutVLCCode(bsbuffer, vuiParams->log2_max_mv_length_vertical);
        PutVLCCode(bsbuffer, vuiParams->num_reorder_frames);
        PutVLCCode(bsbuffer, vuiParams->max_dec_frame_buffering);
    }

    return eStatus;
}

MOS_STATUS AvcEncodeHeaderPacker::PackSeqParams(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams;
    BSBuffer *                        bsbuffer;
    uint8_t                           i;
    MOS_STATUS                        eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pBsBuffer);
    ENCODE_CHK_NULL_RETURN(params->pSeqParams);

    seqParams = params->pSeqParams;
    bsbuffer  = params->pBsBuffer;

    PutBits(bsbuffer, seqParams->Profile, 8);

    PutBit(bsbuffer, seqParams->constraint_set0_flag);
    PutBit(bsbuffer, seqParams->constraint_set1_flag);
    PutBit(bsbuffer, seqParams->constraint_set2_flag);
    PutBit(bsbuffer, seqParams->constraint_set3_flag);

    PutBits(bsbuffer, 0, 4);
    PutBits(bsbuffer, seqParams->Level, 8);
    PutVLCCode(bsbuffer, seqParams->seq_parameter_set_id);

    if (seqParams->Profile == CODEC_AVC_HIGH_PROFILE ||
        seqParams->Profile == CODEC_AVC_HIGH10_PROFILE ||
        seqParams->Profile == CODEC_AVC_HIGH422_PROFILE ||
        seqParams->Profile == CODEC_AVC_HIGH444_PROFILE ||
        seqParams->Profile == CODEC_AVC_CAVLC444_INTRA_PROFILE ||
        seqParams->Profile == CODEC_AVC_SCALABLE_BASE_PROFILE ||
        seqParams->Profile == CODEC_AVC_SCALABLE_HIGH_PROFILE)
    {
        PutVLCCode(bsbuffer, seqParams->chroma_format_idc);
        if (seqParams->chroma_format_idc == 3)
        {
            PutBit(bsbuffer, seqParams->separate_colour_plane_flag);
        }
        PutVLCCode(bsbuffer, seqParams->bit_depth_luma_minus8);
        PutVLCCode(bsbuffer, seqParams->bit_depth_chroma_minus8);
        PutBit(bsbuffer, seqParams->qpprime_y_zero_transform_bypass_flag);
        PutBit(bsbuffer, seqParams->seq_scaling_matrix_present_flag);
        if (seqParams->seq_scaling_matrix_present_flag)
        {
            //Iterate thro' the scaling lists. Refer to ITU-T H.264 std. section 7.3.2.1
            for (i = 0; i < 8; i++)
            {
                // scaling list present flag
                PutBit(bsbuffer, seqParams->seq_scaling_list_present_flag[i]);
                if (seqParams->seq_scaling_list_present_flag[i])
                {
                    if (i < 6)
                    {
                        PackScalingList(bsbuffer, &params->pAvcIQMatrixParams->ScalingList4x4[i][0], 16);
                    }
                    else
                    {
                        PackScalingList(bsbuffer, &params->pAvcIQMatrixParams->ScalingList8x8[i - 6][0], 64);
                    }
                }
            }
        }
    }

    PutVLCCode(bsbuffer, seqParams->log2_max_frame_num_minus4);
    PutVLCCode(bsbuffer, seqParams->pic_order_cnt_type);
    if (seqParams->pic_order_cnt_type == 0)
    {
        PutVLCCode(bsbuffer, seqParams->log2_max_pic_order_cnt_lsb_minus4);
    }
    else if (seqParams->pic_order_cnt_type == 1)
    {
        PutBit(bsbuffer, seqParams->delta_pic_order_always_zero_flag);
        PutVLCCode(bsbuffer, SIGNED(seqParams->offset_for_non_ref_pic));
        PutVLCCode(bsbuffer, SIGNED(seqParams->offset_for_top_to_bottom_field));
        PutVLCCode(bsbuffer, seqParams->num_ref_frames_in_pic_order_cnt_cycle);
        for (i = 0; i < seqParams->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            PutVLCCode(bsbuffer, SIGNED(seqParams->offset_for_ref_frame[i]));
        }
    }

    PutVLCCode(bsbuffer, seqParams->NumRefFrames);
    PutBit(bsbuffer, seqParams->gaps_in_frame_num_value_allowed_flag);
    PutVLCCode(bsbuffer, seqParams->pic_width_in_mbs_minus1);
    PutVLCCode(bsbuffer, seqParams->pic_height_in_map_units_minus1);
    PutBit(bsbuffer, seqParams->frame_mbs_only_flag);

    if (!seqParams->frame_mbs_only_flag)
    {
        PutBit(bsbuffer, seqParams->mb_adaptive_frame_field_flag);
    }

    PutBit(bsbuffer, seqParams->direct_8x8_inference_flag);

    if ((!seqParams->frame_cropping_flag) &&
        (params->dwFrameHeight != params->dwOriFrameHeight))
    {
        seqParams->frame_cropping_flag = 1;
        seqParams->frame_crop_bottom_offset =
            (int16_t)((params->dwFrameHeight - params->dwOriFrameHeight) >>
                      (2 - seqParams->frame_mbs_only_flag));  // 4:2:0
    }

    PutBit(bsbuffer, seqParams->frame_cropping_flag);

    if (seqParams->frame_cropping_flag)
    {
        PutVLCCode(bsbuffer, seqParams->frame_crop_left_offset);
        PutVLCCode(bsbuffer, seqParams->frame_crop_right_offset);
        PutVLCCode(bsbuffer, seqParams->frame_crop_top_offset);
        PutVLCCode(bsbuffer, seqParams->frame_crop_bottom_offset);
    }

    PutBit(bsbuffer, seqParams->vui_parameters_present_flag);

    if (seqParams->vui_parameters_present_flag)
    {
        ENCODE_CHK_STATUS_RETURN(PackVuiParams(params));
    }

    *params->pbNewSeqHeader = 1;

    return eStatus;
}

MOS_STATUS AvcEncodeHeaderPacker::PackPicParams(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams;
    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams;
    PBSBuffer                         bsbuffer;
    MOS_STATUS                        eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    ENCODE_CHK_NULL_RETURN(params->pPicParams);
    ENCODE_CHK_NULL_RETURN(params->pBsBuffer);

    seqParams = params->pSeqParams;
    picParams = params->pPicParams;
    bsbuffer  = params->pBsBuffer;

    PutVLCCode(bsbuffer, picParams->pic_parameter_set_id);
    PutVLCCode(bsbuffer, picParams->seq_parameter_set_id);

    PutBit(bsbuffer, picParams->entropy_coding_mode_flag);
    PutBit(bsbuffer, picParams->pic_order_present_flag);

    PutVLCCode(bsbuffer, picParams->num_slice_groups_minus1);

    PutVLCCode(bsbuffer, picParams->num_ref_idx_l0_active_minus1);
    PutVLCCode(bsbuffer, picParams->num_ref_idx_l1_active_minus1);

    PutBit(bsbuffer, picParams->weighted_pred_flag);
    PutBits(bsbuffer, picParams->weighted_bipred_idc, 2);

    PutVLCCode(bsbuffer, SIGNED(picParams->pic_init_qp_minus26));
    PutVLCCode(bsbuffer, SIGNED(picParams->pic_init_qs_minus26));
    PutVLCCode(bsbuffer, SIGNED(picParams->chroma_qp_index_offset));

    PutBit(bsbuffer, picParams->deblocking_filter_control_present_flag);
    PutBit(bsbuffer, picParams->constrained_intra_pred_flag);
    PutBit(bsbuffer, picParams->redundant_pic_cnt_present_flag);

    // The syntax elements transform_8x8_mode_flag, pic_scaling_matrix_present_flag, and second_chroma_qp_index_offset
    // shall not be present for main profile
    if (seqParams->Profile == CODEC_AVC_MAIN_PROFILE || seqParams->Profile == CODEC_AVC_BASE_PROFILE)
    {
        return eStatus;
    }

    PutBit(bsbuffer, picParams->transform_8x8_mode_flag);
    PutBit(bsbuffer, picParams->pic_scaling_matrix_present_flag);
    if (picParams->pic_scaling_matrix_present_flag)
    {
        uint8_t i;

        //Iterate thro' the scaling lists. Refer to ITU-T H.264 std. section 7.3.2.2
        for (i = 0; i < 6 + 2 * picParams->transform_8x8_mode_flag; i++)
        {
            //Put scaling list present flag
            PutBit(bsbuffer, picParams->pic_scaling_list_present_flag[i]);
            if (picParams->pic_scaling_list_present_flag[i])
            {
                if (i < 6)
                {
                    PackScalingList(bsbuffer, &params->pAvcIQMatrixParams->ScalingList4x4[i][0], 16);
                }
                else
                {
                    PackScalingList(bsbuffer, &params->pAvcIQMatrixParams->ScalingList8x8[i - 6][0], 64);
                }
            }
        }
    }

    PutVLCCode(bsbuffer, SIGNED(picParams->second_chroma_qp_index_offset));

    *params->pbNewPPSHeader = 1;

    return eStatus;
}

MOS_STATUS AvcEncodeHeaderPacker::PackPictureHeader(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params)
{
    ENCODE_FUNC_CALL();

    PBSBuffer  bsbuffer;
    uint32_t   indexNALUnit;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pBsBuffer);
    ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    ENCODE_CHK_NULL_RETURN(params->ppNALUnitParams);

    bsbuffer              = params->pBsBuffer;
    *(bsbuffer->pBase)    = 0;  // init first byte to 0
    bsbuffer->pCurrent    = bsbuffer->pBase;
    bsbuffer->SliceOffset = 0;
    bsbuffer->BitOffset   = 0;
    bsbuffer->BitSize     = 0;

    MOS_ZeroMemory(params->ppNALUnitParams[0], sizeof(CODECHAL_NAL_UNIT_PARAMS) * CODECHAL_ENCODE_AVC_MAX_NAL_TYPE);
    indexNALUnit = 0;

    // AU_Delimiter
    // nal_ref_idc to be 0 for all nal_unit_type equal to 6, 9, 10, 11 or12
    params->ppNALUnitParams[indexNALUnit]->uiOffset                  = 0;
    params->ppNALUnitParams[indexNALUnit]->uiNalUnitType             = CODECHAL_ENCODE_AVC_NAL_UT_AUD;
    params->ppNALUnitParams[indexNALUnit]->bInsertEmulationBytes     = true;
    params->ppNALUnitParams[indexNALUnit]->uiSkipEmulationCheckCount = 4;
    SetNalUnit(&bsbuffer->pCurrent, 0, CODECHAL_ENCODE_AVC_NAL_UT_AUD);
    ENCODE_CHK_STATUS_RETURN(PackAUDParams(params));
    SetTrailingBits(bsbuffer);
    //NAL unit are byte aligned, bsbuffer->BitOffset should be 0
    params->ppNALUnitParams[indexNALUnit]->uiSize =
        (uint32_t)(bsbuffer->pCurrent -
                   bsbuffer->pBase -
                   params->ppNALUnitParams[indexNALUnit]->uiOffset);
    indexNALUnit++;

    // If this is a new sequence, write the seq set
    if (params->bNewSeq && !params->pSeqParams->bNoAcceleratorSPSInsertion)
    {
        // Pack SPS
        params->ppNALUnitParams[indexNALUnit]->uiOffset                  = (uint32_t)(bsbuffer->pCurrent - bsbuffer->pBase);
        params->ppNALUnitParams[indexNALUnit]->uiNalUnitType             = CODECHAL_ENCODE_AVC_NAL_UT_SPS;
        params->ppNALUnitParams[indexNALUnit]->bInsertEmulationBytes     = true;
        params->ppNALUnitParams[indexNALUnit]->uiSkipEmulationCheckCount = 4;
        SetNalUnit(&bsbuffer->pCurrent, 1, CODECHAL_ENCODE_AVC_NAL_UT_SPS);
        ENCODE_CHK_STATUS_RETURN(PackSeqParams(params));
        SetTrailingBits(bsbuffer);
        params->ppNALUnitParams[indexNALUnit]->uiSize =
            (uint32_t)(bsbuffer->pCurrent -
                       bsbuffer->pBase -
                       params->ppNALUnitParams[indexNALUnit]->uiOffset);
        indexNALUnit++;
    }

    // Pack PPS
    params->ppNALUnitParams[indexNALUnit]->uiOffset                  = (uint32_t)(bsbuffer->pCurrent - bsbuffer->pBase);
    params->ppNALUnitParams[indexNALUnit]->uiNalUnitType             = CODECHAL_ENCODE_AVC_NAL_UT_PPS;
    params->ppNALUnitParams[indexNALUnit]->bInsertEmulationBytes     = true;
    params->ppNALUnitParams[indexNALUnit]->uiSkipEmulationCheckCount = 4;
    SetNalUnit(&bsbuffer->pCurrent, 1, CODECHAL_ENCODE_AVC_NAL_UT_PPS);
    ENCODE_CHK_STATUS_RETURN(PackPicParams(params));
    SetTrailingBits(bsbuffer);
    params->ppNALUnitParams[indexNALUnit]->uiSize =
        (uint32_t)(bsbuffer->pCurrent -
                   bsbuffer->pBase -
                   params->ppNALUnitParams[indexNALUnit]->uiOffset);
    indexNALUnit++;

    // Pack SEI
    if (params->pSeiData->newSEIData)
    {
        params->ppNALUnitParams[indexNALUnit]->uiOffset                  = (uint32_t)(bsbuffer->pCurrent - bsbuffer->pBase);
        params->ppNALUnitParams[indexNALUnit]->uiNalUnitType             = CODECHAL_ENCODE_AVC_NAL_UT_SEI;
        params->ppNALUnitParams[indexNALUnit]->bInsertEmulationBytes     = false;
        params->ppNALUnitParams[indexNALUnit]->uiSkipEmulationCheckCount = 4;
        eStatus                                                          = MOS_SecureMemcpy(bsbuffer->pCurrent,
            params->pSeiData->dwSEIBufSize,
            params->pSeiData->pSEIBuffer,
            params->pSeiData->dwSEIBufSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        bsbuffer->pCurrent += params->pSeiData->dwSEIDataSize;
        params->pSeiData->newSEIData = false;
        params->ppNALUnitParams[indexNALUnit]->uiSize =
            (uint32_t)(bsbuffer->pCurrent -
                       bsbuffer->pBase -
                       params->ppNALUnitParams[indexNALUnit]->uiOffset);
        indexNALUnit++;
    }

    bsbuffer->SliceOffset = (uint32_t)(bsbuffer->pCurrent - bsbuffer->pBase);

    return eStatus;
}

MOS_STATUS AvcEncodeHeaderPacker::PackSliceHeader(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams;
    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams;
    PCODEC_AVC_ENCODE_SLICE_PARAMS    slcParams;
    PBSBuffer                         bsbuffer;
    uint8_t                           sliceType;
    CODECHAL_ENCODE_AVC_NAL_UNIT_TYPE nalType;
    bool                              ref;
    MOS_STATUS                        eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    ENCODE_CHK_NULL_RETURN(params->pPicParams);
    ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);
    ENCODE_CHK_NULL_RETURN(params->pBsBuffer);

    slcParams = params->pAvcSliceParams;
    picParams = params->pPicParams;
    seqParams = params->pSeqParams;
    bsbuffer  = params->pBsBuffer;
    sliceType = Slice_Type[slcParams->slice_type];
    nalType   = params->NalUnitType;
    ref       = params->ppRefList[params->CurrReconPic.FrameIdx]->bUsedAsRef;

    // Make slice header uint8_t aligned
    while (bsbuffer->BitOffset)
    {
        PutBit(bsbuffer, 0);
    }

    // zero byte shall exist when the byte stream NAL unit syntax structure contains the first
    // NAL unit of an access unit in decoding order, as specified by subclause 7.4.1.2.3.
    // VDEnc Slice header packing handled by PAK does not need the 0 byte inserted
    if (params->UserFlags.bDisableAcceleratorHeaderPacking && (!params->bVdencEnabled))
    {
        *bsbuffer->pCurrent = 0;
        bsbuffer->pCurrent++;
    }

    SetNalUnit(&bsbuffer->pCurrent, (uint8_t)ref, nalType);

    // In the VDEnc mode, PAK only gets this command at the beginning of the frame for slice position X=0, Y=0
    PutVLCCode(bsbuffer, params->bVdencEnabled ? 0 : slcParams->first_mb_in_slice);
    PutVLCCode(bsbuffer, slcParams->slice_type);
    PutVLCCode(bsbuffer, slcParams->pic_parameter_set_id);

    if (seqParams->separate_colour_plane_flag)
    {
        PutBits(bsbuffer, slcParams->colour_plane_id, 2);
    }

    PutBits(bsbuffer, slcParams->frame_num, seqParams->log2_max_frame_num_minus4 + 4);

    if (!seqParams->frame_mbs_only_flag)
    {
        PutBit(bsbuffer, slcParams->field_pic_flag);
        if (slcParams->field_pic_flag)
        {
            PutBit(bsbuffer, slcParams->bottom_field_flag);
        }
    }

    if (nalType == CODECHAL_ENCODE_AVC_NAL_UT_IDR_SLICE)
    {
        PutVLCCode(bsbuffer, slcParams->idr_pic_id);
    }

    if (seqParams->pic_order_cnt_type == 0)
    {
        PutBits(bsbuffer, slcParams->pic_order_cnt_lsb, seqParams->log2_max_pic_order_cnt_lsb_minus4 + 4);
        if (picParams->pic_order_present_flag && !slcParams->field_pic_flag)
        {
            PutVLCCode(bsbuffer, SIGNED(slcParams->delta_pic_order_cnt_bottom));
        }
    }

    if (seqParams->pic_order_cnt_type == 1 && !seqParams->delta_pic_order_always_zero_flag)
    {
        PutVLCCode(bsbuffer, SIGNED(slcParams->delta_pic_order_cnt[0]));
        if (picParams->pic_order_present_flag && !slcParams->field_pic_flag)
        {
            PutVLCCode(bsbuffer, SIGNED(slcParams->delta_pic_order_cnt[1]));
        }
    }

    if (picParams->redundant_pic_cnt_present_flag)
    {
        PutVLCCode(bsbuffer, slcParams->redundant_pic_cnt);
    }

    if (sliceType == SLICE_B)
    {
        PutBit(bsbuffer, slcParams->direct_spatial_mv_pred_flag);
    }

    if (sliceType == SLICE_P || sliceType == SLICE_SP || sliceType == SLICE_B)
    {
        PutBit(bsbuffer, slcParams->num_ref_idx_active_override_flag);
        if (slcParams->num_ref_idx_active_override_flag)
        {
            PutVLCCode(bsbuffer, slcParams->num_ref_idx_l0_active_minus1);
            if (sliceType == SLICE_B)
            {
                PutVLCCode(bsbuffer, slcParams->num_ref_idx_l1_active_minus1);
            }
        }
    }

    // ref_pic_list_reordering()
    ENCODE_CHK_STATUS_RETURN(RefPicListReordering(params));

    if ((picParams->weighted_pred_flag &&
            (sliceType == SLICE_P || sliceType == SLICE_SP)) ||
        (picParams->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE &&
            sliceType == SLICE_B))
    {
        ENCODE_CHK_STATUS_RETURN(PredWeightTable(params));
    }

    if (ref)
    {
        // dec_ref_pic_marking()
        if (nalType == CODECHAL_ENCODE_AVC_NAL_UT_IDR_SLICE)
        {
            PutBit(bsbuffer, slcParams->no_output_of_prior_pics_flag);
            PutBit(bsbuffer, slcParams->long_term_reference_flag);
        }
        else
        {
            PutBit(bsbuffer, slcParams->adaptive_ref_pic_marking_mode_flag);
            if (slcParams->adaptive_ref_pic_marking_mode_flag)
            {
                ENCODE_CHK_STATUS_RETURN(MMCO(params));
            }
        }
    }

    if (picParams->entropy_coding_mode_flag && sliceType != SLICE_I && sliceType != SLICE_SI)
    {
        PutVLCCode(bsbuffer, slcParams->cabac_init_idc);
    }

    PutVLCCode(bsbuffer, SIGNED(slcParams->slice_qp_delta));

    if (sliceType == SLICE_SP || sliceType == SLICE_SI)
    {
        if (sliceType == SLICE_SP)
        {
            PutBit(bsbuffer, slcParams->sp_for_switch_flag);
        }
        PutVLCCode(bsbuffer, SIGNED(slcParams->slice_qs_delta));
    }

    if (picParams->deblocking_filter_control_present_flag)
    {
        PutVLCCode(bsbuffer, slcParams->disable_deblocking_filter_idc);
        if (slcParams->disable_deblocking_filter_idc != 1)
        {
            PutVLCCode(bsbuffer, SIGNED(slcParams->slice_alpha_c0_offset_div2));
            PutVLCCode(bsbuffer, SIGNED(slcParams->slice_beta_offset_div2));
        }
    }

    bsbuffer->BitSize =
        (uint32_t)((bsbuffer->pCurrent - bsbuffer->SliceOffset - bsbuffer->pBase) * 8 + bsbuffer->BitOffset);
    bsbuffer->SliceOffset =
        (uint32_t)(bsbuffer->pCurrent - bsbuffer->pBase + (bsbuffer->BitOffset != 0));

    return eStatus;
}

MOS_STATUS AvcEncodeHeaderPacker::RefPicListReordering(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PBSBuffer                      bsbuffer;
    CODEC_PIC_REORDER *            picOrder;
    uint8_t                        sliceType;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);

    slcParams = params->pAvcSliceParams;
    bsbuffer  = params->pBsBuffer;
    sliceType = Slice_Type[slcParams->slice_type];

    if (!params->UserFlags.bDisableAcceleratorRefPicListReordering)
    {
        // Generate the initial reference list (PicOrder)
        SetInitialRefPicList(params);
    }

    if (sliceType != SLICE_I && sliceType != SLICE_SI)
    {
        if (slcParams->ref_pic_list_reordering_flag_l0)
        {
            if (!params->UserFlags.bDisableAcceleratorRefPicListReordering)
            {
                ENCODE_CHK_STATUS_RETURN(SetRefPicListParam(params, 0));
            }

            PutBit(bsbuffer, slcParams->ref_pic_list_reordering_flag_l0);

            if (slcParams->ref_pic_list_reordering_flag_l0)
            {
                picOrder = &slcParams->PicOrder[0][0];
                do
                {
                    PutVLCCode(bsbuffer, picOrder->ReorderPicNumIDC);
                    if (picOrder->ReorderPicNumIDC == 0 ||
                        picOrder->ReorderPicNumIDC == 1)
                    {
                        PutVLCCode(bsbuffer, picOrder->DiffPicNumMinus1);
                    } else
                    if (picOrder->ReorderPicNumIDC == 2)
                    {
                        PutVLCCode(bsbuffer, picOrder->LongTermPicNum);
                    }
                } while ((picOrder++)->ReorderPicNumIDC != 3);
            }
        }
        else
        {
            PutBit(bsbuffer, slcParams->ref_pic_list_reordering_flag_l0);
        }
    }
    if (sliceType == SLICE_B)
    {
        if (slcParams->ref_pic_list_reordering_flag_l1)
        {
            if (!params->UserFlags.bDisableAcceleratorRefPicListReordering)
            {
                SetRefPicListParam(params, 1);
            }

            PutBit(bsbuffer, slcParams->ref_pic_list_reordering_flag_l1);

            if (slcParams->ref_pic_list_reordering_flag_l1)
            {
                picOrder = &slcParams->PicOrder[1][0];
                do
                {
                    PutVLCCode(bsbuffer, picOrder->ReorderPicNumIDC);
                    if (picOrder->ReorderPicNumIDC == 0 ||
                        picOrder->ReorderPicNumIDC == 1)
                    {
                        PutVLCCode(bsbuffer, picOrder->DiffPicNumMinus1);
                    } else
                    if (picOrder->ReorderPicNumIDC == 2)
                    {
                        PutVLCCode(bsbuffer, picOrder->PicNum);
                    }
                } while ((picOrder++)->ReorderPicNumIDC != 3);
            }
        }
        else
        {
            PutBit(bsbuffer, slcParams->ref_pic_list_reordering_flag_l1);
        }
    }

    return eStatus;
}

MOS_STATUS AvcEncodeHeaderPacker::PredWeightTable(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PBSBuffer                      bsbuffer;
    int16_t                        weight, offset, weight2, offset2;
    uint8_t                        i, weight_flag, chromaIDC;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);
    ENCODE_CHK_NULL_RETURN(params->pBsBuffer);

    bsbuffer  = params->pBsBuffer;
    slcParams = params->pAvcSliceParams;
    chromaIDC = params->pSeqParams->chroma_format_idc;

    PutVLCCode(bsbuffer, slcParams->luma_log2_weight_denom);

    if (chromaIDC)
    {
        PutVLCCode(bsbuffer, slcParams->chroma_log2_weight_denom);
    }

    for (i = 0; i <= slcParams->num_ref_idx_l0_active_minus1; i++)
    {
        // Luma
        weight      = slcParams->Weights[0][i][0][0];
        offset      = slcParams->Weights[0][i][0][1];
        weight_flag = (weight != (1 << slcParams->luma_log2_weight_denom)) || (offset != 0);
        PutBit(bsbuffer, weight_flag);
        if (weight_flag)
        {
            PutVLCCode(bsbuffer, SIGNED(weight));
            PutVLCCode(bsbuffer, SIGNED(offset));
        }

        // Chroma
        if (chromaIDC)
        {
            weight      = slcParams->Weights[0][i][1][0];
            offset      = slcParams->Weights[0][i][1][1];
            weight2     = slcParams->Weights[0][i][2][0];
            offset2     = slcParams->Weights[0][i][2][1];
            weight_flag = (weight != (1 << slcParams->chroma_log2_weight_denom)) ||
                          (weight2 != (1 << slcParams->chroma_log2_weight_denom)) ||
                          (offset != 0) || (offset2 != 0);
            PutBit(bsbuffer, weight_flag);
            if (weight_flag)
            {
                PutVLCCode(bsbuffer, SIGNED(weight));
                PutVLCCode(bsbuffer, SIGNED(offset));
                PutVLCCode(bsbuffer, SIGNED(weight2));
                PutVLCCode(bsbuffer, SIGNED(offset2));
            }
        }
    }

    if (Slice_Type[slcParams->slice_type] == SLICE_B)
    {
        for (i = 0; i <= slcParams->num_ref_idx_l1_active_minus1; i++)
        {
            // Luma
            weight      = slcParams->Weights[1][i][0][0];
            offset      = slcParams->Weights[1][i][0][1];
            weight_flag = (weight != (1 << slcParams->luma_log2_weight_denom)) || (offset != 0);
            PutBit(bsbuffer, weight_flag);
            if (weight_flag)
            {
                PutVLCCode(bsbuffer, SIGNED(weight));
                PutVLCCode(bsbuffer, SIGNED(offset));
            }

            // Chroma
            if (chromaIDC)
            {
                weight      = slcParams->Weights[1][i][1][0];
                offset      = slcParams->Weights[1][i][1][1];
                weight2     = slcParams->Weights[1][i][2][0];
                offset2     = slcParams->Weights[1][i][2][1];
                weight_flag = (weight != (1 << slcParams->chroma_log2_weight_denom)) ||
                              (weight2 != (1 << slcParams->chroma_log2_weight_denom)) ||
                              (offset != 0) || (offset2 != 0);
                PutBit(bsbuffer, weight_flag);
                if (weight_flag)
                {
                    PutVLCCode(bsbuffer, SIGNED(weight));
                    PutVLCCode(bsbuffer, SIGNED(offset));
                    PutVLCCode(bsbuffer, SIGNED(weight2));
                    PutVLCCode(bsbuffer, SIGNED(offset2));
                }
            }
        }
    }

    return eStatus;
}

MOS_STATUS AvcEncodeHeaderPacker::MMCO(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PBSBuffer                      bsbuffer;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);
    ENCODE_CHK_NULL_RETURN(params->pBsBuffer);

    bsbuffer  = params->pBsBuffer;
    slcParams = params->pAvcSliceParams;
    
    PCODEC_SLICE_MMCO mmco = &slcParams->MMCO[0];
    do
    {
        PutVLCCode(bsbuffer, mmco->MmcoIDC);
        if (mmco->MmcoIDC == 1 ||
            mmco->MmcoIDC == 3)
            PutVLCCode(bsbuffer, mmco->DiffPicNumMinus1);
        if (mmco->MmcoIDC == 2)
            PutVLCCode(bsbuffer, mmco->LongTermPicNum);
        if (mmco->MmcoIDC == 3 ||
            mmco->MmcoIDC == 6)
            PutVLCCode(bsbuffer, mmco->LongTermFrameIdx);
        if (mmco->MmcoIDC == 4)
            PutVLCCode(bsbuffer, mmco->MaxLongTermFrameIdxPlus1);
    } while ((mmco++)->MmcoIDC != 0);

    return eStatus;
}

void AvcEncodeHeaderPacker::SetInitialRefPicList(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PCODEC_REF_LIST *              refList;
    CODEC_PIC_REORDER *            picOrder, pTempPicOrder[32];
    CODEC_PICTURE                  picture;
    uint8_t                        i, j, botField;
    uint32_t                       picNum, picOC;
    uint8_t                        topIdx, botIdx, listSize;
    uint32_t                       defaultPicNumOrder[32];
    bool                           reorder;

    ENCODE_CHK_NULL_NO_STATUS_RETURN(params);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(params->pAvcSliceParams);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(params->ppRefList);

    slcParams = params->pAvcSliceParams;
    refList   = params->ppRefList;
    reorder   = false;
    topIdx    = 0;
    botIdx    = 0;
    listSize  = 0;

    if (params->wPictureCodingType == P_TYPE)
    {
        GetPicNum(params, 0);  // list 0
        picOrder = &slcParams->PicOrder[0][0];
        // Save the default pic order.
        for (i = 0; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
        {
            defaultPicNumOrder[i] = picOrder[i].PicNum;
        }
        for (i = 1; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
        {
            picNum  = picOrder[i].PicNum;
            picture = picOrder[i].Picture;
            picOC   = picOrder[i].POC;
            j       = i;
            while ((j > 0) && (picOrder[j - 1].PicNum < picNum))
            {
                picOrder[j].PicNum  = picOrder[j - 1].PicNum;
                picOrder[j].Picture = picOrder[j - 1].Picture;
                picOrder[j].POC     = picOrder[j - 1].POC;
                j--;
                reorder = true;
            }
            picOrder[j].PicNum  = picNum;
            picOrder[j].Picture = picture;
            picOrder[j].POC     = picOC;
        }

        // Sort picOrder[] based on polarity in field case
        if (CodecHal_PictureIsTopField(params->CurrPic))
        {
            while ((topIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1)) ||
                   (botIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1)))
            {
                for (; topIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1); topIdx++)
                {
                    if (CodecHal_PictureIsTopField(picOrder[topIdx].Picture))  //TOP_FIELD
                    {
                        pTempPicOrder[listSize].PicNum  = picOrder[topIdx].PicNum;
                        pTempPicOrder[listSize].Picture = picOrder[topIdx].Picture;
                        pTempPicOrder[listSize].POC     = picOrder[topIdx].POC;
                        listSize++;
                        topIdx++;
                        break;
                    }
                }
                for (; botIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1); botIdx++)
                {
                    if (CodecHal_PictureIsBottomField(picOrder[botIdx].Picture))  //BOTTOM_FIELD
                    {
                        pTempPicOrder[listSize].PicNum  = picOrder[botIdx].PicNum;
                        pTempPicOrder[listSize].Picture = picOrder[botIdx].Picture;
                        pTempPicOrder[listSize].POC     = picOrder[botIdx].POC;
                        listSize++;
                        botIdx++;
                        break;
                    }
                }
            }
        }
        if (CodecHal_PictureIsBottomField(params->CurrPic))
        {
            while ((topIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1)) ||
                   (botIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1)))
            {
                for (; botIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1); botIdx++)
                {
                    if (CodecHal_PictureIsBottomField(picOrder[botIdx].Picture))  //BOTTOM_FIELD
                    {
                        pTempPicOrder[listSize].PicNum  = picOrder[botIdx].PicNum;
                        pTempPicOrder[listSize].Picture = picOrder[botIdx].Picture;
                        pTempPicOrder[listSize].POC     = picOrder[botIdx].POC;
                        listSize++;
                        botIdx++;
                        break;
                    }
                }
                for (; topIdx < (slcParams->num_ref_idx_l0_active_minus1 + 1); topIdx++)
                {
                    if (CodecHal_PictureIsTopField(picOrder[topIdx].Picture))  //TOP_FIELD
                    {
                        pTempPicOrder[listSize].PicNum  = picOrder[topIdx].PicNum;
                        pTempPicOrder[listSize].Picture = picOrder[topIdx].Picture;
                        pTempPicOrder[listSize].POC     = picOrder[topIdx].POC;
                        listSize++;
                        topIdx++;
                        break;
                    }
                }
            }
        }

        if (!CodecHal_PictureIsFrame(params->CurrPic))
        {
            listSize = MOS_MIN(listSize, 32);
            // Copy temp array back to picOrder[]
            for (i = 0; i < listSize; i++)
            {
                picOrder[i].PicNum  = pTempPicOrder[i].PicNum;
                picOrder[i].Picture = pTempPicOrder[i].Picture;
                picOrder[i].POC     = pTempPicOrder[i].POC;
            }

            // Check if picOrder[] has been shuffled compared to the original list
            reorder = false;
            for (i = 0; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
            {
                if (defaultPicNumOrder[i] != picOrder[i].PicNum)
                {
                    reorder = true;
                    break;
                }
            }
        }

        if (reorder)
        {
            slcParams->ref_pic_list_reordering_flag_l0 = 1;
        }
        else
        {
            slcParams->ref_pic_list_reordering_flag_l0 = 0;
        }
        for (i = 0; i < (slcParams->num_ref_idx_l0_active_minus1 + 1); i++)
        {
            botField = (CodecHal_PictureIsBottomField(picOrder[i].Picture)) ? 1 : 0;
            refList[picOrder[i].Picture.FrameIdx]->ucInitialIdx[0][botField] = i;
        }
    }
    if (params->wPictureCodingType == B_TYPE)
    {
    }

    return;
}

MOS_STATUS AvcEncodeHeaderPacker::SetRefPicListParam(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params, uint8_t list)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PCODEC_REF_LIST *              refList;
    PCODEC_PIC_REORDER             picOrder;
    uint8_t                        i, j, idx, picIdx, numReorder, numActiveMinus1, refPolarity;
    uint32_t                       picNumPred, currPicNum, picNumNoWrap, maxPicNum;
    int16_t                        frameNum;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);
    ENCODE_CHK_NULL_RETURN(params->ppRefList);

    slcParams = params->pAvcSliceParams;
    refList   = params->ppRefList;
    frameNum  = refList[params->CurrReconPic.FrameIdx]->sFrameNumber;

    currPicNum = (CodecHal_PictureIsFrame(params->CurrPic)) ? frameNum : 2 * frameNum + 1;
    picNumPred = currPicNum;
    maxPicNum  = (CodecHal_PictureIsFrame(params->CurrPic)) ? slcParams->MaxFrameNum : (2 * slcParams->MaxFrameNum);

    numActiveMinus1 = list ? slcParams->num_ref_idx_l1_active_minus1 : slcParams->num_ref_idx_l0_active_minus1;

    picOrder = &slcParams->PicOrder[list][0];

    idx         = 0;
    picIdx      = picOrder[idx].Picture.FrameIdx;
    refPolarity = (CodecHal_PictureIsBottomField(picOrder[idx].Picture)) ? 1 : 0;
    if (refList[picIdx]->ucFinalIdx[list][refPolarity] ==
        refList[picIdx]->ucInitialIdx[list][refPolarity])
    {
        // Should never happen, something must be wrong in CodecHal_PackSliceHeader_SetInitialRefPicList()
        ENCODE_ASSERT(false);
        if (list)  // L1
        {
            slcParams->ref_pic_list_reordering_flag_l1 = 0;
        }
        else  // L0
        {
            slcParams->ref_pic_list_reordering_flag_l0 = 0;
        }
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    numReorder = refList[picIdx]->ucFinalIdx[list][refPolarity] - refList[picIdx]->ucInitialIdx[list][refPolarity];
    if (numReorder > numActiveMinus1)
    {
        numReorder = numActiveMinus1;
    }
    slcParams->NumReorder = numReorder;
    do
    {
        for (i = (idx + 1); i <= numActiveMinus1; i++)
        {
            picIdx      = picOrder[i].Picture.FrameIdx;
            refPolarity = (CodecHal_PictureIsBottomField(picOrder[i].Picture)) ? 1 : 0;
            if (refList[picIdx]->ucFinalIdx[list][refPolarity] == idx)
            {
                break;
            }
        }
        if (i == (numActiveMinus1 + 1))
        {
            // Should never happen, something must be wrong
            ENCODE_ASSERT(false);
            if (list)  // L1
            {
                slcParams->ref_pic_list_reordering_flag_l1 = 0;
            }
            else  // L0
            {
                slcParams->ref_pic_list_reordering_flag_l0 = 0;
            }
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }

        if (picOrder[i].PicNum > picNumPred)
        {
            picOrder[idx].ReorderPicNumIDC = 1;
        }
        else
        {
            picOrder[idx].ReorderPicNumIDC = 0;
        }

        if (picOrder[i].PicNum > currPicNum)
        {
            picNumNoWrap = picOrder[i].PicNum + maxPicNum;
        }
        else
        {
            picNumNoWrap = picOrder[i].PicNum;
        }

        if (picOrder[idx].ReorderPicNumIDC == 0)
        {
            if (picNumPred > picNumNoWrap)
            {
                picOrder[idx].DiffPicNumMinus1 = picNumPred - picNumNoWrap - 1;
            }
            else
            {
                picOrder[idx].DiffPicNumMinus1 = picNumPred + maxPicNum - picNumNoWrap - 1;
            }
        }
        else
        {
            if (picNumNoWrap > picNumPred)
            {
                picOrder[idx].DiffPicNumMinus1 = picNumNoWrap - picNumPred - 1;
            }
            else
            {
                picOrder[idx].DiffPicNumMinus1 = picNumNoWrap + maxPicNum - picNumPred - 1;
            }
        }
        picNumPred = picNumNoWrap;

        for (j = i; j > idx; j--)
        {
            picOrder[j].Picture = picOrder[j - 1].Picture;
            picOrder[j].PicNum  = picOrder[j - 1].PicNum;
            picOrder[j].POC     = picOrder[j - 1].POC;
        }

        idx++;
    } while (--numReorder);
    picOrder[idx].ReorderPicNumIDC = 3;

    return eStatus;
}

void AvcEncodeHeaderPacker::GetPicNum(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params,uint8_t list)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams;
    PCODEC_REF_LIST *              refList;
    uint32_t                       frameNum, frameNumWrap, picNum;
    uint8_t                        i, botField, size;
    CODEC_PICTURE                  picture;

    ENCODE_CHK_NULL_NO_STATUS_RETURN(params);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(params->pAvcSliceParams);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(params->ppRefList);

    slcParams = params->pAvcSliceParams;
    refList   = params->ppRefList;

    size = list ? (slcParams->num_ref_idx_l1_active_minus1 + 1) : (slcParams->num_ref_idx_l0_active_minus1 + 1);

    for (i = 0; i < size; i++)
    {
        picture                                               = slcParams->PicOrder[list][i].Picture;
        botField                                              = (CodecHal_PictureIsBottomField(picture)) ? 1 : 0;
        refList[picture.FrameIdx]->ucFinalIdx[list][botField] = i;
        frameNum                                              = refList[picture.FrameIdx]->sFrameNumber;
        if (frameNum > (uint32_t)refList[params->CurrReconPic.FrameIdx]->sFrameNumber)
        {
            frameNumWrap = frameNum - slcParams->MaxFrameNum;
        }
        else
        {
            frameNumWrap = frameNum;
        }

        if (CodecHal_PictureIsFrame(params->CurrPic))
        {
            picNum = frameNumWrap;
        }
        else if ((CodecHal_PictureIsTopField(params->CurrPic) &&
                     CodecHal_PictureIsTopField(picture)) ||
                 (CodecHal_PictureIsBottomField(params->CurrPic) &&
                     CodecHal_PictureIsBottomField(picture)))
        {
            // Same polarity
            picNum = (frameNumWrap << 1) + 1;
        }
        else
        {
            picNum = frameNumWrap << 1;
        }
        slcParams->PicOrder[list][i].PicNum = picNum;
        slcParams->PicOrder[list][i].POC =
            refList[picture.FrameIdx]->iFieldOrderCnt[botField];
    }

    return;
}

}  // namespace encode
