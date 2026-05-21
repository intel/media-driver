/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     encode_av1_huc_slbb_update_packet.cpp
//! \brief    Implements the AV1HucSLBBUpdatePkt class for AV1-specific HuC SLBB updates
//!

#include "encode_av1_huc_slbb_update_packet.h"
#include "encode_av1_vdenc_feature_defs.h"
#include "encode_av1_pipeline.h"
#include "encode_av1_brc.h"
#include "codec_def_common_encode.h"
#include "encode_av1_brc.h"
#include "encode_av1_tile.h"
#include "encode_av1_scc.h"
#include "encode_av1_basic_feature_xe3p_lpm_base.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug.h"
#endif

namespace encode
{

AV1HucSLBBUpdatePkt::AV1HucSLBBUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
    : HucSLBBUpdatePkt(pipeline, task, hwInterface)
{
    ENCODE_FUNC_CALL();
    
    // Initialize kernel descriptor for AV1 HuC SLBB Update
    m_vdboxHucKernelDescriptor = m_vdboxHucAv1SlbbUpdateKernelDescriptor;

    // Set DMEM buffer size to match HucAv1SlbbUpdateDmem (64 bytes, cache-line aligned)
    m_slbbUpdateDmemBufferSize = sizeof(HucAv1SlbbUpdateDmem);

    // Get feature manager from pipeline
    m_featureManager = m_pipeline->GetPacketLevelFeatureManager(Av1Pipeline::HucSLBBUpdate);
}

AV1HucSLBBUpdatePkt::~AV1HucSLBBUpdatePkt()
{
    ENCODE_FUNC_CALL();
}

MOS_STATUS AV1HucSLBBUpdatePkt::Init()
{
    ENCODE_FUNC_CALL();
    
    // Call base class Init
    ENCODE_CHK_STATUS_RETURN(HucSLBBUpdatePkt::Init());

    ENCODE_CHK_NULL_RETURN(m_pipeline);
    
    // Get allocator from pipeline
    m_allocator = m_pipeline->GetEncodeAllocator();
    ENCODE_CHK_NULL_RETURN(m_allocator);

    ENCODE_CHK_NULL_RETURN(m_featureManager);
    
    // Get AV1 basic feature from feature manager
    m_basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(commandBuffer);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1PicParams);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1SeqParams);

    // AV1HucSLBBUpdatePkt is first-task-in-phase: its prolog (SendPrologCmds) embeds
    // AddWatchdogTimerStartCmd into the CmdBuffer with the current watchdogCountThreshold.
    // The later main VDEnc pkt's SetWatchdogTimerThreshold updates the field but under
    // single-task-phase the main pkt skips its own prolog, so the already-submitted
    // watchdog register value is never refreshed. Set the encoder-appropriate threshold
    // HERE (based on frame size) so the prolog picks it up. Without this, large frames
    // (e.g. 8K) time out under the default 60ms threshold.
    ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(
        m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

    // Construct batch buffer for SLBB operations
    ENCODE_CHK_STATUS_RETURN(ConstructBatchBuffer());

    bool firstTaskInPhase = packetPhase & firstPacket;
    bool requestProlog = false;

    // Set performance tags for SLBB Update
    uint16_t perfTag = CODECHAL_ENCODE_PERFTAG_CALL_SLBB_UPDATE;
    uint16_t pictureType = (m_basicFeature->m_pictureCodingType == I_TYPE) ? 0 : 
        (m_basicFeature->m_ref.IsLowDelay() ? 
            (m_basicFeature->m_ref.IsPFrame() ? 1 : 3) : 2);
    SetPerfTag(perfTag, (uint16_t)m_basicFeature->m_mode, pictureType);

    if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        requestProlog = true;
    }

    // Execute HuC SLBB Update
    ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, SLBB_UPDATE));

    if (!IsHuCStsUpdNeeded())
    {
        // Write HUC_STATUS mask: DW1 (mask value)
        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        storeDataParams.dwResourceOffset = sizeof(uint32_t);
        storeDataParams.dwValue          = AV1_BRC_HUC_STATUS_REENCODE_MASK;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(commandBuffer));

        // store HUC_STATUS register: DW0 (actual value)
        ENCODE_CHK_COND_RETURN((m_vdboxIndex > MHW_VDBOX_NODE_1), "ERROR - vdbox index exceed the maximum");
        auto mmioRegisters              = m_hucItf->GetMmioRegisters(m_vdboxIndex);
        auto &storeRegParams            = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        storeRegParams                  = {};
        storeRegParams.presStoreBuffer  = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        storeRegParams.dwOffset         = 0;
        storeRegParams.dwRegister       = mmioRegisters->hucStatusRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(commandBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::SetDmem() const
{
    ENCODE_FUNC_CALL();

    // Null pointer checks
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1PicParams);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1SeqParams);
    ENCODE_CHK_NULL_RETURN(m_allocator);
    ENCODE_CHK_NULL_RETURN(m_featureManager);

    // Get Av1Brc feature
    auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    // Get basic feature to access SlbData (now stored in Av1BasicFeature for both BRC and CQP paths)
    auto basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    // Get SLBB data from basic feature
    auto slbData = basicFeature->GetSLBData();

    // Lock DMEM buffer resource for writing
    const uint32_t bufIdx = m_pipeline->m_currRecycledBufIdx;
    auto dmem = (HucAv1SlbbUpdateDmem *)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE*>(&m_slbbUpdateDmemBuffer[bufIdx]));
    ENCODE_CHK_NULL_RETURN(dmem);

    // Zero-initialize DMEM buffer
    MOS_ZeroMemory(dmem, sizeof(HucAv1SlbbUpdateDmem));

    // Populate Section 1 (SLBB Size)
    dmem->SlbbSize = slbData.slbSize;

    // Populate Section 2 (Command Offsets)
    dmem->AvpSegmentStateOffset = slbData.avpSegmentStateOffset;
    dmem->AvpInloopFilterStateOffset = slbData.avpInloopFilterStateOffset;
    dmem->VdencCmd1Offset = slbData.vdencCmd1Offset;
    dmem->VdencCmd2Offset = slbData.vdencCmd2Offset;
    dmem->AvpPicStateOffset = slbData.avpPicStateOffset;
    dmem->AvpPicStateMultiTileOffset = slbData.secondAvpPicStateOffset;
    dmem->VdencTileSliceStateOffset = slbData.vdencTileSliceStateOffset;
    dmem->AVPPiCStateCmdNum = slbData.avpPicStateCmdNum;
    dmem->TileNum = slbData.tileNum;

    // Populate Section 3 (Encoding Parameters)
    dmem->FrameType = (uint8_t)m_basicFeature->m_av1PicParams->PicFlags.fields.frame_type;
    dmem->QpValue = (uint8_t)m_basicFeature->m_av1PicParams->base_qindex;
    dmem->isLowDelay = m_basicFeature->m_ref.IsLowDelay() ? 1 : 0;
    dmem->TargetUsage = (uint8_t)m_basicFeature->m_av1SeqParams->TargetUsage;
