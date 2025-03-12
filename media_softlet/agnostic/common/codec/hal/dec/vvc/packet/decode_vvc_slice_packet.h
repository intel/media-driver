/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_vvc_slice_packet.h
//! \brief    Defines the implementation of VVC decode slice packet
//!

#ifndef __DECODE_VVC_TILE_PACKET_H__
#define __DECODE_VVC_TILE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_vvc_pipeline.h"
#include "decode_utils.h"
#include "decode_vvc_basic_feature.h"
#include "mhw_vdbox_vvcp_itf.h"

using namespace mhw::vdbox::vvcp;

namespace decode
{
class VvcDecodeSlicePkt : public DecodeSubPacket, public Itf::ParSetting
{
public:
    VvcDecodeSlicePkt(VvcPipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
        : DecodeSubPacket(pipeline, hwInterface), m_vvcPipeline(pipeline)
    {
        if (hwInterface != nullptr)
        {
            m_hwInterface = dynamic_cast<CodechalHwInterfaceNext *>(hwInterface);
            if (m_hwInterface != nullptr)
            {
                m_vvcpItf     = std::static_pointer_cast<mhw::vdbox::vvcp::Itf>(m_hwInterface->GetVvcpInterfaceNext());
                m_vdencItf    = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
                m_miItf       = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
            }
        }
    }
    virtual ~VvcDecodeSlicePkt(){};

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
    //! \brief  Execute VVC tile packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint16_t tileIdx);

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
    MOS_STATUS CalculateTileCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize);

protected:
    virtual MOS_STATUS CalcRefIdxSymLx(int8_t& RefIdxSymL0, int8_t& RefIdxSymL1);
    virtual MOS_STATUS ConstructLmcsReshaper() const;
    virtual MOS_STATUS SetRefIdxStateParams();

    bool IsTileInRasterSlice(const uint32_t tileRow, const uint32_t tileCol) const;

    //!
    //! \brief  Calculate tile level command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual MOS_STATUS CalculateSliceStateCommandSize();

    //!
    //! \brief  Get partition info for the current slice
    //!
    //! \return MOS_STATUS
    //!         status
    //!
    MOS_STATUS GetPartitionInfo(uint16_t sliceIdx);

    // VVCP MHW functions
    MOS_STATUS AddAllCmds_VVCP_REF_IDX_STATE(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS AddAllCmds_VVCP_WEIGHTOFFSET_STATE(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS AddAllCmds_VVCP_TILE_CODING(MOS_COMMAND_BUFFER &cmdBuffer);

    MHW_SETPAR_DECL_HDR(VVCP_SLICE_STATE);
    MHW_SETPAR_DECL_HDR(VVCP_BSD_OBJECT);
    MHW_SETPAR_DECL_HDR(VVCP_TILE_CODING);

    VvcPipeline *                           m_vvcPipeline     = nullptr;
    std::shared_ptr<Itf>                    m_vvcpItf         = nullptr;
    std::shared_ptr<mhw::mi::Itf>           m_miItf           = nullptr;
    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf        = nullptr;
    VvcBasicFeature *                       m_vvcBasicFeature = nullptr;
    DecodeAllocator *                       m_allocator       = nullptr;
    DecodeSubPacket *                       m_vvcCpSubPkt     = nullptr;       //!< Pointer to VVC CP packet
    CodechalHwInterfaceNext *               m_hwInterface     = nullptr;

    // Parameters passed from application
    CodecVvcPicParams               *m_vvcPicParams   = nullptr;      //!< Pointer to VVC picture parameter
    CodecVvcSliceParams             *m_curSliceParams = nullptr;      //!< Pointer to current VVC slice parameter
    int16_t                         m_numTilesInSlice = 0;            //!< Number of tiles in the current slice

    //Internal params
    //Current Slice params
    SliceDescriptor                 *m_sliceDesc     = nullptr;       //slice descriptor for the current slice
    bool                            m_lastSliceOfPic = false;

    //Current SubPic
    CodecVvcSubpicParam             *m_subPicParams  = nullptr;

    uint32_t m_sliceStatesSize      = 0;  //!< Slice state command size
    uint32_t m_slicePatchListSize   = 0;  //!< Slice patch list size
    uint32_t m_tileStateSize        = 0;  //!< Tile state command size
    uint32_t m_tilePatchListSize    = 0;  //!< Tile patch list size
    int16_t  m_curTileIdx           = 0;  //!< Current tile idx


MEDIA_CLASS_DEFINE_END(decode__VvcDecodeSlicePkt)
};

}  // namespace decode

#endif
