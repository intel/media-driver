/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     media_libva_interface_next.cpp
//! \brief    libva interface next implementaion.
//!

#if !defined(ANDROID) && defined(X11_FOUND)
#include <X11/Xutil.h>
#include "media_libva_putsurface_linux.h"
#endif

#include <drm_fourcc.h>

#include "media_libva_util_next.h"
#include "media_libva_interface_next.h"
#include "media_libva.h"
#include "mos_utilities.h"
#include "media_interfaces_mmd_next.h"
#include "media_libva_caps_next.h"
#include "media_ddi_prot.h"
#include "media_interfaces_hwinfo_device.h"
#include "mos_oca_interface_specific.h"
#include "media_interfaces_mcpy_next.h"
#include "ddi_decode_functions.h"
#include "ddi_encode_functions.h"
#include "ddi_vp_functions.h"
#include "media_libva_register.h"

MEDIA_MUTEX_T MediaLibvaInterfaceNext::m_GlobalMutex = MEDIA_MUTEX_INITIALIZER;

void MediaLibvaInterfaceNext::FreeForMediaContext(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNC_ENTER;

    MosUtilities::MosUnlockMutex(&m_GlobalMutex);

    if (mediaCtx)
    {
        mediaCtx->SkuTable.reset();
        mediaCtx->WaTable.reset();
        MOS_FreeMemory(mediaCtx->pSurfaceHeap);
        MOS_FreeMemory(mediaCtx->pBufferHeap);
        MOS_FreeMemory(mediaCtx->pImageHeap);
        MOS_FreeMemory(mediaCtx->pDecoderCtxHeap);
        MOS_FreeMemory(mediaCtx->pEncoderCtxHeap);
        MOS_FreeMemory(mediaCtx->pVpCtxHeap);
        MOS_FreeMemory(mediaCtx->pProtCtxHeap);
        mediaCtx->m_userSettingPtr.reset();
        MOS_Delete(mediaCtx);
    }

    return;
}

void MediaLibvaInterfaceNext::DestroyMediaContextMutex(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNC_ENTER;

    // destroy the mutexs
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->SurfaceMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->BufferMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->ImageMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->DecoderMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->EncoderMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->VpMutex);

#if !defined(ANDROID) && defined(X11_FOUND)
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->PutSurfaceRenderMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->PutSurfaceSwapBufferMutex);
#endif

    return;
}

PDDI_MEDIA_CONTEXT MediaLibvaInterfaceNext::CreateMediaDriverContext()
{
    PDDI_MEDIA_CONTEXT   mediaCtx;

    mediaCtx = MOS_New(DDI_MEDIA_CONTEXT);

    return mediaCtx;
}

VAStatus MediaLibvaInterfaceNext::HeapInitialize(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(mediaCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    // Heap initialization here
    mediaCtx->pSurfaceHeap = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr pSurfaceHeap", VA_STATUS_ERROR_ALLOCATION_FAILED);
    mediaCtx->pSurfaceHeap->uiHeapElementSize = sizeof(DDI_MEDIA_SURFACE_HEAP_ELEMENT);

    mediaCtx->pBufferHeap = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr BufferHeap", VA_STATUS_ERROR_ALLOCATION_FAILED);
    mediaCtx->pBufferHeap->uiHeapElementSize = sizeof(DDI_MEDIA_BUFFER_HEAP_ELEMENT);

    mediaCtx->pImageHeap = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    DDI_CHK_NULL(mediaCtx->pImageHeap, "nullptr ImageHeap", VA_STATUS_ERROR_ALLOCATION_FAILED);
    mediaCtx->pImageHeap->uiHeapElementSize = sizeof(DDI_MEDIA_IMAGE_HEAP_ELEMENT);

    mediaCtx->pDecoderCtxHeap = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    DDI_CHK_NULL(mediaCtx->pDecoderCtxHeap, "nullptr DecoderCtxHeap", VA_STATUS_ERROR_ALLOCATION_FAILED);
    mediaCtx->pDecoderCtxHeap->uiHeapElementSize = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    mediaCtx->pEncoderCtxHeap = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    DDI_CHK_NULL(mediaCtx->pEncoderCtxHeap, "nullptr EncoderCtxHeap", VA_STATUS_ERROR_ALLOCATION_FAILED);
    mediaCtx->pEncoderCtxHeap->uiHeapElementSize = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    mediaCtx->pVpCtxHeap = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    DDI_CHK_NULL(mediaCtx->pVpCtxHeap, "nullptr VpCtxHeap", VA_STATUS_ERROR_ALLOCATION_FAILED);
    mediaCtx->pVpCtxHeap->uiHeapElementSize = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    mediaCtx->pProtCtxHeap = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    DDI_CHK_NULL(mediaCtx->pProtCtxHeap, "nullptr pProtCtxHeap", VA_STATUS_ERROR_ALLOCATION_FAILED);
    mediaCtx->pProtCtxHeap->uiHeapElementSize = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    // init the mutexs
    MediaLibvaUtilNext::InitMutex(&mediaCtx->SurfaceMutex);
    MediaLibvaUtilNext::InitMutex(&mediaCtx->BufferMutex);
    MediaLibvaUtilNext::InitMutex(&mediaCtx->ImageMutex);
    MediaLibvaUtilNext::InitMutex(&mediaCtx->DecoderMutex);
    MediaLibvaUtilNext::InitMutex(&mediaCtx->EncoderMutex);
    MediaLibvaUtilNext::InitMutex(&mediaCtx->VpMutex);
    MediaLibvaUtilNext::InitMutex(&mediaCtx->ProtMutex);

    return VA_STATUS_SUCCESS;
}

#ifdef _MMC_SUPPORTED
void MediaLibvaInterfaceNext::MediaMemoryDecompressInternal(
    PMOS_CONTEXT  mosCtx,
    PMOS_RESOURCE osResource)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(mosCtx, "nullptr mosCtx",);
    DDI_CHK_NULL(osResource, "nullptr osResource",);

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    DDI_CHK_NULL(mediaMemDecompState, "nullptr mediaMemDecompState", );

    mediaMemDecompState->MemoryDecompress(osResource);
}

void MediaLibvaInterfaceNext::MediaMemoryCopyInternal(
    PMOS_CONTEXT  mosCtx,
    PMOS_RESOURCE inputOsResource,
    PMOS_RESOURCE outputOsResource,
    bool          boutputcompressed)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(mosCtx, "nullptr mosCtx",);
    DDI_CHK_NULL(inputOsResource, "nullptr input osResource",);
    DDI_CHK_NULL(outputOsResource, "nullptr output osResource",);

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    DDI_CHK_NULL(mediaMemDecompState, "nullptr mediaMemDecompState", );

    mediaMemDecompState->MediaMemoryCopy(inputOsResource, outputOsResource, boutputcompressed);
}

void MediaLibvaInterfaceNext::MediaMemoryCopy2DInternal(
    PMOS_CONTEXT  mosCtx,
    PMOS_RESOURCE inputOsResource,
    PMOS_RESOURCE outputOsResource,
    uint32_t      copyWidth,
    uint32_t      copyHeight,
    uint32_t      copyInputOffset,
    uint32_t      copyOutputOffset,
    uint32_t      bpp,
    bool          boutputcompressed)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(mosCtx, "nullptr mosCtx",);
    DDI_CHK_NULL(inputOsResource, "nullptr input osResource",);
    DDI_CHK_NULL(outputOsResource, "nullptr output osResource",);

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    DDI_CHK_NULL(mediaMemDecompState, "nullptr mediaMemDecompState", );

    mediaMemDecompState->MediaMemoryCopy2D(
            inputOsResource,
            outputOsResource,
            copyWidth,
            copyHeight,
            copyInputOffset,
            copyOutputOffset,
            bpp,
            boutputcompressed);
}

VAStatus MediaLibvaInterfaceNext::MediaMemoryTileConvertInternal(
    PMOS_CONTEXT  mosCtx,
    PMOS_RESOURCE inputOsResource,
    PMOS_RESOURCE outputOsResource,
    uint32_t      copyWidth,
    uint32_t      copyHeight,
    uint32_t      copyInputOffset,
    uint32_t      copyOutputOffset,
    bool          isTileToLinear,
    bool          outputCompressed)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(mosCtx, "nullptr mosCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(inputOsResource, "nullptr input osResource", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(outputOsResource, "nullptr output osResource", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = VA_STATUS_SUCCESS;

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    DDI_CHK_NULL(mediaMemDecompState, "nullptr mediaMemDecompState", VA_STATUS_ERROR_INVALID_PARAMETER);

    MOS_STATUS mosStatus = mediaMemDecompState->MediaMemoryTileConvert(
            inputOsResource,
            outputOsResource,
            copyWidth,
            copyHeight,
            copyInputOffset,
            copyOutputOffset,
            isTileToLinear,
            outputCompressed);
    if (mosStatus != MOS_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_UNKNOWN;
    }

    return vaStatus;
}

#endif

VAStatus MediaLibvaInterfaceNext::Initialize (
    VADriverContextP ctx,
    int32_t          devicefd,
    int32_t          *major_version,
    int32_t          *minor_version
)
{
    DDI_FUNC_ENTER;

#if !defined(ANDROID) && defined(X11_FOUND)
    // ATRACE code in <cutils/trace.h> started from KitKat, version = 440
    // ENABLE_TRACE is defined only for eng build so release build won't leak perf data
    // thus trace code enabled only on KitKat (and newer) && eng build
#if ANDROID_VERSION > 439 && defined(ENABLE_ATRACE)
    char switch_value[PROPERTY_VALUE_MAX];

    property_get("debug.DdiCodec_.umd", switch_value, "0");
    atrace_switch = atoi(switch_value);
#endif
#endif

    if(major_version)
    {
        *major_version = VA_MAJOR_VERSION;
    }

    if(minor_version)
    {
        *minor_version = VA_MINOR_VERSION;
    }

    MosUtilities::MosLockMutex(&m_GlobalMutex);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    if(mediaCtx)
    {
        FreeForMediaContext(mediaCtx);
    }

    mediaCtx = CreateMediaDriverContext();
    if (nullptr == mediaCtx)
    {
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    mediaCtx->uiRef++;
    ctx->pDriverData = (void *)mediaCtx;
    mediaCtx->fd     = devicefd;

#ifdef _MMC_SUPPORTED
    mediaCtx->pfnMemoryDecompress       = MediaMemoryDecompressInternal;
    mediaCtx->pfnMediaMemoryCopy        = MediaMemoryCopyInternal;
    mediaCtx->pfnMediaMemoryCopy2D      = MediaMemoryCopy2DInternal;
    mediaCtx->pfnMediaMemoryTileConvert = MediaMemoryTileConvertInternal;
#endif
    mediaCtx->modularizedGpuCtxEnabled = true;

    mediaCtx->m_userSettingPtr  = std::make_shared<MediaUserSetting::MediaUserSetting>();

    MOS_CONTEXT mosCtx          = {};
    mosCtx.fd                   = mediaCtx->fd;
    mosCtx.m_userSettingPtr     = mediaCtx->m_userSettingPtr;

    MosInterface::InitOsUtilities(&mosCtx);
    MosOcaInterfaceSpecific::InitInterface(&mosCtx);

    mediaCtx->pGtSystemInfo = (MEDIA_SYSTEM_INFO *)MOS_AllocAndZeroMemory(sizeof(MEDIA_SYSTEM_INFO));
    if (nullptr == mediaCtx->pGtSystemInfo)
    {
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (MosInterface::CreateOsDeviceContext(&mosCtx, &mediaCtx->m_osDeviceContext) != MOS_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Unable to create MOS device context.");
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }
    mediaCtx->pDrmBufMgr                = mosCtx.bufmgr;
    mediaCtx->iDeviceId                 = mosCtx.iDeviceId;
    mediaCtx->SkuTable                  = mosCtx.m_skuTable;
    mediaCtx->WaTable                   = mosCtx.m_waTable;
    *mediaCtx->pGtSystemInfo            = mosCtx.m_gtSystemInfo;
    mediaCtx->platform                  = mosCtx.m_platform;
    mediaCtx->m_auxTableMgr             = mosCtx.m_auxTableMgr;
    mediaCtx->pGmmClientContext         = mosCtx.pGmmClientContext;
    mediaCtx->m_useSwSwizzling          = mosCtx.bUseSwSwizzling;
    mediaCtx->m_tileYFlag               = mosCtx.bTileYFlag;
    mediaCtx->bIsAtomSOC                = mosCtx.bIsAtomSOC;
    mediaCtx->perfData                  = mosCtx.pPerfData;
#ifdef _MMC_SUPPORTED
    if (mosCtx.ppMediaMemDecompState == nullptr)
    {
        DDI_ASSERTMESSAGE("media decomp state is null.");
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }
    mediaCtx->pMediaMemDecompState      = *mosCtx.ppMediaMemDecompState;
#endif
    mediaCtx->pMediaCopyState           = *mosCtx.ppMediaCopyState;

    if (HeapInitialize(mediaCtx) != VA_STATUS_SUCCESS)
    {
        DestroyMediaContextMutex(mediaCtx);
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    mediaCtx->m_hwInfo = MediaInterfacesHwInfoDevice::CreateFactory(mediaCtx->platform);
    if(!mediaCtx->m_hwInfo)
    {
        DDI_ASSERTMESSAGE("Query hwInfo failed. Not supported GFX device.");
        DestroyMediaContextMutex(mediaCtx);
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    mediaCtx->m_capsNext = MediaLibvaCapsNext::CreateCaps(mediaCtx);
    if (!mediaCtx->m_capsNext)
    {
        DDI_ASSERTMESSAGE("Caps create failed. Not supported GFX device.");
        if(mediaCtx->m_hwInfo)
        {
            MOS_Delete(mediaCtx->m_hwInfo);
        }
        mediaCtx->m_hwInfo = nullptr;
        DestroyMediaContextMutex(mediaCtx);
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    ctx->max_image_formats = mediaCtx->m_capsNext->GetImageFormatsMaxNum();

#if !defined(ANDROID) && defined(X11_FOUND)
    MediaLibvaUtilNext::InitMutex(&mediaCtx->PutSurfaceRenderMutex);
    MediaLibvaUtilNext::InitMutex(&mediaCtx->PutSurfaceSwapBufferMutex);

    // try to open X11 lib, if fail, assume no X11 environment
    if (VA_STATUS_SUCCESS != ConnectX11(mediaCtx))
    {
        // assume no X11 environment. In current implementation,
        // PutSurface (Linux) needs X11 support, so just replace
        // it with a dummy version. DdiCodec_PutSurfaceDummy() will
        // return VA_STATUS_ERROR_UNIMPLEMENTED directly.
        ctx->vtable->vaPutSurface = NULL;
    }
    
    if (OutputDriInit(ctx) == false)
    {
        DDI_ASSERTMESSAGE("Output driver init path failed.");
    }
#endif

    MediaLibvaUtilNext::SetMediaResetEnableFlag(mediaCtx);

    if (InitCompList(mediaCtx) != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Caps init failed. Not supported GFX device.");
        MediaLibvaInterfaceNext::ReleaseCompList(mediaCtx);
        if(mediaCtx->m_hwInfo)
        {
            MOS_Delete(mediaCtx->m_hwInfo);
        }
        mediaCtx->m_hwInfo = nullptr;
        MOS_Delete(mediaCtx->m_capsNext);
        mediaCtx->m_capsNext = nullptr;
        DestroyMediaContextMutex(mediaCtx);
        ReleaseCompList(mediaCtx);
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    MosUtilities::MosUnlockMutex(&m_GlobalMutex);

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::InitCompList(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNC_ENTER;

    VAStatus status = VA_STATUS_SUCCESS;
    mediaCtx->m_compList[CompCommon] = MOS_New(DdiMediaFunctions);

    if(nullptr == mediaCtx->m_compList[CompCommon])
    {
        status = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DDI_ASSERTMESSAGE("MediaLibvaContextNext::Init: Unable to create compList CompCommon.");
        return status;
    }

    for(int i = CompCommon + 1; i < CompCount; i++)
    {
        if (FunctionsFactory::IsRegistered((CompType)i))
        {
            mediaCtx->m_compList[i] = FunctionsFactory::Create((CompType)i);
            if (nullptr == mediaCtx->m_compList[i])
            {
                status = VA_STATUS_ERROR_ALLOCATION_FAILED;
                DDI_ASSERTMESSAGE("MediaLibvaContextNext::Init: Unable to create compList %d.", i);
                return status;
            }
        }
        else
        {
            mediaCtx->m_compList[i] = mediaCtx->m_compList[CompCommon];
        }
    }

    return status;
}

void MediaLibvaInterfaceNext::ReleaseCompList(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNC_ENTER;

    MOS_Delete(mediaCtx->m_compList[CompCommon]);
    mediaCtx->m_compList[CompCommon] = nullptr;

    for(int i = CompCommon + 1; i < CompCount; i++)
    {
        if(nullptr != mediaCtx->m_compList[i])
        {
            if(FunctionsFactory::IsRegistered((CompType)i))
            {
                MOS_Delete(mediaCtx->m_compList[i]);
            }
            mediaCtx->m_compList[i] = nullptr;
        }
    }
}

void MediaLibvaInterfaceNext::FreeSurfaceHeapElements(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaCtx,            "nullptr mediaCtx", );

    PDDI_MEDIA_HEAP surfaceHeap = mediaCtx->pSurfaceHeap;
    DDI_CHK_NULL(surfaceHeap,         "nullptr surfaceHeap", );

    PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapBase = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surfaceHeap->pHeapBase;
    DDI_CHK_NULL(mediaSurfaceHeapBase, "nullptr mediaSurfaceHeapBase", );

    int32_t surfaceNums = mediaCtx->uiNumSurfaces;
    for (int32_t elementId = 0; elementId < surfaceNums; elementId++)
    {
        PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapElmt = &mediaSurfaceHeapBase[elementId];
        if (nullptr == mediaSurfaceHeapElmt->pSurface)
        {
            continue;
        }
        MediaLibvaUtilNext::FreeSurface(mediaSurfaceHeapElmt->pSurface);
        MOS_FreeMemory(mediaSurfaceHeapElmt->pSurface);
        MediaLibvaUtilNext::ReleasePMediaSurfaceFromHeap(surfaceHeap,mediaSurfaceHeapElmt->uiVaSurfaceID);
        mediaCtx->uiNumSurfaces--;
    }
}

void MediaLibvaInterfaceNext::FreeBufferHeapElements(VADriverContextP    ctx)
{
    DDI_FUNC_ENTER;

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,            "nullptr mediaCtx", );

    PDDI_MEDIA_HEAP  bufferHeap = mediaCtx->pBufferHeap;
    DDI_CHK_NULL(bufferHeap,          "nullptr bufferHeap", );

    PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapBase = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)bufferHeap->pHeapBase;
    DDI_CHK_NULL(mediaBufferHeapBase, "nullptr mediaBufferHeapBase", );

    int32_t bufNums = mediaCtx->uiNumBufs;
    for (int32_t elementId = 0; bufNums > 0; ++elementId)
    {
        PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapElmt = &mediaBufferHeapBase[elementId];
        if (nullptr == mediaBufferHeapElmt->pBuffer)
        {
            continue;
        }
        DestroyBuffer(ctx,mediaBufferHeapElmt->uiVaBufferID);
        //Ensure the non-empty buffer to be destroyed.
        --bufNums;
    }
}

void MediaLibvaInterfaceNext::FreeImageHeapElements(VADriverContextP    ctx)
{
    DDI_FUNC_ENTER;

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,            "nullptr mediaCtx", );

    PDDI_MEDIA_HEAP imageHeap = mediaCtx->pImageHeap;
    DDI_CHK_NULL(imageHeap,           "nullptr imageHeap", );

    PDDI_MEDIA_IMAGE_HEAP_ELEMENT mediaImageHeapBase = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)imageHeap->pHeapBase;
    DDI_CHK_NULL(mediaImageHeapBase,  "nullptr mediaImageHeapBase", );

    int32_t imageNums = mediaCtx->uiNumImages;
    for (int32_t elementId = 0; elementId < imageNums; ++elementId)
    {
        PDDI_MEDIA_IMAGE_HEAP_ELEMENT mediaImageHeapElmt = &mediaImageHeapBase[elementId];
        if (nullptr == mediaImageHeapElmt->pImage)
        {
            continue;
        }
        DestroyImage(ctx, mediaImageHeapElmt->uiVaImageID);
    }
}

void MediaLibvaInterfaceNext::FreeContextHeapElements(VADriverContextP ctx)
{
    DDI_FUNC_ENTER;

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,            "nullptr mediaCtx", );

    //Free EncoderContext
    PDDI_MEDIA_HEAP encoderContextHeap = mediaCtx->pEncoderCtxHeap;
    int32_t encCtxNums        = mediaCtx->uiNumEncoders;
    if (nullptr != encoderContextHeap)
    {
        FreeContextHeap(ctx, encoderContextHeap, DDI_MEDIA_VACONTEXTID_OFFSET_ENCODER, encCtxNums);
    }

    //Free DecoderContext
    PDDI_MEDIA_HEAP decoderContextHeap = mediaCtx->pDecoderCtxHeap;
    int32_t decCtxNums        = mediaCtx->uiNumDecoders;
    if (nullptr != decoderContextHeap)
    {
        FreeContextHeap(ctx, decoderContextHeap, DDI_MEDIA_VACONTEXTID_OFFSET_DECODER, decCtxNums);
    }

    //Free VpContext
    PDDI_MEDIA_HEAP vpContextHeap      = mediaCtx->pVpCtxHeap;
    int32_t vpctxNums         = mediaCtx->uiNumVPs;
    if (nullptr != vpContextHeap)
    {
        FreeContextHeap(ctx, vpContextHeap, DDI_MEDIA_VACONTEXTID_OFFSET_VP, vpctxNums);
    }

    //Free ProtContext
    PDDI_MEDIA_HEAP protContextHeap      = mediaCtx->pProtCtxHeap;
    int32_t protCtxNums         = mediaCtx->uiNumProts;
    if (nullptr != protContextHeap)
    {
        DdiMedia_FreeProtectedSessionHeap(ctx, protContextHeap, DDI_MEDIA_VACONTEXTID_OFFSET_PROT, protCtxNums);
    }

    mediaCtx->pMediaMemDecompState = nullptr;
}

VAStatus MediaLibvaInterfaceNext::HeapDestroy(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(mediaCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    // destroy heaps
    MOS_FreeMemory(mediaCtx->pSurfaceHeap->pHeapBase);
    MOS_FreeMemory(mediaCtx->pSurfaceHeap);

    MOS_FreeMemory(mediaCtx->pBufferHeap->pHeapBase);
    MOS_FreeMemory(mediaCtx->pBufferHeap);

    MOS_FreeMemory(mediaCtx->pImageHeap->pHeapBase);
    MOS_FreeMemory(mediaCtx->pImageHeap);

    MOS_FreeMemory(mediaCtx->pDecoderCtxHeap->pHeapBase);
    MOS_FreeMemory(mediaCtx->pDecoderCtxHeap);

    MOS_FreeMemory(mediaCtx->pEncoderCtxHeap->pHeapBase);
    MOS_FreeMemory(mediaCtx->pEncoderCtxHeap);

    MOS_FreeMemory(mediaCtx->pVpCtxHeap->pHeapBase);
    MOS_FreeMemory(mediaCtx->pVpCtxHeap);

    MOS_FreeMemory(mediaCtx->pProtCtxHeap->pHeapBase);
    MOS_FreeMemory(mediaCtx->pProtCtxHeap);

    // destroy the mutexs
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->SurfaceMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->BufferMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->ImageMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->DecoderMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->EncoderMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->VpMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->ProtMutex);

    //resource checking
    if (mediaCtx->uiNumSurfaces != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the surfaces.");
    }
    if (mediaCtx->uiNumBufs != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the buffers.");
    }
    if (mediaCtx->uiNumImages != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the images.");
    }
    if (mediaCtx->uiNumDecoders != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the decoders.");
    }
    if (mediaCtx->uiNumEncoders != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the encoders.");
    }
    if (mediaCtx->uiNumVPs != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the VPs.");
    }
    if (mediaCtx->uiNumProts != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the Prots.");
    }
    return VA_STATUS_SUCCESS;
}

void MediaLibvaInterfaceNext::FreeContextHeap(
    VADriverContextP ctx,
    PDDI_MEDIA_HEAP  contextHeap,
    int32_t          vaContextOffset,
    int32_t          ctxNums)
{
    DDI_FUNC_ENTER;

    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT mediaContextHeapBase = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)contextHeap->pHeapBase;
    DDI_CHK_NULL(mediaContextHeapBase,  "nullptr mediaContextHeapBase", );

    for (int32_t elementId = 0; elementId < ctxNums; ++elementId)
    {
        PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT mediaContextHeapElmt = &mediaContextHeapBase[elementId];
        if (nullptr == mediaContextHeapElmt->pVaContext)
        {
            continue;
        }
        VAContextID vaCtxID = (VAContextID)(mediaContextHeapElmt->uiVaContextID + vaContextOffset);
        DestroyContext(ctx, vaCtxID);
    }
}

