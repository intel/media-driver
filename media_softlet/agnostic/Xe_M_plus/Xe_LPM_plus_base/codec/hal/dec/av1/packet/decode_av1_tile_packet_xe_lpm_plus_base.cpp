/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_av1_tile_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for av1 decode tile packet for Xe_LPM_plus+
//!
#include "decode_av1_tile_packet_xe_lpm_plus_base.h"

namespace decode
{

MOS_STATUS Av1DecodeTilePktXe_Lpm_Plus_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer, int16_t tileIdx)
{
    DECODE_FUNC_CALL()

    SETPAR_AND_ADDCMD(AVP_INLOOP_FILTER_STATE, m_avpItf, &cmdBuffer);

    if (m_av1PicParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain)
    {
        DECODE_VERBOSEMESSAGE("Film grain is enabled.");
        SETPAR_AND_ADDCMD(AVP_FILM_GRAIN_STATE, m_avpItf, &cmdBuffer);
    }

    DECODE_CHK_STATUS(AddCmd_AVP_TILE_CODING(cmdBuffer, tileIdx));
    DECODE_CHK_STATUS(AddCmd_AVP_BSD_OBJECT(cmdBuffer, tileIdx));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodeTilePktXe_Lpm_Plus_Base::CalculateTileStateCommandSize()
{
    DECODE_FUNC_CALL();

    // Tile Level Commands
    DECODE_CHK_STATUS(m_hwInterface->GetAvpPrimitiveCommandSize(m_av1BasicFeature->m_mode, &m_tileStatesSize, &m_tilePatchListSize));

    return MOS_STATUS_SUCCESS;
}
}
