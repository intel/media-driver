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
//! \file     decode_vp9_picture_packet_g12.h
//! \brief    Defines the implementation of vp9 decode picture packet on GEN12
//!

#ifndef __DECODE_VP9_PICTURE_PACKET_M12_H__
#define __DECODE_VP9_PICTURE_PACKET_M12_H__

#include "decode_vp9_picture_packet_xe_m_base.h"

namespace decode
{
    class Vp9DecodePicPktM12 : public Vp9DecodePicPktXe_M_Base
    {
    public:
        Vp9DecodePicPktM12(Vp9Pipeline *pipeline, CodechalHwInterface *hwInterface)
            : Vp9DecodePicPktXe_M_Base(pipeline, hwInterface)
        {
        }
        virtual ~Vp9DecodePicPktM12(){};

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

    protected:
        virtual MOS_STATUS VdInit(MOS_COMMAND_BUFFER &cmdBuffer);

        virtual MOS_STATUS VdScalabPipeLock(MOS_COMMAND_BUFFER &cmdBuffer);

        virtual void SetHcpPipeModeSelectParams(
            MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &pipeModeSelectParamsBase) override;
        virtual MOS_STATUS AddHcpPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer) override;

        virtual MOS_STATUS SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParamsBase) override;
        virtual MOS_STATUS AddHcpPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer) override;

        virtual MOS_STATUS SetHcpPicStateParams(MHW_VDBOX_VP9_PIC_STATE &picStateParams) override;
        virtual MOS_STATUS AddHcpPicStateCmd(MOS_COMMAND_BUFFER &cmdBuffer); 
        virtual MOS_STATUS AddHcpSegmentStateCmd(MOS_COMMAND_BUFFER &cmdBuffer) override;

        //!
        //! \brief    Calculate picture state command size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculatePictureStateCommandSize();
    MEDIA_CLASS_DEFINE_END(decode__Vp9DecodePicPktM12)
    };
}  // namespace decode
#endif
