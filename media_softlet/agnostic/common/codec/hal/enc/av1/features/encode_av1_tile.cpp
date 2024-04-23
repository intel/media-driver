/*
* Copyright (c) 2019-2023, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,Av1EncodeTile
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
//! \file     encode_av1_tile.cpp
//! \brief    Defines the common interface for av1 tile
//!

#include "encode_av1_tile.h"
#include "encode_av1_vdenc_feature_manager.h"
#include "codec_def_common_av1.h"
#include "encode_av1_basic_feature.h"

namespace encode
{
    VdencStatistics::VdencStatistics()
    {
        MOS_ZeroMemory(this, sizeof(*this));
    }

    Av1EncodeTile::Av1EncodeTile(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings) :
        EncodeTile(featureManager, allocator, hwInterface, constSettings)
    {
        auto encFeatureManager = dynamic_cast<EncodeAv1VdencFeatureManager *>(featureManager);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

        m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);
    }

    Av1EncodeTile::~Av1EncodeTile()
    {
        for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_reportTileGroupParams); i++)
        {
            MOS_FreeMemory(m_reportTileGroupParams[i]);
        }
    }

    MOS_STATUS Av1EncodeTile::Init(void *settings)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(settings);

        // Make these BB recycled
        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::Update(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        EncoderParams* encodeParams = (EncoderParams*)params;

        m_enabled = true;

        m_maxTileNumber = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, av1MinTileWidth) *
            CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, av1MinTileHeight);

        ENCODE_CHK_STATUS_RETURN(EncodeTile::Update(params));

        m_av1TileGroupParams = static_cast<PCODEC_AV1_ENCODE_TILE_GROUP_PARAMS>(encodeParams->pSliceParams);
        ENCODE_CHK_NULL_RETURN(m_av1TileGroupParams);
        m_numTileGroups = encodeParams->dwNumSlices;

        if (m_reportTileGroupParams[m_statisticsBufIndex] != nullptr)
        {
            MOS_FreeMemory(m_reportTileGroupParams[m_statisticsBufIndex]);
            m_reportTileGroupParams[m_statisticsBufIndex] = nullptr;
        }
        m_reportTileGroupParams[m_statisticsBufIndex] = static_cast<Av1ReportTileGroupParams *>(MOS_AllocAndZeroMemory(
            sizeof(Av1ReportTileGroupParams) * m_numTileGroups));

        auto av1PicParam = static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);

        // Check each tile size
        ENCODE_CHK_STATUS_RETURN(TileSizeCheck(av1PicParam));

        // For FrameOBU + single tile, tile group header will be empty in spec, so directly skip below function.
        if (!(av1PicParam->PicFlags.fields.EnableFrameOBU && m_numTiles == 1))
        {
            ENCODE_CHK_STATUS_RETURN(MakeTileGroupHeaderAv1(params));
        }

        // Setup tile group report data
        // Make this structure recycled
        ENCODE_CHK_STATUS_RETURN(SetTileGroupReportParams());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::IsFirstTileInGroup(bool &firstTileInGroup, uint32_t &tileGroupIdx) const
    {
        ENCODE_FUNC_CALL();

        auto tmpTileGroupParams = m_av1TileGroupParams;
        for (uint16_t tileGrupCnt = 0; tileGrupCnt < m_numTileGroups; tileGrupCnt++)
        {
            if (tmpTileGroupParams->TileGroupStart <= m_tileIdx &&
                tmpTileGroupParams->TileGroupEnd >= m_tileIdx)
            {
                firstTileInGroup = (tmpTileGroupParams->TileGroupStart == m_tileIdx);
                tileGroupIdx     = tileGrupCnt;
                break;
            }
            tmpTileGroupParams++;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::SetHucCtrlBuffer(VdencAv1HucCtrlBigData& hucCtrlBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_av1TileGroupParams);

        for (uint32_t i = 0; i < m_numTileGroups; i++)
        {
            hucCtrlBuffer.tileNumberPerGroup[i] = m_av1TileGroupParams[i].TileGroupEnd - m_av1TileGroupParams[i].TileGroupStart + 1;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::CalculateTilesBoundary(
        PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams,
        uint32_t *rowBd,
        uint32_t *colBd)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(av1PicParams);
        ENCODE_CHK_NULL_RETURN(rowBd);
        ENCODE_CHK_NULL_RETURN(colBd);

        for (uint32_t i = 0; i < m_numTileColumns; i++)
        {
            colBd[i + 1] = colBd[i] + av1PicParams->width_in_sbs_minus_1[i] + 1;
        }

        for (uint32_t i = 0; i < m_numTileRows; i++)
        {
            rowBd[i + 1] = rowBd[i] + av1PicParams->height_in_sbs_minus_1[i] + 1;
        }

        return MOS_STATUS_SUCCESS;

    }

    MOS_STATUS Av1EncodeTile::CalculateNumLcuByTiles(PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(av1PicParams);

        m_numSbInPic = 0;
        for (uint32_t i = 0; i < m_numTileRows; i++)
        {
            for (uint32_t j = 0; j < m_numTileColumns; j++)
            {
                m_numSbInPic += (av1PicParams->height_in_sbs_minus_1[i] + 1) * (av1PicParams->width_in_sbs_minus_1[j] + 1);
            }
        }

        if (m_numSbInPic == 0)
        {
            ENCODE_ASSERTMESSAGE("LCU num cal by each tile is zero, sth must be wrong!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::AllocateTileStatistics(void *params)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(params);

        MOS_ZeroMemory(&m_av1FrameStatsOffset, sizeof(Av1TileStatusInfo));
        MOS_ZeroMemory(&m_av1TileStatsOffset, sizeof(Av1TileStatusInfo));
        MOS_ZeroMemory(&m_av1StatsSize, sizeof(Av1TileStatusInfo));

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;

        // Set the maximum size based on frame level statistics.
        m_av1StatsSize.uiAv1PakStatistics  = EncodeBasicFeature::m_sizeOfHcpPakFrameStats;
        m_av1StatsSize.uiVdencStatistics    = m_av1VdencStateSize;

        // Maintain the offsets to use for patching addresses in to the HuC Pak Integration kernel Aggregated Frame Statistics Output Buffer
        // Each offset needs to be page aligned as the combined region is fed into different page aligned HuC regions
        m_av1FrameStatsOffset.uiAv1PakStatistics  = 0;
        m_av1FrameStatsOffset.uiVdencStatistics    = MOS_ALIGN_CEIL(m_av1FrameStatsOffset.uiAv1PakStatistics + m_av1StatsSize.uiAv1PakStatistics, CODECHAL_PAGE_SIZE);

        // Frame level statistics
        m_hwInterface->m_pakIntAggregatedFrameStatsSize = MOS_ALIGN_CEIL(m_av1FrameStatsOffset.uiVdencStatistics + (m_av1StatsSize.uiVdencStatistics * m_numTiles), CODECHAL_PAGE_SIZE);

        // AV1 Frame Statistics Buffer - Output from HuC PAK Integration kernel
        if (Mos_ResourceIsNull(&m_resHuCPakAggregatedFrameStatsBuffer))
        {
            allocParamsForBufferLinear.dwBytes  = m_hwInterface->m_pakIntAggregatedFrameStatsSize;
            allocParamsForBufferLinear.pBufName = "AVP Aggregated Frame Statistics Streamout Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;

            auto resource = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(resource);
            m_resHuCPakAggregatedFrameStatsBuffer = *resource;
        }

        // Maintain the offsets to use for patching addresses in to the Tile Based Statistics Buffer
        // Each offset needs to be page aligned as the combined region is fed into different page aligned HuC regions

        m_av1TileStatsOffset.uiAv1PakStatistics  = 0; // PakStatistics is head of m_resTileBasedStatisticsBuffer;
        m_av1TileStatsOffset.uiVdencStatistics    = MOS_ALIGN_CEIL(m_av1TileStatsOffset.uiAv1PakStatistics + (m_av1StatsSize.uiAv1PakStatistics * m_numTiles), CODECHAL_PAGE_SIZE);
        // Combined statistics size for all tiles
        m_hwInterface->m_pakIntTileStatsSize = MOS_ALIGN_CEIL(m_av1TileStatsOffset.uiVdencStatistics + m_av1StatsSize.uiVdencStatistics * m_numTiles, CODECHAL_PAGE_SIZE);

        // Tile size record size for all tiles
        m_hwInterface->m_tileRecordSize = CODECHAL_CACHELINE_SIZE * m_numTiles;

        MOS_SURFACE surface;
        MOS_ZeroMemory(&surface, sizeof(surface));
        uint32_t curPakIntTileStatsSize = 0;
        surface.OsResource = m_resTileBasedStatisticsBuffer[m_statisticsBufIndex];
        if (!Mos_ResourceIsNull(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]))
        {
            m_allocator->GetSurfaceInfo(&surface);
            curPakIntTileStatsSize = surface.dwHeight * surface.dwWidth;
        }

        if (Mos_ResourceIsNull(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]) ||
            curPakIntTileStatsSize < m_hwInterface->m_pakIntTileStatsSize)
        {
            if (!Mos_ResourceIsNull(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]))
            {
                m_allocator->DestroyResource(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]);
            }
            allocParamsForBufferLinear.dwBytes  = m_hwInterface->m_pakIntTileStatsSize;
            allocParamsForBufferLinear.pBufName = "AVP Tile Level Statistics Streamout Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;

            auto resource = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(resource);
            m_resTileBasedStatisticsBuffer[m_statisticsBufIndex] = *resource;
        }

        // Allocate the updated tile size buffer for PAK integration kernel
        if (Mos_ResourceIsNull(&m_tileRecordBuffer[m_statisticsBufIndex]))
        {
            allocParamsForBufferLinear.dwBytes  = m_hwInterface->m_tileRecordSize;
            allocParamsForBufferLinear.pBufName = "Tile Record Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;

            auto resource = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(resource);
            m_tileRecordBuffer[m_statisticsBufIndex] = *resource;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::CalculateTileWidthAndHeight(
        PCODEC_AV1_ENCODE_PICTURE_PARAMS  av1PicParams,
        uint32_t  rowIndex,
        uint32_t  colIndex,
        uint32_t *rowBd,
        uint32_t *colBd)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(av1PicParams);
        ENCODE_CHK_NULL_RETURN(rowBd);
        ENCODE_CHK_NULL_RETURN(colBd);

        uint32_t idx                = rowIndex * m_numTileColumns + colIndex;
        uint32_t frameWidthInMinCb  = MOS_ROUNDUP_DIVIDE((av1PicParams->frame_width_minus1 + 1), av1MinBlockWidth);
        uint32_t frameHeightInMinCb = MOS_ROUNDUP_DIVIDE((av1PicParams->frame_height_minus1 + 1), av1MinBlockHeight);
        uint32_t ratio              = av1SuperBlockHeight / av1MinBlockHeight;

        if (colIndex != m_numTileColumns - 1)
        {
            m_tileData[idx].tileWidthInMinCbMinus1 = (av1PicParams->width_in_sbs_minus_1[colIndex] + 1) * ratio - 1;
            m_tileData[idx].isLastTileofRow        = false;
        }
        else
        {
            m_tileData[idx].tileWidthInMinCbMinus1 = (frameWidthInMinCb - colBd[colIndex] * ratio) - 1;
            m_tileData[idx].isLastTileofRow        = true;
        }

        if (rowIndex != m_numTileRows - 1)
        {
            m_tileData[idx].isLastTileofColumn      = false;
            m_tileData[idx].tileHeightInMinCbMinus1 = (av1PicParams->height_in_sbs_minus_1[rowIndex] +1 ) * ratio - 1;
        }
        else
        {
            m_tileData[idx].tileHeightInMinCbMinus1 = (frameHeightInMinCb - rowBd[rowIndex] * ratio) - 1;
            m_tileData[idx].isLastTileofColumn      = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::SetTileData(void *params)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus       = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }

        ENCODE_CHK_NULL_RETURN(params);

        EncoderParams* encodeParams = (EncoderParams*)params;
        ENCODE_CHK_NULL_RETURN(encodeParams);

        PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams =
        static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(av1PicParams);

        m_numTileRows    = av1PicParams->tile_rows;
        m_numTileColumns = av1PicParams->tile_cols;

        uint32_t colBd[m_maxTileBdNum] = {0};
        uint32_t rowBd[m_maxTileBdNum] = {0};
        ENCODE_CHK_STATUS_RETURN(CalculateTilesBoundary(av1PicParams, &rowBd[0], &colBd[0]));

        m_numTiles = m_numTileRows * m_numTileColumns;
        if (m_numTiles > CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, av1SuperBlockWidth) *
            CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, av1SuperBlockHeight))
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        uint32_t streamInWidthinSb              = MOS_ROUNDUP_DIVIDE((av1PicParams->frame_width_minus1 +1), av1SuperBlockWidth);
        uint32_t bitstreamByteOffset            = 0;
        uint32_t saoRowstoreOffset              = 0;
        uint32_t cuLevelStreamoutOffset         = 0;
        uint32_t sseRowstoreOffset              = 0;
        uint32_t tileStartSbAddr                = 0;
        uint32_t cumulativeCUTileOffsetInBytes  = 0;
        uint32_t tileLCUStreamOutOffsetInBytes  = 0;

        ENCODE_CHK_STATUS_RETURN(CalculateNumLcuByTiles(av1PicParams));

        for ( uint32_t i = 0; i < m_numTileRows; i++)
        {
            for (uint32_t j = 0; j < m_numTileColumns; j++)
            {
                uint32_t idx          = i * m_numTileColumns + j;
                uint32_t numLcuInTile = (av1PicParams->height_in_sbs_minus_1[i] + 1) *
                    (av1PicParams->width_in_sbs_minus_1[j] + 1);

                m_tileData[idx].tileStartXInSb = colBd[j];
                m_tileData[idx].tileStartYInSb = rowBd[i];

                //m_tileData[idx].tileColumnStoreSelect = j % 2;
                //m_tileData[idx].tileRowStoreSelect    = i % 2;

                ENCODE_CHK_STATUS_RETURN(CalculateTileWidthAndHeight(av1PicParams, i, j, &rowBd[0], &colBd[0]));

                m_tileData[idx].numOfTilesInFrame       = m_numTiles;
                m_tileData[idx].numOfTileColumnsInFrame = m_numTileColumns;

                m_tileData[idx].pakTileStatisticsOffset              = 8 * idx;
                m_tileData[idx].tileSizeStreamoutOffset              = idx;
                m_tileData[idx].vp9ProbabilityCounterStreamoutOffset = 0;

                m_tileData[idx].cumulativeCUTileOffset   = cumulativeCUTileOffsetInBytes / CODECHAL_CACHELINE_SIZE;
                m_tileData[idx].tileLCUStreamOutOffset   = tileLCUStreamOutOffsetInBytes / CODECHAL_CACHELINE_SIZE;
                m_tileData[idx].cuLevelStreamoutOffset   = cuLevelStreamoutOffset;
                m_tileData[idx].sseRowstoreOffset        = sseRowstoreOffset;
                m_tileData[idx].bitstreamByteOffset      = bitstreamByteOffset;
                m_tileData[idx].saoRowstoreOffset        = saoRowstoreOffset;

                uint32_t tileHeightInSb     = MOS_ROUNDUP_DIVIDE(((m_tileData[idx].tileHeightInMinCbMinus1 + 1) * av1MinBlockHeight), av1SuperBlockHeight);
                uint32_t tileWidthInSb      = MOS_ROUNDUP_DIVIDE(((m_tileData[idx].tileWidthInMinCbMinus1 + 1) * av1MinBlockWidth), av1SuperBlockWidth);

                m_tileData[idx].tileEndXInSb  = m_tileData[idx].tileStartXInSb + tileWidthInSb;
                m_tileData[idx].tileEndYInSb  = m_tileData[idx].tileStartYInSb + tileHeightInSb;

                // StreamIn data is 4 CLs per LCU
                m_tileData[idx].tileStreaminOffset       = 4 * (m_tileData[idx].tileStartYInSb * streamInWidthinSb + m_tileData[idx].tileStartXInSb * tileHeightInSb);
                m_tileData[idx].sliceSizeStreamoutOffset = tileStartSbAddr; // correct sliceSizeStreamoutOffset calculation

                uint32_t numOfSbsInTile = tileWidthInSb * tileHeightInSb;
                tileStartSbAddr += numOfSbsInTile;

                // For Cumulative CU Count : 2 bytes per LCU
                cumulativeCUTileOffsetInBytes += numOfSbsInTile * 2;
                cumulativeCUTileOffsetInBytes  = MOS_ALIGN_CEIL(cumulativeCUTileOffsetInBytes, CODECHAL_CACHELINE_SIZE);

                // Max LCU/SB size is 64, Min CU size is 8
                const uint32_t maxNumOfCUInLCU = (av1SuperBlockWidth / av1MinBlockWidth) * (av1SuperBlockHeight / av1MinBlockHeight);

                // For PAKObject Surface, 1 DW for PAK OBJ DATA + 1 DW for SIDE BAND INFORMATION 
                tileLCUStreamOutOffsetInBytes += 2 * BYTES_PER_DWORD * numOfSbsInTile * (NUM_PAK_DWS_PER_LCU + maxNumOfCUInLCU * NUM_DWS_PER_CU);
                // CL alignment at end of every tile
                tileLCUStreamOutOffsetInBytes = MOS_ALIGN_CEIL(tileLCUStreamOutOffsetInBytes, CODECHAL_CACHELINE_SIZE);

                cuLevelStreamoutOffset += (m_tileData[idx].tileWidthInMinCbMinus1 + 1) * (m_tileData[idx].tileHeightInMinCbMinus1 + 1) * 16 / CODECHAL_CACHELINE_SIZE;

                sseRowstoreOffset += (((av1PicParams->width_in_sbs_minus_1[j] + 1) + 3) * ((Av1BasicFeature*)m_basicFeature)->m_sizeOfSseSrcPixelRowStoreBufferPerLcu) / CODECHAL_CACHELINE_SIZE;

                uint64_t totalSizeTemp        = (uint64_t)m_basicFeature->m_bitstreamSize * (uint64_t)numLcuInTile;
                uint32_t bitStreamSizePerTile = (uint32_t)(totalSizeTemp / (uint64_t)m_numSbInPic) + ((totalSizeTemp % (uint64_t)m_numSbInPic) ? 1 : 0);
                bitstreamByteOffset += MOS_ALIGN_CEIL(bitStreamSizePerTile, CODECHAL_CACHELINE_SIZE) / CODECHAL_CACHELINE_SIZE;
            }

            // same row store buffer for different tile rows.
            saoRowstoreOffset = 0;
            sseRowstoreOffset = 0;
        }

        return eStatus;
    }

    MOS_STATUS Av1EncodeTile::SetTileGroupReportParams()
    {
        ENCODE_FUNC_CALL();

        for (uint32_t i = 0; i < m_numTileGroups; i++)
        {
            m_reportTileGroupParams[m_statisticsBufIndex][i].TileGroupStart = m_av1TileGroupParams[i].TileGroupStart;
            m_reportTileGroupParams[m_statisticsBufIndex][i].TileGroupEnd = m_av1TileGroupParams[i].TileGroupEnd;
            m_reportTileGroupParams[m_statisticsBufIndex][i].TileGroupNum = m_numTileGroups;

            auto av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
            ENCODE_CHK_NULL_RETURN(av1BasicFeature);

            m_reportTileGroupParams[m_statisticsBufIndex][i].FirstTileGroupByteOffset = av1BasicFeature->GetAppHdrSizeInBytes();
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::GetTileGroupReportParams(uint32_t idx, const Av1ReportTileGroupParams *&reportTileGroupParams)
    {
        ENCODE_FUNC_CALL();

        if (idx >= EncodeBasicFeature::m_uncompressedSurfaceNum)
        {
            ENCODE_ASSERTMESSAGE("Index exceeds the max number, when try to get tile report data");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        reportTileGroupParams = m_reportTileGroupParams[idx];

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::ReadObuSize(const uint8_t *ObuOffset, uint32_t &size)
    {
        ENCODE_CHK_NULL_RETURN(ObuOffset);

        uint32_t value = 0;
        for (uint32_t i = 0; i < OBU_LEB128_SIZE; ++i)
        {
            const uint8_t cur_byte     = static_cast<uint8_t>(ObuOffset[i]);
            const uint8_t decoded_byte = cur_byte & LEB128_BYTE_MASK;
            value |= ((uint64_t)decoded_byte) << (i * 7);
            if ((cur_byte & ~LEB128_BYTE_MASK) == 0)
            {
                size = value;
                break;
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    uint16_t Av1EncodeTile::CeilLog2(uint16_t x)
    {
        uint16_t k = 0;
        while (x > ((uint16_t)(1 << k))) k++;
        return k;
    }

    uint16_t Av1EncodeTile::TileLog2(uint16_t blksize, uint16_t target)
    {
        uint16_t k = 0;
        while ((uint16_t)(blksize << k) < target) k++;
        return k;
    }

    MOS_STATUS Av1EncodeTile::TileSizeCheck(const PCODEC_AV1_ENCODE_PICTURE_PARAMS &av1PicParam)
    {
        uint32_t maxTileAreaSb = MOS_ROUNDUP_DIVIDE(av1MaxTileArea, av1SuperBlockWidth * av1SuperBlockHeight);
        uint32_t sbCols        = MOS_ROUNDUP_DIVIDE(av1PicParam->frame_width_minus1 + 1, av1SuperBlockWidth);
        uint32_t sbRows        = MOS_ROUNDUP_DIVIDE(av1PicParam->frame_height_minus1 + 1, av1SuperBlockHeight);

        //Tile Width sum equals image width, no pixel leak
        if (av1PicParam->width_in_sbs_minus_1[0] + 1 == 0)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
        uint32_t curTileWidthSb = av1PicParam->width_in_sbs_minus_1[0] + 1;
        uint32_t widestTileSb   = av1PicParam->width_in_sbs_minus_1[0] + 1;
        uint32_t tileWidthSbSum = 0;

        if (m_basicFeature->m_dualEncEnable && m_numTileRows != 1)
        {
            ENCODE_ASSERTMESSAGE("dual encode cannot support multi rows submission yet.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        for (uint8_t i = 0; i < m_numTileColumns; i++)
        {
            curTileWidthSb = av1PicParam->width_in_sbs_minus_1[i] + 1;
            widestTileSb   = MOS_MAX(widestTileSb, curTileWidthSb);
            tileWidthSbSum += curTileWidthSb;
            if (m_basicFeature->m_dualEncEnable && curTileWidthSb == 2)
            {
                m_firstDummyIdx = i;
            }
        }
        if (tileWidthSbSum != sbCols)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        //Tile Height sum equals image height, no pixel leak
        if (av1PicParam->height_in_sbs_minus_1[0] + 1 == 0)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
        uint32_t curTileHeightSb = av1PicParam->height_in_sbs_minus_1[0] + 1;
        uint32_t highestWidthSb  = av1PicParam->height_in_sbs_minus_1[0] + 1;
        uint32_t tileHeightSbSum = 0;

        for (uint8_t i = 0; i < m_numTileRows; i++)
        {
            curTileHeightSb = av1PicParam->height_in_sbs_minus_1[i] + 1;
            highestWidthSb  = MOS_MAX(1, curTileHeightSb);
            tileHeightSbSum += curTileHeightSb;
        }
        if (tileHeightSbSum != sbRows)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Max tile check

        for (uint8_t i = 0; i < m_numTileRows; i++)
        {
            curTileHeightSb = av1PicParam->height_in_sbs_minus_1[i] + 1;
            if ((widestTileSb * curTileHeightSb) > maxTileAreaSb)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    void Av1EncodeTile::WriteObuHeader(Av1TileGroupHeaderInfo &buf, const PCODEC_AV1_ENCODE_PICTURE_PARAMS &av1PicParam)
    {
        if (av1PicParam == nullptr)
        {
            return;
        }

        auto ObuExtensionFlag = av1PicParam->TileGroupOBUHdrInfo.fields.obu_extension_flag;
        auto ObuHasSizeField  = av1PicParam->TileGroupOBUHdrInfo.fields.obu_has_size_field;
        WriteBit(buf, 0);                 //forbidden bit
        WriteLiteral(buf, 4, 4);          //type
        WriteBit(buf, ObuExtensionFlag);  //obu_extension
        WriteBit(buf, ObuHasSizeField);   //obu_has_size_field
        WriteBit(buf, 0);                 //reserved

        //If obu_extension == 1, write OBU extension header
        if (ObuExtensionFlag)
        {
            uint8_t temporalId = av1PicParam->TileGroupOBUHdrInfo.fields.temporal_id;
            uint8_t spatialId  = av1PicParam->TileGroupOBUHdrInfo.fields.spatial_id;
            WriteLiteral(buf, temporalId, 3);   //temporal_id
            WriteLiteral(buf, spatialId,  2);   //spatial_id
            WriteLiteral(buf, 0, 3);            //reserved
        }
    }

    // in_value - [in] integer value to encoded as leb128
    // out_encoded_value - [out] encoded data to be added to a bitstream
    // fixed_output_len - [optional in] fixed len for the output, WA for driver part (value = len, 0 - not used)
    // return - encoded len
    uint8_t Av1EncodeTile::write_leb128(uint64_t in_value, uint64_t &out_encoded_value, const uint8_t fixed_output_len)
    {
        uint8_t *out_buf         = reinterpret_cast<uint8_t *>(&out_encoded_value);
        uint8_t  out_bytes_count = 0;

        if (!fixed_output_len)
        {
            // general encoding
            do
            {
                out_buf[out_bytes_count] = in_value & 0x7fU;
                if (in_value >>= 7)
                {
                    out_buf[out_bytes_count] |= 0x80U;
                }
                out_bytes_count++;
            } while (in_value);
        }
        else
        {
            // WA to get fixed len of output
            uint8_t value_byte_count = 0;
            do
            {
                out_buf[value_byte_count++] = in_value & 0x7fU;
                in_value >>= 7;
            } while (in_value);
            for (int i = 0; i < fixed_output_len - 1; i++)
            {
                out_buf[i] |= 0x80U;
                out_bytes_count++;
            }
            out_bytes_count++;
        }

        return out_bytes_count;
    }

    void Av1EncodeTile::WriteBit(Av1TileGroupHeaderInfo &buf, uint8_t bit)
    {
        const uint16_t byteOffset     = buf.bitOffset / 8;
        const uint8_t  bitsLeftInByte = 8 - 1 - buf.bitOffset % 8;
        if (bitsLeftInByte == 8 - 1)
        {
            buf.pBuffer[byteOffset] = uint8_t(bit << bitsLeftInByte);
        }
        else
        {
            buf.pBuffer[byteOffset] &= ~(1 << bitsLeftInByte);
            buf.pBuffer[byteOffset] |= bit << bitsLeftInByte;
        }
        buf.bitOffset = buf.bitOffset + 1;
    }

    void Av1EncodeTile::WriteLiteral(Av1TileGroupHeaderInfo &buf, uint64_t data, uint64_t bits)
    {
        for (int64_t bit = bits - 1; bit >= 0; bit--)
        {
            WriteBit(buf, (data >> bit) & 1);
        }
    }

    uint16_t Av1EncodeTile::PrepareTileGroupHeaderAv1(uint8_t *buffer, uint32_t index, const PCODEC_AV1_ENCODE_PICTURE_PARAMS &av1PicParam)
    {
        Av1TileGroupHeaderInfo localBuf;
        localBuf.pBuffer   = buffer;
        localBuf.bitOffset = 0;

        if (av1PicParam == nullptr)
        {
            ENCODE_ASSERTMESSAGE("Invalid (NULL) Pointer!");
            return 0;
        }
        bool isFrameObu = av1PicParam->PicFlags.fields.EnableFrameOBU;
        // if frame OBU enabled, should skip the generic OBU syntax, including OBU header and OBU size.
        if (!isFrameObu)
        {
            WriteObuHeader(localBuf, av1PicParam);

            uint64_t encoded_bytes = 0;
            // SAS demands to set fixed-len size (0) for TG OBU for back annotation
            uint32_t encoded_len = write_leb128(0, encoded_bytes, 4);
            for (uint8_t i = 0; i < encoded_len; i++)
            {
                const uint8_t* ptr = reinterpret_cast<uint8_t*>(&encoded_bytes);
                WriteLiteral(localBuf, ptr[i], 8);
            }
        }

        const uint32_t NumTiles = m_numTileRows * m_numTileColumns;
        auto tmpTileGroupParams = m_av1TileGroupParams + index;
        uint8_t  tile_start_and_end_present_flag = m_numTileGroups == 1 ? 0 : 1;
        uint32_t tg_start, tg_end, tempOffset = 0;

        tempOffset = localBuf.bitOffset;

        if (NumTiles > 1)
        {
            WriteBit(localBuf, tile_start_and_end_present_flag);
        }
        if (NumTiles == 1 || !tile_start_and_end_present_flag)
        {
            tg_start = 0;
            tg_end   = NumTiles - 1;
        }
        else
        {
            uint32_t tileBits = static_cast<uint8_t>(CeilLog2(m_numTileRows)) + static_cast<uint8_t>(CeilLog2(m_numTileColumns));
            WriteLiteral(localBuf, tmpTileGroupParams->TileGroupStart, tileBits);
            WriteLiteral(localBuf, tmpTileGroupParams->TileGroupEnd, tileBits);
        }

        //byte_alignment
        while (localBuf.bitOffset & 7)
            WriteBit(localBuf, 0);  //zero_bit

        // HW keeps filling bitstream from this point

        // tile group OBU size should exclude general OBU syntext related size.
        m_reportTileGroupParams[m_statisticsBufIndex][index].TileGroupOBUSizeInBytes = (localBuf.bitOffset - tempOffset)/8;

        auto av1BasicFeature = dynamic_cast<Av1BasicFeature*>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(av1BasicFeature);
        av1BasicFeature->m_tileGroupHeaderSize = (localBuf.bitOffset + 7) / 8;

        return av1BasicFeature->m_tileGroupHeaderSize;
    }

    MOS_STATUS Av1EncodeTile::MakeTileGroupHeaderAv1(void *params)
    {
        ENCODE_CHK_NULL_RETURN(params);

        EncoderParams *encodeParams = (EncoderParams *)params;
        ENCODE_CHK_NULL_RETURN(encodeParams);

        auto av1PicParam = static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
        ENCODE_CHK_NULL_RETURN(av1PicParam);

        uint32_t  slcCount = 0;
        BSBuffer *pBSBuffer;
        uint32_t  hdrDataOffset, hdrDataSize, hdrBufSize, hdrDataByteSize;

        uint8_t *temp = new uint8_t[256];

        for (uint32_t i = 0; i < m_numTileGroups; i++)
        {
            uint16_t bytesWrittenTgHeader = PrepareTileGroupHeaderAv1(temp, i, av1PicParam);
            hdrDataSize = hdrBufSize = bytesWrittenTgHeader * 8;  // In bits
            hdrDataByteSize          = (hdrDataSize + 7) >> 3;    // In bytes
            hdrDataOffset            = 0;

            if (hdrDataOffset >= hdrBufSize)
            {
                ENCODE_ASSERTMESSAGE("Encode: Data offset in packed slice header data is out of bounds.");
                delete []temp;
                return MOS_STATUS_INVALID_FILE_SIZE;
            }

            if (hdrDataByteSize > hdrBufSize)
            {
                ENCODE_ASSERTMESSAGE("Encode: Data length in packed slice header data is greater than buffer size.");
                delete []temp;
                return MOS_STATUS_INVALID_FILE_SIZE;
            }

            PCODEC_ENCODER_SLCDATA pSlcData  = m_basicFeature->m_slcData;
            pBSBuffer                        = &m_basicFeature->m_bsBuffer;
            pSlcData[slcCount].SliceOffset = (uint32_t)(pBSBuffer->pCurrent - pBSBuffer->pBase);
            pSlcData[slcCount].BitSize     = hdrDataSize;
            slcCount++;

            if (slcCount > encodeParams->dwNumSlices)
            {
                ENCODE_ASSERTMESSAGE("Encode: Number of slice headers exceeds number of slices.");
                delete []temp;
                return MOS_STATUS_INVALID_FILE_SIZE;
            }

            MOS_SecureMemcpy(pBSBuffer->pCurrent,
                hdrDataByteSize,
                (temp + hdrDataOffset),
                hdrDataByteSize);

            pBSBuffer->pCurrent += hdrDataByteSize;
        }
        delete []temp;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::GetTileStatsOffset(uint32_t &offset)
    {
        offset = m_av1TileStatsOffset.uiVdencStatistics;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1EncodeTile::GetTileInfo(Av1TileInfo *av1TileInfo) const
    {
        ENCODE_CHK_NULL_RETURN(av1TileInfo);

        av1TileInfo->tileId = static_cast<uint16_t>(m_tileIdx);  // Tile number in a frame

        const auto currTileData = m_tileData[m_tileIdx];

        av1TileInfo->tileColPositionInSb = static_cast<uint16_t>(currTileData.tileStartXInSb);
        av1TileInfo->tileRowPositionInSb = static_cast<uint16_t>(currTileData.tileStartYInSb);

        av1TileInfo->tileWidthInSbMinus1 = currTileData.tileEndXInSb - currTileData.tileStartXInSb - 1;
        av1TileInfo->tileHeightInSbMinus1 = currTileData.tileEndYInSb - currTileData.tileStartYInSb - 1;

        av1TileInfo->firstTileInAFrame = (0 == m_tileIdx);
        av1TileInfo->lastTileOfColumn  = currTileData.isLastTileofColumn;
        av1TileInfo->lastTileOfRow     = currTileData.isLastTileofRow;
        av1TileInfo->lastTileOfFrame   = (m_tileIdx == currTileData.numOfTilesInFrame - 1) ? true : false;

        av1TileInfo->tileStartXInLCU = currTileData.tileStartXInLCU;
        av1TileInfo->tileStartYInLCU = currTileData.tileStartYInLCU;
        av1TileInfo->tileEndXInLCU   = currTileData.tileEndXInLCU;
        av1TileInfo->tileEndYInLCU   = currTileData.tileEndYInLCU;

        auto tmpTileGroupParams = m_av1TileGroupParams;
        for (uint16_t tileGrupCnt = 0; tileGrupCnt < m_numTileGroups; tileGrupCnt++)
        {
            if (tmpTileGroupParams->TileGroupStart <= m_tileIdx &&
                tmpTileGroupParams->TileGroupEnd >= m_tileIdx)
            {
                av1TileInfo->firstTileOfTileGroup = (tmpTileGroupParams->TileGroupStart == m_tileIdx);
                av1TileInfo->lastTileOfTileGroup  = (tmpTileGroupParams->TileGroupEnd == m_tileIdx);
                av1TileInfo->tgTileNum            = m_tileIdx - tmpTileGroupParams->TileGroupStart;
                av1TileInfo->tileGroupId          = tileGrupCnt;
                break;
            }
            tmpTileGroupParams++;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Av1EncodeTile)
    {
        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        MOS_RESOURCE* tileStatisticsBuffer = const_cast<MOS_RESOURCE*>(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]);
        if (!Mos_ResourceIsNull(tileStatisticsBuffer))
        {
            params.streamOutBuffer = tileStatisticsBuffer;
            params.streamOutOffset = m_av1TileStatsOffset.uiVdencStatistics;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_WALKER_STATE, Av1EncodeTile)
    {
        params.tileSliceStartLcuMbX = m_tileData[m_tileIdx].tileStartXInLCU;
        params.tileSliceStartLcuMbY = m_tileData[m_tileIdx].tileStartYInLCU;
        params.nextTileSliceStartLcuMbX = m_tileData[m_tileIdx].tileEndXInLCU;
        params.nextTileSliceStartLcuMbY = m_tileData[m_tileIdx].tileEndYInLCU;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Av1EncodeTile)
    {
        params.tileEnable = true;
        params.tileId = m_tileIdx;
        params.ctbSize = av1SuperBlockWidth;
        params.tileRowStoreSelect = m_curTileCodingParams.TileRowStoreSelect;
        params.tileWidth = (m_curTileCodingParams.TileWidthInMinCbMinus1 + 1) * av1MinBlockWidth;
        params.tileHeight = (m_curTileCodingParams.TileHeightInMinCbMinus1 + 1) * av1MinBlockWidth;
        params.tileStartLCUX = m_curTileCodingParams.TileStartLCUX;
        params.tileStartLCUY = m_curTileCodingParams.TileStartLCUY;
        params.tileStreamInOffset = m_curTileCodingParams.TileStreaminOffset;
        params.tileLCUStreamOutOffset = m_curTileCodingParams.TileLCUStreamOutOffset;
        params.VdencHEVCVP9TileSlicePar18 = true;
        params.VdencHEVCVP9TileSlicePar19 = m_curTileCodingParams.CumulativeCUTileOffset;
        params.tileRowstoreOffset = 0;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1EncodeTile)
    {
        bool     firstTileInGroup = false;
        uint32_t tileGroupIdx     = 0;

        IsFirstTileInGroup(firstTileInGroup, tileGroupIdx);

        params.headerPresent = firstTileInGroup;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIPE_MODE_SELECT, Av1EncodeTile)
    {
        params.tileBasedReplayMode = false;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_TILE_CODING, Av1EncodeTile)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        Av1TileInfo av1TileInfo;
        eStatus = GetTileInfo(&av1TileInfo);
        params.tileId               = av1TileInfo.tileId;
        params.tileColPositionInSb  = av1TileInfo.tileColPositionInSb;
        params.tileRowPositionInSb  = av1TileInfo.tileRowPositionInSb;
        params.tileWidthInSbMinus1  = av1TileInfo.tileWidthInSbMinus1;
        params.tileHeightInSbMinus1 = av1TileInfo.tileHeightInSbMinus1;
        params.firstTileInAFrame    = av1TileInfo.firstTileInAFrame;
        params.lastTileOfColumn     = av1TileInfo.lastTileOfColumn;
        params.lastTileOfRow        = av1TileInfo.lastTileOfRow;
        params.lastTileOfFrame      = av1TileInfo.lastTileOfFrame;
        params.firstTileOfTileGroup = av1TileInfo.firstTileOfTileGroup;
        params.lastTileOfTileGroup  = av1TileInfo.lastTileOfTileGroup;
        params.tgTileNum            = av1TileInfo.tgTileNum;
        params.tileGroupId          = av1TileInfo.tileGroupId;

        return eStatus;
    }

    MHW_SETPAR_DECL_SRC(AVP_IND_OBJ_BASE_ADDR_STATE, Av1EncodeTile)
    {
        auto basicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(basicFeature);

        if (basicFeature->m_enableSWStitching || basicFeature->m_dualEncEnable)
        {
            params.pakBaseObjectOffset = MOS_ALIGN_CEIL(m_tileData[m_tileIdx].bitstreamByteOffset * CODECHAL_CACHELINE_SIZE, MOS_PAGE_SIZE);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PAK_INSERT_OBJECT, Av1EncodeTile)
    {
        bool                                firstTileInGroup = false;
        uint32_t                            tileGroupIdx     = 0;

        IsFirstTileInGroup(firstTileInGroup, tileGroupIdx);

        if (firstTileInGroup)
        {
            params.bsBuffer             = &m_basicFeature->m_bsBuffer;
            params.bitSize              = m_basicFeature->m_slcData[tileGroupIdx].BitSize;
            params.offset               = m_basicFeature->m_slcData[tileGroupIdx].SliceOffset;
            params.endOfHeaderInsertion = false;
            params.lastHeader           = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIPE_BUF_ADDR_STATE, Av1EncodeTile)
    {
        params.tileSizeStreamoutBuffer       = const_cast<PMOS_RESOURCE>(&m_tileRecordBuffer[m_statisticsBufIndex]);
        params.tileSizeStreamoutBufferOffset = m_tileIdx * CODECHAL_CACHELINE_SIZE;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1EncodeTile)
    {
        ENCODE_FUNC_CALL();
        auto av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(av1BasicFeature);
        auto av1Picparams = av1BasicFeature->m_av1PicParams;
        ENCODE_CHK_NULL_RETURN(av1Picparams);
        auto av1Seqparams = av1BasicFeature->m_av1SeqParams;
        ENCODE_CHK_NULL_RETURN(av1Seqparams);

        // for BRC, adaptive rounding is done in HuC, so we can skip it here.
        if (av1BasicFeature->m_roundingMethod == RoundingMethod::adaptiveRounding && !IsRateControlBrc(av1Seqparams->RateControlMethod))
        {
            uint32_t frameSizeIn8x8Units1 = ((av1BasicFeature->m_oriFrameWidth + 63) >> 6) * ((av1BasicFeature->m_oriFrameHeight + 63) >> 6);
            uint32_t frameSizeIn8x8Units2 = ((av1BasicFeature->m_oriFrameWidth + 7) >> 3) * ((av1BasicFeature->m_oriFrameHeight + 7) >> 3);
            av1BasicFeature->m_par65Inter = 2;

            if (av1BasicFeature->m_encodedFrameNum > 0)
            {
                MOS_RESOURCE* buf = nullptr;

                ENCODE_CHK_STATUS_RETURN(GetTileBasedStatisticsBuffer(m_prevStatisticsBufIndex, buf));
                ENCODE_CHK_NULL_RETURN(buf);

                //will be optimized to avoid perf degradation when async > 1
                const auto* bufData = (VdencStatistics*)((uint8_t*)m_allocator->LockResourceForRead(buf) + m_av1TileStatsOffset.uiVdencStatistics);
                ENCODE_CHK_NULL_RETURN(bufData);

                uint32_t newFCost1 = bufData->DW1.IntraCuCountNormalized / frameSizeIn8x8Units1;

                m_allocator->UnLock(buf);

                if (newFCost1 >= 2)
                {
                    av1BasicFeature->m_par65Inter = 3;
                }
                else if (newFCost1 == 0)
                {
                    av1BasicFeature->m_par65Inter = 1;
                }
            }

            av1BasicFeature->m_par65Intra = 7;

            if (av1Picparams->base_qindex <= 100 || frameSizeIn8x8Units2 < 5000)
            {
                av1BasicFeature->m_par65Intra = 6;
            }

            if (av1Picparams->PicFlags.fields.frame_type != keyFrame)
            {
                av1BasicFeature->m_par65Intra = 5;
            }
        }
        else if (av1BasicFeature->m_roundingMethod == RoundingMethod::fixedRounding)
        {
            av1BasicFeature->m_par65Inter = 2;
            av1BasicFeature->m_par65Intra = 6;
        }

        if (av1BasicFeature->m_roundingMethod == RoundingMethod::adaptiveRounding
            || av1BasicFeature->m_roundingMethod == RoundingMethod::fixedRounding)
        {
#if _MEDIA_RESERVED
            for (auto i = 0; i < 3; i++)
            {
                params.vdencCmd2Par65[i][0][0] = av1BasicFeature->m_par65Intra;
                params.vdencCmd2Par65[i][0][1] = av1BasicFeature->m_par65Intra;
                params.vdencCmd2Par65[i][1][0] = av1BasicFeature->m_par65Inter;
                params.vdencCmd2Par65[i][1][1] = av1BasicFeature->m_par65Inter;
                params.vdencCmd2Par65[i][2][0] = av1BasicFeature->m_par65Inter;
                params.vdencCmd2Par65[i][2][1] = av1BasicFeature->m_par65Inter;
            }
#else
            params.extSettings.emplace_back(
                [av1BasicFeature](uint32_t *data) {
                    uint8_t tmp0 = av1BasicFeature->m_par65Intra & 0xf;
                    uint8_t tmp1 = av1BasicFeature->m_par65Inter & 0xf;

                    data[32] |= (tmp1 << 16);
                    data[32] |= (tmp1 << 20);
                    data[32] |= (tmp0 << 24);
                    data[32] |= (tmp0 << 28);

                    data[33] |= tmp1;
                    data[33] |= (tmp1 << 4);
                    data[33] |= (tmp1 << 8);
                    data[33] |= (tmp1 << 12);
                    data[33] |= (tmp0 << 16);
                    data[33] |= (tmp0 << 20);
                    data[33] |= (tmp1 << 24);
                    data[33] |= (tmp1 << 28);

                    data[34] |= tmp1;
                    data[34] |= (tmp1 << 4);
                    data[34] |= (tmp0 << 8);
                    data[34] |= (tmp0 << 12);
                    data[34] |= (tmp1 << 16);
                    data[34] |= (tmp1 << 20);

                    return MOS_STATUS_SUCCESS;
                });
#endif  // _MEDIA_RESERVED  
        }


        return MOS_STATUS_SUCCESS;
    }

MOS_STATUS Av1EncodeTile::GetTileStatusInfo(
        Av1TileStatusInfo &av1TileStatsOffset,
        Av1TileStatusInfo &av1StatsSize)
    {
        av1TileStatsOffset = m_av1TileStatsOffset;
        av1StatsSize = m_av1StatsSize;
        
        return MOS_STATUS_SUCCESS;
    }

    }  // namespace encode
