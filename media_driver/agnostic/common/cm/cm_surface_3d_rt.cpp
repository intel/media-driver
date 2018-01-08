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
                            CmSurfaceManager* pSurfaceManager, CmSurface3DRT* &pSurface )
{
    int32_t result = CM_SUCCESS;

    pSurface = new (std::nothrow) CmSurface3DRT( handle, width, height, depth, format, pSurfaceManager );
    if( pSurface )
    {

        result = pSurface->Initialize( index );
        if( result != CM_SUCCESS )
        {
            CmSurface* pBaseSurface = pSurface;
            CmSurface::Destroy( pBaseSurface );
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
    CmSurfaceManager* pSurfaceManager ):
    CmSurface( pSurfaceManager,true ),
    m_Handle( handle ),
    m_Width( width ),
    m_Height( height ),
    m_Depth( depth ),
    m_Format( format )
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
    handle = m_Handle;
    return CM_SUCCESS;
}


CM_RT_API int32_t CmSurface3DRT::WriteSurface( const unsigned char* pSysMem,
                                                CmEvent* pEvent,
                                                uint64_t sysMemSize )
{
    CM_RETURN_CODE  hr              = CM_SUCCESS;
    uint64_t        uSizeInBytes    = 0;
    uint32_t        uWidthInBytes   = 0;
    uint8_t         *pTempDst        = nullptr;
    uint8_t         *pTempSrc        = nullptr;
    uint8_t         *pRPlane         = nullptr;

    if(pSysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
        return CM_INVALID_ARG_VALUE;
    }

    uint32_t pixel = 0;
    switch (m_Format)
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

    uSizeInBytes = m_Width * m_Height * m_Depth * pixel;
    if (sysMemSize < uSizeInBytes)
    {
        CM_ASSERTMESSAGE("Error: Invalid copy size.")
        return CM_INVALID_ARG_VALUE;
    }

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        FlushDeviceQueue( pEventRT );  // wait specific owner task finished
    }

    WaitForReferenceFree();   // wait all owner task finished

    CM_HAL_3DRESOURCE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_3DRESOURCE_PARAM ) );

    CmDeviceRT* pCmDev = nullptr;
    m_SurfaceMgr->GetCmDevice( pCmDev );
    CM_ASSERT( pCmDev );
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDev->GetAccelData();

    inParam.handle = m_Handle;
    inParam.data = (void*)pSysMem; //Any non-nullptr value will work
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.depth = m_Depth;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    // Lock 3D Resource
    // Lock may fail due to the out of memory/out of page-in in KMD.
    // Touch queue for the buffer/surface data release
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnLock3DResource(pCmData->cmHalState, &inParam));
    CMCHK_NULL(inParam.data);

    uWidthInBytes = inParam.width * pixel;

    //Copy Resource
    pTempDst    = (uint8_t*)inParam.data;
    pTempSrc    = (uint8_t*)pSysMem;
    pRPlane     = (uint8_t*)inParam.data;

    // Only use Qpitch when Qpitch is supported by HW
    if (inParam.qpitchEnabled)
    {
        if (inParam.pitch == uWidthInBytes && inParam.qpitch == inParam.height)
        {
            CmFastMemCopyWC(pTempDst, pTempSrc, (size_t)uSizeInBytes);
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                pTempDst = pRPlane;
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyWC(pTempDst, pTempSrc, uWidthInBytes);
                    pTempSrc += uWidthInBytes;
                    pTempDst += inParam.pitch;
                }
                pRPlane += inParam.qpitch * inParam.pitch;
            }
        }
    }
    else
    {
        if (inParam.pitch == uWidthInBytes)
        {
            CmFastMemCopyWC(pTempDst, pTempSrc, (size_t)uSizeInBytes);
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyWC(pTempDst, pTempSrc, uWidthInBytes);
                    pTempSrc += uWidthInBytes;
                    pTempDst += inParam.pitch;
                }
            }
        }
    }

    // unlock 3D resource
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnUnlock3DResource(pCmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}


