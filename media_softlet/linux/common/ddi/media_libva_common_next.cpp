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
//! \file     media_libva_common_next.cpp
//! \brief    libva common next implementaion.
//!
#include <stdint.h>
#include "mos_utilities.h"
#include "media_libva_common_next.h"
#include "media_libva_util_next.h"
#include "mos_solo_generic.h"
#include "mos_interface.h"
#include "media_libva_interface_next.h"
#include "media_ddi_prot.h"

DDI_MEDIA_SURFACE* MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(PDDI_MEDIA_CONTEXT mediaCtx, VASurfaceID surfaceID)
{
    uint32_t                         id             = 0;
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  surfaceElement = nullptr;
    PDDI_MEDIA_SURFACE               surface        = nullptr;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", nullptr);

    id                = (uint32_t)surfaceID;
    bool validSurface = (id != VA_INVALID_SURFACE);
    if(validSurface)
    {
        DDI_CHK_LESS(id, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "invalid surface id", nullptr);
        MosUtilities::MosLockMutex(&mediaCtx->SurfaceMutex);
        surfaceElement  = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)mediaCtx->pSurfaceHeap->pHeapBase;
        surfaceElement += id;
        surface         = surfaceElement->pSurface;
        MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
    }

    return surface;
}

void MediaLibvaCommonNext::MediaSurfaceToMosResource(DDI_MEDIA_SURFACE *mediaSurface, MOS_RESOURCE  *mosResource)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface",);
    DDI_CHK_NULL(mosResource,  "nullptr mosResource",);

    MosInterface::ConvertResourceFromDdi(mediaSurface, mosResource, OS_SPECIFIC_RESOURCE_SURFACE, 0);

    Mos_Solo_SetOsResource(mediaSurface->pGmmResourceInfo, mosResource);

    return;
}

void MediaLibvaCommonNext::MediaBufferToMosResource(DDI_MEDIA_BUFFER *mediaBuffer, MOS_RESOURCE *mosResource)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer",);
    DDI_CHK_NULL(mosResource, "nullptr mosResource",);

    if (nullptr == mediaBuffer->bo)
    {
        DDI_ASSERTMESSAGE("nullptr mediaBuffer->bo");
    }

    MosInterface::ConvertResourceFromDdi(mediaBuffer, mosResource, OS_SPECIFIC_RESOURCE_BUFFER, 0);

    Mos_Solo_SetOsResource(mediaBuffer->pGmmResourceInfo, mosResource);

    return;
}

VASurfaceID MediaLibvaCommonNext::GetVASurfaceIDFromSurface(PDDI_MEDIA_SURFACE surface)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surface, "nullptr surface", VA_INVALID_SURFACE);
    DDI_CHK_NULL(surface->pMediaCtx, "nullptr mediaCtx", VA_INVALID_SURFACE);
    DDI_CHK_NULL(surface->pMediaCtx->pSurfaceHeap, "nullptr surface heap", VA_INVALID_SURFACE);

    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  surfaceElement = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surface->pMediaCtx->pSurfaceHeap->pHeapBase;
    DDI_CHK_NULL(surfaceElement, "nullptr surface element", VA_INVALID_SURFACE);
    for(uint32_t i = 0; i < surface->pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements; i ++)
    {
        if(surface == surfaceElement->pSurface)
        {
            return surfaceElement->uiVaSurfaceID;
        }
        surfaceElement ++;
    }
    return VA_INVALID_SURFACE;
}

