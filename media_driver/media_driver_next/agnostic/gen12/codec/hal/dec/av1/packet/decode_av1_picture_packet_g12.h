/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     decode_av1_picture_packet_g12.h
//! \brief    Defines the implementation of av1 decode picture packet on GEN12
//!

#ifndef __DECODE_AV1_PICTURE_PACKET_G12_H__
#define __DECODE_AV1_PICTURE_PACKET_G12_H__

#include "decode_av1_picture_packet.h"

namespace decode
{
    class Av1DecodePicPktG12 : public Av1DecodePicPkt
    {
    public:
        Av1DecodePicPktG12(Av1Pipeline *pipeline, CodechalHwInterface *hwInterface)
            : Av1DecodePicPkt(pipeline, hwInterface)
        {
        }
        virtual ~Av1DecodePicPktG12(){};

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

    protected:
        virtual MOS_STATUS VdInit(MOS_COMMAND_BUFFER &cmdBuffer);

        virtual void SetAvpPipeModeSelectParams(
            MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 &vdboxPipeModeSelectParams) override;
        virtual MOS_STATUS AddAvpPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer) override;

        virtual MOS_STATUS AddAvpPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer) override;
        MOS_STATUS         SetSurfaceMmcState(MhwVdboxAvpPipeBufAddrParams& pipeBufAddrParams);

        virtual MOS_STATUS SetAvpPicStateParams(MhwVdboxAvpPicStateParams &picStateParams) override;
        virtual MOS_STATUS SetInterPredStateParams(MhwVdboxAvpPicStateParams &picStateParams);
        virtual MOS_STATUS SetInloopFilterStateParams(MhwVdboxAvpPicStateParams &picStateParams) override;
        virtual MOS_STATUS AddAvpInterPredStateCmd(MOS_COMMAND_BUFFER &cmdBuffer);
        virtual MOS_STATUS AddAvpPicStateCmd(MOS_COMMAND_BUFFER &cmdBuffer);
        virtual MOS_STATUS AddAvpInloopFilterStateCmd(MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief    Calculate picture state command size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculatePictureStateCommandSize();
    };
}  // namespace decode
#endif
