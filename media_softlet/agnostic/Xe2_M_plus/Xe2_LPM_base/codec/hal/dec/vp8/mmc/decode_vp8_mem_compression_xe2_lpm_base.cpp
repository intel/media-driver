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
//! \file     decode_vp8_mem_compression_xe2_lpm_base.cpp
//! \brief    Defines the common interface for vp8 decode mmc for Xe2_LPM
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of vp8 decode
//!

#include "decode_vp8_mem_compression_xe2_lpm_base.h"

namespace decode
{
    Vp8DecodeMemCompXe2_Lpm_Base::Vp8DecodeMemCompXe2_Lpm_Base(CodechalHwInterfaceNext *hwInterface)
        : DecodeMemCompXe2_Lpm_Base(hwInterface), Vp8DecodeMemComp(hwInterface)
    {
    }
}
