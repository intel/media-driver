/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \brief    Defines the interface for vp resource allocate
//! \details  vp allocator will allocate and destory buffers, the caller
//!           can use directly
//!

#include "vp_allocator.h"
#include "vp_utils.h"
#include "mos_solo_generic.h"

using namespace vp;

VpAllocator::VpAllocator(PMOS_INTERFACE osInterface, MediaMemComp *mmc) :
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
    VP_FUNC_CALL();
    if (!m_allocator)
        return nullptr;

    return m_allocator->AllocateResource(param, zeroOnAllocate, COMPONENT_VPCommon);
}

MOS_STATUS VpAllocator::DestroyResource(MOS_RESOURCE *resource)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->DestroyResource(resource);
}

MOS_STATUS VpAllocator::DestroyAllResources()
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->DestroyAllResources();
}

//Paried with FreeResource
MOS_STATUS VpAllocator::AllocateResource(MOS_RESOURCE *res, MOS_ALLOC_GFXRES_PARAMS &param)
{
    VP_FUNC_CALL();
    if (!m_allocator)
        return MOS_STATUS_NULL_POINTER;

    return m_allocator->AllocateResource(res, param);
}

MOS_STATUS VpAllocator::FreeResource(MOS_RESOURCE *resource)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->FreeResource(resource);
}

void VpAllocator::UpdateSurfacePlaneOffset(MOS_SURFACE &surf)
{
    VP_FUNC_CALL();
    // dwOffset/YPlaneOffset/UPlaneOffset/VPlaneOffset will not be initialized during GetSurfaceInfo.
    // Initialize them with RenderOffset when needed.
    if (IS_RGB32_FORMAT(surf.Format) ||
        IS_RGB16_FORMAT(surf.Format) ||
        IS_RGB64_FORMAT(surf.Format) ||
        surf.Format == Format_RGB    ||
        surf.Format == Format_Y410)
    {
        surf.dwOffset                        = surf.RenderOffset.RGB.BaseOffset;
        surf.YPlaneOffset.iSurfaceOffset     = surf.RenderOffset.RGB.BaseOffset;
        surf.YPlaneOffset.iXOffset           = surf.RenderOffset.RGB.XOffset;
        surf.YPlaneOffset.iYOffset           = surf.RenderOffset.RGB.YOffset;
    }
    else // YUV or PL3_RGB
    {
        // Get Y plane information (plane offset, X/Y offset)
        surf.dwOffset                        = surf.RenderOffset.YUV.Y.BaseOffset;
        surf.YPlaneOffset.iSurfaceOffset     = surf.RenderOffset.YUV.Y.BaseOffset;
        surf.YPlaneOffset.iXOffset           = surf.RenderOffset.YUV.Y.XOffset;
        surf.YPlaneOffset.iYOffset           = surf.RenderOffset.YUV.Y.YOffset;
        surf.YPlaneOffset.iLockSurfaceOffset = surf.LockOffset.YUV.Y;

        // Get U/UV plane information (plane offset, X/Y offset)
        surf.UPlaneOffset.iSurfaceOffset     = surf.RenderOffset.YUV.U.BaseOffset;
        surf.UPlaneOffset.iXOffset           = surf.RenderOffset.YUV.U.XOffset;
        surf.UPlaneOffset.iYOffset           = surf.RenderOffset.YUV.U.YOffset;
        surf.UPlaneOffset.iLockSurfaceOffset = surf.LockOffset.YUV.U;

        // Get V plane information (plane offset, X/Y offset)
        surf.VPlaneOffset.iSurfaceOffset     = surf.RenderOffset.YUV.V.BaseOffset;
        surf.VPlaneOffset.iXOffset           = surf.RenderOffset.YUV.V.XOffset;
        surf.VPlaneOffset.iYOffset           = surf.RenderOffset.YUV.V.YOffset;
        surf.VPlaneOffset.iLockSurfaceOffset = surf.LockOffset.YUV.V;
    }
}

//Paried with AllocateSurface
MOS_SURFACE* VpAllocator::AllocateSurface(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate)
{
    VP_FUNC_CALL();
    if (!m_allocator)
        return nullptr;

    MOS_SURFACE* surf = m_allocator->AllocateSurface(param, zeroOnAllocate, COMPONENT_VPCommon);

    if (surf)
    {
        // Format is not initialized in Allocator::AllocateSurface. Remove it after
        // it being fixed in Allocator::AllocateSurface.
        surf->Format = param.Format;

        if (MOS_FAILED(SetMmcFlags(*surf)))
        {
            VP_PUBLIC_ASSERTMESSAGE("Set mmc flags failed during AllocateSurface!");
            m_allocator->DestroySurface(surf);
            return nullptr;
        }

        UpdateSurfacePlaneOffset(*surf);
    }

    return surf;
}

MOS_STATUS VpAllocator::DestroySurface(MOS_SURFACE *surface, MOS_GFXRES_FREE_FLAGS flags)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    MOS_GFXRES_FREE_FLAGS resFreeFlags = {0};
    if (IsSyncFreeNeededForMMCSurface(surface))
    {
        resFreeFlags.SynchronousDestroy = 1;
    }
    return m_allocator->DestroySurface(surface, resFreeFlags);
}

VP_SURFACE* VpAllocator::AllocateVpSurface(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate, VPHAL_CSPACE ColorSpace, uint32_t ChromaSiting)
{
    VP_FUNC_CALL();
    VP_SURFACE *surface = MOS_New(VP_SURFACE);
    if (nullptr == surface)
    {
        return nullptr;
    }
    MOS_ZeroMemory(surface, sizeof(VP_SURFACE));

    // Only used for Buffer surface
    uint32_t bufferWidth  = 0;
    uint32_t bufferHeight = 0;


    if (param.Format == Format_Buffer)
    {
        bufferWidth   = param.dwWidth;
        bufferHeight  = param.dwHeight;
        param.dwWidth = param.dwWidth * param.dwHeight;
        param.dwHeight = 1;
    }

    surface->osSurface = AllocateSurface(param, zeroOnAllocate);

    if (nullptr == surface->osSurface)
    {
        MOS_Delete(surface);
        MT_ERR1(MT_VP_HAL_ALLOC_SURF, MT_CODE_LINE, __LINE__);
        return nullptr;
    }

    surface->isResourceOwner = true;
    surface->ColorSpace = ColorSpace;
    surface->ChromaSiting = ChromaSiting;
    surface->SampleType = SAMPLE_PROGRESSIVE; // Hardcode to SAMPLE_PROGRESSIVE for intermedia surface. Set to correct value for DI later.

    surface->rcSrc.left     = surface->rcSrc.top = 0;
    surface->rcSrc.right    = surface->osSurface->dwWidth;
    surface->rcSrc.bottom   = surface->osSurface->dwHeight;
    surface->rcDst          = surface->rcSrc;
    surface->rcMaxSrc       = surface->rcSrc;


    if (param.Format == Format_Buffer)
    {
        surface->bufferWidth = bufferWidth;
        surface->bufferHeight = bufferHeight;
    }

    return surface;
}

