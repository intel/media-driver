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
#include "vp_cmd_packet.h"
#include "vp_utils.h"
#include "vp_vebox_cmd_packet_g12.h"
#include "vp_packet_factory_g12.h"

using namespace vp;

PacketFactoryG12::PacketFactoryG12()
{
}

PacketFactoryG12::~PacketFactoryG12()
{
}

VpCmdPacket *PacketFactoryG12::CreateVeboxPacket()
{
    return MOS_New(VpVeboxCmdPacketG12, m_pTask, m_pHwInterface, m_pAllocator, m_pMmc);
}

VpCmdPacket *PacketFactoryG12::CreateRenderPacket()
{
    return nullptr;
}
