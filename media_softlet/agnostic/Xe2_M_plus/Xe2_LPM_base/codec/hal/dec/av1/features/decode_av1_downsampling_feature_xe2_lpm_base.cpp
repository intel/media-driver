/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_av1_downsampling_feature_xe2_lpm_base.cpp
//! \brief    Defines the interface for Av1 decode downsampling feature
//! \details  The Av1 decode downsampling feature interface is maintaining the down sampling context.
//!
#include "decode_av1_downsampling_feature_xe2_lpm_base.h"
#include "decode_av1_basic_feature.h"
#include "decode_av1_reference_frames.h"
#include "decode_utils.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
Av1DownSamplingFeatureXe2_Lpm_Base::Av1DownSamplingFeatureXe2_Lpm_Base(
    MediaFeatureManager *featureManager, DecodeAllocator *allocator, PMOS_INTERFACE osInterface)
    : DecodeDownSamplingFeature(featureManager, allocator, osInterface)
{
}

Av1DownSamplingFeatureXe2_Lpm_Base::~Av1DownSamplingFeatureXe2_Lpm_Base()
{
}

MOS_STATUS Av1DownSamplingFeatureXe2_Lpm_Base::GetRefFrameList(std::vector<uint32_t> &refFrameList)
{
    DECODE_FUNC_CALL();
    Av1BasicFeature *av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(av1BasicFeature);

    std::vector<uint32_t> refFrameIndexList;
    refFrameIndexList.clear();
    for (auto i = 0; i < av1TotalRefsPerFrame; i++)
    {
        uint8_t index = av1BasicFeature->m_av1PicParams->m_refFrameMap[i].FrameIdx;
        if (index < CODECHAL_MAX_DPB_NUM_AV1)
        {
            refFrameIndexList.push_back(index);
        }
    }

    refFrameList.clear();
    for (uint32_t frameIdx : refFrameIndexList)
    {
        refFrameList.push_back(frameIdx);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DownSamplingFeatureXe2_Lpm_Base::GetDecodeTargetSize(SurfaceWidthT &width, SurfaceHeightT &height)
{
    width  = m_basicFeature->m_width;
    height = m_basicFeature->m_height;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DownSamplingFeatureXe2_Lpm_Base::GetDecodeTargetFormat(MOS_FORMAT &format)
{
    DECODE_FUNC_CALL()

    auto av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(av1BasicFeature);

    auto av1PicParams = av1BasicFeature->m_av1PicParams;
    DECODE_CHK_NULL(av1PicParams);

    if (av1PicParams->m_profile == 0)
    {
        if (av1PicParams->m_bitDepthIdx == 0)
        {
            format = Format_NV12;
        }
        else if (av1PicParams->m_bitDepthIdx == 1)
        {
            format = Format_P010;
        }
        else
        {
            DECODE_ASSERTMESSAGE("Unsupported sub-sampling format!");
        }
    }
    else
    {
        DECODE_ASSERTMESSAGE("The profile has not been supported yet!");
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DownSamplingFeatureXe2_Lpm_Base::UpdateDecodeTarget(MOS_SURFACE &surface)
{
    DECODE_FUNC_CALL();
    Av1BasicFeature *av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(av1BasicFeature);

    DECODE_CHK_STATUS(av1BasicFeature->UpdateDestSurface(surface));

    Av1ReferenceFrames &refFrame = av1BasicFeature->m_refFrames;
    PCODEC_REF_LIST_AV1 pRefList = refFrame.m_currRefList;
    DECODE_CHK_NULL(pRefList);
    DECODE_CHK_STATUS(refFrame.UpdateCurResource(pRefList));
    return MOS_STATUS_SUCCESS;
}

}  // namespace decode

#endif
