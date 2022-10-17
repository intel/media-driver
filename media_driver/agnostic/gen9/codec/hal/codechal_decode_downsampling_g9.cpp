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
//! \file     codechal_decode_downsampling_g9.cpp
//! \brief    Implements the decode interface extension for downsampling on Gen9.
//!

#include "codechal_decoder.h"
#include "igcodeckrn_g9.h"
#include "codechal_decode_downsampling_g9.h"

FieldScalingInterfaceG9::FieldScalingInterfaceG9(CodechalHwInterface *hwInterface):
    FieldScalingInterface()
{
    CODECHAL_DECODE_FUNCTION_ENTER;
    m_kernelBase = (uint8_t*)IGCODECKRN_G9;
    InitInterfaceStateHeapSetting(hwInterface);
}