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
//! \file     decode_filmgrain_gennoise_rp2_packet_g12.cpp
//! \brief    film grain regress phase2 kernel render packet which used in by mediapipline.
//! \details  film grain regress phase2 kernel render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "decode_filmgrain_gennoise_rp2_packet_g12.h"
#include "decode_av1_filmgrain_feature_g12.h"
#include "decode_av1_feature_defs_g12.h"
#include "mos_defs.h"
#include "hal_oca_interface.h"
#include "codechal_utilities.h"

namespace decode
{

FilmGrainRp2Packet::FilmGrainRp2Packet(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface):
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

MOS_STATUS FilmGrainRp2Packet::Init()
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

MOS_STATUS FilmGrainRp2Packet::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_hwInterface);

    m_picParams = m_av1BasicFeature->m_av1PicParams;

    ResetBindingTableEntry();

    DECODE_CHK_STATUS(RenderEngineSetup());
    DECODE_CHK_STATUS(KernelStateSetup());
    DECODE_CHK_STATUS(SetUpSurfaceState());
    DECODE_CHK_STATUS(SetCurbeRegressPhase2());
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

MOS_STATUS FilmGrainRp2Packet::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
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
    pPerfProfiler  = pRenderHalLegacy->pPerfProfiler;
    pOsContext     = pOsInterface->pOsContext;
    pMmioRegisters = pMhwRender->GetMmioRegisters();

    RENDER_PACKET_CHK_STATUS_RETURN(SetPowerMode(CODECHAl_MEDIA_STATE_AV1_FILM_GRAIN_RP2));

    // Initialize command buffer and insert prolog
    RENDER_PACKET_CHK_STATUS_RETURN(pRenderHalLegacy->pfnInitCommandBuffer(pRenderHalLegacy, commandBuffer, &GenericPrologParams));

    HalOcaInterface::On1stLevelBBStart(*commandBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle,
        *m_hwInterface->GetMiInterface(), *m_hwInterface->GetMiInterface()->GetMmioRegisters());
    HalOcaInterface::TraceMessage(*commandBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, __FUNCTION__, sizeof(__FUNCTION__));
    HalOcaInterface::OnDispatch(*commandBuffer, *m_osInterface,  *m_hwInterface->GetMiInterface(),  *m_hwInterface->GetMiInterface()->GetMmioRegisters());

    if(pOsInterface && !m_av1BasicFeature->m_singleKernelPerfFlag)
    {
        pOsInterface->pfnSetPerfTag(pOsInterface, ((PERFTAG_CALL_FILM_GRAIN_RP2_KERNEL << 8) | CODECHAL_DECODE_MODE_AV1VLD << 4 | m_av1BasicFeature->m_pictureCodingType));
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

    if (!m_av1BasicFeature->m_singleKernelPerfFlag)
    {
        RENDER_PACKET_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectEndCmd((void *)pRenderHalLegacy, pOsInterface, pMhwMiInterface, commandBuffer));
    }

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

