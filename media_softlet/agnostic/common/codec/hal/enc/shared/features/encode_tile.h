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
//! \file     encode_tile.h
//! \brief    Defines the common interface for encode tile
//!
#ifndef __ENCODE_TILE_H__
#define __ENCODE_TILE_H__
#include <array>

#include "encode_basic_feature.h"
#include "encode_pipeline.h"

#define __THIRD_LVL_BB_DEFAULT_COUNT 4

namespace encode
{
struct EncodeTileCodingParams
{
    uint32_t        NumOfTilesInFrame;
    uint32_t        NumOfTileColumnsInFrame;
    uint32_t        TileStartLCUX;
    uint32_t        TileStartLCUY;
    uint16_t        TileHeightInMinCbMinus1;
    uint16_t        TileWidthInMinCbMinus1;
    bool            IsLastTileofColumn;
    bool            IsLastTileofRow;
    uint32_t        TileRowStoreSelect;
    uint32_t        TileColumnStoreSelect;
    uint32_t        Mode;
    bool            IsFirstPass;
    bool            IsLastPass;
    bool            bTileReplayEnable;

    // Offsets for scalability 
    uint32_t            NumberOfActiveBePipes;
    uint32_t            BitstreamByteOffset;
    uint32_t            PakTileStatisticsOffset;
    uint32_t            CuLevelStreamoutOffset;
    uint32_t            SliceSizeStreamoutOffset;
    uint32_t            CuRecordOffset;
    uint32_t            SseRowstoreOffset;
    uint32_t            SaoRowstoreOffset;
    uint32_t            TileSizeStreamoutOffset;
    uint32_t            Vp9ProbabilityCounterStreamoutOffset;
    uint32_t            TileStreaminOffset;
    uint32_t            CumulativeCUTileOffset;
    uint32_t            TileLCUStreamOutOffset;

    PMOS_RESOURCE   presHcpSyncBuffer; // this buffer is not used for either HEVC/VP9 encoder and decoder.

    //Decode specific sparameters
    uint8_t                           ucNumDecodePipes;
    uint8_t                           ucPipeIdx;
};

static constexpr uint32_t m_maxTileBdNum = 100;  //!> Max number of tile boundaries in row or column
static constexpr uint32_t m_codecHalNumTileLevelBatchBuffers = 3;
//!
//! \struct   EncodeTileData
//! \brief    Encode tile data for each tile
//!
struct EncodeTileData
{
    uint32_t numOfTilesInFrame;
    uint32_t numOfTileColumnsInFrame;
    union
    {
        uint32_t tileStartXInLCU;
        uint32_t tileStartXInSb;
        uint32_t tileStartX;
    };
    union
    {
        uint32_t tileEndXInLCU;
        uint32_t tileEndXInSb;
        uint32_t tileEndX;
    };
    union
    {
        uint32_t tileStartYInLCU;
        uint32_t tileStartYInSb;
        uint32_t tileStartY;
    };
    union
    {
        uint32_t tileEndYInLCU;
        uint32_t tileEndYInSb;
        uint32_t tileEndY;
    };
    union
    {
        uint16_t tileHeightInMinCbMinus1;
        uint16_t tileHeightInMinSbMinus1;
        uint16_t tileHeightInMinMinus1;
    };
    union
    {
        uint16_t tileWidthInMinCbMinus1;
        uint16_t tileWidthInMinSbMinus1;
        uint16_t tileWidthInMinMinus1;
    };
    uint32_t tileRowStoreSelect;
    uint32_t tileColumnStoreSelect;
    uint32_t mode;
    bool     isLastTileofColumn;
    bool     isLastTileofRow;
    bool     isFirstPass;  // for tile replay
    bool     isLastPass;   // for tile replay
    bool     tileReplayEnable;

    // Offsets for scalability
    uint32_t bitstreamByteOffset;
    uint32_t pakTileStatisticsOffset;
    uint32_t cuLevelStreamoutOffset;
    uint32_t sliceSizeStreamoutOffset;
    uint32_t cuRecordOffset;
    uint32_t sseRowstoreOffset;
    uint32_t saoRowstoreOffset;
    uint32_t tileSizeStreamoutOffset;
    uint32_t vp9ProbabilityCounterStreamoutOffset;
    uint32_t tileStreaminOffset;
    uint32_t cumulativeCUTileOffset;
    uint32_t tileLCUStreamOutOffset;
};

struct EncodeReportTileData
{
    bool     reportValid = false;
    uint16_t tileHeightInMinCbMinus1 = 0;
    uint16_t tileWidthInMinCbMinus1 = 0;
    uint32_t bitstreamByteOffset = 0;
    uint32_t numTileColumns = 1;
};

class EncodePipeline;

class EncodeTile : public MediaFeature
{
public:
    EncodeTile(MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings);

    virtual ~EncodeTile();

    //!
    //! \brief  Init encode parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *settings);

