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
//! \file     encode_av1_vdenc_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for av1 encode vdenc packet of Xe_LPM_plus+
//!
#include "encode_av1_vdenc_packet_xe_lpm_plus_base.h"
#include "mos_solo_generic.h"
#include "encode_av1_superres.h"

namespace encode
{

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::Init()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Av1VdencPkt::Init());
    auto superResFeature = dynamic_cast<Av1SuperRes *>(m_featureManager->GetFeature(Av1FeatureIDs::av1SuperRes));
    ENCODE_CHK_NULL_RETURN(superResFeature);
    m_mmcState = m_pipeline->GetMmcState();
    ENCODE_CHK_NULL_RETURN(m_mmcState);
    ENCODE_CHK_STATUS_RETURN(superResFeature->InitMMCState(m_mmcState));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::AllocateResources()
{
    ENCODE_FUNC_CALL();

    mhw::vdbox::avp::AvpBufferSizePar avpBufSizeParam;
    memset(&avpBufSizeParam, 0, sizeof(avpBufSizeParam));
    avpBufSizeParam.bitDepthIdc      = (m_basicFeature->m_bitDepth - 8) >> 1;
    avpBufSizeParam.height        = CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, av1SuperBlockHeight);
    avpBufSizeParam.width         = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, av1SuperBlockWidth);
    avpBufSizeParam.tileWidth        = CODECHAL_GET_HEIGHT_IN_BLOCKS(av1MaxTileWidth, av1SuperBlockWidth);
    avpBufSizeParam.isSb128x128      = 0;
    avpBufSizeParam.curFrameTileNum  = av1MaxTileNum;
    avpBufSizeParam.numTileCol       = av1MaxTileColumn;
    avpBufSizeParam.numOfActivePipes = 1;

    ENCODE_CHK_STATUS_RETURN(Av1VdencPkt::AllocateResources());

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type     = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format   = Format_Buffer;

    allocParams.dwBytes         = MOS_ALIGN_CEIL(sizeof(Av1VdencPakInfo), CODECHAL_PAGE_SIZE);
    allocParams.pBufName        = "VDENC BRC PakInfo";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_recycleBuf->RegisterResource(PakInfo, allocParams, 6);

    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type     = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format   = Format_Buffer;

    allocParams.dwBytes                   = avpBufSizeParam.height * avpBufSizeParam.width * 4;
    allocParams.pBufName                  = "VDEnc Cumulative CU Count Streamout Surface";
    allocParams.ResUsageType              = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_resCumulativeCuCountStreamoutBuffer = m_allocator->AllocateResource(allocParams, false);
    if (m_resCumulativeCuCountStreamoutBuffer == nullptr)
    {
        ENCODE_ASSERTMESSAGE("Failed to allocate VDEnc Cumulative CU Count Streamout Surface.");
        return MOS_STATUS_UNKNOWN;
    }

    allocParams.dwBytes = (m_basicFeature->m_bitDepth == 8) ? avpBufSizeParam.width * 2 * av1SuperBlockWidth * CODECHAL_CACHELINE_SIZE
                                                            : avpBufSizeParam.width * 4 * av1SuperBlockWidth * CODECHAL_CACHELINE_SIZE;  // Number of Cachelines per SB;
    allocParams.pBufName               = "m_resMfdIntraRowStoreScratchBuffer";
    allocParams.ResUsageType           = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_resMfdIntraRowStoreScratchBuffer = m_allocator->AllocateResource(allocParams, false);

    if (m_basicFeature->m_resMfdIntraRowStoreScratchBuffer == nullptr)
    {
        ENCODE_ASSERTMESSAGE("Failed to allocate VDEnc Tile Row Store Buffer.");
        return MOS_STATUS_UNKNOWN;
    }

    // VDENC tile row store buffer
    allocParams.dwBytes         = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, 32) * CODECHAL_CACHELINE_SIZE * 2;
    allocParams.pBufName        = "VDENC Tile Row Store Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_vdencTileRowStoreBuffer   = m_allocator->AllocateResource(allocParams, false);

    if (m_vdencTileRowStoreBuffer == nullptr)
    {
        ENCODE_ASSERTMESSAGE("Failed to allocate VDEnc Tile Row Store Buffer.");
        return MOS_STATUS_UNKNOWN;
    }

    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type     = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format   = Format_Buffer;

    // Bitstream decode line rowstore buffer
    if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::bsdLineBuffer))
    {
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::bsdLineBuffer, &avpBufSizeParam));
        allocParams.dwBytes         = avpBufSizeParam.bufferSize;
        allocParams.pBufName        = "Bitstream Decoder Encoder Line Rowstore Read Write buffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);
    }

    // Intra Prediction Tile Line Rowstore Read/Write Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::intraPredLineBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Intra Prediction Tile Line Rowstore Read Write Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_intraPredictionTileLineRowstoreReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

    // Spatial motion vector Line rowstore buffer
    if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::spatialMvLineBuffer))
    {
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::spatialMvLineBuffer, &avpBufSizeParam));
        allocParams.dwBytes         = avpBufSizeParam.bufferSize;
        allocParams.pBufName        = "Spatial motion vector Line rowstore buffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_spatialMotionVectorLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);
    }

    // Spatial motion vector Tile Line Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::spatialMvTileLineBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Spatial motion vector Tile Line Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_spatialMotionVectorCodingTileLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Meta Tile Column Read/Write Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::lrMetaTileColBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Meta Tile Column Read Write Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationMetaTileColumnReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Read/Write Line Y Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::lrTileLineYBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Read Write Line Y Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileReadWriteLineYBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Read/Write Line U Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::lrTileLineUBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Read Write Line U Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileReadWriteLineUBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Read Write Line V Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::lrTileLineVBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Read Write Line V Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileReadWriteLineVBuffer = m_allocator->AllocateResource(allocParams, false);

    // Deblocker Filter Line Read Write Y Buffer
    if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::deblockLineYBuffer))
    {
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockLineYBuffer, &avpBufSizeParam));
        allocParams.dwBytes         = avpBufSizeParam.bufferSize;
        allocParams.pBufName        = "Deblocker Filter Line Read Write Y Buffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_deblockerFilterLineReadWriteYBuffer = m_allocator->AllocateResource(allocParams, false);
    }

    // Deblocker Filter Line Read Write U Buffer
    if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::deblockLineUBuffer))
    {
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockLineUBuffer, &avpBufSizeParam));
        allocParams.dwBytes         = avpBufSizeParam.bufferSize;
        allocParams.pBufName        = "Deblocker Filter Line Read Write U Buffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_deblockerFilterLineReadWriteUBuffer = m_allocator->AllocateResource(allocParams, false);
    }

    // Deblocker Filter Line Read Write V Buffer
    if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::deblockLineVBuffer))
    {
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockLineVBuffer, &avpBufSizeParam));
        allocParams.dwBytes         = avpBufSizeParam.bufferSize;
        allocParams.pBufName        = "Deblocker Filter Line Read Write V Buffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_deblockerFilterLineReadWriteVBuffer = m_allocator->AllocateResource(allocParams, false);
    }

    // Super-Res Tile Column Read/Write Y Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::superResTileColYBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Super Res Tile Column Read Write Y Buffer ";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_superResTileColumnReadWriteYBuffer = m_allocator->AllocateResource(allocParams, false);

    // Super-Res Tile Column Read/Write U Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::superResTileColUBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Super Res Tile Column Read Write U Buffer ";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_superResTileColumnReadWriteUBuffer = m_allocator->AllocateResource(allocParams, false);

    // Super-Res Tile Column Read/Write V Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::superResTileColVBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Super Res Tile Column Read Write V Buffer ";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_superResTileColumnReadWriteVBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Column Read/Write Y Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::lrTileColYBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Column Read Write Y Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileColumnReadWriteYBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Column Read/Write U Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::lrTileColUBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Column Read Write U Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileColumnReadWriteUBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Column Read/Write V Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::lrTileColVBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Column Read Write V Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileColumnReadWriteVBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Column Alignment Read Write Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::lrTileColAlignBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Column Alignment Read Write Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileColumnAlignmentBuf = m_allocator->AllocateResource(allocParams, false);

    // Decoded Frame Status/Error Buffer Base Address
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::frameStatusErrBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Decoded Frame Status Error Buffer Base Address";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_decodedFrameStatusErrorBuffer = m_allocator->AllocateResource(allocParams, false);

    // Decoded Block Data Streamout Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::dbdStreamoutBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Decoded Block Data Streamout Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_decodedBlockDataStreamoutBuffer = m_allocator->AllocateResource(allocParams, false);

    // Tile Statistics Streamout Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::tileStatStreamOutBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Tile Statistics Streamout Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_tileStatisticsPakStreamoutBuffer = m_allocator->AllocateResource(allocParams, false);

    // CU Streamout Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cuStreamoutBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "CU Streamout Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_cuStreamoutBuffer = m_allocator->AllocateResource(allocParams, false);

    // SSE Line Read/Write Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::sseLineBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "SSE Line Read Write Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_sseLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

    // SSE Tile Line Read/Write Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::sseTileLineBuffer, &avpBufSizeParam));
    allocParams.dwBytes          = avpBufSizeParam.bufferSize;
    allocParams.pBufName         = "SSE Tile Line Read Write Buffer";
    allocParams.ResUsageType     = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_sseTileLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::RegisterPostCdef()
{
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_Y;
    allocParamsForBuffer2D.Format   = Format_NV12;
    allocParamsForBuffer2D.dwWidth  = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, av1SuperBlockWidth);
    allocParamsForBuffer2D.dwHeight = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, av1SuperBlockHeight);
