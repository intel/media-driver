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
//! \file     encode_vp9_hpu_super_frame_packet.cpp
//! \brief    Defines the implementation of vp9 HPU super frame packet
//!

#include "encode_vp9_hpu_super_frame_packet.h"

namespace encode
{

MOS_STATUS Vp9HpuSuperFramePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_vp9HucProbPkt);

    // Enable super frame state
    ENCODE_CHK_STATUS_RETURN(m_vp9HucProbPkt->SetSuperFrameHucPass(true));
    ENCODE_CHK_STATUS_RETURN(m_vp9HucProbPkt->PatchHucProbCommands(commandBuffer, packetPhase));

    return MOS_STATUS_SUCCESS;
}

} // namsespace encode