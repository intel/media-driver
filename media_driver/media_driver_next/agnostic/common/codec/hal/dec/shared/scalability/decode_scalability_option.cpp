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
//! \file     decode_scalability_option.cpp
//! \brief    Defines the common interface for decode scalability option.
//!

#include "decode_scalability_option.h"

namespace decode
{
DecodeScalabilityOption::DecodeScalabilityOption(const DecodeScalabilityOption &pOption)
{
    m_numPipe = pOption.m_numPipe;
    m_usingSlimVdbox = pOption.m_usingSlimVdbox;
}

MOS_STATUS DecodeScalabilityOption::SetScalabilityOption(ScalabilityPars *params)
{
    SCALABILITY_CHK_NULL_RETURN(params);

    DecodeScalabilityPars *decPars = (DecodeScalabilityPars *)params;

    if (!decPars->enableVE)
    {
        m_numPipe = 1;
        return MOS_STATUS_SUCCESS;
    }

    if (decPars->disableVirtualTile && decPars->disableRealTile)
    {
        m_numPipe = 1;
        m_usingSlimVdbox = decPars->usingSlimVdbox;
        return MOS_STATUS_SUCCESS;
    }

    m_numPipe = decPars->numVdbox;
    if (decPars->numTileColumns != m_numPipe)
    {
        m_numPipe = 1;// switch back to the single VDBOX mode if invalid tile column test cases.
        //only set it when tile colums number less than vdbox number.
        if (decPars->numTileColumns < decPars->numVdbox && decPars->numTileColumns >= 1 && decPars->numTileColumns <= 4)
        {
            m_numPipe = static_cast<uint8_t>(decPars->numTileColumns);
        }
    }

    if (!decPars->forceMultiPipe)  // && !m_enableTileReplay)
    {
        uint32_t frameWidthTh  = m_4KFrameWdithTh;
        uint32_t frameHeightTh = m_4KFrameHeightTh;
        if (!IsRextFormat(decPars->surfaceFormat))
        {
            frameWidthTh = m_5KFrameWdithTh;
            frameHeightTh = m_5KFrameWdithTh;
        }
        if (decPars->frameWidth < frameWidthTh && decPars->frameHeight <frameHeightTh)
        {
            m_numPipe = 1;
        }
    }

    m_usingSFC = decPars->usingSfc;
    m_usingSlimVdbox = decPars->usingSlimVdbox;
    m_raMode = params->raMode;

    SCALABILITY_VERBOSEMESSAGE("Tile Column = %d, System VDBOX Num = %d, Decided Pipe Num = %d, Using SFC = %d, Using Slim Vdbox = %d.",
        decPars->numTileColumns, decPars->numVdbox, m_numPipe, m_usingSFC, m_usingSlimVdbox);
    return MOS_STATUS_SUCCESS;
}

bool DecodeScalabilityOption::IsScalabilityOptionMatched(ScalabilityPars *params)
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
    DecodeScalabilityPars *decPars = (DecodeScalabilityPars *)params;

    DecodeScalabilityOption newOption;
    newOption.SetScalabilityOption(params);

    if (m_numPipe  != newOption.GetNumPipe() ||
        m_usingSFC != newOption.IsUsingSFC() ||
        m_usingSlimVdbox != newOption.IsUsingSlimVdbox() ||
        m_raMode != newOption.GetRAMode())
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
