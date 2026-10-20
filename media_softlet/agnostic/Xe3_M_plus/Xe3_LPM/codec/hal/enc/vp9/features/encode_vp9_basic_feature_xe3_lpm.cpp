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
//! \file     encode_vp9_basic_feature_Xe3_Lpm_base.cpp
//! \brief    Defines the Xe3_LPM+ common class for encode vp9 basic feature
//!

#include "encode_vp9_basic_feature_xe3_lpm.h"

using namespace mhw::vdbox;

namespace encode
{
MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Vp9BasicFeatureXe3_Lpm)
{
    ENCODE_CHK_STATUS_RETURN(Vp9BasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params));

    params.verticalShift32Minus1 = 0;
    params.numVerticalReqMinus1 = 11;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, Vp9BasicFeatureXe3_Lpm)
{
    ENCODE_CHK_STATUS_RETURN(Vp9BasicFeature::MHW_SETPAR_F(VDENC_CMD2)(params));
    
    // DW2
    params.temporalMvp = false;
    // DW17
    params.temporalMvEnableForIntegerSearch = params.temporalMvp; // TemporalMVEnableForIntegerSearch
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
