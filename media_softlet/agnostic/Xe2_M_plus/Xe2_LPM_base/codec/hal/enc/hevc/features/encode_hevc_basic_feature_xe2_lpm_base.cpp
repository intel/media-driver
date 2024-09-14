/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     encode_hevc_basic_feature_xe2_lpm_base.cpp
//! \brief    Defines the common interface for encode hevc Xe2_LPM base parameter
//!

#include "encode_hevc_basic_feature_xe2_lpm_base.h"

namespace encode
{

MOS_STATUS HevcBasicFeatureXe2_Lpm_Base::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    ENCODE_CHK_STATUS_RETURN(HevcBasicFeature::Update(params));

    EncoderParams* encodeParams = (EncoderParams*)params;

    // raw surface
    if (IS_RGB_FORMAT(m_rawSurface.Format))
    {
        ENCODE_CHK_STATUS_RETURN(m_allocator->UpdateResourceUsageType(&m_rawSurface.OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE));
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
