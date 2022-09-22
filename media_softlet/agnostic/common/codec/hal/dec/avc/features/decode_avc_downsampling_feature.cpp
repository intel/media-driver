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
//! \file     decode_avc_downsampling_feature.cpp
//! \brief    Defines the interface for avc decode downsampling feature
//! \details  The avc decode downsampling feature interface is maintaining the down sampling context.
//!
#include "decode_avc_downsampling_feature.h"
#include "decode_avc_basic_feature.h"
#include "decode_utils.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
AvcDownSamplingFeature::AvcDownSamplingFeature(
    MediaFeatureManager *featureManager, DecodeAllocator *allocator, PMOS_INTERFACE osInterface) :
    DecodeDownSamplingFeature(featureManager, allocator, osInterface)
{
}

AvcDownSamplingFeature::~AvcDownSamplingFeature()
{
}

MOS_STATUS AvcDownSamplingFeature::GetRefFrameList(std::vector<uint32_t> &refFrameList)
{
    AvcBasicFeature *avcBasicFeature = dynamic_cast<AvcBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(avcBasicFeature);

    const std::vector<uint8_t> & activeRefList =
        avcBasicFeature->m_refFrames.GetActiveReferenceList(*avcBasicFeature->m_avcPicParams);

    refFrameList.clear();

    for (uint8_t frameIdx : activeRefList)
    {
        refFrameList.push_back(frameIdx);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDownSamplingFeature::UpdateInternalTargets(DecodeBasicFeature &basicFeature)
{
    DECODE_FUNC_CALL();

    AvcBasicFeature *avcBasicFeature = dynamic_cast<AvcBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(avcBasicFeature);

    uint32_t curFrameIdx = avcBasicFeature->m_curRenderPic.FrameIdx;
    std::vector<uint32_t> refFrameList;
    DECODE_CHK_STATUS(GetRefFrameList(refFrameList));
    DECODE_CHK_STATUS(m_internalTargets.UpdateRefList(curFrameIdx, refFrameList, avcBasicFeature->m_fixedFrameIdx));

    MOS_SURFACE surface;
    MOS_ZeroMemory(&surface, sizeof(surface));
    DECODE_CHK_STATUS(GetDecodeTargetSize(surface.dwWidth, surface.dwHeight));
    DECODE_CHK_STATUS(GetDecodeTargetFormat(surface.Format));
    DECODE_CHK_STATUS(m_internalTargets.ActiveCurSurf(
        curFrameIdx, &surface, basicFeature.IsMmcEnabled(), resourceOutputPicture));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDownSamplingFeature::GetDecodeTargetSize(SurfaceWidthT &width, SurfaceHeightT &height)
{
    width  = m_basicFeature->m_width;
    height = m_basicFeature->m_height;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDownSamplingFeature::GetDecodeTargetFormat(MOS_FORMAT &format)
{
    AvcBasicFeature *avcBasicFeature = dynamic_cast<AvcBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(avcBasicFeature);
    PCODEC_AVC_PIC_PARAMS avcPicParams = avcBasicFeature->m_avcPicParams;
    DECODE_CHK_NULL(avcPicParams);

    if (avcPicParams->seq_fields.chroma_format_idc == avcChromaFormat420)
    {
        format = Format_NV12;
    }
    else if (avcPicParams->seq_fields.chroma_format_idc == avcChromaFormatMono)
    {
        format = Format_400P;
    }
    else
    {
        DECODE_ASSERTMESSAGE("Unsupported subsampling format!");
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcDownSamplingFeature::UpdateDecodeTarget(MOS_SURFACE &surface)
{
    AvcBasicFeature *avcBasicFeature = dynamic_cast<AvcBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(avcBasicFeature);

    DECODE_CHK_STATUS(avcBasicFeature->UpdateDestSurface(surface));

    DECODE_CHK_NULL(avcBasicFeature->m_avcPicParams);
    AvcReferenceFrames &refFrames = avcBasicFeature->m_refFrames;
    DECODE_CHK_STATUS(refFrames.UpdateCurResource(*avcBasicFeature->m_avcPicParams));

    return MOS_STATUS_SUCCESS;
}

}

#endif
