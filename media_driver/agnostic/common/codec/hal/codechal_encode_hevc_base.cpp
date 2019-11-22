/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     codechal_encode_hevc_base.cpp
//! \brief    Defines base class for HEVC encoder.
//!

#include "codechal_encode_hevc_base.h"
#include "codechal_vdenc_hevc.h"
#include "codechal_encode_hevc.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug.h"
#endif

const uint8_t CodechalEncodeHevcBase::TransformSkipCoeffsTable[4][2][2][2][2] =
{
    { { { { 42, 37 },{ 32, 40 } },{ { 40, 40 },{ 32, 45 } } },{ { { 29, 48 },{ 26, 53 } },{ { 26, 56 },{ 24, 62 } } } },
    { { { { 42, 40 },{ 32, 45 } },{ { 40, 46 },{ 32, 48 } } },{ { { 26, 53 },{ 24, 58 } },{ { 32, 53 },{ 26, 64 } } } },
    { { { { 38, 42 },{ 32, 51 } },{ { 43, 43 },{ 35, 46 } } },{ { { 26, 56 },{ 24, 64 } },{ { 35, 50 },{ 32, 57 } } } },
    { { { { 35, 46 },{ 32, 52 } },{ { 51, 42 },{ 38, 53 } } },{ { { 29, 56 },{ 29, 70 } },{ { 38, 47 },{ 37, 64 } } } },
};

const uint16_t CodechalEncodeHevcBase::TransformSkipLambdaTable[QP_NUM] =
{
    149, 149, 149, 149, 149, 149, 149, 149,
    149, 149, 149, 149, 149, 149, 149, 149,
    149, 149, 149, 149, 149, 149, 149, 149,
    149, 162, 174, 186, 199, 211, 224, 236,
    249, 261, 273, 286, 298, 298, 298, 298,
    298, 298, 298, 298, 298, 298, 298, 298,
    298, 298, 298, 298
};

MOS_STATUS CodechalEncodeHevcBase::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeHevc, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(settings);

#ifndef _FULL_OPEN_SOURCE
    // for HEVC: the Ds+Copy kernel is by default used to do CSC and copy non-aligned surface
    m_cscDsState->EnableCopy();
    m_cscDsState->EnableColor();
#endif
    m_mfeEnabled = settings->isMfeEnabled;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::Initialize(settings));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMmcState());

    m_is10BitHevc  = (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS) ? true : false;
    m_chromaFormat = settings->chromaFormat;
    m_bitDepth     = (settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_8_BITS) ? 8 : ((settings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS) ? 10 : 12);
    m_frameNum = 0;

    const uint32_t picWidthInLCU = MOS_ROUNDUP_DIVIDE(m_frameWidth, CODECHAL_HEVC_MIN_LCU_SIZE);        //assume smallest LCU to get max width
    const uint32_t picHeightInLCU = MOS_ROUNDUP_DIVIDE(m_frameHeight, CODECHAL_HEVC_MIN_LCU_SIZE);      //assume smallest LCU to get max height                                                                                            // MaxNumLcu is when LCU size is min lcu size(16)
    const uint32_t maxNumLCUs = picWidthInLCU *  picHeightInLCU;
    m_mvOffset = MOS_ALIGN_CEIL((maxNumLCUs * (m_hcpInterface->GetHcpPakObjSize()) * sizeof(uint32_t)), CODECHAL_PAGE_SIZE);

    // MaxNumCuRecords is when LCU size is max lcu size(64)
    const uint32_t maxNumCuRecords = MOS_ROUNDUP_DIVIDE(m_frameWidth, MAX_LCU_SIZE) *
        MOS_ROUNDUP_DIVIDE(m_frameHeight, MAX_LCU_SIZE) * 64;
    m_mbCodeSize =
        m_mvOffset + MOS_ALIGN_CEIL((maxNumCuRecords * m_hcpInterface->GetHevcEncCuRecordSize()), CODECHAL_PAGE_SIZE);

    m_widthAlignedMaxLcu  = MOS_ALIGN_CEIL(m_frameWidth, MAX_LCU_SIZE);
    m_heightAlignedMaxLcu = MOS_ALIGN_CEIL(m_frameHeight, MAX_LCU_SIZE);

    m_hevcBrcPakStatisticsSize = HEVC_BRC_PAK_STATISTCS_SIZE; // size for sturcture: CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER
    m_sizeOfHcpPakFrameStats   = 8 * CODECHAL_CACHELINE_SIZE;

    // Initialize kernel State
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelState());

    // Get max binding table count
    m_maxBtCount = GetMaxBtCount();

    // Picture Level Commands
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CalculatePictureStateCommandSize());

    // Slice Level Commands
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->GetHxxPrimitiveCommandSize(
            CODECHAL_ENCODE_MODE_HEVC,
            &m_defaultSliceStatesSize,
            &m_defaultSlicePatchListSize,
            m_singleTaskPhaseSupported));

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_IFRAME_RDOQ_ENABLE_ID,
        &userFeatureData);
    m_hevcIFrameRdoqEnabled = userFeatureData.i32Data ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_RDOQ_INTRA_TU_OVERRIDE_ID,
        &userFeatureData);
    m_rdoqIntraTuOverride = (uint32_t)userFeatureData.u32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_RDOQ_INTRA_TU_DISABLE_ID,
        &userFeatureData);
    m_rdoqIntraTuDisableOverride = (uint32_t)userFeatureData.u32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_CODECHAL_RDOQ_INTRA_TU_THRESHOLD_ID,
        &userFeatureData);
    m_rdoqIntraTuThresholdOverride = (uint32_t)userFeatureData.u32Data;
#endif
    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AllocatePakResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    const uint32_t minLcuSize = 16;
    const uint32_t picWidthInMinLCU = MOS_ROUNDUP_DIVIDE(m_frameWidth, minLcuSize);        //assume smallest LCU to get max width
    const uint32_t picHeightInMinLCU = MOS_ROUNDUP_DIVIDE(m_frameHeight, minLcuSize);      //assume smallest LCU to get max height

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    // Deblocking Filter Row Store Scratch data surface
    const uint32_t formatDenom = 2;
    uint32_t       formatMultiFactor = m_chromaFormat == HCP_CHROMA_FORMAT_YUV444 ? 3 : 2;
    formatMultiFactor *= m_is10BitHevc ? 2 : 1;

    uint32_t size = ((m_frameWidth + 31) & 0xFFFFFFE0) >> 3;
    size = MOS_ALIGN_CEIL(MOS_ROUNDUP_DIVIDE(size * formatMultiFactor, formatDenom), 4);
    size *= CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "DeblockingScratchBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resDeblockingFilterRowStoreScratchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Deblocking Filter Row Store Scratch Buffer.");
        return eStatus;
    }

    // Deblocking Filter Tile Row Store Scratch data surface
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "DeblockingTileScratchBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resDeblockingFilterTileRowStoreScratchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Deblocking Filter Tile Row Store Scratch Buffer.");
        return eStatus;
    }

    // Deblocking Filter Column Row Store Scratch data surface
    size = ((m_frameHeight + picHeightInMinLCU * 6 + 31) & 0xFFFFFFE0) >> 3;
    size = MOS_ALIGN_CEIL(MOS_ROUNDUP_DIVIDE(size * formatMultiFactor, formatDenom), 4);
    size *= CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "DeblockingColumnScratchBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resDeblockingFilterColumnRowStoreScratchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Deblocking Filter Column Row Store Scratch Buffer.");
        return eStatus;
    }

    // Metadata Line buffer
    size = MOS_MAX(
        MOS_ALIGN_CEIL((m_frameWidth + picWidthInMinLCU * 8 + 1023) >> 9, 2) * CODECHAL_CACHELINE_SIZE,                       // intra-slice
        MOS_ALIGN_CEIL((((m_frameWidth + 15) >> 4) * 188 + picWidthInMinLCU * 9 + 1023) >> 9, 2) * CODECHAL_CACHELINE_SIZE    // inter-slice
    );
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "MetadataLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resMetadataLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Metadata Line Buffer.");
        return eStatus;
    }

    // Metadata Tile Line buffer
    size = MOS_MAX(
        MOS_ALIGN_CEIL((m_frameWidth + picWidthInMinLCU * 8 + 1023) >> 9, 2) * CODECHAL_CACHELINE_SIZE,                  // intra-slice
        MOS_ALIGN_CEIL((((m_frameWidth + 15) >> 4) * 172 + picWidthInMinLCU * 9 + 1023) >> 9, 2) * CODECHAL_CACHELINE_SIZE   // inter-slice
    );
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "MetadataTileLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resMetadataTileLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Metadata Tile Line Buffer.");
        return eStatus;
    }

    // Metadata Tile Column buffer
    size = MOS_MAX(
        MOS_ALIGN_CEIL((m_frameHeight + picHeightInMinLCU * 8 + 1023) >> 9, 2) * CODECHAL_CACHELINE_SIZE,                    // intra-slice
        MOS_ALIGN_CEIL((((m_frameHeight + 15) >> 4) * 172 + picHeightInMinLCU * 9 + 1023) >> 9, 2) * CODECHAL_CACHELINE_SIZE // inter-slice
    );
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "MetadataTileColumnBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resMetadataTileColumnBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Metadata Tile Column Buffer.");
        return eStatus;
    }

    MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.ucMaxBitDepth  = m_bitDepth;
    hcpBufSizeParam.ucChromaFormat = m_chromaFormat;

    hcpBufSizeParam.dwCtbLog2SizeY = 6; // assume Max LCU size
    hcpBufSizeParam.dwPicWidth = MOS_ALIGN_CEIL(m_frameWidth, MAX_LCU_SIZE);
    hcpBufSizeParam.dwPicHeight = MOS_ALIGN_CEIL(m_frameHeight, MAX_LCU_SIZE);
    // SAO Line buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for SAO Line Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "SaoLineBuffer";
    eStatus                             = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO Line Buffer.");
        return eStatus;
    }

    // SAO Tile Line buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for SAO Tile Line Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "SaoTileLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoTileLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO Tile Line Buffer.");
        return eStatus;
    }

    // SAO Tile Column buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for SAO Tile Column Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "SaoTileColumnBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoTileColumnBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO Tile Column Buffer.");
        return eStatus;
    }

    // Lcu ILDB StreamOut buffer
    size = 1000000;
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "LcuILDBStreamOutBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resLcuIldbStreamOutBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate LCU ILDB StreamOut Buffer.");
        return eStatus;
    }

    // Lcu Base Address buffer
    // HEVC Encoder Mode: Slice size is written to this buffer when slice size conformance is enabled.
    // 1 CL (= 16 DWs = 64 bytes) per slice * Maximum number of dynamic slice = 600
    // Note that simulation is assigning much larger space for this.
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "LcuBaseAddressBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resLcuBaseAddressBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate LCU Base Address Buffer.");
        return eStatus;
    }

    uint32_t mvt_size = MOS_ALIGN_CEIL(((m_frameWidth + 63) >> 6)*((m_frameHeight + 15) >> 4), 2) * CODECHAL_CACHELINE_SIZE;
    uint32_t mvtb_size = MOS_ALIGN_CEIL(((m_frameWidth + 31) >> 5)*((m_frameHeight + 31) >> 5), 2) * CODECHAL_CACHELINE_SIZE;
    m_sizeOfMvTemporalBuffer = MOS_MAX(mvt_size, mvtb_size);

    // SAO StreamOut buffer
    size = MOS_ALIGN_CEIL(picWidthInMinLCU * picHeightInMinLCU * 16, CODECHAL_CACHELINE_SIZE);  // 16 bytes per LCU
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "SaoStreamOutBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoStreamOutBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO StreamOut Buffer.");
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::AllocateResources());

    // Allocate Ref Lists
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAllocateDataList(
        m_refList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC));

    // Create the sync objects which will be used by each reference frame
    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_refSync); i++)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(m_osInterface, &m_refSync[i].resSyncObject));
        m_refSync[i].bInUsed = false;
    }

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(AllocatePakResources(), "Failed to allocate PAK resources.");

    if (m_encEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(AllocateEncResources(), "Failed to allocate ENC resources.");

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(AllocateBrcResources(), "Failed to allocate BRC resources.");
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitSurfaceInfoTable());
    CreateMhwParams();

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AllocateBuffer(
    PCODECHAL_ENCODE_BUFFER buffer,
    uint32_t size,
    const char* name)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(buffer);

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format = Format_Buffer;
    allocParams.dwBytes = size;
    allocParams.pBufName = name;
    buffer->dwSize = size;

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        &buffer->sResource);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate %s.", name);
        return eStatus;
    }

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &buffer->sResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, size);

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &buffer->sResource);

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AllocateBuffer2D(
    PMOS_SURFACE surface,
    uint32_t width,
    uint32_t height,
    const char* name,
    MOS_TILE_TYPE tileType)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(surface);

    MOS_ZeroMemory(surface, sizeof(*surface));

    surface->TileType = tileType;
    surface->bArraySpacing = true;
    surface->Format = Format_Buffer_2D;
    surface->dwWidth = MOS_ALIGN_CEIL(width, 64);
    surface->dwHeight = height;
    surface->dwPitch = surface->dwWidth;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = surface->TileType;
    allocParamsForBuffer2D.Format = surface->Format;
    allocParamsForBuffer2D.dwWidth = surface->dwWidth;
    allocParamsForBuffer2D.dwHeight = surface->dwHeight;
    allocParamsForBuffer2D.pBufName = name;

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBuffer2D,
        &surface->OsResource);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate %s.", name);
        return eStatus;
    }

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &(surface->OsResource),
        &lockFlagsWriteOnly);

    if (data == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock 2D Surface.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    MOS_ZeroMemory(data, surface->dwWidth * surface->dwHeight);

    m_osInterface->pfnUnlockResource(m_osInterface, &(surface->OsResource));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
        m_osInterface,
        surface));

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AllocateSurface(
    PMOS_SURFACE surface,
    uint32_t width,
    uint32_t height,
    const char* name)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(surface);

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type = MOS_GFXRES_2D;
    allocParams.TileType = MOS_TILE_Y;
    allocParams.Format = Format_NV12;
    allocParams.dwWidth = width;
    allocParams.dwHeight = height;
    allocParams.pBufName = name;

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        &surface->OsResource);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate %s.", name);
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
        m_osInterface,
        surface));

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AllocateBatchBufferForPakSlices(
    uint32_t numSlices,
    uint8_t  numPakPasses)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_ZeroMemory(
        &m_batchBufferForPakSlices[m_currPakSliceIdx],
        sizeof(MHW_BATCH_BUFFER));

    // Get the slice size
    uint32_t size = (numPakPasses + 1) * numSlices * m_sliceStatesSize;

    m_batchBufferForPakSlices[m_currPakSliceIdx].bSecondLevel = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
        m_osInterface,
        &m_batchBufferForPakSlices[m_currPakSliceIdx],
        nullptr,
        size));

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    uint8_t *data       = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_batchBufferForPakSlices[m_currPakSliceIdx].OsResource,
        &lockFlags);

    if (data == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to lock batch buffer for PAK slices.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    MOS_ZeroMemory(data, size);
    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_batchBufferForPakSlices[m_currPakSliceIdx].OsResource);

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::ReadSseStatistics(PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    // encodeStatus is offset by 2 DWs in the resource
    uint32_t sseOffsetinBytes = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) + sizeof(uint32_t) * 2 + m_encodeStatusBuf.dwSumSquareErrorOffset;
    for (auto i = 0; i < 6; i++)    // 64 bit SSE values for luma/ chroma channels need to be copied
    {
        MHW_MI_COPY_MEM_MEM_PARAMS miCpyMemMemParams;
        MOS_ZeroMemory(&miCpyMemMemParams, sizeof(miCpyMemMemParams));
        miCpyMemMemParams.presSrc     = &m_resFrameStatStreamOutBuffer;
        miCpyMemMemParams.dwSrcOffset = (HEVC_PAK_STATISTICS_SSE_OFFSET + i) * sizeof(uint32_t);    // SSE luma offset is located at DW32 in Frame statistics, followed by chroma
        miCpyMemMemParams.presDst = &m_encodeStatusBuf.resStatusBuffer;
        miCpyMemMemParams.dwDstOffset = sseOffsetinBytes + i * sizeof(uint32_t);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(cmdBuffer, &miCpyMemMemParams));
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::CalculatePSNR(
    EncodeStatus       *encodeStatus,
    EncodeStatusReport *encodeStatusReport)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatus);
    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusReport);
    uint32_t numLumaPixels = 0, numPixelsPerChromaChannel = 0;

    numLumaPixels = m_frameHeight * m_frameWidth;
    switch (m_hevcSeqParams->chroma_format_idc)
    {
    case HCP_CHROMA_FORMAT_MONOCHROME:
        numPixelsPerChromaChannel = 0;
        break;
    case HCP_CHROMA_FORMAT_YUV420:
        numPixelsPerChromaChannel = numLumaPixels / 4;
        break;
    case HCP_CHROMA_FORMAT_YUV422:
        numPixelsPerChromaChannel = numLumaPixels / 2;
        break;
    case HCP_CHROMA_FORMAT_YUV444:
        numPixelsPerChromaChannel = numLumaPixels;
        break;
    default:
        numPixelsPerChromaChannel = numLumaPixels / 2;
        break;
    }

    double squarePeakPixelValue = pow((1 << (m_hevcSeqParams->bit_depth_luma_minus8 + 8)) - 1, 2);

    for (auto i = 0; i < 3; i++)
    {
        uint32_t numPixels = i ? numPixelsPerChromaChannel : numLumaPixels;

        if (m_hevcSeqParams->bit_depth_luma_minus8 == 0)
        {
            //8bit pixel data is represented in 10bit format in HW. so SSE should right shift by 4.
            encodeStatus->sumSquareError[i] >>= 4;
        }
        encodeStatusReport->PSNRx100[i] = (uint16_t) CodecHal_Clip3(0, 10000,
            (uint16_t) (encodeStatus->sumSquareError[i] ? 1000 * log10(squarePeakPixelValue * numPixels / encodeStatus->sumSquareError[i]) : -1));

        CODECHAL_ENCODE_VERBOSEMESSAGE("PSNRx100[%d]:%d.\n", i, encodeStatusReport->PSNRx100[i]);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::ReleaseBatchBufferForPakSlices(uint32_t index)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_batchBufferForPakSlices[index].iSize)
    {
        Mhw_FreeBb(m_osInterface, &m_batchBufferForPakSlices[index], nullptr);
        m_batchBufferForPakSlices[index].iSize = 0;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::FreePakResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    for (auto i = 0; i < CODECHAL_HEVC_NUM_PAK_SLICE_BATCH_BUFFERS; i++)
    {
        ReleaseBatchBufferForPakSlices(i);
    }

    m_osInterface->pfnFreeResource(m_osInterface, &m_resDeblockingFilterRowStoreScratchBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resDeblockingFilterTileRowStoreScratchBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resDeblockingFilterColumnRowStoreScratchBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resMetadataLineBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resMetadataTileLineBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resMetadataTileColumnBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resSaoLineBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resSaoTileLineBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resSaoTileColumnBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resLcuIldbStreamOutBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resLcuBaseAddressBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resSaoStreamOutBuffer);

    return MOS_STATUS_SUCCESS;
}

