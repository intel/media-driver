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
//! \file     codechal_decode_vc1_g12.cpp
//! \brief    Implements the decode interface extension for Gen12 VC1.
//! \details  Implements all functions required by CodecHal for Gen12 VC1 decoding.
//!

#include "codeckrnheader.h"

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif

#include "codechal_decode_vc1_g12.h"
#include "codechal_secure_decode_interface.h"
#include "mhw_vdbox_mfx_g12_X.h"
#include "codechal_mmc_decode_vc1_g12.h"
#include "mhw_render_g12_X.h"
#include "hal_oca_interface.h"

MOS_STATUS CodechalDecodeVc1G12::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeVc1G12, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVc1G12::AllocateStandard(
    CodechalSetting *          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVc1::AllocateStandard(settings));

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

MOS_STATUS CodechalDecodeVc1G12::SetGpuCtxCreatOption(
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

MOS_STATUS CodechalDecodeVc1G12::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVc1::SetFrameStates());

    if (MOS_VE_SUPPORTED(m_osInterface) && !MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;

        MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
        vesetParams.bSFCInUse = false;
        vesetParams.bNeedSyncWithPrevious = true;
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

    bool isBPicture = m_mfxInterface->IsVc1BPicture(
                        m_vc1PicParams->CurrPic,
                        m_vc1PicParams->picture_fields.is_first_field,
                        m_vc1PicParams->picture_fields.picture_type);

    bool isOverlapSmoothingFilter = false;

    isOverlapSmoothingFilter |= m_vc1PicParams->sequence_fields.AdvancedProfileFlag &&
                                m_vc1PicParams->sequence_fields.overlap;

    isOverlapSmoothingFilter |= !(isBPicture ||
                                  (m_vc1PicParams->pic_quantizer_fields.pic_quantizer_scale < 9) ||
                                  !m_vc1PicParams->sequence_fields.overlap);

    isOverlapSmoothingFilter |= m_intelEntrypointInUse &&
                                (m_mode == CODECHAL_DECODE_MODE_VC1VLD) &&
                                m_vc1PicParams->conditional_overlap_flag;

    //For VC1 frame with overlap smoothing filter enabled, force to post-deblock if MMC on
    if (m_mmc && m_mmc->IsMmcEnabled() && isOverlapSmoothingFilter)
    {
        m_deblockingEnabled = true;
    }
#endif

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1G12::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODEC_REF_LIST     *vc1RefList;
    vc1RefList = &(m_vc1RefList[0]);

    uint8_t destIdx   = m_vc1PicParams->CurrPic.FrameIdx;
    uint8_t fwdRefIdx = (uint8_t)m_vc1PicParams->ForwardRefIdx;
    uint8_t bwdRefIdx = (uint8_t)m_vc1PicParams->BackwardRefIdx;

    bool isIPicture = m_mfxInterface->IsVc1IPicture(
                          m_vc1PicParams->CurrPic,
                          m_vc1PicParams->picture_fields.is_first_field,
                          m_vc1PicParams->picture_fields.picture_type)
                          ? true
                          : false;
    bool isPPicture = m_mfxInterface->IsVc1PPicture(
                          m_vc1PicParams->CurrPic,
                          m_vc1PicParams->picture_fields.is_first_field,
                          m_vc1PicParams->picture_fields.picture_type)
                          ? true
                          : false;
    bool isBPicture = m_mfxInterface->IsVc1BPicture(
                          m_vc1PicParams->CurrPic,
                          m_vc1PicParams->picture_fields.is_first_field,
                          m_vc1PicParams->picture_fields.picture_type)
                          ? true
                          : false;

    PMOS_SURFACE    destSurface;
    PMOS_RESOURCE   fwdRefSurface, bwdRefSurface;
    if (m_unequalFieldWaInUse && CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
    {
        destSurface =
            &(m_unequalFieldSurface[vc1RefList[destIdx]->dwUnequalFieldSurfaceIdx]);
        fwdRefSurface =
            &(m_unequalFieldSurface[vc1RefList[fwdRefIdx]->dwUnequalFieldSurfaceIdx].OsResource);
        bwdRefSurface =
            &(m_unequalFieldSurface[vc1RefList[bwdRefIdx]->dwUnequalFieldSurfaceIdx].OsResource);

        // Overwrite the actual surface height with the coded height and width of the frame
        // for VC1 since it's possible for a VC1 frame to change size during playback
        destSurface->dwWidth = m_width;
        destSurface->dwHeight = m_height;
    }
    else
    {
        destSurface   = &m_destSurface;
        fwdRefSurface = &(vc1RefList[fwdRefIdx]->resRefPic);
        bwdRefSurface = &(vc1RefList[bwdRefIdx]->resRefPic);
    }

    // WA for SP/MP short format
    if (m_shortFormatInUse &&
        !m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ConstructBistreamBuffer());
    }

    MOS_COMMAND_BUFFER  cmdBuffer;
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

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS   pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    pipeModeSelectParams.bStreamOutEnabled = m_streamOutEnabled;
    pipeModeSelectParams.bPostDeblockOutEnable = m_deblockingEnabled;
    pipeModeSelectParams.bPreDeblockOutEnable  = !m_deblockingEnabled;
    pipeModeSelectParams.bShortFormatInUse     = m_shortFormatInUse;
    pipeModeSelectParams.bVC1OddFrameHeight    = m_vc1OddFrameHeight;

    MHW_VDBOX_SURFACE_PARAMS    surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode = m_mode;
    surfaceParams.psSurface = destSurface;

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS  pipeBufAddrParams;
    pipeBufAddrParams.Mode = m_mode;
    if (m_deblockingEnabled)
    {
        pipeBufAddrParams.psPostDeblockSurface = destSurface;
    }
    else
    {
        pipeBufAddrParams.psPreDeblockSurface = destSurface;
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(&pipeBufAddrParams));
#endif

        // when there is not a forward or backward reference,
        // the index is set to the destination frame index
    m_presReferences[CodechalDecodeFwdRefTop] =
        m_presReferences[CodechalDecodeFwdRefBottom] =
            fwdRefSurface;
    m_presReferences[CodechalDecodeBwdRefTop] =
        m_presReferences[CodechalDecodeBwdRefBottom] =
            bwdRefSurface;
    // special case for second fields
    if (!m_vc1PicParams->picture_fields.is_first_field &&
        !m_mfxInterface->IsVc1IPicture(
            m_vc1PicParams->CurrPic,
            m_vc1PicParams->picture_fields.is_first_field,
            m_vc1PicParams->picture_fields.picture_type))
    {
        if (m_vc1PicParams->picture_fields.top_field_first)
        {
            m_presReferences[CodechalDecodeFwdRefTop] =
                &destSurface->OsResource;
        }
        else
        {
            m_presReferences[CodechalDecodeFwdRefBottom] =
                &destSurface->OsResource;
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

    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(pipeBufAddrParams.presReferences, sizeof(PMOS_RESOURCE) * CODEC_MAX_NUM_REF_FRAME_NON_AVC, m_presReferences, sizeof(PMOS_RESOURCE) * CODEC_MAX_NUM_REF_FRAME_NON_AVC));

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
        for (int i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
        {
            if (pipeBufAddrParams.presReferences[i])
            {
                MOS_SURFACE dstSurface;

                MOS_ZeroMemory(&dstSurface, sizeof(MOS_SURFACE));
                dstSurface.Format = Format_NV12;
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

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS      indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = m_mode;
    if (m_shortFormatInUse &&
        !m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        indObjBaseAddrParams.dwDataSize     = m_dataSize + CODECHAL_DECODE_VC1_STUFFING_BYTES;
        indObjBaseAddrParams.presDataBuffer = &m_resPrivateBistreamBuffer;
    }
    else
    {
        indObjBaseAddrParams.dwDataSize     = m_dataSize;
        indObjBaseAddrParams.presDataBuffer = &m_resDataBuffer;
    }

    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS  bspBufBaseAddrParams;
    MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
    bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resBsdMpcRowStoreScratchBuffer;

    if (m_vc1PicParams->raw_coding.bitplane_present || m_shortFormatInUse)
    {
        bspBufBaseAddrParams.presBitplaneBuffer = &m_resBitplaneBuffer;
    }

    MHW_VDBOX_VC1_PRED_PIPE_PARAMS  vc1PredPipeParams;
    vc1PredPipeParams.pVc1PicParams = m_vc1PicParams;
    vc1PredPipeParams.ppVc1RefList = vc1RefList;

    MHW_VDBOX_VC1_PIC_STATE vc1PicState;
    vc1PicState.pVc1PicParams             = m_vc1PicParams;
    vc1PicState.Mode = m_mode;
    vc1PicState.ppVc1RefList = vc1RefList;
    vc1PicState.wPrevAnchorPictureTFF     = m_prevAnchorPictureTff;
    vc1PicState.bPrevEvenAnchorPictureIsP = m_prevEvenAnchorPictureIsP;
    vc1PicState.bPrevOddAnchorPictureIsP  = m_prevOddAnchorPictureIsP;

    if (m_shortFormatInUse)
    {
        // WA for WMP. WMP does not provide REFDIST for I/P pictures correctly
        if (m_vc1PicParams->sequence_fields.AdvancedProfileFlag &&
            CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
            (isIPicture || isPPicture) &&
            m_vc1PicParams->reference_fields.reference_distance_flag)
        {
            if (m_vc1PicParams->picture_fields.is_first_field)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(ParsePictureHeader());
                m_referenceDistance = m_vc1PicParams->reference_fields.reference_distance;
            }
            else
            {
                m_vc1PicParams->reference_fields.reference_distance = m_referenceDistance;
            }
        }

        // WA for WMP. WMP does not provide BFRACTION correctly. So parse picture header to get BFRACTION
        if (isBPicture)
        {
            if (m_vc1PicParams->picture_fields.is_first_field)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(ParsePictureHeader());
            }
        }
    }

    MHW_VDBOX_VC1_DIRECTMODE_PARAMS vc1DirectmodeParams;
    if (m_mode == CODECHAL_DECODE_MODE_VC1VLD)
    {
        uint8_t dmvBufferIdx                   = (m_vc1PicParams->CurrPic.PicFlags == PICTURE_BOTTOM_FIELD) ? CODECHAL_DECODE_VC1_DMV_ODD : CODECHAL_DECODE_VC1_DMV_EVEN;
        vc1DirectmodeParams.presDmvReadBuffer  = &m_resVc1BsdMvData[dmvBufferIdx];
        vc1DirectmodeParams.presDmvWriteBuffer = &m_resVc1BsdMvData[dmvBufferIdx];
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, !m_olpNeeded));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer));
    }

    if (!CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        m_vc1PicParams->picture_fields.picture_type == vc1SkippedFrame)
    {
        // no further picture level commands needed for skipped frames
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        return eStatus;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetSurfaceState(&surfaceParams));
#endif
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    if (m_mode == CODECHAL_DECODE_MODE_VC1VLD)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(&cmdBuffer, &bspBufBaseAddrParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxVc1PredPipeCmd(&cmdBuffer, &vc1PredPipeParams));

    if (m_intelEntrypointInUse || m_mode == CODECHAL_DECODE_MODE_VC1IT)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxVc1LongPicCmd(&cmdBuffer, &vc1PicState));
    }
    else if (m_shortFormatInUse)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVc1ShortPicCmd(&cmdBuffer, &vc1PicState));
    }
    else
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported decode mode.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    if (m_mode == CODECHAL_DECODE_MODE_VC1VLD)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxVc1DirectmodeCmd(&cmdBuffer, &vc1DirectmodeParams));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1G12::DecodePrimitiveLevelVLD()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_SYNC_PARAMS syncParams;

    // static VC1 slice parameters
    MHW_VDBOX_VC1_SLICE_STATE vc1SliceState;
    vc1SliceState.presDataBuffer = &m_resDataBuffer;

    uint16_t frameFieldHeightInMb;
    CodecHal_GetFrameFieldHeightInMb(
        m_vc1PicParams->CurrPic,
        m_picHeightInMb,
        frameFieldHeightInMb);

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (m_incompletePicture)
    {
        MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
        MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
        forceWakeupParams.bMFXPowerWellControl = true;
        forceWakeupParams.bMFXPowerWellControlMask = true;
        forceWakeupParams.bHEVCPowerWellControl = false;
        forceWakeupParams.bHEVCPowerWellControlMask = true;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
            &cmdBuffer,
            &forceWakeupParams));
    }

    if (!CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        m_vc1PicParams->picture_fields.picture_type == vc1SkippedFrame)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(HandleSkipFrame());
        goto submit;
    }
    else
    {
        PCODEC_VC1_SLICE_PARAMS slc             = m_vc1SliceParams;
        bool firstValidSlice = true;
        int prevValidSlc = 0;
        for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
        {
            m_vldSliceRecord[slcCount].dwSliceYOffset     = slc->slice_vertical_position;
            m_vldSliceRecord[slcCount].dwNextSliceYOffset = frameFieldHeightInMb;  // init to last slice

            int32_t lLength = slc->slice_data_size >> 3;
            int32_t lOffset = slc->macroblock_offset >> 3;

            // Check that the slice data does not overrun the bitstream buffer size
            if ((slc->slice_data_offset + lLength) > m_dataSize)
            {
                lLength = m_dataSize - slc->slice_data_offset;

                if (lLength < 0)
                {
                    lLength = 0;
                }
            }

            // Error handling for garbage data
            if (slc->slice_data_offset > m_dataSize)
            {
                slc++;
                m_vldSliceRecord[slcCount].dwSkip = true;
                continue;
            }

            // Check offset not larger than slice length, can have slice length of 0
            if (lOffset > lLength)
            {
                slc++;
                m_vldSliceRecord[slcCount].dwSkip = true;
                continue;
            }

            // Check that the slices do not overlap, else do not send the lower slice
            if (!firstValidSlice &&
                (m_vldSliceRecord[slcCount].dwSliceYOffset <= m_vldSliceRecord[prevValidSlc].dwSliceYOffset))
            {
                slc++;
                m_vldSliceRecord[slcCount].dwSkip = true;
                continue;
            }

            if (firstValidSlice)
            {
                // Ensure that the first slice starts from 0
                m_vldSliceRecord[slcCount].dwSliceYOffset = 0;
                slc->slice_vertical_position = 0;
            }
            else
            {
                // Set next slice start Y offset of previous slice
                m_vldSliceRecord[prevValidSlc].dwNextSliceYOffset =
                    m_vldSliceRecord[slcCount].dwSliceYOffset;
            }

            if (m_shortFormatInUse)
            {
                if (m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
                {
                    if ((slc->macroblock_offset >> 3) < CODECHAL_DECODE_VC1_SC_PREFIX_LENGTH)
                    {
                        slc++;
                        m_vldSliceRecord[slcCount].dwSkip = true;
                        continue;
                    }

                    // set macroblock_offset of the first slice to 0 to avoid crash
                    if (slcCount == 0)
                    {
                        slc->macroblock_offset = CODECHAL_DECODE_VC1_SC_PREFIX_LENGTH << 3;
                    }

                    lOffset = CODECHAL_DECODE_VC1_SC_PREFIX_LENGTH;
                }
                else // Simple Profile or Main Profile
                {
                    {
                        lOffset = CODECHAL_DECODE_VC1_STUFFING_BYTES - 1;
                        lLength += CODECHAL_DECODE_VC1_STUFFING_BYTES;
                        slc->macroblock_offset += CODECHAL_DECODE_VC1_STUFFING_BYTES << 3;
                        slc->macroblock_offset &= (~0x7); // Clear bit offset of first MB for short format
                    }
                }
            }

            m_vldSliceRecord[slcCount].dwOffset = lOffset;
            m_vldSliceRecord[slcCount].dwLength = lLength - lOffset;
            firstValidSlice = false;
            prevValidSlc = slcCount;
            slc++;
        }

        if (m_shortFormatInUse &&
            m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetSliceMbDataOffset());
        }

        // Reset slc pointer
        slc -= m_numSlices;

        //------------------------------------
        // Fill BSD Object Commands
        //------------------------------------
        for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
        {
            if (m_vldSliceRecord[slcCount].dwSkip)
            {
                slc++;
                continue;
            }

            vc1SliceState.pSlc = slc;
            vc1SliceState.dwOffset               = m_vldSliceRecord[slcCount].dwOffset;
            vc1SliceState.dwLength               = m_vldSliceRecord[slcCount].dwLength;
            vc1SliceState.dwNextVerticalPosition = m_vldSliceRecord[slcCount].dwNextSliceYOffset;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVc1BsdObjectCmd(&cmdBuffer, &vc1SliceState));

            slc++;
        }

        // Free VLD slice record
        MOS_ZeroMemory(m_vldSliceRecord, (m_numSlices * sizeof(CODECHAL_VC1_VLD_SLICE_RECORD)));
    }

    // Check if destination surface needs to be synchronized
    if (m_unequalFieldWaInUse &&
        CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        !m_vc1PicParams->picture_fields.is_first_field)
    {
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));
    }
    else
    {
        // Check if destination surface needs to be synchronized
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly = false;
        syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        if (!CodecHal_PictureIsField(m_vc1PicParams->CurrPic) ||
            m_vc1PicParams->picture_fields.is_first_field)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

            // Update the resource tag (s/w tag) for On-Demand Sync
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync &&
            !(CodecHal_PictureIsField(m_vc1PicParams->CurrPic) && m_vc1PicParams->picture_fields.is_first_field))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
        }
    }

