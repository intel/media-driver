/*===================== begin_copyright_notice ==================================
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
//! \file      cm_surface_manager_base.h
//! \brief     Contains Class CmSurfaceManagerBase definitions
//!

#ifndef MEDIADRIVER_COMMON_CM_CMSURFACEMANAGER_BASE_H_
#define MEDIADRIVER_COMMON_CM_CMSURFACEMANAGER_BASE_H_

#include "cm_def.h"
#include "cm_hal.h"
#include <set>

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

class CmSurfaceManagerBase
{
public:
    int32_t CreateSurface2D(uint32_t width, uint32_t height, uint32_t pitch,
                            bool bIsCmCreated, CM_SURFACE_FORMAT format,
                            CmSurface2DRT* & pSurface);
    int32_t CreateBuffer(size_t size, CM_BUFFER_TYPE type,
                         bool isCMRTAllocatedSVM, CmBuffer_RT* & pSurface1D,
                         PMOS_RESOURCE pMosResource, void* &pSysMem,
                         bool isConditionalBuffer, uint32_t comparisonValue );
    int32_t DestroySurface( CmBuffer_RT* & pSurface, SURFACE_DESTROY_KIND destroyKind);
    int32_t UpdateBuffer(MOS_RESOURCE * mosResource, int index, uint32_t handle);

    int32_t CreateSurface2DUP(uint32_t width, uint32_t height,
                              CM_SURFACE_FORMAT format, void* pSysMem,
                              CmSurface2DUPRT* & pSurface2D);
    int32_t DestroySurface( CmSurface2DUPRT* & pSurface,  SURFACE_DESTROY_KIND destroyKind);

    virtual int32_t CreateSurface2DFromMosResource(PMOS_RESOURCE pMosResource,
                                           bool bIsCmCreated, CmSurface2DRT* & pSurface2D) = 0;
    virtual int32_t UpdateSurface2D(MOS_RESOURCE * mosResource, int index, uint32_t handle) = 0;

    int32_t DestroySurface( CmSurface2DRT* & pSurface, SURFACE_DESTROY_KIND destroyKind);
    int32_t CreateSurface3D(uint32_t width, uint32_t height, uint32_t depth,
                            CM_SURFACE_FORMAT format, CmSurface3DRT* & pSurface );
    int32_t DestroySurface( CmSurface3DRT* & pSurface, SURFACE_DESTROY_KIND destroyKind);

    int32_t CreateVmeSurface( CmSurface2DRT* pCurrentSurface,CmSurface2DRT** pForwardSurface,
                          CmSurface2DRT** pBackwardSurface,const uint32_t surfaceCountForward,
                          const uint32_t surfaceCountBackward, SurfaceIndex* & pVmeIndex );
    int32_t DestroyVmeSurface( SurfaceIndex* & pVmeIndex );
    int32_t DestroySurface( CmSurfaceVme* & pSurfaceVme );

    int32_t CreateSampler8x8Surface(CmSurface2DRT* pCurrentSurface,
                                    SurfaceIndex* & pSampler8x8Index,
                                    CM_SAMPLER8x8_SURFACE sampler8x8_type,
                                    CM_SURFACE_ADDRESS_CONTROL_MODE mode, CM_FLAG* pFlag);
    int32_t DestroySampler8x8Surface( SurfaceIndex* & pSampler8x8Index );
    int32_t CreateSampler8x8SurfaceFromAlias(
                                    CmSurface2DRT *originalSurface,
                                    SurfaceIndex *aliasIndex,
                                    CM_SURFACE_ADDRESS_CONTROL_MODE addressControl,
                                    SurfaceIndex* &sampler8x8SurfaceIndex);

    int32_t DestroySurface( CmSurfaceSampler8x8* & pSurfaceSampler8x8 );

    int32_t GetSurface( const uint32_t index, CmSurface* & pSurface );
    int32_t GetCmDevice( CmDeviceRT* & pCmDevice );

    int32_t GetPixelBytesAndHeight(uint32_t width, uint32_t height,
                                   CM_SURFACE_FORMAT format, uint32_t& sizePerPixel,
                                   uint32_t& updatedHeight);
    virtual int32_t Surface2DSanityCheck(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format) = 0;

    int32_t CreateSamplerSurface(CmSurface2DRT* pCurrentSurface,
                                 SurfaceIndex* & pSamplerSurfaceIndex, CM_FLAG* pFlag);
    int32_t CreateSamplerSurface(CmSurface2DUPRT* pCurrentSurface,
                                 SurfaceIndex* & pSamplerSurfaceIndex);
    int32_t CreateSamplerSurface(CmSurface3DRT* pCurrentSurface,
                                 SurfaceIndex* & pSamplerSurfaceIndex);
    int32_t DestroySamplerSurface(SurfaceIndex* & pSamplerSurfaceIndex );
    int32_t DestroySurface( CmSurfaceSampler* & pSurfaceSampler );
    uint32_t GetSurfacePoolSize();
    int32_t IncreaseSurfaceUsage(uint32_t index);
    int32_t DecreaseSurfaceUsage(uint32_t index);
    int32_t RefreshDelayDestroySurfaces(uint32_t &freeSurfaceCount);
    int32_t TouchSurfaceInPoolForDestroy();
    int32_t GetFreeSurfaceIndexFromPool(uint32_t &freeIndex);
    int32_t GetFreeSurfaceIndex(uint32_t &index);

    int32_t AllocateSurfaceIndex(size_t width, uint32_t height,
                                 uint32_t depth, CM_SURFACE_FORMAT format,
                                 uint32_t &index, void *pSysMem);

    int32_t DestroySurfaceArrayElement( uint32_t index );
    inline int32_t GetMemorySizeOfSurfaces();

    int32_t GetSurfaceArraySize(uint32_t& surfaceArraySize);

    int32_t UpdateStateForDelayedDestroy(SURFACE_DESTROY_KIND destroyKind, uint32_t i);

    int32_t UpdateStateForRealDestroy(uint32_t i, CM_ENUM_CLASS_TYPE kind);
    int32_t UpdateProfileFor2DSurface(uint32_t index, uint32_t width,
                                      uint32_t height, CM_SURFACE_FORMAT format);
    int32_t UpdateProfileFor1DSurface(uint32_t index, uint32_t size);
    int32_t UpdateProfileFor3DSurface(uint32_t index, uint32_t width,
                                      uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format);

    int32_t UpdateSurface2DTableMosResource( uint32_t index, PMOS_RESOURCE pMosResource );
    int32_t UpdateSurface2DTableRotation(uint32_t index, CM_ROTATION rotationFlag);

    int32_t UpdateSurface2DTableFrameType(uint32_t index, CM_FRAME_TYPE frameType);
    int32_t UpdateSurface2DTableChromaSiting(uint32_t index, int32_t chromaSiting);

    int32_t DestroyStateBuffer( CmStateBuffer *&buffer_ptr,
                                SURFACE_DESTROY_KIND destroyKind );
    int32_t DestroyMediaState( void  *media_state_ptr );

    bool IsCmReservedSurfaceIndex(uint32_t surfBTI);
    bool IsValidSurfaceIndex(uint32_t surfBTI);
    uint32_t MaxIndirectSurfaceCount();
    uint32_t ValidSurfaceIndexStart();
    bool IsSupportedForSamplerSurface2D(CM_SURFACE_FORMAT format);
    inline void SetLatestVeboxTrackerAddr(uint32_t *tracker) {m_latestVeboxTracker = tracker; }
    inline uint32_t LatestVeboxTracker() {return *m_latestVeboxTracker; }

    void AddToDelayDestroyList(CmSurface *surface);
    void RemoveFromDelayDestroyList(CmSurface *surface);
    std::set<CmSurface *> & GetStatelessSurfaceArray() { return m_statelessSurfaceArray; }

protected:
    CmSurfaceManagerBase( CmDeviceRT* pCmDevice );
    CmSurfaceManagerBase();

    virtual ~CmSurfaceManagerBase( void );
    int32_t Initialize( CM_HAL_MAX_VALUES HalMaxValues, CM_HAL_MAX_VALUES_EX HalMaxValuesEx );

    int32_t AllocateBuffer(size_t size, CM_BUFFER_TYPE type, uint32_t & handle,
                           PMOS_RESOURCE pMosResource, void* &pSysMem,
                           uint64_t &gfxMem); // Create the internal buffer of surface1D in driver
    int32_t FreeBuffer( uint32_t handle ); // Destroy the internal buffer of surface1D in driver

    int32_t AllocateSurface2DUP(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format,
                                void* pSysMem, uint32_t & handle );
    int32_t FreeSurface2DUP( uint32_t handle );

    int32_t AllocateSurface2D(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format,
                              uint32_t & handle, uint32_t & pitch );
    int32_t AllocateSurface2D(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format,
                              PMOS_RESOURCE pMosResource, uint32_t & handle);

    int32_t FreeSurface2D( uint32_t handle );

    int32_t Allocate3DSurface(uint32_t width, uint32_t height, uint32_t depth,
                              CM_SURFACE_FORMAT format, uint32_t & handle );
    int32_t Free3DSurface( uint32_t handle );

    int32_t GetFormatSize(CM_SURFACE_FORMAT format,uint32_t &sizeperpixel);

    virtual int32_t GetSurfaceInfo( PMOS_RESOURCE pResource, uint32_t &width,
                                    uint32_t &height, uint32_t &pitch, CM_SURFACE_FORMAT &format) = 0;

    int32_t GetSurfaceBTIInfo();

public:
    // mamimum number of cm device allowed for creating a cm surf2d wrapper for a mos resource
    static const uint32_t MAX_DEVICE_FOR_SAME_SURF = 64;
protected:

    CmDeviceRT* m_device;
    // if VPHAL is not defined, it should be CM_MAX_ALL_SURFACE_COUNT
    uint32_t m_surfaceArraySize;

    CmSurface** m_surfaceArray;
    // the max index allocated in the m_SurfaceArray
    uint32_t m_maxSurfaceIndexAllocated;
    // Size of each surface in surface array
    int32_t *m_surfaceSizes;

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

    uint32_t *m_latestVeboxTracker;

    CmSurface *m_delayDestroyHead;
    CmSurface *m_delayDestroyTail;
    CSync m_delayDestoryListSync;

    std::set<CmSurface *> m_statelessSurfaceArray;

private:
    CmSurfaceManagerBase(const CmSurfaceManagerBase& other);
    CmSurfaceManagerBase& operator= (const CmSurfaceManagerBase& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_COMMON_CM_CMSURFACEMANAGER_BASE_H_
