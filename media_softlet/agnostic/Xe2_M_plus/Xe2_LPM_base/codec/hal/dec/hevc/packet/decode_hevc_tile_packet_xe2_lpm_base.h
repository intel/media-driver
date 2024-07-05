/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_hevc_tile_packet_xe2_lpm_base.h
//! \brief    Defines the implementation of hevc decode tile coding packet for Xe2_LPM+
//!

#ifndef __DECODE_HEVC_TILE_PACKET_XE2_LPM_BASE_H__
#define __DECODE_HEVC_TILE_PACKET_XE2_LPM_BASE_H__

#include "media_cmd_packet.h"
#include "decode_hevc_pipeline.h"
#include "decode_utils.h"
#include "decode_hevc_basic_feature.h"
#include "mhw_vdbox_hcp_hwcmd_xe2_lpm.h"

namespace decode
{

class HevcDecodeTilePktXe2_Lpm_Base : public DecodeSubPacket
{
public:
    HevcDecodeTilePktXe2_Lpm_Base(HevcPipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
        : DecodeSubPacket(pipeline, hwInterface), m_hevcPipeline(pipeline)
    {
        if (m_hwInterface != nullptr)
        {
            m_hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
            m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
        }
    }
    virtual ~HevcDecodeTilePktXe2_Lpm_Base();

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
    virtual MOS_STATUS SET_HCP_TILE_CODING(uint16_t tileX, uint16_t tileY);
    virtual MOS_STATUS AddCmd_HCP_Tile_Coding(MOS_COMMAND_BUFFER &cmdBuffer, uint16_t tileX, uint16_t tileY);
    virtual MOS_STATUS SET_HCP_TILE_CODING(uint8_t virtualTileIdx);
    virtual MOS_STATUS AddCmd_HCP_Tile_Coding(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t virtualTileIdx);

    static constexpr uint32_t m_virtualTileMaxNum = 8; //!< The max num of virtual tile

    HevcPipeline                         *m_hevcPipeline     = nullptr; //!< Pointer to hevc pipeline
    HevcBasicFeature                     *m_hevcBasicFeature = nullptr; //!< Pointer to hevc basic feature
    PCODEC_HEVC_PIC_PARAMS                m_hevcPicParams    = nullptr; //!< Pointer to picture parameter
    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf           = nullptr;
#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t                  m_dbgOvrdWidthInMinCb = 0; //!< debug override for picture width in min ctb
#endif

MEDIA_CLASS_DEFINE_END(decode__HevcDecodeTilePktXe2_Lpm_Base)
};

}  // namespace decode
#endif
