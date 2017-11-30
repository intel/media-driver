/*
* Copyright (c) 2015-2017, Intel Corporation
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

static void* DdiMedia_GetVaContextFromHeap(PDDI_MEDIA_HEAP  pMediaHeap, uint32_t uiIndex, PMEDIA_MUTEX_T pMutex)
{
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT  pVaCtxHeapElmt;
    void                              *pContext;

    DdiMediaUtil_LockMutex(pMutex);
    if(nullptr == pMediaHeap || uiIndex >= pMediaHeap->uiAllocatedHeapElements)
    {
        DdiMediaUtil_UnLockMutex(pMutex);
        return nullptr;
    }
    pVaCtxHeapElmt  = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)pMediaHeap->pHeapBase;
    pVaCtxHeapElmt += uiIndex;
    pContext        = pVaCtxHeapElmt->pVaContext;
    DdiMediaUtil_UnLockMutex(pMutex);

    return pContext;
}

void DdiMedia_MediaSurfaceToMosResource(DDI_MEDIA_SURFACE *pMediaSurface, MOS_RESOURCE  *pMosResource)
{
    DDI_ASSERT(pMediaSurface->bo);

    switch (pMediaSurface->format)
    {
        case Media_Format_NV12:
            pMosResource->Format    = Format_NV12;
            break;
        case Media_Format_NV21:
            pMosResource->Format    = Format_NV21;
            break;
        case Media_Format_YUY2:
            pMosResource->Format    = Format_YUY2;
            break;
        case Media_Format_X8R8G8B8:
            pMosResource->Format    = Format_X8R8G8B8;
            break;
        case Media_Format_X8B8G8R8:
            pMosResource->Format    = Format_X8B8G8R8;
            break;
        case Media_Format_A8B8G8R8:
            pMosResource->Format    = Format_A8B8G8R8;
            break;
        case Media_Format_A8R8G8B8:
            pMosResource->Format    = Format_A8R8G8B8;
            break;
        case Media_Format_R5G6B5:
            pMosResource->Format    = Format_R5G6B5;
            break;
        case Media_Format_R8G8B8:
            pMosResource->Format    = Format_R8G8B8;
            break;
        case Media_Format_444P:
            pMosResource->Format    = Format_444P;
            break;
        case Media_Format_411P:
            pMosResource->Format    = Format_411P;
            break;
        case Media_Format_IMC3:
            pMosResource->Format    = Format_IMC3;
            break;
        case Media_Format_400P:
            pMosResource->Format    = Format_400P;
            break;
        case Media_Format_422H:
            pMosResource->Format    = Format_422H;
            break;
        case Media_Format_422V:
            pMosResource->Format    = Format_422V;
            break;
        case Media_Format_Buffer:
            pMosResource->Format    = Format_Any;
        case Media_Format_P010:
            pMosResource->Format    = Format_P010;
            break;
        case Media_Format_R10G10B10A2:
            pMosResource->Format    = Format_R10G10B10A2;
            break;
        case Media_Format_B10G10R10A2:
            pMosResource->Format    = Format_B10G10R10A2;
            break;
        case Media_Format_UYVY:
            pMosResource->Format    = Format_UYVY;
            break;
        default:
            DDI_ASSERTMESSAGE("DDI: unsupported media format for surface.");
            break;
    }
    pMosResource->iWidth    = pMediaSurface->iWidth;
    pMosResource->iHeight   = pMediaSurface->iHeight;
    pMosResource->iPitch    = pMediaSurface->iPitch;
    pMosResource->iCount    = pMediaSurface->iRefCount;
    pMosResource->isTiled   = pMediaSurface->isTiled;
    pMosResource->TileType  = LinuxToMosTileType(pMediaSurface->TileType);
    pMosResource->bo        = pMediaSurface->bo;
    pMosResource->name      = pMediaSurface->name;
    
    pMosResource->ppCurrentFrameSemaphore   = &pMediaSurface->pCurrentFrameSemaphore;
    pMosResource->ppReferenceFrameSemaphore = &pMediaSurface->pReferenceFrameSemaphore;
    pMosResource->bSemInitialized           = false;
    pMosResource->bMapped                   = false;

    if(pMediaSurface->bMapped == true)
    {
        pMosResource->pData     = pMediaSurface->pData;
    }
    else
    {
        pMosResource->pData     = nullptr;
    }
    pMosResource->pGmmResInfo   = pMediaSurface->pGmmResourceInfo;
    pMosResource->dwGfxAddress  = 0;

    // for MOS wrapper
    pMosResource->bConvertedFromDDIResource = true;

    Mos_Solo_SetOsResource(pMediaSurface->pGmmResourceInfo, pMosResource);

}

void DdiMedia_MediaBufferToMosResource(DDI_MEDIA_BUFFER *pMediaBuffer, MOS_RESOURCE *pMosResource)
{
    DDI_ASSERT(pMediaBuffer->bo);

    switch (pMediaBuffer->format)
    {
        case Media_Format_Buffer:
            pMosResource->Format  = Format_Buffer;
            pMosResource->iWidth    = pMediaBuffer->iSize;
            pMosResource->iHeight   = 1;
            pMosResource->iPitch    = pMediaBuffer->iSize;
            break;
        case Media_Format_Perf_Buffer:
            pMosResource->Format  = Format_Buffer;
            pMosResource->iWidth    = pMediaBuffer->iSize;
            pMosResource->iHeight   = 1;
            pMosResource->iPitch    = pMediaBuffer->iSize;
            break;
        case Media_Format_2DBuffer:
            pMosResource->Format  = Format_Buffer_2D;
            pMosResource->iWidth    = pMediaBuffer->iWidth;
            pMosResource->iHeight   = pMediaBuffer->iHeight;
            pMosResource->iPitch    = pMediaBuffer->iPitch;
            break;
        case Media_Format_CPU:
            return;
        default:
            pMosResource->iWidth    = pMediaBuffer->iSize;
            pMosResource->iHeight   = 1;
            pMosResource->iPitch    = pMediaBuffer->iSize;
            DDI_ASSERTMESSAGE("DDI: unsupported media format for surface.");
            break;
    }
    pMosResource->iCount    = pMediaBuffer->iRefCount;
    pMosResource->isTiled   = 0;
    pMosResource->TileType  = LinuxToMosTileType(pMediaBuffer->TileType);
    pMosResource->bo        = pMediaBuffer->bo;
    pMosResource->name      = pMediaBuffer->name;
    pMosResource->bMapped   = false;

    if(pMediaBuffer->bMapped == true)
    {
        pMosResource->pData     = pMediaBuffer->pData;
    }
    else
    {
        pMosResource->pData     = nullptr;
    }
    pMosResource->dwGfxAddress  = 0;
    pMosResource->pGmmResInfo   = pMediaBuffer->pGmmResourceInfo;

    // for MOS wrapper
    pMosResource->bConvertedFromDDIResource = true;	

    Mos_Solo_SetOsResource(pMediaBuffer->pGmmResourceInfo, pMosResource);
}

void* DdiMedia_GetContextFromContextID (VADriverContextP ctx, VAContextID vaCtxID, uint32_t *pCtxType)
{
    PDDI_MEDIA_CONTEXT       pMediaCtx;
    uint32_t                 uiIndex;

    pMediaCtx  = DdiMedia_GetMediaContext(ctx);
    uiIndex    = vaCtxID & DDI_MEDIA_MASK_VACONTEXTID;
    
    if (uiIndex >= DDI_MEDIA_MAX_INSTANCE_NUMBER)
        return nullptr;
    if ((vaCtxID&DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_CENC)
    {
        DDI_VERBOSEMESSAGE("Cenc context detected: 0x%x", vaCtxID);
        *pCtxType = DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER;
        return DdiMedia_GetVaContextFromHeap(pMediaCtx->pDecoderCtxHeap, uiIndex, &pMediaCtx->DecoderMutex);
    }
    else if ((vaCtxID&DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_DECODER)
    {
        DDI_VERBOSEMESSAGE("Decode context detected: 0x%x", vaCtxID);
        *pCtxType = DDI_MEDIA_CONTEXT_TYPE_DECODER;
        return DdiMedia_GetVaContextFromHeap(pMediaCtx->pDecoderCtxHeap, uiIndex, &pMediaCtx->DecoderMutex);
    }
    else if ((vaCtxID&DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_ENCODER)
    {
        *pCtxType = DDI_MEDIA_CONTEXT_TYPE_ENCODER;
        return DdiMedia_GetVaContextFromHeap(pMediaCtx->pEncoderCtxHeap, uiIndex, &pMediaCtx->EncoderMutex);
    }
    else if ((vaCtxID & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_VP)
    {
        *pCtxType = DDI_MEDIA_CONTEXT_TYPE_VP;
        return DdiMedia_GetVaContextFromHeap(pMediaCtx->pVpCtxHeap, uiIndex, &pMediaCtx->VpMutex);
    }
    else if ((vaCtxID & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_CM)
    {
        *pCtxType = DDI_MEDIA_CONTEXT_TYPE_CM;
        return DdiMedia_GetVaContextFromHeap(pMediaCtx->pCmCtxHeap, uiIndex, &pMediaCtx->CmMutex);
    }
    else if ((vaCtxID & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_MFE)
    {
        *pCtxType = DDI_MEDIA_CONTEXT_TYPE_MFE;
        return DdiMedia_GetVaContextFromHeap(pMediaCtx->pMfeCtxHeap, uiIndex, &pMediaCtx->MfeMutex);
    }
    else
    {
        DDI_ASSERTMESSAGE("Invalid context: 0x%x", vaCtxID);
        *pCtxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
        return nullptr;
    }

}

DDI_MEDIA_SURFACE* DdiMedia_GetSurfaceFromVASurfaceID (PDDI_MEDIA_CONTEXT pMediaCtx, VASurfaceID surfaceID)
{
    uint32_t                         i;
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  pSurfaceElement;
    PDDI_MEDIA_SURFACE               pSurface;

    i                = (uint32_t)surfaceID;
    DDI_CHK_LESS(i, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "invalid surface id", nullptr);
    DdiMediaUtil_LockMutex(&pMediaCtx->SurfaceMutex);
    pSurfaceElement  = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)pMediaCtx->pSurfaceHeap->pHeapBase;
    pSurfaceElement += i;
    pSurface         = pSurfaceElement->pSurface;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->SurfaceMutex);

    return pSurface;
}

DDI_MEDIA_BUFFER* DdiMedia_GetBufferFromVABufferID (PDDI_MEDIA_CONTEXT pMediaCtx, VABufferID bufferID)
{
    uint32_t                       i;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT pBufHeapElement;
    PDDI_MEDIA_BUFFER              pBuf;

    i                = (uint32_t)bufferID;
    DDI_CHK_LESS(i, pMediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", nullptr);
    DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
    pBufHeapElement  = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)pMediaCtx->pBufferHeap->pHeapBase;
    pBufHeapElement += i;
    pBuf             = pBufHeapElement->pBuffer;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);

    return pBuf;
}

bool DdiMedia_DestroyBufFromVABufferID (PDDI_MEDIA_CONTEXT pMediaCtx, VABufferID bufferID)
{
    DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
    DdiMediaUtil_ReleasePMediaBufferFromHeap(pMediaCtx->pBufferHeap, bufferID);
    pMediaCtx->uiNumBufs--;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
    return true;
}



