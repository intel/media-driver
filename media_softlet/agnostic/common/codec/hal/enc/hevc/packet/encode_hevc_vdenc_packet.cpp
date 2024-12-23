/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_hevc_vdenc_packet.cpp
//! \brief    Defines the interface for hevc encode vdenc packet
//!
#include "encode_hevc_vdenc_packet.h"
#include "mos_solo_generic.h"
#include "encode_vdenc_lpla_analysis.h"
#include "encode_hevc_vdenc_weighted_prediction.h"
#include "mhw_mi_itf.h"
#include "media_perf_profiler.h"
#include "codec_hw_next.h"
#include "hal_oca_interface_next.h"

using namespace mhw::vdbox;

namespace encode
{
    MOS_STATUS HevcVdencPkt::AllocateResources()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(m_allocator);
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;

        allocParamsForBufferLinear.dwBytes  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, m_basicFeature->m_maxLCUSize) * CODECHAL_CACHELINE_SIZE * 2 * 2;
        allocParamsForBufferLinear.pBufName = "vdencIntraRowStoreScratch";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        m_vdencIntraRowStoreScratch         = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        // VDENC tile row store buffer
        allocParamsForBufferLinear.dwBytes  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, 32) * CODECHAL_CACHELINE_SIZE * 2;
        allocParamsForBufferLinear.pBufName = "VDENC Tile Row Store Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        m_vdencTileRowStoreBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear,false);

        hcp::HcpBufferSizePar hcpBufSizePar;
        MOS_ZeroMemory(&hcpBufSizePar, sizeof(hcpBufSizePar));

        hcpBufSizePar.ucMaxBitDepth  = m_basicFeature->m_bitDepth;
        hcpBufSizePar.ucChromaFormat = m_basicFeature->m_chromaFormat;
        // We should move the buffer allocation to picture level if the size is dependent on LCU size
        hcpBufSizePar.dwCtbLog2SizeY = 6;  //assume Max LCU size
        hcpBufSizePar.dwPicWidth     = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, m_basicFeature->m_maxLCUSize);
        hcpBufSizePar.dwPicHeight    = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, m_basicFeature->m_maxLCUSize);

        auto AllocateHcpBuffer = [&](PMOS_RESOURCE &res, const hcp::HCP_INTERNAL_BUFFER_TYPE bufferType, const char *bufferName) {
            uint32_t bufSize = 0;
            hcpBufSizePar.bufferType = bufferType;
            eStatus                  = m_hcpItf->GetHcpBufSize(hcpBufSizePar, bufSize);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                ENCODE_ASSERTMESSAGE("Failed to get hcp buffer size.");
                return eStatus;
            }
            allocParamsForBufferLinear.dwBytes  = bufSize;
            allocParamsForBufferLinear.pBufName = bufferName;
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
            res                                 = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
            return MOS_STATUS_SUCCESS;
        };

        // Metadata Line buffer
        ENCODE_CHK_STATUS_RETURN(AllocateHcpBuffer(m_resMetadataLineBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::META_LINE, "MetadataLineBuffer"));
        // Metadata Tile Line buffer
        ENCODE_CHK_STATUS_RETURN(AllocateHcpBuffer(m_resMetadataTileLineBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::META_TILE_LINE, "MetadataTileLineBuffer"));
        // Metadata Tile Column buffer
        ENCODE_CHK_STATUS_RETURN(AllocateHcpBuffer(m_resMetadataTileColumnBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::META_TILE_COL, "MetadataTileColumnBuffer"));

        // Lcu ILDB StreamOut buffer
        // This is not enabled with HCP_PIPE_MODE_SELECT yet, placeholder here
        allocParamsForBufferLinear.dwBytes = CODECHAL_CACHELINE_SIZE;
        allocParamsForBufferLinear.pBufName = "LcuILDBStreamOutBuffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        m_resLCUIldbStreamOutBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear,false);

        // Allocate SSE Source Pixel Row Store Buffer
        uint32_t maxTileColumns    = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, CODECHAL_HEVC_MIN_TILE_SIZE);
        allocParamsForBufferLinear.dwBytes  = 2 * m_basicFeature->m_sizeOfSseSrcPixelRowStoreBufferPerLcu * (m_basicFeature->m_widthAlignedMaxLCU + 3 * maxTileColumns);
        allocParamsForBufferLinear.pBufName = "SseSrcPixelRowStoreBuffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        m_resSSESrcPixelRowStoreBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear,false);

        uint32_t frameWidthInCus = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, CODECHAL_HEVC_MIN_CU_SIZE);
        uint32_t frameHeightInCus = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameHeight, CODECHAL_HEVC_MIN_CU_SIZE);
        uint32_t frameWidthInLCUs = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, CODECHAL_HEVC_VDENC_LCU_SIZE);
        uint32_t frameHeightInLCUs = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameHeight, CODECHAL_HEVC_VDENC_LCU_SIZE);
        // PAK CU Level Streamout Data:   DW57-59 in HCP pipe buffer address command
        // One CU has 16-byte. But, each tile needs to be aliged to the cache line
        auto size = MOS_ALIGN_CEIL(frameWidthInCus * frameHeightInCus * 16, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.dwBytes = size;
        allocParamsForBufferLinear.pBufName = "PAK CU Level Streamout Data";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        m_resPakcuLevelStreamOutData = m_allocator->AllocateResource(allocParamsForBufferLinear,false);

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.dwBytes  = frameWidthInLCUs * frameHeightInLCUs * 4;
        allocParamsForBufferLinear.pBufName = "VDEnc Cumulative CU Count Streamout Surface";
        m_resCumulativeCuCountStreamoutBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

        if(m_osInterface->bInlineCodecStatusUpdate)
        {
            m_atomicScratchBuf.size = MOS_ALIGN_CEIL(sizeof(AtomicScratchBuffer), sizeof(uint64_t));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            size  = MHW_CACHELINE_SIZE * 4 * 2; //  each set of scratch is 4 cacheline size, and allocate 2 set.
            allocParamsForBufferLinear.dwBytes  = size;
            allocParamsForBufferLinear.pBufName = "atomic sratch buffer";

            if (MEDIA_IS_SKU(m_hwInterface->GetSkuTable(), FtrLocalMemory))
            {
                allocParamsForBufferLinear.dwMemType = MOS_MEMPOOL_DEVICEMEMORY;
            }
            else
            {
                allocParamsForBufferLinear.dwMemType = MOS_MEMPOOL_VIDEOMEMORY;
            }

            m_atomicScratchBuf.resAtomicScratchBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

            ENCODE_CHK_NULL_RETURN(m_atomicScratchBuf.resAtomicScratchBuffer);

            m_atomicScratchBuf.size               = size;
            m_atomicScratchBuf.zeroValueOffset    = 0;
            m_atomicScratchBuf.operand1Offset     = MHW_CACHELINE_SIZE;
            m_atomicScratchBuf.operand2Offset     = MHW_CACHELINE_SIZE * 2;
            m_atomicScratchBuf.operand3Offset     = MHW_CACHELINE_SIZE * 3;
            m_atomicScratchBuf.encodeUpdateIndex  = 0;
            m_atomicScratchBuf.tearDownIndex      = 1;
            m_atomicScratchBuf.operandSetSize     = MHW_CACHELINE_SIZE * 4;
        }

#if USE_CODECHAL_DEBUG_TOOL && _ENCODE_RESERVED
        m_hevcParDump = std::make_shared<HevcVdencParDump>(m_pipeline);
#endif  // USE_CODECHAL_DEBUG_TOOL

        return eStatus;
    }

    MOS_STATUS HevcVdencPkt::Prepare()
    {
        ENCODE_FUNC_CALL();

        m_pictureStatesSize    = m_defaultPictureStatesSize;
        m_picturePatchListSize = m_defaultPicturePatchListSize;
        m_sliceStatesSize      = m_defaultSliceStatesSize;
        m_slicePatchListSize   = m_defaultSlicePatchListSize;

        HevcPipeline *pipeline = dynamic_cast<HevcPipeline *>(m_pipeline);
        ENCODE_CHK_NULL_RETURN(pipeline);

        m_hevcSeqParams      = m_basicFeature->m_hevcSeqParams;
        m_hevcPicParams      = m_basicFeature->m_hevcPicParams;
        m_hevcSliceParams    = m_basicFeature->m_hevcSliceParams;
        m_hevcIqMatrixParams = m_basicFeature->m_hevcIqMatrixParams;
        m_nalUnitParams      = m_basicFeature->m_nalUnitParams;

        ENCODE_CHK_STATUS_RETURN(ValidateVdboxIdx(m_vdboxIndex));

        m_pakOnlyPass = false;

        ENCODE_CHK_STATUS_RETURN(SetBatchBufferForPakSlices());

        SetRowstoreCachingOffsets();

        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, SetPipeNumber, m_pipeline->GetPipeNum());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::SubmitPictureLevel(
        MOS_COMMAND_BUFFER* commandBuffer,
        uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        MOS_COMMAND_BUFFER& cmdBuffer = *commandBuffer;
        ENCODE_CHK_STATUS_RETURN(Mos_Solo_PreProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));

        // Ensure the input is ready to be read.
        // Currently, mos RegisterResource has sync limitation for Raw resource.
        // Temporaly, call Resource Wait to do the sync explicitly.
        if(m_pipeline->IsFirstPass())
        {
            MOS_SYNC_PARAMS       syncParams;
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);
            syncParams.presSyncResource = &m_basicFeature->m_rawSurface.OsResource;
            syncParams.bReadOnly = true;
            ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        ENCODE_CHK_STATUS_RETURN(PatchPictureLevelCommands(packetPhase, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::SubmitTileLevel(
        MOS_COMMAND_BUFFER* commandBuffer,
        uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();
        auto eStatus = MOS_STATUS_SUCCESS;

        if (!m_hevcPicParams->tiles_enabled_flag)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        MOS_COMMAND_BUFFER& cmdBuffer = *commandBuffer;

        ENCODE_CHK_STATUS_RETURN(Construct3rdLevelBatch());

        uint16_t numTileColumns = 1;
        uint16_t numTileRows = 1;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

        if (m_pipeline->GetPipeNum() == 2)
        {
            ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                cmdBuffer,
                m_pipeline->GetCurrentRow(),
                m_pipeline->GetCurrentPipe(),
                m_pipeline->GetCurrentSubPass()));
        }
        else
        {
            for (uint16_t tileCol = 0; tileCol < numTileColumns; tileCol++)
            {
                ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                    cmdBuffer,
                    m_pipeline->GetCurrentRow(),
                    tileCol,
                    m_pipeline->GetCurrentPass()));
            }
        }

        // Insert end of sequence/stream if set
        if ((m_basicFeature->m_lastPicInSeq || m_basicFeature->m_lastPicInStream) && m_pipeline->IsLastPipe())
        {
            ENCODE_CHK_STATUS_RETURN(InsertSeqStreamEnd(cmdBuffer));
        }

        if (m_pipeline->GetCurrentRow() == (numTileRows - 1))
        {
            // Send VD_CONTROL_STATE (Memory Implict Flush)
            auto &vdControlStateParams = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
            vdControlStateParams = {};
            vdControlStateParams.memoryImplicitFlush = true;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

            m_flushCmd = waitHevc;
            SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);

            ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

            // Wait all pipe cmds done for the packet
            auto scalability = m_pipeline->GetMediaScalability();
            ENCODE_CHK_STATUS_RETURN(scalability->SyncPipe(syncOnePipeWaitOthers, 0, &cmdBuffer));

            // Store PAK frameSize MMIO to PakInfo buffer
            auto mmioRegisters                  = m_hcpItf->GetMmioRegisters(m_vdboxIndex);
            auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
            miStoreRegMemParams                 = {};
            miStoreRegMemParams.presStoreBuffer = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);;
            miStoreRegMemParams.dwOffset        = 0;
            miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

            ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));
        }
        // post-operations are done by pak integrate pkt

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::Submit(
        MOS_COMMAND_BUFFER* commandBuffer,
        uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        if (m_submitState == submitFrameByDefault)
        {
            ENCODE_CHK_STATUS_RETURN(SubmitPictureLevel(commandBuffer, packetPhase));

            MOS_COMMAND_BUFFER& cmdBuffer = *commandBuffer;

            bool tileEnabled = false;
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);
            if (!tileEnabled)
            {
                ENCODE_CHK_STATUS_RETURN(PatchSliceLevelCommands(cmdBuffer, packetPhase));
            }
            else
            {
                ENCODE_CHK_STATUS_RETURN(PatchTileLevelCommands(cmdBuffer, packetPhase));
            }

            ENCODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessEncode(m_osInterface, &m_basicFeature->m_resBitstreamBuffer, &m_basicFeature->m_reconSurface));
        }
        else
        {
            switch (m_submitState)
            {
                case submitPic:
                {
                    ENCODE_CHK_STATUS_RETURN(SubmitPictureLevel(commandBuffer, packetPhase));
                    m_submitState = submitInvalid;
                    break;
                };
                case submitTile:
                {
                    ENCODE_FUNC_CALL();

                    bool tileEnabled = false;
                    RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);
                    ENCODE_CHK_STATUS_RETURN(SubmitTileLevel(commandBuffer, packetPhase));

                    m_submitState = submitInvalid;
                    break;
                };
                default:
                    m_submitState = submitInvalid;
                    break;
            }
        }

        m_enableVdencStatusReport = true;

    #if USE_CODECHAL_DEBUG_TOOL && _ENCODE_RESERVED
        m_hevcParDump->SetParFile();
        ENCODE_CHK_STATUS_RETURN(DumpInput());
    #endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER  &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        cmdBuffer.Attributes.bFrequencyBoost = (m_basicFeature->m_hevcSeqParams->ScenarioInfo == ESCENARIO_REMOTEGAMING);

        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

        SetPerfTag();

        auto feature = dynamic_cast<HEVCEncodeBRC*>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(feature);
        bool firstTaskInPhase = packetPhase & firstPacket;
        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)//(m_pipeline->IsFirstPass() && !feature->IsVdencHucUsed())) && m_pipeline->GetPipeNum() == 1) || m_pipeline->GetPipeNum() >= 2)
        {
            ENCODE_CHK_STATUS_RETURN(AddForceWakeup(cmdBuffer));

            // Send command buffer header at the beginning (OS dependent)
            ENCODE_CHK_STATUS_RETURN(SendPrologCmds(cmdBuffer));
        }

        if (m_pipeline->GetPipeNum() >= 2)
        {
            auto scalability = m_pipeline->GetMediaScalability();
            if (m_pipeline->IsFirstPass())
            {
                // Reset multi-pipe sync semaphores
                ENCODE_CHK_STATUS_RETURN(scalability->ResetSemaphore(syncOnePipeWaitOthers, m_pipeline->GetCurrentPipe(), &cmdBuffer));
            }

            // For brc case, other pipes wait for BRCupdate done on first pipe
            // For cqp case, pipes also need sync 
            ENCODE_CHK_STATUS_RETURN(scalability->SyncPipe(syncOtherPipesForOne, 0, &cmdBuffer));
        }

        m_streamInEnabled = false;
        RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature,
            EnableStreamIn, m_pipeline->IsFirstPass(), m_pipeline->IsLastPass(), m_streamInEnabled);

        ENCODE_CHK_STATUS_RETURN(AddCondBBEndForLastPass(cmdBuffer));

        if (m_pipeline->IsFirstPipe())
        {
            ENCODE_CHK_STATUS_RETURN(StartStatusReport(statusReportMfx, &cmdBuffer));
        }

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, &cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddPictureHcpCommands(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddPictureVdencCommands(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddPicStateWithNoTile(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::PatchSliceLevelCommands(MOS_COMMAND_BUFFER  &cmdBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        if (m_hevcPicParams->tiles_enabled_flag)
        {
            return MOS_STATUS_SUCCESS;
        }

        auto feature = dynamic_cast<HEVCEncodeBRC*>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(feature);
        auto vdenc2ndLevelBatchBuffer = feature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);

        // starting location for executing slice level cmds
        vdenc2ndLevelBatchBuffer->dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;

        PCODEC_ENCODER_SLCDATA slcData = m_basicFeature->m_slcData;
        for (uint32_t startLcu = 0, slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
        {
            if (m_pipeline->IsFirstPass())
            {
                slcData[slcCount].CmdOffset = startLcu * (m_hcpItf->GetHcpPakObjSize()) * sizeof(uint32_t);
            }
            m_basicFeature->m_curNumSlices = slcCount;

            ENCODE_CHK_STATUS_RETURN(SendHwSliceEncodeCommand(nullptr, slcCount, cmdBuffer));

            startLcu += m_hevcSliceParams[slcCount].NumLCUsInSlice;

            m_batchBufferForPakSlicesStartOffset = (uint32_t)m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx].iCurrent;
            if (feature->IsACQPEnabled() || feature->IsBRCEnabled())
            {
                // save offset for next 2nd level batch buffer usage
                // This is because we don't know how many times HCP_WEIGHTOFFSET_STATE & HCP_PAK_INSERT_OBJECT will be inserted for each slice
                // dwVdencBatchBufferPerSliceConstSize: constant size for each slice
                // m_vdencBatchBufferPerSliceVarSize:   variable size for each slice
                vdenc2ndLevelBatchBuffer->dwOffset += m_hwInterface->m_vdencBatchBufferPerSliceConstSize + m_basicFeature->m_vdencBatchBufferPerSliceVarSize[slcCount];
            }

            m_flushCmd = waitVdenc;
            SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);
        }

        if (m_useBatchBufferForPakSlices)
        {
            ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(
                m_osInterface,
                &m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx],
                m_lastTaskInPhase));
        }

        // Insert end of sequence/stream if set
        if (m_basicFeature->m_lastPicInSeq || m_basicFeature->m_lastPicInStream)
        {
            ENCODE_CHK_STATUS_RETURN(InsertSeqStreamEnd(cmdBuffer));
        }

        ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

        m_flushCmd = waitHevc;
        SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(m_vdboxIndex, m_statusReport, cmdBuffer));
        // BRC PAK statistics different for each pass
        if (feature->IsBRCEnabled())
        {
            uint8_t                     ucPass = (uint8_t)m_pipeline->GetCurrentPass();
            EncodeReadBrcPakStatsParams readBrcPakStatsParams;
            MOS_RESOURCE *              osResource = nullptr;
            uint32_t                    offset = 0;
            ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportNumberPasses, osResource, offset));
            RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature, SetReadBrcPakStatsParams, ucPass, offset, osResource, readBrcPakStatsParams);
            ReadBrcPakStatistics(&cmdBuffer, &readBrcPakStatsParams);
        }
        ENCODE_CHK_STATUS_RETURN(ReadExtStatistics(cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(ReadSliceSize(cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(PrepareHWMetaData(&cmdBuffer));
        RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature, StoreLookaheadStatistics, cmdBuffer, m_vdboxIndex);

#if USE_CODECHAL_DEBUG_TOOL
        uint32_t sizeInByte = 0;
        bool     isIframe   = m_basicFeature->m_pictureCodingType == I_TYPE;
        ENCODE_CHK_NULL_RETURN(m_packetUtilities);
        if (m_packetUtilities->GetFakeHeaderSettings(sizeInByte, isIframe))
        {
            ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
            ENCODE_CHK_STATUS_RETURN(m_packetUtilities->ModifyEncodedFrameSizeWithFakeHeaderSize(
                &cmdBuffer,
                sizeInByte,
                m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0),
                0,
                m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0),
                sizeof(uint32_t) * 4));
        }
#endif

        ENCODE_CHK_STATUS_RETURN(EndStatusReport(statusReportMfx, &cmdBuffer));

        if (Mos_Solo_Extension((MOS_CONTEXT_HANDLE)m_osInterface->pOsContext))
        {
            if (m_pipeline->IsLastPass() && m_pipeline->IsFirstPipe())
            {
                MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer);
            }
        }
        else if (m_osInterface->bInlineCodecStatusUpdate
            && !(m_basicFeature->m_422State && m_basicFeature->m_422State->GetFeature422Flag())
            )
        {
            if (feature->IsBRCEnabled())
            {
                ENCODE_CHK_STATUS_RETURN(UpdateStatusReport(statusReportGlobalCount, &cmdBuffer));
            }
            else
            {
                ENCODE_CHK_STATUS_RETURN(MediaPacket::UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
            }
        }

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState) {
                m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_reconSurface));
            })
        // Reset parameters for next PAK execution
        if (false == m_pipeline->IsFrameTrackingEnabled() && m_pipeline->IsLastPass() && m_pipeline->IsLastPipe())
        {
            UpdateParameters();
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::Construct3rdLevelBatch()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        // Begin patching 3rd level batch cmds
        MOS_COMMAND_BUFFER constructedCmdBuf;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, BeginPatch3rdLevelBatch, constructedCmdBuf);

        SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, &constructedCmdBuf);

        SETPAR_AND_ADDCMD(HCP_PIC_STATE, m_hcpItf, &constructedCmdBuf);

        SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, &constructedCmdBuf);

        // set MI_BATCH_BUFFER_END command
        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr));

        // End patching 3rd level batch cmds
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, EndPatch3rdLevelBatch);

        return eStatus;
    }

    MOS_STATUS HevcVdencPkt::AddSlicesCommandsInTile(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        PCODEC_ENCODER_SLCDATA         slcData = m_basicFeature->m_slcData;

        uint32_t slcCount, sliceNumInTile = 0;
        for (slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
        {
            m_basicFeature->m_curNumSlices = slcCount;
            bool sliceInTile               = false;
            m_lastSliceInTile              = false;

            EncodeTileData curTileData = {};
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetCurrentTile, curTileData);
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsSliceInTile, slcCount, &curTileData, &sliceInTile, &m_lastSliceInTile);

            m_basicFeature->m_lastSliceInTile = m_lastSliceInTile;
            if (!sliceInTile)
            {
                continue;
            }

            ENCODE_CHK_STATUS_RETURN(SendHwSliceEncodeCommand(nullptr, slcCount, cmdBuffer));

            m_flushCmd = waitHevcVdenc;
            SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);

            sliceNumInTile++;
        }  // end of slice

        if (0 == sliceNumInTile)
        {
            // One tile must have at least one slice
            ENCODE_ASSERT(false);
            return MOS_STATUS_INVALID_PARAMETER;
        }

        uint16_t numTileRows    = 1;
        uint16_t numTileColumns = 1;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

        if (sliceNumInTile > 1 && (numTileColumns > 1 || numTileRows > 1))
        {
            ENCODE_ASSERTMESSAGE("Multi-slices in a tile is not supported!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddOneTileCommands(
        MOS_COMMAND_BUFFER &cmdBuffer,
        uint32_t            tileRow,
        uint32_t            tileCol,
        uint32_t            tileRowPass)
    {
        ENCODE_FUNC_CALL();
        PMOS_COMMAND_BUFFER tempCmdBuffer         = &cmdBuffer;
        PMHW_BATCH_BUFFER   tileLevelBatchBuffer  = nullptr;
        auto                eStatus               = MOS_STATUS_SUCCESS;
        MOS_COMMAND_BUFFER constructTileBatchBuf = {};

        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, SetCurrentTile, tileRow, tileCol, m_pipeline);

        if ((m_pipeline->GetPipeNum() > 1) && (tileCol != m_pipeline->GetCurrentPipe()))
        {
            return MOS_STATUS_SUCCESS;
        }

        if (!m_osInterface->bUsesPatchList)
        {
            // Begin patching tile level batch cmds
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, BeginPatchTileLevelBatch, tileRowPass, constructTileBatchBuf);

            // Add batch buffer start for tile
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileLevelBatchBuffer, tileLevelBatchBuffer);
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

        // HCP Lock for multiple pipe mode
        if (m_pipeline->GetPipeNum() > 1)
        {
            auto &vdControlStateParams                = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
            vdControlStateParams                      = {};
            vdControlStateParams.scalableModePipeLock = true;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(tempCmdBuffer));
        }

        SETPAR_AND_ADDCMD(VDENC_PIPE_MODE_SELECT, m_vdencItf, tempCmdBuffer);

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
        auto &mfxWaitParams                 = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(tempCmdBuffer));

        SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpItf, tempCmdBuffer);

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(tempCmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddPicStateWithTile(*tempCmdBuffer));

        SETPAR_AND_ADDCMD(HCP_TILE_CODING, m_hcpItf, tempCmdBuffer);

        ENCODE_CHK_STATUS_RETURN(AddSlicesCommandsInTile(*tempCmdBuffer));

        //HCP unLock for multiple pipe mode
        if (m_pipeline->GetPipeNum() > 1)
        {
            auto &vdControlStateParams                  = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
            vdControlStateParams                        = {};
            vdControlStateParams.scalableModePipeUnlock = true;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(tempCmdBuffer));
        }

        m_flushCmd = waitHevc;
        SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, tempCmdBuffer);

        ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(*tempCmdBuffer));

        if (!m_osInterface->bUsesPatchList)
        {
            // For 2nd level BB, we must use tileLevelBatchBuffer to prevent adding Epilogue before MI_BATCH_BUFFER_END
            ENCODE_CHK_NULL_RETURN(tileLevelBatchBuffer);
            tileLevelBatchBuffer->iCurrent   = tempCmdBuffer->iOffset;
            tileLevelBatchBuffer->iRemaining = tempCmdBuffer->iRemaining;
            ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, tileLevelBatchBuffer));
            HalOcaInterfaceNext::OnSubLevelBBStart(
                cmdBuffer,
                m_osInterface->pOsContext,
                &tempCmdBuffer->OsResource,
                0,
                false,
                tempCmdBuffer->iOffset);
            HalOcaInterfaceNext::On1stLevelBBEnd(*tempCmdBuffer, *m_osInterface);

        #if USE_CODECHAL_DEBUG_TOOL
            if (tempCmdBuffer->pCmdPtr && tempCmdBuffer->pCmdBase &&
                tempCmdBuffer->pCmdPtr > tempCmdBuffer->pCmdBase)
            {
                CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
                std::string             name("TileLevelBatchBuffer");
                name += "Row" + std::to_string(tileRow) + "Col" + std::to_string(tileCol);

                ENCODE_CHK_STATUS_RETURN(debugInterface->DumpData(
                    tempCmdBuffer->pCmdBase,
                    (uint32_t)(4 * (tempCmdBuffer->pCmdPtr - tempCmdBuffer->pCmdBase)),
                    CodechalDbgAttr::attrCmdBufferMfx,
                    name.c_str()));
            }
        #endif
        }

        // End patching tile level batch cmds
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, EndPatchTileLevelBatch);

        return eStatus;
    }

    MOS_STATUS HevcVdencPkt::PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();
        auto eStatus = MOS_STATUS_SUCCESS;

        if (!m_hevcPicParams->tiles_enabled_flag)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // multi tiles cases on Liunx, 3rd level batch buffer is 2nd level.
        ENCODE_CHK_STATUS_RETURN(Construct3rdLevelBatch());

        uint16_t numTileColumns = 1;
        uint16_t numTileRows    = 1;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileRowColumns, numTileRows, numTileColumns);

        for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
        {
            uint32_t Pass = m_pipeline->GetCurrentPass();
            for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
            {
                ENCODE_CHK_STATUS_RETURN(AddOneTileCommands(
                    cmdBuffer,
                    tileRow,
                    tileCol,
                    Pass));
            }
        }

        if(m_pipeline->IsLastPipe())
        {
            // increment the 3rd lvl bb to break successive frames dependency
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IncrementThirdLevelBatchBuffer);

            // Insert end of sequence/stream if set
            if (m_basicFeature->m_lastPicInSeq || m_basicFeature->m_lastPicInStream)
            {
                ENCODE_CHK_STATUS_RETURN(InsertSeqStreamEnd(cmdBuffer));
            }
        }

        // Send VD_CONTROL_STATE (Memory Implict Flush)
        auto &vdControlStateParams               = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
        vdControlStateParams                     = {};
        vdControlStateParams.memoryImplicitFlush = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

        m_flushCmd = waitHevc;
        SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(EnsureAllCommandsExecuted(cmdBuffer));

        // read info from MMIO register in VDENC, incase pak int can't get info
        auto feature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(feature);
        if (m_pipeline->GetPipeNum() <= 1 && !m_pipeline->IsSingleTaskPhaseSupported())
        {
            ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(m_vdboxIndex, m_statusReport, cmdBuffer));
            // BRC PAK statistics different for each pass
            if (feature->IsBRCEnabled())
            {
                uint8_t                     ucPass = (uint8_t)m_pipeline->GetCurrentPass();
                EncodeReadBrcPakStatsParams readBrcPakStatsParams;
                MOS_RESOURCE               *osResource = nullptr;
                uint32_t                    offset     = 0;
                ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportNumberPasses, osResource, offset));
                RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature, SetReadBrcPakStatsParams, ucPass, offset, osResource, readBrcPakStatsParams);
                ReadBrcPakStatistics(&cmdBuffer, &readBrcPakStatsParams);
            }
        }

        // Wait all pipe cmds done for the packet
        auto scalability = m_pipeline->GetMediaScalability();
        ENCODE_CHK_STATUS_RETURN(scalability->SyncPipe(syncOnePipeWaitOthers, 0, &cmdBuffer));

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, &cmdBuffer));

        // post-operations are done by pak integrate pkt

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddPicStateWithNoTile(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        bool tileEnabled = false;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);
        if (tileEnabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);
        auto vdenc2ndLevelBatchBuffer      = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
        vdenc2ndLevelBatchBuffer->dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;

        if (brcFeature->IsBRCUpdateRequired())
        {
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, vdenc2ndLevelBatchBuffer)));
            HalOcaInterfaceNext::OnSubLevelBBStart(
                cmdBuffer,
                m_osInterface->pOsContext,
                &vdenc2ndLevelBatchBuffer->OsResource,
                vdenc2ndLevelBatchBuffer->dwOffset,
                false,
                m_hwInterface->m_vdencBatchBuffer2ndGroupSize);
        }
        // When tile is enabled, below commands are needed for each tile instead of each picture
        else
        {
            SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, &cmdBuffer);

            SETPAR_AND_ADDCMD(HCP_PIC_STATE, m_hcpItf, &cmdBuffer);

            SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, &cmdBuffer);
        }

        auto rdoqFeature = dynamic_cast<HevcEncodeCqp *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcCqpFeature));
        ENCODE_CHK_NULL_RETURN(rdoqFeature);
        if (rdoqFeature->IsRDOQEnabled())
        {
            SETPAR_AND_ADDCMD(HEVC_VP9_RDOQ_STATE, m_hcpItf, &cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddPicStateWithTile(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        bool tileEnabled = false;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);
        if (!tileEnabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);
        auto vdenc2ndLevelBatchBuffer      = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
        vdenc2ndLevelBatchBuffer->dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;

        if (brcFeature->IsBRCUpdateRequired())
        {
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, vdenc2ndLevelBatchBuffer)));
            HalOcaInterfaceNext::OnSubLevelBBStart(
                cmdBuffer,
                m_osInterface->pOsContext,
                &vdenc2ndLevelBatchBuffer->OsResource,
                vdenc2ndLevelBatchBuffer->dwOffset,
                false,
                m_hwInterface->m_vdencBatchBuffer2ndGroupSize);
        }
        // When tile is enabled, below commands are needed for each tile instead of each picture
        else
        {
            PMHW_BATCH_BUFFER thirdLevelBatchBuffer = nullptr;
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetThirdLevelBatchBuffer, thirdLevelBatchBuffer);
            ENCODE_CHK_NULL_RETURN(thirdLevelBatchBuffer);
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, thirdLevelBatchBuffer)));
            HalOcaInterfaceNext::OnSubLevelBBStart(
                cmdBuffer,
                m_osInterface->pOsContext,
                &thirdLevelBatchBuffer->OsResource,
                thirdLevelBatchBuffer->dwOffset,
                false,
                m_hwInterface->m_vdencBatchBuffer2ndGroupSize);
        }

        // Send HEVC_VP9_RDOQ_STATE command
        SETPAR_AND_ADDCMD(HEVC_VP9_RDOQ_STATE, m_hcpItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    void HevcVdencPkt::UpdateParameters()
    {
        ENCODE_FUNC_CALL();

        if (!m_pipeline->IsSingleTaskPhaseSupported())
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_basicFeature->m_currPakSliceIdx = (m_basicFeature->m_currPakSliceIdx + 1) % m_basicFeature->m_codecHalHevcNumPakSliceBatchBuffers;
    }

    MOS_STATUS HevcVdencPkt::UpdateStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        //initialize following
        MOS_RESOURCE *osResourceInline = nullptr;
        uint32_t      offsetInline     = 0;
        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportGlobalCount, osResourceInline, offsetInline));
        offsetInline             = m_atomicScratchBuf.operandSetSize * m_atomicScratchBuf.encodeUpdateIndex;
        uint32_t zeroValueOffset = offsetInline;
        uint32_t operand1Offset  = offsetInline + m_atomicScratchBuf.operand1Offset;
        uint32_t operand2Offset  = offsetInline + m_atomicScratchBuf.operand2Offset;
        uint32_t operand3Offset  = offsetInline + m_atomicScratchBuf.operand3Offset;
        auto     mmioRegisters   = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(m_vdboxIndex);

        // Make Flush DW call to make sure all previous work is done
        auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams       = {};
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
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;  // VCS_GPR4_Hi
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = m_atomicScratchBuf.resAtomicScratchBuffer;
        registerMemParams.dwOffset        = zeroValueOffset;
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4HiOffset;  // VCS_GPR4_Hi
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

        // Make Flush DW call to make sure all previous work is done
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        // step-3: n2 = (n2 == 0:1) ? 0:0 : n2      // uint64_t CMP
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

        int aluCount = 0;

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

        auto &miMathParams          = m_miItf->MHW_GETPAR_F(MI_MATH)();
        miMathParams                = {};
        miMathParams.dwNumAluParams = aluCount;
        miMathParams.pAluPayload    = aluParams;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuffer));

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

    MOS_STATUS HevcVdencPkt::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        // Send MI_FLUSH command
        auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                               = {};
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::InsertSeqStreamEnd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_PAK_INSERT_OBJECT(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddPictureVdencCommands(MOS_COMMAND_BUFFER & cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        SETPAR_AND_ADDCMD(VDENC_PIPE_MODE_SELECT, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_SRC_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_REF_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_DS_REF_SURFACE_STATE, m_vdencItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(VDENC_PIPE_BUF_ADDR_STATE, m_vdencItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddPictureHcpCommands(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(AddHcpPipeModeSelect(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_SURFACE_STATE(&cmdBuffer));

        SETPAR_AND_ADDCMD(HCP_PIPE_BUF_ADDR_STATE, m_hcpItf, &cmdBuffer);

        SETPAR_AND_ADDCMD(HCP_IND_OBJ_BASE_ADDR_STATE, m_hcpItf, &cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_FQM_STATE(&cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_QM_STATE(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddHcpPipeModeSelect(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        SETPAR_AND_ADDCMD(VDENC_CONTROL_STATE, m_vdencItf, &cmdBuffer);

        auto &vdControlStateParams          = m_miItf->MHW_GETPAR_F(VD_CONTROL_STATE)();
        vdControlStateParams                = {};
        vdControlStateParams.initialization = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(VD_CONTROL_STATE)(&cmdBuffer));

        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
        auto &mfxWaitParams                 = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer));

        SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpItf, &cmdBuffer);

        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::CalculatePictureStateCommandSize()
    {
        ENCODE_FUNC_CALL();

        uint32_t hcpCommandsSize  = 0;
        uint32_t hcpPatchListSize = 0;
        uint32_t cpCmdsize        = 0;
        uint32_t cpPatchListSize  = 0;
        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;

        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
        
        hcpCommandsSize =
            m_vdencItf->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)() +
            m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_PIPE_MODE_SELECT)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_SURFACE_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_PIPE_BUF_ADDR_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_IND_OBJ_BASE_ADDR_STATE)() +
            m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_REG)() * 8;

        hcpPatchListSize =
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_SURFACE_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_IND_OBJ_BASE_ADDR_STATE_CMD);
        
        // HCP_QM_STATE_CMD may be issued up to 20 times: 3x Colour Component plus 2x intra/inter plus 4x SizeID minus 4 for the 32x32 chroma components.
        // HCP_FQP_STATE_CMD may be issued up to 8 times: 4 scaling list per intra and inter. 
        hcpCommandsSize +=
            2 * m_miItf->MHW_GETSIZE_F(VD_CONTROL_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_SURFACE_STATE)() +  // encoder needs two surface state commands. One is for raw and another one is for recon surfaces.
            20 * m_hcpItf->MHW_GETSIZE_F(HCP_QM_STATE)() +
            8 * m_hcpItf->MHW_GETSIZE_F(HCP_FQM_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_PIC_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HEVC_VP9_RDOQ_STATE)() +        // RDOQ
            2 * m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() +       // Slice level commands
            2 * m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +             // need for Status report, Mfc Status and
            10 * m_miItf->MHW_GETSIZE_F(MI_STORE_REGISTER_MEM)() +  // 8 for BRCStatistics and 2 for RC6 WAs
            m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_MEM)() +        // 1 for RC6 WA
            2 * m_hcpItf->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)() +  // Two PAK insert object commands are for headers before the slice header and the header for the end of stream
            4 * m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)() +       // two (BRC+reference frame) for clean-up HW semaphore memory and another two for signal it
            17 * m_miItf->MHW_GETSIZE_F(MI_SEMAPHORE_WAIT)() +      // Use HW wait command for each reference and one wait for current semaphore object
            m_miItf->MHW_GETSIZE_F(MI_SEMAPHORE_WAIT)() +           // Use HW wait command for each BRC pass
            +m_miItf->MHW_GETSIZE_F(MI_SEMAPHORE_WAIT)()            // Use HW wait command for each VDBOX
            + 2 * m_miItf->MHW_GETSIZE_F(MI_STORE_DATA_IMM)()       // One is for reset and another one for set per VDBOX
            + 8 * m_miItf->MHW_GETSIZE_F(MI_COPY_MEM_MEM)()         // Need to copy SSE statistics/ Slice Size overflow into memory
            ;

        hcpPatchListSize +=
            20 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_QM_STATE_CMD) +
            8 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_FQM_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIC_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_BATCH_BUFFER_START_CMD) +       // When BRC is on, HCP_PIC_STATE_CMD command is in the BB
            2 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_STORE_DATA_IMM_CMD) +       // Slice level commands
            2 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_FLUSH_DW_CMD) +             // need for Status report, Mfc Status and
            11 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_STORE_REGISTER_MEM_CMD) +  // 8 for BRCStatistics and 3 for RC6 WAs
            22 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_STORE_DATA_IMM_CMD)        // Use HW wait commands plus its memory clean-up and signal (4+ 16 + 1 + 1)
            + 8 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_BATCH_BUFFER_START_CMD)   // At maximal, there are 8 batch buffers for 8 VDBOXes for VE. Each box has one BB.
            + PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_FLUSH_DW_CMD)                 // Need one flush before copy command
            + PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MFX_WAIT_CMD)                    // Need one wait after copy command
            + 3 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_STORE_DATA_IMM_CMD)       // one wait commands and two for reset and set semaphore memory
            + 8 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::MI_COPY_MEM_MEM_CMD)         // Need to copy SSE statistics/ Slice Size overflow into memory
            ;
         
        auto cpInterface = m_hwInterface->GetCpInterface();
        cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t *)&hucCommandsSize, (uint32_t *)&hucPatchListSize, &stateCmdSizeParams));

        m_defaultPictureStatesSize    = hcpCommandsSize + hucCommandsSize + (uint32_t)cpCmdsize;
        m_defaultPicturePatchListSize = hcpPatchListSize + hucPatchListSize + (uint32_t)cpPatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        // VDENC does not use batch buffer for slice state
        // add HCP_REF_IDX command
        ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_REF_IDX_STATE(&cmdBuffer));

        bool              vdencHucInUse    = false;
        PMHW_BATCH_BUFFER vdencBatchBuffer = nullptr;

        RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature, SetVdencBatchBufferState, m_pipeline->m_currRecycledBufIdx, currSlcIdx, vdencBatchBuffer, vdencHucInUse);

        if (vdencHucInUse)
        {
            // 2nd level batch buffer
            PMHW_BATCH_BUFFER secondLevelBatchBufferUsed = vdencBatchBuffer;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, secondLevelBatchBufferUsed)));
            HalOcaInterfaceNext::OnSubLevelBBStart(
                cmdBuffer,
                m_osInterface->pOsContext,
                &secondLevelBatchBufferUsed->OsResource,
                secondLevelBatchBufferUsed->dwOffset,
                false,
                m_basicFeature->m_vdencBatchBufferPerSlicePart2Start[currSlcIdx] - secondLevelBatchBufferUsed->dwOffset);
            ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_PAK_INSERT_OBJECT_BRC(&cmdBuffer));
            secondLevelBatchBufferUsed->dwOffset = m_basicFeature->m_vdencBatchBufferPerSlicePart2Start[currSlcIdx];
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START(&cmdBuffer, secondLevelBatchBufferUsed)));
            HalOcaInterfaceNext::OnSubLevelBBStart(
                cmdBuffer,
                m_osInterface->pOsContext,
                &secondLevelBatchBufferUsed->OsResource,
                secondLevelBatchBufferUsed->dwOffset,
                false,
                m_basicFeature->m_vdencBatchBufferPerSlicePart2Size[currSlcIdx]);
            if (m_basicFeature->m_hevcPicParams->AdaptiveTUEnabled == 0)
            {
                SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, &cmdBuffer);
            }
        }
        else
        {
            // Weighted Prediction
            // This slice level command is issued, if the weighted_pred_flag or weighted_bipred_flag equals one.
            // If zero, then this command is not issued.
            ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_WEIGHTOFFSET_STATE(&cmdBuffer));

            m_basicFeature->m_useDefaultRoundingForHcpSliceState = false;
            SETPAR_AND_ADDCMD(HCP_SLICE_STATE, m_hcpItf, &cmdBuffer);

            // add HCP_PAK_INSERT_OBJECTS command
            ENCODE_CHK_STATUS_RETURN(AddAllCmds_HCP_PAK_INSERT_OBJECT(&cmdBuffer));

            SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, &cmdBuffer);

            SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, &cmdBuffer);
        }
        
        SETPAR_AND_ADDCMD(VDENC_WALKER_STATE, m_vdencItf, &cmdBuffer);
        return eStatus;
    }

