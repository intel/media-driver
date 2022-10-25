/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_avc_slice_xe_m_base_packet.h
//! \brief    Defines the implementation of avc decode slice packet
//!

#ifndef __DECODE_AVC_SLICE_XE_M_BASE_PACKET_H__
#define __DECODE_AVC_SLICE_XE_M_BASE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_avc_pipeline.h"
#include "decode_utils.h"
#include "decode_avc_basic_feature.h"
#include "codechal_hw_g12_X.h"

namespace decode
{

class AvcDecodeSlcPktXe_M_Base : public DecodeSubPacket
{
public:
    AvcDecodeSlcPktXe_M_Base(AvcPipeline *pipeline, CodechalHwInterface *hwInterface)
        : DecodeSubPacket(pipeline, *hwInterface), m_avcPipeline(pipeline)
    {
        m_hwInterface = hwInterface;
        if (m_hwInterface != nullptr)
        {
            m_miInterface  = m_hwInterface->GetMiInterface();
            m_osInterface  = m_hwInterface->GetOsInterface();
            m_mfxInterface  =  static_cast<CodechalHwInterfaceG12*>(hwInterface)->GetMfxInterface();
        }
    }
    virtual ~AvcDecodeSlcPktXe_M_Base(){};

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
    //! \brief  Execute avc slice packet
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t slcIdx) = 0;

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
    virtual MOS_STATUS SetAvcSliceStateParams(MHW_VDBOX_AVC_SLICE_STATE &avcSliceState, uint32_t slcIdx);
    virtual MOS_STATUS SetAvcPhantomSliceParams(MHW_VDBOX_AVC_SLICE_STATE &avcSliceState, uint32_t slcIdx);
    virtual void SetSliceWeightOffsetParams(MHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS &weightOffsetParams, uint32_t slcIdx);
    virtual void SetSliceRefIdxParams(MHW_VDBOX_AVC_REF_IDX_PARAMS &refIdxParams, uint32_t slcIdx);

    //!
    //! \brief  Calculate slice level command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual MOS_STATUS CalculateSliceStateCommandSize();

    AvcPipeline              *m_avcPipeline     = nullptr;
    MhwVdboxMfxInterface     *m_mfxInterface    = nullptr;
    AvcBasicFeature          *m_avcBasicFeature = nullptr;
    DecodeAllocator          *m_allocator        = nullptr;

    // Parameters passed from application
    CODEC_AVC_PIC_PARAMS               *m_avcPicParams                         = nullptr;      //!< Pointer to AVC picture parameter
    CODEC_AVC_SLICE_PARAMS             *m_avcSliceParams                       = nullptr;      //!< Pointer to AVC slices parameter

    uint32_t m_sliceStatesSize      = 0;  //!< Slice state command size
    uint32_t m_slicePatchListSize   = 0;  //!< Slice patch list size
    CodechalHwInterface *m_hwInterface          = nullptr;
    MhwMiInterface      *m_miInterface          = nullptr;
    MEDIA_CLASS_DEFINE_END(decode__AvcDecodeSlcPktXe_M_Base)
};

}  // namespace decode
#endif
