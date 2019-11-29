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
//! \file      cm_surface_3d_rt.cpp
//! \brief     Contains CmSurface3DRT implementations.
//!

#include "cm_surface_3d_rt.h"

#include "cm_device_rt.h"
#include "cm_event_rt.h"
#include "cm_mem.h"
#include "cm_surface_manager.h"
#include "cm_hal.h"

namespace CMRT_UMD
{
int32_t CmSurface3DRT::Create( uint32_t index, uint32_t handle,
                            uint32_t width, uint32_t height, uint32_t depth,
                            CM_SURFACE_FORMAT format,
                            CmSurfaceManager* surfaceManager, CmSurface3DRT* &surface )
{
    int32_t result = CM_SUCCESS;

    surface = new (std::nothrow) CmSurface3DRT( handle, width, height, depth, format, surfaceManager );
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
        CM_ASSERTMESSAGE("Error: Failed to create CmSurface3D due to out of system memory.")
        result = CM_OUT_OF_HOST_MEMORY;
    }

    return result;
}

CmSurface3DRT::CmSurface3DRT( uint32_t handle,
    uint32_t width,
    uint32_t height,
    uint32_t depth,
    CM_SURFACE_FORMAT format,
    CmSurfaceManager* surfaceManager ):
    CmSurface( surfaceManager,true ),
    m_handle( handle ),
    m_width( width ),
    m_height( height ),
    m_depth( depth ),
    m_format( format )
{
    CmSurface::SetMemoryObjectControl(MEMORY_OBJECT_CONTROL_UNKNOW, CM_USE_PTE, 0);
}

CmSurface3DRT::~CmSurface3DRT( void )
{
}

int32_t CmSurface3DRT::Initialize( uint32_t index )
{
    return CmSurface::Initialize( index );
}

