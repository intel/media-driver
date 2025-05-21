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
//! \file     encode_avc_huc_brc_update_packet.cpp
//! \brief    Defines the implementation of avc huc update packet
//!

#include "encode_avc_huc_brc_update_packet.h"
#include "encode_avc_vdenc_stream_in_feature.h"
#include "encode_avc_basic_feature.h"
#include "encode_avc_brc.h"
#include "encode_avc_vdenc_const_settings.h"
#include "media_avc_feature_defs.h"
#include "mos_os_cp_interface_specific.h"

namespace encode {

#define VDENC_AVC_STATIC_FRAME_INTRACOSTSCLRatioP   240
#define VDENC_AVC_BRC_HUC_STATUS_REENCODE_MASK      (1 << 31)

MOS_STATUS AvcHucBrcUpdatePkt::Init()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_featureManager);
    m_basicFeature = dynamic_cast<AvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    m_brcFeature = dynamic_cast<AvcEncodeBRC *>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
    ENCODE_CHK_NULL_RETURN(m_brcFeature);

    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePkt::SetDmemBuffer()const
{
    ENCODE_FUNC_CALL();

    // Program update DMEM
    auto hucVdencBrcUpdateDmem = (VdencAvcHucBrcUpdateDmem*)m_allocator->LockResourceForWrite(
        m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]);
    ENCODE_CHK_NULL_RETURN(hucVdencBrcUpdateDmem);

    RUN_FEATURE_INTERFACE_RETURN(AvcEncodeBRC, AvcFeatureIDs::avcBrcFeature, SetDmemForUpdate, hucVdencBrcUpdateDmem, m_pipeline->GetCurrentPass(), m_pipeline->IsLastPass());

    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(
        m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePkt::DumpOutput()
{
    ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL
    ENCODE_CHK_STATUS_RETURN(DumpHucBrcUpdate(false));
    ENCODE_CHK_STATUS_RETURN(DumpEncodeImgStats(nullptr));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePkt::AllocateResources()
{
    ENCODE_FUNC_CALL();

    PMOS_RESOURCE allocatedbuffer;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    // Const Data buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencBrcConstDataBufferSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC Const Data Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
    for (uint32_t i = 0; i < CODECHAL_ENCODE_VDENC_BRC_CONST_BUFFER_NUM; i++)
    {
        allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_vdencBrcConstDataBuffer[i] = allocatedbuffer;
    }

    for (uint32_t k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        // VDENC IMG STATE read buffer
        allocParamsForBufferLinear.dwBytes  = m_brcFeature->GetVdencBRCImgStateBufferSize();
        allocParamsForBufferLinear.pBufName = "VDENC BRC IMG State Read Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_vdencBrcImageStatesReadBufferOrigin[k] = allocatedbuffer;

        // VDENC IMG STATE read buffer
        allocParamsForBufferLinear.dwBytes      = m_brcFeature->GetVdencBRCImgStateBufferSize();
        allocParamsForBufferLinear.pBufName     = "VDENC BRC IMG State Read Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        allocatedbuffer                         = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_vdencBrcImageStatesReadBufferTU7[k] = allocatedbuffer;

        for (auto i = 0; i < VDENC_BRC_NUM_OF_PASSES; i++)
        {
            // BRC update DMEM
            allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDENC BrcUpdate DmemBuffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
            allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencBrcUpdateDmemBuffer[k][i] = allocatedbuffer;
        }
    }

    // PAK statistics output buffer 
    allocParamsForBufferLinear.dwBytes  = CODECHAL_PAGE_SIZE;
    allocParamsForBufferLinear.pBufName = "VDENC PAK Statistics MMIO Registers Output Buffer";
    allocatedbuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
    ENCODE_CHK_NULL_RETURN(allocatedbuffer);
    m_resPakOutputViaMmioBuffer = allocatedbuffer;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePkt::SetConstDataHuCBrcUpdate()const
{
    ENCODE_FUNC_CALL();

    // Set VDENC BRC constant buffer, data remains the same till BRC Init is called
    if (m_brcFeature->IsBRCInit())
    {
        for (uint8_t picType = 0; picType < CODECHAL_ENCODE_VDENC_BRC_CONST_BUFFER_NUM; picType++)
        {
            auto hucConstData = (uint8_t *)m_allocator->LockResourceForWrite(m_vdencBrcConstDataBuffer[picType]);
            ENCODE_CHK_NULL_RETURN(hucConstData);

            RUN_FEATURE_INTERFACE_RETURN(AvcEncodeBRC, AvcFeatureIDs::avcBrcFeature, FillHucConstData, hucConstData, picType);

            m_allocator->UnLock(m_vdencBrcConstDataBuffer[picType]);
        }
    }

    if (m_vdencStaticFrame)
    {
        auto hucConstData = (VdencAvcHucBrcConstantData *)m_allocator->LockResourceForWrite(m_vdencBrcConstDataBuffer[GetCurrConstDataBufIdx()]);
        ENCODE_CHK_NULL_RETURN(hucConstData);

        auto settings = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
        ENCODE_CHK_NULL_RETURN(settings);

        const auto &constTable4 = settings->vdencCMD3Table->AvcVdencCMD3ConstSettings_4;

        for (int j = 0; j < 42; j++)
        {
            hucConstData->UPD_P_Intra16x16[j] = constTable4[10 + j];
        }

        m_allocator->UnLock(m_vdencBrcConstDataBuffer[GetCurrConstDataBufIdx()]);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePkt::Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function)
{
    HUC_CHK_NULL_RETURN(cmdBuffer);

#if _SW_BRC
    HUC_CHK_STATUS_RETURN(InitSwBrc(function));
    if (m_swBrc && m_swBrc->SwBrcEnabled())
    {
        m_brcFeature->m_swBrc = m_swBrc;
        SETPAR(HUC_DMEM_STATE, m_hucItf);
        SETPAR(HUC_VIRTUAL_ADDR_STATE, m_hucItf);

        auto virtualAddrParams = m_hucItf->MHW_GETPAR_F(HUC_VIRTUAL_ADDR_STATE)();
        auto dmemParams        = m_hucItf->MHW_GETPAR_F(HUC_DMEM_STATE)();

        EncodeBasicFeature* basicFeature = dynamic_cast<EncodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        HUC_CHK_NULL_RETURN(basicFeature);
        return m_swBrc->SwBrcImpl(
            function,
            virtualAddrParams,
            dmemParams,
            basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0));
    }
#endif // !_SW_BRC

    if (prologNeeded)
    {
        SETPAR_AND_ADDCMD(MI_FORCE_WAKEUP, m_miItf, cmdBuffer);
        ENCODE_CHK_STATUS_RETURN(SendPrologCmds(*cmdBuffer));
    }

    if (m_brcFeature->IsBRCInitRequired())
    {
        m_resHucStatus2Buffer = m_brcFeature->GetHucStatus2Buffer();

        // Insert conditional batch buffer end for HuC valid IMEM loaded check
        auto &miConditionalBatchBufferEndParams               = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        miConditionalBatchBufferEndParams                     = {};
        miConditionalBatchBufferEndParams.presSemaphoreBuffer = m_resHucStatus2Buffer;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(cmdBuffer));
    }

    SetPerfTag(m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE : CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_SECOND_PASS,
        (uint16_t)m_basicFeature->m_mode,
        m_basicFeature->m_pictureCodingType);
    ENCODE_CHK_STATUS_RETURN(StartPerfCollect(*cmdBuffer));
    if (m_pipeline->IsSingleTaskPhaseSupported())
    {
        auto &miCpyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        miCpyMemMemParams             = {};
        miCpyMemMemParams.presSrc     = m_resPakOutputViaMmioBuffer;
        miCpyMemMemParams.presDst     = (m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]);
        miCpyMemMemParams.dwSrcOffset = miCpyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(VdencAvcHucBrcUpdateDmem, FrameByteCount);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

        miCpyMemMemParams.dwSrcOffset = miCpyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(VdencAvcHucBrcUpdateDmem, ImgStatusCtrl);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

        miCpyMemMemParams.dwSrcOffset = miCpyMemMemParams.dwDstOffset = CODECHAL_OFFSETOF(VdencAvcHucBrcUpdateDmem, NumOfSlice);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
    }

    // load kernel from WOPCM into L2 storage RAM
    AddAllCmds_HUC_IMEM_STATE(cmdBuffer);

    // pipe mode select
    AddAllCmds_HUC_PIPE_MODE_SELECT(cmdBuffer);

    // set HuC DMEM param
    SETPAR_AND_ADDCMD(HUC_DMEM_STATE, m_hucItf, cmdBuffer);

    // Copy data from BrcPakStatisticBufferFull to BrcPakStatisticBuffer if m_perMBStreamOutEnable is true
    if (m_basicFeature->m_perMBStreamOutEnable)
    {
        CodechalHucStreamoutParams hucStreamOutParams;
        MOS_ZeroMemory(&hucStreamOutParams, sizeof(hucStreamOutParams));

        auto setting = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
        ENCODE_CHK_NULL_RETURN(setting);

        PMOS_RESOURCE sourceSurface = m_basicFeature->m_recycleBuf->GetBuffer(BrcPakStatisticBufferFull, m_basicFeature->m_frameNum);
        PMOS_RESOURCE destSurface   = m_basicFeature->m_recycleBuf->GetBuffer(BrcPakStatisticBuffer, 0);
        uint32_t      copySize      = setting->brcSettings.vdencBrcPakStatsBufferSize;
        uint32_t      sourceOffset  = m_basicFeature->m_picWidthInMb * m_basicFeature->m_picHeightInMb * 64;
        uint32_t      destOffset    = 0;

        // Ind Obj Addr command
        hucStreamOutParams.dataBuffer            = sourceSurface;
        hucStreamOutParams.dataSize              = copySize + sourceOffset;
        hucStreamOutParams.dataOffset            = MOS_ALIGN_FLOOR(sourceOffset, CODECHAL_PAGE_SIZE);
        hucStreamOutParams.streamOutObjectBuffer = destSurface;
        hucStreamOutParams.streamOutObjectSize   = copySize + destOffset;
        hucStreamOutParams.streamOutObjectOffset = MOS_ALIGN_FLOOR(destOffset, CODECHAL_PAGE_SIZE);

        // Stream object params
        hucStreamOutParams.indStreamInLength    = copySize;
        hucStreamOutParams.inputRelativeOffset  = sourceOffset - hucStreamOutParams.dataOffset;
        hucStreamOutParams.outputRelativeOffset = destOffset - hucStreamOutParams.streamOutObjectOffset;

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->PerformHucStreamOut(
            &hucStreamOutParams,
            cmdBuffer));
    }

    SETPAR_AND_ADDCMD(HUC_VIRTUAL_ADDR_STATE, m_hucItf, cmdBuffer);

    HUC_CHK_STATUS_RETURN(StoreHuCStatus2Register(cmdBuffer, storeHucStatus2Needed));

    SETPAR_AND_ADDCMD(HUC_START, m_hucItf, cmdBuffer);

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpInput()))

    //TODO: double confirm why need Vdenc pipe flush
    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, cmdBuffer);

    // Flush the engine to ensure memory written out
    auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams                               = {};
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(EndPerfCollect(*cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_picParam);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_seqParam);

    ENCODE_CHK_STATUS_RETURN(ConstructImageStateReadBuffer(m_vdencBrcImageStatesReadBufferOrigin[m_pipeline->m_currRecycledBufIdx]));

    if (m_basicFeature->m_picParam->AdaptiveTUEnabled != 0)
    {
        auto original_TU              = m_basicFeature->m_targetUsage;
        m_basicFeature->m_targetUsage = m_basicFeature->m_seqParam->TargetUsage = 7;
        ENCODE_CHK_STATUS_RETURN(ConstructImageStateReadBuffer(m_vdencBrcImageStatesReadBufferTU7[m_pipeline->m_currRecycledBufIdx]));
        m_basicFeature->m_targetUsage = m_basicFeature->m_seqParam->TargetUsage = original_TU;
    }

    bool firstTaskInPhase = packetPhase & firstPacket;
    bool requestProlog = false;

    if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        requestProlog = true;
    }

    ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, BRC_UPDATE));

    ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxItf->GetMaxVdboxIndex()),
                           "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hucItf->GetMmioRegisters(m_vdboxIndex);

