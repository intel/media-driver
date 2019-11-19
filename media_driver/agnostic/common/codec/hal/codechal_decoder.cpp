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
//! \file     codechal_decoder.cpp
//! \brief    Implements the decode interface for CodecHal.
//! \details  The decode interface is further sub-divided by standard, this file is for the base interface which is shared by all decode standards.
//!

#include "codechal_decoder.h"
#include "codechal_secure_decode_interface.h"
#include "mos_solo_generic.h"
#include "codechal_debug.h"
#include "codechal_decode_histogram.h"

#ifdef _HEVC_DECODE_SUPPORTED
#include "codechal_decode_hevc.h"
#endif

#ifdef _VP9_DECODE_SUPPORTED
#include "codechal_decode_vp9.h"
#endif

#ifdef _HYBRID_HEVC_DECODE_SUPPORTED
#include "codechal_decode_hybrid_hevc.h"
#endif

#ifdef _HYBRID_VP9_DECODE_SUPPORTED
#include "codechal_decode_hybrid_vp9.h"
#endif

#ifdef _AVC_DECODE_SUPPORTED
#include "codechal_decode_avc.h"
#endif

#ifdef _JPEG_DECODE_SUPPORTED
#include "codechal_decode_jpeg.h"
#endif

#ifdef _VC1_DECODE_SUPPORTED
#include "codechal_decode_vc1.h"
#endif

#ifdef _VP8_DECODE_SUPPORTED
#include "codechal_decode_vp8.h"
#endif

#ifdef _MPEG2_DECODE_SUPPORTED
#include "codechal_decode_mpeg2.h"
#endif

#if USE_CODECHAL_DEBUG_TOOL
#include <sstream>
#include <fstream>
#include "codechal_debug.h"
#endif

MOS_STATUS CodechalDecode::AllocateBuffer(
    PMOS_RESOURCE   resource,
    uint32_t        size,
    const char      *name,
    bool            initialize,
    uint8_t         value,
    bool            bPersistent)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(resource);

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type            = MOS_GFXRES_BUFFER;
    allocParams.TileType        = MOS_TILE_LINEAR;
    allocParams.Format          = Format_Buffer;
    allocParams.dwBytes         = size;
    allocParams.pBufName        = name;
    allocParams.bIsPersistent   = bPersistent;

    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        resource),
        "Failed to allocate %s.", name);

    if (initialize)
    {
        CodechalResLock ResourceLock(m_osInterface, resource);
        auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);
        CODECHAL_DECODE_CHK_NULL_RETURN(data);

        MOS_FillMemory(data, size, value);
    }

    return eStatus;
}

MOS_STATUS CodechalDecode::AllocateSurface(
    PMOS_SURFACE    surface,
    uint32_t        width,
    uint32_t        height,
    const char      *name,
    MOS_FORMAT      format,
    bool            isCompressible)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_DECODE_CHK_NULL_RETURN(surface);

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type        = MOS_GFXRES_2D;
    allocParams.TileType    = MOS_TILE_Y;
    allocParams.Format      = format;
    allocParams.dwWidth     = width;
    allocParams.dwHeight    = height;
    allocParams.dwArraySize = 1;
    allocParams.pBufName    = name;
    allocParams.bIsCompressible = isCompressible;

    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        &surface->OsResource),
        "Failed to allocate %s.", name);

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
        m_osInterface,
        surface));

    return eStatus;
}

MOS_STATUS CodechalDecode::HucCopy(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMOS_RESOURCE       src,
    PMOS_RESOURCE       dst,
    uint32_t            copyLength,
    uint32_t            copyInputOffset,
    uint32_t            copyOutputOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(src);
    CODECHAL_DECODE_CHK_NULL_RETURN(dst);

    CodechalHucStreamoutParams hucStreamOutParams;
    MOS_ZeroMemory(&hucStreamOutParams, sizeof(hucStreamOutParams));

    // Ind Obj Addr command
    hucStreamOutParams.dataBuffer                 = src;
    hucStreamOutParams.dataSize                   = copyLength + copyInputOffset;
    hucStreamOutParams.dataOffset                 = MOS_ALIGN_FLOOR(copyInputOffset, MHW_PAGE_SIZE);
    hucStreamOutParams.streamOutObjectBuffer      = dst;
    hucStreamOutParams.streamOutObjectSize        = copyLength + copyOutputOffset;
    hucStreamOutParams.streamOutObjectOffset      = MOS_ALIGN_FLOOR(copyOutputOffset, MHW_PAGE_SIZE);

    // Stream object params
    hucStreamOutParams.indStreamInLength          = copyLength;
    hucStreamOutParams.inputRelativeOffset        = copyInputOffset - hucStreamOutParams.dataOffset;
    hucStreamOutParams.outputRelativeOffset       = copyOutputOffset - hucStreamOutParams.streamOutObjectOffset;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->PerformHucStreamOut(
        &hucStreamOutParams,
        cmdBuffer));

    return eStatus;
}

uint32_t CodechalDecode::LinearToYTiledAddress(
    uint32_t x,
    uint32_t y,
    uint32_t pitch)
{
    uint32_t tileW = 128;
    uint32_t tileH = 32;

    uint32_t tileSize = tileW * tileH;

    uint32_t rowSize = (pitch / tileW) * tileSize;

    uint32_t xOffWithinTile = x % tileW;
    uint32_t yOffWithinTile = y % tileH;

    uint32_t tileNumberInX = x / tileW;
    uint32_t tileNumberInY = y / tileH;

    uint32_t tileOffset =
        rowSize * tileNumberInY +
        tileSize * tileNumberInX +
        tileH * 16 * (xOffWithinTile / 16) +
        yOffWithinTile * 16 +
        (xOffWithinTile % 16);

    return tileOffset;
}

CodechalDecode::CodechalDecode (
    CodechalHwInterface        *hwInterface,
    CodechalDebugInterface      *debugInterface,
    PCODECHAL_STANDARD_INFO     standardInfo):
    Codechal(hwInterface, debugInterface)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_dummyReference, sizeof(MOS_SURFACE));

    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(hwInterface->GetOsInterface());
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(hwInterface->GetMiInterface());
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(hwInterface->GetCpInterface());
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(standardInfo);

    m_mfxInterface      = hwInterface->GetMfxInterface();
    m_hcpInterface      = hwInterface->GetHcpInterface();
    m_hucInterface      = hwInterface->GetHucInterface();
    m_vdencInterface    = hwInterface->GetVdencInterface();
    m_miInterface       = hwInterface->GetMiInterface();
    m_cpInterface       = hwInterface->GetCpInterface();

    PLATFORM platform;
    m_osInterface->pfnGetPlatform(m_osInterface, &platform);
    m_waTable   = m_osInterface->pfnGetWaTable(m_osInterface);
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_waTable);
    m_skuTable  = m_osInterface->pfnGetSkuTable(m_osInterface);
    CODECHAL_DECODE_CHK_NULL_NO_STATUS_RETURN(m_skuTable);

    m_mode              = standardInfo->Mode;
    m_isHybridDecoder   = standardInfo->bIsHybridCodec ? true : false;
}

MOS_STATUS CodechalDecode::SetGpuCtxCreatOption(
    CodechalSetting *          codecHalSetting)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_UNUSED(codecHalSetting);

    m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

    return eStatus;
}

MOS_STATUS CodechalDecode::CreateGpuContexts(
    CodechalSetting *codecHalSettings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(codecHalSettings);

    MHW_VDBOX_GPUNODE_LIMIT gpuNodeLimit;
    gpuNodeLimit.bHuCInUse = false;
    gpuNodeLimit.bHcpInUse = m_hcpInUse;
    gpuNodeLimit.bSfcInUse = IsSfcInUse(codecHalSettings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->FindGpuNodeToUse(
        &gpuNodeLimit));

    m_videoGpuNode = (MOS_GPU_NODE)(gpuNodeLimit.dwGpuNodeToUse);

    CODECHAL_UPDATE_VDBOX_USER_FEATURE(m_videoGpuNode);
    CodecHalDecodeMapGpuNodeToGpuContex(m_videoGpuNode, m_videoContext, false);

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetGpuCtxCreatOption(codecHalSettings));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        m_videoContext,
        m_videoGpuNode,
        m_gpuCtxCreatOpt));

    // Create Video2 Context for MPEG2 WA and JPEG incomplete bitstream & VP9 / HEVC DRC support
    // For decode device, we use VDBOX0 always for the WA context
    // For AVC,VC1,VP9, use WA context for huc stream out copy
    if (Mos_Solo_IsInUse(m_osInterface))
    {
        Mos_Solo_DecodeMapGpuNodeToGpuContex(MOS_GPU_NODE_VIDEO, m_videoContextForWa, true, false);
    }
    else
    {
        CodecHalDecodeMapGpuNodeToGpuContex(MOS_GPU_NODE_VIDEO, m_videoContextForWa, true);
    }

    MOS_GPUCTX_CREATOPTIONS createOption;
    eStatus = (MOS_STATUS)m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        m_videoContextForWa,
        MOS_GPU_NODE_VIDEO,
        &createOption);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // use context Video1. It should be valid
        if (Mos_Solo_IsInUse(m_osInterface))
        {
            Mos_Solo_DecodeMapGpuNodeToGpuContex(MOS_GPU_NODE_VIDEO, m_videoContextForWa, false, false);
        }
        else
        {
            CodecHalDecodeMapGpuNodeToGpuContex(MOS_GPU_NODE_VIDEO, m_videoContextForWa, false);
        }
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnIsGpuContextValid(
            m_osInterface,
            m_videoContextForWa));
    }

    // Do not need to create render context here, it will be created by standard specific decoder

    return eStatus;
}

