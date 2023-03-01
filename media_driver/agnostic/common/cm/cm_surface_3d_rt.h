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
//! \file      cm_surface_3d_rt.h
//! \brief     Contains CmSurface3DRT declarations.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSURFACE3DRT_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSURFACE3DRT_H_

#include "cm_surface_3d.h"
#include "cm_surface.h"

namespace CMRT_UMD
{
class CmSurface3DRT: public CmSurface, public CmSurface3D
{
public:
    static int32_t Create(uint32_t index,
                          uint32_t handle,
                          uint32_t width,
                          uint32_t height,
                          uint32_t depth,
                          CM_SURFACE_FORMAT format,
                          CmSurfaceManager *surfaceManager,
                          CmSurface3DRT* &surface);

    CM_RT_API int32_t ReadSurface(unsigned char *sysMem,
                                  CmEvent *event,
                                  uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t WriteSurface(const unsigned char *sysMem,
                                   CmEvent *event,
                                   uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL);

    CM_RT_API int32_t InitSurface(const uint32_t initValue, CmEvent *event);

    CM_RT_API int32_t GetIndex(SurfaceIndex* &index);

    virtual int32_t GetHandle(uint32_t &handle);

    virtual int32_t GetProperties(uint32_t &width,
                          uint32_t &height,
                          uint32_t &depth,
                          CM_SURFACE_FORMAT &format);

    int32_t SetProperties(uint32_t width,
                          uint32_t height,
                          uint32_t depth,
                          CM_SURFACE_FORMAT format);

    //NOT depend on RTTI::dynamic_cast
    CM_ENUM_CLASS_TYPE Type() const { return CM_ENUM_CLASS_TYPE_CMSURFACE3D; };

    int32_t SetMemoryObjectControl(MEMORY_OBJECT_CONTROL memCtrl,
                                   MEMORY_TYPE memType,
                                   uint32_t age);

    CM_RT_API int32_t
    SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL memCtrl);

    CMRT_UMD_API int32_t SetResourceUsage(const MOS_HW_RESOURCE_DEF mosUsage);

    void Log(std::ostringstream &oss);

    void DumpContent(uint32_t kernelNumber,
                     char *kernelName,
                     int32_t taskId,
                     uint32_t argIndex,
                     uint32_t vectorIndex);

protected:
    CmSurface3DRT(uint32_t handle,
                  uint32_t width,
                  uint32_t height,
                  uint32_t depth,
                  CM_SURFACE_FORMAT format,
                  CmSurfaceManager *surfaceManager);

    ~CmSurface3DRT();

    int32_t Initialize(uint32_t index);

    uint32_t m_handle;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_depth;
    CM_SURFACE_FORMAT m_format;

private:
    CmSurface3DRT(const CmSurface3DRT& other);
    CmSurface3DRT& operator=(const CmSurface3DRT& other);
};
};  //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMSURFACE3DRT_H_