#if _SW_BRC
    if (!m_swBrc->SwBrcEnabled())
#endif
    {
        // Write HUC_STATUS mask
        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        storeDataParams.dwResourceOffset = sizeof(uint32_t);
        storeDataParams.dwValue          = VDENC_AVC_BRC_HUC_STATUS_REENCODE_MASK;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(commandBuffer));

        // store HUC_STATUS register
        auto &storeRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        storeRegMemParams                 = {};
        storeRegMemParams.presStoreBuffer = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        storeRegMemParams.dwOffset        = 0;
        storeRegMemParams.dwRegister      = mmioRegisters->hucStatusRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(commandBuffer));
    }

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpHucBrcUpdate(true));)

    // Set HuC DMEM buffers which need to be updated.
    // They are first pass of next frame and next pass of current frame, as the 2nd VDEnc+PAK pass may not be triggered.
    uint32_t nextRecycledBufIdx = (m_pipeline->m_currRecycledBufIdx + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
    uint32_t nextPass           = (m_pipeline->GetCurrentPass() + 1) % CODECHAL_VDENC_BRC_NUM_OF_PASSES;

    PMOS_RESOURCE vdencBrcUpdateDmemBuffer0 = m_vdencBrcUpdateDmemBuffer[nextRecycledBufIdx][0];
    PMOS_RESOURCE vdencBrcUpdateDmemBuffer1 = m_pipeline->IsLastPass() ? nullptr : m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][nextPass];