submit:
    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;

        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_currDecodedPic     = m_vc1PicParams->CurrPic;
        if (m_olpNeeded)
        {
            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_currDeblockedPic.FrameIdx = (uint8_t)m_vc1PicParams->DeblockedPicIdx;
                decodeStatusReport.m_currDeblockedPic.PicFlags = PICTURE_FRAME;)
            decodeStatusReport.m_deblockedPicResOlp = m_deblockSurface.OsResource;
        }
        else
        {
            decodeStatusReport.m_currDeblockedPic = m_vc1PicParams->CurrPic;
        }
        decodeStatusReport.m_codecStatus = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_currDecodedPicRes = m_vc1RefList[m_vc1PicParams->CurrPic.FrameIdx]->resRefPic;

        CODECHAL_DEBUG_TOOL(
            decodeStatusReport.m_secondField =
                (m_vc1PicParams->picture_fields.is_first_field == 1) ? false : true;
            decodeStatusReport.m_olpNeeded = m_olpNeeded;
            decodeStatusReport.m_frameType = m_perfType;)

        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(decodeStatusReport, &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

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

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        CodecHalDecodeSinglePipeVE_PopulateHintParams(m_veState, &cmdBuffer, false);
    }

    if (m_huCCopyInUse)
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        m_huCCopyInUse = false;
    }

    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);)

    if (m_unequalFieldWaInUse &&
        CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        !m_vc1PicParams->picture_fields.is_first_field)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_vc1RefList);

        uint32_t destFrameIdx = m_vc1PicParams->CurrPic.FrameIdx;

        CODECHAL_DECODE_CHK_STATUS_RETURN(FormatUnequalFieldPicture(
            m_unequalFieldSurface[m_vc1RefList[destFrameIdx]->dwUnequalFieldSurfaceIdx],
            m_destSurface,
            true,
            m_videoContextUsesNullHw ? true : false));
    }

    if (m_olpNeeded)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(PerformVc1Olp());
    }
    else
    {
        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_videoContextUsesNullHw));
        }
    }

    // Needs to be re-set for Linux buffer re-use scenarios
    m_vc1RefList[m_vc1PicParams->CurrPic.FrameIdx]->resRefPic = m_destSurface.OsResource;

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    if (!(CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
            m_vc1PicParams->picture_fields.is_first_field))
    {
        MOS_SYNC_PARAMS syncParams;
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_destSurface.OsResource;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));

        if (m_olpNeeded)
        {
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_renderContext;
            syncParams.presSyncResource = &m_deblockSurface.OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
        }
    }

    m_olpNeeded = false;

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1G12::DecodePrimitiveLevelIT()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_SYNC_PARAMS syncParams;

    PCODEC_VC1_MB_PARAMS mb = m_vc1MbParams;

    MHW_VDBOX_VC1_MB_STATE vc1MbState;
    MOS_ZeroMemory(&vc1MbState, sizeof(vc1MbState));

    // static VC1 MB parameters
    vc1MbState.presDataBuffer     = &m_resDataBuffer;
    vc1MbState.pVc1PicParams      = m_vc1PicParams;
    vc1MbState.pWaTable = m_waTable;
    vc1MbState.pDeblockDataBuffer = m_deblockDataBuffer;
    vc1MbState.dwDataSize         = m_dataSize;
    vc1MbState.wPicWidthInMb      = m_picWidthInMb;
    vc1MbState.wPicHeightInMb     = m_picHeightInMb;
    vc1MbState.PicFlags           = m_vc1PicParams->CurrPic.PicFlags;
    vc1MbState.bFieldPolarity     = m_fieldPolarity;

    uint16_t frameFieldHeightInMb;
    CodecHal_GetFrameFieldHeightInMb(
        m_vc1PicParams->CurrPic,
        m_picHeightInMb,
        frameFieldHeightInMb);

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (m_incompletePicture)
    {
        MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
        MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
        forceWakeupParams.bMFXPowerWellControl = true;
        forceWakeupParams.bMFXPowerWellControlMask = true;
        forceWakeupParams.bHEVCPowerWellControl = false;
        forceWakeupParams.bHEVCPowerWellControlMask = true;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
            &cmdBuffer,
            &forceWakeupParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_itObjectBatchBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_LockBb(m_osInterface, &m_itObjectBatchBuffer));

    PMHW_BATCH_BUFFER batchBuffer = &m_itObjectBatchBuffer;

    uint32_t mbAddress = 0;
    uint32_t mbCount;
    for (mbCount = 0; mbCount < m_numMacroblocks; mbCount++)
    {
        vc1MbState.pMb = mb + mbCount;

        // Skipped MBs before current MB
        uint16_t skippedMBs = (mbCount) ?
            (mb[mbCount].mb_address - mb[mbCount - 1].mb_address - 1) :
            (mb[mbCount].mb_address);

        while (skippedMBs--)
        {
            vc1MbState.bMbHorizOrigin = (uint8_t)(mbAddress % m_picWidthInMb);
            vc1MbState.bMbVertOrigin  = (uint8_t)(mbAddress / m_picWidthInMb);
            vc1MbState.bSkipped = true;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVc1ItObjectCmd(batchBuffer, &vc1MbState));

            mbAddress++;
        }

        // Current MB
        if (mbCount + 1 == m_numMacroblocks)
        {
            vc1MbState.dwLength = m_dataSize - mb[mbCount].data_offset;
        }
        else
        {
            vc1MbState.dwLength = mb[mbCount + 1].data_offset - mb[mbCount].data_offset;
        }

        vc1MbState.bMbHorizOrigin = mb[mbCount].mb_address % m_picWidthInMb;
        vc1MbState.bMbVertOrigin  = mb[mbCount].mb_address / m_picWidthInMb;
        vc1MbState.dwOffset = (vc1MbState.dwLength) ? mb[mbCount].data_offset : 0;
        vc1MbState.bSkipped = false;

        if (m_vc1PicParams->entrypoint_fields.loopfilter)
        {
            eStatus = MOS_SecureMemcpy(vc1MbState.DeblockData,
                CODEC_NUM_BLOCK_PER_MB,
                m_deblockDataBuffer + CODEC_NUM_BLOCK_PER_MB * mb[mbCount].mb_address,
                CODEC_NUM_BLOCK_PER_MB);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_DECODE_ASSERTMESSAGE("Failed to copy memory.");
                m_olpNeeded = false;
                return eStatus;
            }
        }

        if (!mb[mbCount].mb_type.intra_mb)
        {
            if (mb[mbCount].mb_type.motion_forward || mb[mbCount].mb_type.motion_backward)
            {
                PackMotionVectors(
                    &vc1MbState,
                    (short *)mb[mbCount].motion_vector,
                    (short *)vc1MbState.PackedLumaMvs,
                    (short *)&vc1MbState.PackedChromaMv);
            }
            else
            {
                mb[mbCount].mb_type.motion_forward = 1;
                MOS_ZeroMemory(vc1MbState.PackedLumaMvs, sizeof(vc1MbState.PackedLumaMvs)); // MV's of zero
                vc1MbState.bMotionSwitch = 0;
            }
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVc1ItObjectCmd(batchBuffer, &vc1MbState));

        mbAddress = mb[mbCount].mb_address;
    }

    m_fieldPolarity = vc1MbState.bFieldPolarity;

    // skipped MBs at the end
    uint16_t skippedMBs = m_picWidthInMb * frameFieldHeightInMb - mb[mbCount - 1].mb_address - 1;

    while (skippedMBs--)
    {
        vc1MbState.bSkipped = true;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVc1ItObjectCmd(batchBuffer, &vc1MbState));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, &m_itObjectBatchBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
            batchBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));
    )

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_UnlockBb(m_osInterface, &m_itObjectBatchBuffer, true));

    // Check if destination surface needs to be synchronized
    if (m_unequalFieldWaInUse &&
        CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
    {
        if (!m_vc1PicParams->picture_fields.is_first_field)
        {
            MHW_MI_FLUSH_DW_PARAMS flushDwParams;
            MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));
        }
    }
    else
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly = false;
        syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        if (!CodecHal_PictureIsField(m_vc1PicParams->CurrPic) ||
            m_vc1PicParams->picture_fields.is_first_field)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

            // Update the resource tag (s/w tag) for On-Demand Sync
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync &&
            !(CodecHal_PictureIsField(m_vc1PicParams->CurrPic) && m_vc1PicParams->picture_fields.is_first_field))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
        }
    }

    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;

        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_currDecodedPic     = m_vc1PicParams->CurrPic;
        if (m_olpNeeded)
        {
            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_currDeblockedPic.FrameIdx = (uint8_t)m_vc1PicParams->DeblockedPicIdx;
                decodeStatusReport.m_currDeblockedPic.PicFlags = PICTURE_FRAME;)
            decodeStatusReport.m_deblockedPicResOlp = m_deblockSurface.OsResource;
        }
        else
        {
            decodeStatusReport.m_currDeblockedPic = m_vc1PicParams->CurrPic;
        }
        decodeStatusReport.m_codecStatus = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_currDecodedPicRes = m_vc1RefList[m_vc1PicParams->CurrPic.FrameIdx]->resRefPic;

        CODECHAL_DEBUG_TOOL(
            decodeStatusReport.m_secondField =
                (m_vc1PicParams->picture_fields.is_first_field == 1) ? false : true;
            decodeStatusReport.m_olpNeeded = m_olpNeeded;
            decodeStatusReport.m_frameType = m_perfType;)

        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(decodeStatusReport, &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

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

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        CodecHalDecodeSinglePipeVE_PopulateHintParams(m_veState, &cmdBuffer, false);
    }

    if (m_huCCopyInUse)
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        m_huCCopyInUse = false;
    }

    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);)

    if (m_unequalFieldWaInUse &&
        CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        !m_vc1PicParams->picture_fields.is_first_field)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_vc1RefList);

        uint32_t destFrameIdx = m_vc1PicParams->CurrPic.FrameIdx;

        CODECHAL_DECODE_CHK_STATUS_RETURN(FormatUnequalFieldPicture(
            m_unequalFieldSurface[m_vc1RefList[destFrameIdx]->dwUnequalFieldSurfaceIdx],
            m_destSurface,
            true,
            m_videoContextUsesNullHw ? true : false));
        }

        if (m_olpNeeded)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(PerformVc1Olp());
    }
    else
    {
        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_videoContextUsesNullHw));
        }
    }

    // Needs to be re-set for Linux buffer re-use scenarios
    m_vc1RefList[m_vc1PicParams->CurrPic.FrameIdx]->resRefPic = m_destSurface.OsResource;

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    if (!(CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
            m_vc1PicParams->picture_fields.is_first_field))
    {
        MOS_SYNC_PARAMS syncParams;
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_destSurface.OsResource;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));

        if (m_olpNeeded)
        {
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_renderContext;
            syncParams.presSyncResource = &m_deblockSurface.OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
        }
    }

    m_olpNeeded = false;

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1G12::HandleSkipFrame()
{
    MOS_COMMAND_BUFFER                  cmdBuffer;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    MHW_GENERIC_PROLOG_PARAMS           genericPrologParams;
    MOS_SURFACE                         srcSurface;
    uint8_t                             fwdRefIdx;
    uint32_t                            surfaceSize;
    MOS_SYNC_PARAMS                     syncParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    fwdRefIdx = (uint8_t)m_vc1PicParams->ForwardRefIdx;

    MOS_ZeroMemory(&srcSurface, sizeof(MOS_SURFACE));
    srcSurface.Format = Format_NV12;
    srcSurface.OsResource = m_vc1RefList[fwdRefIdx]->resRefPic;
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, &srcSurface));

    CODECHAL_DECODE_CHK_NULL_RETURN(srcSurface.OsResource.pGmmResInfo);
    surfaceSize = ((srcSurface.OsResource.pGmmResInfo->GetArraySize()) > 1) ?
        ((uint32_t)(srcSurface.OsResource.pGmmResInfo->GetQPitchPlanar(GMM_PLANE_Y) *
                    srcSurface.OsResource.pGmmResInfo->GetRenderPitch())) :
        (uint32_t)(srcSurface.OsResource.pGmmResInfo->GetSizeMainSurface());

    if (m_hwInterface->m_noHuC)
    {
        CodechalDataCopyParams dataCopyParams;
        MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
        dataCopyParams.srcResource = &srcSurface.OsResource;
        dataCopyParams.srcSize     = surfaceSize;
        dataCopyParams.srcOffset = srcSurface.dwOffset;
        dataCopyParams.dstResource = &m_destSurface.OsResource;
        dataCopyParams.dstSize     = surfaceSize;
        dataCopyParams.dstOffset   = m_destSurface.dwOffset;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(&dataCopyParams));
    }
    else
    {
        m_huCCopyInUse = true;

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContextForWa));
        m_osInterface->pfnResetOsStates(m_osInterface);

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, false));

        CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
            &cmdBuffer,                             // pCmdBuffer
            &srcSurface.OsResource,                 // presSrc
            &m_destSurface.OsResource,              // presDst
            surfaceSize,                            // u32CopyLength
            srcSurface.dwOffset,                    // u32CopyInputOffset
            m_destSurface.dwOffset));               // u32CopyOutputOffset
