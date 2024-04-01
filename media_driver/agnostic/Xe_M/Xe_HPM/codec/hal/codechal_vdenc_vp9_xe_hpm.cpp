/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/


//!
//! \file     codechal_vdenc_vp9_xe_hpm.cpp
//! \brief    This file implements the C++ class/interface for Xe_XPM platform's VP9
//!           VDEnc encoding to be used CODECHAL components.
//!

#include "codechal_vdenc_vp9_xe_hpm.h"
#include "codechal_vdenc_vp9_g12.h"
#include "mhw_vdbox_hcp_xe_xpm.h"
#include "mhw_vdbox_vdenc_xe_hpm.h"
#include "mhw_mi_g12_X.h"
#include "mos_solo_generic.h"
#include "codechal_hw_g12_X.h"

MOS_STATUS CodechalVdencVp9StateXe_Xpm::SetTileCommands(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12 vdencWalkerStateParams;
    vdencWalkerStateParams.Mode = CODECHAL_ENCODE_MODE_VP9;
    vdencWalkerStateParams.pVp9EncPicParams = m_vp9PicParams;
    vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_SINGLE_PIPE;

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;
    MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
    // MFXPipeDone should not be set for tail insertion
    vdPipelineFlushParams.Flags.bWaitDoneMFX =
        (m_lastPicInStream || m_lastPicInSeq) ? 0 : 1;
    vdPipelineFlushParams.Flags.bWaitDoneVDENC = 1;
    vdPipelineFlushParams.Flags.bFlushVDENC = 1;
    vdPipelineFlushParams.Flags.bFlushHEVC = 1;
    vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;

    if (IsFirstPipe() && IsFirstPass())
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetTileData());
    }

    MHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS vdencWeightOffsetParams;
    uint32_t                             numTileColumns = (1 << m_vp9PicParams->log2_tile_columns);
    uint32_t                             numTileRows    = (1 << m_vp9PicParams->log2_tile_rows);
    int                                  currentPipe    = GetCurrentPipe();
    for (uint32_t tileRow = 0, tileIdx = 0; tileRow < numTileRows; tileRow++)
    {
        for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++, tileIdx++)
        {
            if (m_numPipe > 1)
            {
                if (tileCol != currentPipe)
                {
                    continue;
                }
            }

            if (m_scalableMode)
            {
                MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
                //in scalability mode
                MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
                vdCtrlParam.scalableModePipeLock  = true;
                MhwMiInterfaceG12 *miInterfaceG12 = static_cast<MhwMiInterfaceG12 *>(m_miInterface);
                CODECHAL_ENCODE_CHK_STATUS_RETURN((miInterfaceG12)->AddMiVdControlStateCmd(cmdBuffer, &vdCtrlParam));
            }

            // HCP_TILE_CODING commmand
            CODECHAL_ENCODE_CHK_STATUS_RETURN(static_cast<MhwVdboxHcpInterfaceG12 *>(m_hcpInterface)->AddHcpTileCodingCmd(cmdBuffer, &m_tileParams[tileIdx]));

            MOS_ZeroMemory(&vdencWeightOffsetParams, sizeof(vdencWeightOffsetParams));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWeightsOffsetsStateCmd(cmdBuffer, nullptr, &vdencWeightOffsetParams));

            vdencWalkerStateParams.pTileCodingParams = &m_tileParams[tileIdx];
            vdencWalkerStateParams.dwTileId          = tileIdx;
            switch (m_numPipe)
            {
            case 0:
            case 1:
                vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_SINGLE_PIPE;
                break;
            case 2:
                vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_TWO_PIPE;
                break;
            case 4:
                vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_FOUR_PIPE;
                break;
            default:
                vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_INVALID;
                CODECHAL_ENCODE_ASSERTMESSAGE("Num Pipes invalid");
                return eStatus;
                break;
            }

            auto vdencInterface = static_cast<MhwVdboxVdencInterfaceXe_Hpm *>(m_vdencInterface);
            CODECHAL_ENCODE_CHK_NULL_RETURN(vdencInterface);

            CODECHAL_ENCODE_CHK_STATUS_RETURN(vdencInterface->AddVdencVp9TileSliceStateCmd(cmdBuffer, &vdencWalkerStateParams));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWalkerStateCmd(cmdBuffer, &vdencWalkerStateParams));

            if (m_scalableMode)
            {
                MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
                MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
                vdCtrlParam.scalableModePipeUnlock = true;
                MhwMiInterfaceG12 *miInterfaceG12  = static_cast<MhwMiInterfaceG12 *>(m_miInterface);
                CODECHAL_ENCODE_CHK_STATUS_RETURN((miInterfaceG12)->AddMiVdControlStateCmd(cmdBuffer, &vdCtrlParam));
            }

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(cmdBuffer, &vdPipelineFlushParams));

            // Send MI_FLUSH command
            MHW_MI_FLUSH_DW_PARAMS flushDwParams;
            MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
            flushDwParams.bVideoPipelineCacheInvalidate = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::SetSequenceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9StateG12::SetSequenceStructs());

    switch (m_vp9SeqParams->TargetUsage)
    {
    case 1: case 2:
        m_vp9SeqParams->TargetUsage = 2;
        break;
    case 3: case 4: case 5:
        m_vp9SeqParams->TargetUsage = 4;
        break;
    case 6: case 7:
        m_vp9SeqParams->TargetUsage = 7;
        break;
    default:
        m_vp9SeqParams->TargetUsage = 4;
        break;
    }

    m_targetUsage = (uint32_t)m_vp9SeqParams->TargetUsage;

    // Get row store cache offset as all the needed information is got here

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::SetDmemHuCPakInt()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t currPass = (uint8_t)GetCurrentPass();

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    // All bytes in below dmem for fields not used by VP9 to be set to 0xFF.
    HucPakIntDmemXehp *dmem = (HucPakIntDmemXehp *)m_osInterface->pfnLockResource(
        m_osInterface, &m_hucPakIntDmemBuffer[m_currRecycledBufIdx][currPass], &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dmem);

    MOS_ZeroMemory(dmem, sizeof(HucPakIntDmemXehp));
    // CODECHAL_VDENC_VP9_PAK_INT_DMEM_OFFSETS_SIZE size of offsets in the CODECHAL_VDENC_VP9_HUC_PAK_INT_DMEM struct.
    // Reset offsets to 0xFFFFFFFF as unavailable
    memset(dmem, 0xFF, m_pakIntDmemOffsetsSize);

    dmem->totalSizeInCommandBuffer = GetNumTilesInFrame() * CODECHAL_CACHELINE_SIZE;
    dmem->offsetInCommandBuffer    = 0xFFFF;  // Not used for VP9, all bytes in dmem for fields not used are 0xFF
    dmem->picWidthInPixel          = (uint16_t)m_frameWidth;
    dmem->picHeightInPixel         = (uint16_t)m_frameHeight;
    dmem->totalNumberOfPaks        = m_numPipe;
    dmem->codec                    = m_pakIntVp9CodecId;
    dmem->maxPass                  = m_brcMaxNumPasses;  // Only VDEnc CQP and BRC
    dmem->currentPass              = currPass + 1;
    dmem->lastTileBSStartInBytes   = m_tileParams[GetNumTilesInFrame() - 1].TileSizeStreamoutOffset * CODECHAL_CACHELINE_SIZE + 8;
    dmem->picStateStartInBytes     = 0xFFFF;

    dmem->StitchEnable        = true;
    dmem->StitchCommandOffset = 0;
    dmem->BBEndforStitch      = HUC_BATCH_BUFFER_END;

    // Offset 0 is for region 1 - output of integrated frame stats from PAK integration kernel

    dmem->tileSizeRecordOffset[0]   = m_frameStatsOffset.tileSizeRecord;
    dmem->vdencStatOffset[0]        = m_frameStatsOffset.vdencStats;
    dmem->vp9PakStatOffset[0]       = m_frameStatsOffset.pakStats;
    dmem->vp9CounterBufferOffset[0] = m_frameStatsOffset.counterBuffer;

    //Offset 1 - 4 is for region 0 - Input to PAK integration kernel for all tile statistics per pipe
    for (auto i = 1; i <= m_numPipe; i++)
    {
        dmem->numTilesPerPipe[i - 1]    = (GetNumTilesInFrame()) / m_numPipe;
        dmem->tileSizeRecordOffset[i]   = m_tileStatsOffset.tileSizeRecord + ((i - 1) * (dmem->numTilesPerPipe[i - 1]) * m_statsSize.tileSizeRecord);
        dmem->vdencStatOffset[i]        = m_tileStatsOffset.vdencStats + ((i - 1) * (dmem->numTilesPerPipe[i - 1]) * m_statsSize.vdencStats);
        dmem->vp9PakStatOffset[i]       = m_tileStatsOffset.pakStats + ((i - 1) * (dmem->numTilesPerPipe[i - 1]) * m_statsSize.pakStats);
        dmem->vp9CounterBufferOffset[i] = m_tileStatsOffset.counterBuffer + ((i - 1) * (dmem->numTilesPerPipe[i - 1]) * m_statsSize.counterBuffer);
    }
    m_osInterface->pfnUnlockResource(m_osInterface, &m_hucPakIntDmemBuffer[m_currRecycledBufIdx][currPass]);

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::HuCVp9PakInt(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!IsFirstPipe())
    {
        return eStatus;
    }

    CODECHAL_DEBUG_TOOL(
        uint32_t    hucRegionSize[16] = {0};
        const char *hucRegionName[16] = {"\0"};

        hucRegionName[0]  = "_MultiPakStreamout_input";
        hucRegionSize[0]  = m_tileStatsPakIntegrationBufferSize;
        hucRegionName[1]  = "_IntegratedStreamout_output";
        hucRegionSize[1]  = m_frameStatsPakIntegrationBufferSize;
        hucRegionName[4]  = "_BitStream_input";
        hucRegionSize[4]  = MOS_ALIGN_CEIL(m_bitstreamUpperBound, CODECHAL_PAGE_SIZE);
        hucRegionName[5]  = "_BitStream_output";
        hucRegionSize[5]  = MOS_ALIGN_CEIL(m_bitstreamUpperBound, CODECHAL_PAGE_SIZE);
        hucRegionName[6]  = "_HistoryBufferOutput";
        hucRegionSize[6]  = MOS_ALIGN_CEIL(64, CODECHAL_PAGE_SIZE);
        hucRegionName[7]  = "_HCPPICSTATEInputDummy";
        hucRegionSize[7]  = MOS_ALIGN_CEIL(64, CODECHAL_PAGE_SIZE);
        hucRegionName[8]  = "_HCPPICSTATEOutputDummy";
        hucRegionSize[8]  = MOS_ALIGN_CEIL(64, CODECHAL_PAGE_SIZE);
        hucRegionName[9]  = "_BrcDataOutputBuffer";  // This is the pak MMIO region 7 , not 4, of BRC update
        hucRegionSize[9]  = MOS_ALIGN_CEIL(CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE, CODECHAL_PAGE_SIZE);
        hucRegionName[15] = "_TileRecordBuffer";
        hucRegionSize[15] = m_maxTileNumber * MOS_ALIGN_CEIL(m_hcpInterface->GetPakHWTileSizeRecordSize(), CODECHAL_CACHELINE_SIZE);)

    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_vdboxHucPakIntegrationKernelDescriptor;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCPakInt());

    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_hucPakIntDmemBuffer[m_currRecycledBufIdx][GetCurrentPass()];
    dmemParams.dwDataLength      = MOS_ALIGN_CEIL(sizeof(HucPakIntDmemXehp), CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset      = HUC_DMEM_OFFSET_RTOS_GEMS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(cmdBuffer, &dmemParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ConfigStitchDataBuffer());

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    MOS_ZeroMemory(&virtualAddrParams, sizeof(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS));
    virtualAddrParams.regionParams[0].presRegion  = &m_tileStatsPakIntegrationBuffer[m_virtualEngineBBIndex].sResource;  // Region 0 - Tile based input statistics from PAK/ VDEnc
    virtualAddrParams.regionParams[0].dwOffset    = 0;
    virtualAddrParams.regionParams[1].presRegion  = &m_frameStatsPakIntegrationBuffer.sResource;  // Region 1 - HuC Frame statistics output
    virtualAddrParams.regionParams[1].isWritable  = true;
    virtualAddrParams.regionParams[4].presRegion  = &m_hucPakIntDummyBuffer;  // Region 4 - Not used for VP9
    virtualAddrParams.regionParams[5].presRegion  = &m_hucPakIntDummyBuffer;  // Region 5 - Not used for VP9
    virtualAddrParams.regionParams[5].isWritable  = true;
    virtualAddrParams.regionParams[6].presRegion  = &m_hucPakIntDummyBuffer;  // Region 6 - Not used for VP9
    virtualAddrParams.regionParams[6].isWritable  = true;
    virtualAddrParams.regionParams[7].presRegion  = &m_hucPakIntDummyBuffer;                                            // Region 7 - Not used for VP9
    virtualAddrParams.regionParams[8].presRegion  = &m_resHucStitchDataBuffer[m_currRecycledBufIdx][GetCurrentPass()];  // Region 8 - data buffer read by HUC for stitching cmd generation
    virtualAddrParams.regionParams[8].isWritable  = true;
    virtualAddrParams.regionParams[9].presRegion  = &m_hucPakIntBrcDataBuffer;  // Region 9 - HuC outputs BRC data
    virtualAddrParams.regionParams[9].isWritable  = true;
    virtualAddrParams.regionParams[10].presRegion = &m_HucStitchCmdBatchBuffer.OsResource;  // Region 10 - SLB for stitching cmd output from Huc
    virtualAddrParams.regionParams[10].isWritable = true;
    virtualAddrParams.regionParams[15].presRegion = &m_tileRecordBuffer[m_virtualEngineBBIndex].sResource;  // Region 15 [In/Out] - Tile Record Buffer
    virtualAddrParams.regionParams[15].dwOffset   = 0;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(cmdBuffer, &virtualAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreHuCStatus2Report(cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC    = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9State::AllocateResources());

    // create the tile coding state parameters
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_tileParams =
                                        (PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12)MOS_AllocAndZeroMemory(sizeof(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12) * m_maxTileNumber));

    if (m_isTilingSupported)
    {
        // VDENC tile row store buffer
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        allocParamsForBufferLinear.dwBytes  = MOS_ROUNDUP_DIVIDE(m_frameWidth, 32) * CODECHAL_CACHELINE_SIZE * 2;
        allocParamsForBufferLinear.pBufName = "VDENC Tile Row Store Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                      m_osInterface,
                                                      &allocParamsForBufferLinear,
                                                      &m_vdencTileRowStoreBuffer),
            "Failed to allocate VDENC Tile Row Store Buffer");

        uint32_t maxPicWidthInSb  = MOS_ROUNDUP_DIVIDE(m_maxPicWidth, CODEC_VP9_SUPER_BLOCK_WIDTH);
        uint32_t maxPicHeightInSb = MOS_ROUNDUP_DIVIDE(m_maxPicHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);

        //PAK CU Level Streamout Data:   DW57-59 in HCP pipe buffer address command
        uint32_t size = maxPicWidthInSb * maxPicHeightInSb * 64 * CODECHAL_CACHELINE_SIZE;  // One CU has 16-byte, and there are 64 CU in one SB. But, each tile needs to be aliged to the cache line
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.dwBytes  = size;
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        allocParamsForBufferLinear.pBufName = "PAK CU Level Streamout Data";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resPakcuLevelStreamoutData.sResource);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);

        //PAK Slice Level Streamut Data. DW60-DW62 in HCP pipe buffer address command
        // one LCU has one cache line. Use CU as LCU during creation
        allocParamsForBufferLinear.dwBytes  = size;
        allocParamsForBufferLinear.pBufName = "PAK Slice Level Streamout Data";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resPakSliceLevelStreamutData.sResource);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);

        //HCP scalability Sync buffer
        size                                = CODECHAL_ENCODE_VP9_MAX_NUM_HCP_PIPE * CODECHAL_CACHELINE_SIZE;
        allocParamsForBufferLinear.dwBytes  = size;
        allocParamsForBufferLinear.pBufName = "Hcp scalability Sync buffer ";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_hcpScalabilitySyncBuffer.sResource);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
        m_hcpScalabilitySyncBuffer.dwSize = size;

        //HCP Tile Size Streamout Buffer. Use in HCP_IND_OBJ_CMD
        size                                = m_maxTileNumber * MOS_ALIGN_CEIL(m_hcpInterface->GetPakHWTileSizeRecordSize(), CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.dwBytes  = size;
        allocParamsForBufferLinear.pBufName = "HCP Tile Record Buffer";

        if (m_scalableMode && m_hucEnabled)
        {
            //Sizes of each buffer to be loaded into the region 0 as input and 1 loaded out as output.

            MOS_ZeroMemory(&m_statsSize, sizeof(StatsInfo));
            m_statsSize.tileSizeRecord = m_hcpInterface->GetPakHWTileSizeRecordSize();
            m_statsSize.vdencStats     = m_brcStatsBufSize;     // VDEnc stats size
            m_statsSize.pakStats       = m_brcPakStatsBufSize;  // Frame stats size
            m_statsSize.counterBuffer  = m_probabilityCounterBufferSize;

            // HUC Pak Int DMEM buffer
            allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucPakIntDmemXehp), CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.pBufName = "Huc Pak Int Dmem Buffer";
            for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
            {
                for (auto j = 0; j < m_brcMaxNumPasses; j++)
                {
                    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
                        m_osInterface,
                        &allocParamsForBufferLinear,
                        &m_hucPakIntDmemBuffer[i][j]);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);
                }
            }

            // HuC PAK Int Region 1 programming related stats
            MOS_ZeroMemory(&m_frameStatsOffset, sizeof(StatsInfo));
            m_frameStatsOffset.tileSizeRecord = 0;
            m_frameStatsOffset.vdencStats     = MOS_ALIGN_CEIL((m_frameStatsOffset.tileSizeRecord + (m_maxTileNumber * m_statsSize.tileSizeRecord)), CODECHAL_PAGE_SIZE);
            m_frameStatsOffset.pakStats       = MOS_ALIGN_CEIL((m_frameStatsOffset.vdencStats + m_statsSize.vdencStats), CODECHAL_PAGE_SIZE);
            m_frameStatsOffset.counterBuffer  = MOS_ALIGN_CEIL((m_frameStatsOffset.pakStats + m_statsSize.pakStats), CODECHAL_PAGE_SIZE);

            // HuC PAK Int DMEM region 1 buffer allocation
            allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_frameStatsOffset.counterBuffer + m_statsSize.counterBuffer, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "PAK HUC Integrated Frame Stats Buffer";
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;

            m_frameStatsPakIntegrationBufferSize = allocParamsForBufferLinear.dwBytes;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_frameStatsPakIntegrationBuffer.sResource));
            m_frameStatsPakIntegrationBuffer.dwSize = allocParamsForBufferLinear.dwBytes;

            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;
            uint8_t *data       = nullptr;

            data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_frameStatsPakIntegrationBuffer.sResource, &lockFlags);
            MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_frameStatsPakIntegrationBuffer.sResource);

            // HuC PAK Int region 7, 8
            allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(64, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "HUC PAK Int Dummy Buffer";

            eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_hucPakIntDummyBuffer);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus);

            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;

            data = (uint8_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &m_hucPakIntDummyBuffer,
                &lockFlags);

            CODECHAL_ENCODE_CHK_NULL_RETURN(data);
            MOS_ZeroMemory(
                data,
                allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_hucPakIntDummyBuffer);

            // Allocate region 9 of pak integration to be fed as input to HUC BRC region 7
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "Xe_HPM PAK Integration FrameByteCount output";
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_hucPakIntBrcDataBuffer));

            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = 1;
            data                = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_hucPakIntBrcDataBuffer, &lockFlags);
            MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_hucPakIntBrcDataBuffer);

            // Allocate Semaphore memory for HUC to signal other pipe VDENC/PAK to continue
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.dwBytes  = sizeof(uint32_t);
            allocParamsForBufferLinear.pBufName = "Xe_HPM HUC done Semaphore Memory";

            for (auto i = 0; i < m_numPipe; i++)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_hucDoneSemaphoreMem[i].sResource));
                m_hucDoneSemaphoreMem[i].dwSize = allocParamsForBufferLinear.dwBytes;
            }

            // Allocate Semaphore memory for VDEnc/PAK on all pipes to signal stitch command to stop waiting
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.dwBytes  = sizeof(uint32_t);
            allocParamsForBufferLinear.pBufName = "Xe_HPM VDEnc PAK done Semaphore Memory";

            for (auto i = 0; i < m_numPipe; i++)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_stitchWaitSemaphoreMem[i].sResource));
                m_stitchWaitSemaphoreMem[i].dwSize = allocParamsForBufferLinear.dwBytes;
            }

            // Allocate semaphore memory for HUC HPU or BRC to wait on previous pass' PAK Integration command to finish
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.dwBytes  = sizeof(uint32_t);
            allocParamsForBufferLinear.pBufName = "Xe_HPM VDEnc PAK Int done Semaphore Memory";

            CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_pakIntDoneSemaphoreMem.sResource));
            m_pakIntDoneSemaphoreMem.dwSize = allocParamsForBufferLinear.dwBytes;
        }
    }

    if (m_enableTileStitchByHW)
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;

        for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            for (auto j = 0; j < CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES; j++)
            {
                // HuC stitching Data buffer
                allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucCommandData), CODECHAL_PAGE_SIZE);
                allocParamsForBufferLinear.pBufName = "VP9 HuC Stitch Data Buffer";
                CODECHAL_ENCODE_CHK_STATUS_RETURN(
                    m_osInterface->pfnAllocateResource(
                        m_osInterface,
                        &allocParamsForBufferLinear,
                        &m_resHucStitchDataBuffer[i][j]));
                MOS_LOCK_PARAMS lockFlagsWriteOnly;
                MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
                lockFlagsWriteOnly.WriteOnly = 1;
                uint8_t *pData               = (uint8_t *)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &m_resHucStitchDataBuffer[i][j],
                    &lockFlagsWriteOnly);
                CODECHAL_ENCODE_CHK_NULL_RETURN(pData);
                MOS_ZeroMemory(pData, allocParamsForBufferLinear.dwBytes);
                m_osInterface->pfnUnlockResource(m_osInterface, &m_resHucStitchDataBuffer[i][j]);
            }
        }
        //Second level BB for huc stitching cmd
        MOS_ZeroMemory(&m_HucStitchCmdBatchBuffer, sizeof(m_HucStitchCmdBatchBuffer));
        m_HucStitchCmdBatchBuffer.bSecondLevel = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &m_HucStitchCmdBatchBuffer,
            nullptr,
            m_hwInterface->m_HucStitchCmdBatchBufferSize));
    }

    uint32_t aligned_width  = MOS_ALIGN_CEIL(m_frameWidth, 64);
    uint32_t aligned_height = MOS_ALIGN_CEIL(m_frameHeight, 64);
    uint32_t num_lcu        = (aligned_width * aligned_height) / (64 * 64);

    MOS_ALLOC_GFXRES_PARAMS allocParamsForSurface;
    MOS_ZeroMemory(&allocParamsForSurface, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForSurface.Type     = MOS_GFXRES_BUFFER;
    allocParamsForSurface.TileType = MOS_TILE_LINEAR;
    allocParamsForSurface.Format   = Format_Buffer;
    allocParamsForSurface.dwBytes  = num_lcu * 4;
    allocParamsForSurface.pBufName = "VDEnc Cumulative CU Count Streamout Surface";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                  m_osInterface,
                                                  &allocParamsForSurface,
                                                  &m_vdencCumulativeCuCountStreamoutSurface),
        "Failed to allocate VDEnc Cumulative CU Count Streamout Surface");

    // HUC STATUS 2 Buffer for HuC status check in COND_BB_END
    allocParamsForSurface.dwBytes = sizeof(uint64_t);
    allocParamsForSurface.pBufName = "Huc authentication status Buffer";
    allocParamsForSurface.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForSurface,
        &m_hucAuthBuf);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Huc authentication status Buffer.");
        return eStatus;
    }

    for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
    {
        // second level batch buffer
        m_2ndLevelBB[j] = {};
        m_2ndLevelBB[j].bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_hwInterface->GetOsInterface(),
            &m_2ndLevelBB[j],
            nullptr,
            CODECHAL_CACHELINE_SIZE));
    }

    return eStatus;
}