#ifdef _MMC_SUPPORTED     
    ENCODE_CHK_NULL_RETURN(m_mmcState);
    if (m_mmcState->IsMmcEnabled())
    {
        allocParamsForBuffer2D.CompressionMode = MOS_MMC_MC;
        allocParamsForBuffer2D.bIsCompressible = true;
    }
#endif   
    if (m_basicFeature->m_is10Bit)
    {
        // This is temporary fix for Sim specific ( Grits Utility) issue, HW has no restriction for current platform
        allocParamsForBuffer2D.dwWidth = MOS_ALIGN_CEIL(allocParamsForBuffer2D.dwWidth, 32) * 2;
    }
    allocParamsForBuffer2D.pBufName = "postCdefReconSurface";
    allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_trackedBuf->RegisterParam(encode::BufferType::postCdefReconSurface, allocParamsForBuffer2D));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::Submit(
    MOS_COMMAND_BUFFER *commandBuffer,
    uint8_t             packetPhase)
{
    ENCODE_FUNC_CALL();

    MOS_COMMAND_BUFFER &cmdBuffer = *commandBuffer;
    ENCODE_CHK_STATUS_RETURN(Mos_Solo_PreProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

    // Ensure the input is ready to be read.
    // Currently, mos RegisterResource has sync limitation for Raw resource.
    // Temporaly, call Resource Wait to do the sync explicitly.
    // TODO, Refine it when MOS refactor ready.
    MOS_SYNC_PARAMS syncParams;
    syncParams                  = g_cInitSyncParams;
    syncParams.GpuContext       = m_osInterface->pfnGetGpuContext(m_osInterface);
    syncParams.presSyncResource = &m_basicFeature->m_rawSurface.OsResource;
    syncParams.bReadOnly        = true;
    ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
    m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

    ENCODE_CHK_STATUS_RETURN(RegisterPostCdef());

    ENCODE_CHK_STATUS_RETURN(PatchPictureLevelCommands(packetPhase, cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(PatchTileLevelCommands(cmdBuffer, packetPhase));

    ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));
#if USE_CODECHAL_DEBUG_TOOL
    ENCODE_CHK_STATUS_RETURN(DumpStatistics());
#endif  // USE_CODECHAL_DEBUG_TOOL
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

    SetPerfTag();

    const bool firstTaskInPhase = packetPhase & firstPacket;
    const auto brcFeature       = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    // if BRC is enabled, MI_FORCE_WAKEUP and prolog commands are added by BRC packet
    if (firstTaskInPhase || !brcFeature->IsBRCEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(AddForceWakeup(cmdBuffer));

        // Send command buffer header at the beginning (OS dependent)
        ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));
    }

    if (m_pipeline->GetPipeNum() >= 2)
    {
        auto scalability = m_pipeline->GetMediaScalability();

        ENCODE_CHK_STATUS_RETURN(scalability->SyncPipe(syncOtherPipesForOne, 0, &cmdBuffer));
    }

    ENCODE_CHK_STATUS_RETURN(AddCondBBEndFor2ndPass(cmdBuffer));

    if (m_pipeline->GetPipeNum() >= 2 && m_pipeline->IsFirstPipe())
    {
        PMOS_RESOURCE bsSizeBuf = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
        ENCODE_CHK_NULL_RETURN(bsSizeBuf);
        // clear bitstream size buffer at first tile
        auto& miStoreDataParams = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        miStoreDataParams = {};
        miStoreDataParams.pOsResource = bsSizeBuf;
        miStoreDataParams.dwResourceOffset = 0;
        miStoreDataParams.dwValue = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));
    }

    if (m_pipeline->IsFirstPipe())
    {
        ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));
    }
    else {
        // add perf record for other pipes - first pipe perf record within StartStatusReport
        MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd(
            (void*)m_pipeline, m_osInterface, m_miItf, &cmdBuffer));
    }

    ENCODE_CHK_STATUS_RETURN(AddPictureVdencCommands(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::Construct3rdLevelBatch()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //To be added. When BRC is enabled, some of the commands
    //will be added into 3rd level batch

    return eStatus;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::AddOneTileCommands(
    MOS_COMMAND_BUFFER &cmdBuffer,
    uint32_t            tileRow,
    uint32_t            tileCol,
    uint32_t            tileRowPass)
{
    ENCODE_FUNC_CALL();
    auto eStatus = MOS_STATUS_SUCCESS;

    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);

    // Begin patching tile level batch cmds
    MOS_COMMAND_BUFFER constructTileBatchBuf = {};
    PMOS_COMMAND_BUFFER tempCmdBuffer = &cmdBuffer;
    // Add batch buffer start for tile
    PMHW_BATCH_BUFFER tileLevelBatchBuffer = nullptr;

    if (!m_osInterface->bUsesPatchList)
    {
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, BeginPatchTileLevelBatch,
            tileRowPass, constructTileBatchBuf);

        // Add batch buffer start for tile
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileLevelBatchBuffer,
            tileLevelBatchBuffer);
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, tileLevelBatchBuffer));

        tempCmdBuffer = &constructTileBatchBuf;
    }

    auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);
    auto vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
    auto slbbData                 = brcFeature->GetSLBData();

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_PIPE_MODE_SELECT(tempCmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SURFACE_STATE(tempCmdBuffer));

    SETPAR_AND_ADDCMD(AVP_PIPE_BUF_ADDR_STATE, m_avpItf, tempCmdBuffer);
    SETPAR_AND_ADDCMD(AVP_IND_OBJ_BASE_ADDR_STATE, m_avpItf, tempCmdBuffer);

    if (brcFeature->IsBRCEnabled())
    {
        bool     firstTileInGroup = false;
        uint32_t tileGroupIdx     = 0;
        RUN_FEATURE_INTERFACE_NO_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, IsFirstTileInGroup, firstTileInGroup, tileGroupIdx);
        vdenc2ndLevelBatchBuffer->dwOffset = firstTileInGroup ? slbbData.avpPicStateOffset : slbbData.secondAvpPicStateOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(tempCmdBuffer, vdenc2ndLevelBatchBuffer));
    }
    else
    {
        SETPAR_AND_ADDCMD(AVP_PIC_STATE, m_avpItf, tempCmdBuffer);
    }

    SETPAR_AND_ADDCMD(AVP_INTER_PRED_STATE, m_avpItf, tempCmdBuffer);

    if (brcFeature->IsBRCEnabled())
    {
        vdenc2ndLevelBatchBuffer->dwOffset = slbbData.avpSegmentStateOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(tempCmdBuffer, vdenc2ndLevelBatchBuffer));
    }
    else
    {
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SEGMENT_STATE(tempCmdBuffer));
        SETPAR_AND_ADDCMD(AVP_INLOOP_FILTER_STATE, m_avpItf, tempCmdBuffer);
    }

    SETPAR_AND_ADDCMD(AVP_TILE_CODING, m_avpItf, tempCmdBuffer);

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_PAK_INSERT_OBJECT(tempCmdBuffer));

    SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, tempCmdBuffer);

    if (brcFeature->IsBRCEnabled())
    {
        vdenc2ndLevelBatchBuffer->dwOffset = slbbData.vdencCmd1Offset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(tempCmdBuffer, vdenc2ndLevelBatchBuffer));
    }
    else
    {
        SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, tempCmdBuffer);
    }

    SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, tempCmdBuffer);

    if (brcFeature->IsBRCEnabled())
    {
        vdenc2ndLevelBatchBuffer->dwOffset = slbbData.vdencCmd2Offset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(tempCmdBuffer, vdenc2ndLevelBatchBuffer));
    }
    else
    {
        SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, tempCmdBuffer);
    }

    SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencItf, tempCmdBuffer);

    m_basicFeature->m_flushCmd = Av1BasicFeature::waitVdenc;
    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, tempCmdBuffer);

