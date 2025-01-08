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
//! \file     decode_mpeg2_picture_packet_xe3_lpm_base.h
//! \brief    Defines the implementation of mpeg2 decode picture packet on Xe3_LPM+
//!

#ifndef __DECODE_MPEG2_PICTURE_PACKET_XE3_LPM_BASE_H__
#define __DECODE_MPEG2_PICTURE_PACKET_XE3_LPM_BASE_H__

#include "decode_mpeg2_picture_packet.h"
#include "codec_hw_xe3_lpm_base.h"

namespace decode
{
    class Mpeg2DecodePicPktXe3_Lpm_Base : public Mpeg2DecodePicPkt
    {
    public:
        Mpeg2DecodePicPktXe3_Lpm_Base(Mpeg2Pipeline *pipeline, CodechalHwInterfaceNext* hwInterface)
            : Mpeg2DecodePicPkt(pipeline, hwInterface)
        {
        }
        virtual ~Mpeg2DecodePicPktXe3_Lpm_Base() {};

        //!
        //! \brief  Initialize the media packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        //!
        //! \brief  Execute mpeg2 picture packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer) override;

        virtual MOS_STATUS GetMpeg2StateCommandsDataSize(
            uint32_t  mode,
            uint32_t *commandsSize,
            uint32_t *patchListSize,
            bool      isShortFormat);

    protected:
        //!
        //! \brief    Calculate picture state command size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculatePictureStateCommandSize();

    MEDIA_CLASS_DEFINE_END(decode__Mpeg2DecodePicPktXe3_Lpm_Base)
    };

}  // namespace decode

#endif  // __DECODE_MPEG2_PICTURE_PACKET_XE3_LPM_BASE_H__