CodechalVdencVp9StateXe_Xpm::~CodechalVdencVp9StateXe_Xpm()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(m_osInterface, &m_hucAuthBuf);

    for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
    {
        MOS_STATUS eStatus = Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_2ndLevelBB[j], nullptr);
        ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    }
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::Initialize(CodechalSetting * settings)
{
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9StateG12::Initialize(settings));

    // Disable HME temporarily
    m_16xMeSupported                = false;
    m_hmeSupported                  = false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::SetupSegmentationStreamIn()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_segmentMapProvided && !m_hmeEnabled)  // If we're not going to use the streamin surface leave now
    {
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    MOS_LOCK_PARAMS lockFlagsReadOnly;
    MOS_ZeroMemory(&lockFlagsReadOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsReadOnly.ReadOnly = 1;

    mhw_vdbox_vdenc_g12_X::VDENC_HEVC_VP9_STREAMIN_STATE_CMD *
        streamIn = (mhw_vdbox_vdenc_g12_X::VDENC_HEVC_VP9_STREAMIN_STATE_CMD *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
            &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(streamIn);

    // align to cache line size is OK since streamin state is padded to cacheline size - HW uses cacheline size to read, not command size
    uint32_t blockWidth   = MOS_ALIGN_CEIL(m_frameWidth, CODEC_VP9_SUPER_BLOCK_WIDTH) / 32;
    uint32_t blockHeight  = MOS_ALIGN_CEIL(m_frameHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT) / 32;
    uint32_t streamInSize = blockHeight * blockWidth * CODECHAL_CACHELINE_SIZE;
    MOS_ZeroMemory(streamIn, streamInSize);

    // If segment map isn't provided then we unlock surface and exit function here.
    // Reason why check isn't done before function call is to take advantage of the fact that
    // we need the surface locked here if seg map is provided and we want it 0'd either way.
    // This saves us from doing 2 locks on this buffer per frame.
    if (!m_segmentMapProvided)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &m_resVdencStreamInBuffer[m_currRecycledBufIdx]));
        return eStatus;
    }

    char *data = (char *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_mbSegmentMapSurface.OsResource,
        &lockFlagsReadOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    // Rasterization is done within a tile and then for each tile within the frame in raster order.
    if (m_isTilingSupported)
    {
        uint32_t numTileColumns          = (1 << m_vp9PicParams->log2_tile_columns);
        uint32_t numTileRows             = (1 << m_vp9PicParams->log2_tile_rows);
        uint32_t numTiles                = numTileColumns * numTileRows;
        uint32_t currTileStartX64Aligned = 0, dwCurrTileStartY64Aligned = 0;  //Set tile Y coordinate 0
        m_32BlocksRasterized = 0;                                             //Count of rasterized blocks for this frame
        uint32_t tileX       = 0;
        uint32_t tileY       = 0;
        for (uint32_t tileIdx = 0; tileIdx < numTiles; tileIdx++)
        {
            tileX = tileIdx % numTileColumns;  //Current tile column position
            tileY = tileIdx / numTileColumns;  //Current tile row position

            currTileStartX64Aligned   = ((tileX * m_picWidthInSb) >> m_vp9PicParams->log2_tile_columns) * CODEC_VP9_SUPER_BLOCK_WIDTH;
            dwCurrTileStartY64Aligned = ((tileY * m_picHeightInSb) >> m_vp9PicParams->log2_tile_rows) * CODEC_VP9_SUPER_BLOCK_HEIGHT;

            uint32_t tileWidth64Aligned = (((tileX == (numTileColumns - 1)) ? m_picWidthInSb : (((tileX + 1) * m_picWidthInSb) >> m_vp9PicParams->log2_tile_columns)) *
                                              CODEC_VP9_SUPER_BLOCK_WIDTH) -
                                          currTileStartX64Aligned;

            uint32_t tileHeight64Aligned = (((tileY == (numTileRows - 1)) ? m_picHeightInSb : (((tileY + 1) * m_picHeightInSb) >> m_vp9PicParams->log2_tile_rows)) *
                                               CODEC_VP9_SUPER_BLOCK_HEIGHT) -
                                           dwCurrTileStartY64Aligned;

            // last tile col raw width and raw height not necessarily 64 aligned, use this length to duplicate values from segmap for empty padding blocks in last tiles.
            uint32_t lastTileColWidth  = (tileX == (numTileColumns - 1)) ? (m_frameWidth - currTileStartX64Aligned) : tileWidth64Aligned;
            uint32_t lastTileRowHeight = (tileY == (numTileRows - 1)) ? (m_frameHeight - dwCurrTileStartY64Aligned) : tileHeight64Aligned;

            uint32_t tileWidth  = (tileX == (numTileColumns - 1)) ? lastTileColWidth : tileWidth64Aligned;
            uint32_t tileHeight = (tileY == (numTileRows - 1)) ? lastTileRowHeight : tileHeight64Aligned;

            // Recreate the mapbuffer and remap it if, for this frame, tile height and width have changed from previous tile
            // which was processed from this frame or previous,
            // or if map buffer is created for previous frame and tile map has changed from previous frame (numtilerows and cols)
            if (!m_mapBuffer ||
                tileHeight != m_segStreamInHeight ||
                tileWidth != m_segStreamInWidth ||
                numTileColumns != m_tileParams[tileIdx].NumOfTileColumnsInFrame ||
                m_tileParams[tileIdx].NumOfTilesInFrame != numTiles)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(InitZigZagToRasterLUTPerTile(tileHeight,
                    tileWidth,
                    dwCurrTileStartY64Aligned,
                    currTileStartX64Aligned));
            }
            m_tileParams[tileIdx].NumOfTileColumnsInFrame = numTileColumns;
            m_tileParams[tileIdx].NumOfTilesInFrame       = numTiles;
        }
    }

    uint32_t dwPitch = m_mbSegmentMapSurface.dwPitch;
    if (m_osInterface->pfnGetResType(&m_mbSegmentMapSurface.OsResource) == MOS_GFXRES_BUFFER)
    {
        //application can send 1D or 2D buffer, based on that change the pitch to correctly access the map buffer
        //driver reads the seg ids from the buffer for each 16x16 block. Reads 4 values for each 32x32 block
        dwPitch = MOS_ALIGN_CEIL(m_frameWidth, CODECHAL_MACROBLOCK_WIDTH) / CODECHAL_MACROBLOCK_WIDTH;
    }
    // set seg ID's of streamin states
    for (uint32_t i = 0; i < blockHeight * blockWidth; ++i)
    {
        uint32_t addrOffset = CalculateBufferOffset(
            m_mapBuffer[i],
            m_frameWidth,
            m_vp9PicParams->PicFlags.fields.seg_id_block_size,
            dwPitch);
        uint32_t segId                            = *(data + addrOffset);
        streamIn[i].DW7.SegidEnable               = 1;
        streamIn[i].DW7.Segid32X32016X1603Vp9Only = segId | (segId << 4) | (segId << 8) | (segId << 12);

        // TU functions copied from there.
        streamIn[i].DW0.Maxtusize = VDENC_XE_XPM_MAXTUSIZE_32x32;
        streamIn[i].DW0.Maxcusize = VDENC_XE_XPM_MAXCUSIZE_64x64;

        // For InterFrames we change the CUsize to 32x32 if we have sub 32 blocks with different segids in superblock
        if ((i % 4) == 3 && m_pictureCodingType == P_TYPE)
        {
            if (!(streamIn[i - 3].DW7.Segid32X32016X1603Vp9Only == streamIn[i - 2].DW7.Segid32X32016X1603Vp9Only &&
                    streamIn[i - 2].DW7.Segid32X32016X1603Vp9Only == streamIn[i - 1].DW7.Segid32X32016X1603Vp9Only &&
                    streamIn[i - 1].DW7.Segid32X32016X1603Vp9Only == streamIn[i].DW7.Segid32X32016X1603Vp9Only))
            {
                streamIn[i - 3].DW0.Maxcusize = streamIn[i - 2].DW0.Maxcusize = streamIn[i - 1].DW0.Maxcusize = streamIn[i].DW0.Maxcusize = VDENC_XE_XPM_MAXCUSIZE_32x32;
            }
        }

        switch (m_vp9SeqParams->TargetUsage)
        {
        case 1:  // Quality mode
        case 2:
            streamIn[i].DW6.Nummergecandidatecu64X64 = 3;
            streamIn[i].DW6.Nummergecandidatecu32X32 = 3;
            streamIn[i].DW6.Nummergecandidatecu16X16 = 3;
            streamIn[i].DW6.Nummergecandidatecu8X8   = 3;
            streamIn[i].DW0.Numimepredictors         = CODECHAL_VDENC_NUMIMEPREDICTORS_QUALITY;
            break;
        case 4:  // Normal mode
            streamIn[i].DW6.Nummergecandidatecu64X64 = 3;
            streamIn[i].DW6.Nummergecandidatecu32X32 = 3;
            streamIn[i].DW6.Nummergecandidatecu16X16 = 2;
            streamIn[i].DW6.Nummergecandidatecu8X8   = 2;
            streamIn[i].DW0.Numimepredictors         = CODECHAL_VDENC_NUMIMEPREDICTORS;
            break;
        case 7:  // Speed mode
            streamIn[i].DW6.Nummergecandidatecu64X64 = 2;
            streamIn[i].DW6.Nummergecandidatecu32X32 = 2;
            streamIn[i].DW6.Nummergecandidatecu16X16 = 1;
            streamIn[i].DW6.Nummergecandidatecu8X8   = 2;
            streamIn[i].DW0.Numimepredictors         = CODECHAL_VDENC_NUMIMEPREDICTORS_SPEED;
            break;
        default:
            MHW_ASSERTMESSAGE("Invalid TU provided!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        MEDIA_WA_TABLE *pWaTable = m_osInterface->pfnGetWaTable(m_osInterface);
        MHW_CHK_NULL_RETURN(pWaTable);

        if (MEDIA_IS_WA(pWaTable, Wa_22011549751) && m_vp9PicParams->PicFlags.fields.frame_type == 0 && !m_osInterface->bSimIsActive && !Mos_Solo_Extension(m_osInterface->pOsContext))
        {
            streamIn[i].DW6.Nummergecandidatecu8X8   = 2;
            streamIn[i].DW6.Nummergecandidatecu16X16 = 0;
            streamIn[i].DW6.Nummergecandidatecu32X32 = 0;
            streamIn[i].DW6.Nummergecandidatecu64X64 = 0;
            streamIn[i].DW0.Numimepredictors         = 0;
        }

    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_mbSegmentMapSurface.OsResource));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_resVdencStreamInBuffer[m_currRecycledBufIdx]));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::SetCurbeMe(
    MeCurbeParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    CODECHAL_ENCODE_ASSERT(params->TargetUsage <= NUM_TARGET_USAGE_MODES);

    uint32_t scaleFactor       = 0;
    bool     useMvFromPrevStep = false, writeDistortions = false;
    uint8_t  mvShiftFactor = 0, prevMvReadPosFactor = 0;
    bool     framePicture = CodecHal_PictureIsFrame(params->CurrOriginalPic);
    char     qpPrimeY     = (params->pic_init_qp_minus26 + 26) + params->slice_qp_delta;

    switch (params->hmeLvl)
    {
    case HME_LEVEL_32x:
        useMvFromPrevStep = m_hmeFirstStep;
        writeDistortions  = false;
        scaleFactor       = SCALE_FACTOR_32x;
        mvShiftFactor     = m_mvShiftFactor32x;
        break;
    case HME_LEVEL_16x:
        useMvFromPrevStep   = (params->b32xMeEnabled) ? m_hmeFollowingStep : m_hmeFirstStep;
        writeDistortions    = false;
        scaleFactor         = SCALE_FACTOR_16x;
        mvShiftFactor       = m_mvShiftFactor16x;
        prevMvReadPosFactor = m_prevMvReadPosition16x;
        break;
    case HME_LEVEL_4x:
        useMvFromPrevStep   = (params->b16xMeEnabled) ? m_hmeFollowingStep : m_hmeFirstStep;
        writeDistortions    = true;
        scaleFactor         = SCALE_FACTOR_4x;
        mvShiftFactor       = m_mvShiftFactor4x;
        prevMvReadPosFactor = m_prevMvReadPosition4x;
        break;
    default:
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MeCurbe cmd;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &cmd,
        sizeof(MeCurbe),
        meCurbeInit,
        sizeof(MeCurbe)));

    cmd.DW3.SubPelMode = 3;
    if (m_fieldScalingOutputInterleaved)
    {
        cmd.DW3.SrcAccess =
            cmd.DW3.RefAccess    = CodecHal_PictureIsField(params->CurrOriginalPic) ? 1 : 0;
        cmd.DW7.SrcFieldPolarity = CodecHal_PictureIsBottomField(params->CurrOriginalPic) ? 1 : 0;
    }

    cmd.DW4.PictureHeightMinus1 = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scaleFactor) - 1;
    cmd.DW4.PictureWidth        = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / scaleFactor);
    cmd.DW5.QpPrimeY            = qpPrimeY;
    cmd.DW6.WriteDistortions    = writeDistortions;
    cmd.DW6.UseMvFromPrevStep   = useMvFromPrevStep;

    cmd.DW6.SuperCombineDist = m_superCombineDistGeneric[params->TargetUsage];
    cmd.DW6.MaxVmvR          = (framePicture) ? params->MaxMvLen * 4 : (params->MaxMvLen >> 1) * 4;

    if (m_pictureCodingType == B_TYPE)
    {
        // This field is irrelevant since we are not using the bi-direct search.
        cmd.DW1.BiWeight             = 32;
        cmd.DW13.NumRefIdxL1MinusOne = params->num_ref_idx_l1_active_minus1;
    }

    if (m_pictureCodingType == P_TYPE ||
        m_pictureCodingType == B_TYPE)
    {
        if (params->hmeLvl == HME_LEVEL_4x && m_useNonLegacyStreamin)
        {
            cmd.DW30.ActualMBHeight = m_frameHeight;
            cmd.DW30.ActualMBWidth  = m_frameWidth;
        }
        else if (m_vdencEnabled && m_16xMeSupported)
        {
            cmd.DW30.ActualMBHeight = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight);
            cmd.DW30.ActualMBWidth  = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth);
        }
        cmd.DW13.NumRefIdxL0MinusOne =
            params->num_ref_idx_l0_active_minus1;
    }

    cmd.DW13.RefStreaminCost = 5;
    // This flag is to indicate the ROI source type instead of indicating ROI is enabled or not
    cmd.DW13.ROIEnable = 0;

    if (!framePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.DW14.List0RefID0FieldParity = params->List0RefID0FieldParity;
            cmd.DW14.List0RefID1FieldParity = params->List0RefID1FieldParity;
            cmd.DW14.List0RefID2FieldParity = params->List0RefID2FieldParity;
            cmd.DW14.List0RefID3FieldParity = params->List0RefID3FieldParity;
            cmd.DW14.List0RefID4FieldParity = params->List0RefID4FieldParity;
            cmd.DW14.List0RefID5FieldParity = params->List0RefID5FieldParity;
            cmd.DW14.List0RefID6FieldParity = params->List0RefID6FieldParity;
            cmd.DW14.List0RefID7FieldParity = params->List0RefID7FieldParity;
        }
        if (m_pictureCodingType == B_TYPE)
        {
            cmd.DW14.List1RefID0FieldParity = params->List1RefID0FieldParity;
            cmd.DW14.List1RefID1FieldParity = params->List1RefID1FieldParity;
        }
    }

    cmd.DW15.MvShiftFactor       = mvShiftFactor;
    cmd.DW15.PrevMvReadPosFactor = prevMvReadPosFactor;

    // r3 & r4
    uint8_t targetUsage = params->TargetUsage;
    uint8_t meMethod    = 0;
    if (m_pictureCodingType == B_TYPE)
    {
        meMethod = params->pBMEMethodTable ? params->pBMEMethodTable[targetUsage]
                                           : m_bMeMethodGeneric[targetUsage];
    }
    else
    {
        meMethod = params->pMEMethodTable ? params->pMEMethodTable[targetUsage]
                                          : m_meMethodGeneric[targetUsage];
    }

    uint8_t tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    eStatus          = MOS_SecureMemcpy(&(cmd.SPDelta), 14 * sizeof(uint32_t), m_encodeSearchPath[tableIdx][meMethod], 14 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // Non legacy stream in is for hevc vp9 streamin kernel
    if (params->hmeLvl == HME_LEVEL_4x && m_useNonLegacyStreamin)
    {
        //StreamIn CURBE
        cmd.DW6.LCUSize                    = 1;  //Only LCU64 supported by the VDEnc HW
        cmd.DW6.InputStreamInSurfaceEnable = params->segmapProvided;
        cmd.DW31.MaxCuSize                 = VDENC_XE_XPM_MAXCUSIZE_64x64;
        cmd.DW31.MaxTuSize                 = VDENC_XE_XPM_MAXTUSIZE_32x32;

        switch (m_vp9SeqParams->TargetUsage)
        {
        case 1:  // Quality mode
        case 2:
            cmd.DW36.NumMergeCandidateCu64x64 = 3;
            cmd.DW36.NumMergeCandidateCu32x32 = 3;
            cmd.DW36.NumMergeCandidateCu16x16 = 3;
            cmd.DW36.NumMergeCandidateCu8x8   = 3;
            cmd.DW31.NumImePredictors         = CODECHAL_VDENC_NUMIMEPREDICTORS_QUALITY;
            break;
        case 4:  // Normal mode
            cmd.DW36.NumMergeCandidateCu64x64 = 3;
            cmd.DW36.NumMergeCandidateCu32x32 = 3;
            cmd.DW36.NumMergeCandidateCu16x16 = 2;
            cmd.DW36.NumMergeCandidateCu8x8   = 2;
            cmd.DW31.NumImePredictors         = CODECHAL_VDENC_NUMIMEPREDICTORS;
            break;
        case 7:  // Speed mode
            cmd.DW36.NumMergeCandidateCu64x64 = 2;
            cmd.DW36.NumMergeCandidateCu32x32 = 2;
            cmd.DW36.NumMergeCandidateCu16x16 = 1;
            cmd.DW36.NumMergeCandidateCu8x8   = 2;
            cmd.DW31.NumImePredictors         = CODECHAL_VDENC_NUMIMEPREDICTORS_SPEED;
            break;
        default:
            MHW_ASSERTMESSAGE("Invalid TU provided!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        MEDIA_WA_TABLE *pWaTable = m_osInterface->pfnGetWaTable(m_osInterface);
        MHW_CHK_NULL_RETURN(pWaTable);

        if (MEDIA_IS_WA(pWaTable, Wa_22011549751) && m_vp9PicParams->PicFlags.fields.frame_type == 0 && !m_osInterface->bSimIsActive && !Mos_Solo_Extension(m_osInterface->pOsContext))
        {
            cmd.DW36.NumMergeCandidateCu8x8          = 2;
            cmd.DW36.NumMergeCandidateCu16x16        = 0;
            cmd.DW36.NumMergeCandidateCu32x32        = 0;
            cmd.DW36.NumMergeCandidateCu64x64        = 0;
            cmd.DW31.NumImePredictors                = 0;
        }
    }

    // r5
    cmd.DW40._4xMeMvOutputDataSurfIndex      = CODECHAL_ENCODE_ME_MV_DATA_SURFACE_G12;
    cmd.DW41._16xOr32xMeMvInputDataSurfIndex = (params->hmeLvl == HME_LEVEL_32x) ? CODECHAL_ENCODE_32xME_MV_DATA_SURFACE_G12 : CODECHAL_ENCODE_16xME_MV_DATA_SURFACE_G12;
    cmd.DW42._4xMeOutputDistSurfIndex        = CODECHAL_ENCODE_ME_DISTORTION_SURFACE_G12;
    cmd.DW43._4xMeOutputBrcDistSurfIndex     = CODECHAL_ENCODE_ME_BRC_DISTORTION_G12;
    cmd.DW44.VMEFwdInterPredictionSurfIndex  = CODECHAL_ENCODE_ME_CURR_FOR_FWD_REF_G12;
    cmd.DW45.VMEBwdInterPredictionSurfIndex  = CODECHAL_ENCODE_ME_CURR_FOR_BWD_REF_G12;
    cmd.DW46.VDEncStreamInOutputSurfIndex    = CODECHAL_ENCODE_ME_VDENC_STREAMIN_OUTPUT_G12;
    cmd.DW47.VDEncStreamInInputSurfIndex     = CODECHAL_ENCODE_ME_VDENC_STREAMIN_INPUT_G12;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::ConstructPicStateBatchBuf(
    PMOS_RESOURCE picStateBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(picStateBuffer);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported || m_firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        bool requestFrameTracking = false;
        if (!m_vp9PicParams->PicFlags.fields.super_frame)
        {
            requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : m_lastTaskInPhase;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
        m_firstTaskInPhase = false;
    }

    ReturnCommandBuffer(&cmdBuffer);

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, picStateBuffer, &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
    constructedCmdBuf.pCmdBase = (uint32_t*)data;
    constructedCmdBuf.pCmdPtr = (uint32_t*)data;
    constructedCmdBuf.iOffset = 0;
    constructedCmdBuf.iRemaining = m_vdencPicStateSecondLevelBatchBufferSize;

    eStatus = AddCommandsVp9(CODECHAL_CMD1, &constructedCmdBuf);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, picStateBuffer);
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to add CODECHAL_CMD1 command.");
        return eStatus;
    }

    // HCP_VP9_PIC_STATE
    MHW_VDBOX_VP9_ENCODE_PIC_STATE picState;
    MOS_ZeroMemory(&picState, sizeof(picState));
    picState.pVp9PicParams = m_vp9PicParams;
    picState.pVp9SeqParams = m_vp9SeqParams;
    picState.ppVp9RefList = &(m_refList[0]);
    picState.PrevFrameParams.fields.KeyFrame = m_prevFrameInfo.KeyFrame;
    picState.PrevFrameParams.fields.IntraOnly = m_prevFrameInfo.IntraOnly;
    picState.PrevFrameParams.fields.Display = m_prevFrameInfo.ShowFrame;
    picState.dwPrevFrmWidth = m_prevFrameInfo.FrameWidth;
    picState.dwPrevFrmHeight = m_prevFrameInfo.FrameHeight;
    picState.ucTxMode = m_txMode;
    picState.bSSEEnable = m_vdencBrcEnabled;
    picState.bUseDysRefSurface = (m_dysRefFrameFlags != DYS_REF_NONE) && m_dysVdencMultiPassEnabled;
    picState.bVdencPakOnlyPassFlag = m_vdencPakonlyMultipassEnabled;
    picState.uiMaxBitRate = m_vp9SeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;
    picState.uiMinBitRate = m_vp9SeqParams->MinBitRate * CODECHAL_ENCODE_BRC_KBPS;
    m_hucPicStateOffset = (uint16_t)constructedCmdBuf.iOffset;
    picState.bNonFirstPassFlag = !IsFirstPass();

    eStatus = m_hcpInterface->AddHcpVp9PicStateEncCmd(&constructedCmdBuf, nullptr, &picState);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, picStateBuffer);
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to add HCP_VP9_PIC_STATE command.");
        return eStatus;
    }

    // HCP_VP9_SEGMENT_STATE
    MHW_VDBOX_VP9_SEGMENT_STATE segmentState;
    MOS_ZeroMemory(&segmentState, sizeof(segmentState));
    segmentState.Mode = m_mode;
    segmentState.pVp9EncodeSegmentParams = m_vp9SegmentParams;
    uint8_t segmentCount = (m_vp9PicParams->PicFlags.fields.segmentation_enabled) ? CODEC_VP9_MAX_SEGMENTS : 1;

    for (uint8_t i = 0; i < segmentCount; i++)
    {
        segmentState.ucCurrentSegmentId = i;
        eStatus = m_hcpInterface->AddHcpVp9SegmentStateCmd(&constructedCmdBuf, nullptr, &segmentState);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, picStateBuffer);
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to add MHW_VDBOX_VP9_SEGMENT_STATE command.");
            return eStatus;
        }
    }

    // Adjust cmd buffer offset to have 8 segment state blocks
    if (segmentCount < CODEC_VP9_MAX_SEGMENTS)
    {
        // Max 7 segments, 32 bytes each
        uint8_t zeroBlock[m_segmentStateBlockSize * (CODEC_VP9_MAX_SEGMENTS - 1)];
        MOS_ZeroMemory(zeroBlock, sizeof(zeroBlock));
        Mhw_AddCommandCmdOrBB(m_osInterface, &constructedCmdBuf, nullptr, zeroBlock, (CODEC_VP9_MAX_SEGMENTS - segmentCount) * m_segmentStateBlockSize);
    }

    m_slbbImgStateOffset = (uint16_t)constructedCmdBuf.iOffset;
    eStatus = AddCommandsVp9(CODECHAL_CMD2, &constructedCmdBuf);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, picStateBuffer);
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to add CODECHAL_CMD2 command.");
        return eStatus;
    }

    // BB_END
    eStatus = m_miInterface->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, picStateBuffer);
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to add MI Batch Buffer End command.");
        return eStatus;
    }

    constructedCmdBuf.iOffset += 24;  // padding for alignment on 64
    constructedCmdBuf.iRemaining -= 24;
    if ((constructedCmdBuf.iOffset != m_vdencPicStateSecondLevelBatchBufferSize) &&
        (constructedCmdBuf.iRemaining < 0))
    {
        ENCODE_ASSERTMESSAGE("Failed to constructed Second Level Batch Buffer: No space for padding bytes.");
        return MOS_STATUS_NO_SPACE;
    }


    m_hucSlbbSize = (uint16_t)constructedCmdBuf.iOffset;

    m_osInterface->pfnUnlockResource(m_osInterface, picStateBuffer);

    return eStatus;
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::PackHucAuthCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Write HuC Load Info Mask
    MHW_MI_STORE_DATA_PARAMS storeDataParams{};
    storeDataParams.pOsResource         = &m_hucAuthBuf;
    storeDataParams.dwResourceOffset    = 0;
    storeDataParams.dwValue             = HUC_LOAD_INFO_REG_MASK_G12;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &storeDataParams));

    // Store Huc Auth register
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams{};
    storeRegParams.presStoreBuffer = &m_hucAuthBuf;
    storeRegParams.dwOffset        = sizeof(uint32_t);
    storeRegParams.dwRegister      = m_hucInterface->GetMmioRegisters(MHW_VDBOX_NODE_1)->hucLoadInfoOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &storeRegParams));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams{};
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Check Huc auth: if equals to 0 continue chained BB until reset, otherwise send BB end cmd.
    uint32_t compareOperation = mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADEQUALIDD;
    auto hwInterface = dynamic_cast<CodechalHwInterfaceG12 *>(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hwInterface);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(hwInterface->SendCondBbEndCmd(
        &m_hucAuthBuf, 0, 0, false, true, compareOperation, &cmdBuffer));

    // Chained BB loop
    auto miItf = static_cast<MhwMiInterfaceG12 *>(m_miInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(miItf);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miItf->AddMiBatchBufferStartCmd(&cmdBuffer, m_batchBuf, true));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::CheckHucLoadStatus()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_COMMAND_BUFFER cmdBuffer{};
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // add media reset check 100ms, which equals to 1080p WDT threshold
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->SetWatchdogTimerThreshold(1920, 1080, true));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(&cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStartCmd(&cmdBuffer));

    // program 2nd level chained BB for Huc auth
    m_batchBuf = &m_2ndLevelBB[m_currRecycledBufIdx];
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_batchBuf);

    MOS_LOCK_PARAMS lockFlags{};
    lockFlags.WriteOnly = true;

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &(m_batchBuf->OsResource), &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER hucAuthCmdBuffer;
    MOS_ZeroMemory(&hucAuthCmdBuffer, sizeof(hucAuthCmdBuffer));
    hucAuthCmdBuffer.pCmdBase   = (uint32_t *)data;
    hucAuthCmdBuffer.pCmdPtr    = hucAuthCmdBuffer.pCmdBase;
    hucAuthCmdBuffer.iRemaining = m_batchBuf->iSize;
    hucAuthCmdBuffer.OsResource = m_batchBuf->OsResource;
    hucAuthCmdBuffer.cmdBuf1stLvl = &cmdBuffer;

    //pak check huc status command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(PackHucAuthCmds(hucAuthCmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, &(m_batchBuf->OsResource)));

    // BB start for 2nd level BB
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, m_batchBuf));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencVp9StateXe_Xpm::HuCBrcInitReset()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (MEDIA_IS_WA(m_waTable, WaCheckHucAuthenticationStatus))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CheckHucLoadStatus());
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencVp9StateG12::HuCBrcInitReset());

    return MOS_STATUS_SUCCESS;
}