// Allocate vp surface from vphalSurf. Reuse the resource in vphalSurf.
VP_SURFACE *VpAllocator::AllocateVpSurface(VPHAL_SURFACE &vphalSurf)
{
    VP_FUNC_CALL();
    if (Mos_ResourceIsNull(&vphalSurf.OsResource))
    {
        if (vphalSurf.pPipeIntermediateSurface)
        {
            VP_SURFACE *surf = MOS_New(VP_SURFACE);
            if (surf == nullptr)
            {
                return nullptr;
            }
            surf->osSurface = MOS_New(MOS_SURFACE);
            if (MOS_FAILED(CopyVpSurface(*surf, *vphalSurf.pPipeIntermediateSurface)))
            {
                MOS_Delete(surf->osSurface);
                MOS_Delete(surf);
                return nullptr;
            }
            return surf;
        }
        return nullptr;
    }
    VP_SURFACE *surf = MOS_New(VP_SURFACE);

    if (nullptr == surf)
    {
        return nullptr;
    }

    surf->osSurface = MOS_New(MOS_SURFACE);
    if (nullptr == surf->osSurface)
    {
        MOS_Delete(surf);
        return nullptr;
    }

    surf->isResourceOwner = false;
    surf->Clean();

    // Initialize the mos surface in vp surface structure.
    MOS_SURFACE &osSurface = *surf->osSurface;
    MOS_ZeroMemory(&osSurface, sizeof(MOS_SURFACE));

    // Set input parameters dwArraySlice, dwMipSlice and S3dChannel if needed later.
    osSurface.Format                            = vphalSurf.Format;
    osSurface.OsResource                        = vphalSurf.OsResource;

    if (MOS_FAILED(m_allocator->GetSurfaceInfo(&osSurface.OsResource, &osSurface)))
    {
        MOS_Delete(surf->osSurface);
        MOS_Delete(surf);
        return nullptr;
    }

    // Align the format with vphal surface. Some format need be remapped in vphal surface.
    // For example, format_420O is mapped to Format_NV12 in VpHal.
    // But it is mapped to several different Formats in CodecHal under different conditions.
    osSurface.Format                            = vphalSurf.Format;

    // Add offset info
    osSurface.dwOffset                    = vphalSurf.dwOffset;
    osSurface.YPlaneOffset.iSurfaceOffset = vphalSurf.YPlaneOffset.iSurfaceOffset;
    osSurface.YPlaneOffset.iXOffset       = vphalSurf.YPlaneOffset.iXOffset;
    osSurface.YPlaneOffset.iYOffset       = vphalSurf.YPlaneOffset.iYOffset;
    if (IS_RGB32_FORMAT(osSurface.Format) ||
        IS_RGB16_FORMAT(osSurface.Format) ||
        IS_RGB64_FORMAT(osSurface.Format) ||
        osSurface.Format == Format_RGB ||
        osSurface.Format == Format_Y410)
    {
        osSurface.dwOffset                    = vphalSurf.dwOffset;
        osSurface.YPlaneOffset.iSurfaceOffset = vphalSurf.YPlaneOffset.iSurfaceOffset;
        osSurface.YPlaneOffset.iXOffset       = vphalSurf.YPlaneOffset.iXOffset;
        osSurface.YPlaneOffset.iYOffset       = vphalSurf.YPlaneOffset.iYOffset;
    }
    else  // YUV or PL3_RGB
    {
        // Get Y plane information (plane offset, X/Y offset)
        osSurface.dwOffset                        = vphalSurf.dwOffset;
        osSurface.YPlaneOffset.iSurfaceOffset     = vphalSurf.YPlaneOffset.iSurfaceOffset;
        osSurface.YPlaneOffset.iXOffset           = vphalSurf.YPlaneOffset.iXOffset;
        osSurface.YPlaneOffset.iYOffset           = vphalSurf.YPlaneOffset.iYOffset;
        osSurface.YPlaneOffset.iLockSurfaceOffset = vphalSurf.YPlaneOffset.iLockSurfaceOffset;

        // Get U/UV plane information (plane offset, X/Y offset)
        osSurface.UPlaneOffset.iSurfaceOffset     = vphalSurf.UPlaneOffset.iSurfaceOffset;
        osSurface.UPlaneOffset.iXOffset           = vphalSurf.UPlaneOffset.iXOffset;
        osSurface.UPlaneOffset.iYOffset           = vphalSurf.UPlaneOffset.iYOffset;
        osSurface.UPlaneOffset.iLockSurfaceOffset = vphalSurf.UPlaneOffset.iLockSurfaceOffset;

        // Get V plane information (plane offset, X/Y offset)
        osSurface.VPlaneOffset.iSurfaceOffset     = vphalSurf.VPlaneOffset.iSurfaceOffset;
        osSurface.VPlaneOffset.iXOffset           = vphalSurf.VPlaneOffset.iXOffset;
        osSurface.VPlaneOffset.iYOffset           = vphalSurf.VPlaneOffset.iYOffset;
        osSurface.VPlaneOffset.iLockSurfaceOffset = vphalSurf.VPlaneOffset.iLockSurfaceOffset;
    }

    // Initialize other parameters in vp surface according to vphal surface.
    surf->ColorSpace                            = vphalSurf.ColorSpace;
    surf->ExtendedGamut                         = vphalSurf.ExtendedGamut;
    surf->Palette                               = vphalSurf.Palette;
    surf->bQueryVariance                        = vphalSurf.bQueryVariance;
    surf->FrameID                               = vphalSurf.FrameID;
    surf->uFwdRefCount                          = vphalSurf.uFwdRefCount;
    surf->uBwdRefCount                          = vphalSurf.uBwdRefCount;
    surf->pFwdRef                               = vphalSurf.pFwdRef;
    surf->pBwdRef                               = vphalSurf.pBwdRef;
    surf->SurfType                              = vphalSurf.SurfType;
    surf->SampleType                            = vphalSurf.SampleType;
    surf->ChromaSiting                          = vphalSurf.ChromaSiting;
    surf->rcSrc                                 = vphalSurf.rcSrc;
    surf->rcDst                                 = vphalSurf.rcDst;
    surf->rcMaxSrc                              = vphalSurf.rcMaxSrc;

    if (MOS_FAILED(SetMmcFlags(osSurface)))
    {
        VP_PUBLIC_ASSERTMESSAGE("Set mmc flags failed during AllocateVpSurface!");
        DestroyVpSurface(surf);
        return nullptr;
    }
    return surf;
}

