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
//! \file     decode_vvc_s2l_packet_xe2_lpm_base.h
//! \brief    Defines the implementation of VVC decode S2L packet
//!

#ifndef __DECODE_VVC_S2L_PACKET_XE2_LPM_BASE_H__
#define __DECODE_VVC_S2L_PACKET_XE2_LPM_BASE_H__

#include "decode_vvc_s2l_packet.h"

namespace decode
{
class VvcDecodeS2LPktXe2_Lpm_Base : public VvcDecodeS2LPkt
{
public:
    VvcDecodeS2LPktXe2_Lpm_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : VvcDecodeS2LPkt(pipeline, task, hwInterface)
    {
    }

    virtual ~VvcDecodeS2LPktXe2_Lpm_Base() {}

    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER &cmdBuffer, bool prologNeeded) override;

    MEDIA_CLASS_DEFINE_END(decode__VvcDecodeS2LPktXe2_Lpm_Base)
};

}  // namespace decode
#endif  // !__DECODE_VVC_S2L_PACKET_XE2_LPM_BASE_H__