void CodechalEncodeHevcBase::FreeResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncoderState::FreeResources();

    FreeEncResources();
    FreeBrcResources();
    FreePakResources();

    // Release Ref Lists
    CodecHalFreeDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC);

    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_refSync); i++)
    {
        m_osInterface->pfnDestroySyncResource(m_osInterface, &m_refSync[i].resSyncObject);
    }

    if (m_sliceStateParams)
    {
        MOS_Delete(m_sliceStateParams);
        m_sliceStateParams = nullptr;
    }
    if (m_pipeModeSelectParams)
    {
        MOS_Delete(m_pipeModeSelectParams);
        m_pipeModeSelectParams = nullptr;
    }
    if (m_pipeBufAddrParams)
    {
        MOS_Delete(m_pipeBufAddrParams);
        m_pipeBufAddrParams = nullptr;
    }
}

MOS_STATUS CodechalEncodeHevcBase::SetSequenceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t frameWidth = (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);

    uint32_t frameHeight = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);

    // check if there is a dynamic resolution change
    if ((m_oriFrameHeight && (m_oriFrameHeight != frameHeight)) ||
        (m_oriFrameWidth && (m_oriFrameWidth != frameWidth)))
    {
        if (frameHeight > m_createHeight || frameWidth > m_createWidth)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Resolution reset from lower resolution to higher resolution not supported if it is higher than the resolution of first frame.%d, %d %d, %d", m_createWidth, m_createHeight, frameWidth, frameHeight);
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
        m_resolutionChanged = true;
        m_brcInit           = true;
    }
    else
    {
        m_resolutionChanged = false;
    }

    // setup internal parameters
    m_oriFrameWidth = m_frameWidth = frameWidth;
    m_oriFrameHeight = m_frameHeight = frameHeight;

    m_picWidthInMb = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_oriFrameWidth);
    m_picHeightInMb = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_oriFrameHeight);

    // Get row store cache params: as all the needed information is got here
    if (m_hcpInterface->IsRowStoreCachingSupported())
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        rowstoreParams.Mode = m_mode;
        rowstoreParams.dwPicWidth = m_frameWidth;
                rowstoreParams.ucChromaFormat   = m_chromaFormat;
                rowstoreParams.ucBitDepthMinus8 = m_hevcSeqParams->bit_depth_luma_minus8;
                rowstoreParams.ucLCUSize        = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
                m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams);
    }

    m_brcEnabled = IsRateControlBrc(m_hevcSeqParams->RateControlMethod);

    if (m_brcEnabled)
    {
        switch (m_hevcSeqParams->MBBRC)
        {
        case mbBrcInternal:
            m_lcuBrcEnabled = (m_hevcSeqParams->TargetUsage == 1);
            break;
        case mbBrcDisabled:
            m_lcuBrcEnabled = false;
            break;
        case mbBrcEnabled:
            m_lcuBrcEnabled = true;
            break;
        }

        if (m_hevcSeqParams->RateControlMethod == RATECONTROL_ICQ  || 
            m_hevcSeqParams->RateControlMethod == RATECONTROL_QVBR ||
            m_hevcPicParams->NumROI)
        {
            // ICQ or ROI must result in LCU-based BRC to be enabled.
            m_lcuBrcEnabled = true;  
        }
    }

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_VCM && m_lcuBrcEnabled)
    {
        m_lcuBrcEnabled = false;  // when VCM is enabled, only frame-based BRC
    }

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_ICQ || m_hevcSeqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (m_hevcSeqParams->ICQQualityFactor < CODECHAL_ENCODE_HEVC_MIN_ICQ_QUALITYFACTOR ||
            m_hevcSeqParams->ICQQualityFactor > CODECHAL_ENCODE_HEVC_MAX_ICQ_QUALITYFACTOR)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid ICQ Quality Factor input (%d)\n", m_hevcSeqParams->ICQQualityFactor);
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
    }

    m_usAvbrAccuracy    = CODECHAL_ENCODE_HEVC_DEFAULT_AVBR_ACCURACY;
    m_usAvbrConvergence = CODECHAL_ENCODE_HEVC_DEFAULT_AVBR_CONVERGENCE;

    // Calculate 4x, 16x, 32x dimensions as applicable
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CalcScaledDimensions());

    // It is assumed to be frame-mode always
    m_frameFieldHeight = m_frameHeight;
    m_frameFieldHeightInMb = m_picHeightInMb;
    m_downscaledFrameFieldHeightInMb16x = m_downscaledHeightInMb16x;
    m_downscaledFrameFieldHeightInMb4x = m_downscaledHeightInMb4x;
    m_downscaledFrameFieldHeightInMb32x = m_downscaledHeightInMb32x;

    m_brcReset          = m_hevcSeqParams->bResetBRC;
    m_roiValueInDeltaQp = m_hevcSeqParams->ROIValueInDeltaQP;

    uint32_t lcuInRow    = MOS_ALIGN_CEIL(m_frameWidth, (1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3))) >> (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    uint32_t lcu2MbRatio = (1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3)) / CODECHAL_MACROBLOCK_WIDTH;
    if (lcuInRow < 1 || lcu2MbRatio < 1)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (m_brcReset &&
        (!m_brcEnabled ||
            m_hevcSeqParams->RateControlMethod == RATECONTROL_ICQ))
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("BRC Reset cannot be trigerred in CQP/ICQ modes - invalid BRC parameters.");
        m_brcReset = false;
    }

    if (m_hevcSeqParams->TargetUsage == 0x07 && !m_enable26WalkingPattern)
    {
        m_enable26WalkingPattern = true;  // in the performance mode (TU=7), 26z walking pattern is not supported
    }

    if (!m_32xMeUserfeatureControl && m_32xMeSupported && m_hevcSeqParams->TargetUsage == 0x07)
    {
        m_32xMeSupported = false; // TU7 does not support ultra HME
    }

    m_encode4KSequence = ((m_frameWidth * m_frameHeight) >=
                             (ENCODE_HEVC_4K_PIC_WIDTH * ENCODE_HEVC_4K_PIC_HEIGHT))
                             ? true
                             : false;

    m_encode16KSequence = ((m_frameWidth * m_frameHeight) >=
                              (ENCODE_HEVC_16K_PIC_WIDTH * ENCODE_HEVC_16K_PIC_HEIGHT))
                             ? true
                             : false;

    // if GOP structure is I-frame only, we use 3 non-ref slots for tracked buffer
    m_gopIsIdrFrameOnly = (m_hevcSeqParams->GopPicSize == 1);

    // check output Chroma format
    m_outputChromaFormat = m_hevcSeqParams->chroma_format_idc;

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_refIdxMapping[i]  = -1;
        m_currUsedRefPic[i] = false;
    }

    // To obtain current "used" reference frames. The number of current used reference frames cannot be greater than 8
    auto slcParams = m_hevcSliceParams;
    for (uint32_t s = 0; s < m_numSlices; s++, slcParams++)
    {
        for (auto ll = 0; ll < 2; ll++)
        {
            uint32_t numRef = (ll == 0) ? slcParams->num_ref_idx_l0_active_minus1 :
                slcParams->num_ref_idx_l1_active_minus1;

            if (numRef > CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Invalid number of ref frames for l0 %d or l1 %d", slcParams->num_ref_idx_l0_active_minus1, slcParams->num_ref_idx_l1_active_minus1);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
            for (uint32_t i = 0; i <= numRef; i++)
            {
                CODEC_PICTURE refPic = slcParams->RefPicList[ll][i];
                if (!CodecHal_PictureIsInvalid(refPic) &&
                    !CodecHal_PictureIsInvalid(m_hevcPicParams->RefFrameList[refPic.FrameIdx]))
                {
                    m_currUsedRefPic[refPic.FrameIdx] = true;
                }
            }
        }
    }

    for (uint8_t i = 0, RefIdx = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (!m_currUsedRefPic[i])
        {
            continue;
        }

        uint8_t index         = m_hevcPicParams->RefFrameList[i].FrameIdx;
        bool duplicatedIdx = false;
        for (unsigned char ii = 0; ii < i; ii++)
        {
            if (m_currUsedRefPic[i] && index == m_hevcPicParams->RefFrameList[ii].FrameIdx)
            {
                // We find the same FrameIdx in the ref_frame_list. Multiple reference frames are the same.
                // In other words, RefFrameList[i] and RefFrameList[ii] have the same surface Id
                duplicatedIdx = true;
                m_refIdxMapping[i] = m_refIdxMapping[ii];
                break;
            }
        }

        if (duplicatedIdx)
        {
            continue;
        }

        if (RefIdx >= CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC)
        {
            // Total number of distingushing reference frames cannot be geater than 8.
            CODECHAL_ENCODE_ASSERT(false);
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        // Map reference frame index [0-15] into a set of unique IDs within [0-7]
        m_refIdxMapping[i] = RefIdx;
        RefIdx++;
    }

    if (m_hevcPicParams->CodingType != I_TYPE && m_hevcPicParams->CollocatedRefPicIndex != 0xFF && m_hevcPicParams->CollocatedRefPicIndex < CODEC_MAX_NUM_REF_FRAME_HEVC)
    {
        uint8_t frameStoreId = (uint8_t)m_refIdxMapping[m_hevcPicParams->CollocatedRefPicIndex];

        if (frameStoreId >= CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC || !m_currUsedRefPic[m_hevcPicParams->CollocatedRefPicIndex])
        {
            // CollocatedRefPicIndex is wrong in this case for the reference frame is not used be used
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
    }

    if (m_hevcPicParams->QpY > CODECHAL_ENCODE_HEVC_MAX_SLICE_QP)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (Mos_ResourceIsNull(&m_reconSurface.OsResource) &&
        (!m_hevcPicParams->bUseRawPicForRef || m_codecFunction != CODECHAL_FUNCTION_ENC))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_hevcSeqParams->scaling_list_enable_flag && !m_hevcPicParams->scaling_list_data_present_flag)
    {
        CreateDefaultScalingList();
    }
    else if (!m_hevcSeqParams->scaling_list_enable_flag)
    {
        CreateFlatScalingList();
    }

    unsigned char prevRefIdx = m_currReconstructedPic.FrameIdx;
    PCODEC_REF_LIST *refListFull = &m_refList[0];

    // Sync initialize
    if ((m_firstFrame) ||
        (!m_brcEnabled && m_hevcPicParams->bUseRawPicForRef) ||
        (!m_brcEnabled && (m_hevcPicParams->CodingType == I_TYPE)) ||
        (!m_brcEnabled && !refListFull[prevRefIdx]->bUsedAsRef))
    {
        m_waitForPak = false;
    }
    else
    {
        m_waitForPak = true;
    }

    if (m_brcEnabled || m_hevcPicParams->bUsedAsRef)
    {
        m_signalEnc = true;
    }
    else
    {
        m_signalEnc = false;
    }

    m_currEncBbSet = MB_ENC_Frame_BB;
    m_lastPicInSeq               = m_hevcPicParams->bLastPicInSeq;
    m_lastPicInStream            = m_hevcPicParams->bLastPicInStream;
    m_statusReportFeedbackNumber = m_hevcPicParams->StatusReportFeedbackNumber;
    m_currOriginalPic            = m_hevcPicParams->CurrOriginalPic;
    m_currReconstructedPic       = m_hevcPicParams->CurrReconstructedPic;

    unsigned char currRefIdx                       = m_hevcPicParams->CurrReconstructedPic.FrameIdx;
    refListFull[currRefIdx]->sRefReconBuffer = m_reconSurface;
    refListFull[currRefIdx]->sRefRawBuffer = m_rawSurface;
    refListFull[currRefIdx]->RefPic                = m_hevcPicParams->CurrOriginalPic;
    refListFull[currRefIdx]->bUsedAsRef            = m_hevcPicParams->bUsedAsRef;
    refListFull[currRefIdx]->resBitstreamBuffer = m_resBitstreamBuffer;
    refListFull[currRefIdx]->bFormatConversionDone = false;

    // P/B frames with empty ref lists are internally encoded as I frames,
    // while picture header packing remains the original value
    m_pictureCodingType = m_hevcPicParams->CodingType;

    bool emptyRefFrmList = true;
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (m_hevcPicParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            emptyRefFrmList = false;
            break;
        }
    }

    if (emptyRefFrmList && m_pictureCodingType != I_TYPE)
    {
        // If there is no reference frame in the list, just mark the current picture as the I type
        m_pictureCodingType = I_TYPE;
    }

    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_picIdx[i].bValid = false;
        if (m_hevcPicParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            uint8_t index         = m_hevcPicParams->RefFrameList[i].FrameIdx;
            bool duplicatedIdx = false;
            for (auto ii = 0; ii < i; ii++)
            {
                if (m_picIdx[ii].bValid && index == m_hevcPicParams->RefFrameList[ii].FrameIdx)
                {
                    // We find the same FrameIdx in the ref_frame_list. Multiple reference frames are the same.
                    // In other words, RefFrameList[i] and RefFrameList[ii] have the same surface Id
                    duplicatedIdx = true;
                    break;
                }
            }

            if (duplicatedIdx)
            {
                continue;
            }

            // this reference frame in unique. Save it into the full reference list with 127 items
            refListFull[index]->RefPic.PicFlags =
                CodecHal_CombinePictureFlags(refListFull[index]->RefPic, m_hevcPicParams->RefFrameList[i]);
            refListFull[index]->iFieldOrderCnt[0] = m_hevcPicParams->RefFramePOCList[i];
            refListFull[index]->iFieldOrderCnt[1] = m_hevcPicParams->RefFramePOCList[i];
            refListFull[index]->sRefBuffer        = m_hevcPicParams->bUseRawPicForRef ? refListFull[index]->sRefRawBuffer : refListFull[index]->sRefReconBuffer;

            m_picIdx[i].bValid   = true;
            m_picIdx[i].ucPicIdx = index;
        }
    }

    // Save the current RefList
    uint8_t ii = 0;
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (m_picIdx[i].bValid)
        {
            refListFull[currRefIdx]->RefList[ii] = m_hevcPicParams->RefFrameList[i];
            ii++;
        }
    }
    refListFull[currRefIdx]->ucNumRef = ii;
    m_currRefList = refListFull[currRefIdx];

    CodecEncodeHevcFeiPicParams *feiPicParams = (CodecEncodeHevcFeiPicParams *)m_encodeParams.pFeiPicParams;
    if ((m_codecFunction == CODECHAL_FUNCTION_ENC_PAK) ||
       ((m_codecFunction == CODECHAL_FUNCTION_FEI_ENC_PAK) && (feiPicParams->bCTBCmdCuRecordEnable == false)) ||
        (m_codecFunction == CODECHAL_FUNCTION_ENC_VDENC_PAK))
    {
        m_currMinus2MbCodeIndex = m_lastMbCodeIndex;
        m_lastMbCodeIndex       = m_currMbCodeIdx;
        // the actual MbCode/MvData surface to be allocated later
        m_trackedBuf->SetAllocationFlag(true);
    }
    else if (m_codecFunction == CODECHAL_FUNCTION_ENC)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_encodeParams.presMbCodeSurface);
        m_resMbCodeSurface = *m_encodeParams.presMbCodeSurface;
    }
    else if(((m_codecFunction == CODECHAL_FUNCTION_FEI_ENC_PAK) && feiPicParams->bCTBCmdCuRecordEnable) ||
             (m_codecFunction == CODECHAL_FUNCTION_FEI_ENC) ||
             (m_codecFunction == CODECHAL_FUNCTION_FEI_PAK))
    {
        if(Mos_ResourceIsNull(&feiPicParams->resCURecord) || Mos_ResourceIsNull(&feiPicParams->resCTBCmd))
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    refListFull[currRefIdx]->iFieldOrderCnt[0] = m_hevcPicParams->CurrPicOrderCnt;
    refListFull[currRefIdx]->iFieldOrderCnt[1] = m_hevcPicParams->CurrPicOrderCnt;

    m_hmeEnabled    = m_hmeSupported && m_pictureCodingType != I_TYPE;
    m_b16XMeEnabled = m_16xMeSupported && m_pictureCodingType != I_TYPE;
    m_b32XMeEnabled = m_32xMeSupported && m_pictureCodingType != I_TYPE;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CalcLCUMaxCodingSize());

    // Screen content flag will come in with PPS on Linux/Android, but in SPS on other platforms,
    // we will use screen content flag in PPS for kernel programming, and update
    // the PPS screen content flag based on the SPS screen content flag if enabled.
    m_hevcPicParams->bScreenContent |= m_hevcSeqParams->bScreenContent;

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::CalcLCUMaxCodingSize()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint16_t log2_max_coding_block_size = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    uint32_t rawCTUBits = (1 << (2 * log2_max_coding_block_size));

    switch (m_hevcSeqParams->chroma_format_idc)
    {
        // 420
    case 1:
        rawCTUBits = rawCTUBits * 3 / 2;
        break;
        // 422
    case 2:
        rawCTUBits = rawCTUBits * 2;
        break;
        // 444
    case 3:
        rawCTUBits = rawCTUBits * 3;
        break;
    default:
        break;
    };

    rawCTUBits = rawCTUBits * (m_hevcSeqParams->bit_depth_luma_minus8 + 8);
    rawCTUBits = (5 * rawCTUBits / 3);

    if (m_hevcPicParams->LcuMaxBitsizeAllowed == 0 || m_hevcPicParams->LcuMaxBitsizeAllowed > rawCTUBits)
    {
        m_hevcPicParams->LcuMaxBitsizeAllowed = rawCTUBits;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::SetSliceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_numSlices > m_maxNumSlicesSupported)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Number of slice exceeds limit!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // first slice must come with slice_segment_address = 0
    if (m_hevcSliceParams->slice_segment_address != 0)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("First slice segment_address != 0!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_refList[m_currReconstructedPic.FrameIdx]->ucQPValue[0] = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;

    m_lowDelay               = true;
    m_sameRefList            = true;
    m_arbitraryNumMbsInSlice = false;

    uint32_t lcuInRow  = MOS_ALIGN_CEIL(m_frameWidth, (1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3))) >> (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    auto     slcParams = m_hevcSliceParams;
    for (uint32_t startLCU = 0, slcCount = 0; slcCount < m_numSlices; slcCount++, slcParams++)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ValidateRefFrameData(slcParams));

        if ((m_hevcPicParams->QpY + slcParams->slice_qp_delta) > CODECHAL_ENCODE_HEVC_MAX_SLICE_QP)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ValidateLowDelayBFrame(slcParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ValidateSameRefInL0L1(slcParams));

        if (m_arbitraryNumMbsInSlice == false && (slcParams->NumLCUsInSlice % lcuInRow))
        {
            // Slice number must be multiple of LCU rows
            m_arbitraryNumMbsInSlice = true;
        }

        if (!m_hevcPicParams->tiles_enabled_flag)
        {
            CODECHAL_ENCODE_ASSERT(slcParams->slice_segment_address == startLCU);
            startLCU += slcParams->NumLCUsInSlice;
        }
    }
    
    if (m_lowDelay && !m_sameRefList)
    {
        CODECHAL_ENCODE_NORMALMESSAGE("Attention: LDB frame but with different L0/L1 list !");
    }

    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_VCM && m_pictureCodingType == B_TYPE && !m_lowDelay)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("VCM BRC mode does not support regular B-frames\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySliceSAOState());

#if (_DEBUG || _RELEASE_INTERNAL)
    m_forceSinglePakPass = false;
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    //read user feature key for pak pass number forcing.
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_FORCE_PAK_PASS_NUM_ID,
        &userFeatureData);
    if (userFeatureData.u32Data > 0 && userFeatureData.u32Data <= m_numPasses)
    {
        m_numPasses = (uint8_t)userFeatureData.u32Data - 1;
        if (m_numPasses == 0)
        {
            m_forceSinglePakPass = true;
            CODECHAL_ENCODE_VERBOSEMESSAGE("Force to single PAK pass\n");
        }
    }
