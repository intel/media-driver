/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_av1_vdenc_packet_xe_m_base.cpp
//! \brief    Defines the interface for av1 encode vdenc packet of xe m base
//!
#include "encode_av1_vdenc_packet_xe_m_base.h"
#include "mos_solo_generic.h"
#include "media_user_settings_mgr_g12_plus.h"
#include "mhw_mi_itf.h"

namespace encode
{
    MOS_STATUS Av1VdencPktXe_M_Base::AllocateResources()
    {
        ENCODE_FUNC_CALL();

        MediaUserSetting::Value outValue;
        ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "Lockable Resource",
            MediaUserSetting::Group::Sequence);
        m_basicFeature->m_lockableResource = outValue.Get<bool>();

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
        allocParams.Type               = MOS_GFXRES_BUFFER;
        allocParams.TileType           = MOS_TILE_LINEAR;
        allocParams.Format             = Format_Buffer;
        allocParams.Flags.bNotLockable = false;

        allocParams.dwBytes  = avpBufSizeParam.height * avpBufSizeParam.width * 4;
        allocParams.pBufName = "VDEnc Cumulative CU Count Streamout Surface";
        m_resCumulativeCuCountStreamoutBuffer = m_allocator->AllocateResource(allocParams, false);
        if (m_resCumulativeCuCountStreamoutBuffer == nullptr)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate VDEnc Cumulative CU Count Streamout Surface.");
            return MOS_STATUS_UNKNOWN;
        }

        allocParams.dwBytes  = MOS_ALIGN_CEIL(sizeof(Av1VdencPakInfo), CODECHAL_PAGE_SIZE);
        allocParams.pBufName = "VDENC BRC PakInfo";
        m_basicFeature->m_recycleBuf->RegisterResource(PakInfo, allocParams, 6);

        allocParams.dwBytes  = (m_basicFeature->m_bitDepth == 8) ? avpBufSizeParam.width * 2 * av1SuperBlockWidth * CODECHAL_CACHELINE_SIZE
            : avpBufSizeParam.width * 4 * av1SuperBlockWidth *CODECHAL_CACHELINE_SIZE; // Number of Cachelines per SB;
        allocParams.pBufName = "m_resMfdIntraRowStoreScratchBuffer";
        m_basicFeature->m_resMfdIntraRowStoreScratchBuffer = m_allocator->AllocateResource(allocParams,false);

        if (m_basicFeature->m_resMfdIntraRowStoreScratchBuffer == nullptr)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate VDEnc Tile Row Store Buffer.");
            return MOS_STATUS_UNKNOWN;
        }

        // VDENC tile row store buffer
        allocParams.dwBytes                 = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, 32) * CODECHAL_CACHELINE_SIZE * 2;
        allocParams.pBufName                = "VDENC Tile Row Store Buffer";
        m_vdencTileRowStoreBuffer           = m_allocator->AllocateResource(allocParams, false);

        if (m_vdencTileRowStoreBuffer == nullptr)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate VDEnc Tile Row Store Buffer.");
            return MOS_STATUS_UNKNOWN;
        }

        // Bitstream decode line rowstore buffer
        if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::bsdLineBuffer))
        {
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::bsdLineBuffer, &avpBufSizeParam));
            allocParams.dwBytes  = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Bitstream Decoder Encoder Line Rowstore Read Write buffer";
            m_basicFeature->m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);
        }

        // Bistream decode Tile Line rowstore buffer
        if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::bsdTileLineBuffer))
        {
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::bsdTileLineBuffer, &avpBufSizeParam));
            allocParams.dwBytes                                      = avpBufSizeParam.bufferSize;
            allocParams.pBufName                                     = "Bitstream Decoder Encoder Tile Line Rowstore Read Write buffer";
            m_basicFeature->m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);
        }

        // Intra Prediction Tile Line Rowstore Read/Write Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::intraPredLineBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Intra Prediction Tile Line Rowstore Read Write Buffer";
        m_basicFeature->m_intraPredictionTileLineRowstoreReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

        // Spatial motion vector Line rowstore buffer
        if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::spatialMvLineBuffer))
        {
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::spatialMvLineBuffer, &avpBufSizeParam));
            allocParams.dwBytes = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Spatial motion vector Line rowstore buffer";
            m_basicFeature->m_spatialMotionVectorLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);
        }

        // Spatial motion vector Tile Line Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::spatialMvTileLineBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Spatial motion vector Tile Line Buffer";
        m_basicFeature->m_spatialMotionVectorCodingTileLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

        // Deblocker Filter Line Read Write Y Buffer
        if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::deblockLineYBuffer))
        {
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockLineYBuffer, &avpBufSizeParam));
            allocParams.dwBytes = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Deblocker Filter Line Read Write Y Buffer";
            m_basicFeature->m_deblockerFilterLineReadWriteYBuffer = m_allocator->AllocateResource(allocParams, false);
        }

        // Deblocker Filter Line Read Write U Buffer
        if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::deblockLineUBuffer))
        {
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockLineUBuffer, &avpBufSizeParam));
            allocParams.dwBytes = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Deblocker Filter Line Read Write U Buffer";
            m_basicFeature->m_deblockerFilterLineReadWriteUBuffer = m_allocator->AllocateResource(allocParams, false);
        }

        // Deblocker Filter Line Read Write V Buffer
        if (!m_avpItf->IsBufferRowstoreCacheEnabled(mhw::vdbox::avp::deblockLineVBuffer))
        {
            ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockLineVBuffer, &avpBufSizeParam));
            allocParams.dwBytes = avpBufSizeParam.bufferSize;
            allocParams.pBufName = "Deblocker Filter Line Read Write V Buffer";
            m_basicFeature->m_deblockerFilterLineReadWriteVBuffer = m_allocator->AllocateResource(allocParams, false);
        }

        // Deblocker Filter Tile Line Read/Write Y Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileLineYBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Deblocker Filter Tile Line Read Write Y Buffer";
        m_basicFeature->m_deblockerFilterTileLineReadWriteYBuffer = m_allocator->AllocateResource(allocParams, false);

        // Deblocker Filter Tile Line Read/Write U Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileLineUBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Deblocker Filter Tile Line Read Write U Buffer";
        m_basicFeature->m_deblockerFilterTileLineReadWriteUBuffer = m_allocator->AllocateResource(allocParams, false);

        // Deblocker Filter Tile Line Read/Write V Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileLineVBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Deblocker Filter Tile Line Read Write V Buffer";
        m_basicFeature->m_deblockerFilterTileLineReadWriteVBuffer = m_allocator->AllocateResource(allocParams, false);

        // Deblocker Filter Tile Column Read/Write Y Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileColYBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Deblocker Filter Tile Column Read Write Y Buffer";
        m_basicFeature->m_deblockerFilterTileColumnReadWriteYBuffer = m_allocator->AllocateResource(allocParams, false);

        // Deblocker Filter Tile Column Read/Write U Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileColUBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Deblocker Filter Tile Column Read Write U Buffer";
        m_basicFeature->m_deblockerFilterTileColumnReadWriteUBuffer = m_allocator->AllocateResource(allocParams, false);

        // Deblocker Filter Tile Column Read/Write V Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::deblockTileColVBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Deblocker Filter Tile Column Read Write V Buffer";
        m_basicFeature->m_deblockerFilterTileColumnReadWriteVBuffer = m_allocator->AllocateResource(allocParams, false);

        // CDEF Filter Line Read/Write Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefLineBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "CDEF Filter Line Read Write Buffer";
        m_basicFeature->m_cdefFilterLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

        // CDEF Filter Tile Line Read/Write Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefTileLineBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "CDEF Filter Tile Line Read Write Buffer";
        m_basicFeature->m_cdefFilterTileLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

        // CDEF Filter Tile Column Read/Write Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefTileColBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "CDEF Filter Tile Column Read Write Buffer";
        m_basicFeature->m_cdefFilterTileColumnReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

        // CDEF Filter Meta Tile Line Read Write Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefMetaTileLineBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "CDEF Filter Meta Tile Line Read Write Buffer";
        m_basicFeature->m_cdefFilterMetaTileLineReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

        // CDEF Filter Meta Tile Column Read Write Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefMetaTileColBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "CDEF Filter Meta Tile Column Read Write Buffer";
        m_basicFeature->m_cdefFilterMetaTileColumnReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

        // CDEF Filter Top Left Corner Read Write Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::cdefTopLeftCornerBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "CDEF Filter Top Left Corner Read Write Buffer";
        m_basicFeature->m_cdefFilterTopLeftCornerReadWriteBuffer = m_allocator->AllocateResource(allocParams, false);

        // Decoded Frame Status/Error Buffer Base Address
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::frameStatusErrBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Decoded Frame Status Error Buffer Base Address";
        m_basicFeature->m_decodedFrameStatusErrorBuffer = m_allocator->AllocateResource(allocParams, false);

        // Decoded Block Data Streamout Buffer
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::dbdStreamoutBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Decoded Block Data Streamout Buffer";
        m_basicFeature->m_decodedBlockDataStreamoutBuffer = m_allocator->AllocateResource(allocParams, false);

        // Tile Statistics Streamout Buffer
        allocParams.Flags.bNotLockable = !(m_basicFeature->m_lockableResource);
        ENCODE_CHK_STATUS_RETURN(m_avpItf->GetAvpBufSize(mhw::vdbox::avp::tileStatStreamOutBuffer, &avpBufSizeParam));
        allocParams.dwBytes = avpBufSizeParam.bufferSize;
        allocParams.pBufName = "Tile Statistics Streamout Buffer";
        m_basicFeature->m_tileStatisticsPakStreamoutBuffer = m_allocator->AllocateResource(allocParams, false);

        if(m_osInterface->bInlineCodecStatusUpdate)
        {
            MOS_LOCK_PARAMS lockFlagsWriteOnly;
            MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
            lockFlagsWriteOnly.WriteOnly = 1;

            m_atomicScratchBuf.size = MOS_ALIGN_CEIL(sizeof(AtomicScratchBuffer), sizeof(uint64_t));

            uint32_t size  = MHW_CACHELINE_SIZE * 4 * 2; //  each set of scratch is 4 cacheline size, and allocate 2 set.
            allocParams.dwBytes  = size;
            allocParams.pBufName = "atomic sratch buffer";

            m_atomicScratchBuf.resAtomicScratchBuffer = m_allocator->AllocateResource(allocParams, false);

            ENCODE_CHK_NULL_RETURN(m_atomicScratchBuf.resAtomicScratchBuffer);

            uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
                m_osInterface,
                m_atomicScratchBuf.resAtomicScratchBuffer,
                &lockFlagsWriteOnly);

            ENCODE_CHK_NULL_RETURN(data);

            MOS_ZeroMemory(data, size);
            m_atomicScratchBuf.pData              = (uint32_t*)data;
            m_atomicScratchBuf.size               = size;
            m_atomicScratchBuf.zeroValueOffset    = 0;
            m_atomicScratchBuf.operand1Offset     = MHW_CACHELINE_SIZE;
            m_atomicScratchBuf.operand2Offset     = MHW_CACHELINE_SIZE * 2;
            m_atomicScratchBuf.operand3Offset     = MHW_CACHELINE_SIZE * 3;
            m_atomicScratchBuf.encodeUpdateIndex  = 0;
            m_atomicScratchBuf.tearDownIndex      = 1;
            m_atomicScratchBuf.operandSetSize     = MHW_CACHELINE_SIZE * 4;

            ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, m_atomicScratchBuf.resAtomicScratchBuffer));
        }

        return MOS_STATUS_SUCCESS;

    }

    MOS_STATUS Av1VdencPktXe_M_Base::RegisterPostCdef()
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
        MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBuffer2D.Type               = MOS_GFXRES_2D;
        allocParamsForBuffer2D.TileType           = MOS_TILE_Y;
        allocParamsForBuffer2D.Format             = Format_NV12;
        allocParamsForBuffer2D.Flags.bNotLockable = !(m_basicFeature->m_lockableResource);
        allocParamsForBuffer2D.dwWidth            = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, av1SuperBlockWidth);
        allocParamsForBuffer2D.dwHeight           = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, av1SuperBlockHeight);

        if (m_mmcState->IsMmcEnabled() && m_basicFeature->m_reconSurface.bCompressible)
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
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_trackedBuf->RegisterParam(encode::BufferType::postCdefReconSurface, allocParamsForBuffer2D));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::Prepare()
    {
        ENCODE_FUNC_CALL();

        Av1VdencPkt::Prepare();

        m_basicFeature->m_ref.SetPostCdefAsEncRef(true);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::Submit(
        MOS_COMMAND_BUFFER* commandBuffer,
        uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        MOS_COMMAND_BUFFER &cmdBuffer = *commandBuffer;
        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PreProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

        // Ensure the input is ready to be read.
        // Currently, mos RegisterResource has sync limitation for Raw resource.
        // Temporaly, call Resource Wait to do the sync explicitly.
        // Refine it when MOS refactor ready.
        MOS_SYNC_PARAMS       syncParams;
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);
        syncParams.presSyncResource = &m_basicFeature->m_rawSurface.OsResource;
        syncParams.bReadOnly = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

        ENCODE_CHK_STATUS_RETURN(RegisterPostCdef());
        ENCODE_CHK_STATUS_RETURN(PatchPictureLevelCommands(packetPhase, cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(PatchTileLevelCommands(cmdBuffer, packetPhase));

        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

    #if MHW_HWCMDPARSER_ENABLED
        auto instance = mhw::HwcmdParser::GetInstance();
        if (instance)
        {
            instance->ParseCmdBuf(cmdBuffer.pCmdBase, cmdBuffer.iOffset / sizeof(uint32_t));
        }
    #endif
#if USE_CODECHAL_DEBUG_TOOL
        ENCODE_CHK_STATUS_RETURN(DumpStatistics());
#endif  // USE_CODECHAL_DEBUG_TOOL
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER  &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

        uint16_t perfTag = m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE : CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE_SECOND_PASS;
        uint16_t pictureType = (m_basicFeature->m_pictureCodingType == I_TYPE) ? 1 : (m_basicFeature->m_ref.IsLowDelay() ? (m_basicFeature->m_ref.IsPFrame() ? 2 : 0) : 3);
        SetPerfTag(perfTag, (uint16_t)m_basicFeature->m_mode, pictureType);

        bool firstTaskInPhase = packetPhase & firstPacket;
        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            ENCODE_CHK_STATUS_RETURN(AddForceWakeup(cmdBuffer));

            // Send command buffer header at the beginning (OS dependent)
            ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(AddCondBBEndFor2ndPass(cmdBuffer));

        if (m_pipeline->IsFirstPipe())
        {
            ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(AddPictureVdencCommands(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        SETPAR_AND_ADDCMD(VDENC_CONTROL_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_PIPE_MODE_SELECT, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_SRC_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_REF_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_DS_REF_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_PIPE_BUF_ADDR_STATE, m_vdencItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::Construct3rdLevelBatch()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        //To be added. When BRC is enabled, some of the commands
        //will be added into 3rd level batch

        return eStatus;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::AddOneTileCommands(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t tileRow,
        uint32_t tileCol,
        uint32_t tileRowPass)
    {
        ENCODE_FUNC_CALL();

        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);
        if ((m_pipeline->GetPipeNum() > 1) && (tileCol != m_pipeline->GetCurrentPipe()))
        {
            return MOS_STATUS_SUCCESS;
        }

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

        //AV1 Tile Commands
        //set up VD_CONTROL_STATE command
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_PIPE_MODE_SELECT(tempCmdBuffer));
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_AVP_SURFACE_STATE(tempCmdBuffer));
        SETPAR_AND_ADDCMD(AVP_PIPE_BUF_ADDR_STATE, m_avpItf, tempCmdBuffer);
        SETPAR_AND_ADDCMD(AVP_IND_OBJ_BASE_ADDR_STATE, m_avpItf, tempCmdBuffer);
        if (brcFeature->IsBRCEnabled())
        {
            bool firstTileInGroup = false;
            uint32_t tileGroupIdx = 0;
            RUN_FEATURE_INTERFACE_NO_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, IsFirstTileInGroup, firstTileInGroup, tileGroupIdx);
            vdenc2ndLevelBatchBuffer->dwOffset = firstTileInGroup? slbbData.avpPicStateOffset : slbbData.secondAvpPicStateOffset;
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

        //VDENC AV1 Tile Commands
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

        //VDENC AV1 Encode
        SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencItf, tempCmdBuffer);

        m_basicFeature->m_flushCmd = Av1BasicFeature::waitVdenc;
        SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, tempCmdBuffer);

        ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(*tempCmdBuffer));

        if (!m_osInterface->bUsesPatchList)
        {
            // For 2nd level BB, we must use tileLevelBatchBuffer to prevent adding Epilogue before MI_BATCH_BUFFER_END
            ENCODE_CHK_NULL_RETURN(tileLevelBatchBuffer);
            tileLevelBatchBuffer->iCurrent = constructTileBatchBuf.iOffset;
            tileLevelBatchBuffer->iRemaining = constructTileBatchBuf.iRemaining;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, tileLevelBatchBuffer));
        }

    #if MHW_HWCMDPARSER_ENABLED
        auto instance = mhw::HwcmdParser::GetInstance();
        if (instance)
        {
            instance->ParseCmdBuf(tempCmdBuffer->pCmdBase, tempCmdBuffer->iOffset / sizeof(uint32_t));
        }
    #endif

