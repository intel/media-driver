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
//! \file     decode_hevc_phase_long.cpp
//! \brief    Defines the interface for Hevc decode long phase.
//!

#include "decode_hevc_phase_long.h"
#include "decode_utils.h"

namespace decode
{

uint32_t HevcPhaseLong::GetCmdBufIndex()
{
    DECODE_FUNC_CALL();
    return m_primaryCmdBufIdx;
}

uint32_t HevcPhaseLong::GetSubmissionType()
{
    DECODE_FUNC_CALL();
    return SUBMISSION_TYPE_SINGLE_PIPE;
}

MOS_STATUS HevcPhaseLong::GetMode(uint32_t &pipeWorkMode, uint32_t &multiEngineMode)
{
    DECODE_FUNC_CALL();
    pipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
    return MOS_STATUS_SUCCESS;
}

uint32_t HevcPhaseLong::GetPktId()
{
    DECODE_FUNC_CALL();
    return DecodePacketId(m_pipeline, hevcLongPacketId);
}

bool HevcPhaseLong::ImmediateSubmit()
{
    DECODE_FUNC_CALL();
    return true;
}

bool HevcPhaseLong::RequiresContextSwitch()
{
    DECODE_FUNC_CALL();
    if (m_pipeline->IsShortFormat())
    {
        return false; // same context as s2l
    }
    else
    {
        return true; // switch context for first phase
    }
}

}