#ifdef _MMC_SUPPORTED
        auto mmc = static_cast<CodechalMmcDecodeVc1G12*>(m_mmc);
        CODECHAL_DECODE_CHK_STATUS_RETURN(mmc->CopyAuxSurfForSkip(&cmdBuffer, &srcSurface.OsResource, &m_destSurface.OsResource));
#endif
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly = false;
        syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

        // Update the resource tag (s/w tag) for On-Demand Sync
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(
                &cmdBuffer,
                &syncParams));
        }

        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            &cmdBuffer,
            &flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        if ( MOS_VE_SUPPORTED(m_osInterface))
        {
            CodecHalDecodeSinglePipeVE_PopulateHintParams(m_veState, &cmdBuffer, false);
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext));
    }

    return (MOS_STATUS)eStatus;
}

MOS_STATUS CodechalDecodeVc1G12::PerformVc1Olp()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MhwRenderInterface *renderEngineInterface = m_hwInterface->GetRenderInterface();
    PMHW_KERNEL_STATE         kernelState           = &m_olpKernelState;
    PMHW_STATE_HEAP_INTERFACE stateHeapInterface = renderEngineInterface->m_stateHeapInterface;

    CODECHAL_DECODE_CHK_NULL_RETURN(stateHeapInterface);

    MOS_SYNC_PARAMS syncParams;
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource = &m_resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_renderContext;
    syncParams.presSyncResource = &m_resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

    // Initialize DSH kernel region
    m_osInterface->pfnSetGpuContext(m_osInterface, m_renderContext);
    m_osInterface->pfnResetOsStates(m_osInterface);

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | OLP_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    CodecHalGetResourceInfo(m_osInterface, &m_deblockSurface);  // DstSurface

