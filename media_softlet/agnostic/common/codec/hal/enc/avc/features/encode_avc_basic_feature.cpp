/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     encode_avc_basic_feature.cpp
//! \brief    Defines the common interface for encode avc parameter
//!

#include "encode_avc_basic_feature.h"
#include "encode_avc_header_packer.h"
#include "encode_avc_vdenc_const_settings.h"
#include "mos_os_cp_interface_specific.h"

namespace encode
{
AvcBasicFeature::~AvcBasicFeature()
{
    if (m_colocatedMVBufferForIFrames) {
        m_allocator->DestroyResource(m_colocatedMVBufferForIFrames);
        m_colocatedMVBufferForIFrames = nullptr;
    }
}

MOS_STATUS AvcBasicFeature::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::Init(setting));

    ENCODE_CHK_STATUS_RETURN(InitRefFrames());

    MediaUserSetting::Value outValue;

    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "Encode Suppress Recon Pic",
        MediaUserSetting::Group::Sequence);

    m_suppressReconPicSupported = (outValue.Get<int32_t>()) ? true : false;

    outValue = 0;

    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "AVC Encode Adaptive Rounding Inter Enable",
        MediaUserSetting::Group::Sequence);

    m_adaptiveRoundingInterEnable = (outValue.Get<int32_t>()) ? true : false;

    m_targetUsageOverride = (uint8_t)0;

    m_brcAdaptiveRegionBoostSupported = true;

#if (_DEBUG || _RELEASE_INTERNAL)

    outValue = 0;

    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "Encode TU Override",
        MediaUserSetting::Group::Sequence);

    m_targetUsageOverride = uint8_t(outValue.Get<uint32_t>());

    outValue = 0;

    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "AVC VDEnc PerMB StreamOut Enable",
        MediaUserSetting::Group::Sequence);

    m_perMBStreamOutEnable = (outValue.Get<int32_t>()) ? true : false;

    outValue = 0;

    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "AVC VDEnc TCBRC ARB Disable",
        MediaUserSetting::Group::Sequence);

    m_brcAdaptiveRegionBoostSupported = (outValue.Get<int32_t>()) ? false : m_brcAdaptiveRegionBoostSupported;