MOS_STATUS FilmGrainRp2Packet::SetupMediaWalker()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_hwInterface);

    // Current only add Media Walker Support in film Grain
    m_walkerType = WALKER_TYPE_MEDIA;

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    memset(&walkerCodecParams, 0, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode    = MHW_WALKER_MODE_DUAL;
    walkerCodecParams.dwResolutionX = 1;
    walkerCodecParams.dwResolutionY = 1;
    walkerCodecParams.bNoDependency = true;  // raster scan mode

    DECODE_CHK_STATUS(CodecHalInitMediaObjectWalkerParams(m_hwInterface, &m_mediaWalkerParams, &walkerCodecParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainRp2Packet::Initilize()
{
    m_kernelIndex = regressPhase2;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainRp2Packet::KernelStateSetup()
{
    m_kernelCount                 = 1;
    MHW_KERNEL_STATE *kernelState = &m_filmGrainFeature->m_kernelStates[m_kernelIndex];
    uint32_t          btCount     = m_filmGrainFeature->m_filmGrainBindingTableCount[m_kernelIndex];
    int32_t           curbeLength = m_filmGrainFeature->m_filmGrainCurbeSize[m_kernelIndex];

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
    m_renderData.KernelParam.blocks_x           = 1;
    m_renderData.KernelParam.blocks_y           = 1;

    m_renderData.iCurbeOffset = pRenderHalLegacy->pMhwStateHeap->GetSizeofCmdInterfaceDescriptorData();

    // Set Parameters for Kernel Entry
    m_renderData.KernelEntry.iKUID       = 0;
    m_renderData.KernelEntry.iKCID       = m_kernelIndex;
    m_renderData.KernelEntry.iFilterSize = 2;
    m_renderData.KernelEntry.pFilter     = m_filter;
    m_renderData.KernelEntry.iSize       = kernelState->KernelParams.iSize;
    m_renderData.KernelEntry.pBinary     = kernelState->KernelParams.pBinary;

    // set Curbe/Inline Data length
    m_renderData.iInlineLength = 0;
    m_renderData.iCurbeLength  = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainRp2Packet::SetUpSurfaceState()
{
    DECODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //Set Surface States
    RENDERHAL_SURFACE_NEXT renderHalSurfaceNext;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));

    //Y random values - input
    bool isWritable                  = false;
    RENDERHAL_SURFACE_STATE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;

    m_bindingTableIndex[rp2InputYRandomValue] = SetSurfaceForHwAccess(
        m_filmGrainFeature->m_yRandomValuesSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("RP2: surface[%d] Input Y Random values index: %d\n", rp2InputYRandomValue, m_bindingTableIndex[rp2InputYRandomValue]);

    //U random values - input
    isWritable = false;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[rp2InputURandomValue] = SetSurfaceForHwAccess(
        m_filmGrainFeature->m_uRandomValuesSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("RP2: surface[%d] Input U Random values BT index: %d\n", rp2InputURandomValue, m_bindingTableIndex[rp2InputURandomValue]);

    //V random values - input
    isWritable = false;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[rp2InputVRandomValue] = SetSurfaceForHwAccess(
        m_filmGrainFeature->m_vRandomValuesSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("RP2: surface[%d] Input V Random values BT index: %d\n", rp2InputVRandomValue, m_bindingTableIndex[rp2InputVRandomValue]);

    //Y dithering surface - input
    isWritable                  = true;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[rp2InputYDithering] = SetSurfaceForHwAccess(
        m_filmGrainFeature->m_yDitheringTempSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("RP2: surface[%d] Input Y dithering surface BT index: %d\n", rp2InputYDithering, m_bindingTableIndex[rp2InputYDithering]);

    //Y dithering surface - output
    isWritable                  = true;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[rp2OutputYDithering] = SetSurfaceForHwAccess(
        m_filmGrainFeature->m_yDitheringSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("RP2: surface[%d] Output Y dithering surface BT index: %d\n", rp2OutputYDithering, m_bindingTableIndex[rp2OutputYDithering]);

    //U dithering surface - output
    isWritable                  = true;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[rp2OutputUDithering] = SetSurfaceForHwAccess(
        m_filmGrainFeature->m_uDitheringSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("RP2: surface[%d] Output U dithering surface BT index: %d\n", rp2OutputUDithering, m_bindingTableIndex[rp2OutputUDithering]);

    //V dithering surface - output
    isWritable                  = true;
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = true;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[rp2OutputVDithering] = SetSurfaceForHwAccess(
        m_filmGrainFeature->m_vDitheringSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("RP2: surface[%d] Output V dithering surface BT index: %d\n", rp2OutputVDithering, m_bindingTableIndex[rp2OutputVDithering]);

    //Y coefficients - input
    isWritable                  = false;
    m_filmGrainFeature->m_yCoeffSurface->size = 32 * sizeof(short);
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = false;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParams.bBufferUse    = true;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[rp2InputYCoeff] = SetBufferForHwAccess(
        *m_filmGrainFeature->m_yCoeffSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("RP2: surface[%d] Y coeff input surface BT index: %d\n", rp2InputYCoeff, m_bindingTableIndex[rp2InputYCoeff]);

    //U coefficients - input
    isWritable                      = false;
    m_filmGrainFeature->m_uCoeffSurface->size = 32 * sizeof(short);
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl         = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = false;
    surfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParams.bBufferUse        = true;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT));
    m_bindingTableIndex[rp2InputUCoeff] = SetBufferForHwAccess(
        *m_filmGrainFeature->m_uCoeffSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("RP2: surface[%d] U coeff input surface BT index: %d\n", rp2InputUCoeff, m_bindingTableIndex[rp2InputUCoeff]);
    
    //V coefficients - input
    isWritable                  = false;
    m_filmGrainFeature->m_vCoeffSurface->size = 32 * sizeof(short);
    MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
    surfaceParams.MemObjCtl     = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3].Value;
    surfaceParams.isOutput = false;
    surfaceParams.Boundary      = RENDERHAL_SS_BOUNDARY_ORIGINAL;
    surfaceParams.bBufferUse    = true;
    MOS_ZeroMemory(&renderHalSurfaceNext, sizeof(RENDERHAL_SURFACE_NEXT)); 
    m_bindingTableIndex[rp2InputVCoeff] = SetBufferForHwAccess(
        *m_filmGrainFeature->m_vCoeffSurface,
        &renderHalSurfaceNext,
        &surfaceParams,
        isWritable);
    DECODE_VERBOSEMESSAGE("RP2: surface[%d] U coeff input surface BT index: %d\n", rp2InputVCoeff, m_bindingTableIndex[rp2InputVCoeff]);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainRp2Packet::SetCurbeRegressPhase2()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    CodecAv1FilmGrainParams *filmGrainParams = (CodecAv1FilmGrainParams *)&m_picParams->m_filmGrainParams;

    FilmGrainRegressPhase2Curbe curbe;
    curbe.DW0.YRandomValuesSurfaceIndex   = rp2InputYRandomValue;
    curbe.DW1.URandomValuesSurfaceIndex   = rp2InputURandomValue;
    curbe.DW2.VRandomValuesSurfaceIndex   = rp2InputVRandomValue;
    curbe.DW3.YDitheringInputSurfaceIndex = rp2InputYDithering;
    curbe.DW4.YDitheringSurfaceIndex      = rp2OutputYDithering;
    curbe.DW5.UDitheringSurfaceIndex      = rp2OutputUDithering;
    curbe.DW6.VDitheringSurfaceIndex      = rp2OutputVDithering;
    curbe.DW7.YCoeffSurfaceIndex          = rp2InputYCoeff;
    curbe.DW8.UCoeffSurfaceIndex          = rp2InputUCoeff;
    curbe.DW9.VCoeffSurfaceIndex          = rp2InputVCoeff;
    curbe.DW10.RegressionCoefficientShift = m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_arCoeffShiftMinus6 + 6;

    DECODE_CHK_STATUS(SetupCurbe(
        &curbe,
        sizeof(FilmGrainRegressPhase2Curbe),
        m_renderData.KernelParam.Thread_Count));

    return eStatus;
}


MOS_STATUS FilmGrainRp2Packet::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    commandBufferSize      = m_hwInterface->GetKernelLoadCommandSize(m_renderData.KernelParam.BT_Count);
    requestedPatchListSize = 0;

    return MOS_STATUS_SUCCESS;
}

}