#endif
    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::ValidateSameRefInL0L1(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(slcParams);

    if (m_sameRefList &&
        slcParams->num_ref_idx_l0_active_minus1 >= slcParams->num_ref_idx_l1_active_minus1)
    {
        for (int refIdx = 0; refIdx < slcParams->num_ref_idx_l1_active_minus1 + 1; refIdx++)
        {
            CODEC_PICTURE refPicL0 = slcParams->RefPicList[0][refIdx];
            CODEC_PICTURE refPicL1 = slcParams->RefPicList[1][refIdx];

            if (!CodecHal_PictureIsInvalid(refPicL0) && !CodecHal_PictureIsInvalid(refPicL1) && refPicL0.FrameIdx != refPicL1.FrameIdx)
            {
                m_sameRefList = false;
                break;
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::ValidateLowDelayBFrame(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(slcParams);

    // Examine if now it is in the low delay mode
    if (slcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE && m_lowDelay)
    {
        // forward
        for (int refIdx = 0; (refIdx < slcParams->num_ref_idx_l0_active_minus1 + 1) && m_lowDelay; refIdx++)
        {
            if (refIdx >= CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                break;
            }

            CODEC_PICTURE  refPic = slcParams->RefPicList[0][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && m_hevcPicParams->RefFramePOCList[refPic.FrameIdx] > m_hevcPicParams->CurrPicOrderCnt)
            {
                m_lowDelay = false;
            }
        }

        // backward
        for (int refIdx = 0; (refIdx < slcParams->num_ref_idx_l1_active_minus1 + 1) && m_lowDelay; refIdx++)
        {
            if (refIdx >= CODEC_MAX_NUM_REF_FRAME_HEVC)
            {
                break;
            }

            CODEC_PICTURE refPic = slcParams->RefPicList[1][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && m_hevcPicParams->RefFramePOCList[refPic.FrameIdx] > m_hevcPicParams->CurrPicOrderCnt)
            {
                m_lowDelay = false;
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::VerifySliceSAOState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_hevcSeqParams->SAO_enabled_flag)
    {
        auto     slcParams       = m_hevcSliceParams;
        uint32_t slcSaoLumaCount = 0, slcSaoChromaCount = 0;

        for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++, slcParams++)
        {
            slcSaoLumaCount += slcParams->slice_sao_luma_flag;
            slcSaoChromaCount += slcParams->slice_sao_chroma_flag;
        }

        // For HCP_SLICE_STATE command, slices must have the same SAO setting within a picture for encoder.
        if (((slcSaoLumaCount > 0) && (slcSaoLumaCount != m_numSlices)) ||
            ((slcSaoChromaCount > 0) && (slcSaoChromaCount != m_numSlices)))
        {
            m_hevcSeqParams->SAO_enabled_flag = false;
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid SAO parameters in slice. All slices must have the same SAO setting within a picture.");
        }
    }

    m_uc2NdSaoPass = 0;  // Assume there is no 2nd SAO pass

    if (m_hevcSeqParams->SAO_enabled_flag && m_b2NdSaoPassNeeded)
    {
        m_numPasses = m_numPasses + 1; // one more pass for the 2nd SAO, i.e., BRC0, BRC1, ..., BRCn, and SAOn+1
        m_uc2NdSaoPass = m_numPasses;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::UpdateYUY2SurfaceInfo(
    PMOS_SURFACE        surface,
    bool                is10Bit)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(surface);

    if (Format_YUY2V == surface->Format ||
        Format_Y216V == surface->Format)
    {
        // surface has been updated
        return eStatus;
    }

    surface->Format = is10Bit ? Format_Y216V : Format_YUY2V;
    surface->dwWidth = m_oriFrameWidth;
    surface->dwHeight = m_oriFrameHeight;

    surface->YPlaneOffset.iXOffset = 0;
    surface->YPlaneOffset.iYOffset = 0;

    surface->UPlaneOffset.iSurfaceOffset = surface->YPlaneOffset.iSurfaceOffset + surface->dwHeight * surface->dwPitch;
    surface->UPlaneOffset.iXOffset = 0;
    surface->UPlaneOffset.iYOffset = surface->dwHeight;

    surface->VPlaneOffset.iSurfaceOffset = surface->UPlaneOffset.iSurfaceOffset;
    surface->VPlaneOffset.iXOffset = 0;
    surface->VPlaneOffset.iYOffset = surface->dwHeight;

    return eStatus;
}

void CodechalEncodeHevcBase::CreateFlatScalingList()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    for (auto i = 0; i < 6; i++)
    {
        memset(&(m_hevcIqMatrixParams->ucScalingLists0[i][0]),
            0x10,
            sizeof(m_hevcIqMatrixParams->ucScalingLists0[i]));

        memset(&(m_hevcIqMatrixParams->ucScalingLists1[i][0]),
            0x10,
            sizeof(m_hevcIqMatrixParams->ucScalingLists1[i]));

        memset(&(m_hevcIqMatrixParams->ucScalingLists2[i][0]),
            0x10,
            sizeof(m_hevcIqMatrixParams->ucScalingLists2[i]));
    }

    memset(&(m_hevcIqMatrixParams->ucScalingLists3[0][0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingLists3[0]));

    memset(&(m_hevcIqMatrixParams->ucScalingLists3[1][0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingLists3[1]));

    memset(&(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID2[0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID2));

    memset(&(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID3[0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID3));
}

void CodechalEncodeHevcBase::CreateDefaultScalingList()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    const uint8_t flatScalingList4x4[16] =
    {
        16,16,16,16,
        16,16,16,16,
        16,16,16,16,
        16,16,16,16
    };

    const uint8_t defaultScalingList8x8[2][64] =
    {
        {
            16,16,16,16,17,18,21,24,
            16,16,16,16,17,19,22,25,
            16,16,17,18,20,22,25,29,
            16,16,18,21,24,27,31,36,
            17,17,20,24,30,35,41,47,
            18,19,22,27,35,44,54,65,
            21,22,25,31,41,54,70,88,
            24,25,29,36,47,65,88,115
        },
        {
            16,16,16,16,17,18,20,24,
            16,16,16,17,18,20,24,25,
            16,16,17,18,20,24,25,28,
            16,17,18,20,24,25,28,33,
            17,18,20,24,25,28,33,41,
            18,20,24,25,28,33,41,54,
            20,24,25,28,33,41,54,71,
            24,25,28,33,41,54,71,91
        }
    };

    for (auto i = 0; i < 6; i++)
    {
        memcpy(&(m_hevcIqMatrixParams->ucScalingLists0[i][0]),
            flatScalingList4x4,
            sizeof(m_hevcIqMatrixParams->ucScalingLists0[i]));
    }

    for (auto i = 0; i < 3; i++)
    {
        memcpy(&(m_hevcIqMatrixParams->ucScalingLists1[i][0]),
            defaultScalingList8x8[0],
            sizeof(m_hevcIqMatrixParams->ucScalingLists1[i]));

        memcpy(&(m_hevcIqMatrixParams->ucScalingLists1[3 + i][0]),
            defaultScalingList8x8[1],
            sizeof(m_hevcIqMatrixParams->ucScalingLists1[3 + i]));

        memcpy(&(m_hevcIqMatrixParams->ucScalingLists2[i][0]),
            defaultScalingList8x8[0],
            sizeof(m_hevcIqMatrixParams->ucScalingLists2[i]));

        memcpy(&(m_hevcIqMatrixParams->ucScalingLists2[3 + i][0]),
            defaultScalingList8x8[1],
            sizeof(m_hevcIqMatrixParams->ucScalingLists2[3 + i]));
    }

    memcpy(&(m_hevcIqMatrixParams->ucScalingLists3[0][0]),
        defaultScalingList8x8[0],
        sizeof(m_hevcIqMatrixParams->ucScalingLists3[0]));

    memcpy(&(m_hevcIqMatrixParams->ucScalingLists3[1][0]),
        defaultScalingList8x8[1],
        sizeof(m_hevcIqMatrixParams->ucScalingLists3[1]));

    memset(&(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID2[0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID2));

    memset(&(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID3[0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID3));
}

MOS_STATUS CodechalEncodeHevcBase::VerifyCommandBufferSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // resize CommandBuffer Size for every BRC pass
    if (!m_singleTaskPhaseSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::GetCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, cmdBuffer, 0));

    return eStatus;

}

MOS_STATUS CodechalEncodeHevcBase::ReturnCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, cmdBuffer, 0);
    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::SubmitCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                nullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, nullRendering));
    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    bool                        frameTrackingRequested,
    MHW_MI_MMIOREGISTERS       *mmioRegister)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::SendPrologWithFrameTracking(cmdBuffer, frameTrackingRequested, mmioRegister));

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::GetMaxMBPS(uint32_t levelIdc, uint32_t* maxMBPS, uint64_t* maxBytePerPic)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(maxMBPS);
    CODECHAL_ENCODE_CHK_NULL_RETURN(maxBytePerPic);

    switch (levelIdc)
    {
    case 30:
        *maxMBPS = 552960;
        *maxBytePerPic = 36864; break;
    case 60:
        *maxMBPS = 3686400;
        *maxBytePerPic = 122880; break;
    case 63:
        *maxMBPS = 7372800;
        *maxBytePerPic = 245760; break;
    case 90:
        *maxMBPS = 16588800;
        *maxBytePerPic = 552760; break;
    case 93:
        *maxMBPS = 33177600;
        *maxBytePerPic = 983040; break;
    case 120:
        *maxMBPS = 66846720;
        *maxBytePerPic = 2228224; break;
    case 123:
        *maxMBPS = 133693440;
        *maxBytePerPic = 2228224; break;
    case 150:
        *maxMBPS = 267386880;
        *maxBytePerPic = 8912896; break;
    case 153:
        *maxMBPS = 534773760;
        *maxBytePerPic = 8912896; break;
    case 156:
        *maxMBPS = 1069547520;
        *maxBytePerPic = 8912896; break;
    case 180:
        *maxMBPS = 1069547520;
        *maxBytePerPic = 35651584; break;
    case 183:
        *maxMBPS = 2139095040;
        *maxBytePerPic = 35651584; break;
    case 186:
        *maxMBPS = 4278190080;
        *maxBytePerPic = 35651584; break;
    default:
        *maxMBPS = 16588800;
        *maxBytePerPic = 552760; // CModel defaults to level 3.0 value if not found,
                              // we can do the same, just output that the issue exists and continue
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported LevelIDC setting for HEVC");
        break;
    }

    return eStatus;
}

uint32_t CodechalEncodeHevcBase::GetProfileLevelMaxFrameSize()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t         minCR = 2;
    float_t         formatFactor = 1.5;
    float_t         fminCrScale = 1.0;
    uint32_t        maxMBPS;
    uint64_t        maxBytePerPic;
    int32_t         levelIdc = m_hevcSeqParams->Level * 3;

    if (levelIdc == 186 || levelIdc == 150)
    {
        minCR = 6;
    }
    else if (levelIdc >150)
    {
        minCR = 8;
    }
    else if (levelIdc >93)
    {
        minCR = 4;
    }

    if (m_hevcSeqParams->chroma_format_idc == 0)
    {
        if (m_hevcSeqParams->bit_depth_luma_minus8 == 0)
            formatFactor = 1.0;
        else if (m_hevcSeqParams->bit_depth_luma_minus8 == 8)
            formatFactor = 2.0;
    }
    else if (m_hevcSeqParams->chroma_format_idc == 1)
    {
        if (m_hevcSeqParams->bit_depth_luma_minus8 == 2)
        {
            formatFactor = 1.875;
        }
        else if (m_hevcSeqParams->bit_depth_luma_minus8 == 4)
        {
            formatFactor = 2.25;
        }
    }
    else if (m_hevcSeqParams->chroma_format_idc == 2)
    {
        fminCrScale = 0.5;
        if (m_hevcSeqParams->bit_depth_luma_minus8 == 2)
        {
            formatFactor = 2.5;
        }
        else if (m_hevcSeqParams->bit_depth_luma_minus8 == 4)
        {
            formatFactor = 3.0;
        }
    }
    else
    {
        fminCrScale = 0.5;
        formatFactor = 3.0;

        if (m_hevcSeqParams->bit_depth_luma_minus8 == 2)
        {
            formatFactor = 3.75;
        }
        else if (m_hevcSeqParams->bit_depth_luma_minus8 == 4)
        {
            formatFactor = 4.5;
        }
    }

    fminCrScale *= minCR;
    formatFactor /= fminCrScale;
    GetMaxMBPS(levelIdc, &maxMBPS, &maxBytePerPic);
    auto     maxBytePerPicNot0    = (uint64_t)((((float_t)maxMBPS * (float_t)m_hevcSeqParams->FrameRate.Denominator) / (float_t)m_hevcSeqParams->FrameRate.Numerator) * formatFactor);
    uint32_t profileLevelMaxFrame = 0;
    
    uint32_t userMaxFrameSize = m_hevcSeqParams->UserMaxIFrameSize;
    if ((m_hevcPicParams->CodingType != I_TYPE) && (m_hevcSeqParams->UserMaxPBFrameSize > 0))
    {
        userMaxFrameSize = m_hevcSeqParams->UserMaxPBFrameSize;
    }
    
    if (userMaxFrameSize != 0)
    {
        profileLevelMaxFrame = (uint32_t)MOS_MIN(userMaxFrameSize, maxBytePerPic);
        profileLevelMaxFrame = (uint32_t)MOS_MIN(maxBytePerPicNot0, profileLevelMaxFrame);
    }
    else {
        profileLevelMaxFrame = (uint32_t)MOS_MIN(maxBytePerPicNot0, maxBytePerPic);
    }

    profileLevelMaxFrame = (uint32_t)MOS_MIN((m_frameHeight * m_frameWidth), profileLevelMaxFrame);

    return profileLevelMaxFrame;
}

void CodechalEncodeHevcBase::CalcTransformSkipParameters(
    MHW_VDBOX_ENCODE_HEVC_TRANSFORM_SKIP_PARAMS& params)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_hevcPicParams->transform_skip_enabled_flag)
    {
        return;
    }

    params.Transformskip_enabled = true;
    int sliceQP = CalSliceQp();

    int qpIdx = 0;
    if (sliceQP <= 22)
    {
        qpIdx = 0;
    }
    else if (sliceQP <= 27)
    {
        qpIdx = 1;
    }
    else if (sliceQP <= 32)
    {
        qpIdx = 2;
    }
    else
    {
        qpIdx = 3;
    }

    params.Transformskip_lambda = TransformSkipLambdaTable[sliceQP];

    if (m_hevcPicParams->CodingType == I_TYPE)
    {
        params.Transformskip_Numzerocoeffs_Factor0 = TransformSkipCoeffsTable[qpIdx][0][0][0][0];
        params.Transformskip_Numzerocoeffs_Factor1 = TransformSkipCoeffsTable[qpIdx][0][0][1][0];
        params.Transformskip_Numnonzerocoeffs_Factor0 = TransformSkipCoeffsTable[qpIdx][0][0][0][1] + 32;
        params.Transformskip_Numnonzerocoeffs_Factor1 = TransformSkipCoeffsTable[qpIdx][0][0][1][1] + 32;
    }
    else
    {
        params.Transformskip_Numzerocoeffs_Factor0 = TransformSkipCoeffsTable[qpIdx][1][0][0][0];
        params.Transformskip_Numzerocoeffs_Factor1 = TransformSkipCoeffsTable[qpIdx][1][0][1][0];
        params.Transformskip_Numnonzerocoeffs_Factor0 = TransformSkipCoeffsTable[qpIdx][1][0][0][1] + 32;
        params.Transformskip_Numnonzerocoeffs_Factor1 = TransformSkipCoeffsTable[qpIdx][1][0][1][1] + 32;
    }
}

