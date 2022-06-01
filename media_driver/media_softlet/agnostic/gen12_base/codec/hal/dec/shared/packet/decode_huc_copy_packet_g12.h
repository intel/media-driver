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
//! \file     decode_huc_copy_packet_g12.h
//! \brief    Defines the implementation of huc copy packet 
//!

#ifndef __CODECHAL_HUC_COPY_PACKET_G12_H__
#define __CODECHAL_HUC_COPY_PACKET_G12_H__

#include "media_cmd_packet.h"
#include "decode_huc_g12_base.h"
#include "media_pipeline.h"
#include "codechal_hw.h"
#include "decode_utils.h"
#include "decode_huc_copy_packet_itf.h"

namespace decode
{
class HucCopyPktG12 : public DecodeHucBasic_G12_Base, public HucCopyPktItf
    {
    public:

        HucCopyPktG12(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface)
            : DecodeHucBasic_G12_Base(pipeline, task, hwInterface)
        {
        }

        virtual ~HucCopyPktG12() { m_copyParamsList.clear(); }

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

        virtual void       SetImemParameters(MHW_VDBOX_HUC_IMEM_STATE_PARAMS &imemParams) override;
        virtual MOS_STATUS AddHucImem(MOS_COMMAND_BUFFER &cmdBuffer) override;

        virtual void       SetHucPipeModeSelectParameters(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &pipeModeSelectParams) override;
        virtual MOS_STATUS AddHucPipeModeSelect(MOS_COMMAND_BUFFER &cmdBuffer) override;

        virtual void       SetDmemParameters(MHW_VDBOX_HUC_DMEM_STATE_PARAMS &dmemParams) override;
        virtual MOS_STATUS AddHucDmem(MOS_COMMAND_BUFFER &cmdBuffer) override;

        virtual void       SetRegionParameters(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS &virtualAddrParams) override;
        virtual MOS_STATUS AddHucRegion(MOS_COMMAND_BUFFER &cmdBuffer) override;

        virtual void       SetIndObjParameters(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjParams) override;
        virtual MOS_STATUS AddHucIndObj(MOS_COMMAND_BUFFER &cmdBuffer) override;

        virtual void       SetStreamObjectParameters(MHW_VDBOX_HUC_STREAM_OBJ_PARAMS &streamObjParams,
                                                     CODEC_HEVC_SLICE_PARAMS &sliceParams) override;
        virtual MOS_STATUS AddHucStreamObject(MOS_COMMAND_BUFFER &cmdBuffer,
                                              CODEC_HEVC_SLICE_PARAMS &sliceParams) override;

        void SetPerfTag();

        std::vector<HucCopyParams> m_copyParamsList; //!< Copy parameters list
        uint32_t                   m_copyParamsIdx = 0; //!< Copy parameters index
    MEDIA_CLASS_DEFINE_END(decode__HucCopyPktG12)
    };

}  // namespace decode
#endif
