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
//! \file     encode_preenc_packet.cpp
//! \brief    Defines the interface for preenc packet
//!

#include "encode_preenc_packet.h"
#include "encode_status_report_defs.h"
#include "media_perf_profiler.h"
#include "encode_utils.h"
#include "encode_status_report_defs.h"
#include "mos_solo_generic.h"
#include "encode_status_report_defs.h"
#include "codec_hw_next.h"

using namespace mhw::vdbox;

namespace encode
{
    MOS_STATUS EncodePreEncPacket::Init()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_statusReport);

        ENCODE_CHK_STATUS_RETURN(CmdPacket::Init());
        m_basicFeature = dynamic_cast<PreEncBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::preEncFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->GetEncodeMode(m_encodeMode));

#ifdef _MMC_SUPPORTED
        m_mmcState = m_pipeline->GetMmcState();
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        m_basicFeature->m_mmcState = m_mmcState;
#endif

        m_allocator = m_pipeline->GetEncodeAllocator();
        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            ENCODE_CHK_STATUS_RETURN(m_statusReport->RegistObserver(this));
        }

        CalculatePictureStateCommandSize();

        uint32_t vdencPictureStatesSize = 0, vdencPicturePatchListSize = 0;
        GetVdencStateCommandsDataSize(vdencPictureStatesSize, vdencPicturePatchListSize);
        m_defaultPictureStatesSize += vdencPictureStatesSize;
        m_defaultPicturePatchListSize += vdencPicturePatchListSize;

        GetHxxPrimitiveCommandSize();

        m_usePatchList = m_osInterface->bUsesPatchList;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::Prepare()
    {
        ENCODE_FUNC_CALL();

        m_pictureStatesSize    = m_defaultPictureStatesSize;
        m_picturePatchListSize = m_defaultPicturePatchListSize;
        m_sliceStatesSize      = m_defaultSliceStatesSize;
        m_slicePatchListSize   = m_defaultSlicePatchListSize;

        MediaPipeline *pipeline = dynamic_cast<MediaPipeline *>(m_pipeline);
        ENCODE_CHK_NULL_RETURN(pipeline);

        m_hevcIqMatrixParams = &(m_basicFeature->m_hevcIqMatrixParams);
        m_nalUnitParams      = m_basicFeature->m_nalUnitParams;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::Destroy()
    {
        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            m_statusReport->UnregistObserver(this);
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        ENCODE_CHK_NULL_RETURN(m_allocator);
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;

        allocParamsForBufferLinear.dwBytes  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, m_basicFeature->m_maxLCUSize) * CODECHAL_CACHELINE_SIZE * 2 * 2;
        allocParamsForBufferLinear.pBufName = "vdencIntraRowStoreScratch";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        m_vdencIntraRowStoreScratch         = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;

        // VDENC tile row store buffer
        allocParamsForBufferLinear.dwBytes  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, 32) * CODECHAL_CACHELINE_SIZE * 2;
        allocParamsForBufferLinear.pBufName = "VDENC Tile Row Store Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        m_vdencTileRowStoreBuffer           = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

        hcp::HcpBufferSizePar hcpBufSizePar;
        MOS_ZeroMemory(&hcpBufSizePar, sizeof(hcpBufSizePar));

        hcpBufSizePar.ucMaxBitDepth  = m_basicFeature->m_bitDepth;
        hcpBufSizePar.ucChromaFormat = m_basicFeature->m_chromaFormat;
        // We should move the buffer allocation to picture level if the size is dependent on LCU size
        hcpBufSizePar.dwCtbLog2SizeY = 6;  //assume Max LCU size
        hcpBufSizePar.dwPicWidth     = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, m_basicFeature->m_maxLCUSize);
        hcpBufSizePar.dwPicHeight    = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, m_basicFeature->m_maxLCUSize);

        auto AllocateHcpBuffer = [&](PMOS_RESOURCE &res, const hcp::HCP_INTERNAL_BUFFER_TYPE bufferType, const char *bufferName) {
            uint32_t bufSize         = 0;
            hcpBufSizePar.bufferType = bufferType;
            eStatus                  = m_hcpItf->GetHcpBufSize(hcpBufSizePar, bufSize);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                ENCODE_ASSERTMESSAGE("Failed to get hcp buffer size.");
                return eStatus;
            }
            allocParamsForBufferLinear.dwBytes  = bufSize;
            allocParamsForBufferLinear.pBufName = bufferName;
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
            res                                 = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
            return MOS_STATUS_SUCCESS;
        };

        // Metadata Line buffer
        ENCODE_CHK_STATUS_RETURN(AllocateHcpBuffer(m_resMetadataLineBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::META_LINE, "MetadataLineBuffer"));
        // Metadata Tile Line buffer
        ENCODE_CHK_STATUS_RETURN(AllocateHcpBuffer(m_resMetadataTileLineBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::META_TILE_LINE, "MetadataTileLineBuffer"));
        // Metadata Tile Column buffer
        ENCODE_CHK_STATUS_RETURN(AllocateHcpBuffer(m_resMetadataTileColumnBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::META_TILE_COL, "MetadataTileColumnBuffer"));

        return eStatus;
    }

    void EncodePreEncPacket::SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType)
    {
        ENCODE_FUNC_CALL();

        PerfTagSetting perfTag    = {};
        perfTag.Value             = 0;
        perfTag.Mode              = mode & CODECHAL_ENCODE_MODE_BIT_MASK;
        perfTag.CallType          = type;
        perfTag.PictureCodingType = picCodingType > 3 ? 0 : picCodingType;
        m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
        m_osInterface->pfnIncPerfBufferID(m_osInterface);
    }

    MOS_STATUS EncodePreEncPacket::SendPrologCmds(
            MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

#ifdef _MMC_SUPPORTED
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        ENCODE_CHK_STATUS_RETURN(m_mmcState->SendPrologCmd(&cmdBuffer, false));
#endif

        MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
        MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
        genericPrologParams.pOsInterface  = m_osInterface;
        genericPrologParams.pvMiInterface = nullptr;
        genericPrologParams.bMmcEnabled   = m_mmcState ? m_mmcState->IsMmcEnabled() : false;
        ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

        return eStatus;
    }

    MOS_STATUS EncodePreEncPacket::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        auto &forceWakeupParams                     = m_miItf->MHW_GETPAR_F(MI_FORCE_WAKEUP)();
        forceWakeupParams                           = {};
        forceWakeupParams.bMFXPowerWellControl      = true;
        forceWakeupParams.bMFXPowerWellControlMask  = true;
        forceWakeupParams.bHEVCPowerWellControl     = true;
        forceWakeupParams.bHEVCPowerWellControlMask = true;

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FORCE_WAKEUP)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, EncodePreEncPacket)
    {
        ENCODE_FUNC_CALL();

        params.codecStandardSelect = CODECHAL_HEVC - CODECHAL_HCP_BASE;
        params.bStreamOutEnabled   = false;
        params.bVdencEnabled       = true;
        params.codecSelect         = 1;

        params.multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
        params.pipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;

        params.bTileBasedReplayMode = 0;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AddHcpSurfaceStateCmds(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        m_curHcpSurfStateId = CODECHAL_HCP_SRC_SURFACE_ID;
        SETPAR_AND_ADDCMD(HCP_SURFACE_STATE, m_hcpItf, cmdBuffer);

        m_curHcpSurfStateId = CODECHAL_HCP_DECODED_SURFACE_ID;
        SETPAR_AND_ADDCMD(HCP_SURFACE_STATE, m_hcpItf, cmdBuffer);

        m_curHcpSurfStateId = CODECHAL_HCP_REF_SURFACE_ID;
        SETPAR_AND_ADDCMD(HCP_SURFACE_STATE, m_hcpItf, cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, EncodePreEncPacket)
    {
        ENCODE_FUNC_CALL();

        params.Mode                 = m_basicFeature->m_mode;
        params.psPreDeblockSurface  = &m_basicFeature->m_reconSurface;
        params.psPostDeblockSurface = &m_basicFeature->m_reconSurface;
        params.psRawSurface         = m_basicFeature->m_preEncRawSurface;

        params.presMetadataLineBuffer       = m_resMetadataLineBuffer;
        params.presMetadataTileLineBuffer   = m_resMetadataTileLineBuffer;
        params.presMetadataTileColumnBuffer = m_resMetadataTileColumnBuffer;

        params.bRawIs10Bit = m_basicFeature->m_is10Bit;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, EncodePreEncPacket)
    {
        ENCODE_FUNC_CALL();

        params.presMvObjectBuffer      = m_basicFeature->m_resMbCodeBuffer;
        params.dwMvObjectOffset        = 0;
        params.dwMvObjectSize          = m_basicFeature->m_mbCodeSize;
        params.presPakBaseObjectBuffer = &m_basicFeature->m_resBitstreamBuffer;
        params.dwPakBaseObjectSize     = m_basicFeature->m_bitstreamSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::StartStatusReport(
            uint32_t            srType,
            MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            ENCODE_CHK_STATUS_RETURN(MediaPacket::StartStatusReportNext(srType, cmdBuffer));
        }

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd(
                    (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::EndStatusReport(
            uint32_t            srType,
            MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            ENCODE_CHK_STATUS_RETURN(MediaPacket::EndStatusReportNext(srType, cmdBuffer));
        }

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
                    (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::ReadHcpStatus(
            MHW_VDBOX_NODE_IND  vdboxIndex,
            MediaStatusReport * statusReport,
            MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;

        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        MOS_RESOURCE *osResource = nullptr;
        uint32_t      offset     = 0;

        EncodeStatusReadParams params;
        MOS_ZeroMemory(&params, sizeof(params));

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportMfxBitstreamByteCountPerFrame, osResource, offset));
        params.resBitstreamByteCountPerFrame    = osResource;
        params.bitstreamByteCountPerFrameOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportMfxBitstreamSyntaxElementOnlyBitCount, osResource, offset));
        params.resBitstreamSyntaxElementOnlyBitCount    = osResource;
        params.bitstreamSyntaxElementOnlyBitCountOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportQPStatusCount, osResource, offset));
        params.resQpStatusCount    = osResource;
        params.qpStatusCountOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportImageStatusMask, osResource, offset));
        params.resImageStatusMask    = osResource;
        params.imageStatusMaskOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportImageStatusCtrl, osResource, offset));
        params.resImageStatusCtrl    = osResource;
        params.imageStatusCtrlOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportNumSlices, osResource, offset));
        params.resNumSlices    = osResource;
        params.numSlicesOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->ReadHcpStatus(vdboxIndex, params, &cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->ReadImageStatusForHcp(vdboxIndex, params, &cmdBuffer));
        return eStatus;
    }

    MOS_STATUS EncodePreEncPacket::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;
