/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_pak_integrate_packet.cpp
//! \brief    Defines the interface for pak integrate packet
//!
#include "mos_defs.h"
#include "encode_pak_integrate_packet.h"
#include "mhw_vdbox.h"
#include "encode_hevc_brc.h"
#include "encode_status_report_defs.h"
#include "mos_os_cp_interface_specific.h"

namespace encode {
    MOS_STATUS HevcPakIntegratePkt::Init()
    {
        ENCODE_FUNC_CALL();

        m_basicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(HevcFeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Init());

        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        m_osInterface  = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(m_osInterface);

        m_miItf = m_hwInterface->GetMiInterfaceNext();
        ENCODE_CHK_NULL_RETURN(m_miItf);

        ENCODE_CHK_NULL_RETURN(m_pipeline);
#ifdef _MMC_SUPPORTED
        m_mmcState = m_pipeline->GetMmcState();
        ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::AllocateResources()
    {
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

        // Only needed when tile & BRC is enabled, but the size is not changing at frame level
        if (m_resHucPakStitchDmemBuffer[0][0] == nullptr)
        {
            uint8_t *data;
            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;

            // Pak stitch DMEM
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucPakIntegrateDmem), CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.pBufName = "PAK Stitch Dmem Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
            auto numOfPasses                    = CODECHAL_VDENC_BRC_NUM_OF_PASSES;

            for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
            {
                for (auto i = 0; i < numOfPasses; i++)
                {
                    m_resHucPakStitchDmemBuffer[k][i] = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
                }
            }

            if (m_basicFeature->m_enableTileStitchByHW)
            {
                // HuC stitching data buffer
                allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucCommandData), CODECHAL_PAGE_SIZE);
                allocParamsForBufferLinear.pBufName = "HEVC HuC Stitch Data Buffer";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
                MOS_RESOURCE *allocatedBuffer       = nullptr;
                for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; ++i)
                {
                    for (auto j = 0; j < CODECHAL_VDENC_BRC_NUM_OF_PASSES; ++j)
                    {
                        allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
                        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
                        m_resHucStitchDataBuffer[i][j] = *allocatedBuffer;
                    }
                }

