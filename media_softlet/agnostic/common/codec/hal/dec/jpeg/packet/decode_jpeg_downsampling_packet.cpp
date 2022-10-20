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
//! \file     decode_jpeg_downsampling_packet.cpp
//! \brief    Defines the interface for jpeg decode down sampling sub packet
//!
#include "decode_jpeg_downsampling_packet.h"
#include "decode_jpeg_basic_feature.h"

#ifdef _DECODE_PROCESSING_SUPPORTED
namespace decode
{
JpegDownSamplingPkt::JpegDownSamplingPkt(DecodePipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
    : DecodeDownSamplingPkt(pipeline, hwInterface)
{
    m_jpegPipeline = dynamic_cast<JpegPipeline *>(pipeline);
}

MOS_STATUS JpegDownSamplingPkt::Init()
{
    DECODE_CHK_STATUS(DecodeDownSamplingPkt::Init());
    DECODE_CHK_NULL(m_jpegPipeline);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDownSamplingPkt::SetSfcMode(MEDIA_SFC_INTERFACE_MODE &mode)
{
    mode.veboxSfcEnabled = 0;
    mode.vdboxSfcEnabled = 1;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDownSamplingPkt::InitSfcParams(VDBOX_SFC_PARAMS &sfcParams)
{
    JpegBasicFeature *jpegBasicFeature = dynamic_cast<JpegBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(jpegBasicFeature);
    DECODE_CHK_STATUS(DecodeDownSamplingPkt::InitSfcParams(sfcParams));
    //Align Jpeg sfc output surface size
    sfcParams.output.surface->dwWidth  = jpegBasicFeature->m_destSurface.dwWidth;
    sfcParams.output.surface->dwHeight = jpegBasicFeature->m_destSurface.dwHeight;
    sfcParams.videoParams.jpeg.jpegChromaType = (CodecDecodeJpegChromaType)jpegBasicFeature->m_jpegPicParams->m_chromaType;
    return MOS_STATUS_SUCCESS;
}

}
#endif