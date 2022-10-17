/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_huc_la_init_packet.h
//! \brief    Defines the interface of Huc Lookahead init packet
//!

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "encode_hevc_vdenc_pipeline.h"

#ifndef __CODECHAL_HUC_LA_INIT_PACKET_H__
#define __CODECHAL_HUC_LA_INIT_PACKET_H__

namespace encode
{
class HucLaInitPkt : public EncodeHucPkt
{
public:
    HucLaInitPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : EncodeHucPkt(pipeline, task, hwInterface)
    {
    }

    virtual ~HucLaInitPkt() {}

    virtual MOS_STATUS Init() override;

    MOS_STATUS         Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

    //!
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "LAINIT";
    }

protected:
    virtual MOS_STATUS AllocateResources() override;
    MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

    HevcBasicFeature *m_basicFeature = nullptr;  //!< Hevc Basic Feature used in each frame

MEDIA_CLASS_DEFINE_END(encode__HucLaInitPkt)
};
} //encode

#endif
