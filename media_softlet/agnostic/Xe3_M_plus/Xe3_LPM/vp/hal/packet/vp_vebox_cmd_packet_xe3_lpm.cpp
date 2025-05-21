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
//! \file     vp_vebox_cmd_packet_xe3_lpm.cpp
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "vp_vebox_cmd_packet_xe3_lpm.h"

using namespace vp;

VpVeboxCmdPacketXe3_Lpm::VpVeboxCmdPacketXe3_Lpm(MediaTask * task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc, bool disbaleSfcDithering) :
    CmdPacket(task),
    VpCmdPacket(task, hwInterface, allocator, mmc, VP_PIPELINE_PACKET_VEBOX),
    VpVeboxCmdPacketBase(task, hwInterface, allocator, mmc),
    VpVeboxCmdPacket(task, hwInterface, allocator, mmc),
    VpVeboxCmdPacketXe3_Lpm_Base(task, hwInterface, allocator, mmc)
{
}

VpVeboxCmdPacketXe3_Lpm::~VpVeboxCmdPacketXe3_Lpm()
{
}
