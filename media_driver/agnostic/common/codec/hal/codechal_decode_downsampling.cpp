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
//! \file     codechal_decode_downsampling.cpp
//! \brief    Implements the decode interface extension for field downsampling.
//! \details  Downsampling in this case is supported by EU kernels.
//!
#include "codechal_decoder.h"
#include "codeckrnheader.h"

// Construct function of Class MediaWalkerFieldScalingStaticData
MediaWalkerFieldScalingStaticData::MediaWalkerFieldScalingStaticData()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    memset(&m_mediaWalkerData, 0, sizeof(m_mediaWalkerData));
    m_mediaWalkerData.m_dword07.m_value         = 0x7;
    m_mediaWalkerData.m_dword14.m_nlasEnable    = false;
}

// Initialize the static const float variables in class FieldScalingInterface.
const float FieldScalingInterface::m_maxScaleRatio = 1.0f;
const float FieldScalingInterface::m_minScaleRatio = 0.125f;

FieldScalingInterface::FieldScalingInterface(CodechalHwInterface *hwInterface)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    memset(&m_kernelSize, 0, sizeof(m_kernelSize));
    memset(&m_dshSize, 0, sizeof(m_dshSize));
    memset(&m_syncObject, 0, sizeof(m_syncObject));

    for (uint8_t i = stateNv12; i < stateMax; i++)
    {
        m_kernelBinary[i] = nullptr;
        m_kernelStates[i] = MHW_KERNEL_STATE();
    }

    m_kernelUID[stateNv12] = IDR_CODEC_ALLPL2ToNV12iScale;
    m_kernelUID[stateYuy2] = IDR_CODEC_ALLPL2ToPAiScale;

    m_curbeLength = MediaWalkerFieldScalingStaticData::m_byteSize;
}

FieldScalingInterface::~FieldScalingInterface()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_mmcState != nullptr)
    {
        MOS_Delete(m_mmcState);
        m_mmcState = nullptr;
    }

    CODECHAL_DECODE_ASSERT(m_osInterface);
    if (m_osInterface != nullptr)
    {
        m_osInterface->pfnDestroySyncResource(m_osInterface, &m_syncObject);
    }
}

MOS_STATUS FieldScalingInterface::InitInterfaceStateHeapSetting(
    CodechalHwInterface               *hwInterface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MHW_KERNEL_STATE        *kernelState;
    for (auto krnlIdx = 0; krnlIdx < stateMax; krnlIdx++)
    {
        kernelState = &m_kernelStates[krnlIdx];

        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(
            m_kernelBase,
            m_kernelUID[krnlIdx],
            &m_kernelBinary[krnlIdx],
            &m_kernelSize[krnlIdx]));

        kernelState->KernelParams.iCurbeLength  = m_curbeLength;
        kernelState->KernelParams.pBinary       = m_kernelBinary[krnlIdx];
        kernelState->KernelParams.iSize         = m_kernelSize[krnlIdx];

        hwInterface->GetStateHeapSettings()->dwIshSize +=
            MOS_ALIGN_CEIL(
                kernelState->KernelParams.iSize,
                (1 << MHW_KERNEL_OFFSET_SHIFT));
    }

    hwInterface->GetStateHeapSettings()->dwNumSyncTags += m_numSyncTags;
    hwInterface->GetStateHeapSettings()->dwDshSize     += m_initDshSize;

    return eStatus;
}

