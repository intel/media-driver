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
//! \file      cm_surface_2d_rt.cpp
//! \brief     Contains OS-agnostic CmSurface2DRT member functions
//!

#include "cm_surface_2d_rt.h"

#include "cm_event_rt.h"
#include "cm_surface_manager.h"
#include "cm_device_rt.h"
#include "cm_mem.h"
#include "cm_queue_rt.h"
#include "cm_wrapper_os.h"

#define COPY_OPTION(option)    (option & 0x1)

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Surface 2D
//| Arguments :
//|               index             [in]     index in runtime Surface2D table
//|               handle            [in]     index in driver's surface2D table
//|               width             [in]     width of the  CmSurface2D
//|               height            [in]     height of the CmSurface2D
//|               pitch             [in]     pitch of the CmSurface2D
//|               format            [out]    format of CmSurface2D
//|               isCmCreated       [out]    ture,if the surface created by CM;
//|                                          false,if the surface created externally
//|               surfaceManager   [out]    Pointer to CmSurfaceManager
//|               surface          [out]    Reference to the Pointer to CmSurface2D

//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurface2DRT::Create(
                           uint32_t index,
                           uint32_t handle,
                           uint32_t width,
                           uint32_t height,
                           uint32_t pitch,
                           CM_SURFACE_FORMAT format,
                           bool isCmCreated,
                           CmSurfaceManager* surfaceManager,
                           CmSurface2DRT* &surface )
{
    int32_t result = CM_SUCCESS;

    surface = new (std::nothrow) CmSurface2DRT( handle,width,height, pitch,format,surfaceManager, isCmCreated);
    if( surface )
    {
        result = surface->Initialize( index );
        if( result != CM_SUCCESS )
        {
            CmSurface* baseSurface = surface;
            CmSurface::Destroy( baseSurface );
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to CmSurface2D due to out of system memory.")
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurface2DRT::Initialize( uint32_t index )
{
    return CmSurface::Initialize( index );
}

bool CmSurface2DRT::IsGPUCopy(void *sysMem, uint32_t widthInBytes, uint32_t height, uint32_t horizontalStrideInBytes)
{
    return ( (widthInBytes <= CM_MAX_THREADSPACE_WIDTH_FOR_MW*128) && (height <= CM_MAX_THREADSPACE_HEIGHT_FOR_MW*32) && (uintptr_t(sysMem) & 0xF) == 0 && (horizontalStrideInBytes & 0xF) == 0 );
}

bool CmSurface2DRT::IsUnalignedGPUCopy(uint32_t widthInBytes, uint32_t height)
{
    return (widthInBytes <= CM_MAX_THREADSPACE_WIDTH_FOR_MW*64) && (height <= CM_MAX_THREADSPACE_HEIGHT_FOR_MW*8);
}


//*-----------------------------------------------------------------------------
//| Purpose:    Get planar memory layout information details
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t GetPlanarInfomation(CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam, uint32_t& sizePerPixel, uint32_t& UVwidth, uint32_t& UVheight, uint32_t& UVpitch, uint32_t& planes)
{
    //default single plane format UVheight = 0
    UVheight = 0;
    UVpitch = 0;
    UVwidth = 0;
    planes = 1;
    switch (inParam.format)
    {
    case CM_SURFACE_FORMAT_R32G32B32A32F:
        sizePerPixel = 16;
        break;

    case CM_SURFACE_FORMAT_A16B16G16R16:
    case CM_SURFACE_FORMAT_A16B16G16R16F:
    case CM_SURFACE_FORMAT_Y416:
    case CM_SURFACE_FORMAT_D32F_S8X24_UINT:
    case CM_SURFACE_FORMAT_R32G8X24_TYPELESS:
        sizePerPixel = 8;
        break;

    case CM_SURFACE_FORMAT_X8R8G8B8:
    case CM_SURFACE_FORMAT_A8R8G8B8:
    case CM_SURFACE_FORMAT_A8B8G8R8:
    case CM_SURFACE_FORMAT_R32F:
    case CM_SURFACE_FORMAT_D32F:
    case CM_SURFACE_FORMAT_R32_UINT:
    case CM_SURFACE_FORMAT_R32_SINT:
    case CM_SURFACE_FORMAT_R10G10B10A2:
    case CM_SURFACE_FORMAT_AYUV:
    case CM_SURFACE_FORMAT_R16G16_UNORM:
    case CM_SURFACE_FORMAT_Y410:
    case CM_SURFACE_FORMAT_Y216:
    case CM_SURFACE_FORMAT_Y210:
    case CM_SURFACE_FORMAT_D24_UNORM_S8_UINT:
    case CM_SURFACE_FORMAT_R32_TYPELESS:
    case CM_SURFACE_FORMAT_R24G8_TYPELESS:
    case CM_SURFACE_FORMAT_R16G16_SINT:
        sizePerPixel = 4;
        break;

    case CM_SURFACE_FORMAT_R8G8_SNORM:
    case CM_SURFACE_FORMAT_R16_UINT:
    case CM_SURFACE_FORMAT_R16_SINT:
    case CM_SURFACE_FORMAT_R16_UNORM:
    case CM_SURFACE_FORMAT_D16:
    case CM_SURFACE_FORMAT_L16:
    case CM_SURFACE_FORMAT_R8G8_UNORM:
    case CM_SURFACE_FORMAT_UYVY:
    case CM_SURFACE_FORMAT_VYUY:
    case CM_SURFACE_FORMAT_YUY2:
    case CM_SURFACE_FORMAT_Y16_SNORM:
    case CM_SURFACE_FORMAT_Y16_UNORM:
    case CM_SURFACE_FORMAT_IRW0:
    case CM_SURFACE_FORMAT_IRW1:
    case CM_SURFACE_FORMAT_IRW2:
    case CM_SURFACE_FORMAT_IRW3:
    case CM_SURFACE_FORMAT_R16_FLOAT:
    case CM_SURFACE_FORMAT_V8U8:
    case CM_SURFACE_FORMAT_A8P8:
    case CM_SURFACE_FORMAT_R16_TYPELESS:
        sizePerPixel = 2;
        break;

    case CM_SURFACE_FORMAT_A8:
    case CM_SURFACE_FORMAT_P8:
    case CM_SURFACE_FORMAT_R8_UINT:
    case CM_SURFACE_FORMAT_Y8_UNORM:
    case CM_SURFACE_FORMAT_L8:
    case CM_SURFACE_FORMAT_IA44:
    case CM_SURFACE_FORMAT_AI44:
    case CM_SURFACE_FORMAT_400P:
    case CM_SURFACE_FORMAT_BUFFER_2D:
    case CM_SURFACE_FORMAT_R8_UNORM:
        sizePerPixel = 1;
        break;

        // 2 planes
        //_________________________
        //|Y0|Y1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|U0|V0|                  |
        //|__|__|                  |
        //|                        |
        //|________________________|

    case CM_SURFACE_FORMAT_P016:
    case CM_SURFACE_FORMAT_P010:
        sizePerPixel = 2;
        UVheight = (inParam.height + 1) / 2;
        UVpitch = inParam.pitch;
        UVwidth = inParam.width;
        planes = 2;
        break;
    case CM_SURFACE_FORMAT_NV12:            //NV12
        sizePerPixel = 1;
        //To support NV12 format with odd height here.
        //if original height is even, the UV plane's height is set as updatedHeight/2, which equals to (updatedHeight+1)/2
        //if original height is odd, the UV plane's height is set as roundup(updatedHeight/2), which equals to (updatedHeight+1)/2 too
        UVheight = (inParam.height + 1) / 2;
        UVpitch = inParam.pitch;
        UVwidth = inParam.width;
        planes = 2;
        break;
    case CM_SURFACE_FORMAT_P208:
        sizePerPixel = 1;
        UVheight = inParam.height;
        UVpitch = inParam.pitch;
        UVwidth = inParam.width;
        planes = 2;
        break;

        // 3 planes
        // 4:1:1 (12-bits per pixel)      // 4:2:2 (16-bits per pixel)
        // 411P                           // 422H
        // ----------------->             // ----------------->
        // ________________________       // ________________________
        //|Y0|Y1|                  |      //|Y0|Y1|                  |
        //|__|__|                  |      //|__|__|                  |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|________________________|      //|________________________|
        //|U0|U1||                 |      //|U0|U1|      |           |
        //|__|__||                 |      //|__|__|      |           |
        //|      |                 |      //|            |           |
        //|      |      PAD        |      //|            |    PAD    |
        //|      |                 |      //|            |           |
        //|      |                 |      //|            |           |
        //|      |                 |      //|            |           |
        //|______|_________________|      //|____________|___________|
        //|V0|V1||                 |      //|V0|V1|      |           |
        //|__|__||                 |      //|__|__|      |           |
        //|      |                 |      //|            |           |
        //|      |      PAD        |      //|            |    PAD    |
        //|      |                 |      //|            |           |
        //|      |                 |      //|            |           |
        //|      |                 |      //|            |           |
        //|______|_________________|      //|____________|___________|

        // 4:4:4 (24-bits per pixel)
        // 444P
        // ----------------->
        // ________________________
        //|Y0|Y1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|U0|U1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|V0|V1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|

    case CM_SURFACE_FORMAT_411P:
    case CM_SURFACE_FORMAT_422H:
    case CM_SURFACE_FORMAT_444P:
    case CM_SURFACE_FORMAT_RGBP:
    case CM_SURFACE_FORMAT_BGRP:
        sizePerPixel = 1;
        UVheight = inParam.height;
        UVpitch = inParam.pitch;
        UVwidth = inParam.width;
        planes = 3;
        break;

        // 4:2:0 (12-bits per pixel)
        // IMC1                           // IMC3
        // ----------------->             // ----------------->
        // ________________________       // ________________________
        //|Y0|Y1|                  |      //|Y0|Y1|                  |
        //|__|__|                  |      //|__|__|                  |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|________________________|      //|________________________|
        //|V0|V1|      |           |      //|U0|U1|      |           |
        //|__|__|      |           |      //|__|__|      |           |
        //|            |           |      //|            |           |
        //|____________|  PAD      |      //|____________|  PAD      |
        //|U0|U1|      |           |      //|V0|V1|      |           |
        //|__|__|      |           |      //|__|__|      |           |
        //|            |           |      //|            |           |
        //|____________|___________|      //|____________|___________|
    case CM_SURFACE_FORMAT_IMC3:
        sizePerPixel = 1;
        UVheight = (inParam.height + 1) / 2;
        UVpitch = inParam.pitch;
        UVwidth = inParam.width;
        planes = 3;
        break;


        // 4:2:2V (16-bits per pixel)
        // 422V
        // ----------------->
        // ________________________
        //|Y0|Y1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|U0|U1|                  |
        //|__|__|                  |
        //|                        |
        //|________________________|
        //|V0|V1|                  |
        //|__|__|                  |
        //|                        |
        //|________________________|

    case CM_SURFACE_FORMAT_422V:
        sizePerPixel = 1;
        UVheight = (inParam.height + 1) / 2;
        UVpitch = inParam.pitch;
        UVwidth = inParam.width;
        planes = 3;
        break;

        // 4:2:0 (12-bits per pixel)
        // I420
        // ----------------->
        // ________________________
        //|Y0|Y1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|U0|U1|      |
        //|__|__|      |
        //|            |
        //|____________|
        //|V0|V1|      |
        //|__|__|      |
        //|            |
        //|____________|
    case CM_SURFACE_FORMAT_YV12:
    case CM_SURFACE_FORMAT_I420:            //I420
        sizePerPixel = 1;
        UVheight = (inParam.height + 1) / 2;
        UVpitch = inParam.pitch / 2;
        UVwidth = inParam.width / 2;
        planes = 3;
        break;
    case CM_SURFACE_FORMAT_411R://411R
        sizePerPixel = 1;
        UVheight = inParam.height;
        UVpitch = inParam.pitch / 4;
        UVwidth = inParam.width / 4;
        planes = 3;
        break;

    default:
        CM_ASSERTMESSAGE("Error: Unsupported surface format.");
        return CM_SURFACE_FORMAT_NOT_SUPPORTED;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the index of CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::GetIndex(SurfaceIndex*& index)
{
    index = m_index;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface2DRT::SetCompressionMode(MEMCOMP_STATE mmcMode)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr = CM_SUCCESS;
    CM_HAL_SURFACE2D_COMPRESSIOM_PARAM mmcModeParam;
    CmDeviceRT * cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_ASSERT(cmDevice);
    mmcModeParam.handle = m_handle;
    mmcModeParam.mmcMode = mmcMode;
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_ASSERT(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnSetCompressionMode(cmData->cmHalState, mmcModeParam));

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Write data from  system memory to Surface 2D to with stride
//|             It clips larger picture frame content into smaller gfx surface size
//|             horizontalStride >= surface width * sizePerPixel
//|             verticalStride   >= surface height
//| Arguments :
//|               sysMem      [in]       Pointer to system memory
//|               event       [in]       Pointer to CmEvent
//|               horizontalStride [in]  Stride in system memory in bytes
//|               verticalStride[in]     Height Stride in system memory in rows
//|               sysMemSize   [out]     Size of Memory need to read
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::WriteSurfaceFullStride(const unsigned char* sysMem, CmEvent* event,
    const uint32_t horizontalStride, const uint32_t verticalStride, uint64_t sysMemSize)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint8_t         *dst = nullptr;
    uint8_t         *src = nullptr;
    uint32_t        sizePerPixel = 0;
    uint32_t        updatedHeight = 0;
    uint32_t        widthInByte = 0;
    uint32_t        pitch = 0;
    uint32_t        row = 0;
    uint32_t        UVHeight = 0;
    uint32_t        UVpitch = 0;
    uint32_t        UVwidth = 0;
    uint32_t        planeHeight = 0;
    uint32_t        planes = 0;
    uint32_t        offset0 = 0;
    uint32_t        offset1 = 0;
    uint32_t        offset2 = 0;
    uint32_t        offsetn = 0;
    uint32_t        tmp = 0;
    UNUSED(sysMemSize);

    if (sysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
            return CM_INVALID_ARG_VALUE;
    }

    if (event)
    {
        CmEventRT *eventRT = dynamic_cast<CmEventRT *>(event);
        if (eventRT)
        {
            FlushDeviceQueue(eventRT);
        }
        else
        {
            event->WaitForTaskFinished();
        }
    }
    WaitForReferenceFree();   // wait all owner task finished

    CmDeviceRT * cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    uint32_t platform = 0;
    cmDevice->GetGenPlatform(platform);
    //Lock for surface read/write
    CSync* surfaceLock = cmDevice->GetSurfaceLock();
    CM_ASSERT(surfaceLock);
    CLock locker(*surfaceLock);

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM));
    inParam.width = m_width;
    inParam.height = m_height;
    inParam.handle = m_handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;
    inParam.useGmmOffset = true;

    // Lock Surface Resource
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLock2DResource(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);
    //make sure format is set correctly
    inParam.format = m_format;
    // Get planar memory layout information according to the format
    CM_CHK_CMSTATUS_GOTOFINISH(GetPlanarInfomation(inParam, sizePerPixel, UVwidth, UVHeight, UVpitch, planes));

    // Write copy surface content data
    widthInByte = m_width * sizePerPixel;
    pitch = m_pitch;
    planeHeight = inParam.height;

    // convert gmmlib plane offset to linear offset
    offset0 = inParam.YSurfaceOffset.iSurfaceOffset + (inParam.YSurfaceOffset.iXOffset * sizePerPixel) + (inParam.YSurfaceOffset.iYOffset * pitch);
    offset1 = inParam.USurfaceOffset.iSurfaceOffset + (inParam.USurfaceOffset.iXOffset * sizePerPixel) + (inParam.USurfaceOffset.iYOffset * UVpitch);
    offset2 = inParam.VSurfaceOffset.iSurfaceOffset + (inParam.VSurfaceOffset.iXOffset * sizePerPixel) + (inParam.VSurfaceOffset.iYOffset * UVpitch);

    tmp = offset1;
    if (offset1 > offset2)
    {
        offset1 = offset2;
        offset2 = tmp;
        // some VSurfaceOffset.iSurfaceOffset could be 0 from gmm resource info
        if (offset1 == 0)
            offset1 = offset2;
    }

    // write copy Y plane or single plane format
    dst = (uint8_t *)(inParam.data);
    src = (uint8_t *)sysMem;
    if ((pitch > widthInByte) || (horizontalStride != pitch))
    {
        // scan line copy
        for (row = 0; row < planeHeight; row++)
        {
            CmFastMemCopyWC(dst, src, widthInByte);
            src += horizontalStride;
            dst += pitch;
        }
    }
    else
    {   // block copy
        CmFastMemCopyWC(dst, src, widthInByte * planeHeight);
    }

    if (UVHeight > 0)
    {
        // Write copy 2nd plane
        if (planes > 1)
        {
            offsetn = offset1;
            //  Limit the Gmm offset usage to after Gen11
            if ((platform <= IGFX_GEN11LP_CORE) || !inParam.useGmmOffset)
            {
                offsetn = (planeHeight * pitch);
            }
            dst = (uint8_t *)(inParam.data) + offsetn;
            src = (uint8_t *)sysMem + horizontalStride * verticalStride;
            for (row = 0; row < UVHeight; row++)
            {
                CmFastMemCopyWC(dst, src, UVwidth * sizePerPixel);
                src += horizontalStride * UVwidth / m_width;
                dst += UVpitch;
            }
        }

        // Write copy 3rd plane
        if (planes > 2)
        {
            offsetn = offset2;
            //  Limit the Gmm offset usage to after Gen11
            if ((platform <= IGFX_GEN11LP_CORE) || !inParam.useGmmOffset)
            {
                offsetn = (planeHeight * pitch) + (UVHeight * UVpitch);
            }
            dst = (uint8_t *)(inParam.data) + offsetn;
            // calculate system memory frame buffer UV plain width and height with ratio of video surface UV plane vs Y plane
            // UVwidth / m_width  is horizontal ratio;  UVHeight / m_height  is vertical ratio
            // system memory frame buffer 3rd plane offset is Y plane size + 2nd Plane size
            src = (uint8_t *)sysMem + (horizontalStride * verticalStride) +
                  (horizontalStride * UVwidth / m_width * verticalStride * UVHeight / m_height);
            for (row = 0; row < UVHeight; row++)
            {
                CmFastMemCopyWC(dst, src, UVwidth * sizePerPixel);
                src += horizontalStride * UVwidth / m_width;
                dst += UVpitch;
            }
        }
    }

    //Unlock Surface2D
    inParam.data = nullptr;
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlock2DResource(cmData->cmHalState, &inParam));
finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Write data from  system memory to Surface 2D to with stride
//|             It clips larger picture frame content into smaller gfx surface size
//|             horizontalStride >= surface width * sizePerPixel
//|             verticalStride   == surface height
//| Arguments :
//|               sysMem      [in]       Pointer to system memory
//|               event       [in]       Pointer to CmEvent
//|               Stride       [in]       Stride in system memory in bytes
//|               sysMemSize   [out]      Size of Memory need to read
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::WriteSurfaceStride(const unsigned char* sysMem, CmEvent* event, const uint32_t stride, uint64_t sysMemSize)
{
    INSERT_API_CALL_LOG();

    return WriteSurfaceFullStride(sysMem, event, stride, m_height, sysMemSize);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Write data from system memory to Surface 2D
//|             picture frame content size equals gfx surface size
//| Arguments :
//|               sysMem      [in]       Pointer to system memory
//|               event       [in]       Pointer to CmEvent
//|               sysMemSize   [out]      Size of Memory need to write
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------

CM_RT_API int32_t CmSurface2DRT::WriteSurface(const unsigned char* sysMem, CmEvent* event, uint64_t sysMemSize)
{
    INSERT_API_CALL_LOG();
    uint32_t        sizePerPixel = 0;
    uint32_t        UVHeight = 0;
    uint32_t        UVpitch = 0;
    uint32_t        UVwidth = 0;
    uint32_t        planes = 0;
    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM));
    inParam.width = m_width;
    inParam.height = m_height;
    inParam.format = m_format;
    // Get planar memory layout information according to the format
    GetPlanarInfomation(inParam, sizePerPixel, UVwidth, UVHeight, UVpitch, planes);
    return WriteSurfaceFullStride(sysMem, event, sizePerPixel * m_width, m_height, sysMemSize);
}


//*-----------------------------------------------------------------------------
//| Purpose:    Hybrid memory copy from  system memory to Surface 2D to with stride
//| Arguments :
//|               sysMem      [in]       Pointer to system memory
//|               event       [in]       Pointer to CmEvent
//|               horizontalStride [in]       Stride in system memory in bytes
//|               verticalStride[in]       Height Stride in system memory in rows
//|               sysMemSize   [in]       Size of Memory need to read
//|               option     [in]       Option to disable/enable hybrid memory copy
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::WriteSurfaceHybridStrides( const unsigned char* sysMem, CmEvent* event, const uint32_t horizontalStride, const uint32_t verticalStride, uint64_t sysMemSize, uint32_t option )
{
    INSERT_API_CALL_LOG();

    int32_t     hr                  = CM_SUCCESS;
    uint32_t    sizePerPixel        = 0;
    uint32_t    updatedHeight       = 0;
    uint32_t    widthInBytes        = 0;
    CmQueue    *cmQueue            = nullptr;
    CmDeviceRT *cmDevice           = nullptr;
    CmQueueRT  *cmQueueRT          = nullptr;
    bool        forceCPUCopy       = COPY_OPTION(option);
    CM_STATUS   status;

    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);

    int32_t result = m_surfaceMgr->GetPixelBytesAndHeight(m_width, m_height, m_format, sizePerPixel, updatedHeight);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Failed to get correct surface info.")
        return result;
    }

    widthInBytes = m_width * sizePerPixel;

    WaitForReferenceFree();   // wait all owner task finished

    if (forceCPUCopy)
    {
        CM_CHK_CMSTATUS_GOTOFINISH(WriteSurfaceFullStride(sysMem, event, horizontalStride, verticalStride, sysMemSize));
    }
    else
    {
        CM_CHK_CMSTATUS_GOTOFINISH(cmDevice->CreateQueue(cmQueue));
        if(IsGPUCopy((void*)sysMem, widthInBytes, m_height, horizontalStride))
        {
            CmEvent *tempEvent = CM_NO_EVENT;
            CM_CHK_CMSTATUS_GOTOFINISH(cmQueue->EnqueueCopyCPUToGPUFullStride(this, sysMem, horizontalStride, verticalStride, CM_FASTCOPY_OPTION_BLOCKING, tempEvent));
        }
        else if (IsUnalignedGPUCopy(widthInBytes, m_height))
        {
            cmQueueRT = static_cast<CmQueueRT *>(cmQueue);
            CM_CHK_CMSTATUS_GOTOFINISH(cmQueueRT->EnqueueUnalignedCopyInternal(this, (unsigned char*)sysMem, horizontalStride, verticalStride, CM_FASTCOPY_CPU2GPU));
        }
        else
        {
            CM_CHK_CMSTATUS_GOTOFINISH(WriteSurfaceFullStride(sysMem, event, horizontalStride, verticalStride, sysMemSize));
        }
    }

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Read data from  Surface 2D to system memory with stride
//|             It copies smaller gfx surface content into larger system memory picture frame
//|             horizontalStride >= surface width * sizePerPixel
//|             verticalStride   == surface height
//| Arguments :
//|               sysMem      [in]       Pointer to system memory
//|               event       [in]       Pointer to CmEvent
//|               Stride       [in]       Stride in system memory in bytes
//|               sysMemSize   [out]      Size of Memory need to read
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::ReadSurfaceStride( unsigned char* sysMem, CmEvent* event, const uint32_t stride, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();

    return ReadSurfaceFullStride(sysMem, event, stride, m_height, sysMemSize);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Read data from  Surface 2D to system memory with stride
//|             It copies smaller gfx surface content into larger system memory picture frame
//|             horizontalStride >= surface width * sizePerPixel
//|             verticalStride   >= surface height
//| Arguments :
//|               sysMem      [in]       Pointer to system memory
//|               event       [in]       Pointer to CmEvent
//|               horizontalStride [in]       Width  Stride in system memory in bytes
//|               verticalStride[in]       Height Stride in system memory in rows
//|               sysMemSize   [out]      Size of Memory need to read
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::ReadSurfaceFullStride( unsigned char* sysMem, CmEvent* event,
                    const uint32_t horizontalStride, const uint32_t verticalStride, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint8_t         *dst = nullptr;
    uint8_t         *src = nullptr;
    uint32_t        sizePerPixel = 0;
    uint32_t        UVHeight = 0;
    uint32_t        UVwidth = 0;
    uint32_t        UVpitch = 0;
    uint32_t        updatedHeight = 0;
    uint32_t        widthInByte = 0;
    uint32_t        pitch = 0;
    uint32_t        row = 0;
    uint32_t        planeHeight = 0;
    uint32_t        planes = 0;
    uint32_t        offset0 = 0;
    uint32_t        offset1 = 0;
    uint32_t        offset2 = 0;
    uint32_t        tmp = 0;
    UNUSED(sysMemSize);

    if (sysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
            return CM_INVALID_ARG_VALUE;
    }

    if (event)
    {
        CmEventRT *eventRT = dynamic_cast<CmEventRT *>(event);
        if (eventRT)
        {
            FlushDeviceQueue(eventRT);
        }
        else
        {
            event->WaitForTaskFinished();
        }
    }

    WaitForReferenceFree();   // wait all owner task finished

    CmDeviceRT * cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    uint32_t platform = 0;
    cmDevice->GetGenPlatform(platform);

    //Lock for surface read/write
    CSync* surfaceLock = cmDevice->GetSurfaceLock();
    CM_ASSERT(surfaceLock);
    CLock locker(*surfaceLock);

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM));
    inParam.width = m_width;
    inParam.height = m_height;
    inParam.handle = m_handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;
    inParam.useGmmOffset = true;

    // Lock Surface Resource
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLock2DResource(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);
    //make sure format is set correctly
    inParam.format = m_format;
    // Get planar memory layout information according to the format
    CM_CHK_CMSTATUS_GOTOFINISH(GetPlanarInfomation(inParam, sizePerPixel, UVwidth, UVHeight, UVpitch, planes));

    // Read copy surface content data
    widthInByte = m_width * sizePerPixel;
    pitch = m_pitch;
    planeHeight = inParam.height;

    // convert gmmlib plane offset to linear offset
    offset0 = inParam.YSurfaceOffset.iSurfaceOffset + (inParam.YSurfaceOffset.iXOffset * sizePerPixel) + (inParam.YSurfaceOffset.iYOffset * pitch);
    offset1 = inParam.USurfaceOffset.iSurfaceOffset + (inParam.USurfaceOffset.iXOffset * sizePerPixel) + (inParam.USurfaceOffset.iYOffset * UVpitch);
    offset2 = inParam.VSurfaceOffset.iSurfaceOffset + (inParam.VSurfaceOffset.iXOffset * sizePerPixel) + (inParam.VSurfaceOffset.iYOffset * UVpitch);

    tmp = offset1;
    if (offset1 > offset2)
    {
        offset1 = offset2;
        offset2 = tmp;
        if (offset1 == 0)
            offset1 = offset2;
    }

    dst = (uint8_t *)sysMem;
    src = (uint8_t *)(inParam.data);
    // Read copy Y plane
    if ((pitch > widthInByte) || (horizontalStride != pitch))
    {
        // scan line copy
        for (row = 0; row < planeHeight; row++)
        {
            CmFastMemCopyFromWC(dst, src, widthInByte, GetCpuInstructionLevel());
            dst += horizontalStride;
            src += pitch;
        }
    }
    else
    {   // block copy
        CmFastMemCopyFromWC(dst, src, pitch * planeHeight, GetCpuInstructionLevel());
    }

    // Read copy 2nd plane
    if (UVHeight > 0)
    {
        if (planes > 1)
        {
            int offsetn = offset1;
            //  Limit the Gmm offset usage to after Gen11
            if ((platform <= IGFX_GEN11LP_CORE) || !inParam.useGmmOffset)
            {
                offsetn = planeHeight * pitch;
            }
            src = (uint8_t *)(inParam.data) + offsetn;
            dst = (uint8_t *)sysMem + (horizontalStride * verticalStride);
            for (row = 0; row < UVHeight; row++)
            {
                CmFastMemCopyFromWC(dst, src, UVwidth * sizePerPixel, GetCpuInstructionLevel());
                dst += horizontalStride * UVwidth / m_width;
                src += UVpitch;
            }
        }

        // Read copy 3rd plane
        if (planes > 2)
        {
            int offsetn = offset2;
            //  Limit the Gmm offset usage to after Gen11
            if ((platform <= IGFX_GEN11LP_CORE) || !inParam.useGmmOffset)
            {
                offsetn = (planeHeight * pitch) + (UVHeight * UVpitch);
            }
            src = (uint8_t *)(inParam.data) + offsetn;
            // calculate system memory frame buffer UV plain width and height with ratio of video surface UV plane vs Y plane
            // UVwidth / m_width  is horizontal ratio;  UVHeight / m_height  is vertical ratio
            // system memory frame buffer 3rd plane offset is Y plane size + 2nd Plane size
            dst = (uint8_t *)sysMem + (horizontalStride * verticalStride) +
                  (horizontalStride * UVwidth / m_width * verticalStride * UVHeight / m_height);

            for (row = 0; row < UVHeight; row++)
            {
                CmFastMemCopyFromWC(dst, src, UVwidth * sizePerPixel, GetCpuInstructionLevel());
                dst += horizontalStride * UVwidth / m_width;
                src += UVpitch;
            }
        }
    }

    //Unlock
    inParam.data = nullptr;
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlock2DResource(cmData->cmHalState, &inParam));

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Read data from  Surface 2D to system memory
//| Arguments :
//|               sysMem      [in]       Pointer to system memory
//|               event       [in]       Pointer to CmEvent
//|               sysMemSize   [out]      Size of Memory need to read
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::ReadSurface(unsigned char* sysMem, CmEvent* event, uint64_t sysMemSize)
{
    INSERT_API_CALL_LOG();
    uint32_t        sizePerPixel = 0;
    uint32_t        UVHeight = 0;
    uint32_t        UVpitch = 0;
    uint32_t        UVwidth = 0;
    uint32_t        planes = 0;
    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM));
    inParam.width = m_width;
    inParam.height = m_height;
    inParam.format = m_format;
    // Get planar memory layout information according to the format
    GetPlanarInfomation(inParam, sizePerPixel, UVwidth, UVHeight, UVpitch, planes);
    return ReadSurfaceFullStride(sysMem, event, sizePerPixel * m_width, m_height, sysMemSize);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Hybrid memory copy from  Surface 2D to system memory with stride
