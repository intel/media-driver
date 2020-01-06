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
#include "cm_surface_2d_rt_base.h"

#include "cm_hal.h"

namespace CMRT_UMD
{
class CmSurfaceManager;

class CmSurface2DRT: public CmSurface2DRTBase
{
public:
    static int32_t Create(unsigned int index,
                          unsigned int handle,
                          unsigned int width,
                          unsigned int height,
                          unsigned int pitch,
                          CM_SURFACE_FORMAT format,
                          bool isCmCreated,
                          CmSurfaceManager *surfaceManager,
                          CmSurface2DRT* &surface);

    CM_RT_API int32_t GetVaSurfaceID(VASurfaceID  &vaSurface);

    int32_t SetVaSurfaceID(VASurfaceID vaSurface, void *vaDisplay);

protected:
    CmSurface2DRT(unsigned int handle,
                  unsigned int width,
                  unsigned int height,
                  unsigned int pitch,
                  CM_SURFACE_FORMAT format,
                  CmSurfaceManager *surfaceManager,
                  bool isCmCreated);

    ~CmSurface2DRT();

    VASurfaceID m_vaSurfaceID;

    int m_vaCreated;

    void *m_vaDisplay;

private:
    CmSurface2DRT(const CmSurface2DRT& other);
    CmSurface2DRT& operator=(const CmSurface2DRT& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMSURFACE2DRT_H_
