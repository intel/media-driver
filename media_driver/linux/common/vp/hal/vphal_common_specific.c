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

    PMOS_RESOURCE    pResource = &pSurface->OsResource;
    MOS_SURFACE      ResDetails;

    VPHAL_PUBLIC_ASSERT(!Mos_ResourceIsNull(&pSurface->OsResource));
    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));
    ResDetails.dwArraySlice = pInfo->ArraySlice;
    ResDetails.dwMipSlice   = pInfo->MipSlice;
    ResDetails.S3dChannel   = pInfo->S3dChannel;
    ResDetails.Format       = pSurface->Format;
    VPHAL_PUBLIC_CHK_STATUS(pOsInterface->pfnGetResourceInfo(pOsInterface, &pSurface->OsResource, &ResDetails));

    // Get resource information
    pSurface->dwWidth         = ResDetails.dwWidth;
    pSurface->dwHeight        = ResDetails.dwHeight;
    pSurface->dwPitch         = ResDetails.dwPitch;
    pSurface->TileType        = ResDetails.TileType;
    pSurface->dwDepth         = ResDetails.dwDepth;
    pSurface->bCompressible   = ResDetails.bCompressible;
    pSurface->bIsCompressed   = ResDetails.bIsCompressed;
    pSurface->CompressionMode = ResDetails.CompressionMode;

    // Get planes
    pSurface->UPlaneOffset.iSurfaceOffset = 0;
    pSurface->UPlaneOffset.iYOffset       = 0;
    pSurface->UPlaneOffset.iXOffset       = 0;
    pSurface->VPlaneOffset.iSurfaceOffset = 0;
    pSurface->VPlaneOffset.iXOffset       = 0;
    pSurface->VPlaneOffset.iYOffset       = 0;

    switch (pResource->Format)
    {
        case Format_NV12:
        case Format_P010:
        case Format_P016:
            pSurface->UPlaneOffset.iSurfaceOffset = (pSurface->dwHeight - pSurface->dwHeight % 32) * pSurface->dwPitch;
            pSurface->UPlaneOffset.iYOffset       = pSurface->dwHeight % 32;
            break;
        case Format_NV21:
            pSurface->UPlaneOffset.iSurfaceOffset = pSurface->dwHeight * pSurface->dwPitch;
            break;
        case Format_YV12:
            pSurface->VPlaneOffset.iSurfaceOffset = pSurface->dwHeight * pSurface->dwPitch;
            pSurface->VPlaneOffset.iYOffset       = 0;
            pSurface->UPlaneOffset.iSurfaceOffset = pSurface->dwHeight * pSurface->dwPitch * 5 / 4;
            pSurface->UPlaneOffset.iYOffset       = 0;
            break;
        case Format_I420:
            pSurface->VPlaneOffset.iSurfaceOffset = pSurface->dwHeight * pSurface->dwPitch * 5 / 4;
            pSurface->VPlaneOffset.iYOffset       = 0;
            pSurface->UPlaneOffset.iSurfaceOffset = pSurface->dwHeight * pSurface->dwPitch;
            pSurface->UPlaneOffset.iYOffset       = 0;
            break;
        case Format_422H:
            // Calculation methods are derived from Gmm's result.
            pSurface->UPlaneOffset.iSurfaceOffset = (pSurface->dwHeight - pSurface->dwHeight % 32) * pSurface->dwPitch;
            pSurface->UPlaneOffset.iYOffset       = pSurface->dwHeight % 32;
            pSurface->VPlaneOffset.iSurfaceOffset = (pSurface->dwHeight * 2 - (pSurface->dwHeight * 2) % 32) * pSurface->dwPitch;
            pSurface->VPlaneOffset.iYOffset       = (pSurface->dwHeight * 2) % 32;
            break;
        case Format_IMC3:
        case Format_422V:
            // Calculation methods are derived from Gmm's result.
            pSurface->UPlaneOffset.iSurfaceOffset = (pSurface->dwHeight - pSurface->dwHeight % 32) * pSurface->dwPitch;
            pSurface->UPlaneOffset.iYOffset       = pSurface->dwHeight % 32;
            pSurface->VPlaneOffset.iSurfaceOffset = (pSurface->dwHeight * 3 / 2 - (pSurface->dwHeight * 3 / 2) % 32) * pSurface->dwPitch;
            pSurface->VPlaneOffset.iYOffset       = (pSurface->dwHeight * 3 / 2) % 32;
            break;
        case Format_IMC4:
            pSurface->UPlaneOffset.iSurfaceOffset = pSurface->dwHeight * pSurface->dwPitch;
            pSurface->UPlaneOffset.iYOffset       = pSurface->dwHeight;
            pSurface->VPlaneOffset.iYOffset       = pSurface->dwHeight * 3 / 2;
            break;
        case Format_411P:
            pSurface->UPlaneOffset.iSurfaceOffset = pSurface->dwHeight * pSurface->dwPitch;
            pSurface->UPlaneOffset.iYOffset       = 0;
            pSurface->VPlaneOffset.iSurfaceOffset = pSurface->dwHeight * pSurface->dwPitch * 2;
            pSurface->VPlaneOffset.iYOffset       = 0;
            break;
        case Format_444P:
            pSurface->UPlaneOffset.iSurfaceOffset = pSurface->dwHeight * pSurface->dwPitch;
            pSurface->UPlaneOffset.iYOffset       = 0;
            pSurface->VPlaneOffset.iSurfaceOffset = pSurface->dwHeight * pSurface->dwPitch * 2;
            pSurface->VPlaneOffset.iYOffset       = 0;
            break;

        default:
            break;
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
