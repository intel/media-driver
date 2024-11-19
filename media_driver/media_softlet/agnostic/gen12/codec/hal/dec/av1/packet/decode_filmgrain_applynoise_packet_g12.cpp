/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_filmgrain_applynoise_packet_g12.cpp
//! \brief    film grain apply noise kernel render packet which used in by mediapipline.
//! \details  film grain apply noise kernel render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "decode_filmgrain_applynoise_packet_g12.h"
#include "decode_av1_filmgrain_feature_g12.h"
#include "decode_av1_feature_defs_g12.h"
#include "mos_defs.h"
#include "hal_oca_interface.h"
#include "codechal_utilities.h"

namespace decode 
{

FilmGrainAppNoisePkt::FilmGrainAppNoisePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface):
    CmdPacket(task),
    RenderCmdPacket(task, hwInterface->GetOsInterface(), hwInterface->GetRenderHalInterface())
{
        if (pipeline != nullptr)
        {
            m_statusReport   = pipeline->GetStatusReportInstance();
            m_featureManager = pipeline->GetFeatureManager();
            m_av1Pipeline    = dynamic_cast<Av1PipelineG12_Base *>(pipeline);
        }
        if (hwInterface != nullptr)
        {
            m_hwInterface    = hwInterface;
            m_miInterface    = hwInterface->GetMiInterface();
            m_osInterface    = hwInterface->GetOsInterface();
            m_vdencInterface = hwInterface->GetVdencInterface();
            m_renderHal      = hwInterface->GetRenderHalInterface();
        }
    }