MOS_STATUS CodechalEncodeHevcBase::SetSemaphoreMem(
    PMOS_RESOURCE semaphoreMem,
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t value)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(semaphoreMem);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = semaphoreMem;
    storeDataParams.dwResourceOffset = 0;
    storeDataParams.dwValue = value;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::SendHWWaitCommand(
    PMOS_RESOURCE semaphoreMem,
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t semValue)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(semaphoreMem);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    MHW_MI_SEMAPHORE_WAIT_PARAMS miSemaphoreWaitParams;
    MOS_ZeroMemory(&miSemaphoreWaitParams, sizeof(miSemaphoreWaitParams));
    miSemaphoreWaitParams.presSemaphoreMem = semaphoreMem;
    miSemaphoreWaitParams.bPollingWaitMode = true;
    miSemaphoreWaitParams.dwSemaphoreData = semValue;
    miSemaphoreWaitParams.CompareOperation = MHW_MI_SAD_EQUAL_SDD;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiSemaphoreWaitCmd(cmdBuffer, &miSemaphoreWaitParams));

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::SendMIAtomicCmd(
    PMOS_RESOURCE               semaMem,
    uint32_t                    immData,
    MHW_COMMON_MI_ATOMIC_OPCODE opCode,
    PMOS_COMMAND_BUFFER         cmdBuffer
    )
{
    MHW_MI_ATOMIC_PARAMS       atomicParams;
    MOS_STATUS                 eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_ZeroMemory((&atomicParams), sizeof(atomicParams));
    atomicParams.pOsResource = semaMem;
    atomicParams.dwDataSize = sizeof(uint32_t);
    atomicParams.Operation = opCode;
    atomicParams.bInlineData = true;
    atomicParams.dwOperand1Data[0] = immData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiAtomicCmd(cmdBuffer, &atomicParams));

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::WaitForVDBOX(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    if (!m_firstFrame &&
        !Mos_ResourceIsNull(&m_refSync[m_lastMbCodeIndex].resSemaphoreMem.sResource))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            SendHWWaitCommand(
                &m_refSync[m_lastMbCodeIndex].resSemaphoreMem.sResource,
                cmdBuffer,
                1));
    }

    //keep these codes here, in case later we need support parallel frame PAK (need more than one set of internal buffers used by PAK HW).
#if 0
    if (m_pictureCodingType == I_TYPE)
    {
        return eStatus;
    }

    bool refHasBeenWaited[CODEC_NUM_TRACKED_BUFFERS] = { false };

    // check all reference frames. If one of them has not be waited, then it needs to be wait and ensure it has been encoded completely.
    PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams = pHevcSliceParams;
    for (uint32_t s = 0; s < m_numSlices; s++, slcParams++)
    {
        for (auto ll = 0; ll < 2; ll++)
        {
            uint32_t numRef = (ll == 0) ? slcParams->num_ref_idx_l0_active_minus1 :
                slcParams->num_ref_idx_l1_active_minus1;

            for (uint32_t i = 0; i <= numRef; i++)
            {
                CODEC_PICTURE refPic = slcParams->RefPicList[ll][i];
                if (!CodecHal_PictureIsInvalid(refPic) &&
                    !CodecHal_PictureIsInvalid(pHevcPicParams->RefFrameList[refPic.FrameIdx]))
                {
                    uint32_t idx = pHevcPicParams->RefFrameList[refPic.FrameIdx].FrameIdx;
                    uint8_t ucMbCodeIdx = pRefList[idx]->ucMbCodeIdx;

                    if (ucMbCodeIdx >= CODEC_NUM_TRACKED_BUFFERS)
                    {
                        // MB code index is wrong
                        eStatus = MOS_STATUS_INVALID_PARAMETER;
                        return eStatus;
                    }

                    if (refHasBeenWaited[ucMbCodeIdx] || Mos_ResourceIsNull(&RefSync[ucMbCodeIdx].resSemaphoreMem.sResource))
                    {
                        continue;
                    }

                    // Use HW wait command
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(
                        SendHWWaitCommand(
                            &RefSync[ucMbCodeIdx].resSemaphoreMem.sResource,
                            cmdBuffer, 1));

                    refHasBeenWaited[ucMbCodeIdx] = true;
                }
            }
        }
    }
#endif
    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::ReadBrcPakStatistics(
    PMOS_COMMAND_BUFFER cmdBuffer,
    EncodeReadBrcPakStatsParams* params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presBrcPakStatisticBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presStatusBuffer);

    if (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex())                                                                         \
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_BITSTREAM_BYTECOUNT_FRAME);
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_BITSTREAM_BYTECOUNT_FRAME_NOHEADER);
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncBitstreamBytecountFrameNoHeaderRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params->presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL);
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    storeDataParams.pOsResource = params->presStatusBuffer;
    storeDataParams.dwResourceOffset = params->dwStatusBufNumPassesOffset;
    storeDataParams.dwValue = params->ucPass;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::ReadHcpStatus(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    EncodeStatusBuffer *encodeStatusBuf = &m_encodeStatusBuf;

    uint32_t baseOffset =
        (encodeStatusBuf->wCurrIndex * encodeStatusBuf->dwReportSize) +
        sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);
    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset = baseOffset + encodeStatusBuf->dwBSByteCountOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset = baseOffset + encodeStatusBuf->dwBSSEBitCountOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncBitstreamSeBitcountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset = baseOffset + encodeStatusBuf->dwQpStatusCountOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncQpStatusCountRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::ReadImageStatus(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    EncodeStatusBuffer *encodeStatusBuf = &m_encodeStatusBuf;

    uint32_t baseOffset =
        (encodeStatusBuf->wCurrIndex * encodeStatusBuf->dwReportSize) +
        sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource

    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);
    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset = baseOffset + encodeStatusBuf->dwImageStatusMaskOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusMaskRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    miStoreRegMemParams.dwOffset = baseOffset + encodeStatusBuf->dwImageStatusCtrlOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::UserFeatureKeyReport())

    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MODE_ID, m_codecFunction);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ME_ENABLE_ID, m_hmeSupported);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_16xME_ENABLE_ID, m_16xMeSupported);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_32xME_ENABLE_ID, m_32xMeSupported);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_26Z_ENABLE_ID, (!m_enable26WalkingPattern));
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENCODE_RATECONTROL_METHOD_ID, m_hevcSeqParams->RateControlMethod);

#if (_DEBUG || _RELEASE_INTERNAL)
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_CODEC_SIM_ENABLE_ID, m_osInterface->bSimIsActive);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_RDOQ_ENABLE_ID, m_hevcRdoqEnabled);
#endif

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::GetStatusReport(
    EncodeStatus *encodeStatus,
    EncodeStatusReport *encodeStatusReport)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatus);
    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusReport);

    // The last pass of BRC may have a zero value of hcpCumulativeFrameDeltaQp
    if (encodeStatus->ImageStatusCtrl.hcpTotalPass && encodeStatus->ImageStatusCtrl.hcpCumulativeFrameDeltaQp == 0)
    {
        encodeStatus->ImageStatusCtrl.hcpCumulativeFrameDeltaQp = encodeStatus->ImageStatusCtrlOfLastBRCPass.hcpCumulativeFrameDeltaQp;
    }
    encodeStatus->ImageStatusCtrlOfLastBRCPass.hcpCumulativeFrameDeltaQp = 0;

    encodeStatusReport->CodecStatus = CODECHAL_STATUS_SUCCESSFUL;
    encodeStatusReport->bitstreamSize = encodeStatus->dwMFCBitstreamByteCountPerFrame + encodeStatus->dwHeaderBytesInserted;

    encodeStatusReport->PanicMode = encodeStatus->ImageStatusCtrl.Panic;
    encodeStatusReport->AverageQp = 0;
    encodeStatusReport->QpY = 0;
    encodeStatusReport->SuggestedQpYDelta = encodeStatus->ImageStatusCtrl.hcpCumulativeFrameDeltaQp;
    encodeStatusReport->NumberPasses = (unsigned char)encodeStatus->ImageStatusCtrl.hcpTotalPass + 1; //initial pass is considered to be 0,hence +1 to report;
    CODECHAL_ENCODE_VERBOSEMESSAGE("Single Pipe Mode Exectued PAK Pass number: %d\n", encodeStatusReport->NumberPasses);

    if (m_frameWidth != 0 && m_frameHeight != 0)
    {
        //The CumulativeQp from the PAK has accumulation unit of LCU, so we align and divide height, width by LCU size
        uint32_t log2LcuSize = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;

        encodeStatusReport->QpY = encodeStatusReport->AverageQp =
            (uint8_t)(((uint32_t)encodeStatus->QpStatusCount.hcpCumulativeQP)
                / ((MOS_ALIGN_CEIL(m_frameWidth, (1 << log2LcuSize)) >> log2LcuSize) *
                (MOS_ALIGN_CEIL(m_frameHeight, (1 << log2LcuSize)) >> log2LcuSize)));
    }

    if (!Mos_ResourceIsNull(&m_resFrameStatStreamOutBuffer))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CalculatePSNR(encodeStatus, encodeStatusReport));
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::InitializePicture(const EncoderParams& params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_hevcSeqParams      = (PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS)(params.pSeqParams);
    m_hevcPicParams      = (PCODEC_HEVC_ENCODE_PICTURE_PARAMS)(params.pPicParams);
    m_hevcSliceParams    = (PCODEC_HEVC_ENCODE_SLICE_PARAMS)params.pSliceParams;
    m_hevcFeiPicParams   = (CodecEncodeHevcFeiPicParams *)params.pFeiPicParams;
    m_hevcIqMatrixParams = (PCODECHAL_HEVC_IQ_MATRIX_PARAMS)params.pIQMatrixBuffer;
    m_nalUnitParams      = params.ppNALUnitParams;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hevcSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hevcPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hevcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hevcIqMatrixParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_nalUnitParams);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(PlatformCapabilityCheck());

    if (CodecHalIsFeiEncode(m_codecFunction))
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_hevcFeiPicParams);
        m_hevcSeqParams->TargetUsage = 0x04;
    }

    if (m_newSeq)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());

        if (m_hevcSeqParams->log2_min_coding_block_size_minus3)
        {
            m_cscDsState->SetHcpReconAlignment(1 << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetPictureStructs());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSliceStructs());

    // Scaling occurs when either HME or BRC is enabled
    m_scalingEnabled = m_hmeSupported || m_brcEnabled;
    m_useRawForRef   = m_hevcPicParams->bUseRawPicForRef;

    if (m_hevcPicParams->SkipFrameFlag == FRAME_SKIP_NORMAL)
    {
        m_skipFrameFlag  = m_hevcPicParams->SkipFrameFlag;
        m_numSkipFrames  = m_hevcPicParams->NumSkipFrames;
        m_sizeSkipFrames = m_hevcPicParams->SizeSkipFrames;
    }

    m_pictureStatesSize    = m_defaultPictureStatesSize;
    m_picturePatchListSize = m_defaultPicturePatchListSize;

    m_sliceStatesSize    = m_defaultSliceStatesSize;
    m_slicePatchListSize = m_defaultSlicePatchListSize;

    // Mb Qp data
    m_mbQpDataEnabled = params.bMbQpDataEnabled;
    if (m_mbQpDataEnabled)
    {
        m_mbQpDataSurface = *(params.psMbQpDataSurface);
    }

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->m_currPic            = m_hevcPicParams->CurrOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = m_storeData;
        m_debugInterface->m_frameType          = m_pictureCodingType;

        if (m_newSeq) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpSeqParams(
                m_hevcSeqParams));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpPicParams(
            m_hevcPicParams));

        if (CodecHalIsFeiEncode(m_codecFunction)) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpFeiPicParams(
                m_hevcFeiPicParams));
        }

        for (uint32_t i = 0; i < m_numSlices; i++) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpSliceParams(
                &m_hevcSliceParams[i],
                m_hevcPicParams));
        })

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetStatusReportParams(
        m_refList[m_currReconstructedPic.FrameIdx]));

    m_bitstreamUpperBound = m_encodeParams.dwBitstreamSize;

    return eStatus;
}

void CodechalEncodeHevcBase::SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams)
{
    pipeModeSelectParams = {};
    pipeModeSelectParams.Mode = m_mode;
    pipeModeSelectParams.bStreamOutEnabled = m_vdencEnabled;
    pipeModeSelectParams.bVdencEnabled = m_vdencEnabled;
    pipeModeSelectParams.bRdoqEnable = m_hevcRdoqEnabled ? (m_pictureCodingType == I_TYPE ? m_hevcIFrameRdoqEnabled : 1) : 0;
    pipeModeSelectParams.bAdvancedRateControlEnable = m_vdencBrcEnabled;

    if (m_hevcSeqParams->SAO_enabled_flag)
    {
        // uses pipe mode select command to tell if this is the first or second pass of SAO
        pipeModeSelectParams.bSaoFirstPass = !IsLastPass();

        if (m_singleTaskPhaseSupportedInPak &&
            m_b2NdSaoPassNeeded &&
            m_brcEnabled)
        {
            if (GetCurrentPass() == m_uc2NdSaoPass - 1)  // the last BRC pass. This separates BRC passes and the 2nd pass SAO into different DMA buffer submissions
            {
                m_lastTaskInPhase = true;
            }
            else if (GetCurrentPass() == m_uc2NdSaoPass)  // the 2nd SAO pass
            {
                m_firstTaskInPhase = true;
                m_lastTaskInPhase = true;
            }
        }
    }
}

