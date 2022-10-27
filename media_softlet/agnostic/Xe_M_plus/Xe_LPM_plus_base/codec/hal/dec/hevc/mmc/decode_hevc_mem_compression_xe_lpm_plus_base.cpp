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
//! \file     decode_hevc_mem_compression_xe_lpm_plus_base.cpp
//! \brief    Defines the common interface for Hevc decode mmc for Xe_LPM_plus+
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of Hevc decode
//!

#include "decode_hevc_mem_compression_xe_lpm_plus_base.h"

namespace decode
{

HevcDecodeMemCompXe_Lpm_Plus_Base::HevcDecodeMemCompXe_Lpm_Plus_Base(CodechalHwInterfaceNext *hwInterface)
    : DecodeMemCompXe_Lpm_Plus_Base(hwInterface), HevcDecodeMemComp(hwInterface)
{
}

}
