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

#include "vp_utils.h"
#include "vp_common.h"
#include "hal_kerneldll_next.h"
#include "mos_interface.h"

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
