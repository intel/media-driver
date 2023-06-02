/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_hevc_vdenc_422_packet.h
//! \brief    Defines the interface to adapt to HEVC VDENC pipeline
//!

#ifndef __CODECHAL_HEVC_VDENC_PACKET_422_H__
#define __CODECHAL_HEVC_VDENC_PACKET_422_H__

#include "encode_hevc_vdenc_packet.h"

namespace encode
{
#define MINFRAMESIZE_OFFSET  18

struct SubRSVD
{
    uint32_t DW0;
    uint32_t DW1;
};

struct StreamoutRSVD1
{
    SubRSVD DW[5];
};

struct StreamoutRSVD2
{
    SubRSVD DW[8];
};

struct PakRSVD2
{
    uint32_t DW0;
    uint32_t DW1;
    uint32_t DW2;
    uint32_t DW3;
    uint32_t DW4;
    uint32_t DW5;
    uint32_t DW6;
    uint32_t DW7;
};

struct PakRSVD1
{
    uint32_t DW0;
    uint32_t DW1;
    uint32_t DW2;
    uint32_t DW3;
    uint32_t DW4;
    uint32_t DW5;
    uint32_t DW6;
    uint32_t DW7;
};

class HevcVdencPkt422 : public HevcVdencPkt
{
public:
    HevcVdencPkt422(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
        HevcVdencPkt(pipeline, task, hwInterface) {}
    virtual ~HevcVdencPkt422() {}

    MOS_STATUS AllocateResources() override;

    MOS_STATUS Conversion();

    MOS_STATUS Prepare() override;
    MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

    MOS_STATUS PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS PatchSliceLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase) override;
    MOS_STATUS AddPicStateWithNoTile(MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer) override;
    MOS_STATUS AddHcpPipeModeSelect(MOS_COMMAND_BUFFER &cmdBuffer) override;

    virtual std::string GetPacketName() override
    {
        return "STANDALONE_PAK_PASS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    }

    MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(HCP_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(HCP_PIC_STATE);

protected:
    PMOS_RESOURCE      m_res422MbCodeBuffer = nullptr;  //!< Pointer to MOS_RESOURCE of MbCode buffer
    uint32_t           m_422mvOffset        = 0;        //!< MV data offset, in 64 byte
    uint32_t           m_422mbCodeSize      = 0;        //!< MB code buffer size
    uint32_t           m_422maxNumLCUs      = 0;        //!< MB code buffer size


MEDIA_CLASS_DEFINE_END(encode__HevcVdencPkt422)
};

}  // namespace encode
#endif
