/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vphal_common_specific_next.c
//! \brief    Definition common utilities for vphal
//! \details  Definition common utilities for vphal including:
//!           some macro, enum, union, structure, function

#include "vp_utils.h"
#include "mos_interface.h"
MOS_STATUS VpHal_GetSurfaceInfo(
    PMOS_INTERFACE          osInterface,
    PVPHAL_GET_SURFACE_INFO info,
    PVPHAL_SURFACE          surface)
{
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    VP_PUBLIC_ASSERT(osInterface);
    VP_PUBLIC_ASSERT(info);
    VP_PUBLIC_ASSERT(surface);

    PMOS_RESOURCE     resource   = &surface->OsResource;
    MOS_SURFACE       resDetails = {};
    MOS_MEMCOMP_STATE mmcMode    = MOS_MEMCOMP_DISABLED;

    VP_PUBLIC_ASSERT(!Mos_ResourceIsNull(&surface->OsResource));
    MOS_ZeroMemory(&resDetails, sizeof(MOS_SURFACE));
    resDetails.dwArraySlice = info->ArraySlice;
    resDetails.dwMipSlice   = info->MipSlice;
    resDetails.S3dChannel   = info->S3dChannel;
    resDetails.Format       = surface->Format;
    VP_PUBLIC_CHK_STATUS(osInterface->pfnGetResourceInfo(osInterface, &surface->OsResource, &resDetails));

    if (resDetails.Format == Format_420O)
    {
        resDetails.Format = Format_NV12;
    }

    // Get resource information
    surface->dwWidth         = resDetails.dwWidth;
    surface->dwHeight        = resDetails.dwHeight;
    surface->dwPitch         = resDetails.dwPitch;
    surface->dwSlicePitch    = resDetails.dwSlicePitch;
    surface->dwDepth         = resDetails.dwDepth;
    surface->TileType        = resDetails.TileType;
    surface->TileModeGMM     = resDetails.TileModeGMM;
    surface->bGMMTileEnabled = resDetails.bGMMTileEnabled;
    surface->bOverlay        = resDetails.bOverlay;
    surface->bFlipChain      = resDetails.bFlipChain;
    surface->Format          = resDetails.Format;
    surface->bCompressible   = resDetails.bCompressible;
    surface->bIsCompressed   = resDetails.bIsCompressed;

    MOS_ZeroMemory(&mmcMode, sizeof(mmcMode));
    osInterface->pfnGetMemoryCompressionMode(osInterface, &surface->OsResource, &mmcMode);

    if (mmcMode &&
        (surface->TileType == MOS_TILE_Y ||
            surface->TileType == MOS_TILE_YS))
    {
        surface->bCompressible   = true;
        surface->bIsCompressed   = true;
        surface->CompressionMode = (MOS_RESOURCE_MMC_MODE)mmcMode;

        osInterface->pfnGetMemoryCompressionFormat(osInterface, &surface->OsResource, &surface->CompressionFormat);
    }
    else
    {
        // Do not modify the bCompressible flag even MmcMode is disable, since the surface size/pitch may be different
        // between Compressible and Uncompressible, which will affect the DN surface allocation.
        surface->bIsCompressed     = false;
        surface->CompressionMode   = MOS_MMC_DISABLED;
        surface->CompressionFormat = 0;
    }

    if (IS_RGB32_FORMAT(surface->Format) ||
        IS_RGB16_FORMAT(surface->Format) ||
        surface->Format == Format_RGB ||
        surface->Format == Format_Y410)
    {
        surface->dwOffset                    = resDetails.RenderOffset.RGB.BaseOffset;
        surface->YPlaneOffset.iSurfaceOffset = resDetails.RenderOffset.RGB.BaseOffset;
        surface->YPlaneOffset.iXOffset       = resDetails.RenderOffset.RGB.XOffset;
        surface->YPlaneOffset.iYOffset       = resDetails.RenderOffset.RGB.YOffset;
    }
    else  // YUV or PL3_RGB
    {
        // Get Y plane information (plane offset, X/Y offset)
        surface->dwOffset                        = resDetails.RenderOffset.YUV.Y.BaseOffset;
        surface->YPlaneOffset.iSurfaceOffset     = resDetails.RenderOffset.YUV.Y.BaseOffset;
        surface->YPlaneOffset.iXOffset           = resDetails.RenderOffset.YUV.Y.XOffset;
        surface->YPlaneOffset.iYOffset           = resDetails.RenderOffset.YUV.Y.YOffset;
        surface->YPlaneOffset.iLockSurfaceOffset = resDetails.LockOffset.YUV.Y;

        // Get U/UV plane information (plane offset, X/Y offset)
        surface->UPlaneOffset.iSurfaceOffset     = resDetails.RenderOffset.YUV.U.BaseOffset;
        surface->UPlaneOffset.iXOffset           = resDetails.RenderOffset.YUV.U.XOffset;
        surface->UPlaneOffset.iYOffset           = resDetails.RenderOffset.YUV.U.YOffset;
        surface->UPlaneOffset.iLockSurfaceOffset = resDetails.LockOffset.YUV.U;

        // Get V plane information (plane offset, X/Y offset)
        surface->VPlaneOffset.iSurfaceOffset     = resDetails.RenderOffset.YUV.V.BaseOffset;
        surface->VPlaneOffset.iXOffset           = resDetails.RenderOffset.YUV.V.XOffset;
        surface->VPlaneOffset.iYOffset           = resDetails.RenderOffset.YUV.V.YOffset;
        surface->VPlaneOffset.iLockSurfaceOffset = resDetails.LockOffset.YUV.V;
    }

    eStatus = MOS_STATUS_SUCCESS;
    goto finish;

finish:
    return eStatus;
}

void VpHal_AllocParamsInitType(
    PMOS_ALLOC_GFXRES_PARAMS allocParams,
    PVPHAL_SURFACE           surface,
    MOS_GFXRES_TYPE          defaultResType,
    MOS_TILE_TYPE            defaultTileType)
{
    VP_PUBLIC_ASSERT(allocParams);
    VP_PUBLIC_ASSERT(surface);

    // First time allocation. Caller must specify default params
    allocParams->Type     = defaultResType;
    allocParams->TileType = defaultTileType;
}