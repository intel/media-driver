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
//! \file     decode_allocator.h
//! \brief    Defines the interface for decode resouce allocate
//! \details  decode allocator will allocate and destory buffers, the caller
//!           can use directly
//!

#ifndef __DECODE_ALLOCATOR_H__
#define __DECODE_ALLOCATOR_H__

#include "media_allocator.h"
#include "mhw_utilities.h"

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

class DecodeAllocator
{
public:
    //!
    //! \brief  Constructor
    //! \param  [in] osInterface
    //!         Pointer to MOS_INTERFACE
    //!
    DecodeAllocator(PMOS_INTERFACE osInterface);

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
        ResourceUsage resUsageType = resourceDefault, bool initOnAllocate = false, uint8_t initValue = 0, bool bPersistent = false);

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
    //! \param  [in] initOnAllocate
    //!         Indicate if this buffer need to be initialized when allocate, by default is false
    //! \param  [in] initValue
    //!         Initialization value when initOnAllocate flag is true, by default is 0
    //! \return BufferArray*
    //!         return the pointer to BufferArray
    //!
    BufferArray * AllocateBufferArray(
        const uint32_t sizeOfBuffer, const char* nameOfBuffer, const uint32_t numberOfBuffer,
        ResourceUsage resUsageType = resourceDefault, bool initOnAllocate = false, uint8_t initValue = 0, bool bPersistent = false);

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
    //! \return MOS_SURFACE*
    //!         return the pointer to MOS_SURFACE
    //!
    MOS_SURFACE* AllocateSurface(
        const uint32_t width, const uint32_t height, const char* nameOfSurface, MOS_FORMAT format = Format_NV12,
        bool isCompressible = false, ResourceUsage resUsageType = resourceDefault);

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
    //! \return SurfaceArray*
    //!         return the pointer to SurfaceArray
    //!
    SurfaceArray * AllocateSurfaceArray(
        const uint32_t width, const uint32_t height, const char* nameOfSurface,
        const uint32_t numberOfSurface, MOS_FORMAT format = Format_NV12, bool isCompressed = false, ResourceUsage resUsageType = resourceDefault);

    //!
    //! \brief  Allocate batch buffer
    //! \param  [in] sizeOfBuffer
    //!         Batch buffer size
    //! \param  [in] numOfBuffer
    //!         Surface height
    //! \return PMHW_BATCH_BUFFER
    //!         return the pointer to allocated batch buffer if success, else nullptr
    //!
    PMHW_BATCH_BUFFER AllocateBatchBuffer(const uint32_t sizeOfBuffer, const uint32_t numOfBuffer=1);

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
    //! \return BatchBufferArray*
    //!         return the pointer to BatchBufferArray
    //!
    BatchBufferArray * AllocateBatchBufferArray(
        const uint32_t sizeOfSubBuffer, const uint32_t numOfSubBuffer,
        const uint32_t numberOfBatchBuffer, bool secondLevel);

    //!
    //! \brief  Resize linear buffer
    //! \param  [in] sizeNew
    //!         New size for linear buffer
    //! \param  [in] force
    //!         Flag indicates whether resize buffer by force when size changed
    //! \param  [in] clearData
    //!         Flag indicates whether clear cached buffer data when size changed
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Resize(MOS_BUFFER* &buffer, const uint32_t sizeNew, bool force = false, bool clearData = false);

    //!
    //! \brief  Resize surface
    //! \param  [in] widthNew
    //!         New width
    //! \param  [in] heightNew
    //!         New height
    //! \param  [in] force
    //!         Flag indicates whether resize surface by force when size changed
    //! \param  [in] nameOfSurface
    //!         Name of the surface
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Resize(MOS_SURFACE* &surface, const uint32_t widthNew, const uint32_t heightNew,
                      bool force = false, const char* nameOfSurface = "");

    //!
    //! \brief  Resize batch buffer
    //! \param  [in] sizeOfBufferNew
    //!         Size of new batch buffer
    //! \param  [in] numOfBufferNew
    //!         Number of new batch buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Resize(PMHW_BATCH_BUFFER &batchBuffer, 
                      const uint32_t sizeOfBufferNew, const uint32_t numOfBufferNew);

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
    //! \brief  Lock resource only for reading
    //! \param  [in] buffer
    //!         Pointer to MOS_BUFFER
    //! \return void*
    //!         a poniter to data
    //!
    void* LockResouceForRead(MOS_BUFFER* buffer);

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
    //! \brief    Convert from GMM usage type of resource to MOS_HW_RESOURCE_DEF
    //! \details  Convert from GMM usage type of resource to MOS_HW_RESOURCE_DEF
    //! \param    GMM_RESOURCE_USAGE_TYPE gmmResUsage
    //!           [in] GMM usage type of resource
    //! \return   ResourceUsage
    //!
    ResourceUsage ConvertGmmResourceUsage(const GMM_RESOURCE_USAGE_TYPE gmmResUsage);

protected:
    PMOS_INTERFACE m_osInterface = nullptr;  //!< PMOS_INTERFACE
    Allocator *m_allocator = nullptr;

};
}
#endif // !__DECODE_ALLOCATOR_H__
