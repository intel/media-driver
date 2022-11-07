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
//! \file     encode_hevc_tile.h
//! \brief    Defines the common interface for hevc tile
//!
#ifndef __ENCODE_HEVC_TILE_H__
#define __ENCODE_HEVC_TILE_H__

#include "encode_tile.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_hcp_itf.h"

namespace encode
{

struct HevcTileStatusInfo
{
    uint32_t tileSizeRecord = 0;
    uint32_t hevcPakStatistics = 0;
    uint32_t vdencStatistics = 0;
    uint32_t hevcSliceStreamout = 0;
};

struct HevcTileInfo
{
    uint16_t tileId      = 0;
    uint16_t tileNum     = 0;
    uint16_t tileGroupId = 0;

    uint16_t tileColPositionInSb = 0;
    uint16_t tileRowPositionInSb = 0;

    uint16_t tileWidthInSbMinus1  = 0;
    uint16_t tileHeightInSbMinus1 = 0;

    uint32_t tileStartXInLCU = 0;
    uint32_t tileStartYInLCU = 0;
    uint32_t tileEndXInLCU   = 0;
    uint32_t tileEndYInLCU   = 0;
};

class EncodePipeline;

class HevcEncodeTile : public EncodeTile, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting
{
public:
    HevcEncodeTile(MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings);

    virtual ~HevcEncodeTile() {}

    virtual MOS_STATUS Init(void* settings) override;

    //!
    //! \brief  Update encode parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    //!
    //! \brief  Set pipe number
    //! \param  [in] numPipes
    //!         Active pipe number
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetPipeNumber(uint32_t numPipes);

    //!
    //! \brief  Set current tile from slice index
    //! \param  [in] slcCount
    //!         Index of slice
    //! \param  [in] pipeline
    //!         Pipeline used
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurrentTileFromSliceIndex(
        uint32_t        slcCount,
        EncodePipeline *pipeline);

    //!
    //! \brief  Judge if slice is in current tile
    //! \param  [in] sliceNumber
    //!         Index of slice
    //! \param  [in] currentTile
    //!         Tile data for currentTile
    //! \param  [out] sliceInTile
    //!         Pointer to the flag to indicate if slice is in current tile
    //! \param  [out] lastSliceInTile
    //!         Pointer to the flag to indicate if this slice is last slice in tile
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS IsSliceInTile(
        uint32_t        sliceNumber,
        EncodeTileData *currentTile,
        bool *          sliceInTile,
        bool *          lastSliceInTile);

    //!
    //! \brief  Get tile Level Batch Buffer from Encode Tile features
    //! \param  [out] hevcTileStatsOffset
    //!         Tile Stats offset
    //! \param  [out] hevcFrameStatsOffset
    //!         Frame Stats offset
    //! \param  [out] hevcStatsSize
    //!         Stats size.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetTileStatusInfo(
        HevcTileStatusInfo &hevcTileStatsOffset,
        HevcTileStatusInfo &hevcFrameStatsOffset,
        HevcTileStatusInfo &hevcStatsSize);

    //! \brief    Get tile information
    //!
    //! \param    [in] xPosition
    //!           Position of X
    //! \param    [in] xPosition
    //!           Position of Y
    //! \param    [out] tileId
    //!           Tile ID
    //! \param    [out] tileStartLcuX
    //!           Position of X of Tile Start
    //! \param    [out] tileEndLcuX
    //!           Position of X of Tile End
    //! \param    [out] tileStartLcuY
    //!           Position of Y of Tile Start
    //! \param    [out] tileEndLcuY
    //!           Position of Y of Tile End
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetTileInfo(
        uint32_t xPosition,
        uint32_t yPosition,
        uint32_t &tileStartLcuX,
        uint32_t &tileEndLcuX,
        uint32_t &tileStartLcuY,
        uint32_t &tileEndLcuY,
        uint32_t &tileStreaminOffset);

    MOS_STATUS GetTileInfo(HevcTileInfo *hevcTileInfo) const;

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_WALKER_STATE);

    MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(HCP_TILE_CODING);

    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(HCP_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

protected:
    MOS_STATUS SetTileData(void *params) override;

    //!
    //! \brief    Allocate Tile Statistics
    //!
    //! \param  [in] params
    //!         Pointer to parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateTileStatistics(void *params) override;

    //!
    //! \brief  Judge if lcu is in current tile
    //! \param  [in] lcuX
    //!         X coordinate of lcu
    //! \param  [in] lcuY
    //!         Y coordinate of lcu
    //! \param  [in] currentTile
    //!         Tile data for currentTile
    //! \return bool
    //!         True if lcu is inside current tile, else false.
    //!
    bool IsLcuInTile(
        uint32_t        lcuX,
        uint32_t        lcuY,
        EncodeTileData *currentTile);

    //!
    //! \brief  Calculate Lcu number by tile layout
    //!
    //! \param  [in] hevcPicParams
    //!         Picture params used for cal
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateNumLcuByTiles(PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams);

    //!
    //! \brief  Calculate one tile's width and height
    //!
    //! \param  [in] hevcPicParams
    //!         Picture params used for cal
    //! \param  [out] rowBd
    //!         Tile row boundary
    //! \param  [out] colBd
    //!         Tile column boundary
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateTilesBoundary(
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams,
        uint32_t *rowBd,
        uint32_t *colBd);

    //!
    //! \brief  Calculate one tile's width and height
    //!
    //! \param  [in] hevcPicParams
    //!         Picture params used for cal
    //! \param  [in] hevcSeqParams
    //!         Sequence params used for cal
    //! \param  [in] rowIndex
    //!         Tile row index
    //! \param  [in] colIndex
    //!         Tile column index
    //! \param  [in] rowBd
    //!         Tile row boundary
    //! \param  [in] colBd
    //!         Tile column boundary
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateTileWidthAndHeight(
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams,
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
        uint32_t  rowIndex,
        uint32_t  colIndex,
        uint32_t *rowBd,
        uint32_t *colBd);

    MOS_CONTEXT_HANDLE  m_mosCtx = nullptr;

    uint32_t              m_numLcuInPic       = 0;        //!< Total number of LCU in pic cal by each tile

    HevcTileStatusInfo m_hevcTileStatsOffset  = {};   //!< Page aligned offsets used to program HCP / VDEnc pipe and HuC PAK Integration kernel input
    HevcTileStatusInfo m_hevcFrameStatsOffset = {};   //!< Page aligned offsets used to program HuC PAK Integration kernel output, HuC BRC kernel input
    HevcTileStatusInfo m_hevcStatsSize        = {};   //!< HEVC Statistics size

    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf = nullptr;

MEDIA_CLASS_DEFINE_END(encode__HevcEncodeTile)
};

}  // namespace encode

#endif  // !__ENCODE_HEVC_TILE_H__