#if _SW_BRC
    if (!m_swBrc->SwBrcEnabled())
#endif
    {
        if (m_pipeline->IsSingleTaskPhaseSupported())
        {
            vdencBrcUpdateDmemBuffer0 = m_resPakOutputViaMmioBuffer;
            vdencBrcUpdateDmemBuffer1 = nullptr;
        }
    }

    RUN_FEATURE_INTERFACE_NO_RETURN(AvcEncodeBRC, AvcFeatureIDs::avcBrcFeature, SaveBrcUpdateDmemBufferPtr, vdencBrcUpdateDmemBuffer0, vdencBrcUpdateDmemBuffer1);

    // Disable Brc Init/reset  here after init cmd executed, APP will re-trigger the reset by DDI params m_seqParam->bResetBRC
    RUN_FEATURE_INTERFACE_NO_RETURN(AvcEncodeBRC, AvcFeatureIDs::avcBrcFeature, DisableBrcInitReset);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePkt::ConstructImageStateReadBuffer(PMOS_RESOURCE imageStateBuffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_mfxItf);
    ENCODE_CHK_NULL_RETURN(m_vdencItf);

    uint32_t mfxAvcImgStateSize = m_mfxItf->MHW_GETSIZE_F(MFX_AVC_IMG_STATE)();
    uint32_t vdencAvcCostStateSize = m_vdencItf->MHW_GETSIZE_F(VDENC_CMD3)();
    uint32_t vdencAvcImgStateSize  = m_vdencItf->MHW_GETSIZE_F(VDENC_AVC_IMG_STATE)();
    uint32_t miBatchBufferEndSize = m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

    uint8_t *data = (uint8_t*)m_allocator->LockResourceForWrite(imageStateBuffer);

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(MOS_COMMAND_BUFFER));
    constructedCmdBuf.pCmdBase   = constructedCmdBuf.pCmdPtr = (uint32_t *) data;
    constructedCmdBuf.iRemaining = m_brcFeature->GetVdencBRCImgStateBufferSize();

    // Set MFX_AVC_IMG_STATE command
    SETPAR_AND_ADDCMD(MFX_AVC_IMG_STATE, m_mfxItf, &constructedCmdBuf);

    // Set VDENC_CMD3 command
    SETPAR_AND_ADDCMD(VDENC_CMD3, m_vdencItf, &constructedCmdBuf);

    // Set VDENC_AVC_IMG_STATE command
    SETPAR_AND_ADDCMD(VDENC_AVC_IMG_STATE, m_vdencItf, &constructedCmdBuf);

    // Add batch buffer end insertion flag
    ENCODE_CHK_STATUS_RETURN(m_miItf->AddBatchBufferEndInsertionFlag(constructedCmdBuf));

    // AddBatchBufferEndInsertionFlag doesn't modify pCmdPtr + iOffset
    constructedCmdBuf.pCmdPtr += miBatchBufferEndSize / sizeof(uint32_t);
    constructedCmdBuf.iOffset += miBatchBufferEndSize;
    constructedCmdBuf.iRemaining -= miBatchBufferEndSize;

    // Add MI_NOOPs to align to CODECHAL_CACHELINE_SIZE
    uint32_t size = (MOS_ALIGN_CEIL(constructedCmdBuf.iOffset, CODECHAL_CACHELINE_SIZE) - constructedCmdBuf.iOffset) / sizeof(uint32_t);
    for (uint32_t i = 0; i < size; i++)
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&constructedCmdBuf));

    for (uint16_t slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
    {
        size = constructedCmdBuf.iOffset;

        // Set MFX_AVC_SLICE_STATE command
        SETPAR_AND_ADDCMD(MFX_AVC_SLICE_STATE, m_mfxItf, &constructedCmdBuf);

        // Set VDENC_AVC_SLICE_STATE command
        SETPAR_AND_ADDCMD(VDENC_AVC_SLICE_STATE, m_vdencItf, &constructedCmdBuf);

        m_miItf->AddBatchBufferEndInsertionFlag(constructedCmdBuf);
        constructedCmdBuf.pCmdPtr += miBatchBufferEndSize / sizeof(uint32_t);
        constructedCmdBuf.iOffset += miBatchBufferEndSize;
        constructedCmdBuf.iRemaining -= miBatchBufferEndSize;

        ENCODE_ASSERT(constructedCmdBuf.iOffset - size % CODECHAL_CACHELINE_SIZE != 0);
    }

    if (data)
    {
        m_allocator->UnLock(imageStateBuffer);
    }

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(PopulatePakParam(
            nullptr,
            nullptr));

        ENCODE_CHK_STATUS_RETURN(PopulateEncParam(
            0,
            nullptr));)

    return MOS_STATUS_SUCCESS;
}

