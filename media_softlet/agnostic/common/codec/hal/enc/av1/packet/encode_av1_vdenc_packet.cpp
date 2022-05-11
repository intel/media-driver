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
#include "codechal_utilities.h"
#include "encode_av1_vdenc_packet.h"
#include "encode_status_report_defs.h"
#include "codec_def_common_av1.h"
#include "codechal_mmc.h"

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

        MediaPerfProfilerNext *perfProfiler = MediaPerfProfilerNext::Instance();
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

        MediaPerfProfilerNext *perfProfiler = MediaPerfProfilerNext::Instance();
        ENCODE_CHK_NULL_RETURN(perfProfiler);
        ENCODE_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd(
            (void *)m_pipeline, m_osInterface, m_miItf, cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::ReadAvpStatus(MHW_VDBOX_NODE_IND vdboxIndex, MediaStatusReport *statusReport, MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        CODECHAL_HW_FUNCTION_ENTER;

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

        CODECHAL_DEBUG_TOOL(
            CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
            ENCODE_CHK_NULL_RETURN(debugInterface);
            if (debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar)) {
                InitParParams();
            })

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

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    void Av1VdencPkt::SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType)
    {
        ENCODE_FUNC_CALL();

        PerfTagSetting perfTag;
        perfTag.Value             = 0;
        perfTag.Mode              = mode & CODECHAL_ENCODE_MODE_BIT_MASK;
        perfTag.CallType          = type;
        perfTag.PictureCodingType = picCodingType > 3 ? 0 : picCodingType;
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
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
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

            ENCODE_CHK_STATUS_RETURN(m_hwInterface->SetRowstoreCachingOffsets(&rowStoreParams));
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
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_vdencIntraRowStoreScratch);

        allocParamsForBufferLinear.Flags.bNotLockable = !(m_basicFeature->m_lockableResource);
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_basicFeature->m_vdencBrcStatsBufferSize * maxTileNumber, MHW_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDEncStatsBuffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        m_resVDEncStatsBuffer               = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_resVDEncStatsBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::ReadPakMmioRegisters(PMOS_COMMAND_BUFFER cmdBuf, bool firstTile)
    {
        ENCODE_FUNC_CALL();

        CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuf);

        auto mmioRegs = m_miItf->GetMmioRegisters();
        auto mmioRegsAvp = m_avpItf->GetMmioRegisters(MHW_VDBOX_NODE_1);
        CODECHAL_ENCODE_CHK_NULL_RETURN(mmioRegs);
        PMOS_RESOURCE bsSizeBuf = m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);
        CODECHAL_ENCODE_CHK_NULL_RETURN(bsSizeBuf);

        if (firstTile)
        {
            // clear bitstream size buffer at first tile
            auto &miStoreDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            miStoreDataParams                  = {};
            miStoreDataParams.pOsResource      = bsSizeBuf;
            miStoreDataParams.dwResourceOffset = 0;
            miStoreDataParams.dwValue          = 0;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(cmdBuf));
        }

        // load current tile size to VCS_GPR0_Lo
        auto &miLoadRegaParams         = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_REG)();
        miLoadRegaParams               = {};
        miLoadRegaParams.dwSrcRegister = mmioRegsAvp->avpAv1BitstreamByteCountTileRegOffset;
        miLoadRegaParams.dwDstRegister = mmioRegs->generalPurposeRegister0LoOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_REG)(cmdBuf));
      
        // load bitstream size buffer to VCS_GPR4_Lo
        auto &miLoadRegMemParams           = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_MEM)();
        miLoadRegMemParams                 = {};
        miLoadRegMemParams.presStoreBuffer = bsSizeBuf;
        miLoadRegMemParams.dwOffset        = 0;
        miLoadRegMemParams.dwRegister      = mmioRegs->generalPurposeRegister4LoOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_MEM)(cmdBuf));

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
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_MATH)(cmdBuf));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mos_AddCommand(
                cmdBuf,
                &miMathParams.pAluPayload[0],
                sizeof(mhw::mi::MHW_MI_ALU_PARAMS) * miMathParams.dwNumAluParams));

        //store VCS_GPR0_Lo to bitstream size buffer
        auto &miStoreRegMemParams              = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                                  = {};
        miStoreRegMemParams.presStoreBuffer                  = bsSizeBuf;
        miStoreRegMemParams.dwOffset                         = 0;
        miStoreRegMemParams.dwRegister                       = mmioRegs->generalPurposeRegister0LoOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuf));

        // Make Flush DW call to make sure all previous work is done
        auto &flushDwParams              = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                    = {};
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuf));

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
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CalculateAvpPictureStateCommandSize(&avpPictureStatesSize, &avpPicturePatchListSize));

        m_pictureStatesSize    += avpPictureStatesSize;
        m_picturePatchListSize += avpPicturePatchListSize;

        // Tile Level Commands
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetAvpPrimitiveCommandsDataSize(
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

    static bool CheckUniformSpacing(const uint32_t *size, uint32_t num)
    {
        for (uint8_t i = 1; i < num; i++)
        {
            if (size[i] != size[0])
            {
                return false;
            }
        }

        return true;
    }

    static bool IsPowerOfTwo(const size_t number)
    {
        return ((number & (number - 1)) == 0);
    }

    static uint8_t Log2(const size_t number)
    {
        size_t power2 = 0;
        size_t num    = number;
        while (num >>= 1)
            ++power2;

        return power2;
    }

    MOS_STATUS Av1VdencPkt::PopulateParFileParams(const CODEC_AV1_ENCODE_SEQUENCE_PARAMS *seqParams,
        const CODEC_AV1_ENCODE_PICTURE_PARAMS *                                           picParams,
        const uint32_t                                                                    numTileGroups,
        const CODEC_AV1_ENCODE_TILE_GROUP_PARAMS *                                        tileGroupParams,
        const MHW_BATCH_BUFFER *                                                          batchBuffer)
    {
        m_av1Par->NumFrames             = m_basicFeature->m_encodedFrameNum + 1;
        m_av1Par->Width                 = picParams->frame_width_minus1 + 1;
        m_av1Par->Height                = picParams->frame_height_minus1 + 1;
        m_av1Par->NumP                  = seqParams->GopPicSize;

        m_av1Par->ChromaFormatIDC  = m_basicFeature->m_outputChromaFormat;
        m_av1Par->InputBitDepth    = m_basicFeature->m_is10Bit ? 10 : 8;
        m_av1Par->OutputBitDepth   = m_basicFeature->m_is10Bit ? 10 : 8;
        m_av1Par->InternalBitDepth = m_basicFeature->m_is10Bit ? 10 : 8;
        m_av1Par->InputFileFormat  = m_basicFeature->m_is10Bit ? 2 : 0;

        m_av1Par->NumTileCols = picParams->tile_cols;
        for (uint32_t i = 0; i < m_av1Par->NumTileCols; i++)
        {
            m_av1Par->TileWidths[i] = picParams->width_in_sbs_minus_1[i] + 1;
        }

        m_av1Par->NumTileRows = picParams->tile_rows;
        for (uint32_t i = 0; i < m_av1Par->NumTileRows; i++)
        {
            m_av1Par->TileHeights[i] = picParams->height_in_sbs_minus_1[i] + 1;
        }

        m_av1Par->UniformSpacingFlag =
            CheckUniformSpacing(m_av1Par->TileWidths, m_av1Par->NumTileCols) &&
            CheckUniformSpacing(m_av1Par->TileHeights, m_av1Par->NumTileRows) &&
            IsPowerOfTwo(picParams->tile_cols) &&
            IsPowerOfTwo(picParams->tile_rows);

        if (m_av1Par->UniformSpacingFlag)
        {
            m_av1Par->Log2TileCols = Log2(picParams->tile_cols);
            m_av1Par->Log2TileRows = Log2(picParams->tile_rows);
        }

        m_av1Par->NumTileGroup        = numTileGroups;
        m_av1Par->DisableCdfUpdate    = picParams->PicFlags.fields.disable_cdf_update;
        m_av1Par->FrameParallel       = picParams->PicFlags.fields.disable_frame_end_update_cdf;
        m_av1Par->ContextUpdateTileId = picParams->context_update_tile_id;

        for (int8_t i = 0; i < AV1_NUM_OF_REF_LF_DELTAS; i++)
        {
            m_av1Par->RefTypeLFDelta[i] = picParams->ref_deltas[i];
        }
        // QP
        if (picParams->PicFlags.fields.frame_type == keyFrame)
        {
            m_av1Par->BaseKeyFrameQP = picParams->base_qindex;
            m_av1Par->BaseLoopFilterLevel[0] = picParams->filter_level[0];

            m_av1Par->KeyCTQPDelta[AV1_Y_DC] = picParams->y_dc_delta_q;
            m_av1Par->KeyCTQPDelta[AV1_U_DC] = picParams->u_dc_delta_q;
            m_av1Par->KeyCTQPDelta[AV1_U_AC] = picParams->u_ac_delta_q;
            m_av1Par->KeyCTQPDelta[AV1_V_DC] = picParams->v_dc_delta_q;
            m_av1Par->KeyCTQPDelta[AV1_V_AC] = picParams->v_ac_delta_q;

            for (uint8_t i = 0; i < av1MaxSegments; i++)
            {
                m_av1Par->KeyFrameQP[i] = picParams->base_qindex +
                                          picParams->stAV1Segments.feature_data[i][segLvlAltQ];
                m_av1Par->SegLoopFilterLevel[0][i] = picParams->filter_level[0] +
                                                     picParams->stAV1Segments.feature_data[i][segLvlAltLfYh];
            }
        }
        else
        {
            m_av1Par->BasePFrameQP = picParams->base_qindex;
            // Cmodel assigns BaseLoopFilterLevel[1] to "PBaseLoopFilterLevel"
            m_av1Par->BaseLoopFilterLevel[1] = picParams->filter_level[0];
            m_av1Par->PCTQPDelta[AV1_Y_DC]   = picParams->y_dc_delta_q;
            m_av1Par->PCTQPDelta[AV1_U_DC]   = picParams->u_dc_delta_q;
            m_av1Par->PCTQPDelta[AV1_U_AC]   = picParams->u_ac_delta_q;
            m_av1Par->PCTQPDelta[AV1_V_DC]   = picParams->v_dc_delta_q;
            m_av1Par->PCTQPDelta[AV1_V_AC]   = picParams->v_ac_delta_q;

            for (uint8_t i = 0; i < av1MaxSegments; i++)
            {
                m_av1Par->PFrameQP[i] = picParams->base_qindex +
                                        picParams->stAV1Segments.feature_data[i][segLvlAltQ];
                // Cmodel assigns SegLoopFilterLevel[1] to "PSegLoopFilterLevel"
                m_av1Par->SegLoopFilterLevel[1][i] = picParams->filter_level[0] +
                                                     picParams->stAV1Segments.feature_data[i][segLvlAltLfYh];
            }
        }

        // segmentation
        m_av1Par->EnableSeg = m_av1Par->StreamInEnable = m_av1Par->StreamInSegEnable =
            picParams->stAV1Segments.SegmentFlags.fields.segmentation_enabled;

        if (m_av1Par->EnableSeg)
        {
            if (picParams->PicFlags.fields.frame_type != keyFrame)
            {
                m_av1Par->SegMapUpdateCycle =
                    picParams->stAV1Segments.SegmentFlags.fields.update_map ? 1 : (std::numeric_limits<uint32_t>::max)();

                m_av1Par->SegTemporalUpdate = picParams->stAV1Segments.SegmentFlags.fields.temporal_update;
            }

            for (uint8_t i = 0; i < av1MaxSegments; i++)
            {
                m_av1Par->SegmentRef[i]      = -1;
                m_av1Par->SegmentSkip[i]     = 0;
                m_av1Par->SegmentGlobalMV[i] = 0;
            }
        }

        const CommonStreamInParams& par = m_basicFeature->m_streamIn.GetCommonParams();

        m_av1Par->StreamInMaxCuSize                = par.MaxCuSize;
        m_av1Par->StreamInMaxTuSize                = par.MaxTuSize;
        m_av1Par->StreamInNumImePredictors         = par.NumImePredictors;
        m_av1Par->StreamInNumMergeCandidateCu8x8   = par.NumMergeCandidateCu8x8 ;
        m_av1Par->StreamInNumMergeCandidateCu16x16 = par.NumMergeCandidateCu16x16;
        m_av1Par->StreamInNumMergeCandidateCu32x32 = par.NumMergeCandidateCu32x32;
        m_av1Par->StreamInNumMergeCandidateCu64x64 = par.NumMergeCandidateCu64x64;

        m_av1Par->FrameRateNom   = seqParams->FrameRate[0].Numerator;
        m_av1Par->FrameRateDeNom = seqParams->FrameRate[0].Denominator;
        m_av1Par->EncMode        = 2;  // VDEnc
        m_av1Par->McMode         = picParams->interp_filter;
        m_av1Par->AllowHpMv      = picParams->PicFlags.fields.allow_high_precision_mv;

#if 0  // get CompPredMode from DDI                          \
       // DDI 0.06 still uses obsolete version of mode types \
       // so can't copy directly from DDI to Cmodel par
        m_av1Par->CompPredMode = picParams->dwModeControlFlags.fields.reference_mode == singleRefernece ? 0 : 1;
#else
        m_av1Par->CompPredMode = 0;
#endif
        m_av1Par->CompPredMode = picParams->dwModeControlFlags.fields.reference_mode == referenceModeSelect;

        m_av1Par->CDEFMode = seqParams->CodingToolFlags.fields.enable_cdef;
        m_av1Par->LRMode   = seqParams->CodingToolFlags.fields.enable_restoration;

        // GOP and ref lists (only one ref frame case)
        m_av1Par->IntraPeriod                    = seqParams->GopPicSize;
        m_av1Par->BGOPSize                       = 1;
        m_av1Par->PerBFramePOC[0]                = 1;
        m_av1Par->PerBFrameNumRefPics[0]         = 2;
        m_av1Par->PerBFrameRefPics[0]            = -1;
        m_av1Par->PerBFrameRefPics[1]            = -2;

        // for now only I, P frames are supported
        if (picParams->PicFlags.fields.frame_type != keyFrame && picParams->PicFlags.fields.frame_type != intraOnlyFrame)
        {
            uint8_t numRefsL0 = 0;
            uint8_t tempRefL0 = m_basicFeature->m_ref.RefFrameL0L1(picParams->ref_frame_ctrl_l0);
            uint8_t numRefsL1 = 0;
            uint8_t tempRefL1 = m_basicFeature->m_ref.RefFrameL0L1(picParams->ref_frame_ctrl_l1);

            for (uint32_t i = 0; i < 7; i++)
            {
                if (tempRefL0 & (1 << i))
                {
                    numRefsL0++;
                }
            }

            for (uint32_t i = 0; i < 7; i++)
            {
                if (tempRefL1 & (1 << i))
                {
                    numRefsL1++;
                }
            }

            m_av1Par->PerBFrameNumRefPicsActiveL0[0] = numRefsL0;
            m_av1Par->PerBFrameNumRefPicsActiveL1[0] = numRefsL1;

            if (numRefsL0 + numRefsL1 == 3)
            {
                if (numRefsL0 == 3)
                {
                    m_av1Par->PerBFrameNumRefPics[0] = 3;
                    m_av1Par->PerBFrameRefPics[2] = -3;
                }
                m_av1Par->AdditionalFWDAlphaSearchEnable = 1;
                m_av1Par->AdditionalFWDBetaSearchEnable  = 1;
            }
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        MediaUserSetting::Value outValue;
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            "AV1 Encode RDO Enable",
            MediaUserSetting::Group::Sequence);
        m_av1Par->RdoEnable               = outValue.Get<bool>();
#endif
        m_av1Par->AdaptiveRounding        = m_basicFeature->m_adaptiveRounding;

        m_av1Par->EnableSuperResolution   = m_basicFeature->m_av1SeqParams->CodingToolFlags.fields.enable_superres &&
                                          m_basicFeature->m_av1PicParams->PicFlags.fields.use_superres;
        m_av1Par->SuperresScaleDenominator = m_basicFeature->m_av1PicParams->superres_scale_denominator;

        m_av1Par->LRMode = seqParams->CodingToolFlags.fields.enable_restoration;
        m_av1Par->LRFilterTypeY = picParams->LoopRestorationFlags.fields.yframe_restoration_type;
        m_av1Par->LRFilterTypeU = picParams->LoopRestorationFlags.fields.cbframe_restoration_type;
        m_av1Par->LRFilterTypeV = picParams->LoopRestorationFlags.fields.crframe_restoration_type;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::InitParParams()
    {
        m_av1Par = MOS_New(EncodeAv1Par);
        ENCODE_CHK_NULL_RETURN(m_av1Par);
        MOS_ZeroMemory(m_av1Par, sizeof(EncodeAv1Par));
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1VdencPkt::PopulateParFileParams()
    {
        CodechalDebugInterface* debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        if (debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
        {
            auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
            ENCODE_CHK_NULL_RETURN(basicFeature);

            PCODEC_AV1_ENCODE_TILE_GROUP_PARAMS tileGroupParams = nullptr;
            uint32_t                            numTileGroups = 0;
            RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileGroupInfo, tileGroupParams, numTileGroups);
            ENCODE_CHK_NULL_RETURN(tileGroupParams);

            PMHW_BATCH_BUFFER tileBatchBuf = nullptr;
            RUN_FEATURE_INTERFACE_RETURN(Av1EncodeTile, Av1FeatureIDs::encodeTile, GetTileLevelBatchBuffer, tileBatchBuf);

            auto brcFeature = dynamic_cast<Av1Brc*>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
            ENCODE_CHK_NULL_RETURN(brcFeature);
            auto vdenc2ndLevelBatchBuffer = brcFeature->GetVdenc2ndLevelBatchBuffer(m_pipeline->m_currRecycledBufIdx);

            auto tempBatchBuffer = brcFeature->IsBRCEnabled() ? vdenc2ndLevelBatchBuffer : tileBatchBuf;
            tempBatchBuffer->OsResource.pData = (uint8_t *)m_allocator->LockResourceForRead(&(tempBatchBuffer->OsResource));
            ENCODE_CHK_NULL_RETURN(tempBatchBuffer->OsResource.pData);

            ENCODE_CHK_STATUS_RETURN(PopulateParFileParams(basicFeature->m_av1SeqParams,
                basicFeature->m_av1PicParams,
                numTileGroups,
                tileGroupParams,
                tempBatchBuffer));

            ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(&(tempBatchBuffer->OsResource)));
        }

        return MOS_STATUS_SUCCESS;
    }

    template <typename T, size_t N>
    static void PrintNumEntries(std::ostringstream &oss, T (&array)[N], uint32_t num)
    {
        for (uint32_t i = 0; i < num; i++)
        {
            // mimic output in Cmodel golden par
            oss << std::dec << array[i];
            if (i == num - 1)
                oss << std::endl;
            else
                oss << ",";
        }
    }

    MOS_STATUS Av1VdencPkt::DumpSeqParFile()
    {
        CODECHAL_DEBUG_FUNCTION_ENTER;
        std::ostringstream oss;
        oss.setf(std::ios::showbase | std::ios::uppercase);

        oss << "NumFrames = " << std::dec << m_av1Par->NumFrames << std::endl;
        oss << "GopRefDist = " << std::dec << m_av1Par->GopRefDist << std::endl;
        oss << "GopOptFlag = " << std::dec << m_av1Par->GopOptFlag << std::endl;
        oss << "SourceFile = " << std::endl;
        oss << "DstFile = " << std::endl;
        oss << "SourceWidth = " << std::dec << m_av1Par->Width << std::endl;
        oss << "SourceHeight = " << std::dec << m_av1Par->Height << std::endl;
        oss << "NumP = " << std::dec << m_av1Par->NumP << std::endl;
        oss << "KeyIntraPrediction = " << std::dec << m_av1Par->KeyIntraPrediction << std::endl;
        oss << "PIntraPrediction = " << std::dec << m_av1Par->PIntraPrediction << std::endl;
        oss << "ChromaFormatIDC = " << std::dec << m_av1Par->ChromaFormatIDC << std::endl;
        oss << "InputBitDepth = " << std::dec << m_av1Par->InputBitDepth << std::endl;
        oss << "OutputBitDepth = " << std::dec << m_av1Par->OutputBitDepth << std::endl;
        oss << "InternalBitDepth = " << std::dec << m_av1Par->InternalBitDepth << std::endl;
        oss << "InputFileFormat = " << std::dec << m_av1Par->InputFileFormat << std::endl;
        oss << "NumTileCols = " << std::dec << m_av1Par->NumTileCols << std::endl;
        oss << "NumTileRows = " << std::dec << m_av1Par->NumTileRows << std::endl;
        oss << "UniformSpacingFlag = " << std::dec << m_av1Par->UniformSpacingFlag << std::endl;

        if (m_av1Par->NumTileCols > 0)
        {
            oss << "TileWidths = ";
            for (uint32_t i = 0; i < m_av1Par->NumTileCols - 1; i++)
            {
                oss << std::dec << m_av1Par->TileWidths[i] << ",";
            }
            oss << std::dec << m_av1Par->TileWidths[m_av1Par->NumTileCols - 1] << std::endl;
        }

        if (m_av1Par->NumTileRows > 0)
        {
            oss << "TileHeights = ";
            for (uint32_t i = 0; i < m_av1Par->NumTileRows - 1; i++)
            {
                oss << std::dec << m_av1Par->TileHeights[i] << ",";
            }
            oss << std::dec << m_av1Par->TileHeights[m_av1Par->NumTileRows - 1] << std::endl;
        }

        oss << "Log2TileCols = " << std::dec << m_av1Par->Log2TileCols << std::endl;
        oss << "Log2TileRows = " << std::dec << m_av1Par->Log2TileRows << std::endl;
        oss << "NumTileGroup = " << std::dec << m_av1Par->NumTileGroup << std::endl;
        oss << "BaseKeyFrameQP = " << std::dec << m_av1Par->BaseKeyFrameQP << std::endl;
        oss << "BasePFrameQP = " << std::dec << m_av1Par->BasePFrameQP << std::endl;
        oss << "KeyCTQPDelta = " << std::dec << m_av1Par->KeyCTQPDelta[AV1_Y_DC] << ","
            << m_av1Par->KeyCTQPDelta[AV1_U_DC] << ","
            << m_av1Par->KeyCTQPDelta[AV1_U_AC] << ","
            << m_av1Par->KeyCTQPDelta[AV1_V_DC] << ","
            << m_av1Par->KeyCTQPDelta[AV1_V_AC] << std::endl;
        oss << "PCTQPDelta = " << std::dec << m_av1Par->PCTQPDelta[AV1_Y_DC] << ","
            << m_av1Par->PCTQPDelta[AV1_U_DC] << ","
            << m_av1Par->PCTQPDelta[AV1_U_AC] << ","
            << m_av1Par->PCTQPDelta[AV1_V_DC] << ","
            << m_av1Par->PCTQPDelta[AV1_V_AC] << std::endl;

        const uint8_t numEntriesToPrint = 8;

        oss << "RefTypeLFDelta = ";
        // mimic output in Cmodel golden par
        PrintNumEntries(oss, m_av1Par->RefTypeLFDelta, numEntriesToPrint);

        // print segmentation parameters
        oss << "SegKeyFrameQP = ";
        // mimic output in Cmodel golden par
        PrintNumEntries(oss, m_av1Par->KeyFrameQP, numEntriesToPrint);
        oss << "SegPFrameQP = ";
        // mimic output in Cmodel golden par
        PrintNumEntries(oss, m_av1Par->PFrameQP, numEntriesToPrint);

        oss << "EnableSeg = " << std::dec << m_av1Par->EnableSeg << std::endl;
        if (m_av1Par->EnableSeg)
        {
            oss << "StreamInEnable = " << std::dec << m_av1Par->StreamInEnable << std::endl;
            oss << "StreamInSegEnable = " << std::dec << m_av1Par->StreamInSegEnable << std::endl;
            oss << "SegMapUpdateCycle = " << std::dec << m_av1Par->SegMapUpdateCycle << std::endl;
            oss << "SegTemporalUpdate = " << std::dec << m_av1Par->SegTemporalUpdate << std::endl;

            oss << "KeySegLoopFilterLevel = ";
            PrintNumEntries(oss, m_av1Par->SegLoopFilterLevel[0], numEntriesToPrint);
            oss << "PSegLoopFilterLevel = ";
            PrintNumEntries(oss, m_av1Par->SegLoopFilterLevel[1], numEntriesToPrint);

            oss << "SegRef = ";
            PrintNumEntries(oss, m_av1Par->SegmentRef, numEntriesToPrint);
            oss << "SegSkip = ";
            PrintNumEntries(oss, m_av1Par->SegmentSkip, numEntriesToPrint);
            oss << "SegGlobalMV = ";
            PrintNumEntries(oss, m_av1Par->SegmentGlobalMV, numEntriesToPrint);

            oss << "KeyLumaLoopFilterLevelVert = " << std::dec << m_av1Par->BaseLoopFilterLevel[0] << std::endl;
            oss << "KeyLumaLoopFilterLevelHor = " << std::dec << m_av1Par->BaseLoopFilterLevel[0] << std::endl;
            oss << "KeyChromaULoopFilterLevel = " << std::dec << m_av1Par->BaseLoopFilterLevel[0] << std::endl;
            oss << "KeyChromaVLoopFilterLevel = " << std::dec << m_av1Par->BaseLoopFilterLevel[0] << std::endl;
            oss << "PLumaLoopFilterLevelVert = " << std::dec << m_av1Par->BaseLoopFilterLevel[1] << std::endl;
            oss << "PLumaLoopFilterLevelHor = " << std::dec << m_av1Par->BaseLoopFilterLevel[1] << std::endl;
            oss << "PChromaULoopFilterLevel = " << std::dec << m_av1Par->BaseLoopFilterLevel[1] << std::endl;
            oss << "PChromaVLoopFilterLevel = " << std::dec << m_av1Par->BaseLoopFilterLevel[1] << std::endl;
        }

        if (m_av1Par->StreamInEnable)
        {
            oss << "StreamInMaxCuSize = " << std::dec << m_av1Par->StreamInMaxCuSize << std::endl;
            oss << "StreamInMaxTuSize = " << std::dec << m_av1Par->StreamInMaxTuSize << std::endl;
            oss << "StreamInNumImePredictors = " << std::dec << m_av1Par->StreamInNumImePredictors << std::endl;
            oss << "StreamInNumMergeCandidateCu8x8 = " << std::dec << m_av1Par->StreamInNumMergeCandidateCu8x8 << std::endl;
            oss << "StreamInNumMergeCandidateCu16x16 = " << std::dec << m_av1Par->StreamInNumMergeCandidateCu16x16 << std::endl;
            oss << "StreamInNumMergeCandidateCu32x32 = " << std::dec << m_av1Par->StreamInNumMergeCandidateCu32x32 << std::endl;
            oss << "StreamInNumMergeCandidateCu64x64 = " << std::dec << m_av1Par->StreamInNumMergeCandidateCu64x64 << std::endl;
        }


        oss << "FrameRateNom = " << std::dec << m_av1Par->FrameRateNom << std::endl;
        oss << "FrameRateDeNom = " << std::dec << m_av1Par->FrameRateDeNom << std::endl;
        oss << "EncMode = " << std::dec << m_av1Par->EncMode << std::endl;
        oss << "McMode = " << std::dec << m_av1Par->McMode << std::endl;
        oss << "AllowHpMv = " << std::dec << m_av1Par->AllowHpMv << std::endl;
        oss << "CompPredMode = " << std::dec << m_av1Par->CompPredMode << std::endl;
        oss << "CDEFMode = " << std::dec << m_av1Par->CDEFMode << std::endl;
        oss << "RdoEnable = " << std::dec << m_av1Par->RdoEnable << std::endl;
        oss << "LRMode = " << std::dec << m_av1Par->LRMode << std::endl;
        oss << "LRFilterTypeY = " << std::dec << m_av1Par->LRFilterTypeY << std::endl;
        oss << "LRFilterTypeU = " << std::dec << m_av1Par->LRFilterTypeU << std::endl;
        oss << "LRFilterTypeV = " << std::dec << m_av1Par->LRFilterTypeV << std::endl;

        oss << "IntraPeriod = " << std::dec << m_av1Par->IntraPeriod << std::endl;
        oss << "BGOPSize = " << std::dec << m_av1Par->BGOPSize << std::endl;
        if (m_av1Par->IntraPeriod > 1 && m_av1Par->BGOPSize > 0)
        {
            oss << "PerBFramePOC = ";
            PrintNumEntries(oss, m_av1Par->PerBFramePOC, m_av1Par->BGOPSize);
            oss << "PerBFrameNumRefPics = ";
            PrintNumEntries(oss, m_av1Par->PerBFrameNumRefPics, m_av1Par->BGOPSize);
            oss << "PerBFrameRefPics = ";
            PrintNumEntries(oss, m_av1Par->PerBFrameRefPics, std::accumulate(m_av1Par->PerBFrameNumRefPics, m_av1Par->PerBFrameNumRefPics + m_av1Par->BGOPSize, 0));
            oss << "PerBFrameNumRefPicsActiveL0 = ";
            PrintNumEntries(oss, m_av1Par->PerBFrameNumRefPicsActiveL0, m_av1Par->BGOPSize);
            oss << "PerBFrameNumRefPicsActiveL1 = ";
            PrintNumEntries(oss, m_av1Par->PerBFrameNumRefPicsActiveL1, m_av1Par->BGOPSize);
            oss << "AdditionalFWDAlphaSearchEnable = " << std::dec << m_av1Par->AdditionalFWDAlphaSearchEnable << std::endl;
            oss << "AdditionalFWDBetaSearchEnable = " << std::dec << m_av1Par->AdditionalFWDBetaSearchEnable << std::endl;
        }

        oss << "DisableCdfUpdate = " << std::dec << m_av1Par->DisableCdfUpdate << std::endl;
        oss << "FrameParallel = " << std::dec << m_av1Par->FrameParallel << std::endl;
        oss << "ContextUpdateTileId = " << std::dec << m_av1Par->ContextUpdateTileId << std::endl;
        oss << "EnableSuperResolution = " << std::dec << m_av1Par->EnableSuperResolution << std::endl;
        oss << "SuperresScaleDenominator = " << std::dec << m_av1Par->SuperresScaleDenominator << std::endl;

        // oss << "ProfileIDC = " << std::dec << +m_avcPar->ProfileIDC << std::endl;
        CodechalDebugInterface* debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);
        const char* fileName = debugInterface->CreateFileName(
            "EncodeSequence",
            "EncodePar",
            CodechalDbgExtType::par);

        std::ofstream ofs(fileName, std::ios::out);
        ofs << oss.str();
        ofs.close();

        return MOS_STATUS_SUCCESS;
    }
#endif

}  // namespace encode