void CodechalEncodeHevcBase::SetHcpSrcSurfaceParams(MHW_VDBOX_SURFACE_PARAMS& srcSurfaceParams)
{
    MOS_ZeroMemory(&srcSurfaceParams, sizeof(srcSurfaceParams));
    srcSurfaceParams.Mode = m_mode;
    srcSurfaceParams.psSurface = m_rawSurfaceToPak;
    srcSurfaceParams.ucSurfaceStateId = CODECHAL_HCP_SRC_SURFACE_ID;
    srcSurfaceParams.ucBitDepthLumaMinus8   = m_hevcSeqParams->bit_depth_luma_minus8;
    srcSurfaceParams.ucBitDepthChromaMinus8 = m_hevcSeqParams->bit_depth_chroma_minus8;
    srcSurfaceParams.bDisplayFormatSwizzle  = m_hevcPicParams->bDisplayFormatSwizzle;
    srcSurfaceParams.ChromaType = m_outputChromaFormat;
    srcSurfaceParams.bSrc8Pak10Mode         = false; //No usage for 8->10 bit encode
    srcSurfaceParams.dwActualHeight = ((m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
#ifdef _MMC_SUPPORTED
    m_mmcState->SetSurfaceState(&srcSurfaceParams);
#endif
}

void CodechalEncodeHevcBase::SetHcpReconSurfaceParams(MHW_VDBOX_SURFACE_PARAMS& reconSurfaceParams)
{
    MOS_ZeroMemory(&reconSurfaceParams, sizeof(reconSurfaceParams));
    reconSurfaceParams.Mode = m_mode;
    reconSurfaceParams.psSurface = &m_reconSurface;
    reconSurfaceParams.ucSurfaceStateId = CODECHAL_HCP_DECODED_SURFACE_ID;
    reconSurfaceParams.ucBitDepthLumaMinus8   = m_hevcSeqParams->bit_depth_luma_minus8;
    reconSurfaceParams.ucBitDepthChromaMinus8 = m_hevcSeqParams->bit_depth_chroma_minus8;
    reconSurfaceParams.ChromaType = m_outputChromaFormat;
    reconSurfaceParams.dwActualHeight         = ((m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
    reconSurfaceParams.dwReconSurfHeight = m_rawSurfaceToPak->dwHeight;
#ifdef _MMC_SUPPORTED
    m_mmcState->SetSurfaceState(&reconSurfaceParams);
#endif
}

void CodechalEncodeHevcBase::SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    pipeBufAddrParams = {};
    pipeBufAddrParams.Mode = m_mode;
    pipeBufAddrParams.psPreDeblockSurface = &m_reconSurface;
    pipeBufAddrParams.psPostDeblockSurface = &m_reconSurface;
    pipeBufAddrParams.psRawSurface = m_rawSurfaceToPak;
    pipeBufAddrParams.presStreamOutBuffer = m_vdencEnabled ? &m_resStreamOutBuffer[0] : nullptr;
    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer    = &m_resDeblockingFilterRowStoreScratchBuffer;
    pipeBufAddrParams.presDeblockingFilterTileRowStoreScratchBuffer   = &m_resDeblockingFilterTileRowStoreScratchBuffer;
    pipeBufAddrParams.presDeblockingFilterColumnRowStoreScratchBuffer = &m_resDeblockingFilterColumnRowStoreScratchBuffer;

    pipeBufAddrParams.presMetadataLineBuffer        = &m_resMetadataLineBuffer;
    pipeBufAddrParams.presMetadataTileLineBuffer    = &m_resMetadataTileLineBuffer;
    pipeBufAddrParams.presMetadataTileColumnBuffer  = &m_resMetadataTileColumnBuffer;
    pipeBufAddrParams.presSaoLineBuffer             = &m_resSaoLineBuffer;
    pipeBufAddrParams.presSaoTileLineBuffer         = &m_resSaoTileLineBuffer;
    pipeBufAddrParams.presSaoTileColumnBuffer       = &m_resSaoTileColumnBuffer;
    pipeBufAddrParams.presCurMvTempBuffer = m_trackedBuf->GetMvTemporalBuffer(CODEC_CURR_TRACKED_BUFFER);
    pipeBufAddrParams.presLcuBaseAddressBuffer      = &m_resLcuBaseAddressBuffer;
    pipeBufAddrParams.dwLcuStreamOutOffset = 0;
    pipeBufAddrParams.presLcuILDBStreamOutBuffer    = &m_resLcuIldbStreamOutBuffer;
    pipeBufAddrParams.presSaoStreamOutBuffer        = &m_resSaoStreamOutBuffer;
    pipeBufAddrParams.presFrameStatStreamOutBuffer  = &m_resFrameStatStreamOutBuffer;
    pipeBufAddrParams.dwFrameStatStreamOutOffset =  0;
    pipeBufAddrParams.presSseSrcPixelRowStoreBuffer = &m_resSseSrcPixelRowStoreBuffer;
    pipeBufAddrParams.presPakCuLevelStreamoutBuffer =
        Mos_ResourceIsNull(&m_resPakcuLevelStreamoutData.sResource) ? nullptr : &m_resPakcuLevelStreamoutData.sResource;
    pipeBufAddrParams.bRawIs10Bit = m_is10BitHevc;

    //add for B frame support
    if (m_pictureCodingType != I_TYPE)
    {
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            if (!m_picIdx[i].bValid || !m_currUsedRefPic[i])
            {
                continue;
            }

            uint8_t idx = m_picIdx[i].ucPicIdx;
            CodecHalGetResourceInfo(m_osInterface, &(m_refList[idx]->sRefReconBuffer));

            uint8_t frameStoreId                           = (uint8_t)m_refIdxMapping[i];
            pipeBufAddrParams.presReferences[frameStoreId] = &(m_refList[idx]->sRefReconBuffer.OsResource);

            uint8_t refMbCodeIdx = m_refList[idx]->ucScalingIdx;
            pipeBufAddrParams.presColMvTempBuffer[frameStoreId] = m_trackedBuf->GetMvTemporalBuffer(refMbCodeIdx);
        }
    }
}

void CodechalEncodeHevcBase::SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams)
{
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = CODECHAL_ENCODE_MODE_HEVC;
    indObjBaseAddrParams.presMvObjectBuffer = &m_resMbCodeSurface;
    indObjBaseAddrParams.dwMvObjectOffset = m_mvOffset;
    indObjBaseAddrParams.dwMvObjectSize = m_mbCodeSize - m_mvOffset;
    indObjBaseAddrParams.presPakBaseObjectBuffer = &m_resBitstreamBuffer;
    indObjBaseAddrParams.dwPakBaseObjectSize =m_bitstreamUpperBound;
    indObjBaseAddrParams.presPakTileSizeStasBuffer =  nullptr;
    indObjBaseAddrParams.dwPakTileSizeStasBufferSize = 0;
    indObjBaseAddrParams.dwPakTileSizeRecordOffset = 0;
}

void CodechalEncodeHevcBase::SetHcpQmStateParams(MHW_VDBOX_QM_PARAMS& fqmParams, MHW_VDBOX_QM_PARAMS& qmParams)
{
    MOS_ZeroMemory(&fqmParams, sizeof(fqmParams));
    fqmParams.Standard = CODECHAL_HEVC;
    fqmParams.pHevcIqMatrix = (PMHW_VDBOX_HEVC_QM_PARAMS)m_hevcIqMatrixParams;

    MOS_ZeroMemory(&qmParams, sizeof(qmParams));
    qmParams.Standard = CODECHAL_HEVC;
    qmParams.pHevcIqMatrix = (PMHW_VDBOX_HEVC_QM_PARAMS)m_hevcIqMatrixParams;
}

void CodechalEncodeHevcBase::SetHcpPicStateParams(MHW_VDBOX_HEVC_PIC_STATE& picStateParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    picStateParams = {};
    picStateParams.pHevcEncSeqParams     = m_hevcSeqParams;
    picStateParams.pHevcEncPicParams     = m_hevcPicParams;
    picStateParams.bSAOEnable            = m_hevcSeqParams->SAO_enabled_flag ? (m_hevcSliceParams->slice_sao_luma_flag || m_hevcSliceParams->slice_sao_chroma_flag) : 0;
    picStateParams.bUseVDEnc = m_vdencEnabled;
    picStateParams.bNotFirstPass = m_vdencEnabled && !IsFirstPass() ;
    picStateParams.bHevcRdoqEnabled = m_hevcRdoqEnabled ? (m_pictureCodingType == I_TYPE ? m_hevcIFrameRdoqEnabled : 1) : 0;
    picStateParams.bRDOQIntraTUDisable   = m_hevcRdoqEnabled && (1 != m_hevcSeqParams->TargetUsage);
    picStateParams.wRDOQIntraTUThreshold = (uint16_t)m_rdoqIntraTuThreshold;
    picStateParams.bTransformSkipEnable  = m_hevcPicParams->transform_skip_enabled_flag;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_rdoqIntraTuOverride)
    {
        int32_t RDOQIntraTUThreshold = 0;

        if (m_rdoqIntraTuThresholdOverride >= 100)
        {
            RDOQIntraTUThreshold = 65535;
        }
        else if (m_rdoqIntraTuThresholdOverride > 0)
        {
            uint32_t frameWidth = (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);
            uint32_t frameHeight = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);
            int32_t frameSize = frameWidth * frameHeight;
            RDOQIntraTUThreshold = (((frameSize * m_rdoqIntraTuThreshold) / 100) >> 8);

            uint16_t numPipe = m_numVdbox;
            if ((m_hevcPicParams->num_tile_columns_minus1 + 1) > m_numVdbox)
            {
                numPipe = 1;
            }
            if ((m_hevcPicParams->tiles_enabled_flag) && (numPipe > 1))
            {
                RDOQIntraTUThreshold /= numPipe;
            }
            if (RDOQIntraTUThreshold > 65535)
            {
                RDOQIntraTUThreshold = 65535;
            }
        }

        picStateParams.bRDOQIntraTUDisable = m_rdoqIntraTuDisableOverride;
        picStateParams.wRDOQIntraTUThreshold = (uint16_t)RDOQIntraTUThreshold;
    }
#endif

    picStateParams.currPass = m_currPass;
    if (CodecHalIsFeiEncode(m_codecFunction) && m_hevcFeiPicParams && m_hevcFeiPicParams->dwMaxFrameSize)
    {
        picStateParams.deltaQp = m_hevcFeiPicParams->pDeltaQp;
        picStateParams.maxFrameSize = m_hevcFeiPicParams->dwMaxFrameSize;
    }
}

MOS_STATUS CodechalEncodeHevcBase::SetBatchBufferForPakSlices()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_useBatchBufferForPakSlices         = m_singleTaskPhaseSupported && m_singleTaskPhaseSupportedInPak;
    m_batchBufferForPakSlicesStartOffset = 0;

    if (m_useBatchBufferForPakSlices)
    {
        if (IsFirstPass())
        {
            // The same buffer is used for all slices for all passes
            uint32_t batchBufferForPakSlicesSize =
                (m_numPasses + 1) * m_numSlices * m_sliceStatesSize;

            CODECHAL_ENCODE_ASSERT(batchBufferForPakSlicesSize);

            if (batchBufferForPakSlicesSize >
                (uint32_t)m_batchBufferForPakSlices[m_currPakSliceIdx].iSize)
            {
                if (m_batchBufferForPakSlices[m_currPakSliceIdx].iSize)
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReleaseBatchBufferForPakSlices(m_currPakSliceIdx));
                }

                CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBatchBufferForPakSlices(
                    m_numSlices,
                    m_numPasses));
            }
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_LockBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_currPakSliceIdx]));
        m_batchBufferForPakSlicesStartOffset = IsFirstPass() ? 0 : (uint32_t)m_batchBufferForPakSlices[m_currPakSliceIdx].iCurrent;
    }

    return eStatus;
}

void CodechalEncodeHevcBase::CreateMhwParams()
{
    m_sliceStateParams = MOS_New(MHW_VDBOX_HEVC_SLICE_STATE);
    m_pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS);
    m_pipeBufAddrParams = MOS_New(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS);
}

void CodechalEncodeHevcBase::SetHcpSliceStateCommonParams(MHW_VDBOX_HEVC_SLICE_STATE& sliceStateParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    sliceStateParams = {};
    sliceStateParams.presDataBuffer = &m_resMbCodeSurface;
    sliceStateParams.pHevcPicIdx           = &(m_picIdx[0]);
    sliceStateParams.pEncodeHevcSeqParams  = m_hevcSeqParams;
    sliceStateParams.pEncodeHevcPicParams  = m_hevcPicParams;
    sliceStateParams.pBsBuffer = &m_bsBuffer;
    sliceStateParams.ppNalUnitParams       = m_nalUnitParams;
    sliceStateParams.bBrcEnabled           = m_brcEnabled;
    sliceStateParams.dwHeaderBytesInserted = 0;
    sliceStateParams.dwHeaderDummyBytes = 0;
    sliceStateParams.pRefIdxMapping        = m_refIdxMapping;
    sliceStateParams.bIsLowDelay           = m_lowDelay;
    sliceStateParams.RoundingIntra         = m_roundingIntra;
    sliceStateParams.RoundingInter         = m_roundingInter;
}

void CodechalEncodeHevcBase::SetHcpSliceStateParams(
    MHW_VDBOX_HEVC_SLICE_STATE& sliceStateParams,
    PCODEC_ENCODER_SLCDATA slcData,
    uint32_t currSlcIdx)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    sliceStateParams.pEncodeHevcSliceParams    = &m_hevcSliceParams[currSlcIdx];
    sliceStateParams.dwDataBufferOffset = slcData[currSlcIdx].CmdOffset;
    sliceStateParams.dwOffset = slcData[currSlcIdx].SliceOffset;
    sliceStateParams.dwLength = slcData[currSlcIdx].BitSize;
    sliceStateParams.uiSkipEmulationCheckCount = slcData[currSlcIdx].SkipEmulationByteCount;
    sliceStateParams.dwSliceIndex = currSlcIdx;
    sliceStateParams.bLastSlice = (currSlcIdx == m_numSlices - 1);
    sliceStateParams.bFirstPass = IsFirstPass();
    sliceStateParams.bLastPass = IsLastPass();
    sliceStateParams.bInsertBeforeSliceHeaders = (currSlcIdx == 0);
    sliceStateParams.bSaoLumaFlag              = (m_hevcSeqParams->SAO_enabled_flag) ? m_hevcSliceParams[currSlcIdx].slice_sao_luma_flag : 0;
    sliceStateParams.bSaoChromaFlag            = (m_hevcSeqParams->SAO_enabled_flag) ? m_hevcSliceParams[currSlcIdx].slice_sao_chroma_flag : 0;
    sliceStateParams.DeblockingFilterDisable   = m_hevcSliceParams[currSlcIdx].slice_deblocking_filter_disable_flag;
    sliceStateParams.TcOffsetDiv2              = m_hevcSliceParams[currSlcIdx].tc_offset_div2;
    sliceStateParams.BetaOffsetDiv2            = m_hevcSliceParams[currSlcIdx].beta_offset_div2;


    if (m_useBatchBufferForPakSlices)
    {
        sliceStateParams.pBatchBufferForPakSlices =
            &m_batchBufferForPakSlices[m_currPakSliceIdx];
        sliceStateParams.bSingleTaskPhaseSupported = true;
        sliceStateParams.dwBatchBufferForPakSlicesStartOffset = m_batchBufferForPakSlicesStartOffset;
    }

    if (m_hevcPicParams->transform_skip_enabled_flag)
    {
        CalcTransformSkipParameters(sliceStateParams.EncodeHevcTransformSkipParams);
    }
}

MOS_STATUS CodechalEncodeHevcBase::AddHcpRefIdxCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcPicParams);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = params->pEncodeHevcPicParams;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams = params->pEncodeHevcSliceParams;

    if (hevcSlcParams->slice_type != CODECHAL_ENCODE_HEVC_I_SLICE)
    {
        MHW_VDBOX_HEVC_REF_IDX_PARAMS refIdxParams;

        refIdxParams.CurrPic = hevcPicParams->CurrReconstructedPic;
        refIdxParams.isEncode = true;
        refIdxParams.ucList = LIST_0;
        refIdxParams.ucNumRefForList = hevcSlcParams->num_ref_idx_l0_active_minus1 + 1;
        eStatus = MOS_SecureMemcpy(&refIdxParams.RefPicList, sizeof(refIdxParams.RefPicList),
            &hevcSlcParams->RefPicList, sizeof(hevcSlcParams->RefPicList));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        refIdxParams.hevcRefList = (void**)m_refList;
        refIdxParams.poc_curr_pic = hevcPicParams->CurrPicOrderCnt;
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            refIdxParams.poc_list[i] = hevcPicParams->RefFramePOCList[i];
        }

        refIdxParams.pRefIdxMapping = params->pRefIdxMapping;
        refIdxParams.RefFieldPicFlag = 0; // there is no interlaced support in encoder
        refIdxParams.RefBottomFieldFlag = 0; // there is no interlaced support in encoder

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(cmdBuffer, batchBuffer, &refIdxParams));

        if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
        {
            refIdxParams.ucList = LIST_1;
            refIdxParams.ucNumRefForList = hevcSlcParams->num_ref_idx_l1_active_minus1 + 1;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(cmdBuffer, batchBuffer, &refIdxParams));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AddHcpPakInsertNALUs(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ppNalUnitParams);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    //insert AU, SPS, PSP headers before first slice header
    if (params->bInsertBeforeSliceHeaders)
    {
        uint32_t maxBytesInPakInsertObjCmd = ((2 << 11) - 1) * 4; // 12 bits for Length field in PAK_INSERT_OBJ cmd

        for (auto i = 0; i < HEVC_MAX_NAL_UNIT_TYPE; i++)
        {
            uint32_t nalunitPosiSize = params->ppNalUnitParams[i]->uiSize;
            uint32_t nalunitPosiOffset = params->ppNalUnitParams[i]->uiOffset;

            while (nalunitPosiSize > 0)
            {
                uint32_t bitSize = MOS_MIN(maxBytesInPakInsertObjCmd * 8, nalunitPosiSize * 8);
                uint32_t offSet = nalunitPosiOffset;

                MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
                MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
                pakInsertObjectParams.bEmulationByteBitsInsert = params->ppNalUnitParams[i]->bInsertEmulationBytes;
                pakInsertObjectParams.uiSkipEmulationCheckCount = params->ppNalUnitParams[i]->uiSkipEmulationCheckCount;
                pakInsertObjectParams.pBsBuffer = params->pBsBuffer;
                pakInsertObjectParams.dwBitSize = bitSize;
                pakInsertObjectParams.dwOffset = offSet;
                pakInsertObjectParams.pBatchBufferForPakSlices = batchBuffer;
                pakInsertObjectParams.bVdencInUse = params->bVdencInUse;

                if (nalunitPosiSize > maxBytesInPakInsertObjCmd)
                {
                    nalunitPosiSize -= maxBytesInPakInsertObjCmd;
                    nalunitPosiOffset += maxBytesInPakInsertObjCmd;
                }
                else
                {
                    nalunitPosiSize = 0;
                }

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(cmdBuffer, &pakInsertObjectParams));
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AddHcpPakInsertSliceHeader(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    // Insert slice header
    MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
    MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
    pakInsertObjectParams.bLastHeader = true;
    pakInsertObjectParams.bEmulationByteBitsInsert = true;
    pakInsertObjectParams.pBatchBufferForPakSlices = batchBuffer;

    // App does the slice header packing, set the skip count passed by the app
    pakInsertObjectParams.uiSkipEmulationCheckCount = params->uiSkipEmulationCheckCount;
    pakInsertObjectParams.pBsBuffer = params->pBsBuffer;
    pakInsertObjectParams.dwBitSize = params->dwLength;
    pakInsertObjectParams.dwOffset = params->dwOffset;
    pakInsertObjectParams.bVdencInUse = params->bVdencInUse;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(
        cmdBuffer,
        &pakInsertObjectParams));

    return eStatus;
}

CodechalEncodeHevcBase::CodechalEncodeHevcBase(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncoderState(hwInterface, debugInterface, standardInfo)
{
    // initialze class members
    MOS_ZeroMemory(&m_resDeblockingFilterRowStoreScratchBuffer, sizeof(m_resDeblockingFilterRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resDeblockingFilterTileRowStoreScratchBuffer, sizeof(m_resDeblockingFilterTileRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resDeblockingFilterColumnRowStoreScratchBuffer, sizeof(m_resDeblockingFilterColumnRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resMetadataLineBuffer, sizeof(m_resMetadataLineBuffer));
    MOS_ZeroMemory(&m_resMetadataTileLineBuffer, sizeof(m_resMetadataTileLineBuffer));
    MOS_ZeroMemory(&m_resMetadataTileColumnBuffer, sizeof(m_resMetadataTileColumnBuffer));
    MOS_ZeroMemory(&m_resSaoLineBuffer, sizeof(m_resSaoLineBuffer));
    MOS_ZeroMemory(&m_resSaoTileLineBuffer, sizeof(m_resSaoTileLineBuffer));
    MOS_ZeroMemory(&m_resSaoTileColumnBuffer, sizeof(m_resSaoTileColumnBuffer));
    MOS_ZeroMemory(&m_resLcuBaseAddressBuffer, sizeof(m_resLcuBaseAddressBuffer));
    MOS_ZeroMemory(&m_resLcuIldbStreamOutBuffer, sizeof(m_resLcuIldbStreamOutBuffer));
    MOS_ZeroMemory(&m_resSaoStreamOutBuffer, sizeof(m_resSaoStreamOutBuffer));
    MOS_ZeroMemory(&m_resFrameStatStreamOutBuffer, sizeof(m_resFrameStatStreamOutBuffer));
    MOS_ZeroMemory(&m_resSseSrcPixelRowStoreBuffer, sizeof(m_resSseSrcPixelRowStoreBuffer));
    MOS_ZeroMemory(m_batchBufferForPakSlices, sizeof(m_batchBufferForPakSlices));

    MOS_ZeroMemory(m_refSync, sizeof(m_refSync));
    MOS_ZeroMemory(m_refIdxMapping, sizeof(m_refIdxMapping));

    MOS_ZeroMemory(m_currUsedRefPic, sizeof(m_currUsedRefPic));
    ;
    MOS_ZeroMemory(m_picIdx, sizeof(m_picIdx));
    MOS_ZeroMemory(m_refList, sizeof(m_refList));

    MOS_ZeroMemory(&m_s4XMeMvDataBuffer, sizeof(m_s4XMeMvDataBuffer));
    MOS_ZeroMemory(&m_s16XMeMvDataBuffer, sizeof(m_s16XMeMvDataBuffer));
    MOS_ZeroMemory(&m_s32XMeMvDataBuffer, sizeof(m_s32XMeMvDataBuffer));
    MOS_ZeroMemory(&m_s4XMeDistortionBuffer, sizeof(m_s4XMeDistortionBuffer));
    MOS_ZeroMemory(&m_mbQpDataSurface, sizeof(m_mbQpDataSurface));

    m_fieldScalingOutputInterleaved = false;
    m_interlacedFieldDisabled = true;
    m_firstField = true;     // Each frame is treated as the first field

    m_userFeatureKeyReport = true;
    m_useCmScalingKernel = true;
    m_codecGetStatusReportDefined = true;       // Codec specific GetStatusReport is implemented.

    m_vdboxOneDefaultUsed = true;
}

MOS_STATUS CodechalEncodeHevcBase::CalculatePictureStateCommandSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->GetHxxStateCommandSize(
            CODECHAL_ENCODE_MODE_HEVC,
            &m_defaultPictureStatesSize,
            &m_defaultPicturePatchListSize,
            &stateCmdSizeParams));

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AddHcpPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    *m_pipeBufAddrParams = {};
    SetHcpPipeBufAddrParams(*m_pipeBufAddrParams);
#ifdef _MMC_SUPPORTED
    m_mmcState->SetPipeBufAddr(m_pipeBufAddrParams);
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeBufAddrCmd(cmdBuffer, m_pipeBufAddrParams));

    return eStatus;
}

short CodechalEncodeHevcBase::ComputeTemporalDifferent(CODEC_PICTURE  refPic)
{
    short diff_poc = 0;

    if (!CodecHal_PictureIsInvalid(refPic))
    {
        diff_poc = m_hevcPicParams->CurrPicOrderCnt - m_hevcPicParams->RefFramePOCList[refPic.FrameIdx];

        if (diff_poc < -16)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("POC out of range, it will be clipped.");
            diff_poc = -16;
        }
        else
            if (diff_poc > 16)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("POC out of range, it will be clipped.");
                diff_poc = 16;
            }
    }

    return diff_poc;
}

