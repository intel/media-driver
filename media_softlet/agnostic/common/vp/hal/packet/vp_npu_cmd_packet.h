/*
* Copyright (c) 2025, Intel Corporation
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
#ifndef __VP_NPU_CMD_PACKET_H__
#define __VP_NPU_CMD_PACKET_H__
#include "npu_cmd_packet.h"
#include "vp_cmd_packet.h"
#include "vp_base.h"

namespace vp
{

class VpNpuCmdPacket : virtual public NpuCmdPacket, virtual public VpCmdPacket
{
public:
    VpNpuCmdPacket(MediaTask* task, PVP_MHWINTERFACE hwInterface, PVpAllocator& allocator, VPMediaMemComp* mmc);

    virtual ~VpNpuCmdPacket() {};

    MOS_STATUS PacketInit(
        VP_SURFACE         *inputSurface,
        VP_SURFACE         *outputSurface,
        VP_SURFACE         *previousSurface,
        VP_SURFACE_SETTING &surfSetting,
        VP_EXECUTE_CAPS     packetCaps) override;

    MOS_STATUS Prepare() override 
    {
        return NpuCmdPacket::Prepare();
    };

    MOS_STATUS Destroy() override
    {
        return NpuCmdPacket::Destroy();
    };

    MOS_STATUS Init() override
    {
        return NpuCmdPacket::Init();
    };

    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase) override
    {
        return NpuCmdPacket::Submit(commandBuffer, packetPhase);
    };

    MOS_STATUS DumpOutput() override
    {
        return NpuCmdPacket::DumpOutput();
    };

protected:

MEDIA_CLASS_DEFINE_END(vp__VpNpuCmdPacket)
};

}  // namespace vp

#endif