#if USE_CODECHAL_DEBUG_TOOL
        auto preencFeature = dynamic_cast<PreEncBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::preEncFeature));
        ENCODE_CHK_NULL_RETURN(preencFeature);
        preencFeature->EncodePreencBasicFuntion1();
#endif

        m_basicFeature->Reset((CODEC_REF_LIST *)statusReportData->currRefList);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::GetVdencStateCommandsDataSize(uint32_t &vdencPictureStatesSize, uint32_t &vdencPicturePatchListSize)
    {
        vdencPictureStatesSize =
            m_vdencItf->MHW_GETSIZE_F(VDENC_PIPE_MODE_SELECT)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_SRC_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_REF_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_DS_REF_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_PIPE_BUF_ADDR_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_WEIGHTSOFFSETS_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_WALKER_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)() +
            m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_IMM)()*8 +
            m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() +
            m_hcpItf->MHW_GETSIZE_F(HEVC_VP9_RDOQ_STATE)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

        vdencPicturePatchListSize = PATCH_LIST_COMMAND(mhw::vdbox::vdenc::Itf::VDENC_PIPE_BUF_ADDR_STATE_CMD);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::GetHxxPrimitiveCommandSize()
    {
        uint32_t hcpCommandsSize  = 0;
        uint32_t hcpPatchListSize = 0;
        hcpCommandsSize =
            m_hcpItf->MHW_GETSIZE_F(HCP_REF_IDX_STATE)() * 2 +
            m_hcpItf->MHW_GETSIZE_F(HCP_WEIGHTOFFSET_STATE)() * 2 +
            m_hcpItf->MHW_GETSIZE_F(HCP_SLICE_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() * 2 +
            m_hcpItf->MHW_GETSIZE_F(HCP_TILE_CODING)();  // one slice cannot be with more than one tile

        hcpPatchListSize =
            mhw::vdbox::hcp::Itf::HCP_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES * 2 +
            mhw::vdbox::hcp::Itf::HCP_WEIGHTOFFSET_STATE_CMD_NUMBER_OF_ADDRESSES * 2 +
            mhw::vdbox::hcp::Itf::HCP_SLICE_STATE_CMD_NUMBER_OF_ADDRESSES +
            mhw::vdbox::hcp::Itf::HCP_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES +
            mhw::vdbox::hcp::Itf::MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES * 2 +  // One is for the PAK command and another one is for the BB when BRC and single task mode are on
            mhw::vdbox::hcp::Itf::HCP_TILE_CODING_COMMAND_NUMBER_OF_ADDRESSES;         // HCP_TILE_CODING_STATE command

        uint32_t cpCmdsize = 0;
        uint32_t cpPatchListSize = 0;
        if (m_hwInterface->GetCpInterface())
        {
            m_hwInterface->GetCpInterface()->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
        }

        m_defaultSliceStatesSize = hcpCommandsSize + (uint32_t)cpCmdsize;
        m_defaultSlicePatchListSize = hcpPatchListSize + (uint32_t)cpPatchListSize;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        m_pictureStatesSize    = m_defaultPictureStatesSize;
        m_picturePatchListSize = m_defaultPicturePatchListSize;
        m_sliceStatesSize      = m_defaultSliceStatesSize;
        m_slicePatchListSize   = m_defaultSlicePatchListSize;

        commandBufferSize      = CalculateCommandBufferSize();
        requestedPatchListSize = CalculatePatchListSize();
        return MOS_STATUS_SUCCESS;
    }

    uint32_t EncodePreEncPacket::CalculateCommandBufferSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t commandBufferSize = 0;

        commandBufferSize = m_pictureStatesSize + m_sliceStatesSize;

        if (m_pipeline->IsSingleTaskPhaseSupported())
        {
            commandBufferSize *= m_pipeline->GetPassNum();
        }

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return commandBufferSize;
    }

    uint32_t EncodePreEncPacket::CalculatePatchListSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t requestedPatchListSize = 0;
        if (m_usePatchList)
        {
            requestedPatchListSize = m_picturePatchListSize + m_slicePatchListSize;

            if (m_pipeline->IsSingleTaskPhaseSupported())
            {
                requestedPatchListSize *= m_pipeline->GetPassNum();
            }

            // Multi pipes are sharing one patchlist
            requestedPatchListSize *= m_pipeline->GetPipeNum();
        }
        return requestedPatchListSize;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, EncodePreEncPacket)
    {
        //params.tlbPrefetch = true;

        // can be enabled by reg key (disabled by default)
        params.pakObjCmdStreamOut = false;

        if (!MEDIA_IS_WA(m_osInterface->pfnGetWaTable(m_osInterface), WaEnableOnlyASteppingFeatures))
        {
            params.VdencPipeModeSelectPar0 = 1;
        }

        params.rgbEncodingMode = false;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, EncodePreEncPacket)
    {
        params.intraRowStoreScratchBuffer       = m_vdencIntraRowStoreScratch;
        params.tileRowStoreBuffer               = m_vdencTileRowStoreBuffer;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, EncodePreEncPacket)
    {
        switch (m_flushCmd)
        {
            case waitHevc:
                params.waitDoneHEVC           = true;
                params.flushHEVC              = true;
                params.waitDoneVDCmdMsgParser = true;
                break;
            case waitVdenc:
                params.waitDoneMFX            = true;
                params.waitDoneVDENC          = true;
                params.flushVDENC             = true;
                params.waitDoneVDCmdMsgParser = true;
                break;
            case waitHevcVdenc:
                params.waitDoneMFX            = true;
                params.waitDoneVDENC          = true;
                params.flushVDENC             = true;
                params.flushHEVC              = true;
                params.waitDoneVDCmdMsgParser = true;
                break;
        }

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS EncodePreEncPacket::DumpResources(
            EncodeStatusMfx *       encodeStatusMfx,
            EncodeStatusReportData *statusReportData)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(encodeStatusMfx);
        ENCODE_CHK_NULL_RETURN(statusReportData);
        ENCODE_CHK_NULL_RETURN(m_pipeline);
        ENCODE_CHK_NULL_RETURN(m_statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_trackedBuf);

        CodechalDebugInterface *debugInterface = m_pipeline->GetStatusReportDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        CODEC_REF_LIST currRefList = *((CODEC_REF_LIST *)statusReportData->currRefList);
        currRefList.RefPic         = statusReportData->currOriginalPic;

        debugInterface->m_currPic            = statusReportData->currOriginalPic;
        debugInterface->m_bufferDumpFrameNum = m_statusReport->GetReportedCount();
        debugInterface->m_frameType          = encodeStatusMfx->pictureCodingType;

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                    &currRefList.resBitstreamBuffer,
                    CodechalDbgAttr::attrBitstream,
                    "_PAK",
                    statusReportData->bitstreamSize,
                    0,
                    CODECHAL_NUM_MEDIA_STATES));

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpData(
                    statusReportData,
                    sizeof(EncodeStatusReportData),
                    CodechalDbgAttr::attrStatusReport,
                    "EncodeStatusReport_Buffer"));

        MOS_SURFACE *ds4xSurface = m_basicFeature->m_trackedBuf->GetSurface(
                BufferType::ds4xSurface, currRefList.ucScalingIdx);

        if (ds4xSurface != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                        ds4xSurface,
                        CodechalDbgAttr::attrReconstructedSurface,
                        "4xScaledSurf"))
        }

        MOS_SURFACE *ds8xSurface = m_basicFeature->m_trackedBuf->GetSurface(
                BufferType::ds8xSurface, currRefList.ucScalingIdx);

        if (ds8xSurface != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                        ds8xSurface,
                        CodechalDbgAttr::attrReconstructedSurface,
                        "8xScaledSurf"))
        }

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                    &currRefList.sRefReconBuffer,
                    CodechalDbgAttr::attrReconstructedSurface,
                    "ReconSurf"))

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                    &currRefList.sRefRawBuffer,
                    CodechalDbgAttr::attrEncodeRawInputSurface,
                    "SrcSurf"))

        return MOS_STATUS_SUCCESS;
    }

