/*
* Copyright (c) 2022-2025, Intel Corporation
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
//! \file     encode_avc_vdenc_pipeline_adapter_xe3_lpm.h
//! \brief    Defines the interface to adapt to avc vdenc encode pipeline Xe3_LPM
//!

#ifndef __ENCODE_AVC_VDENC_PIPELINE_ADAPTER_XE3_LPM_H__
#define __ENCODE_AVC_VDENC_PIPELINE_ADAPTER_XE3_LPM_H__

#include "encode_avc_vdenc_pipeline_adapter.h"

class EncodeAvcVdencPipelineAdapterXe3_Lpm : public EncodeAvcVdencPipelineAdapter
{
public:
    EncodeAvcVdencPipelineAdapterXe3_Lpm(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface) : EncodeAvcVdencPipelineAdapter(hwInterface, debugInterface) {}

    virtual ~EncodeAvcVdencPipelineAdapterXe3_Lpm() {}

    virtual MOS_STATUS Allocate(CodechalSetting *codecHalSettings) override;

    virtual MOS_STATUS ResolveMetaData(PMOS_RESOURCE pInput, PMOS_RESOURCE pOutput);

MEDIA_CLASS_DEFINE_END(EncodeAvcVdencPipelineAdapterXe3_Lpm)
};

#endif // !__ENCODE_AVC_VDENC_PIPELINE_ADAPTER_XE3_LPM_H__

