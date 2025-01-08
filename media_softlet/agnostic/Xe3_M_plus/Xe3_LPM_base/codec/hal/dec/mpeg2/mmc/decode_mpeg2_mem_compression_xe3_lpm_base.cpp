/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_mpeg2_mem_compression_xe3_lpm_base.cpp
//! \brief    Defines the common interface for MPEG2 decode mmc for Xe3_LPM+
//! \details  The mmc is to handle mmc operations,
//! including compression and decompressin of MPEG2 decode
//!

#include "decode_mpeg2_mem_compression_xe3_lpm_base.h"

namespace decode
{

    Mpeg2DecodeMemCompXe3_Lpm_Base::Mpeg2DecodeMemCompXe3_Lpm_Base(CodechalHwInterfaceNext* hwInterface)
        : DecodeMemCompXe3_Lpm_Base(hwInterface), Mpeg2DecodeMemComp(hwInterface)
    {
    }

    MOS_STATUS Mpeg2DecodeMemCompXe3_Lpm_Base::CheckReferenceList(
        Mpeg2BasicFeature &mpeg2BasicFeature, MOS_MEMCOMP_STATE &preDeblockSurfMmcState, MOS_MEMCOMP_STATE &postDeblockSurfMmcState)
    {
        return MOS_STATUS_SUCCESS;
    }

}  // namespace decode
