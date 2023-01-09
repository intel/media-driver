/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_avc_vdenc_pipeline_adapter.cpp
//! \brief    Defines the interface to adapt to avc vdenc encode pipeline
//!

#include "encode_avc_vdenc_pipeline_adapter.h"

EncodeAvcVdencPipelineAdapter::EncodeAvcVdencPipelineAdapter(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : EncoderPipelineAdapter(hwInterface, debugInterface)
{
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    m_osInterface->pfnVirtualEngineSupported(m_osInterface, false, true);
    Mos_SetVirtualEngineSupported(m_osInterface, true);

    m_vdencEnabled = true;
}

MOS_STATUS EncodeAvcVdencPipelineAdapter::Execute(void    *params)
{
    ENCODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_HAL);

    ENCODE_CHK_STATUS_RETURN(m_encoder->Prepare(params));
    return m_encoder->Execute();
}

MOS_STATUS EncodeAvcVdencPipelineAdapter::GetStatusReport(
    void                *status,
    uint16_t            numStatus)
{
    ENCODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_HAL);

    return m_encoder->GetStatusReport(status, numStatus);
}

void EncodeAvcVdencPipelineAdapter::Destroy()
{
    ENCODE_FUNC_CALL();

    m_encoder->Destroy();
}