MOS_STATUS CodechalEncodeHevcBase::InitSurfaceCodecParams1D(
    PCODECHAL_SURFACE_CODEC_PARAMS  surfaceCodecParams,
    PMOS_RESOURCE                   buffer,
    uint32_t                        size,
    uint32_t                        offset,
    uint32_t                        cacheabilityControl,
    uint32_t                        bindingTableOffset,
    bool                            isWritable)
{
    if (surfaceCodecParams == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    MOS_ZeroMemory(surfaceCodecParams, sizeof(*surfaceCodecParams));
    surfaceCodecParams->presBuffer = buffer;
    surfaceCodecParams->dwSize = size;
    surfaceCodecParams->dwOffset = offset;
    surfaceCodecParams->dwCacheabilityControl = cacheabilityControl;
    surfaceCodecParams->dwBindingTableOffset = bindingTableOffset;
    surfaceCodecParams->bIsWritable =
        surfaceCodecParams->bRenderTarget = isWritable;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::InitSurfaceCodecParams2D(
    PCODECHAL_SURFACE_CODEC_PARAMS  surfaceCodecParams,
    PMOS_SURFACE                    surface,
    uint32_t                        cacheabilityControl,
    uint32_t                        bindingTableOffset,
    uint32_t                        verticalLineStride,
    bool                            isWritable)
{
    if (surfaceCodecParams == nullptr)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    MOS_ZeroMemory(surfaceCodecParams, sizeof(*surfaceCodecParams));
    surfaceCodecParams->bIs2DSurface = true;
    surfaceCodecParams->bMediaBlockRW = true; // Use media block RW for DP 2D surface access
    surfaceCodecParams->psSurface = surface;
    surfaceCodecParams->dwCacheabilityControl = cacheabilityControl;
    surfaceCodecParams->dwBindingTableOffset = bindingTableOffset;
    surfaceCodecParams->dwVerticalLineStride = verticalLineStride;
    surfaceCodecParams->bIsWritable =
        surfaceCodecParams->bRenderTarget = isWritable;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::AllocateResources4xME(
    HmeParams *param)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(param);

    if (!m_encEnabled || !m_hmeSupported)
    {
        return eStatus;
    }

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    allocParamsForBuffer2D.Format   = Format_Buffer_2D;

    MOS_ZeroMemory(param->ps4xMeMvDataBuffer, sizeof(MOS_SURFACE));
    param->ps4xMeMvDataBuffer->TileType      = MOS_TILE_LINEAR;
    param->ps4xMeMvDataBuffer->bArraySpacing = true;
    param->ps4xMeMvDataBuffer->Format        = Format_Buffer_2D;
    param->ps4xMeMvDataBuffer->dwWidth       = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64); // MediaBlockRW requires pitch multiple of 64 bytes when linear.
    param->ps4xMeMvDataBuffer->dwHeight      = (m_downscaledHeightInMb4x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
    param->ps4xMeMvDataBuffer->dwPitch       = param->ps4xMeMvDataBuffer->dwWidth;

    allocParamsForBuffer2D.dwWidth  = param->ps4xMeMvDataBuffer->dwWidth;
    allocParamsForBuffer2D.dwHeight = param->ps4xMeMvDataBuffer->dwHeight;
    allocParamsForBuffer2D.pBufName = "4xME MV Data Buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBuffer2D,
        &param->ps4xMeMvDataBuffer->OsResource);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate 4xME MV Data Buffer.");
        return eStatus;
    }

    CleanUpResource(&param->ps4xMeMvDataBuffer->OsResource, &allocParamsForBuffer2D);

    if (param->b4xMeDistortionBufferSupported)
    {
        uint32_t ajustedHeight =
            m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT * SCALE_FACTOR_4x;
        uint32_t downscaledFieldHeightInMB4x =
            CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(((ajustedHeight + 1) >> 1) / 4);

        MOS_ZeroMemory(param->ps4xMeDistortionBuffer, sizeof(MOS_SURFACE));
        param->ps4xMeDistortionBuffer->TileType      = MOS_TILE_LINEAR;
        param->ps4xMeDistortionBuffer->bArraySpacing = true;
        param->ps4xMeDistortionBuffer->Format        = Format_Buffer_2D;
        param->ps4xMeDistortionBuffer->dwWidth       = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);
        param->ps4xMeDistortionBuffer->dwHeight      = 2 * MOS_ALIGN_CEIL((downscaledFieldHeightInMB4x * 4 * 10), 8);
        param->ps4xMeDistortionBuffer->dwPitch       = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);

        allocParamsForBuffer2D.dwWidth  = param->ps4xMeDistortionBuffer->dwWidth;
        allocParamsForBuffer2D.dwHeight = param->ps4xMeDistortionBuffer->dwHeight;
        allocParamsForBuffer2D.pBufName = "4xME Distortion Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBuffer2D,
            &param->ps4xMeDistortionBuffer->OsResource);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate 4xME Distortion Buffer.");
            return eStatus;
        }
        CleanUpResource(&param->ps4xMeDistortionBuffer->OsResource, &allocParamsForBuffer2D);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AllocateResources16xME(
    HmeParams  *param)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(param);

    if (!m_encEnabled || !m_hmeSupported)
    {
        return eStatus;
    }

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    allocParamsForBuffer2D.Format   = Format_Buffer_2D;

    if (m_16xMeSupported)
    {
        MOS_ZeroMemory(param->ps16xMeMvDataBuffer, sizeof(MOS_SURFACE));
        param->ps16xMeMvDataBuffer->TileType      = MOS_TILE_LINEAR;
        param->ps16xMeMvDataBuffer->bArraySpacing = true;
        param->ps16xMeMvDataBuffer->Format        = Format_Buffer_2D;
        param->ps16xMeMvDataBuffer->dwWidth       = MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64); // MediaBlockRW requires pitch multiple of 64 bytes when linear
        param->ps16xMeMvDataBuffer->dwHeight      = (m_downscaledHeightInMb16x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
        param->ps16xMeMvDataBuffer->dwPitch       = param->ps16xMeMvDataBuffer->dwWidth;

        allocParamsForBuffer2D.dwWidth  = param->ps16xMeMvDataBuffer->dwWidth;
        allocParamsForBuffer2D.dwHeight = param->ps16xMeMvDataBuffer->dwHeight;
        allocParamsForBuffer2D.pBufName = "16xME MV Data Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBuffer2D,
            &param->ps16xMeMvDataBuffer->OsResource);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate 16xME MV Data Buffer.");
            return eStatus;
        }
        CleanUpResource(&param->ps16xMeMvDataBuffer->OsResource, &allocParamsForBuffer2D);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::AllocateResources32xME(
    HmeParams  *param)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(param);

    if (!m_encEnabled || !m_hmeSupported)
    {
        return eStatus;
    }

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    allocParamsForBuffer2D.Format   = Format_Buffer_2D;

    if (m_32xMeSupported)
    {
        MOS_ZeroMemory(param->ps32xMeMvDataBuffer, sizeof(MOS_SURFACE));
        param->ps32xMeMvDataBuffer->TileType      = MOS_TILE_LINEAR;
        param->ps32xMeMvDataBuffer->bArraySpacing = true;
        param->ps32xMeMvDataBuffer->Format        = Format_Buffer_2D;
        param->ps32xMeMvDataBuffer->dwWidth       = MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64); // MediaBlockRW requires pitch multiple of 64 bytes when linear
        param->ps32xMeMvDataBuffer->dwHeight      = (m_downscaledHeightInMb32x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
        param->ps32xMeMvDataBuffer->dwPitch       = param->ps32xMeMvDataBuffer->dwWidth;

        allocParamsForBuffer2D.dwWidth  = param->ps32xMeMvDataBuffer->dwWidth;
        allocParamsForBuffer2D.dwHeight = param->ps32xMeMvDataBuffer->dwHeight;
        allocParamsForBuffer2D.pBufName = "32xME MV Data Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBuffer2D,
            &param->ps32xMeMvDataBuffer->OsResource);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate 32xME MV Data Buffer\n", __FUNCTION__);
            return eStatus;
        }
        CleanUpResource(&param->ps32xMeMvDataBuffer->OsResource, &allocParamsForBuffer2D);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeHevcBase::DestroyMEResources(
    HmeParams *param)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(param);

    if (nullptr != param->ps16xMeMvDataBuffer)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &param->ps16xMeMvDataBuffer->OsResource);
    }

    if (nullptr != param->ps32xMeMvDataBuffer)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &param->ps32xMeMvDataBuffer->OsResource);
    }

    if (nullptr != param->ps4xMeDistortionBuffer)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &param->ps4xMeDistortionBuffer->OsResource);
    }

    if (nullptr != param->ps4xMeMvDataBuffer)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &param->ps4xMeMvDataBuffer->OsResource);
    }

    if (nullptr != param->presMvAndDistortionSumSurface)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            param->presMvAndDistortionSumSurface);
    }

    return eStatus;
}

void CodechalEncodeHevcBase::MotionEstimationDisableCheck()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_downscaledWidth4x < m_minScaledDimension || m_downscaledWidthInMb4x < m_minScaledDimensionInMb ||
        m_downscaledHeight4x < m_minScaledDimension || m_downscaledHeightInMb4x < m_minScaledDimensionInMb)
    {
        if (m_downscaledWidth4x < m_minScaledDimension || m_downscaledWidthInMb4x < m_minScaledDimensionInMb)
        {
            m_downscaledWidth4x = m_minScaledDimension;
            m_downscaledWidthInMb4x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth4x);
        }
        if (m_downscaledHeight4x < m_minScaledDimension || m_downscaledHeightInMb4x < m_minScaledDimensionInMb)
        {
            m_downscaledHeight4x = m_minScaledDimension;
            m_downscaledHeightInMb4x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_downscaledHeight4x);
        }
    }

    if (m_downscaledWidth16x < m_minScaledDimension || m_downscaledWidthInMb16x < m_minScaledDimensionInMb ||
        m_downscaledHeight16x < m_minScaledDimension || m_downscaledHeightInMb16x < m_minScaledDimensionInMb)
    {
        if (m_downscaledWidth16x < m_minScaledDimension || m_downscaledWidthInMb16x < m_minScaledDimensionInMb)
        {
            m_downscaledWidth16x = m_minScaledDimension;
            m_downscaledWidthInMb16x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth16x);
        }
        if (m_downscaledHeight16x < m_minScaledDimension || m_downscaledHeightInMb16x < m_minScaledDimensionInMb)
        {
            m_downscaledHeight16x = m_minScaledDimension;
            m_downscaledHeightInMb16x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_downscaledHeight16x);
        }
    }

    if (m_downscaledWidth32x < m_minScaledDimension || m_downscaledWidthInMb32x < m_minScaledDimensionInMb)
    {
        m_downscaledWidth32x = m_minScaledDimension;
        m_downscaledWidthInMb32x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth32x);
    }
    if (m_downscaledHeight32x < m_minScaledDimension || m_downscaledHeightInMb32x < m_minScaledDimensionInMb)
    {
        m_downscaledHeight32x = m_minScaledDimension;
        m_downscaledHeightInMb32x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_downscaledHeight32x);
    }

}

MOS_STATUS CodechalEncodeHevcBase::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeKernelFunctions());

    return eStatus;
}

