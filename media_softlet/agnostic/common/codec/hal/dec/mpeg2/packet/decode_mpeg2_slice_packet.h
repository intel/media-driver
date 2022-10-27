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
//! \file     decode_mpeg2_slice_packet.h
//! \brief    Defines the implementation of mpeg2 decode slice packet
//!

#ifndef __DECODE_MPEG2_SLICE_PACKET_H__
#define __DECODE_MPEG2_SLICE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_mpeg2_pipeline.h"
#include "decode_utils.h"
#include "decode_mpeg2_basic_feature.h"
#include "mhw_vdbox_mfx_itf.h"

using namespace mhw::vdbox::mfx;

namespace decode
{

class Mpeg2DecodeSlcPkt : public DecodeSubPacket, public Itf::ParSetting
{
public:
    Mpeg2DecodeSlcPkt(Mpeg2Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
        : DecodeSubPacket(pipeline, hwInterface), m_mpeg2Pipeline(pipeline)
    {
        if (m_hwInterface != nullptr)
        {
            m_mfxItf = std::static_pointer_cast<Itf>(m_hwInterface->GetMfxInterfaceNext());
            m_miItf        = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
        }
    }
    virtual ~Mpeg2DecodeSlcPkt(){};

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
    //! \brief  Execute mpeg2 slice packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MHW_BATCH_BUFFER& batchBuffer, uint16_t slcIdx) = 0;

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

    //!
    //! \brief  Calculate slice level command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual MOS_STATUS CalculateSliceStateCommandSize();

    MOS_STATUS AddCmd_MFD_MPEG2_BSD_OBJECT(MHW_BATCH_BUFFER &batchBuffer, uint16_t slcIdx);
    MOS_STATUS AddAllCmdsInsertDummySlice(MHW_BATCH_BUFFER &batchBuffer, uint16_t startMB, uint16_t endMB);

    Mpeg2Pipeline               *m_mpeg2Pipeline     = nullptr;
    Mpeg2BasicFeature           *m_mpeg2BasicFeature = nullptr;
    DecodeAllocator             *m_allocator         = nullptr;
    std::shared_ptr<Itf>         m_mfxItf            = nullptr;

    // Parameters passed from application
    CodecDecodeMpeg2PicParams   *m_mpeg2PicParams    = nullptr;      //!< Pointer to MPEG2 picture parameter

    uint32_t m_sliceStatesSize      = 0;  //!< Slice state command size
    uint32_t m_slicePatchListSize   = 0;  //!< Slice patch list size

MEDIA_CLASS_DEFINE_END(decode__Mpeg2DecodeSlcPkt)
};

}  // namespace decode
#endif