#ifdef _MMC_SUPPORTED
    if (m_mmc && !m_mmc->IsMmcEnabled())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->DisableSurfaceMmcState(&m_deblockSurface));
    }
#endif

    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        kernelState,
        false,
        m_olpDshSize,
        false,
        m_decodeStatusBuf.m_swStoreData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(SetCurbeOlp());

    // Send HW commands (including SSH)
    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    MHW_PIPE_CONTROL_PARAMS pipeControlParams;
    MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetDefaultSSEuSetting(CODECHAL_MEDIA_STATE_OLP, false, false, false));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    if (renderEngineInterface->GetL3CacheConfig()->bL3CachingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->SetL3Cache(&cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->EnablePreemption(&cmdBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddPipelineSelectCmd(&cmdBuffer, false));

    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    // common function for codec needed when we make change for AVC
    MHW_RCS_SURFACE_PARAMS surfaceParamsSrc;
    MOS_ZeroMemory(&surfaceParamsSrc, sizeof(surfaceParamsSrc));
    surfaceParamsSrc.dwNumPlanes = 2;    // Y, UV
    surfaceParamsSrc.psSurface          = &m_destSurface;
    surfaceParamsSrc.psSurface->dwDepth = 1;    // depth needs to be 0 for codec 2D surface
                                                // Y Plane
    surfaceParamsSrc.dwBindingTableOffset[MHW_Y_PLANE] = CODECHAL_DECODE_VC1_OLP_SRC_Y;
    surfaceParamsSrc.ForceSurfaceFormat[MHW_Y_PLANE] = MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM;
    // UV Plane
    surfaceParamsSrc.dwBindingTableOffset[MHW_U_PLANE] = CODECHAL_DECODE_VC1_OLP_SRC_UV;
    surfaceParamsSrc.ForceSurfaceFormat[MHW_U_PLANE] = MHW_GFX3DSTATE_SURFACEFORMAT_R16_UINT;
    surfaceParamsSrc.dwBaseAddrOffset[MHW_U_PLANE] =
        m_destSurface.dwPitch *
        MOS_ALIGN_FLOOR(m_destSurface.UPlaneOffset.iYOffset, MOS_YTILE_H_ALIGNMENT);
    surfaceParamsSrc.dwHeightToUse[MHW_U_PLANE] = surfaceParamsSrc.psSurface->dwHeight / 2;
    surfaceParamsSrc.dwYOffset[MHW_U_PLANE] =
        (m_destSurface.UPlaneOffset.iYOffset % MOS_YTILE_H_ALIGNMENT);

#ifdef _MMC_SUPPORTED
    if (m_mmc)
    {
        CODECHAL_SURFACE_CODEC_PARAMS srcSurfaceParam = {};
        srcSurfaceParam.psSurface = &m_destSurface;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetSurfaceParams(&srcSurfaceParam));
    }
#endif

    MHW_RCS_SURFACE_PARAMS surfaceParamsDst;
    MOS_ZeroMemory(&surfaceParamsDst, sizeof(surfaceParamsDst));
    surfaceParamsDst = surfaceParamsSrc;
    surfaceParamsDst.bIsWritable = true;
    surfaceParamsDst.psSurface                         = &m_deblockSurface;
    surfaceParamsDst.psSurface->dwDepth = 1;    // depth needs to be 0 for codec 2D surface
    surfaceParamsDst.dwBindingTableOffset[MHW_Y_PLANE] = CODECHAL_DECODE_VC1_OLP_DST_Y;
    surfaceParamsDst.dwBindingTableOffset[MHW_U_PLANE] = CODECHAL_DECODE_VC1_OLP_DST_UV;

#ifdef _MMC_SUPPORTED
    if (m_mmc)
    {
        CODECHAL_SURFACE_CODEC_PARAMS dstSurfaceParam = {};
        dstSurfaceParam.psSurface = &m_deblockSurface;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetSurfaceParams(&dstSurfaceParam));
    }
