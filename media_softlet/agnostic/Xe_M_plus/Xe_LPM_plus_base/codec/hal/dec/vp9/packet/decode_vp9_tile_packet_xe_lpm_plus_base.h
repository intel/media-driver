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
//! \file     decode_vp9_tile_packet_xe_lpm_plus_base.h
//! \brief    Defines the implementation of vp9 decode tile coding packet for Xe_LPM_plus+
//!

#ifndef __DECODE_VP9_TILE_PACKET_XE_LPM_PLUS_BASE_H__
#define __DECODE_VP9_TILE_PACKET_XE_LPM_PLUS_BASE_H__

#include "media_cmd_packet.h"
#include "decode_vp9_pipeline.h"
#include "decode_utils.h"
#include "decode_vp9_basic_feature.h"
#include "mhw_vdbox_hcp_hwcmd_xe_lpm_plus.h"

namespace decode
{

class Vp9DecodeTilePktXe_Lpm_Plus_Base : public DecodeSubPacket, public mhw::vdbox::hcp::Itf::ParSetting
{
public:
    Vp9DecodeTilePktXe_Lpm_Plus_Base(Vp9Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
        : DecodeSubPacket(pipeline, hwInterface), m_vp9Pipeline(pipeline)
    {
        if (m_hwInterface != nullptr)
        {
            m_hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
            m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
        }
    }
    virtual ~Vp9DecodeTilePktXe_Lpm_Plus_Base();

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
    //! \brief  Execute vp9 tile packet for virtual tile
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
    MOS_STATUS SET_HCP_TILE_CODING(uint8_t virtualTileIdx);
    MOS_STATUS AddCmd_HCP_TILE_CODING(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t virtualTileIdx);

    static constexpr uint32_t m_virtualTileMaxNum = 8; //!< The max num of virtual tile

    Vp9Pipeline                          *m_vp9Pipeline     = nullptr;  //!< Pointer to vp9 pipeline
    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf          = nullptr;
    Vp9BasicFeature                      *m_vp9BasicFeature = nullptr;  //!< Pointer to vp9 basic feature
    PCODEC_VP9_PIC_PARAMS                 m_vp9PicParams    = nullptr;  //!< Pointer to picture parameter

#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t m_dbgOvrdWidthInMinCb = 0; //!< debug override for picture width in min ctb
#endif

MEDIA_CLASS_DEFINE_END(decode__Vp9DecodeTilePktXe_Lpm_Plus_Base)
};

} // namespace decode
#endif
