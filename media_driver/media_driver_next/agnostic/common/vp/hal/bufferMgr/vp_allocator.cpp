/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     vp_allocator.cpp
//! \brief    Defines the interface for vp resouce allocate
//! \details  vp allocator will allocate and destory buffers, the caller
//!           can use directly
//!

#include "vp_allocator.h"
#include "vp_utils.h"

namespace vp {

VpAllocator::VpAllocator(PMOS_INTERFACE osInterface, VPMediaMemComp *mmc) :
    m_osInterface(osInterface),
    m_mmc(mmc)
{
    m_allocator = MOS_New(Allocator, m_osInterface);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(m_allocator);
}

VpAllocator::~VpAllocator()
{
    if (m_allocator)
    {
        m_allocator->DestroyAllResources();
        MOS_Delete(m_allocator);
    }
}

//Paried with DestroyResource or DestroyAllResources
MOS_RESOURCE* VpAllocator::AllocateResource(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate)
{
    if (!m_allocator)
        return nullptr;

    return m_allocator->AllocateResource(param, zeroOnAllocate, COMPONENT_VPCommon);
}

MOS_STATUS VpAllocator::DestroyResource(MOS_RESOURCE *resource)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->DestroyResource(resource);
}

MOS_STATUS VpAllocator::DestroyAllResources()
{
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->DestroyAllResources();
}

//Paried with FreeResource
MOS_STATUS VpAllocator::AllocateResource(MOS_RESOURCE *res, MOS_ALLOC_GFXRES_PARAMS &param)
{
    if (!m_allocator)
        return MOS_STATUS_NULL_POINTER;

    return m_allocator->AllocateResource(res, param);
}

MOS_STATUS VpAllocator::FreeResource(MOS_RESOURCE *resource)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->FreeResource(resource);
}

//Paried with AllocateSurface
MOS_SURFACE* VpAllocator::AllocateSurface(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate)
{
    if (!m_allocator)
        return nullptr;

    return m_allocator->AllocateSurface(param, zeroOnAllocate, COMPONENT_VPCommon);
}

MOS_STATUS VpAllocator::DestroySurface(MOS_SURFACE *surface)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->DestroySurface(surface);
}

VP_SURFACE* VpAllocator::AllocateVpSurface(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate, VPHAL_CSPACE ColorSpace, uint32_t ChromaSiting)
{
    VP_SURFACE *surface = MOS_New(VP_SURFACE);
    if (nullptr == surface)
    {
        return nullptr;
    }
    MOS_ZeroMemory(surface, sizeof(VP_SURFACE));
    surface->osSurface = AllocateSurface(param, zeroOnAllocate);

    if (nullptr == surface->osSurface)
    {
        MOS_Delete(surface);
        return nullptr;
    }

    surface->isInternalSurface = true;
    surface->ColorSpace = ColorSpace;
    surface->ChromaSiting = ChromaSiting;
    surface->SampleType = SAMPLE_PROGRESSIVE; // Hardcode to SAMPLE_PROGRESSIVE for intermedia surface. Set to correct value for DI later.

    surface->rcSrc.left     = surface->rcSrc.top = 0;
    surface->rcSrc.right    = surface->osSurface->dwWidth;
    surface->rcSrc.bottom   = surface->osSurface->dwHeight;
    surface->rcDst          = surface->rcSrc;
    surface->rcMaxSrc       = surface->rcSrc;

    return surface;
}