int32_t CmSurface3DRT::GetHandle( uint32_t& handle)
{
    handle = m_handle;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface3DRT::WriteSurface( const unsigned char* sysMem,
                                                CmEvent* event,
                                                uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();
    CM_RETURN_CODE  hr              = CM_SUCCESS;
    uint64_t        uSizeInBytes    = 0;
    uint32_t        uWidthInBytes   = 0;
    uint8_t         *tempDst        = nullptr;
    uint8_t         *tempSrc        = nullptr;
    uint8_t         *rPlane         = nullptr;

    if(sysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
        return CM_INVALID_ARG_VALUE;
    }

    uint32_t pixel = 0;
    switch (m_format)
    {
        case CM_SURFACE_FORMAT_X8R8G8B8:
        case CM_SURFACE_FORMAT_A8R8G8B8:
            pixel = 4;
            break;
        case CM_SURFACE_FORMAT_A16B16G16R16:
            pixel = 8;
            break;
        default:
            CM_ASSERTMESSAGE("Error: Unsupported surface format.")
            return CM_INVALID_ARG_VALUE;
    }

    uSizeInBytes = m_width * m_height * m_depth * pixel;
    if (sysMemSize < uSizeInBytes)
    {
        CM_ASSERTMESSAGE("Error: Invalid copy size.")
        return CM_INVALID_ARG_VALUE;
    }

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( event )
    {
        CmEventRT *eventRT = static_cast<CmEventRT *>(event);
        FlushDeviceQueue( eventRT );  // wait specific owner task finished
    }

    WaitForReferenceFree();   // wait all owner task finished

    CM_HAL_3DRESOURCE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_3DRESOURCE_PARAM ) );

    CmDeviceRT* device = nullptr;
    m_surfaceMgr->GetCmDevice( device );
    CM_ASSERT( device );
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)device->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    inParam.handle = m_handle;
    inParam.data = (void*)sysMem; //Any non-nullptr value will work
    inParam.width = m_width;
    inParam.height = m_height;
    inParam.depth = m_depth;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    // Lock 3D Resource
    // Lock may fail due to the out of memory/out of page-in in KMD.
    // Touch queue for the buffer/surface data release
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLock3DResource(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);

    uWidthInBytes = inParam.width * pixel;

    //Copy Resource
    tempDst    = (uint8_t*)inParam.data;
    tempSrc    = (uint8_t*)sysMem;
    rPlane     = (uint8_t*)inParam.data;

    // Only use Qpitch when Qpitch is supported by HW
    if (inParam.qpitchEnabled)
    {
        if (inParam.pitch == uWidthInBytes && inParam.qpitch == inParam.height)
        {
            CmFastMemCopyWC(tempDst, tempSrc, (size_t)uSizeInBytes);
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                tempDst = rPlane;
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyWC(tempDst, tempSrc, uWidthInBytes);
                    tempSrc += uWidthInBytes;
                    tempDst += inParam.pitch;
                }
                rPlane += inParam.qpitch * inParam.pitch;
            }
        }
    }
    else
    {
        if (inParam.pitch == uWidthInBytes)
        {
            CmFastMemCopyWC(tempDst, tempSrc, (size_t)uSizeInBytes);
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyWC(tempDst, tempSrc, uWidthInBytes);
                    tempSrc += uWidthInBytes;
                    tempDst += inParam.pitch;
                }
            }
        }
    }

    // unlock 3D resource
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlock3DResource(cmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

CM_RT_API int32_t CmSurface3DRT::ReadSurface( unsigned char* sysMem, CmEvent* event, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();
    CM_RETURN_CODE  hr              = CM_SUCCESS;
    uint64_t        uSizeInBytes    = 0;
    uint32_t        uWidthInBytes   = 0;
    uint8_t         *tempDst        = nullptr;
    uint8_t         *tempSrc        = nullptr;
    uint8_t         *rPlane         = nullptr;

    if(sysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
        return CM_INVALID_ARG_VALUE;
    }

    uint32_t pixel = 0;
    switch (m_format)
    {
        case CM_SURFACE_FORMAT_X8R8G8B8:
        case CM_SURFACE_FORMAT_A8R8G8B8:
            pixel = 4;
            break;
        case CM_SURFACE_FORMAT_A16B16G16R16:
            pixel = 8;
            break;
        default:
            CM_ASSERTMESSAGE("Error: Unsupported surface format.")
            return CM_INVALID_ARG_VALUE;
    }

    uSizeInBytes = m_width * m_height * m_depth * pixel;
    if (sysMemSize < uSizeInBytes)
    {
        CM_ASSERTMESSAGE("Error: Invalid copy size.")
        return CM_INVALID_ARG_VALUE;
    }

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( event )
    {
        CmEventRT *eventRT = static_cast<CmEventRT *>(event);
        int hr = FlushDeviceQueue( eventRT );  // wait specific owner task finished
        {
            CM_ASSERTMESSAGE("Fail to flush queue.");
            return hr;
        }
    }

    WaitForReferenceFree();   // wait all owner task finished

    CM_HAL_3DRESOURCE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_3DRESOURCE_PARAM ) );

    CmDeviceRT* device = nullptr;
    m_surfaceMgr->GetCmDevice( device );
    CM_ASSERT( device );
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)device->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    inParam.handle = m_handle;
    inParam.data = (void*)sysMem; //Any non-nullptr value will work
    inParam.width = m_width;
    inParam.height = m_height;
    inParam.depth = m_depth;
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;

    // Lock 3D Resource
    // Lock may fail due to the out of memory/out of page-in in KMD.
    // Touch queue for the buffer/surface data release
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLock3DResource(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);

    uWidthInBytes = inParam.width * pixel;

    //Copy Resource
    tempDst    = (uint8_t*)sysMem;
    tempSrc    = (uint8_t*)inParam.data;
    rPlane     = (uint8_t*)inParam.data;

    // Only use Qpitch when Qpitch is supported by HW
    if (inParam.qpitchEnabled)
    {
        if (inParam.pitch == uWidthInBytes && inParam.qpitch == inParam.height)
        {
            CmFastMemCopyFromWC(tempDst, tempSrc, (size_t)uSizeInBytes, GetCpuInstructionLevel());
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                tempSrc = rPlane;
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyFromWC(tempDst, tempSrc, uWidthInBytes, GetCpuInstructionLevel());
                    tempSrc += inParam.pitch;
                    tempDst += uWidthInBytes;
                }
                rPlane += inParam.qpitch * inParam.pitch;
            }
        }
    }
    else
    {
        if (inParam.pitch == uWidthInBytes)
        {
            CmFastMemCopyFromWC(tempDst, tempSrc, (size_t)uSizeInBytes, GetCpuInstructionLevel());
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyFromWC(tempDst, tempSrc, uWidthInBytes, GetCpuInstructionLevel());
                    tempSrc += inParam.pitch;
                    tempDst += uWidthInBytes;
                }
            }
        }
    }

    // unlock 3D resource
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlock3DResource(cmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

CM_RT_API int32_t CmSurface3DRT::GetIndex( SurfaceIndex*& index )
{
    index = m_index;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Properties of  CmSurface3DRT
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmSurface3DRT::GetProperties( uint32_t& width,  uint32_t& height, uint32_t& depth, CM_SURFACE_FORMAT& format)
{
    width  = m_width;
    height = m_height;
    depth = m_depth;
    format = m_format;
    return CM_SUCCESS;
}

int32_t CmSurface3DRT::SetProperties( uint32_t width,  uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format)
{
    m_width  = width;
    m_height = height;
    m_depth  = depth;
    m_format = format;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface3DRT::InitSurface(const uint32_t initValue, CmEvent* event)
{
    INSERT_API_CALL_LOG();
    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint32_t        uSizeInBytes = 0;
    uint32_t        uWidthInBytes = 0;
    uint8_t         *tempDst = nullptr;
    uint8_t         *rPlane = nullptr;

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( event )
    {
        CmEventRT *eventRT = static_cast<CmEventRT *>(event);
        FlushDeviceQueue( eventRT );  // wait specific owner task finished
    }

    WaitForReferenceFree();   // wait all owner task finished

    CM_HAL_3DRESOURCE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_3DRESOURCE_PARAM ) );

    CmDeviceRT* device = nullptr;
    m_surfaceMgr->GetCmDevice( device );
    CM_ASSERT( device );
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)device->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    uint32_t sizePerPixel = 0;
    uint32_t updatedHeight = 0;
    CM_CHK_CMSTATUS_GOTOFINISH(m_surfaceMgr->GetPixelBytesAndHeight(m_width, m_height, m_format, sizePerPixel, updatedHeight));

    inParam.handle = m_handle;
    inParam.data = (void*)0x44; //Any non-nullptr value will work
    inParam.width = m_width;
    inParam.height = m_height;
    inParam.depth = m_depth;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnLock3DResource(cmData->cmHalState, &inParam));
    CM_CHK_NULL_GOTOFINISH_CMERROR(inParam.data);

    uSizeInBytes = inParam.width * inParam.height * inParam.depth * sizePerPixel;
    uWidthInBytes = inParam.width * sizePerPixel;

    //Copy Resource
    tempDst = (uint8_t*)inParam.data;
    rPlane  = (uint8_t*)inParam.data;

    // Only use Qpitch when Qpitch is supported by HW
    if (inParam.qpitchEnabled)
    {
        if (inParam.pitch == uWidthInBytes && inParam.qpitch == inParam.height)
        {
            CmDwordMemSet(tempDst, initValue, uSizeInBytes);
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                tempDst = rPlane;
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmDwordMemSet(tempDst, initValue, uWidthInBytes);
                    tempDst += inParam.pitch;
                }
                rPlane += inParam.qpitch * inParam.pitch;
            }
        }
    }
    else
    {
        if (inParam.pitch == uWidthInBytes)
        {
            CmDwordMemSet(tempDst, initValue, uSizeInBytes);
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmDwordMemSet(tempDst, initValue, uWidthInBytes);
                    tempDst += inParam.pitch;
                }
            }
        }
    }

    // unlock
    inParam.data = nullptr;
    inParam.handle = m_handle;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnUnlock3DResource(cmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

int32_t CmSurface3DRT::SetMemoryObjectControl( MEMORY_OBJECT_CONTROL memCtrl, MEMORY_TYPE memType, uint32_t age)
{
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

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnSetSurfaceMOCS(cmData->cmHalState, m_handle, mocs, ARG_KIND_SURFACE_3D));

finish:
    return hr;
}

