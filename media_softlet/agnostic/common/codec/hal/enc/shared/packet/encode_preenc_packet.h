/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     encode_preenc_packet.h
//! \brief    Defines the interface to adapt to preenc pipeline
//!

#ifndef __CODECHAL_PREENC_PACKET_H__
#define __CODECHAL_PREENC_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_pipeline.h"
#include "encode_utils.h"
#include "encode_preenc_basic_feature.h"
#include "encode_status_report.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_hcp_itf.h"
#include "media_sfc_interface.h"

namespace encode
{
    class EncodePreEncPacket : public CmdPacket, public MediaStatusReportObserver, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting
    {
    public:
        enum FlushCmd
        {
            waitHevc = 0,
            waitVdenc,
            waitHevcVdenc
        };
        EncodePreEncPacket(MediaPipeline* pipeline, MediaTask* task, CodechalHwInterfaceNext* hwInterface) : CmdPacket(task),
            m_pipeline(dynamic_cast<EncodePipeline*>(pipeline)),
            m_hwInterface(dynamic_cast<CodechalHwInterfaceNext*>(hwInterface))
        {
            ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_pipeline);
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);

            m_osInterface = hwInterface->GetOsInterface();
            m_statusReport = m_pipeline->GetStatusReportInstance();

            m_featureManager = m_pipeline->GetPacketLevelFeatureManager(EncodePipeline::encodePreEncPacket);
            m_vdencItf      = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(hwInterface->GetVdencInterfaceNext());

