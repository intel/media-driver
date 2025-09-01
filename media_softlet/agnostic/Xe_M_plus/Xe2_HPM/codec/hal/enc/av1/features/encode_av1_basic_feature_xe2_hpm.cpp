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
//! \file     encode_av1_basic_feature_xe2_hpm.cpp
//! \brief    Defines the Xe2_HPM class for encode av1 basic feature
//!

#include "encode_av1_basic_feature_xe2_hpm.h"
#include "encode_av1_vdenc_const_settings.h"

namespace encode
{

MOS_STATUS Av1BasicFeatureXe2_Hpm::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeatureXe_Lpm_Plus_Base::Update(params));
    if (m_roundingMethod == RoundingMethod::fixedRounding && m_av1SeqParams && m_av1SeqParams->ScenarioInfo == ESCENARIO_VIDEOCONFERENCE && m_av1SeqParams->GopRefDist == 1)
    {
        m_roundingMethod = RoundingMethod::lookUpTableRounding;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