CM_RT_API int32_t CmSurface3DRT::SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL memCtrl)
{
    return SetMemoryObjectControl(memCtrl, CM_USE_PTE, 0);
}

CMRT_UMD_API int32_t CmSurface3DRT::SetResourceUsage(const MOS_HW_RESOURCE_DEF mosUsage)
{
    int32_t  hr = CM_SUCCESS;
    uint16_t mocs = 0;
    hr = CmSurface::SetResourceUsage(mosUsage);

    CmDeviceRT *cmDevice = nullptr;
    m_surfaceMgr->GetCmDevice(cmDevice);
    CM_CHK_NULL_RETURN_CMERROR(cmDevice);
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)cmDevice->GetAccelData();
    CM_CHK_NULL_RETURN_CMERROR(cmData);
    CM_CHK_NULL_RETURN_CMERROR(cmData->cmHalState);

    mocs = (m_memObjCtrl.mem_ctrl << 8) | (m_memObjCtrl.mem_type << 4) | m_memObjCtrl.age;
    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(cmData->cmHalState->pfnSetSurfaceMOCS(cmData->cmHalState, m_handle, mocs, ARG_KIND_SURFACE_3D));
finish:
    return hr;
}


void CmSurface3DRT::Log(std::ostringstream &oss)
{
#if CM_LOG_ON
    oss << " Surface3D Info "
        << " Width:" << m_width
        << " Height:" << m_height
        << " Depth:" << m_depth
        << " Format:" << GetFormatString(m_format)
        << " Handle:" << m_handle
        << " SurfaceIndex:" << m_index->get_data()
        << " IsCmCreated:" << m_isCmCreated
        << std::endl;
#endif
}

