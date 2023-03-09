/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     decode_av1_tile_coding.cpp
//! \brief    Defines the common interface for av1 decode tile coding
//!

#include "decode_av1_tile_coding.h"
#include "decode_av1_basic_feature.h"
#include "codec_def_common.h"
#include "decode_pipeline.h"

namespace decode
{
    Av1DecodeTile::~Av1DecodeTile()
    {
        // tile descriptors
        if (m_tileDesc)
        {
            free(m_tileDesc);
            m_tileDesc = nullptr;
        }
    }

    MOS_STATUS Av1DecodeTile::Init(Av1BasicFeature *basicFeature, CodechalSetting *codecSettings)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(basicFeature);
        DECODE_CHK_NULL(codecSettings);

        m_basicFeature = basicFeature;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTile::Update(CodecAv1PicParams & picParams,
                                        CodecAv1TileParams *tileParams)
    {
        DECODE_FUNC_CALL();

        // Initialize state params for multiple tiles
        if (m_newFrameStart)
        {
            m_lastTileId        = -1;
            m_curTile           = -1;
            m_tileStartOffset   = 0;
            m_firstTileInTg     = 0;
            m_tileGroupId       = -1;
            m_isTruncatedTile   = false;
            m_decPassNum        = 1;
            m_hasTileMissing    = false;
            m_hasDuplicateTile  = false;
        }

        if (m_numTiles > av1MaxTileNum)
        {
            DECODE_ASSERTMESSAGE("Tile number exceeds the max supported number %d!", av1MaxTileNum);
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (picParams.m_tileCols > av1MaxTileColumn || picParams.m_tileRows > av1MaxTileRow)
        {
            DECODE_NORMALMESSAGE("Invalid tile row/col.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        uint16_t tileNumLimit = (picParams.m_picInfoFlags.m_fields.m_largeScaleTile) ? av1MaxTileNum : (picParams.m_tileCols * picParams.m_tileRows);
        if (nullptr != m_tileDesc)
        {
            if (m_prevFrmTileNum < tileNumLimit)
            {
                free(m_tileDesc);
                m_tileDesc = nullptr;
            }
            else
            {
                memset(m_tileDesc, 0, (sizeof(TileDesc) * m_prevFrmTileNum));
            }
        }
        if (nullptr == m_tileDesc)
        {
            m_tileDesc = (TileDesc *)malloc(sizeof(TileDesc) * tileNumLimit);
            if (nullptr != m_tileDesc)
            {
                memset(m_tileDesc, 0, (sizeof(TileDesc) * tileNumLimit));
            }
        }
        m_prevFrmTileNum = tileNumLimit;

        //Calculate tile info for max tile
        DECODE_CHK_STATUS(CalcTileInfoMaxTile(picParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTile::ErrorDetectAndConceal()
    {
        DECODE_FUNC_CALL()
        DECODE_CHK_NULL(m_tileDesc);
        uint64_t datasize = 0;

        // detect if any tile missing
        if (m_numTiles < m_totalTileNum)
        {
            if (!m_hasTileMissing)
            {
                m_hasTileMissing = true;
            }
        }

        // make sure the last tile equals to m_totalTileNum-1
        if (m_hasTileMissing)
        {
            if (m_lastTileId != m_totalTileNum - 1)
            {
                m_lastTileId    = m_totalTileNum - 1;
                m_newFrameStart = true;
            }
        }

        // Error Concealment for Tile size
        // m_numTiles means the tile number from application
        // m_totalTileNum means the total number of tile, m_lastTileId means the last tile index
        for (uint32_t i = 0; i < m_totalTileNum; i++)
        {
            // m_tileDesc[i].m_size + m_tileDesc[i].m_offset could oversize the maximum of uint32_t
            datasize = (uint64_t)m_tileDesc[i].m_size + (uint64_t)m_tileDesc[i].m_offset;
            if (datasize > m_basicFeature->m_dataSize)
            {
                if (i == m_lastTileId)
                {
                    if (m_basicFeature->m_dataSize > m_tileDesc[i].m_offset)
                    {
                        m_tileDesc[i].m_size = m_basicFeature->m_dataSize - m_tileDesc[i].m_offset;
                        DECODE_ASSERTMESSAGE("The last tile size is oversize, the remaining size is %d\n", m_tileDesc[i].m_size);
                    }
                    else
                    {
                        m_tileDesc[i].m_size = 0;
                        DECODE_ASSERTMESSAGE("The last tile size is invalid, take current tile as missing tile and then set 4 byte dummy WL!!");
                    }
                }
                else
                {
                    m_tileDesc[i].m_size = 0;
                    DECODE_ASSERTMESSAGE("The non-last tile size is oversize, take current tile as missing tile and then set 4 byte dummy WL!\n");
                }
            }
            // For tile missing scenario
            if (m_tileDesc[i].m_size == 0)
            {
                DECODE_ASSERTMESSAGE("The %d tile is missing, set 4 byte dummy WL!\n", i);
                m_tileDesc[i].m_size       = 4;
                m_tileDesc[i].m_offset     = 0;
                m_tileDesc[i].m_tileRow    = i / m_basicFeature->m_av1PicParams->m_tileCols;
                m_tileDesc[i].m_tileColumn = i % m_basicFeature->m_av1PicParams->m_tileCols;
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTile::ParseTileInfo(const CodecAv1PicParams & picParams, CodecAv1TileParams *tileParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_tileDesc);
        m_totalTileNum      = (picParams.m_picInfoFlags.m_fields.m_largeScaleTile) ?
            (picParams.m_tileCountMinus1 + 1) : picParams.m_tileRows * picParams.m_tileCols;

        int16_t tileId           = 0;
        int16_t tileGroupId      = -1;
        int16_t lastStartTileIdx = -1;
        for (uint32_t i = 0; i < m_numTiles; i++)
        {
            DECODE_ASSERT(tileParams[i].m_badBSBufferChopping == 0);//this is to assume the whole tile is in one single bitstream buffer
            DECODE_ASSERT(tileParams[i].m_bsTileBytesInBuffer == tileParams[i].m_bsTilePayloadSizeInBytes);//this is to assume the whole tile is in one single bitstream buffer

            // Check invalid tile column and tile row
            if (tileParams[i].m_tileColumn > picParams.m_tileCols || tileParams[i].m_tileRow > picParams.m_tileRows)
            {
                DECODE_ASSERTMESSAGE("Invalid tile column or tile row\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            if (!picParams.m_picInfoFlags.m_fields.m_largeScaleTile)
            {
                //record info
                if (tileParams[i].m_startTileIdx != lastStartTileIdx)
                {
                    tileGroupId++;
                }
                lastStartTileIdx = tileParams[i].m_startTileIdx;
                tileId = tileParams[i].m_tileColumn + tileParams[i].m_tileRow * picParams.m_tileCols;
                if (tileParams[i].m_badBSBufferChopping == 0 || tileParams[i].m_badBSBufferChopping == 2)//if all tile data received
                {
                    m_lastTileId = tileId;//record the last tile ID whose bitstream is available for HW to decode
                }
            }

            // check duplicate tile
            auto index = (picParams.m_picInfoFlags.m_fields.m_largeScaleTile) ? i : tileId; 
            if (m_tileDesc[index].m_tileIndexCount > 0 )
            {
                if (tileParams[i].m_bsTileBytesInBuffer > m_tileDesc[index].m_size)
                {
                    m_tileDesc[index].m_offset = tileParams[i].m_bsTileDataLocation;
                    m_tileDesc[index].m_size   = tileParams[i].m_bsTileBytesInBuffer;
                }
                m_tileDesc[index].m_tileIndexCount++;
                m_hasDuplicateTile = true;
            }
            else
            {
                m_tileDesc[index].m_offset     = tileParams[i].m_bsTileDataLocation;
                m_tileDesc[index].m_size       = tileParams[i].m_bsTileBytesInBuffer;
                m_tileDesc[index].m_tileRow    = tileParams[i].m_tileRow;
                m_tileDesc[index].m_tileColumn = tileParams[i].m_tileColumn;
                m_tileDesc[index].m_tileIndexCount++;
            }

            if (!picParams.m_picInfoFlags.m_fields.m_largeScaleTile)
            {
                m_tileDesc[index].m_tileGroupId = (uint16_t)tileGroupId;
                m_tileDesc[index].m_lastInGroup = (tileId == tileParams[i].m_endTileIdx) ? true : false;
                m_tileDesc[index].m_tileNum     = tileId - tileParams[i].m_startTileIdx;
            }
            else
            {
                //No TG for ext-tile, set TG related params to 0
                m_tileDesc[index].m_tileGroupId = 0;
                m_tileDesc[index].m_lastInGroup = 0;
                m_tileDesc[index].m_tileNum     = 0;

                //ext-tile specific params
                m_tileDesc[index].m_tileIndex      = tileParams[i].m_tileIndex;
                m_tileDesc[index].m_anchorFrameIdx = tileParams[i].m_anchorFrameIdx.FrameIdx;
            }
        }

        if ((m_lastTileId + 1) == m_totalTileNum)
        {
            m_newFrameStart = true;
        }
        else
        {
            m_newFrameStart = false;
        }

        // Do error detection and concealment
        DECODE_CHK_STATUS(ErrorDetectAndConceal());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTile::CalculateTileCols(CodecAv1PicParams & picParams)
    {
        DECODE_FUNC_CALL();

        int32_t mibSizeLog2 = picParams.m_seqInfoFlags.m_fields.m_use128x128Superblock ? av1MaxMibSizeLog2 : av1MinMibSizeLog2;
        int32_t miCols = MOS_ALIGN_CEIL(m_miCols, 1 << mibSizeLog2);
        int32_t sbCols = miCols >> mibSizeLog2;

        //calc tile col start for all the tiles except the last one
        uint16_t i, start_sb = 0;
        for (i = 0, start_sb = 0; i < picParams.m_tileCols - 1; i++)
        {
            m_tileColStartSb[i] = start_sb;//calc tile col start
            start_sb += picParams.m_widthInSbsMinus1[i] + 1;
        }

        //calc for the last tile
        m_tileColStartSb[i] = start_sb;
        DECODE_CHK_COND(sbCols < (start_sb + 1), "Invalid tile col start of the last tile");
        picParams.m_widthInSbsMinus1[i] = sbCols - start_sb - 1;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTile::CalculateTileRows(CodecAv1PicParams &picParams)
    {
        DECODE_FUNC_CALL();

        int32_t mibSizeLog2 = picParams.m_seqInfoFlags.m_fields.m_use128x128Superblock ? av1MaxMibSizeLog2 : av1MinMibSizeLog2;
        int32_t miRows = MOS_ALIGN_CEIL(m_miRows, 1 << mibSizeLog2);
        int32_t sbRows = miRows >> mibSizeLog2;

        //calc tile row start for all the tiles except the last one
        uint16_t i, start_sb;
        for (i = 0, start_sb = 0; i < picParams.m_tileRows - 1; i++)
        {
            m_tileRowStartSb[i] = start_sb;//calc tile Row start
            start_sb += picParams.m_heightInSbsMinus1[i] + 1;
        }

        //calc for the last tile
        m_tileRowStartSb[i] = start_sb;
        DECODE_CHK_COND(sbRows < (start_sb + 1), "Invalid tile row start of the last tile");
        picParams.m_heightInSbsMinus1[i] = sbRows - start_sb - 1;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTile::CalcTileInfoMaxTile(CodecAv1PicParams &picParams)
    {
        DECODE_FUNC_CALL();

        m_miCols = MOS_ALIGN_CEIL(picParams.m_frameWidthMinus1 + 1, 8) >> av1MiSizeLog2;
        m_miRows = MOS_ALIGN_CEIL(picParams.m_frameHeightMinus1 + 1, 8) >> av1MiSizeLog2;
        DECODE_CHK_STATUS(CalculateTileCols(picParams));
        DECODE_CHK_STATUS(CalculateTileRows(picParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodeTile::CalcNumPass(const CodecAv1PicParams &picParams, CodecAv1TileParams *tileParams)
    {
        DECODE_FUNC_CALL();

        uint16_t passNum;
        uint16_t startTile = m_lastTileId + 1;//record before parsing new bitstream portion

        DECODE_CHK_STATUS(ParseTileInfo(picParams, tileParams));

        if (picParams.m_picInfoFlags.m_fields.m_largeScaleTile)
        {
            passNum   = picParams.m_tileCountMinus1 + 1;
            m_curTile = 0;
        }
        else
        {
            passNum   = m_lastTileId - startTile + 1;
            m_curTile = startTile;
        }

        m_decPassNum = passNum;
        return MOS_STATUS_SUCCESS;
    }

    void Av1DecodeTile::GetUpscaleConvolveStepX0(const CodecAv1PicParams &picParams, bool isChroma)
    {
        DECODE_FUNC_CALL();

        int32_t ssX = isChroma && picParams.m_seqInfoFlags.m_fields.m_subsamplingX;
        int32_t downscaledPlaneWidth    = ROUND_POWER_OF_TWO(picParams.m_frameWidthMinus1 + 1, ssX);
        int32_t upscaledPlaneWidth      = ROUND_POWER_OF_TWO(picParams.m_superResUpscaledWidthMinus1 + 1, ssX);

        //calculate step
        int32_t xStepQn = ((downscaledPlaneWidth << av1RsScaleSubpelBits) + upscaledPlaneWidth / 2) / upscaledPlaneWidth;
        if (isChroma)
        {
            m_chromaXStepQn = xStepQn;
        }
        else
        {
            m_lumaXStepQn = xStepQn;
        }

        //Calculate x0_qn for each tile column
        int32_t err = upscaledPlaneWidth * xStepQn - (downscaledPlaneWidth << av1RsScaleSubpelBits);
        int32_t x0 =
            (-((upscaledPlaneWidth - downscaledPlaneWidth) << (av1RsScaleSubpelBits - 1)) +
                upscaledPlaneWidth / 2) /
            upscaledPlaneWidth +
            av1RsScaleExtraOff - err / 2;
        x0 = (int32_t)((uint32_t)x0 & av1RsScaleSubpelMask);

        if (picParams.m_tileCols > 64)
        {
            DECODE_ASSERTMESSAGE("Array index exceeds upper bound.");
            return;
        }

        for (auto col = 0; col < picParams.m_tileCols; col++)
        {
            if (isChroma)
            {
                m_chromaX0Qn[col] = x0;
            }
            else
            {
                m_lumaX0Qn[col] = x0;
            }

            int32_t tileColEndSb;
            if (col < picParams.m_tileCols - 1)
            {
                tileColEndSb = m_tileColStartSb[col + 1];
            }
            else
            {
                tileColEndSb = m_tileColStartSb[picParams.m_tileCols - 1] + picParams.m_widthInSbsMinus1[picParams.m_tileCols - 1];

            }
            int32_t mibSizeLog2 = picParams.m_seqInfoFlags.m_fields.m_use128x128Superblock ? av1MaxMibSizeLog2 : av1MinMibSizeLog2;

            int32_t miColEnd = tileColEndSb << mibSizeLog2;
            miColEnd = AOMMIN(miColEnd, m_miCols);
            int32_t downscaledX1    = miColEnd << (av1MiSizeLog2 - ssX);
            int32_t downscaledX0    = m_tileColStartSb[col] << mibSizeLog2 << (av1MiSizeLog2 - ssX);

            int32_t srcWidth    = downscaledX1 - downscaledX0;
            int32_t upscaledX0  = (downscaledX0 * picParams.m_superresScaleDenominator) / av1ScaleNumerator;
            int32_t upscaledX1;
            if (col == picParams.m_tileCols - 1)
            {
                upscaledX1 = upscaledPlaneWidth;
            }
            else
            {
                upscaledX1 = (downscaledX1 * picParams.m_superresScaleDenominator) / av1ScaleNumerator;
            }
            int32_t dstWidth = upscaledX1 - upscaledX0;

            // Update the fractional pixel offset to prepare for the next tile column.
            x0 += (dstWidth * xStepQn) - (srcWidth << av1RsScaleSubpelBits);
        }
    }

}   // namespace decode
