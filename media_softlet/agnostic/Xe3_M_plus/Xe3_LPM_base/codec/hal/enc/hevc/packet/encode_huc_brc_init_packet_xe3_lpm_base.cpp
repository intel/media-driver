/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     encode_huc_brc_init_packet_xe3_lpm_base.cpp
//! \brief    Defines the implementation of huc init packet Xe3 LPM Base
//!

#include "encode_huc_brc_init_packet_xe3_lpm_base.h"

namespace encode
{
MOS_STATUS HucBrcInitPktXe3_Lpm_Base::SetDmemBuffer() const
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(HucBrcInitPkt::SetDmemBuffer());

    if (m_basicFeature->m_hevcSeqParams->RateControlMethod != RATECONTROL_VBR)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto hucVdencBrcInitDmem = (VdencHevcHucBrcInitDmem *)
    m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE*>(&m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx]));
    ENCODE_CHK_NULL_RETURN(hucVdencBrcInitDmem);

    hucVdencBrcInitDmem->MinQP_U8 = m_basicFeature->m_hevcPicParams->BRCMinQp < CODEC_HEVC_MIN_QP1 ? CODEC_HEVC_MIN_QP1 : m_basicFeature->m_hevcPicParams->BRCMinQp;

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
