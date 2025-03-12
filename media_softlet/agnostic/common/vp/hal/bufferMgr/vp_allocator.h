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
//! \file     vp_allocator.h
//! \brief    Defines the interface for vp resource allocate
//! \details  vp allocator will allocate and destory buffers, the caller
//!           can use directly
//!

#ifndef __VP_ALLOCATOR_H__
#define __VP_ALLOCATOR_H__

#include "media_allocator.h"
#include "vp_mem_compression.h"
#include "vp_vebox_common.h"
#include "vp_pipeline_common.h"

namespace vp {

class VpAllocator
{
public:
    //!
    //! \brief  Constructor
    //! \param  [in] osInterface
    //!         Pointer to MOS_INTERFACE
    //!
    VpAllocator(PMOS_INTERFACE osInterface, MediaMemComp *mmc);

    //!
    //! \brief  vpAllocator Destructor
    //!
    ~VpAllocator();

    //!
    //! \brief  New MOS_RESOURCE and Allocate Resource, add the resource in the resource pool, using DestroyResource or DestroyAllResources to free resource.
    //! \param  [in] component
    //!         component type to track the buffer
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] zeroOnAllocate
    //!         zero the memory if true
    //! \return MOS_RESOURCE*
    //!         return the pointer to MOS_RESOURCE
    //!
    MOS_RESOURCE* AllocateResource(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate);

    //!
    //! \brief  Destroy a resource registered on the resource pool and free its MOS_RESOURCE struct
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyResource(MOS_RESOURCE* resource);

    //!
    //! \brief  Destroy all resources registered on the resource pool and free the MOS_RESOURCE struct
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyAllResources();

    //!
    //! \brief  Allocate Resource but not register the resource to the resource pool, using FreeResource to free the resource
    //! \param  [in] component
    //!         component type to track the buffer
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \return MOS_RESOURCE*
    //!         return the pointer to MOS_RESOURCE
    //!
    MOS_STATUS AllocateResource(MOS_RESOURCE *res, MOS_ALLOC_GFXRES_PARAMS &param);

    //!
    //! \brief  Free Resource
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeResource(MOS_RESOURCE *resource);

