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
//! \file     decode_huc_g12_base.h.h
//! \brief    Defines the common interface for decode huc usage
//! \details  The media huc interface is further sub-divided by different huc usages,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __DECODE_HUC_G12_Base_H__
#define __DECODE_HUC_G12_Base_H__

#include "codechal_hw.h"
#include "mhw_vdbox_huc_interface.h"
#include "mhw_vdbox_hcp_interface.h"
#include "mos_defs.h"
#include "media_cmd_packet.h"
#include "media_pipeline.h"
#include "decode_basic_feature.h"
#include "decodecp_interface.h"
#include "decode_status_report.h"

namespace decode
{
    class DecodePipeline;

    class DecodeHucBasic_G12_Base : public CmdPacket, public MediaStatusReportObserver
    {
    public:
        //!
        //! \brief  Decode huc basic constructor
        //!
        DecodeHucBasic_G12_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface);

        //!
        //! \brief  Decode huc basic destructor
        //!
        virtual ~DecodeHucBasic_G12_Base();

        virtual MOS_STATUS Init() override;

        virtual MOS_STATUS AllocateResources();

        virtual MOS_STATUS Destroy() override;

        //!
        //! \brief  One frame is completed
        //! \param  [in] mfxStatus
        //!         pointer to status buffer which for MFX
        //! \param  [in] rcsStatus
        //!         pointer to status buffer which for RCS
        //! \param  [in, out] statusReport
        //!         pointer of DecodeStatusReport
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Completed(void* mfxStatus, void* rcsStatus, void* statusReport) override;

    protected:
        //!
        //! \brief  Assemble huc commands
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer, bool prologNeeded) = 0;

        virtual void       SetImemParameters(MHW_VDBOX_HUC_IMEM_STATE_PARAMS &imemParams) = 0;
        virtual MOS_STATUS AddHucImem(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

        virtual void       SetHucPipeModeSelectParameters(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &pipeModeSelectParams) = 0;
        virtual MOS_STATUS AddHucPipeModeSelect(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

        virtual void       SetDmemParameters(MHW_VDBOX_HUC_DMEM_STATE_PARAMS &dmemParams) = 0;
        virtual MOS_STATUS AddHucDmem(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

        virtual void       SetRegionParameters(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS &virtualAddrParams) = 0;
        virtual MOS_STATUS AddHucRegion(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

        virtual void       SetIndObjParameters(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS &indObjParams) = 0;
        virtual MOS_STATUS AddHucIndObj(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

        virtual void       SetStreamObjectParameters(MHW_VDBOX_HUC_STREAM_OBJ_PARAMS &streamObjParams,
                                                     CODEC_HEVC_SLICE_PARAMS &sliceParams) = 0;
        virtual MOS_STATUS AddHucStreamObject(MOS_COMMAND_BUFFER &cmdBuffer,
                                              CODEC_HEVC_SLICE_PARAMS &sliceParams) = 0;

        //!
        //! \brief  Store HucStatus MMIO to m_resHucStatusBuffer
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS StoreHucStatusRegister(MOS_COMMAND_BUFFER& cmdBuffer);

        //!
        //! \brief  Store HucStatus2 MMIO to m_resHucStatus2Buffer
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS StoreHucStatus2Register(MOS_COMMAND_BUFFER& cmdBuffer);

        //!
        //! \brief  Force wakeup VDBOX
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \param  [in] mfxWakeup
        //!         MFX pipeline wakeup flag
        //! \param  [in] hcpWakeup
        //!         HCP wakeup flag
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER& cmdBuffer, bool mfxWakeup, bool hcpWakeup);

        //!
        //! \brief  Send prolog cmds
        //! \param  [in] cmdBuffer
        //!         Reference to command buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS SendPrologCmds(MOS_COMMAND_BUFFER& cmdBuffer);

        //!
        //! \brief  Set HucStatus MMIO mask
        //! \param  [in] hucStatusMask
        //!         MMIO mask for HucStatus
        //! \param  [in] hucStatus2Mask
        //!         MMIO mask for HucStatus2
        //!
        void SetHucStatusMask(uint32_t hucStatusMask, uint32_t hucStatus2Mask);

        virtual MOS_STATUS StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;
        virtual MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;

        virtual MOS_STATUS MemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer);

        DecodePipeline *         m_pipeline       = nullptr;
        MediaFeatureManager *    m_featureManager = nullptr;
        DecodeAllocator *        m_allocator      = nullptr;
        CodechalHwInterface *    m_hwInterface    = nullptr;
        MhwVdboxHucInterface *   m_hucInterface   = nullptr;
        MhwVdboxVdencInterface * m_vdencInterface = nullptr;
        DecodeBasicFeature *     m_basicFeature   = nullptr;
        DecodeCpInterface*       m_decodecp       = nullptr;

        static const uint32_t m_hucStatusInvalidMask = 0;         //!< Invalid mask of Huc status MMIO
        uint32_t       m_hucStatusMask = m_hucStatusInvalidMask;  //!< MMIO mask for HuC status
        uint32_t       m_hucStatus2Mask = m_hucStatusInvalidMask; //!< MMIO mask for HuC status2
    MEDIA_CLASS_DEFINE_END(decode__DecodeHucBasic_G12_Base)
    };
}
#endif  // !__DECODE_HUC_G12_Base_H__
