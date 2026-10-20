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
//! \file     encode_vp9_hpu_super_frame_packet_xe3p_lpm_base.cpp
//! \brief    Defines the implementation of vp9 HPU super frame packet
//!

#include "encode_vp9_hpu_super_frame_packet_xe3p_lpm_base.h"

namespace encode
{

MOS_STATUS Vp9HpuSuperFramePktXe3p_Lpm_Base::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_hucPktExt);

    // Enable super frame state
    ENCODE_CHK_STATUS_RETURN(m_hucPktExt->SetSuperFrameHucPass(true));
    ENCODE_CHK_STATUS_RETURN(m_hucPktExt->PatchHucProbCommands(commandBuffer, packetPhase));

    return MOS_STATUS_SUCCESS;
}

} // namsespace encode