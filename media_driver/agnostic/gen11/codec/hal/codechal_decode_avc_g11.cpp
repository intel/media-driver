/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file      codechal_decode_avc_g11.cpp
//! \brief     This modules implements Render interface layer for AVC decoding to be used on all operating systems/DDIs, across CODECHAL components.
//!

#include "codechal_decoder.h"
#include "codechal_decode_avc_g11.h"
#include "codechal_secure_decode_interface.h"
#include "mhw_vdbox_mfx_g11_X.h"
#include "hal_oca_interface.h"

MOS_STATUS CodechalDecodeAvcG11::AllocateStandard(
    CodechalSetting *          settings)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeAvc::AllocateStandard(settings));

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        static_cast<MhwVdboxMfxInterfaceG11*>(m_mfxInterface)->DisableScalabilitySupport();

        //single pipe VE initialize
        m_veState = (PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE));
        CODECHAL_DECODE_CHK_NULL_RETURN(m_veState);
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_InitInterface(m_osInterface, m_veState));
    }

    return eStatus;
}

CodechalDecodeAvcG11::~CodechalDecodeAvcG11()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_veState)
    {
        MOS_FreeMemAndSetNull(m_veState);
        m_veState = nullptr;
    }

    return;
}

MOS_STATUS CodechalDecodeAvcG11::SetGpuCtxCreatOption(
    CodechalSetting *          codecHalSetting)
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

        bool sfcInUse = IsSfcInUse(codecHalSetting);

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_ConstructParmsForGpuCtxCreation(    
            m_veState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt,
            sfcInUse));

        if (sfcInUse)
        {
            m_videoContext = MOS_GPU_CONTEXT_VIDEO4;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                m_osInterface,
                m_videoContext,
                MOS_GPU_NODE_VIDEO,
                m_gpuCtxCreatOpt));

            MOS_GPUCTX_CREATOPTIONS createOption;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
                m_osInterface,
                MOS_GPU_CONTEXT_VIDEO,
                MOS_GPU_NODE_VIDEO,
                &createOption));
        }
        else
        {
            m_videoContext = MOS_GPU_CONTEXT_VIDEO;
        }
    }
        
    return eStatus;
}

MOS_STATUS CodechalDecodeAvcG11::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeAvc::SetFrameStates());

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
        {
            MOS_VIRTUALENGINE_SET_PARAMS vesetParams;

            MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
#ifdef _DECODE_PROCESSING_SUPPORTED
            vesetParams.bSFCInUse = m_sfcState->m_sfcPipeOut;
#else
            vesetParams.bSFCInUse                   = false;
#endif
            vesetParams.bNeedSyncWithPrevious       = true;
            vesetParams.bSameEngineAsLastSubmission = false;
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_SetHintParams(m_veState, &vesetParams));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeAvcG11::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->Execute(this));
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    PIC_MHW_PARAMS picMhwParams;
    CODECHAL_DECODE_CHK_STATUS_RETURN(InitPicMhwParams(&picMhwParams));

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);
    HalOcaInterface::On1stLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);

    if (m_cencBuf && m_cencBuf->checkStatusRequired)
    {
        CODECHAL_DECODE_COND_ASSERTMESSAGE((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");

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

MOS_STATUS CodechalDecodeAvcG11::DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_DECODE_PROCESSING_PARAMS *decProcessingParams = nullptr;

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

#ifdef _DECODE_PROCESSING_SUPPORTED
    decProcessingParams = (CODECHAL_DECODE_PROCESSING_PARAMS *)m_decodeParams.m_procParams;
    if (decProcessingParams != nullptr && decProcessingParams->bIsReferenceOnlyPattern)
    {
        HucCopy(&cmdBuffer, 
            &m_destSurface.OsResource, 
            &decProcessingParams->pOutputSurface->OsResource, 
            decProcessingParams->pOutputSurface->dwSize,
            m_destSurface.dwOffset,
            decProcessingParams->pOutputSurface->dwOffset
        );
    }
#endif

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

            decodeStatusReport.m_secondField = CodecHal_PictureIsBottomField(m_avcPicParams->CurrPic);
            decodeStatusReport.m_frameType   = m_perfType;)

        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(decodeStatusReport, &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

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
            "_DEC"));

    //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
    //    m_debugInterface,
    //    &cmdBuffer));
    )

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        CodecHalDecodeSinglePipeVE_PopulateHintParams(m_veState, &cmdBuffer, true);
    }

    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);)
#ifdef _DECODE_PROCESSING_SUPPORTED
    decProcessingParams = (CODECHAL_DECODE_PROCESSING_PARAMS *)m_decodeParams.m_procParams;
    if (decProcessingParams != nullptr && !m_sfcState->m_sfcPipeOut && (m_isSecondField || m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag))
    {
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
            syncParams.presSyncResource = &decProcessingParams->pOutputSurface->OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
        }
#endif
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DEBUG_TOOL(
        // Dump out downsampling result
        if (decProcessingParams && decProcessingParams->pOutputSurface)
        {
            MOS_SURFACE dstSurface;
            MOS_ZeroMemory(&dstSurface, sizeof(dstSurface));
            dstSurface.Format = Format_NV12;
            dstSurface.OsResource = decProcessingParams->pOutputSurface->OsResource;

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

CodechalDecodeAvcG11::CodechalDecodeAvcG11(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalDecodeAvc(hwInterface, debugInterface, standardInfo),
                                            m_veState(nullptr)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    Mos_CheckVirtualEngineSupported(m_osInterface, true, true);
};

