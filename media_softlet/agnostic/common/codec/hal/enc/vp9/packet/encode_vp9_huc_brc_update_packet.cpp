/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_vp9_huc_brc_update_packet.cpp
//! \brief    Defines the implementation of huc update packet for VP9
//!

#include "encode_vp9_huc_brc_update_packet.h"
#include <encode_vp9_vdenc_feature_manager.h>
#include "encode_vp9_brc.h"
#include "encode_vp9_segmentation.h"
#include "encode_vp9_pak.h"

namespace encode
{
const uint32_t Vp9HucBrcUpdatePkt::m_brcUpdateDmem[64] =
{
    0x00061A80, 0x00000000, 0x0007A120, 0x000493E0, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x0032000A, 0x00960064, 0x01680280, 0x02200000, 0x007802B8,
    0x00000000, 0x00000000, 0x00000000, 0x02032000, 0xB4785028, 0x67614B28, 0x0101A07D, 0x28010203,
    0x01030505, 0x00FEFCFA, 0x04060402, 0x78503C1E, 0x00FFC88C, 0x503C1E04, 0xFFC88C78, 0x28140200,
    0xC8A08246, 0x090800FF, 0x040C0B0A, 0x07060605, 0x06060504, 0xFB650007, 0xFB0501FF, 0x000501FE,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

MOS_STATUS Vp9HucBrcUpdatePkt::Init()
{
    ENCODE_FUNC_CALL();
    HUC_CHK_STATUS_RETURN(EncodeHucPkt::Init());
    ENCODE_CHK_NULL_RETURN(m_featureManager);

    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(Vp9FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcUpdatePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    // To match watchdog timer threshold in codechal
    ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

    // Construct picture state 2nd level batch buffer
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, ConstructPicStateBatchBuffer, m_pipeline);

    bool firstTaskInPhase = packetPhase & firstPacket;
    bool requestProlog    = false;

    if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        requestProlog = true;
    }

    ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog));

    // Write HUC_STATUS mask: DW1 (mask value)
    auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
    storeDataParams                  = {};
    storeDataParams.pOsResource      = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
    storeDataParams.dwResourceOffset = sizeof(uint32_t);
    storeDataParams.dwValue          = (1 << 31); // CODECHAL_VDENC_VP9_BRC_HUC_STATUS_REENCODE_MASK;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(commandBuffer));

    // Store HUC_STATUS register: DW0 (actual value)
    ENCODE_CHK_COND_RETURN((m_vdboxIndex > MHW_VDBOX_NODE_1), "ERROR - vdbox index exceed the maximum");
    ENCODE_CHK_NULL_RETURN(m_hwInterface->GetHucInterfaceNext());
    auto  mmioRegisters                 = m_hwInterface->GetHucInterfaceNext()->GetMmioRegisters(m_vdboxIndex);
    auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
    miStoreRegMemParams                 = {};
    miStoreRegMemParams.presStoreBuffer = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
    miStoreRegMemParams.dwOffset        = 0;
    miStoreRegMemParams.dwRegister      = mmioRegisters->hucStatusRegOffset;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(commandBuffer));

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpInput());)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcUpdatePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    ENCODE_FUNC_CALL();

    auto osInterface = m_hwInterface->GetOsInterface();
    ENCODE_CHK_NULL_RETURN(osInterface);

    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
    uint32_t                       vdencHucStatesSize = 0;
    uint32_t                       hucPatchListSize   = 0;

    ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
        m_basicFeature->m_mode, (uint32_t *)&vdencHucStatesSize, (uint32_t *)&hucPatchListSize, &stateCmdSizeParams));

    commandBufferSize      = vdencHucStatesSize;
    requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

    if (m_pipeline->IsSingleTaskPhaseSupported())
    {
        commandBufferSize *= m_pipeline->GetPassNum();
    }

    // 4K align since allocation is in chunks of 4K bytes
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcUpdatePkt::DumpOutput()
{
    ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL
    // Dump Huc Brc Update output

    uint32_t brcHistoryBufferSize                 = 0;
    uint32_t vdencPicState2ndLevelBatchBufferSize = 0;

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, GetBrcHistoryBufferSize, brcHistoryBufferSize);
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, GetVdencPictureState2ndLevelBatchBufferSize, vdencPicState2ndLevelBatchBufferSize);

    // Region 0 - BRC history buffer (In/Out)
    ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_BrcHistory", false, hucRegionDumpUpdate, brcHistoryBufferSize));
    // Region 4 - BRC data buffer
    ENCODE_CHK_STATUS_RETURN(DumpRegion(4, "_BrcData", false, hucRegionDumpUpdate, CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE));
    // Region 6 - Output second level batch (SLB) buffer
    ENCODE_CHK_STATUS_RETURN(DumpRegion(6, "_OutputSLBB", false, hucRegionDumpUpdate, vdencPicState2ndLevelBatchBufferSize));
    // Region 15 - For kernel debug purpose. May be used only in HuC debug build.
    ENCODE_CHK_STATUS_RETURN(DumpRegion(15, "_BrcDebugBuffer", false, hucRegionDumpUpdate));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcUpdatePkt::AllocateResources()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

    // Initiate allocation parameters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;

    // BRC update DMEM
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(sizeof(HucBrcUpdateDmem), CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BrcUpdate DmemBuffer";
    MOS_RESOURCE *allocatedBuffer       = nullptr;
    for (auto i = 0; i < 3; ++i)
    {
        for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; ++j)
        {
            allocatedBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedBuffer);
            m_resVdencBrcUpdateDmemBuffer[i][j] = *allocatedBuffer;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcUpdatePkt::SetDmemBuffer() const
{
    ENCODE_FUNC_CALL();

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    // Setup BRC DMEM
    auto              currPass = m_pipeline->GetCurrentPass();
    HucBrcUpdateDmem *dmem     = (HucBrcUpdateDmem *)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE *>(&m_resVdencBrcUpdateDmemBuffer[currPass][m_pipeline->m_currRecycledBufIdx]));
    ENCODE_CHK_NULL_RETURN(dmem);

    MOS_SecureMemcpy(dmem, sizeof(HucBrcUpdateDmem), m_brcUpdateDmem, sizeof(m_brcUpdateDmem));

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, SetDmemForUpdate, dmem, m_pipeline->IsFirstPass());
    RUN_FEATURE_INTERFACE_RETURN(Vp9Segmentation, Vp9FeatureIDs::vp9Segmentation, SetDmemForUpdate, dmem);

    // PAK info
    dmem->UPD_MaxNumPAKs_U8 = m_pipeline->GetPassNum() - 1;
    dmem->UPD_PAKPassNum_U8 = (uint8_t)currPass;

    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(const_cast<MOS_RESOURCE *>(&m_resVdencBrcUpdateDmemBuffer[currPass][m_pipeline->m_currRecycledBufIdx])));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Vp9HucBrcUpdatePkt::DumpInput()
{
    ENCODE_FUNC_CALL();

    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    uint32_t currentPass = m_pipeline->GetCurrentPass();
    uint32_t size        = 0;

    // Dump HucBrcUpdate input buffers
    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
        &m_resVdencBrcUpdateDmemBuffer[currentPass][m_pipeline->m_currRecycledBufIdx],
        sizeof(HucBrcUpdateDmem),
        currentPass,
        hucRegionDumpUpdate));

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    uint32_t pakMmioBufferSize = MOS_ALIGN_CEIL(CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE, CODECHAL_PAGE_SIZE);

    uint32_t brcHistoryBufferSize                 = 0;
    uint32_t vdencBrcStatsBufferSize              = 0;
    uint32_t vdencBrcPakStatsBufferSize           = 0;
    uint32_t vdencPicState2ndLevelBatchBufferSize = 0;

    brcFeature->GetBrcHistoryBufferSize(brcHistoryBufferSize);
    brcFeature->GetVdencBrcStatsBufferSize(vdencBrcStatsBufferSize);
    brcFeature->GetVdencBrcPakStatsBufferSize(vdencBrcPakStatsBufferSize);

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, GetVdencPictureState2ndLevelBatchBufferSize, vdencPicState2ndLevelBatchBufferSize);

    // Region 0 - BRC history buffer (In/Out)
    ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_BrcHistory", true, hucRegionDumpUpdate, brcHistoryBufferSize));
    // Region 1 - VDENC statistics buffer dump
    ENCODE_CHK_STATUS_RETURN(DumpRegion(1, "_VdencStats", true, hucRegionDumpUpdate, vdencBrcStatsBufferSize));
    // Region 2 -  PAK statistics buffer dump
    ENCODE_CHK_STATUS_RETURN(DumpRegion(2, "_PakStats", true, hucRegionDumpUpdate, vdencBrcPakStatsBufferSize));
    // Region 3 - Input second level batch (SLB) buffer
    ENCODE_CHK_STATUS_RETURN(DumpRegion(3, "_InputSLBB", true, hucRegionDumpUpdate, vdencPicState2ndLevelBatchBufferSize));
    // Region 5 - Constant data buffer dump
    ENCODE_CHK_STATUS_RETURN(DumpRegion(5, "_ConstData", true, hucRegionDumpUpdate, brcFeature->m_brcConstantSurfaceSize));
    // Region 7 - PAK MMIO
    ENCODE_CHK_STATUS_RETURN(DumpRegion(7, "_PakMmio", true, hucRegionDumpUpdate, pakMmioBufferSize));

    return MOS_STATUS_SUCCESS;
}
#endif

MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, Vp9HucBrcUpdatePkt)
{
    ENCODE_FUNC_CALL();

    params.kernelDescriptor = m_vdboxHucVp9VdencBrcUpdateKernelDescriptor;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, Vp9HucBrcUpdatePkt)
{
    ENCODE_FUNC_CALL();
    
    params.function      = BRC_UPDATE;

    ENCODE_CHK_STATUS_RETURN(SetDmemBuffer());

    params.passNum       = static_cast<uint8_t>(m_pipeline->GetPassNum());
    params.currentPass   = static_cast<uint8_t>(m_pipeline->GetCurrentPass());
    params.hucDataSource = const_cast<PMOS_RESOURCE>(&m_resVdencBrcUpdateDmemBuffer[m_pipeline->GetCurrentPass()][m_pipeline->m_currRecycledBufIdx]);
    params.dataLength    = MOS_ALIGN_CEIL(sizeof(HucBrcUpdateDmem), CODECHAL_CACHELINE_SIZE);
    params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, Vp9HucBrcUpdatePkt)
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    int32_t currentPass = m_pipeline->GetCurrentPass();
    if (currentPass < 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);

    params.function = BRC_UPDATE;

    auto brcFeature = dynamic_cast<Vp9EncodeBrc *>(m_featureManager->GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    uint32_t statBufIdx = 0;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetStatisticsBufferIndex, statBufIdx);
    MOS_RESOURCE *resTileBasedStatisticsBuffer = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, GetTileBasedStatisticsBuffer, statBufIdx, resTileBasedStatisticsBuffer);
    uint32_t offset = 0;

    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, InitConstantDataBuffer);
    MOS_RESOURCE *resBrcDataBuffer = nullptr;
    HucBrcBuffers *hucBrcBuffers = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, GetHucBrcBuffers, hucBrcBuffers);

    // Regions (in) : 0, 1, 2, 5, 7
    // Regions (out): 0, 4
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeBrc, Vp9FeatureIDs::vp9BrcFeature, SetRegionsForBrcUpdate, params);

    // Regions (in) : 3
    // Regions (out): 6
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodePak, Vp9FeatureIDs::vp9PakFeature, SetRegionsForBrcUpdate, params, currentPass);

    if (m_pipeline->IsFirstPass())
    {
        if (m_basicFeature->m_lastFrameScalableMode)
        {
            // Regions (in): 1, 2, 7
            RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, SetRegionsForBrcUpdate, params);
        }
    }
    else  // Second BRC Update Pass
    {
        if (m_basicFeature->m_scalableMode)
        {
            // Regions (in): 1, 2, 7
            RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, Vp9FeatureIDs::encodeTile, SetRegionsForBrcUpdate, params);
        }
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
