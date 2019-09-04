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
#include "cm_surface_manager_base.h"

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

class CmSurfaceManager:public CmSurfaceManagerBase
{
public:
    static int32_t Create(
        CmDeviceRT* device,
        CM_HAL_MAX_VALUES halMaxValues,
        CM_HAL_MAX_VALUES_EX lalMaxValuesEx,
        CmSurfaceManager* &manager );

    static int32_t Destroy( CmSurfaceManager* &manager );

    int32_t CreateSurface2DFromMosResource(MOS_RESOURCE * mosResource, bool createdByCm, CmSurface2DRT* &surface);
    int32_t UpdateSurface2D(MOS_RESOURCE * mosResource, int index, uint32_t handle);    

    int32_t Surface2DSanityCheck(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format);

protected:
    CmSurfaceManager(CmDeviceRT* device);
    CmSurfaceManager();
    ~CmSurfaceManager( void );
    int32_t GetSurfaceInfo( MOS_RESOURCE *mosResource, uint32_t &width, uint32_t &height, uint32_t &pitch, CM_SURFACE_FORMAT &format);
   
private:
    CmSurfaceManager (const CmSurfaceManager& other);
    CmSurfaceManager& operator= (const CmSurfaceManager& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMSURFACEMANAGER_H_
