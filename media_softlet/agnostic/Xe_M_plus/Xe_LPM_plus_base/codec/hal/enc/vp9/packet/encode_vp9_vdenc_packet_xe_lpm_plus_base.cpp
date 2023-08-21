/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     encode_vp9_vdenc_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for vp9 encode vdenc packet
//!

#include "encode_vp9_vdenc_packet_xe_lpm_plus_base.h"
#include "mos_solo_generic.h"
#include "encode_vp9_huc_brc_update_packet.h"
#include "encode_vp9_brc.h"
#include "encode_vp9_pak.h"
#include "encode_vp9_tile.h"
#include "encode_vp9_hpu.h"

namespace encode
{
MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::Init()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Vp9VdencPkt::Init());

    // This flag enables pak only mode for Repak pass
    m_basicFeature->m_pakOnlyModeEnabledForLastPass = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::Prepare()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Vp9VdencPkt::Prepare());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::Destroy()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    return Vp9VdencPkt::Destroy();
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    MOS_COMMAND_BUFFER &cmdBuffer = *commandBuffer;

    if (m_pipeline->IsFirstPass() && m_pipeline->IsFirstPipe())
    {
        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PreProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));
    }

    // Ensure the input is ready to be read.
    // Currently, mos RegisterResource has sync limitation for Raw resource.
    // Temporaly, call Resource wait to do the sync explicitly.
    // TODO, refine it when MOS refactor ready.
    MOS_SYNC_PARAMS syncParams;
    syncParams                  = g_cInitSyncParams;
    syncParams.GpuContext       = m_osInterface->pfnGetGpuContext(m_osInterface);
    syncParams.presSyncResource = &m_basicFeature->m_rawSurface.OsResource;
    syncParams.bReadOnly        = true;
    ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
    m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

    ENCODE_CHK_STATUS_RETURN(PatchPictureLevelCommands(cmdBuffer, packetPhase));

    ENCODE_CHK_STATUS_RETURN(PatchTileLevelCommands(cmdBuffer, packetPhase));

    if (m_pipeline->IsLastPass() && m_pipeline->IsLastPipe())
    {
        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));
    }

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpInput());
    )

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AllocateResources()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Vp9VdencPkt::AllocateResources());

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    MOS_RESOURCE *allocatedBuffer = nullptr;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    // VDENC tile row store buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, 32) * CODECHAL_CACHELINE_SIZE * 2;
    allocParamsForBufferLinear.pBufName = "VDENC Tile Row Store Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_vdencTileRowStoreBuffer = *allocatedBuffer;

    uint32_t maxPicWidthInSb  = m_basicFeature->m_picWidthInSb;   // MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, CODEC_VP9_SUPER_BLOCK_WIDTH);
    uint32_t maxPicHeightInSb = m_basicFeature->m_picHeightInSb;  //MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);

    // PAK CU level streamout data: DW57-59 in HCP pipe buffer address command
    uint32_t size                       = maxPicWidthInSb * maxPicHeightInSb * 64 * CODECHAL_CACHELINE_SIZE;  // One CU has 16-byte, and there are 64 CU in one SB. But, each tile needs to be aligned to the cache line.
    allocParamsForBufferLinear.dwBytes  = size;
    allocParamsForBufferLinear.pBufName = "PAK CU Level Streamout Data";
    allocatedBuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_resPakcuLevelStreamoutData = *allocatedBuffer;

    uint32_t aligned_width  = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64);
    uint32_t aligned_height = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64);
    uint32_t num_lcu        = (aligned_width * aligned_height) / (64 * 64);

    MOS_ALLOC_GFXRES_PARAMS allocParamsForSurface;
    MOS_ZeroMemory(&allocParamsForSurface, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForSurface.Type     = MOS_GFXRES_BUFFER;
    allocParamsForSurface.TileType = MOS_TILE_LINEAR;
    allocParamsForSurface.Format   = Format_Buffer;
    allocParamsForSurface.dwBytes  = num_lcu * 4;
    allocParamsForSurface.pBufName = "VDenc Cumulative CU Count Streamout Surface";
    allocatedBuffer                = m_allocator->AllocateResource(allocParamsForSurface, true);
    ENCODE_CHK_NULL_RETURN(allocatedBuffer);
    m_vdencCumulativeCuCountStreamoutSurface = *allocatedBuffer;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddPictureHcpCommands(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(AddHcpPipeModeSelectCmd(cmdBuffer));

    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = false;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_SURFACE_STATE(&cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddHcpPipeBufAddrCmd(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddHcpIndObjBaseAddrCmd(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddHcpPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);
    SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpInterfaceNew, &cmdBuffer);
    SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    // Add vdenc pipe mode select command
    ENCODE_CHK_STATUS_RETURN(AddVdencPipeModeSelectCmd(cmdBuffer));
    // Set vdenc surfaces
    ENCODE_CHK_STATUS_RETURN(AddVdencSurfacesStateCmd(cmdBuffer));
    // Add vdenc pipeline buffer address command
    ENCODE_CHK_STATUS_RETURN(AddVdencPipeBufAddrCmd(cmdBuffer));

    // Add second level batch buffer command
    ENCODE_CHK_STATUS_RETURN(AddVdencSecondLevelBatchBufferCmd(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddVdencPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Vp9VdencPkt::AddVdencPipeModeSelectCmd(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::SetVdencPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &vdboxPipeModeSelectParams)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Vp9VdencPkt::SetVdencPipeModeSelectParams(vdboxPipeModeSelectParams));

    auto pipeModeSelectParams = dynamic_cast<MHW_VDBOX_PIPE_MODE_SELECT_PARAMS *>(&vdboxPipeModeSelectParams);
    ENCODE_CHK_NULL_RETURN(pipeModeSelectParams);

    auto dysRefFrameFlags         = m_basicFeature->m_ref.DysRefFrameFlags();
    auto dysVdencMultiPassEnabled = m_basicFeature->m_dysVdencMultiPassEnabled;

    pipeModeSelectParams->bDynamicScalingEnabled = (dysRefFrameFlags != DYS_REF_NONE) && !dysVdencMultiPassEnabled;

    if (m_basicFeature->m_scalableMode)
    {
        // Running in the multiple VDBOX mode
        if (m_pipeline->IsFirstPipe())
        {
            pipeModeSelectParams->MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;
        }
        else if (m_pipeline->IsLastPipe())
        {
            pipeModeSelectParams->MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;
        }
        else
        {
            pipeModeSelectParams->MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;
        }
        pipeModeSelectParams->PipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;
    }
    else
    {
        pipeModeSelectParams->MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
        pipeModeSelectParams->PipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::PatchPictureLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

    uint16_t perfTag = m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE : CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE_SECOND_PASS;
    SetPerfTag(perfTag, (uint16_t)m_basicFeature->m_mode, m_basicFeature->m_pictureCodingType);

    bool firstTaskInPhase = ((packetPhase & firstPacket) == firstPacket);

    if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
    {
        ENCODE_CHK_STATUS_RETURN(AddForceWakeup(cmdBuffer));
        // Send command buffer at the beginning (OS dependent)
        ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));
    }

    if (m_basicFeature->m_scalableMode)
    {
        auto scalability = m_pipeline->GetMediaScalability();
        if (m_pipeline->IsFirstPass())
        {
            // Reset multi-pipe sync semaphores
            ENCODE_CHK_STATUS_RETURN(scalability->ResetSemaphore(syncOnePipeWaitOthers, m_pipeline->GetCurrentPipe(), &cmdBuffer));
        }
        if (m_basicFeature->m_hucEnabled)
        {
            // Other pipes wait for brc update or hpu done on first pipe
            ENCODE_CHK_STATUS_RETURN(scalability->SyncPipe(syncOtherPipesForOne, 0, &cmdBuffer));
        }
    }

    auto dysRefFrameFlags         = m_basicFeature->m_ref.DysRefFrameFlags();
    auto dysVdencMultiPassEnabled = m_basicFeature->m_dysVdencMultiPassEnabled;

    if (dysRefFrameFlags != DYS_REF_NONE)
    {
        // Dynamic scaling implemented in separate class Vp9DynamicScalPktXe_Lpm_Plus
        if (m_pipeline->GetCurrentPass() == 1 && dysVdencMultiPassEnabled)
        {
            // Restore state informaiton here if need.
            m_basicFeature->m_ref.SetDysRefFrameFlags(DYS_REF_NONE);
        }
    }
    else
    {
        if (!m_pipeline->IsLastPass())
        {
            m_vdencPakObjCmdStreamOutEnabled   = true;
            m_resVdencPakObjCmdStreamOutBuffer = m_basicFeature->m_resMbCodeBuffer;
        }
        else
        {
            m_vdencPakObjCmdStreamOutEnabled = false;
        }

        if (!m_basicFeature->m_hucEnabled)
        {
            // Construct picture state 2nd level batch buffer
            RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, ConstructPicStateBatchBuffer, m_pipeline);
            // Refresh internal bufferes
            RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeHpu, Vp9FeatureIDs::vp9HpuFeature, RefreshFrameInternalBuffers);
        }
    }

    ENCODE_CHK_STATUS_RETURN(AddCondBBEndForLastPass(cmdBuffer));

    if (m_pipeline->IsFirstPipe())
    {
        ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));
    }

    // Add VDENC_CONTROL_STATE for VDEnc pipe initialization
    SETPAR_AND_ADDCMD(VDENC_CONTROL_STATE, m_vdencInterfaceNew, &cmdBuffer);
    // Add VD_CONTROL_STATE for HCP pipe initialization
    ENCODE_CHK_STATUS_RETURN(AddVdControlInitialize(cmdBuffer));

    // Add picture hcp commands
    ENCODE_CHK_STATUS_RETURN(AddPictureHcpCommands(cmdBuffer));

    auto &vdControlStateParams          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
    vdControlStateParams                = {};
    vdControlStateParams.vdencEnabled   = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

    // Add picture vdenc commands
    ENCODE_CHK_STATUS_RETURN(AddPictureVdencCommands(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddHcpTileCodingCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, SetHcpTileCodingParams, m_pipeline->GetPipeNum());

    SETPAR_AND_ADDCMD(HCP_TILE_CODING, m_hcpInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddVdencWalkerStateCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencInterfaceNew, &cmdBuffer);
    SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddVdencWeightOffsetsStateCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencInterfaceNew, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddOneTileCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t tileRow, uint32_t tileCol, uint32_t tileRowPass)
{
    ENCODE_FUNC_CALL();

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);
    if (m_basicFeature->m_scalableMode && (tileCol != m_pipeline->GetCurrentPipe()))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Begin patching tile level batch commands
    MOS_COMMAND_BUFFER constructTileBatchBuf = {};
    RUN_FEATURE_INTERFACE_RETURN(
        Vp9EncodeTile, Vp9FeatureIDs::encodeTile, BeginPatchTileLevelBatch, tileRowPass, constructTileBatchBuf);

    // Add batch buffer start for tile
    PMHW_BATCH_BUFFER tileLevelBatchBuffer = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(
        Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileLevelBatchBuffer, tileLevelBatchBuffer);
    ENCODE_CHK_NULL_RETURN(tileLevelBatchBuffer);

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, tileLevelBatchBuffer));

    // VP9 tile commands
    // Add lock scalable mode pipe
    if (m_basicFeature->m_scalableMode)
    {
        ENCODE_CHK_STATUS_RETURN(AddVdControlScalableModePipeLock(constructTileBatchBuf));
    }

    // Add hcp tile coding command
    ENCODE_CHK_STATUS_RETURN(AddHcpTileCodingCmd(constructTileBatchBuf));
    // Add vdenc weight offset state command
    ENCODE_CHK_STATUS_RETURN(AddVdencWeightOffsetsStateCmd(constructTileBatchBuf));
    // Add vdenc walker state commands
    ENCODE_CHK_STATUS_RETURN(AddVdencWalkerStateCmd(constructTileBatchBuf));

    // Add unlock scalable mode pipe
    if (m_basicFeature->m_scalableMode)
    {
        ENCODE_CHK_STATUS_RETURN(AddVdControlScalableModePipeUnlock(constructTileBatchBuf));
    }
    // Wait for vdenc command complete
    m_flushCmd = waitVdenc;
    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencInterfaceNew, &constructTileBatchBuf);
    // Ensure all commands executed
    ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(constructTileBatchBuf));

    // For 2nd level BB, we must use tileLevelBatchBuffer to prevent adding Epilogue before MI_BATCH_BUFFER_END
    tileLevelBatchBuffer->iCurrent   = constructTileBatchBuf.iOffset;
    tileLevelBatchBuffer->iRemaining = constructTileBatchBuf.iRemaining;
    ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, tileLevelBatchBuffer));

    CODECHAL_DEBUG_TOOL(
        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        std::string tileLevelBatchName = "_TLB_Pass";
        tileLevelBatchName += std::to_string((uint32_t)m_pipeline->GetCurrentPass());
        tileLevelBatchName += ("_" + std::to_string((uint32_t)m_pipeline->GetCurrentPipe()));
        tileLevelBatchName += ("_r" + std::to_string(tileRow) + "_c" + std::to_string(tileCol));

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpCmdBuffer(
            &constructTileBatchBuf,
            CODECHAL_NUM_MEDIA_STATES,
            tileLevelBatchName.c_str()));)

    // End patching tile level batch commands
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, EndPatchTileLevelBatch);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddOneTileCommandsNoTLBB(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t tileRow, uint32_t tileCol, uint32_t tileRowPass)
{
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);
    if (m_basicFeature->m_scalableMode && (tileCol != m_pipeline->GetCurrentPipe()))
    {
        return MOS_STATUS_SUCCESS;
    }

    // VP9 tile commands
    // Add lock scalable mode pipe
    if (m_basicFeature->m_scalableMode)
    {
        ENCODE_CHK_STATUS_RETURN(AddVdControlScalableModePipeLock(cmdBuffer));
    }

    // Add hcp tile coding command
    ENCODE_CHK_STATUS_RETURN(AddHcpTileCodingCmd(cmdBuffer));
    // Add vdenc weight offset state command
    ENCODE_CHK_STATUS_RETURN(AddVdencWeightOffsetsStateCmd(cmdBuffer));
    // Add vdenc walker state commands
    ENCODE_CHK_STATUS_RETURN(AddVdencWalkerStateCmd(cmdBuffer));

    // Add unlock scalable mode pipe
    if (m_basicFeature->m_scalableMode)
    {
        ENCODE_CHK_STATUS_RETURN(AddVdControlScalableModePipeUnlock(cmdBuffer));
    }

    // Wait for vdenc command complete
    m_flushCmd = waitVdenc;
    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencInterfaceNew, &cmdBuffer);
    // Ensure all commands executed
    ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    if (!m_basicFeature->m_hucEnabled)
    {
        // Construct pak insert batch buffer
        RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, ConstructPakInsertObjBatchBuffer);
    }

    if (m_pipeline->IsFirstPipe())
    {
        MHW_BATCH_BUFFER secondLevelBatchBuffer;
        MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(secondLevelBatchBuffer));
        RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, SetHucPakInsertObjBatchBuffer, secondLevelBatchBuffer);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, &secondLevelBatchBuffer));
    }

    // Setup Tile PAK commands
    uint16_t numTileColumns = 1;
    uint16_t numTileRows    = 1;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    for (uint32_t tileRow = 0; tileRow < numTileRows; ++tileRow)
    {
        uint32_t rowPass = m_pipeline->GetCurrentPass();
        for (uint32_t tileCol = 0; tileCol < numTileColumns; ++tileCol)
        {
            ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(cmdBuffer, tileRow, tileCol, rowPass));
        }
    }

    ENCODE_CHK_STATUS_RETURN(AddVdControlMemoryImplicitFlush(cmdBuffer));

    m_flushCmd = waitVp9;
    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencInterfaceNew, &cmdBuffer);

    ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

    // Wait all pipe cmds done for the packet
    auto scalability = m_pipeline->GetMediaScalability();
    ENCODE_CHK_STATUS_RETURN(scalability->SyncPipe(syncOnePipeWaitOthers, 0, &cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, &cmdBuffer));

    if (!m_basicFeature->m_scalableMode)
    {
        ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(m_vdboxIndex, m_statusReport, cmdBuffer));
    }

    // Send MI_FLUSH command
    auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams       = {};
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    if (m_osInterface->bInlineCodecStatusUpdate)
    {
        ENCODE_CHK_STATUS_RETURN(UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
    }

    CODECHAL_DEBUG_TOOL(
        if (m_mmcState) {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_reconSurface));
        })

    UpdateParameters();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddCondBBEndForLastPass(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    // Repak conditional batch buffer end based on repak flag written by Huc to HUC_STATUS regster
    if (m_basicFeature->m_hucEnabled && (m_pipeline->GetPassNum() > 0) && m_pipeline->IsLastPass())
    {
        ENCODE_CHK_NULL_RETURN(m_miItf);

        // Insert conditional batch buffer end
        // Bit 30 has been added as a success condition, therefore this needs to be masked to only check 31 for RePAK
        // or else if HuC decides not to do RePAK for conditional RePAK yet terminates successfully RePAK will still happen.
        // Success = bit 30 set to 1, Do RePAK = bit 31 set to 1, value is always 0; if 0 < memory, continue
        auto &miConditionalBatchBufferEndParams = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        miConditionalBatchBufferEndParams       = {};

        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        // Make the DisableCompareMask 0, so that the HW will do AND operation on DW0 with Mask DW1, refer to HuCVp9Prob() for the settings
        // and compare the result against the Semaphore data which in our case dwValue = 0.
        // If result > dwValue then continue execution otherwise terminate the batch buffer
        miConditionalBatchBufferEndParams.bDisableCompareMask = false;

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(&cmdBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddVdControlInitialize(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_miItf);

    auto &vdControlStateParams          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
    vdControlStateParams                = {};
    vdControlStateParams.initialization = true;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddVdControlMemoryImplicitFlush(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_miItf);

    auto &vdControlStateParams               = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
    vdControlStateParams                     = {};
    vdControlStateParams.memoryImplicitFlush = true;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddVdControlScalableModePipeLock(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_miItf);

    auto &vdControlStateParams                = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
    vdControlStateParams                      = {};
    vdControlStateParams.scalableModePipeLock = true;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::AddVdControlScalableModePipeUnlock(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_miItf);

    auto &vdControlStateParams                  = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
    vdControlStateParams                        = {};
    vdControlStateParams.scalableModePipeUnlock = true;

    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9VdencPktXe_Lpm_Plus_Base::UpdateParameters()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Vp9VdencPkt::UpdateParameters());

    // Reset parameters for next PAK execution
    if (m_pipeline->IsLastPipe() && m_pipeline->IsLastPass())
    {
        if (!m_pipeline->IsSingleTaskPhaseSupported())
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, Vp9VdencPktXe_Lpm_Plus_Base)
{
    ENCODE_FUNC_CALL();

    params.codecStandardSelect = CODEC_STANDARD_SELECT_VP9;
    params.codecSelect         = CODEC_SELECT_ENCODE;

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    params.bStreamOutEnabled      = brcFeature->IsVdencBrcEnabled();
    params.bVdencEnabled          = true;

    auto dysRefFrameFlags         = m_basicFeature->m_ref.DysRefFrameFlags();
    auto dysVdencMultiPassEnabled = m_basicFeature->m_dysVdencMultiPassEnabled;

    params.bDynamicScalingEnabled = (dysRefFrameFlags != DYS_REF_NONE) && !dysVdencMultiPassEnabled;

    params.multiEngineMode        = getMultiEngineMode();
    params.pipeWorkMode           = getPipeWorkMode();

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
