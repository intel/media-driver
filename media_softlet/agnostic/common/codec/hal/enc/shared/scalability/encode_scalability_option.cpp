/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_scalability_option.cpp
//! \brief    Defines the common interface for encode scalability option.
//!

#include "encode_scalability_option.h"
#include <stdint.h>
#include "encode_scalability_defs.h"
#include "media_scalability_defs.h"
#include "mhw_vdbox.h"

namespace encode
{
EncodeScalabilityOption::EncodeScalabilityOption(const EncodeScalabilityOption &pOption)
{
    m_numPipe = pOption.m_numPipe;
    m_enabledVdenc = pOption.m_enabledVdenc;
}

MOS_STATUS EncodeScalabilityOption::SetScalabilityOption(ScalabilityPars *params)
{
    SCALABILITY_CHK_NULL_RETURN(params);

    EncodeScalabilityPars *encPars = (EncodeScalabilityPars *)params;

    if (encPars->enableVE == false)
    {
        m_numPipe = 1;
        return MOS_STATUS_SUCCESS;
    }

    m_numPipe = encPars->numVdbox;
    if (encPars->numTileColumns != m_numPipe && !encPars->allowSwArbitarySplit)
    {
        m_numPipe = 1;// switch back to the single VDBOX mode if invalid tile column test cases.
        //only set it when tile colums number less than vdbox number.
        if (encPars->numTileColumns < encPars->numVdbox && encPars->numTileColumns >= 1 && encPars->numTileColumns <= 4)
        {
            m_numPipe = static_cast<uint8_t>(encPars->numTileColumns);
        }
    }

    if (!encPars->forceMultiPipe && !encPars->enableTileReplay)
    {
        uint32_t frameWidthTh  = m_4KFrameWdithTh;
        uint32_t frameHeightTh = m_4KFrameHeightTh;
        if (encPars->outputChromaFormat == (uint8_t)HCP_CHROMA_FORMAT_YUV420)
        {
            frameWidthTh = m_5KFrameWdithTh;
            frameHeightTh = m_5KFrameWdithTh;
        }
        if (encPars->frameWidth < frameWidthTh && encPars->frameHeight <frameHeightTh)
        {
            m_numPipe = 1;
        }
    }
    SCALABILITY_VERBOSEMESSAGE("Tile Column = %d, System VDBOX Num = %d, Decided Pipe Num = %d.", encPars->numTileColumns, encPars->numVdbox, m_numPipe);

    m_enabledVdenc = encPars->enableVDEnc;
    m_raMode = encPars->raMode;
    m_protectMode = encPars->protectMode;

    return MOS_STATUS_SUCCESS;
}

bool EncodeScalabilityOption::IsScalabilityOptionMatched(ScalabilityPars *params)
{
    if (params == nullptr)
    {
        return false;
    }

    bool                   matched  = false;
    EncodeScalabilityPars *encPars = (EncodeScalabilityPars *)params;

    EncodeScalabilityOption newOption;
    newOption.SetScalabilityOption(params);

    if (m_numPipe != newOption.GetNumPipe() ||
        m_enabledVdenc != newOption.IsVdencEnabled()||
        m_raMode != newOption.GetRAMode() ||
        m_protectMode != newOption.GetProtectMode())
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