CM_RT_API int32_t CmSurface3DRT::ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize )
{
    CM_RETURN_CODE  hr              = CM_SUCCESS;
    uint64_t        uSizeInBytes    = 0;
    uint32_t        uWidthInBytes   = 0;
    uint8_t         *pTempDst        = nullptr;
    uint8_t         *pTempSrc        = nullptr;
    uint8_t         *pRPlane         = nullptr;

    if(pSysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
        return CM_INVALID_ARG_VALUE;
    }

    uint32_t pixel = 0;
    switch (m_Format)
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

    uSizeInBytes = m_Width * m_Height * m_Depth * pixel;
    if (sysMemSize < uSizeInBytes)
    {
        CM_ASSERTMESSAGE("Error: Invalid copy size.")
        return CM_INVALID_ARG_VALUE;
    }

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        int hr = FlushDeviceQueue( pEventRT );  // wait specific owner task finished
        {
            CM_ASSERTMESSAGE("Fail to flush queue.");
            return hr;
        }
    }

    WaitForReferenceFree();   // wait all owner task finished

    CM_HAL_3DRESOURCE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_3DRESOURCE_PARAM ) );

    CmDeviceRT* pCmDev = nullptr;
    m_SurfaceMgr->GetCmDevice( pCmDev );
    CM_ASSERT( pCmDev );
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDev->GetAccelData();

    inParam.handle = m_Handle;
    inParam.data = (void*)pSysMem; //Any non-nullptr value will work
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.depth = m_Depth;
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;

    // Lock 3D Resource
    // Lock may fail due to the out of memory/out of page-in in KMD.
    // Touch queue for the buffer/surface data release
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnLock3DResource(pCmData->cmHalState, &inParam));
    CMCHK_NULL(inParam.data);

    uWidthInBytes = inParam.width * pixel;

    //Copy Resource
    pTempDst    = (uint8_t*)pSysMem;
    pTempSrc    = (uint8_t*)inParam.data;
    pRPlane     = (uint8_t*)inParam.data;

    // Only use Qpitch when Qpitch is supported by HW
    if (inParam.qpitchEnabled)
    {
        if (inParam.pitch == uWidthInBytes && inParam.qpitch == inParam.height)
        {
            CmFastMemCopyFromWC(pTempDst, pTempSrc, (size_t)uSizeInBytes, GetCpuInstructionLevel());
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                pTempSrc = pRPlane;
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyFromWC(pTempDst, pTempSrc, uWidthInBytes, GetCpuInstructionLevel());
                    pTempSrc += inParam.pitch;
                    pTempDst += uWidthInBytes;
                }
                pRPlane += inParam.qpitch * inParam.pitch;
            }
        }
    }
    else
    {
        if (inParam.pitch == uWidthInBytes)
        {
            CmFastMemCopyFromWC(pTempDst, pTempSrc, (size_t)uSizeInBytes, GetCpuInstructionLevel());
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyFromWC(pTempDst, pTempSrc, uWidthInBytes, GetCpuInstructionLevel());
                    pTempSrc += inParam.pitch;
                    pTempDst += uWidthInBytes;
                }
            }
        }
    }

    // unlock 3D resource
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnUnlock3DResource(pCmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}


