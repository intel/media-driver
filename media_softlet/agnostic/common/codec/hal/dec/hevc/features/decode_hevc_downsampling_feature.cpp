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
//! \file     decode_hevc_downsampling_feature.cpp
//! \brief    Defines the interface for Hevc decode downsampling feature
//! \details  The Hevc decode downsampling feature interface is maintaining the down sampling context.
//!
#include "decode_hevc_downsampling_feature.h"
#include "decode_hevc_basic_feature.h"
#include "decode_utils.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
HevcDownSamplingFeature::HevcDownSamplingFeature(
    MediaFeatureManager *featureManager, DecodeAllocator *allocator, PMOS_INTERFACE osInterface) :
    DecodeDownSamplingFeature(featureManager, allocator, osInterface)
{
}

HevcDownSamplingFeature::~HevcDownSamplingFeature()
{
}

MOS_STATUS HevcDownSamplingFeature::GetRefFrameList(std::vector<uint32_t> &refFrameList)
{
    HevcBasicFeature *hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(hevcBasicFeature);

    uint8_t curFrameIdx = hevcBasicFeature->m_hevcPicParams->CurrPic.FrameIdx;
    DECODE_CHK_COND(curFrameIdx >= hevcBasicFeature->m_maxFrameIndex, "Frame Index of current frame out of range!");
    PCODEC_REF_LIST destEntry = hevcBasicFeature->m_refFrames.m_refList[curFrameIdx];
    DECODE_CHK_NULL(destEntry);

    refFrameList.clear();
    for(auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        uint8_t refFrameIdx = destEntry->RefList[i].FrameIdx;
        if (refFrameIdx < hevcBasicFeature->m_maxFrameIndex)
        {
            refFrameList.push_back(refFrameIdx);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDownSamplingFeature::GetDecodeTargetSize(SurfaceWidthT &width, SurfaceHeightT &height)
{
    width  = m_basicFeature->m_width;
    height = m_basicFeature->m_height;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDownSamplingFeature::GetDecodeTargetFormat(MOS_FORMAT &format)
{
    HevcBasicFeature *hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(hevcBasicFeature);
    PCODEC_HEVC_PIC_PARAMS hevcPicParams = hevcBasicFeature->m_hevcPicParams;
    DECODE_CHK_NULL(hevcPicParams);

    if (hevcPicParams->chroma_format_idc == HCP_CHROMA_FORMAT_YUV444)
    {
        if (hevcPicParams->bit_depth_luma_minus8   > 2 ||
            hevcPicParams->bit_depth_chroma_minus8 > 2)
        {
            format = Format_Y416;
        }
        else if (hevcPicParams->bit_depth_luma_minus8   > 0 ||
                 hevcPicParams->bit_depth_chroma_minus8 > 0)
        {
            format = Format_Y410;
        }
        else
        {
            format = Format_AYUV;
        }
    }
    else if (hevcPicParams->chroma_format_idc == HCP_CHROMA_FORMAT_YUV422)
    {
        if (hevcPicParams->bit_depth_luma_minus8   > 2 ||
            hevcPicParams->bit_depth_chroma_minus8 > 2)
        {
            format = Format_Y216;
        }
        else if (hevcPicParams->bit_depth_luma_minus8   > 0 ||
                 hevcPicParams->bit_depth_chroma_minus8 > 0)
        {
            format = Format_Y210;
        }
        else
        {
            format = Format_YUY2;
        }
    }
    else
    {
        if (hevcPicParams->bit_depth_luma_minus8   > 2 ||
            hevcPicParams->bit_depth_chroma_minus8 > 2)
        {
            format = Format_P016;
        }
        else if (hevcPicParams->bit_depth_luma_minus8   > 0 ||
                 hevcPicParams->bit_depth_chroma_minus8 > 0)
        {
            format = Format_P010;
        }
        else
        {
            format = Format_NV12;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDownSamplingFeature::UpdateDecodeTarget(MOS_SURFACE &surface)
{
    HevcBasicFeature *hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(hevcBasicFeature);

    DECODE_CHK_STATUS(hevcBasicFeature->UpdateDestSurface(surface));
    if (hevcBasicFeature->m_isSCCIBCMode)
    {
        DECODE_CHK_STATUS(hevcBasicFeature->CreateReferenceBeforeLoopFilter());
    }

    DECODE_CHK_NULL(hevcBasicFeature->m_hevcPicParams);
    HevcReferenceFrames &refFrames = hevcBasicFeature->m_refFrames;
    DECODE_CHK_STATUS(refFrames.UpdateCurResource(
        *hevcBasicFeature->m_hevcPicParams, hevcBasicFeature->m_isSCCIBCMode));

    return MOS_STATUS_SUCCESS;
}

}

#endif
