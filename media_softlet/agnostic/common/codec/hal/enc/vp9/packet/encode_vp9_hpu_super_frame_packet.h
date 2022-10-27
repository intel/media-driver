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
//! \file     encode_vp9_hpu_super_frame_packet.h
//! \brief    Defines the interface for vp9 HPU super frame packet
//!

#ifndef __ENCODE_VP9_HUC_SUPER_FRAME_PACKET_H__
#define __ENCODE_VP9_HUC_SUPER_FRAME_PACKET_H__

#include "encode_vp9_hpu_packet.h"

namespace encode
{
class Vp9HpuSuperFramePkt : public CmdPacket
{
public:
    //!
    //! \brief  Vp9HucSuperFramePkt constructor
    //! \param  [in] task
    //!         Pointer to media task
    //! \param  [in] pkt
    //!         Pointer Vp9HucProbPkt packet
    //! \param  [in] hwInterface
    //!         Pointer to HW interface
    //!
    Vp9HpuSuperFramePkt(MediaTask *task, Vp9HpuPkt *pkt)
        : CmdPacket(task),
          m_vp9HucProbPkt(pkt)
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_vp9HucProbPkt);
    }

    //!
    //! \brief  Vp9HucSuperFramePkt destructor
    //!
    virtual ~Vp9HpuSuperFramePkt() {}

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \param  [in] packetPhase
    //!         Indicate packet phase stage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(
        MOS_COMMAND_BUFFER *commandBuffer,
        uint8_t             packetPhase = otherPacket) override;

    //!
    //! \brief  Dump output resources or infomation after submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpOutput() override { return m_vp9HucProbPkt->DumpOutput(); }

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override { return m_vp9HucProbPkt->GetPacketName(); }

protected:
    Vp9HpuPkt *m_vp9HucProbPkt = nullptr;

MEDIA_CLASS_DEFINE_END(encode__Vp9HpuSuperFramePkt)
};

}  // namespace encode

#endif  // __ENCODE_VP9_HUC_SUPER_FRAME_PACKET_H__