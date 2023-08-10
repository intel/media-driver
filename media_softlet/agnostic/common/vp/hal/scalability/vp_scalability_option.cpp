/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     vp_scalability_option.cpp
//! \brief    Defines the common interface for vp scalability option.
//!

#include "vp_scalability_option.h"
#include "media_scalability_defs.h"

namespace vp 
{
VpScalabilityOption::VpScalabilityOption(const VpScalabilityOption &pOption)
{
    m_numPipe = pOption.m_numPipe;
    m_raMode  = pOption.GetRAMode();
    m_protectMode = pOption.GetProtectMode();
}

MOS_STATUS VpScalabilityOption::SetScalabilityOption(ScalabilityPars *params)
{
    SCALABILITY_CHK_NULL_RETURN(params);

    if (params->enableVE)
    {
        m_numPipe = params->numVebox;
    }
    else
    {
        m_numPipe = 1;
        SCALABILITY_NORMALMESSAGE("Force numbPipe to 1 for enableVE == false case.");
    }
    m_raMode  = params->raMode;
    m_protectMode = params->protectMode;
    SCALABILITY_NORMALMESSAGE("Tile Column = %d, System VEBOX Num = %d, Decided Pipe Num = %d, raMode = %d, protectMode = %d.",
        params->numTileColumns, params->numVebox, m_numPipe, params->raMode, params->protectMode);

    return MOS_STATUS_SUCCESS;
}

bool VpScalabilityOption::IsScalabilityOptionMatched(ScalabilityPars *params)
{
    if (params == nullptr)
    {
        return false;
    }

    bool                   matched  = false;

    VpScalabilityOption newOption;
    newOption.SetScalabilityOption(params);

    if (m_numPipe != newOption.GetNumPipe() ||
        m_raMode != newOption.GetRAMode() ||
        m_protectMode != newOption.GetProtectMode())
    {
        SCALABILITY_NORMALMESSAGE("Not Matched. numPipe %d -> %d, raMode %d -> %d, protectMode %d -> %d",
            m_numPipe, newOption.GetNumPipe(), m_raMode, newOption.GetRAMode(), m_protectMode, newOption.GetProtectMode());
        matched = false;
    }
    else
    {
        SCALABILITY_NORMALMESSAGE("Matched. m_numPipe %d, m_raMode %d, m_protectMode %d", m_numPipe, m_raMode, m_protectMode);
        matched = true;
    }
    return matched;
}

uint32_t VpScalabilityOption::GetLRCACount()
{
    return m_numPipe;
}
}