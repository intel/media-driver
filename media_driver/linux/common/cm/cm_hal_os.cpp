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
#include "i915_drm.h"
#include "cm_execution_adv.h"

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
    PMOS_RESOURCE    osResource)
{
    if (osResource && osResource->bo)
    {
        mos_bo_reference((MOS_LINUX_BO *)(osResource->bo));
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    unreferece the OsResource
//| Returns:    N/A
//*-----------------------------------------------------------------------------
void HalCm_OsResource_Unreference(
    PMOS_RESOURCE    osResource)
{
    if (osResource && osResource->bo)
    {
        mos_bo_unreference((MOS_LINUX_BO *)(osResource->bo));
        osResource->bo = nullptr;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Reference the Command Buffer and Pass it to event
//| Returns:    N/A
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_ReferenceCommandBuf_Linux(
    PMOS_RESOURCE     osResource,        //  [in]  Command Buffer's MOS Resurce
    void             **cmdBuffer)       // [out] Comamnd Buffer to pass to event
{
    if (osResource && osResource->bo)
    {
        mos_bo_reference((MOS_LINUX_BO *)(osResource->bo));
        *cmdBuffer = osResource->bo;
    }

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Command Buffer Resource and Pass it to event
//| Returns:    N/A
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_SetCommandBufResource_Linux(
    PMOS_RESOURCE     osResource,        //  [in]  Command Buffer's MOS Resurce
    void             **cmdBuffer)       // [out] Comamnd Buffer to pass to event
{
    if (osResource && osResource->bo)
    {
        *cmdBuffer = osResource->bo;
    }

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get 2D surface info and register to OS-Command-Buffer's patch list.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetSurfaceAndRegister(
    PCM_HAL_STATE                state,
    PRENDERHAL_SURFACE           renderHalSurface,
    CM_HAL_KERNEL_ARG_KIND       surfKind,
    uint32_t                     index,
    bool                         pixelPitch)
{
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;
    RENDERHAL_GET_SURFACE_INFO info;
    PRENDERHAL_INTERFACE     renderHal     = state->renderHal;
    PMOS_SURFACE             surface       = &renderHalSurface->OsSurface;
    PRENDERHAL_MEDIA_STATE   mediaState = nullptr;

    if (!renderHalSurface)
    {
        goto finish;
    }
    else
    {
        MOS_ZeroMemory(renderHalSurface, sizeof(RENDERHAL_SURFACE));
    }

    switch(surfKind)
    {
        case CM_ARGUMENT_STATE_BUFFER:
            mediaState = state->pfnGetMediaStatePtrForSurfaceIndex( state, index );
            CM_CHK_HRESULT_GOTOFINISH_MOSERROR(renderHal->pOsInterface->pfnRegisterResource(
                renderHal->pOsInterface, mediaState->pDynamicState->memoryBlock.GetResource(), true, true));
            surface->OsResource.user_provided_va = state->pfnGetStateBufferVAPtrForSurfaceIndex( state, index );
            surface->dwWidth = mediaState->pDynamicState->Curbe.dwSize;
            surface->dwHeight = 1;
            surface->Format = Format_RAW;
            return MOS_STATUS_SUCCESS; // state buffer's OS resource belong to DSH, so don't need sync it and just return directly.

        case CM_ARGUMENT_SURFACEBUFFER:
            surface->dwWidth      = state->bufferTable[index].size;
            surface->dwHeight     = 1;
            surface->Format       = Format_RAW;
            renderHalSurface->rcSrc.right  = surface->dwWidth;
            renderHalSurface->rcSrc.bottom = surface->dwHeight;
            renderHalSurface->rcDst        = renderHalSurface->rcSrc;
            CM_CHK_HRESULT_GOTOFINISH_MOSERROR(renderHal->pOsInterface->pfnRegisterResource(
                renderHal->pOsInterface, &(state->bufferTable[index].osResource), true, true));
            surface->OsResource  = state->bufferTable[index].osResource;

        break;

        case CM_ARGUMENT_SURFACE3D:
            /* First register resource on Linux to get allocation slot on GpuContext */
            CM_CHK_HRESULT_GOTOFINISH_MOSERROR(renderHal->pOsInterface->pfnRegisterResource(
                renderHal->pOsInterface, &(state->surf3DTable[index].osResource), true, true));

            surface->OsResource   = state->surf3DTable[index].osResource;
            surface->Format       = Format_Invalid;

            MOS_ZeroMemory(&info, sizeof(RENDERHAL_GET_SURFACE_INFO));

            CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_GetSurfaceInfo(
                renderHal->pOsInterface,
                &info,
                surface));

            surface->Format       = state->surf3DTable[index].osResource.Format;
            renderHalSurface->rcSrc.right  = surface->dwWidth;
            renderHalSurface->rcSrc.bottom = surface->dwHeight;
            renderHalSurface->rcDst        = renderHalSurface->rcSrc;
        break;

        case CM_ARGUMENT_SURFACE2D:
            /* First register resource on Linux to get allocation slot on GpuContext */
            CM_CHK_HRESULT_GOTOFINISH_MOSERROR(renderHal->pOsInterface->pfnRegisterResource(
                renderHal->pOsInterface, &(state->umdSurf2DTable[index].osResource), true, true));

            surface->OsResource     = state->umdSurf2DTable[index].osResource;

            MOS_ZeroMemory(&info, sizeof(RENDERHAL_GET_SURFACE_INFO));
            CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_GetSurfaceInfo(
                renderHal->pOsInterface,
                &info,
                surface));

            if ( (surface->Format == Format_NV12 || surface->Format == Format_YV12 || surface->Format == Format_Y210 || surface->Format == Format_Y216)
                  && (!pixelPitch))
            {
                renderHalSurface->SurfType = RENDERHAL_SURF_OUT_RENDERTARGET;
            }

            surface->Format         = state->umdSurf2DTable[index].format;
            renderHalSurface->rcSrc.right  = surface->dwWidth;
            renderHalSurface->rcSrc.bottom = surface->dwHeight;
            renderHalSurface->rcDst        = renderHalSurface->rcSrc;
            break;

        case CM_ARGUMENT_SURFACE2D_UP:
            /* First register resource on Linux to get allocation slot on GpuContext */
            CM_CHK_HRESULT_GOTOFINISH_MOSERROR(renderHal->pOsInterface->pfnRegisterResource(
                renderHal->pOsInterface,  &(state->surf2DUPTable[index].osResource), true, true));

            // Get Details of 2D surface and fill the VPHAL surface
            surface->OsResource = state->surf2DUPTable[index].osResource;
            surface->dwWidth    = state->surf2DUPTable[index].width;
            surface->dwHeight   = state->surf2DUPTable[index].height;
            surface->Format     = state->surf2DUPTable[index].format;
            surface->dwDepth    = 1;
            surface->TileType   = MOS_TILE_LINEAR;
            //surface->Channel    = MOS_S3D_NONE;
            surface->dwOffset   = 0;

            if ( (surface->Format == Format_NV12 || surface->Format == Format_YV12 || surface->Format == Format_Y210 || surface->Format == Format_Y216)
                  && (!pixelPitch))
            {
                renderHalSurface->SurfType = RENDERHAL_SURF_OUT_RENDERTARGET;
            }

            MOS_ZeroMemory(&info, sizeof(RENDERHAL_GET_SURFACE_INFO));

            CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_GetSurfaceInfo(
                renderHal->pOsInterface,
                &info,
                surface));

            renderHalSurface->rcSrc.right  = surface->dwWidth;
            renderHalSurface->rcSrc.bottom = surface->dwHeight;
            renderHalSurface->rcDst        = renderHalSurface->rcSrc;
            break;

        case CM_ARGUMENT_VME_STATE:
            /* First register resource on Linux to get allocation slot on GpuContext */
            CM_CHK_HRESULT_GOTOFINISH_MOSERROR(renderHal->pOsInterface->pfnRegisterResource(
                renderHal->pOsInterface, &(state->umdSurf2DTable[index].osResource), true, true));

            surface->OsResource = state->umdSurf2DTable[index].osResource;

            MOS_ZeroMemory(&info, sizeof(RENDERHAL_GET_SURFACE_INFO));

            CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_GetSurfaceInfo(
                renderHal->pOsInterface,
                &info,
                surface));

            surface->Format     = state->umdSurf2DTable[index].format;

            if (!state->cmHalInterface->IsSupportedVMESurfaceFormat(surface->Format))
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CM_ASSERTMESSAGE("Invalid VME surface format");
                goto finish;
            }

            renderHalSurface->rcSrc.right  = surface->dwWidth;
            renderHalSurface->rcSrc.bottom = surface->dwHeight;
            renderHalSurface->rcDst        = renderHalSurface->rcSrc;
            break;

        case CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS:
        case CM_ARGUMENT_SURFACE_SAMPLER8X8_VA:
            /* First register resource on Linux to get allocation slot on GpuContext */
            CM_CHK_HRESULT_GOTOFINISH_MOSERROR(renderHal->pOsInterface->pfnRegisterResource(
                renderHal->pOsInterface, &(state->umdSurf2DTable[index].osResource), true, true));

            // Get Details of 2D surface and fill the VPHAL surface
            surface->OsResource  = state->umdSurf2DTable[index].osResource;

            MOS_ZeroMemory(&info, sizeof(RENDERHAL_GET_SURFACE_INFO));

            CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_GetSurfaceInfo(
                renderHal->pOsInterface,
                &info,
                surface));

            surface->Format        = state->umdSurf2DTable[index].osResource.Format;
            renderHalSurface->rcSrc.right  = surface->dwWidth;
            renderHalSurface->rcSrc.bottom = surface->dwHeight;
            renderHalSurface->rcDst        = renderHalSurface->rcSrc;
            break;

        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CM_ASSERTMESSAGE(
                "Argument kind '%d' is not supported", surfKind);
            goto finish;
    }

    //Tag-based Sync on the Resource/surface
    CM_CHK_MOSSTATUS_GOTOFINISH(HalCm_SyncOnResource(state, surface, true));

    eStatus = MOS_STATUS_SUCCESS;