    //!
    //! \brief  Allocate vp surface
    //! \param  [in] component
    //!         component type to track the buffer
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] zeroOnAllocate
    //!         zero the memory if true
    //! \param  [in] ColorSpace
    //!         Surface color space config
    //! \param  [in] ChromaSiting
    //!         Surface chromasiting config
    //! \param  [in] ChromaSiting
    //!         Surface rotation config
    //! \return VP_SURFACE*
    //!         return the pointer to VP_SURFACE
    //!
    VP_SURFACE* AllocateVpSurface(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate = false, VPHAL_CSPACE ColorSpace = CSpace_None, uint32_t ChromaSiting = 0);

    //!
    //! \brief  Allocate vp surface
    //! \param  [in] vphalSurf
    //!         The vphal surface that vp surface created from. The resource will be reused in vp surface.
    //! \return VP_SURFACE*
    //!         return the pointer to VP_SURFACE
    //!
    VP_SURFACE* AllocateVpSurface(VPHAL_SURFACE &vphalSurf);

    //!
    //! \brief  Allocate vp surface
    //! \param  [in] vpSurf
    //!         The surface that vp surface created from. The resource will be reused in vp surface.
    //! \return VP_SURFACE*
    //!         return the pointer to VP_SURFACE
    //!
    VP_SURFACE* AllocateVpSurface(VP_SURFACE &vphalSurf);

    //!
    //! \brief  Allocate vp surface
    //! \param  [in] osSurf
    //!         The surface that vp surface created from. The resource will be reused in vp surface.
    //! \param  [in] colorSpace
    //!         colorSpace of vp surface.
    //! \param  [in] chromaSiting
    //!         chromaSiting of vp surface.
    //! \param  [in] rcSrc
    //!         rcSrc of vp surface.
    //! \param  [in] rcDst
    //!         rcDst of vp surface.
    //! \param  [in] SurfType
    //!         SurfType of vp surface.
    //! \param  [in] updatePlaneOffset
    //!         true, update plane offset of vp surface, otherwise, use the one in osSurf.
    //! \return VP_SURFACE*
    //!         return the pointer to VP_SURFACE
    //!
    VP_SURFACE *AllocateVpSurface(
        MOS_SURFACE &osSurf,
        VPHAL_CSPACE colorSpace,
        uint32_t chromaSiting,
        RECT rcSrc,
        RECT rcDst,
        VPHAL_SURFACE_TYPE SurfType,
        bool updatePlaneOffset = false);

    //!
    //! \brief  Allocate vp surface without resource
    //! \return VP_SURFACE*
    //!         return the pointer to VP_SURFACE
    //!
    VP_SURFACE *AllocateVpSurface();

    //!
    //! \brief  Copy vp surface from src to dst
    //! \param  [in] dst
    //!         The target vp surface for copy. The isResourceOwner flag should be false.
    //! \param  [in] src
    //!         The source vp surface for copy. The resource will be reused in dst.
    //! \return MOS_STATUS
    //!         return MOS_STATUS_SUCCESS if no error occur.
    //!
    MOS_STATUS CopyVpSurface(VP_SURFACE &dst, VP_SURFACE &src);

    //!
    //! \brief  Destroy Surface
    //! \param  [in] surface
    //!         Pointer to VP_SURFACE
    //! \param  [in] deferredDestroyed
    //!         Deferred destroy the resource until CleanRecycler being called.
    //! \param  [in] flags
    //!         flags for vp surface destroy
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyVpSurface(VP_SURFACE *&surface, bool deferredDestroyed = false, MOS_GFXRES_FREE_FLAGS flags = {0});

    //!
    //! \brief  Allocate Surface
    //! \param  [in] component
    //!         component type to track the buffer
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] zeroOnAllocate
    //!         zero the memory if true
    //! \return MOS_SURFACE*
    //!         return the pointer to MOS_SURFACE
    //!
    MOS_SURFACE* AllocateSurface(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate);

    //!
    //! \brief  Destroy Surface
    //! \param  [in] surface
    //!         Pointer to MOS_SURFACE
    //! \param  [in] flags
    //!         flags for surface destroy
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroySurface(MOS_SURFACE* surface, MOS_GFXRES_FREE_FLAGS flags = {0});

    //!
    //! \brief  Lock resource
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \param  [in] lockFlag
    //!         Pointer to MOS_LOCK_PARAMS
    //! \return void*
    //!         a poniter to data
    //!
    void* Lock(MOS_RESOURCE *resource, MOS_LOCK_PARAMS *lockFlag);

    //!
    //! \brief  Lock resource only for writing
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return void*
    //!         a poniter to data
    //!
    void* LockResourceForWrite(MOS_RESOURCE *resource);

    //!
    //! \brief  Lock resource with no overwrite flag
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return void*
    //!         a poniter to data
    //!
    void* LockResourceWithNoOverwrite(MOS_RESOURCE *resource);

    //!
    //! \brief  Lock resource only for reading
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return void*
    //!         a poniter to data
    //!
    void* LockResourceForRead(MOS_RESOURCE *resource);

    //!
    //! \brief  UnLock resource
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UnLock(MOS_RESOURCE *resource);

    //!
    //! \brief  Skip sync resource
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SkipResourceSync(MOS_RESOURCE* resource);

    //!
    //! \brief  get surface info from resource
    //! \param  [in, out] surface
    //!         Pointer to VPHAL_SURFACE
    //! \param  [in] info
    //!         Reference to PVPHAL_GET_SURFACE_INFO
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetSurfaceInfo(VPHAL_SURFACE *surface, VPHAL_GET_SURFACE_INFO &info);

    MOS_STATUS GetSurfaceInfo(VP_SURFACE* surface, VPHAL_GET_SURFACE_INFO& info);

    //!
    //! \brief    Initial the Type/TileType fields in Alloc Params structure
    //! \details  Initial the Type/TileType fields in Alloc Params structure
    //!           - Use the last type from GMM resource
    //! \param    [in, out] allocParams
    //!           Reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param    [in] surface
    //!           Pointer to VPHAL_SURFACE
    //! \param    [in] defaultResType
    //!           Expected Resource Type
    //! \param    [in] defaultTileType
    //!           Expected Surface Tile Type
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocParamsInitType(
        MOS_ALLOC_GFXRES_PARAMS     &allocParams,
        PVPHAL_SURFACE              surface,
        MOS_GFXRES_TYPE             defaultResType,
        MOS_TILE_TYPE               defaultTileType);

    //!
    //! \brief    Initial the Type/TileType fields in Alloc Params structure
    //! \details  Initial the Type/TileType fields in Alloc Params structure
    //!           - Use the last type from GMM resource
    //! \param    [in, out] allocParams
    //!           Reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param    [in] surface
    //!           Pointer to VP_SURFACE
    //! \param    [in] defaultResType
    //!           Expected Resource Type
    //! \param    [in] defaultTileType
    //!           Expected Surface Tile Type
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocParamsInitType(
        MOS_ALLOC_GFXRES_PARAMS     &allocParams,
        VP_SURFACE                  *surface,
        MOS_GFXRES_TYPE             defaultResType,
        MOS_TILE_TYPE               defaultTileType);

    //!
    //! \brief    Reallocates the VP Surface
    //! \details  Reallocates the VP Surface
    //!           - if the surface is not already allocated OR
    //!           - resource dimenisions OR format changed
    //! \param    [in,out] surface
    //!           Pointer to VP_SURFACE
    //! \param    [in] surfaceName
    //!           Pointer to surface name
    //! \param    [in] format
    //!           Expected MOS_FORMAT
    //! \param    [in] defaultResType
    //!           Expected Resource Type
    //! \param    [in] defaultTileType
    //!           Expected Surface Tile Type
    //! \param    [in] width
    //!           Expected Surface Width
    //! \param    [in] height
    //!           Expected Surface Height
    //! \param    [in] compressible
    //!           Surface compressible or not
    //! \param    [in] compressionMode
    //!           Compression Mode
    //! \param    [out] allocated
    //!           true if allocated, false for not
    //! \param    [in] zeroOnAllocate
    //!           zero when surface allocated
    //! \param    [in] deferredDestroyed
    //!           Deferred destroy the resource until CleanRecycler being called.
    //! \param    [in] resUsageType
    //!           resource usage type for cache policy
    //! \param    [in] Flag to indicate whether resource being lockable
    //!           resource usage type for cache policy
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    MOS_STATUS ReAllocateSurface(
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
        bool                    zeroOnAllocate = 0,
        bool                    deferredDestroyed = false,
        MOS_HW_RESOURCE_DEF     resUsageType   = MOS_HW_RESOURCE_DEF_MAX,
        MOS_TILE_MODE_GMM       tileModeByForce = MOS_TILE_UNSET_GMM,
        Mos_MemPool             memType = MOS_MEMPOOL_VIDEOMEMORY,
        bool                    isNotLockable = false,
        void                    *systemMemory = nullptr,
        uint32_t                depth = 0);

    //!
    //! \brief    Allocates the Surface
    //! \details  Allocates the Surface
    //!           - if the surface is not already allocated OR
    //!           - resource dimenisions OR format changed
    //! \param    [in,out] pSurface
    //!           Pointer to VPHAL_SURFACE
    //! \param    [in] pSurfaceName
    //!           Pointer to surface name
    //! \param    [in] Format
    //!           Expected MOS_FORMAT
    //! \param    [in] DefaultResType
    //!           Expected Resource Type
    //! \param    [in] DefaultTileType
    //!           Expected Surface Tile Type
    //! \param    [in] dwWidth
    //!           Expected Surface Width
    //! \param    [in] dwHeight
    //!           Expected Surface Height
    //! \param    [in] bCompressible
    //!           Surface being compressible or not
    //! \param    [in] CompressionMode
    //!           Compression Mode
    //! \param    [out] pbAllocated
    //!           true if allocated, false for not
    //! \param    [in] resUsageType
    //!           resource usage type for caching
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    // for debug purpose
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS ReAllocateSurface(
        PVPHAL_SURFACE        surface,                                   // [in/out]Pointer to surface
        PCCHAR                surfaceName,                               // [in]    Pointer to surface name
        MOS_FORMAT            format,                                    // [in]    Surface Format
        MOS_GFXRES_TYPE       defaultResType,                            // [in]    Default Resource Type to use if resource has not be allocated yet
        MOS_TILE_TYPE         defaultTileType,                           // [in]    Default Resource Tile Type to use if resource has not be allocated yet
        uint32_t              width,                                     // [in]    Resource Width
        uint32_t              height,                                    // [in]    Resource Height
        bool                  compressible,                              // [in]    Flag indaicated reource is compressible or not
        MOS_RESOURCE_MMC_MODE compressionMode,                           // [in]    Compression mode
        bool *                allocated,                                 // [out]   Flag indicating new allocation
        MOS_HW_RESOURCE_DEF   resUsageType    = MOS_HW_RESOURCE_DEF_MAX, // [in]    resource usage type
        MOS_TILE_MODE_GMM     tileModeByForce = MOS_TILE_UNSET_GMM);     // [in]    Flag to indicate if GMM flag tile64 need set