// Allocate vp surface from vpSurfSrc. Reuse the resource in vpSurfSrc.
VP_SURFACE *VpAllocator::AllocateVpSurface(VP_SURFACE &vpSurfSrc)
{
    VP_FUNC_CALL();
    if (nullptr == vpSurfSrc.osSurface || Mos_ResourceIsNull(&vpSurfSrc.osSurface->OsResource))
    {
        return nullptr;
    }

    VP_SURFACE *surf = MOS_New(VP_SURFACE);

    if (nullptr == surf)
    {
        return nullptr;
    }

    MOS_SURFACE *osSurface = MOS_New(MOS_SURFACE);

    if (nullptr == osSurface)
    {
        MOS_Delete(surf);
        return nullptr;
    }

    *osSurface = *vpSurfSrc.osSurface;
    *surf = vpSurfSrc;

    surf->osSurface = osSurface;
    surf->isResourceOwner = false;

    return surf;
}

// Allocate vp surface from osSurf. Reuse the resource in osSurf.
VP_SURFACE *VpAllocator::AllocateVpSurface(MOS_SURFACE &osSurf,
    VPHAL_CSPACE colorSpace, uint32_t chromaSiting, RECT rcSrc, RECT rcDst, VPHAL_SURFACE_TYPE SurfType, bool updatePlaneOffset)
{
    VP_FUNC_CALL();
    if (Mos_ResourceIsNull(&osSurf.OsResource))
    {
        return nullptr;
    }

    VP_SURFACE *surf = MOS_New(VP_SURFACE);

    if (nullptr == surf)
    {
        return nullptr;
    }

    MOS_SURFACE *osSurface = MOS_New(MOS_SURFACE);

    if (nullptr == osSurface)
    {
        MOS_Delete(surf);
        return nullptr;
    }

    *osSurface = osSurf;
    if (updatePlaneOffset)
    {
        UpdateSurfacePlaneOffset(*osSurface);
    }

    MOS_ZeroMemory(surf, sizeof(VP_SURFACE));
    surf->osSurface         = osSurface;
    surf->isResourceOwner   = false;
    surf->ColorSpace        = colorSpace;               //!< Color Space
    surf->ChromaSiting      = chromaSiting;             //!< ChromaSiting
    surf->rcSrc             = rcSrc;                    //!< Source rectangle
    surf->rcDst             = rcDst;                    //!< Destination rectangle
    surf->rcMaxSrc          = rcSrc;                    //!< Max source rectangle
    surf->SurfType          = SurfType;                 //!< Surface type (context). Not in use for internal surface
    surf->SampleType        = SAMPLE_PROGRESSIVE;       //!<  Interlaced/Progressive sample type.

    return surf;
}

// Allocate empty vp surface.
VP_SURFACE *VpAllocator::AllocateVpSurface()
{
    VP_FUNC_CALL();
    // Allocate VpSurface without resource.
    VP_SURFACE *surf = MOS_New(VP_SURFACE);

    if (nullptr == surf)
    {
        return nullptr;
    }

    MOS_SURFACE *osSurface = MOS_New(MOS_SURFACE);

    if (nullptr == osSurface)
    {
        MOS_Delete(surf);
        return nullptr;
    }

    surf->osSurface = osSurface;
    surf->isResourceOwner = false;
    surf->Clean();

    return surf;
}