#if USE_CODECHAL_DEBUG_TOOL
        CodechalDebugInterface* debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpCmdBuffer(
            &constructTileBatchBuf,
            CODECHAL_NUM_MEDIA_STATES,
            "_TILE_CMD_BUFFER"));
#endif

        // End patching tile level batch cmds
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, EndPatchTileLevelBatch);

        ENCODE_CHK_STATUS_RETURN(ReadPakMmioRegisters(&cmdBuffer, tileRow == 0 && tileCol == 0));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        // Send MI_FLUSH command
        auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                               = {};
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::UpdateUserFeatureKey(PMOS_SURFACE surface)
    {
        if (m_userFeatureUpdated_post_cdef)
        {
            return MOS_STATUS_SUCCESS;
        }
        m_userFeatureUpdated_post_cdef = true;
        ReportUserSetting(m_userSettingPtr,
            "AV1 Post CDEF Recon Compressible",
            surface->bCompressible,
            MediaUserSetting::Group::Sequence);
        ReportUserSetting(m_userSettingPtr ,
            "AV1 Post CDEF Recon Compress Mode",
            surface->MmcState,
            MediaUserSetting::Group::Sequence);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(Construct3rdLevelBatch());

        uint16_t numTileColumns = 1;
        uint16_t numTileRows = 1;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

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

        m_basicFeature->m_flushCmd = Av1BasicFeature::waitAvp;
        SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

#if USE_CODECHAL_DEBUG_TOOL
        ENCODE_CHK_STATUS_RETURN(Av1VdencPkt::PopulateParFileParams());
#endif  // USE_CODECHAL_DEBUG_TOOL

        // Wait all pipe cmds done for the packet
        auto scalability = m_pipeline->GetMediaScalability();
        ENCODE_CHK_STATUS_RETURN(scalability->SyncPipe(syncOnePipeWaitOthers, 0, &cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, &cmdBuffer));

        if (Mos_Solo_Extension(m_osInterface->pOsContext))
        {
            if (m_pipeline->IsLastPass() && m_pipeline->IsFirstPipe())
            {
                ENCODE_CHK_STATUS_RETURN(MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
            }
        }
        else if (m_osInterface->bInlineCodecStatusUpdate)
        {
            ENCODE_CHK_STATUS_RETURN(UpdateStatusReport(statusReportGlobalCount, &cmdBuffer));
        }
        else if (m_pipeline->IsLastPass() && m_pipeline->IsFirstPipe())
        {
            ENCODE_CHK_STATUS_RETURN(MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
        }

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState) {
                m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_reconSurface));
            })

        PCODEC_REF_LIST currRefList     = m_basicFeature->m_ref.GetCurrRefList();
        MOS_SURFACE *  postCdefSurface = m_basicFeature->m_trackedBuf->GetSurface(
            BufferType::postCdefReconSurface, currRefList->ucScalingIdx);

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState) { UpdateUserFeatureKey(postCdefSurface);})
        UpdateParameters();

        return MOS_STATUS_SUCCESS;
    }

    void Av1VdencPktXe_M_Base::UpdateParameters()
    {
        ENCODE_FUNC_CALL();

        Av1VdencPkt::UpdateParameters();

        if (!m_pipeline->IsSingleTaskPhaseSupported())
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        //TBD
    }

    MOS_STATUS Av1VdencPktXe_M_Base::ReadAvpStatus(MHW_VDBOX_NODE_IND vdboxIndex, MediaStatusReport * statusReport, MOS_COMMAND_BUFFER & cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODECHAL_HW_FUNCTION_ENTER;

        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        auto hwInterfaceG12 = dynamic_cast<CodechalHwInterfaceG12 *>(m_hwInterface);
        ENCODE_CHK_NULL_RETURN(hwInterfaceG12);

        MOS_RESOURCE *osResource    = nullptr;
        uint32_t     offset         = 0;

        EncodeStatusReadParams params;
        MOS_ZeroMemory(&params, sizeof(params));

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportMfxBitstreamByteCountPerFrame, osResource, offset));
        params.resBitstreamByteCountPerFrame    = osResource;
        params.bitstreamByteCountPerFrameOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportQPStatusCount, osResource, offset));
        params.resQpStatusCount    = osResource;
        params.qpStatusCountOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportImageStatusMask, osResource, offset));
        params.resImageStatusMask    = osResource;
        params.imageStatusMaskOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportImageStatusCtrl, osResource, offset));
        params.resImageStatusCtrl    = osResource;
        params.imageStatusCtrlOffset = offset;

        ENCODE_CHK_STATUS_RETURN(hwInterfaceG12->ReadAvpStatus(vdboxIndex, params, &cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(hwInterfaceG12->ReadImageStatusForAvp(vdboxIndex, params, &cmdBuffer));

        return eStatus;
    }

    MHW_SETPAR_DECL_SRC(AVP_IND_OBJ_BASE_ADDR_STATE, Av1VdencPktXe_M_Base)
    {
        params.mvObjectOffset = m_mvOffset;
        params.mvObjectSize   = m_basicFeature->m_mbCodeSize - m_mvOffset;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1VdencPktXe_M_Base)
    {
        params.notFirstPass = !m_pipeline->IsFirstPass();

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_TILE_CODING, Av1VdencPktXe_M_Base)
    {
        uint32_t tileIdx = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileIdx, tileIdx);
        params.disableFrameContextUpdateFlag = m_av1PicParams->PicFlags.fields.disable_frame_end_update_cdf || (tileIdx != m_av1PicParams->context_update_tile_id);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::AddAllCmds_AVP_PIPE_MODE_SELECT(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        auto &vdControlStateParams          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
        vdControlStateParams                = {};
        vdControlStateParams.initialization = true;
        vdControlStateParams.avpEnabled     = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(cmdBuffer));

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after AVP Pipemode select.
        auto &mfxWaitParams                 = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(cmdBuffer));

        SETPAR_AND_ADDCMD(AVP_PIPE_MODE_SELECT, m_avpItf, cmdBuffer);

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after AVP Pipemode select.
        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(cmdBuffer));

        // AVP Lock for multiple pipe mode
        if (m_pipeline->GetPipeNum() > 1)
        {
            vdControlStateParams                      = {};
            vdControlStateParams.avpEnabled           = true;
            vdControlStateParams.scalableModePipeLock = true;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::AddAllCmds_AVP_SEGMENT_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_featureManager);

        auto& par = m_avpItf->MHW_GETPAR_F(AVP_SEGMENT_STATE)();
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

    MOS_STATUS Av1VdencPktXe_M_Base::AddAllCmds_AVP_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        auto& params = m_avpItf->MHW_GETPAR_F(AVP_PAK_INSERT_OBJECT)();
        params      = {};

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

                CODECHAL_ENCODE_ASSERT(nalUnitSize < CODECHAL_ENCODE_AV1_PAK_INSERT_UNCOMPRESSED_HEADER);

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
                        Mos_AddCommand(cmdBuffer, GetExtraData(), GetExtraSize());
                    }
                }
                else
                {
                    m_avpItf->MHW_ADDCMD_F(AVP_PAK_INSERT_OBJECT)(cmdBuffer);
                    Mos_AddCommand(cmdBuffer, GetExtraData(), GetExtraSize());
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
                Mos_AddCommand(cmdBuffer, GetExtraData(), GetExtraSize());
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::UpdateStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        //initialize following
        MOS_RESOURCE *osResourceInline = nullptr;
        uint32_t      offsetInline     = 0;
        m_statusReport->GetAddress(statusReportGlobalCount, osResourceInline, offsetInline);
        offsetInline             = m_atomicScratchBuf.operandSetSize * m_atomicScratchBuf.encodeUpdateIndex;
        uint32_t zeroValueOffset = offsetInline;
        uint32_t operand1Offset  = offsetInline + m_atomicScratchBuf.operand1Offset;
        uint32_t operand2Offset  = offsetInline + m_atomicScratchBuf.operand2Offset;
        uint32_t operand3Offset  = offsetInline + m_atomicScratchBuf.operand3Offset;
        auto     mmioRegisters   = m_hwInterface->SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);

        // Make Flush DW call to make sure all previous work is done
        auto &flushDwParams      = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams            = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // n1_lo = 0x00
        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        storeDataParams.dwResourceOffset = operand1Offset;
        storeDataParams.dwValue          = 0x00;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        // n2_lo = dwImageStatusMask
        auto &copyMemMemParams       = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        copyMemMemParams             = {};
        copyMemMemParams.presSrc     = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        copyMemMemParams.dwSrcOffset = (sizeof(uint32_t) * 1);
        copyMemMemParams.presDst     = m_atomicScratchBuf.resAtomicScratchBuffer;
        copyMemMemParams.dwDstOffset = operand2Offset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

        // VCS_GPR0_Lo = ImageStatusCtrl
        auto &registerMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        registerMemParams.dwOffset        = (sizeof(uint32_t) * 0);
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;  // VCS_GPR0_Lo
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Reset GPR4_Lo
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = zeroValueOffset;                                 //Offset 0, has value of 0.
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;  // VCS_GPR4
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // step-1: n2_lo = n2_lo & VCS_GPR0_Lo = dwImageStatusMask & ImageStatusCtrl
        auto &atomicParams            = m_miItf->MHW_GETPAR_F(MI_ATOMIC)();
        atomicParams                  = {};
        atomicParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        atomicParams.dwResourceOffset = operand2Offset;
        atomicParams.dwDataSize       = sizeof(uint32_t);
        atomicParams.Operation        = mhw::mi::MHW_MI_ATOMIC_AND;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

        // n3_lo = 0
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        storeDataParams.dwResourceOffset = operand3Offset;
        storeDataParams.dwValue          = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // GPR0_lo = n1_lo = 0
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = operand1Offset;
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;  // VCS_GPR0
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Reset GPR4_Lo
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = zeroValueOffset;                                 //Offset 0, has value of 0.
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;  // VCS_GPR4
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // step-2: n2_lo == n1_lo ? 0 : n2_lo
        // compare n1 vs n2. i.e. GRP0 vs. memory of operand2
        atomicParams                  = {};
        atomicParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        atomicParams.dwResourceOffset = operand2Offset;
        atomicParams.dwDataSize       = sizeof(uint32_t);
        atomicParams.Operation        = mhw::mi::MHW_MI_ATOMIC_CMP;
        atomicParams.bReturnData      = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

        // n2_hi = 1
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        storeDataParams.dwResourceOffset = operand2Offset + sizeof(uint32_t);
        storeDataParams.dwValue          = 1;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        // n3_hi = 1
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        storeDataParams.dwResourceOffset = operand3Offset + sizeof(uint32_t);
        storeDataParams.dwValue          = 1;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        // VCS_GPR0_Lo = n3_lo = 0
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = operand3Offset;
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;  // VCS_GPR0_Lo
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // GPR0_Hi = n2_hi = 1
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = operand2Offset + sizeof(uint32_t);               // update 1
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0HiOffset;  // VCS_GPR0_Hi
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Reset GPR4_Lo and GPR4_Hi
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = zeroValueOffset;
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;  // VCS_GPR4_Lo
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = zeroValueOffset;
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4HiOffset;  // VCS_GPR4_Hi
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // steop-3: n2 = (n2 == 0:1) ? 0:0 : n2      // uint64_t CMP
        // If n2==0 (Lo) and 1 (Hi), covert n2 to 0 (Lo)and 0 (Hi), else no change.
        // n2 == 0:1 means encoding completsion. the n2 memory will be updated with 0:0, otherwise, no change.
        atomicParams                  = {};
        atomicParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        atomicParams.dwResourceOffset = operand2Offset;
        atomicParams.dwDataSize       = sizeof(uint64_t);
        atomicParams.Operation        = mhw::mi::MHW_MI_ATOMIC_CMP;
        atomicParams.bReturnData      = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // VCS_GPR0_Lo = n3_hi = 1
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = operand3Offset + sizeof(uint32_t);
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;  // VCS_GPR0_Lo
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // step-4: n2_hi = n2_hi ^ VCS_GPR0_Lo = n2_hi ^ n3_hi
        atomicParams                  = {};
        atomicParams.pOsResource      = m_atomicScratchBuf.resAtomicScratchBuffer;
        atomicParams.dwResourceOffset = operand2Offset + sizeof(uint32_t);
        atomicParams.dwDataSize       = sizeof(uint32_t);
        atomicParams.Operation        = mhw::mi::MHW_MI_ATOMIC_XOR;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

        // VCS_GPR0_Lo = n2_hi
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = operand2Offset + sizeof(uint32_t);
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;  // VCS_GPR0_Lo
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // step-5: m_storeData = m_storeData + VCS_GPR0_Lo = m_storeData + n2_hi
        // if not completed n2_hi should be 0, then m_storeData = m_storeData + 0
        // if completed, n2_hi should be 1, then m_storeData = m_storeData + 1
        auto &miLoadRegMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = osResourceInline;
        miLoadRegMemParams.dwOffset        = 0;
        miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        mhw::mi::MHW_MI_ALU_PARAMS aluParams[4] = { 0 };
        int               aluCount = 0;

        aluCount = 0;

        //load1 srca, reg1
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCA;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG0;
        ++aluCount;
        //load srcb, reg2
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_SRCB;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_GPREG4;
        ++aluCount;
        //add
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_ADD;
        ++aluCount;
        //store reg1, accu
        aluParams[aluCount].AluOpcode = MHW_MI_ALU_STORE;
        aluParams[aluCount].Operand1  = MHW_MI_ALU_GPREG0;
        aluParams[aluCount].Operand2  = MHW_MI_ALU_ACCU;
        ++aluCount;

        auto &miMathParams           = m_miItf->MHW_GETPAR_F(MI_MATH)();
        miMathParams                 = {};
        miMathParams.dwNumAluParams  = aluCount;
        miMathParams.pAluPayload     = aluParams;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(Mos_AddCommand(
            cmdBuffer,
            &miMathParams.pAluPayload[0],
            sizeof(mhw::mi::MHW_MI_ALU_PARAMS) * miMathParams.dwNumAluParams));

        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = osResourceInline;
        miStoreRegMemParams.dwOffset        = 0;
        miStoreRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPktXe_M_Base::CalculateAvpPictureStateCommandSize(uint32_t * commandsSize, uint32_t * patchListSize)
    {
        ENCODE_FUNC_CALL();

        MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 stateCmdSizeParams;
        MOS_ZeroMemory(&stateCmdSizeParams, sizeof(MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            m_hwInterface->GetAvpStateCommandSize(
                CODECHAL_ENCODE_MODE_AV1,
                commandsSize,
                patchListSize,
                &stateCmdSizeParams));

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Av1VdencPktXe_M_Base::DumpStatistics()
    {

        CodechalDebugInterface *debugInterface =  m_pipeline->GetStatusReportDebugInterface();
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
        uint32_t offset = 0;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileStatsOffset, offset);

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            tileStatisticsBuffer,
            CodechalDbgAttr::attrFrameState,
            "VDEnc_Frame_Stats",
            m_hwInterface->m_pakIntTileStatsSize,
            offset,
            CODECHAL_NUM_MEDIA_STATES));

        MOS_RESOURCE *pakinfo = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            pakinfo,
            CodechalDbgAttr::attrFrameState,
            "VDEnc_PAK_INFO",
            MOS_ALIGN_CEIL(sizeof(Av1VdencPakInfo), CODECHAL_PAGE_SIZE),
            0,
            CODECHAL_NUM_MEDIA_STATES));

        return MOS_STATUS_SUCCESS;
    }
#endif  // USE_CODECHAL_DEBUG_TOOL
    }   // namespace encode