// Decoder Public Interface Functions
MOS_STATUS CodechalDecode::Allocate (CodechalSetting * codecHalSettings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(Codechal::Allocate(codecHalSettings));

    m_standard                  = codecHalSettings->standard;
    m_mode                      = codecHalSettings->mode;
    m_disableDecodeSyncLock     = codecHalSettings->disableDecodeSyncLock ? true : false;
    m_disableLockForTranscode   = MEDIA_IS_WA(m_waTable, WaDisableLockForTranscodePerf);

    // register cp params via codechal_Setting
    m_cpInterface->RegisterParams(codecHalSettings->GetCpParams());

    {
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.u32Data = MOS_STATUS_REPORT_DEFAULT;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_STATUS_REPORTING_ENABLE_ID,
            &userFeatureData);
        m_statusQueryReportingEnabled = (userFeatureData.u32Data) ? true : false;

#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_statusQueryReportingEnabled)
        {
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_STREAM_OUT_ENABLE_ID,
                &userFeatureData);
            m_streamOutEnabled = (userFeatureData.u32Data) ? true : false;

        }

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_FE_BE_TIMING,
            &userFeatureData);
        m_perfFEBETimingEnabled = userFeatureData.bData;

#endif // _DEBUG || _RELEASE_INTERNAL
    }

//#if (_DEBUG || _RELEASE_INTERNAL)
//#ifdef _MD5_DEBUG_SUPPORTED
//    {
//    // For multi-thread decoder case, MD5 kernel will share the same context with hybrid decoder.
//    // And it will be initialized in decoder worker thread function.
//    if ((!m_isHybridDecoder || (m_isHybridDecoder && !IsFrameMTEnabled())) &&
//        m_debugInterface != nullptr)
//    {
//        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgInitMD5Context(
//            m_debugInterface,
//            nullptr));
//        if (m_debugInterface->pMD5Context)
//        {
//            m_debugInterface->bMD5DDIThreadExecute = true;
//        }
//    }
//    }
//#endif // _MD5_DEBUG_SUPPORTED
//#endif // _DEBUG || _RELEASE_INTERNAL

    // Set decoder running flag to OS context so that VPP driver can query this flag and use
    // this flag to decide if disable VPP DNDI feature in VEBOX for power saving.
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetHybridDecoderRunningFlag(
        m_osInterface,
        m_isHybridDecoder));

    // eStatus Query reporting
    if (m_statusQueryReportingEnabled)
    {
        uint32_t statusBufferSize = sizeof(CodechalDecodeStatus) * CODECHAL_DECODE_STATUS_NUM + sizeof(uint32_t) * 2;

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &m_decodeStatusBuf.m_statusBuffer,
            statusBufferSize,
            "StatusQueryBuffer"),
            "Failed to allocate decode eStatus buffer.");

        MOS_LOCK_PARAMS lockFlagsNoOverWrite;
        MOS_ZeroMemory(&lockFlagsNoOverWrite, sizeof(MOS_LOCK_PARAMS));
        lockFlagsNoOverWrite.WriteOnly = 1;
        lockFlagsNoOverWrite.NoOverWrite = 1;

        uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_decodeStatusBuf.m_statusBuffer,
            &lockFlagsNoOverWrite);

        MOS_ZeroMemory(data, statusBufferSize);
        m_decodeStatusBuf.m_data            = (uint32_t *)data;
        m_decodeStatusBuf.m_decodeStatus    = (CodechalDecodeStatus *)(data + sizeof(uint32_t) * 2);
        m_decodeStatusBuf.m_currIndex       = 0;
        m_decodeStatusBuf.m_firstIndex      = 0;
        m_decodeStatusBuf.m_swStoreData     = 1;

        m_decodeStatusBuf.m_storeDataOffset             = 0;
        m_decodeStatusBuf.m_decErrorStatusOffset        = CODECHAL_OFFSETOF(CodechalDecodeStatus, m_mmioErrorStatusReg);
        m_decodeStatusBuf.m_decFrameCrcOffset           = CODECHAL_OFFSETOF(CodechalDecodeStatus, m_mmioFrameCrcReg);
        m_decodeStatusBuf.m_decMBCountOffset            = CODECHAL_OFFSETOF(CodechalDecodeStatus, m_mmioMBCountReg);
        m_decodeStatusBuf.m_csEngineIdOffset            = CODECHAL_OFFSETOF(CodechalDecodeStatus, m_mmioCsEngineIdReg);
        m_decodeStatusBuf.m_hucErrorStatus2MaskOffset   = CODECHAL_OFFSETOF(CodechalDecodeStatus, m_hucErrorStatus2);
        m_decodeStatusBuf.m_hucErrorStatus2RegOffset    = CODECHAL_OFFSETOF(CodechalDecodeStatus, m_hucErrorStatus2) + sizeof(uint32_t);
        m_decodeStatusBuf.m_hucErrorStatusMaskOffset    = CODECHAL_OFFSETOF(CodechalDecodeStatus, m_hucErrorStatus);
        m_decodeStatusBuf.m_hucErrorStatusRegOffset     = CODECHAL_OFFSETOF(CodechalDecodeStatus, m_hucErrorStatus) + sizeof(uint32_t);

        // Set IMEM Loaded bit (in DW1) to 1 by default in the first status buffer
        // This bit will be changed later after storing register
        if (m_hucInterface)
        {
            m_decodeStatusBuf.m_decodeStatus->m_hucErrorStatus2 = (uint64_t)m_hucInterface->GetHucStatus2ImemLoadedMask() << 32;
        }

        //if kernels are used update the media state heap with status pointers to keep track of when buffers are done
        if (m_hwInterface->GetRenderInterface() != nullptr &&
            m_hwInterface->GetRenderInterface()->m_stateHeapInterface != nullptr)
        {
            PMHW_STATE_HEAP_INTERFACE pStateHeapInterface =
                m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

            CODECHAL_DECODE_CHK_STATUS_RETURN(pStateHeapInterface->pfnSetCmdBufStatusPtr(
                pStateHeapInterface,
                m_decodeStatusBuf.m_data));
        }

        // StreamOut Buffer Allocation
        if (m_streamOutEnabled)
        {
            uint32_t numMacroblocks =
                (codecHalSettings->height / CODECHAL_MACROBLOCK_HEIGHT) *
                (codecHalSettings->width / CODECHAL_MACROBLOCK_WIDTH);
            uint32_t streamOutBufSize = MOS_ALIGN_CEIL(numMacroblocks * CODEC_SIZE_MFX_STREAMOUT_DATA, 64);

            m_streamOutCurrBufIdx = 0;

            for (auto i = 0; i < CODECHAL_DECODE_NUM_STREAM_OUT_BUFFERS; i++)
            {
                CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                    &(m_streamOutBuffer[i]),
                    streamOutBufSize,
                    "StreamOutBuffer",
                    true,
                    0),
                    "Failed to allocate streamout buffer.");

                m_streamOutCurrStatusIdx[i] = CODECHAL_DECODE_STATUS_NUM;
            }
        }
    }

    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
        &m_predicationBuffer,
        sizeof(uint32_t),
        "PredicationBuffer",
        true,
        0),
        "Failed to allocate predication buffer.");

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateStandard(codecHalSettings));

    if(!m_isHybridDecoder)
    {
        // Create Video Contexts
        CODECHAL_DECODE_CHK_STATUS_RETURN(CreateGpuContexts(codecHalSettings));
        // Set Vdbox index in use
        m_vdboxIndex = (m_videoGpuNode == MOS_GPU_NODE_VIDEO2)? MHW_VDBOX_NODE_2 : MHW_VDBOX_NODE_1;

        // Set FrameCrc reg offset
        if (m_standard == CODECHAL_HEVC)
        {
            m_hcpFrameCrcRegOffset = m_hcpInterface->GetMmioRegisters(m_vdboxIndex)->hcpFrameCrcRegOffset;
        }
    }

    if (!m_mmc)
    {
        m_mmc = MOS_New(CodecHalMmcState, m_hwInterface);
    }

    m_secureDecoder = Create_SecureDecodeInterface(codecHalSettings, m_hwInterface); 