#endif

    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetSurfaceState(
        stateHeapInterface,
        kernelState,
        &cmdBuffer,
        1,
        &surfaceParamsSrc));
    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetSurfaceState(
        stateHeapInterface,
        kernelState,
        &cmdBuffer,
        1,
        &surfaceParamsDst));

    MHW_STATE_BASE_ADDR_PARAMS stateBaseAddrParams;
    MOS_ZeroMemory(&stateBaseAddrParams, sizeof(stateBaseAddrParams));
    MOS_RESOURCE *dsh = nullptr, *ish = nullptr;
    CODECHAL_DECODE_CHK_NULL_RETURN(dsh = kernelState->m_dshRegion.GetResource());
    CODECHAL_DECODE_CHK_NULL_RETURN(ish = kernelState->m_ishRegion.GetResource());
    stateBaseAddrParams.presDynamicState = dsh;
    stateBaseAddrParams.dwDynamicStateSize = kernelState->m_dshRegion.GetHeapSize();
    stateBaseAddrParams.presInstructionBuffer = ish;
    stateBaseAddrParams.dwInstructionBufferSize = kernelState->m_ishRegion.GetHeapSize();
    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddStateBaseAddrCmd(&cmdBuffer, &stateBaseAddrParams));

    MHW_VFE_PARAMS_G12 vfeParams= {};
    vfeParams.pKernelState = kernelState;
    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaVfeCmd(&cmdBuffer, &vfeParams));

    MHW_CURBE_LOAD_PARAMS curbeLoadParams;
    MOS_ZeroMemory(&curbeLoadParams, sizeof(curbeLoadParams));
    curbeLoadParams.pKernelState = kernelState;
    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaCurbeLoadCmd(&cmdBuffer, &curbeLoadParams));

    MHW_ID_LOAD_PARAMS idLoadParams;
    MOS_ZeroMemory(&idLoadParams, sizeof(idLoadParams));
    idLoadParams.pKernelState = kernelState;
    idLoadParams.dwNumKernelsLoaded = 1;
    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaIDLoadCmd(&cmdBuffer, &idLoadParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            CODECHAL_MEDIA_STATE_OLP,
            MHW_DSH_TYPE,
            kernelState));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
        CODECHAL_MEDIA_STATE_OLP,
        MHW_SSH_TYPE,
        kernelState));
    )

    CODECHAL_DECODE_VC1_OLP_PARAMS vc1OlpParams;
    vc1OlpParams.pCmdBuffer = &cmdBuffer;
    vc1OlpParams.pPipeControlParams = &pipeControlParams;
    vc1OlpParams.pStateBaseAddrParams = &stateBaseAddrParams;
    vc1OlpParams.pVfeParams = &vfeParams;
    vc1OlpParams.pCurbeLoadParams = &curbeLoadParams;
    vc1OlpParams.pIdLoadParams = &idLoadParams;
    CODECHAL_DECODE_CHK_STATUS_RETURN(AddVc1OlpCmd(&vc1OlpParams));

    // Check if destination surface needs to be synchronized, before command buffer submission
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_renderContext;
    syncParams.presSyncResource         = &m_deblockSurface.OsResource;
    syncParams.bReadOnly = false;
    syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

    // Update the resource tag (s/w tag) for On-Demand Sync
    m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

    // Update GPU Sync tag for on demand synchronization
    if (m_osInterface->bTagResourceSync)
    {
        pipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(&cmdBuffer, nullptr, &pipeControlParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
    }
    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSubmitBlocks(
        stateHeapInterface,
        kernelState));
    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
        stateHeapInterface));

    // Add PipeControl to invalidate ISP and MediaState to avoid PageFault issue
    // This code is temporal and it will be moved to batch buffer end in short
    if (GFX_IS_GEN_9_OR_LATER(m_hwInterface->GetPlatform()))
    {
        MHW_PIPE_CONTROL_PARAMS pipeControlParams;

        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
        pipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
        pipeControlParams.bGenericMediaStateClear = true;
        pipeControlParams.bIndirectStatePointersDisable = true;
        pipeControlParams.bDisableCSStall = false;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(&cmdBuffer, nullptr, &pipeControlParams));

    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    // To clear the SSEU values in the hw interface struct, so next kernel can be programmed correctly
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, false, true));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_MEDIA_STATE_OLP,
            "_DEC"));
    )

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        CodecHalDecodeSinglePipeVE_PopulateHintParams(m_veState, &cmdBuffer, false);
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_renderContextUsesNullHw));
    }

    m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext);

    return eStatus;
}

CodechalDecodeVc1G12::CodechalDecodeVc1G12(
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecodeVc1(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    Mos_CheckVirtualEngineSupported(m_osInterface, true, true);

    m_olpCurbeStaticDataLength = CODECHAL_DECODE_VC1_CURBE_SIZE_OLP;

    uint32_t kuid = 0;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    eStatus = CodecHalGetKernelBinaryAndSize(
        (uint8_t *)IGCODECKRN_G12,
        IDR_CODEC_AllVC1_NV12,
        &m_olpKernelBase,
        &m_olpKernelSize);
#endif

    CODECHAL_DECODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_DECODE_VC1_NUM_SYNC_TAGS;
    hwInterface->GetStateHeapSettings()->dwIshSize =
        MOS_ALIGN_CEIL(m_olpKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));
    hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_DECODE_VC1_INITIAL_DSH_SIZE;
}

CodechalDecodeVc1G12::~CodechalDecodeVc1G12()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_veState != nullptr)
    {
        MOS_FreeMemAndSetNull(m_veState);
        m_veState = nullptr;
    }
}
