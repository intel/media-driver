/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_encode_csc_ds_g8.cpp
//! \brief    This file implements the Csc+Ds feature for all codecs on Gen8 platform
//!

#include "codechal_encoder_base.h"
#include "codechal_encode_csc_ds_g8.h"
#include "codeckrnheader.h"
#include "igcodeckrn_g8.h"

CodechalEncodeCscDsG8::CodechalEncodeCscDsG8(CodechalEncoderState* encoder)
    : CodechalEncodeCscDs(encoder)
{
    m_rawSurfAlignment = 16;  // on Gen8 raw surface has to be 16-aligned
    m_cscKernelUID = IDR_CODEC_Downscale_Copy;
    m_cscCurbeLength = sizeof(CscKernelCurbeData);
    m_kernelBase = (uint8_t*)IGCODECKRN_G8;
}