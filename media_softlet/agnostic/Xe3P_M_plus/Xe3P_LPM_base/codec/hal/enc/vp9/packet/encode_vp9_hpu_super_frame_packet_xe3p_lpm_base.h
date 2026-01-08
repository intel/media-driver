/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     encode_vp9_hpu_super_frame_packet_xe3p_lpm_base.h
//! \brief    Defines the interface for vp9 HPU super frame packet
//!

#ifndef __ENCODE_VP9_HUC_SUPER_FRAME_PACKET_XE3P_LPM_BASE_H__
#define __ENCODE_VP9_HUC_SUPER_FRAME_PACKET_XE3P_LPM_BASE_H__

#include "encode_vp9_hpu_packet_xe3p_lpm_base.h"
#include "encode_vp9_hpu_super_frame_packet.h"

namespace encode
{
class Vp9HpuSuperFramePktXe3p_Lpm_Base : public Vp9HpuSuperFramePkt
{
public:

    Vp9HpuSuperFramePktXe3p_Lpm_Base(MediaTask *task, Vp9HpuPktXe3p_Lpm_Base *pkt)
        : Vp9HpuSuperFramePkt(task, pkt), m_hucPktExt(pkt)
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hucPktExt);
    }

    virtual ~Vp9HpuSuperFramePktXe3p_Lpm_Base() {}

    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

protected:
    Vp9HpuPktXe3p_Lpm_Base *m_hucPktExt = nullptr;

MEDIA_CLASS_DEFINE_END(encode__Vp9HpuSuperFramePktXe3p_Lpm_Base)
};

}  // namespace encode

#endif  // __ENCODE_VP9_HUC_SUPER_FRAME_PACKET_XE3P_LPM_BASE_H__