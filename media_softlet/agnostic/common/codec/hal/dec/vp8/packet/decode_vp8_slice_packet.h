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
//! \file     decode_vp8_slice_packet.h
//! \brief    Defines the implementation of vp8 decode slice packet
//!

#ifndef __DECODE_VP8_SLICE_PACKET_H__
#define __DECODE_VP8_SLICE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_vp8_pipeline.h"
#include "decode_utils.h"
#include "decode_vp8_basic_feature.h"
#include "mhw_vdbox_mfx_itf.h"

using namespace mhw::vdbox::mfx;

namespace decode
{

class Vp8DecodeSlcPkt : public DecodeSubPacket, public Itf::ParSetting
{
public:
    Vp8DecodeSlcPkt(Vp8Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
        : DecodeSubPacket(pipeline, hwInterface), m_vp8Pipeline(pipeline)
    {
        if (m_hwInterface != nullptr)
        {
            m_mfxItf       = std::static_pointer_cast<Itf>(m_hwInterface->GetMfxInterfaceNext());
            m_miItf        = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
        }
    }
    virtual ~Vp8DecodeSlcPkt(){};

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
    //! \brief  Execute Vp8 slice packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer) = 0;

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
    virtual MOS_STATUS AddMiFlushDwCmd(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS AddMiBatchBufferEnd(MOS_COMMAND_BUFFER &cmdBuffer);

    MHW_SETPAR_DECL_HDR(MFD_VP8_BSD_OBJECT);

    //!
    //! \brief  Calculate slice level command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual MOS_STATUS CalculateSliceStateCommandSize();


    Vp8Pipeline               *m_vp8Pipeline     = nullptr;
    Vp8BasicFeature           *m_vp8BasicFeature = nullptr;
    DecodeAllocator           *m_allocator       = nullptr;
    std::shared_ptr<Itf>       m_mfxItf          = nullptr;

    // Parameters passed from application
    CODEC_VP8_PIC_PARAMS      *m_vp8PicParams    = nullptr;      //!< Pointer to Vp8 picture parameter
    CODEC_VP8_SLICE_PARAMS    *m_vp8SliceParams  = nullptr;      //!< Pointer to Vp8 slices parameter

    uint32_t m_sliceStatesSize                   = 0;            //!< Slice state command size
    uint32_t m_slicePatchListSize                = 0;            //!< Slice patch list size

MEDIA_CLASS_DEFINE_END(decode__Vp8DecodeSlcPkt)
};

}  // namespace decode
#endif
