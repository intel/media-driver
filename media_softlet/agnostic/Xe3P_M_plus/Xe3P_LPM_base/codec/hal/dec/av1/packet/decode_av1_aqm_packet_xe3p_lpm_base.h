/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     decode_av1_aqm_packet_xe3p_lpm_base.h
//! \brief    Defines the implementation of av1 decode aqm packet for Xe3P_LPM_Base
//!

#ifndef __DECODE_AV1_AQM_PACKET_XE3P_LPM_BASE_H__
#define __DECODE_AV1_AQM_PACKET_XE3P_LPM_BASE_H__

#include "decode_av1_aqm_packet.h"
#ifdef _MEDIA_RESERVED
#include "mhw_vdbox_aqm_impl_xe3p_lpm.h"
#endif
#include "decode_av1_downsampling_feature_xe3p_lpm_base.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

using namespace mhw::vdbox::avp;
namespace decode
{
    class Av1DecodeAqmPktXe3PLpmBase : public Av1DecodeAqmPkt
    {
    public:
        //!
        //! \brief  Av1DecodeAqmPktXe3PLpmBase constructor
        //!
        Av1DecodeAqmPktXe3PLpmBase(Av1Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
            : Av1DecodeAqmPkt(pipeline, hwInterface)
        {
            if (m_hwInterface != nullptr)
            {
                m_avpItf = std::static_pointer_cast<mhw::vdbox::avp::Itf>(m_hwInterface->GetAvpInterfaceNext());
                m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
#ifdef _MEDIA_RESERVED
                m_aqmItf = std::make_shared<mhw::vdbox::aqm::xe3p_lpm_base::xe3p_lpm::Impl>(m_osInterface);
#endif
            }
        }

        //!
        //! \brief  Av1DecodeAqmPktXe3PLpmBase deconstructor
        //!
        virtual ~Av1DecodeAqmPktXe3PLpmBase();

        //!
        //! \brief  Initialize the media packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        //! \brief  Execute av1 aqm packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER &cmdBuffer);

        //! \brief  Flush av1 aqm packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Flush(MOS_COMMAND_BUFFER &cmdBuffer);

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

        MHW_SETPAR_DECL_HDR(AQM_HIST_BUFF_ADDR_STATE);

        Av1DownSamplingFeatureXe3P_Lpm_Base *m_downSampling = nullptr;

    protected:
#ifdef _MEDIA_RESERVED
        std::shared_ptr<mhw::vdbox::aqm::Itf> m_aqmItf = nullptr;
#endif

        MEDIA_CLASS_DEFINE_END(decode__Av1DecodeAqmPktXe3PLpmBase)
    };

}  // namespace decode
#endif //_DECODE_PROCESSING_SUPPORTED

#endif