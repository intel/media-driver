/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_decode_mpeg2_g12.cpp
//! \brief    Implements the decode interface extension for MPEG2.
//! \details  Implements all functions and constants required by CodecHal for MPEG2 decoding.
//!

#include "codechal_decoder.h"
#include "codechal_decode_mpeg2_g12.h"
#include "codechal_secure_decode_interface.h"
#include "mhw_vdbox_mfx_g12_X.h"
#include "codechal_mmc_decode_mpeg2_g12.h"
#include "hal_oca_interface.h"

CodechalDecodeMpeg2G12::~CodechalDecodeMpeg2G12 ()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_veState != nullptr)
    {
        MOS_FreeMemAndSetNull(m_veState);
        m_veState = nullptr;
    }
}

MOS_STATUS  CodechalDecodeMpeg2G12::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeMpeg2G12, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeMpeg2G12::SetGpuCtxCreatOption(
    CodechalSetting *codecHalSetting)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CodechalDecode::SetGpuCtxCreatOption(codecHalSetting);
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
        bool sfcInUse    = (codecHalSetting->sfcInUseHinted && codecHalSetting->downsamplingHinted
                            && (MEDIA_IS_SKU(m_skuTable, FtrSFCPipe) && !MEDIA_IS_SKU(m_skuTable, FtrDisableVDBox2SFC)));
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
            m_veState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt,
            sfcInUse));
        m_videoContext = MOS_GPU_CONTEXT_VIDEO;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeMpeg2G12::SetFrameStates ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeMpeg2::SetFrameStates());

    if (MOS_VE_SUPPORTED(m_osInterface) && !MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;

        MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
        vesetParams.bSFCInUse                   = false;
        vesetParams.bNeedSyncWithPrevious       = true;
        vesetParams.bSameEngineAsLastSubmission = false;
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_SetHintParams(m_veState, &vesetParams));
    }

#ifdef _MMC_SUPPORTED
    // To WA invalid aux data caused HW issue when MMC on
    if (m_mmc && m_mmc->IsMmcEnabled() && MEDIA_IS_WA(m_waTable, Wa_1408785368) && 
        !Mos_ResourceIsNull(&m_destSurface.OsResource) && 
        m_destSurface.OsResource.bConvertedFromDDIResource)
    {
        CODECHAL_DECODE_VERBOSEMESSAGE("Clear CCS by VE resolve before frame %d submission", m_frameNum);
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnDecompResource(m_osInterface, &m_destSurface.OsResource));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext));
    }
#endif

    return eStatus;
}

