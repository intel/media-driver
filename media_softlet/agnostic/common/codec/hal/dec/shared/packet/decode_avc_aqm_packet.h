/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     decode_avc_aqm_packet.h
//! \brief    Defines the implementation of avc decode aqm packet
//!

#ifndef __DECODE_AVC_AQM_PACKET_H__
#define __DECODE_AVC_AQM_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_avc_pipeline.h"
#include "decode_utils.h"
#include "decode_avc_basic_feature.h"
#include "mhw_vdbox_mfx_itf.h"
#include "mhw_vdbox_aqm_itf.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

using namespace mhw::vdbox::mfx;
namespace decode
{
    class AvcDecodeAqmPkt : public DecodeSubPacket, public mhw::vdbox::aqm::Itf::ParSetting
    {
    public:
        //!
        //! \brief  AvcDecodeAqmPkt constructor
        //!
        AvcDecodeAqmPkt(AvcPipeline* pipeline, CodechalHwInterfaceNext* hwInterface)
            : DecodeSubPacket(pipeline, hwInterface), m_avcPipeline(pipeline)
        {
            if (m_hwInterface != nullptr)
            {
                m_mfxItf = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(m_hwInterface->GetMfxInterfaceNext());
                m_miItf = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
                //m_aqmItf = std::make_shared<mhw::vdbox::aqm::Itf>(m_osInterface);
            }
        }

        //!
        //! \brief  AvcDecodeAqmPkt deconstructor
        //!
        virtual ~AvcDecodeAqmPkt();

        //!
        //! \brief  Initialize the media packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        virtual MOS_STATUS Prepare() override;

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
            uint32_t& commandBufferSize,
            uint32_t& requestedPatchListSize) override;

        MHW_SETPAR_DECL_HDR(AQM_HIST_STATE);
        MHW_SETPAR_DECL_HDR(AQM_HIST_BUFF_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(AQM_HIST_FLUSH);
        MHW_SETPAR_DECL_HDR(AQM_VD_CONTROL_STATE);
        MHW_SETPAR_DECL_HDR(AQM_FRAME_START);

    protected:

        //Interfaces
        CodechalHwInterfaceNext* m_hwInterfaceNext = nullptr;
        std::shared_ptr<mhw::vdbox::mfx::Itf> m_mfxItf = nullptr;

        AvcPipeline               *m_avcPipeline = nullptr;
        DecodeAllocator           *m_allocator = nullptr;
        AvcBasicFeature           *m_avcBasicFeature = nullptr;
        CODEC_AVC_PIC_PARAMS      *m_avcPicParams    = nullptr;
        DecodeDownSamplingFeature *m_downSampling = nullptr;

        MEDIA_CLASS_DEFINE_END(decode__AvcDecodeAqmPkt)
    };
}  // namespace decode

#endif // _DECODE_PROCESSING_SUPPORTED
#endif //__DECODE_AVC_AQM_PACKET_H__