// Allocate vp surface from vphalSurf. Reuse the resource in vphalSurf.
VP_SURFACE *VpAllocator::AllocateVpSurface(VPHAL_SURFACE &vphalSurf)
{
    VP_SURFACE *surf = MOS_New(VP_SURFACE);

    if (nullptr == surf || Mos_ResourceIsNull(&vphalSurf.OsResource))
    {
        return nullptr;
    }

    MOS_ZeroMemory(surf, sizeof(VP_SURFACE));

    surf->osSurface = MOS_New(MOS_SURFACE);

    if (nullptr == surf->osSurface)
    {
        MOS_Delete(surf);
        return nullptr;
    }

    // Initialize the mos surface in vp surface structure.
    MOS_SURFACE &osSurface = *surf->osSurface;
    MOS_ZeroMemory(&osSurface, sizeof(MOS_SURFACE));

    // Set input parameters dwArraySlice, dwMipSlice and S3dChannel if needed later.
    osSurface.Format            = vphalSurf.Format;
    osSurface.OsResource        = vphalSurf.OsResource;

    if (MOS_FAILED(m_allocator->GetSurfaceInfo(&osSurface.OsResource, &osSurface)))
    {
        MOS_Delete(surf->osSurface);
        MOS_Delete(surf);
        return nullptr;
    }

    // Align the MMC related flag with vphal surface.
    osSurface.bCompressible     = vphalSurf.bCompressible;
    osSurface.bIsCompressed     = vphalSurf.bIsCompressed;
    osSurface.CompressionMode   = vphalSurf.CompressionMode;
    osSurface.CompressionFormat = vphalSurf.CompressionFormat;
    osSurface.MmcState          = (MOS_MEMCOMP_STATE)vphalSurf.CompressionMode;

    // Initialize other parameters in vp surface according to vphal surface.
    surf->ColorSpace            = vphalSurf.ColorSpace;
    surf->ExtendedGamut         = vphalSurf.ExtendedGamut;
    surf->Palette               = vphalSurf.Palette;
    surf->bQueryVariance        = vphalSurf.bQueryVariance;
    surf->FrameID               = vphalSurf.FrameID;
    surf->uFwdRefCount          = vphalSurf.uFwdRefCount;
    surf->uBwdRefCount          = vphalSurf.uBwdRefCount;
    surf->pFwdRef               = vphalSurf.pFwdRef;
    surf->pBwdRef               = vphalSurf.pBwdRef;
    surf->SurfType              = vphalSurf.SurfType;
    surf->SampleType            = vphalSurf.SampleType;
    surf->ChromaSiting          = vphalSurf.ChromaSiting;
    surf->rcSrc                 = vphalSurf.rcSrc;
    surf->rcDst                 = vphalSurf.rcDst;
    surf->rcMaxSrc              = vphalSurf.rcMaxSrc;

    surf->isInternalSurface     = false;

    // Only be used in VpVeboxCmdPacket::PacketInit for current stage.
    // Should be removed after vphal surface being cleaned from VpVeboxCmdPacket.
    surf->pCurrent              = &vphalSurf;

    return surf;
}

MOS_STATUS VpAllocator::DestroyVpSurface(VP_SURFACE* &surface)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    VP_PUBLIC_CHK_NULL_RETURN(surface);

    if (surface->isInternalSurface)
    {
        status = DestroySurface(surface->osSurface);
    }
    else
    {
        MOS_Delete(surface->osSurface);
    }

    MOS_Delete(surface);
    return status;
}

void* VpAllocator::Lock(MOS_RESOURCE* resource, MOS_LOCK_PARAMS *lockFlag)
{
    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, lockFlag);
}

