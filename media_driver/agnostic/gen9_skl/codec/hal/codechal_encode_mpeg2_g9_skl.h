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
//! \file     codechal_encode_mpeg2_g9_skl.h
//! \brief    MPEG2 dual-pipe encoder for GEN9 SKL platform.
//!

#ifndef __CODECHAL_ENCODE_MPEG2_G9_SKL_H__
#define __CODECHAL_ENCODE_MPEG2_G9_SKL_H__

#include "codechal_encode_mpeg2_g9.h"

//!
//! \class   CodechalEncodeMpeg2G9Skl
//! \brief   MPEG2 dual-pipe encoder base class for GEN9 SKL
//! \details This class defines the member fields, functions for GEN9 SKL platform
//!
class CodechalEncodeMpeg2G9Skl : public CodechalEncodeMpeg2G9
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeMpeg2G9Skl(
        CodechalHwInterface*    hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo):
        CodechalEncodeMpeg2G9(hwInterface, debugInterface, standardInfo) {};

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeMpeg2G9Skl() {};

    //!
    //! \brief    Update the slice count according to the DymanicSliceShutdown policy
    //!
    void UpdateSSDSliceCount();
};

#endif // __CODECHAL_ENCODE_MPEG2_G9_SKL_H__

