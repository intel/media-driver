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
//! \file     encode_vp9_tile.h
//! \brief    Defines the common interface for vp9 tile
//!
#ifndef __ENCODE_VP9_TILE_H__
#define __ENCODE_VP9_TILE_H__

#include "media_cmd_packet.h"
#include "encode_tile.h"
#include "mhw_vdbox_hcp_itf.h"

#include "encode_huc.h"
#include "media_pipeline.h"
#include "encode_utils.h"
#include "encode_vp9_pipeline.h"
#include "encode_vp9_vdenc_pipeline.h"
#include "encode_vp9_basic_feature.h"

namespace encode
{
// Integrated stats information
struct Vp9TileStatusInfo
{
    uint32_t tileSizeRecord  = 0;
    uint32_t vdencStats      = 0;
    uint32_t pakStats        = 0;
    uint32_t counterBuffer   = 0;
};

class Vp9EncodeTile : public EncodeTile, public mhw::vdbox::huc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting, public mhw::vdbox::vdenc::Itf::ParSetting
{
public:
    //!
    //! \brief  Vp9EncodeTile feature constructor
    //! \param  [in] featureManager
    //!         Pointer to MediaFeatureManager
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] constSettings
    //!         Pointer to const settings
    //!
    Vp9EncodeTile(
        MediaFeatureManager *featureManager,
        EncodeAllocator *    allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *               constSettings);

    //!
    //! \brief  Vp9EncodeTile feature destructor
    //!
    virtual ~Vp9EncodeTile() {}

    //!
    //! \brief  Init encode tile feature's parameters setting
    //! \param  [in] setting
    //!         Pointer to CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Update encode tile feature parameters
    //! \param  [in] params
    //!         Pointer to EncoderParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

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
        EncodePipeline *pipeline) override;

    //!
    //! \brief  Set regions for brc update
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRegionsForBrcUpdate(
        mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params) const;

    //!
    //! \brief  Set regions for pak integrate
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRegionsForPakInt(
        mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params) const;

    //!
    //! \brief  Set encode tile features setting to tile coding parameters
    //! \param  [in] activePipes
    //!         Active pipe number
    //! \param  [in, out] params
    //!         Pointer to tile coding parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetHcpTileCodingParams(
        uint32_t activePipes);

    //!
    //! \brief  Set vdenc pipeline buffer address parameters
    //! \param  [out] pipeBufAddrParams
    //!         Reference to Pipe Buffer Address parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetVdencPipeBufAddrParams(
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);

    //!
    //! \brief  Get tile level batch buffer from encode tile features
    //! \param  [out] vp9TileStatsOffset
    //!         Tile Stats offset
    //! \param  [out] vp9FrameStatsOffset
    //!         Frame Stats offset
    //! \param  [out] vp9StatsSize
    //!         Stats size.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetTileStatusInfo(
        Vp9TileStatusInfo &vp9TileStatsOffset,
        Vp9TileStatusInfo &vp9FrameStatsOffset,
        Vp9TileStatusInfo &vp9StatsSize);

    //!
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_TILE_CODING);
    MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_WEIGHTSOFFSETS_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_WALKER_STATE);

protected:
    //!
    //! \brief  Allocate feature related resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources() override;

    //!
    //! \brief  Setup Codechal Tile data for each tile
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetTileData(void *params) override;

    //!
    //! \brief  Allocate Tile Statistics
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateTileStatistics(void *params) override;

    static constexpr uint32_t m_probabilityCounterBufferSize = 193 * CODECHAL_CACHELINE_SIZE;

    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpInterfaceNew = nullptr;

    Vp9TileStatusInfo m_tileStatsOffset  = {};  // Page aligned offsets for HuC PAK Integration kernel input
    Vp9TileStatusInfo m_frameStatsOffset = {};  // Page aligned offsets for HuC PAK Integration kernel output
    Vp9TileStatusInfo m_statsSize        = {};  // Sizes for the stats for HuC PAK Integration kernel input

    MOS_RESOURCE m_resTileRecordStrmOutBuffer            = {0};  //!< Tile record stream out buffer
    MOS_RESOURCE m_resCuStatsStrmOutBuffer               = {0};  //!< CU statistics stream out buffer

MEDIA_CLASS_DEFINE_END(encode__Vp9EncodeTile)
};

}  // namespace encode

#endif  // !__ENCODE_VP9_TILE_H__
