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
//! \file      cm_surface_manager_os.cpp 
//! \brief     Contains Class CmSurfaceManager  definitions 
//!
#include "cm_surface_manager.h"

#include "cm_surface_2d_rt.h"

#include "cm_surface_2d_rt.h"
#include "cm_device_rt.h"
#include "cm_mem.h"

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface Manager
//| Arguments :
//|               device         [in]       Pointer to Cm Device
//|               halMaxValues   [in]       Cm Max values
//|               surfaceManager [out]      Reference to pointer to CmSurfaceManager
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurfaceManager::Create(
    CmDeviceRT* device,
    CM_HAL_MAX_VALUES halMaxValues,
    CM_HAL_MAX_VALUES_EX halMaxValuesEx,
    CmSurfaceManager* &surfaceManager)
{
    int32_t result = CM_SUCCESS;

    surfaceManager = new (std::nothrow) CmSurfaceManager(device);
    if (surfaceManager)
    {
        result = surfaceManager->Initialize(halMaxValues, halMaxValuesEx);
        if (result != CM_SUCCESS)
        {
            CmSurfaceManager::Destroy(surfaceManager);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmSurfaceManager due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy CmSurfaceManager
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurfaceManager::Destroy(CmSurfaceManager* &surfaceManager)
{
    if (surfaceManager)
    {
        delete surfaceManager;
        surfaceManager = nullptr;
    }
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of CmSurfaceManager
//| Returns:    None
//*-----------------------------------------------------------------------------
CmSurfaceManager::CmSurfaceManager(CmDeviceRT* device) :CmSurfaceManagerBase(device) {}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of CmSurfaceManager
//| Returns:    None
//*-----------------------------------------------------------------------------
CmSurfaceManager::~CmSurfaceManager() {}

//*-----------------------------------------------------------------------------
//| Purpose:    Create surface 2d with mos info
//| Arguments :
//|               mosResource        [in]       pointer to mos resource
//|               createdByCm        [in]       if this surface created by thin layer
//|               surface          [IN/out]   Reference to CmSurface2D
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
 int32_t CmSurfaceManager::CreateSurface2DFromMosResource(MOS_RESOURCE * mosResource, bool createdByCm, CmSurface2DRT* & surface)
 {
    uint32_t handle         = 0;
    uint32_t index          = ValidSurfaceIndexStart();
    int32_t result         = 0;
    uint32_t width          = 0;
    uint32_t height         = 0;
    uint32_t pitch          = 0;
    CM_SURFACE_FORMAT   format = CM_SURFACE_FORMAT_INVALID;

    surface = nullptr;

    //Get width, height, pitch, format from mosResource
    result = GetSurfaceInfo(mosResource, width, height, pitch, format);
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Failed to get surface info from mosResource.");
        return result;
    }

    //Sanity check
    result = Surface2DSanityCheck(width, height, format);
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Sanity check for surface 2D failure.");
        return result;
    }

    // For 2D surface, since the real memory buffer is not allocated in CMRT@UMD, no reuse/manager can be done
    // Real reuse is controlled by the CMRT library.
    if (GetFreeSurfaceIndex(index) != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Exceed the maximum surface amount.");
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    if( m_2DSurfaceCount >= m_max2DSurfaceCount )
    {
        CM_ASSERTMESSAGE("Error: Exceed the maximum surface amount.");
        return CM_EXCEED_SURFACE_AMOUNT;
    }

    result = AllocateSurface2D( width, height, format, mosResource, handle);
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Failed to allocate surface.");
        return result;
    }

    result = CmSurface2DRT::Create( index, handle, width, height, pitch, format, createdByCm, this, surface );
    if( result != CM_SUCCESS )
    {
        FreeSurface2D( handle );
        CM_ASSERTMESSAGE("Error: Failed to create CmSurface2D.");
        return result;
    }

    m_surfaceArray[ index ] = surface;
    UpdateProfileFor2DSurface(index, width, height, format);

    return CM_SUCCESS;
 }

//*-----------------------------------------------------------------------------
//| Purpose:    Check the legality of surface's width,height and format
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurfaceManager::Surface2DSanityCheck(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format)
{
    if( ( width < CM_MIN_SURF_WIDTH ) || ( width > CM_MAX_2D_SURF_WIDTH ) )
    {
        CM_ASSERTMESSAGE("Error: Invalid surface width.");
        return CM_INVALID_WIDTH;
    }

    if( ( height < CM_MIN_SURF_HEIGHT ) || ( height > CM_MAX_2D_SURF_HEIGHT ) )
    {
        CM_ASSERTMESSAGE("Error: Invalid surface height.");
        return CM_INVALID_HEIGHT;
    }

    switch( format )
    {
        case CM_SURFACE_FORMAT_X8R8G8B8:
        case CM_SURFACE_FORMAT_A8R8G8B8:
        case CM_SURFACE_FORMAT_A8B8G8R8:
        case CM_SURFACE_FORMAT_R32F:
        case CM_SURFACE_FORMAT_A8:
        case CM_SURFACE_FORMAT_P8:
        case CM_SURFACE_FORMAT_R8G8_SNORM:
        case CM_SURFACE_FORMAT_Y8_UNORM:
        case CM_SURFACE_FORMAT_Y16_UNORM:
        case CM_SURFACE_FORMAT_Y16_SNORM:
        case CM_SURFACE_FORMAT_A16B16G16R16:
        case CM_SURFACE_FORMAT_R10G10B10A2:
        case CM_SURFACE_FORMAT_A16B16G16R16F:
        case CM_SURFACE_FORMAT_V8U8:
        case CM_SURFACE_FORMAT_R16_FLOAT:
        case CM_SURFACE_FORMAT_L8:
        case CM_SURFACE_FORMAT_R32_SINT:
        case CM_SURFACE_FORMAT_R32_UINT:
        case CM_SURFACE_FORMAT_BUFFER_2D:
        case CM_SURFACE_FORMAT_Y216:
        case CM_SURFACE_FORMAT_Y416:
        case CM_SURFACE_FORMAT_AYUV:
        case CM_SURFACE_FORMAT_Y210:
        case CM_SURFACE_FORMAT_Y410:
        case CM_SURFACE_FORMAT_R32G32B32A32F:
        case CM_SURFACE_FORMAT_400P:
            break;

        case CM_SURFACE_FORMAT_R8_UINT:
        case CM_SURFACE_FORMAT_R16_UINT:
        case CM_SURFACE_FORMAT_L16:
            break;

        case CM_SURFACE_FORMAT_UYVY:
        case CM_SURFACE_FORMAT_YUY2:
            if( width & 0x1 )
            {
                CM_ASSERTMESSAGE("Error: Invalid surface width.");
                return CM_INVALID_WIDTH;
            }
            break;

        case CM_SURFACE_FORMAT_NV12:
            if (width & 0x1)
            {
                CM_ASSERTMESSAGE("Error: Invalid surface width.");
                return CM_INVALID_WIDTH;
            }
            break;

        case CM_SURFACE_FORMAT_P010:
        case CM_SURFACE_FORMAT_P016:
        case CM_SURFACE_FORMAT_YV12:
            if( width & 0x1 )
            {
                CM_ASSERTMESSAGE("Error: Invalid surface width.");
                return CM_INVALID_WIDTH;
            }
            if( height & 0x1 )
            {
                CM_ASSERTMESSAGE("Error: Invalid surface height.");
                return CM_INVALID_HEIGHT;
            }
            break;

        case CM_SURFACE_FORMAT_411P:
        case CM_SURFACE_FORMAT_411R:
        case CM_SURFACE_FORMAT_IMC3:
        case CM_SURFACE_FORMAT_422H:
        case CM_SURFACE_FORMAT_422V:
        case CM_SURFACE_FORMAT_444P:
        case CM_SURFACE_FORMAT_RGBP:
        case CM_SURFACE_FORMAT_BGRP:
        case CM_SURFACE_FORMAT_P208:
            if( width & 0x1 )
            {
                CM_ASSERTMESSAGE("Error: Invalid surface width.");
                return CM_INVALID_WIDTH;
            }
            if( height & 0x1 )
            {
                CM_ASSERTMESSAGE("Error: Invalid surface height.");
                return CM_INVALID_HEIGHT;
            }
            break;

        default:
            CM_ASSERTMESSAGE("Error: Unsupported surface format.");
            return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    return CM_SUCCESS;
}

//!
//! \brief    Get mos surface's information (width, height, pitch and format)
//! \details  This function calls mos interface pfnGetResourceInfo to get details of surface.
//! \param    [in] mosResource
//!           pointer to mos resource
//! \param    [in,out] width
//!           reference to surface's width
//! \param    [in,out] height
//!           reference to surface's height
//! \param    [in,out] pitch
//!           reference to surface's lock pitch
//! \param    [in,out] format
//!           reference to surface's format
//! \return   MOS_STATUS
//!
int32_t CmSurfaceManager::GetSurfaceInfo( MOS_RESOURCE * mosResource, uint32_t &width, uint32_t &height, uint32_t &pitch, CM_SURFACE_FORMAT &format)
{
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();
    PCM_HAL_STATE state  = cmData->cmHalState;

    MOS_SURFACE          surfaceDetails;
    MOS_ZeroMemory(&surfaceDetails, sizeof(surfaceDetails));
    surfaceDetails.Format = CM_SURFACE_FORMAT_INVALID;
    state->osInterface->pfnGetResourceInfo(state->osInterface, mosResource, &surfaceDetails);

    width    = surfaceDetails.dwWidth;
    height   = surfaceDetails.dwHeight;
    format   = surfaceDetails.Format;
    pitch    = surfaceDetails.dwLockPitch;

    return CM_SUCCESS;
}

int32_t CmSurfaceManager::UpdateSurface2D(MOS_RESOURCE * mosResource, int index, uint32_t handle)
{
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();
    PCM_HAL_STATE state = cmData->cmHalState;

    CM_SURFACE_FORMAT format;
    uint32_t width          = 0;
    uint32_t height         = 0;
    uint32_t pitch          = 0;
    int result = GetSurfaceInfo(mosResource, width, height, pitch, format);
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Failed to get surface info from pMosResource.");
        return result;
    }

    CM_HAL_SURFACE2D_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_SURFACE2D_PARAM ) );
    inParam.width                  = width;
    inParam.height                 = height;
    inParam.format                 = format;
    inParam.mosResource            = mosResource;
    inParam.isAllocatedbyCmrtUmd   = false;
    inParam.handle                 = handle;

    state->pfnUpdateSurface2D(state, &inParam);

    CmSurface2DRT *surface = static_cast<CmSurface2DRT *>(m_surfaceArray[index]);

    int ret = surface->UpdateSurfaceProperty(width, height, pitch, format);

    return ret;

}

}  // namespace