#endif

    //!
    //! \brief    Reallocates the VP Surface w/ the same config of a Vphal Surface
    //! \details  Reallocates the VP Surface w/ the same config of a Vphal Surface
    //!           - if the surface is not already allocated OR
    //!           - resource dimenisions OR format changed
    //! \param    [in,out] surface
    //!           Pointer to VP_SURFACE
    //! \param    [in] surfaceName
    //!           Pointer to surface name
    //! \param    [in] vphalSurface
    //!           Pointer to vphal surface which is the source config of allocated vp surface
    //! \param    [out] allocated
    //!           true if allocated, false for not

    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    MOS_STATUS ReAllocateVpSurfaceWithSameConfigOfVphalSurface(
        VP_SURFACE          *&surface,
        const PVPHAL_SURFACE &vphalSurface,
        PCCHAR                surfaceName,
        bool                 &allocated);

    //!
    //! \brief    Unified OS fill Resource
    //! \details  Locks the surface and fills the resource with data
    //! \param    PMOS_RESOURCE pOsResource
    //!           [in] Pointer to OS Resource
    //! \param    uint32_t dwSize
    //!           [in] Size of the Buffer
    //! \param    uint8_t iValue
    //!           [in] Value to be filled
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS OsFillResource(
        PMOS_RESOURCE     osResource,
        uint32_t          size,
        uint8_t           value);

    //!
    //! \brief    Reads the Surface contents and copy to the Dst Buffer
    //! \details  Reads the Surface contents and copy to the Dst Buffer
    //!           - 1 lock surface
    //!           - 2 copy surface data to pDst
    //!           - 3 unlock surface
    //! \param    [in] surface
    //!           Pointer to VPHAL_SURFACE
    //! \param    [in] bpp
    //!           bit per pixel of surface contents
    //! \param    [out] dst
    //!           output buffer to store Surface contents
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    MOS_STATUS ReadSurface(
        PVPHAL_SURFACE      surface,
        uint32_t            bpp,
        uint8_t             *dst);

    //!
    //! \brief    Copy Data from input Buffer to the Surface contents
    //! \details  Copy Data from input Buffer to the Surface contents
    //!           - 1 lock surface
    //!           - 2 copy data from pSrc to Surface
    //!           - 3 unlock surface
    //! \param    [out] Surface
    //!           Pointer to VPHAL_SURFACE
    //! \param    [in] Bpp
    //!           bit per pixel of input buffer
    //! \param    [in] Src
    //!           Input buffer to store Surface contents
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    MOS_STATUS WriteSurface(
        PVPHAL_SURFACE      surface,
        uint32_t            bpp,
        const uint8_t       *src);

    //!
    //! \brief    Copy Data from input Buffer to the Surface contents
    //! \details  Copy Data from input Buffer to the Surface contents
    //!           - 1 lock surface
    //!           - 2 copy data from pSrc to Surface
    //!           - 3 unlock surface
    //! \param    [out] Surface
    //!           Pointer to VP_SURFACE
    //! \param    [in] Bpp
    //!           bit per pixel of input buffer
    //! \param    [in] Src
    //!           Input buffer to store Surface contents
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    MOS_STATUS WriteSurface(
        VP_SURFACE         *surface,
        uint32_t            bpp,
        const uint8_t      *src);

    //!
    //! \brief    Copy Data from input Buffer to the Surface contents
    //! \details  Copy Data from input Buffer to the Surface contents
    //!           - 1 lock surface
    //!           - 2 copy data from pSrc to Surface
    //!           - 3 unlock surface
    //! \param    [out] Surface
    //!           Pointer to VP_SURFACE
    //! \param    [in] Src
    //!           Input buffer to store Surface contents
    //! \param    [in] srcSize
    //!           size of Src to be copied.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    MOS_STATUS Write1DSurface(
        VP_SURFACE         *surface,
        const uint8_t      *src,
        uint32_t            srcSize);

    //!
    //! \brief    Tag based synchronization at the resource level
    //! \details  Tag based synchronization at the resource level
    //! \param    PMOS_RESOURCE OsResource
    //!           [in] OS resource sturcture
    //! \param    BOOL bWriteOperation
    //!           [in] Indicate if it is a write operation
    //! \return   VOID
    //!
    MOS_STATUS SyncOnResource(
        PMOS_RESOURCE         osResource,
        bool                  bWriteOperation);

    //!
    //! \brief    Update the usage type of resource for cache policy
    //! \details  Update the usage type of resource for cache policy
    //! \param    PMOS_RESOURCE OsResource
    //!           [in] OS resource sturcture
    //! \param    MOS_HW_RESOURCE_DEF resUsageType
    //!           [in] MOS_HW_RESOURCE_DEF to be set
    //! \return   VOID
    //!
    MOS_STATUS UpdateResourceUsageType(
        PMOS_RESOURCE           osResource,
        MOS_HW_RESOURCE_DEF     resUsageType);

    //!
    //! \brief    Check if sync free needed for compressed surface
    //! \param    PMOS_SURFACE pOsSurface
    //!           [in] os surface pointer
    //! \return   bool
    //!           true if success, otherwise failed reason
    //!
    bool IsSyncFreeNeededForMMCSurface(PMOS_SURFACE pOsSurface);
    void CleanRecycler();

    //!
    //! \brief    Allocate resource from cpu buffer
    //! \details  Allocate resource from cpu buffer
    //! \param    PMOS_RESOURCE pOsResource
    //!           [in/out] Pointer to OS resource
    //! \param    size_t linearAddress
    //!           [in]    CPU address
    //! \param    uint32_t dataSize
    //!           [in]    data size of CPU buffer
    //! \param    uint32_t height
    //!           [in]    height of resource
    //! \param    uint64_t width
    //!           [in]    width of resource
    //! \param    uint64_t planePitch
    //!           [in]    pitch of resource
    //! \param    uint32_t CpTag
    //!           [in]    Cp surface tag value
    //! \param    GMM_RESOURCE_FORMAT Format
    //!           [in]    resouce format
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AllocateCPUResource(
        PMOS_RESOURCE osResource,           // [in/out]Pointer to OS resource
        size_t linearAddress,               // [in]    CPU address
        uint32_t dataSize,                  // [in]    data size of CPU buffer
        uint32_t height,                    // [in]    height of resource
        uint64_t width,                     // [in]    width of resource
        uint64_t planePitch,                // [in]    pitch of resource
        uint32_t CpTag,                     // [in]    Cp surface tag value
        GMM_RESOURCE_FORMAT Format          // [in]    resouce format
    );

    MOS_HW_RESOURCE_DEF GetResourceCache(uint32_t feature, bool bOut, ENGINE_TYPE engineType, MOS_COMPONENT id = COMPONENT_VPCommon);

    int64_t GetTotalSize()
    {
        return m_totalSize;
    }

    int64_t GetPeakSize()
    {
        return m_peakSize;
    }

protected:
    //!
    //! \brief    Set mmc flags to surface
    //! \details  Set mmc flags to surface
    //! \param    MOS_SURFACE &osSurface
    //!           [in, out] OS surface sturcture
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetMmcFlags(MOS_SURFACE &osSurface);

    //!
    //! \brief    Update surface plane offset
    //! \details  Update surface plane offset with render offset
    //! \param    surf
    //!           [in, out] surface to be updated.
    //! \return   VOID
    //!
    void UpdateSurfacePlaneOffset(MOS_SURFACE &surf);

    PMOS_INTERFACE  m_osInterface   = nullptr;
    Allocator       *m_allocator    = nullptr;
    MediaMemComp    *m_mmc          = nullptr;
    std::vector<VP_SURFACE *> m_recycler;   // Container for delayed destroyed surface.
    int64_t         m_totalSize     = 0; // current total memory size.
    int64_t         m_peakSize      = 0;  // the peak value of memory size.

MEDIA_CLASS_DEFINE_END(vp__VpAllocator)
};

typedef VpAllocator* PVpAllocator;
}
#endif // !__vp_ALLOCATOR_H__
