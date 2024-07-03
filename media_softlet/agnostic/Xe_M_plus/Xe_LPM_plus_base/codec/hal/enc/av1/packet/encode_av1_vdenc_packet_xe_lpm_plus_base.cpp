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
#include "hal_oca_interface_next.h"

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
        MHW_MI_MMIOREGISTERS mmioRegister;
        if (m_vdencItf->ConvertToMiRegister(MHW_VDBOX_NODE_1, mmioRegister))
        {
            HalOcaInterfaceNext::On1stLevelBBStart(
                *tempCmdBuffer,
                (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext,
                m_osInterface->CurrentGpuContextHandle,
                m_miItf,
                mmioRegister);
        }
    }

    auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);
    auto vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
    auto slbbData                 = brcFeature->GetSLBData();

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_PIPE_MODE_SELECT(tempCmdBuffer));

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SURFACE_STATE(tempCmdBuffer));

    SETPAR_AND_ADDCMD(AVP_PIPE_BUF_ADDR_STATE, m_avpItf, tempCmdBuffer);
    SETPAR_AND_ADDCMD(AVP_IND_OBJ_BASE_ADDR_STATE, m_avpItf, tempCmdBuffer);
    bool firstTileInGroup = false;
    if (brcFeature->IsBRCEnabled())
    {
        uint32_t tileGroupIdx     = 0;
        RUN_FEATURE_INTERFACE_NO_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, IsFirstTileInGroup, firstTileInGroup, tileGroupIdx);
        vdenc2ndLevelBatchBuffer->dwOffset = firstTileInGroup ? slbbData.avpPicStateOffset : slbbData.secondAvpPicStateOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(tempCmdBuffer, vdenc2ndLevelBatchBuffer));
        HalOcaInterfaceNext::OnSubLevelBBStart(
            *tempCmdBuffer,
            m_osInterface->pOsContext,
            &vdenc2ndLevelBatchBuffer->OsResource,
            vdenc2ndLevelBatchBuffer->dwOffset,
            false,
            slbbData.slbSize - vdenc2ndLevelBatchBuffer->dwOffset);
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
        HalOcaInterfaceNext::OnSubLevelBBStart(
            *tempCmdBuffer,
            m_osInterface->pOsContext,
            &vdenc2ndLevelBatchBuffer->OsResource,
            vdenc2ndLevelBatchBuffer->dwOffset,
            false,
            slbbData.vdencCmd1Offset - vdenc2ndLevelBatchBuffer->dwOffset);
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
        HalOcaInterfaceNext::OnSubLevelBBStart(
            *tempCmdBuffer,
            m_osInterface->pOsContext,
            &vdenc2ndLevelBatchBuffer->OsResource,
            vdenc2ndLevelBatchBuffer->dwOffset,
            false,
            slbbData.vdencCmd2Offset - vdenc2ndLevelBatchBuffer->dwOffset);
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
        HalOcaInterfaceNext::OnSubLevelBBStart(
            *tempCmdBuffer,
            m_osInterface->pOsContext,
            &vdenc2ndLevelBatchBuffer->OsResource,
            vdenc2ndLevelBatchBuffer->dwOffset,
            false,
            (firstTileInGroup ? slbbData.avpPicStateOffset : slbbData.secondAvpPicStateOffset) - vdenc2ndLevelBatchBuffer->dwOffset);
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
        HalOcaInterfaceNext::OnSubLevelBBStart(
            cmdBuffer,
            m_osInterface->pOsContext,
            &tempCmdBuffer->OsResource,
            0,
            false,
            tempCmdBuffer->iOffset);
        HalOcaInterfaceNext::On1stLevelBBEnd(*tempCmdBuffer, *m_osInterface);
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

    ENCODE_CHK_STATUS_RETURN(PrepareHWMetaDataFromStreamoutTileLevel(&cmdBuffer, tileCol, tileRow));

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
}  // namespace encode
