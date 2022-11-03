/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_hevc_tile_packet_m12.h
//! \brief    Defines the implementation of hevc decode tile coding packet for M12
//!

#ifndef __DECODE_HEVC_TILE_PACKET_M12_H__
#define __DECODE_HEVC_TILE_PACKET_M12_H__

#include "media_cmd_packet.h"
#include "decode_hevc_pipeline.h"
#include "decode_utils.h"
#include "decode_hevc_basic_feature.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "mhw_mi_g12_X.h"
#include "codechal_hw.h"

namespace decode
{

class HevcDecodeTilePktM12 : public DecodeSubPacket
{
public:
    HevcDecodeTilePktM12(HevcPipeline *pipeline, CodechalHwInterface *hwInterface)
        : DecodeSubPacket(pipeline, *hwInterface), m_hevcPipeline(pipeline)
    {
        m_hwInterface = hwInterface;
        if (m_hwInterface != nullptr)
        {
            m_miInterface  = m_hwInterface->GetMiInterface();
            m_osInterface  = m_hwInterface->GetOsInterface();
            m_hcpInterface = dynamic_cast<MhwVdboxHcpInterfaceG12 *>(hwInterface->GetHcpInterface());
        }
    }
    virtual ~HevcDecodeTilePktM12();

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() override;

    //!
    //! \brief  Execute hevc tile packet for real tile
    //! \param  [in] cmdBuffer
    //!         Commnd buffer to program
    //! \param  [in] tileX
    //!         Tile X offset of slice
    //! \param  [in] tileY
    //!         Tile Y offset of slice
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint16_t tileX, uint16_t tileY);

    //!
    //! \brief  Execute hevc tile packet for virtual tile
    //! \param  [in] cmdBuffer
    //!         Commnd buffer to program
    //! \param  [in] virtualTileIdx
    //!         The index of virtual tile
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint8_t virtualTileIdx);

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

protected:
    virtual MOS_STATUS SetHcpTileCodingParams(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 &tileCodingParams,
                                              uint16_t tileX, uint16_t tileY);

    virtual MOS_STATUS AddHcpTileCoding(MOS_COMMAND_BUFFER &cmdBuffer, uint16_t tileX, uint16_t tileY);

    virtual MOS_STATUS SetHcpTileCodingParams(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 &tileCodingParams,
                                              uint8_t virtualTileIdx);

    virtual MOS_STATUS AddHcpTileCoding(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t virtualTileIdx);

    static constexpr uint32_t m_virtualTileMaxNum = 8; //!< The max num of virtual tile

    HevcPipeline *            m_hevcPipeline     = nullptr; //!< Pointer to hevc pipeline
    MhwVdboxHcpInterfaceG12 * m_hcpInterface     = nullptr; //!< Pointer to hcp hw interface
    HevcBasicFeature *        m_hevcBasicFeature = nullptr; //!< Pointer to hevc basic feature
    PCODEC_HEVC_PIC_PARAMS    m_hevcPicParams    = nullptr; //!< Pointer to picture parameter
    CodechalHwInterface      *m_hwInterface      = nullptr;
    MhwMiInterface           *m_miInterface      = nullptr;
#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t                  m_dbgOvrdWidthInMinCb = 0; //!< debug override for picture width in min ctb
#endif
MEDIA_CLASS_DEFINE_END(decode__HevcDecodeTilePktM12)
};

}  // namespace decode
#endif