#if _MEDIA_RESERVED
    AddCommandsExt(*tempCmdBuffer);
#endif  // !(_MEDIA_RESERVED)
    ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(*tempCmdBuffer));

    if (!m_osInterface->bUsesPatchList)
    {
        // For 2nd level BB, we must use tileLevelBatchBuffer to prevent adding Epilogue before MI_BATCH_BUFFER_END
        ENCODE_CHK_NULL_RETURN(tileLevelBatchBuffer);
        tileLevelBatchBuffer->iCurrent   = constructTileBatchBuf.iOffset;
        tileLevelBatchBuffer->iRemaining = constructTileBatchBuf.iRemaining;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_END)(nullptr, tileLevelBatchBuffer));
    }

#if USE_CODECHAL_DEBUG_TOOL
    std::string             name           = std::to_string(tileRow) + std::to_string(tileCol) + std::to_string(tileRowPass) + "_TILE_CMD_BUFFER";
    CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpCmdBuffer(
        &constructTileBatchBuf,
        CODECHAL_NUM_MEDIA_STATES,
        name.c_str()));
#endif

    // End patching tile level batch cmds
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, EndPatchTileLevelBatch);

    if (tileRowPass != 1) // for dummy tile, donnot calculate tile size into frame size.
    {
        if (m_pipeline->GetPipeNum() > 1)
        {
            ENCODE_CHK_STATUS_RETURN(ReadPakMmioRegistersAtomic(&cmdBuffer));
        }
        else
        {
            ENCODE_CHK_STATUS_RETURN(ReadPakMmioRegisters(&cmdBuffer, tileRow == 0 && tileCol == 0));
        }
    }

    return eStatus;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    // Send MI_FLUSH command
    auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams                               = {};
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::UpdateUserFeatureKey(PMOS_SURFACE surface)
{
    if (m_userFeatureUpdated_post_cdef)
    {
        return MOS_STATUS_SUCCESS;
    }
    m_userFeatureUpdated_post_cdef = true;

    ReportUserSetting(
        m_userSettingPtr,
        "AV1 Post CDEF Recon Compressible",
        surface->bCompressible,
        MediaUserSetting::Group::Sequence);
    ReportUserSetting(
        m_userSettingPtr,
        "AV1 Post CDEF Recon Compress Mode",
        surface->MmcState,
        MediaUserSetting::Group::Sequence);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Construct3rdLevelBatch());

    uint16_t numTileColumns = 1;
    uint16_t numTileRows    = 1;
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

    ENCODE_CHK_NULL_RETURN(m_pipeline);
    ENCODE_CHK_NULL_RETURN(m_av1PicParams);
    if (!m_pipeline->IsDualEncEnabled())
    {
        for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
        {
            for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
            {
                ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                    cmdBuffer,
                    tileRow,
                    tileCol));
            }
        }
    }
    else
    {
        if (numTileRows != 1)  // dual encode only support column based workload submission
        {
            ENCODE_ASSERTMESSAGE("dual encode cannot support multi rows submission yet.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        uint8_t dummyIdx = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetDummyIdx, dummyIdx);
        if (m_pipeline->GetCurrentPipe() == 0)
        {
            for (auto i = 0; i < dummyIdx; i++)
            {
                ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                    cmdBuffer,
                    0,
                    i));
            }
            ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                cmdBuffer,
                0,
                dummyIdx,
                1));
        }
        else
        {
            for (auto i = dummyIdx; i < numTileColumns; i++)
            {
                ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                    cmdBuffer,
                    0,
                    i));
            }
        }
    }

    m_basicFeature->m_flushCmd = Av1BasicFeature::waitAvp;
    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);

    ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

    // Wait all pipe cmds done for the packet
    auto scalability = m_pipeline->GetMediaScalability();
    ENCODE_CHK_STATUS_RETURN(scalability->SyncPipe(syncOnePipeWaitOthers, 0, &cmdBuffer));

    if (m_pipeline->IsFirstPipe())
    {
        for (auto i = 0; i < m_pipeline->GetPipeNum(); ++i)
        {
            ENCODE_CHK_STATUS_RETURN(scalability->ResetSemaphore(syncOnePipeWaitOthers, i, &cmdBuffer));
        }
        ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, &cmdBuffer));
    }
    else {
        // add perf record for other pipes - first pipe perf record within EndStatusReport
        MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
            (void*)m_pipeline, m_osInterface, m_miItf, &cmdBuffer));
    }

    auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    if (Mos_Solo_Extension((MOS_CONTEXT_HANDLE)m_osInterface->pOsContext))
    {
        ENCODE_CHK_STATUS_RETURN(MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
    }
    else if (brcFeature->IsBRCEnabled() && m_osInterface->bInlineCodecStatusUpdate)
    {
        ENCODE_CHK_STATUS_RETURN(UpdateStatusReport(statusReportGlobalCount, &cmdBuffer));
    }
    else if (m_pipeline->IsLastPass() && m_pipeline->IsFirstPipe())
    {
        ENCODE_CHK_STATUS_RETURN(MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
    }

    if (m_pipeline->IsDualEncEnabled())
    {
        SETPAR_AND_ADDCMD(VDENC_CONTROL_STATE, m_vdencItf, &cmdBuffer);
    }

    CODECHAL_DEBUG_TOOL(
        if (m_mmcState) {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_reconSurface));
        })

    if (m_av1SeqParams->CodingToolFlags.fields.enable_superres)
    {
        PCODEC_REF_LIST currRefList     = m_basicFeature->m_ref.GetCurrRefList();
        MOS_SURFACE *postCdefSurface = m_basicFeature->m_trackedBuf->GetSurface(
            BufferType::postCdefReconSurface, currRefList->ucScalingIdx);

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState) { UpdateUserFeatureKey(postCdefSurface); })
    }

    UpdateParameters();

    return MOS_STATUS_SUCCESS;
}

