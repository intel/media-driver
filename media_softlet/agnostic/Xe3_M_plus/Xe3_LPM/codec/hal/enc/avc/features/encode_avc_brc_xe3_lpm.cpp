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
//! \file     encode_avc_brc_xe3_lpm.cpp
//! \brief    Defines the common interface for encode avc Xe3_LPM brc feature 
//!

#include "encode_avc_brc_xe3_lpm.h"
#include "encode_avc_huc_brc_init_packet.h"
#include "codec_def_encode_avc.h"

namespace encode
{

MOS_STATUS AvcEncodeBRCXe3_Lpm::SetDmemForInit(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(AvcEncodeBRC::SetDmemForInit(params));

    if (m_basicFeature->m_seqParam->RateControlMethod != RATECONTROL_VBR)
    {
        return MOS_STATUS_SUCCESS;
    }

    ENCODE_CHK_NULL_RETURN(params);
    auto hucVdencBrcInitDmem = (VdencAvcHucBrcInitDmem*)params;

    if (m_basicFeature->m_minMaxQpControlEnabled)
    {
        hucVdencBrcInitDmem->INIT_MinQP_U16 = m_basicFeature->m_iMinQp;
        hucVdencBrcInitDmem->INIT_MaxQP_U16 = m_basicFeature->m_iMaxQp;
    }
    else
    {
        hucVdencBrcInitDmem->INIT_MinQP_U16 = CODEC_AVC_MIN_QP5;
        hucVdencBrcInitDmem->INIT_MaxQP_U16 = CODEC_AVC_MAX_QP;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
