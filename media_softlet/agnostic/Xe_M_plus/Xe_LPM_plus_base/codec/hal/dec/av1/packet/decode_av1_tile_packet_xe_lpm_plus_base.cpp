/*
* Copyright (c) 2020-2024, Intel Corporation
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

MOS_STATUS Av1DecodeTilePktXe_Lpm_Plus_Base::AddCmd_AVP_TILE_CODING(MOS_COMMAND_BUFFER &cmdBuffer, int16_t tileIdx)
{
    DECODE_FUNC_CALL()
    auto &par = m_avpItf->MHW_GETPAR_F(AVP_TILE_CODING)();
    par       = {};

    Av1DecodeTile::TileDesc *m_tileDesc = m_av1BasicFeature->m_tileCoding.m_tileDesc;
    uint16_t                 curCol     = m_tileDesc[tileIdx].m_tileColumn;
    uint16_t                 curRow     = m_tileDesc[tileIdx].m_tileRow;
    uint16_t                 srcTileId  = curCol + curRow * m_av1PicParams->m_tileCols;

    if (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile)
    {
        DECODE_ASSERT(tileIdx == m_tileDesc[tileIdx].m_tileIndex);
    }

    if (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile)
    {
        par.tileId                 = srcTileId;
        par.tgTileNum              = srcTileId;
        par.tileGroupId            = 0;
        par.tileColPositionInSb    = m_av1BasicFeature->m_tileCoding.m_tileColStartSb[curCol];
        par.tileRowPositionInSb    = m_av1BasicFeature->m_tileCoding.m_tileRowStartSb[curRow];
        par.tileWidthInSbMinus1    = m_av1PicParams->m_widthInSbsMinus1[curCol];
        par.tileHeightInSbMinus1   = m_av1PicParams->m_heightInSbsMinus1[curRow];
        par.tileRowIndependentFlag = true;
        par.lastTileOfColumn       = (curRow == m_av1PicParams->m_tileRows - 1) ? true : false;
        par.lastTileOfRow          = (curCol == m_av1PicParams->m_tileCols - 1) ? true : false;
        par.firstTileOfTileGroup   = (srcTileId == 0) ? true : false;
        par.lastTileOfTileGroup    = (curCol == m_av1PicParams->m_tileCols - 1) && (curRow == m_av1PicParams->m_tileRows - 1);
        par.lastTileOfFrame        = (curCol == m_av1PicParams->m_tileCols - 1) && (curRow == m_av1PicParams->m_tileRows - 1);
    }
    else
    {
        par.tileId                 = tileIdx;
        par.tgTileNum              = m_tileDesc[tileIdx].m_tileNum;
        par.tileGroupId            = m_tileDesc[tileIdx].m_tileGroupId;
        par.tileColPositionInSb    = m_av1BasicFeature->m_tileCoding.m_tileColStartSb[curCol];
        par.tileRowPositionInSb    = m_av1BasicFeature->m_tileCoding.m_tileRowStartSb[curRow];
        par.tileWidthInSbMinus1    = m_av1PicParams->m_widthInSbsMinus1[curCol];
        par.tileHeightInSbMinus1   = m_av1PicParams->m_heightInSbsMinus1[curRow];
        par.tileRowIndependentFlag = true;
        par.lastTileOfColumn       = (curRow == m_av1PicParams->m_tileRows - 1) ? true : false;
        par.lastTileOfRow          = (curCol == m_av1PicParams->m_tileCols - 1) ? true : false;
        par.firstTileOfTileGroup   = (m_tileDesc[tileIdx].m_tileNum == 0) ? true : false;
        par.lastTileOfTileGroup    = m_tileDesc[tileIdx].m_lastInGroup;
        par.lastTileOfFrame        = (curCol == m_av1PicParams->m_tileCols - 1) && (curRow == m_av1PicParams->m_tileRows - 1);
    }

    par.disableCdfUpdateFlag          = m_av1PicParams->m_picInfoFlags.m_fields.m_disableCdfUpdate;
    par.disableFrameContextUpdateFlag = m_av1PicParams->m_picInfoFlags.m_fields.m_disableFrameEndUpdateCdf || (tileIdx != m_av1PicParams->m_contextUpdateTileId);
    par.numOfActiveBePipes            = 1;

    if (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile)
    {
        par.numOfTileColumnsInFrame = m_av1PicParams->m_outputFrameWidthInTilesMinus1 + 1;
        par.numOfTileRowsInFrame    = m_av1PicParams->m_outputFrameHeightInTilesMinus1 + 1;
        par.outputDecodedTileColPos = (m_tileDesc[tileIdx].m_tileIndex % (m_av1PicParams->m_outputFrameWidthInTilesMinus1 + 1)) *
                                      (m_av1PicParams->m_widthInSbsMinus1[0] + 1);                                                //AV1 Conformance: tile width is identical for all tiles
        par.outputDecodedTileRowPos = (m_tileDesc[tileIdx].m_tileIndex / (m_av1PicParams->m_outputFrameWidthInTilesMinus1 + 1));  //tile height is exactly one SB
    }
    else
    {
        par.numOfTileColumnsInFrame = m_av1PicParams->m_tileCols;
        par.numOfTileRowsInFrame    = m_av1PicParams->m_tileRows;
    }

    m_av1BasicFeature->m_frameCompletedFlag = par.lastTileOfFrame;

#if (_DEBUG || _RELEASE_INTERNAL)
    auto debugInterface = m_av1Pipeline->GetDebugInterface();
    DECODE_CHK_NULL(debugInterface);
    if (debugInterface->IsHwDebugHooksEnable())
    {
        par.enableAvpDebugMode = true;
    }
#endif

    DECODE_CHK_STATUS(m_avpItf->MHW_ADDCMD_F(AVP_TILE_CODING)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

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
