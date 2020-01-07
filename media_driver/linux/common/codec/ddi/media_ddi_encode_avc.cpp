/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     media_ddi_encode_avc.cpp
//! \brief    Implements class for DDI media avc encode
//!

#include "media_libva.h"
#include "media_libva_encoder.h"
#include "media_libva_util.h"
#include "hwinfo_linux.h"
#include "media_ddi_encode_base.h"
#include "media_ddi_encode_avc.h"
#include "media_ddi_encode_const.h"
#include "media_ddi_factory.h"
#include "media_libva_caps.h"

// refer to spec section E.2.2, bit rate and cbp buffer size are shifted left with several bits, k is 1024
static const uint16_t vuiKbps = 1024;

static const uint8_t sliceTypeP = 0;
static const uint8_t sliceTypeB = 1;
static const uint8_t sliceTypeI = 2;
static const uint8_t maxPassesNum = 4;

//Inter MB partition 16x16 can't be disabled.
static const uint8_t disMbPartMask      = 0x7E;
static const uint8_t subpelModeMask     = 0x3;
static const uint8_t subpelModeQuant    = 0x3;
static const uint8_t subpelModeReserved = 0x2;
static const uint8_t subpelModeHalf     = 0x1;
extern template class MediaDdiFactoryNoArg<DdiEncodeBase>;
static bool isEncodeAvcRegistered =
    MediaDdiFactoryNoArg<DdiEncodeBase>::RegisterCodec<DdiEncodeAvc>(ENCODE_ID_AVC);

DdiEncodeAvc::~DdiEncodeAvc()
{
    if (nullptr == m_encodeCtx)
    {
        return;
    }
    MOS_FreeMemory(m_encodeCtx->pSeqParams);
    m_encodeCtx->pSeqParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pPicParams);
    m_encodeCtx->pPicParams = nullptr;

    if (nullptr != m_encodeCtx->ppNALUnitParams)
    {
        // Allocate one contiguous memory for the NALUnitParams buffers
        // only need to free one time
        MOS_FreeMemory(m_encodeCtx->ppNALUnitParams[0]);
        m_encodeCtx->ppNALUnitParams[0] = nullptr;

        MOS_FreeMemory(m_encodeCtx->ppNALUnitParams);
        m_encodeCtx->ppNALUnitParams = nullptr;
    }

    MOS_FreeMemory(m_encodeCtx->pVuiParams);
    m_encodeCtx->pVuiParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pSliceParams);
    m_encodeCtx->pSliceParams = nullptr;

    MOS_FreeMemory(m_encodeCtx->pEncodeStatusReport);
    m_encodeCtx->pEncodeStatusReport = nullptr;

    if (m_encodeCtx->pSEIFromApp)
    {
        MOS_FreeMemory(m_encodeCtx->pSEIFromApp->pSEIBuffer);
        m_encodeCtx->pSEIFromApp->pSEIBuffer = nullptr;

        MOS_FreeMemory(m_encodeCtx->pSEIFromApp);
        m_encodeCtx->pSEIFromApp = nullptr;
    }

    MOS_FreeMemory(m_encodeCtx->pSliceHeaderData);
    m_encodeCtx->pSliceHeaderData = nullptr;

    if (m_encodeCtx->pbsBuffer)
    {
        MOS_FreeMemory(m_encodeCtx->pbsBuffer->pBase);
        m_encodeCtx->pbsBuffer->pBase = nullptr;

        MOS_FreeMemory(m_encodeCtx->pbsBuffer);
        m_encodeCtx->pbsBuffer = nullptr;
    }

    MOS_FreeMemory(m_qcParams);
    m_qcParams = nullptr;

    MOS_FreeMemory(m_roundingParams);
    m_roundingParams = nullptr;

    MOS_FreeMemory(iqMatrixParams);
    iqMatrixParams = nullptr;

    MOS_FreeMemory(iqWeightScaleLists);
    iqWeightScaleLists = nullptr;
}

uint8_t DdiEncodeAvc::ConvertSliceStruct(uint32_t vaSliceStruct)
{
    switch (vaSliceStruct)
    {
    case VA_ENC_SLICE_STRUCTURE_ARBITRARY_ROWS:
        return CODECHAL_SLICE_STRUCT_ARBITRARYROWSLICE;
    case VA_ENC_SLICE_STRUCTURE_POWER_OF_TWO_ROWS:
        return CODECHAL_SLICE_STRUCT_POW2ROWS;
    case VA_ENC_SLICE_STRUCTURE_ARBITRARY_MACROBLOCKS:
        return CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE;
    case VA_ENC_SLICE_STRUCTURE_EQUAL_ROWS:
        return CODECHAL_SLICE_STRUCT_ROWSLICE;
    default:
        return CODECHAL_SLICE_STRUCT_ONESLICE;
    }
}

CODEC_AVC_PROFILE_IDC DdiEncodeAvc::GetAVCProfileFromVAProfile()
{
    switch (m_encodeCtx->vaProfile)
    {
    case VAProfileH264ConstrainedBaseline:
        return CODEC_AVC_BASE_PROFILE;
    case VAProfileH264Main:
        return CODEC_AVC_MAIN_PROFILE;
    case VAProfileH264High:
        return CODEC_AVC_HIGH_PROFILE;
    default:
        return CODEC_AVC_MAIN_PROFILE;
    }
}

uint8_t DdiEncodeAvc::CodechalPicTypeFromVaSlcType(uint8_t vaSliceType)
{
    switch (vaSliceType)
    {
    case sliceTypeI:
    case (sliceTypeI + 5):
        return I_TYPE;
    case sliceTypeP:
    case (sliceTypeP + 5):
        return P_TYPE;
    case sliceTypeB:
    case (sliceTypeB + 5):
        return B_TYPE;
    default:
        return 0;
    }
}

