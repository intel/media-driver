/*
* Copyright (c) 2024-2026, Intel Corporation
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
//! \file     encode_avc_huc_brc_update_packet_xe3p_lpm_base.cpp
//! \brief    Defines the implementation of avc huc update packet
//!

#include "encode_avc_vdenc_stream_in_feature.h"
#include "encode_avc_basic_feature_xe3p_lpm.h"
#include "encode_avc_brc.h"
#include "encode_avc_vdenc_const_settings.h"
#include "media_avc_feature_defs.h"
#include "mos_os_cp_interface_specific.h"
#include "encode_avc_huc_brc_update_packet_xe3p_lpm_base.h"
#include "mhw_vdbox_huc_ppgtt_itf.h"
#include "mhw_vdbox_huc_ppgtt_cmdpar.h"
#include "media_interfaces_huc_kernel_source.h"

namespace encode {
#define VDENC_AVC_BRC_HUC_STATUS_REENCODE_MASK (1 << 31)

MOS_STATUS AvcHucBrcUpdatePktXe3p_Lpm_Base::Init()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_hucKernelSource = HucKernelSourceDevice::CreateFactory(m_osInterface);
    ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
    ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->Init(m_hwInterface->GetSkuTable(), m_userSettingPtr));
    m_isPPGTT = m_hucKernelSource->IsPpgttMode();

    ENCODE_CHK_NULL_RETURN(m_osInterface);
    if (m_osInterface->osCpInterface != nullptr && m_osInterface->osCpInterface->IsHMEnabled())
    {
        m_isPxp = true;
    }

    ENCODE_CHK_STATUS_RETURN(AvcHucBrcUpdatePkt::Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePktXe3p_Lpm_Base::AllocateResources()
{
    ENCODE_FUNC_CALL();

    // Expanded parent AllocateResources() implementation with SLBB allocation removed
    // SLBB buffers (m_vdencBrcImageStatesReadBufferOrigin and m_vdencBrcImageStatesReadBufferTU7)
    // are now allocated in xe3p_lpm basic feature module instead

    PMOS_RESOURCE allocatedbuffer;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    // Const Data buffer
    allocParamsForBufferLinear.dwBytes      = m_vdencBrcConstDataBufferSize;
    allocParamsForBufferLinear.pBufName     = "VDENC BRC Const Data Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
    for (uint32_t i = 0; i < CODECHAL_ENCODE_VDENC_BRC_CONST_BUFFER_NUM; i++)
    {
        allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_vdencBrcConstDataBuffer[i] = allocatedbuffer;
    }

    for (uint32_t k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        // VDENC BRC IMG State Read Buffers are now allocated in xe3p_lpm basic feature
        // Cast m_basicFeature to platform-specific type to access SLBB buffers
        auto basicFeatureXe3pLpm = dynamic_cast<AvcBasicFeatureXe3P_Lpm *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(basicFeatureXe3pLpm);

        // BRC must read the SLBB Update OUTPUT (vdenc2ndLevelBatchBuffer), not the origin input.
        // The HuC SLBB kernel modifies the origin buffer and writes to vdenc2ndLevelBatchBuffer.
        // BRC then reads those HuC-modified values to apply rate control adjustments.
        // This matches the AV1 flow in encode_av1_brc_update_packet_xe3p_lpm_base.cpp.
        auto vdenc2ndLevelBatchBuffer = basicFeatureXe3pLpm->GetVdenc2ndLevelBatchBuffer(k);
        ENCODE_CHK_NULL_RETURN(vdenc2ndLevelBatchBuffer);
        m_vdencBrcImageStatesReadBufferOrigin[k] = &vdenc2ndLevelBatchBuffer->OsResource;

        auto vdenc2ndLevelBatchBufferTU7 = basicFeatureXe3pLpm->GetVdenc2ndLevelBatchBufferTU7(k);
        ENCODE_CHK_NULL_RETURN(vdenc2ndLevelBatchBufferTU7);
        m_vdencBrcImageStatesReadBufferTU7[k]    = &vdenc2ndLevelBatchBufferTU7->OsResource;

        for (auto i = 0; i < VDENC_BRC_NUM_OF_PASSES; i++)
        {
            // BRC update DMEM
            allocParamsForBufferLinear.dwBytes      = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.pBufName     = "VDENC BrcUpdate DmemBuffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
            allocatedbuffer                         = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
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

    // Allocate PPGTT kernel binary buffer if needed (existing xe3p_lpm_base logic)
    if (m_isPPGTT && m_kernelBinBuffer == nullptr)
    {
        HucKernelSource::HucBinary hucBinary{};
        ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
        if (m_isPxp)
        {
            ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelBin(HucKernelSource::avcPxpBrcUpdateKernelId, hucBinary));
        }
        else
        {
            ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelBin(HucKernelSource::vdencBrcUpdateKernelId, hucBinary));
        }

        ENCODE_CHK_NULL_RETURN(hucBinary.m_data);

        // initiate allocation paramters
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type               = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType           = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format             = Format_Buffer;
        allocParamsForBufferLinear.dwBytes            = MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName           = "AvcUpdateKernelBinBuffer";
        allocParamsForBufferLinear.ResUsageType       = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        allocParamsForBufferLinear.Flags.bNotLockable = false;  // Resource can be CPU accessed

        m_kernelBinBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        auto data         = (uint16_t *)m_allocator->LockResourceForWrite(m_kernelBinBuffer);
        ENCODE_CHK_NULL_RETURN(data);
        MOS_SecureMemcpy(data, hucBinary.m_size, hucBinary.m_data, hucBinary.m_size);
        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_kernelBinBuffer));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePktXe3p_Lpm_Base::Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function)
{
    ENCODE_FUNC_CALL();
    HUC_CHK_NULL_RETURN(cmdBuffer);
    ENCODE_CHK_NULL_RETURN(m_itfPPGTT);
    ENCODE_CHK_NULL_RETURN(m_miItf);

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

    if (m_isPPGTT)
    {
        SETPAR_AND_ADDCMD(HUC_IMEM_ADDR, m_itfPPGTT, cmdBuffer);
    }
    // load kernel from WOPCM into L2 storage RAM
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HUC_IMEM_STATE(cmdBuffer));

    // pipe mode select
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HUC_PIPE_MODE_SELECT(cmdBuffer));

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

MOS_STATUS AvcHucBrcUpdatePktXe3p_Lpm_Base::AddAllCmds_HUC_IMEM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
{
    ENCODE_FUNC_CALL();
    MHW_MI_CHK_NULL(cmdBuffer);

    uint32_t hashIndex = HucKernelSource::vdencBrcUpdateKernelId;
    ENCODE_CHK_NULL_RETURN(m_hucKernelSource);

    if (m_isPxp)
    {
       ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelHashIdx(HucKernelSource::avcPxpBrcUpdateKernelId, hashIndex));
    }

    auto setting = static_cast<AvcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(setting);

    auto &par = m_hucItf->MHW_GETPAR_F(HUC_IMEM_STATE)();
    par       = {};

    if (m_isPPGTT)
    {
        par.kernelDescriptor = hashIndex;
    }
    else
    {
        par.kernelDescriptor = setting->brcSettings.vdboxHucVdencBrcUpdateKernelDescriptor;
    }

    ENCODE_CHK_STATUS_RETURN(m_hucItf->MHW_ADDCMD_F(HUC_IMEM_STATE)(cmdBuffer));

    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    ENCODE_CHK_STATUS_RETURN((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(cmdBuffer)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePktXe3p_Lpm_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(AvcHucBrcUpdatePkt::CalculateCommandSize(commandBufferSize, requestedPatchListSize));

    if (m_isPPGTT)
    {
        ENCODE_CHK_NULL_RETURN(m_itfPPGTT);
        commandBufferSize += m_itfPPGTT->MHW_GETSIZE_F(HUC_IMEM_ADDR)();
        requestedPatchListSize += PATCH_LIST_COMMAND(mhw::vdbox::huc::ItfPPGTT::HUC_IMEM_ADDR_CMD);
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcHucBrcUpdatePktXe3p_Lpm_Base::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_picParam);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_seqParam);

    // SLBB construction logic removed - SLBB buffers are now constructed by AVCHucSLBBUpdatePkt

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

    // Disable Brc Init/reset here after init cmd executed, APP will re-trigger the reset by DDI params m_seqParam->bResetBRC
    RUN_FEATURE_INTERFACE_NO_RETURN(AvcEncodeBRC, AvcFeatureIDs::avcBrcFeature, DisableBrcInitReset);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_IMEM_ADDR, AvcHucBrcUpdatePktXe3p_Lpm_Base)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_kernelBinBuffer);

    params.kernelbinOffset = 0;
    params.kernelBinBuffer = m_kernelBinBuffer;
    params.kernelBinSize   = m_kernelBinBuffer->iSize;
    params.integrityEnable = false;

    if (m_isPxp)
    {
        params.integrityEnable = true;
    }

    return MOS_STATUS_SUCCESS;
}
}
