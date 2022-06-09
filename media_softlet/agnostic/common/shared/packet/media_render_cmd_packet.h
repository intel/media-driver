/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     media_render_cmd_packet.h
//! \brief    Defines the interface for media render workload cmd packet
//! \details  The media cmd packet is dedicated for command buffer sequenece submit
//!
#ifndef __MEDIA_RENDER_CMD_PACKET_H__
#define __MEDIA_RENDER_CMD_PACKET_H__

#include "media_render_cmd_packet_next.h"

class RenderCmdPacket : public RenderCmdPacketNext
{
public:
    RenderCmdPacket(MediaTask *task, PMOS_INTERFACE pOsinterface, RENDERHAL_INTERFACE *m_renderHal);

    virtual ~RenderCmdPacket();
    //virtual MOS_STATUS Init();
    //virtual MOS_STATUS Destroy();
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket);

MEDIA_CLASS_DEFINE_END(RenderCmdPacket)
};

#endif // __MEDIA_RENDER_CMD_PACKET_H__
