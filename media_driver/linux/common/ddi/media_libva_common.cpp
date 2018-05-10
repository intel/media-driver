/*
* Copyright (c) 2015-2018, Intel Corporation
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
//! \file     media_libva_common.cpp
//! \brief    libva(and its extension) interface implemantation common functions
//!

#include "media_libva.h"
#include "media_libva_util.h"
#include "mos_solo_generic.h"

static void* DdiMedia_GetVaContextFromHeap(PDDI_MEDIA_HEAP  mediaHeap, uint32_t index, PMEDIA_MUTEX_T mutex)
{
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT  vaCtxHeapElmt;
    void                              *context;

    DdiMediaUtil_LockMutex(mutex);
    if(nullptr == mediaHeap || index >= mediaHeap->uiAllocatedHeapElements)
    {
        DdiMediaUtil_UnLockMutex(mutex);
        return nullptr;
    }
    vaCtxHeapElmt  = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)mediaHeap->pHeapBase;
    vaCtxHeapElmt += index;
    context        = vaCtxHeapElmt->pVaContext;
    DdiMediaUtil_UnLockMutex(mutex);

    return context;
}

void DdiMedia_MediaSurfaceToMosResource(DDI_MEDIA_SURFACE *mediaSurface, MOS_RESOURCE  *mosResource)
{
    DDI_ASSERT(mediaSurface->bo);

    switch (mediaSurface->format)
    {
        case Media_Format_NV12:
            mosResource->Format    = Format_NV12;
            break;
        case Media_Format_NV21:
            mosResource->Format    = Format_NV21;
            break;
        case Media_Format_YUY2:
            mosResource->Format    = Format_YUY2;
            break;
        case Media_Format_X8R8G8B8:
            mosResource->Format    = Format_X8R8G8B8;
            break;
        case Media_Format_X8B8G8R8:
            mosResource->Format    = Format_X8B8G8R8;
            break;
        case Media_Format_A8B8G8R8:
        case Media_Format_R8G8B8A8:
            mosResource->Format    = Format_A8B8G8R8;
            break;
        case Media_Format_A8R8G8B8:
            mosResource->Format    = Format_A8R8G8B8;
            break;
        case Media_Format_R5G6B5:
            mosResource->Format    = Format_R5G6B5;
            break;
        case Media_Format_R8G8B8:
            mosResource->Format    = Format_R8G8B8;
            break;
        case Media_Format_444P:
            mosResource->Format    = Format_444P;
            break;
        case Media_Format_411P:
            mosResource->Format    = Format_411P;
            break;
        case Media_Format_IMC3:
            mosResource->Format    = Format_IMC3;
            break;
        case Media_Format_400P:
            mosResource->Format    = Format_400P;
            break;
        case Media_Format_422H:
            mosResource->Format    = Format_422H;
            break;
        case Media_Format_422V:
            mosResource->Format    = Format_422V;
            break;
        case Media_Format_Buffer:
            mosResource->Format    = Format_Any;
        case Media_Format_P010:
            mosResource->Format    = Format_P010;
            break;
        case Media_Format_R10G10B10A2:
            mosResource->Format    = Format_R10G10B10A2;
            break;
        case Media_Format_B10G10R10A2:
            mosResource->Format    = Format_B10G10R10A2;
            break;
        case Media_Format_UYVY:
            mosResource->Format    = Format_UYVY;
            break;
        default:
            DDI_ASSERTMESSAGE("DDI: unsupported media format for surface.");
            break;
    }
    mosResource->iWidth    = mediaSurface->iWidth;
    mosResource->iHeight   = mediaSurface->iHeight;
    mosResource->iPitch    = mediaSurface->iPitch;
    mosResource->iCount    = mediaSurface->iRefCount;
    mosResource->isTiled   = mediaSurface->isTiled;
    mosResource->TileType  = LinuxToMosTileType(mediaSurface->TileType);
    mosResource->bo        = mediaSurface->bo;
    mosResource->name      = mediaSurface->name;

    mosResource->ppCurrentFrameSemaphore   = &mediaSurface->pCurrentFrameSemaphore;
    mosResource->ppReferenceFrameSemaphore = &mediaSurface->pReferenceFrameSemaphore;
    mosResource->bSemInitialized           = false;
    mosResource->bMapped                   = false;

    if(mediaSurface->bMapped == true)
    {
        mosResource->pData     = mediaSurface->pData;
    }
    else
    {
        mosResource->pData     = nullptr;
    }
    mosResource->pGmmResInfo   = mediaSurface->pGmmResourceInfo;
    mosResource->dwGfxAddress  = 0;

    // for MOS wrapper
    mosResource->bConvertedFromDDIResource = true;

    Mos_Solo_SetOsResource(mediaSurface->pGmmResourceInfo, mosResource);

}

void DdiMedia_MediaBufferToMosResource(DDI_MEDIA_BUFFER *mediaBuffer, MOS_RESOURCE *mosResource)
{
    DDI_ASSERT(mediaBuffer->bo);

    switch (mediaBuffer->format)
    {
        case Media_Format_Buffer:
            mosResource->Format  = Format_Buffer;
            mosResource->iWidth    = mediaBuffer->iSize;
            mosResource->iHeight   = 1;
            mosResource->iPitch    = mediaBuffer->iSize;
            break;
        case Media_Format_Perf_Buffer:
            mosResource->Format  = Format_Buffer;
            mosResource->iWidth    = mediaBuffer->iSize;
            mosResource->iHeight   = 1;
            mosResource->iPitch    = mediaBuffer->iSize;
            break;
        case Media_Format_2DBuffer:
            mosResource->Format  = Format_Buffer_2D;
            mosResource->iWidth    = mediaBuffer->iWidth;
            mosResource->iHeight   = mediaBuffer->iHeight;
            mosResource->iPitch    = mediaBuffer->iPitch;
            break;
        case Media_Format_CPU:
            return;
        default:
            mosResource->iWidth    = mediaBuffer->iSize;
            mosResource->iHeight   = 1;
            mosResource->iPitch    = mediaBuffer->iSize;
            DDI_ASSERTMESSAGE("DDI: unsupported media format for surface.");
            break;
    }
    mosResource->iCount    = mediaBuffer->iRefCount;
    mosResource->isTiled   = 0;
    mosResource->TileType  = LinuxToMosTileType(mediaBuffer->TileType);
    mosResource->bo        = mediaBuffer->bo;
    mosResource->name      = mediaBuffer->name;
    mosResource->bMapped   = false;

    if(mediaBuffer->bMapped == true)
    {
        mosResource->pData     = mediaBuffer->pData;
    }
    else
    {
        mosResource->pData     = nullptr;
    }
    mosResource->dwGfxAddress  = 0;
    mosResource->pGmmResInfo   = mediaBuffer->pGmmResourceInfo;

    // for MOS wrapper
    mosResource->bConvertedFromDDIResource = true;

    Mos_Solo_SetOsResource(mediaBuffer->pGmmResourceInfo, mosResource);
}

void* DdiMedia_GetContextFromContextID (VADriverContextP ctx, VAContextID vaCtxID, uint32_t *ctxType)
{
    PDDI_MEDIA_CONTEXT       mediaCtx;
    uint32_t                 index;

    mediaCtx  = DdiMedia_GetMediaContext(ctx);
    index    = vaCtxID & DDI_MEDIA_MASK_VACONTEXTID;

    if (index >= DDI_MEDIA_MAX_INSTANCE_NUMBER)
        return nullptr;
    if ((vaCtxID&DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_CENC)
    {
        DDI_VERBOSEMESSAGE("Cenc context detected: 0x%x", vaCtxID);
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER;
        return DdiMedia_GetVaContextFromHeap(mediaCtx->pDecoderCtxHeap, index, &mediaCtx->DecoderMutex);
    }
    else if ((vaCtxID&DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_DECODER)
    {
        DDI_VERBOSEMESSAGE("Decode context detected: 0x%x", vaCtxID);
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_DECODER;
        return DdiMedia_GetVaContextFromHeap(mediaCtx->pDecoderCtxHeap, index, &mediaCtx->DecoderMutex);
    }
    else if ((vaCtxID&DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_ENCODER)
    {
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_ENCODER;
        return DdiMedia_GetVaContextFromHeap(mediaCtx->pEncoderCtxHeap, index, &mediaCtx->EncoderMutex);
    }
    else if ((vaCtxID & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_VP)
    {
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_VP;
        return DdiMedia_GetVaContextFromHeap(mediaCtx->pVpCtxHeap, index, &mediaCtx->VpMutex);
    }
    else if ((vaCtxID & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_CM)
    {
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_CM;
        return DdiMedia_GetVaContextFromHeap(mediaCtx->pCmCtxHeap, index, &mediaCtx->CmMutex);
    }
    else if ((vaCtxID & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_MFE)
    {
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_MFE;
        return DdiMedia_GetVaContextFromHeap(mediaCtx->pMfeCtxHeap, index, &mediaCtx->MfeMutex);
    }
    else
    {
        DDI_ASSERTMESSAGE("Invalid context: 0x%x", vaCtxID);
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
        return nullptr;
    }

}

DDI_MEDIA_SURFACE* DdiMedia_GetSurfaceFromVASurfaceID (PDDI_MEDIA_CONTEXT mediaCtx, VASurfaceID surfaceID)
{
    uint32_t                         i;
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  surfaceElement;
    PDDI_MEDIA_SURFACE               surface;

    i                = (uint32_t)surfaceID;
    DDI_CHK_LESS(i, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "invalid surface id", nullptr);
    DdiMediaUtil_LockMutex(&mediaCtx->SurfaceMutex);
    surfaceElement  = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)mediaCtx->pSurfaceHeap->pHeapBase;
    surfaceElement += i;
    surface         = surfaceElement->pSurface;
    DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);

    return surface;
}

DDI_MEDIA_BUFFER* DdiMedia_GetBufferFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID)
{
    uint32_t                       i;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufHeapElement;
    PDDI_MEDIA_BUFFER              buf;

    i                = (uint32_t)bufferID;
    DDI_CHK_LESS(i, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", nullptr);
    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    bufHeapElement  = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)mediaCtx->pBufferHeap->pHeapBase;
    bufHeapElement += i;
    buf             = bufHeapElement->pBuffer;
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);

    return buf;
}

bool DdiMedia_DestroyBufFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID)
{
    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    DdiMediaUtil_ReleasePMediaBufferFromHeap(mediaCtx->pBufferHeap, bufferID);
    mediaCtx->uiNumBufs--;
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
    return true;
}