                // Second level batch buffer for HuC stitching CMD
                MOS_ZeroMemory(&m_HucStitchCmdBatchBuffer, sizeof(m_HucStitchCmdBatchBuffer));
                m_HucStitchCmdBatchBuffer.bSecondLevel = true;
                ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                    m_osInterface,
                    &m_HucStitchCmdBatchBuffer,
                    nullptr,
                    m_hwInterface->m_HucStitchCmdBatchBufferSize));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::FreeResources()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        eStatus = Mhw_FreeBb(m_osInterface, &m_HucStitchCmdBatchBuffer, nullptr);
        ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

        return eStatus;
    }

    void HevcPakIntegratePkt::UpdateParameters()
    {
        ENCODE_FUNC_CALL();

        if (!m_pipeline->IsSingleTaskPhaseSupported())
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_basicFeature->m_currPakSliceIdx = (m_basicFeature->m_currPakSliceIdx + 1) % m_basicFeature->m_codecHalHevcNumPakSliceBatchBuffers;
    }

    MOS_STATUS HevcPakIntegratePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog = !m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase;

        uint16_t perfTag = CODECHAL_ENCODE_PERFTAG_CALL_PAK_KERNEL;
        SetPerfTag(perfTag, (uint16_t)m_basicFeature->m_mode, m_basicFeature->m_pictureCodingType);

        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        ENCODE_CHK_STATUS_RETURN(AddCondBBEndForLastPass(*commandBuffer));

        m_vdencHucUsed = brcFeature->IsVdencHucUsed();

        bool isTileReplayEnabled = false;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, IsTileReplayEnabled, isTileReplayEnabled);

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, commandBuffer));

        if (m_vdencHucUsed || (m_basicFeature->m_enableTileStitchByHW && (isTileReplayEnabled || m_pipeline->GetPipeNum() > 1)))
        {
            // Huc basic
            ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog));

            // Add huc status update to status buffer
            PMOS_RESOURCE osResource = nullptr;
            uint32_t offset = 0;
            ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportHucStatusRegMask, osResource, offset));
            ENCODE_CHK_NULL_RETURN(osResource);

            // Write HUC_STATUS mask
            auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            storeDataParams                  = {};
            storeDataParams.pOsResource      = osResource;
            storeDataParams.dwResourceOffset = offset;
            storeDataParams.dwValue          = m_hwInterface->GetHucInterfaceNext()->GetHucStatusReEncodeMask();
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(commandBuffer));

            // store HUC_STATUS register
            osResource = nullptr;
            offset     = 0;
            ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportHucStatusReg, osResource, offset));
            ENCODE_CHK_NULL_RETURN(osResource);
            auto mmioRegisters             = m_hucItf->GetMmioRegisters(m_vdboxIndex);
            auto &storeRegParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
            storeDataParams                = {};
            storeRegParams.presStoreBuffer = osResource;
            storeRegParams.dwOffset        = offset;
            storeRegParams.dwRegister      = mmioRegisters->hucStatusRegOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(commandBuffer));
        }
        
        // Use HW stitch commands only in the scalable mode
        // For single pipe with tile replay, stitch also needed
        if (m_basicFeature->m_enableTileStitchByHW && (isTileReplayEnabled || m_pipeline->GetPipeNum() > 1))
        {
            ENCODE_CHK_STATUS_RETURN(PerformHwStitch(commandBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(ReadSseStatistics(*commandBuffer));

        ENCODE_CHK_STATUS_RETURN(ReadSliceSize(*commandBuffer));

        ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, commandBuffer));
        if (false == m_pipeline->IsFrameTrackingEnabled())
        {
            ENCODE_CHK_STATUS_RETURN(UpdateStatusReportNext(statusReportGlobalCount, commandBuffer));
        }
        CODECHAL_DEBUG_TOOL(
            if (m_mmcState) {
                m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_reconSurface));
            })
        // Reset parameters for next PAK execution
        if (false == m_pipeline->IsFrameTrackingEnabled())
        {
            UpdateParameters();
        }

        CODECHAL_DEBUG_TOOL
        (
            ENCODE_CHK_STATUS_RETURN(DumpInput());
        )

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::EndStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);
        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);
        if (m_pipeline->GetPipeNum() <= 1 && m_pipeline->IsSingleTaskPhaseSupported())
        {
            // single pipe mode can read the info from MMIO register. Otherwise,
            // we have to use the tile size statistic buffer
            ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(m_vdboxIndex, m_statusReport, *cmdBuffer));
            // BRC PAK statistics different for each pass
            if (brcFeature->IsBRCEnabled())
            {
                uint8_t ucPass = (uint8_t)m_pipeline->GetCurrentPass();
                EncodeReadBrcPakStatsParams readBrcPakStatsParams;
                MOS_RESOURCE *osResource = nullptr;
                uint32_t      offset = 0;
                ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportNumberPasses, osResource, offset));
                RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature, SetReadBrcPakStatsParams, ucPass, offset, osResource, readBrcPakStatsParams);
                ReadBrcPakStatistics(cmdBuffer, &readBrcPakStatsParams);
            }
        }
        ENCODE_CHK_STATUS_RETURN(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::ReadHcpStatus(
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
        uint32_t      offset = 0;

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

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->ReadHcpStatus(vdboxIndex, params, &cmdBuffer));

        // Slice Size Conformance
        if (m_basicFeature->m_hevcSeqParams->SliceSizeControl)
        {
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeDss, HevcFeatureIDs::hevcVdencDssFeature, ReadHcpStatus, vdboxIndex, cmdBuffer);
        }
        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);
        bool vdencHucUsed  = brcFeature->IsVdencHucUsed();
        if (vdencHucUsed)
        {
            // Store PAK frameSize MMIO to PakInfo buffer
            auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
            miStoreRegMemParams                 = {};
            miStoreRegMemParams.presStoreBuffer = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
            miStoreRegMemParams.dwOffset        = 0;
            auto mmioRegisters                  = m_hcpItf->GetMmioRegisters(m_vdboxIndex);
            ENCODE_CHK_NULL_RETURN(mmioRegisters);
            miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;

            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));
        }
        ENCODE_CHK_STATUS_RETURN(m_hwInterface->ReadImageStatusForHcp(vdboxIndex, params, &cmdBuffer));
        return eStatus;
    }

    MOS_STATUS HevcPakIntegratePkt::ReadBrcPakStatistics(
        PMOS_COMMAND_BUFFER          cmdBuffer,
        EncodeReadBrcPakStatsParams *params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);
        ENCODE_CHK_NULL_RETURN(params);
        ENCODE_CHK_NULL_RETURN(params->presBrcPakStatisticBuffer);
        ENCODE_CHK_NULL_RETURN(params->presStatusBuffer);

        ENCODE_CHK_STATUS_RETURN(ValidateVdboxIdx(m_vdboxIndex));

        auto mmioRegisters = m_hcpItf->GetMmioRegisters(m_vdboxIndex);
        ENCODE_CHK_NULL_RETURN(mmioRegisters);

        auto AddMiStoreRegisterMemCmd = [&](uint32_t offset, uint32_t hcpMmioRegister) {
            auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
            miStoreRegMemParams                 = {};
            miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
            miStoreRegMemParams.dwOffset        = offset;
            miStoreRegMemParams.dwRegister      = hcpMmioRegister;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));
            return MOS_STATUS_SUCCESS;
        };

        ENCODE_CHK_STATUS_RETURN(AddMiStoreRegisterMemCmd(CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_BITSTREAM_BYTECOUNT_FRAME), mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset));
        ENCODE_CHK_STATUS_RETURN(AddMiStoreRegisterMemCmd(CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_BITSTREAM_BYTECOUNT_FRAME_NOHEADER), mmioRegisters->hcpEncBitstreamBytecountFrameNoHeaderRegOffset));
        ENCODE_CHK_STATUS_RETURN(AddMiStoreRegisterMemCmd(CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL), mmioRegisters->hcpEncImageStatusCtrlRegOffset));

        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = params->presStatusBuffer;
        storeDataParams.dwResourceOffset = params->dwStatusBufNumPassesOffset;
        storeDataParams.dwValue          = params->ucPass;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        return eStatus;
    }

        // Inline functions
    MOS_STATUS HevcPakIntegratePkt::ValidateVdboxIdx(const MHW_VDBOX_NODE_IND &vdboxIndex)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        if (vdboxIndex > m_hwInterface->GetMaxVdboxIndex())
        {
            //ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }

        return eStatus;
    }

    MOS_STATUS HevcPakIntegratePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();

        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

        stateCmdSizeParams.uNumStoreDataImm = 2;
        stateCmdSizeParams.uNumStoreReg     = 4;
        stateCmdSizeParams.uNumMfxWait      = 11;
        stateCmdSizeParams.uNumMiCopy       = 5;
        stateCmdSizeParams.uNumMiFlush      = 2;
        stateCmdSizeParams.uNumVdPipelineFlush  = 1;
        stateCmdSizeParams.bPerformHucStreamOut = true;
        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, &stateCmdSizeParams));

        bool isTileReplayEnabled = false;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, IsTileReplayEnabled, isTileReplayEnabled);
        if (m_basicFeature->m_enableTileStitchByHW && (isTileReplayEnabled || m_pipeline->GetPipeNum() > 1))
        {
            uint32_t maxSize = 0;
            uint32_t patchListMaxSize = 0;
            ENCODE_CHK_NULL_RETURN(m_hwInterface);
            ENCODE_CHK_NULL_RETURN(m_hwInterface->GetCpInterface());
            MhwCpInterface *cpInterface = m_hwInterface->GetCpInterface();
            cpInterface->GetCpStateLevelCmdSize(maxSize, patchListMaxSize);
            hucCommandsSize     += maxSize;
            hucPatchListSize    += patchListMaxSize;
        }

        commandBufferSize = hucCommandsSize;
        requestedPatchListSize = m_osInterface->bUsesPatchList ? hucPatchListSize : 0;

        // reserve cmd size for hw stitch
        commandBufferSize += m_hwStitchCmdSize;

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::SetDmemBuffer() const
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        int32_t currentPass = m_pipeline->GetCurrentPass();
        if (currentPass < 0 || currentPass >= CODECHAL_VDENC_BRC_NUM_OF_PASSES)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        HucPakIntegrateDmem *hucPakStitchDmem =
            (HucPakIntegrateDmem *)m_allocator->LockResourceForWrite(m_resHucPakStitchDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]);

        ENCODE_CHK_NULL_RETURN(hucPakStitchDmem);
        MOS_ZeroMemory(hucPakStitchDmem, sizeof(HucPakIntegrateDmem));

        // Reset all the offsets to be shared in the huc dmem (6*5 DW's)
        MOS_FillMemory(hucPakStitchDmem, 6 * (MAX_PAK_NUM + 1) * sizeof(uint32_t), 0xFF);

        uint16_t numTileColumns = 1;
        uint16_t numTileRows    = 1;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

        uint32_t numTiles = 1;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileNum, numTiles);

        uint16_t numTilesPerPipe = (uint16_t)(numTiles / m_pipeline->GetPipeNum());

        auto feature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(feature);

        hucPakStitchDmem->TotalSizeInCommandBuffer = numTiles * CODECHAL_CACHELINE_SIZE;
        // Last tile length may get modified by HuC. Obtain last Tile Record, Add an offset of 8bytes to skip address field in Tile Record
        hucPakStitchDmem->OffsetInCommandBuffer = (numTiles - 1) * CODECHAL_CACHELINE_SIZE + 8;
        hucPakStitchDmem->PicWidthInPixel       = (uint16_t)m_basicFeature->m_frameWidth;
        hucPakStitchDmem->PicHeightInPixel      = (uint16_t)m_basicFeature->m_frameHeight;
        hucPakStitchDmem->TotalNumberOfPAKs     = feature->IsBRCEnabled() ? m_pipeline->GetPipeNum() : 0;
        hucPakStitchDmem->Codec                 = 2;  // 1: HEVC DP; 2: HEVC VDEnc; 3: VP9 VDEnc

        hucPakStitchDmem->MAXPass           = feature->IsBRCEnabled() ? CODECHAL_VDENC_BRC_NUM_OF_PASSES : 1;
        hucPakStitchDmem->CurrentPass       = (uint8_t)currentPass + 1;  // Current BRC pass [1..MAXPass]
        hucPakStitchDmem->MinCUSize         = m_basicFeature->m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
        hucPakStitchDmem->CabacZeroWordFlag = true;
        hucPakStitchDmem->bitdepth_luma     = m_basicFeature->m_hevcSeqParams->bit_depth_luma_minus8 + 8;    // default: 8
        hucPakStitchDmem->bitdepth_chroma   = m_basicFeature->m_hevcSeqParams->bit_depth_chroma_minus8 + 8;  // default: 8
        hucPakStitchDmem->ChromaFormatIdc   = m_basicFeature->m_hevcSeqParams->chroma_format_idc;

        uint32_t       lastTileIndex = numTiles - 1;
        EncodeTileData tileData      = {};
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileByIndex, tileData, lastTileIndex);
        hucPakStitchDmem->LastTileBS_StartInBytes = (tileData.bitstreamByteOffset * CODECHAL_CACHELINE_SIZE) & (CODECHAL_PAGE_SIZE - 1);

        hucPakStitchDmem->PIC_STATE_StartInBytes = (uint16_t)m_basicFeature->m_picStateCmdStartInBytes;

        HevcTileStatusInfo hevcTileStatsOffset  = {};
        HevcTileStatusInfo hevcFrameStatsOffset = {};
        HevcTileStatusInfo hevcStatsSize        = {};
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileStatusInfo, hevcTileStatsOffset, hevcFrameStatsOffset, hevcStatsSize);

        if (m_pipeline->GetPipeNum() > 1)
        {
            //Set the kernel output offsets
            hucPakStitchDmem->HEVC_PAKSTAT_offset[0]   = feature->IsBRCEnabled() ? hevcFrameStatsOffset.hevcPakStatistics : 0xFFFFFFFF;
            hucPakStitchDmem->HEVC_Streamout_offset[0] = feature->IsBRCEnabled() ? hevcFrameStatsOffset.hevcSliceStreamout : 0xFFFFFFFF;
            hucPakStitchDmem->TileSizeRecord_offset[0] = hevcFrameStatsOffset.tileSizeRecord;
            hucPakStitchDmem->VDENCSTAT_offset[0]      = feature->IsBRCEnabled() ? hevcFrameStatsOffset.vdencStatistics : 0xFFFFFFFF;

            // Calculate number of slices that execute on a single pipe
            for (auto tileRow = 0; tileRow < numTileRows; tileRow++)
            {
                for (auto tileCol = 0; tileCol < numTileColumns; tileCol++)
                {
                    PCODEC_ENCODER_SLCDATA slcData = m_basicFeature->m_slcData;
                    uint16_t               slcCount, idx, sliceNumInTile = 0;

                    idx = tileRow * numTileColumns + tileCol;
                    for (slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
                    {
                        bool lastSliceInTile = false, sliceInTile = false;

                        EncodeTileData curTileData = {};
                        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileByIndex, curTileData, idx);
                        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsSliceInTile, slcCount, &curTileData, &sliceInTile, &lastSliceInTile);

                        if (!sliceInTile)
                        {
                            continue;
                        }

                        sliceNumInTile++;
                    }  // end of slice
                    if (0 == sliceNumInTile)
                    {
                        // One tile must have at least one slice
                        ENCODE_ASSERT(false);
                        eStatus = MOS_STATUS_INVALID_PARAMETER;
                        break;
                    }
                    // Set the number of slices per pipe in the Dmem structure
                    hucPakStitchDmem->NumSlices[tileCol] += sliceNumInTile;
                }
            }

            for (auto i = 0; i < m_pipeline->GetPipeNum(); i++)
            {
                hucPakStitchDmem->NumTiles[i]  = numTilesPerPipe;
                hucPakStitchDmem->NumSlices[i] = numTilesPerPipe;  // Assuming 1 slice/ tile. To do: change this later.

                // Statistics are dumped out at a tile level. Driver shares with kernel starting offset of each pipe statistic.
                // Offset is calculated by adding size of statistics/pipe to the offset in combined statistics region.
                hucPakStitchDmem->TileSizeRecord_offset[i + 1] = (i * numTilesPerPipe * hevcStatsSize.tileSizeRecord) + hevcTileStatsOffset.tileSizeRecord;
                hucPakStitchDmem->HEVC_PAKSTAT_offset[i + 1]   = (i * numTilesPerPipe * hevcStatsSize.hevcPakStatistics) + hevcTileStatsOffset.hevcPakStatistics;
                hucPakStitchDmem->VDENCSTAT_offset[i + 1]      = (i * numTilesPerPipe * hevcStatsSize.vdencStatistics) + hevcTileStatsOffset.vdencStatistics;
                hucPakStitchDmem->HEVC_Streamout_offset[i + 1] = (i * hucPakStitchDmem->NumSlices[i] * CODECHAL_CACHELINE_SIZE) + hevcTileStatsOffset.hevcSliceStreamout;
            }
        }
        else
        {
            hucPakStitchDmem->NumTiles[0]       = (uint16_t)numTiles;
            hucPakStitchDmem->TotalNumberOfPAKs = m_pipeline->GetPipeNum();

            // non-scalable mode, only VDEnc statistics need to be aggregated
            hucPakStitchDmem->VDENCSTAT_offset[0] = hevcFrameStatsOffset.vdencStatistics;
            hucPakStitchDmem->VDENCSTAT_offset[1] = hevcTileStatsOffset.vdencStatistics;
        }

        bool isTileReplayEnabled = false;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, IsTileReplayEnabled, isTileReplayEnabled);
        if (m_basicFeature->m_enableTileStitchByHW && (isTileReplayEnabled || m_pipeline->GetPipeNum() > 1))
        {
            hucPakStitchDmem->StitchEnable        = true;
            hucPakStitchDmem->StitchCommandOffset = 0;
            hucPakStitchDmem->BBEndforStitch      = HUC_BATCH_BUFFER_END;
        }

        m_allocator->UnLock(m_resHucPakStitchDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]);

        return eStatus;
    }

    MOS_STATUS HevcPakIntegratePkt::ReadSseStatistics(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        // implement SSE
        ENCODE_FUNC_CALL();

        PMOS_RESOURCE osResource = nullptr;
        uint32_t      offset     = 0;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportSumSquareError, osResource, offset));

        for (auto i = 0; i < 3; i++)  // 64 bit SSE values for luma/ chroma channels need to be copied
        {
            auto &miCpyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
            miCpyMemMemParams             = {};
            MOS_RESOURCE *resHuCPakAggregatedFrameStatsBuffer = nullptr;
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetHucPakAggregatedFrameStatsBuffer, resHuCPakAggregatedFrameStatsBuffer);
            ENCODE_CHK_NULL_RETURN(resHuCPakAggregatedFrameStatsBuffer);
            bool tiles_enabled = false;
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tiles_enabled);
            miCpyMemMemParams.presSrc     = tiles_enabled && (m_pipeline->GetPipeNum() > 1) ? resHuCPakAggregatedFrameStatsBuffer : m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);
            miCpyMemMemParams.dwSrcOffset = (m_basicFeature->m_hevcPakStatsSSEOffset + i) * sizeof(uint32_t);  // SSE luma offset is located at DW32 in Frame statistics, followed by chroma
            miCpyMemMemParams.presDst     = osResource;
            miCpyMemMemParams.dwDstOffset = offset + i * sizeof(uint32_t);
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(&cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::ReadSliceSize(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        // Use FrameStats buffer if in single pipe mode.
        if (m_pipeline->GetPipeNum() == 1)
        {
            return ReadSliceSizeForSinglePipe(cmdBuffer);
        }

        // In multi-tile multi-pipe mode, use PAK integration kernel output
        // PAK integration kernel accumulates frame statistics across tiles, which should be used to setup slice size report
        // Report slice size to app only when dynamic scaling is enabled
        if (!m_basicFeature->m_hevcSeqParams->SliceSizeControl)
        {
            return eStatus;
        }

        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeDss, HevcFeatureIDs::hevcVdencDssFeature, ReadSliceSize, m_pipeline, cmdBuffer);
        return eStatus;
    }

    MOS_STATUS HevcPakIntegratePkt::ReadSliceSizeForSinglePipe(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

         // Report slice size to app only when dynamic slice is enabled
        if (!m_basicFeature->m_hevcSeqParams->SliceSizeControl)
        {
            return eStatus;
        }
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeDss, HevcFeatureIDs::hevcVdencDssFeature, ReadSliceSizeForSinglePipe, m_pipeline, cmdBuffer);

        return eStatus;
    }

    MOS_STATUS HevcPakIntegratePkt::SetupTilesStatusData(void *mfxStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(mfxStatus);
        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        EncodeStatusMfx *       encodeStatusMfx  = (EncodeStatusMfx *)mfxStatus;
        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;

        uint32_t statBufIdx     = statusReportData->currOriginalPic.FrameIdx;
        const EncodeReportTileData *tileReportData = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetReportTileData, statBufIdx, tileReportData);
        if(tileReportData == nullptr)
         {
            // When Tile feature is not enabled, not need following complete options
            ENCODE_NORMALMESSAGE("Free tileReportData for frames, which include only one tile.");
            return MOS_STATUS_SUCCESS;
        }

        if (tileReportData[0].reportValid == false)
        {
            // Only multi-pipe contain tile report data. No tile report data needed for one-pipe.
            return MOS_STATUS_SUCCESS;
        }

        statusReportData->codecStatus                                           = CODECHAL_STATUS_SUCCESSFUL;
        statusReportData->panicMode                                             = false;
        statusReportData->averageQP                                             = 0;
        statusReportData->qpY                                                   = 0;
        statusReportData->suggestedQPYDelta                                     = 0;
        statusReportData->numberPasses                                          = 1;
        statusReportData->bitstreamSize                                         = 0;
        statusReportData->numberSlices                                          = 0;
        encodeStatusMfx->imageStatusCtrlOfLastBRCPass.hcpCumulativeFrameDeltaQP = 0;

        // Allocate the tile size report memory
        statusReportData->sizeOfTileInfoBuffer = statusReportData->numberTilesInFrame * sizeof(CodechalTileInfo);
        if (statusReportData->hevcTileinfo)
        {
            MOS_FreeMemory(statusReportData->hevcTileinfo);
        }
        statusReportData->hevcTileinfo = (CodechalTileInfo *)MOS_AllocAndZeroMemory(statusReportData->sizeOfTileInfoBuffer);
        ENCODE_CHK_NULL_RETURN(statusReportData->hevcTileinfo);

        MOS_RESOURCE *tileSizeStatusBuffer = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRecordBuffer, statBufIdx, tileSizeStatusBuffer);
        ENCODE_CHK_NULL_RETURN(tileSizeStatusBuffer);

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        PakHwTileSizeRecord *tileStatusReport =
            (PakHwTileSizeRecord *)m_allocator->Lock(tileSizeStatusBuffer, &lockFlags);
        ENCODE_CHK_NULL_RETURN(tileStatusReport);

        uint32_t *sliceSize = nullptr;

        // pSliceSize is set/ allocated only when dynamic slice is enabled. Cannot use SSC flag here, as it is an asynchronous call
        if (encodeStatusMfx->sliceReport.sliceSize)
        {
            sliceSize = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, encodeStatusMfx->sliceReport.sliceSize, &lockFlags);
            ENCODE_CHK_NULL_RETURN(sliceSize);
        }
        encodeStatusMfx->imageStatusCtrlOfLastBRCPass.hcpCumulativeFrameDeltaQP = 0;

        uint32_t totalCU    = 0;
        uint32_t sliceCount = 0;
        double   sumQp      = 0.0;
        for (uint32_t i = 0; i < statusReportData->numberTilesInFrame; i++)
        {
            if (tileStatusReport[i].Length == 0)
            {
                statusReportData->codecStatus = CODECHAL_STATUS_INCOMPLETE;
                return MOS_STATUS_SUCCESS;
            }

            // Tile Replay currently shares same frame level status report as tile

            statusReportData->hevcTileinfo[i].TileSizeInBytes = tileStatusReport[i].Length;
            // The offset only valid if there is no stream stitching
            statusReportData->hevcTileinfo[i].TileBitStreamOffset = tileReportData[i].bitstreamByteOffset * CODECHAL_CACHELINE_SIZE;
            statusReportData->hevcTileinfo[i].TileRowNum          = i / tileReportData[i].numTileColumns;
            statusReportData->hevcTileinfo[i].TileColNum          = i % tileReportData[i].numTileColumns;
            statusReportData->numTileReported                     = i + 1;
            statusReportData->bitstreamSize += tileStatusReport[i].Length;
            totalCU += (tileReportData[i].tileHeightInMinCbMinus1 + 1) * (tileReportData[i].tileWidthInMinCbMinus1 + 1);
            sumQp += tileStatusReport[i].Hcp_Qp_Status_Count;

            //Add silce Size Control support in each tile
            if (sliceSize)
            {
                statusReportData->sliceSizes = (uint16_t *)sliceSize;
                statusReportData->numberSlices += (uint8_t)tileStatusReport[i].Hcp_Slice_Count_Tile;
                uint16_t prevCumulativeSliceSize = 0;
                // HW writes out a DW for each slice size. Copy in place the DW into 16bit fields expected by App
                for (uint32_t idx = 0; idx < tileStatusReport[i].Hcp_Slice_Count_Tile; idx++)
                {
                    // PAK output the sliceSize at 16DW intervals.
                    ENCODE_CHK_NULL_RETURN(&sliceSize[sliceCount * 16]);

                    //convert cummulative slice size to individual, first slice may have PPS/SPS,
                    uint32_t CurrAccumulatedSliceSize           = sliceSize[sliceCount * 16];
                    statusReportData->sliceSizes[sliceCount] = CurrAccumulatedSliceSize - prevCumulativeSliceSize;
                    prevCumulativeSliceSize += statusReportData->sliceSizes[sliceCount];
                    sliceCount++;
                }
            }
        }

        if (sliceSize)
        {
            statusReportData->sizeOfSliceSizesBuffer = sizeof(uint16_t) * statusReportData->numberSlices;
            statusReportData->sliceSizeOverflow      = (encodeStatusMfx->sliceReport.sliceSizeOverflow >> 16) & 1;
            m_osInterface->pfnUnlockResource(m_osInterface, encodeStatusMfx->sliceReport.sliceSize);
        }

        if (statusReportData->bitstreamSize == 0 ||
            statusReportData->bitstreamSize > m_basicFeature->m_bitstreamSize)
        {
            statusReportData->codecStatus   = CODECHAL_STATUS_ERROR;
            statusReportData->bitstreamSize = 0;
            return MOS_STATUS_INVALID_FILE_SIZE;
        }

        if (totalCU != 0)
        {
            statusReportData->qpY = statusReportData->averageQP =
                (uint8_t)((sumQp / (double)totalCU) / 4.0);  // due to TU is 4x4 and there are 4 TUs in one CU
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (!m_basicFeature->m_enableTileStitchByHW && m_pipeline->GetPipeNum() > 1)
        {
            ENCODE_CHK_STATUS_RETURN(PerformSwStitch(tileReportData, tileStatusReport, statusReportData));
        }

        if (tileStatusReport)
        {
            // clean-up the tile status report buffer
            MOS_ZeroMemory(tileStatusReport, sizeof(tileStatusReport[0]) * statusReportData->numberTilesInFrame);
            m_allocator->UnLock(tileSizeStatusBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(mfxStatus);
        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;

        if (statusReportData->numberTilesInFrame == 1)
        {
            // When Tile feature is not enabled, not need following complete options
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Completed(mfxStatus, rcsStatus, statusReport));

        // Tile status data is only update and performed in multi-pipe mode
        ENCODE_CHK_STATUS_RETURN(SetupTilesStatusData(mfxStatus, statusReport));

        m_basicFeature->Reset((CODEC_REF_LIST *)statusReportData->currRefList);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::PerformSwStitch(
        const EncodeReportTileData *tileReportData,
        PakHwTileSizeRecord *       tileStatusReport,
        EncodeStatusReportData *    statusReportData)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(tileReportData);
        ENCODE_CHK_NULL_RETURN(tileStatusReport);

        uint8_t *tempBsBuffer = nullptr, *bufPtr = nullptr;
        tempBsBuffer = bufPtr = (uint8_t *)MOS_AllocAndZeroMemory(statusReportData->bitstreamSize);
        ENCODE_CHK_NULL_RETURN(tempBsBuffer);

        PCODEC_REF_LIST currRefList = (PCODEC_REF_LIST)statusReportData->currRefList;

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly = 1;
        uint8_t *bitstream = (uint8_t *)m_allocator->Lock(
            &currRefList->resBitstreamBuffer,
            &lockFlags);
        if (bitstream == nullptr)
        {
            MOS_FreeMemory(tempBsBuffer);
            ENCODE_CHK_NULL_RETURN(nullptr);
        }

        for (uint32_t i = 0; i < statusReportData->numberTilesInFrame; i++)
        {
            uint32_t offset = tileReportData[i].bitstreamByteOffset * CODECHAL_CACHELINE_SIZE;
            uint32_t len    = tileStatusReport[i].Length;

            MOS_SecureMemcpy(bufPtr, len, &bitstream[offset], len);
            bufPtr += len;
        }

        MOS_SecureMemcpy(bitstream, statusReportData->bitstreamSize, tempBsBuffer, statusReportData->bitstreamSize);
        MOS_ZeroMemory(&bitstream[statusReportData->bitstreamSize], m_basicFeature->m_bitstreamSize - statusReportData->bitstreamSize);

        if (bitstream)
        {
            m_allocator->UnLock(&currRefList->resBitstreamBuffer);
        }

        MOS_FreeMemory(tempBsBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::PerformHwStitch(
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        // 2nd level BB buffer for stitching cmd
        // Current location to add cmds in 2nd level batch buffer
        m_HucStitchCmdBatchBuffer.iCurrent = 0;
        // Reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
        m_HucStitchCmdBatchBuffer.dwOffset = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(cmdBuffer, &m_HucStitchCmdBatchBuffer));
        // This wait cmd is needed to make sure copy command is done as suggested by HW folk in encode cases
        auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = m_osInterface->osCpInterface->IsCpEnabled() ? true : false;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::ConfigStitchDataBuffer() const
    {
        ENCODE_FUNC_CALL();

        auto currPass = m_pipeline->GetCurrentPass();
        HucCommandData *hucStitchDataBuf = (HucCommandData*)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE*>(&m_resHucStitchDataBuffer[m_pipeline->m_currRecycledBufIdx][currPass]));
        ENCODE_CHK_NULL_RETURN(hucStitchDataBuf);

        MOS_ZeroMemory(hucStitchDataBuf, sizeof(HucCommandData));
        hucStitchDataBuf->TotalCommands          = 1;
        hucStitchDataBuf->InputCOM[0].SizeOfData = 0xf;

        uint16_t numTileColumns = 1;
        uint16_t numTileRows    = 1;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

        HucInputCmd hucInputCmd;
        MOS_ZeroMemory(&hucInputCmd, sizeof(HucInputCmd));

        ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);
        hucInputCmd.SelectionForIndData = m_osInterface->osCpInterface->IsCpEnabled() ? 4 : 0;
        hucInputCmd.CmdMode             = HUC_CMD_LIST_MODE;
        hucInputCmd.LengthOfTable       = numTileRows * numTileColumns;
        hucInputCmd.CopySize            = m_hwInterface->m_tileRecordSize;

        // Tile record always in m_tileRecordBuffer even in scalable node
        uint32_t      statBufIdx = m_basicFeature->m_currOriginalPic.FrameIdx;
        MOS_RESOURCE *presSrc    = nullptr;

        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRecordBuffer, statBufIdx, presSrc);
        ENCODE_CHK_NULL_RETURN(presSrc);

        ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnRegisterResource(
            m_osInterface,
            presSrc,
            false,
            false));

        ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnRegisterResource(
            m_osInterface,
            &m_basicFeature->m_resBitstreamBuffer,
            true,
            true));

        uint64_t srcAddr = m_osInterface->pfnGetResourceGfxAddress(m_osInterface, presSrc);
        uint64_t destrAddr = m_osInterface->pfnGetResourceGfxAddress(m_osInterface, &m_basicFeature->m_resBitstreamBuffer);
        hucInputCmd.SrcAddrBottom  = (uint32_t)(srcAddr & 0x00000000FFFFFFFF);
        hucInputCmd.SrcAddrTop     = (uint32_t)((srcAddr & 0xFFFFFFFF00000000) >> 32);
        hucInputCmd.DestAddrBottom = (uint32_t)(destrAddr & 0x00000000FFFFFFFF);
        hucInputCmd.DestAddrTop    = (uint32_t)((destrAddr & 0xFFFFFFFF00000000) >> 32);

        MOS_SecureMemcpy(hucStitchDataBuf->InputCOM[0].data, sizeof(HucInputCmd), &hucInputCmd, sizeof(HucInputCmd));

        m_allocator->UnLock(const_cast<MOS_RESOURCE*>(&m_resHucStitchDataBuffer[m_pipeline->m_currRecycledBufIdx][currPass]));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, HevcPakIntegratePkt)
    {
        params.kernelDescriptor = m_vdboxHucPakIntKernelDescriptor;
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, HevcPakIntegratePkt)
    {
        params.function = PAK_INTEGRATE;

        ENCODE_CHK_STATUS_RETURN(SetDmemBuffer());

        int32_t currentPass  = m_pipeline->GetCurrentPass();
        params.hucDataSource = m_resHucPakStitchDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass];
        params.dataLength    = MOS_ALIGN_CEIL(sizeof(HucPakIntegrateDmem), CODECHAL_CACHELINE_SIZE);
        params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, HevcPakIntegratePkt)
    {
        params.function = PAK_INTEGRATE;

        uint32_t statBufIdx = 0;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetStatisticsBufferIndex, statBufIdx);

        MOS_RESOURCE *resTileBasedStatisticsBuffer = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileBasedStatisticsBuffer, statBufIdx, resTileBasedStatisticsBuffer);
        MOS_RESOURCE *resHuCPakAggregatedFrameStatsBuffer = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetHucPakAggregatedFrameStatsBuffer, resHuCPakAggregatedFrameStatsBuffer);
        MOS_RESOURCE *resTileRecordBuffer = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRecordBuffer, statBufIdx, resTileRecordBuffer);
        uint32_t numTiles = 1;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileNum, numTiles);
        uint32_t       lastTileIndex = numTiles - 1;
        EncodeTileData tileData      = {};
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileByIndex, tileData, lastTileIndex);

        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);
        auto vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);

        // Add Virtual addr
        params.regionParams[0].presRegion = resTileBasedStatisticsBuffer;                 // Region 0 Input - Tile based input statistics from PAK/ VDEnc
        params.regionParams[0].dwOffset   = 0;
        params.regionParams[1].presRegion = resHuCPakAggregatedFrameStatsBuffer;          // Region 1 Output - HuC Frame statistics output
        params.regionParams[1].isWritable = true;

        params.regionParams[4].presRegion = &m_basicFeature->m_resBitstreamBuffer;        // Region 4 Input - Last Tile bitstream
        params.regionParams[4].dwOffset   = MOS_ALIGN_FLOOR(tileData.bitstreamByteOffset * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
        params.regionParams[5].presRegion = &m_basicFeature->m_resBitstreamBuffer;        // Region 5 Output - HuC modifies the last tile bitstream before stitch
        params.regionParams[5].dwOffset   = MOS_ALIGN_FLOOR(tileData.bitstreamByteOffset * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
        params.regionParams[5].isWritable = true;
        params.regionParams[6].presRegion =
            m_basicFeature->m_recycleBuf->GetBuffer(VdencBRCHistoryBuffer, m_basicFeature->m_frameNum); // Region 6 Output - History Buffer (Input/Output)
        params.regionParams[6].isWritable = true;
        params.regionParams[7].presRegion = &vdenc2ndLevelBatchBuffer->OsResource;         // Region 7 Input- HCP PIC state command
        MOS_RESOURCE *resBrcDataBuffer                 = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature, GetBrcDataBuffer, resBrcDataBuffer);
        params.regionParams[9].presRegion = resBrcDataBuffer;                              // Region 9 Output - HuC outputs BRC data
        params.regionParams[9].isWritable = true;

        params.regionParams[15].presRegion = resTileRecordBuffer;
        params.regionParams[15].dwOffset = 0;

        bool isTileReplayEnabled = false;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, FeatureIDs::encodeTile, IsTileReplayEnabled, isTileReplayEnabled);
        if (m_basicFeature->m_enableTileStitchByHW && (isTileReplayEnabled || m_pipeline->GetPipeNum() > 1))
        {
            ENCODE_CHK_STATUS_RETURN(ConfigStitchDataBuffer());

            uint32_t currentPass               = m_pipeline->GetCurrentPass();
            params.regionParams[8].presRegion  = const_cast<PMOS_RESOURCE>(&m_resHucStitchDataBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]);  // Region 8 - data buffer read by HUC for stitching cmd generation
            params.regionParams[10].presRegion = const_cast<PMOS_RESOURCE>(&m_HucStitchCmdBatchBuffer.OsResource);  // Region 10 - SLB for stitching cmd output from Huc
            params.regionParams[10].isWritable = true;
        }

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS HevcPakIntegratePkt::DumpInput()
    {
        ENCODE_FUNC_CALL();
        int32_t currentPass = m_pipeline->GetCurrentPass();

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
            m_resHucPakStitchDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass],
            m_vdencHucPakDmemBufferSize,
            currentPass,
            hucRegionDumpPakIntegrate));

        ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_TileBasedStatistic", true, hucRegionDumpPakIntegrate));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(4, "_Bitstream", true, hucRegionDumpPakIntegrate));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(7, "_HcpPicState", true, hucRegionDumpPakIntegrate));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(15, "_TileRecord", true, hucRegionDumpPakIntegrate));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcPakIntegratePkt::DumpOutput()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(DumpRegion(1, "_HuCPakAggregatedFrameStats", false, hucRegionDumpPakIntegrate));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(5, "_Bitstream", false, hucRegionDumpPakIntegrate));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(6, "_BrcHistory", false, hucRegionDumpPakIntegrate));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(9, "_OutputBrcData", false, hucRegionDumpPakIntegrate));
        ENCODE_CHK_STATUS_RETURN(DumpRegion(10, "_StitchCmd", false, hucRegionDumpPakIntegrate));

        return MOS_STATUS_SUCCESS;
    }
