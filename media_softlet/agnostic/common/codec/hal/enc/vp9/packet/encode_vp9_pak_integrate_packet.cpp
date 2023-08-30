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
//! \file     encode_vp9_pak_integrate_packet.cpp
//! \brief    Defines the interface for vp9 pak integrate packet
//!
#include "encode_vp9_pak_integrate_packet.h"
#include "mos_os_cp_interface_specific.h"

namespace encode
{
Vp9PakIntegratePkt::~Vp9PakIntegratePkt()
{
    FreeResources();
}

MOS_STATUS Vp9PakIntegratePkt::Init()
{
    ENCODE_FUNC_CALL();

    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(Vp9FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    m_hcpInterfaceNew = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
    ENCODE_CHK_NULL_RETURN(m_hcpInterfaceNew);

    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Init());

    ENCODE_CHK_NULL_RETURN(m_pipeline);
#ifdef _MMC_SUPPORTED
    m_mmcState = m_pipeline->GetMmcState();
    ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif  // __MMC_SUPPORTED

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    bool firstTaskPhase = packetPhase & firstPacket;
    bool requestProlog  = false;

    if (m_basicFeature->m_hucEnabled)
    {
        // Huc Basic
        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog));

        // Add huc status update to status buffer
        PMOS_RESOURCE osResource = nullptr;
        uint32_t      offset     = 0;
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

