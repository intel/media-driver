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
//! \file     codechal_encode_jpeg_g11_jsl_ehl.h
//! \brief    Defines the encode interface extension for JPEG.
//! \details  Defines all types, macros, and functions required by CodecHal for JPEG encoding. Definitions are not externally facing.
//!

#ifndef __CODECHAL_ENCODEC_JPEG_G11_EHLJSLLP_H__
#define __CODECHAL_ENCODEC_JPEG_G11_EHLJSLLP_H__

#include "codechal_encode_jpeg_g11.h"

class CodechalEncodeJpegStateG11JslEhl : public CodechalEncodeJpegStateG11
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeJpegStateG11JslEhl(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo)
        : CodechalEncodeJpegStateG11(hwInterface, debugInterface, standardInfo)
    {
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeJpegStateG11JslEhl() { }


    virtual MOS_STATUS GetStatusReport(
        EncodeStatus* encodeStatus,
        EncodeStatusReport* encodeStatusReport) override;
};

#endif  // __CODECHAL_ENCODEC_JPEG_G11_EHLJSLLP_H__