MOS_STATUS HevcVdencPkt::AddAllCmds_HCP_PAK_INSERT_OBJECT_BRC(PMOS_COMMAND_BUFFER cmdBuffer) const
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(cmdBuffer);

    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PAK_INSERT_OBJECT)();
    params       = {};

    PCODECHAL_NAL_UNIT_PARAMS *ppNalUnitParams = (CODECHAL_NAL_UNIT_PARAMS **)m_nalUnitParams;

    auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    PBSBuffer pBsBuffer = &(m_basicFeature->m_bsBuffer);
    uint32_t  bitSize   = 0;
    uint32_t  offSet    = 0;

    //insert AU, SPS, PSP headers before first slice header
    if (m_basicFeature->m_curNumSlices == 0)
    {
        uint32_t maxBytesInPakInsertObjCmd = ((2 << 11) - 1) * 4;  // 12 bits for Length field in PAK_INSERT_OBJ cmd

        for (auto i = 0; i < HEVC_MAX_NAL_UNIT_TYPE; i++)
        {
            uint32_t nalunitPosiSize   = ppNalUnitParams[i]->uiSize;
            uint32_t nalunitPosiOffset = ppNalUnitParams[i]->uiOffset;

            while (nalunitPosiSize > 0)
            {
                bitSize = MOS_MIN(maxBytesInPakInsertObjCmd * 8, nalunitPosiSize * 8);
                offSet  = nalunitPosiOffset;

                params = {};

                params.dwPadding                 = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                params.bEmulationByteBitsInsert  = ppNalUnitParams[i]->bInsertEmulationBytes;
                params.uiSkipEmulationCheckCount = ppNalUnitParams[i]->uiSkipEmulationCheckCount;
                params.dataBitsInLastDw          = bitSize % 32;
                if (params.dataBitsInLastDw == 0)
                {
                    params.dataBitsInLastDw = 32;
                }

                if (nalunitPosiSize > maxBytesInPakInsertObjCmd)
                {
                    nalunitPosiSize -= maxBytesInPakInsertObjCmd;
                    nalunitPosiOffset += maxBytesInPakInsertObjCmd;
                }
                else
                {
                    nalunitPosiSize = 0;
                }
                m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
                uint32_t byteSize = (bitSize + 7) >> 3;
                if (byteSize)
                {
                    MHW_MI_CHK_NULL(pBsBuffer);
                    MHW_MI_CHK_NULL(pBsBuffer->pBase);
                    uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

    MOS_STATUS HevcVdencPkt::AddCondBBEndForLastPass(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        if (m_pipeline->IsFirstPass() || m_pipeline->GetPassNum() == 1)
        {
            return MOS_STATUS_SUCCESS;
        }

        bool conditionalPass = true;
        RUN_FEATURE_INTERFACE_RETURN(VdencLplaAnalysis, HevcFeatureIDs::vdencLplaAnalysisFeature, 
            SetConditionalPass, m_pipeline->IsLastPass(), conditionalPass);

        if (conditionalPass)
        {
            auto &miConditionalBatchBufferEndParams = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
            miConditionalBatchBufferEndParams       = {};

            // VDENC uses HuC FW generated semaphore for conditional 2nd pass
            miConditionalBatchBufferEndParams.presSemaphoreBuffer =
                m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);

            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(&cmdBuffer));
        }

        // where is m_encodeStatusBuf?
        auto mmioRegisters = m_hcpItf->GetMmioRegisters(m_vdboxIndex);
        MOS_RESOURCE *osResource = nullptr;
        uint32_t      offset = 0;
        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportImageStatusCtrl, osResource, offset));
        //uint32_t baseOffset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) + sizeof(uint32_t) * 2;  // encodeStatus is offset by 2 DWs in the resource

        // Write back the HCP image control register for RC6 may clean it out
        auto &registerMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        registerMemParams                 = {};
        registerMemParams.presStoreBuffer = osResource;
        registerMemParams.dwOffset        = offset;
        registerMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(&cmdBuffer));
        
        HevcVdencBrcBuffers *vdencBrcBuffers = nullptr;
        auto feature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(feature);
        vdencBrcBuffers = feature->GetHevcVdencBrcBuffers();
        ENCODE_CHK_NULL_RETURN(vdencBrcBuffers);

        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = vdencBrcBuffers->resBrcPakStatisticBuffer[vdencBrcBuffers->currBrcPakStasIdxForWrite];
        miStoreRegMemParams.dwOffset        = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS);
        miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));
        
        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportImageStatusCtrlOfLastBRCPass, osResource, offset));
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = osResource;
        miStoreRegMemParams.dwOffset        = offset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::FreeResources()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        #if USE_CODECHAL_DEBUG_TOOL && _ENCODE_RESERVED
        CODECHAL_DEBUG_TOOL(
            CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
            if (debugInterface && debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar)) {
                m_hevcParDump->DumpParFile();
            })
        #endif
        for (auto j = 0; j < HevcBasicFeature::m_codecHalHevcNumPakSliceBatchBuffers; j++)
        {
            eStatus = Mhw_FreeBb(m_osInterface, &m_batchBufferForPakSlices[j], nullptr);
            ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);
        }

        return eStatus;
    }

    MOS_STATUS HevcVdencPkt::Init()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_statusReport);

        ENCODE_CHK_STATUS_RETURN(CmdPacket::Init());
        m_basicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(HevcFeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

#ifdef _MMC_SUPPORTED
        m_mmcState = m_pipeline->GetMmcState();
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        m_basicFeature->m_mmcState = m_mmcState;
        m_basicFeature->m_ref.m_mmcState = m_mmcState;
#endif
        m_allocator = m_pipeline->GetEncodeAllocator();
        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        ENCODE_CHK_STATUS_RETURN(m_statusReport->RegistObserver(this));

        CalculatePictureStateCommandSize();

        uint32_t vdencPictureStatesSize = 0, vdencPicturePatchListSize = 0;
        GetVdencStateCommandsDataSize(vdencPictureStatesSize, vdencPicturePatchListSize);
        m_defaultPictureStatesSize += vdencPictureStatesSize;
        m_defaultPicturePatchListSize += vdencPicturePatchListSize;

        GetHxxPrimitiveCommandSize();

        m_usePatchList = m_osInterface->bUsesPatchList;

        m_packetUtilities = m_pipeline->GetPacketUtilities();
        ENCODE_CHK_NULL_RETURN(m_packetUtilities);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::SetRowstoreCachingOffsets()
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowStoreParams;

        rowStoreParams.Mode             = m_basicFeature->m_mode;
        rowStoreParams.dwPicWidth       = m_basicFeature->m_frameWidth;
        rowStoreParams.ucChromaFormat   = m_basicFeature->m_chromaFormat;
        rowStoreParams.ucBitDepthMinus8 = m_hevcSeqParams->bit_depth_luma_minus8;
        rowStoreParams.ucLCUSize        = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
        // VDEnc only support LCU64 for now
        ENCODE_ASSERT(rowStoreParams.ucLCUSize == m_basicFeature->m_maxLCUSize);
        ENCODE_CHK_STATUS_RETURN(m_hwInterface->SetRowstoreCachingOffsets(&rowStoreParams));

        if (m_vdencItf)
        {
            mhw::vdbox::vdenc::RowStorePar par = {};

            par.mode = mhw::vdbox::vdenc::RowStorePar::HEVC;
            par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_8;
            if (rowStoreParams.ucBitDepthMinus8 == 1 || rowStoreParams.ucBitDepthMinus8 == 2)
            {
                par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_10;
            }
            else if (rowStoreParams.ucBitDepthMinus8 > 2)
            {
                par.bitDepth = mhw::vdbox::vdenc::RowStorePar::DEPTH_12;
            }
            par.lcuSize = mhw ::vdbox::vdenc::RowStorePar::SIZE_OTHER;
            if (rowStoreParams.ucLCUSize == 32)
            {
                par.lcuSize = mhw ::vdbox::vdenc::RowStorePar::SIZE_32;
            }
            else if (rowStoreParams.ucLCUSize == 64)
            {
                par.lcuSize = mhw ::vdbox::vdenc::RowStorePar::SIZE_64;
            }
            par.frameWidth = rowStoreParams.dwPicWidth;
            switch (rowStoreParams.ucChromaFormat)
            {
            case HCP_CHROMA_FORMAT_MONOCHROME:
                par.format = mhw ::vdbox::vdenc::RowStorePar::MONOCHROME;
                break;
            case HCP_CHROMA_FORMAT_YUV420:
                par.format = mhw ::vdbox::vdenc::RowStorePar::YUV420;
                break;
            case HCP_CHROMA_FORMAT_YUV422:
                par.format = mhw ::vdbox::vdenc::RowStorePar::YUV422;
                break;
            case HCP_CHROMA_FORMAT_YUV444:
                par.format = mhw ::vdbox::vdenc::RowStorePar::YUV444;
                break;
            }

            ENCODE_CHK_STATUS_RETURN(m_vdencItf->SetRowstoreCachingOffsets(par));
        }

        hcp::HcpVdboxRowStorePar rowstoreParams = {};
        rowstoreParams.Mode                     = m_basicFeature->m_mode;
        rowstoreParams.dwPicWidth               = m_basicFeature->m_frameWidth;
        rowstoreParams.ucChromaFormat           = m_basicFeature->m_chromaFormat;
        rowstoreParams.ucBitDepthMinus8         = m_hevcSeqParams->bit_depth_luma_minus8;
        rowstoreParams.ucLCUSize                = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
        // VDEnc only support LCU64 for now
        ENCODE_ASSERT(rowstoreParams.ucLCUSize == m_basicFeature->m_maxLCUSize);
        m_hcpItf->SetRowstoreCachingOffsets(rowstoreParams);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::Destroy()
    {
        m_statusReport->UnregistObserver(this);
        return MOS_STATUS_SUCCESS;
    }

    void HevcVdencPkt::SetPakPassType()
    {
        ENCODE_FUNC_CALL();

        // default: VDEnc+PAK pass
        m_pakOnlyPass = false;

        return;
    }

    // Inline functions
    MOS_STATUS HevcVdencPkt::ValidateVdboxIdx(const MHW_VDBOX_NODE_IND &vdboxIndex)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        if (vdboxIndex > m_hwInterface->GetMaxVdboxIndex())
        {
            ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }

        return eStatus;
    }

    void HevcVdencPkt::SetPerfTag()
    {
        ENCODE_FUNC_CALL();

        uint16_t callType = m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE : CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE_SECOND_PASS;
        uint16_t picType  = m_basicFeature->m_pictureCodingType;
        if (m_basicFeature->m_pictureCodingType == B_TYPE && m_basicFeature->m_ref.IsLowDelay())
        {
            picType = 0;
        }

        PerfTagSetting perfTag;
        perfTag.Value             = 0;
        perfTag.Mode              = (uint16_t)m_basicFeature->m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
        perfTag.CallType          = callType;
        perfTag.PictureCodingType = picType;
        m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
        m_osInterface->pfnIncPerfBufferID(m_osInterface);
    }

    MOS_STATUS HevcVdencPkt::SetSemaphoreMem(
        MOS_RESOURCE &      semaphoreMem,
        uint32_t            value,
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = &semaphoreMem;
        storeDataParams.dwResourceOffset = 0;
        storeDataParams.dwValue          = value;

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::SendPrologCmds(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        auto packetUtilities = m_pipeline->GetPacketUtilities();
        ENCODE_CHK_NULL_RETURN(packetUtilities);
        if (m_basicFeature->m_setMarkerEnabled)
        {
            PMOS_RESOURCE presSetMarker = m_osInterface->pfnGetMarkerResource(m_osInterface);
            ENCODE_CHK_STATUS_RETURN(packetUtilities->SendMarkerCommand(&cmdBuffer, presSetMarker));
        }

#ifdef _MMC_SUPPORTED
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        ENCODE_CHK_STATUS_RETURN(m_mmcState->SendPrologCmd(&cmdBuffer, false));
#endif

        MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
        MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
        genericPrologParams.pOsInterface  = m_osInterface;
        genericPrologParams.pvMiInterface = nullptr;
        genericPrologParams.bMmcEnabled   = m_mmcState ? m_mmcState->IsMmcEnabled() : false;
        ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(&cmdBuffer, &genericPrologParams, m_miItf));

        // Send predication command
        if (m_basicFeature->m_predicationEnabled)
        {
            ENCODE_CHK_STATUS_RETURN(packetUtilities->SendPredicationCommand(&cmdBuffer));
        }

        return eStatus;
    }

    MOS_STATUS HevcVdencPkt::AllocateBatchBufferForPakSlices(
        uint32_t numSlices,
        uint16_t numPakPasses)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        MOS_ZeroMemory(
            &m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx],
            sizeof(MHW_BATCH_BUFFER));

        // Get the slice size
        uint32_t size = numPakPasses * numSlices * m_sliceStatesSize;

        m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx].bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx],
            nullptr,
            size));

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t *data       = (uint8_t *)m_allocator->LockResourceForWrite(&m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx].OsResource);

        if (data == nullptr)
        {
            ENCODE_ASSERTMESSAGE("Failed to lock batch buffer for PAK slices.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }

        m_allocator->UnLock(&m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx].OsResource);

        return eStatus;
    }

    MOS_STATUS HevcVdencPkt::ReadExtStatistics(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        PMOS_RESOURCE osResource = nullptr;
        uint32_t      offset     = 0;

        ENCODE_CHK_STATUS_RETURN(m_statusReport->GetAddress(statusReportSumSquareError, osResource, offset));

        for (auto i = 0; i < 3; i++)  // 64 bit SSE values for luma/ chroma channels need to be copied
        {
            auto &miCpyMemMemParams = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
            miCpyMemMemParams       = {};
            MOS_RESOURCE *resHuCPakAggregatedFrameStatsBuffer = nullptr;
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetHucPakAggregatedFrameStatsBuffer, resHuCPakAggregatedFrameStatsBuffer);
            ENCODE_CHK_NULL_RETURN(resHuCPakAggregatedFrameStatsBuffer);
            miCpyMemMemParams.presSrc     = m_hevcPicParams->tiles_enabled_flag && (m_pipeline->GetPipeNum() > 1) ? resHuCPakAggregatedFrameStatsBuffer : m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);
            miCpyMemMemParams.dwSrcOffset = (m_basicFeature->m_hevcPakStatsSSEOffset + i) * sizeof(uint32_t);  // SSE luma offset is located at DW32 in Frame statistics, followed by chroma
            miCpyMemMemParams.presDst     = osResource;
            miCpyMemMemParams.dwDstOffset = offset + i * sizeof(uint32_t);
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(&cmdBuffer));
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        auto &forceWakeupParams                     = m_miItf->MHW_GETPAR_F(MI_FORCE_WAKEUP)();
        forceWakeupParams                           = {};
        forceWakeupParams.bMFXPowerWellControl      = true;
        forceWakeupParams.bMFXPowerWellControlMask  = true;
        forceWakeupParams.bHEVCPowerWellControl     = true;
        forceWakeupParams.bHEVCPowerWellControlMask = true;

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FORCE_WAKEUP)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::SetBatchBufferForPakSlices()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        if (m_hevcPicParams->tiles_enabled_flag)
        {
            return eStatus;
        }

        m_useBatchBufferForPakSlices         = m_pipeline->IsSingleTaskPhaseSupported() && m_pipeline->IsSingleTaskPhaseSupportedInPak();
        m_batchBufferForPakSlicesStartOffset = 0;

        if (m_useBatchBufferForPakSlices)
        {
            if (m_pipeline->IsFirstPass())
            {
                // The same buffer is used for all slices for all passes
                uint32_t batchBufferForPakSlicesSize =
                    m_pipeline->GetPassNum() * m_basicFeature->m_numSlices * m_sliceStatesSize;

                ENCODE_ASSERT(batchBufferForPakSlicesSize);

                if (batchBufferForPakSlicesSize >
                    (uint32_t)m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx].iSize)
                {
                    if (m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx].iSize)
                    {
                        Mhw_FreeBb(m_osInterface, &m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx], nullptr);
                        m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx].iSize = 0;
                    }

                    ENCODE_CHK_STATUS_RETURN(AllocateBatchBufferForPakSlices(
                        m_basicFeature->m_numSlices,
                        m_pipeline->GetPassNum()));
                }
            }

            ENCODE_CHK_STATUS_RETURN(Mhw_LockBb(
                m_osInterface,
                &m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx]));

            m_batchBufferForPakSlicesStartOffset =
                m_pipeline->IsFirstPass() ? 0 : (uint32_t)m_batchBufferForPakSlices[m_basicFeature->m_currPakSliceIdx].iCurrent;
        }

        return eStatus;
    }

    MOS_STATUS HevcVdencPkt::StartStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(MediaPacket::StartStatusReportNext(srType, cmdBuffer));
        m_encodecp->StartCpStatusReport(cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::EndStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::ReadHcpStatus(
        MHW_VDBOX_NODE_IND  vdboxIndex,
        MediaStatusReport * statusReport,
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        CODEC_HW_FUNCTION_ENTER;

        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        MOS_RESOURCE *osResource;
        uint32_t      offset;

        EncodeStatusReadParams params;
        MOS_ZeroMemory(&params, sizeof(params));

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportMfxBitstreamByteCountPerFrame, osResource, offset));
        params.resBitstreamByteCountPerFrame    = osResource;
        params.bitstreamByteCountPerFrameOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportMfxBitstreamSyntaxElementOnlyBitCount, osResource, offset));
        params.resBitstreamSyntaxElementOnlyBitCount    = osResource;
        params.bitstreamSyntaxElementOnlyBitCountOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportQPStatusCount, osResource, offset));
        params.resQpStatusCount    = osResource;
        params.qpStatusCountOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportImageStatusMask, osResource, offset));
        params.resImageStatusMask    = osResource;
        params.imageStatusMaskOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportImageStatusCtrl, osResource, offset));
        params.resImageStatusCtrl    = osResource;
        params.imageStatusCtrlOffset = offset;

        ENCODE_CHK_STATUS_RETURN(statusReport->GetAddress(encode::statusReportNumSlices, osResource, offset));
        params.resNumSlices    = osResource;
        params.numSlicesOffset = offset;

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->ReadHcpStatus(vdboxIndex, params, &cmdBuffer));

        // Slice Size Conformance
        if (m_hevcSeqParams->SliceSizeControl)
        {
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeDss, HevcFeatureIDs::hevcVdencDssFeature, ReadHcpStatus, vdboxIndex, cmdBuffer);
        }

        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);
        bool vdencHucUsed  = brcFeature->IsVdencHucUsed();
        auto mmioRegisters = m_hcpItf->GetMmioRegisters(m_vdboxIndex);
        if (vdencHucUsed)
        {
            // Store PAK frameSize MMIO to PakInfo buffer
            auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
            miStoreRegMemParams                 = {};
            miStoreRegMemParams.presStoreBuffer = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
            miStoreRegMemParams.dwOffset        = 0;
            miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));
        }
        ENCODE_CHK_STATUS_RETURN(m_hwInterface->ReadImageStatusForHcp(vdboxIndex, params, &cmdBuffer));
        return eStatus;
    }

    MOS_STATUS HevcVdencPkt::ReadSliceSizeForSinglePipe(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        // Report slice size to app only when dynamic slice is enabled
        if (!m_hevcSeqParams->SliceSizeControl)
        {
            return eStatus;
        }
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeDss, HevcFeatureIDs::hevcVdencDssFeature, ReadSliceSizeForSinglePipe, m_pipeline, cmdBuffer);

        return eStatus;
    }

    MOS_STATUS HevcVdencPkt::ReadSliceSize(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        // Use FrameStats buffer if in single pipe mode.
        if (m_pipeline->GetPipeNum() == 1)
        {
            return ReadSliceSizeForSinglePipe(cmdBuffer);
        }

        // In multi-tile multi-pipe mode, use PAK integration kernel output
        // PAK integration kernel accumulates frame statistics across tiles, which should be used to setup slice size report
        // Report slice size to app only when dynamic scaling is enabled
        if (!m_hevcSeqParams->SliceSizeControl)
        {
            return eStatus;
        }

        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeDss, HevcFeatureIDs::hevcVdencDssFeature, ReadSliceSize, m_pipeline, cmdBuffer);

        return eStatus;
    }

    MOS_STATUS HevcVdencPkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        if (!m_enableVdencStatusReport)
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_NULL_RETURN(mfxStatus);
        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        EncodeStatusMfx *       encodeStatusMfx  = (EncodeStatusMfx *)mfxStatus;
        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;
        if (statusReportData->hwCtr)
        {
            m_encodecp->UpdateCpStatusReport(statusReport);
        }

        // The last pass of BRC may have a zero value of hcpCumulativeFrameDeltaQp
        if (encodeStatusMfx->imageStatusCtrl.hcpTotalPass && encodeStatusMfx->imageStatusCtrl.hcpCumulativeFrameDeltaQP == 0)
        {
            encodeStatusMfx->imageStatusCtrl.hcpCumulativeFrameDeltaQP = encodeStatusMfx->imageStatusCtrlOfLastBRCPass.hcpCumulativeFrameDeltaQP;
        }
        encodeStatusMfx->imageStatusCtrlOfLastBRCPass.hcpCumulativeFrameDeltaQP = 0;

        statusReportData->codecStatus   = CODECHAL_STATUS_SUCCESSFUL;
        statusReportData->bitstreamSize = encodeStatusMfx->mfcBitstreamByteCountPerFrame + encodeStatusMfx->headerBytesInserted;

        statusReportData->numberSlices      = 0;
        statusReportData->panicMode         = encodeStatusMfx->imageStatusCtrl.panic;
        statusReportData->averageQP         = 0;
        statusReportData->qpY               = 0;
        statusReportData->suggestedQPYDelta = encodeStatusMfx->imageStatusCtrl.hcpCumulativeFrameDeltaQP;
        statusReportData->numberPasses      = (unsigned char)encodeStatusMfx->imageStatusCtrl.hcpTotalPass + 1;  //initial pass is considered to be 0,hence +1 to report;
        ENCODE_VERBOSEMESSAGE("Exectued PAK Pass number: %d\n", encodeStatusMfx->numberPasses);

        if (m_basicFeature->m_frameWidth != 0 && m_basicFeature->m_frameHeight != 0)
        {
            ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcSeqParams);

            uint32_t log2CBSize = 2;

            // Based on HW team:
            // The CumulativeQp from the PAK accumulated at TU level and normalized to TU4x4
            // qp(for TU 8x8) = qp*4
            // qp(for TU 16x16) = qp *16
            // qp(for TU 32x32) = qp*64
            // all these qp are accumulated for entire frame.
            // the HW will ceil the CumulativeQp number to max (24 bit)

            uint32_t log2McuSize = m_basicFeature->m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;

            uint32_t numLumaPixels = ((m_basicFeature->m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << log2McuSize) *
                            ((m_basicFeature->m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << log2McuSize);

            statusReportData->qpY = statusReportData->averageQP = static_cast<uint8_t>(
                static_cast<double>(encodeStatusMfx->qpStatusCount.hcpCumulativeQP)
                / (numLumaPixels / 16) - (m_basicFeature->m_hevcSeqParams->bit_depth_luma_minus8 != 0) * 12);
        }

        // When tile replay is enabled with tile replay, need to report out the tile size and the bit stream is not continous
        if (m_pipeline->GetPipeNum() == 1)
        {
            //ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::GetStatusReport(encodeStatus, encodeStatusReport));
            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.ReadOnly = 1;

            uint32_t *sliceSize = nullptr;
            // pSliceSize is set/ allocated only when dynamic slice is enabled. Cannot use SSC flag here, as it is an asynchronous call
            if (encodeStatusMfx->sliceReport.sliceSize)
            {
                sliceSize = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, encodeStatusMfx->sliceReport.sliceSize, &lockFlags);
                ENCODE_CHK_NULL_RETURN(sliceSize);

                statusReportData->numberSlices           = encodeStatusMfx->sliceReport.numberSlices;
                statusReportData->sizeOfSliceSizesBuffer = sizeof(uint16_t) * encodeStatusMfx->sliceReport.numberSlices;
                statusReportData->sliceSizeOverflow      = (encodeStatusMfx->sliceReport.sliceSizeOverflow >> 16) & 1;
                statusReportData->sliceSizes             = (uint16_t *)sliceSize;

                uint16_t prevCumulativeSliceSize = 0;
                // HW writes out a DW for each slice size. Copy in place the DW into 16bit fields expected by App
                for (auto sliceCount = 0; sliceCount < encodeStatusMfx->sliceReport.numberSlices; sliceCount++)
                {
                    // PAK output the sliceSize at 16DW intervals.
                    ENCODE_CHK_NULL_RETURN(&sliceSize[sliceCount * 16]);
                    uint32_t CurrAccumulatedSliceSize = sliceSize[sliceCount * 16];

                    //convert cummulative slice size to individual, first slice may have PPS/SPS,
                    statusReportData->sliceSizes[sliceCount] = CurrAccumulatedSliceSize - prevCumulativeSliceSize;
                    prevCumulativeSliceSize += statusReportData->sliceSizes[sliceCount];
                }
                m_osInterface->pfnUnlockResource(m_osInterface, encodeStatusMfx->sliceReport.sliceSize);
            }
        }

        ENCODE_CHK_STATUS_RETURN(ReportExtStatistics(*encodeStatusMfx, *statusReportData));

        CODECHAL_DEBUG_TOOL(
            ENCODE_CHK_STATUS_RETURN(DumpResources(encodeStatusMfx, statusReportData)););

        if (statusReportData->numberTilesInFrame > 1)
        {
            // When Tile feature enabled, Reset is not in vdenc packet
            return MOS_STATUS_SUCCESS;
        }

        m_basicFeature->Reset((CODEC_REF_LIST *)statusReportData->currRefList);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::ReportExtStatistics(
        EncodeStatusMfx        &encodeStatusMfx,
        EncodeStatusReportData &statusReportData)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        uint32_t numLumaPixels = 0, numPixelsPerChromaChannel = 0;

        numLumaPixels = m_basicFeature->m_frameHeight * m_basicFeature->m_frameWidth;
        switch (m_basicFeature->m_hevcSeqParams->chroma_format_idc)
        {
        case HCP_CHROMA_FORMAT_MONOCHROME:
            numPixelsPerChromaChannel = 0;
            break;
        case HCP_CHROMA_FORMAT_YUV420:
            numPixelsPerChromaChannel = numLumaPixels / 4;
            break;
        case HCP_CHROMA_FORMAT_YUV422:
            numPixelsPerChromaChannel = numLumaPixels / 2;
            break;
        case HCP_CHROMA_FORMAT_YUV444:
            numPixelsPerChromaChannel = numLumaPixels;
            break;
        default:
            numPixelsPerChromaChannel = numLumaPixels / 2;
            break;
        }

        double squarePeakPixelValue = pow((1 << (m_basicFeature->m_hevcSeqParams->bit_depth_luma_minus8 + 8)) - 1, 2);

        for (auto i = 0; i < 3; i++)
        {
            uint32_t numPixels = i ? numPixelsPerChromaChannel : numLumaPixels;

            if (m_basicFeature->m_hevcSeqParams->bit_depth_luma_minus8 == 0)
            {
                //8bit pixel data is represented in 10bit format in HW. so SSE should right shift by 4.
                encodeStatusMfx.sumSquareError[i] >>= 4;
            }
            statusReportData.psnrX100[i] = (uint16_t)CodecHal_Clip3(0, 10000, (uint16_t)(encodeStatusMfx.sumSquareError[i] ? 1000 * log10(squarePeakPixelValue * numPixels / encodeStatusMfx.sumSquareError[i]) : -1));

            ENCODE_VERBOSEMESSAGE("psnrX100[%d]:%d.\n", i, statusReportData.psnrX100[i]);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::GetVdencStateCommandsDataSize(uint32_t &vdencPictureStatesSize, uint32_t &vdencPicturePatchListSize)
    {
        vdencPictureStatesSize =
            m_vdencItf->MHW_GETSIZE_F(VDENC_PIPE_MODE_SELECT)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_SRC_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_REF_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_DS_REF_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_PIPE_BUF_ADDR_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_WEIGHTSOFFSETS_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_WALKER_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)() +
            m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_IMM)()*8 +
            m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() +
            m_hcpItf->MHW_GETSIZE_F(HEVC_VP9_RDOQ_STATE)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

        vdencPicturePatchListSize = PATCH_LIST_COMMAND(mhw::vdbox::vdenc::Itf::VDENC_PIPE_BUF_ADDR_STATE_CMD);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::GetHxxPrimitiveCommandSize()
    {
        uint32_t hcpCommandsSize  = 0;
        uint32_t hcpPatchListSize = 0;
        hcpCommandsSize =
            m_hcpItf->MHW_GETSIZE_F(HCP_REF_IDX_STATE)() * 2 +
            m_hcpItf->MHW_GETSIZE_F(HCP_WEIGHTOFFSET_STATE)() * 2 +
            m_hcpItf->MHW_GETSIZE_F(HCP_SLICE_STATE)() +
            m_hcpItf->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() * 2 +
            m_hcpItf->MHW_GETSIZE_F(HCP_TILE_CODING)();  // one slice cannot be with more than one tile

        hcpPatchListSize =
            mhw::vdbox::hcp::Itf::HCP_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES * 2 +
            mhw::vdbox::hcp::Itf::HCP_WEIGHTOFFSET_STATE_CMD_NUMBER_OF_ADDRESSES * 2 +
            mhw::vdbox::hcp::Itf::HCP_SLICE_STATE_CMD_NUMBER_OF_ADDRESSES +
            mhw::vdbox::hcp::Itf::HCP_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES +
            mhw::vdbox::hcp::Itf::MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES * 2 +  // One is for the PAK command and another one is for the BB when BRC and single task mode are on
            mhw::vdbox::hcp::Itf::HCP_TILE_CODING_COMMAND_NUMBER_OF_ADDRESSES;         // HCP_TILE_CODING_STATE command

        uint32_t cpCmdsize = 0;
        uint32_t cpPatchListSize = 0;
        if (m_hwInterface->GetCpInterface())
        {
            m_hwInterface->GetCpInterface()->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
        }

        m_defaultSliceStatesSize = hcpCommandsSize + (uint32_t)cpCmdsize;
        m_defaultSlicePatchListSize = hcpPatchListSize + (uint32_t)cpPatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        m_pictureStatesSize    = m_defaultPictureStatesSize;
        m_picturePatchListSize = m_defaultPicturePatchListSize;
        m_sliceStatesSize      = m_defaultSliceStatesSize;
        m_slicePatchListSize   = m_defaultSlicePatchListSize;

        commandBufferSize      = CalculateCommandBufferSize();
        requestedPatchListSize = CalculatePatchListSize();
        return MOS_STATUS_SUCCESS;
    }

    uint32_t HevcVdencPkt::CalculateCommandBufferSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t commandBufferSize = 0;

        // To be refined later, differentiate BRC and CQP
        commandBufferSize =
            m_pictureStatesSize +
            (m_sliceStatesSize * m_basicFeature->m_numSlices);

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return commandBufferSize;
    }

    uint32_t HevcVdencPkt::CalculatePatchListSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t requestedPatchListSize = 0;
        if (m_usePatchList)
        {
            requestedPatchListSize =
                m_picturePatchListSize +
                (m_slicePatchListSize * m_basicFeature->m_numSlices);

            // Multi pipes are sharing one patchlist
            requestedPatchListSize *= m_pipeline->GetPipeNum();
        }
        return requestedPatchListSize;
    }

    MOS_STATUS HevcVdencPkt::ReadBrcPakStatistics(
        PMOS_COMMAND_BUFFER          cmdBuffer,
        EncodeReadBrcPakStatsParams *params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);
        ENCODE_CHK_NULL_RETURN(params);
        ENCODE_CHK_NULL_RETURN(params->presBrcPakStatisticBuffer);
        ENCODE_CHK_NULL_RETURN(params->presStatusBuffer);

        ENCODE_CHK_STATUS_RETURN(ValidateVdboxIdx(m_vdboxIndex));
        auto mmioRegisters = m_hcpItf->GetMmioRegisters(m_vdboxIndex);

        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
        miStoreRegMemParams.dwOffset        = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_BITSTREAM_BYTECOUNT_FRAME);
        miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
        miStoreRegMemParams.dwOffset        = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_BITSTREAM_BYTECOUNT_FRAME_NOHEADER);
        miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncBitstreamBytecountFrameNoHeaderRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
        miStoreRegMemParams.dwOffset        = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL);
        miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = params->presStatusBuffer;
        storeDataParams.dwResourceOffset = params->dwStatusBufNumPassesOffset;
        storeDataParams.dwValue          = params->ucPass;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        return eStatus;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CONTROL_STATE, HevcVdencPkt)
    {
        params.vdencInitialization = true;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, HevcVdencPkt)
    {
        //params.tlbPrefetch = true;

        params.pakObjCmdStreamOut = m_vdencPakObjCmdStreamOutForceEnabled? true : m_hevcPicParams->StatusReportEnable.fields.BlockStats;

        // needs to be enabled for 1st pass in multi-pass case
        // This bit is ignored if PAK only second pass is enabled.
        if ((m_pipeline->GetCurrentPass() == 0) && !m_pipeline->IsLastPass()
            || (m_basicFeature->m_422State && m_basicFeature->m_422State->GetFeature422Flag())
        )
        {
            params.pakObjCmdStreamOut = true;
        }

        if (!MEDIA_IS_WA(m_osInterface->pfnGetWaTable(m_osInterface), WaEnableOnlyASteppingFeatures))
        {
            params.VdencPipeModeSelectPar0 = 1;
        }

        MHW_VDBOX_HCP_MULTI_ENGINE_MODE multiEngineMode;
        if (m_pipeline->GetPipeNum() > 1)
        {
            // Running in the multiple VDBOX mode
            if (m_pipeline->IsFirstPipe())
            {
                multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;
            }
            else if (m_pipeline->IsLastPipe())
            {
                multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;
            }
            else
            {
                multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;
            }
        }
        else
        {
            multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
        }

        // Enable RGB encoding
        params.rgbEncodingMode = false;
        params.scalabilityMode = !(multiEngineMode == MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY);

        auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
        ENCODE_CHK_NULL_RETURN(waTable);

        if (MEDIA_IS_WA(waTable, Wa_22011549751) &&
            !m_osInterface->bSimIsActive &&
            !m_basicFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
        {
            params.hmeRegionPrefetch = m_basicFeature->m_hevcPicParams->CodingType != I_TYPE;
        }

        if (MEDIA_IS_WA(waTable, Wa_14012254246))
        {
            params.hmeRegionPrefetch        = 0;
            params.leftPrefetchAtWrapAround = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, HevcVdencPkt)
    {
        params.intraRowStoreScratchBuffer       = m_vdencIntraRowStoreScratch;
        params.tileRowStoreBuffer               = m_vdencTileRowStoreBuffer;
        params.cumulativeCuCountStreamOutBuffer = m_resCumulativeCuCountStreamoutBuffer;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, HevcVdencPkt)
    {
        switch (m_flushCmd)
        {
        case waitHevc:
            params.waitDoneHEVC           = true;
            params.flushHEVC              = true;
            params.waitDoneVDCmdMsgParser = true;
            break;
        case waitVdenc:
            params.waitDoneMFX            = true;
            params.waitDoneVDENC          = true;
            params.flushVDENC             = true;
            params.waitDoneVDCmdMsgParser = true;
            break;
        case waitHevcVdenc:
            params.waitDoneMFX            = true;
            params.waitDoneVDENC          = true;
            params.flushVDENC             = true;
            params.flushHEVC              = true;
            params.waitDoneVDCmdMsgParser = true;
            break;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_SURFACE_STATE, HevcVdencPkt)
    {
        params.surfaceStateId = m_curHcpSurfStateId;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, HevcVdencPkt)
    {
        uint32_t dwNumberOfPipes = 0;
        switch (m_pipeline->GetPipeNum())
        {
        case 0:
        case 1:
            dwNumberOfPipes = VDENC_PIPE_SINGLE_PIPE;
            break;
        case 2:
            dwNumberOfPipes = VDENC_PIPE_TWO_PIPE;
            break;
        case 4:
            dwNumberOfPipes = VDENC_PIPE_FOUR_PIPE;
            break;
        default:
            dwNumberOfPipes = VDENC_PIPE_INVALID;
            ENCODE_ASSERT(false);
            break;
        }

        params.numPipe = dwNumberOfPipes;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::PrepareHWMetaData(MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        if (!m_basicFeature->m_resMetadataBuffer)
        {
            return MOS_STATUS_SUCCESS;
        }

        // Intra/Inter/Skip CU Cnt
        auto xCalAtomic = [&](PMOS_RESOURCE presDst, uint32_t dstOffset, PMOS_RESOURCE presSrc, uint32_t srcOffset, mhw::mi::MHW_COMMON_MI_ATOMIC_OPCODE opCode) {
            auto  mmioRegisters      = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(m_vdboxIndex);
            auto &miLoadRegMemParams = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
            auto &flushDwParams      = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
            auto &atomicParams       = m_miItf->MHW_GETPAR_F(MI_ATOMIC)();

            miLoadRegMemParams = {};
            flushDwParams      = {};
            atomicParams       = {};

            miLoadRegMemParams.presStoreBuffer = presSrc;
            miLoadRegMemParams.dwOffset        = srcOffset;
            miLoadRegMemParams.dwRegister      = mmioRegisters->generalPurposeRegister0LoOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuffer));

            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

            atomicParams.pOsResource      = presDst;
            atomicParams.dwResourceOffset = dstOffset;
            atomicParams.dwDataSize       = sizeof(uint32_t);
            atomicParams.Operation        = opCode;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));

            return MOS_STATUS_SUCCESS;
        };

        MetaDataOffset resourceOffset = m_basicFeature->m_metaDataOffset;
        PMOS_RESOURCE  resLcuBaseAddressBuffer = m_basicFeature->m_recycleBuf->GetBuffer(LcuBaseAddressBuffer, 0);
        ENCODE_CHK_NULL_RETURN(resLcuBaseAddressBuffer);

        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_basicFeature->m_resMetadataBuffer;
        storeDataParams.dwResourceOffset = resourceOffset.dwEncodeErrorFlags;
        storeDataParams.dwValue          = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        storeDataParams.dwResourceOffset = resourceOffset.dwWrittenSubregionsCount;
        storeDataParams.dwValue          = m_basicFeature->m_numSlices;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        auto &miCpyMemMemParams   = m_miItf->MHW_GETPAR_F(MI_COPY_MEM_MEM)();
        miCpyMemMemParams         = {};
        miCpyMemMemParams.presSrc = resLcuBaseAddressBuffer;
        miCpyMemMemParams.presDst = m_basicFeature->m_resMetadataBuffer;

        for (uint16_t slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
        {
            uint32_t subRegionStartOffset = resourceOffset.dwMetaDataSize + slcCount * resourceOffset.dwMetaDataSubRegionSize;

            storeDataParams.dwResourceOffset = subRegionStartOffset + resourceOffset.dwbStartOffset;
            storeDataParams.dwValue          = 0;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

            storeDataParams.dwResourceOffset = subRegionStartOffset + resourceOffset.dwbHeaderSize;
            storeDataParams.dwValue          = m_basicFeature->m_slcData[slcCount].BitSize;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

            miCpyMemMemParams.presSrc     = resLcuBaseAddressBuffer;
            miCpyMemMemParams.presDst     = m_basicFeature->m_resMetadataBuffer;
            miCpyMemMemParams.dwSrcOffset = slcCount * 16 * sizeof(uint32_t);  //slice size offset in resLcuBaseAddressBuffer is 16DW
            miCpyMemMemParams.dwDstOffset = subRegionStartOffset + resourceOffset.dwbSize;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
            if (slcCount)
            {
                ENCODE_CHK_STATUS_RETURN(xCalAtomic(
                    m_basicFeature->m_resMetadataBuffer, 
                    subRegionStartOffset + resourceOffset.dwbSize, 
                    resLcuBaseAddressBuffer, 
                    (slcCount - 1) * 16 * sizeof(uint32_t), 
                    mhw::mi::MHW_MI_ATOMIC_SUB));
            }
        }

        auto mmioRegisters                = m_hcpItf->GetMmioRegisters(m_vdboxIndex);
        auto &storeRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        storeRegMemParams                 = {};
        storeRegMemParams.presStoreBuffer = m_basicFeature->m_resMetadataBuffer;
        storeRegMemParams.dwOffset        = resourceOffset.dwEncodedBitstreamWrittenBytesCount;
        storeRegMemParams.dwRegister      = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));

        // Statistics
        // Average QP
        if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP)
        {
            storeDataParams.dwResourceOffset = resourceOffset.dwEncodeStats + resourceOffset.dwAverageQP;
            storeDataParams.dwValue          = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));
        }
        else
        {
            auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
            ENCODE_CHK_NULL_RETURN(brcFeature);

            miCpyMemMemParams.presSrc     = brcFeature->GetHevcVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
            miCpyMemMemParams.dwSrcOffset = 0x6F * sizeof(uint32_t);
            miCpyMemMemParams.presDst     = m_basicFeature->m_resMetadataBuffer;
            miCpyMemMemParams.dwDstOffset = resourceOffset.dwEncodeStats + resourceOffset.dwAverageQP;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));

            auto &atomicParams             = m_miItf->MHW_GETPAR_F(MI_ATOMIC)();
            atomicParams                   = {};
            atomicParams.pOsResource       = m_basicFeature->m_resMetadataBuffer;
            atomicParams.dwResourceOffset  = resourceOffset.dwEncodeStats + resourceOffset.dwAverageQP;
            atomicParams.dwDataSize        = sizeof(uint32_t);
            atomicParams.Operation         = mhw::mi::MHW_MI_ATOMIC_AND;
            atomicParams.bInlineData       = true;
            atomicParams.dwOperand1Data[0] = 0xFF;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_ATOMIC)(cmdBuffer));
        }

        PMOS_RESOURCE resFrameStatStreamOutBuffer = m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);
        ENCODE_CHK_NULL_RETURN(resFrameStatStreamOutBuffer);

        // LCUSkipIn8x8Unit
        miCpyMemMemParams.presSrc     = resFrameStatStreamOutBuffer;
        miCpyMemMemParams.dwSrcOffset = 7 * sizeof(uint32_t);
        miCpyMemMemParams.presDst     = m_basicFeature->m_resMetadataBuffer;
        miCpyMemMemParams.dwDstOffset = resourceOffset.dwEncodeStats + resourceOffset.dwSkipCodingUnitsCount;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(xCalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwSkipCodingUnitsCount, resFrameStatStreamOutBuffer, 7 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD));
        ENCODE_CHK_STATUS_RETURN(xCalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwSkipCodingUnitsCount, resFrameStatStreamOutBuffer, 7 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD));
        ENCODE_CHK_STATUS_RETURN(xCalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwSkipCodingUnitsCount, resFrameStatStreamOutBuffer, 7 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD));

        // NumCU_IntraDC, NumCU_IntraPlanar, NumCU_IntraAngular
        miCpyMemMemParams.presSrc     = resFrameStatStreamOutBuffer;
        miCpyMemMemParams.dwSrcOffset = 20 * sizeof(uint32_t);
        miCpyMemMemParams.dwDstOffset = resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(xCalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, resFrameStatStreamOutBuffer, 21 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD));
        ENCODE_CHK_STATUS_RETURN(xCalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwIntraCodingUnitsCount, resFrameStatStreamOutBuffer, 22 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD));

        //NumCU_Merge (LCUSkipIn8x8Unit), NumCU_MVdirL0, NumCU_MVdirL1, NumCU_MVdirBi
        miCpyMemMemParams.presSrc     = resFrameStatStreamOutBuffer;
        miCpyMemMemParams.dwSrcOffset = 27 * sizeof(uint32_t);
        miCpyMemMemParams.dwDstOffset = resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_COPY_MEM_MEM)(cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(xCalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount, resFrameStatStreamOutBuffer, 28 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD));
        ENCODE_CHK_STATUS_RETURN(xCalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount, resFrameStatStreamOutBuffer, 29 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD));
        ENCODE_CHK_STATUS_RETURN(xCalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount, resFrameStatStreamOutBuffer, 30 * sizeof(uint32_t), mhw::mi::MHW_MI_ATOMIC_ADD));
        ENCODE_CHK_STATUS_RETURN(xCalAtomic(m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwInterCodingUnitsCount, m_basicFeature->m_resMetadataBuffer, resourceOffset.dwEncodeStats + resourceOffset.dwSkipCodingUnitsCount, mhw::mi::MHW_MI_ATOMIC_SUB));

        // Average MV_X/MV_Y, report (0,0) as temp solution, later may need kernel involved
        storeDataParams.dwResourceOffset = resourceOffset.dwEncodeStats + resourceOffset.dwAverageMotionEstimationXDirection;
        storeDataParams.dwValue          = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        storeDataParams.dwResourceOffset = resourceOffset.dwEncodeStats + resourceOffset.dwAverageMotionEstimationYDirection;
        storeDataParams.dwValue          = 0;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuffer));

        return eStatus;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS HevcVdencPkt::DumpInput()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_pipeline);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        debugInterface->m_DumpInputNum         = m_basicFeature->m_frameNum - 1;

        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_ref.GetCurrRefList());
        CODEC_REF_LIST currRefList = *((CODEC_REF_LIST *)m_basicFeature->m_ref.GetCurrRefList());

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            &currRefList.sRefRawBuffer,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf"))
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::DumpResources(
        EncodeStatusMfx *       encodeStatusMfx,
        EncodeStatusReportData *statusReportData)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(encodeStatusMfx);
        ENCODE_CHK_NULL_RETURN(statusReportData);
        ENCODE_CHK_NULL_RETURN(m_pipeline);
        ENCODE_CHK_NULL_RETURN(m_statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_trackedBuf);

        CodechalDebugInterface *debugInterface = m_pipeline->GetStatusReportDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        CODEC_REF_LIST currRefList = *((CODEC_REF_LIST *)statusReportData->currRefList);
        currRefList.RefPic         = statusReportData->currOriginalPic;

        debugInterface->m_currPic            = statusReportData->currOriginalPic;
        debugInterface->m_bufferDumpFrameNum = m_basicFeature->m_frameNum - 1;
        debugInterface->m_frameType          = encodeStatusMfx->pictureCodingType;

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            &currRefList.resBitstreamBuffer,
            CodechalDbgAttr::attrBitstream,
            "_PAK",
            statusReportData->bitstreamSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpData(
            statusReportData,
            sizeof(EncodeStatusReportData),
            CodechalDbgAttr::attrStatusReport,
            "EncodeStatusReport_Buffer"));

        PMOS_RESOURCE frameStatStreamOutBuffer = m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);
        ENCODE_CHK_NULL_RETURN(frameStatStreamOutBuffer);
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
            frameStatStreamOutBuffer,
            CodechalDbgAttr::attrFrameState,
            "FrameStatus",
            frameStatStreamOutBuffer->iSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));

        MOS_SURFACE *ds4xSurface = m_basicFeature->m_trackedBuf->GetSurface(
            BufferType::ds4xSurface, currRefList.ucScalingIdx);

        if (ds4xSurface != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                ds4xSurface,
                CodechalDbgAttr::attrReconstructedSurface,
                "4xScaledSurf"))
        }

        MOS_SURFACE *ds8xSurface = m_basicFeature->m_trackedBuf->GetSurface(
            BufferType::ds8xSurface, currRefList.ucScalingIdx);

        if (ds8xSurface != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                ds8xSurface,
                CodechalDbgAttr::attrReconstructedSurface,
                "8xScaledSurf"))
        }

        MOS_RESOURCE *mbCodedBuffer = m_basicFeature->m_trackedBuf->GetBuffer(
            BufferType::mbCodedBuffer, currRefList.ucScalingIdx);
        if (mbCodedBuffer != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                mbCodedBuffer,
                CodechalDbgAttr::attrVdencOutput,
                "_MbCode",
                m_basicFeature->m_mbCodeSize,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }

        // Slice Size Conformance
        if (m_hevcSeqParams->SliceSizeControl)
        {
            uint32_t dwSize = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * CODECHAL_CACHELINE_SIZE;
            if (!m_hevcPicParams->tiles_enabled_flag || m_pipeline->GetPipeNum() <= 1)
            {
                // Slice Size StreamOut Surface
                ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                    m_basicFeature->m_recycleBuf->GetBuffer(LcuBaseAddressBuffer, 0),
                    CodechalDbgAttr::attrVdencOutput,
                    "_SliceSize",
                    dwSize,
                    0,
                    CODECHAL_NUM_MEDIA_STATES));
            }

            dwSize          = MOS_ALIGN_CEIL(4, CODECHAL_CACHELINE_SIZE);
            auto dssFeature = dynamic_cast<HevcEncodeDss *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcVdencDssFeature));
            ENCODE_CHK_NULL_RETURN(dssFeature);
            PMOS_RESOURCE resSliceCountBuffer     = nullptr;
            PMOS_RESOURCE resVDEncModeTimerBuffer = nullptr;
            ENCODE_CHK_STATUS_RETURN(dssFeature->GetDssBuffer(resSliceCountBuffer, resVDEncModeTimerBuffer));
            // Slice Count buffer 1 DW = 4 Bytes
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                resSliceCountBuffer,
                CodechalDbgAttr::attrVdencOutput,
                "_SliceCount",
                dwSize,
                0,
                CODECHAL_NUM_MEDIA_STATES));

            // VDEncMode Timer buffer 1 DW = 4 Bytes
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                resVDEncModeTimerBuffer,
                CodechalDbgAttr::attrVdencOutput,
                "_ModeTimer",
                dwSize,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }

        auto          streamInBufferSize = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;
        PMOS_RESOURCE streamInbuffer     = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, debugInterface->m_bufferDumpFrameNum);
        if (streamInbuffer)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                streamInbuffer,
                CodechalDbgAttr::attrStreamIn,
                "_ROIStreamin",
                streamInBufferSize,
                0,
                CODECHAL_NUM_MEDIA_STATES))
        }

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBltOutput(
            &currRefList.sRefReconBuffer,
            CodechalDbgAttr::attrDecodeBltOutput));
        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            &currRefList.sRefReconBuffer,
            CodechalDbgAttr::attrReconstructedSurface,
            "ReconSurf"))

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBltOutput(
            &currRefList.sRefRawBuffer,
            CodechalDbgAttr::attrDecodeBltOutput));

        return MOS_STATUS_SUCCESS;
    }

