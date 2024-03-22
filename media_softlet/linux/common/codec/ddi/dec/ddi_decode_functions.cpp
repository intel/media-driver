/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     ddi_decode_functions.cpp
//! \brief    ddi decode functions implementaion.
//!

#include <sys/ioctl.h>
#include <fcntl.h>
#if defined(__linux__)
#include <linux/fb.h>
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

#include "ddi_decode_functions.h"
#include "media_libva_util_next.h"
#include "media_libva_common_next.h"
#include "ddi_register_components_specific.h"
#include "decode_status_report.h"
#include "vphal_render_vebox_memdecomp.h"
#include "media_libva_interface_next.h"
#include "ddi_decode_trace_specific.h"

#define DDI_DECODE_SFC_MAX_WIDTH       4096
#define DDI_DECODE_SFC_MAX_HEIGHT      4096
#define DDI_DECODE_SFC_MIN_WIDTH       128
#define DDI_DECODE_SFC_MIN_HEIGHT      128
#define DDI_DECODE_HCP_SFC_MAX_WIDTH   (16*1024)
#define DDI_DECODE_HCP_SFC_MAX_HEIGHT  (16*1024)

#ifndef VA_ENCRYPTION_TYPE_NONE
#define VA_ENCRYPTION_TYPE_NONE        0x00000000
#endif

using namespace decode;