#endif

     MOS_STATUS HevcPakIntegratePkt::AddCondBBEndForLastPass(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        if (m_pipeline->IsSingleTaskPhaseSupported() || m_pipeline->IsFirstPass() || m_pipeline->GetPassNum() == 1)
        {
            return MOS_STATUS_SUCCESS;
        }

        auto &miConditionalBatchBufferEndParams = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        miConditionalBatchBufferEndParams       = {};

        // VDENC uses HuC FW generated semaphore for conditional 2nd pass
        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(&cmdBuffer));

        auto          mmioRegisters = m_hcpItf->GetMmioRegisters(m_vdboxIndex);
        MOS_RESOURCE *osResource    = nullptr;
        uint32_t      offset        = 0;
        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportImageStatusCtrl, osResource, offset));
        //uint32_t baseOffset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) + sizeof(uint32_t) * 2;  // encodeStatus is offset by 2 DWs in the resource

        // Write back the HCP image control register for RC6 may clean it out
        auto &registerMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = osResource;
        registerMemParams.dwOffset        = offset;
        registerMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(&cmdBuffer));

        HevcVdencBrcBuffers *vdencBrcBuffers = nullptr;
        auto                 feature         = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(feature);
        vdencBrcBuffers = feature->GetHevcVdencBrcBuffers();
        ENCODE_CHK_NULL_RETURN(vdencBrcBuffers);

        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = vdencBrcBuffers->resBrcPakStatisticBuffer[vdencBrcBuffers->currBrcPakStasIdxForWrite];
        miStoreRegMemParams.dwOffset        = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS);
        miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportImageStatusCtrlOfLastBRCPass, osResource, offset));
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = osResource;
        miStoreRegMemParams.dwOffset        = offset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    }
