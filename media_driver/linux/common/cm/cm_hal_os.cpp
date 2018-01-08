/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file      cm_hal_os.cpp
//! \brief     HAL Layer for CM Component on Linux
//!
#include "mos_os.h"
#include "cm_hal.h"
#include "cm_def_os.h"

#define Y_TILE_WIDTH  128
#define Y_TILE_HEIGHT 32
#define X_TILE_WIDTH  512
#define X_TILE_HEIGHT 8
#define ROUND_UP_TO(x, y) (((x) + (y) - 1) / (y) * (y))

//*-----------------------------------------------------------------------------
//| Purpose:    Referece the OsResource
//| Returns:    N/A
//*-----------------------------------------------------------------------------
void HalCm_OsResource_Reference(
    PMOS_RESOURCE    pOsResource)
{
    if (pOsResource && pOsResource->bo)
    {
        mos_bo_reference((MOS_LINUX_BO *)(pOsResource->bo));
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    unreferece the OsResource
//| Returns:    N/A
//*-----------------------------------------------------------------------------
void HalCm_OsResource_Unreference(
    PMOS_RESOURCE    pOsResource)
{
    if (pOsResource && pOsResource->bo)
    {
        mos_bo_unreference((MOS_LINUX_BO *)(pOsResource->bo));
        pOsResource->bo = nullptr;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Reference the Command Buffer and Pass it to event 
//| Returns:    N/A
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ReferenceCommandBuf_Linux(
    PMOS_RESOURCE     pOsResource,        //  [in]  Command Buffer's MOS Resurce
    void             **ppCmdBuffer)       // [out] Comamnd Buffer to pass to event
{
    if (pOsResource && pOsResource->bo)
    {
        mos_bo_reference((MOS_LINUX_BO *)(pOsResource->bo));
        *ppCmdBuffer = pOsResource->bo;
    }

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Command Buffer Resource and Pass it to event 
//| Returns:    N/A
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetCommandBufResource_Linux(
    PMOS_RESOURCE     pOsResource,        //  [in]  Command Buffer's MOS Resurce
    void             **ppCmdBuffer)       // [out] Comamnd Buffer to pass to event
{
    if (pOsResource && pOsResource->bo)
    {
        *ppCmdBuffer = pOsResource->bo;
    }

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Convert Mos Format to Gmm Fmt
//| Returns:    Gmm Fmt
//*-----------------------------------------------------------------------------
GMM_RESOURCE_FORMAT HalCm_ConvertMosFmtToGmmFmt(
    MOS_FORMAT format)
{
    switch (format)
    {
        case Format_Buffer      : return GMM_FORMAT_GENERIC_8BIT;
        case Format_Buffer_2D   : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_L8          : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_STMM        : return GMM_FORMAT_R8_UNORM_TYPE;              // matching size as format
        case Format_AI44        : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_IA44        : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_R5G6B5      : return GMM_FORMAT_B5G6R5_UNORM_TYPE;
        case Format_R8G8B8      : return GMM_FORMAT_R8G8B8_UNORM;
        case Format_X8R8G8B8    : return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case Format_A8R8G8B8    : return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case Format_A8B8G8R8    : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case Format_R32F        : return GMM_FORMAT_R32_FLOAT_TYPE;
        case Format_V8U8        : return GMM_FORMAT_GENERIC_16BIT;              // matching size as format
        case Format_YUY2        : return GMM_FORMAT_YUY2;
        case Format_UYVY        : return GMM_FORMAT_UYVY;
        case Format_P8          : return GMM_FORMAT_RENDER_8BIT_TYPE;           // matching size as format
        case Format_A8          : return GMM_FORMAT_A8_UNORM_TYPE;
        case Format_AYUV        : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case Format_NV12        : return GMM_FORMAT_NV12_TYPE;
        case Format_NV21        : return GMM_FORMAT_NV21_TYPE;
        case Format_YV12        : return GMM_FORMAT_YV12_TYPE;
        case Format_R32U        : return GMM_FORMAT_R32_UINT_TYPE;
        case Format_RAW         : return GMM_FORMAT_GENERIC_8BIT;
        case Format_P010        : return GMM_FORMAT_P010_TYPE;
        case Format_A16B16G16R16: return GMM_FORMAT_R16G16B16A16_UNORM_TYPE;
        default                 : return GMM_FORMAT_INVALID;
    }
}
//*-----------------------------------------------------------------------------
//| Purpose:    Get 2D surface info and register to OS-Command-Buffer's patch list.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetSurfaceAndRegister(
    PCM_HAL_STATE                pState, 
    PRENDERHAL_SURFACE           pRenderHalSurface,
    CM_HAL_KERNEL_ARG_KIND       surfKind,
    uint32_t                     iIndex,
    bool                         pixelPitch)
{
    MOS_STATUS             hr;
    RENDERHAL_GET_SURFACE_INFO Info;
    PRENDERHAL_INTERFACE     pRenderHal     = pState->pRenderHal;
    PMOS_SURFACE             pSurface       = &pRenderHalSurface->OsSurface;
    PRENDERHAL_MEDIA_STATE   media_state_ptr = nullptr;

    hr           = MOS_STATUS_UNKNOWN;

    if (!pRenderHalSurface) 
    {
        goto finish;
    }
    else 
    {
        MOS_ZeroMemory(pRenderHalSurface, sizeof(RENDERHAL_SURFACE));
    }

    switch(surfKind)
    {
        case CM_ARGUMENT_STATE_BUFFER:
            media_state_ptr = pState->pfnGetMediaStatePtrForSurfaceIndex( pState, iIndex );
            pSurface->OsResource.user_provided_va = pState->pfnGetStateBufferVAPtrForSurfaceIndex( pState, iIndex );
            pSurface->dwWidth = media_state_ptr->pDynamicState->Curbe.dwSize;
            pSurface->dwHeight = 1;
            pSurface->Format = Format_RAW;
            return MOS_STATUS_SUCCESS; // state buffer's OS resource belong to DSH, so don't need sync it and just return directly.

        case CM_ARGUMENT_SURFACEBUFFER:
            pSurface->dwWidth      = pState->pBufferTable[iIndex].iSize;
            pSurface->dwHeight     = 1;
            pSurface->Format       = Format_RAW;
            pRenderHalSurface->rcSrc.right  = pSurface->dwWidth;
            pRenderHalSurface->rcSrc.bottom = pSurface->dwHeight;
            pRenderHalSurface->rcDst        = pRenderHalSurface->rcSrc;
            CM_HRESULT2MOSSTATUS_AND_CHECK(pRenderHal->pOsInterface->pfnRegisterResource(
                pRenderHal->pOsInterface, &(pState->pBufferTable[iIndex].OsResource), true, true));
            pSurface->OsResource  = pState->pBufferTable[iIndex].OsResource;

        break;

        case CM_ARGUMENT_SURFACE3D:
            /* First register resource on Linux to get allocation slot on GpuContext */
            CM_HRESULT2MOSSTATUS_AND_CHECK(pRenderHal->pOsInterface->pfnRegisterResource(
                pRenderHal->pOsInterface, &(pState->pSurf3DTable[iIndex].OsResource), true, true));

            pSurface->OsResource   = pState->pSurf3DTable[iIndex].OsResource;
            pSurface->Format       = Format_Invalid;

            MOS_ZeroMemory(&Info, sizeof(RENDERHAL_GET_SURFACE_INFO));

            CM_CHK_MOSSTATUS(RenderHal_GetSurfaceInfo(
                pRenderHal->pOsInterface,
                &Info,
                pSurface));

            pSurface->Format       = pState->pSurf3DTable[iIndex].OsResource.Format;
            pRenderHalSurface->rcSrc.right  = pSurface->dwWidth;
            pRenderHalSurface->rcSrc.bottom = pSurface->dwHeight;
            pRenderHalSurface->rcDst        = pRenderHalSurface->rcSrc;
        break;

        case CM_ARGUMENT_SURFACE2D:
            /* First register resource on Linux to get allocation slot on GpuContext */
            CM_HRESULT2MOSSTATUS_AND_CHECK(pRenderHal->pOsInterface->pfnRegisterResource(
                pRenderHal->pOsInterface, &(pState->pUmdSurf2DTable[iIndex].OsResource), true, true));

            pSurface->OsResource     = pState->pUmdSurf2DTable[iIndex].OsResource;

            MOS_ZeroMemory(&Info, sizeof(RENDERHAL_GET_SURFACE_INFO));
            CM_CHK_MOSSTATUS(RenderHal_GetSurfaceInfo(
                pRenderHal->pOsInterface,
                &Info,
                pSurface));

            if ( (pSurface->Format == Format_NV12 || pSurface->Format == Format_YV12)
                  && (!pixelPitch)) 
            {
                pRenderHalSurface->SurfType = RENDERHAL_SURF_OUT_RENDERTARGET;
            }

            pSurface->Format         = pState->pUmdSurf2DTable[iIndex].format;
            pRenderHalSurface->rcSrc.right  = pSurface->dwWidth;
            pRenderHalSurface->rcSrc.bottom = pSurface->dwHeight;
            pRenderHalSurface->rcDst        = pRenderHalSurface->rcSrc;
            break;

        case CM_ARGUMENT_SURFACE2D_UP:
            /* First register resource on Linux to get allocation slot on GpuContext */
            CM_HRESULT2MOSSTATUS_AND_CHECK(pRenderHal->pOsInterface->pfnRegisterResource(
                pRenderHal->pOsInterface,  &(pState->pSurf2DUPTable[iIndex].OsResource), true, true));

            // Get Details of 2D surface and fill the VPHAL Surface 
            pSurface->OsResource = pState->pSurf2DUPTable[iIndex].OsResource;
            pSurface->dwWidth    = pState->pSurf2DUPTable[iIndex].iWidth;
            pSurface->dwHeight   = pState->pSurf2DUPTable[iIndex].iHeight;
            pSurface->Format     = pState->pSurf2DUPTable[iIndex].format;
            pSurface->dwDepth    = 1;
            pSurface->TileType   = MOS_TILE_LINEAR;
            //pSurface->Channel    = MOS_S3D_NONE;
            pSurface->dwOffset   = 0;

            if ( (pSurface->Format == Format_NV12 || pSurface->Format == Format_YV12)
                  && (!pixelPitch)) 
            {
                pRenderHalSurface->SurfType = RENDERHAL_SURF_OUT_RENDERTARGET;
            }

            MOS_ZeroMemory(&Info, sizeof(RENDERHAL_GET_SURFACE_INFO));

            CM_CHK_MOSSTATUS(RenderHal_GetSurfaceInfo(
                pRenderHal->pOsInterface,
                &Info,
                pSurface));

            pRenderHalSurface->rcSrc.right  = pSurface->dwWidth;
            pRenderHalSurface->rcSrc.bottom = pSurface->dwHeight;
            pRenderHalSurface->rcDst        = pRenderHalSurface->rcSrc;
            break;

        case CM_ARGUMENT_VME_STATE:
            /* First register resource on Linux to get allocation slot on GpuContext */
            CM_HRESULT2MOSSTATUS_AND_CHECK(pRenderHal->pOsInterface->pfnRegisterResource(
                pRenderHal->pOsInterface, &(pState->pUmdSurf2DTable[iIndex].OsResource), true, true));

            pSurface->OsResource = pState->pUmdSurf2DTable[iIndex].OsResource;

            MOS_ZeroMemory(&Info, sizeof(RENDERHAL_GET_SURFACE_INFO));

            CM_CHK_MOSSTATUS(RenderHal_GetSurfaceInfo(
                pRenderHal->pOsInterface,
                &Info,
                pSurface));

            pSurface->Format     = pState->pUmdSurf2DTable[iIndex].format;

            if (!pState->pCmHalInterface->IsSupportedVMESurfaceFormat(pSurface->Format))
            {
                CM_ERROR_ASSERT("Invalid VME Surface Format");
                goto finish;
            }

            pRenderHalSurface->rcSrc.right  = pSurface->dwWidth;
            pRenderHalSurface->rcSrc.bottom = pSurface->dwHeight;
            pRenderHalSurface->rcDst        = pRenderHalSurface->rcSrc;
            break;

        case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
        case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
            /* First register resource on Linux to get allocation slot on GpuContext */
            CM_HRESULT2MOSSTATUS_AND_CHECK(pRenderHal->pOsInterface->pfnRegisterResource(
                pRenderHal->pOsInterface, &(pState->pUmdSurf2DTable[iIndex].OsResource), true, true));

            // Get Details of 2D surface and fill the VPHAL Surface 
            pSurface->OsResource  = pState->pUmdSurf2DTable[iIndex].OsResource;

            MOS_ZeroMemory(&Info, sizeof(RENDERHAL_GET_SURFACE_INFO));

            CM_CHK_MOSSTATUS(RenderHal_GetSurfaceInfo(
                pRenderHal->pOsInterface,
                &Info,
                pSurface));

            pSurface->Format        = pState->pUmdSurf2DTable[iIndex].OsResource.Format;
            pRenderHalSurface->rcSrc.right  = pSurface->dwWidth;
            pRenderHalSurface->rcSrc.bottom = pSurface->dwHeight;
            pRenderHalSurface->rcDst        = pRenderHalSurface->rcSrc;
            break;

        default:
            CM_ERROR_ASSERT(
                "Argument kind '%d' is not supported", surfKind);
            goto finish;
    }

    //Tag-based Sync on the Resource/Surface
    CM_CHK_MOSSTATUS(HalCm_SyncOnResource(pState, pSurface, true));

    hr = MOS_STATUS_SUCCESS;
finish:
    return hr;
}

//===============<Os-dependent DDI Functions>============================================
//*-----------------------------------------------------------------------------
//| Purpose:    Get 2D surface pitch and physical size for SURFACE2D_UP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetSurfPitchSize(
    uint32_t Width, uint32_t Height, MOS_FORMAT Format, uint32_t *pPitch, uint32_t *pPhysicalSize)
{

    MOS_STATUS      hr = MOS_STATUS_SUCCESS;
    GMM_RESOURCE_INFO*   pGmmResInfo = nullptr;
    GMM_RESOURCE_FLAG    gmmFlags;
    GMM_RESCREATE_PARAMS gmmParams;

    MOS_ZeroMemory( &gmmFlags, sizeof( GMM_RESOURCE_FLAG ) );
    MOS_ZeroMemory( &gmmParams, sizeof( GMM_RESCREATE_PARAMS ) );

    if( nullptr == pPitch ||
        nullptr == pPhysicalSize)
    {
        hr = MOS_STATUS_NULL_POINTER;
        goto finish;
    }

    gmmFlags.Info.Linear    = true;
    gmmFlags.Info.Cacheable = true;
    gmmFlags.Gpu.Texture    = true;

    gmmParams.Type           = RESOURCE_2D;
    gmmParams.Format         = HalCm_ConvertMosFmtToGmmFmt( Format );
    gmmParams.Flags          = gmmFlags;
    gmmParams.BaseWidth      = Width;
    gmmParams.BaseHeight     = Height;
    gmmParams.Depth          = 1;
    gmmParams.ArraySize      = 1;
    gmmParams.NoGfxMemory    = true;

    // get pitch and size
    pGmmResInfo = GmmResCreate( &gmmParams );
    if (pGmmResInfo != nullptr)
    {
        *pPitch             = GmmResGetLockPitch( pGmmResInfo );
        *pPhysicalSize      = static_cast<uint32_t>( GmmResGetSizeSurface( pGmmResInfo ) );
        GmmResFree( pGmmResInfo );
    }
    else
    {
        *pPitch = 0;
        *pPhysicalSize = 0;
    }
    
finish:
    return hr;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Get 2D surface pitch and physical size for SURFACE2D_UP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetSurface2DPitchAndSize_Linux(
    PCM_HAL_STATE                   pState,                                             // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_UP_PARAM      pParam)                                             // [in]  Pointer to Buffer Param
{
    UNUSED(pState);
    return HalCm_GetSurfPitchSize(pParam->width, pParam->height, pParam->format, 
                                  &pParam->pitch, &pParam->physicalSize);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register APP/Runtime-level created Event Handle as a KMD Object;
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_RegisterKMDNotifyEventHandle_Linux(
    PCM_HAL_STATE             pState,
    PCM_HAL_OSSYNC_PARAM      pSyncParam)
{
    MOS_STATUS              hr;
    UNUSED(pState);
    UNUSED(pSyncParam);
    //-----------------------------------------
    CM_ASSERT(pState);
    CM_ASSERT(pSyncParam);
    //-----------------------------------------
    hr               = MOS_STATUS_SUCCESS;

    return hr;
}

uint32_t HalCm_GetSurf2DUPBaseWidth( uint32_t Width, uint32_t Pitch, MOS_FORMAT format)
{
    uint32_t BaseWidth = Width;
    uint32_t PixelSize = 1;

    switch(format)
    {
        case Format_L8 : 
        case Format_P8 : 
        case Format_A8 : 
        case Format_NV12:
            PixelSize = 1;
            break;
            
        case Format_X8R8G8B8    : 
        case Format_A8R8G8B8    : 
        case Format_A8B8G8R8    : 
        case Format_R32F        : 
            PixelSize = 4;
            break;
        
        case Format_V8U8        :
        case Format_YUY2        : 
        case Format_UYVY        : 
            PixelSize = 2;
            break;

        default:
            CM_ASSERTMESSAGE("Error: Unsupported surface format.");
            break;
    }

    BaseWidth = Pitch/PixelSize;
    return BaseWidth;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Gmm Resource Info for 2DUP 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_CreateGmmResInfo2DUP( PMOS_RESOURCE pOsResource, void  *pSysMem, uint32_t SysMemSize)
{
    MOS_STATUS              eStatus;
    GMM_RESCREATE_PARAMS    GmmParams;

    eStatus               = MOS_STATUS_SUCCESS;
    MOS_ZeroMemory(&GmmParams, sizeof(GmmParams));

    CM_ASSERT(pOsResource);

    // Set GmmParam
    GmmParams.BaseWidth             = pOsResource->iWidth;
    GmmParams.BaseHeight            = pOsResource->iHeight;
    GmmParams.ArraySize             = 1;
    GmmParams.Type                  = RESOURCE_2D; 

    GmmParams.Format                = HalCm_ConvertMosFmtToGmmFmt(pOsResource->Format);
    
    GmmParams.Flags.Gpu.Video      = true;
    GmmParams.Flags.Info.Linear    = true;
    GmmParams.Flags.Info.ExistingSysMem = true;
    
    GmmParams.ExistingSysMemSize   = SysMemSize;
    GmmParams.pExistingSysMem      = (GMM_VOIDPTR64)pSysMem;

    //Create GmmResourceInfo
    pOsResource->pGmmResInfo = GmmResCreate(&GmmParams);

    MOS_OS_CHK_NULL(pOsResource->pGmmResInfo);

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create Gmm Resource Info for 3D 
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_CreateGmmResInfo3D( PMOS_RESOURCE pOsResource)
{
    MOS_STATUS              eStatus;
    GMM_RESCREATE_PARAMS    GmmParams;

    eStatus               = MOS_STATUS_SUCCESS;
    MOS_ZeroMemory(&GmmParams, sizeof(GmmParams));

    CM_ASSERT(pOsResource);

    // Set GmmParam
    GmmParams.BaseWidth             = pOsResource->iWidth;
    GmmParams.BaseHeight            = pOsResource->iHeight;
    GmmParams.Depth                 = pOsResource->iDepth;
    GmmParams.ArraySize             = 1;
    GmmParams.Type                  = RESOURCE_3D;
    GmmParams.Format                = HalCm_ConvertMosFmtToGmmFmt(pOsResource->Format);

    GmmParams.Flags.Gpu.Video      = true;
    GmmParams.Flags.Info.Linear    = true;
    
    //Create GmmResourceInfo
    pOsResource->pGmmResInfo = GmmResCreate(&GmmParams);

    MOS_OS_CHK_NULL(pOsResource->pGmmResInfo);

finish:
    return eStatus;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Linear Buffer or BufferUP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateBuffer_Linux(
    PCM_HAL_STATE           pState,                                             // [in]  Pointer to CM State
    PCM_HAL_BUFFER_PARAM    pParam)                                             // [in]  Pointer to Buffer Param
{
    MOS_STATUS              hr;
    PMOS_INTERFACE          pOsInterface;
    PCM_HAL_BUFFER_ENTRY    pEntry = nullptr;
    MOS_ALLOC_GFXRES_PARAMS AllocParams;
    uint32_t                i;
    uint32_t                iSize;
    uint32_t                tileformat;
    const char              *fmt;
    PMOS_RESOURCE           pOsResource;
    MOS_LINUX_BO     	    *bo = nullptr;

    iSize  = pParam->size;
    tileformat = I915_TILING_NONE;

    //-----------------------------------------------
    CM_ASSERT(pParam->size > 0);
    //-----------------------------------------------

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pRenderHal->pOsInterface;


    // Find a free slot
    for (i = 0; i < pState->CmDeviceParam.iMaxBufferTableSize; i++)
    {
        if (pState->pBufferTable[i].iSize == 0)
        {
            pEntry              = &pState->pBufferTable[i];
            pParam->handle      = (uint32_t)i;
            break;
        }
    }

    if (!pEntry)
    {
        CM_ERROR_ASSERT("Buffer table is full");
        goto finish;
    }

    // State buffer doesn't need any MOS RESOURCE, so it will return directly after getting a position in the buffer table
    if ( pParam->type == CM_BUFFER_STATE )
    {
        pEntry->iSize = pParam->size;
        pEntry->isAllocatedbyCmrtUmd = false;
        return hr;
    }

    pOsResource = &(pEntry->OsResource);

    if (pParam->isAllocatedbyCmrtUmd)
    {
        // Resets the Resource
        Mos_ResetResource(pOsResource);

        if (pParam->data == nullptr)
        {
            MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            AllocParams.Type          = MOS_GFXRES_BUFFER;
            AllocParams.TileType      = MOS_TILE_LINEAR;
            AllocParams.dwBytes       = pParam->size;
            AllocParams.pSystemMemory = pParam->data;
            AllocParams.Format        = Format_Buffer;  //used in VpHal_OsAllocateResource_Linux!
            AllocParams.pBufName      = "CmBuffer";

            CM_HRESULT2MOSSTATUS_AND_CHECK(pOsInterface->pfnAllocateResource(
                pOsInterface, 
                &AllocParams,
                &pEntry->OsResource));
        }
        else  //BufferUP
        {
#if defined(DRM_IOCTL_I915_GEM_USERPTR) 
           bo =  mos_bo_alloc_userptr(pOsInterface->pOsContext->bufmgr, 
                                 "CM Buffer UP", 
                                 (void *)(pParam->data), 
                                 tileformat, 
                                 ROUND_UP_TO(iSize,MOS_PAGE_SIZE),
                                 ROUND_UP_TO(iSize,MOS_PAGE_SIZE),
#if defined(ANDROID)
                                 I915_USERPTR_UNSYNCHRONIZED
#else
				 0
#endif
				 );
#else
           bo =  mos_bo_alloc_vmap(pOsInterface->pOsContext->bufmgr, 
                                "CM Buffer UP", 
                                (void *)(pParam->data), 
                                tileformat, 
                                ROUND_UP_TO(iSize,MOS_PAGE_SIZE),
                                ROUND_UP_TO(iSize,MOS_PAGE_SIZE),
#if defined(ANDROID)
                                 I915_USERPTR_UNSYNCHRONIZED
#else
				 0
#endif
				 );
#endif

            pOsResource->bMapped = false;
            if (bo)
            {
                pOsResource->Format   = Format_Buffer;
                pOsResource->iWidth   = ROUND_UP_TO(iSize,MOS_PAGE_SIZE);
                pOsResource->iHeight  = 1;
                pOsResource->iPitch   = ROUND_UP_TO(iSize,MOS_PAGE_SIZE);
                pOsResource->bo       = bo;
                pOsResource->TileType = LinuxToMosTileType(tileformat);
                pOsResource->pData    = (uint8_t*) bo->virt;
            }
            else
            {
                fmt = "BufferUP";
                CM_DDI_ASSERTMESSAGE("Fail to Alloc BufferUP %7d bytes (%d x %d %s resource)\n",iSize, iSize, 1, fmt);
                hr = MOS_STATUS_UNKNOWN;
            }
            pOsResource->bConvertedFromDDIResource = true;
        }
    }
    else
    {
        pEntry->OsResource = *pParam->mosResource;
        HalCm_OsResource_Reference(&pEntry->OsResource);
    }

    pEntry->iSize = pParam->size;
    pEntry->isAllocatedbyCmrtUmd = pParam->isAllocatedbyCmrtUmd;
    pEntry->surfaceStateEntry[0].surfaceStateSize = pEntry->iSize;
    pEntry->surfaceStateEntry[0].surfaceStateOffset = 0;
    pEntry->surfaceStateEntry[0].surfaceStateMOCS = 0;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Surface2DUP (zero-copy, map system memory to video address space)
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateSurface2DUP_Linux(
    PCM_HAL_STATE                   pState,                                             // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_UP_PARAM      pParam)                                             // [in]  Pointer to Buffer Param
{
    MOS_STATUS                 hr;
    PMOS_INTERFACE             pOsInterface;
    PCM_HAL_SURFACE2D_UP_ENTRY pEntry = nullptr;
    uint32_t                   i;

    PMOS_RESOURCE              pOsResource;
    MOS_FORMAT                 Format;
    uint32_t                   iHeight;
    uint32_t                   iWidth;
    uint32_t                   iSize   = 0;
    uint32_t                   iPitch  = 0;
    uint32_t                   align_x = 0;
    uint32_t                   align_y = 0;
    MOS_LINUX_BO               *bo      = nullptr;
    void                       *pSysMem = nullptr;
    uint32_t                   tileformat = I915_TILING_NONE;

    //-----------------------------------------------
    CM_ASSERT(pState);
    CM_ASSERT(pParam->width > 0);
    //-----------------------------------------------

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pRenderHal->pOsInterface;

    // Find a free slot
    for (i = 0; i < pState->CmDeviceParam.iMax2DSurfaceUPTableSize; i++)
    {
        if (pState->pSurf2DUPTable[i].iWidth == 0)
        {
            pEntry              = &pState->pSurf2DUPTable[i];
            pParam->handle      = (uint32_t)i;
            break;
        }
    }

    if (!pEntry)
    {
        CM_ERROR_ASSERT("Surface2DUP table is full");
        goto finish;
    }

    Format  = pParam->format;
    iWidth  = pParam->width;
    iHeight = pParam->height;
    pSysMem = (void *)pParam->data;

    pOsResource = &(pEntry->OsResource);
    // Resets the Resource
    Mos_ResetResource(pOsResource);
    //Get Surface2D's physical size and pitch 
    HalCm_GetSurfPitchSize(iWidth, iHeight, Format, &align_x, &iSize);

#if defined(DRM_IOCTL_I915_GEM_USERPTR) 
    bo =  mos_bo_alloc_userptr(pOsInterface->pOsContext->bufmgr, 
                         "CM Surface2D UP", 
                         (void *)(pSysMem), 
                         tileformat, 
                         align_x, 
                         iSize,
#if defined(ANDROID)
                         I915_USERPTR_UNSYNCHRONIZED
#else
			 0
#endif
			 );
#else
    bo =  mos_bo_alloc_vmap(pOsInterface->pOsContext->bufmgr, 
                         "CM Surface2D UP", 
                         (void *)(pSysMem), 
                         tileformat, 
                         align_x, 
                         iSize,
#if defined(ANDROID)
                         I915_USERPTR_UNSYNCHRONIZED
#else
			 0
#endif
                         );
#endif

    pOsResource->bMapped = false;
    if (bo)
    {
        pOsResource->Format   = Format;
        pOsResource->iWidth   = iWidth;
        pOsResource->iHeight  = iHeight;
        pOsResource->iPitch   = align_x;
        pOsResource->bo       = bo;
        pOsResource->TileType = LinuxToMosTileType(tileformat);
        pOsResource->pData    = (uint8_t*) bo->virt;
       
        // Create Gmm resource info
        CM_CHK_MOSSTATUS(HalCm_CreateGmmResInfo2DUP(pOsResource, pSysMem, iSize));
    }
    else
    {
        hr = MOS_STATUS_UNKNOWN;
    }

    pEntry->iWidth  = pParam->width;
    pEntry->iHeight = pParam->height;
    pEntry->format  = Format;

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate 3D resource
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Allocate3DResource_Linux(
    PCM_HAL_STATE               pState,                                         // [in]  Pointer to CM State
    PCM_HAL_3DRESOURCE_PARAM    pParam)                                         // [in]  Pointer to Buffer Param
{
    MOS_STATUS                  hr;
    PMOS_INTERFACE              pOsInterface;
    PCM_HAL_3DRESOURCE_ENTRY    pEntry;
    uint32_t                    i;

    PMOS_RESOURCE               pOsResource;
    MOS_FORMAT                  Format;
    uint32_t                    tileformat;
    int32_t                     iHeight;
    int32_t                     iWidth;
    int32_t                     iDepth;
    int32_t                     iSize  = 0;
    int32_t                     iPitch = 0;
    MOS_LINUX_BO                *bo    = nullptr;

    //-----------------------------------------------
    CM_ASSERT(pState);
    CM_ASSERT(pParam->depth  > 1);
    CM_ASSERT(pParam->width  > 0);
    CM_ASSERT(pParam->height > 0);
    //-----------------------------------------------

    hr              = MOS_STATUS_SUCCESS;
    pOsInterface    = pState->pRenderHal->pOsInterface;
    pEntry          = nullptr;

    // Find a free slot
    for (i = 0; i < pState->CmDeviceParam.iMax3DSurfaceTableSize; i++)
    {
        if (Mos_ResourceIsNull(&pState->pSurf3DTable[i].OsResource))
        {
            pEntry              = &pState->pSurf3DTable[i];
            pParam->handle      = (uint32_t)i;
            break;
        }
    }

    if (!pEntry)
    {
        CM_ERROR_ASSERT("3D Surface table is full");
        goto finish;
    }

    Format  = pParam->format;
    iWidth  = pParam->width;
    iHeight = pParam->height;
    iDepth  = pParam->depth;

    pOsResource = &(pEntry->OsResource);
    // Resets the Resource
    Mos_ResetResource(pOsResource);

    if ((iDepth < 1) || \
        ((Format != Format_A8R8G8B8) && \
         (Format != Format_X8R8G8B8) && \
         (Format != Format_A16B16G16R16)))
    {
        CM_ERROR_ASSERT("Invalid Argument for 3D Surface!");
        goto finish;
    }

    switch (Format)
    {
        case Format_A8R8G8B8:
            iPitch = 4 * iWidth;
            tileformat = I915_TILING_NONE;
            break;
        case Format_X8R8G8B8:
            iPitch = 4 * iWidth;
            tileformat = I915_TILING_NONE;
            break;
        case Format_A16B16G16R16:
            iPitch = 8 * iWidth;
            tileformat = I915_TILING_NONE;
            break;
        default:
            iPitch = iWidth;
            tileformat = I915_TILING_NONE;
    }

    iSize = iHeight * iPitch * iDepth;

    if( tileformat == I915_TILING_NONE ){
        bo = mos_bo_alloc(pOsInterface->pOsContext->bufmgr, "CM 3D Surface", iSize, 4096);
    }

    pOsResource->bMapped = false;

    if (bo)
    {
        pOsResource->Format  = Format;
        pOsResource->iWidth  = iWidth;
        pOsResource->iHeight = iHeight;
        pOsResource->iPitch  = iPitch;
        pOsResource->iDepth  = iDepth;
        pOsResource->bo      = bo;
        pOsResource->TileType = LinuxToMosTileType(tileformat);
        pOsResource->pData    = (uint8_t*) bo->virt;
        HalCm_CreateGmmResInfo3D(pOsResource);
    }
    else
    {
        CM_DDI_ASSERTMESSAGE("Fail to Alloc %7d bytes (%d x %d resource)\n", iSize, iWidth, iHeight);
        hr = MOS_STATUS_UNKNOWN;
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get GPU current frequency
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetGPUCurrentFrequency_Linux(
    PCM_HAL_STATE               pState,                                         // [in]  Pointer to CM State
    uint32_t                    *pCurrentFreq)                                   // [out] Pointer to current frequency
{
    MOS_STATUS            hr;
    UNUSED(pState);

    //-----------------------------------------
    CM_ASSERT(pState);
    //-----------------------------------------
    *pCurrentFreq   = 0;
    hr              = MOS_STATUS_SUCCESS;

    return hr;
}

MOS_STATUS HalCm_GetGpuTime_Linux(PCM_HAL_STATE pState, uint64_t *pGpuTime)
{
    UNUSED(pState);
    *pGpuTime = 0;

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
// Purpose: Check if conditional batch buffer supported
// Returns: False on Linux
//*-----------------------------------------------------------------------------
bool HalCm_IsCbbEnabled(
    PCM_HAL_STATE                           pState)
{
    return false;
}

//*-----------------------------------------------------------------------------
// Purpose: Wait kernel finished in state heap
// Returns: Result of the operation
//*-----------------------------------------------------------------------------
int32_t HalCm_SyncKernel(
    PCM_HAL_STATE                           pState,
    uint32_t                                dwSync)
{
    int32_t                    hr = CM_SUCCESS;
    PRENDERHAL_INTERFACE       pRenderHal = pState->pRenderHal;
    PRENDERHAL_STATE_HEAP      pStateHeap = pRenderHal->pStateHeap;

    // Update Sync tags
    CMCHK_HR(pRenderHal->pfnRefreshSync(pRenderHal));

    while ( ( int32_t )( pStateHeap->dwSyncTag - dwSync ) < 0 )
    {
        CMCHK_HR( pRenderHal->pfnRefreshSync( pRenderHal ) );
    }

finish:
    return hr;
}

//===============<Os-dependent Private/Non-DDI Functions, Part 2>============================================

//Require DRM VMAP patch,
//Referecing:
//    [Intel-gfx] [PATCH 21/21] drm/i915: Introduce vmap (mapping of user pages into video memory) ioctl
//    http://lists.freedesktop.org/archives/intel-gfx/2011-April/010241.html
void HalCm_GetLibDrmVMapFnt(
                 PCM_HAL_STATE           pCmState)
{
    pCmState->hLibModule = nullptr;
    pCmState->pDrmVMap = nullptr;
    return ;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Gets platform information like slices/sub-slices/EUPerSubSlice etc.
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetPlatformInfo_Linux(
    PCM_HAL_STATE                  pState,              // [in] Pointer to CM State
    PCM_PLATFORM_INFO	           platformInfo,        // [out] Pointer to platformInfo
    bool                           bEUSaturation)
{

    MOS_STATUS                 hr        = MOS_STATUS_SUCCESS;
    MEDIA_SYSTEM_INFO             *pGtSystemInfo;

    UNUSED(bEUSaturation);
    
    pGtSystemInfo = pState->pOsInterface->pfnGetGtSystemInfo(pState->pOsInterface);

    platformInfo->numHWThreadsPerEU    = pGtSystemInfo->ThreadCount / pGtSystemInfo->EUCount;
    platformInfo->numEUsPerSubSlice    = pGtSystemInfo->EUCount / pGtSystemInfo->SubSliceCount;
    platformInfo->numSlices            = pGtSystemInfo->SliceCount;
    platformInfo->numSubSlices         = pGtSystemInfo->SubSliceCount;
        
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Gets GT system information for which slices/sub-slices are enabled
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetGTSystemInfo_Linux(
    PCM_HAL_STATE                pState,                // [in] Pointer to CM state
    PCM_GT_SYSTEM_INFO           pSystemInfo            // [out] Pointer to CM GT system info
)
{
    MEDIA_SYSTEM_INFO            *pGtSystemInfo;
    CM_EXPECTED_GT_SYSTEM_INFO    expectedGTInfo;

    pGtSystemInfo = pState->pOsInterface->pfnGetGtSystemInfo(pState->pOsInterface);

    pSystemInfo->numMaxSlicesSupported    = pGtSystemInfo->MaxSlicesSupported;
    pSystemInfo->numMaxSubSlicesSupported = pGtSystemInfo->MaxSubSlicesSupported;

    pState->pCmHalInterface->GetExpectedGtSystemConfig(&expectedGTInfo);
    
    // check numSlices/SubSlices enabled equal the expected number for this GT
    // if match, pSystemInfo->isSliceInfoValid = true, else pSystemInfo->isSliceInfoValid = false
    if ((expectedGTInfo.numSlices    == pGtSystemInfo->SliceCount) &&
        (expectedGTInfo.numSubSlices == pGtSystemInfo->SubSliceCount)) 
    {
        pSystemInfo->isSliceInfoValid = true;
    }
    else
    {
        pSystemInfo->isSliceInfoValid = false;
    }

    // if valid, set the number slice/subSlice to enabled for numSlices/numSubSlices
    if(pSystemInfo->isSliceInfoValid)
    {
        for(int i = 0; i < pGtSystemInfo->SliceCount; ++i)
        {
            pSystemInfo->sliceInfo[i].Enabled = true;
            for(int j = 0; j < pGtSystemInfo->SubSliceCount; ++j)
            {
                pSystemInfo->sliceInfo[i].SubSliceInfo[j].Enabled = true;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Query the status of the task
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_QueryTask_Linux(
    PCM_HAL_STATE             pState,
    PCM_HAL_QUERY_TASK_PARAM  pQueryParam)
{
    MOS_STATUS              hr;
    PRENDERHAL_INTERFACE    pRenderHal;
    int64_t                 *piSyncStart;
    int64_t                 *piSyncEnd;
    uint64_t                iTicks;
    int32_t                 iSyncOffset;
    PRENDERHAL_STATE_HEAP   pStateHeap;
    uint64_t                iHWStartTicks;
    uint64_t                iHWEndTicks;
    int32_t                 iMaxTasks;

    //-----------------------------------------
    CM_ASSERT(pState);
    CM_ASSERT(pQueryParam);
    //-----------------------------------------

    hr = MOS_STATUS_SUCCESS;

    iMaxTasks = (int32_t)pState->CmDeviceParam.iMaxTasks;
    if ((pQueryParam->taskId < 0) || (pQueryParam->taskId >= iMaxTasks) ||
        (pState->pTaskStatusTable[pQueryParam->taskId] == CM_INVALID_INDEX))
    {
        CM_ERROR_ASSERT("Invalid Task ID'%d'.", pQueryParam->taskId);
        goto finish;
    }

    pRenderHal = pState->pRenderHal;
    pStateHeap = pRenderHal->pStateHeap;
    iSyncOffset = pState->pfnGetTaskSyncLocation(pQueryParam->taskId);
    piSyncStart = (int64_t*)(pState->Render_TsResource.pData + iSyncOffset);
    piSyncEnd = piSyncStart + 1;
    pQueryParam->taskDurationNs = CM_INVALID_INDEX;

    if (*piSyncStart == CM_INVALID_INDEX)
    {
        pQueryParam->status = CM_TASK_QUEUED;
    }
    else if (*piSyncEnd == CM_INVALID_INDEX)
    {
        pQueryParam->status = CM_TASK_IN_PROGRESS;
    }
    else
    {
        pQueryParam->status = CM_TASK_FINISHED;

        pRenderHal->pfnConvertToNanoSeconds(
            pRenderHal,
            *piSyncStart,
            &iHWStartTicks);

        pRenderHal->pfnConvertToNanoSeconds(
            pRenderHal,
            *piSyncEnd,
            &iHWEndTicks);

        iTicks = *piSyncEnd - *piSyncStart;

        // Convert ticks to Nanoseconds
        pRenderHal->pfnConvertToNanoSeconds(
            pRenderHal,
            iTicks,
            &pQueryParam->taskDurationNs);

        pQueryParam->taskGlobalSubmitTimeCpu = pState->pTaskTimeStamp->iGlobalCmSubmitTime[pQueryParam->taskId];
        CM_CHK_MOSSTATUS(pState->pfnConvertToQPCTime(pState->pTaskTimeStamp->iCMSubmitTimeStamp[pQueryParam->taskId], &pQueryParam->taskSubmitTimeGpu));
        CM_CHK_MOSSTATUS(pState->pfnConvertToQPCTime(iHWStartTicks, &pQueryParam->taskHWStartTimeStamp));
        CM_CHK_MOSSTATUS(pState->pfnConvertToQPCTime(iHWEndTicks, &pQueryParam->taskHWEndTimeStamp));

        pState->pTaskStatusTable[pQueryParam->taskId] = CM_INVALID_INDEX;
    }

finish:
    return hr;
}

MOS_STATUS HalCm_WriteGPUStatusTagToCMTSResource_Linux(
    PCM_HAL_STATE             pState,
    PMOS_COMMAND_BUFFER       pCmdBuffer,
    int32_t                   iTaskID,
    bool                      isVebox)
{
    UNUSED(pState);
    UNUSED(pCmdBuffer);
    UNUSED(iTaskID);
    UNUSED(isVebox);
    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Lock the resource and return
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Lock2DResource(
    PCM_HAL_STATE               pState,                                         // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM     pParam)                                         // [in]  Pointer to 2D Param
{
    MOS_STATUS                  hr = MOS_STATUS_SUCCESS;
    MOS_LOCK_PARAMS             LockFlags;
    RENDERHAL_GET_SURFACE_INFO  Info;

    MOS_SURFACE                 Surface;
    PMOS_INTERFACE              pOsInterface = nullptr;

    if ((pParam->lockFlag != CM_HAL_LOCKFLAG_READONLY) && (pParam->lockFlag != CM_HAL_LOCKFLAG_WRITEONLY) )
    {
        CM_ERROR_ASSERT("Invalid lock flag!");
        hr = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    MOS_ZeroMemory(&Surface, sizeof(Surface));
    Surface.Format = Format_Invalid;
    pOsInterface   = pState->pOsInterface;

    if(pParam->data == nullptr)
    {   // CMRT@UMD 
        PCM_HAL_SURFACE2D_ENTRY    pEntry;

        // Get the 2D Resource Entry
        pEntry = &pState->pUmdSurf2DTable[pParam->handle];

        // Get resource information
        Surface.OsResource = pEntry->OsResource;
        MOS_ZeroMemory(&Info, sizeof(RENDERHAL_GET_SURFACE_INFO));

        CM_CHK_MOSSTATUS(RenderHal_GetSurfaceInfo(
                  pOsInterface,
                  &Info,
                  &Surface));

        pParam->pitch = Surface.dwPitch;
        // Lock the resource
        MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

        if (pParam->lockFlag == CM_HAL_LOCKFLAG_READONLY)
        {
            LockFlags.ReadOnly = true;
        }
        else
        {
            LockFlags.WriteOnly = true;
        }

        LockFlags.ForceCached = true;	

        pParam->data = pOsInterface->pfnLockResource(
                        pOsInterface, 
                        &pEntry->OsResource,
                        &LockFlags);

    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to lock surface 2d resource.");
        hr = MOS_STATUS_UNKNOWN;
    }
    CM_CHK_NULL_RETURN_MOSSTATUS(pParam->data);

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Unlock the resource and return
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Unlock2DResource(
    PCM_HAL_STATE                           pState,                                         // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM     pParam)                                         // [in]  Pointer to 2D Param
{
    MOS_STATUS              hr              = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE          pOsInterface    = pState->pOsInterface;

    if(pParam->data == nullptr)
    {
        PCM_HAL_SURFACE2D_ENTRY     pEntry;

        // Get the 2D Resource Entry
        pEntry = &pState->pUmdSurf2DTable[pParam->handle];

        // UnLock the resource
        CM_HRESULT2MOSSTATUS_AND_CHECK(pOsInterface->pfnUnlockResource(pOsInterface, &(pEntry->OsResource)));
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to unlock surface 2d resource.");
        hr = MOS_STATUS_UNKNOWN;
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Get the GfxMap Filter based on the texture filter type
//| Returns: Result
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetGfxMapFilter(
    uint32_t                     filterMode,
    MHW_GFX3DSTATE_MAPFILTER     *pGfxFilter)
{
    MOS_STATUS hr;
    hr = MOS_STATUS_SUCCESS;

    switch(filterMode)
    {
    case CM_TEXTURE_FILTER_TYPE_LINEAR:
        *pGfxFilter = MHW_GFX3DSTATE_MAPFILTER_LINEAR;
        break;
    case CM_TEXTURE_FILTER_TYPE_POINT:
        *pGfxFilter = MHW_GFX3DSTATE_MAPFILTER_NEAREST;
        break;
    case CM_TEXTURE_FILTER_TYPE_ANISOTROPIC:
        *pGfxFilter = MHW_GFX3DSTATE_MAPFILTER_ANISOTROPIC;
        break;

    default:
        CM_ERROR_ASSERT("Filter '%d' not supported", filterMode);
        goto finish;
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Get the Gfx Texture Address based on the texture coordinate type
//| Returns: Result
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetGfxTextAddress(
    uint32_t                     addressMode,
    MHW_GFX3DSTATE_TEXCOORDMODE  *pGfxAddress)
{
    MOS_STATUS hr;
    hr = MOS_STATUS_SUCCESS;

    switch(addressMode)
    {

    case CM_TEXTURE_ADDRESS_WRAP:
        *pGfxAddress = MHW_GFX3DSTATE_TEXCOORDMODE_WRAP;
        break;
    case CM_TEXTURE_ADDRESS_MIRROR:
        *pGfxAddress = MHW_GFX3DSTATE_TEXCOORDMODE_MIRROR;
        break;
    case CM_TEXTURE_ADDRESS_CLAMP:
        *pGfxAddress = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
        break;
    case CM_TEXTURE_ADDRESS_BORDER:
        *pGfxAddress = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP_BORDER;
        break;
        
    default:
        CM_ERROR_ASSERT("Address '%d' not supported", addressMode);
        goto finish;
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    If WA required to set SLM in L3
//| Returns:    Display corruption observed when running MDF workload.
//|             This issue is related to SLM setting in L3. 
//|             To resolve this problem, we need to disable SLM after 
//|             command submission. 
//*-----------------------------------------------------------------------------
bool HalCm_IsWaSLMinL3Cache_Linux()
{
    bool flag;
#if ANDROID
    flag = false;
#else
    flag = true;
#endif
    return flag;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Enable GPU frequency Turbo boost on Linux 
//| Returns:    MOS_STATUS_SUCCESS.
//*-----------------------------------------------------------------------------
#define I915_CONTEXT_PRIVATE_PARAM_BOOST 0x80000000
MOS_STATUS HalCm_EnableTurboBoost_Linux(    
    PCM_HAL_STATE             pState)
{
#ifndef ANDROID
    struct drm_i915_gem_context_param CtxParam;
    int32_t RetVal = 0;

    MOS_ZeroMemory( &CtxParam, sizeof( CtxParam ) );
    CtxParam.param = I915_CONTEXT_PRIVATE_PARAM_BOOST;
    CtxParam.value = 1;
    RetVal = drmIoctl( pState->pOsInterface->pOsContext->fd, 
                      DRM_IOCTL_I915_GEM_CONTEXT_SETPARAM, &CtxParam );
#endif
    //if drmIoctl fail, we will stay in normal mode.
    return MOS_STATUS_SUCCESS;
}

void HalCm_OsInitInterface(
    PCM_HAL_STATE           pCmState)          // [out]  pointer to CM State
{
    CM_ASSERT(pCmState);

    pCmState->pfnGetSurface2DPitchAndSize            = HalCm_GetSurface2DPitchAndSize_Linux;
    pCmState->pfnRegisterKMDNotifyEventHandle        = HalCm_RegisterKMDNotifyEventHandle_Linux;
    pCmState->pfnAllocateBuffer                      = HalCm_AllocateBuffer_Linux;
    pCmState->pfnAllocateSurface2DUP                 = HalCm_AllocateSurface2DUP_Linux;
    pCmState->pfnAllocate3DResource                  = HalCm_Allocate3DResource_Linux;
    pCmState->pfnGetGPUCurrentFrequency              = HalCm_GetGPUCurrentFrequency_Linux;
    pCmState->pfnGetGpuTime                          = HalCm_GetGpuTime_Linux;
    pCmState->pfnGetPlatformInfo                     = HalCm_GetPlatformInfo_Linux;
    pCmState->pfnGetGTSystemInfo                     = HalCm_GetGTSystemInfo_Linux;
    pCmState->pfnReferenceCommandBuffer              = HalCm_ReferenceCommandBuf_Linux;
    pCmState->pfnSetCommandBufferResource            = HalCm_SetCommandBufResource_Linux;
    pCmState->pfnQueryTask                           = HalCm_QueryTask_Linux;
    pCmState->pfnWriteGPUStatusTagToCMTSResource     = HalCm_WriteGPUStatusTagToCMTSResource_Linux;
    pCmState->pfnIsWASLMinL3Cache                    = HalCm_IsWaSLMinL3Cache_Linux;
    pCmState->pfnEnableTurboBoost                    = HalCm_EnableTurboBoost_Linux;

    HalCm_GetLibDrmVMapFnt(pCmState);
    return;
}

//*-------------------------------------------------------------------------------------
//| Purpose:    Add PipeControl with a Conditional Time Stamp (valid/invalid time stamp)
//| Returns:    On Success, right before Pipe Control commands, insert commands to write
//|             time stamp to Sync Location. When the condition same as following 
//|             "conditional buffer end" command is assert, the time stamp is a valid value, 
//|             otherwise an invalid time stamp is used to write Sync Location.
//*-------------------------------------------------------------------------------------

MOS_STATUS HalCm_OsAddArtifactConditionalPipeControl(
    PCM_HAL_MI_REG_OFFSETS pOffsets,
    PCM_HAL_STATE pState,
    PMOS_COMMAND_BUFFER pCmdBuffer, //commmand buffer
    int32_t iSyncOffset,   //offset to syncation of time stamp
    PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS pConditionalParams)    //comparing Params
{
    MOS_STATUS                   hr = MOS_STATUS_SUCCESS;
    MHW_MI_LOAD_REGISTER_REG_PARAMS     LoadRegRegParams;
    MHW_MI_LOAD_REGISTER_IMM_PARAMS     LoadRegImmParams;
    MHW_MI_LOAD_REGISTER_MEM_PARAMS     LoadRegMemParams;

    MHW_MI_STORE_REGISTER_MEM_PARAMS    StoreRegParams;
    MHW_MI_STORE_DATA_PARAMS            StoreDataParams;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    MHW_MI_MATH_PARAMS                  MathParams;
    MHW_MI_ALU_PARAMS                   AluParams[20];
    MHW_MI_STORE_REGISTER_MEM_PARAMS    StoreRegMemParams;
    MHW_PIPE_CONTROL_PARAMS             PipeCtlParams;

    PMHW_MI_INTERFACE  pMhwMiInterface = pState->pRenderHal->pMhwMiInterface;

    MOS_ZeroMemory(&LoadRegRegParams, sizeof(LoadRegRegParams));
    LoadRegRegParams.dwSrcRegister = pOffsets->TimeStampOffset;                                                  //read time stamp 
    LoadRegRegParams.dwDstRegister = pOffsets->GPROffset + 0;                                                    
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiLoadRegisterRegCmd(pCmdBuffer, &LoadRegRegParams));     
    LoadRegRegParams.dwSrcRegister = pOffsets->TimeStampOffset + 4;                                              
    LoadRegRegParams.dwDstRegister = pOffsets->GPROffset + 4;                                                    
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiLoadRegisterRegCmd(pCmdBuffer, &LoadRegRegParams));     //R0: time stamp

    MOS_ZeroMemory(&MathParams, sizeof(MathParams));
    MOS_ZeroMemory(&AluParams, sizeof(AluParams));

    AluParams[0].AluOpcode = MHW_MI_ALU_AND;

    // store      reg1, CF
    AluParams[1].AluOpcode = MHW_MI_ALU_STORE;
    AluParams[1].Operand1 = MHW_MI_ALU_GPREG1;
    AluParams[1].Operand2 = MHW_MI_ALU_CF;
    // store      reg2, CF
    AluParams[2].AluOpcode = MHW_MI_ALU_STORE;
    AluParams[2].Operand1 = MHW_MI_ALU_GPREG2;
    AluParams[2].Operand2 = MHW_MI_ALU_CF;                                                               
    // store      reg3, CF
    AluParams[2].AluOpcode = MHW_MI_ALU_STORE;
    AluParams[2].Operand1 = MHW_MI_ALU_GPREG3;
    AluParams[2].Operand2 = MHW_MI_ALU_CF;                                                                      //clear R1 -- R3
        
    MathParams.pAluPayload = AluParams;
    MathParams.dwNumAluParams = 3; 
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiMathCmd(pCmdBuffer, &MathParams));                     //MI cmd to clear R1 - R3

    MOS_ZeroMemory(&LoadRegMemParams, sizeof(LoadRegMemParams));
    LoadRegMemParams.presStoreBuffer = pConditionalParams->presSemaphoreBuffer;
    LoadRegMemParams.dwOffset = pConditionalParams->dwOffset;
    LoadRegMemParams.dwRegister = pOffsets->GPROffset + 8 * 1;
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiLoadRegisterMemCmd(pCmdBuffer, &LoadRegMemParams));    //R1: compared value 32bits, in coditional surface
    if (!pConditionalParams->bDisableCompareMask) 
    {
        LoadRegMemParams.dwOffset = pConditionalParams->dwOffset + 4;
        LoadRegMemParams.dwRegister = pOffsets->GPROffset + 8 * 2;
        CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiLoadRegisterMemCmd(pCmdBuffer, &LoadRegMemParams)); //r1, r2: compared value and its mask  
        //load1 reg1, srca
        AluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
        AluParams[0].Operand1 = MHW_MI_ALU_SRCA;
        AluParams[0].Operand2 = MHW_MI_ALU_GPREG1;
        //load reg2, srcb
        AluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
        AluParams[1].Operand1 = MHW_MI_ALU_SRCB;
        AluParams[1].Operand2 = MHW_MI_ALU_GPREG2;
        //add
        AluParams[2].AluOpcode = MHW_MI_ALU_AND;
        //store reg1, accu
        AluParams[3].AluOpcode = MHW_MI_ALU_STORE;
        AluParams[3].Operand1 = MHW_MI_ALU_GPREG1;
        AluParams[3].Operand2 = MHW_MI_ALU_ACCU;                                                                     //REG14 = TS + 1

        MathParams.pAluPayload = AluParams;
        MathParams.dwNumAluParams = 4; 
        CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiMathCmd(pCmdBuffer, &MathParams));                     //R1 & R2 --> R1: compared value, to be used
    }

    MOS_ZeroMemory(&LoadRegImmParams, sizeof(LoadRegImmParams));
    LoadRegImmParams.dwData = pConditionalParams->dwValue;
    LoadRegImmParams.dwRegister = pOffsets->GPROffset + 8 * 2;
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiLoadRegisterImmCmd(pCmdBuffer, &LoadRegImmParams));    //R2: user value 32bits

    MOS_ZeroMemory(&LoadRegImmParams, sizeof(LoadRegImmParams));
    LoadRegImmParams.dwData = 1;
    LoadRegImmParams.dwRegister = pOffsets->GPROffset + 8 * 3;
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiLoadRegisterImmCmd(pCmdBuffer, &LoadRegImmParams));    //R3 = 1
    

    //load1 reg3, srca
    AluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
    AluParams[0].Operand1 = MHW_MI_ALU_SRCA;
    AluParams[0].Operand2 = MHW_MI_ALU_GPREG3;
    //load reg0, srcb
    AluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
    AluParams[1].Operand1 = MHW_MI_ALU_SRCB;
    AluParams[1].Operand2 = MHW_MI_ALU_GPREG0;
    //add
    AluParams[2].AluOpcode = MHW_MI_ALU_ADD;
    //store reg14, accu
    AluParams[3].AluOpcode = MHW_MI_ALU_STORE;
    AluParams[3].Operand1 = MHW_MI_ALU_GPREG14;
    AluParams[3].Operand2 = MHW_MI_ALU_ACCU;                                                                     //REG14 = TS + 1
    
    // load     srcB, reg1
    AluParams[4].AluOpcode = MHW_MI_ALU_LOAD;
    AluParams[4].Operand1 = MHW_MI_ALU_SRCB;
    AluParams[4].Operand2 = MHW_MI_ALU_GPREG1;                                               //load compared val 
    // load     srcA, reg2
    AluParams[5].AluOpcode = MHW_MI_ALU_LOAD;
    AluParams[5].Operand1 = MHW_MI_ALU_SRCA;
    AluParams[5].Operand2 = MHW_MI_ALU_GPREG2;                                              //load user val
    // sub      srcA, srcB
    AluParams[6].AluOpcode = MHW_MI_ALU_SUB;                                                //if (compared > user) 1 ==> CF otherwise 0 ==> CF
    // store      reg4, CF
    AluParams[7].AluOpcode = MHW_MI_ALU_STOREINV;
    AluParams[7].Operand1 = MHW_MI_ALU_GPREG4;
    AluParams[7].Operand2 = MHW_MI_ALU_CF;                                                 //!CF ==> R4, mask
    //load      reg4, srcA
    AluParams[8].AluOpcode = MHW_MI_ALU_LOAD;   
    AluParams[8].Operand1 = MHW_MI_ALU_SRCA;
    AluParams[8].Operand2 = MHW_MI_ALU_GPREG4;
    //load reg14, SRCB
    AluParams[9].AluOpcode = MHW_MI_ALU_LOAD;
    AluParams[9].Operand1 = MHW_MI_ALU_SRCB;
    AluParams[9].Operand2 = MHW_MI_ALU_GPREG14;
    //and
    AluParams[10].AluOpcode = MHW_MI_ALU_AND;                                             //0 or TS+1 (!CF & TS)

    //store reg6, accu  
    AluParams[11].AluOpcode = MHW_MI_ALU_STORE;                                       //R6 = (TS+1) (to terminate) or 0 (to continue)
    AluParams[11].Operand1 = MHW_MI_ALU_GPREG6;
    AluParams[11].Operand2 = MHW_MI_ALU_ACCU;

    //invalud time stamp is all '1's for MDF
    //load reg6, SRCA
    AluParams[12].AluOpcode = MHW_MI_ALU_LOAD;
    AluParams[12].Operand1 = MHW_MI_ALU_SRCA;
    AluParams[12].Operand2 = MHW_MI_ALU_GPREG6;
    //load1 SRCB
    AluParams[13].AluOpcode = MHW_MI_ALU_LOAD;
    AluParams[13].Operand1 = MHW_MI_ALU_SRCB;
    AluParams[13].Operand2 = MHW_MI_ALU_GPREG3;
    //sub
    AluParams[14].AluOpcode = MHW_MI_ALU_SUB;                                             //-1 or TS (!CF & TS)
    //store reg7, accu  
    AluParams[15].AluOpcode = MHW_MI_ALU_STORE;                                       //R7 = (TS) (to terminate) or all '1'  ( -1 to continue)
    AluParams[15].Operand1 = MHW_MI_ALU_GPREG7;
    AluParams[15].Operand2 = MHW_MI_ALU_ACCU;
    
    
    MathParams.pAluPayload = AluParams;
    MathParams.dwNumAluParams = 16;
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiMathCmd(pCmdBuffer, &MathParams));            //set artifact time stamp (-1 or TS) per condition in GPR R7

    //store R6 to synclocation
    MOS_ZeroMemory(&StoreRegMemParams, sizeof(StoreRegMemParams));
    StoreRegMemParams.presStoreBuffer = &pState->Render_TsResource.OsResource;
    StoreRegMemParams.dwOffset = iSyncOffset + sizeof(uint64_t);
    StoreRegMemParams.dwRegister = pOffsets->GPROffset + 8 * 7; 
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiStoreRegisterMemCmd(pCmdBuffer, &StoreRegMemParams));
    StoreRegMemParams.presStoreBuffer = &pState->Render_TsResource.OsResource;
    StoreRegMemParams.dwOffset = iSyncOffset + sizeof(uint64_t) + 4;
    StoreRegMemParams.dwRegister = pOffsets->GPROffset + 4 + 8 * 7 ;
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddMiStoreRegisterMemCmd(pCmdBuffer, &StoreRegMemParams));

    // Insert a pipe control for synchronization
    PipeCtlParams = g_cRenderHal_InitPipeControlParams;
    PipeCtlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
    PipeCtlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, &PipeCtlParams));

    PipeCtlParams.dwFlushMode = MHW_FLUSH_READ_CACHE;
    CM_CHK_MOSSTATUS(pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, &PipeCtlParams));
    
finish:
    return hr;
};

uint32_t HalCm_GetNumCmdBuffers(PMOS_INTERFACE pOsInterface, uint32_t maxTaskNumber)
{
    UNUSED(maxTaskNumber);
    return 0;
}

MOS_STATUS HalCm_GetSipBinary(PCM_HAL_STATE pState)
{
	UNUSED(pState);
    // Function not implemented on Linux, just return success for sanity check
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HalCm_SetupSipSurfaceState(
    PCM_HAL_STATE               pState,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable)
{
    UNUSED(pState);
    UNUSED(pIndexParam);
    UNUSED(iBindingTable);
    // Function not implemented on Linux, just return success for sanity check
    return MOS_STATUS_SUCCESS;
}

