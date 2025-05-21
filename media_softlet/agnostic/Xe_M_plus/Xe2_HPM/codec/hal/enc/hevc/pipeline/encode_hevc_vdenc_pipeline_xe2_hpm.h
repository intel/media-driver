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
//! \file     encode_hevc_vdenc_pipeline_xe2_hpm.h
//! \brief    Defines the interface for hevc vdenc encode pipeline xe2 hpm
//!
#ifndef __ENCODE_HEVC_VDENC_PIPELINE_XE2_HPM_H__
#define __ENCODE_HEVC_VDENC_PIPELINE_XE2_HPM_H__

#include "encode_hevc_vdenc_pipeline_xe_lpm_plus_base.h"

namespace encode {

class HevcVdencPipelineXe2_Hpm : public HevcVdencPipelineXe_Lpm_Plus_Base
{
public:
    HevcVdencPipelineXe2_Hpm(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~HevcVdencPipelineXe2_Hpm() {}

    MOS_STATUS Init(void *settings) override;

protected:
    virtual MOS_STATUS CreateFeatureManager() override;

MEDIA_CLASS_DEFINE_END(encode__HevcVdencPipelineXe2_Hpm)
};

}
#endif // !__ENCODE_HEVC_VDENC_PIPELINE_XE2_HPM_H__
