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
//! \file     decode_cp_bitstream_m12.h
//! \brief    Defines the common interface for decode cp bitstream
//! \details  Defines the interface to handle the decode cp bitstream
//!

#ifndef __DECODE_CP_BITSTREAM_M12_H__
#define __DECODE_CP_BITSTREAM_M12_H__

#include "decode_cp_bitstream.h"

namespace decode
{
class DecodeStreamOutM12 : public DecodeStreamOut
{
public:
    DecodeStreamOutM12(DecodePipeline *pipeline, MediaTask *task, uint8_t numVdbox, CodechalHwInterface *hwInterface)
        : DecodeStreamOut(pipeline, task, numVdbox)
    {}

    virtual ~DecodeStreamOutM12(){}
MEDIA_CLASS_DEFINE_END(decode__DecodeStreamOutM12)
};
}  // namespace decode

#endif
