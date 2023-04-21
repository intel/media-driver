/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_hevc_vdenc_packet_xe_lpm_plus.h
//! \brief    Defines the interface to adapt to hevc vdenc xe_lpm_plus encode pipeline
//!

#ifndef __CODECHAL_HEVC_VDENC_PACKET_XE_LPM_PLUS_H__
#define __CODECHAL_HEVC_VDENC_PACKET_XE_LPM_PLUS_H__

#include "encode_hevc_vdenc_packet.h"

namespace encode
{
class HevcVdencPktXe_Lpm_Plus : public HevcVdencPkt
{
public:
    HevcVdencPktXe_Lpm_Plus(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : HevcVdencPkt(pipeline, task, hwInterface)
    {
    }

    virtual ~HevcVdencPktXe_Lpm_Plus(){};

protected:
    MOS_STATUS SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS AddAllCmds_HCP_PAK_INSERT_OBJECT_BRC(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MEDIA_CLASS_DEFINE_END(encode__HevcVdencPktXe_Lpm_Plus)
};

}  // namespace encode

#endif
