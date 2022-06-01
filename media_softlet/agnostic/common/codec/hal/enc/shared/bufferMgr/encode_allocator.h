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
//! \file     encode_allocator.h
//! \brief    Defines the interface for encode resource allocate
//! \details  encode allocator will allocate and destory buffers, the caller
//!           can use directly
//!

#ifndef __ENCODE_ALLOCATOR_H__
#define __ENCODE_ALLOCATOR_H__

#include "media_class_trace.h"
#include "mos_defs.h"
#include "mos_os.h"
#include "mos_os_hw.h"
#include "mos_os_specific.h"
class Allocator;

namespace encode {

class EncodeAllocator
{
public:
    //!
    //! \brief  Constructor
    //! \param  [in] osInterface
    //!         Pointer to MOS_INTERFACE
    //!
    EncodeAllocator(PMOS_INTERFACE osInterface);

    //!
    //! \brief  EncodeAllocator Destructor
    //!
    virtual ~EncodeAllocator();

    //!
    //! \brief  Allocate Resource
    //! \param  [in] component
    //!         component type to track the buffer
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] zeroOnAllocate
    //!         zero the memory if true
    //! \param  [in] resUsageType
    //!         resource usage for cache policy
    //! \return MOS_RESOURCE*
    //!         return the pointer to MOS_RESOURCE
    //!
    MOS_RESOURCE* AllocateResource(
        MOS_ALLOC_GFXRES_PARAMS &param,
        bool zeroOnAllocate,
        MOS_HW_RESOURCE_DEF resUsageType = MOS_HW_RESOURCE_DEF_MAX);

    //!
    //! \brief  Allocate Surface
    //! \param  [in] component
    //!         component type to track the buffer
    //! \param  [in] param
    //!         reference to MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] zeroOnAllocate
    //!         zero the memory if true
    //! \param  [in] resUsageType
    //!         resource usage for cache policy
    //! \return MOS_SURFACE*
    //!         return the pointer to MOS_SURFACE
    //!
    virtual MOS_SURFACE* AllocateSurface(
        MOS_ALLOC_GFXRES_PARAMS &param,
        bool zeroOnAllocate,
        MOS_HW_RESOURCE_DEF resUsageType = MOS_HW_RESOURCE_DEF_MAX);

    //!
    //! \brief  Destroy Resource
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyResource(MOS_RESOURCE* resource);

    MOS_STATUS DestroyAllResources();

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
    void* Lock(MOS_RESOURCE* resource, MOS_LOCK_PARAMS* lockFlag);

    //!
    //! \brief  Lock resource only for writing
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return void*
    //!         a poniter to data
    //!
    virtual void* LockResourceForWrite(MOS_RESOURCE *resource);

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
    virtual void* LockResourceForRead(MOS_RESOURCE *resource);

    //!
    //! \brief  UnLock resource
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UnLock(MOS_RESOURCE* resource);

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
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetSurfaceInfo(PMOS_SURFACE surface);

    //!
    //! \brief    Update the GMM usage type of resource for cache policy
    //! \details  Update the GMM usage type of resource for cache policy
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
    //! \brief    Tag based synchronization at the resource level
    //! \details  Tag based synchronization at the resource level
    //! \param    PMOS_RESOURCE OsResource
    //!           [in] OS resource sturcture
    //! \param    BOOL bWriteOperation
    //!           [in] Indicate if it is a write operation
    //! \return   VOID
    //!
    MOS_STATUS SyncOnResource(
        PMOS_RESOURCE osResource,
        bool          bWriteOperation);

protected:
    PMOS_INTERFACE m_osInterface = nullptr;  //!< PMOS_INTERFACE
    Allocator *m_allocator = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodeAllocator)
};
}
#endif // !__ENCODE_ALLOCATOR_H__