// Copy surface info from src to dst. dst shares the resource of src.
MOS_STATUS VpAllocator::CopyVpSurface(VP_SURFACE &dst, VP_SURFACE &src)
{
    VP_FUNC_CALL();
    if (nullptr == dst.osSurface || nullptr == src.osSurface || dst.isResourceOwner)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_SURFACE &osSurface = *dst.osSurface;
    osSurface = *src.osSurface;
    dst = src;

    dst.osSurface = &osSurface;
    dst.isResourceOwner = false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::DestroyVpSurface(VP_SURFACE* &surface, bool deferredDestroyed, MOS_GFXRES_FREE_FLAGS flags)
{
    VP_FUNC_CALL();
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    if (nullptr == surface)
    {
        return status;
    }

    if (surface && nullptr == surface->osSurface)
    {
        // VP_SURFACE should always be allocated by interface in VpAllocator,
        // which will ensure nullptr != surface->osSurface.
        VP_PUBLIC_NORMALMESSAGE("Surfaces already been deleted, return status!");
        return status;
    }

    if (deferredDestroyed)
    {
        m_recycler.push_back(surface);
        surface = nullptr;
        return MOS_STATUS_SUCCESS;
    }

    if (surface->isResourceOwner)
    {
#if !EMUL
    MT_LOG5(MT_VP_HAL_DESTROY_SURF, MT_NORMAL,
        MT_VP_HAL_SURF_ALLOC_PARAM_PTR, *(int64_t *)(&surface),
        MT_VP_HAL_SURF_ALLOC_PARAM_MOS_SURF_PTR, *(int64_t *)(&surface->osSurface),
        MT_VP_HAL_SURF_ALLOC_PARAM_IS_RES_OWNER, surface->isResourceOwner,
        MT_VP_HAL_SURF_ALLOC_PARAM_HANDLE, static_cast<int64_t>(surface->GetAllocationHandle(m_osInterface)),
        MT_VP_HAL_SURF_ALLOC_PARAM_SIZE, static_cast<int64_t>(surface->osSurface->OsResource.pGmmResInfo ? surface->osSurface->OsResource.pGmmResInfo->GetSizeAllocation() : 0));
    VP_PUBLIC_NORMALMESSAGE(
        "VP_HAL_DESTROY_SURF. "
        "VP_HAL_Surface: %p, "
        "VP_HAL_OsSurface: %p, "
        "VP_HAL_isResourceOwner: %d, "
        "VP_HAL_Surface_Handle: 0x%llx, "
        "VP_HAL_Surface_Size: 0x%llx",
        surface,
        surface->osSurface,
        surface->isResourceOwner ? 1 : 0,
        surface->GetAllocationHandle(m_osInterface),
        surface->osSurface->OsResource.pGmmResInfo ? surface->osSurface->OsResource.pGmmResInfo->GetSizeAllocation() : 0);

        int64_t currentSize = static_cast<int64_t>(surface->osSurface->OsResource.pGmmResInfo ? surface->osSurface->OsResource.pGmmResInfo->GetSizeAllocation() : 0);
        m_totalSize         = m_totalSize - currentSize;
#endif 
        status = DestroySurface(surface->osSurface, flags);
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
    VP_FUNC_CALL();
    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, lockFlag);
}

void* VpAllocator::LockResourceForWrite(MOS_RESOURCE *resource)
{
    VP_FUNC_CALL();
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

void* VpAllocator::LockResourceWithNoOverwrite(MOS_RESOURCE *resource)
{
    VP_FUNC_CALL();
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly   = 1;
    lockFlags.NoOverWrite = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

void* VpAllocator::LockResourceForRead(MOS_RESOURCE *resource)
{
    VP_FUNC_CALL();
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    if (!m_allocator)
        return nullptr;

    return m_allocator->Lock(resource, &lockFlags);
}

MOS_STATUS VpAllocator::UnLock(MOS_RESOURCE *resource)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->UnLock(resource);
}

MOS_STATUS VpAllocator::SkipResourceSync(MOS_RESOURCE *resource)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return m_allocator->SkipResourceSync(resource);
}

MOS_STATUS VpAllocator::GetSurfaceInfo(VPHAL_SURFACE *surface, VPHAL_GET_SURFACE_INFO &info)
{
    VP_FUNC_CALL();
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
    surface->dwYPitch        = resDetails.dwYPitch;
    surface->dwUPitch        = resDetails.dwUPitch;
    surface->dwVPitch        = resDetails.dwVPitch;
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
    surface->CacheSetting    = resDetails.CacheSetting;

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
        surface->CompressionMode = (mmcMode == MOS_MEMCOMP_MC) ? MOS_MMC_MC :
                                   (mmcMode == MOS_MEMCOMP_RC) ? MOS_MMC_RC : MOS_MMC_DISABLED;
    }
    else
    {
        surface->CompressionMode = MOS_MMC_DISABLED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::GetSurfaceInfo(VP_SURFACE* surface, VPHAL_GET_SURFACE_INFO& info)
{
    VP_FUNC_CALL();
    MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
    MOS_SURFACE       resDetails;

    VP_PUBLIC_CHK_NULL_RETURN(m_mmc);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_NULL_RETURN(surface);
    VP_PUBLIC_CHK_NULL_RETURN(surface->osSurface);

    if (Mos_ResourceIsNull(&surface->osSurface->OsResource))
    {
        VP_PUBLIC_NORMALMESSAGE("invalid resource handle");
        return MOS_STATUS_INVALID_HANDLE;
    }

    MOS_ZeroMemory(&resDetails, sizeof(MOS_SURFACE));
    resDetails.dwArraySlice = info.ArraySlice;
    resDetails.dwMipSlice   = info.MipSlice;
    resDetails.S3dChannel   = info.S3dChannel;
    resDetails.Format       = surface->osSurface->Format;

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(&surface->osSurface->OsResource, &resDetails));

    // Format_420O is mapped to Format_NV12 in VpHal here.
    // But it is mapped to several different Formats in CodecHal under different conditions.
    if (resDetails.Format == Format_420O)
    {
        resDetails.Format = Format_NV12;
    }

    // Get resource information
    surface->osSurface->dwWidth         = resDetails.dwWidth;
    surface->osSurface->dwHeight        = resDetails.dwHeight;
    surface->osSurface->dwPitch         = resDetails.dwPitch;
    surface->osSurface->dwSlicePitch    = resDetails.dwSlicePitch;
    surface->osSurface->dwDepth         = resDetails.dwDepth;
    surface->osSurface->TileType        = resDetails.TileType;
    surface->osSurface->TileModeGMM     = resDetails.TileModeGMM;
    surface->osSurface->bGMMTileEnabled = resDetails.bGMMTileEnabled;
    surface->osSurface->bOverlay        = resDetails.bOverlay ? true : false;
    surface->osSurface->bFlipChain      = resDetails.bFlipChain ? true : false;
    surface->osSurface->Format          = resDetails.Format;
    surface->osSurface->bCompressible   = resDetails.bCompressible ? true : false;
    surface->osSurface->bIsCompressed   = resDetails.bIsCompressed ? true : false;
    surface->osSurface->CacheSetting    = resDetails.CacheSetting;
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::AllocParamsInitType(
    MOS_ALLOC_GFXRES_PARAMS     &allocParams,
    PVPHAL_SURFACE              surface,
    MOS_GFXRES_TYPE             defaultResType,
    MOS_TILE_TYPE               defaultTileType)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(surface);

#if !EMUL && !LINUX && !ANDROID
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
    VP_FUNC_CALL();
    //  Need to reallocate surface according to expected tiletype instead of tiletype of the surface what we have
    if (surface != nullptr                                      &&
        surface->osSurface != nullptr                           &&
        !Mos_ResourceIsNull(&surface->osSurface->OsResource)    &&
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
        VP_SURFACE             *&surface,
        PCCHAR                  surfaceName,
        MOS_FORMAT              format,
        MOS_GFXRES_TYPE         defaultResType,
        MOS_TILE_TYPE           defaultTileType,
        uint32_t                width,
        uint32_t                height,
        bool                    compressible,
        MOS_RESOURCE_MMC_MODE   compressionMode,
        bool                    &allocated,
        bool                    zeroOnAllocate,
        bool                    deferredDestroyed,
        MOS_HW_RESOURCE_DEF     resUsageType,
        MOS_TILE_MODE_GMM       tileModeByForce,
        Mos_MemPool             memType,
        bool                    isNotLockable,
        void *                  systemMemory,
        uint32_t                depth)
{
    VP_FUNC_CALL();
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    MOS_ALLOC_GFXRES_PARAMS allocParams = {};
    MOS_GFXRES_FREE_FLAGS   resFreeFlags = {0};

    allocated = false;

    //---------------------------------
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    if (!m_mmc->IsMmcEnabled() || 
        !m_mmc->IsCompressibelSurfaceSupported())
    {
        compressible    = 0;
        compressionMode = MOS_MMC_DISABLED;
    }
    //---------------------------------

    auto surfInfoCheck = [=](VP_SURFACE *&surface) -> bool
                            {
                                return  (surface->osSurface->Format               == format)          &&
                                        ((surface->osSurface->bCompressible != 0) == compressible)    &&
                                        (surface->osSurface->CompressionMode      == compressionMode) &&
                                        (surface->osSurface->TileType             == defaultTileType  ||
                                        MOS_TILE_Y                                == defaultTileType  &&
                                        IS_Y_MAJOR_TILE_FORMAT(surface->osSurface->TileType))         &&
                                        ((Format_Buffer                           == format           &&
                                        surface->bufferWidth                      == width            &&
                                        surface->bufferHeight                     == height)          ||
                                        (Format_Buffer                            != format           &&
                                        surface->osSurface->dwWidth               == width            &&
                                        surface->osSurface->dwHeight              == height)           );
                            };

    // compressible should be compared with bCompressible since it is inited by bCompressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (surface                                                       &&
        surface->osSurface                                            &&
        !Mos_ResourceIsNull(&surface->osSurface->OsResource)          &&
        surfInfoCheck(surface)                                        )
    {
        return eStatus;
    }

    // reuse the allocated buffer if the allocated size was larger than request size when OptimizeCpuTiming is enabled
    if (m_osInterface->bOptimizeCpuTiming                               &&
        surface                                                         &&
        surface->osSurface                                              &&
        (!Mos_ResourceIsNull(&surface->osSurface->OsResource))          &&
        (Format_Buffer                                == format)        &&
        (surface->bufferWidth * surface->bufferHeight >= width * height))
    {
        return eStatus;
    }

    if (surface && nullptr == surface->osSurface)
    {
        // VP_SURFACE should always be allocated by interface in VpAllocator,
        // which will ensure nullptr != surface->osSurface.
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    //if free the compressed surface, need set the sync dealloc flag as 1 for sync dealloc for aux table update
    if (surface && IsSyncFreeNeededForMMCSurface(surface->osSurface))
    {
        resFreeFlags.SynchronousDestroy = 1;
        VP_PUBLIC_NORMALMESSAGE("Set SynchronousDestroy flag for compressed resource %s", surfaceName);
    }
    VP_PUBLIC_CHK_STATUS_RETURN(DestroyVpSurface(surface, deferredDestroyed, resFreeFlags));

    AllocParamsInitType(allocParams, surface, defaultResType, defaultTileType);

    allocParams.dwWidth         = width;
    allocParams.dwHeight        = height;
    allocParams.dwDepth         = depth;
    allocParams.Format          = format;
    allocParams.bIsCompressible = compressible;
    allocParams.CompressionMode = compressionMode;
    allocParams.pBufName        = surfaceName;
    allocParams.dwArraySize     = 1;
    allocParams.ResUsageType    = resUsageType;
    allocParams.m_tileModeByForce = tileModeByForce;
    allocParams.dwMemType       = memType;
    allocParams.Flags.bNotLockable = isNotLockable;
    allocParams.pSystemMemory      = systemMemory;

    surface = AllocateVpSurface(allocParams, zeroOnAllocate);
    VP_PUBLIC_CHK_NULL_RETURN(surface);
    VP_PUBLIC_CHK_NULL_RETURN(surface->osSurface);
    if (Mos_ResourceIsNull(&surface->osSurface->OsResource))
    {
        VP_PUBLIC_CHK_NULL_RETURN(nullptr);
    }

    if (!surfInfoCheck(surface))
    {
        VP_PUBLIC_ASSERTMESSAGE("Allocated surface is not matched with allocating parameter.");
        VP_PUBLIC_ASSERTMESSAGE("Allocated surface : format %d, compressible %d, compressionMode %d, tileType %d, bufferWidth %d, bufferHeight %d, width %d, height %d",
            surface->osSurface->Format,
            surface->osSurface->bCompressible,
            surface->osSurface->CompressionMode,
            surface->osSurface->TileType,
            surface->bufferWidth,
            surface->bufferHeight,
            surface->osSurface->dwWidth,
            surface->osSurface->dwHeight);
        VP_PUBLIC_ASSERTMESSAGE("Parameter to allocate : format %d, compressible %d, compressionMode %d, tileType %d, width %d, height %d",
            format,
            compressible,
            compressionMode,
            defaultTileType,
            width,
            height);
    }

    MT_LOG7(MT_VP_HAL_REALLOC_SURF, MT_NORMAL, MT_VP_HAL_INTER_SURF_TYPE, surfaceName ? *((int64_t*)surfaceName) : 0,
        MT_SURF_WIDTH, width, MT_SURF_HEIGHT, height, MT_SURF_MOS_FORMAT, format, MT_SURF_TILE_MODE, surface->osSurface->TileModeGMM,
        MT_SURF_COMP_ABLE, surface->osSurface->bCompressible, MT_SURF_COMP_MODE, surface->osSurface->CompressionMode);

#if !EMUL
    MT_LOG6(MT_VP_HAL_REALLOC_SURF, MT_NORMAL,
        MT_VP_HAL_SURF_ALLOC_PARAM_PTR, *(int64_t *)(&surface),
        MT_VP_HAL_SURF_ALLOC_PARAM_MOS_SURF_PTR, *(int64_t *)(&surface->osSurface),
        MT_VP_HAL_SURF_ALLOC_PARAM_IS_RES_OWNER, surface->isResourceOwner,
        MT_VP_HAL_SURF_ALLOC_PARAM_HANDLE, static_cast<int64_t>(surface->GetAllocationHandle(m_osInterface)),
        MT_VP_HAL_SURF_ALLOC_PARAM_SIZE, static_cast<int64_t>(surface->osSurface->OsResource.pGmmResInfo ? surface->osSurface->OsResource.pGmmResInfo->GetSizeAllocation() : 0),
        MT_VP_HAL_SURF_ALLOC_PARAM_NAME, surfaceName ? *((int64_t *)surfaceName) : 0);
    VP_PUBLIC_NORMALMESSAGE(
        "VP_HAL_REALLOC_SURF. "
        "VP_HAL_Surface: %p, "
        "VP_HAL_OsSurface: %p, "
        "VP_HAL_isResourceOwner: %d, "
        "VP_HAL_Surface_Handle: 0x%llx, "
        "VP_HAL_Surface_Size: 0x%llx, "
        "VP_HAL_Surface_Name: %s",
        surface,
        surface->osSurface,
        surface->isResourceOwner ? 1 : 0,
        surface->GetAllocationHandle(m_osInterface),
        surface->osSurface->OsResource.pGmmResInfo ? surface->osSurface->OsResource.pGmmResInfo->GetSizeAllocation() : 0,
        surfaceName ? surfaceName : "");
    int64_t currentSize  = static_cast<int64_t>(surface->osSurface->OsResource.pGmmResInfo ? surface->osSurface->OsResource.pGmmResInfo->GetSizeAllocation() : 0);
    m_totalSize          = m_totalSize + currentSize;
    m_peakSize           = m_peakSize > m_totalSize ? m_peakSize : m_totalSize;
#endif
    allocated = true;
    return MOS_STATUS_SUCCESS;
}

// for debug purpose
#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS VpAllocator::ReAllocateSurface(
    PVPHAL_SURFACE        surface,
    PCCHAR                surfaceName,
    MOS_FORMAT            format,
    MOS_GFXRES_TYPE       defaultResType,
    MOS_TILE_TYPE         defaultTileType,
    uint32_t              width,
    uint32_t              height,
    bool                  compressible,
    MOS_RESOURCE_MMC_MODE compressionMode,
    bool *                allocated,
    MOS_HW_RESOURCE_DEF   resUsageType,
    MOS_TILE_MODE_GMM     tileModeByForce)
{
    VP_FUNC_CALL();
    MOS_STATUS              eStatus;
    VPHAL_GET_SURFACE_INFO  info;
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_GFXRES_FREE_FLAGS   resFreeFlags = {0};

    //---------------------------------
    VP_PUBLIC_ASSERT(&surface->OsResource);
    //---------------------------------

    eStatus      = MOS_STATUS_SUCCESS;
    *allocated = false;

    // compressible should be compared with compressible since it is inited by compressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (!Mos_ResourceIsNull(&surface->OsResource) &&
        (surface->dwWidth == width) &&
        (surface->dwHeight == height) &&
        (surface->Format == format) &&
        (surface->bCompressible == compressible) &&
        (surface->CompressionMode == compressionMode) &&
        (surface->TileType == defaultTileType))
    {
        return eStatus;
    }

    // reuse the allocated buffer if the allocated size was larger than request size when OptimizeCpuTiming is enabled
    if (m_osInterface->bOptimizeCpuTiming                       &&
        (!Mos_ResourceIsNull(&surface->OsResource))           &&
        (Format_Buffer                        == format)        &&
        (surface->dwWidth * surface->dwHeight >= width * height))
    {
        return eStatus;
    }

    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    VpHal_AllocParamsInitType(&allocParams, surface, defaultResType, defaultTileType);

    allocParams.dwWidth           = width;
    allocParams.dwHeight          = height;
    allocParams.Format            = format;
    allocParams.bIsCompressible   = compressible;
    allocParams.CompressionMode   = compressionMode;
    allocParams.pBufName          = surfaceName;
    allocParams.dwArraySize       = 1;
    allocParams.ResUsageType      = resUsageType;
    allocParams.m_tileModeByForce = tileModeByForce;

    VP_PUBLIC_CHK_STATUS_RETURN(DestroyResource(&surface->OsResource));

    // Allocate surface
    VP_PUBLIC_CHK_STATUS_RETURN(AllocateResource(&surface->OsResource, allocParams));

    // Get surface information
    MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));

    // Pre-set to get surface info
    surface->Format = format;

    VP_PUBLIC_CHK_STATUS_RETURN(GetSurfaceInfo(surface, info));

    *allocated = true;

    VP_PUBLIC_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}
