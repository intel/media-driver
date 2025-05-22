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
//! \file     encode_av1_vdenc_pipeline_xe3_lpm.cpp
//! \brief    Defines the interface for av1 vdenc encode pipeline Xe3_LPM
//!
#include "encode_av1_vdenc_pipeline_xe3_lpm.h"
#include "encode_av1_vdenc_feature_manager_xe3_lpm_base.h"
#include "encode_av1_vdenc_packet_xe3_lpm.h"
#include "codechal_debug.h"

namespace encode
{
MOS_STATUS Av1VdencPipelineXe3_Lpm::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Av1VdencPipelineXe3_Lpm_Base::Init(settings));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPipelineXe3_Lpm::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Av1VdencPipelineXe3_Lpm_Base::CreateFeatureManager());
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
