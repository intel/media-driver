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
//! \file     encode_avc_basic_feature_xe3_lpm.cpp
//! \brief    Defines the common interface for encode avc Xe3_LPM parameter
//!

#include "encode_avc_basic_feature_xe3_lpm.h"
#include "codec_def_encode_avc.h"
#include "mhw_vdbox_vdenc_hwcmd_xe3_lpm.h"

namespace encode
{
uint8_t AvcBasicFeatureXe3_Lpm::GetMinAvcQp()
{
    ENCODE_FUNC_CALL();
    
    // For VBR rate control, use minimum QP of 5 (CODEC_AVC_MIN_QP5)
    // For other rate control methods, use base class default of 10 (CODEC_AVC_MIN_QP10)
    if (m_seqParam->RateControlMethod == RATECONTROL_VBR)
    {
        return CODEC_AVC_MIN_QP5;  // Returns 5 for VBR
    }
    
    return AvcBasicFeature::GetMinAvcQp();  // Returns CODEC_AVC_MIN_QP10 (10) from base class
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, AvcBasicFeatureXe3_Lpm)
{
    AvcBasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params);

    params.verticalShift32Minus1   = 0;
    params.numVerticalReqMinus1    = 11;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_IMG_STATE, AvcBasicFeatureXe3_Lpm)
{
    ENCODE_FUNC_CALL();

    if (m_seqParam->RateControlMethod == RATECONTROL_VBR)
    {
        params.minQp = CODEC_AVC_MIN_QP5; // set init minQp to CODEC_AVC_MIN_QP5
    }
    
    AvcBasicFeature::MHW_SETPAR_F(VDENC_AVC_IMG_STATE)(params);
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
