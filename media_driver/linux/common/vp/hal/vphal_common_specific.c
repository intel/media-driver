/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     vphal_common_specific.c
//! \brief    Definition common utilities for vphal
//! \details  Definition common utilities for vphal including:
//!           some macro, enum, union, structure, function
//!
#include "vphal.h"
#include "mos_os.h"

//!
//! \brief    Get Surface Info from OsResource
//! \details  Update surface info in PVPHAL_SURFACE based on allocated OsResource
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \param    [in] pInfo
//!           Pointer to VPHAL_GET_SURFACE_INFO
//! \param    [in,out] pSurface
//!           Pointer to VPHAL_SURFACE
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_GetSurfaceInfo(
    PMOS_INTERFACE          pOsInterface,
    PVPHAL_GET_SURFACE_INFO pInfo,
    PVPHAL_SURFACE          pSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;

    VPHAL_PUBLIC_ASSERT(pOsInterface);
    VPHAL_PUBLIC_ASSERT(pInfo);
    VPHAL_PUBLIC_ASSERT(pSurface);

    PMOS_RESOURCE     pResource = &pSurface->OsResource;
    MOS_SURFACE       ResDetails;
    MOS_MEMCOMP_STATE mmcMode;

    VPHAL_PUBLIC_ASSERT(!Mos_ResourceIsNull(&pSurface->OsResource));
    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));
    ResDetails.dwArraySlice = pInfo->ArraySlice;
    ResDetails.dwMipSlice   = pInfo->MipSlice;
    ResDetails.S3dChannel   = pInfo->S3dChannel;
    ResDetails.Format       = pSurface->Format;
    VPHAL_PUBLIC_CHK_STATUS(pOsInterface->pfnGetResourceInfo(pOsInterface, &pSurface->OsResource, &ResDetails));

    if (ResDetails.Format == Format_420O)
    {
        ResDetails.Format = Format_NV12;
    }

    // Get resource information
    pSurface->dwWidth         = ResDetails.dwWidth;
    pSurface->dwHeight        = ResDetails.dwHeight;
    pSurface->dwPitch         = ResDetails.dwPitch;
    pSurface->dwSlicePitch    = ResDetails.dwSlicePitch;
    pSurface->dwDepth         = ResDetails.dwDepth;
    pSurface->TileType        = ResDetails.TileType;
    pSurface->TileModeGMM     = ResDetails.TileModeGMM;
    pSurface->bGMMTileEnabled = ResDetails.bGMMTileEnabled;
    pSurface->bOverlay        = ResDetails.bOverlay;
    pSurface->bFlipChain      = ResDetails.bFlipChain;
    pSurface->Format          = ResDetails.Format;
    pSurface->bCompressible   = ResDetails.bCompressible;
    pSurface->bIsCompressed   = ResDetails.bIsCompressed;

    MOS_ZeroMemory(&mmcMode, sizeof(mmcMode));
    pOsInterface->pfnGetMemoryCompressionMode(pOsInterface, &pSurface->OsResource, &mmcMode);

    if (mmcMode                           &&
        (pSurface->TileType == MOS_TILE_Y ||
         pSurface->TileType == MOS_TILE_YS))
    {
        pSurface->bCompressible   = true;
        pSurface->bIsCompressed   = true;
        pSurface->CompressionMode = (MOS_RESOURCE_MMC_MODE)mmcMode;

        pOsInterface->pfnGetMemoryCompressionFormat(pOsInterface, &pSurface->OsResource, &pSurface->CompressionFormat);
    }
    else
    {
        // Do not modify the bCompressible flag even MmcMode is disable, since the surface size/pitch may be different
        // between Compressible and Uncompressible, which will affect the DN surface allocation.
        pSurface->bIsCompressed     = false;
        pSurface->CompressionMode   = MOS_MMC_DISABLED;
        pSurface->CompressionFormat = 0;
    }

    if (IS_RGB32_FORMAT(pSurface->Format) ||
        IS_RGB16_FORMAT(pSurface->Format) ||
        pSurface->Format == Format_RGB    ||
        pSurface->Format == Format_Y410)
    {
        pSurface->dwOffset                    = ResDetails.RenderOffset.RGB.BaseOffset;
        pSurface->YPlaneOffset.iSurfaceOffset = ResDetails.RenderOffset.RGB.BaseOffset;
        pSurface->YPlaneOffset.iXOffset       = ResDetails.RenderOffset.RGB.XOffset;
        pSurface->YPlaneOffset.iYOffset       = ResDetails.RenderOffset.RGB.YOffset;
    }
    else // YUV or PL3_RGB
    {
        // Get Y plane information (plane offset, X/Y offset)
        pSurface->dwOffset                        = ResDetails.RenderOffset.YUV.Y.BaseOffset;
        pSurface->YPlaneOffset.iSurfaceOffset     = ResDetails.RenderOffset.YUV.Y.BaseOffset;
        pSurface->YPlaneOffset.iXOffset           = ResDetails.RenderOffset.YUV.Y.XOffset;
        pSurface->YPlaneOffset.iYOffset           = ResDetails.RenderOffset.YUV.Y.YOffset;
        pSurface->YPlaneOffset.iLockSurfaceOffset = ResDetails.LockOffset.YUV.Y;

        // Get U/UV plane information (plane offset, X/Y offset)
        pSurface->UPlaneOffset.iSurfaceOffset     = ResDetails.RenderOffset.YUV.U.BaseOffset;
        pSurface->UPlaneOffset.iXOffset           = ResDetails.RenderOffset.YUV.U.XOffset;
        pSurface->UPlaneOffset.iYOffset           = ResDetails.RenderOffset.YUV.U.YOffset;
        pSurface->UPlaneOffset.iLockSurfaceOffset = ResDetails.LockOffset.YUV.U;

        // Get V plane information (plane offset, X/Y offset)
        pSurface->VPlaneOffset.iSurfaceOffset     = ResDetails.RenderOffset.YUV.V.BaseOffset;
        pSurface->VPlaneOffset.iXOffset           = ResDetails.RenderOffset.YUV.V.XOffset;
        pSurface->VPlaneOffset.iYOffset           = ResDetails.RenderOffset.YUV.V.YOffset;
        pSurface->VPlaneOffset.iLockSurfaceOffset = ResDetails.LockOffset.YUV.V;
    }

    eStatus = MOS_STATUS_SUCCESS;
    goto finish;

finish:
    return eStatus;
}

//!
//! \brief    Initial the Type/TileType fields in Alloc Params structure
//! \details  Initial the Type/TileType fields in Alloc Params structure
//!           - Use the last type from GMM resource
//! \param    [in, out] pAllocParams
//!           Pointer to MOS_ALLOC_GFXRES_PARAMS
//! \param    [in] pSurface
//!           Pointer to VPHAL_SURFACE
//! \param    [in] DefaultResType
//!           Expected Resource Type
//! \param    [in] DefaultTileType
//!           Expected Surface Tile Type
//!
void VpHal_AllocParamsInitType(
    PMOS_ALLOC_GFXRES_PARAMS    pAllocParams,
    PVPHAL_SURFACE              pSurface,
    MOS_GFXRES_TYPE             DefaultResType,
    MOS_TILE_TYPE               DefaultTileType)
{
    VPHAL_PUBLIC_ASSERT(pAllocParams);
    VPHAL_PUBLIC_ASSERT(pSurface);

    // First time allocation. Caller must specify default params
    pAllocParams->Type     = DefaultResType;
    pAllocParams->TileType = DefaultTileType;
}
