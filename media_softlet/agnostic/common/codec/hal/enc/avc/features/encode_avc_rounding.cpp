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
//! \file     encode_avc_rounding.cpp
//! \brief    Defines the common interface for avc encode rounding feature
//!

#include "encode_avc_rounding.h"
#include "encode_avc_brc.h"
#include "encode_avc_vdenc_feature_manager.h"
#include "encode_avc_vdenc_const_settings.h"
#include "media_avc_feature_defs.h"

namespace encode
{

AvcEncodeRounding::AvcEncodeRounding(
    MediaFeatureManager *featureManager,
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *constSettings) :
    MediaFeature(constSettings)
{
    m_featureManager = featureManager;
    auto encFeatureManager = dynamic_cast<EncodeAvcVdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<AvcBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
}

MOS_STATUS AvcEncodeRounding::Update(void *params)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(params);
    EncoderParams* encodeParams = (EncoderParams*)params;
    m_roundingParams = (PCODECHAL_ENCODE_AVC_ROUNDING_PARAMS)encodeParams->pAVCRoundingParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeRounding::GetRounding(MHW_VDBOX_AVC_SLICE_STATE &sliceState) const
{
    ENCODE_FUNC_CALL();

    auto    seqParams   = m_basicFeature->m_seqParam;
    auto    picParams   = m_basicFeature->m_picParam;
    auto    sliceParams = &m_basicFeature->m_sliceParams[m_basicFeature->m_curNumSlices];
    uint8_t sliceQP     = picParams->pic_init_qp_minus26 + 26 + sliceParams->slice_qp_delta;

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    auto brcFeature = dynamic_cast<AvcEncodeBRC*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    sliceState.dwRoundingIntraValue = settings->DefaultIntraRounding;

    if (Slice_Type[sliceParams->slice_type] != SLICE_I)
    {
        bool isRef   = m_basicFeature->m_ref->GetRefList()[m_basicFeature->m_currReconstructedPic.FrameIdx]->bUsedAsRef;
        bool isB     = (Slice_Type[sliceParams->slice_type] == SLICE_B);
        bool isIPGOP = (seqParams->GopRefDist == 1);

        auto index   = (isB) ? (isRef ? 1 : 0) : (isIPGOP ? 3 : 2);

        if (brcFeature->IsVdencBrcEnabled() || !m_basicFeature->m_adaptiveRoundingInterEnable)
        {
            sliceState.dwRoundingValue      = settings->StaticInterRounding[index];
            sliceState.dwRoundingIntraValue = settings->StaticIntraRounding[index];
        }
        else
        {
            sliceState.dwRoundingValue      = settings->AdaptiveInterRounding[index][sliceQP];
            sliceState.dwRoundingIntraValue = settings->AdaptiveIntraRounding[index][sliceQP];
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcEncodeRounding::SetRoundingParams(MHW_VDBOX_AVC_SLICE_STATE &sliceState) const
{
    ENCODE_FUNC_CALL();

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    sliceState.bRoundingInterEnable = true;

    if(m_roundingParams != nullptr && m_roundingParams->bEnableCustomRoudingIntra)
        sliceState.dwRoundingIntraValue = m_roundingParams->dwRoundingIntra;
    else
        sliceState.dwRoundingIntraValue = settings->DefaultIntraRounding;

    if (m_roundingParams != nullptr && m_roundingParams->bEnableCustomRoudingInter)
        sliceState.dwRoundingValue = m_roundingParams->dwRoundingInter;
    else
        ENCODE_CHK_STATUS_RETURN(GetRounding(sliceState));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_SLICE_STATE, AvcEncodeRounding)
{
    auto sliceParams = &m_basicFeature->m_sliceParams[m_basicFeature->m_curNumSlices];

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    ENCODE_CHK_STATUS_RETURN(SetRoundingParams(sliceState));

    params.roundIntra = (uint8_t)sliceState.dwRoundingIntraValue;
    params.roundInter = 2;

    if (m_basicFeature->IsAvcPSlice(sliceParams->slice_type) ||
        m_basicFeature->IsAvcBSlice(sliceParams->slice_type))
    {
        params.roundInterEnable = sliceState.bRoundingInterEnable;
        params.roundInter       = (uint8_t)sliceState.dwRoundingValue;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_SLICE_STATE, AvcEncodeRounding)
{
    auto sliceParams = &m_basicFeature->m_sliceParams[m_basicFeature->m_curNumSlices];

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    ENCODE_CHK_STATUS_RETURN(SetRoundingParams(sliceState));

    params.roundintra = (uint8_t)sliceState.dwRoundingIntraValue;
    params.roundinter = 2;

    if (m_basicFeature->IsAvcPSlice(sliceParams->slice_type) ||
        m_basicFeature->IsAvcBSlice(sliceParams->slice_type))
    {
        params.roundinterenable = sliceState.bRoundingInterEnable;
        params.roundinter = (uint8_t)sliceState.dwRoundingValue;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