#endif

MOS_STATUS VpAllocator::ReAllocateVpSurfaceWithSameConfigOfVphalSurface(
    VP_SURFACE          *&surface,
    const PVPHAL_SURFACE &vphalSurface,
    PCCHAR                surfaceName,
    bool                 &allocated)
{
    VP_PUBLIC_CHK_NULL_RETURN(vphalSurface);

    MOS_GFXRES_TYPE     defaultResType  = MOS_GFXRES_INVALID;
    MOS_TILE_TYPE       defaultTileType = MOS_TILE_LINEAR;
    MOS_HW_RESOURCE_DEF resUsageType    = MOS_HW_RESOURCE_DEF_MAX;
    MOS_TILE_MODE_GMM   tileModeByForce = MOS_TILE_UNSET_GMM;
    if (Mos_ResourceIsNull(&vphalSurface->OsResource))
    {
        bool isBuffer   = (vphalSurface->Format == Format_Buffer || vphalSurface->Format == Format_RAW);
        defaultResType  = isBuffer ? MOS_GFXRES_BUFFER : MOS_GFXRES_2D;
        defaultTileType = isBuffer ? MOS_TILE_LINEAR : MOS_TILE_Y;
        resUsageType    = vphalSurface->OsResource.mocsMosResUsageType;
        tileModeByForce = vphalSurface->OsResource.TileModeGMM;
    }
    else
    {
        defaultResType = m_osInterface->pfnGetResType(&vphalSurface->OsResource);
        defaultTileType = vphalSurface->OsResource.TileType;
        resUsageType    = MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER;
        tileModeByForce = MOS_TILE_UNSET_GMM;
    }
    VP_PUBLIC_CHK_STATUS_RETURN(ReAllocateSurface(
        surface,
        surfaceName,
        vphalSurface->Format,
        defaultResType,
        defaultTileType,
        vphalSurface->dwWidth,
        vphalSurface->dwHeight,
        vphalSurface->bCompressible,
        vphalSurface->CompressionMode,
        allocated,
        false,
        false,
        resUsageType,
        tileModeByForce));

    // Initialize other parameters in vp surface according to vphal surface.
    surface->ColorSpace     = vphalSurface->ColorSpace;
    surface->ExtendedGamut  = vphalSurface->ExtendedGamut;
    surface->Palette        = vphalSurface->Palette;
    surface->bQueryVariance = vphalSurface->bQueryVariance;
    surface->FrameID        = vphalSurface->FrameID;
    surface->uFwdRefCount   = vphalSurface->uFwdRefCount;
    surface->uBwdRefCount   = vphalSurface->uBwdRefCount;
    surface->pFwdRef        = vphalSurface->pFwdRef;
    surface->pBwdRef        = vphalSurface->pBwdRef;
    surface->SurfType       = vphalSurface->SurfType;
    surface->SampleType     = vphalSurface->SampleType;
    surface->ChromaSiting   = vphalSurface->ChromaSiting;
    surface->rcSrc          = vphalSurface->rcSrc;
    surface->rcDst          = vphalSurface->rcDst;
    surface->rcMaxSrc       = vphalSurface->rcMaxSrc;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAllocator::OsFillResource(
    PMOS_RESOURCE     osResource,
    uint32_t          size,
    uint8_t           value)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    return m_allocator->OsFillResource(osResource, size, value);
}

