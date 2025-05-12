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
#include "mhw_vdbox_vdenc_hwcmd_xe3_lpm.h"

namespace encode
{

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, AvcBasicFeatureXe3_Lpm)
{
    AvcBasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params);

    if (m_seqParam->EnableStreamingBufferLLC || m_seqParam->EnableStreamingBufferDDR)
    {
        params.streamingBufferConfig = mhw::vdbox::vdenc::xe3_lpm_base::xe3_lpm::Cmd::VDENC_PIPE_MODE_SELECT_CMD::STREAMING_BUFFER_64;
        params.captureMode           = mhw::vdbox::vdenc::xe3_lpm_base::xe3_lpm::Cmd::VDENC_PIPE_MODE_SELECT_CMD::CAPTURE_MODE_PARALLEFROMCAMERAPIPE;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
