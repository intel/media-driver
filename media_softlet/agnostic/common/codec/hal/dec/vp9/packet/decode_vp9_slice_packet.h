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
//! \file     decode_vp9_slice_packet.h
//! \brief    Defines the implementation of vp9 decode slice packet
//!

#ifndef __DECODE_VP9_SLICE_PACKET_H__
#define __DECODE_VP9_SLICE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_vp9_pipeline.h"
#include "decode_utils.h"
#include "decode_vp9_basic_feature.h"
#include "mhw_vdbox_hcp_itf.h"

namespace decode
{
class Vp9DecodeSlcPkt : public DecodeSubPacket, public mhw::vdbox::hcp::Itf::ParSetting
{
public:
    Vp9DecodeSlcPkt(Vp9Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
        : DecodeSubPacket(pipeline, hwInterface), m_vp9Pipeline(pipeline)
    {
        if (m_hwInterface != nullptr)
        {
            m_hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
            m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
        }
    }
    virtual ~Vp9DecodeSlcPkt(){};

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
    //! \brief  Execute Vp9 slice packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t slcIdx, uint32_t subTileIdx) = 0;

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

    MHW_SETPAR_DECL_HDR(HCP_BSD_OBJECT);
    MOS_STATUS AddHcpCpState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx);

    //!
    //! \brief  Calculate slice level command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual MOS_STATUS CalculateSliceStateCommandSize();

    Vp9Pipeline                          *m_vp9Pipeline     = nullptr;
    Vp9BasicFeature                      *m_vp9BasicFeature = nullptr;
    DecodeAllocator                      *m_allocator       = nullptr;
    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf          = nullptr;

    // Parameters passed from application
    CODEC_VP9_PIC_PARAMS *  m_vp9PicParams   = nullptr;  //!< Pointer to Vp9 picture parameter
    CODEC_VP9_SLICE_PARAMS *m_vp9SliceParams = nullptr;  //!< Pointer to Vp9 slices parameter

    uint32_t m_sliceStatesSize    = 0; //!< Slice state command size
    uint32_t m_slicePatchListSize = 0; //!< Slice patch list size

MEDIA_CLASS_DEFINE_END(decode__Vp9DecodeSlcPkt);
};

} // namespace decode
#endif