#ifdef _DECODE_PROCESSING_SUPPORTED
    m_downsamplingHinted = codecHalSettings->downsamplingHinted ? true : false;
    if (CodecHalIsEnableFieldScaling(CODECHAL_FUNCTION_DECODE, m_standard, m_downsamplingHinted))
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_fieldScalingInterface);
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_fieldScalingInterface->InitializeKernelState(
            this,
            m_hwInterface,
            m_osInterface));
    }
#endif

    m_renderContextUsesNullHw = m_useNullHw[m_renderContext];
    if(!m_isHybridDecoder)
    {
        m_videoContextUsesNullHw = m_useNullHw[m_videoContext];
        m_videoContextForWaUsesNullHw = m_useNullHw[m_videoContextForWa];

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            m_videoContext));
    }

    if (!m_perfProfiler)
    {
        m_perfProfiler = MediaPerfProfiler::Instance();
        CODECHAL_DECODE_CHK_NULL_RETURN(m_perfProfiler);

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->Initialize((void*)this, m_osInterface));
    }

    return eStatus;
}

MOS_STATUS CodechalDecode::AllocateRefSurfaces(
    uint32_t                    allocWidth,
    uint32_t                    allocHeight,
    MOS_FORMAT                  format)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (allocWidth == 0 || allocHeight == 0)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid Downsampling Reference Frame Width or Height !");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_refSurfaces = (MOS_SURFACE*)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE) * m_refFrmCnt);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_refSurfaces);

    CODECHAL_DEBUG_TOOL(
        m_downsampledSurfaces = (MOS_SURFACE*)MOS_AllocAndZeroMemory(m_refFrmCnt * sizeof(MOS_SURFACE));
    )

    for (uint32_t i = 0; i < m_refFrmCnt; i++)
    {
        eStatus = AllocateSurface(
            &m_refSurfaces[i],
            allocWidth,
            allocHeight,
            "DownsamplingRefSurface",
            format,
            CodecHalMmcState::IsMmcEnabled());

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate decode downsampling reference surface.");
            DeallocateRefSurfaces();
            return eStatus;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecode::RefSurfacesResize(
    uint32_t     frameIdx,
    uint32_t     width,
    uint32_t     height,
    MOS_FORMAT   format)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_DECODE_FUNCTION_ENTER;
  
    if (m_refSurfaces[frameIdx].dwWidth == 0 || m_refSurfaces[frameIdx].dwHeight == 0)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid Downsampling Reference Frame Width or Height !");
        return MOS_STATUS_INVALID_PARAMETER;
    }
  
    DeallocateSpecificRefSurfaces(frameIdx);
  
    eStatus = AllocateSurface(
        &m_refSurfaces[frameIdx],
        width,
        height,
        "DownsamplingRefSurface",
        format,
        CodecHalMmcState::IsMmcEnabled());
  
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Failed to allocate decode downsampling reference surface.");
        DeallocateRefSurfaces();
        return eStatus;
    }
  
    return MOS_STATUS_SUCCESS;
}

void CodechalDecode::DeallocateSpecificRefSurfaces(uint32_t frameIdx)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_refSurfaces != nullptr && frameIdx != 0)
    {
        if (!Mos_ResourceIsNull(&m_refSurfaces[frameIdx].OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_refSurfaces[frameIdx].OsResource);
        }
    }
}

void CodechalDecode::DeallocateRefSurfaces()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_refSurfaces != nullptr && m_refFrmCnt != 0)
    {
        CODECHAL_DEBUG_TOOL(
            MOS_FreeMemAndSetNull(m_downsampledSurfaces);
        )

        for (uint32_t i = 0; i < m_refFrmCnt; i++)
        {
            if (!Mos_ResourceIsNull(&m_refSurfaces[i].OsResource))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_refSurfaces[i].OsResource);
            }
        }

        MOS_FreeMemory(m_refSurfaces);
        m_refSurfaces = nullptr;
    }
}

MOS_STATUS CodechalDecode::SetDummyReference()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (MEDIA_IS_WA(m_waTable, WaDummyReference))
    {
        // If can't find valid dummy reference, create one or use current decode output surface
        if (Mos_ResourceIsNull(&m_dummyReference.OsResource))
        {
            // If MMC enabled
            if (m_mmc != nullptr && m_mmc->IsMmcEnabled() && 
                !m_mmc->IsMmcExtensionEnabled() && 
                m_decodeParams.m_destSurface->bIsCompressed)
            {
                if (m_mode == CODECHAL_DECODE_MODE_HEVCVLD)
                {
                    eStatus = AllocateSurface(
                        &m_dummyReference,
                        m_decodeParams.m_destSurface->dwWidth,
                        m_decodeParams.m_destSurface->dwHeight,
                        "dummy reference resource",
                        m_decodeParams.m_destSurface->Format,
                        m_decodeParams.m_destSurface->bIsCompressed);

                    if (eStatus != MOS_STATUS_SUCCESS)
                    {
                        CODECHAL_DECODE_ASSERTMESSAGE("Failed to create dummy reference!");
                        return eStatus;
                    }
                    else
                    {
                        m_dummyReferenceStatus = CODECHAL_DUMMY_REFERENCE_ALLOCATED;
                        CODECHAL_DECODE_VERBOSEMESSAGE("Dummy reference is created!");
                    }
                }
            }
            else    // Use decode output surface as dummy reference
            {
                m_dummyReference.OsResource = m_decodeParams.m_destSurface->OsResource;
                m_dummyReferenceStatus = CODECHAL_DUMMY_REFERENCE_DEST_SURFACE;
            }
        }
    }

    return eStatus;
}

CodechalDecode::~CodechalDecode()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    Delete_SecureDecodeInterface(m_secureDecoder);
    m_secureDecoder = nullptr;

    if (m_mmc)
    {
        MOS_Delete(m_mmc);
        m_mmc = nullptr;
    }

    // Destroy decode histogram
    if (m_decodeHistogram != nullptr)
    {
        MOS_Delete(m_decodeHistogram);
        m_decodeHistogram = nullptr;
    }

    if (MEDIA_IS_SKU(m_skuTable, FtrVcs2) && (m_videoGpuNode < MOS_GPU_NODE_MAX))
    {
        // Destroy decode video node association
        m_osInterface->pfnDestroyVideoNodeAssociation(m_osInterface, m_videoGpuNode);
    }

    if (m_statusQueryReportingEnabled)
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &(m_decodeStatusBuf.m_statusBuffer));

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &(m_decodeStatusBuf.m_statusBuffer));

        if (m_streamOutEnabled)
        {
            for (auto i = 0; i < CODECHAL_DECODE_NUM_STREAM_OUT_BUFFERS; i++)
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &(m_streamOutBuffer[i]));
            }
        }
    }

    if (m_gpuCtxCreatOpt)
    {
        MOS_Delete(m_gpuCtxCreatOpt);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_predicationBuffer);

    DeallocateRefSurfaces();

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (CodecHalIsEnableFieldScaling(CODECHAL_FUNCTION_DECODE, m_standard, m_downsamplingHinted))
    {
        if (m_fieldScalingInterface != nullptr)
        {
            MOS_Delete(m_fieldScalingInterface);
            m_fieldScalingInterface = nullptr;
        }
    }
#endif

    if (m_perfProfiler)
    {
        MediaPerfProfiler::Destroy(m_perfProfiler, (void*)this, m_osInterface);
        m_perfProfiler = nullptr;
    }

    if (m_dummyReferenceStatus == CODECHAL_DUMMY_REFERENCE_ALLOCATED &&
        !Mos_ResourceIsNull(&m_dummyReference.OsResource))
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_dummyReference.OsResource);
    }
}

void CodechalDecode::CalcRequestedSpace(
    uint32_t       &requestedSize,
    uint32_t       &additionalSizeNeeded,
    uint32_t       &requestedPatchListSize)
{
    requestedSize = m_commandBufferSizeNeeded +
        (m_standardDecodeSizeNeeded * (m_decodeParams.m_numSlices + 1));
    requestedPatchListSize = m_commandPatchListSizeNeeded +
        (m_standardDecodePatchListSizeNeeded * (m_decodeParams.m_numSlices + 1));
    additionalSizeNeeded = COMMAND_BUFFER_RESERVED_SPACE;
}

