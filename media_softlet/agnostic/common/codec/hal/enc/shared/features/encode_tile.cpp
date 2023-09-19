/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     encode_tile.cpp
//! \brief    Defines the common interface for encode tile
//!

#include "encode_tile.h"
#include "codec_def_common.h"
#include "encode_pipeline.h"

namespace encode
{
    EncodeTile::EncodeTile(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings) :
        MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr),
        m_allocator(allocator)
    {
        m_hwInterface = hwInterface;
        m_featureManager = featureManager;
        m_currentThirdLevelBatchBuffer = m_thirdLevelBatchBuffers.begin();
    }

    EncodeTile::~EncodeTile()
    {
        if (m_hwInterface != nullptr)
        {
            for(auto& iter : m_thirdLevelBatchBuffers){
                Mhw_FreeBb(m_hwInterface->GetOsInterface(), &iter, nullptr);
            }
        }
        FreeTileLevelBatch();
        FreeTileRowLevelBRCBatch();

        if (m_allocator != nullptr)
        {
            if(!Mos_ResourceIsNull(m_resTileBasedStatisticsBuffer))
            {
                for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resTileBasedStatisticsBuffer); i++)
                {
                    if(!Mos_ResourceIsNull(&m_resTileBasedStatisticsBuffer[i]))
                    {
                        m_allocator->DestroyResource(&m_resTileBasedStatisticsBuffer[i]);
                    }
                }
            }
            if (!Mos_ResourceIsNull(m_tileRecordBuffer))
            {
                for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_tileRecordBuffer); i++)
                {
                    if (!Mos_ResourceIsNull(&m_tileRecordBuffer[i]))
                    {
                        m_allocator->DestroyResource(&m_tileRecordBuffer[i]);
                    }
                }
            }
            if (!Mos_ResourceIsNull(&m_resHuCPakAggregatedFrameStatsBuffer))
            {
                m_allocator->DestroyResource(&m_resHuCPakAggregatedFrameStatsBuffer);
            }

        }

        for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_reportTileData); i++)
        {
            MOS_FreeMemory(m_reportTileData[i]);
        }

        if (m_tileData != nullptr)
        {
            MOS_FreeMemory(m_tileData);
        }

    }

    MOS_STATUS EncodeTile::Init(void *settings)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(settings);

        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::Update(void *params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);

        if (m_maxTileNumber > m_maxTileNumberUsed)
        {
            if (m_tileData)
            {
                MOS_FreeMemory(m_tileData);
            }
            m_tileData = nullptr;
            m_maxTileNumberUsed = m_maxTileNumber;
        }

        // create the tile data parameters
        if (!m_tileData)
        {
            m_tileData = (EncodeTileData *)MOS_AllocAndZeroMemory(
                sizeof(EncodeTileData) * m_maxTileNumber);
        }

        m_prevStatisticsBufIndex = m_statisticsBufIndex;
        m_statisticsBufIndex     = m_basicFeature->m_currOriginalPic.FrameIdx;

        if (!m_enabled)
        {
            if (m_reportTileData[m_statisticsBufIndex] != nullptr)
            {
                MOS_FreeMemory(m_reportTileData[m_statisticsBufIndex]);
                m_reportTileData[m_statisticsBufIndex] = nullptr;
            }
            return MOS_STATUS_SUCCESS;
        }

        m_tileBatchBufferIndex = (m_tileBatchBufferIndex + 1) % m_codecHalNumTileLevelBatchBuffers;

        // Setup tile data
        ENCODE_CHK_STATUS_RETURN(SetTileData(params));

        // Setup tile report data
        ENCODE_CHK_STATUS_RETURN(SetTileReportData());

        ENCODE_CHK_STATUS_RETURN(AllocateTileLevelBatch());
        ENCODE_CHK_STATUS_RETURN(AllocateTileRowLevelBRCBatch());
        ENCODE_CHK_STATUS_RETURN(AllocateTileStatistics(params));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::AllocateResources()
    {
        ENCODE_FUNC_CALL();

        m_thirdLevelBatchSize = MOS_ALIGN_CEIL(1024, CODECHAL_PAGE_SIZE);

        // 3rd level batch buffer
        // To be moved to a more proper place later
        for(auto& iter: m_thirdLevelBatchBuffers){
            MOS_ZeroMemory(&iter, sizeof(iter));
            iter.bSecondLevel = true;
            ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                m_hwInterface->GetOsInterface(),
                &iter,
                nullptr,
                m_thirdLevelBatchSize));
        }
        m_currentThirdLevelBatchBuffer = m_thirdLevelBatchBuffers.begin();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::IncrementThirdLevelBatchBuffer()
    {
        ENCODE_FUNC_CALL();

        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        if(++m_currentThirdLevelBatchBuffer == m_thirdLevelBatchBuffers.end()){
            m_currentThirdLevelBatchBuffer = m_thirdLevelBatchBuffers.begin();
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::GetThirdLevelBatchBuffer(
        PMHW_BATCH_BUFFER &thirdLevelBatchBuffer)
    {
        ENCODE_FUNC_CALL();
        
        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        thirdLevelBatchBuffer = &(*m_currentThirdLevelBatchBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::GetTileLevelBatchBuffer(
        PMHW_BATCH_BUFFER &tileLevelBatchBuffer)
    {
        ENCODE_FUNC_CALL();
        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        tileLevelBatchBuffer = &m_tileLevelBatchBuffer[m_tileBatchBufferIndex][m_tileRowPass][m_tileIdx];
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::GetCurrentTile(
        EncodeTileData &tileData)
    {
        ENCODE_FUNC_CALL();
        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        tileData = m_tileData[m_tileIdx];
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::GetTileByIndex(
        EncodeTileData &tileData,
        uint32_t        index)
    {
        ENCODE_FUNC_CALL();
        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        tileData = m_tileData[index];
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::SetCurrentTile(
        uint32_t tileRow,
        uint32_t tileCol,
        EncodePipeline *pipeline)
    {
        ENCODE_FUNC_CALL();
        if (!m_enabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        m_tileIdx = tileRow * m_numTileColumns + tileCol;

        m_tileData[m_tileIdx].isFirstPass      = pipeline->IsFirstPass();
        m_tileData[m_tileIdx].isLastPass       = pipeline->IsLastPass();
        m_tileData[m_tileIdx].tileReplayEnable = m_enableTileReplay;

        MOS_ZeroMemory(&m_curTileCodingParams, sizeof(EncodeTileCodingParams));

        m_curTileCodingParams.NumOfTilesInFrame       = m_tileData[m_tileIdx].numOfTilesInFrame;
        m_curTileCodingParams.NumOfTileColumnsInFrame = m_tileData[m_tileIdx].numOfTileColumnsInFrame;
        m_curTileCodingParams.TileStartLCUX           = m_tileData[m_tileIdx].tileStartX;
        m_curTileCodingParams.TileStartLCUY           = m_tileData[m_tileIdx].tileStartY;
        m_curTileCodingParams.TileHeightInMinCbMinus1 = m_tileData[m_tileIdx].tileHeightInMinMinus1;
        m_curTileCodingParams.TileWidthInMinCbMinus1  = m_tileData[m_tileIdx].tileWidthInMinCbMinus1;
        m_curTileCodingParams.IsLastTileofColumn      = m_tileData[m_tileIdx].isLastTileofColumn;
        m_curTileCodingParams.IsLastTileofRow         = m_tileData[m_tileIdx].isLastTileofRow;
        m_curTileCodingParams.TileRowStoreSelect      = m_tileData[m_tileIdx].tileRowStoreSelect;
        m_curTileCodingParams.TileColumnStoreSelect   = m_tileData[m_tileIdx].tileColumnStoreSelect;
        m_curTileCodingParams.Mode                    = m_tileData[m_tileIdx].mode;
        m_curTileCodingParams.IsFirstPass             = m_tileData[m_tileIdx].isFirstPass;
        m_curTileCodingParams.IsLastPass              = m_tileData[m_tileIdx].isLastPass;
        m_curTileCodingParams.bTileReplayEnable       = m_tileData[m_tileIdx].tileReplayEnable;

        m_curTileCodingParams.BitstreamByteOffset      = m_tileData[m_tileIdx].bitstreamByteOffset;
        m_curTileCodingParams.PakTileStatisticsOffset  = m_tileData[m_tileIdx].pakTileStatisticsOffset;
        m_curTileCodingParams.CuLevelStreamoutOffset   = m_tileData[m_tileIdx].cuLevelStreamoutOffset;
        m_curTileCodingParams.SliceSizeStreamoutOffset = m_tileData[m_tileIdx].sliceSizeStreamoutOffset;
        m_curTileCodingParams.CuRecordOffset           = m_tileData[m_tileIdx].cuRecordOffset;
        m_curTileCodingParams.SseRowstoreOffset        = m_tileData[m_tileIdx].sseRowstoreOffset;
        m_curTileCodingParams.SaoRowstoreOffset        = m_tileData[m_tileIdx].saoRowstoreOffset;
        m_curTileCodingParams.TileSizeStreamoutOffset  = m_tileData[m_tileIdx].tileSizeStreamoutOffset;
        m_curTileCodingParams.TileStreaminOffset       = m_tileData[m_tileIdx].tileStreaminOffset;

        m_curTileCodingParams.CumulativeCUTileOffset  = m_tileData[m_tileIdx].cumulativeCUTileOffset;
        m_curTileCodingParams.TileLCUStreamOutOffset  = m_tileData[m_tileIdx].tileLCUStreamOutOffset;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::BeginPatch3rdLevelBatch(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(&(m_currentThirdLevelBatchBuffer->OsResource));
        ENCODE_CHK_NULL_RETURN(data);
        m_currentThirdLevelBatchBuffer->pData = data;

        MOS_ZeroMemory(&cmdBuffer, sizeof(cmdBuffer));
        cmdBuffer.pCmdBase = cmdBuffer.pCmdPtr = (uint32_t *)data;
        cmdBuffer.iRemaining                   = m_thirdLevelBatchSize;
        cmdBuffer.OsResource                   = m_currentThirdLevelBatchBuffer->OsResource;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::EndPatch3rdLevelBatch()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(&(m_currentThirdLevelBatchBuffer->OsResource)));
        m_currentThirdLevelBatchBuffer->pData = nullptr;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::BeginPatchTileLevelBatch(
        uint32_t            tileRowPass,
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        m_tileRowPass = tileRowPass;

        uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(
            &(m_tileLevelBatchBuffer[m_tileBatchBufferIndex][m_tileRowPass][m_tileIdx].OsResource));
        ENCODE_CHK_NULL_RETURN(data);

        m_tileLevelBatchBuffer[m_tileBatchBufferIndex][m_tileRowPass][m_tileIdx].pData = data;

        MOS_ZeroMemory(&cmdBuffer, sizeof(cmdBuffer));
        cmdBuffer.pCmdBase = cmdBuffer.pCmdPtr = (uint32_t *)data;
        cmdBuffer.iRemaining                   = m_tileLevelBatchSize;
        cmdBuffer.OsResource                   = m_tileLevelBatchBuffer[m_tileBatchBufferIndex][m_tileRowPass][m_tileIdx].OsResource;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::EndPatchTileLevelBatch()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(
            &(m_tileLevelBatchBuffer[m_tileBatchBufferIndex][m_tileRowPass][m_tileIdx].OsResource)));

        m_tileLevelBatchBuffer[m_tileBatchBufferIndex][m_tileRowPass][m_tileIdx].pData = nullptr;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::AllocateTileLevelBatch()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        // Only allocate when the number of tile changed
        if (m_numTileBatchAllocated[m_tileBatchBufferIndex] >= m_numTiles)
        {
            return eStatus;
        }

        // Make it simple, free first if need reallocate
        if (m_numTileBatchAllocated[m_tileBatchBufferIndex] > 0)
        {
            ENCODE_CHK_STATUS_RETURN(FreeTileLevelBatch());
        }

        // Caculate the batch buffer size for each tile
        // To add the MHW interface later, can be fine tuned
        m_tileLevelBatchSize = m_hwInterface->m_vdenc2ndLevelBatchBufferSize;

        // First allocate the batch buffer array
        for (int32_t idx = 0; idx < EncodeBasicFeature::m_vdencBrcPassNum; idx++)
        {
            if (m_tileLevelBatchBuffer[m_tileBatchBufferIndex][idx] == nullptr)
            {
                m_tileLevelBatchBuffer[m_tileBatchBufferIndex][idx] = (PMHW_BATCH_BUFFER)MOS_AllocAndZeroMemory(sizeof(MHW_BATCH_BUFFER) * m_numTiles);

                if (nullptr == m_tileLevelBatchBuffer[m_tileBatchBufferIndex][idx])
                {
                    ENCODE_ASSERTMESSAGE("Allocate memory for tile batch buffer failed");
                    return MOS_STATUS_NO_SPACE;
                }
            }

            // Allocate the batch buffer for each tile
            uint32_t i = 0;
            for (i = 0; i < m_numTiles; i++)
            {
                MOS_ZeroMemory(&m_tileLevelBatchBuffer[m_tileBatchBufferIndex][idx][i], sizeof(MHW_BATCH_BUFFER));
                m_tileLevelBatchBuffer[m_tileBatchBufferIndex][idx][i].bSecondLevel = true;
                ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                    m_hwInterface->GetOsInterface(),
                    &m_tileLevelBatchBuffer[m_tileBatchBufferIndex][idx][i],
                    nullptr,
                    m_tileLevelBatchSize));
            }
        }

        // Record the number of allocated batch buffer for tiles
        m_numTileBatchAllocated[m_tileBatchBufferIndex] = m_numTiles;

        return eStatus;
    }

    MOS_STATUS EncodeTile::FreeTileLevelBatch()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        // Free the batch buffer for each tile
        uint32_t i = 0;
        uint32_t j = 0;
        for(int idx = 0; idx < m_codecHalNumTileLevelBatchBuffers; idx++)
        {
            for (i = 0; i < EncodeBasicFeature::m_vdencBrcPassNum; i++)
            {
                if (m_hwInterface != nullptr)
                {
                    for (j = 0; j < m_numTileBatchAllocated[idx]; j++)
                    {
                        ENCODE_CHK_STATUS_RETURN(Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_tileLevelBatchBuffer[idx][i][j], nullptr));
                    }
                }

                MOS_FreeMemory(m_tileLevelBatchBuffer[idx][i]);
                m_tileLevelBatchBuffer[idx][i] = nullptr;
            }

            // Reset the number of tile batch allocated
            m_numTileBatchAllocated[idx] = 0;
        }

        return eStatus;
    }

    MOS_STATUS EncodeTile::SetTileReportData()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus         = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }

        if (m_reportTileData[m_statisticsBufIndex] != nullptr)
        {
            MOS_FreeMemory(m_reportTileData[m_statisticsBufIndex]);
            m_reportTileData[m_statisticsBufIndex] = nullptr;
        }
        m_reportTileData[m_statisticsBufIndex] = (EncodeReportTileData *)MOS_AllocAndZeroMemory(
            sizeof(EncodeReportTileData) * m_numTiles);

        for (uint32_t i = 0; i < m_numTileRows; i++)
        {
            for (uint32_t j = 0; j < m_numTileColumns; j++)
            {
                uint32_t idx = i * m_numTileColumns + j;
                m_reportTileData[m_statisticsBufIndex][idx].bitstreamByteOffset     = m_tileData[idx].bitstreamByteOffset;
                m_reportTileData[m_statisticsBufIndex][idx].tileHeightInMinCbMinus1 = m_tileData[idx].tileHeightInMinCbMinus1;
                m_reportTileData[m_statisticsBufIndex][idx].tileWidthInMinCbMinus1  = m_tileData[idx].tileWidthInMinCbMinus1;
                m_reportTileData[m_statisticsBufIndex][idx].numTileColumns          = m_numTileColumns;
            }
        }

        return eStatus;
    }

    MOS_STATUS EncodeTile::SetTileReportDataVaild(bool isValid)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_enabled)
        {
            return eStatus;
        }
        
        if (m_reportTileData[m_statisticsBufIndex] == nullptr)
        {
            return MOS_STATUS_NULL_POINTER;
        }
        for (uint32_t i = 0; i < m_numTileRows; i++)
        {
            for (uint32_t j = 0; j < m_numTileColumns; j++)
            {
                uint32_t idx = i * m_numTileColumns + j;
                m_reportTileData[m_statisticsBufIndex][idx].reportValid = isValid;
            }
        }

        return eStatus;
    }

    MOS_STATUS EncodeTile::GetTileBasedStatisticsBuffer(uint32_t idx, MOS_RESOURCE *&buffer) const
    {
        ENCODE_FUNC_CALL();

        if (idx >= EncodeBasicFeature::m_uncompressedSurfaceNum)
        {
            ENCODE_ASSERTMESSAGE("Index exceeds the max number, when try to get TileBasedStatisticsBuffer");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        buffer = const_cast<MOS_RESOURCE *>(&m_resTileBasedStatisticsBuffer[idx]);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::GetTileRecordBuffer(uint32_t idx, MOS_RESOURCE *&buffer)
    {
        ENCODE_FUNC_CALL();

        if (idx >= EncodeBasicFeature::m_uncompressedSurfaceNum)
        {
            ENCODE_ASSERTMESSAGE("Index exceeds the max number, when try to get TileBasedStatisticsBuffer");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        buffer = &m_tileRecordBuffer[idx];

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::GetHucPakAggregatedFrameStatsBuffer(MOS_RESOURCE *&buffer)
    {
        ENCODE_FUNC_CALL();

        buffer = &m_resHuCPakAggregatedFrameStatsBuffer;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::GetStatisticsBufferIndex(uint32_t &idx)
    {
        ENCODE_FUNC_CALL();

        idx = m_statisticsBufIndex;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::GetPrevStatisticsBufferIndex(uint32_t& idx)
    {
        ENCODE_FUNC_CALL();

        idx = m_prevStatisticsBufIndex;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::GetReportTileData(uint32_t idx, const EncodeReportTileData *&reportTileData)
    {
        ENCODE_FUNC_CALL();

        if (idx >= EncodeBasicFeature::m_uncompressedSurfaceNum)
        {
            ENCODE_ASSERTMESSAGE("Index exceeds the max number, when try to get tile report data");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        reportTileData = m_reportTileData[idx];

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeTile::GetTileRowColumns(uint16_t &row, uint16_t &col)
    {
        ENCODE_FUNC_CALL();

        if (!m_enabled)
        {
            row = col = 1;
            return MOS_STATUS_SUCCESS;
        }

        row = m_numTileRows;
        col = m_numTileColumns;

        return MOS_STATUS_SUCCESS;
    }
}  // namespace encode
