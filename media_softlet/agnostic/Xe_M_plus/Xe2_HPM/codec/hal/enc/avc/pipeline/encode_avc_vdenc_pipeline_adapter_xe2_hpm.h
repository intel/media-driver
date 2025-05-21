/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     encode_avc_vdenc_pipeline_adapter_xe2_hpm.h
//! \brief    Defines the interface to adapt to avc vdenc encode pipeline Xe2_HPM
//!

#ifndef __ENCODE_AVC_VDENC_PIPELINE_ADAPTER_XE2_HPM_H__
#define __ENCODE_AVC_VDENC_PIPELINE_ADAPTER_XE2_HPM_H__

#include "encode_avc_vdenc_pipeline_adapter_xe_lpm_plus_base.h"

class EncodeAvcVdencPipelineAdapterXe2_Hpm : public EncodeAvcVdencPipelineAdapterXe_Lpm_Plus_Base
{
public:
    EncodeAvcVdencPipelineAdapterXe2_Hpm(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~EncodeAvcVdencPipelineAdapterXe2_Hpm() {}
    virtual MOS_STATUS Allocate(CodechalSetting *codecHalSettings) override;

    virtual MOS_STATUS ResolveMetaData(PMOS_RESOURCE pInput, PMOS_RESOURCE pOutput) override;

MEDIA_CLASS_DEFINE_END(EncodeAvcVdencPipelineAdapterXe2_Hpm)
};
#endif // !__ENCODE_AVC_VDENC_PIPELINE_ADAPTER_XE2_HPM_H__