MOS_STATUS CodechalDecode::VerifySpaceAvailable ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint32_t requestedSize = 0, additionalSizeNeeded = 0, requestedPatchListSize = 0;
    CalcRequestedSpace(requestedSize, additionalSizeNeeded, requestedPatchListSize);

    uint32_t primRequestedSize = RequestedSpaceSize(requestedSize);

    // Try a maximum of 3 attempts to request the required sizes from OS
    // OS could reset the sizes if necessary, therefore, requires to re-verify
    for (auto i = 0; i < 3; i++)
    {
        if (m_osInterface->bUsesPatchList || MEDIA_IS_SKU(m_skuTable, FtrMediaPatchless))
        {
            eStatus = (MOS_STATUS)m_osInterface->pfnVerifyPatchListSize(
                m_osInterface,
                requestedPatchListSize);

            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->ResizeCommandBufferAndPatchList(
                    0,
                    requestedPatchListSize));
            }
        }

        eStatus = (MOS_STATUS)m_osInterface->pfnVerifyCommandBufferSize(
            m_osInterface,
            primRequestedSize,
            0);

        if (eStatus == MOS_STATUS_SUCCESS)
        {
            break;
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->ResizeCommandBufferAndPatchList(
                primRequestedSize + additionalSizeNeeded,
                0));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(VerifyExtraSpace(requestedSize, additionalSizeNeeded));

    return eStatus;
}

MOS_STATUS CodechalDecode::EndFrame ()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(
        CodechalDecodeStatusReport * decodeStatusReport;
        auto     tempSurfNum         = m_debugInterface->m_decodeSurfDumpFrameNum;  // to handle BB_END data not written case
        uint16_t preIndex            = m_debugInterface->m_preIndex;
        uint32_t numReportsAvailable = (m_decodeStatusBuf.m_currIndex - preIndex) & (CODECHAL_DECODE_STATUS_NUM - 1);
        CODECHAL_DECODE_VERBOSEMESSAGE("NumReportsAvailable = %d", numReportsAvailable);

        for (uint32_t i = 0; i < numReportsAvailable; i++) {
            uint16_t index = (m_debugInterface->m_preIndex + i) % CODECHAL_DECODE_STATUS_NUM;
            decodeStatusReport =
                &(m_decodeStatusBuf.m_decodeStatus[index].m_decodeStatusReport);

            // record SurfDumpFrameNum to handle BB_END data not written case
            if (CodecHal_PictureIsFrame(decodeStatusReport->m_currDecodedPic) ||
                CodecHal_PictureIsInterlacedFrame(decodeStatusReport->m_currDecodedPic) ||
                decodeStatusReport->m_secondField)
            {
                tempSurfNum++;
            }

            if (m_standard == CODECHAL_HEVC     &&
                m_isHybridDecoder               &&
                (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrReferenceSurfaces)|| m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDecodeOutputSurface)))
            {
                CODECHAL_DECODE_CHK_STATUS_BREAK(DecodeGetHybridStatus(
                    m_decodeStatusBuf.m_decodeStatus, index, CODECHAL_STATUS_QUERY_START_FLAG));
            }

            auto tempFrameNum                      = m_debugInterface->m_bufferDumpFrameNum;
            auto tempPic                           = m_debugInterface->m_currPic;
            auto tempFrameType                     = m_debugInterface->m_frameType;
            m_debugInterface->m_bufferDumpFrameNum = m_debugInterface->m_decodeSurfDumpFrameNum;
            m_debugInterface->m_currPic            = decodeStatusReport->m_currDecodedPic;
            m_debugInterface->m_frameType          = decodeStatusReport->m_frameType;
            bool olpDump = false;

            MOS_SURFACE dstSurface;
            if ((CodecHal_PictureIsFrame(decodeStatusReport->m_currDecodedPic) ||
                 CodecHal_PictureIsInterlacedFrame(decodeStatusReport->m_currDecodedPic) ||
                 CodecHal_PictureIsField(decodeStatusReport->m_currDecodedPic)) && 
                (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDecodeBltOutput) || 
                 m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDecodeOutputSurface) || 
                 m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrStreamOut)))
            {
                MOS_ZeroMemory(&dstSurface, sizeof(dstSurface));
                dstSurface.Format       = Format_NV12;
                dstSurface.OsResource   = decodeStatusReport->m_currDecodedPicRes;
                CODECHAL_DECODE_CHK_STATUS_BREAK(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));

                m_debugInterface->DumpBltOutput(
                    &dstSurface,
                    CodechalDbgAttr::attrDecodeBltOutput);

                CODECHAL_DECODE_CHK_STATUS_BREAK(m_debugInterface->DumpYUVSurface(
                    &dstSurface,
                    CodechalDbgAttr::attrDecodeOutputSurface,
                    "DstSurf"));

                if (m_streamOutEnabled)
                {
                    //dump streamout buffer
                    CODECHAL_DECODE_CHK_STATUS_BREAK(m_debugInterface->DumpBuffer(
                        decodeStatusReport->m_streamOutBuf,
                        CodechalDbgAttr::attrStreamOut,
                        "StreamOut",
                        dstSurface.dwWidth));
                    // reset the capture status of the streamout buffer
                    m_streamOutCurrStatusIdx[decodeStatusReport->m_streamoutIdx] = CODECHAL_DECODE_STATUS_NUM;
                }

                olpDump = true;
            }

            MOS_USER_FEATURE_VALUE_DATA userFeatureData;
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_DECOMPRESS_DECODE_OUTPUT_ID,
                &userFeatureData);
            if (userFeatureData.u32Data)
            {
                CODECHAL_DECODE_VERBOSEMESSAGE("force ve decompress decode output");
                MOS_ZeroMemory(&dstSurface, sizeof(dstSurface));
                dstSurface.Format       = Format_NV12;
                dstSurface.OsResource   = decodeStatusReport->m_currDecodedPicRes;
                CODECHAL_DECODE_CHK_STATUS_BREAK(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));
                MOS_LOCK_PARAMS lockFlags {};
                lockFlags.ReadOnly = 1;
                lockFlags.TiledAsTiled = 1;
                lockFlags.NoDecompress = 0;
                m_osInterface->pfnLockResource(m_osInterface, &dstSurface.OsResource, &lockFlags);
                m_osInterface->pfnUnlockResource(m_osInterface, &dstSurface.OsResource);
            }

            if (m_standard == CODECHAL_VC1      &&
                decodeStatusReport->m_olpNeeded &&
                olpDump && 
                m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDecodeOutputSurface))
            {
                MOS_ZeroMemory(&dstSurface, sizeof(dstSurface));
                dstSurface.Format     = Format_NV12;
                dstSurface.OsResource = decodeStatusReport->m_deblockedPicResOlp;

                CODECHAL_DECODE_CHK_STATUS_BREAK(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &dstSurface,
                    CodechalDbgAttr::attrDecodeOutputSurface,
                    "OLP_DstSurf"));
            }

            if ((m_standard == CODECHAL_HEVC || m_standard == CODECHAL_VP9) &&
                (decodeStatusReport->m_currSfcOutputPicRes != nullptr) && 
                m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSfcOutputSurface))
            {
                MOS_ZeroMemory(&dstSurface, sizeof(dstSurface));
                dstSurface.Format     = Format_NV12;
                dstSurface.OsResource = *decodeStatusReport->m_currSfcOutputPicRes;

                CODECHAL_DECODE_CHK_STATUS_BREAK(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));

                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &dstSurface,
                    CodechalDbgAttr::attrSfcOutputSurface,
                    "SfcDstSurf"));
            }

            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_DECOMPRESS_DECODE_SFC_OUTPUT_ID,
                &userFeatureData);
            if (userFeatureData.u32Data)
            {
                CODECHAL_DECODE_VERBOSEMESSAGE("force ve decompress sfc output");
                MOS_ZeroMemory(&dstSurface, sizeof(dstSurface));
                dstSurface.Format       = Format_NV12;
                dstSurface.OsResource   = *decodeStatusReport->m_currSfcOutputPicRes;
                CODECHAL_DECODE_CHK_STATUS_BREAK(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));

                MOS_LOCK_PARAMS lockFlags {};
                lockFlags.ReadOnly = 1;
                lockFlags.TiledAsTiled = 1;
                lockFlags.NoDecompress = 0;
                m_osInterface->pfnLockResource(m_osInterface, &dstSurface.OsResource, &lockFlags);
                m_osInterface->pfnUnlockResource(m_osInterface, &dstSurface.OsResource);

            }

            if (CodecHal_PictureIsFrame(decodeStatusReport->m_currDecodedPic) ||
                CodecHal_PictureIsInterlacedFrame(decodeStatusReport->m_currDecodedPic) ||
                CodecHal_PictureIsField(decodeStatusReport->m_currDecodedPic))
            {
                CODECHAL_DECODE_CHK_STATUS_BREAK(m_debugInterface->DeleteCfgLinkNode(m_debugInterface->m_decodeSurfDumpFrameNum));
                m_debugInterface->m_decodeSurfDumpFrameNum = tempSurfNum;
            }
            m_debugInterface->m_bufferDumpFrameNum = tempFrameNum;
            m_debugInterface->m_currPic            = tempPic;
            m_debugInterface->m_frameType          = tempFrameType;

            if (m_decodeStatusBuf.m_decodeStatus[index].m_hwStoredData == CODECHAL_STATUS_QUERY_END_FLAG)
            {
                // report the CS Engine ID to user feature
                for (auto j = 0; j < CODECHAL_CS_INSTANCE_ID_MAX; j++)
                {
                    CODECHAL_CS_ENGINE_ID csEngineIdValue;
                    csEngineIdValue.value = m_decodeStatusBuf.m_decodeStatus[index].m_mmioCsEngineIdReg[j];

                    //validate the user feature value
                    if (csEngineIdValue.value)
                    {
                        CODECHAL_DECODE_ASSERT(csEngineIdValue.fields.ClassId == CODECHAL_CLASS_ID_VIDEO_ENGINE);
                        CODECHAL_DECODE_ASSERT(csEngineIdValue.fields.InstanceId < CODECHAL_CS_INSTANCE_ID_MAX);
                        CODECHAL_UPDATE_USED_VDBOX_ID_USER_FEATURE(csEngineIdValue.fields.InstanceId);
                    }
                }
                preIndex = index + 1;
            }
        }

        m_debugInterface->m_preIndex = preIndex;
    )

    if (m_consecutiveMbErrorConcealmentInUse &&
        m_incompletePicture)
    {
        CODECHAL_DECODE_VERBOSEMESSAGE("Processing incomplete frame MBs");

        if (!m_isHybridDecoder)
        {
            m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext);
        }

        m_decodePhantomMbs = true;

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(DecodePrimitiveLevel(),
            "Primitive level decoding failed.");
    }

    m_decodePhantomMbs = false;

    return eStatus;
}