void CmSurface3DRT::DumpContent(uint32_t kernelNumber, char *kernelName, int32_t taskId, uint32_t argIndex, uint32_t vectorIndex)
{
#if MDF_SURFACE_CONTENT_DUMP
    std::ostringstream outputFileName;
    static uint32_t    surface3DDumpNumber = 0;
    char               fileNamePrefix[MAX_PATH] = { 0 };
    std::ofstream      outputFileStream;

    outputFileName << "t_" << taskId
        << "_k_" << kernelNumber
        << "_" << kernelName
        << "_argi_" << argIndex
        << "_vector_index_" << vectorIndex
        << "_surf2d_surfi_" << m_index->get_data()
        << "_w_" << m_width
        << "_h_" << m_height
        << "_d_" << m_depth
        << "_f_" << GetFormatString(m_format)
        << "_" << surface3DDumpNumber;

    GetLogFileLocation(outputFileName.str().c_str(), fileNamePrefix);   

    // Open file
    outputFileStream.open(fileNamePrefix, std::ios::app | std::ios::binary);
    CM_ASSERT(outputFileStream);

    uint32_t        surfaceSize = 0;
    uint32_t        sizePerPixel = 0;
    uint32_t        updatedHeight = 0;
    uint32_t        uWidthInBytes = 0;
    uint8_t         *tempDst = nullptr;
    uint8_t         *tempSrc = nullptr;
    uint8_t         *rPlane = nullptr;
    m_surfaceMgr->GetPixelBytesAndHeight(m_width, m_height, m_format, sizePerPixel, updatedHeight);
    surfaceSize = m_width * updatedHeight * m_depth * sizePerPixel;
    uWidthInBytes = m_width * sizePerPixel;
    std::vector<char>surface(surfaceSize);

    CM_HAL_3DRESOURCE_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_3DRESOURCE_PARAM));

    CmDeviceRT* device = nullptr;
    m_surfaceMgr->GetCmDevice(device);
    CM_ASSERT(device);
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)device->GetAccelData();
    CM_ASSERT(cmData);
    CM_ASSERT(cmData->cmHalState);

    inParam.handle = m_handle;
    inParam.data = (void*)&surface[0];
    inParam.width = m_width;
    inParam.height = m_height;
    inParam.depth = m_depth;
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;

    cmData->cmHalState->pfnLock3DResource(cmData->cmHalState, &inParam);
    if (inParam.data == nullptr)
        return;
    tempDst = (uint8_t*)&surface[0];
    tempSrc = (uint8_t*)inParam.data;
    rPlane = (uint8_t*)inParam.data;
    if (inParam.qpitchEnabled)
    {
        if (inParam.pitch == uWidthInBytes && inParam.qpitch == inParam.height)
        {
            CmFastMemCopyFromWC(tempDst, tempSrc, (size_t)surfaceSize, GetCpuInstructionLevel());
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                tempSrc = rPlane;
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyFromWC(tempDst, tempSrc, uWidthInBytes, GetCpuInstructionLevel());
                    tempSrc += inParam.pitch;
                    tempDst += uWidthInBytes;
                }
                rPlane += inParam.qpitch * inParam.pitch;
            }
        }
    }
    else
    {
        if (inParam.pitch == uWidthInBytes)
        {
            CmFastMemCopyFromWC(tempDst, tempSrc, (size_t)surfaceSize, GetCpuInstructionLevel());
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyFromWC(tempDst, tempSrc, uWidthInBytes, GetCpuInstructionLevel());
                    tempSrc += inParam.pitch;
                    tempDst += uWidthInBytes;
                }
            }
        }
    }
    cmData->cmHalState->pfnUnlock3DResource(cmData->cmHalState, &inParam);

    outputFileStream.write(&surface[0], surfaceSize);
    outputFileStream.close();
    surface3DDumpNumber++;
#endif
}
}
