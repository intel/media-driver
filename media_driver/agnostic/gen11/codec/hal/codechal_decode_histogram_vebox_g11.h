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
//! \file     codechal_decode_histogram_vebox_g11.h
//! \brief    Defines the interface for decode histogram from vebox for GEN11 platform.
//!

#ifndef __CODECHAL_DECODE_HISTOGRAM_VEBOX_G11_H__
#define __CODECHAL_DECODE_HISTOGRAM_VEBOX_G11_H__

#include "codechal_decode_histogram_vebox.h"

//!
//! \class CodechalDecodeHistogramVeboxG11
//! \brief This class render histogram through vebox for GEN11 platform.
//!
class CodechalDecodeHistogramVeboxG11 : public CodechalDecodeHistogramVebox
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalDecodeHistogramVeboxG11(
        CodechalHwInterface *hwInterface,
        MOS_INTERFACE *osInterface);
};

#endif