PDDI_MEDIA_SURFACE MediaLibvaCommonNext::ReplaceSurfaceWithNewFormat(PDDI_MEDIA_SURFACE surface, DDI_MEDIA_FORMAT expectedFormat)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surface, "nullptr surface", nullptr);

    PDDI_MEDIA_SURFACE_HEAP_ELEMENT surfaceElement = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surface->pMediaCtx->pSurfaceHeap->pHeapBase;
    PDDI_MEDIA_CONTEXT mediaCtx = surface->pMediaCtx;

    // Check some conditions
    if (expectedFormat == surface->format)
    {
        return surface;
    }
    // Create new dst surface and copy the structure
    PDDI_MEDIA_SURFACE dstSurface = (DDI_MEDIA_SURFACE *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE));
    if (nullptr == surfaceElement)
    {
        MOS_FreeMemory(dstSurface);
        return nullptr;
    }

    MOS_SecureMemcpy(dstSurface, sizeof(DDI_MEDIA_SURFACE), surface, sizeof(DDI_MEDIA_SURFACE));
    DDI_CHK_NULL(dstSurface, "nullptr dstSurface", nullptr);
    dstSurface->format = expectedFormat;
    dstSurface->uiLockedBufID = VA_INVALID_ID;
    dstSurface->uiLockedImageID = VA_INVALID_ID;
    dstSurface->pSurfDesc = nullptr;
    // Lock surface heap
    MosUtilities::MosLockMutex(&mediaCtx->SurfaceMutex);
    uint32_t i;
    // Get current element heap and index
    for (i = 0; i < mediaCtx->pSurfaceHeap->uiAllocatedHeapElements; i++)
    {
        if (surface == surfaceElement->pSurface)
        {
            break;
        }
        surfaceElement++;
    }
    // If cant find
    if (i == surface->pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements)
    {
        MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
        MOS_FreeMemory(dstSurface);
        return nullptr;
    }
    // FreeSurface
    MediaLibvaUtilNext::FreeSurface(surface);
    MOS_FreeMemory(surface);
    // CreateNewSurface
    MediaLibvaUtilNext::CreateSurface(dstSurface,mediaCtx);
    surfaceElement->pSurface = dstSurface;

    MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);

    return dstSurface;
}

PDDI_MEDIA_SURFACE MediaLibvaCommonNext::ReplaceSurfaceWithVariant(PDDI_MEDIA_SURFACE surface, VAEntrypoint entrypoint)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(surface, "nullptr surface", nullptr);

    PDDI_MEDIA_CONTEXT mediaCtx = surface->pMediaCtx;
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", nullptr);
    uint32_t alignedWidth, alignedHeight;
    DDI_MEDIA_FORMAT alignedFormat;

    if(surface->uiVariantFlag)
    {
        return surface;
    }

    VASurfaceID vaID = MediaLibvaCommonNext::GetVASurfaceIDFromSurface(surface);
    if(VA_INVALID_SURFACE == vaID)
    {
        return nullptr;
    }

    MosUtilities::MosLockMutex(&mediaCtx->SurfaceMutex);
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT surfaceElement = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surface->pMediaCtx->pSurfaceHeap->pHeapBase;
    if (surfaceElement == nullptr)
    {
        MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
        return nullptr;
    }
    surfaceElement += vaID;
    MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);

    alignedFormat = surface->format;
    switch (surface->format)
    {
        case Media_Format_AYUV:
#if VA_CHECK_VERSION(1, 13, 0)
        case Media_Format_XYUV:
#endif
            alignedWidth = MOS_ALIGN_CEIL(surface->iWidth, 128);
            alignedHeight = MOS_ALIGN_CEIL(surface->iHeight * 3 / 4, 64);
            break;
        case Media_Format_Y410:
            alignedWidth = MOS_ALIGN_CEIL(surface->iWidth, 64);
            alignedHeight = MOS_ALIGN_CEIL(surface->iHeight * 3 / 2, 64);
            break;
        case Media_Format_Y216:
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y212:
#endif
        case Media_Format_Y210:
            alignedWidth = (surface->iWidth + 1) >> 1;
            alignedHeight = surface->iHeight * 2;
            alignedFormat = Media_Format_Y216;
            break;
        case Media_Format_YUY2:
            alignedWidth = (surface->iWidth + 1) >> 1;
            alignedHeight = surface->iHeight * 2;
            break;
        case Media_Format_P016:
        case Media_Format_P012:
        case Media_Format_P010:
            alignedHeight = surface->iHeight;
            alignedWidth = surface->iWidth;
            if(entrypoint == VAEntrypointEncSlice)
            {
                alignedWidth = surface->iWidth * 2;
                alignedFormat = Media_Format_NV12;
            }
            else
            {
                alignedFormat = Media_Format_P016;
            }
            break;
        default:
            return surface;
       }

    //create new dst surface and copy the structure
    PDDI_MEDIA_SURFACE dstSurface = (DDI_MEDIA_SURFACE *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE));
    DDI_CHK_NULL(dstSurface, "nullptr dstSurface", nullptr);
    MOS_SecureMemcpy(dstSurface,sizeof(DDI_MEDIA_SURFACE),surface,sizeof(DDI_MEDIA_SURFACE));

    dstSurface->uiVariantFlag = 1;
    dstSurface->format = alignedFormat;
    dstSurface->iWidth = alignedWidth;
    dstSurface->iHeight = alignedHeight;
   //CreateNewSurface
    if(MediaLibvaUtilNext::CreateSurface(dstSurface,mediaCtx) != VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(dstSurface);
        return surface;
    }
    //replace the surface
    MosUtilities::MosLockMutex(&mediaCtx->SurfaceMutex);
    surfaceElement = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surface->pMediaCtx->pSurfaceHeap->pHeapBase;
    surfaceElement += vaID;
    surfaceElement->pSurface = dstSurface;
    MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
    //FreeSurface
    MediaLibvaUtilNext::FreeSurface(surface);
    MOS_FreeMemory(surface);

    return dstSurface;
}