CM_RT_API int32_t CmSurface3DRT::GetIndex( SurfaceIndex*& pIndex )
{
    pIndex = m_pIndex;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Properties of  CmSurface3DRT
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmSurface3DRT::GetProperties( uint32_t& width,  uint32_t& height, uint32_t& depth, CM_SURFACE_FORMAT& format)
{
    width  = m_Width;
    height = m_Height;
    depth = m_Depth;
    format = m_Format;
    return CM_SUCCESS;
}

int32_t CmSurface3DRT::SetProperties( uint32_t width,  uint32_t height, uint32_t depth, CM_SURFACE_FORMAT format)
{
    m_Width  = width;
    m_Height = height;
    m_Depth  = depth;
    m_Format = format;
    return CM_SUCCESS;
}


CM_RT_API int32_t CmSurface3DRT::InitSurface(const uint32_t initValue, CmEvent* pEvent)
{
    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint32_t        uSizeInBytes = 0;
    uint32_t        uWidthInBytes = 0;
    uint8_t         *pTempDst = nullptr;
    uint8_t         *pRPlane = nullptr;

    // It makes sense to flush the whole enqueued tasks for each surface read
    // because usually we read the output of the last task.
    // Update: using event not to flush the whole enqueued tasks
    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        FlushDeviceQueue( pEventRT );  // wait specific owner task finished
    }

    WaitForReferenceFree();   // wait all owner task finished

    CM_HAL_3DRESOURCE_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_3DRESOURCE_PARAM ) );

    CmDeviceRT* pCmDev = nullptr;
    m_SurfaceMgr->GetCmDevice( pCmDev );
    CM_ASSERT( pCmDev );
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDev->GetAccelData();

    uint32_t sizePerPixel = 0;
    uint32_t updatedHeight = 0;
    CMCHK_HR(m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight));

    inParam.handle = m_Handle;
    inParam.data = (void*)0x44; //Any non-nullptr value will work
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.depth = m_Depth;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnLock3DResource(pCmData->cmHalState, &inParam));
    CMCHK_NULL(inParam.data);

    uSizeInBytes = inParam.width * inParam.height * inParam.depth * sizePerPixel;
    uWidthInBytes = inParam.width * sizePerPixel;

    //Copy Resource
    pTempDst = (uint8_t*)inParam.data;
    pRPlane  = (uint8_t*)inParam.data;

    // Only use Qpitch when Qpitch is supported by HW
    if (inParam.qpitchEnabled)
    {
        if (inParam.pitch == uWidthInBytes && inParam.qpitch == inParam.height)
        {
            CmDwordMemSet(pTempDst, initValue, uSizeInBytes);
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                pTempDst = pRPlane;
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmDwordMemSet(pTempDst, initValue, uWidthInBytes);
                    pTempDst += inParam.pitch;
                }
                pRPlane += inParam.qpitch * inParam.pitch;
            }
        }
    }
    else
    {
        if (inParam.pitch == uWidthInBytes)
        {
            CmDwordMemSet(pTempDst, initValue, uSizeInBytes);
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmDwordMemSet(pTempDst, initValue, uWidthInBytes);
                    pTempDst += inParam.pitch;
                }
            }
        }
    }

    // unlock
    inParam.data = nullptr;
    inParam.handle = m_Handle;

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnUnlock3DResource(pCmData->cmHalState, &inParam));

finish:
    if (hr < CM_MOS_STATUS_CONVERTED_CODE_OFFSET) {
        hr = CM_LOCK_SURFACE_FAIL;
    }
    return hr;
}

int32_t CmSurface3DRT::SetMemoryObjectControl( MEMORY_OBJECT_CONTROL mem_ctrl, MEMORY_TYPE mem_type, uint32_t age)
{
    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint16_t mocs = 0;

    CmSurface::SetMemoryObjectControl( mem_ctrl, mem_type, age );

    CmDeviceRT *pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();
    CMCHK_NULL(pCmData);

    mocs = (m_MemObjCtrl.mem_ctrl << 8) | (m_MemObjCtrl.mem_type<<4) | m_MemObjCtrl.age;

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnSetSurfaceMOCS(pCmData->cmHalState, m_Handle, mocs, ARG_KIND_SURFACE_3D));

finish:
    return hr;
}

CM_RT_API int32_t CmSurface3DRT::SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl)
{
    return SetMemoryObjectControl(mem_ctrl, CM_USE_PTE, 0);
}