#endif  // _DEBUG || _RELEASE_INTERNAL

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcBasicFeature::InitRefFrames()
{
    ENCODE_FUNC_CALL();

    m_ref = std::make_shared<AvcReferenceFrames>();
    ENCODE_CHK_NULL_RETURN(m_ref);

    ENCODE_CHK_STATUS_RETURN(m_ref->Init(this, m_allocator));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcBasicFeature::Update(void *params)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::Update(params));

    EncoderParams* encodeParams = (EncoderParams*)params;
    ENCODE_CHK_NULL_RETURN(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(encodeParams->pSliceParams);
    ENCODE_CHK_NULL_RETURN(encodeParams->pVuiParams);
    ENCODE_CHK_NULL_RETURN(encodeParams->pIQMatrixBuffer);
    ENCODE_CHK_NULL_RETURN(encodeParams->pIQWeightScaleLists);
    ENCODE_CHK_NULL_RETURN(encodeParams->ppNALUnitParams);

    auto ppsidx = ((PCODEC_AVC_ENCODE_SLICE_PARAMS)(encodeParams->pSliceParams))->pic_parameter_set_id;
    auto spsidx = ((PCODEC_AVC_ENCODE_PIC_PARAMS)(encodeParams->pPicParams))->seq_parameter_set_id;
    if (ppsidx >= CODEC_AVC_MAX_PPS_NUM || spsidx >= CODEC_AVC_MAX_SPS_NUM)
    {
        //should never happen
        ENCODE_ASSERTMESSAGE("Invalid ppsidx/spsidx");
        return MOS_STATUS_UNKNOWN;
    }

    m_seqParam = m_seqParams[spsidx] = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(encodeParams->pSeqParams);
    m_picParam = m_picParams[ppsidx] = (PCODEC_AVC_ENCODE_PIC_PARAMS)(encodeParams->pPicParams);
    m_vuiParams                      = (PCODECHAL_ENCODE_AVC_VUI_PARAMS)encodeParams->pVuiParams;
    m_sliceParams                    = (PCODEC_AVC_ENCODE_SLICE_PARAMS)encodeParams->pSliceParams;
    m_iqMatrixParams                 = (PCODEC_AVC_IQ_MATRIX_PARAMS)encodeParams->pIQMatrixBuffer;
    m_iqWeightScaleLists             = (PCODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS)encodeParams->pIQWeightScaleLists;
    m_nalUnitParams                  = encodeParams->ppNALUnitParams;
    m_sliceStructCaps                = encodeParams->uiSlcStructCaps;
    m_madEnabled                     = encodeParams->bMADEnabled;
    m_acceleratorHeaderPackingCaps   = encodeParams->bAcceleratorHeaderPackingCaps;
    m_skipFrameFlag                  = m_picParam->SkipFrameFlag;

    if (encodeParams->pSeiData != nullptr)
    {
        ENCODE_CHK_STATUS_RETURN(UpdateSeiParameters(encodeParams));
    }

    m_deblockingEnabled = 0;
    for (uint32_t i = 0; i < m_numSlices; i++)
    {
        if (m_sliceParams[i].disable_deblocking_filter_idc != 1)
        {
            m_deblockingEnabled = 1;
            break;
        }
    }

    if (m_newSeq)
    {
        ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
    }

    ENCODE_CHK_STATUS_RETURN(SetPictureStructs());
    ENCODE_CHK_STATUS_RETURN(SetSliceStructs());

    m_useRawForRef   = m_userFlags.bUseRawPicForRef;

    // Set min/max QP values in AVC State based on frame type if atleast one of them is non-zero
    if (m_picParam->ucMinimumQP || m_picParam->ucMaximumQP)
    {
        UpdateMinMaxQp();
    }

    if (m_codecFunction != CODECHAL_FUNCTION_ENC && !m_userFlags.bDisableAcceleratorHeaderPacking && m_acceleratorHeaderPackingCaps)
    {
        ENCODE_CHK_STATUS_RETURN(PackPictureHeader());
    }

    CheckResolutionChange();

    if (m_resolutionChanged)
    {
        ENCODE_CHK_STATUS_RETURN(UpdateTrackedBufferParameters());
    }

    ENCODE_CHK_STATUS_RETURN(GetTrackedBuffers());

    m_brcAdaptiveRegionBoostEnabled = m_brcAdaptiveRegionBoostSupported && (m_picParam->TargetFrameSize != 0) && m_lookaheadDepth == 0;

    // HW limitation, skip block count for B frame must be acquired in GetAvcVdencMBLevelStatusExt
    if (m_picParam->StatusReportEnable.fields.BlockStats ||
        (m_picParam->StatusReportEnable.fields.FrameStats && m_picParam->CodingType == B_TYPE))
    {
        m_perMBStreamOutEnable = true;
    }

#if MHW_HWCMDPARSER_ENABLED
    char frameType = '\0';
    switch (m_picParam->CodingType)
    {
    case I_TYPE:
        frameType = 'I';
        break;
    case P_TYPE:
        frameType = 'P';
        break;
    case B_TYPE:
        frameType = (m_picParam->RefPicFlag) ? 'B' : 'b';
        break;
    }

    auto instance = mhw::HwcmdParser::GetInstance();
    if (instance)
    {
        instance->Update(frameType, (void *)m_featureManager);
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcBasicFeature::SetSequenceStructs()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    auto seqParams = m_seqParam;

    // For G12+ TCBRC is used instead of LowDelayBRC, also needs TargetFrameSize in PPS.
    m_forcedTCBRC = false;
    if (seqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW && !seqParams->LookaheadDepth)
    {
        ENCODE_NORMALMESSAGE("LDBRC switched to TCBRC\n");
        m_forcedTCBRC                 = true;
        seqParams->FrameSizeTolerance = EFRAMESIZETOL_NORMAL;
        seqParams->MBBRC              = mbBrcDisabled;  // no need with ARB
    }

    if (m_targetUsageOverride)
    {
        seqParams->TargetUsage = m_targetUsageOverride;
    }

    // seq_scaling_matrix_present_flag and chroma_format_idc
    // shall not be present for main profile
    if (seqParams->Profile == CODEC_AVC_MAIN_PROFILE)
    {
        seqParams->seq_scaling_matrix_present_flag = 0;
        for (uint8_t i = 0; i < 12; i++)
        {
            seqParams->seq_scaling_list_present_flag[i] = 0;
        }
        seqParams->chroma_format_idc = 1;
    }
    // high profile chroma_format_idc in the range of 0 to 1 inclusive
    if (seqParams->chroma_format_idc > 1)
    {
        seqParams->chroma_format_idc = 1;
    }

    // main & high profile support only 8bpp
    seqParams->bit_depth_luma_minus8   = 0;
    seqParams->bit_depth_chroma_minus8 = 0;

    // setup parameters corresponding to H264 bit stream definition
    seqParams->pic_height_in_map_units_minus1       = seqParams->frame_mbs_only_flag ? CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(seqParams->FrameHeight) - 1 : (CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(seqParams->FrameHeight) + 1) / 2 - 1;
    seqParams->pic_width_in_mbs_minus1              = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(seqParams->FrameWidth) - 1;
    seqParams->constraint_set0_flag                 = 0;
    seqParams->constraint_set1_flag                 = (seqParams->Profile == CODEC_AVC_BASE_PROFILE) ? 1 : 0;
    seqParams->constraint_set2_flag                 = 0;
    seqParams->constraint_set3_flag                 = 0;
    seqParams->gaps_in_frame_num_value_allowed_flag = 0;
    seqParams->qpprime_y_zero_transform_bypass_flag = 0;
    seqParams->separate_colour_plane_flag           = 0;

    // setup internal parameters
    m_picWidthInMb  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(seqParams->FrameWidth);
    m_picHeightInMb = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(seqParams->FrameHeight);
    m_frameWidth    = m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH;
    m_frameHeight   = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;

    m_downscaledWidth4x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x) * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x = ((CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x) + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
    m_downscaledHeight4x = MOS_ALIGN_CEIL(m_downscaledHeight4x, MOS_YTILE_H_ALIGNMENT) << 1;

    m_targetUsage = seqParams->TargetUsage & 0x7;

    if (!seqParams->frame_cropping_flag)
    {
        // do cropping only when the picture dimension is not MB aligned...
        seqParams->frame_crop_left_offset = 0;
        seqParams->frame_crop_top_offset  = 0;

        if (m_frameWidth != seqParams->FrameWidth ||
            m_frameHeight != seqParams->FrameHeight)
        {
            seqParams->frame_cropping_flag      = 1;
            seqParams->frame_crop_right_offset  = (uint16_t)((m_frameWidth - seqParams->FrameWidth) >> 1);                                       // 4:2:0
            seqParams->frame_crop_bottom_offset = (uint16_t)((m_frameHeight - seqParams->FrameHeight) >> (2 - seqParams->frame_mbs_only_flag));  // 4:2:0
        }
        else
        {
            seqParams->frame_cropping_flag      = 0;
            seqParams->frame_crop_right_offset  = 0;
            seqParams->frame_crop_bottom_offset = 0;
        }
    }

    // App does tail insertion in VDEnc dynamic slice non-CP case
    m_vdencNoTailInsertion =
        seqParams->EnableSliceLevelRateCtrl &&
        (!m_osInterface->osCpInterface->IsCpEnabled());

    m_maxNumSlicesAllowed = GetMaxNumSlicesAllowed(
        (CODEC_AVC_PROFILE_IDC)(seqParams->Profile),
        (CODEC_AVC_LEVEL_IDC)(seqParams->Level),
        seqParams->FramesPer100Sec);

    // Lookahead setup
    m_lookaheadDepth = seqParams->LookaheadDepth;
    if (m_lookaheadDepth > 0)
    {
        uint64_t targetBitRate = (uint64_t)seqParams->TargetBitRate;
        if ((seqParams->FramesPer100Sec < 100) || ((targetBitRate * 100) < seqParams->FramesPer100Sec))
        {
            ENCODE_ASSERTMESSAGE("Invalid FrameRate or TargetBitRate in lookahead pass!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        m_averageFrameSize = (uint32_t)(targetBitRate * 100 / seqParams->FramesPer100Sec);

        if (seqParams->VBVBufferSizeInBit < seqParams->InitVBVBufferFullnessInBit)
        {
            ENCODE_ASSERTMESSAGE("VBVBufferSizeInBit is less than InitVBVBufferFullnessInBit\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_targetBufferFulness == 0)
        {
            m_targetBufferFulness = seqParams->VBVBufferSizeInBit - seqParams->InitVBVBufferFullnessInBit;
        }
    }

    m_bitDepth     = m_seqParam->bit_depth_luma_minus8 + 8;
    m_chromaFormat = m_seqParam->chroma_format_idc;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcBasicFeature::SetPictureStructs()
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto picParams = m_picParam;
    auto seqParams = m_seqParam;
    auto slcParams = m_sliceParams;

    // TCBRC forced from LowDelayBRC also needs TargetFrameSize
    if (m_forcedTCBRC)
    {
        picParams->TargetFrameSize = uint32_t(seqParams->TargetBitRate * (100. / 8) / seqParams->FramesPer100Sec);
    }

    m_userFlags       = picParams->UserFlags;
    m_nalUnitType     = picParams->bIdrPic ? CODECHAL_ENCODE_AVC_NAL_UT_IDR_SLICE : CODECHAL_ENCODE_AVC_NAL_UT_SLICE;
    m_lastPicInSeq    = picParams->bLastPicInSeq;
    m_lastPicInStream = picParams->bLastPicInStream;

    ENCODE_CHK_STATUS_RETURN(m_ref->UpdatePicture());
    m_pictureCodingType = m_ref->GetPictureCodingType();

    auto prevPic    = m_currOriginalPic;
    auto prevIdx    = prevPic.FrameIdx;
    auto currPic    = picParams->CurrOriginalPic;
    auto currIdx    = currPic.FrameIdx;

    m_currOriginalPic        = currPic;
    m_currReconstructedPic   = picParams->CurrReconstructedPic;

    m_prevReconFrameIdx = m_currReconFrameIdx;
    m_currReconFrameIdx = picParams->CurrReconstructedPic.FrameIdx;

    if (Mos_ResourceIsNull(&m_reconSurface.OsResource) &&
        (!picParams->UserFlags.bUseRawPicForRef || m_codecFunction != CODECHAL_FUNCTION_ENC))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (seqParams->Profile == CODEC_AVC_MAIN_PROFILE || seqParams->Profile == CODEC_AVC_BASE_PROFILE)
    {
        picParams->transform_8x8_mode_flag         = 0;
        picParams->pic_scaling_matrix_present_flag = 0;
        for (uint8_t i = 0; i < 12; i++)
        {
            picParams->pic_scaling_list_present_flag[i] = 0;
        }
        picParams->second_chroma_qp_index_offset = picParams->chroma_qp_index_offset;
    }
    if (picParams->QpY < 0)
    {
        picParams->QpY = 25;  // Set to default, recommended value used in simulation.
    }
    else if (picParams->QpY > CODECHAL_ENCODE_AVC_MAX_SLICE_QP)
    {
        picParams->QpY = CODECHAL_ENCODE_AVC_MAX_SLICE_QP;  // Crop to 51 if larger
    }
    picParams->pic_init_qp_minus26 = picParams->QpY - 26;

    if (!seqParams->seq_scaling_matrix_present_flag)
    {
        if (!picParams->pic_scaling_matrix_present_flag)
            ScalingListFlat();
        else if (!picParams->pic_scaling_list_present_flag[0])
            ScalingListFallbackRuleA();
    }
    else if (!seqParams->seq_scaling_list_present_flag[0] &&
             !picParams->pic_scaling_list_present_flag[0])
    {  // fall-back rule A
        ScalingListFallbackRuleA();
    }

    picParams->num_slice_groups_minus1                = 0;  // do not support flexible MB ordering
    picParams->deblocking_filter_control_present_flag = 1;  // always set to 1
    picParams->redundant_pic_cnt_present_flag         = 0;
    picParams->pic_init_qs_minus26                    = 0;  // not used

    // App handles tail insertion for VDEnc dynamic slice in non-cp case
    if (m_vdencNoTailInsertion)
    {
        m_lastPicInSeq = m_lastPicInStream = 0;
    }

    m_frameFieldHeight     = m_frameHeight;
    m_frameFieldHeightInMb = m_picHeightInMb;
    m_firstField           = 1;

    seqParams->mb_adaptive_frame_field_flag = 0;
    m_mbaffEnabled                          = 0;

    if ((m_lookaheadDepth > 0) && (m_prevTargetFrameSize > 0))
    {
        int64_t targetBufferFulness = (int64_t)m_targetBufferFulness;
        targetBufferFulness += (((int64_t)m_prevTargetFrameSize) << 3) - (int64_t)m_averageFrameSize;
        m_targetBufferFulness = (uint32_t)MOS_CLAMP_MIN_MAX(targetBufferFulness, 0, 0xFFFFFFFF);
    }
    m_prevTargetFrameSize = picParams->TargetFrameSize;

    if (picParams->FieldCodingFlag || picParams->FieldFrameCodingFlag)
    {
        ENCODE_ASSERTMESSAGE("VDEnc does not support interlaced picture\n");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS AvcBasicFeature::SetSliceStructs()
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_STATUS_RETURN(m_ref->UpdateSlice());

    auto slcParams = m_sliceParams;
    auto seqParams = m_seqParam;
    auto picParams = m_picParam;

    uint32_t numMbsInPrevSlice = slcParams->NumMbsForSlice;  // Initiailize to num mbs in first slice
    uint32_t numMbsForFirstSlice;
    uint32_t numMbs = 0;

    for (uint32_t sliceCount = 0; sliceCount < m_numSlices; sliceCount++)
    {
        if (m_sliceStructCaps != CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE)
        {
            if (sliceCount == 0)
            {
                numMbsForFirstSlice = slcParams->NumMbsForSlice;
                if (numMbsForFirstSlice % m_picWidthInMb)
                {
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    return eStatus;
                }
                m_sliceHeight = numMbsForFirstSlice / m_picWidthInMb;
                if (m_sliceStructCaps == CODECHAL_SLICE_STRUCT_POW2ROWS && (m_sliceHeight & (m_sliceHeight - 1)))
                {
                    // app can only pass orig numMBs in picture for single slice, set slice height to the nearest pow2
                    if (m_numSlices == 1)
                    {
                        uint16_t sliceHeightPow2 = 1;
                        while (sliceHeightPow2 < m_sliceHeight)
                        {
                            sliceHeightPow2 <<= 1;
                        }
                        m_sliceHeight = sliceHeightPow2;
                    }
                    else
                    {
                        eStatus = MOS_STATUS_INVALID_PARAMETER;
                        return eStatus;
                    }
                }
            }
            else if (m_sliceStructCaps == CODECHAL_SLICE_STRUCT_ROWSLICE)
            {
                if ((sliceCount < m_numSlices - 1 && numMbsForFirstSlice != slcParams->NumMbsForSlice) ||
                    (sliceCount == m_numSlices - 1 && numMbsForFirstSlice < slcParams->NumMbsForSlice))
                {
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    return eStatus;
                }
            }

            // Gaps between slices are not allowed
            if (slcParams->first_mb_in_slice != numMbs)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
            numMbs += slcParams->NumMbsForSlice;
        }
        else  // SLICE_STRUCT_ARBITRARYMBSLICE
        {
            if ((slcParams->NumMbsForSlice % m_picWidthInMb) ||                                          // If slice is partial MB row,
                ((sliceCount < m_numSlices - 1) && (numMbsInPrevSlice != slcParams->NumMbsForSlice)) ||  // OR not the last slice and num mbs is not same as prev slice
                ((sliceCount == m_numSlices - 1) && ((numMbsInPrevSlice < slcParams->NumMbsForSlice))))  // OR it is the last slice and num mbs is not less than prev slice
            {
                m_sliceHeight            = 1;  // Slice height doesn't matter if using slicemap just set to any non-zero value.
            }
            else if ((m_numSlices == 1) || (sliceCount == 0))
            {
                m_sliceHeight            = slcParams->NumMbsForSlice / m_picWidthInMb;
            }
            numMbsInPrevSlice = slcParams->NumMbsForSlice;
        }

        if ((picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta) > CODECHAL_ENCODE_AVC_MAX_SLICE_QP)
        {
            slcParams->slice_qp_delta = CODECHAL_ENCODE_AVC_MAX_SLICE_QP - (picParams->pic_init_qp_minus26 + 26);
        }
        slcParams->redundant_pic_cnt                  = 0;
        slcParams->sp_for_switch_flag                 = 0;
        slcParams->slice_qs_delta                     = 0;
        slcParams->ref_pic_list_reordering_flag_l1    = 0;
        slcParams->no_output_of_prior_pics_flag       = 0;
        slcParams->redundant_pic_cnt                  = 0;

        slcParams->MaxFrameNum       = 1 << (seqParams[picParams->seq_parameter_set_id].log2_max_frame_num_minus4 + 4);
        slcParams->frame_num         = picParams->frame_num;
        slcParams->field_pic_flag    = picParams->FieldCodingFlag;
        slcParams->bottom_field_flag = (CodecHal_PictureIsBottomField(picParams->CurrOriginalPic)) ? 1 : 0;

        slcParams++;
    }

    return MOS_STATUS_SUCCESS;
}

void AvcBasicFeature::CheckResolutionChange()
{
    ENCODE_FUNC_CALL();

    uint32_t frameWidth = m_seqParam->FrameWidth;
    uint32_t frameHeight = m_seqParam->FrameHeight;
    uint16_t frame_crop_bottom_offset = m_seqParam->frame_crop_bottom_offset;
    uint16_t frame_mbs_only_flag      = m_seqParam->frame_mbs_only_flag;
    uint16_t frame_cropping_flag      = m_seqParam->frame_cropping_flag;

    // Only for first frame
    if (m_frameNum == 0)
    {
        m_oriFrameHeight = frameHeight;
        m_oriFrameWidth = frameWidth;
        m_frame_crop_bottom_offset = frame_crop_bottom_offset;
        m_frame_mbs_only_flag        = frame_mbs_only_flag;
        m_frame_cropping_flag      = frame_cropping_flag;
        m_resolutionChanged = true;
    }
    else
    {
        // check if there is a dynamic resolution change
        if ((m_oriFrameHeight && (m_oriFrameHeight != frameHeight)) ||
            (m_oriFrameWidth && (m_oriFrameWidth != frameWidth)))
        {
            m_resolutionChanged = true;
            m_oriFrameHeight = frameHeight;
            m_oriFrameWidth = frameWidth;
            m_frame_crop_bottom_offset = frame_crop_bottom_offset;
            m_frame_mbs_only_flag      = frame_mbs_only_flag;
            m_frame_cropping_flag      = frame_cropping_flag;
        }
        else
        {
            m_resolutionChanged = false;
        }
    }
}

MOS_STATUS AvcBasicFeature::PackPictureHeader()
{
    ENCODE_FUNC_CALL();

    CODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS packPicHeaderParams;
    packPicHeaderParams.pBsBuffer          = &m_bsBuffer;
    packPicHeaderParams.pPicParams         = m_picParam;
    packPicHeaderParams.pSeqParams         = m_seqParam;
    packPicHeaderParams.pAvcVuiParams      = m_vuiParams;
    packPicHeaderParams.pAvcIQMatrixParams = m_iqMatrixParams;
    packPicHeaderParams.ppNALUnitParams    = m_nalUnitParams;
    packPicHeaderParams.pSeiData           = &m_seiData;
    packPicHeaderParams.dwFrameHeight      = m_frameHeight;
    packPicHeaderParams.dwOriFrameHeight   = m_oriFrameHeight;
    packPicHeaderParams.wPictureCodingType = m_picParam->CodingType;
    packPicHeaderParams.bNewSeq            = m_newSeq;
    packPicHeaderParams.pbNewPPSHeader     = &m_newPpsHeader;
    packPicHeaderParams.pbNewSeqHeader     = &m_newSeqHeader;

    ENCODE_CHK_STATUS_RETURN(AvcEncodeHeaderPacker::PackPictureHeader(&packPicHeaderParams));

    return MOS_STATUS_SUCCESS;
}

bool AvcBasicFeature::InputSurfaceNeedsExtraCopy(const MOS_SURFACE &input)
{
#if _DEBUG || _RELEASE_INTERNAL
    static int8_t supported = -1;

    if (supported == -1)
    {
        MediaUserSetting::Value outValue{};

        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            "DisableInputSurfaceCopy",
            MediaUserSetting::Group::Sequence);

        supported = !outValue.Get<bool>();
    }

    if (!supported)
    {
        return false;
    }
#endif

    if (m_osInterface->osCpInterface && m_osInterface->osCpInterface->IsCpEnabled())
    {
        return false;
    }

    uint32_t alignedSize = 0;
    switch (input.Format)
    {
    case Format_NV12:
        alignedSize = MOS_MAX((uint32_t)m_seqParam->FrameWidth, (uint32_t)input.dwPitch) * m_seqParam->FrameHeight * 3 / 2;
        break;
    case Format_A8R8G8B8:
        alignedSize = MOS_MAX((uint32_t)m_seqParam->FrameWidth * 4, (uint32_t)input.dwPitch) * m_seqParam->FrameHeight;
        break;
    default:
        alignedSize = 0;
        break;
    }

    return input.dwSize < alignedSize;
}

MOS_STATUS AvcBasicFeature::UpdateTrackedBufferParameters()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);

    ENCODE_CHK_STATUS_RETURN(m_trackedBuf->OnSizeChange());

    uint32_t fieldNumMBs = (uint32_t)m_picWidthInMb * ((m_picHeightInMb + 1) >> 1);

    m_mbCodeSize = MOS_ALIGN_CEIL(fieldNumMBs * 16 * 4, CODECHAL_PAGE_SIZE) + fieldNumMBs * 16 * 4;
    m_mvDataSize = 0;

    uint32_t widthInMbRoundUp  = (m_frameWidth + 31) >> 4;
    uint32_t heightInMbRoundUp = (m_frameHeight + 15) >> 4;
    m_colocatedMVBufferSize    = widthInMbRoundUp * heightInMbRoundUp * CODECHAL_CACHELINE_SIZE / 2; // Cacheline per 2 MBs

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format = Format_Buffer;

    if (m_colocatedMVBufferSize > 0)
    {
        allocParams.dwBytes = m_colocatedMVBufferSize;
        allocParams.pBufName = "mvTemporalBuffer";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::mvTemporalBuffer, allocParams));

        if (m_colocatedMVBufferForIFrames) {
            m_allocator->DestroyResource(m_colocatedMVBufferForIFrames);
            m_colocatedMVBufferForIFrames = nullptr;
        }
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_colocatedMVBufferForIFrames = m_allocator->AllocateResource(allocParams, true);
        ENCODE_CHK_NULL_RETURN(m_colocatedMVBufferForIFrames);
        uint8_t* pData = (uint8_t*)m_allocator->LockResourceForWrite(m_colocatedMVBufferForIFrames);
        ENCODE_CHK_NULL_RETURN(pData);

        uint32_t* pT = (uint32_t*)(pData)+7;
        for (uint32_t j = 0; j < m_picHeightInMb; j++)
        {
            for (uint32_t i = 0; i < m_picWidthInMb; i++)
            {
                *pT = 0x4000; pT += 8;
            }
        }
        m_allocator->UnLock(m_colocatedMVBufferForIFrames);
    }

    // Input surface copy is needed if height is not MB aligned
    if (InputSurfaceNeedsExtraCopy(m_rawSurface))
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForAlignedRawSurface{};
        allocParamsForAlignedRawSurface.Type     = MOS_GFXRES_2D;
        allocParamsForAlignedRawSurface.Format   = m_rawSurface.Format;
        allocParamsForAlignedRawSurface.TileType = MOS_TILE_Y;
        allocParamsForAlignedRawSurface.dwWidth  = m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH;
        allocParamsForAlignedRawSurface.dwHeight = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;
        allocParamsForAlignedRawSurface.pBufName = "Aligned Raw Surface";
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(BufferType::AlignedRawSurface, allocParamsForAlignedRawSurface));
    }

    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::UpdateTrackedBufferParameters());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcBasicFeature::UpdateSeiParameters(EncoderParams* params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (params->pSeiData->dwSEIBufSize > 0)  // sei is present
    {
        if (params->pSeiData->dwSEIBufSize > m_seiData.dwSEIBufSize)
        {
            // Destroy and re-allocate
            if (m_seiData.pSEIBuffer)
            {
                MOS_FreeMemory(m_seiData.pSEIBuffer);
                m_seiData.pSEIBuffer = nullptr;
            }
            m_seiData.dwSEIBufSize = params->pSeiData->dwSEIBufSize;
            m_seiData.pSEIBuffer   = (uint8_t *)MOS_AllocAndZeroMemory(m_seiData.dwSEIBufSize);
            ENCODE_CHK_NULL_RETURN(m_seiData.pSEIBuffer);
        }

        m_seiParamBuffer        = params->pSeiParamBuffer;
        m_seiDataOffset         = params->dwSEIDataOffset;
        m_seiData.newSEIData    = params->pSeiData->newSEIData;
        m_seiData.dwSEIDataSize = params->pSeiData->dwSEIDataSize;

        eStatus = MOS_SecureMemcpy(m_seiData.pSEIBuffer,
            m_seiData.dwSEIDataSize,
            (m_seiParamBuffer + m_seiDataOffset),
            m_seiData.dwSEIDataSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    m_extraPictureStatesSize = m_seiData.dwSEIDataSize;

    return eStatus;
}

void AvcBasicFeature::UpdateMinMaxQp()
{
    ENCODE_FUNC_CALL();

    m_minMaxQpControlEnabled = true;
    if (m_picParam->CodingType == I_TYPE)
    {
        m_iMaxQp = MOS_MIN(MOS_MAX(m_picParam->ucMaximumQP, 10), 51);        // Clamp maxQP to [10, 51]
        m_iMinQp = MOS_MIN(MOS_MAX(m_picParam->ucMinimumQP, 10), m_iMaxQp);  // Clamp minQP to [10, maxQP] to make sure minQP <= maxQP
        if (!m_pFrameMinMaxQpControl)
        {
            m_pMinQp = m_iMinQp;
            m_pMaxQp = m_iMaxQp;
        }
        if (!m_bFrameMinMaxQpControl)
        {
            m_bMinQp = m_iMinQp;
            m_bMaxQp = m_iMaxQp;
        }
    }
    else if (m_picParam->CodingType == P_TYPE)
    {
        m_pFrameMinMaxQpControl = true;
        m_pMaxQp                = MOS_MIN(MOS_MAX(m_picParam->ucMaximumQP, 10), 51);        // Clamp maxQP to [10, 51]
        m_pMinQp                = MOS_MIN(MOS_MAX(m_picParam->ucMinimumQP, 10), m_pMaxQp);  // Clamp minQP to [10, maxQP] to make sure minQP <= maxQP
        if (!m_bFrameMinMaxQpControl)
        {
            m_bMinQp = m_pMinQp;
            m_bMaxQp = m_pMaxQp;
        }
    }
    else  // B_TYPE
    {
        m_bFrameMinMaxQpControl = true;
        m_bMaxQp                = MOS_MIN(MOS_MAX(m_picParam->ucMaximumQP, 10), 51);        // Clamp maxQP to [10, 51]
        m_bMinQp                = MOS_MIN(MOS_MAX(m_picParam->ucMinimumQP, 10), m_bMaxQp);  // Clamp minQP to [10, maxQP] to make sure minQP <= maxQP
    }
    // Zero out the QP values, so we don't update the AVCState settings until new values are sent in MiscParamsRC
    m_picParam->ucMinimumQP = 0;
    m_picParam->ucMaximumQP = 0;
}

int32_t AvcBasicFeature::GetMaxMBPS(uint8_t levelIdc)
{
    int maxMBPS = 11880;

    // See JVT Spec Annex A Table A-1 Level limits for below mapping
    // maxMBPS is in MB/s
    switch (levelIdc)
    {
    case CODEC_AVC_LEVEL_1:
    case CODEC_AVC_LEVEL_1b:
        maxMBPS = 1485;
        break;
    case CODEC_AVC_LEVEL_11:
        maxMBPS = 3000;
        break;
    case CODEC_AVC_LEVEL_12:
        maxMBPS = 6000;
        break;
    case CODEC_AVC_LEVEL_13:
    case CODEC_AVC_LEVEL_2:
        maxMBPS = 11880;
        break;
    case CODEC_AVC_LEVEL_21:
        maxMBPS = 19800;
        break;
    case CODEC_AVC_LEVEL_22:
        maxMBPS = 20250;
        break;
    case CODEC_AVC_LEVEL_3:
        maxMBPS = 40500;
        break;
    case CODEC_AVC_LEVEL_31:
        maxMBPS = 108000;
        break;
    case CODEC_AVC_LEVEL_32:
        maxMBPS = 216000;
        break;
    case CODEC_AVC_LEVEL_4:
    case CODEC_AVC_LEVEL_41:
        maxMBPS = 245760;
        break;
    case CODEC_AVC_LEVEL_42:
        maxMBPS = 522240;
        break;
    case CODEC_AVC_LEVEL_5:
        maxMBPS = 589824;
        break;
    case CODEC_AVC_LEVEL_51:
        maxMBPS = 983040;
        break;
    case CODEC_AVC_LEVEL_52:
        maxMBPS = 2073600;
        break;
    default:
        maxMBPS = 0;
        break;
    }

    return maxMBPS;
}

uint32_t AvcBasicFeature::GetProfileLevelMaxFrameSize()
{
    ENCODE_FUNC_CALL();

    double     bitsPerMB, tmpf;
    int32_t    iMaxMBPS, numMBPerFrame, minCR;
    uint64_t   maxBytePerPic, maxBytePerPicNot0;
    uint32_t   profileLevelMaxFrame, userMaxFrameSize;
    double     frameRateD = 100;

    minCR = 4;

    if (m_seqParam->Level >= CODEC_AVC_LEVEL_31 && m_seqParam->Level <= CODEC_AVC_LEVEL_4)
    {
        bitsPerMB = 96;
    }
    else
    {
        bitsPerMB = 192;
        minCR     = 2;
    }

    iMaxMBPS = GetMaxMBPS(m_seqParam->Level);
    if (!iMaxMBPS)
    {
        ENCODE_ASSERTMESSAGE("Unsupported LevelIDC setting.");
        return 0;
    }

    numMBPerFrame = m_picWidthInMb * m_frameFieldHeightInMb;

    tmpf = (double)numMBPerFrame;
    if (tmpf < iMaxMBPS / 172.)
    {
        tmpf = iMaxMBPS / 172.;
    }

    maxBytePerPic = (uint64_t)(tmpf * bitsPerMB);
    maxBytePerPicNot0 =
        (uint64_t)((((double)iMaxMBPS * frameRateD) /
                       (double)m_seqParam->FramesPer100Sec) *
                   bitsPerMB);
    userMaxFrameSize = m_seqParam->UserMaxFrameSize;

    if ((m_pictureCodingType != I_TYPE) && (m_seqParam->UserMaxPBFrameSize > 0))
    {
        userMaxFrameSize = m_seqParam->UserMaxPBFrameSize;
    }

    if (userMaxFrameSize != 0)
    {
        profileLevelMaxFrame = (uint32_t)MOS_MIN(userMaxFrameSize, maxBytePerPic);
        profileLevelMaxFrame = (uint32_t)MOS_MIN(maxBytePerPicNot0, profileLevelMaxFrame);
    }
    else
    {
        profileLevelMaxFrame = (uint32_t)MOS_MIN(maxBytePerPicNot0, maxBytePerPic);
    }

    profileLevelMaxFrame = (uint32_t)MOS_MIN((m_frameHeight * m_frameHeight), profileLevelMaxFrame);

    return profileLevelMaxFrame;
}

MOS_STATUS AvcBasicFeature::GetTrackedBuffers()
{
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);
    ENCODE_CHK_NULL_RETURN(m_picParams);
    ENCODE_CHK_NULL_RETURN(m_allocator);

    auto currRefList = m_ref->GetCurrRefList();
    ENCODE_CHK_STATUS_RETURN(m_trackedBuf->Acquire(currRefList, false));

    m_resMbCodeBuffer = m_trackedBuf->GetBuffer(BufferType::mbCodedBuffer, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_resMbCodeBuffer);

    m_4xDSSurface = m_trackedBuf->GetSurface(BufferType::ds4xSurface, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_4xDSSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_4xDSSurface));

    // Input surface copy is needed if height is not MB aligned
    if (InputSurfaceNeedsExtraCopy(m_rawSurface))
    {
        auto alignedRawSurf = m_trackedBuf->GetSurface(BufferType::AlignedRawSurface, m_trackedBuf->GetCurrIndex());
        ENCODE_CHK_NULL_RETURN(alignedRawSurf);
        m_allocator->GetSurfaceInfo(alignedRawSurf);
        alignedRawSurf->OsResource.pGmmResInfo->GetSetCpSurfTag(true, m_rawSurface.OsResource.pGmmResInfo->GetSetCpSurfTag(false, 0));
        ENCODE_CHK_STATUS_RETURN(m_allocator->UpdateResourceUsageType(&alignedRawSurf->OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_INPUT_RAW));

        if (m_mediaCopyWrapper)
        {
            ENCODE_CHK_STATUS_RETURN(m_mediaCopyWrapper->MediaCopy(
                &m_rawSurface.OsResource,
                &alignedRawSurf->OsResource,
                MCPY_METHOD_DEFAULT));

            m_rawSurface      = *alignedRawSurf;
            m_rawSurfaceToEnc = m_rawSurfaceToPak = &m_rawSurface;
        }
        else
        {
            ENCODE_ASSERTMESSAGE("Input surface height %d is not 16-aligned, needs extra copy but MediaCopy is not avaliable.", m_rawSurface.dwHeight);
        }
    }

    return MOS_STATUS_SUCCESS;
}