VAStatus MediaLibvaInterfaceNext::Terminate(VADriverContextP ctx)
{
    DDI_FUNC_ENTER;
    MOS_CONTEXT mosCtx = {};

    DDI_CHK_NULL(ctx,       "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx   = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    mosCtx.fd               = mediaCtx->fd;
    mosCtx.m_userSettingPtr = mediaCtx->m_userSettingPtr;

    MosUtilities::MosLockMutex(&m_GlobalMutex);

#if !defined(ANDROID) && defined(X11_FOUND)
    DestroyX11Connection(mediaCtx);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->PutSurfaceRenderMutex);
    MediaLibvaUtilNext::DestroyMutex(&mediaCtx->PutSurfaceSwapBufferMutex);

    if (mediaCtx->m_capsNext)
    {
        if (mediaCtx->dri_output != nullptr)
        {
            if (mediaCtx->dri_output->handle)
            {
                dso_close(mediaCtx->dri_output->handle);
            }

            free(mediaCtx->dri_output);
            mediaCtx->dri_output = nullptr;
        }
    }
#endif

    if (mediaCtx->m_capsNext)
    {
        MOS_Delete(mediaCtx->m_capsNext);
        mediaCtx->m_capsNext = nullptr;
    }
    //destory resources
    FreeSurfaceHeapElements(mediaCtx);
    FreeBufferHeapElements(ctx);
    FreeImageHeapElements(ctx);
    FreeContextHeapElements(ctx);

    HeapDestroy(mediaCtx);
    DdiMediaProtected::FreeInstances();

    MosInterface::DestroyOsDeviceContext(mediaCtx->m_osDeviceContext);
    mediaCtx->m_osDeviceContext = MOS_INVALID_HANDLE;
    MOS_FreeMemory(mediaCtx->pGtSystemInfo);
    MosOcaInterfaceSpecific::UninitInterface();
    MosInterface::CloseOsUtilities(&mosCtx);

    ReleaseCompList(mediaCtx);
    if(mediaCtx->m_hwInfo)
    {
        MOS_Delete(mediaCtx->m_hwInfo);
    }
    mediaCtx->m_hwInfo = nullptr;

    if (mediaCtx->uiRef > 1)
    {
        mediaCtx->uiRef--;
        MosUtilities::MosUnlockMutex(&m_GlobalMutex);

        return VA_STATUS_SUCCESS;
    }
    mediaCtx->SkuTable.reset();
    mediaCtx->WaTable.reset();

    // release media driver context, ctx creation is behind the mos_utilities_init
    // If free earilier than MOS_utilities_close, memnja count error.
    MOS_Delete(mediaCtx);
    mediaCtx         = nullptr;
    ctx->pDriverData = nullptr;

    MosUtilities::MosUnlockMutex(&m_GlobalMutex);

    return VA_STATUS_SUCCESS;
}

#if !defined(ANDROID) && defined(X11_FOUND)

#define X11_LIB_NAME "libX11.so.6"

void MediaLibvaInterfaceNext::DestroyX11Connection(
    PDDI_MEDIA_CONTEXT mediaCtx
)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx", );
    DDI_CHK_NULL(mediaCtx->X11FuncTable, "nullptr X11FuncTable", );

    MosUtilities::MosFreeLibrary(mediaCtx->X11FuncTable->pX11LibHandle);
    MOS_FreeMemory(mediaCtx->X11FuncTable);
    mediaCtx->X11FuncTable = nullptr;

    return;
}

VAStatus MediaLibvaInterfaceNext::ConnectX11(
    PDDI_MEDIA_CONTEXT mediaCtx
)
{
    DDI_FUNC_ENTER;
    
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaCtx->X11FuncTable = (PDDI_X11_FUNC_TABLE)MOS_AllocAndZeroMemory(sizeof(DDI_X11_FUNC_TABLE));
    DDI_CHK_NULL(mediaCtx->X11FuncTable, "Allocation Failed for X11FuncTable", VA_STATUS_ERROR_ALLOCATION_FAILED);

    HMODULE    h_module   = nullptr;
    MOS_STATUS mos_status = MosUtilities::MosLoadLibrary(X11_LIB_NAME, &h_module);
    if (MOS_STATUS_SUCCESS != mos_status || nullptr == h_module)
    {
        DestroyX11Connection(mediaCtx);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    mediaCtx->X11FuncTable->pX11LibHandle = h_module;

    mediaCtx->X11FuncTable->pfnXCreateGC =
        MosUtilities::MosGetProcAddress(h_module, "XCreateGC");
    mediaCtx->X11FuncTable->pfnXFreeGC =
        MosUtilities::MosGetProcAddress(h_module, "XFreeGC");
    mediaCtx->X11FuncTable->pfnXCreateImage =
        MosUtilities::MosGetProcAddress(h_module, "XCreateImage");
    mediaCtx->X11FuncTable->pfnXDestroyImage =
        MosUtilities::MosGetProcAddress(h_module, "XDestroyImage");
    mediaCtx->X11FuncTable->pfnXPutImage =
        MosUtilities::MosGetProcAddress(h_module, "XPutImage");

    if (nullptr == mediaCtx->X11FuncTable->pfnXCreateGC     ||
        nullptr == mediaCtx->X11FuncTable->pfnXFreeGC       ||
        nullptr == mediaCtx->X11FuncTable->pfnXCreateImage  ||
        nullptr == mediaCtx->X11FuncTable->pfnXDestroyImage ||
        nullptr == mediaCtx->X11FuncTable->pfnXPutImage)
    {
        DestroyX11Connection(mediaCtx);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    return VA_STATUS_SUCCESS;
}

bool MediaLibvaInterfaceNext::OutputDriInit(VADriverContextP ctx)
{
    DDI_CHK_NULL(ctx, "nullptr ctx", false);

    PDDI_MEDIA_CONTEXT mediaDrvCtx = nullptr;
    mediaDrvCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaDrvCtx, "nullptr ctx", false);

    struct dso_handle *dso_handle = nullptr;
    struct dri_vtable *dri_vtable = nullptr;

    mediaDrvCtx->dri_output = nullptr;

    static const struct dso_symbol symbols[] = {
        { "va_dri_get_drawable",
          offsetof(struct dri_vtable, get_drawable) },
        { "va_dri_get_rendering_buffer",
          offsetof(struct dri_vtable, get_rendering_buffer) },
        { "va_dri_swap_buffer",
          offsetof(struct dri_vtable, swap_buffer) },
        { nullptr, }
    };

    mediaDrvCtx->dri_output = (va_dri_output*) calloc(1, sizeof(struct va_dri_output));
    if (!mediaDrvCtx->dri_output)
    {
        return false;
    }

    mediaDrvCtx->dri_output->handle = DsoOpen(LIBVA_X11_NAME);
    if (!mediaDrvCtx->dri_output->handle)
    {
        free(mediaDrvCtx->dri_output);
        mediaDrvCtx->dri_output = nullptr;
        return false;
    }

    dso_handle = mediaDrvCtx->dri_output->handle;
    dri_vtable = &mediaDrvCtx->dri_output->vtable;
    if (!DsoGetSymbols(dso_handle, dri_vtable, sizeof(*dri_vtable), symbols))
    {
        dso_close(mediaDrvCtx->dri_output->handle);
        free(mediaDrvCtx->dri_output);
        mediaDrvCtx->dri_output = nullptr;
        return false;
    }
    return true;
}

struct dso_handle* MediaLibvaInterfaceNext::DsoOpen(const char *path)
{
    struct dso_handle *h = nullptr;

    h = (dso_handle *)calloc(1, sizeof(*h));
    if (!h)
    {
        return nullptr;
    }

    if (path) 
    {
        h->handle = dlopen(path, RTLD_LAZY|RTLD_LOCAL);
        if (!h->handle)
        {
            dso_close(h);
            return nullptr;
        }
    }
    else
    {
        h->handle = RTLD_DEFAULT;
    }
    return h;
}

bool MediaLibvaInterfaceNext::DsoGetSymbols(
    struct dso_handle          *h,
    void                       *vtable,
    uint32_t                   vtable_length,
    const struct dso_symbol    *symbols
)
{
    DDI_CHK_NULL(h, "nullptr h", false);

    const struct dso_symbol *s = nullptr;
    if (nullptr == symbols)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    for (s = symbols; s->name != nullptr; s++) 
    {
        if (s->offset + sizeof(dso_generic_func) > vtable_length)
        {
            return false;
        }
        if (!GetSymbol(h, ((char *)vtable) + s->offset, s->name))
        {
            return false;
        }
    }
    return true;
}

bool MediaLibvaInterfaceNext::GetSymbol(
    struct dso_handle *h,
    void              *func_vptr,
    const char        *name)
{
    DDI_CHK_NULL(h, "nullptr h", false);
    DDI_CHK_NULL(func_vptr, "nullptr func_vptr", false);

    dso_generic_func func;
    dso_generic_func * const func_ptr = (dso_generic_func*) func_vptr;
    const char *error = nullptr;

    dlerror();
    func = (dso_generic_func)dlsym(h->handle, name);
    error = dlerror();
    if (error) 
    {
        fprintf(stderr, "error: failed to resolve %s(): %s\n", name, error);
        return false;
    }
    *func_ptr = func;
    return true;
}

#endif

VAStatus MediaLibvaInterfaceNext::LoadFunction(VADriverContextP ctx)
{
    DDI_FUNC_ENTER;
    
    DDI_CHK_NULL(ctx,         "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);

    struct VADriverVTable    *pVTable     = DDI_CODEC_GET_VTABLE(ctx);
    DDI_CHK_NULL(pVTable,     "nullptr pVTable",      VA_STATUS_ERROR_INVALID_CONTEXT);

    struct VADriverVTableVPP *pVTableVpp  = DDI_CODEC_GET_VTABLE_VPP(ctx);
    DDI_CHK_NULL(pVTableVpp,  "nullptr pVTableVpp",   VA_STATUS_ERROR_INVALID_CONTEXT);

#if VA_CHECK_VERSION(1,11,0)
    struct VADriverVTableProt *pVTableProt = DDI_CODEC_GET_VTABLE_PROT(ctx);
    DDI_CHK_NULL(pVTableProt,  "nullptr pVTableProt",   VA_STATUS_ERROR_INVALID_CONTEXT);
#endif

    ctx->version_major                       = VA_MAJOR_VERSION;
    ctx->version_minor                       = VA_MINOR_VERSION;
    ctx->max_profiles                        = DDI_CODEC_GEN_MAX_PROFILES;
    ctx->max_entrypoints                     = DDI_CODEC_GEN_MAX_ENTRYPOINTS;
    ctx->max_attributes                      = (int32_t)VAConfigAttribTypeMax;
    ctx->max_subpic_formats                  = DDI_CODEC_GEN_MAX_SUBPIC_FORMATS;
    ctx->max_display_attributes              = DDI_MEDIA_GEN_MAX_DISPLAY_ATTRIBUTES ;
    ctx->str_vendor                          = DDI_CODEC_GEN_STR_VENDOR;
    ctx->vtable_tpi                          = nullptr;

    pVTable->vaTerminate                     = Terminate;

    pVTable->vaQueryConfigEntrypoints        = QueryConfigEntrypoints;
    pVTable->vaQueryConfigProfiles           = QueryConfigProfiles;
    pVTable->vaQueryConfigAttributes         = QueryConfigAttributes;
    pVTable->vaCreateConfig                  = CreateConfig;
    pVTable->vaDestroyConfig                 = DestroyConfig;
    pVTable->vaGetConfigAttributes           = GetConfigAttributes;

    pVTable->vaCreateSurfaces                = CreateSurfaces;
    pVTable->vaDestroySurfaces               = DestroySurfaces;
    pVTable->vaCreateSurfaces2               = CreateSurfaces2;

    pVTable->vaCreateContext                 = CreateContext;
    pVTable->vaDestroyContext                = DestroyContext;
    pVTable->vaCreateBuffer                  = CreateBuffer;
    pVTable->vaBufferSetNumElements          = BufferSetNumElements;
    pVTable->vaMapBuffer                     = MapBuffer;
    pVTable->vaUnmapBuffer                   = UnmapBuffer;
    pVTable->vaDestroyBuffer                 = DestroyBuffer;
    pVTable->vaBeginPicture                  = BeginPicture;
    pVTable->vaRenderPicture                 = RenderPicture;
    pVTable->vaEndPicture                    = EndPicture;
    pVTable->vaSyncSurface                   = SyncSurface;
#if VA_CHECK_VERSION(1, 9, 0)
    pVTable->vaSyncSurface2                  = SyncSurface2;
    pVTable->vaSyncBuffer                    = SyncBuffer;
#endif
    pVTable->vaQuerySurfaceStatus            = QuerySurfaceStatus;
    pVTable->vaQuerySurfaceError             = QuerySurfaceError;
    pVTable->vaQuerySurfaceAttributes        = QuerySurfaceAttributes;
    pVTable->vaPutSurface                    = PutSurface;
    pVTable->vaQueryImageFormats             = QueryImageFormats;

    pVTable->vaCreateImage                   = CreateImage;
    pVTable->vaDeriveImage                   = DeriveImage;
    pVTable->vaDestroyImage                  = DestroyImage;
    pVTable->vaSetImagePalette               = SetImagePalette;
    pVTable->vaGetImage                      = GetImage;
    pVTable->vaPutImage                      = PutImage;
    pVTable->vaQuerySubpictureFormats        = QuerySubpictureFormats;
    pVTable->vaCreateSubpicture              = CreateSubpicture;
    pVTable->vaDestroySubpicture             = DestroySubpicture;
    pVTable->vaSetSubpictureImage            = SetSubpictureImage;
    pVTable->vaSetSubpictureChromakey        = SetSubpictureChromakey;
    pVTable->vaSetSubpictureGlobalAlpha      = SetSubpictureGlobalAlpha;
    pVTable->vaAssociateSubpicture           = AssociateSubpicture;
    pVTable->vaDeassociateSubpicture         = DeassociateSubpicture;
    pVTable->vaQueryDisplayAttributes        = QueryDisplayAttributes;
    pVTable->vaGetDisplayAttributes          = GetDisplayAttributes;
    pVTable->vaSetDisplayAttributes          = SetDisplayAttributes;
    pVTable->vaQueryProcessingRate           = QueryProcessingRate;
#if VA_CHECK_VERSION(1,10,0)
    pVTable->vaCopy                          = Copy;
#endif

    // vaTrace
    pVTable->vaBufferInfo                    = BufferInfo;
    pVTable->vaLockSurface                   = LockSurface;
    pVTable->vaUnlockSurface                 = UnlockSurface;

    pVTableVpp->vaQueryVideoProcFilters      = QueryVideoProcFilters;
    pVTableVpp->vaQueryVideoProcFilterCaps   = QueryVideoProcFilterCaps;
    pVTableVpp->vaQueryVideoProcPipelineCaps = QueryVideoProcPipelineCaps;

#if VA_CHECK_VERSION(1,11,0)
    pVTableProt->vaCreateProtectedSession    = CreateProtectedSession;
    pVTableProt->vaDestroyProtectedSession   = DestroyProtectedSession;
    pVTableProt->vaAttachProtectedSession    = AttachProtectedSession;
    pVTableProt->vaDetachProtectedSession    = DetachProtectedSession;
    pVTableProt->vaProtectedSessionExecute   = ProtectedSessionExecute;
#endif

    pVTable->vaGetSurfaceAttributes          = GetSurfaceAttributes;
    //Export PRIMEFD/FLINK to application for buffer sharing with OpenCL/GL
    pVTable->vaAcquireBufferHandle           = AcquireBufferHandle;
    pVTable->vaReleaseBufferHandle           = ReleaseBufferHandle;
    pVTable->vaExportSurfaceHandle           = ExportSurfaceHandle;

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::CreateContext (
    VADriverContextP  ctx,
    VAConfigID        configId,
    int32_t           pictureWidth,
    int32_t           pictureHeight,
    int32_t           flag,
    VASurfaceID       *renderTarget,
    int32_t           renderTargetsNum,
    VAContextID       *context)
{
    VAStatus     vaStatus    = VA_STATUS_SUCCESS;

    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx,     "nullptr ctx",      VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(context, "nullptr context",  VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaDrvCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaDrvCtx, "nullptr mediaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaDrvCtx->m_capsNext, "nullptr m_capsNext",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mediaDrvCtx->m_capsNext->m_capsTable, "nullptr m_capsTable",   VA_STATUS_ERROR_INVALID_PARAMETER);

    if(!IS_VALID_CONFIG_ID(configId))
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }
    if(renderTargetsNum > 0)
    {
        DDI_CHK_NULL(renderTarget,            "nullptr renderTarget",            VA_STATUS_ERROR_INVALID_PARAMETER);
        DDI_CHK_NULL(mediaDrvCtx->pSurfaceHeap, "nullptr mediaDrvCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
        for(int32_t i = 0; i < renderTargetsNum; i++)
        {
            uint32_t surfaceId = (uint32_t)renderTarget[i];
            DDI_CHK_LESS(surfaceId, mediaDrvCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid Surface", VA_STATUS_ERROR_INVALID_SURFACE);
        }
    }

    if(mediaDrvCtx->m_capsNext->m_capsTable->IsDecConfigId(configId) && REMOVE_CONFIG_ID_DEC_OFFSET(configId) < mediaDrvCtx->m_capsNext->m_capsTable->m_configList.size())
    {
        DDI_CHK_NULL(mediaDrvCtx->m_compList[CompDecode],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);
        vaStatus = mediaDrvCtx->m_compList[CompDecode]->CreateContext(
            ctx, configId, pictureWidth, pictureHeight, flag, renderTarget, renderTargetsNum, context);
    }
    else if(mediaDrvCtx->m_capsNext->m_capsTable->IsEncConfigId(configId) && REMOVE_CONFIG_ID_ENC_OFFSET(configId) < mediaDrvCtx->m_capsNext->m_capsTable->m_configList.size())
    {
        DDI_CHK_NULL(mediaDrvCtx->m_compList[CompEncode],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);
        vaStatus = mediaDrvCtx->m_compList[CompEncode]->CreateContext(
            ctx, configId, pictureWidth, pictureHeight, flag, renderTarget, renderTargetsNum, context);
    }
    else if(mediaDrvCtx->m_capsNext->m_capsTable->IsVpConfigId(configId) && mediaDrvCtx->m_capsNext->m_capsTable->m_configList.size())
    {
        DDI_CHK_NULL(mediaDrvCtx->m_compList[CompVp],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);
        vaStatus = mediaDrvCtx->m_compList[CompVp]->CreateContext(
            ctx, configId, pictureWidth, pictureHeight, flag, renderTarget, renderTargetsNum, context);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: Invalid configID");
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }
    if(vaStatus != VA_STATUS_SUCCESS)
    {
        return vaStatus;
    }
    if(*context < DDI_MEDIA_VACONTEXTID_BASE)
    {
        DDI_ASSERTMESSAGE("DDI: Invalid contextID");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::DestroyContext (
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,         "nullptr ctx",        VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaDrvCtx = GetMediaContext(ctx);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void *ctxPtr = MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CHK_NULL(mediaDrvCtx, "nullptr mediaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaDrvCtx->m_compList[componentIndex], "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(componentIndex != CompCodec && componentIndex != CompEncode &&
        componentIndex != CompDecode && componentIndex != CompVp &&
        componentIndex != CompCp)
    {
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }
    return mediaDrvCtx->m_compList[componentIndex]->DestroyContext(ctx, context);
}

VAStatus MediaLibvaInterfaceNext::CreateBuffer (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferType      type,
    uint32_t          size,
    uint32_t          elementsNum,
    void              *data,
    VABufferID        *bufId)
{
    DDI_FUNC_ENTER;
    int32_t event[] = {size, elementsNum, type};
    MOS_TraceEventExt(EVENT_VA_BUFFER, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL(ctx,       "nullptr ctx",      VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(bufId,     "nullptr bufId",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(size, 0, "Invalid size",     VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,  "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CHK_NULL(ctxPtr,    "nullptr ctxPtr",   VA_STATUS_ERROR_INVALID_CONTEXT);

    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex], "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);
    *bufId = VA_INVALID_ID;

    MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
    VAStatus vaStatus = mediaCtx->m_compList[componentIndex]->CreateBuffer(ctx, context, type, size, elementsNum, data, bufId);
    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);

    MOS_TraceEventExt(EVENT_VA_BUFFER, EVENT_TYPE_END, bufId, sizeof(bufId), nullptr, 0);
    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::DestroyBuffer (
    VADriverContextP    ctx,
    VABufferID          bufId)
{
    PDDI_MEDIA_CONTEXT mediaCtx = nullptr;
    DDI_MEDIA_BUFFER   *buf     = nullptr;
    uint32_t           ctxType  = DDI_MEDIA_CONTEXT_TYPE_NONE;

    DDI_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_START, &bufId, sizeof(bufId), nullptr, 0);

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,              "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr  mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)bufId, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufId", VA_STATUS_ERROR_INVALID_BUFFER);

    buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx,  bufId);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, bufId);

    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex], "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = mediaCtx->m_compList[componentIndex]->DestroyBuffer(mediaCtx, bufId);

    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::BeginPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       renderTarget)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,                    "nullptr ctx",                    VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx   = GetMediaContext(ctx);

    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)renderTarget, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "renderTarget", VA_STATUS_ERROR_INVALID_SURFACE);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    uint32_t event[] = {(uint32_t)context, ctxType, (uint32_t)renderTarget};
    MOS_TraceEventExt(EVENT_VA_PICTURE, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    PDDI_MEDIA_SURFACE surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, renderTarget);
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);

    MosUtilities::MosLockMutex(&mediaCtx->SurfaceMutex);
    surface->curCtxType = ctxType;
    surface->curStatusReportQueryState = DDI_MEDIA_STATUS_REPORT_QUERY_STATE_PENDING;
    if(ctxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        surface->curStatusReport.vpp.status = VPREP_NOTAVAILABLE;
    }
    MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);

    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex],  "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[componentIndex]->BeginPicture(ctx, context, renderTarget);
}