MOS_STATUS FieldScalingInterface::SetCurbeFieldScaling(
    MHW_KERNEL_STATE                    *kernelState,
    CODECHAL_DECODE_PROCESSING_PARAMS   *procParams)
{
    MOS_STATUS                                 eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(kernelState);
    CODECHAL_DECODE_CHK_NULL_RETURN(procParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(procParams->pInputSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(procParams->pOutputSurface);

    MOS_SURFACE *inputSurface = procParams->pInputSurface;
    MOS_SURFACE *outputSurface = procParams->pOutputSurface;

    float stepX = (float)procParams->rcInputSurfaceRegion.Width /
        (float)(procParams->rcOutputSurfaceRegion.Width * inputSurface->dwWidth);
    float stepY = (float)procParams->rcInputSurfaceRegion.Height /
        (float)(procParams->rcOutputSurfaceRegion.Height * inputSurface->dwHeight);

    MediaWalkerFieldScalingStaticData cmd;
    cmd.m_mediaWalkerData.m_dword07.m_pointerToInlineParameters         = 0xB;
    cmd.m_mediaWalkerData.m_dword08.m_destinationRectangleWidth         = procParams->rcOutputSurfaceRegion.Width;
    cmd.m_mediaWalkerData.m_dword08.m_destinationRectangleHeight        = procParams->rcOutputSurfaceRegion.Height;

    if (outputSurface->Format == Format_NV12)
    {
        cmd.m_mediaWalkerData.m_dword10.m_chromaSitingLocation          = CODECHAL_CHROMA_SUBSAMPLING_CENTER_LEFT;
    }
    else
    {
        cmd.m_mediaWalkerData.m_dword10.m_chromaSitingLocation          = CODECHAL_CHROMA_SUBSAMPLING_TOP_LEFT;
    }

    cmd.m_mediaWalkerData.m_dword16.m_horizontalScalingStepRatioLayer0  = stepX;
    cmd.m_mediaWalkerData.m_dword24.m_verticalScalingStepRatioLayer0    = stepY;
    cmd.m_mediaWalkerData.m_dword48.m_destXTopLeftLayer0                = 0;
    cmd.m_mediaWalkerData.m_dword48.m_destYTopLeftLayer0                = 0;
    cmd.m_mediaWalkerData.m_dword56.m_destXBottomRightLayer0            = procParams->rcOutputSurfaceRegion.Width - 1;
    cmd.m_mediaWalkerData.m_dword56.m_destYBottomRightLayer0            = procParams->rcOutputSurfaceRegion.Height - 1;
    cmd.m_mediaWalkerData.m_dword64.m_mainVideoXScalingStepLeft         = 1.0F;

    CODECHAL_DECODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &cmd.m_mediaWalkerData,
        kernelState->dwCurbeOffset,
        cmd.m_byteSize));

    return eStatus;
}

bool FieldScalingInterface::IsFieldScalingSupported(
    CODECHAL_DECODE_PROCESSING_PARAMS  *params)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (!params || !params->pInputSurface || !params->pOutputSurface)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid Parameters");
        return false;
    }

    MOS_SURFACE *srcSurface  = params->pInputSurface;
    MOS_SURFACE *destSurface = params->pOutputSurface;

    // Check input size
    if (!MOS_WITHIN_RANGE(srcSurface->dwWidth, m_minInputWidth, m_maxInputWidth)     ||
        !MOS_WITHIN_RANGE(srcSurface->dwHeight, m_minInputHeight, m_maxInputHeight))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Input Resolution '0x%08x'x'0x%08x' for field scaling.", srcSurface->dwWidth, srcSurface->dwHeight);
        return false;
    }

    // Check input format
    if (srcSurface->Format != Format_NV12)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Input Format '0x%08x' for field scaling.", srcSurface->Format);
        return false;
    }

    // Check input region rectangles
    if ((params->rcInputSurfaceRegion.Width > srcSurface->dwWidth) ||
        (params->rcInputSurfaceRegion.Height > srcSurface->dwHeight))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Input region is out of bound for field scaling.");
        return false;
    }

    // Check output format
    if (destSurface->Format != Format_NV12 && destSurface->Format != Format_YUY2)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Output Format '0x%08x' for field scaling.", destSurface->Format);
        return false;
    }

    // Check output size
    if (!MOS_WITHIN_RANGE(destSurface->dwWidth, m_minInputWidth, m_maxInputWidth)     ||
        !MOS_WITHIN_RANGE(destSurface->dwHeight, m_minInputHeight, m_maxInputHeight))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported Output Resolution '0x%08x'x'0x%08x' for field scaling.", destSurface->dwWidth, destSurface->dwHeight);
        return false;
    }

    // Check output region rectangles
    if ((params->rcOutputSurfaceRegion.Width > destSurface->dwWidth) ||
        (params->rcOutputSurfaceRegion.Height > destSurface->dwHeight))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Output region is out of bound for field scaling.");
        return false;
    }

    // Check scaling ratio
    // Scaling range is [0.125, 1] for both X and Y direction.
    float scaleX = (float)params->rcOutputSurfaceRegion.Width / (float)params->rcInputSurfaceRegion.Width;
    float scaleY = (float)params->rcOutputSurfaceRegion.Height / (float)params->rcInputSurfaceRegion.Height;

    if (!MOS_WITHIN_RANGE(scaleX, m_minScaleRatio, m_maxScaleRatio) ||
        !MOS_WITHIN_RANGE(scaleY, m_minScaleRatio, m_maxScaleRatio))
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Scaling factor not supported by field scaling.");
        return false;
    }

    return true;
}

