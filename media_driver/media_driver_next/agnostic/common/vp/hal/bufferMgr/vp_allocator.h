/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \brief    Defines the interface for vp resouce allocate
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
    VpAllocator(PMOS_INTERFACE osInterface, VPMediaMemComp *mmc);

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
    //! \brief  Allocate Surface
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
    //! \brief  Allocate Surface
    //! \param  [in] vphalSurf
    //!         The vphal surface that vp surface created from. The resource will be reused in vp surface.
    //! \return VP_SURFACE*
    //!         return the pointer to VP_SURFACE
    //!
    VP_SURFACE* AllocateVpSurface(VPHAL_SURFACE &vphalSurf);

    //!
    //! \brief  Destroy Surface
    //! \param  [in] surface
    //!         Pointer to VP_SURFACE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyVpSurface(VP_SURFACE *&surface);

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
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroySurface(MOS_SURFACE* surface);

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
    void* LockResouceForWrite(MOS_RESOURCE *resource);

    //!
    //! \brief  Lock resource with no overwrite flag
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return void*
    //!         a poniter to data
    //!
    void* LockResouceWithNoOverwrite(MOS_RESOURCE *resource);

    //!
    //! \brief  Lock resource only for reading
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return void*
    //!         a poniter to data
    //!
    void* LockResouceForRead(MOS_RESOURCE *resource);

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
    //! \brief    Allocates the Surface
    //! \details  Allocates the Surface
    //!           - if the surface is not already allocated OR
    //!           - resource dimenisions OR format changed
    //! \param    [in,out] surface
    //!           Pointer to VPHAL_SURFACE
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
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    MOS_STATUS ReAllocateSurface(
        PVPHAL_SURFACE          surface,
        PCCHAR                  surfaceName,
        MOS_FORMAT              format,
        MOS_GFXRES_TYPE         defaultResType,
        MOS_TILE_TYPE           defaultTileType,
        uint32_t                width,
        uint32_t                height,
        bool                    compressible,
        MOS_RESOURCE_MMC_MODE   compressionMode,
        bool                    &allocated);

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
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success. Error code otherwise
    //!
    MOS_STATUS ReAllocateSurface(
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
        bool                    zeroOnAllocate = 0);

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

protected:
    PMOS_INTERFACE  m_osInterface   = nullptr;
    Allocator       *m_allocator    = nullptr;
    VPMediaMemComp  *m_mmc          = nullptr;
};

typedef VpAllocator* PVpAllocator;
}
#endif // !__vp_ALLOCATOR_H__
