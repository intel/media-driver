/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_avc_trellis.cpp
//! \brief    Defines the common interface for avc encode trellis feature
//!

#include "encode_avc_trellis.h"
#include "encode_avc_vdenc_feature_manager.h"
#include "encode_avc_vdenc_const_settings.h"

namespace encode
{

#define ENCODE_AVC_DEFAULT_TRELLIS_QUANT_ROUNDING 3

AvcEncodeTrellis::AvcEncodeTrellis(
    MediaFeatureManager *featureManager,
    EncodeAllocator     *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void                *constSettings) :
    MediaFeature(constSettings)
{
    m_featureManager = featureManager;
    auto encFeatureManager = dynamic_cast<EncodeAvcVdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<AvcBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
}

MOS_STATUS AvcEncodeTrellis::Update(void *params)
{
    ENCODE_FUNC_CALL();

    auto seqParams = m_basicFeature->m_seqParam;

    m_trellis = seqParams->Trellis;

    ENCODE_CHK_STATUS_RETURN(UpdateTrellisParameters());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeTrellis::GetTrellisQuantization(
    PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params,
    PCODECHAL_ENCODE_AVC_TQ_PARAMS trellisQuantParams)
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(trellisQuantParams);

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    trellisQuantParams->dwTqEnabled  = settings->TrellisQuantizationEnable[params->ucTargetUsage];
    trellisQuantParams->dwTqRounding = trellisQuantParams->dwTqEnabled ?
                                       settings->TrellisQuantizationRounding[params->ucTargetUsage] : 0;

    return eStatus;
}

MOS_STATUS AvcEncodeTrellis::UpdateTrellisParameters()
{
    ENCODE_FUNC_CALL();

    uint8_t sliceQP = m_basicFeature->m_picParam->pic_init_qp_minus26 + 26 + m_basicFeature->m_sliceParams->slice_qp_delta;

    // Determine if Trellis Quantization should be enabled
    MOS_ZeroMemory(&m_trellisQuantParams, sizeof(m_trellisQuantParams));

    // Trellis must remain switched off if it is explicitly disabled or picture is encoded with CAVLC
    if (!(m_trellis & trellisDisabled) && m_basicFeature->m_picParam->entropy_coding_mode_flag)
    {
        if (m_trellis == trellisInternal)
        {
            CODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS tqInputParams;
            tqInputParams.ucQP               = sliceQP;
            tqInputParams.ucTargetUsage      = m_basicFeature->m_seqParam->TargetUsage;
            tqInputParams.wPictureCodingType = m_basicFeature->m_pictureCodingType;
            tqInputParams.bBrcEnabled        = false;
            tqInputParams.bVdEncEnabled      = true;

            ENCODE_CHK_STATUS_RETURN(GetTrellisQuantization(
                &tqInputParams,
                &m_trellisQuantParams));
        }
        else if ((m_basicFeature->m_pictureCodingType == I_TYPE && (m_trellis & trellisEnabledI)) ||
                 (m_basicFeature->m_pictureCodingType == P_TYPE && (m_trellis & trellisEnabledP)) ||
                 (m_basicFeature->m_pictureCodingType == B_TYPE && (m_trellis & trellisEnabledB)))
        {
            m_trellisQuantParams.dwTqEnabled  = true;
            m_trellisQuantParams.dwTqRounding = ENCODE_AVC_DEFAULT_TRELLIS_QUANT_ROUNDING;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_IMG_STATE, AvcEncodeTrellis)
{
    params.trellisQuantEn = (uint8_t)m_trellisQuantParams.dwTqEnabled;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_IMG_STATE, AvcEncodeTrellis)
{
    if (m_trellisQuantParams.dwTqEnabled && m_basicFeature->m_picParam->entropy_coding_mode_flag)
    {
        params.trellisQuantizationEnabledTqenb = m_trellisQuantParams.dwTqEnabled;
        params.trellisQuantizationRoundingTqr = m_trellisQuantParams.dwTqRounding;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