void AvcBasicFeature::ScalingListFlat()
{
    // 4x4 block
    for (uint8_t idx2 = 0; idx2 < 6; idx2++)
    {
        for (uint8_t idx1 = 0; idx1 < 16; idx1++)
        {
            m_iqWeightScaleLists->WeightScale4x4[idx2][idx1] = 16;
        }
    }
    // 8x8 block
    for (uint8_t idx2 = 0; idx2 < 2; idx2++)
    {
        for (uint8_t idx1 = 0; idx1 < 64; idx1++)
        {
            m_iqWeightScaleLists->WeightScale8x8[idx2][idx1] = 16;
        }
    }
}

void AvcBasicFeature::ScalingListFallbackRuleA()
{
    for (uint8_t idx1 = 0; idx1 < 16; idx1++)
    {
        for (uint8_t idx2 = 0; idx2 < 3; idx2++)
        {
            m_iqWeightScaleLists->WeightScale4x4[idx2][CODEC_AVC_Qmatrix_scan_4x4[idx1]] =
                CODEC_AVC_Default_4x4_Intra[idx1];
        }
        for (uint8_t idx2 = 3; idx2 < 6; idx2++)
        {
            m_iqWeightScaleLists->WeightScale4x4[idx2][CODEC_AVC_Qmatrix_scan_4x4[idx1]] =
                CODEC_AVC_Default_4x4_Inter[idx1];
        }
    }
    // 8x8 block
    for (uint8_t idx1 = 0; idx1 < 64; idx1++)
    {
        m_iqWeightScaleLists->WeightScale8x8[0][CODEC_AVC_Qmatrix_scan_8x8[idx1]] =
            CODEC_AVC_Default_8x8_Intra[idx1];
        m_iqWeightScaleLists->WeightScale8x8[1][CODEC_AVC_Qmatrix_scan_8x8[idx1]] =
            CODEC_AVC_Default_8x8_Inter[idx1];
    }
}