#endif

    MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HevcVdencPkt)
    {
        params.codecStandardSelect = CodecHal_GetStandardFromMode(m_basicFeature->m_mode) - CODECHAL_HCP_BASE;
        params.bStreamOutEnabled   = true;
        params.bVdencEnabled       = true;
        params.codecSelect         = 1;

        if (m_pipeline->GetPipeNum() > 1)
        {
            // Running in the multiple VDBOX mode
            if (m_pipeline->IsFirstPipe())
            {
                params.multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;
            }
            else if (m_pipeline->IsLastPipe())
            {
                params.multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;
            }
            else
            {
                params.multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;
            }
            params.pipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;
        }
        else
        {
            params.multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
            params.pipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
        }

        if (m_hevcPicParams->tiles_enabled_flag)
        {
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsTileReplayEnabled, params.bTileBasedReplayMode);
        }
        else
        {
            params.bTileBasedReplayMode = 0;
        }
        
        auto cpInterface     = m_hwInterface->GetCpInterface();
        bool twoPassScalable = params.multiEngineMode != MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY && !params.bTileBasedReplayMode;

        ENCODE_CHK_NULL_RETURN(cpInterface);
        params.setProtectionSettings = [=](uint32_t *data) { return cpInterface->SetProtectionSettingsForHcpPipeModeSelect(data, twoPassScalable); };

        auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
        ENCODE_CHK_NULL_RETURN(waTable);

        if(MEDIA_IS_WA(waTable, Wa_14012254246))
        {
            MediaUserSetting::Value outValue;
            ReadUserSetting(
                m_userSettingPtr,
                outValue,
                "DisableTlbPrefetch",
                MediaUserSetting::Group::Sequence);
            params.prefetchDisable = outValue.Get<bool>();
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_TILE_CODING, HevcVdencPkt)
    {
        ENCODE_FUNC_CALL();
        params.numberOfActiveBePipes = m_pipeline->GetPipeNum();
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddAllCmds_HCP_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        bool bLastPicInSeq    = m_basicFeature->m_lastPicInSeq;
        bool bLastPicInStream = m_basicFeature->m_lastPicInStream;
        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PAK_INSERT_OBJECT)();
        params       = {};

        if (bLastPicInSeq && bLastPicInStream)
        {
            params = {};

            uint32_t dwPadding[3];

            params.dwPadding                   = sizeof(dwPadding) / sizeof(dwPadding[0]);
            params.bHeaderLengthExcludeFrmSize = 0;
            params.bEndOfSlice                 = 1;
            params.bLastHeader                 = 1;
            params.bEmulationByteBitsInsert    = 0;
            params.uiSkipEmulationCheckCount   = 0;
            params.dataBitsInLastDw            = 16;
            params.databyteoffset              = 0;
            params.bIndirectPayloadEnable      = 0;

            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);

            dwPadding[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOS << 1) << 24));
            dwPadding[1] = (1L | (1L << 24));
            dwPadding[2] = (HEVC_NAL_UT_EOB << 1) | (1L << 8);
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, &dwPadding[0], sizeof(dwPadding)));
        }
        else if (bLastPicInSeq || bLastPicInStream)
        {
            params = {};
            uint32_t dwLastPicInSeqData[2], dwLastPicInStreamData[2];

            params.dwPadding                   = bLastPicInSeq * 2 + bLastPicInStream * 2;
            params.bHeaderLengthExcludeFrmSize = 0;
            params.bEndOfSlice                 = 1;
            params.bLastHeader                 = 1;
            params.bEmulationByteBitsInsert    = 0;
            params.uiSkipEmulationCheckCount   = 0;
            params.dataBitsInLastDw            = 8;
            params.databyteoffset              = 0;
            params.bIndirectPayloadEnable      = 0;

            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);

            if (bLastPicInSeq)
            {
                dwLastPicInSeqData[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOS << 1) << 24));
                dwLastPicInSeqData[1] = 1;  // nuh_temporal_id_plus1
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, &dwLastPicInSeqData[0], sizeof(dwLastPicInSeqData)));
            }

            if (bLastPicInStream)
            {
                dwLastPicInStreamData[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOB << 1) << 24));
                dwLastPicInStreamData[1] = 1;  // nuh_temporal_id_plus1
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, &dwLastPicInStreamData[0], sizeof(dwLastPicInStreamData)));
            }
        }
        else
        {
            PCODECHAL_NAL_UNIT_PARAMS *ppNalUnitParams = (CODECHAL_NAL_UNIT_PARAMS **)m_nalUnitParams;

            auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
            ENCODE_CHK_NULL_RETURN(brcFeature);

            PMHW_BATCH_BUFFER batchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);
            PBSBuffer         pBsBuffer   = &(m_basicFeature->m_bsBuffer);
            uint32_t          bitSize     = 0;
            uint32_t          offSet      = 0;

            //insert AU, SPS, PSP headers before first slice header
            if (m_basicFeature->m_curNumSlices == 0)
            {
                uint32_t maxBytesInPakInsertObjCmd = ((2 << 11) - 1) * 4;  // 12 bits for Length field in PAK_INSERT_OBJ cmd

                for (auto i = 0; i < HEVC_MAX_NAL_UNIT_TYPE; i++)
                {
                    uint32_t nalunitPosiSize   = ppNalUnitParams[i]->uiSize;
                    uint32_t nalunitPosiOffset = ppNalUnitParams[i]->uiOffset;

                    while (nalunitPosiSize > 0)
                    {
                        bitSize = MOS_MIN(maxBytesInPakInsertObjCmd * 8, nalunitPosiSize * 8);
                        offSet  = nalunitPosiOffset;

                        params = {};

                        params.dwPadding                 = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                        params.bEmulationByteBitsInsert  = ppNalUnitParams[i]->bInsertEmulationBytes;
                        params.uiSkipEmulationCheckCount = ppNalUnitParams[i]->uiSkipEmulationCheckCount;
                        params.dataBitsInLastDw          = bitSize % 32;
                        if (params.dataBitsInLastDw == 0)
                        {
                            params.dataBitsInLastDw = 32;
                        }

                        if (nalunitPosiSize > maxBytesInPakInsertObjCmd)
                        {
                            nalunitPosiSize -= maxBytesInPakInsertObjCmd;
                            nalunitPosiOffset += maxBytesInPakInsertObjCmd;
                        }
                        else
                        {
                            nalunitPosiSize = 0;
                        }
                        m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
                        uint32_t byteSize = (bitSize + 7) >> 3;
                        if (byteSize)
                        {
                            MHW_MI_CHK_NULL(pBsBuffer);
                            MHW_MI_CHK_NULL(pBsBuffer->pBase);
                            uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, data, byteSize));
                        }
                    }
                }
            }

            params = {};
            // Insert slice header
            params.bLastHeader              = true;
            params.bEmulationByteBitsInsert = true;

            // App does the slice header packing, set the skip count passed by the app
            PCODEC_ENCODER_SLCDATA slcData    = m_basicFeature->m_slcData;
            uint32_t               currSlcIdx = m_basicFeature->m_curNumSlices;

            params.uiSkipEmulationCheckCount = slcData[currSlcIdx].SkipEmulationByteCount;
            bitSize                          = slcData[currSlcIdx].BitSize;
            offSet                           = slcData[currSlcIdx].SliceOffset;

            if (m_hevcSeqParams->SliceSizeControl)
            {
                params.bLastHeader                = false;
                params.bEmulationByteBitsInsert   = false;
                bitSize                           = m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
                params.bResetBitstreamStartingPos = true;
                params.dwPadding                  = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                params.dataBitsInLastDw           = bitSize % 32;
                if (params.dataBitsInLastDw == 0)
                {
                    params.dataBitsInLastDw = 32;
                }

                m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
                uint32_t byteSize = (bitSize + 7) >> 3;
                if (byteSize)
                {
                    MHW_MI_CHK_NULL(pBsBuffer);
                    MHW_MI_CHK_NULL(pBsBuffer->pBase);
                    uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, data, byteSize));
                }

                // Send HCP_PAK_INSERT_OBJ command. For dynamic slice, we are skipping the beginning part of slice header.
                params.bLastHeader = true;
                bitSize            = bitSize - m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
                offSet += ((m_hevcSliceParams->BitLengthSliceHeaderStartingPortion + 7) / 8);  // Skips the first 5 bytes which is Start Code + Nal Unit Header
                params.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                params.dataBitsInLastDw = bitSize % 32;
                if (params.dataBitsInLastDw == 0)
                {
                    params.dataBitsInLastDw = 32;
                }
                params.bResetBitstreamStartingPos = true;
                m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
                byteSize = (bitSize + 7) >> 3;
                if (byteSize)
                {
                    MHW_MI_CHK_NULL(pBsBuffer);
                    MHW_MI_CHK_NULL(pBsBuffer->pBase);
                    uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, data, byteSize));
                }
            }
            else
            {
                params.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
                params.dataBitsInLastDw = bitSize % 32;
                if (params.dataBitsInLastDw == 0)
                {
                    params.dataBitsInLastDw = 32;
                }
                m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
                uint32_t byteSize = (bitSize + 7) >> 3;
                if (byteSize)
                {
                    MHW_MI_CHK_NULL(pBsBuffer);
                    MHW_MI_CHK_NULL(pBsBuffer->pBase);
                    uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, data, byteSize));
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, HevcVdencPkt)
    {
        ENCODE_FUNC_CALL();

        params.Mode                 = m_basicFeature->m_mode;
        params.psPreDeblockSurface  = &m_basicFeature->m_reconSurface;
        params.psPostDeblockSurface = &m_basicFeature->m_reconSurface;
        params.psRawSurface         = m_basicFeature->m_rawSurfaceToPak;

        params.presMetadataLineBuffer       = m_resMetadataLineBuffer;
        params.presMetadataTileLineBuffer   = m_resMetadataTileLineBuffer;
        params.presMetadataTileColumnBuffer = m_resMetadataTileColumnBuffer;

        params.presCurMvTempBuffer           = m_basicFeature->m_resMvTemporalBuffer;
        params.dwLcuStreamOutOffset          = 0;
        params.presLcuILDBStreamOutBuffer    = m_resLCUIldbStreamOutBuffer;
        params.dwFrameStatStreamOutOffset    = 0;
        params.presSseSrcPixelRowStoreBuffer = m_resSSESrcPixelRowStoreBuffer;
        params.presPakCuLevelStreamoutBuffer = m_resPakcuLevelStreamOutData;
        //    Mos_ResourceIsNull(&m_resPakcuLevelStreamoutData.sResource) ? nullptr : &m_resPakcuLevelStreamoutData.sResource;

        params.bRawIs10Bit = m_basicFeature->m_is10Bit;

#ifdef _MMC_SUPPORTED
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        if (m_mmcState->IsMmcEnabled())
        {
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_reconSurface, &params.PreDeblockSurfMmcState));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_rawSurface, &params.RawSurfMmcState));
        }
        else
        {
            params.PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
            params.RawSurfMmcState        = MOS_MEMCOMP_DISABLED;
        }

        CODECHAL_DEBUG_TOOL(
            m_basicFeature->m_reconSurface.MmcState = params.PreDeblockSurfMmcState;)
