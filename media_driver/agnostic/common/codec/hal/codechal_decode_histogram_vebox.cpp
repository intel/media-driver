/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_decode_histogram_vebox.cpp
//! \brief    implements the decode histogram through vebox.
//! \details  decode histogram through vebox.
//!
#include "codechal_decode_histogram_vebox.h"
#include "mos_utilities.h"

CodechalDecodeHistogramVebox::CodechalDecodeHistogramVebox(
    CodechalHwInterface *hwInterface,
    MOS_INTERFACE *osInterface):
    CodechalDecodeHistogram(hwInterface, osInterface),
    m_veboxInterface(hwInterface->GetVeboxInterface())
{
    MOS_ZeroMemory(&m_resSyncObject, sizeof(m_resSyncObject));
    MOS_ZeroMemory(&m_resStatisticsOutput, sizeof(m_resStatisticsOutput));
    MOS_ZeroMemory(&m_outputSurface, sizeof(m_outputSurface));
    // allocate heap
    m_veboxInterface->CreateHeap();

    // create Vebox context
    MOS_GPUCTX_CREATOPTIONS  createOpts;
    m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        MOS_GPU_CONTEXT_VEBOX,
        MOS_GPU_NODE_VE,
        &createOpts);

    // register Vebox GPU context with the Batch Buffer completion event
    m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        MOS_GPU_CONTEXT_VEBOX);
}

CodechalDecodeHistogramVebox::~CodechalDecodeHistogramVebox()
{
    if (!Mos_ResourceIsNull(&m_resSyncObject))
    {
        m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObject);
    }
    if (!Mos_ResourceIsNull(&m_resStatisticsOutput))
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resStatisticsOutput);
    }
    if (!Mos_ResourceIsNull(&m_outputSurface.OsResource))
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_outputSurface.OsResource);
    }
}

MOS_STATUS CodechalDecodeHistogramVebox::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    // allocate sync object
    if (Mos_ResourceIsNull(&m_resSyncObject))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
            m_osInterface, &m_resSyncObject));
    }

    uint32_t size = 0;
    // allocate internal histogram resource
    if (Mos_ResourceIsNull(&m_resHistogram) ||
        m_preWidth != m_inputSurface->dwWidth ||
        m_preHeight != m_inputSurface->dwHeight)
    {
        // Need to reallocate
        if (m_preWidth != m_inputSurface->dwWidth ||
            m_preHeight != m_inputSurface->dwHeight)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_resHistogram);
        }

        m_hwInterface->GetHcpInterface()->GetOsResLaceOrAceOrRgbHistogramBufferSize(
            m_inputSurface->dwWidth,
            m_inputSurface->dwHeight,
            &size);

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = size;
        allocParamsForBufferLinear.pBufName = "ResLaceOrAceOrRgbHistogram";

        eStatus = m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resHistogram);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate histogram buffer.");
            return eStatus;
        }
    }

    // allocate statistics output resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&m_resStatisticsOutput) ||
        m_preWidth != m_inputSurface->dwWidth ||
        m_preHeight != m_inputSurface->dwHeight)
    {
        // Need to reallocate
        if (m_preWidth != m_inputSurface->dwWidth ||
            m_preHeight != m_inputSurface->dwHeight)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_resStatisticsOutput);
        }

        m_hwInterface->GetHcpInterface()->GetOsResStatisticsOutputBufferSize(
            m_inputSurface->dwWidth,
            m_inputSurface->dwHeight,
            &size);

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = size;
        allocParamsForBufferLinear.pBufName = "m_resStatisticsOutput";

        eStatus = m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resStatisticsOutput);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate statistics output buffer.");
            return eStatus;
        }
    }

    // allocate vebox output surface
    if (Mos_ResourceIsNull(&m_outputSurface.OsResource) ||
        m_preWidth != m_inputSurface->dwWidth ||
        m_preHeight != m_inputSurface->dwHeight)
    {
        // Need to reallocate
        if (m_preWidth != m_inputSurface->dwWidth ||
            m_preHeight != m_inputSurface->dwHeight)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_outputSurface.OsResource);
        }

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(m_decoder->AllocateSurface(
                                                      &m_outputSurface,
                                                      m_inputSurface->dwWidth,
                                                      m_inputSurface->dwHeight,
                                                      "VeboxOutputBuffer"),
            "Failed to allocate vebox output surface buffer.");
    }

    m_preWidth  = m_inputSurface->dwWidth;
    m_preHeight = m_inputSurface->dwHeight;

    return eStatus;
}