void CmSurface3DRT::Log(std::ostringstream &oss)
{
#if CM_LOG_ON
    oss << " Surface3D Info "
        << " Width:" << m_Width
        << " Height:" << m_Height
        << " Depth:" << m_Depth
        << " Format:" << GetFormatString(m_Format)
        << " Handle:" << m_Handle
        << " SurfaceIndex:" << m_pIndex->get_data()
        << " IsCmCreated:" << m_IsCmCreated
        << std::endl;
#endif
}

void CmSurface3DRT::DumpContent(uint32_t kernelNumber, int32_t taskId, uint32_t argIndex)
{
#if MDF_SURFACE_CONTENT_DUMP
    std::ostringstream outputFileName;
    static uint32_t surface3DDumpNumber = 0;
    outputFileName << "t_" << taskId
        << "_k_" << kernelNumber
        << "_argi_" << argIndex
        << "_surf2d_surfi_" << m_pIndex->get_data()
        << "_w_" << m_Width
        << "_h_" << m_Height
        << "_d_" << m_Depth
        << "_f_" << GetFormatString(m_Format)
        << "_" << surface3DDumpNumber;

    std::ofstream outputFileStream;
    outputFileStream.open(outputFileName.str().c_str(), std::ofstream::binary);

    uint32_t        surfaceSize = 0;
    uint32_t        sizePerPixel = 0;
    uint32_t        updatedHeight = 0;
    uint32_t        uWidthInBytes = 0;
    uint8_t         *pTempDst = nullptr;
    uint8_t         *pTempSrc = nullptr;
    uint8_t         *pRPlane = nullptr;
    m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight);
    surfaceSize = m_Width * updatedHeight * m_Depth * sizePerPixel;
    uWidthInBytes = m_Width * sizePerPixel;
    std::vector<char>surface(surfaceSize);

    CM_HAL_3DRESOURCE_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_3DRESOURCE_PARAM));

    CmDeviceRT* pCmDev = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDev);
    CM_ASSERT(pCmDev);
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDev->GetAccelData();

    inParam.handle = m_Handle;
    inParam.data = (void*)&surface[0];
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.depth = m_Depth;
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;

    pCmData->cmHalState->pfnLock3DResource(pCmData->cmHalState, &inParam);
    if (inParam.data == nullptr)
        return;
    pTempDst = (uint8_t*)&surface[0];
    pTempSrc = (uint8_t*)inParam.data;
    pRPlane = (uint8_t*)inParam.data;
    if (inParam.qpitchEnabled)
    {
        if (inParam.pitch == uWidthInBytes && inParam.qpitch == inParam.height)
        {
            CmFastMemCopyFromWC(pTempDst, pTempSrc, (size_t)surfaceSize, GetCpuInstructionLevel());
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                pTempSrc = pRPlane;
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyFromWC(pTempDst, pTempSrc, uWidthInBytes, GetCpuInstructionLevel());
                    pTempSrc += inParam.pitch;
                    pTempDst += uWidthInBytes;
                }
                pRPlane += inParam.qpitch * inParam.pitch;
            }
        }
    }
    else
    {
        if (inParam.pitch == uWidthInBytes)
        {
            CmFastMemCopyFromWC(pTempDst, pTempSrc, (size_t)surfaceSize, GetCpuInstructionLevel());
        }
        else
        {
            for (uint32_t uZ = 0; uZ < inParam.depth; uZ++)
            {
                for (uint32_t uY = 0; uY < inParam.height; uY++)
                {
                    CmFastMemCopyFromWC(pTempDst, pTempSrc, uWidthInBytes, GetCpuInstructionLevel());
                    pTempSrc += inParam.pitch;
                    pTempDst += uWidthInBytes;
                }
            }
        }
    }
    pCmData->cmHalState->pfnUnlock3DResource(pCmData->cmHalState, &inParam);

    outputFileStream.write(&surface[0], surfaceSize);
    outputFileStream.close();
    surface3DDumpNumber++;
#endif
}
}
