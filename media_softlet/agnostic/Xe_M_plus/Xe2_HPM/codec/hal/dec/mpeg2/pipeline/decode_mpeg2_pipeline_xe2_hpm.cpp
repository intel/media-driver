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
//! \file     decode_mpeg2_pipeline_xe2_hpm.cpp
//! \brief    Defines the interface for mpeg2 decode pipeline
//!
#include "decode_mpeg2_pipeline_xe2_hpm.h"
#include "decode_mpeg2_mem_compression_xe2_hpm.h"

namespace decode
{

Mpeg2PipelineXe2_Hpm::Mpeg2PipelineXe2_Hpm(
    CodechalHwInterfaceNext *hwInterface,
    CodechalDebugInterface  *debugInterface)
    : Mpeg2PipelineXe_Lpm_Plus_Base(hwInterface, debugInterface)
{
}

#ifdef _MMC_SUPPORTED
MOS_STATUS Mpeg2PipelineXe2_Hpm::InitMmcState()
{
    DECODE_FUNC_CALL();

    m_mmcState = MOS_New(Mpeg2DecodeMemCompXe2_Hpm, m_hwInterface);
    DECODE_CHK_NULL(m_mmcState);
    DECODE_CHK_STATUS(m_basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));

    return MOS_STATUS_SUCCESS;
}
#endif

}
