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
//! \file     decode_mpeg2_reference_frames.cpp
//! \brief    Defines reference list related logic for mpeg2 decode
//!

#include "decode_mpeg2_basic_feature.h"
#include "decode_utils.h"
#include "codec_utilities_next.h"
#include "decode_mpeg2_reference_frames.h"
#include "codec_def_decode_mpeg2.h"

namespace decode{

Mpeg2ReferenceFrames::Mpeg2ReferenceFrames()
{
    MOS_ZeroMemory(m_refList, sizeof(m_refList));
}

MOS_STATUS Mpeg2ReferenceFrames::Init(Mpeg2BasicFeature *basicFeature, DecodeAllocator& allocator)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(basicFeature);

    m_basicFeature = basicFeature;
    m_allocator = &allocator;
    DECODE_CHK_STATUS(CodecUtilities::CodecHalAllocateDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2));

    //mark all reference res as invalid when they are no initialized
    for (uint32_t i = 0; i < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2; i++)
    {
        m_refList[i]->RefPic.PicFlags = PICTURE_INVALID;
    }

    return MOS_STATUS_SUCCESS;
}

Mpeg2ReferenceFrames::~Mpeg2ReferenceFrames()
{
    DECODE_FUNC_CALL();
    CodecUtilities::CodecHalFreeDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2);
    m_activeReferenceList.clear();
}

MOS_STATUS Mpeg2ReferenceFrames::UpdatePicture(CodecDecodeMpeg2PicParams &picParams)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(UpdateCurFrame(picParams));
    DECODE_CHK_STATUS(UpdateCurRefList(picParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2ReferenceFrames::UpdateCurResource(const CodecDecodeMpeg2PicParams &picParams)
{
    DECODE_FUNC_CALL();

    PCODEC_REF_LIST destEntry = m_refList[picParams.m_currPic.FrameIdx];
    destEntry->resRefPic = m_basicFeature->m_destSurface.OsResource;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2ReferenceFrames::UpdateCurFrame(const CodecDecodeMpeg2PicParams &picParams)
{
    DECODE_FUNC_CALL();

    CODEC_PICTURE currPic     = picParams.m_currPic;
    PCODEC_REF_LIST destEntry = m_refList[currPic.FrameIdx];
    destEntry->RefPic         = currPic;
    DECODE_CHK_STATUS(UpdateCurResource(picParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2ReferenceFrames::UpdateCurRefList(const CodecDecodeMpeg2PicParams &picParams)
{
    DECODE_FUNC_CALL();

    // Do not use data that has not been initialized
    if (CodecHal_PictureIsInvalid(m_refList[m_basicFeature->m_fwdRefIdx]->RefPic))
    {
        m_basicFeature->m_fwdRefIdx = picParams.m_currPic.FrameIdx;
    }
    if (CodecHal_PictureIsInvalid(m_refList[m_basicFeature->m_bwdRefIdx]->RefPic))
    {
        m_basicFeature->m_bwdRefIdx = picParams.m_currPic.FrameIdx;
    }

    // Override reference list with ref surface passed from DDI if needed
    uint8_t surfCount = 0;
    uint8_t surfIndex = 0;
    while (surfCount < m_basicFeature->m_refSurfaceNum && surfIndex < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        if (!m_allocator->ResourceIsNull(&m_basicFeature->m_refFrameSurface[surfIndex].OsResource))
        {
            m_refList[surfIndex]->resRefPic = m_basicFeature->m_refFrameSurface[surfIndex].OsResource;
            surfCount++;
        }
        surfIndex++;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
