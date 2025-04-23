/*
* Copyright (c) 2017-2021, Intel Corporation
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
//! \file      codechal_decode_avc_g12.cpp
//! \brief     This modules implements Render interface layer for AVC decoding to be used on all operating systems/DDIs, across CODECHAL components.
//!

#include "codechal_decoder.h"
#include "codechal_decode_avc_g12.h"
#include "codechal_secure_decode_interface.h"
#include "codechal_decode_histogram.h"
#include "mhw_vdbox_mfx_g12_X.h"
#include "codechal_mmc_decode_avc_g12.h"
#include "hal_oca_interface.h"
#include "mos_os_cp_interface_specific.h"

MOS_STATUS CodechalDecodeAvcG12::AllocateStandard(
    CodechalSetting *          settings)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeAvc::AllocateStandard(settings));
#ifdef _MMC_SUPPORTED
    // To WA invalid aux data caused HW issue when MMC on
    // Add disable Clear CCS WA due to green corruption issue
    if (m_mmc->IsMmcEnabled())
    {
        if (MEDIA_IS_WA(m_waTable, Wa_1408785368) || MEDIA_IS_WA(m_waTable, Wa_22010493002) && (!MEDIA_IS_WA(m_waTable, WaDisableClearCCS)))
        {
            //Add HUC STATE Commands
            MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

            m_hwInterface->GetHucStateCommandSize(
                CODECHAL_DECODE_MODE_AVCVLD,
                &m_HucStateCmdBufferSizeNeeded,
                &m_HucPatchListSizeNeeded,
                &stateCmdSizeParams);
        }
    }
#endif

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

MOS_STATUS CodechalDecodeAvcG12::InitSfcState()
{
#ifdef _DECODE_PROCESSING_SUPPORTED
    m_sfcState = MOS_New(CodechalAvcSfcStateG12);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_sfcState);
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_sfcState->InitializeSfcState(
        this,
        m_hwInterface,
        m_osInterface));
#endif
    return MOS_STATUS_SUCCESS;
}

CodechalDecodeAvcG12::~CodechalDecodeAvcG12()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_veState)
    {
        MOS_FreeMemAndSetNull(m_veState);
        m_veState = nullptr;
    }

    if (m_histogramSurface)
    {
        if (!Mos_ResourceIsNull(&m_histogramSurface->OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_histogramSurface->OsResource);
        }
        
        MOS_FreeMemory(m_histogramSurface);
        m_histogramSurface = nullptr;
    }

    return;
}

MOS_STATUS CodechalDecodeAvcG12::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeAvcG12, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif

    if (m_osInterface->pfnIsMismatchOrderProgrammingSupported())
    {
        m_mmc->SetMmcDisabled();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeAvcG12::SetGpuCtxCreatOption(
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

MOS_STATUS CodechalDecodeAvcG12::AllocateHistogramSurface()
{
    MOS_ALLOC_GFXRES_PARAMS allocParams;

    if (m_histogramSurface == nullptr)
    {
        m_histogramSurface = (MOS_SURFACE*)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE));
        CODECHAL_DECODE_CHK_NULL_RETURN(m_histogramSurface);

        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type = MOS_GFXRES_BUFFER;
        allocParams.TileType = MOS_TILE_LINEAR;
        allocParams.Format = Format_Buffer;
        allocParams.dwBytes = 256 * 4;
        allocParams.pBufName = "HistogramStreamOut";

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParams,
            &m_histogramSurface->OsResource));

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface,
            m_histogramSurface));
    }

    if (m_decodeHistogram)
        m_decodeHistogram->SetSrcHistogramSurface(m_histogramSurface);

    return MOS_STATUS_SUCCESS;
}

void CodechalDecodeAvcG12::CalcRequestedSpace(
    uint32_t &requestedSize,
    uint32_t &additionalSizeNeeded,
    uint32_t &requestedPatchListSize) 
{  
    CODECHAL_DECODE_FUNCTION_ENTER;

    requestedSize          = m_commandBufferSizeNeeded + m_HucStateCmdBufferSizeNeeded +
                    (m_standardDecodeSizeNeeded * (m_decodeParams.m_numSlices + 1));
    requestedPatchListSize = m_commandPatchListSizeNeeded + m_HucPatchListSizeNeeded +
                             (m_standardDecodePatchListSizeNeeded * (m_decodeParams.m_numSlices + 1));
    additionalSizeNeeded = COMMAND_BUFFER_RESERVED_SPACE;

}

MOS_STATUS CodechalDecodeAvcG12::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_secureDecoder)
    {
        m_secureDecoder->EnableSampleGroupConstantIV();
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_decodeParams.m_procParams)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateHistogramSurface());

        ((DecodeProcessingParams*)m_decodeParams.m_procParams)->m_histogramSurface = m_histogramSurface;

        if (m_decodeHistogram)
            m_decodeHistogram->SetSrcHistogramSurface(m_histogramSurface);

    }
#endif

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeAvc::SetFrameStates());

    CODECHAL_DECODE_CHK_STATUS_RETURN(ErrorDetectAndConceal());

    if (MOS_VE_SUPPORTED(m_osInterface) && !MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        MOS_VIRTUALENGINE_SET_PARAMS vesetParams;

        MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
#ifdef _DECODE_PROCESSING_SUPPORTED
        vesetParams.bSFCInUse = m_sfcState->m_sfcPipeOut;
#else
        vesetParams.bSFCInUse                   = false;
#endif
        vesetParams.bNeedSyncWithPrevious = true;
        vesetParams.bSameEngineAsLastSubmission = false;
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_SetHintParams(m_veState, &vesetParams));
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeAvcG12::ErrorDetectAndConceal()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

#ifdef _DECODE_PROCESSING_SUPPORTED
    // Skip check for sfc downsampling cases.
    if (m_sfcState->m_sfcPipeOut  == true)
    {
        return eStatus;
    }
#endif

    if ((uint32_t)(m_destSurface.dwWidth * m_destSurface.dwHeight) < (m_width * m_height))
    {
        // Return an error when the output size is insufficient for AVC decoding
        CODECHAL_DECODE_ASSERTMESSAGE("Incorrect decode output allocation.")
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeAvcG12::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

#ifdef _MMC_SUPPORTED
    // To WA invalid aux data caused HW issue when MMC on
    // Add disable Clear CCS WA due to green corruption issue
    if (m_mmc->IsMmcEnabled() && m_decodeParams.m_destSurface && !Mos_ResourceIsNull(&m_decodeParams.m_destSurface->OsResource) &&
        m_decodeParams.m_destSurface->OsResource.bConvertedFromDDIResource)
    {
        if (MEDIA_IS_WA(m_waTable, Wa_1408785368) || MEDIA_IS_WA(m_waTable, Wa_22010493002) && (!MEDIA_IS_WA(m_waTable, WaDisableClearCCS)))
        {
            if (m_secureDecoder && m_secureDecoder->IsAuxDataInvalid(&m_decodeParams.m_destSurface->OsResource))
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->InitAuxSurface(&m_decodeParams.m_destSurface->OsResource, false, true));
            }
            else
            {
                CODECHAL_DECODE_VERBOSEMESSAGE("Clear CCS by VE resolve before frame %d submission", m_frameNum);
                CODECHAL_DECODE_CHK_STATUS_RETURN(static_cast<CodecHalMmcStateG12 *>(m_mmc)->ClearAuxSurf(
                    this, m_miInterface, &m_decodeParams.m_destSurface->OsResource, m_veState));
            }
        }

    }
#endif

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->Execute(this));
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

    HalOcaInterface::On1stLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    HalOcaInterface::OnDispatch(cmdBuffer, *m_osInterface, *m_miInterface, *m_miInterface->GetMmioRegisters());
    
    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl = true;
    forceWakeupParams.bMFXPowerWellControlMask = true;
    forceWakeupParams.bHEVCPowerWellControl = false;
    forceWakeupParams.bHEVCPowerWellControlMask = true;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
        &cmdBuffer,
        &forceWakeupParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    PIC_MHW_PARAMS picMhwParams;
    CODECHAL_DECODE_CHK_STATUS_RETURN(InitPicMhwParams(&picMhwParams));

    if (m_cencBuf && m_cencBuf->checkStatusRequired)
    {
        CODECHAL_DECODE_COND_ASSERTMESSAGE((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
        auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetCpInterface()->CheckStatusReportNum(
            mmioRegisters,
            m_cencBuf->bufIdx,
            m_cencBuf->resStatus,
            &cmdBuffer));
    }

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AddPictureCmds(&cmdBuffer, &picMhwParams));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeAvcG12::DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_avcPicParams);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (m_cencBuf)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SetCencBatchBuffer(&cmdBuffer));
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseSlice(&cmdBuffer));
    }

    // Check if destination surface needs to be synchronized
    MOS_SYNC_PARAMS syncParams;
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource         = &m_destSurface.OsResource;
    syncParams.bReadOnly = false;
    syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    if (!CodecHal_PictureIsField(m_avcPicParams->CurrPic) ||
        !m_isSecondField)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

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
        (!CodecHal_PictureIsField(m_avcPicParams->CurrPic) || m_isSecondField))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
    }

    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;

        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_currDecodedPic     = m_avcPicParams->CurrPic;
        decodeStatusReport.m_currDeblockedPic   = m_avcPicParams->CurrPic;
        decodeStatusReport.m_codecStatus = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_currDecodedPicRes  = m_avcRefList[m_avcPicParams->CurrPic.FrameIdx]->resRefPic;

        CODECHAL_DEBUG_TOOL(
            if (m_streamOutEnabled) {
                // add current streamout buffer to the report and move onto the next one
                decodeStatusReport.m_streamOutBuf = &(m_streamOutBuffer[m_streamOutCurrBufIdx]);
                decodeStatusReport.m_streamoutIdx = m_streamOutCurrBufIdx;
                if (++m_streamOutCurrBufIdx >= CODECHAL_DECODE_NUM_STREAM_OUT_BUFFERS)
                {
                    m_streamOutCurrBufIdx = 0;
                }
                // check next buffer in the list is free.
                if (m_streamOutCurrStatusIdx[m_streamOutCurrBufIdx] != CODECHAL_DECODE_STATUS_NUM)
                {
                    // We've run out of buffers. Temporarily lock the next one down to force a wait. Then mark it as free.
                    CodechalResLock ResourceLock(m_osInterface, &(m_streamOutBuffer[m_streamOutCurrBufIdx]));
                    ResourceLock.Lock(CodechalResLock::readOnly);

                    m_streamOutCurrStatusIdx[m_streamOutCurrBufIdx] = CODECHAL_DECODE_STATUS_NUM;
                }
            }

            decodeStatusReport.m_secondField = m_secondField;
            decodeStatusReport.m_frameType   = m_perfType;)

        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(decodeStatusReport, &cmdBuffer));
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (CodecHal_PictureIsFrame(m_crrPic) ||
        CodecHal_PictureIsInterlacedFrame(m_crrPic) ||
        m_secondField)
    {
        uint32_t curIdx = (GetDecodeStatusBuf()->m_currIndex + CODECHAL_DECODE_STATUS_NUM - 1) % CODECHAL_DECODE_STATUS_NUM;
        uint32_t frameCrcOffset = curIdx * sizeof(CodechalDecodeStatus) + GetDecodeStatusBuf()->m_decFrameCrcOffset + sizeof(uint32_t) * 2;
        std::vector<MOS_RESOURCE> vSemaResource{GetDecodeStatusBuf()->m_statusBuffer};
        m_debugInterface->DetectCorruptionHw(m_hwInterface, &m_frameCountTypeBuf, curIdx, frameCrcOffset, vSemaResource, &cmdBuffer, m_frameNum);
    }
#endif
    if (!m_osInterface->pfnIsMismatchOrderProgrammingSupported())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    bool syncCompleteFrame = (m_avcPicParams->seq_fields.chroma_format_idc == avcChromaFormatMono && !m_hwInterface->m_noHuC);
    if (syncCompleteFrame)
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_AVC_DECODE_0"));

    //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
    //    m_debugInterface,
    //    &cmdBuffer));
    )

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        CodecHalDecodeSinglePipeVE_PopulateHintParams(m_veState, &cmdBuffer, true);
    }

    HalOcaInterface::DumpCodechalParam(cmdBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, m_pCodechalOcaDumper, CODECHAL_AVC);
    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);)
#ifdef _DECODE_PROCESSING_SUPPORTED
    auto decProcessingParams = (DecodeProcessingParams *)m_decodeParams.m_procParams;
    if (decProcessingParams != nullptr && !m_sfcState->m_sfcPipeOut && (m_isSecondField || m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag))
    {
#ifdef _MMC_SUPPORTED
        // To Clear invalid aux data of output surface when MMC on
        if (m_mmc && m_mmc->IsMmcEnabled() &&
            !Mos_ResourceIsNull(&decProcessingParams->m_outputSurface->OsResource) &&
            decProcessingParams->m_outputSurface->OsResource.bConvertedFromDDIResource)
        {
            CODECHAL_DECODE_VERBOSEMESSAGE("Clear invalid aux data of output surface before frame %d submission", m_frameNum);
            CODECHAL_DECODE_CHK_STATUS_RETURN(static_cast<CodecHalMmcStateG12 *>(m_mmc)->ClearAuxSurf(
                this, m_miInterface, &decProcessingParams->m_outputSurface->OsResource, m_veState));
        }
#endif
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_fieldScalingInterface->DoFieldScaling(
            decProcessingParams,
            m_renderContext,
            m_disableDecodeSyncLock,
            m_disableLockForTranscode));
    }
    else
#endif
    {
        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_videoContextUsesNullHw));
        }
    }

    // Needs to be re-set for Linux buffer re-use scenarios
    m_avcRefList[m_avcPicParams->CurrPic.FrameIdx]->resRefPic =
        m_destSurface.OsResource;

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    if (!CodecHal_PictureIsField(m_avcPicParams->CurrPic) || m_isSecondField)
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_destSurface.OsResource;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
#ifdef _DECODE_PROCESSING_SUPPORTED
        if (decProcessingParams && !m_sfcState->m_sfcPipeOut)
        {
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_renderContext;
            syncParams.presSyncResource = &decProcessingParams->m_outputSurface->OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
        }
#endif
    }
#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DEBUG_TOOL(
        // Dump out downsampling result
        if (decProcessingParams && decProcessingParams->m_outputSurface)
        {
            MOS_SURFACE dstSurface;
            MOS_ZeroMemory(&dstSurface, sizeof(dstSurface));
            dstSurface.Format = Format_NV12;
            dstSurface.OsResource = decProcessingParams->m_outputSurface->OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                m_osInterface,
                &dstSurface));

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &dstSurface,
                CodechalDbgAttr::attrSfcOutputSurface,
                "SfcDstSurf"));
        }
    )
#endif
        return eStatus;
}

CodechalDecodeAvcG12::CodechalDecodeAvcG12(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalDecodeAvc(hwInterface, debugInterface, standardInfo),
                                            m_veState(nullptr)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    m_osInterface->pfnVirtualEngineSupported(m_osInterface, true, true);
}

MOS_STATUS CodechalDecodeAvcG12::FormatAvcMonoPicture(PMOS_SURFACE surface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    PCODEC_AVC_PIC_PARAMS picParams = (PCODEC_AVC_PIC_PARAMS)m_avcPicParams;
    if (picParams->seq_fields.chroma_format_idc != avcChromaFormatMono)
    {
        CODECHAL_DECODE_VERBOSEMESSAGE("None mono case");
        return MOS_STATUS_SUCCESS;
    }

    if (surface == nullptr || Mos_ResourceIsNull(&surface->OsResource))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Surface pointer is NULL!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

#ifdef _MMC_SUPPORTED
    // Initialize the UV aux data of protected surfaces before HucCopy of UV plane
    if (m_mmc && m_mmc->IsMmcEnabled() && !MEDIA_IS_WA(m_waTable, Wa_1408785368) &&
        m_secureDecoder && m_osInterface->osCpInterface->IsHMEnabled())
    {
        CODECHAL_DECODE_VERBOSEMESSAGE("Initialize the UV aux data for %d submission", m_frameNum);
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->InitAuxSurface(&surface->OsResource, true, true));
    }
#endif

    return CodechalDecodeAvc::FormatAvcMonoPicture(surface);
}