void* VpAllocator::LockResouceForWrite(MOS_RESOURCE *resource)
{
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

void* VpAllocator::LockResouceWithNoOverwrite(MOS_RESOURCE *resource)
{
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly   = 1;
    lockFlags.NoOverWrite = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

void* VpAllocator::LockResouceForRead(MOS_RESOURCE *resource)
{
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

MOS_STATUS VpAllocator::UnLock(MOS_RESOURCE *resource)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->UnLock(resource);
}

MOS_STATUS VpAllocator::SkipResourceSync(MOS_RESOURCE *resource)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->SkipResourceSync(resource);
}

MOS_STATUS VpAllocator::GetSurfaceInfo(VPHAL_SURFACE *surface, VPHAL_GET_SURFACE_INFO &info)
{
    MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
    MOS_SURFACE       resDetails;

    VP_PUBLIC_CHK_NULL_RETURN(m_mmc);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_NULL_RETURN(surface);

    VP_PUBLIC_ASSERT(!Mos_ResourceIsNull(&surface->OsResource));

    MOS_ZeroMemory(&resDetails, sizeof(MOS_SURFACE));
    resDetails.dwArraySlice = info.ArraySlice;
    resDetails.dwMipSlice   = info.MipSlice;
    resDetails.S3dChannel   = info.S3dChannel;
    resDetails.Format       = surface->Format;

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(&surface->OsResource, &resDetails));

    // Format_420O is mapped to Format_NV12 in VpHal here.
    // But it is mapped to several different Formats in CodecHal under different conditions.
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
    surface->bOverlay        = resDetails.bOverlay ? true : false;
    surface->bFlipChain      = resDetails.bFlipChain ? true : false;
    surface->Format          = resDetails.Format;
    surface->bCompressible   = resDetails.bCompressible ? true : false;
    surface->bIsCompressed   = resDetails.bIsCompressed ? true : false;

    if (IS_RGB32_FORMAT(surface->Format) ||
        IS_RGB16_FORMAT(surface->Format) ||
        IS_RGB64_FORMAT(surface->Format) ||
        surface->Format == Format_RGB    ||
        surface->Format == Format_Y410)
    {
        surface->dwOffset                    = resDetails.RenderOffset.RGB.BaseOffset;
        surface->YPlaneOffset.iSurfaceOffset = resDetails.RenderOffset.RGB.BaseOffset;
        surface->YPlaneOffset.iXOffset       = resDetails.RenderOffset.RGB.XOffset;
        surface->YPlaneOffset.iYOffset       = resDetails.RenderOffset.RGB.YOffset;
    }
    else // YUV or PL3_RGB
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

    VP_PUBLIC_CHK_STATUS_RETURN(m_mmc->GetResourceMmcState(&surface->OsResource, mmcMode));
    if (mmcMode &&
        (surface->TileType == MOS_TILE_Y ||
         surface->TileType == MOS_TILE_YS))
    {
        surface->bCompressible   = true;
        surface->CompressionMode = (MOS_RESOURCE_MMC_MODE)mmcMode;
    }
    else
    {
        surface->CompressionMode = MOS_MMC_DISABLED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::AllocParamsInitType(
    MOS_ALLOC_GFXRES_PARAMS     &allocParams,
    PVPHAL_SURFACE              surface,
    MOS_GFXRES_TYPE             defaultResType,
    MOS_TILE_TYPE               defaultTileType)
{
    VP_PUBLIC_CHK_NULL_RETURN(surface);

#if !EMUL && !LINUX
    //  Need to reallocate surface according to expected tiletype instead of tiletype of the surface what we have
    if ( surface                           != nullptr &&
         surface->OsResource.pGmmResInfo   != nullptr &&
         surface->TileType                 == defaultTileType)
    {
        // Reallocate but use same tile type and resource type as current
        allocParams.TileType = surface->OsResource.TileType;
        allocParams.Type     = surface->OsResource.ResType;
    }
    else
#endif
    {
        // First time allocation. Caller must specify default params
        allocParams.Type     = defaultResType;
        allocParams.TileType = defaultTileType;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::AllocParamsInitType(
        MOS_ALLOC_GFXRES_PARAMS     &allocParams,
        VP_SURFACE                  *surface,
        MOS_GFXRES_TYPE             defaultResType,
        MOS_TILE_TYPE               defaultTileType)
{
    VP_PUBLIC_CHK_NULL_RETURN(surface);
    VP_PUBLIC_CHK_NULL_RETURN(surface->osSurface);

    //  Need to reallocate surface according to expected tiletype instead of tiletype of the surface what we have
    if (surface != nullptr &&
        surface->osSurface != nullptr &&
        surface->osSurface->OsResource.pGmmResInfo != nullptr &&
        surface->osSurface->TileType == defaultTileType)
    {
        // Reallocate but use same tile type and resource type as current
        allocParams.TileType = surface->osSurface->TileType;
        allocParams.Type     = surface->osSurface->Type;
    }
    else
    {
        // First time allocation. Caller must specify default params
        allocParams.Type     = defaultResType;
        allocParams.TileType = defaultTileType;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::ReAllocateSurface(
    PVPHAL_SURFACE          surface,
    PCCHAR                  surfaceName,
    MOS_FORMAT              format,
    MOS_GFXRES_TYPE         defaultResType,
    MOS_TILE_TYPE           defaultTileType,
    uint32_t                width,
    uint32_t                height,
    bool                    compressible,
    MOS_RESOURCE_MMC_MODE   compressionMode,
    bool                    &allocated)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    VPHAL_GET_SURFACE_INFO  info;
    MOS_ALLOC_GFXRES_PARAMS allocParams;

    allocated = false;

    //---------------------------------
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_NULL_RETURN(surface);
    //---------------------------------

    // compressible should be compared with bCompressible since it is inited by bCompressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (!Mos_ResourceIsNull(&surface->OsResource) &&
        (surface->dwWidth           == width) &&
        (surface->dwHeight          == height) &&
        (surface->Format            == format) &&
        (surface->bCompressible     == compressible) &&
        (surface->CompressionMode   == compressionMode) &&
        (surface->TileType          == defaultTileType))
    {
        return eStatus;
    }

    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    AllocParamsInitType(allocParams, surface, defaultResType, defaultTileType);

    allocParams.dwWidth         = width;
    allocParams.dwHeight        = height;
    allocParams.Format          = format;
    allocParams.bIsCompressible = compressible;
    allocParams.CompressionMode = compressionMode;
    allocParams.pBufName        = surfaceName;
    allocParams.dwArraySize     = 1;

    // Delete resource if already allocated
    FreeResource(&surface->OsResource);

    // Allocate surface
    VP_PUBLIC_CHK_STATUS_RETURN(AllocateResource(&surface->OsResource, allocParams));

    // Get surface information
    MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));

    VP_PUBLIC_CHK_STATUS_RETURN(GetSurfaceInfo(surface, info));

    surface->Format = format;
    allocated = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::ReAllocateSurface(
        VP_SURFACE             *surface,
        PCCHAR                  surfaceName,
        MOS_FORMAT              format,
        MOS_GFXRES_TYPE         defaultResType,
        MOS_TILE_TYPE           defaultTileType,
        uint32_t                width,
        uint32_t                height,
        bool                    compressible,
        MOS_RESOURCE_MMC_MODE   compressionMode,
        bool                    &allocated,
        bool                    zeroOnAllocate)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    VPHAL_GET_SURFACE_INFO  info;
    MOS_ALLOC_GFXRES_PARAMS allocParams;

    allocated = false;

    //---------------------------------
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_NULL_RETURN(surface);
    //---------------------------------

    // compressible should be compared with bCompressible since it is inited by bCompressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (!surface->osSurface                                        &&
        !Mos_ResourceIsNull(&surface->osSurface->OsResource)       &&
        (surface->osSurface->dwWidth              == width)           &&
        (surface->osSurface->dwHeight             == height)          &&
        (surface->osSurface->Format               == format)          &&
        ((surface->osSurface->bCompressible != 0) == compressible)    &&
        (surface->osSurface->CompressionMode      == compressionMode) &&
        (surface->osSurface->TileType             == defaultTileType))
    {
        return eStatus;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(DestroyVpSurface(surface));
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    AllocParamsInitType(allocParams, surface, defaultResType, defaultTileType);

    allocParams.dwWidth         = width;
    allocParams.dwHeight        = height;
    allocParams.Format          = format;
    allocParams.bIsCompressible = compressible;
    allocParams.CompressionMode = compressionMode;
    allocParams.pBufName        = surfaceName;
    allocParams.dwArraySize     = 1;

    surface = AllocateVpSurface(allocParams, zeroOnAllocate);
    VP_PUBLIC_CHK_NULL_RETURN(surface);

    allocated = true;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::OsFillResource(
    PMOS_RESOURCE     osResource,
    uint32_t          size,
    uint8_t           value)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    return m_allocator->OsFillResource(osResource, size, value);
}

MOS_STATUS VpAllocator::ReadSurface (
    PVPHAL_SURFACE      surface,
    uint32_t            bpp,
    uint8_t             *dst)
{
    uint8_t         *src        = nullptr;
    uint8_t         *tempSrc    = nullptr;
    uint8_t         *tempDst    = nullptr;
    uint32_t        size        = 0;
    uint32_t        widthInBytes = 0;
    uint32_t        y           = 0;
    uint32_t        z           = 0;

    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    //----------------------------------------------
    VP_PUBLIC_ASSERT(surface);
    VP_PUBLIC_ASSERT(surface->dwWidth   > 0);
    VP_PUBLIC_ASSERT(surface->dwHeight  > 0);
    VP_PUBLIC_ASSERT(surface->dwDepth   > 0);
    VP_PUBLIC_ASSERT(surface->dwPitch >= surface->dwWidth);
    VP_PUBLIC_ASSERT(bpp > 0);
    VP_PUBLIC_ASSERT(dst);
    VP_PUBLIC_ASSERT(!Mos_ResourceIsNull(&surface->OsResource));
    //----------------------------------------------

    src = (uint8_t*)LockResouceForRead(&surface->OsResource);
    VP_PUBLIC_CHK_NULL_RETURN(src);

    // Calculate Size in Bytes
    size = surface->dwWidth * surface->dwHeight * surface->dwDepth * bpp/8;
    widthInBytes = surface->dwWidth * bpp / 8;
    if (surface->dwPitch == widthInBytes)
    {
        MOS_SecureMemcpy(dst, size, src, size);
    }
    else
    {
        tempSrc    = src;
        tempDst    = dst;

        for (z = 0; z < surface->dwDepth; z++)
        {
            for (y = 0; y < surface->dwHeight; y++)
            {
                MOS_SecureMemcpy(tempDst, widthInBytes, tempSrc, widthInBytes);
                tempSrc += surface->dwPitch;
                tempDst += widthInBytes;
            }
        }
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&surface->OsResource));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::WriteSurface (
    PVPHAL_SURFACE      surface,
    uint32_t            bpp,
    const uint8_t       *src)
{
    uint8_t             *dst        = nullptr;
    uint8_t             *tempSrc    = nullptr;
    uint8_t             *tempDst    = nullptr;
    uint32_t            widthInBytes = 0;
    uint32_t            size        = 0;
    uint32_t            y           = 0;
    uint32_t            z           = 0;

    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    //----------------------------------------------
    VP_PUBLIC_ASSERT(surface);
    VP_PUBLIC_ASSERT(surface->dwWidth   > 0);
    VP_PUBLIC_ASSERT(surface->dwHeight  > 0);
    VP_PUBLIC_ASSERT(surface->dwDepth   > 0);
    VP_PUBLIC_ASSERT(surface->dwPitch >= surface->dwWidth);
    VP_PUBLIC_ASSERT(bpp > 0);
    VP_PUBLIC_ASSERT(src);
    VP_PUBLIC_ASSERT(!Mos_ResourceIsNull(&surface->OsResource));
    //----------------------------------------------

    dst = (uint8_t*)LockResouceForWrite(&surface->OsResource);
    VP_PUBLIC_CHK_NULL_RETURN(dst);

    // Calculate Size in Bytes
    size = surface->dwWidth * surface->dwHeight * surface->dwDepth * bpp/8;
    widthInBytes = surface->dwWidth * bpp/8;

    if (surface->dwPitch == widthInBytes)
    {
        MOS_SecureMemcpy(dst, size, src, size);
    }
    else
    {
        tempSrc    = (uint8_t*)src;
        tempDst    = dst;

        for (z = 0; z < surface->dwDepth; z++)
        {
            for (y = 0; y < surface->dwHeight; y++)
            {
                MOS_SecureMemcpy(tempDst, widthInBytes, tempSrc, widthInBytes);
                tempSrc += widthInBytes;
                tempDst += surface->dwPitch;
            }
        }
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&surface->OsResource));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::WriteSurface(VP_SURFACE* vpsurface, uint32_t bpp, const uint8_t* src)
{
    uint8_t* dst = nullptr;
    uint8_t* tempSrc = nullptr;
    uint8_t* tempDst = nullptr;
    uint32_t            widthInBytes = 0;
    uint32_t            size = 0;
    uint32_t            y = 0;
    uint32_t            z = 0;
    MOS_SURFACE* surface;

    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    //----------------------------------------------
    VP_PUBLIC_ASSERT(vpsurface);

    surface = vpsurface->osSurface;
    VP_PUBLIC_ASSERT(surface);

    VP_PUBLIC_ASSERT(surface->dwWidth > 0);
    VP_PUBLIC_ASSERT(surface->dwHeight > 0);
    VP_PUBLIC_ASSERT(surface->dwDepth > 0);
    VP_PUBLIC_ASSERT(surface->dwPitch >= surface->dwWidth);
    VP_PUBLIC_ASSERT(bpp > 0);
    VP_PUBLIC_ASSERT(src);
    VP_PUBLIC_ASSERT(!Mos_ResourceIsNull(&surface->OsResource));
    //----------------------------------------------

    dst = (uint8_t*)LockResouceForWrite(&surface->OsResource);
    VP_PUBLIC_CHK_NULL_RETURN(dst);

    // Calculate Size in Bytes
    size = surface->dwWidth * surface->dwHeight * surface->dwDepth * bpp / 8;
    widthInBytes = surface->dwWidth * bpp / 8;

    if (surface->dwPitch == widthInBytes)
    {
        MOS_SecureMemcpy(dst, size, src, size);
    }
    else
    {
        tempSrc = (uint8_t*)src;
        tempDst = dst;

        for (z = 0; z < surface->dwDepth; z++)
        {
            for (y = 0; y < surface->dwHeight; y++)
            {
                MOS_SecureMemcpy(tempDst, widthInBytes, tempSrc, widthInBytes);
                tempSrc += widthInBytes;
                tempDst += surface->dwPitch;
            }
        }
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&surface->OsResource));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::SyncOnResource(
    PMOS_RESOURCE         osResource,
    bool                  bWriteOperation)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return (m_allocator->SyncOnResource(osResource, bWriteOperation));
}
}