MOS_STATUS CodechalDecodeMpeg2G12::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint8_t fwdRefIdx = (uint8_t)m_picParams->m_forwardRefIdx;
    uint8_t bwdRefIdx = (uint8_t)m_picParams->m_backwardRefIdx;

    // Do not use data that has not been initialized
    if (CodecHal_PictureIsInvalid(m_mpeg2RefList[fwdRefIdx]->RefPic))
    {
        fwdRefIdx = m_picParams->m_currPic.FrameIdx;
    }
    if (CodecHal_PictureIsInvalid(m_mpeg2RefList[bwdRefIdx]->RefPic))
    {
        bwdRefIdx = m_picParams->m_currPic.FrameIdx;
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);
    HalOcaInterface::On1stLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);

    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl = true;
    forceWakeupParams.bMFXPowerWellControlMask = true;
    forceWakeupParams.bHEVCPowerWellControl = false;
    forceWakeupParams.bHEVCPowerWellControlMask = true;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
        &cmdBuffer,
        &forceWakeupParams));

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode                  = m_mode;
    pipeModeSelectParams.bStreamOutEnabled     = m_streamOutEnabled;
    pipeModeSelectParams.bPostDeblockOutEnable = m_deblockingEnabled;
    pipeModeSelectParams.bPreDeblockOutEnable  = !m_deblockingEnabled;

    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode                      = m_mode;
    surfaceParams.psSurface                 = &m_destSurface;

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams.Mode                      = m_mode;
    if (m_deblockingEnabled)
    {
        pipeBufAddrParams.psPostDeblockSurface = &m_destSurface;
    }
    else
    {
        pipeBufAddrParams.psPreDeblockSurface = &m_destSurface;
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(&pipeBufAddrParams));
#endif

    // when there is not a forward or backward reference,
    // the index is set to the destination frame index
    m_presReferences[CodechalDecodeFwdRefTop] =
        m_presReferences[CodechalDecodeFwdRefBottom] =
            m_presReferences[CodechalDecodeBwdRefTop] =
                m_presReferences[CodechalDecodeBwdRefBottom] = &m_destSurface.OsResource;

    if (fwdRefIdx < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        m_presReferences[CodechalDecodeFwdRefTop] =
            m_presReferences[CodechalDecodeFwdRefBottom] = &m_mpeg2RefList[fwdRefIdx]->resRefPic;
    }
    if (bwdRefIdx < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        m_presReferences[CodechalDecodeBwdRefTop] =
            m_presReferences[CodechalDecodeBwdRefBottom] = &m_mpeg2RefList[bwdRefIdx]->resRefPic;
    }

    // special case for second fields
    if (m_picParams->m_secondField && m_picParams->m_pictureCodingType == P_TYPE)
    {
        if (m_picParams->m_topFieldFirst)
        {
            m_presReferences[CodechalDecodeFwdRefTop] =
                &m_destSurface.OsResource;
        }
        else
        {
            m_presReferences[CodechalDecodeFwdRefBottom] =
                &m_destSurface.OsResource;
        }
    }

    // set all ref pic addresses to valid addresses for error concealment purpose
    for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
    {
        if (m_presReferences[i] == nullptr && 
            MEDIA_IS_WA(m_waTable, WaDummyReference) && 
            !Mos_ResourceIsNull(&m_dummyReference.OsResource))
        {
            m_presReferences[i] = &m_dummyReference.OsResource;
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        pipeBufAddrParams.presReferences,
        sizeof(PMOS_RESOURCE) * CODEC_MAX_NUM_REF_FRAME_NON_AVC,
        m_presReferences,
        sizeof(PMOS_RESOURCE) * CODEC_MAX_NUM_REF_FRAME_NON_AVC));

    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer =
        &m_resMfdDeblockingFilterRowStoreScratchBuffer;

    if (m_streamOutEnabled)
    {
        pipeBufAddrParams.presStreamOutBuffer =
            &(m_streamOutBuffer[m_streamOutCurrBufIdx]);
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->CheckReferenceList(&pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetRefrenceSync(m_disableDecodeSyncLock, m_disableLockForTranscode));
#endif

    CODECHAL_DEBUG_TOOL(
        for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
        {
            if (pipeBufAddrParams.presReferences[i])
            {
                MOS_SURFACE dstSurface;

                MOS_ZeroMemory(&dstSurface, sizeof(MOS_SURFACE));
                dstSurface.Format     = Format_NV12;
                dstSurface.OsResource = *(pipeBufAddrParams.presReferences[i]);
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));

                m_debugInterface->m_refIndex = (uint16_t)i;
                std::string refSurfName      = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &dstSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.data()));
            }
        }
    )

    //set correctly indirect BSD object base address.
    PMOS_RESOURCE indObjBase;
    if (m_copiedDataBufferInUse)
    {
        indObjBase = &m_resCopiedDataBuffer[m_currCopiedData];
    }
    else
    {
        indObjBase = &m_resDataBuffer;
    }

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode               = m_mode;
    indObjBaseAddrParams.dwDataSize =
        m_copiedDataBufferInUse ? m_copiedDataBufferSize : m_dataSize;
    indObjBaseAddrParams.presDataBuffer     = indObjBase;

    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS bspBufBaseAddrParams;
    MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
    bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resBsdMpcRowStoreScratchBuffer;

    MHW_VDBOX_MPEG2_PIC_STATE mpeg2PicState;
    mpeg2PicState.Mode                                  = m_mode;
    mpeg2PicState.pMpeg2PicParams                       = m_picParams;
    mpeg2PicState.bDeblockingEnabled                    = m_deblockingEnabled;
    mpeg2PicState.dwMPEG2ISliceConcealmentMode          = m_mpeg2ISliceConcealmentMode;
    mpeg2PicState.dwMPEG2PBSliceConcealmentMode         = m_mpeg2PbSliceConcealmentMode;
    mpeg2PicState.dwMPEG2PBSlicePredBiDirMVTypeOverride = m_mpeg2PbSlicePredBiDirMvTypeOverride;
    mpeg2PicState.dwMPEG2PBSlicePredMVOverride          = m_mpeg2PbSlicePredMvOverride;

    MHW_VDBOX_QM_PARAMS qmParams;
    qmParams.Standard                       = CODECHAL_MPEG2;
    qmParams.pMpeg2IqMatrix                 = m_iqMatrixBuffer;

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(
            &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(
        &cmdBuffer,
        &pipeModeSelectParams));

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetSurfaceState(&surfaceParams));
#endif
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(
        &cmdBuffer,
        &surfaceParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(
        &cmdBuffer,
        &pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(
        &cmdBuffer,
        &indObjBaseAddrParams));

    if (CodecHalIsDecodeModeVLD(m_mode))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(
            &cmdBuffer,
            &bspBufBaseAddrParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxMpeg2PicCmd(
        &cmdBuffer,
        &mpeg2PicState));

    if (CodecHalIsDecodeModeVLD(m_mode))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxQmCmd(
            &cmdBuffer,
            &qmParams));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeMpeg2G12::SliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if ((m_decodePhantomMbs) || (m_incompletePicture))
    {
        if (m_bbInUsePerFrame >= m_bbAllocated / CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP)
        {
            m_bbAllocated += CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP;
            if (m_bbAllocated >= CODECHAL_DECODE_MPEG2_MAXIMUM_BATCH_BUFFERS)
            {
                CODECHAL_DECODE_ASSERTMESSAGE(
                    "The number of MPEG2 second level batch buffer is not big enough to hold the whole frame.");
                return MOS_STATUS_EXCEED_MAX_BB_SIZE;
            }

            for (uint32_t i = 0; i < CODECHAL_DECODE_MPEG2_BATCH_BUFFERS_PER_GROUP; i++)
            {
                uint32_t j = m_bbAllocated - i - 1;
                MOS_ZeroMemory(&m_mediaObjectBatchBuffer[j], sizeof(MHW_BATCH_BUFFER));

                uint32_t u32Size = m_standardDecodeSizeNeeded * m_picWidthInMb * m_picHeightInMb +
                                   m_hwInterface->m_sizeOfCmdBatchBufferEnd;

                CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                    m_osInterface,
                    &m_mediaObjectBatchBuffer[j],
                    nullptr,
                    u32Size));
                m_mediaObjectBatchBuffer[j].bSecondLevel = true;
            }
        }
    }

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    MHW_BATCH_BUFFER batchBuffer = m_mediaObjectBatchBuffer[m_bbInUse];

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
        &cmdBuffer,
        &batchBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_LockBb(
        m_osInterface,
        &batchBuffer));

    if (m_decodePhantomMbs)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(InsertDummySlices(
            &batchBuffer,
            m_lastMbAddress,
            m_picWidthInMb * m_picHeightInMb));
    }
    else
    {
        CodecDecodeMpeg2SliceParams *slc = m_sliceParams;

        uint16_t prevSliceMBEnd = m_lastMbAddress;

        for (uint16_t slcCount = 0; slcCount < m_numSlices; slcCount++)
        {
            if (!m_vldSliceRecord[slcCount].dwSkip)
            {
                if (prevSliceMBEnd != m_vldSliceRecord[slcCount].dwSliceStartMbOffset)
                {
                    CODECHAL_DECODE_CHK_STATUS_RETURN(InsertDummySlices(
                        &batchBuffer,
                        prevSliceMBEnd,
                        (uint16_t)m_vldSliceRecord[slcCount].dwSliceStartMbOffset));
                }

                if (m_vldSliceRecord[slcCount].bIsLastSlice)
                {
                    uint16_t expectedFinalMb = m_picWidthInMb * m_picHeightInMb;

                    m_lastMbAddress =
                        (uint16_t)(m_vldSliceRecord[slcCount].dwSliceStartMbOffset +
                                   slc->m_numMbsForSlice);

                    if (m_lastMbAddress < expectedFinalMb)
                    {
                        m_incompletePicture                = true;
                        m_vldSliceRecord[slcCount].bIsLastSlice = false;
                    }
                    else
                    {
                        //Indicate It's complete picture now
                        m_incompletePicture                = false;
                    }
                }

                // static MPEG2 slice parameters
                MHW_VDBOX_MPEG2_SLICE_STATE mpeg2SliceState;
                mpeg2SliceState.presDataBuffer          = &m_resDataBuffer;
                mpeg2SliceState.wPicWidthInMb           = m_picWidthInMb;
                mpeg2SliceState.wPicHeightInMb          = m_picHeightInMb;
                mpeg2SliceState.pMpeg2SliceParams       = slc;
                mpeg2SliceState.dwLength                = m_vldSliceRecord[slcCount].dwLength;
                mpeg2SliceState.dwOffset =
                    m_vldSliceRecord[slcCount].dwOffset + m_copiedDataOffset;
                mpeg2SliceState.dwSliceStartMbOffset = m_vldSliceRecord[slcCount].dwSliceStartMbOffset;
                mpeg2SliceState.bLastSlice           = m_vldSliceRecord[slcCount].bIsLastSlice;

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdMpeg2BsdObject(
                    nullptr,
                    &batchBuffer,
                    &mpeg2SliceState));

                prevSliceMBEnd =
                    (uint16_t)(m_vldSliceRecord[slcCount].dwSliceStartMbOffset +
                               slc->m_numMbsForSlice);
            }

            slc++;
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        nullptr,
        &batchBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
            &batchBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));
    )

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_UnlockBb(
        m_osInterface,
        &batchBuffer,
        true));

    m_bbInUse = (m_bbInUse + 1) % m_bbAllocated;
    m_bbInUsePerFrame++;

    if (!m_incompletePicture)
    {
        // Check if destination surface needs to be synchronized
        MOS_SYNC_PARAMS syncParams          = g_cInitSyncParams;
        syncParams.GpuContext               = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly                = false;
        syncParams.bDisableDecodeSyncLock   = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        if (!CodecHal_PictureIsField(m_picParams->m_currPic) ||
            !m_picParams->m_secondField)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(
                m_osInterface,
                &syncParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(
                m_osInterface,
                &syncParams));

            // Update the resource tag (s/w tag) for On-Demand Sync
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            &cmdBuffer,
            &flushDwParams));

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync &&
            (!CodecHal_PictureIsField(m_picParams->m_currPic) || m_picParams->m_secondField))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(
                &cmdBuffer,
                &syncParams));
        }

        if (m_statusQueryReportingEnabled)
        {
            CodechalDecodeStatusReport decodeStatusReport;

            decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
            decodeStatusReport.m_currDecodedPic     = m_picParams->m_currPic;
            decodeStatusReport.m_currDeblockedPic   = m_picParams->m_currPic;
            decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
            decodeStatusReport.m_currDecodedPicRes  = m_mpeg2RefList[m_picParams->m_currPic.FrameIdx]->resRefPic;

            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_secondField = m_picParams->m_secondField ? true : false;
                decodeStatusReport.m_frameType   = m_perfType;)

            CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(
                decodeStatusReport,
                &cmdBuffer));
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        CODECHAL_DEBUG_TOOL(
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                &cmdBuffer,
                CODECHAL_NUM_MEDIA_STATES,
                "_DEC"));

            //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
            //    m_debugInterface,
            //    &cmdBuffer));
        )

        //Sync up complete frame
        syncParams                      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContextForWa;
        syncParams.presSyncResource     = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams                      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContext;
        syncParams.presSyncResource     = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        if ( MOS_VE_SUPPORTED(m_osInterface))
        {
            CodecHalDecodeSinglePipeVE_PopulateHintParams(m_veState, &cmdBuffer, false);
        }

        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
            m_osInterface,
            &cmdBuffer,
            m_videoContextUsesNullHw));

        CODECHAL_DEBUG_TOOL(
            m_mmc->UpdateUserFeatureKey(&m_destSurface);)

        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(
                m_videoContextUsesNullHw));
        }

        // Needs to be re-set for Linux buffer re-use scenarios
        m_mpeg2RefList[m_picParams->m_currPic.FrameIdx]->resRefPic =
            m_destSurface.OsResource;

        // Send the signal to indicate decode completion, in case On-Demand Sync is not present
        if (!CodecHal_PictureIsField(m_picParams->m_currPic) || m_picParams->m_secondField)
        {
            syncParams                      = g_cInitSyncParams;
            syncParams.GpuContext           = m_videoContext;
            syncParams.presSyncResource     = &m_destSurface.OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
        }
    }
    else
    {
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeMpeg2G12::AllocateStandard (
    CodechalSetting *          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeMpeg2::AllocateStandard(
        settings));

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        static_cast<MhwVdboxMfxInterfaceG12*>(m_mfxInterface)->DisableScalabilitySupport();

        //single pipe VE initialize
        m_veState = (PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE));
        CODECHAL_DECODE_CHK_NULL_RETURN(m_veState);
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_InitInterface(m_osInterface, m_veState));
    }

    return eStatus;
}

CodechalDecodeMpeg2G12::CodechalDecodeMpeg2G12 (
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecodeMpeg2(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    Mos_CheckVirtualEngineSupported(m_osInterface, true, true);
}

