/*
* Copyright (c) 2009-2021, Intel Corporation
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

#if !defined(ANDROID) && defined(X11_FOUND)
#include <X11/Xutil.h>
#endif

#include "media_libva.h"

#include "media_libva_util.h"
#include "media_libva_decoder.h"
#include "media_libva_encoder.h"
#if !defined(ANDROID) && defined(X11_FOUND)
#include "media_libva_putsurface_linux.h"
#endif
#include "media_libva_vp.h"
#include "media_ddi_prot.h"
#include "mos_os.h"

#include "hwinfo_linux.h"
#include "mediamemdecomp.h"
#include "mos_solo_generic.h"
#include "media_libva_caps.h"
#include "media_interfaces_mmd.h"
#include "media_interfaces_mcpy.h"
#include "media_user_settings_mgr.h"
#include "media_interfaces.h"
#include "mos_interface.h"
#include "drm_fourcc.h"
#include "media_libva_interface.h"
#include "media_libva_interface_next.h"
#include "media_libva_apo_decision.h"

#ifdef __cplusplus
extern "C" {
#endif
extern VAStatus DdiDestroyContextCM (VADriverContextP   vaDrvCtx, VAContextID         vaCtxID);
#ifdef __cplusplus
}
#endif

extern template class MediaInterfacesFactory<MhwInterfaces>;

VAProcFilterType vp_supported_filters[DDI_VP_MAX_NUM_FILTERS] = {
    VAProcFilterNoiseReduction,
    VAProcFilterDeinterlacing,
    VAProcFilterSharpening,
    VAProcFilterColorBalance,
    VAProcFilterSkinToneEnhancement,
    VAProcFilterTotalColorCorrection,
    VAProcFilterHVSNoiseReduction,
    VAProcFilterHighDynamicRangeToneMapping
};

VAProcColorStandardType vp_input_color_std[DDI_VP_NUM_INPUT_COLOR_STD] = {
    VAProcColorStandardBT601,
    VAProcColorStandardBT709,
    VAProcColorStandardSRGB,
    VAProcColorStandardSTRGB,
    VAProcColorStandardBT2020,
    VAProcColorStandardExplicit
};

VAProcColorStandardType vp_output_color_std[DDI_VP_NUM_OUT_COLOR_STD] = {
    VAProcColorStandardBT601,
    VAProcColorStandardBT709,
    VAProcColorStandardSRGB,
    VAProcColorStandardSTRGB,
    VAProcColorStandardBT2020,
    VAProcColorStandardExplicit
};

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

VAStatus DdiMedia_DestroyImage (
    VADriverContextP ctx,
    VAImageID        image
);

static PDDI_MEDIA_CONTEXT DdiMedia_CreateMediaDriverContext()
{
    PDDI_MEDIA_CONTEXT   mediaCtx;

    mediaCtx = (PDDI_MEDIA_CONTEXT)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_CONTEXT));

    return mediaCtx;
}

// refine this for decoder later
static bool DdiMedia_ReleaseSliceControlBuffer(
    uint32_t          ctxType,
    void             *ctx,
    DDI_MEDIA_BUFFER *buf)
{
    DDI_UNUSED(buf);

    switch (ctxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        {
            PDDI_DECODE_CONTEXT  decCtx;

            decCtx = DdiDecode_GetDecContextFromPVOID(ctx);
            DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(decCtx->BufMgr);

            switch (decCtx->wMode)
            {
                case CODECHAL_DECODE_MODE_AVCVLD:
                    if(decCtx->bShortFormatInUse)
                    {
                        if(bufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264Base == nullptr)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if(bufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264 == nullptr)
                        {
                            return false;
                        }
                    }
                    break;
                case CODECHAL_DECODE_MODE_MPEG2VLD:
                    if(bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 == nullptr)
                    {
                        return false;
                    }
                    break;
                case CODECHAL_DECODE_MODE_VC1VLD:
                    if(bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 == nullptr)
                    {
                        return false;
                    }
                    break;
                case CODECHAL_DECODE_MODE_JPEG:
                    if(bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG == nullptr)
                    {
                        return false;
                    }
                    break;
                case CODECHAL_DECODE_MODE_VP8VLD:
                    if(bufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8 == nullptr)
                    {
                        return false;
                    }
                    break;
                case CODECHAL_DECODE_MODE_HEVCVLD:
                    if(decCtx->bShortFormatInUse)
                    {
                        if(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC == nullptr)
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC == nullptr &&
                            bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext == nullptr)
                        {
                            return false;
                        }
                    }
                    break;
                case CODECHAL_DECODE_MODE_VP9VLD:
                    if(bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9 == nullptr)
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
    DDI_CODEC_COM_BUFFER_MGR   *bufMgr,
    DDI_MEDIA_BUFFER           *buf)
{
    DDI_UNUSED(bufMgr);
    DDI_UNUSED(buf);
    return true;
}

static bool DdiMedia_ReleaseBsBuffer(
    DDI_CODEC_COM_BUFFER_MGR   *bufMgr,
    DDI_MEDIA_BUFFER           *buf)
{
    if ((bufMgr == nullptr) || (buf == nullptr))
    {
        return true;
    }

    if (buf->format == Media_Format_CPU)
    {
        for(uint32_t i = 0; i < bufMgr->dwNumSliceData; i++)
        {
            if(bufMgr->pSliceData[i].pBaseAddress == buf->pData)
            {
                DdiMediaUtil_FreeBuffer(buf);
                bufMgr->pSliceData[i].pBaseAddress = nullptr;
                if (bufMgr->pSliceData[i].pMappedGPUBuffer != nullptr)
                {
                    DdiMediaUtil_UnlockBuffer(bufMgr->pSliceData[i].pMappedGPUBuffer);
                    if (bufMgr->pSliceData[i].pMappedGPUBuffer->bMapped == false)
                    {
                        DdiMediaUtil_FreeBuffer(bufMgr->pSliceData[i].pMappedGPUBuffer);
                        MOS_FreeMemory(bufMgr->pSliceData[i].pMappedGPUBuffer);
                    }
                }
                MOS_ZeroMemory((void*)(&(bufMgr->pSliceData[i])), sizeof(bufMgr->pSliceData[0]));
                bufMgr->dwNumSliceData --;
                return true;
            }
        }
        return false;
    }
    else
    {
        if (bufMgr->dwNumSliceData)
            bufMgr->dwNumSliceData--;
    }
    return true;
}

static uint32_t DdiMedia_CreateRenderTarget(
    PDDI_MEDIA_CONTEXT            mediaDrvCtx,
    DDI_MEDIA_FORMAT              mediaFormat,
    uint32_t                      width,
    uint32_t                      height,
    DDI_MEDIA_SURFACE_DESCRIPTOR *surfDesc,
    uint32_t                      surfaceUsageHint,
    int                           memType
)
{
    DdiMediaUtil_LockMutex(&mediaDrvCtx->SurfaceMutex);

    PDDI_MEDIA_SURFACE_HEAP_ELEMENT surfaceElement = DdiMediaUtil_AllocPMediaSurfaceFromHeap(mediaDrvCtx->pSurfaceHeap);
    if (nullptr == surfaceElement)
    {
        DdiMediaUtil_UnLockMutex(&mediaDrvCtx->SurfaceMutex);
        return VA_INVALID_ID;
    }

    surfaceElement->pSurface = (DDI_MEDIA_SURFACE *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE));
    if (nullptr == surfaceElement->pSurface)
    {
        DdiMediaUtil_ReleasePMediaSurfaceFromHeap(mediaDrvCtx->pSurfaceHeap, surfaceElement->uiVaSurfaceID);
        DdiMediaUtil_UnLockMutex(&mediaDrvCtx->SurfaceMutex);
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

    if(DdiMediaUtil_CreateSurface(surfaceElement->pSurface, mediaDrvCtx)!= VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(surfaceElement->pSurface);
        DdiMediaUtil_ReleasePMediaSurfaceFromHeap(mediaDrvCtx->pSurfaceHeap, surfaceElement->uiVaSurfaceID);
        DdiMediaUtil_UnLockMutex(&mediaDrvCtx->SurfaceMutex);
        return VA_INVALID_ID;
    }

    mediaDrvCtx->uiNumSurfaces++;
    uint32_t surfaceID = surfaceElement->uiVaSurfaceID;
    DdiMediaUtil_UnLockMutex(&mediaDrvCtx->SurfaceMutex);
    return surfaceID;
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

//!
//! \brief  Set Frame ID 
//! 
//! \param  [in] dpy
//!         VA display
//! \param  [in] surface
//!         VA surface ID
//! \param  [in] frame_id
//!         Frame ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_SetFrameID(
    VADisplay    dpy,
    VASurfaceID  surface,
    uint32_t     frame_id
)
{
    VADriverContextP ctx            = (((VADisplayContextP)dpy)->pDriverContext);
    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx     = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_SURFACE *mediaSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface.", VA_STATUS_ERROR_INVALID_PARAMETER);

    mediaSurface->frame_idx = frame_id;

    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Convert media format to OS format 
//! 
//! \param  [in] format
//!         Ddi media format
//!
//! \return Os format if call sucesss,else
//!     VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT if fail
//!
int32_t DdiMedia_MediaFormatToOsFormat(DDI_MEDIA_FORMAT format)
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

//!
//! \brief  Convert Os format to media format 
//! 
//! \param  [in] fourcc
//!         FourCC
//! \param  [in] rtformatType
//!         Rt format type
//!
//! \return DDI_MEDIA_FORMAT
//!     Ddi media format
//!
DDI_MEDIA_FORMAT DdiMedia_OsFormatToMediaFormat(int32_t fourcc, int32_t rtformatType)
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

static VAStatus DdiMedia_GetChromaPitchHeight(
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


#if !defined(ANDROID) && defined(X11_FOUND)

#define X11_LIB_NAME "libX11.so.6"

/*
 * Close opened libX11.so lib, free related function table.
 */
static void DdiMedia_DestroyX11Connection(
    PDDI_MEDIA_CONTEXT mediaCtx
)
{
    if (nullptr == mediaCtx || nullptr == mediaCtx->X11FuncTable)
    {
        return;
    }

    MOS_FreeLibrary(mediaCtx->X11FuncTable->pX11LibHandle);
    MOS_FreeMemory(mediaCtx->X11FuncTable);
    mediaCtx->X11FuncTable = nullptr;

    return;
}

/*
 * dlopen libX11.so, setup the function table, which is used by
 * DdiCodec_PutSurface (Linux) so far.
 */
static VAStatus DdiMedia_ConnectX11(
    PDDI_MEDIA_CONTEXT mediaCtx
)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaCtx->X11FuncTable = (PDDI_X11_FUNC_TABLE)MOS_AllocAndZeroMemory(sizeof(DDI_X11_FUNC_TABLE));
    DDI_CHK_NULL(mediaCtx->X11FuncTable, "Allocation Failed for X11FuncTable", VA_STATUS_ERROR_ALLOCATION_FAILED);

    HMODULE    h_module   = nullptr;
    MOS_STATUS mos_status = MOS_LoadLibrary(X11_LIB_NAME, &h_module);
    if (MOS_STATUS_SUCCESS != mos_status || nullptr == h_module)
    {
        DdiMedia_DestroyX11Connection(mediaCtx);
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
        DdiMedia_DestroyX11Connection(mediaCtx);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    return VA_STATUS_SUCCESS;
}
#endif

/////////////////////////////////////////////////////////////////////////////
//! \Free allocated surfaceheap elements
//! \params
//! [in] PDDI_MEDIA_CONTEXT
//! [out] none
//! \returns
/////////////////////////////////////////////////////////////////////////////
static void DdiMedia_FreeSurfaceHeapElements(PDDI_MEDIA_CONTEXT mediaCtx)
{
    if (nullptr == mediaCtx)
        return;
    PDDI_MEDIA_HEAP surfaceHeap = mediaCtx->pSurfaceHeap;

    if (nullptr == surfaceHeap)
        return;

    PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapBase = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)surfaceHeap->pHeapBase;
    if (nullptr == mediaSurfaceHeapBase)
        return;

    int32_t surfaceNums = mediaCtx->uiNumSurfaces;
    for (int32_t elementId = 0; elementId < surfaceNums; elementId++)
    {
        PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapElmt = &mediaSurfaceHeapBase[elementId];
        if (nullptr == mediaSurfaceHeapElmt->pSurface)
            continue;

        DdiMediaUtil_FreeSurface(mediaSurfaceHeapElmt->pSurface);
        MOS_FreeMemory(mediaSurfaceHeapElmt->pSurface);
        DdiMediaUtil_ReleasePMediaSurfaceFromHeap(surfaceHeap,mediaSurfaceHeapElmt->uiVaSurfaceID);
        mediaCtx->uiNumSurfaces--;
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
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    if (nullptr == mediaCtx)
        return;

    PDDI_MEDIA_HEAP  bufferHeap = mediaCtx->pBufferHeap;
    if (nullptr == bufferHeap)
        return;

    PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapBase = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)bufferHeap->pHeapBase;
    if (nullptr == mediaBufferHeapBase)
        return;

    int32_t bufNums = mediaCtx->uiNumBufs;
    for (int32_t elementId = 0; bufNums > 0; ++elementId)
    {
        PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapElmt = &mediaBufferHeapBase[elementId];
        if (nullptr == mediaBufferHeapElmt->pBuffer)
            continue;
        DdiMedia_DestroyBuffer(ctx,mediaBufferHeapElmt->uiVaBufferID);
        //Ensure the non-empty buffer to be destroyed.
        --bufNums;
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
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    if (nullptr == mediaCtx)
        return;

    PDDI_MEDIA_HEAP imageHeap = mediaCtx->pImageHeap;
    if (nullptr == imageHeap)
        return;

    PDDI_MEDIA_IMAGE_HEAP_ELEMENT mediaImageHeapBase = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)imageHeap->pHeapBase;
    if (nullptr == mediaImageHeapBase)
        return;

    int32_t imageNums = mediaCtx->uiNumImages;
    for (int32_t elementId = 0; elementId < imageNums; ++elementId)
    {
        PDDI_MEDIA_IMAGE_HEAP_ELEMENT mediaImageHeapElmt = &mediaImageHeapBase[elementId];
        if (nullptr == mediaImageHeapElmt->pImage)
            continue;
        DdiMedia_DestroyImage(ctx,mediaImageHeapElmt->uiVaImageID);
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
static void DdiMedia_FreeContextHeap(VADriverContextP ctx, PDDI_MEDIA_HEAP contextHeap,int32_t vaContextOffset, int32_t ctxNums)
{
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT mediaContextHeapBase = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)contextHeap->pHeapBase;
    if (nullptr == mediaContextHeapBase)
        return;

    for (int32_t elementId = 0; elementId < ctxNums; ++elementId)
    {
        PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT mediaContextHeapElmt = &mediaContextHeapBase[elementId];
        if (nullptr == mediaContextHeapElmt->pVaContext)
            continue;
        VAContextID vaCtxID = (VAContextID)(mediaContextHeapElmt->uiVaContextID + vaContextOffset);
        DdiMedia_DestroyContext(ctx,vaCtxID);
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
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    if (nullptr == mediaCtx)
        return;

    //Free EncoderContext
    PDDI_MEDIA_HEAP encoderContextHeap = mediaCtx->pEncoderCtxHeap;
    int32_t encCtxNums        = mediaCtx->uiNumEncoders;
    if (nullptr != encoderContextHeap)
        DdiMedia_FreeContextHeap(ctx,encoderContextHeap,DDI_MEDIA_VACONTEXTID_OFFSET_ENCODER,encCtxNums);

    //Free DecoderContext
    PDDI_MEDIA_HEAP decoderContextHeap = mediaCtx->pDecoderCtxHeap;
    int32_t decCtxNums        = mediaCtx->uiNumDecoders;
    if (nullptr != decoderContextHeap)
        DdiMedia_FreeContextHeap(ctx,decoderContextHeap,DDI_MEDIA_VACONTEXTID_OFFSET_DECODER,decCtxNums);

    //Free VpContext
    PDDI_MEDIA_HEAP vpContextHeap      = mediaCtx->pVpCtxHeap;
    int32_t vpctxNums         = mediaCtx->uiNumVPs;
    if (nullptr != vpContextHeap)
        DdiMedia_FreeContextHeap(ctx,vpContextHeap,DDI_MEDIA_VACONTEXTID_OFFSET_VP,vpctxNums);

    //Free ProtContext
    PDDI_MEDIA_HEAP protContextHeap      = mediaCtx->pProtCtxHeap;
    int32_t protCtxNums         = mediaCtx->uiNumProts;
    if (nullptr != protContextHeap)
        DdiMedia_FreeProtectedSessionHeap(ctx,protContextHeap,DDI_MEDIA_VACONTEXTID_OFFSET_PROT,protCtxNums);

    //Free MfeContext
    PDDI_MEDIA_HEAP mfeContextHeap     = mediaCtx->pMfeCtxHeap;
    int32_t mfeCtxNums        = mediaCtx->uiNumMfes;
    if (nullptr != mfeContextHeap)
        DdiMedia_FreeContextHeap(ctx, mfeContextHeap, DDI_MEDIA_VACONTEXTID_OFFSET_MFE, mfeCtxNums);

    // Free media memory decompression data structure
    if (!mediaCtx->m_apoMosEnabled && mediaCtx->pMediaMemDecompState)
    {
        MediaMemDecompBaseState *mediaMemCompState =
            static_cast<MediaMemDecompBaseState*>(mediaCtx->pMediaMemDecompState);
        MOS_Delete(mediaMemCompState);
    }
    mediaCtx->pMediaMemDecompState = nullptr;
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
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    if (nullptr == mediaCtx)
        return;

    int32_t cmnums = mediaCtx->uiNumCMs;
    for (int32_t elementId = 0; elementId < cmnums; elementId++)
    {
        VAContextID vaCtxID = elementId + DDI_MEDIA_VACONTEXTID_OFFSET_CM;
        DdiDestroyContextCM(ctx,vaCtxID);
    }
}

//!
//! \brief  Get VA image from VA image ID
//! 
//! \param  [in] mediaCtx
//!         Pointer to ddi media context
//! \param  [in] imageID
//!         VA image ID
//!
//! \return VAImage*
//!     Pointer to VAImage
//!
VAImage* DdiMedia_GetVAImageFromVAImageID (PDDI_MEDIA_CONTEXT mediaCtx, VAImageID imageID)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", nullptr);

    uint32_t i       = (uint32_t)imageID;
    DDI_CHK_LESS(i, mediaCtx->pImageHeap->uiAllocatedHeapElements, "invalid image id", nullptr);
    DdiMediaUtil_LockMutex(&mediaCtx->ImageMutex);
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT imageElement = (PDDI_MEDIA_IMAGE_HEAP_ELEMENT)mediaCtx->pImageHeap->pHeapBase;
    imageElement    += i;
    VAImage *vaImage = imageElement->pImage;
    DdiMediaUtil_UnLockMutex(&mediaCtx->ImageMutex);

    return vaImage;
}

//!
//! \brief  Get ctx from VA buffer ID
//! 
//! \param  [in] mediaCtx
//!         pddi media context
//! \param  [in] bufferID
//!         VA Buffer ID
//!
//! \return void*
//!     Pointer to buffer heap element context
//!
void* DdiMedia_GetCtxFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", nullptr);

    uint32_t i      = (uint32_t)bufferID;
    DDI_CHK_LESS(i, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", nullptr);
    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufHeapElement  = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)mediaCtx->pBufferHeap->pHeapBase;
    bufHeapElement += i;
    void *temp      = bufHeapElement->pCtx;
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);

    return temp;
}

//!
//! \brief  Get ctx type from VA buffer ID
//! 
//! \param  [in] mediaCtx
//!         Pointer to ddi media context
//! \param  [in] bufferID
//!         VA buffer ID
//!
//! \return uint32_t
//1     Context type
//!
uint32_t DdiMedia_GetCtxTypeFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", DDI_MEDIA_CONTEXT_TYPE_NONE);

    uint32_t i       = (uint32_t)bufferID;
    DDI_CHK_LESS(i, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", DDI_MEDIA_CONTEXT_TYPE_NONE);
    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufHeapElement  = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)mediaCtx->pBufferHeap->pHeapBase;
    bufHeapElement  += i;
    uint32_t ctxType = bufHeapElement->uiCtxType;
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);

    return ctxType;

}

