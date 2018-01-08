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
//! \brief     Contains OS-agnostic CmSurface2DRT member functions.  
//!

#include "cm_surface_2d_rt.h"

#include "cm_event_rt.h"
#include "cm_surface_manager.h"
#include "cm_device_rt.h"
#include "cm_mem.h"
#include "cm_queue_rt.h"

#define COPY_OPTION(uiOption)    (uiOption & 0x1)

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
//|               pSurfaceManager   [out]    Pointer to CmSurfaceManager
//|               pSurface          [out]    Reference to the Pointer to CmSurface2D

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
                           CmSurfaceManager* pSurfaceManager, 
                           CmSurface2DRT* &pSurface )
{
    int32_t result = CM_SUCCESS;

    pSurface = new (std::nothrow) CmSurface2DRT( handle,width,height, pitch,format,pSurfaceManager, isCmCreated);
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

bool CmSurface2DRT::IsGPUCopy(void *pSysMem, uint32_t iWidthInBytes, uint32_t iHeight, uint32_t iWidthStrideInBytes)
{
    return ( (iWidthInBytes <= CM_MAX_THREADSPACE_WIDTH_FOR_MW*128) && (iHeight <= CM_MAX_THREADSPACE_HEIGHT_FOR_MW*32) && (uintptr_t(pSysMem) & 0xF) == 0 && (iWidthStrideInBytes & 0xF) == 0 );
}

bool CmSurface2DRT::IsUnalignedGPUCopy(uint32_t iWidthInBytes, uint32_t iHeight)
{
    return (iWidthInBytes <= CM_MAX_THREADSPACE_WIDTH_FOR_MW*64) && (iHeight <= CM_MAX_THREADSPACE_HEIGHT_FOR_MW*8);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Hybrid memory copy from  system memory to Surface 2D to with stride
//| Arguments :
//|               pSysMem      [in]       Pointer to system memory
//|               pEvent       [in]       Pointer to CmEvent
//|               iWidthStride [in]       Stride in system memory in bytes
//|               iHeightStride[in]       Height Stride in system memory in rows
//|               sysMemSize   [in]       Size of Memory need to read
//|               uiOption     [in]       Option to disable/enable hybrid memory copy
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::WriteSurfaceHybridStrides( const unsigned char* pSysMem, CmEvent* pEvent, const uint32_t iWidthStride, const uint32_t iHeightStride, uint64_t sysMemSize, uint32_t uiOption )
{
    INSERT_API_CALL_LOG();

    int32_t     hr                  = CM_SUCCESS;
    uint32_t    sizePerPixel        = 0;
    uint32_t    updatedHeight       = 0;
    uint32_t    widthInBytes        = 0;
    CmQueue    *pCmQueue            = nullptr;
    CmDeviceRT *pCmDevice           = nullptr;
    CmQueueRT  *pCmQueueRT          = nullptr;
    bool        bForceCPUCopy       = COPY_OPTION(uiOption);
    CM_STATUS   status;

    m_SurfaceMgr->GetCmDevice(pCmDevice);
    int32_t result = m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Failed to get correct surface info.")
        return result; 
    }

    widthInBytes = m_Width * sizePerPixel;
    if(pSysMem == nullptr || iWidthStride < widthInBytes || iHeightStride < m_Height)
    {
        CM_ASSERTMESSAGE("Error: Invalid input arguments.")
        return CM_INVALID_ARG_VALUE;
    }

    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        FlushDeviceQueue( pEventRT ); //wait specific task finished
    }
    WaitForReferenceFree();   // wait all owner task finished

    if (bForceCPUCopy)
    {
        CMCHK_HR(WriteSurfaceFullStride(pSysMem, pEvent, iWidthStride, iHeightStride, sysMemSize));
    }
    else
    {
        CMCHK_HR(pCmDevice->CreateQueue(pCmQueue));
        if(IsGPUCopy((void*)pSysMem, widthInBytes, m_Height, iWidthStride))
        {
            CMCHK_HR(pCmQueue->EnqueueCopyCPUToGPUFullStride(this, pSysMem, iWidthStride, iHeightStride, 0, pEvent));
            if(pEvent)
            {
                CMCHK_HR(pEvent->GetStatus(status));
                while(status != CM_STATUS_FINISHED)
                {
                    if (status == CM_STATUS_RESET)
                    {
                        hr = CM_TASK_MEDIA_RESET;
                        goto finish;
                    }
                    CMCHK_HR(pEvent->GetStatus(status));
                }
            }
            else
            {
                CM_ASSERTMESSAGE("Error: Hybrid memory copy from system memory to Surface 2D failure.")
                return CM_FAILURE;
            }
        }
        else if (IsUnalignedGPUCopy(widthInBytes, m_Height))
        {
            pCmQueueRT = static_cast<CmQueueRT *>(pCmQueue);
            CMCHK_HR(pCmQueueRT->EnqueueUnalignedCopyInternal(this, (unsigned char*)pSysMem, iWidthStride, iHeightStride, CM_FASTCOPY_CPU2GPU, pEvent));
        }
        else
        {
            CMCHK_HR(WriteSurfaceFullStride(pSysMem, pEvent, iWidthStride, iHeightStride, sysMemSize));
        }
    }
    
finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Write data from system memory to Surface 2D
//| Arguments :
//|               pSysMem      [in]       Pointer to system memory
//|               pEvent       [in]       Pointer to CmEvent
//|               sysMemSize   [out]      Size of Memory need to write
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr              = CM_SUCCESS;
    uint8_t         *pDst            = nullptr;
    uint8_t         *pSurf           = nullptr;
    uint32_t        sizePerPixel    = 0;
    uint32_t        updatedHeight   = 0;
    uint32_t        size            = 0;
    uint32_t        pitch           = 0;
    uint32_t        row             = 0;
    UNUSED(sysMemSize);

    if(pSysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
        return CM_INVALID_ARG_VALUE;
    }

    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        FlushDeviceQueue( pEventRT ); //wait specific task finished
    }
    WaitForReferenceFree();   // wait all owner task finished

    CmDeviceRT * pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);

    //Lock for surface read/write
    CSync* pSurfaceLock = pCmDevice->GetSurfaceLock();
    CM_ASSERT(pSurfaceLock);
    CLock locker(*pSurfaceLock);

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();

    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM ) );
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.handle = m_Handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    // Lock Surface Resource:
    // Lock may fail due to the out of memory/out of page-in in KMD.
    // Touch queue for the buffer/surface data release
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnLock2DResource(pCmData->cmHalState, &inParam));
    CMCHK_NULL(inParam.data);

    //Copy data
    pDst  = ( uint8_t *)(inParam.data);
    pSurf = ( uint8_t *)pSysMem;

    // Get the memory size according to the format 
    CMCHK_HR(m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight));

    size = m_Width * sizePerPixel;
    pitch = m_Pitch;
    if(pitch != size)
    {
        for (row = 0; row < updatedHeight; row ++)
        {
            CmFastMemCopyWC(pDst, pSurf, size);

            pSurf += size;
            pDst += pitch;
        }
    }
    else
    {
        CmFastMemCopyWC(pDst, pSurf, pitch * updatedHeight);
    }

    //Unlock Surface2D
    inParam.data = nullptr;
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnUnlock2DResource(pCmData->cmHalState, &inParam));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Hybrid memory copy from  Surface 2D to system memory with stride
//| Arguments :
//|               pSysMem      [out]       Pointer to system memory
//|               pEvent       [in]       Pointer to CmEvent
//|               iWidthStride [in]       Width  Stride in system memory in bytes
//|               iHeightStride[in]       Height Stride in system memory in rows
//|               sysMemSize   [in]       Size of Memory need to read
//|               uiOption     [in]       Option to disable/enable hybrid memory copy
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::ReadSurfaceHybridStrides( unsigned char* pSysMem, CmEvent* pEvent, const uint32_t iWidthStride, const uint32_t iHeightStride, uint64_t sysMemSize, uint32_t uiOption )
{
    INSERT_API_CALL_LOG();

    int32_t     hr                  = CM_SUCCESS;
    uint32_t    sizePerPixel        = 0;
    uint32_t    updatedHeight       = 0;
    uint32_t    widthInBytes        = 0;
    CmQueue    *pCmQueue            = nullptr;
    CmDeviceRT *pCmDevice           = nullptr;
    CmQueueRT  *pCmQueueRT          = nullptr;
    bool        bForceCPUCopy       = COPY_OPTION(uiOption);
    CM_STATUS   status;

    m_SurfaceMgr->GetCmDevice(pCmDevice);
    int32_t result = m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight);
    if (result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Failed to get correct surface info.")
        return result; 
    }

    widthInBytes = m_Width * sizePerPixel;
    if(pSysMem == nullptr || iWidthStride < widthInBytes || iHeightStride < m_Height)
    {
        CM_ASSERTMESSAGE("Error: Invalid input arguments.")
        return CM_INVALID_ARG_VALUE;
    }

    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        FlushDeviceQueue( pEventRT );
    }

    WaitForReferenceFree();   // wait all owner task finished
    
    if (bForceCPUCopy)
    {
        CMCHK_HR(ReadSurfaceFullStride(pSysMem, pEvent, iWidthStride, iHeightStride, sysMemSize));
    }
    else
    {
        CMCHK_HR(pCmDevice->CreateQueue(pCmQueue));
        if(IsGPUCopy((void*)pSysMem, widthInBytes, m_Height, iWidthStride))
        {
            CMCHK_HR(pCmQueue->EnqueueCopyGPUToCPUFullStride(this, pSysMem, iWidthStride, iHeightStride, 0, pEvent));
            if(pEvent)
            {
                CMCHK_HR(pEvent->GetStatus(status));
                while(status != CM_STATUS_FINISHED)
                {
                    if (status == CM_STATUS_RESET)
                    {
                        hr = CM_TASK_MEDIA_RESET;
                        goto finish;
                    }
                    CMCHK_HR(pEvent->GetStatus(status));
                }
            }
            else
            {
                CM_ASSERTMESSAGE("Error: Hybrid memory copy from surface 2D to system memory failure.")
                return CM_FAILURE;
            }
        }
        else if (IsUnalignedGPUCopy(widthInBytes, m_Height))
        {
            pCmQueueRT = static_cast<CmQueueRT *>(pCmQueue);
            CMCHK_HR(pCmQueueRT->EnqueueUnalignedCopyInternal(this, (unsigned char*)pSysMem, iWidthStride, iHeightStride, CM_FASTCOPY_GPU2CPU, pEvent));
        }
        else
        {
            CMCHK_HR(ReadSurfaceFullStride(pSysMem, pEvent, iWidthStride, iHeightStride, sysMemSize));
        }
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Read data from  Surface 2D to system memory
//| Arguments :
//|               pSysMem      [in]       Pointer to system memory
//|               pEvent       [in]       Pointer to CmEvent
//|               sysMemSize   [out]      Size of Memory need to read
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr              = CM_SUCCESS;
    uint8_t         *pDst            = nullptr;
    uint8_t         *pSurf           = nullptr;
    uint32_t        sizePerPixel    = 0;
    uint32_t        updatedHeight   = 0;
    uint32_t        widthInByte     = 0;
    uint32_t        pitch           = 0;
    uint32_t        row             = 0;
    UNUSED(sysMemSize);

    if(pSysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
        return CM_INVALID_ARG_VALUE;
    }

    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        FlushDeviceQueue( pEventRT );
    }

    WaitForReferenceFree();   // wait all owner task finished

    CmDeviceRT * pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);

    //Lock for surface read/write
    CSync* pSurfaceLock = pCmDevice->GetSurfaceLock();
    CM_ASSERT(pSurfaceLock);
    CLock locker(*pSurfaceLock);

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();

    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM ) );
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.handle = m_Handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;

    // Lock Surface Resource:
    // Lock may fail due to the out of memory/out of page-in in KMD.
    // Touch queue for the buffer/surface data release
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnLock2DResource(pCmData->cmHalState, &inParam));
    CMCHK_NULL(inParam.data);

    //Copy data
    pDst = ( uint8_t *)pSysMem;
    pSurf= ( uint8_t *)(inParam.data);

    CMCHK_HR(m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight));

    widthInByte = m_Width * sizePerPixel;
    pitch = m_Pitch;
    if (pitch != widthInByte)
    {
        for (row = 0; row < updatedHeight; row ++)
        {
            CmFastMemCopyFromWC(pDst, pSurf, widthInByte, GetCpuInstructionLevel());
            pSurf += pitch;
            pDst += widthInByte;
        }
    }
    else
    {
        CmFastMemCopyFromWC(pDst, pSurf, pitch * updatedHeight, GetCpuInstructionLevel());
    }

    //Unlock
    inParam.data = nullptr;
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnUnlock2DResource(pCmData->cmHalState, &inParam));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the index of CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::GetIndex( SurfaceIndex*& pIndex ) 
{ 
    pIndex = m_pIndex; 
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Write data from  system memory to Surface 2D to with stride
//| Arguments :
//|               pSysMem      [in]       Pointer to system memory
//|               pEvent       [in]       Pointer to CmEvent
//|               Stride       [in]       Stride in system memory in bytes
//|               sysMemSize   [out]      Size of Memory need to read
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::WriteSurfaceStride( const unsigned char* pSysMem, CmEvent* pEvent, const uint32_t stride, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr              = CM_SUCCESS;
    uint8_t         *pDst            = nullptr;
    uint8_t         *pSrc            = nullptr;
    uint32_t        sizePerPixel    = 0;
    uint32_t        updatedHeight   = 0;
    uint32_t        widthInByte     = 0;
    uint32_t        pitch           = 0;
    uint32_t        row             = 0;
    UNUSED(sysMemSize);

    if(pSysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
        return CM_INVALID_ARG_VALUE;
    }

    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        FlushDeviceQueue( pEventRT ); // wait specific owner task finished
    }

    WaitForReferenceFree();   // wait all owner tasks finished

    CmDeviceRT * pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);

    //Lock for surface read/write
    CSync* pSurfaceLock = pCmDevice->GetSurfaceLock();
    CM_ASSERT(pSurfaceLock);
    CLock locker(*pSurfaceLock);

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();

    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM ) );
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.handle = m_Handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    // Lock Surface Resource:
    // Lock may fail due to the out of memory/out of page-in in KMD.
    // Touch queue for the buffer/surface data release
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnLock2DResource(pCmData->cmHalState, &inParam));
    CMCHK_NULL(inParam.data);

    //Copy data
    pDst = ( uint8_t *)(inParam.data);
    pSrc = ( uint8_t *)pSysMem;

    // Get the memory size according to the format 
    CMCHK_HR(m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight));

    widthInByte = m_Width * sizePerPixel;
    pitch = m_Pitch;
    if ((pitch != widthInByte) || (stride != pitch))
    {
        for (row = 0; row < updatedHeight; row ++)
        {
            CmFastMemCopyWC(pDst, pSrc, widthInByte);
            pSrc += stride;
            pDst += pitch;
        }
    }
    else
    {
        CmFastMemCopyWC(pDst, pSrc, pitch * updatedHeight);
    }

    //Unlock Surface2D
    inParam.data = nullptr; //Set pData to Null to differentiate route from app or cmrt@umd
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnUnlock2DResource(pCmData->cmHalState, &inParam));

