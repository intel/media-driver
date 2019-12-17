/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_allocator.h
//! \brief    Defines the common interface for media resouce manage
//! \details  Media allocator will allocate and destory buffers, each component
//!           is recommended to inherits and implement it's owner allocator
//!

#ifndef __MEDIA_ALLOCATOR_H__
#define __MEDIA_ALLOCATOR_H__
#include "mos_os.h"
#include <map>

class Allocator
{
public:
    //!
    //! \brief  Static method to get the allocator instance
    //! \param  [in] osInterface
    //!         pointer to MOS_INTERFACE
    //! \return Allocator&
    //!         reference to static allocator
    //!
    static Allocator &GetAllocator(PMOS_INTERFACE osInterface)
    {
        static Allocator allocator(osInterface);
        return allocator;
    }

    //!
    //! \brief  Constructor
    //! \param  [in] osInterface
    //!         Pointer to MOS_INTERFACE
    //!
    Allocator(PMOS_INTERFACE osInterface);

    //!
    //! \brief  Destructor
    //!
    ~Allocator();

    //!
    //! \brief  Destroy all resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, otherwise failed reason
    //!
    MOS_STATUS DestroyAllResources();

    //!
    //! \brief  Allocate Resource
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] zeroOnAllocate
    //!         zero the memory if true
    //! \param  [in] component
    //!         component type to track the buffer
    //! \return MOS_RESOURCE*
    //!         return the pointer to MOS_RESOURCE
    //!
    MOS_RESOURCE *AllocateResource(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate, MOS_COMPONENT component);

    //!
    //! \brief  Allocate Buffer
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] zeroOnAllocate
    //!         zero the memory if true
    //! \param  [in] component
    //!         component type to track the buffer
    //! \return MOS_BUFFER*
    //!         return the pointer to MOS_BUFFER
    //!
    PMOS_BUFFER AllocateBuffer(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate, MOS_COMPONENT component);

    //!
    //! \brief  Allocate Surface
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] zeroOnAllocate
    //!         zero the memory if true
    //! \param  [in] component
    //!         component type to track the buffer
    //! \return MOS_SURFACE*
    //!         return the pointer to MOS_SURFACE
    //!
    MOS_SURFACE *AllocateSurface(MOS_ALLOC_GFXRES_PARAMS &param, bool zeroOnAllocate, MOS_COMPONENT component);

    //!
    //! \brief  Allocate a resource for a surface, so far vp does not use the mos_surface.
    //! \res    [in] param
    //!         pointer to MOS_RESOURCE
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, otherwise failed reason
    //!
    MOS_STATUS AllocateResource(MOS_RESOURCE *res, MOS_ALLOC_GFXRES_PARAMS &param);

    //!
    //! \brief  Destroy Resource
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyResource(MOS_RESOURCE *resource);

    //!
    //! \brief  Destroy Buffer
    //! \param  [in] buffer
    //!         Pointer to MOS_BUFFER
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyBuffer(MOS_BUFFER *buffer);

    //!
    //! \brief  Destroy Surface
    //! \param  [in] surface
    //!         Pointer to MOS_SURFACE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroySurface(MOS_SURFACE *surface);

    //!
    //! \brief  Free a resource
    //! \param  [in] res
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeResource(MOS_RESOURCE *res);

    //!
    //! \brief  Lock Surface
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \param  [in] lockFlag
    //!         Pointer to MOS_LOCK_PARAMS
    //! \return void*
    //!         a poniter to data
    //!
    void *Lock(MOS_RESOURCE *resource, MOS_LOCK_PARAMS *lockFlag);

    //!
    //! \brief  UnLock Surface
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
    MOS_STATUS SkipResourceSync(MOS_RESOURCE *resource);

    //!
    //! \brief  get surface info from resource
    //! \param  [in] osResource
    //!         Pointer to PMOS_RESOURCE
    //! \param  [out] resDetails
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetSurfaceInfo(PMOS_RESOURCE osResource, PMOS_SURFACE resDetails);

    //!
    //! \brief    OS fill Resource
    //! \details  Locks the surface and fills the resource with data
    //! \param    PMOS_RESOURCE osResource
    //!           [in] Pointer to OS Resource
    //! \param    uint32_t size
    //!           [in] Size of the Buffer
    //! \param    uint8_t value
    //!           [in] Value to be filled
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS OsFillResource(PMOS_RESOURCE osResource, uint32_t  size, uint8_t value);

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

    //!
    //! \brief  Clear Resource
    //! \param  [in] resource
    //!         pointer to MOS_RESOURCE
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ClearResource(MOS_RESOURCE *resource, MOS_ALLOC_GFXRES_PARAMS &param);

#if (_DEBUG || _RELEASE_INTERNAL)
    struct TraceInfo
    {
        MOS_COMPONENT component;
        std::string   name;
    };

    std::map<MOS_RESOURCE *, TraceInfo *> m_resourcePool;
    std::map<MOS_SURFACE *, TraceInfo *>  m_surfacePool;
#else
    std::vector<MOS_RESOURCE *> m_resourcePool;
    std::vector<MOS_SURFACE *>  m_surfacePool;
#endif

    PMOS_INTERFACE m_osInterface = nullptr;  //!< PMOS_INTERFACE
};
#endif  // !__MEDIA_ALLOCATOR_H__
