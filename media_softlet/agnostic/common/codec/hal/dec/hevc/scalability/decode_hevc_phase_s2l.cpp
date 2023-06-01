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
//! \file     decode_hevc_phase_s2l.cpp
//! \brief    Defines the interface for Hevc decode s2l phase.
//!

#include "decode_hevc_phase_s2l.h"
#include "decode_utils.h"

namespace decode
{

uint32_t HevcPhaseS2L::GetCmdBufIndex()
{
    DECODE_FUNC_CALL();
    if (m_scalabOption.GetNumPipe() == 1)
    {
        return m_primaryCmdBufIdx;
    }

    //It is not allowed for guc submission by adding cmd in primaryCmdBuf
    if (m_scalabOption.IsFESeparateSubmission() && !m_pipeline->IsParallelSubmission())
    {
        return m_primaryCmdBufIdx;
    }
    else
    {
        return m_secondaryCmdBufIdxBase;
    }
}

uint32_t HevcPhaseS2L::GetSubmissionType()
{
    DECODE_FUNC_CALL();
    if (m_scalabOption.GetNumPipe() == 1)
    {
        return SUBMISSION_TYPE_SINGLE_PIPE;
    }
    else
    {
        return SUBMISSION_TYPE_MULTI_PIPE_ALONE;
    }
}

MOS_STATUS HevcPhaseS2L::GetMode(uint32_t &pipeWorkMode, uint32_t &multiEngineMode)
{
    DECODE_FUNC_CALL();
    pipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
    return MOS_STATUS_SUCCESS;
}

uint32_t HevcPhaseS2L::GetPktId()
{
    DECODE_FUNC_CALL();
    return DecodePacketId(m_pipeline, hucS2lPacketId);
}

bool HevcPhaseS2L::ImmediateSubmit()
{
    DECODE_FUNC_CALL();
    if ((m_scalabOption.GetNumPipe() > 1) && m_pipeline->IsParallelSubmission())
    {
        return false;
    }
    else
    {
        return !m_pipeline->IsSingleTaskPhaseSupported();
    }
}

bool HevcPhaseS2L::RequiresContextSwitch()
{
    DECODE_FUNC_CALL();
    // Switch context for first phase
    return true;
}

DecodeScalabilityOption* HevcPhaseS2L::GetDecodeScalabilityOption()
{
    DECODE_FUNC_CALL();

    if (m_scalabOption.IsFESeparateSubmission())
    {
        // Create single pipe scalability for front end separate submission
        HevcScalabilityPars scalPars;
        MOS_ZeroMemory(&scalPars, sizeof(scalPars));
        if (m_FEScalabOption.SetScalabilityOption(&scalPars) != MOS_STATUS_SUCCESS)
        {
            return nullptr;
        }

        return &m_FEScalabOption;
    }

    return HevcPhase::GetDecodeScalabilityOption();
}

}