    //!
    //! \brief  Update encode parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params);

    //! \brief  Allocate feature related resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief  Begin Patching third Level Batch Buffer
    //! \param  [out] cmdBuffer
    //!         Reference of cmdBuffer constructed from tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS BeginPatch3rdLevelBatch(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  End Patching third Level Batch Buffer
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EndPatch3rdLevelBatch();

    //!
    //! \brief  Get third Level Batch Buffer from Encode Tile features
    //! \param  [out] thirdLevelBatchBuffer
    //!         Pointer thirdLevelBatchBuffer got from tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetThirdLevelBatchBuffer(
        PMHW_BATCH_BUFFER &thirdLevelBatchBuffer);

    //!
    //! \brief  Increment the current third Level Batch Buffer from Encode Tile features
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IncrementThirdLevelBatchBuffer();

    //!
    //! \brief  Begin Patching tile Level Batch Buffer
    //! \param  [in] tileRowPass
    //!         Current tile Row Pass
    //! \param  [out] cmdBuffer
    //!         Reference of cmdBuffer constructed from tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS BeginPatchTileLevelBatch(
        uint32_t            tileRowPass,
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  End Patching tile Level Batch Buffer
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EndPatchTileLevelBatch();

    //!
    //! \brief  Get tile Level Batch Buffer from Encode Tile features
    //! \param  [in] tileRowPass
    //!         Tile Row Pass used to get batch buffer
    //! \param  [out] tileLevelBatchBuffer
    //!         Pointer tileLevelBatchBuffer got from tile feature
    //! \param  [out] tileLevelBatchSize
    //!         Tile level batch buffer size
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTileLevelBatchBuffer(
        PMHW_BATCH_BUFFER &tileLevelBatchBuffer);

