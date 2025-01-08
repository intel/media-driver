/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_av1_tile_packet_xe3_lpm_base.cpp
//! \brief    Defines the interface for av1 decode tile packet for Xe3_LPM+
//!
#include "decode_av1_tile_packet_xe3_lpm_base.h"

namespace decode
{

MOS_STATUS Av1DecodeTilePktXe3_Lpm_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer, int16_t tileIdx)
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

MOS_STATUS Av1DecodeTilePktXe3_Lpm_Base::CalculateTileStateCommandSize()
{
    DECODE_FUNC_CALL();

    // Tile Level Commands
    DECODE_CHK_STATUS(GetAvpPrimitiveCommandSize(m_av1BasicFeature->m_mode, &m_tileStatesSize, &m_tilePatchListSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodeTilePktXe3_Lpm_Base::GetAvpPrimitiveCmdSize(uint32_t *commandsSize,
    uint32_t                       *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(commandsSize);
    DECODE_CHK_NULL(patchListSize);

    uint32_t maxSize          = 0;
    uint32_t patchListMaxSize = 0;

    maxSize =
        m_avpItf->GETSIZE_AVP_TILE_CODING() +
        m_avpItf->GETSIZE_AVP_BSD_OBJECT() +
        m_miItf->GETSIZE_MI_BATCH_BUFFER_END();

    patchListMaxSize =
        PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_TILE_CODING_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_BSD_OBJECT_CMD);

    *commandsSize  = maxSize;
    *patchListSize = patchListMaxSize;

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS Av1DecodeTilePktXe3_Lpm_Base::GetAvpPrimitiveCommandSize(uint32_t mode, uint32_t *commandsSize, uint32_t *patchListSize)
{
    DECODE_FUNC_CALL();
    //calculate AVP related commands size
    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

    uint32_t avpCommandsSize  = 0;
    uint32_t avpPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    CODEC_HW_CHK_STATUS_RETURN(GetAvpPrimitiveCmdSize(
        (uint32_t *)&avpCommandsSize,
        (uint32_t *)&avpPatchListSize,
        &stateCmdSizeParams));

    if (m_hwInterface->GetCpInterface())
    {
        m_hwInterface->GetCpInterface()->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    //Calc final command size
    *commandsSize  = avpCommandsSize + cpCmdsize;
    *patchListSize = avpPatchListSize + cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

}
