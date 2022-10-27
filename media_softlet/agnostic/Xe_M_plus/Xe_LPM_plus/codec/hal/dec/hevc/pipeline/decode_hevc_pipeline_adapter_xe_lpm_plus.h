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
//! \file     decode_hevc_pipeline_adapter_xe_lpm_plus.h
//! \brief    Defines the interface to adapt to hevc decode pipeline MTL
//!

#ifndef __DECODE_HEVC_PIPELINE_ADAPTER_XE_LPM_PLUS_H__
#define __DECODE_HEVC_PIPELINE_ADAPTER_XE_LPM_PLUS_H__

#include "decode_hevc_pipeline_adapter_xe_lpm_plus_base.h"

class DecodeHevcPipelineAdapterXe_Lpm_Plus : public DecodeHevcPipelineAdapterXe_Lpm_Plus_Base
{
public:
    DecodeHevcPipelineAdapterXe_Lpm_Plus(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~DecodeHevcPipelineAdapterXe_Lpm_Plus() {}

MEDIA_CLASS_DEFINE_END(DecodeHevcPipelineAdapterXe_Lpm_Plus)
};
#endif // !__DECODE_HEVC_PIPELINE_ADAPTER_XE_LPM_PLUS_H__
