/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_surface_manager.h
//! \brief     Contains Class CmSurfaceManager  definitions
//!

#ifndef MEDIADRIVER_LINUX_COMMON_CM_CMSURFACEMANAGER_H_
#define MEDIADRIVER_LINUX_COMMON_CM_CMSURFACEMANAGER_H_

#include "cm_def.h"
#include "cm_hal.h"
typedef enum _MOS_FORMAT MOS_FORMAT;

namespace CMRT_UMD
{
class CmDeviceRT;
class CmSurface;
class CmBuffer_RT;
class CmSurfaceVme;
class CmSurfaceSampler8x8;
class CmSurfaceSampler;
class CmSurface2DUPRT;
class CmSurface2DRT;
class CmSurface3DRT;
class CmStateBuffer;
class CmKernelRT;

class CmSurfaceManager
{
public:
    static int32_t Create(
        CmDeviceRT* device,
        CM_HAL_MAX_VALUES halMaxValues,
        CM_HAL_MAX_VALUES_EX lalMaxValuesEx,
        CmSurfaceManager* &manager );

    static int32_t Destroy( CmSurfaceManager* &manager );

    int32_t CreateBuffer(uint32_t size, CM_BUFFER_TYPE type, bool svmAllocatedByCm, CmBuffer_RT* &buffer, MOS_RESOURCE *mosResource, void* &sysMem, bool isConditionalBuffer, uint32_t comparisonValue );
    int32_t DestroySurface( CmBuffer_RT* &buffer, SURFACE_DESTROY_KIND destroyKind);
    int32_t UpdateBuffer(MOS_RESOURCE * mosResource, int index, uint32_t handle);

    int32_t CreateSurface2DUP(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, void* sysMem, CmSurface2DUPRT* &surface);
    int32_t DestroySurface( CmSurface2DUPRT* &surface2dUP,  SURFACE_DESTROY_KIND destroyKind);

    int32_t CreateSurface2D(uint32_t width, uint32_t height, uint32_t pitch, bool createdByCm, CM_SURFACE_FORMAT format, CmSurface2DRT* &surface);
    int32_t CreateSurface2D(MOS_RESOURCE * mosResource, bool createdByCm, CmSurface2DRT* &surface);
    int32_t UpdateSurface2D(MOS_RESOURCE * mosResource, int index, uint32_t handle);

    int32_t DestroySurface( CmSurface2DRT* &surface2d, SURFACE_DESTROY_KIND destroyKind);
    int32_t CreateSurface3D(uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format, CmSurface3DRT* &surface);
    int32_t DestroySurface( CmSurface3DRT* &surface3d, SURFACE_DESTROY_KIND destroyKind);

    int32_t CreateVmeSurface( CmSurface2DRT* currentSurface,CmSurface2DRT** forwardSurfaceArray,
                          CmSurface2DRT** backwardSurfaceArray,const uint32_t surfaceCountForward,
                          const uint32_t surfaceCountBackward, SurfaceIndex* & vmeSurfaceIndex );
    int32_t DestroyVmeSurface( SurfaceIndex* & vmeSurfaceIndex );
    int32_t DestroySurface( CmSurfaceVme* &vmeSurface);

    int32_t CreateSampler8x8Surface(CmSurface2DRT* currentSurface, SurfaceIndex* & sampler8x8SurfaceIndex, CM_SAMPLER8x8_SURFACE sampler8x8Type, CM_SURFACE_ADDRESS_CONTROL_MODE addressControl, CM_FLAG* flag);
    int32_t DestroySampler8x8Surface(SurfaceIndex* &sampler8x8SurfaceIndex );
    int32_t DestroySurface( CmSurfaceSampler8x8* &sampler8x8Surface);

    int32_t GetSurface( const uint32_t index, CmSurface* &surface);
    int32_t GetCmDevice( CmDeviceRT* &device);

    int32_t GetPixelBytesAndHeight(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, uint32_t& sizePerPixel, uint32_t& updatedHeight);
    int32_t Surface2DSanityCheck(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format);

