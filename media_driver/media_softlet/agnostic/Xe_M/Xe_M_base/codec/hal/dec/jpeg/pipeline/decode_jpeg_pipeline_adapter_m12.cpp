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
//! \file     decode_jpeg_pipeline_adapter_m12.cpp
//! \brief    Defines the interface to adapt to jpeg decode pipeline
//!

#include "decode_jpeg_pipeline_adapter_m12.h"
#include "decode_utils.h"

DecodeJpegPipelineAdapterM12::DecodeJpegPipelineAdapterM12(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipelineAdapter(*hwInterface, debugInterface)
{
    DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);
    m_osInterface->pfnVirtualEngineSupported(m_osInterface, true, true);
    Mos_SetVirtualEngineSupported(m_osInterface, true);
    m_hwInterface = hwInterface;
}

MOS_STATUS DecodeJpegPipelineAdapterM12::BeginFrame()
{
    DECODE_FUNC_CALL();

    decode::DecodePipelineParams decodeParams;
    decodeParams.m_pipeMode = decode::decodePipeModeBegin;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));
    return m_decoder->Execute();
}

MOS_STATUS DecodeJpegPipelineAdapterM12::EndFrame()
{
    DECODE_FUNC_CALL();

    decode::DecodePipelineParams decodeParams;
    decodeParams.m_pipeMode = decode::decodePipeModeEnd;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));
    return m_decoder->Execute();
}

MOS_STATUS DecodeJpegPipelineAdapterM12::Allocate(CodechalSetting *codecHalSettings)
{
    DECODE_FUNC_CALL();

    m_decoder = std::make_shared<decode::JpegPipelineM12>(m_hwInterface, m_debugInterface);
    DECODE_CHK_NULL(m_decoder);

    return m_decoder->Init(codecHalSettings);
}

MOS_STATUS DecodeJpegPipelineAdapterM12::Execute(void *params)
{
    DECODE_FUNC_CALL();

    decode::DecodePipelineParams decodeParams;
    decodeParams.m_params = (CodechalDecodeParams*)params;
    decodeParams.m_pipeMode = decode::decodePipeModeProcess;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));
    return m_decoder->Execute();
}

MOS_STATUS DecodeJpegPipelineAdapterM12::GetStatusReport(
    void                *status,
    uint16_t            numStatus)
{
    DECODE_FUNC_CALL();

    return m_decoder->GetStatusReport(status, numStatus);
}

bool DecodeJpegPipelineAdapterM12::IsIncompletePicture()
{
     return (!m_decoder->IsCompleteBitstream());
}

bool DecodeJpegPipelineAdapterM12::IsIncompleteJpegScan()
{
    return (!m_decoder->IsCompleteBitstream());
}

uint32_t DecodeJpegPipelineAdapterM12::GetCompletedReport()
{
    return m_decoder->GetCompletedReport();
}

void DecodeJpegPipelineAdapterM12::Destroy()
{
    DECODE_FUNC_CALL();

    m_decoder->Destroy();
}

MOS_GPU_CONTEXT DecodeJpegPipelineAdapterM12::GetDecodeContext()
{
    DECODE_FUNC_CALL();

    return m_decoder->GetDecodeContext();
}

GPU_CONTEXT_HANDLE DecodeJpegPipelineAdapterM12::GetDecodeContextHandle()
{
    DECODE_FUNC_CALL();

    return m_decoder->GetDecodeContextHandle();
}

MOS_SURFACE* DecodeJpegPipelineAdapterM12::GetDummyReference()
{
    DECODE_FUNC_CALL();

    return m_decoder->GetDummyReference();
}

CODECHAL_DUMMY_REFERENCE_STATUS DecodeJpegPipelineAdapterM12::GetDummyReferenceStatus()
{
    DECODE_FUNC_CALL();
    return m_decoder->GetDummyReferenceStatus();
}

void DecodeJpegPipelineAdapterM12::SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_STATUS status)
{
    DECODE_FUNC_CALL();
    m_decoder->SetDummyReferenceStatus(status);
}

#ifdef _DECODE_PROCESSING_SUPPORTED
bool DecodeJpegPipelineAdapterM12::IsDownSamplingSupported()
{
    return m_decoder->IsDownSamplingSupported();
}
#endif

