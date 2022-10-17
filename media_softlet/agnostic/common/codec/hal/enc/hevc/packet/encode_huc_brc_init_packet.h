/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_huc_brc_init_packet.h
//! \brief    Defines the implementation of huc init packet
//!

#ifndef __CODECHAL_HUC_BRC_INIT_PACKET_H__
#define __CODECHAL_HUC_BRC_INIT_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "media_pipeline.h"
#include "codec_hw_next.h"
#include "encode_utils.h"
#include "encode_hevc_vdenc_pipeline.h"
#include "encode_hevc_basic_feature.h"

namespace encode
{
    class HucBrcInitPkt : public EncodeHucPkt
    {
    public:
        HucBrcInitPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
            EncodeHucPkt(pipeline, task, hwInterface)
        {
        }

        virtual ~HucBrcInitPkt() {}

        virtual MOS_STATUS Init() override;

        MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

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
        virtual MOS_STATUS CalculateCommandSize(
            uint32_t &commandBufferSize,
            uint32_t &requestedPatchListSize) override;

        //!
        //! \brief  Get Packet Name
        //! \return std::string
        //!
        virtual std::string GetPacketName() override
        {
            return "BRCINIT";
        }

    protected:
        MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);
        MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
        MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);
        virtual MOS_STATUS AllocateResources() override;

        virtual MOS_STATUS SetDmemBuffer() const;

#if USE_CODECHAL_DEBUG_TOOL
        virtual MOS_STATUS DumpHucBrcInit();
#endif
        static constexpr uint32_t               m_vdboxHucHevcBrcInitKernelDescriptor = 8;//!< Huc HEVC Brc init kernel descriptor

        uint32_t m_vdencBrcInitDmemBufferSize = sizeof(VdencHevcHucBrcInitDmem);
        MOS_RESOURCE m_vdencBrcInitDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {}; //!< VDEnc BrcInit DMEM buffer

        HevcBasicFeature    *m_basicFeature = nullptr;  //!< Hevc Basic Feature used in each frame

    MEDIA_CLASS_DEFINE_END(encode__HucBrcInitPkt)
    };

}  // namespace encode
#endif
