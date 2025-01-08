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
//! \file     decode_hevc_pipeline_adapter_xe3_lpm_base.cpp
//! \brief    Defines the interface to adapt to hevc decode pipeline
//!

#include "decode_hevc_pipeline_adapter_xe3_lpm_base.h"
#include "decode_utils.h"

DecodeHevcPipelineAdapterXe3_Lpm_Base::DecodeHevcPipelineAdapterXe3_Lpm_Base(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipelineAdapter(hwInterface, debugInterface)
{
    DECODE_ASSERT(m_osInterface != nullptr);
    if (m_osInterface != nullptr)
    {
        m_osInterface->pfnVirtualEngineSupported(m_osInterface, true, true);
        Mos_SetVirtualEngineSupported(m_osInterface, true);
    }
}

MOS_STATUS DecodeHevcPipelineAdapterXe3_Lpm_Base::BeginFrame()
{
    DECODE_FUNC_CALL();

    decode::DecodePipelineParams decodeParams;
    decodeParams.m_pipeMode = decode::decodePipeModeBegin;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));
    return m_decoder->Execute();
}

MOS_STATUS DecodeHevcPipelineAdapterXe3_Lpm_Base::EndFrame()
{
    DECODE_FUNC_CALL();

    decode::DecodePipelineParams decodeParams;
    decodeParams.m_pipeMode = decode::decodePipeModeEnd;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));
    return m_decoder->Execute();
}

MOS_STATUS DecodeHevcPipelineAdapterXe3_Lpm_Base::Allocate(CodechalSetting *codecHalSettings)
{
    DECODE_FUNC_CALL();

    m_decoder = std::make_shared<decode::HevcPipelineXe3_Lpm_Base>(m_hwInterface, m_debugInterface);
    DECODE_CHK_NULL(m_decoder);

    return m_decoder->Init(codecHalSettings);
}

MOS_STATUS DecodeHevcPipelineAdapterXe3_Lpm_Base::Execute(void    *params)
{
    DECODE_FUNC_CALL();

    decode::DecodePipelineParams decodeParams;
    decodeParams.m_params = (CodechalDecodeParams*)params;
    decodeParams.m_pipeMode = decode::decodePipeModeProcess;
    DECODE_CHK_STATUS(m_decoder->Prepare(&decodeParams));
    return m_decoder->Execute();
}

MOS_STATUS DecodeHevcPipelineAdapterXe3_Lpm_Base::GetStatusReport(
    void                *status,
    uint16_t            numStatus)
{
    DECODE_FUNC_CALL();

    return m_decoder->GetStatusReport(status, numStatus);
}

bool DecodeHevcPipelineAdapterXe3_Lpm_Base::IsIncompletePicture()
{
     return (!m_decoder->IsCompleteBitstream());
}

#ifdef _DECODE_PROCESSING_SUPPORTED
bool DecodeHevcPipelineAdapterXe3_Lpm_Base::IsDownSamplingSupported()
{
    return m_decoder->IsDownSamplingSupported();
}
#endif

MOS_SURFACE* DecodeHevcPipelineAdapterXe3_Lpm_Base::GetDummyReference()
{
    DECODE_FUNC_CALL();
    return m_decoder->GetDummyReference();
}

CODECHAL_DUMMY_REFERENCE_STATUS DecodeHevcPipelineAdapterXe3_Lpm_Base::GetDummyReferenceStatus()
{
    DECODE_FUNC_CALL();
    return m_decoder->GetDummyReferenceStatus();
}

void DecodeHevcPipelineAdapterXe3_Lpm_Base::SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_STATUS status)
{
    DECODE_FUNC_CALL();
    m_decoder->SetDummyReferenceStatus(status);
}

uint32_t DecodeHevcPipelineAdapterXe3_Lpm_Base::GetCompletedReport()
{
    return m_decoder->GetCompletedReport();
}

void DecodeHevcPipelineAdapterXe3_Lpm_Base::Destroy()
{
    DECODE_FUNC_CALL();

    m_decoder->Destroy();
}

MOS_GPU_CONTEXT DecodeHevcPipelineAdapterXe3_Lpm_Base::GetDecodeContext()
{
    DECODE_FUNC_CALL();

    return m_decoder->GetDecodeContext();
}

GPU_CONTEXT_HANDLE DecodeHevcPipelineAdapterXe3_Lpm_Base::GetDecodeContextHandle()
{
    DECODE_FUNC_CALL();

    return m_decoder->GetDecodeContextHandle();
}