MOS_STATUS FieldScalingInterface::InitializeKernelState(
    CodechalDecode                      *decoder,
    CodechalHwInterface                 *hwInterface,
    PMOS_INTERFACE                      osInterface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(hwInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(osInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(hwInterface->GetMiInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(hwInterface->GetRenderInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(hwInterface->GetRenderInterface()->GetHwCaps());
    CODECHAL_DECODE_CHK_NULL_RETURN(hwInterface->GetRenderInterface()->m_stateHeapInterface);

    this->m_decoder         = decoder;
    m_osInterface           = osInterface;
    m_hwInterface           = hwInterface;
    m_renderInterface       = m_hwInterface->GetRenderInterface();
    m_stateHeapInterface    = m_renderInterface->m_stateHeapInterface;
    m_miInterface           = m_hwInterface->GetMiInterface();

    MHW_KERNEL_STATE *kernelState;
    for (auto krnIdx = 0; krnIdx < stateMax; krnIdx++)
    {
        kernelState = &m_kernelStates[krnIdx];

        kernelState->KernelParams.iThreadCount   = m_renderInterface->GetHwCaps()->dwMaxThreads;
        kernelState->KernelParams.iBTCount       = numSurfaces;
        kernelState->KernelParams.iBlockWidth    = CODECHAL_MACROBLOCK_WIDTH;
        kernelState->KernelParams.iBlockHeight   = CODECHAL_MACROBLOCK_HEIGHT;
        kernelState->KernelParams.iIdCount       = 1;
        kernelState->KernelParams.iSamplerCount  = m_samplerNum;
        kernelState->KernelParams.iSamplerLength = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdSampleState();

        kernelState->dwCurbeOffset        = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelState->dwSamplerOffset      =
            kernelState->dwCurbeOffset +
            MOS_ALIGN_CEIL(kernelState->KernelParams.iCurbeLength, m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
        kernelState->dwKernelBinaryOffset = 0;

        MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelState->KernelParams.iBTCount,
            &kernelState->dwSshSize,
            &kernelState->dwBindingTableSize));

        m_dshSize[krnIdx] =
            m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData() +
            MOS_ALIGN_CEIL(kernelState->KernelParams.iCurbeLength, m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment()) +
            kernelState->KernelParams.iSamplerLength * m_samplerNum;

        MHW_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(
            m_stateHeapInterface,
            kernelState));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &m_syncObject));

    return eStatus;
}

MOS_STATUS FieldScalingInterface::SetupMediaVfe(
    PMOS_COMMAND_BUFFER  cmdBuffer,
    MHW_KERNEL_STATE     *kernelState)
{
    MHW_VFE_PARAMS vfeParams = {};

    vfeParams.pKernelState = kernelState;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_renderInterface->AddMediaVfeCmd(cmdBuffer, &vfeParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FieldScalingInterface::DoFieldScaling(
    CODECHAL_DECODE_PROCESSING_PARAMS   *procParams,
    MOS_GPU_CONTEXT                     renderContext,
    bool                                disableDecodeSyncLock,
    bool                                disableLockForTranscode)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(procParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(procParams->pInputSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(procParams->pOutputSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    MOS_SYNC_PARAMS syncParams;
    syncParams                  = g_cInitSyncParams;
    syncParams.GpuContext       = m_decoder->GetVideoContext();
    syncParams.presSyncResource = &m_syncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

    syncParams                  = g_cInitSyncParams;
    syncParams.GpuContext       = renderContext;
    syncParams.presSyncResource = &m_syncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

    FieldScalingKernelStateIdx  kernelStateIdx;
    if (procParams->pOutputSurface->Format == Format_NV12)
    {
        kernelStateIdx = stateNv12;
    }
    else if (procParams->pOutputSurface->Format == Format_YUY2)
    {
        kernelStateIdx = stateYuy2;
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_osInterface->pfnSetGpuContext(m_osInterface, renderContext);
    m_osInterface->pfnResetOsStates(m_osInterface);

    MHW_KERNEL_STATE *kernelState = &m_kernelStates[kernelStateIdx];

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        m_stateHeapInterface,
        kernelState->KernelParams.iBTCount));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        m_dshSize[kernelStateIdx],
        false,
        m_decoder->GetDecodeStatusBuf()->m_swStoreData));

    // Initialize DSH kernel region
    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    memset(&idParams, 0, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        kernelState->KernelParams.iIdCount,
        &idParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetCurbeFieldScaling(
        kernelState,
        procParams));

    MHW_SAMPLER_STATE_PARAM samplerParams[m_samplerNum];
    memset(&samplerParams[0], 0, sizeof(MHW_SAMPLER_STATE_PARAM) * m_samplerNum);
    samplerParams[0].bInUse                 = false;
    samplerParams[0].pKernelState           = kernelState;
    for (uint32_t index = 1; index < m_samplerNum - 1; index++)
    {
        samplerParams[index].bInUse       = true;
        samplerParams[index].pKernelState = kernelState;
    }
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetSamplerState(
        m_stateHeapInterface,
        nullptr,
        &samplerParams[0]));

    // Send HW commands (including SSH)
    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    MHW_PIPE_CONTROL_PARAMS pipeControlParams;
    memset(&pipeControlParams, 0, sizeof(pipeControlParams));

    // Send command buffer header at the beginning (OS dependent)
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_decoder->SendPrologWithFrameTracking(
        &cmdBuffer, true));

    if (m_renderInterface->GetL3CacheConfig()->bL3CachingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_renderInterface->SetL3Cache(&cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_renderInterface->EnablePreemption(&cmdBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_renderInterface->AddPipelineSelectCmd(&cmdBuffer, false));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    // Source Surface
    // Top Field
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    memset(&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface               = true;
    surfaceCodecParams.bUseHalfHeight             = true;
    surfaceCodecParams.bUseUVPlane                = true;
    surfaceCodecParams.psSurface                  = procParams->pInputSurface;
    surfaceCodecParams.dwCacheabilityControl      = surfaceCacheabilityControlBitsFromGtt;

    surfaceCodecParams.dwBindingTableOffset       = fieldTopSrcY;
    surfaceCodecParams.dwUVBindingTableOffset     = fieldTopSrcUV;
    surfaceCodecParams.dwVerticalLineStride       = 1;
    surfaceCodecParams.dwVerticalLineStrideOffset = 0;

    surfaceCodecParams.bForceChromaFormat         = true;
    surfaceCodecParams.ChromaType                 = MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UNORM;

    PMOS_INTERFACE osInterface = m_osInterface;
    CodecHalGetResourceInfo(osInterface,surfaceCodecParams.psSurface);
    
#ifdef _MMC_SUPPORTED
    if (m_mmcState)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
    }
#endif

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        &cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Bottom Field
    surfaceCodecParams.dwBindingTableOffset       = fieldBotSrcY;
    surfaceCodecParams.dwUVBindingTableOffset     = fieldBotSrcUV;
    surfaceCodecParams.dwVerticalLineStrideOffset = 1;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        &cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Destination Surface (NV12 & YUY2, RGB8 support is not yet implemented)
    memset(&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface               = true;
    surfaceCodecParams.psSurface                  = procParams->pOutputSurface;
    surfaceCodecParams.bMediaBlockRW              = true;
    surfaceCodecParams.bIsWritable                = true;
    surfaceCodecParams.dwBindingTableOffset       = dstY;
    surfaceCodecParams.dwUVBindingTableOffset     = dstUV;
    surfaceCodecParams.dwCacheabilityControl      = surfaceCacheabilityControlBitsFromGtt;

    if (procParams->pOutputSurface->Format == Format_NV12)
    {
        surfaceCodecParams.bUseUVPlane              = true;
        surfaceCodecParams.bForceChromaFormat       = true;
        surfaceCodecParams.ChromaType               = MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UNORM;
    }

#ifdef _MMC_SUPPORTED
    if (m_mmcState)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
    }
#endif

    CodecHalGetResourceInfo(osInterface,surfaceCodecParams.psSurface);
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        &cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    MHW_STATE_BASE_ADDR_PARAMS stateBaseAddrParams;
    memset(&stateBaseAddrParams, 0, sizeof(stateBaseAddrParams));
    MOS_RESOURCE *dsh = nullptr, *ish = nullptr;
    CODECHAL_DECODE_CHK_NULL_RETURN(dsh = kernelState->m_dshRegion.GetResource());
    CODECHAL_DECODE_CHK_NULL_RETURN(ish = kernelState->m_ishRegion.GetResource());
    stateBaseAddrParams.presDynamicState = dsh;
    stateBaseAddrParams.dwDynamicStateSize = kernelState->m_dshRegion.GetHeapSize();
    stateBaseAddrParams.presInstructionBuffer = ish;
    stateBaseAddrParams.dwInstructionBufferSize = kernelState->m_ishRegion.GetHeapSize();
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_renderInterface->AddStateBaseAddrCmd(&cmdBuffer, &stateBaseAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetupMediaVfe(&cmdBuffer, kernelState));

    MHW_CURBE_LOAD_PARAMS curbeLoadParams;
    memset(&curbeLoadParams, 0, sizeof(curbeLoadParams));
    curbeLoadParams.pKernelState    = kernelState;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_renderInterface->AddMediaCurbeLoadCmd(&cmdBuffer, &curbeLoadParams));

    MHW_ID_LOAD_PARAMS idLoadParams;
    memset(&idLoadParams, 0, sizeof(idLoadParams));
    idLoadParams.pKernelState       = kernelState;
    idLoadParams.dwNumKernelsLoaded = 1;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_renderInterface->AddMediaIDLoadCmd(&cmdBuffer, &idLoadParams));

    uint32_t resolutionX = MOS_ROUNDUP_DIVIDE(procParams->rcOutputSurfaceRegion.Width, 16);
    uint32_t resolutionY = MOS_ROUNDUP_DIVIDE(procParams->rcOutputSurfaceRegion.Height, 16);

    CODECHAL_WALKER_CODEC_PARAMS            walkerCodecParams;
    memset(&walkerCodecParams, 0, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode            = MHW_WALKER_MODE_DUAL;
    walkerCodecParams.dwResolutionX         = resolutionX;
    walkerCodecParams.dwResolutionY         = resolutionY;
    walkerCodecParams.bNoDependency         = true;     // raster scan mode

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_renderInterface->AddMediaObjectWalkerCmd(&cmdBuffer, &walkerParams));

    // Check if destination surface needs to be synchronized, before command buffer submission
    syncParams                          = g_cInitSyncParams;
    syncParams.GpuContext               = renderContext;
    syncParams.presSyncResource         = &procParams->pOutputSurface->OsResource;
    syncParams.bReadOnly                = false;
    syncParams.bDisableDecodeSyncLock   = disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = disableLockForTranscode;

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

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
        m_stateHeapInterface));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_decoder->GetRenderContextUsesNullHw()));

    if (m_decoder->IsStatusQueryReportingEnabled())
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_decoder->ResetStatusReport(m_decoder->GetRenderContextUsesNullHw()));
    }

    m_osInterface->pfnSetGpuContext(m_osInterface, m_decoder->GetVideoContext());

    return (MOS_STATUS)eStatus;
}

MOS_STATUS FieldScalingInterface::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    if (m_mmcState == nullptr)
    {
        m_mmcState = MOS_New(CodecHalMmcState, m_hwInterface);
        CODECHAL_DECODE_CHK_NULL_RETURN(m_mmcState);
    }
#endif
    return MOS_STATUS_SUCCESS;
}
