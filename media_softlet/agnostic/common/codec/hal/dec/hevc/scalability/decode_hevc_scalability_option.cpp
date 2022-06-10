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
//! \file     decode_hevc_scalability_option.cpp
//! \brief    Defines the interface for Hevc decode scalability option.
//!

#include "decode_hevc_scalability_option.h"
#include "decode_hevc_scalability_defs.h"
#include "decode_scalability_defs.h"
#include "media_scalability_defs.h"

namespace decode
{

DecodeHevcScalabilityOption::DecodeHevcScalabilityOption(const DecodeHevcScalabilityOption &option)
    : DecodeScalabilityOption(option)
{
}

MOS_STATUS DecodeHevcScalabilityOption::SetScalabilityOption(ScalabilityPars *params)
{
    SCALABILITY_CHK_NULL_RETURN(params);
    SCALABILITY_CHK_STATUS_RETURN(DecodeScalabilityOption::SetScalabilityOption(params));

    HevcScalabilityPars *hevcPars = (HevcScalabilityPars *)params;
    // SCC is not able to be supported with virtual tile, fall back to single pipe
    if (hevcPars->isSCC && m_mode == scalabilityVirtualTileMode)
    {
        m_numPipe = 1;
        m_FESeparateSubmission = false;
        m_mode = scalabilitySingleMode;
    }

    SCALABILITY_VERBOSEMESSAGE(
        "Tile Column = %d, System VDBOX Num = %d, Decided Pipe Num = %d, "
        "Using SFC = %d, Using Slim Vdbox = %d, Scalability Mode = %d, , FE separate submission = %d.",
        hevcPars->numTileColumns, hevcPars->numVdbox, m_numPipe,
        m_usingSFC, m_usingSlimVdbox, m_mode, m_FESeparateSubmission);

    return MOS_STATUS_SUCCESS;
}

}
