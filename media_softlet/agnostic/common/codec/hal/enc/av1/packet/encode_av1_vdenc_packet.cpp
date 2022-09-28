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
//! \file     encode_av1_vdenc_packet.cpp
//! \brief    Defines the interface for av1 encode vdenc packet
//!
#include <numeric>
#include "encode_av1_vdenc_packet.h"
#include "encode_status_report_defs.h"
#include "codec_def_common_av1.h"
#include "media_perf_profiler.h"

namespace encode{
    Av1VdencPkt::Av1VdencPkt(MediaPipeline* pipeline, MediaTask* task, CodechalHwInterface* hwInterface) :
        CmdPacket(task),
        m_pipeline(dynamic_cast<Av1VdencPipeline*>(pipeline)),
        m_hwInterface(hwInterface)
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_pipeline);

        m_osInterface = hwInterface->GetOsInterface();
        m_statusReport = m_pipeline->GetStatusReportInstance();
        m_featureManager = m_pipeline->GetPacketLevelFeatureManager(Av1Pipeline::Av1VdencPacket);

        m_vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
        m_avpItf   = std::static_pointer_cast<mhw::vdbox::avp::Itf>(m_hwInterface->GetAvpInterfaceNext());
        m_miItf    = std::static_pointer_cast<mhw::mi::Itf> (m_hwInterface->GetMiInterfaceNext());
        
        if (m_vdencItf)
        {
            mhw::vdbox::vdenc::RowStorePar par = {};

            par.mode = mhw::vdbox::vdenc::RowStorePar::AV1;

            m_vdencItf->SetRowstoreCachingOffsets(par);
        }
        if(m_osInterface)
        {
            m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
        }
        if (!m_userSettingPtr)
        {
            ENCODE_NORMALMESSAGE("Initialize m_userSettingPtr instance failed!");
        }
    }

    MOS_STATUS Av1VdencPkt::StartStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(MediaPacket::StartStatusReportNext(srType, cmdBuffer));

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        ENCODE_CHK_STATUS_RETURN(ReadAvpStatus(m_vdboxIndex, m_statusReport, *cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(MediaPacket::EndStatusReportNext(srType, cmdBuffer));

        MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::ReadAvpStatus(MHW_VDBOX_NODE_IND vdboxIndex, MediaStatusReport *statusReport, MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        CODEC_HW_FUNCTION_ENTER;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::Init()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_statusReport);
        ENCODE_CHK_STATUS_RETURN(CmdPacket::Init());

        m_basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

#ifdef _MMC_SUPPORTED
        m_mmcState = m_pipeline->GetMmcState();
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        m_basicFeature->m_mmcState = m_mmcState;
#endif
        m_allocator = m_pipeline->GetEncodeAllocator();
        ENCODE_CHK_STATUS_RETURN(AllocateResources());

        ENCODE_CHK_STATUS_RETURN(m_statusReport->RegistObserver(this));

        CalculateVdencCommandsSize();
        CalculateAvpCommandsSize();

        m_usePatchList = m_osInterface->bUsesPatchList;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::AddCondBBEndFor2ndPass(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        if (m_pipeline->IsFirstPass() || m_pipeline->GetPassNum() == 1)
        {
            return MOS_STATUS_SUCCESS;
        }

        auto &miConditionalBatchBufferEndParams = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        miConditionalBatchBufferEndParams       = {};

        // VDENC uses HuC FW generated semaphore for conditional 2nd pass
        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    void Av1VdencPkt::SetPerfTag()
    {
        ENCODE_FUNC_CALL();

        uint16_t callType = m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE :
            CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE_SECOND_PASS;
        uint16_t picType  = (m_basicFeature->m_pictureCodingType == I_TYPE) ? 1 :
            (m_basicFeature->m_ref.IsLowDelay() ? (m_basicFeature->m_ref.IsPFrame() ? 2 : 0) : 3);

        PerfTagSetting perfTag;
        perfTag.Value             = 0;
        perfTag.Mode              = (uint16_t)m_basicFeature->m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
        perfTag.CallType          = callType;
        perfTag.PictureCodingType = picType > 3 ? 0 : picType;
        m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
        m_osInterface->pfnIncPerfBufferID(m_osInterface);
    }

    MOS_STATUS Av1VdencPkt::AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer)
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

    MOS_STATUS Av1VdencPkt::SendPrologCmds(
        MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();

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

        return eStatus;
    }

    void Av1VdencPkt::UpdateParameters()
    {
        ENCODE_FUNC_CALL();

        m_prevFrameType  = m_av1PicParams->PicFlags.fields.frame_type;
        m_basicFeature->m_encodedFrameNum++;
    }

    MOS_STATUS Av1VdencPkt::SetPipeBufAddr(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
        MHW_VDBOX_SURFACE_PARAMS &      srcSurfaceParams,
        MHW_VDBOX_SURFACE_PARAMS &      reconSurfaceParams)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_mmcState);
        ENCODE_CHK_NULL_RETURN(srcSurfaceParams.psSurface);
        ENCODE_CHK_NULL_RETURN(reconSurfaceParams.psSurface);

        if (m_mmcState->IsMmcEnabled())
        {
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_reconSurface, &pipeBufAddrParams->PreDeblockSurfMmcState));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(&m_basicFeature->m_rawSurface, &pipeBufAddrParams->RawSurfMmcState));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(srcSurfaceParams.psSurface, &srcSurfaceParams.dwCompressionFormat));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(reconSurfaceParams.psSurface, &reconSurfaceParams.dwCompressionFormat));
        }
        else
        {
            pipeBufAddrParams->PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
            pipeBufAddrParams->RawSurfMmcState        = MOS_MEMCOMP_DISABLED;
        }

        CODECHAL_DEBUG_TOOL(
            m_basicFeature->m_reconSurface.MmcState = pipeBufAddrParams->PreDeblockSurfMmcState;)

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::SetSurfaceState(
        PMHW_VDBOX_SURFACE_PARAMS surfaceStateParams)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(surfaceStateParams);
        ENCODE_CHK_NULL_RETURN(surfaceStateParams->psSurface);
        ENCODE_CHK_NULL_RETURN(m_mmcState);

        ENCODE_FUNC_CALL();

        if (m_mmcState->IsMmcEnabled())
        {
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(surfaceStateParams->psSurface, &surfaceStateParams->mmcState));
            ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(surfaceStateParams->psSurface, &surfaceStateParams->dwCompressionFormat));
        }
        else
        {
            surfaceStateParams->mmcState = MOS_MEMCOMP_DISABLED;
        }
        return eStatus;
    }

    MOS_STATUS Av1VdencPkt::Prepare()
    {
        ENCODE_FUNC_CALL();

        Av1Pipeline *pipeline = dynamic_cast<Av1Pipeline *>(m_pipeline);
        ENCODE_CHK_NULL_RETURN(pipeline);

        m_av1SeqParams  = ((Av1BasicFeature *)m_basicFeature)->m_av1SeqParams;
        m_av1PicParams  = ((Av1BasicFeature *)m_basicFeature)->m_av1PicParams;
        m_nalUnitParams = ((Av1BasicFeature *)m_basicFeature)->m_nalUnitParams;

        SetRowstoreCachingOffsets();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::SetRowstoreCachingOffsets()
    {
        // Get row store cache offset as all the needed information is got here
        if (m_avpItf->IsRowStoreCachingSupported())
        {
            MHW_VDBOX_ROWSTORE_PARAMS rowStoreParams;

            rowStoreParams.Mode             = codechalEncodeModeAv1;
            rowStoreParams.dwPicWidth       = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, av1MinBlockWidth);
            rowStoreParams.ucChromaFormat   = m_basicFeature->m_outputChromaFormat;
            rowStoreParams.ucBitDepthMinus8 = m_basicFeature->m_is10Bit ? 2 : 0;

            ENCODE_CHK_STATUS_RETURN(m_hwInterface->m_hwInterfaceNext->SetRowstoreCachingOffsets(&rowStoreParams));
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::Completed(void *mfxStatus, void *rcsStatus, void *statusReport)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(mfxStatus);
        ENCODE_CHK_NULL_RETURN(statusReport);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        EncodeStatusMfx        *encodeStatusMfx  = (EncodeStatusMfx *)mfxStatus;
        EncodeStatusReportData *statusReportData = (EncodeStatusReportData *)statusReport;

        uint32_t statBufIdx     = statusReportData->currOriginalPic.FrameIdx;
        const EncodeReportTileData *tileReportData = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetReportTileData, statBufIdx, tileReportData);
        ENCODE_CHK_NULL_RETURN(tileReportData);

        MOS_RESOURCE *tileRecordBuffer = nullptr;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileRecordBuffer, statBufIdx, tileRecordBuffer);
        ENCODE_CHK_NULL_RETURN(tileRecordBuffer);

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        PakHwTileSizeRecord *tileRecord =
            (PakHwTileSizeRecord *)m_allocator->Lock(tileRecordBuffer, &lockFlags);
        ENCODE_CHK_NULL_RETURN(tileRecord);

        statusReportData->bitstreamSize = 0;
        for (uint32_t i = 0; i < statusReportData->numberTilesInFrame; i++)
        {
            if (tileRecord[i].Length == 0)
            {
                statusReportData->codecStatus = CODECHAL_STATUS_INCOMPLETE;
                return MOS_STATUS_SUCCESS;
            }

            statusReportData->bitstreamSize += tileRecord[i].Length;
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_basicFeature->m_enableSWStitching)
        {
            PerformSwStitch(tileReportData, tileRecord, statusReportData);
        }