            m_hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
            m_miItf  = m_hwInterface->GetMiInterfaceNext();
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_miItf);

            m_sfcItf = m_hwInterface->GetMediaSfcInterface();
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_sfcItf);

            MEDIA_SFC_INTERFACE_MODE sfcMode = {};
            sfcMode.vdboxSfcEnabled          = false;
            sfcMode.veboxSfcEnabled          = true;
            m_sfcItf->Initialize(sfcMode);
        }
        virtual ~EncodePreEncPacket() {}

        virtual MOS_STATUS Init() override;

        virtual MOS_STATUS Prepare() override;

        virtual MOS_STATUS Destroy() override;

        virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override;

        virtual MOS_STATUS Completed(void* mfxStatus, void* rcsStatus, void* statusReport) override;

        virtual MOS_STATUS CalculatePictureStateCommandSize();

        virtual MOS_STATUS GetVdencStateCommandsDataSize(uint32_t& vdencPictureStatesSize, uint32_t& vdencPicturePatchListSize);

        virtual MOS_STATUS PrepareRawSurface();

        MOS_STATUS CalculateCommandSize(
            uint32_t& commandBufferSize,
            uint32_t& requestedPatchListSize) override;

        MOS_STATUS RawSurfaceDownScaling(const PMOS_SURFACE inSurf, PMOS_SURFACE outSurf);

        virtual std::string GetPacketName() override
        {
            return "VDENC_PASS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
        }

    protected:

        virtual MOS_STATUS GetHxxPrimitiveCommandSize();

        virtual uint32_t CalculateCommandBufferSize();

        virtual uint32_t CalculatePatchListSize();

        virtual MOS_STATUS StartStatusReport(
            uint32_t srType,
            MOS_COMMAND_BUFFER* cmdBuffer) override;

        virtual MOS_STATUS EndStatusReport(
            uint32_t            srType,
            MOS_COMMAND_BUFFER* cmdBuffer) override;

        MOS_STATUS ReadHcpStatus(
            MHW_VDBOX_NODE_IND  vdboxIndex,
            MediaStatusReport* statusReport,
            MOS_COMMAND_BUFFER& cmdBuffer);

        virtual MOS_STATUS AllocateResources();

        virtual MOS_STATUS AddPicStateWithNoTile(
            MOS_COMMAND_BUFFER& cmdBuffer);

        virtual MOS_STATUS SendHwSliceEncodeCommand(
            MOS_COMMAND_BUFFER& cmdBuffer);

        virtual MOS_STATUS AddHcpPipeModeSelect(
            MOS_COMMAND_BUFFER& cmdBuffer);

        void SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType);

        MOS_STATUS SendPrologCmds(
            MOS_COMMAND_BUFFER& cmdBuffer);

        MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER& cmdBuffer);

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

        MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

        MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

        MHW_SETPAR_DECL_HDR(HCP_SURFACE_STATE);

        MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(HCP_SLICE_STATE);

        MOS_STATUS AddHcpSurfaceStateCmds(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddHcpFqmStateCmds(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddHcpQMStateCmds(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddHcpPakInsertObjectCmds(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddHcpRefIdxStateCmds(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddHcpBSliceRefIdxStateCmds(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS SubmitPictureLevel(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase);

        MOS_STATUS PatchSliceLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase);

        MOS_STATUS AddPictureHcpCommands(MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer);

#if USE_CODECHAL_DEBUG_TOOL
        MOS_STATUS DumpResources(
            EncodeStatusMfx* encodeStatusMfx,
            EncodeStatusReportData* statusReportData);
#endif
        EncodePipeline *m_pipeline = nullptr;

        // Interfaces
        EncodeAllocator *       m_allocator      = nullptr;
        CodechalHwInterfaceNext *   m_hwInterface    = nullptr;
        PreEncBasicFeature *    m_basicFeature   = nullptr;  //!< Encode parameters used in each frame
        EncodeMemComp *         m_mmcState       = nullptr;

        std::shared_ptr<mhw::vdbox::vdenc::Itf>           m_vdencItf       = nullptr;
        std::shared_ptr<mhw::vdbox::hcp::Itf>             m_hcpItf         = nullptr;
        std::shared_ptr<MediaSfcInterface>                m_sfcItf         = nullptr;
        std::shared_ptr<MediaFeatureManager::ManagerLite> m_featureManager = nullptr;

        mutable uint8_t m_curHcpSurfStateId = 0;

        // Parameters passed from application
        const CODECHAL_HEVC_IQ_MATRIX_PARAMS* m_hevcIqMatrixParams = nullptr;  //!< Pointer to IQ matrix parameter
        const PCODECHAL_NAL_UNIT_PARAMS* m_nalUnitParams = nullptr;  //!< Pointer to NAL unit parameters

        // PAK resources
        PMOS_RESOURCE m_resMetadataLineBuffer = nullptr;  //!< Metadata line data buffer
        PMOS_RESOURCE m_resMetadataTileLineBuffer = nullptr;  //!< Metadata tile line data buffer
        PMOS_RESOURCE m_resMetadataTileColumnBuffer = nullptr;  //!< Metadata tile column data buffer

        PMOS_RESOURCE m_vdencTileRowStoreBuffer = nullptr;  //!< Tile row store buffer

        uint32_t m_sliceStatesSize = 0;  //!< Slice states size

        PMOS_RESOURCE m_vdencIntraRowStoreScratch = nullptr;

        bool     m_usePatchList = 0;  //!< Use Ptach List or not
        uint32_t m_defaultPictureStatesSize = 0;  //!< Picture state command size
        uint32_t m_pictureStatesSize = 0;  //!< Picture states size
        uint32_t m_defaultSliceStatesSize = 0;  //!< Slice state command size
        uint32_t m_defaultPicturePatchListSize = 0;  //!< Picture state patch list size
        uint32_t m_picturePatchListSize = 0;  //!< Picture patch list size
        uint32_t m_defaultSlicePatchListSize = 0;  //!< Slice state patch list size
        uint32_t m_slicePatchListSize = 0;  //!< Slice patch list size

        FlushCmd m_flushCmd = waitHevc;
        uint32_t m_encodeMode = 0;

        bool m_enablePreEncStatusReport = false;

    MEDIA_CLASS_DEFINE_END(encode__EncodePreEncPacket)
    };

}  // namespace encode
#endif
