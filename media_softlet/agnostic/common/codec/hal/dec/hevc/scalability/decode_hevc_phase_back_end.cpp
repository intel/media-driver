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
//! \file     decode_hevc_phase_back_end.cpp
//! \brief    Defines the interface for Hevc decode back end phase.
//!

#include "decode_hevc_phase_back_end.h"
#include "decode_utils.h"

namespace decode
{

uint32_t HevcPhaseBackEnd::GetCmdBufIndex()
{
    DECODE_FUNC_CALL();
    DECODE_ASSERT(m_scalabOption.GetNumPipe() > 1);

    if (m_scalabOption.IsFESeparateSubmission() && !m_pipeline->IsParallelSubmission())
    {
        return m_secondaryCmdBufIdxBase + GetPipe();
    }
    else
    {
        return m_secondaryCmdBufIdxBase + GetPipe() + 1;
    }
}

uint32_t HevcPhaseBackEnd::GetSubmissionType()
{
    DECODE_FUNC_CALL();
    if (IsFirstBackEnd())
    {
        return SUBMISSION_TYPE_MULTI_PIPE_MASTER;
    }
    else if (IsLastBackEnd())
    {
        return SUBMISSION_TYPE_MULTI_PIPE_SLAVE | SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
    }
    else
    {
        return SUBMISSION_TYPE_MULTI_PIPE_SLAVE;
    }
}

MOS_STATUS HevcPhaseBackEnd::GetMode(uint32_t &pipeWorkMode, uint32_t &multiEngineMode)
{
    DECODE_FUNC_CALL();
    pipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;
    if (IsFirstBackEnd())
    {
        multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;
    }
    else if (IsLastBackEnd())
    {
        multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;
    }
    else
    {
        multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;
    }
    return MOS_STATUS_SUCCESS;
}

uint32_t HevcPhaseBackEnd::GetPktId()
{
    DECODE_FUNC_CALL();
    return DecodePacketId(m_pipeline, hevcBackEndPacketId);
}

bool HevcPhaseBackEnd::ImmediateSubmit()
{
    DECODE_FUNC_CALL();
    return (IsLastBackEnd());
}

bool HevcPhaseBackEnd::RequiresContextSwitch()
{
    DECODE_FUNC_CALL();
    if (m_scalabOption.IsFESeparateSubmission() && IsFirstBackEnd())
    {
        return true; // switch context for first back end in separate submission mode
    }
    else
    {
        return false;
    }
}

}
