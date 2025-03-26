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
//!
//! \file     vp_npu_cmd_packet.cpp
//! \brief    render packet which used in by mediapipline.
//! \details  render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "vp_npu_cmd_packet.h"

namespace vp
{

VpNpuCmdPacket::VpNpuCmdPacket(MediaTask *task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc) : 
    CmdPacket(task),
    NpuCmdPacket(task, hwInterface ? hwInterface->m_osInterface : nullptr),
    VpCmdPacket(task, hwInterface, allocator, mmc, VP_PIPELINE_PACKET_NPU)
{
    m_levelzeroRuntimeInUse = true;
}

MOS_STATUS VpNpuCmdPacket::PacketInit(
    VP_SURFACE* inputSurface,
    VP_SURFACE* outputSurface,
    VP_SURFACE* previousSurface,
    VP_SURFACE_SETTING& surfSetting,
    VP_EXECUTE_CAPS     packetCaps)
{
    VP_FUNC_CALL();

    // will remodify when normal render path enabled
    VP_UNUSED(inputSurface);
    VP_UNUSED(outputSurface);
    VP_UNUSED(previousSurface);

    m_PacketCaps = packetCaps;

    // Init packet surface params.
    m_surfSetting = surfSetting;

    m_packetResourcesPrepared = false;

    return MOS_STATUS_SUCCESS;
}

}  // namespace vp