#if (_DEBUG || _RELEASE_INTERNAL)
    {
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            "AV1 Encode RDO Enable",
            MediaUserSetting::Group::Sequence);
        dmem->RdoEnable = outValue.Get<bool>() ? 1 : 0;
    }
#else
    dmem->RdoEnable = 0;
#endif

    // Get reference frame counts
    uint8_t fwdRefNum = 0, bwdRefNum = 0;
    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref.GetFwdBwdRefNum(fwdRefNum, bwdRefNum));
    dmem->numRefL0 = fwdRefNum;
    dmem->numRefL1 = bwdRefNum;

    // IBCEnabledForCurrentTile: per-tile IBC status (allow_intrabc && wSB64 > INTRABC_DELAY_SB64+1).
    // Mirrors Av1Scc::VDENC_CMD2 which reads m_IBCEnabledForCurrentTile; ConstructBatchBuffer() (called
    // before SetDmem) already invoked UpdateIBCStatusForCurrentTile per tile, so this reflects the
    // last-processed tile's state -- matching the frame-level VDENC_CMD2 convention on the driver side.
    auto sccFeature = dynamic_cast<Av1Scc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1Scc));
    dmem->IBCEnabledForCurrentTile = (sccFeature && sccFeature->IsIBCEnabledForCurrentTile()) ? 1 : 0;

    // IsLossless: derived from pic params (base_qindex == 0 and no delta-q)
    dmem->IsLossless = IsFrameLossless(*m_basicFeature->m_av1PicParams) ? 1 : 0;

    // ChromaFormat: 0=YUV420 (standard), 1=YUV422, 2=YUV444
    if (m_basicFeature->m_chromaFormat == AVP_CHROMA_FORMAT_YUV422)
        dmem->ChromaFormat = 1;
    else if (m_basicFeature->m_chromaFormat == AVP_CHROMA_FORMAT_YUV444)
        dmem->ChromaFormat = 2;
    else
        dmem->ChromaFormat = 0;

    // EnableIntraEdgeFilter: from sequence-level CodingToolFlags
    dmem->EnableIntraEdgeFilter =
        m_basicFeature->m_av1SeqParams->CodingToolFlags.fields.enable_intra_edge_filter ? 1 : 0;

    dmem->Par65Inter = basicFeature->m_par65Inter;
    dmem->Par65Intra = basicFeature->m_par65Intra;

    dmem->EnablePalette = m_basicFeature->m_av1PicParams->PicFlags.fields.PaletteModeEnable ? 1 : 0;
    dmem->EnableIBC = m_basicFeature->m_av1PicParams->PicFlags.fields.allow_intrabc ? 1 : 0;

    auto osInterface = m_hwInterface->GetOsInterface();
    auto waTable = osInterface->pfnGetWaTable(osInterface);
    dmem->Wa_15014143531 = MEDIA_IS_WA(waTable, Wa_15014143531) ? 1 : 0;
    dmem->Wa_15017726119 = MEDIA_IS_WA(waTable, Wa_15017726119) ? 1 : 0;
    dmem->Wa_16025947269 = MEDIA_IS_WA(waTable, Wa_16025947269) ? 1 : 0;

    dmem->AdaptiveTUEnabled = m_basicFeature->m_av1PicParams->AdaptiveTUEnabled;

    // ExtendedPlatform: when set, the HuC kernel preserves driver-managed
    // VDENC_CMD2 conflict-surface fields (vendor-specific DW63 / DW64 bits).
    // Default base = 0 (base platforms — HuC owns these via TU tables). Extended
    // platform subclass overrides IsExtendedPlatform() to return true so the kernel
    // skips and the driver's L2/L5 lambda pre-fills survive into the SLBB.
    dmem->ExtendedPlatform = IsExtendedPlatform() ? 1 : 0;

    // Unlock resource
    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(const_cast<MOS_RESOURCE*>(&m_slbbUpdateDmemBuffer[bufIdx])));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    ENCODE_FUNC_CALL();

    auto osInterface = m_hwInterface->GetOsInterface();
    ENCODE_CHK_NULL_RETURN(osInterface);

    uint32_t hucCommandsSize = 0;
    uint32_t hucPatchListSize = 0;
    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

    ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
        m_basicFeature->m_mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, &stateCmdSizeParams));

    commandBufferSize = hucCommandsSize;
    requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

    if (m_pipeline->IsSingleTaskPhaseSupported())
    {
        commandBufferSize *= m_pipeline->GetPassNum();
    }

    // 4K align since allocation is in chunks of 4K bytes.
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

    // Add space for HUC_IMEM_ADDR command and patch list entry in PPGTT mode
    if (m_isPPGTT)
    {
        ENCODE_CHK_NULL_RETURN(m_itfPPGTT);
        commandBufferSize += m_itfPPGTT->MHW_GETSIZE_F(HUC_IMEM_ADDR)();
        requestedPatchListSize += PATCH_LIST_COMMAND(mhw::vdbox::huc::ItfPPGTT::HUC_IMEM_ADDR_CMD);
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::AllocateResources()
{
    ENCODE_FUNC_CALL();
    
    // Call base class AllocateResources
    ENCODE_CHK_STATUS_RETURN(HucSLBBUpdatePkt::AllocateResources());

    // Load kernel binary for PPGTT mode
    if (m_isPPGTT && m_kernelBinBuffer == nullptr)
    {
        ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
        HucKernelSource::HucBinary hucBinary{};
        ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelBin(HucKernelSource::av1SlbbUpdateKernelId, hucBinary));

        ENCODE_CHK_NULL_RETURN(hucBinary.m_data);

        // Initialize allocation parameters for kernel binary buffer
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type               = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType           = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format             = Format_Buffer;
        allocParamsForBufferLinear.dwBytes            = MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName           = "Av1SlbbUpdateKernelBinBuffer";
        allocParamsForBufferLinear.ResUsageType       = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        allocParamsForBufferLinear.Flags.bNotLockable = false;  // Resource can be CPU accessed

        m_kernelBinBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        auto data         = (uint16_t *)m_allocator->LockResourceForWrite(m_kernelBinBuffer);
        ENCODE_CHK_NULL_RETURN(data);
        MOS_SecureMemcpy(data, hucBinary.m_size, hucBinary.m_data, hucBinary.m_size);
        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_kernelBinBuffer));
    }

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    MOS_RESOURCE *allocatedbuffer = nullptr;

    for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        // Const Data buffer for SLBB operations
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_slbbConstDataBufferSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "AV1 HuC SLBB Const Data Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
        allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_slbbConstDataBuffer[k] = *allocatedbuffer;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::ConstructBatchBuffer()
{
    ENCODE_FUNC_CALL();
    
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_pipeline);
    
    // Cast to xe3p_lpm_base basic feature to access GetVdencReadBatchBufferOrigin
    auto basicFeatureXe3pLpm = dynamic_cast<Av1BasicFeatureXe3P_Lpm_Base *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeatureXe3pLpm);
    
    // Get current recycled buffer index
    const uint32_t recycledBufIdx = m_pipeline->m_currRecycledBufIdx;
    
    // CRITICAL: Retrieve SLBB buffer using HARDCODED brcPass=0
    // SLBB update operates on first pass only, as evidenced by pipeline activation patterns
    // where all three codec pipelines (AV1, AVC, HEVC) activate SLBB update with pass=0
    // BEFORE BRC iterations begin
    MOS_RESOURCE *batchBuffer = basicFeatureXe3pLpm->GetVdencReadBatchBufferOrigin(recycledBufIdx, 0);
    ENCODE_CHK_NULL_RETURN(batchBuffer);
    
    // Lock buffer for writing
    auto batchbufferAddr = (uint8_t *)m_allocator->LockResourceForWrite(batchBuffer);
    ENCODE_CHK_NULL_RETURN(batchbufferAddr);
    
    // Initialize command buffer structure
    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
    constructedCmdBuf.pCmdBase = constructedCmdBuf.pCmdPtr = (uint32_t *)batchbufferAddr;
    constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_basicFeature->GetSlbbBufferSize(), CODECHAL_PAGE_SIZE);
    
    /*--------------SLBB layout--------------------*/
    /*----Group1----*/
    //AVP_SEGMENT_STATE
    //AVP_INLOOP_FILTER_STATE
    //BB_END
    //
    /*----Group2----*/
    //VDENC_CMD1
    //BB_END
    //
    /*----Group3----*/
    //VDENC_CMD2
    //BB_END
    //
    /*----Group4----*/
    //AVP_PIC_STATE
    //BB_END
    //AVP_PIC_STATE (if multi tile)
    //BB_END (if multi tile)
    //
    /*----Group5----*/
    //VDENC_HEVC_VP9_TILE_SLICE_STATE (if palette mode enabled)
    //BB_END (if palette mode enabled)
    /*--------------End of SLBB layout--------------------*/
    
    SlbData slbData;
    
    /*----Group1----*/
    slbData.avpSegmentStateOffset = (uint16_t)constructedCmdBuf.iOffset;
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SEGMENT_STATE(&constructedCmdBuf));
    slbData.avpInloopFilterStateOffset = (uint16_t)constructedCmdBuf.iOffset;
    SETPAR_AND_ADDCMD(AVP_INLOOP_FILTER_STATE, m_avpItf, &constructedCmdBuf);
    ENCODE_CHK_STATUS_RETURN(AddBBEnd(constructedCmdBuf));
    
    /*----Group2----*/
    slbData.vdencCmd1Offset = (uint16_t)constructedCmdBuf.iOffset;
    SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, &constructedCmdBuf);
    ENCODE_CHK_STATUS_RETURN(AddBBEnd(constructedCmdBuf));
    
    /*----Group3----*/
    slbData.vdencCmd2Offset = (uint16_t)constructedCmdBuf.iOffset;
    SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, &constructedCmdBuf);
    ENCODE_CHK_STATUS_RETURN(AddBBEnd(constructedCmdBuf));
    
    /*----Group4----*/
    slbData.avpPicStateOffset = (uint16_t)constructedCmdBuf.iOffset;
    ENCODE_CHK_STATUS_RETURN(AddAvpPicStateBaseOnTile(constructedCmdBuf, slbData));
    
    /*----Group5----*/
    slbData.vdencTileSliceStateOffset = (uint16_t)constructedCmdBuf.iOffset;
    ENCODE_CHK_STATUS_RETURN(AddVdencTileSliceBaseOnTile(constructedCmdBuf, slbData));
    
    // Calculate total SLBB size
    slbData.slbSize = (uint16_t)constructedCmdBuf.iOffset - slbData.avpSegmentStateOffset;
    
    // Store SlbData structure
    auto basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);
    basicFeature->SetSLBData(slbData);
    
    // Unlock buffer
    m_allocator->UnLock(batchBuffer);

    // TU7 batch buffer construction for AdaptiveTU
    if (m_basicFeature->m_av1PicParams->AdaptiveTUEnabled != 0)
    {
        auto original_TU = m_basicFeature->m_targetUsage;
        m_basicFeature->m_targetUsage = m_basicFeature->m_av1SeqParams->TargetUsage = 7;

        MOS_RESOURCE *tu7Buffer = basicFeatureXe3pLpm->GetVdencReadBatchBufferTU7(recycledBufIdx, 0);
        ENCODE_CHK_NULL_RETURN(tu7Buffer);

        auto tu7Addr = (uint8_t *)m_allocator->LockResourceForWrite(tu7Buffer);
        ENCODE_CHK_NULL_RETURN(tu7Addr);

        MOS_COMMAND_BUFFER tu7CmdBuf;
        MOS_ZeroMemory(&tu7CmdBuf, sizeof(tu7CmdBuf));
        tu7CmdBuf.pCmdBase = tu7CmdBuf.pCmdPtr = (uint32_t *)tu7Addr;
        tu7CmdBuf.iRemaining = MOS_ALIGN_CEIL(m_basicFeature->GetSlbbBufferSize(), CODECHAL_PAGE_SIZE);

        SlbData tu7SlbData;

        /*----Group1----*/
        tu7SlbData.avpSegmentStateOffset = (uint16_t)tu7CmdBuf.iOffset;
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SEGMENT_STATE(&tu7CmdBuf));
        tu7SlbData.avpInloopFilterStateOffset = (uint16_t)tu7CmdBuf.iOffset;
        SETPAR_AND_ADDCMD(AVP_INLOOP_FILTER_STATE, m_avpItf, &tu7CmdBuf);
        ENCODE_CHK_STATUS_RETURN(AddBBEnd(tu7CmdBuf));

        /*----Group2----*/
        tu7SlbData.vdencCmd1Offset = (uint16_t)tu7CmdBuf.iOffset;
        SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, &tu7CmdBuf);
        ENCODE_CHK_STATUS_RETURN(AddBBEnd(tu7CmdBuf));

        /*----Group3----*/
        tu7SlbData.vdencCmd2Offset = (uint16_t)tu7CmdBuf.iOffset;
        SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, &tu7CmdBuf);
        ENCODE_CHK_STATUS_RETURN(AddBBEnd(tu7CmdBuf));

        /*----Group4----*/
        tu7SlbData.avpPicStateOffset = (uint16_t)tu7CmdBuf.iOffset;
        ENCODE_CHK_STATUS_RETURN(AddAvpPicStateBaseOnTile(tu7CmdBuf, tu7SlbData));

        /*----Group5----*/
        tu7SlbData.vdencTileSliceStateOffset = (uint16_t)tu7CmdBuf.iOffset;
        ENCODE_CHK_STATUS_RETURN(AddVdencTileSliceBaseOnTile(tu7CmdBuf, tu7SlbData));

        m_allocator->UnLock(tu7Buffer);

        m_basicFeature->m_targetUsage = m_basicFeature->m_av1SeqParams->TargetUsage = original_TU;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::AddBBEnd(MOS_COMMAND_BUFFER& cmdBuffer)
{
    ENCODE_FUNC_CALL();
    
    ENCODE_CHK_NULL_RETURN(m_miItf);
    
    MHW_BATCH_BUFFER TempBatchBuffer = {};
    TempBatchBuffer.iSize = CODECHAL_PAGE_SIZE;
    TempBatchBuffer.pData = (uint8_t *)cmdBuffer.pCmdBase;
    TempBatchBuffer.iCurrent = cmdBuffer.iOffset;
    TempBatchBuffer.iRemaining = cmdBuffer.iRemaining;
    
    ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
    
    cmdBuffer.pCmdPtr += (TempBatchBuffer.iCurrent - cmdBuffer.iOffset) / 4;
    cmdBuffer.iOffset = TempBatchBuffer.iCurrent;
    cmdBuffer.iRemaining = TempBatchBuffer.iRemaining;
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::AddAllCmds_AVP_SEGMENT_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
{
    ENCODE_FUNC_CALL();
    
    ENCODE_CHK_NULL_RETURN(m_featureManager);
    
    auto& par = m_avpItf->MHW_GETPAR_F(AVP_SEGMENT_STATE)();
    par = {};
    
    auto segmentFeature = dynamic_cast<Av1Segmentation *>(m_featureManager->GetFeature(Av1FeatureIDs::av1Segmentation));
    ENCODE_CHK_NULL_RETURN(segmentFeature);
    
    MHW_CHK_STATUS_RETURN(segmentFeature->MHW_SETPAR_F(AVP_SEGMENT_STATE)(par));
    
    const bool segmentEnabled = par.av1SegmentParams.m_enabled;
    
    for (uint8_t i = 0; i < av1MaxSegments; i++)
    {
        par.currentSegmentId = i;
        m_avpItf->MHW_ADDCMD_F(AVP_SEGMENT_STATE)(cmdBuffer);
        
        // If segmentation is not enabled, then AVP_SEGMENT_STATE must still be sent once for SegmentID = 0
        // If i == numSegments -1, means all segments are issued, break the loop
        if (!segmentEnabled || (i == par.numSegments - 1))
        {
            break;
        }
    }
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::AddAvpPicStateBaseOnTile(MOS_COMMAND_BUFFER& cmdBuffer, SlbData &slbData)
{
    ENCODE_FUNC_CALL();
    
    uint16_t numTileColumns = 1;
    uint16_t numTileRows = 1;
    bool firstTileInGroup = false;
    uint32_t tileGroupIdx = 0;
    
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);
    slbData.tileNum = numTileRows * numTileColumns;
    
    for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
    {
        for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
        {
            RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);
            RUN_FEATURE_INTERFACE_NO_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, IsFirstTileInGroup, firstTileInGroup, tileGroupIdx);
            RUN_FEATURE_INTERFACE_RETURN(Av1Scc, Av1FeatureIDs::av1Scc, UpdateIBCStatusForCurrentTile);

            if (tileRow == 0 && tileCol == 0)
            {
                SETPAR_AND_ADDCMD(AVP_PIC_STATE, m_avpItf, &cmdBuffer);
                ENCODE_CHK_STATUS_RETURN(AddBBEnd(cmdBuffer));
            }

            if (!firstTileInGroup)
            {
                slbData.avpPicStateCmdNum = 2;
                slbData.secondAvpPicStateOffset = (uint16_t)cmdBuffer.iOffset;
                SETPAR_AND_ADDCMD(AVP_PIC_STATE, m_avpItf, &cmdBuffer);
                ENCODE_CHK_STATUS_RETURN(AddBBEnd(cmdBuffer));
                break;
            }
        }
        
        if (slbData.avpPicStateCmdNum == 2)
        {
            break;
        }
    }
    
    // Add MI_NOOPs to align with CODECHAL_CACHELINE_SIZE
    uint32_t size = (MOS_ALIGN_CEIL(cmdBuffer.iOffset, CODECHAL_CACHELINE_SIZE) - cmdBuffer.iOffset) / sizeof(uint32_t);
    for (uint32_t i = 0; i < size; i++)
    {
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&cmdBuffer));
    }
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::AddVdencTileSliceBaseOnTile(MOS_COMMAND_BUFFER& cmdBuffer, SlbData& slbData)
{
    ENCODE_FUNC_CALL();
    
    uint16_t numTileColumns = 1;
    uint16_t numTileRows = 1;
    
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);
    
    auto& par = m_vdencItf->MHW_GETPAR_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)();
    par = {};
    
    for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
    {
        for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
        {
            RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);
            RUN_FEATURE_INTERFACE_RETURN(Av1Scc, Av1FeatureIDs::av1Scc, UpdateIBCStatusForCurrentTile);
            auto tileIdx = tileRow * numTileColumns + tileCol;

            m_basicFeature->m_vdencTileSliceStart[tileIdx] = cmdBuffer.iOffset;
            SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, &cmdBuffer);
            ENCODE_CHK_STATUS_RETURN(AddBBEnd(cmdBuffer));
            
            // Add MI_NOOPs to align with CODECHAL_CACHELINE_SIZE
            uint32_t size = (MOS_ALIGN_CEIL(cmdBuffer.iOffset, CODECHAL_CACHELINE_SIZE) - cmdBuffer.iOffset) / sizeof(uint32_t);
            for (uint32_t i = 0; i < size; i++)
            {
                ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&cmdBuffer));
            }
        }
    }
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::SetConstDataHuCSlbbUpdate()
{
    ENCODE_FUNC_CALL();

    // Lock constant data buffer and set up codec-specific constant data
    auto hucConstData = (uint8_t *)m_allocator->LockResourceForWrite(&m_slbbConstDataBuffer[m_pipeline->m_currRecycledBufIdx]);
    ENCODE_CHK_NULL_RETURN(hucConstData);

    // Set up AV1-specific constant data for HuC SLBB operations
    // This would typically include:
    // - Codec configuration parameters
    // - SLBB layout information
    // - Command buffer offsets and sizes
    // For now, zero out the buffer as placeholder
    MOS_ZeroMemory(hucConstData, m_slbbConstDataBufferSize);

    m_allocator->UnLock(&m_slbbConstDataBuffer[m_pipeline->m_currRecycledBufIdx]);

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS AV1HucSLBBUpdatePkt::DumpInput()
{
    ENCODE_FUNC_CALL();

    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    // Dump DMEM (HUC input)
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
        &m_slbbUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx],
        sizeof(HucAv1SlbbUpdateDmem),
        0,
        hucRegionDumpSlbbUpdate));

    // Dump Region 0 - Input SLBB (region0): the initial SLBB content HuC will update
    ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_InputSLBB", true, hucRegionDumpSlbbUpdate,
        MOS_ALIGN_CEIL(m_basicFeature->GetSlbbBufferSize(), CODECHAL_PAGE_SIZE)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AV1HucSLBBUpdatePkt::DumpOutput()
{
    ENCODE_FUNC_CALL();

    // Dump Region 1 - Output SLBB (region1): the SLBB content after HuC update
    ENCODE_CHK_STATUS_RETURN(DumpRegion(1, "_OutputSLBB", false, hucRegionDumpSlbbUpdate,
        MOS_ALIGN_CEIL(m_basicFeature->GetSlbbBufferSize(), CODECHAL_PAGE_SIZE)));

    return MOS_STATUS_SUCCESS;
}
#endif

MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, AV1HucSLBBUpdatePkt)
{
    ENCODE_FUNC_CALL();

    params.waitDoneVDCmdMsgParser = true;
    params.waitDoneAV1            = true;
    params.flushAV1               = true;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, AV1HucSLBBUpdatePkt)
{
    ENCODE_FUNC_CALL();
    
    params.kernelDescriptor = m_vdboxHucAv1SlbbUpdateKernelDescriptor;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, AV1HucSLBBUpdatePkt)
{
    params.notFirstPass = !m_pipeline->IsFirstPass();

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, AV1HucSLBBUpdatePkt)
{
    switch (m_pipeline->GetPipeNum())
    {
    case 0:
    case 1:
        params.numPipe = VDENC_PIPE_SINGLE_PIPE;
        break;
    case 2:
        params.numPipe = VDENC_PIPE_TWO_PIPE;
        break;
    case 4:
        params.numPipe = VDENC_PIPE_FOUR_PIPE;
        break;
    default:
        params.numPipe = VDENC_PIPE_INVALID;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, AV1HucSLBBUpdatePkt)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    params.function = SLBB_UPDATE;

    const uint32_t bufIdx = m_pipeline->m_currRecycledBufIdx;

    // Cast to xe3p_lpm_base to access platform-specific buffers
    auto basicFeatureXe3pLpm = dynamic_cast<Av1BasicFeatureXe3P_Lpm_Base *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeatureXe3pLpm);

    // Region 0 - Input SLB Buffer (Input Origin)
    int32_t currentPass = m_pipeline->GetCurrentPass();
    if (currentPass < 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    MOS_RESOURCE *inputBuffer = basicFeatureXe3pLpm->GetVdencReadBatchBufferOrigin(bufIdx, currentPass);
    ENCODE_CHK_NULL_RETURN(inputBuffer);
    params.regionParams[0].presRegion = inputBuffer;

    // Region 1 - Output SLBB Buffer (HuC firmware writes to HUC_REGION1)
    auto vdenc2ndLevelBatchBuffer = basicFeatureXe3pLpm->GetVdenc2ndLevelBatchBuffer(bufIdx);
    ENCODE_CHK_NULL_RETURN(vdenc2ndLevelBatchBuffer);
    params.regionParams[1].presRegion = &vdenc2ndLevelBatchBuffer->OsResource;
    params.regionParams[1].isWritable = true;

    // Region 2/3 - TU7 SLBB Buffer (AdaptiveTU: separate input and output buffers)
    if (m_basicFeature->m_av1PicParams->AdaptiveTUEnabled != 0)
    {
        MOS_RESOURCE *tu7InputBuffer = basicFeatureXe3pLpm->GetVdencReadBatchBufferTU7(bufIdx, currentPass);
        ENCODE_CHK_NULL_RETURN(tu7InputBuffer);
        params.regionParams[2].presRegion = tu7InputBuffer;

        auto tu7OutputBuffer = basicFeatureXe3pLpm->GetVdenc2ndLevelBatchBufferTU7(bufIdx);
        ENCODE_CHK_NULL_RETURN(tu7OutputBuffer);
        params.regionParams[3].presRegion = &tu7OutputBuffer->OsResource;
        params.regionParams[3].isWritable = true;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode