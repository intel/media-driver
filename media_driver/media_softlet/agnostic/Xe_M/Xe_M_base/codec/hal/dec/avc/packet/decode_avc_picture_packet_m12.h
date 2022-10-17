/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_avc_picture_packet_m12.h
//! \brief    Defines the implementation of avc decode picture packet on GEN12
//!

#ifndef __DECODE_AVC_PICTURE_PACKET_M12_H__
#define __DECODE_AVC_PICTURE_PACKET_M12_H__

#include "decode_avc_picture_xe_m_base_packet.h"

namespace decode
{
    class AvcDecodePicPktM12 : public AvcDecodePicPktXe_M_Base
    {
    public:
        AvcDecodePicPktM12(AvcPipeline *pipeline, CodechalHwInterface *hwInterface)
            : AvcDecodePicPktXe_M_Base(pipeline, hwInterface)
        {
            m_hwInterface = hwInterface;
        }
        virtual ~AvcDecodePicPktM12(){};

        //!
        //! \brief  Initialize the media packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        //!
        //! \brief  Execute avc picture packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer) override;

        CodechalHwInterface *m_hwInterface = nullptr;

    protected:
        virtual MOS_STATUS AddMfxPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer) override;
        virtual MOS_STATUS AddMfxPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer) override;

        MOS_STATUS SetSurfaceMmcState(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams);

        //!
        //! \brief    Calculate picture state command size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculatePictureStateCommandSize();
    MEDIA_CLASS_DEFINE_END(decode__AvcDecodePicPktM12)
    };
}  // namespace decode
#endif
