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
//! \file     decode_avc_picture_packet_xe2_lpm_base.h
//! \brief    Defines the implementation of avc decode picture packet on Xe2_LPM+
//!

#ifndef __DECODE_AVC_PICTURE_PACKET_XE2_LPM_BASE_H__
#define __DECODE_AVC_PICTURE_PACKET_XE2_LPM_BASE_H__

#include "decode_avc_picture_packet.h"
#include "codec_hw_xe2_lpm_base.h"
#include "mhw_vdbox_xe2_lpm_base.h"
#include "decode_downsampling_packet.h"

using namespace mhw::vdbox::xe2_lpm_base;

namespace decode
{
class AvcDecodePicPktXe2_Lpm_Base : public AvcDecodePicPkt
    {
    public:
        AvcDecodePicPktXe2_Lpm_Base(AvcPipeline *pipeline, CodechalHwInterfaceNext *hwInterface)
            : AvcDecodePicPkt(pipeline, hwInterface)
        {
        }
        virtual ~AvcDecodePicPktXe2_Lpm_Base(){};

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

    protected:
        MOS_STATUS SetSurfaceMmcState(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams);

        //!
        //! \brief    Calculate picture state command size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculatePictureStateCommandSize();

    MEDIA_CLASS_DEFINE_END(decode__AvcDecodePicPktXe2_Lpm_Base)
    };
}  // namespace decode
#endif  //!__DECODE_AVC_PICTURE_PACKET_XE2_LPM_BASE_H__
