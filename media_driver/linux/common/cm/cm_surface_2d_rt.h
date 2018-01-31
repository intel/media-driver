/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      cm_surface_2d.h
//! \brief     Contains CmSurface2DRT declaration.
//!
#ifndef MEDIADRIVER_LINUX_COMMON_CM_CMSURFACE2DRT_H_
#define MEDIADRIVER_LINUX_COMMON_CM_CMSURFACE2DRT_H_

#include "cm_surface.h"
#include "cm_surface_2d.h"

#include "cm_hal.h"

namespace CMRT_UMD
{
class CmSurfaceManager;

class CmSurface2DRT: public CmSurface, public CmSurface2D
{
public:
    static int32_t Create(unsigned int index,
                          unsigned int handle,
                          unsigned int width,
                          unsigned int height,
                          unsigned int pitch,
                          MOS_FORMAT format,
                          bool isCmCreated,
                          CmSurfaceManager *surfaceManager,
                          CmSurface2DRT* &surface);

    CM_RT_API int32_t
    ReadSurface(unsigned char *sysMem,
                CmEvent *event,
                uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t WriteSurface(const unsigned char *sysMem,
                                   CmEvent *event,
                                   uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t
    ReadSurfaceStride(unsigned char *sysMem,
                      CmEvent *event,
                      const unsigned int stride,
                      uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t
    WriteSurfaceStride(const unsigned char *sysMem,
                       CmEvent *event,
                       const unsigned int stride,
                       uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t
    ReadSurfaceHybridStrides(unsigned char *sysMem,
                             CmEvent *event,
                             const unsigned int horizontalStride,
                             const unsigned int verticalStride,
                             uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL,
                             unsigned int option = 0);

    CM_RT_API int32_t
    WriteSurfaceHybridStrides(const unsigned char *sysMem,
                              CmEvent *event,
                              const unsigned int horizontalStride,
                              const unsigned int verticalStride,
                              uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL,
                              unsigned int option = 0);

    CM_RT_API int32_t GetIndex(SurfaceIndex* &index);

    CM_RT_API int32_t GetSurfaceDesc(unsigned int &width,
                                     unsigned int &height,
                                     MOS_FORMAT &format,
                                     unsigned int &sizeperpixel);

    CM_RT_API int32_t InitSurface(const unsigned int initValue,
                                  CmEvent *event);

    CM_RT_API int32_t
    SetSurfaceStateParam(SurfaceIndex *surfaceIndex,
                         const CM_SURFACE2D_STATE_PARAM *surfStateParam);

    CM_RT_API int32_t ReadSurfaceFullStride(unsigned char *sysMem,
                                            CmEvent *event,
                                            const unsigned int horizontalStride,
                                            const unsigned int verticalStride,
                                            uint64_t sysMemSize);

    CM_RT_API int32_t WriteSurfaceFullStride(const unsigned char *sysMem,
                                             CmEvent *event,
                                             const unsigned int horizontalStride,
                                             const unsigned int verticalStride,
                                             uint64_t sysMemSize);

    int32_t SetMemoryObjectControl(MEMORY_OBJECT_CONTROL memCtrl,
                                   MEMORY_TYPE memType,
                                   unsigned int age);

    CM_RT_API int32_t
    SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL memCtrl);

    CM_RT_API int32_t SetCompressionMode(MEMCOMP_STATE mmcMode);

    CM_RT_API int32_t GetVaSurfaceID(VASurfaceID  &vaSurface);

    CM_RT_API int32_t SetProperty(CM_FRAME_TYPE frameType);

    int32_t GetIndexFor2D(unsigned int &index);

    int32_t GetHandle(unsigned int &handle);

    int32_t SetSurfaceProperties(unsigned int width,
                                 unsigned int height,
                                 MOS_FORMAT format);

    //NOT depend on RTTI::dynamic_cast
    CM_ENUM_CLASS_TYPE Type() const
    { return CM_ENUM_CLASS_TYPE_CMSURFACE2D; }

    bool IsGPUCopy(void *sysMem,
                   uint32_t width,
                   uint32_t height,
                   uint32_t horizontalStride);
    
    bool IsUnalignedGPUCopy(uint32_t widthInBytes,
                            uint32_t height);

    CMRT_UMD_API int32_t SetReadSyncFlag(bool readSync);

    CMRT_UMD_API int32_t
    NotifyUmdResourceChanged(UMD_RESOURCE umdResource,
                             int updateMosResource = 0,
                             PMOS_RESOURCE mosResource = nullptr);

    int32_t Create2DAlias(SurfaceIndex* &aliasSurfIndex);

    int32_t GetNumAliases(unsigned int &numAliases);

    void Log(std::ostringstream &oss);

    void DumpContent(uint32_t kernelNumber,
                     int32_t taskId,
                     uint32_t argIndex);

    int32_t SetVaSurfaceID(VASurfaceID vaSurface, void *vaDisplay);

protected:
    CmSurface2DRT(unsigned int handle,
                  unsigned int width,
                  unsigned int height,
                  unsigned int pitch,
                  MOS_FORMAT format,
                  CmSurfaceManager *surfaceManager,
                  bool isCmCreated);

    ~CmSurface2DRT();

    int32_t Initialize(unsigned int index);

    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_handle;
    unsigned int m_pitch;
    MOS_FORMAT m_format;

    // a pointer to UMD resource, set to nullptr if no resource related to
    UMD_RESOURCE m_umdResource;

    // number of surfaces created as an alias
    unsigned int m_numAliases;

    SurfaceIndex* m_aliasIndexes[CM_HAL_MAX_NUM_2D_ALIASES];

    VASurfaceID m_vaSurfaceID;

    int m_vaCreated;

    void *m_vaDisplay;

    CM_FRAME_TYPE m_frameType;
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMSURFACE2DRT_H_
