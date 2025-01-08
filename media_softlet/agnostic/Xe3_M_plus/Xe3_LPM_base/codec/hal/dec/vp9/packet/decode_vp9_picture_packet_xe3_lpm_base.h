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
//! \file     decode_vp9_picture_packet_xe3_lpm_base.h
//! \brief    Defines the implementation of vp9 decode picture packet on Xe3_LPM+
//!

#ifndef __DECODE_VP9_PICTURE_PACKET_XE3_LPM_BASE_H__
#define __DECODE_VP9_PICTURE_PACKET_XE3_LPM_BASE_H__

#include "decode_vp9_picture_packet.h"

namespace decode
{
class Vp9DecodePicPktXe3_Lpm_Base : public Vp9DecodePicPkt
    {
    public:
        Vp9DecodePicPktXe3_Lpm_Base(Vp9Pipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
            : Vp9DecodePicPkt(pipeline, hwInterface)
        {
            if (m_hwInterface != nullptr)
            {
                m_vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
            }
        }
        virtual ~Vp9DecodePicPktXe3_Lpm_Base(){};

        //!
        //! \brief  Initialize the media packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        //!
        //! \brief  Execute hevc picture packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer) override;

        //!
        //! \brief  End batch buffer execution if cabac stream out overflow
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ValidateCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS GetVp9StateCommandSize(
            uint32_t                        mode,
            uint32_t                       *commandsSize,
            uint32_t                       *patchListSize,
            PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    protected:
        virtual MOS_STATUS VdInit(MOS_COMMAND_BUFFER &cmdBuffer);

        virtual MOS_STATUS VdScalabPipeLock(MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief    Calculate picture state command size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculatePictureStateCommandSize();

        MOS_STATUS AddAllCmds_HCP_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer);
        MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);
        MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(HCP_VP9_PIC_STATE);

        std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__Vp9DecodePicPktXe3_Lpm_Base)
    };
}  // namespace decode
#endif