MOS_STATUS CodechalDecode::Execute(void *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(Codechal::Execute(params));

    CodechalDecodeParams *decodeParams = (CodechalDecodeParams *)params;
    m_executeCallIndex = decodeParams->m_executeCallIndex;

    // MSDK event handling
    Mos_Solo_SetGpuAppTaskEvent(m_osInterface, decodeParams->m_gpuAppTaskEvent);

#if (_DEBUG || _RELEASE_INTERNAL)

    MOS_TraceEvent(EVENT_CODEC_DECODE, EVENT_TYPE_START, &m_standard, sizeof(uint32_t), &m_frameNum, sizeof(uint32_t));

#endif  // _DEBUG || _RELEASE_INTERNAL

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->m_bufferDumpFrameNum = m_frameNum;)

    if (m_cencBuf!= nullptr)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(Mos_Solo_DisableAubcaptureOptimizations(
            m_osInterface,
            IsFirstExecuteCall()));
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (decodeParams->m_refFrameCnt != 0)
    {
        PCODECHAL_DECODE_PROCESSING_PARAMS  procParams;
        uint32_t                            allocWidth;
        uint32_t                            allocHeight;
        MOS_FORMAT                          format;
        uint8_t                             frameIdx;

        CODECHAL_DECODE_CHK_NULL_RETURN(decodeParams->m_picParams);
        CODECHAL_DECODE_CHK_NULL_RETURN(decodeParams->m_procParams);

        procParams = (PCODECHAL_DECODE_PROCESSING_PARAMS)decodeParams->m_procParams;
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
            m_osInterface,
            procParams->pOutputSurface));

        if (procParams->bIsSourceSurfAllocated)
        {
            procParams->rcOutputSurfaceRegion.Width = procParams->pOutputSurface->dwWidth;
            procParams->rcOutputSurfaceRegion.Height = procParams->pOutputSurface->dwHeight;
            frameIdx = 0;

            m_refSurfaces = procParams->pInputSurface;
            CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                m_osInterface,
                m_refSurfaces));

            procParams->rcInputSurfaceRegion.X      = 0;
            procParams->rcInputSurfaceRegion.Y      = 0;
            procParams->rcInputSurfaceRegion.Width  = m_refSurfaces->dwWidth;
            procParams->rcInputSurfaceRegion.Height = m_refSurfaces->dwHeight;
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CalcDownsamplingParams(
                decodeParams->m_picParams, &allocWidth, &allocHeight, &format, &frameIdx));

            if (frameIdx >= decodeParams->m_refFrameCnt)
            {
                CODECHAL_DECODE_ASSERTMESSAGE("Invalid Downsampling Reference Frame Index !");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            if (m_refSurfaces == nullptr)
            {
                m_refFrmCnt = decodeParams->m_refFrameCnt;
                CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateRefSurfaces(allocWidth, allocHeight, format));
            }
            else
            {
                PMOS_SURFACE currSurface = &m_refSurfaces[frameIdx];
                if (currSurface->dwHeight < allocHeight || currSurface->dwWidth < allocWidth)
                {
                    CODECHAL_DECODE_CHK_STATUS_RETURN(RefSurfacesResize(frameIdx, allocWidth, allocHeight, format));
                }
            }

            procParams->rcInputSurfaceRegion.X = 0;
            procParams->rcInputSurfaceRegion.Y = 0;
            procParams->rcInputSurfaceRegion.Width = allocWidth;
            procParams->rcInputSurfaceRegion.Height = allocHeight;
          
            procParams->pInputSurface = &m_refSurfaces[frameIdx];
        }
        decodeParams->m_destSurface = &m_refSurfaces[frameIdx];
    }
#endif
    m_decodeParams  = *decodeParams;

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mos_Solo_PreProcessDecode(
        m_osInterface,
        m_decodeParams.m_destSurface));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_cpInterface->UpdateParams(true));

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
        m_osInterface,
        decodeParams->m_destSurface));

    if(!m_isHybridDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
            m_osInterface,
            m_videoContext));
    }
    if (!m_incompletePicture)
    {
        m_osInterface->pfnResetOsStates(m_osInterface);
    }

    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(SetFrameStates(),
        "Decoding initialization failed.");

    CODECHAL_DECODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

    CODECHAL_DECODE_CHK_STATUS_RETURN(SetDummyReference());

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->SetWatchdogTimerThreshold(m_width, m_height, false));

    if ((!m_incompletePicture) && (!m_isHybridDecoder))
    {
        m_osInterface->pfnIncPerfFrameID(m_osInterface);
        m_osInterface->pfnSetPerfTag(
            m_osInterface,
            (uint16_t)(((m_mode << 4) & 0xF0) | (m_perfType & 0xF)));
        m_osInterface->pfnResetPerfBufferID(m_osInterface);
    }

    CODECHAL_DEBUG_TOOL(

        if (decodeParams->m_dataBuffer &&
            (m_standard != CODECHAL_JPEG && m_cencBuf == nullptr) &&
            !(m_standard == CODECHAL_HEVC && m_isHybridDecoder) &&
            !(m_standard == CODECHAL_HEVC && (m_incompletePicture || !IsFirstExecuteCall())))
        {
            if (m_mode == CODECHAL_DECODE_MODE_MPEG2VLD ||
                m_mode == CODECHAL_DECODE_MODE_VC1VLD   ||
                m_mode == CODECHAL_DECODE_MODE_AVCVLD   ||
                m_mode == CODECHAL_DECODE_MODE_VP8VLD   ||
                m_mode == CODECHAL_DECODE_MODE_HEVCVLD  ||
                m_mode == CODECHAL_DECODE_MODE_VP9VLD)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    decodeParams->m_dataBuffer,
                    CodechalDbgAttr::attrBitstream,
                    "_DEC",
                    decodeParams->m_dataSize,
                    decodeParams->m_dataOffset,
                    CODECHAL_NUM_MEDIA_STATES
                    ));
            }
            else
            {
               //  Dump ResidualDifference
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    decodeParams->m_dataBuffer,
                    CodechalDbgAttr::attrResidualDifference,
                    "_DEC",
                    decodeParams->m_dataSize));
            }
        }
    )
#ifdef _DECODE_PROCESSING_SUPPORTED
    CODECHAL_DEBUG_TOOL(

        if (decodeParams->m_procParams)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpProcessingParams(
                (PCODECHAL_DECODE_PROCESSING_PARAMS)decodeParams->m_procParams));
        }
    )
#endif
    for(auto i = 0; i < m_decodePassNum; i++)
    {
        if (!m_incompletePicture)
        {
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(DecodeStateLevel(),
                "State level decoding failed.");
        }

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(DecodePrimitiveLevel(),
            "Primitive level decoding failed.");
    }

    if (m_secureDecoder != nullptr)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->UpdateHuCStreamoutBufferIndex());
    }

    *decodeParams = m_decodeParams;

    if (m_decodeHistogram != nullptr)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_decodeHistogram->RenderHistogram(this, m_decodeParams.m_destSurface));
    }

