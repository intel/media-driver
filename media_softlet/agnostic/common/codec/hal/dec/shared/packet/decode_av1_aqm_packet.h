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
//! \file     decode_av1_aqm_packet.h
//! \brief    Defines the implementation of av1 decode aqm packet
//!

#ifndef __DECODE_AV1_AQM_PACKET_H__
#define __DECODE_AV1_AQM_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_av1_pipeline.h"
#include "decode_utils.h"
#include "decode_av1_basic_feature.h"
#include "mhw_vdbox_avp_itf.h"
#include "mhw_vdbox_aqm_itf.h"
#include "decode_downsampling_feature.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

using namespace mhw::vdbox::avp;
namespace decode
{
    class Av1DecodeAqmPkt : public DecodeSubPacket, public mhw::vdbox::aqm::Itf::ParSetting
    {
    public:
        //!
        //! \brief  Av1DecodeAqmPkt constructor
        //!
        Av1DecodeAqmPkt(Av1Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
            : DecodeSubPacket(pipeline, hwInterface), m_av1Pipeline(pipeline)
        {
            if (m_hwInterface != nullptr)
            {
                m_avpItf = std::static_pointer_cast<mhw::vdbox::avp::Itf>(m_hwInterface->GetAvpInterfaceNext());
                m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
            }
        }

        //!
        //! \brief  Av1DecodeAqmPkt deconstructor
        //!
        virtual ~Av1DecodeAqmPkt();

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
        CodechalHwInterfaceNext *m_hwInterfaceNext = nullptr;
        std::shared_ptr<mhw::vdbox::avp::Itf> m_avpItf = nullptr;

        Av1Pipeline               *m_av1Pipeline     = nullptr;
        DecodeAllocator           *m_allocator       = nullptr;
        Av1BasicFeature           *m_av1BasicFeature = nullptr;
        CodecAv1PicParams         *m_av1PicParams    = nullptr;
        DecodeDownSamplingFeature *m_downSampling    = nullptr;

MEDIA_CLASS_DEFINE_END(decode__Av1DecodeAqmPkt)
    };

}  // namespace decode

#endif //_DECODE_PROCESSING_SUPPORTED
#endif //__DECODE_AV1_AQM_PACKET_H__