    int32_t CreateSamplerSurface(CmSurface2DRT* currentSurface2d, SurfaceIndex* &samplerSurfaceIndex, CM_FLAG* flag);
    int32_t CreateSamplerSurface(CmSurface2DUPRT* currentSurface2dUP, SurfaceIndex* &samplerSurfaceIndex);
    int32_t CreateSamplerSurface(CmSurface3DRT* currentSurface3d, SurfaceIndex* &samplerSurfaceIndex);
    int32_t DestroySamplerSurface(SurfaceIndex* &samplerSurfaceIndex);
    int32_t DestroySurface( CmSurfaceSampler* &samplerSurface);
    uint32_t GetSurfacePoolSize();
    int32_t IncreaseSurfaceUsage(uint32_t index);
    int32_t DecreaseSurfaceUsage(uint32_t index);
    int32_t RefreshDelayDestroySurfaces(uint32_t &freeSurfaceCount);
    int32_t TouchSurfaceInPoolForDestroy();
    int32_t GetFreeSurfaceIndexFromPool(uint32_t &freeIndex);
    int32_t GetFreeSurfaceIndex(uint32_t &index);

    int32_t AllocateSurfaceIndex(uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format, uint32_t &index, void *sysMem);

    int32_t DestroySurfaceArrayElement( uint32_t index );
    inline int32_t GetMemorySizeOfSurfaces();

    int32_t GetSurfaceArraySize(uint32_t& surfaceArraySize);

    int32_t UpdateStateForDelayedDestroy(SURFACE_DESTROY_KIND destroyKind, uint32_t index);

    int32_t UpdateStateForRealDestroy(uint32_t index, CM_ENUM_CLASS_TYPE surfaceType);
    int32_t UpdateProfileFor2DSurface(uint32_t index, uint32_t width, uint32_t height, CM_SURFACE_FORMAT format);
    int32_t UpdateProfileFor1DSurface(uint32_t index, uint32_t size);
    int32_t UpdateProfileFor3DSurface(uint32_t index, uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format);

    int32_t UpdateSurface2DTableMosResource(uint32_t index, MOS_RESOURCE * mosResource);
    int32_t UpdateSurface2DTableRotation(uint32_t index, CM_ROTATION rotationFlag);

    int32_t UpdateSurface2DTableFrameType(uint32_t index, CM_FRAME_TYPE frameType);
    int32_t UpdateSurface2DTableChromaSiting(uint32_t index, int32_t chromaSiting);

    int32_t CreateStateBuffer( CM_STATE_BUFFER_TYPE stateBufferType, uint32_t size, void  *mediaState, CmKernelRT *kernel, CmStateBuffer *&buffer);
    int32_t DestroyStateBuffer( CmStateBuffer* &buffer, SURFACE_DESTROY_KIND destroyKind );

    int32_t CreateMediaStateByCurbeSize( void  *&mediaState, uint32_t curbeSize );
    int32_t DestroyMediaState( void  *mediaState );

    bool IsCmReservedSurfaceIndex(uint32_t surfaceBTI);
    bool IsValidSurfaceIndex(uint32_t surfaceBTI);
    uint32_t MaxIndirectSurfaceCount();
    uint32_t ValidSurfaceIndexStart();
    bool IsSupportedForSamplerSurface2D(CM_SURFACE_FORMAT format);

    inline void SetLatestRenderTrackerAddr(uint32_t *tracker) {m_latestRenderTracker = tracker; }
    inline void SetLatestFastTrackerAddr(uint32_t *tracker) {m_latestFastTracker = tracker; }
    inline void SetLatestVeboxTrackerAddr(uint32_t *tracker) {m_latestVeboxTracker = tracker; }
    inline uint32_t LatestRenderTracker() {return (m_latestRenderTracker == nullptr)?0:*m_latestRenderTracker; }
    inline uint32_t LatestFastTracker() {return (m_latestFastTracker == nullptr)?0:*m_latestFastTracker; }
    inline uint32_t LatestVeboxTracker() {return (m_latestVeboxTracker == nullptr)?0:*m_latestVeboxTracker; }