//#if (_DEBUG || _RELEASE_INTERNAL)
//#ifdef _MD5_DEBUG_SUPPORTED
//    if (CodecHal_PictureIsFrame(m_debugInterface->CurrPic) ||
//        CodecHal_PictureIsInterlacedFrame(m_debugInterface->CurrPic) ||
//        m_debugInterface->bSecondField ||
//        m_isHybridDecoder)
//    {
//        if (m_debugInterface->pMD5Context != nullptr)
//        {
//            if (m_debugInterface->bMD5DDIThreadExecute)
//            {
//                //calculate md5 hash for each RT surface
//                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgExecuteMD5Hash(
//                    m_debugInterface,
//                    &decodeParams->m_destSurface->OsResource,
//                    nullptr,
//                    0,
//                    0));
//            }
//        }
//    }
//#endif // _MD5_DEBUG_SUPPORTED
//#endif // _DEBUG || _RELEASE_INTERNAL

    CODECHAL_DEBUG_TOOL(
        if (CodecHal_PictureIsFrame(m_debugInterface->m_currPic) ||
            CodecHal_PictureIsInterlacedFrame(m_debugInterface->m_currPic) ||
            m_debugInterface->m_secondField) {
            if (!m_statusQueryReportingEnabled)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DeleteCfgLinkNode(m_debugInterface->m_bufferDumpFrameNum));
            }
        })

    if (CodecHal_PictureIsFrame(m_crrPic) ||
        CodecHal_PictureIsInterlacedFrame(m_crrPic) ||
        m_secondField)
    {
        m_frameNum++;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mos_Solo_PostProcessDecode(m_osInterface, m_decodeParams.m_destSurface));

#if (_DEBUG || _RELEASE_INTERNAL)

    MOS_TraceEvent(EVENT_CODEC_DECODE, EVENT_TYPE_END, &eStatus, sizeof(eStatus), nullptr, 0);

#endif  // _DEBUG || _RELEASE_INTERNAL

    return eStatus;
}

MOS_STATUS CodechalDecode::StartStatusReport(
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);

    uint32_t offset =
        (m_decodeStatusBuf.m_currIndex * sizeof(CodechalDecodeStatus)) +
        m_decodeStatusBuf.m_storeDataOffset +
        sizeof(uint32_t) * 2;

    MHW_MI_STORE_DATA_PARAMS params;
    params.pOsResource      = &m_decodeStatusBuf.m_statusBuffer;
    params.dwResourceOffset = offset;
    params.dwValue          = CODECHAL_STATUS_QUERY_START_FLAG;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &params));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectStartCmd((void *)this, m_osInterface, m_miInterface, cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalDecode::EndStatusReport(
    CodechalDecodeStatusReport      &decodeStatusReport,
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");
    auto mmioRegistersMfx = m_hwInterface->SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);
    auto mmioRegistersHcp = m_hcpInterface ? m_hcpInterface->GetMmioRegisters(m_vdboxIndex) : nullptr;

    uint32_t currIndex = m_decodeStatusBuf.m_currIndex;
    //Error Status report
    uint32_t errStatusOffset =
        currIndex * sizeof(CodechalDecodeStatus) +
        m_decodeStatusBuf.m_decErrorStatusOffset +
        sizeof(uint32_t) * 2;

    MHW_MI_STORE_REGISTER_MEM_PARAMS regParams;
    regParams.presStoreBuffer   = &m_decodeStatusBuf.m_statusBuffer;
    regParams.dwOffset          = errStatusOffset;
    regParams.dwRegister        = (m_standard == CODECHAL_HEVC && mmioRegistersHcp) ?
        mmioRegistersHcp->hcpCabacStatusRegOffset : mmioRegistersMfx->mfxErrorFlagsRegOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
        cmdBuffer,
        &regParams));

    //Frame CRC
    if (m_reportFrameCrc)
    {
        uint32_t frameCrcOffset =
            currIndex * sizeof(CodechalDecodeStatus) +
            m_decodeStatusBuf.m_decFrameCrcOffset +
            sizeof(uint32_t) * 2;

        regParams.presStoreBuffer   = &m_decodeStatusBuf.m_statusBuffer;
        regParams.dwOffset          = frameCrcOffset;
        if (m_standard == CODECHAL_AVC)
        {
            regParams.dwRegister        = mmioRegistersMfx->mfxFrameCrcRegOffset;
        }
        else if(m_standard == CODECHAL_HEVC)
        {
            regParams.dwRegister        = m_hcpFrameCrcRegOffset;
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
            cmdBuffer,
            &regParams));
    }

    //MB Count
    uint32_t mbCountOffset =
        currIndex * sizeof(CodechalDecodeStatus) +
        m_decodeStatusBuf.m_decMBCountOffset +
        sizeof(uint32_t) * 2;

    regParams.presStoreBuffer   = &m_decodeStatusBuf.m_statusBuffer;
    regParams.dwOffset          = mbCountOffset;
    regParams.dwRegister        = (m_standard == CODECHAL_HEVC && mmioRegistersHcp) ?
        mmioRegistersHcp->hcpDecStatusRegOffset : mmioRegistersMfx->mfxMBCountRegOffset;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
        cmdBuffer,
        &regParams));

    // First copy all the SW data in to the eStatus buffer
    m_decodeStatusBuf.m_decodeStatus[currIndex].m_swStoredData       = m_decodeStatusBuf.m_swStoreData;
    m_decodeStatusBuf.m_decodeStatus[currIndex].m_decodeStatusReport = decodeStatusReport;

    uint32_t storeDataOffset =
        currIndex * sizeof(CodechalDecodeStatus) +
        m_decodeStatusBuf.m_storeDataOffset +
        sizeof(uint32_t) * 2;

    MHW_MI_STORE_DATA_PARAMS dataParams;
    dataParams.pOsResource      = &m_decodeStatusBuf.m_statusBuffer;
    dataParams.dwResourceOffset = storeDataOffset;
    dataParams.dwValue          = CODECHAL_STATUS_QUERY_END_FLAG;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &dataParams));

    m_decodeStatusBuf.m_currIndex = (m_decodeStatusBuf.m_currIndex + 1) % CODECHAL_DECODE_STATUS_NUM;

    CodechalDecodeStatus *decodeStatus = &m_decodeStatusBuf.m_decodeStatus[m_decodeStatusBuf.m_currIndex];
    MOS_ZeroMemory(decodeStatus, sizeof(CodechalDecodeStatus));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_perfProfiler->AddPerfCollectEndCmd((void*)this, m_osInterface, m_miInterface, cmdBuffer));
    if (!m_osInterface->bEnableKmdMediaFrameTracking && m_osInterface->bInlineCodecStatusUpdate)
    {
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        // Send MI_FLUSH with protection bit off, which will FORCE exit protected mode for MFX
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        flushDwParams.pOsResource                   = &m_decodeStatusBuf.m_statusBuffer;
        flushDwParams.dwDataDW1                     = m_decodeStatusBuf.m_swStoreData;
        MHW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            cmdBuffer,
            &flushDwParams));
    }

    return eStatus;
}

MOS_STATUS CodechalDecode::ResetStatusReport(
    bool nullHwInUse)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (!m_osInterface->bEnableKmdMediaFrameTracking &&
        !m_osInterface->bInlineCodecStatusUpdate)
    {
        MOS_COMMAND_BUFFER cmdBuffer;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
            m_osInterface,
            &cmdBuffer,
            0));

        // initialize command buffer attributes
        cmdBuffer.Attributes.bTurboMode     = m_hwInterface->m_turboMode;

        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
            &cmdBuffer,
            false));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
            m_osInterface,
            &cmdBuffer,
            nullHwInUse));
    }

    m_decodeStatusBuf.m_swStoreData++;

    return eStatus;
}

