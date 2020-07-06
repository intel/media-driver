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
//! \file     decode_av1_tile_packet.cpp
//! \brief    Defines the interface for av1 decode tile packet
//!
#include "codechal_utilities.h"
#include "decode_av1_tile_packet.h"

namespace decode
{
    MOS_STATUS Av1DecodeTilePkt::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miInterface);
        DECODE_CHK_NULL(m_av1Pipeline);
        DECODE_CHK_NULL(m_avpInterface);

        m_av1BasicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_av1BasicFeature);

        m_allocator = m_pipeline ->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        DECODE_CHK_STATUS(CalculateTileStateCommandSize());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::Prepare()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_av1BasicFeature->m_av1PicParams);
        DECODE_CHK_NULL(m_av1BasicFeature->m_av1TileParams);

        m_av1PicParams = m_av1BasicFeature->m_av1PicParams;
        m_av1TileParams = m_av1BasicFeature->m_av1TileParams;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::SetAvpTileCodingParams(
        MhwVdboxAvpTileCodingParams &tileCodingParams,
        int16_t  tileIdx)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&tileCodingParams, sizeof(tileCodingParams));
        Av1DecodeTile::TileDesc *m_tileDesc=m_av1BasicFeature->m_tileCoding.m_tileDesc;
        uint16_t curCol = m_tileDesc[tileIdx].m_tileColumn;
        uint16_t curRow = m_tileDesc[tileIdx].m_tileRow;
        uint16_t srcTileId = curCol + curRow * m_av1PicParams->m_tileCols;

        tileCodingParams.m_tileId                           =  tileIdx;
        tileCodingParams.m_tileNum                          = m_tileDesc[tileIdx].m_tileNum;
        tileCodingParams.m_tileGroupId                      = m_tileDesc[tileIdx].m_tileGroupId;
        tileCodingParams.m_tileColPositionInSb              = m_av1BasicFeature->m_tileCoding.m_tileColStartSb[curCol];
        tileCodingParams.m_tileRowPositionInSb              = m_av1BasicFeature->m_tileCoding.m_tileRowStartSb[curRow];
        tileCodingParams.m_tileWidthInSbMinus1              = m_av1PicParams->m_widthInSbsMinus1[curCol];
        tileCodingParams.m_tileHeightInSbMinus1             = m_av1PicParams->m_heightInSbsMinus1[curRow];
        tileCodingParams.m_tileRowIndependentFlag           = true;
        tileCodingParams.m_isLastTileOfColumn               = (curRow == m_av1PicParams->m_tileRows - 1) ? true : false;
        tileCodingParams.m_isLastTileOfRow                  = (curCol == m_av1PicParams->m_tileCols - 1) ? true : false;
        tileCodingParams.m_isFirstTileOfTileGroup           = (m_tileDesc[tileIdx].m_tileNum == 0) ? true : false;
        tileCodingParams.m_isLastTileOfTileGroup            = m_tileDesc[tileIdx].m_lastInGroup;
        tileCodingParams.m_isLastTileOfFrame                = (curCol == m_av1PicParams->m_tileCols - 1) && (curRow == m_av1PicParams->m_tileRows - 1);
        tileCodingParams.m_disableCdfUpdateFlag             = m_av1PicParams->m_picInfoFlags.m_fields.m_disableCdfUpdate;
        tileCodingParams.m_disableFrameContextUpdateFlag    = m_av1PicParams->m_picInfoFlags.m_fields.m_disableFrameEndUpdateCdf || (tileIdx != m_av1PicParams->m_contextUpdateTileId);
        tileCodingParams.m_numOfActiveBePipes               = 1;
        tileCodingParams.m_numOfTileColumnsInFrame          = m_av1PicParams->m_tileCols;
        tileCodingParams.m_numOfTileRowsInFrame             = m_av1PicParams->m_tileRows;

        m_av1BasicFeature->frameCompletedFlag = tileCodingParams.m_isLastTileOfFrame;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::SetBsdObjParams(
        MhwVdboxAvpBsdParams &bsdObjParams,
        int16_t                  tileIdx)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&bsdObjParams ,sizeof(bsdObjParams));
        Av1DecodeTile::TileDesc *m_tileDesc=m_av1BasicFeature->m_tileCoding.m_tileDesc;
        bsdObjParams.m_bsdDataLength = m_tileDesc[tileIdx].m_size;
        bsdObjParams.m_bsdDataStartOffset =m_tileDesc[tileIdx].m_offset;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::AddBsdObj(
        MOS_COMMAND_BUFFER &cmdBuffer,
        int16_t            tileIdx)
    {
        DECODE_FUNC_CALL();

        MhwVdboxAvpBsdParams bsdObjParams;
        DECODE_CHK_STATUS(SetBsdObjParams(bsdObjParams, tileIdx));
        DECODE_CHK_STATUS(m_avpInterface->AddAvpBsdObjectCmd(&cmdBuffer, nullptr, &bsdObjParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::CalculateCommandSize(uint32_t &commandBufferSize,
                                                      uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = m_tileStatesSize;
        requestedPatchListSize = m_tilePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::CalculateTileStateCommandSize()
    {
        DECODE_FUNC_CALL();

        // Tile Level Commands
        DECODE_CHK_STATUS(static_cast<CodechalHwInterfaceG12*>(m_hwInterface)->GetAvpPrimitiveCommandSize(
                                                                    m_av1BasicFeature->m_mode,
                                                                    &m_tileStatesSize,
                                                                    &m_tilePatchListSize));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTilePkt::AddAvpTileState(
        MOS_COMMAND_BUFFER &cmdBuffer,
        int16_t           tileIdx)
    {
        DECODE_FUNC_CALL();

        MhwVdboxAvpTileCodingParams tileCodingParams;
        DECODE_CHK_STATUS(SetAvpTileCodingParams(tileCodingParams, tileIdx));
        DECODE_CHK_STATUS(m_avpInterface->AddAvpTileCodingCmd(&cmdBuffer, nullptr, &tileCodingParams));

        return MOS_STATUS_SUCCESS;
    }
}
