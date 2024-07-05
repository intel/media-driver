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
//! \file     decode_av1_picture_packet_xe2_lpm_base.h
//! \brief    Defines the implementation of av1 decode picture packet on Xe2_LPM+
//!

#ifndef __DECODE_AV1_PICTURE_PACKET_XE2_LPM_BASE_H__
#define __DECODE_AV1_PICTURE_PACKET_XE2_LPM_BASE_H__

#include "decode_av1_picture_packet.h"
#include "codec_hw_xe2_lpm_base.h"
#include "decode_downsampling_packet.h"

using namespace mhw::vdbox::avp;

namespace decode
{
    class Av1DecodePicPktXe2_Lpm_Base : public Av1DecodePicPkt
    {
    public:
        Av1DecodePicPktXe2_Lpm_Base(Av1Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
            : Av1DecodePicPkt(pipeline, hwInterface)
        {
            if (m_hwInterface != nullptr)
            {
                m_avpItf = std::static_pointer_cast<mhw::vdbox::avp::Itf>(m_hwInterface->GetAvpInterfaceNext());
                m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
            }
        }

        virtual ~Av1DecodePicPktXe2_Lpm_Base() {};

        //!
        //! \brief  Initialize the media packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        //!
        //! \brief  Execute av1 picture packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer) override;

        //!
        //! \brief  Init av1 state commands
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS InitAv1State(MOS_COMMAND_BUFFER& cmdBuffer) override;

    protected:
        virtual MOS_STATUS VdInit(MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief    Calculate picture state command size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculatePictureStateCommandSize();
#ifdef _DECODE_PROCESSING_SUPPORTED
        DecodeDownSamplingFeature *m_downSamplingFeature = nullptr;
        DecodeDownSamplingPkt *    m_downSamplingPkt     = nullptr;
#endif

    MEDIA_CLASS_DEFINE_END(decode__Av1DecodePicPktXe2_Lpm_Base)
    };
}  // namespace decode
#endif  //!__DECODE_AV1_PICTURE_PACKET_XE2_LPM_BASE_H__
