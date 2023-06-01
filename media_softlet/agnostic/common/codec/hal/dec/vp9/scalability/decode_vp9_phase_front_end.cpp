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
//! \file     decode_vp9_phase_front_end.cpp
//! \brief    Defines the interface for Vp9 decode front end phase.
//!

#include "decode_vp9_phase_front_end.h"
#include "decode_utils.h"

namespace decode
{

uint32_t Vp9PhaseFrontEnd::GetCmdBufIndex()
{
    DECODE_FUNC_CALL();
    DECODE_ASSERT(m_scalabOption.GetNumPipe() > 1);

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

uint32_t Vp9PhaseFrontEnd::GetSubmissionType()
{
    DECODE_FUNC_CALL();
    return SUBMISSION_TYPE_MULTI_PIPE_ALONE;
}

MOS_STATUS Vp9PhaseFrontEnd::GetMode(uint32_t &pipeWorkMode, uint32_t &multiEngineMode)
{
    DECODE_FUNC_CALL();
    pipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_CABAC_FE;
    multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
    return MOS_STATUS_SUCCESS;
}

uint32_t Vp9PhaseFrontEnd::GetPktId()
{
    DECODE_FUNC_CALL();
    return DecodePacketId(m_pipeline, vp9FrontEndPacketId);
}

bool Vp9PhaseFrontEnd::ImmediateSubmit()
{
    DECODE_FUNC_CALL();
    return (m_scalabOption.IsFESeparateSubmission() && !m_pipeline->IsParallelSubmission());
}

bool Vp9PhaseFrontEnd::RequiresContextSwitch()
{
    DECODE_FUNC_CALL();
    return true; // switch context for first phase
}

DecodeScalabilityOption* Vp9PhaseFrontEnd::GetDecodeScalabilityOption()
{
    DECODE_FUNC_CALL();

    if (m_scalabOption.IsFESeparateSubmission())
    {
        // Create single pipe scalability for front end separate submission
        DecodeScalabilityPars scalPars;
        MOS_ZeroMemory(&scalPars, sizeof(scalPars));
        if (m_FEScalabOption.SetScalabilityOption(&scalPars) != MOS_STATUS_SUCCESS)
        {
            return nullptr;
        }

        return &m_FEScalabOption;
    }

    return Vp9Phase::GetDecodeScalabilityOption();
}

}