finish:
    return hr;
}

CM_RT_API int32_t CmSurface2DRT::SetCompressionMode(MEMCOMP_STATE MmcMode)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr = CM_SUCCESS;
    CM_HAL_SURFACE2D_COMPRESSIOM_PARAM MmCModeParam;
    CmDeviceRT * pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);
    CM_ASSERT(pCmDevice); 
    MmCModeParam.handle = m_Handle;
    MmCModeParam.mmcMode = MmcMode;
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();
    CM_ASSERT(pCmData);
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnSetCompressionMode(pCmData->cmHalState, MmCModeParam));

finish:
    return hr;
}
//*-----------------------------------------------------------------------------
//| Purpose:    Read data from  Surface 2D to system memory with stride
//| Arguments :
//|               pSysMem      [in]       Pointer to system memory
//|               pEvent       [in]       Pointer to CmEvent
//|               Stride       [in]       Stride in system memory in bytes
//|               sysMemSize   [out]      Size of Memory need to read
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::ReadSurfaceStride( unsigned char* pSysMem, CmEvent* pEvent, const uint32_t stride, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();

    return ReadSurfaceFullStride(pSysMem, pEvent, stride, m_Height, sysMemSize);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Read data from  Surface 2D to system memory with stride
//| Arguments :
//|               pSysMem      [in]       Pointer to system memory
//|               pEvent       [in]       Pointer to CmEvent
//|               iWidthStride [in]       Width  Stride in system memory in bytes
//|               iHeightStride[in]       Height Stride in system memory in rows
//|               sysMemSize   [out]      Size of Memory need to read
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::ReadSurfaceFullStride( unsigned char* pSysMem, CmEvent* pEvent, 
                    const uint32_t iWidthStride, const uint32_t iHeightStride, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr              = CM_SUCCESS;
    uint8_t         *pDst            = nullptr;
    uint8_t         *pSrc            = nullptr;
    uint32_t        sizePerPixel    = 0;
    uint32_t        updatedHeight   = 0;
    UNUSED(sysMemSize);

    if(pSysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
        return CM_INVALID_ARG_VALUE;
    }

    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        FlushDeviceQueue( pEventRT ); //wait specific task finished
    }

    WaitForReferenceFree();   // wait all owner task finished

    CmDeviceRT * pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);

    //Lock for surface read/write
    CSync* pSurfaceLock = pCmDevice->GetSurfaceLock();
    CM_ASSERT(pSurfaceLock);
    CLock locker(*pSurfaceLock);

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();

    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM ) );
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.handle = m_Handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;

    // Lock Data
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnLock2DResource(pCmData->cmHalState, &inParam));
    CMCHK_NULL(inParam.data);

    //Copy data
    pDst = ( uint8_t *)pSysMem;
    pSrc = ( uint8_t *)(inParam.data);

    // Get the memory size according to the format 
    CMCHK_HR(m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight));

    if( m_Format == CM_SURFACE_FORMAT_NV12)
    {
        for( uint32_t i=0 ; i< m_Height ; i++)
        { // Y plane
          CmFastMemCopyFromWC(pDst, pSrc, m_Width, GetCpuInstructionLevel());
          pSrc  += m_Pitch;
          pDst  += iWidthStride;
        }

        pSrc  = ( uint8_t *)(inParam.data) + m_Height * m_Pitch;
        pDst  = ( uint8_t *)pSysMem + iWidthStride * iHeightStride;

        //To support NV12 format with odd height here.
        //if original height is even, the UV plane's height is set as m_Height/2, which equals to (m_Height+1)/2 
        //if original height is odd, the UV plane's height is set as roundup(m_Height/2), which equals to (m_Height+1)/2 too
        for (uint32_t i = 0; i< (m_Height + 1) / 2; i++)
        { // UV plane
          CmFastMemCopyFromWC(pDst, pSrc, m_Width, GetCpuInstructionLevel());
          pSrc  += m_Pitch;
          pDst  += iWidthStride;
        }
    }
    else
    {
        uint32_t size = m_Width * sizePerPixel;
        uint32_t pitch = m_Pitch;
        if((pitch != size)||(iWidthStride != size))
        {
           for (uint32_t i=0; i < updatedHeight; i++)
           {
              CmFastMemCopyFromWC(pDst, pSrc, size, GetCpuInstructionLevel());
              pSrc  += pitch;
              pDst  += iWidthStride;
           }
         }
         else
         {
             CmFastMemCopyFromWC(pDst, pSrc, pitch * updatedHeight, GetCpuInstructionLevel());
         }
    }

    //Unlock
    inParam.data = nullptr; //Set pData to Null to differentiate route from app or cmrt@umd
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnUnlock2DResource(pCmData->cmHalState, &inParam));

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Write data from  system memory to Surface 2D to with stride
//| Arguments :
//|               pSysMem      [in]       Pointer to system memory
//|               pEvent       [in]       Pointer to CmEvent
//|               iWidthStride [in]       Stride in system memory in bytes
//|               iHeightStride[in]       Height Stride in system memory in rows
//|               sysMemSize   [out]      Size of Memory need to read
//|
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmSurface2DRT::WriteSurfaceFullStride( const unsigned char* pSysMem, CmEvent* pEvent, const uint32_t iWidthStride, const uint32_t iHeightStride, uint64_t sysMemSize )
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr              = CM_SUCCESS;
    uint8_t         *pDst            = nullptr;
    uint8_t         *pSrc            = nullptr;
    uint32_t        sizePerPixel    = 0;
    uint32_t        updatedHeight   = 0;
    UNUSED(sysMemSize);

    if(pSysMem == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to system memory is null.")
        return CM_INVALID_ARG_VALUE;
    }

    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        FlushDeviceQueue( pEventRT ); // wait specific owner task finished
    }

    WaitForReferenceFree();   // wait all owner tasks finished

    CmDeviceRT * pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);

    //Lock for surface read/write
    CSync* pSurfaceLock = pCmDevice->GetSurfaceLock();
    CM_ASSERT(pSurfaceLock);
    CLock locker(*pSurfaceLock);

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();

    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM ) );
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.handle = m_Handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    // Lock Surface Resource:
    // Lock may fail due to the out of memory/out of page-in in KMD.
    // Touch queue for the buffer/surface data release
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnLock2DResource(pCmData->cmHalState, &inParam));
    CMCHK_NULL(inParam.data);

    //Copy data
    pDst = ( uint8_t *)(inParam.data);
    pSrc = ( uint8_t *)pSysMem;

    // Get the memory size according to the format 
    CMCHK_HR(m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight));

    if( m_Format == CM_SURFACE_FORMAT_NV12)
    {
        for( uint32_t i=0 ; i< m_Height ; i++)
        { // Y plane
          CmFastMemCopyFromWC(pDst, pSrc, m_Width, GetCpuInstructionLevel());
          pSrc  += iWidthStride ;
          pDst  += m_Pitch;
        }

        pDst  = ( uint8_t *)(inParam.data) + m_Height * m_Pitch;
        pSrc  = ( uint8_t *)pSysMem + iWidthStride * iHeightStride;

        //To support NV12 format with odd height here.
        //if original height is even, the UV plane's height is set as m_Height/2, which equals to (m_Height+1)/2 
        //if original height is odd, the UV plane's height is set as roundup(m_Height/2), which equals to (m_Height+1)/2 too
        for( uint32_t i=0 ; i< ( m_Height + 1) /2 ; i++)
        { // UV plane
          CmFastMemCopyFromWC(pDst, pSrc, m_Width, GetCpuInstructionLevel());
          pSrc  += iWidthStride ;
          pDst  += m_Pitch;
        }
    }
    else
    {
        uint32_t size = m_Width * sizePerPixel;
        uint32_t pitch = m_Pitch;
        if((pitch != size)||(iWidthStride != pitch))
        {
            for (uint32_t i=0; i < updatedHeight; i++)
            {
                CmFastMemCopyWC(pDst, pSrc, size);
                pSrc += iWidthStride;
                pDst += pitch;
            }
        }
        else
        {
            CmFastMemCopyWC(pDst, pSrc, pitch * updatedHeight);
        }
    }

    //Unlock Surface2D
    inParam.data = nullptr; //Set pData to Null to differentiate route from app or cmrt@umd
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnUnlock2DResource(pCmData->cmHalState, &inParam));

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
//|             pMosResource      [in]  Pointer to the new valid MosResource 
//|                                     that the CMSurface2D will be based on.
//|                                     Do not set this parameter if the 
//|                                     MosResource is already deleted.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CMRT_UMD_API int32_t
CmSurface2DRT::NotifyUmdResourceChanged(UMD_RESOURCE umdResource,
                                        int updateMosResource,
                                        PMOS_RESOURCE pMosResource)
{
    m_pUMDResource = umdResource;

    //
    if ( updateMosResource )
    {
        m_SurfaceMgr->UpdateSurface2DTableMosResource( m_Handle, pMosResource );
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the handle of  CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurface2DRT::GetHandle( uint32_t& handle)
{
    handle = m_Handle;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the handle of  CmSurface2D
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmSurface2DRT::GetIndexFor2D( uint32_t& index )
{
    index = m_Handle;
    return CM_SUCCESS;
}

int32_t CmSurface2DRT::SetSurfaceProperties(uint32_t width, uint32_t height, CM_SURFACE_FORMAT format)
{
    if (format == CM_SURFACE_FORMAT_NV12)
    {
        m_Pitch = MOS_ALIGN_CEIL(width * m_Pitch /m_Width , 2);
    }
    m_Width = width;
    m_Height = height;
    m_Format = format;

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

    width  = m_Width;
    height = m_Height;
    format = m_Format;

    // Get size per pixel
    ret = m_SurfaceMgr->GetPixelBytesAndHeight(width,  height,  format,  sizeperpixel, updatedHeight);

    return CM_SUCCESS;
}

CM_RT_API int32_t CmSurface2DRT::InitSurface(const unsigned int pInitValue, CmEvent* pEvent)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE                      hr          = CM_SUCCESS;
    CmDeviceRT*                         pCmDevice   = nullptr;
    PCM_CONTEXT_DATA                    pCmData     = nullptr;
    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM  inParam;
    uint32_t                            pitch       = 0;
    uint32_t                            *pSurf       = nullptr;
    uint32_t                            widthInBytes = 0;

    if( pEvent )
    {
        CmEventRT *pEventRT = static_cast<CmEventRT *>(pEvent);
        FlushDeviceQueue( pEventRT );
    }

    WaitForReferenceFree();   // wait all owner task finished

    uint32_t sizePerPixel  = 0;
    uint32_t updatedHeight = 0;
    CMCHK_HR(m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight));

    m_SurfaceMgr->GetCmDevice(pCmDevice);
    pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();

    CmSafeMemSet( &inParam, 0, sizeof( CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM ) );
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.handle = m_Handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_WRITEONLY;

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnLock2DResource(pCmData->cmHalState, &inParam));
    CMCHK_NULL(inParam.data);

    pitch = inParam.pitch;
    pSurf = ( uint32_t *)inParam.data;

    widthInBytes = m_Width * sizePerPixel; 
    if(widthInBytes != pitch)
    {
        for (uint32_t i=0; i < updatedHeight; i++)
        {
            if (widthInBytes % sizeof(uint32_t) == 0)
            {
                CmDwordMemSet(pSurf, pInitValue, widthInBytes);
            }
            else
            {
                CmDwordMemSet(pSurf, pInitValue, widthInBytes + sizeof(uint32_t));
            }
            
           pSurf += (pitch >> 2); // divide by 4 byte to dword
        }
    }
    else
    {
        CmDwordMemSet(pSurf, pInitValue, pitch * updatedHeight);
    }

    //Unlock Surface2D
    inParam.data = nullptr;
    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnUnlock2DResource(pCmData->cmHalState, &inParam));

