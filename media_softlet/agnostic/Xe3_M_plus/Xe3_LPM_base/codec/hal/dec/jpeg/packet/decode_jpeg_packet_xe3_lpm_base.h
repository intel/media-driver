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
//! \file     decode_jpeg_packet_xe3_lpm_base.h
//! \brief    Defines the implementation of jpeg decode packet of Xe2_LPM+
//!

#ifndef __DECODE_JPEG_PACKET_XE3_LPM_H__
#define __DECODE_JPEG_PACKET_XE3_LPM_H__

#include "mhw_vdbox_xe3_lpm_base.h"
#include "codec_hw_xe3_lpm_base.h"
#include "decode_jpeg_packet.h"

namespace decode{

class JpegDecodePktXe3_Lpm_Base : public JpegDecodePkt
{
public:
    JpegDecodePktXe3_Lpm_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) 
        : JpegDecodePkt(pipeline, task, hwInterface)
    {
        m_hwInterface = dynamic_cast<CodechalHwInterfaceXe3_Lpm_Base *>(hwInterface);
    }

    virtual ~JpegDecodePktXe3_Lpm_Base() {}

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override;

protected:
    MOS_STATUS PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer);

    CodechalHwInterfaceXe3_Lpm_Base *m_hwInterface = nullptr;

MEDIA_CLASS_DEFINE_END(decode__JpegDecodePktXe3_Lpm_Base)
};

}

#endif //__DECODE_JPEG_PACKET_XE_LPM_PLUS_H__