uint16_t AvcBasicFeature::GetMaxNumSlicesAllowed(
    CODEC_AVC_PROFILE_IDC profileIdc,
    CODEC_AVC_LEVEL_IDC   levelIdc,
    uint32_t              framesPer100Sec)
{
    uint16_t maxAllowedNumSlices = 0;

    if ((profileIdc == CODEC_AVC_MAIN_PROFILE) ||
        (profileIdc == CODEC_AVC_HIGH_PROFILE) ||
        (profileIdc == CODEC_AVC_HIGH10_PROFILE) ||
        (profileIdc == CODEC_AVC_HIGH422_PROFILE) ||
        (profileIdc == CODEC_AVC_HIGH444_PROFILE) ||
        (profileIdc == CODEC_AVC_CAVLC444_INTRA_PROFILE))
    {
        switch (levelIdc)
        {
        case CODEC_AVC_LEVEL_3:
            maxAllowedNumSlices = (uint16_t)(40500.0 * 100 / 22.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_31:
            maxAllowedNumSlices = (uint16_t)(108000.0 * 100 / 60.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_32:
            maxAllowedNumSlices = (uint16_t)(216000.0 * 100 / 60.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_4:
        case CODEC_AVC_LEVEL_41:
            maxAllowedNumSlices = (uint16_t)(245760.0 * 100 / 24.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_42:
            maxAllowedNumSlices = (uint16_t)(522240.0 * 100 / 24.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_5:
            maxAllowedNumSlices = (uint16_t)(589824.0 * 100 / 24.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_51:
            maxAllowedNumSlices = (uint16_t)(983040.0 * 100 / 24.0 / framesPer100Sec);
            break;
        case CODEC_AVC_LEVEL_52:
            maxAllowedNumSlices = (uint16_t)(2073600.0 * 100 / 24.0 / framesPer100Sec);
            break;
        default:
            maxAllowedNumSlices = 0;
        }
    }

    return maxAllowedNumSlices;
}

bool AvcBasicFeature::IsAvcPSlice(uint8_t sliceType) const
{
    return (sliceType < MHW_ARRAY_SIZE(Slice_Type)) ?
        (Slice_Type[sliceType] == SLICE_P) : false;
}

bool AvcBasicFeature::IsAvcBSlice(uint8_t sliceType) const
{
    return (sliceType < MHW_ARRAY_SIZE(Slice_Type)) ?
        (Slice_Type[sliceType] == SLICE_B) : false;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, AvcBasicFeature)
{
    params.standardSelect = (uint8_t)CodecHal_GetStandardFromMode(m_mode);
    params.dynamicSlice   = m_seqParam->EnableSliceLevelRateCtrl;
    params.chromaType     = m_seqParam->chroma_format_idc;
    params.randomAccess   = (m_picParam->CodingType == B_TYPE);
    params.frameStatisticsStreamOut = m_picParam->StatusReportEnable.fields.FrameStats;

    // override perf settings for AVC codec on B-stepping
    static const uint8_t par1table[] = { 0, 0, 1, 1, 1, 1, 1 };
    params.verticalShift32Minus1   = 0;
    params.hzShift32Minus1         = 15;
    params.numVerticalReqMinus1    = 5;
    params.numHzReqMinus1          = 0;
    params.VdencPipeModeSelectPar1 = par1table[m_seqParam->TargetUsage - 1];
    params.VdencPipeModeSelectPar0 = 1;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_SRC_SURFACE_STATE, AvcBasicFeature)
{
    params.pitch                         = m_rawSurfaceToPak->dwPitch;
    params.tileType                      = m_rawSurfaceToPak->TileType;
    params.tileModeGmm                   = m_rawSurfaceToPak->TileModeGMM;
    params.format                        = m_rawSurfaceToPak->Format;
    params.gmmTileEn                     = m_rawSurfaceToPak->bGMMTileEnabled;
    params.uOffset                       = m_rawSurfaceToPak->YoffsetForUplane;
    params.vOffset                       = m_rawSurfaceToPak->YoffsetForVplane;
    params.displayFormatSwizzle          = m_picParam->bDisplayFormatSwizzle;
    params.width                         = m_seqParam->FrameWidth;
    params.height                        = m_seqParam->FrameHeight;
    params.colorSpaceSelection           = (m_seqParam->InputColorSpace == ECOLORSPACE_P709) ? 1 : 0;
    params.chromaDownsampleFilterControl = 7;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_REF_SURFACE_STATE, AvcBasicFeature)
{
    params.pitch       = m_reconSurface.dwPitch;
    params.tileType    = m_reconSurface.TileType;
    params.tileModeGmm = m_reconSurface.TileModeGMM;
    params.format      = m_reconSurface.Format;
    params.gmmTileEn   = m_reconSurface.bGMMTileEnabled;
    params.uOffset     = m_reconSurface.YoffsetForUplane;
    params.vOffset     = m_reconSurface.YoffsetForVplane;
    params.width       = m_reconSurface.dwWidth;
    params.height      = m_reconSurface.dwHeight;

    if (m_reconSurface.Format == Format_Y410 || m_reconSurface.Format == Format_444P || m_reconSurface.Format == Format_AYUV)
    {
        if (m_reconSurface.Format == Format_Y410)
        {
            params.pitch = m_reconSurface.dwPitch / 2;
        }
        else
        {
            params.pitch = m_reconSurface.dwPitch / 4;
        }
        params.uOffset = m_rawSurfaceToPak->dwHeight;
        params.vOffset = m_rawSurfaceToPak->dwHeight << 1;
    }
    else if (m_reconSurface.Format == Format_Y216 || m_reconSurface.Format == Format_YUY2 || m_reconSurface.Format == Format_YUYV)
    {
        params.uOffset = m_rawSurfaceToPak->dwHeight;
        params.vOffset = m_rawSurfaceToPak->dwHeight;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_DS_REF_SURFACE_STATE, AvcBasicFeature)
{
    params.widthStage1       = m_4xDSSurface->dwWidth;
    params.heightStage1      = m_4xDSSurface->dwHeight;
    params.tileTypeStage1    = m_4xDSSurface->TileType;
    params.tileModeGmmStage1 = m_4xDSSurface->TileModeGMM;
    params.gmmTileEnStage1   = m_4xDSSurface->bGMMTileEnabled;
    params.pitchStage1       = m_4xDSSurface->dwPitch;
    params.uOffsetStage1     = m_4xDSSurface->YoffsetForUplane;
    params.vOffsetStage1     = m_4xDSSurface->YoffsetForVplane;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, AvcBasicFeature)
{
    params.surfaceRaw      = m_rawSurfaceToPak;
    params.streamOutBuffer = m_recycleBuf->GetBuffer(VdencStatsBuffer, 0);
    params.surfaceDsStage1 = m_4xDSSurface;

#ifdef _MMC_SUPPORTED
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        if (m_mmcState->IsMmcEnabled())
        {
            ENCODE_CHK_NULL_RETURN(m_rawSurfaceToPak)

            params.mmcEnabled = true;

            MOS_MEMCOMP_STATE reconSurfMmcState = MOS_MEMCOMP_DISABLED;

            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(&m_reconSurface), &reconSurfMmcState));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(const_cast<PMOS_SURFACE>(&m_reconSurface), &params.compressionFormatRecon));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(m_rawSurfaceToPak, &params.mmcStateRaw));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(m_rawSurfaceToPak, &params.compressionFormatRaw));

            if (m_deblockingEnabled)
            {
                params.mmcStatePreDeblock = MOS_MEMCOMP_DISABLED;
                params.mmcStatePostDeblock = reconSurfMmcState;
            }
            else
            {
                params.mmcStatePreDeblock = reconSurfMmcState;
                params.mmcStatePostDeblock = MOS_MEMCOMP_DISABLED;
            }
        }
        else
        {
            params.mmcEnabled = false;
            params.mmcStatePreDeblock = MOS_MEMCOMP_DISABLED;
            params.mmcStateRaw = MOS_MEMCOMP_DISABLED;
        }
#endif

    if (m_ref->GetRefList()[m_currReconstructedPic.FrameIdx]->bUsedAsRef)
    {
        auto curIdx = m_trackedBuf->GetCurrIndex();

        m_ref->GetRefList()[m_currReconstructedPic.FrameIdx]->bIsIntra = (m_pictureCodingType == I_TYPE);
        params.colocatedMvWriteBuffer = (m_pictureCodingType == I_TYPE) ? nullptr : m_trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, curIdx);
    }
    else
    {
        params.colocatedMvWriteBuffer = nullptr;
    }

    if (m_pictureCodingType == B_TYPE)
    {
        auto refPic = m_sliceParams->RefPicList[1][0];
        if (!CodecHal_PictureIsInvalid(refPic))
        {
            auto picIdx = m_ref->GetRefListIndex(refPic.FrameIdx);
            if (m_ref->GetRefList()[picIdx]->bIsIntra)
            {
                params.colocatedMvReadBuffer = m_colocatedMVBufferForIFrames;
            }
            else
            {
                auto mvTempBufIdx = m_ref->GetRefList()[picIdx]->ucScalingIdx;
                params.colocatedMvReadBuffer = m_trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, mvTempBufIdx);
            }
        }
        else
        {
            params.colocatedMvReadBuffer = nullptr;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD3, AvcBasicFeature)
{
    auto settings = static_cast<AvcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    for (const auto &lambda : settings->vdencCmd3Settings)
    {
        ENCODE_CHK_STATUS_RETURN(lambda(params));
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_IMG_STATE, AvcBasicFeature)
{
    params.pictureType      = m_picParam->CodingType - 1;
    params.transform8X8Flag = m_picParam->transform_8x8_mode_flag;

    params.pictureHeightMinusOne = m_picHeightInMb - 1;
    params.pictureWidth          = m_picWidthInMb;

    ENCODE_CHK_STATUS_RETURN(m_ref->MHW_SETPAR_F(VDENC_AVC_IMG_STATE)(params));

    if (m_ref->GetRefList()[m_currReconstructedPic.FrameIdx]->bUsedAsRef && m_pictureCodingType != I_TYPE)
        params.colMVWriteEnable = true;

    if (m_pictureCodingType == B_TYPE) {
        if (!CodecHal_PictureIsInvalid(m_sliceParams->RefPicList[1][0])) {
            params.colMVReadEnable = true;
        }
    }

    // Rolling-I settings
    if (m_picParam->CodingType != I_TYPE && m_picParam->EnableRollingIntraRefresh != ROLLING_I_DISABLED)
    {
        params.intraRefreshMbPos                = m_picParam->IntraRefreshMBNum;
        params.intraRefreshMbSizeMinusOne       = m_picParam->IntraRefreshUnitinMB;
        params.intraRefreshEnableRollingIEnable = m_picParam->EnableRollingIntraRefresh != ROLLING_I_DISABLED ? 1 : 0;
        params.intraRefreshMode                 = m_picParam->EnableRollingIntraRefresh == ROLLING_I_ROW ? 0 : 1;
        params.qpAdjustmentForRollingI          = m_picParam->IntraRefreshQPDelta;
    }

    if (m_minMaxQpControlEnabled)
    {
        params.minQp = (m_pictureCodingType == I_TYPE) ? m_iMinQp
                    : ((m_pictureCodingType == P_TYPE) ? m_pMinQp
                                                       : m_bMinQp);
        params.maxQp = (m_pictureCodingType == I_TYPE) ? m_iMaxQp
                    : ((m_pictureCodingType == P_TYPE) ? m_pMaxQp
                                                       : m_bMaxQp);
    }

    params.mbLevelDeltaQpEnable = m_mbQpDataEnabled && m_picParam->NumDeltaQpForNonRectROI;
    params.qpPrimeY                               = m_picParam->QpY + m_sliceParams->slice_qp_delta;

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    for (const auto &lambda : settings->vdencAvcImgStateSettings)
    {
        ENCODE_CHK_STATUS_RETURN(lambda(params));
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_SLICE_STATE, AvcBasicFeature)
{
    auto sliceParams = &m_sliceParams[m_curNumSlices];

    params.log2WeightDenomLuma = sliceParams->luma_log2_weight_denom;
    if (sliceParams->slice_type == 1 || sliceParams->slice_type == 6) // B slice
    {
        if (m_picParam->weighted_bipred_idc == IMPLICIT_WEIGHTED_INTER_PRED_MODE)
        {
            params.log2WeightDenomLuma = 0;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_WALKER_STATE, AvcBasicFeature)
{
    auto sliceParams = &m_sliceParams[m_curNumSlices];
    auto frameHeight = static_cast<uint32_t>(CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_seqParam->FrameHeight));
    auto frameWidth = static_cast<uint32_t>(CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_seqParam->FrameWidth));

    params.tileSliceStartLcuMbY = sliceParams->first_mb_in_slice / frameWidth;
    params.firstSuperSlice = (m_curNumSlices == 0);

    auto nextsliceMbStartYPosition = (sliceParams->first_mb_in_slice + sliceParams->NumMbsForSlice) / frameWidth;
    params.nextTileSliceStartLcuMbY = nextsliceMbStartYPosition > frameHeight ? frameHeight : nextsliceMbStartYPosition;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_MODE_SELECT, AvcBasicFeature)
{
    bool suppressReconPic = ((!m_ref->GetRefList()[m_currReconstructedPic.FrameIdx]->bUsedAsRef) && m_suppressReconPicSupported);

    params.standardSelect = CodecHal_GetStandardFromMode(m_mode);
    params.codecSelect = encoderCodec;
    // Enable PAK statistics streamout
    params.frameStatisticsStreamoutEnable = true;
    params.scaledSurfaceEnable = false;
    params.preDeblockingOutputEnablePredeblockoutenable = !m_deblockingEnabled && !suppressReconPic;
    params.postDeblockingOutputEnablePostdeblockoutenable = m_deblockingEnabled && !suppressReconPic;

    // Disable PAK streamout from previous PAK pass, as VDEnc does not support standalone PAK
    params.streamOutEnable = false;
    if (m_perMBStreamOutEnable)
    {
        // Enable PerMB streamOut PAK Statistics
        params.streamOutEnable = true;
        params.extendedStreamOutEnable = true;
    }

    params.deblockerStreamOutEnable = 0;
    params.vdencMode = 1;
    params.decoderShortFormatMode = 1;
    params.sliceSizeStreamout32bit = true;

    return MOS_STATUS_SUCCESS;
}

static inline uint32_t GetHwTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
{
    uint32_t tileMode = 0;

    if (gmmTileEnabled)
    {
        return tileModeGMM;
    }

    switch (tileType)
    {
    case MOS_TILE_LINEAR:
        tileMode = 0;
        break;
    case MOS_TILE_YS:
        tileMode = 1;
        break;
    case MOS_TILE_X:
        tileMode = 2;
        break;
    default:
        tileMode = 3;
        break;
    }

    return tileMode;
}

uint8_t MosToMediaStateFormat(MOS_FORMAT format)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
        return MHW_MEDIASTATE_SURFACEFORMAT_R8G8B8A8_UNORM;
    case Format_422H:
    case Format_422V:
        return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_422_8;
    case Format_AYUV:
    case Format_AUYV:
        return MHW_MEDIASTATE_SURFACEFORMAT_A8Y8U8V8_UNORM;
    case Format_NV12:
    case Format_NV11:
    case Format_P208:
    case Format_IMC1:
    case Format_IMC3:
        return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_420_8;
    case Format_400P:
    case Format_P8:
        return MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM;
    case Format_411P:
    case Format_411R:
        return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_411_8;
    case Format_UYVY:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPY;
    case Format_YVYU:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUV;
    case Format_VYUY:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUVY;
    case Format_YUY2:
    case Format_YUYV:
    case Format_444P:
    case Format_IMC2:
    case Format_IMC4:
    default:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL;
    }

    return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL;
}

bool IsVPlanePresent(MOS_FORMAT format)
{
    switch (format)
    {
    case Format_NV12:
    case Format_NV11:
    case Format_P208:
    case Format_IMC1:
    case Format_IMC3:
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_422H:
    case Format_422V:
        // Adding RGB formats because RGB is treated like YUV for JPEG encode and decode
    case Format_RGBP:
    case Format_BGRP:
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
    case Format_411P:
    case Format_411R:
    case Format_444P:
    case Format_IMC2:
    case Format_IMC4:
        return true;
    default:
        return false;
    }
}

MHW_SETPAR_DECL_SRC(MFX_SURFACE_STATE, AvcBasicFeature)
{
    PMOS_SURFACE psSurface        = nullptr;
    uint32_t     uvPlaneAlignment = 0;

    switch (params.surfaceId)
    {
    case CODECHAL_MFX_REF_SURFACE_ID:
        psSurface        = const_cast<PMOS_SURFACE>(&m_reconSurface);
        params.height    = psSurface->dwHeight - 1;
        params.width     = psSurface->dwWidth - 1;
        uvPlaneAlignment = MHW_VDBOX_MFX_RECON_UV_PLANE_ALIGNMENT;
        break;
    case CODECHAL_MFX_SRC_SURFACE_ID:
        psSurface        = m_rawSurfaceToPak;
        params.height    = m_seqParam->FrameHeight - 1;
        params.width     = m_seqParam->FrameWidth - 1;
        uvPlaneAlignment = MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9;
        break;
    case CODECHAL_MFX_DSRECON_SURFACE_ID:
        psSurface        = m_4xDSSurface;
        params.height    = psSurface->dwHeight - 1;
        params.width     = psSurface->dwWidth - 1;
        uvPlaneAlignment = MHW_VDBOX_MFX_RECON_UV_PLANE_ALIGNMENT;
        break;
    }

    ENCODE_CHK_NULL_RETURN(psSurface);

    params.tilemode         = GetHwTileType(psSurface->TileType, psSurface->TileModeGMM, psSurface->bGMMTileEnabled);
    params.surfacePitch     = psSurface->dwPitch - 1;
    params.interleaveChroma = psSurface->Format == Format_P8 ? 0 : 1;
    params.surfaceFormat    = MosToMediaStateFormat(psSurface->Format);

    params.yOffsetForUCb = params.yOffsetForVCr =
        MOS_ALIGN_CEIL((psSurface->UPlaneOffset.iSurfaceOffset - psSurface->dwOffset)/psSurface->dwPitch + psSurface->RenderOffset.YUV.U.YOffset, uvPlaneAlignment);
    if (IsVPlanePresent(psSurface->Format))
    {
        params.yOffsetForVCr =
            MOS_ALIGN_CEIL((psSurface->VPlaneOffset.iSurfaceOffset - psSurface->dwOffset)/psSurface->dwPitch + psSurface->RenderOffset.YUV.V.YOffset, uvPlaneAlignment);
    }

#ifdef _MMC_SUPPORTED
    if (m_mmcState && m_mmcState->IsMmcEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(psSurface, &params.compressionFormat));
    }
#endif
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_BUF_ADDR_STATE, AvcBasicFeature)
{
    params.decodeInUse                  = false;
    params.oneOnOneMapping              = false;
    params.psPreDeblockSurface          = const_cast<PMOS_SURFACE>(&m_reconSurface);
    params.psPostDeblockSurface         = const_cast<PMOS_SURFACE>(&m_reconSurface);
    params.psRawSurface                 = m_rawSurfaceToPak;
    params.presSliceSizeStreamOutBuffer = m_recycleBuf->GetBuffer(PakSliceSizeStreamOutBuffer, m_frameNum);

    ENCODE_CHK_STATUS_RETURN(m_ref->MHW_SETPAR_F(MFX_PIPE_BUF_ADDR_STATE)(params));

#ifdef _MMC_SUPPORTED
    MOS_MEMCOMP_STATE reconSurfMmcState = MOS_MEMCOMP_DISABLED;
    if (m_mmcState->IsMmcEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(m_rawSurfaceToPak, &params.RawSurfMmcState));
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(&m_reconSurface), &reconSurfMmcState));

        if (m_deblockingEnabled)
        {
            params.PreDeblockSurfMmcState  = MOS_MEMCOMP_DISABLED;
            params.PostDeblockSurfMmcState = reconSurfMmcState;
        }
        else
        {
            params.PreDeblockSurfMmcState  = reconSurfMmcState;
            params.PostDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        }
    }
    else
    {
        params.PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        params.RawSurfMmcState        = MOS_MEMCOMP_DISABLED;
    }

    CODECHAL_DEBUG_TOOL(
        params.psPreDeblockSurface->MmcState = reconSurfMmcState;)
