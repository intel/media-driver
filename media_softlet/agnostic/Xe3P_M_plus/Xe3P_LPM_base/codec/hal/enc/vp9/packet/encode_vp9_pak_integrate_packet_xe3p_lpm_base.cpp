/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     encode_vp9_pak_integrate_packet_xe3p_lpm_base.cpp
//! \brief    Defines the interface for vp9 pak integrate packet
//!
#include "encode_vp9_pak_integrate_packet_xe3p_lpm_base.h"
#include "media_interfaces_huc_kernel_source.h"

namespace encode
{
MOS_STATUS Vp9PakIntegratePktXe3p_Lpm_Base::SetDmemBuffer() const
{
    ENCODE_FUNC_CALL();

    auto currentPass = m_pipeline->GetCurrentPass();
    if (currentPass >= Vp9EncodeBrc::m_brcMaxNumPasses)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    HucPakIntDmemXe3p_Lpm_Base *dmem = (HucPakIntDmemXe3p_Lpm_Base *)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE *>(&m_hucPakIntDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]));
    ENCODE_CHK_NULL_RETURN(dmem);
    MOS_ZeroMemory(dmem, sizeof(HucPakIntDmemXe3p_Lpm_Base));

    MOS_FillMemory(dmem, m_pakIntDmemOffsetsSize, 0xFF);

    uint16_t numTileColumns = 1;
    uint16_t numTileRows    = 1;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);
    uint32_t numTiles = 1;
    RUN_FEATURE_INTERFACE_RETURN(Vp9EncodeTile, FeatureIDs::encodeTile, GetTileNum, numTiles);

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

    //Offset 1 - 4 is for region 0 - Input to PAK integration kernel for all tile statistics per pipe
    for (auto i = 1; i <= m_pipeline->GetPipeNum(); ++i)
    {
        dmem->numTilesPerPipe[i - 1]    = numTilesPerPipe;
        dmem->tileSizeRecordOffset[i]   = vp9TileStatsOffset.tileSizeRecord + ((i - 1) * (dmem->numTilesPerPipe[i - 1]) * vp9StatsSize.tileSizeRecord);
        dmem->vdencStatOffset[i]        = vp9TileStatsOffset.vdencStats + ((i - 1) * (dmem->numTilesPerPipe[i - 1]) * vp9StatsSize.vdencStats);
        dmem->vp9PakStatOffset[i]       = vp9TileStatsOffset.pakStats + ((i - 1) * (dmem->numTilesPerPipe[i - 1]) * vp9StatsSize.pakStats);
        dmem->vp9CounterBufferOffset[i] = vp9TileStatsOffset.counterBuffer + ((i - 1) * (dmem->numTilesPerPipe[i - 1]) * vp9StatsSize.counterBuffer);
    }

    m_allocator->UnLock(const_cast<MOS_RESOURCE *>(&m_hucPakIntDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePktXe3p_Lpm_Base::Init()
{
    ENCODE_FUNC_CALL();

    m_hucKernelSource = HucKernelSourceDevice::CreateFactory(m_osInterface);
    ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_isPPGTT = m_hucKernelSource->IsPpgttMode(m_hwInterface->GetSkuTable(), m_userSettingPtr);

    ENCODE_CHK_STATUS_RETURN(Vp9PakIntegratePkt::Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePktXe3p_Lpm_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Vp9PakIntegratePkt::CalculateCommandSize(commandBufferSize, requestedPatchListSize));

    if (m_isPPGTT)
    {
        ENCODE_CHK_NULL_RETURN(m_itfPPGTT);
        commandBufferSize += m_itfPPGTT->MHW_GETSIZE_F(HUC_IMEM_ADDR)();
        requestedPatchListSize += PATCH_LIST_COMMAND(mhw::vdbox::huc::ItfPPGTT::HUC_IMEM_ADDR_CMD);
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9PakIntegratePktXe3p_Lpm_Base::AllocateResources()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Vp9PakIntegratePkt::AllocateResources());

    if (m_isPPGTT && m_kernelBinBuffer == nullptr)
    {
        ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
        HucKernelSource::HucBinary hucBinary{};
        ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelBin(HucKernelSource::pakIntegrationKernelId, hucBinary));

        ENCODE_CHK_NULL_RETURN(hucBinary.m_data);

        // initiate allocation paramters
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type               = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType           = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format             = Format_Buffer;
        allocParamsForBufferLinear.dwBytes            = MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName           = "Vp9PakIntegrationKernelBinBuffer";
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

MOS_STATUS Vp9PakIntegratePktXe3p_Lpm_Base::Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function)
{
    ENCODE_FUNC_CALL();
    HUC_CHK_NULL_RETURN(cmdBuffer);
    ENCODE_CHK_NULL_RETURN(m_itfPPGTT);

#if _SW_BRC
    HUC_CHK_STATUS_RETURN(InitSwBrc(function));
    if (function != NONE_BRC && m_swBrc && m_swBrc->SwBrcEnabled())
    {
        SETPAR(HUC_DMEM_STATE, m_hucItf);
        SETPAR(HUC_VIRTUAL_ADDR_STATE, m_hucItf);

        auto &virtualAddrParams = m_hucItf->MHW_GETPAR_F(HUC_VIRTUAL_ADDR_STATE)();
        auto &dmemParams        = m_hucItf->MHW_GETPAR_F(HUC_DMEM_STATE)();

        CODECHAL_DEBUG_TOOL(
            ENCODE_CHK_STATUS_RETURN(DumpInput());)

        EncodeBasicFeature *basicFeature = dynamic_cast<EncodeBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        HUC_CHK_NULL_RETURN(basicFeature);
        return m_swBrc->SwBrcImpl(
            function,
            virtualAddrParams,
            dmemParams,
            basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0));
    }
#endif  // !_SW_BRC

    if (prologNeeded)
    {
        ENCODE_CHK_STATUS_RETURN(AddForceWakeup(*cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(SendPrologCmds(*cmdBuffer));
    }

    ENCODE_CHK_STATUS_RETURN(StartPerfCollect(*cmdBuffer));

    if (m_isPPGTT)
    {
        SETPAR_AND_ADDCMD(HUC_IMEM_ADDR, m_itfPPGTT, cmdBuffer);
    }
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HUC_IMEM_STATE(cmdBuffer));
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HUC_PIPE_MODE_SELECT(cmdBuffer));

    SETPAR_AND_ADDCMD(HUC_DMEM_STATE, m_hucItf, cmdBuffer);
    SETPAR_AND_ADDCMD(HUC_VIRTUAL_ADDR_STATE, m_hucItf, cmdBuffer);

    m_enableHucStatusReport = true;
    HUC_CHK_STATUS_RETURN(StoreHuCStatus2Register(cmdBuffer, storeHucStatus2Needed));

    SETPAR_AND_ADDCMD(HUC_START, m_hucItf, cmdBuffer);

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpInput());)

    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, cmdBuffer);

    // Flush the engine to ensure memory written out
    ENCODE_CHK_NULL_RETURN(m_miItf);
    auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams                               = {};
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    HUC_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(EndPerfCollect(*cmdBuffer));
    HUC_CHK_STATUS_RETURN(StoreHuCStatusRegister(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}
MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, Vp9PakIntegratePktXe3p_Lpm_Base)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(SetDmemBuffer());

    params.function      = PAK_INTEGRATE;
    uint32_t currentPass = m_pipeline->GetCurrentPass();
    params.hucDataSource = const_cast<PMOS_RESOURCE>(&m_hucPakIntDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]);
    params.dataLength    = MOS_ALIGN_CEIL(m_hucPakIntDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