uint32_t AvcHucBrcUpdatePkt::GetCurrConstDataBufIdx() const
{
    return m_basicFeature->m_picParam->CodingType == B_TYPE && m_basicFeature->m_picParam->RefPicFlag ? m_basicFeature->m_picParam->CodingType  // refB
                                                                                                      : m_basicFeature->m_picParam->CodingType - 1;
}

MHW_SETPAR_DECL_SRC(MFX_AVC_IMG_STATE, AvcHucBrcUpdatePkt)
{
    bool bIPCMPass = m_pipeline->GetCurrentPass() && m_pipeline->IsLastPass() && (!m_brcFeature->IsVdencBrcEnabled());
    params.mbstatenabled = bIPCMPass ? true : false; // Disable for the first pass

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, AvcHucBrcUpdatePkt)
{
    auto setting = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(setting);

    params.kernelDescriptor = setting->brcSettings.vdboxHucVdencBrcUpdateKernelDescriptor;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, AvcHucBrcUpdatePkt)
{
    ENCODE_CHK_STATUS_RETURN(SetDmemBuffer());

    params.function      = BRC_UPDATE;
    params.hucDataSource = m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()];
    params.dataLength    = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, AvcHucBrcUpdatePkt)
{
    params.function = BRC_UPDATE;

    // Set Const Data buffer
    ENCODE_CHK_STATUS_RETURN(SetConstDataHuCBrcUpdate());

    // Input regions
    params.regionParams[1].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencStatsBuffer, 0);
    params.regionParams[2].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(BrcPakStatisticBuffer, 0);
    params.regionParams[3].presRegion = m_vdencBrcImageStatesReadBufferOrigin[m_pipeline->m_currRecycledBufIdx];
    params.regionParams[5].presRegion = m_vdencBrcConstDataBuffer[GetCurrConstDataBufIdx()];
    params.regionParams[7].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(PakSliceSizeStreamOutBuffer, m_pipeline->GetCurrentPass() ?
        m_basicFeature->m_frameNum : m_basicFeature->m_frameNum ? m_basicFeature->m_frameNum-1 : 0);   // use stats from previous frame for pass 0
                                                                                                       // use stats from pass 0 for pass 1

    if (m_basicFeature->m_picParam->NumROI && !m_basicFeature->m_picParam->bNativeROI)  // Only for BRC non-native ROI
    {
        if (m_hwInterface->GetOsInterface()->osCpInterface != nullptr &&
            m_hwInterface->GetOsInterface()->osCpInterface->IsCpEnabled())
        {
            ENCODE_ASSERTMESSAGE("Non-native BRC ROI doesn't supports in CP case");
            return MOS_STATUS_UNIMPLEMENTED;
        }
        params.regionParams[8].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::HucRoiMapBuffer, m_basicFeature->m_frameNum);
    }

    if (m_basicFeature->m_picParam->AdaptiveTUEnabled != 0)
    {
        params.regionParams[12].presRegion = m_vdencBrcImageStatesReadBufferTU7[m_pipeline->m_currRecycledBufIdx];
    }

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS AvcHucBrcUpdatePkt::DumpHucBrcUpdate(bool isInput)
{
    ENCODE_FUNC_CALL();

    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    auto settings = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    auto brcSettings = settings->brcSettings;

    uint16_t currentPass = m_pipeline->GetCurrentPass();

    auto virtualAddrParams = m_hucItf->MHW_GETPAR_F(HUC_VIRTUAL_ADDR_STATE)();

    if (isInput)
    {
        //HUC DMEM dump
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
            m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass],
            m_vdencBrcUpdateDmemBufferSize,
            currentPass,
            hucRegionDumpUpdate));

        // History Buffer dump
        ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_History", isInput, hucRegionDumpUpdate, brcSettings.vdencBrcHistoryBufferSize));

        // Constant Data Buffer dump
        ENCODE_CHK_STATUS_RETURN(DumpRegion(5, "_ConstData", isInput, hucRegionDumpUpdate, m_vdencBrcConstDataBufferSize));

        // VDENC Statistics Buffer dump
        ENCODE_CHK_STATUS_RETURN(DumpRegion(1, "_VdencStats", isInput, hucRegionDumpUpdate, brcSettings.vdencBrcStatsBufferSize));

        // PAK Statistics Buffer dump
        ENCODE_CHK_STATUS_RETURN(DumpRegion(2, "_PakStats", isInput, hucRegionDumpUpdate, brcSettings.vdencBrcPakStatsBufferSize));

        // VDENC Img State Read Buffer dump
        ENCODE_CHK_STATUS_RETURN(DumpRegion(3, "_ImageStateRead", isInput, hucRegionDumpUpdate, m_brcFeature->GetVdencBRCImgStateBufferSize()));

        // Slice size Buffer dump
        ENCODE_CHK_STATUS_RETURN(DumpRegion(7, "_SliceSizeStreamOut", isInput, hucRegionDumpUpdate, CODECHAL_ENCODE_SLICESIZE_BUF_SIZE));

        // BRC non-native ROI
        if (m_basicFeature->m_picParam->NumROI && !m_basicFeature->m_picParam->bNativeROI)
        {
            ENCODE_CHK_STATUS_RETURN(DumpRegion(8, "_BrcROI_idxs", isInput, hucRegionDumpUpdate, m_basicFeature->m_picWidthInMb * m_basicFeature->m_picHeightInMb));
        }

        // VDEnc StreamIn
        if (m_basicFeature->m_picParam->NumROI && !m_basicFeature->m_picParam->bNativeROI)
        {
            ENCODE_CHK_STATUS_RETURN(DumpRegion(9, "_VDEnc_StreamIn", isInput, hucRegionDumpUpdate, m_basicFeature->m_picWidthInMb * m_basicFeature->m_picHeightInMb * CODECHAL_CACHELINE_SIZE));
        }
    }
    else
    {
        // History Buffer dump
        ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_History", isInput, hucRegionDumpUpdate, brcSettings.vdencBrcHistoryBufferSize));

        // VDENC Img State Write Buffer dump
        ENCODE_CHK_STATUS_RETURN(DumpRegion(6, "_ImageStateWrite", isInput, hucRegionDumpUpdate, m_brcFeature->GetVdencBRCImgStateBufferSize()));

        // VDEnc StreamIn
        if (m_basicFeature->m_picParam->NumROI && !m_basicFeature->m_picParam->bNativeROI)
        {
            ENCODE_CHK_STATUS_RETURN(DumpRegion(10, "_VDEnc_StreamIn", isInput, hucRegionDumpUpdate, m_basicFeature->m_picWidthInMb * m_basicFeature->m_picHeightInMb * CODECHAL_CACHELINE_SIZE));
        }

        ENCODE_CHK_STATUS_RETURN(DumpRegion(15, "_Debug", isInput, hucRegionDumpUpdate, brcSettings.vdencBrcDbgBufferSize));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePkt::DumpEncodeImgStats(
    PMOS_COMMAND_BUFFER cmdbuffer)
{
    ENCODE_FUNC_CALL();

    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    if (!debugInterface->DumpIsEnabled(CodechalDbgAttr::attrImageState))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::string SurfName = "Pak_VDEnc_Pass[" + std::to_string(static_cast<uint32_t>(m_pipeline->GetCurrentPass())) + "]";

    // MFX_AVC_IMG_STATE
    if (m_brcFeature->IsVdencBrcEnabled())
    {
        // BRC case: both MFX_AVC_IMG_STATE and VDENC_IMG_STATE are updated by HuC FW
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            &(m_brcFeature->GetBatchBufferForVdencImgStat()->OsResource),
            CodechalDbgAttr::attrImageState,
            SurfName.c_str(),
            m_hwInterface->m_vdencBrcImgStateBufferSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}
