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
//! \file     decode_jpeg_input_bitstream.h
//! \brief    Defines the common interface for decode jpeg input bitstream
//!
#ifndef __DECODE_JPEG_INPUT_BITSTREAM_H__
#define __DECODE_JPEG_INPUT_BITSTREAM_H__

#include "decode_input_bitstream.h"
#include "decode_jpeg_basic_feature.h"

namespace decode 
{
class DecodeJpegInputBitstream : public DecodeInputBitstream
{
public:
    //!
    //! \brief  JPEG Decode input bitstream constructor
    //!
    DecodeJpegInputBitstream(DecodePipeline *pipeline, MediaTask *task, uint8_t numVdbox);

    //!
    //! \brief  Decode input bitstream destructor
    //!
    virtual ~DecodeJpegInputBitstream() {}

    MOS_STATUS Init(CodechalSetting &settings) override;

    MOS_STATUS Append(const CodechalDecodeParams &decodeParams) override;
    bool       IsComplete() override;

private:
    JpegBasicFeature *m_jpegBasicFeature = nullptr;  //!< Decode basic feature
    bool             m_completeJpegScan = false;  //to indicate the scan was complete
    bool             m_completeBitStream = false;

MEDIA_CLASS_DEFINE_END(decode__DecodeJpegInputBitstream)
};

}//decode

#endif


