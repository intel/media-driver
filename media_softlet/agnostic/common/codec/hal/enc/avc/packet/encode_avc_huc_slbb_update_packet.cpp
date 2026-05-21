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
//! \file     encode_avc_huc_slbb_update_packet.cpp
//! \brief    Implements the AVCHucSLBBUpdatePkt class for AVC-specific HuC SLBB updates
//!

#include "encode_avc_huc_slbb_update_packet.h"
#include "media_avc_feature_defs.h"
#include "encode_avc_vdenc_pipeline.h"
#include "codec_def_common_encode.h"
#include "encode_avc_basic_feature_xe3p_lpm.h"
#include "encode_avc_vdenc_fastpass.h"
#include "mos_os_cp_interface_specific.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug.h"
#endif

namespace encode
{

AVCHucSLBBUpdatePkt::AVCHucSLBBUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
    : HucSLBBUpdatePkt(pipeline, task, hwInterface)
{
    ENCODE_FUNC_CALL();

    // Initialize kernel descriptor for AVC HuC SLBB Update
    m_vdboxHucKernelDescriptor = m_vdboxHucAvcSlbbUpdateKernelDescriptor;

    // Get feature manager from pipeline
    m_featureManager = m_pipeline->GetPacketLevelFeatureManager(AvcVdencPipeline::HucSLBBUpdate);

    // Set DMEM buffer size to match HucAvcSlbbUpdateDmem (64 bytes, cache-line aligned)
    m_slbbUpdateDmemBufferSize = sizeof(HucAvcSlbbUpdateDmem);
}

AVCHucSLBBUpdatePkt::~AVCHucSLBBUpdatePkt()
{
    ENCODE_FUNC_CALL();
}

MOS_STATUS AVCHucSLBBUpdatePkt::Init()
{
    ENCODE_FUNC_CALL();

    // Detect PXP BEFORE base Init (matches BRC PXP reference pattern)
    ENCODE_CHK_NULL_RETURN(m_osInterface);
    if (m_osInterface->osCpInterface != nullptr && m_osInterface->osCpInterface->IsHMEnabled())
    {
        m_isPxp = true;
    }

    // Call base class Init (sets up m_hucKernelSource and m_isPPGTT)
    ENCODE_CHK_STATUS_RETURN(HucSLBBUpdatePkt::Init());

    ENCODE_CHK_NULL_RETURN(m_pipeline);

    // Get allocator from pipeline
    m_allocator = m_pipeline->GetEncodeAllocator();
    ENCODE_CHK_NULL_RETURN(m_allocator);

    ENCODE_CHK_NULL_RETURN(m_featureManager);
    
    // Get AVC basic feature from feature manager
    m_basicFeature = dynamic_cast<AvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AVCHucSLBBUpdatePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_picParam);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_seqParam);

    // AVCHucSLBBUpdatePkt is first-task-in-phase: its prolog (SendPrologCmds) embeds
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

    // Set performance tags for AVC SLBB Update
    uint16_t perfTag = CODECHAL_ENCODE_PERFTAG_CALL_SLBB_UPDATE;
    uint16_t pictureType = (m_basicFeature->m_pictureCodingType == I_TYPE) ? 0 : 
        (m_basicFeature->m_pictureCodingType == P_TYPE ? 1 : 
            (m_basicFeature->m_pictureCodingType == B_TYPE ? 2 : 3));
    SetPerfTag(perfTag, (uint16_t)m_basicFeature->m_mode, pictureType);

    if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        requestProlog = true;
    }

    // Execute HuC SLBB Update
    ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, SLBB_UPDATE));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AVCHucSLBBUpdatePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
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

    // Reserve extra space for HUC_IMEM_ADDR emitted in PPGTT mode (matches BRC PXP reference)
    if (m_isPPGTT)
    {
        ENCODE_CHK_NULL_RETURN(m_itfPPGTT);
        commandBufferSize += m_itfPPGTT->MHW_GETSIZE_F(HUC_IMEM_ADDR)();
        requestedPatchListSize += PATCH_LIST_COMMAND(mhw::vdbox::huc::ItfPPGTT::HUC_IMEM_ADDR_CMD);
    }

    // 4K align since allocation is in chunks of 4K bytes.
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AVCHucSLBBUpdatePkt::AllocateResources()
{
    ENCODE_FUNC_CALL();
    
    // Call base class AllocateResources
    ENCODE_CHK_STATUS_RETURN(HucSLBBUpdatePkt::AllocateResources());

    // Load SLBB kernel binary for PPGTT mode
    if (m_isPPGTT && m_kernelBinBuffer == nullptr)
    {
        ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
        HucKernelSource::HucBinary hucBinary{};
        const uint32_t slbbKernelId = m_isPxp
                                          ? HucKernelSource::avcPxpSlbbUpdateKernelId
                                          : HucKernelSource::avcSlbbUpdateKernelId;
        ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelBin(slbbKernelId, hucBinary));

        ENCODE_CHK_NULL_RETURN(hucBinary.m_data);

        // Initiate allocation parameters for kernel binary buffer
        MOS_ALLOC_GFXRES_PARAMS allocParamsForKernelBuffer;
        MOS_ZeroMemory(&allocParamsForKernelBuffer, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForKernelBuffer.Type               = MOS_GFXRES_BUFFER;
        allocParamsForKernelBuffer.TileType           = MOS_TILE_LINEAR;
        allocParamsForKernelBuffer.Format             = Format_Buffer;
        allocParamsForKernelBuffer.dwBytes            = MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE);
        allocParamsForKernelBuffer.pBufName           = "AvcSlbbUpdateKernelBinBuffer";
        allocParamsForKernelBuffer.ResUsageType       = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        allocParamsForKernelBuffer.Flags.bNotLockable = false;  // Resource can be CPU accessed

        m_kernelBinBuffer = m_allocator->AllocateResource(allocParamsForKernelBuffer, false);
        ENCODE_CHK_NULL_RETURN(m_kernelBinBuffer);
        
        auto data = (uint16_t *)m_allocator->LockResourceForWrite(m_kernelBinBuffer);
        ENCODE_CHK_NULL_RETURN(data);
        MOS_SecureMemcpy(data, hucBinary.m_size, hucBinary.m_data, hucBinary.m_size);
        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_kernelBinBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AVCHucSLBBUpdatePkt::ConstructBatchBuffer()
{
    ENCODE_FUNC_CALL();
    
    // Null pointer checks
    ENCODE_CHK_NULL_RETURN(m_mfxItf);
    ENCODE_CHK_NULL_RETURN(m_vdencItf);
    ENCODE_CHK_NULL_RETURN(m_miItf);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_pipeline);
    
    // Cast m_basicFeature to platform-specific type
    auto basicFeatureXe3pLpm = dynamic_cast<AvcBasicFeatureXe3P_Lpm *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeatureXe3pLpm);
    
    // Get current recycled buffer index
    const uint32_t recycledBufIdx = m_pipeline->m_currRecycledBufIdx;
    
    // Retrieve Origin SLBB buffer
    PMOS_RESOURCE slbbBufferOrigin = basicFeatureXe3pLpm->GetVdencReadBatchBufferOrigin(recycledBufIdx);
    ENCODE_CHK_NULL_RETURN(slbbBufferOrigin);
    
    // Query command sizes
    uint32_t mfxAvcImgStateSize = m_mfxItf->MHW_GETSIZE_F(MFX_AVC_IMG_STATE)();
    uint32_t vdencAvcCostStateSize = m_vdencItf->MHW_GETSIZE_F(VDENC_CMD3)();
    uint32_t vdencAvcImgStateSize = m_vdencItf->MHW_GETSIZE_F(VDENC_AVC_IMG_STATE)();
    uint32_t miBatchBufferEndSize = m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();
    
    // Construct Origin SLBB Buffer
    uint8_t *data = (uint8_t*)m_allocator->LockResourceForWrite(slbbBufferOrigin);
    ENCODE_CHK_NULL_RETURN(data);
    
    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(MOS_COMMAND_BUFFER));
    constructedCmdBuf.pCmdBase = constructedCmdBuf.pCmdPtr = (uint32_t *)data;
    constructedCmdBuf.iRemaining = m_basicFeature->GetSlbbBufferSize();
    
    // Add image-level commands
    SETPAR_AND_ADDCMD(MFX_AVC_IMG_STATE, m_mfxItf, &constructedCmdBuf);
    SETPAR_AND_ADDCMD(VDENC_CMD3, m_vdencItf, &constructedCmdBuf);
    SETPAR_AND_ADDCMD(VDENC_AVC_IMG_STATE, m_vdencItf, &constructedCmdBuf);
    
    // Add batch buffer end and alignment
    ENCODE_CHK_STATUS_RETURN(m_miItf->AddBatchBufferEndInsertionFlag(constructedCmdBuf));
    constructedCmdBuf.pCmdPtr += miBatchBufferEndSize / sizeof(uint32_t);
    constructedCmdBuf.iOffset += miBatchBufferEndSize;
    constructedCmdBuf.iRemaining -= miBatchBufferEndSize;
    
    uint32_t size = (MOS_ALIGN_CEIL(constructedCmdBuf.iOffset, CODECHAL_CACHELINE_SIZE) - constructedCmdBuf.iOffset) / sizeof(uint32_t);
    for (uint32_t i = 0; i < size; i++)
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&constructedCmdBuf));
    
    // Add slice-level commands
    for (uint16_t slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
    {
        size = constructedCmdBuf.iOffset;
        SETPAR_AND_ADDCMD(MFX_AVC_SLICE_STATE, m_mfxItf, &constructedCmdBuf);
        SETPAR_AND_ADDCMD(VDENC_AVC_SLICE_STATE, m_vdencItf, &constructedCmdBuf);
        m_miItf->AddBatchBufferEndInsertionFlag(constructedCmdBuf);
        constructedCmdBuf.pCmdPtr += miBatchBufferEndSize / sizeof(uint32_t);
        constructedCmdBuf.iOffset += miBatchBufferEndSize;
        constructedCmdBuf.iRemaining -= miBatchBufferEndSize;
        ENCODE_ASSERT((constructedCmdBuf.iOffset - size) % CODECHAL_CACHELINE_SIZE != 0);
    }
    
    // Unlock Origin buffer
    if (data)
    {
        m_allocator->UnLock(slbbBufferOrigin);
    }

    // TU7 batch buffer construction for AdaptiveTU
    if (m_basicFeature->m_picParam->AdaptiveTUEnabled != 0)
    {
        auto original_TU = m_basicFeature->m_targetUsage;
        m_basicFeature->m_targetUsage = m_basicFeature->m_seqParam->TargetUsage = 7;

        PMOS_RESOURCE tu7Buffer = basicFeatureXe3pLpm->GetVdencReadBatchBufferTU7(recycledBufIdx);
        ENCODE_CHK_NULL_RETURN(tu7Buffer);

        uint8_t *dataTU7 = (uint8_t *)m_allocator->LockResourceForWrite(tu7Buffer);
        ENCODE_CHK_NULL_RETURN(dataTU7);

        MOS_COMMAND_BUFFER tu7CmdBuf;
        MOS_ZeroMemory(&tu7CmdBuf, sizeof(MOS_COMMAND_BUFFER));
        tu7CmdBuf.pCmdBase = tu7CmdBuf.pCmdPtr = (uint32_t *)dataTU7;
        tu7CmdBuf.iRemaining = m_basicFeature->GetSlbbBufferSize();

        SETPAR_AND_ADDCMD(MFX_AVC_IMG_STATE, m_mfxItf, &tu7CmdBuf);
        SETPAR_AND_ADDCMD(VDENC_CMD3, m_vdencItf, &tu7CmdBuf);
        SETPAR_AND_ADDCMD(VDENC_AVC_IMG_STATE, m_vdencItf, &tu7CmdBuf);

        ENCODE_CHK_STATUS_RETURN(m_miItf->AddBatchBufferEndInsertionFlag(tu7CmdBuf));
        tu7CmdBuf.pCmdPtr += miBatchBufferEndSize / sizeof(uint32_t);
        tu7CmdBuf.iOffset += miBatchBufferEndSize;
        tu7CmdBuf.iRemaining -= miBatchBufferEndSize;

        uint32_t tu7AlignSize = (MOS_ALIGN_CEIL(tu7CmdBuf.iOffset, CODECHAL_CACHELINE_SIZE) - tu7CmdBuf.iOffset) / sizeof(uint32_t);
        for (uint32_t i = 0; i < tu7AlignSize; i++)
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&tu7CmdBuf));

        for (uint16_t slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
        {
            uint32_t slcStart = tu7CmdBuf.iOffset;
            SETPAR_AND_ADDCMD(MFX_AVC_SLICE_STATE, m_mfxItf, &tu7CmdBuf);
            SETPAR_AND_ADDCMD(VDENC_AVC_SLICE_STATE, m_vdencItf, &tu7CmdBuf);
            m_miItf->AddBatchBufferEndInsertionFlag(tu7CmdBuf);
            tu7CmdBuf.pCmdPtr += miBatchBufferEndSize / sizeof(uint32_t);
            tu7CmdBuf.iOffset += miBatchBufferEndSize;
            tu7CmdBuf.iRemaining -= miBatchBufferEndSize;
            ENCODE_ASSERT((tu7CmdBuf.iOffset - slcStart) % CODECHAL_CACHELINE_SIZE != 0);
        }

        if (dataTU7)
        {
            m_allocator->UnLock(tu7Buffer);
        }

        m_basicFeature->m_targetUsage = m_basicFeature->m_seqParam->TargetUsage = original_TU;
    }

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS AVCHucSLBBUpdatePkt::DumpInput()
{
    ENCODE_FUNC_CALL();

    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    // Dump DMEM (HUC input)
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
        &m_slbbUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx],
        sizeof(HucAvcSlbbUpdateDmem),
        0,
        hucRegionDumpSlbbUpdate));

    // Dump Region 0 - Input SLBB (region0): the initial SLBB content HuC will update
    ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_InputSLBB", true, hucRegionDumpSlbbUpdate,
        m_basicFeature->GetSlbbBufferSize()));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AVCHucSLBBUpdatePkt::DumpOutput()
{
    ENCODE_FUNC_CALL();

    // Dump Region 1 - Output SLBB (region1): the SLBB content after HuC update
    ENCODE_CHK_STATUS_RETURN(DumpRegion(1, "_OutputSLBB", false, hucRegionDumpSlbbUpdate,
        m_basicFeature->GetSlbbBufferSize()));

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS AVCHucSLBBUpdatePkt::SetDmem() const
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_picParam);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_seqParam);
    ENCODE_CHK_NULL_RETURN(m_allocator);
    ENCODE_CHK_NULL_RETURN(m_mfxItf);
    ENCODE_CHK_NULL_RETURN(m_vdencItf);
    ENCODE_CHK_NULL_RETURN(m_miItf);

    const uint32_t bufIdx = m_pipeline->m_currRecycledBufIdx;
    auto dmem = (HucAvcSlbbUpdateDmem *)m_allocator->LockResourceForWrite(
        const_cast<MOS_RESOURCE *>(&m_slbbUpdateDmemBuffer[bufIdx]));
    ENCODE_CHK_NULL_RETURN(dmem);

    MOS_ZeroMemory(dmem, sizeof(HucAvcSlbbUpdateDmem));

    // Validate structure size matches HuC firmware expectation
    ENCODE_ASSERT(sizeof(HucAvcSlbbUpdateDmem) == 64);

    // Retrieve command sizes from MHW interfaces
    uint32_t mfxAvcImgStateSize     = m_mfxItf->MHW_GETSIZE_F(MFX_AVC_IMG_STATE)();
    uint32_t vdencAvcCostStateSize  = m_vdencItf->MHW_GETSIZE_F(VDENC_CMD3)();
    uint32_t vdencAvcImgStateSize   = m_vdencItf->MHW_GETSIZE_F(VDENC_AVC_IMG_STATE)();
    uint32_t miBatchBufferEndSize   = m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();
    uint32_t mfxAvcSliceStateSize   = m_mfxItf->MHW_GETSIZE_F(MFX_AVC_SLICE_STATE)();
    uint32_t vdencAvcSliceStateSize = m_vdencItf->MHW_GETSIZE_F(VDENC_AVC_SLICE_STATE)();

    uint32_t currentOffset = 0;

    dmem->MfxAvcImgStateOffset = (uint16_t)currentOffset;
    currentOffset += mfxAvcImgStateSize;

    dmem->VdencAvcCostStateOffset = (uint16_t)currentOffset;
    currentOffset += vdencAvcCostStateSize;

    dmem->VdencAvcImgStateOffset = (uint16_t)currentOffset;
    currentOffset += vdencAvcImgStateSize;

    // Skip MI_BATCH_BUFFER_END and alignment padding
    currentOffset += miBatchBufferEndSize;
    uint32_t group1EndOffset = MOS_ALIGN_CEIL(currentOffset, CODECHAL_CACHELINE_SIZE);

    dmem->MfxAvcSliceStateOffset = (uint16_t)group1EndOffset;
    dmem->VdencAvcSliceStateOffset = (uint16_t)(group1EndOffset + mfxAvcSliceStateSize);

    uint16_t numSlices = (uint16_t)MOS_MAX(1, m_basicFeature->m_numSlices);
    uint32_t perSliceSize = mfxAvcSliceStateSize + vdencAvcSliceStateSize + miBatchBufferEndSize;
    uint32_t totalSlbbSize = group1EndOffset + (uint32_t)numSlices * perSliceSize;
    ENCODE_CHK_COND_RETURN((totalSlbbSize > UINT16_MAX), "AVC SLBB size exceeds uint16_t range");
    dmem->SlbbSize = (uint16_t)totalSlbbSize;

    dmem->SliceNum   = numSlices;
    dmem->MaxPassNum = 0;

    switch (m_basicFeature->m_pictureCodingType)
    {
    case I_TYPE: dmem->FrameType = 2; break;
    case P_TYPE: dmem->FrameType = 0; break;
    case B_TYPE: dmem->FrameType = 1; break;
    default:     dmem->FrameType = 0; break;
    }

    int32_t qpY   = m_basicFeature->m_picParam->QpY;
    dmem->QpValue = (uint8_t)MOS_CLAMP_MIN_MAX(qpY, 0, 51);

    dmem->TargetUsage       = (uint8_t)m_basicFeature->m_seqParam->TargetUsage;
    dmem->RateControlMethod = (uint8_t)m_basicFeature->m_seqParam->RateControlMethod;
    dmem->PictureCodingType = (uint8_t)m_basicFeature->m_pictureCodingType;
    dmem->Level             = (uint8_t)m_basicFeature->m_seqParam->Level;
    dmem->RefPicFlag        = (uint8_t)m_basicFeature->m_picParam->RefPicFlag;
    dmem->LowDelayMode      = (uint8_t)m_basicFeature->m_seqParam->LowDelayMode;

    // Query FastPass feature from the pipeline-wide feature manager so the HuC sees
    // the downscaled picture dimensions used for the actual encode.
    uint32_t picFrameWidth  = m_basicFeature->m_seqParam->FrameWidth;
    uint32_t picFrameHeight = m_basicFeature->m_seqParam->FrameHeight;
    if (m_pipeline != nullptr)
    {
        auto featureMgr = m_pipeline->GetFeatureManager();
        if (featureMgr != nullptr)
        {
            auto fastPass = dynamic_cast<AvcVdencFastPass *>(featureMgr->GetFeature(AvcFeatureIDs::avcFastPass));
            if (fastPass != nullptr && fastPass->IsEnabled())
            {
                uint32_t dsW = fastPass->GetFastPassDsWidth();
                uint32_t dsH = fastPass->GetFastPassDsHeight();
                if (dsW > 0 && dsH > 0)
                {
                    picFrameWidth  = dsW;
                    picFrameHeight = dsH;
                }
            }
        }
    }
    dmem->PicWidthInMb  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(picFrameWidth);
    dmem->PicHeightInMb = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(picFrameHeight);
    dmem->GopPicSize    = (uint16_t)m_basicFeature->m_seqParam->GopPicSize;
    dmem->GopRefDist    = (uint16_t)m_basicFeature->m_seqParam->GopRefDist;

    dmem->EnableSliceLevelRateCtrl  = (uint8_t)m_basicFeature->m_seqParam->EnableSliceLevelRateCtrl;
    dmem->EnableRollingIntraRefresh = (uint8_t)m_basicFeature->m_picParam->EnableRollingIntraRefresh;

    auto osInterface = m_hwInterface->GetOsInterface();
    if (osInterface)
    {
        auto waTable = osInterface->pfnGetWaTable(osInterface);
        dmem->Wa_18011246551 = (waTable && MEDIA_IS_WA(waTable, Wa_18011246551)) ? 1 : 0;
    }

    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_sliceParams);
    dmem->NumRefIdxL0ActiveMinus1 = (uint8_t)m_basicFeature->m_sliceParams->num_ref_idx_l0_active_minus1;
    dmem->SliceQp                 = (uint8_t)(m_basicFeature->m_picParam->QpY + m_basicFeature->m_sliceParams->slice_qp_delta);

    if (m_basicFeature->m_seqParam->chroma_format_idc == 2 && m_basicFeature->m_seqParam->bit_depth_luma_minus8 == 2)
    {
        dmem->QpMin = 0;
        dmem->Is10Bit422 = 1;
    }
    else
    {
        dmem->QpMin = 10;
        dmem->Is10Bit422 = 0;
    }

    dmem->EntropyCodingModeFlag = m_basicFeature->m_picParam->entropy_coding_mode_flag;

    dmem->SignedQp = (int8_t)((qpY < 0) ? qpY : (qpY + m_basicFeature->m_sliceParams->slice_qp_delta));

    dmem->AdaptiveTUEnabled = m_basicFeature->m_picParam->AdaptiveTUEnabled;

    dmem->UseExtendedThreshold = m_basicFeature->m_useExtendedThreshold ? 1 : 0;

    dmem->ExtendedPlatform = IsExtendedPlatform() ? 1 : 0;

    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(
        const_cast<MOS_RESOURCE *>(&m_slbbUpdateDmemBuffer[bufIdx])));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, AVCHucSLBBUpdatePkt)
{
    ENCODE_FUNC_CALL();

    // In PPGTT+PXP the authenticated manifest publishes the kernel at a
    // hash-table slot rather than the legacy fixed descriptor; look it up
    // from the HuC kernel source so the HW fetches the verified image.
    if (m_isPPGTT && m_isPxp)
    {
        ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
        uint32_t hashIndex = HucKernelSource::avcSlbbUpdateKernelId;
        ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelHashIdx(HucKernelSource::avcPxpSlbbUpdateKernelId, hashIndex));
        params.kernelDescriptor = hashIndex;
    }
    else
    {
        params.kernelDescriptor = m_vdboxHucAvcSlbbUpdateKernelDescriptor;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_IMEM_ADDR, AVCHucSLBBUpdatePkt)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_kernelBinBuffer);

    params.kernelbinOffset = 0;
    params.kernelBinBuffer = m_kernelBinBuffer;
    params.kernelBinSize   = m_kernelBinBuffer->iSize;
    params.integrityEnable = m_isPxp;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, AVCHucSLBBUpdatePkt)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    params.function = SLBB_UPDATE;

    const uint32_t bufIdx = m_pipeline->m_currRecycledBufIdx;

    // Cast m_basicFeature to platform-specific type
    auto basicFeatureXe3pLpm = dynamic_cast<AvcBasicFeatureXe3P_Lpm *>(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(basicFeatureXe3pLpm);

    // Region 0 - Input SLB Buffer (Input Origin)
    MOS_RESOURCE *inputBuffer = basicFeatureXe3pLpm->GetVdencReadBatchBufferOrigin(bufIdx);
    ENCODE_CHK_NULL_RETURN(inputBuffer);
    params.regionParams[0].presRegion = inputBuffer;

    // Region 1 - Output SLBB Buffer (HuC firmware writes to HUC_REGION1)
    auto vdenc2ndLevelBatchBuffer = basicFeatureXe3pLpm->GetVdenc2ndLevelBatchBuffer(bufIdx);
    ENCODE_CHK_NULL_RETURN(vdenc2ndLevelBatchBuffer);
    params.regionParams[1].presRegion = &vdenc2ndLevelBatchBuffer->OsResource;
    params.regionParams[1].isWritable = true;

    // Region 2/3 - TU7 SLBB Buffer (AdaptiveTU: separate input and output buffers)
    if (m_basicFeature->m_picParam->AdaptiveTUEnabled != 0)
    {
        PMOS_RESOURCE tu7InputBuffer = basicFeatureXe3pLpm->GetVdencReadBatchBufferTU7(bufIdx);
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