VAStatus DdiDecodeFunctions::CreateConfig (
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attribList,
    int32_t           numAttribs,
    VAConfigID        *configId)
{
    DDI_CODEC_FUNC_ENTER;

    VAStatus status = VA_STATUS_SUCCESS;
    DDI_CODEC_CHK_NULL(configId,   "nullptr configId",   VA_STATUS_ERROR_INVALID_PARAMETER);
    if(numAttribs)
    {
        DDI_CODEC_CHK_NULL(attribList, "nullptr attribList", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(mediaCtx->m_capsNext->m_capsTable, "nullptr m_capsTable", VA_STATUS_ERROR_INVALID_PARAMETER);

    status = mediaCtx->m_capsNext->CreateConfig(profile, entrypoint, attribList, numAttribs, configId);
    DDI_CODEC_CHK_RET(status, "Create common config failed");

    VAConfigAttrib decAttributes[3];

    decAttributes[0].type  = VAConfigAttribDecSliceMode;
    decAttributes[0].value = VA_DEC_SLICE_MODE_NORMAL;
    decAttributes[1].type  = VAConfigAttribEncryption;
    decAttributes[1].value = VA_ENCRYPTION_TYPE_NONE;
    decAttributes[2].type  = VAConfigAttribDecProcessing;
    decAttributes[2].value = VA_DEC_PROCESSING_NONE;

    int32_t i = 0, j = 0;
    for (j = 0; j < numAttribs; j++)
    {
        for (i = 0; i < 3; i++)
        {
            if (attribList[j].type == decAttributes[i].type)
            {
                decAttributes[i].value = attribList[j].value;
                break;
            }
        }
    }

    auto configList = mediaCtx->m_capsNext->GetConfigList();
    DDI_CODEC_CHK_NULL(configList, "Get configList failed", VA_STATUS_ERROR_INVALID_PARAMETER);

    for (int i = 0; i < configList->size(); i++)
    {
        if ((configList->at(i).profile == profile) &&
            (configList->at(i).entrypoint == entrypoint))
        {
            if (decAttributes[0].value == configList->at(i).componentData.data.sliceMode   &&
                decAttributes[1].value == configList->at(i).componentData.data.encryptType &&
                decAttributes[2].value == configList->at(i).componentData.data.processType)
            {
                uint32_t curConfigID = ADD_CONFIG_ID_DEC_OFFSET(i);
                if (!mediaCtx->m_capsNext->m_capsTable->IsDecConfigId(curConfigID))
                {
                    DDI_CODEC_ASSERTMESSAGE("DDI: Invalid configID.");
                    return VA_STATUS_ERROR_INVALID_CONFIG;
                }
                *configId = curConfigID;
                return VA_STATUS_SUCCESS;
            }
        }
    }

    *configId = 0xFFFFFFFF;
    return VA_STATUS_ERROR_ATTR_NOT_SUPPORTED;
}

VAStatus DdiDecodeFunctions::CreateContext(
    VADriverContextP ctx,
    VAConfigID       configId,
    int32_t          pictureWidth,
    int32_t          pictureHeight,
    int32_t          flag,
    VASurfaceID      *renderTargets,
    int32_t          renderTargetsNum,
    VAContextID      *context)
{
    DDI_CODEC_FUNC_ENTER;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_CREATECONTEXT_START eventData;
        eventData.configId = configId;
        MOS_TraceEvent(EVENT_DECODE_DDI_CREATECONTEXTVA, EVENT_TYPE_START, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_DDI);

    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx in Decode CreateContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (renderTargetsNum > DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_capsNext", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaCtx->m_capsNext->m_capsTable, "nullptr m_capsTable", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_UNUSED(flag);

    ConfigLinux *configItem = nullptr;
    configItem = mediaCtx->m_capsNext->m_capsTable->QueryConfigItemFromIndex(configId + DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE);
    DDI_CODEC_CHK_NULL(configItem, "Invalid config id!", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_DECODE_CONTEXT decCtx = nullptr;
    VAStatus va = VA_STATUS_SUCCESS;

    DdiDecodeBase *ddiDecode = DdiDecodeFactory::Create(ComponentInfo{configItem->profile, configItem->entrypoint});
    DDI_CODEC_CHK_NULL(ddiDecode, "DDI: failed to Create Decode Context in CreateContext", VA_STATUS_ERROR_ALLOCATION_FAILED);

    va = ddiDecode->CheckDecodeResolution(configItem, pictureWidth, pictureHeight);
    if (va != VA_STATUS_SUCCESS)
    {
        return va;
    }

    if (ddiDecode->BasicInit(configItem) != VA_STATUS_SUCCESS)
    {
        MOS_Delete(ddiDecode);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    /* one instance of DdiDecodeBase is created for the codec */
    decCtx = ddiDecode->m_decodeCtx;
    if (nullptr == decCtx)
    {
        if (ddiDecode)
        {
            MOS_Delete(ddiDecode);
        }
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    decCtx->pMediaCtx   = mediaCtx;
    decCtx->m_ddiDecodeNext = ddiDecode;

    MOS_CONTEXT mosCtx = {};
    mosCtx.bufmgr                = mediaCtx->pDrmBufMgr;
    mosCtx.fd                    = mediaCtx->fd;
    mosCtx.iDeviceId             = mediaCtx->iDeviceId;
    mosCtx.m_skuTable            = mediaCtx->SkuTable;
    mosCtx.m_waTable             = mediaCtx->WaTable;
    mosCtx.m_gtSystemInfo        = *mediaCtx->pGtSystemInfo;
    mosCtx.m_platform            = mediaCtx->platform;
    mosCtx.ppMediaMemDecompState = &mediaCtx->pMediaMemDecompState;
    mosCtx.pfnMemoryDecompress   = mediaCtx->pfnMemoryDecompress;
    mosCtx.pfnMediaMemoryCopy    = mediaCtx->pfnMediaMemoryCopy;
    mosCtx.pfnMediaMemoryCopy2D  = mediaCtx->pfnMediaMemoryCopy2D;
    mosCtx.ppMediaCopyState      = &mediaCtx->pMediaCopyState;
    mosCtx.m_auxTableMgr         = mediaCtx->m_auxTableMgr;
    mosCtx.pGmmClientContext     = mediaCtx->pGmmClientContext;
    mosCtx.m_osDeviceContext     = mediaCtx->m_osDeviceContext;
    mosCtx.m_apoMosEnabled       = true;
    mosCtx.m_userSettingPtr      = mediaCtx->m_userSettingPtr;
    mosCtx.pPerfData             = (PERF_DATA *)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));

    if (nullptr == mosCtx.pPerfData)
    {
        va = VA_STATUS_ERROR_ALLOCATION_FAILED;
        CleanUp(ctx, decCtx);
        return va;
    }

    ddiDecode->ContextInit(pictureWidth, pictureHeight);

    // Initialize DDI level CP interface
    decCtx->pCpDdiInterfaceNext = CreateDdiCpNext(&mosCtx);
    if (nullptr == decCtx->pCpDdiInterfaceNext)
    {
        va = VA_STATUS_ERROR_ALLOCATION_FAILED;
        CleanUp(ctx, decCtx);
        return va;
    }

    va = ddiDecode->CodecHalInit(mediaCtx, &mosCtx);
    if (va != VA_STATUS_SUCCESS)
    {
        CleanUp(ctx, decCtx);
        return va;
    }

    GetDisplayInfo(ctx);

    // register render targets
    for (int32_t i = 0; i < renderTargetsNum; i++)
    {
        DDI_MEDIA_SURFACE *surface;
        surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, renderTargets[i]);
        if (nullptr == surface)
        {
            DDI_CODEC_ASSERTMESSAGE("DDI: invalid render target %d in vpgCreateContext.",i);
            va = VA_STATUS_ERROR_INVALID_SURFACE;
            CleanUp(ctx, decCtx);
            return va;
        }

        if (VA_STATUS_SUCCESS != ddiDecode->RegisterRTSurfaces(&decCtx->RTtbl, surface))
        {
            va = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
            CleanUp(ctx, decCtx);
            return va;
        }
    }

    MosUtilities::MosLockMutex(&mediaCtx->DecoderMutex);
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaContextHeapElmt = MediaLibvaUtilNext::DdiAllocPVAContextFromHeap(mediaCtx->pDecoderCtxHeap);
    if (nullptr == vaContextHeapElmt)
    {
        MosUtilities::MosUnlockMutex(&mediaCtx->DecoderMutex);
        va = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        CleanUp(ctx, decCtx);
        return va;
    }

    vaContextHeapElmt->pVaContext = (void*)decCtx;
    mediaCtx->uiNumDecoders++;
    *context = (VAContextID)(vaContextHeapElmt->uiVaContextID + DDI_MEDIA_SOFTLET_VACONTEXTID_DECODER_OFFSET);
    MosUtilities::MosUnlockMutex(&mediaCtx->DecoderMutex);

    // init the RecListSUrfaceID for checking DPB.
    for(int32_t i = 0; i < CODEC_AVC_NUM_UNCOMPRESSED_SURFACE; i++)
    {
        decCtx->RecListSurfaceID[i] = VA_INVALID_ID;
    }

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_CREATECONTEXT eventData;
        eventData.configId = configId;
        eventData.hRes = va;
        MOS_TraceEvent(EVENT_DECODE_DDI_CREATECONTEXTVA, EVENT_TYPE_END, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    return va;
}

void DdiDecodeFunctions::FreeBufferHeapElements(VADriverContextP ctx, PDDI_DECODE_CONTEXT decCtx)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx in Decode DestroyContext", );
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", );

    PDDI_MEDIA_HEAP bufferHeap = mediaCtx->pBufferHeap;
    if (nullptr == bufferHeap)
    {
        return;
    }

    PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapBase = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)bufferHeap->pHeapBase;
    if (nullptr == mediaBufferHeapBase)
    {
        return;
    }

    int32_t bufNums = mediaCtx->uiNumBufs;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_FREEBUFFERHEAPELEMENTS eventData;
        eventData.bufNums = bufNums;
        MOS_TraceEvent(EVENT_DECODE_DDI_FREEBUFFERHEAPELEMENTSVA, EVENT_TYPE_START, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    for (int32_t elementId = 0; bufNums > 0; ++elementId)
    {
        PDDI_MEDIA_BUFFER_HEAP_ELEMENT mediaBufferHeapElmt = &mediaBufferHeapBase[elementId];
        if (nullptr == mediaBufferHeapElmt->pBuffer)
            continue;

        void *pDecContext = nullptr;
        uint32_t i = (uint32_t)mediaBufferHeapElmt->uiVaBufferID;
        DDI_CODEC_CHK_LESS(i, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "invalid buffer id", );
        MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
        PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufHeapElement = (PDDI_MEDIA_BUFFER_HEAP_ELEMENT)mediaCtx->pBufferHeap->pHeapBase;
        bufHeapElement += i;
        pDecContext = bufHeapElement->pCtx;
        MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);

        if (pDecContext == decCtx)
        {
            DDI_MEDIA_BUFFER *buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, mediaBufferHeapElmt->uiVaBufferID);

            if (nullptr == buf)
            {
                return;
            }

            if (buf->uiType == VASliceDataBufferType ||
                buf->uiType == VAProtectedSliceDataBufferType ||
                buf->uiType == VASliceParameterBufferType)
            {
                MediaLibvaInterfaceNext::DestroyBuffer(ctx, mediaBufferHeapElmt->uiVaBufferID);
            }
        }
        // Ensure the non-empty buffer to be destroyed.
        --bufNums;
    }

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_FREEBUFFERHEAPELEMENTS eventData;
        eventData.bufNums = bufNums;
        MOS_TraceEvent(EVENT_DECODE_DDI_FREEBUFFERHEAPELEMENTSVA, EVENT_TYPE_END, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    return;
}

VAStatus DdiDecodeFunctions::DestroyContext(
    VADriverContextP ctx,
    VAContextID      context)
{
    DDI_CODEC_FUNC_ENTER;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_DESTROYCONTEXT_START eventData;
        eventData.context = context;
        MOS_TraceEvent(EVENT_DECODE_DDI_DESTROYCONTEXTVA, EVENT_TYPE_START, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx in Decode DestroyContext", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    uint32_t ctxType;
    PDDI_DECODE_CONTEXT decCtx = (decltype(decCtx))MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CODEC_CHK_NULL(decCtx, "nullptr decCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(decCtx->pCodecHal, "nullptr decCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    /* Free the context id from the context_heap earlier */
    uint32_t decIndex = (uint32_t)context & DDI_MEDIA_MASK_VACONTEXTID;
    MosUtilities::MosLockMutex(&mediaCtx->DecoderMutex);
    MediaLibvaUtilNext::DdiReleasePVAContextFromHeap(mediaCtx->pDecoderCtxHeap, decIndex);
    mediaCtx->uiNumDecoders--;
    MosUtilities::MosUnlockMutex(&mediaCtx->DecoderMutex);

    FreeBufferHeapElements(ctx, decCtx);

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_DESTROYCONTEXT eventData;
        eventData.context = context;
        MOS_TraceEvent(EVENT_DECODE_DDI_DESTROYCONTEXTVA, EVENT_TYPE_END, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    if (decCtx->m_ddiDecodeNext)
    {
        CleanUp(ctx, decCtx);
        return VA_STATUS_SUCCESS;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::CreateBuffer(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferType     type,
    uint32_t         size,
    uint32_t         elementsNum,
    void             *data,
    VABufferID       *bufId)
{
    DDI_CODEC_FUNC_ENTER;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        MOS_TraceEvent(EVENT_DECODE_DDI_CREATEBUFFERVA, EVENT_TYPE_START, NULL, 0, NULL, 0);
    }
#endif

    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx in Decode CreateBuffer", VA_STATUS_ERROR_INVALID_CONTEXT);
    uint32_t ctxType;
    PDDI_DECODE_CONTEXT decCtx = (decltype(decCtx))MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CODEC_CHK_NULL(decCtx, "nullptr decCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    *bufId = VA_INVALID_ID;
    if (decCtx->m_ddiDecodeNext)
    {
        DDI_CHK_RET(decCtx->m_ddiDecodeNext->CreateBuffer(type, size, elementsNum, data, bufId), "Decode CreateBuffer failed!");
    }

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_CREATEBUFFER eventData;
        eventData.type = type;
        eventData.size = size;
        eventData.numElements = elementsNum;
        eventData.bufId = bufId;
        MOS_TraceEvent(EVENT_DECODE_DDI_CREATEBUFFERVA, EVENT_TYPE_END, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::BeginPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VASurfaceID      renderTarget)
{
    DDI_CODEC_FUNC_ENTER;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_BEGINPICTURE_START eventData;
        eventData.FrameIndex    = DecodeFrameIndex;
        MOS_TraceEvent(EVENT_DECODE_DDI_BEGINPICTUREVA, EVENT_TYPE_START, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_DDI);

    DDI_CODEC_CHK_NULL(ctx, "nullptr context in Decode BeginPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t  ctxType = 0;
    PDDI_DECODE_CONTEXT decCtx = (decltype(decCtx))MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CODEC_CHK_NULL(decCtx, "Null decCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (decCtx->pCpDdiInterfaceNext)
    {
        VAStatus ret = decCtx->pCpDdiInterfaceNext->IsAttachedSessionAlive();
        DDI_CODEC_CHK_CONDITION(VA_STATUS_SUCCESS != ret, "Session not alive!", ret);
    }

    if (decCtx->m_ddiDecodeNext)
    {
        VAStatus va = decCtx->m_ddiDecodeNext->BeginPicture(ctx, context, renderTarget);

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
        {
            DECODE_EVENTDATA_VA_BEGINPICTURE eventData;
            eventData.FrameIndex                  = DecodeFrameIndex;
            eventData.hRes                        = va;
            MOS_TraceEvent(EVENT_DECODE_DDI_BEGINPICTUREVA, EVENT_TYPE_END, &eventData, sizeof(eventData), NULL, 0);
        }
#endif

        return va;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID       *buffers,
    int32_t          buffersNum)
{
    DDI_CODEC_FUNC_ENTER;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_RENDERPICTURE_START eventData;
        eventData.buffers = buffers;
        MOS_TraceEvent(EVENT_DECODE_DDI_RENDERPICTUREVA, EVENT_TYPE_START, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_DDI);

    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx in decode RenderPicture", VA_STATUS_ERROR_INVALID_CONTEXT);
    // assume the VAContextID is decoder ID
    uint32_t  ctxType = 0;
    PDDI_DECODE_CONTEXT decCtx = (decltype(decCtx))MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CODEC_CHK_NULL(decCtx, "Null decCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (decCtx->pCpDdiInterfaceNext)
    {
        DDI_CHK_RET(decCtx->pCpDdiInterfaceNext->IsAttachedSessionAlive(), "Session not alive!");
    }

    VAStatus va                     = VA_STATUS_SUCCESS;
    int32_t  priorityIndexInBuffers = -1;
    int32_t  numOfBuffers           = buffersNum;
    int32_t  priority               = 0;
    bool     updatePriority         = false;

    priorityIndexInBuffers = MediaLibvaCommonNext::GetGpuPriority(ctx, buffers, numOfBuffers, &updatePriority, &priority);
    if (priorityIndexInBuffers != -1)
    {
        if (updatePriority)
        {
            va = SetGpuPriority(ctx, decCtx, priority);
            if (va != VA_STATUS_SUCCESS)
            {
                return va;
            }
        }
        MediaLibvaCommonNext::MovePriorityBufferIdToEnd(buffers, priorityIndexInBuffers, numOfBuffers);
        numOfBuffers--;
    }

    if (numOfBuffers == 0)
    {
        return va;
    }

    if (decCtx->m_ddiDecodeNext)
    {
        va = decCtx->m_ddiDecodeNext->RenderPicture(ctx, context, buffers, numOfBuffers);

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
        {
            DECODE_EVENTDATA_VA_RENDERPICTURE eventData;
            eventData.buffers = buffers;
            eventData.hRes    = va;
            eventData.numBuffers = buffersNum;
            MOS_TraceEvent(EVENT_DECODE_DDI_RENDERPICTUREVA, EVENT_TYPE_END, &eventData, sizeof(eventData), NULL, 0);
        }
#endif

        return va;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::EndPicture(
    VADriverContextP ctx,
    VAContextID      context)
{
    DDI_CODEC_FUNC_ENTER;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_ENDPICTURE_START eventData;
        eventData.FrameIndex = DecodeFrameIndex;
        MOS_TraceEvent(EVENT_DECODE_DDI_ENDPICTUREVA, EVENT_TYPE_START, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_DDI);

    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx in Decode EndPicture", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t ctxType = 0;
    PDDI_DECODE_CONTEXT decCtx = (decltype(decCtx))MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CODEC_CHK_NULL(decCtx, "Null decCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (decCtx->pCpDdiInterfaceNext)
    {
        DDI_CHK_RET(decCtx->pCpDdiInterfaceNext->IsAttachedSessionAlive(), "Session not alive!");

        if (decCtx->pCpDdiInterfaceNext->IsCencProcessing())
        {
            VAStatus va = decCtx->pCpDdiInterfaceNext->EndPicture(ctx, context);

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
            {
                DECODE_EVENTDATA_VA_ENDPICTURE eventData;
                eventData.FrameIndex = DecodeFrameIndex;
                eventData.hRes       = va;
                MOS_TraceEvent(EVENT_DECODE_DDI_ENDPICTUREVA, EVENT_TYPE_END, &eventData, sizeof(eventData), NULL, 0);
                DecodeFrameIndex++;
            }
#endif

            return va;
        }
    }

    if (decCtx->m_ddiDecodeNext)
    {
        VAStatus va = decCtx->m_ddiDecodeNext->EndPicture(ctx, context);

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
        {
            DECODE_EVENTDATA_VA_ENDPICTURE eventData;
            eventData.FrameIndex = DecodeFrameIndex;
            eventData.hRes       = va;
            MOS_TraceEvent(EVENT_DECODE_DDI_ENDPICTUREVA, EVENT_TYPE_END, &eventData, sizeof(eventData), NULL, 0);
            DecodeFrameIndex++;
        }
#endif

        return va;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::QueryVideoProcPipelineCaps(
    VADriverContextP   ctx,
    VAContextID        context,
    VABufferID         *filters,
    uint32_t           filtersNum,
    VAProcPipelineCaps *pipelineCaps)
{
    DDI_CODEC_FUNC_ENTER;

    PDDI_MEDIA_CONTEXT mediaCtx = nullptr;

    DDI_CODEC_CHK_NULL(ctx,          "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(pipelineCaps, "nullptr pipelineCaps", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (filtersNum > 0)
    {
        DDI_CODEC_CHK_NULL(filters, "nullptr filters", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pipelineCaps->pipeline_flags             = VA_PROC_PIPELINE_FAST;
    pipelineCaps->filter_flags               = 0;
    pipelineCaps->rotation_flags             = (1 << VA_ROTATION_NONE) | (1 << VA_ROTATION_90) | (1 << VA_ROTATION_180) | (1 << VA_ROTATION_270);
    pipelineCaps->mirror_flags               = VA_MIRROR_HORIZONTAL  | VA_MIRROR_VERTICAL;
    pipelineCaps->blend_flags                = VA_BLEND_GLOBAL_ALPHA | VA_BLEND_PREMULTIPLIED_ALPHA | VA_BLEND_LUMA_KEY;
    pipelineCaps->num_forward_references     = DDI_CODEC_NUM_FWD_REF;
    pipelineCaps->num_backward_references    = DDI_CODEC_NUM_BK_REF;
    pipelineCaps->input_color_standards      = const_cast<VAProcColorStandardType*>(m_vpInputColorStd);
    pipelineCaps->num_input_color_standards  = DDI_VP_NUM_INPUT_COLOR_STD;
    pipelineCaps->output_color_standards     = const_cast<VAProcColorStandardType*>(m_vpOutputColorStd);
    pipelineCaps->num_output_color_standards = DDI_VP_NUM_OUT_COLOR_STD;

    pipelineCaps->num_input_pixel_formats    = 1;
    pipelineCaps->input_pixel_format[0]      = VA_FOURCC_NV12;
    pipelineCaps->num_output_pixel_formats   = 1;
    pipelineCaps->output_pixel_format[0]     = VA_FOURCC_NV12;

    if ((MEDIA_IS_SKU(&(mediaCtx->SkuTable), FtrHCP2SFCPipe)))
    {
        pipelineCaps->max_input_width        = DDI_DECODE_HCP_SFC_MAX_WIDTH;
        pipelineCaps->max_input_height       = DDI_DECODE_HCP_SFC_MAX_HEIGHT;
    }
    else
    {
        pipelineCaps->max_input_width        = DDI_DECODE_SFC_MAX_WIDTH;
        pipelineCaps->max_input_height       = DDI_DECODE_SFC_MAX_HEIGHT;
    }
    pipelineCaps->min_input_width            = DDI_DECODE_SFC_MIN_WIDTH;
    pipelineCaps->min_input_height           = DDI_DECODE_SFC_MIN_HEIGHT;
    pipelineCaps->max_output_width           = DDI_DECODE_SFC_MAX_WIDTH;
    pipelineCaps->max_output_height          = DDI_DECODE_SFC_MAX_HEIGHT;
    pipelineCaps->min_output_width           = DDI_DECODE_SFC_MIN_WIDTH;
    pipelineCaps->min_output_height          = DDI_DECODE_SFC_MIN_HEIGHT;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::MapBufferInternal(
    DDI_MEDIA_CONTEXT *mediaCtx,
    VABufferID        buf_id,
    void              **pbuf,
    uint32_t          flag)
{
    DDI_CODEC_FUNC_ENTER;

    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_START, &buf_id, sizeof(buf_id), &flag, sizeof(flag));

    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx in Decode MapBufferInternal", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_LESS((uint32_t)buf_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufferId", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_MEDIA_BUFFER *buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, buf_id);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    // The context is nullptr when the buffer is created from DeriveImage
    // So doesn't need to check the context for all cases
    // Only check the context in dec/enc mode
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    uint32_t ctxType  = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, buf_id);
    void    *ctxPtr   = MediaLibvaCommonNext::GetCtxFromVABufferID(mediaCtx, buf_id);
    DDI_CODEC_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = nullptr;
    PDDI_DECODE_CONTEXT decCtx = nullptr;

    switch (ctxType)
    {
    case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        decCtx = (decltype(decCtx)) GetDecContextFromPVOID(ctxPtr);
        bufMgr = &(decCtx->BufMgr);
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
        if(decCtx != nullptr)
        {
            switch (decCtx->wMode)
            {
            case CODECHAL_DECODE_MODE_AVCVLD:
                if (decCtx->bShortFormatInUse)
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
                if (decCtx->bShortFormatInUse)
                    *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC) + buf->uiOffset);
                else
                {
                    if (!decCtx->m_ddiDecodeNext->IsRextProfile())
                        *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC) + buf->uiOffset);
                    else
                        *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext) + buf->uiOffset);
                }
                break;
            case CODECHAL_DECODE_MODE_VP9VLD:
                *pbuf = (void *)((uint8_t*)(bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9) + buf->uiOffset);
                break;
            case CODECHAL_DECODE_MODE_AV1VLD:
            case CODECHAL_DECODE_MODE_RESERVED0:
                *pbuf = (void *)((uint8_t*)(bufMgr->pCodecSlcParamReserved) + buf->uiOffset);
                break;
            default:
                break;
            }
        }
        break;

    case VADecodeStreamoutBufferType:
        if (buf->bo)
        {
            uint32_t timeout_NS = 100000000;
            while (0 != mos_bo_wait(buf->bo, timeout_NS))
            {
                // Just loop while gem_bo_wait times-out.
            }
            *pbuf = MediaLibvaUtilNext::LockBuffer(buf, flag);
        }
        break;

    case VAStatsStatisticsParameterBufferType:
        *pbuf = (void *)(buf->pData + buf->uiOffset);
        break;

    case VAProbabilityBufferType:
        *pbuf = (void *)(buf->pData + buf->uiOffset);
        break;

    case VAImageBufferType:
    default:
        if ((buf->format != Media_Format_CPU) && (MediaLibvaInterfaceNext::MediaFormatToOsFormat(buf->format) != VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT))
        {
            MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
            // A critical section starts.
            // Make sure not to bailout with a return until the section ends.
            if (nullptr != buf->pSurface && Media_Format_CPU != buf->format)
            {
                vaStatus = MediaLibvaInterfaceNext::MediaMemoryDecompress(mediaCtx, buf->pSurface);
            }

            if (VA_STATUS_SUCCESS == vaStatus)
            {
                *pbuf = MediaLibvaUtilNext::LockBuffer(buf, flag);
                if (nullptr == *pbuf)
                {
                    vaStatus = VA_STATUS_ERROR_OPERATION_FAILED;
                }
            }

            // The critical section ends.
            MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
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

VAStatus DdiDecodeFunctions::UnmapBuffer(
    DDI_MEDIA_CONTEXT *mediaCtx,
    VABufferID        buf_id)
{
    DDI_CODEC_FUNC_ENTER;

    MOS_TraceEventExt(EVENT_VA_UNMAP, EVENT_TYPE_START, &buf_id, sizeof(buf_id), nullptr, 0);

    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx in Decode UnmapBuffer", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_LESS((uint32_t)buf_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufferId", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_MEDIA_BUFFER *buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, buf_id);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    // The context is nullptr when the buffer is created from DeriveImage
    // So doesn't need to check the context for all cases
    // Only check the context in dec/enc mode
    void *ctxPtr = MediaLibvaCommonNext::GetCtxFromVABufferID(mediaCtx, buf_id);;
    DDI_CODEC_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);
    uint32_t ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, buf_id);
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = nullptr;
    PDDI_DECODE_CONTEXT decCtx = nullptr;

    switch (ctxType)
    {
    case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        decCtx = (decltype(decCtx)) GetDecContextFromPVOID(ctxPtr);
        bufMgr = &(decCtx->BufMgr);
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
    case VADecodeStreamoutBufferType:
        if (buf->bo)
        {
            MediaLibvaUtilNext::UnlockBuffer(buf);
        }
        break;
    case VAImageBufferType:
    default:
        if ((buf->format != Media_Format_CPU) && (MediaLibvaInterfaceNext::MediaFormatToOsFormat(buf->format) != VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT))
        {
            MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
            MediaLibvaUtilNext::UnlockBuffer(buf);
            MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
        }
        break;
    }

    MOS_TraceEventExt(EVENT_VA_UNMAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::DestroyBuffer(
    DDI_MEDIA_CONTEXT *mediaCtx,
    VABufferID        buffer_id)
{
    DDI_CODEC_FUNC_ENTER;

    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_START, &buffer_id, sizeof(buffer_id), nullptr, 0);

    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx in Decode DestroyBuffer", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_LESS((uint32_t)buffer_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufferId", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_MEDIA_BUFFER *buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, buffer_id);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    void    *ctxPtr  = MediaLibvaCommonNext::GetCtxFromVABufferID(mediaCtx, buffer_id);
    uint32_t ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, buffer_id);
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = nullptr;
    PDDI_DECODE_CONTEXT decCtx = nullptr;

    switch (ctxType)
    {
    case DDI_MEDIA_CONTEXT_TYPE_DECODER:
        decCtx = (decltype(decCtx)) GetDecContextFromPVOID(ctxPtr);
        bufMgr = &(decCtx->BufMgr);
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
        ReleaseBsBuffer(bufMgr, buf);
        break;
    case VABitPlaneBufferType:
        ReleaseBpBuffer(bufMgr, buf);
        break;
    case VAProbabilityBufferType:
        ReleaseBpBuffer(bufMgr, buf);
        break;
    case VASliceParameterBufferType:
        ReleaseSliceControlBuffer(ctxType, ctxPtr, buf);
        break;
    case VAPictureParameterBufferType:
        break;
    case VADecodeStreamoutBufferType:
        MediaLibvaUtilNext::FreeBuffer(buf);
        break;
    case VAImageBufferType:
        if (buf->format == Media_Format_CPU)
        {
            MOS_FreeMemory(buf->pData);
        }
        else
        {
            MediaLibvaUtilNext::UnRefBufObjInMediaBuffer(buf);
            if (buf->uiExportcount)
            {
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
        MOS_FreeMemory(buf->pData);
        break;
    default:
        MOS_FreeMemory(buf->pData);
        break;
    }
    MOS_FreeMemory(buf);

    MediaLibvaInterfaceNext::DestroyBufFromVABufferID(mediaCtx, buffer_id);
    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeFunctions::StatusCheck(
    PDDI_MEDIA_CONTEXT mediaCtx,
    DDI_MEDIA_SURFACE *surface,
    VASurfaceID        surfaceId)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx in Decode StatusCheck", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(surface,  "nullptr surface in Decode StatusCheck", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t i = 0;
    PDDI_DECODE_CONTEXT decCtx = (decltype(decCtx))surface->pDecCtx;
    MediaLibvaUtilNext_LockGuard guard(&mediaCtx->SurfaceMutex);

    Codechal *codecHal = decCtx->pCodecHal;
    // return success just avoid vaDestroyContext is ahead of vaSyncSurface
    DDI_CODEC_CHK_NULL(codecHal, "nullptr decCtx->pCodecHal", VA_STATUS_SUCCESS);

    // return success just avoid vaDestroyContext is ahead of vaSyncSurface
    DecodePipelineAdapter *decoder = dynamic_cast<DecodePipelineAdapter *>(codecHal);
    DDI_CODEC_CHK_NULL(decoder, "nullptr (DecodePipelineAdapter *decoder)", VA_STATUS_SUCCESS);
    return StatusReport(mediaCtx, decoder, surface);
}

VAStatus DdiDecodeFunctions::StatusReport(
    PDDI_MEDIA_CONTEXT    mediaCtx,
    DecodePipelineAdapter *decoder,
    DDI_MEDIA_SURFACE     *surface)
{
    DDI_CODEC_FUNC_ENTER;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        MOS_TraceEvent(EVENT_DECODE_DDI_STATUSREPORTVA, EVENT_TYPE_START, NULL, 0, NULL, 0);
    }
#endif

    if (surface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUERY_STATE_PENDING) // TODO: Use soltlet class
    {
        uint32_t uNumCompletedReport = decoder->GetCompletedReport();
        if (uNumCompletedReport != 0)
        {
            DDI_CODEC_CHK_CONDITION((uNumCompletedReport == 0), "No report available at all", VA_STATUS_ERROR_OPERATION_FAILED);

            for (uint32_t i = 0; i < uNumCompletedReport; i++)
            {
                DecodeStatusReportData tempNewReport;
                MOS_ZeroMemory(&tempNewReport, sizeof(CodechalDecodeStatusReport));
                MOS_STATUS eStatus = decoder->GetStatusReport(&tempNewReport, 1);
                DDI_CODEC_CHK_CONDITION(MOS_STATUS_SUCCESS != eStatus, "Get status report fail", VA_STATUS_ERROR_OPERATION_FAILED);

                MOS_LINUX_BO *bo = tempNewReport.currDecodedPicRes.bo;

                if ((tempNewReport.codecStatus == CODECHAL_STATUS_SUCCESSFUL)   ||
                    (tempNewReport.codecStatus == CODECHAL_STATUS_ERROR)        ||
                    (tempNewReport.codecStatus == CODECHAL_STATUS_RESET)        ||
                    (tempNewReport.codecStatus == CODECHAL_STATUS_INCOMPLETE))
                {
                    PDDI_MEDIA_SURFACE_HEAP_ELEMENT mediaSurfaceHeapElmt = (PDDI_MEDIA_SURFACE_HEAP_ELEMENT)mediaCtx->pSurfaceHeap->pHeapBase;

                    uint32_t j = 0;
                    for (j = 0; j < mediaCtx->pSurfaceHeap->uiAllocatedHeapElements && mediaSurfaceHeapElmt != nullptr; j++, mediaSurfaceHeapElmt++)
                    {
                        if (mediaSurfaceHeapElmt->pSurface != nullptr && bo == mediaSurfaceHeapElmt->pSurface->bo)
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
        // The surface is not busy in HW, but uNumCompletedReport is 0, treat as engine reset 
        else
        {
            surface->curStatusReport.decode.status = CODECHAL_STATUS_INCOMPLETE;
            surface->curStatusReportQueryState = DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED;
            DDI_ASSERTMESSAGE("No report available at all! Engine reset may have occured.");
        }
    }

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        MOS_TraceEvent(EVENT_DECODE_DDI_STATUSREPORTVA, EVENT_TYPE_END, NULL, 0, NULL, 0);
    }
#endif

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
        else if (surface->curStatusReport.decode.status == CODECHAL_STATUS_INCOMPLETE   ||
                 surface->curStatusReport.decode.status == CODECHAL_STATUS_RESET        ||
                 surface->curStatusReport.decode.status == CODECHAL_STATUS_UNAVAILABLE)
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

VAStatus DdiDecodeFunctions::QuerySurfaceError(
    VADriverContextP ctx,
    VASurfaceID      renderTarget,
    VAStatus         errorStatus,
    void             **errorInfo)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_UNUSED(errorStatus);

    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx in Decode QuerySurfaceError", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_SURFACE *surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, renderTarget);
    DDI_CODEC_CHK_NULL(surface, "nullptr surface", VA_STATUS_ERROR_INVALID_SURFACE);


    PDDI_DECODE_CONTEXT decCtx = (decltype(decCtx))surface->pDecCtx;
    DDI_CODEC_CHK_NULL(decCtx, "nullptr surface->pDecCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    VASurfaceDecodeMBErrors *surfaceErrors = decCtx->vaSurfDecErrOutput;
    DDI_CODEC_CHK_NULL(surfaceErrors , "nullptr surfaceErrors", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = VA_STATUS_SUCCESS;

    MosUtilities::MosLockMutex(&mediaCtx->SurfaceMutex);
    if (surface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED)
    {
        if (errorStatus != -1 && surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_DECODER)
        {
            if(surface->curStatusReport.decode.status == CODECHAL_STATUS_ERROR  ||
               surface->curStatusReport.decode.status == CODECHAL_STATUS_RESET)
            {
                surfaceErrors[1].status            = -1;
                surfaceErrors[0].status            = 1;
                surfaceErrors[0].start_mb          = 0;
                surfaceErrors[0].end_mb            = 0;
                surfaceErrors[0].num_mb            = surface->curStatusReport.decode.errMbNum;
#if VA_CHECK_VERSION(1, 20, 0)
                surfaceErrors[0].decode_error_type = (surface->curStatusReport.decode.status == CODECHAL_STATUS_RESET) ? VADecodeReset : VADecodeMBError;
#else
                surfaceErrors[0].decode_error_type = VADecodeMBError;
#endif
                *errorInfo = surfaceErrors;
                MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
                return VA_STATUS_SUCCESS;
            }
#if VA_CHECK_VERSION(1, 20, 0)
            else if (surface->curStatusReport.decode.status == CODECHAL_STATUS_INCOMPLETE  ||
                     surface->curStatusReport.decode.status == CODECHAL_STATUS_UNAVAILABLE)
            {
                MOS_ZeroMemory(&surfaceErrors[0], sizeof(VASurfaceDecodeMBErrors));
                surfaceErrors[1].status            = -1;
                surfaceErrors[0].status            = 1;
                surfaceErrors[0].decode_error_type = VADecodeReset;
                *errorInfo                         = surfaceErrors;
                MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
                return VA_STATUS_SUCCESS;
            }
#endif
        }

        if (errorStatus == -1 && surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_DECODER)
            //&& surface->curStatusReport.decode.status == CODECHAL_STATUS_SUCCESSFUL) // get the crc value whatever the status is
        {
            if (nullptr == decCtx->m_ddiDecodeNext)
            {
                DDI_CODEC_ASSERTMESSAGE("nullptr decCtx->m_ddiDecodeNext");
                vaStatus = VA_STATUS_ERROR_INVALID_CONTEXT;
            }
            else
            {
                if (decCtx->m_ddiDecodeNext->GetStandard() != CODECHAL_AVC)
                {
                    vaStatus = VA_STATUS_ERROR_UNIMPLEMENTED;
                }
                else
                {
                    *errorInfo = (void *)&surface->curStatusReport.decode.crcValue;
                }
            }

            MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
            return vaStatus;
        }

        if (surface->curCtxType == DDI_MEDIA_CONTEXT_TYPE_VP &&
            surface->curStatusReport.vpp.status == CODECHAL_STATUS_ERROR)
        {
            MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
            return VA_STATUS_SUCCESS;
        }
    }

    surfaceErrors[0].status = -1;
    MosUtilities::MosUnlockMutex(&mediaCtx->SurfaceMutex);
    return VA_STATUS_SUCCESS;
}

int32_t DdiDecodeFunctions::GetDisplayInfo(VADriverContextP ctx)
{
    DDI_CODEC_FUNC_ENTER;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        MOS_TraceEvent(EVENT_DECODE_DDI_DISPLAYINFOVA, EVENT_TYPE_START, NULL, 0, NULL, 0);
    }
#endif

    PDDI_MEDIA_CONTEXT mediaDrvCtx = GetMediaContext(ctx);
    int32_t fd  = -1;
    struct fb_var_screeninfo vsinfo;
    vsinfo.xres = 0;
    vsinfo.yres = 0;

    fd = open("/dev/graphics/fb0",O_RDONLY);
    if (fd >= 0)
    {
        if (ioctl(fd, FBIOGET_VSCREENINFO, &vsinfo) < 0)
        {
            DDI_CODEC_NORMALMESSAGE("ioctl: fail to get display information!\n");
        }
        close(fd);
    }
    else
    {
        DDI_CODEC_NORMALMESSAGE("GetDisplayInfo: cannot open device!\n");
    }

    if (vsinfo.xres <= 0 || vsinfo.yres <= 0)
    {
        vsinfo.xres = 1280;
        vsinfo.yres = 720;
    }
    mediaDrvCtx->uiDisplayWidth  = vsinfo.xres;
    mediaDrvCtx->uiDisplayHeight = vsinfo.yres;

    DDI_CODEC_NORMALMESSAGE("DDI:mediaDrvCtx->uiDisplayWidth =%d", mediaDrvCtx->uiDisplayWidth);
    DDI_CODEC_NORMALMESSAGE("DDI:mediaDrvCtx->uiDisplayHeight =%d",mediaDrvCtx->uiDisplayHeight);

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_DISPLAYINFO eventData;
        eventData.uiDisplayWidth  = mediaDrvCtx->uiDisplayWidth;
        eventData.uiDisplayHeight = mediaDrvCtx->uiDisplayHeight;
        MOS_TraceEvent(EVENT_DECODE_DDI_DISPLAYINFOVA, EVENT_TYPE_END, &eventData, sizeof(eventData), NULL, 0);
    }
#endif

    return 0;
}

void DdiDecodeFunctions::CleanUp(
    VADriverContextP    ctx,
    PDDI_DECODE_CONTEXT decCtx)
{
    DDI_CODEC_FUNC_ENTER;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        MOS_TraceEvent(EVENT_DECODE_DDI_CLEARUPVA, EVENT_TYPE_START, NULL, 0, NULL, 0);
    }
#endif

    if (decCtx)
    {
        if (decCtx->m_ddiDecodeNext)
        {
            decCtx->m_ddiDecodeNext->DestroyContext(ctx);
            MOS_Delete(decCtx->m_ddiDecodeNext);
            MOS_FreeMemory(decCtx);
            decCtx = nullptr;
        }
    }

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        MOS_TraceEvent(EVENT_DECODE_DDI_CLEARUPVA, EVENT_TYPE_END, NULL, 0, NULL, 0);
    }
#endif

    return;
}

VAStatus DdiDecodeFunctions::SetGpuPriority(
    VADriverContextP    ctx,
    PDDI_DECODE_CONTEXT decCtx,
    int32_t             priority)
{
    DDI_CODEC_FUNC_ENTER;

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        MOS_TraceEvent(EVENT_DECODE_DDI_SETGPUPRIORITYVA, EVENT_TYPE_START, NULL, 0, NULL, 0);
    }
#endif

    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx in Decode SetGpuPriority", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(decCtx, "nullptr decCtx in Decode SetGpuPriority", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    // Set the priority for Gpu
    if (decCtx->pCodecHal != nullptr)
    {
        PMOS_INTERFACE osInterface = decCtx->pCodecHal->GetOsInterface();
        DDI_CODEC_CHK_NULL(osInterface, "nullptr osInterface.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        osInterface->pfnSetGpuPriority(osInterface, priority);
    }

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        MOS_TraceEvent(EVENT_DECODE_DDI_SETGPUPRIORITYVA, EVENT_TYPE_END, NULL, 0, NULL, 0);
    }
#endif

    return VA_STATUS_SUCCESS;
}

bool DdiDecodeFunctions::ReleaseBsBuffer(DDI_CODEC_COM_BUFFER_MGR *bufMgr, DDI_MEDIA_BUFFER *buf)
{
    DDI_CODEC_FUNC_ENTER;

    if ((nullptr == bufMgr) || (nullptr == buf))
    {
        return true;
    }

    if (Media_Format_CPU == buf->format)
    {
        for (uint32_t i = 0; i < bufMgr->dwNumSliceData; i++)
        {
            if (bufMgr->pSliceData[i].pBaseAddress == buf->pData)
            {
                MediaLibvaUtilNext::FreeBuffer(buf);
                bufMgr->pSliceData[i].pBaseAddress = nullptr;
                if (nullptr != bufMgr->pSliceData[i].pMappedGPUBuffer)
                {
                    MediaLibvaUtilNext::UnlockBuffer(bufMgr->pSliceData[i].pMappedGPUBuffer);
                    if (false == bufMgr->pSliceData[i].pMappedGPUBuffer->bMapped)
                    {
                        MediaLibvaUtilNext::FreeBuffer(bufMgr->pSliceData[i].pMappedGPUBuffer);
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

bool DdiDecodeFunctions::ReleaseBpBuffer(
    DDI_CODEC_COM_BUFFER_MGR *bufMgr,
    DDI_MEDIA_BUFFER         *buf)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_UNUSED(bufMgr);
    DDI_UNUSED(buf);
    return true;
}

bool DdiDecodeFunctions::ReleaseSliceControlBuffer(uint32_t ctxType, void *ctx, DDI_MEDIA_BUFFER *buf)
{
    DDI_CODEC_FUNC_ENTER;

    PDDI_DECODE_CONTEXT      decCtx  = (decltype(decCtx)) GetDecContextFromPVOID(ctx);
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(decCtx->BufMgr);
    switch (decCtx->wMode)
    {
    case CODECHAL_DECODE_MODE_AVCVLD:
        if (decCtx->bShortFormatInUse)
        {
            if (nullptr == bufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264Base)
            {
                return false;
            }
        }
        else
        {
            if (nullptr == bufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264)
            {
                return false;
            }
        }
        break;
    case CODECHAL_DECODE_MODE_MPEG2VLD:
        if (nullptr == bufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2)
        {
            return false;
        }
        break;
    case CODECHAL_DECODE_MODE_VC1VLD:
        if (nullptr == bufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1)
        {
            return false;
        }
        break;
    case CODECHAL_DECODE_MODE_JPEG:
        if (nullptr == bufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG)
        {
            return false;
        }
        break;
    case CODECHAL_DECODE_MODE_VP8VLD:
        if (nullptr == bufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8)
        {
            return false;
        }
        break;
    case CODECHAL_DECODE_MODE_HEVCVLD:
        if (decCtx->bShortFormatInUse)
        {
            if (nullptr == bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC)
            {
                return false;
            }
        }
        else
        {
            if (nullptr == bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC &&
                nullptr == bufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVCRext)
            {
                return false;
            }
        }
        break;
    case CODECHAL_DECODE_MODE_VP9VLD:
        if (nullptr == bufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9)
        {
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}
