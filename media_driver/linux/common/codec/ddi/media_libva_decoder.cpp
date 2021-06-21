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
//! \file     media_libva_decoder.cpp
//! \brief    libva(and its extension) decoder implementation.
//!
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "media_libva_decoder.h"
#include "media_libva_util.h"
#include "media_libva_cp_interface.h"
#include "media_libva_caps.h"
#include "codechal_memdecomp.h"
#include "mos_solo_generic.h"
#include "media_ddi_factory.h"
#include "media_ddi_decode_base.h"
#include "media_interfaces.h"
#include "media_ddi_decode_const.h"
#include "decode_status_report.h"
#include "vphal_render_vebox_memdecomp.h"

#if !defined(ANDROID) && defined(X11_FOUND)
#include <X11/Xutil.h>
#endif

#if defined(__linux__)
#include <linux/fb.h>
#define DEFAULT_FBDEV "/dev/graphics/fb0"
#elif defined(__DragonFly__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__sun)
#include <sys/fbio.h>
# if defined(__sun)
#define DEFAULT_FBDEV "/dev/fb"
# else
#define DEFAULT_FBDEV "/dev/ttyv0"
# endif
#define FBIOGET_VSCREENINFO FBIOGTYPE
#define fb_var_screeninfo fbtype
#define xres fb_width
#define yres fb_height
#elif defined(__NetBSD__) || defined(__OpenBSD__)
#include <dev/wscons/wsconsio.h>
# if defined(__OpenBSD__)
#define DEFAULT_FBDEV "/dev/ttyC0"
# else
#define DEFAULT_FBDEV "/dev/ttyE0"
# endif
#define FBIOGET_VSCREENINFO WSDISPLAYIO_GINFO
#define fb_var_screeninfo wsdisplay_fbinfo
#define xres width
#define yres height
#else
#warning "Cannot query framebuffer properties on this platform."
#define DEFAULT_FBDEV "/dev/fb0"
#define FBIOGET_VSCREENINFO 0
struct fb_var_screeninfo {
    uint32_t xres;
    uint32_t yres;
};
#endif

typedef MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR> DdiDecodeFactory;
static int32_t DdiDecode_GetDisplayInfo(VADriverContextP ctx)
{
    PDDI_MEDIA_CONTEXT mediaDrvCtx        = DdiMedia_GetMediaContext(ctx);
    int32_t fd                            = -1;
    struct fb_var_screeninfo              vsinfo;
    vsinfo.xres                           = 0;
    vsinfo.yres                           = 0;

    fd = open(DEFAULT_FBDEV,O_RDONLY);
    if(fd > 0)
    {
        if(ioctl(fd, FBIOGET_VSCREENINFO, &vsinfo) < 0)
        {
            DDI_NORMALMESSAGE("ioctl: fail to get display information!\n");
        }
        close(fd);
    }
    else
    {
        DDI_NORMALMESSAGE("GetDisplayInfo: cannot open device!\n");
    }

    if(vsinfo.xres <= 0 || vsinfo.yres <= 0)
    {
        vsinfo.xres = 1280;
        vsinfo.yres = 720;
    }
    mediaDrvCtx->uiDisplayWidth  = vsinfo.xres;
    mediaDrvCtx->uiDisplayHeight = vsinfo.yres;

    DDI_NORMALMESSAGE("DDI:mediaDrvCtx->uiDisplayWidth =%d", mediaDrvCtx->uiDisplayWidth);
    DDI_NORMALMESSAGE("DDI:mediaDrvCtx->uiDisplayHeight =%d",mediaDrvCtx->uiDisplayHeight);

    return 0;
}

VAStatus DdiDecode_CreateBuffer(
    VADriverContextP         ctx,
    PDDI_DECODE_CONTEXT      decCtx,
    VABufferType             type,
    uint32_t                 size,
    uint32_t                 numElements,
    void                    *data,
    VABufferID              *bufId
)
{
    *bufId     = VA_INVALID_ID;
    if (decCtx->m_ddiDecode){
        DDI_CHK_RET(decCtx->m_ddiDecode->CreateBuffer(type, size, numElements, data, bufId),"DdiDecode_CreateBuffer failed!");
    }

    return VA_STATUS_SUCCESS;

}

