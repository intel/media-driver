/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     encode_av1_vdenc_packet_xe2_lpm_base.cpp
//! \brief    Defines the interface for av1 encode vdenc packet of Xe2_LPM+
//!
#include "encode_av1_vdenc_packet_xe2_lpm_base.h"
#include "encode_av1_pipeline.h"
#include "encode_av1_basic_feature.h"
#include "encode_av1_segmentation.h"
#include "encode_status_report_defs.h"
#include "mos_solo_generic.h"
#include "encode_status_report_defs.h"
#include "encode_tile.h"
#include "mhw_vdbox_avp_itf.h"
#include "encode_av1_tile.h"
#include "encode_av1_scc.h"
#include "encode_utils.h"
#include "encode_av1_vdenc_packet.h"
#include "encode_av1_superres.h"
#include "hal_oca_interface_next.h"

namespace encode
{

MOS_STATUS Av1VdencPktXe2_Lpm_Base::Init()
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

MOS_STATUS Av1VdencPktXe2_Lpm_Base::AllocateResources()
{
    ENCODE_FUNC_CALL();

    mhw::vdbox::avp::AvpBufferSizePar avpBufSizeParam;
    MOS_ZeroMemory(&avpBufSizeParam, sizeof(avpBufSizeParam));
    avpBufSizeParam.bitDepthIdc      = (m_basicFeature->m_bitDepth - 8) >> 1;
    avpBufSizeParam.height           = CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, av1SuperBlockHeight);
    avpBufSizeParam.width            = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, av1SuperBlockWidth);
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
        ENCODE_ASSERTMESSAGE("Failed to allocate Mfd Intra Row Store Buffer.");
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
    if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::AvpBufferType::bsdLineBuffer))
    {
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::bsdLineBuffer, &avpBufSizeParam));
        allocParams.dwBytes         = avpBufSizeParam.bufferSize;
        allocParams.pBufName        = "Bitstream Decoder Encoder Line Rowstore Read Write buffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);
    }
    
    // Intra Prediction Tile Line Rowstore Read/Write Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::intraPredTileLineBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Intra Prediction Tile Line Rowstore Read Write Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_intraPredictionTileLineRowstoreReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

    // Spatial motion vector Line rowstore buffer
    if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::AvpBufferType::spatialMvLineBuffer))
    {
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::spatialMvLineBuffer, &avpBufSizeParam));
        allocParams.dwBytes         = avpBufSizeParam.bufferSize;
        allocParams.pBufName        = "Spatial motion vector Line rowstore buffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_spatialMotionVectorLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);
    }

    // Spatial motion vector Tile Line Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::spatialMvTileLineBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Spatial motion vector Tile Line Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_spatialMotionVectorCodingTileLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Meta Tile Column Read/Write Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::lrMetaTileColBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Meta Tile Column Read Write Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationMetaTileColumnReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Read/Write Line Y Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::lrTileLineYBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Read Write Line Y Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileReadWriteLineYBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Read/Write Line U Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::lrTileLineUBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Read Write Line U Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileReadWriteLineUBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Read Write Line V Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::lrTileLineVBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Read Write Line V Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileReadWriteLineVBuffer = m_allocator->AllocateResource(allocParams, false);

    // Deblocker Filter Line Read Write Y Buffer
    if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::deblockLineYBuffer))
    {
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::deblockLineYBuffer, &avpBufSizeParam));
        allocParams.dwBytes         = avpBufSizeParam.bufferSize;
        allocParams.pBufName        = "Deblocker Filter Line Read Write Y Buffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_deblockerFilterLineReadWriteYBuffer = m_allocator->AllocateResource(allocParams, false);
    }

    // Deblocker Filter Line Read Write U Buffer
    if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::deblockLineUBuffer))
    {
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::deblockLineUBuffer, &avpBufSizeParam));
        allocParams.dwBytes         = avpBufSizeParam.bufferSize;
        allocParams.pBufName        = "Deblocker Filter Line Read Write U Buffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_deblockerFilterLineReadWriteUBuffer = m_allocator->AllocateResource(allocParams, false);
    }

    // Deblocker Filter Line Read Write V Buffer
    if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::deblockLineVBuffer))
    {
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::deblockLineVBuffer, &avpBufSizeParam));
        allocParams.dwBytes         = avpBufSizeParam.bufferSize;
        allocParams.pBufName        = "Deblocker Filter Line Read Write V Buffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_basicFeature->m_deblockerFilterLineReadWriteVBuffer = m_allocator->AllocateResource(allocParams, false);
    }

    // Super-Res Tile Column Read/Write Y Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::superResTileColYBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Super Res Tile Column Read Write Y Buffer ";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_superResTileColumnReadWriteYBuffer = m_allocator->AllocateResource(allocParams, false);

    // Super-Res Tile Column Read/Write U Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::superResTileColUBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Super Res Tile Column Read Write U Buffer ";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_superResTileColumnReadWriteUBuffer = m_allocator->AllocateResource(allocParams, false);

    // Super-Res Tile Column Read/Write V Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::superResTileColVBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Super Res Tile Column Read Write V Buffer ";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_superResTileColumnReadWriteVBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Column Read/Write Y Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::lrTileColYBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Column Read Write Y Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileColumnReadWriteYBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Column Read/Write U Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::lrTileColUBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Column Read Write U Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileColumnReadWriteUBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Column Read/Write V Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::lrTileColVBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Column Read Write V Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileColumnReadWriteVBuffer = m_allocator->AllocateResource(allocParams, false);

    // Loop Restoration Filter Tile Column Alignment Read Write Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::lrTileColAlignBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Loop Restoration Filter Tile Column Alignment Read Write Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_loopRestorationFilterTileColumnAlignmentBuf = m_allocator->AllocateResource(allocParams, false);

    // Decoded Frame Status/Error Buffer Base Address
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::frameStatusErrBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Decoded Frame Status Error Buffer Base Address";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_decodedFrameStatusErrorBuffer = m_allocator->AllocateResource(allocParams, false);

    // Decoded Block Data Streamout Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::dbdStreamoutBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Decoded Block Data Streamout Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_decodedBlockDataStreamoutBuffer = m_allocator->AllocateResource(allocParams, false);

    // Tile Statistics Streamout Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::tileStatStreamOutBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "Tile Statistics Streamout Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_tileStatisticsPakStreamoutBuffer = m_allocator->AllocateResource(allocParams, false);

    // CU Streamout Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::cuStreamoutBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "CU Streamout Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    m_basicFeature->m_cuStreamoutBuffer = m_allocator->AllocateResource(allocParams, false);

    // SSE Line Read/Write Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::sseLineBuffer, &avpBufSizeParam));
    allocParams.dwBytes         = avpBufSizeParam.bufferSize;
    allocParams.pBufName        = "SSE Line Read Write Buffer";
    allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_sseLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

    // SSE Tile Line Read/Write Buffer
    ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::AvpBufferType::sseTileLineBuffer, &avpBufSizeParam));
    allocParams.dwBytes          = avpBufSizeParam.bufferSize;
    allocParams.pBufName         = "SSE Tile Line Read Write Buffer";
    allocParams.ResUsageType     = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_basicFeature->m_sseTileLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1VdencPktXe2_Lpm_Base::RegisterPostCdef()
{
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_Y;
    allocParamsForBuffer2D.Format   = Format_NV12;
    allocParamsForBuffer2D.dwWidth  = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, av1SuperBlockWidth);
    allocParamsForBuffer2D.dwHeight = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, av1SuperBlockHeight);
    if (m_mmcState->IsMmcEnabled())
    {
        allocParamsForBuffer2D.CompressionMode = MOS_MMC_MC;
        allocParamsForBuffer2D.bIsCompressible = true;
    }
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