VAStatus DdiEncodeAvc::ParseMiscParamHRD(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterHRD          *vaEncMiscParamHRD = (VAEncMiscParameterHRD *)data;
    CODECHAL_ENCODE_AVC_VUI_PARAMS *vuiParam          = (CODECHAL_ENCODE_AVC_VUI_PARAMS *)m_encodeCtx->pVuiParams;
    // Assume only one SPS here, modify when enable multiple SPS support
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    DDI_CHK_NULL(vuiParam, "nullptr vuiParam", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    vuiParam->cbr_flag                    = 0x1;
    seqParams->VBVBufferSizeInBit         = vaEncMiscParamHRD->buffer_size;
    seqParams->InitVBVBufferFullnessInBit = vaEncMiscParamHRD->initial_buffer_fullness;
    vuiParam->cpb_size_value_minus1[0]    = MOS_ROUNDUP_DIVIDE(seqParams->VBVBufferSizeInBit, vuiKbps) - 1;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamFR(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Assume only one SPS here, modify when enable multiple SPS support
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams      = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    VAEncMiscParameterFrameRate      *encMiscParamFR = (VAEncMiscParameterFrameRate *)data;
    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t numerator = (encMiscParamFR->framerate & 0xffff) * 100; /** 100*/;
    auto denominator = (encMiscParamFR->framerate >> 16)&0xfff;
    if(denominator == 0)
    {
       denominator = 1;
    }

    // Pass frame rate value to seqParams and set BRC reset flag and bNewSeq to true while dynamic BRC occures
    seqParams->FramesPer100Sec = (uint16_t)(numerator / denominator);
    if(m_previousFRper100sec != 0)
    {
        if(m_previousFRper100sec != seqParams->FramesPer100Sec)
        {
            seqParams->bResetBRC = 0x1;
            m_encodeCtx->bNewSeq = true;
        }
    }
    m_previousFRper100sec = seqParams->FramesPer100Sec;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamRC(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    CODECHAL_ENCODE_AVC_VUI_PARAMS *vuiParam = (CODECHAL_ENCODE_AVC_VUI_PARAMS *)m_encodeCtx->pVuiParams;

    // Assume only one SPS here, modify when enable multiple SPS support
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams      = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams) + current_seq_parameter_set_id;
    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams      = (PCODEC_AVC_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams) + current_pic_parameter_set_id;
    VAEncMiscParameterRateControl    *encMiscParamRC = (VAEncMiscParameterRateControl *)data;
    DDI_CHK_NULL(vuiParam, "nullptr vuiParam", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    seqParams->TargetBitRate           = encMiscParamRC->bits_per_second;
    vuiParam->bit_rate_value_minus1[0] = MOS_ROUNDUP_SHIFT(encMiscParamRC->bits_per_second, 6 + vuiParam->bit_rate_scale) - 1;
    seqParams->MBBRC                   = encMiscParamRC->rc_flags.bits.mb_rate_control;

    // Assuming picParams are sent before MiscParams
    picParams->ucMinimumQP = encMiscParamRC->min_qp;
    picParams->ucMaximumQP = encMiscParamRC->max_qp;
    if (picParams->ucMaximumQP == 0 && picParams->ucMinimumQP)
        picParams->ucMaximumQP = 51;

    if ((VA_RC_CBR == m_encodeCtx->uiRCMethod) || ((VA_RC_CBR | VA_RC_MB) == m_encodeCtx->uiRCMethod))
    {
        seqParams->MaxBitRate = seqParams->TargetBitRate;
        seqParams->MinBitRate = seqParams->TargetBitRate;
        vuiParam->cbr_flag    = 0x1;
        if (m_encodeCtx->uiTargetBitRate != seqParams->TargetBitRate)
        {
            if (m_encodeCtx->uiTargetBitRate)
            {
                seqParams->bResetBRC = 0x1;
                m_encodeCtx->bNewSeq = true;
            }
            m_encodeCtx->uiTargetBitRate = seqParams->TargetBitRate;
            m_encodeCtx->uiMaxBitRate    = seqParams->TargetBitRate;
        }
    }
    else if (VA_RC_ICQ == m_encodeCtx->uiRCMethod)
    {
        seqParams->ICQQualityFactor = encMiscParamRC->ICQ_quality_factor;
    }
    else if (VA_RC_AVBR == m_encodeCtx->uiRCMethod)
    {
        seqParams->AVBRAccuracy = encMiscParamRC->target_percentage;
        seqParams->AVBRConvergence = encMiscParamRC->window_size;
    }
    else
    {
        seqParams->MaxBitRate    = seqParams->TargetBitRate;
        seqParams->MinBitRate    = (uint32_t)((uint64_t)seqParams->TargetBitRate * (2 * encMiscParamRC->target_percentage - 100) / 100);
        seqParams->TargetBitRate = (uint32_t)((uint64_t)seqParams->TargetBitRate * encMiscParamRC->target_percentage / 100);
        vuiParam->cbr_flag       = 0x0;

        if (VA_RC_QVBR == m_encodeCtx->uiRCMethod)
        {
            seqParams->ICQQualityFactor = encMiscParamRC->quality_factor;
        }

        if ((m_encodeCtx->uiTargetBitRate != seqParams->TargetBitRate) ||
            (m_encodeCtx->uiMaxBitRate != seqParams->MaxBitRate))
        {
            if ((m_encodeCtx->uiTargetBitRate != 0) && (m_encodeCtx->uiMaxBitRate != 0))
            {
                seqParams->bResetBRC = 0x1;
                m_encodeCtx->bNewSeq = true;
            }
            m_encodeCtx->uiTargetBitRate = seqParams->TargetBitRate;
            m_encodeCtx->uiMaxBitRate    = seqParams->MaxBitRate;
        }
    }
    //if RateControl method is VBR/CBR, we can set MBBRC to enable or disable
    if (VA_RC_CQP != m_encodeCtx->uiRCMethod)
    {
        if (encMiscParamRC->rc_flags.bits.mb_rate_control <= mbBrcDisabled)
            seqParams->MBBRC = encMiscParamRC->rc_flags.bits.mb_rate_control;
    }

#ifndef ANDROID
    seqParams->FrameSizeTolerance = static_cast<ENCODE_FRAMESIZE_TOLERANCE>(encMiscParamRC->rc_flags.bits.frame_tolerance_mode);
#endif

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamSkipFrame(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Assume only one PPS here, modify when enable multiple PPS support
    PCODEC_AVC_ENCODE_PIC_PARAMS picParams               = (PCODEC_AVC_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams) + current_pic_parameter_set_id;
    VAEncMiscParameterSkipFrame *vaEncMiscParamSkipFrame = (VAEncMiscParameterSkipFrame *)data;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    // populate skipped frame params from DDI
    picParams->SkipFrameFlag  = vaEncMiscParamSkipFrame->skip_frame_flag;
    picParams->NumSkipFrames  = vaEncMiscParamSkipFrame->num_skip_frames;
    picParams->SizeSkipFrames = vaEncMiscParamSkipFrame->size_skip_frames;

    if (FRAME_SKIP_NORMAL < picParams->SkipFrameFlag)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamMaxFrameSize(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Assume only one SPS here, modify when enable multiple SPS support
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS     seqParams                  = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    VAEncMiscParameterBufferMaxFrameSize *vaEncMiscParamMaxFrameSize = (VAEncMiscParameterBufferMaxFrameSize *)data;
    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    // populate MaxFrameSize from DDI
    seqParams->UserMaxFrameSize = vaEncMiscParamMaxFrameSize->max_frame_size >> 3;  // convert to byte

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamMultiPassFrameSize(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    PCODEC_AVC_ENCODE_PIC_PARAMS picParams = (PCODEC_AVC_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams) + current_pic_parameter_set_id;
    VAEncMiscParameterBufferMultiPassFrameSize *vaEncMiscParamMultiPassFrameSize = (VAEncMiscParameterBufferMultiPassFrameSize *)data;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    //add for multiple pass pak
    picParams->dwMaxFrameSize = vaEncMiscParamMultiPassFrameSize->max_frame_size;
    if (picParams->dwMaxFrameSize)
    {
        picParams->dwNumPasses = vaEncMiscParamMultiPassFrameSize->num_passes;
        if ((picParams->dwNumPasses == 0) || (picParams->dwNumPasses > maxPassesNum))
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (picParams->pDeltaQp != nullptr)
        {
            MOS_FreeMemory(picParams->pDeltaQp);
        }
        picParams->pDeltaQp = (uint8_t *)MOS_AllocAndZeroMemory(sizeof(uint8_t) * picParams->dwNumPasses);
        if (!picParams->pDeltaQp)
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (MOS_STATUS_SUCCESS != MOS_SecureMemcpy(picParams->pDeltaQp, picParams->dwNumPasses, vaEncMiscParamMultiPassFrameSize->delta_qp, picParams->dwNumPasses))
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamEncQuality(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Assume only one SPS here, modify when enable multiple SPS support
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams                = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams) + current_seq_parameter_set_id;

    VAEncMiscParameterEncQuality     *vaEncMiscParamEncQuality = (VAEncMiscParameterEncQuality *)data;
    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams                = (PCODEC_AVC_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams) + current_pic_parameter_set_id;

    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    seqParams->bForcePanicModeControl = 1;
    seqParams->bPanicModeDisable = (uint8_t)vaEncMiscParamEncQuality->PanicModeDisable;
    picParams->UserFlags.bUseRawPicForRef = vaEncMiscParamEncQuality->useRawPicForRef;
    m_qcParams->skipCheckDisable            = vaEncMiscParamEncQuality->skipCheckDisable;
    m_qcParams->FTQOverride                 = vaEncMiscParamEncQuality->FTQOverride;
    if (m_qcParams->FTQOverride)
    {
        m_qcParams->FTQEnable = vaEncMiscParamEncQuality->FTQEnable;
    }
    m_qcParams->FTQSkipThresholdLUTInput = vaEncMiscParamEncQuality->FTQSkipThresholdLUTInput;
    if (m_qcParams->FTQSkipThresholdLUTInput)
    {
        MOS_SecureMemcpy(m_qcParams->FTQSkipThresholdLUT, 52, vaEncMiscParamEncQuality->FTQSkipThresholdLUT, 52);
    }
    m_qcParams->NonFTQSkipThresholdLUTInput = vaEncMiscParamEncQuality->NonFTQSkipThresholdLUTInput;
    if (m_qcParams->NonFTQSkipThresholdLUTInput)
    {
        MOS_SecureMemcpy(m_qcParams->NonFTQSkipThresholdLUT, 52, vaEncMiscParamEncQuality->NonFTQSkipThresholdLUT, 52);
    }

    m_qcParams->directBiasAdjustmentEnable       = vaEncMiscParamEncQuality->directBiasAdjustmentEnable;
    m_qcParams->globalMotionBiasAdjustmentEnable = vaEncMiscParamEncQuality->globalMotionBiasAdjustmentEnable;
    m_qcParams->HMEMVCostScalingFactor           = vaEncMiscParamEncQuality->HMEMVCostScalingFactor;
    //disable HME
    m_qcParams->HMEDisable = vaEncMiscParamEncQuality->HMEDisable;
    //disable Super HME
    m_qcParams->SuperHMEDisable = vaEncMiscParamEncQuality->SuperHMEDisable;
    //disable Ultra HME
    m_qcParams->UltraHMEDisable = vaEncMiscParamEncQuality->UltraHMEDisable;

    // Force RepartitionCheck
    m_qcParams->ForceRepartitionCheck = vaEncMiscParamEncQuality->ForceRepartitionCheck;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamQuantization(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams                  = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    VAEncMiscParameterQuantization   *vaEncMiscParamQuantization = (VAEncMiscParameterQuantization *)data;
    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    seqParams->Trellis = trellisInternal;

    if (vaEncMiscParamQuantization->quantization_flags.bits.disable_trellis)
    {
        seqParams->Trellis = trellisDisabled;
    }
    else
    {
        if (vaEncMiscParamQuantization->quantization_flags.bits.enable_trellis_I)
        {
            seqParams->Trellis |= trellisEnabledI;
        }
        if (vaEncMiscParamQuantization->quantization_flags.bits.enable_trellis_P)
        {
            seqParams->Trellis |= trellisEnabledP;
        }
        if (vaEncMiscParamQuantization->quantization_flags.bits.enable_trellis_B)
        {
            seqParams->Trellis |= trellisEnabledB;
        }
        if (!seqParams->Trellis)
        {
            DDI_ASSERTMESSAGE("trellis enabled, but the input parameters is invalided");
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParameterRIR(void *data)
{
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams         = (PCODEC_AVC_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams) + current_pic_parameter_set_id;
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams         = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams) + current_seq_parameter_set_id;
    VAEncMiscParameterRIR            *vaEncMiscParamRIR = (VAEncMiscParameterRIR *)data;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    picParams->EnableRollingIntraRefresh = vaEncMiscParamRIR->rir_flags.value & 0x3;  //only lower two bits are valid

#ifdef Android
    //Convert row based rolling I to suqare region based rolling I before MSDK is ready to support square region based rolling I
    if (ROLLING_I_ROW == picParams->EnableRollingIntraRefresh)
    {
        if (GFX_IS_PRODUCT(m_encodeCtx->pMediaCtx->platform, IGFX_BROADWELL))
        {
            picParams->EnableRollingIntraRefresh = ROLLING_I_SQUARE;
        }
        vaEncMiscParamRIR->intra_insert_size *= (seqParams->FrameWidth / CODECHAL_ENCODE_AVC_ROI_WIDTH_SCALE_FACTOR);
        // From the experiment result, -6 could be the proper setting
        vaEncMiscParamRIR->qp_delta_for_inserted_intra = -6;
    }
#endif
    // Set for all frames since pic type is not known yet. Disable in slice params if its I or B type.
    switch (picParams->EnableRollingIntraRefresh)
    {
    case ROLLING_I_COLUMN:
        picParams->IntraRefreshMBx      = (uint8_t)vaEncMiscParamRIR->intra_insertion_location;
        picParams->IntraRefreshMBNum    = (uint8_t)vaEncMiscParamRIR->intra_insertion_location;
        picParams->IntraRefreshUnitinMB = (uint8_t)vaEncMiscParamRIR->intra_insert_size;
        break;
    case ROLLING_I_ROW:
        picParams->IntraRefreshMBy      = (uint8_t)vaEncMiscParamRIR->intra_insertion_location;
        picParams->IntraRefreshMBNum    = (uint8_t)vaEncMiscParamRIR->intra_insertion_location;
        picParams->IntraRefreshUnitinMB = (uint8_t)vaEncMiscParamRIR->intra_insert_size;
        break;
    case ROLLING_I_SQUARE:
        picParams->IntraRefreshUnitinMB = (uint8_t)GFX_UF_ROUND(sqrt((double)vaEncMiscParamRIR->intra_insert_size));
        break;
    case ROLLING_I_DISABLED:
        break;
    default:
        return MOS_STATUS_INVALID_PARAMETER;
    }
    picParams->IntraRefreshQPDelta = vaEncMiscParamRIR->qp_delta_for_inserted_intra;

    // UMD tracks the MBx/MBy for the intra square
    uint32_t rightBorder  = ((seqParams->FrameWidth + CODECHAL_ENCODE_AVC_ROI_WIDTH_SCALE_FACTOR - 1) / CODECHAL_ENCODE_AVC_ROI_WIDTH_SCALE_FACTOR) - 1;
    uint32_t bottomBorder = ((seqParams->FrameHeight + CODECHAL_ENCODE_AVC_ROI_FRAME_HEIGHT_SCALE_FACTOR - 1) / CODECHAL_ENCODE_AVC_ROI_FRAME_HEIGHT_SCALE_FACTOR) - 1;
    if ((ROLLING_I_SQUARE == picParams->EnableRollingIntraRefresh))
    {
        if (m_encodeCtx->uiIntraRefreshFrameCnt == 0)
        {
            m_encodeCtx->uiIntraRefreshFrameCnt = 1;
            m_encodeCtx->uiIntraRefreshMBx      = 0;
            m_encodeCtx->uiIntraRefreshMBy      = 0;
        }
        else
        {
            m_encodeCtx->uiIntraRefreshMBx += picParams->IntraRefreshUnitinMB;
            if (m_encodeCtx->uiIntraRefreshMBx >= rightBorder)
            {
                m_encodeCtx->uiIntraRefreshMBx = 0;
                m_encodeCtx->uiIntraRefreshMBy += picParams->IntraRefreshUnitinMB;
                if (m_encodeCtx->uiIntraRefreshMBy >= bottomBorder)
                {
                    m_encodeCtx->uiIntraRefreshMBx = 0;
                    m_encodeCtx->uiIntraRefreshMBy = 0;
                }
            }
        }
        // set MBx/MBy to CODECHAL
        picParams->IntraRefreshMBx = m_encodeCtx->uiIntraRefreshMBx;
        picParams->IntraRefreshMBy = m_encodeCtx->uiIntraRefreshMBy;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamQualityLevel(void *data)
{
    // Assume only one SPS here, modify when enable multiple SPS support
    VAEncMiscParameterBufferQualityLevel *vaEncMiscParamQualityLevel = (VAEncMiscParameterBufferQualityLevel *)data;

    m_encodeCtx->targetUsage = (uint8_t)vaEncMiscParamQualityLevel->quality_level;
    uint8_t qualityUpperBoundary = TARGETUSAGE_BEST_SPEED;
#ifdef _FULL_OPEN_SOURCE
    if (!GFX_IS_PRODUCT(m_encodeCtx->pMediaCtx->platform, IGFX_ICELAKE_LP))
        qualityUpperBoundary = 5;
#endif
    // check if TU setting is valid, otherwise change to default
    if ((m_encodeCtx->targetUsage > qualityUpperBoundary) || (0 == m_encodeCtx->targetUsage))
    {
        m_encodeCtx->targetUsage = TARGETUSAGE_RT_SPEED;
        DDI_ASSERTMESSAGE("Quality Level setting from application is not correct, should be in (0,%d].", qualityUpperBoundary);
        return VA_STATUS_SUCCESS;
    }

#ifdef _FULL_OPEN_SOURCE
    if (!GFX_IS_PRODUCT(m_encodeCtx->pMediaCtx->platform, IGFX_ICELAKE_LP))
    {
        if (m_encodeCtx->targetUsage >= 1 && m_encodeCtx->targetUsage <= 2)
        {
            m_encodeCtx->targetUsage = 4;
        }
        else if (m_encodeCtx->targetUsage >= 3 &&m_encodeCtx->targetUsage <= 5)
        {
            m_encodeCtx->targetUsage = 7;
        }
    }

#endif

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamDirtyROI(void *data)
{
    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams = (PCODEC_AVC_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams) + current_pic_parameter_set_id;
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams) + current_seq_parameter_set_id;
    VAEncMiscParameterBufferDirtyRect *dirtyRect = (VAEncMiscParameterBufferDirtyRect *)data;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(dirtyRect, "nullptr dirtyRect", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(dirtyRect->roi_rectangle, "nullptr dirtyRect->roi_rectangle", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (dirtyRect->num_roi_rectangle > 0)
    {
        uint16_t mbHeightScaleFactor = picParams->FieldCodingFlag ? CODECHAL_ENCODE_AVC_ROI_FIELD_HEIGHT_SCALE_FACTOR : CODECHAL_ENCODE_AVC_ROI_FRAME_HEIGHT_SCALE_FACTOR;

        // Support upto 4 Dirty rectangles
        int32_t maxDirtyRects = (dirtyRect->num_roi_rectangle > CODEC_AVC_NUM_MAX_DIRTY_RECT) ? CODEC_AVC_NUM_MAX_DIRTY_RECT : dirtyRect->num_roi_rectangle;

        picParams->NumDirtyROI = 0;

        MOS_ZeroMemory(picParams->DirtyROI, CODEC_AVC_NUM_MAX_DIRTY_RECT * sizeof(CODEC_ROI));

        PCODEC_ROI roi;
        for (int32_t i = 0; i < maxDirtyRects; i++)
        {
            if (nullptr != dirtyRect->roi_rectangle)
            {
                roi         = &picParams->DirtyROI[picParams->NumDirtyROI];
                roi->Left   = MOS_MIN(MOS_MAX(dirtyRect->roi_rectangle->x, 0), seqParams->FrameWidth - 1);
                roi->Top    = MOS_MIN(MOS_MAX(dirtyRect->roi_rectangle->y, 0), seqParams->FrameHeight - 1);
                roi->Right  = MOS_MIN(dirtyRect->roi_rectangle->x + dirtyRect->roi_rectangle->width, seqParams->FrameWidth - 1);
                roi->Bottom = MOS_MIN(dirtyRect->roi_rectangle->y + dirtyRect->roi_rectangle->height, seqParams->FrameHeight - 1);

                // Check and adjust if ROI not within frame boundaries
                roi->Left   = MOS_MIN(roi->Left, seqParams->FrameWidth - 1);
                roi->Top    = MOS_MIN(roi->Top, seqParams->FrameHeight - 1);
                roi->Right  = MOS_MIN(roi->Right, seqParams->FrameWidth - 1);
                roi->Bottom = MOS_MIN(roi->Bottom, seqParams->FrameHeight - 1);

                roi->Right  = MOS_ALIGN_CEIL(roi->Right, CODECHAL_ENCODE_AVC_ROI_WIDTH_SCALE_FACTOR);
                roi->Bottom = MOS_ALIGN_CEIL(roi->Bottom, mbHeightScaleFactor);

                // Convert from pixel units to MB units
                roi->Left /= CODECHAL_ENCODE_AVC_ROI_WIDTH_SCALE_FACTOR;
                roi->Right /= CODECHAL_ENCODE_AVC_ROI_WIDTH_SCALE_FACTOR;
                roi->Top /= mbHeightScaleFactor;
                roi->Bottom /= mbHeightScaleFactor;

                dirtyRect->roi_rectangle++;
                picParams->NumDirtyROI++;
            }
            else
            {
                continue;
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamROI(void *data)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams = (PCODEC_AVC_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams) + current_pic_parameter_set_id;
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams) + current_seq_parameter_set_id;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterBufferROI *vaEncMiscParamROI = (VAEncMiscParameterBufferROI *)data;
    PCODEC_ROI                   roi               = &picParams->ROI[0];

    DDI_CHK_NULL(m_encodeCtx->pMediaCtx, "nullptr pMediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pMediaCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t maxROIsupported = 0;
    bool isROIValueInDeltaQP = false;
    m_encodeCtx->pMediaCtx->m_caps->QueryAVCROIMaxNum(m_encodeCtx->uiRCMethod, m_encodeCtx->bVdencActive, &maxROIsupported, &isROIValueInDeltaQP);
    if (maxROIsupported == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    seqParams->ROIValueInDeltaQP = (isROIValueInDeltaQP ? 1 : 0);

    picParams->NumROI     = MOS_MIN(vaEncMiscParamROI->num_roi, maxROIsupported);
    picParams->MaxDeltaQp = vaEncMiscParamROI->max_delta_qp;
    picParams->MinDeltaQp = vaEncMiscParamROI->min_delta_qp;

    uint8_t mbHeightScaleFactor = picParams->FieldCodingFlag ? CODECHAL_ENCODE_AVC_ROI_FIELD_HEIGHT_SCALE_FACTOR : CODECHAL_ENCODE_AVC_ROI_FRAME_HEIGHT_SCALE_FACTOR;

    // Process  information for all ROIs and pass to codecHAL
    for (uint8_t i = 0; i < picParams->NumROI; i++)
    {
        DDI_CHK_NULL(vaEncMiscParamROI->roi, "nullptr vaEncMiscParamROI->roi", VA_STATUS_ERROR_INVALID_PARAMETER);
        DDI_CHK_NULL(roi, "nullptr roi", VA_STATUS_ERROR_INVALID_PARAMETER);

        // Check and adjust if ROI not within frame boundaries
        roi->Left   = MOS_MIN(MOS_MAX(vaEncMiscParamROI->roi->roi_rectangle.x, 0), seqParams->FrameWidth - 1);
        roi->Top    = MOS_MIN(MOS_MAX(vaEncMiscParamROI->roi->roi_rectangle.y, 0), seqParams->FrameHeight - 1);
        roi->Right  = MOS_MIN(roi->Left + vaEncMiscParamROI->roi->roi_rectangle.width, seqParams->FrameWidth - 1);
        roi->Bottom = MOS_MIN(roi->Top + vaEncMiscParamROI->roi->roi_rectangle.height, seqParams->FrameHeight - 1);

        // Convert from pixel units to MB units
        roi->Left /= CODECHAL_ENCODE_AVC_ROI_WIDTH_SCALE_FACTOR;
        roi->Top /= mbHeightScaleFactor;
        roi->Right  = (roi->Right + CODECHAL_ENCODE_AVC_ROI_WIDTH_SCALE_FACTOR - 1) / CODECHAL_ENCODE_AVC_ROI_WIDTH_SCALE_FACTOR;
        roi->Bottom = (roi->Bottom + mbHeightScaleFactor - 1) / mbHeightScaleFactor;

        roi->PriorityLevelOrDQp = vaEncMiscParamROI->roi->roi_value;

        // Move to next ROI
        vaEncMiscParamROI->roi++;
        roi++;
    }
#ifndef ANDROID
    seqParams->ROIValueInDeltaQP = vaEncMiscParamROI->roi_flags.bits.roi_value_is_qp_delta;
    if(picParams->NumROI != 0 && seqParams->ROIValueInDeltaQP == 0)
    {
        DDI_ASSERTMESSAGE("ROI does not support priority level now.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
#endif
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamMaxSliceSize(void *data)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    m_encodeCtx->EnableSliceLevelRateCtrl = true;

    PCODEC_AVC_ENCODE_PIC_PARAMS      picParams = (PCODEC_AVC_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams) + current_pic_parameter_set_id;
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams) + current_seq_parameter_set_id;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(seqParams, "nullptr seqParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    seqParams->EnableSliceLevelRateCtrl                        = m_encodeCtx->EnableSliceLevelRateCtrl;
    VAEncMiscParameterMaxSliceSize *vaEncMiscParamMaxSliceSize = (VAEncMiscParameterMaxSliceSize *)data;

    picParams->SliceSizeInBytes = vaEncMiscParamMaxSliceSize->max_slice_size;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamRounding(void *data)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterCustomRoundingControl *vaEncMiscParamRounding = (VAEncMiscParameterCustomRoundingControl *)data;

    if (vaEncMiscParamRounding->rounding_offset_setting.bits.enable_custom_rouding_intra)
    {
        m_roundingParams->bEnableCustomRoudingIntra = vaEncMiscParamRounding->rounding_offset_setting.bits.enable_custom_rouding_intra;
        m_roundingParams->dwRoundingIntra           = vaEncMiscParamRounding->rounding_offset_setting.bits.rounding_offset_intra;
    }
    if (vaEncMiscParamRounding->rounding_offset_setting.bits.enable_custom_rounding_inter)
    {
        m_roundingParams->bEnableCustomRoudingInter = vaEncMiscParamRounding->rounding_offset_setting.bits.enable_custom_rounding_inter;
        m_roundingParams->dwRoundingInter           = vaEncMiscParamRounding->rounding_offset_setting.bits.rounding_offset_inter;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParamSubMbPartPel(void *data)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_PARAMETER);

    PCODEC_AVC_ENCODE_PIC_PARAMS picParams = (PCODEC_AVC_ENCODE_PIC_PARAMS)(m_encodeCtx->pPicParams) + current_pic_parameter_set_id;
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterSubMbPartPelH264 *vaEncMiscParamSubMbPartPel = (VAEncMiscParameterSubMbPartPelH264*)data;
    if (vaEncMiscParamSubMbPartPel->disable_inter_sub_mb_partition)
    {
        picParams->bEnableSubMbPartMask = true;
        //Inter 16x16 can't be disabled
        picParams->SubMbPartMask = vaEncMiscParamSubMbPartPel->inter_sub_mb_partition_mask.value & disMbPartMask;
    }

    if (vaEncMiscParamSubMbPartPel->enable_sub_pel_mode)
    {
        picParams->bEnableSubPelMode = true;
        picParams->SubPelMode        = vaEncMiscParamSubMbPartPel->sub_pel_mode & subpelModeMask;
        if (picParams->SubPelMode == subpelModeReserved)
        {
            //When Quarter-pel mode is enabled, Half-pel must be enabled as well.
            picParams->SubPelMode = subpelModeQuant;
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ContextInitialize(CodechalSetting * codecHalSettings)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "nullptr m_encodeCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(codecHalSettings, "nullptr codecHalSettings.", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (m_encodeCtx->bVdencActive == true)
    {
        codecHalSettings->codecFunction = CODECHAL_FUNCTION_ENC_VDENC_PAK;
    }
    else
    {
        codecHalSettings->codecFunction = m_encodeCtx->codecFunction;
    }

    codecHalSettings->width  = m_encodeCtx->dworiFrameWidth;
    codecHalSettings->height = m_encodeCtx->dworiFrameHeight;
    codecHalSettings->mode     = m_encodeCtx->wModeType;
    codecHalSettings->standard = CODECHAL_AVC;

    m_encodeCtx->pSeqParams = (void *)MOS_AllocAndZeroMemory(CODEC_AVC_MAX_SPS_NUM * sizeof(CODEC_AVC_ENCODE_SEQUENCE_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pSeqParams, "nullptr m_encodeCtx->pSeqParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pPicParams = (void *)MOS_AllocAndZeroMemory(CODEC_AVC_MAX_PPS_NUM * sizeof(CODEC_AVC_ENCODE_PIC_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pPicParams, "nullptr m_encodeCtx->pPicParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate NAL unit params
    m_encodeCtx->ppNALUnitParams = (PCODECHAL_NAL_UNIT_PARAMS *)MOS_AllocAndZeroMemory(sizeof(PCODECHAL_NAL_UNIT_PARAMS) * CODECHAL_ENCODE_AVC_MAX_NAL_TYPE);
    DDI_CHK_NULL(m_encodeCtx->ppNALUnitParams, "nullptr m_encodeCtx->ppNALUnitParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    PCODECHAL_NAL_UNIT_PARAMS nalUnitParams = (PCODECHAL_NAL_UNIT_PARAMS)MOS_AllocAndZeroMemory(sizeof(CODECHAL_NAL_UNIT_PARAMS) * CODECHAL_ENCODE_AVC_MAX_NAL_TYPE);
    DDI_CHK_NULL(nalUnitParams, "nullptr nalUnitParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    for (int32_t i = 0; i < CODECHAL_ENCODE_AVC_MAX_NAL_TYPE; i++)
    {
        m_encodeCtx->ppNALUnitParams[i] = &(nalUnitParams[i]);
    }

    VAStatus status = m_encodeCtx->pCpDdiInterface->ParseCpParamsForEncode();
    if (VA_STATUS_SUCCESS != status)
    {
        return status;
    }

    m_encodeCtx->pVuiParams = (void *)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_AVC_VUI_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pVuiParams, "nullptr m_encodeCtx->pVuiParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate SliceParams
    // Supports equal row slices.
    m_encodeCtx->pSliceParams = (void *)MOS_AllocAndZeroMemory(ENCODE_AVC_MAX_SLICES_SUPPORTED * sizeof(CODEC_AVC_ENCODE_SLICE_PARAMS));
    DDI_CHK_NULL(m_encodeCtx->pSliceParams, "nullptr m_encodeCtx->pSliceParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate Encode Status Report
    m_encodeCtx->pEncodeStatusReport = (void *)MOS_AllocAndZeroMemory(CODECHAL_ENCODE_STATUS_NUM * sizeof(EncodeStatusReport));
    DDI_CHK_NULL(m_encodeCtx->pEncodeStatusReport, "nullptr m_encodeCtx->pEncodeStatusReport", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Allocate SEI structure
    m_encodeCtx->pSEIFromApp = (CodechalEncodeSeiData*)MOS_AllocAndZeroMemory(sizeof(CodechalEncodeSeiData));
    DDI_CHK_NULL(m_encodeCtx->pSEIFromApp, "nullptr m_encodeCtx->pSEIFromApp", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // for slice header from application
    m_encodeCtx->pSliceHeaderData = (PCODEC_ENCODER_SLCDATA)MOS_AllocAndZeroMemory(ENCODE_AVC_MAX_SLICES_SUPPORTED *
                                                                                   sizeof(CODEC_ENCODER_SLCDATA));
    DDI_CHK_NULL(m_encodeCtx->pSliceHeaderData, "nullptr m_encodeCtx->pSliceHeaderData", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Create the bit stream buffer to hold the packed headers from application
    m_encodeCtx->pbsBuffer = (PBSBuffer)MOS_AllocAndZeroMemory(sizeof(BSBuffer));
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer, "nullptr m_encodeCtx->pbsBuffer", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pbsBuffer->pBase = (uint8_t *)MOS_AllocAndZeroMemory(ENCODE_AVC_MAX_SLICES_SUPPORTED * PACKED_HEADER_SIZE_PER_ROW);
    DDI_CHK_NULL(m_encodeCtx->pbsBuffer->pBase, "nullptr m_encodeCtx->pbsBuffer->pBase", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_encodeCtx->pbsBuffer->BufferSize = ENCODE_AVC_MAX_SLICES_SUPPORTED * PACKED_HEADER_SIZE_PER_ROW;

    m_qcParams = (CODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS *)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS));
    DDI_CHK_NULL(m_qcParams, "nullptr m_qcParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    m_roundingParams = (CODECHAL_ENCODE_AVC_ROUNDING_PARAMS *)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_AVC_ROUNDING_PARAMS));
    DDI_CHK_NULL(m_roundingParams, "nullptr m_roundingParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    iqMatrixParams = (PCODEC_AVC_IQ_MATRIX_PARAMS)MOS_AllocAndZeroMemory(sizeof(CODEC_AVC_IQ_MATRIX_PARAMS));
    DDI_CHK_NULL(iqMatrixParams, "nullptr iqMatrixParams", VA_STATUS_ERROR_ALLOCATION_FAILED);

    iqWeightScaleLists = (PCODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS)MOS_AllocAndZeroMemory(sizeof(CODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS));
    DDI_CHK_NULL(iqWeightScaleLists, "nullptr iqWeightScaleLists", VA_STATUS_ERROR_ALLOCATION_FAILED);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          numBuffers)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t numSlices = 0;
    VAStatus vaStatus  = VA_STATUS_SUCCESS;

    DDI_MEDIA_BUFFER *buf;
    void             *data;
    uint32_t         dataSize;
    for (int32_t i = 0; i < numBuffers; i++)
    {
        buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, buffers[i]);
        DDI_CHK_NULL(buf, "Invalid buffer.", VA_STATUS_ERROR_INVALID_BUFFER);
        if (buf->uiType == VAEncMacroblockDisableSkipMapBufferType)
        {
            DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resPerMBSkipMapBuffer));
            m_encodeCtx->bMbDisableSkipMapEnabled = true;
            continue;
        }
        dataSize = buf->iSize;
        // can use internal function instead of DdiMedia_MapBuffer here?
        DdiMedia_MapBuffer(ctx, buffers[i], &data);

        DDI_CHK_NULL(data, "nullptr data", VA_STATUS_ERROR_INVALID_BUFFER);

        switch (buf->uiType)
        {
        case VAIQMatrixBufferType:
        case VAQMatrixBufferType:
            DDI_CHK_STATUS(Qmatrix(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncSequenceParameterBufferType:
            DDI_CHK_STATUS(ParseSeqParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            m_encodeCtx->bNewSeq = true;
            break;

        case VAEncPictureParameterBufferType:
            DDI_CHK_STATUS(ParsePicParams(mediaCtx, data), VA_STATUS_ERROR_INVALID_BUFFER);

            DDI_CHK_STATUS(
                    AddToStatusReportQueue((void *)m_encodeCtx->resBitstreamBuffer.bo),
                    VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncSliceParameterBufferType:
            numSlices = buf->uiNumElements;
            DDI_CHK_STATUS(ParseSlcParams(mediaCtx, data, numSlices), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncPackedHeaderParameterBufferType:
            DDI_CHK_STATUS(ParsePackedHeaderParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncPackedHeaderDataBufferType:
            vaStatus = ParsePackedHeaderData(data);
            break;

        case VAEncMiscParameterBufferType:
            DDI_CHK_STATUS(ParseMiscParams(data), VA_STATUS_ERROR_INVALID_BUFFER);
            break;

        case VAEncQPBufferType:
            DdiMedia_MediaBufferToMosResource(buf, &m_encodeCtx->resMBQpBuffer);
            m_encodeCtx->bMBQpEnable = true;
            break;

        default:
            if(m_encodeCtx->pCpDdiInterface)
            {
                vaStatus = m_encodeCtx->pCpDdiInterface->RenderCencPicture(ctx, context, buf, data);
            }
            else
            {
                vaStatus = VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE;
            }
            break;
        }
        DdiMedia_UnmapBuffer(ctx, buffers[i]);
    }

    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

VAStatus DdiEncodeAvc::EncodeInCodecHal(uint32_t numSlices)
{
    uint8_t   ppsIdx, spsIdx;
    PCODEC_AVC_ENCODE_PIC_PARAMS  picParams;

    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pMediaCtx, "nullptr m_encodeCtx->pMediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx->pCpDdiInterface, "nullptr m_encodeCtx->pCpDdiInterface.", VA_STATUS_ERROR_INVALID_PARAMETER);

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl     = &(m_encodeCtx->RTtbl);

    EncoderParams *encodeParams = &m_encodeCtx->EncodeParams;
    MOS_ZeroMemory(encodeParams, sizeof(EncoderParams));

    encodeParams->newSeqHeader           = m_newSeqHeader;
    encodeParams->newPpsHeader           = m_newPpsHeader;
    encodeParams->arbitraryNumMbsInSlice = m_arbitraryNumMbsInSlice;

    if (m_encodeCtx->bVdencActive == true)
    {
        encodeParams->ExecCodecFunction = CODECHAL_FUNCTION_ENC_VDENC_PAK;
    }
    else
    {
        encodeParams->ExecCodecFunction = CODECHAL_FUNCTION_ENC_PAK;
    }

    // Raw Surface
    PMOS_SURFACE rawSurface = &encodeParams->rawSurface;
    rawSurface->Format   = Format_NV12;
    rawSurface->dwOffset = 0;

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentRT, &(rawSurface->OsResource));

    PMOS_INTERFACE osInterface = m_encodeCtx->pCodecHal->GetOsInterface();
    m_encodeCtx->pCpDdiInterface->SetInputResourceEncryption(osInterface, &(rawSurface->OsResource));

    // Recon Surface
    PMOS_SURFACE reconSurface = &encodeParams->reconSurface;
    reconSurface->Format   = Format_NV12;
    reconSurface->dwOffset = 0;

    DdiMedia_MediaSurfaceToMosResource(rtTbl->pCurrentReconTarget, &(reconSurface->OsResource));

    //clear registered recon/ref surface flags
    DDI_CHK_RET(ClearRefList(&m_encodeCtx->RTtbl, false), "ClearRefList failed!");

    // Bitstream surface
    PMOS_RESOURCE bitstreamSurface = &encodeParams->resBitstreamBuffer;
    *bitstreamSurface        = m_encodeCtx->resBitstreamBuffer;  // in render picture
    bitstreamSurface->Format = Format_Buffer;

    encodeParams->psRawSurface = &encodeParams->rawSurface;
    encodeParams->psReconSurface = &encodeParams->reconSurface;
    encodeParams->presBitstreamBuffer = &encodeParams->resBitstreamBuffer;

    PMOS_SURFACE mbQpSurface = &encodeParams->mbQpSurface;
    if (m_encodeCtx->bMBQpEnable)
    {
        mbQpSurface->Format             = Format_Buffer_2D;
        mbQpSurface->dwOffset           = 0;
        mbQpSurface->OsResource         = m_encodeCtx->resMBQpBuffer;
        encodeParams->psMbQpDataSurface = &encodeParams->mbQpSurface;
        encodeParams->bMbQpDataEnabled  = true;
    }

    PMOS_SURFACE disableSkipMapSurface = &encodeParams->disableSkipMapSurface;
    encodeParams->bMbDisableSkipMapEnabled = m_encodeCtx->bMbDisableSkipMapEnabled;
    if (encodeParams->bMbDisableSkipMapEnabled)
    {
        disableSkipMapSurface->Format           = Format_Buffer;
        disableSkipMapSurface->dwOffset         = 0;
        disableSkipMapSurface->OsResource       = m_encodeCtx->resPerMBSkipMapBuffer;
        encodeParams->psMbDisableSkipMapSurface = &encodeParams->disableSkipMapSurface;
    }

    // correct some params
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams;
    CODECHAL_ENCODE_AVC_VUI_PARAMS    *vuiParam;

    vuiParam  = (CODECHAL_ENCODE_AVC_VUI_PARAMS *)m_encodeCtx->pVuiParams;
    seqParams = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    seqParams->TargetUsage = m_encodeCtx->targetUsage;
    if (VA_RC_CQP == m_encodeCtx->uiRCMethod)
    {
        vuiParam->bit_rate_value_minus1[0]    = 0;
        vuiParam->cpb_size_value_minus1[0]    = 0;
        seqParams->TargetBitRate              = 0;
        seqParams->MaxBitRate                 = 0;
        seqParams->MinBitRate                 = 0;
        seqParams->InitVBVBufferFullnessInBit = 0;
        seqParams->VBVBufferSizeInBit         = 0;
    }

    encodeParams->uiSlcStructCaps = CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE;

    ppsIdx                           = ((PCODEC_AVC_ENCODE_SLICE_PARAMS)(m_encodeCtx->pSliceParams))->pic_parameter_set_id;
    picParams                        = (PCODEC_AVC_ENCODE_PIC_PARAMS)m_encodeCtx->pPicParams + ppsIdx;
    spsIdx                           = picParams->seq_parameter_set_id;
    encodeParams->pSeqParams         = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)m_encodeCtx->pSeqParams + spsIdx;
    encodeParams->pPicParams         = picParams;

    encodeParams->pVuiParams         = m_encodeCtx->pVuiParams;
    encodeParams->pSliceParams       = m_encodeCtx->pSliceParams;
    encodeParams->pAVCQCParams       = m_qcParams;
    encodeParams->pAVCRoundingParams = m_roundingParams;

    // Sequence data
    encodeParams->bNewSeq = m_encodeCtx->bNewSeq;
    // VUI
    encodeParams->bNewVuiData = m_encodeCtx->bNewVuiData;

    // Slice level data
    encodeParams->dwNumSlices = numSlices;

    // IQmatrix params
    encodeParams->bNewQmatrixData = m_encodeCtx->bNewQmatrixData;
    encodeParams->bPicQuant       = m_encodeCtx->bPicQuant;
    encodeParams->ppNALUnitParams = m_encodeCtx->ppNALUnitParams;
    encodeParams->pSeiData        = m_encodeCtx->pSEIFromApp;
    encodeParams->pSeiParamBuffer = m_encodeCtx->pSEIFromApp->pSEIBuffer;
    encodeParams->dwSEIDataOffset = 0;

    MOS_STATUS status = MOS_SecureMemcpy(&iqMatrixParams->ScalingList4x4,
        6 * 16 * sizeof(uint8_t),
        &m_scalingLists4x4,
        6 * 16 * sizeof(uint8_t));
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed to copy scaling list 4x4!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    status = MOS_SecureMemcpy(&iqMatrixParams->ScalingList8x8,
        2 * 64 * sizeof(uint8_t),
        &m_scalingLists8x8,
        2 * 64 * sizeof(uint8_t));
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed to copy scaling list 8x8!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    encodeParams->pIQMatrixBuffer = iqMatrixParams;

    status = MOS_SecureMemcpy(&iqWeightScaleLists->WeightScale4x4,
        (CODEC_AVC_WEIGHT_SCALE_4x4 * sizeof(uint32_t)),
        &m_weightScale4x4,
        (CODEC_AVC_WEIGHT_SCALE_4x4 * sizeof(uint32_t)));
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed to copy weight scale list 4x4!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    status = MOS_SecureMemcpy(&iqWeightScaleLists->WeightScale8x8,
        (CODEC_AVC_WEIGHT_SCALE_8x8 * sizeof(uint32_t)),
        &m_weightScale8x8,
        (CODEC_AVC_WEIGHT_SCALE_8x8 * sizeof(uint32_t)));
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("DDI:Failed to copy weight scale list 8x8!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    encodeParams->pIQWeightScaleLists = iqWeightScaleLists;

    // whether driver need to pack slice header
    if (true == m_encodeCtx->bHavePackedSliceHdr)
    {
        encodeParams->bAcceleratorHeaderPackingCaps = false;
    }
    else
    {
        encodeParams->bAcceleratorHeaderPackingCaps = true;
    }

    encodeParams->pBSBuffer      = m_encodeCtx->pbsBuffer;
    encodeParams->pSlcHeaderData = (void *)m_encodeCtx->pSliceHeaderData;

    CodechalEncoderState *encoder = dynamic_cast<CodechalEncoderState *>(m_encodeCtx->pCodecHal);
    DDI_CHK_NULL(encoder, "nullptr Codechal encode", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (!encoder->m_mfeEnabled)
    {
        status = m_encodeCtx->pCodecHal->Execute(encodeParams);
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:Failed in Codechal!");
            return VA_STATUS_ERROR_ENCODING_ERROR;
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ResetAtFrameLevel()
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Assume there is only one SPS parameter
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(m_encodeCtx->pSeqParams);
    seqParams->bInitBRC                         = 0x0;
    seqParams->bResetBRC                        = 0x0;

    m_encodeCtx->dwNumSlices      = 0x0;
    m_encodeCtx->indexNALUnit     = 0x0;
    m_encodeCtx->uiSliceHeaderCnt = 0x0;

    // reset bsbuffer every frame
    m_encodeCtx->pbsBuffer->pCurrent    = m_encodeCtx->pbsBuffer->pBase;
    m_encodeCtx->pbsBuffer->SliceOffset = 0x0;
    m_encodeCtx->pbsBuffer->BitOffset   = 0x0;
    m_encodeCtx->pbsBuffer->BitSize     = 0x0;

    // clear the packed header information
    if (nullptr != m_encodeCtx->ppNALUnitParams)
    {
        MOS_ZeroMemory(m_encodeCtx->ppNALUnitParams[0], sizeof(CODECHAL_NAL_UNIT_PARAMS) * CODECHAL_ENCODE_AVC_MAX_NAL_TYPE);
    }

    m_encodeCtx->bHavePackedSliceHdr      = false;
    m_encodeCtx->bLastPackedHdrIsSlice    = false;
    m_encodeCtx->bMbDisableSkipMapEnabled = false;
    m_encodeCtx->bMBQpEnable              = false;

    if (nullptr != m_roundingParams)
    {
        MOS_ZeroMemory(m_roundingParams, sizeof(CODECHAL_ENCODE_AVC_ROUNDING_PARAMS));
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::Qmatrix(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAIQMatrixBufferH264 *qm = (VAIQMatrixBufferH264 *)ptr;

    MOS_STATUS status = MOS_SecureMemcpy((void *)&m_scalingLists4x4,
        6 * 16 * sizeof(uint8_t),
        (void *)&qm->ScalingList4x4,
        6 * 16 * sizeof(uint8_t));
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("Failed to copy QM scaling list 4x4.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    MOS_SecureMemcpy((void *)&m_scalingLists8x8,
        2 * 64 * sizeof(uint8_t),
        (void *)&qm->ScalingList8x8,
        2 * 64 * sizeof(uint8_t));
    if (MOS_STATUS_SUCCESS != status)
    {
        DDI_ASSERTMESSAGE("Failed to copy QM scaling list 8x8.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    uint8_t idx1, idx2;
    // 4x4 block
    for (idx2 = 0; idx2 < 6; idx2++)
    {
        for (idx1 = 0; idx1 < 16; idx1++)
        {
            m_weightScale4x4[idx2][CODEC_AVC_Qmatrix_scan_4x4[idx1]] = qm->ScalingList4x4[idx2][idx1];
        }
    }
    // 8x8 block
    for (idx2 = 0; idx2 < 2; idx2++)
    {
        for (idx1 = 0; idx1 < 64; idx1++)
        {
            m_weightScale8x8[idx2][CODEC_AVC_Qmatrix_scan_8x8[idx1]] = qm->ScalingList8x8[idx2][idx1];
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseSeqParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncSequenceParameterBufferH264 *seq       = (VAEncSequenceParameterBufferH264 *)ptr;
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)((uint8_t *)m_encodeCtx->pSeqParams + seq->seq_parameter_set_id * sizeof(CODEC_AVC_ENCODE_SEQUENCE_PARAMS));

    if (seq->seq_parameter_set_id >= CODEC_AVC_MAX_SPS_NUM)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    seqParams->FrameWidth  = seq->picture_width_in_mbs * 16;
    seqParams->FrameHeight = seq->picture_height_in_mbs * 16;
    if ((seq->picture_width_in_mbs <= m_encodeCtx->wContextPicWidthInMB) && (seq->picture_height_in_mbs <= m_encodeCtx->wContextPicHeightInMB) && (seq->picture_width_in_mbs != m_encodeCtx->wOriPicWidthInMB) && (seq->picture_height_in_mbs != m_encodeCtx->wOriPicHeightInMB))
    {
        m_encodeCtx->wPicWidthInMB     = seq->picture_width_in_mbs;
        m_encodeCtx->wPicHeightInMB    = seq->picture_height_in_mbs;
        m_encodeCtx->wOriPicWidthInMB  = m_encodeCtx->wPicWidthInMB;
        m_encodeCtx->wOriPicHeightInMB = m_encodeCtx->wPicHeightInMB;
        seqParams->bInitBRC            = true;
    }
    else if ((seq->picture_width_in_mbs > m_encodeCtx->wContextPicWidthInMB) || (seq->picture_height_in_mbs > m_encodeCtx->wContextPicHeightInMB))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    seqParams->Profile              = GetAVCProfileFromVAProfile();
    seqParams->Level                = seq->level_idc;
    seqParams->seq_parameter_set_id = seq->seq_parameter_set_id;
    seqParams->chroma_format_idc    = 1;

    seqParams->bit_depth_luma_minus8   = seq->bit_depth_luma_minus8;
    seqParams->bit_depth_chroma_minus8 = seq->bit_depth_chroma_minus8;

    seqParams->GopPicSize = seq->intra_period;
    seqParams->GopRefDist = seq->ip_period;
    seqParams->RateControlMethod = VARC2HalRC(m_encodeCtx->uiRCMethod);

    seqParams->TargetBitRate = seq->bits_per_second;
    seqParams->MaxBitRate    = seq->bits_per_second;
    seqParams->MinBitRate    = seq->bits_per_second;
    // fps it set to 30 by default, can be overwritten by misc paramter
    if(!seqParams->FramesPer100Sec)
    {
        seqParams->FramesPer100Sec = 3000;
    }


    // set default same as MSDK, can be overwritten by HRD params
    seqParams->InitVBVBufferFullnessInBit = seq->bits_per_second;
    seqParams->VBVBufferSizeInBit         = seq->bits_per_second << 1;

    seqParams->NumRefFrames = seq->max_num_ref_frames;

    seqParams->log2_max_frame_num_minus4             = seq->seq_fields.bits.log2_max_frame_num_minus4;
    seqParams->pic_order_cnt_type                    = seq->seq_fields.bits.pic_order_cnt_type;
    seqParams->log2_max_pic_order_cnt_lsb_minus4     = seq->seq_fields.bits.log2_max_pic_order_cnt_lsb_minus4;
    seqParams->num_ref_frames_in_pic_order_cnt_cycle = seq->num_ref_frames_in_pic_order_cnt_cycle;
    seqParams->delta_pic_order_always_zero_flag      = seq->seq_fields.bits.delta_pic_order_always_zero_flag;
    seqParams->direct_8x8_inference_flag             = seq->seq_fields.bits.direct_8x8_inference_flag;
    seqParams->vui_parameters_present_flag           = seq->vui_parameters_present_flag;
    seqParams->frame_mbs_only_flag                   = seq->seq_fields.bits.frame_mbs_only_flag;
    seqParams->offset_for_non_ref_pic                = seq->offset_for_non_ref_pic;
    seqParams->offset_for_top_to_bottom_field        = seq->offset_for_top_to_bottom_field;
    for (uint8_t i = 0; i < seqParams->num_ref_frames_in_pic_order_cnt_cycle; i++)
    {
        seqParams->offset_for_ref_frame[i] = seq->offset_for_ref_frame[i];
    }

    seqParams->frame_cropping_flag      = seq->frame_cropping_flag;
    seqParams->frame_crop_bottom_offset = seq->frame_crop_bottom_offset;
    seqParams->frame_crop_left_offset   = seq->frame_crop_left_offset;
    seqParams->frame_crop_right_offset  = seq->frame_crop_right_offset;
    seqParams->frame_crop_top_offset    = seq->frame_crop_top_offset;

    // set up VUI parameter if rate control is enabled
    if (seqParams->vui_parameters_present_flag)
    {
        CODECHAL_ENCODE_AVC_VUI_PARAMS *vuiParam;

        vuiParam                                          = (CODECHAL_ENCODE_AVC_VUI_PARAMS *)m_encodeCtx->pVuiParams;
        vuiParam->nal_hrd_parameters_present_flag         = 1;
        vuiParam->cpb_cnt_minus1                          = 0;
        vuiParam->bit_rate_scale                          = 0;
        vuiParam->cpb_size_scale                          = 6;
        vuiParam->bit_rate_value_minus1[0]                = MOS_ROUNDUP_SHIFT(seq->bits_per_second, 6 + vuiParam->bit_rate_scale) - 1;
        vuiParam->cpb_size_value_minus1[0]                = MOS_ROUNDUP_DIVIDE(seqParams->VBVBufferSizeInBit, vuiKbps) - 1;
        vuiParam->cbr_flag                                = 0x0;
        vuiParam->initial_cpb_removal_delay_length_minus1 = 23;
        vuiParam->cpb_removal_delay_length_minus1         = 23;
        vuiParam->dpb_output_delay_length_minus1          = 23;
        vuiParam->time_offset_length                      = 24;

        vuiParam->timing_info_present_flag = seq->vui_fields.bits.timing_info_present_flag;
        vuiParam->num_units_in_tick        = seq->num_units_in_tick;
        vuiParam->time_scale               = seq->time_scale;
        vuiParam->fixed_frame_rate_flag    = 1;

        vuiParam->bitstream_restriction_flag              = seq->vui_fields.bits.bitstream_restriction_flag;
        vuiParam->motion_vectors_over_pic_boundaries_flag = 1;
        vuiParam->max_bytes_per_pic_denom                 = 2;
        vuiParam->max_bits_per_mb_denom                   = 1;
        vuiParam->max_dec_frame_buffering                 = seq->max_num_ref_frames + 1;
        vuiParam->num_reorder_frames                      = seq->max_num_ref_frames;
        vuiParam->log2_max_mv_length_horizontal           = seq->vui_fields.bits.log2_max_mv_length_horizontal;
        vuiParam->log2_max_mv_length_vertical             = seq->vui_fields.bits.log2_max_mv_length_vertical;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParsePicParams(
    PDDI_MEDIA_CONTEXT mediaCtx,
    void               *ptr)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncPictureParameterBufferH264 *pic       = (VAEncPictureParameterBufferH264 *)ptr;
    PCODEC_AVC_ENCODE_PIC_PARAMS     picParams = (PCODEC_AVC_ENCODE_PIC_PARAMS)((uint8_t *)m_encodeCtx->pPicParams + pic->pic_parameter_set_id * sizeof(CODEC_AVC_ENCODE_PIC_PARAMS));
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    current_pic_parameter_set_id = pic->pic_parameter_set_id;
    current_seq_parameter_set_id = pic->seq_parameter_set_id;
    MOS_ZeroMemory(picParams, sizeof(CODEC_AVC_ENCODE_PIC_PARAMS));

    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)((uint8_t *)m_encodeCtx->pSeqParams + pic->seq_parameter_set_id * sizeof(CODEC_AVC_ENCODE_SEQUENCE_PARAMS));
    if ((pic->seq_parameter_set_id >= CODEC_AVC_MAX_SPS_NUM) ||
        (pic->pic_parameter_set_id >= CODEC_AVC_MAX_PPS_NUM))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (pic->CurrPic.flags == VA_PICTURE_H264_TOP_FIELD ||
        pic->CurrPic.flags == VA_PICTURE_H264_BOTTOM_FIELD)
    {
        picParams->FieldCodingFlag = 1;
    }

    if (pic->CurrPic.picture_id != VA_INVALID_SURFACE)
    {
        RegisterRTSurfaces(&(m_encodeCtx->RTtbl),
            DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx,
                pic->CurrPic.picture_id));
    }

    // Curr Recon Pic
    SetupCodecPicture(mediaCtx, &(m_encodeCtx->RTtbl), &picParams->CurrReconstructedPic,pic->CurrPic, picParams->FieldCodingFlag, false, false);
    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_encodeCtx->RTtbl);

    rtTbl->pCurrentReconTarget = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, pic->CurrPic.picture_id);
    // The surface for reconstructed frame is not registered, return error to app
    if (nullptr == rtTbl->pCurrentReconTarget)
    {
        DDI_ASSERTMESSAGE("invalid surface for reconstructed frame");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // curr orig pic
    // Hard to understand here!
    picParams->CurrOriginalPic.FrameIdx = (uint8_t)GetRenderTargetID(rtTbl, rtTbl->pCurrentReconTarget);
    picParams->CurrOriginalPic.PicFlags = picParams->CurrReconstructedPic.PicFlags;

    // The surface for reconstructed frame is not registered, return error to app
    if (picParams->CurrOriginalPic.FrameIdx == 0xFF)
    {
        DDI_ASSERTMESSAGE("unregistered surface for reconstructed frame");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // RefFrame List
    for (uint32_t i = 0; i < DDI_CODEC_NUM_MAX_REF_FRAME; i++)
    {
        if(pic->ReferenceFrames[i].picture_id!= VA_INVALID_SURFACE)
        {
            UpdateRegisteredRTSurfaceFlag(&(m_encodeCtx->RTtbl),
                DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx,
                    pic->ReferenceFrames[i].picture_id));
        }
        SetupCodecPicture(mediaCtx, &(m_encodeCtx->RTtbl), &(picParams->RefFrameList[i]), pic->ReferenceFrames[i], picParams->FieldCodingFlag, true, false);
    }

    for (uint32_t i = 0; i < DDI_CODEC_NUM_MAX_REF_FRAME; i++)
    {
        picParams->FieldOrderCntList[i][0] = pic->ReferenceFrames[i].TopFieldOrderCnt;
        picParams->FieldOrderCntList[i][1] = pic->ReferenceFrames[i].BottomFieldOrderCnt;
    }

    picParams->seq_parameter_set_id = pic->seq_parameter_set_id;
    picParams->pic_parameter_set_id = pic->pic_parameter_set_id;
    /* pic->coding_type; App is always setting this to 0 */
    picParams->CodingType                    = I_TYPE;
    picParams->second_chroma_qp_index_offset = pic->second_chroma_qp_index_offset;
    picParams->num_ref_idx_l0_active_minus1  = pic->num_ref_idx_l0_active_minus1;
    picParams->num_ref_idx_l1_active_minus1  = pic->num_ref_idx_l1_active_minus1;
    picParams->QpY                           = pic->pic_init_qp;

    if (pic->CurrPic.flags == VA_PICTURE_H264_SHORT_TERM_REFERENCE ||
        pic->CurrPic.flags == VA_PICTURE_H264_LONG_TERM_REFERENCE)
    {
        picParams->UsedForReferenceFlags = 1;
    }
    picParams->CurrFieldOrderCnt[0] = pic->CurrPic.TopFieldOrderCnt;
    picParams->CurrFieldOrderCnt[1] = pic->CurrPic.BottomFieldOrderCnt;
    picParams->frame_num            = pic->frame_num;

    picParams->bLastPicInSeq          = (pic->last_picture & H264_LAST_PICTURE_EOSEQ) ? 1 : 0;
    picParams->bLastPicInStream       = (pic->last_picture & H264_LAST_PICTURE_EOSTREAM) ? 1 : 0;
    picParams->chroma_qp_index_offset = pic->chroma_qp_index_offset;
    picParams->bIdrPic                         = pic->pic_fields.bits.idr_pic_flag;
    picParams->RefPicFlag                      = pic->pic_fields.bits.reference_pic_flag;
    picParams->entropy_coding_mode_flag        = pic->pic_fields.bits.entropy_coding_mode_flag;
    picParams->weighted_pred_flag              = pic->pic_fields.bits.weighted_pred_flag;
    picParams->weighted_bipred_idc             = pic->pic_fields.bits.weighted_bipred_idc;
    picParams->constrained_intra_pred_flag     = pic->pic_fields.bits.constrained_intra_pred_flag;
    picParams->transform_8x8_mode_flag         = pic->pic_fields.bits.transform_8x8_mode_flag;
    picParams->pic_order_present_flag          = pic->pic_fields.bits.pic_order_present_flag;
    picParams->pic_scaling_matrix_present_flag = pic->pic_fields.bits.pic_scaling_matrix_present_flag;
    for (uint32_t i = 0; i < 12; i++)
    {
        picParams->pic_scaling_list_present_flag[i] = pic->pic_fields.bits.pic_scaling_matrix_present_flag;
    }

    // pps, sps header packing in app
    //Slice header packing will be done in app where bDisableAccelaratorHeaderPacking flag is defaulted to 0
    //this flag shouldnt be true for slice header packing
    picParams->UserFlags.bDisableAcceleratorHeaderPacking = true;
    picParams->UserFlags.bUseRawPicForRef                 = false;  //true;

    //Reset it to zero now
    picParams->NumSlice = 0;

    //Hardcode SFD mvThreshold to 80.
    picParams->dwZMvThreshold = 80;

    DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, pic->coded_buf);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    // MSDK will re-use the buffer so need to remove before adding to status report again
    RemoveFromStatusReportQueue(buf);
    DdiMedia_MediaBufferToMosResource(buf, &(m_encodeCtx->resBitstreamBuffer));

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseSlcParams(
    PDDI_MEDIA_CONTEXT mediaCtx,
    void               *ptr,
    uint32_t           numSlices)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    CodechalEncoderState *encoder = dynamic_cast<CodechalEncoderState *>(m_encodeCtx->pCodecHal);
    DDI_CHK_NULL(encoder, "nullptr codehal encoder", VA_STATUS_ERROR_INVALID_CONTEXT);
    VAEncSliceParameterBufferH264 *slc       = (VAEncSliceParameterBufferH264 *)ptr;
    PCODEC_AVC_ENCODE_SLICE_PARAMS slcParams = (CODEC_AVC_ENCODE_SLICE_PARAMS *)m_encodeCtx->pSliceParams;
    PCODEC_AVC_ENCODE_PIC_PARAMS   picParams = (PCODEC_AVC_ENCODE_PIC_PARAMS)((uint8_t *)m_encodeCtx->pPicParams + slc->pic_parameter_set_id * sizeof(CODEC_AVC_ENCODE_PIC_PARAMS));
    DDI_CHK_NULL(slcParams, "nullptr slcParams", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(picParams, "nullptr picParams", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint8_t  numSlcs          = 0;
    uint32_t totalMBs         = m_encodeCtx->wPicWidthInMB * m_encodeCtx->wPicHeightInMB;
    uint32_t usedMBs          = 0;
    uint32_t mBScalerForField = 1;  //related with frame/field coding
    uint32_t sliceHeightInMB  = 1;
    uint32_t sliceActualMBs   = 0;

    if (slc->pic_parameter_set_id >= CODEC_AVC_MAX_PPS_NUM)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    numSlcs = picParams->NumSlice;

    if (numSlcs == 0)
    {
        picParams->CodingType = CodechalPicTypeFromVaSlcType(slc->slice_type);
    }

    // Disable rolling intra refresh if not a P-type picture
    if ((picParams->EnableRollingIntraRefresh != ROLLING_I_DISABLED) && (picParams->CodingType != P_TYPE))
    {
        picParams->EnableRollingIntraRefresh = ROLLING_I_DISABLED;
        m_encodeCtx->uiIntraRefreshFrameCnt  = 0;
        m_encodeCtx->uiIntraRefreshMBx       = 0;
        m_encodeCtx->uiIntraRefreshMBy       = 0;
    }

    slcParams += numSlcs;
    MOS_ZeroMemory(
        slcParams,
        numSlices * sizeof(CODEC_AVC_ENCODE_SLICE_PARAMS));

    if (numSlcs != 0)
    {
        slcParams--;
        usedMBs = (slcParams->NumMbsForSlice + slcParams->first_mb_in_slice) / mBScalerForField;
        if (usedMBs >= totalMBs)
        {
            return VA_STATUS_SUCCESS;
        }
        slcParams++;
    }

    uint32_t slcCount;
    uint32_t i;
    for (slcCount = 0; slcCount < numSlices; slcCount++)
    {
        if (usedMBs >= totalMBs)
            break;

        sliceActualMBs = slc->num_macroblocks;
        if (sliceActualMBs % m_encodeCtx->wPicWidthInMB)  // If any slice is a partial row of macroblocks, then set flag to indicate sliceMapSurface is required.
        {
            m_arbitraryNumMbsInSlice = 1;
        }

        if (sliceActualMBs + usedMBs > totalMBs)
            sliceActualMBs = totalMBs - usedMBs;

        slcParams->NumMbsForSlice    = sliceActualMBs * mBScalerForField;
        slcParams->first_mb_in_slice = usedMBs * mBScalerForField;
        usedMBs += sliceActualMBs;

        slcParams->slice_type                       = slc->slice_type;
        slcParams->pic_parameter_set_id             = slc->pic_parameter_set_id;
        slcParams->idr_pic_id                       = slc->idr_pic_id;
        slcParams->pic_order_cnt_lsb                = slc->pic_order_cnt_lsb;
        slcParams->delta_pic_order_cnt_bottom       = slc->delta_pic_order_cnt_bottom;
        slcParams->delta_pic_order_cnt[0]           = slc->delta_pic_order_cnt[0];
        slcParams->delta_pic_order_cnt[1]           = slc->delta_pic_order_cnt[1];
        slcParams->direct_spatial_mv_pred_flag      = slc->direct_spatial_mv_pred_flag;
        slcParams->num_ref_idx_active_override_flag = slc->num_ref_idx_active_override_flag;
        slcParams->num_ref_idx_l0_active_minus1     = slc->num_ref_idx_l0_active_minus1;
        slcParams->num_ref_idx_l1_active_minus1     = slc->num_ref_idx_l1_active_minus1;
        slcParams->luma_log2_weight_denom           = slc->luma_log2_weight_denom;
        slcParams->chroma_log2_weight_denom         = slc->chroma_log2_weight_denom;
        slcParams->cabac_init_idc                   = slc->cabac_init_idc;
        slcParams->slice_qp_delta                   = slc->slice_qp_delta;
        slcParams->disable_deblocking_filter_idc    = slc->disable_deblocking_filter_idc;
        slcParams->slice_alpha_c0_offset_div2       = slc->slice_alpha_c0_offset_div2;
        slcParams->slice_beta_offset_div2           = slc->slice_beta_offset_div2;

        slcParams->num_ref_idx_l0_active_minus1_from_DDI = slcParams->num_ref_idx_l0_active_minus1;
        slcParams->num_ref_idx_l1_active_minus1_from_DDI = slcParams->num_ref_idx_l1_active_minus1;
        slcParams->slice_id              = numSlcs + slcCount;
        slcParams->luma_weight_flag[0]   = slc->luma_weight_l0_flag;
        slcParams->chroma_weight_flag[0] = slc->chroma_weight_l0_flag;
        slcParams->luma_weight_flag[1]   = slc->luma_weight_l1_flag;
        slcParams->chroma_weight_flag[1] = slc->chroma_weight_l1_flag;

        for (i = 0; i < 32; i++)
        {
            // list 0
            if ((slc->luma_weight_l0_flag) &&
                ((slc->luma_weight_l0[i] != 0) || (slc->luma_offset_l0[i] != 0)))
            {
                slcParams->Weights[0][i][0][0] = slc->luma_weight_l0[i];  // Y weight
                slcParams->Weights[0][i][0][1] = slc->luma_offset_l0[i];  // Y offset
            }
            else
            {
                slcParams->Weights[0][i][0][0] = 1 << slcParams->luma_log2_weight_denom;  // Y weight
                slcParams->Weights[0][i][0][1] = 0;                                       // Y offset
            }

            if ((slc->chroma_weight_l0_flag) &&
                ((slc->chroma_weight_l0[i][0] != 0) || (slc->chroma_offset_l0[i][0] != 0) ||
                    (slc->chroma_weight_l0[i][1] != 0) || (slc->chroma_offset_l0[i][1] != 0)))
            {
                slcParams->Weights[0][i][1][0] = slc->chroma_weight_l0[i][0];  // Cb weight
                slcParams->Weights[0][i][1][1] = slc->chroma_offset_l0[i][0];  // Cb offset
                slcParams->Weights[0][i][2][0] = slc->chroma_weight_l0[i][1];  // Cr weight
                slcParams->Weights[0][i][2][1] = slc->chroma_offset_l0[i][1];  // Cr offset
            }
            else
            {
                slcParams->Weights[0][i][1][0] = 1;  // Cb weight
                slcParams->Weights[0][i][1][1] = 0;  // Cb offset
                slcParams->Weights[0][i][2][0] = 1;  // Cr weight
                slcParams->Weights[0][i][2][1] = 0;  // Cr offset
            }

            // list 1
            if ((slc->luma_weight_l1_flag) &&
                ((slc->luma_weight_l1[i] != 0) || (slc->luma_offset_l1[i] != 0)))
            {
                slcParams->Weights[1][i][0][0] = slc->luma_weight_l1[i];  // Y weight
                slcParams->Weights[1][i][0][1] = slc->luma_offset_l1[i];  // Y offset
            }
            else
            {
                slcParams->Weights[1][i][0][0] = 1 << slcParams->luma_log2_weight_denom;  // Y weight
                slcParams->Weights[1][i][0][1] = 0;                                       // Y offset
            }

            if ((slc->chroma_weight_l1_flag) &&
                ((slc->chroma_weight_l1[i][0] != 0) || (slc->chroma_offset_l1[i][0] != 0) ||
                    (slc->chroma_weight_l1[i][1] != 0) || (slc->chroma_offset_l1[i][1] != 0)))
            {
                slcParams->Weights[1][i][1][0] = slc->chroma_weight_l1[i][0];  // Cb weight
                slcParams->Weights[1][i][1][1] = slc->chroma_offset_l1[i][0];  // Cb offset

                slcParams->Weights[1][i][2][0] = slc->chroma_weight_l1[i][1];  // Cr weight
                slcParams->Weights[1][i][2][1] = slc->chroma_offset_l1[i][1];  // Cr offset
            }
            else
            {
                slcParams->Weights[1][i][1][0] = 1;  // Cb weight
                slcParams->Weights[1][i][1][1] = 0;  // Cb offset

                slcParams->Weights[1][i][2][0] = 1;  // Cr weight
                slcParams->Weights[1][i][2][1] = 0;  // Cr offset
            }
        }

        for (i = 0; i < CODEC_MAX_NUM_REF_FIELD; i++)
        {
            SetupCodecPicture(mediaCtx, &(m_encodeCtx->RTtbl), &(slcParams->RefPicList[0][i]), slc->RefPicList0[i], picParams->FieldCodingFlag, false, true);
            GetSlcRefIdx(&(picParams->RefFrameList[0]), &(slcParams->RefPicList[0][i]));
        }
        for (i = 0; i < CODEC_MAX_NUM_REF_FIELD; i++)
        {
            SetupCodecPicture(mediaCtx, &(m_encodeCtx->RTtbl), &(slcParams->RefPicList[1][i]), slc->RefPicList1[i], picParams->FieldCodingFlag, false, true);
            GetSlcRefIdx(&(picParams->RefFrameList[0]), &(slcParams->RefPicList[1][i]));
        }

        slc++;
        slcParams++;
    }

    picParams->NumSlice += slcCount;
    if (picParams->NumSlice > ENCODE_AVC_MAX_SLICES_SUPPORTED)
    {
        DDI_ASSERTMESSAGE("Number of slices exceeds max supported");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_encodeCtx->dwNumSlices = picParams->NumSlice;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::FindNalUnitStartCodes(
    uint8_t * buf,
    uint32_t size,
    uint32_t * startCodesOffset,
    uint32_t * startCodesLength)
{
    uint8_t i = 0;

    while (((i + 3) < size) &&
           (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01) &&
           (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0 || buf[i+3] != 0x01))
    {
        i++;
    }

    if ((i + 3) == size)
    {
        if (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01)
        {
            return VA_STATUS_ERROR_INVALID_BUFFER; //NALU start codes doesn't exit
        }
        else
        {
            *startCodesOffset = size - 3;
            *startCodesLength = 3;
            return VA_STATUS_SUCCESS;
        }
    }

    if (buf[i] != 0 || buf[i+1] != 0 || buf[i+2] != 0x01)
    {
        *startCodesOffset = i;
        *startCodesLength = 4;
    }
    else
    {
        *startCodesOffset = i;
        *startCodesLength = 3;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParsePackedHeaderParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    m_encodeCtx->bLastPackedHdrIsSlice                        = false;
    VAEncPackedHeaderParameterBuffer *encPackedHeaderParamBuf = (VAEncPackedHeaderParameterBuffer *)ptr;
    CODECHAL_ENCODE_AVC_NAL_UNIT_TYPE nalUnitType;
    if (encPackedHeaderParamBuf->type == VAEncPackedHeaderH264_SPS)
    {
        nalUnitType    = CODECHAL_ENCODE_AVC_NAL_UT_SPS;
        m_newSeqHeader = 1;
    }
    else if (encPackedHeaderParamBuf->type == VAEncPackedHeaderH264_PPS)
    {
        nalUnitType    = CODECHAL_ENCODE_AVC_NAL_UT_PPS;
        m_newPpsHeader = 1;
    }
    else if (encPackedHeaderParamBuf->type == VAEncPackedHeaderH264_Slice)
    {
        nalUnitType                        = CODECHAL_ENCODE_AVC_NAL_UT_SLICE;
        m_encodeCtx->bLastPackedHdrIsSlice = true;
        m_encodeCtx->bHavePackedSliceHdr   = true;

        // check the slice header number. Max supported is AVC_ENCODE_MAX_SLICES_SUPPORTED
        if (m_encodeCtx->uiSliceHeaderCnt >= ENCODE_AVC_MAX_SLICES_SUPPORTED)
        {
            DDI_ASSERTMESSAGE("Number of slices exceeds max supported");
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }

        // get the packed header size
        m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].BitSize                = encPackedHeaderParamBuf->bit_length;
        //don't know NALU start codes now, assign to 4 here when has_emulation_bytes is 0 and later will correct it
        m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].SkipEmulationByteCount = (encPackedHeaderParamBuf->has_emulation_bytes) ? (encPackedHeaderParamBuf->bit_length + 7) / 8 : 4;
    }
    else if (encPackedHeaderParamBuf->type == VAEncPackedHeaderRawData)
    {
        // currently RawData is packed nal unit beside SPS,PPS,SLICE HEADER etc.
        // use AUD just because we need a Type to distinguish with Slice header
        nalUnitType = CODECHAL_ENCODE_AVC_NAL_UT_AUD;
    }
    else
    {
        nalUnitType = CODECHAL_ENCODE_AVC_MAX_NAL_TYPE;
    }

    if (encPackedHeaderParamBuf->type != VAEncPackedHeaderH264_Slice)
    {
        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiNalUnitType             = nalUnitType;
        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->bInsertEmulationBytes     = (encPackedHeaderParamBuf->has_emulation_bytes) ? false : true;
        //don't know NALU start codes now, assign to 4 here when has_emulation_bytes is 0 and later will correct it
        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSkipEmulationCheckCount = (encPackedHeaderParamBuf->has_emulation_bytes) ? (encPackedHeaderParamBuf->bit_length + 7) / 8 : 4;
        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSize                    = (encPackedHeaderParamBuf->bit_length + 7) / 8;
        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiOffset                  = 0;
    }

    return VA_STATUS_SUCCESS;
}

AvcOutBits::AvcOutBits(uint8_t *pOutBits, uint32_t BitSize)
{
    m_pOutBits = pOutBits;
    m_BitSize = BitSize;
    m_BitOffset = 0;
}

inline uint32_t AvcOutBits::GetBitOffset()
{
    return m_BitOffset;
}

void AvcOutBits::PutBit(uint32_t v)
{
    DDI_ASSERT(m_BitOffset + 1 <= m_BitSize);

    uint32_t LeftOffset = m_BitOffset % 8;
    uint8_t *p = m_pOutBits + m_BitOffset / 8;
    (*p) |= ((v & 1) << (7 - LeftOffset));

    m_BitOffset++;
}

void AvcOutBits::PutBits(uint32_t v, uint32_t n)
{
    DDI_ASSERT((n > 0) && (n <= 32));

    uint32_t t = 0;

    if (!(m_BitOffset % 8) && !(n % 8))
    {
        uint32_t nBytes = n / 8;
        uint8_t *p = m_pOutBits + m_BitOffset / 8;

        while (nBytes-- > 0)
            (*p++) = (v >> nBytes) & 0xFF;

        m_BitOffset += n;
        return;
    }

    while (n-- > 0)
        PutBit((v >> n) & 1);
}

AvcInBits::AvcInBits(uint8_t *pInBits, uint32_t BitSize)
{
    m_pInBits = pInBits;
    m_BitSize = BitSize;
    m_BitOffset = 0;
}

void AvcInBits::SkipBits(uint32_t n)
{
    DDI_ASSERT(n > 0);
    DDI_ASSERT(m_BitOffset + n <= m_BitSize);

    m_BitOffset += n;
}

uint32_t AvcInBits::GetBit()
{
    DDI_ASSERT(m_BitOffset + 1 <= m_BitSize);

    uint32_t LeftOffset = m_BitOffset % 8;
    uint8_t const *p = m_pInBits + m_BitOffset / 8;
    uint32_t v = (*p >> (7 - LeftOffset)) & 1;

    m_BitOffset++;
    return v;
}

uint32_t AvcInBits::GetBits(uint32_t n)
{
    DDI_ASSERT((n > 0) && (n <= 32));

    uint32_t v = 0;

    if (!(m_BitOffset % 8) && !(n % 8))
    {
        uint32_t nBytes = n / 8;
        uint8_t const *p = m_pInBits + m_BitOffset / 8;

        while (nBytes-- > 0)
            v = (v << 8) | (*p++);

        m_BitOffset += n;
        return v;
    }

    while (n-- > 0)
        v = (v << 1) | GetBit();

    return v;
}

uint32_t AvcInBits::AvcInBits::GetUE()
{
    uint32_t nZero = 0;
    while(!GetBit())
        nZero++;

    return nZero ? ((1 << nZero) | GetBits(nZero)) - 1 : 0;
}

inline uint32_t AvcInBits::GetBitOffset()
{
    return m_BitOffset;
}

inline void AvcInBits::ResetBitOffset()
{
    m_BitOffset = 0;
}

MOS_STATUS DdiEncodeAvc::CheckPackedSlcHeaderData(
    void *pInSlcHdr,
    uint32_t InBitSize,
    void **ppOutSlcHdr,
    uint32_t &OutBitSize)
{
    MOS_STATUS status;
    uint32_t HdrBitSize = 0;

    *ppOutSlcHdr = NULL;
    OutBitSize = 0;

    if (VAEntrypointEncSliceLP != m_encodeCtx->vaEntrypoint)
        return MOS_STATUS_SUCCESS;

    if (0 == InBitSize || NULL == pInSlcHdr)
        return MOS_STATUS_SUCCESS;

    AvcInBits InBits((uint8_t*)pInSlcHdr, InBitSize);

    // Skip start code
    uint8_t StartCode = 0;
    while (1 != StartCode) {
        StartCode = InBits.GetBits(8);
        HdrBitSize += 8;
    }

    uint32_t StartBitSize = HdrBitSize;

    // Check NAL Unit type
    HdrBitSize += 8;
    InBits.SkipBits(1);
    InBits.SkipBits(2);
    uint32_t nalUnitType = InBits.GetBits(5);
    if (20 == nalUnitType)
    {
        // MVC enxtension
        InBits.SkipBits(24);
        HdrBitSize += 24;
    }

    // find first_mb_in_slice
    uint32_t first_mb_in_slice = InBits.GetUE();
    if (0 == first_mb_in_slice)
        return MOS_STATUS_SUCCESS;

    // Force first_mb_in_slice to 0 for AVC VDENC
    uint32_t LeftBitSize = InBitSize - InBits.GetBitOffset();
    OutBitSize = LeftBitSize + HdrBitSize + 1;
    *ppOutSlcHdr = MOS_AllocAndZeroMemory((OutBitSize + 7) / 8);

    AvcOutBits OutBits((uint8_t*)(*ppOutSlcHdr), OutBitSize);

    InBits.ResetBitOffset();
    OutBits.PutBits(InBits.GetBits(StartBitSize), StartBitSize);
    OutBits.PutBits(InBits.GetBits(8), 8);
    if (20 == nalUnitType)
        OutBits.PutBits(InBits.GetBits(24), 24);

    // Replace first_mb_in_slice
    first_mb_in_slice = InBits.GetUE();
    OutBits.PutBit(0);

    // Copy the left data
    while (LeftBitSize >= 32)
    {
        OutBits.PutBits(InBits.GetBits(32), 32);
        LeftBitSize -= 32;
    }

    if (LeftBitSize)
        OutBits.PutBits(InBits.GetBits(LeftBitSize), LeftBitSize);

    return MOS_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParsePackedHeaderData(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    BSBuffer *bsBuffer = m_encodeCtx->pbsBuffer;
    DDI_CHK_NULL(bsBuffer, "nullptr bsBuffer", VA_STATUS_ERROR_INVALID_PARAMETER);

    if ((m_encodeCtx->indexNALUnit == 0) && (m_encodeCtx->uiSliceHeaderCnt == 0))
    {
        *(bsBuffer->pBase)    = 0;
        bsBuffer->pCurrent    = bsBuffer->pBase;
        bsBuffer->SliceOffset = 0;
        bsBuffer->BitOffset   = 0;
        bsBuffer->BitSize     = 0;
    }

    uint32_t hdrDataSize;
    if (true == m_encodeCtx->bLastPackedHdrIsSlice)
    {
        void *temp_ptr = NULL;
        uint32_t temp_size = 0;

        MOS_STATUS status = CheckPackedSlcHeaderData(ptr,
            m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].BitSize,
            &temp_ptr, temp_size);
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:packed slice header is not supported!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (temp_size && temp_ptr)
        {
            m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].BitSize = temp_size;
        }

        hdrDataSize = (m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].BitSize + 7) / 8;

        status = MOS_SecureMemcpy(bsBuffer->pCurrent,
            bsBuffer->BufferSize - bsBuffer->SliceOffset,
            (uint8_t *)(temp_ptr ? temp_ptr : ptr),
            hdrDataSize);
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:packed slice header size is too large to be supported!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        if (temp_size && temp_ptr)
        {
            MOS_FreeMemory(temp_ptr);
            temp_size = 0;
            temp_ptr = NULL;
        }

        m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].SliceOffset = bsBuffer->pCurrent - bsBuffer->pBase;

        // correct SkipEmulationByteCount
        // according to LibVA principle, one packed header buffer should only contain one NALU,
        // so when has_emulation_bytes is 0, SkipEmulationByteCount only needs to skip the NALU start codes
        if (m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].SkipEmulationByteCount != hdrDataSize)
        {
            uint32_t startCodesOffset = 0;
            uint32_t startCodesLength = 0;
            VAStatus vaSts = VA_STATUS_SUCCESS;
            vaSts = FindNalUnitStartCodes((uint8_t *)ptr, hdrDataSize, &startCodesOffset, &startCodesLength);
            if (VA_STATUS_SUCCESS != vaSts)
            {
                DDI_ASSERTMESSAGE("DDI: packed slice header doesn't include NAL unit start codes!");
                return vaSts;
            }
            m_encodeCtx->pSliceHeaderData[m_encodeCtx->uiSliceHeaderCnt].SkipEmulationByteCount = MOS_MIN(15, (startCodesOffset + startCodesLength));
        }

        m_encodeCtx->uiSliceHeaderCnt++;
        m_encodeCtx->bLastPackedHdrIsSlice = false;
    }
    else
    {
        // copy sps and pps header data
        hdrDataSize       = m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSize;
        MOS_STATUS status = MOS_SecureMemcpy(bsBuffer->pCurrent,
            bsBuffer->BufferSize - bsBuffer->SliceOffset,
            (uint8_t *)ptr,
            hdrDataSize);
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:packed header size is too large to be supported!");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        // correct uiSkipEmulationCheckCount
        // according to LibVA principle, one packed header buffer should only contain one NALU,
        // so when has_emulation_bytes is 0, uiSkipEmulationCheckCount only needs to skip the NALU start codes
        if (m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSkipEmulationCheckCount != hdrDataSize)
        {
            uint32_t startCodesOffset = 0;
            uint32_t startCodesLength = 0;
            VAStatus vaSts = VA_STATUS_SUCCESS;
            vaSts = FindNalUnitStartCodes((uint8_t *)ptr, hdrDataSize, &startCodesOffset, &startCodesLength);
            if (VA_STATUS_SUCCESS != vaSts)
            {
                DDI_ASSERTMESSAGE("DDI: packed header doesn't include NAL unit start codes!");
                return vaSts;
            }
            m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiSkipEmulationCheckCount = MOS_MIN(15, (startCodesOffset + startCodesLength));
        }

        m_encodeCtx->ppNALUnitParams[m_encodeCtx->indexNALUnit]->uiOffset = bsBuffer->pCurrent - bsBuffer->pBase;
        m_encodeCtx->indexNALUnit++;
    }
    bsBuffer->pCurrent += hdrDataSize;
    bsBuffer->SliceOffset += hdrDataSize;
    bsBuffer->BitSize += hdrDataSize * 8;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeAvc::ParseMiscParams(void *ptr)
{
    DDI_CHK_NULL(m_encodeCtx, "nullptr m_encodeCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ptr, "nullptr ptr", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAEncMiscParameterBuffer *miscParamBuf = (VAEncMiscParameterBuffer *)ptr;
    DDI_CHK_NULL(miscParamBuf->data, "nullptr miscParamBuf->data", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus status = VA_STATUS_SUCCESS;
    switch ((int32_t)(miscParamBuf->type))
    {
    case VAEncMiscParameterTypeHRD:
        status = ParseMiscParamHRD((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeFrameRate:
        status = ParseMiscParamFR((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeRateControl:
        status = ParseMiscParamRC((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeEncQuality:
        status = ParseMiscParamEncQuality((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeQuantization:
        status = ParseMiscParamQuantization((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeRIR:
        status = ParseMiscParameterRIR((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeSkipFrame:
        status = ParseMiscParamSkipFrame((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeMaxFrameSize:
        status = ParseMiscParamMaxFrameSize((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeMultiPassFrameSize:
        status = ParseMiscParamMultiPassFrameSize((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeQualityLevel:
        status = ParseMiscParamQualityLevel((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeMaxSliceSize:
        status = ParseMiscParamMaxSliceSize((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeROI:
        status = ParseMiscParamROI((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeDirtyRect:
        status = ParseMiscParamDirtyROI((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeCustomRoundingControl:
        status = ParseMiscParamRounding((void *)miscParamBuf->data);
        break;

    case VAEncMiscParameterTypeSubMbPartPel:
        status = ParseMiscParamSubMbPartPel((void *)miscParamBuf->data);
        break;

    default:
        DDI_ASSERTMESSAGE("unsupported misc parameter type.");
        status = VA_STATUS_ERROR_INVALID_PARAMETER;
        break;
    }

    return status;
}

void DdiEncodeAvc::GetSlcRefIdx(CODEC_PICTURE *picReference, CODEC_PICTURE *slcReference)
{
    if (nullptr == picReference || nullptr == slcReference)
    {return;}

    int32_t i = 0;
    if (slcReference->FrameIdx != CODEC_AVC_NUM_UNCOMPRESSED_SURFACE)
    {
        for (i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
        {
            if (slcReference->FrameIdx == picReference[i].FrameIdx)
            {
                slcReference->FrameIdx = i;
                break;
            }
        }
        if (i == CODEC_MAX_NUM_REF_FRAME)
        {
            slcReference->FrameIdx = CODEC_AVC_NUM_UNCOMPRESSED_SURFACE;
        }
    }
}

void DdiEncodeAvc::SetupCodecPicture(
    DDI_MEDIA_CONTEXT                   *mediaCtx,
    DDI_CODEC_RENDER_TARGET_TABLE       *rtTbl,
    CODEC_PICTURE                       *codecHalPic,
    VAPictureH264                       vaPic,
    bool                                fieldPicFlag,
    bool                                picReference,
    bool                                sliceReference)
{
    if(vaPic.picture_id != DDI_CODEC_INVALID_FRAME_INDEX)
    {
        DDI_MEDIA_SURFACE *surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, vaPic.picture_id);
        vaPic.frame_idx    = GetRenderTargetID(rtTbl, surface);
        codecHalPic->FrameIdx = (uint8_t)vaPic.frame_idx;
    }
    else
    {
        vaPic.frame_idx    = DDI_CODEC_INVALID_FRAME_INDEX;
        codecHalPic->FrameIdx = CODEC_AVC_NUM_UNCOMPRESSED_SURFACE - 1;
    }

    if (picReference)
    {
        if (vaPic.frame_idx == DDI_CODEC_INVALID_FRAME_INDEX)
        {
            codecHalPic->PicFlags = PICTURE_INVALID;
        }
        else if ((vaPic.flags&VA_PICTURE_H264_LONG_TERM_REFERENCE) == VA_PICTURE_H264_LONG_TERM_REFERENCE)
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
            if ((vaPic.flags&VA_PICTURE_H264_BOTTOM_FIELD) == VA_PICTURE_H264_BOTTOM_FIELD)
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

    if (sliceReference && (vaPic.picture_id == VA_INVALID_ID))//VA_INVALID_ID is used to indicate invalide picture in LIBVA.
    {
        codecHalPic->PicFlags = PICTURE_INVALID;
    }
}

uint32_t DdiEncodeAvc::getSliceParameterBufferSize()
{
    return sizeof(VAEncSliceParameterBufferH264);
}

uint32_t DdiEncodeAvc::getSequenceParameterBufferSize()
{
    return sizeof(VAEncSequenceParameterBufferH264);
}

uint32_t DdiEncodeAvc::getPictureParameterBufferSize()
{
    return sizeof(VAEncPictureParameterBufferH264);
}

uint32_t DdiEncodeAvc::getQMatrixBufferSize()
{
    return sizeof(VAIQMatrixBufferH264);
}

void DdiEncodeAvc::ClearPicParams()
{
    uint8_t ppsIdx = ((PCODEC_AVC_ENCODE_SLICE_PARAMS)(m_encodeCtx->pSliceParams))->pic_parameter_set_id;
    PCODEC_AVC_ENCODE_PIC_PARAMS  picParams = (PCODEC_AVC_ENCODE_PIC_PARAMS)m_encodeCtx->pPicParams + ppsIdx;

    if (picParams != nullptr && picParams->pDeltaQp != nullptr)
    {
        MOS_FreeMemory(picParams->pDeltaQp);
        picParams->pDeltaQp = nullptr;
    }
}