//!
//! \brief  Destroy image from VA image ID 
//! 
//! \param  [in] mediaCtx
//!         Pointer to ddi media context
//! \param  [in] imageID
//!     VA image ID
//!
//! \return bool
//!     True if destroy image from VA image ID, else fail
//!
bool DdiMedia_DestroyImageFromVAImageID (PDDI_MEDIA_CONTEXT mediaCtx, VAImageID imageID)
{
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", false);

    DdiMediaUtil_LockMutex(&mediaCtx->ImageMutex);
    DdiMediaUtil_ReleasePVAImageFromHeap(mediaCtx->pImageHeap, (uint32_t)imageID);
    mediaCtx->uiNumImages--;
    DdiMediaUtil_UnLockMutex(&mediaCtx->ImageMutex);
    return true;
}
#ifdef _MMC_SUPPORTED
//!
//! \brief  Decompress internal media memory 
//! 
//! \param  [in] mosCtx
//!         Pointer to mos context
//! \param  [in] osResource
//!         Pointer mos resource
//!
void DdiMedia_MediaMemoryDecompressInternal(PMOS_CONTEXT mosCtx, PMOS_RESOURCE osResource)
{
    DDI_CHK_NULL(mosCtx, "nullptr mosCtx",);
    DDI_CHK_NULL(osResource, "nullptr osResource",);
    DDI_ASSERT(osResource);

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    if (mosCtx->m_apoMosEnabled && !mediaMemDecompState)
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

//!
//! \brief  copy internal media surface to another surface 
//! 
//! \param  [in] mosCtx
//!         Pointer to mos context
//! \param  [in] inputOsResource
//!         Pointer input mos resource
//! \param  [in] outputOsResource
//!         Pointer output mos resource
//! \param  [in] boutputcompressed
//!         output can be compressed or not
//!
void DdiMedia_MediaMemoryCopyInternal(PMOS_CONTEXT mosCtx, PMOS_RESOURCE inputOsResource, PMOS_RESOURCE outputOsResource, bool boutputcompressed)
{
    DDI_CHK_NULL(mosCtx, "nullptr mosCtx",);
    DDI_CHK_NULL(inputOsResource, "nullptr input osResource",);
    DDI_CHK_NULL(outputOsResource, "nullptr output osResource",);
    DDI_ASSERT(inputOsResource);
    DDI_ASSERT(outputOsResource);

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    if (mosCtx->m_apoMosEnabled && !mediaMemDecompState)
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

//!
//! \brief  copy internal media surface/buffer to another surface/buffer 
//! 
//! \param  [in] mosCtx
//!         Pointer to mos context
//! \param  [in] inputOsResource
//!         Pointer input mos resource
//! \param  [in] outputOsResource
//!         Pointer output mos resource
//! \param  [in] boutputcompressed
//!         output can be compressed or not
//! \param  [in] copyWidth
//!         The 2D surface Width
//! \param  [in] copyHeight
//!         The 2D surface height
//! \param  [in] copyInputOffset
//!         The offset of copied surface from
//! \param  [in] copyOutputOffset
//!         The offset of copied to
//!
void DdiMedia_MediaMemoryCopy2DInternal(PMOS_CONTEXT mosCtx, PMOS_RESOURCE inputOsResource, PMOS_RESOURCE outputOsResource, uint32_t copyWidth, uint32_t copyHeight, uint32_t copyInputOffset, uint32_t copyOutputOffset, uint32_t bpp, bool boutputcompressed)
{
    DDI_CHK_NULL(mosCtx, "nullptr mosCtx",);
    DDI_CHK_NULL(inputOsResource, "nullptr input osResource",);
    DDI_CHK_NULL(outputOsResource, "nullptr output osResource",);
    DDI_ASSERT(inputOsResource);
    DDI_ASSERT(outputOsResource);

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    if (mosCtx->m_apoMosEnabled && !mediaMemDecompState)
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

//!
//! \brief  Tile/Linear format conversion for media surface/buffer
//!
//! \param  [in] mosCtx
//!         Pointer to mos context
//! \param  [in] inputOsResource
//!         Pointer input mos resource
//! \param  [in] outputOsResource
//!         Pointer output mos resource
//! \param  [in] copyWidth
//!         The 2D surface Width
//! \param  [in] copyHeight
//!         The 2D surface height
//! \param  [in] copyInputOffset
//!         The offset of copied surface from
//! \param  [in] copyOutputOffset
//!         The offset of copied to
//! \param  [in] isTileToLinear
//!         Convertion direction, true: tile->linear, false: linear->tile
//! \param  [in] outputCompressed
//!         output can be compressed or not
//!
VAStatus DdiMedia_MediaMemoryTileConvertInternal(
    PMOS_CONTEXT mosCtx,
    PMOS_RESOURCE inputOsResource,
    PMOS_RESOURCE outputOsResource,
    uint32_t copyWidth,
    uint32_t copyHeight,
    uint32_t copyInputOffset,
    uint32_t copyOutputOffset,
    bool isTileToLinear,
    bool outputCompressed)
{
    DDI_CHK_NULL(mosCtx, "nullptr mosCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(inputOsResource, "nullptr input osResource", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(outputOsResource, "nullptr output osResource", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_ASSERT(inputOsResource);
    DDI_ASSERT(outputOsResource);

    VAStatus vaStatus = VA_STATUS_SUCCESS;

    MediaMemDecompBaseState *mediaMemDecompState = static_cast<MediaMemDecompBaseState*>(*mosCtx->ppMediaMemDecompState);

    if (mosCtx->m_apoMosEnabled && !mediaMemDecompState)
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

//!
//! \brief  Decompress a compressed surface.
//! 
//! \param  [in]     mediaCtx
//!     Pointer to ddi media context
//! \param  [in]     mediaSurface
//!     Ddi media surface 
//!
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_MediaMemoryDecompress(PDDI_MEDIA_CONTEXT mediaCtx, DDI_MEDIA_SURFACE *mediaSurface)
{
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
        MOS_CONTEXT  mosCtx;
        MOS_RESOURCE surface;
        DdiCpInterface *pCpDdiInterface;
        
        MOS_ZeroMemory(&mosCtx, sizeof(mosCtx));
        MOS_ZeroMemory(&surface, sizeof(surface));

        mosCtx.bufmgr          = mediaCtx->pDrmBufMgr;
        mosCtx.m_gpuContextMgr = mediaCtx->m_gpuContextMgr;
        mosCtx.m_cmdBufMgr     = mediaCtx->m_cmdBufMgr;
        mosCtx.fd              = mediaCtx->fd;
        mosCtx.iDeviceId       = mediaCtx->iDeviceId;
        mosCtx.SkuTable        = mediaCtx->SkuTable;
        mosCtx.WaTable         = mediaCtx->WaTable;
        mosCtx.gtSystemInfo    = *mediaCtx->pGtSystemInfo;
        mosCtx.platform        = mediaCtx->platform;

        mosCtx.ppMediaMemDecompState = &mediaCtx->pMediaMemDecompState;
        mosCtx.pfnMemoryDecompress   = mediaCtx->pfnMemoryDecompress;
        mosCtx.pfnMediaMemoryCopy    = mediaCtx->pfnMediaMemoryCopy;
        mosCtx.pfnMediaMemoryCopy2D  = mediaCtx->pfnMediaMemoryCopy2D;
        mosCtx.gtSystemInfo          = *mediaCtx->pGtSystemInfo;
        mosCtx.m_auxTableMgr         = mediaCtx->m_auxTableMgr;
        mosCtx.pGmmClientContext     = mediaCtx->pGmmClientContext;

        mosCtx.m_osDeviceContext     = mediaCtx->m_osDeviceContext;
        mosCtx.m_apoMosEnabled       = mediaCtx->m_apoMosEnabled;

        pCpDdiInterface = Create_DdiCpInterface(mosCtx);

        if (nullptr == pCpDdiInterface)
        {
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        else
        {
            DdiMediaUtil_LockMutex(&mediaCtx->SurfaceMutex);

            DdiMedia_MediaSurfaceToMosResource(mediaSurface, &surface);
            DdiMedia_MediaMemoryDecompressInternal(&mosCtx, &surface);

            DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);

            if (pCpDdiInterface)
            {
                Delete_DdiCpInterface(pCpDdiInterface);
                pCpDdiInterface = NULL;
            }
        }
#else
        vaStatus = VA_STATUS_ERROR_INVALID_SURFACE;
        DDI_ASSERTMESSAGE("MMC unsupported! [%d].", vaStatus);
#endif
    }

    return vaStatus;
}

/*
 * Initialize the library
 */

// Global mutex
MEDIA_MUTEX_T  GlobalMutex = MEDIA_MUTEX_INITIALIZER;

//!
//! \brief  Initialize
//!
//! \param  [in] mediaCtx
//!         Pointer to DDI media driver context
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
static VAStatus DdiMedia_HeapInitialize(
    PDDI_MEDIA_CONTEXT mediaCtx)
{
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

    mediaCtx->pCmCtxHeap = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    DDI_CHK_NULL(mediaCtx->pCmCtxHeap, "nullptr CmCtxHeap", VA_STATUS_ERROR_ALLOCATION_FAILED);
    mediaCtx->pCmCtxHeap->uiHeapElementSize = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    mediaCtx->pMfeCtxHeap = (DDI_MEDIA_HEAP *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_HEAP));
    DDI_CHK_NULL(mediaCtx->pMfeCtxHeap, "nullptr MfeCtxHeap", VA_STATUS_ERROR_ALLOCATION_FAILED);
    mediaCtx->pMfeCtxHeap->uiHeapElementSize = sizeof(DDI_MEDIA_VACONTEXT_HEAP_ELEMENT);

    // init the mutexs
    DdiMediaUtil_InitMutex(&mediaCtx->SurfaceMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->BufferMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->ImageMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->DecoderMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->EncoderMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->VpMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->ProtMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->CmMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->MfeMutex);

    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Initialize
//!
//! \param  [in] mediaCtx
//!         Pointer to DDI media driver context
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
static VAStatus DdiMedia_HeapDestroy(
    PDDI_MEDIA_CONTEXT mediaCtx)
{
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

    MOS_FreeMemory(mediaCtx->pCmCtxHeap->pHeapBase);
    MOS_FreeMemory(mediaCtx->pCmCtxHeap);

    MOS_FreeMemory(mediaCtx->pMfeCtxHeap->pHeapBase);
    MOS_FreeMemory(mediaCtx->pMfeCtxHeap);
    // destroy the mutexs
    DdiMediaUtil_DestroyMutex(&mediaCtx->SurfaceMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->BufferMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->ImageMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->DecoderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->EncoderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->VpMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->ProtMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->CmMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->MfeMutex);

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
    if (mediaCtx->uiNumCMs != 0)
    {
        DDI_ASSERTMESSAGE("APP does not destroy all the CMs.");
    }
    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Free for media context
//!
//! \param  [in] mediaCtx
//!         Pointer to ddi media context
//!
void FreeForMediaContext(PDDI_MEDIA_CONTEXT mediaCtx)
{
    DdiMediaUtil_UnLockMutex(&GlobalMutex);

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
        MOS_FreeMemory(mediaCtx->pMfeCtxHeap);
        MOS_FreeMemory(mediaCtx);
    }

    return;
}

void DestroyMediaContextMutex(PDDI_MEDIA_CONTEXT mediaCtx)
{
    // destroy the mutexs
    DdiMediaUtil_DestroyMutex(&mediaCtx->SurfaceMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->BufferMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->ImageMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->DecoderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->EncoderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->VpMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->CmMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->MfeMutex);
#if !defined(ANDROID) && defined(X11_FOUND)
    DdiMediaUtil_DestroyMutex(&mediaCtx->PutSurfaceRenderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->PutSurfaceSwapBufferMutex);
#endif
    return;
}

VAStatus DdiMedia_GetDeviceFD (
    VADriverContextP ctx,
    int32_t         *pDevicefd
)
{
    struct drm_state *pDRMState = (struct drm_state *)ctx->drm_state;
    DDI_CHK_NULL(pDRMState,    "nullptr pDRMState", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pDevicefd,    "nullptr pDevicefd", VA_STATUS_ERROR_INVALID_CONTEXT);

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
    *pDevicefd = pDRMState->fd;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia__Initialize (
    VADriverContextP ctx,
    int32_t         *major_version,
    int32_t         *minor_version
)
{
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

    DDI_CHK_NULL(ctx,          "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    bool    apoDdiEnabled = false;
    int32_t devicefd = 0;
    if(DdiMedia_GetDeviceFD(ctx, &devicefd) != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Unable to get device FD");
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    apoDdiEnabled = MediaLibvaApoDecision::InitDdiApoState(devicefd);

    if(MediaLibvaInterface::LoadFunction(ctx) != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Failed to load function pointer");
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    if(apoDdiEnabled)
    {
        return MediaLibvaInterfaceNext::Initialize(ctx, devicefd, major_version, minor_version);
    }
    else
    {
        return DdiMedia_InitMediaContext(ctx, devicefd, major_version, minor_version);
    }
}

VAStatus DdiMedia_InitMediaContext (
    VADriverContextP ctx,
    int32_t          devicefd,
    int32_t         *major_version,
    int32_t         *minor_version
)
{
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
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    if(mediaCtx)
    {
        mediaCtx->uiRef++;
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_SUCCESS;
    }

    mediaCtx = DdiMedia_CreateMediaDriverContext();
    if (nullptr == mediaCtx)
    {
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    mediaCtx->uiRef++;
    ctx->pDriverData = (void *)mediaCtx;
    mediaCtx->fd     = devicefd;

    mediaCtx->m_apoMosEnabled = SetupApoMosSwitch(devicefd);

#ifdef _MMC_SUPPORTED
    mediaCtx->pfnMemoryDecompress  = DdiMedia_MediaMemoryDecompressInternal;
    mediaCtx->pfnMediaMemoryCopy   = DdiMedia_MediaMemoryCopyInternal;
    mediaCtx->pfnMediaMemoryCopy2D = DdiMedia_MediaMemoryCopy2DInternal;
    mediaCtx->pfnMediaMemoryTileConvert = DdiMedia_MediaMemoryTileConvertInternal;
#endif
    mediaCtx->modularizedGpuCtxEnabled = true;

    if (mediaCtx->m_apoMosEnabled)
    {
        MOS_CONTEXT mosCtx     = {};
        mosCtx.fd              = mediaCtx->fd;
        mosCtx.m_apoMosEnabled = mediaCtx->m_apoMosEnabled;

        MosInterface::InitOsUtilities(&mosCtx);

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
    }
    else if (mediaCtx->modularizedGpuCtxEnabled)
    {
        // prepare m_osContext
        MosUtilities::MosUtilitiesInit(nullptr);
        //Read user feature key here for Per Utility Tool Enabling

        if (!g_perfutility->bPerfUtilityKey)
        {
            MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
            MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
            MOS_UserFeature_ReadValue_ID(
                NULL,
                __MEDIA_USER_FEATURE_VALUE_PERF_UTILITY_TOOL_ENABLE_ID,
                &UserFeatureData,
                nullptr);
            g_perfutility->dwPerfUtilityIsEnabled = UserFeatureData.i32Data;

            char                        sFilePath[MOS_MAX_PERF_FILENAME_LEN + 1] = "";
            MOS_USER_FEATURE_VALUE_DATA perfFilePath;
            MOS_STATUS                  eStatus_Perf = MOS_STATUS_SUCCESS;

            MOS_ZeroMemory(&perfFilePath, sizeof(perfFilePath));
            perfFilePath.StringData.pStringData = sFilePath;
            eStatus_Perf                        = MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_PERF_OUTPUT_DIRECTORY_ID,
                &perfFilePath,
                nullptr);
            if (eStatus_Perf == MOS_STATUS_SUCCESS)
            {
                g_perfutility->setupFilePath(sFilePath);
            }
            else
            {
                g_perfutility->setupFilePath();
            }

            g_perfutility->bPerfUtilityKey = true;
        }

        mediaCtx->pDrmBufMgr = mos_bufmgr_gem_init(mediaCtx->fd, DDI_CODEC_BATCH_BUFFER_SIZE);
        if (nullptr == mediaCtx->pDrmBufMgr)
        {
            DDI_ASSERTMESSAGE("DDI:No able to allocate buffer manager, fd=0x%d", mediaCtx->fd);
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        mos_bufmgr_gem_enable_reuse(mediaCtx->pDrmBufMgr);

        //Latency reducation:replace HWGetDeviceID to get device using ioctl from drm.
        mediaCtx->iDeviceId = mos_bufmgr_gem_get_devid(mediaCtx->pDrmBufMgr);

        //TO--DO, apo set it to FALSE by default, to remove the logic in apo mos controlled by it????
        mediaCtx->bIsAtomSOC = IS_ATOMSOC(mediaCtx->iDeviceId);

        MEDIA_FEATURE_TABLE *skuTable = &mediaCtx->SkuTable;
        MEDIA_WA_TABLE *     waTable  = &mediaCtx->WaTable;
        skuTable->reset();
        waTable->reset();
        // get Sku/Wa tables and platform information
        PLATFORM platform = {};
        // Allocate memory for Media System Info
        mediaCtx->pGtSystemInfo = (MEDIA_SYSTEM_INFO *)MOS_AllocAndZeroMemory(sizeof(MEDIA_SYSTEM_INFO));
        if (nullptr == mediaCtx->pGtSystemInfo)
        {
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        MOS_STATUS eStatus = HWInfo_GetGfxInfo(mediaCtx->fd, mediaCtx->pDrmBufMgr, &platform, skuTable, waTable, mediaCtx->pGtSystemInfo);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            DDI_ASSERTMESSAGE("Fatal error - unsuccesfull Sku/Wa/GtSystemInfo initialization");
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }
        mediaCtx->platform = platform;

        MOS_TraceSetupInfo(
            (VA_MAJOR_VERSION << 16) | VA_MINOR_VERSION,
            platform.eProductFamily,
            platform.eRenderCoreFamily,
            (platform.usRevId << 16) | platform.usDeviceID);

        //TO--DO, gen12+ still need this wa????, not porting yet
        if (MEDIA_IS_SKU(skuTable, FtrEnableMediaKernels) == 0)
        {
            MEDIA_WR_WA(waTable, WaHucStreamoutOnlyDisable, 0);
        }
        MediaUserSettingsMgr::MediaUserSettingsInit(platform.eProductFamily);

        GMM_SKU_FEATURE_TABLE gmmSkuTable;
        memset(&gmmSkuTable, 0, sizeof(gmmSkuTable));

        GMM_WA_TABLE gmmWaTable;
        memset(&gmmWaTable, 0, sizeof(gmmWaTable));

        GMM_GT_SYSTEM_INFO gmmGtInfo;
        memset(&gmmGtInfo, 0, sizeof(gmmGtInfo));

        eStatus = HWInfo_GetGmmInfo(mediaCtx->fd, &gmmSkuTable, &gmmWaTable, &gmmGtInfo);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            DDI_ASSERTMESSAGE("Fatal error - unsuccesfull Gmm Sku/Wa/GtSystemInfo initialization");
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }

        eStatus = Mos_Solo_DdiInitializeDeviceId(
            (void *)mediaCtx->pDrmBufMgr,
            &mediaCtx->SkuTable,
            &mediaCtx->WaTable,
            &gmmSkuTable,
            &gmmWaTable,
            &gmmGtInfo,
            &mediaCtx->iDeviceId,
            &mediaCtx->fd,
            &mediaCtx->platform);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }

        GMM_STATUS gmmStatus = OpenGmm(&mediaCtx->GmmFuncs);
        if (gmmStatus != GMM_SUCCESS)
        {
            DDI_ASSERTMESSAGE("gmm init failed.");
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }

        // init GMM context
        gmmStatus = mediaCtx->GmmFuncs.pfnCreateSingletonContext(mediaCtx->platform,
            &gmmSkuTable,
            &gmmWaTable,
            &gmmGtInfo);

        if (gmmStatus != GMM_SUCCESS)
        {
            DDI_ASSERTMESSAGE("gmm init failed.");
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }

        // Create GMM Client Context
        mediaCtx->pGmmClientContext = mediaCtx->GmmFuncs.pfnCreateClientContext((GMM_CLIENT)GMM_LIBVA_LINUX);

        // Create GMM page table manager
        mediaCtx->m_auxTableMgr = AuxTableMgr::CreateAuxTableMgr(mediaCtx->pDrmBufMgr, &mediaCtx->SkuTable);

        MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_SIM_ENABLE_ID,
            &UserFeatureData,
            nullptr);
#endif

        mediaCtx->m_useSwSwizzling = UserFeatureData.i32Data || MEDIA_IS_SKU(&mediaCtx->SkuTable, FtrUseSwSwizzling);
        mediaCtx->m_tileYFlag      = MEDIA_IS_SKU(&mediaCtx->SkuTable, FtrTileY);

        mediaCtx->m_osContext = OsContext::GetOsContextObject();
        if (mediaCtx->m_osContext == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Unable to get the active OS context.");
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }

        // fill in the mos context struct as input to initialize m_osContext
        MOS_CONTEXT mosCtx           = {};
        mosCtx.bufmgr                = mediaCtx->pDrmBufMgr;
        mosCtx.fd                    = mediaCtx->fd;
        mosCtx.iDeviceId             = mediaCtx->iDeviceId;
        mosCtx.SkuTable              = mediaCtx->SkuTable;
        mosCtx.WaTable               = mediaCtx->WaTable;
        mosCtx.gtSystemInfo          = *mediaCtx->pGtSystemInfo;
        mosCtx.platform              = mediaCtx->platform;
        mosCtx.ppMediaMemDecompState = &mediaCtx->pMediaMemDecompState;
        mosCtx.pfnMemoryDecompress   = mediaCtx->pfnMemoryDecompress;
        mosCtx.pfnMediaMemoryCopy    = mediaCtx->pfnMediaMemoryCopy;
        mosCtx.pfnMediaMemoryCopy2D  = mediaCtx->pfnMediaMemoryCopy2D;
        mosCtx.ppMediaCopyState      = &mediaCtx->pMediaCopyState;
        mosCtx.m_auxTableMgr         = mediaCtx->m_auxTableMgr;
        mosCtx.pGmmClientContext     = mediaCtx->pGmmClientContext;

        eStatus = mediaCtx->m_osContext->Init(&mosCtx);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            MOS_OS_ASSERTMESSAGE("Unable to initialize OS context.");
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }

        // Prepare the command buffer manager
        mediaCtx->m_cmdBufMgr = CmdBufMgr::GetObject();
        if (mediaCtx->m_cmdBufMgr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE(" nullptr returned by CmdBufMgr::GetObject");
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }

        MOS_STATUS ret = mediaCtx->m_cmdBufMgr->Initialize(mediaCtx->m_osContext, COMMAND_BUFFER_SIZE/2);
        if (ret != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE(" cmdBufMgr Initialization failed");
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }

        // Prepare the gpu Context manager
        mediaCtx->m_gpuContextMgr = GpuContextMgr::GetObject(mediaCtx->pGtSystemInfo, mediaCtx->m_osContext);
        if (mediaCtx->m_gpuContextMgr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE(" nullptr returned by GpuContextMgr::GetObject");
            FreeForMediaContext(mediaCtx);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }
    }
    else
    {
        MOS_OS_ASSERTMESSAGE(" Invalid mos module state");
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (DdiMedia_HeapInitialize(mediaCtx) != VA_STATUS_SUCCESS)
    {
        DestroyMediaContextMutex(mediaCtx);
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    //Caps need platform and sku table, especially in MediaLibvaCapsCp::IsDecEncryptionSupported
    mediaCtx->m_caps = MediaLibvaCaps::CreateMediaLibvaCaps(mediaCtx);
    if (!mediaCtx->m_caps)
    {
        DDI_ASSERTMESSAGE("Caps create failed. Not supported GFX device.");
        DestroyMediaContextMutex(mediaCtx);
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (mediaCtx->m_caps->Init() != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Caps init failed. Not supported GFX device.");
        MOS_Delete(mediaCtx->m_caps);
        mediaCtx->m_caps = nullptr;
        DestroyMediaContextMutex(mediaCtx);
        FreeForMediaContext(mediaCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    ctx->max_image_formats = mediaCtx->m_caps->GetImageFormatsMaxNum();

#if !defined(ANDROID) && defined(X11_FOUND)
    DdiMediaUtil_InitMutex(&mediaCtx->PutSurfaceRenderMutex);
    DdiMediaUtil_InitMutex(&mediaCtx->PutSurfaceSwapBufferMutex);

    // try to open X11 lib, if fail, assume no X11 environment
    if (VA_STATUS_SUCCESS != DdiMedia_ConnectX11(mediaCtx))
    {
        // assume no X11 environment. In current implementation,
        // PutSurface (Linux) needs X11 support, so just replace
        // it with a dummy version. DdiCodec_PutSurfaceDummy() will
        // return VA_STATUS_ERROR_UNIMPLEMENTED directly.
        ctx->vtable->vaPutSurface = NULL;
    }
    output_dri_init(ctx);
#endif

    DdiMediaUtil_SetMediaResetEnableFlag(mediaCtx);

    DdiMediaUtil_UnLockMutex(&GlobalMutex);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_Terminate (
    VADriverContextP ctx
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,       "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DdiMediaUtil_LockMutex(&GlobalMutex);

#if !defined(ANDROID) && defined(X11_FOUND)
    DdiMedia_DestroyX11Connection(mediaCtx);
    DdiMediaUtil_DestroyMutex(&mediaCtx->PutSurfaceRenderMutex);
    DdiMediaUtil_DestroyMutex(&mediaCtx->PutSurfaceSwapBufferMutex);

    if (mediaCtx->m_caps)
    {
        if (mediaCtx->dri_output != nullptr) {
            if (mediaCtx->dri_output->handle)
                dso_close(mediaCtx->dri_output->handle);

            free(mediaCtx->dri_output);
            mediaCtx->dri_output = nullptr;
        }
    }
#endif

    if (mediaCtx->m_caps)
    {
        MOS_Delete(mediaCtx->m_caps);
        mediaCtx->m_caps = nullptr;
    }
    //destory resources
    DdiMedia_FreeSurfaceHeapElements(mediaCtx);
    DdiMedia_FreeBufferHeapElements(ctx);
    DdiMedia_FreeImageHeapElements(ctx);
    DdiMedia_FreeContextHeapElements(ctx);
    DdiMedia_FreeContextCMElements(ctx);

    DdiMedia_HeapDestroy(mediaCtx);
    DdiMediaProtected::FreeInstances();

    if (mediaCtx->m_apoMosEnabled)
    {
        MosInterface::DestroyOsDeviceContext(mediaCtx->m_osDeviceContext);
        mediaCtx->m_osDeviceContext = MOS_INVALID_HANDLE;
        MOS_FreeMemory(mediaCtx->pGtSystemInfo);
        MosInterface::CloseOsUtilities(nullptr);
    }
    else if (mediaCtx->modularizedGpuCtxEnabled)
    {
        if (mediaCtx->m_auxTableMgr != nullptr)
        {
            MOS_Delete(mediaCtx->m_auxTableMgr);
            mediaCtx->m_auxTableMgr = nullptr;
        }

        if (mediaCtx->m_gpuContextMgr)
        {
            mediaCtx->m_gpuContextMgr->CleanUp();
            MOS_Delete(mediaCtx->m_gpuContextMgr);
        }

        if (mediaCtx->m_cmdBufMgr)
        {
            mediaCtx->m_cmdBufMgr->CleanUp();
            MOS_Delete(mediaCtx->m_cmdBufMgr);
        }

        if (mediaCtx->m_osContext)
        {
            mediaCtx->m_osContext->CleanUp();
            MOS_Delete(mediaCtx->m_osContext);
        }

        // destroy libdrm buffer manager
        mos_bufmgr_destroy(mediaCtx->pDrmBufMgr);

        // Destroy memory allocated to store Media System Info
        MOS_FreeMemory(mediaCtx->pGtSystemInfo);
        // Free GMM memory.
        mediaCtx->GmmFuncs.pfnDeleteClientContext(mediaCtx->pGmmClientContext);
        mediaCtx->GmmFuncs.pfnDestroySingletonContext();
        MosUtilities::MosUtilitiesClose(nullptr);
    }

    if (mediaCtx->uiRef > 1)
    {
        mediaCtx->uiRef--;
        DdiMediaUtil_UnLockMutex(&GlobalMutex);

        return VA_STATUS_SUCCESS;
    }
    mediaCtx->SkuTable.reset();
    mediaCtx->WaTable.reset();

    // release media driver context, ctx creation is behind the mos_utilities_init
    // If free earilier than MOS_utilities_close, memnja count error.
    MOS_FreeMemory(mediaCtx);
    mediaCtx         = nullptr;
    ctx->pDriverData = nullptr;

    DdiMediaUtil_UnLockMutex(&GlobalMutex);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_QueryConfigEntrypoints(
    VADriverContextP    ctx,
    VAProfile           profile,
    VAEntrypoint       *entrypoint_list,
    int32_t            *num_entrypoints
)
{
    DDI_FUNCTION_ENTER();

    PERF_UTILITY_START_ONCE("First Frame Time", PERF_MOS, PERF_LEVEL_DDI);

    DDI_CHK_NULL(ctx, "nullptr Ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(entrypoint_list, "nullptr entrypoint_list", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(num_entrypoints, "nullptr num_entrypoints", VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_caps->QueryConfigEntrypoints(profile,
            entrypoint_list, num_entrypoints);
}

VAStatus DdiMedia_QueryConfigProfiles (
    VADriverContextP    ctx,
    VAProfile          *profile_list,
    int32_t            *num_profiles
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr Ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(profile_list, "nullptr profile_list", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(num_profiles, "nullptr num_profiles", VA_STATUS_ERROR_INVALID_PARAMETER);

    return mediaCtx->m_caps->QueryConfigProfiles(profile_list, num_profiles);
}

VAStatus DdiMedia_QueryConfigAttributes (
    VADriverContextP    ctx,
    VAConfigID          config_id,
    VAProfile          *profile,
    VAEntrypoint       *entrypoint,
    VAConfigAttrib     *attrib_list,
    int32_t            *num_attribs
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(profile,     "nullptr profile", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(entrypoint,  "nullptr entrypoint", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ctx,         "nullptr Ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(num_attribs, "nullptr num_attribs", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,   "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_caps->QueryConfigAttributes(
                config_id, profile, entrypoint, attrib_list, num_attribs);
}

VAStatus DdiMedia_CreateConfig (
    VADriverContextP    ctx,
    VAProfile           profile,
    VAEntrypoint        entrypoint,
    VAConfigAttrib     *attrib_list,
    int32_t             num_attribs,
    VAConfigID         *config_id
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,       "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(config_id, "nullptr config_id", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_caps->CreateConfig(
            profile, entrypoint, attrib_list, num_attribs, config_id);
}

VAStatus DdiMedia_DestroyConfig (
    VADriverContextP    ctx,
    VAConfigID          config_id
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,       "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_caps->DestroyConfig(config_id);
}

VAStatus DdiMedia_GetConfigAttributes(
    VADriverContextP    ctx,
    VAProfile           profile,
    VAEntrypoint        entrypoint,
    VAConfigAttrib     *attrib_list,
    int32_t             num_attribs
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,       "nullptr ctx",       VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_caps->GetConfigAttributes(
            profile, entrypoint, attrib_list, num_attribs);
}

VAStatus DdiMedia_CreateSurfaces (
    VADriverContextP    ctx,
    int32_t             width,
    int32_t             height,
    int32_t             format,
    int32_t             num_surfaces,
    VASurfaceID        *surfaces
)
{
    DDI_FUNCTION_ENTER();
    int32_t event[] = {width, height, format};
    MOS_TraceEventExt(EVENT_VA_SURFACE, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL(ctx,               "nullptr ctx",             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LARGER(num_surfaces, 0, "Invalid num_surfaces", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(surfaces,          "nullptr surfaces",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(width,        0, "Invalid width",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(height,       0, "Invalid height",       VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaDrvCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaDrvCtx,       "nullptr mediaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    if( format != VA_RT_FORMAT_YUV420 ||
        format != VA_RT_FORMAT_YUV422 ||
        format != VA_RT_FORMAT_YUV444 ||
        format != VA_RT_FORMAT_YUV400 ||
        format != VA_RT_FORMAT_YUV411)
    {
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    DDI_MEDIA_FORMAT mediaFmt = DdiMedia_OsFormatToMediaFormat(VA_FOURCC_NV12,format);

    height = MOS_ALIGN_CEIL(height, 16);
    for(int32_t i = 0; i < num_surfaces; i++)
    {
        VASurfaceID vaSurfaceID = (VASurfaceID)DdiMedia_CreateRenderTarget(mediaDrvCtx, mediaFmt, width, height, nullptr, VA_SURFACE_ATTRIB_USAGE_HINT_GENERIC, MOS_MEMPOOL_VIDEOMEMORY);
        if (VA_INVALID_ID != vaSurfaceID)
            surfaces[i] = vaSurfaceID;
        else
        {
            // here to release the successful allocated surfaces?
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }

    MOS_TraceEventExt(EVENT_VA_SURFACE, EVENT_TYPE_END, &num_surfaces, sizeof(int32_t), surfaces, num_surfaces*sizeof(VAGenericID));
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_DestroySurfaces (
    VADriverContextP    ctx,
    VASurfaceID        *surfaces,
    int32_t             num_surfaces
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL  (ctx,             "nullptr ctx",             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LARGER(num_surfaces, 0, "Invalid num_surfaces", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL  (surfaces,        "nullptr surfaces",        VA_STATUS_ERROR_INVALID_PARAMETER);
    MOS_TraceEventExt(EVENT_VA_FREE_SURFACE, EVENT_TYPE_START, &num_surfaces, sizeof(int32_t), surfaces, num_surfaces*sizeof(VAGenericID));

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL  (mediaCtx,                  "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL  (mediaCtx->pSurfaceHeap,    "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_SURFACE surface = nullptr;
    for(int32_t i = 0; i < num_surfaces; i++)
    {
        DDI_CHK_LESS((uint32_t)surfaces[i], mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaces", VA_STATUS_ERROR_INVALID_SURFACE);
        surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surfaces[i]);
        DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
        if(surface->pCurrentFrameSemaphore)
        {
            DdiMediaUtil_WaitSemaphore(surface->pCurrentFrameSemaphore);
            DdiMediaUtil_PostSemaphore(surface->pCurrentFrameSemaphore);
        }
        if(surface->pReferenceFrameSemaphore)
        {
            DdiMediaUtil_WaitSemaphore(surface->pReferenceFrameSemaphore);
            DdiMediaUtil_PostSemaphore(surface->pReferenceFrameSemaphore);
        }
    }

    for(int32_t i = 0; i < num_surfaces; i++)
    {
        DDI_CHK_LESS((uint32_t)surfaces[i], mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaces", VA_STATUS_ERROR_INVALID_SURFACE);
        surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surfaces[i]);
        DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
        if(surface->pCurrentFrameSemaphore)
        {
            DdiMediaUtil_DestroySemaphore(surface->pCurrentFrameSemaphore);
            surface->pCurrentFrameSemaphore = nullptr;
        }

        if(surface->pReferenceFrameSemaphore)
        {
            DdiMediaUtil_DestroySemaphore(surface->pReferenceFrameSemaphore);
            surface->pReferenceFrameSemaphore = nullptr;
        }

        DdiMediaUtil_UnRegisterRTSurfaces(ctx, surface);

        DdiMediaUtil_LockMutex(&mediaCtx->SurfaceMutex);
        DdiMediaUtil_FreeSurface(surface);
        MOS_FreeMemory(surface);
        DdiMediaUtil_ReleasePMediaSurfaceFromHeap(mediaCtx->pSurfaceHeap, (uint32_t)surfaces[i]);
        mediaCtx->uiNumSurfaces--;
        DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);
    }

    MOS_TraceEventExt(EVENT_VA_FREE_SURFACE, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_CreateSurfaces2(
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
    DDI_FUNCTION_ENTER();
    uint32_t event[] = {width, height, format};
    MOS_TraceEventExt(EVENT_VA_SURFACE, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL  (ctx,             "nullptr ctx",             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LARGER(num_surfaces, 0, "Invalid num_surfaces", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL  (surfaces,        "nullptr surfaces",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(width,        0, "Invalid width",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(height,       0, "Invalid height",       VA_STATUS_ERROR_INVALID_PARAMETER);

    if(num_attribs > 0)
    {
        DDI_CHK_NULL(attrib_list, "nullptr attrib_list", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    PDDI_MEDIA_CONTEXT mediaCtx    = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,       "nullptr mediaCtx",   VA_STATUS_ERROR_INVALID_CONTEXT);

    int32_t expected_fourcc = VA_FOURCC_NV12;
    switch(format)
    {
        case VA_RT_FORMAT_YUV420:
            expected_fourcc = VA_FOURCC_NV12;
            break;
        case VA_RT_FORMAT_YUV420_12:
            expected_fourcc = VA_FOURCC_P012;
            break;
        case VA_RT_FORMAT_YUV422:
            expected_fourcc = VA_FOURCC_YUY2;
            break;
        case VA_RT_FORMAT_YUV422_10:
            expected_fourcc = VA_FOURCC_Y210;
            break;
        case VA_RT_FORMAT_YUV422_12:
#if VA_CHECK_VERSION(1, 9, 0)
            expected_fourcc = VA_FOURCC_Y212;
#else
            expected_fourcc = VA_FOURCC_Y216;
#endif
            break;
        case VA_RT_FORMAT_YUV444:
            expected_fourcc = VA_FOURCC_444P;
            break;
        case VA_RT_FORMAT_YUV444_10:
            expected_fourcc = VA_FOURCC_Y410;
            break;
        case VA_RT_FORMAT_YUV444_12:
#if VA_CHECK_VERSION(1, 9, 0)
            expected_fourcc = VA_FOURCC_Y412;
#else
            expected_fourcc = VA_FOURCC_Y416;
#endif
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
        case VA_FOURCC_P010:
            expected_fourcc = VA_FOURCC_P010;
            break;
        case VA_FOURCC_P012:
            expected_fourcc = VA_FOURCC_P012;
            break;
        case VA_FOURCC_P016:
            expected_fourcc = VA_FOURCC_P016;
            break;
        case VA_FOURCC_Y210:
            expected_fourcc = VA_FOURCC_Y210;
            break;
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y212:
            expected_fourcc = VA_FOURCC_Y212;
            break;
#endif
        case VA_FOURCC_Y216:
            expected_fourcc = VA_FOURCC_Y216;
            break;
        case VA_FOURCC_AYUV:
            expected_fourcc = VA_FOURCC_AYUV;
            break;
        case VA_FOURCC_Y410:
            expected_fourcc = VA_FOURCC_Y410;
            break;
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FOURCC_Y412:
            expected_fourcc = VA_FOURCC_Y412;
            break;
#endif
        case VA_FOURCC_Y416:
            expected_fourcc = VA_FOURCC_Y416;
            break;
        case VA_FOURCC_I420:
            expected_fourcc = VA_FOURCC_I420;
            break;
#endif
        default:
            DDI_ASSERTMESSAGE("Invalid VAConfigAttribRTFormat: 0x%x. Please uses the format defined in libva/va.h", format);
            return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    VASurfaceAttribExternalBuffers externalBufDescripor;
    VADRMPRIMESurfaceDescriptor drmPrimeSurfaceDescriptor;
    MOS_ZeroMemory(&externalBufDescripor, sizeof(VASurfaceAttribExternalBuffers));
    MOS_ZeroMemory(&drmPrimeSurfaceDescriptor, sizeof(VADRMPRIMESurfaceDescriptor));

    int32_t  memTypeFlag      = VA_SURFACE_ATTRIB_MEM_TYPE_VA;
    int32_t  descFlag         = 0;
    uint32_t surfaceUsageHint = VA_SURFACE_ATTRIB_USAGE_HINT_GENERIC;
    bool     surfDescProvided = false;
    bool     surfIsUserPtr    = false;

    for (uint32_t i = 0; i < num_attribs && attrib_list; i++)
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
                if ((attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_VA)          ||
                    (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM)  ||
                    (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME)   ||
                    (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2) ||
                    (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR))
                {
                    memTypeFlag = attrib_list[i].value.value.i;
                    surfIsUserPtr = (attrib_list[i].value.value.i == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR);
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
                    //remove the check for libva-utils conformance test, need libva-utils change cases
                    //after libva-utils fix the case, return VA_STATUS_ERROR_INVALID_PARAMETER;
                    break;
                }
                surfDescProvided = true;
                if (memTypeFlag == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2)
                {
                    MOS_SecureMemcpy(&drmPrimeSurfaceDescriptor, sizeof(VADRMPRIMESurfaceDescriptor),  attrib_list[i].value.value.p, sizeof(VADRMPRIMESurfaceDescriptor));
                    expected_fourcc  = drmPrimeSurfaceDescriptor.fourcc;
                    width            = drmPrimeSurfaceDescriptor.width;
                    height           = drmPrimeSurfaceDescriptor.height;
                }
                else
                {
                    MOS_SecureMemcpy(&externalBufDescripor, sizeof(VASurfaceAttribExternalBuffers),  attrib_list[i].value.value.p, sizeof(VASurfaceAttribExternalBuffers));

                    expected_fourcc  = externalBufDescripor.pixel_format;
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

    DDI_MEDIA_FORMAT mediaFmt = DdiMedia_OsFormatToMediaFormat(expected_fourcc,format);
    if (mediaFmt == Media_Format_Count)
    {
        DDI_ASSERTMESSAGE("DDI: unsupported surface type in DdiMedia_CreateSurfaces2.");
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }

    for (uint32_t i = 0; i < num_surfaces; i++)
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

            if (memTypeFlag == VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2)
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
            else if (memTypeFlag != VA_SURFACE_ATTRIB_MEM_TYPE_VA) {
                surfDesc->uiPlanes       = externalBufDescripor.num_planes;
                surfDesc->ulBuffer       = externalBufDescripor.buffers[i];
                surfDesc->uiSize         = externalBufDescripor.data_size;
                surfDesc->uiBuffserSize  = externalBufDescripor.data_size;

                eStatus = MOS_SecureMemcpy(surfDesc->uiPitches, sizeof(surfDesc->uiPitches), externalBufDescripor.pitches, sizeof(externalBufDescripor.pitches));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    DDI_VERBOSEMESSAGE("DDI:Failed to copy surface buffer data!");
                    return VA_STATUS_ERROR_OPERATION_FAILED;
                }
                eStatus = MOS_SecureMemcpy(surfDesc->uiOffsets, sizeof(surfDesc->uiOffsets), externalBufDescripor.offsets, sizeof(externalBufDescripor.offsets));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    DDI_VERBOSEMESSAGE("DDI:Failed to copy surface buffer data!");
                    return VA_STATUS_ERROR_OPERATION_FAILED;
                }

                if( surfIsUserPtr )
                {
                    surfDesc->uiTile = I915_TILING_NONE;
                    if (surfDesc->ulBuffer % 4096 != 0)
                    {
                        MOS_FreeMemory(surfDesc);
                        DDI_VERBOSEMESSAGE("Buffer Address is invalid");
                        return VA_STATUS_ERROR_INVALID_PARAMETER;
                    }
                }
            }
        }
        VASurfaceID vaSurfaceID = (VASurfaceID)DdiMedia_CreateRenderTarget(mediaCtx, mediaFmt, width, height, surfDesc, surfaceUsageHint, MOS_MEMPOOL_VIDEOMEMORY);
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

    MOS_TraceEventExt(EVENT_VA_SURFACE, EVENT_TYPE_END, &num_surfaces, sizeof(uint32_t), surfaces, num_surfaces*sizeof(VAGenericID));
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_CreateMfeContextInternal(
    VADriverContextP    ctx,
    VAMFContextID      *mfe_context
)
{
    DDI_FUNCTION_ENTER();

    PDDI_MEDIA_CONTEXT mediaDrvCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaDrvCtx, "nullptr pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mfe_context, "nullptr mfe_context", VA_STATUS_ERROR_INVALID_PARAMETER);
    *mfe_context        = DDI_MEDIA_INVALID_VACONTEXTID;

    if (!mediaDrvCtx->m_caps->IsMfeSupportedOnPlatform(mediaDrvCtx->platform))
    {
        DDI_VERBOSEMESSAGE("MFE is not supported on the platform!");
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    PDDI_ENCODE_MFE_CONTEXT encodeMfeContext = (PDDI_ENCODE_MFE_CONTEXT)MOS_AllocAndZeroMemory(sizeof(DDI_ENCODE_MFE_CONTEXT));
    if (nullptr == encodeMfeContext)
    {
        MOS_FreeMemory(encodeMfeContext);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    DdiMediaUtil_LockMutex(&mediaDrvCtx->MfeMutex);
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaContextHeapElmt = DdiMediaUtil_AllocPVAContextFromHeap(mediaDrvCtx->pMfeCtxHeap);
    if (nullptr == vaContextHeapElmt)
    {
        DdiMediaUtil_UnLockMutex(&mediaDrvCtx->MfeMutex);
        MOS_FreeMemory(encodeMfeContext);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    vaContextHeapElmt->pVaContext    = (void*)encodeMfeContext;
    mediaDrvCtx->uiNumMfes++;
    *mfe_context                     = (VAMFContextID)(vaContextHeapElmt->uiVaContextID + DDI_MEDIA_VACONTEXTID_OFFSET_MFE);
    DdiMediaUtil_UnLockMutex(&mediaDrvCtx->MfeMutex);

    // Create shared state, which is used by all the sub contexts
    MfeSharedState *mfeEncodeSharedState   = (MfeSharedState*)MOS_AllocAndZeroMemory(sizeof(MfeSharedState));
    if (nullptr == mfeEncodeSharedState)
    {
        MOS_FreeMemory(encodeMfeContext);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    encodeMfeContext->mfeEncodeSharedState = mfeEncodeSharedState;

    DdiMediaUtil_InitMutex(&encodeMfeContext->encodeMfeMutex);

    return VA_STATUS_SUCCESS;
}

static VAStatus DdiMedia_DestoryMfeContext (
    VADriverContextP    ctx,
    VAMFContextID      mfe_context
)
{
    DDI_FUNCTION_ENTER();

    PDDI_MEDIA_CONTEXT mediaCtx              = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    PDDI_ENCODE_MFE_CONTEXT encodeMfeContext = (PDDI_ENCODE_MFE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, mfe_context, &ctxType);
    DDI_CHK_NULL(encodeMfeContext, "nullptr encodeMfeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    // Release std::vector memory
    encodeMfeContext->pDdiEncodeContexts.clear();
    encodeMfeContext->pDdiEncodeContexts.shrink_to_fit();

    encodeMfeContext->mfeEncodeSharedState->encoders.clear();
    encodeMfeContext->mfeEncodeSharedState->encoders.shrink_to_fit();

    DdiMediaUtil_DestroyMutex(&encodeMfeContext->encodeMfeMutex);
    MOS_FreeMemory(encodeMfeContext->mfeEncodeSharedState);
    MOS_FreeMemory(encodeMfeContext);

    DdiMediaUtil_LockMutex(&mediaCtx->MfeMutex);
    DdiMediaUtil_ReleasePVAContextFromHeap(mediaCtx->pMfeCtxHeap, (mfe_context & DDI_MEDIA_MASK_VACONTEXTID));
    mediaCtx->uiNumMfes--;
    DdiMediaUtil_UnLockMutex(&mediaCtx->MfeMutex);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_AddContextInternal(
    VADriverContextP    ctx,
    VAContextID         context,
    VAMFContextID      mfe_context
)
{
    DDI_FUNCTION_ENTER();

    PDDI_MEDIA_CONTEXT      mediaCtx         = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t                ctxType          = DDI_MEDIA_CONTEXT_TYPE_NONE;
    PDDI_ENCODE_MFE_CONTEXT encodeMfeContext = (PDDI_ENCODE_MFE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, mfe_context, &ctxType);
    DDI_CHK_NULL(encodeMfeContext, "nullptr encodeMfeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (ctxType != DDI_MEDIA_CONTEXT_TYPE_MFE)
    {
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    PDDI_ENCODE_CONTEXT     encodeContext    = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(encodeContext, "nullptr encodeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    CodechalEncoderState    *encoder         = dynamic_cast<CodechalEncoderState *>(encodeContext->pCodecHal);
    DDI_CHK_NULL(encoder, "nullptr codechal encoder", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (!mediaCtx->m_caps->IsMfeSupportedEntrypoint(encodeContext->vaEntrypoint))
    {
        return VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
    }

    if (!mediaCtx->m_caps->IsMfeSupportedProfile(encodeContext->vaProfile))
    {
        return VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
    }

    DdiMediaUtil_LockMutex(&encodeMfeContext->encodeMfeMutex);
    encodeMfeContext->pDdiEncodeContexts.push_back(encodeContext);

    if (encodeMfeContext->currentStreamId == 0)
    {
        encodeMfeContext->isFEI = (encodeContext->vaEntrypoint == VAEntrypointFEI) ? true : false;
    }

    //MFE cannot support legacy and FEI together
    if ((encodeContext->vaEntrypoint != VAEntrypointFEI && encodeMfeContext->isFEI) ||
        (encodeContext->vaEntrypoint == VAEntrypointFEI && !encodeMfeContext->isFEI))
    {
        DdiMediaUtil_UnLockMutex(&encodeMfeContext->encodeMfeMutex);
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    encoder->m_mfeEnabled = true;
    // Assign one unique id to this sub context/stream
    encoder->m_mfeEncodeParams.streamId = encodeMfeContext->currentStreamId;

    MOS_STATUS eStatus = encoder->SetMfeSharedState(encodeMfeContext->mfeEncodeSharedState);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE(
            "Failed to set MFE Shared State for encoder #%d",
            encodeMfeContext->currentStreamId);

        encoder->m_mfeEnabled = false;

        DdiMediaUtil_UnLockMutex(&encodeMfeContext->encodeMfeMutex);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    encodeMfeContext->currentStreamId++;
    DdiMediaUtil_UnLockMutex(&encodeMfeContext->encodeMfeMutex);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_ReleaseContextInternal(
    VADriverContextP    ctx,
    VAContextID         context,
    VAMFContextID      mfe_context
)
{
    DDI_FUNCTION_ENTER();

    PDDI_MEDIA_CONTEXT mediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t  ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    PDDI_ENCODE_MFE_CONTEXT encodeMfeContext  = (PDDI_ENCODE_MFE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, mfe_context, &ctxType);
    DDI_CHK_NULL(encodeMfeContext, "nullptr encodeMfeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (ctxType != DDI_MEDIA_CONTEXT_TYPE_MFE ||
        encodeMfeContext->pDdiEncodeContexts.size() == 0)
    {
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    PDDI_ENCODE_CONTEXT encodeContext  = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(encodeMfeContext, "nullptr encodeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    bool contextErased = false;
    DdiMediaUtil_LockMutex(&encodeMfeContext->encodeMfeMutex);
    for (uint32_t i = 0; i < encodeMfeContext->pDdiEncodeContexts.size(); i++)
    {
        if (encodeMfeContext->pDdiEncodeContexts[i] == encodeContext)
        {
            encodeMfeContext->pDdiEncodeContexts.erase(encodeMfeContext->pDdiEncodeContexts.begin() + i);
            contextErased = true;
            break;
        }
    }

    if (!contextErased)
    {
        DdiMediaUtil_UnLockMutex(&encodeMfeContext->encodeMfeMutex);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    DdiMediaUtil_UnLockMutex(&encodeMfeContext->encodeMfeMutex);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_CreateContext (
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
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,     "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(context, "nullptr context",      VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaDrvCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaDrvCtx,     "nullptr mediaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(num_render_targets > 0)
    {
        DDI_CHK_NULL(render_targets,             "nullptr render_targets",             VA_STATUS_ERROR_INVALID_PARAMETER);
        DDI_CHK_NULL(mediaDrvCtx->pSurfaceHeap, "nullptr mediaDrvCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
        for(int32_t i = 0; i < num_render_targets; i++)
        {
            uint32_t surfaceId = (uint32_t)render_targets[i];
            DDI_CHK_LESS(surfaceId, mediaDrvCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid Surface", VA_STATUS_ERROR_INVALID_SURFACE);
        }
    }

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    if(mediaDrvCtx->m_caps->IsDecConfigId(config_id))
    {
        vaStatus = DdiDecode_CreateContext(ctx, config_id - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE, picture_width, picture_height, flag, render_targets, num_render_targets, context);
    }
    else if(mediaDrvCtx->m_caps->IsEncConfigId(config_id))
    {
        vaStatus = DdiEncode_CreateContext(ctx, config_id - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE, picture_width, picture_height, flag, render_targets, num_render_targets, context);
    }
    else if(mediaDrvCtx->m_caps->IsVpConfigId(config_id))
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

VAStatus DdiMedia_DestroyContext (
    VADriverContextP    ctx,
    VAContextID         context
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t            ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);

    switch (ctxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
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

VAStatus DdiMedia_CreateBuffer (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferType        type,
    uint32_t            size,
    uint32_t            num_elements,
    void                *data,
    VABufferID          *bufId
)
{
    DDI_FUNCTION_ENTER();
    int32_t event[] = {size, num_elements, type};
    MOS_TraceEventExt(EVENT_VA_BUFFER, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL(ctx,        "nullptr ctx",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(bufId,     "nullptr buf_id",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(size, 0,  "Invalid size", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    DDI_CHK_NULL(ctxPtr,      "nullptr ctxPtr",      VA_STATUS_ERROR_INVALID_CONTEXT);

    *bufId     = VA_INVALID_ID;

    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    VAStatus va = VA_STATUS_SUCCESS;
    switch (ctxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
            va = DdiDecode_CreateBuffer(ctx, DdiDecode_GetDecContextFromPVOID(ctxPtr), type, size, num_elements, data, bufId);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            va = DdiEncode_CreateBuffer(ctx, context, type, size, num_elements, data, bufId);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            va = DdiVp_CreateBuffer(ctx, ctxPtr, type, size, num_elements, data, bufId);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_PROTECTED:
            va = DdiMediaProtected::DdiMedia_ProtectedSessionCreateBuffer(ctx, context, type, size, num_elements, data, bufId);
            break;
        default:
            va = VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
    MOS_TraceEventExt(EVENT_VA_BUFFER, EVENT_TYPE_END, bufId, sizeof(bufId), nullptr, 0);
    return va;
}

VAStatus DdiMedia_BufferSetNumElements (
    VADriverContextP    ctx,
    VABufferID          buf_id,
    uint32_t            num_elements
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,              "nullptr mediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)buf_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid buf_id", VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_MEDIA_BUFFER *buf       = DdiMedia_GetBufferFromVABufferID(mediaCtx, buf_id);
    DDI_CHK_NULL(buf, "Invalid buffer.", VA_STATUS_ERROR_INVALID_BUFFER);

    // only for VASliceParameterBufferType of buffer, the number of elements can be greater than 1
    if(buf->uiType != VASliceParameterBufferType &&
       num_elements > 1)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if(buf->uiType == VASliceParameterBufferType &&
       buf->uiNumElements < num_elements)
    {
        MOS_FreeMemory(buf->pData);
        buf->iSize = buf->iSize / buf->uiNumElements;
        buf->pData = (uint8_t*)MOS_AllocAndZeroMemory(buf->iSize * num_elements);
        buf->iSize = buf->iSize * num_elements;
    }

    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Map data store of the buffer into the client's address space
//!         vaCreateBuffer() needs to be called with "data" set to nullptr before
//!         calling vaMapBuffer()
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] buf_id
//!         VA buffer ID
//! \param  [out] pbuf
//!         Pointer to buffer
//! \param  [in] flag
//!         Flag
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_MapBufferInternal (
    VADriverContextP    ctx,
    VABufferID          buf_id,
    void              **pbuf,
    uint32_t            flag
)
{
    DDI_FUNCTION_ENTER();
    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_START, &buf_id, sizeof(buf_id), &flag, sizeof(flag));

    DDI_CHK_NULL(ctx,   "nullptr ctx",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pbuf,  "nullptr pbuf",    VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,              "nullptr mediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)buf_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufferId", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_BUFFER   *buf     = DdiMedia_GetBufferFromVABufferID(mediaCtx, buf_id);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    // The context is nullptr when the buffer is created from DdiMedia_DeriveImage
    // So doesn't need to check the context for all cases
    // Only check the context in dec/enc mode
    VAStatus                 vaStatus  = VA_STATUS_SUCCESS;
    uint32_t                 ctxType = DdiMedia_GetCtxTypeFromVABufferID(mediaCtx, buf_id);
    void                     *ctxPtr = nullptr;
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = nullptr;
    PDDI_ENCODE_CONTEXT      encCtx  = nullptr;
    PDDI_DECODE_CONTEXT      decCtx  = nullptr;
    switch (ctxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_VP:
        case DDI_MEDIA_CONTEXT_TYPE_PROTECTED:
            break;
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
            ctxPtr = DdiMedia_GetCtxFromVABufferID(mediaCtx, buf_id);
            DDI_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);

            decCtx = DdiDecode_GetDecContextFromPVOID(ctxPtr);
            bufMgr = &(decCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            ctxPtr = DdiMedia_GetCtxFromVABufferID(mediaCtx, buf_id);
            DDI_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);

            encCtx = DdiEncode_GetEncContextFromPVOID(ctxPtr);
            DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
            bufMgr = &(encCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_MEDIA:
            break;
        default:
            return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_INFO, &ctxType, sizeof(ctxType), &buf->uiType, sizeof(uint32_t));
    switch ((int32_t)buf->uiType)
    {
        case VASliceDataBufferType:
        case VAProtectedSliceDataBufferType:
        case VABitPlaneBufferType:
            *pbuf = (void *)(buf->pData + buf->uiOffset);
            break;

        case VASliceParameterBufferType:
            ctxPtr = DdiMedia_GetCtxFromVABufferID(mediaCtx, buf_id);
            DDI_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);

            decCtx = DdiDecode_GetDecContextFromPVOID(ctxPtr);
            bufMgr = &(decCtx->BufMgr);
            switch (decCtx->wMode)
            {
                case CODECHAL_DECODE_MODE_AVCVLD:
                    if(decCtx->bShortFormatInUse)
                        *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264Base) + buf->uiOffset);
                    else
                        *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264) + buf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_MPEG2VLD:
                    *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2) + buf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_VC1VLD:
                    *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1) + buf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_JPEG:
                    *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG) + buf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_VP8VLD:
                    *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8) + buf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_HEVCVLD:
                    if(decCtx->bShortFormatInUse)
                        *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC) + buf->uiOffset);
                    else
                    {
                        if(!decCtx->m_ddiDecode->IsRextProfile())
                           *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC) + buf->uiOffset);
                        else
                           *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext) + buf->uiOffset);
                     }
                    break;
                case CODECHAL_DECODE_MODE_VP9VLD:
                    *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9) + buf->uiOffset);
                    break;
                case CODECHAL_DECODE_MODE_AV1VLD:
                    *pbuf = (void *)((uint8_t*)(bufMgr->pCodecSlcParamReserved) + buf->uiOffset);
                    break;
                default:
                    break;
            }
            break;

        case VAEncCodedBufferType:
            DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

            if( DdiEncode_CodedBufferExistInStatusReport( encCtx, buf ) )
            {
                vaStatus = DdiEncode_StatusReport(encCtx, buf, pbuf);
                MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                return vaStatus;
            }
            // so far a coded buffer that has NOT been added into status report is skipped frame in non-CP case
            // but this can change in future if new usage models come up
            encCtx->BufMgr.pCodedBufferSegment->buf  = DdiMediaUtil_LockBuffer(buf, flag);
            encCtx->BufMgr.pCodedBufferSegment->size = buf->iSize;
            *pbuf =  encCtx->BufMgr.pCodedBufferSegment;

            break;

        case VAStatsStatisticsBufferType:
        case VAStatsStatisticsBottomFieldBufferType:
        case VAStatsMVBufferType:
            {
                DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT)
                DDI_ENCODE_PRE_ENC_BUFFER_TYPE idx = (buf->uiType == VAStatsMVBufferType) ? PRE_ENC_BUFFER_TYPE_MVDATA :
                                                    ((buf->uiType == VAStatsStatisticsBufferType)   ? PRE_ENC_BUFFER_TYPE_STATS
                                                                                                          : PRE_ENC_BUFFER_TYPE_STATS_BOT);
                if((encCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC) && DdiEncode_PreEncBufferExistInStatusReport( encCtx, buf, idx))
                {
                    vaStatus = DdiEncode_PreEncStatusReport(encCtx, buf, pbuf);
                    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                    return vaStatus;
                }
                if(buf->bo)
                {
                    *pbuf = DdiMediaUtil_LockBuffer(buf, flag);
                }
                break;
            }
        case VAStatsMVPredictorBufferType:
        case VAEncFEIMBControlBufferType:
        case VAEncFEIMVPredictorBufferType:
        case VAEncQPBufferType:
            if(buf->bo)
            {
                *pbuf = DdiMediaUtil_LockBuffer(buf, flag);
            }
            break;
        case VADecodeStreamoutBufferType:
            if(buf->bo)
            {
                 uint32_t timeout_NS = 100000000;
                 while (0 != mos_gem_bo_wait(buf->bo, timeout_NS))
                 {
                     // Just loop while gem_bo_wait times-out.
                 }
                 *pbuf = DdiMediaUtil_LockBuffer(buf, flag);
            }
            break;
        case VAEncFEIMVBufferType:
        case VAEncFEIMBCodeBufferType:
        case VAEncFEICURecordBufferType:
        case VAEncFEICTBCmdBufferType:
        case VAEncFEIDistortionBufferType:
            {
                DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT)
                if(encCtx->wModeType == CODECHAL_ENCODE_MODE_AVC)
                {
                    CodecEncodeAvcFeiPicParams *feiPicParams = (CodecEncodeAvcFeiPicParams *)encCtx->pFeiPicParams;

                    DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (buf->uiType == VAEncFEIMVBufferType) ? FEI_ENC_BUFFER_TYPE_MVDATA :
                                       ((buf->uiType == VAEncFEIMBCodeBufferType) ? FEI_ENC_BUFFER_TYPE_MBCODE :
                                                                                        FEI_ENC_BUFFER_TYPE_DISTORTION);
                    if((feiPicParams != nullptr) && (encCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC) && DdiEncode_EncBufferExistInStatusReport( encCtx, buf, idx))
                    {
                        vaStatus = DdiEncode_EncStatusReport(encCtx, buf, pbuf);
                        MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                        return vaStatus;
                    }
                }
                else if(encCtx->wModeType == CODECHAL_ENCODE_MODE_HEVC)

                {
                    CodecEncodeHevcFeiPicParams *feiPicParams = (CodecEncodeHevcFeiPicParams *)encCtx->pFeiPicParams;
                    DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (buf->uiType == VAEncFEICTBCmdBufferType) ? FEI_ENC_BUFFER_TYPE_MVDATA   :
                                                      ((buf->uiType == VAEncFEICURecordBufferType) ? FEI_ENC_BUFFER_TYPE_MBCODE :
                                                                                                      FEI_ENC_BUFFER_TYPE_DISTORTION);
                    if((feiPicParams != nullptr) && (encCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC) && DdiEncode_EncBufferExistInStatusReport( encCtx, buf, idx))
                    {
                        vaStatus = DdiEncode_EncStatusReport(encCtx, buf, pbuf);
                        MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                        return vaStatus;
                    }
                }
                if(buf->bo)
                {
                    *pbuf = DdiMediaUtil_LockBuffer(buf, flag);
                }
            }
            break;
        case VAStatsStatisticsParameterBufferType:
            *pbuf = (void *)(buf->pData + buf->uiOffset);
            break;
        case VAEncMacroblockMapBufferType:
            DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
            *pbuf = DdiMediaUtil_LockBuffer(buf, flag);
            DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
            MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
            if (nullptr == (*pbuf))
            {
                return VA_STATUS_ERROR_OPERATION_FAILED;
            }
            else
            {
                return VA_STATUS_SUCCESS;
            }
            break;

        case VAProbabilityBufferType:
            *pbuf = (void *)(buf->pData + buf->uiOffset);

            break;

        case VAEncMacroblockDisableSkipMapBufferType:
            if(buf->bo)
            {
                *pbuf = DdiMediaUtil_LockBuffer(buf, flag);
            }
            break;

        case VAImageBufferType:
        default:
            if((buf->format != Media_Format_CPU) && (DdiMedia_MediaFormatToOsFormat(buf->format) != VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT))
            {
                DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
                // A critical section starts.
                // Make sure not to bailout with a return until the section ends.

                if (nullptr != buf->pSurface && Media_Format_CPU != buf->format)
                {
                    vaStatus = DdiMedia_MediaMemoryDecompress(mediaCtx, buf->pSurface);
                }

                if (VA_STATUS_SUCCESS == vaStatus)
                {
                    *pbuf = DdiMediaUtil_LockBuffer(buf, flag);

                    if (nullptr == *pbuf)
                    {
                        vaStatus = VA_STATUS_ERROR_OPERATION_FAILED;
                    }
                }

                // The critical section ends.
                DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
            }
            else
            {
                *pbuf = (void *)(buf->pData + buf->uiOffset);
            }
            break;
    }

    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return vaStatus;
}

VAStatus DdiMedia_MapBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id,
    void                **pbuf
)
{
    return DdiMedia_MapBufferInternal(ctx, buf_id, pbuf, MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY);
}

VAStatus DdiMedia_UnmapBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id
)
{
    DDI_FUNCTION_ENTER();
    MOS_TraceEventExt(EVENT_VA_UNMAP, EVENT_TYPE_START, &buf_id, sizeof(buf_id), nullptr, 0);

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL( mediaCtx->pBufferHeap, "nullptr  mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)buf_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid buf_id", VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_MEDIA_BUFFER   *buf     = DdiMedia_GetBufferFromVABufferID(mediaCtx,  buf_id);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    // The context is nullptr when the buffer is created from DdiMedia_DeriveImage
    // So doesn't need to check the context for all cases
    // Only check the context in dec/enc mode
    void     *ctxPtr = nullptr;
    uint32_t ctxType = DdiMedia_GetCtxTypeFromVABufferID(mediaCtx, buf_id);
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = nullptr;
    PDDI_DECODE_CONTEXT decCtx = nullptr;
    PDDI_ENCODE_CONTEXT encCtx = nullptr;
    switch (ctxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_VP:
        case DDI_MEDIA_CONTEXT_TYPE_PROTECTED:
            break;
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
            ctxPtr = DdiMedia_GetCtxFromVABufferID(mediaCtx, buf_id);
            DDI_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);

            decCtx = DdiDecode_GetDecContextFromPVOID(ctxPtr);
            bufMgr = &(decCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            ctxPtr = DdiMedia_GetCtxFromVABufferID(mediaCtx, buf_id);
            DDI_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);

            encCtx = DdiEncode_GetEncContextFromPVOID(ctxPtr);
            bufMgr = &(encCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_MEDIA:
            break;
        default:
            return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    switch ((int32_t)buf->uiType)
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
            if(buf->bo)
            {
                DdiMediaUtil_UnlockBuffer(buf);
            }
            break;
        case VAEncMacroblockDisableSkipMapBufferType:
            if(buf->bo)
            {
                DdiMediaUtil_UnlockBuffer(buf);
            }
            break;

        case VAImageBufferType:
        default:
            if((buf->format != Media_Format_CPU) &&(DdiMedia_MediaFormatToOsFormat(buf->format) != VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT))
            {
                DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
                DdiMediaUtil_UnlockBuffer(buf);
                DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
            }
            break;
    }

    MOS_TraceEventExt(EVENT_VA_UNMAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_DestroyBuffer (
    VADriverContextP    ctx,
    VABufferID          buffer_id
)
{
    DDI_FUNCTION_ENTER();
    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_START, &buffer_id, sizeof(buffer_id), nullptr, 0);

    DDI_CHK_NULL(ctx,                    "nullptr ctx",                    VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,              "nullptr mediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)buffer_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufferId", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_BUFFER   *buf     = DdiMedia_GetBufferFromVABufferID(mediaCtx,  buffer_id);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    void     *ctxPtr = DdiMedia_GetCtxFromVABufferID(mediaCtx,     buffer_id);
    uint32_t ctxType = DdiMedia_GetCtxTypeFromVABufferID(mediaCtx, buffer_id);

    DDI_CODEC_COM_BUFFER_MGR     *bufMgr  = nullptr;
    PDDI_ENCODE_CONTEXT           encCtx  = nullptr;
    PDDI_DECODE_CONTEXT           decCtx  = nullptr;
    switch (ctxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
            DDI_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);
            decCtx = DdiDecode_GetDecContextFromPVOID(ctxPtr);
            bufMgr = &(decCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            DDI_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);
            encCtx = DdiEncode_GetEncContextFromPVOID(ctxPtr);
            bufMgr = &(encCtx->BufMgr);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            break;
        case DDI_MEDIA_CONTEXT_TYPE_MEDIA:
            break;
        case DDI_MEDIA_CONTEXT_TYPE_PROTECTED:
            break;
        default:
            return VA_STATUS_ERROR_INVALID_BUFFER;
    }
    switch ((int32_t)buf->uiType)
    {
        case VASliceDataBufferType:
        case VAProtectedSliceDataBufferType:
            DdiMedia_ReleaseBsBuffer(bufMgr, buf);
            break;
        case VABitPlaneBufferType:
            DdiMedia_ReleaseBpBuffer(bufMgr, buf);
            break;
        case VAProbabilityBufferType:
            DdiMedia_ReleaseBpBuffer(bufMgr, buf);
            break;

        case VASliceParameterBufferType:
            DdiMedia_ReleaseSliceControlBuffer(ctxType, ctxPtr, buf);
            break;
        case VAPictureParameterBufferType:
            break;
        case VAImageBufferType:
            if(buf->format == Media_Format_CPU)
            {
                MOS_FreeMemory(buf->pData);
            }
            else
            {
                DdiMediaUtil_UnRefBufObjInMediaBuffer(buf);

                if (buf->uiExportcount) {
                    buf->bPostponedBufFree = true;
                    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                    return VA_STATUS_SUCCESS;
                }
            }
            break;
        case VAProcPipelineParameterBufferType:
        case VAProcFilterParameterBufferType:
            MOS_FreeMemory(buf->pData);
            break;
        case VASubsetsParameterBufferType:
        case VAIQMatrixBufferType:
        case VAHuffmanTableBufferType:
        case VAEncSliceParameterBufferType:
        case VAEncPictureParameterBufferType:
        case VAEncSequenceParameterBufferType:
        case VAEncPackedHeaderDataBufferType:
        case VAEncPackedHeaderParameterBufferType:
            MOS_FreeMemory(buf->pData);
            break;
        case VAEncMacroblockMapBufferType:
            DdiMediaUtil_FreeBuffer(buf);
            break;
#ifdef ENABLE_ENC_UNLIMITED_OUTPUT
        case VAEncCodedBufferType:
            if(nullptr == encCtx)
            {
                encCtx = DdiEncode_GetEncContextFromPVOID(ctxPtr);
                if(nullptr == encCtx)
                    return VA_STATUS_ERROR_INVALID_CONTEXT;
            }
            DdiMediaUtil_FreeBuffer(buf);
            break;
#endif
        case VAStatsStatisticsParameterBufferType:
            MOS_FreeMemory(buf->pData);
            break;
        case VAStatsStatisticsBufferType:
        case VAStatsStatisticsBottomFieldBufferType:
        case VAStatsMVBufferType:
            {
                if(nullptr == encCtx)
                {
                    encCtx = DdiEncode_GetEncContextFromPVOID(ctxPtr);
                    if(nullptr == encCtx)
                        return VA_STATUS_ERROR_INVALID_CONTEXT;
                }
                if(encCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
                {
                    DDI_ENCODE_PRE_ENC_BUFFER_TYPE idx = (buf->uiType == VAStatsMVBufferType) ? PRE_ENC_BUFFER_TYPE_MVDATA :
                                                        ((buf->uiType == VAStatsStatisticsBufferType)   ? PRE_ENC_BUFFER_TYPE_STATS
                                                                                                              : PRE_ENC_BUFFER_TYPE_STATS_BOT);
                    DdiEncode_RemoveFromPreEncStatusReportQueue(encCtx, buf, idx);
                }
            }
            DdiMediaUtil_FreeBuffer(buf);
            break;
        case VAStatsMVPredictorBufferType:
        case VAEncFEIMBControlBufferType:
        case VAEncFEIMVPredictorBufferType:
        case VAEncQPBufferType:
        case VADecodeStreamoutBufferType:
            DdiMediaUtil_FreeBuffer(buf);
            break;
        case VAEncFEIMVBufferType:
        case VAEncFEIMBCodeBufferType:
        case VAEncFEICURecordBufferType:
        case VAEncFEICTBCmdBufferType:
        case VAEncFEIDistortionBufferType:
            {
                if(nullptr == encCtx)
                {
                    encCtx = DdiEncode_GetEncContextFromPVOID(ctxPtr);
                    if(nullptr == encCtx)
                        return VA_STATUS_ERROR_INVALID_CONTEXT;
                }
                if(encCtx->wModeType == CODECHAL_ENCODE_MODE_AVC)
                {
                    CodecEncodeAvcFeiPicParams *feiPicParams;
                    feiPicParams = (CodecEncodeAvcFeiPicParams *)(encCtx->pFeiPicParams);
                    if((feiPicParams != nullptr) && (encCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC))
                    {
                        DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (buf->uiType == VAEncFEIMVBufferType) ? FEI_ENC_BUFFER_TYPE_MVDATA :
                                                        ((buf->uiType == VAEncFEIMBCodeBufferType) ? FEI_ENC_BUFFER_TYPE_MBCODE :
                                                                                                      FEI_ENC_BUFFER_TYPE_DISTORTION);
                        DdiEncode_RemoveFromEncStatusReportQueue(encCtx, buf, idx);
                    }

                }
                else if(encCtx->wModeType == CODECHAL_ENCODE_MODE_HEVC)
                {
                    CodecEncodeHevcFeiPicParams *feiPicParams;
                    feiPicParams = (CodecEncodeHevcFeiPicParams *)(encCtx->pFeiPicParams);
                    if((feiPicParams != nullptr) && (encCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC))
                    {
                        DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (buf->uiType == VAEncFEICTBCmdBufferType) ? FEI_ENC_BUFFER_TYPE_MVDATA   :
                                                          ((buf->uiType == VAEncFEICURecordBufferType) ? FEI_ENC_BUFFER_TYPE_MBCODE :
                                                                                                          FEI_ENC_BUFFER_TYPE_DISTORTION);
                        DdiEncode_RemoveFromEncStatusReportQueue(encCtx, buf, idx);
                    }
                }
            }
            DdiMediaUtil_FreeBuffer(buf);
            break;
        default: // do not handle any un-listed buffer type
            MOS_FreeMemory(buf->pData);
            break;
            //return va_STATUS_SUCCESS;
    }
    MOS_FreeMemory(buf);

    DdiMedia_DestroyBufFromVABufferID(mediaCtx, buffer_id);
    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         render_target
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx   = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)render_target, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "render_target", VA_STATUS_ERROR_INVALID_SURFACE);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    uint32_t event[] = {(uint32_t)context, ctxType, (uint32_t)render_target};
    MOS_TraceEventExt(EVENT_VA_PICTURE, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    PDDI_MEDIA_SURFACE surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, render_target);
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);

    DdiMediaUtil_LockMutex(&mediaCtx->SurfaceMutex);
    surface->curCtxType = ctxType;
    surface->curStatusReportQueryState = DDI_MEDIA_STATUS_REPORT_QUERY_STATE_PENDING;
    if(ctxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        surface->curStatusReport.vpp.status = VPREP_NOTAVAILABLE;
    }
    DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);

    switch (ctxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
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

VAStatus DdiMedia_RenderPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID         *buffers,
    int32_t             num_buffers
)
{

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(  ctx,            "nullptr ctx",                   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(  buffers,        "nullptr buffers",               VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(num_buffers, 0, "Invalid number buffers",     VA_STATUS_ERROR_INVALID_PARAMETER);
    uint32_t event[] = {(uint32_t)context, (uint32_t)num_buffers};
    MOS_TraceEventExt(EVENT_VA_PICTURE, EVENT_TYPE_INFO, event, sizeof(event), buffers, num_buffers*sizeof(VAGenericID));

    PDDI_MEDIA_CONTEXT mediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,              "nullptr mediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    for(int32_t i = 0; i < num_buffers; i++)
    {
       DDI_CHK_LESS((uint32_t)buffers[i], mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid Buffer", VA_STATUS_ERROR_INVALID_BUFFER);
    }

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);

    switch (ctxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
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

VAStatus DdiMedia_EndPicture (
    VADriverContextP    ctx,
    VAContextID         context
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void     *ctxPtr = DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    VAStatus  vaStatus = VA_STATUS_SUCCESS;
    switch (ctxType)
    {
        case DDI_MEDIA_CONTEXT_TYPE_DECODER:
            vaStatus = DdiDecode_EndPicture(ctx, context);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_ENCODER:
            vaStatus = DdiEncode_EndPicture(ctx, context);
            break;
        case DDI_MEDIA_CONTEXT_TYPE_VP:
            vaStatus = DdiVp_EndPicture(ctx, context);
            break;
        default:
            DDI_ASSERTMESSAGE("DDI: unsupported context in DdiCodec_EndPicture.");
            vaStatus = VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    MOS_TraceEventExt(EVENT_VA_PICTURE, EVENT_TYPE_END, &context, sizeof(context), &vaStatus, sizeof(vaStatus));
    PERF_UTILITY_STOP_ONCE("First Frame Time", PERF_MOS, PERF_LEVEL_DDI);

    return vaStatus;
}

static VAStatus DdiMedia_StatusCheck (
    PDDI_MEDIA_CONTEXT mediaCtx,
    DDI_MEDIA_SURFACE  *surface,
    VASurfaceID surface_id
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(surface,  "nullptr surface",  VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t i = 0;
    PDDI_DECODE_CONTEXT decCtx = (PDDI_DECODE_CONTEXT)surface->pDecCtx;
    if (decCtx && surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_DECODER)
    {
        DdiMediaUtil_LockGuard guard(&mediaCtx->SurfaceMutex);

        Codechal *codecHal = decCtx->pCodecHal;
        //return success just avoid vaDestroyContext is ahead of vaSyncSurface
        DDI_CHK_NULL(codecHal, "nullptr decCtx->pCodecHal", VA_STATUS_SUCCESS);

        //return success just avoid vaDestroyContext is ahead of vaSyncSurface
        if (codecHal->IsApogeiosEnabled())
        {
            DecodePipelineAdapter *decoder = dynamic_cast<DecodePipelineAdapter *>(codecHal);
            DDI_CHK_NULL(decoder, "nullptr (DecodePipelineAdapter *decoder) ", VA_STATUS_SUCCESS);
            return DdiDecode_StatusReport(mediaCtx, decoder, surface);
        }
        else
        {
            CodechalDecode *decoder = dynamic_cast<CodechalDecode *>(codecHal);
            DDI_CHK_NULL(decoder, "nullptr (CodechalDecode *decoder)", VA_STATUS_SUCCESS);
            return DdiDecode_StatusReport(mediaCtx, decoder, surface);
        }
    }

    if (surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP)
    {
        PDDI_VP_CONTEXT vpCtx = (PDDI_VP_CONTEXT)surface->pVpCtx;
        DDI_CHK_NULL(vpCtx ,        "nullptr vpCtx",         VA_STATUS_ERROR_INVALID_CONTEXT);
        DDI_CHK_NULL(vpCtx->pVpHal ,"nullptr vpCtx->pVpHal", VA_STATUS_ERROR_INVALID_CONTEXT);

        QUERY_STATUS_REPORT_APP tempVpReport;
        MOS_ZeroMemory(&tempVpReport, sizeof(QUERY_STATUS_REPORT_APP));

        // Get reported surfaces' count
        uint32_t tableLen = 0;
        vpCtx->pVpHal->GetStatusReportEntryLength(&tableLen);

        if (tableLen > 0 && surface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUERY_STATE_PENDING)
        {
            // Query the status for all of surfaces which have finished
            for(i = 0; i < tableLen; i++)
            {
                MOS_ZeroMemory(&tempVpReport, sizeof(QUERY_STATUS_REPORT_APP));
                vpCtx->pVpHal->GetStatusReport(&tempVpReport, 1);

                // StatusFeedBackID is last time submitted Target Surface ID which is set in BeginPicture,
                // So we can know the report is for which surface here.
                DDI_MEDIA_SURFACE *tempSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, tempVpReport.StatusFeedBackID);
                if(tempSurface == nullptr)
                {
                    return VA_STATUS_ERROR_OPERATION_FAILED;
                }

                // Update the status of the surface which is reported.
                tempSurface->curStatusReport.vpp.status = (uint32_t)tempVpReport.dwStatus;
                tempSurface->curStatusReportQueryState  = DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED;

                if(tempVpReport.StatusFeedBackID == surface_id)
                {
                    break;
                }
            }
        }

        if (surface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED)
        {
            if(surface->curStatusReport.vpp.status == VPREP_OK)
            {
                return VA_STATUS_SUCCESS;
            }
            else if(surface->curStatusReport.vpp.status == VPREP_NOTREADY)
            {
                return mediaCtx->bMediaResetEnable ? VA_STATUS_SUCCESS : VA_STATUS_ERROR_HW_BUSY;
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

VAStatus DdiMedia_SyncSurface (
    VADriverContextP    ctx,
    VASurfaceID         render_target
)
{
    PERF_UTILITY_AUTO(__FUNCTION__, "ENCODE", "DDI");

    DDI_FUNCTION_ENTER();
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_START, &render_target, sizeof(VAGenericID), nullptr, 0);

    DDI_CHK_NULL(ctx,    "nullptr ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)render_target, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid render_target", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE  *surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, render_target);
    DDI_CHK_NULL(surface,    "nullptr surface",      VA_STATUS_ERROR_INVALID_CONTEXT);
    if (surface->pCurrentFrameSemaphore)
    {
        DdiMediaUtil_WaitSemaphore(surface->pCurrentFrameSemaphore);
        DdiMediaUtil_PostSemaphore(surface->pCurrentFrameSemaphore);
    }

    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_INFO, surface->bo? &surface->bo->handle:nullptr, sizeof(uint32_t), nullptr, 0);
    // check the bo here?
    // zero is a expected return value
    uint32_t timeout_NS = 100000000;
    while (0 != mos_gem_bo_wait(surface->bo, timeout_NS))
    {
        // Just loop while gem_bo_wait times-out.
    }

    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return DdiMedia_StatusCheck(mediaCtx, surface, render_target);
}

#if VA_CHECK_VERSION(1, 9, 0)
VAStatus DdiMedia_SyncSurface2 (
    VADriverContextP    ctx,
    VASurfaceID         surface_id,
    uint64_t            timeout_ns
)
{
    PERF_UTILITY_AUTO(__FUNCTION__, "ENCODE", "DDI");

    DDI_FUNCTION_ENTER();
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_START, &surface_id, sizeof(VAGenericID), nullptr, 0);

    DDI_CHK_NULL(ctx,    "nullptr ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)surface_id, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid render_target", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE  *surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface_id);
    DDI_CHK_NULL(surface,    "nullptr surface",      VA_STATUS_ERROR_INVALID_CONTEXT);
    if (surface->pCurrentFrameSemaphore)
    {
        DdiMediaUtil_WaitSemaphore(surface->pCurrentFrameSemaphore);
        DdiMediaUtil_PostSemaphore(surface->pCurrentFrameSemaphore);
    }
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_INFO, surface->bo? &surface->bo->handle:nullptr, sizeof(uint32_t), nullptr, 0);

    if (timeout_ns == VA_TIMEOUT_INFINITE)
    {
        // zero is an expected return value when not hit timeout
        auto ret = mos_gem_bo_wait(surface->bo, DDI_BO_INFINITE_TIMEOUT);
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
        if (timeout_ns >= DDI_BO_MAX_TIMEOUT)
        {
            timeoutBoWait1 = DDI_BO_MAX_TIMEOUT - 1;
            timeoutBoWait2 = timeout_ns - DDI_BO_MAX_TIMEOUT + 1;
        }
        else
        {
            timeoutBoWait1 = (int64_t)timeout_ns;
        }
        
        // zero is an expected return value when not hit timeout
        auto ret = mos_gem_bo_wait(surface->bo, timeoutBoWait1);
        if (0 != ret)
        {
            if (timeoutBoWait2)
            {
                ret = mos_gem_bo_wait(surface->bo, timeoutBoWait2); 
            }
            if (0 != ret)
            {
                DDI_NORMALMESSAGE("vaSyncSurface2: surface is still used by HW\n\r");
                return VA_STATUS_ERROR_TIMEDOUT;
            }
        }
    }
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return DdiMedia_StatusCheck(mediaCtx, surface, surface_id);
}

VAStatus DdiMedia_SyncBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id,
    uint64_t            timeout_ns
)
{
    PERF_UTILITY_AUTO(__FUNCTION__, "ENCODE", "DDI");

    DDI_FUNCTION_ENTER();
    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_START, &buf_id, sizeof(buf_id), nullptr, 0);

    DDI_CHK_NULL(ctx,    "nullptr ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pBufferHeap,  "nullptr mediaCtx->pBufferHeap",  VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)buf_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid buffer", VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_MEDIA_BUFFER  *buffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, buf_id);
    DDI_CHK_NULL(buffer,    "nullptr buffer",      VA_STATUS_ERROR_INVALID_CONTEXT);

    MOS_TraceEventExt(EVENT_VA_SYNC, EVENT_TYPE_INFO, buffer->bo? &buffer->bo->handle:nullptr, sizeof(uint32_t), nullptr, 0);
    if (timeout_ns == VA_TIMEOUT_INFINITE)
    {
        // zero is a expected return value when not hit timeout
        auto ret = mos_gem_bo_wait(buffer->bo, DDI_BO_INFINITE_TIMEOUT);
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
        if (timeout_ns >= DDI_BO_MAX_TIMEOUT)
        {
            timeoutBoWait1 = DDI_BO_MAX_TIMEOUT - 1;
            timeoutBoWait2 = timeout_ns - DDI_BO_MAX_TIMEOUT + 1;
        }
        else
        {
            timeoutBoWait1 = (int64_t)timeout_ns;
        }

        // zero is a expected return value when not hit timeout
        auto ret = mos_gem_bo_wait(buffer->bo, timeoutBoWait1);
        if (0 != ret)
        {
            if (timeoutBoWait2)
            {
                ret = mos_gem_bo_wait(buffer->bo, timeoutBoWait2);
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

VAStatus DdiMedia_QuerySurfaceStatus (
    VADriverContextP    ctx,
    VASurfaceID         render_target,
    VASurfaceStatus    *status
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,    "nullptr ctx",    VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(status, "nullptr status", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,                  "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap,    "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)render_target, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid render_target", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_MEDIA_SURFACE *surface   = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, render_target);
    DDI_CHK_NULL(surface,    "nullptr surface",    VA_STATUS_ERROR_INVALID_SURFACE);

    if (surface->pCurrentFrameSemaphore)
    {
        if(DdiMediaUtil_TryWaitSemaphore(surface->pCurrentFrameSemaphore) == 0)
        {
            DdiMediaUtil_PostSemaphore(surface->pCurrentFrameSemaphore);
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

VAStatus DdiMedia_QuerySurfaceError(
    VADriverContextP ctx,
    VASurfaceID      render_target,
    VAStatus         error_status,
    void             **error_info /*out*/
)
{
    DDI_UNUSED(error_status);

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL( ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT );

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL( mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_SURFACE *surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, render_target);
    DDI_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);

    PDDI_DECODE_CONTEXT decCtx = (PDDI_DECODE_CONTEXT)surface->pDecCtx;
    DDI_CHK_NULL( decCtx, "nullptr surface->pDecCtx", VA_STATUS_ERROR_INVALID_CONTEXT );

    VASurfaceDecodeMBErrors *surfaceErrors   = decCtx->vaSurfDecErrOutput;
    DDI_CHK_NULL(surfaceErrors , "nullptr surfaceErrors", VA_STATUS_ERROR_INVALID_CONTEXT );

    VAStatus vaStatus = VA_STATUS_SUCCESS;

    DdiMediaUtil_LockMutex(&mediaCtx->SurfaceMutex);
    if (surface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED)
    {
        if (error_status != -1 && surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_DECODER &&
            surface->curStatusReport.decode.status == CODECHAL_STATUS_ERROR)
        {
            surfaceErrors[1].status            = -1;
            surfaceErrors[0].status            = 2;
            surfaceErrors[0].start_mb          = 0;
            surfaceErrors[0].end_mb            = 0;
            surfaceErrors[0].num_mb            = surface->curStatusReport.decode.errMbNum;
            surfaceErrors[0].decode_error_type = VADecodeMBError;
            *error_info = surfaceErrors;
            DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);
            return VA_STATUS_SUCCESS;
        }

        if (error_status == -1 && surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_DECODER)
            //&& surface->curStatusReport.decode.status == CODECHAL_STATUS_SUCCESSFUL)  // get the crc value whatever the status is
        {
            CodechalDecode *decoder = dynamic_cast<CodechalDecode *>(decCtx->pCodecHal);

            if (nullptr == decoder)
            {
                DDI_ASSERTMESSAGE("nullptr codechal decoder");
                vaStatus = VA_STATUS_ERROR_INVALID_CONTEXT;
            }
            else
            {
                if (decoder->GetStandard() != CODECHAL_AVC)
                {
                    vaStatus = VA_STATUS_ERROR_UNIMPLEMENTED;
                }
                else
                {
                    *error_info = (void *)&surface->curStatusReport.decode.crcValue;
                }
            }

            DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);
            return vaStatus;
        }

        if (surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP &&
            surface->curStatusReport.vpp.status == CODECHAL_STATUS_ERROR)
        {
            DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);
            return VA_STATUS_SUCCESS;
        }
    }

    surfaceErrors[0].status = -1;
    DdiMediaUtil_UnLockMutex(&mediaCtx->SurfaceMutex);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMedia_QuerySurfaceAttributes(
    VADriverContextP ctx,
    VAConfigID config_id,
    VASurfaceAttrib *attrib_list,
    uint32_t *num_attribs
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,         "nullptr ctx",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(num_attribs, "nullptr num_attribs", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,   "nullptr mediaCtx",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_caps->QuerySurfaceAttributes(config_id,
            attrib_list, num_attribs);
}

VAStatus DdiMedia_PutSurface(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void*            draw,
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
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_PARAMETER);
    if(number_cliprects > 0)
    {
        DDI_CHK_NULL(cliprects, "nullptr cliprects", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    void               *vpCtx        = nullptr;
    PDDI_MEDIA_CONTEXT mediaDrvCtx   = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(mediaDrvCtx,               "nullptr mediaDrvCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaDrvCtx->pSurfaceHeap, "nullptr mediaDrvCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)surface, mediaDrvCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    if (nullptr != mediaDrvCtx->pVpCtxHeap->pHeapBase)
    {
        uint32_t ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
        vpCtx = DdiMedia_GetContextFromContextID(ctx, (VAContextID)(0 + DDI_MEDIA_VACONTEXTID_OFFSET_VP), &ctxType);
    }

#if defined(ANDROID) || !defined(X11_FOUND)
       return VA_STATUS_ERROR_UNIMPLEMENTED;
#else
    if(nullptr == vpCtx)
    {
        VAContextID context = VA_INVALID_ID;
        VAStatus vaStatus = DdiVp_CreateContext(ctx, 0, 0, 0, 0, 0, 0, &context);
        DDI_CHK_RET(vaStatus, "Create VP Context failed");
    }
    return DdiCodec_PutSurfaceLinuxHW(ctx, surface, draw, srcx, srcy, srcw, srch, destx, desty, destw, desth, cliprects, number_cliprects, flags);
#endif

}

VAStatus DdiMedia_QueryImageFormats (
    VADriverContextP    ctx,
    VAImageFormat      *format_list,
    int32_t            *num_formats
)
{
    DDI_FUNCTION_ENTER();

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,   "nullptr mediaCtx.",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mediaCtx->m_caps,   "nullptr pointer.",   VA_STATUS_ERROR_INVALID_PARAMETER);
    return mediaCtx->m_caps->QueryImageFormats(format_list, num_formats);
}

VAStatus DdiMedia_CreateImage(
    VADriverContextP ctx,
    VAImageFormat   *format,
    int32_t          width,
    int32_t          height,
    VAImage         *image     /* out */
)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,         "Invalid context!",    VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(format,      "Invalid format!",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(image,       "Invalid image!",      VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(width,  0, "Invalid width!",      VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_LARGER(height, 0, "Invalid height!",     VA_STATUS_ERROR_INVALID_PARAMETER);
    int32_t event[] = {width, height, format->fourcc};
    MOS_TraceEventExt(EVENT_VA_IMAGE, EVENT_TYPE_START, event, sizeof(event), nullptr, 0);

    PDDI_MEDIA_CONTEXT mediaCtx        = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,   "nullptr mediaCtx.",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(mediaCtx->pGmmClientContext, "nullptr mediaCtx->pGmmClientContext.", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAImage *vaimg           = (VAImage*)MOS_AllocAndZeroMemory(sizeof(VAImage));
    DDI_CHK_NULL(vaimg,  "Insufficient to allocate an VAImage.",  VA_STATUS_ERROR_ALLOCATION_FAILED);

    GMM_RESCREATE_PARAMS        gmmParams;
    GMM_RESOURCE_INFO          *gmmResourceInfo;
    MOS_ZeroMemory(&gmmParams, sizeof(gmmParams));

    gmmParams.BaseWidth         = width;
    gmmParams.BaseHeight        = height;
    gmmParams.ArraySize         = 1;
    gmmParams.Type              = RESOURCE_2D;
    gmmParams.Flags.Gpu.Video   = true;
    gmmParams.Format            = mediaCtx->m_caps->ConvertFourccToGmmFmt(format->fourcc);

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
    vaimg->format               = *format;
    vaimg->format.byte_order    = VA_LSB_FIRST;
    vaimg->width                = width;
    vaimg->height               = height;
    vaimg->data_size            = size;

    mediaCtx->pGmmClientContext->DestroyResInfoObject(gmmResourceInfo);

     switch(format->fourcc)
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
            MOS_FreeMemory(vaimg);
            return VA_STATUS_ERROR_UNIMPLEMENTED;
    }

    DDI_MEDIA_BUFFER *buf  = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
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
    VAStatus status= DdiMediaUtil_CreateBuffer(buf,  mediaCtx->pDrmBufMgr);
    if((status != VA_STATUS_SUCCESS))
    {
        MOS_FreeMemory(vaimg);
        MOS_FreeMemory(buf);
        return status;
    }
    buf->TileType     = I915_TILING_NONE;

    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufferHeapElement  = DdiMediaUtil_AllocPMediaBufferFromHeap(mediaCtx->pBufferHeap);

    if (nullptr == bufferHeapElement)
    {
        DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
        MOS_FreeMemory(vaimg);
        DdiMediaUtil_FreeBuffer(buf);
        MOS_FreeMemory(buf);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    bufferHeapElement->pBuffer   = buf;
    bufferHeapElement->pCtx      = nullptr;
    bufferHeapElement->uiCtxType = DDI_MEDIA_CONTEXT_TYPE_MEDIA;

    vaimg->buf                   = bufferHeapElement->uiVaBufferID;
    mediaCtx->uiNumBufs++;
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);

    DdiMediaUtil_LockMutex(&mediaCtx->ImageMutex);
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT imageHeapElement = DdiMediaUtil_AllocPVAImageFromHeap(mediaCtx->pImageHeap);
    if (nullptr == imageHeapElement)
    {
        DdiMediaUtil_UnLockMutex(&mediaCtx->ImageMutex);
        MOS_FreeMemory(vaimg);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    imageHeapElement->pImage     = vaimg;
    mediaCtx->uiNumImages++;
    vaimg->image_id              = imageHeapElement->uiVaImageID;
    DdiMediaUtil_UnLockMutex(&mediaCtx->ImageMutex);

   *image = *vaimg;
    MOS_TraceEventExt(EVENT_VA_IMAGE, EVENT_TYPE_END, &vaimg->image_id, sizeof(VAGenericID), nullptr, 0);
    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Derive image
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] surface
//!         VA surface ID
//! \param  [in] image
//!         VA image
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_DeriveImage (
    VADriverContextP  ctx,
    VASurfaceID       surface,
    VAImage           *image
)
{
    DDI_FUNCTION_ENTER();
    MOS_TraceEventExt(EVENT_VA_DERIVE, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    DDI_CHK_NULL(ctx,   "nullptr ctx",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(image, "nullptr image", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx     = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE *mediaSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface", VA_STATUS_ERROR_INVALID_SURFACE);

    VAImage *vaimg                  = (VAImage*)MOS_AllocAndZeroMemory(sizeof(VAImage));
    DDI_CHK_NULL(vaimg, "nullptr vaimg", VA_STATUS_ERROR_ALLOCATION_FAILED);

    if (mediaSurface->pCurrentFrameSemaphore)
    {
        DdiMediaUtil_WaitSemaphore(mediaSurface->pCurrentFrameSemaphore);
        DdiMediaUtil_PostSemaphore(mediaSurface->pCurrentFrameSemaphore);
    }
    DdiMediaUtil_LockMutex(&mediaCtx->ImageMutex);
    PDDI_MEDIA_IMAGE_HEAP_ELEMENT imageHeapElement = DdiMediaUtil_AllocPVAImageFromHeap(mediaCtx->pImageHeap);
    if (nullptr == imageHeapElement)
    {
        DdiMediaUtil_UnLockMutex(&mediaCtx->ImageMutex);
        MOS_FreeMemory(vaimg);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    imageHeapElement->pImage        = vaimg;
    mediaCtx->uiNumImages++;
    vaimg->image_id                 = imageHeapElement->uiVaImageID;
    DdiMediaUtil_UnLockMutex(&mediaCtx->ImageMutex);

    vaimg->format.fourcc            = DdiMedia_MediaFormatToOsFormat(mediaSurface->format);
    vaimg->width                    = mediaSurface->iWidth;
    vaimg->height                   = mediaSurface->iRealHeight;
    vaimg->format.byte_order        = VA_LSB_FIRST;

    GMM_RESOURCE_INFO          *gmmResourceInfo = mediaSurface->pGmmResourceInfo;
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

    mediaCtx->m_caps->PopulateColorMaskInfo(&vaimg->format);

    DDI_MEDIA_BUFFER *buf               = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    if (buf == nullptr)
    {
        MOS_FreeMemory(vaimg);
        MOS_FreeMemory(buf);
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

    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufferHeapElement = DdiMediaUtil_AllocPMediaBufferFromHeap(mediaCtx->pBufferHeap);

    if (nullptr == bufferHeapElement)
    {
        DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
        MOS_FreeMemory(vaimg);
        MOS_FreeMemory(buf);
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }
    bufferHeapElement->pBuffer    = buf;
    bufferHeapElement->pCtx       = nullptr;
    bufferHeapElement->uiCtxType  = DDI_MEDIA_CONTEXT_TYPE_MEDIA;

    vaimg->buf             = bufferHeapElement->uiVaBufferID;
    mediaCtx->uiNumBufs++;
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);

    *image = *vaimg;

    MOS_TraceEventExt(EVENT_VA_DERIVE, EVENT_TYPE_END, &surface, sizeof(surface), &vaimg->image_id, sizeof(VAGenericID));
    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Free allocated surfaceheap elements
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] image
//!         VA image ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_DestroyImage (
    VADriverContextP ctx,
    VAImageID        image)
{
    DDI_FUNCTION_ENTER();
    MOS_TraceEventExt(EVENT_VA_FREE_IMAGE, EVENT_TYPE_START, &image, sizeof(image), nullptr, 0);

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(mediaCtx,             "nullptr Media",                        VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pImageHeap, "nullptr mediaCtx->pImageHeap",        VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)image, mediaCtx->pImageHeap->uiAllocatedHeapElements, "Invalid image", VA_STATUS_ERROR_INVALID_IMAGE);

    VAImage *vaImage = DdiMedia_GetVAImageFromVAImageID(mediaCtx, image);
    if (vaImage == nullptr)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    DdiMedia_DestroyBuffer(ctx, vaImage->buf);
    MOS_FreeMemory(vaImage);

    DdiMedia_DestroyImageFromVAImageID(mediaCtx, image);

    MOS_TraceEventExt(EVENT_VA_FREE_IMAGE, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Set image palette
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] image
//!         VA image ID
//! \param  [in] palette
//!         Palette
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED if call success, else fail reason
//!
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

VAStatus SwizzleSurface(PDDI_MEDIA_CONTEXT mediaCtx, PGMM_RESOURCE_INFO pGmmResInfo, void *pLockedAddr, uint32_t TileType, uint8_t* pResourceBase, bool bUpload)
{
    uint32_t            uiSize, uiPitch;
    GMM_RES_COPY_BLT    gmmResCopyBlt;
    uint32_t               uiPicHeight;
    uint32_t               ulSwizzledSize;
    VAStatus            vaStatus = VA_STATUS_SUCCESS;

    DDI_CHK_NULL(pGmmResInfo, "pGmmResInfo is NULL", VA_STATUS_ERROR_OPERATION_FAILED);
    DDI_CHK_NULL(pLockedAddr, "pLockedAddr is NULL", VA_STATUS_ERROR_OPERATION_FAILED);
    DDI_CHK_NULL(pResourceBase, "pResourceBase is NULL", VA_STATUS_ERROR_ALLOCATION_FAILED);

    memset(&gmmResCopyBlt, 0x0, sizeof(GMM_RES_COPY_BLT));
    uiPicHeight = pGmmResInfo->GetBaseHeight();
    uiSize = pGmmResInfo->GetSizeSurface();
    uiPitch = pGmmResInfo->GetRenderPitch();
    gmmResCopyBlt.Gpu.pData = pLockedAddr;
    gmmResCopyBlt.Sys.pData = pResourceBase;
    gmmResCopyBlt.Sys.RowPitch = uiPitch;
    gmmResCopyBlt.Sys.BufferSize = uiSize;
    gmmResCopyBlt.Sys.SlicePitch = uiSize;
    gmmResCopyBlt.Blt.Slices = 1;
    gmmResCopyBlt.Blt.Upload = bUpload;

    if(mediaCtx->pGmmClientContext->IsPlanar(pGmmResInfo->GetResourceFormat()) == true)
    {
        gmmResCopyBlt.Blt.Width = pGmmResInfo->GetBaseWidth();
        gmmResCopyBlt.Blt.Height = uiSize/uiPitch;
    }

    pGmmResInfo->CpuBlt(&gmmResCopyBlt);

    return vaStatus;
}

//!
//! \brief  Copy plane from src to dst row by row when src and dst strides are different
//!
//! \param  [in] dst
//!         Destination plane
//! \param  [in] dstPitch
//!         Destination plane pitch
//! \param  [in] src
//!         Source plane
//! \param  [in] srcPitch
//!         Source plane pitch
//! \param  [in] height
//!         Plane hight
//!
static void DdiMedia_CopyPlane(
    uint8_t *dst,
    uint32_t dstPitch,
    uint8_t *src,
    uint32_t srcPitch,
    uint32_t height)
{
    uint32_t rowSize = std::min(dstPitch, srcPitch);
    for (int y = 0; y < height; y += 1)
    {
        memcpy(dst, src, rowSize);
        dst += dstPitch;
        src += srcPitch;
    }
}

//!
//! \brief  Copy data from surface to image
//!
//! \param  [in] ctx
//!         Input driver context
//! \param  [in] surface
//!         Pointer to surface
//! \param  [in] image
//!         Pointer to image
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
static VAStatus DdiMedia_CopySurfaceToImage(
    VADriverContextP  ctx,
    DDI_MEDIA_SURFACE *surface,
    VAImage           *image)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,       "nullptr ctx.",         VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,  "nullptr mediaCtx.",    VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(surface,  "nullptr meida surface.", VA_STATUS_ERROR_INVALID_BUFFER);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    //Lock Surface
    if ((Media_Format_CPU != surface->format))
    {
        vaStatus = DdiMedia_MediaMemoryDecompress(mediaCtx, surface);

        if (vaStatus != VA_STATUS_SUCCESS)
        {
            DDI_NORMALMESSAGE("surface Decompression fail, continue next steps.");
        }
    }
    void *surfData = DdiMediaUtil_LockSurface(surface, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_NO_SWIZZLE));
    if (surfData == nullptr)
    {
        DDI_ASSERTMESSAGE("nullptr surfData.");
        return vaStatus;
    }

    void *imageData = nullptr;
    vaStatus = DdiMedia_MapBuffer(ctx, image->buf, &imageData);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Failed to map buffer.");
        DdiMediaUtil_UnlockSurface(surface);
        return vaStatus;
    }

    uint8_t *ySrc = nullptr;
    uint8_t *yDst = (uint8_t*)imageData;
    uint8_t *swizzleData = (uint8_t*)MOS_AllocMemory(surface->data_size);

    if (!surface->pMediaCtx->bIsAtomSOC && surface->TileType != I915_TILING_NONE)
    {
        SwizzleSurface(surface->pMediaCtx, surface->pGmmResourceInfo, surfData, (MOS_TILE_TYPE)surface->TileType, (uint8_t *)swizzleData, false);
        ySrc = swizzleData;
    }
    else
    {
        ySrc = (uint8_t*)surfData;
    }

    DdiMedia_CopyPlane(yDst, image->pitches[0], ySrc, surface->iPitch, image->height);
    if (image->num_planes > 1)
    {
        uint8_t *uSrc = ySrc + surface->iPitch * surface->iHeight;
        uint8_t *uDst = yDst + image->offsets[1];
        uint32_t chromaPitch       = 0;
        uint32_t chromaHeight      = 0;
        uint32_t imageChromaPitch  = 0;
        uint32_t imageChromaHeight = 0;
        DdiMedia_GetChromaPitchHeight(DdiMedia_MediaFormatToOsFormat(surface->format), surface->iPitch, surface->iHeight, &chromaPitch, &chromaHeight);
        DdiMedia_GetChromaPitchHeight(image->format.fourcc, image->pitches[0], image->height, &imageChromaPitch, &imageChromaHeight);
        DdiMedia_CopyPlane(uDst, image->pitches[1], uSrc, chromaPitch, imageChromaHeight);

        if(image->num_planes > 2)
        {
            uint8_t *vSrc = uSrc + chromaPitch * chromaHeight;
            uint8_t *vDst = yDst + image->offsets[2];
            DdiMedia_CopyPlane(vDst, image->pitches[2], vSrc, chromaPitch, imageChromaHeight);
        }
    }

    MOS_FreeMemory(swizzleData);

    vaStatus = DdiMedia_UnmapBuffer(ctx, image->buf);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Failed to unmap buffer.");
        DdiMediaUtil_UnlockSurface(surface);
        return vaStatus;
    }

    DdiMediaUtil_UnlockSurface(surface);

    return vaStatus;
}

//!
//! \brief  Retrive surface data into a VAImage
//! \details    Image must be in a format supported by the implementation
//!
//! \param  [in] ctx
//!         Input driver context
//! \param  [in] surface
//!         Input surface ID of source
//! \param  [in] x
//!         X offset of the wanted region
//! \param  [in] y
//!         Y offset of the wanted region
//! \param  [in] width
//!         Width of the wanted region
//! \param  [in] height
//!         Height of the wanted region
//! \param  [in] image
//!     The image ID of the source image
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
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
    DDI_FUNCTION_ENTER();
    uint32_t event[] = {surface, x, y, width, height, image};
    MOS_TraceEventExt(EVENT_VA_GET, EVENT_TYPE_START, &event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL(ctx,       "nullptr ctx.",         VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,  "nullptr mediaCtx.",    VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pSurfaceHeap,    "nullptr mediaCtx->pSurfaceHeap.",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pImageHeap,      "nullptr mediaCtx->pImageHeap.",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface.", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_LESS((uint32_t)image,   mediaCtx->pImageHeap->uiAllocatedHeapElements,   "Invalid image.",   VA_STATUS_ERROR_INVALID_IMAGE);

    VAImage *vaimg = DdiMedia_GetVAImageFromVAImageID(mediaCtx, image);
    DDI_CHK_NULL(vaimg,     "nullptr vaimg.",       VA_STATUS_ERROR_INVALID_IMAGE);

    DDI_MEDIA_BUFFER *buf = DdiMedia_GetBufferFromVABufferID(mediaCtx, vaimg->buf);
    DDI_CHK_NULL(buf,       "nullptr buf.",         VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_MEDIA_SURFACE *inputSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(inputSurface,     "nullptr inputSurface.",      VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(inputSurface->bo, "nullptr inputSurface->bo.",  VA_STATUS_ERROR_INVALID_SURFACE);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
#ifndef _FULL_OPEN_SOURCE
    VASurfaceID target_surface = VA_INVALID_SURFACE;
    VASurfaceID output_surface = surface;

    if (inputSurface->format != DdiMedia_OsFormatToMediaFormat(vaimg->format.fourcc, vaimg->format.alpha_mask) ||
        width != vaimg->width || height != vaimg->height ||
        (MEDIA_IS_WA(&mediaCtx->WaTable, WaEnableVPPCopy) &&
        vaimg->format.fourcc != VA_FOURCC_444P &&
        vaimg->format.fourcc != VA_FOURCC_422V &&
        vaimg->format.fourcc != VA_FOURCC_422H))
    {
        VAContextID context = VA_INVALID_ID;
        //Create VP Context.
        vaStatus = DdiVp_CreateContext(ctx, 0, 0, 0, 0, 0, 0, &context);
        DDI_CHK_RET(vaStatus, "Create VP Context failed.");

        //Create target surface for VP pipeline.
        DDI_MEDIA_FORMAT mediaFmt = DdiMedia_OsFormatToMediaFormat(vaimg->format.fourcc, vaimg->format.fourcc);
        if (mediaFmt == Media_Format_Count)
        {
            DDI_ASSERTMESSAGE("Unsupported surface type.");
            DdiVp_DestroyContext(ctx, context);
            return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
        }
        PDDI_MEDIA_SURFACE_DESCRIPTOR surfDesc = (PDDI_MEDIA_SURFACE_DESCRIPTOR)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_SURFACE_DESCRIPTOR));
        if (!surfDesc) {
            DDI_ASSERTMESSAGE("nullptr surfDesc.");
            DdiVp_DestroyContext(ctx, context);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        surfDesc->uiVaMemType = VA_SURFACE_ATTRIB_MEM_TYPE_VA;
        int memType = MOS_MEMPOOL_VIDEOMEMORY;
        if (MEDIA_IS_SKU(&mediaCtx->SkuTable, FtrLocalMemory))
        {
            memType = MOS_MEMPOOL_SYSTEMMEMORY;
        }
        target_surface = (VASurfaceID)DdiMedia_CreateRenderTarget(mediaCtx, mediaFmt, vaimg->width, vaimg->height, surfDesc, VA_SURFACE_ATTRIB_USAGE_HINT_GENERIC, memType);
        if (VA_INVALID_SURFACE == target_surface)
        {
            DDI_ASSERTMESSAGE("Create temp surface failed.");
            DdiVp_DestroyContext(ctx, context);
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
        vaStatus = DdiVp_VideoProcessPipeline(ctx, context, surface, &srcRect, target_surface, &dstRect);
        if (vaStatus != VA_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("VP Pipeline failed.");
            DdiMedia_DestroySurfaces(ctx, &target_surface, 1);
            DdiVp_DestroyContext(ctx, context);
            return vaStatus;
        }
        vaStatus = DdiMedia_SyncSurface(ctx, target_surface);
        vaStatus = DdiVp_DestroyContext(ctx, context);
        output_surface = target_surface;
    }

    //Get Media Surface from output surface ID
    DDI_MEDIA_SURFACE *mediaSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, output_surface);
    DDI_CHK_NULL(mediaSurface,     "nullptr mediaSurface.",      VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->bo, "nullptr mediaSurface->bo.",  VA_STATUS_ERROR_INVALID_SURFACE);

    vaStatus = DdiMedia_CopySurfaceToImage(ctx, mediaSurface, vaimg);

    if (vaStatus != MOS_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Failed to copy surface to image buffer data!");
        if(target_surface != VA_INVALID_SURFACE)
        {
            DdiMedia_DestroySurfaces(ctx, &target_surface, 1);
        }
        return vaStatus;
    }

    //Destroy temp surface if created
    if(target_surface != VA_INVALID_SURFACE)
    {
        DdiMedia_DestroySurfaces(ctx, &target_surface, 1);
    }
#else
    vaStatus = DdiMedia_CopySurfaceToImage(ctx, inputSurface, vaimg);
    DDI_CHK_RET(vaStatus, "Copy surface to image failed.");
#endif
    MOS_TraceEventExt(EVENT_VA_GET, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Copy data from a VAImage to a surface
//! \details    Image must be in a format supported by the implementation
//!
//! \param  [in] ctx
//!         Input driver context
//! \param  [in] surface
//!         Surface ID of destination
//! \param  [in] image
//!         The image ID of the destination image
//! \param  [in] src_x
//!         Source x offset of the image region
//! \param  [in] src_y
//!         Source y offset of the image region
//! \param  [in] src_width
//!         Source width offset of the image region
//! \param  [in] src_height
//!         Source height offset of the image region
//! \param  [in] dest_x
//!         Destination x offset of the surface region
//! \param  [in] dest_y
//!         Destination y offset of the surface region
//! \param  [in] dest_width
//!         Destination width offset of the surface region
//! \param  [in] dest_height
//!         Destination height offset of the surface region
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
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
    DDI_FUNCTION_ENTER();
    uint32_t event[] = {surface, image, src_x, src_y, src_width, src_height, dest_x, dest_y, dest_width, dest_height};
    MOS_TraceEventExt(EVENT_VA_PUT, EVENT_TYPE_START, &event, sizeof(event), nullptr, 0);

    DDI_CHK_NULL(ctx,                    "nullptr ctx.",                     VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx     = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx.",                VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap.",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pImageHeap,   "nullptr mediaCtx->pImageHeap.",     VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface.", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_LESS((uint32_t)image, mediaCtx->pImageHeap->uiAllocatedHeapElements,     "Invalid image.",   VA_STATUS_ERROR_INVALID_IMAGE);

    DDI_MEDIA_SURFACE *mediaSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(mediaSurface,     "nullptr mediaSurface.", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->bo, "Invalid buffer.",       VA_STATUS_ERROR_INVALID_BUFFER);

    if (mediaSurface->pCurrentFrameSemaphore)
    {
        DdiMediaUtil_WaitSemaphore(mediaSurface->pCurrentFrameSemaphore);
        DdiMediaUtil_PostSemaphore(mediaSurface->pCurrentFrameSemaphore);
    }

    VAImage          *vaimg = DdiMedia_GetVAImageFromVAImageID(mediaCtx, image);
    DDI_CHK_NULL(vaimg,      "Invalid image.",      VA_STATUS_ERROR_INVALID_IMAGE);

    DDI_MEDIA_BUFFER *buf   = DdiMedia_GetBufferFromVABufferID(mediaCtx, vaimg->buf);
    DDI_CHK_NULL(buf,       "Invalid buffer.",      VA_STATUS_ERROR_INVALID_BUFFER);

    VAStatus vaStatus = VA_STATUS_SUCCESS;
    void *imageData   = nullptr;

    vaStatus = DdiMedia_MapBuffer(ctx, vaimg->buf, &imageData);
    DDI_CHK_RET(vaStatus, "MapBuffer failed.");
    DDI_CHK_NULL(imageData, "nullptr imageData.", VA_STATUS_ERROR_INVALID_IMAGE);

    // VP Pipeline will be called for CSC/Scaling if the surface format or data size is not consistent with image.
    if (mediaSurface->format != DdiMedia_OsFormatToMediaFormat(vaimg->format.fourcc, vaimg->format.alpha_mask) ||
        dest_width != src_width || dest_height != src_height ||
        src_x != 0 || dest_x != 0 || src_y != 0 || dest_y != 0)
    {
        VAContextID context     = VA_INVALID_ID;

        //Create VP Context.
        vaStatus = DdiVp_CreateContext(ctx, 0, 0, 0, 0, 0, 0, &context);
        DDI_CHK_RET(vaStatus, "Create VP Context failed");

        //Create temp surface for VP pipeline.
        DDI_MEDIA_FORMAT mediaFmt = DdiMedia_OsFormatToMediaFormat(vaimg->format.fourcc, vaimg->format.fourcc);
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
        VASurfaceID tempSurface = (VASurfaceID)DdiMedia_CreateRenderTarget(mediaCtx, mediaFmt, vaimg->width, vaimg->height, nullptr, VA_SURFACE_ATTRIB_USAGE_HINT_VPP_READ, memType);
        if (tempSurface == VA_INVALID_ID)
        {
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }

        DDI_MEDIA_SURFACE *tempMediaSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, tempSurface);
        DDI_CHK_NULL(tempMediaSurface, "nullptr tempMediaSurface.", VA_STATUS_ERROR_INVALID_SURFACE);

        //Lock Surface
        void *tempSurfData = DdiMediaUtil_LockSurface(tempMediaSurface, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));
        if (nullptr == tempSurfData)
        {
            DdiMedia_DestroySurfaces(ctx, &tempSurface, 1);
            return VA_STATUS_ERROR_SURFACE_BUSY;
        }

        //Copy data from image to temp surferce
        MOS_STATUS eStatus = MOS_SecureMemcpy(tempSurfData, vaimg->data_size, imageData, vaimg->data_size);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("Failed to copy image to surface buffer.");
            DdiMediaUtil_UnlockSurface(tempMediaSurface);
            DdiMedia_DestroySurfaces(ctx, &tempSurface, 1);
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }

        vaStatus = DdiMedia_UnmapBuffer(ctx, vaimg->buf);
        if (vaStatus != VA_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("Failed to unmap buffer.");
            DdiMediaUtil_UnlockSurface(tempMediaSurface);
            DdiMedia_DestroySurfaces(ctx, &tempSurface, 1);
            return vaStatus;
        }

        DdiMediaUtil_UnlockSurface(tempMediaSurface);

        VARectangle srcRect, dstRect;
        srcRect.x      = src_x;
        srcRect.y      = src_y;
        srcRect.width  = src_width;
        srcRect.height = src_height;
        dstRect.x      = dest_x;
        dstRect.y      = dest_y;
        dstRect.width  = dest_width;
        dstRect.height = dest_height;

        //Execute VP pipeline.
        vaStatus = DdiVp_VideoProcessPipeline(ctx, context, tempSurface, &srcRect, surface, &dstRect);
        if (vaStatus != VA_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("VP Pipeline failed.");
            DdiMedia_DestroySurfaces(ctx, &tempSurface, 1);
            return vaStatus;
        }

        vaStatus = DdiMedia_SyncSurface(ctx, tempSurface);
        DdiMedia_DestroySurfaces(ctx, &tempSurface, 1);
        vaStatus = DdiVp_DestroyContext(ctx, context);
    }
    else
    {
        //Lock Surface
        if ((nullptr != buf->pSurface) && (Media_Format_CPU != mediaSurface->format))
        {
            vaStatus = DdiMedia_MediaMemoryDecompress(mediaCtx, mediaSurface);

            if (vaStatus != VA_STATUS_SUCCESS)
            {
                DDI_NORMALMESSAGE("surface Decompression fail, continue next steps.");
            }
        }

        void *surfData = DdiMediaUtil_LockSurface(mediaSurface, (MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY));
        if (nullptr == surfData)
        {
            DDI_ASSERTMESSAGE("Failed to lock surface.");
            return VA_STATUS_ERROR_SURFACE_BUSY;
        }

        if (src_width == dest_width && src_height == dest_height &&
            src_width == vaimg->width && src_height == vaimg->height &&
            src_width == mediaSurface->iWidth && src_height == mediaSurface->iHeight &&
            mediaSurface->data_size == vaimg->data_size)
        {
            //Copy data from image to surface
            MOS_STATUS eStatus = MOS_SecureMemcpy(surfData, vaimg->data_size, imageData, vaimg->data_size);
            DDI_CHK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "Failed to copy image to surface buffer.", VA_STATUS_ERROR_OPERATION_FAILED);
        }
        else
        {
            uint8_t *ySrc = (uint8_t *)imageData + vaimg->offsets[0];
            uint8_t *yDst = (uint8_t *)surfData;
            DdiMedia_CopyPlane(yDst, mediaSurface->iPitch, ySrc, vaimg->pitches[0], src_height);

            if (vaimg->num_planes > 1)
            {
                DDI_MEDIA_SURFACE uPlane = *mediaSurface;

                uPlane.iWidth              = src_width;
                uPlane.iRealHeight         = src_height;
                uPlane.iHeight             = src_height;
                uint32_t chromaHeight      = 0;
                uint32_t chromaPitch       = 0;
                DdiMedia_GetChromaPitchHeight(DdiMedia_MediaFormatToOsFormat(uPlane.format), uPlane.iPitch, uPlane.iHeight, &chromaPitch, &chromaHeight);

                uint8_t *uSrc = (uint8_t *)imageData + vaimg->offsets[1];
                uint8_t *uDst = yDst + mediaSurface->iPitch * mediaSurface->iHeight;
                DdiMedia_CopyPlane(uDst, chromaPitch, uSrc, vaimg->pitches[1], chromaHeight);
                if (vaimg->num_planes > 2)
                {
                    uint8_t *vSrc = (uint8_t *)imageData + vaimg->offsets[2];
                    uint8_t *vDst = uDst + chromaPitch * chromaHeight;
                    DdiMedia_CopyPlane(vDst, chromaPitch, vSrc, vaimg->pitches[2], chromaHeight);
                }
            }
        } 

        vaStatus = DdiMedia_UnmapBuffer(ctx, vaimg->buf);
        if (vaStatus != VA_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("Failed to unmap buffer.");
            DdiMediaUtil_UnlockSurface(mediaSurface);
            return vaStatus;
        }

        DdiMediaUtil_UnlockSurface(mediaSurface);
    }
    MOS_TraceEventExt(EVENT_VA_PUT, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Query subpicture formats
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] format_list
//!         VA image format
//! \param  [in] flags
//!         Flags
//! \param  [in] num_formats
//!         Number of formats
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
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

//!
//! \brief  Create subpicture
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] image
//!         VA image ID
//! \param  [out] subpicture
//!         VA subpicture ID
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
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

//!
//! \brief  Destroy subpicture
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
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

//!
//! \brief  Set subpicture image
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//! \param  [in] image
//!         VA image ID
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
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

//!
//! \brief  Set subpicture chrome key
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//! \param  [in] chromakey_min
//!         Minimum chroma key
//! \param  [in] chromakey_max
//!         Maximum chroma key
//! \param  [in] chromakey_mask
//!         Chromakey mask
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
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

//!
//! \brief  set subpicture global alpha
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//! \param  [in] global_alpha
//!         Global alpha
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
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

//!
//! \brief  Associate subpicture
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//! \param  [in] target_surfaces
//!         VA surface ID
//! \param  [in] num_surfaces
//!         Number of surfaces
//! \param  [in] src_x
//!         Source x of the region
//! \param  [in] src_y
//!         Source y of the region
//! \param  [in] src_width
//!         Source width of the region
//! \param  [in] src_height
//!         Source height of the region
//! \param  [in] dest_x
//!         Destination x
//! \param  [in] dest_y
//!         Destination y
//! \param  [in] dest_width
//!         Destination width
//! \param  [in] dest_height
//!         Destination height
//! \param  [in] flags
//!         Flags
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
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

//!
//! \brief  Deassociate subpicture
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//! \param  [in] target_surfaces
//!         VA surface ID
//! \param  [in] num_surfaces
//!         Number of surfaces
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
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

//!
//! \brief  Query display attributes
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] attr_list
//!         VA display attribute
//! \param  [in] num_attributes
//!         Number of attributes
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
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

//!
//! \brief  Get display attributes
//! \details    This function returns the current attribute values in "attr_list".
//!         Only attributes returned with VA_DISPLAY_ATTRIB_GETTABLE set in the "flags" field
//!         from vaQueryDisplayAttributes() can have their values retrieved.
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] attr_list
//!         VA display attribute
//! \param  [in] num_attributes
//!         Number of attributes
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
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

//!
//! \brief  Set display attributes
//! \details    Only attributes returned with VA_DISPLAY_ATTRIB_SETTABLE set in the "flags" field
//!         from vaQueryDisplayAttributes() can be set.  If the attribute is not settable or
//!         the value is out of range, the function returns VA_STATUS_ERROR_ATTR_NOT_SUPPORTED
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] attr_list
//!         VA display attribute
//! \param  [in] num_attributes
//!         Number of attributes
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
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

//!
//! \brief  Query processing rate
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] config_id
//!         VA configuration ID
//! \param  [in] proc_buf
//!         VA processing rate parameter
//! \param  [out] processing_rate
//!         Processing rate
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus
DdiMedia_QueryProcessingRate(
      VADriverContextP          ctx,
      VAConfigID                config_id,
      VAProcessingRateParameter *proc_buf,
      uint32_t                  *processing_rate /* output parameter */)
{
    DDI_CHK_NULL(ctx,             "nullptr ctx",             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(proc_buf,        "nullptr proc_buf",        VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(processing_rate, "nullptr processing_rate", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    return mediaCtx->m_caps->QueryProcessingRate(config_id,
            proc_buf, processing_rate);
}

//!
//! \brief  media copy internal
//! 
//! \param  [in] mosCtx
//!         Pointer to mos context
//! \param  [in] src
//!         VA copy mos resource src.
//! \param  [in] dst
//!         VA copy mos resrouce dst.
//! \param  [in] option
//!         VA copy option, copy mode.
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus
DdiMedia_CopyInternal(
    PMOS_CONTEXT            mosCtx,
    PMOS_RESOURCE           src,
    PMOS_RESOURCE           dst,
    uint32_t                copy_mode
)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    MOS_STATUS mosStatus = MOS_STATUS_UNINITIALIZED;
    DDI_CHK_NULL(mosCtx, "nullptr mosCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(src, "nullptr input osResource", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(dst, "nullptr output osResource", VA_STATUS_ERROR_INVALID_SURFACE);

    MediaCopyBaseState *mediaCopyState = static_cast<MediaCopyBaseState*>(*mosCtx->ppMediaCopyState);

    if (!mediaCopyState)
    {
        mediaCopyState = static_cast<MediaCopyBaseState*>(McpyDevice::CreateFactory(mosCtx));
        *mosCtx->ppMediaCopyState = mediaCopyState;
    }

    DDI_CHK_NULL(mediaCopyState, "Invalid mediaCopy State", VA_STATUS_ERROR_INVALID_PARAMETER);

    mosStatus = mediaCopyState->SurfaceCopy(src, dst, (MCPY_METHOD)copy_mode);
    if (mosStatus != MOS_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    return vaStatus;
}

#if VA_CHECK_VERSION(1,10,0)
//!
//! \brief  media copy
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] dst_obj
//!         VA copy object dst.
//! \param  [in] src_obj
//!         VA copy object src.
//! \param  [in] option
//!         VA copy option, copy mode.
//! \param  [in] sync_handle
//!         VA copy sync handle
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus
DdiMedia_Copy(
    VADriverContextP    ctx,
    VACopyObject       *dst_obj,
    VACopyObject       *src_obj,
    VACopyOption       option
)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    MOS_CONTEXT  mosCtx;
    MOS_RESOURCE src, dst;
    DdiCpInterface *pCpDdiInterface = nullptr;
    PDDI_MEDIA_SURFACE src_surface = nullptr;
    PDDI_MEDIA_SURFACE dst_surface = nullptr;
    PDDI_MEDIA_BUFFER src_buffer = nullptr;
    PDDI_MEDIA_BUFFER dst_buffer = nullptr;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx   = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pBufferHeap,  "nullptr mediaCtx->pBufferHeap",  VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(dst_obj, "nullptr copy dst", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(src_obj, "nullptr copy src", VA_STATUS_ERROR_INVALID_SURFACE);

    if (dst_obj->obj_type == VACopyObjectSurface)
    {
        DDI_CHK_LESS((uint32_t)dst_obj->object.surface_id, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "copy_dst", VA_STATUS_ERROR_INVALID_SURFACE);
        dst_surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, dst_obj->object.surface_id);
        DDI_CHK_NULL(dst_surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
        DDI_CHK_NULL(dst_surface->pGmmResourceInfo, "nullptr dst_surface->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_PARAMETER);

        MOS_ZeroMemory(&dst, sizeof(dst));
        DdiMedia_MediaSurfaceToMosResource(dst_surface, &dst);
    }
    else if (dst_obj->obj_type == VACopyObjectBuffer)
    {
        DDI_CHK_LESS((uint32_t)dst_obj->object.buffer_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid copy dst buf_id", VA_STATUS_ERROR_INVALID_BUFFER);
        dst_buffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, dst_obj->object.buffer_id);
        DDI_CHK_NULL(dst_buffer, "nullptr buffer", VA_STATUS_ERROR_INVALID_BUFFER);
        DDI_CHK_NULL(dst_buffer->pGmmResourceInfo, "nullptr dst_buffer->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_PARAMETER);

        MOS_ZeroMemory(&dst, sizeof(dst));
        DdiMedia_MediaBufferToMosResource(dst_buffer, &dst);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: unsupported src copy object in DdiMedia_copy.");
    }

    if (src_obj->obj_type == VACopyObjectSurface)
    {
        DDI_CHK_LESS((uint32_t)src_obj->object.surface_id, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "copy_src", VA_STATUS_ERROR_INVALID_SURFACE);
        src_surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, src_obj->object.surface_id);
        DDI_CHK_NULL(src_surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);
        DDI_CHK_NULL(src_surface->pGmmResourceInfo, "nullptr src_surface->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_PARAMETER);

        MOS_ZeroMemory(&src, sizeof(src));
        DdiMedia_MediaSurfaceToMosResource(src_surface, &src);
    }
    else if (src_obj->obj_type == VACopyObjectBuffer)
    {
        DDI_CHK_LESS((uint32_t)src_obj->object.buffer_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid copy dst buf_id", VA_STATUS_ERROR_INVALID_BUFFER);
        src_buffer = DdiMedia_GetBufferFromVABufferID(mediaCtx, src_obj->object.buffer_id);
        DDI_CHK_NULL(src_buffer, "nullptr buffer", VA_STATUS_ERROR_INVALID_BUFFER);
        DDI_CHK_NULL(src_buffer->pGmmResourceInfo, "nullptr src_buffer->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_PARAMETER);

        MOS_ZeroMemory(&src, sizeof(src));
        DdiMedia_MediaBufferToMosResource(src_buffer, &src);
    }
    else
    {
        DDI_ASSERTMESSAGE("DDI: unsupported dst copy object in DdiMedia_copy.");
    }

    MOS_ZeroMemory(&mosCtx, sizeof(mosCtx));

    mosCtx.bufmgr          = mediaCtx->pDrmBufMgr;
    mosCtx.m_gpuContextMgr = mediaCtx->m_gpuContextMgr;
    mosCtx.m_cmdBufMgr     = mediaCtx->m_cmdBufMgr;
    mosCtx.fd              = mediaCtx->fd;
    mosCtx.iDeviceId       = mediaCtx->iDeviceId;
    mosCtx.SkuTable        = mediaCtx->SkuTable;
    mosCtx.WaTable         = mediaCtx->WaTable;
    mosCtx.gtSystemInfo    = *mediaCtx->pGtSystemInfo;
    mosCtx.platform        = mediaCtx->platform;

    mosCtx.ppMediaCopyState      = &mediaCtx->pMediaCopyState;
    mosCtx.gtSystemInfo          = *mediaCtx->pGtSystemInfo;
    mosCtx.m_auxTableMgr         = mediaCtx->m_auxTableMgr;
    mosCtx.pGmmClientContext     = mediaCtx->pGmmClientContext;

    mosCtx.m_osDeviceContext     = mediaCtx->m_osDeviceContext;
    mosCtx.m_apoMosEnabled       = mediaCtx->m_apoMosEnabled;

    pCpDdiInterface = Create_DdiCpInterface(mosCtx);

    if (nullptr == pCpDdiInterface)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    vaStatus = DdiMedia_CopyInternal(&mosCtx, &src, &dst, option.bits.va_copy_mode);

    if ((option.bits.va_copy_sync == VA_EXEC_SYNC) && dst_surface)
    {
        uint32_t timeout_NS = 100000000;
        while (0 != mos_gem_bo_wait(dst_surface->bo, timeout_NS))
        {
            // Just loop while gem_bo_wait times-out.
        }
    }

    if (pCpDdiInterface)
    {
        Delete_DdiCpInterface(pCpDdiInterface);
        pCpDdiInterface = NULL;
    }

    return vaStatus;
}
#endif

//!
//! \brief  Check for buffer info
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] buf_id
//!         VA buffer ID
//! \param  [out] type
//!         VA buffer type
//! \param  [out] size
//!         Size
//! \param  [out] num_elements
//!         Number of elements
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_BufferInfo (
    VADriverContextP ctx,
    VABufferID       buf_id,
    VABufferType    *type,
    uint32_t        *size,
    uint32_t        *num_elements)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,          "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(type,         "nullptr type",         VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(size,         "nullptr size",         VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(num_elements, "nullptr num_elements", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    if (nullptr == mediaCtx)
        return VA_STATUS_ERROR_INVALID_CONTEXT;

    DDI_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)buf_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid buf_id", VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_MEDIA_BUFFER *buf  = DdiMedia_GetBufferFromVABufferID(mediaCtx, buf_id);
    if (nullptr == buf)
    {
        return VA_STATUS_ERROR_INVALID_BUFFER;
    }

    *type         = (VABufferType)buf->uiType;
    *size         = buf->iSize / buf->uiNumElements;
    *num_elements = buf->uiNumElements;

    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Lock surface
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] surface
//!         VA surface ID
//! \param  [out] fourcc
//!         FourCC
//! \param  [out] luma_stride
//!         Luma stride
//! \param  [out] chroma_u_stride
//!         Chroma U stride
//! \param  [out] chroma_v_stride
//!         Chroma V stride
//! \param  [out] luma_offset
//!         Luma offset
//! \param  [out] chroma_u_offset
//!         Chroma U offset
//! \param  [out] chroma_v_offset
//!         Chroma V offset
//! \param  [out] buffer_name
//!         Buffer name
//! \param  [out] buffer
//!         Buffer
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
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
    DDI_FUNCTION_ENTER();
    MOS_TraceEventExt(EVENT_VA_LOCK, EVENT_TYPE_START, &surface, sizeof(surface), nullptr, 0);

    DDI_CHK_NULL(ctx,             "nullptr context",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(fourcc,          "nullptr fourcc",          VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(luma_stride,     "nullptr luma_stride",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chroma_u_stride, "nullptr chroma_u_stride", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chroma_v_stride, "nullptr chroma_v_stride", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(luma_offset,     "nullptr luma_offset",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chroma_u_offset, "nullptr chroma_u_offset", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(chroma_v_offset, "nullptr chroma_v_offset", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(buffer_name,     "nullptr buffer_name",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(buffer,          "nullptr buffer",          VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx          = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr Media",                   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE *mediaSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface);
    
#ifdef _MMC_SUPPORTED
    // Decompress surface is needed
    DdiMedia_MediaMemoryDecompress(mediaCtx, mediaSurface);
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
    VAStatus vaStatus = DdiMedia_DeriveImage(ctx,surface,&tmpImage);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        buffer = nullptr;
        return vaStatus;
    }

    mediaSurface->uiLockedImageID = tmpImage.image_id;

    vaStatus = DdiMedia_MapBuffer(ctx,tmpImage.buf,buffer);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        buffer = nullptr;
        return vaStatus;
    }

    mediaSurface->uiLockedBufID = tmpImage.buf;

    *fourcc                 = tmpImage.format.fourcc;
    *luma_offset            = tmpImage.offsets[0];
    *luma_stride            = tmpImage.pitches[0];
    *chroma_u_offset        = tmpImage.offsets[1];
    *chroma_u_stride        = tmpImage.pitches[1];
    *chroma_v_offset        = tmpImage.offsets[2];
    *chroma_v_stride        = tmpImage.pitches[2];
    *buffer_name            = tmpImage.buf;

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
VAStatus DdiMedia_UnlockSurface (
    VADriverContextP   ctx,
    VASurfaceID        surface)
{
    DDI_FUNCTION_ENTER();
    MOS_TraceEventExt(EVENT_VA_UNLOCK, EVENT_TYPE_START, &surface, sizeof(surface), nullptr, 0);

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",                 VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE *mediaSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface", VA_STATUS_ERROR_INVALID_SURFACE);

    if (mediaSurface->uiLockedImageID == VA_INVALID_ID)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    VABufferID bufID    = (VABufferID)(mediaSurface->uiLockedBufID);
    VAStatus   vaStatus = DdiMedia_UnmapBuffer(ctx, bufID);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        return vaStatus;
    }
    mediaSurface->uiLockedBufID = VA_INVALID_ID;

    VAImageID imageID  = (VAImageID)(mediaSurface->uiLockedImageID);
    vaStatus = DdiMedia_DestroyImage(ctx,imageID);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        return vaStatus;
    }
    mediaSurface->uiLockedImageID = VA_INVALID_ID;

    MOS_TraceEventExt(EVENT_VA_UNLOCK, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Query video proc filters
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] context
//!         VA context ID
//! \param  [in] filters
//!         VA proc filter type
//! \param  [in] num_filters
//!         Number of filters
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus
DdiMedia_QueryVideoProcFilters(
    VADriverContextP    ctx,
    VAContextID         context,
    VAProcFilterType   *filters,
    uint32_t           *num_filters)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(context);

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(filters,     "nullptr filters",     VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(num_filters, "nullptr num_filters", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t  max_num_filters = DDI_VP_MAX_NUM_FILTERS;
    // check if array size is less than VP_MAX_NUM_FILTERS
    if(*num_filters < max_num_filters)
    {
        DDI_NORMALMESSAGE("num_filters %d < max_num_filters %d. Probably caused by Libva version upgrade!", *num_filters, max_num_filters);
    }

    // Set the filters
    uint32_t i = 0;
    while(i < *num_filters && i < DDI_VP_MAX_NUM_FILTERS)
    {
        filters[i] = vp_supported_filters[i];
        i++;
    }

    // Tell the app how many valid filters are filled in the array
    *num_filters = DDI_VP_MAX_NUM_FILTERS;

    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Query video processing filter capabilities.
//!         The real implementation is in media_libva_vp.c, since it needs to use some definitions in vphal.h.
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] context
//!         VA context ID
//! \param  [in] type
//!         VA proc filter type
//! \param  [inout] filter_caps
//!         FIlter caps
//! \param  [inout] num_filter_caps
//!         Number of filter caps
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
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

//!
//! \brief  Query video proc pipeline caps
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context 
//! \param  [in] context
//!         VA context ID
//! \param  [in] filters
//!         VA buffer ID
//! \param  [in] num_filters
//!         Number of filters
//! \param  [in] pipeline_caps
//!         VA proc pipeline caps
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus
DdiMedia_QueryVideoProcPipelineCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID         *filters,
    uint32_t            num_filters,
    VAProcPipelineCaps *pipeline_caps
)
{
    DDI_FUNCTION_ENTER();

    PDDI_MEDIA_CONTEXT mediaCtx   = DdiMedia_GetMediaContext(ctx);

    DDI_CHK_NULL(ctx,           "nullptr ctx",           VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pipeline_caps, "nullptr pipeline_caps", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (num_filters > 0)
        DDI_CHK_NULL(filters,   "nullptr filters",       VA_STATUS_ERROR_INVALID_PARAMETER);

    pipeline_caps->pipeline_flags             = VA_PROC_PIPELINE_FAST;
    pipeline_caps->filter_flags               = 0;
    pipeline_caps->rotation_flags             = (1 << VA_ROTATION_NONE) | (1 << VA_ROTATION_90) | (1 << VA_ROTATION_180) | (1 << VA_ROTATION_270);
    pipeline_caps->mirror_flags               = VA_MIRROR_HORIZONTAL  | VA_MIRROR_VERTICAL;
    pipeline_caps->blend_flags                = VA_BLEND_GLOBAL_ALPHA | VA_BLEND_PREMULTIPLIED_ALPHA | VA_BLEND_LUMA_KEY;
    pipeline_caps->num_forward_references     = DDI_CODEC_NUM_FWD_REF;
    pipeline_caps->num_backward_references    = DDI_CODEC_NUM_BK_REF;
    pipeline_caps->input_color_standards      = vp_input_color_std;
    pipeline_caps->num_input_color_standards  = DDI_VP_NUM_INPUT_COLOR_STD;
    pipeline_caps->output_color_standards     = vp_output_color_std;
    pipeline_caps->num_output_color_standards = DDI_VP_NUM_OUT_COLOR_STD;

    if ((context & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_DECODER)
    {
        //Decode+SFC, go SFC path, the restriction here is the capability of SFC
        pipeline_caps->num_input_pixel_formats    = 1;
        pipeline_caps->input_pixel_format[0]      = VA_FOURCC_NV12;
        pipeline_caps->num_output_pixel_formats   = 1;
        pipeline_caps->output_pixel_format[0]     = VA_FOURCC_NV12;
        if((MEDIA_IS_SKU(&(mediaCtx->SkuTable), FtrHCP2SFCPipe)))
        {
            pipeline_caps->max_input_width            = DDI_DECODE_HCP_SFC_MAX_WIDTH;
            pipeline_caps->max_input_height           = DDI_DECODE_HCP_SFC_MAX_HEIGHT;
        }
        else
        {
            pipeline_caps->max_input_width            = DDI_DECODE_SFC_MAX_WIDTH;
            pipeline_caps->max_input_height           = DDI_DECODE_SFC_MAX_HEIGHT;
        }
        pipeline_caps->min_input_width            = DDI_DECODE_SFC_MIN_WIDTH;
        pipeline_caps->min_input_height           = DDI_DECODE_SFC_MIN_HEIGHT;
        pipeline_caps->max_output_width           = DDI_DECODE_SFC_MAX_WIDTH;
        pipeline_caps->max_output_height          = DDI_DECODE_SFC_MAX_HEIGHT;
        pipeline_caps->min_output_width           = DDI_DECODE_SFC_MIN_WIDTH;
        pipeline_caps->min_output_height          = DDI_DECODE_SFC_MIN_HEIGHT;
    }
    else if ((context & DDI_MEDIA_MASK_VACONTEXT_TYPE) == DDI_MEDIA_VACONTEXTID_OFFSET_VP)
    {
        if(mediaCtx->platform.eRenderCoreFamily <= IGFX_GEN8_CORE)
        {
            //Capability of Gen8- platform
            pipeline_caps->max_input_width            = VP_MAX_PIC_WIDTH_Gen8;
            pipeline_caps->max_input_height           = VP_MAX_PIC_HEIGHT_Gen8;
            pipeline_caps->max_output_width           = VP_MAX_PIC_WIDTH_Gen8;
            pipeline_caps->max_output_height          = VP_MAX_PIC_HEIGHT_Gen8;
        }else
        {
            //Capability of Gen9+ platform
            pipeline_caps->max_input_width            = VP_MAX_PIC_WIDTH;
            pipeline_caps->max_input_height           = VP_MAX_PIC_HEIGHT;
            pipeline_caps->max_output_width           = VP_MAX_PIC_WIDTH;
            pipeline_caps->max_output_height          = VP_MAX_PIC_HEIGHT;
        }
        pipeline_caps->min_input_width            = VP_MIN_PIC_WIDTH;
        pipeline_caps->min_input_height           = VP_MIN_PIC_HEIGHT;
        pipeline_caps->min_output_width           = VP_MIN_PIC_WIDTH;
        pipeline_caps->min_output_height          = VP_MIN_PIC_WIDTH;
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
 * param[in] ctx               the VA display
 * param[in] config            the config identifying a codec or a video
 *     processing pipeline
 * param[out] attrib_list        the list of attributes on output, with at
 *     least \c type fields filled in, and possibly \c value fields whenever
 *     necessary.The updated list of attributes and flags on output
 * param[in] num_attribs       the number of attributes supplied in the
 *     \c attrib_list array
 */
VAStatus DdiMedia_GetSurfaceAttributes(
    VADriverContextP    ctx,
    VAConfigID          config,
    VASurfaceAttrib    *attrib_list,
    uint32_t            num_attribs
)
{
    DDI_UNUSED(ctx);
    DDI_UNUSED(config);
    DDI_UNUSED(attrib_list);
    DDI_UNUSED(num_attribs);

    DDI_FUNCTION_ENTER();

    VAStatus vaStatus = VA_STATUS_ERROR_UNIMPLEMENTED;

    return vaStatus;
}

//!
//! \brief  Aquire buffer handle
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] buf_id
//!         VA buffer ID
//! \param  [in] buf_info
//!         VA buffer Info
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_AcquireBufferHandle(
    VADriverContextP ctx,
    VABufferID buf_id,
    VABufferInfo *buf_info)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,          "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf_info,     "nullptr buf_info",     VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,          "Invalid Media ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_BUFFER   *buf     = DdiMedia_GetBufferFromVABufferID(mediaCtx, buf_id);
    DDI_CHK_NULL(buf,          "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(buf->bo,      "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);

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

    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    // already acquired?
    if (buf->uiExportcount)
    {   // yes, already acquired
        // can't provide access thru another memtype
        if (buf->uiMemtype != buf_info->mem_type)
        {
            DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
    }
    else
    {   // no, not acquired - doing this now
        switch (buf_info->mem_type) {
        case VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM: {
            uint32_t flink = 0;
            if (mos_bo_flink(buf->bo, &flink) != 0)
            {
                DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }
            buf->handle = (intptr_t)flink;
            break;
        }
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME: {
            int32_t prime_fd = 0;
            if (mos_bo_gem_export_to_prime(buf->bo, &prime_fd) != 0)
            {
                DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
                return VA_STATUS_ERROR_INVALID_BUFFER;
            }

            buf->handle = (intptr_t)prime_fd;
            break;
        }
        }
        // saving memtepy which was provided to the user
        buf->uiMemtype = buf_info->mem_type;
    }

    ++buf->uiExportcount;
    mos_bo_reference(buf->bo);

    buf_info->type = buf->uiType;
    buf_info->handle = buf->handle;
    buf_info->mem_size = buf->uiNumElements * buf->iSize;

    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Release buffer handle
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] buf_id
//!         VA bufferID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_ReleaseBufferHandle(
    VADriverContextP ctx,
    VABufferID buf_id)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "Invalid Media ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_BUFFER   *buf     = DdiMedia_GetBufferFromVABufferID(mediaCtx, buf_id);
    DDI_CHK_NULL(buf,          "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_CHK_NULL(buf->bo,      "Invalid Media Buffer", VA_STATUS_ERROR_INVALID_BUFFER);

    DdiMediaUtil_LockMutex(&mediaCtx->BufferMutex);
    if (!buf->uiMemtype || !buf->uiExportcount)
    {
        DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);
        return VA_STATUS_SUCCESS;
    }
    mos_bo_unreference(buf->bo);
    --buf->uiExportcount;

    if (!buf->uiExportcount) {
        switch (buf->uiMemtype) {
        case VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME: {
            close((intptr_t)buf->handle);
            break;
        }
        }
        buf->uiMemtype = 0;
    }
    DdiMediaUtil_UnLockMutex(&mediaCtx->BufferMutex);

    if (!buf->uiExportcount && buf->bPostponedBufFree) {
        MOS_FreeMemory(buf);
        DdiMedia_DestroyBufFromVABufferID(mediaCtx, buf_id);
    }

    return VA_STATUS_SUCCESS;
}

static uint32_t DdiMedia_GetPlaneNum(PDDI_MEDIA_SURFACE mediaSurface, bool hasAuxPlane)
{
    DDI_CHK_NULL(mediaSurface, "nullptr mediaSurface", VA_STATUS_ERROR_INVALID_PARAMETER);

    uint32_t fourcc = DdiMedia_MediaFormatToOsFormat(mediaSurface->format);
    uint32_t plane_num = 0;
    switch(fourcc)
    {
        case VA_FOURCC_NV12:
        case VA_FOURCC_NV21:
        case VA_FOURCC_P010:
        case VA_FOURCC_P012:
        case VA_FOURCC_P016:
            plane_num = hasAuxPlane ? 4 : 2;
            break;
            plane_num = hasAuxPlane ? 4 : 2;
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
            plane_num = 3;
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
            plane_num = hasAuxPlane ? 2 : 1;
            break;
        default:
            DDI_ASSERTMESSAGE("Unsupported format.\n");
    }
    return plane_num;
}

static uint32_t DdiMedia_GetDrmFormatOfSeparatePlane(uint32_t fourcc, int plane)
{
    if (plane == 0)
    {
        switch (fourcc)
        {
        case VA_FOURCC_NV12:
        case VA_FOURCC_I420:
        case VA_FOURCC_YV12:
        case VA_FOURCC_YV16:
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
        case VA_FOURCC_YV12:
        case VA_FOURCC_YV16:
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

static uint32_t DdiMedia_GetDrmFormatOfCompositeObject(uint32_t fourcc)
{
    switch (fourcc)
    {
    case VA_FOURCC_NV12:
        return DRM_FORMAT_NV12;
    case VA_FOURCC_I420:
        return DRM_FORMAT_YUV420;
    case VA_FOURCC_YV12:
        return DRM_FORMAT_YVU420;
    case VA_FOURCC_YV16:
        return DRM_FORMAT_YVU422;
    case VA_FOURCC_YUY2:
        return DRM_FORMAT_YUYV;
    case VA_FOURCC_YVYU:
        return DRM_FORMAT_YVYU;
    case VA_FOURCC_VYUY:
        return DRM_FORMAT_VYUY;
    case VA_FOURCC_UYVY:
        return DRM_FORMAT_UYVY;
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


//!
//! \brief   API for export surface handle to other component
//!
//! \param [in] dpy
//!          VA display.
//! \param [in] surface_id
//!          Surface to export.
//! \param [in] mem_type
//!          Memory type to export to.
//! \param [in] flags
//!          Combination of flags to apply
//!\param [out] descriptor
//!Pointer to the descriptor structure to fill
//!with the handle details.  The type of this structure depends on
//!the value of mem_type.
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_ExportSurfaceHandle(
    VADriverContextP ctx,
    VASurfaceID surface_id,
    uint32_t mem_type,
    uint32_t flags,
    void * descriptor)
{
    DDI_CHK_NULL(descriptor,    "nullptr descriptor",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(ctx,                     "nullptr ctx",                     VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)(surface_id), mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaces", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE  *mediaSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface_id);
    DDI_CHK_NULL(mediaSurface,                   "nullptr mediaSurface",                   VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->bo,               "nullptr mediaSurface->bo",               VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_CHK_NULL(mediaSurface->pGmmResourceInfo, "nullptr mediaSurface->pGmmResourceInfo", VA_STATUS_ERROR_INVALID_SURFACE);

    if (mem_type != VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME_2) {
        DDI_ASSERTMESSAGE("vaExportSurfaceHandle: memory type %08x is not supported.\n", mem_type);
        return VA_STATUS_ERROR_UNSUPPORTED_MEMORY_TYPE;
    }

    if (mos_bo_gem_export_to_prime(mediaSurface->bo, (int32_t*)&mediaSurface->name))
    {
        DDI_ASSERTMESSAGE("Failed drm_intel_gem_export_to_prime operation!!!\n");
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    VADRMPRIMESurfaceDescriptor *desc = (VADRMPRIMESurfaceDescriptor *)descriptor;
    desc->fourcc = DdiMedia_MediaFormatToOsFormat(mediaSurface->format);
    if(desc->fourcc == VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT)
    {
        return VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT;
    }
    desc->width           = mediaSurface->iWidth;
    desc->height          = mediaSurface->iRealHeight;
    desc->num_objects     = 1;
    desc->objects[0].fd   = mediaSurface->name;
    desc->objects[0].size = mediaSurface->pGmmResourceInfo->GetSizeSurface();

    // Prepare Compression info for export surface handle
    GMM_RESOURCE_FLAG       GmmFlags    = {0};
    bool                    bMmcEnabled = false;
    GmmFlags = mediaSurface->pGmmResourceInfo->GetResFlags();

    if ((GmmFlags.Gpu.MMC               ||
        GmmFlags.Gpu.CCS)               &&
        (GmmFlags.Info.MediaCompressed ||
         GmmFlags.Info.RenderCompressed))
    {
        bMmcEnabled = true;
    }
    else
    {
        bMmcEnabled = false;
    }

    switch (mediaSurface->TileType) {
    case I915_TILING_X:
        desc->objects[0].drm_format_modifier = I915_FORMAT_MOD_X_TILED;
        break;
    case I915_TILING_Y:
        if (mediaCtx->m_auxTableMgr && bMmcEnabled)
        {
            desc->objects[0].drm_format_modifier = GmmFlags.Info.MediaCompressed ? I915_FORMAT_MOD_Y_TILED_GEN12_MC_CCS :
             (GmmFlags.Info.RenderCompressed ? I915_FORMAT_MOD_Y_TILED_GEN12_RC_CCS : I915_FORMAT_MOD_Y_TILED);
        }else
        {
            desc->objects[0].drm_format_modifier = I915_FORMAT_MOD_Y_TILED;
        }
        break;
    case I915_TILING_NONE:
    default:
        desc->objects[0].drm_format_modifier = DRM_FORMAT_MOD_NONE;
    }

    int composite_object = flags & VA_EXPORT_SURFACE_COMPOSED_LAYERS;

    uint32_t formats[4];
    bool hasAuxPlane = (mediaCtx->m_auxTableMgr)? true: false;
    uint32_t num_planes = DdiMedia_GetPlaneNum(mediaSurface, hasAuxPlane);
    if(composite_object)
    {
        formats[0] = DdiMedia_GetDrmFormatOfCompositeObject(desc->fourcc);
        if(!formats[0])
        {
            DDI_ASSERTMESSAGE("vaExportSurfaceHandle: fourcc %08x is not supported for export as a composite object.\n", desc->fourcc);
            return VA_STATUS_ERROR_INVALID_SURFACE;
        }
    }
    else
    {
        for (int i = 0; i < num_planes; i++)
        {
            formats[i] = DdiMedia_GetDrmFormatOfSeparatePlane(desc->fourcc,i);
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
    DdiMedia_GetChromaPitchHeight(desc->fourcc, pitch, height, &chromaPitch, &chromaHeight);

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

    if (composite_object) {
        desc->num_layers = 1;
        desc->layers[0].drm_format = formats[0];
        desc->layers[0].num_planes = num_planes;
        if (mediaCtx->m_auxTableMgr)
        {
            // For semi-planar formats like NV12, CCS planes follow the Y and UV planes,
            // i.e. planes 0 and 1 are used for Y and UV surfaces, planes 2 and 3 for the respective CCS.
            for (int i = 0; i < num_planes/2; i++)
            {
                desc->layers[0].object_index[2*i] = 0;
                desc->layers[0].object_index[2*i+1] = 0;
                if (i == 0)
                {
                    // Y plane
                    desc->layers[0].offset[i] = offsetY;
                    desc->layers[0].pitch[i]  = mediaSurface->iPitch;
                    // Y aux plane
                    desc->layers[0].offset[i + num_planes/2] = auxOffsetY;
                    desc->layers[0].pitch[i + num_planes/2] = mediaSurface->iPitch/8;
                }
                else
                {
                    // UV plane
                    desc->layers[0].offset[i] = offsetU;
                    desc->layers[0].pitch[i]  = mediaSurface->iPitch;
                    // UV aux plane
                    desc->layers[0].offset[i + num_planes/2] = auxOffsetUV;
                    desc->layers[0].pitch[i + num_planes/2] = mediaSurface->iPitch/8;
                }
            }
        }else
        {
            for (int i = 0; i < num_planes; i++)
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
    }
    else
    {
        if (mediaCtx->m_auxTableMgr)
        {
            desc->num_layers = num_planes / 2;

            for (int i = 0; i < desc->num_layers; i++)
            {
                desc->layers[i].drm_format = formats[i];
                desc->layers[i].num_planes = 2;

                desc->layers[i].object_index[0] = 0;

                if (i == 0)
                {
                    desc->layers[i].offset[0] = offsetY;
                    desc->layers[i].offset[1] = auxOffsetY;
                    desc->layers[i].pitch[0]  = mediaSurface->iPitch;
                    desc->layers[i].pitch[1]  = mediaSurface->iPitch/8;
                }
                else
                {
                    desc->layers[i].offset[0] = offsetU;
                    desc->layers[i].offset[1] = auxOffsetUV;
                    desc->layers[i].pitch[0]  = mediaSurface->iPitch;
                    desc->layers[i].pitch[1]  = mediaSurface->iPitch/8;
                }
            }
        }else
        {
            desc->num_layers = num_planes;

            for (int i = 0; i < num_planes; i++)
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
    }

    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Init VA driver 0.31
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus __vaDriverInit(VADriverContextP ctx )
{
    return DdiMedia__Initialize(ctx, nullptr, nullptr);
}

#ifdef __cplusplus
extern "C" {
#endif

//! 
//! \brief Get VA_MAJOR_VERSION and VA_MINOR_VERSION from libva
//!         To form the function name in the format of _vaDriverInit_[VA_MAJOR_VERSION]_[VA_MINOR_VERSION]
//!
#define VA_DRV_INIT_DEF(_major,_minor) __vaDriverInit_##_major##_##_minor
#define VA_DRV_INIT_FUNC(va_major_version, va_minor_version) VA_DRV_INIT_DEF(va_major_version,va_minor_version)
#define VA_DRV_INIT_FUC_NAME VA_DRV_INIT_FUNC(VA_MAJOR_VERSION,VA_MINOR_VERSION)

//!
//! \brief  VA driver init function name
//! 
//! \param  [in] ctx
//!         Pointer to VA driver context
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
MEDIAAPI_EXPORT VAStatus VA_DRV_INIT_FUC_NAME(VADriverContextP ctx )
{
    return __vaDriverInit(ctx);
}

//!
//! \brief  Private API for openCL
//! 
//! \param  [in] dpy
//!         VA display
//! \param  [in] surface
//!         VA surface ID
//! \param  [in] prime_fd
//!         Prime fd
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
MEDIAAPI_EXPORT VAStatus DdiMedia_ExtGetSurfaceHandle(
    VADisplay      dpy,
    VASurfaceID   *surface,
    int32_t       *prime_fd)
{
    DDI_CHK_NULL(dpy,                     "nullptr dpy",                     VA_STATUS_ERROR_INVALID_DISPLAY);
    DDI_CHK_NULL(surface,                 "nullptr surfaces",                VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(prime_fd,                "nullptr id",                      VA_STATUS_ERROR_INVALID_PARAMETER);

    VADriverContextP   ctx      = ((VADisplayContextP)dpy)->pDriverContext;
    DDI_CHK_NULL(ctx,                     "nullptr ctx",                     VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)(*surface), mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaces", VA_STATUS_ERROR_INVALID_SURFACE);

    DDI_MEDIA_SURFACE  *mediaSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, *surface);
    if (mediaSurface)
    {
        if (mediaSurface->bo)
        {
            int32_t ret = mos_bo_gem_export_to_prime(mediaSurface->bo, (int32_t*)&mediaSurface->name);
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

    *prime_fd = mediaSurface->name;

    return VA_STATUS_SUCCESS;
}

//!
//! \brief  Map buffer 2
//! 
//! \param  [in] dpy
//!         VA display
//! \param  [in] buf_id
//!         VA buffer ID
//! \param  [out] pbuf
//!         Pointer to buffer
//! \param  [in] flag
//!         Flag
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
MEDIAAPI_EXPORT VAStatus DdiMedia_MapBuffer2(
    VADisplay           dpy,
    VABufferID          buf_id,
    void              **pbuf,
    int32_t             flag
)
{
    DDI_CHK_NULL(dpy,                     "nullptr dpy",                     VA_STATUS_ERROR_INVALID_DISPLAY);

    VADriverContextP ctx = ((VADisplayContextP)dpy)->pDriverContext;

    if ((flag == 0) || (flag & ~(MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY)))
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    return DdiMedia_MapBufferInternal(ctx, buf_id, pbuf, flag);
}

#ifdef __cplusplus
}
#endif

