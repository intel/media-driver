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
//! \file     decode_vvc_pipeline_adapter_xe2_lpm.cpp
//! \brief    Defines the interface to adapt to vvc decode pipeline
//!

#include "decode_vvc_pipeline_adapter_xe2_lpm.h"
#include "decode_utils.h"

DecodeVvcPipelineAdapterXe2Lpm::DecodeVvcPipelineAdapterXe2Lpm(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipelineAdapter(hwInterface, debugInterface)
{
    DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);
    m_osInterface->pfnVirtualEngineSupported(m_osInterface, true, true);
    Mos_SetVirtualEngineSupported(m_osInterface, true);
}

MOS_STATUS DecodeVvcPipelineAdapterXe2Lpm::BeginFrame()
{
    DECODE_FUNC_CALL();
    decode::DecodePipelineParams decodeParams;
    decodeParams.m_pipeMode = decode::decodePipeModeBegin;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));
    return m_decoder->Execute();
}

MOS_STATUS DecodeVvcPipelineAdapterXe2Lpm::EndFrame()
{
    DECODE_FUNC_CALL();
    decode::DecodePipelineParams decodeParams;
    decodeParams.m_pipeMode = decode::decodePipeModeEnd;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));
    return m_decoder->Execute();
}

MOS_STATUS DecodeVvcPipelineAdapterXe2Lpm::Allocate(CodechalSetting *codecHalSettings)
{
    DECODE_FUNC_CALL();
    m_decoder = std::make_shared<decode::VvcPipelineXe2_Lpm>(m_hwInterface, m_debugInterface);
    DECODE_CHK_NULL(m_decoder);
    return m_decoder->Init(codecHalSettings);
}

MOS_STATUS DecodeVvcPipelineAdapterXe2Lpm::Execute(void *params)
{
    DECODE_FUNC_CALL();
    decode::DecodePipelineParams decodeParams;
    decodeParams.m_params = (CodechalDecodeParams*)params;
    decodeParams.m_pipeMode = decode::decodePipeModeProcess;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));
    return m_decoder->Execute();
}

MOS_STATUS DecodeVvcPipelineAdapterXe2Lpm::GetStatusReport(
    void                *status,
    uint16_t            numStatus)
{
    DECODE_FUNC_CALL();
    return m_decoder->GetStatusReport(status, numStatus);
}

bool DecodeVvcPipelineAdapterXe2Lpm::IsIncompletePicture()
{
     return (!m_decoder->IsCompleteBitstream());
}

MOS_SURFACE *DecodeVvcPipelineAdapterXe2Lpm::GetDummyReference()
{
    DECODE_FUNC_CALL();
    return m_decoder->GetDummyReference();
}

CODECHAL_DUMMY_REFERENCE_STATUS DecodeVvcPipelineAdapterXe2Lpm::GetDummyReferenceStatus()
{
    DECODE_FUNC_CALL();
    return m_decoder->GetDummyReferenceStatus();
}

void DecodeVvcPipelineAdapterXe2Lpm::SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_STATUS status)
{
    DECODE_FUNC_CALL();
    m_decoder->SetDummyReferenceStatus(status);
}

uint32_t DecodeVvcPipelineAdapterXe2Lpm::GetCompletedReport()
{
    return m_decoder->GetCompletedReport();
}

void DecodeVvcPipelineAdapterXe2Lpm::Destroy()
{
    DECODE_FUNC_CALL();

    m_decoder->Destroy();
}


MOS_GPU_CONTEXT DecodeVvcPipelineAdapterXe2Lpm::GetDecodeContext()
{
    DECODE_FUNC_CALL();

    return m_decoder->GetDecodeContext();
}

GPU_CONTEXT_HANDLE DecodeVvcPipelineAdapterXe2Lpm::GetDecodeContextHandle()
{
    DECODE_FUNC_CALL();

    return m_decoder->GetDecodeContextHandle();
}

#ifdef _DECODE_PROCESSING_SUPPORTED
bool DecodeVvcPipelineAdapterXe2Lpm::IsDownSamplingSupported()
{
    return false;
}
#endif