#endif
        if (tileRecord)
        {
            m_allocator->UnLock(tileRecordBuffer);
        }

        statusReportData->numberPasses = (uint8_t)encodeStatusMfx->imageStatusCtrl.avpTotalNumPass + 1;  //initial pass is considered to be 0,hence +1 to report;
        ENCODE_VERBOSEMESSAGE("statusReportData->numberPasses: %d\n", statusReportData->numberPasses);

        uint32_t log2MaxSbSize   = av1MiSizeLog2 + av1MinMibSizeLog2;
        uint32_t frameWidthInSb  = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, (1 << log2MaxSbSize)) >> log2MaxSbSize;
        uint32_t frameHeightInSb = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, (1 << log2MaxSbSize)) >> log2MaxSbSize;
        if (frameWidthInSb != 0 && frameHeightInSb != 0)
        {
            ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1SeqParams);
            statusReportData->qpY = (uint8_t)(((uint32_t)encodeStatusMfx->qpStatusCount.avpCumulativeQP) / (frameWidthInSb * frameHeightInSb));
        }

        CODECHAL_DEBUG_TOOL(
            ENCODE_CHK_STATUS_RETURN(DumpResources(encodeStatusMfx, statusReportData)););

        m_basicFeature->Reset((CODEC_REF_LIST *)statusReportData->currRefList);

        return MOS_STATUS_SUCCESS;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS Av1VdencPkt::PerformSwStitch(
        const EncodeReportTileData *tileReportData,
        PakHwTileSizeRecord *       tileRecord,
        EncodeStatusReportData *    statusReportData)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(tileReportData);
        ENCODE_CHK_NULL_RETURN(tileRecord);

        uint8_t *tempBsBuffer = nullptr, *bufPtr = nullptr;
        tempBsBuffer = bufPtr = (uint8_t *)MOS_AllocAndZeroMemory(statusReportData->bitstreamSize);
        ENCODE_CHK_NULL_RETURN(tempBsBuffer);

        PCODEC_REF_LIST currRefList = (PCODEC_REF_LIST)statusReportData->currRefList;

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly = 1;
        uint8_t *bitstream = (uint8_t *)m_allocator->Lock(
            &currRefList->resBitstreamBuffer,
            &lockFlags);
        if (bitstream == nullptr)
        {
            MOS_FreeMemory(tempBsBuffer);
            ENCODE_CHK_NULL_RETURN(nullptr);
        }

        for (uint32_t i = 0; i < statusReportData->numberTilesInFrame; i++)
        {
            uint32_t offset = tileReportData[i].bitstreamByteOffset * CODECHAL_CACHELINE_SIZE;
            uint32_t len    = tileRecord[i].Length;

            MOS_SecureMemcpy(bufPtr, len, &bitstream[offset], len);
            bufPtr += len;
        }

        MOS_SecureMemcpy(bitstream, statusReportData->bitstreamSize, tempBsBuffer, statusReportData->bitstreamSize);
        MOS_ZeroMemory(&bitstream[statusReportData->bitstreamSize], m_basicFeature->m_bitstreamSize - statusReportData->bitstreamSize);

        if (bitstream)
        {
            m_allocator->UnLock(&currRefList->resBitstreamBuffer);
        }

        MOS_FreeMemory(tempBsBuffer);

        return MOS_STATUS_SUCCESS;
    }
