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
//! \file      cm_surface_2d_rt_base.h
//! \brief     Contains CmSurface2DRTBase declaration.
//!

#ifndef MEDIADRIVER_COMMON_CM_CMSURFACE2DRT_BASE_H_
#define MEDIADRIVER_COMMON_CM_CMSURFACE2DRT_BASE_H_

#include "cm_surface.h"
#include "cm_surface_2d.h"

#include "cm_hal.h"

namespace CMRT_UMD
{
//!
//! \brief Abstraction of 2D surfaces in graphics memory.
//!
class CmSurface2DRTBase: public CmSurface, public CmSurface2D
{
public:
    CM_RT_API int32_t ReadSurface(unsigned char *pSysMem,
                                  CmEvent *pEvent,
                                  uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t WriteSurface(const unsigned char *pSysMem,
                                   CmEvent *pEvent,
                                   uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t
    ReadSurfaceStride(unsigned char *pSysMem,
                      CmEvent *pEvent,
                      const unsigned int stride,
                      uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t
    WriteSurfaceStride(const unsigned char *pSysMem,
                       CmEvent *pEvent,
                       const unsigned int stride,
                       uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t
    ReadSurfaceHybridStrides(unsigned char *pSysMem,
                             CmEvent *pEvent,
                             const unsigned int iWidthStride,
                             const unsigned int iHeightStride,
                             uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL,
                             unsigned int uiOption = 0);

    CM_RT_API int32_t
    WriteSurfaceHybridStrides(const unsigned char *pSysMem,
                              CmEvent *pEvent,
                              const unsigned int iWidthStride,
                              const unsigned int iHeightStride,
                              uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL,
                              unsigned int uiOption = 0);

    CM_RT_API int32_t GetIndex(SurfaceIndex *&pIndex);

   virtual CM_RT_API int32_t GetSurfaceDesc(unsigned int &width,
                                     unsigned int &height,
                                     CM_SURFACE_FORMAT &format,
                                     unsigned int &sizeperpixel);

    CM_RT_API int32_t InitSurface(const unsigned int initValue,
                                  CmEvent *pEvent,
                                  unsigned int useGPU = 0);

    CM_RT_API int32_t
    SetSurfaceStateParam(SurfaceIndex *pSurfIndex,
                         const CM_SURFACE2D_STATE_PARAM *pSSParam);

    CM_RT_API int32_t ReadSurfaceFullStride(unsigned char *pSysMem,
                                            CmEvent *pEvent,
                                            const unsigned int iWidthStride,
                                            const unsigned int iHeightStride,
                                            uint64_t sysMemSize);

    CM_RT_API int32_t WriteSurfaceFullStride(const unsigned char *pSysMem,
                                             CmEvent *pEvent,
                                             const unsigned int iWidthStride,
                                             const unsigned int iHeightStride,
                                             uint64_t sysMemSize);

    int32_t SetMemoryObjectControl(MEMORY_OBJECT_CONTROL mem_ctrl,
                                   MEMORY_TYPE mem_type,
                                   unsigned int age);

    CM_RT_API int32_t SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl);

    CMRT_UMD_API int32_t SetResourceUsage(const MOS_HW_RESOURCE_DEF mosUsage);

    CM_RT_API int32_t SetCompressionMode(MEMCOMP_STATE MmcMode);

    CM_RT_API int32_t SetProperty(CM_FRAME_TYPE frameType);

    virtual int32_t GetIndexFor2D(unsigned int &index);

    virtual int32_t GetHandle(unsigned int &handle);

    int32_t SetSurfaceProperties(unsigned int width,
                                 unsigned int height,
                                 CM_SURFACE_FORMAT format);

    //NOT depend on RTTI::dynamic_cast
    CM_ENUM_CLASS_TYPE Type() const { return CM_ENUM_CLASS_TYPE_CMSURFACE2D; }

    bool IsGPUCopy(void *pSysMem,
                   uint32_t iWidth,
                   uint32_t iHeight,
                   uint32_t iWidthStride);

    bool IsUnalignedGPUCopy(uint32_t iWidthInBytes, uint32_t iHeight);

    CMRT_UMD_API int SetReadSyncFlag(bool bReadSync, CmQueue *pCmQueue);


    CMRT_UMD_API int
    NotifyUmdResourceChanged(void *umdResource,
                             int updateMosResource = 0,
                             PMOS_RESOURCE pMosResource = nullptr);

    virtual int32_t UpdateResource(MOS_RESOURCE *resource);

    virtual int32_t Create2DAlias(SurfaceIndex *&pAliasSurfIndex);

    virtual int32_t GetNumAliases(unsigned int &numAliases);

    void Log(std::ostringstream &oss);

    void DumpContent(uint32_t kernelNumber,
                     char *kernelName,
                     int32_t taskId,
                     uint32_t argIndex,
                     uint32_t vectorIndex);

    void DumpContentToFile(const char *filename);

    virtual int32_t UpdateSurfaceProperty(uint32_t width, uint32_t height, uint32_t pitch, CM_SURFACE_FORMAT format);

protected:
    CmSurface2DRTBase(unsigned int handle,
                  unsigned int width,
                  unsigned int height,
                  unsigned int pitch,
                  CM_SURFACE_FORMAT format,
                  CmSurfaceManager *pSurfaceManager,
                  bool isCmCreated);

    virtual ~CmSurface2DRTBase();

    int32_t Initialize(unsigned int index);

    unsigned int m_width;

    unsigned int m_height;

    unsigned int m_handle;

    unsigned int m_pitch;

    CM_SURFACE_FORMAT m_format;

    // a pointer to UMD resource, set to nullptr if no UMD resource related to
    void *m_umdResource;

    // number of surfaces created as an alias
    unsigned int m_numAliases;

    SurfaceIndex *m_aliasIndexes[CM_HAL_MAX_NUM_2D_ALIASES];

    CM_FRAME_TYPE m_frameType;
};
};  //namespace

#endif  // #ifndef MEDIADRIVER_COMMON_CM_CMSURFACE2DRT_BASE_H_
