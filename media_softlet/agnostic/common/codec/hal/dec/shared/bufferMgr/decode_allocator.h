/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     decode_allocator.h
//! \brief    Defines the interface for decode resource allocate
//! \details  decode allocator will allocate and destory buffers, the caller
//!           can use directly
//!

#ifndef __DECODE_ALLOCATOR_H__
#define __DECODE_ALLOCATOR_H__

#include "media_class_trace.h"
#include "mhw_utilities_next.h"
#include "mos_defs.h"
#include "mos_os.h"
#include "mos_os_hw.h"
#include "mos_os_specific.h"
#include "mos_resource_defs.h"
#include "External/Common/GmmCachePolicyExt.h"
#include <stdint.h>
#include <vector>
class Allocator;

namespace decode {

template <class T>
class ResourceArray;

using BufferArray       = ResourceArray<MOS_BUFFER>;
using SurfaceArray      = ResourceArray<MOS_SURFACE>;
using BatchBufferArray  = ResourceArray<MHW_BATCH_BUFFER>;

enum ResourceUsage
{
    resourceOutputPicture            = MOS_HW_RESOURCE_USAGE_DECODE_OUTPUT_PICTURE,
    resourceInputBitstream           = MOS_HW_RESOURCE_USAGE_DECODE_INPUT_BITSTREAM,
    resourceInputReference           = MOS_HW_RESOURCE_USAGE_DECODE_INPUT_REFERENCE,
    resourceInternalRead             = MOS_HW_RESOURCE_USAGE_DECODE_INTERNAL_READ,
    resourceInternalWrite            = MOS_HW_RESOURCE_USAGE_DECODE_INTERNAL_WRITE,
    resourceInternalReadWriteCache   = MOS_HW_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_CACHE,
    resourceInternalReadWriteNoCache = MOS_HW_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE,
    resourceStatisticsWrite          = MOS_HW_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_WRITE,
    resourceStatisticsReadWrite      = MOS_HW_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_READ_WRITE,
    resourceDefault                  = MOS_HW_RESOURCE_DEF_MAX
};

enum ResourceAccessReq
{
    notLockableVideoMem = 0,
    lockableVideoMem,
    lockableSystemMem
};

class DecodeAllocator
{
public:
    //!
    //! \brief  Constructor
    //! \param  [in] osInterface
    //!         Pointer to MOS_INTERFACE
    //! \param  [in] limitedLMemBar
    //!         Indicate if running with limited LMem bar config
    //!
    DecodeAllocator(PMOS_INTERFACE osInterface, bool limitedLMemBar);

    //!
    //! \brief  DecodeAllocator destructor
    //!
    ~DecodeAllocator();

    //!
    //! \brief  Allocate buffer
    //! \param  [in] sizeOfBuffer
    //!         Buffer size
    //! \param  [in] nameOfBuffer
    //!         Buffer name
    //! \param  [in] resUsageType
    //!         ResourceUsage to be set
    //! \param  [in] accessReq
    //!         Resource access requirement, by default is lockable
    //! \param  [in] initOnAllocate
    //!         Indicate if this buffer need to be initialized when allocate, by default is false
    //! \param  [in] initValue
    //!         Initialization value when initOnAllocate flag is true, by default is 0
    //! \param  [in] bPersistent
    //!         persistent flag
    //! \return MOS_BUFFER*
    //!         return the pointer to MOS_BUFFER
    //!
    MOS_BUFFER* AllocateBuffer(const uint32_t sizeOfBuffer, const char* nameOfBuffer,
        ResourceUsage resUsageType = resourceDefault, ResourceAccessReq accessReq = lockableVideoMem,
        bool initOnAllocate = false, uint8_t initValue = 0, bool bPersistent = false);

    //!
    //! \brief  Allocate buffer array
    //! \param  [in] sizeOfBuffer
    //!         Buffer size
    //! \param  [in] nameOfBuffer
    //!         Buffer name
    //! \param  [in] numberOfBuffer
    //!         number of buffer array
    //! \param  [in] resUsageType
    //!         ResourceUsage to be set
    //! \param  [in] accessReq
    //!         Resource access requirement, by default is lockable
    //! \param  [in] initOnAllocate
    //!         Indicate if this buffer need to be initialized when allocate, by default is false
    //! \param  [in] initValue
    //!         Initialization value when initOnAllocate flag is true, by default is 0
    //! \return BufferArray*
    //!         return the pointer to BufferArray
    //!
    BufferArray * AllocateBufferArray(
        const uint32_t sizeOfBuffer, const char* nameOfBuffer, const uint32_t numberOfBuffer,
        ResourceUsage resUsageType = resourceDefault, ResourceAccessReq accessReq = lockableVideoMem,
        bool initOnAllocate = false, uint8_t initValue = 0, bool bPersistent = false);

    //!
    //! \brief  Allocate Surface
    //! \param  [in] width
    //!         Surface width
    //! \param  [in] height
    //!         Surface height
    //! \param  [in] name
    //!         Surface name
    //! \param  [in] format
    //!         Surface format, by default is NV12
    //! \param  [in] isCompressible
    //!         Compressible flag, by default is false
    //! \param  [in] resUsageType
    //!         ResourceUsage to be set
    //! \param  [in] accessReq
    //!         Resource access requirement, by default is lockable
    //! \param  [in] gmmTileMode
    //!         Specified GMM tile mode
    //! \return MOS_SURFACE*
    //!         return the pointer to MOS_SURFACE
    //!
    MOS_SURFACE* AllocateSurface(
        const uint32_t width, const uint32_t height, const char* nameOfSurface,
        MOS_FORMAT format = Format_NV12, bool isCompressible = false, 
        ResourceUsage resUsageType = resourceDefault, ResourceAccessReq accessReq = lockableVideoMem,
        MOS_TILE_MODE_GMM gmmTileMode = MOS_TILE_UNSET_GMM);

    //!
    //! \brief  Allocate surface array
    //! \param  [in] width
    //!         Surface width
    //! \param  [in] height
    //!         Surface height
    //! \param  [in] name
    //!         Surface name
    //! \param  [in] numberOfSurface
    //!         Number of surface array
    //! \param  [in] format
    //!         Surface format, by default is NV12
    //! \param  [in] isCompressed
    //!         Compress flag, by default is false
    //! \param  [in] resUsageType
    //!         ResourceUsage to be set
    //! \param  [in] accessReq
    //!         Resource access requirement, by default is lockable
    //! \return SurfaceArray*
    //!         return the pointer to SurfaceArray
    //!
    SurfaceArray * AllocateSurfaceArray(
        const uint32_t width, const uint32_t height, const char* nameOfSurface,
        const uint32_t numberOfSurface, MOS_FORMAT format = Format_NV12, bool isCompressed = false,
        ResourceUsage resUsageType = resourceDefault, ResourceAccessReq accessReq = lockableVideoMem);

    //!
    //! \brief  Allocate Linear Output Surface
    //! \param  [in] width
    //!         surface width
    //! \param  [in] height
    //!         surface height
    //! \param  [in] surfaceName
    //!         Surface name
    //! \param  [in] format
    //!         Surface format, by default is NV12
    //! \param  [in] compressible
    //!         Compressible flag, by default is false
    //! \param  [in] resUsageType
    //!         ResourceUsage to be set
    //! \param  [in] gmmTileMode
    //!         Specified GMM tile mode
    //!
    MOS_SURFACE * AllocateLinearSurface(
        const uint32_t width, const uint32_t height, const char *nameOfSurface,
        MOS_FORMAT format = Format_NV12, bool isCompressible = false,
        ResourceUsage resUsageType = resourceDefault, ResourceAccessReq accessReq = lockableVideoMem,
        MOS_TILE_MODE_GMM gmmTileMode = MOS_TILE_UNSET_GMM);

    //!
    //! \brief  Allocate batch buffer
    //! \param  [in] sizeOfBuffer
    //!         Batch buffer size
    //! \param  [in] numOfBuffer
    //!         Surface height
    //! \param  [in] accessReq
    //!         Resource access requirement, by default is lockable
    //! \return PMHW_BATCH_BUFFER
    //!         return the pointer to allocated batch buffer if success, else nullptr
    //!
    PMHW_BATCH_BUFFER AllocateBatchBuffer(const uint32_t sizeOfBuffer, const uint32_t numOfBuffer=1,
        ResourceAccessReq accessReq = lockableVideoMem);