MOS_STATUS CodechalDecodeHistogramVebox::SetVeboxStateParams(
    PMHW_VEBOX_STATE_CMD_PARAMS veboxCmdParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    veboxCmdParams->bNoUseVeboxHeap                         = 0;

    veboxCmdParams->VeboxMode.ColorGamutExpansionEnable     = 0;
    veboxCmdParams->VeboxMode.ColorGamutCompressionEnable   = 0;
    // On SKL, GlobalIECP must be enabled when the output pipe is Vebox or SFC
    veboxCmdParams->VeboxMode.GlobalIECPEnable              = 1;
    veboxCmdParams->VeboxMode.DNEnable                      = 0;
    veboxCmdParams->VeboxMode.DIEnable                      = 0;
    veboxCmdParams->VeboxMode.DNDIFirstFrame                = 0;
    veboxCmdParams->VeboxMode.DIOutputFrames                = 0;
    veboxCmdParams->VeboxMode.PipeSynchronizeDisable        = 0;
    veboxCmdParams->VeboxMode.DemosaicEnable                = 0;
    veboxCmdParams->VeboxMode.VignetteEnable                = 0;
    veboxCmdParams->VeboxMode.AlphaPlaneEnable              = 0;
    veboxCmdParams->VeboxMode.HotPixelFilteringEnable       = 0;
    // 0-both slices enabled   1-Slice 0 enabled   2-Slice 1 enabled
    // On SKL GT3 and GT4, there are 2 Veboxes. But only Vebox0 can be used,Vebox1 cannot be used
    veboxCmdParams->VeboxMode.SingleSliceVeboxEnable        = 1;
    veboxCmdParams->VeboxMode.LACECorrectionEnable          = 0;
    veboxCmdParams->VeboxMode.DisableEncoderStatistics      = 1;
    veboxCmdParams->VeboxMode.DisableTemporalDenoiseFilter  = 1;
    veboxCmdParams->VeboxMode.SinglePipeIECPEnable          = 0;
    veboxCmdParams->VeboxMode.SFCParallelWriteEnable        = 0;
    veboxCmdParams->VeboxMode.ScalarMode                    = 0;
    veboxCmdParams->VeboxMode.ForwardGammaCorrectionEnable  = 0;

    return eStatus;
}