DDI_MEDIA_BUFFER* MediaLibvaCommonNext::GetBufferFromVABufferID(
    PDDI_MEDIA_CONTEXT mediaCtx,
    VABufferID         bufferID)
{
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", nullptr);

    uint32_t                       i = 0;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufHeapElement = nullptr;
    PDDI_MEDIA_BUFFER              buf = nullptr;

    i = (uint32_t)bufferID;
    DDI_CHK_LESS(i, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", nullptr);

    MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
    bufHeapElement  = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)mediaCtx->pBufferHeap->pHeapBase;
    bufHeapElement += i;
    buf             = bufHeapElement->pBuffer;
    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);

    return buf;
}

void* MediaLibvaCommonNext::GetVaContextFromHeap(
    PDDI_MEDIA_HEAP  mediaHeap,
    uint32_t         index,
    PMOS_MUTEX       mutex)
{
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaCtxHeapElmt = nullptr;
    void                              *context      = nullptr;
    DDI_FUNC_ENTER;

    MosUtilities::MosLockMutex(mutex);
    if(nullptr == mediaHeap || index >= mediaHeap->uiAllocatedHeapElements)
    {
        MosUtilities::MosUnlockMutex(mutex);
        return nullptr;
    }
    vaCtxHeapElmt  = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)mediaHeap->pHeapBase;
    vaCtxHeapElmt  += index;
    context        = vaCtxHeapElmt->pVaContext;
    MosUtilities::MosUnlockMutex(mutex);

    return context;
}

void* MediaLibvaCommonNext::GetContextFromContextID(
    VADriverContextP ctx,
    VAContextID      vaCtxID,
    uint32_t         *ctxType)
{
    PDDI_MEDIA_CONTEXT       mediaCtx = nullptr;
    uint32_t                 index    = 0;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx, "nullptr ctx", nullptr);
    DDI_CHK_NULL(ctxType, "nullptr ctxType", nullptr);

    if (vaCtxID < DDI_MEDIA_VACONTEXTID_BASE)
    {
        DDI_ASSERTMESSAGE("Invalid ContextID");
        return nullptr;
    }
    mediaCtx = GetMediaContext(ctx);
    index    = vaCtxID & DDI_MEDIA_MASK_VACONTEXTID;

    if (index >= DDI_MEDIA_MAX_INSTANCE_NUMBER)
    {
        return nullptr;
    }

    if ((vaCtxID & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_SOFTLET_VACONTEXTID_DECODER_OFFSET)
    {
        DDI_VERBOSEMESSAGE("Decode context detected: 0x%x", vaCtxID);
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_DECODER;
        return GetVaContextFromHeap(mediaCtx->pDecoderCtxHeap, index, &mediaCtx->DecoderMutex);
    }
    else if ((vaCtxID & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_SOFTLET_VACONTEXTID_ENCODER_OFFSET)
    {
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_ENCODER;
        return GetVaContextFromHeap(mediaCtx->pEncoderCtxHeap, index, &mediaCtx->EncoderMutex);
    }
    else if ((vaCtxID & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_SOFTLET_VACONTEXTID_VP_OFFSET)
    {
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_VP;
        return GetVaContextFromHeap(mediaCtx->pVpCtxHeap, index, &mediaCtx->VpMutex);
    }
    else if ((vaCtxID & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_SOFTLET_VACONTEXTID_CP_OFFSET)
    {
        DDI_VERBOSEMESSAGE("Protected session detected: 0x%x", vaCtxID);
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_PROTECTED;
        index = index & DDI_MEDIA_MASK_VAPROTECTEDSESSION_ID;
        return GetVaContextFromHeap(mediaCtx->pProtCtxHeap, index, &mediaCtx->ProtMutex);
    }
    else
    {
        DDI_ASSERTMESSAGE("Invalid context: 0x%x", vaCtxID);
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
        return nullptr;
    }
}

uint32_t MediaLibvaCommonNext::GetCtxTypeFromVABufferID(PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID)
{
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufHeapElement = nullptr;
    uint32_t                       i              = 0;
    uint32_t                       ctxType        = DDI_MEDIA_CONTEXT_TYPE_NONE;

    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", DDI_MEDIA_CONTEXT_TYPE_NONE);
    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_PARAMETER);

    i = (uint32_t)bufferID;
    DDI_CHK_LESS(i, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", DDI_MEDIA_CONTEXT_TYPE_NONE);
    MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
    bufHeapElement  = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)mediaCtx->pBufferHeap->pHeapBase;
    bufHeapElement  += i;
    ctxType = bufHeapElement->uiCtxType;
    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);

    return ctxType;
}

void* MediaLibvaCommonNext::GetCtxFromVABufferID(PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID)
{
    uint32_t                       i              = 0;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufHeapElement = nullptr;
    DDI_FUNC_ENTER;
    DDI_CHK_NULL(mediaCtx,              "nullptr mediaCtx",              nullptr);
    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", nullptr);

    i = (uint32_t)bufferID;
    DDI_CHK_LESS(i, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", nullptr);
    MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
    bufHeapElement  = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)mediaCtx->pBufferHeap->pHeapBase;
    bufHeapElement += i;
    void *temp      = bufHeapElement->pCtx;
    MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);

    return temp;
}

