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
//! \file     media_libva.cpp
//! \brief    libva(and its extension) interface implementaion.
//!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef ANDROID
#include <va/va_android.h>
#include <ufo/gralloc.h>
#endif


#ifndef ANDROID
#include <X11/Xutil.h>
#endif

#include <linux/fb.h>

#include "media_libva_util.h"
#include "media_libva_decoder.h"
#include "media_libva_encoder.h"
#ifndef ANDROID
#include "media_libva_putsurface_linux.h"
#else
#include "media_libva_putsurface_android.h"
#endif
#include "media_libva_vp.h"
#include "mos_os.h"

#include "hwinfo_linux.h"
#include "codechal_memdecomp.h"
#include "mos_solo_generic.h"
#include "media_libva_caps.h"
#include "media_interfaces_mmd.h"
#include "mos_util_user_interface.h"

#ifdef __cplusplus
extern "C" {
#endif
extern VAStatus DdiDestroyContextCM (VADriverContextP   pVaDrvCtx, VAContextID         vaCtxID);
#ifdef __cplusplus
}
#endif

VAProcFilterType vp_supported_filters[DDI_VP_MAX_NUM_FILTERS] = {
    VAProcFilterNoiseReduction,
    VAProcFilterDeinterlacing,
    VAProcFilterSharpening,
    VAProcFilterColorBalance,
    VAProcFilterSkinToneEnhancement,
    VAProcFilterTotalColorCorrection
};

VAProcColorStandardType vp_input_color_std[DDI_VP_NUM_INPUT_COLOR_STD] = {
    VAProcColorStandardBT601,
    VAProcColorStandardBT709,
    VAProcColorStandardSRGB,
    VAProcColorStandardSTRGB
};

VAProcColorStandardType vp_output_color_std[DDI_VP_NUM_OUT_COLOR_STD] = {
    VAProcColorStandardBT601,
    VAProcColorStandardBT709,
    VAProcColorStandardSRGB,
    VAProcColorStandardSTRGB
};

uint32_t vp_query_surface_attr[DDI_CODEC_NUM_QUERY_ATTR_VP] = {
    VA_FOURCC('I', '4', '2', '0'),
    VA_FOURCC('Y', 'V', '1', '2'),
    VA_FOURCC('Y', 'U', 'Y', '2'),
    VA_FOURCC('4', '2', '2', 'H'),
    VA_FOURCC('4', '2', '2', 'V'),
    VA_FOURCC('R', 'G', 'B', 'A'),
    VA_FOURCC('B', 'G', 'R', 'A'),
    VA_FOURCC('R', 'G', 'B', 'X'),
    VA_FOURCC('P', '0', '1', '0')
};

static VAStatus DdiMedia_DestroyContext (
    VADriverContextP    ctx,
    VAContextID         context);

// Making this API public since media_libva_vp.c calls this
VAStatus DdiMedia_MapBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id,    /* in */
    void                **pbuf     /* out */
);

VAStatus DdiMedia_UnmapBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id    /* in */
);

VAStatus DdiMedia_DestroyBuffer (
    VADriverContextP    ctx,
    VABufferID          buffer_id
);

VAStatus DdiMedia_DestroyImage (
    VADriverContextP ctx,
    VAImageID        image
);

static PDDI_MEDIA_CONTEXT DdiMedia_CreateMediaDriverContext()
{
    PDDI_MEDIA_CONTEXT   pMediaCtx;

    pMediaCtx = (PDDI_MEDIA_CONTEXT)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_CONTEXT));

    return pMediaCtx;
}

// refine this for decoder later
static bool DdiMedia_ReleaseSliceControlBuffer(
    uint32_t          uiCtxType,
    void             *pCtx,
    DDI_MEDIA_BUFFER *pBuf)
{
    DDI_CODEC_COM_BUFFER_MGR   *pBufMgr;
    DDI_UNUSED(pBuf);

    switch (uiCtxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        case DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER:
        {
            PDDI_DECODE_CONTEXT  pDecCtx;

            pDecCtx = DdiDecode_GetDecContextFromPVOID(pCtx);
            pBufMgr = &(pDecCtx->BufMgr);

            switch (pDecCtx->wMode)
            {
                case CODECHAL_DECODE_MODE_AVCVLD:
                    if(pDecCtx->bShortFormatInUse)
                    {
                        if(pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264Base == nullptr)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if(pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264 == nullptr)
                        {
                            return false;
                        }
                    }
                    break;
                case CODECHAL_DECODE_MODE_MPEG2VLD:
                    if(pBufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 == nullptr)
                    {
                        return false;
                    }
                    break;
                case CODECHAL_DECODE_MODE_VC1VLD:
                    if(pBufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 == nullptr)
                    {
                        return false;
                    }
                    break;
                case CODECHAL_DECODE_MODE_JPEG:
                    if(pBufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG == nullptr)
                    {
                        return false;
                    }
                    break;
                case CODECHAL_DECODE_MODE_VP8VLD:
                    if(pBufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8 == nullptr)
                    {
                        return false;
                    }
                    break;
                case CODECHAL_DECODE_MODE_HEVCVLD:
                    if(pDecCtx->bShortFormatInUse)
                    {
                        if(pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC == nullptr)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if(pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC == nullptr)
                        {
                            return false;
                        }
                    }
                    break;
                case CODECHAL_DECODE_MODE_VP9VLD:
                    if(pBufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9 == nullptr)
                    {
                        return false;
                    }
                    break;
                default:
                    return false;
            }
            break;
        }
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            break;
        default:
            break;
    }

    return true;
}

static bool DdiMedia_ReleaseBpBuffer(
    DDI_CODEC_COM_BUFFER_MGR   *pBufMgr,
    DDI_MEDIA_BUFFER           *pBuf)
{
    DDI_UNUSED(pBufMgr);
    DDI_UNUSED(pBuf);
    return true;
}

static bool DdiMedia_ReleaseBsBuffer(
    DDI_CODEC_COM_BUFFER_MGR   *pBufMgr,
    DDI_MEDIA_BUFFER           *pBuf)
{
    int32_t i;

    if ((pBufMgr == nullptr) || (pBuf == nullptr))
    {
        return true;
    }

    if (pBuf->format == Media_Format_CPU)
    {
        for(i = 0; i < pBufMgr->dwNumSliceData; i++)
        {
            if(pBufMgr->pSliceData[i].pBaseAddress == pBuf->pData)
            {
                DdiMediaUtil_FreeBuffer(pBuf);
                pBufMgr->pSliceData[i].pBaseAddress = nullptr;
                if (pBufMgr->pSliceData[i].pMappedGPUBuffer != nullptr)
                {
                    DdiMediaUtil_UnlockBuffer(pBufMgr->pSliceData[i].pMappedGPUBuffer);
                    if (pBufMgr->pSliceData[i].pMappedGPUBuffer->bMapped == false)
                    {
                        DdiMediaUtil_FreeBuffer(pBufMgr->pSliceData[i].pMappedGPUBuffer);
                        MOS_FreeMemory(pBufMgr->pSliceData[i].pMappedGPUBuffer);
                    }
                }
                MOS_ZeroMemory((void*)(&(pBufMgr->pSliceData[i])), sizeof(pBufMgr->pSliceData[0]));
                pBufMgr->dwNumSliceData --;
                return true;
            }
        }
        return false;
    }
    return true;
}

static uint32_t DdiMedia_CreateRenderTarget(
    PDDI_MEDIA_CONTEXT            pMediaDrvCtx,
    DDI_MEDIA_FORMAT              mediaFormat,
    uint32_t                      dwWidth,
    uint32_t                      dwHeight,
    DDI_MEDIA_SURFACE_DESCRIPTOR *pSurfDesc,
    uint32_t                      surfaceUsageHint
)
{
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT  pSurfaceElement;
    uint32_t                         uiSurfaceID;

    DdiMediaUtil_LockMutex(&pMediaDrvCtx->SurfaceMutex);

    pSurfaceElement = DdiMediaUtil_AllocPMediaSurfaceFromHeap(pMediaDrvCtx->pSurfaceHeap);
    if (nullptr == pSurfaceElement)
    {
        DdiMediaUtil_UnLockMutex(&pMediaDrvCtx->SurfaceMutex);
        return VA_INVALID_ID;
    }

    pSurfaceElement->pSurface = (DDI_MEDIA_SURFACE *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE));
    if (nullptr == pSurfaceElement->pSurface)
    {
        DdiMediaUtil_ReleasePMediaSurfaceFromHeap(pMediaDrvCtx->pSurfaceHeap, pSurfaceElement->uiVaSurfaceID);
        DdiMediaUtil_UnLockMutex(&pMediaDrvCtx->SurfaceMutex);
        return VA_INVALID_ID;
    }

    pSurfaceElement->pSurface->pMediaCtx       = pMediaDrvCtx;
    pSurfaceElement->pSurface->iWidth          = dwWidth;
    pSurfaceElement->pSurface->iHeight         = dwHeight;
    pSurfaceElement->pSurface->pSurfDesc       = pSurfDesc;
    pSurfaceElement->pSurface->format          = mediaFormat;
    pSurfaceElement->pSurface->uiLockedBufID   = VA_INVALID_ID;
    pSurfaceElement->pSurface->uiLockedImageID = VA_INVALID_ID;
    pSurfaceElement->pSurface->surfaceUsageHint= surfaceUsageHint;

    if(DdiMediaUtil_CreateSurface(pSurfaceElement->pSurface, pMediaDrvCtx)!= VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(pSurfaceElement->pSurface);
        DdiMediaUtil_ReleasePMediaSurfaceFromHeap(pMediaDrvCtx->pSurfaceHeap, pSurfaceElement->uiVaSurfaceID);
        DdiMediaUtil_UnLockMutex(&pMediaDrvCtx->SurfaceMutex);
        return VA_INVALID_ID;
    }

    pMediaDrvCtx->uiNumSurfaces++;
    uiSurfaceID = pSurfaceElement->uiVaSurfaceID;
    DdiMediaUtil_UnLockMutex(&pMediaDrvCtx->SurfaceMutex);
    return uiSurfaceID;
}