        ENCODE_CHK_NULL_RETURN(m_hwInterface->GetHucInterfaceNext());
        auto  mmioRegisters                 = m_hwInterface->GetHucInterfaceNext()->GetMmioRegisters(m_vdboxIndex);
        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = osResource;
        miStoreRegMemParams.dwOffset        = offset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->hucStatusRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(commandBuffer));

        if (m_basicFeature->m_enableTileStitchByHW)
        {
            // 2nd level BB buffer for stitching cmd
            // Current location to add cmds in 2nd level batch buffer
            m_HucStitchCmdBatchBuffer.iCurrent = 0;
            // Reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
            m_HucStitchCmdBatchBuffer.dwOffset = 0;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(commandBuffer, &m_HucStitchCmdBatchBuffer));
            // This wait cmd is needed to make sure copy command is done as suggested by HW folk in encode cases
            auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
            mfxWaitParams                     = {};
            mfxWaitParams.iStallVdboxPipeline = m_osInterface->osCpInterface->IsCpEnabled() ? true : false;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(commandBuffer));
        }
    }

    ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, commandBuffer));
    if (false == m_pipeline->IsFrameTrackingEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(UpdateStatusReportNext(statusReportGlobalCount, commandBuffer));
    }

    CODECHAL_DEBUG_TOOL(
        if (m_mmcState) {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_reconSurface));
        }
    )

    // Reset parameters for next PAK execution
    if (false == m_pipeline->IsFrameTrackingEnabled())
    {
        UpdateParameters();
    }

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpInput());
    )

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    ENCODE_FUNC_CALL();

    uint32_t                       hucCommandsSize  = 0;
    uint32_t                       hucPatchListSize = 0;
    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

    ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
        m_basicFeature->m_mode, (uint32_t *)&hucCommandsSize, (uint32_t *)&hucPatchListSize, &stateCmdSizeParams));

    commandBufferSize      = hucCommandsSize;
    requestedPatchListSize = m_osInterface->bUsesPatchList ? hucPatchListSize : 0;

    // Reserve cmd size for hw stitch
    commandBufferSize += m_hwStitchCmdSize;

    // 4K align since allocation is in chunks of 4K bytes.
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePkt::DumpOutput()
{
    ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL

    // Region 1 - HuC Frame statistics output
    ENCODE_CHK_STATUS_RETURN(DumpRegion(1, "_IntegratedStreamout_output", false, hucRegionDumpPakIntegrate, m_hwInterface->m_pakIntAggregatedFrameStatsSize));
    // Region 8 - data buffer read by HUC for stitching cmd generation
    ENCODE_CHK_STATUS_RETURN(DumpRegion(8, "_HucStitchDataBuffer", false, hucRegionDumpPakIntegrate, MOS_ALIGN_CEIL(sizeof(HucCommandData), CODECHAL_PAGE_SIZE)));
    // Region 9 - HuC outputs BRC data
    ENCODE_CHK_STATUS_RETURN(DumpRegion(9, "_BrcDataOutputBuffer", false, hucRegionDumpPakIntegrate));
    // Region 10 - SLB for stitching cmd output from Huc
    ENCODE_CHK_STATUS_RETURN(DumpRegion(10, "_SLBHucStitchCmdBuffer", false, hucRegionDumpPakIntegrate, m_hwInterface->m_HucStitchCmdBatchBufferSize));
    // Region 15 [In/Out] - Tile Record Buffer
    ENCODE_CHK_STATUS_RETURN(DumpRegion(15, "_TileRecordBuffer", false, hucRegionDumpPakIntegrate, m_basicFeature->m_maxTileNumber * MOS_ALIGN_CEIL(m_hcpInterfaceNew->GetPakHWTileSizeRecordSize(), CODECHAL_CACHELINE_SIZE)));

#endif  // USE_CODECHAL_DEBUG_TOOL

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(mfxStatus);
    ENCODE_CHK_NULL_RETURN(statusReport);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;

    if (statusReportData->numberTilesInFrame == 1 || !m_basicFeature->m_scalableMode)
    {
        // When Tile feature is not enabled or not in scalable mode, not need following complete options
        return MOS_STATUS_SUCCESS;
    }

    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Completed(mfxStatus, rcsStatus, statusReport));

    // Tile status data is only update and performed in multi-pipe mode
    ENCODE_CHK_STATUS_RETURN(SetupTilesStatusData(mfxStatus, statusReport));

    m_basicFeature->Reset((CODEC_REF_LIST *)statusReportData->currRefList);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePkt::AllocateResources()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

    if (m_basicFeature->m_hucPakIntBrcDataBuffer == nullptr)
    {
        MOS_RESOURCE *          allocatedBuffer = nullptr;
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;

        // HUC PAK Int DMEM buffer
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_hucPakIntDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "Huc Pak Int Dmem Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; ++i)
        {
            for (auto j = 0; j < Vp9EncodeBrc::m_brcMaxNumPasses; ++j)
            {
                allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
                ENCODE_CHK_NULL_RETURN(allocatedBuffer);
                m_hucPakIntDmemBuffer[i][j] = *allocatedBuffer;
            }
        }

        // HuC PAK Int region 7, 8
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(64, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "HUC PAK Int Dummy Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_hucPakIntDummyBuffer = *allocatedBuffer;

        // Allocate region 9 of PAK integration to be fed as input to HuC BRC region 7
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "HUC PAK Integration FrameByteCount output";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
        allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedBuffer);
        m_basicFeature->m_hucPakIntBrcDataBuffer = allocatedBuffer;

        if (m_basicFeature->m_enableTileStitchByHW)
        {
            // HuC stitching data buffer
            allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucCommandData), CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VP9 HuC Stitch Data Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;

            for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; ++i)
            {
                for (auto j = 0; j < CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES; ++j)
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

MOS_STATUS Vp9PakIntegratePkt::SetDmemBuffer() const
{
    ENCODE_FUNC_CALL();

    auto currentPass = m_pipeline->GetCurrentPass();
    if (currentPass >= Vp9EncodeBrc::m_brcMaxNumPasses)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    HucPakIntDmem *dmem = (HucPakIntDmem *)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE *>(&m_hucPakIntDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]));
    ENCODE_CHK_NULL_RETURN(dmem);
    MOS_ZeroMemory(dmem, sizeof(HucPakIntDmem));

    MOS_FillMemory(dmem, m_pakIntDmemOffsetsSize, 0xFF);

    uint16_t numTileColumns = 1;
    uint16_t numTileRows    = 1;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);
    uint32_t numTiles = 1;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileNum, numTiles);

    dmem->totalSizeInCommandBuffer = numTiles * CODECHAL_CACHELINE_SIZE;
    dmem->offsetInCommandBuffer    = 0xFFFF;  // Not used for VP9, all bytes in dmem for fields not used are 0xFF
    dmem->picWidthInPixel          = (uint16_t)m_basicFeature->m_frameWidth;
    dmem->picHeightInPixel         = (uint16_t)m_basicFeature->m_frameHeight;
    dmem->totalNumberOfPaks        = (uint16_t)m_pipeline->GetPipeNum();
    dmem->codec                    = m_pakIntVp9CodecId;
    dmem->maxPass                  = Vp9EncodeBrc::m_brcMaxNumPasses;  // Only VDEnc CQP and BRC
    dmem->currentPass              = currentPass + 1;

    uint32_t       lastTileIndex = numTiles - 1;
    EncodeTileData tileData      = {};
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileByIndex, tileData, lastTileIndex);

    dmem->lastTileBSStartInBytes = tileData.tileSizeStreamoutOffset * CODECHAL_CACHELINE_SIZE + 8;
    dmem->picStateStartInBytes   = 0xFFFF;

    if (m_basicFeature->m_enableTileStitchByHW)
    {
        dmem->StitchEnable        = true;
        dmem->StitchCommandOffset = 0;
        dmem->BBEndforStitch      = HUC_BATCH_BUFFER_END;
    }

    Vp9TileStatusInfo vp9TileStatsOffset  = {};
    Vp9TileStatusInfo vp9FrameStatsOffset = {};
    Vp9TileStatusInfo vp9StatsSize        = {};
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileStatusInfo, vp9TileStatsOffset, vp9FrameStatsOffset, vp9StatsSize);

    // Offset 0 is for region 1 - output of integrated frame stats from PAK integration kernel

    dmem->tileSizeRecordOffset[0]   = vp9FrameStatsOffset.tileSizeRecord;
    dmem->vdencStatOffset[0]        = vp9FrameStatsOffset.vdencStats;
    dmem->vp9PakStatOffset[0]       = vp9FrameStatsOffset.pakStats;
    dmem->vp9CounterBufferOffset[0] = vp9FrameStatsOffset.counterBuffer;

    uint16_t numTilesPerPipe = (uint16_t)(numTiles / m_pipeline->GetPipeNum());

    // Offset 1 - 4 is for region 0 - Input to PAK integration kernel for all tile statistics per pipe
    for (auto i = 1; i <= m_pipeline->GetPipeNum(); ++i)
    {
        dmem->numTiles[i - 1]           = numTilesPerPipe;
        dmem->tileSizeRecordOffset[i]   = vp9TileStatsOffset.tileSizeRecord + ((i - 1) * (dmem->numTiles[i - 1]) * vp9StatsSize.tileSizeRecord);
        dmem->vdencStatOffset[i]        = vp9TileStatsOffset.vdencStats + ((i - 1) * (dmem->numTiles[i - 1]) * vp9StatsSize.vdencStats);
        dmem->vp9PakStatOffset[i]       = vp9TileStatsOffset.pakStats + ((i - 1) * (dmem->numTiles[i - 1]) * vp9StatsSize.pakStats);
        dmem->vp9CounterBufferOffset[i] = vp9TileStatsOffset.counterBuffer + ((i - 1) * (dmem->numTiles[i - 1]) * vp9StatsSize.counterBuffer);
    }

    m_allocator->UnLock(const_cast<MOS_RESOURCE *>(&m_hucPakIntDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePkt::ReadHcpStatus(MHW_VDBOX_NODE_IND vdboxIndex, MediaStatusReport *statusReport, MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

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

    ENCODE_CHK_STATUS_RETURN(m_hwInterface->ReadHcpStatus(vdboxIndex, params, &cmdBuffer));

    auto mmioRegisters = m_hcpInterfaceNew->GetMmioRegisters(vdboxIndex);

    auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    miStoreRegMemParams                 = {};
    miStoreRegMemParams.presStoreBuffer = params.resBitstreamByteCountPerFrame;
    miStoreRegMemParams.dwOffset        = params.bitstreamByteCountPerFrameOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->hcpVp9EncBitstreamBytecountFrameRegOffset;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(m_hwInterface->ReadImageStatusForHcp(vdboxIndex, params, &cmdBuffer));

    HucBrcBuffers *hucBrcBuffers = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, GetHucBrcBuffers, hucBrcBuffers);
    ENCODE_CHK_NULL_RETURN(hucBrcBuffers);

    auto &copyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
    copyMemMemParams             = {};
    copyMemMemParams.presSrc     = params.resBitstreamByteCountPerFrame;
    copyMemMemParams.dwSrcOffset = params.bitstreamByteCountPerFrameOffset;
    copyMemMemParams.presDst     = &(hucBrcBuffers->resBrcBitstreamSizeBuffer);
    copyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(EncodeVp9BSBuffer, dwHcpBitstreamByteCountFrame);
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePkt::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(cmdBuffer);

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    if (!m_basicFeature->m_scalableMode)
    {
        // Single pipe mode can read the info from MMIO register. Otherwise,
        // we have to use the tile size statistic buffer
        ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(m_vdboxIndex, m_statusReport, *cmdBuffer));
    }
    ENCODE_CHK_STATUS_RETURN(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    ENCODE_CHK_NULL_RETURN(perfProfiler);
    ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
        (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

void Vp9PakIntegratePkt::UpdateParameters()
{
    ENCODE_FUNC_CALL();

    if (!m_pipeline->IsSingleTaskPhaseSupported())
    {
        m_osInterface->pfnResetPerfBufferID(m_osInterface);
    }
}

MOS_STATUS Vp9PakIntegratePkt::SetupTilesStatusData(void *mfxStatus, void *statusReport)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(mfxStatus);
    ENCODE_CHK_NULL_RETURN(statusReport);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    EncodeStatusMfx *       encodeStatusMfx  = (EncodeStatusMfx *)mfxStatus;
    EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;

    uint32_t                    statBufIdx     = statusReportData->currOriginalPic.FrameIdx;
    const EncodeReportTileData *tileReportData = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetReportTileData, statBufIdx, tileReportData);
    ENCODE_CHK_NULL_RETURN(tileReportData);

    MOS_RESOURCE *tileSizeStatusBuffer = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileRecordBuffer, statBufIdx, tileSizeStatusBuffer);
    ENCODE_CHK_NULL_RETURN(tileSizeStatusBuffer);

    PakHwTileSizeRecord *tileStatusReport =
        (PakHwTileSizeRecord *)m_allocator->LockResourceForRead(tileSizeStatusBuffer);
    ENCODE_CHK_NULL_RETURN(tileStatusReport);

    statusReportData->codecStatus       = CODECHAL_STATUS_SUCCESSFUL;
    statusReportData->panicMode         = false;
    statusReportData->averageQP         = 0;
    statusReportData->qpY               = 0;
    statusReportData->suggestedQPYDelta = 0;
    statusReportData->numberPasses      = 1;
    statusReportData->bitstreamSize     = 0;

    encodeStatusMfx->imageStatusCtrlOfLastBRCPass.hcpCumulativeFrameDeltaQP = 0;

    double   sumQP   = 0.0;
    uint32_t totalCU = 0;
    for (uint32_t i = 0; i < statusReportData->numberTilesInFrame; ++i)
    {
        if (tileStatusReport[i].Length == 0)
        {
            statusReportData->codecStatus = CODECHAL_STATUS_INCOMPLETE;
            // Clean-up the tile status report buffer
            MOS_ZeroMemory(tileStatusReport, sizeof(tileStatusReport[0]) * statusReportData->numberTilesInFrame);
            m_allocator->UnLock(tileSizeStatusBuffer);
            return MOS_STATUS_SUCCESS;
        }
        statusReportData->bitstreamSize += tileStatusReport[i].Length;
        totalCU += (tileReportData[i].tileHeightInMinCbMinus1 + 1) * (tileReportData[i].tileWidthInMinCbMinus1 + 1);
        sumQP += tileStatusReport[i].Hcp_Qp_Status_Count;
    }

    if (totalCU != 0)
    {
        statusReportData->qpY = statusReportData->averageQP =
            (uint8_t)((sumQP / (double)totalCU) / 4.0);  // Due to TU is 4x4 and there are 4 TUs in one CU
    }
    else
    {
        // Clean-up the tile status report buffer
        MOS_ZeroMemory(tileStatusReport, sizeof(tileStatusReport[0]) * statusReportData->numberTilesInFrame);
        m_allocator->UnLock(tileSizeStatusBuffer);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_basicFeature->m_enableTileStitchByHW)
    {
        // Clean-up the tile status report buffer
        MOS_ZeroMemory(tileStatusReport, sizeof(tileStatusReport[0]) * statusReportData->numberTilesInFrame);
        m_allocator->UnLock(tileSizeStatusBuffer);
        return MOS_STATUS_SUCCESS;
    }

    uint8_t *bufPtr       = (uint8_t *)MOS_AllocAndZeroMemory(statusReportData->bitstreamSize);
    uint8_t *tempBsBuffer = bufPtr;

    auto tempTerminateFunc = [&]()
    {
        MOS_SafeFreeMemory(tempBsBuffer);

        // Clean-up the tile status report buffer
        MOS_ZeroMemory(tileStatusReport, sizeof(tileStatusReport[0]) * statusReportData->numberTilesInFrame);
        m_allocator->UnLock(tileSizeStatusBuffer);
    };
    ENCODE_CHK_NULL_WITH_DESTROY_RETURN_VALUE(tempBsBuffer, tempTerminateFunc);

    PCODEC_REF_LIST currRefList = (PCODEC_REF_LIST)statusReportData->currRefList;
    ENCODE_CHK_NULL_WITH_DESTROY_RETURN_VALUE(currRefList, tempTerminateFunc);
    uint8_t *bitstream = (uint8_t *)m_allocator->LockResourceForWrite(&currRefList->resBitstreamBuffer);
    ENCODE_CHK_NULL_WITH_DESTROY_RETURN_VALUE(bitstream, tempTerminateFunc);

    for (uint32_t i = 0; i < statusReportData->numberTilesInFrame; ++i)
    {
        uint32_t offset = tileReportData[i].bitstreamByteOffset * CODECHAL_CACHELINE_SIZE;
        uint32_t len    = tileStatusReport[i].Length;
        if (offset + len >= m_basicFeature->m_bitstreamSize)
        {
            ENCODE_ASSERTMESSAGE("Error: Tile offset and length add up to more than bitstream upper bound");
            statusReportData->codecStatus   = CODECHAL_STATUS_ERROR;
            statusReportData->bitstreamSize = 0;

            MOS_FreeMemory(tempBsBuffer);
            m_allocator->UnLock(&currRefList->resBitstreamBuffer);

            // Clean-up the tile status report buffer
            MOS_ZeroMemory(tileStatusReport, sizeof(tileStatusReport[0]) * statusReportData->numberTilesInFrame);
            m_allocator->UnLock(tileSizeStatusBuffer);

            return MOS_STATUS_INVALID_FILE_SIZE;
        }

        MOS_SecureMemcpy(bufPtr, len, &bitstream[offset], len);
        bufPtr += len;
    }

    MOS_SecureMemcpy(bitstream, statusReportData->bitstreamSize, tempBsBuffer, statusReportData->bitstreamSize);
    MOS_ZeroMemory(&bitstream[statusReportData->bitstreamSize], m_basicFeature->m_bitstreamSize - statusReportData->bitstreamSize);

    MOS_FreeMemory(tempBsBuffer);
    m_allocator->UnLock(&currRefList->resBitstreamBuffer);

    // Clean-up the tile status report buffer
    MOS_ZeroMemory(tileStatusReport, sizeof(tileStatusReport[0]) * statusReportData->numberTilesInFrame);
    m_allocator->UnLock(tileSizeStatusBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePkt::FreeResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    eStatus = Mhw_FreeBb(m_osInterface, &m_HucStitchCmdBatchBuffer, nullptr);
    ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    return eStatus;
}

MOS_STATUS Vp9PakIntegratePkt::ConfigStitchDataBuffer() const
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
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    HucInputCmdG12 hucInputCmd;
    MOS_ZeroMemory(&hucInputCmd, sizeof(HucInputCmdG12));

    ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);
    hucInputCmd.SelectionForIndData = m_osInterface->osCpInterface->IsCpEnabled() ? 4 : 0;
    hucInputCmd.CmdMode             = HUC_CMD_LIST_MODE;
    hucInputCmd.LengthOfTable       = numTileRows * numTileColumns;
    hucInputCmd.CopySize            = m_hwInterface->m_tileRecordSize;

    // Tile record always in m_tileRecordBuffer even in scalable node
    uint32_t      statBufIdx = m_basicFeature->m_currOriginalPic.FrameIdx;
    MOS_RESOURCE *presSrc    = nullptr;

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileRecordBuffer, statBufIdx, presSrc);
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

    MOS_SecureMemcpy(hucStitchDataBuf->InputCOM[0].data, sizeof(HucInputCmdG12), &hucInputCmd, sizeof(HucInputCmdG12));

    m_allocator->UnLock(const_cast<MOS_RESOURCE*>(&m_resHucStitchDataBuffer[m_pipeline->m_currRecycledBufIdx][currPass]));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Vp9PakIntegratePkt::DumpInput()
{
    ENCODE_FUNC_CALL();

    int32_t currentPass = m_pipeline->GetCurrentPass();

    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
        &m_hucPakIntDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass],
        m_hucPakIntDmemBufferSize,
        currentPass,
        hucRegionDumpPakIntegrate));

    // Region 0 - Tile based input statistics from PAK/ VDEnc
    ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_PakIntStitchBuffer", true, hucRegionDumpPakIntegrate, m_hwInterface->m_pakIntTileStatsSize));
    // Region 15 [In/Out] - Tile Record Buffer
    ENCODE_CHK_STATUS_RETURN(DumpRegion(15, "_TileRecordBuffer", true, hucRegionDumpPakIntegrate, m_basicFeature->m_maxTileNumber * MOS_ALIGN_CEIL(m_hcpInterfaceNew->GetPakHWTileSizeRecordSize(), CODECHAL_CACHELINE_SIZE)));

    return MOS_STATUS_SUCCESS;
}
#endif  // USE_CODECHAL_DEBUG_TOOL

MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, Vp9PakIntegratePkt)
{
    ENCODE_FUNC_CALL();

    params.kernelDescriptor = m_vdboxHucPakIntegrationKernelDescriptor;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, Vp9PakIntegratePkt)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(SetDmemBuffer());

    params.function = PAK_INTEGRATE;
    uint32_t currentPass = m_pipeline->GetCurrentPass();
    params.hucDataSource = const_cast<PMOS_RESOURCE>(&m_hucPakIntDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]);
    params.dataLength    = MOS_ALIGN_CEIL(m_hucPakIntDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, Vp9PakIntegratePkt)
{
    ENCODE_FUNC_CALL();

    params.function = PAK_INTEGRATE;

    if (m_basicFeature->m_enableTileStitchByHW)
    {
        ENCODE_CHK_STATUS_RETURN(ConfigStitchDataBuffer());
    }

    uint32_t currentPass = m_pipeline->GetCurrentPass();

    // Region 0, 1, 15
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, SetRegionsForPakInt, params);

    params.regionParams[4].presRegion              = const_cast<PMOS_RESOURCE>(&m_hucPakIntDummyBuffer);  // Region 4 - Not used for VP9
    params.regionParams[5].presRegion              = const_cast<PMOS_RESOURCE>(&m_hucPakIntDummyBuffer);  // Region 5 - Not used for VP9
    params.regionParams[5].isWritable              = true;
    params.regionParams[6].presRegion              = const_cast<PMOS_RESOURCE>(&m_hucPakIntDummyBuffer);  // Region 6 - Not used for VP9
    params.regionParams[6].isWritable              = true;
    params.regionParams[7].presRegion              = const_cast<PMOS_RESOURCE>(&m_hucPakIntDummyBuffer);  // Region 7 - Not used for VP9

    if (m_basicFeature->m_enableTileStitchByHW)
    {
        params.regionParams[8].presRegion               = const_cast<PMOS_RESOURCE>(&m_resHucStitchDataBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]);  // Region 8 - data buffer read by HUC for stitching cmd generation
        params.regionParams[8].isWritable               = true;
        params.regionParams[10].presRegion              = const_cast<PMOS_RESOURCE>(&m_HucStitchCmdBatchBuffer.OsResource);  // Region 10 - SLB for stitching cmd output from Huc
        params.regionParams[10].isWritable              = true;
    }

    params.regionParams[9].presRegion              = m_basicFeature->m_hucPakIntBrcDataBuffer;  // Region 9 - HuC outputs BRC data
    params.regionParams[9].isWritable = true;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