//| Arguments :
//|               sysMem      [out]       Pointer to system memory
//|               event       [in]       Pointer to CmEvent
//|               horizontalStride [in]       Width  Stride in system memory in bytes
//|               verticalStride[in]       Height Stride in system memory in rows
//|               sysMemSize   [in]       Size of Memory need to read
//|               option     [in]       Option to disable/enable hybrid memory copy
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::ReadSurfaceHybridStrides(unsigned char* sysMem, CmEvent* event, const uint32_t horizontalStride, const uint32_t verticalStride, uint64_t sysMemSize, uint32_t option)
{
    INSERT_API_CALL_LOG();

    int32_t     hr = CM_SUCCESS;
    uint32_t    sizePerPixel = 0;
    uint32_t    updatedHeight = 0;
    uint32_t    widthInBytes = 0;
    CmQueue    *cmQueue = nullptr;
    CmDeviceRT *cmDevice = nullptr;
    CmQueueRT  *cmQueueRT = nullptr;
    bool        forceCPUCopy = COPY_OPTION(option);
    CM_STATUS   status;

    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);

    int32_t result = m_surfaceMgr->GetPixelBytesAndHeight(m_width, m_height, m_format, sizePerPixel, updatedHeight);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Failed to get correct surface info.")
            return result;
    }

    widthInBytes = m_width * sizePerPixel;

    WaitForReferenceFree();   // wait all owner task finished

    if (forceCPUCopy)
    {
        CM_CHK_CMSTATUS_GOTOFINISH(ReadSurfaceFullStride(sysMem, event, horizontalStride, verticalStride, sysMemSize));
    }
    else
    {
        CM_CHK_CMSTATUS_GOTOFINISH(cmDevice->CreateQueue(cmQueue));
        if (IsGPUCopy((void*)sysMem, widthInBytes, m_height, horizontalStride))
        {
            CmEvent *tempEvent = CM_NO_EVENT;
            CM_CHK_CMSTATUS_GOTOFINISH(cmQueue->EnqueueCopyGPUToCPUFullStride(this, sysMem, horizontalStride, verticalStride, CM_FASTCOPY_OPTION_BLOCKING, tempEvent));
        }
        else if (IsUnalignedGPUCopy(widthInBytes, m_height))
        {
            cmQueueRT = static_cast<CmQueueRT *>(cmQueue);
            CM_CHK_CMSTATUS_GOTOFINISH(cmQueueRT->EnqueueUnalignedCopyInternal(this, (unsigned char*)sysMem, horizontalStride, verticalStride, CM_FASTCOPY_GPU2CPU));
        }
        else
        {
            CM_CHK_CMSTATUS_GOTOFINISH(ReadSurfaceFullStride(sysMem, event, horizontalStride, verticalStride, sysMemSize));
        }
    }

finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Let CM know that the UMD resource of current CmSurface2D
//              wrapper was changed. This change may happen at the CmSurface2D
//              creation and destroy time or changed from outside of CM. This
//              should be called immediately after the UMD resource changed.
//
//| Arguments :
//|             umdResource       [in]  Pointer to the UMD resource for the CM
//|                                     wrapper. Set it to nullptr when the
//|                                     third party MosResource was deleted
//|                                     before the CmSurface2D.
//|             updateMosResource [in]  1: will update the MosResource.
//|                                     0: will not update the MosResource.
//|             mosResource      [in]  Pointer to the new valid MosResource
//|                                     that the CMSurface2D will be based on.
//|                                     Do not set this parameter if the
//|                                     MosResource is already deleted.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CMRT_UMD_API int32_t
CmSurface2DRT::NotifyUmdResourceChanged(UMD_RESOURCE umdResource,
                                        int updateMosResource,
                                        PMOS_RESOURCE mosResource)
{
    m_umdResource = umdResource;

    //
    if ( updateMosResource )
    {
        m_surfaceMgr->UpdateSurface2DTableMosResource( m_handle, mosResource );
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the handle of  CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurface2DRT::GetHandle( uint32_t& handle)
{
    handle = m_handle;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the handle of  CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurface2DRT::GetIndexFor2D( uint32_t& index )
{
    index = m_handle;
    return CM_SUCCESS;
}

int32_t CmSurface2DRT::SetSurfaceProperties(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format)
{
    if (format == CM_SURFACE_FORMAT_NV12)
    {
        m_pitch = MOS_ALIGN_CEIL(width * m_pitch /m_width , 2);
    }
    m_width = width;
    m_height = height;
    m_format = format;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the description of surface 2d, width,height,format and the size of pixel .
//| Arguments :
//|               width         [out]     Reference to  width of surface
//|               height        [out]     Reference to  height of surface
//|               format        [out]     Reference to  format of surface
//|               sizeperpixel  [out]     Reference to  the pixel's size in bytes
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::GetSurfaceDesc(uint32_t &width, uint32_t &height, CM_SURFACE_FORMAT &format,uint32_t &sizeperpixel)
{

    int ret = CM_SUCCESS;
    uint32_t updatedHeight = 0 ;

    width  = m_width;
    height = m_height;
    format = m_format;

    // Get size per pixel
    ret = m_surfaceMgr->GetPixelBytesAndHeight(width,  height,  format,  sizeperpixel, updatedHeight);

    return ret;
}

CM_RT_API int32_t CmSurface2DRT::InitSurface(const unsigned int initValue, CmEvent* event)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE                      hr          = CM_SUCCESS;
    CmDeviceRT*                         cmDevice   = nullptr;
    PCM_CONTEXT_DATA                    cmData     = nullptr;
    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM  inParam;
    uint32_t                            pitch       = 0;
    uint32_t                            *surf       = nullptr;
    uint32_t                            widthInBytes = 0;

    if( event )
    {
        CmEventRT *eventRT = dynamic_cast<CmEventRT *>(event);
        if (eventRT)
        {
            FlushDeviceQueue(eventRT);
        }
        else
        {
            event->WaitForTaskFinished();
        }
    }

    WaitForReferenceFree();   // wait all owner task finished

    uint32_t sizePerPixel  = 0;
    uint32_t updatedHeight = 0;
    CM_CHK_CMSTATUS_GOTOFINISH(m_surfaceMgr->GetPixelBytesAndHeight(m_width, m_height, m_format, sizePerPixel, updatedHeight));

    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM ) );
    inParam.width = m_width;
    inParam.height = m_height;
    inParam.handle = m_handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLock2DResource(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);

    pitch = inParam.pitch;
    surf = ( uint32_t *)inParam.data;

    widthInBytes = m_width * sizePerPixel;
    if(widthInBytes != pitch)
    {
        for (uint32_t i=0; i < updatedHeight; i++)
        {
            if (widthInBytes % sizeof(uint32_t) == 0)
            {
                CmDwordMemSet(surf, initValue, widthInBytes);
            }
            else
            {
                CmDwordMemSet(surf, initValue, widthInBytes + sizeof(uint32_t));
            }

           surf += (pitch >> 2); // divide by 4 byte to dword
        }
    }
    else
    {
        CmDwordMemSet(surf, initValue, pitch * updatedHeight);
    }

    //Unlock Surface2D
    inParam.data = nullptr;
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlock2DResource(cmData->cmHalState, &inParam));

finish:
    return hr;
}