#endif

    MOS_STATUS Av1VdencPkt::Destroy()
    {
        ENCODE_FUNC_CALL();

        m_statusReport->UnregistObserver(this);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::AllocateResources()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_allocator);
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type               = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType           = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format             = Format_Buffer;
        allocParamsForBufferLinear.Flags.bNotLockable = true;

        uint32_t maxTileNumber              = CODECHAL_GET_WIDTH_IN_BLOCKS(m_basicFeature->m_frameWidth, av1MinTileWidth) *
                                              CODECHAL_GET_HEIGHT_IN_BLOCKS(m_basicFeature->m_frameHeight, av1MinTileHeight);

        allocParamsForBufferLinear.dwBytes  = MOS_ROUNDUP_DIVIDE(m_basicFeature->m_frameWidth, av1SuperBlockWidth) * MHW_CACHELINE_SIZE * 2 * 2;
        allocParamsForBufferLinear.pBufName = "vdencIntraRowStoreScratch";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        m_vdencIntraRowStoreScratch         = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        ENCODE_CHK_NULL_RETURN(m_vdencIntraRowStoreScratch);

        allocParamsForBufferLinear.Flags.bNotLockable = !(m_basicFeature->m_lockableResource);
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_basicFeature->m_vdencBrcStatsBufferSize * maxTileNumber, MHW_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDEncStatsBuffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        m_resVDEncStatsBuffer               = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        ENCODE_CHK_NULL_RETURN(m_resVDEncStatsBuffer);

        if (m_osInterface->bInlineCodecStatusUpdate)
        {
            MOS_LOCK_PARAMS lockFlagsWriteOnly;
            MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
            lockFlagsWriteOnly.WriteOnly = 1;

            m_atomicScratchBuf.size = MOS_ALIGN_CEIL(sizeof(AtomicScratchBuffer), sizeof(uint64_t));
            allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format   = Format_Buffer;

            uint32_t size        = MHW_CACHELINE_SIZE * 4 * 2;  //  each set of scratch is 4 cacheline size, and allocate 2 set.
            allocParamsForBufferLinear.dwBytes      = size;
            allocParamsForBufferLinear.pBufName     = "atomic sratch buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_CODEC_RESOURCE_USAGE_BEGIN_CODEC;

            m_atomicScratchBuf.resAtomicScratchBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);

            ENCODE_CHK_NULL_RETURN(m_atomicScratchBuf.resAtomicScratchBuffer);

            uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                m_atomicScratchBuf.resAtomicScratchBuffer,
                &lockFlagsWriteOnly);

            ENCODE_CHK_NULL_RETURN(data);

            MOS_ZeroMemory(data, size);
            m_atomicScratchBuf.pData             = (uint32_t *)data;
            m_atomicScratchBuf.size              = size;
            m_atomicScratchBuf.zeroValueOffset   = 0;
            m_atomicScratchBuf.operand1Offset    = MHW_CACHELINE_SIZE;
            m_atomicScratchBuf.operand2Offset    = MHW_CACHELINE_SIZE * 2;
            m_atomicScratchBuf.operand3Offset    = MHW_CACHELINE_SIZE * 3;
            m_atomicScratchBuf.encodeUpdateIndex = 0;
            m_atomicScratchBuf.tearDownIndex     = 1;
            m_atomicScratchBuf.operandSetSize    = MHW_CACHELINE_SIZE * 4;

            ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, m_atomicScratchBuf.resAtomicScratchBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::ReadPakMmioRegisters(PMOS_COMMAND_BUFFER cmdBuf, bool firstTile)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuf);

        auto mmioRegs = m_miItf->GetMmioRegisters();
        auto mmioRegsAvp = m_avpItf->GetMmioRegisters(MHW_VDBOX_NODE_1);
        ENCODE_CHK_NULL_RETURN(mmioRegs);
        PMOS_RESOURCE bsSizeBuf = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
        ENCODE_CHK_NULL_RETURN(bsSizeBuf);

        if (firstTile)
        {
            // clear bitstream size buffer at first tile
            auto &miStoreDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            miStoreDataParams                  = {};
            miStoreDataParams.pOsResource      = bsSizeBuf;
            miStoreDataParams.dwResourceOffset = 0;
            miStoreDataParams.dwValue          = 0;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuf));
        }

        // load current tile size to VCS_GPR0_Lo
        auto &miLoadRegaParams         = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_REG)();
        miLoadRegaParams               = {};
        miLoadRegaParams.dwSrcRegister = mmioRegsAvp->avpAv1BitstreamByteCountTileRegOffset;
        miLoadRegaParams.dwDstRegister = mmioRegs->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_REG)(cmdBuf));
      
        // load bitstream size buffer to VCS_GPR4_Lo
        auto &miLoadRegMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = bsSizeBuf;
        miLoadRegMemParams.dwOffset        = 0;
        miLoadRegMemParams.dwRegister      = mmioRegs->generalPurposeRegister4LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuf));

        mhw::mi::MHW_MI_ALU_PARAMS aluParams[4] = {};
        int32_t aluCount               = 0;

        //load1 srca, reg1
        aluParams[aluCount].AluOpcode  = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1   = MHW_MI_ALU_SRCA;
        aluParams[aluCount].Operand2   = MHW_MI_ALU_GPREG0;
        ++aluCount;

        //load2 srcb, reg2
        aluParams[aluCount].AluOpcode  = MHW_MI_ALU_LOAD;
        aluParams[aluCount].Operand1   = MHW_MI_ALU_SRCB;
        aluParams[aluCount].Operand2   = MHW_MI_ALU_GPREG4;
        ++aluCount;
            
        //add srca + srcb
        aluParams[aluCount].AluOpcode  = MHW_MI_ALU_ADD;
        ++aluCount;
            
        //store reg1, accu
        aluParams[aluCount].AluOpcode  = MHW_MI_ALU_STORE;
        aluParams[aluCount].Operand1   = MHW_MI_ALU_GPREG0;
        aluParams[aluCount].Operand2   = MHW_MI_ALU_ACCU;
        ++aluCount;

        auto &miMathParams              = m_miItf->MHW_GETPAR_F(MI_MATH)();
        miMathParams                    = {};
        miMathParams.dwNumAluParams     = aluCount;
        miMathParams.pAluPayload        = aluParams;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuf));

        //store VCS_GPR0_Lo to bitstream size buffer
        auto &miStoreRegMemParams              = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                                  = {};
        miStoreRegMemParams.presStoreBuffer                  = bsSizeBuf;
        miStoreRegMemParams.dwOffset                         = 0;
        miStoreRegMemParams.dwRegister                       = mmioRegs->generalPurposeRegister0LoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuf));

        // Make Flush DW call to make sure all previous work is done
        auto &flushDwParams              = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                    = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuf));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        commandBufferSize = CalculateCommandBufferSize();
        requestedPatchListSize = CalculatePatchListSize();
        return MOS_STATUS_SUCCESS;
    }

    uint32_t Av1VdencPkt::CalculateCommandBufferSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t commandBufferSize = 0;

        uint32_t tileNum = 1;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileNum, tileNum);

        // To be refined later, differentiate BRC and CQP
        commandBufferSize =
            m_pictureStatesSize +
            (m_tileStatesSize * tileNum);

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return commandBufferSize;
    }

    uint32_t Av1VdencPkt::CalculatePatchListSize()
    {
        ENCODE_FUNC_CALL();
        uint32_t requestedPatchListSize = 0;

        uint32_t tileNum = 1;
        RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileNum, tileNum);

        if (m_usePatchList)
        {
            requestedPatchListSize =
                m_picturePatchListSize +
                (m_tilePatchListSize * tileNum);
        }

        return requestedPatchListSize;
    }

    MOS_STATUS Av1VdencPkt::CalculateVdencCommandsSize()
    {
        uint32_t vdencPictureStatesSize    = 0;
        uint32_t vdencPicturePatchListSize = 0;
        uint32_t vdencTileStatesSize       = 0;
        uint32_t vdencTilePatchListSize    = 0;

        // Picture Level Commands
        ENCODE_CHK_STATUS_RETURN(GetVdencStateCommandsDataSize(
            &vdencPictureStatesSize,
            &vdencPicturePatchListSize));

        m_pictureStatesSize    += vdencPictureStatesSize;
        m_picturePatchListSize += vdencPicturePatchListSize;

        // Tile Level Commands
        ENCODE_CHK_STATUS_RETURN(GetVdencPrimitiveCommandsDataSize(
            &vdencTileStatesSize,
            &vdencTilePatchListSize));

        m_tileStatesSize    += vdencTileStatesSize;
        m_tilePatchListSize += vdencTilePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::CalculateAvpPictureStateCommandSize(uint32_t * commandsSize, uint32_t * patchListSize)
    {
        // there are no picture level AVP commands for encode
        commandsSize  = 0;
        patchListSize = 0;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::CalculateAvpCommandsSize()
    {
        uint32_t avpPictureStatesSize    = 0;
        uint32_t avpPicturePatchListSize = 0;
        uint32_t avpTileStatesSize       = 0;
        uint32_t avpTilePatchListSize    = 0;

        // Picture Level Commands
        ENCODE_CHK_STATUS_RETURN(CalculateAvpPictureStateCommandSize(&avpPictureStatesSize, &avpPicturePatchListSize));

        m_pictureStatesSize    += avpPictureStatesSize;
        m_picturePatchListSize += avpPicturePatchListSize;

        // Tile Level Commands
        ENCODE_CHK_STATUS_RETURN(GetAvpPrimitiveCommandsDataSize(
            &avpTileStatesSize,
            &avpTilePatchListSize));

        m_tileStatesSize    += avpTileStatesSize;
        m_tilePatchListSize += avpTilePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, Av1VdencPkt)
    {
        switch (m_basicFeature->m_flushCmd)
        {
        case Av1BasicFeature::waitVdenc:
            params                           = {};
            params.waitDoneVDCmdMsgParser    = true;
            params.waitDoneVDENC             = true;
            params.flushVDENC                = true;
            params.flushAV1                  = true;
            params.waitDoneAV1               = true;
            break;
        case Av1BasicFeature::waitAvp:
            params                           = {};
            params.waitDoneVDCmdMsgParser    = true;
            params.waitDoneAV1               = true;
            params.flushAV1                  = true;
            break;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Av1VdencPkt)
    {
        params.pakObjCmdStreamOut = m_vdencPakObjCmdStreamOutEnabled;

        // needs to be enabled for 1st pass in multi-pass case
        // This bit is ignored if PAK only second pass is enabled.
        if ((m_pipeline->GetCurrentPass() == 0) && !m_pipeline->IsLastPass())
        {
            params.pakObjCmdStreamOut = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Av1VdencPkt)
    {
        params.intraRowStoreScratchBuffer       = m_vdencIntraRowStoreScratch;
        params.tileRowStoreBuffer               = m_vdencTileRowStoreBuffer;
        params.cumulativeCuCountStreamOutBuffer = m_resCumulativeCuCountStreamoutBuffer;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Av1VdencPkt)
    {
        switch (m_pipeline->GetPipeNum())
        {
        case 0:
        case 1:
            params.numPipe = VDENC_PIPE_SINGLE_PIPE;
            break;
        case 2:
            params.numPipe = VDENC_PIPE_TWO_PIPE;
            break;
        case 4:
            params.numPipe = VDENC_PIPE_FOUR_PIPE;
            break;
        default:
            params.numPipe = VDENC_PIPE_INVALID;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1VdencPkt)
    {
        params.surfaceStateId = m_curAvpSurfStateId;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIPE_MODE_SELECT, Av1VdencPkt)
    {
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
            params.pipeWorkingMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;
        }
        else
        {
            params.multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
            params.pipeWorkingMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
        }

        return MOS_STATUS_SUCCESS;
    }


    MOS_STATUS Av1VdencPkt::AddAllCmds_AVP_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        m_curAvpSurfStateId = srcInputPic;
        SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);

        m_curAvpSurfStateId = origUpscaledSrc;
        SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);

        m_curAvpSurfStateId = reconPic;
        SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);

        m_curAvpSurfStateId = av1CdefPixelsStreamout;
        SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);

        if (m_av1PicParams->PicFlags.fields.frame_type != keyFrame && m_av1PicParams->PicFlags.fields.frame_type != intraOnlyFrame)
        {
            for (uint8_t i = 0; i < av1TotalRefsPerFrame; i++)
            {
                m_curAvpSurfStateId = i + av1IntraFrame;
                SETPAR_AND_ADDCMD(AVP_SURFACE_STATE, m_avpItf, cmdBuffer);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::GetVdencStateCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const
    {
        uint32_t            maxSize          = 0;
        uint32_t            patchListMaxSize = 0;

        maxSize = maxSize +
            m_vdencItf->MHW_GETSIZE_F(VDENC_CONTROL_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_PIPE_MODE_SELECT)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_SRC_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_REF_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_DS_REF_SURFACE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_PIPE_BUF_ADDR_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_WALKER_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VD_PIPELINE_FLUSH)();

        patchListMaxSize = patchListMaxSize +
            PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(mhw::mi::Itf::MI_BATCH_BUFFER_START_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::vdenc::Itf::VDENC_PIPE_BUF_ADDR_STATE_CMD);

        maxSize = maxSize +
            m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_IMM)()*8 +
            m_miItf->MHW_GETSIZE_F(MI_FLUSH_DW)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

        ENCODE_CHK_NULL_RETURN(commandsSize);
        ENCODE_CHK_NULL_RETURN(patchListSize);
        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::UpdateStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer)
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
        registerMemParams.dwRegister      = mmioRegisters->generalPurposeRegister4LoOffset;  // VCS_GPR4_Lo
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

        mhw::mi::MHW_MI_ALU_PARAMS aluParams[4] = {0};
        int                        aluCount     = 0;

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

    MOS_STATUS Av1VdencPkt::GetVdencPrimitiveCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const
    {
        uint32_t            maxSize          = 0;
        uint32_t            patchListMaxSize = 0;

        maxSize = maxSize +
            m_vdencItf->MHW_GETSIZE_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_CMD1)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_CMD2)() +
            m_vdencItf->MHW_GETSIZE_F(VDENC_WALKER_STATE)();

        ENCODE_CHK_NULL_RETURN(commandsSize);
        ENCODE_CHK_NULL_RETURN(patchListSize);
        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::GetAvpPrimitiveCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const
    {
        uint32_t maxSize          = 0;
        uint32_t patchListMaxSize = 0;

        maxSize = m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_START)() * 5 +
            m_miItf->MHW_GETSIZE_F(VD_CONTROL_STATE)()                +
            m_avpItf->MHW_GETSIZE_F(AVP_PIPE_MODE_SELECT)() * 2       +
            m_avpItf->MHW_GETSIZE_F(AVP_SURFACE_STATE)() * 11         +
            m_avpItf->MHW_GETSIZE_F(AVP_PIPE_BUF_ADDR_STATE)()        +
            m_avpItf->MHW_GETSIZE_F(AVP_IND_OBJ_BASE_ADDR_STATE)()    +
            m_avpItf->MHW_GETSIZE_F(AVP_PIC_STATE)()                  +
            m_avpItf->MHW_GETSIZE_F(AVP_INTER_PRED_STATE)()           +
            m_avpItf->MHW_GETSIZE_F(AVP_SEGMENT_STATE)() * 8          +
            m_avpItf->MHW_GETSIZE_F(AVP_INLOOP_FILTER_STATE)()        +
            m_avpItf->MHW_GETSIZE_F(AVP_TILE_CODING)()                +
            m_avpItf->MHW_GETSIZE_F(AVP_PAK_INSERT_OBJECT)() * 9      +
            m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

        patchListMaxSize =
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::VD_PIPELINE_FLUSH_CMD)           +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIPE_MODE_SELECT_CMD)        +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_SURFACE_STATE_CMD) * 11      +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIPE_BUF_ADDR_STATE_CMD)     +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_IND_OBJ_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIC_STATE_CMD)               +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_INTER_PRED_STATE_CMD)        +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_SEGMENT_STATE_CMD) * 8       +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_INLOOP_FILTER_STATE_CMD)     +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_TILE_CODING_CMD)             +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PAK_INSERT_OBJECT_CMD) * 9;

        ENCODE_CHK_NULL_RETURN(commandsSize);
        ENCODE_CHK_NULL_RETURN(patchListSize);
        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Av1VdencPkt::DumpResources(EncodeStatusMfx *encodeStatusMfx, EncodeStatusReportData *statusReportData)
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
        debugInterface->m_bufferDumpFrameNum = m_statusReport->GetReportedCount();
        debugInterface->m_frameType          = encodeStatusMfx->pictureCodingType;

        if (m_resVDEncPakObjCmdStreamOutBuffer != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                m_resVDEncPakObjCmdStreamOutBuffer,
                CodechalDbgAttr::attrPakObjStreamout,
                "_PakObj",
                m_basicFeature->m_mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }

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

        auto streamInBufferSize = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;
        PMOS_RESOURCE streamInBuffer = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, m_basicFeature->m_frameNum);
        if (streamInBuffer != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
                streamInBuffer,
                CodechalDbgAttr::attrStreamIn,
                "StreamIn",
                streamInBufferSize,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            &currRefList.sRefReconBuffer,
            CodechalDbgAttr::attrReconstructedSurface,
            "ReconSurf",
            CODECHAL_NUM_MEDIA_STATES,
            m_basicFeature->m_frameWidth,
            m_basicFeature->m_frameHeight))

        MOS_SURFACE* postCdefSurface = m_basicFeature->m_trackedBuf->GetSurface(
            BufferType::postCdefReconSurface, currRefList.ucScalingIdx);
        if (postCdefSurface != nullptr)
        {
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
                postCdefSurface,
                CodechalDbgAttr::attrReconstructedSurface,
                "PostCdefReconSurf",
                CODECHAL_NUM_MEDIA_STATES,
                m_basicFeature->m_frameWidth,
                m_basicFeature->m_frameHeight))
        }

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpYUVSurface(
            &currRefList.sRefRawBuffer,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf"))

        return MOS_STATUS_SUCCESS;
    }

#endif

}  // namespace encode
