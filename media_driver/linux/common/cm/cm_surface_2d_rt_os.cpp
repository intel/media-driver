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
//! \file      cm_surface_2d_rt_os.cpp
//! \brief     Contains Linux-dependent CmSurface2DRT member functions.
//!

#include "cm_surface_2d_rt.h"

#include "cm_surface_manager.h"
#include "cm_device_rt.h"
#include "cm_mem.h"

#define COPY_OPTION(option)    (option & 0x1)

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of CmSurface2D
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmSurface2DRT::CmSurface2DRT(
    uint32_t handle,
    uint32_t width,
    uint32_t height,
    uint32_t pitch,
    CM_SURFACE_FORMAT format,
    CmSurfaceManager* surfaceManager ,
    bool isCmCreated):
    CmSurface( surfaceManager,isCmCreated ),
    m_width( width ),
    m_height( height ),
    m_handle( handle ),
    m_pitch(pitch),
    m_format(format),
    m_numAliases( 0 ),
    m_vaSurfaceID( 0 ),
    m_vaCreated ( false ),
    m_umdResource( nullptr ),
    m_frameType(CM_FRAME)
{
    CmSurface::SetMemoryObjectControl(MEMORY_OBJECT_CONTROL_UNKNOW, CM_USE_PTE, 0);
    CmSafeMemSet(m_aliasIndexes, 0, sizeof(SurfaceIndex*) * CM_HAL_MAX_NUM_2D_ALIASES);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of CmSurface2D
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmSurface2DRT::~CmSurface2DRT( void )
{
    for( uint32_t i = 0; i < CM_HAL_MAX_NUM_2D_ALIASES; ++i )
    {
        MosSafeDelete(m_aliasIndexes[i]);
    }
    if(m_vaCreated && IsCmCreated())
   // if(m_vaCreated )
    {
        // Release VA Surface created in thin layer via call back
        CmDeviceRT *device;
        m_surfaceMgr->GetCmDevice(device);
        device->ReleaseVASurface(m_vaDisplay, &m_vaSurfaceID);
    }
}

int32_t CmSurface2DRT::SetVaSurfaceID( VASurfaceID  vaSurface, void  *vaDisplay)
{
    m_vaSurfaceID = vaSurface;
    m_vaCreated  = true;
    m_vaDisplay      = vaDisplay;

    return CM_SUCCESS;
}
CM_RT_API int32_t CmSurface2DRT::GetVaSurfaceID( VASurfaceID  &vaSurface)
{
   vaSurface = m_vaSurfaceID;
   return CM_SUCCESS;
}
}  // namespace
