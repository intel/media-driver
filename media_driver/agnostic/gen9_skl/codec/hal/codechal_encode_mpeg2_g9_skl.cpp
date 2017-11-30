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
//! \file     codechal_encode_mpeg2_g9_skl.cpp
//! \brief    MPEG2 dual-pipe encoder for GEN9 SKL.
//!

#include "codechal_encode_mpeg2_g9_skl.h"

void CodechalEncodeMpeg2G9Skl::UpdateSSDSliceCount()
{
    CodechalEncodeMpeg2::UpdateSSDSliceCount();

    uint32_t sliceCount;
    if (m_frameHeight * m_frameWidth >= 1280*720)
    {
        sliceCount = 2;
    }
    else
    {
        sliceCount = 1;
    }

    if (m_osInterface->pfnSetSliceCount)
    {
        m_osInterface->pfnSetSliceCount(m_osInterface, &sliceCount);
    }
}