MOS_STATUS Av1VdencPktXe2_Lpm_Base::AddOneTileCommands(
    MOS_COMMAND_BUFFER &cmdBuffer,
    uint32_t            tileRow,
    uint32_t            tileCol,
    uint32_t            tileRowPass)
{
    ENCODE_FUNC_CALL();
    auto eStatus = MOS_STATUS_SUCCESS;

    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);

    //update IBC status for current tile
    RUN_FEATURE_INTERFACE_RETURN(Av1Scc, Av1FeatureIDs::av1Scc, UpdateIBCStatusForCurrentTile);
    
    if ((m_pipeline->GetPipeNum() > 1) && (tileCol != m_pipeline->GetCurrentPipe()))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Begin patching tile level batch cmds
    MOS_COMMAND_BUFFER constructTileBatchBuf = {};
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, BeginPatchTileLevelBatch, tileRowPass, constructTileBatchBuf);

    // Add batch buffer start for tile
    PMHW_BATCH_BUFFER tileLevelBatchBuffer = nullptr;
    RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileLevelBatchBuffer, tileLevelBatchBuffer);
    ENCODE_CHK_NULL_RETURN(tileLevelBatchBuffer);
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, tileLevelBatchBuffer));

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_PIPE_MODE_SELECT(&constructTileBatchBuf));

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SURFACE_STATE(&constructTileBatchBuf));

    SETPAR_AND_ADDCMD(AVP_PIPE_BUF_ADDR_STATE, m_avpItf, &constructTileBatchBuf);
    SETPAR_AND_ADDCMD(AVP_IND_OBJ_BASE_ADDR_STATE, m_avpItf, &constructTileBatchBuf);
    SETPAR_AND_ADDCMD(AVP_PIC_STATE, m_avpItf, &constructTileBatchBuf);

    SETPAR_AND_ADDCMD(AVP_INTER_PRED_STATE, m_avpItf, &constructTileBatchBuf);

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SEGMENT_STATE(&constructTileBatchBuf));

    SETPAR_AND_ADDCMD(AVP_INLOOP_FILTER_STATE, m_avpItf, &constructTileBatchBuf);
    SETPAR_AND_ADDCMD(AVP_TILE_CODING, m_avpItf, &constructTileBatchBuf);

    ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_PAK_INSERT_OBJECT(&constructTileBatchBuf));

    SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, &constructTileBatchBuf);
    SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, &constructTileBatchBuf);
    SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, &constructTileBatchBuf);
    SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, &constructTileBatchBuf);
    SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencItf, &constructTileBatchBuf);

    m_basicFeature->m_flushCmd = Av1BasicFeature::waitVdenc;
    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &constructTileBatchBuf);

    ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(constructTileBatchBuf));
    // For 2nd level BB, we must use tileLevelBatchBuffer to prevent adding Epilogue before MI_BATCH_BUFFER_END
    tileLevelBatchBuffer->iCurrent   = constructTileBatchBuf.iOffset;
    tileLevelBatchBuffer->iRemaining = constructTileBatchBuf.iRemaining;
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_END)(nullptr, tileLevelBatchBuffer));
    HalOcaInterfaceNext::OnSubLevelBBStart(
        cmdBuffer,
        m_osInterface->pOsContext,
        &constructTileBatchBuf.OsResource,
        0,
        false,
        constructTileBatchBuf.iOffset);

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

    ENCODE_CHK_STATUS_RETURN(ReadPakMmioRegisters(&cmdBuffer, tileRow == 0 && tileCol == 0));

    ENCODE_CHK_STATUS_RETURN(PrepareHWMetaDataFromStreamoutTileLevel(&cmdBuffer, tileCol, tileRow));

    return eStatus;
}

MOS_STATUS Av1VdencPktXe2_Lpm_Base::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
{
    ENCODE_FUNC_CALL();

    // Send MI_FLUSH command
    auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams                               = {};
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    auto *skuTable                              = m_hwInterface->GetSkuTable();
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
    {
        // Add PPC fulsh
        flushDwParams.bEnablePPCFlush = true;
    }
    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