MOS_STATUS VpAllocator::ReadSurface (
    PVPHAL_SURFACE      surface,
    uint32_t            bpp,
    uint8_t             *dst)
{
    VP_FUNC_CALL();
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

    src = (uint8_t*)LockResourceForRead(&surface->OsResource);
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
    VP_FUNC_CALL();
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

    dst = (uint8_t*)LockResourceForWrite(&surface->OsResource);
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
    VP_FUNC_CALL();
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

    dst = (uint8_t*)LockResourceForWrite(&surface->OsResource);
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

MOS_STATUS VpAllocator::Write1DSurface(VP_SURFACE* vpsurface, const uint8_t* src, uint32_t srcSize)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(vpsurface);
    VP_PUBLIC_CHK_NULL_RETURN(vpsurface->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(src);
    VP_PUBLIC_CHK_VALUE_RETURN(srcSize > 0, true);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_VALUE_RETURN(vpsurface->osSurface->dwSize > 0, true);

#if MOS_MEDIASOLO_SUPPORTED
    if (!m_osInterface->bSoloInUse)
#endif
    {
        VP_PUBLIC_CHK_VALUE_RETURN(vpsurface->osSurface->Type, MOS_GFXRES_BUFFER);
    }

    VP_PUBLIC_ASSERT(!Mos_ResourceIsNull(&vpsurface->osSurface->OsResource));

    MOS_SURFACE* surface = vpsurface->osSurface;
    uint8_t* dst = (uint8_t*)LockResourceForWrite(&surface->OsResource);

    VP_PUBLIC_CHK_NULL_RETURN(dst);

    MOS_STATUS status = MOS_SecureMemcpy(dst, surface->dwSize, src, srcSize);

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->UnLock(&surface->OsResource));

    return status;
}