#endif

        m_basicFeature->m_ref.MHW_SETPAR_F(HCP_PIPE_BUF_ADDR_STATE)(params);

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_IND_OBJ_BASE_ADDR_STATE, HevcVdencPkt)
    {
        ENCODE_FUNC_CALL();

        params.presMvObjectBuffer      = m_basicFeature->m_resMbCodeBuffer;
        params.dwMvObjectOffset        = m_mvOffset;
        params.dwMvObjectSize          = m_basicFeature->m_mbCodeSize - m_mvOffset;
        params.presPakBaseObjectBuffer = &m_basicFeature->m_resBitstreamBuffer;
        params.dwPakBaseObjectSize     = m_basicFeature->m_bitstreamSize;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_SLICE_STATE, HevcVdencPkt)
    {
        ENCODE_FUNC_CALL();

        params.intrareffetchdisable = m_pakOnlyPass;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddAllCmds_HCP_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        m_curHcpSurfStateId = CODECHAL_HCP_SRC_SURFACE_ID;
        SETPAR_AND_ADDCMD(HCP_SURFACE_STATE, m_hcpItf, cmdBuffer);

        m_curHcpSurfStateId = CODECHAL_HCP_DECODED_SURFACE_ID;
        SETPAR_AND_ADDCMD(HCP_SURFACE_STATE, m_hcpItf, cmdBuffer);

        m_curHcpSurfStateId = CODECHAL_HCP_REF_SURFACE_ID;
        SETPAR_AND_ADDCMD(HCP_SURFACE_STATE, m_hcpItf, cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddAllCmds_HCP_REF_IDX_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_REF_IDX_STATE)();
        params       = {};

        uint32_t                          currSlcIdx    = m_basicFeature->m_curNumSlices;
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = (CODEC_HEVC_ENCODE_PICTURE_PARAMS *)m_hevcPicParams;
        PCODEC_HEVC_ENCODE_SLICE_PARAMS   hevcSlcParams = (CODEC_HEVC_ENCODE_SLICE_PARAMS *)&m_hevcSliceParams[currSlcIdx];

        CODEC_PICTURE currPic                                     = {};
        CODEC_PICTURE refPicList[2][CODEC_MAX_NUM_REF_FRAME_HEVC] = {};
        void **       hevcRefList                                 = nullptr;
        int32_t       pocCurrPic                                  = 0;
        int8_t *      pRefIdxMapping                              = nullptr;
        int32_t       pocList[CODEC_MAX_NUM_REF_FRAME_HEVC]       = {};

        if (hevcSlcParams->slice_type != encodeHevcISlice)
        {
            currPic                                    = hevcPicParams->CurrReconstructedPic;
            params.ucList                              = LIST_0;
            params.numRefIdxLRefpiclistnumActiveMinus1 = hevcSlcParams->num_ref_idx_l0_active_minus1;
            eStatus                                    = MOS_SecureMemcpy(&refPicList, sizeof(refPicList), &hevcSlcParams->RefPicList, sizeof(hevcSlcParams->RefPicList));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                ENCODE_ASSERTMESSAGE("Failed to copy memory.");
                return eStatus;
            }

            hevcRefList = (void **)m_basicFeature->m_ref.GetRefList();
            pocCurrPic  = hevcPicParams->CurrPicOrderCnt;
            for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                pocList[i] = hevcPicParams->RefFramePOCList[i];
            }

            pRefIdxMapping = m_basicFeature->m_ref.GetRefIdxMapping();

            MHW_ASSERT(currPic.FrameIdx != 0x7F);

            for (uint8_t i = 0; i <= params.numRefIdxLRefpiclistnumActiveMinus1; i++)
            {
                uint8_t refFrameIDx = refPicList[params.ucList][i].FrameIdx;
                if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
                {
                    MHW_ASSERT(*(pRefIdxMapping + refFrameIDx) >= 0);

                    params.listEntryLxReferencePictureFrameIdRefaddr07[i] = *(pRefIdxMapping + refFrameIDx);
                    int32_t pocDiff                                       = pocCurrPic - pocList[refFrameIDx];
                    params.referencePictureTbValue[i]                     = (uint8_t)CodecHal_Clip3(-128, 127, pocDiff);
                    CODEC_REF_LIST **refList                              = (CODEC_REF_LIST **)hevcRefList;
                    params.longtermreference[i]                           = CodecHal_PictureIsLongTermRef(refList[currPic.FrameIdx]->RefList[refFrameIDx]);
                    params.bottomFieldFlag[i]                             = 1;
                }
                else
                {
                    params.listEntryLxReferencePictureFrameIdRefaddr07[i] = 0;
                    params.referencePictureTbValue[i]                     = 0;
                    params.longtermreference[i]                           = false;
                    params.bottomFieldFlag[i]                             = 0;
                }
            }

            for (uint8_t i = (uint8_t)(params.numRefIdxLRefpiclistnumActiveMinus1 + 1); i < 16; i++)
            {
                params.listEntryLxReferencePictureFrameIdRefaddr07[i] = 0;
                params.referencePictureTbValue[i]                     = 0;
                params.longtermreference[i]                           = false;
                params.bottomFieldFlag[i]                             = 0;
            }

            ENCODE_CHK_NULL_RETURN(m_featureManager);
            auto sccFeature = dynamic_cast<HevcVdencScc *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcVdencSccFeature));
            ENCODE_CHK_NULL_RETURN(sccFeature);

            MHW_CHK_STATUS_RETURN(sccFeature->MHW_SETPAR_F(HCP_REF_IDX_STATE)(params));

            m_hcpItf->MHW_ADDCMD_F(HCP_REF_IDX_STATE)(cmdBuffer);

            params = {};

            if (hevcSlcParams->slice_type == encodeHevcBSlice)
            {
                params.ucList                              = LIST_1;
                params.numRefIdxLRefpiclistnumActiveMinus1 = hevcSlcParams->num_ref_idx_l1_active_minus1;
                for (uint8_t i = 0; i <= params.numRefIdxLRefpiclistnumActiveMinus1; i++)
                {
                    uint8_t refFrameIDx = refPicList[params.ucList][i].FrameIdx;
                    if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
                    {
                        MHW_ASSERT(*(pRefIdxMapping + refFrameIDx) >= 0);

                        params.listEntryLxReferencePictureFrameIdRefaddr07[i] = *(pRefIdxMapping + refFrameIDx);
                        int32_t pocDiff                                       = pocCurrPic - pocList[refFrameIDx];
                        params.referencePictureTbValue[i]                     = (uint8_t)CodecHal_Clip3(-128, 127, pocDiff);
                        CODEC_REF_LIST **refList                              = (CODEC_REF_LIST **)hevcRefList;
                        params.longtermreference[i]                           = CodecHal_PictureIsLongTermRef(refList[currPic.FrameIdx]->RefList[refFrameIDx]);
                        params.bottomFieldFlag[i]                             = 1;
                    }
                    else
                    {
                        params.listEntryLxReferencePictureFrameIdRefaddr07[i] = 0;
                        params.referencePictureTbValue[i]                     = 0;
                        params.longtermreference[i]                           = false;
                        params.bottomFieldFlag[i]                             = 0;
                    }
                }

                for (uint8_t i = (uint8_t)(params.numRefIdxLRefpiclistnumActiveMinus1 + 1); i < 16; i++)
                {
                    params.listEntryLxReferencePictureFrameIdRefaddr07[i] = 0;
                    params.referencePictureTbValue[i]                     = 0;
                    params.longtermreference[i]                           = false;
                    params.bottomFieldFlag[i]                             = 0;
                }
                m_hcpItf->MHW_ADDCMD_F(HCP_REF_IDX_STATE)(cmdBuffer);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddAllCmds_HCP_FQM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        MHW_MI_CHK_NULL(m_hevcIqMatrixParams);

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_FQM_STATE)();
        params       = {};

        auto      iqMatrix = (PMHW_VDBOX_HEVC_QM_PARAMS)m_hevcIqMatrixParams;
        uint16_t *fqMatrix = (uint16_t *)params.quantizermatrix;

        /* 4x4 */
        for (uint8_t i = 0; i < 32; i++)
        {
            params.quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            params.intraInter     = intraInter;
            params.sizeid         = 0;
            params.colorComponent = 0;

            for (uint8_t i = 0; i < 16; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List4x4[3 * intraInter][i]);
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_FQM_STATE)(cmdBuffer);
        }

        /* 8x8, 16x16 and 32x32 */
        for (uint8_t i = 0; i < 32; i++)
        {
            params.quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            params.intraInter     = intraInter;
            params.sizeid         = 1;
            params.colorComponent = 0;

            for (uint8_t i = 0; i < 64; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List8x8[3 * intraInter][i]);
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_FQM_STATE)(cmdBuffer);
        }

        /* 16x16 DC */
        for (uint8_t i = 0; i < 32; i++)
        {
            params.quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            params.intraInter     = intraInter;
            params.sizeid         = 2;
            params.colorComponent = 0;
            params.fqmDcValue1Dc  = GetReciprocalScalingValue(iqMatrix->ListDC16x16[3 * intraInter]);

            for (uint8_t i = 0; i < 64; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List16x16[3 * intraInter][i]);
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_FQM_STATE)(cmdBuffer);
        }

        /* 32x32 DC */
        for (uint8_t i = 0; i < 32; i++)
        {
            params.quantizermatrix[i] = 0;
        }
        for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
        {
            params.intraInter     = intraInter;
            params.sizeid         = 3;
            params.colorComponent = 0;
            params.fqmDcValue1Dc  = GetReciprocalScalingValue(iqMatrix->ListDC32x32[intraInter]);

            for (uint8_t i = 0; i < 64; i++)
            {
                fqMatrix[i] =
                    GetReciprocalScalingValue(iqMatrix->List32x32[intraInter][i]);
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_FQM_STATE)(cmdBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddAllCmds_HCP_QM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        MHW_MI_CHK_NULL(m_hevcIqMatrixParams);

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_QM_STATE)();
        params       = {};

        auto     iqMatrix = (PMHW_VDBOX_HEVC_QM_PARAMS)m_hevcIqMatrixParams;
        uint8_t *qMatrix  = (uint8_t *)params.quantizermatrix;

        for (uint8_t sizeId = 0; sizeId < 4; sizeId++)  // 4x4, 8x8, 16x16, 32x32
        {
            for (uint8_t predType = 0; predType < 2; predType++)  // Intra, Inter
            {
                for (uint8_t color = 0; color < 3; color++)  // Y, Cb, Cr
                {
                    if ((sizeId == 3) && (color != 0))
                        break;

                    params.sizeid         = sizeId;
                    params.predictionType = predType;
                    params.colorComponent = color;
                    switch (sizeId)
                    {
                    case 0:
                    case 1:
                    default:
                        params.dcCoefficient = 0;
                        break;
                    case 2:
                        params.dcCoefficient = iqMatrix->ListDC16x16[3 * predType + color];
                        break;
                    case 3:
                        params.dcCoefficient = iqMatrix->ListDC32x32[predType];
                        break;
                    }

                    if (sizeId == 0)
                    {
                        for (uint8_t i = 0; i < 4; i++)
                        {
                            for (uint8_t ii = 0; ii < 4; ii++)
                            {
                                qMatrix[4 * i + ii] = iqMatrix->List4x4[3 * predType + color][4 * i + ii];
                            }
                        }
                    }
                    else if (sizeId == 1)
                    {
                        for (uint8_t i = 0; i < 8; i++)
                        {
                            for (uint8_t ii = 0; ii < 8; ii++)
                            {
                                qMatrix[8 * i + ii] = iqMatrix->List8x8[3 * predType + color][8 * i + ii];
                            }
                        }
                    }
                    else if (sizeId == 2)
                    {
                        for (uint8_t i = 0; i < 8; i++)
                        {
                            for (uint8_t ii = 0; ii < 8; ii++)
                            {
                                qMatrix[8 * i + ii] = iqMatrix->List16x16[3 * predType + color][8 * i + ii];
                            }
                        }
                    }
                    else  // 32x32
                    {
                        for (uint8_t i = 0; i < 8; i++)
                        {
                            for (uint8_t ii = 0; ii < 8; ii++)
                            {
                                qMatrix[8 * i + ii] = iqMatrix->List32x32[predType][8 * i + ii];
                            }
                        }
                    }

                    m_hcpItf->MHW_ADDCMD_F(HCP_QM_STATE)(cmdBuffer);
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencPkt::AddAllCmds_HCP_WEIGHTOFFSET_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        auto wpFeature = dynamic_cast<HevcVdencWeightedPred *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcVdencWpFeature));
        ENCODE_CHK_NULL_RETURN(wpFeature);
        if (wpFeature->IsEnabled())
        {
            auto &params                                           = m_hcpItf->MHW_GETPAR_F(HCP_WEIGHTOFFSET_STATE)();
            params                                                 = {};
            CODEC_HEVC_ENCODE_SLICE_PARAMS *pEncodeHevcSliceParams = (CODEC_HEVC_ENCODE_SLICE_PARAMS *)&m_hevcSliceParams[m_basicFeature->m_curNumSlices];
            if (pEncodeHevcSliceParams->slice_type == encodeHevcPSlice ||
                pEncodeHevcSliceParams->slice_type == encodeHevcBSlice)
            {
                params.ucList = LIST_0;
                MHW_CHK_STATUS_RETURN(wpFeature->MHW_SETPAR_F(HCP_WEIGHTOFFSET_STATE)(params));
                m_hcpItf->MHW_ADDCMD_F(HCP_WEIGHTOFFSET_STATE)(cmdBuffer);
            }

            if (pEncodeHevcSliceParams->slice_type == encodeHevcBSlice)
            {
                params.ucList = LIST_1;
                MHW_CHK_STATUS_RETURN(wpFeature->MHW_SETPAR_F(HCP_WEIGHTOFFSET_STATE)(params));
                m_hcpItf->MHW_ADDCMD_F(HCP_WEIGHTOFFSET_STATE)(cmdBuffer);
            }
        }
        return MOS_STATUS_SUCCESS;
    }
    }