void Av1VdencPktXe_Lpm_Plus_Base::UpdateParameters()
{
    ENCODE_FUNC_CALL();

    Av1VdencPkt::UpdateParameters();

    if (!m_pipeline->IsSingleTaskPhaseSupported())
    {
        m_osInterface->pfnResetPerfBufferID(m_osInterface);
    }
}


MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1VdencPktXe_Lpm_Plus_Base)
{
    params.notFirstPass = !m_pipeline->IsFirstPass();

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_IND_OBJ_BASE_ADDR_STATE, Av1VdencPktXe_Lpm_Plus_Base)
{
    params.mvObjectOffset = m_mvOffset;
    params.mvObjectSize   = m_basicFeature->m_mbCodeSize - m_mvOffset;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_TILE_CODING, Av1VdencPktXe_Lpm_Plus_Base)
{
    uint32_t tileIdx = 0;
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileIdx, tileIdx);
    params.disableFrameContextUpdateFlag = m_av1PicParams->PicFlags.fields.disable_frame_end_update_cdf || (tileIdx != m_av1PicParams->context_update_tile_id);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::AddAllCmds_AVP_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_osInterface);
    auto &params = m_avpItf->MHW_GETPAR_F(AVP_PAK_INSERT_OBJECT)();
    params       = {};

    auto GetExtraData = [&]() { return params.bsBuffer->pBase + params.offset; };
    auto GetExtraSize = [&]() { return (params.bitSize + 7) >> 3; };

    // First, Send all other OBU bit streams other than tile group OBU when it's first tile in frame
    uint32_t tileIdx    = 0;
    bool     tgOBUValid = m_basicFeature->m_slcData[0].BitSize > 0 ? true : false;

    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileIdx, tileIdx);
    auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    if (tileIdx == 0)
    {
        uint32_t nalNum = 0;
        for (uint32_t i = 0; i < MAX_NUM_OBU_TYPES && m_nalUnitParams[i]->uiSize > 0; i++)
        {
            nalNum = i;
        }

        params.bsBuffer             = &m_basicFeature->m_bsBuffer;
        params.endOfHeaderInsertion = false;

        // Support multiple packed header buffer
        for (uint32_t i = 0; i <= nalNum; i++)
        {
            uint32_t nalUnitSize   = m_nalUnitParams[i]->uiSize;
            uint32_t nalUnitOffset = m_nalUnitParams[i]->uiOffset;

            ENCODE_ASSERT(nalUnitSize < CODECHAL_ENCODE_AV1_PAK_INSERT_UNCOMPRESSED_HEADER);

            params.bitSize    = nalUnitSize * 8;
            params.offset     = nalUnitOffset;
            params.lastHeader = !tgOBUValid && (i == nalNum);

            if (IsFrameHeader(*(m_basicFeature->m_bsBuffer.pBase + nalUnitOffset)))
            {
                if (brcFeature->IsBRCEnabled())
                {
                    auto pakInsertOutputBatchBuffer = brcFeature->GetPakInsertOutputBatchBuffer(m_pipeline->m_currRecycledBufIdx);
                    ENCODE_CHK_NULL_RETURN(pakInsertOutputBatchBuffer);
                    // send pak insert obj cmds after back annotation
                    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(cmdBuffer, pakInsertOutputBatchBuffer));
                }
                else
                {
                    m_avpItf->MHW_ADDCMD_F(AVP_PAK_INSERT_OBJECT)(cmdBuffer);
                    m_osInterface->pfnAddCommand(cmdBuffer, GetExtraData(), GetExtraSize());
                }
            }
            else
            {
                m_avpItf->MHW_ADDCMD_F(AVP_PAK_INSERT_OBJECT)(cmdBuffer);
                m_osInterface->pfnAddCommand(cmdBuffer, GetExtraData(), GetExtraSize());
            }
        }
    }

    // Second, Send tile group OBU when it is first tile in tile group
    if (tgOBUValid)
    {
        ENCODE_CHK_NULL_RETURN(m_featureManager);

        auto tileFeature = dynamic_cast<Av1EncodeTile *>(m_featureManager->GetFeature(Av1FeatureIDs::encodeTile));
        ENCODE_CHK_NULL_RETURN(tileFeature);

        MHW_CHK_STATUS_RETURN(tileFeature->MHW_SETPAR_F(AVP_PAK_INSERT_OBJECT)(params));
        if (params.bitSize)
        {
            m_avpItf->MHW_ADDCMD_F(AVP_PAK_INSERT_OBJECT)(cmdBuffer);
            m_osInterface->pfnAddCommand(cmdBuffer, GetExtraData(), GetExtraSize());
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::AddAllCmds_AVP_PIPE_MODE_SELECT(PMOS_COMMAND_BUFFER cmdBuffer) const
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(cmdBuffer);

    auto &vdControlStateParams          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
    vdControlStateParams                = {};
    vdControlStateParams.initialization = true;
    vdControlStateParams.avpEnabled     = true;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(cmdBuffer));

    SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, cmdBuffer);
    SETPAR_AND_ADDCMD(AVP_PIPE_MODE_SELECT, m_avpItf, cmdBuffer);
    SETPAR_AND_ADDCMD(MFX_WAIT, m_miItf, cmdBuffer);

    if (m_pipeline->GetPipeNum() > 1)
    {
        vdControlStateParams                      = {};
        vdControlStateParams.avpEnabled           = true;
        vdControlStateParams.scalableModePipeLock = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(cmdBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::AddAllCmds_AVP_SEGMENT_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(cmdBuffer);
    ENCODE_CHK_NULL_RETURN(m_featureManager);

    auto &par = m_avpItf->MHW_GETPAR_F(AVP_SEGMENT_STATE)();
    par      = {};

    auto segmentFeature = dynamic_cast<Av1Segmentation *>(m_featureManager->GetFeature(Av1FeatureIDs::av1Segmentation));
    ENCODE_CHK_NULL_RETURN(segmentFeature);

    MHW_CHK_STATUS_RETURN(segmentFeature->MHW_SETPAR_F(AVP_SEGMENT_STATE)(par));

    const bool segmentEnabled = par.av1SegmentParams.m_enabled;

    for (uint8_t i = 0; i < av1MaxSegments; i++)
    {
        par.currentSegmentId = i;
        m_avpItf->MHW_ADDCMD_F(AVP_SEGMENT_STATE)(cmdBuffer);

        // If segmentation is not enabled, then AV1_SEGMENT_STATE must still be sent once for SegmentID = 0
        // If i == numSegments -1, means all segments are issued, break the loop
        if (!segmentEnabled || (i == par.numSegments - 1))
        {
            break;
        }
    }

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Av1VdencPktXe_Lpm_Plus_Base::DumpStatistics()
{
    CodechalDebugInterface* debugInterface = m_pipeline->GetStatusReportDebugInterface();
    ENCODE_CHK_NULL_RETURN(debugInterface);

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        m_basicFeature->m_tileStatisticsPakStreamoutBuffer,
        CodechalDbgAttr::attrTileBasedStats,
        "Pak_Tile_Stats",
        512,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    MOS_RESOURCE* tileStatisticsBuffer = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, FeatureIDs::encodeTile, GetTileBasedStatisticsBuffer, 0, tileStatisticsBuffer);

    ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        tileStatisticsBuffer,
        CodechalDbgAttr::attrFrameState,
        "VDEnc_Frame_Stats",
        m_hwInterface->m_pakIntTileStatsSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    return MOS_STATUS_SUCCESS;
}
#endif  // USE_CODECHAL_DEBUG_TOOL
}  // namespace encode
