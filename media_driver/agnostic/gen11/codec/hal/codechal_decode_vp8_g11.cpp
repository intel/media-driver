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
//! \file     codechal_decode_vp8_g11.cpp
//! \brief    Implements the decode interface extension for VP8.
//! \details  Implements all functions and constants required by CodecHal for VP8 decoding.
//!

#include "codechal_decoder.h"
#include "codechal_decode_vp8_g11.h"
#include "mhw_vdbox_mfx_g11_X.h"

CodechalDecodeVp8G11::~CodechalDecodeVp8G11()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_veState != nullptr)
    {
        MOS_FreeMemAndSetNull(m_veState);
        m_veState = nullptr;
    }
}

MOS_STATUS CodechalDecodeVp8G11::SetGpuCtxCreatOption(
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

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_ConstructParmsForGpuCtxCreation(    
            m_veState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt,
            false));
    }
        
    return eStatus;
}

MOS_STATUS CodechalDecodeVp8G11::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVp8::SetFrameStates());

    if ( MOS_VE_SUPPORTED(m_osInterface))
    {
        if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
        {
            MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;

            MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
            vesetParams.bSFCInUse = false;
            vesetParams.bNeedSyncWithPrevious = true;
            vesetParams.bSameEngineAsLastSubmission = false;
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalDecodeSinglePipeVE_SetHintParams(m_veState, &vesetParams));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVp8G11::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODEC_REF_LIST *vp8RefList = m_vp8RefList;

    uint8_t lastRefPicIndex   = m_vp8PicParams->ucLastRefPicIndex;
    uint8_t goldenRefPicIndex = m_vp8PicParams->ucGoldenRefPicIndex;
    uint8_t altRefPicIndex    = m_vp8PicParams->ucAltRefPicIndex;

    PMOS_SURFACE destSurface = &m_destSurface;
    if (m_vp8PicParams->key_frame)  // reference surface should be nullptr when key_frame == true
    {
        m_presLastRefSurface   = nullptr;
        m_presGoldenRefSurface = nullptr;
        m_presAltRefSurface    = nullptr;
    }
    else
    {
        m_presLastRefSurface   = &(vp8RefList[lastRefPicIndex]->resRefPic);
        m_presGoldenRefSurface = &(vp8RefList[goldenRefPicIndex]->resRefPic);
        m_presAltRefSurface    = &(vp8RefList[altRefPicIndex]->resRefPic);
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode               = m_mode;
    pipeModeSelectParams.bStreamOutEnabled  = m_streamOutEnabled;
    pipeModeSelectParams.bPostDeblockOutEnable = m_deblockingEnabled;
    pipeModeSelectParams.bPreDeblockOutEnable  = !m_deblockingEnabled;
    pipeModeSelectParams.bShortFormatInUse     = m_shortFormatInUse;

    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode = m_mode;
    surfaceParams.psSurface = destSurface;

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams.Mode = m_mode;
    // MMC is only enabled for frame decoding and no interlaced support in VP8
    // So no need to check frame/field type here.
    if (m_deblockingEnabled)
    {
        pipeBufAddrParams.psPostDeblockSurface = destSurface;
    }
    else
    {
        pipeBufAddrParams.psPreDeblockSurface = destSurface;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(&pipeBufAddrParams));

    // when there is no last, golden and alternate reference,
    // the index is set to the destination frame index
    pipeBufAddrParams.presReferences[CodechalDecodeLastRef]      = m_presLastRefSurface;
    pipeBufAddrParams.presReferences[CodechalDecodeGoldenRef]    = m_presGoldenRefSurface;
    pipeBufAddrParams.presReferences[CodechalDecodeAlternateRef] = m_presAltRefSurface;

    pipeBufAddrParams.presMfdIntraRowStoreScratchBuffer            = &m_resMfdIntraRowStoreScratchBuffer;
    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer = &m_resMfdDeblockingFilterRowStoreScratchBuffer;
    if (m_streamOutEnabled)
    {
        pipeBufAddrParams.presStreamOutBuffer =
            &(m_streamOutBuffer[m_streamOutCurrBufIdx]);
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->CheckReferenceList(&pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetRefrenceSync(m_disableDecodeSyncLock, m_disableLockForTranscode));

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = m_mode;
    indObjBaseAddrParams.dwDataSize     = m_dataSize;
    indObjBaseAddrParams.presDataBuffer = &m_resDataBuffer;

    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS bspBufBaseAddrParams;
    MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
    bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resBsdMpcRowStoreScratchBuffer;
    bspBufBaseAddrParams.presMprRowStoreScratchBuffer    = &m_resMprRowStoreScratchBuffer;

    MHW_VDBOX_VP8_PIC_STATE vp8PicState;
    vp8PicState.pVp8PicParams                  = m_vp8PicParams;
    vp8PicState.pVp8IqMatrixParams             = m_vp8IqMatrixParams;
    vp8PicState.presSegmentationIdStreamBuffer = &m_resSegmentationIdStreamBuffer;
    vp8PicState.dwCoefProbTableOffset = 0;
    vp8PicState.presCoefProbBuffer             = &m_resCoefProbBuffer;

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(&cmdBuffer, &bspBufBaseAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxVp8PicCmd(&cmdBuffer, &vp8PicState));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeVp8G11::DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // Fill BSD Object Commands
    MHW_VDBOX_VP8_BSD_PARAMS vp8BsdParams;
    vp8BsdParams.pVp8PicParams = m_vp8PicParams;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVp8BsdObjectCmd(&cmdBuffer, &vp8BsdParams));

    // Check if destination surface needs to be synchronized
    MOS_SYNC_PARAMS syncParams;
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource         = &m_destSurface.OsResource;
    syncParams.bReadOnly = false;
    syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

    // Update the resource tag (s/w tag) for On-Demand Sync
    m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
    if (m_osInterface->bTagResourceSync)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
    }

    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;

        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_currDecodedPic     = m_vp8PicParams->CurrPic;
        decodeStatusReport.m_currDeblockedPic   = m_vp8PicParams->CurrPic;
        decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_currDecodedPicRes  = m_vp8RefList[m_vp8PicParams->CurrPic.FrameIdx]->resRefPic;
        CODECHAL_DEBUG_TOOL(
            decodeStatusReport.m_secondField = CodecHal_PictureIsBottomField(m_vp8PicParams->CurrPic);
            decodeStatusReport.m_frameType   = m_perfType;)
        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(decodeStatusReport, &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));
    )

        if ( MOS_VE_SUPPORTED(m_osInterface))
        {
            CodecHalDecodeSinglePipeVE_PopulateHintParams(m_veState, &cmdBuffer, true);
        }

        if (m_huCCopyInUse)
        {
            syncParams                  = g_cInitSyncParams;
            syncParams.GpuContext       = m_videoContextForWa;
            syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

            syncParams                  = g_cInitSyncParams;
            syncParams.GpuContext       = m_videoContext;
            syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

            m_huCCopyInUse = false;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);)

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_videoContextUsesNullHw));
        }

    // Needs to be re-set for Linux buffer re-use scenarios
        m_vp8RefList[m_vp8PicParams->ucCurrPicIndex]->resRefPic =
            m_destSurface.OsResource;

        // Send the signal to indicate decode completion, in case On-Demand Sync is not present
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));

        return eStatus;
}

MOS_STATUS CodechalDecodeVp8G11::AllocateStandard(
    CodechalSetting *          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVp8::AllocateStandard(settings));

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

CodechalDecodeVp8G11::CodechalDecodeVp8G11(
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecodeVp8(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    Mos_CheckVirtualEngineSupported(m_osInterface, true, true);
}

