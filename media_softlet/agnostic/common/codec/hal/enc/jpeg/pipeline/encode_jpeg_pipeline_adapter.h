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
//! \file     encode_jpeg_pipeline_adapter.h
//! \brief    Defines the interface to adapt to jpeg encode pipeline
//!

#ifndef __ENCODE_JPEG_PIPELINE_ADAPTER_H__
#define __ENCODE_JPEG_PIPELINE_ADAPTER_H__

#include "encode_pipeline_adapter.h"
#include "encode_jpeg_pipeline.h"

class EncodeJpegPipelineAdapter : public EncoderPipelineAdapter
{
public:
    EncodeJpegPipelineAdapter(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface);

    virtual ~EncodeJpegPipelineAdapter() {}

    virtual MOS_STATUS Execute(void *params);

    virtual MOS_STATUS Allocate(CodechalSetting *codecHalSettings);

    virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus);

    virtual void Destroy();

protected:
    std::shared_ptr<encode::JpegPipeline> m_encoder = nullptr;  //ToDo: think about moving this pointer to base class

MEDIA_CLASS_DEFINE_END(EncodeJpegPipelineAdapter)
};
#endif // !__ENCODE_JPEG_PIPELINE_ADAPTER_H__

