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
//! \file     encode_hevc_tile.cpp
//! \brief    Defines the common interface for hevc tile
//!

#include "encode_hevc_tile.h"
#include "encode_hevc_basic_feature.h"
#include "codec_def_common.h"
#include "encode_hevc_vdenc_feature_manager.h"
#include "encode_pipeline.h"

namespace encode
{
    HevcEncodeTile::HevcEncodeTile(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings) :
        EncodeTile(featureManager, allocator, hwInterface, constSettings)
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
        m_mosCtx = hwInterface->GetOsInterface()->pOsContext;

        auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager *>(featureManager);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

        m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

        m_hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(hwInterface->GetHcpInterfaceNext());
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hcpItf);
    }

    MOS_STATUS HevcEncodeTile::Init(void* settings)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(settings);

        ENCODE_CHK_STATUS_RETURN(EncodeTile::Init(settings));

#if (_DEBUG || _RELEASE_INTERNAL)
        // Tile Replay Enable should be passed from DDI, will change later when DDI is ready
        MediaUserSetting::Value outValue;
        ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "HEVC VDEnc TileReplay Enable",
            MediaUserSetting::Group::Sequence);
        m_enableTileReplay = outValue.Get<bool>();
#else
        m_enableTileReplay = false;
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcEncodeTile::Update(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);

        EncoderParams* encodeParams = (EncoderParams*)params;;
        ENCODE_CHK_NULL_RETURN(encodeParams);

        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams =
        static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(hevcPicParams);

        m_enabled = hevcPicParams->tiles_enabled_flag ? true : false;


        m_maxTileNumber = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, CODECHAL_HEVC_MIN_TILE_SIZE) *
                             CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, CODECHAL_HEVC_MIN_TILE_SIZE);

        ENCODE_CHK_STATUS_RETURN(EncodeTile::Update(params));

        return MOS_STATUS_SUCCESS;
    }

    bool HevcEncodeTile::IsLcuInTile(
        uint32_t        lcuX,
        uint32_t        lcuY,
        EncodeTileData *currentTile)
    {
        ENCODE_FUNC_CALL();

        if (m_basicFeature == nullptr)
        {
            ENCODE_ASSERTMESSAGE("Null basic feature, unexpected!");
            return false;
        }
        auto hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        if (hevcBasicFeature == nullptr)
        {
            ENCODE_ASSERTMESSAGE("Null hevc basic feature, unexpected!");
            return false;
        }
        auto hevcSeqParams = hevcBasicFeature->m_hevcSeqParams;
        if (hevcSeqParams == nullptr)
        {
            ENCODE_ASSERTMESSAGE("Null seq parameters, unexpected!");
            return false;
        }

        uint32_t shift           = hevcSeqParams->log2_max_coding_block_size_minus3 - hevcSeqParams->log2_min_coding_block_size_minus3;
        uint32_t residual        = (1 << shift) - 1;
        uint32_t tileColumnWidth = (currentTile->tileWidthInMinCbMinus1 + 1 + residual) >> shift;
        uint32_t tileRowHeight   = (currentTile->tileHeightInMinCbMinus1 + 1 + residual) >> shift;

        return (lcuX < currentTile->tileStartXInLCU ||
                lcuY < currentTile->tileStartYInLCU ||
                lcuX >= currentTile->tileStartXInLCU + tileColumnWidth ||
                lcuY >= currentTile->tileStartYInLCU + tileRowHeight);
    }

    MOS_STATUS HevcEncodeTile::IsSliceInTile(
        uint32_t       sliceNumber,
        EncodeTileData *currentTile,
        bool *         sliceInTile,
        bool *         lastSliceInTile)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(currentTile);
        ENCODE_CHK_NULL_RETURN(sliceInTile);
        ENCODE_CHK_NULL_RETURN(lastSliceInTile);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        auto tmpHevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(tmpHevcBasicFeature);

        auto hevcPicParams   = tmpHevcBasicFeature->m_hevcPicParams;
        auto hevcSeqParams   = tmpHevcBasicFeature->m_hevcSeqParams;
        auto hevcSliceParams = tmpHevcBasicFeature->m_hevcSliceParams;
        ENCODE_CHK_NULL_RETURN(hevcPicParams);
        ENCODE_CHK_NULL_RETURN(hevcSeqParams);
        ENCODE_CHK_NULL_RETURN(hevcSliceParams);

        if (!m_enabled)
        {
            *lastSliceInTile = *sliceInTile = true;
            return eStatus;
        }

        uint32_t shift            = hevcSeqParams->log2_max_coding_block_size_minus3 - hevcSeqParams->log2_min_coding_block_size_minus3;
        uint32_t residual         = (1 << shift) - 1;
        uint32_t frameWidthInLCU  = (hevcSeqParams->wFrameWidthInMinCbMinus1 + 1 + residual) >> shift;
        uint32_t frameHeightInLCU = (hevcSeqParams->wFrameHeightInMinCbMinus1 + 1 + residual) >> shift;

        uint32_t tileColumnWidth = (currentTile->tileWidthInMinCbMinus1 + 1 + residual) >> shift;
        uint32_t tileRowHeight   = (currentTile->tileHeightInMinCbMinus1 + 1 + residual) >> shift;

        const CODEC_HEVC_ENCODE_SLICE_PARAMS *hevcSlcParams = &hevcSliceParams[sliceNumber];
        uint32_t                              sliceStartLCU = hevcSlcParams->slice_segment_address;
        uint32_t                              sliceLCUx     = sliceStartLCU % frameWidthInLCU;
        uint32_t                              sliceLCUy     = sliceStartLCU / frameWidthInLCU;

        if (IsLcuInTile(sliceLCUx, sliceLCUy, currentTile))
        {
            // slice start is not in the tile boundary
            *lastSliceInTile = *sliceInTile = false;
            return eStatus;
        }

        sliceLCUx += (hevcSlcParams->NumLCUsInSlice - 1) % tileColumnWidth;
        sliceLCUy += (hevcSlcParams->NumLCUsInSlice - 1) / tileColumnWidth;

        if (sliceLCUx >= currentTile->tileStartXInLCU + tileColumnWidth)
        {
            sliceLCUx -= tileColumnWidth;
            sliceLCUy++;
        }

        if (IsLcuInTile(sliceLCUx, sliceLCUy, currentTile))
        {
            // last LCU of the slice is out of the tile boundary
            *lastSliceInTile = *sliceInTile = false;
            return eStatus;
        }

        *sliceInTile = true;

        sliceLCUx++;
        sliceLCUy++;

        // the end of slice is at the boundary of tile
        *lastSliceInTile = (sliceLCUx == currentTile->tileStartXInLCU + tileColumnWidth &&
                            sliceLCUy == currentTile->tileStartYInLCU + tileRowHeight);

        return eStatus;
    }

    MOS_STATUS HevcEncodeTile::SetPipeNumber(uint32_t numPipes)
    {
        ENCODE_FUNC_CALL();

        m_curTileCodingParams.NumberOfActiveBePipes = numPipes;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcEncodeTile::SetCurrentTileFromSliceIndex(
        uint32_t        slcCount,
        EncodePipeline *pipeline)
    {
        ENCODE_FUNC_CALL();

        bool lastSliceInTile = false, sliceInTile = false;

        EncodeTileData curTileData = {};

        // Find the tileData corresponding to this slice
        for (uint32_t tileRow = 0; tileRow < m_numTileRows; tileRow++)
        {
            for (uint32_t tileCol = 0; tileCol < m_numTileColumns; tileCol++)
            {
                ENCODE_CHK_STATUS_RETURN(SetCurrentTile(tileRow, tileCol, pipeline));
                ENCODE_CHK_STATUS_RETURN(GetCurrentTile(curTileData));
                ENCODE_CHK_STATUS_RETURN(IsSliceInTile(slcCount, &curTileData, &sliceInTile, &lastSliceInTile));

                if (sliceInTile)
                {
                    break;
                }
            }
            if (sliceInTile)
            {
                break;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcEncodeTile::CalculateNumLcuByTiles(PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(hevcPicParams);

        m_numLcuInPic = 0;
        for (uint32_t numLcusInTiles = 0, i = 0; i < m_numTileRows; i++)
        {
            for (uint32_t j = 0; j < m_numTileColumns; j++)
            {
                m_numLcuInPic += hevcPicParams->tile_row_height[i] * hevcPicParams->tile_column_width[j];
            }
        }
        if (m_numLcuInPic == 0)
        {
            ENCODE_ASSERTMESSAGE("LCU num cal by each tile is zero, sth must be wrong!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcEncodeTile::CalculateTilesBoundary(
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams,
        uint32_t *rowBd,
        uint32_t *colBd)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(hevcPicParams);
        ENCODE_CHK_NULL_RETURN(rowBd);
        ENCODE_CHK_NULL_RETURN(colBd);

        for (uint32_t i = 0; i < m_numTileColumns; i++)
        {
            colBd[i + 1] = colBd[i] + hevcPicParams->tile_column_width[i];
        }

        for (uint32_t i = 0; i < m_numTileRows; i++)
        {
            rowBd[i + 1] = rowBd[i] + hevcPicParams->tile_row_height[i];
        }

        return MOS_STATUS_SUCCESS;

    }

    MOS_STATUS HevcEncodeTile::CalculateTileWidthAndHeight(
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams,
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
        uint32_t  rowIndex,
        uint32_t  colIndex,
        uint32_t *rowBd,
        uint32_t *colBd)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(hevcPicParams);
        ENCODE_CHK_NULL_RETURN(hevcSeqParams);
        ENCODE_CHK_NULL_RETURN(rowBd);
        ENCODE_CHK_NULL_RETURN(colBd);

        uint32_t idx                = rowIndex * m_numTileColumns + colIndex;
        int32_t  frameWidthInMinCb  = hevcSeqParams->wFrameWidthInMinCbMinus1 + 1;
        int32_t  frameHeightInMinCb = hevcSeqParams->wFrameHeightInMinCbMinus1 + 1;
        int32_t  shift              = hevcSeqParams->log2_max_coding_block_size_minus3 - hevcSeqParams->log2_min_coding_block_size_minus3;

        if (colIndex != m_numTileColumns - 1)
        {
            m_tileData[idx].tileWidthInMinCbMinus1 = (hevcPicParams->tile_column_width[colIndex] << shift) - 1;
            m_tileData[idx].isLastTileofRow        = false;
        }
        else
        {
            m_tileData[idx].tileWidthInMinCbMinus1 = (frameWidthInMinCb - (colBd[colIndex] << shift)) - 1;
            m_tileData[idx].isLastTileofRow        = true;
        }

        if (rowIndex != m_numTileRows - 1)
        {
            m_tileData[idx].isLastTileofColumn      = false;
            m_tileData[idx].tileHeightInMinCbMinus1 = (hevcPicParams->tile_row_height[rowIndex] << shift) - 1;
        }
        else
        {
            m_tileData[idx].tileHeightInMinCbMinus1 = (frameHeightInMinCb - (rowBd[rowIndex] << shift)) - 1;
            m_tileData[idx].isLastTileofColumn      = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcEncodeTile::GetTileInfo(
        uint32_t xPosition,
        uint32_t yPosition,
        uint32_t &tileStartLcuX,
        uint32_t &tileEndLcuX,
        uint32_t &tileStartLcuY,
        uint32_t &tileEndLcuY,
        uint32_t &tileStreaminOffset)
    {
        MOS_STATUS eStatus = MOS_STATUS_INVALID_PARAMETER;

        for (uint32_t i = 0; i < m_numTiles; i++)
        {
            tileStartLcuX = m_tileData[i].tileStartXInLCU;
            tileStartLcuY = m_tileData[i].tileStartYInLCU;
            tileEndLcuX   = m_tileData[i].tileEndXInLCU;
            tileEndLcuY   = m_tileData[i].tileEndYInLCU;

            tileStreaminOffset = m_tileData[i].tileStreaminOffset;

            if ((xPosition >= (tileStartLcuX * 2)) &&
                (yPosition >= (tileStartLcuY * 2)) &&
                (xPosition < (tileEndLcuX * 2)) &&
                (yPosition < (tileEndLcuY * 2)))
            {
                eStatus = MOS_STATUS_SUCCESS;
                break;
            }
        }

        return eStatus;
    }

    MOS_STATUS HevcEncodeTile::SetTileData(void *params)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus       = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }

        ENCODE_CHK_NULL_RETURN(params);

        EncoderParams* encodeParams = (EncoderParams*)params;;
        ENCODE_CHK_NULL_RETURN(encodeParams);

        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams =
        static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(hevcPicParams);

        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams =
        static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
        ENCODE_CHK_NULL_RETURN(hevcSeqParams);

        PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSliceParams =
        static_cast<PCODEC_HEVC_ENCODE_SLICE_PARAMS>(encodeParams->pSliceParams);
        ENCODE_CHK_NULL_RETURN(hevcSliceParams);

        m_numTileRows    = hevcPicParams->num_tile_rows_minus1 + 1;
        m_numTileColumns = hevcPicParams->num_tile_columns_minus1 + 1;

        uint32_t colBd[m_maxTileBdNum] = {0};
        uint32_t rowBd[m_maxTileBdNum] = {0};
        ENCODE_CHK_STATUS_RETURN(CalculateTilesBoundary(hevcPicParams, &rowBd[0], &colBd[0]));

        m_numTiles = m_numTileRows * m_numTileColumns;
        if (m_numTiles > CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, CODECHAL_HEVC_VDENC_MIN_TILE_WIDTH_SIZE) *
                CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, CODECHAL_HEVC_VDENC_MIN_TILE_HEIGHT_SIZE))
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        bool is10Bit = m_basicFeature->m_is10Bit;

        uint32_t const numCuRecordTab[] = {1, 4, 16, 64};  //LCU: 8x8->1, 16x16->4, 32x32->16, 64x64->64
        uint32_t       numCuRecord      = numCuRecordTab[MOS_MIN(3, hevcSeqParams->log2_max_coding_block_size_minus3)];
        uint32_t       maxBytePerLCU    = 1 << (hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
        maxBytePerLCU = maxBytePerLCU * maxBytePerLCU;  // number of pixels per LCU
        maxBytePerLCU = maxBytePerLCU * 3 / (is10Bit ? 1 : 2);  //assume 4:2:0 format
        uint32_t bitstreamByteOffset = 0, saoRowstoreOffset = 0, cuLevelStreamoutOffset = 0, sseRowstoreOffset = 0;
        int32_t  frameWidthInMinCb  = hevcSeqParams->wFrameWidthInMinCbMinus1 + 1;
        int32_t  frameHeightInMinCb = hevcSeqParams->wFrameHeightInMinCbMinus1 + 1;
        int32_t  shift              = hevcSeqParams->log2_max_coding_block_size_minus3 - hevcSeqParams->log2_min_coding_block_size_minus3;
        uint32_t ctbSize            = 1 << (hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
        uint32_t streamInWidthinLCU = MOS_ROUNDUP_DIVIDE((frameWidthInMinCb << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), ctbSize);
        uint32_t tileStartLCUAddr   = 0;

        ENCODE_CHK_STATUS_RETURN(CalculateNumLcuByTiles(hevcPicParams));

        uint64_t    activeBitstreamSize = (uint64_t)m_basicFeature->m_bitstreamSize;
        // There would be padding at the end of last tile in CBR, reserve dedicated part in the BS buf
        if (hevcSeqParams->RateControlMethod == RATECONTROL_CBR)
        {
            // Assume max padding num < target frame size derived from target bit rate and frame rate
            if (hevcSeqParams->FrameRate.Denominator == 0)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
            uint32_t actualFrameRate = hevcSeqParams->FrameRate.Numerator / hevcSeqParams->FrameRate.Denominator;
            uint64_t reservedPart    = (uint64_t)hevcSeqParams->TargetBitRate / 8 / (uint64_t)actualFrameRate * 1024;

            if (reservedPart > activeBitstreamSize)
            {
                ENCODE_ASSERTMESSAGE("Frame size cal from target Bit rate is larger than BS buf! Issues in CBR paras!");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            // Capping the reserved part to 1/10 of bs buf size
            if (reservedPart > activeBitstreamSize / 10)
            {
                reservedPart = activeBitstreamSize / 10;
            }

            activeBitstreamSize -= reservedPart;
        }

        for (uint32_t numLcusInTiles = 0, i = 0; i < m_numTileRows; i++)
        {
            for (uint32_t j = 0; j < m_numTileColumns; j++)
            {
                uint32_t idx          = i * m_numTileColumns + j;
                uint32_t numLcuInTile = hevcPicParams->tile_row_height[i] * hevcPicParams->tile_column_width[j];

                m_tileData[idx].tileStartXInLCU = colBd[j];
                m_tileData[idx].tileStartYInLCU = rowBd[i];

                m_tileData[idx].tileColumnStoreSelect = j % 2;
                m_tileData[idx].tileRowStoreSelect    = i % 2;

                ENCODE_CHK_STATUS_RETURN(CalculateTileWidthAndHeight(hevcPicParams, hevcSeqParams, i, j, &rowBd[0], &colBd[0]));

                m_tileData[idx].numOfTilesInFrame       = m_numTiles;
                m_tileData[idx].numOfTileColumnsInFrame = m_numTileColumns;
                m_tileData[idx].cuRecordOffset = MOS_ALIGN_CEIL(((numCuRecord * numLcusInTiles) * m_hcpItf->GetEncCuRecordSize()),
                                                     CODECHAL_CACHELINE_SIZE) / CODECHAL_CACHELINE_SIZE;

                m_tileData[idx].pakTileStatisticsOffset              = 9 * idx;
                m_tileData[idx].tileSizeStreamoutOffset              = idx;
                m_tileData[idx].vp9ProbabilityCounterStreamoutOffset = 0;

                m_tileData[idx].cuLevelStreamoutOffset   = cuLevelStreamoutOffset;
                m_tileData[idx].sseRowstoreOffset        = sseRowstoreOffset;
                m_tileData[idx].bitstreamByteOffset      = bitstreamByteOffset;
                m_tileData[idx].saoRowstoreOffset        = saoRowstoreOffset;

                uint32_t tileHeightInLCU                 = MOS_ROUNDUP_DIVIDE(((m_tileData[idx].tileHeightInMinCbMinus1 + 1) << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), ctbSize);
                uint32_t tileWidthInLCU                  = MOS_ROUNDUP_DIVIDE(((m_tileData[idx].tileWidthInMinCbMinus1 + 1) << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), ctbSize);

                m_tileData[idx].tileEndXInLCU              = m_tileData[idx].tileStartXInLCU + tileWidthInLCU;
                m_tileData[idx].tileEndYInLCU              = m_tileData[idx].tileStartYInLCU + tileHeightInLCU;

                //StreamIn data is 4 CLs per LCU
                m_tileData[idx].tileStreaminOffset       = 4 * (m_tileData[idx].tileStartYInLCU * streamInWidthinLCU + m_tileData[idx].tileStartXInLCU * tileHeightInLCU);
                m_tileData[idx].sliceSizeStreamoutOffset = tileStartLCUAddr;

                tileStartLCUAddr += (tileWidthInLCU * tileHeightInLCU);

                cuLevelStreamoutOffset += (m_tileData[idx].tileWidthInMinCbMinus1 + 1) * (m_tileData[idx].tileHeightInMinCbMinus1 + 1) * 16 / CODECHAL_CACHELINE_SIZE;
                sseRowstoreOffset += ((hevcPicParams->tile_column_width[j] + 3) * ((HevcBasicFeature*)m_basicFeature)->m_sizeOfSseSrcPixelRowStoreBufferPerLcu) / CODECHAL_CACHELINE_SIZE;
                saoRowstoreOffset += (MOS_ALIGN_CEIL(hevcPicParams->tile_column_width[j], 4) * CODECHAL_HEVC_SAO_STRMOUT_SIZE_PERLCU) / CODECHAL_CACHELINE_SIZE;

                uint64_t totalSizeTemp = (uint64_t)activeBitstreamSize * (uint64_t)numLcuInTile;
                uint32_t bitStreamSizePerTile = (uint32_t)(totalSizeTemp / (uint64_t)m_numLcuInPic) + ((totalSizeTemp % (uint64_t)m_numLcuInPic) ? 1 : 0);
                bitstreamByteOffset += MOS_ALIGN_CEIL(bitStreamSizePerTile, CODECHAL_CACHELINE_SIZE) / CODECHAL_CACHELINE_SIZE;

                numLcusInTiles += numLcuInTile;
            }

            // same row store buffer for different tile rows.
            saoRowstoreOffset = 0;
            sseRowstoreOffset = 0;
        }

        return eStatus;
    }

    MOS_STATUS HevcEncodeTile::AllocateTileStatistics(void *params)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(params);

        if (!m_enabled)
        {
            return eStatus;
        }

        EncoderParams *encodeParams = (EncoderParams *)params;
        ENCODE_CHK_NULL_RETURN(encodeParams);

        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams =
            static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(hevcPicParams);

        auto num_tile_rows    = hevcPicParams->num_tile_rows_minus1 + 1;
        auto num_tile_columns = hevcPicParams->num_tile_columns_minus1 + 1;
        auto num_tiles        = num_tile_rows * num_tile_columns;

        uint32_t maxLcuSize = 64;
        if (m_numLcuInPic == 0)
        {
            ENCODE_ASSERTMESSAGE("LCU num cal by each tile is zero, sth must be wrong!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        MOS_ZeroMemory(&m_hevcFrameStatsOffset, sizeof(HevcTileStatusInfo));
        MOS_ZeroMemory(&m_hevcTileStatsOffset, sizeof(HevcTileStatusInfo));
        MOS_ZeroMemory(&m_hevcStatsSize, sizeof(HevcTileStatusInfo));

        // Set the maximum size based on frame level statistics.
        m_hevcStatsSize.tileSizeRecord     = CODECHAL_CACHELINE_SIZE;
        m_hevcStatsSize.hevcPakStatistics  = EncodeBasicFeature::m_sizeOfHcpPakFrameStats;
        m_hevcStatsSize.vdencStatistics    = CODECHAL_HEVC_VDENC_STATS_SIZE;
        m_hevcStatsSize.hevcSliceStreamout = CODECHAL_CACHELINE_SIZE;

        // Maintain the offsets to use for patching addresses in to the HuC Pak Integration kernel Aggregated Frame Statistics Output Buffer
        // Each offset needs to be page aligned as the combined region is fed into different page aligned HuC regions
        m_hevcFrameStatsOffset.tileSizeRecord     = 0;  // Tile Size Record is not present in resHuCPakAggregatedFrameStatsBuffer
        m_hevcFrameStatsOffset.hevcPakStatistics  = 0;
        m_hevcFrameStatsOffset.vdencStatistics    = MOS_ALIGN_CEIL(m_hevcFrameStatsOffset.hevcPakStatistics + m_hevcStatsSize.hevcPakStatistics, CODECHAL_PAGE_SIZE);
        m_hevcFrameStatsOffset.hevcSliceStreamout = MOS_ALIGN_CEIL(m_hevcFrameStatsOffset.vdencStatistics + m_hevcStatsSize.vdencStatistics, CODECHAL_PAGE_SIZE);

        // Frame level statistics
        m_hwInterface->m_pakIntAggregatedFrameStatsSize = MOS_ALIGN_CEIL(m_hevcFrameStatsOffset.hevcSliceStreamout + (m_hevcStatsSize.hevcSliceStreamout * m_numLcuInPic), CODECHAL_PAGE_SIZE);

        // HEVC Frame Statistics Buffer - Output from HuC PAK Integration kernel
        if (Mos_ResourceIsNull(&m_resHuCPakAggregatedFrameStatsBuffer))
        {
            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.dwBytes  = m_hwInterface->m_pakIntAggregatedFrameStatsSize;
            allocParamsForBufferLinear.pBufName = "HCP Aggregated Frame Statistics Streamout Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            auto resource = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(resource);
            m_resHuCPakAggregatedFrameStatsBuffer = *resource;
        }

        // Maintain the offsets to use for patching addresses in to the Tile Based Statistics Buffer
        // Each offset needs to be page aligned as the combined region is fed into different page aligned HuC regions
        m_hevcTileStatsOffset.tileSizeRecord     = 0; // TileRecord is a separated resource
        m_hevcTileStatsOffset.hevcPakStatistics  = 0; // PakStatistics is head of m_resTileBasedStatisticsBuffer;
        m_hevcTileStatsOffset.vdencStatistics    = MOS_ALIGN_CEIL(m_hevcTileStatsOffset.hevcPakStatistics + (m_hevcStatsSize.hevcPakStatistics * num_tiles), CODECHAL_PAGE_SIZE);
        m_hevcTileStatsOffset.hevcSliceStreamout = MOS_ALIGN_CEIL(m_hevcTileStatsOffset.vdencStatistics + (m_hevcStatsSize.vdencStatistics * num_tiles), CODECHAL_PAGE_SIZE);
        // Combined statistics size for all tiles
        m_hwInterface->m_pakIntTileStatsSize = MOS_ALIGN_CEIL(m_hevcTileStatsOffset.hevcSliceStreamout + m_hevcStatsSize.hevcSliceStreamout * m_numLcuInPic, CODECHAL_PAGE_SIZE);

        // Tile size record size for all tiles
        m_hwInterface->m_tileRecordSize = m_hevcStatsSize.tileSizeRecord * num_tiles;

        MOS_SURFACE surface;
        MOS_ZeroMemory(&surface, sizeof(surface));
        surface.OsResource = m_resTileBasedStatisticsBuffer[m_statisticsBufIndex];
        if (!Mos_ResourceIsNull(&surface.OsResource))
        {
            m_allocator->GetSurfaceInfo(&surface);
        }
        uint32_t curPakIntTileStatsSize = surface.dwHeight * surface.dwWidth;

        if (Mos_ResourceIsNull(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]) ||
            curPakIntTileStatsSize < m_hwInterface->m_pakIntTileStatsSize)
        {
            if (!Mos_ResourceIsNull(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]))
            {
                m_allocator->DestroyResource(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]);
            }
            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.dwBytes  = m_hwInterface->m_pakIntTileStatsSize;
            allocParamsForBufferLinear.pBufName = "HCP Tile Level Statistics Streamout Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
            auto resource = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(resource);
            m_resTileBasedStatisticsBuffer[m_statisticsBufIndex] = *resource;
        }

        // Allocate the updated tile size buffer for PAK integration kernel
        if (Mos_ResourceIsNull(&m_tileRecordBuffer[m_statisticsBufIndex]))
        {
            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.dwBytes  = CODECHAL_CACHELINE_SIZE * num_tiles;
            allocParamsForBufferLinear.pBufName = "Tile Record Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;

            auto resource = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(resource);
            m_tileRecordBuffer[m_statisticsBufIndex] = *resource;
        }

        return eStatus;
    }

    MOS_STATUS HevcEncodeTile::GetTileStatusInfo(
        HevcTileStatusInfo &hevcTileStatsOffset,
        HevcTileStatusInfo &hevcFrameStatsOffset,
        HevcTileStatusInfo &hevcStatsSize)
    {
        ENCODE_FUNC_CALL();
        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        hevcTileStatsOffset  = m_hevcTileStatsOffset;
        hevcFrameStatsOffset = m_hevcFrameStatsOffset;
        hevcStatsSize        = m_hevcStatsSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcEncodeTile::GetTileInfo(HevcTileInfo *hevcTileInfo) const
    {
        ENCODE_CHK_NULL_RETURN(hevcTileInfo);

        if (m_enabled)
        {
            hevcTileInfo->tileId = static_cast<uint16_t>(m_tileIdx);  // Tile number in a frame

            const auto currTileData = m_tileData[m_tileIdx];

            hevcTileInfo->tileColPositionInSb = static_cast<uint16_t>(currTileData.tileStartX);
            hevcTileInfo->tileRowPositionInSb = static_cast<uint16_t>(currTileData.tileStartY);

            hevcTileInfo->tileWidthInSbMinus1  = currTileData.tileEndX - currTileData.tileStartX - 1;
            hevcTileInfo->tileHeightInSbMinus1 = currTileData.tileEndY - currTileData.tileStartY - 1;

            hevcTileInfo->tileStartXInLCU = currTileData.tileStartX;
            hevcTileInfo->tileStartYInLCU = currTileData.tileStartY;
            hevcTileInfo->tileEndXInLCU   = currTileData.tileEndX;
            hevcTileInfo->tileEndYInLCU   = currTileData.tileEndY;
            hevcTileInfo->tileNum         = static_cast<uint16_t>(m_tileIdx);
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, HevcEncodeTile)
    {
        if (m_enabled)
        {
            params.tileBasedReplayMode = m_enableTileReplay;
        }
        else
        {
            params.tileBasedReplayMode = 0;
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, HevcEncodeTile)
    {
        ENCODE_FUNC_CALL();

        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        MOS_RESOURCE *tileStatisticsBuffer = const_cast<MOS_RESOURCE *>(& m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]);

        if (!Mos_ResourceIsNull(tileStatisticsBuffer))
        {
            params.streamOutBuffer = tileStatisticsBuffer;
            params.streamOutOffset = m_hevcTileStatsOffset.vdencStatistics;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_WALKER_STATE, HevcEncodeTile)
    {
        auto hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcBasicFeature);
        auto picParams = hevcBasicFeature->m_hevcPicParams;
        ENCODE_CHK_NULL_RETURN(picParams);
        auto seqParams = hevcBasicFeature->m_hevcSeqParams;
        ENCODE_CHK_NULL_RETURN(seqParams);
        auto t_sliceParams = hevcBasicFeature->m_hevcSliceParams;
        ENCODE_CHK_NULL_RETURN(t_sliceParams);
        CODEC_HEVC_ENCODE_SLICE_PARAMS *sliceParams = (CODEC_HEVC_ENCODE_SLICE_PARAMS *)&t_sliceParams[hevcBasicFeature->m_curNumSlices];

        uint32_t ctbSize     = 1 << (seqParams->log2_max_coding_block_size_minus3 + 3);
        uint32_t widthInPix  = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameWidthInMinCbMinus1 + 1);
        uint32_t widthInCtb  = (widthInPix / ctbSize) + ((widthInPix % ctbSize) ? 1 : 0);  // round up
        uint32_t heightInPix = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameHeightInMinCbMinus1 + 1);
        uint32_t heightInCtb = (heightInPix / ctbSize) + ((heightInPix % ctbSize) ? 1 : 0);  // round up
        uint32_t shift       = seqParams->log2_max_coding_block_size_minus3 - seqParams->log2_min_coding_block_size_minus3;

        if (!m_enabled)
        {
            params.firstSuperSlice = 0;
            // No tiling support
            params.tileSliceStartLcuMbY      = sliceParams->slice_segment_address / widthInCtb;
            params.nextTileSliceStartLcuMbX  = (sliceParams->slice_segment_address + sliceParams->NumLCUsInSlice) / heightInCtb;
            params.nextTileSliceStartLcuMbY  = (sliceParams->slice_segment_address + sliceParams->NumLCUsInSlice) / widthInCtb;
        }
        else
        {
            params.tileSliceStartLcuMbX = (&m_curTileCodingParams)->TileStartLCUX;
            params.tileSliceStartLcuMbY = (&m_curTileCodingParams)->TileStartLCUY;

            // In HEVC vdnec, always first super slice in each tile
            params.firstSuperSlice = 1;

            params.nextTileSliceStartLcuMbX = (&m_curTileCodingParams)->TileStartLCUX + ((&m_curTileCodingParams)->TileWidthInMinCbMinus1 >> shift) + 1;
            params.nextTileSliceStartLcuMbY = (&m_curTileCodingParams)->TileStartLCUY + ((&m_curTileCodingParams)->TileHeightInMinCbMinus1 >> shift) + 1;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HevcEncodeTile)
    {
        ENCODE_FUNC_CALL();

        // In single pipe mode, if TileBasedReplayMode is enabled,
        // the bit stream for each tile will not be continuous
        if (m_enabled)
        {
            params.bTileBasedReplayMode = m_enableTileReplay;
        }
        else
        {
            params.bTileBasedReplayMode = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_TILE_CODING, HevcEncodeTile)
    {
        ENCODE_FUNC_CALL();
        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        params.numOfTileColumnsInFrame              = m_curTileCodingParams.NumOfTileColumnsInFrame;
        params.tileRowStoreSelect                   = m_curTileCodingParams.TileRowStoreSelect;
        params.tileColumnStoreSelect                = m_curTileCodingParams.TileColumnStoreSelect;
        params.tileStartLCUX                        = m_curTileCodingParams.TileStartLCUX;
        params.tileStartLCUY                        = m_curTileCodingParams.TileStartLCUY;
        params.isLastTileofColumn                   = m_curTileCodingParams.IsLastTileofColumn;
        params.isLastTileofRow                      = m_curTileCodingParams.IsLastTileofRow;
        params.tileHeightInMinCbMinus1              = m_curTileCodingParams.TileHeightInMinCbMinus1;
        params.tileWidthInMinCbMinus1               = m_curTileCodingParams.TileWidthInMinCbMinus1;
        params.cuRecordOffset                       = m_curTileCodingParams.CuRecordOffset;
        params.bitstreamByteOffset                  = m_curTileCodingParams.BitstreamByteOffset;
        params.pakTileStatisticsOffset              = m_curTileCodingParams.PakTileStatisticsOffset;
        params.cuLevelStreamoutOffset               = m_curTileCodingParams.CuLevelStreamoutOffset;
        params.sliceSizeStreamoutOffset             = m_curTileCodingParams.SliceSizeStreamoutOffset;
        params.sseRowstoreOffset                    = m_curTileCodingParams.SseRowstoreOffset;
        params.saoRowstoreOffset                    = m_curTileCodingParams.SaoRowstoreOffset;
        params.tileSizeStreamoutOffset              = m_curTileCodingParams.TileSizeStreamoutOffset;
        params.vp9ProbabilityCounterStreamoutOffset = 0;
        params.nonFirstPassTile                     = m_curTileCodingParams.bTileReplayEnable && (!m_curTileCodingParams.IsFirstPass);
        params.bitstreamByteOffsetEnable            = m_curTileCodingParams.bTileReplayEnable;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, HevcEncodeTile)
    {
        ENCODE_FUNC_CALL();

        if (m_enabled && m_curTileCodingParams.NumberOfActiveBePipes > 1)
        {
            MOS_RESOURCE *tileStatisticsBuffer = const_cast<PMOS_RESOURCE>(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]);
            if (!Mos_ResourceIsNull(tileStatisticsBuffer))
            {
                params.presLcuBaseAddressBuffer     = tileStatisticsBuffer;
                params.dwLcuStreamOutOffset         = m_hevcTileStatsOffset.hevcSliceStreamout;
                params.presFrameStatStreamOutBuffer = tileStatisticsBuffer;
                params.dwFrameStatStreamOutOffset   = m_hevcTileStatsOffset.hevcPakStatistics;
            }
        }
        else
        {
            params.presLcuBaseAddressBuffer     = m_basicFeature->m_recycleBuf->GetBuffer(LcuBaseAddressBuffer, 0);
            params.presFrameStatStreamOutBuffer = m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_SLICE_STATE, HevcEncodeTile)
    {
        ENCODE_FUNC_CALL();

        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        params.lastSliceInTile       = hevcFeature->m_lastSliceInTile;
        params.lastSliceInTileColumn = (hevcFeature->m_lastSliceInTile & m_tileData[m_tileIdx].isLastTileofColumn) ? true : false;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, HevcEncodeTile)
    {
        ENCODE_FUNC_CALL();

        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        MOS_RESOURCE *tileRecordBuffer = const_cast<PMOS_RESOURCE>(&m_tileRecordBuffer[m_statisticsBufIndex]);
        if (!Mos_ResourceIsNull(tileRecordBuffer))
        {
            params.presPakTileSizeStasBuffer   = tileRecordBuffer;
            params.dwPakTileSizeStasBufferSize = m_hwInterface->m_tileRecordSize;
            params.dwPakTileSizeRecordOffset   = m_hevcTileStatsOffset.tileSizeRecord;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, HevcEncodeTile)
    {
        auto hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcBasicFeature);
        auto picParams = hevcBasicFeature->m_hevcPicParams;
        ENCODE_CHK_NULL_RETURN(picParams);
        auto seqParams = hevcBasicFeature->m_hevcSeqParams;
        ENCODE_CHK_NULL_RETURN(seqParams);
        auto sliceParams = hevcBasicFeature->m_hevcSliceParams;
        ENCODE_CHK_NULL_RETURN(sliceParams);

        uint32_t ctbSize          = 1 << (seqParams->log2_max_coding_block_size_minus3 + 3);
        uint32_t widthInPix       = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameWidthInMinCbMinus1 + 1);
        uint32_t heightInPix      = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameHeightInMinCbMinus1 + 1);
        uint32_t minCodingBlkSize = seqParams->log2_min_coding_block_size_minus3 + 3;
        params.ctbSize            = ctbSize;

        if (!m_enabled)
        {
            // No tiling support
            params.tileWidth  = widthInPix;
            params.tileHeight = heightInPix;
        }
        else
        {
            params.tileStartLCUX = (&m_curTileCodingParams)->TileStartLCUX;
            params.tileStartLCUY = (&m_curTileCodingParams)->TileStartLCUY;

            params.tileWidth  = (((&m_curTileCodingParams)->TileWidthInMinCbMinus1 + 1) << minCodingBlkSize);
            params.tileHeight = (((&m_curTileCodingParams)->TileHeightInMinCbMinus1 + 1) << minCodingBlkSize);

            // NumParEngine is not used by HW
            //cmd.DW3.NumParEngine = tileSlcParams->dwNumberOfPipes;

            params.tileId             = m_tileIdx;
            params.tileRowStoreSelect = (&m_curTileCodingParams)->TileRowStoreSelect;
            params.tileEnable         = 1;
            params.tileStreamInOffset = (&m_curTileCodingParams)->TileStreaminOffset;

            // PAK Object StreamOut Offset Computation
            uint32_t tileLCUStreamOutByteOffset = 0;
            if ((&m_curTileCodingParams)->TileStartLCUX != 0 || (&m_curTileCodingParams)->TileStartLCUY != 0)
            {
                uint32_t NumOfCUInLCU       = (ctbSize >> 3) * (ctbSize >> 3);  // Min CU size is 8
                uint32_t ImgWidthInLCU      = (widthInPix + ctbSize - 1) / ctbSize;
                uint32_t ImgHeightInLCU     = (heightInPix + ctbSize - 1) / ctbSize;
                uint32_t NumLCUsCurLocation = (&m_curTileCodingParams)->TileStartLCUY * ImgWidthInLCU + (&m_curTileCodingParams)->TileStartLCUX *
                                                                                                            (((((&m_curTileCodingParams)->TileHeightInMinCbMinus1 + 1) << minCodingBlkSize) + ctbSize - 1) / ctbSize);
                //For PAKObject Surface
                tileLCUStreamOutByteOffset = 2 * BYTES_PER_DWORD * NumLCUsCurLocation * (NUM_PAK_DWS_PER_LCU + NumOfCUInLCU * NUM_DWS_PER_CU);
                //Add 1 CL for size info at the beginning of each tile
                tileLCUStreamOutByteOffset += MHW_CACHELINE_SIZE;
                //CL alignment at end of every tile
                tileLCUStreamOutByteOffset = MOS_ROUNDUP_DIVIDE(tileLCUStreamOutByteOffset, MHW_CACHELINE_SIZE);
            }

            params.tileLCUStreamOutOffset = tileLCUStreamOutByteOffset;
            params.tileRowstoreOffset     = (params.tileStartLCUY == 0) ? (params.tileStartLCUX * params.ctbSize) / 32 : 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    }  // namespace encode