int32_t CmSurface2DRT::SetMemoryObjectControl( MEMORY_OBJECT_CONTROL memCtrl, MEMORY_TYPE memType, uint32_t age)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint16_t mocs = 0;

    CmSurface::SetMemoryObjectControl( memCtrl, memType, age );

    CmDeviceRT *cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    mocs = (m_memObjCtrl.mem_ctrl << 8) | (m_memObjCtrl.mem_type<<4) | m_memObjCtrl.age;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnSetSurfaceMOCS(cmData->cmHalState, m_handle, mocs, ARG_KIND_SURFACE_2D));

finish:
    return hr;
}

CM_RT_API int32_t CmSurface2DRT::SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL memCtrl)
{
    return SetMemoryObjectControl(memCtrl, CM_USE_PTE, 0);
}

int32_t CmSurface2DRT::Create2DAlias(SurfaceIndex* & aliasIndex)
{
    INSERT_API_CALL_LOG();

    uint32_t surfArraySize = 0;

    if( m_numAliases < CM_HAL_MAX_NUM_2D_ALIASES )
    {
        uint32_t origIndex = m_index->get_data();
        m_surfaceMgr->GetSurfaceArraySize(surfArraySize);
        uint32_t newIndex = origIndex + ( (m_numAliases + 1) * surfArraySize);
        m_aliasIndexes[m_numAliases] = MOS_New(SurfaceIndex, newIndex);
        if( m_aliasIndexes[m_numAliases] )
        {
            aliasIndex = m_aliasIndexes[m_numAliases];
            m_numAliases++;
            return CM_SUCCESS;
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Failed to create surface 2d alias due to out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    else
    {
        return CM_EXCEED_MAX_NUM_2D_ALIASES;
    }
}

CM_RT_API int32_t CmSurface2DRT::GetNumAliases(uint32_t& numAliases)
{
    numAliases = m_numAliases;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface2DRT::SetSurfaceStateParam( SurfaceIndex *surfIndex, const CM_SURFACE2D_STATE_PARAM *surfStateParam )
{
    CM_RETURN_CODE  hr = CM_SUCCESS;
    CmDeviceRT * cmDevice = nullptr;
    PCM_CONTEXT_DATA cmData = nullptr;
    CM_HAL_SURFACE2D_SURFACE_STATE_PARAM inParam;
    uint32_t aliasIndex = 0;

    m_surfaceMgr->GetCmDevice( cmDevice );
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmDevice);
    cmData = ( PCM_CONTEXT_DATA )cmDevice->GetAccelData();
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmData);
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmData->cmHalState);

    CmSafeMemSet( &inParam, 0, sizeof( inParam ) );
    inParam.width       = surfStateParam->width;
    inParam.height      = surfStateParam->height;
    if (surfStateParam->format)
    {
        inParam.format = surfStateParam->format;
    }
    inParam.depth       = surfStateParam->depth;
    inParam.pitch       = surfStateParam->pitch;
    inParam.memoryObjectControl   = surfStateParam->memory_object_control;
    inParam.surfaceXOffset        = surfStateParam->surface_x_offset;
    inParam.surfaceYOffset        = surfStateParam->surface_y_offset;

    if (surfIndex)
    {
        aliasIndex = surfIndex->get_data();
    }
    else
    {
        aliasIndex = m_index->get_data();
    }

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR( cmData->cmHalState->pfnSet2DSurfaceStateParam(cmData->cmHalState, &inParam, aliasIndex, m_handle) );

finish:
    return hr;
}

