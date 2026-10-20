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
//! \file     encode_avc_huc_brc_init_packet_xe3p_lpm_base.h
//! \brief    Defines the implementation of avc huc init packet
//!

#ifndef __CODECHAL_AVC_HUC_BRC_INIT_PACKET_XE3P_LPM_BASE_H__
#define __CODECHAL_AVC_HUC_BRC_INIT_PACKET_XE3P_LPM_BASE_H__

#include "encode_avc_huc_brc_init_packet.h"
#include "encode_huc_ppgtt.h"
#include "huc_kernel_source.h"

namespace encode
{

class AvcHucBrcInitPktXe3p_Lpm_Base : public AvcHucBrcInitPkt, public EncodeHucPPGTTPkt
{
public:
     AvcHucBrcInitPktXe3p_Lpm_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
         AvcHucBrcInitPkt(pipeline, task, hwInterface),
        EncodeHucPPGTTPkt(hwInterface) 
     {
         m_itfPPGTT = std::dynamic_pointer_cast<mhw::vdbox::huc::ItfPPGTT>(m_hucItf);
     }

    virtual ~AvcHucBrcInitPktXe3p_Lpm_Base()
    {
        if (m_hucKernelSource != nullptr)
        {
            m_hucKernelSource->ReportMode(m_userSettingPtr);
        }

         return;
    }

    virtual MOS_STATUS Init() override;
    virtual MOS_STATUS AddAllCmds_HUC_IMEM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;
    virtual MOS_STATUS Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function = NONE_BRC);
    virtual MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;
    virtual MHW_SETPAR_DECL_HDR(HUC_IMEM_ADDR);

protected:
    virtual MOS_STATUS AllocateResources() override;
    std::shared_ptr<mhw::vdbox::huc::ItfPPGTT> m_itfPPGTT = nullptr;
    bool             m_isPPGTT         = false;
    bool             m_isPxp           = false;
    HucKernelSource *m_hucKernelSource = nullptr;

MEDIA_CLASS_DEFINE_END(encode__AvcHucBrcInitPktXe3p_Lpm_Base)
};

}  // namespace encode

#endif   // !__CODECHAL_AVC_HUC_BRC_INIT_PACKET_XE3P_LPM_BASE_H__