finish:
    return eStatus;
}

//===============<Os-dependent DDI Functions>============================================
//*-----------------------------------------------------------------------------
//| Purpose:    Get 2D surface pitch and physical size for SURFACE2D_UP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetSurfPitchSize(
    uint32_t width, uint32_t height, MOS_FORMAT format, uint32_t *pitch, uint32_t *physicalSize, PCM_HAL_STATE state)
{

    MOS_STATUS           eStatus = MOS_STATUS_SUCCESS;
    GMM_RESOURCE_INFO*   gmmResInfo = nullptr;
    GMM_RESOURCE_FLAG    gmmFlags;
    GMM_RESCREATE_PARAMS gmmParams;

    MOS_ZeroMemory( &gmmFlags, sizeof( GMM_RESOURCE_FLAG ) );
    MOS_ZeroMemory( &gmmParams, sizeof( GMM_RESCREATE_PARAMS ) );

    if( nullptr == state || 
        nullptr == state->osInterface ||
        nullptr == pitch ||
        nullptr == physicalSize)
    {
        eStatus = MOS_STATUS_NULL_POINTER;
        goto finish;
    }

    gmmFlags.Info.Linear    = true;
    gmmFlags.Info.Cacheable = true;
    gmmFlags.Gpu.Texture    = true;

    gmmParams.Type           = RESOURCE_2D;
    gmmParams.Format         = state->osInterface->pfnFmt_MosToGmm( format );
    gmmParams.Flags          = gmmFlags;
    gmmParams.BaseWidth      = width;
    gmmParams.BaseHeight     = height;
    gmmParams.Depth          = 1;
    gmmParams.ArraySize      = 1;
    gmmParams.NoGfxMemory    = true;

    // get pitch and size
    if (nullptr == state ||
        nullptr == state->osInterface)
    {
        eStatus = MOS_STATUS_NULL_POINTER;
        goto finish;
    }
    gmmResInfo = state->osInterface->pfnGetGmmClientContext(state->osInterface)->CreateResInfoObject(&gmmParams);
    if (gmmResInfo != nullptr)
    {
        *pitch             = static_cast<uint32_t>( gmmResInfo->GetRenderPitch() );
        *physicalSize      = static_cast<uint32_t>( gmmResInfo->GetSizeSurface() );
        state->osInterface->pfnGetGmmClientContext(state->osInterface)->DestroyResInfoObject(gmmResInfo);
    }
    else
    {
        *pitch = 0;
        *physicalSize = 0;
    }

finish:
    return eStatus;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Get 2D surface pitch and physical size for SURFACE2D_UP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetSurface2DPitchAndSize_Linux(
    PCM_HAL_STATE                   state,                                             // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_UP_PARAM      param)                                             // [in]  Pointer to Buffer Param
{
    return HalCm_GetSurfPitchSize(param->width, param->height, param->format,
                                  &param->pitch, &param->physicalSize, state);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Register APP/Runtime-level created Event Handle as a UMD Object;
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_RegisterUMDNotifyEventHandle_Linux(
    PCM_HAL_STATE             state,
    PCM_HAL_OSSYNC_PARAM      syncParam)
{
    MOS_STATUS              eStatus;
    UNUSED(state);
    UNUSED(syncParam);
    //-----------------------------------------
    CM_ASSERT(state);
    CM_ASSERT(syncParam);
    //-----------------------------------------
    eStatus               = MOS_STATUS_SUCCESS;

    return eStatus;
}

uint32_t HalCm_GetSurf2DUPBaseWidth( uint32_t width, uint32_t pitch, MOS_FORMAT format)
{
    uint32_t baseWidth = width;
    uint32_t pixelSize = 1;

    switch(format)
    {
        case Format_L8 :
        case Format_P8 :
        case Format_A8 :
        case Format_NV12:
            pixelSize = 1;
            break;

        case Format_X8R8G8B8    :
        case Format_A8R8G8B8    :
        case Format_A8B8G8R8    :
        case Format_R32F        :
            pixelSize = 4;
            break;

        case Format_V8U8        :
        case Format_YUY2        :
        case Format_UYVY        :
            pixelSize = 2;
            break;

        default:
            CM_ASSERTMESSAGE("Error: Unsupported surface format.");
            break;
    }

    baseWidth = pitch/pixelSize;
    return baseWidth;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Linear Buffer or BufferUP
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateBuffer_Linux(
    PCM_HAL_STATE           state,                                             // [in]  Pointer to CM State
    PCM_HAL_BUFFER_PARAM    param)                                             // [in]  Pointer to Buffer Param
{
    MOS_STATUS              eStatus;
    PMOS_INTERFACE          osInterface;
    PCM_HAL_BUFFER_ENTRY    entry = nullptr;
    MOS_ALLOC_GFXRES_PARAMS allocParams;
    uint32_t                i;
    uint32_t                size;
    uint32_t                tileformat;
    const char              *fmt;
    PMOS_RESOURCE           osResource;
    MOS_LINUX_BO             *bo = nullptr;

    size  = param->size;
    tileformat = I915_TILING_NONE;

    //-----------------------------------------------
    CM_ASSERT(param->size > 0);
    //-----------------------------------------------

    eStatus        = MOS_STATUS_SUCCESS;
    osInterface    = state->renderHal->pOsInterface;

    // Find a free slot
    for (i = 0; i < state->cmDeviceParam.maxBufferTableSize; i++)
    {
        if (state->bufferTable[i].size == 0)
        {
            entry              = &state->bufferTable[i];
            param->handle      = (uint32_t)i;
            break;
        }
    }

    if (!entry)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Buffer table is full");
        goto finish;
    }

    // State buffer doesn't need any MOS RESOURCE, so it will return directly after getting a position in the buffer table
    if ( param->type == CM_BUFFER_STATE )
    {
        entry->size = param->size;
        entry->isAllocatedbyCmrtUmd = false;
        return eStatus;
    }

    osResource = &(entry->osResource);

    if (param->isAllocatedbyCmrtUmd)
    {
        // Resets the Resource
        Mos_ResetResource(osResource);

        if (param->data == nullptr)
        {
            MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParams.Type          = MOS_GFXRES_BUFFER;
            allocParams.TileType      = MOS_TILE_LINEAR;
            allocParams.dwBytes       = param->size;
            allocParams.pSystemMemory = param->data;
            allocParams.Format        = Format_Buffer;  //used in VpHal_OsAllocateResource_Linux!

            if (param->type == CM_BUFFER_N)
            {
                allocParams.pBufName = "CmBuffer";
            }
            else if (param->type == CM_BUFFER_STATELESS)
            {
                allocParams.pBufName = "CmBufferStateless";
            }

            CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnAllocateResource(
                osInterface,
                &allocParams,
                &entry->osResource));
        }
        else  //BufferUP
        {
#if defined(DRM_IOCTL_I915_GEM_USERPTR)
           bo =  mos_bo_alloc_userptr(osInterface->pOsContext->bufmgr,
                                 "CM Buffer UP",
                                 (void *)(param->data),
                                 tileformat,
                                 ROUND_UP_TO(size,MOS_PAGE_SIZE),
                                 ROUND_UP_TO(size,MOS_PAGE_SIZE),
#if defined(ANDROID)
                                 I915_USERPTR_UNSYNCHRONIZED
#else
                 0
#endif
                 );
#else
           bo =  mos_bo_alloc_vmap(osInterface->pOsContext->bufmgr,
                                "CM Buffer UP",
                                (void *)(param->data),
                                tileformat,
                                ROUND_UP_TO(size,MOS_PAGE_SIZE),
                                ROUND_UP_TO(size,MOS_PAGE_SIZE),
#if defined(ANDROID)
                                 I915_USERPTR_UNSYNCHRONIZED
#else
                 0
#endif
                 );
#endif

            osResource->bMapped = false;
            if (bo)
            {
                osResource->Format   = Format_Buffer;
                osResource->iWidth   = ROUND_UP_TO(size,MOS_PAGE_SIZE);
                osResource->iHeight  = 1;
                osResource->iPitch   = ROUND_UP_TO(size,MOS_PAGE_SIZE);
                osResource->bo       = bo;
                osResource->TileType = LinuxToMosTileType(tileformat);
                osResource->pData    = (uint8_t*) bo->virt;
            }
            else
            {
                fmt = "BufferUP";
                CM_ASSERTMESSAGE("Fail to Alloc BufferUP %7d bytes (%d x %d %s resource)\n",size, size, 1, fmt);
                eStatus = MOS_STATUS_UNKNOWN;
            }
            osResource->bConvertedFromDDIResource = true;
        }
    }
    else
    {
        entry->osResource = *param->mosResource;
        HalCm_OsResource_Reference(&entry->osResource);
    }

    entry->size = param->size;
    entry->isAllocatedbyCmrtUmd = param->isAllocatedbyCmrtUmd;
    entry->surfaceStateEntry[0].surfaceStateSize = entry->size;
    entry->surfaceStateEntry[0].surfaceStateOffset = 0;
    entry->surfaceStateEntry[0].surfaceStateMOCS = 0;
    if(param->type == CM_BUFFER_STATELESS)
    {
        state->statelessBufferUsed = true;

        // get GPU virtual address
        // Fix: pfnGetResourceGfxAddress is not implemented. Need to find solution to
        // get the GFX address.
        param->gfxAddress = osInterface->pfnGetResourceGfxAddress(osInterface,
                                                                  &(entry->osResource));
        entry->address = reinterpret_cast<void *>(param->gfxAddress);
    }

    if (state->advExecutor)
    {
        entry->surfStateMgr = state->advExecutor->CreateBufferStateMgr(&entry->osResource);
        state->advExecutor->SetBufferOrigSize(entry->surfStateMgr, entry->size);
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Allocate Surface2DUP (zero-copy, map system memory to video address space)
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_AllocateSurface2DUP_Linux(
    PCM_HAL_STATE state,               // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_UP_PARAM param)  // [in]  Pointer to Buffer Param
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE osInterface = state->renderHal->pOsInterface;
    PCM_HAL_SURFACE2D_UP_ENTRY entry = nullptr;

    MOS_ALLOC_GFXRES_PARAMS allocParams;

    //-----------------------------------------------
    CM_ASSERT(state);
    CM_ASSERT(param->width > 0 && param->height > 0);
    //-----------------------------------------------

    // Find a free slot
    for (uint32_t i = 0; i < state->cmDeviceParam.max2DSurfaceUPTableSize; ++i)
    {
        if (state->surf2DUPTable[i].width == 0)
        {
            entry              = &state->surf2DUPTable[i];
            param->handle      = (uint32_t)i;
            break;
        }
    }
    if (!entry)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Surface2DUP table is full");
        goto finish;
    }
    MOS_ZeroMemory(&allocParams, sizeof(allocParams));
    allocParams.Type          = MOS_GFXRES_2D;
    allocParams.TileType      = MOS_TILE_LINEAR;
    allocParams.dwWidth       = param->width;
    allocParams.dwHeight      = param->height;
    allocParams.pSystemMemory = param->data; 
    allocParams.Format        = param->format;
    allocParams.pBufName      = "CmSurface2DUP";

    CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnAllocateResource(
        osInterface,
        &allocParams,
        &entry->osResource));

    entry->width  = param->width;
    entry->height = param->height;
    entry->format  = param->format;

    if (state->advExecutor)
    {
        entry->surfStateMgr = state->advExecutor->Create2DStateMgr(&entry->osResource);
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get GPU current frequency
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetGPUCurrentFrequency_Linux(
    PCM_HAL_STATE               state,                                         // [in]  Pointer to CM State
    uint32_t                    *currentFrequency)                                   // [out] Pointer to current frequency
{
    UNUSED(state);

    //-----------------------------------------
    CM_ASSERT(state);
    //-----------------------------------------
    *currentFrequency   = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HalCm_GetGpuTime_Linux(PCM_HAL_STATE state, uint64_t *gpuTime)
{
    UNUSED(state);
    *gpuTime = 0;

    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
// Purpose: Check if conditional batch buffer supported
// Returns: False on Linux
//*-----------------------------------------------------------------------------
bool HalCm_IsCbbEnabled(
    PCM_HAL_STATE                           state)
{
    return false;
}

//*-----------------------------------------------------------------------------
// Purpose: Wait kernel finished in state heap
// Returns: Result of the operation
//*-----------------------------------------------------------------------------
int32_t HalCm_SyncKernel(
    PCM_HAL_STATE                           state,
    uint32_t                                sync)
{
    int32_t                    eStatus = CM_SUCCESS;
    PRENDERHAL_INTERFACE       renderHal = state->renderHal;
    PRENDERHAL_STATE_HEAP      stateHeap = renderHal->pStateHeap;

    // Update Sync tags
    CM_CHK_MOSSTATUS_GOTOFINISH(renderHal->pfnRefreshSync(renderHal));

    while ( ( int32_t )( stateHeap->dwSyncTag - sync ) < 0 )
    {
        CM_CHK_MOSSTATUS_GOTOFINISH( renderHal->pfnRefreshSync( renderHal ) );
    }

finish:
    return eStatus;
}

//===============<Os-dependent Private/Non-DDI Functions, Part 2>============================================

//Require DRM VMAP patch,
//Referecing:
//    [Intel-gfx] [PATCH 21/21] drm/i915: Introduce vmap (mapping of user pages into video memory) ioctl
//    http://lists.freedesktop.org/archives/intel-gfx/2011-April/010241.html
void HalCm_GetLibDrmVMapFnt(
                 PCM_HAL_STATE           cmState)
{
    cmState->hLibModule = nullptr;
    cmState->drmVMap = nullptr;
    return ;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Gets platform information like slices/sub-slices/EUPerSubSlice etc.
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetPlatformInfo_Linux(
    PCM_HAL_STATE                  state,              // [in] Pointer to CM State
    PCM_PLATFORM_INFO               platformInfo,        // [out] Pointer to platformInfo
    bool                           euSaturated)
{

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MEDIA_SYSTEM_INFO             *gtSystemInfo;

    UNUSED(euSaturated);

    gtSystemInfo = state->osInterface->pfnGetGtSystemInfo(state->osInterface);

    platformInfo->numHWThreadsPerEU    = gtSystemInfo->ThreadCount / gtSystemInfo->EUCount;
    platformInfo->numEUsPerSubSlice    = gtSystemInfo->EUCount / gtSystemInfo->SubSliceCount;

    if (state->cmHalInterface->CheckMediaModeAvailability())
    {
        platformInfo->numSlices            = gtSystemInfo->SliceCount;
        platformInfo->numSubSlices         = gtSystemInfo->SubSliceCount;
    }
    else
    { // not use Slice/SubSlice count  set to 0
        platformInfo->numSlices = 0;
        platformInfo->numSubSlices = 0;
    }

    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Gets GT system information for which slices/sub-slices are enabled
//| Returns:  Result of the operation
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetGTSystemInfo_Linux(
    PCM_HAL_STATE                state,                // [in] Pointer to CM state
    PCM_GT_SYSTEM_INFO           pSystemInfo            // [out] Pointer to CM GT system info
)
{
    MEDIA_SYSTEM_INFO            *gtSystemInfo;
    CM_EXPECTED_GT_SYSTEM_INFO    expectedGTInfo;

    gtSystemInfo = state->osInterface->pfnGetGtSystemInfo(state->osInterface);

    pSystemInfo->numMaxSlicesSupported    = gtSystemInfo->MaxSlicesSupported;
    pSystemInfo->numMaxSubSlicesSupported = gtSystemInfo->MaxSubSlicesSupported;

    state->cmHalInterface->GetExpectedGtSystemConfig(&expectedGTInfo);

    // check numSlices/SubSlices enabled equal the expected number for this GT
    // if match, pSystemInfo->isSliceInfoValid = true, else pSystemInfo->isSliceInfoValid = false
    if ((expectedGTInfo.numSlices    == gtSystemInfo->SliceCount) &&
        (expectedGTInfo.numSubSlices == gtSystemInfo->SubSliceCount))
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
        for(uint32_t i = 0; i < gtSystemInfo->SliceCount; ++i)
        {
            pSystemInfo->sliceInfo[i].Enabled = true;
            for(uint32_t j = 0; j < gtSystemInfo->SubSliceCount; ++j)
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
    PCM_HAL_STATE             state,
    PCM_HAL_QUERY_TASK_PARAM  queryParam)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE    renderHal;
    int64_t                 *piSyncStart;
    int64_t                 *piSyncEnd;
    uint64_t                ticks;
    int32_t                 syncOffset;
    PRENDERHAL_STATE_HEAP   stateHeap;
    uint64_t                hwStartNs;
    uint64_t                hwEndNs;
    int32_t                 maxTasks;

    //-----------------------------------------
    CM_ASSERT(state);
    CM_ASSERT(queryParam);
    //-----------------------------------------

    maxTasks = (int32_t)state->cmDeviceParam.maxTasks;
    if ((queryParam->taskId < 0) || (queryParam->taskId >= maxTasks) ||
        (state->taskStatusTable[queryParam->taskId] == CM_INVALID_INDEX))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Invalid Task ID'%d'.", queryParam->taskId);
        goto finish;
    }

    renderHal = state->renderHal;
    stateHeap = renderHal->pStateHeap;
    syncOffset = state->pfnGetTaskSyncLocation(state, queryParam->taskId);
    piSyncStart = (int64_t*)(state->renderTimeStampResource.data + syncOffset);
    piSyncEnd = piSyncStart + 1;
    queryParam->taskDurationNs = CM_INVALID_INDEX;

    if (*piSyncStart == CM_INVALID_INDEX)
    {
        queryParam->status = CM_TASK_QUEUED;
    }
    else if (*piSyncEnd == CM_INVALID_INDEX)
    {
        queryParam->status = CM_TASK_IN_PROGRESS;
    }
    else
    {
        queryParam->status = CM_TASK_FINISHED;

        hwStartNs = HalCm_ConvertTicksToNanoSeconds(state, *piSyncStart);
        hwEndNs = HalCm_ConvertTicksToNanoSeconds(state, *piSyncEnd);

        ticks = *piSyncEnd - *piSyncStart;

        queryParam->taskDurationTicks = ticks;
        queryParam->taskHWStartTimeStampInTicks = *piSyncStart;
        queryParam->taskHWEndTimeStampInTicks   = *piSyncEnd;

        // Convert ticks to Nanoseconds
        queryParam->taskDurationNs = HalCm_ConvertTicksToNanoSeconds(state, ticks);

        queryParam->taskGlobalSubmitTimeCpu = state->taskTimeStamp->submitTimeInCpu[queryParam->taskId];
        CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnConvertToQPCTime(state->taskTimeStamp->submitTimeInGpu[queryParam->taskId], &queryParam->taskSubmitTimeGpu));
        CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnConvertToQPCTime(hwStartNs, &queryParam->taskHWStartTimeStamp));
        CM_CHK_MOSSTATUS_GOTOFINISH(state->pfnConvertToQPCTime(hwEndNs, &queryParam->taskHWEndTimeStamp));

        state->taskStatusTable[queryParam->taskId] = CM_INVALID_INDEX;
    }

finish:
    return eStatus;
}

MOS_STATUS HalCm_WriteGPUStatusTagToCMTSResource_Linux(
    PCM_HAL_STATE             state,
    PMOS_COMMAND_BUFFER       cmdBuffer,
    int32_t                   taskID,
    bool                      isVebox)
{
    UNUSED(state);
    UNUSED(cmdBuffer);
    UNUSED(taskID);
    UNUSED(isVebox);
    return MOS_STATUS_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Lock the resource and return
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Lock2DResource(
    PCM_HAL_STATE               state,                                         // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM     param)                                         // [in]  Pointer to 2D Param
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_LOCK_PARAMS             lockFlags;
    RENDERHAL_GET_SURFACE_INFO  info;

    MOS_SURFACE                 surface;
    PMOS_INTERFACE              osInterface = nullptr;

    if ((param->lockFlag != CM_HAL_LOCKFLAG_READONLY) && (param->lockFlag != CM_HAL_LOCKFLAG_WRITEONLY) )
    {
        CM_ASSERTMESSAGE("Invalid lock flag!");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    MOS_ZeroMemory(&surface, sizeof(surface));
    surface.Format = Format_Invalid;
    osInterface   = state->osInterface;

    if(param->data == nullptr)
    {   // CMRT@UMD
        PCM_HAL_SURFACE2D_ENTRY    entry;

        // Get the 2D Resource Entry
        entry = &state->umdSurf2DTable[param->handle];

        // Get resource information
        surface.OsResource = entry->osResource;
        MOS_ZeroMemory(&info, sizeof(RENDERHAL_GET_SURFACE_INFO));

        CM_CHK_MOSSTATUS_GOTOFINISH(RenderHal_GetSurfaceInfo(
                  osInterface,
                  &info,
                  &surface));

        param->pitch = surface.dwPitch;
        param->format = surface.Format;
        param->YSurfaceOffset = surface.YPlaneOffset;
        param->USurfaceOffset = surface.UPlaneOffset;
        param->VSurfaceOffset = surface.VPlaneOffset;

        // Lock the resource
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));

        if (param->lockFlag == CM_HAL_LOCKFLAG_READONLY)
        {
            lockFlags.ReadOnly = true;
        }
        else
        {
            lockFlags.WriteOnly = true;
        }

        lockFlags.ForceCached = true;

        param->data = osInterface->pfnLockResource(
                        osInterface,
                        &entry->osResource,
                        &lockFlags);

    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to lock surface 2d resource.");
        eStatus = MOS_STATUS_UNKNOWN;
    }
    CM_CHK_NULL_GOTOFINISH_MOSERROR(param->data);

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Unlock the resource and return
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_Unlock2DResource(
    PCM_HAL_STATE                           state,                                         // [in]  Pointer to CM State
    PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM     param)                                         // [in]  Pointer to 2D Param
{
    MOS_STATUS              eStatus        = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE          osInterface    = state->osInterface;

    if(param->data == nullptr)
    {
        PCM_HAL_SURFACE2D_ENTRY     entry;

        // Get the 2D Resource Entry
        entry = &state->umdSurf2DTable[param->handle];

        // UnLock the resource
        CM_CHK_HRESULT_GOTOFINISH_MOSERROR(osInterface->pfnUnlockResource(osInterface, &(entry->osResource)));
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to unlock surface 2d resource.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Get the GfxMap Filter based on the texture filter type
//| Returns: Result
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetGfxMapFilter(
    uint32_t                     filterMode,
    MHW_GFX3DSTATE_MAPFILTER     *gfxFilter)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    switch(filterMode)
    {
    case CM_TEXTURE_FILTER_TYPE_LINEAR:
        *gfxFilter = MHW_GFX3DSTATE_MAPFILTER_LINEAR;
        break;
    case CM_TEXTURE_FILTER_TYPE_POINT:
        *gfxFilter = MHW_GFX3DSTATE_MAPFILTER_NEAREST;
        break;
    case CM_TEXTURE_FILTER_TYPE_ANISOTROPIC:
        *gfxFilter = MHW_GFX3DSTATE_MAPFILTER_ANISOTROPIC;
        break;

    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Filter '%d' not supported", filterMode);
        goto finish;
    }

finish:
    return eStatus;
}

//*-----------------------------------------------------------------------------
//| Purpose: Get the Gfx Texture Address based on the texture coordinate type
//| Returns: Result
//*-----------------------------------------------------------------------------
MOS_STATUS HalCm_GetGfxTextAddress(
    uint32_t                     addressMode,
    MHW_GFX3DSTATE_TEXCOORDMODE  *gfxAddress)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    switch(addressMode)
    {

    case CM_TEXTURE_ADDRESS_WRAP:
        *gfxAddress = MHW_GFX3DSTATE_TEXCOORDMODE_WRAP;
        break;
    case CM_TEXTURE_ADDRESS_MIRROR:
        *gfxAddress = MHW_GFX3DSTATE_TEXCOORDMODE_MIRROR;
        break;
    case CM_TEXTURE_ADDRESS_CLAMP:
        *gfxAddress = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP;
        break;
    case CM_TEXTURE_ADDRESS_BORDER:
        *gfxAddress = MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP_BORDER;
        break;

    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CM_ASSERTMESSAGE("Address '%d' not supported", addressMode);
        goto finish;
    }

finish:
    return eStatus;
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
    PCM_HAL_STATE             state)
{
#ifndef ANDROID
    struct drm_i915_gem_context_param ctxParam;
    int32_t retVal = 0;

    MOS_ZeroMemory( &ctxParam, sizeof( ctxParam ) );
    ctxParam.param = I915_CONTEXT_PRIVATE_PARAM_BOOST;
    ctxParam.value = 1;
    retVal = drmIoctl( state->osInterface->pOsContext->fd,
                      DRM_IOCTL_I915_GEM_CONTEXT_SETPARAM, &ctxParam );
#endif
    //if drmIoctl fail, we will stay in normal mode.
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Updates tracker resource used in state heap management
//! \param    [in] state
//!           CM HAL State
//! \param    [in,out] cmdBuffer
//!           Command buffer containing the workload
//! \param    [in] tag
//|           Tag to write to tracker resource
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS HalCm_UpdateTrackerResource_Linux(
    PCM_HAL_STATE       state,
    PMOS_COMMAND_BUFFER cmdBuffer,
    uint32_t            tag)
{
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_GPU_CONTEXT          gpuContext = MOS_GPU_CONTEXT_INVALID_HANDLE;
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;

    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    gpuContext = state->renderHal->pOsInterface->CurrentGpuContextOrdinal;
    if (gpuContext == MOS_GPU_CONTEXT_VEBOX)
    {
        MOS_RESOURCE osResource = state->renderHal->veBoxTrackerRes.osResource;
        storeDataParams.pOsResource = &osResource;
    }
    else
    {
        state->renderHal->trackerProducer.GetLatestTrackerResource(state->renderHal->currentTrackerIndex,
                            &storeDataParams.pOsResource,
                            &storeDataParams.dwResourceOffset);
    }

    storeDataParams.dwValue = tag;
    eStatus = state->renderHal->pMhwMiInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams);
    return eStatus;
}

uint32_t HalCm_RegisterStream(CM_HAL_STATE *state)
{
    return state->osInterface->streamIndex;
}

void HalCm_OsInitInterface(
    PCM_HAL_STATE           cmState)          // [out]  pointer to CM State
{
    CM_ASSERT(cmState);

    cmState->pfnGetSurface2DPitchAndSize            = HalCm_GetSurface2DPitchAndSize_Linux;
    cmState->pfnRegisterUMDNotifyEventHandle        = HalCm_RegisterUMDNotifyEventHandle_Linux;
    cmState->pfnAllocateBuffer                      = HalCm_AllocateBuffer_Linux;
    cmState->pfnAllocateSurface2DUP                 = HalCm_AllocateSurface2DUP_Linux;
    cmState->pfnGetGPUCurrentFrequency              = HalCm_GetGPUCurrentFrequency_Linux;
    cmState->pfnGetGpuTime                          = HalCm_GetGpuTime_Linux;
    cmState->pfnGetPlatformInfo                     = HalCm_GetPlatformInfo_Linux;
    cmState->pfnGetGTSystemInfo                     = HalCm_GetGTSystemInfo_Linux;
    cmState->pfnReferenceCommandBuffer              = HalCm_ReferenceCommandBuf_Linux;
    cmState->pfnSetCommandBufferResource            = HalCm_SetCommandBufResource_Linux;
    cmState->pfnQueryTask                           = HalCm_QueryTask_Linux;
    cmState->pfnIsWASLMinL3Cache                    = HalCm_IsWaSLMinL3Cache_Linux;
    cmState->pfnEnableTurboBoost                    = HalCm_EnableTurboBoost_Linux;
    cmState->pfnUpdateTrackerResource               = HalCm_UpdateTrackerResource_Linux;
    cmState->pfnRegisterStream                      = HalCm_RegisterStream;

    HalCm_GetLibDrmVMapFnt(cmState);
    cmState->syncOnResource                         = false;
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
    PCM_HAL_MI_REG_OFFSETS offsets,
    PCM_HAL_STATE state,
    PMOS_COMMAND_BUFFER cmdBuffer, //commmand buffer
    int32_t syncOffset,   //offset to syncation of time stamp
    PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS conditionalParams, //comparing Params
    uint32_t trackerTag)    
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    MHW_MI_LOAD_REGISTER_REG_PARAMS     loadRegRegParams;
    MHW_MI_LOAD_REGISTER_IMM_PARAMS     loadRegImmParams;
    MHW_MI_LOAD_REGISTER_MEM_PARAMS     loadRegMemParams;

    MHW_MI_STORE_REGISTER_MEM_PARAMS    storeRegParams;
    MHW_MI_STORE_DATA_PARAMS            storeDataParams;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    MHW_MI_MATH_PARAMS                  mathParams;
    MHW_MI_ALU_PARAMS                   aluParams[20];
    MHW_MI_STORE_REGISTER_MEM_PARAMS    storeRegMemParams;
    MHW_PIPE_CONTROL_PARAMS             pipeCtrlParams;

    PMHW_MI_INTERFACE  mhwMiInterface = state->renderHal->pMhwMiInterface;

    MOS_ZeroMemory(&loadRegRegParams, sizeof(loadRegRegParams));                                                
    loadRegRegParams.dwSrcRegister = offsets->timeStampOffset;         //read time stamp 
    loadRegRegParams.dwDstRegister = offsets->gprOffset + 0;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiLoadRegisterRegCmd(cmdBuffer, &loadRegRegParams));
    loadRegRegParams.dwSrcRegister = offsets->timeStampOffset + 4;
    loadRegRegParams.dwDstRegister = offsets->gprOffset + 4;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiLoadRegisterRegCmd(cmdBuffer, &loadRegRegParams));     //R0: time stamp

    MOS_ZeroMemory(&mathParams, sizeof(mathParams));
    MOS_ZeroMemory(&aluParams, sizeof(aluParams));

    aluParams[0].AluOpcode = MHW_MI_ALU_AND;

    // store      reg1, CF
    aluParams[1].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[1].Operand1 = MHW_MI_ALU_GPREG1;
    aluParams[1].Operand2 = MHW_MI_ALU_CF;
    // store      reg2, CF
    aluParams[2].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[2].Operand1 = MHW_MI_ALU_GPREG2;
    aluParams[2].Operand2 = MHW_MI_ALU_CF;
    // store      reg3, CF
    aluParams[2].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[2].Operand1 = MHW_MI_ALU_GPREG3;
    aluParams[2].Operand2 = MHW_MI_ALU_CF;                                                                      //clear R1 -- R3

    mathParams.pAluPayload = aluParams;
    mathParams.dwNumAluParams = 3;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiMathCmd(cmdBuffer, &mathParams));                     //MI cmd to clear R1 - R3

    MOS_ZeroMemory(&loadRegMemParams, sizeof(loadRegMemParams));
    loadRegMemParams.presStoreBuffer = conditionalParams->presSemaphoreBuffer;
    loadRegMemParams.dwOffset = conditionalParams->dwOffset;
    loadRegMemParams.dwRegister = offsets->gprOffset + 8 * 1;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &loadRegMemParams));    //R1: compared value 32bits, in coditional surface
                                                                                                  // Load current tracker tag from resource to R8
    MOS_ZeroMemory(&loadRegMemParams, sizeof(loadRegMemParams));
    state->renderHal->trackerProducer.GetLatestTrackerResource(state->renderHal->currentTrackerIndex,
                                &loadRegMemParams.presStoreBuffer,
                                &loadRegMemParams.dwOffset);
    loadRegMemParams.dwRegister = offsets->gprOffset+ 8 * 8;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &loadRegMemParams));  // R8: current tracker tag

                                                                                                // Load new tag passed to this function to R9
    MOS_ZeroMemory(&loadRegImmParams, sizeof(loadRegImmParams));
    loadRegImmParams.dwData = trackerTag;
    loadRegImmParams.dwRegister = offsets->gprOffset+ 8 * 9;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &loadRegImmParams));  // R9: new tracker tag
                                                                                                //

    if (!conditionalParams->bDisableCompareMask)
    {
        loadRegMemParams.presStoreBuffer = conditionalParams->presSemaphoreBuffer;
        loadRegMemParams.dwOffset = conditionalParams->dwOffset + 4;
        loadRegMemParams.dwRegister = offsets->gprOffset + 8 * 2;
        CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &loadRegMemParams)); //r1, r2: compared value and its mask
        //load1 reg1, srca
        aluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[0].Operand1 = MHW_MI_ALU_SRCA;
        aluParams[0].Operand2 = MHW_MI_ALU_GPREG1;
        //load reg2, srcb
        aluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
        aluParams[1].Operand1 = MHW_MI_ALU_SRCB;
        aluParams[1].Operand2 = MHW_MI_ALU_GPREG2;
        //add
        aluParams[2].AluOpcode = MHW_MI_ALU_AND;
        //store reg1, accu
        aluParams[3].AluOpcode = MHW_MI_ALU_STORE;
        aluParams[3].Operand1 = MHW_MI_ALU_GPREG1;
        aluParams[3].Operand2 = MHW_MI_ALU_ACCU;                                                                     //REG14 = TS + 1

        mathParams.pAluPayload = aluParams;
        mathParams.dwNumAluParams = 4;
        CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiMathCmd(cmdBuffer, &mathParams));                     //R1 & R2 --> R1: compared value, to be used
    }

    MOS_ZeroMemory(&loadRegImmParams, sizeof(loadRegImmParams));
    loadRegImmParams.dwData = conditionalParams->dwValue;
    loadRegImmParams.dwRegister = offsets->gprOffset + 8 * 2;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &loadRegImmParams));    //R2: user value 32bits

    MOS_ZeroMemory(&loadRegImmParams, sizeof(loadRegImmParams));
    loadRegImmParams.dwData = 1;
    loadRegImmParams.dwRegister = offsets->gprOffset + 8 * 3;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &loadRegImmParams));    //R3 = 1

    //load1 reg3, srca
    aluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[0].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[0].Operand2 = MHW_MI_ALU_GPREG3;
    //load reg0, srcb
    aluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[1].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[1].Operand2 = MHW_MI_ALU_GPREG0;
    //add
    aluParams[2].AluOpcode = MHW_MI_ALU_ADD;
    //store reg14, accu
    aluParams[3].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[3].Operand1 = MHW_MI_ALU_GPREG14;
    aluParams[3].Operand2 = MHW_MI_ALU_ACCU;                                                                     //REG14 = TS + 1

    // load     srcB, reg1
    aluParams[4].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[4].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[4].Operand2 = MHW_MI_ALU_GPREG1;                                               //load compared val
    // load     srcA, reg2
    aluParams[5].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[5].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[5].Operand2 = MHW_MI_ALU_GPREG2;                                              //load user val
    // sub      srcA, srcB
    aluParams[6].AluOpcode = MHW_MI_ALU_SUB;                                                //if (compared > user) 1 ==> CF otherwise 0 ==> CF
    // store      reg4, CF
    aluParams[7].AluOpcode = MHW_MI_ALU_STOREINV;
    aluParams[7].Operand1 = MHW_MI_ALU_GPREG4;
    aluParams[7].Operand2 = MHW_MI_ALU_CF;                                                 //!CF ==> R4, mask
    //load      reg4, srcA
    aluParams[8].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[8].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[8].Operand2 = MHW_MI_ALU_GPREG4;
    //load reg14, SRCB
    aluParams[9].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[9].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[9].Operand2 = MHW_MI_ALU_GPREG14;
    //and
    aluParams[10].AluOpcode = MHW_MI_ALU_AND;                                             //0 or TS+1 (!CF & TS)

    //store reg6, accu
    aluParams[11].AluOpcode = MHW_MI_ALU_STORE;                                       //R6 = (TS+1) (to terminate) or 0 (to continue)
    aluParams[11].Operand1 = MHW_MI_ALU_GPREG6;
    aluParams[11].Operand2 = MHW_MI_ALU_ACCU;

    //invalud time stamp is all '1's for MDF
    //load reg6, SRCA
    aluParams[12].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[12].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[12].Operand2 = MHW_MI_ALU_GPREG6;
    //load1 SRCB
    aluParams[13].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[13].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[13].Operand2 = MHW_MI_ALU_GPREG3;
    //sub
    aluParams[14].AluOpcode = MHW_MI_ALU_SUB;                                             //-1 or TS (!CF & TS)
    //store reg7, accu
    aluParams[15].AluOpcode = MHW_MI_ALU_STORE;                                       //R7 = (TS) (to terminate) or all '1'  ( -1 to continue)
    aluParams[15].Operand1 = MHW_MI_ALU_GPREG7;
    aluParams[15].Operand2 = MHW_MI_ALU_ACCU;

    mathParams.pAluPayload = aluParams;
    mathParams.dwNumAluParams = 16;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiMathCmd(cmdBuffer, &mathParams));            //set artifact time stamp (-1 or TS) per condition in GPR R7

    // Add R3 (has value 1) to R4 (~CF) and store result in R10
    aluParams[0].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[0].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[0].Operand2 = MHW_MI_ALU_GPREG3;

    aluParams[1].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[1].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[1].Operand2 = MHW_MI_ALU_GPREG4;

    aluParams[2].AluOpcode = MHW_MI_ALU_ADD;

    aluParams[3].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[3].Operand1 = MHW_MI_ALU_GPREG10;
    aluParams[3].Operand2 = MHW_MI_ALU_ACCU;

    // AND R8 (current tracker tag) with R10 (~CF + 1) and store in R11
    aluParams[4].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[4].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[4].Operand2 = MHW_MI_ALU_GPREG8;

    aluParams[5].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[5].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[5].Operand2 = MHW_MI_ALU_GPREG10;

    aluParams[6].AluOpcode = MHW_MI_ALU_AND;

    aluParams[7].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[7].Operand1 = MHW_MI_ALU_GPREG11;
    aluParams[7].Operand2 = MHW_MI_ALU_ACCU;

    // Store inverse of R10 (-1 --> continue, 0 --> terminate) to R12
    aluParams[8].AluOpcode = MHW_MI_ALU_STOREINV;
    aluParams[8].Operand1 = MHW_MI_ALU_GPREG12;
    aluParams[8].Operand2 = MHW_MI_ALU_GPREG10;

    // AND R9 (new tracker tag) and R12 and store in R13
    aluParams[9].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[9].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[9].Operand2 = MHW_MI_ALU_GPREG9;

    aluParams[10].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[10].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[10].Operand2 = MHW_MI_ALU_GPREG12;

    aluParams[11].AluOpcode = MHW_MI_ALU_AND;

    aluParams[12].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[12].Operand1 = MHW_MI_ALU_GPREG13;
    aluParams[12].Operand2 = MHW_MI_ALU_ACCU;

    // ADD R11 and R13 and store in R15
    aluParams[13].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[13].Operand1 = MHW_MI_ALU_SRCA;
    aluParams[13].Operand2 = MHW_MI_ALU_GPREG11;

    aluParams[14].AluOpcode = MHW_MI_ALU_LOAD;
    aluParams[14].Operand1 = MHW_MI_ALU_SRCB;
    aluParams[14].Operand2 = MHW_MI_ALU_GPREG13;

    aluParams[15].AluOpcode = MHW_MI_ALU_ADD;

    aluParams[16].AluOpcode = MHW_MI_ALU_STORE;
    aluParams[16].Operand1 = MHW_MI_ALU_GPREG15;
    aluParams[16].Operand2 = MHW_MI_ALU_ACCU;

    mathParams.pAluPayload = aluParams;
    mathParams.dwNumAluParams = 17;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiMathCmd(cmdBuffer, &mathParams));

    // Store R15 to trackerResource
    MOS_ZeroMemory(&storeRegMemParams, sizeof(storeRegMemParams));
    state->renderHal->trackerProducer.GetLatestTrackerResource(state->renderHal->currentTrackerIndex,
                                            &storeRegMemParams.presStoreBuffer,
                                            &storeRegMemParams.dwOffset);
    storeRegMemParams.dwRegister = offsets->gprOffset+ 8 * 15;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &storeRegMemParams));

    //store R6 to synclocation
    MOS_ZeroMemory(&storeRegMemParams, sizeof(storeRegMemParams));
    storeRegMemParams.presStoreBuffer = &state->renderTimeStampResource.osResource;
    storeRegMemParams.dwOffset = syncOffset + sizeof(uint64_t);
    storeRegMemParams.dwRegister = offsets->gprOffset + 8 * 7;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &storeRegMemParams));
    storeRegMemParams.presStoreBuffer = &state->renderTimeStampResource.osResource;
    storeRegMemParams.dwOffset = syncOffset + sizeof(uint64_t) + 4;
    storeRegMemParams.dwRegister = offsets->gprOffset + 4 + 8 * 7 ;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &storeRegMemParams));

    // Insert a pipe control for synchronization
    pipeCtrlParams = g_cRenderHal_InitPipeControlParams;
    pipeCtrlParams.dwPostSyncOp = MHW_FLUSH_NOWRITE;
    pipeCtrlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(cmdBuffer, nullptr, &pipeCtrlParams));

    pipeCtrlParams.dwFlushMode = MHW_FLUSH_READ_CACHE;
    CM_CHK_MOSSTATUS_GOTOFINISH(mhwMiInterface->AddPipeControl(cmdBuffer, nullptr, &pipeCtrlParams));

