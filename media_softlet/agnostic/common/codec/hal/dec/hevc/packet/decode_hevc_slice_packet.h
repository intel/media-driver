/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_hevc_slice_packet.h
//! \brief    Defines the implementation of hevc decode slice packet
//!

#ifndef __DECODE_HEVC_SLICE_PACKET_H__
#define __DECODE_HEVC_SLICE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_hevc_pipeline.h"
#include "decode_utils.h"
#include "decode_hevc_basic_feature.h"
#include "mhw_vdbox_hcp_itf.h"

namespace decode
{

class HevcDecodeSlcPkt : public DecodeSubPacket, public mhw::vdbox::hcp::Itf::ParSetting
{
public:
    HevcDecodeSlcPkt(HevcPipeline *pipeline, CodechalHwInterfaceNext*hwInterface)
        : DecodeSubPacket(pipeline, hwInterface), m_hevcPipeline(pipeline)
    {
        if (m_hwInterface != nullptr)
        {
            m_hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
            m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
        }
    }
    virtual ~HevcDecodeSlcPkt();

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
    //! \brief  Execute hevc slice packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx) = 0;

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
    virtual MOS_STATUS ValidateSubTileIdx(const HevcTileCoding::SliceTileInfo &sliceTileInfo, uint32_t subTileIdx);
    MOS_STATUS AddHcpCpState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx);

    //!
    //! \brief  Calculate slcie level command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual MOS_STATUS CalculateSliceStateCommandSize();

    virtual MOS_STATUS AddCmd_HCP_PALETTE_INITIALIZER_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx);
    virtual MOS_STATUS SET_HCP_SLICE_STATE(uint32_t sliceIdx, uint32_t subTileIdx);
    virtual MOS_STATUS SET_HCP_REF_IDX_STATE(uint32_t sliceIdx);
    virtual MOS_STATUS SET_HCP_WEIGHTOFFSET_STATE(uint32_t sliceIdx);
    virtual MOS_STATUS AddCmd_HCP_WEIGHTOFFSET_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx);
    virtual MOS_STATUS SET_HCP_BSD_OBJECT(uint32_t sliceIdx, uint32_t subTileIdx);
    virtual MOS_STATUS AddCmd_HCP_BSD_OBJECT(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx);

    HevcPipeline                         *m_hevcPipeline     = nullptr;
    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf           = nullptr;
    HevcBasicFeature                     *m_hevcBasicFeature = nullptr;
    DecodeAllocator                      *m_allocator        = nullptr;

    // Parameters passed from application
    PCODEC_HEVC_PIC_PARAMS       m_hevcPicParams       = nullptr;  //!< Pointer to picture parameter
    PCODEC_HEVC_SLICE_PARAMS     m_hevcSliceParams     = nullptr;  //!< Pointer to slice parameter
    PCODEC_HEVC_EXT_PIC_PARAMS   m_hevcRextPicParams   = nullptr;  //!< Extended pic params for Rext
    PCODEC_HEVC_EXT_SLICE_PARAMS m_hevcRextSliceParams = nullptr;  //!< Extended slice params for Rext
    PCODEC_HEVC_SCC_PIC_PARAMS   m_hevcSccPicParams    = nullptr;  //!< Pic params for SCC

    uint32_t              m_sliceStatesSize    = 0;  //!< Slice state command size
    uint32_t              m_slicePatchListSize = 0;  //!< Slice patch list size
    static const uint32_t m_HevcSccPaletteSize = 96; //!< For HEVC SCC palette size on Gen12+

    //! \brief SLICE_TYPE
    //! \details
    //!  In VDENC mode, for HEVC standard this field can be 0 or 2 only.
    enum SLICE_TYPE
    {
        SLICE_TYPE_B_SLICE         = 0,  //!< No additional details
        SLICE_TYPE_P_SLICE         = 1,  //!< No additional details
        SLICE_TYPE_I_SLICE         = 2,  //!< No additional details
        SLICE_TYPE_ILLEGALRESERVED = 3,  //!< No additional details
    };

MEDIA_CLASS_DEFINE_END(decode__HevcDecodeSlcPkt)
};

} // namespace decode
#endif