MOS_STATUS VpAllocator::SyncOnResource(
    PMOS_RESOURCE         osResource,
    bool                  bWriteOperation)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return (m_allocator->SyncOnResource(osResource, bWriteOperation));
}

MOS_STATUS VpAllocator::UpdateResourceUsageType(
    PMOS_RESOURCE           osResource,
    MOS_HW_RESOURCE_DEF     resUsageType)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    return (m_allocator->UpdateResourceUsageType(osResource, resUsageType));
}


bool VpAllocator::IsSyncFreeNeededForMMCSurface(PMOS_SURFACE pOsSurface)
{
    VP_FUNC_CALL();
    if (nullptr == pOsSurface)
    {
        return false;
    }

    return (m_allocator->isSyncFreeNeededForMMCSurface(pOsSurface));
}

void VpAllocator::CleanRecycler()
{
    VP_FUNC_CALL();
    while (!m_recycler.empty())
    {
        MOS_GFXRES_FREE_FLAGS resFreeFlags = {};
        VP_SURFACE *surf = m_recycler.back();
        m_recycler.pop_back();
        //if free the compressed surface, need set the sync dealloc flag as 1 for sync dealloc for aux table update
        if (surf && IsSyncFreeNeededForMMCSurface(surf->osSurface))
        {
            resFreeFlags.SynchronousDestroy = 1;
            VP_PUBLIC_NORMALMESSAGE("Set SynchronousDestroy flag for compressed resource.");
        }
        DestroyVpSurface(surf, false, resFreeFlags);
    }
}

