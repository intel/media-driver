/*
* Copyright (c) 2023-2024, Intel Corporation
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
//! \file     decode_huc_s2l_packet_xe3p_lpm_base.h
//! \brief    Defines the implementation of huc S2L packet for Xe3P_LPM_Base
//!

#ifndef __DECODE_HUC_S2L_PACKET_XE3P_LPM_BASE_H__
#define __DECODE_HUC_S2L_PACKET_XE3P_LPM_BASE_H__

#include "decode_huc_s2l_packet.h"
#include "codec_hw_xe3p_lpm_base.h"
#include "mhw_vdbox_huc_itf_ext.h"
#include "huc_kernel_source.h"

namespace decode
{
    struct HucHevcS2lPicBssXe3P_Lpm_Base : public HucHevcS2lPicBss
    {
        uint8_t     IsRealTileEnable;
        uint8_t     NumScalablePipes;
        uint8_t     IsSCCIBCMode;
        uint8_t     IsSCCPLTMode;
        uint8_t     MVRControlIdc;
        uint8_t     UseSliceACTOffset;
        int8_t      pps_act_y_qp_offset;
        int8_t      pps_act_cb_qp_offset;
        int8_t      pps_act_cr_qp_offset;
        uint8_t     PredictorPaletteSize;
        uint16_t    PredictorPaletteEntries[3][128];
        uint32_t    BatchBufferSize;
    };

    struct HucHevcS2lBssXe3P_Lpm_Base
    {
        // Platfrom information
        uint32_t            ProductFamily;
        uint16_t            RevId;
    
        // Flag to indicate if create dummy HCP_REF_IDX_STATE or not
        uint32_t            DummyRefIdxState;

        //Flag to indicate if insert dummy HCP_VD_CONTROL_STATE or not
        uint32_t            DummyVDControlState;

        //Flag to indicate if insert MFX_WAIT and VD_PIPELINE_FLUSH for Scalability
        uint32_t            WaTileFlushScalability;

        // Picture level DMEM data
        HucHevcS2lPicBssXe3P_Lpm_Base PictureBss;

        // Slice level DMEM data
        HucHevcS2lSliceBss SliceBss[CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6];
    };

    class HucS2lPktXe3P_Lpm_Base : public HucS2lPkt, public mhw::vdbox::huc::ItfExt::ParSetting
    {
    public:
        HucS2lPktXe3P_Lpm_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
            : HucS2lPkt(pipeline, task, hwInterface)
        {
            m_hwInterface = dynamic_cast<CodechalHwInterfaceXe3P_Lpm_Base*>(hwInterface);            
        }

        virtual ~HucS2lPktXe3P_Lpm_Base() {}

        virtual MOS_STATUS Init() override;

        MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

        //!
        //! \brief  Get Packet Name
        //! \return std::string
        //!
        virtual std::string GetPacketName() override
        {
            return "S2L_DECODE_PASS" + std::to_string(static_cast<uint32_t>(m_hevcPipeline->GetCurrentPass())) + "_";
        }

        //!
        //! \brief  Prepare interal parameters, should be invoked for each frame
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Prepare() override;

        virtual MOS_STATUS AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer) override;

        virtual MHW_SETPAR_DECL_HDR(HUC_IMEM_ADDR);

    protected:
        virtual MOS_STATUS AllocateResources() override;
        virtual MOS_STATUS Destroy() override;

        virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, bool prologNeeded) override;

        MOS_STATUS PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
        MOS_STATUS PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
        MOS_STATUS SetDmemBuffer();

        virtual MOS_STATUS SetHucDmemPictureBss(HucHevcS2lPicBss &hucHevcS2LPicBss) override;
      
        MOS_STATUS VdPipelineFlush(MOS_COMMAND_BUFFER &cmdBuffer);

        CodechalHwInterfaceXe3P_Lpm_Base *m_hwInterface = nullptr;

        HucKernelSource *m_hucKernelSource = nullptr;

        BufferArray *m_s2lDmemBufferArray = nullptr; //!< S2L DMEM buffer array

        MOS_BUFFER *m_kernelBinBuffer = nullptr;

        bool m_isPpgttMode = false;

    MEDIA_CLASS_DEFINE_END(decode__HucS2lPktXe3P_Lpm_Base)
    };

}  // namespace decode
#endif
