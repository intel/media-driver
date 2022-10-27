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
//! \file     encode_avc_vdenc_weighted_prediction.cpp
//! \brief    Implemetation for avc weighted prediction feature
//!

#include "encode_avc_vdenc_weighted_prediction.h"
#include "encode_avc_vdenc_feature_manager.h"

namespace encode
{

AvcVdencWeightedPred::AvcVdencWeightedPred(
    MediaFeatureManager *featureManager,
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *constSettings) :
    MediaFeature(constSettings)
{
    auto encFeatureManager = dynamic_cast<EncodeAvcVdencFeatureManager*>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<AvcBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
}

MHW_SETPAR_DECL_SRC(VDENC_WEIGHTSOFFSETS_STATE, AvcVdencWeightedPred)
{
    auto picParams   = m_basicFeature->m_picParam;
    auto sliceParams = &m_basicFeature->m_sliceParams[m_basicFeature->m_curNumSlices];

    params.weightsLuma[0][0] = 1;
    params.weightsLuma[0][1] = 1;
    params.weightsLuma[0][2] = 1;
    params.weightsLuma[1][0] = 1;

    if ((Slice_Type[sliceParams->slice_type] == SLICE_P) &&
        (picParams->weighted_pred_flag == EXPLICIT_WEIGHTED_INTER_PRED_MODE) ||
        (Slice_Type[sliceParams->slice_type] == SLICE_B) &&
        (picParams->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE))
    {
        if (picParams->weighted_pred_flag)
        {
            params.weightsLuma[0][0] = (int8_t)sliceParams->Weights[0][0][0][0];
            params.offsetsLuma[0][0] = sliceParams->Weights[0][0][0][1];
            params.weightsLuma[0][1] = (int8_t)sliceParams->Weights[0][1][0][0];
            params.offsetsLuma[0][1] = sliceParams->Weights[0][1][0][1];
            params.weightsLuma[0][2] = (int8_t)sliceParams->Weights[0][2][0][0];
            params.offsetsLuma[0][2] = sliceParams->Weights[0][2][0][1];
        }

        if (picParams->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE)
        {
            params.weightsLuma[1][0] = (int8_t)sliceParams->Weights[1][0][0][0];
            params.offsetsLuma[1][0] = sliceParams->Weights[1][0][0][1];
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_IMG_STATE, AvcVdencWeightedPred)
{
    auto picParams = m_basicFeature->m_picParam;

    if (picParams->CodingType == B_TYPE)
    {
        if (picParams->weighted_bipred_idc == 2)
        {
            params.bidirectionalWeight = (uint8_t)m_basicFeature->m_ref->GetBiWeight();
        }
        else
        {
            params.bidirectionalWeight = 0x20;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_WEIGHTOFFSET_STATE, AvcVdencWeightedPred)
{
    auto sliceParams = &m_basicFeature->m_sliceParams[m_basicFeature->m_curNumSlices];

    int32_t uiNumRefForList[2] = {sliceParams->num_ref_idx_l0_active_minus1 + 1, sliceParams->num_ref_idx_l1_active_minus1 + 1};

    for (auto i = 0; i < uiNumRefForList[params.uiList]; i++)
    {
        if (sliceParams->luma_weight_flag[params.uiList] & (1 << i))
        {
            params.weightoffset[3 * i] = sliceParams->Weights[params.uiList][i][0][0] & 0xFFFF; // Y weight
            params.weightoffset[3 * i] |= (sliceParams->Weights[params.uiList][i][0][1] & 0xFFFF) << 16; // Y offset
        }
        else
        {
            params.weightoffset[3 * i] = 1 << (sliceParams->luma_log2_weight_denom); // Y weight
        }

        if (sliceParams->chroma_weight_flag[params.uiList] & (1 << i))
        {
            params.weightoffset[3 * i + 1] = sliceParams->Weights[params.uiList][i][1][0] & 0xFFFF; // Cb weight
            params.weightoffset[3 * i + 1] |= (sliceParams->Weights[params.uiList][i][1][1] & 0xFFFF) << 16; // Cb offset
            params.weightoffset[3 * i + 2] = sliceParams->Weights[params.uiList][i][2][0] & 0xFFFF; // Cr weight
            params.weightoffset[3 * i + 2] |= (sliceParams->Weights[params.uiList][i][2][1] & 0xFFFF) << 16; // Cr offset
        }
        else
        {
            params.weightoffset[3 * i + 1] = 1 << (sliceParams->chroma_log2_weight_denom); // Cb  weight
            params.weightoffset[3 * i + 2] = 1 << (sliceParams->chroma_log2_weight_denom); // Cr  weight
        }
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
