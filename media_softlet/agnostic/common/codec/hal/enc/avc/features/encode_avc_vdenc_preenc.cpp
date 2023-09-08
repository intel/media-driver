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
//! \file     encode_avc_vdenc_preenc.cpp
//! \brief    Defines the common interface for encode avc preenc parameter
//!

#include "encode_avc_basic_feature.h"
#include "encode_avc_vdenc_preenc.h"
#include "encode_allocator.h"

namespace encode
{
AvcVdencPreEnc::AvcVdencPreEnc(
    MediaFeatureManager *featureManager,
    EncodeAllocator     *allocator,
    CodechalHwInterfaceNext *hwInterface,
    TrackedBuffer       *trackedBuf,
    RecycleResource     *recycleBuf,
    void                *constSettings) : PreEncBasicFeature(featureManager, allocator, hwInterface, trackedBuf, recycleBuf, constSettings)
{
    ENCODE_FUNC_CALL();
    auto encFeatureManager = (featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);
    m_allocator    = allocator;
    m_basicFeature = dynamic_cast<AvcBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
}

AvcVdencPreEnc::~AvcVdencPreEnc()
{
}

// This function may be used by fullenc feature.
// Please don't use class member variables in the function.
MOS_STATUS AvcVdencPreEnc::CalculatePreEncInfo(uint32_t width, uint32_t height, PreEncInfo &preEncInfo)
{
    ENCODE_FUNC_CALL();

    preEncInfo.EncodePreEncInfo2 = 2;
    if (width >= 1920 && height >= 1080)
        preEncInfo.EncodePreEncInfo2 = 2;  //8x8
    else
        preEncInfo.EncodePreEncInfo2 = 1;  //8x8

    preEncInfo.EncodePreEncInfo3 = 0;
    if (width >= 3840 && height >= 2160)
        preEncInfo.EncodePreEncInfo3 = 2;  //4x
    else if (width >= 1920 && height >= 1080)
        preEncInfo.EncodePreEncInfo3 = 1;  //2x

    preEncInfo.EncodePreEncInfo0 = 0;
    if (width >= 3840 && height >= 2160)
        preEncInfo.EncodePreEncInfo0 = 2;  //4x
    else if (width >= 1920 && height >= 1080)
        preEncInfo.EncodePreEncInfo0 = 1;  //2x
    else
        preEncInfo.EncodePreEncInfo0 = 0;  //1x

    //calculate the mv replication factor
    uint8_t vdencMvRepFactor = MAX(0, (5 - preEncInfo.EncodePreEncInfo2) + (preEncInfo.EncodePreEncInfo0 == 3 ? -1 : preEncInfo.EncodePreEncInfo0) - 4);

    uint32_t offset          = (1 << preEncInfo.EncodePreEncInfo3) - 1;
    uint32_t preEncSrcWidth  = (width + offset) >> preEncInfo.EncodePreEncInfo3;
    uint32_t preEncSrcHeight = (height + offset) >> preEncInfo.EncodePreEncInfo3;

    preEncSrcWidth  = ((preEncSrcWidth + 7) >> 3) << 3;
    preEncSrcHeight = ((preEncSrcHeight + 7) >> 3) << 3;

    preEncInfo.preEncSrcWidth  = preEncSrcWidth;
    preEncInfo.preEncSrcHeight = preEncSrcHeight;

    //pitch be 64 aligned and height be 32 aligned
    uint16_t vdencPreencInfoStride = (uint16_t)((((preEncSrcWidth + 63) >> 6) << 6) >> (5 - preEncInfo.EncodePreEncInfo2));
    uint16_t vdencPreencInfoHeight = (uint16_t)((((preEncSrcHeight + 63) >> 6) << 6) >> (5 - preEncInfo.EncodePreEncInfo2));

    vdencPreencInfoStride = ((vdencPreencInfoStride + 7) >> 3) << 3;

    vdencPreencInfoStride <<= vdencMvRepFactor;
    vdencPreencInfoHeight <<= vdencMvRepFactor;

    preEncInfo.EncodePreEncInfo1 = vdencPreencInfoStride * vdencPreencInfoHeight;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencPreEnc::PreparePreEncConfig(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    EncoderParams *encodeParams = (EncoderParams *)params;

    m_avcSeqParams = static_cast<PCODEC_AVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(m_avcSeqParams);
    m_avcPicParams = static_cast<PCODEC_AVC_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(m_avcPicParams);
    m_avcSliceParams = static_cast<PCODEC_AVC_ENCODE_SLICE_PARAMS>(encodeParams->pSliceParams);
    ENCODE_CHK_NULL_RETURN(m_avcSliceParams);
    m_nalUnitParams = encodeParams->ppNALUnitParams;
    ENCODE_CHK_NULL_RETURN(m_nalUnitParams);
    m_NumNalUnits = encodeParams->uiNumNalUnits;

    m_preEncConfig.LowDelayMode         = m_avcSeqParams->LowDelayMode;
    m_preEncConfig.BitDepthLumaMinus8   = m_avcSeqParams->bit_depth_luma_minus8;
    m_preEncConfig.BitDepthChromaMinus8 = m_avcSeqParams->bit_depth_chroma_minus8;
    m_preEncConfig.CodingType           = m_avcPicParams->CodingType;
    if (m_preEncConfig.CodingType == P_TYPE)
    {
        m_preEncConfig.CodingType = B_TYPE;
        m_preEncConfig.isPToB     = true;
    }
    else
    {
        m_preEncConfig.isPToB = false;
    }
    m_preEncConfig.CurrReconstructedPic = m_avcPicParams->CurrReconstructedPic;
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_preEncConfig.RefFrameList[i]    = m_avcPicParams->RefFrameList[i];
        m_preEncConfig.RefFramePOCList[i] = m_avcPicParams->FieldOrderCntList[i][0] / 2;
    }
    for (auto i = 0; i < 2; i++)
    {
        for (auto ii = 0; ii < CODEC_MAX_NUM_REF_FRAME_HEVC; ii++)
        {
            m_preEncConfig.RefPicList[i][ii] = m_avcSliceParams->RefPicList[i][ii];
        }
    }
    m_preEncConfig.CurrPicOrderCnt  = m_avcPicParams->CurrFieldOrderCnt[0] / 2;
    m_preEncConfig.HierarchicalFlag = m_avcSeqParams->HierarchicalFlag;
    m_preEncConfig.GopRefDist       = (uint8_t)m_avcSeqParams->GopRefDist;

    uint8_t  depth = 0;
    uint32_t poc   = 0;
    if (m_preEncConfig.GopRefDist == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    poc = m_preEncConfig.CurrPicOrderCnt % m_preEncConfig.GopRefDist;
    

    if (poc == 0)
    {
        depth = 0;
    }
    else
    {
        uint32_t step = m_preEncConfig.GopRefDist;

        depth = 0;

        for (uint32_t ii = step >> 1; ii >= 1; ii >>= 1)
        {
            for (uint32_t jj = ii; jj < (uint32_t)m_preEncConfig.GopRefDist; jj += step)
            {
                if (jj == poc)
                {
                    ii = 0;
                    break;
                }
            }

            step >>= 1;
            depth++;
        }
    }
    m_preEncConfig.HierarchLevelPlus1 = depth + 1;

    ENCODE_CHK_COND_RETURN(m_avcSliceParams->slice_type >= 10, "slice_type cannot bigger than 10.");
    m_preEncConfig.SliceType       = (uint8_t)HevcSliceType[m_avcSliceParams->slice_type];
    m_preEncConfig.CurrOriginalPic = m_avcPicParams->CurrOriginalPic;
    m_preEncConfig.UsedAsRef       = m_avcPicParams->RefPicFlag;
    m_preEncConfig.InputColorSpace = m_avcSeqParams->InputColorSpace;

    PCODEC_PIC_ID pAvcPicIdx = m_basicFeature->m_ref->GetPicIndex();

    for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_preEncConfig.PicIdx[i] = pAvcPicIdx[i];
    }

    m_preEncConfig.RefList = m_basicFeature->m_ref->GetRefList();

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