#endif

    MOS_STATUS EncodePreEncPacket::SubmitPictureLevel(
            MOS_COMMAND_BUFFER *commandBuffer,
            uint8_t             packetPhase)
    {
        ENCODE_FUNC_CALL();

        MOS_COMMAND_BUFFER &cmdBuffer = *commandBuffer;
        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PreProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

        MOS_SYNC_PARAMS syncParams;
        syncParams                  = g_cInitSyncParams;
        syncParams.GpuContext       = m_osInterface->pfnGetGpuContext(m_osInterface);
        syncParams.presSyncResource = &m_basicFeature->m_preEncRawSurface->OsResource;
        syncParams.bReadOnly        = true;
        ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

        ENCODE_CHK_STATUS_RETURN(PatchPictureLevelCommands(packetPhase, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::Submit(
            MOS_COMMAND_BUFFER *commandBuffer,
            uint8_t             packetPhase)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(PrepareRawSurface());

        ENCODE_CHK_STATUS_RETURN(SubmitPictureLevel(commandBuffer, packetPhase));

        MOS_COMMAND_BUFFER &cmdBuffer = *commandBuffer;

        ENCODE_CHK_STATUS_RETURN(PatchSliceLevelCommands(cmdBuffer, packetPhase));

        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

        m_enablePreEncStatusReport = true;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::PrepareRawSurface()
    {
        ENCODE_FUNC_CALL();

        if (m_basicFeature->m_rawDsSurface != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(RawSurfaceDownScaling(&m_basicFeature->m_rawSurface, m_basicFeature->m_rawDsSurface));
            m_basicFeature->m_preEncRawSurface = m_basicFeature->m_rawDsSurface;
        }
        else
        {
            m_basicFeature->m_preEncRawSurface = &m_basicFeature->m_rawSurface;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::RawSurfaceDownScaling(const PMOS_SURFACE inSurf, PMOS_SURFACE outSurf)
    {
        ENCODE_FUNC_CALL();

        if (outSurf == nullptr)
        {
            return MOS_STATUS_SUCCESS;
        }

        VEBOX_SFC_PARAMS params = {};

        params.input.surface      = inSurf;
        params.input.colorSpace   = CSpace_Any;
        params.input.chromaSiting = 0;
        params.input.rcSrc        = {0, 0, (long)inSurf->dwWidth, (long)inSurf->dwHeight};
        params.input.rotation     = ROTATION_IDENTITY;

        params.output.surface      = outSurf;
        params.output.colorSpace   = CSpace_Any;
        params.output.chromaSiting = 0;
        params.output.rcDst        = {0, 0, (long)outSurf->dwWidth, (long)outSurf->dwHeight};

        ENCODE_CHK_STATUS_RETURN(m_sfcItf->Render(params));

        m_pipeline->ContextSwitchBack();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

        SetPerfTag(CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE, (uint16_t)m_basicFeature->m_mode, m_basicFeature->GetPictureCodingType());

        bool firstTaskInPhase = packetPhase & firstPacket;
        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            ENCODE_CHK_STATUS_RETURN(AddForceWakeup(cmdBuffer));

            // Send command buffer header at the beginning (OS dependent)
            ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddPictureHcpCommands(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddPictureVdencCommands(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddPicStateWithNoTile(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::PatchSliceLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(SendHwSliceEncodeCommand(cmdBuffer));

        m_flushCmd = waitVdenc;
        SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

        m_flushCmd = waitHevc;
        SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(MHW_VDBOX_NODE_1, m_statusReport, cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, &cmdBuffer));

        if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC || m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC)
        {
            ENCODE_CHK_STATUS_RETURN(MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AddPicStateWithNoTile(
            MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, &cmdBuffer);

        SETPAR_AND_ADDCMD(HCP_PIC_STATE, m_hcpItf, &cmdBuffer);

        SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, &cmdBuffer);

        // Send HEVC_VP9_RDOQ_STATE command
        SETPAR_AND_ADDCMD(HEVC_VP9_RDOQ_STATE, m_hcpItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        // Send MI_FLUSH command
        auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                               = {};
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        SETPAR_AND_ADDCMD(VDENC_PIPE_MODE_SELECT, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_SRC_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_REF_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_DS_REF_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_PIPE_BUF_ADDR_STATE, m_vdencItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AddPictureHcpCommands(
            MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(AddHcpPipeModeSelect(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddHcpSurfaceStateCmds(&cmdBuffer));

        SETPAR_AND_ADDCMD(HCP_PIPE_BUF_ADDR_STATE, m_hcpItf, &cmdBuffer);

        SETPAR_AND_ADDCMD(HCP_IND_OBJ_BASE_ADDR_STATE, m_hcpItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(AddHcpFqmStateCmds(&cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(AddHcpQMStateCmds(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AddHcpPipeModeSelect(
            MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        SETPAR_AND_ADDCMD(VDENC_CONTROL_STATE, m_vdencItf, &cmdBuffer);

        auto &vdControlStateParams          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
        vdControlStateParams                = {};
        vdControlStateParams.initialization = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
        auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer));

        SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpItf, &cmdBuffer);

        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::CalculatePictureStateCommandSize()
    {
        ENCODE_FUNC_CALL();

        uint32_t hcpCommandsSize  = 0;
        uint32_t hcpPatchListSize = 0;
        uint32_t cpCmdsize        = 0;
        uint32_t cpPatchListSize  = 0;
        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;

        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

        hcpCommandsSize =
            m_vdencItf->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)() +
            m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_PIPE_MODE_SELECT)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_SURFACE_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_PIPE_BUF_ADDR_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_IND_OBJ_BASE_ADDR_STATE)() +
            m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_REG)() * 8;

        hcpPatchListSize =
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_SURFACE_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_IND_OBJ_BASE_ADDR_STATE_CMD);

        // HCP_QM_STATE_CMD may be issued up to 20 times: 3x Colour Component plus 2x intra/inter plus 4x SizeID minus 4 for the 32x32 chroma components.
        // HCP_FQP_STATE_CMD may be issued up to 8 times: 4 scaling list per intra and inter. 
        hcpCommandsSize +=
            2 * m_miItf->MHW_GETSIZE_F(VD_CONTROL_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_SURFACE_STATE)() +  // encoder needs two surface state commands. One is for raw and another one is for recon surfaces.
            20 * m_hcpItf->MHW_GETSIZE_F(HCP_QM_STATE)() +
            8 * m_hcpItf->MHW_GETSIZE_F(HCP_FQM_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_PIC_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HEVC_VP9_RDOQ_STATE)() +        // RDOQ
            2 * m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() +       // Slice level commands
            2 * m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +             // need for Status report, Mfc Status and
            10 * m_miItf->MHW_GETSIZE_F(MI_STORE_REGISTER_MEM)() +  // 8 for BRCStatistics and 2 for RC6 WAs
            m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_MEM)() +        // 1 for RC6 WA
            2 * m_hcpItf->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)() +  // Two PAK insert object commands are for headers before the slice header and the header for the end of stream
            4 * m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() +       // two (BRC+reference frame) for clean-up HW semaphore memory and another two for signal it
            17 * m_miItf->MHW_GETSIZE_F(MI_SEMAPHORE_WAIT)() +      // Use HW wait command for each reference and one wait for current semaphore object
            m_miItf->MHW_GETSIZE_F(MI_SEMAPHORE_WAIT)() +           // Use HW wait command for each BRC pass
            +m_miItf->MHW_GETSIZE_F(MI_SEMAPHORE_WAIT)()            // Use HW wait command for each VDBOX
            + 2 * m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)()       // One is for reset and another one for set per VDBOX
            + 8 * m_miItf->MHW_GETSIZE_F(MI_COPY_MEM_MEM)()         // Need to copy SSE statistics/ Slice Size overflow into memory
            ;

        hcpPatchListSize +=
            20 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_QM_STATE_CMD) +
            8 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_FQM_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIC_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_BATCH_BUFFER_START_CMD) +       // When BRC is on, HCP_PIC_STATE_CMD command is in the BB
            2 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_STORE_DATA_IMM_CMD) +       // Slice level commands
            2 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_FLUSH_DW_CMD) +             // need for Status report, Mfc Status and
            11 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_STORE_REGISTER_MEM_CMD) +  // 8 for BRCStatistics and 3 for RC6 WAs
            22 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_STORE_DATA_IMM_CMD)        // Use HW wait commands plus its memory clean-up and signal (4+ 16 + 1 + 1)
            + 8 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_BATCH_BUFFER_START_CMD)   // At maximal, there are 8 batch buffers for 8 VDBOXes for VE. Each box has one BB.
            + PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_FLUSH_DW_CMD)                 // Need one flush before copy command
            + PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MFX_WAIT_CMD)                    // Need one wait after copy command
            + 3 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_STORE_DATA_IMM_CMD)       // one wait commands and two for reset and set semaphore memory
            + 8 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_COPY_MEM_MEM_CMD)         // Need to copy SSE statistics/ Slice Size overflow into memory
            ;

        auto cpInterface = m_hwInterface->GetCpInterface();
        cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
                m_basicFeature->m_mode, (uint32_t *)&hucCommandsSize, (uint32_t *)&hucPatchListSize, &stateCmdSizeParams));

        m_defaultPictureStatesSize    = hcpCommandsSize + hucCommandsSize + (uint32_t)cpCmdsize;
        m_defaultPicturePatchListSize = hcpPatchListSize + hucPatchListSize + (uint32_t)cpPatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::SendHwSliceEncodeCommand(
            MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        // VDENC does not use batch buffer for slice state
        // add HCP_REF_IDX command
        ENCODE_CHK_STATUS_RETURN(AddHcpRefIdxStateCmds(&cmdBuffer));

        // add HEVC Slice state commands
        SETPAR_AND_ADDCMD(HCP_SLICE_STATE, m_hcpItf, &cmdBuffer);

        // add HCP_PAK_INSERT_OBJECTS command
        ENCODE_CHK_STATUS_RETURN(AddHcpPakInsertObjectCmds(&cmdBuffer));

        // Send VDENC_WEIGHT_OFFSETS_STATE command
        SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, &cmdBuffer);

        SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, &cmdBuffer);

        // Send VDENC_WALKER_STATE command
        SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencItf, &cmdBuffer);

        return eStatus;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, EncodePreEncPacket)
    {

        params.numPipe = VDENC_PIPE_SINGLE_PIPE;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_SURFACE_STATE, EncodePreEncPacket)
    {
        params.surfaceStateId = m_curHcpSurfStateId;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_SLICE_STATE, EncodePreEncPacket)
    {
        ENCODE_FUNC_CALL();

        params.intrareffetchdisable = false;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AddHcpFqmStateCmds(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        MHW_MI_CHK_NULL(m_hevcIqMatrixParams);

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_FQM_STATE)();
        params       = {};

        auto      iqMatrix = (PMHW_VDBOX_HEVC_QM_PARAMS)m_hevcIqMatrixParams;
        uint16_t *fqMatrix = (uint16_t *)params.quantizermatrix;

        /* 4x4 */
        for (uint8_t i = 0; i < 32; i++)
        {
            params.quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            params.intraInter     = intraInter;
            params.sizeid         = 0;
            params.colorComponent = 0;

            for (uint8_t i = 0; i < 16; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List4x4[3 * intraInter][i]);
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_FQM_STATE)(cmdBuffer);
        }

        /* 8x8, 16x16 and 32x32 */
        for (uint8_t i = 0; i < 32; i++)
        {
            params.quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            params.intraInter     = intraInter;
            params.sizeid         = 1;
            params.colorComponent = 0;

            for (uint8_t i = 0; i < 64; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List8x8[3 * intraInter][i]);
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_FQM_STATE)(cmdBuffer);
        }

        /* 16x16 DC */
        for (uint8_t i = 0; i < 32; i++)
        {
            params.quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            params.intraInter     = intraInter;
            params.sizeid         = 2;
            params.colorComponent = 0;
            params.fqmDcValue1Dc  = GetReciprocalScalingValue(iqMatrix->ListDC16x16[3 * intraInter]);

            for (uint8_t i = 0; i < 64; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List16x16[3 * intraInter][i]);
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_FQM_STATE)(cmdBuffer);
        }

        /* 32x32 DC */
        for (uint8_t i = 0; i < 32; i++)
        {
            params.quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            params.intraInter     = intraInter;
            params.sizeid         = 3;
            params.colorComponent = 0;
            params.fqmDcValue1Dc  = GetReciprocalScalingValue(iqMatrix->ListDC32x32[intraInter]);

            for (uint8_t i = 0; i < 64; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List32x32[intraInter][i]);
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_FQM_STATE)(cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AddHcpQMStateCmds(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        MHW_MI_CHK_NULL(m_hevcIqMatrixParams);

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_QM_STATE)();
        params       = {};

        auto     iqMatrix = (PMHW_VDBOX_HEVC_QM_PARAMS)m_hevcIqMatrixParams;
        uint8_t *qMatrix  = (uint8_t *)params.quantizermatrix;

        for (uint8_t sizeId = 0; sizeId < 4; sizeId++)  // 4x4, 8x8, 16x16, 32x32
        {
            for (uint8_t predType = 0; predType < 2; predType++)  // Intra, Inter
            {
                for (uint8_t color = 0; color < 3; color++)  // Y, Cb, Cr
                {
                    if ((sizeId == 3) && (color != 0))
                        break;

                    params.sizeid         = sizeId;
                    params.predictionType = predType;
                    params.colorComponent = color;
                    switch (sizeId)
                    {
                        case 0:
                        case 1:
                        default:
                            params.dcCoefficient = 0;
                            break;
                        case 2:
                            params.dcCoefficient = iqMatrix->ListDC16x16[3 * predType + color];
                            break;
                        case 3:
                            params.dcCoefficient = iqMatrix->ListDC32x32[predType];
                            break;
                    }

                    if (sizeId == 0)
                    {
                        for (uint8_t i = 0; i < 4; i++)
                        {
                            for (uint8_t ii = 0; ii < 4; ii++)
                            {
                                qMatrix[4 * i + ii] = iqMatrix->List4x4[3 * predType + color][4 * i + ii];
                            }
                        }
                    }
                    else if (sizeId == 1)
                    {
                        for (uint8_t i = 0; i < 8; i++)
                        {
                            for (uint8_t ii = 0; ii < 8; ii++)
                            {
                                qMatrix[8 * i + ii] = iqMatrix->List8x8[3 * predType + color][8 * i + ii];
                            }
                        }
                    }
                    else if (sizeId == 2)
                    {
                        for (uint8_t i = 0; i < 8; i++)
                        {
                            for (uint8_t ii = 0; ii < 8; ii++)
                            {
                                qMatrix[8 * i + ii] = iqMatrix->List16x16[3 * predType + color][8 * i + ii];
                            }
                        }
                    }
                    else  // 32x32
                    {
                        for (uint8_t i = 0; i < 8; i++)
                        {
                            for (uint8_t ii = 0; ii < 8; ii++)
                            {
                                qMatrix[8 * i + ii] = iqMatrix->List32x32[predType][8 * i + ii];
                            }
                        }
                    }

                    m_hcpItf->MHW_ADDCMD_F(HCP_QM_STATE)(cmdBuffer);
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AddHcpPakInsertObjectCmds(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_osInterface);
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PAK_INSERT_OBJECT)();
        params       = {};

        PCODECHAL_NAL_UNIT_PARAMS *ppNalUnitParams = (CODECHAL_NAL_UNIT_PARAMS **)m_nalUnitParams;

        PBSBuffer pBsBuffer = &(m_basicFeature->m_bsBuffer);
        uint32_t  bitSize   = 0;
        uint32_t  offSet    = 0;

        //insert AU, SPS, PSP headers before first slice header
        uint32_t maxBytesInPakInsertObjCmd = ((2 << 11) - 1) * 4;  // 12 bits for Length field in PAK_INSERT_OBJ cmd

        for (uint32_t i = 0; i < m_basicFeature->m_NumNalUnits; i++)
        {
            uint32_t nalunitPosiSize   = ppNalUnitParams[i]->uiSize;
            uint32_t nalunitPosiOffset = ppNalUnitParams[i]->uiOffset;

            while (nalunitPosiSize > 0)
            {
                bitSize = MOS_MIN(maxBytesInPakInsertObjCmd * 8, nalunitPosiSize * 8);
                offSet  = nalunitPosiOffset;

                params = {};

                params.dwPadding                 = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                params.bEmulationByteBitsInsert  = ppNalUnitParams[i]->bInsertEmulationBytes;
                params.uiSkipEmulationCheckCount = ppNalUnitParams[i]->uiSkipEmulationCheckCount;
                params.dataBitsInLastDw          = bitSize % 32;
                if (params.dataBitsInLastDw == 0)
                {
                    params.dataBitsInLastDw = 32;
                }

                if (nalunitPosiSize > maxBytesInPakInsertObjCmd)
                {
                    nalunitPosiSize -= maxBytesInPakInsertObjCmd;
                    nalunitPosiOffset += maxBytesInPakInsertObjCmd;
                }
                else
                {
                    nalunitPosiSize = 0;
                }
                m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
                uint32_t byteSize = (bitSize + 7) >> 3;
                if (byteSize)
                {
                    MHW_MI_CHK_NULL(pBsBuffer);
                    MHW_MI_CHK_NULL(pBsBuffer->pBase);
                    uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, data, byteSize));
                }
            }
        }

        params = {};
        // Insert slice header
        params.bLastHeader              = true;
        params.bEmulationByteBitsInsert = true;

        // App does the slice header packing, set the skip count passed by the app
        PCODEC_ENCODER_SLCDATA slcData = m_basicFeature->m_slcData;

        params.uiSkipEmulationCheckCount = slcData->SkipEmulationByteCount;
        bitSize                          = slcData->BitSize;
        offSet                           = slcData->SliceOffset;

        // Hevc header needs slcData->BitSize > 0 to caculate params.dwPadding and params.dataBitsInLastDw for cmd  HCP_PAK_INSERT_OBJECT.
        // For Av1, slcData params are zeros. Av1 use Hevc preEnc packet, need to set bitSize>0. 
        // The bitstream header size will have no effect on the mv.
        if (bitSize == 0)
        {
            bitSize = sizeof(uint8_t) * 8;
        }

        params.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
        params.dataBitsInLastDw = bitSize % 32;
        if (params.dataBitsInLastDw == 0)
        {
            params.dataBitsInLastDw = 32;
        }
        m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
        uint32_t byteSize = (bitSize + 7) >> 3;
        if (byteSize)
        {
            MHW_MI_CHK_NULL(pBsBuffer);
            MHW_MI_CHK_NULL(pBsBuffer->pBase);
            uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
            MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, data, byteSize));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AddHcpRefIdxStateCmds(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_REF_IDX_STATE)();
        params       = {};

        CODEC_PICTURE currPic                                     = {};
        CODEC_PICTURE refPicList[2][CODEC_MAX_NUM_REF_FRAME_HEVC] = {};
        void **       hevcRefList                                 = nullptr;
        int32_t       pocCurrPic                                  = 0;
        int8_t *      pRefIdxMapping                              = nullptr;
        int32_t       pocList[CODEC_MAX_NUM_REF_FRAME_HEVC]       = {};

        if (m_basicFeature->m_preEncConfig.SliceType != encodeHevcISlice)
        {
            currPic                                    = m_basicFeature->m_preEncConfig.CurrReconstructedPic;
            params.ucList                              = LIST_0;
            params.numRefIdxLRefpiclistnumActiveMinus1 = 0;
            eStatus                                    = MOS_SecureMemcpy(&refPicList, sizeof(refPicList), &m_basicFeature->m_preEncConfig.RefPicList, sizeof(m_basicFeature->m_preEncConfig.RefPicList));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                ENCODE_ASSERTMESSAGE("Failed to copy memory.");
                return eStatus;
            }

            hevcRefList = (void **)m_basicFeature->GetRefList();
            pocCurrPic  = m_basicFeature->m_preEncConfig.CurrPicOrderCnt;
            for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                pocList[i] = m_basicFeature->m_preEncConfig.RefFramePOCList[i];
            }

            pRefIdxMapping = m_basicFeature->GetRefIdxMapping();

            MHW_ASSERT(currPic.FrameIdx != 0x7F);

            for (uint8_t i = 0; i <= params.numRefIdxLRefpiclistnumActiveMinus1; i++)
            {
                uint8_t refFrameIDx = refPicList[params.ucList][i].FrameIdx;
                if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
                {
                    MHW_ASSERT(*(pRefIdxMapping + refFrameIDx) >= 0);

                    params.listEntryLxReferencePictureFrameIdRefaddr07[i] = *(pRefIdxMapping + refFrameIDx);
                    int32_t pocDiff                                       = pocCurrPic - pocList[refFrameIDx];
                    params.referencePictureTbValue[i]                     = (uint8_t)CodecHal_Clip3(-128, 127, pocDiff);
                    CODEC_REF_LIST **refList                              = (CODEC_REF_LIST **)hevcRefList;
                    params.longtermreference[i]                           = CodecHal_PictureIsLongTermRef(refList[currPic.FrameIdx]->RefList[refFrameIDx]);
                    params.bottomFieldFlag[i]                             = 1;
                }
                else
                {
                    params.listEntryLxReferencePictureFrameIdRefaddr07[i] = 0;
                    params.referencePictureTbValue[i]                     = 0;
                    params.longtermreference[i]                           = false;
                    params.bottomFieldFlag[i]                             = 0;
                }
            }

            for (uint8_t i = (uint8_t)(params.numRefIdxLRefpiclistnumActiveMinus1 + 1); i < 16; i++)
            {
                params.listEntryLxReferencePictureFrameIdRefaddr07[i] = 0;
                params.referencePictureTbValue[i]                     = 0;
                params.longtermreference[i]                           = false;
                params.bottomFieldFlag[i]                             = 0;
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_REF_IDX_STATE)(cmdBuffer);

            params = {};

            if (m_basicFeature->m_preEncConfig.SliceType == encodeHevcBSlice)
            {
                AddHcpBSliceRefIdxStateCmds(cmdBuffer);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodePreEncPacket::AddHcpBSliceRefIdxStateCmds(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_REF_IDX_STATE)();
        params       = {};

        CODEC_PICTURE currPic                                     = {};
        CODEC_PICTURE refPicList[2][CODEC_MAX_NUM_REF_FRAME_HEVC] = {};
        void **       hevcRefList                                 = nullptr;
        int32_t       pocCurrPic                                  = 0;
        int8_t *      pRefIdxMapping                              = nullptr;
        int32_t       pocList[CODEC_MAX_NUM_REF_FRAME_HEVC]       = {};

        currPic = m_basicFeature->m_preEncConfig.CurrReconstructedPic;
        eStatus = MOS_SecureMemcpy(&refPicList, sizeof(refPicList), &m_basicFeature->m_preEncConfig.RefPicList, sizeof(m_basicFeature->m_preEncConfig.RefPicList));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        hevcRefList = (void **)m_basicFeature->GetRefList();
        pocCurrPic  = m_basicFeature->m_preEncConfig.CurrPicOrderCnt;
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            pocList[i] = m_basicFeature->m_preEncConfig.RefFramePOCList[i];
        }

        pRefIdxMapping = m_basicFeature->GetRefIdxMapping();

        MHW_ASSERT(currPic.FrameIdx != 0x7F);

        params.ucList                              = LIST_1;
        params.numRefIdxLRefpiclistnumActiveMinus1 = 0;
        for (uint8_t i = 0; i <= params.numRefIdxLRefpiclistnumActiveMinus1; i++)
        {
            uint8_t refFrameIDx = m_basicFeature->m_preEncConfig.isPToB ? refPicList[0][i].FrameIdx : refPicList[1][i].FrameIdx;
            if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                MHW_ASSERT(*(pRefIdxMapping + refFrameIDx) >= 0);

                params.listEntryLxReferencePictureFrameIdRefaddr07[i] = *(pRefIdxMapping + refFrameIDx);
                int32_t pocDiff                                       = pocCurrPic - pocList[refFrameIDx];
                params.referencePictureTbValue[i]                     = (uint8_t)CodecHal_Clip3(-128, 127, pocDiff);
                CODEC_REF_LIST **refList                              = (CODEC_REF_LIST **)hevcRefList;
                params.longtermreference[i]                           = CodecHal_PictureIsLongTermRef(refList[currPic.FrameIdx]->RefList[refFrameIDx]);
                params.bottomFieldFlag[i]                             = 1;
            }
            else
            {
                params.listEntryLxReferencePictureFrameIdRefaddr07[i] = 0;
                params.referencePictureTbValue[i]                     = 0;
                params.longtermreference[i]                           = false;
                params.bottomFieldFlag[i]                             = 0;
            }
        }

        for (uint8_t i = (uint8_t)(params.numRefIdxLRefpiclistnumActiveMinus1 + 1); i < 16; i++)
        {
            params.listEntryLxReferencePictureFrameIdRefaddr07[i] = 0;
            params.referencePictureTbValue[i]                     = 0;
            params.longtermreference[i]                           = false;
            params.bottomFieldFlag[i]                             = 0;
        }
        m_hcpItf->MHW_ADDCMD_F(HCP_REF_IDX_STATE)(cmdBuffer);
        return eStatus;
    }
}  // namespace encode