int32_t MediaLibvaCommonNext::GetGpuPriority(
    VADriverContextP ctx,
    VABufferID       *buffers,
    int32_t          numBuffers,
    bool             *updatePriority,
    int32_t          *priority)
{
    void                           *data                 = nullptr;
    uint32_t                       updateSessionPriority = 0;
    uint32_t                       priorityValue         = 0;
    int32_t                        priorityIndexInBuf    = -1;
    PDDI_MEDIA_CONTEXT             mediaCtx              = nullptr;
    DDI_MEDIA_BUFFER               *buf                  = nullptr;
    VAContextParameterUpdateBuffer *contextParamBuf      = nullptr;

    DDI_FUNC_ENTER;
    DDI_CHK_NULL(ctx, "nullptr context in GetGpuPriority!", -1);

    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", -1);

#if VA_CHECK_VERSION(1, 10, 0)
    for (int32_t i = 0; i < numBuffers; i++)
    {
        buf = GetBufferFromVABufferID(mediaCtx, buffers[i]);
        DDI_CHK_NULL(buf, "Invalid buffer.", -1);

        if ((int32_t)buf->uiType == VAContextParameterUpdateBufferType)
        {
            //Read the priority from the VAContextParameterUpdateBuffer
            MediaLibvaInterfaceNext::MapBuffer(ctx, buffers[i], &data);
            DDI_CHK_NULL(data, "nullptr data.", -1);

            contextParamBuf = (VAContextParameterUpdateBuffer *)data;
            DDI_CHK_NULL(contextParamBuf, "nullptr contextParamBuf.", -1);

            updateSessionPriority = contextParamBuf->flags.bits.context_priority_update;
            priorityValue         = contextParamBuf->context_priority.bits.priority;

            if (updateSessionPriority)
            {
                *updatePriority = true;
                if (priorityValue <= CONTEXT_PRIORITY_MAX)
                {
                    *priority = priorityValue - CONTEXT_PRIORITY_MAX / 2;
                }
                else
                {
                    *priority = 0;
                }
            }
            else
            {
                *updatePriority = false;
                *priority       = 0;
            }

            MediaLibvaInterfaceNext::UnmapBuffer(ctx, buffers[i]);
            priorityIndexInBuf = i;
            break;
        }
    }
#endif
    return priorityIndexInBuf;
}

void MediaLibvaCommonNext::MovePriorityBufferIdToEnd(VABufferID *buffers, int32_t priorityIndexInBuf, int32_t numBuffers)
{
    VABufferID  vaBufferID = 0;
    DDI_FUNC_ENTER;

    if( (numBuffers > 1) && (priorityIndexInBuf < numBuffers -1) )
    {
        vaBufferID = buffers[priorityIndexInBuf];
        while(priorityIndexInBuf < (numBuffers - 1) )
        {
            buffers[priorityIndexInBuf] = buffers[priorityIndexInBuf+1];
            priorityIndexInBuf++;
        }
        buffers[numBuffers -1] = vaBufferID;
    }
    return;
}
