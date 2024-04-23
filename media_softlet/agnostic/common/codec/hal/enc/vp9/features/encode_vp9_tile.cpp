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
//! \file     encode_vp9_tile.cpp
//! \brief    Defines the common interface for vp9 tile
//!

#include "encode_vp9_tile.h"
#include "encode_vp9_basic_feature.h"
#include "codec_def_common.h"
#include "encode_vp9_vdenc_feature_manager.h"
#include "encode_vp9_brc.h"

namespace encode
{
Vp9EncodeTile::Vp9EncodeTile(
    MediaFeatureManager *featureManager,
    EncodeAllocator *    allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *               constSettings) : EncodeTile(featureManager, allocator, hwInterface, constSettings)
{
    auto encFeatureManager = dynamic_cast<EncodeVp9VdencFeatureManager *>(featureManager);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);

    m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);

    m_hcpInterfaceNew = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(hwInterface->GetHcpInterfaceNext());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hcpInterfaceNew);
}

MOS_STATUS Vp9EncodeTile::Init(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(EncodeTile::Init(settings));

    m_maxTileNumber = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_WIDTH) *
                      CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_HEIGHT);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeTile::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    EncoderParams *encodeParams = (EncoderParams *)params;
    ENCODE_CHK_NULL_RETURN(encodeParams);

    PCODEC_VP9_ENCODE_PIC_PARAMS vp9PicParams = static_cast<PCODEC_VP9_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(vp9PicParams);

    m_enabled = true;

    m_maxTileNumber = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_WIDTH) *
                      CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_HEIGHT);

    ENCODE_CHK_STATUS_RETURN(EncodeTile::Update(params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeTile::SetCurrentTile(uint32_t tileRow, uint32_t tileCol, EncodePipeline *pipeline)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodeTile::SetCurrentTile(tileRow, tileCol, pipeline));

    auto basicFeature = static_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    // Update any fields here as need
    if (basicFeature->m_scalableMode)
    {
        m_curTileCodingParams.Vp9ProbabilityCounterStreamoutOffset = m_tileData[m_tileIdx].vp9ProbabilityCounterStreamoutOffset;
    }
    else
    {
        m_curTileCodingParams.CuRecordOffset           = 0;
        m_curTileCodingParams.SliceSizeStreamoutOffset = 0;
        m_curTileCodingParams.SseRowstoreOffset        = 0;
        m_curTileCodingParams.SaoRowstoreOffset        = 0;
        m_curTileCodingParams.BitstreamByteOffset      = 0;
        m_curTileCodingParams.CuLevelStreamoutOffset   = 0;
        m_curTileCodingParams.TileSizeStreamoutOffset  = 0;
        // DW5
        m_curTileCodingParams.PakTileStatisticsOffset = 0;
        // DW12
        m_curTileCodingParams.Vp9ProbabilityCounterStreamoutOffset = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeTile::SetRegionsForBrcUpdate(mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params) const
{
    ENCODE_FUNC_CALL();

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    // VDEnc Stats Buffer - IN
    params.regionParams[1].presRegion = const_cast<PMOS_RESOURCE>(&m_resHuCPakAggregatedFrameStatsBuffer);
    params.regionParams[1].dwOffset   = m_tileStatsOffset.vdencStats;
    // Frame (not PAK) Stats Buffer - IN
    params.regionParams[2].presRegion = const_cast<PMOS_RESOURCE>(&m_resHuCPakAggregatedFrameStatsBuffer);
    params.regionParams[2].dwOffset   = m_frameStatsOffset.pakStats;
    // PAK MMIO - IN
    params.regionParams[7].presRegion = basicFeature->m_hucPakIntBrcDataBuffer;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeTile::SetRegionsForPakInt(mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params) const
{
    ENCODE_FUNC_CALL();
    // Region 0 - Tile based input statistics from PAK/ VDEnc
    params.regionParams[0].presRegion = const_cast<PMOS_RESOURCE>(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]);
    params.regionParams[0].dwOffset   = 0;
    // Region 1 - HuC Frame statistics output
    params.regionParams[1].presRegion = const_cast<PMOS_RESOURCE>(&m_resHuCPakAggregatedFrameStatsBuffer);
    params.regionParams[1].isWritable = true;
    // Region 15 [In/Out] - Tile Record Buffer
    params.regionParams[15].presRegion = const_cast<PMOS_RESOURCE>(&m_tileRecordBuffer[m_statisticsBufIndex]);
    params.regionParams[15].dwOffset   = 0;
    params.regionParams[15].isWritable = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeTile::SetHcpTileCodingParams(uint32_t activePipes)
{
    ENCODE_FUNC_CALL();
    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    m_curTileCodingParams.NumberOfActiveBePipes = activePipes;

    if (basicFeature->m_scalableMode)
    {
        m_curTileCodingParams.Vp9ProbabilityCounterStreamoutOffset = m_tileData[m_tileIdx].vp9ProbabilityCounterStreamoutOffset;
    }
    else
    {
        m_curTileCodingParams.Vp9ProbabilityCounterStreamoutOffset = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeTile::SetVdencPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
{
    ENCODE_FUNC_CALL();
    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_RESOURCE *tileStatisticsBuffer = &m_resTileBasedStatisticsBuffer[m_statisticsBufIndex];
    if (!Mos_ResourceIsNull(tileStatisticsBuffer))
    {
        pipeBufAddrParams.presVdencStreamOutBuffer    = tileStatisticsBuffer;
        pipeBufAddrParams.dwVdencStatsStreamOutOffset = m_tileStatsOffset.vdencStats;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeTile::GetTileStatusInfo(Vp9TileStatusInfo &vp9TileStatsOffset, Vp9TileStatusInfo &vp9FrameStatsOffset, Vp9TileStatusInfo &vp9StatsSize)
{
    ENCODE_FUNC_CALL();
    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    vp9TileStatsOffset  = m_tileStatsOffset;
    vp9FrameStatsOffset = m_frameStatsOffset;
    vp9StatsSize        = m_statsSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeTile::AllocateResources()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodeTile::AllocateResources());

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    MOS_RESOURCE *allocatedBuffer  = nullptr;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    // Tile record stream out buffer
    uint32_t size                       = basicFeature->m_maxPicSizeInSb * CODECHAL_CACHELINE_SIZE;  // worst case: each SB is tile
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "TileRecordStreamOutBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resTileRecordStrmOutBuffer = *allocatedBuffer;

    // CU statistics stream out buffer
    size                                = MOS_ALIGN_CEIL(basicFeature->m_maxPicSizeInSb * 64 * 8, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "CuStatsStrmOutBuffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resCuStatsStrmOutBuffer = *allocatedBuffer;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9EncodeTile::SetTileData(void *params)
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (!m_enabled)
    {
        return eStatus;
    }

    ENCODE_CHK_NULL_RETURN(params);

    EncoderParams *encodeParams = (EncoderParams *)params;
    ENCODE_CHK_NULL_RETURN(encodeParams);

    PCODEC_VP9_ENCODE_PIC_PARAMS vp9PicParams = static_cast<PCODEC_VP9_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(vp9PicParams);

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    m_numTileRows    = (1 << vp9PicParams->log2_tile_rows);
    m_numTileColumns = (1 << vp9PicParams->log2_tile_columns);

    // Tile width needs to be minimum size 256, error out if less
    if ((m_numTileColumns != 1) && ((vp9PicParams->SrcFrameWidthMinus1 + 1) < m_numTileColumns * CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_WIDTH))
    {
        ENCODE_ASSERTMESSAGE("Incorrect number of columns input parameter, Tile width is < 256");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_numTileRows > 4)
    {
        ENCODE_ASSERTMESSAGE("Max number of rows cannot exceeds 4 by VP9 Spec.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_numTiles = m_numTileRows * m_numTileColumns;
    if (m_numTiles > CODECHAL_GET_WIDTH_IN_BLOCKS(basicFeature->m_frameWidth, CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_WIDTH) *
                         CODECHAL_GET_HEIGHT_IN_BLOCKS(basicFeature->m_frameHeight, CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_HEIGHT))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    //max LCU size is 64, min Cu size is 8
    const uint32_t maxNumOfCUInSB     = (CODEC_VP9_SUPER_BLOCK_HEIGHT / CODEC_VP9_MIN_BLOCK_HEIGHT) * (CODEC_VP9_SUPER_BLOCK_WIDTH / CODEC_VP9_MIN_BLOCK_WIDTH);
    const uint32_t numCachelinesPerSB = MOS_ROUNDUP_DIVIDE((2 * BYTES_PER_DWORD * (NUM_PAK_DWS_PER_LCU + maxNumOfCUInSB * NUM_DWS_PER_CU)), MHW_CACHELINE_SIZE);

    uint32_t bitstreamSizePerTile          = basicFeature->m_bitstreamUpperBound / (m_numTiles * CODECHAL_CACHELINE_SIZE);
    uint32_t numCuRecord                   = 64;
    uint32_t cuLevelStreamoutOffset        = 0;
    uint32_t sliceSizeStreamoutOffset      = 0;
    uint32_t bitstreamByteOffset           = 0;
    uint32_t saoRowstoreOffset             = 0;
    uint32_t sseRowstoreOffset             = 0;
    uint32_t tileStartSbAddr               = 0;
    uint32_t cumulativeCUTileOffsetInBytes = 0;

    uint32_t picWidthInSb  = basicFeature->m_picWidthInSb;
    uint32_t picHeightInSb = basicFeature->m_picHeightInSb;
    bool     scalableMode  = basicFeature->m_scalableMode;

    for (uint32_t numLcusInTiles = 0, tileY = 0; tileY < m_numTileRows; ++tileY)
    {
        bool     isLastTileRow     = ((m_numTileRows - 1) == tileY);
        uint32_t tileStartSbY      = (tileY * picHeightInSb) >> vp9PicParams->log2_tile_rows;
        uint32_t tileHeightInSb    = (isLastTileRow ? picHeightInSb : (((tileY + 1) * picHeightInSb) >> vp9PicParams->log2_tile_rows)) - tileStartSbY;
        uint32_t lastTileRowHeight = (MOS_ALIGN_CEIL((vp9PicParams->SrcFrameHeightMinus1 + 1 - tileStartSbY * CODEC_VP9_SUPER_BLOCK_HEIGHT), CODEC_VP9_MIN_BLOCK_HEIGHT) / CODEC_VP9_MIN_BLOCK_HEIGHT) - 1;

        for (uint32_t tileX = 0; tileX < m_numTileColumns; ++tileX)
        {
            uint32_t idx = tileY * m_numTileColumns + tileX;

            bool     isLastTileCol    = ((m_numTileColumns - 1) == tileX);
            uint32_t tileStartSbX     = (tileX * picWidthInSb) >> vp9PicParams->log2_tile_columns;
            uint32_t tileWidthInSb    = (isLastTileCol ? picWidthInSb : (((tileX + 1) * picWidthInSb) >> vp9PicParams->log2_tile_columns)) - tileStartSbX;
            uint32_t lastTileColWidth = (MOS_ALIGN_CEIL((vp9PicParams->SrcFrameWidthMinus1 + 1 - tileStartSbX * CODEC_VP9_SUPER_BLOCK_WIDTH), CODEC_VP9_MIN_BLOCK_WIDTH) / CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
            uint32_t numLcuInTile     = tileWidthInSb * tileHeightInSb;

            m_tileData[idx].mode           = CODECHAL_ENCODE_MODE_VP9;
            m_tileData[idx].tileStartXInSb = tileStartSbX;
            m_tileData[idx].tileStartYInSb = tileStartSbY;
            m_tileData[idx].tileEndXInSb   = m_tileData[idx].tileStartXInSb + tileWidthInSb;
            m_tileData[idx].tileEndYInSb   = m_tileData[idx].tileStartYInSb + tileHeightInSb;

            //m_tileData[idx].tileColumnStoreSelect = tileX % 2;
            //m_tileData[idx].tileRowStoreSelect    = tileY % 2;

            m_tileData[idx].numOfTilesInFrame       = m_numTiles;
            m_tileData[idx].numOfTileColumnsInFrame = m_numTileColumns;

            m_tileData[idx].tileStartXInLCU = tileStartSbX;
            m_tileData[idx].tileStartYInLCU = tileStartSbY;
            m_tileData[idx].tileEndXInLCU   = m_tileData[idx].tileStartXInLCU + tileWidthInSb;
            m_tileData[idx].tileEndYInLCU   = m_tileData[idx].tileStartYInLCU + tileHeightInSb;

            m_tileData[idx].isLastTileofColumn = isLastTileRow;
            m_tileData[idx].isLastTileofRow    = isLastTileCol;

            m_tileData[idx].tileWidthInMinCbMinus1  = isLastTileCol ? lastTileColWidth : (tileWidthInSb * CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
            m_tileData[idx].tileHeightInMinCbMinus1 = isLastTileRow ? lastTileRowHeight : (tileHeightInSb * CODEC_VP9_MIN_BLOCK_HEIGHT) - 1;

            // Reset the following fields for non-scalable mode in SetCurrentTile()
            sseRowstoreOffset = (m_tileData[idx].tileStartXInSb + (3 * tileX)) << 5;

            m_tileData[idx].cuRecordOffset           = MOS_ALIGN_CEIL(((numCuRecord * numLcusInTiles) * 64), CODECHAL_CACHELINE_SIZE) / CODECHAL_CACHELINE_SIZE;
            m_tileData[idx].sliceSizeStreamoutOffset = sliceSizeStreamoutOffset;
            m_tileData[idx].sseRowstoreOffset        = sseRowstoreOffset;
            m_tileData[idx].saoRowstoreOffset        = saoRowstoreOffset;
            m_tileData[idx].bitstreamByteOffset      = bitstreamByteOffset;
            m_tileData[idx].cuLevelStreamoutOffset   = cuLevelStreamoutOffset;

            m_tileData[idx].cumulativeCUTileOffset = cumulativeCUTileOffsetInBytes / CODECHAL_CACHELINE_SIZE;
            m_tileData[idx].bitstreamByteOffset    = bitstreamByteOffset;
            m_tileData[idx].tileStreaminOffset     = 4 * (m_tileData[idx].tileStartYInSb * picWidthInSb + m_tileData[idx].tileStartXInSb * tileHeightInSb);

            uint32_t numOfSbsInTile = tileWidthInSb * tileHeightInSb;
            tileStartSbAddr += numOfSbsInTile;
            // For Cumulative CU Count : 2 bytes per LCU
            cumulativeCUTileOffsetInBytes += numOfSbsInTile * 2;
            cumulativeCUTileOffsetInBytes  = MOS_ALIGN_CEIL(cumulativeCUTileOffsetInBytes, CODECHAL_CACHELINE_SIZE);

            if (m_tileData[idx].tileStartXInSb != 0 || m_tileData[idx].tileStartYInSb != 0)
            {
                uint32_t numOfSBs                      = m_tileData[idx].tileStartYInLCU * picWidthInSb + m_tileData[idx].tileStartXInLCU * tileHeightInSb;
                m_tileData[idx].tileLCUStreamOutOffset = numOfSBs * numCachelinesPerSB;
            }

            cuLevelStreamoutOffset    += (m_tileData[idx].tileWidthInMinCbMinus1 + 1) * (m_tileData[idx].tileHeightInMinCbMinus1 + 1);
            sliceSizeStreamoutOffset  += (m_tileData[idx].tileWidthInMinCbMinus1 + 1) * (m_tileData[idx].tileHeightInMinCbMinus1 + 1);
            bitstreamByteOffset       += bitstreamSizePerTile;
            numLcusInTiles            += numLcuInTile;

            m_tileData[idx].tileSizeStreamoutOffset = (idx * m_hcpInterfaceNew->GetPakHWTileSizeRecordSize() + CODECHAL_CACHELINE_SIZE - 1) / CODECHAL_CACHELINE_SIZE;

            // DW5
            const uint32_t frameStatsStreamoutSize  = Vp9EncodeBrc::m_brcPakStatsBufSize;
            m_tileData[idx].pakTileStatisticsOffset = (idx * frameStatsStreamoutSize + CODECHAL_CACHELINE_SIZE - 1) / CODECHAL_CACHELINE_SIZE;

            // DW12
            m_tileData[idx].vp9ProbabilityCounterStreamoutOffset = ((idx * m_probabilityCounterBufferSize) + (CODECHAL_CACHELINE_SIZE - 1)) / CODECHAL_CACHELINE_SIZE;
        }
    }

    return eStatus;
}

MOS_STATUS Vp9EncodeTile::AllocateTileStatistics(void *params)
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(params);

    if (!m_enabled)
    {
        return eStatus;
    }

    EncoderParams *              encodeParams = (EncoderParams *)params;
    PCODEC_VP9_ENCODE_PIC_PARAMS vp9PicParams = static_cast<PCODEC_VP9_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);

    MOS_ZeroMemory(&m_statsSize, sizeof(Vp9TileStatusInfo));
    MOS_ZeroMemory(&m_frameStatsOffset, sizeof(Vp9TileStatusInfo));
    MOS_ZeroMemory(&m_tileStatsOffset, sizeof(Vp9TileStatusInfo));

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    // Sizes of each buffer to be loaded into the region 0 as input and 1 loaded out as output
    m_statsSize.tileSizeRecord = m_hcpInterfaceNew->GetPakHWTileSizeRecordSize();
    m_statsSize.vdencStats     = Vp9EncodeBrc::m_brcStatsBufSize;     // VDEnc stats size
    m_statsSize.pakStats       = Vp9EncodeBrc::m_brcPakStatsBufSize;  // Frame stats size
    m_statsSize.counterBuffer  = m_probabilityCounterBufferSize;

    // Maintain the offsets to use for patching addresses in to the HuC Pak Integration kernel Aggregated Frame Statistics Output Buffer
    // Each offset needs to be page aligned as the combined region is fed into different page aligned HuC regions
    m_frameStatsOffset.tileSizeRecord = 0;
    m_frameStatsOffset.vdencStats     = MOS_ALIGN_CEIL((m_frameStatsOffset.tileSizeRecord + (m_maxTileNumber * m_statsSize.tileSizeRecord)), CODECHAL_PAGE_SIZE);
    m_frameStatsOffset.pakStats       = MOS_ALIGN_CEIL((m_frameStatsOffset.vdencStats + m_statsSize.vdencStats), CODECHAL_PAGE_SIZE);
    m_frameStatsOffset.counterBuffer  = MOS_ALIGN_CEIL((m_frameStatsOffset.pakStats + m_statsSize.pakStats), CODECHAL_PAGE_SIZE);

    // Frame level statistics
    m_hwInterface->m_pakIntAggregatedFrameStatsSize = MOS_ALIGN_CEIL((m_frameStatsOffset.counterBuffer + m_statsSize.counterBuffer), CODECHAL_PAGE_SIZE);

    // VP9 Frame Statistics Buffer - Output from HuC PAK Integration kernel
    // Ref. CodechalVdencVp9StateG12::m_frameStatsPakIntegrationBuffer
    if (Mos_ResourceIsNull(&m_resHuCPakAggregatedFrameStatsBuffer))
    {
        allocParamsForBufferLinear.dwBytes  = m_hwInterface->m_pakIntAggregatedFrameStatsSize;
        allocParamsForBufferLinear.pBufName = "PAK HUC Integrated Aggregated Frame Statistics Streamout Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;

        auto allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_resHuCPakAggregatedFrameStatsBuffer = *allocatedBuffer;
    }
    // Max row is 4 by VP9 Spec
    uint32_t maxScalableModeTiles = m_maxTileNumber;

    // Maintain the offsets to use for patching addresses in to the Tile Based Statistics Buffer
    // Each offset needs to be page aligned as the combined region is fed into different page aligned HuC regions
    // Fill Pak integration kernel input tile stats structure
    // TileSizeRecord has to be 4K aligned
    m_tileStatsOffset.tileSizeRecord     = 0;
    // VdencStats has to be 4k aligned
    m_tileStatsOffset.vdencStats         = MOS_ALIGN_CEIL((m_tileStatsOffset.tileSizeRecord + (maxScalableModeTiles * m_statsSize.tileSizeRecord)), CODECHAL_PAGE_SIZE);
    // VP9PAKStats has to be 64 byte aligned
    m_tileStatsOffset.pakStats           = MOS_ALIGN_CEIL((m_tileStatsOffset.vdencStats + (maxScalableModeTiles * m_statsSize.vdencStats)), CODECHAL_PAGE_SIZE);
    // VP9CounterBuffer has to be 4k aligned
    m_tileStatsOffset.counterBuffer      = MOS_ALIGN_CEIL((m_tileStatsOffset.pakStats + (maxScalableModeTiles * m_statsSize.pakStats)), CODECHAL_PAGE_SIZE);
    // Combined statistics size for all tiles
    m_hwInterface->m_pakIntTileStatsSize = MOS_ALIGN_CEIL((m_tileStatsOffset.counterBuffer + (maxScalableModeTiles * m_statsSize.counterBuffer)), CODECHAL_PAGE_SIZE);

    // Tile size record size for all tiles
    m_hwInterface->m_tileRecordSize = m_statsSize.tileSizeRecord * maxScalableModeTiles;

    uint32_t    curPakIntTileStatsSize = 0;
    MOS_SURFACE surface;
    MOS_ZeroMemory(&surface, sizeof(surface));
    // Ref. CodechalVdencVp9StateG12::m_tileStatsPakIntegrationBuffer[]
    surface.OsResource = m_resTileBasedStatisticsBuffer[m_statisticsBufIndex];
    if (!Mos_ResourceIsNull(&surface.OsResource))
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
        allocParamsForBufferLinear.pBufName = "Tile Level Statistics Streamout Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;

        auto allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_resTileBasedStatisticsBuffer[m_statisticsBufIndex] = *allocatedBuffer;
    }

    // Allocate the updated tile size buffer for PAK integration kernel
    // Ref. CodechalVdencVp9StateG12::m_tileRecordBuffer[]
    if (Mos_ResourceIsNull(&m_tileRecordBuffer[m_statisticsBufIndex]))
    {
        auto size                           = m_maxTileNumber * MOS_ALIGN_CEIL(m_hcpInterfaceNew->GetPakHWTileSizeRecordSize(), CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.dwBytes  = size;
        allocParamsForBufferLinear.pBufName = "Tile Record Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;

        auto allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_tileRecordBuffer[m_statisticsBufIndex] = *allocatedBuffer;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_TILE_CODING, Vp9EncodeTile)
{
    ENCODE_FUNC_CALL();

    auto tileCodingParams = m_curTileCodingParams;

    params.numberOfActiveBePipes     = tileCodingParams.NumberOfActiveBePipes;
    params.numOfTileColumnsInFrame   = tileCodingParams.NumOfTileColumnsInFrame;
    params.tileRowStoreSelect        = tileCodingParams.TileRowStoreSelect;
    params.tileColumnStoreSelect     = tileCodingParams.TileColumnStoreSelect;

    params.tileStartLCUX             = tileCodingParams.TileStartLCUX;
    params.tileStartLCUY             = tileCodingParams.TileStartLCUY;
    params.nonFirstPassTile          = tileCodingParams.bTileReplayEnable && (!tileCodingParams.IsFirstPass);
    params.isLastTileofColumn        = tileCodingParams.IsLastTileofColumn;
    params.isLastTileofRow           = tileCodingParams.IsLastTileofRow;
    params.tileHeightInMinCbMinus1   = tileCodingParams.TileHeightInMinCbMinus1;
    params.tileWidthInMinCbMinus1    = tileCodingParams.TileWidthInMinCbMinus1;

    params.bitstreamByteOffsetEnable = tileCodingParams.bTileReplayEnable;

    params.cuRecordOffset            = tileCodingParams.CuRecordOffset;
    params.bitstreamByteOffset       = tileCodingParams.BitstreamByteOffset;
    params.pakTileStatisticsOffset   = tileCodingParams.PakTileStatisticsOffset;
    params.cuLevelStreamoutOffset    = tileCodingParams.CuLevelStreamoutOffset;
    params.sliceSizeStreamoutOffset  = tileCodingParams.SliceSizeStreamoutOffset;
    params.tileSizeStreamoutOffset   = tileCodingParams.TileSizeStreamoutOffset;
    params.sseRowstoreOffset         = tileCodingParams.SseRowstoreOffset;
    params.saoRowstoreOffset         = tileCodingParams.SaoRowstoreOffset;
    params.tileSizeStreamoutOffset   = tileCodingParams.TileSizeStreamoutOffset;

    params.vp9ProbabilityCounterStreamoutOffset = tileCodingParams.Vp9ProbabilityCounterStreamoutOffset;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, Vp9EncodeTile)
{
    ENCODE_FUNC_CALL();

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    uint32_t picSizeInSb = basicFeature->m_picSizeInSb;

    if (basicFeature->m_ref.DysRefFrameFlags() != DYS_REF_NONE)
    {
        params.presTileRecordBuffer = const_cast<PMOS_RESOURCE>(&m_resTileRecordStrmOutBuffer);
        params.dwTileRecordSize     = picSizeInSb * CODECHAL_CACHELINE_SIZE;
        params.presCuStatsBuffer    = const_cast<PMOS_RESOURCE>(&m_resCuStatsStrmOutBuffer);
        params.dwCuStatsSize        = MOS_ALIGN_CEIL(picSizeInSb * 64 * 8, CODECHAL_CACHELINE_SIZE);
    }
    else
    {
        if (basicFeature->m_scalableMode && basicFeature->m_hucEnabled)
        {
            // Overwrite presProbabilityCounterBuffer and it's params for scalable mode
            params.presProbabilityCounterBuffer = const_cast<PMOS_RESOURCE>(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]);
            params.dwProbabilityCounterOffset   = m_tileStatsOffset.counterBuffer;
            params.dwProbabilityCounterSize     = m_statsSize.counterBuffer;
        }

        MOS_RESOURCE *tileRecordBuffer = const_cast<PMOS_RESOURCE>(&m_tileRecordBuffer[m_statisticsBufIndex]);
        if (!Mos_ResourceIsNull(tileRecordBuffer))
        {
            params.presPakTileSizeStasBuffer   = tileRecordBuffer;
            params.dwPakTileSizeStasBufferSize = m_statsSize.tileSizeRecord * m_numTiles;
            params.dwPakTileSizeRecordOffset   = m_tileStatsOffset.tileSizeRecord;
        }
        else
        {
            params.presPakTileSizeStasBuffer   = nullptr;
            params.dwPakTileSizeStasBufferSize = 0;
            params.dwPakTileSizeRecordOffset   = 0;
        }

        // Need to use presPakTileSizeStasBuffer instead of presTileRecordBuffer, so setting to null
        params.presTileRecordBuffer = nullptr;
        params.dwTileRecordSize     = 0;
        params.presCuStatsBuffer    = const_cast<PMOS_RESOURCE>(&m_resCuStatsStrmOutBuffer);
        params.dwCuStatsSize        = MOS_ALIGN_CEIL(picSizeInSb * 64 * 8, CODECHAL_CACHELINE_SIZE);
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9EncodeTile)
{
    ENCODE_FUNC_CALL();

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    if (basicFeature->m_scalableMode && basicFeature->m_hucEnabled)
    {
        MOS_RESOURCE *tileStatisticsBuffer = const_cast<PMOS_RESOURCE>(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]);
        if (!Mos_ResourceIsNull(tileStatisticsBuffer))
        {
            params.presVdencStreamOutBuffer    = tileStatisticsBuffer;
            params.dwVdencStatsStreamOutOffset = m_tileStatsOffset.vdencStats;

            // The new framestats streamout will now be the tile level stats buffer because each pak is spewing out tile level stats
            params.presFrameStatStreamOutBuffer = tileStatisticsBuffer;
            params.dwFrameStatStreamOutOffset   = m_tileStatsOffset.pakStats;
            // Main Frame Stats are integrated by PAK integration kernel
        }
        else
        {
            params.presFrameStatStreamOutBuffer = nullptr;
            params.dwFrameStatStreamOutOffset   = 0;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_WEIGHTSOFFSETS_STATE, Vp9EncodeTile)
{
    ENCODE_FUNC_CALL();

    int8_t size = sizeof(params.weightsLuma) / sizeof(int8_t);
    memset(params.weightsLuma, 1, size);
    memset(params.offsetsLuma, 0, size);

    size = sizeof(params.weightsChroma) / sizeof(int8_t);
    memset(params.weightsChroma, 0, size);
    memset(params.offsetsChroma, 0, size);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Vp9EncodeTile)
{
    ENCODE_FUNC_CALL();

    auto vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(vp9BasicFeature);
    auto picParams       = vp9BasicFeature->m_vp9PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    auto tileCodingParams = m_curTileCodingParams;
    params.ctbSize        = CODEC_VP9_SUPER_BLOCK_WIDTH;

    if (!m_enabled)
    {
        params.tileWidth  = picParams->SrcFrameWidthMinus1 + 1;
        params.tileHeight = picParams->SrcFrameHeightMinus1 + 1;
    }
    else
    {
        params.tileWidth  = ((tileCodingParams.TileWidthInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_WIDTH);
        params.tileHeight = ((tileCodingParams.TileHeightInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_HEIGHT);

        params.tileStartLCUX = tileCodingParams.TileStartLCUX;
        params.tileStartLCUY = tileCodingParams.TileStartLCUY;

        params.tileId  = m_tileIdx;

        params.tileEnable              = 1;
        params.tileStreamInOffset      = tileCodingParams.TileStreaminOffset;
        params.tileLCUStreamOutOffset  = tileCodingParams.TileLCUStreamOutOffset;
        params.tileRowstoreOffset      = (params.tileStartLCUY == 0) ? (params.tileStartLCUX * params.ctbSize) / 32 : 0;

        params.VdencHEVCVP9TileSlicePar18 = 0;
        params.VdencHEVCVP9TileSlicePar19 = tileCodingParams.CumulativeCUTileOffset;
    }

    // Default values
    params.VdencHEVCVP9TileSlicePar12 = 0x3f;
    params.VdencHEVCVP9TileSlicePar13 = 2;

    params.VdencHEVCVP9TileSlicePar17[0] = 0x3f;
    params.VdencHEVCVP9TileSlicePar17[1] = 0x3f;
    params.VdencHEVCVP9TileSlicePar17[2] = 0x3f;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_WALKER_STATE, Vp9EncodeTile)
{
    ENCODE_FUNC_CALL();

    auto vp9BasicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(vp9BasicFeature);
    auto picParams = vp9BasicFeature->m_vp9PicParams;
    ENCODE_CHK_NULL_RETURN(picParams);
    auto tileCodingParams = m_curTileCodingParams;

    if (!m_enabled)
    {
        params.nextTileSliceStartLcuMbX = CODECHAL_GET_WIDTH_IN_BLOCKS(picParams->SrcFrameWidthMinus1, CODEC_VP9_SUPER_BLOCK_WIDTH);
        params.nextTileSliceStartLcuMbY = CODECHAL_GET_HEIGHT_IN_BLOCKS(picParams->SrcFrameHeightMinus1, CODEC_VP9_SUPER_BLOCK_HEIGHT);
        params.firstSuperSlice          = 1;
    }
    else
    {
        params.tileSliceStartLcuMbX = tileCodingParams.TileStartLCUX;
        params.tileSliceStartLcuMbY = tileCodingParams.TileStartLCUY;

        uint16_t tileWidth  = ((tileCodingParams.TileWidthInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
        uint16_t tileHeight = ((tileCodingParams.TileHeightInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_HEIGHT) - 1;

        uint32_t tileStartCtbX = tileCodingParams.TileStartLCUX * CODEC_VP9_SUPER_BLOCK_WIDTH;
        uint32_t tileStartCtbY = tileCodingParams.TileStartLCUY * CODEC_VP9_SUPER_BLOCK_HEIGHT;

        params.nextTileSliceStartLcuMbX = CODECHAL_GET_WIDTH_IN_BLOCKS((tileStartCtbX + tileWidth + 1), CODEC_VP9_SUPER_BLOCK_WIDTH);
        params.nextTileSliceStartLcuMbY = CODECHAL_GET_HEIGHT_IN_BLOCKS((tileStartCtbY + tileHeight + 1), CODEC_VP9_SUPER_BLOCK_HEIGHT);

        params.firstSuperSlice = 1;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Vp9EncodeTile)
{
    ENCODE_FUNC_CALL();

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeature);

    if (basicFeature->m_scalableMode && basicFeature->m_hucEnabled)
    {
        MOS_RESOURCE *tileStatisticsBuffer = const_cast<PMOS_RESOURCE>(&m_resTileBasedStatisticsBuffer[m_statisticsBufIndex]);
        if (!Mos_ResourceIsNull(tileStatisticsBuffer))
        {
            params.streamOutBuffer = tileStatisticsBuffer;
            params.streamOutOffset = m_tileStatsOffset.vdencStats;
        }
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
