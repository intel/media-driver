/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     encode_av1_vdenc_preenc.cpp
//! \brief    Defines the common interface for encode av1 preenc parameter
//!

#include "encode_utils.h"
#include "encode_av1_vdenc_preenc.h"
#include "encode_av1_reference_frames.h"

namespace encode
{
Av1VdencPreEnc::Av1VdencPreEnc(
    MediaFeatureManager *featureManager,
    EncodeAllocator *    allocator,
    CodechalHwInterfaceNext *hwInterface,
    TrackedBuffer *      trackedBuf,
    RecycleResource *    recycleBuf,
    void *               constSettings) : PreEncBasicFeature(featureManager, allocator, hwInterface, trackedBuf, recycleBuf, constSettings)
{
    ENCODE_FUNC_CALL();

    m_basicFeature = dynamic_cast<Av1BasicFeature *>(featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
}

MOS_STATUS Av1VdencPreEnc::PreparePreEncConfig(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    const EncoderParams *encodeParams = (EncoderParams *)params;

    const auto av1SeqPar = static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(av1SeqPar);

    const auto av1PicPar = static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(av1PicPar);

    const CODEC_REF_LIST_AV1 *currRefList = m_basicFeature->m_ref.GetCurrRefList();

    m_preEncConfig.LowDelayMode = (av1SeqPar->GopRefDist == 1);
    m_preEncConfig.BitDepthLumaMinus8 = m_preEncConfig.BitDepthChromaMinus8
        = m_basicFeature->m_is10Bit ? 2 : 0;
    //(m_pictureCodingType == I_TYPE) ? I_TYPE : IsLowDelay ? (IsPFrame ? P_TYPE : LDB_TYPE) : RAB_TYPE);
    m_preEncConfig.CodingType = (m_basicFeature->m_pictureCodingType == I_TYPE) ? I_TYPE : (m_basicFeature->m_ref.IsLowDelay() ? (m_basicFeature->m_ref.IsPFrame() ? P_TYPE : B_TYPE) : B_TYPE);
    m_preEncConfig.CurrReconstructedPic = av1PicPar->CurrReconstructedPic;
    m_preEncConfig.HierarchicalFlag = av1SeqPar->SeqFlags.fields.HierarchicalFlag;
    m_preEncConfig.HierarchLevelPlus1 = av1PicPar->HierarchLevelPlus1;
    m_preEncConfig.GopRefDist = av1SeqPar->GopRefDist;
    m_preEncConfig.CurrOriginalPic = av1PicPar->CurrOriginalPic;
    m_preEncConfig.UsedAsRef = currRefList->bUsedAsRef;
    m_preEncConfig.InputColorSpace = av1SeqPar->InputColorSpace;
    m_preEncConfig.SliceType = (uint8_t)HevcSliceType[m_preEncConfig.CodingType - 1];
    m_mode = CODECHAL_MODE::CODECHAL_ENCODE_MODE_HEVC;
    
    m_orderHintOffset[av1PicPar->order_hint % ENCODE_AV1_ORDER_HINT_SIZE] += 1;
    m_preEncConfig.CurrPicOrderCnt = av1PicPar->order_hint + ENCODE_AV1_ORDER_HINT_SIZE * (m_orderHintOffset[av1PicPar->order_hint % ENCODE_AV1_ORDER_HINT_SIZE] - 1);

    for (uint8_t i = 0; i < CODEC_AV1_NUM_REF_FRAMES; i++)
    {
        m_preEncConfig.RefFrameList[i] = av1PicPar->RefFrameList[i];
        m_preEncConfig.PicIdx[i] = m_basicFeature->m_ref.GetPicId(i);
    }

    if (m_preEncConfig.CodingType != I_TYPE)
    {
        m_basicFeature->m_ref.GetFwdBwdRefPicList(m_preEncConfig.RefPicList);
        m_basicFeature->m_ref.GetRefFramePOC(m_preEncConfig.RefFramePOCList, m_preEncConfig.CurrPicOrderCnt);
    }

    m_preEncConfig.RefList = (PCODEC_REF_LIST*)(m_basicFeature->m_ref.GetRefList());

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
