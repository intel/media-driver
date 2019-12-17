/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_cmd_packet.h
//! \brief    Defines the interface for media cmd packet
//! \details  The media cmd packet is dedicated for command buffer sequenece submit
//!
#ifndef __MEDIA_CMD_PACKET_H__
#define __MEDIA_CMD_PACKET_H__

#include "media_packet.h"

class CmdPacket : public MediaPacket
{
public:
    CmdPacket(MediaTask* task) : MediaPacket(task) {}
    virtual ~CmdPacket() { }
    virtual MOS_STATUS Init() override;
    virtual MOS_STATUS Prepare() override;
    virtual MOS_STATUS Destroy() override;
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override
    {
        return MOS_STATUS_SUCCESS;
    }
};
#endif // !__MEDIA_CMD_PACKET_H__