    void AddToDelayDestroyList(CmSurface *surface);
    void RemoveFromDelayDestroyList(CmSurface *surface);
protected:
    CmSurfaceManager(CmDeviceRT* device);
    CmSurfaceManager();

    ~CmSurfaceManager( void );
    int32_t Initialize( CM_HAL_MAX_VALUES halMaxValues, CM_HAL_MAX_VALUES_EX halMaxValuesEx);

    int32_t AllocateBuffer(uint32_t size, CM_BUFFER_TYPE type, uint32_t & handle, MOS_RESOURCE * mosResource, void* sysMem = nullptr ); // Create the internal buffer of surface1D in driver
    int32_t FreeBuffer( uint32_t handle ); // Destroy the internal buffer of surface1D in driver

    int32_t AllocateSurface2DUP(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, void* sysMem, uint32_t & handle );
    int32_t FreeSurface2DUP( uint32_t handle );

    int32_t AllocateSurface2D(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, uint32_t & handle, uint32_t & pitch );
    int32_t AllocateSurface2D(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format, MOS_RESOURCE * mosResource, uint32_t & handle);

    int32_t FreeSurface2D( uint32_t handle );

    int32_t Allocate3DSurface(uint32_t width, uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format, uint32_t & handle );
    int32_t Free3DSurface( uint32_t handle );

    int32_t GetFormatSize(CM_SURFACE_FORMAT format,uint32_t &sizePerPixel);

    int32_t GetSurfaceInfo( MOS_RESOURCE *mosResource, uint32_t &width, uint32_t &height, uint32_t &pitch, CM_SURFACE_FORMAT &format);

    int32_t RemoveUserDataEntryIfNeeded(CmSurface2DRT *surface) { return 0; }
    int32_t GetSurfaceBTIInfo();

public:
    static const uint32_t MAX_DEVICE_FOR_SAME_SURF = 64; // mamimum number of cm device allowed for creating a cm surf2d wrapper for a mos resource

protected:
    CmDeviceRT* m_device;

    uint32_t m_surfaceArraySize; // if VPHAL is not defined, it should be CM_MAX_ALL_SURFACE_COUNT

    CmSurface** m_surfaceArray;
    uint32_t m_maxSurfaceIndexAllocated; // the max index allocated in the m_surfaceArray

    int32_t *m_surfaceSizes;         // Size of each surface in surface array

    uint32_t m_maxBufferCount;
    uint32_t m_bufferCount;

    uint32_t m_max2DSurfaceCount;
    uint32_t m_2DSurfaceCount;

    uint32_t m_max3DSurfaceCount;
    uint32_t m_3DSurfaceCount;

    uint32_t m_max2DUPSurfaceCount;
    uint32_t m_2DUPSurfaceCount;

    uint32_t m_bufferAllCount;
    uint32_t m_2DSurfaceAllCount;
    uint32_t m_3DSurfaceAllCount;

    uint32_t m_bufferAllSize;
    uint32_t m_2DSurfaceAllSize;
    uint32_t m_3DSurfaceAllSize;

    uint32_t m_garbageCollectionTriggerTimes;
    uint32_t m_garbageCollection1DSize;
    uint32_t m_garbageCollection2DSize;
    uint32_t m_garbageCollection3DSize;

    CM_SURFACE_BTI_INFO m_surfaceBTIInfo;

    uint32_t *m_latestRenderTracker;
    uint32_t *m_latestFastTracker;
    uint32_t *m_latestVeboxTracker;

    CmSurface *m_delayDestroyHead;
    CmSurface *m_delayDestroyTail;
    CSync m_delayDestoryListSync;

private:
    CmSurfaceManager (const CmSurfaceManager& other);
    CmSurfaceManager& operator= (const CmSurfaceManager& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMSURFACEMANAGER_H_