VAStatus DdiDecode_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         renderTarget
)
{
    DDI_FUNCTION_ENTER();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_DDI);

    uint32_t  ctxType;
    PDDI_DECODE_CONTEXT decCtx  = (PDDI_DECODE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    DDI_CHK_NULL(decCtx,            "nullptr decCtx",            VA_STATUS_ERROR_INVALID_CONTEXT);

    if (decCtx->pCpDdiInterface)
    {
        DDI_CHK_RET(decCtx->pCpDdiInterface->IsAttachedSessionAlive(), "Session not alive!");
    }

    if (decCtx->m_ddiDecode)
    {
        VAStatus va = decCtx->m_ddiDecode->BeginPicture(ctx, context, renderTarget);
        DDI_FUNCTION_EXIT(va);
        return va;
    }

    DDI_FUNCTION_EXIT(VA_STATUS_ERROR_UNIMPLEMENTED);
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

/*
 * Make the end of rendering for a picture.
 * The server should start processing all pending operations for this
 * surface. This call is non-blocking. The client can start another
 * Begin/Render/End sequence on a different render target.
 */
VAStatus DdiDecode_EndPicture (
    VADriverContextP    ctx,
    VAContextID         context
)
{
    DDI_FUNCTION_ENTER();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_DDI);

    DDI_CHK_NULL(ctx,                "nullptr context in vpgDecodeEndPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);
    uint32_t                         ctxType;
    // assume the VAContextID is decoder ID
    PDDI_DECODE_CONTEXT decCtx     = (PDDI_DECODE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    DDI_CHK_NULL(decCtx,            "nullptr decCtx",            VA_STATUS_ERROR_INVALID_CONTEXT);

    if (decCtx->pCpDdiInterface)
    {
        DDI_CHK_RET(decCtx->pCpDdiInterface->IsAttachedSessionAlive(), "Session not alive!");

        if (decCtx->pCpDdiInterface->IsCencProcessing())
        {
            VAStatus va = decCtx->pCpDdiInterface->EndPicture(ctx, context);
            DDI_FUNCTION_EXIT(va);
            return va;
        }
    }

    if (decCtx->m_ddiDecode)
    {
        VAStatus va = decCtx->m_ddiDecode->EndPicture(ctx, context);
        DDI_FUNCTION_EXIT(va);
        return va;
    }

    DDI_FUNCTION_EXIT(VA_STATUS_ERROR_UNIMPLEMENTED);
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

/*
 * Send decode buffers to the server.
 * Buffers are automatically destroyed afterwards
 */
VAStatus DdiDecode_RenderPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID         *buffers,
    int32_t             numBuffers
)
{
    VAStatus        va                      = VA_STATUS_SUCCESS;
    int32_t         numOfBuffers            = numBuffers;
    int32_t         priority                = 0;
    int32_t         priorityIndexInBuffers  = -1;
    bool            updatePriority          = false;

    DDI_FUNCTION_ENTER();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_DDI);

    DDI_CHK_NULL(ctx,                "nullptr context in vpgDecodeRenderPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);
    // assume the VAContextID is decoder ID
    uint32_t  ctxType;
    PDDI_DECODE_CONTEXT decCtx  = (PDDI_DECODE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    DDI_CHK_NULL(decCtx,            "nullptr decCtx",            VA_STATUS_ERROR_INVALID_CONTEXT);

    if (decCtx->pCpDdiInterface)
    {
        DDI_CHK_RET(decCtx->pCpDdiInterface->IsAttachedSessionAlive(), "Session not alive!");
    }

    priorityIndexInBuffers = DdiMedia_GetGpuPriority(ctx, buffers, numOfBuffers, &updatePriority, &priority);
    if (priorityIndexInBuffers != -1)
    {
        if(updatePriority)
        {
            va = DdiDecode_SetGpuPriority(ctx, decCtx, priority);
            if(va != VA_STATUS_SUCCESS)
                return va;
        }
        MovePriorityBufferIdToEnd(buffers, priorityIndexInBuffers, numOfBuffers);
        numOfBuffers--;
    }
    if (numOfBuffers == 0)
        return va;

    if (decCtx->m_ddiDecode)
    {
        va = decCtx->m_ddiDecode->RenderPicture(ctx, context, buffers, numOfBuffers);
        DDI_FUNCTION_EXIT(va);
        return va;
    }

    DDI_FUNCTION_EXIT(VA_STATUS_ERROR_UNIMPLEMENTED);
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

//!
//! \brief  Clean and free decode context structure.
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] decCtx
//!     Pointer to ddi decode context
//!
void DdiDecodeCleanUp(
    VADriverContextP    ctx,
    PDDI_DECODE_CONTEXT decCtx)
{
    if(decCtx)
    {
        if(decCtx->m_ddiDecode)
        {
            decCtx->m_ddiDecode->DestroyContext(ctx);
            MOS_Delete(decCtx->m_ddiDecode);
            MOS_FreeMemory(decCtx);
            decCtx = nullptr;
        }
    }
    return;
}

VAStatus DdiDecode_StatusReport(PDDI_MEDIA_CONTEXT mediaCtx, CodechalDecode *decoder, DDI_MEDIA_SURFACE *surface)
{
    if (decoder->IsStatusQueryReportingEnabled())
    {
        uint32_t i = 0;
        if (surface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUERY_STATE_PENDING)
        {
            CodechalDecodeStatusBuffer *decodeStatusBuf = decoder->GetDecodeStatusBuf();
            uint32_t uNumAvailableReport = (decodeStatusBuf->m_currIndex - decodeStatusBuf->m_firstIndex) & (CODECHAL_DECODE_STATUS_NUM - 1);
            DDI_CHK_CONDITION((uNumAvailableReport == 0),
                "No report available at all", VA_STATUS_ERROR_OPERATION_FAILED);

            for (i = 0; i < uNumAvailableReport; i++)
            {
                int32_t index = (decodeStatusBuf->m_firstIndex + i) & (CODECHAL_DECODE_STATUS_NUM - 1);
                if ((decodeStatusBuf->m_decodeStatus[index].m_decodeStatusReport.m_currDecodedPicRes.bo == surface->bo) ||
                    (decoder->GetStandard() == CODECHAL_VC1 && decodeStatusBuf->m_decodeStatus[index].m_decodeStatusReport.m_deblockedPicResOlp.bo == surface->bo))
                {
                    break;
                }
            }

            DDI_CHK_CONDITION((i == uNumAvailableReport),
                "No report available for this surface", VA_STATUS_ERROR_OPERATION_FAILED);

            uint32_t uNumCompletedReport = i+1;

            for (i = 0; i < uNumCompletedReport; i++)
            {
                CodechalDecodeStatusReport tempNewReport;
                MOS_ZeroMemory(&tempNewReport, sizeof(CodechalDecodeStatusReport));
                MOS_STATUS eStatus = decoder->GetStatusReport(&tempNewReport, 1);
                DDI_CHK_CONDITION(MOS_STATUS_SUCCESS != eStatus, "Get status report fail", VA_STATUS_ERROR_OPERATION_FAILED);

                MOS_LINUX_BO *bo = tempNewReport.m_currDecodedPicRes.bo;

                if (decoder->GetStandard() == CODECHAL_VC1)
                {
                    bo = (tempNewReport.m_deblockedPicResOlp.bo) ? tempNewReport.m_deblockedPicResOlp.bo : bo;
                }

                if ((tempNewReport.m_codecStatus == CODECHAL_STATUS_SUCCESSFUL) || (tempNewReport.m_codecStatus == CODECHAL_STATUS_ERROR) || (tempNewReport.m_codecStatus == CODECHAL_STATUS_INCOMPLETE))
                {
                    PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapElmt = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)mediaCtx->pSurfaceHeap->pHeapBase;

                    uint32_t j = 0;
                    for (j = 0; j < mediaCtx->pSurfaceHeap->uiAllocatedHeapElements; j++, mediaSurfaceHeapElmt++)
                    {
                        if (mediaSurfaceHeapElmt != nullptr &&
                                mediaSurfaceHeapElmt->pSurface != nullptr &&
                                bo == mediaSurfaceHeapElmt->pSurface->bo)
                        {
                            mediaSurfaceHeapElmt->pSurface->curStatusReport.decode.status = (uint32_t)tempNewReport.m_codecStatus;
                            mediaSurfaceHeapElmt->pSurface->curStatusReport.decode.errMbNum = (uint32_t)tempNewReport.m_numMbsAffected;
                            mediaSurfaceHeapElmt->pSurface->curStatusReport.decode.crcValue = (decoder->GetStandard() == CODECHAL_AVC)?(uint32_t)tempNewReport.m_frameCrc:0;
                            mediaSurfaceHeapElmt->pSurface->curStatusReportQueryState = DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED;
                            break;
                        }
                    }

                    if (j == mediaCtx->pSurfaceHeap->uiAllocatedHeapElements)
                    {
                        return VA_STATUS_ERROR_OPERATION_FAILED;
                    }
                }
                else
                {
                    // return failed if queried INCOMPLETE or UNAVAILABLE report.
                    return VA_STATUS_ERROR_OPERATION_FAILED;
                }
            }
        }

        // check the report ptr of current surface.
        if (surface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED)
        {
            if (surface->curStatusReport.decode.status == CODECHAL_STATUS_SUCCESSFUL)
            {
                return VA_STATUS_SUCCESS;
            }
            else if (surface->curStatusReport.decode.status == CODECHAL_STATUS_ERROR)
            {
                return VA_STATUS_ERROR_DECODING_ERROR;
            }
            else if (surface->curStatusReport.decode.status == CODECHAL_STATUS_INCOMPLETE || surface->curStatusReport.decode.status == CODECHAL_STATUS_UNAVAILABLE)
            {
                return mediaCtx->bMediaResetEnable ? VA_STATUS_SUCCESS : VA_STATUS_ERROR_HW_BUSY;
            }
        }
        else
        {
            return VA_STATUS_ERROR_OPERATION_FAILED;
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecode_StatusReport(PDDI_MEDIA_CONTEXT mediaCtx, DecodePipelineAdapter *decoder, DDI_MEDIA_SURFACE *surface)
{
    if (surface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUERY_STATE_PENDING)
    {
        uint32_t uNumCompletedReport = decoder->GetCompletedReport();
        DDI_CHK_CONDITION((uNumCompletedReport == 0),
            "No report available at all", VA_STATUS_ERROR_OPERATION_FAILED);

        for (uint32_t i = 0; i < uNumCompletedReport; i++)
        {
            decode::DecodeStatusReportData tempNewReport;
            MOS_ZeroMemory(&tempNewReport, sizeof(CodechalDecodeStatusReport));
            MOS_STATUS eStatus = decoder->GetStatusReport(&tempNewReport, 1);
            DDI_CHK_CONDITION(MOS_STATUS_SUCCESS != eStatus, "Get status report fail", VA_STATUS_ERROR_OPERATION_FAILED);

            MOS_LINUX_BO *bo = tempNewReport.currDecodedPicRes.bo;

            if ((tempNewReport.codecStatus == CODECHAL_STATUS_SUCCESSFUL) || (tempNewReport.codecStatus == CODECHAL_STATUS_ERROR) || (tempNewReport.codecStatus == CODECHAL_STATUS_INCOMPLETE))
            {
                PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapElmt = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)mediaCtx->pSurfaceHeap->pHeapBase;

                uint32_t j = 0;
                for (j = 0; j < mediaCtx->pSurfaceHeap->uiAllocatedHeapElements; j++, mediaSurfaceHeapElmt++)
                {
                    if (mediaSurfaceHeapElmt != nullptr &&
                            mediaSurfaceHeapElmt->pSurface != nullptr &&
                            bo == mediaSurfaceHeapElmt->pSurface->bo)
                    {
                        mediaSurfaceHeapElmt->pSurface->curStatusReport.decode.status = (uint32_t)tempNewReport.codecStatus;
                        mediaSurfaceHeapElmt->pSurface->curStatusReport.decode.errMbNum = (uint32_t)tempNewReport.numMbsAffected;
                        mediaSurfaceHeapElmt->pSurface->curStatusReport.decode.crcValue = (uint32_t)tempNewReport.frameCrc;
                        mediaSurfaceHeapElmt->pSurface->curStatusReportQueryState = DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED;
                        break;
                    }
                }

                if (j == mediaCtx->pSurfaceHeap->uiAllocatedHeapElements)
                {
                    return VA_STATUS_ERROR_OPERATION_FAILED;
                }
            }
            else
            {
                // return failed if queried INCOMPLETE or UNAVAILABLE report.
                return VA_STATUS_ERROR_OPERATION_FAILED;
            }
        }
    }

    // check the report ptr of current surface.
    if (surface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED)
    {
        if (surface->curStatusReport.decode.status == CODECHAL_STATUS_SUCCESSFUL)
        {
            return VA_STATUS_SUCCESS;
        }
        else if (surface->curStatusReport.decode.status == CODECHAL_STATUS_ERROR)
        {
            return VA_STATUS_ERROR_DECODING_ERROR;
        }
        else if (surface->curStatusReport.decode.status == CODECHAL_STATUS_INCOMPLETE || surface->curStatusReport.decode.status == CODECHAL_STATUS_UNAVAILABLE)
        {
            return mediaCtx->bMediaResetEnable ? VA_STATUS_SUCCESS : VA_STATUS_ERROR_HW_BUSY;
        }
    }
    else
    {
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }
    return VA_STATUS_SUCCESS;
}

/*
 *  vpgDecodeCreateContext - Create a decode context
 *  dpy: display
 *  config_id: configuration for the context
 *  picture_width: coded picture width
 *  picture_height: coded picture height
 *  render_targets: render targets (surfaces) tied to the context
 *  num_render_targets: number of render targets in the above array
 *  context: created context id upon return
 */
VAStatus DdiDecode_CreateContext (
    VADriverContextP    ctx,
    VAConfigID          configId,
    int32_t             pictureWidth,
    int32_t             pictureHeight,
    int32_t             flag,
    VASurfaceID        *renderTargets,
    int32_t             numRenderTargets,
    VAContextID        *context
)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_DDI);

    MOS_CONTEXT                       mosCtx = {};
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT contextHeapElement;
    DdiMediaDecode                    *ddiDecBase;
    DDI_DECODE_CONFIG_ATTR            decConfigAttr;

    DDI_UNUSED(flag);

    VAStatus va            = VA_STATUS_SUCCESS;
    decConfigAttr.uiDecSliceMode = VA_DEC_SLICE_MODE_BASE;
    *context            = VA_INVALID_ID;

    uint16_t mode               = CODECHAL_DECODE_MODE_AVCVLD;

    DDI_CHK_NULL(ctx, "nullptr Ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx  = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_DECODE_CONTEXT decCtx = nullptr;
    if (numRenderTargets > DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    std::string codecKey = DECODE_ID_NONE;

    DDI_CHK_NULL(mediaCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_RET(mediaCtx->m_caps->GetDecConfigAttr(
            configId + DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE,
            &decConfigAttr.profile,
            &decConfigAttr.entrypoint,
            &decConfigAttr.uiDecSliceMode,
            &decConfigAttr.uiEncryptionType,
            &decConfigAttr.uiDecProcessingType),"Invalide config_id!");

    mode = mediaCtx->m_caps->GetDecodeCodecMode(decConfigAttr.profile);
    codecKey =  mediaCtx->m_caps->GetDecodeCodecKey(decConfigAttr.profile);
    va       =  mediaCtx->m_caps->CheckDecodeResolution(
                mode,
                decConfigAttr.profile,
                pictureWidth,
                pictureHeight);
    if (va != VA_STATUS_SUCCESS)
    {
        DdiDecodeCleanUp(ctx,decCtx);
        return va;
    }

    ddiDecBase = DdiDecodeFactory::CreateCodec(codecKey, nullptr);

    if (ddiDecBase == nullptr)
    {
        DDI_ASSERTMESSAGE("DDI: failed to Create DecodeContext in vaCreateContext\n");
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (ddiDecBase->BasicInit(&decConfigAttr) != VA_STATUS_SUCCESS)
    {
        MOS_Delete(ddiDecBase);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    /* one instance of DdiMediaDecode is created for the codec */
    decCtx = (DDI_DECODE_CONTEXT *) (*ddiDecBase);

    if (nullptr == decCtx)
    {
        if (ddiDecBase)
            MOS_Delete(ddiDecBase);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    decCtx->pMediaCtx                       = mediaCtx;
    decCtx->m_ddiDecode                     = ddiDecBase;

    mosCtx.bufmgr                = mediaCtx->pDrmBufMgr;
    mosCtx.m_gpuContextMgr       = mediaCtx->m_gpuContextMgr;
    mosCtx.m_cmdBufMgr           = mediaCtx->m_cmdBufMgr;
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
    mosCtx.m_osDeviceContext     = mediaCtx->m_osDeviceContext;
    mosCtx.m_apoMosEnabled       = mediaCtx->m_apoMosEnabled;
    mosCtx.pPerfData             = (PERF_DATA *)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));

    if (nullptr == mosCtx.pPerfData)
    {
        va = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiDecodeCleanUp(ctx, decCtx);
        return va;
    }

    ddiDecBase->ContextInit(pictureWidth, pictureHeight);

    //initialize DDI level CP interface
    decCtx->pCpDdiInterface = Create_DdiCpInterface(mosCtx);
    if (nullptr == decCtx->pCpDdiInterface)
    {
        va = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiDecodeCleanUp(ctx,decCtx);
        return va;
    }

    /* the step three */
    va = ddiDecBase->CodecHalInit(mediaCtx, &mosCtx);
    if (va != VA_STATUS_SUCCESS)
    {
        DdiDecodeCleanUp(ctx,decCtx);
        return va;
    }

    DdiDecode_GetDisplayInfo(ctx);

    // register render targets
    for (int32_t i = 0; i < numRenderTargets; i++)
    {
        DDI_MEDIA_SURFACE   *surface;

        surface   = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, renderTargets[i]);
        if (nullptr == surface)
        {
            DDI_ASSERTMESSAGE("DDI: invalid render target %d in vpgCreateContext.",i);
            va = VA_STATUS_ERROR_INVALID_SURFACE;
            DdiDecodeCleanUp(ctx,decCtx);
            return va;
        }
        if (VA_STATUS_SUCCESS != ddiDecBase->RegisterRTSurfaces(&decCtx->RTtbl, surface))
        {
            va = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
            DdiDecodeCleanUp(ctx,decCtx);
            return va;
        }
    }

    DdiMediaUtil_LockMutex(&mediaCtx->DecoderMutex);
    contextHeapElement = DdiMediaUtil_AllocPVAContextFromHeap(mediaCtx->pDecoderCtxHeap);

    if (nullptr == contextHeapElement)
    {
        DdiMediaUtil_UnLockMutex(&mediaCtx->DecoderMutex);
        va = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        DdiDecodeCleanUp(ctx,decCtx);
        return va;
    }

    contextHeapElement->pVaContext     = (void*)decCtx;
    mediaCtx->uiNumDecoders++;
    *context                           = (VAContextID)(contextHeapElement->uiVaContextID + DDI_MEDIA_VACONTEXTID_OFFSET_DECODER);
    DdiMediaUtil_UnLockMutex(&mediaCtx->DecoderMutex);

    // init the RecListSUrfaceID for checking DPB.
    for(int32_t i = 0; i < CODEC_AVC_NUM_UNCOMPRESSED_SURFACE; i++)
    {
        decCtx->RecListSurfaceID[i] = VA_INVALID_ID;
    }
    return va;
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
static void* DdiMedia_GetDecCtxFromVABufferID (PDDI_MEDIA_CONTEXT mediaCtx, VABufferID bufferID)
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


/////////////////////////////////////////////////////////////////////////////
//! \Free allocated bufferheap elements
//! \params
//! [in] VADriverContextP
//! [out] none
//! \returns
/////////////////////////////////////////////////////////////////////////////
static void DdiMedia_FreeBufferHeapElements(VADriverContextP    ctx, PDDI_DECODE_CONTEXT decCtx)
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

        void *pDecContext =DdiMedia_GetDecCtxFromVABufferID(mediaCtx, mediaBufferHeapElmt->uiVaBufferID);
        if(pDecContext == decCtx)
        {
            DDI_MEDIA_BUFFER   *buf     = DdiMedia_GetBufferFromVABufferID(mediaCtx,  mediaBufferHeapElmt->uiVaBufferID);

            if (nullptr == buf)
            {
                return;
            }

            if(buf->uiType == VASliceDataBufferType ||
                buf->uiType == VAProtectedSliceDataBufferType ||
                buf->uiType == VASliceParameterBufferType)
            {
                DdiMedia_DestroyBuffer(ctx, mediaBufferHeapElmt->uiVaBufferID);
            }
        }
        //Ensure the non-empty buffer to be destroyed.
        --bufNums;
    }
}

VAStatus DdiDecode_DestroyContext (
    VADriverContextP    ctx,
    VAContextID         context
)
{
    PDDI_MEDIA_CONTEXT mediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,          "nullptr mediaCtx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    uint32_t  ctxType;
    PDDI_DECODE_CONTEXT decCtx    = (PDDI_DECODE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, context, &ctxType);
    DDI_CHK_NULL(decCtx,            "nullptr decCtx",            VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(decCtx->pCodecHal, "nullptr decCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    /* Free the context id from the context_heap earlier */
    uint32_t decIndex                 = (uint32_t)context & DDI_MEDIA_MASK_VACONTEXTID;
    DdiMediaUtil_LockMutex(&mediaCtx->DecoderMutex);
    DdiMediaUtil_ReleasePVAContextFromHeap(mediaCtx->pDecoderCtxHeap, decIndex);
    mediaCtx->uiNumDecoders--;
    DdiMediaUtil_UnLockMutex(&mediaCtx->DecoderMutex);

    DdiMedia_FreeBufferHeapElements(ctx, decCtx);

    if (decCtx->m_ddiDecode) {
    DdiDecodeCleanUp(ctx,decCtx);
        return VA_STATUS_SUCCESS;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecode_SetGpuPriority(
    VADriverContextP     ctx,
    PDDI_DECODE_CONTEXT  decCtx,
    int32_t              priority
)
{
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,           "nullptr mediaCtx",                             VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(decCtx,             "nullptr decCtx",                               VA_STATUS_ERROR_INVALID_CONTEXT);

    //Set the priority for Gpu
    if(decCtx->pCodecHal != nullptr)
    {
        PMOS_INTERFACE osInterface = decCtx->pCodecHal->GetOsInterface();
        DDI_CHK_NULL(osInterface, "nullptr osInterface.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        osInterface->pfnSetGpuPriority(osInterface, priority);
    }
#ifdef _MMC_SUPPORTED
    //set the priority for decomp interface
    if(mediaCtx->pMediaMemDecompState)
    {
        MediaVeboxDecompState *mediaVeboxDecompState = static_cast<MediaVeboxDecompState*>(mediaCtx->pMediaMemDecompState);
        if(mediaVeboxDecompState->m_osInterface)
            mediaVeboxDecompState->m_osInterface->pfnSetGpuPriority(mediaVeboxDecompState->m_osInterface, priority);
    }
#endif
    return VA_STATUS_SUCCESS;
}

