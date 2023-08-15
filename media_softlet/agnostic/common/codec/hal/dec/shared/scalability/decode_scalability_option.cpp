/*
* Copyright (c) 2019-2022, Intel Corporation
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
#include "media_scalability_defs.h"

namespace decode
{
DecodeScalabilityOption::DecodeScalabilityOption(const DecodeScalabilityOption &option)
{
    m_numPipe               = option.m_numPipe;
    m_mode                  = option.m_mode;
    m_usingSFC              = option.m_usingSFC;
    m_usingSlimVdbox        = option.m_usingSlimVdbox;
    m_FESeparateSubmission  = option.m_FESeparateSubmission;
    m_raMode                = option.m_raMode;
    m_protectMode           = option.m_protectMode;
}

MOS_STATUS DecodeScalabilityOption::SetScalabilityOption(ScalabilityPars *params)
{
    SCALABILITY_CHK_NULL_RETURN(params);

    DecodeScalabilityPars *decPars = (DecodeScalabilityPars *)params;

    m_numPipe        = 1;
    m_mode           = scalabilitySingleMode;
    m_usingSFC       = decPars->usingSfc;
    m_usingSlimVdbox = decPars->usingSlimVdbox;
    m_raMode         = decPars->raMode;
    m_protectMode    = decPars->protectMode;

    if (IsSinglePipeDecode(*decPars))
    {
        return MOS_STATUS_SUCCESS;
    }

    bool isRealTileDecode   = IsRealTileDecode(*decPars);
    if (!isRealTileDecode && decPars->disableVirtualTile)
    {
        //If Not Real Tile Decode and Virtual tile was disabled, use single pipe.
        return MOS_STATUS_SUCCESS;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (decPars->forceMultiPipe)
    {
        m_numPipe = GetUserPipeNum(decPars->numVdbox, decPars->userPipeNum);
    }
    else if (decPars->modeSwithThreshold2 > 0 && decPars->frameWidth >= decPars->modeSwithThreshold2)
    {
        m_numPipe = (decPars->numVdbox >= m_maxNumMultiPipe) ? m_maxNumMultiPipe : m_typicalNumMultiPipe;
    }
    else if (decPars->modeSwithThreshold1 > 0 && decPars->frameWidth >= decPars->modeSwithThreshold1)
    {
        m_numPipe = m_typicalNumMultiPipe;
    }
    else
#endif
    if (!decPars->disableVirtualTile && IsResolutionMatchMultiPipeThreshold2(decPars->frameWidth, decPars->frameHeight))
    {
        m_numPipe = (decPars->numVdbox >= m_maxNumMultiPipe) ? m_maxNumMultiPipe : m_typicalNumMultiPipe;
    }
    else if (isRealTileDecode ||
             (!decPars->disableVirtualTile &&
              IsResolutionMatchMultiPipeThreshold1(decPars->frameWidth, decPars->frameHeight, decPars->surfaceFormat)))
    {
        m_numPipe = m_typicalNumMultiPipe;
    }

    if (m_numPipe >= m_typicalNumMultiPipe)
    {
        m_mode = isRealTileDecode ? scalabilityRealTileMode : scalabilityVirtualTileMode;
    }

    if (m_mode == scalabilityVirtualTileMode && decPars->numVdbox >= m_maxNumMultiPipe)
    {
        m_FESeparateSubmission = true;
    }
    else
    {
        m_FESeparateSubmission = false;
    }

    SCALABILITY_VERBOSEMESSAGE(
        "Tile Column = %d, System VDBOX Num = %d, Decided Pipe Num = %d, "
        "Using SFC = %d, Using Slim Vdbox = %d, Scalability Mode = %d, FE separate submission = %d.",
        decPars->numTileColumns, decPars->numVdbox, m_numPipe,
        m_usingSFC, m_usingSlimVdbox, m_mode, m_FESeparateSubmission);
    return MOS_STATUS_SUCCESS;
}

bool DecodeScalabilityOption::IsScalabilityOptionMatched(ScalabilityPars *params)
{
    if (params == nullptr)
    {
        return false;
    }

    bool                   matched  = false;
    DecodeScalabilityPars *decPars = (DecodeScalabilityPars *)params;

    DecodeScalabilityOption newOption;
    if(MOS_STATUS_SUCCESS != newOption.SetScalabilityOption(params))
    {
        return false;
    }

    if (m_numPipe              != newOption.GetNumPipe()             ||
        m_usingSFC             != newOption.IsUsingSFC()             ||
        m_usingSlimVdbox       != newOption.IsUsingSlimVdbox()       ||
        m_mode                 != newOption.GetMode()                ||
        m_FESeparateSubmission != newOption.IsFESeparateSubmission() ||
        m_raMode               != newOption.GetRAMode()              ||
        m_protectMode          != newOption.GetProtectMode())
    {
        matched = false;
    }
    else
    {
        matched = true;
    }
    return matched;
}

bool DecodeScalabilityOption::IsScalabilityOptionMatched(MediaScalabilityOption &scalabOption)
{
    auto decodeOption = dynamic_cast<DecodeScalabilityOption *>(&scalabOption);
    if (decodeOption == nullptr)
    {
        return false;
    }

    if (m_numPipe              != decodeOption->GetNumPipe()             ||
        m_usingSFC             != decodeOption->IsUsingSFC()             ||
        m_usingSlimVdbox       != decodeOption->IsUsingSlimVdbox()       ||
        m_mode                 != decodeOption->GetMode()                ||
        m_FESeparateSubmission != decodeOption->IsFESeparateSubmission() ||
        m_raMode               != decodeOption->GetRAMode()              ||
        m_protectMode          != decodeOption->GetProtectMode())
    {
        return false;
    }

    return true;
}

bool DecodeScalabilityOption::IsSinglePipeDecode(DecodeScalabilityPars &params)
{
    // multipipe depends on VE
    if (!params.enableVE)
    {
        return true;
    }

    if (params.disableScalability)
    {
        return true;
    }

    if (params.numVdbox <= 1)
    {
        return true;
    }

    return false;
}

bool DecodeScalabilityOption::IsRealTileDecode(DecodeScalabilityPars &params)
{
    if (params.disableRealTile)
    {
        return false;
    }

    if (params.numTileColumns <= 1)
    {
        return false;
    }

    return (params.numTileColumns <= params.maxTileColumn &&
            params.numTileRows <= params.maxTileRow);
}

bool DecodeScalabilityOption::IsResolutionMatchMultiPipeThreshold1(
    uint32_t frameWidth, uint32_t frameHeight, MOS_FORMAT surfaceFormat)
{
    uint32_t frameWidthTh  = m_4KFrameWdithTh;
    uint32_t frameHeightTh = m_4KFrameHeightTh;
    if (!IsRextFormat(surfaceFormat))
    {
        frameWidthTh = m_5KFrameWdithTh;
        frameHeightTh = m_5KFrameHeightTh;
    }

    return (frameWidth >= frameWidthTh && frameHeight >= frameHeightTh) ||
           (frameWidth * frameHeight >= frameWidthTh * frameHeightTh);
}

bool DecodeScalabilityOption::IsResolutionMatchMultiPipeThreshold2(
    uint32_t frameWidth, uint32_t frameHeight)
{
    return ((frameWidth * frameHeight) >= (m_8KFrameWdithTh * m_8KFrameHeightTh));
}

#if (_DEBUG || _RELEASE_INTERNAL)
uint8_t DecodeScalabilityOption::GetUserPipeNum(uint8_t numVdbox, uint8_t userPipeNum)
{
    if (numVdbox >= m_maxNumMultiPipe)
    {
        if (userPipeNum >= m_typicalNumMultiPipe && userPipeNum <= numVdbox)
        {
            return userPipeNum;
        }
    }
    return m_typicalNumMultiPipe;
}
#endif

uint32_t DecodeScalabilityOption::GetLRCACount()
{
    // on GT2 or debug override enabled, FE separate submission = false, FE run on the same engine of BEs;
    // on GT3, FE separate submission = true, scalability submission includes only BEs.
    if (m_numPipe == m_typicalNumMultiPipe)
    {
        return m_numPipe;
    }
    else if (m_numPipe >= m_maxNumMultiPipe)
    {
        // in release driver bFESeparateSubmission is always false since this is on GT3 or GT4.
        // bFESeparateSubmission could be false if debug override enabled.
        if (m_FESeparateSubmission || m_mode == scalabilityRealTileMode)
        {
            return m_numPipe;
        }
        else
        {
            return m_numPipe + 1;
        }
    }

    return m_numPipe;
}

}
