/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     encode_huc.h
//! \brief    Defines the common interface for media huc usage
//! \details  The media huc interface is further sub-divided by different huc usages,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_HUC_H__
#define __ENCODE_HUC_H__

#include "codec_hw_next.h"
#include "mos_defs.h"
#include "media_cmd_packet.h"
#include "encode_pipeline.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_huc_itf.h"
#include "mhw_vdbox_avp_itf.h"
#include "mhw_vdbox_mfx_itf.h"
#include "mhw_mi_itf.h"

#if (_SW_BRC)
#include "encode_sw_brc.h"
#endif  // !_SW_BRC

#include "encode_status_report.h"
#include "encode_status_report_defs.h"

#define HUC_CHK_STATUS_RETURN(_stmt)                                               \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt)

#define HUC_CHK_NULL_RETURN(_stmt)                                               \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_HW, _stmt)

namespace encode
{
    class EncodeHucPkt : public CmdPacket,
        public MediaStatusReportObserver,
        public mhw::vdbox::vdenc::Itf::ParSetting,
        public mhw::vdbox::huc::Itf::ParSetting,
        public mhw::vdbox::avp::Itf::ParSetting,
        public mhw::vdbox::mfx::Itf::ParSetting,
        public mhw::mi::Itf::ParSetting
    {
    public:
        EncodeHucPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext  *hwInterface) : 
            CmdPacket(task),
            m_pipeline(dynamic_cast<EncodePipeline *>(pipeline))
        {
            ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_pipeline);

            m_hwInterface    = hwInterface;
            m_osInterface    = hwInterface->GetOsInterface();
            m_featureManager = m_pipeline->GetFeatureManager();
            m_statusReport   = m_pipeline->GetStatusReportInstance();

            m_miItf = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
            if(m_osInterface)
            {
                m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
            }
            if (!m_userSettingPtr)
            {
                ENCODE_NORMALMESSAGE("Initialize m_userSettingPtr instance failed!");
            }

            m_hucItf   = std::static_pointer_cast<mhw::vdbox::huc::Itf>(hwInterface->GetHucInterfaceNext());
            m_vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
            m_avpItf   = std::static_pointer_cast<mhw::vdbox::avp::Itf>((hwInterface->GetAvpInterfaceNext()));
            m_mfxItf   = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(hwInterface->GetMfxInterfaceNext());
        }

        virtual ~EncodeHucPkt() {}

        virtual MOS_STATUS Init() override;

        virtual MOS_STATUS AllocateResources();

        virtual MOS_STATUS Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function = NONE_BRC);

    protected:

        //!
        //! \brief  Add HUC_PIPE_MODE_SELECT command
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS AddAllCmds_HUC_PIPE_MODE_SELECT(PMOS_COMMAND_BUFFER cmdBuffer) const;

        //!
        //! \brief  Add HUC_IMEM_STATE command
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS AddAllCmds_HUC_IMEM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

        //!
        //! \brief  Store HuCStatus2
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \param  [in] storeHucStatus2Needed
        //!         inditect if sstore huc status2 register needed
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS StoreHuCStatus2Register(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed);

        //!
        //! \brief  Store HuCStatus
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS StoreHuCStatusRegister(PMOS_COMMAND_BUFFER cmdBuffer);

        //!
        //! \brief  One frame is completed
        //! \param  [in] mfxStatus
        //!         pointer to status buffer which for MFX
        //! \param  [in] rcsStatus
        //!         pointer to status buffer which for RCS
        //! \param  [in, out] statusReport
        //!         pointer of EncoderStatusReport
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

        bool IsHuCStsUpdNeeded();

        MOS_STATUS SendPrologCmds(
            MOS_COMMAND_BUFFER &cmdBuffer);

        virtual MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer);

        void SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType);

        MOS_STATUS StartPerfCollect(MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS EndPerfCollect(MOS_COMMAND_BUFFER &cmdBuffer);

#if USE_CODECHAL_DEBUG_TOOL

        virtual MOS_STATUS DumpInput() { return MOS_STATUS_SUCCESS; };

        virtual MOS_STATUS DumpRegion(
            uint32_t    regionNum,
            const char *regionName,
            bool        inputBuffer,
            CodechalHucRegionDumpType dumpType,
            uint32_t    size = 0);
#endif
#if _SW_BRC
        virtual MOS_STATUS InitSwBrc(HuCFunction function);
        std::shared_ptr<EncodeSwBrc> m_swBrc = nullptr;
#endif  // !_SW_BRC

        EncodePipeline         *m_pipeline       = nullptr;
        EncodeAllocator        *m_allocator      = nullptr;
        MediaFeatureManager    *m_featureManager = nullptr;
        CodechalHwInterfaceNext *m_hwInterface   = nullptr;

        std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;
        std::shared_ptr<mhw::vdbox::huc::Itf>   m_hucItf   = nullptr;
        std::shared_ptr<mhw::vdbox::avp::Itf>   m_avpItf   = nullptr;
        std::shared_ptr<mhw::vdbox::mfx::Itf>   m_mfxItf   = nullptr;

        PMOS_RESOURCE      m_resHucStatus2Buffer      = nullptr;
        MHW_VDBOX_NODE_IND m_vdboxIndex               = MHW_VDBOX_NODE_1;
        bool               m_skuFtrEnableMediaKernels = true;
        uint32_t           m_hucStatus2ImemLoadedMask = 0x40;
        bool               m_enableHucStatusReport    = false;

    MEDIA_CLASS_DEFINE_END(encode__EncodeHucPkt)
    };
}
#endif  // !__ENCODE_HUC_H__