VAStatus MediaLibvaInterfaceNext::RenderPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           buffersNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(  ctx,            "nullptr ctx",             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(  buffers,        "nullptr buffers",         VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(buffersNum, 0,  "Invalid number buffers",  VA_STATUS_ERROR_INVALID_PARAMETER);
    uint32_t event[] = {(uint32_t)context, (uint32_t)buffersNum};
    MOS_TraceEventExt(EVENT_VA_PICTURE, EVENT_TYPE_INFO, event, sizeof(event), buffers, buffersNum*sizeof(VAGenericID));

    PDDI_MEDIA_CONTEXT mediaCtx   = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,              "nullptr mediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    for(int32_t i = 0; i < buffersNum; i++)
    {
       DDI_CHK_LESS((uint32_t)buffers[i], mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid Buffer", VA_STATUS_ERROR_INVALID_BUFFER);
    }

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex],  "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[componentIndex]->RenderPicture(ctx, context, buffers, buffersNum);
}

VAStatus MediaLibvaInterfaceNext::EndPicture(
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,                                   "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                              "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = mediaCtx->m_compList[componentIndex]->EndPicture(ctx, context);

    MOS_TraceEventExt(EVENT_VA_PICTURE, EVENT_TYPE_END, &context, sizeof(context), &vaStatus, sizeof(vaStatus));
    PERF_UTILITY_STOP_ONCE("First Frame Time", PERF_MOS, PERF_LEVEL_DDI);

    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::SyncSurface(
    VADriverContextP    ctx,
    VASurfaceID         renderTarget)
{
    DDI_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_START, &renderTarget, sizeof(VAGenericID), nullptr, 0);

    DDI_CHK_NULL(ctx,    "nullptr ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",                VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)renderTarget, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid renderTarget", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE  *surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, renderTarget);
    DDI_CHK_NULL(surface,    "nullptr surface",      VA_STATUS_ERROR_INVALID_CONTEXT);
    if (surface->pCurrentFrameSemaphore)
    {
        MediaLibvaUtilNext::WaitSemaphore(surface->pCurrentFrameSemaphore);
        MediaLibvaUtilNext::PostSemaphore(surface->pCurrentFrameSemaphore);
    }

    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_INFO, surface->bo? &surface->bo->handle:nullptr, sizeof(uint32_t), nullptr, 0);
    // check the bo here?
    // zero is a expected return value
    uint32_t timeout_NS = 100000000;
    while (0 != mos_bo_wait(surface->bo, timeout_NS))
    {
        // Just loop while gem_bo_wait times-out.
    }

    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_END, nullptr, 0, nullptr, 0);

    CompType componentIndex = CompCommon;
    PDDI_DECODE_CONTEXT decCtx = (PDDI_DECODE_CONTEXT)surface->pDecCtx;
    if (decCtx && surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_DECODER)
    {
        componentIndex = CompDecode;
    }
    else if (surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        componentIndex = CompVp;
    }

    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex],  "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);
    return mediaCtx->m_compList[componentIndex]->StatusCheck(mediaCtx, surface, renderTarget);
}

VAStatus MediaLibvaInterfaceNext::QuerySurfaceError(
    VADriverContextP ctx,
    VASurfaceID      renderTarget,
    VAStatus         errorStatus,
    void             **errorInfo)
{
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::PutSurface(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void             *draw,
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle      *cliprects,      /* client supplied clip list */
    uint32_t         numberCliprects, /* number of clip rects in the clip list */
    uint32_t         flags)           /* de-interlacing flags */
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    if(numberCliprects > 0)
    {
        DDI_CHK_NULL(cliprects, "nullptr cliprects", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    PDDI_MEDIA_CONTEXT mediaDrvCtx   = GetMediaContext(ctx);

    DDI_CHK_NULL(mediaDrvCtx,                      "nullptr mediaDrvCtx",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaDrvCtx->m_compList[CompVp],  "nullptr complist",      VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaDrvCtx->m_compList[CompVp]->PutSurface(ctx, surface, draw, srcx, srcy, srcw, srch, destx, desty, destw, desth, cliprects, numberCliprects, flags);
}

VAImage* MediaLibvaInterfaceNext::GetVAImageFromVAImageID(PDDI_MEDIA_CONTEXT mediaCtx, VAImageID imageID)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", nullptr);

    uint32_t i       = (uint32_t)imageID;
    DDI_CHK_LESS(i, mediaCtx->pImageHeap->uiAllocatedHeapElements, "invalid image id", nullptr);
    MosUtilities::MosLockMutex(&mediaCtx->ImageMutex);

    PDDI_MEDIA_IMAGE_HEAP_ELEMENT imageElement = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)mediaCtx->pImageHeap->pHeapBase;
    imageElement    += i;
    VAImage *vaImage = imageElement->pImage;

    MosUtilities::MosUnlockMutex(&mediaCtx->ImageMutex);

    return vaImage;
}

bool MediaLibvaInterfaceNext::DestroyImageFromVAImageID(PDDI_MEDIA_CONTEXT mediaCtx, VAImageID imageID)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", false);

    MosUtilities::MosLockMutex(&mediaCtx->ImageMutex);

    MediaLibvaUtilNext::ReleasePVAImageFromHeap(mediaCtx->pImageHeap, (uint32_t)imageID);
    mediaCtx->uiNumImages--;

    MosUtilities::MosUnlockMutex(&mediaCtx->ImageMutex);

    return true;
}

VAStatus MediaLibvaInterfaceNext::DestroyImage(
    VADriverContextP ctx,
    VAImageID        image)
{
    DDI_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_FREE_IMAGE, EVENT_TYPE_START, &image, sizeof(image), nullptr, 0);

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);

    DDI_CHK_NULL(mediaCtx,             "nullptr Media",                       VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pImageHeap, "nullptr mediaCtx->pImageHeap",        VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)image, mediaCtx->pImageHeap->uiAllocatedHeapElements, "Invalid image", VA_STATUS_ERROR_INVALID_IMAGE);

    VAImage *vaImage = GetVAImageFromVAImageID(mediaCtx, image);
    if (vaImage == nullptr)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    DDI_CHK_NULL(mediaCtx->m_compList[CompCommon],  "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);
    mediaCtx->m_compList[CompCommon]->DestroyBuffer(mediaCtx, vaImage->buf);
    MOS_FreeMemory(vaImage);

    DestroyImageFromVAImageID(mediaCtx, image);

    MOS_TraceEventExt(EVENT_VA_FREE_IMAGE, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

void MediaLibvaInterfaceNext::CopyPlane(
    uint8_t  *dst,
    uint32_t dstPitch,
    uint8_t  *src,
    uint32_t srcPitch,
    uint32_t height)
{
    uint32_t rowSize = std::min(dstPitch, srcPitch);
    for (int y = 0; y < height; y += 1)
    {
        MOS_SecureMemcpy(dst, rowSize, src, rowSize);
        dst += dstPitch;
        src += srcPitch;
    }
}

VAStatus MediaLibvaInterfaceNext::CopySurfaceToImage(
    VADriverContextP  ctx,
    DDI_MEDIA_SURFACE *surface,
    VAImage           *image)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,       "nullptr ctx.",         VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,  "nullptr mediaCtx.",    VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(surface,  "nullptr meida surface.", VA_STATUS_ERROR_INVALID_BUFFER);
    uint32_t flag = MOS_LOCKFLAG_READONLY;
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    //Lock Surface
    if ((Media_Format_CPU != surface->format))
    {
        vaStatus = MediaMemoryDecompress(mediaCtx, surface);

        if (vaStatus != VA_STATUS_SUCCESS)
        {
            DDI_NORMALMESSAGE("surface Decompression fail, continue next steps.");
        }
    }

    void *surfData = MediaLibvaUtilNext::LockSurface(surface, flag);
    if (surfData == nullptr)
    {
        DDI_ASSERTMESSAGE("nullptr surfData.");
        return vaStatus;
    }

    void *imageData = nullptr;
    vaStatus = MapBuffer(ctx, image->buf, &imageData);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Failed to map buffer.");
        MediaLibvaUtilNext::UnlockSurface(surface);
        return vaStatus;
    }

    uint8_t *ySrc = (uint8_t*)surfData;
    uint8_t *yDst = (uint8_t*)imageData;

    CopyPlane(yDst, image->pitches[0], ySrc, surface->iPitch, image->height);
    if (image->num_planes > 1)
    {
        uint8_t *uSrc = ySrc + surface->iPitch * surface->iHeight;
        uint8_t *uDst = yDst + image->offsets[1];
        uint32_t chromaPitch       = 0;
        uint32_t chromaHeight      = 0;
        uint32_t imageChromaPitch  = 0;
        uint32_t imageChromaHeight = 0;
        GetChromaPitchHeight(MediaFormatToOsFormat(surface->format), surface->iPitch, surface->iHeight, &chromaPitch, &chromaHeight);
        GetChromaPitchHeight(image->format.fourcc, image->pitches[0], image->height, &imageChromaPitch, &imageChromaHeight);
        CopyPlane(uDst, image->pitches[1], uSrc, chromaPitch, imageChromaHeight);

        if(image->num_planes > 2)
        {
            uint8_t *vSrc = uSrc + chromaPitch * chromaHeight;
            uint8_t *vDst = yDst + image->offsets[2];
            CopyPlane(vDst, image->pitches[2], vSrc, chromaPitch, imageChromaHeight);
        }
    }

    vaStatus = UnmapBuffer(ctx, image->buf);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Failed to unmap buffer.");
        MediaLibvaUtilNext::UnlockSurface(surface);
        return vaStatus;
    }

    MediaLibvaUtilNext::UnlockSurface(surface);

    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::GetImage(
    VADriverContextP ctx,
    VASurfaceID      surface,
    int32_t          x,     /* coordinates of the upper left source pixel */
    int32_t          y,
    uint32_t         width, /* width and height of the region */
    uint32_t         height,
    VAImageID        image
)
{
    DDI_FUNC_ENTER;

    uint32_t event[] = {surface, x, y, width, height, image};
    MOS_TraceEventExt(EVENT_VA_GET, EVENT_TYPE_START, &event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL(ctx,       "nullptr ctx.",         VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,  "nullptr mediaCtx.",    VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pSurfaceHeap,    "nullptr mediaCtx->pSurfaceHeap.",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pImageHeap,      "nullptr mediaCtx->pImageHeap.",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface.", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_LESS((uint32_t)image,   mediaCtx->pImageHeap->uiAllocatedHeapElements,   "Invalid image.",   VA_STATUS_ERROR_INVALID_IMAGE);

    VAImage *vaimg = GetVAImageFromVAImageID(mediaCtx, image);
    DDI_CHK_NULL(vaimg,     "nullptr vaimg.",       VA_STATUS_ERROR_INVALID_IMAGE);

    DDI_MEDIA_BUFFER *buf =  MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, vaimg->buf);
    DDI_CHK_NULL(buf,       "nullptr buf.",         VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_MEDIA_SURFACE *inputSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(inputSurface,     "nullptr inputSurface.",      VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(inputSurface->bo, "nullptr inputSurface->bo.",  VA_STATUS_ERROR_INVALID_SURFACE);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
#ifndef _FULL_OPEN_SOURCE
    VASurfaceID targetSurface = VA_INVALID_SURFACE;
    VASurfaceID outputSurface = surface;

    if (inputSurface->format != OsFormatToMediaFormat(vaimg->format.fourcc, vaimg->format.alpha_mask) ||
        (width != vaimg->width || height != vaimg->height) &&
        (vaimg->format.fourcc != VA_FOURCC_444P &&
        vaimg->format.fourcc != VA_FOURCC_422V &&
        vaimg->format.fourcc != VA_FOURCC_422H))
    {
        VAContextID context = VA_INVALID_ID;
        //Create VP Context.
        DDI_CHK_NULL(mediaCtx->m_compList[CompVp],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);
        vaStatus = mediaCtx->m_compList[CompVp]->CreateContext(ctx, 0, 0, 0, 0, 0, 0, &context);
        DDI_CHK_RET(vaStatus, "Create VP Context failed.");

        //Create target surface for VP pipeline.
        DDI_MEDIA_FORMAT mediaFmt = OsFormatToMediaFormat(vaimg->format.fourcc, vaimg->format.fourcc);
        if (mediaFmt == Media_Format_Count)
        {
            DDI_ASSERTMESSAGE("Unsupported surface type.");
            mediaCtx->m_compList[CompVp]->DestroyContext(ctx, context);
            return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
        }

        PDDI_MEDIA_SURFACE_DESCRIPTOR surfDesc = (PDDI_MEDIA_SURFACE_DESCRIPTOR)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE_DESCRIPTOR));
        if (!surfDesc)
        {
            DDI_ASSERTMESSAGE("nullptr surfDesc.");
            mediaCtx->m_compList[CompVp]->DestroyContext(ctx, context);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        surfDesc->uiVaMemType = VA_SURFACE_ATTRIB_MEM_TYPE_VA;
        int memType = MOS_MEMPOOL_VIDEOMEMORY;
        if (MEDIA_IS_SKU(&mediaCtx->SkuTable, FtrLocalMemory))
        {
            memType = MOS_MEMPOOL_SYSTEMMEMORY;
        }
        targetSurface = (VASurfaceID)CreateRenderTarget(mediaCtx, mediaFmt, vaimg->width, vaimg->height, surfDesc, VA_SURFACE_ATTRIB_USAGE_HINT_GENERIC, memType);
        if (VA_INVALID_SURFACE == targetSurface)
        {
            DDI_ASSERTMESSAGE("Create temp surface failed.");
            mediaCtx->m_compList[CompVp]->DestroyContext(ctx, context);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        VARectangle srcRect, dstRect;
        srcRect.x      = x;
        srcRect.y      = y;
        srcRect.width  = width;
        srcRect.height = height;
        dstRect.x      = 0;
        dstRect.y      = 0;
        dstRect.width  = vaimg->width;
        dstRect.height = vaimg->height;

        //Execute VP pipeline.
        vaStatus = mediaCtx->m_compList[CompVp]->ProcessPipeline(ctx, context, surface, &srcRect, targetSurface, &dstRect);
        if (vaStatus != VA_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("VP Pipeline failed.");
            DestroySurfaces(ctx, &targetSurface, 1);
            mediaCtx->m_compList[CompVp]->DestroyContext(ctx, context);
            return vaStatus;
        }
        vaStatus = SyncSurface(ctx, targetSurface);
        DDI_CHK_RET(vaStatus, "Sync surface failed.");

        vaStatus = mediaCtx->m_compList[CompVp]->DestroyContext(ctx, context);
        DDI_CHK_RET(vaStatus, "destroy context failed.");

        outputSurface = targetSurface;
    }

    //Get Media Surface from output surface ID
    DDI_MEDIA_SURFACE *mediaSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, outputSurface);
    DDI_CHK_NULL(mediaSurface,     "nullptr mediaSurface.",      VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->bo, "nullptr mediaSurface->bo.",  VA_STATUS_ERROR_INVALID_SURFACE);

    vaStatus = CopySurfaceToImage(ctx, mediaSurface, vaimg);
    if (vaStatus != MOS_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Failed to copy surface to image buffer data!");
        if(targetSurface != VA_INVALID_SURFACE)
        {
            DestroySurfaces(ctx, &targetSurface, 1);
        }
        return vaStatus;
    }

    //Destroy temp surface if created
    if(targetSurface != VA_INVALID_SURFACE)
    {
        DestroySurfaces(ctx, &targetSurface, 1);
    }
#else
    vaStatus = CopySurfaceToImage(ctx, inputSurface, vaimg);
    DDI_CHK_RET(vaStatus, "Copy surface to image failed.");
#endif

    MOS_TraceEventExt(EVENT_VA_GET, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::PutImage(
    VADriverContextP ctx,
    VASurfaceID      surface,
    VAImageID        image,
    int32_t          srcX,
    int32_t          srcY,
    uint32_t         srcWidth,
    uint32_t         srcHeight,
    int32_t          destX,
    int32_t          destY,
    uint32_t         destWidth,
    uint32_t         destHeight
)
{
    DDI_FUNC_ENTER;
    uint32_t event[] = {surface, image, srcX, srcY, srcWidth, srcHeight, destX, destY, destWidth, destHeight};
    MOS_TraceEventExt(EVENT_VA_PUT, EVENT_TYPE_START, &event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL(ctx,                    "nullptr ctx.",                     VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx     = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx.",                VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap.",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pImageHeap,   "nullptr mediaCtx->pImageHeap.",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface.", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_LESS((uint32_t)image, mediaCtx->pImageHeap->uiAllocatedHeapElements,     "Invalid image.",   VA_STATUS_ERROR_INVALID_IMAGE);

    DDI_MEDIA_SURFACE *mediaSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(mediaSurface,     "nullptr mediaSurface.", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->bo, "Invalid buffer.",       VA_STATUS_ERROR_INVALID_BUFFER);

    if (mediaSurface->pCurrentFrameSemaphore)
    {
        MediaLibvaUtilNext::WaitSemaphore(mediaSurface->pCurrentFrameSemaphore);
        MediaLibvaUtilNext::PostSemaphore(mediaSurface->pCurrentFrameSemaphore);
    }

    VAImage          *vaimg = GetVAImageFromVAImageID(mediaCtx, image);
    DDI_CHK_NULL(vaimg,      "Invalid image.",      VA_STATUS_ERROR_INVALID_IMAGE);

    DDI_MEDIA_BUFFER *buf   = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, vaimg->buf);
    DDI_CHK_NULL(buf,       "Invalid buffer.",      VA_STATUS_ERROR_INVALID_BUFFER);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    void *imageData   = nullptr;

    vaStatus = MapBuffer(ctx, vaimg->buf, &imageData);
    DDI_CHK_RET(vaStatus,   "MapBuffer failed.");
    DDI_CHK_NULL(imageData, "nullptr imageData.", VA_STATUS_ERROR_INVALID_IMAGE);

    // VP Pipeline will be called for CSC/Scaling if the surface format or data size is not consistent with image.
    if (mediaSurface->format != OsFormatToMediaFormat(vaimg->format.fourcc, vaimg->format.alpha_mask) ||
        destWidth != srcWidth || destHeight != srcHeight ||
        srcX != 0 || destX != 0 || srcY != 0 || destY != 0)
    {
        VAContextID context     = VA_INVALID_ID;

        //Create VP Context.
        DDI_CHK_NULL(mediaCtx->m_compList[CompVp],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);
        vaStatus = mediaCtx->m_compList[CompVp]->CreateContext(ctx, 0, 0, 0, 0, 0, 0, &context);
        DDI_CHK_RET(vaStatus, "Create VP Context failed");

        //Create temp surface for VP pipeline.
        DDI_MEDIA_FORMAT mediaFmt = OsFormatToMediaFormat(vaimg->format.fourcc, vaimg->format.fourcc);
        if (mediaFmt == Media_Format_Count)
        {
            DDI_ASSERTMESSAGE("Unsupported surface type.");
            return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
        }

        int memType = MOS_MEMPOOL_VIDEOMEMORY;
        if (MEDIA_IS_SKU(&mediaCtx->SkuTable, FtrLocalMemory))
        {
            memType = MOS_MEMPOOL_SYSTEMMEMORY;
        }
        VASurfaceID tempSurface = (VASurfaceID)CreateRenderTarget(mediaCtx, mediaFmt, vaimg->width, vaimg->height, nullptr, VA_SURFACE_ATTRIB_USAGE_HINT_VPP_READ, memType);
        if (tempSurface == VA_INVALID_ID)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        DDI_MEDIA_SURFACE *tempMediaSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, tempSurface);
        DDI_CHK_NULL(tempMediaSurface, "nullptr tempMediaSurface.", VA_STATUS_ERROR_INVALID_SURFACE);

        //Lock Surface
        void *tempSurfData = MediaLibvaUtilNext::LockSurface(tempMediaSurface, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));
        if (nullptr == tempSurfData)
        {
            DestroySurfaces(ctx, &tempSurface, 1);
            return VA_STATUS_ERROR_SURFACE_BUSY;
        }

        //Copy data from image to temp surferce
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        if (tempMediaSurface->data_size >= vaimg->data_size)
        {
            eStatus = MOS_SecureMemcpy(tempSurfData, tempMediaSurface->data_size, imageData, vaimg->data_size);
        }
        else
        {
            eStatus = MOS_SecureMemcpy(tempSurfData, tempMediaSurface->data_size, imageData, tempMediaSurface->data_size);
        }

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("Failed to copy image to surface buffer.");
            MediaLibvaUtilNext::UnlockSurface(tempMediaSurface);
            DestroySurfaces(ctx, &tempSurface, 1);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }

        vaStatus = UnmapBuffer(ctx, vaimg->buf);
        if (vaStatus != VA_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("Failed to unmap buffer.");
            MediaLibvaUtilNext::UnlockSurface(tempMediaSurface);
            DestroySurfaces(ctx, &tempSurface, 1);
            return vaStatus;
        }

        MediaLibvaUtilNext::UnlockSurface(tempMediaSurface);

        VARectangle srcRect, dstRect;
        srcRect.x      = srcX;
        srcRect.y      = srcY;
        srcRect.width  = srcWidth;
        srcRect.height = srcHeight;
        dstRect.x      = destX;
        dstRect.y      = destY;
        dstRect.width  = destWidth;
        dstRect.height = destHeight;

        //Execute VP pipeline.
        vaStatus = mediaCtx->m_compList[CompVp]->ProcessPipeline(ctx, context, tempSurface, &srcRect, surface, &dstRect);
        if (vaStatus != VA_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("VP Pipeline failed.");
            DestroySurfaces(ctx, &tempSurface, 1);
            return vaStatus;
        }

        vaStatus = SyncSurface(ctx, tempSurface);
        DDI_CHK_RET(vaStatus, "sync surface failed.");

        vaStatus = DestroySurfaces(ctx, &tempSurface, 1);
        DDI_CHK_RET(vaStatus, "destroy surface failed.");

        vaStatus = mediaCtx->m_compList[CompVp]->DestroyContext(ctx, context);
    }
    else
    {
        //Lock Surface
        if ((nullptr != buf->pSurface) && (Media_Format_CPU != mediaSurface->format))
        {
            vaStatus = MediaMemoryDecompress(mediaCtx, mediaSurface);

            if (vaStatus != VA_STATUS_SUCCESS)
            {
                DDI_NORMALMESSAGE("surface Decompression fail, continue next steps.");
            }
        }

        void *surfData = MediaLibvaUtilNext::LockSurface(mediaSurface, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));
        if (nullptr == surfData)
        {
            DDI_ASSERTMESSAGE("Failed to lock surface.");
            return VA_STATUS_ERROR_SURFACE_BUSY;
        }

        if (srcWidth == destWidth && srcHeight == destHeight &&
            srcWidth == vaimg->width && srcHeight == vaimg->height &&
            srcWidth == mediaSurface->iWidth && srcHeight == mediaSurface->iHeight &&
            mediaSurface->data_size == vaimg->data_size &&
            (vaimg->num_planes == 1 ||
            (vaimg->num_planes > 1 && vaimg->offsets[1] == mediaSurface->iPitch * mediaSurface->iHeight)))
        {
            //Copy data from image to surface
            MOS_STATUS eStatus = MOS_SecureMemcpy(surfData, vaimg->data_size, imageData, vaimg->data_size);
            DDI_CHK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "Failed to copy image to surface buffer.", VA_STATUS_ERROR_OPERATION_FAILED);
        }
        else
        {
            uint8_t *ySrc = (uint8_t *)imageData + vaimg->offsets[0];
            uint8_t *yDst = (uint8_t *)surfData;
            CopyPlane(yDst, mediaSurface->iPitch, ySrc, vaimg->pitches[0], srcHeight);

            if (vaimg->num_planes > 1)
            {
                DDI_MEDIA_SURFACE uPlane = *mediaSurface;

                uint32_t chromaHeight      = 0;
                uint32_t chromaPitch       = 0;
                GetChromaPitchHeight(MediaFormatToOsFormat(uPlane.format), uPlane.iPitch, uPlane.iHeight, &chromaPitch, &chromaHeight);

                uint8_t *uSrc = (uint8_t *)imageData + vaimg->offsets[1];
                uint8_t *uDst = yDst + mediaSurface->iPitch * mediaSurface->iHeight;
                CopyPlane(uDst, chromaPitch, uSrc, vaimg->pitches[1], chromaHeight);
                if (vaimg->num_planes > 2)
                {
                    uint8_t *vSrc = (uint8_t *)imageData + vaimg->offsets[2];
                    uint8_t *vDst = uDst + chromaPitch * chromaHeight;
                    CopyPlane(vDst, chromaPitch, vSrc, vaimg->pitches[2], chromaHeight);
                }
            }
        } 

        vaStatus = UnmapBuffer(ctx, vaimg->buf);
        if (vaStatus != VA_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("Failed to unmap buffer.");
            MediaLibvaUtilNext::UnlockSurface(mediaSurface);
            return vaStatus;
        }

        MediaLibvaUtilNext::UnlockSurface(mediaSurface);
    }
    MOS_TraceEventExt(EVENT_VA_PUT, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::LockSurface(
    VADriverContextP ctx,
    VASurfaceID      surface,
    uint32_t        *fourcc,
    uint32_t        *lumaStride,
    uint32_t        *chromaUStride,
    uint32_t        *chromaVStride,
    uint32_t        *lumaOffset,
    uint32_t        *chromaUOffset,
    uint32_t        *chromaVOffset,
    uint32_t        *bufferName,
    void           **buffer)
{
    DDI_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_LOCK, EVENT_TYPE_START, &surface, sizeof(surface), nullptr, 0);

    DDI_CHK_NULL(ctx,           "nullptr context",       VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(fourcc,        "nullptr fourcc",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(lumaStride,    "nullptr lumaStride",    VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chromaUStride, "nullptr chromaUStride", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chromaVStride, "nullptr chromaVStride", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(lumaOffset,    "nullptr lumaOffset",    VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chromaUOffset, "nullptr chromaUOffset", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chromaVOffset, "nullptr chromaVOffset", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(bufferName,    "nullptr bufferName",    VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(buffer,        "nullptr buffer",        VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx          = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr Media",                  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE *mediaSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surface);
    
#ifdef _MMC_SUPPORTED
    // Decompress surface is needed
    DDI_CHK_RET(MediaMemoryDecompress(mediaCtx, mediaSurface), "Decompress surface is failed");
#endif

    if (nullptr == mediaSurface)
    {
        // Surface is absent.
        buffer = nullptr;
        return VA_STATUS_ERROR_INVALID_SURFACE;
    }

    if (mediaSurface->uiLockedImageID != VA_INVALID_ID)
    {
        // Surface is locked already.
        buffer = nullptr;
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    VAImage tmpImage;
    tmpImage.image_id = VA_INVALID_ID;
    VAStatus vaStatus = DeriveImage(ctx, surface, &tmpImage);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        buffer = nullptr;
        return vaStatus;
    }

    mediaSurface->uiLockedImageID = tmpImage.image_id;

    vaStatus = MapBuffer(ctx,tmpImage.buf, buffer);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        buffer = nullptr;
        return vaStatus;
    }

    mediaSurface->uiLockedBufID = tmpImage.buf;

    *fourcc               = tmpImage.format.fourcc;
    *lumaOffset           = tmpImage.offsets[0];
    *lumaStride           = tmpImage.pitches[0];
    *chromaUOffset        = tmpImage.offsets[1];
    *chromaUStride        = tmpImage.pitches[1];
    *chromaVOffset        = tmpImage.offsets[2];
    *chromaVStride        = tmpImage.pitches[2];
    *bufferName           = tmpImage.buf;

    MOS_TraceEventExt(EVENT_VA_LOCK, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Unlock surface
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] surface
//!         VA surface ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus MediaLibvaInterfaceNext::UnlockSurface(
    VADriverContextP   ctx,
    VASurfaceID        surface)
{
    DDI_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_UNLOCK, EVENT_TYPE_START, &surface, sizeof(surface), nullptr, 0);

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",                 VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE *mediaSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface", VA_STATUS_ERROR_INVALID_SURFACE);

    if (mediaSurface->uiLockedImageID == VA_INVALID_ID)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    VABufferID bufID    = (VABufferID)(mediaSurface->uiLockedBufID);
    VAStatus   vaStatus = UnmapBuffer(ctx, bufID);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        return vaStatus;
    }
    mediaSurface->uiLockedBufID = VA_INVALID_ID;

    VAImageID imageID  = (VAImageID)(mediaSurface->uiLockedImageID);
    vaStatus = DestroyImage(ctx,imageID);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        return vaStatus;
    }
    mediaSurface->uiLockedImageID = VA_INVALID_ID;

    MOS_TraceEventExt(EVENT_VA_UNLOCK, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::QueryConfigEntrypoints(
    VADriverContextP ctx,
    VAProfile        profile,
    VAEntrypoint     *entrypointList,
    int32_t          *entrypointsNum)
{
    DDI_FUNC_ENTER;

    PERF_UTILITY_START_ONCE("First Frame Time", PERF_MOS, PERF_LEVEL_DDI);

    DDI_CHK_NULL(ctx,                  "nullptr Ctx",      VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps",   VA_STATUS_ERROR_INVALID_PARAMETER);

    DDI_CHK_NULL(entrypointList, "nullptr entrypointList", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypointsNum, "nullptr entrypointsNum", VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_capsNext->QueryConfigEntrypoints(profile, entrypointList, entrypointsNum);
}

VAStatus MediaLibvaInterfaceNext::QueryConfigProfiles(
    VADriverContextP  ctx,
    VAProfile         *profileList,
    int32_t           *profilesNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,                  "nullptr Ctx",      VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps",   VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(profileList, "nullptr profileList", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(profilesNum, "nullptr profilesNum", VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_capsNext->QueryConfigProfiles(profileList, profilesNum);
}

VAStatus MediaLibvaInterfaceNext::QueryConfigAttributes(
    VADriverContextP  ctx,
    VAConfigID        configId,
    VAProfile         *profile,
    VAEntrypoint      *entrypoint,
    VAConfigAttrib    *attribList,
    int32_t           *attribsNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(profile,     "nullptr profile",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypoint,  "nullptr entrypoint",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ctx,         "nullptr ctx",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(attribList,  "nullptr attribList",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(attribsNum,  "nullptr attribsNum",  VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps",   VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_capsNext->QueryConfigAttributes(
                configId, profile, entrypoint, attribList, attribsNum);
}

VAStatus MediaLibvaInterfaceNext::CreateConfig(
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attribList,
    int32_t           attribsNum,
    VAConfigID        *configId
)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx,      "nullptr ctx",      VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx   = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    CompType componentIndex = MapCompTypeFromEntrypoint(entrypoint);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[componentIndex]->CreateConfig(
        ctx, profile, entrypoint, attribList, attribsNum, configId);
}

VAStatus MediaLibvaInterfaceNext::DestroyConfig(
    VADriverContextP ctx,
    VAConfigID       configId)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,       "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps",   VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_capsNext->DestroyConfig(configId);
}

VAStatus MediaLibvaInterfaceNext::GetConfigAttributes(
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attribList,
    int32_t           attribsNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,        "nullptr ctx",        VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(attribList, "nullptr attribList", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps",   VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_capsNext->GetConfigAttributes(profile, entrypoint, attribList, attribsNum);
}

VAStatus MediaLibvaInterfaceNext::QuerySurfaceAttributes(
    VADriverContextP  ctx,
    VAConfigID        configId,
    VASurfaceAttrib   *attribList,
    uint32_t          *attribsNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,        "nullptr ctx",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(attribsNum, "nullptr attribsNum",  VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps",   VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_capsNext->QuerySurfaceAttributes(configId, attribList, attribsNum);
}

VAStatus MediaLibvaInterfaceNext::QueryImageFormats(
    VADriverContextP  ctx,
    VAImageFormat     *formatList,
    int32_t           *formatsNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,        "nullptr ctx",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(formatList, "nullptr formatList",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(formatsNum, "nullptr formatsNum",  VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr caps",      VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_capsNext->QueryImageFormats(formatList, formatsNum);
}

VAStatus MediaLibvaInterfaceNext::SetImagePalette(
    VADriverContextP ctx,
    VAImageID        image,
    unsigned char    *palette)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(image);
    DDI_UNUSED(palette);
    DDI_FUNC_ENTER;

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::QuerySubpictureFormats(
    VADriverContextP ctx,
    VAImageFormat    *formatList,
    uint32_t         *flags,
    uint32_t         *formatsNum)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(formatList);
    DDI_UNUSED(flags);
    DDI_UNUSED(formatsNum);

    DDI_FUNC_ENTER;

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::CreateSubpicture(
    VADriverContextP ctx,
    VAImageID        image,
    VASubpictureID   *subpicture)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(image);
    DDI_UNUSED(subpicture);

    DDI_FUNC_ENTER;

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::DestroySubpicture(
    VADriverContextP ctx,
    VASubpictureID   subpicture)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);

    DDI_FUNC_ENTER;

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::SetSubpictureImage(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    VAImageID        image)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);
    DDI_UNUSED(image);

    DDI_FUNC_ENTER;

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::SetSubpictureChromakey(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    uint32_t         chromakeyMin,
    uint32_t         chromakeyMax,
    uint32_t         chromakeyMask)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);
    DDI_UNUSED(chromakeyMin);
    DDI_UNUSED(chromakeyMax);
    DDI_UNUSED(chromakeyMask);

    DDI_FUNC_ENTER;

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::SetSubpictureGlobalAlpha(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    float            globalAlpha)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);
    DDI_UNUSED(globalAlpha);

    DDI_FUNC_ENTER;

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::AssociateSubpicture(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    VASurfaceID      *targetSurfaces,
    int32_t          surfacesNum,
    int16_t          srcX,
    int16_t          srcY,
    uint16_t         srcWidth,
    uint16_t         srcHeight,
    int16_t          destX,
    int16_t          destY,
    uint16_t         destWidth,
    uint16_t         destHeight,
    uint32_t         flags)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);
    DDI_UNUSED(targetSurfaces);
    DDI_UNUSED(surfacesNum);
    DDI_UNUSED(srcX);
    DDI_UNUSED(srcY);
    DDI_UNUSED(srcWidth);
    DDI_UNUSED(srcHeight);
    DDI_UNUSED(destX);
    DDI_UNUSED(destY);
    DDI_UNUSED(destWidth);
    DDI_UNUSED(destHeight);
    DDI_UNUSED(flags);

    DDI_FUNC_ENTER;

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::DeassociateSubpicture(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    VASurfaceID      *targetSurfaces,
    int32_t          surfacesNum)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);
    DDI_UNUSED(targetSurfaces);
    DDI_UNUSED(surfacesNum);

    DDI_FUNC_ENTER;

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::SetDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute  *attrList,
    int32_t             attributesNum)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(attrList);
    DDI_UNUSED(attributesNum);

    DDI_FUNC_ENTER;

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::QueryDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute  *attribList,
    int32_t             *attributesNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,        "nullptr ctx",         VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr caps",      VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_capsNext->QueryDisplayAttributes(attribList, attributesNum);
}

VAStatus MediaLibvaInterfaceNext::GetDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute  *attribList,
    int32_t             attributesNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,        "nullptr ctx",         VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr caps",      VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_capsNext->GetDisplayAttributes(attribList, attributesNum);
}

VAStatus MediaLibvaInterfaceNext::BufferSetNumElements(
    VADriverContextP ctx,
    VABufferID       bufId,
    uint32_t         elementsNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,              "nullptr mediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)bufId, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufId", VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_MEDIA_BUFFER* buf       = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, bufId);
    DDI_CHK_NULL(buf, "Invalid buffer.", VA_STATUS_ERROR_INVALID_BUFFER);

    // only for VASliceParameterBufferType of buffer, the number of elements can be greater than 1
    if(buf->uiType != VASliceParameterBufferType &&
       elementsNum > 1)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if(buf->uiType == VASliceParameterBufferType &&
       buf->uiNumElements < elementsNum)
    {
        MOS_FreeMemory(buf->pData);
        buf->iSize = buf->iSize / buf->uiNumElements;
        buf->pData = (uint8_t*)MOS_AllocAndZeroMemory(buf->iSize * elementsNum);
        buf->iSize = buf->iSize * elementsNum;
    }

    return VA_STATUS_SUCCESS;
}