VAStatus
DdiMedia_HybridQueryBufferAttributes (
    VADisplay    dpy,
    VAContextID  context,
    VABufferType bufferType,
    void        *outputData,
    uint32_t    *outputDataLen
)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMedia_SetFrameID(
    VADisplay    dpy,
    VASurfaceID  surface,
    uint32_t     frame_id
)
{
    VADriverContextP              ctx;
    PDDI_MEDIA_CONTEXT            pMediaCtx;
    DDI_MEDIA_SURFACE             *pSurface;
    MOS_STATUS                    eStatus = MOS_STATUS_SUCCESS;

    ctx = (((VADisplayContextP)dpy)->pDriverContext);
    DDI_CHK_NULL(ctx, "Null ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx           = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    pSurface     = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, surface);
    DDI_CHK_NULL(pSurface, "Null pSurface.", VA_STATUS_ERROR_INVALID_PARAMETER);

    pSurface->frame_idx = frame_id;

    return VA_STATUS_SUCCESS;
}

/*
 * vaCreateSurfaces - Create an array of surfaces used for decode and display
 *  dpy: display
 *  width: surface width
 *  height: surface height
 *  format: VA_RT_FORMAT_YUV420, VA_RT_FORMAT_YUV422 or VA_RT_FORMAT_YUV444
 *  num_surfaces: number of surfaces to be created
 *  surfaces: array of surfaces created upon return
 */
static DDI_MEDIA_FORMAT  DdiMedia_VaFmtToMediaFmt (int32_t format, int32_t fourcc)
{
    switch (format)
    {
        case VA_RT_FORMAT_YUV420:
            return (fourcc == VA_FOURCC('P','0','1','0')) ? Media_Format_P010 : Media_Format_NV12;
        case VA_RT_FORMAT_YUV422:
            return Media_Format_422H;
        case VA_RT_FORMAT_YUV444:
            return Media_Format_444P;
        case VA_RT_FORMAT_YUV400:
            return Media_Format_400P;
        case VA_RT_FORMAT_YUV411:
            return Media_Format_411P;
        default:
            return Media_Format_Count;
    }
}

int32_t DdiMedia_MediaFormatToOsFormat(DDI_MEDIA_FORMAT format)
{
    switch (format)
    {
        case Media_Format_X8R8G8B8:
            return VA_FOURCC_XRGB;
        case Media_Format_X8B8G8R8:
            return VA_FOURCC_XBGR;
        case Media_Format_A8B8G8R8:
        case Media_Format_R10G10B10A2:
            return VA_FOURCC_ABGR;
        case Media_Format_A8R8G8B8:
        case Media_Format_B10G10R10A2:
            return VA_FOURCC_ARGB;
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
        case Media_Format_Buffer:
            return VA_FOURCC_P208;
        case Media_Format_P010:
            return VA_FOURCC('P','0','1','0');
        default:
            return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }
}

DDI_MEDIA_FORMAT DdiMedia_OsFormatToMediaFormat(int32_t fourcc, int32_t RTFormatType)
{
    switch (fourcc)
    {
        case VA_FOURCC_BGRA:
        case VA_FOURCC_ARGB:
#ifdef VA_RT_FORMAT_RGB32_10BPP
            if(VA_RT_FORMAT_RGB32_10BPP == RTFormatType)
            {
                return Media_Format_B10G10R10A2;
            }
#endif
            return Media_Format_A8R8G8B8;
        case VA_FOURCC_RGBA:
        case VA_FOURCC_ABGR:
#ifdef VA_RT_FORMAT_RGB32_10BPP
            if(VA_RT_FORMAT_RGB32_10BPP == RTFormatType)
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
        case VA_FOURCC_RGBP:
        case VA_FOURCC_BGRP:
            return Media_Format_444P;
        case VA_FOURCC_P208:
            return Media_Format_Buffer;
        case VA_FOURCC('P','0','1','0'):
            return Media_Format_P010;
        default:
            return Media_Format_Count;
    }
}

DDI_MEDIA_FORMAT DdiMedia_OsFormatAlphaMaskToMediaFormat(int32_t fourcc, int32_t alphaMask)
{
    switch (fourcc)
    {
        case VA_FOURCC_BGRA:
        case VA_FOURCC_ARGB:
            if(RGB_10BIT_ALPHAMASK == alphaMask)
            {
                return Media_Format_B10G10R10A2;
            }
            else
            {
                return Media_Format_A8R8G8B8;
            }
        case VA_FOURCC_RGBA:
        case VA_FOURCC_ABGR:
            if(RGB_10BIT_ALPHAMASK == alphaMask)
            {
                return Media_Format_R10G10B10A2;
            }
            else
            {
                return Media_Format_A8B8G8R8;
            }
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
        case VA_FOURCC_YV12:
            return Media_Format_YV12;
        case VA_FOURCC_IYUV:
            return Media_Format_IYUV;
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
        case VA_FOURCC_RGBP:
        case VA_FOURCC_BGRP:
            return Media_Format_444P;
        case VA_FOURCC_P208:
            return Media_Format_Buffer;
        case VA_FOURCC('P','0','1','0'):
            return Media_Format_P010;
        default:
            return Media_Format_Count;
    }
}

#ifndef ANDROID
// move from media_libva_putsurface_linux.c
static unsigned long DdiMedia_mask2shift(unsigned long mask)
{
    unsigned long shift;

    shift = 0;
    while((mask & 0x1) == 0)
    {
        mask = mask >> 1;
        shift++;
    }
    return shift;
}
static void DdiMedia_yuv2pixel(uint32_t *pixel, int32_t y, int32_t u, int32_t v,
                               unsigned long rshift, unsigned long rmask,
                               unsigned long gshift, unsigned long gmask,
                               unsigned long bshift, unsigned long bmask)
{
    int32_t r, g, b;
    /* Warning, magic values ahead */
    r = y + ((351 * (v-128)) >> 8);
    g = y - (((179 * (v-128)) + (86 * (u-128))) >> 8);
    b = y + ((444 * (u-128)) >> 8);

    if (r > 255) r = 255;
    if (g > 255) g = 255;
    if (b > 255) b = 255;
    if (r < 0)   r = 0;
    if (g < 0)   g = 0;
    if (b < 0)   b = 0;

    *pixel = (uint32_t)(((r << rshift) & rmask) | ((g << gshift) & gmask) |((b << bshift) & bmask));
}


#define YUV_444P_TO_ARGB() \
    pSrcY = pUmdContextY + pitch * srcy;\
    pSrcU = pSrcY + pitch * ((pMediaSurface->iHeight));\
    pSrcV = pSrcU + pitch * ((pMediaSurface->iHeight));\
     \
    for(y = srcy; y < (srcy + height); y += 1) \
    {\
        for(x = srcx; x < (srcx + width); x += 1) \
        {\
            y1 = *(pSrcY + x); \
            u1 = *(pSrcU + x);\
            v1 = *(pSrcV + x);\
            \
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + (x * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
           \
        }\
        pSrcY += pitch;\
        pSrcU += pitch;\
        pSrcV += pitch;\
    }

#define YUV_422H_TO_ARGB()\
    pSrcY = pUmdContextY + pitch * srcy;\
    pSrcU = pSrcY + pitch * pMediaSurface->iHeight;\
    pSrcV = pSrcU + pitch * pMediaSurface->iHeight;\
    \
    for(y = srcy; y < (srcy + height); y += 1)\
    {\
        for(x = srcx; x < (srcx + width); x += 2)\
        {\
            y1 = *(pSrcY + x);\
            y2 = *(pSrcY + x + 1);\
            u1 = *(pSrcU + x / 2);\
            v1 = *(pSrcV + x / 2);\
            \
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + (x * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + ((x + 1) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
        }\
        pSrcY += pitch;\
        pSrcU += pitch;\
        pSrcV += pitch;\
    }

#define YUV_422V_TO_ARGB() \
    pSrcY = pUmdContextY + pitch * srcy;\
    pSrcU = pSrcY + pitch * pMediaSurface->iHeight;\
    pSrcV = pSrcU + pitch * pMediaSurface->iHeight / 2;\
    \
    for(y = srcy; y < (srcy + width); y += 1)\
    {\
        for(x = srcx; x < (srcx + height); x += 2)\
        {\
            y1 = *(pSrcY + x * pitch);\
            y2 = *(pSrcY + (x + 1) * pitch);\
            u1 = *(pSrcU + (x / 2) * pitch);\
            v1 = *(pSrcV + (x / 2) * pitch);\
            \
            pPixel = (uint32_t *)(pXimg->data + (x * pXimg->bytes_per_line) + (y * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + (x* pXimg->bytes_per_line) + ((y + 1) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            \
        }\
        \
        pSrcY += 1;\
        pSrcU += 1;\
        pSrcV += 1;\
    }

#define YUV_IMC3_TO_ARGB() \
    pSrcY = pUmdContextY + pitch * srcy;\
    pSrcU = pSrcY + pitch * pMediaSurface->iHeight;\
    pSrcV = pSrcU + pitch * pMediaSurface->iHeight / 2;\
    \
    for(y = srcy; y < (srcy + height); y += 2) \
    {\
        for(x = srcx; x < (srcx + width); x += 2) \
        {\
            y1 = *(pSrcY + x);\
            y2 = *(pSrcY + x + 1);\
            y3 = *(pSrcY + x + pitch);\
            y4 = *(pSrcY + x + pitch + 1);\
            \
            u1 = *(pSrcU + x / 2);\
            v1 = *(pSrcV + x / 2);\
            \
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + (x * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + ((x + 1) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + ((y + 1) * pXimg->bytes_per_line) + (x * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y3, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + ((y + 1) * pXimg->bytes_per_line) + ((x + 1) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y4, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask); \
        }\
        pSrcY += pitch * 2;\
        pSrcU += pitch;\
        pSrcV += pitch;\
    }

#define YUV_411P_TO_ARGB() \
    pSrcY = pUmdContextY + pitch * srcy;\
    pSrcU = pSrcY + pitch * pMediaSurface->iHeight;\
    pSrcV = pSrcU + pitch * pMediaSurface->iHeight;\
    \
    for(y = srcy; y < (srcy + height); y += 1)\
    {\
        for(x = srcx; x < (srcx + width); x += 4)\
        {\
            y1 = *(pSrcY + x);\
            y2 = *(pSrcY + x + 1);\
            y3 = *(pSrcY + x + 2);\
            y4 = *(pSrcY + x + 3);\
            \
            u1 = *(pSrcU + x / 4);\
            v1 = *(pSrcV + x / 4);\
            \
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + (x * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + ((x + 1) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + ((y ) * pXimg->bytes_per_line) + ((x+2) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y3, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + ((y) * pXimg->bytes_per_line) + ((x + 3) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y4, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
        }\
        pSrcY  += pitch;\
        pSrcU  += pitch;\
        pSrcV  += pitch;\
    }

#define YUV_400P_TO_ARGB()\
    pSrcY = pUmdContextY + pitch * srcy;\
    pSrcU = pSrcY;\
    pSrcV = pSrcY;\
    \
    for(y = srcy; y < (srcy + height); y += 2)\
    {\
        for(x = srcx; x < (srcx + width); x += 2)\
        {\
            y1 = *(pSrcY + x);\
            y2 = *(pSrcY + x + 1);\
            y3 = *(pSrcY + x + pitch);\
            y4 = *(pSrcY + x + pitch + 1);\
            \
            u1 = 128;\
            v1 = 128;\
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + (x * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + ((x + 1) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + ((y + 1) * pXimg->bytes_per_line) + (x * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y3, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + ((y + 1) * pXimg->bytes_per_line) + ((x + 1) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y4, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
        }\
        pSrcY += pitch * 2;\
        pSrcU += pitch;\
        pSrcV += pitch;\
    }

#define YUV_NV12_TO_ARGB()\
    pSrcY = pUmdContextY + pitch * srcy;\
    pSrcU = pSrcY + pitch * pMediaSurface->iHeight;\
    pSrcV = pSrcU + 1;\
    \
    for(y = srcy; y < (srcy + height); y += 2)\
    {\
        for(x = srcx; x < (srcx + width); x += 2)\
        {\
            y1 = *(pSrcY + x);\
            y2 = *(pSrcY + x + 1);\
            y3 = *(pSrcY + x + pitch);\
            y4 = *(pSrcY + x + pitch + 1);\
            \
            u1 = *(pSrcU + x);\
            v1 = *(pSrcU + x +1);\
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + (x * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y1, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + (y * pXimg->bytes_per_line) + ((x + 1) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y2, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + ((y + 1) * pXimg->bytes_per_line) + (x * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y3, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
            pPixel = (uint32_t *)(pXimg->data + ((y + 1) * pXimg->bytes_per_line) + ((x + 1) * (pXimg->bits_per_pixel >> 3)));\
            DdiMedia_yuv2pixel(pPixel, y4, u1, v1, rshift, rmask, gshift, gmask, bshift, bmask);\
        }\
        pSrcY += pitch * 2;\
        pSrcU += pitch;\
    }

VAStatus DdiMedia_PutSurfaceLinuxSW(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void*            draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,        /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
)
{
    GC               gc;
    XImage          *pXimg = nullptr;
    Visual          *visual;
    uint16_t         width, height;
    int32_t          depth;
    int32_t          x, y;
    uint8_t         *pSurface = nullptr;
    uint32_t        *pPixel;
    uint8_t         *pSrcY;
    uint8_t         *pSrcU;
    uint8_t         *pSrcV;
    int32_t          y1, y2, y3, y4, u1, v1;

    PDDI_MEDIA_CONTEXT             pMediaCtx;
    DDI_MEDIA_SURFACE             *pMediaSurface;
    uint8_t                       *ptr = nullptr;
    int32_t                        pitch =0;
    uint8_t                       *pUmdContextY = nullptr;
    unsigned long                  rmask, gmask, bmask;
    unsigned long                  rshift, gshift, bshift;
    uint32_t                       uiAdjustU     = 1;
    uint32_t                       uiAdjustD     = 1;
    uint32_t                       uiSurfaceSize = 0;
    uint8_t                       *pDispTempBuffer;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    TypeXCreateGC                  pfn_XCreateGC     = nullptr;
    TypeXFreeGC                    pfn_XFreeGC       = nullptr;
    TypeXCreateImage               pfn_XCreateImage  = nullptr;
    TypeXDestroyImage              pfn_XDestroyImage = nullptr;
    TypeXPutImage                  pfn_XPutImage     = nullptr;

    pMediaCtx        = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(pMediaCtx->X11FuncTable, "Null X11FuncTable", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->X11FuncTable->pfnXCreateGC, "Null pfnXCreateGC", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->X11FuncTable->pfnXFreeGC, "Null pfnXFreeGC", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->X11FuncTable->pfnXCreateImage, "Null pfnXCreateImage", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->X11FuncTable->pfnXDestroyImage, "Null pfnXDestroyImage", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->X11FuncTable->pfnXPutImage, "Null pfnXPutImage", VA_STATUS_ERROR_INVALID_CONTEXT);

    pfn_XCreateGC     = (TypeXCreateGC)(pMediaCtx->X11FuncTable->pfnXCreateGC);
    pfn_XFreeGC       = (TypeXFreeGC)(pMediaCtx->X11FuncTable->pfnXFreeGC);
    pfn_XCreateImage  = (TypeXCreateImage)(pMediaCtx->X11FuncTable->pfnXCreateImage);
    pfn_XDestroyImage = (TypeXDestroyImage)(pMediaCtx->X11FuncTable->pfnXDestroyImage);
    pfn_XPutImage     = (TypeXPutImage)(pMediaCtx->X11FuncTable->pfnXPutImage);

    pMediaSurface     = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, surface);
    DDI_CHK_NULL(pMediaSurface, "Null pMediaSurface.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (srcw <= destw)
        width = srcw;
    else
        width = destw;

    if (srch <= desth)
        height = srch;
    else
        height = desth;

    pitch      = pMediaSurface->iPitch;
    switch(pMediaSurface->format)
    {
        case Media_Format_422H:
        case Media_Format_444P:
        case Media_Format_411P:
            uiAdjustU = 3;
            uiAdjustD = 1;
            break;
        case Media_Format_400P:
            uiAdjustU = 1;
            uiAdjustD = 1;
            break;
        case Media_Format_422V:
        case Media_Format_IMC3:
            uiAdjustU = 2;
            uiAdjustD = 1;
            break;
        case Media_Format_NV12:
            uiAdjustU = 3;
            uiAdjustD = 2;
            break;
        default:
            DDI_ASSERTMESSAGE("Color Format is not supported: %d",pMediaSurface->format);
            return VA_STATUS_ERROR_INVALID_VALUE;
    }
    ptr                    = (uint8_t*)DdiMediaUtil_LockSurface(pMediaSurface, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));

    uiSurfaceSize          = pitch * pMediaSurface->iHeight * uiAdjustU / uiAdjustD;
    pDispTempBuffer        = (uint8_t *)malloc(uiSurfaceSize);
    if (pDispTempBuffer == nullptr)
    {
        DdiMediaUtil_UnlockSurface(pMediaSurface);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    pUmdContextY = pDispTempBuffer;
    eStatus = MOS_SecureMemcpy(pUmdContextY, uiSurfaceSize, ptr, uiSurfaceSize);
    DDI_CHK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "DDI:Failed to copy surface buffer data!", VA_STATUS_ERROR_OPERATION_FAILED);
    pSurface     = pUmdContextY;
    visual       = DefaultVisual(ctx->native_dpy, ctx->x11_screen);
    gc           = (*pfn_XCreateGC)((Display*)ctx->native_dpy, (Drawable)draw, 0, nullptr);
    depth        = DefaultDepth(ctx->native_dpy, ctx->x11_screen);

    if (TrueColor != visual->c_class)
    {
        (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
        MOS_FreeMemory(pDispTempBuffer);
        return VA_STATUS_ERROR_UNKNOWN;
    }

    rmask  = visual->red_mask;
    gmask  = visual->green_mask;
    bmask  = visual->blue_mask;

    rshift = DdiMedia_mask2shift(rmask);
    gshift = DdiMedia_mask2shift(gmask);
    bshift = DdiMedia_mask2shift(bmask);

    pXimg   = (*pfn_XCreateImage)((Display*)ctx->native_dpy, visual, depth, ZPixmap, 0, nullptr,width, height, 32, 0 );
    if (pXimg == nullptr)
    {
        MOS_FreeMemory(pDispTempBuffer);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (pXimg->bits_per_pixel != 32)
    {
         (*pfn_XDestroyImage)(pXimg);
         (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
         MOS_FreeMemory(pDispTempBuffer);
         return VA_STATUS_ERROR_UNKNOWN;
    }

    pXimg->data = (char *) malloc(pXimg->bytes_per_line * MOS_ALIGN_CEIL(height, 2)); // If height is odd, need to add it by one for we process two lines per iteration
    if (nullptr == pXimg->data)
    {
        (*pfn_XDestroyImage)(pXimg);
        (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
        MOS_FreeMemory(pDispTempBuffer);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

     switch(pMediaSurface->format)
    {
        case Media_Format_444P:
            YUV_444P_TO_ARGB();
            break;
        case Media_Format_422H:
            YUV_422H_TO_ARGB();
            break;
        case Media_Format_422V:
            YUV_422V_TO_ARGB();
            break;
        case Media_Format_IMC3:
            YUV_IMC3_TO_ARGB();
            break;
        case Media_Format_411P:
            YUV_411P_TO_ARGB();
            break;
        case Media_Format_400P:
            YUV_400P_TO_ARGB();
            break;
        case Media_Format_NV12:
            YUV_NV12_TO_ARGB();
            break;
        default:
            DDI_ASSERTMESSAGE("Color Format is not supported: %d", pMediaSurface->format);
    }

    DdiMediaUtil_UnlockSurface(pMediaSurface);

    (*pfn_XPutImage)((Display*)ctx->native_dpy,(Drawable)draw, gc, pXimg, 0, 0, destx, desty, destw, desth);

    if (pXimg != nullptr)
    {
        (*pfn_XDestroyImage)(pXimg);
    }
    (*pfn_XFreeGC)((Display*)ctx->native_dpy, gc);
    MOS_FreeMemory(pDispTempBuffer);
    return VA_STATUS_SUCCESS;
}

static VAStatus DdiMedia_PutSurfaceDummy(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void            *draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,        /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

#define X11_LIB_NAME "libX11.so"

/*
 * Close opened libX11.so lib, free related function table.
 */
static void DdiMedia_DestroyX11Connection(
    PDDI_MEDIA_CONTEXT pMediaCtx
)
{
    if (nullptr == pMediaCtx || nullptr == pMediaCtx->X11FuncTable)
    {
        return;
    }

    MOS_FreeLibrary(pMediaCtx->X11FuncTable->pX11LibHandle);
    MOS_FreeMemory(pMediaCtx->X11FuncTable);
    pMediaCtx->X11FuncTable = nullptr;

    return;
}

/*
 * dlopen libX11.so, setup the function table, which is used by
 * DdiCodec_PutSurface (Linux) so far.
 */
static VAStatus DdiMedia_ConnectX11(
    PDDI_MEDIA_CONTEXT pMediaCtx
)
{
    HMODULE    h_module   = nullptr;
    MOS_STATUS mos_status = MOS_STATUS_UNKNOWN;
    void      *p_sym      = nullptr;

    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx->X11FuncTable = (PDDI_X11_FUNC_TABLE)MOS_AllocAndZeroMemory(sizeof(DDI_X11_FUNC_TABLE));
    DDI_CHK_NULL(pMediaCtx->X11FuncTable, "Allocation Failed for X11FuncTable", VA_STATUS_ERROR_ALLOCATION_FAILED);

    mos_status = MOS_LoadLibrary(X11_LIB_NAME, &h_module);
    if (MOS_STATUS_SUCCESS != mos_status || nullptr == h_module)
    {
        goto err_finish;
    }

    pMediaCtx->X11FuncTable->pX11LibHandle = h_module;

    pMediaCtx->X11FuncTable->pfnXCreateGC =
        MOS_GetProcAddress(h_module, "XCreateGC");
    pMediaCtx->X11FuncTable->pfnXFreeGC =
        MOS_GetProcAddress(h_module, "XFreeGC");
    pMediaCtx->X11FuncTable->pfnXCreateImage =
        MOS_GetProcAddress(h_module, "XCreateImage");
    pMediaCtx->X11FuncTable->pfnXDestroyImage =
        MOS_GetProcAddress(h_module, "XDestroyImage");
    pMediaCtx->X11FuncTable->pfnXPutImage =
        MOS_GetProcAddress(h_module, "XPutImage");

    if (nullptr == pMediaCtx->X11FuncTable->pfnXCreateGC     ||
        nullptr == pMediaCtx->X11FuncTable->pfnXFreeGC       ||
        nullptr == pMediaCtx->X11FuncTable->pfnXCreateImage  ||
        nullptr == pMediaCtx->X11FuncTable->pfnXDestroyImage ||
        nullptr == pMediaCtx->X11FuncTable->pfnXPutImage)
    {
        goto err_finish;
    }

    return VA_STATUS_SUCCESS;

err_finish:
    DdiMedia_DestroyX11Connection(pMediaCtx);
    return VA_STATUS_ERROR_OPERATION_FAILED;
}
#endif


/////////////////////////////////////////////////////////////////////////////
//! \Free allocated surfaceheap elements
//! \params
//! [in] PDDI_MEDIA_CONTEXT
//! [out] none
//! \returns
/////////////////////////////////////////////////////////////////////////////
static void DdiMedia_FreeSurfaceHeapElements(PDDI_MEDIA_CONTEXT pMediaCtx)
{
    PDDI_MEDIA_HEAP                 pSurfaceHeap;
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT pMediaSurfaceHeapBase;
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT pMediaSurfaceHeapElmt;
    int32_t                         uiElementId;
    int32_t                         uiSurfaceNums;

    if (nullptr == pMediaCtx)
        return;
    pSurfaceHeap = pMediaCtx->pSurfaceHeap;

    if (nullptr == pSurfaceHeap)
        return;

    pMediaSurfaceHeapBase = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)pSurfaceHeap->pHeapBase;
    if (nullptr == pMediaSurfaceHeapBase)
        return;

    uiSurfaceNums = pMediaCtx->uiNumSurfaces;
    for (uiElementId = 0; uiElementId < uiSurfaceNums; uiElementId++)
    {
        pMediaSurfaceHeapElmt = &pMediaSurfaceHeapBase[uiElementId];
        if (nullptr == pMediaSurfaceHeapElmt->pSurface)
            continue;

        DdiMediaUtil_FreeSurface(pMediaSurfaceHeapElmt->pSurface);
        MOS_FreeMemory(pMediaSurfaceHeapElmt->pSurface);
        DdiMediaUtil_ReleasePMediaSurfaceFromHeap(pSurfaceHeap,pMediaSurfaceHeapElmt->uiVaSurfaceID);
        pMediaCtx->uiNumSurfaces--;
    }
}

/////////////////////////////////////////////////////////////////////////////
//! \Free allocated bufferheap elements
//! \params
//! [in] VADriverContextP
//! [out] none
//! \returns
/////////////////////////////////////////////////////////////////////////////
static void DdiMedia_FreeBufferHeapElements(VADriverContextP    ctx)
{
    PDDI_MEDIA_CONTEXT             pMediaCtx;
    PDDI_MEDIA_HEAP                pBufferHeap;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT pMediaBufferHeapElmt;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT pMediaBufferHeapBase;
    int32_t                        uiElementId;
    int32_t                        uiBufNums;

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    if (nullptr == pMediaCtx)
        return;

    pBufferHeap = pMediaCtx->pBufferHeap;
    if (nullptr == pBufferHeap)
        return;

    pMediaBufferHeapBase = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)pBufferHeap->pHeapBase;
    if (nullptr == pMediaBufferHeapBase)
        return;

    uiBufNums = pMediaCtx->uiNumBufs;
    for (uiElementId = 0; uiElementId < uiBufNums; ++uiElementId)
    {
        pMediaBufferHeapElmt = &pMediaBufferHeapBase[uiElementId];
        if (nullptr == pMediaBufferHeapElmt->pBuffer)
            continue;
        DdiMedia_DestroyBuffer(ctx,pMediaBufferHeapElmt->uiVaBufferID);
    }
}

/////////////////////////////////////////////////////////////////////////////
//! \Free allocated Imageheap elements
//! \params
//! [in] VADriverContextP
//! [out] none
//! \returns
/////////////////////////////////////////////////////////////////////////////
static void DdiMedia_FreeImageHeapElements(VADriverContextP    ctx)
{
    PDDI_MEDIA_CONTEXT             pMediaCtx;
    PDDI_MEDIA_HEAP                pImageHeap;
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT  pMediaImageHeapElmt;
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT  pMediaImageHeapBase;
    int32_t                        uiElementId;
    int32_t                        uiImageNums;

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    if (nullptr == pMediaCtx)
        return;

    pImageHeap = pMediaCtx->pImageHeap;
    if (nullptr == pImageHeap)
        return;

    pMediaImageHeapBase = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)pImageHeap->pHeapBase;
    if (nullptr == pMediaImageHeapBase)
        return;

    uiImageNums = pMediaCtx->uiNumImages;
    for (uiElementId = 0; uiElementId < uiImageNums; ++uiElementId)
    {
        pMediaImageHeapElmt = &pMediaImageHeapBase[uiElementId];
        if (nullptr == pMediaImageHeapElmt->pImage)
            continue;
        DdiMedia_DestroyImage(ctx,pMediaImageHeapElmt->uiVaImageID);
    }
}

/////////////////////////////////////////////////////////////////////////////
//! \Execute free allocated bufferheap elements for FreeContextHeapElements function
//! \params
//! [in] VADriverContextP
//! [in] PDDI_MEDIA_HEAP
//! [out] none
//! \returns
/////////////////////////////////////////////////////////////////////////////
static void DdiMedia_FreeContextHeap(VADriverContextP ctx, PDDI_MEDIA_HEAP pContextHeap,int32_t VaContextOffset, int32_t uiCtxNums)
{
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT  pMediaContextHeapBase;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT  pMediaContextHeapElmt;
    int32_t                            uiElementId;
    VAContextID                        uiVaCtxID;

    pMediaContextHeapBase = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)pContextHeap->pHeapBase;
    if (nullptr == pMediaContextHeapBase)
        return;

    for (uiElementId = 0; uiElementId < uiCtxNums; ++uiElementId)
    {
        pMediaContextHeapElmt = &pMediaContextHeapBase[uiElementId];
        if (nullptr == pMediaContextHeapElmt->pVaContext)
            continue;
        uiVaCtxID = (VAContextID)(pMediaContextHeapElmt->uiVaContextID + VaContextOffset);
        DdiMedia_DestroyContext(ctx,uiVaCtxID);
    }

}

/////////////////////////////////////////////////////////////////////////////
//! \Free allocated contextheap elements
//! \params
//! [in] VADriverContextP
//! [out] none
//! \returns
/////////////////////////////////////////////////////////////////////////////
static void DdiMedia_FreeContextHeapElements(VADriverContextP    ctx)
{
    PDDI_MEDIA_CONTEXT     pMediaCtx;
    PDDI_MEDIA_HEAP        pEncoderContextHeap;
    PDDI_MEDIA_HEAP        pDecoderContextHeap;
    PDDI_MEDIA_HEAP        pVpContextHeap;
    PDDI_MEDIA_HEAP        pMfeContextHeap;
    int32_t                uiEncCtxNums;
    int32_t                uiDecCtxNums;
    int32_t                uiVPCtxNums;
    int32_t                uiMfeCtxNums;

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    if (nullptr == pMediaCtx)
        return;

    //Free EncoderContext
    pEncoderContextHeap = pMediaCtx->pEncoderCtxHeap;
    uiEncCtxNums        = pMediaCtx->uiNumEncoders;
    if (nullptr != pEncoderContextHeap)
        DdiMedia_FreeContextHeap(ctx,pEncoderContextHeap,DDI_MEDIA_VACONTEXTID_OFFSET_ENCODER,uiEncCtxNums);

    //Free DecoderContext
    pDecoderContextHeap = pMediaCtx->pDecoderCtxHeap;
    uiDecCtxNums        = pMediaCtx->uiNumDecoders;
    if (nullptr != pDecoderContextHeap)
        DdiMedia_FreeContextHeap(ctx,pDecoderContextHeap,DDI_MEDIA_VACONTEXTID_OFFSET_DECODER,uiDecCtxNums);

    //Free VpContext
    pVpContextHeap = pMediaCtx->pVpCtxHeap;
    uiVPCtxNums    = pMediaCtx->uiNumVPs;
    if (nullptr != pVpContextHeap)
        DdiMedia_FreeContextHeap(ctx,pVpContextHeap,DDI_MEDIA_VACONTEXTID_OFFSET_VP,uiVPCtxNums);

    //Free MfeContext
    pMfeContextHeap = pMediaCtx->pMfeCtxHeap;
    uiMfeCtxNums    = pMediaCtx->uiNumMfes;
    if (nullptr != pMfeContextHeap)
        DdiMedia_FreeContextHeap(ctx, pMfeContextHeap, DDI_MEDIA_VACONTEXTID_OFFSET_MFE, uiMfeCtxNums);

    // Free media memory decompression data structure
    if (pMediaCtx->pMediaMemDecompState)
    {
        MediaMemDecompState *pMediaMemCompState = 
            static_cast<MediaMemDecompState*>(pMediaCtx->pMediaMemDecompState);
        MOS_Delete(pMediaMemCompState);
        pMediaCtx->pMediaMemDecompState = nullptr;
    }
}

/////////////////////////////////////////////////////////////////////////////
//! \Free allocated ContextCM elements
//! \params
//! [in] VADriverContextP
//! [out] none
//! \returns
/////////////////////////////////////////////////////////////////////////////
static void DdiMedia_FreeContextCMElements(VADriverContextP ctx)
{
    PDDI_MEDIA_CONTEXT     pMediaCtx;
    VAContextID            vaCtxID;
    int32_t                uiElementId;
    int32_t                uiCMNums;

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    if (nullptr == pMediaCtx)
        return;

    uiCMNums = pMediaCtx->uiNumCMs;
    for (uiElementId = 0; uiElementId < uiCMNums; uiElementId++)
    {
        vaCtxID = uiElementId + DDI_MEDIA_VACONTEXTID_OFFSET_CM;
        DdiDestroyContextCM(ctx,vaCtxID);
    }
}

VAImage* DdiMedia_GetVAImageFromVAImageID (PDDI_MEDIA_CONTEXT pMediaCtx, VAImageID ImageID)
{
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT pImageElement;
    uint32_t                      i;
    VAImage                      *pVaImage;

    i               = (uint32_t)ImageID;
    DDI_CHK_LESS(i, pMediaCtx->pImageHeap->uiAllocatedHeapElements, "invalid image id", nullptr);
    DdiMediaUtil_LockMutex(&pMediaCtx->ImageMutex);
    pImageElement   = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)pMediaCtx->pImageHeap->pHeapBase;
    pImageElement  += i;
    pVaImage        = pImageElement->pImage;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->ImageMutex);

    return pVaImage;
}
void* DdiMedia_GetCtxFromVABufferID (PDDI_MEDIA_CONTEXT pMediaCtx, VABufferID bufferID)
{
    uint32_t                       i;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT pBufHeapElement;
    void                          *pTemp;

    i                = (uint32_t)bufferID;
    DDI_CHK_LESS(i, pMediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", nullptr);
    DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
    pBufHeapElement  = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)pMediaCtx->pBufferHeap->pHeapBase;
    pBufHeapElement += i;
    pTemp            = pBufHeapElement->pCtx;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);

    return pTemp;
}

uint32_t DdiMedia_GetCtxTypeFromVABufferID (PDDI_MEDIA_CONTEXT pMediaCtx, VABufferID bufferID)
{
    uint32_t                       i;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT pBufHeapElement;
    uint32_t                       uiCtxType;

    i                = (uint32_t)bufferID;
    DDI_CHK_LESS(i, pMediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", DDI_MEDIA_CONTEXT_TYPE_NONE);
    DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
    pBufHeapElement  = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)pMediaCtx->pBufferHeap->pHeapBase;
    pBufHeapElement += i;
    uiCtxType        = pBufHeapElement->uiCtxType;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);

    return uiCtxType;

}

bool DdiMedia_DestroyImageFromVAImageID (PDDI_MEDIA_CONTEXT pMediaCtx, VAImageID ImageID)
{
    DdiMediaUtil_LockMutex(&pMediaCtx->ImageMutex);
    DdiMediaUtil_ReleasePVAImageFromHeap(pMediaCtx->pImageHeap, (uint32_t)ImageID);
    pMediaCtx->uiNumImages--;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->ImageMutex);
    return true;
}
#ifdef _MMC_SUPPORTED
void DdiMedia_MediaMemoryDecompressInternal(PMOS_CONTEXT pMosCtx, PMOS_RESOURCE pOsResource)
{
    MediaMemDecompState* pMediaMemDecompState = nullptr;

    DDI_ASSERT(pMosCtx);
    DDI_ASSERT(pMosCtx->ppMediaMemDecompState);
    DDI_ASSERT(pOsResource);

    pMediaMemDecompState = static_cast<MediaMemDecompState*>(*pMosCtx->ppMediaMemDecompState);

    if (!pMediaMemDecompState)
    {
        pMediaMemDecompState =
            static_cast<MediaMemDecompState*>(MmdDevice::CreateFactory(pMosCtx));
    }

    if (pMediaMemDecompState)    
    {
        pMediaMemDecompState->MemoryDecompress(pOsResource);
    }
    else
    {
        DDI_ASSERTMESSAGE("Invalid memory decompression state.");
    }
}
#endif
////////////////////////////////////////////////////////////////////////////////
//! \purpose
//!  Decompress a compressed surface.
//! \params
//! [in]     pMediaCtx : Media driver context
//! [in]     pMediaSurface : Media surface
//! [out]    None
//! \returns VA_STATUS_SUCCESS if call succeeds
////////////////////////////////////////////////////////////////////////////////
VAStatus DdiMedia_MediaMemoryDecompress(PDDI_MEDIA_CONTEXT pMediaCtx, DDI_MEDIA_SURFACE *pMediaSurface)
{
#ifdef ANDROID
    intel_ufo_bo_datatype_t datatype;
#endif
    MOS_CONTEXT             MosCtx;
    MOS_RESOURCE            resSurface;

    DDI_ASSERT(pMediaSurface);
#ifdef _MMC_SUPPORTED
#ifdef ANDROID
    DDI_ASSERT(pMediaSurface->bo);
    mos_bo_get_datatype(pMediaSurface->bo, &datatype.value);
    if ((MOS_MEMCOMP_STATE)datatype.compression_mode != MOS_MEMCOMP_DISABLED)
#else
    if (GmmResIsMediaMemoryCompressed(pMediaSurface->pGmmResourceInfo, 0))
#endif
    {
        MosCtx.ppMediaMemDecompState = &pMediaCtx->pMediaMemDecompState;
        DdiMedia_MediaSurfaceToMosResource(pMediaSurface, &resSurface);
        DdiMedia_MediaMemoryDecompressInternal(&MosCtx, &resSurface);
    }
    return VA_STATUS_SUCCESS;
#else
    return VA_STATUS_ERROR_INVALID_SURFACE;
#endif
}

/*
 * Initialize the library
 */

// Global mutex
MEDIA_MUTEX_T  GlobalMutex = MEDIA_MUTEX_INITIALIZER;

VAStatus DdiMedia__Initialize (
    VADriverContextP ctx,
    int32_t         *major_version,     /* out */
    int32_t         *minor_version      /* out */
)
{
    struct drm_state            *pDRMState;
    PDDI_MEDIA_CONTEXT           pMediaCtx;
    PLATFORM                     platform;
    MEDIA_FEATURE_TABLE         *skuTable;
    MEDIA_WA_TABLE              *waTable;
    GMM_SKU_FEATURE_TABLE        gmmSkuTable;
    GMM_WA_TABLE                 gmmWaTable;
    GMM_GT_SYSTEM_INFO           gmmGtInfo;
    
    VAStatus                     vaStatus;
    GMM_STATUS                   gmmStatus;
    int32_t                      iDevicefd;
    MOS_STATUS                   eStatus;

#ifdef ANDROID
    // ATRACE code in <cutils/trace.h> started from KitKat, version = 440
    // ENABLE_TRACE is defined only for eng build so release build won't leak perf data
    // thus trace code enabled only on KitKat (and newer) && eng build
#if ANDROID_VERSION > 439 && defined(ENABLE_ATRACE)
    char switch_value[PROPERTY_VALUE_MAX];

    property_get("debug.DdiCodec_.umd", switch_value, "0");
    atrace_switch = atoi(switch_value);
#endif
#endif

    vaStatus  = VA_STATUS_SUCCESS;
    gmmStatus = GMM_ERROR;

    DDI_CHK_NULL(ctx,          "Null ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    pDRMState = (struct drm_state *)ctx->drm_state;
    DDI_CHK_NULL(pDRMState,    "Null pDRMState", VA_STATUS_ERROR_INVALID_CONTEXT);

    // If libva failes to open the graphics card, try to open it again within Media Driver
    if(pDRMState->fd < 0 || pDRMState->fd == 0 )
    {
        DDI_ASSERTMESSAGE("DDI:LIBVA Wrapper doesn't pass file descriptor for graphics adaptor, trying to open the graphics... ");
        pDRMState->fd = DdiMediaUtil_OpenGraphicsAdaptor((char *)DEVICE_NAME);
        if (pDRMState->fd < 0) {
            DDI_ASSERTMESSAGE("DDI: Still failed to open the graphic adaptor, return failure");
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }
    iDevicefd = pDRMState->fd;

    if(major_version)
    {
        *major_version = VA_MAJOR_VERSION;
    }

    if(minor_version)
    {
        *minor_version = VA_MINOR_VERSION;
    }

    DdiMediaUtil_LockMutex(&GlobalMutex);
    // media context is already created, return directly to support multiple entry
    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    if(pMediaCtx)
    {
        pMediaCtx->uiRef++;
        vaStatus = VA_STATUS_SUCCESS;
        goto finish;
    }

    MOS_utilities_init();

    pMediaCtx = DdiMedia_CreateMediaDriverContext();
    if (nullptr == pMediaCtx)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    pMediaCtx->uiRef++;

    // Heap initialization here
    pMediaCtx->pSurfaceHeap                         = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    if (nullptr == pMediaCtx->pSurfaceHeap)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    pMediaCtx->pSurfaceHeap->uiHeapElementSize      = sizeof(DDI_MEDIA_SURFACE_HEAP_ELEMENT);

    pMediaCtx->pBufferHeap                          = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    if (nullptr == pMediaCtx->pBufferHeap)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    pMediaCtx->pBufferHeap->uiHeapElementSize       = sizeof(DDI_MEDIA_BUFFER_HEAP_ELEMENT);

    pMediaCtx->pImageHeap                           = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    if (nullptr == pMediaCtx->pImageHeap)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    pMediaCtx->pImageHeap->uiHeapElementSize        = sizeof(DDI_MEDIA_IMAGE_HEAP_ELEMENT);

    pMediaCtx->pDecoderCtxHeap                      = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    if (nullptr == pMediaCtx->pDecoderCtxHeap)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    pMediaCtx->pDecoderCtxHeap->uiHeapElementSize   = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    pMediaCtx->pEncoderCtxHeap                      = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    if (nullptr == pMediaCtx->pEncoderCtxHeap)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    pMediaCtx->pEncoderCtxHeap->uiHeapElementSize   = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    pMediaCtx->pVpCtxHeap                           = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    if (nullptr == pMediaCtx->pVpCtxHeap)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    pMediaCtx->pVpCtxHeap->uiHeapElementSize        = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    pMediaCtx->pCmCtxHeap                          = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    if (nullptr == pMediaCtx->pCmCtxHeap)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    pMediaCtx->pCmCtxHeap->uiHeapElementSize        = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    pMediaCtx->pMfeCtxHeap                           = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    if (nullptr == pMediaCtx->pMfeCtxHeap)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }
    pMediaCtx->pMfeCtxHeap->uiHeapElementSize        = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    // Allocate memory for Media System Info
    pMediaCtx->pGtSystemInfo                        = (MEDIA_SYSTEM_INFO *)MOS_AllocAndZeroMemory(sizeof(MEDIA_SYSTEM_INFO));
    if(nullptr == pMediaCtx->pGtSystemInfo)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto finish;
    }

    pMediaCtx->fd         = iDevicefd;
    pMediaCtx->pDrmBufMgr = mos_bufmgr_gem_init(pMediaCtx->fd, DDI_CODEC_BATCH_BUFFER_SIZE);
    if( nullptr == pMediaCtx->pDrmBufMgr)
    {
        DDI_ASSERTMESSAGE("DDI:No able to allocate buffer manager, fd=0x%d", pMediaCtx->fd);
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
        goto finish;
    }
    mos_bufmgr_gem_enable_reuse(pMediaCtx->pDrmBufMgr);

    //Latency reducation:replace HWGetDeviceID to get device using ioctl from drm.
    pMediaCtx->iDeviceId = mos_bufmgr_gem_get_devid(pMediaCtx->pDrmBufMgr);

    skuTable = &pMediaCtx->SkuTable;
    waTable  = &pMediaCtx->WaTable;
    skuTable->reset();
    waTable->reset();

    // get Sku/Wa tables and platform information
    DDI_CHK_STATUS_MESSAGE(
        HWInfo_GetGfxInfo(pMediaCtx->fd, &platform, skuTable, waTable, pMediaCtx->pGtSystemInfo),
        "Fatal error - unsuccesfull Sku/Wa/GtSystemInfo initialization"
    );

    MosUtilUserInterfaceInit(platform.eProductFamily);

    pMediaCtx->platform = platform;
 
    if (MEDIA_IS_SKU(skuTable, FtrEnableMediaKernels) == 0)
    {
        MEDIA_WR_WA(waTable, WaHucStreamoutOnlyDisable, 0);
    }

    DdiMediaUtil_GetEnabledFeature(pMediaCtx);

    pMediaCtx->m_caps = MediaLibvaCaps::CreateMediaLibvaCaps(pMediaCtx);
    if (!pMediaCtx->m_caps)
    {
        DDI_ASSERTMESSAGE("Caps init failed. Not supported GFX device.");
        vaStatus = VA_STATUS_ERROR_OPERATION_FAILED;
        goto finish;
    }

#ifdef _MMC_SUPPORTED
    pMediaCtx->pfnMemoryDecompress = DdiMedia_MediaMemoryDecompressInternal;
#endif
    // init the mutexs
    DdiMediaUtil_InitMutex(&pMediaCtx->SurfaceMutex);
    DdiMediaUtil_InitMutex(&pMediaCtx->BufferMutex);
    DdiMediaUtil_InitMutex(&pMediaCtx->ImageMutex);
    DdiMediaUtil_InitMutex(&pMediaCtx->DecoderMutex);
    DdiMediaUtil_InitMutex(&pMediaCtx->EncoderMutex);
    DdiMediaUtil_InitMutex(&pMediaCtx->VpMutex);
    DdiMediaUtil_InitMutex(&pMediaCtx->CmMutex);
    DdiMediaUtil_InitMutex(&pMediaCtx->MfeMutex);
#ifndef ANDROID
    DdiMediaUtil_InitMutex(&pMediaCtx->PutSurfaceRenderMutex);
    DdiMediaUtil_InitMutex(&pMediaCtx->PutSurfaceSwapBufferMutex);

    // try to open X11 lib, if fail, assume no X11 environment
    vaStatus = DdiMedia_ConnectX11(pMediaCtx);
    if (VA_STATUS_SUCCESS != vaStatus)
    {
        // assume no X11 environment. In current implementation,
        // PutSurface (Linux) needs X11 support, so just replace
        // it with a dummy version. DdiCodec_PutSurfaceDummy() will
        // return VA_STATUS_ERROR_UNIMPLEMENTED directly.
        ctx->vtable->vaPutSurface = DdiMedia_PutSurfaceDummy;
    }
#endif

    ctx->pDriverData  = (void*)pMediaCtx;
    pMediaCtx->bIsAtomSOC = IS_ATOMSOC(pMediaCtx->iDeviceId);

#ifndef ANDROID
    output_dri_init(ctx);
#endif

    eStatus = Mos_Solo_DdiInitializeDeviceId(
                 (void*)pMediaCtx->pDrmBufMgr,
                 &pMediaCtx->SkuTable,
                 &pMediaCtx->WaTable,
                 &pMediaCtx->iDeviceId,
                 &pMediaCtx->fd);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
	    vaStatus = VA_STATUS_ERROR_OPERATION_FAILED;
        goto finish;
    }

    memset(&gmmSkuTable, 0, sizeof(gmmSkuTable));
    memset(&gmmWaTable, 0, sizeof(gmmWaTable));
    memset(&gmmGtInfo, 0, sizeof(gmmGtInfo));
    DDI_CHK_STATUS_MESSAGE(
        HWInfo_GetGmmInfo(pMediaCtx->fd, &gmmSkuTable, &gmmWaTable, &gmmGtInfo),
        "Fatal error - unsuccesfull Gmm Sku/Wa/GtSystemInfo initialization"
    );

    // init GMM context
    gmmStatus = GmmInitGlobalContext(pMediaCtx->platform,
                                     &gmmSkuTable,
                                     &gmmWaTable,
                                     &gmmGtInfo,
                                     (GMM_CLIENT)GMM_LIBVA_LINUX);

    if(gmmStatus != GMM_SUCCESS)
    {
        DDI_ASSERTMESSAGE("gmm init failed.");
        vaStatus = VA_STATUS_ERROR_OPERATION_FAILED;
        goto finish;
    }

    DdiMediaUtil_UnLockMutex(&GlobalMutex);

    return VA_STATUS_SUCCESS;

finish:
    DdiMediaUtil_UnLockMutex(&GlobalMutex);

    if (pMediaCtx)
    {
        pMediaCtx->SkuTable.reset();
        pMediaCtx->WaTable.reset();
        MOS_FreeMemory(pMediaCtx->pSurfaceHeap);
        MOS_FreeMemory(pMediaCtx->pBufferHeap);
        MOS_FreeMemory(pMediaCtx->pImageHeap);
        MOS_FreeMemory(pMediaCtx->pDecoderCtxHeap);
        MOS_FreeMemory(pMediaCtx->pEncoderCtxHeap);
        MOS_FreeMemory(pMediaCtx->pVpCtxHeap);
        MOS_FreeMemory(pMediaCtx->pMfeCtxHeap);
        MOS_FreeMemory(pMediaCtx);
    }

    return vaStatus;
}

/*
 * After this call, all library internal resources will be cleaned up
 */
static VAStatus DdiMedia_Terminate (
    VADriverContextP ctx
)
{
    PDDI_MEDIA_CONTEXT pMediaCtx;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,       "Null ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DdiMediaUtil_LockMutex(&GlobalMutex);

#ifndef ANDROID
    DdiMedia_DestroyX11Connection(pMediaCtx);

    if (pMediaCtx->m_caps)
    {
        if (pMediaCtx->dri_output != nullptr) {
            if (pMediaCtx->dri_output->handle)
                dso_close(pMediaCtx->dri_output->handle);

            free(pMediaCtx->dri_output);
            pMediaCtx->dri_output = nullptr;
        }
    }
#endif

    //destory resources
    DdiMedia_FreeSurfaceHeapElements(pMediaCtx);
    DdiMedia_FreeBufferHeapElements(ctx);
    DdiMedia_FreeImageHeapElements(ctx);
    DdiMedia_FreeContextHeapElements(ctx);
    DdiMedia_FreeContextCMElements(ctx);

    if (pMediaCtx->uiRef > 1)
    {
        pMediaCtx->uiRef--;
        DdiMediaUtil_UnLockMutex(&GlobalMutex);

        return VA_STATUS_SUCCESS;
    }

    pMediaCtx->SkuTable.reset();
    pMediaCtx->WaTable.reset();
    // destroy libdrm buffer manager
    mos_bufmgr_destroy(pMediaCtx->pDrmBufMgr);

    // destroy heaps
    MOS_FreeMemory(pMediaCtx->pSurfaceHeap->pHeapBase);
    MOS_FreeMemory(pMediaCtx->pSurfaceHeap);

    MOS_FreeMemory(pMediaCtx->pBufferHeap->pHeapBase);
    MOS_FreeMemory(pMediaCtx->pBufferHeap);

    MOS_FreeMemory(pMediaCtx->pImageHeap->pHeapBase);
    MOS_FreeMemory(pMediaCtx->pImageHeap);

    MOS_FreeMemory(pMediaCtx->pDecoderCtxHeap->pHeapBase);
    MOS_FreeMemory(pMediaCtx->pDecoderCtxHeap);

    MOS_FreeMemory(pMediaCtx->pEncoderCtxHeap->pHeapBase);
    MOS_FreeMemory(pMediaCtx->pEncoderCtxHeap);

    MOS_FreeMemory(pMediaCtx->pVpCtxHeap->pHeapBase);
    MOS_FreeMemory(pMediaCtx->pVpCtxHeap);

    MOS_FreeMemory(pMediaCtx->pCmCtxHeap->pHeapBase);
    MOS_FreeMemory(pMediaCtx->pCmCtxHeap);

    MOS_FreeMemory(pMediaCtx->pMfeCtxHeap->pHeapBase);
    MOS_FreeMemory(pMediaCtx->pMfeCtxHeap);

    // Destroy memory allocated to store Media System Info
    MOS_FreeMemory(pMediaCtx->pGtSystemInfo);

    // destroy the mutexs
    DdiMediaUtil_DestroyMutex(&pMediaCtx->SurfaceMutex);
    DdiMediaUtil_DestroyMutex(&pMediaCtx->BufferMutex);
    DdiMediaUtil_DestroyMutex(&pMediaCtx->ImageMutex);
    DdiMediaUtil_DestroyMutex(&pMediaCtx->DecoderMutex);
    DdiMediaUtil_DestroyMutex(&pMediaCtx->EncoderMutex);
    DdiMediaUtil_DestroyMutex(&pMediaCtx->VpMutex);
    DdiMediaUtil_DestroyMutex(&pMediaCtx->CmMutex);
    DdiMediaUtil_DestroyMutex(&pMediaCtx->MfeMutex);

    //resource checking
    if (pMediaCtx->uiNumSurfaces != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the surfaces.");
    }
    if (pMediaCtx->uiNumBufs != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the buffers.");
    }
    if (pMediaCtx->uiNumImages != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the images.");
    }
    if (pMediaCtx->uiNumDecoders != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the decoders.");
    }
    if (pMediaCtx->uiNumEncoders != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the encoders.");
    }
    if (pMediaCtx->uiNumVPs != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the VPs.");
    }
    if (pMediaCtx->uiNumCMs != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the CMs.");
    }

    if (pMediaCtx->m_caps)
    {
        MOS_Delete(pMediaCtx->m_caps);
        pMediaCtx->m_caps = nullptr;
    }

    // release media driver context
    MOS_FreeMemory(pMediaCtx);

    ctx->pDriverData = nullptr;
    MOS_utilities_close();

    // Free GMM memory.
    GmmDestroyGlobalContext();

    DdiMediaUtil_UnLockMutex(&GlobalMutex);

    return VA_STATUS_SUCCESS;
}

/*
 * Query supported entrypoints for a given profile
 * The caller must provide an "entrypoint_list" array that can hold at
 * least vaMaxNumEntrypoints() entries. The actual number of entrypoints
 * returned in "entrypoint_list" is returned in "num_entrypoints".
 */
static VAStatus DdiMedia_QueryConfigEntrypoints(
    VADriverContextP    ctx,
    VAProfile           profile,
    VAEntrypoint       *entrypoint_list,
    int32_t            *num_entrypoints
)
{
    uint32_t              uiProfileTblIdx;
    int32_t               n;
    PDDI_MEDIA_CONTEXT    pMediaCtx;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "Null Ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->m_caps, "Null m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(entrypoint_list, "Null entrypoint_list", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(num_entrypoints, "Null num_entrypoints", VA_STATUS_ERROR_INVALID_PARAMETER);

    return pMediaCtx->m_caps->QueryConfigEntrypoints(profile,
            entrypoint_list, num_entrypoints);
}

/*
 * Query supported profiles
 * The caller must provide a "profile_list" array that can hold at
 * least vaMaxNumProfile() entries. The actual number of profiles
 * returned in "profile_list" is returned in "num_profile".
 */
static VAStatus DdiMedia_QueryConfigProfiles (
    VADriverContextP    ctx,
    VAProfile          *profile_list,
    int32_t            *num_profiles
)
{
    uint32_t    uiProfileTblIdx;
    uint32_t    uiNumProfiles = 0;
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT    pMediaCtx;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "Null Ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    pMediaCtx = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->m_caps, "Null m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(profile_list, "Null profile_list", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(num_profiles, "Null num_profiles", VA_STATUS_ERROR_INVALID_PARAMETER);

    return pMediaCtx->m_caps->QueryConfigProfiles(profile_list, num_profiles);
}

/*
 * Query all attributes for a given configuration
 * The profile of the configuration is returned in "profile"
 * The entrypoint of the configuration is returned in "entrypoint"
 * The caller must provide an "attrib_list" array that can hold at least
 * vaMaxNumConfigAttributes() entries. The actual number of attributes
 * returned in "attrib_list" is returned in "num_attribs"
 */
static VAStatus DdiMedia_QueryConfigAttributes (
    VADriverContextP    ctx,
    VAConfigID          config_id,
    VAProfile          *profile,
    VAEntrypoint       *entrypoint,
    VAConfigAttrib     *attrib_list,
    int32_t            *num_attribs
)
{
    PDDI_MEDIA_CONTEXT      pMediaCtx;
    uint32_t                uiNumAttribs = 0;
    VAConfigAttrib         *pAttribList = attrib_list;
    bool                    bROIBRCPriorityLevelSupport = false;
    bool                    bROIBRCQpDeltaSupport       = false;
    int32_t                 iMaxNumOfROI = 0;
    VAStatus                vaStatus = VA_STATUS_SUCCESS;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(profile,     "Null profile", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypoint,  "Null entrypoint", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ctx,         "Null Ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(num_attribs, "Null num_attribs", VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,   "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->m_caps, "Null m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return pMediaCtx->m_caps->QueryConfigAttributes(
                config_id, profile, entrypoint, attrib_list, num_attribs);
}

/*
 * Create a configuration for the encode/decode/vp pipeline
 * it passes in the attribute list that specifies the attributes it cares
 * about, with the rest taking default values.
 */
static VAStatus DdiMedia_CreateConfig (
    VADriverContextP    ctx,
    VAProfile           profile,
    VAEntrypoint        entrypoint,
    VAConfigAttrib     *attrib_list,
    int32_t             num_attribs,
    VAConfigID         *config_id
)
{
    PDDI_MEDIA_CONTEXT  pMediaCtx;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,       "Null ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(config_id, "Null config_id", VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->m_caps, "Null m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return pMediaCtx->m_caps->CreateConfig(
            profile, entrypoint, attrib_list, num_attribs, config_id);
}

/*
 * Free resources associated with a given config
 */
static VAStatus DdiMedia_DestroyConfig (
    VADriverContextP    ctx,
    VAConfigID          config_id
)
{
    DDI_FUNCTION_ENTER();

    PDDI_MEDIA_CONTEXT pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->m_caps, "Null m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return pMediaCtx->m_caps->DestroyConfig(config_id);
}

/*
 * Get attributes for a given profile/entrypoint pair
 * The caller must provide an "attrib_list" with all attributes to be
 * retrieved.  Upon return, the attributes in "attrib_list" have been
 * updated with their value.  Unknown attributes or attributes that are
 * not supported for the given profile/entrypoint pair will have their
 * value set to VA_ATTRIB_NOT_SUPPORTED
 */
static VAStatus DdiMedia_GetConfigAttributes(
    VADriverContextP    ctx,
    VAProfile           profile,
    VAEntrypoint        entrypoint,
    VAConfigAttrib     *attrib_list,
    int32_t             num_attribs
)
{
    int32_t                      i;
    PDDI_MEDIA_CONTEXT           pMediaCtx;
    MOS_USER_FEATURE             UserFeature;
    MOS_USER_FEATURE_VALUE       UserFeatureValue;
    int32_t                      iMaxNumOfROI = 0;
    bool                         bROIBRCPriorityLevelSupport = false;
    bool                         bROIBRCQpDeltaSupport       = false;

    DDI_FUNCTION_ENTER();

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->m_caps, "Null m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return pMediaCtx->m_caps->GetConfigAttributes(
            profile, entrypoint, attrib_list, num_attribs);
}


static VAStatus DdiMedia_CreateSurfaces (
    VADriverContextP    ctx,
    int32_t             width,
    int32_t             height,
    int32_t             format,
    int32_t             num_surfaces,
    VASurfaceID        *surfaces
)
{
    PDDI_MEDIA_CONTEXT            pMediaDrvCtx;
    DDI_MEDIA_FORMAT              mediaFmt;
    int32_t                       i;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,               "Null ctx",             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LARGER(num_surfaces, 0, "Invalid num_surfaces", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(surfaces,          "Null surfaces",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(width,        0, "Invalid width",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(height,       0, "Invalid height",       VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaDrvCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaDrvCtx,       "Null pMediaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    if( format != VA_RT_FORMAT_YUV420 ||
        format != VA_RT_FORMAT_YUV422 ||
        format != VA_RT_FORMAT_YUV444 ||
        format != VA_RT_FORMAT_YUV400 ||
        format != VA_RT_FORMAT_YUV411)
    {
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    mediaFmt = DdiMedia_VaFmtToMediaFmt(format, VA_FOURCC_NV12);

    height = MOS_ALIGN_CEIL(height, 16);
    for(i = 0; i < num_surfaces; i++)
    {
        VASurfaceID vaSurfaceID;

        vaSurfaceID = (VASurfaceID)DdiMedia_CreateRenderTarget(pMediaDrvCtx, mediaFmt, width, height, nullptr, VA_SURFACE_ATTRIB_USAGE_HINT_GENERIC);
        if (VA_INVALID_ID != vaSurfaceID)
            surfaces[i] = vaSurfaceID;
        else
        {
            // here to release the successful allocated surfaces?
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }

    return VA_STATUS_SUCCESS;
}

/*
 * vaDestroySurfaces - Destroy resources associated with surfaces.
 *  Surfaces can only be destroyed after the context associated has been
 *  destroyed.
 *  dpy: display
 *  surfaces: array of surfaces to destroy
 *  num_surfaces: number of surfaces in the array to be destroyed.
 */
static VAStatus DdiMedia_DestroySurfaces (
    VADriverContextP    ctx,
    VASurfaceID        *surfaces,
    int32_t             num_surfaces
)
{
    PDDI_MEDIA_CONTEXT              pMediaCtx;
    int32_t                         i;
    PDDI_MEDIA_SURFACE              pSurface = nullptr;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,             "Null ctx",             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LARGER(num_surfaces, 0, "Invalid num_surfaces", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(surfaces,        "Null surfaces",        VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,                  "Null pMediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->pSurfaceHeap,    "Null pMediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    for(i = 0; i < num_surfaces; i++)
    {
        DDI_CHK_LESS((uint32_t)surfaces[i], pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaces", VA_STATUS_ERROR_INVALID_SURFACE);
        pSurface = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, surfaces[i]);
        DDI_CHK_NULL(pSurface, "Null pSurface", VA_STATUS_ERROR_INVALID_SURFACE);
        if(pSurface->pCurrentFrameSemaphore)
        {
            DdiMediaUtil_WaitSemaphore(pSurface->pCurrentFrameSemaphore);
            DdiMediaUtil_PostSemaphore(pSurface->pCurrentFrameSemaphore);
        }
        if(pSurface->pReferenceFrameSemaphore)
        {
            DdiMediaUtil_WaitSemaphore(pSurface->pReferenceFrameSemaphore);
            DdiMediaUtil_PostSemaphore(pSurface->pReferenceFrameSemaphore);
        }
    }

    for(i = 0; i < num_surfaces; i++)
    {
        DDI_CHK_LESS((uint32_t)surfaces[i], pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaces", VA_STATUS_ERROR_INVALID_SURFACE);
        pSurface = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, surfaces[i]);
        DDI_CHK_NULL(pSurface, "Null pSurface", VA_STATUS_ERROR_INVALID_SURFACE);
        if(pSurface->pCurrentFrameSemaphore)
        {
            DdiMediaUtil_DestroySemaphore(pSurface->pCurrentFrameSemaphore);
            pSurface->pCurrentFrameSemaphore = nullptr;
        }

        if(pSurface->pReferenceFrameSemaphore)
        {
            DdiMediaUtil_DestroySemaphore(pSurface->pReferenceFrameSemaphore);
            pSurface->pReferenceFrameSemaphore = nullptr;
        }

        DdiDecode_UnRegisterRTSurfaces(ctx, pSurface);

        DdiMediaUtil_FreeSurface(pSurface);
        MOS_FreeMemory(pSurface);
        DdiMediaUtil_LockMutex(&pMediaCtx->SurfaceMutex);
        DdiMediaUtil_ReleasePMediaSurfaceFromHeap(pMediaCtx->pSurfaceHeap, (uint32_t)surfaces[i]);
        pMediaCtx->uiNumSurfaces--;
        DdiMediaUtil_UnLockMutex(&pMediaCtx->SurfaceMutex);
    }

    return VA_STATUS_SUCCESS;
}

static VAStatus
DdiMedia_CreateSurfaces2(
    VADriverContextP    ctx,
    uint32_t            format,
    uint32_t            width,
    uint32_t            height,
    VASurfaceID        *surfaces,
    uint32_t            num_surfaces,
    VASurfaceAttrib    *attrib_list,
    uint32_t            num_attribs
    )
{
    int32_t                        i;
    PDDI_MEDIA_CONTEXT             pMediaCtx;
    VAStatus                       vaStatus;
    int32_t                        expected_fourcc;
    DDI_MEDIA_FORMAT               mediaFmt;
    uint32_t                       surfaceUsageHint;
    bool                           bSurfDescProvided;
    bool                           bSurfIsGralloc;
    bool                           bSurfIsUserPtr;
    VASurfaceAttribExternalBuffers tmpExternalBufDesc;
    VASurfaceAttribExternalBuffers *pExternalBufDesc;
    uintptr_t                      *bo_names;
    GMM_RESCREATE_PARAMS           *GmmParams;
    int32_t                        memTypeFlag;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,             "Null ctx",             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LARGER(num_surfaces, 0, "Invalid num_surfaces", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(surfaces,        "Null surfaces",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(width,        0, "Invalid width",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(height,       0, "Invalid height",       VA_STATUS_ERROR_INVALID_PARAMETER);

    surfaceUsageHint = VA_SURFACE_ATTRIB_USAGE_HINT_GENERIC;

    if(num_attribs > 0)
    {
        DDI_CHK_NULL(attrib_list, "Null attrib_list", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    pMediaCtx    = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,       "Null pMediaCtx",   VA_STATUS_ERROR_INVALID_CONTEXT);

    vaStatus          = VA_STATUS_SUCCESS;
    expected_fourcc   = 0;
    bSurfDescProvided = false;
    bSurfIsGralloc    = false;
    bSurfIsUserPtr    = false;
    pExternalBufDesc  = &tmpExternalBufDesc;
    bo_names          = nullptr;
    GmmParams         = nullptr;

    memset(pExternalBufDesc, 0, sizeof(VASurfaceAttribExternalBuffers));
    expected_fourcc = VA_FOURCC_NV12;

    switch(format)
    {
        case VA_RT_FORMAT_YUV420:
            expected_fourcc = VA_FOURCC_NV12;
            break;
        case VA_RT_FORMAT_YUV422:
            expected_fourcc = VA_FOURCC_422H;
            break;
        case VA_RT_FORMAT_YUV444:
            expected_fourcc = VA_FOURCC_444P;
            break;
        case VA_RT_FORMAT_YUV411:
            expected_fourcc = VA_FOURCC_411P;
            break;
        case VA_RT_FORMAT_YUV400:
            expected_fourcc = VA_FOURCC('4','0','0','P');
            break;
        case VA_RT_FORMAT_YUV420_10BPP:
            expected_fourcc = VA_FOURCC_P010;
            break;
        case VA_RT_FORMAT_RGB16:
            expected_fourcc = VA_FOURCC_R5G6B5;
            break;
        case VA_RT_FORMAT_RGB32:
            expected_fourcc = VA_FOURCC_BGRA;
            break;
        case VA_RT_FORMAT_RGBP:
            expected_fourcc = VA_FOURCC_RGBP;
            break;
#ifdef VA_RT_FORMAT_RGB32_10BPP
        case VA_RT_FORMAT_RGB32_10BPP:
            expected_fourcc = VA_FOURCC_BGRA;
            break;
#endif
#if 1 //added for having MDF sanity test pass, will be removed after MDF formal patch checked in
        case VA_FOURCC_NV12:
            expected_fourcc = VA_FOURCC_NV12;
            break;
        case VA_FOURCC_NV21:
            expected_fourcc = VA_FOURCC_NV21;
            break;
        case VA_FOURCC_ABGR:
            expected_fourcc = VA_FOURCC_ABGR;
            break;
        case VA_FOURCC_ARGB:
            expected_fourcc = VA_FOURCC_ARGB;
            break;
        case VA_FOURCC_XBGR:
            expected_fourcc = VA_FOURCC_XBGR;
            break;
        case VA_FOURCC_XRGB:
            expected_fourcc = VA_FOURCC_XRGB;
            break;
        case VA_FOURCC_R5G6B5:
            expected_fourcc = VA_FOURCC_R5G6B5;
            break;
        case VA_FOURCC_R8G8B8:
            expected_fourcc = VA_FOURCC_R8G8B8;
            break;
        case VA_FOURCC_YUY2:
            expected_fourcc = VA_FOURCC_YUY2;
            break;
        case VA_FOURCC_YV12:
            expected_fourcc = VA_FOURCC_YV12;
            break;
        case VA_FOURCC_422H:
            expected_fourcc = VA_FOURCC_422H;
            break;
        case VA_FOURCC_422V:
            expected_fourcc = VA_FOURCC_422V;
            break;
        case VA_FOURCC_P208:
            expected_fourcc = VA_FOURCC_P208;
            break;
        case VA_FOURCC('P','0','1','0'):
            expected_fourcc = VA_FOURCC('P','0','1','0');
            break;
#endif
        default:
            DDI_ASSERTMESSAGE("Invalid VAConfigAttribRTFormat: 0x%x. Please uses the format defined in libva/va.h", format);
            return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }


    memTypeFlag = 0;
    for (i = 0; i < num_attribs && attrib_list; i++)
    {
        if (attrib_list[i].flags & VA_SURFACE_ATTRIB_SETTABLE)
        {
             switch (attrib_list[i].type)
             {
                  case VASurfaceAttribUsageHint:
                      DDI_ASSERT(attrib_list[i].value.type == VAGenericValueTypeInteger);
                      surfaceUsageHint = attrib_list[i].value.value.i;
                      break;
                  case VASurfaceAttribPixelFormat:
                      DDI_ASSERT(attrib_list[i].value.type == VAGenericValueTypeInteger);
                      expected_fourcc = attrib_list[i].value.value.i;
                      break;
                  case VASurfaceAttribMemoryType:
                      DDI_ASSERT(attrib_list[i].value.type == VAGenericValueTypeInteger);
                      if ( (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM)
                          ||(attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME)
#ifdef ANDROID
                          ||(attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC)

                          ||(attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR)
#endif
                          )
                      {
                          memTypeFlag = attrib_list[i].value.value.i;
#ifdef ANDROID
                          bSurfIsGralloc = (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC);
                          bSurfIsUserPtr = (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR);
#else
                          bSurfIsGralloc = false;
                          bSurfIsUserPtr = false;
#endif
                      }
                      else
                      {
                           DDI_ASSERTMESSAGE("Not supported external buffer type.");
                           return VA_STATUS_ERROR_INVALID_PARAMETER;
                      }
                      break;
                  case (VASurfaceAttribType)VASurfaceAttribExternalBufferDescriptor:
                      DDI_ASSERT(attrib_list[i].value.type == VAGenericValueTypePointer);
                      if( nullptr == attrib_list[i].value.value.p )
                      {
                          DDI_ASSERTMESSAGE("Invalid VASurfaceAttribExternalBuffers used.");
                          return VA_STATUS_ERROR_INVALID_PARAMETER;
                      }
                      MOS_SecureMemcpy(pExternalBufDesc, sizeof(VASurfaceAttribExternalBuffers),  attrib_list[i].value.value.p, sizeof(VASurfaceAttribExternalBuffers));

                      expected_fourcc  = pExternalBufDesc->pixel_format;
                      width            = pExternalBufDesc->width;
                      height           = pExternalBufDesc->height;
                      bSurfDescProvided = true;
                      // the following code is for backward compatible and it will be removed in the future
                     // new implemention should use VASurfaceAttribMemoryType attrib and set its value to VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM
                     if( (pExternalBufDesc->flags & VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM )
                         || (pExternalBufDesc->flags & VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME)
#ifdef ANDROID
                         || (pExternalBufDesc->flags & VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC)

                         || (pExternalBufDesc->flags & VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR)
#endif
                         )
                      {

                           memTypeFlag       = pExternalBufDesc->flags;
#ifdef ANDROID
                           bSurfIsGralloc = ((pExternalBufDesc->flags & VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC) != 0);
                           bSurfIsUserPtr = ((pExternalBufDesc->flags & VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR) != 0);
#else
                           bSurfIsGralloc = false;
                           bSurfIsUserPtr = false;
#endif
                      }

                      break;
                  default:
                      DDI_ASSERTMESSAGE("Unsupported type.");
                      break;
              }
        }
    }

    mediaFmt = DdiMedia_OsFormatToMediaFormat(expected_fourcc,format);
    if (mediaFmt == Media_Format_Count)
    {
        DDI_ASSERTMESSAGE("DDI: unsupported surface type in DdiMedia_CreateSurfaces2.");
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    if ( bSurfDescProvided == true && memTypeFlag == 0) {
        DDI_ASSERTMESSAGE("Not supported external buffer type.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

#ifdef ANDROID
    if( bSurfDescProvided == true )
    {
        // following code is ported from DdiCodec_CreateSurfacesWithAttribute to handle VA_SURFACE_ATTRIB_MEM_TYPE_ANDROID_GRALLOC
        if( bSurfIsGralloc == true )
        {
            int32_t     flags;
            uint32_t    uiHeight;
            uint32_t    uiSize;

            flags    = 0;
            uiHeight = 0;
            uiSize   = 0;
            bo_names = (uintptr_t*)MOS_AllocAndZeroMemory( sizeof(uintptr_t) * pExternalBufDesc->num_buffers);
            GmmParams = (GMM_RESCREATE_PARAMS *)MOS_AllocAndZeroMemory( sizeof(GMM_RESCREATE_PARAMS) * pExternalBufDesc->num_buffers );
            if( bo_names == nullptr || GmmParams == nullptr)
            {
                vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
                goto finish;
            }

            for (i = 0; i < pExternalBufDesc->num_buffers; i++)
            {
                vaStatus = android_get_buffer_info(pExternalBufDesc->buffers[i], &bo_names[i], &flags, &uiHeight, &uiSize);
                if (vaStatus != VA_STATUS_SUCCESS)
                {
                    goto finish;
                }

                // get GmmResource from Gralloc
                if( android_get_gralloc_gmm_params( pExternalBufDesc->buffers[i], &GmmParams[i] ) )
                {
                    vaStatus = VA_STATUS_ERROR_INVALID_BUFFER;
                    goto finish;
                }
            }

            // The surface height is 736, but App pass 720 to driver.
            pExternalBufDesc->height        = uiHeight;
            pExternalBufDesc->buffers       = (unsigned long *)bo_names;
            pExternalBufDesc->data_size     = uiSize;

            height                          = pExternalBufDesc->height;
            num_surfaces                    = pExternalBufDesc->num_buffers;
        }

        if( bSurfIsUserPtr == true )
        {
            // only support NV12 Linear and P010 Linear usage for tiling and linear conversion
            // Expect buffer  size = pitch * height * 3/2. so it can't be smaller thant the expeced size
            if (VA_RT_FORMAT_YUV420 != format)
            {
                DDI_VERBOSEMESSAGE("Input color format doesn't support");
                vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
                goto finish;
            }

            mediaFmt  = DdiMedia_VaFmtToMediaFmt(format, expected_fourcc);
            if ((Media_Format_NV12 != mediaFmt) && (Media_Format_P010 != mediaFmt))
            {
                DDI_VERBOSEMESSAGE("Internal media format is invalid");
                vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
                goto finish;
            }

            if( !MOS_IS_ALIGNED(pExternalBufDesc->data_size, MOS_PAGE_SIZE) )
            {
                pExternalBufDesc->data_size = MOS_ALIGN_CEIL(pExternalBufDesc->data_size, MOS_PAGE_SIZE);
            }

            if ((Media_Format_NV12 == mediaFmt) &&
                    (pExternalBufDesc->data_size < pExternalBufDesc->pitches[0] * height * 3/2))
            {
                DDI_VERBOSEMESSAGE("Buffer size is too small");
                vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
                goto finish;
            }
            else if ((Media_Format_P010 == mediaFmt) &&
                        (pExternalBufDesc->data_size < pExternalBufDesc->pitches[0] * height * 3))
            {
                DDI_VERBOSEMESSAGE("Buffer size is too small");
                vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
                goto finish;

            }
            else
            {
                DDI_VERBOSEMESSAGE("Internal media format is invalid");
                vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
                goto finish;
            }
        }
    }
#endif

    for(i = 0; i < num_surfaces; i++)
    {
        VASurfaceID                   vaSurfaceID;
        PDDI_MEDIA_SURFACE_DESCRIPTOR pSurfDesc;
        MOS_STATUS                    eStatus = MOS_STATUS_SUCCESS;

        pSurfDesc = nullptr;

        if( bSurfDescProvided == true )
        {
            pSurfDesc = (PDDI_MEDIA_SURFACE_DESCRIPTOR)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE_DESCRIPTOR));
            if( pSurfDesc == nullptr )
            {
               vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
               goto finish;
            }
            memset(pSurfDesc,0,sizeof(DDI_MEDIA_SURFACE_DESCRIPTOR));

            pSurfDesc->uiPlanes       = pExternalBufDesc->num_planes;
            pSurfDesc->ulBuffer       = pExternalBufDesc->buffers[i];
            pSurfDesc->uiFlags        = memTypeFlag;
            pSurfDesc->uiSize         = pExternalBufDesc->data_size;
            pSurfDesc->uiBuffserSize  = pExternalBufDesc->data_size;

            eStatus = MOS_SecureMemcpy(pSurfDesc->uiPitches, sizeof(pSurfDesc->uiPitches), pExternalBufDesc->pitches, sizeof(pExternalBufDesc->pitches));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                DDI_VERBOSEMESSAGE("DDI:Failed to copy surface buffer data!");
                vaStatus = VA_STATUS_ERROR_OPERATION_FAILED;
                goto finish;
            }
            eStatus = MOS_SecureMemcpy(pSurfDesc->uiOffsets, sizeof(pSurfDesc->uiOffsets), pExternalBufDesc->offsets, sizeof(pExternalBufDesc->offsets));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                DDI_VERBOSEMESSAGE("DDI:Failed to copy surface buffer data!");
                vaStatus = VA_STATUS_ERROR_OPERATION_FAILED;
                goto finish;
            }

            // get the GmmParams for Gralloc buffer
            if( bSurfIsGralloc == true )
            {
                pSurfDesc->bIsGralloc = true;
                pSurfDesc->GmmParam = GmmParams[i];
            }

            if( bSurfIsUserPtr )
            {
                pSurfDesc->uiTile = I915_TILING_NONE;

                if (pSurfDesc->ulBuffer % 4096 != 0)
                {
                    DDI_VERBOSEMESSAGE("Buffer Address is invalid");
                    vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
                    goto finish;
                }
            }
        }
        vaSurfaceID = (VASurfaceID)DdiMedia_CreateRenderTarget(pMediaCtx, mediaFmt, width, height, (bSurfDescProvided ? pSurfDesc : nullptr), surfaceUsageHint );
        if (VA_INVALID_ID != vaSurfaceID)
        {
            surfaces[i] = vaSurfaceID;
        }
        else
        {
            // here to release the successful allocated surfaces?
            if( nullptr != pSurfDesc )
            {
                MOS_FreeMemory(pSurfDesc);
            }
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            goto finish;
        }
    }

finish:
    if (nullptr != bo_names)
    {
        MOS_FreeMemory(bo_names);
    }
    if( nullptr != GmmParams )
    {
        MOS_FreeMemory(GmmParams);
    }

    return vaStatus;
}

static VAStatus DdiMedia_CreateMfeContextInternal(
    VADriverContextP    ctx,
    VAMFContextID      *mfe_context
)
{
    PDDI_MEDIA_CONTEXT                pMediaDrvCtx;
    VAStatus                          vaStatus;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT pVaContextHeapElmt;
    PDDI_ENCODE_MFE_CONTEXT           pEncodeMfeContext;
    MfeSharedState* pMfeEncodeSharedState;

    pMediaDrvCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaDrvCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    vaStatus            = VA_STATUS_SUCCESS;
    *mfe_context        = DDI_MEDIA_INVALID_VACONTEXTID;

    if (!GFX_IS_PRODUCT(pMediaDrvCtx->platform, IGFX_SKYLAKE))
    {
        DDI_VERBOSEMESSAGE("MFE is not supported on the platform!");
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    pEncodeMfeContext = (PDDI_ENCODE_MFE_CONTEXT)MOS_AllocAndZeroMemory(sizeof(DDI_ENCODE_MFE_CONTEXT));
    if (nullptr == pEncodeMfeContext)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    DdiMediaUtil_LockMutex(&pMediaDrvCtx->MfeMutex);
    pVaContextHeapElmt = DdiMediaUtil_AllocPVAContextFromHeap(pMediaDrvCtx->pMfeCtxHeap);
    if (nullptr == pVaContextHeapElmt)
    {
        DdiMediaUtil_UnLockMutex(&pMediaDrvCtx->MfeMutex);
        vaStatus = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        goto CleanUpandReturn;
    }

    pVaContextHeapElmt->pVaContext    = (void*)pEncodeMfeContext;
    pMediaDrvCtx->uiNumMfes++;
    *mfe_context                          = (VAMFContextID)(pVaContextHeapElmt->uiVaContextID + DDI_MEDIA_VACONTEXTID_OFFSET_MFE);
    DdiMediaUtil_UnLockMutex(&pMediaDrvCtx->MfeMutex);

    // Create shared state, which is used by all the sub contexts
    pMfeEncodeSharedState = (MfeSharedState*)MOS_AllocAndZeroMemory(sizeof(MfeSharedState));
    if (nullptr == pMfeEncodeSharedState)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    pEncodeMfeContext->mfeEncodeSharedState = pMfeEncodeSharedState;

    DdiMediaUtil_InitMutex(&pEncodeMfeContext->encodeMfeMutex);

    return vaStatus;

CleanUpandReturn:
    if (pEncodeMfeContext)
    {
        MOS_FreeMemory(pEncodeMfeContext);
    }
    return vaStatus;
}

static VAStatus DdiMedia_DestoryMfeContext (
    VADriverContextP    ctx,
    VAMFContextID      mfe_context
)
{
    PDDI_MEDIA_CONTEXT                  pMediaCtx;
    PDDI_ENCODE_MFE_CONTEXT             pEncodeMfeContext;
    uint32_t                            uiCtxType = DDI_MEDIA_CONTEXT_TYPE_NONE;

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    pEncodeMfeContext  = (PDDI_ENCODE_MFE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, mfe_context, &uiCtxType);
    DDI_CHK_NULL(pEncodeMfeContext, "Null pEncodeMfeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    // Release std::vector memory
    pEncodeMfeContext->pDdiEncodeContexts.clear();
    pEncodeMfeContext->pDdiEncodeContexts.shrink_to_fit();

    pEncodeMfeContext->mfeEncodeSharedState->encoders.clear();
    pEncodeMfeContext->mfeEncodeSharedState->encoders.shrink_to_fit();

    DdiMediaUtil_DestroyMutex(&pEncodeMfeContext->encodeMfeMutex);
    MOS_FreeMemory(pEncodeMfeContext->mfeEncodeSharedState);
    MOS_FreeMemory(pEncodeMfeContext);

    DdiMediaUtil_LockMutex(&pMediaCtx->MfeMutex);
    DdiMediaUtil_ReleasePVAContextFromHeap(pMediaCtx->pMfeCtxHeap, (mfe_context & DDI_MEDIA_MASK_VACONTEXTID));
    pMediaCtx->uiNumMfes--;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->MfeMutex);
    return VA_STATUS_SUCCESS;
}

static VAStatus DdiMedia_AddContextInternal(
    VADriverContextP    ctx,
    VAContextID         context,
    VAMFContextID      mfe_context
)
{
    PDDI_MEDIA_CONTEXT                  pMediaCtx;
    PDDI_ENCODE_MFE_CONTEXT             pEncodeMfeContext;
    PDDI_ENCODE_CONTEXT                 pEncodeContext;
    uint32_t                            uiCtxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    CodechalEncoderState                *encoder;

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    pEncodeMfeContext  = (PDDI_ENCODE_MFE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, mfe_context, &uiCtxType);
    DDI_CHK_NULL(pEncodeMfeContext, "Null pEncodeMfeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    pEncodeContext  = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(pEncodeContext, "Null pEncodeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    encoder = dynamic_cast<CodechalEncoderState *>(pEncodeContext->pCodecHal);
    DDI_CHK_NULL(encoder, "Null codechal encoder", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (pEncodeContext->wModeType != CODECHAL_ENCODE_MODE_AVC)
    {
        DDI_VERBOSEMESSAGE("MFE is not supported for the codec!");
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    DdiMediaUtil_LockMutex(&pEncodeMfeContext->encodeMfeMutex);
    pEncodeMfeContext->pDdiEncodeContexts.push_back(pEncodeContext);

    encoder->m_mfeEnabled = true;
    // Assign one unique id to this sub context/stream
    encoder->m_mfeEncodeParams.streamId = pEncodeMfeContext->currentStreamId;
    encoder->m_mfeEncodeSharedState     = pEncodeMfeContext->mfeEncodeSharedState;

    pEncodeMfeContext->currentStreamId++;
    DdiMediaUtil_UnLockMutex(&pEncodeMfeContext->encodeMfeMutex);
    return VA_STATUS_SUCCESS;
}

static VAStatus DdiMedia_ReleaseContextInternal(
    VADriverContextP    ctx,
    VAContextID         context,
    VAMFContextID      mfe_context
)
{
    PDDI_MEDIA_CONTEXT                  pMediaCtx;
    PDDI_ENCODE_MFE_CONTEXT             pEncodeMfeContext;
    PDDI_ENCODE_CONTEXT                 pEncodeContext;
    uint32_t                            uiCtxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    int32_t                             i;

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    pEncodeMfeContext  = (PDDI_ENCODE_MFE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, mfe_context, &uiCtxType);
    DDI_CHK_NULL(pEncodeMfeContext, "Null pEncodeMfeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    pEncodeContext  = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(pEncodeMfeContext, "Null pEncodeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    DdiMediaUtil_LockMutex(&pEncodeMfeContext->encodeMfeMutex);
    for (i = 0; i < pEncodeMfeContext->pDdiEncodeContexts.size(); i++)
    {
        if (pEncodeMfeContext->pDdiEncodeContexts[i] == pEncodeContext)
        {
            pEncodeMfeContext->pDdiEncodeContexts.erase(pEncodeMfeContext->pDdiEncodeContexts.begin() + i);
            break;
        }
    }
    DdiMediaUtil_UnLockMutex(&pEncodeMfeContext->encodeMfeMutex);

    return VA_STATUS_SUCCESS;
}

static VAStatus DdiMedia_CreateContext (
    VADriverContextP    ctx,
    VAConfigID          config_id,
    int32_t             picture_width,
    int32_t             picture_height,
    int32_t             flag,
    VASurfaceID        *render_targets,
    int32_t             num_render_targets,
    VAContextID        *context
)
{
    PDDI_MEDIA_CONTEXT     pMediaDrvCtx;
    VAStatus               vaStatus;
    int32_t                i;
    uint32_t               surfaceId;

    DDI_FUNCTION_ENTER();

    vaStatus            = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(ctx,     "Null ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(context, "Null context",      VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaDrvCtx    = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaDrvCtx,     "Null pMediaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(num_render_targets > 0)
    {
        DDI_CHK_NULL(render_targets,             "Null render_targets",             VA_STATUS_ERROR_INVALID_PARAMETER);
        DDI_CHK_NULL(pMediaDrvCtx->pSurfaceHeap, "Null pMediaDrvCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
        for(i = 0; i < num_render_targets; i++)
        {
            surfaceId = (uint32_t)render_targets[i];
            DDI_CHK_LESS(surfaceId, pMediaDrvCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid Surface", VA_STATUS_ERROR_INVALID_SURFACE);
        }
    }

    if(pMediaDrvCtx->m_caps->IsDecConfigId(config_id))
    {
        vaStatus = DdiDecode_CreateContext(ctx, config_id - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE, picture_width, picture_height, flag, render_targets, num_render_targets, context);
    }
    else if(pMediaDrvCtx->m_caps->IsEncConfigId(config_id))
    {
        vaStatus = DdiEncode_CreateContext(ctx, config_id - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE, picture_width, picture_height, flag, render_targets, num_render_targets, context);
    }
    else if(pMediaDrvCtx->m_caps->IsVpConfigId(config_id))
    {
        vaStatus = DdiVp_CreateContext(ctx, config_id - DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE, picture_width, picture_height, flag, render_targets, num_render_targets, context);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: Invalid config_id");
        vaStatus = VA_STATUS_ERROR_INVALID_CONFIG;
    }

    return vaStatus;
}

static VAStatus DdiMedia_DestroyContext (
    VADriverContextP    ctx,
    VAContextID         context
)
{
    void               *pCtx;
    uint32_t            uiCtxType = DDI_MEDIA_CONTEXT_TYPE_NONE;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "Null ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pCtx  = DdiMedia_GetContextFromContextID(ctx, context, &uiCtxType);

    switch (uiCtxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        case DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER:
            return DdiDecode_DestroyContext(ctx, context);
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            return DdiEncode_DestroyContext(ctx, context);
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            return DdiVp_DestroyContext(ctx, context);
        case DDI_MEDIA_CONTEXT_TYPE_MFE:
            return DdiMedia_DestoryMfeContext(ctx, context);
        default:
            DDI_ASSERTMESSAGE("DDI: unsupported context in DdiCodec_DestroyContext.");
            return VA_STATUS_ERROR_INVALID_CONTEXT;
    }
}

static VAStatus DdiMedia_CreateBuffer (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferType        type,
    uint32_t            size,
    uint32_t            num_elements,
    void                *pData,
    VABufferID          *pBufId
)
{
    VAStatus                     va;
    PDDI_MEDIA_CONTEXT           pMediaCtx;
    uint32_t                     uiCtxType;
    void                        *pCtx;

    DDI_FUNCTION_ENTER();

    va          = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(ctx,        "Null ctx",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pBufId,     "Null buf_id",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(size, 0,  "Invalid size", VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pCtx = DdiMedia_GetContextFromContextID(ctx, context, &uiCtxType);
    DDI_CHK_NULL(pCtx,      "Null pCtx",      VA_STATUS_ERROR_INVALID_CONTEXT);

    *pBufId     = VA_INVALID_ID;

    DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
    switch (uiCtxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        case DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER:
            va = DdiDecode_CreateBuffer(ctx, DdiDecode_GetDecContextFromPVOID(pCtx), type, size, num_elements, pData, pBufId);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            va = DdiEncode_CreateBuffer(ctx, context, type, size, num_elements, pData, pBufId);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            va = DdiVp_CreateBuffer(ctx, pCtx, type, size, num_elements, pData, pBufId);
            break;
        default:
            va = VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
    return va;
}

/*
 * Convey to the server how many valid elements are in the buffer.
 * e.g. if multiple slice parameters are being held in a single buffer,
 * this will communicate to the server the number of slice parameters
 * that are valid in the buffer.
 */
static VAStatus DdiMedia_BufferSetNumElements (
    VADriverContextP    ctx,
    VABufferID          buf_id,
    uint32_t            num_elements
)
{
    PDDI_MEDIA_CONTEXT     pMediaCtx;
    DDI_MEDIA_BUFFER      *pBuf;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "Null ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,              "Null pMediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(pMediaCtx->pBufferHeap, "Null pMediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)buf_id, pMediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid buf_id", VA_STATUS_ERROR_INVALID_BUFFER);

    pBuf      = DdiMedia_GetBufferFromVABufferID(pMediaCtx, buf_id);
    DDI_CHK_NULL(pBuf, "Invalid buffer.", VA_STATUS_ERROR_INVALID_BUFFER);

    // only for VASliceParameterBufferType of buffer, the number of elements can be greater than 1
    if(pBuf->uiType != VASliceParameterBufferType &&
       num_elements > 1)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if(pBuf->uiType == VASliceParameterBufferType &&
       pBuf->iNumElements < num_elements)
    {
        MOS_FreeMemory(pBuf->pData);
        pBuf->iSize = pBuf->iSize / pBuf->iNumElements;
        pBuf->pData = (uint8_t*)MOS_AllocAndZeroMemory(pBuf->iSize * num_elements);
        pBuf->iSize = pBuf->iSize * num_elements;
    }

    return VA_STATUS_SUCCESS;
}

/*
 * Map data store of the buffer into the client's address space
 * vaCreateBuffer() needs to be called with "data" set to nullptr before
 * calling vaMapBuffer()
 *
 * if buffer type is VAEncCodedBufferType, pbuf points to link-list of
 * VACodedBufferSegment, and the list is terminated if "next" is nullptr
 */
VAStatus DdiMedia_MapBufferInternal (
    VADriverContextP    ctx,
    VABufferID          buf_id,
    void              **pbuf,
    uint32_t            flag
)
{
    VAStatus sts;
    PDDI_MEDIA_CONTEXT            pMediaCtx;
    DDI_MEDIA_BUFFER             *pBuf;
    void                         *pCtx;
    uint32_t                      uiCtxType;
    DDI_CODEC_COM_BUFFER_MGR     *pBufMgr;
    PDDI_ENCODE_CONTEXT           pEncCtx;
    PDDI_DECODE_CONTEXT           pDecCtx;
    uint32_t                      TIMEOUT_NS;

    DDI_FUNCTION_ENTER();

    sts     = VA_STATUS_SUCCESS;

    pEncCtx = nullptr;
    pDecCtx = nullptr;

    DDI_CHK_NULL(ctx,   "Null ctx",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pbuf,  "Null pbuf",    VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,              "Null pMediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(pMediaCtx->pBufferHeap, "Null pMediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)buf_id, pMediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufferId", VA_STATUS_ERROR_INVALID_CONTEXT);

    pBuf      = DdiMedia_GetBufferFromVABufferID(pMediaCtx, buf_id);
    DDI_CHK_NULL(pBuf, "Null pBuf", VA_STATUS_ERROR_INVALID_BUFFER);

    // The context is nullptr when the buffer is created from DdiMedia_DeriveImage
    // So doesn't need to check the context for all cases
    // Only check the context in dec/enc mode
    uiCtxType = DdiMedia_GetCtxTypeFromVABufferID(pMediaCtx, buf_id);

    switch (uiCtxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            break;
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        case DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER:
            pCtx = DdiMedia_GetCtxFromVABufferID(pMediaCtx, buf_id);
            DDI_CHK_NULL(pCtx, "Null pCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

            pDecCtx = DdiDecode_GetDecContextFromPVOID(pCtx);
            pBufMgr = &(pDecCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            pCtx = DdiMedia_GetCtxFromVABufferID(pMediaCtx, buf_id);
            DDI_CHK_NULL(pCtx, "Null pCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

            pEncCtx = DdiEncode_GetEncContextFromPVOID(pCtx);
            DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
            pBufMgr = &(pEncCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_MEDIA:
            break;
        default:
            return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    switch ((int32_t)pBuf->uiType)
    {
        case VASliceDataBufferType:
        case VAProtectedSliceDataBufferType:
        case VABitPlaneBufferType:
            *pbuf = (void *)(pBuf->pData + pBuf->uiOffset);
            break;

        case VASliceParameterBufferType:
            pCtx = DdiMedia_GetCtxFromVABufferID(pMediaCtx, buf_id);
            DDI_CHK_NULL(pCtx, "Null pCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

            pDecCtx = DdiDecode_GetDecContextFromPVOID(pCtx);
            pBufMgr = &(pDecCtx->BufMgr);
            switch (pDecCtx->wMode)
            {
                case CODECHAL_DECODE_MODE_AVCVLD:
                    if(pDecCtx->bShortFormatInUse)
                        *pbuf = (void *)((uint8_t*)(pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264Base) + pBuf->uiOffset);
                    else
                        *pbuf = (void *)((uint8_t*)(pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264) + pBuf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_MPEG2VLD:
                    *pbuf = (void *)((uint8_t*)(pBufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2) + pBuf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_VC1VLD:
                    *pbuf = (void *)((uint8_t*)(pBufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1) + pBuf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_JPEG:
                    *pbuf = (void *)((uint8_t*)(pBufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG) + pBuf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_VP8VLD:
                    *pbuf = (void *)((uint8_t*)(pBufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8) + pBuf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_HEVCVLD:
                    if(pDecCtx->bShortFormatInUse)
                        *pbuf = (void *)((uint8_t*)(pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC) + pBuf->uiOffset);
                    else
                        *pbuf = (void *)((uint8_t*)(pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC) + pBuf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_VP9VLD:
                    *pbuf = (void *)((uint8_t*)(pBufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9) + pBuf->uiOffset);
                    break;
                default:
                    break;
            }
            break;

        case VAEncCodedBufferType:
            DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

            if( DdiEncode_CodedBufferExistInStatusReport( pEncCtx, pBuf ) )
            {
                return DdiEncode_StatusReport(pEncCtx, pBuf, pbuf);
            }
            // so far a coded buffer that has NOT been added into status report is skipped frame in non-CP case
            // but this can change in future if new usage models come up
            pEncCtx->BufMgr.pCodedBufferSegment->buf  = DdiMediaUtil_LockBuffer(pBuf, flag);
            pEncCtx->BufMgr.pCodedBufferSegment->size = pBuf->iSize;
            *pbuf =  pEncCtx->BufMgr.pCodedBufferSegment;

            break;

        case VAStatsStatisticsBufferType:
        case VAStatsStatisticsBottomFieldBufferType:
        case VAStatsMVBufferType:
            {
                DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT)
                DDI_ENCODE_PRE_ENC_BUFFER_TYPE idx = (pBuf->uiType == VAStatsMVBufferType) ? PRE_ENC_BUFFER_TYPE_MVDATA :
                                                    ((pBuf->uiType == VAStatsStatisticsBufferType)   ? PRE_ENC_BUFFER_TYPE_STATS
                                                                                                          : PRE_ENC_BUFFER_TYPE_STATS_BOT);
                if((pEncCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC) && DdiEncode_PreEncBufferExistInStatusReport( pEncCtx, pBuf, idx))
                {
                    return  DdiEncode_PreEncStatusReport(pEncCtx, pBuf, pbuf);
                }
                if(pBuf->bo)
                {
                    *pbuf = DdiMediaUtil_LockBuffer(pBuf, flag);
                }
                break;
            }
        case VAStatsMVPredictorBufferType:
        case VAEncFEIMBControlBufferType:
        case VAEncFEIMVPredictorBufferType:
        case VAEncQPBufferType:
            if(pBuf->bo)
            {
                *pbuf = DdiMediaUtil_LockBuffer(pBuf, flag);
            }
            break;
        case VADecodeStreamoutBufferType:
            if(pBuf->bo)
            {
                 TIMEOUT_NS = 100000000;
                 while (0 != mos_gem_bo_wait(pBuf->bo, TIMEOUT_NS))
                 {
                     // Just loop while gem_bo_wait times-out.
                 }
                 *pbuf = DdiMediaUtil_LockBuffer(pBuf, flag);
            }
            break;
        case VAEncFEIMVBufferType:
        case VAEncFEIMBCodeBufferType:
        case VAEncFEICURecordBufferType:
        case VAEncFEICTBCmdBufferType:
        case VAEncFEIDistortionBufferType:
            {
                DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT)
                if(pEncCtx->wModeType == CODECHAL_ENCODE_MODE_AVC)
                {
                    CodecEncodeAvcFeiPicParams *pFeiPicParams = (CodecEncodeAvcFeiPicParams *)pEncCtx->pFeiPicParams;

                    DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (pBuf->uiType == VAEncFEIMVBufferType) ? FEI_ENC_BUFFER_TYPE_MVDATA :
                                       ((pBuf->uiType == VAEncFEIMBCodeBufferType) ? FEI_ENC_BUFFER_TYPE_MBCODE :
                                                                                        FEI_ENC_BUFFER_TYPE_DISTORTION);
                    if((pFeiPicParams != nullptr) && (pEncCtx->feiFunction == CODECHAL_FUNCTION_FEI_ENC) && DdiEncode_EncBufferExistInStatusReport( pEncCtx, pBuf, idx))
                    {
                        return  DdiEncode_EncStatusReport(pEncCtx, pBuf, pbuf);
                    }
                }
                else if(pEncCtx->wModeType == CODECHAL_ENCODE_MODE_HEVC)

                {
                    CodecEncodeHevcFeiPicParams *pFeiPicParams = (CodecEncodeHevcFeiPicParams *)pEncCtx->pFeiPicParams;
                    DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (pBuf->uiType == VAEncFEICTBCmdBufferType) ? FEI_ENC_BUFFER_TYPE_CTB_CMD   :
                                                      ((pBuf->uiType == VAEncFEICURecordBufferType) ? FEI_ENC_BUFFER_TYPE_CU_RECORD :
                                                                                                      FEI_ENC_BUFFER_TYPE_DISTORTION);
                    if((pFeiPicParams != nullptr) && (pEncCtx->feiFunction == CODECHAL_FUNCTION_FEI_ENC) && DdiEncode_EncBufferExistInStatusReport( pEncCtx, pBuf, idx))
                    {
                        return  DdiEncode_EncStatusReport(pEncCtx, pBuf, pbuf);
                    }
                }
                if(pBuf->bo)
                {
                    *pbuf = DdiMediaUtil_LockBuffer(pBuf, flag);
                }
            }
            break;
        case VAStatsStatisticsParameterBufferType:
            *pbuf = (void *)(pBuf->pData + pBuf->uiOffset);
            break;
        case VAEncMacroblockMapBufferType:
            DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
            *pbuf = DdiMediaUtil_LockBuffer(pBuf, flag);
            DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
            if (nullptr == (*pbuf))
            {
                return VA_STATUS_ERROR_OPERATION_FAILED;
            }
            else
            {
                return VA_STATUS_SUCCESS;
            }
            break;

        case VABufferTypeMax:
            if (DdiMedia_MediaFormatToOsFormat(pBuf->format) != VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT)
            {
                DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);

                if ((nullptr != pBuf->pSurface) && (Media_Format_CPU != pBuf->format))
                {
                    DDI_CHK_RET(DdiMedia_MediaMemoryDecompress(pMediaCtx, pBuf->pSurface),"MMD unsupported!");
                }

                *pbuf = DdiMediaUtil_LockBuffer(pBuf, flag);
                DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
                if (nullptr == (*pbuf))
                {
                    return VA_STATUS_ERROR_OPERATION_FAILED;
                }
                else
                {
                    return VA_STATUS_SUCCESS;
                }
            }
            return VA_STATUS_ERROR_INVALID_BUFFER;

        case VAProbabilityBufferType:
            *pbuf = (void *)(pBuf->pData + pBuf->uiOffset);

            break;

        case VAEncMacroblockDisableSkipMapBufferType:
            if(pBuf->bo)
            {
                *pbuf = DdiMediaUtil_LockBuffer(pBuf, flag);
            }
            break;

        case VAImageBufferType:
        default:
            *pbuf = (void *)(pBuf->pData + pBuf->uiOffset);
            break;
    }

    return VA_STATUS_SUCCESS;
}


VAStatus DdiMedia_MapBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id,
    void                **pbuf
)
{
    return DdiMedia_MapBufferInternal(ctx, buf_id, pbuf, MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY);
}

/*
 * After client making changes to a mapped data store, it needs to
 * "Unmap" it to let the server know that the data is ready to be
 * consumed by the server
 */
VAStatus DdiMedia_UnmapBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id
)
{
    PDDI_MEDIA_CONTEXT            pMediaCtx;
    DDI_MEDIA_BUFFER             *pBuf;
    void                         *pCtx;
    uint32_t                      uiCtxType;
    DDI_CODEC_COM_BUFFER_MGR     *pBufMgr;
    PDDI_ENCODE_CONTEXT           pEncCtx = nullptr;
    PDDI_DECODE_CONTEXT           pDecCtx;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "Null ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,               "Null pMediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL( pMediaCtx->pBufferHeap, "Null  pMediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)buf_id, pMediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid buf_id", VA_STATUS_ERROR_INVALID_BUFFER);

    pBuf      = DdiMedia_GetBufferFromVABufferID(pMediaCtx,  buf_id);
    DDI_CHK_NULL(pBuf, "Null pBuf", VA_STATUS_ERROR_INVALID_BUFFER);

    // The context is nullptr when the buffer is created from DdiMedia_DeriveImage
    // So doesn't need to check the context for all cases
    // Only check the context in dec/enc mode
    uiCtxType = DdiMedia_GetCtxTypeFromVABufferID(pMediaCtx, buf_id);

    switch (uiCtxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            break;
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        case DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER:
            pCtx = DdiMedia_GetCtxFromVABufferID(pMediaCtx, buf_id);
            DDI_CHK_NULL(pCtx, "Null pCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

            pDecCtx = DdiDecode_GetDecContextFromPVOID(pCtx);
            pBufMgr = &(pDecCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            pCtx = DdiMedia_GetCtxFromVABufferID(pMediaCtx, buf_id);
            DDI_CHK_NULL(pCtx, "Null pCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

            pEncCtx = DdiEncode_GetEncContextFromPVOID(pCtx);
            pBufMgr = &(pEncCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_MEDIA:
            break;
        default:
            return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    switch ((int32_t)pBuf->uiType)
    {
        case VASliceDataBufferType:
        case VAProtectedSliceDataBufferType:
        case VABitPlaneBufferType:
            break;
        case VAEncCodedBufferType:
        case VAStatsStatisticsBufferType:
        case VAStatsStatisticsBottomFieldBufferType:
        case VAStatsMVBufferType:
        case VAStatsMVPredictorBufferType:
        case VAEncFEIMBControlBufferType:
        case VAEncFEIMVPredictorBufferType:
        case VAEncFEIMVBufferType:
        case VAEncFEIMBCodeBufferType:
        case VAEncFEICURecordBufferType:
        case VAEncFEICTBCmdBufferType:
        case VAEncFEIDistortionBufferType:
        case VAEncQPBufferType:
        case VADecodeStreamoutBufferType:
            if(pBuf->bo)
            {
                DdiMediaUtil_UnlockBuffer(pBuf);
            }
            break;

        case VABufferTypeMax:
            if (DdiMedia_MediaFormatToOsFormat(pBuf->format) != VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT)
            {
                DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
                DdiMediaUtil_UnlockBuffer(pBuf);
                DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
                return VA_STATUS_SUCCESS;
            }
            return VA_STATUS_ERROR_INVALID_BUFFER;
        case VAEncMacroblockDisableSkipMapBufferType:
            if(pBuf->bo)
            {
                DdiMediaUtil_UnlockBuffer(pBuf);
            }
            break;

        case VAImageBufferType:
         default:
            break;
    }

    return VA_STATUS_SUCCESS;
}

/*
 * After this call, the buffer is deleted and this buffer_id is no longer valid
 * Only call this if the buffer is not going to be passed to vaRenderBuffer
 */
VAStatus DdiMedia_DestroyBuffer (
    VADriverContextP    ctx,
    VABufferID          buffer_id
)
{
    PDDI_MEDIA_CONTEXT            pMediaCtx;
    DDI_MEDIA_BUFFER             *pBuf;
    void                         *pCtx;
    uint32_t                      uiCtxType;
    DDI_CODEC_COM_BUFFER_MGR     *pBufMgr  = nullptr;
    PDDI_ENCODE_CONTEXT           pEncCtx  = nullptr;
    PDDI_DECODE_CONTEXT           pDecCtx  = nullptr;

    DDI_FUNCTION_ENTER();

    pEncCtx = nullptr;

    DDI_CHK_NULL(ctx,                    "Null ctx",                    VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,              "Null pMediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(pMediaCtx->pBufferHeap, "Null pMediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)buffer_id, pMediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufferId", VA_STATUS_ERROR_INVALID_CONTEXT);

    pBuf      = DdiMedia_GetBufferFromVABufferID(pMediaCtx,  buffer_id);
    DDI_CHK_NULL(pBuf, "Null pBuf", VA_STATUS_ERROR_INVALID_BUFFER);

    pCtx      = DdiMedia_GetCtxFromVABufferID(pMediaCtx,     buffer_id);
    uiCtxType = DdiMedia_GetCtxTypeFromVABufferID(pMediaCtx, buffer_id);

    switch (uiCtxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        case DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER:
            DDI_CHK_NULL(pCtx, "Null pCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
            pDecCtx = DdiDecode_GetDecContextFromPVOID(pCtx);
            pBufMgr = &(pDecCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            DDI_CHK_NULL(pCtx, "Null pCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
            pEncCtx = DdiEncode_GetEncContextFromPVOID(pCtx);
            pBufMgr = &(pEncCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            break;
        case DDI_MEDIA_CONTEXT_TYPE_MEDIA:
            break;
        default:
            return VA_STATUS_ERROR_INVALID_BUFFER;
    }
    switch ((int32_t)pBuf->uiType)
    {
        case VASliceDataBufferType:
        case VAProtectedSliceDataBufferType:
            DdiMedia_ReleaseBsBuffer(pBufMgr, pBuf);
            break;
        case VABitPlaneBufferType:
            DdiMedia_ReleaseBpBuffer(pBufMgr, pBuf);
            break;
        case VAProbabilityBufferType:
            DdiMedia_ReleaseBpBuffer(pBufMgr, pBuf);
            break;

        case VASliceParameterBufferType:
            DdiMedia_ReleaseSliceControlBuffer(uiCtxType, pCtx, pBuf);
            break;
        case VAPictureParameterBufferType:
            break;
        case VAImageBufferType:
            MOS_FreeMemory(pBuf->pData);
            break;
        case VAProcPipelineParameterBufferType:
        case VAProcFilterParameterBufferType:
            MOS_FreeMemory(pBuf->pData);
            break;
        case VAIQMatrixBufferType:
        case VAHuffmanTableBufferType:
        case VAEncSliceParameterBufferType:
        case VAEncPictureParameterBufferType:
        case VAEncSequenceParameterBufferType:
        case VAEncPackedHeaderDataBufferType:
        case VAEncPackedHeaderParameterBufferType:
            MOS_FreeMemory(pBuf->pData);
            break;
        case VABufferTypeMax:
            DdiMediaUtil_UnRefBufObjInMediaBuffer(pBuf);
            break;
        case VAEncMacroblockMapBufferType:
            DdiMediaUtil_FreeBuffer(pBuf);
            break;
#ifdef ENABLE_ENC_UNLIMITED_OUTPUT
        case VAEncCodedBufferType:
            if(nullptr == pEncCtx)
            {
                pEncCtx = DdiEncode_GetEncContextFromPVOID(pCtx);
                if(nullptr == pEncCtx)
                    return VA_STATUS_ERROR_INVALID_CONTEXT;
            }
            DdiEncode_RemoveFromStatusReportQueue(pEncCtx,pBuf);
            DdiMediaUtil_FreeBuffer(pBuf);
            break;
#endif
        case VAStatsStatisticsParameterBufferType:
            MOS_FreeMemory(pBuf->pData);
            break;
        case VAStatsStatisticsBufferType:
        case VAStatsStatisticsBottomFieldBufferType:
        case VAStatsMVBufferType:
            {
                if(nullptr == pEncCtx)
                {
                    pEncCtx = DdiEncode_GetEncContextFromPVOID(pCtx);
                    if(nullptr == pEncCtx)
                        return VA_STATUS_ERROR_INVALID_CONTEXT;
                }
                if(pEncCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
                {
                    DDI_ENCODE_PRE_ENC_BUFFER_TYPE idx = (pBuf->uiType == VAStatsMVBufferType) ? PRE_ENC_BUFFER_TYPE_MVDATA :
                                                        ((pBuf->uiType == VAStatsStatisticsBufferType)   ? PRE_ENC_BUFFER_TYPE_STATS
                                                                                                              : PRE_ENC_BUFFER_TYPE_STATS_BOT);
                    DdiEncode_RemoveFromPreEncStatusReportQueue(pEncCtx, pBuf, idx);
                }
            }
            DdiMediaUtil_FreeBuffer(pBuf);
            break;
        case VAStatsMVPredictorBufferType:
        case VAEncFEIMBControlBufferType:
        case VAEncFEIMVPredictorBufferType:
        case VAEncQPBufferType:
        case VADecodeStreamoutBufferType:
            DdiMediaUtil_FreeBuffer(pBuf);
            break;
        case VAEncFEIMVBufferType:
        case VAEncFEIMBCodeBufferType:
        case VAEncFEICURecordBufferType:
        case VAEncFEICTBCmdBufferType:
        case VAEncFEIDistortionBufferType:
            {
                if(nullptr == pEncCtx)
                {
                    pEncCtx = DdiEncode_GetEncContextFromPVOID(pCtx);
                    if(nullptr == pEncCtx)
                        return VA_STATUS_ERROR_INVALID_CONTEXT;
                }
                if(pEncCtx->wModeType == CODECHAL_ENCODE_MODE_AVC)
                {
                    CodecEncodeAvcFeiPicParams *pFeiPicParams;
                    pFeiPicParams = (CodecEncodeAvcFeiPicParams *)(pEncCtx->pFeiPicParams);
                    if((pFeiPicParams != nullptr) && (pEncCtx->feiFunction == CODECHAL_FUNCTION_FEI_ENC))
                    {
                        DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (pBuf->uiType == VAEncFEIMVBufferType) ? FEI_ENC_BUFFER_TYPE_MVDATA :
                                                        ((pBuf->uiType == VAEncFEIMBCodeBufferType) ? FEI_ENC_BUFFER_TYPE_MBCODE :
                                                                                                      FEI_ENC_BUFFER_TYPE_DISTORTION);
                        DdiEncode_RemoveFromEncStatusReportQueue(pEncCtx, pBuf, idx);
                    }

                }
                else if(pEncCtx->wModeType == CODECHAL_ENCODE_MODE_HEVC)
                {
                    CodecEncodeHevcFeiPicParams *pFeiPicParams;
                    pFeiPicParams = (CodecEncodeHevcFeiPicParams *)(pEncCtx->pFeiPicParams);
                    if((pFeiPicParams != nullptr) && (pEncCtx->feiFunction == CODECHAL_FUNCTION_FEI_ENC))
                    {
                        DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (pBuf->uiType == VAEncFEICTBCmdBufferType) ? FEI_ENC_BUFFER_TYPE_CTB_CMD   :
                                                          ((pBuf->uiType == VAEncFEICURecordBufferType) ? FEI_ENC_BUFFER_TYPE_CU_RECORD :
                                                                                                          FEI_ENC_BUFFER_TYPE_DISTORTION);
                        DdiEncode_RemoveFromEncStatusReportQueue(pEncCtx, pBuf, idx);
                    }
                }
            }
            DdiMediaUtil_FreeBuffer(pBuf);
            break;
        default: // do not handle any un-listed buffer type
            MOS_FreeMemory(pBuf->pData);
            break;
            //return VA_STATUS_SUCCESS;
    }
    MOS_FreeMemory(pBuf);

    DdiMedia_DestroyBufFromVABufferID(pMediaCtx, buffer_id);

    return VA_STATUS_SUCCESS;
}


/*
 * Get ready to decode a picture to a target surface
 */
static VAStatus DdiMedia_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         render_target
)
{
    void                    *pCtx;
    uint32_t                 uiCtxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    PDDI_MEDIA_CONTEXT       pMediaCtx;
    PDDI_MEDIA_SURFACE       pSurface;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "Null ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(pMediaCtx,               "Null pMediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->pSurfaceHeap, "Null pMediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)render_target, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "render_target", VA_STATUS_ERROR_INVALID_SURFACE);

    pCtx  = DdiMedia_GetContextFromContextID(ctx, context, &uiCtxType);

    pSurface = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, render_target);
    DDI_CHK_NULL(pSurface, "Null pSurface", VA_STATUS_ERROR_INVALID_SURFACE);

    DdiMediaUtil_LockMutex(&pMediaCtx->SurfaceMutex);
    pSurface->curCtxType = uiCtxType;
    pSurface->curStatusReportQueryState = DDI_MEDIA_STATUS_REPORT_QUREY_STATE_PENDING;
    if(uiCtxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        pSurface->curStatusReport.vpp.status = VPREP_NOTAVAILABLE;
    }
    DdiMediaUtil_UnLockMutex(&pMediaCtx->SurfaceMutex);

    switch (uiCtxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        case DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER:
            return DdiDecode_BeginPicture(ctx, context, render_target);
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            return DdiEncode_BeginPicture(ctx, context, render_target);
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            return DdiVp_BeginPicture(ctx, context, render_target);
        default:
            DDI_ASSERTMESSAGE("DDI: unsupported context in DdiCodec_BeginPicture.");
            return VA_STATUS_ERROR_INVALID_CONTEXT;
    }
}

/*
 * Send decode buffers to the server.
 * Buffers are automatically destroyed afterwards
 */
static VAStatus DdiMedia_RenderPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID         *buffers,
    int32_t             num_buffers
)
{

    uint32_t                         uiCtxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    PDDI_MEDIA_CONTEXT               pMediaCtx;
    int32_t                          i;
    void                            *pCtx;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(  ctx,            "Null ctx",                   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(  buffers,        "Null buffers",               VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(num_buffers, 0, "Invalid number buffers",     VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,              "Null pMediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->pBufferHeap, "Null pMediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    for(i = 0; i < num_buffers; i++)
    {
       DDI_CHK_LESS((uint32_t)buffers[i], pMediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid Buffer", VA_STATUS_ERROR_INVALID_BUFFER);
    }

    pCtx  = DdiMedia_GetContextFromContextID(ctx, context, &uiCtxType);

    switch (uiCtxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        case DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER:
            return DdiDecode_RenderPicture(ctx, context, buffers, num_buffers);
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            return DdiEncode_RenderPicture(ctx, context, buffers, num_buffers);
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            return DdiVp_RenderPicture(ctx, context, buffers, num_buffers);
        default:
            DDI_ASSERTMESSAGE("DDI: unsupported context in DdiCodec_RenderPicture.");
            return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

}


/*
 * Make the end of rendering for a picture.
 * The server should start processing all pending operations for this
 * surface. This call is non-blocking. The client can start another
 * Begin/Render/End sequence on a different render target.
 */
static VAStatus DdiMedia_EndPicture (
    VADriverContextP    ctx,
    VAContextID         context
)
{

    void       *pCtx;
    uint32_t    uiCtxType = DDI_MEDIA_CONTEXT_TYPE_NONE;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "Null ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pCtx  = DdiMedia_GetContextFromContextID(ctx, context, &uiCtxType);

    switch (uiCtxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        case DDI_MEDIA_CONTEXT_TYPE_CENC_DECODER:
            return DdiDecode_EndPicture(ctx, context);
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            return DdiEncode_EndPicture(ctx, context);
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            return DdiVp_EndPicture(ctx, context);
        default:
            DDI_ASSERTMESSAGE("DDI: unsupported context in DdiCodec_EndPicture.");
            return VA_STATUS_ERROR_INVALID_CONTEXT;
    }
}

/*
 * This function blocks until all pending operations on the render target
 * have been completed.  Upon return it is safe to use the render target for a
 * different picture.
 */
static VAStatus DdiMedia_SyncSurface (
    VADriverContextP    ctx,
    VASurfaceID         render_target
)
{
    PDDI_MEDIA_CONTEXT              pMediaCtx;
    DDI_MEDIA_SURFACE              *pSurface;
    DDI_MEDIA_SURFACE              *pTempSurface;
    PDDI_DECODE_CONTEXT             pDecCtx    = nullptr;
    Codechal                       *pCodecHal  = nullptr;
    CodechalDecode                 *pDecoder = nullptr;
    CodechalDecodeStatus           *pDecStatus = nullptr;
    CodechalDecodeStatusReport     *pDecStatusReport = nullptr;
    MOS_STATUS                      eStatus = MOS_STATUS_UNKNOWN;
    int32_t                         i, j, index;
    uint32_t                        uNumAvailableReport = 0, uNumCompletedReport = 0;
    uint32_t                        TIMEOUT_NS = 100000000;
    MOS_LINUX_BO                   *bo = nullptr;
    CodechalDecodeStatusReport      tempNewReport;
    PDDI_MEDIA_SURFACE_HEAP_ELEMENT pMediaSurfaceHeapElmt = nullptr;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,    "Null ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,               "Null pMediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->pSurfaceHeap, "Null pMediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)render_target, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid render_target", VA_STATUS_ERROR_INVALID_SURFACE);

    pSurface  = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, render_target);
    DDI_CHK_NULL(pSurface,    "Null pSurface",      VA_STATUS_ERROR_INVALID_CONTEXT);
    if (pSurface->pCurrentFrameSemaphore)
    {
        DdiMediaUtil_WaitSemaphore(pSurface->pCurrentFrameSemaphore);
        DdiMediaUtil_PostSemaphore(pSurface->pCurrentFrameSemaphore);
    }

    // check the bo here?
    // zero is a expected return value
    while (0 != mos_gem_bo_wait(pSurface->bo, TIMEOUT_NS))
    {
        // Just loop while gem_bo_wait times-out.
    }

    pDecCtx = (PDDI_DECODE_CONTEXT)pSurface->pDecCtx;
    if (pDecCtx && pSurface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_DECODER)
    {
        DDI_CHK_NULL(pCodecHal = pDecCtx->pCodecHal,
            "Null pDecCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);
        DDI_CHK_NULL(pDecoder = dynamic_cast<CodechalDecode *>(pCodecHal),
            "Null pCodecHal->pDecoder", VA_STATUS_ERROR_INVALID_CONTEXT);

        if (pDecoder->IsStatusQueryReportingEnabled() && pDecoder->GetStandard() != CODECHAL_JPEG)
        {
            if (pSurface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUREY_STATE_PENDING)
            {
                CodechalDecodeStatusBuffer *decodeStatusBuf = pDecoder->GetDecodeStatusBuf();
                uNumAvailableReport = (decodeStatusBuf->m_currIndex - decodeStatusBuf->m_firstIndex) & (CODECHAL_DECODE_STATUS_NUM - 1);
                DDI_CHK_CONDITION((uNumAvailableReport == 0),
                    "No report available at all", VA_STATUS_ERROR_OPERATION_FAILED);

                for (i = 0; i < uNumAvailableReport; i++)
                {
                    index = (decodeStatusBuf->m_firstIndex + i) & (CODECHAL_DECODE_STATUS_NUM - 1);
                    if ((decodeStatusBuf->m_decodeStatus[index].m_decodeStatusReport.m_currDecodedPicRes.bo == pSurface->bo) ||
                        (pDecoder->GetStandard() == CODECHAL_VC1 && decodeStatusBuf->m_decodeStatus[index].m_decodeStatusReport.m_deblockedPicResOlp.bo == pSurface->bo))
                    {
                        break;
                    }
                }

                DDI_CHK_CONDITION((i == uNumAvailableReport),
                    "No report available for this surface", VA_STATUS_ERROR_OPERATION_FAILED);

                uNumCompletedReport = i+1;

                for (i = 0; i < uNumCompletedReport; i++)
                {
                    eStatus = pDecoder->GetStatusReport(&tempNewReport, 1);
                    DDI_CHK_CONDITION(MOS_STATUS_SUCCESS != eStatus, "Get status report fail", VA_STATUS_ERROR_OPERATION_FAILED);

                    bo = tempNewReport.m_currDecodedPicRes.bo;

                    if (pDecoder->GetStandard() == CODECHAL_VC1)
                    {
                        bo = (tempNewReport.m_deblockedPicResOlp.bo) ? tempNewReport.m_deblockedPicResOlp.bo : bo;
                    }

                    if ((tempNewReport.m_codecStatus == CODECHAL_STATUS_SUCCESSFUL) || (tempNewReport.m_codecStatus == CODECHAL_STATUS_ERROR) || (tempNewReport.m_codecStatus == CODECHAL_STATUS_INCOMPLETE))
                    {
                        DdiMediaUtil_LockMutex(&pMediaCtx->SurfaceMutex);
                        pMediaSurfaceHeapElmt = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)pMediaCtx->pSurfaceHeap->pHeapBase;

                        for (j = 0; j < pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements; j++, pMediaSurfaceHeapElmt++)
                        {
                            if (pMediaSurfaceHeapElmt != nullptr && 
                                    pMediaSurfaceHeapElmt->pSurface != nullptr && 
                                    bo == pMediaSurfaceHeapElmt->pSurface->bo)
                            {
                                pMediaSurfaceHeapElmt->pSurface->curStatusReport.decode.status = (uint32_t)tempNewReport.m_codecStatus;
                                pMediaSurfaceHeapElmt->pSurface->curStatusReport.decode.errMbNum = (uint32_t)tempNewReport.m_numMbsAffected;
                                pMediaSurfaceHeapElmt->pSurface->curStatusReport.decode.crcValue = (pDecoder->GetStandard() == CODECHAL_AVC)?(uint32_t)tempNewReport.m_frameCrc:0;
                                pMediaSurfaceHeapElmt->pSurface->curStatusReportQueryState = DDI_MEDIA_STATUS_REPORT_QUREY_STATE_COMPLETED;
                                break;
                            }
                        }

                        if (j == pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements)
                        {
                            DdiMediaUtil_UnLockMutex(&pMediaCtx->SurfaceMutex);
                            return VA_STATUS_ERROR_OPERATION_FAILED;
                        }
                        DdiMediaUtil_UnLockMutex(&pMediaCtx->SurfaceMutex);
                    }
                    else
                    {
                        // return failed if queried INCOMPLETE or UNAVAILABLE report.
                        return VA_STATUS_ERROR_OPERATION_FAILED;
                    }
                }
            }

            // check the report ptr of current surface.
            if (pSurface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUREY_STATE_COMPLETED)
            {
                if (pSurface->curStatusReport.decode.status == CODECHAL_STATUS_ERROR)
                {
                    return VA_STATUS_ERROR_DECODING_ERROR;
                }
                else if (pSurface->curStatusReport.decode.status == CODECHAL_STATUS_INCOMPLETE || pSurface->curStatusReport.decode.status == CODECHAL_STATUS_UNAVAILABLE)
                {
                    return VA_STATUS_ERROR_HW_BUSY;
                }
            }
            else
            {
                return VA_STATUS_ERROR_OPERATION_FAILED;
            }
        }
    }

    if (pSurface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        PDDI_VP_CONTEXT             pVpCtx;
        QUERY_STATUS_REPORT_APP     tempVpReport;
        uint32_t                    uiTableLen;

        pVpCtx = (PDDI_VP_CONTEXT)pSurface->pVpCtx;
        DDI_CHK_NULL(pVpCtx ,        "Null pVpCtx",         VA_STATUS_ERROR_INVALID_CONTEXT);
        DDI_CHK_NULL(pVpCtx->pVpHal ,"Null pVpCtx->pVpHal", VA_STATUS_ERROR_INVALID_CONTEXT);

        MOS_ZeroMemory(&tempVpReport, sizeof(QUERY_STATUS_REPORT_APP));
        
        // Get reported surfaces' count
        uiTableLen = 0;
        pVpCtx->pVpHal->GetStatusReportEntryLength(&uiTableLen);

        if (uiTableLen > 0 && pSurface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUREY_STATE_PENDING)
        {
            // Query the status for all of surfaces which have finished
            for(i = 0; i < uiTableLen; i++)
            { 
                MOS_ZeroMemory(&tempVpReport, sizeof(QUERY_STATUS_REPORT_APP));
                pVpCtx->pVpHal->GetStatusReport(&tempVpReport, 1);

                // StatusFeedBackID is last time submitted Target Surface ID which is set in BeginPicture,
                // So we can know the report is for which surface here.
                pTempSurface = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, tempVpReport.StatusFeedBackID);
                if(pTempSurface == nullptr)
                {
                    return VA_STATUS_ERROR_OPERATION_FAILED;
                }

                // Update the status of the surface which is reported.
                pTempSurface->curStatusReport.vpp.status = (uint32_t)tempVpReport.dwStatus;
                pTempSurface->curStatusReportQueryState  = DDI_MEDIA_STATUS_REPORT_QUREY_STATE_COMPLETED;

                if(tempVpReport.StatusFeedBackID == render_target)
                {
                    break;
                }
            }
        }

        if (pSurface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUREY_STATE_COMPLETED)
        {
            if(pSurface->curStatusReport.vpp.status == VPREP_OK)
            {
                return VA_STATUS_SUCCESS;
            }
            else if(pSurface->curStatusReport.vpp.status == VPREP_NOTREADY)
            {
                return VA_STATUS_ERROR_HW_BUSY;
            }
            else
            {
                return VA_STATUS_ERROR_OPERATION_FAILED;
            }
        }
        else
        {
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }
    }

    return VA_STATUS_SUCCESS;
}

/*
 * Find out any pending ops on the render target
 */
static VAStatus DdiMedia_QuerySurfaceStatus (
    VADriverContextP    ctx,
    VASurfaceID         render_target,
    VASurfaceStatus    *status
)
{
    PDDI_MEDIA_CONTEXT     pMediaCtx;
    DDI_MEDIA_SURFACE     *pSurface;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,    "Null ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(status, "Null status", VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,                  "Null pMediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->pSurfaceHeap,    "Null pMediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)render_target, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid render_target", VA_STATUS_ERROR_INVALID_SURFACE);
    pSurface   = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, render_target);
    DDI_CHK_NULL(pSurface,    "Null pSurface",    VA_STATUS_ERROR_INVALID_SURFACE);

    if (pSurface->pDecCtx)
    {
        auto pDecCtx = (PDDI_DECODE_CONTEXT)pSurface->pDecCtx;
        DDI_CHK_NULL(pDecCtx, "Null pDecCtx", VA_STATUS_ERROR_INVALID_SURFACE);
        DDI_CHK_NULL(pDecCtx->pCpDdiInterface, "Null pCpDdiInterface", VA_STATUS_ERROR_INVALID_SURFACE);

        DDI_CHK_RET(pDecCtx->pCpDdiInterface->QueryCencStatus(ctx, status), "Fail to query Decrypt status!");
    }

    if (pSurface->pCurrentFrameSemaphore)
    {
        if(DdiMediaUtil_TryWaitSemaphore(pSurface->pCurrentFrameSemaphore) == 0)
        {
            DdiMediaUtil_PostSemaphore(pSurface->pCurrentFrameSemaphore);
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
    if(mos_bo_busy(pSurface->bo))
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

/*
 * Report MB error info
 */
VAStatus DdiMedia_QuerySurfaceError(
    VADriverContextP ctx,
    VASurfaceID      render_target,
    VAStatus         error_status,
    void             **error_info /*out*/
)
{
    PDDI_MEDIA_CONTEXT             pMediaCtx      = nullptr;
    DDI_MEDIA_SURFACE             *pSurface       = nullptr;
    PDDI_DECODE_CONTEXT            pDecCtx        = nullptr;
    VASurfaceDecodeMBErrors       *pSurfaceErrors = nullptr;
    CodechalDecode                *decoder        = nullptr;
    int32_t                        i;
    DDI_UNUSED(error_status);

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL( ctx, "Null ctx", VA_STATUS_ERROR_INVALID_CONTEXT );

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL( pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pSurface = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, render_target);
    DDI_CHK_NULL(pSurface, "Null pSurface", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL( pDecCtx = (PDDI_DECODE_CONTEXT)pSurface->pDecCtx,
            "Null pSurface->pDecCtx", VA_STATUS_ERROR_INVALID_CONTEXT );

    pSurfaceErrors   = pDecCtx->vaSurfDecErrOutput;

    DdiMediaUtil_LockMutex(&pMediaCtx->SurfaceMutex);
    if (pSurface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUREY_STATE_COMPLETED)
    {
        if (error_status == -1 && pSurface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_DECODER)
            //&& pSurface->curStatusReport.decode.status == CODECHAL_STATUS_SUCCESSFUL)  // get the crc value whatever the status is
        {
            decoder = dynamic_cast<CodechalDecode *>(pDecCtx->pCodecHal);
            DDI_CHK_NULL(decoder, "Null codechal decoder", VA_STATUS_ERROR_INVALID_CONTEXT);
            if (decoder->GetStandard() != CODECHAL_AVC)
            {
                return VA_STATUS_ERROR_UNIMPLEMENTED;
            }
            *error_info = (void *)&pSurface->curStatusReport.decode.crcValue;
            DdiMediaUtil_UnLockMutex(&pMediaCtx->SurfaceMutex);
            return VA_STATUS_SUCCESS;
        }

        if (error_status != -1 && pSurface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_DECODER &&
            pSurface->curStatusReport.decode.status == CODECHAL_STATUS_ERROR)
        {
            pSurfaceErrors[1].status            = -1;
            pSurfaceErrors[0].status            = 2;
            pSurfaceErrors[0].start_mb          = 0;
            pSurfaceErrors[0].end_mb            = 0;
            pSurfaceErrors[0].num_mb            = pSurface->curStatusReport.decode.errMbNum;
            pSurfaceErrors[0].decode_error_type = VADecodeMBError;
            *error_info = pSurfaceErrors;
            DdiMediaUtil_UnLockMutex(&pMediaCtx->SurfaceMutex);
            return VA_STATUS_SUCCESS;
        }

        if (pSurface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP &&
            pSurface->curStatusReport.vpp.status == CODECHAL_STATUS_ERROR)
        {
            DdiMediaUtil_UnLockMutex(&pMediaCtx->SurfaceMutex);
            return VA_STATUS_SUCCESS;
        }
    }

    pSurfaceErrors[0].status = -1;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->SurfaceMutex);
    return VA_STATUS_SUCCESS;
}

/*
 * Query surface attributes for the supplied config
 */
static VAStatus
DdiMedia_QuerySurfaceAttributes(
    VADriverContextP ctx,
    VAConfigID config_id,
    VASurfaceAttrib *attrib_list,
    uint32_t *num_attribs
)
{
    PDDI_MEDIA_CONTEXT      pMediaCtx;
    VAEntrypoint            entrypoint;
    VAProfile               profile;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,         "Null ctx",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(num_attribs, "Null num_attribs", VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,   "Null pMediaCtx",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->m_caps, "Null m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return pMediaCtx->m_caps->QuerySurfaceAttributes(config_id,
            attrib_list, num_attribs);
}

static VAStatus DdiMedia_PutSurface(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void*            draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,        /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
)
{
    PDDI_MEDIA_CONTEXT              pMediaDrvCtx;
    void                           *pVpCtx;
    uint32_t                        uiCtxType;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "Null ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    if(number_cliprects > 0)
    {
        DDI_CHK_NULL(cliprects, "Null cliprects", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    pVpCtx         = nullptr;
    pMediaDrvCtx   = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(pMediaDrvCtx,               "Null pMediaDrvCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaDrvCtx->pSurfaceHeap, "Null pMediaDrvCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)surface, pMediaDrvCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    if (nullptr != pMediaDrvCtx->pVpCtxHeap->pHeapBase)
    {
        pVpCtx = DdiMedia_GetContextFromContextID(ctx, (VAContextID)(0 + DDI_MEDIA_VACONTEXTID_OFFSET_VP), &uiCtxType);
    }

#ifdef ANDROID
    if(nullptr != pVpCtx)
    {
        return DdiCodec_PutSurfaceAndroidExt(
          ctx, surface, draw, srcx, srcy, srcw, srch, destx, desty, destw, desth, cliprects, number_cliprects, flags);

    }
    else
    {
        return DdiCodec_PutSurfaceAndroid(
          ctx, surface, draw, srcx, srcy, srcw, srch, destx, desty, destw, desth, cliprects, number_cliprects, flags);
    }
#else
    if(nullptr != pVpCtx)
    {
        return DdiCodec_PutSurfaceLinuxHW(
                ctx, surface, draw, srcx, srcy, srcw, srch, destx, desty, destw, desth, cliprects, number_cliprects, flags);

    }
    else
    {
        return DdiMedia_PutSurfaceLinuxSW(
          ctx, surface, draw, srcx, srcy, srcw, srch, destx, desty, destw, desth, cliprects, number_cliprects, flags);
    }
#endif

}

/* List all the VAImageFormats supported during vaCreateSurfaces
 *  It can be used by vaQueryImageFormats and other functions
 */
/*
 * Query supported image formats
 * The caller must provide a "format_list" array that can hold at
 * least vaMaxNumImageFormats() entries. The actual number of formats
 * returned in "format_list" is returned in "num_formats".
 */
static VAStatus DdiMedia_QueryImageFormats (
    VADriverContextP    ctx,
    VAImageFormat      *format_list,
    int32_t            *num_formats
)
{
    DDI_FUNCTION_ENTER();

    PDDI_MEDIA_CONTEXT pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,   "Null pMediaCtx.",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pMediaCtx->m_caps,   "Null pointer.",   VA_STATUS_ERROR_INVALID_PARAMETER);
    return pMediaCtx->m_caps->QueryImageFormats(format_list, num_formats);
}

///////////////////////////////////////////////////
// Create an image
// ctx[in]:     driver context
// format[in]:  the format of image
// width[in]:   the width of the image
// height[in]:  the height of the image
// image[out]:  the generated image
///////////////////////////////////////////////////
VAStatus DdiMedia_CreateImage(
    VADriverContextP ctx,
    VAImageFormat   *format,
    int32_t          width,
    int32_t          height,
    VAImage         *image     /* out */
)
{
    PDDI_MEDIA_CONTEXT                pMediaCtx;
    VAImage                          *pVAImg;
    DDI_MEDIA_BUFFER                 *pBuf;
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT     pImageHeapElement;
    int32_t                           i;
    int32_t                           iPitch;
    int32_t                           halfwidth, halfheight;
    VAStatus                          status;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT    pBufferHeapElement;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,         "Invalid context!",    VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(format,      "Invalid format!",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(image,       "Invalid image!",      VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(width,  0, "Invalid width!",      VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(height, 0, "Invalid height!",     VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx        = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,   "Null pMediaCtx.",   VA_STATUS_ERROR_INVALID_PARAMETER);

    pVAImg           = (VAImage*)MOS_AllocAndZeroMemory(sizeof(VAImage));
    DDI_CHK_NULL(pVAImg,  "Insufficient to allocate an VAImage.",  VA_STATUS_ERROR_ALLOCATION_FAILED);
    pVAImg->format   = *format;

    if(pVAImg->format.fourcc == VA_FOURCC_RGBA || pVAImg->format.fourcc == VA_FOURCC_BGRA)
    {
        iPitch = width * 4;
        pVAImg->format.byte_order        = VA_LSB_FIRST;
        pVAImg->format.bits_per_pixel    = 32;
        pVAImg->width                    = width;
        pVAImg->height                   = height;
        pVAImg->data_size                = iPitch * height;
        pVAImg->num_planes               = 1;
        pVAImg->pitches[0]               = iPitch;
    }
    else if (pVAImg->format.fourcc == VA_FOURCC_YV12)
    {
        iPitch      = width;
        halfwidth   = (width  + 1) / 2;
        halfheight  = (height + 1) / 2;
        pVAImg->format.byte_order        = VA_LSB_FIRST;
        pVAImg->format.bits_per_pixel    = 12;
        pVAImg->width                    = width;
        pVAImg->height                   = height;
        //should be width*height  + 2*width2*height2;
        pVAImg->data_size                = width * height + 4 * halfwidth * halfheight;
        pVAImg->num_planes               = 3;
        pVAImg->pitches[0]               = iPitch;
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = halfwidth;
        pVAImg->offsets[1]               = height * iPitch;
        pVAImg->offsets[2]               = height * iPitch + halfwidth * halfheight;
    }
    else if(pVAImg->format.fourcc == VA_FOURCC_YUY2)
    {
        iPitch = 2 * width;
        pVAImg->format.byte_order        = VA_LSB_FIRST;
        pVAImg->format.bits_per_pixel    = 16;
        pVAImg->width                    = width;
        pVAImg->height                   = height;
        pVAImg->data_size                = iPitch * height;
        pVAImg->num_planes               = 1;
        pVAImg->pitches[0]               = iPitch;
        pVAImg->pitches[1]               = 0;
        pVAImg->pitches[2]               = 0;
        pVAImg->offsets[1]               = 0;
        pVAImg->offsets[2]               = 0;

    }
    else if(pVAImg->format.fourcc == VA_FOURCC_NV12)
    {
        iPitch = MOS_ALIGN_CEIL(width, 128);

        pVAImg->format.byte_order        = VA_LSB_FIRST;
        pVAImg->format.bits_per_pixel    = format->bits_per_pixel;
        pVAImg->width                    = width;
        pVAImg->height                   = height;
        pVAImg->data_size                = iPitch * height * 3 / 2;
        pVAImg->num_planes               = 2;
        pVAImg->pitches[0]               = iPitch;
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = iPitch;
#if UFO_GRALLOC_NEW_FORMAT
        pVAImg->offsets[1]               = MOS_ALIGN_CEIL(height,64) * iPitch;
#else
        pVAImg->offsets[1]               = MOS_ALIGN_CEIL(height,32) * iPitch;
#endif
        pVAImg->offsets[2]               = pVAImg->offsets[1] + 1;
    }
    else if(pVAImg->format.fourcc == VA_FOURCC_NV21)
    {
        iPitch = width;

        pVAImg->format.byte_order        = VA_LSB_FIRST;
        pVAImg->format.bits_per_pixel    = format->bits_per_pixel;
        pVAImg->width                    = width;
        pVAImg->height                   = height;
        pVAImg->data_size                = iPitch * height * 3 / 2;
        pVAImg->num_planes               = 2;
        pVAImg->pitches[0]               = iPitch;
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = iPitch;
        pVAImg->offsets[1]               = MOS_ALIGN_CEIL(height,32) * iPitch;
        pVAImg->offsets[2]               = pVAImg->offsets[1] + 1;
    }
    else if(pVAImg->format.fourcc == VA_FOURCC('P','0','1','0'))
    {
        iPitch = MOS_ALIGN_CEIL(width, 128) * 2;

        pVAImg->format.byte_order        = VA_LSB_FIRST;
        pVAImg->format.bits_per_pixel    = format->bits_per_pixel;
        pVAImg->width                    = width;
        pVAImg->height                   = height;
        pVAImg->data_size                = iPitch * height * 3;
        pVAImg->num_planes               = 2;
        pVAImg->pitches[0]               = iPitch;
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = iPitch;
        pVAImg->offsets[1]               = MOS_ALIGN_CEIL(height,32) * iPitch * 2;
        pVAImg->offsets[2]               = pVAImg->offsets[1] + 2;
    }
    else
    {
       MOS_FreeMemory(pVAImg);
       return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    pBuf                    = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    if (nullptr == pBuf)
    {
        MOS_FreeMemory(pVAImg);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    pBuf->iNumElements      = 1;
    pBuf->iSize             = pVAImg->data_size;
    pBuf->uiType            = VAImageBufferType;
    pBuf->format            = Media_Format_CPU;//DdiCodec_OsFormatToMediaFormat(pVAImg->format.fourcc); //Media_Format_Buffer;
    pBuf->uiOffset          = 0;
    pBuf->pMediaCtx         = pMediaCtx;

    //Put Image in untiled buffer for better CPU access?
    if( (status = DdiMediaUtil_CreateBuffer(pBuf,  pMediaCtx->pDrmBufMgr) != VA_STATUS_SUCCESS))
    {
        MOS_FreeMemory(pVAImg);
        MOS_FreeMemory(pBuf);
        return status;
    }
    pBuf->TileType     = I915_TILING_NONE;

    DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
    pBufferHeapElement  = DdiMediaUtil_AllocPMediaBufferFromHeap(pMediaCtx->pBufferHeap);

    if (nullptr == pBufferHeapElement)
    {
        DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
        MOS_FreeMemory(pVAImg);
        DdiMediaUtil_FreeBuffer(pBuf);
        MOS_FreeMemory(pBuf);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    pBufferHeapElement->pBuffer   = pBuf;
    pBufferHeapElement->pCtx      = nullptr;
    pBufferHeapElement->uiCtxType = DDI_MEDIA_CONTEXT_TYPE_MEDIA;

    pVAImg->buf                   = pBufferHeapElement->uiVaBufferID;
    pMediaCtx->uiNumBufs++;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);

    DdiMediaUtil_LockMutex(&pMediaCtx->ImageMutex);
    pImageHeapElement             = DdiMediaUtil_AllocPVAImageFromHeap(pMediaCtx->pImageHeap);
    if (nullptr == pImageHeapElement)
    {
        DdiMediaUtil_UnLockMutex(&pMediaCtx->ImageMutex);
        MOS_FreeMemory(pVAImg);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    pImageHeapElement->pImage     = pVAImg;
    pMediaCtx->uiNumImages++;
    pVAImg->image_id              = pImageHeapElement->uiVaImageID;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->ImageMutex);

   *image = *pVAImg;
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_DeriveImage (
    VADriverContextP  ctx,
    VASurfaceID       surface,
    VAImage           *image
)
{
    PDDI_MEDIA_CONTEXT             pMediaCtx;
    DDI_MEDIA_SURFACE              *pSurface;
    VAImage                        *pVAImg;
    DDI_MEDIA_BUFFER               *pBuf = nullptr;
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT  pImageHeapElement;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT pBufferHeapElement;
    VAStatus                       vaStatus;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,   "Null ctx",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(image, "Null image", VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx        = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(pMediaCtx->pSurfaceHeap, "Null pMediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    pSurface         = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, surface);
    DDI_CHK_NULL(pSurface, "Null pSurface", VA_STATUS_ERROR_INVALID_SURFACE);

    pVAImg           = (VAImage*)MOS_AllocAndZeroMemory(sizeof(VAImage));
    DDI_CHK_NULL(pVAImg, "Null pVAImg", VA_STATUS_ERROR_ALLOCATION_FAILED);

    if (pSurface->pCurrentFrameSemaphore)
    {
        DdiMediaUtil_WaitSemaphore(pSurface->pCurrentFrameSemaphore);
        DdiMediaUtil_PostSemaphore(pSurface->pCurrentFrameSemaphore);
    }
    DdiMediaUtil_LockMutex(&pMediaCtx->ImageMutex);
    pImageHeapElement                = DdiMediaUtil_AllocPVAImageFromHeap(pMediaCtx->pImageHeap);
    if (nullptr == pImageHeapElement)
    {
        DdiMediaUtil_UnLockMutex(&pMediaCtx->ImageMutex);
        vaStatus = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        goto CleanUpandReturn;
    }
    pImageHeapElement->pImage        = pVAImg;
    pMediaCtx->uiNumImages++;
    pVAImg->image_id                 = pImageHeapElement->uiVaImageID;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->ImageMutex);

    pVAImg->format.fourcc            = DdiMedia_MediaFormatToOsFormat(pSurface->format);
    pVAImg->width                    = pSurface->iWidth;
    pVAImg->height                   = pSurface->iRealHeight;
    pVAImg->format.byte_order        = VA_LSB_FIRST;

    switch( pSurface->format )
    {
    case Media_Format_YV12:
        pVAImg->format.bits_per_pixel    = 12;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight * 3 / 2;
        pVAImg->num_planes               = 3;
        pVAImg->pitches[0]               = pSurface->iPitch;
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = pSurface->iPitch / 2;
        pVAImg->offsets[1]               = pSurface->iHeight * pSurface->iPitch;
        pVAImg->offsets[2]               = pSurface->iPitch * pSurface->iHeight * 5 / 4;
        break;
    case Media_Format_I420:
        pVAImg->format.bits_per_pixel    = 12;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight * 3 / 2;
        pVAImg->num_planes               = 3;
        pVAImg->pitches[0]               = pSurface->iPitch;
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = pSurface->iPitch / 2;
        pVAImg->offsets[1]               = pSurface->iPitch * pSurface->iHeight * 5 / 4;
        pVAImg->offsets[2]               = pSurface->iHeight * pSurface->iPitch;
        break;
    case Media_Format_A8B8G8R8:
    case Media_Format_A8R8G8B8:
        pVAImg->format.bits_per_pixel    = 32;
        pVAImg->format.alpha_mask        = RGB_8BIT_ALPHAMASK;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight;
        pVAImg->num_planes               = 1;
        pVAImg->pitches[0]               = pSurface->iPitch;
        break;
    case Media_Format_X8R8G8B8:
    case Media_Format_X8B8G8R8:
        pVAImg->format.bits_per_pixel    = 32;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight;
        pVAImg->num_planes               = 1;
        pVAImg->pitches[0]               = pSurface->iPitch;
        break;
    case Media_Format_R10G10B10A2:
    case Media_Format_B10G10R10A2:
        pVAImg->format.bits_per_pixel    = 32;
        pVAImg->format.alpha_mask        = RGB_10BIT_ALPHAMASK;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight;
        pVAImg->num_planes               = 1;
        pVAImg->pitches[0]               = pSurface->iPitch;
        break;
   case Media_Format_R5G6B5:
        pVAImg->format.bits_per_pixel    = 16;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight;
        pVAImg->num_planes               = 1;
        pVAImg->pitches[0]               = pSurface->iPitch;
        break;
   case Media_Format_R8G8B8:
        pVAImg->format.bits_per_pixel    = 24;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight;
        pVAImg->num_planes               = 1;
        pVAImg->pitches[0]               = pSurface->iPitch;
        break;
     case Media_Format_YUY2:
        pVAImg->format.bits_per_pixel    = 16;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight;
        pVAImg->num_planes               = 1;
        pVAImg->pitches[0]               = pSurface->iPitch;
        break;
     case Media_Format_400P:
        pVAImg->format.bits_per_pixel    = 8;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight;
        pVAImg->num_planes               = 1;
        pVAImg->pitches[0]               = pSurface->iPitch;
        break;
    case Media_Format_444P:
        pVAImg->format.bits_per_pixel    = 24;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight * 3;
        pVAImg->num_planes               = 3;
        pVAImg->pitches[0]               =
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = pSurface->iPitch;
        pVAImg->offsets[1]               = pSurface->iHeight * pSurface->iPitch;
        pVAImg->offsets[2]               = pSurface->iHeight * pSurface->iPitch * 2;
        break;
    case Media_Format_IMC3:
        pVAImg->format.bits_per_pixel    = 12;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight * 2;
        pVAImg->num_planes               = 3;
        pVAImg->pitches[0]               =
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = pSurface->iPitch;
        pVAImg->offsets[1]               = pSurface->iHeight * pSurface->iPitch;
        pVAImg->offsets[2]               = pSurface->iHeight * pSurface->iPitch * 3 / 2;
        break;
    case Media_Format_411P:
        pVAImg->format.bits_per_pixel    = 12;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight * 3;
        pVAImg->num_planes               = 3;
        pVAImg->pitches[0]               =
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = pSurface->iPitch;
        pVAImg->offsets[1]               = pSurface->iHeight * pSurface->iPitch;
        pVAImg->offsets[2]               = pSurface->iHeight * pSurface->iPitch * 2;
        break;
    case Media_Format_422V:
        pVAImg->format.bits_per_pixel    = 16;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight * 2;
        pVAImg->num_planes               = 3;
        pVAImg->pitches[0]               =
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = pSurface->iPitch;
        pVAImg->offsets[1]               = pSurface->iHeight * pSurface->iPitch;
        pVAImg->offsets[2]               = pSurface->iHeight * pSurface->iPitch * 3 / 2;
        break;
    case Media_Format_422H:
        pVAImg->format.bits_per_pixel    = 16;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight * 3;
        pVAImg->num_planes               = 3;
        pVAImg->pitches[0]               =
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = pSurface->iPitch;
        pVAImg->offsets[1]               = pSurface->iHeight * pSurface->iPitch;
        pVAImg->offsets[2]               = pSurface->iHeight * pSurface->iPitch * 2;
        break;
     case Media_Format_P010:
         pVAImg->format.bits_per_pixel    = 24;
         pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight * 3;
         pVAImg->num_planes               = 2;
         pVAImg->pitches[0]               = pSurface->iPitch;
         pVAImg->pitches[1]               =
         pVAImg->pitches[2]               = pSurface->iPitch;
         pVAImg->offsets[1]               = pSurface->iHeight * pSurface->iPitch;
         pVAImg->offsets[2]               = pVAImg->offsets[1] + 2;
        break;
     default:
        pVAImg->format.bits_per_pixel    = 12;
        pVAImg->data_size                = pSurface->iPitch * pSurface->iHeight * 3 / 2;
        pVAImg->num_planes               = 2;
        pVAImg->pitches[0]               = pSurface->iPitch;
        pVAImg->pitches[1]               =
        pVAImg->pitches[2]               = pSurface->iPitch;
        pVAImg->offsets[1]               = pSurface->iHeight * pSurface->iPitch;
        pVAImg->offsets[2]               = pVAImg->offsets[1] + 1;
        break;
    }

    pBuf               = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
	if (pBuf == nullptr)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }
    pBuf->iNumElements = 1;
    pBuf->iSize        = pVAImg->data_size;
    pBuf->uiType       = VABufferTypeMax;
    pBuf->format       = pSurface->format;
    pBuf->uiOffset     = 0;

    pBuf->bo           = pSurface->bo;
    pBuf->format       = pSurface->format;
    pBuf->TileType     = pSurface->TileType;
    pBuf->pSurface     = pSurface;
    mos_bo_reference(pSurface->bo);

    DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
    pBufferHeapElement = DdiMediaUtil_AllocPMediaBufferFromHeap(pMediaCtx->pBufferHeap);

    if (nullptr == pBufferHeapElement)
    {
        DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
        vaStatus = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        goto CleanUpandReturn;
    }
    pBufferHeapElement->pBuffer    = pBuf;
    pBufferHeapElement->pCtx       = nullptr;
    pBufferHeapElement->uiCtxType  = DDI_MEDIA_CONTEXT_TYPE_MEDIA;

    pVAImg->buf             = pBufferHeapElement->uiVaBufferID;
    pMediaCtx->uiNumBufs++;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);

    *image = *pVAImg;

    return VA_STATUS_SUCCESS;

CleanUpandReturn:

    MOS_FreeMemory(pVAImg);
    MOS_FreeMemory(pBuf);

    return vaStatus;

}

/////////////////////////////////////////////////////////////////////////////
//! \Free allocated surfaceheap elements
//! \params
//! [in] VADriverContextP
//! [in] VAImageID
//! [out] none
//! \returns VA_STATUS_SUCCESS if call succeeds
/////////////////////////////////////////////////////////////////////////////

VAStatus DdiMedia_DestroyImage (
    VADriverContextP ctx,
    VAImageID        image)
{
    VAImage                *pImage;
    PDDI_MEDIA_CONTEXT     pMediaCtx;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "Null ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    pMediaCtx = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(pMediaCtx,             "Null Media",                        VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->pImageHeap, "Null pMediaCtx->pImageHeap",        VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)image, pMediaCtx->pImageHeap->uiAllocatedHeapElements, "Invalid image", VA_STATUS_ERROR_INVALID_IMAGE);

    pImage    = DdiMedia_GetVAImageFromVAImageID(pMediaCtx, image);
    if (pImage == nullptr)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    DdiMedia_DestroyBuffer(ctx, pImage->buf);
    MOS_FreeMemory(pImage);

    DdiMedia_DestroyImageFromVAImageID(pMediaCtx, image);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_SetImagePalette(
    VADriverContextP ctx,
    VAImageID        image,
    unsigned char    *palette
)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(image);
    DDI_UNUSED(palette);
    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}


////////////////////////////////////////////////////
// Retrive surface data into a VAImage
// Image must be in a format supported by the implementation
// ctx[in] : input driver conetxt
// surface[in]: input surface ID of source
// x[in] : x offset of the wanted region
// y[in] : y offset of the wanted region
// width : width of the wanted region
// height: height of the wanted region
// image:  the image ID of the source iamge
////////////////////////////////////////////////////
VAStatus DdiMedia_GetImage(
    VADriverContextP ctx,
    VASurfaceID      surface,
    int32_t          x,     /* coordinates of the upper left source pixel */
    int32_t          y,
    uint32_t         width, /* width and height of the region */
    uint32_t         height,
    VAImageID        image
)
{
    PDDI_MEDIA_CONTEXT            pMediaCtx;
    VAImage                      *pVAImg;
    DDI_MEDIA_SURFACE            *pSurface;
    DDI_MEDIA_BUFFER             *pBuf;
    VAStatus                      status;
    void                         *pSurfData;
    void                         *pImageData;
    MOS_STATUS                    eStatus = MOS_STATUS_SUCCESS;
    DDI_UNUSED(x);
    DDI_UNUSED(y);
    DDI_UNUSED(width);
    DDI_UNUSED(height);

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,                     "Null ctx.",                    VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx       = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,               "Null pMediaCtx.",              VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pMediaCtx->pSurfaceHeap, "Null pMediaCtx->pSurfaceHeap",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(pMediaCtx->pImageHeap,   "Null pMediaCtx->pImageHeap",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)image,   pMediaCtx->pImageHeap->uiAllocatedHeapElements,   "Invalid image",   VA_STATUS_ERROR_INVALID_IMAGE);

    pSurface        = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, surface);
    DDI_CHK_NULL(pSurface,     "Null pSurface.",      VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pSurface->bo, "Null pSurface->bo.",  VA_STATUS_ERROR_INVALID_PARAMETER);

    if (pSurface->pCurrentFrameSemaphore)
    {
        DdiMediaUtil_WaitSemaphore(pSurface->pCurrentFrameSemaphore);
        DdiMediaUtil_PostSemaphore(pSurface->pCurrentFrameSemaphore);
    }

    pVAImg          = DdiMedia_GetVAImageFromVAImageID(pMediaCtx, image);
    DDI_CHK_NULL(pVAImg,       "Null pVAImg.",        VA_STATUS_ERROR_INVALID_PARAMETER);

    pBuf            = DdiMedia_GetBufferFromVABufferID(pMediaCtx, pVAImg->buf);
    DDI_CHK_NULL(pBuf,         "Null pBuf.",          VA_STATUS_ERROR_INVALID_PARAMETER);

    if (pSurface->format != DdiMedia_OsFormatAlphaMaskToMediaFormat(pVAImg->format.fourcc, pVAImg->format.alpha_mask))
    {
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    //Lock Surface
    pSurfData = DdiMediaUtil_LockSurface(pSurface, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));
    if (nullptr == pSurfData)
    {
        return VA_STATUS_ERROR_SURFACE_BUSY;
    }

    status = DdiMedia_MapBuffer(ctx, pVAImg->buf, &pImageData);
    if (status != VA_STATUS_SUCCESS)
    {
        DdiMediaUtil_UnlockSurface(pSurface);
        return VA_STATUS_ERROR_UNKNOWN;
    }

    //copy data from surface to image
    //this is temp solution, will copy by difference size and difference format in further
    eStatus = MOS_SecureMemcpy(pImageData, pVAImg->data_size, pSurfData, pVAImg->data_size);
    DDI_CHK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "DDI:Failed to copy surface to image buffer data!", VA_STATUS_ERROR_OPERATION_FAILED);

    status = DdiMedia_UnmapBuffer(ctx, pVAImg->buf);
    if (status != VA_STATUS_SUCCESS)
    {
        DdiMediaUtil_UnlockSurface(pSurface);
        return VA_STATUS_ERROR_UNKNOWN;
    }

    DdiMediaUtil_UnlockSurface(pSurface);

    return VA_STATUS_SUCCESS;

}

////////////////////////////////////////////////////////////
// Copy data from a VAImage to a surface
// Image must be in a format supported by the implementation
// ctx[in] : input driver conetxt
// surface[in]: surface ID of destination
// image[in]:  the image ID of the destination iamge
// src_x[in] : source x offset of the surface region
// src_y[in] : source y offset of the surface region
// src_width : source width of the surface region
// src_height: source height of the surface region
// dest_x[in] : destination x offset of the image region
// dest_y[in] : destination y offset of the image region
// dest_width[in] : destination width of the image region
// dest_height[in]: destination height of the image region
////////////////////////////////////////////////////////////
VAStatus DdiMedia_PutImage(
    VADriverContextP ctx,
    VASurfaceID      surface,
    VAImageID        image,
    int32_t          src_x,
    int32_t          src_y,
    uint32_t         src_width,
    uint32_t         src_height,
    int32_t          dest_x,
    int32_t          dest_y,
    uint32_t         dest_width,
    uint32_t         dest_height
)
{

    PDDI_MEDIA_CONTEXT            pMediaCtx;
    VAImage                      *pVAImg;
    DDI_MEDIA_SURFACE            *pSurface;
    DDI_MEDIA_BUFFER             *pBuf;
    VAStatus                      status;
    void                         *pSurfData;
    void                         *pImageData;
    MOS_STATUS                    eStatus = MOS_STATUS_SUCCESS;
    DDI_UNUSED(src_x);
    DDI_UNUSED(src_y);
    DDI_UNUSED(src_width);
    DDI_UNUSED(src_height);
    DDI_UNUSED(dest_x);
    DDI_UNUSED(dest_y);
    DDI_UNUSED(dest_width);
    DDI_UNUSED(dest_height);

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,                     "Null ctx.",                    VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx        = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,               "Null pMediaCtx.",              VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pMediaCtx->pSurfaceHeap, "Null pMediaCtx->pSurfaceHeap",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(pMediaCtx->pImageHeap,   "Null pMediaCtx->pImageHeap",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)image, pMediaCtx->pImageHeap->uiAllocatedHeapElements,     "Invalid image",   VA_STATUS_ERROR_INVALID_IMAGE);

    pSurface     = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, surface);
    DDI_CHK_NULL(pSurface, "Null pSurface.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (pSurface->pCurrentFrameSemaphore)
    {
        DdiMediaUtil_WaitSemaphore(pSurface->pCurrentFrameSemaphore);
        DdiMediaUtil_PostSemaphore(pSurface->pCurrentFrameSemaphore);
    }

    pVAImg          = DdiMedia_GetVAImageFromVAImageID(pMediaCtx, image);
    DDI_CHK_NULL(pVAImg,      "Invalid image.",      VA_STATUS_ERROR_INVALID_PARAMETER);

    pBuf = DdiMedia_GetBufferFromVABufferID(pMediaCtx, pVAImg->buf);
    DDI_CHK_NULL(pBuf,       "Invalid buffer.",      VA_STATUS_ERROR_INVALID_PARAMETER);

    if (pSurface->format != DdiMedia_OsFormatAlphaMaskToMediaFormat(pVAImg->format.fourcc,pVAImg->format.alpha_mask))
    {
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    DDI_CHK_NULL(pSurface->bo, "Invalid buffer.", VA_STATUS_ERROR_INVALID_PARAMETER);

    //Lock Surface
    pSurfData = DdiMediaUtil_LockSurface(pSurface, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));
    if (nullptr == pSurfData)
    {
        return VA_STATUS_ERROR_SURFACE_BUSY;
    }

    status = DdiMedia_MapBuffer(ctx, pVAImg->buf, &pImageData);
    if (status != VA_STATUS_SUCCESS)
    {
        DdiMediaUtil_UnlockSurface(pSurface);
        return VA_STATUS_ERROR_UNKNOWN;
    }

    //copy data from image to surferce
    //this is temp solution, will copy by difference size and difference format in further
    eStatus = MOS_SecureMemcpy(pSurfData, pVAImg->data_size, pImageData, pVAImg->data_size);
    DDI_CHK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "DDI:Failed to copy image to surface buffer data!", VA_STATUS_ERROR_OPERATION_FAILED);

    status = DdiMedia_UnmapBuffer(ctx, pVAImg->buf);
    if (status != VA_STATUS_SUCCESS)
    {
        DdiMediaUtil_UnlockSurface(pSurface);
        return VA_STATUS_ERROR_UNKNOWN;
    }

    DdiMediaUtil_UnlockSurface(pSurface);

    return VA_STATUS_SUCCESS;

}

VAStatus DdiMedia_QuerySubpictureFormats(
    VADriverContextP ctx,
    VAImageFormat   *format_list,
    uint32_t        *flags,
    uint32_t        *num_formats)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(format_list);
    DDI_UNUSED(flags);
    DDI_UNUSED(num_formats);

    DDI_FUNCTION_ENTER();

    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_CreateSubpicture(
    VADriverContextP ctx,
    VAImageID        image,
    VASubpictureID  *subpicture   /* out */
)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(image);
    DDI_UNUSED(subpicture);

    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMedia_DestroySubpicture(
    VADriverContextP ctx,
    VASubpictureID   subpicture
)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);

    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMedia_SetSubpictureImage(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    VAImageID        image
)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);
    DDI_UNUSED(image);

    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMedia_SetSubpictureChromakey(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    uint32_t         chromakey_min,
    uint32_t         chromakey_max,
    uint32_t         chromakey_mask
)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);
    DDI_UNUSED(chromakey_min);
    DDI_UNUSED(chromakey_max);
    DDI_UNUSED(chromakey_mask);

    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMedia_SetSubpictureGlobalAlpha(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    float            global_alpha
)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);
    DDI_UNUSED(global_alpha);

    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMedia_AssociateSubpicture(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    VASurfaceID     *target_surfaces,
    int32_t          num_surfaces,
    int16_t          src_x,  /* upper left offset in subpicture */
    int16_t          src_y,
    uint16_t         src_width,
    uint16_t         src_height,
    int16_t          dest_x, /* upper left offset in surface */
    int16_t          dest_y,
    uint16_t         dest_width,
    uint16_t         dest_height,
    /*
     * whether to enable chroma-keying or global-alpha
     * see VA_SUBPICTURE_XXX values
     */
    uint32_t     flags
)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);
    DDI_UNUSED(target_surfaces);
    DDI_UNUSED(num_surfaces);
    DDI_UNUSED(src_x);
    DDI_UNUSED(src_y);
    DDI_UNUSED(src_width);
    DDI_UNUSED(src_height);
    DDI_UNUSED(dest_x);
    DDI_UNUSED(dest_y);
    DDI_UNUSED(dest_width);
    DDI_UNUSED(dest_height);
    DDI_UNUSED(flags);

    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMedia_DeassociateSubpicture(
    VADriverContextP ctx,
    VASubpictureID   subpicture,
    VASurfaceID     *target_surfaces,
    int32_t          num_surfaces
)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(subpicture);
    DDI_UNUSED(target_surfaces);
    DDI_UNUSED(num_surfaces);

    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus DdiMedia_QueryDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute *attr_list,
    int32_t            *num_attributes)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(attr_list);

    DDI_FUNCTION_ENTER();

    if (num_attributes)
        *num_attributes = 0;

    return VA_STATUS_SUCCESS;
}

/*
 * Get display attributes
 * This function returns the current attribute values in "attr_list".
 * Only attributes returned with VA_DISPLAY_ATTRIB_GETTABLE set in the "flags" field
 * from vaQueryDisplayAttributes() can have their values retrieved.
 */

VAStatus DdiMedia_GetDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute *attr_list,
    int32_t             num_attributes)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(attr_list);
    DDI_UNUSED(num_attributes);

    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

/*
 * Set display attributes
 * Only attributes returned with VA_DISPLAY_ATTRIB_SETTABLE set in the "flags" field
 * from vaQueryDisplayAttributes() can be set.  If the attribute is not settable or
 * the value is out of range, the function returns VA_STATUS_ERROR_ATTR_NOT_SUPPORTED
 */
VAStatus DdiMedia_SetDisplayAttributes(
    VADriverContextP    ctx,
    VADisplayAttribute *attr_list,
    int32_t             num_attributes)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(attr_list);
    DDI_UNUSED(num_attributes);

    DDI_FUNCTION_ENTER();

    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

VAStatus
DdiMedia_QueryProcessingRate(
      VADriverContextP	        ctx,
      VAConfigID                config_id,
      VAProcessingRateParameter *proc_buf,
      uint32_t                  *processing_rate /* output parameter */)
{
    PDDI_MEDIA_CONTEXT             pMediaCtx;
    PLATFORM                       platform;
    MEDIA_FEATURE_TABLE            SkuTable;
    MEDIA_WA_TABLE                 WaTable;
    VAProcessingRateParameterEnc*  pProcessingRateBuffEnc = nullptr;
    VAProcessingRateParameterDec*  pProcessingRateBuffDec = nullptr;
    uint32_t                       quality_level;
    static const int32_t           TU_IDX_TABLE[] = {7, 6, 5, 4, 3, 2, 1, 0};
    uint32_t                       TuIdx = TU_IDX_TABLE[TARGETUSAGE_BEST_SPEED];
    bool                           bRes = false;
    uint32_t                       uiMbProcessingRatePerSec;
    VAEntrypoint                   entrypoint;
    MOS_STATUS                     eStatus;
    bool                           bVdencActive = false;
    CODECHAL_MODE                  eEncodeMode = CODECHAL_UNSUPPORTED_MODE;
    VAProfile                      profile;

    DDI_CHK_NULL(ctx,             "Null ctx",             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(proc_buf,        "Null proc_buf",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(processing_rate, "Null processing_rate", VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->m_caps, "Null m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);
 
    return pMediaCtx->m_caps->QueryProcessingRate(config_id,
            proc_buf, processing_rate);
}

VAStatus DdiMedia_BufferInfo (
    VADriverContextP ctx,
    VABufferID       buf_id,
    VABufferType    *type,
    uint32_t        *size,
    uint32_t        *num_elements)
{
    PDDI_MEDIA_CONTEXT     pMediaCtx;
    DDI_MEDIA_BUFFER       *pBuf;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,          "Null ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(type,         "Null type",         VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(size,         "Null size",         VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(num_elements, "Null num_elements", VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    if (nullptr == pMediaCtx)
        return VA_STATUS_ERROR_INVALID_CONTEXT;

    DDI_CHK_NULL(pMediaCtx->pBufferHeap, "Null pMediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)buf_id, pMediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid buf_id", VA_STATUS_ERROR_INVALID_BUFFER);

    pBuf  = DdiMedia_GetBufferFromVABufferID(pMediaCtx, buf_id);
    if (nullptr == pBuf)
    {
        return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    *type         = (VABufferType)pBuf->uiType;
    *size         = pBuf->iSize / pBuf->iNumElements;
    *num_elements = pBuf->iNumElements;

    return VA_STATUS_SUCCESS;
}


VAStatus DdiMedia_LockSurface (
    VADriverContextP ctx,
    VASurfaceID      surface,
    uint32_t        *fourcc,
    uint32_t        *luma_stride,
    uint32_t        *chroma_u_stride,
    uint32_t        *chroma_v_stride,
    uint32_t        *luma_offset,
    uint32_t        *chroma_u_offset,
    uint32_t        *chroma_v_offset,
    uint32_t        *buffer_name,
    void           **buffer )
{
    VAStatus               vaStatus;
    PDDI_MEDIA_CONTEXT     pMediaCtx;
    DDI_MEDIA_SURFACE     *pSurface;
    VAImage                tmpImage;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,             "Null context",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(fourcc,          "Null fourcc",          VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(luma_stride,     "Null luma_stride",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chroma_u_stride, "Null chroma_u_stride", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chroma_v_stride, "Null chroma_v_stride", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(luma_offset,     "Null luma_offset",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chroma_u_offset, "Null chroma_u_offset", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chroma_v_offset, "Null chroma_v_offset", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(buffer_name,     "Null buffer_name",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(buffer,          "Null buffer",          VA_STATUS_ERROR_INVALID_PARAMETER);


    vaStatus          = VA_STATUS_SUCCESS;
    pSurface          = nullptr;
    tmpImage.image_id = VA_INVALID_ID;

    pMediaCtx         = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,               "Null Media",                   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->pSurfaceHeap, "Null pMediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    pSurface          = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, surface);
    if (nullptr == pSurface)
    {
        // Surface is absent.
        vaStatus = VA_STATUS_ERROR_INVALID_SURFACE;
        goto error;
    }

    if (pSurface->uiLockedImageID != VA_INVALID_ID)
    {
        // Surface is locked already.
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
        goto error;
    }

    vaStatus = DdiMedia_DeriveImage(ctx,surface,&tmpImage);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto error;
    }

    pSurface->uiLockedImageID = tmpImage.image_id;

    vaStatus = DdiMedia_MapBuffer(ctx,tmpImage.buf,buffer);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto error;
    }

    pSurface->uiLockedBufID = tmpImage.buf;

    *fourcc                 = tmpImage.format.fourcc;
    *luma_offset            = tmpImage.offsets[0];
    *luma_stride            = tmpImage.pitches[0];
    *chroma_u_offset        = tmpImage.offsets[1];
    *chroma_u_stride        = tmpImage.pitches[1];
    *chroma_v_offset        = tmpImage.offsets[2];
    *chroma_v_stride        = tmpImage.pitches[2];
    *buffer_name            = tmpImage.buf;

error:
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        buffer = nullptr;
    }

    return vaStatus;
}

VAStatus DdiMedia_UnlockSurface (
    VADriverContextP   ctx,
    VASurfaceID        surface)
{
    VAStatus               vaStatus;
    VABufferID             BufID;
    VAImageID              ImageID;
    PDDI_MEDIA_CONTEXT     pMediaCtx;
    DDI_MEDIA_SURFACE      *pSurface;

    DDI_FUNCTION_ENTER();

    vaStatus  = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(ctx, "Null ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,               "Null pMediaCtx",                 VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->pSurfaceHeap, "Null pMediaCtx->pSurfaceHeap",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    pSurface  = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, surface);
    DDI_CHK_NULL(pSurface, "Null pSurface", VA_STATUS_ERROR_INVALID_SURFACE);

    if (pSurface->uiLockedImageID == VA_INVALID_ID)
    {
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
        goto error;
    }

    BufID    = (VABufferID)(pSurface->uiLockedBufID);
    vaStatus = DdiMedia_UnmapBuffer(ctx, BufID);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto error;
    }
    pSurface->uiLockedBufID = VA_INVALID_ID;

    ImageID  = (VAImageID)(pSurface->uiLockedImageID);
    vaStatus = DdiMedia_DestroyImage(ctx,ImageID);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto error;
    }
    pSurface->uiLockedImageID = VA_INVALID_ID;

error:
    return vaStatus;
}

VAStatus
DdiMedia_QueryVideoProcFilters(
    VADriverContextP    ctx,
    VAContextID         context,
    VAProcFilterType   *filters,
    uint32_t           *num_filters)
{
    uint32_t  i = 0;
    DDI_UNUSED(ctx);
    DDI_UNUSED(context);

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(filters,     "Null filters",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(num_filters, "Null num_filters", VA_STATUS_ERROR_INVALID_PARAMETER);

    // check if array size is less than VP_MAX_NUM_FILTERS
    if(*num_filters < DDI_VP_MAX_NUM_FILTERS)
    {
        // Tell the app, how many filter we can support
        *num_filters = DDI_VP_MAX_NUM_FILTERS;
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    // Set the filters
    while(i < *num_filters && i < DDI_VP_MAX_NUM_FILTERS)
    {
        filters[i] = vp_supported_filters[i];
        i++;
    }

    for (; i < DDI_VP_MAX_NUM_FILTERS ; i++)
    {
        filters[i] = VAProcFilterNone;
    }

    // Tell the app how many valid filters are filled in the array
    *num_filters = DDI_VP_MAX_NUM_FILTERS;

    return VA_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////
//! \purpose
//!  Query video processing filter capabilities.
//!  The real implementation is in media_libva_vp.c, since it needs to use some definitions in vphal.h.
//! \params
//! [in]     ctx :
//! [in]     context :
//! [in]     type :
//! [inout]  filter_caps :
//! [inout]  num_filter_caps :
//! [out]    None
//! \returns VA_STATUS_SUCCESS if call succeeds
//! THIS
////////////////////////////////////////////////////////////////////////////////
VAStatus
DdiMedia_QueryVideoProcFilterCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VAProcFilterType    type,
    void               *filter_caps,
    uint32_t           *num_filter_caps
)
{
    DDI_FUNCTION_ENTER();

    return DdiVp_QueryVideoProcFilterCaps(ctx, context, type, filter_caps, num_filter_caps);
}

VAStatus
DdiMedia_QueryVideoProcPipelineCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID         *filters,
    uint32_t            num_filters,
    VAProcPipelineCaps *pipeline_caps
)
{
    PDDI_MEDIA_CONTEXT                              pMediaCtx;
    uint32_t                                        dwDataSize;
    PDDI_MEDIA_BUFFER                               pBuf;
    int32_t                                         i;
    void                                           *pData;
    VAProcFilterParameterBuffer                    *filter_buffer;

    DDI_FUNCTION_ENTER();

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(ctx,           "Null ctx",           VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pipeline_caps, "Null pipeline_caps", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (num_filters > 0)
        DDI_CHK_NULL(filters,   "Null filters",       VA_STATUS_ERROR_INVALID_PARAMETER);

    pipeline_caps->pipeline_flags             = VA_PROC_PIPELINE_FAST;
    pipeline_caps->filter_flags               = 0;
    pipeline_caps->rotation_flags             = (1 << VA_ROTATION_NONE) | (1 << VA_ROTATION_90) | (1 << VA_ROTATION_180) | (1 << VA_ROTATION_270);
    pipeline_caps->blend_flags                = VA_BLEND_GLOBAL_ALPHA | VA_BLEND_PREMULTIPLIED_ALPHA | VA_BLEND_LUMA_KEY;
    pipeline_caps->num_forward_references     = DDI_CODEC_NUM_FWD_REF;
    pipeline_caps->num_backward_references    = DDI_CODEC_NUM_BK_REF;
    pipeline_caps->input_color_standards      = vp_input_color_std;
    pipeline_caps->num_input_color_standards  = DDI_VP_NUM_INPUT_COLOR_STD;
    pipeline_caps->output_color_standards     = vp_output_color_std;
    pipeline_caps->num_output_color_standards = DDI_VP_NUM_OUT_COLOR_STD;

    if ((context & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_DECODER)
    {
        pipeline_caps->num_input_pixel_formats    = 1;
        pipeline_caps->input_pixel_format[0]      = VA_FOURCC_NV12;
        pipeline_caps->num_output_pixel_formats   = 1;
        pipeline_caps->output_pixel_format[0]     = VA_FOURCC_NV12;
        pipeline_caps->max_input_width            = DDI_DECODE_SFC_MAX_WIDTH;
        pipeline_caps->max_input_height           = DDI_DECODE_SFC_MAX_HEIGHT;
        pipeline_caps->min_input_width            = DDI_DECODE_SFC_MIN_WIDTH;
        pipeline_caps->min_input_height           = DDI_DECODE_SFC_MIN_HEIGHT;
        pipeline_caps->max_output_width           = DDI_DECODE_SFC_MAX_WIDTH;
        pipeline_caps->max_output_height          = DDI_DECODE_SFC_MAX_HEIGHT;
        pipeline_caps->min_output_width           = DDI_DECODE_SFC_MIN_WIDTH;
        pipeline_caps->min_output_height          = DDI_DECODE_SFC_MIN_HEIGHT;
    }
    return VA_STATUS_SUCCESS;
}

/**
 * \brief Get surface attributes for the supplied config.
 *
 * This function retrieves the surface attributes matching the supplied
 * config. The caller shall provide an \c attrib_list with all attributes
 * to be retrieved. Upon successful return, the attributes in \c attrib_list
 * are updated with the requested value. Unknown attributes or attributes
 * that are not supported for the given config will have their \c flags
 * field set to \c VA_SURFACE_ATTRIB_NOT_SUPPORTED.
 *
 * @param[in] ctx               the VA display
 * @param[in] config            the config identifying a codec or a video
 *     processing pipeline
 * @param[out] attrib_list        the list of attributes on output, with at
 *     least \c type fields filled in, and possibly \c value fields whenever
 *     necessary.The updated list of attributes and flags on output
 * @param[in] num_attribs       the number of attributes supplied in the
 *     \c attrib_list array
 */
VAStatus DdiMedia_GetSurfaceAttributes(
    VADriverContextP    ctx,
    VAConfigID          config,
    VASurfaceAttrib    *attrib_list,
    uint32_t            num_attribs
)
{
    VAStatus vaStatus;
    DDI_UNUSED(ctx);
    DDI_UNUSED(config);
    DDI_UNUSED(attrib_list);
    DDI_UNUSED(num_attribs);

    DDI_FUNCTION_ENTER();

    vaStatus = VA_STATUS_ERROR_UNIMPLEMENTED;

    return vaStatus;
}

VAStatus DdiMedia_AcquireBufferHandle(
    VADriverContextP ctx,
    VABufferID buf_id,
    VABufferInfo *buf_info)
{
    PDDI_MEDIA_CONTEXT  pMediaCtx;
    DDI_MEDIA_BUFFER    *pBuf;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,          "Null ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf_info,     "Null buf_info",     VA_STATUS_ERROR_INVALID_PARAMETER);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,          "Invalid Media ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pBuf = DdiMedia_GetBufferFromVABufferID(pMediaCtx, buf_id);
    DDI_CHK_NULL(pBuf,          "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(pBuf->bo,      "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);

    // If user did not specify memtype he want's we use something we prefer, we prefer PRIME
    if (!buf_info->mem_type)
    {
       buf_info->mem_type = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
    }
    // now chekcing memtype whether we support it
    if ((buf_info->mem_type != VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME) &&
        (buf_info->mem_type != VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM))
    {
        return VA_STATUS_ERROR_UNSUPPORTED_MEMORY_TYPE;
    }

    DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
    // already acquired?
    if (pBuf->uiExportcount)
    {   // yes, already acquired
        // can't provide access thru another memtype
        if (pBuf->uiMemtype != buf_info->mem_type)
        {
            DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }
    else
    {   // no, not acquired - doing this now
        switch (buf_info->mem_type) {
        case VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM: {
            uint32_t flink;
            if (mos_bo_flink(pBuf->bo, &flink) != 0)
            {
                DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }
            pBuf->handle = (intptr_t)flink;
            break;
        }
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME: {
            int32_t prime_fd;
            if (mos_bo_gem_export_to_prime(pBuf->bo, &prime_fd) != 0)
            {
                DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            pBuf->handle = (intptr_t)prime_fd;
            break;
        }
        }
        // saving memtepy which was provided to the user
        pBuf->uiMemtype = buf_info->mem_type;
    }

    ++pBuf->uiExportcount;
    mos_bo_reference(pBuf->bo);

    buf_info->type = pBuf->uiType;
    buf_info->handle = pBuf->handle;
    buf_info->mem_size = pBuf->iNumElements * pBuf->iSize;

    DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_ReleaseBufferHandle(
    VADriverContextP ctx,
    VABufferID buf_id)
{
    PDDI_MEDIA_CONTEXT  pMediaCtx;
    DDI_MEDIA_BUFFER*   pBuf;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "Null ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Invalid Media ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pBuf = DdiMedia_GetBufferFromVABufferID(pMediaCtx, buf_id);
    DDI_CHK_NULL(pBuf,          "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(pBuf->bo,      "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);

    DdiMediaUtil_LockMutex(&pMediaCtx->BufferMutex);
    if (!pBuf->uiMemtype || !pBuf->uiExportcount)
    {
        DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
        return VA_STATUS_SUCCESS;
    }
    mos_bo_unreference(pBuf->bo);
    --pBuf->uiExportcount;

    if (!pBuf->uiExportcount) {
        switch (pBuf->uiMemtype) {
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME: {
            close((intptr_t)pBuf->handle);
            break;
        }
        }
        pBuf->uiMemtype = 0;
    }
    DdiMediaUtil_UnLockMutex(&pMediaCtx->BufferMutex);
    return VA_STATUS_SUCCESS;
}

VAStatus __vaDriverInit_0_31(VADriverContextP ctx )
{
    struct VADriverVTable    *pVTable;
    struct VADriverVTableVPP *pVTableVpp;
    struct VADriverVTableTPI *pVTableTpi;

    DDI_CHK_NULL(ctx,         "Null ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);

    pVTable     = DDI_CODEC_GET_VTABLE(ctx);
    DDI_CHK_NULL(pVTable,     "Null pVTable",      VA_STATUS_ERROR_INVALID_CONTEXT);

    pVTableVpp  = DDI_CODEC_GET_VTABLE_VPP(ctx);
    DDI_CHK_NULL(pVTableVpp,  "Null pVTableVpp",   VA_STATUS_ERROR_INVALID_CONTEXT);

    ctx->pDriverData                         = nullptr;
    ctx->version_major                       = VA_MAJOR_VERSION;
    ctx->version_minor                       = VA_MINOR_VERSION;
    ctx->max_profiles                        = DDI_CODEC_GEN_MAX_PROFILES;
    ctx->max_entrypoints                     = DDI_CODEC_GEN_MAX_ENTRYPOINTS;
    ctx->max_attributes                      = (int32_t)VAConfigAttribTypeMax;
    ctx->max_image_formats                   = MediaLibvaCaps::GetImageFormatsMaxNum();
    ctx->max_subpic_formats                  = DDI_CODEC_GEN_MAX_SUBPIC_FORMATS;
    ctx->max_display_attributes              = DDI_CODEC_GEN_MAX_DISPLAY_ATTRIBUTES ;
    ctx->str_vendor                          = DDI_CODEC_GEN_STR_VENDOR;

    pVTable->vaTerminate                     = DdiMedia_Terminate;
    pVTable->vaQueryConfigEntrypoints        = DdiMedia_QueryConfigEntrypoints;
    pVTable->vaQueryConfigProfiles           = DdiMedia_QueryConfigProfiles;
    pVTable->vaQueryConfigAttributes         = DdiMedia_QueryConfigAttributes;
    pVTable->vaCreateConfig                  = DdiMedia_CreateConfig;
    pVTable->vaDestroyConfig                 = DdiMedia_DestroyConfig;
    pVTable->vaGetConfigAttributes           = DdiMedia_GetConfigAttributes;

    pVTable->vaCreateSurfaces                = DdiMedia_CreateSurfaces;
    pVTable->vaDestroySurfaces               = DdiMedia_DestroySurfaces;
    pVTable->vaCreateSurfaces2               = DdiMedia_CreateSurfaces2;

    pVTable->vaCreateContext                 = DdiMedia_CreateContext;
    pVTable->vaDestroyContext                = DdiMedia_DestroyContext;
    pVTable->vaCreateBuffer                  = DdiMedia_CreateBuffer;
    pVTable->vaBufferSetNumElements          = DdiMedia_BufferSetNumElements;
    pVTable->vaMapBuffer                     = DdiMedia_MapBuffer;
    pVTable->vaUnmapBuffer                   = DdiMedia_UnmapBuffer;
    pVTable->vaDestroyBuffer                 = DdiMedia_DestroyBuffer;
    pVTable->vaBeginPicture                  = DdiMedia_BeginPicture;
    pVTable->vaRenderPicture                 = DdiMedia_RenderPicture;
    pVTable->vaEndPicture                    = DdiMedia_EndPicture;
    pVTable->vaSyncSurface                   = DdiMedia_SyncSurface;
    pVTable->vaQuerySurfaceStatus            = DdiMedia_QuerySurfaceStatus;
    pVTable->vaQuerySurfaceError             = DdiMedia_QuerySurfaceError;
    pVTable->vaQuerySurfaceAttributes        = DdiMedia_QuerySurfaceAttributes;
    pVTable->vaPutSurface                    = DdiMedia_PutSurface;
    pVTable->vaQueryImageFormats             = DdiMedia_QueryImageFormats;

    pVTable->vaCreateImage                   = DdiMedia_CreateImage;
    pVTable->vaDeriveImage                   = DdiMedia_DeriveImage;
    pVTable->vaDestroyImage                  = DdiMedia_DestroyImage;
    pVTable->vaSetImagePalette               = DdiMedia_SetImagePalette;
    pVTable->vaGetImage                      = DdiMedia_GetImage;
    pVTable->vaPutImage                      = DdiMedia_PutImage;
    pVTable->vaQuerySubpictureFormats        = DdiMedia_QuerySubpictureFormats;
    pVTable->vaCreateSubpicture              = DdiMedia_CreateSubpicture;
    pVTable->vaDestroySubpicture             = DdiMedia_DestroySubpicture;
    pVTable->vaSetSubpictureImage            = DdiMedia_SetSubpictureImage;
    pVTable->vaSetSubpictureChromakey        = DdiMedia_SetSubpictureChromakey;
    pVTable->vaSetSubpictureGlobalAlpha      = DdiMedia_SetSubpictureGlobalAlpha;
    pVTable->vaAssociateSubpicture           = DdiMedia_AssociateSubpicture;
    pVTable->vaDeassociateSubpicture         = DdiMedia_DeassociateSubpicture;
    pVTable->vaQueryDisplayAttributes        = DdiMedia_QueryDisplayAttributes;
    pVTable->vaGetDisplayAttributes          = DdiMedia_GetDisplayAttributes;
    pVTable->vaSetDisplayAttributes          = DdiMedia_SetDisplayAttributes;
    pVTable->vaQueryProcessingRate           = DdiMedia_QueryProcessingRate;

    // vaTrace
    pVTable->vaBufferInfo                    = DdiMedia_BufferInfo;
    pVTable->vaLockSurface                   = DdiMedia_LockSurface;
    pVTable->vaUnlockSurface                 = DdiMedia_UnlockSurface;

    pVTableVpp->vaQueryVideoProcFilters      = DdiMedia_QueryVideoProcFilters;
    pVTableVpp->vaQueryVideoProcFilterCaps   = DdiMedia_QueryVideoProcFilterCaps;
    pVTableVpp->vaQueryVideoProcPipelineCaps = DdiMedia_QueryVideoProcPipelineCaps;

    //pVTable->vaSetSurfaceAttributes          = DdiMedia_SetSurfaceAttributes;
    pVTable->vaGetSurfaceAttributes          = DdiMedia_GetSurfaceAttributes;
    //Export PRIMEFD/FLINK to application for buffer sharing with OpenCL/GL
    pVTable->vaAcquireBufferHandle           = DdiMedia_AcquireBufferHandle;
    pVTable->vaReleaseBufferHandle           = DdiMedia_ReleaseBufferHandle;
#ifndef ANDROID    
    pVTable->vaCreateMFContext               = DdiMedia_CreateMfeContextInternal;
    pVTable->vaMFAddContext                  = DdiMedia_AddContextInternal;
    pVTable->vaMFReleaseContext              = DdiMedia_ReleaseContextInternal;
    pVTable->vaMFSubmit                      = DdiEncode_MfeSubmit;
#endif    
    return DdiMedia__Initialize(ctx, nullptr, nullptr);
}

#ifdef __cplusplus
extern "C" {
#endif
#ifndef ANDROID
#if VA_MINOR_VERSION
#define VA_DRV_INIT_FUC_NAME __vaDriverInit_1_1
#else
#define VA_DRV_INIT_FUC_NAME __vaDriverInit_1_0
#endif
#else
#define VA_DRV_INIT_FUC_NAME __vaDriverInit_0_32
#endif

MEDIAAPI_EXPORT VAStatus DdiMedia_CreateMfeContext(
    VADisplay           dpy,
    VAMFContextID      *mfe_context
)
{
    VADriverContextP            ctx;

    DDI_CHK_NULL(dpy,                     "Null dpy",                     VA_STATUS_ERROR_INVALID_DISPLAY);
    ctx = ((VADisplayContextP)dpy)->pDriverContext;

    return DdiMedia_CreateMfeContextInternal(ctx, mfe_context);
}

MEDIAAPI_EXPORT VAStatus DdiMedia_AddContext(
    VADisplay           dpy,
    VAContextID         context,
    VAMFContextID      mfe_context
)
{
    VADriverContextP            ctx;

    DDI_CHK_NULL(dpy,                     "Null dpy",                     VA_STATUS_ERROR_INVALID_DISPLAY);
    ctx = ((VADisplayContextP)dpy)->pDriverContext;

    return DdiMedia_AddContextInternal(ctx, context, mfe_context);
}

MEDIAAPI_EXPORT VAStatus DdiMedia_ReleaseContext(
    VADisplay           dpy,
    VAContextID         context,
    VAMFContextID      mfe_context
)
{
    VADriverContextP            ctx;

    DDI_CHK_NULL(dpy,                     "Null dpy",                     VA_STATUS_ERROR_INVALID_DISPLAY);
    ctx = ((VADisplayContextP)dpy)->pDriverContext;

    return DdiMedia_ReleaseContextInternal(ctx, context, mfe_context);
}

MEDIAAPI_EXPORT VAStatus DdiMedia_MfeSubmit(
    VADisplay           dpy,
    VAMFContextID      mfe_context,
    VAContextID        *contexts,
    int32_t             num_contexts
)
{
    VADriverContextP            ctx;

    DDI_CHK_NULL(dpy,                     "Null dpy",                     VA_STATUS_ERROR_INVALID_DISPLAY);
    ctx = ((VADisplayContextP)dpy)->pDriverContext;

    return DdiEncode_MfeSubmit(ctx, mfe_context, contexts, num_contexts);
}

MEDIAAPI_EXPORT VAStatus VA_DRV_INIT_FUC_NAME(VADriverContextP ctx )
{
    return __vaDriverInit_0_31(ctx);
}

// private API for openCL
MEDIAAPI_EXPORT VAStatus DdiMedia_ExtGetSurfaceHandle(
    VADisplay      dpy,
    VASurfaceID   *surface,
    int32_t       *prime_fd)
{
    VADriverContextP            ctx;
    PDDI_MEDIA_CONTEXT          pMediaCtx;
    DDI_MEDIA_SURFACE          *pSurface;
    int32_t                     ret;

    DDI_CHK_NULL(dpy,                     "Null dpy",                     VA_STATUS_ERROR_INVALID_DISPLAY);
    DDI_CHK_NULL(surface,                 "Null surfaces",                VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(prime_fd,                "Null id",                      VA_STATUS_ERROR_INVALID_PARAMETER);

    ctx = ((VADisplayContextP)dpy)->pDriverContext;
    DDI_CHK_NULL(ctx,                     "Null ctx",                     VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,               "Null pMediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaCtx->pSurfaceHeap, "Null pMediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)(*surface), pMediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaces", VA_STATUS_ERROR_INVALID_SURFACE);

    pSurface = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, *surface);
    if (pSurface)
    {
        if (pSurface->bo)
        {
            ret = mos_bo_gem_export_to_prime(pSurface->bo, (int32_t*)&pSurface->name);
            if (ret)
            {
                //LOGE("Failed drm_intel_gem_export_to_prime operation!!!\n");
                return VA_STATUS_ERROR_OPERATION_FAILED;
            }
        }
    }
    else
    {
        return VA_STATUS_ERROR_INVALID_SURFACE;
    }

    *prime_fd = pSurface->name;

    return VA_STATUS_SUCCESS;
}

MEDIAAPI_EXPORT VAStatus DdiMedia_MapBuffer2(
    VADisplay           dpy,
    VABufferID          buf_id,
    void              **pbuf,
    int32_t             flag
)
{
    VADriverContextP            ctx;

    DDI_CHK_NULL(dpy,                     "Null dpy",                     VA_STATUS_ERROR_INVALID_DISPLAY);

    ctx = ((VADisplayContextP)dpy)->pDriverContext;

    if ((flag == 0) || (flag & ~(MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY)))
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    return DdiMedia_MapBufferInternal(ctx, buf_id, pbuf, flag);
}

#ifdef __cplusplus
}
#endif


