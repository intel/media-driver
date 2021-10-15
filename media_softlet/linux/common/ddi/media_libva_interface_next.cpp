/*
* Copyright (c) 2021, Intel Corporation
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

#include "media_libva_interface_next.h"
#include "media_libva_util.h"
#include "media_libva.h"
#include "mos_utilities.h"
#include "media_interfaces_mmd.h"
#include "media_libva_caps_next.h"
#include "media_ddi_prot.h"
#include "media_interfaces_hwinfo_device.h"
#include "mos_oca_interface_specific.h"

#include "media_libva_caps.h"
#include "ddi_cp_functions.h"
#include "ddi_decode_functions.h"
#include "ddi_encode_functions.h"
#include "ddi_vp_functions.h"

MEDIA_MUTEX_T MediaLibvaInterfaceNext::m_GlobalMutex = MEDIA_MUTEX_INITIALIZER;

void MediaLibvaInterfaceNext::FreeForMediaContext(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNCTION_ENTER();

    DdiMediaUtil_UnLockMutex(&m_GlobalMutex);

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
        MOS_FreeMemory(mediaCtx);
    }

    return;
}

void MediaLibvaInterfaceNext::DestroyMediaContextMutex(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNCTION_ENTER();

    // destroy the mutexs
    DdiMediaUtil_DestroyMutex(&mediaCtx->SurfaceMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->BufferMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->ImageMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->DecoderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->EncoderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->VpMutex);

#if !defined(ANDROID) && defined(X11_FOUND)
    DdiMediaUtil_DestroyMutex(&mediaCtx->PutSurfaceRenderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->PutSurfaceSwapBufferMutex);
#endif

    return;
}

PDDI_MEDIA_CONTEXT MediaLibvaInterfaceNext::CreateMediaDriverContext()
{
    PDDI_MEDIA_CONTEXT   mediaCtx;

    mediaCtx = (PDDI_MEDIA_CONTEXT)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_CONTEXT));

    return mediaCtx;
}

VAStatus MediaLibvaInterfaceNext::HeapInitialize(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNCTION_ENTER();

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
    DdiMediaUtil_InitMutex(&mediaCtx->SurfaceMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->BufferMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->ImageMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->DecoderMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->EncoderMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->VpMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->ProtMutex);

    return VA_STATUS_SUCCESS;
}

#ifdef _MMC_SUPPORTED
void MediaLibvaInterfaceNext::MediaMemoryDecompressInternal(
    PMOS_CONTEXT  mosCtx,
    PMOS_RESOURCE osResource)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(mosCtx, "nullptr mosCtx",);
    DDI_CHK_NULL(osResource, "nullptr osResource",);

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    if (!mediaMemDecompState)
    {
        DDI_CHK_NULL(mediaMemDecompState, "nullptr mediaMemDecompState", );
    }

    if (!mediaMemDecompState)
    {
        mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(MmdDevice::CreateFactory(mosCtx));
        *mosCtx->ppMediaMemDecompState = mediaMemDecompState;
    }

    if (mediaMemDecompState)
    {
        mediaMemDecompState->MemoryDecompress(osResource);
    }
    else
    {
        DDI_ASSERTMESSAGE("Invalid memory decompression state.");
    }
}

void MediaLibvaInterfaceNext::MediaMemoryCopyInternal(
    PMOS_CONTEXT  mosCtx,
    PMOS_RESOURCE inputOsResource,
    PMOS_RESOURCE outputOsResource,
    bool          boutputcompressed)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(mosCtx, "nullptr mosCtx",);
    DDI_CHK_NULL(inputOsResource, "nullptr input osResource",);
    DDI_CHK_NULL(outputOsResource, "nullptr output osResource",);

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    if (!mediaMemDecompState)
    {
        DDI_CHK_NULL(mediaMemDecompState, "nullptr mediaMemDecompState", );
    }

    if (!mediaMemDecompState)
    {
        mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(MmdDevice::CreateFactory(mosCtx));
        *mosCtx->ppMediaMemDecompState = mediaMemDecompState;
    }

    if (mediaMemDecompState)
    {
        mediaMemDecompState->MediaMemoryCopy(inputOsResource, outputOsResource, boutputcompressed);
    }
    else
    {
        DDI_ASSERTMESSAGE("Invalid memory decompression state.");
    }
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
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(mosCtx, "nullptr mosCtx",);
    DDI_CHK_NULL(inputOsResource, "nullptr input osResource",);
    DDI_CHK_NULL(outputOsResource, "nullptr output osResource",);

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    if (!mediaMemDecompState)
    {
        DDI_CHK_NULL(mediaMemDecompState, "nullptr mediaMemDecompState", );
    }

    if (!mediaMemDecompState)
    {
        mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(MmdDevice::CreateFactory(mosCtx));
        *mosCtx->ppMediaMemDecompState = mediaMemDecompState;
    }

    if (mediaMemDecompState)
    {
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
    else
    {
        DDI_ASSERTMESSAGE("Invalid memory decompression state.");
    }
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
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(mosCtx, "nullptr mosCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(inputOsResource, "nullptr input osResource", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(outputOsResource, "nullptr output osResource", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = VA_STATUS_SUCCESS;

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    if (!mediaMemDecompState)
    {
        DDI_CHK_NULL(mediaMemDecompState, "nullptr mediaMemDecompState", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    if (!mediaMemDecompState)
    {
        mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(MmdDevice::CreateFactory(mosCtx));
        *mosCtx->ppMediaMemDecompState = mediaMemDecompState;
    }

    DDI_CHK_NULL(mediaMemDecompState, "Invalid memory decompression state", VA_STATUS_ERROR_INVALID_PARAMETER);

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
    DDI_FUNCTION_ENTER();

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

    DdiMediaUtil_LockMutex(&m_GlobalMutex);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    if(mediaCtx)
    {
        mediaCtx->uiRef++;
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_SUCCESS;
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

    MOS_CONTEXT mosCtx     = {};
    mosCtx.fd              = mediaCtx->fd;

    MosInterface::InitOsUtilities(&mosCtx);
    if (mediaCtx->m_apoMosEnabled)
    {
        MosOcaInterfaceSpecific::InitInterface();
    }

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
    mediaCtx->SkuTable                  = mosCtx.SkuTable;
    mediaCtx->WaTable                   = mosCtx.WaTable;
    *mediaCtx->pGtSystemInfo            = mosCtx.gtSystemInfo;
    mediaCtx->platform                  = mosCtx.platform;
    mediaCtx->m_auxTableMgr             = mosCtx.m_auxTableMgr;
    mediaCtx->pGmmClientContext         = mosCtx.pGmmClientContext;
    mediaCtx->m_useSwSwizzling          = mosCtx.bUseSwSwizzling;
    mediaCtx->m_tileYFlag               = mosCtx.bTileYFlag;
    mediaCtx->bIsAtomSOC                = mosCtx.bIsAtomSOC;
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
            MOS_FreeMemory(mediaCtx->m_hwInfo);
        }
        mediaCtx->m_hwInfo = nullptr;
        DestroyMediaContextMutex(mediaCtx);
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    ctx->max_image_formats = mediaCtx->m_capsNext->GetImageFormatsMaxNum();

#if !defined(ANDROID) && defined(X11_FOUND)
    DdiMediaUtil_InitMutex(&mediaCtx->PutSurfaceRenderMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->PutSurfaceSwapBufferMutex);

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

    DdiMediaUtil_SetMediaResetEnableFlag(mediaCtx);

    if (InitCompList(mediaCtx) != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Caps init failed. Not supported GFX device.");
        MediaLibvaInterfaceNext::ReleaseCompList(mediaCtx);
        if(mediaCtx->m_hwInfo)
        {
            MOS_FreeMemory(mediaCtx->m_hwInfo);
        }
        mediaCtx->m_hwInfo = nullptr;
        MOS_Delete(mediaCtx->m_capsNext);
        mediaCtx->m_capsNext = nullptr;
        DestroyMediaContextMutex(mediaCtx);
        ReleaseCompList(mediaCtx);
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    DdiMediaUtil_UnLockMutex(&m_GlobalMutex);

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus MediaLibvaInterfaceNext::InitCompList(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DDI_FUNCTION_ENTER();

    VAStatus status = VA_STATUS_SUCCESS;
    mediaCtx->m_compList[CompCommon] = (DdiMediaFunctions*)MOS_AllocAndZeroMemory(sizeof(DdiMediaFunctions));

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
    DDI_FUNCTION_ENTER();

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
    DDI_FUNCTION_ENTER();
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
        DdiMediaUtil_FreeSurface(mediaSurfaceHeapElmt->pSurface);
        MOS_FreeMemory(mediaSurfaceHeapElmt->pSurface);
        DdiMediaUtil_ReleasePMediaSurfaceFromHeap(surfaceHeap,mediaSurfaceHeapElmt->uiVaSurfaceID);
        mediaCtx->uiNumSurfaces--;
    }
}

void MediaLibvaInterfaceNext::FreeBufferHeapElements(VADriverContextP    ctx)
{
    DDI_FUNCTION_ENTER();

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
        DdiMedia_DestroyBuffer(ctx,mediaBufferHeapElmt->uiVaBufferID);
        //Ensure the non-empty buffer to be destroyed.
        --bufNums;
    }
}

void MediaLibvaInterfaceNext::FreeImageHeapElements(VADriverContextP    ctx)
{
    DDI_FUNCTION_ENTER();

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
        DdiMedia_DestroyImage(ctx, mediaImageHeapElmt->uiVaImageID);
    }
}

void MediaLibvaInterfaceNext::FreeContextHeapElements(VADriverContextP ctx)
{
    DDI_FUNCTION_ENTER();

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
    DDI_FUNCTION_ENTER();

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
    DdiMediaUtil_DestroyMutex(&mediaCtx->SurfaceMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->BufferMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->ImageMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->DecoderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->EncoderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->VpMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->ProtMutex);

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
    DDI_FUNCTION_ENTER();

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
        DdiMedia_DestroyContext(ctx, vaCtxID);
    }
}

VAStatus MediaLibvaInterfaceNext::Terminate(VADriverContextP ctx)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,       "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx   = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DdiMediaUtil_LockMutex(&m_GlobalMutex);

#if !defined(ANDROID) && defined(X11_FOUND)
    DestroyX11Connection(mediaCtx);
    DdiMediaUtil_DestroyMutex(&mediaCtx->PutSurfaceRenderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->PutSurfaceSwapBufferMutex);

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
    MosInterface::CloseOsUtilities(nullptr);

    ReleaseCompList(mediaCtx);
    if(mediaCtx->m_hwInfo)
    {
        MOS_FreeMemory(mediaCtx->m_hwInfo);
    }
    mediaCtx->m_hwInfo = nullptr;

    if (mediaCtx->uiRef > 1)
    {
        mediaCtx->uiRef--;
        DdiMediaUtil_UnLockMutex(&m_GlobalMutex);

        return VA_STATUS_SUCCESS;
    }
    mediaCtx->SkuTable.reset();
    mediaCtx->WaTable.reset();

    // release media driver context, ctx creation is behind the mos_utilities_init
    // If free earilier than MOS_utilities_close, memnja count error.
    MOS_FreeMemory(mediaCtx);
    mediaCtx         = nullptr;
    ctx->pDriverData = nullptr;

    DdiMediaUtil_UnLockMutex(&m_GlobalMutex);

    return VA_STATUS_SUCCESS;
}

#if !defined(ANDROID) && defined(X11_FOUND)

#define X11_LIB_NAME "libX11.so.6"

void MediaLibvaInterfaceNext::DestroyX11Connection(
    PDDI_MEDIA_CONTEXT mediaCtx
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx", );
    DDI_CHK_NULL(mediaCtx->X11FuncTable, "nullptr X11FuncTable", );

    MOS_FreeLibrary(mediaCtx->X11FuncTable->pX11LibHandle);
    MOS_FreeMemory(mediaCtx->X11FuncTable);
    mediaCtx->X11FuncTable = nullptr;

    return;
}

VAStatus MediaLibvaInterfaceNext::ConnectX11(
    PDDI_MEDIA_CONTEXT mediaCtx
)
{
    DDI_FUNCTION_ENTER();
    
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaCtx->X11FuncTable = (PDDI_X11_FUNC_TABLE)MOS_AllocAndZeroMemory(sizeof(DDI_X11_FUNC_TABLE));
    DDI_CHK_NULL(mediaCtx->X11FuncTable, "Allocation Failed for X11FuncTable", VA_STATUS_ERROR_ALLOCATION_FAILED);

    HMODULE    h_module   = nullptr;
    MOS_STATUS mos_status = MOS_LoadLibrary(X11_LIB_NAME, &h_module);
    if (MOS_STATUS_SUCCESS != mos_status || nullptr == h_module)
    {
        DestroyX11Connection(mediaCtx);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    mediaCtx->X11FuncTable->pX11LibHandle = h_module;

    mediaCtx->X11FuncTable->pfnXCreateGC =
        MOS_GetProcAddress(h_module, "XCreateGC");
    mediaCtx->X11FuncTable->pfnXFreeGC =
        MOS_GetProcAddress(h_module, "XFreeGC");
    mediaCtx->X11FuncTable->pfnXCreateImage =
        MOS_GetProcAddress(h_module, "XCreateImage");
    mediaCtx->X11FuncTable->pfnXDestroyImage =
        MOS_GetProcAddress(h_module, "XDestroyImage");
    mediaCtx->X11FuncTable->pfnXPutImage =
        MOS_GetProcAddress(h_module, "XPutImage");

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
    DDI_FUNCTION_ENTER();
    
    DDI_CHK_NULL(ctx,         "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);

    struct VADriverVTable    *pVTable     = DDI_CODEC_GET_VTABLE(ctx);
    DDI_CHK_NULL(pVTable,     "nullptr pVTable",      VA_STATUS_ERROR_INVALID_CONTEXT);

    struct VADriverVTableVPP *pVTableVpp  = DDI_CODEC_GET_VTABLE_VPP(ctx);
    DDI_CHK_NULL(pVTableVpp,  "nullptr pVTableVpp",   VA_STATUS_ERROR_INVALID_CONTEXT);

#if VA_CHECK_VERSION(1,11,0)
    struct VADriverVTableProt *pVTableProt = DDI_CODEC_GET_VTABLE_PROT(ctx);
    DDI_CHK_NULL(pVTableProt,  "nullptr pVTableProt",   VA_STATUS_ERROR_INVALID_CONTEXT);
#endif

    ctx->pDriverData                         = nullptr;
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
    pVTable->vaQuerySurfaceAttributes        = QuerySurfaceAttributes;
    pVTable->vaQueryImageFormats             = QueryImageFormats;

    pVTable->vaCreateContext                 = CreateContext;
    pVTable->vaDestroyContext                = DestroyContext;
    pVTable->vaCreateBuffer                  = CreateBuffer;

    pVTableVpp->vaQueryVideoProcFilters      = QueryVideoProcFilters;
    pVTableVpp->vaQueryVideoProcFilterCaps   = QueryVideoProcFilterCaps;
    pVTableVpp->vaQueryVideoProcPipelineCaps = QueryVideoProcPipelineCaps;

    pVTable->vaBeginPicture                  = BeginPicture;
    pVTable->vaRenderPicture                 = RenderPicture;
    pVTable->vaEndPicture                    = EndPicture;

#if VA_CHECK_VERSION(1,11,0)
    pVTableProt->vaCreateProtectedSession    = CreateProtectedSession;
    pVTableProt->vaDestroyProtectedSession   = DestroyProtectedSession;
    pVTableProt->vaAttachProtectedSession    = AttachProtectedSession;
    pVTableProt->vaDetachProtectedSession    = DetachProtectedSession;
    pVTableProt->vaProtectedSessionExecute   = ProtectedSessionExecute;
#endif

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
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,     "nullptr ctx",      VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(context, "nullptr context",  VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaDrvCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaDrvCtx, "nullptr mediaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

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

    uint32_t ctxType        = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void*    ctxPtr         = DdiMedia_GetContextFromContextID(ctx, *context, &ctxType);
    CompType componentIndex = MapComponentFromCtxType(ctxType);
    VAStatus vaStatus = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(mediaDrvCtx->m_compList[componentIndex],  "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);
    vaStatus = mediaDrvCtx->m_compList[componentIndex]->CreateContext(
        ctx, configId - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE, pictureWidth, pictureHeight, flag, renderTarget, renderTargetsNum, context);

    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::DestroyContext (
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,         "nullptr ctx",        VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaDrvCtx = GetMediaContext(ctx);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    DDI_CHK_NULL(mediaDrvCtx, "nullptr mediaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaDrvCtx->m_compList[componentIndex], "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);

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
    DDI_FUNCTION_ENTER();
    int32_t event[] = {size, elementsNum, type};
    MOS_TraceEventExt(EVENT_VA_BUFFER, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL(ctx,       "nullptr ctx",      VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(bufId,     "nullptr buf_id",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(size, 0, "Invalid size",     VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,  "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    DDI_CHK_NULL(ctxPtr,    "nullptr ctxPtr",   VA_STATUS_ERROR_INVALID_CONTEXT);

    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex], "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);
    *bufId = VA_INVALID_ID;

    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    VAStatus vaStatus = mediaCtx->m_compList[componentIndex]->CreateBuffer(ctx, context, type, size, elementsNum, data, bufId);
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);

    MOS_TraceEventExt(EVENT_VA_BUFFER, EVENT_TYPE_END, bufId, sizeof(bufId), nullptr, 0);
    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::BeginPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       renderTarget)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,                    "nullptr ctx",                    VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx   = GetMediaContext(ctx);

    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)renderTarget, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "renderTarget", VA_STATUS_ERROR_INVALID_SURFACE);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    uint32_t event[] = {(uint32_t)context, ctxType, (uint32_t)renderTarget};
    MOS_TraceEventExt(EVENT_VA_PICTURE, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    PDDI_MEDIA_SURFACE surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, renderTarget);
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);

    DdiMediaUtil_LockMutex(&mediaCtx->SurfaceMutex);
    surface->curCtxType = ctxType;
    surface->curStatusReportQueryState = DDI_MEDIA_STATUS_REPORT_QUERY_STATE_PENDING;
    if(ctxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        surface->curStatusReport.vpp.status = VPREP_NOTAVAILABLE;
    }
    DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);

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
    DDI_FUNCTION_ENTER();

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
    void     *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex],  "nullptr complist", VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[componentIndex]->RenderPicture(ctx, context, buffers, buffersNum);
}

VAStatus MediaLibvaInterfaceNext::EndPicture (
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,                                   "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                              "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    CompType componentIndex = MapComponentFromCtxType(ctxType);
    DDI_CHK_NULL(mediaCtx->m_compList[componentIndex],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = mediaCtx->m_compList[componentIndex]->EndPicture(ctx, context);

    MOS_TraceEventExt(EVENT_VA_PICTURE, EVENT_TYPE_END, &context, sizeof(context), &vaStatus, sizeof(vaStatus));
    PERF_UTILITY_STOP_ONCE("First Frame Time", PERF_MOS, PERF_LEVEL_DDI);

    return vaStatus;
}

VAStatus MediaLibvaInterfaceNext::QueryConfigEntrypoints(
    VADriverContextP ctx,
    VAProfile        profile,
    VAEntrypoint     *entrypointList,
    int32_t          *entrypointsNum)
{
    DDI_FUNCTION_ENTER();

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
    DDI_FUNCTION_ENTER();

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
    DDI_FUNCTION_ENTER();

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
    DDI_FUNCTION_ENTER();
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
    DDI_FUNCTION_ENTER();

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
    DDI_FUNCTION_ENTER();

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
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,        "nullptr ctx",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(attribsNum, "nullptr attribsNum",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(attribList, "nullptr attribList",  VA_STATUS_ERROR_INVALID_PARAMETER);

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
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,        "nullptr ctx",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(formatList, "nullptr formatList",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(formatsNum, "nullptr formatsNum",  VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,             "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_capsNext, "nullptr caps",      VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_capsNext->QueryImageFormats(formatList, formatsNum);
}

CompType MediaLibvaInterfaceNext::MapCompTypeFromEntrypoint(VAEntrypoint entrypoint)
{
    DDI_FUNCTION_ENTER();

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
    DDI_FUNCTION_ENTER();
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
    DDI_FUNCTION_ENTER();
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
    DDI_FUNCTION_ENTER();
    DDI_CHK_NULL(ctx,                           "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                      "nullptr mediaCtx",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_compList[CompVp],  "nullptr complist",  VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_compList[CompVp]->QueryVideoProcPipelineCaps(ctx, context, filters, filtersNum,
        pipelineCaps);
}

#if VA_CHECK_VERSION(1,11,0)
VAStatus MediaLibvaInterfaceNext::CreateProtectedSession(
    VADriverContextP      ctx,
    VAConfigID            configId,
    VAProtectedSessionID  *protectedSession)
{
    DDI_FUNCTION_ENTER();
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
    DDI_FUNCTION_ENTER();
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
    DDI_FUNCTION_ENTER();
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
    DDI_FUNCTION_ENTER();
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
    DDI_FUNCTION_ENTER();
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