CompType MediaLibvaInterfaceNext::MapCompTypeFromEntrypoint(VAEntrypoint entrypoint)
{
    DDI_FUNC_ENTER;

    switch(entrypoint)
    {
        case VAEntrypointEncSlice:
        case VAEntrypointEncSliceLP:
        case VAEntrypointEncPicture:
        case VAEntrypointFEI:
        case VAEntrypointStats:
            return CompEncode;
        case VAEntrypointVLD:
            return CompDecode;
        case VAEntrypointVideoProc:
            return CompVp;
        case VAEntrypointProtectedContent:
        case VAEntrypointProtectedTEEComm:
            return CompCp;
        default:
            return CompCommon;
    }
}

VAStatus MediaLibvaInterfaceNext::QueryVideoProcFilters(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  *filters,
    uint32_t          *filtersNum)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx   = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                      "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_compList[CompVp],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[CompVp]->QueryVideoProcFilters(ctx, context, filters, filtersNum);
}

VAStatus MediaLibvaInterfaceNext::QueryVideoProcFilterCaps(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  type,
    void              *filterCaps,
    uint32_t          *filterCapsNum)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx,                          "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                     "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_compList[CompVp], "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[CompVp]->QueryVideoProcFilterCaps(ctx, context, type, filterCaps, filterCapsNum);
}