CMRT_UMD_API int32_t CmSurface2DRT::SetReadSyncFlag(bool readSync, CmQueue *cmQueue)
{
    int32_t hr = CM_SUCCESS;

    CmDeviceRT *cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    CmQueueRT *cmQueueRT = static_cast<CmQueueRT *>(cmQueue);
    CM_CHK_NULL_RETURN_CMERROR(cmQueueRT);

    hr = cmData->cmHalState->pfnSetSurfaceReadFlag(cmData->cmHalState, m_handle, readSync,
                                                   (MOS_GPU_CONTEXT)cmQueueRT->GetQueueOption().GPUContext);

    if( FAILED(hr) )
    {
        CM_ASSERTMESSAGE("Error: Set read sync flag failure.")
        return CM_FAILURE;
    }

    return hr;
}

void CmSurface2DRT::Log(std::ostringstream &oss)
{
#if CM_LOG_ON
    oss << " Surface2D Info "
        << " Width:" << m_width
        << " Height:"<< m_height
        << " Format:"<< GetFormatString(m_format)
        << " Pitch:" << m_pitch
        << " Handle:" << m_handle
        << " SurfaceIndex:" << m_index->get_data()
        << " IsCmCreated:"<<m_isCmCreated
        << std::endl;
#endif
}

void CmSurface2DRT::DumpContent(uint32_t kernelNumber, char *kernelName, int32_t taskId, uint32_t argIndex)
{
#if MDF_SURFACE_CONTENT_DUMP
    std::ostringstream         outputFileName;
    static uint32_t            surface2DDumpNumber = 0;
    char                       fileNamePrefix[MAX_PATH];
    std::ofstream              outputFileStream;

    outputFileName << "t_" << taskId
        << "_k_" << kernelNumber
        << "_" << kernelName
        << "_argi_" << argIndex
        << "_surf2d_surfi_"<< m_index->get_data()
        << "_w_" << m_width
        << "_h_" << m_height
        << "_p_" << m_pitch
        << "_f_" << GetFormatString(m_format)
        << "_" << surface2DDumpNumber;

    GetLogFileLocation(outputFileName.str().c_str(), fileNamePrefix);

    // Open file
    outputFileStream.open(fileNamePrefix, std::ios::app);
    CM_ASSERT(outputFileStream);

    CmDeviceRT * cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_ASSERT(cmDevice);
    
    CSync* surfaceLock = cmDevice->GetSurfaceLock();
    CM_ASSERT(surfaceLock);
    CLock locker(*surfaceLock);
    uint32_t        sizePerPixel = 0;
    uint32_t        updatedHeight = 0;
    uint32_t        surfaceSize = 0;
    uint32_t        widthInByte = 0;
    uint8_t         *dst = nullptr;
    uint8_t         *surf = nullptr;
    m_surfaceMgr->GetPixelBytesAndHeight(m_width, m_height, m_format, sizePerPixel, updatedHeight);
    surfaceSize = m_width*sizePerPixel*updatedHeight;
    widthInByte = m_width * sizePerPixel;

    std::vector<char>surface(surfaceSize);

    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_ASSERT(cmData);
    CM_ASSERT(cmData->cmHalState);

    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM));
    inParam.width = m_width;
    inParam.height = m_height;
    inParam.handle = m_handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;
    cmData->cmHalState->pfnLock2DResource(cmData->cmHalState, &inParam);
    if (inParam.data == nullptr)
        return;
    dst = (uint8_t *)&surface[0];
    surf = (uint8_t *)(inParam.data);
    if (m_pitch != widthInByte)
    {
        for (uint32_t row = 0; row < updatedHeight; row++)
        {
            CmFastMemCopyFromWC(dst, surf, widthInByte, GetCpuInstructionLevel());
            surf += m_pitch;
            dst += widthInByte;
        }
    }
    else
    {
        CmFastMemCopyFromWC((unsigned char *)&surface[0], surf, m_pitch * updatedHeight, GetCpuInstructionLevel());
    }
    inParam.data = nullptr;
    cmData->cmHalState->pfnUnlock2DResource(cmData->cmHalState, &inParam);

    outputFileStream.write(&surface[0], surfaceSize);
    outputFileStream.close();
    surface2DDumpNumber++;
#endif
}

CM_RT_API int32_t CmSurface2DRT::SetProperty(CM_FRAME_TYPE frameType)
{
    m_frameType = frameType;
    m_surfaceMgr->UpdateSurface2DTableFrameType(m_handle, frameType);
    return CM_SUCCESS;
}

int32_t CmSurface2DRT::UpdateResource(MOS_RESOURCE *resource)
{
    // get index
    int index = m_index->get_data();
    return m_surfaceMgr->UpdateSurface2D(resource, index, m_handle);
}

int32_t CmSurface2DRT::UpdateSurfaceProperty(uint32_t width, uint32_t height, uint32_t pitch, CM_SURFACE_FORMAT format)
{
    int result = m_surfaceMgr->Surface2DSanityCheck(width, height, format);
    if( result != CM_SUCCESS )
    {
        CM_ASSERTMESSAGE("Error: Surface 2D sanity check failure.");
        return result;
    }
    m_width = width;
    m_height = height;
    m_pitch = pitch;
    m_format = format;
    return CM_SUCCESS;
}

}
