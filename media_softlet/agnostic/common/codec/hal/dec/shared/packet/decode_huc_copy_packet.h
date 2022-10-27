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
//! \file     decode_huc_copy_packet.h
//! \brief    Defines the implementation of huc copy packet 
//!

#ifndef __CODECHAL_HUC_COPY_PACKET_H__
#define __CODECHAL_HUC_COPY_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_huc.h"
#include "media_pipeline.h"
#include "codec_hw_next.h"
#include "decode_utils.h"
#include "decode_huc_copy_packet_itf.h"
#include "mhw_vdbox_huc_cmdpar.h"
#include "mhw_vdbox_huc_itf.h"
#include "mhw_cmdpar.h"

namespace decode
{
class HucCopyPkt : public DecodeHucBasic, public HucCopyPktItf, public mhw::vdbox::huc::Itf::ParSetting
{
    public:

        HucCopyPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
            : DecodeHucBasic(pipeline, task, hwInterface) {}     
        
        virtual ~HucCopyPkt() { m_copyParamsList.clear(); }

        virtual MOS_STATUS PushCopyParams(HucCopyParams &copyParams) override;

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
            return "HUC_COPY";
        }

    protected:
        virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, bool prologNeeded) override;
        void SetPerfTag();
        virtual MOS_STATUS AddCmd_HUC_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer);
        virtual MOS_STATUS AddHucIndState(MOS_COMMAND_BUFFER &cmdBuffer);
        virtual MHW_SETPAR_DECL_HDR(HUC_IND_OBJ_BASE_ADDR_STATE);
        virtual MHW_SETPAR_DECL_HDR(HUC_STREAM_OBJECT);

        std::vector<HucCopyParams> m_copyParamsList; //!< Copy parameters list
        uint32_t                   m_copyParamsIdx = 0; //!< Copy parameters index

    MEDIA_CLASS_DEFINE_END(decode__HucCopyPkt)
    };

}  // namespace decode
#endif
