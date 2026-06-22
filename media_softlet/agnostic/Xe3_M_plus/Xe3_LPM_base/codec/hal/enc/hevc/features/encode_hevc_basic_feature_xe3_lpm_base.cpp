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
//! \file     encode_hevc_basic_feature_xe3_lpm_base.cpp
//! \brief    Defines the common interface for encode hevc Xe3_LPM base parameter
//!

#include "encode_hevc_basic_feature_xe3_lpm_base.h"

namespace encode
{

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, HevcBasicFeatureXe3_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(HevcBasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params));

    params.chromaPrefetchDisable = m_chromaPrefetchDisable;

    params.verticalShift32Minus1 = 0;
    params.numVerticalReqMinus1  = 11;
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, HevcBasicFeatureXe3_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(HevcBasicFeature::MHW_SETPAR_F(VDENC_CMD2)(params));

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_VBR)
    {
        params.minQp = m_hevcPicParams->BRCMinQp < CODEC_HEVC_MIN_QP1 ? CODEC_HEVC_MIN_QP1 : m_hevcPicParams->BRCMinQp;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
