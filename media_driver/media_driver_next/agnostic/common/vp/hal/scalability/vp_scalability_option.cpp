/*
* Copyright (c) 2019, Intel Corporation
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
}

MOS_STATUS VpScalabilityOption::SetScalabilityOption(ScalabilityPars *params)
{
    SCALABILITY_CHK_NULL_RETURN(params);

    if (params->enableVE == false)
    {
        m_numPipe = 1;
        return MOS_STATUS_SUCCESS;
    }

    m_numPipe = params->numVebox;
    if (params->numTileColumns != m_numPipe)
    {
        m_numPipe = 1;// switch back to the single VDBOX mode if invalid tile column test cases.
    }

    SCALABILITY_VERBOSEMESSAGE("Tile Column = %d, System VDBOX Num = %d, Decided Pipe Num = %d.", params->numTileColumns, params->numVebox, m_numPipe);

    return MOS_STATUS_SUCCESS;
}

bool VpScalabilityOption::IsScalabilityOptionMatched(ScalabilityPars *params)
{
    if (params == nullptr)
    {
        return false;
    }

    if (params->enableMdf == true)
    {
        return true;
    }

    bool                   matched  = false;

    VpScalabilityOption newOption;
    newOption.SetScalabilityOption(params);

    if (m_numPipe != newOption.GetNumPipe())
    {
        matched = false;
    }
    else
    {
        matched = true;
    }
    return matched;
}
}