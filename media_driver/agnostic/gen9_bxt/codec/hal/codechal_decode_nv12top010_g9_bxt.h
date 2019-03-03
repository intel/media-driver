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
//! \file     codechal_decode_nv12top010_g9_bxt.h
//! \brief    Defines the interface for conversion from NV12 to P010 format for GEN9 BXT platform.
//!

#ifndef __CODECHAL_DECODE_NV12TOP010_G9_BXT_H__
#define __CODECHAL_DECODE_NV12TOP010_G9_BXT_H__

#include "codechal_decode_nv12top010.h"

//!
//! \class CodechalDecodeNv12ToP010G9Bxt
//! \brief This class converts NV12 to P010 format for GEN9 BXT platform.
//!
class CodechalDecodeNv12ToP010G9Bxt : public CodechalDecodeNV12ToP010
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalDecodeNv12ToP010G9Bxt(PMOS_INTERFACE osInterface);
};

#endif

