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
//! \file     encode_jpeg_reference_frames.cpp
//! \brief    Defines reference list related logic for encode jpeg
//!

#include "encode_jpeg_reference_frames.h"
#include "encode_jpeg_basic_feature.h"

namespace encode
{

MOS_STATUS JpegReferenceFrames::Init(JpegBasicFeature *basicFeature)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(basicFeature);

    m_basicFeature = basicFeature;
    ENCODE_CHK_STATUS_RETURN(EncodeAllocateDataList(
        m_refList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_JPEG));

    return MOS_STATUS_SUCCESS;
}

JpegReferenceFrames::~JpegReferenceFrames()
{
    ENCODE_FUNC_CALL();

    EncodeFreeDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_JPEG);
}

MOS_STATUS JpegReferenceFrames::UpdatePicture()
{
    ENCODE_FUNC_CALL();

    auto jpegRefList = &m_refList[0];
    uint8_t currRefIdx  = m_basicFeature->m_currOriginalPic.FrameIdx;
    
    jpegRefList[currRefIdx]->sRefRawBuffer   = m_basicFeature->m_rawSurface;
    jpegRefList[currRefIdx]->sFrameNumber    = (int16_t)m_basicFeature->m_frameNum;
    jpegRefList[currRefIdx]->RefPic          = m_basicFeature->m_currOriginalPic;

    jpegRefList[currRefIdx]->resBitstreamBuffer = m_basicFeature->m_resBitstreamBuffer;

    m_currRefList = jpegRefList[currRefIdx];

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
