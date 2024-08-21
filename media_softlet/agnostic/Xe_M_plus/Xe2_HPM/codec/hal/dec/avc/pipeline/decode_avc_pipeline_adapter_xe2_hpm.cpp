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
//! \file     decode_avc_pipeline_adapter_xe2_hpm.cpp
//! \brief    Defines the interface to adapt to avc decode pipeline
//!

#include "decode_avc_pipeline_adapter_xe2_hpm.h"
#include "decode_utils.h"

DecodeAvcPipelineAdapterXe2_Hpm::DecodeAvcPipelineAdapterXe2_Hpm(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipelineAdapter(hwInterface, debugInterface)
{
    DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);
    m_osInterface->pfnVirtualEngineSupported(m_osInterface, true, true);
    Mos_SetVirtualEngineSupported(m_osInterface, true);
}

MOS_STATUS DecodeAvcPipelineAdapterXe2_Hpm::BeginFrame()
{
    DECODE_FUNC_CALL();

    decode::DecodePipelineParams decodeParams;
    decodeParams.m_pipeMode = decode::decodePipeModeBegin;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));

    return m_decoder->Execute();
}

MOS_STATUS DecodeAvcPipelineAdapterXe2_Hpm::EndFrame()
{
    DECODE_FUNC_CALL();

    decode::DecodePipelineParams decodeParams;
    decodeParams.m_pipeMode = decode::decodePipeModeEnd;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));

    return m_decoder->Execute();
}

MOS_STATUS DecodeAvcPipelineAdapterXe2_Hpm::Allocate(CodechalSetting *codecHalSettings)
{
    DECODE_FUNC_CALL();

    m_decoder = std::make_shared<decode::AvcPipelineXe2_Hpm>(m_hwInterface, m_debugInterface);
    DECODE_CHK_NULL(m_decoder);

    return m_decoder->Init(codecHalSettings);
}

MOS_STATUS DecodeAvcPipelineAdapterXe2_Hpm::Execute(void *params)
{
    DECODE_FUNC_CALL();

    decode::DecodePipelineParams decodeParams;
    decodeParams.m_params   = (CodechalDecodeParams *)params;
    decodeParams.m_pipeMode = decode::decodePipeModeProcess;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));

    return m_decoder->Execute();
}

MOS_STATUS DecodeAvcPipelineAdapterXe2_Hpm::GetStatusReport(
    void *   status,
    uint16_t numStatus)
{
    DECODE_FUNC_CALL();
    return m_decoder->GetStatusReport(status, numStatus);
}

bool DecodeAvcPipelineAdapterXe2_Hpm::IsIncompletePicture()
{
    return (!m_decoder->IsCompleteBitstream());
}

#ifdef _DECODE_PROCESSING_SUPPORTED
bool DecodeAvcPipelineAdapterXe2_Hpm::IsDownSamplingSupported()
{
    return m_decoder->IsDownSamplingSupported();
}
#endif

MOS_SURFACE *DecodeAvcPipelineAdapterXe2_Hpm::GetDummyReference()
{
    DECODE_FUNC_CALL();
    return m_decoder->GetDummyReference();
}

CODECHAL_DUMMY_REFERENCE_STATUS DecodeAvcPipelineAdapterXe2_Hpm::GetDummyReferenceStatus()
{
    DECODE_FUNC_CALL();
    return m_decoder->GetDummyReferenceStatus();
}

void DecodeAvcPipelineAdapterXe2_Hpm::SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_STATUS status)
{
    DECODE_FUNC_CALL();
    m_decoder->SetDummyReferenceStatus(status);
}

uint32_t DecodeAvcPipelineAdapterXe2_Hpm::GetCompletedReport()
{
    return m_decoder->GetCompletedReport();
}

void DecodeAvcPipelineAdapterXe2_Hpm::Destroy()
{
    DECODE_FUNC_CALL();
    m_decoder->Destroy();
}

MOS_GPU_CONTEXT DecodeAvcPipelineAdapterXe2_Hpm::GetDecodeContext()
{
    DECODE_FUNC_CALL();
    return m_decoder->GetDecodeContext();
}

GPU_CONTEXT_HANDLE DecodeAvcPipelineAdapterXe2_Hpm::GetDecodeContextHandle()
{
    DECODE_FUNC_CALL();

    return m_decoder->GetDecodeContextHandle();
}