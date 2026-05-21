/*
* Copyright (c) 2024-2026, Intel Corporation
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
//! \file     encode_hevc_huc_brc_update_packet_xe3p_lpm_base.h
//! \brief    Defines the implementation of huc update packet 
//!

#ifndef __CODECHAL_HEVC_HUC_BRC_UPDATE_XE3P_LPM_BASE_PACKET_H__
#define __CODECHAL_HEVC_HUC_BRC_UPDATE_XE3P_LPM_BASE_PACKET_H__

#include "encode_huc_brc_update_packet.h"
#include "encode_huc_ppgtt.h"
#include "huc_kernel_source.h"
#include "encode_hevc_basic_feature_xe3p_lpm_base.h"

namespace encode
{
class HevcHucBrcUpdatePktXe3p_Lpm_Base : public HucBrcUpdatePkt, public EncodeHucPPGTTPkt
    {
    public:
        HevcHucBrcUpdatePktXe3p_Lpm_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
            HucBrcUpdatePkt(pipeline, task, hwInterface),
            EncodeHucPPGTTPkt(hwInterface)
        {
            m_itfPPGTT = std::dynamic_pointer_cast<mhw::vdbox::huc::ItfPPGTT>(m_hucItf);
        }

        virtual ~HevcHucBrcUpdatePktXe3p_Lpm_Base()
        {
            if (m_hucKernelSource != nullptr)
            {
                m_hucKernelSource->ReportMode(m_userSettingPtr);
            }

            return;
        }

        virtual MOS_STATUS Init() override;
        virtual MOS_STATUS Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function = NONE_BRC);

        //!
        //! \brief  Submit HEVC HuC BRC update packet
        //! \details Override to remove SLBB construction logic that has been moved to HEVCHucSLBBUpdatePkt
        //! \param  [in] commandBuffer
        //!         Pointer to command buffer
        //! \param  [in] packetPhase
        //!         Packet phase flag
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

        virtual MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;

    protected:
        virtual MOS_STATUS AllocateResources() override;
        std::shared_ptr<mhw::vdbox::huc::ItfPPGTT> m_itfPPGTT = nullptr;
        bool             m_isPPGTT         = false;
        HucKernelSource *m_hucKernelSource = nullptr;
    MEDIA_CLASS_DEFINE_END(encode__HevcHucBrcUpdatePktXe3p_Lpm_Base)
    };

}  // namespace encode
#endif
