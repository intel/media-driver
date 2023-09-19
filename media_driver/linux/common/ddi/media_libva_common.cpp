/*
* Copyright (c) 2015-2021, Intel Corporation
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
#include "media_ddi_prot.h"
#include "mos_solo_generic.h"
#include "mos_interface.h"
#include "media_libva_caps.h"

static void* DdiMedia_GetVaContextFromHeap(PDDI_MEDIA_HEAP  mediaHeap, uint32_t index, PMEDIA_MUTEX_T mutex)
{
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT  vaCtxHeapElmt = nullptr;
    void                              *context = nullptr;

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
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface",);
    DDI_CHK_NULL(mosResource, "nullptr mosResource",);
    DDI_ASSERT(mosResource->bo);

    MosInterface::ConvertResourceFromDdi(mediaSurface, mosResource, OS_SPECIFIC_RESOURCE_SURFACE, 0);

    Mos_Solo_SetOsResource(mediaSurface->pGmmResourceInfo, mosResource);

    return;
}

void DdiMedia_MediaBufferToMosResource(DDI_MEDIA_BUFFER *mediaBuffer, MOS_RESOURCE *mosResource)
{
    DDI_CHK_NULL(mediaBuffer, "nullptr mediaBuffer",);
    DDI_CHK_NULL(mosResource, "nullptr mosResource",);
    DDI_ASSERT(mediaBuffer->bo);

    MosInterface::ConvertResourceFromDdi(mediaBuffer, mosResource, OS_SPECIFIC_RESOURCE_BUFFER, 0);

    Mos_Solo_SetOsResource(mediaBuffer->pGmmResourceInfo, mosResource);

    return;
}

void* DdiMedia_GetContextFromContextID (VADriverContextP ctx, VAContextID vaCtxID, uint32_t *ctxType)
{
    PDDI_MEDIA_CONTEXT       mediaCtx = nullptr;
    uint32_t                 index = 0;

    DDI_CHK_NULL(ctx, "nullptr ctx", nullptr);
    DDI_CHK_NULL(ctxType, "nullptr ctxType", nullptr);

    mediaCtx  = DdiMedia_GetMediaContext(ctx);
    index    = vaCtxID & DDI_MEDIA_MASK_VACONTEXTID;

    if (index >= DDI_MEDIA_MAX_INSTANCE_NUMBER)
        return nullptr;
    if ((vaCtxID&DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_PROT)
    {
        DDI_VERBOSEMESSAGE("Protected session detected: 0x%x", vaCtxID);
        *ctxType = DDI_MEDIA_CONTEXT_TYPE_PROTECTED;
        index = index & DDI_MEDIA_MASK_VAPROTECTEDSESSION_ID;
        return DdiMedia_GetVaContextFromHeap(mediaCtx->pProtCtxHeap, index, &mediaCtx->ProtMutex);
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
    uint32_t                         i = 0;
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  surfaceElement = nullptr;
    PDDI_MEDIA_SURFACE               surface = nullptr;

    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", nullptr);

    i                = (uint32_t)surfaceID;
    bool validSurface = (i != VA_INVALID_SURFACE);
    if(validSurface)
    {
        DDI_CHK_LESS(i, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "invalid surface id", nullptr);
        DdiMediaUtil_LockMutex(&mediaCtx->SurfaceMutex);
        surfaceElement  = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)mediaCtx->pSurfaceHeap->pHeapBase;
        surfaceElement += i;
        surface         = surfaceElement->pSurface;
        DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);
    }

    return surface;
}

VASurfaceID DdiMedia_GetVASurfaceIDFromSurface(PDDI_MEDIA_SURFACE surface)
{
    DDI_CHK_NULL(surface, "nullptr surface", VA_INVALID_SURFACE);

    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  surfaceElement = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surface->pMediaCtx->pSurfaceHeap->pHeapBase;
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

PDDI_MEDIA_SURFACE DdiMedia_ReplaceSurfaceWithNewFormat(PDDI_MEDIA_SURFACE surface, DDI_MEDIA_FORMAT expectedFormat)
{
    DDI_CHK_NULL(surface, "nullptr surface", nullptr);

    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  surfaceElement = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surface->pMediaCtx->pSurfaceHeap->pHeapBase;
    PDDI_MEDIA_CONTEXT mediaCtx = surface->pMediaCtx;

    //check some conditions
    if(expectedFormat == surface->format)
    {
        return surface;
    }
    //create new dst surface and copy the structure
    PDDI_MEDIA_SURFACE dstSurface = (DDI_MEDIA_SURFACE *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE));
    DDI_CHK_NULL(dstSurface, "nullptr dstSurface", nullptr);
    if (nullptr == surfaceElement)
    {
        MOS_FreeMemory(dstSurface);
        return nullptr;
    }

    MOS_SecureMemcpy(dstSurface,sizeof(DDI_MEDIA_SURFACE),surface,sizeof(DDI_MEDIA_SURFACE));
    dstSurface->format = expectedFormat;
    dstSurface->uiLockedBufID = VA_INVALID_ID;
    dstSurface->uiLockedImageID = VA_INVALID_ID;
    dstSurface->pSurfDesc = nullptr;

    // copy for some pointer resource
    if(dstSurface->pShadowBuffer)
    {
        dstSurface->pShadowBuffer = (PDDI_MEDIA_BUFFER)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
        if (nullptr == dstSurface->pShadowBuffer)
        {
            MOS_FreeMemory(dstSurface);
            return nullptr;
        }
        MOS_SecureMemcpy(dstSurface->pShadowBuffer, sizeof(DDI_MEDIA_BUFFER), surface->pShadowBuffer, sizeof(DDI_MEDIA_BUFFER));
        mos_bo_reference(dstSurface->pShadowBuffer->bo);

        dstSurface->pShadowBuffer->pGmmResourceInfo = (GMM_RESOURCE_INFO *)MOS_AllocAndZeroMemory(sizeof(GMM_RESOURCE_INFO));
        if (nullptr == dstSurface->pShadowBuffer->pGmmResourceInfo)
        {
            MOS_FreeMemory(dstSurface->pShadowBuffer);
            MOS_FreeMemory(dstSurface);
            return nullptr;
        }
        MOS_SecureMemcpy(dstSurface->pShadowBuffer->pGmmResourceInfo, sizeof(GMM_RESOURCE_INFO), surface->pShadowBuffer->pGmmResourceInfo, sizeof(GMM_RESOURCE_INFO));
    }

    //lock surface heap
    DdiMediaUtil_LockMutex(&mediaCtx->SurfaceMutex);
    uint32_t i;
    //get current element heap and index
    for(i = 0; i < mediaCtx->pSurfaceHeap->uiAllocatedHeapElements; i ++)
    {
        if(surface == surfaceElement->pSurface)
        {
            break;
        }
        surfaceElement ++;
    }
    //if cant find
    if(i == surface->pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements)
    {
        DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);
        if(dstSurface->pShadowBuffer)
        {
            MOS_FreeMemory(dstSurface->pShadowBuffer->pGmmResourceInfo);
            MOS_FreeMemory(dstSurface->pShadowBuffer);
        }
        MOS_FreeMemory(dstSurface);
        return nullptr;
    }
    //CreateNewSurface
    DdiMediaUtil_CreateSurface(dstSurface,mediaCtx);
    surfaceElement->pSurface = dstSurface;
    //FreeSurface
    DdiMediaUtil_FreeSurface(surface);
    MOS_FreeMemory(surface);

    DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);

    return dstSurface;
}

PDDI_MEDIA_SURFACE DdiMedia_ReplaceSurfaceWithVariant(PDDI_MEDIA_SURFACE surface, VAEntrypoint entrypoint)
{
    DDI_CHK_NULL(surface, "nullptr surface", nullptr);

    PDDI_MEDIA_CONTEXT mediaCtx = surface->pMediaCtx;
    uint32_t aligned_width, aligned_height;
    DDI_MEDIA_FORMAT aligned_format;

    //check some conditions
    if(surface->uiVariantFlag)
    {
        return surface;
    }

    VASurfaceID vaID = DdiMedia_GetVASurfaceIDFromSurface(surface);
    if(VA_INVALID_SURFACE == vaID)
    {
        return nullptr;
    }

    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  surfaceElement = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surface->pMediaCtx->pSurfaceHeap->pHeapBase;
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
    if (nullptr == surfaceElement)
    {
        return nullptr;
    }
    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    surfaceElement += vaID;
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);

    aligned_format = surface->format;
    switch (surface->format)
    {
        case Media_Format_AYUV:
#if VA_CHECK_VERSION(1, 13, 0)
        case Media_Format_XYUV:
#endif
            aligned_width = MOS_ALIGN_CEIL(surface->iWidth, 128);
            aligned_height = MOS_ALIGN_CEIL(surface->iHeight * 3 / 4, 64);
            break;
        case Media_Format_Y410:
            aligned_width = MOS_ALIGN_CEIL(surface->iWidth, 64);
            aligned_height = MOS_ALIGN_CEIL(surface->iHeight * 3 / 2, 64);
            break;
        case Media_Format_Y216:
#if VA_CHECK_VERSION(1, 9, 0)
        case Media_Format_Y212:
#endif
        case Media_Format_Y210:
            aligned_width = (surface->iWidth + 1) >> 1;
            aligned_height = surface->iHeight * 2;
            aligned_format = Media_Format_Y216;
            break;
        case Media_Format_YUY2:
            aligned_width = (surface->iWidth + 1) >> 1;
            aligned_height = surface->iHeight * 2;
            break;
        case Media_Format_P016:
        case Media_Format_P012:
        case Media_Format_P010:
            aligned_height = surface->iHeight;
            aligned_width = surface->iWidth;
            if(entrypoint == VAEntrypointEncSlice)
            {
                aligned_width = surface->iWidth * 2;
                aligned_format = Media_Format_NV12;
            }
            else
            {
                aligned_format = Media_Format_P016;
            }
            break;
        default:
            return surface;
       }

    //create new dst surface and copy the structure
    PDDI_MEDIA_SURFACE dstSurface = (DDI_MEDIA_SURFACE *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE));

    MOS_SecureMemcpy(dstSurface,sizeof(DDI_MEDIA_SURFACE),surface,sizeof(DDI_MEDIA_SURFACE));
    DDI_CHK_NULL(dstSurface, "nullptr dstSurface", nullptr);

    dstSurface->uiVariantFlag = 1;
    dstSurface->format = aligned_format;
    dstSurface->iWidth = aligned_width;
    dstSurface->iHeight = aligned_height;
   //CreateNewSurface
    if(DdiMediaUtil_CreateSurface(dstSurface,mediaCtx) != VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(dstSurface);
        return surface;
    }
    //replace the surface
    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    surfaceElement = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surface->pMediaCtx->pSurfaceHeap->pHeapBase;
    surfaceElement += vaID;
    surfaceElement->pSurface = dstSurface;
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
    //FreeSurface
    DdiMediaUtil_FreeSurface(surface);
    MOS_FreeMemory(surface);

    return dstSurface;
}

DDI_MEDIA_BUFFER* DdiMedia_GetBufferFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID)
{
    uint32_t                       i = 0;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufHeapElement = nullptr;
    PDDI_MEDIA_BUFFER              buf = nullptr;

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

void* DdiMedia_GetContextFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID)
{
    uint32_t                       i;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufHeapElement;
    void *                         ctx;

    i                = (uint32_t)bufferID;
    DDI_CHK_LESS(i, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", nullptr);
    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    bufHeapElement  = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)mediaCtx->pBufferHeap->pHeapBase;
    bufHeapElement += bufferID;
    ctx            = bufHeapElement->pCtx;
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);

    return ctx;
}

int32_t DdiMedia_GetGpuPriority (VADriverContextP ctx, VABufferID *buffers, int32_t numBuffers, bool *updatePriority, int32_t *priority)
{
    void *        data;
    uint32_t      updateSessionPriority  = 0;
    uint32_t      priorityValue          = 0;
    int32_t       priorityIndexInBuf     = -1;

    DDI_CHK_NULL(ctx, "nullptr context in DdiMedia_GetGpuPriority!", -1);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", -1);
#if VA_CHECK_VERSION(1, 10, 0)
    for (int32_t i = 0; i < numBuffers; i++)
    {
        DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, buffers[i]);
        DDI_CHK_NULL(buf, "Invalid buffer.", -1);

        if((int32_t)buf->uiType == VAContextParameterUpdateBufferType)
        {
            //Read the priority from the VAContextParameterUpdateBuffer
            DdiMedia_MapBuffer(ctx, buffers[i], &data);
            DDI_CHK_NULL(data, "nullptr data.", -1);

            VAContextParameterUpdateBuffer *ContextParamBuf = (VAContextParameterUpdateBuffer *)data;
            DDI_CHK_NULL(ContextParamBuf, "nullptr ContextParamBuf.", -1);

            updateSessionPriority   = ContextParamBuf->flags.bits.context_priority_update;
            priorityValue           = ContextParamBuf->context_priority.bits.priority;

            if (updateSessionPriority)
            {
                *updatePriority = true;
                if(priorityValue <= CONTEXT_PRIORITY_MAX)
                {
                    *priority = priorityValue - CONTEXT_PRIORITY_MAX/2;
                }
                else
                {
                    *priority = 0;
                }
            }
            else
            {
                *updatePriority = false;
                *priority = 0;
            }

            DdiMedia_UnmapBuffer(ctx, buffers[i]);
            priorityIndexInBuf = i;
            break;
        }
    }
#endif
    return priorityIndexInBuf;
}

//Move the priority bufferID to the end of buffers 
void MovePriorityBufferIdToEnd (VABufferID *buffers, int32_t priorityIndexInBuf, int32_t numBuffers)
{
    VABufferID    vaBufferID = 0;
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
}

