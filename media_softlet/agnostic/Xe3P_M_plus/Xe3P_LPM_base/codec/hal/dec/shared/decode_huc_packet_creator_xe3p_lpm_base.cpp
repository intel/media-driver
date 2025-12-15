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
//! \file     decode_huc_packet_creator_xe3p_lpm_base.cpp
//!

#include "decode_huc_packet_creator_xe3p_lpm_base.h"
#include "decode_huc_prob_update_packet_ppgtt.h"

namespace decode
{

CmdPacket *HucPacketCreatorXe3P_Lpm_Base::CreateProbUpdatePkt(
    MediaPipeline           *pipeline,
    MediaTask               *task,
    CodechalHwInterfaceNext *hwInterface)
{
    CmdPacket *probUpdatePkt = MOS_New(HucVp9ProbUpdatePktPpgtt, pipeline, task, hwInterface);
    return probUpdatePkt;
}

}  // namespace decode