MOS_STATUS CodechalDecodeHistogramVebox::SetVeboxSurfaceStateParams(
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS veboxSurfParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    // Initialize SurfInput
    veboxSurfParams->SurfInput.bActive          = true;
    veboxSurfParams->SurfInput.Format           = m_inputSurface->Format;
    veboxSurfParams->SurfInput.dwWidth          = m_inputSurface->dwWidth;
    veboxSurfParams->SurfInput.dwHeight         = m_inputSurface->UPlaneOffset.iYOffset;  // For Planar formats, pParams->SurfInput.dwHeight will be assigned to VEBOX U.Y offset, which is only used for PLANAR surface formats.
    veboxSurfParams->SurfInput.dwUYoffset       = m_inputSurface->UPlaneOffset.iYOffset;
    veboxSurfParams->SurfInput.dwPitch          = m_inputSurface->dwPitch;
    veboxSurfParams->SurfInput.TileType         = m_inputSurface->TileType;
    veboxSurfParams->SurfInput.TileModeGMM      = m_inputSurface->TileModeGMM;
    veboxSurfParams->SurfInput.bGMMTileEnabled  = m_inputSurface->bGMMTileEnabled;
    veboxSurfParams->SurfInput.pOsResource      = &m_inputSurface->OsResource;
    veboxSurfParams->SurfInput.rcMaxSrc.left    = 0;
    veboxSurfParams->SurfInput.rcMaxSrc.top     = 0;
    veboxSurfParams->SurfInput.rcMaxSrc.right =
        MOS_ALIGN_CEIL(m_inputSurface->dwWidth, MHW_SFC_VE_WIDTH_ALIGN);
    veboxSurfParams->SurfInput.rcMaxSrc.bottom =
        MOS_ALIGN_CEIL(m_inputSurface->dwHeight, MHW_SFC_VE_HEIGHT_ALIGN);

    // Initialize SurfSTMM
    veboxSurfParams->SurfSTMM.dwPitch = m_inputSurface->dwPitch;

    veboxSurfParams->bDIEnable                  = false;
    veboxSurfParams->bOutputValid               = false;

    return eStatus;
}

MOS_STATUS CodechalDecodeHistogramVebox::SetVeboxDiIecpParams(
    PMHW_VEBOX_DI_IECP_CMD_PARAMS veboxDiIecpParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    veboxDiIecpParams->dwStartingX              = 0;
    veboxDiIecpParams->dwEndingX                = m_inputSurface->dwWidth - 1;
    veboxDiIecpParams->dwCurrInputSurfOffset    = m_inputSurface->dwOffset;
    veboxDiIecpParams->pOsResCurrInput          = &m_inputSurface->OsResource;
    veboxDiIecpParams->pOsResCurrOutput         = &m_outputSurface.OsResource;
    veboxDiIecpParams->CurrInputSurfCtrl.Value  = 0;
    veboxDiIecpParams->CurrOutputSurfCtrl.Value = 0;

    CodecHalGetResourceInfo(m_osInterface, m_inputSurface);
    CodecHalGetResourceInfo(m_osInterface, &m_outputSurface);

    veboxDiIecpParams->CurInputSurfMMCState =
        (MOS_MEMCOMP_STATE)(m_inputSurface->CompressionMode);
    veboxDiIecpParams->pOsResLaceOrAceOrRgbHistogram    = &m_resHistogram;
    veboxDiIecpParams->pOsResStatisticsOutput           = &m_resStatisticsOutput;

    return eStatus;
}

MOS_STATUS CodechalDecodeHistogramVebox::SetVeboxIecpParams(
    PMHW_VEBOX_IECP_PARAMS veboxIecpParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    veboxIecpParams->ColorPipeParams.bActive    = true;
    veboxIecpParams->ColorPipeParams.bEnableACE = true;
    veboxIecpParams->AceParams.bActive          = true;
    veboxIecpParams->srcFormat                  = m_inputSurface->Format;
    veboxIecpParams->bCSCEnable                 = false;

    return eStatus;
}

