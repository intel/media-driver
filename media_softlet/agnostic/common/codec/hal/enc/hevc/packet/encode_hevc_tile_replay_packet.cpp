/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     encode_hevc_tile_replay_packet.cpp
//! \brief    Defines the interface for hevc tile replay packet
//!
#include "encode_hevc_tile_replay_packet.h"
#include "mhw_vdbox.h"

namespace encode {
    MOS_STATUS HevcVdencPicPacket::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        m_hevcVdencPkt->SetSubmitState(HevcVdencPkt::SubmitState::submitPic);
        m_hevcVdencPkt->Submit(commandBuffer, packetPhase);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencTileRowPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        m_hevcVdencPkt->SetSubmitState(HevcVdencPkt::SubmitState::submitTile);
        m_hevcVdencPkt->Submit(commandBuffer, packetPhase);

        return MOS_STATUS_SUCCESS;
    }
}
