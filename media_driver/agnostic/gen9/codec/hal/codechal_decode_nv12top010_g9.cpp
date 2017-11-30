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
//! \file     codechal_decode_nv12top010_g9.cpp
//! \brief    Implements the conversion from NV12 to P010 format.
//! \details  Implements all functions required by conversion from NV12 to P010 format for GEN9 platform.
//!

#include "codechal_decoder.h"
#include "codechal_decode_nv12top010_g9.h"

extern const unsigned int uiNV12ToP010KernelSizeGen9;
extern const unsigned int pNV12ToP010KernelBinaryGen9[];

CodechalDecodeNV12ToP010G9::CodechalDecodeNV12ToP010G9(PMOS_INTERFACE osInterface)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    m_nv12ToP010KernelBinary = pNV12ToP010KernelBinaryGen9;
    m_nv12ToP010KernelSize = uiNV12ToP010KernelSizeGen9;

    Init(osInterface);
}

