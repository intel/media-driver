/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_vp9_downsampling_feature.cpp
//! \brief    Defines the interface for Vp9 decode downsampling feature
//! \details  The Vp9 decode downsampling feature interface is maintaining the down sampling context.
//!
#include "decode_vp9_downsampling_feature.h"
#include "decode_vp9_basic_feature.h"
#include "decode_utils.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
Vp9DownSamplingFeature::Vp9DownSamplingFeature(
    MediaFeatureManager *featureManager, DecodeAllocator *allocator, PMOS_INTERFACE osInterface) : 
    DecodeDownSamplingFeature(featureManager, allocator, osInterface)
{
}

Vp9DownSamplingFeature::~Vp9DownSamplingFeature()
{
}

MOS_STATUS Vp9DownSamplingFeature::GetRefFrameList(std::vector<uint32_t> &refFrameList)
{
    Vp9BasicFeature *vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(vp9BasicFeature);

    uint8_t curFrameIdx = vp9BasicFeature->m_vp9PicParams->CurrPic.FrameIdx;
    DECODE_CHK_COND(curFrameIdx >= vp9BasicFeature->m_maxFrameIndex, "Frame Index of current frame out of range!");
    PCODEC_REF_LIST destEntry = vp9BasicFeature->m_refFrames.m_vp9RefList[curFrameIdx];
    DECODE_CHK_NULL(destEntry);

    refFrameList.clear();
    for (auto i = 0; i < CODECHAL_DECODE_VP9_MAX_NUM_REF_FRAME; i++)
    {
        uint8_t refFrameIdx = destEntry->RefList[i].FrameIdx;
        if (refFrameIdx < vp9BasicFeature->m_maxFrameIndex)
        {
            refFrameList.push_back(refFrameIdx);
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DownSamplingFeature::GetDecodeTargetFormat(MOS_FORMAT &format)
{
    Vp9BasicFeature *vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(vp9BasicFeature);
    PCODEC_VP9_PIC_PARAMS vp9PicParams = vp9BasicFeature->m_vp9PicParams;
    DECODE_CHK_NULL(vp9PicParams);
    if (vp9PicParams->subsampling_x == 1 && vp9PicParams->subsampling_y == 1) //HCP_CHROMA_FORMAT_YUV420
    {
        if (vp9PicParams->BitDepthMinus8 > 2)
        {
            format = Format_P016;
        }
        else if (vp9PicParams->BitDepthMinus8 > 0)
        {
            format = Format_P010;
        }
        else
        {
            format = Format_NV12;
        }
    }
    else if (vp9PicParams->subsampling_x == 0 && vp9PicParams->subsampling_y == 0)  //HCP_CHROMA_FORMAT_YUV444
    {
        if (vp9PicParams->BitDepthMinus8 > 2)
        {
            format = Format_Y416;
        }
        else if (vp9PicParams->BitDepthMinus8 > 0)
        {
            format = Format_Y410;
        }
        else
        {
            format = Format_AYUV;
        }
    }
    else
    {
        DECODE_ASSERTMESSAGE("Invalid Chroma sampling format!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DownSamplingFeature::UpdateDecodeTarget(MOS_SURFACE &surface)
{
    Vp9BasicFeature *vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(vp9BasicFeature);

    DECODE_CHK_STATUS(vp9BasicFeature->UpdateDestSurface(surface));
    DECODE_CHK_NULL(vp9BasicFeature->m_vp9PicParams);
    Vp9ReferenceFrames &refFrames = vp9BasicFeature->m_refFrames;
    DECODE_CHK_STATUS(refFrames.UpdateCurResource(
        *vp9BasicFeature->m_vp9PicParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DownSamplingFeature::GetDecodeTargetSize(SurfaceWidthT &width, SurfaceHeightT &height)
{
    Vp9BasicFeature *vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(vp9BasicFeature);
    width  = (uint32_t)vp9BasicFeature->m_vp9PicParams->FrameWidthMinus1 + 1;
    height = (uint32_t)vp9BasicFeature->m_vp9PicParams->FrameHeightMinus1 + 1;
    return MOS_STATUS_SUCCESS;
}

}

#endif