    //!
    //! \brief  Allocate batch buffer array
    //! \param  [in] sizeOfSubBuffer
    //!         Size of sub buffer
    //! \param  [in] numOfSubBuffer
    //!         Number of sub buffer
    //! \param  [in] numberOfBatchBuffer
    //!         Number of batch buffer array
    //! \param  [in] secondLevel
    //!         Flag to indicate second level batch buffer
    //! \param  [in] accessReq
    //!         Resource access requirement, by default is lockable
    //! \return BatchBufferArray*
    //!         return the pointer to BatchBufferArray
    //!
    BatchBufferArray * AllocateBatchBufferArray(
        const uint32_t sizeOfSubBuffer, const uint32_t numOfSubBuffer,
        const uint32_t numberOfBatchBuffer, bool secondLevel,
        ResourceAccessReq accessReq = lockableVideoMem);

    //!
    //! \brief  Resize linear buffer
    //! \param  [in/out] buffer
    //!         The pointer of linear buffer
    //! \param  [in] sizeNew
    //!         New size for linear buffer
    //! \param  [in] accessReq
    //!         Resource access requirement, by default is lockable
    //! \param  [in] force
    //!         Flag indicates whether resize buffer by force when size changed
    //! \param  [in] clearData
    //!         Flag indicates whether clear cached buffer data when size changed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Resize(MOS_BUFFER* &buffer, const uint32_t sizeNew, ResourceAccessReq accessReq = lockableVideoMem,
        bool force = false, bool clearData = false);

    //!
    //! \brief  Resize surface
    //! \param  [in/out] surface
    //!         The pointer of surface
    //! \param  [in] widthNew
    //!         New width
    //! \param  [in] heightNew
    //!         New height
    //! \param  [in] accessReq
    //!         Resource access requirement, by default is lockable
    //! \param  [in] force
    //!         Flag indicates whether resize surface by force when size changed
    //! \param  [in] nameOfSurface
    //!         Name of the surface
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Resize(MOS_SURFACE* &surface, const uint32_t widthNew, const uint32_t heightNew,
        ResourceAccessReq accessReq = lockableVideoMem, bool force = false, const char* nameOfSurface = "");