VAStatus MediaLibvaInterfaceNext::QueryVideoProcPipelineCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID          *filters,
    uint32_t            filtersNum,
    VAProcPipelineCaps  *pipelineCaps)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx,                           "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                      "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_compList[CompVp],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_compList[CompDecode], "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    if(context < DDI_MEDIA_VACONTEXTID_BASE)
    {
        DDI_ASSERTMESSAGE("Invalid ContextID.");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    if ((context & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_SOFTLET_VACONTEXTID_VP_OFFSET)
    {
        return mediaCtx->m_compList[CompVp]->QueryVideoProcPipelineCaps(ctx, context, filters, filtersNum, pipelineCaps);
    }
    else if ((context & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_SOFTLET_VACONTEXTID_DECODER_OFFSET)
    {
        //Decode+SFC, go SFC path, the restriction here is the capability of SFC
        return mediaCtx->m_compList[CompDecode]->QueryVideoProcPipelineCaps(ctx, context, filters, filtersNum, pipelineCaps);
    }
    else
    {
        DDI_ASSERTMESSAGE("Invalid ContextID.");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }
}

VAStatus MediaLibvaInterfaceNext::CreateImage (
    VADriverContextP  ctx,
    VAImageFormat     *format,
    int32_t           width,
    int32_t           height,
    VAImage           *image)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,         "Invalid context!",    VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(format,      "Invalid format!",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(image,       "Invalid image!",      VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(width,  0, "Invalid width!",      VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(height, 0, "Invalid height!",     VA_STATUS_ERROR_INVALID_PARAMETER);
    
    int32_t event[] = {width, height, format->fourcc};
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    
    MOS_TraceEventExt(EVENT_VA_IMAGE, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                    "nullptr mediaCtx.",                    VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mediaCtx->pGmmClientContext, "nullptr mediaCtx->pGmmClientContext.", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAImage *vaimg  = (VAImage*)MOS_AllocAndZeroMemory(sizeof(VAImage));
    DDI_CHK_NULL(vaimg,  "Insufficient to allocate an VAImage.",  VA_STATUS_ERROR_ALLOCATION_FAILED);

    GMM_RESCREATE_PARAMS       gmmParams;
    GMM_RESOURCE_INFO          *gmmResourceInfo;
    MOS_ZeroMemory(&gmmParams, sizeof(gmmParams));

    gmmParams.BaseWidth         = width;
    gmmParams.BaseHeight        = height;
    gmmParams.ArraySize         = 1;
    gmmParams.Type              = RESOURCE_2D;
    gmmParams.Flags.Gpu.Video   = true;
    gmmParams.Format            = MediaLibvaUtilNext::ConvertFourccToGmmFmt(format->fourcc);

    if (gmmParams.Format == GMM_FORMAT_INVALID)
    {
        MOS_FreeMemory(vaimg);
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    gmmResourceInfo = mediaCtx->pGmmClientContext->CreateResInfoObject(&gmmParams);
    if(nullptr == gmmResourceInfo)
    {
        DDI_ASSERTMESSAGE("Gmm Create Resource Failed.");
        MOS_FreeMemory(vaimg);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    vaStatus = GenerateVaImgFromOsFormat(*format, width, height, gmmResourceInfo, vaimg);
    if(vaStatus != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Invaild Input format.");
        MOS_FreeMemory(vaimg);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    mediaCtx->pGmmClientContext->DestroyResInfoObject(gmmResourceInfo);

    DDI_MEDIA_BUFFER *buf  = MOS_New(DDI_MEDIA_BUFFER);
    if (nullptr == buf)
    {
        MOS_FreeMemory(vaimg);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    buf->uiNumElements     = 1;
    buf->iSize             = vaimg->data_size;
    buf->uiType            = VAImageBufferType;
    buf->format            = Media_Format_CPU;//DdiCodec_OsFormatToMediaFormat(vaimg->format.fourcc); //Media_Format_Buffer;
    buf->uiOffset          = 0;
    buf->pMediaCtx         = mediaCtx;

    //Put Image in untiled buffer for better CPU access?
    VAStatus status= MediaLibvaUtilNext::CreateBuffer(buf,  mediaCtx->pDrmBufMgr);
    if((status != VA_STATUS_SUCCESS))
    {
        MOS_FreeMemory(vaimg);
        MOS_Delete(buf);
        return status;
    }
    buf->TileType     = TILING_NONE;

    MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufferHeapElement  = MediaLibvaUtilNext::AllocPMediaBufferFromHeap(mediaCtx->pBufferHeap);

    if (nullptr == bufferHeapElement)
    {
        MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
        MOS_FreeMemory(vaimg);
        MediaLibvaUtilNext::FreeBuffer(buf);
        MOS_Delete(buf);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    bufferHeapElement->pBuffer   = buf;
    bufferHeapElement->pCtx      = nullptr;
    bufferHeapElement->uiCtxType = DDI_MEDIA_CONTEXT_TYPE_MEDIA;

    vaimg->buf                   = bufferHeapElement->uiVaBufferID;
    mediaCtx->uiNumBufs++;
    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);

    MosUtilities::MosLockMutex(&mediaCtx->ImageMutex);
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT imageHeapElement = MediaLibvaUtilNext::AllocPVAImageFromHeap(mediaCtx->pImageHeap);
    if (nullptr == imageHeapElement)
    {
        MosUtilities::MosUnlockMutex(&mediaCtx->ImageMutex);
        MOS_FreeMemory(vaimg);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    imageHeapElement->pImage     = vaimg;
    mediaCtx->uiNumImages++;
    vaimg->image_id              = imageHeapElement->uiVaImageID;
    MosUtilities::MosUnlockMutex(&mediaCtx->ImageMutex);

   *image = *vaimg;
    MOS_TraceEventExt(EVENT_VA_IMAGE, EVENT_TYPE_END, &vaimg->image_id, sizeof(VAGenericID), nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::DeriveImage (
    VADriverContextP  ctx,
    VASurfaceID       surface,
    VAImage           *image
)
{
    DDI_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_DERIVE, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    DDI_CHK_NULL(ctx,   "nullptr ctx",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(image, "nullptr image", VA_STATUS_ERROR_INVALID_PARAMETER);
    
    VAStatus           status       = VA_STATUS_SUCCESS;
    DDI_MEDIA_BUFFER   *buf         = nullptr;
    PDDI_MEDIA_CONTEXT mediaCtx     = GetMediaContext(ctx);
    
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE *mediaSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface", VA_STATUS_ERROR_INVALID_SURFACE);

    VAImage *vaimg                  = (VAImage*)MOS_AllocAndZeroMemory(sizeof(VAImage));
    DDI_CHK_NULL(vaimg, "nullptr vaimg", VA_STATUS_ERROR_ALLOCATION_FAILED);

    if (mediaSurface->pCurrentFrameSemaphore)
    {
        MediaLibvaUtilNext::WaitSemaphore(mediaSurface->pCurrentFrameSemaphore);
        MediaLibvaUtilNext::PostSemaphore(mediaSurface->pCurrentFrameSemaphore);
    }
    MosUtilities::MosLockMutex(&mediaCtx->ImageMutex);
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT imageHeapElement = MediaLibvaUtilNext::AllocPVAImageFromHeap(mediaCtx->pImageHeap);
    if (nullptr == imageHeapElement)
    {
        MosUtilities::MosUnlockMutex(&mediaCtx->ImageMutex);
        MOS_FreeMemory(vaimg);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    imageHeapElement->pImage        = vaimg;
    mediaCtx->uiNumImages++;
    vaimg->image_id                 = imageHeapElement->uiVaImageID;
    MosUtilities::MosUnlockMutex(&mediaCtx->ImageMutex);

    vaimg->format.fourcc            = MediaFormatToOsFormat(mediaSurface->format);
    vaimg->width                    = mediaSurface->iWidth;
    vaimg->height                   = mediaSurface->iRealHeight;
    vaimg->format.byte_order        = VA_LSB_FIRST;
    
    status = GenerateVaImgFromMediaFormat(mediaSurface, mediaCtx, vaimg);
    if (status != VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(vaimg);
        return status;
    }

    if ((mediaSurface->pSurfDesc != nullptr) && (mediaSurface->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR))
    {
        vaimg->num_planes               = mediaSurface->pSurfDesc->uiPlanes;
        for (uint32_t i = 0; i < vaimg->num_planes; i++)
        {
            vaimg->pitches[i]           = mediaSurface->pSurfDesc->uiPitches[i];
            vaimg->offsets[i]           = mediaSurface->pSurfDesc->uiOffsets[i];
        }
    }
    mediaCtx->m_capsNext->PopulateColorMaskInfo(&vaimg->format);

    buf = MOS_New(DDI_MEDIA_BUFFER);
    if (buf == nullptr)
    {
        MOS_FreeMemory(vaimg);
        MOS_Delete(buf);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    buf->uiNumElements = 1;
    buf->iSize         = vaimg->data_size;
    buf->uiType        = VAImageBufferType;
    buf->format        = mediaSurface->format;
    buf->uiOffset      = 0;

    buf->bo            = mediaSurface->bo;
    buf->format        = mediaSurface->format;
    buf->TileType      = mediaSurface->TileType;
    buf->pSurface      = mediaSurface;
    mos_bo_reference(mediaSurface->bo);

    MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufferHeapElement = MediaLibvaUtilNext::AllocPMediaBufferFromHeap(mediaCtx->pBufferHeap);

    if (nullptr == bufferHeapElement)
    {
        MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
        MOS_FreeMemory(vaimg);
        MOS_Delete(buf);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    bufferHeapElement->pBuffer    = buf;
    bufferHeapElement->pCtx       = nullptr;
    bufferHeapElement->uiCtxType  = DDI_MEDIA_CONTEXT_TYPE_MEDIA;

    vaimg->buf             = bufferHeapElement->uiVaBufferID;
    mediaCtx->uiNumBufs++;
    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);

    *image = *vaimg;

    MOS_TraceEventExt(EVENT_VA_DERIVE, EVENT_TYPE_END, &surface, sizeof(surface), &vaimg->image_id, sizeof(VAGenericID));
    return VA_STATUS_SUCCESS;
}

#if VA_CHECK_VERSION(1, 9, 0)

VAStatus MediaLibvaInterfaceNext::SyncSurface2 (
    VADriverContextP    ctx,
    VASurfaceID         surfaceId,
    uint64_t            timeoutNs
)
{
    DDI_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_START, &surfaceId, sizeof(VAGenericID), nullptr, 0);

    DDI_CHK_NULL(ctx,    "nullptr ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)surfaceId, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid renderTarget", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE  *surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surfaceId);
    DDI_CHK_NULL(surface,    "nullptr surface",      VA_STATUS_ERROR_INVALID_CONTEXT);
    if (surface->pCurrentFrameSemaphore)
    {
        MediaLibvaUtilNext::WaitSemaphore(surface->pCurrentFrameSemaphore);
        MediaLibvaUtilNext::PostSemaphore(surface->pCurrentFrameSemaphore);
    }
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_INFO, surface->bo? &surface->bo->handle:nullptr, sizeof(uint32_t), nullptr, 0);

    if (timeoutNs == VA_TIMEOUT_INFINITE)
    {
        // zero is an expected return value when not hit timeout
        auto ret = mos_bo_wait(surface->bo, DDI_BO_INFINITE_TIMEOUT);
        if (0 != ret)
        {
            DDI_NORMALMESSAGE("vaSyncSurface2: surface is still used by HW\n\r");
            return VA_STATUS_ERROR_TIMEDOUT;
        }
    }
    else
    {
        int64_t timeoutBoWait1 = 0;
        int64_t timeoutBoWait2 = 0;
        if (timeoutNs >= DDI_BO_MAX_TIMEOUT)
        {
            timeoutBoWait1 = DDI_BO_MAX_TIMEOUT - 1;
            timeoutBoWait2 = timeoutNs - DDI_BO_MAX_TIMEOUT + 1;
        }
        else
        {
            timeoutBoWait1 = (int64_t)timeoutNs;
        }
        
        // zero is an expected return value when not hit timeout
        auto ret = mos_bo_wait(surface->bo, timeoutBoWait1);
        if (0 != ret)
        {
            if (timeoutBoWait2)
            {
                ret = mos_bo_wait(surface->bo, timeoutBoWait2); 
            }
            if (0 != ret)
            {
                DDI_NORMALMESSAGE("vaSyncSurface2: surface is still used by HW\n\r");
                return VA_STATUS_ERROR_TIMEDOUT;
            }
        }
    }
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_END, nullptr, 0, nullptr, 0);

    CompType componentIndex = CompCommon;
    PDDI_DECODE_CONTEXT decCtx = (PDDI_DECODE_CONTEXT)surface->pDecCtx;
    if (decCtx && surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_DECODER)
    {
        componentIndex = CompDecode;
    }
    else if (surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        componentIndex = CompVp;
    }

    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex],  "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);
    return mediaCtx->m_compList[componentIndex]->StatusCheck(mediaCtx, surface, surfaceId);
}

VAStatus MediaLibvaInterfaceNext::SyncBuffer (
    VADriverContextP    ctx,
    VABufferID          bufId,
    uint64_t            timeoutNs)
{
    PERF_UTILITY_AUTO(__FUNCTION__, "ENCODE", "DDI");

    DDI_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_START, &bufId, sizeof(bufId), nullptr, 0);

    DDI_CHK_NULL(ctx,    "nullptr ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pBufferHeap,  "nullptr mediaCtx->pBufferHeap",  VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)bufId, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid buffer", VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_MEDIA_BUFFER  *buffer = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, bufId);
    DDI_CHK_NULL(buffer,  "nullptr buffer", VA_STATUS_ERROR_INVALID_CONTEXT);

    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_INFO, buffer->bo? &buffer->bo->handle:nullptr, sizeof(uint32_t), nullptr, 0);
    if (timeoutNs == VA_TIMEOUT_INFINITE)
    {
        // zero is a expected return value when not hit timeout
        auto ret = mos_bo_wait(buffer->bo, DDI_BO_INFINITE_TIMEOUT);
        if (0 != ret)
        {
            DDI_NORMALMESSAGE("vaSyncBuffer: buffer is still used by HW\n\r");
            return VA_STATUS_ERROR_TIMEDOUT;
        }
    }
    else
    {
        int64_t timeoutBoWait1 = 0;
        int64_t timeoutBoWait2 = 0;
        if (timeoutNs >= DDI_BO_MAX_TIMEOUT)
        {
            timeoutBoWait1 = DDI_BO_MAX_TIMEOUT - 1;
            timeoutBoWait2 = timeoutNs - DDI_BO_MAX_TIMEOUT + 1;
        }
        else
        {
            timeoutBoWait1 = (int64_t)timeoutNs;
        }

        // zero is a expected return value when not hit timeout
        auto ret = mos_bo_wait(buffer->bo, timeoutBoWait1);
        if (0 != ret)
        {
            if (timeoutBoWait2)
            {
                ret = mos_bo_wait(buffer->bo, timeoutBoWait2);
            }
            if (0 != ret)
            {
                DDI_NORMALMESSAGE("vaSyncBuffer: buffer is still used by HW\n\r");
                return VA_STATUS_ERROR_TIMEDOUT;
            }
        }
    }
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

#endif

VAStatus MediaLibvaInterfaceNext::QuerySurfaceStatus (
    VADriverContextP   ctx,
    VASurfaceID        renderTarget,
    VASurfaceStatus    *status
)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,    "nullptr ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(status, "nullptr status", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                  "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap,    "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)renderTarget, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid renderTarget", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_MEDIA_SURFACE *surface   = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, renderTarget);
    DDI_CHK_NULL(surface,    "nullptr surface",    VA_STATUS_ERROR_INVALID_SURFACE);

    if (surface->pCurrentFrameSemaphore)
    {
        if(MediaLibvaUtilNext::TryWaitSemaphore(surface->pCurrentFrameSemaphore) == 0)
        {
            MediaLibvaUtilNext::PostSemaphore(surface->pCurrentFrameSemaphore);
        }
        else
        {
            // Return busy state if the surface is not submitted
            *status = VASurfaceRendering;
            return VA_STATUS_SUCCESS;
        }
    }

    // Query the busy state of bo.
    // check the bo here?
    if(mos_bo_busy(surface->bo))
    {
        // busy
        *status = VASurfaceRendering;
    }
    else
    {
        // idle
        *status = VASurfaceReady;
    }

    return VA_STATUS_SUCCESS;
}

#if VA_CHECK_VERSION(1,11,0)
VAStatus MediaLibvaInterfaceNext::CreateProtectedSession(
    VADriverContextP      ctx,
    VAConfigID            configId,
    VAProtectedSessionID  *protectedSession)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx,                          "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                     "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_compList[CompCp], "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[CompCp]->CreateProtectedSession(ctx, configId, protectedSession);
}

VAStatus MediaLibvaInterfaceNext::DestroyProtectedSession(
    VADriverContextP      ctx,
    VAProtectedSessionID  protectedSession)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx,                          "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                     "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_compList[CompCp], "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[CompCp]->DestroyProtectedSession(ctx, protectedSession);
}

VAStatus MediaLibvaInterfaceNext::AttachProtectedSession(
    VADriverContextP      ctx,
    VAContextID           context,
    VAProtectedSessionID  protectedSession)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx,                          "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                     "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_compList[CompCp], "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[CompCp]->AttachProtectedSession(ctx, context, protectedSession);
}

VAStatus MediaLibvaInterfaceNext::DetachProtectedSession(
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx,                          "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                     "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_compList[CompCp], "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[CompCp]->DetachProtectedSession(ctx, context);
}

VAStatus MediaLibvaInterfaceNext::ProtectedSessionExecute(
    VADriverContextP      ctx,
    VAProtectedSessionID  protectedSession,
    VABufferID            data)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx,                          "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                     "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_compList[CompCp], "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[CompCp]->ProtectedSessionExecute(ctx, protectedSession, data);
}
#endif

CompType MediaLibvaInterfaceNext::MapComponentFromCtxType(uint32_t ctxType)
{
    switch (ctxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
            return CompDecode;
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            return CompEncode;
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            return CompVp;
        case DDI_MEDIA_CONTEXT_TYPE_PROTECTED:
            return CompCp;
        default:
            return CompCommon;
    }
}

VAStatus MediaLibvaInterfaceNext::CreateSurfaces (
    VADriverContextP    ctx,
    int32_t             width,
    int32_t             height,
    int32_t             format,
    int32_t             surfacesNum,
    VASurfaceID         *surfaces
)
{
    DDI_FUNC_ENTER;
    int32_t event[] = {width, height, format};
    MOS_TraceEventExt(EVENT_VA_SURFACE, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL(ctx,               "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LARGER(surfacesNum, 0, "Invalid surfacesNum", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(surfaces,          "nullptr surfaces",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(width,        0, "Invalid width",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(height,       0, "Invalid height",       VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaDrvCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaDrvCtx,       "nullptr mediaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
}

VAStatus MediaLibvaInterfaceNext::DestroySurfaces (
    VADriverContextP    ctx,
    VASurfaceID         *surfaces,
    int32_t             surfacesNum
)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL  (ctx,             "nullptr ctx",             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LARGER(surfacesNum, 0,  "Invalid surfacesNum",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL  (surfaces,        "nullptr surfaces",        VA_STATUS_ERROR_INVALID_PARAMETER);
    MOS_TraceEventExt(EVENT_VA_FREE_SURFACE, EVENT_TYPE_START, &surfacesNum, sizeof(int32_t), surfaces, surfacesNum*sizeof(VAGenericID));

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL  (mediaCtx,                  "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL  (mediaCtx->pSurfaceHeap,    "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_SURFACE surface = nullptr;
    for(int32_t i = 0; i < surfacesNum; i++)
    {
        DDI_CHK_LESS((uint32_t)surfaces[i], mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaces", VA_STATUS_ERROR_INVALID_SURFACE);
        surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surfaces[i]);
        DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
        if(surface->pCurrentFrameSemaphore)
        {
            MediaLibvaUtilNext::WaitSemaphore(surface->pCurrentFrameSemaphore);
            MediaLibvaUtilNext::PostSemaphore(surface->pCurrentFrameSemaphore);
        }
        if(surface->pReferenceFrameSemaphore)
        {
            MediaLibvaUtilNext::WaitSemaphore(surface->pReferenceFrameSemaphore);
            MediaLibvaUtilNext::PostSemaphore(surface->pReferenceFrameSemaphore);
        }
    }

    for(int32_t i = 0; i < surfacesNum; i++)
    {
        DDI_CHK_LESS((uint32_t)surfaces[i], mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaces", VA_STATUS_ERROR_INVALID_SURFACE);
        surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surfaces[i]);
        DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
        if(surface->pCurrentFrameSemaphore)
        {
            MediaLibvaUtilNext::DestroySemaphore(surface->pCurrentFrameSemaphore);
            surface->pCurrentFrameSemaphore = nullptr;
        }

        if(surface->pReferenceFrameSemaphore)
        {
            MediaLibvaUtilNext::DestroySemaphore(surface->pReferenceFrameSemaphore);
            surface->pReferenceFrameSemaphore = nullptr;
        }

        MediaLibvaUtilNext::UnRegisterRTSurfaces(ctx, surface);

        MosUtilities::MosLockMutex(&mediaCtx->SurfaceMutex);
        MediaLibvaUtilNext::FreeSurface(surface);
        MOS_FreeMemory(surface);
        MediaLibvaUtilNext::ReleasePMediaSurfaceFromHeap(mediaCtx->pSurfaceHeap, (uint32_t)surfaces[i]);
        mediaCtx->uiNumSurfaces--;
        MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
    }

    MOS_TraceEventExt(EVENT_VA_FREE_SURFACE, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::CreateSurfaces2 (
    VADriverContextP  ctx,
    uint32_t          format,
    uint32_t          width,
    uint32_t          height,
    VASurfaceID       *surfaces,
    uint32_t          surfacesNum,
    VASurfaceAttrib   *attribList,
    uint32_t          attribsNum
    )
{
    DDI_FUNC_ENTER;

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    uint32_t event[] = {width, height, format};
    MOS_TraceEventExt(EVENT_VA_SURFACE, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL  (ctx,             "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LARGER(surfacesNum,  0, "Invalid surfacesNum",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL  (surfaces,        "nullptr surfaces",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(width,        0, "Invalid width",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(height,       0, "Invalid height",       VA_STATUS_ERROR_INVALID_PARAMETER);

    if(attribsNum > 0)
    {
        DDI_CHK_NULL(attribList, "nullptr attribList", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    PDDI_MEDIA_CONTEXT mediaCtx    = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,       "nullptr mediaCtx",   VA_STATUS_ERROR_INVALID_CONTEXT);

    int32_t expectedFourcc = VA_FOURCC_NV12;

    vaStatus = RtFormatToOsFormat(format, expectedFourcc);
    if(vaStatus != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Invalid VAConfigAttribRTFormat: 0x%x. Please uses the format defined in libva/va.h", format);
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    VASurfaceAttribExternalBuffers externalBufDescripor;
    VADRMPRIMESurfaceDescriptor drmPrimeSurfaceDescriptor;
    MosUtilities::MosZeroMemory(&externalBufDescripor, sizeof(VASurfaceAttribExternalBuffers));
    MosUtilities::MosZeroMemory(&drmPrimeSurfaceDescriptor, sizeof(VADRMPRIMESurfaceDescriptor)); 
#if VA_CHECK_VERSION(1, 21, 0)
    VADRMPRIME3SurfaceDescriptor drmPrime3SurfaceDescriptor;
    MosUtilities::MosZeroMemory(&drmPrime3SurfaceDescriptor, sizeof(VADRMPRIME3SurfaceDescriptor));
#endif

    int32_t  memTypeFlag      = VA_SURFACE_ATTRIB_MEM_TYPE_VA;
    int32_t  descFlag         = 0;
    uint32_t surfaceUsageHint = VA_SURFACE_ATTRIB_USAGE_HINT_GENERIC;
    bool     surfDescProvided = false;
    bool     surfIsUserPtr    = false;

    for (uint32_t i = 0; i < attribsNum && attribList; i++)
    {
        if (attribList[i].flags & VA_SURFACE_ATTRIB_SETTABLE)
        {
            switch (attribList[i].type)
            {
            case VASurfaceAttribUsageHint:
                DDI_ASSERT(attribList[i].value.type == VAGenericValueTypeInteger);
                surfaceUsageHint = attribList[i].value.value.i;
                break;
            case VASurfaceAttribPixelFormat:
                DDI_ASSERT(attribList[i].value.type == VAGenericValueTypeInteger);
                expectedFourcc = attribList[i].value.value.i;
                break;
            case VASurfaceAttribMemoryType:
                DDI_ASSERT(attribList[i].value.type == VAGenericValueTypeInteger);
                if ((attribList[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_VA)          ||
                    (attribList[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM)  ||
                    (attribList[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME)   ||
                    (attribList[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2) ||
#if VA_CHECK_VERSION(1, 21, 0)
                    (attribList[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3) ||
#endif
                    (attribList[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR))
                {
                    memTypeFlag = attribList[i].value.value.i;
                    surfIsUserPtr = (attribList[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR);
                }
                else
                {
                    DDI_ASSERTMESSAGE("Not supported external buffer type.");
                    return VA_STATUS_ERROR_INVALID_PARAMETER;
                }
                break;
            case (VASurfaceAttribType)VASurfaceAttribExternalBufferDescriptor:
                DDI_ASSERT(attribList[i].value.type == VAGenericValueTypePointer);
                if( nullptr == attribList[i].value.value.p )
                {
                    DDI_ASSERTMESSAGE("Invalid VASurfaceAttribExternalBuffers used.");
                    //remove the check for libva-utils conformance test, need libva-utils change cases
                    //after libva-utils fix the case, return VA_STATUS_ERROR_INVALID_PARAMETER;
                    break;
                }
                surfDescProvided = true;
                if (memTypeFlag == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2)
                {
                    MosUtilities::MosSecureMemcpy(&drmPrimeSurfaceDescriptor, sizeof(VADRMPRIMESurfaceDescriptor),  attribList[i].value.value.p, sizeof(VADRMPRIMESurfaceDescriptor));
                    expectedFourcc  = drmPrimeSurfaceDescriptor.fourcc;
                    width            = drmPrimeSurfaceDescriptor.width;
                    height           = drmPrimeSurfaceDescriptor.height;
                }
#if VA_CHECK_VERSION(1, 21, 0)
                else if (memTypeFlag == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3)
                {
                    MosUtilities::MosSecureMemcpy(&drmPrime3SurfaceDescriptor, sizeof(VADRMPRIME3SurfaceDescriptor), attribList[i].value.value.p, sizeof(VADRMPRIME3SurfaceDescriptor));
                    expectedFourcc            = drmPrime3SurfaceDescriptor.fourcc;
                    width                     = drmPrime3SurfaceDescriptor.width;
                    height                    = drmPrime3SurfaceDescriptor.height;
                    drmPrimeSurfaceDescriptor = *((VADRMPRIMESurfaceDescriptor*)&drmPrime3SurfaceDescriptor);
                    descFlag                  = drmPrime3SurfaceDescriptor.flags;
                }
#endif
                else
                {
                    MosUtilities::MosSecureMemcpy(&externalBufDescripor, sizeof(VASurfaceAttribExternalBuffers),  attribList[i].value.value.p, sizeof(VASurfaceAttribExternalBuffers));

                    expectedFourcc  = externalBufDescripor.pixel_format;
                    width            = externalBufDescripor.width;
                    height           = externalBufDescripor.height;
                    // the following code is for backward compatible and it will be removed in the future
                    // new implemention should use VASurfaceAttribMemoryType attrib and set its value to VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM
                    if ((externalBufDescripor.flags & VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM )||
                        (externalBufDescripor.flags & VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME)||
                        (externalBufDescripor.flags & VA_SURFACE_EXTBUF_DESC_PROTECTED)||
                        (externalBufDescripor.flags & VA_SURFACE_EXTBUF_DESC_ENABLE_TILING))
                    {
                        if (externalBufDescripor.flags & VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM)
                        {
                            memTypeFlag = VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM;
                        }
                        else if (externalBufDescripor.flags & VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME)
                        {
                            memTypeFlag = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
                        }

                        descFlag      = (externalBufDescripor.flags & ~(VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM | VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME));
                        surfIsUserPtr = false;
                    }
                }

                break;
            default:
                DDI_ASSERTMESSAGE("Unsupported type.");
                break;
            }
        }
    }

    DDI_MEDIA_FORMAT mediaFmt = OsFormatToMediaFormat(expectedFourcc, format);
    if (mediaFmt == Media_Format_Count)
    {
        DDI_ASSERTMESSAGE("DDI: unsupported surface type in CreateSurfaces2.");
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    for (uint32_t i = 0; i < surfacesNum; i++)
    {
        PDDI_MEDIA_SURFACE_DESCRIPTOR surfDesc = nullptr;
        MOS_STATUS                    eStatus = MOS_STATUS_SUCCESS;

        if (surfDescProvided == true)
        {
            surfDesc = (PDDI_MEDIA_SURFACE_DESCRIPTOR)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE_DESCRIPTOR));
            if (surfDesc == nullptr)
            {
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
            surfDesc->uiFlags        = descFlag;
            surfDesc->uiVaMemType    = memTypeFlag;

            if (
#if VA_CHECK_VERSION(1, 21, 0)
                memTypeFlag == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 || 
#endif
                memTypeFlag == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2)
            {
                surfDesc->ulBuffer       = drmPrimeSurfaceDescriptor.objects[0].fd;
                surfDesc->modifier       = drmPrimeSurfaceDescriptor.objects[0].drm_format_modifier;
                surfDesc->uiSize         = drmPrimeSurfaceDescriptor.objects[0].size;
                surfDesc->uiBuffserSize  = drmPrimeSurfaceDescriptor.objects[0].size;
                if(drmPrimeSurfaceDescriptor.num_layers > 1)
                {
                     surfDesc->uiPlanes = drmPrimeSurfaceDescriptor.num_layers;
                     for (uint32_t k = 0; k < surfDesc->uiPlanes; k ++)
                     {
                         surfDesc->uiPitches[k] = drmPrimeSurfaceDescriptor.layers[k].pitch[0];
                         surfDesc->uiOffsets[k] = drmPrimeSurfaceDescriptor.layers[k].offset[0];
                     }
                }
                else
                {
                    surfDesc->uiPlanes       = drmPrimeSurfaceDescriptor.layers[0].num_planes;
                    for(uint32_t k = 0; k < surfDesc->uiPlanes; k ++)
                    {
                        surfDesc->uiPitches[k] = drmPrimeSurfaceDescriptor.layers[0].pitch[k];
                        surfDesc->uiOffsets[k] = drmPrimeSurfaceDescriptor.layers[0].offset[k];
                    }
                }
            }
            else if (memTypeFlag != VA_SURFACE_ATTRIB_MEM_TYPE_VA)
            {
                surfDesc->uiPlanes       = externalBufDescripor.num_planes;
                surfDesc->ulBuffer       = externalBufDescripor.buffers[i];
                surfDesc->uiSize         = externalBufDescripor.data_size;
                surfDesc->uiBuffserSize  = externalBufDescripor.data_size;

                eStatus = MosUtilities::MosSecureMemcpy(surfDesc->uiPitches, sizeof(surfDesc->uiPitches), externalBufDescripor.pitches, sizeof(externalBufDescripor.pitches));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_FreeMemory(surfDesc);
                    DDI_VERBOSEMESSAGE("DDI:Failed to copy surface buffer data!");
                    return VA_STATUS_ERROR_OPERATION_FAILED;
                }
                eStatus = MosUtilities::MosSecureMemcpy(surfDesc->uiOffsets, sizeof(surfDesc->uiOffsets), externalBufDescripor.offsets, sizeof(externalBufDescripor.offsets));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_FreeMemory(surfDesc);
                    DDI_VERBOSEMESSAGE("DDI:Failed to copy surface buffer data!");
                    return VA_STATUS_ERROR_OPERATION_FAILED;
                }

                if( surfIsUserPtr )
                {
                    surfDesc->uiTile = TILING_NONE;
                    if (surfDesc->ulBuffer % 4096 != 0)
                    {
                        MOS_FreeMemory(surfDesc);
                        DDI_VERBOSEMESSAGE("Buffer Address is invalid");
                        return VA_STATUS_ERROR_INVALID_PARAMETER;
                    }
                }
            }
        }
        VASurfaceID vaSurfaceID = (VASurfaceID)CreateRenderTarget(mediaCtx, mediaFmt, width, height, surfDesc, surfaceUsageHint, MOS_MEMPOOL_VIDEOMEMORY);
        if (VA_INVALID_ID != vaSurfaceID)
        {
            surfaces[i] = vaSurfaceID;
        }
        else
        {
            // here to release the successful allocated surfaces?
            if( nullptr != surfDesc )
            {
                MOS_FreeMemory(surfDesc);
            }
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }

    MOS_TraceEventExt(EVENT_VA_SURFACE, EVENT_TYPE_END, &surfacesNum, sizeof(uint32_t), surfaces, surfacesNum*sizeof(VAGenericID));
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::BufferInfo (
    VADriverContextP ctx,
    VABufferID       bufId,
    VABufferType     *type,
    uint32_t         *size,
    uint32_t         *elementsNum)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,          "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(type,         "nullptr type",         VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(size,         "nullptr size",         VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(elementsNum,  "nullptr num_elements", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,     "nullptr mediaCtx",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)bufId, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid buf_id", VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_MEDIA_BUFFER *buf  = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, bufId);
    DDI_CHK_NULL(buf,          "nullptr buffer",       VA_STATUS_ERROR_INVALID_BUFFER);

    *type         = (VABufferType)buf->uiType;
    *size         = buf->iSize / buf->uiNumElements;
    *elementsNum  = buf->uiNumElements;

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::GetSurfaceAttributes(
    VADriverContextP   ctx,
    VAConfigID         config,
    VASurfaceAttrib    *attribList,
    uint32_t           attribsNum)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(config);
    DDI_UNUSED(attribList);
    DDI_UNUSED(attribsNum);

    DDI_FUNC_ENTER;

    VAStatus vaStatus = VA_STATUS_ERROR_UNIMPLEMENTED;

    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::AcquireBufferHandle(
    VADriverContextP ctx,
    VABufferID       bufId,
    VABufferInfo     *bufInfo)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,       "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(bufInfo,   "nullptr buf_info",     VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,  "Invalid Media ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_BUFFER   *buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, bufId);
    DDI_CHK_NULL(buf,       "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(buf->bo,   "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);

    // If user did not specify memtype he want's we use something we prefer, we prefer PRIME
    if (!bufInfo->mem_type)
    {
       bufInfo->mem_type = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
    }
    // now chekcing memtype whether we support it
    if ((bufInfo->mem_type != VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME) &&
        (bufInfo->mem_type != VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM))
    {
        return VA_STATUS_ERROR_UNSUPPORTED_MEMORY_TYPE;
    }

    MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
    // already acquired?
    if (buf->uiExportcount)
    {   // yes, already acquired
        // can't provide access thru another memtype
        if (buf->uiMemtype != bufInfo->mem_type)
        {
            MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }
    else
    {   // no, not acquired - doing this now
        switch (bufInfo->mem_type) 
        {
            case VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM:
            {
                uint32_t flink = 0;
                if (mos_bo_flink(buf->bo, &flink) != 0)
                {
                    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
                    return VA_STATUS_ERROR_INVALID_BUFFER;
                }
                buf->handle = (intptr_t)flink;
                break;
            }
            case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME:
            {
                int32_t prime_fd = 0;
                if (mos_bo_export_to_prime(buf->bo, &prime_fd) != 0)
                {
                    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
                    return VA_STATUS_ERROR_INVALID_BUFFER;
                }
                buf->handle = (intptr_t)prime_fd;
                break;
            }
        }
        // saving memtepy which was provided to the user
        buf->uiMemtype = bufInfo->mem_type;
    }

    ++buf->uiExportcount;
    mos_bo_reference(buf->bo);

    bufInfo->type     = buf->uiType;
    bufInfo->handle   = buf->handle;
    bufInfo->mem_size = buf->uiNumElements * buf->iSize;

    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::ReleaseBufferHandle(
    VADriverContextP ctx,
    VABufferID       bufId)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx,      "nullptr ctx",           VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "Invalid Media ctx",     VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_BUFFER   *buf     = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, bufId);
    DDI_CHK_NULL(buf,       "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(buf->bo,   "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);

    MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
    if (!buf->uiMemtype || !buf->uiExportcount)
    {
        MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
        return VA_STATUS_SUCCESS;
    }
    mos_bo_unreference(buf->bo);
    --buf->uiExportcount;

    if (!buf->uiExportcount)
    {
        switch (buf->uiMemtype)
        {
            case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME: 
            {
                close((intptr_t)buf->handle);
                break;
            }
        }
        buf->uiMemtype = 0;
    }
    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);

    if (!buf->uiExportcount && buf->bPostponedBufFree)
    {
        MOS_FreeMemory(buf);
        DestroyBufFromVABufferID(mediaCtx, bufId);
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::InitSurfaceDescriptorWithoutAuxTableMgr(
    VADRMPRIMESurfaceDescriptor *desc,
    uint32_t                    *formats,
    int                         compositeObject,
    uint32_t                    planesNum,
    uint32_t                    offsetY,
    uint32_t                    offsetU,
    uint32_t                    offsetV,
    uint32_t                    pitch,
    uint32_t                    chromaPitch)
{
    DDI_CHK_NULL(desc,    "Invaild surface descriptor.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(formats, "Invaild formats.",            VA_STATUS_ERROR_INVALID_PARAMETER);

    if(compositeObject)
    {
        desc->num_layers = 1;
        desc->layers[0].drm_format = formats[0];
        desc->layers[0].num_planes = planesNum;

        for (int i = 0; i < planesNum; i++)
        {
            desc->layers[0].object_index[i] = 0;
            switch(i)
            {
            case 0:
                desc->layers[0].offset[i] = offsetY;
                desc->layers[0].pitch[i]  = pitch;
                break;
            case 1:
                if (desc->fourcc == VA_FOURCC_YV12)
                {
                    desc->layers[0].offset[i] = offsetV;
                }
                else
                {
                    desc->layers[0].offset[i] = offsetU;
                }
                desc->layers[0].pitch[i]  = chromaPitch;
                break;
            case 2:
                if (desc->fourcc == VA_FOURCC_YV12)
                {
                    desc->layers[0].offset[i] = offsetU;
                }
                else
                {
                    desc->layers[0].offset[i] = offsetV;
                }
                desc->layers[0].pitch[i]  = chromaPitch;
                break;
            default:
                DDI_ASSERTMESSAGE("vaExportSurfaceHandle: invalid plan numbers");
            }
        }
    }
    else
    {
        desc->num_layers = planesNum;

        for (int i = 0; i < planesNum; i++)
        {
            desc->layers[i].drm_format = formats[i];
            desc->layers[i].num_planes = 1;

            desc->layers[i].object_index[0] = 0;

            switch(i)
            {
            case 0:
                desc->layers[i].offset[0] = offsetY;
                desc->layers[i].pitch[0]  = pitch;
                break;
            case 1:
                if (desc->fourcc == VA_FOURCC_YV12)
                {
                    desc->layers[i].offset[0] = offsetV;
                }
                else
                {
                    desc->layers[i].offset[0] = offsetU;
                }
                desc->layers[i].pitch[0]  = chromaPitch;
                break;
            case 2:
                if (desc->fourcc == VA_FOURCC_YV12)
                {
                    desc->layers[i].offset[0] = offsetU;
                }
                else
                {
                    desc->layers[i].offset[0] = offsetV;
                }
                desc->layers[i].pitch[0]  = chromaPitch;
                break;
            default:
                DDI_ASSERTMESSAGE("vaExportSurfaceHandle: invalid plan numbers");
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::InitSurfaceDescriptorWithAuxTableMgr(
    VADRMPRIMESurfaceDescriptor *desc,
    uint32_t                    *formats,
    int                         compositeObject,
    uint32_t                    planesNum,
    uint32_t                    offsetY,
    uint32_t                    offsetU,
    uint32_t                    offsetV,
    uint32_t                    auxOffsetY,
    uint32_t                    auxOffsetUV,
    int32_t                     pitch)
{
    DDI_CHK_NULL(desc,    "Invaild surface descriptor.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(formats, "Invaild formats.",            VA_STATUS_ERROR_INVALID_PARAMETER);

    if(compositeObject)
    {
        desc->num_layers = 1;
        desc->layers[0].drm_format = formats[0];
        desc->layers[0].num_planes = planesNum;
        // For semi-planar formats like NV12, CCS planes follow the Y and UV planes,
        // i.e. planes 0 and 1 are used for Y and UV surfaces, planes 2 and 3 for the respective CCS.
        for (int i = 0; i < planesNum/2; i++)
        {
            desc->layers[0].object_index[2*i] = 0;
            desc->layers[0].object_index[2*i+1] = 0;
            if (i == 0)
            {
                // Y plane
                desc->layers[0].offset[i] = offsetY;
                desc->layers[0].pitch[i]  = pitch;
                // Y aux plane
                desc->layers[0].offset[i + planesNum/2] = auxOffsetY;
                desc->layers[0].pitch[i + planesNum/2] = pitch/8;
            }
            else
            {
                // UV plane
                desc->layers[0].offset[i] = offsetU;
                desc->layers[0].pitch[i]  = pitch;
                // UV aux plane
                desc->layers[0].offset[i + planesNum/2] = auxOffsetUV;
                desc->layers[0].pitch[i + planesNum/2] = pitch/8;
            }
        }
    }
    else
    {
        desc->num_layers = planesNum / 2;

        for (int i = 0; i < desc->num_layers; i++)
        {
            desc->layers[i].drm_format = formats[i];
            desc->layers[i].num_planes = 2;
            desc->layers[i].object_index[0] = 0;

            if (i == 0)
            {
                desc->layers[i].offset[0] = offsetY;
                desc->layers[i].offset[1] = auxOffsetY;
                desc->layers[i].pitch[0]  = pitch;
                desc->layers[i].pitch[1]  = pitch/8;
            }
            else
            {
                desc->layers[i].offset[0] = offsetU;
                desc->layers[i].offset[1] = auxOffsetUV;
                desc->layers[i].pitch[0]  = pitch;
                desc->layers[i].pitch[1]  = pitch/8;
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::ExportSurfaceHandle(
    VADriverContextP ctx,
    VASurfaceID      surfaceId,
    uint32_t         memType,
    uint32_t         flags,
    void             *descriptor)
{
    DDI_FUNC_ENTER;

    DDI_CHK_NULL(descriptor, "nullptr descriptor",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ctx,        "nullptr ctx",         VA_STATUS_ERROR_INVALID_CONTEXT);
    
    VAStatus status = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)(surfaceId), mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaces", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE  *mediaSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surfaceId);
    DDI_CHK_NULL(mediaSurface,                   "nullptr mediaSurface",                   VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->bo,               "nullptr mediaSurface->bo",               VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->pGmmResourceInfo, "nullptr mediaSurface->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_SURFACE);

    if (
#if VA_CHECK_VERSION(1, 21, 0)
        memType != VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3 && 
#endif
        memType != VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2)
    {
        DDI_ASSERTMESSAGE("vaExportSurfaceHandle: memory type %08x is not supported.\n", memType);
        return VA_STATUS_ERROR_UNSUPPORTED_MEMORY_TYPE;
    }

    if (mos_bo_export_to_prime(mediaSurface->bo, (int32_t*)&mediaSurface->name))
    {
        DDI_ASSERTMESSAGE("Failed drm_intel_gem_export_to_prime operation!!!\n");
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    VADRMPRIMESurfaceDescriptor *desc = (VADRMPRIMESurfaceDescriptor *)descriptor;
#if VA_CHECK_VERSION(1, 21, 0)
    if(memType == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_3)
    {
        VADRMPRIME3SurfaceDescriptor *desc = (VADRMPRIME3SurfaceDescriptor *)descriptor;
        if(mediaSurface->pGmmResourceInfo->GetSetCpSurfTag(false, 0))
        {
            desc->flags |= VA_SURFACE_EXTBUF_DESC_PROTECTED;
        }
    }
#endif

    desc->fourcc = MediaFormatToOsFormat(mediaSurface->format);
    if(desc->fourcc == VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT)
    {
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }
    desc->width           = mediaSurface->iWidth;
    desc->height          = mediaSurface->iRealHeight;
    desc->num_objects     = 1;
    desc->objects[0].fd   = mediaSurface->name;
    desc->objects[0].size = mediaSurface->pGmmResourceInfo->GetSizeSurface();

    if(VA_STATUS_SUCCESS != MediaLibvaUtilNext::GetSurfaceModifier(mediaCtx, mediaSurface, desc->objects[0].drm_format_modifier))
    {
        DDI_ASSERTMESSAGE("could not find related modifier values");
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    // Query Aux Plane info form GMM
    bool hasAuxPlane           = false;
    GMM_RESOURCE_FLAG GmmFlags = mediaSurface->pGmmResourceInfo->GetResFlags();

    if (((GmmFlags.Gpu.MMC                           ||
          GmmFlags.Gpu.CCS)                          &&
         (GmmFlags.Info.MediaCompressed              ||
          GmmFlags.Info.RenderCompressed)            &&
          mediaCtx->m_auxTableMgr) )
          {
              hasAuxPlane = true;
          }
          else
          {
              hasAuxPlane = false;
          }

    int compositeObject = flags & VA_EXPORT_SURFACE_COMPOSED_LAYERS;

    uint32_t formats[4];
    uint32_t planesNum = GetPlaneNum(mediaSurface, hasAuxPlane);

    if(compositeObject)
    {
        formats[0] = GetDrmFormatOfCompositeObject(desc->fourcc);
        if(!formats[0])
        {
            DDI_ASSERTMESSAGE("vaExportSurfaceHandle: fourcc %08x is not supported for export as a composite object.\n", desc->fourcc);
            return VA_STATUS_ERROR_INVALID_SURFACE;
        }
    }
    else
    {
        for (int i = 0; i < planesNum; i++)
        {
            formats[i] = GetDrmFormatOfSeparatePlane(desc->fourcc,i);
            if (!formats[i])
            {
                DDI_ASSERTMESSAGE("vaExportSurfaceHandle: fourcc %08x is not supported for export as separate planes.\n", desc->fourcc);
                return VA_STATUS_ERROR_INVALID_SURFACE;
            }
        }
    }

    uint32_t pitch, height, chromaPitch, chromaHeight = 0;
    pitch = mediaSurface->iPitch;
    height = mediaSurface->iRealHeight;
    GetChromaPitchHeight(desc->fourcc, pitch, height, &chromaPitch, &chromaHeight);

    // Get offset from GMM
    GMM_REQ_OFFSET_INFO reqInfo = {0};
    reqInfo.Plane = GMM_PLANE_Y;
    reqInfo.ReqRender = 1;
    mediaSurface->pGmmResourceInfo->GetOffset(reqInfo);
    uint32_t offsetY = reqInfo.Render.Offset;
    MOS_ZeroMemory(&reqInfo, sizeof(GMM_REQ_OFFSET_INFO));
    reqInfo.Plane = GMM_PLANE_U;
    reqInfo.ReqRender = 1;
    mediaSurface->pGmmResourceInfo->GetOffset(reqInfo);
    uint32_t offsetU = reqInfo.Render.Offset;
    MOS_ZeroMemory(&reqInfo, sizeof(GMM_REQ_OFFSET_INFO));
    reqInfo.Plane = GMM_PLANE_V;
    reqInfo.ReqRender = 1;
    mediaSurface->pGmmResourceInfo->GetOffset(reqInfo);
    uint32_t offsetV = reqInfo.Render.Offset;
    uint32_t auxOffsetY = (uint32_t)mediaSurface->pGmmResourceInfo->GetPlanarAuxOffset(0, GMM_AUX_Y_CCS);
    uint32_t auxOffsetUV = (uint32_t)mediaSurface->pGmmResourceInfo->GetPlanarAuxOffset(0, GMM_AUX_UV_CCS);

    if(hasAuxPlane)
    {
        status = InitSurfaceDescriptorWithAuxTableMgr(desc, formats, compositeObject, planesNum,
            offsetY, offsetU, offsetV, auxOffsetY, auxOffsetUV, mediaSurface->iPitch);
    }
    else
    {
        status = InitSurfaceDescriptorWithoutAuxTableMgr(desc, formats, compositeObject, planesNum,
            offsetY, offsetU, offsetV, pitch, chromaPitch);
    }

    return status;
}

bool MediaLibvaInterfaceNext::DestroyBufFromVABufferID(
    PDDI_MEDIA_CONTEXT mediaCtx,
    VABufferID         bufferID)
{
    MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
    MediaLibvaUtilNext::ReleasePMediaBufferFromHeap(mediaCtx->pBufferHeap, bufferID);
    mediaCtx->uiNumBufs--;
    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
    return true;
}

VAStatus MediaLibvaInterfaceNext::GenerateVaImgFromMediaFormat(
    DDI_MEDIA_SURFACE *mediaSurface,
    PDDI_MEDIA_CONTEXT mediaCtx,
    VAImage           *vaimg)
{
    DDI_CHK_NULL(vaimg,            "Invaild VAImage.",            VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mediaSurface,     "Invaild media surface.",      VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mediaCtx,         "Invaild media context.",      VA_STATUS_ERROR_INVALID_CONTEXT);
    
    GMM_RESOURCE_INFO *gmmResourceInfo = mediaSurface->pGmmResourceInfo;
    DDI_CHK_NULL(gmmResourceInfo,  "Invaild gmm resource info.",  VA_STATUS_ERROR_INVALID_PARAMETER);

    GMM_REQ_OFFSET_INFO reqInfo     = {0};
    reqInfo.Plane                   = GMM_PLANE_U;
    reqInfo.ReqRender               = 1;
    gmmResourceInfo->GetOffset(reqInfo);
    uint32_t offsetU                = reqInfo.Render.Offset;
    MOS_ZeroMemory(&reqInfo, sizeof(GMM_REQ_OFFSET_INFO));
    reqInfo.Plane                   = GMM_PLANE_V;
    reqInfo.ReqRender               = 1;
    gmmResourceInfo->GetOffset(reqInfo);
    uint32_t offsetV                = reqInfo.Render.Offset;
    vaimg->data_size                = (uint32_t)gmmResourceInfo->GetSizeSurface();

    switch( mediaSurface->format )
    {
    case Media_Format_YV12:
    case Media_Format_I420:
    case Media_Format_IYUV:
        vaimg->format.bits_per_pixel    = 12;
        vaimg->num_planes               = 3;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->pitches[1]               =
        vaimg->pitches[2]               = mediaSurface->iPitch / 2;
        vaimg->offsets[0]               = 0;
        vaimg->offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        vaimg->offsets[2]               = vaimg->offsets[1] + vaimg->pitches[1] * MOS_ALIGN_CEIL(mediaSurface->iHeight, 2) / 2;
        break;
    case Media_Format_A8B8G8R8:
    case Media_Format_R8G8B8A8:
    case Media_Format_A8R8G8B8:
        vaimg->format.bits_per_pixel    = 32;
        vaimg->format.alpha_mask        = RGB_8BIT_ALPHAMASK;
        vaimg->num_planes               = 1;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        break;
    case Media_Format_X8R8G8B8:
    case Media_Format_X8B8G8R8:
        vaimg->format.bits_per_pixel    = 32;
        vaimg->num_planes               = 1;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        break;
    case Media_Format_R10G10B10A2:
    case Media_Format_B10G10R10A2:
        vaimg->format.bits_per_pixel    = 32;
        vaimg->format.alpha_mask        = RGB_10BIT_ALPHAMASK;
        vaimg->num_planes               = 1;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        break;
    case Media_Format_R10G10B10X2:
    case Media_Format_B10G10R10X2:
        vaimg->format.bits_per_pixel    = 32;
        vaimg->num_planes               = 1;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        break;
    case Media_Format_R5G6B5:
        vaimg->format.bits_per_pixel    = 16;
        vaimg->data_size                = mediaSurface->iPitch * mediaSurface->iHeight;
        vaimg->num_planes               = 1;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        break;
    case Media_Format_R8G8B8:
        vaimg->format.bits_per_pixel    = 24;
        vaimg->num_planes               = 1;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        break;
    case Media_Format_YUY2:
    case Media_Format_UYVY:
        vaimg->format.bits_per_pixel    = 16;
        vaimg->data_size                = mediaSurface->iPitch * mediaSurface->iHeight;
        vaimg->num_planes               = 1;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        break;
    case Media_Format_400P:
        vaimg->format.bits_per_pixel    = 8;
        vaimg->num_planes               = 1;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        break;
    case Media_Format_444P:
    case Media_Format_RGBP:
    case Media_Format_BGRP:
        vaimg->format.bits_per_pixel    = 24;
        vaimg->num_planes               = 3;
        vaimg->pitches[0]               =
        vaimg->pitches[1]               =
        vaimg->pitches[2]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        vaimg->offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        vaimg->offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch * 2;
        break;
    case Media_Format_IMC3:
        vaimg->format.bits_per_pixel    = 12;
        vaimg->num_planes               = 3;
        vaimg->pitches[0]               =
        vaimg->pitches[1]               =
        vaimg->pitches[2]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        vaimg->offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        vaimg->offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch * 3 / 2;
        break;
    case Media_Format_411P:
        vaimg->format.bits_per_pixel    = 12;
        vaimg->num_planes               = 3;
        vaimg->pitches[0]               =
        vaimg->pitches[1]               =
        vaimg->pitches[2]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        vaimg->offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        vaimg->offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch * 2;
        break;
    case Media_Format_422V:
        vaimg->format.bits_per_pixel    = 16;
        vaimg->num_planes               = 3;
        vaimg->pitches[0]               =
        vaimg->pitches[1]               =
        vaimg->pitches[2]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        vaimg->offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        vaimg->offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch * 3 / 2;
        break;
    case Media_Format_422H:
        vaimg->format.bits_per_pixel    = 16;
        vaimg->num_planes               = 3;
        vaimg->pitches[0]               =
        vaimg->pitches[1]               =
        vaimg->pitches[2]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        vaimg->offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        vaimg->offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch * 2;
        break;
    case Media_Format_P010:
    case Media_Format_P012:
    case Media_Format_P016:
        vaimg->format.bits_per_pixel    = 24;
        vaimg->num_planes               = 2;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->pitches[1]               =
        vaimg->pitches[2]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        vaimg->offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        vaimg->offsets[2]               = vaimg->offsets[1] + 2;
        break;
    case Media_Format_Y410:
    case Media_Format_AYUV:
#if VA_CHECK_VERSION(1, 13, 0)
    case Media_Format_XYUV:
#endif
    case Media_Format_Y210:
#if VA_CHECK_VERSION(1, 9, 0)
    case Media_Format_Y212:
#endif
    case Media_Format_Y216:
        vaimg->format.bits_per_pixel    = 32;
        vaimg->data_size                = mediaSurface->iPitch * mediaSurface->iHeight;
        vaimg->num_planes               = 1;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        break;
#if VA_CHECK_VERSION(1, 9, 0)
    case Media_Format_Y412:
#endif
    case Media_Format_Y416:
        vaimg->format.bits_per_pixel    = 64; // packed format [alpha, Y, U, V], 16 bits per channel
        vaimg->num_planes               = 1;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        break;
     default:
        vaimg->format.bits_per_pixel    = 12;
        vaimg->num_planes               = 2;
        vaimg->pitches[0]               = mediaSurface->iPitch;
        vaimg->pitches[1]               =
        vaimg->pitches[2]               = mediaSurface->iPitch;
        vaimg->offsets[0]               = 0;
        if(MEDIA_IS_WA(&mediaCtx->WaTable, WaDisableGmmLibOffsetInDeriveImage))
        {
            vaimg->offsets[1]           = mediaSurface->iHeight * mediaSurface->iPitch;
            vaimg->offsets[2]           = vaimg->offsets[1] + 1;
        }
        else
        {
            vaimg->offsets[1]           = offsetU;
            vaimg->offsets[2]           = offsetV;
        }
        break;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::GenerateVaImgFromOsFormat(
    VAImageFormat      format,
    int32_t            width,
    int32_t            height,
    GMM_RESOURCE_INFO  *gmmResourceInfo,
    VAImage            *vaimg)
{
    DDI_CHK_NULL(vaimg,            "Invaild VAImage.",          VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(gmmResourceInfo,  "Invaild gmmResourceInfo.",  VA_STATUS_ERROR_INVALID_PARAMETER);

    // Get offset from GMM
    GMM_REQ_OFFSET_INFO reqInfo = {0};
    reqInfo.Plane               = GMM_PLANE_U;
    reqInfo.ReqRender           = 1;
    gmmResourceInfo->GetOffset(reqInfo);
    uint32_t offsetU            = reqInfo.Render.Offset;

    MOS_ZeroMemory(&reqInfo, sizeof(GMM_REQ_OFFSET_INFO));
    reqInfo.Plane               = GMM_PLANE_V;
    reqInfo.ReqRender           = 1;
    gmmResourceInfo->GetOffset(reqInfo);
    uint32_t offsetV            = reqInfo.Render.Offset;

    uint32_t size               = (uint32_t)gmmResourceInfo->GetSizeSurface();
    uint32_t pitch              = (uint32_t)gmmResourceInfo->GetRenderPitch();

    vaimg->format               = format;
    vaimg->format.byte_order    = VA_LSB_FIRST;
    vaimg->width                = width;
    vaimg->height               = height;
    vaimg->data_size            = size;

    switch(format.fourcc)
    {
        case VA_FOURCC_RGBA:
        case VA_FOURCC_BGRA:
        case VA_FOURCC_ARGB:
        case VA_FOURCC_ABGR:
        case VA_FOURCC_BGRX:
        case VA_FOURCC_RGBX:
        case VA_FOURCC_XRGB:
        case VA_FOURCC_XBGR:
        case VA_FOURCC_A2R10G10B10:
        case VA_FOURCC_A2B10G10R10:
        case VA_FOURCC_X2R10G10B10:
        case VA_FOURCC_X2B10G10R10:
        case VA_FOURCC_R8G8B8:
        case VA_FOURCC_RGB565:
        case VA_FOURCC_UYVY:
        case VA_FOURCC_YUY2:
        case VA_FOURCC_VYUY:
        case VA_FOURCC_YVYU:
        case VA_FOURCC_AYUV:
#if VA_CHECK_VERSION(1, 13, 0)
        case VA_FOURCC_XYUV:
#endif
        case VA_FOURCC_Y210:
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y212:
#endif
        case VA_FOURCC_Y216:
        case VA_FOURCC_Y410:
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y412:
#endif
        case VA_FOURCC_Y416:
        case VA_FOURCC_Y800:
            vaimg->num_planes = 1;
            vaimg->pitches[0] = pitch;
            vaimg->offsets[0] = 0;
            break;
        case VA_FOURCC_NV12:
        case VA_FOURCC_NV21:
        case VA_FOURCC_P010:
        case VA_FOURCC_P012:
        case VA_FOURCC_P016:
            vaimg->num_planes = 2;
            vaimg->pitches[0] = pitch;
            vaimg->pitches[1] = pitch;
            vaimg->offsets[0] = 0;
            vaimg->offsets[1] = offsetU;
            break;
        case VA_FOURCC_YV12:
            vaimg->num_planes = 3;
            vaimg->pitches[0] = pitch;
            vaimg->pitches[1] = pitch / 2;
            vaimg->pitches[2] = pitch / 2;
            vaimg->offsets[0] = 0;
            vaimg->offsets[1] = offsetV;
            vaimg->offsets[2] = offsetU;
            break;
        case VA_FOURCC_I420:
            vaimg->num_planes = 3;
            vaimg->pitches[0] = pitch;
            vaimg->pitches[1] = pitch / 2;
            vaimg->pitches[2] = pitch / 2;
            vaimg->offsets[0] = 0;
            vaimg->offsets[1] = offsetU;
            vaimg->offsets[2] = offsetV;
            break;
        case VA_FOURCC_IMC3:
        case VA_FOURCC_411P:
        case VA_FOURCC_422V:
        case VA_FOURCC_422H:
        case VA_FOURCC_444P:
        case VA_FOURCC_RGBP:
        case VA_FOURCC_BGRP:
            vaimg->num_planes = 3;
            vaimg->pitches[0] = pitch;
            vaimg->pitches[1] = pitch;
            vaimg->pitches[2] = pitch;
            vaimg->offsets[0] = 0;
            vaimg->offsets[1] = offsetU;
            vaimg->offsets[2] = offsetV;
            break;
        default:
            return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaLibvaInterfaceNext::RtFormatToOsFormat(uint32_t format, int32_t &expectedFourcc)
{
    DDI_FUNC_ENTER;
    expectedFourcc = VA_FOURCC_NV12;
    switch(format)
    {
        case VA_RT_FORMAT_YUV420:
            expectedFourcc = VA_FOURCC_NV12;
            break;
        case VA_RT_FORMAT_YUV420_12:
            expectedFourcc = VA_FOURCC_P012;
            break;
        case VA_RT_FORMAT_YUV422:
            expectedFourcc = VA_FOURCC_YUY2;
            break;
        case VA_RT_FORMAT_YUV422_10:
            expectedFourcc = VA_FOURCC_Y210;
            break;
        case VA_RT_FORMAT_YUV422_12:
#if VA_CHECK_VERSION(1, 9, 0)
            expectedFourcc = VA_FOURCC_Y212;
#else
            expectedFourcc = VA_FOURCC_Y216;
#endif
            break;
        case VA_RT_FORMAT_YUV444:
            expectedFourcc = VA_FOURCC_444P;
            break;
        case VA_RT_FORMAT_YUV444_10:
            expectedFourcc = VA_FOURCC_Y410;
            break;
        case VA_RT_FORMAT_YUV444_12:
#if VA_CHECK_VERSION(1, 9, 0)
            expectedFourcc = VA_FOURCC_Y412;
#else
            expectedFourcc = VA_FOURCC_Y416;
#endif
            break;
        case VA_RT_FORMAT_YUV411:
            expectedFourcc = VA_FOURCC_411P;
            break;
        case VA_RT_FORMAT_YUV400:
            expectedFourcc = VA_FOURCC('4','0','0','P');
            break;
        case VA_RT_FORMAT_YUV420_10BPP:
            expectedFourcc = VA_FOURCC_P010;
            break;
        case VA_RT_FORMAT_RGB16:
            expectedFourcc = VA_FOURCC_R5G6B5;
            break;
        case VA_RT_FORMAT_RGB32:
            expectedFourcc = VA_FOURCC_BGRA;
            break;
        case VA_RT_FORMAT_RGBP:
            expectedFourcc = VA_FOURCC_RGBP;
            break;
#ifdef VA_RT_FORMAT_RGB32_10BPP
        case VA_RT_FORMAT_RGB32_10BPP:
            expectedFourcc = VA_FOURCC_BGRA;
            break;
#endif
#if 1 //added for having MDF sanity test pass, will be removed after MDF formal patch checked in
        case VA_FOURCC_NV12:
            expectedFourcc = VA_FOURCC_NV12;
            break;
        case VA_FOURCC_NV21:
            expectedFourcc = VA_FOURCC_NV21;
            break;
        case VA_FOURCC_ABGR:
            expectedFourcc = VA_FOURCC_ABGR;
            break;
        case VA_FOURCC_ARGB:
            expectedFourcc = VA_FOURCC_ARGB;
            break;
        case VA_FOURCC_XBGR:
            expectedFourcc = VA_FOURCC_XBGR;
            break;
        case VA_FOURCC_XRGB:
            expectedFourcc = VA_FOURCC_XRGB;
            break;
        case VA_FOURCC_R5G6B5:
            expectedFourcc = VA_FOURCC_R5G6B5;
            break;
        case VA_FOURCC_R8G8B8:
            expectedFourcc = VA_FOURCC_R8G8B8;
            break;
        case VA_FOURCC_YUY2:
            expectedFourcc = VA_FOURCC_YUY2;
            break;
        case VA_FOURCC_YV12:
            expectedFourcc = VA_FOURCC_YV12;
            break;
        case VA_FOURCC_422H:
            expectedFourcc = VA_FOURCC_422H;
            break;
        case VA_FOURCC_422V:
            expectedFourcc = VA_FOURCC_422V;
            break;
        case VA_FOURCC_P208:
            expectedFourcc = VA_FOURCC_P208;
            break;
        case VA_FOURCC_P010:
            expectedFourcc = VA_FOURCC_P010;
            break;
        case VA_FOURCC_P012:
            expectedFourcc = VA_FOURCC_P012;
            break;
        case VA_FOURCC_P016:
            expectedFourcc = VA_FOURCC_P016;
            break;
        case VA_FOURCC_Y210:
            expectedFourcc = VA_FOURCC_Y210;
            break;
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y212:
            expectedFourcc = VA_FOURCC_Y212;
            break;
#endif
        case VA_FOURCC_Y216:
            expectedFourcc = VA_FOURCC_Y216;
            break;
        case VA_FOURCC_AYUV:
            expectedFourcc = VA_FOURCC_AYUV;
            break;
#if VA_CHECK_VERSION(1, 13, 0)
        case VA_FOURCC_XYUV:
            expectedFourcc = VA_FOURCC_XYUV;
            break;
#endif
        case VA_FOURCC_Y410:
            expectedFourcc = VA_FOURCC_Y410;
            break;
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y412:
            expectedFourcc = VA_FOURCC_Y412;
            break;
#endif
        case VA_FOURCC_Y416:
            expectedFourcc = VA_FOURCC_Y416;
            break;
        case VA_FOURCC_I420:
            expectedFourcc = VA_FOURCC_I420;
            break;
        case VA_FOURCC_UYVY:
            expectedFourcc = VA_FOURCC_UYVY;
            break;
#endif
        default:
            DDI_ASSERTMESSAGE("Invalid VAConfigAttribRTFormat: 0x%x. Please uses the format defined in libva/va.h", format);
            return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    return VA_STATUS_SUCCESS;
}

DDI_MEDIA_FORMAT MediaLibvaInterfaceNext::OsFormatToMediaFormat(int32_t fourcc, int32_t rtformatType)
{
    switch (fourcc)
    {
        case VA_FOURCC_A2R10G10B10:
            return Media_Format_B10G10R10A2;
        case VA_FOURCC_A2B10G10R10:
            return Media_Format_R10G10B10A2;
        case VA_FOURCC_X2R10G10B10:
            return Media_Format_B10G10R10X2;
        case VA_FOURCC_X2B10G10R10:
            return Media_Format_R10G10B10X2;
        case VA_FOURCC_BGRA:
        case VA_FOURCC_ARGB:
#ifdef VA_RT_FORMAT_RGB32_10BPP
            if(VA_RT_FORMAT_RGB32_10BPP == rtformatType)
            {
                return Media_Format_B10G10R10A2;
            }
#endif
            return Media_Format_A8R8G8B8;
        case VA_FOURCC_RGBA:
#ifdef VA_RT_FORMAT_RGB32_10BPP
            if(VA_RT_FORMAT_RGB32_10BPP == rtformatType)
            {
                return Media_Format_R10G10B10A2;
            }
#endif
            return Media_Format_R8G8B8A8;
        case VA_FOURCC_ABGR:
#ifdef VA_RT_FORMAT_RGB32_10BPP
            if(VA_RT_FORMAT_RGB32_10BPP == rtformatType)
            {
                return Media_Format_R10G10B10A2;
            }
#endif
            return Media_Format_A8B8G8R8;
        case VA_FOURCC_BGRX:
        case VA_FOURCC_XRGB:
            return Media_Format_X8R8G8B8;
        case VA_FOURCC_XBGR:
        case VA_FOURCC_RGBX:
            return Media_Format_X8B8G8R8;
        case VA_FOURCC_R5G6B5:
            return Media_Format_R5G6B5;
        case VA_FOURCC_R8G8B8:
            return Media_Format_R8G8B8;
        case VA_FOURCC_NV12:
            return Media_Format_NV12;
        case VA_FOURCC_NV21:
            return Media_Format_NV21;
        case VA_FOURCC_YUY2:
            return Media_Format_YUY2;
        case VA_FOURCC_UYVY:
            return Media_Format_UYVY;
        case VA_FOURCC_YV12:
            return Media_Format_YV12;
        case VA_FOURCC_IYUV:
            return Media_Format_IYUV;
        case VA_FOURCC_I420:
            return Media_Format_I420;
        case VA_FOURCC_422H:
            return Media_Format_422H;
        case VA_FOURCC_422V:
            return Media_Format_422V;
        case VA_FOURCC('4','0','0','P'):
        case VA_FOURCC_Y800:
            return Media_Format_400P;
        case VA_FOURCC_411P:
            return Media_Format_411P;
        case VA_FOURCC_IMC3:
            return Media_Format_IMC3;
        case VA_FOURCC_444P:
            return Media_Format_444P;
        case VA_FOURCC_BGRP:
            return Media_Format_BGRP;
        case VA_FOURCC_RGBP:
            return Media_Format_RGBP;
        case VA_FOURCC_P208:
            return Media_Format_Buffer;
        case VA_FOURCC_P010:
            return Media_Format_P010;
        case VA_FOURCC_P012:
            return Media_Format_P012;
        case VA_FOURCC_P016:
            return Media_Format_P016;
        case VA_FOURCC_Y210:
            return Media_Format_Y210;
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y212:
            return Media_Format_Y212;
#endif
        case VA_FOURCC_Y216:
            return Media_Format_Y216;
        case VA_FOURCC_AYUV:
            return Media_Format_AYUV;
#if VA_CHECK_VERSION(1, 13, 0)
        case VA_FOURCC_XYUV:
            return Media_Format_XYUV;
#endif
        case VA_FOURCC_Y410:
            return Media_Format_Y410;
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y412:
            return Media_Format_Y412;
#endif
        case VA_FOURCC_Y416:
            return Media_Format_Y416;
        case VA_FOURCC_Y8:
            return Media_Format_Y8;
        case VA_FOURCC_Y16:
            return Media_Format_Y16S;
        case VA_FOURCC_VYUY:
            return Media_Format_VYUY;
        case VA_FOURCC_YVYU:
            return Media_Format_YVYU;
        case VA_FOURCC_ARGB64:
            return Media_Format_A16R16G16B16;
        case VA_FOURCC_ABGR64:
            return Media_Format_A16B16G16R16;

        default:
            return Media_Format_Count;
    }
}

uint32_t MediaLibvaInterfaceNext::GetPlaneNum(PDDI_MEDIA_SURFACE mediaSurface, bool hasAuxPlane)
{
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t fourcc = MediaFormatToOsFormat(mediaSurface->format);
    uint32_t planeNum = 0;
    switch(fourcc)
    {
        case VA_FOURCC_NV12:
        case VA_FOURCC_NV21:
        case VA_FOURCC_P010:
        case VA_FOURCC_P012:
        case VA_FOURCC_P016:
            planeNum = hasAuxPlane ? 4 : 2;
            break;
        case VA_FOURCC_I420:
        case VA_FOURCC_YV12:
        case VA_FOURCC_411P:
        case VA_FOURCC_422H:
        case VA_FOURCC_422V:
        case VA_FOURCC_444P:
        case VA_FOURCC_IMC3:
        case VA_FOURCC_RGBP:
        case VA_FOURCC_BGRP:
            planeNum = 3;
            break;
        case VA_FOURCC_YUY2:
        case VA_FOURCC_UYVY:
        case VA_FOURCC_YVYU:
        case VA_FOURCC_VYUY:
        case VA_FOURCC_Y800:
        case VA_FOURCC_Y210:
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y212:
#endif
        case VA_FOURCC_Y216:
        case VA_FOURCC_Y410:
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y412:
#endif
        case VA_FOURCC_Y416:
        case VA_FOURCC_AYUV:
#if VA_CHECK_VERSION(1, 13, 0)
        case VA_FOURCC_XYUV:
#endif
        case VA_FOURCC_RGBA:
        case VA_FOURCC_RGBX:
        case VA_FOURCC_BGRA:
        case VA_FOURCC_BGRX:
        case VA_FOURCC_ARGB:
        case VA_FOURCC_ABGR:
        case VA_FOURCC_XRGB:
        case VA_FOURCC_XBGR:
        case VA_FOURCC_RGB565:
        case VA_FOURCC_R8G8B8:
        case VA_FOURCC_A2R10G10B10:
        case VA_FOURCC_A2B10G10R10:
        case VA_FOURCC_X2R10G10B10:
        case VA_FOURCC_X2B10G10R10:
            planeNum = hasAuxPlane ? 2 : 1;
            break;
        default:
            DDI_ASSERTMESSAGE("Unsupported format.\n");
    }
    return planeNum;
}

uint32_t MediaLibvaInterfaceNext::GetDrmFormatOfSeparatePlane(uint32_t fourcc, int plane)
{
    if (plane == 0)
    {
        switch (fourcc)
        {
        case VA_FOURCC_NV12:
        case VA_FOURCC_I420:
        case VA_FOURCC_IMC3:
        case VA_FOURCC_YV12:
        case VA_FOURCC_YV16:
        case VA_FOURCC_422H:
        case VA_FOURCC_422V:
        case VA_FOURCC_444P:
        case VA_FOURCC_Y800:
        case VA_FOURCC_RGBP:
        case VA_FOURCC_BGRP:
            return DRM_FORMAT_R8;
        case VA_FOURCC_P010:
        case VA_FOURCC_P012:
        case VA_FOURCC_P016:
        case VA_FOURCC_I010:
            return DRM_FORMAT_R16;

        case VA_FOURCC_YUY2:
            return DRM_FORMAT_YUYV;
        case VA_FOURCC_YVYU:
            return DRM_FORMAT_YVYU;
        case VA_FOURCC_VYUY:
            return DRM_FORMAT_VYUY;
        case VA_FOURCC_UYVY:
            return DRM_FORMAT_UYVY;
        case VA_FOURCC_AYUV:
            return DRM_FORMAT_AYUV;
#if VA_CHECK_VERSION(1, 13, 0)
        case VA_FOURCC_XYUV:
            return DRM_FORMAT_XYUV8888;
#endif
        case VA_FOURCC_Y210:
            return DRM_FORMAT_Y210;
        case VA_FOURCC_Y216:
            return DRM_FORMAT_Y216;
        case VA_FOURCC_Y410:
            return DRM_FORMAT_Y410;
        case VA_FOURCC_Y416:
            return DRM_FORMAT_Y416;
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y212:
            return DRM_FORMAT_Y216;
        case VA_FOURCC_Y412:
            return DRM_FORMAT_Y416;
#endif

        case VA_FOURCC_ARGB:
            return DRM_FORMAT_ARGB8888;
        case VA_FOURCC_ABGR:
            return DRM_FORMAT_ABGR8888;
        case VA_FOURCC_RGBA:
            return DRM_FORMAT_RGBA8888;
        case VA_FOURCC_BGRA:
            return DRM_FORMAT_BGRA8888;
        case VA_FOURCC_XRGB:
            return DRM_FORMAT_XRGB8888;
        case VA_FOURCC_XBGR:
            return DRM_FORMAT_XBGR8888;
        case VA_FOURCC_RGBX:
            return DRM_FORMAT_RGBX8888;
        case VA_FOURCC_BGRX:
            return DRM_FORMAT_BGRX8888;
        case VA_FOURCC_A2R10G10B10:
            return DRM_FORMAT_ARGB2101010;
        case VA_FOURCC_A2B10G10R10:
            return DRM_FORMAT_ABGR2101010;
        case VA_FOURCC_X2R10G10B10:
            return DRM_FORMAT_XRGB2101010;
        case VA_FOURCC_X2B10G10R10:
            return DRM_FORMAT_XBGR2101010;
        }
    }
    else
    {
        switch (fourcc)
        {
        case VA_FOURCC_NV12:
            return DRM_FORMAT_GR88;
        case VA_FOURCC_I420:
        case VA_FOURCC_IMC3:
        case VA_FOURCC_YV12:
        case VA_FOURCC_YV16:
        case VA_FOURCC_422H:
        case VA_FOURCC_422V:
        case VA_FOURCC_444P:
        case VA_FOURCC_RGBP:
        case VA_FOURCC_BGRP:
            return DRM_FORMAT_R8;
        case VA_FOURCC_P010:
        case VA_FOURCC_P012:
        case VA_FOURCC_P016:
            return DRM_FORMAT_GR1616;
        case VA_FOURCC_I010:
            return DRM_FORMAT_R16;
        }
    }
    return 0;
}

uint32_t MediaLibvaInterfaceNext::GetDrmFormatOfCompositeObject(uint32_t fourcc)
{
    switch (fourcc)
    {
    case VA_FOURCC_NV12:
        return DRM_FORMAT_NV12;
    case VA_FOURCC_I420:
        return DRM_FORMAT_YUV420;
    case VA_FOURCC_IMC3:
        return DRM_FORMAT_YUV420;
    case VA_FOURCC_YV12:
        return DRM_FORMAT_YVU420;
    case VA_FOURCC_YV16:
        return DRM_FORMAT_YVU422;
    case VA_FOURCC_422H:
        return DRM_FORMAT_YUV422;
    case VA_FOURCC_422V:
        return DRM_FORMAT_YUV422;
    case VA_FOURCC_444P:
        return DRM_FORMAT_YUV444;
    case VA_FOURCC_YUY2:
        return DRM_FORMAT_YUYV;
    case VA_FOURCC_YVYU:
        return DRM_FORMAT_YVYU;
    case VA_FOURCC_VYUY:
        return DRM_FORMAT_VYUY;
    case VA_FOURCC_UYVY:
        return DRM_FORMAT_UYVY;
    case VA_FOURCC_AYUV:
        return DRM_FORMAT_AYUV;
#if VA_CHECK_VERSION(1, 13, 0)
    case VA_FOURCC_XYUV:
        return DRM_FORMAT_XYUV8888;
#endif
    case VA_FOURCC_Y210:
        return DRM_FORMAT_Y210;
#if VA_CHECK_VERSION(1, 9, 0)
    case VA_FOURCC_Y212:
        return DRM_FORMAT_Y216;
#endif
    case VA_FOURCC_Y216:
        return DRM_FORMAT_Y216;
    case VA_FOURCC_Y410:
        return DRM_FORMAT_Y410;
#if VA_CHECK_VERSION(1, 9, 0)
    case VA_FOURCC_Y412:
        return DRM_FORMAT_Y416;
#endif
    case VA_FOURCC_Y416:
        return DRM_FORMAT_Y416;
    case VA_FOURCC_Y800:
        return DRM_FORMAT_R8;
    case VA_FOURCC_P010:
        return DRM_FORMAT_P010;
    case VA_FOURCC_P012:
        return DRM_FORMAT_P016;
    case VA_FOURCC_P016:
        return DRM_FORMAT_P016;
    case VA_FOURCC_ARGB:
        return DRM_FORMAT_ARGB8888;
    case VA_FOURCC_ABGR:
        return DRM_FORMAT_ABGR8888;
    case VA_FOURCC_RGBA:
        return DRM_FORMAT_RGBA8888;
    case VA_FOURCC_BGRA:
        return DRM_FORMAT_BGRA8888;
    case VA_FOURCC_XRGB:
        return DRM_FORMAT_XRGB8888;
    case VA_FOURCC_XBGR:
        return DRM_FORMAT_XBGR8888;
    case VA_FOURCC_RGBX:
        return DRM_FORMAT_RGBX8888;
    case VA_FOURCC_BGRX:
        return DRM_FORMAT_BGRX8888;
    case VA_FOURCC_A2R10G10B10:
        return DRM_FORMAT_ARGB2101010;
    case VA_FOURCC_A2B10G10R10:
        return DRM_FORMAT_ABGR2101010;
    case VA_FOURCC_X2R10G10B10:
        return DRM_FORMAT_XRGB2101010;
    case VA_FOURCC_X2B10G10R10:
        return DRM_FORMAT_XBGR2101010;
    }
    return 0;
}

int32_t MediaLibvaInterfaceNext::MediaFormatToOsFormat(DDI_MEDIA_FORMAT format)
{
    switch (format)
    {
        case Media_Format_X8R8G8B8:
            return VA_FOURCC_XRGB;
        case Media_Format_X8B8G8R8:
            return VA_FOURCC_XBGR;
        case Media_Format_A8B8G8R8:
            return VA_FOURCC_ABGR;
        case Media_Format_R10G10B10A2:
            return VA_FOURCC_A2B10G10R10;
        case Media_Format_R8G8B8A8:
            return VA_FOURCC_RGBA;
        case Media_Format_A8R8G8B8:
            return VA_FOURCC_ARGB;
        case Media_Format_B10G10R10A2:
            return VA_FOURCC_A2R10G10B10;
        case Media_Format_R10G10B10X2:
            return VA_FOURCC_X2B10G10R10;
        case Media_Format_B10G10R10X2:
            return VA_FOURCC_X2R10G10B10;
        case Media_Format_R5G6B5:
            return VA_FOURCC_R5G6B5;
        case Media_Format_R8G8B8:
            return VA_FOURCC_R8G8B8;
        case Media_Format_NV12:
            return VA_FOURCC_NV12;
        case Media_Format_NV21:
            return VA_FOURCC_NV21;
        case  Media_Format_YUY2:
            return VA_FOURCC_YUY2;
        case  Media_Format_UYVY:
            return VA_FOURCC_UYVY;
        case Media_Format_YV12:
            return VA_FOURCC_YV12;
        case Media_Format_IYUV:
            return VA_FOURCC_IYUV;
        case Media_Format_I420:
            return VA_FOURCC_I420;
        case Media_Format_400P:
            return VA_FOURCC('4','0','0','P');
        case Media_Format_IMC3:
            return VA_FOURCC_IMC3;
        case Media_Format_422H:
            return VA_FOURCC_422H;
        case Media_Format_422V:
            return VA_FOURCC_422V;
        case Media_Format_411P:
            return VA_FOURCC_411P;
        case Media_Format_444P:
            return VA_FOURCC_444P;
        case Media_Format_RGBP:
            return VA_FOURCC_RGBP;
        case Media_Format_BGRP:
            return VA_FOURCC_BGRP;
        case Media_Format_Buffer:
            return VA_FOURCC_P208;
        case Media_Format_P010:
            return VA_FOURCC_P010;
        case Media_Format_P012:
            return VA_FOURCC_P012;
        case Media_Format_P016:
            return VA_FOURCC_P016;
        case Media_Format_Y210:
            return VA_FOURCC_Y210;
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y212:
            return VA_FOURCC_Y212;
#endif
        case Media_Format_Y216:
            return VA_FOURCC_Y216;
        case Media_Format_AYUV:
            return VA_FOURCC_AYUV;
#if VA_CHECK_VERSION(1, 13, 0)
        case Media_Format_XYUV:
            return VA_FOURCC_XYUV;
#endif
        case Media_Format_Y410:
            return VA_FOURCC_Y410;
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y412:
            return VA_FOURCC_Y412;
#endif
        case Media_Format_Y416:
            return VA_FOURCC_Y416;
        case Media_Format_Y8:
            return VA_FOURCC_Y8;
        case Media_Format_Y16S:
            return VA_FOURCC_Y16;
        case Media_Format_Y16U:
            return VA_FOURCC_Y16;
        case Media_Format_VYUY:
            return VA_FOURCC_VYUY;
        case Media_Format_YVYU:
            return VA_FOURCC_YVYU;
        case Media_Format_A16R16G16B16:
            return VA_FOURCC_ARGB64;
        case Media_Format_A16B16G16R16:
            return VA_FOURCC_ABGR64;
        default:
            return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }
}

VAStatus MediaLibvaInterfaceNext::GetChromaPitchHeight(
    uint32_t fourcc,
    uint32_t pitch,
    uint32_t height,
    uint32_t *chromaPitch,
    uint32_t *chromaHeight)
{
    DDI_CHK_NULL(chromaPitch, "nullptr chromaPitch", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chromaHeight, "nullptr chromaHeight", VA_STATUS_ERROR_INVALID_PARAMETER);

    switch(fourcc)
    {
        case VA_FOURCC_NV12:
        case VA_FOURCC_P010:
        case VA_FOURCC_P012:
        case VA_FOURCC_P016:
            *chromaHeight = MOS_ALIGN_CEIL(height, 2) / 2;
            *chromaPitch = pitch;
            break;
        case VA_FOURCC_I420:
        case VA_FOURCC_YV12:
            *chromaHeight = MOS_ALIGN_CEIL(height, 2) / 2;
            *chromaPitch = MOS_ALIGN_CEIL(pitch, 2) / 2;
            break;
        case VA_FOURCC_411P:
        case VA_FOURCC_422H:
        case VA_FOURCC_444P:
        case VA_FOURCC_RGBP:
            *chromaHeight = height;
            *chromaPitch = pitch;
            break;
        case VA_FOURCC_422V:
        case VA_FOURCC_IMC3:
            *chromaHeight = MOS_ALIGN_CEIL(height, 2) / 2;
            *chromaPitch = pitch;
            break;
        default:
            *chromaPitch = 0;
            *chromaHeight = 0;
    }

    return VA_STATUS_SUCCESS;
}

uint32_t MediaLibvaInterfaceNext::CreateRenderTarget(
    PDDI_MEDIA_CONTEXT            mediaDrvCtx,
    DDI_MEDIA_FORMAT              mediaFormat,
    uint32_t                      width,
    uint32_t                      height,
    DDI_MEDIA_SURFACE_DESCRIPTOR  *surfDesc,
    uint32_t                      surfaceUsageHint,
    int                           memType
)
{
    DDI_FUNC_ENTER;
    MosUtilities::MosLockMutex(&mediaDrvCtx->SurfaceMutex);

    PDDI_MEDIA_SURFACE_HEAP_ELEMENT surfaceElement = MediaLibvaUtilNext::AllocPMediaSurfaceFromHeap(mediaDrvCtx->pSurfaceHeap);
    if (nullptr == surfaceElement)
    {
        MosUtilities::MosUnlockMutex(&mediaDrvCtx->SurfaceMutex);
        return VA_INVALID_ID;
    }

    surfaceElement->pSurface = (DDI_MEDIA_SURFACE *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE));
    if (nullptr == surfaceElement->pSurface)
    {
        MediaLibvaUtilNext::ReleasePMediaSurfaceFromHeap(mediaDrvCtx->pSurfaceHeap, surfaceElement->uiVaSurfaceID);
        MosUtilities::MosUnlockMutex(&mediaDrvCtx->SurfaceMutex);
        return VA_INVALID_ID;
    }

    surfaceElement->pSurface->pMediaCtx       = mediaDrvCtx;
    surfaceElement->pSurface->iWidth          = width;
    surfaceElement->pSurface->iHeight         = height;
    surfaceElement->pSurface->pSurfDesc       = surfDesc;
    surfaceElement->pSurface->format          = mediaFormat;
    surfaceElement->pSurface->uiLockedBufID   = VA_INVALID_ID;
    surfaceElement->pSurface->uiLockedImageID = VA_INVALID_ID;
    surfaceElement->pSurface->surfaceUsageHint= surfaceUsageHint;
    surfaceElement->pSurface->memType         = memType;

    if(MediaLibvaUtilNext::CreateSurface(surfaceElement->pSurface, mediaDrvCtx)!= VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(surfaceElement->pSurface);
        MediaLibvaUtilNext::ReleasePMediaSurfaceFromHeap(mediaDrvCtx->pSurfaceHeap, surfaceElement->uiVaSurfaceID);
        MosUtilities::MosUnlockMutex(&mediaDrvCtx->SurfaceMutex);
        return VA_INVALID_ID;
    }

    mediaDrvCtx->uiNumSurfaces++;
    uint32_t surfaceID = surfaceElement->uiVaSurfaceID;
    MosUtilities::MosUnlockMutex(&mediaDrvCtx->SurfaceMutex);
    return surfaceID;
}

VAStatus MediaLibvaInterfaceNext::CopyInternal(
    PMOS_CONTEXT    mosCtx,
    PMOS_RESOURCE   src,
    PMOS_RESOURCE   dst,
    uint32_t        copy_mode
)
{
    VAStatus   vaStatus  = VA_STATUS_SUCCESS;
    MOS_STATUS mosStatus = MOS_STATUS_UNINITIALIZED;
    DDI_CHK_NULL(mosCtx, "nullptr mosCtx",            VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(src,    "nullptr input osResource",  VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(dst,    "nullptr output osResource", VA_STATUS_ERROR_INVALID_SURFACE);

    MediaCopyBaseState *mediaCopyState = static_cast<MediaCopyBaseState*>(*mosCtx->ppMediaCopyState);

    if (!mediaCopyState)
    {
        mediaCopyState = static_cast<MediaCopyBaseState*>(McpyDeviceNext::CreateFactory(mosCtx));
        *mosCtx->ppMediaCopyState = mediaCopyState;
    }

    DDI_CHK_NULL(mediaCopyState, "Invalid mediaCopy State", VA_STATUS_ERROR_INVALID_PARAMETER);
    
#if (_DEBUG || _RELEASE_INTERNAL)
    // enable reg key report to avoid conflict with media copy cases.
    mediaCopyState->SetRegkeyReport(true);
#endif

    mosStatus = mediaCopyState->SurfaceCopy(src, dst, (MCPY_METHOD)copy_mode);
    if (mosStatus != MOS_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    return vaStatus;
}

#if VA_CHECK_VERSION(1,10,0)
VAStatus MediaLibvaInterfaceNext::Copy(
    VADriverContextP    ctx,
    VACopyObject       *dst_obj,
    VACopyObject       *src_obj,
    VACopyOption       option
)
{
    VAStatus           vaStatus = VA_STATUS_SUCCESS;
    MOS_CONTEXT        mosCtx   = {};
    MOS_RESOURCE       src, dst;
    PDDI_MEDIA_SURFACE src_surface = nullptr;
    PDDI_MEDIA_SURFACE dst_surface = nullptr;
    PDDI_MEDIA_BUFFER  src_buffer = nullptr;
    PDDI_MEDIA_BUFFER  dst_buffer = nullptr;

    DDI_FUNC_ENTER;

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);

    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pBufferHeap,  "nullptr mediaCtx->pBufferHeap",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(dst_obj, "nullptr copy dst", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(src_obj, "nullptr copy src", VA_STATUS_ERROR_INVALID_SURFACE);

    if (dst_obj->obj_type == VACopyObjectSurface)
    {
        DDI_CHK_LESS((uint32_t)dst_obj->object.surface_id, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "copy_dst", VA_STATUS_ERROR_INVALID_SURFACE);
        dst_surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, dst_obj->object.surface_id);
        DDI_CHK_NULL(dst_surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
        DDI_CHK_NULL(dst_surface->pGmmResourceInfo, "nullptr dst_surface->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_PARAMETER);

        MOS_ZeroMemory(&dst, sizeof(dst));
        MediaLibvaCommonNext::MediaSurfaceToMosResource(dst_surface, &dst);
    }
    else if (dst_obj->obj_type == VACopyObjectBuffer)
    {
        DDI_CHK_LESS((uint32_t)dst_obj->object.buffer_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid copy dst buf_id", VA_STATUS_ERROR_INVALID_BUFFER);
        dst_buffer = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, dst_obj->object.buffer_id);
        DDI_CHK_NULL(dst_buffer, "nullptr buffer", VA_STATUS_ERROR_INVALID_BUFFER);
        DDI_CHK_NULL(dst_buffer->pGmmResourceInfo, "nullptr dst_buffer->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_PARAMETER);

        MOS_ZeroMemory(&dst, sizeof(dst));
        MediaLibvaCommonNext::MediaBufferToMosResource(dst_buffer, &dst);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: unsupported src copy object in copy.");
    }

    if (src_obj->obj_type == VACopyObjectSurface)
    {
        DDI_CHK_LESS((uint32_t)src_obj->object.surface_id, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "copy_src", VA_STATUS_ERROR_INVALID_SURFACE);
        src_surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, src_obj->object.surface_id);
        DDI_CHK_NULL(src_surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
        DDI_CHK_NULL(src_surface->pGmmResourceInfo, "nullptr src_surface->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_PARAMETER);

        MOS_ZeroMemory(&src, sizeof(src));
        MediaLibvaCommonNext::MediaSurfaceToMosResource(src_surface, &src);
    }
    else if (src_obj->obj_type == VACopyObjectBuffer)
    {
        DDI_CHK_LESS((uint32_t)src_obj->object.buffer_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid copy dst buf_id", VA_STATUS_ERROR_INVALID_BUFFER);
        src_buffer = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, src_obj->object.buffer_id);
        DDI_CHK_NULL(src_buffer, "nullptr buffer", VA_STATUS_ERROR_INVALID_BUFFER);
        DDI_CHK_NULL(src_buffer->pGmmResourceInfo, "nullptr src_buffer->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_PARAMETER);

        MOS_ZeroMemory(&src, sizeof(src));
        MediaLibvaCommonNext::MediaBufferToMosResource(src_buffer, &src);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: unsupported dst copy object in copy.");
    }

    mosCtx.bufmgr          = mediaCtx->pDrmBufMgr;
    mosCtx.fd              = mediaCtx->fd;
    mosCtx.iDeviceId       = mediaCtx->iDeviceId;
    mosCtx.m_skuTable      = mediaCtx->SkuTable;
    mosCtx.m_waTable       = mediaCtx->WaTable;
    mosCtx.m_gtSystemInfo  = *mediaCtx->pGtSystemInfo;
    mosCtx.m_platform      = mediaCtx->platform;

    mosCtx.ppMediaCopyState      = &mediaCtx->pMediaCopyState;
    mosCtx.m_gtSystemInfo        = *mediaCtx->pGtSystemInfo;
    mosCtx.m_auxTableMgr         = mediaCtx->m_auxTableMgr;
    mosCtx.pGmmClientContext     = mediaCtx->pGmmClientContext;

    mosCtx.m_osDeviceContext     = mediaCtx->m_osDeviceContext;
    mosCtx.m_apoMosEnabled       = true;
    mosCtx.pPerfData             = mediaCtx->perfData;
    mosCtx.m_userSettingPtr      = mediaCtx->m_userSettingPtr;

    vaStatus = CopyInternal(&mosCtx, &src, &dst, option.bits.va_copy_mode);

    if ((option.bits.va_copy_sync == VA_EXEC_SYNC) && dst_surface)
    {
        uint32_t timeout_NS = 100000000;
        while (0 != mos_bo_wait(dst_surface->bo, timeout_NS))
        {
            // Just loop while gem_bo_wait times-out.
        }
    }

    return vaStatus;
}
#endif

VAStatus MediaLibvaInterfaceNext::MapBuffer(
    VADriverContextP    ctx,
    VABufferID          buf_id,
    void                **pbuf)
{
    DDI_FUNC_ENTER;
    return MapBufferInternal(ctx, buf_id, pbuf, MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY);
}

VAStatus MediaLibvaInterfaceNext::MapBufferInternal(
    VADriverContextP ctx,
    VABufferID       bufId,
    void             **buf,
    uint32_t         flag)
{
    VAStatus                 vaStatus  = VA_STATUS_SUCCESS;
    DDI_MEDIA_BUFFER         *mediaBuf = nullptr;
    PDDI_MEDIA_CONTEXT       mediaCtx  = nullptr;
    uint32_t                 ctxType   = DDI_MEDIA_CONTEXT_TYPE_NONE;

    DDI_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_START, &bufId, sizeof(bufId), &flag, sizeof(flag));

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)bufId, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufferId", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaBuf     = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, bufId);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, bufId);
    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex], "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);

    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_INFO, &ctxType, sizeof(ctxType), &mediaBuf->uiType, sizeof(uint32_t));
    vaStatus = mediaCtx->m_compList[componentIndex]->MapBufferInternal(mediaCtx, bufId, buf, flag);

    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::UnmapBuffer(
    VADriverContextP ctx,
    VABufferID       bufId)
{
    VAStatus                 vaStatus = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT       mediaCtx = nullptr;
    DDI_MEDIA_BUFFER         *buf     = nullptr;
    uint32_t                 ctxType  = DDI_MEDIA_CONTEXT_TYPE_NONE;

    DDI_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_UNMAP, EVENT_TYPE_START, &bufId, sizeof(bufId), nullptr, 0);

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr  mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)bufId, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufId", VA_STATUS_ERROR_INVALID_BUFFER);

    buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, bufId);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, bufId);
    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex], "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);

    vaStatus = mediaCtx->m_compList[componentIndex]->UnmapBuffer(mediaCtx, bufId);

    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::MediaMemoryDecompress(
    PDDI_MEDIA_CONTEXT mediaCtx,
    DDI_MEDIA_SURFACE  *mediaSurface)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaCtx, "Null mediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mediaSurface->pGmmResourceInfo, "nullptr mediaSurface->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    GMM_RESOURCE_FLAG GmmFlags;

    MOS_ZeroMemory(&GmmFlags, sizeof(GmmFlags));
    GmmFlags = mediaSurface->pGmmResourceInfo->GetResFlags();

    if (((GmmFlags.Gpu.MMC                                                        ||
          GmmFlags.Gpu.CCS)                                                       &&
          GmmFlags.Info.MediaCompressed)                                          ||
          mediaSurface->pGmmResourceInfo->IsMediaMemoryCompressed(0))
    {
#ifdef _MMC_SUPPORTED
        MOS_CONTEXT  mosCtx = {};
        MOS_RESOURCE surface;

        MOS_ZeroMemory(&surface, sizeof(surface));

        mosCtx.bufmgr          = mediaCtx->pDrmBufMgr;
        mosCtx.fd              = mediaCtx->fd;
        mosCtx.iDeviceId       = mediaCtx->iDeviceId;
        mosCtx.m_skuTable      = mediaCtx->SkuTable;
        mosCtx.m_waTable       = mediaCtx->WaTable;
        mosCtx.m_gtSystemInfo  = *mediaCtx->pGtSystemInfo;
        mosCtx.m_platform      = mediaCtx->platform;

        mosCtx.ppMediaMemDecompState = &mediaCtx->pMediaMemDecompState;
        mosCtx.pfnMemoryDecompress   = mediaCtx->pfnMemoryDecompress;
        mosCtx.pfnMediaMemoryCopy    = mediaCtx->pfnMediaMemoryCopy;
        mosCtx.pfnMediaMemoryCopy2D  = mediaCtx->pfnMediaMemoryCopy2D;
        mosCtx.m_gtSystemInfo        = *mediaCtx->pGtSystemInfo;
        mosCtx.m_auxTableMgr         = mediaCtx->m_auxTableMgr;
        mosCtx.pGmmClientContext     = mediaCtx->pGmmClientContext;

        mosCtx.m_osDeviceContext     = mediaCtx->m_osDeviceContext;
        mosCtx.m_apoMosEnabled       = true;

        mosCtx.m_userSettingPtr      = mediaCtx->m_userSettingPtr;

        MosUtilities::MosLockMutex(&mediaCtx->SurfaceMutex);
        MosUtilities::MosLockMutex(&mediaCtx->MemDecompMutex);

        MediaLibvaCommonNext::MediaSurfaceToMosResource(mediaSurface, &surface);
        MediaLibvaInterfaceNext::MediaMemoryDecompressInternal(&mosCtx, &surface);

        MosUtilities::MosUnlockMutex(&mediaCtx->MemDecompMutex);
        MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
#else
        vaStatus = VA_STATUS_ERROR_INVALID_SURFACE;
        DDI_ASSERTMESSAGE("MMC unsupported! [%d].", vaStatus);
#endif
    }

    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::QueryProcessingRate(
    VADriverContextP           ctx,
    VAConfigID                 configId,
    VAProcessingRateParameter  *procBuf,
    uint32_t                   *processingRate)
{
    DDI_FUNC_ENTER;
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}