MOS_STATUS FilmGrainAppNoisePkt::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_miInterface);
    DECODE_CHK_NULL(m_statusReport);
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_av1Pipeline);
    DECODE_CHK_NULL(m_osInterface);
    DECODE_CHK_NULL(m_vdencInterface);

    DECODE_CHK_STATUS(RenderCmdPacket::Init());

    m_av1BasicFeature = dynamic_cast<Av1BasicFeatureG12 *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_av1BasicFeature);

    m_filmGrainFeature = dynamic_cast<Av1DecodeFilmGrainG12 *>(m_featureManager->GetFeature(Av1FeatureIDs::av1SwFilmGrain));
    DECODE_CHK_NULL(m_filmGrainFeature);

    m_allocator = m_av1Pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(Initilize());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainAppNoisePkt::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_hwInterface);

    m_picParams = m_av1BasicFeature->m_av1PicParams;

    ResetBindingTableEntry();

    m_filmGrainProcParams = m_av1BasicFeature->m_filmGrainProcParams;

    DECODE_CHK_STATUS(RenderEngineSetup());
    DECODE_CHK_STATUS(KernelStateSetup());
    DECODE_CHK_STATUS(SetUpSurfaceState());
    DECODE_CHK_STATUS(SetCurbeApplyNoise(m_filmGrainProcParams));
    DECODE_CHK_STATUS(LoadKernel());

    if (m_walkerType == WALKER_TYPE_MEDIA)
    {
        DECODE_CHK_STATUS(SetupMediaWalker());
    }
    else if (m_walkerType == WALKER_TYPE_COMPUTE)
    {
        m_renderData.walkerParam.alignedRect.left   = 0;
        m_renderData.walkerParam.alignedRect.top    = 0;
        m_renderData.walkerParam.alignedRect.right  = m_av1BasicFeature->m_filmGrainProcParams->m_outputSurface->dwWidth;
        m_renderData.walkerParam.alignedRect.bottom = m_av1BasicFeature->m_filmGrainProcParams->m_outputSurface->dwHeight;
        m_renderData.walkerParam.iCurbeLength       = m_renderData.iCurbeLength;
        m_renderData.walkerParam.iCurbeOffset       = m_renderData.iCurbeOffset;
        m_renderData.walkerParam.iBindingTable      = m_renderData.bindingTable;
        m_renderData.walkerParam.iMediaID           = m_renderData.mediaID;
        m_renderData.walkerParam.iBlocksX           = m_renderData.KernelParam.blocks_x;
        m_renderData.walkerParam.iBlocksY           = m_renderData.KernelParam.blocks_y;
        DECODE_CHK_STATUS(PrepareComputeWalkerParams(m_renderData.walkerParam, m_gpgpuWalkerParams));
    }
    else
    {
        DECODE_ASSERTMESSAGE("Walker is disabled!");
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainAppNoisePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    PMOS_INTERFACE                  pOsInterface = nullptr;
    MOS_STATUS                      eStatus      = MOS_STATUS_SUCCESS;
    uint32_t                        dwSyncTag    = 0;
    int32_t                         i = 0, iRemaining = 0;
    PMHW_MI_INTERFACE               pMhwMiInterface     = nullptr;
    MhwRenderInterface *            pMhwRender          = nullptr;
    MHW_MEDIA_STATE_FLUSH_PARAM     FlushParam          = {};
    bool                            bEnableSLM          = false;
    RENDERHAL_GENERIC_PROLOG_PARAMS GenericPrologParams = {};
    MOS_RESOURCE                    GpuStatusBuffer     = {};
    MediaPerfProfiler *             pPerfProfiler       = nullptr;
    MOS_CONTEXT *                   pOsContext          = nullptr;
    PMHW_MI_MMIOREGISTERS           pMmioRegisters      = nullptr;
    PRENDERHAL_INTERFACE_LEGACY     pRenderHalLegacy    = (PRENDERHAL_INTERFACE_LEGACY)m_renderHal;
    
    RENDER_PACKET_CHK_NULL_RETURN(pRenderHalLegacy);
    RENDER_PACKET_CHK_NULL_RETURN(pRenderHalLegacy->pMhwRenderInterface);
    RENDER_PACKET_CHK_NULL_RETURN(pRenderHalLegacy->pMhwMiInterface);
    RENDER_PACKET_CHK_NULL_RETURN(pRenderHalLegacy->pMhwRenderInterface->GetMmioRegisters());
    RENDER_PACKET_CHK_NULL_RETURN(pRenderHalLegacy->pOsInterface);
    RENDER_PACKET_CHK_NULL_RETURN(pRenderHalLegacy->pOsInterface->pOsContext);

    eStatus         = MOS_STATUS_UNKNOWN;
    pOsInterface    = pRenderHalLegacy->pOsInterface;
    pMhwMiInterface = pRenderHalLegacy->pMhwMiInterface;
    pMhwRender      = pRenderHalLegacy->pMhwRenderInterface;
    iRemaining      = 0;
    FlushParam      = g_cRenderHal_InitMediaStateFlushParams;
    pPerfProfiler   = pRenderHalLegacy->pPerfProfiler;
    pOsContext      = pOsInterface->pOsContext;
    pMmioRegisters  = pMhwRender->GetMmioRegisters();

    RENDER_PACKET_CHK_STATUS_RETURN(SetPowerMode(CODECHAl_MEDIA_STATE_AV1_FILM_GRAIN_AN));

    PMOS_RESOURCE resource = nullptr;
    uint32_t      offset   = 0;
    DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::statusReportGlobalCount, resource, offset));

    GenericPrologParams.bEnableMediaFrameTracking     = true;
    GenericPrologParams.presMediaFrameTrackingSurface = resource;
    GenericPrologParams.dwMediaFrameTrackingTag       = m_statusReport->GetSubmittedCount() + 1;
    // Set media frame tracking address offset(the offset from the decoder status buffer page)
    GenericPrologParams.dwMediaFrameTrackingAddrOffset = offset;
    // Initialize command buffer and insert prolog
    RENDER_PACKET_CHK_STATUS_RETURN(pRenderHalLegacy->pfnInitCommandBuffer(pRenderHalLegacy, commandBuffer, &GenericPrologParams));
    RENDER_PACKET_CHK_STATUS_RETURN(StartStatusReport(statusReportRcs, commandBuffer));

    HalOcaInterface::On1stLevelBBStart(*commandBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle,
        *m_hwInterface->GetMiInterface(), *m_hwInterface->GetMiInterface()->GetMmioRegisters());
    HalOcaInterface::TraceMessage(*commandBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, __FUNCTION__, sizeof(__FUNCTION__));
    HalOcaInterface::OnDispatch(*commandBuffer, *m_osInterface,  *m_hwInterface->GetMiInterface(),  *m_hwInterface->GetMiInterface()->GetMmioRegisters());

    if (pOsInterface && !m_av1BasicFeature->m_singleKernelPerfFlag)
    {
        RENDER_PACKET_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectStartCmd((void *)pRenderHalLegacy, pOsInterface, pMhwMiInterface, commandBuffer));
    }

    // Write timing data for 3P budget
    RENDER_PACKET_CHK_STATUS_RETURN(pRenderHalLegacy->pfnSendTimingData(pRenderHalLegacy, commandBuffer, true));

    bEnableSLM = false;  // Media walker first
    RENDER_PACKET_CHK_STATUS_RETURN(pRenderHalLegacy->pfnSetCacheOverrideParams(
        pRenderHalLegacy,
        &pRenderHalLegacy->L3CacheSettings,
        bEnableSLM));

    // Flush media states
    RENDER_PACKET_CHK_STATUS_RETURN(pRenderHalLegacy->pfnSendMediaStates(
        pRenderHalLegacy,
        commandBuffer,
        m_walkerType == WALKER_TYPE_MEDIA ? &m_mediaWalkerParams : nullptr,
        m_walkerType == WALKER_TYPE_MEDIA ? nullptr : &m_gpgpuWalkerParams));

    // Write back GPU Status tag
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        RENDER_PACKET_CHK_STATUS_RETURN(pRenderHalLegacy->pfnSendRcsStatusTag(pRenderHalLegacy, commandBuffer));
    }

    RENDER_PACKET_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectEndCmd((void *)pRenderHalLegacy, pOsInterface, pMhwMiInterface, commandBuffer));

    // Write timing data for 3P budget
    RENDER_PACKET_CHK_STATUS_RETURN(pRenderHalLegacy->pfnSendTimingData(pRenderHalLegacy, commandBuffer, false));

    MHW_PIPE_CONTROL_PARAMS PipeControlParams;

    MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
    PipeControlParams.dwFlushMode                   = MHW_FLUSH_WRITE_CACHE;
    PipeControlParams.bGenericMediaStateClear       = false;
    PipeControlParams.bIndirectStatePointersDisable = true;
    PipeControlParams.bDisableCSStall               = false;
    RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddPipeControl(commandBuffer, nullptr, &PipeControlParams));

    if (MEDIA_IS_WA(pRenderHalLegacy->pWaTable, WaSendDummyVFEafterPipelineSelect))
    {
        MHW_VFE_PARAMS VfeStateParams       = {};
        VfeStateParams.dwNumberofURBEntries = 1;
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwRender->AddMediaVfeCmd(commandBuffer, &VfeStateParams));
    }

    // Add media flush command in case HW not cleaning the media state
    if (MEDIA_IS_WA(pRenderHalLegacy->pWaTable, WaMSFWithNoWatermarkTSGHang))
    {
        FlushParam.bFlushToGo = true;
        if (m_walkerType == WALKER_TYPE_MEDIA)
        {
            FlushParam.ui8InterfaceDescriptorOffset = m_mediaWalkerParams.InterfaceDescriptorOffset;
        }
        else
        {
            RENDER_PACKET_ASSERTMESSAGE("ERROR, pWalkerParams is nullptr and cannot get InterfaceDescriptorOffset.");
        }
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddMediaStateFlush(commandBuffer, nullptr, &FlushParam));
    }
    else if (MEDIA_IS_WA(pRenderHalLegacy->pWaTable, WaAddMediaStateFlushCmd))
    {
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddMediaStateFlush(commandBuffer, nullptr, &FlushParam));
    }

    //Status report
    DECODE_CHK_STATUS(EndStatusReport(statusReportRcs, commandBuffer));
    DECODE_CHK_STATUS(UpdateStatusReport(statusReportGlobalCount, commandBuffer));

    HalOcaInterface::On1stLevelBBEnd(*commandBuffer, *m_osInterface);

    if (pBatchBuffer)
    {
        // Send Batch Buffer end command (HW/OS dependent)
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddMiBatchBufferEnd(commandBuffer, nullptr));
    }
    else if (IsMiBBEndNeeded(pOsInterface))
    {
        // Send Batch Buffer end command for 1st level Batch Buffer
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddMiBatchBufferEnd(commandBuffer, nullptr));
    }
    else if (pRenderHalLegacy->pOsInterface->bNoParsingAssistanceInKmd)
    {
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddMiBatchBufferEnd(commandBuffer, nullptr));
    }

    // Return unused command buffer space to OS
    pOsInterface->pfnReturnCommandBuffer(pOsInterface, commandBuffer, 0);

    MOS_NULL_RENDERING_FLAGS NullRenderingFlags = pOsInterface->pfnGetNullHWRenderFlags(pOsInterface);

    if ((NullRenderingFlags.VPLgca ||
            NullRenderingFlags.VPGobal) == false)
    {
        dwSyncTag = pRenderHalLegacy->pStateHeap->dwNextTag++;

        // Set media state and batch buffer as busy
        pRenderHalLegacy->pStateHeap->pCurMediaState->bBusy = true;
        if (pBatchBuffer)
        {
            pBatchBuffer->bBusy     = true;
            pBatchBuffer->dwSyncTag = dwSyncTag;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainAppNoisePkt::SetupMediaWalker()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_hwInterface);

    // Current only add Media Walker Support in film Grain
    m_walkerType = WALKER_TYPE_MEDIA;

    uint32_t resolutionX = MOS_ROUNDUP_DIVIDE(m_picParams->m_superResUpscaledWidthMinus1 + 1, 32);
    uint32_t resolutionY = MOS_ROUNDUP_DIVIDE(m_picParams->m_superResUpscaledHeightMinus1 + 1, 8);

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    memset(&walkerCodecParams, 0, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode    = MHW_WALKER_MODE_DUAL;
    walkerCodecParams.dwResolutionX = resolutionX;
    walkerCodecParams.dwResolutionY = resolutionY;
    walkerCodecParams.bNoDependency = true;  // raster scan mode

    DECODE_CHK_STATUS(CodecHalInitMediaObjectWalkerParams(m_hwInterface, &m_mediaWalkerParams, &walkerCodecParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainAppNoisePkt::Initilize()
{
    m_kernelIndex = applyNoise;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainAppNoisePkt::KernelStateSetup()
{
    MHW_KERNEL_STATE *kernelState = &m_filmGrainFeature->m_kernelStates[m_kernelIndex];
    uint32_t          btCount     = m_filmGrainFeature->m_filmGrainBindingTableCount[m_kernelIndex];
    int32_t           curbeLength = m_filmGrainFeature->m_filmGrainCurbeSize[m_kernelIndex];
    m_kernelCount                 = 1;

    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)m_renderHal;
    // Initialize States
    MOS_ZeroMemory(m_filter, sizeof(Kdll_FilterEntry));
    MOS_ZeroMemory(&m_renderData.KernelEntry, sizeof(Kdll_CacheEntry));

    // Set Kernel Parameter
    m_renderData.KernelParam.GRF_Count          = 0;
    m_renderData.KernelParam.BT_Count           = btCount;
    m_renderData.KernelParam.Sampler_Count      = 0;
    m_renderData.KernelParam.Thread_Count       = pRenderHalLegacy->pMhwRenderInterface->GetHwCaps()->dwMaxThreads;
    m_renderData.KernelParam.GRF_Start_Register = 0;
    m_renderData.KernelParam.CURBE_Length       = curbeLength;
    m_renderData.KernelParam.block_width        = CODECHAL_MACROBLOCK_WIDTH;
    m_renderData.KernelParam.block_height       = CODECHAL_MACROBLOCK_HEIGHT;

    //Set per kernel
    uint32_t resolutionX              = MOS_ROUNDUP_DIVIDE(m_picParams->m_superResUpscaledWidthMinus1 + 1, 32);
    uint32_t resolutionY              = MOS_ROUNDUP_DIVIDE(m_picParams->m_superResUpscaledHeightMinus1 + 1, 8);
    m_renderData.KernelParam.blocks_x = resolutionX;
    m_renderData.KernelParam.blocks_y = resolutionY;

    m_renderData.iCurbeOffset = pRenderHalLegacy->pMhwStateHeap->GetSizeofCmdInterfaceDescriptorData();

    // Set Parameters for Kernel Entry
    m_renderData.KernelEntry.iKUID       = 0;
    m_renderData.KernelEntry.iKCID       = m_kernelIndex;
    m_renderData.KernelEntry.iFilterSize = 2;
    m_renderData.KernelEntry.pFilter     = m_filter;
    m_renderData.KernelEntry.iSize       = kernelState->KernelParams.iSize;
    m_renderData.KernelEntry.pBinary     = kernelState->KernelParams.pBinary;

    // Set Curbe/Inline Data length
    m_renderData.iInlineLength  = 0;
    m_renderData.iCurbeLength   = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainAppNoisePkt::SetUpSurfaceState()
{
    DECODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Input YUV surface
    RENDERHAL_SURFACE_NEXT renderHalSurfaceNext;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));

    bool                           isWritable = false;
    RENDERHAL_SURFACE_STATE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl         = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput     = true;
    surfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    if (m_hwInterface->Uses2PlanesInputSurfaceFilmGrain())
    {
        surfaceParams.b2PlaneNV12NeededByKernel = true;
    }
    else
    {
        surfaceParams.bUseSinglePlane      = true;
        renderHalSurfaceNext.dwHeightInUse = (m_filmGrainProcParams->m_inputSurface->UPlaneOffset.iYOffset * 3) / 2;
        renderHalSurfaceNext.dwWidthInUse  = 0;
    }

    m_bindingTableIndex[anInputYOrYuv] = SetSurfaceForHwAccess(
        m_filmGrainProcParams->m_inputSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);

    if (m_hwInterface->Uses2PlanesInputSurfaceFilmGrain())
    {
        m_bindingTableIndex[anInputUv] = m_bindingTableIndex[anInputYOrYuv] + 1;
        DECODE_VERBOSEMESSAGE("AN:surface[%d] Input Y surface index: %d\n", anInputYOrYuv, m_bindingTableIndex[anInputYOrYuv]);
        DECODE_VERBOSEMESSAGE("AN:surface[%d] Input UV surface index: %d\n", anInputUv, m_bindingTableIndex[anInputUv]);
    }
    else
    {
        DECODE_VERBOSEMESSAGE("AN:surface[%d] Input YUV surface index: %d\n", anInputYOrYuv, m_bindingTableIndex[anInputYOrYuv]);
    }

    // Output Y/UV surface
    isWritable                  = true;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParams.bWidthInDword_Y = true;
    surfaceParams.bWidthInDword_UV = true;

    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[anOutputY] = SetSurfaceForHwAccess(
        m_filmGrainProcParams->m_outputSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    m_bindingTableIndex[anOutputUv] = m_bindingTableIndex[anOutputY] + 1;
    DECODE_VERBOSEMESSAGE("AN:surface[%d] Output Y surface index: %d\n", anOutputY, m_bindingTableIndex[anOutputY]);
    DECODE_VERBOSEMESSAGE("AN:surface[%d] Output UV surface index: %d\n", anOutputUv, m_bindingTableIndex[anOutputUv]);

    //Y dithering surface - input
    isWritable                  = true;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[anInputYDithering] = SetSurfaceForHwAccess(
        m_filmGrainFeature->m_yDitheringSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("AN:surface[%d] Input Y dithering surface BT index: %d\n", anInputYDithering, m_bindingTableIndex[anInputYDithering]);

    //U dithering surface - input
    isWritable                  = true;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[anInputUDithering] = SetSurfaceForHwAccess(
        m_filmGrainFeature->m_uDitheringSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("AN:surface[%d] Input U dithering surface BT index: %d\n", anInputUDithering, m_bindingTableIndex[anInputUDithering]);

    //V dithering surface - input
    isWritable                  = true;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[anInputVDithering] = SetSurfaceForHwAccess(
        m_filmGrainFeature->m_vDitheringSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("AN:surface[%d] Input V dithering surface BT index: %d\n", anInputVDithering, m_bindingTableIndex[anInputVDithering]);

    //Coordinates random values - input
    isWritable = true;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParams.bBufferUse    = true;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[anInputRandomValuesCoordinates] = SetBufferForHwAccess(
        *m_filmGrainFeature->m_coordinatesRandomValuesSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("AN:surface[%d] Coordinate random values BT index: %d\n", anInputRandomValuesCoordinates, m_bindingTableIndex[anInputRandomValuesCoordinates]);

    //Y Gamma LUT surface - input
    isWritable                 = false;
    m_filmGrainFeature->m_yGammaLUTSurface->size   = MOS_ALIGN_CEIL(257 * sizeof(int16_t), CODECHAL_PAGE_SIZE);

    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = false;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParams.bBufferUse    = true;

    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[anInputYGammaLut] = SetBufferForHwAccess(
        *m_filmGrainFeature->m_yGammaLUTSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("AN:surface[%d] Input Y Gamma LUT BT index: %d\n", anInputYGammaLut, m_bindingTableIndex[anInputYGammaLut]);

    //U Gamma LUT surface - input
    isWritable               = false;
    m_filmGrainFeature->m_uGammaLUTSurface->size = MOS_ALIGN_CEIL(257 * sizeof(int16_t), CODECHAL_PAGE_SIZE);

    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = false;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParams.bBufferUse    = true;

    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[anInputUGammaLut] = SetBufferForHwAccess(
        *m_filmGrainFeature->m_uGammaLUTSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("AN:surface[%d] Input U Gamma LUT BT index: %d\n", anInputUGammaLut, m_bindingTableIndex[anInputUGammaLut]);

    //V Gamma LUT surface - input
    isWritable               = false;
    m_filmGrainFeature->m_vGammaLUTSurface->size = MOS_ALIGN_CEIL(257 * sizeof(int16_t), CODECHAL_PAGE_SIZE);

    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = false;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParams.bBufferUse    = true;

    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[anInputVGammaLut] = SetBufferForHwAccess(
        *m_filmGrainFeature->m_vGammaLUTSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("AN:surface[%d] Input V Gamma LUT BT index: %d\n", anInputVGammaLut, m_bindingTableIndex[anInputVGammaLut]);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainAppNoisePkt::SetCurbeApplyNoise(
    FilmGrainProcParams *procParams)
{
    DECODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CodecAv1FilmGrainParams *filmGrainParams = (CodecAv1FilmGrainParams *)&m_picParams->m_filmGrainParams;

    DECODE_CHK_NULL(procParams);
    DECODE_CHK_NULL(procParams->m_inputSurface);
    DECODE_CHK_NULL(procParams->m_outputSurface);

    MOS_SURFACE *inputSurface  = procParams->m_inputSurface;
    MOS_SURFACE *outputSurface = procParams->m_outputSurface;

    FilmGrainApplyNoiseCurbe curbe;

    curbe.DW0.InputYuvSurfaceIndex                   = m_bindingTableIndex[anInputYOrYuv];
    curbe.DW1.OutputYSurfaceIndex                    = m_bindingTableIndex[anOutputY];
    curbe.DW2.OutputUvSurfaceIndex                   = m_bindingTableIndex[anOutputUv];
    curbe.DW3.YDitheringSurfaceIndex                 = m_bindingTableIndex[anInputYDithering];
    curbe.DW4.UDitheringSurfaceIndex                 = m_bindingTableIndex[anInputUDithering];
    curbe.DW5.VDitheringSurfaceIndex                 = m_bindingTableIndex[anInputVDithering];
    curbe.DW6.RandomValuesForCoordinatesSurfaceIndex = m_bindingTableIndex[anInputRandomValuesCoordinates];
    curbe.DW7.YGammaCorrectionLutSurfaceIndex        = m_bindingTableIndex[anInputYGammaLut];
    curbe.DW8.UGammaCorrectionLutSurfaceIndex        = m_bindingTableIndex[anInputUGammaLut];
    curbe.DW9.VGammaCorrectionLutSurfaceIndex        = m_bindingTableIndex[anInputVGammaLut];

    int apply_y  = (m_picParams->m_filmGrainParams.m_numYPoints > 0) ? 1 : 0;
    int apply_cb = (m_picParams->m_filmGrainParams.m_numCbPoints > 0 || m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_chromaScalingFromLuma) ? 1 : 0;
    int apply_cr = (m_picParams->m_filmGrainParams.m_numCrPoints > 0 || m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_chromaScalingFromLuma) ? 1 : 0;

    curbe.DW10.EnableYFilmGrain                     = apply_y;
    curbe.DW10.EnableUFilmGrain                     = apply_cb;
    curbe.DW11.EnableVFilmGrain                     = apply_cr;
    curbe.DW11.RandomValuesForCoordinatesTableWidth = MOS_ROUNDUP_SHIFT(m_picParams->m_superResUpscaledWidthMinus1 + 1, 6);
    curbe.DW12.ImageHeight                          = inputSurface->UPlaneOffset.iYOffset;  //specify UV vertical offset
    curbe.DW12.ScalingShiftValue                    = m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_grainScalingMinus8 + 8;

    int32_t minLuma, maxLuma, minChroma, maxChroma;
    int32_t bitDepth = m_picParams->m_bitDepthIdx? 10:8;//m_bitDepthIndicator ? 10 : 8;
    if (m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_clipToRestrictedRange)
    {
        minLuma = m_minLumaLegalRange << (bitDepth - 8);
        maxLuma = m_maxLumaLegalRange << (bitDepth - 8);

        if (m_picParams->m_matrixCoefficients == mcIdentity)
        {
            minChroma = m_minLumaLegalRange << (bitDepth - 8);
            maxChroma = m_maxLumaLegalRange << (bitDepth - 8);
        }
        else
        {
            minChroma = m_minChromaLegalRange << (bitDepth - 8);
            maxChroma = m_maxChromaLegalRange << (bitDepth - 8);
        }
    }
    else
    {
        minLuma = minChroma = 0;
        maxLuma = maxChroma = (256 << (bitDepth - 8)) - 1;
    }

    curbe.DW13.MaximumYClippingValue  = maxLuma;
    curbe.DW13.MinimumYClippingValue  = minLuma;
    curbe.DW14.MaximumUvClippingValue = maxChroma;
    curbe.DW14.MinimumUvClippingValue = minChroma;

    int32_t cbMult     = m_picParams->m_filmGrainParams.m_cbMult - 128;
    int32_t cbLumaMult = m_picParams->m_filmGrainParams.m_cbLumaMult - 128;
    int32_t cbOffset   = (m_picParams->m_filmGrainParams.m_cbOffset << (bitDepth - 8)) - (1 << bitDepth);
    int32_t crMult     = m_picParams->m_filmGrainParams.m_crMult - 128;
    int32_t crLumaMult = m_picParams->m_filmGrainParams.m_crLumaMult - 128;
    int32_t crOffset   = (m_picParams->m_filmGrainParams.m_crOffset << (bitDepth - 8)) - (1 << bitDepth);

    if (m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_chromaScalingFromLuma)
    {
        cbMult     = 0;
        cbLumaMult = 64;
        cbOffset   = 0;

        crMult     = 0;
        crLumaMult = 64;
        crOffset   = 0;
    }
    curbe.DW15.CbLumaMultiplier = cbLumaMult;
    curbe.DW15.CbMultiplier     = cbMult;
    curbe.DW16.CbOffset         = cbOffset;
    curbe.DW16.CrLumaMultiplier = crLumaMult;
    curbe.DW17.CrMultiplier     = crMult;
    curbe.DW17.CrOffset         = crOffset;

    DECODE_CHK_STATUS(SetupCurbe(
        &curbe,
        sizeof(FilmGrainApplyNoiseCurbe),
        m_renderData.KernelParam.Thread_Count));

    return eStatus;
}

MOS_STATUS FilmGrainAppNoisePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    commandBufferSize      = m_hwInterface->GetKernelLoadCommandSize(m_renderData.KernelParam.BT_Count);
    requestedPatchListSize = 0;

    return MOS_STATUS_SUCCESS;
}

}
