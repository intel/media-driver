/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     decode_av1_picture_packet_xe3_lpm_base.h
//! \brief    Defines the implementation of av1 decode picture packet on Xe3_LPM+
//!

#ifndef __DECODE_AV1_PICTURE_PACKET_XE3_LPM_BASE_H__
#define __DECODE_AV1_PICTURE_PACKET_XE3_LPM_BASE_H__

#include "decode_av1_picture_packet.h"
#include "codec_hw_xe3_lpm_base.h"
#include "decode_downsampling_packet.h"

using namespace mhw::vdbox::avp;

namespace decode
{
    class Av1DecodePicPktXe3_Lpm_Base : public Av1DecodePicPkt
    {
    public:
        Av1DecodePicPktXe3_Lpm_Base(Av1Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
            : Av1DecodePicPkt(pipeline, hwInterface)
        {
            if (m_hwInterface != nullptr)
            {
                m_avpItf = std::static_pointer_cast<mhw::vdbox::avp::Itf>(m_hwInterface->GetAvpInterfaceNext());
                m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
                m_vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
            }
        }

        virtual ~Av1DecodePicPktXe3_Lpm_Base() {};

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

        virtual MOS_STATUS GetAvpStateCommandSize(
            uint32_t                        mode,
            uint32_t                       *commandsSize,
            uint32_t                       *patchListSize,
            PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

        virtual MOS_STATUS GetAvpStateCmdSize(uint32_t *commandsSize, uint32_t *patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params); 

    protected:
        virtual MOS_STATUS VdInit(MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief    Calculate picture state command size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculatePictureStateCommandSize();
        MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);
        MHW_SETPAR_DECL_HDR(AVP_SURFACE_STATE);
        virtual MOS_STATUS GetChromaFormat() override;

#ifdef _DECODE_PROCESSING_SUPPORTED
        DecodeDownSamplingFeature *m_downSamplingFeature = nullptr;
        DecodeDownSamplingPkt *    m_downSamplingPkt     = nullptr;
#endif
        std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__Av1DecodePicPktXe3_Lpm_Base)
    };
}  // namespace decode
#endif  //!__DECODE_AV1_PICTURE_PACKET_Xe3_LPM_BASE_H__