MOS_STATUS CodechalDecode::GetStatusReport(
    void        *status,
    uint16_t    numStatus)
{
    uint16_t    reportsGenerated    = 0;
    MOS_STATUS  eStatus             = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(status);
    CodechalDecodeStatusReport *codecStatus = (CodechalDecodeStatusReport *)status;

    uint32_t numReportsAvailable    =
        (m_decodeStatusBuf.m_currIndex - m_decodeStatusBuf.m_firstIndex) &
        (CODECHAL_DECODE_STATUS_NUM - 1);
    uint32_t globalHWStoredData     = *(m_decodeStatusBuf.m_data);
    uint32_t globalCount            = m_decodeStatusBuf.m_swStoreData - globalHWStoredData;

    CODECHAL_DECODE_VERBOSEMESSAGE("    numStatus = %d, numReportsAvailable = %d.",
        numStatus, numReportsAvailable);
    CODECHAL_DECODE_VERBOSEMESSAGE("    HWStoreData = %d, globalCount = %d",
        globalHWStoredData, globalCount);

    if (numReportsAvailable < numStatus)
    {
        for (auto i = numReportsAvailable ; i < numStatus && i < CODECHAL_DECODE_STATUS_NUM ; i ++)
        {
            // These buffers are not yet received by driver. Just don't report anything back.
            codecStatus[i].m_codecStatus = CODECHAL_STATUS_UNAVAILABLE;
        }

        numStatus = (uint16_t)numReportsAvailable;
    }

    if (numReportsAvailable == 0)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("No reports available, m_currIndex = %d, m_firstIndex = %d",
            m_decodeStatusBuf.m_currIndex,
            m_decodeStatusBuf.m_firstIndex);
        return eStatus;
    }

    if (m_videoContextUsesNullHw ||
        m_videoContextForWaUsesNullHw ||
        m_renderContextUsesNullHw)
    {
        for (auto j = 0 ; j < numStatus ; j++)
        {
            uint32_t i = (m_decodeStatusBuf.m_firstIndex + numStatus - j - 1) & (CODECHAL_DECODE_STATUS_NUM - 1);
            codecStatus[j] = m_decodeStatusBuf.m_decodeStatus[i].m_decodeStatusReport;
            codecStatus[j].m_codecStatus = CODECHAL_STATUS_SUCCESSFUL;
            reportsGenerated++;
        }

        m_decodeStatusBuf.m_firstIndex =
            (m_decodeStatusBuf.m_firstIndex + reportsGenerated) % CODECHAL_DECODE_STATUS_NUM;

        return eStatus;
    }

    // Report eStatus in reverse temporal order
    for (auto j = 0; j < numStatus; j ++)
    {
        uint32_t i = (m_decodeStatusBuf.m_firstIndex + numStatus - j - 1) & (CODECHAL_DECODE_STATUS_NUM - 1);
        CodechalDecodeStatusReport decodeStatusReport = m_decodeStatusBuf.m_decodeStatus[i].m_decodeStatusReport;
        uint32_t localCount = m_decodeStatusBuf.m_decodeStatus[i].m_swStoredData - globalHWStoredData;

        if (m_isHybridDecoder)
        {
            codecStatus[j] = decodeStatusReport;
            // Consider the decode finished if the event dosen't present.
            CODECHAL_DECODE_CHK_STATUS_RETURN(DecodeGetHybridStatus(
                m_decodeStatusBuf.m_decodeStatus, i, CODECHAL_STATUS_QUERY_END_FLAG));

            if (m_decodeStatusBuf.m_decodeStatus[i].m_hwStoredData == CODECHAL_STATUS_QUERY_END_FLAG)
            {
                codecStatus[j].m_codecStatus = CODECHAL_STATUS_SUCCESSFUL;
                reportsGenerated ++;
            }
            else
            {
                codecStatus[j].m_codecStatus = CODECHAL_STATUS_INCOMPLETE;
            }
        }
        else
        {
            if (localCount == 0 || localCount > globalCount)
            {
                codecStatus[j] = decodeStatusReport;

                // HW execution of these commands is complete.
                if (m_osInterface->pfnIsGPUHung(m_osInterface))
                {
                    codecStatus[j].m_codecStatus = CODECHAL_STATUS_INCOMPLETE;
                }
                else if (m_decodeStatusBuf.m_decodeStatus[i].m_hwStoredData == CODECHAL_STATUS_QUERY_END_FLAG)
                {
                    // No problem in execution
                    codecStatus[j].m_codecStatus = CODECHAL_STATUS_SUCCESSFUL;

                    if (m_standard == CODECHAL_HEVC)
                    {
                        if ((m_decodeStatusBuf.m_decodeStatus[i].m_mmioErrorStatusReg &
                             m_hcpInterface->GetHcpCabacErrorFlagsMask()) != 0)
                        {
                            codecStatus[j].m_codecStatus = CODECHAL_STATUS_ERROR;
                            codecStatus[j].m_numMbsAffected =
                                (m_decodeStatusBuf.m_decodeStatus[i].m_mmioMBCountReg & 0xFFFC0000) >> 18;
                        }

                        if (m_reportFrameCrc)
                        {
                            codecStatus[j].m_frameCrc = m_decodeStatusBuf.m_decodeStatus[i].m_mmioFrameCrcReg;
                            CODECHAL_DECODE_NORMALMESSAGE("HCP CRC:: %d\n", codecStatus[j].m_frameCrc);
                        }
                    }
                    else
                    {
                        // Check to see if decoding error occurs
                        if (m_standard != CODECHAL_JPEG)
                        {
                            if ((m_decodeStatusBuf.m_decodeStatus[i].m_mmioErrorStatusReg &
                                 m_mfxInterface->GetMfxErrorFlagsMask()) != 0)
                            {
                                codecStatus[j].m_codecStatus = CODECHAL_STATUS_ERROR;
                            }
                        //MB Count bit[15:0] is error concealment MB count for none JPEG decoder.
                            codecStatus[j].m_numMbsAffected =
                                m_decodeStatusBuf.m_decodeStatus[i].m_mmioMBCountReg & 0xFFFF;
                        }
                        if (m_standard == CODECHAL_AVC)
                        {
                            codecStatus[j].m_frameCrc = m_decodeStatusBuf.m_decodeStatus[i].m_mmioFrameCrcReg;
                        }
                    }
                }
                else if (m_decodeStatusBuf.m_decodeStatus[i].m_hwStoredData == CODECHAL_STATUS_QUERY_SKIPPED)
                {
                    // In the case of AVC PR3.0 it is possible to drop a batch buffer execution
                    CODECHAL_DECODE_NORMALMESSAGE("Decode skipped.");
                    codecStatus[j].m_codecStatus = CODECHAL_STATUS_SUCCESSFUL;
                }
                else
                {
                    // BB_END data not written. Media reset might have occurred.
                    CODECHAL_DECODE_NORMALMESSAGE("Media reset may have occured.");
                    codecStatus[j].m_codecStatus = CODECHAL_STATUS_ERROR;
                }

                if (m_standard == CODECHAL_HEVC)
                {
                    // Print HuC_Status and HuC_Status2 registers
                    CODECHAL_DECODE_VERBOSEMESSAGE("Index = %d", i);
                    CODECHAL_DECODE_VERBOSEMESSAGE("HUC_STATUS register = 0x%x",
                        m_decodeStatusBuf.m_decodeStatus[i].m_hucErrorStatus >> 32);
                    CODECHAL_DECODE_VERBOSEMESSAGE("HUC_STATUS2 register = 0x%x",
                        m_decodeStatusBuf.m_decodeStatus[i].m_hucErrorStatus2 >> 32);
                }

                CODECHAL_DECODE_VERBOSEMESSAGE("Incrementing reports generated to %d.",
                    (reportsGenerated + 1));
                reportsGenerated ++;
            }
            else
            {
                CODECHAL_DECODE_VERBOSEMESSAGE("status buffer %d is INCOMPLETE.", j);
                codecStatus[j] = decodeStatusReport;
                codecStatus[j].m_codecStatus = CODECHAL_STATUS_INCOMPLETE;
                if(m_osInterface->bInlineCodecStatusUpdate)
                {
                   // In Linux/Android, inline decode status reporting is enabled.
                   // If still received CODECHAL_STATUS_INCOMPLETE,
                   // it will be treat as GPU Hang. so need generate a report.
                   reportsGenerated ++;
                }
            }
        }
    }

    m_decodeStatusBuf.m_firstIndex =
        (m_decodeStatusBuf.m_firstIndex + reportsGenerated) % CODECHAL_DECODE_STATUS_NUM;
    CODECHAL_DECODE_VERBOSEMESSAGE("m_firstIndex now becomes %d.", m_decodeStatusBuf.m_firstIndex);

    return eStatus;
}

MOS_STATUS CodechalDecode::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    bool                            frameTrackingRequested)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);

    MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

    // Send Start Marker command
    if (m_decodeParams.m_setMarkerEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SendMarkerCommand(cmdBuffer, MOS_RCS_ENGINE_USED(gpuContext)));
    }

    if (frameTrackingRequested)
    {
        // initialize command buffer attributes
        cmdBuffer->Attributes.bTurboMode = m_hwInterface->m_turboMode;
        cmdBuffer->Attributes.bMediaPreemptionEnabled = MOS_RCS_ENGINE_USED(gpuContext) ?
            m_hwInterface->GetRenderInterface()->IsPreemptionEnabled() : 0;
        cmdBuffer->Attributes.bEnableMediaFrameTracking = true;
        cmdBuffer->Attributes.resMediaFrameTrackingSurface = m_decodeStatusBuf.m_statusBuffer;
        cmdBuffer->Attributes.dwMediaFrameTrackingTag = m_decodeStatusBuf.m_swStoreData;
        // Set media frame tracking address offset(the offset from the decoder status buffer page, refer to CodecHalDecode_Initialize)
        cmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset = 0;
    }

    if (m_mmc)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SendPrologCmd(m_miInterface, cmdBuffer, gpuContext));
    }

    MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface                    = m_osInterface;
    genericPrologParams.pvMiInterface                   = m_miInterface;
    genericPrologParams.bMmcEnabled                     = CodecHalMmcState::IsMmcEnabled();

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmd(
        cmdBuffer,
        &genericPrologParams));

    // Send predication command
    if (m_decodeParams.m_predicationEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPredicationCommand(
            cmdBuffer));
    }

    return eStatus;
}