#endif
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_IND_OBJ_BASE_ADDR_STATE, AvcBasicFeature)
{
    params.Mode                    = CODECHAL_ENCODE_MODE_AVC;
    params.presPakBaseObjectBuffer = const_cast<PMOS_RESOURCE>(&m_resBitstreamBuffer);
    params.dwPakBaseObjectSize     = m_bitstreamSize;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_IMG_STATE, AvcBasicFeature)
{
    auto settings = static_cast<AvcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    if (m_seqParam->EnableSliceLevelRateCtrl)
    {
        params.targetSliceSizeInBytes = m_picParam->SliceSizeInBytes;
    }

    uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
    params.frameSize = (numMBs > 0xFFFF) ? 0xFFFF : numMBs;

    params.frameHeight = m_picHeightInMb - 1;
    params.frameWidth = m_picWidthInMb - 1;

    params.imgstructImageStructureImgStructure10 = (CodecHal_PictureIsFrame(m_picParam->CurrOriginalPic) ?
                                                    avcFrame : (CodecHal_PictureIsTopField(m_picParam->CurrOriginalPic) ?
                                                    avcTopField : avcBottomField));

    params.weightedBipredIdc = m_picParam->weighted_bipred_idc;
    params.weightedPredFlag = m_picParam->weighted_pred_flag;

    params.firstChromaQpOffset = m_picParam->chroma_qp_index_offset;
    params.secondChromaQpOffset = m_picParam->second_chroma_qp_index_offset;

    params.fieldpicflag = CodecHal_PictureIsField(m_picParam->CurrOriginalPic);
    params.mbaffflameflag = m_seqParam->mb_adaptive_frame_field_flag;
    params.framembonlyflag = m_seqParam->frame_mbs_only_flag;
    params.transform8X8Flag = m_picParam->transform_8x8_mode_flag;
    params.direct8X8Infflag = m_seqParam->direct_8x8_inference_flag;
    params.constrainedipredflag = m_picParam->constrained_intra_pred_flag;
    params.entropycodingflag = m_picParam->entropy_coding_mode_flag;
    params.mbmvformatflag = 1;
    params.chromaformatidc = m_seqParam->chroma_format_idc;
    params.mvunpackedflag = 1;

    params.intrambmaxbitflagIntrambmaxsizereportmask = 1;
    params.intermbmaxbitflagIntermbmaxsizereportmask = 1;
    params.frameszoverflagFramebitratemaxreportmask = 1;
    params.frameszunderflagFramebitrateminreportmask = 1;
    params.intraIntermbipcmflagForceipcmcontrolmask = 1;
    params.mbratectrlflagMbLevelRateControlEnablingFlag = 0;

    params.intrambmaxsz = settings->intraMbMaxSize;
    params.intermbmaxsz = settings->interMbMaxSize;

    // Set this to New mode (= 1) - FrameBitRateMaxUnit and FrameBitRateMinUnit are in new mode (32byte/4Kb)
    params.framebitratemaxunitmode = params.framebitrateminunitmode = 1;
    // Set this to Kilo Byte (= 1) - FrameBitRateMax and FrameBitRateMin are in units of 4KBytes when FrameBitrateMaxUnitMode and FrameBitRateMinUnitMode are set to 1
    params.framebitratemaxunit = params.framebitrateminunit = 1;
    // We have 14 bits available when FrameBitRateMaxUnitMode is set to 1, so set it to max value
    params.framebitratemax = (1 << 14) - 1;
    // Set this to min available value
    params.framebitratemin = 0;
    // Set frame bitrate max and min delta to 0
    params.framebitratemindelta = params.framebitratemaxdelta = 0;

    params.sliceStatsStreamoutEnable = true;
    params.extendedRhodomainStatisticsEnable = true;

    if (m_seqParam->EnableSliceLevelRateCtrl)
    {
        uint8_t qpY                     = m_picParam->QpY;
        uint32_t dwVdencSliceMinusBytes = (m_pictureCodingType == I_TYPE) ? settings->SliceSizeThrsholdsI[qpY] : settings->SliceSizeThrsholdsP[qpY];
        params.thresholdSizeInBytes     = m_picParam->SliceSizeInBytes - MOS_MIN(m_picParam->SliceSizeInBytes, dwVdencSliceMinusBytes);
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_REF_IDX_STATE, AvcBasicFeature)
{
    PCODEC_AVC_ENCODE_SLICE_PARAMS sliceParams        = &m_sliceParams[m_curNumSlices];
    PCODEC_REF_LIST *              avcRefList         = m_ref->GetRefList();
    AvcRefListWrite *              cmdAvcRefListWrite = (AvcRefListWrite *)&(params.referenceListEntry);

    int32_t uiNumRefForList[2] = {sliceParams->num_ref_idx_l0_active_minus1 + 1, sliceParams->num_ref_idx_l1_active_minus1 + 1};

    for (auto i = 0; i < uiNumRefForList[params.uiList]; i++)
    {
        uint8_t idx = sliceParams->RefPicList[params.uiList][i].FrameIdx;
        if (idx >= CODEC_MAX_NUM_REF_FRAME)
        {
            ENCODE_ASSERT(false); // Idx must be within 0 to 15
            idx = 0;
        }
        idx = m_ref->GetPicIndex()[idx].ucPicIdx;

        cmdAvcRefListWrite->UC[i].frameStoreID = avcRefList[idx]->ucFrameId;
        cmdAvcRefListWrite->UC[i].bottomField  = CodecHal_PictureIsBottomField(sliceParams->RefPicList[params.uiList][i]);
        cmdAvcRefListWrite->UC[i].fieldPicFlag = CodecHal_PictureIsField(sliceParams->RefPicList[params.uiList][i]);
        cmdAvcRefListWrite->UC[i].longTermFlag = CodecHal_PictureIsLongTermRef(avcRefList[idx]->RefPic);
        cmdAvcRefListWrite->UC[i].nonExisting  = 0;
    }

    for (auto i = uiNumRefForList[params.uiList]; i < 32; i++)
    {
        cmdAvcRefListWrite->UC[i].value = 0x80;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_SLICE_STATE, AvcBasicFeature)
{
    auto sliceParams = &m_sliceParams[m_curNumSlices];

    uint16_t widthInMb = m_seqParam->pic_width_in_mbs_minus1 + 1;
    uint32_t startMbNum = sliceParams->first_mb_in_slice * (1 + m_seqParam->mb_adaptive_frame_field_flag);

    params.sliceType = Slice_Type[sliceParams->slice_type];

    params.log2WeightDenomLuma = sliceParams->luma_log2_weight_denom;
    params.log2WeightDenomChroma = sliceParams->chroma_log2_weight_denom;
    params.numberOfReferencePicturesInInterPredictionList0 = 0;
    params.numberOfReferencePicturesInInterPredictionList1 = 0;

    params.sliceAlphaC0OffsetDiv2 = sliceParams->slice_alpha_c0_offset_div2;
    params.sliceBetaOffsetDiv2 = sliceParams->slice_beta_offset_div2;
    params.sliceQuantizationParameter = 26 + m_picParam->pic_init_qp_minus26 + sliceParams->slice_qp_delta;
    params.cabacInitIdc10 = sliceParams->cabac_init_idc;
    params.disableDeblockingFilterIndicator = sliceParams->disable_deblocking_filter_idc;
    params.directPredictionType = IsAvcBSlice(sliceParams->slice_type) ? sliceParams->direct_spatial_mv_pred_flag : 0;
    params.weightedPredictionIndicator = DEFAULT_WEIGHTED_INTER_PRED_MODE;

    params.sliceHorizontalPosition = startMbNum % widthInMb;
    params.sliceVerticalPosition = startMbNum / widthInMb;
    params.nextSliceHorizontalPosition = (startMbNum + sliceParams->NumMbsForSlice) % widthInMb;
    params.nextSliceVerticalPosition = (startMbNum + sliceParams->NumMbsForSlice) / widthInMb;

    params.sliceId30 = (uint8_t)sliceParams->slice_id;
    params.cabaczerowordinsertionenable = 0;
    params.emulationbytesliceinsertenable = 1;
    params.tailInsertionPresentInBitstream = m_vdencNoTailInsertion ? 0 : (m_picParam->bLastPicInSeq || m_picParam->bLastPicInStream);
    params.slicedataInsertionPresentInBitstream = 1;
    params.headerInsertionPresentInBitstream = 1;
    params.isLastSlice = (startMbNum + sliceParams->NumMbsForSlice) >= (uint32_t)(widthInMb * m_frameFieldHeightInMb);

    if (IsAvcPSlice(sliceParams->slice_type))
    {
        params.numberOfReferencePicturesInInterPredictionList0 = sliceParams->num_ref_idx_l0_active_minus1_from_DDI + 1;
        params.weightedPredictionIndicator = m_picParam->weighted_pred_flag;
    }
    else if (IsAvcBSlice(sliceParams->slice_type))
    {
        params.numberOfReferencePicturesInInterPredictionList1 = sliceParams->num_ref_idx_l1_active_minus1_from_DDI + 1;
        params.numberOfReferencePicturesInInterPredictionList0 = sliceParams->num_ref_idx_l0_active_minus1_from_DDI + 1;
        params.weightedPredictionIndicator = m_picParam->weighted_bipred_idc;
        if (m_picParam->weighted_bipred_idc == IMPLICIT_WEIGHTED_INTER_PRED_MODE)
        {
            params.log2WeightDenomLuma = 0;
            params.log2WeightDenomChroma = 0;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_DIRECTMODE_STATE, AvcBasicFeature)
{
    params.CurrPic                 = m_picParam->CurrReconstructedPic;
    params.uiUsedForReferenceFlags = 0xFFFFFFFF;
    params.pAvcPicIdx              = m_ref->GetPicIndex();
    params.avcRefList              = (void**)m_ref->GetRefList();
    params.bPicIdRemappingInUse    = false;
    params.bDisableDmvBuffers      = true;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MI_FORCE_WAKEUP, AvcBasicFeature)
{
    params.bMFXPowerWellControl      = true;
    params.bMFXPowerWellControlMask  = true;
    params.bHEVCPowerWellControl     = false;
    params.bHEVCPowerWellControlMask = true;
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_WAIT, AvcBasicFeature)
{
    params.iStallVdboxPipeline = true;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_PIPE_MODE_SELECT, AvcBasicFeature)
{
    params.mode = m_mode;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