uint32_t CodecHalHevcEncode_GetBitstreamBufferSize(
    uint32_t    frameWidth,
    uint32_t    frameHeight,
    uint8_t     chromaFormat,
    bool        is10Bits)
{
    // 4:2:0 uncompression buffer size

    frameHeight = (frameHeight * 3) / (is10Bits ? 1 : 2);

    if (chromaFormat == HCP_CHROMA_FORMAT_YUV422)
    {
        frameWidth = (frameWidth * 8) / 6; //4:2:2 v.s 4:2:0
    }
    else if (chromaFormat == HCP_CHROMA_FORMAT_YUV444)
    {
        frameWidth = (frameWidth * 12) / 6; //4:4:4 v.s 4:2:0
    }

    return frameWidth * frameHeight;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncodeHevcBase::DumpSeqParams(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS seqParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSeqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(seqParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "wFrameWidthInMinCbMinus1 = " << +seqParams->wFrameWidthInMinCbMinus1 << std::endl;
    oss << "wFrameHeightInMinCbMinus1 = " << +seqParams->wFrameHeightInMinCbMinus1 << std::endl;
    oss << "general_profile_idc = " << +seqParams->general_profile_idc << std::endl;
    oss << "Level = " << +seqParams->Level << std::endl;
    oss << "general_tier_flag = " << +seqParams->general_tier_flag << std::endl;
    oss << "GopPicSize = " << +seqParams->GopPicSize << std::endl;
    oss << "GopRefDist = " << +seqParams->GopRefDist << std::endl;
    oss << "GopOptFlag = " << +seqParams->GopOptFlag << std::endl;
    oss << "TargetUsage = " << +seqParams->TargetUsage << std::endl;
    oss << "RateControlMethod = " << +seqParams->RateControlMethod << std::endl;
    oss << "TargetBitRate = " << +seqParams->TargetBitRate << std::endl;
    oss << "MaxBitRate = " << +seqParams->MaxBitRate << std::endl;
    oss << "MinBitRate = " << +seqParams->MinBitRate << std::endl;
    oss << "FramesRate.Numerator = " << +seqParams->FrameRate.Numerator << std::endl;
    oss << "FramesRate.Denominator = " << +seqParams->FrameRate.Denominator << std::endl;
    oss << "InitVBVBufferFullnessInBit = " << +seqParams->InitVBVBufferFullnessInBit << std::endl;
    oss << "VBVBufferSizeInBit = " << +seqParams->VBVBufferSizeInBit << std::endl;
    oss << "bResetBRC = " << +seqParams->bResetBRC << std::endl;
    oss << "GlobalSearch = " << +seqParams->GlobalSearch << std::endl;
    oss << "LocalSearch = " << +seqParams->LocalSearch << std::endl;
    oss << "EarlySkip = " << +seqParams->EarlySkip << std::endl;
    oss << "MBBRC = " << +seqParams->MBBRC << std::endl;
    oss << "ParallelBRC = " << +seqParams->ParallelBRC << std::endl;
    oss << "SliceSizeControl = " << +seqParams->SliceSizeControl << std::endl;
    oss << "SourceFormat = " << +seqParams->SourceFormat << std::endl;
    oss << "SourceBitDepth = " << +seqParams->SourceBitDepth << std::endl;
    oss << "QpAdjustment = " << +seqParams->QpAdjustment << std::endl;
    oss << "ROIValueInDeltaQP = " << +seqParams->ROIValueInDeltaQP << std::endl;
    oss << "BlockQPforNonRectROI = " << +seqParams->BlockQPforNonRectROI << std::endl;
    oss << "EnableTileBasedEncode = " << +seqParams->EnableTileBasedEncode << std::endl;
    oss << "bAutoMaxPBFrameSizeForSceneChange = " << +seqParams->bAutoMaxPBFrameSizeForSceneChange << std::endl;
    oss << "EnableStreamingBufferLLC = " << +seqParams->EnableStreamingBufferLLC << std::endl;
    oss << "EnableStreamingBufferDDR  = " << +seqParams->EnableStreamingBufferDDR << std::endl;
    oss << "LowDelayMode = " << +seqParams->LowDelayMode << std::endl;
    oss << "DisableHRDConformance = " << +seqParams->DisableHRDConformance << std::endl;
    oss << "HierarchicalFlag = " << +seqParams->HierarchicalFlag << std::endl;
    oss << "UserMaxIFrameSize = " << +seqParams->UserMaxIFrameSize << std::endl;
    oss << "UserMaxPBFrameSize = " << +seqParams->UserMaxPBFrameSize << std::endl;
    oss << "ICQQualityFactor = " << +seqParams->ICQQualityFactor << std::endl;
    oss << "NumB = " << +seqParams->NumOfBInGop[0] << std::endl;
    oss << "NumB1 = " << +seqParams->NumOfBInGop[1] << std::endl;
    oss << "NumB2 = " << +seqParams->NumOfBInGop[2] << std::endl;
    oss << "UserMaxIFrameSize = " << +seqParams->UserMaxIFrameSize << std::endl;
    oss << "UserMaxPBFrameSize = " << +seqParams->UserMaxPBFrameSize << std::endl;
    oss << "ICQQualityFactor = " << +seqParams->ICQQualityFactor << std::endl;
    oss << "scaling_list_enable_flag = " << +seqParams->scaling_list_enable_flag << std::endl;
    oss << "sps_temporal_mvp_enable_flag = " << +seqParams->sps_temporal_mvp_enable_flag << std::endl;
    oss << "strong_intra_smoothing_enable_flag = " << +seqParams->strong_intra_smoothing_enable_flag << std::endl;
    oss << "amp_enabled_flag = " << +seqParams->amp_enabled_flag << std::endl;
    oss << "SAO_enabled_flag = " << +seqParams->SAO_enabled_flag << std::endl;
    oss << "pcm_enabled_flag = " << +seqParams->pcm_enabled_flag << std::endl;
    oss << "pcm_loop_filter_disable_flag = " << +seqParams->pcm_loop_filter_disable_flag << std::endl;
    oss << "chroma_format_idc = " << +seqParams->chroma_format_idc << std::endl;
    oss << "separate_colour_plane_flag = " << +seqParams->separate_colour_plane_flag << std::endl;
    oss << "palette_mode_enabled_flag = " << +seqParams->palette_mode_enabled_flag << std::endl;
    oss << "RGBEncodingEnable = " << +seqParams->RGBEncodingEnable << std::endl;
    oss << "PrimaryChannelForRGBEncoding = " << +seqParams->PrimaryChannelForRGBEncoding << std::endl;
    oss << "SecondaryChannelForRGBEncoding = " << +seqParams->SecondaryChannelForRGBEncoding << std::endl;

    oss << "log2_max_coding_block_size_minus3 = " << +seqParams->log2_max_coding_block_size_minus3 << std::endl;
    oss << "log2_min_coding_block_size_minus3 = " << +seqParams->log2_min_coding_block_size_minus3 << std::endl;
    oss << "log2_max_transform_block_size_minus2 = " << +seqParams->log2_max_transform_block_size_minus2 << std::endl;
    oss << "log2_min_transform_block_size_minus2 = " << +seqParams->log2_min_transform_block_size_minus2 << std::endl;
    oss << "max_transform_hierarchy_depth_intra = " << +seqParams->max_transform_hierarchy_depth_intra << std::endl;
    oss << "max_transform_hierarchy_depth_inter = " << +seqParams->max_transform_hierarchy_depth_inter << std::endl;
    oss << "log2_min_PCM_cb_size_minus3 = " << +seqParams->log2_min_PCM_cb_size_minus3 << std::endl;
    oss << "log2_max_PCM_cb_size_minus3 = " << +seqParams->log2_max_PCM_cb_size_minus3 << std::endl;
    oss << "bit_depth_luma_minus8 = " << +seqParams->bit_depth_luma_minus8 << std::endl;
    oss << "bit_depth_chroma_minus8 = " << +seqParams->bit_depth_chroma_minus8 << std::endl;
    oss << "pcm_sample_bit_depth_luma_minus1 = " << +seqParams->pcm_sample_bit_depth_luma_minus1 << std::endl;
    oss << "pcm_sample_bit_depth_chroma_minus1 = " << +seqParams->pcm_sample_bit_depth_chroma_minus1 << std::endl;
    oss << "Video Surveillance Mode = " << +seqParams->bVideoSurveillance << std::endl;
    oss << "Frame Size Tolerance = " << +seqParams->FrameSizeTolerance << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSeqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "SeqParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::DumpPicParams(
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "CurrOriginalPic = " << +picParams->CurrOriginalPic.FrameIdx << std::endl;
    oss << "CurrReconstructedPic = " << +picParams->CurrReconstructedPic.FrameIdx << std::endl;
    oss << "CollocatedRefPicIndex = " << +picParams->CollocatedRefPicIndex << std::endl;

    for (uint16_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; ++i)
    {
        oss << "RefFrameList[" << +i << "] = " << +picParams->RefFrameList[i].FrameIdx << std::endl;
    }

    oss << "CurrPicOrderCnt = " << +picParams->CurrPicOrderCnt << std::endl;

    for (uint16_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; ++i)
    {
        oss << "RefFramePOCList[" << +i << "] = " << +picParams->RefFramePOCList[i] << std::endl;
    }

    oss << "CodingType = " << +picParams->CodingType << std::endl;
    oss << "HierarchLevelPlus1 = " << +picParams->HierarchLevelPlus1 << std::endl; 
    oss << "NumSlices = " << +picParams->NumSlices << std::endl;
    oss << "tiles_enabled_flag = " << +picParams->tiles_enabled_flag << std::endl;

    if (picParams->tiles_enabled_flag)
    {
        oss << "num_tile_columns = " << +picParams->num_tile_columns_minus1 + 1 << std::endl;
        for (uint32_t i = 0; i <= picParams->num_tile_columns_minus1; i++)
        {
            oss << "tile_column_width[" << +i << "] = " << +picParams->tile_column_width[i] << std::endl;
        }
        oss << "num_tile_rows = " << +picParams->num_tile_rows_minus1 + 1 << std::endl;
        for (uint32_t i = 0; i <= picParams->num_tile_rows_minus1; i++)
        {
            oss << "tile_row_height[" << +i << "] = " << +picParams->tile_row_height[i] << std::endl;
        }
    }

    oss << "entropy_coding_sync_enabled_flag = " << +picParams->entropy_coding_sync_enabled_flag << std::endl;
    oss << "sign_data_hiding_flag = " << +picParams->sign_data_hiding_flag << std::endl;
    oss << "constrained_intra_pred_flag = " << +picParams->constrained_intra_pred_flag << std::endl;
    oss << "transform_skip_enabled_flag = " << +picParams->transform_skip_enabled_flag << std::endl;
    oss << "transquant_bypass_enabled_flag = " << +picParams->transquant_bypass_enabled_flag << std::endl;
    oss << "cu_qp_delta_enabled_flag = " << +picParams->cu_qp_delta_enabled_flag << std::endl;
    oss << "weighted_pred_flag = " << +picParams->weighted_pred_flag << std::endl;
    oss << "weighted_bipred_flag = " << +picParams->weighted_bipred_flag << std::endl;
    oss << "bEnableGPUWeightedPrediction = " << +picParams->bEnableGPUWeightedPrediction << std::endl;
    oss << "loop_filter_across_slices_flag = " << +picParams->loop_filter_across_slices_flag << std::endl;
    oss << "loop_filter_across_tiles_flag = " << +picParams->loop_filter_across_tiles_flag << std::endl;
    oss << "scaling_list_data_present_flag = " << +picParams->scaling_list_data_present_flag << std::endl;
    oss << "dependent_slice_segments_enabled_flag = " << +picParams->dependent_slice_segments_enabled_flag << std::endl;
    oss << "bLastPicInSeq = " << +picParams->bLastPicInSeq << std::endl;
    oss << "bLastPicInStream = " << +picParams->bLastPicInStream << std::endl;
    oss << "bUseRawPicForRef = " << +picParams->bUseRawPicForRef << std::endl;
    oss << "bEmulationByteInsertion = " << +picParams->bEmulationByteInsertion << std::endl;
    oss << "bEnableRollingIntraRefresh = " << +picParams->bEnableRollingIntraRefresh << std::endl;
    oss << "BRCPrecision = " << +picParams->BRCPrecision << std::endl;
    oss << "bScreenContent = " << +picParams->bScreenContent << std::endl;
    oss << "QpY = " << +picParams->QpY << std::endl;
    oss << "diff_cu_qp_delta_depth = " << +picParams->diff_cu_qp_delta_depth << std::endl;
    oss << "pps_cb_qp_offset = " << +picParams->pps_cb_qp_offset << std::endl;
    oss << "pps_cr_qp_offset = " << +picParams->pps_cr_qp_offset << std::endl;
    oss << "num_tile_columns_minus1 = " << +picParams->num_tile_columns_minus1 << std::endl;
    oss << "num_tile_rows_minus1 = " << +picParams->num_tile_rows_minus1 << std::endl;
    oss << "log2_parallel_merge_level_minus2 = " << +picParams->log2_parallel_merge_level_minus2 << std::endl;
    oss << "num_ref_idx_l0_default_active_minus1 = " << +picParams->num_ref_idx_l0_default_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_default_active_minus1 = " << +picParams->num_ref_idx_l1_default_active_minus1 << std::endl;
    oss << "LcuMaxBitsizeAllowed = " << +picParams->LcuMaxBitsizeAllowed << std::endl;
    oss << "IntraInsertionLocation = " << +picParams->IntraInsertionLocation << std::endl;
    oss << "IntraInsertionSize = " << +picParams->IntraInsertionSize << std::endl;
    oss << "QpDeltaForInsertedIntra = " << +picParams->QpDeltaForInsertedIntra << std::endl;
    oss << "StatusReportFeedbackNumber = " << +picParams->StatusReportFeedbackNumber << std::endl;
    oss << "slice_pic_parameter_set_id = " << +picParams->slice_pic_parameter_set_id << std::endl;
    oss << "nal_unit_type = " << +picParams->nal_unit_type << std::endl;
    oss << "MaxSliceSizeInBytes = " << +picParams->MaxSliceSizeInBytes << std::endl;
    oss << "NumROI = " << +picParams->NumROI << std::endl;

    for (uint8_t i = 0; i < 16; ++i)
    {
        oss << "ROI[" << +i << "] = ";
        oss << picParams->ROI[i].Top << " ";
        oss << picParams->ROI[i].Bottom << " ";
        oss << picParams->ROI[i].Left << " ";
        oss << picParams->ROI[i].Right << " ";
        oss << picParams->ROI[i].PriorityLevelOrDQp << std::endl;
    }
    oss << "MaxDeltaQp = " << +picParams->MaxDeltaQp << std::endl;
    oss << "MinDeltaQp = " << +picParams->MinDeltaQp << std::endl;
    oss << "NumDirtyRects = " << +picParams->NumDirtyRects << std::endl;

    if ((picParams->NumDirtyRects > 0) && picParams->pDirtyRect)
    {
        for (uint16_t i = 0; i < picParams->NumDirtyRects; i++)
        {
            oss << "pDirtyRect[" << +i << "].Bottom = " << +picParams->pDirtyRect[i].Bottom << std::endl;
            oss << "pDirtyRect[" << +i << "].Top = " << +picParams->pDirtyRect[i].Top << std::endl;
            oss << "pDirtyRect[" << +i << "].Left = " << +picParams->pDirtyRect[i].Left << std::endl;
            oss << "pDirtyRect[" << +i << "].Right = " << +picParams->pDirtyRect[i].Right << std::endl;
        }
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "PicNum"
                << " = \"" << m_debugInterface->m_bufferDumpFrameNum << "\"" << std::endl;
            ofs << "PicParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::DumpFeiPicParams(
    CodecEncodeHevcFeiPicParams *feiPicParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrFeiPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(feiPicParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "NumMVPredictorsL0 = " << +feiPicParams->NumMVPredictorsL0 << std::endl;
    oss << "NumMVPredictorsL1 = " << +feiPicParams->NumMVPredictorsL1 << std::endl;
    oss << "bCTBCmdCuRecordEnable = " << +feiPicParams->bCTBCmdCuRecordEnable << std::endl;
    oss << "bDistortionEnable = " << +feiPicParams->bDistortionEnable << std::endl;
    oss << "SearchPath = " << +feiPicParams->SearchPath << std::endl;
    oss << "LenSP = " << +feiPicParams->LenSP << std::endl;
    oss << "MultiPredL0 = " << +feiPicParams->MultiPredL0 << std::endl;
    oss << "MultiPredL1 = " << +feiPicParams->MultiPredL1 << std::endl;
    oss << "SubPelMode = " << +feiPicParams->SubPelMode << std::endl;
    oss << "AdaptiveSearch = " << +feiPicParams->AdaptiveSearch << std::endl;
    oss << "MVPredictorInput = " << +feiPicParams->MVPredictorInput << std::endl;
    oss << "bPerBlockQP = " << +feiPicParams->bPerBlockQP << std::endl;
    oss << "bPerCTBInput = " << +feiPicParams->bPerCTBInput << std::endl;
    oss << "bColocatedCTBDistortion = " << +feiPicParams->bColocatedCTBDistortion << std::endl;
    oss << "bForceLCUSplit = " << +feiPicParams->bForceLCUSplit << std::endl;
    oss << "bEnableCU64Check = " << +feiPicParams->bEnableCU64Check << std::endl;
    oss << "bEnableCU64AmpCheck = " << +feiPicParams->bEnableCU64AmpCheck << std::endl;
    oss << "bCU64SkipCheckOnly = " << +feiPicParams->bCU64SkipCheckOnly << std::endl;
    oss << "RefWidth = " << +feiPicParams->RefWidth << std::endl;
    oss << "RefHeight = " << +feiPicParams->RefHeight << std::endl;
    oss << "SearchWindow = " << +feiPicParams->SearchWindow << std::endl;
    oss << "MaxNumIMESearchCenter = " << +feiPicParams->MaxNumIMESearchCenter << std::endl;
    oss << "NumConcurrentEncFramePartition = " << +feiPicParams->NumConcurrentEncFramePartition << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufFeiPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "PicNum"
                << " = \"" << m_debugInterface->m_bufferDumpFrameNum << "\"" << std::endl;
            ofs << "FeiPicParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::DumpSliceParams(
    PCODEC_HEVC_ENCODE_SLICE_PARAMS   sliceParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    m_debugInterface->m_sliceId = sliceParams->slice_id;  // set here for constructing debug file name

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "# DDI Parameters:" << std::endl;
    oss << "slice_segment_address = " << +sliceParams->slice_segment_address << std::endl;
    oss << "NumLCUsInSlice = " << +sliceParams->NumLCUsInSlice << std::endl;

    // RefPicList (2 x CODEC_MAX_NUM_REF_FRAME_HEVC)
    for (uint8_t i = 0; i < 2; ++i)
    {
        for (uint8_t j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; ++j)
        {
            oss << "RefPicList[" << +i << "][" << +j << "] = " << +sliceParams->RefPicList[i][j].PicEntry << std::endl;
        }
    }

    oss << "num_ref_idx_l0_active_minus1 = " << +sliceParams->num_ref_idx_l0_active_minus1 << std::endl;
    oss << "num_ref_idx_l1_active_minus1 = " << +sliceParams->num_ref_idx_l1_active_minus1 << std::endl;
    oss << "bLastSliceOfPic = " << +sliceParams->bLastSliceOfPic << std::endl;
    oss << "dependent_slice_segment_flag = " << +sliceParams->dependent_slice_segment_flag << std::endl;
    oss << "slice_temporal_mvp_enable_flag = " << +sliceParams->slice_temporal_mvp_enable_flag << std::endl;
    oss << "slice_type = " << +sliceParams->slice_type << std::endl;
    oss << "slice_sao_luma_flag = " << +sliceParams->slice_sao_luma_flag << std::endl;
    oss << "slice_sao_chroma_flag = " << +sliceParams->slice_sao_chroma_flag << std::endl;
    oss << "mvd_l1_zero_flag = " << +sliceParams->mvd_l1_zero_flag << std::endl;
    oss << "cabac_init_flag = " << +sliceParams->cabac_init_flag << std::endl;
    oss << "slice_deblocking_filter_disable_flag = " << +sliceParams->slice_deblocking_filter_disable_flag << std::endl;
    oss << "collocated_from_l0_flag = " << +sliceParams->collocated_from_l0_flag << std::endl;
    oss << "slice_qp_delta = " << +sliceParams->slice_qp_delta << std::endl;
    oss << "slice_cb_qp_offset = " << +sliceParams->slice_cb_qp_offset << std::endl;
    oss << "slice_cr_qp_offset = " << +sliceParams->slice_cr_qp_offset << std::endl;
    oss << "beta_offset_div2 = " << +sliceParams->beta_offset_div2 << std::endl;
    oss << "tc_offset_div2 = " << +sliceParams->tc_offset_div2 << std::endl;
    oss << "luma_log2_weight_denom = " << +sliceParams->luma_log2_weight_denom << std::endl;
    oss << "delta_chroma_log2_weight_denom = " << +sliceParams->delta_chroma_log2_weight_denom << std::endl;

    for (uint8_t i = 0; i < 2; ++i)
    {
        for (uint8_t j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; ++j)
        {
            oss << "luma_offset[" << +i << "][" << +j << "] = " << +sliceParams->luma_offset[i][j] << std::endl;

            oss << "delta_luma_weight[" << +i << "][" << +j << "] = " << +sliceParams->delta_luma_weight[i][j] << std::endl;
        }
    }

    for (uint8_t i = 0; i < 2; ++i)
    {
        for (uint8_t j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; ++j)
        {
            for (uint8_t k = 0; k < 2; ++k)
            {
                oss << "chroma_offset[" << +i << "][" << +j << "][" << +k << "] = " << +sliceParams->chroma_offset[i][j][k] << std::endl;
                oss << "delta_chroma_weight[" << +i << "][" << +j << "][" << +k << "] = " << +sliceParams->delta_chroma_weight[i][j][k] << std::endl;
            }
        }
    }

    oss << "PredWeightTableBitOffset = " << +sliceParams->PredWeightTableBitOffset << std::endl;
    oss << "PredWeightTableBitLength = " << +sliceParams->PredWeightTableBitLength << std::endl;
    oss << "MaxNumMergeCand = " << +sliceParams->MaxNumMergeCand << std::endl;
    oss << "slice_id = " << +sliceParams->slice_id << std::endl;
    oss << "SliceHeaderByteOffset = " << +sliceParams->SliceHeaderByteOffset << std::endl;
    oss << "BitLengthSliceHeaderStartingPortion = " << +sliceParams->BitLengthSliceHeaderStartingPortion << std::endl;
    oss << "SliceSAOFlagBitOffset = " << +sliceParams->SliceSAOFlagBitOffset << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSlcParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDriverUltDump))
    {
        if (!m_debugInterface->m_ddiFileName.empty())
        {
            std::ofstream ofs(m_debugInterface->m_ddiFileName, std::ios::app);
            ofs << "SlcParamFile"
                << " = \"" << m_debugInterface->m_fileName << "\"" << std::endl;
            ofs.close();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::DumpMbEncPakOutput(PCODEC_REF_LIST currRefList, CodechalDebugInterface* debugInterface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(currRefList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(debugInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        &currRefList->resRefMbCodeBuffer,
        CodechalDbgAttr::attrOutput,
        "PakObj",
        m_mvOffset,
        0,
        CODECHAL_MEDIA_STATE_ENC_NORMAL));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        &currRefList->resRefMbCodeBuffer,
        CodechalDbgAttr::attrOutput,
        "CuRecord",
        m_mbCodeSize - m_mvOffset,
        m_mvOffset,
        CODECHAL_MEDIA_STATE_ENC_NORMAL));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::DumpFrameStatsBuffer(CodechalDebugInterface* debugInterface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(debugInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(debugInterface->DumpBuffer(
        &m_resFrameStatStreamOutBuffer,
        CodechalDbgAttr::attrFrameState,
        "FrameStatus",
        m_sizeOfHcpPakFrameStats,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::CreateHevcPar()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_hevcPar = MOS_New(EncodeHevcPar));
        MOS_ZeroMemory(m_hevcPar, sizeof(EncodeHevcPar));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::DestroyHevcPar()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpSeqParFile());
    MOS_Delete(m_hevcPar);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::PopulateConstParam()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    m_encodeParState->m_isConstDumped = true; //Not use this for now, force ture so it can return directly.

    if (m_encodeParState->m_isConstDumped)
    {
        return MOS_STATUS_SUCCESS;
    }

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);
    oss << "StartFrameNum                           = 0" << std::endl;
    oss << "SliceStartLCU                           = 0" << std::endl;
    oss << "LowDelay                                = 1" << std::endl;
    oss << "EnableBAsRefs                           = 1" << std::endl;
    oss << "HMECoarseRefPic                         = 0" << std::endl;
    oss << "RefListModforWeightPred                 = 0" << std::endl;
    oss << "EnableStatistics                        = 0xf00" << std::endl;
    oss << "OutputQualityType                       = 3" << std::endl;
    oss << "StaticFrameZMVPercent                   = 80" << std::endl;
    oss << "HMEStreamInRefCost                      = 50" << std::endl;

    m_encodeParState->m_isConstDumped = true;

    const char *fileName = m_debugInterface->CreateFileName(
        "EncodeSequence",
        "EncodePar",
        CodechalDbgExtType::par);

    std::ofstream ofs(fileName, std::ios::app);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeHevcBase::PopulateDdiParam(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    hevcSlcParams)
{
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_I_SLICE)
    {
        m_hevcPar->ISliceQP                             = hevcPicParams->QpY + hevcSlcParams->slice_qp_delta;
        m_hevcPar->StartFrameNum                        = 0;
        m_hevcPar->ProfileIDC                           = hevcSeqParams->general_profile_idc;
        m_hevcPar->LevelIDC                             = hevcSeqParams->Level;        
        m_hevcPar->NumP                                 = 0;
        m_hevcPar->NumB                                 = hevcSeqParams->GopPicSize - 1;
        m_hevcPar->NumSlices                            = hevcPicParams->NumSlices;
        m_hevcPar->SliceStartLCU                        = 0;
        m_hevcPar->Log2MaxCUSize                        = hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
        m_hevcPar->Log2MinCUSize                        = hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
        m_hevcPar->Log2MaxTUSize                        = hevcSeqParams->log2_max_transform_block_size_minus2 + 2;
        m_hevcPar->Log2MinTUSize                        = hevcSeqParams->log2_min_transform_block_size_minus2 + 2;

        uint32_t inputChromaFormatIDC = 0;
        switch (m_rawSurface.Format)
        {
        case (Format_NV12):
        case (Format_P010):
            inputChromaFormatIDC = 1;
            break;
        case (Format_YUY2):
        case (Format_Y210):
            inputChromaFormatIDC = 2;
            break;
        case (Format_Y410):
        case (Format_AYUV):
            inputChromaFormatIDC = 3;
            break;
        default:
            inputChromaFormatIDC = 0;
            break;
        }
        m_hevcPar->InputChromaFormatIDC                 = inputChromaFormatIDC;
        m_hevcPar->ChromaFormatIDC                      = hevcSeqParams->chroma_format_idc;
        m_hevcPar->InputBitDepthLuma                    = hevcSeqParams->SourceBitDepth ? 10 : 8;
        m_hevcPar->InputBitDepthChroma                  = hevcSeqParams->SourceBitDepth ? 10 : 8;
        m_hevcPar->OutputBitDepthLuma                   = hevcSeqParams->bit_depth_luma_minus8 + 8;
        m_hevcPar->OutputBitDepthChroma                 = hevcSeqParams->bit_depth_chroma_minus8 + 8;
        m_hevcPar->InternalBitDepthLuma                 = hevcSeqParams->bit_depth_luma_minus8 + 8;
        m_hevcPar->InternalBitDepthChroma               = hevcSeqParams->bit_depth_chroma_minus8 + 8;
        m_hevcPar->Log2TUMaxDepthInter                  = hevcSeqParams->max_transform_hierarchy_depth_inter;
        m_hevcPar->Log2TUMaxDepthIntra                  = hevcSeqParams->max_transform_hierarchy_depth_intra;
        m_hevcPar->Log2ParallelMergeLevel               = hevcPicParams->log2_parallel_merge_level_minus2 + 2;
        m_hevcPar->TransquantBypassEnableFlag           = hevcPicParams->transquant_bypass_enabled_flag;
        m_hevcPar->TransformSkipEnabledFlag             = hevcPicParams->transform_skip_enabled_flag;
        m_hevcPar->TSDecisionEnabledFlag                = hevcPicParams->transform_skip_enabled_flag;
        m_hevcPar->TemporalMvpEnableFlag                = hevcSeqParams->sps_temporal_mvp_enable_flag;
        m_hevcPar->CollocatedFromL0Flag                 = hevcSlcParams->collocated_from_l0_flag;
        m_hevcPar->CollocatedRefIdx                     = hevcPicParams->CollocatedRefPicIndex;
        m_hevcPar->MvdL1ZeroFlag                        = hevcSlcParams->mvd_l1_zero_flag;
        m_hevcPar->AmpEnabledFlag                       = hevcSeqParams->amp_enabled_flag;
        m_hevcPar->CuQpDeltaEnabledFlag                 = hevcPicParams->cu_qp_delta_enabled_flag;
        m_hevcPar->DiffCuQpDeltaDepth                   = hevcPicParams->diff_cu_qp_delta_depth;
        m_hevcPar->ChromaCbQpOffset                     = hevcPicParams->pps_cb_qp_offset;
        m_hevcPar->ChromaCrQpOffset                     = hevcPicParams->pps_cr_qp_offset;
        m_hevcPar->DeblockingTc                         = hevcSlcParams->tc_offset_div2;
        m_hevcPar->DeblockingIDC                        = hevcSlcParams->slice_deblocking_filter_disable_flag;
        m_hevcPar->LoopFilterAcrossSlicesEnabledFlag    = hevcPicParams->loop_filter_across_slices_flag;
        m_hevcPar->SignDataHidingFlag                   = hevcPicParams->sign_data_hiding_flag;
        m_hevcPar->CabacInitFlag                        = hevcSlcParams->cabac_init_flag;
        m_hevcPar->ConstrainedIntraPred                 = hevcPicParams->constrained_intra_pred_flag;
        m_hevcPar->LowDelay                             = 1;
        m_hevcPar->EnableBAsRefs                        = 1;
        m_hevcPar->BitRate                              = hevcSeqParams->TargetBitRate * 1000;
        m_hevcPar->MaxBitRate                           = hevcSeqParams->MaxBitRate * 1000;
        m_hevcPar->VbvSzInBit                           = hevcSeqParams->VBVBufferSizeInBit;
        m_hevcPar->InitVbvFullnessInBit                 = hevcSeqParams->InitVBVBufferFullnessInBit;
        m_hevcPar->CuRC                                 = hevcSeqParams->MBBRC;
        m_hevcPar->EnableMultipass                      = 1;
        m_hevcPar->MaxNumPakPassesI                     = m_numPasses + 1;
        m_hevcPar->MaxNumPakPassesPB                    = m_numPasses + 1;
        m_hevcPar->UserMaxIFrame                        = hevcSeqParams->UserMaxIFrameSize;
        m_hevcPar->UserMaxPBFrame                       = hevcSeqParams->UserMaxPBFrameSize;
        m_hevcPar->FrameRateM                           = hevcSeqParams->FrameRate.Numerator;
        m_hevcPar->FrameRateD                           = hevcSeqParams->FrameRate.Denominator;
        m_hevcPar->IntraRefreshEnable                   = hevcPicParams->bEnableRollingIntraRefresh ? 1 : 0;
        m_hevcPar->IntraRefreshMode                     = hevcPicParams->bEnableRollingIntraRefresh == 2 ? 1 : 0;
        m_hevcPar->IntraRefreshSizeIn32x32              = hevcPicParams->IntraInsertionSize;
        m_hevcPar->IntraRefreshDeltaQP                  = hevcPicParams->QpDeltaForInsertedIntra;
        m_hevcPar->HMECoarseRefPic                      = 0;
        //Weighted Prediction
        m_hevcPar->FadeDetectionEnable                  = hevcPicParams->bEnableGPUWeightedPrediction;
        m_hevcPar->WeightedPred                         = hevcPicParams->weighted_pred_flag;
        m_hevcPar->WeightedBiPred                       = hevcPicParams->weighted_bipred_flag;
        m_hevcPar->Log2LumaDenom                        = hevcSlcParams->luma_log2_weight_denom;
        m_hevcPar->Log2ChromaDenom                      = hevcSlcParams->delta_chroma_log2_weight_denom;
        m_hevcPar->RefListModforWeightPred              = hevcPicParams->bEnableGPUWeightedPrediction;

        m_hevcPar->OutputQualityType                    = 3;
        m_hevcPar->SliceSizeCtrl                        = hevcSeqParams->SliceSizeControl;
        m_hevcPar->SliceSizeThreshold                   = hevcPicParams->MaxSliceSizeInBytes; //?
        m_hevcPar->MaxSliceSize                         = hevcPicParams->MaxSliceSizeInBytes;
        m_hevcPar->VDEncMode                            = m_hevcSeqParams->TargetUsage == 7 ? 0 : 1;
        //TU related
        m_hevcPar->DisableIntraLuma4x4Tu                = m_hevcSeqParams->TargetUsage == 7 ? 1 : 0;
        m_hevcPar->HMERef1Disable                       = m_hevcSeqParams->TargetUsage == 7 ? 1 : 0;
        m_hevcPar->NumBetaPreditors                     = m_hevcSeqParams->TargetUsage == 7 ? 2 : 4;
        m_hevcPar->MaxNumImePredictor                   = m_hevcSeqParams->TargetUsage == 7 ? 4 : 8;
        m_hevcPar->NumMergeCandidateCu8x8               = m_hevcSeqParams->TargetUsage == 7 ? 0 : 1;
        m_hevcPar->NumMergeCandidateCu16x16             = 2;
        m_hevcPar->NumMergeCandidateCu32x32             = m_hevcSeqParams->TargetUsage == 7 ? 2 : 3;
        m_hevcPar->NumMergeCandidateCu64x64             = m_hevcSeqParams->TargetUsage == 7 ? 2 : 4;
        m_hevcPar->MaxRefIdxL0                          = MOS_MAX(m_hevcPar->MaxRefIdxL0, hevcSlcParams->num_ref_idx_l0_active_minus1);
        m_hevcPar->MaxRefIdxL1                          = MOS_MAX(m_hevcPar->MaxRefIdxL1, hevcSlcParams->num_ref_idx_l1_active_minus1);

        uint8_t brcMethod = 0;
        uint8_t brcType = 0;

        if (CodecHalIsRateControlBrc(hevcSeqParams->RateControlMethod, CODECHAL_HEVC))
        {
            brcMethod = 2;

            switch (hevcSeqParams->RateControlMethod)
            {
            case RATECONTROL_ICQ:
                brcMethod = m_vdencEnabled ? 2 : 3;
                brcType = 0;
                break;
            case RATECONTROL_QVBR:
                brcMethod = m_vdencEnabled ? 2 : 4;
                brcType = 2;
                break;
            case RATECONTROL_CBR:
                brcType = 1;
                break;
            case RATECONTROL_VBR:
                brcType = 2;
                break;
            case RATECONTROL_VCM:
                brcType = m_vdencEnabled ? 4 : 3;
                break;
            default:
                brcMethod = 0;
                brcType = 0;
                break;
            }

            if (hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
            {
                // low delay mode
                brcType = 8;
            }
        }

        m_hevcPar->CRFQualityFactor                     = hevcSeqParams->ICQQualityFactor;
        m_hevcPar->BRCMethod                            = brcMethod;
        m_hevcPar->BRCType                              = brcType;
        m_hevcPar->SAOEnabledFlag                       = hevcSlcParams->slice_sao_luma_flag | hevcSlcParams->slice_sao_chroma_flag;
        m_hevcPar->IntraFrameRDOQEnabledFlag            = m_hevcRdoqEnabled;
        m_hevcPar->InterFrameRDOQEnabledFlag            = m_hevcRdoqEnabled;
        m_hevcPar->StreamInEn                           = m_vdencStreamInEnabled;
        m_hevcPar->StreamInMvPredictorRef               = m_hevcSeqParams->TargetUsage == 1 ? 7 : 0;
        m_hevcPar->StaticFrameZMVPercent                = 80;
        m_hevcPar->HMEStreamInRefCost                   = 50;
        m_hevcPar->IntraPeriod                          = hevcSeqParams->GopPicSize;
        m_hevcPar->BGOPSize                             = 0;
    }
    else if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_P_SLICE)
    {
        m_hevcPar->StreamInEn                           = m_vdencStreamInEnabled;
        m_hevcPar->PSliceQP                             = hevcPicParams->QpY + hevcSlcParams->slice_qp_delta;
        m_hevcPar->MaxRefIdxL0                          = MOS_MAX(m_hevcPar->MaxRefIdxL0, static_cast<uint32_t>(hevcSlcParams->num_ref_idx_l0_active_minus1 + 1));
        m_hevcPar->MaxRefIdxL1                          = MOS_MAX(m_hevcPar->MaxRefIdxL1, static_cast<uint32_t>(hevcSlcParams->num_ref_idx_l1_active_minus1 + 1));

    }
    else if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
    {
        m_hevcPar->StreamInEn                           = m_vdencStreamInEnabled;
        m_hevcPar->BSliceQP                             = hevcPicParams->QpY + hevcSlcParams->slice_qp_delta;
        m_hevcPar->MaxRefIdxL0                          = MOS_MAX(m_hevcPar->MaxRefIdxL0, static_cast<uint32_t>(hevcSlcParams->num_ref_idx_l0_active_minus1 + 1));
        m_hevcPar->MaxRefIdxL1                          = MOS_MAX(m_hevcPar->MaxRefIdxL1, static_cast<uint32_t>(hevcSlcParams->num_ref_idx_l1_active_minus1 + 1));
    }

    return MOS_STATUS_SUCCESS;
}
#endif