MOS_STATUS CodechalDecode::SendPredicationCommand(
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_miInterface);

    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS  condBBEndParams;
    MOS_ZeroMemory(&condBBEndParams, sizeof(condBBEndParams));

    // Skip current frame if presPredication is not equal to zero
    if (m_decodeParams.m_predicationNotEqualZero)
    {
        auto mmioRegistersMfx = m_hwInterface->SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);
        MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

        // load presPredication to general purpose register0
        MHW_MI_STORE_REGISTER_MEM_PARAMS    loadRegisterMemParams;
        MOS_ZeroMemory(&loadRegisterMemParams, sizeof(loadRegisterMemParams));
        loadRegisterMemParams.presStoreBuffer = m_decodeParams.m_presPredication;
        loadRegisterMemParams.dwOffset = (uint32_t)m_decodeParams.m_predicationResOffset;
        loadRegisterMemParams.dwRegister = mmioRegistersMfx->generalPurposeRegister0LoOffset;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(
            cmdBuffer,
            &loadRegisterMemParams));
        MHW_MI_LOAD_REGISTER_IMM_PARAMS     loadRegisterImmParams;
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData = 0;
        loadRegisterImmParams.dwRegister = mmioRegistersMfx->generalPurposeRegister0HiOffset;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(
            cmdBuffer,
            &loadRegisterImmParams));

        // load 0 to general purpose register4
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData = 0;
        loadRegisterImmParams.dwRegister = mmioRegistersMfx->generalPurposeRegister4LoOffset;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(
            cmdBuffer,
            &loadRegisterImmParams));
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData = 0;
        loadRegisterImmParams.dwRegister = mmioRegistersMfx->generalPurposeRegister4HiOffset;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(
            cmdBuffer,
            &loadRegisterImmParams));

        //perform the add operation
        MHW_MI_MATH_PARAMS  miMathParams;
        MHW_MI_ALU_PARAMS   miAluParams[4];
        MOS_ZeroMemory(&miMathParams, sizeof(miMathParams));
        MOS_ZeroMemory(&miAluParams, sizeof(miAluParams));
        // load     srcA, reg0
        miAluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
        miAluParams[0].Operand1 = MHW_MI_ALU_SRCA;
        miAluParams[0].Operand2 = MHW_MI_ALU_GPREG0;
        // load     srcB, reg4
        miAluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
        miAluParams[1].Operand1 = MHW_MI_ALU_SRCB;
        miAluParams[1].Operand2 = MHW_MI_ALU_GPREG4;
        // add      srcA, srcB
        miAluParams[2].AluOpcode = MHW_MI_ALU_ADD;
        miAluParams[2].Operand1 = MHW_MI_ALU_SRCB;
        miAluParams[2].Operand2 = MHW_MI_ALU_GPREG4;
        // store      reg0, ZF
        miAluParams[3].AluOpcode = MHW_MI_ALU_STORE;
        miAluParams[3].Operand1 = MHW_MI_ALU_GPREG0;
        miAluParams[3].Operand2 = MHW_MI_ALU_ZF;
        miMathParams.pAluPayload = miAluParams;
        miMathParams.dwNumAluParams = 4; // four ALU commands needed for this substract opertaion. see following ALU commands.
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiMathCmd(
            cmdBuffer,
            &miMathParams));

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        MHW_MI_STORE_REGISTER_MEM_PARAMS    storeRegParams;
        MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
        storeRegParams.presStoreBuffer = &m_predicationBuffer;
        storeRegParams.dwOffset = 0;
        storeRegParams.dwRegister = mmioRegistersMfx->generalPurposeRegister0LoOffset;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
            cmdBuffer,
            &storeRegParams));

        condBBEndParams.presSemaphoreBuffer = &m_predicationBuffer;
        condBBEndParams.dwOffset = 0;
        condBBEndParams.dwValue = 0;
        condBBEndParams.bDisableCompareMask = true;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            cmdBuffer,
            &condBBEndParams));

        *m_decodeParams.m_tempPredicationBuffer = &m_predicationBuffer;
    }
    else
    {
        // Skip current frame if presPredication is equal to zero
        condBBEndParams.presSemaphoreBuffer = m_decodeParams.m_presPredication;
        condBBEndParams.dwOffset = (uint32_t)m_decodeParams.m_predicationResOffset;
        condBBEndParams.bDisableCompareMask = true;
        condBBEndParams.dwValue = 0;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            cmdBuffer,
            &condBBEndParams));
    }

    return eStatus;
}

MOS_STATUS CodechalDecode::SendMarkerCommand(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool isRender)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_miInterface);

    if (isRender)
    {
        // Send pipe_control to get the timestamp
        MHW_PIPE_CONTROL_PARAMS             pipeControlParams;
        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
        pipeControlParams.presDest          = (PMOS_RESOURCE)m_decodeParams.m_presSetMarker;
        pipeControlParams.dwResourceOffset  = 0;
        pipeControlParams.dwPostSyncOp      = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        pipeControlParams.dwFlushMode       = MHW_FLUSH_WRITE_CACHE;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(cmdBuffer, NULL, &pipeControlParams));
    }
    else
    {
        // Send flush_dw to get the timestamp 
        MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.pOsResource           = (PMOS_RESOURCE)m_decodeParams.m_presSetMarker;
        flushDwParams.dwResourceOffset      = 0;
        flushDwParams.postSyncOperation     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        flushDwParams.bQWordEnable          = 1;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
    }

    return eStatus;
}

MOS_STATUS CodechalDecode::SetCencBatchBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBuffer);

    MHW_BATCH_BUFFER        batchBuffer;
    MOS_ZeroMemory(&batchBuffer, sizeof(MHW_BATCH_BUFFER));
    MOS_RESOURCE *resHeap = nullptr;
    CODECHAL_DECODE_CHK_NULL_RETURN(resHeap = m_cencBuf->secondLvlBbBlock->GetResource());
    batchBuffer.OsResource   = *resHeap;
    batchBuffer.dwOffset     = m_cencBuf->secondLvlBbBlock->GetOffset();
    batchBuffer.iSize        = m_cencBuf->secondLvlBbBlock->GetSize();
    batchBuffer.bSecondLevel = true;
#if (_DEBUG || _RELEASE_INTERNAL)
    batchBuffer.iLastCurrent = batchBuffer.iSize;
#endif  // (_DEBUG || _RELEASE_INTERNAL)

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
        cmdBuffer,
        &batchBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
            &batchBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_2ndLvlBatch"));)

    // Update GlobalCmdBufId
    MHW_MI_STORE_DATA_PARAMS miStoreDataParams;
    MOS_ZeroMemory(&miStoreDataParams, sizeof(miStoreDataParams));
    miStoreDataParams.pOsResource = m_cencBuf->resTracker;
    miStoreDataParams.dwValue     = m_cencBuf->trackerId;
    CODECHAL_DECODE_VERBOSEMESSAGE("dwCmdBufId = %d", miStoreDataParams.dwValue);
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &miStoreDataParams));
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
#ifdef _DECODE_PROCESSING_SUPPORTED
MOS_STATUS CodechalDecode::DumpProcessingParams(PCODECHAL_DECODE_PROCESSING_PARAMS decProcParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDecodeProcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(decProcParams);
    CODECHAL_DEBUG_CHK_NULL(decProcParams->pInputSurface);
    CODECHAL_DEBUG_CHK_NULL(decProcParams->pOutputSurface);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "Input Surface Resolution: "
        << +decProcParams->pInputSurface->dwWidth << " x " << +decProcParams->pInputSurface->dwHeight << std::endl;
    oss << "Input Region Resolution: "
        << +decProcParams->rcInputSurfaceRegion.Width << " x " << +decProcParams->rcInputSurfaceRegion.Height << std::endl;
    oss << "Input Region Offset: ("
        << +decProcParams->rcInputSurfaceRegion.X << "," << +decProcParams->rcInputSurfaceRegion.Y << ")" << std::endl;
    oss << "Input Surface Format: "
        << (decProcParams->pInputSurface->Format == Format_NV12 ? "NV12" : "P010" )<< std::endl;
    oss << "Output Surface Resolution: "
        << +decProcParams->pOutputSurface->dwWidth << " x " << +decProcParams->pOutputSurface->dwHeight << std::endl;
    oss << "Output Region Resolution: "
        << +decProcParams->rcOutputSurfaceRegion.Width << " x " << +decProcParams->rcOutputSurfaceRegion.Height << std::endl;
    oss << "Output Region Offset: ("
        << +decProcParams->rcOutputSurfaceRegion.X << ", " << +decProcParams->rcOutputSurfaceRegion.Y << ")" << std::endl;
    oss << "Output Surface Format: "
        << (decProcParams->pOutputSurface->Format == Format_NV12 ? "NV12" : "YUY2" )<< std::endl;

    const char* filePath = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufDecProcParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(filePath, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

#endif
#endif

