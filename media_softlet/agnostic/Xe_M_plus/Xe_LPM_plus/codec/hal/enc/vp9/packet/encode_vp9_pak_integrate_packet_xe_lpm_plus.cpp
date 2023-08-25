/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_vp9_pak_integrate_packet_xe_lpm_plus.cpp
//! \brief    Defines the interface for pak integrate packet
//!
#include "encode_vp9_pak_integrate_packet_xe_lpm_plus.h"

namespace encode
{

MOS_STATUS Vp9PakIntegratePktXe_Lpm_Plus::SetDmemBuffer() const 
{
    ENCODE_FUNC_CALL();

    auto currentPass = m_pipeline->GetCurrentPass();
    if (currentPass >= Vp9EncodeBrc::m_brcMaxNumPasses)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    HucPakIntDmemXe_Lpm_Plus *dmem = (HucPakIntDmemXe_Lpm_Plus *)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE *>(&m_hucPakIntDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass]));
    ENCODE_CHK_NULL_RETURN(dmem);
    MOS_ZeroMemory(dmem, sizeof(HucPakIntDmemXe_Lpm_Plus));

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

MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, Vp9PakIntegratePktXe_Lpm_Plus)
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