MOS_STATUS VpAllocator::AllocateCPUResource(PMOS_RESOURCE osResource, size_t linearAddress, uint32_t dataSize, uint32_t height, uint64_t width, uint64_t planePitch, uint32_t CpTag, GMM_RESOURCE_FORMAT Format)
{
    VP_FUNC_CALL();
    MOS_STATUS eStatusKey = MOS_STATUS_SUCCESS;
    //Create MOS_GFXRES_BUFFER type resource
    
    size_t   linearAddressAligned = 0;
    uint32_t addedShiftLeftOffset = 0;
    MOS_ALLOC_GFXRES_PARAMS sParams;

    if ((linearAddress & 0xf) || (linearAddress == 0))
    {
        VP_PUBLIC_NORMALMESSAGE("Error: Start address of system memory is not 16-byte aligned!");
        return MOS_STATUS_UNKNOWN;
    }
    if (sizeof(void *) == 8)  //64-bit
    {
        linearAddressAligned = linearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X64;
    }
    else  //32-bit
    {
        linearAddressAligned = linearAddress & ADDRESS_PAGE_ALIGNMENT_MASK_X86;
    }
    //Calculate  Left Shift offset
    addedShiftLeftOffset = (uint32_t)(linearAddress - linearAddressAligned);
    VP_PUBLIC_NORMALMESSAGE("System memory address: 0x%x, Aligned address: 0x%x, offset: %u", linearAddress, linearAddressAligned, addedShiftLeftOffset);

    MOS_ZeroMemory(&sParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    sParams.Type           = MOS_GFXRES_BUFFER;
    sParams.dwBytes        = dataSize + addedShiftLeftOffset;
    sParams.pSystemMemory  = (void *)linearAddressAligned;
    sParams.TileType       = MOS_TILE_LINEAR;
    sParams.Format         = Format_Buffer;
    sParams.pBufName       = "cpuResourceBuffer_1D";
    sParams.bBypassMODImpl = 1;
    sParams.bIsPersistent  = 1;
    
    eStatusKey = m_osInterface->pfnAllocateResource(m_osInterface, &sParams, osResource);
    if (eStatusKey != MOS_STATUS_SUCCESS)
    {
        VP_PUBLIC_NORMALMESSAGE("Error: m_osInterface.pfnAllocateResource failed!");
        return eStatusKey;
    }
#if !EMUL
    osResource->pGmmResInfo->GetSetCpSurfTag(true, CpTag);
    osResource->pGmmResInfo->OverridePitch(planePitch);

    osResource->pGmmResInfo->OverrideSurfaceFormat(Format);
    osResource->pGmmResInfo->OverrideSurfaceType(RESOURCE_2D);
    osResource->pGmmResInfo->OverrideBaseWidth(width);
    osResource->pGmmResInfo->OverrideBaseHeight(height);

    osResource->pGmmResInfo->OverridePlanarXOffset(GMM_NO_PLANE, (GMM_GFX_SIZE_T)addedShiftLeftOffset);
    osResource->pGmmResInfo->OverridePlanarYOffset(GMM_NO_PLANE, 0);
    osResource->pGmmResInfo->OverridePlanarXOffset(GMM_PLANE_Y, (GMM_GFX_SIZE_T)addedShiftLeftOffset);
    osResource->pGmmResInfo->OverridePlanarYOffset(GMM_PLANE_Y, 0);
#endif

    return eStatusKey;

}

bool VP_SURFACE::IsEmpty()
{
    VP_FUNC_CALL();
    return nullptr == osSurface || Mos_ResourceIsNull(&osSurface->OsResource);
}

MOS_STATUS VP_SURFACE::Clean()
{
    VP_FUNC_CALL();
    // The vp surface, which owns the resource, cannot be cleaned.
    if (isResourceOwner)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }
    if (osSurface)
    {
        MOS_ZeroMemory(osSurface, sizeof(MOS_SURFACE));
    }

    isResourceOwner     = false;
    ColorSpace          = CSpace_Any;
    ChromaSiting        = 0;
    bQueryVariance      = 0;
    FrameID             = 0;
    ExtendedGamut       = false;
    SurfType            = SURF_NONE;
    uFwdRefCount        = 0;
    uBwdRefCount        = 0;
    pFwdRef             = nullptr;
    pBwdRef             = nullptr;
    SampleType          = SAMPLE_PROGRESSIVE;
    MOS_ZeroMemory(&Palette, sizeof(Palette));
    MOS_ZeroMemory(&rcSrc, sizeof(rcSrc));
    MOS_ZeroMemory(&rcDst, sizeof(rcDst));
    MOS_ZeroMemory(&rcMaxSrc, sizeof(rcMaxSrc));
    bVEBOXCroppingUsed  = false;

    return MOS_STATUS_SUCCESS;
}

uint64_t VP_SURFACE::GetAllocationHandle(MOS_INTERFACE* osIntf)
{
    VP_FUNC_CALL();
    if (nullptr == osIntf)
    {
        return 0;
    }
#if MOS_MEDIASOLO_SUPPORTED
    if (osIntf && osIntf->bSoloInUse)
    {
        uint64_t handle = osSurface ? (uint64_t)osSurface->OsResource.pData : 0;
        if (handle)
        {
            return handle;
        }
        // Media solo external surface will come here, in which case,
        // AllocationHandle or bo->handle should be valid.
    }
#endif

    if (nullptr == osSurface || nullptr == osIntf)
    {
        return 0;
    }
    return osIntf->pfnGetResourceHandle(osIntf->osStreamState, &osSurface->OsResource);
}

MOS_STATUS VpAllocator::SetMmcFlags(MOS_SURFACE &osSurface)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_mmc);

    // Init MMC related flags.
    m_mmc->SetSurfaceMmcMode(&osSurface);
    if (osSurface.CompressionMode   &&
        (osSurface.TileType == MOS_TILE_Y ||
         osSurface.TileType == MOS_TILE_YS))
    {
        uint32_t mmcFormat   = 0;

        osSurface.bCompressible   = true;
        osSurface.bIsCompressed   = true;
        VP_PUBLIC_CHK_STATUS_RETURN(m_mmc->GetSurfaceMmcFormat(&osSurface, &mmcFormat));
        osSurface.CompressionFormat = mmcFormat;
    }
    else
    {
        // Do not modify the bCompressible flag even MmcMode is disable, since the surface size/pitch may be different
        // between Compressible and Uncompressible, which will affect the DN surface allocation.
        osSurface.bIsCompressed     = false;
        osSurface.CompressionMode   = MOS_MMC_DISABLED;
        osSurface.CompressionFormat = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_HW_RESOURCE_DEF VpAllocator::GetResourceCache(uint32_t feature, bool bOut, ENGINE_TYPE engineType, MOS_COMPONENT id)
{
    VP_FUNC_CALL();
    MOS_CACHE_ELEMENT element(MOS_CODEC_RESOURCE_USAGE_BEGIN_CODEC, MOS_CODEC_RESOURCE_USAGE_BEGIN_CODEC);
    bool              res = m_osInterface->pfnGetCacheSetting(id, SUFACE_TYPE_ASSIGNED(feature), bOut, engineType, element, false);
    if (res == false)
    {
        VP_PUBLIC_ASSERTMESSAGE("Resource %u was not found in cache manager, use default usage MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER!", feature);
        return MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER;
    }

    VP_PUBLIC_NORMALMESSAGE("Resource %u was found in cache manager, use mocs usage %u!", feature, element.mocsUsageType);
    return element.mocsUsageType;
}