finish:
    return hr;
}

int32_t CmSurface2DRT::SetMemoryObjectControl( MEMORY_OBJECT_CONTROL mem_ctrl, MEMORY_TYPE mem_type, uint32_t age)
{
    INSERT_API_CALL_LOG();

    CM_RETURN_CODE  hr = CM_SUCCESS;
    uint16_t mocs = 0;

    CmSurface::SetMemoryObjectControl( mem_ctrl, mem_type, age );

    CmDeviceRT *pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();
    CMCHK_NULL(pCmData);

    mocs = (m_MemObjCtrl.mem_ctrl << 8) | (m_MemObjCtrl.mem_type<<4) | m_MemObjCtrl.age;

    CHK_MOSSTATUS_RETURN_CMERROR(pCmData->cmHalState->pfnSetSurfaceMOCS(pCmData->cmHalState, m_Handle, mocs, ARG_KIND_SURFACE_2D));

finish:
    return hr;
}

CM_RT_API int32_t CmSurface2DRT::SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl)
{
    return SetMemoryObjectControl(mem_ctrl, CM_USE_PTE, 0);
}

int32_t CmSurface2DRT::Create2DAlias(SurfaceIndex* & pAliasIndex)
{
    INSERT_API_CALL_LOG();

    uint32_t surfArraySize = 0;
    uint32_t newIndex = 0;
    uint32_t origIndex = 0;

    if( m_numAliases < CM_HAL_MAX_NUM_2D_ALIASES )
    {
        origIndex = m_pIndex->get_data();
        m_SurfaceMgr->GetSurfaceArraySize(surfArraySize);
        newIndex = origIndex + ( (m_numAliases + 1) * surfArraySize);
        m_pAliasIndexes[m_numAliases] = MOS_New(SurfaceIndex, newIndex);
        if( m_pAliasIndexes[m_numAliases] )
        {
            pAliasIndex = m_pAliasIndexes[m_numAliases];
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

CM_RT_API int32_t CmSurface2DRT::SetSurfaceStateParam( SurfaceIndex *pSurfIndex, const CM_SURFACE2D_STATE_PARAM *pSSParam )
{
    CM_RETURN_CODE  hr = CM_SUCCESS;
    CmDeviceRT * pCmDevice = nullptr;
    PCM_CONTEXT_DATA pCmData = nullptr;
    CM_HAL_SURFACE2D_SURFACE_STATE_PARAM inParam;
    uint32_t iAliasIndex = 0;

    m_SurfaceMgr->GetCmDevice( pCmDevice );
    CMCHK_NULL(pCmDevice);
    pCmData = ( PCM_CONTEXT_DATA )pCmDevice->GetAccelData();
    CMCHK_NULL(pCmData);
    CMCHK_NULL(pCmData->cmHalState);

    CmSafeMemSet( &inParam, 0, sizeof( inParam ) );
    inParam.width       = pSSParam->width;
    inParam.height      = pSSParam->height;
    inParam.format      = pSSParam->format;
    inParam.depth       = pSSParam->depth;
    inParam.pitch       = pSSParam->pitch;
    inParam.memoryObjectControl   = pSSParam->memory_object_control;
    inParam.surfaceXOffset        = pSSParam->surface_x_offset;
    inParam.surfaceYOffset        = pSSParam->surface_y_offset;

    if (pSurfIndex)
    {
        iAliasIndex = pSurfIndex->get_data();
    }
    else
    {
        iAliasIndex = m_pIndex->get_data();
    }
    
    CHK_MOSSTATUS_RETURN_CMERROR( pCmData->cmHalState->pfnSet2DSurfaceStateParam(pCmData->cmHalState, &inParam, iAliasIndex, m_Handle) );

finish:
    return hr;
}

CMRT_UMD_API int32_t CmSurface2DRT::SetReadSyncFlag(bool bReadSync)
{
    int32_t hr = CM_SUCCESS;

    CmDeviceRT *pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();

    hr = pCmData->cmHalState->pfnSetSurfaceReadFlag(pCmData->cmHalState, m_Handle, bReadSync);

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
        << " Width:" << m_Width 
        << " Height:"<< m_Height
        << " Format:"<< GetFormatString(m_Format)
        << " Pitch:" << m_Pitch
        << " Handle:" << m_Handle
        << " SurfaceIndex:" << m_pIndex->get_data()
        << " IsCmCreated:"<<m_IsCmCreated
        << std::endl;
#endif
}


void CmSurface2DRT::DumpContent(uint32_t kernelNumber, int32_t taskId, uint32_t argIndex)
{
#if MDF_SURFACE_CONTENT_DUMP
    std::ostringstream outputFileName;
    static uint32_t surface2DDumpNumber = 0;
    outputFileName << "t_" << taskId
        << "_k_" << kernelNumber
        << "_argi_" << argIndex
        << "_surf2d_surfi_"<< m_pIndex->get_data()
        << "_w_" << m_Width
        << "_h_" << m_Height
        << "_p_" << m_Pitch
        << "_f_" << GetFormatString(m_Format)
        << "_" << surface2DDumpNumber;

    std::ofstream outputFileStream;
    outputFileStream.open(outputFileName.str().c_str(), std::ofstream::binary);

    CmDeviceRT * pCmDevice = nullptr;
    m_SurfaceMgr->GetCmDevice(pCmDevice);
    CSync* pSurfaceLock = pCmDevice->GetSurfaceLock();
    CM_ASSERT(pSurfaceLock);
    CLock locker(*pSurfaceLock);
    uint32_t        sizePerPixel = 0;
    uint32_t        updatedHeight = 0;
    uint32_t        surfaceSize = 0;
    uint32_t        widthInByte = 0; 
    uint8_t         *pDst = nullptr;
    uint8_t         *pSurf = nullptr;
    m_SurfaceMgr->GetPixelBytesAndHeight(m_Width, m_Height, m_Format, sizePerPixel, updatedHeight);
    surfaceSize = m_Width*sizePerPixel*updatedHeight;
    widthInByte = m_Width * sizePerPixel;

    std::vector<char>surface(surfaceSize);

    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)pCmDevice->GetAccelData();
    CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM inParam;
    CmSafeMemSet(&inParam, 0, sizeof(CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM));
    inParam.width = m_Width;
    inParam.height = m_Height;
    inParam.handle = m_Handle;
    inParam.lockFlag = CM_HAL_LOCKFLAG_READONLY;
    pCmData->cmHalState->pfnLock2DResource(pCmData->cmHalState, &inParam);
    if (inParam.data == nullptr)
        return;
    pDst = (uint8_t *)&surface[0];
    pSurf = (uint8_t *)(inParam.data);
    if (m_Pitch != widthInByte)
    {
        for (uint32_t row = 0; row < updatedHeight; row++)
        {
            CmFastMemCopyFromWC(pDst, pSurf, widthInByte, GetCpuInstructionLevel());
            pSurf += m_Pitch;
            pDst += widthInByte;
        }
    }
    else
    {
        CmFastMemCopyFromWC((unsigned char *)&surface[0], pSurf, m_Pitch * updatedHeight, GetCpuInstructionLevel());
    }
    inParam.data = nullptr; 
    pCmData->cmHalState->pfnUnlock2DResource(pCmData->cmHalState, &inParam);

    outputFileStream.write(&surface[0], surfaceSize);
    outputFileStream.close();
    surface2DDumpNumber++;
#endif
}




CM_RT_API int32_t CmSurface2DRT::SetProperty(CM_FRAME_TYPE frameType)
{
    m_frameType = frameType;
    m_SurfaceMgr->UpdateSurface2DTableFrameType(m_Handle, frameType);
    return CM_SUCCESS;
}
}