finish:
    return eStatus;
};

uint32_t HalCm_GetNumCmdBuffers(PMOS_INTERFACE osInterface, uint32_t maxTaskNumber)
{
    UNUSED(maxTaskNumber);
    return 0;
}

MOS_STATUS HalCm_GetSipBinary(PCM_HAL_STATE state)
{
    UNUSED(state);
    // Function not implemented on Linux, just return success for sanity check
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HalCm_SetupSipSurfaceState(
    PCM_HAL_STATE               state,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable)
{
    UNUSED(state);
    UNUSED(indexParam);
    UNUSED(bindingTable);
    // Function not implemented on Linux, just return success for sanity check
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Prepare virtual engine hint parametere
//! \details  Prepare virtual engine hint parameter for CCS node
//! \param    PCM_HAL_STATE state
//!           [in] Pointer to CM_HAL_STATE Structure
//! \param    bool bScalable
//!           [in] is scalable pipe or single pipe
//! \param    PMOS_VIRTUALENGINE_HINT_PARAMS pVeHintParam
//!           [out] Pointer to prepared VE hint parameter struct
//! \return   MOS_STATUS
//!
MOS_STATUS HalCm_PrepareVEHintParam(
    PCM_HAL_STATE                  state,
    bool                           bScalable,
    PMOS_VIRTUALENGINE_HINT_PARAMS pVeHintParam)
{
    return MOS_STATUS_UNIMPLEMENTED;
}


//!
//! \brief    Decompress the surface
//! \details  Decompress the media compressed surface
//! \param    PCM_HAL_STATE state
//!           [in] Pointer to CM_HAL_STATE Structure
//! \param    PCM_HAL_KERNEL_ARG_PARAM argParam
//!           [in]Pointer to HAL cm kernel argrument parameter
//! \param    uint32_t threadIndex
//!           [in] is used to get index of surface array
//! \return   MOS_STATUS
//!
MOS_STATUS HalCm_DecompressSurface(
    PCM_HAL_STATE              state,
    PCM_HAL_KERNEL_ARG_PARAM   argParam,
    uint32_t                   threadIndex)
{
    MOS_STATUS                 eStatus = MOS_STATUS_SUCCESS;
    uint32_t                   handle = 0;
    uint8_t                    *src = nullptr;
    PCM_HAL_SURFACE2D_ENTRY    pEntry = nullptr;
    PMOS_RESOURCE              pOsResource = nullptr;
    PMOS_INTERFACE             pOsInterface = nullptr;
    GMM_RESOURCE_FLAG          GmmFlags = { 0 };

    //Get the index of  surface array handle from kernel data
    CM_ASSERT(argParam->unitSize == sizeof(handle));
    src = argParam->firstValue + (threadIndex * argParam->unitSize);
    handle = *((uint32_t *)src) & CM_SURFACE_MASK;
    if (handle == CM_NULL_SURFACE)
    {
        eStatus = MOS_STATUS_SUCCESS;
        goto finish;
    }

    pEntry = &state->umdSurf2DTable[handle];
    pOsResource = &pEntry->osResource;
    pOsInterface = state->osInterface;

    if (pOsResource->pGmmResInfo)
    {
        GmmFlags = pOsResource->pGmmResInfo->GetResFlags();
        if (GmmFlags.Gpu.MMC || pOsResource->pGmmResInfo->IsMediaMemoryCompressed(0))
        {
            PMOS_CONTEXT pOsContext = pOsInterface->pOsContext;
            MOS_OS_ASSERT(pOsContext);
            MOS_OS_ASSERT(pOsContext->ppMediaMemDecompState);
            MOS_OS_ASSERT(pOsContext->pfnMemoryDecompress);
            pOsContext->pfnMemoryDecompress(pOsContext, pOsResource);
        }
    }

finish:
    return eStatus;
}

MOS_STATUS HalCm_SurfaceSync(
    PCM_HAL_STATE                pState,
    PMOS_SURFACE                 pSurface,
    bool                         bReadSync )
{
    UNUSED(pState);
    UNUSED(pSurface);
    UNUSED(bReadSync);

    return MOS_STATUS_SUCCESS;
}
