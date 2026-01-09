/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_jpeg_pipeline_adapter_xe3p_lpm_base.cpp
//! \brief    Defines the interface to adapt to jpeg encode pipeline Xe3P_LPM_Base
//!

#include "encode_jpeg_pipeline_adapter_xe3p_lpm_base.h"
#include "encode_jpeg_pipeline_xe3p_lpm_base.h"

MOS_STATUS EncodeJpegPipelineAdapterXe3P_Lpm_Base::Allocate(CodechalSetting *codecHalSettings)
{
    ENCODE_FUNC_CALL();

    m_encoder = std::make_shared<encode::JpegPipelineXe3P_Lpm_Base>(m_hwInterface, m_debugInterface);
    ENCODE_CHK_NULL_RETURN(m_encoder);

    return m_encoder->Init(codecHalSettings);
}
