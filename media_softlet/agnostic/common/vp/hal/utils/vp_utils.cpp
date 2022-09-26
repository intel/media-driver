/*
* Copyright (c) 2021-2022, Intel Corporation
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

#include <algorithm>
#include <map>
#include <utility>
#include "vp_utils.h"
#include "vp_common.h"
#include "hal_kerneldll_next.h"
#include "media_common_defs.h"
#include "media_user_setting_configure.h"
#include "mos_interface.h"

MOS_SURFACE VpUtils::VpHalConvertVphalSurfaceToMosSurface(PVPHAL_SURFACE surface)
{
    VP_FUNC_CALL();

    MOS_SURFACE outSurface = {};
    MOS_ZeroMemory(&outSurface, sizeof(MOS_SURFACE));

    if (surface)
    {
        outSurface.OsResource        = surface->OsResource;
        outSurface.Format            = surface->Format;
        outSurface.dwWidth           = surface->dwWidth;
        outSurface.dwHeight          = surface->dwHeight;
        outSurface.TileType          = surface->TileType;
        outSurface.TileModeGMM       = surface->TileModeGMM;
        outSurface.bGMMTileEnabled   = surface->bGMMTileEnabled;
        outSurface.dwDepth           = surface->dwDepth;
        outSurface.dwPitch           = surface->dwPitch;
        outSurface.dwSlicePitch      = surface->dwSlicePitch;
        outSurface.dwOffset          = surface->dwOffset;
        outSurface.bCompressible     = surface->bCompressible;
        outSurface.bIsCompressed     = surface->bIsCompressed;
        outSurface.CompressionMode   = surface->CompressionMode;
        outSurface.CompressionFormat = surface->CompressionFormat;
    }

    return outSurface;
}

VPHAL_COLORPACK VpUtils::GetSurfaceColorPack(
    MOS_FORMAT format)
{
    VPHAL_COLORPACK colorPack = VPHAL_COLORPACK_UNKNOWN;

    VP_FUNC_CALL();
    
    static const std::map<const VPHAL_COLORPACK, const MosFormatArray> colorPackMap =
    {
        {VPHAL_COLORPACK_400, {Format_Y8,
                               Format_Y16S,
                               Format_Y16U,
                               Format_400P}},
        {VPHAL_COLORPACK_420, {Format_IMC1,
                               Format_IMC2,
                               Format_IMC3,
                               Format_IMC4,
                               Format_NV12,
                               Format_NV21,
                               Format_YV12,
                               Format_I420,
                               Format_IYUV,
                               Format_P010,
                               Format_P016}},
        {VPHAL_COLORPACK_422, {Format_YUY2,
                               Format_YUYV,
                               Format_YVYU,
                               Format_UYVY,
                               Format_VYUY,
                               Format_P208,
                               Format_422H,
                               Format_422V,
                               Format_Y210,
                               Format_Y216}},
        {VPHAL_COLORPACK_444, {Format_A8R8G8B8,
                               Format_X8R8G8B8,
                               Format_A8B8G8R8,
                               Format_X8B8G8R8,
                               Format_A16B16G16R16,
                               Format_A16R16G16B16,
                               Format_R5G6B5,
                               Format_R8G8B8,
                               Format_RGBP,
                               Format_BGRP,
                               Format_Y416,
                               Format_Y410,
                               Format_AYUV,
                               Format_AUYV,
                               Format_444P,
                               Format_R10G10B10A2,
                               Format_B10G10R10A2,
                               Format_A16B16G16R16F,
                               Format_A16R16G16B16F}},
        {VPHAL_COLORPACK_411, {Format_411P}}
    };
    for (auto mapIt = colorPackMap.begin(); mapIt != colorPackMap.end(); mapIt++)
    {
        auto &iFormatArray = mapIt->second;
        auto  iter         = std::find(iFormatArray.begin(), iFormatArray.end(), format);
        if (iter == iFormatArray.end())
        {
            continue;
        }
        return mapIt->first;
    }

    VP_PUBLIC_ASSERTMESSAGE("Input format color pack unknown.");
    return VPHAL_COLORPACK_UNKNOWN;
}

bool VpUtils::GetCscMatrixForRender8Bit(
    VPHAL_COLOR_SAMPLE_8  *output,
    VPHAL_COLOR_SAMPLE_8  *input,
    VPHAL_CSPACE          srcCspace,
    VPHAL_CSPACE          dstCspace)
{
    float   pfCscMatrix[12] = {0};
    int32_t iCscMatrix[12]  = {0};
    bool    bResult         = false;
    int32_t i               = 0;

    KernelDll_GetCSCMatrix(srcCspace, dstCspace, pfCscMatrix);

    // convert float to fixed point format for the 3x4 matrix
    for (i = 0; i < 12; i++)
    {
        // multiply by 2^20 and round up
        iCscMatrix[i] = (int32_t)((pfCscMatrix[i] * 1048576.0f) + 0.5f);
    }

    bResult = GetCscMatrixForRender8BitWithCoeff(output, input, srcCspace, dstCspace, iCscMatrix);

    return bResult;
}

MOS_STATUS VpUtils::ReAllocateSurface(
    PMOS_INTERFACE        osInterface,
    PVPHAL_SURFACE        surface,
    PCCHAR                surfaceName,
    MOS_FORMAT            format,
    MOS_GFXRES_TYPE       defaultResType,
    MOS_TILE_TYPE         defaultTileType,
    uint32_t              dwWidth,
    uint32_t              dwHeight,
    bool                  bCompressible,
    MOS_RESOURCE_MMC_MODE compressionMode,
    bool                  *bAllocated,
    MOS_HW_RESOURCE_DEF   resUsageType,
    MOS_TILE_MODE_GMM     tileModeByForce,
    Mos_MemPool           memType,
    bool                  isNotLockable)
{
    MOS_STATUS              eStatus      = MOS_STATUS_SUCCESS;
    VPHAL_GET_SURFACE_INFO  info         = {};
    MOS_ALLOC_GFXRES_PARAMS allocParams  = {};
    MOS_GFXRES_FREE_FLAGS   resFreeFlags = {0};

    //---------------------------------
    VP_PUBLIC_ASSERT(osInterface);
    VP_PUBLIC_ASSERT(surface);
    //---------------------------------

    *bAllocated = false;

    // bCompressible should be compared with bCompressible since it is inited by bCompressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (!Mos_ResourceIsNull(&surface->OsResource) &&
        (surface->dwWidth == dwWidth) &&
        (surface->dwHeight == dwHeight) &&
        (surface->Format == format) &&
        (surface->bCompressible == bCompressible) &&
        (surface->CompressionMode == compressionMode) &&
        (surface->TileType == defaultTileType))
    {
        goto finish;
    }

    if (osInterface->bOptimizeCpuTiming &&
        (defaultResType == MOS_GFXRES_BUFFER) &&
        (surface->dwWidth >= dwWidth))
    {
        goto finish;
    }

    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    VpHal_AllocParamsInitType(&allocParams, surface, defaultResType, defaultTileType);

    allocParams.dwWidth            = dwWidth;
    allocParams.dwHeight           = dwHeight;
    allocParams.Format             = format;
    allocParams.bIsCompressible    = bCompressible;
    allocParams.CompressionMode    = compressionMode;
    allocParams.pBufName           = surfaceName;
    allocParams.dwArraySize        = 1;
    allocParams.ResUsageType       = resUsageType;
    allocParams.m_tileModeByForce  = tileModeByForce;
    allocParams.dwMemType          = memType;
    allocParams.Flags.bNotLockable = isNotLockable;

    // Delete resource if already allocated
    //if free the compressed surface, need set the sync dealloc flag as 1 for sync dealloc for aux table update
    if (IsSyncFreeNeededForMMCSurface(surface, osInterface))
    {
        resFreeFlags.SynchronousDestroy = 1;
        VP_PUBLIC_NORMALMESSAGE("Set SynchronousDestroy flag for compressed resource %s", surfaceName);
    }
    osInterface->pfnFreeResourceWithFlag(osInterface, &(surface->OsResource), resFreeFlags.Value);

    // Allocate surface
    VP_PUBLIC_CHK_STATUS(osInterface->pfnAllocateResource(
        osInterface,
        &allocParams,
        &surface->OsResource));

    // Get surface information
    MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));

    // Pre-set to get surface info
    surface->Format = format;

    VP_PUBLIC_CHK_STATUS(VpHal_GetSurfaceInfo(osInterface, &info, surface));

    *bAllocated = true;

    MT_LOG7(MT_VP_HAL_REALLOC_SURF, MT_NORMAL, MT_VP_HAL_INTER_SURF_TYPE, surfaceName ? *((int64_t *)surfaceName) : 0, MT_SURF_WIDTH, dwWidth, MT_SURF_HEIGHT, dwHeight, MT_SURF_MOS_FORMAT, format, MT_SURF_TILE_MODE, surface->TileModeGMM, MT_SURF_COMP_ABLE, surface->bCompressible, MT_SURF_COMP_MODE, surface->CompressionMode);

finish:
    VP_PUBLIC_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

bool VpUtils::IsSyncFreeNeededForMMCSurface(PVPHAL_SURFACE surface, PMOS_INTERFACE osInterface)
{
    if (nullptr == surface || nullptr == osInterface)
    {
        return false;
    }

    //Compressed surface aux table update is after resource dealloction, aux table update need wait the WLs complete
    //the sync deallocation flag will make sure deallocation API return after all surface related WL been completed and resource been destroyed by OS
    auto *pSkuTable = osInterface->pfnGetSkuTable(osInterface);
    if (pSkuTable &&
        MEDIA_IS_SKU(pSkuTable, FtrE2ECompression) &&                                    //Compression enabled platform
        !MEDIA_IS_SKU(pSkuTable, FtrFlatPhysCCS) &&                                      //NOT DGPU compression
        ((surface->bCompressible) && (surface->CompressionMode != MOS_MMC_DISABLED)))  //Compressed enabled surface
    {
        return true;
    }

    return false;
}

bool VpUtils::GetCscMatrixForRender8BitWithCoeff(
    VPHAL_COLOR_SAMPLE_8  *output,
    VPHAL_COLOR_SAMPLE_8  *input,
    VPHAL_CSPACE          srcCspace,
    VPHAL_CSPACE          dstCspace,
    int32_t               *iCscMatrix)
{
    bool    bResult = true;
    int32_t a = 0, r = 0, g = 0, b = 0;
    int32_t y1 = 0, u1 = 0, v1 = 0;

    y1 = r = input->YY;
    u1 = g = input->Cb;
    v1 = b = input->Cr;
    a      = input->Alpha;

    if (srcCspace == dstCspace)
    {
        // no conversion needed
        if ((dstCspace == CSpace_sRGB) || (dstCspace == CSpace_stRGB) || IS_COLOR_SPACE_BT2020_RGB(dstCspace))
        {
            output->A = (uint8_t)a;
            output->R = (uint8_t)r;
            output->G = (uint8_t)g;
            output->B = (uint8_t)b;
        }
        else
        {
            output->a = (uint8_t)a;
            output->Y = (uint8_t)y1;
            output->U = (uint8_t)u1;
            output->V = (uint8_t)v1;
        }
    }
    else
    {
        // conversion needed
        r = (y1 * iCscMatrix[0] + u1 * iCscMatrix[1] +
                v1 * iCscMatrix[2] + iCscMatrix[3] + 0x00080000) >>
            20;
        g = (y1 * iCscMatrix[4] + u1 * iCscMatrix[5] +
                v1 * iCscMatrix[6] + iCscMatrix[7] + 0x00080000) >>
            20;
        b = (y1 * iCscMatrix[8] + u1 * iCscMatrix[9] +
                v1 * iCscMatrix[10] + iCscMatrix[11] + 0x00080000) >>
            20;

        switch (dstCspace)
        {
        case CSpace_sRGB:
            output->A = (uint8_t)a;
            output->R = MOS_MIN(MOS_MAX(0, r), 255);
            output->G = MOS_MIN(MOS_MAX(0, g), 255);
            output->B = MOS_MIN(MOS_MAX(0, b), 255);
            break;

        case CSpace_stRGB:
            output->A = (uint8_t)a;
            output->R = MOS_MIN(MOS_MAX(16, r), 235);
            output->G = MOS_MIN(MOS_MAX(16, g), 235);
            output->B = MOS_MIN(MOS_MAX(16, b), 235);
            break;

        case CSpace_BT601:
        case CSpace_BT709:
            output->a = (uint8_t)a;
            output->Y = MOS_MIN(MOS_MAX(16, r), 235);
            output->U = MOS_MIN(MOS_MAX(16, g), 240);
            output->V = MOS_MIN(MOS_MAX(16, b), 240);
            break;

        case CSpace_xvYCC601:
        case CSpace_xvYCC709:
        case CSpace_BT601_FullRange:
        case CSpace_BT709_FullRange:
            output->a = (uint8_t)a;
            output->Y = MOS_MIN(MOS_MAX(0, r), 255);
            output->U = MOS_MIN(MOS_MAX(0, g), 255);
            output->V = MOS_MIN(MOS_MAX(0, b), 255);
            break;

        default:
            VP_PUBLIC_NORMALMESSAGE("Unsupported Output ColorSpace %d.", (uint32_t)dstCspace);
            bResult = false;
            break;
        }
    }

    return bResult;
}

void VpUtils::GetCscMatrixForVeSfc8Bit(
    VPHAL_CSPACE srcCspace,      
    VPHAL_CSPACE dstCspace,      
    float        *fCscCoeff,     
    float        *fCscInOffset,  
    float        *fCscOutOffset)       
{
    float   fCscMatrix[12] = {0};
    int32_t i              =  0;

    KernelDll_GetCSCMatrix(
        srcCspace,
        dstCspace,
        fCscMatrix);

    // Copy [3x3] into Coeff
    for (i = 0; i < 3; i++)
    {
        MOS_SecureMemcpy(
            &fCscCoeff[i * 3],
            sizeof(float) * 3,
            &fCscMatrix[i * 4],
            sizeof(float) * 3);
    }

    // Get the input offsets
    switch (srcCspace)
    {
    CASE_YUV_CSPACE_LIMITEDRANGE:
        fCscInOffset[0] = -16.0F;
        fCscInOffset[1] = -128.0F;
        fCscInOffset[2] = -128.0F;
        break;

    CASE_YUV_CSPACE_FULLRANGE:
        fCscInOffset[0] = 0.0F;
        fCscInOffset[1] = -128.0F;
        fCscInOffset[2] = -128.0F;
        break;

    case CSpace_sRGB:
        fCscInOffset[0] = 0.0F;
        fCscInOffset[1] = 0.0F;
        fCscInOffset[2] = 0.0F;
        break;

    case CSpace_stRGB:
        fCscInOffset[0] = -16.0F;
        fCscInOffset[1] = -16.0F;
        fCscInOffset[2] = -16.0F;
        break;

    //BT2020 YUV->RGB
    case CSpace_BT2020:
        fCscInOffset[0] = -16.0F;
        fCscInOffset[1] = -128.0F;
        fCscInOffset[2] = -128.0F;
        break;

    case CSpace_BT2020_FullRange:
        fCscInOffset[0] = 0.0F;
        fCscInOffset[1] = -128.0F;
        fCscInOffset[2] = -128.0F;
        break;

    //BT2020 RGB->YUV
    case CSpace_BT2020_RGB:
        fCscInOffset[0] = 0.0F;
        fCscInOffset[1] = 0.0F;
        fCscInOffset[2] = 0.0F;
        break;

    //BT2020 RGB->YUV
    case CSpace_BT2020_stRGB:
        fCscInOffset[0] = -16.0F;
        fCscInOffset[1] = -16.0F;
        fCscInOffset[2] = -16.0F;
        break;

    default:
        VP_PUBLIC_NORMALMESSAGE("Unsupported Input ColorSpace for Vebox %d.", (uint32_t)srcCspace);
    }

    // Get the output offsets
    switch (dstCspace)
    {
    CASE_YUV_CSPACE_LIMITEDRANGE:
        fCscOutOffset[0] = 16.0F;
        fCscOutOffset[1] = 128.0F;
        fCscOutOffset[2] = 128.0F;
        break;

    CASE_YUV_CSPACE_FULLRANGE:
        fCscOutOffset[0] = 0.0F;
        fCscOutOffset[1] = 128.0F;
        fCscOutOffset[2] = 128.0F;
        break;

    case CSpace_sRGB:
        fCscOutOffset[0] = 0.0F;
        fCscOutOffset[1] = 0.0F;
        fCscOutOffset[2] = 0.0F;
        break;

    case CSpace_stRGB:
        fCscOutOffset[0] = 16.0F;
        fCscOutOffset[1] = 16.0F;
        fCscOutOffset[2] = 16.0F;
        break;

    //BT2020 RGB->YUV
    case CSpace_BT2020:
        fCscOutOffset[0] = 16.0F;
        fCscOutOffset[1] = 128.0F;
        fCscOutOffset[2] = 128.0F;
        break;

    case CSpace_BT2020_FullRange:
        fCscOutOffset[0] = 0.0F;
        fCscOutOffset[1] = 128.0F;
        fCscOutOffset[2] = 128.0F;
        break;

    case CSpace_BT2020_RGB:
        fCscOutOffset[0] = 0.0F;
        fCscOutOffset[1] = 0.0F;
        fCscOutOffset[2] = 0.0F;
        break;

    case CSpace_BT2020_stRGB:
        fCscOutOffset[0] = 16.0F;
        fCscOutOffset[1] = 16.0F;
        fCscOutOffset[2] = 16.0F;
        break;

    default:
        VP_PUBLIC_NORMALMESSAGE("Unsupported Output ColorSpace for Vebox %d.", (uint32_t)dstCspace);
    }
}

uint32_t VpUtils::GetSurfaceBitDepth(
    MOS_FORMAT format)
{
    uint32_t bitDepth = 0;

    VP_FUNC_CALL();

    static const std::map<const uint32_t, const MosFormatArray> bitDepthMap =
    {
        {8, {Format_IMC1,
             Format_IMC2,
             Format_IMC3,
             Format_IMC4,
             Format_NV12,
             Format_NV21,
             Format_YV12,
             Format_I420,
             Format_IYUV,
             Format_YUY2,
             Format_YUYV,
             Format_YVYU,
             Format_UYVY,
             Format_VYUY,
             Format_P208,
             Format_422H,
             Format_422V,
             Format_R5G6B5,
             Format_R8G8B8,
             Format_A8R8G8B8,
             Format_X8R8G8B8,
             Format_A8B8G8R8,
             Format_X8B8G8R8,
             Format_444P,
             Format_AYUV,
             Format_AUYV,
             Format_RGBP,
             Format_BGRP}},
        {10, {Format_P010,
              Format_R10G10B10A2,
              Format_B10G10R10A2,
              Format_Y210,
              Format_Y410,
              Format_P210}},
        {16, {Format_A16B16G16R16,
              Format_A16R16G16B16,
              Format_A16B16G16R16F,
              Format_A16R16G16B16F,
              Format_P016,
              Format_Y416,
              Format_Y216,
              Format_P216}},
    };
    for (auto mapIt = bitDepthMap.begin(); mapIt != bitDepthMap.end(); mapIt++)
    {
        auto &iFormatArray = mapIt->second;
        auto  iter         = std::find(iFormatArray.begin(), iFormatArray.end(), format);
        if (iter == iFormatArray.end())
        {
            continue;
        }
        return mapIt->first;
    }

    VP_PUBLIC_ASSERTMESSAGE("Unknown Input format for bit depth.");
    return 0;
}

MOS_STATUS VpUtils::DeclareUserSettings(MediaUserSettingSharedPtr userSettingPtr)
{
    DeclareUserSettingKey(  //For debugging purpose. true for disabling SFC
        userSettingPtr,
        __VPHAL_VEBOX_DISABLE_SFC,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKey(  //Disabling SFC DTR output. 1: Disable, 0: Enable
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_DTR_DISABLE,
        MediaUserSetting::Group::Sequence,
        1,
        true);

    DeclareUserSettingKey(  // For Notify which datapath Vebox used
        userSettingPtr,
        __VPHAL_VEBOX_OUTPUTPIPE_MODE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKey(//For Notify which feature Vebox used
        userSettingPtr,
        __VPHAL_VEBOX_FEATURE_INUSE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKey( //Disabling SFC Centering output. 1 -- Disable, 0 -- Enable.
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_CENTERING_DISABLE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKey( // VP Bypass Composition Mode
        userSettingPtr,
        __VPHAL_BYPASS_COMPOSITION,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKey(  //Enable memory compression
        userSettingPtr,
        __VPHAL_ENABLE_MMC,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_VEBOX_TGNE_ENABLE_VP,
        MediaUserSetting::Group::Sequence,
        1,
        true);  // Enable Vebox GNE. 1: Enable, 0: Disable

    DeclareUserSettingKey(//Slice Shutdown Control
        userSettingPtr,
        __VPHAL_RNDR_SSD_CONTROL,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKey(  // FALSE if CSC coefficient setting mode is Patch mode, otherwise Curbe mode
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_CSC_COEFF_PATCH_MODE_DISABLE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKey(  
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_DN,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_PACKET_REUSE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_PACKET_REUSE_TEAMS_ALWAYS,
        MediaUserSetting::Group::Sequence,
        0,
        true);

#if (_DEBUG || _RELEASE_INTERNAL)
    DeclareUserSettingKeyForDebug(  // FORCE VP DECOMPRESSED OUTPUT
        userSettingPtr,
        __VPHAL_RNDR_FORCE_VP_DECOMPRESSED_OUTPUT,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  //Software Scoreboard enable Control
        userSettingPtr,
        __VPHAL_RNDR_SCOREBOARD_CONTROL,
        MediaUserSetting::Group::Sequence,
        1,
        true);

    DeclareUserSettingKeyForDebug(  // CM based FC enable Control
        userSettingPtr,
        __VPHAL_RNDR_CMFC_CONTROL,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  //Enable 1K 1DLUT
        userSettingPtr,
        __VPHAL_ENABLE_1K_1DLUT,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __VPHAL_VEBOX_HDR_MODE,
        MediaUserSetting::Group::Sequence,
        0,
        true); //"HDR Mode. 0x1: H2S kernel, 0x3: H2H kernel, 0x21 65size H2S, 0x23 65size H2H, 0x31 33size H2S, 0x33 33size H2H."

    DeclareUserSettingKeyForDebug(  // VP Compression Enable
        userSettingPtr,
        __VPHAL_MMC_ENABLE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // VP Render Target Compressible
        userSettingPtr,
        __VPHAL_RT_MMC_COMPRESSIBLE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // VP Render Target Compression Mode
        userSettingPtr,
        __VPHAL_RT_MMC_COMPRESSMODE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // VP Primary Input Compressible
        userSettingPtr,
        __VPHAL_PRIMARY_MMC_COMPRESSIBLE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // VP Primary Input Compression Mode
        userSettingPtr,
        __VPHAL_PRIMARY_MMC_COMPRESSMODE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // VP Enable Compute Context
        userSettingPtr,
        __VPHAL_ENABLE_COMPUTE_CONTEXT,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // Force VP Memorycopy Outputcompressed
        userSettingPtr,
        __VPHAL_VEBOX_FORCE_VP_MEMCOPY_OUTPUTCOMPRESSED,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // VP Composition 8Tap Adaptive Enable
        userSettingPtr,
        __VPHAL_COMP_8TAP_ADAPTIVE_ENABLE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // Set SFC NV12/P010 Linear Output
        userSettingPtr,
        __VPHAL_ENABLE_SFC_NV12_P010_LINEAR_OUTPUT,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // Set SFC RGBP Linear/Tile RGB24 Linear Output
        userSettingPtr,
        __VPHAL_ENABLE_SFC_RGBP_RGB24_OUTPUT,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // VP Parameters Dump Outfile
        userSettingPtr,
        __VPHAL_DBG_PARAM_DUMP_OUTFILE_KEY_NAME,
        MediaUserSetting::Group::Sequence,
        "",
        true);

    DeclareUserSettingKeyForDebug(  // VP Parameters Dump Start Frame
        userSettingPtr,
        __VPHAL_DBG_PARAM_DUMP_START_FRAME_KEY_NAME,
        MediaUserSetting::Group::Sequence,
        1,
        true);

    DeclareUserSettingKeyForDebug(  // VP Parameters Dump End Frame
        userSettingPtr,
        __VPHAL_DBG_PARAM_DUMP_END_FRAME_KEY_NAME,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // Vphal Debug Dump Output Directory
        userSettingPtr,
        __VPHAL_DBG_DUMP_OUTPUT_DIRECTORY,
        MediaUserSetting::Group::Sequence,
        "",
        true);

    DeclareUserSettingKeyForDebug(  // VP parameter dump sku and wa info enable
        userSettingPtr,
        __VPHAL_DBG_PARA_DUMP_ENABLE_SKUWA_DUMP,
        MediaUserSetting::Group::Sequence,
        0,
        true);

#endif

    return MOS_STATUS_SUCCESS;
}
