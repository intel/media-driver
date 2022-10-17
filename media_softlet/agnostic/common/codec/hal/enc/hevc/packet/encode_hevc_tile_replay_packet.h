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
//! \file     encode_hevc_tile_replay_packet.h
//! \brief    Defines the implementation of hevc tile replay packet
//!

#ifndef __CODECHAL_HEVC_TILE_REPLAY_PACKET_H__
#define __CODECHAL_HEVC_TILE_REPLAY_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "media_pipeline.h"
#include "codec_hw_next.h"
#include "encode_utils.h"
#include "encode_hevc_vdenc_pipeline.h"
#include "encode_hevc_basic_feature.h"
#include "encode_status_report.h"
#include "encode_hevc_vdenc_packet.h"

namespace encode
{
class HevcVdencPicPacket : public CmdPacket, public MediaStatusReportObserver
{
public:
    HevcVdencPicPacket(MediaTask *task, HevcVdencPkt *pkt) :
        CmdPacket(task),
        m_hevcVdencPkt(pkt) { }

    virtual ~HevcVdencPicPacket() {}

    virtual MOS_STATUS Submit(
        MOS_COMMAND_BUFFER* commandBuffer,
        uint8_t packetPhase = otherPacket) override;

    virtual MOS_STATUS Init() override { return m_hevcVdencPkt->Init(); };
    virtual MOS_STATUS Prepare() override { return m_hevcVdencPkt->Prepare(); };
    virtual MOS_STATUS Destroy() override { return m_hevcVdencPkt->Destroy(); }; // TYPO: Should be destroy...
    MOS_STATUS CalculateCommandSize(uint32_t& commandBufferSize, uint32_t& requestedPatchListSize) override { return m_hevcVdencPkt->CalculateCommandSize(commandBufferSize, requestedPatchListSize); }

    virtual MOS_STATUS Completed(void* mfxStatus, void* rcsStatus, void* statusReport) override { return m_hevcVdencPkt->Completed(mfxStatus, rcsStatus, statusReport); };

protected:
    HevcVdencPkt* m_hevcVdencPkt = nullptr;

MEDIA_CLASS_DEFINE_END(encode__HevcVdencPicPacket)
};

class HevcVdencTileRowPkt : public CmdPacket, public MediaStatusReportObserver
{
public:
    HevcVdencTileRowPkt(MediaTask *task, HevcVdencPkt *pkt) :
        CmdPacket(task),
        m_hevcVdencPkt(pkt) { }

    virtual ~HevcVdencTileRowPkt() {}

    virtual MOS_STATUS Submit(
        MOS_COMMAND_BUFFER* commandBuffer,
        uint8_t packetPhase = otherPacket) override;

    virtual MOS_STATUS Completed(void* mfxStatus, void* rcsStatus, void* statusReport) override { return MOS_STATUS_SUCCESS; };

protected:

    HevcVdencPkt* m_hevcVdencPkt = nullptr;

MEDIA_CLASS_DEFINE_END(encode__HevcVdencTileRowPkt)
};
}  // namespace encode
#endif
