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
//! \file     encode_check_huc_load_packet.h
//! \brief    Defines the implementation of check HuC load status packet
//!

#ifndef __ENCODE_CHECK_HUC_LOAD_PACKET_H__
#define __ENCODE_CHECK_HUC_LOAD_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_utils.h"
#include "encode_pipeline.h"

namespace encode
{
    class EncodeCheckHucLoadPkt : public CmdPacket,
        public mhw::vdbox::huc::Itf::ParSetting,
        public mhw::mi::Itf::ParSetting
    {
    public:
        EncodeCheckHucLoadPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext  *hwInterface) :
            CmdPacket(task),
            m_pipeline(dynamic_cast<EncodePipeline *>(pipeline))
        {
            ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_pipeline);

            m_hwInterface = hwInterface;
            m_miItf  = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
            m_hucItf = std::static_pointer_cast<mhw::vdbox::huc::Itf>(hwInterface->GetHucInterfaceNext());
        }

        virtual ~EncodeCheckHucLoadPkt();

        //!
        //! \brief  Initialize the packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Init();

        MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

        PMHW_BATCH_BUFFER Get2ndLevelBatchBuffer(uint32_t currRecycledBufIdx) 
        {
            return &m_2ndLevelBB[currRecycledBufIdx];
        };

    protected:
        MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER *cmdBuffer);
        MOS_STATUS SendPrologCmds(MOS_COMMAND_BUFFER *cmdBuffer);

    private:
        MOS_STATUS PackHucAuthCmds(MOS_COMMAND_BUFFER &cmdBuffer);

        static const uint32_t m_hucLoadInfoMask = 0x1;
        MHW_VDBOX_NODE_IND    m_vdboxIndex      = MHW_VDBOX_NODE_1;

        CodechalHwInterfaceNext *m_hwInterface = nullptr;
        EncodePipeline          *m_pipeline    = nullptr;
        EncodeAllocator         *m_allocator   = nullptr;

        std::shared_ptr<mhw::vdbox::huc::Itf> m_hucItf = nullptr;

        //Resources
        PMOS_RESOURCE      m_hucAuthBuf                                      = nullptr; //!< Pointer to Huc authentication buffer
        MHW_BATCH_BUFFER   m_2ndLevelBB[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};      //!< 2nd level batch buffer
        PMHW_BATCH_BUFFER  m_batchBuf           = nullptr;

        enum COMPARE_OPERATION
        {
            COMPARE_OPERATION_MADGREATERTHANIDD        = 0, //!< If Indirect fetched data is greater than inline data then continue.
            COMPARE_OPERATION_MADGREATERTHANOREQUALIDD = 1, //!< If Indirect fetched data is greater than or equal to inline data then continue.
            COMPARE_OPERATION_MADLESSTHANIDD           = 2, //!< If Indirect fetched data is less than inline data then continue.
            COMPARE_OPERATION_MADLESSTHANOREQUALIDD    = 3, //!< If Indirect fetched data is less than or equal to inline data then continue.
            COMPARE_OPERATION_MADEQUALIDD              = 4, //!< If Indirect fetched data is equal to inline data then continue.
            COMPARE_OPERATION_MADNOTEQUALIDD           = 5, //!< If Indirect fetched data is not equal to inline data then continue.
        };

MEDIA_CLASS_DEFINE_END(encode__CheckHucLoadPkt)
    };
}  // namespace decode
#endif