    //!
    //! \brief  Get tile data from Encode Tile features by tile index
    //! \param  [out] tileData
    //!         Current tile data got from tile feature
    //! \param  [in] index
    //!         Current tile data got from tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTileByIndex(
        EncodeTileData &tileData,
        uint32_t  index);

    MOS_STATUS GetTileIdx(uint32_t &idx)
    {
        idx = m_tileIdx;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Get current codechal tile data Encode Tile features
    //! \param  [out] tileData
    //!         Current tile data got from tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetCurrentTile(
        EncodeTileData &tileData);

    //!
    //! \brief  Set current tile data Encode Tile feature refers to
    //! \param  [in] tileRow
    //!         Tile row idx to set
    //! \param  [in] tileCol
    //!         Tile column idx to set
    //! \param  [in] pipeline
    //!         Pipeline used
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurrentTile(
        uint32_t        tileRow,
        uint32_t        tileCol,
        EncodePipeline *pipeline);

    //!
    //! \brief  Get tile based statistics buffer
    //! \param  [in] idx
    //!         Index of buffer to get
    //! \param  [out] buffer
    //!         Reference to the buffer get from Tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTileBasedStatisticsBuffer(uint32_t idx, MOS_RESOURCE*& buffer) const;

    //!
    //! \brief  Get tile record buffer of PAK integration kernel output
    //! \param  [in] idx
    //!         Index of buffer to get
    //! \param  [out] buffer
    //!         Reference to the buffer get from Tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTileRecordBuffer(uint32_t idx, MOS_RESOURCE *&buffer);

    //!
    //! \brief  Get Pak Aggregated Frame Stats Buffer buffer as PAK integration kernel output
    //! \param  [out] buffer
    //!         Reference to the buffer get from Tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHucPakAggregatedFrameStatsBuffer(MOS_RESOURCE *&buffer);

    //!
    //! \brief  Get tile statistic index
    //! \param  [out] idx
    //!         Reference to the buffer index get from Tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetStatisticsBufferIndex(uint32_t &idx);

    //!
    //! \brief  Get tile statistic index of previous frame
    //! \param  [out] idx
    //!         Reference to the buffer index get from Tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetPrevStatisticsBufferIndex(uint32_t& idx);

    //!
    //! \brief  Get tile statistic index
    //! \param  [out] idx
    //!         Reference to the buffer index get from Tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTileNum(uint32_t &num)
    {
        num = m_numTiles;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Get tile report data buffer
    //! \param  [in] idx
    //!         Index of buffer to get
    //! \param  [out] buffer
    //!         Reference to the buffer get from Tile feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetReportTileData(uint32_t idx, const EncodeReportTileData *&reportTileData);

    //!
    //! \brief  Get tile row and columns number from Tile feature
    //! \param  [out] row
    //!         Tile row number
    //! \param  [out] col
    //!         Tile column number
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTileRowColumns(uint16_t &row, uint16_t &col);

    //!
    //! \brief    Setup if Codechal Tile data
    //!
    //! \param  [in] isValid
    //!         Indicate if in current frame tile report data valid
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetTileReportDataVaild(bool isValid);

    //!
    //! \brief    Check if tile feature is enabled
    //! \param    [out] enabled
    //!           Enabled flag
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsEnabled(bool &enabled)
    {
        enabled = m_enabled;
        return MOS_STATUS_SUCCESS;
    }


    //!
    //! \brief    Check if tile replay feature is enabled
    //! \param    [out] enabled
    //!           Enabled flag
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS IsTileReplayEnabled(bool& enabled)
    {
        enabled = m_enableTileReplay;
        return MOS_STATUS_SUCCESS;
    }

protected:
    //! \brief    Allocate the batch buffer for each tile
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateTileLevelBatch();

    //! \brief    Allocate the BRC batch buffer for each tile row
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateTileRowLevelBRCBatch() { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Free the batch buffer for each tile row level BRC
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FreeTileRowLevelBRCBatch() { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Free the batch buffer for each tile
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FreeTileLevelBatch();

    //!
    //! \brief    Setup Codechal Tile data for each tile
    //!
    //! \param  [in] params
    //!         Pointer to parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetTileData(void *params) = 0;

    //!
    //! \brief    Setup Codechal Tile data to report
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetTileReportData();

    //!
    //! \brief    Allocate Tile Statistics
    //!
    //! \param  [in] params
    //!         Pointer to parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateTileStatistics(void *params) = 0;

    EncodeAllocator          *m_allocator         = nullptr;  //!< Allocator used in tile feature
    EncodeBasicFeature       *m_basicFeature      = nullptr;  //!< EncodeBasicFeature
    CodechalHwInterfaceNext  *m_hwInterface       = nullptr;  //!< Hw interface as utilities
    MediaFeatureManager      *m_featureManager    = nullptr;  //!< Pointer to feature manager

    EncodeTileData       *m_tileData          = nullptr;  //!< Pointer to the Tile data array
    uint32_t              m_numTiles          = 1;        //!< Total tile numbers
    uint32_t              m_tileIdx           = 0;        //!< Indicate current tile index
    uint32_t              m_maxTileNumber     = 0;        //!< Max tile number for curent frame
    uint32_t              m_maxTileNumberUsed = 0;        //!< Max tile number used for create tile data buffers
    uint32_t              m_tileRowPass       = 0;        //!< Indicate current tile row pass
    uint16_t              m_numTileRows       = 1;        //!< Total number of tile rows
    uint16_t              m_numTileColumns    = 1;        //!< Total number of tile columns

    //MOS_RESOURCE      m_vdencTileRowStoreBuffer;         //!< Tile row store buffer
    //MOS_RESOURCE      m_vdencPaletteModeStreamOutBuffer; //!< Palette mode stream out buffer
    //MOS_RESOURCE      m_resHwCountTileReplay;            //!< Tile based HW Counter buffer

    EncodeTileCodingParams m_curTileCodingParams = {};  //! Current tile coding paramters

    // 3rd Level Batch buffer
    uint32_t                                                                m_thirdLevelBatchSize   = 0;   //!< Size of the 3rd level batch buffer
    std::array<MHW_BATCH_BUFFER, __THIRD_LVL_BB_DEFAULT_COUNT>              m_thirdLevelBatchBuffers = { 0 };  //!< 3rd level batch buffer
    std::array<MHW_BATCH_BUFFER, __THIRD_LVL_BB_DEFAULT_COUNT>::iterator    m_currentThirdLevelBatchBuffer;   

    // Tile level batch buffer
    uint32_t          m_tileLevelBatchSize    = 0;    //!< Size of the 2rd level batch buffer for each tile
    uint32_t          m_numTileBatchAllocated[m_codecHalNumTileLevelBatchBuffers] ={0};    //!< The number of allocated batch buffer for tiles
    uint32_t          m_tileBatchBufferIndex = 0;     //!< Current index for tile batch buffer of same frame, updated per frame
    PMHW_BATCH_BUFFER m_tileLevelBatchBuffer[m_codecHalNumTileLevelBatchBuffers][EncodeBasicFeature::m_vdencBrcPassNum] = {{0}};  //!< Tile level batch buffer for each tile
    MOS_RESOURCE      m_resTileBasedStatisticsBuffer[EncodeBasicFeature::m_uncompressedSurfaceNum] = {};
    MOS_RESOURCE      m_resHuCPakAggregatedFrameStatsBuffer = {};
    MOS_RESOURCE      m_tileRecordBuffer[EncodeBasicFeature::m_uncompressedSurfaceNum] = {};

    EncodeReportTileData *m_reportTileData[EncodeBasicFeature::m_uncompressedSurfaceNum] = {};
    uint8_t           m_prevStatisticsBufIndex = 0;
    uint8_t           m_statisticsBufIndex     = 0;

    bool              m_enableTileReplay = false;  //!< TileReplay Enable

MEDIA_CLASS_DEFINE_END(encode__EncodeTile)
};

}  // namespace encode

#endif  // !__ENCODE_TILE_H__