    //!
    //! \brief  Resize batch buffer
    //! \param  [in] sizeOfBufferNew
    //!         Size of new batch buffer
    //! \param  [in] numOfBufferNew
    //!         Number of new batch buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Resize(PMHW_BATCH_BUFFER &batchBuffer, const uint32_t sizeOfBufferNew, const uint32_t numOfBufferNew,
        ResourceAccessReq accessReq = lockableVideoMem);

    //!
    //! \brief  Destroy buffer
    //! \param  [in] resource
    //!         The buffer to be destroyed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy(MOS_BUFFER* & resource);

    //!
    //! \brief  Destroy Surface
    //! \param  [in] surface
    //!         The surface to be destroyed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy(MOS_SURFACE* & surface);

    //!
    //! \brief  Destroy Surface
    //! \param  [in] surface
    //!         The surface to be destroyed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy(MOS_SURFACE& surface);

    //!
    //! \brief  Destroy buffer array
    //! \param  [in] bufferArray
    //!         The buffer array to be destroyed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy(BufferArray* & bufferArray);

    //!
    //! \brief  Destroy surface array
    //! \param  [in] surfaceArray
    //!         The surface array to be destroyed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy(SurfaceArray * & surfaceArray);

    //!
    //! \brief  Destroy batch buffer
    //! \param  [in] batchBuffer
    //!         The batch buffer to be destroyed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy(PMHW_BATCH_BUFFER &batchBuffer);

    //!
    //! \brief  Destroy batch buffer array
    //! \param  [in] batchBufferArray
    //!         The batch buffer array to be destroyed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy(BatchBufferArray * & batchBufferArray);

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
    //! \brief  Lock resource only for reading
    //! \param  [in] buffer
    //!         Pointer to MOS_BUFFER
    //! \return void*
    //!         a poniter to data
    //!
    void* LockResourceForRead(MOS_BUFFER* buffer);

    //!
    //! \brief  Lock batch buffer
    //! \param  [in] batchBuffer
    //!         Pointer to MHW_BATCH_BUFFER
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Lock(PMHW_BATCH_BUFFER batchBuffer);

    //!
    //! \brief  UnLock resource
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UnLock(MOS_RESOURCE* resource);

    //!
    //! \brief  UnLock buffer
    //! \param  [in] buffer
    //!         Pointer to MOS_BUFFER
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UnLock(MOS_BUFFER* buffer);

    //!
    //! \brief  UnLock batch buffer
    //! \param  [in] batchBuffer
    //!         Pointer to MHW_BATCH_BUFFER
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UnLock(PMHW_BATCH_BUFFER batchBuffer, bool resetBuffer);

    MOS_STATUS DestroyAllResources();

    //!
    //! \brief  Sync on resource
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \param  [in] IsWriteOperation
    //!         Indicate if sync for write operation
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SyncOnResource(MOS_RESOURCE* resource, bool IsWriteOperation);

    //!
    //! \brief  Sync on resource
    //! \param  [in] buffer
    //!         Pointer to MOS_BUFFER
    //! \param  [in] IsWriteOperation
    //!         Indicate if sync for write operation
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SyncOnResource(MOS_BUFFER* buffer, bool IsWriteOperation);

    //!
    //! \brief  Skip sync resource
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SkipResourceSync(MOS_RESOURCE* resource);

    //!
    //! \brief  Skip sync resource
    //! \param  [in] buffer
    //!         Pointer to MOS_BUFFER
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SkipResourceSync(MOS_BUFFER* buffer);

    //!
    //! \brief  Check if resource is null
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \return BOOL
    //!         Return TRUE for nullptr resource, otherwise FALSE
    //!
    bool ResourceIsNull(MOS_RESOURCE* resource);

    //!
    //! \brief  get surface info from resource
    //! \param  [in, out] surface
    //!         Pointer to MOS_RESOURCE
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetSurfaceInfo(PMOS_SURFACE surface);

    //!
    //! \brief    Update the GMM usage type of resource for cache policy
    //! \details  Update the GMM usage type of resource for cache policy
    //! \param    PMOS_RESOURCE OsResource
    //!           [in] OS resource sturcture
    //! \param    [in] resUsageType
    //!           ResourceUsage to be set
    //! \return   VOID
    //!
    MOS_STATUS UpdateResoreceUsageType(
        PMOS_RESOURCE osResource,
        ResourceUsage resUsageType);

    //!
    //! \brief    Registers Resource
    //! \details  Get the Allocation Index from UMD Context and set into OS
    //!           resource structure
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] Pointer to OS Interface
    //! \param    PMOS_RESOURCE pOsResource
    //!           [in/out] Pointer to OS Resource
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS RegisterResource(PMOS_RESOURCE osResource);

    //!
    //! \brief    Convert from GMM usage type of resource to MOS_HW_RESOURCE_DEF
    //! \details  Convert from GMM usage type of resource to MOS_HW_RESOURCE_DEF
    //! \param    GMM_RESOURCE_USAGE_TYPE gmmResUsage
    //!           [in] GMM usage type of resource
    //! \return   ResourceUsage
    //!
    ResourceUsage ConvertGmmResourceUsage(const GMM_RESOURCE_USAGE_TYPE gmmResUsage);

protected:
    //!
    //! \brief    Apply resource access requirement to allocate parameters
    //! \details  Apply resource access requirement to allocate parameters
    //! \param    ResourceAccessReq accessReq
    //!           [in] Resource access requirement
    //! \param    MOS_ALLOC_GFXRES_PARAMS allocParams
    //!           [out] allocation parameters
    //! \return   void
    //!
    void SetAccessRequirement(ResourceAccessReq accessReq, MOS_ALLOC_GFXRES_PARAMS &allocParams);

    PMOS_INTERFACE m_osInterface = nullptr;  //!< PMOS_INTERFACE
    Allocator *m_allocator = nullptr;
    bool m_limitedLMemBar = false; //!< Indicate if running with limited LMem bar config

#if (_DEBUG || _RELEASE_INTERNAL)
    bool m_forceLockable = false;
#endif

MEDIA_CLASS_DEFINE_END(decode__DecodeAllocator)
};
}
#endif // !__DECODE_ALLOCATOR_H__