MOS_STATUS CodechalDecodeHistogramVebox::RenderHistogram(
    CodechalDecode *codechalDecoder,
    MOS_SURFACE *inputSurface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (Mos_ResourceIsNull(&m_inputHistogramSurfaces[m_histogramComponent].OsResource))
    {
        CODECHAL_DECODE_VERBOSEMESSAGE("Input histogram surface is null");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_decoder       = codechalDecoder;
    m_inputSurface  = inputSurface;

    AllocateResources();

    MOS_SYNC_PARAMS syncParams  = g_cInitSyncParams;
    syncParams.GpuContext       = m_decoder->GetVideoContext();
    syncParams.presSyncResource = &m_resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(
        m_osInterface,
        &syncParams));

    syncParams                  = g_cInitSyncParams;
    syncParams.GpuContext       = MOS_GPU_CONTEXT_VEBOX;
    syncParams.presSyncResource = &m_resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(
        m_osInterface,
        &syncParams));

    // Switch GPU context to VEBOX
    m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VEBOX);
    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(m_osInterface);

    // Send command buffer header at the beginning
    MOS_COMMAND_BUFFER cmdBuffer;
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));
    CODECHAL_HW_CHK_STATUS_RETURN(m_decoder->SendPrologWithFrameTracking(
        &cmdBuffer,
        true));

    // Setup cmd prameters
    MHW_VEBOX_STATE_CMD_PARAMS veboxStateCmdParams;
    MOS_ZeroMemory(&veboxStateCmdParams, sizeof(veboxStateCmdParams));
    CODECHAL_HW_CHK_STATUS_RETURN(SetVeboxStateParams(&veboxStateCmdParams));

    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS veboxSurfaceStateCmdParams;
    MOS_ZeroMemory(&veboxSurfaceStateCmdParams, sizeof(veboxSurfaceStateCmdParams));
    CODECHAL_HW_CHK_STATUS_RETURN(SetVeboxSurfaceStateParams(&veboxSurfaceStateCmdParams));

    MHW_VEBOX_DI_IECP_CMD_PARAMS veboxDiIecpCmdParams;
    MOS_ZeroMemory(&veboxDiIecpCmdParams, sizeof(veboxDiIecpCmdParams));
    CODECHAL_HW_CHK_STATUS_RETURN(SetVeboxDiIecpParams(&veboxDiIecpCmdParams));

    MHW_VEBOX_IECP_PARAMS veboxIecpParams;
    MOS_ZeroMemory(&veboxIecpParams, sizeof(veboxIecpParams));
    CODECHAL_HW_CHK_STATUS_RETURN(SetVeboxIecpParams(&veboxIecpParams));

    // send Vebox cmd
    CODECHAL_HW_CHK_STATUS_RETURN(m_veboxInterface->AddVeboxIecpState(
        &veboxIecpParams));

    CODECHAL_HW_CHK_STATUS_RETURN(m_veboxInterface->AddVeboxState(
        &cmdBuffer,
        &veboxStateCmdParams, 0));

    CODECHAL_HW_CHK_STATUS_RETURN(m_veboxInterface->AddVeboxSurfaces(
        &cmdBuffer,
        &veboxSurfaceStateCmdParams));

    CODECHAL_HW_CHK_STATUS_RETURN(m_veboxInterface->AddVeboxDiIecp(
        &cmdBuffer,
        &veboxDiIecpCmdParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_decoder->GetVideoContextUsesNullHw()));

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &veboxStateCmdParams.DummyIecpResource);

    // copy histogram to input buffer
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface, m_decoder->GetVideoWAContext()));
    m_osInterface->pfnResetOsStates(m_osInterface);

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_decoder->GetMode() << 4) & 0xF0) | COPY_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_decoder->SendPrologWithFrameTracking(
        &cmdBuffer,
        false));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_decoder->HucCopy(
        &cmdBuffer,
        &m_resHistogram,
        &m_inputHistogramSurfaces[m_histogramComponent].OsResource,
        HISTOGRAM_BINCOUNT * 4,
        m_veboxHistogramOffset,
        m_inputHistogramSurfaces[m_histogramComponent].dwOffset));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    // sync resource
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = MOS_GPU_CONTEXT_VEBOX;
    syncParams.presSyncResource = &m_resSyncObject;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(
        m_osInterface,
        &syncParams));

    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_decoder->GetVideoWAContext();
    syncParams.presSyncResource = &m_resSyncObject;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(
        m_osInterface,
        &syncParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_decoder->GetVideoContextUsesNullHw()));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_decoder->GetVideoContext()));

    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
    MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
    userFeatureWriteData.Value.i32Data = 1;
    userFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_DECODE_HISTOGRAM_FROM_VEBOX_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

    return eStatus;
}
