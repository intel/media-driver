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
//! \file     encode_hevc_vdenc_preenc.cpp
//! \brief    Defines the common interface for encode hevc preenc parameter
//!

#include "encode_hevc_basic_feature.h"
#include "encode_hevc_vdenc_preenc.h"
#include "encode_allocator.h"

namespace encode
{
HevcVdencPreEnc::HevcVdencPreEnc(
    MediaFeatureManager *featureManager,
    EncodeAllocator *    allocator,
    CodechalHwInterfaceNext *hwInterface,
    TrackedBuffer *      trackedBuf,
    RecycleResource *    recycleBuf,
    void *               constSettings) : PreEncBasicFeature(featureManager, allocator, hwInterface, trackedBuf, recycleBuf, constSettings)
{
    ENCODE_FUNC_CALL();
    auto encFeatureManager = (featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_allocator = allocator;

    m_basicFeature = dynamic_cast<HevcBasicFeature *>(encFeatureManager->GetFeature(HevcFeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
}

HevcVdencPreEnc::~HevcVdencPreEnc()
{
}

MOS_STATUS HevcVdencPreEnc::PreparePreEncConfig(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    EncoderParams *encodeParams = (EncoderParams *)params;

    m_hevcSeqParams = static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(m_hevcSeqParams);
    m_hevcPicParams = static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(m_hevcPicParams);
    m_hevcSliceParams = static_cast<PCODEC_HEVC_ENCODE_SLICE_PARAMS>(encodeParams->pSliceParams);
    ENCODE_CHK_NULL_RETURN(m_hevcSliceParams);
    m_nalUnitParams = encodeParams->ppNALUnitParams;
    ENCODE_CHK_NULL_RETURN(m_nalUnitParams);
    m_NumNalUnits = encodeParams->uiNumNalUnits;

    m_preEncConfig.LowDelayMode         = m_hevcSeqParams->LowDelayMode;
    m_preEncConfig.BitDepthLumaMinus8   = m_hevcSeqParams->bit_depth_luma_minus8;
    m_preEncConfig.BitDepthChromaMinus8 = m_hevcSeqParams->bit_depth_chroma_minus8;
    m_preEncConfig.CodingType           = (uint8_t)m_basicFeature->m_pictureCodingType;
    m_preEncConfig.CurrReconstructedPic = m_hevcPicParams->CurrReconstructedPic;
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_preEncConfig.RefFrameList[i]    = m_hevcPicParams->RefFrameList[i];
        m_preEncConfig.RefFramePOCList[i] = m_hevcPicParams->RefFramePOCList[i];
    }
    for (auto i = 0; i < 2; i++)
    {
        for (auto ii = 0; ii < CODEC_MAX_NUM_REF_FRAME_HEVC; ii++)
        {
            m_preEncConfig.RefPicList[i][ii] = m_hevcSliceParams->RefPicList[i][ii];
        }
    }
    m_preEncConfig.CurrPicOrderCnt         = m_hevcPicParams->CurrPicOrderCnt;
    m_preEncConfig.HierarchicalFlag        = m_hevcSeqParams->HierarchicalFlag;
    m_preEncConfig.HierarchLevelPlus1      = m_hevcPicParams->HierarchLevelPlus1;
    m_preEncConfig.GopRefDist              = m_hevcSeqParams->GopRefDist;
    m_preEncConfig.SliceType               = m_hevcSliceParams->slice_type;
    m_preEncConfig.CurrOriginalPic         = m_hevcPicParams->CurrOriginalPic;
    m_preEncConfig.UsedAsRef               = m_hevcPicParams->bUsedAsRef;
    m_preEncConfig.InputColorSpace         = m_hevcSeqParams->InputColorSpace;

    for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_preEncConfig.PicIdx[i] = m_basicFeature->m_ref.GetPicIndex(i);
    }

    m_preEncConfig.RefList = m_basicFeature->m_ref.GetRefList();

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
