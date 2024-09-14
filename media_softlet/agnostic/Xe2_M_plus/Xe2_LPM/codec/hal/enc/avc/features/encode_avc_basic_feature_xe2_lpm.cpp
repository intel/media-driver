/*
* Copyright (c) 2021 - 2024, Intel Corporation
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
//! \file     encode_avc_basic_feature_xe2_lpm.cpp
//! \brief    Defines the common interface for encode avc Xe2_LPM parameter
//!

#include "encode_avc_basic_feature_xe2_lpm.h"
#include "mhw_vdbox_vdenc_hwcmd_xe2_lpm.h"

namespace encode
{

MOS_STATUS AvcBasicFeatureXe2_Lpm::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    ENCODE_CHK_STATUS_RETURN(AvcBasicFeature::Update(params));

    EncoderParams* encodeParams = (EncoderParams*)params;

    // raw surface
    if (IS_RGB_FORMAT(m_rawSurface.Format))
    {
        ENCODE_CHK_STATUS_RETURN(m_allocator->UpdateResourceUsageType(&m_rawSurface.OsResource, MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE));
    }
    
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, AvcBasicFeatureXe2_Lpm)
{
    AvcBasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params);

    if (m_seqParam->EnableStreamingBufferLLC || m_seqParam->EnableStreamingBufferDDR)
    {
        params.streamingBufferConfig = mhw::vdbox::vdenc::xe2_lpm_base::xe2_lpm::Cmd::VDENC_PIPE_MODE_SELECT_CMD::STREAMING_BUFFER_64;
        params.captureMode           = mhw::vdbox::vdenc::xe2_lpm_base::xe2_lpm::Cmd::VDENC_PIPE_MODE_SELECT_CMD::CAPTURE_MODE_PARALLEFROMCAMERAPIPE;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
