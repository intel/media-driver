/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     ddi_vp_functions.cpp
//! \brief    ddi vp functions implementaion.
//!
#include "ddi_vp_functions.h"
#include "media_libva_util_next.h"
#include "media_libva_common_next.h"
#include "media_libva_caps_next.h"
#include "ddi_vp_tools.h"
#include "mos_solo_generic.h"
#include "media_libva_interface_next.h"
#include "ddi_cp_interface_next.h"
#include "vp_utils.h"
#if !defined(ANDROID) && defined(X11_FOUND)
#include <X11/Xutil.h>
#include "media_libva_putsurface_linux.h"
#endif

#define VP_SETTING_MAX_PHASES               1
#define VP_SETTING_MEDIA_STATES             32
#define VP_SETTING_SAME_SAMPLE_THRESHOLD    1000

#define VP_MAX_PIC_WIDTH                    16384
#define VP_MAX_PIC_HEIGHT                   16384
#define VP_MIN_PIC_WIDTH                    16
#define VP_MIN_PIC_HEIGHT                   16
#define QUERY_CAPS_ATTRIBUTE                1                     /* query the actual filter caps attribute in vp module */

// ITU-T H.265 Table E.3: Colour Primaries
#define COLOUR_PRIMARY_BT2020                  9
#define COLOUR_PRIMARY_BT709                   1
#define COLOUR_PRIMARY_BT601                   5
// ITU-T H.265 Table E.4 Transfer characteristics
#define TRANSFER_CHARACTERISTICS_BT709         1
#define TRANSFER_CHARACTERISTICS_GMAMA2P2      4
#define TRANSFER_CHARACTERISTICS_LINEAR        8
#define TRANSFER_CHARACTERISTICS_BT2020        14
#define TRANSFER_CHARACTERISTICS_ST2084        16

const VAProcFilterCapColorBalance DdiVpFunctions::m_vpColorBalCap[] = {
    /** \brief Hue. */
    {VAProcColorBalanceHue,
        { PROCAMP_HUE_MIN,
          PROCAMP_HUE_MAX,
          PROCAMP_HUE_DEFAULT,
          PROCAMP_HUE_STEP }
    },
    /** \brief Saturation. */
    {VAProcColorBalanceSaturation,
        { PROCAMP_SATURATION_MIN,
          PROCAMP_SATURATION_MAX,
          PROCAMP_SATURATION_DEFAULT,
          PROCAMP_SATURATION_STEP }
    },
    /** \brief Brightness. */
    {VAProcColorBalanceBrightness,
        { PROCAMP_BRIGHTNESS_MIN,
          PROCAMP_BRIGHTNESS_MAX,
          PROCAMP_BRIGHTNESS_DEFAULT,
          PROCAMP_BRIGHTNESS_STEP }
    },
    /** \brief Contrast. */
    {VAProcColorBalanceContrast,
        { PROCAMP_CONTRAST_MIN,
          PROCAMP_CONTRAST_MAX,
          PROCAMP_CONTRAST_DEFAULT,
          PROCAMP_CONTRAST_STEP }
    },
    /** \brief Automatically adjusted contrast. */
    {VAProcColorBalanceAutoContrast,
        { 0.0F, 0.0F, 0.0F, 0.0F }
    }
};

const VAProcFilterType DdiVpFunctions::m_vpSupportedFilters[DDI_VP_MAX_NUM_FILTERS] = {
    VAProcFilterNoiseReduction,
    VAProcFilterDeinterlacing,
    VAProcFilterSharpening,
    VAProcFilterColorBalance,
    VAProcFilterSkinToneEnhancement,
    VAProcFilterTotalColorCorrection,
    VAProcFilterHVSNoiseReduction,
    VAProcFilterHighDynamicRangeToneMapping,
#if VA_CHECK_VERSION(1, 12, 0)
    VAProcFilter3DLUT
#endif
};

VAStatus DdiVpFunctions::CreateContext (
    VADriverContextP  ctx,
    VAConfigID        configId,
    int32_t           pictureWidth,
    int32_t           pictureHeight,
    int32_t           flag,
    VASurfaceID       *renderTargets,
    int32_t           renderTargetsNum,
    VAContextID       *ctxID)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    VAStatus                          status        = VA_STATUS_SUCCESS;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaCtxHeapElmt = nullptr;
    PDDI_MEDIA_CONTEXT                mediaCtx      = nullptr;
    PDDI_VP_CONTEXT                   vpCtx         = nullptr;
    DDI_VP_FUNC_ENTER;

    DDI_UNUSED(configId);
    DDI_UNUSED(pictureWidth);
    DDI_UNUSED(pictureHeight);
    DDI_UNUSED(flag);
    DDI_UNUSED(renderTargets);
    DDI_UNUSED(renderTargetsNum);

    DDI_VP_CHK_NULL(ctx, "nullptr ctx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    *ctxID = VA_INVALID_ID;

    mediaCtx = GetMediaContext(ctx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // allocate vpCtx
    vpCtx = MOS_New(DDI_VP_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // init vpCtx
    status = DdiInitCtx(ctx, vpCtx);
    DDI_CHK_RET(status, "VA_STATUS_ERROR_OPERATION_FAILED");

    MosUtilities::MosLockMutex(&mediaCtx->VpMutex);

    // get Free VP context index
    vaCtxHeapElmt = MediaLibvaUtilNext::DdiAllocPVAContextFromHeap(mediaCtx->pVpCtxHeap);
    if (nullptr == vaCtxHeapElmt)
    {
        MOS_Delete(vpCtx);
        MosUtilities::MosUnlockMutex(&mediaCtx->VpMutex);
        DDI_VP_ASSERTMESSAGE("VP Context number exceeds maximum.");
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    // store vpCtx in pMedia
    vaCtxHeapElmt->pVaContext = (void *)vpCtx;
    *ctxID = (VAContextID)(vaCtxHeapElmt->uiVaContextID + DDI_MEDIA_SOFTLET_VACONTEXTID_VP_OFFSET);

    // increate VP context number
    mediaCtx->uiNumVPs++;

    MosUtilities::MosUnlockMutex(&mediaCtx->VpMutex);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DestroyContext (
    VADriverContextP  ctx,
    VAContextID       ctxID)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);
    VAStatus                 vaStatus  = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT       mediaCtx  = nullptr;
    PDDI_VP_CONTEXT          vpCtx     = nullptr;
    uint32_t                 vpIndex   = 0;
    uint32_t                 ctxType   = DDI_MEDIA_CONTEXT_TYPE_NONE;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(ctx, "nullptr ctx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    mediaCtx = GetMediaContext(ctx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    vpCtx    = (PDDI_VP_CONTEXT)MediaLibvaCommonNext::GetContextFromContextID(ctx, ctxID, &ctxType);
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    MOS_FreeMemory(vpCtx->MosDrvCtx.pPerfData);
    vpCtx->MosDrvCtx.pPerfData = nullptr;

    if (vpCtx->pCpDdiInterfaceNext)
    {
        MOS_Delete(vpCtx->pCpDdiInterfaceNext);
    }

    // destroy vphal
    vaStatus  = DdiDestroyVpHal(vpCtx);

    // Get VP context index
    vpIndex = ctxID & DDI_MEDIA_MASK_VACONTEXTID;

    // remove from context array
    MosUtilities::MosLockMutex(&mediaCtx->VpMutex);
    // destroy vp context
    MOS_Delete(vpCtx);
    MediaLibvaUtilNext::DdiReleasePVAContextFromHeap(mediaCtx->pVpCtxHeap, vpIndex);

    mediaCtx->uiNumVPs--;
    MosUtilities::MosUnlockMutex(&mediaCtx->VpMutex);

    return vaStatus;
}

VAStatus DdiVpFunctions::CreateBuffer (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferType      type,
    uint32_t          size,
    uint32_t          elementsNum,
    void              *data,
    VABufferID        *bufId)
{
    VAStatus                       vaStatus          = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT             mediaCtx          = nullptr;
    PDDI_MEDIA_BUFFER              buf               = nullptr;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT bufferHeapElement = nullptr;
    MOS_STATUS                     eStatus           = MOS_STATUS_SUCCESS;
    PDDI_VP_CONTEXT                vpContext         = nullptr;
    uint32_t                       ctxType           = DDI_MEDIA_CONTEXT_TYPE_NONE;
    void                           *ctxPtr           = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(ctx,  "nullptr ctx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    *bufId = VA_INVALID_ID;

    mediaCtx = GetMediaContext(ctx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    ctxPtr  = MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_VP_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);

    vpContext = (PDDI_VP_CONTEXT)ctxPtr;

    // only for VAProcFilterParameterBufferType and VAProcPipelineParameterBufferType
    if (type != VAProcFilterParameterBufferType && type != VAProcPipelineParameterBufferType
#if VA_CHECK_VERSION(1, 10, 0)
        && type != VAContextParameterUpdateBufferType
#endif
    )
    {
        DDI_VP_ASSERTMESSAGE("Unsupported Va Buffer Type.");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // allocate new buf and init
    buf = MOS_New(DDI_MEDIA_BUFFER);
    DDI_VP_CHK_NULL(buf, "nullptr buf.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    buf->pMediaCtx      = mediaCtx;
    buf->iSize          = size * elementsNum;
    buf->uiNumElements  = elementsNum;
    buf->uiType         = type;
    buf->format         = Media_Format_Buffer;
    buf->uiOffset       = 0;
    buf->pData          = MOS_NewArray(uint8_t, size * elementsNum);
    if (nullptr == buf->pData)
    {
        MOS_Delete(buf);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    buf->format = Media_Format_CPU;

    bufferHeapElement = MediaLibvaUtilNext::AllocPMediaBufferFromHeap(mediaCtx->pBufferHeap);
    if (nullptr == bufferHeapElement)
    {
        MOS_DeleteArray(buf->pData);
        MOS_Delete(buf);
        DDI_VP_ASSERTMESSAGE("Invalid buffer index.");
        return VA_STATUS_ERROR_INVALID_BUFFER;
    }
    bufferHeapElement->pBuffer   = buf;
    bufferHeapElement->pCtx      = (void *)vpContext;
    bufferHeapElement->uiCtxType = DDI_MEDIA_CONTEXT_TYPE_VP;
    *bufId                       = bufferHeapElement->uiVaBufferID;
    mediaCtx->uiNumBufs++;

    // if there is data from client, then dont need to copy data from client
    if (data)
    {
        // copy client data to new buf
        eStatus = MOS_SecureMemcpy(buf->pData, size * elementsNum, data, size * elementsNum);
        DDI_VP_CHK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "DDI: Failed to copy client data!", VA_STATUS_ERROR_MAX_NUM_EXCEEDED);
    }
    else
    {
        // do nothing if there is no data from client
        vaStatus = VA_STATUS_SUCCESS;
    }

    return vaStatus;
}

VAStatus DdiVpFunctions::MapBufferInternal(
    PDDI_MEDIA_CONTEXT  mediaCtx,
    VABufferID          bufId,
    void                **buf,
    uint32_t            flag)
{
    DDI_VP_FUNC_ENTER;

    return DdiMediaFunctions::MapBufferInternal(mediaCtx, bufId, buf, flag);
}

VAStatus DdiVpFunctions::UnmapBuffer (
    PDDI_MEDIA_CONTEXT mediaCtx,
    VABufferID         bufId)
{
    DDI_VP_FUNC_ENTER;

    return DdiMediaFunctions::UnmapBuffer(mediaCtx, bufId);
}

VAStatus DdiVpFunctions::DestroyBuffer(
    DDI_MEDIA_CONTEXT  *mediaCtx,
    VABufferID         bufId)
{
    VAStatus         vaStatus  = VA_STATUS_SUCCESS;
    DDI_MEDIA_BUFFER *mediaBuf = nullptr;
    DDI_VP_FUNC_ENTER;

    mediaBuf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, bufId);
    DDI_VP_CHK_NULL(mediaBuf, "nullptr mediaBuf", VA_STATUS_ERROR_INVALID_BUFFER);

    switch ((int32_t)mediaBuf->uiType)
    {
        case VAProcPipelineParameterBufferType:
        case VAProcFilterParameterBufferType:
#if VA_CHECK_VERSION(1, 10, 0)
        case VAContextParameterUpdateBufferType:
#endif
            MOS_DeleteArray(mediaBuf->pData);
            break;
        default:
            DDI_VP_ASSERTMESSAGE("Unsupported Va Buffer Type.");
    }
    MOS_Delete(mediaBuf);

    MediaLibvaInterfaceNext::DestroyBufFromVABufferID(mediaCtx, bufId);
    return vaStatus;
}

VAStatus DdiVpFunctions::BeginPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       renderTarget)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    VAStatus             vaStatus          = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT   mediaDrvCtx       = nullptr;
    PDDI_VP_CONTEXT      vpCtx             = nullptr;
    uint32_t             ctxType           = DDI_MEDIA_CONTEXT_TYPE_NONE;
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       vpHalTgtSurf      = nullptr;
    PDDI_MEDIA_SURFACE   mediaTgtSurf      = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(ctx, "nullptr ctx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    mediaDrvCtx = GetMediaContext(ctx);
    DDI_VP_CHK_NULL(mediaDrvCtx, "nullptr mediaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    vpCtx = (PDDI_VP_CONTEXT)MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    vpCtx->TargetSurfID = renderTarget;
    vpHalRenderParams   = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    // uDstCount == 0 means no render target is set yet.
    // uDstCount == 1 means 1 render target has been set already.
    // uDstCount == 2 means 2 render targets have been set already.
    DDI_VP_CHK_LESS(vpHalRenderParams->uDstCount, VPHAL_MAX_TARGETS, "Too many render targets for VP.", VA_STATUS_ERROR_INVALID_PARAMETER);

    vpHalTgtSurf = vpHalRenderParams->pTarget[vpHalRenderParams->uDstCount];
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf.", VA_STATUS_ERROR_INVALID_SURFACE);

    mediaTgtSurf = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaDrvCtx, renderTarget);
    DDI_VP_CHK_NULL(mediaTgtSurf, "nullptr mediaTgtSurf.", VA_STATUS_ERROR_INVALID_SURFACE);
    mediaTgtSurf->pVpCtx = vpCtx;

    // Setup Target VpHal Surface
    vpHalTgtSurf->SurfType       = SURF_OUT_RENDERTARGET;
    vpHalTgtSurf->rcSrc.top      = 0;
    vpHalTgtSurf->rcSrc.left     = 0;
    vpHalTgtSurf->rcSrc.right    = mediaTgtSurf->iWidth;
    vpHalTgtSurf->rcSrc.bottom   = mediaTgtSurf->iRealHeight;
    vpHalTgtSurf->rcDst.top      = 0;
    vpHalTgtSurf->rcDst.left     = 0;
    vpHalTgtSurf->rcDst.right    = mediaTgtSurf->iWidth;
    vpHalTgtSurf->rcDst.bottom   = mediaTgtSurf->iRealHeight;
    vpHalTgtSurf->ExtendedGamut  = false;

    // Set os resource for VPHal render
    vaStatus = VpSetOsResource(vpCtx, mediaTgtSurf, vpHalRenderParams->uDstCount);
    DDI_CHK_RET(vaStatus, "Call VpSetOsResource failed");

    vpHalTgtSurf->Format   = vpHalTgtSurf->OsResource.Format;
    vpHalTgtSurf->TileType = vpHalTgtSurf->OsResource.TileType;

    VpSetRenderParams(mediaTgtSurf, vpHalRenderParams, renderTarget);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::RenderPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           buffersNum)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    VAStatus           vaStatus               = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx               = nullptr;
    PDDI_VP_CONTEXT    vpCtx                  = nullptr;
    PDDI_MEDIA_BUFFER  buf                    = nullptr;
    void               *data                  = nullptr;
    uint32_t           ctxType                = DDI_MEDIA_CONTEXT_TYPE_NONE;
    int32_t            numOfBuffers           = buffersNum;
    int32_t            priority               = 0;
    int32_t            priorityIndexInBuffers = -1;
    bool               updatePriority         = false;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(ctx, "nullptr ctx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaCtx = GetMediaContext(ctx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_OPERATION_FAILED);

    vpCtx = (PDDI_VP_CONTEXT)MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    //buffersNum check
    DDI_VP_CHK_CONDITION(((numOfBuffers > VPHAL_MAX_SOURCES) || (numOfBuffers <= 0)),
        "numOfBuffers is Invalid.",
        VA_STATUS_ERROR_INVALID_PARAMETER);

    priorityIndexInBuffers = MediaLibvaCommonNext::GetGpuPriority(ctx, buffers, numOfBuffers, &updatePriority, &priority);
    if (priorityIndexInBuffers != -1)
    {
        if (updatePriority)
        {
            if ((vaStatus = DdiSetGpuPriority(vpCtx, priority)) != VA_STATUS_SUCCESS)
            {
                return vaStatus;
            }
        }
        MediaLibvaCommonNext::MovePriorityBufferIdToEnd(buffers, priorityIndexInBuffers, numOfBuffers);
        numOfBuffers--;
    }
    if (numOfBuffers == 0)
    {
        return vaStatus;
    }

    for (int32_t i = 0; i < numOfBuffers; i++)
    {
        buf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, buffers[i]);
        DDI_VP_CHK_NULL(buf, "nullptr buf.", VA_STATUS_ERROR_INVALID_BUFFER);

        MediaLibvaInterfaceNext::MapBuffer(ctx, buffers[i], &data);
        DDI_VP_CHK_NULL(data, "nullptr data.", VA_STATUS_ERROR_INVALID_BUFFER);

        switch ((int32_t)buf->uiType)
        {
        // VP Buffer Types
        case VAProcPipelineParameterBufferType:
            if (VpIsRenderTarget(ctx, vpCtx, (VAProcPipelineParameterBuffer *)data))
            {
                vaStatus = VpSetRenderTargetParams(ctx, vpCtx, (VAProcPipelineParameterBuffer *)data);
            }
            else
            {
                vaStatus = DdiSetProcPipelineParams(ctx, vpCtx, (VAProcPipelineParameterBuffer *)data);
                DDI_CHK_RET(vaStatus, "Unable to set pipeline parameters");
            }
            break;
        case VAProcFilterParameterBufferType:
            // User is not supposed to pass this buffer type:Refer va_vpp.h
            DDI_VP_ASSERTMESSAGE("Invalid buffer type.");
            vaStatus = VA_STATUS_ERROR_INVALID_BUFFER;
            break;

        default:
            DDI_CHK_RET(vaStatus, "Unsupported buffer type!");
            break;
        }
        MediaLibvaInterfaceNext::UnmapBuffer(ctx, buffers[i]);
    }
    return vaStatus;
}

VAStatus DdiVpFunctions::EndPicture (
    VADriverContextP  ctx,
    VAContextID       context)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;
    PDDI_VP_CONTEXT vpCtx   = nullptr;
    uint32_t        ctxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    VpBase          *vpHal  = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(ctx, "nullptr ctx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    //get VP Context
    vpCtx = (PDDI_VP_CONTEXT)MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    //Add component tag for VP
    DDI_VP_CHK_NULL(vpCtx->pVpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    vpCtx->pVpHalRenderParams->Component = COMPONENT_VPCommon;

    vpHal = vpCtx->pVpHal;
    DDI_VP_CHK_NULL(vpHal, "nullptr vpHal.", VA_STATUS_ERROR_INVALID_PARAMETER);
    eStatus = vpHal->Render(vpCtx->pVpHalRenderParams);

#if (_DEBUG || _RELEASE_INTERNAL)
    DdiVpTools::VpDumpProcPipelineParams(ctx, vpCtx);
#endif  //(_DEBUG || _RELEASE_INTERNAL)

    VpReportFeatureMode(vpCtx);

    // Reset primary surface count for next render call
    vpCtx->iPriSurfs = 0;
    // Reset render target count for next render call
    vpCtx->pVpHalRenderParams->uDstCount = 0;

    if (MOS_FAILED(eStatus))
    {
        DDI_VP_ASSERTMESSAGE("Failed to call render function.");
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::CreateConfig (
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attribList,
    int32_t           attribsNum,
    VAConfigID        *configId)
{
    VAStatus status = VA_STATUS_SUCCESS;

    DDI_VP_FUNC_ENTER;
    // attribList and numAttribs are for future usage.
    DDI_UNUSED(attribList);
    DDI_UNUSED(attribsNum);
    DDI_VP_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(configId, "nullptr configId", VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_VP_CHK_NULL(mediaCtx,             "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(mediaCtx->m_capsNext->m_capsTable, "nullptr m_capsTable",   VA_STATUS_ERROR_INVALID_PARAMETER);

    status = mediaCtx->m_capsNext->CreateConfig(profile, entrypoint, attribList, attribsNum, configId);
    DDI_CHK_RET(status, "Create common config failed");

    auto configList = mediaCtx->m_capsNext->GetConfigList();
    DDI_VP_CHK_NULL(configList, "Get configList failed", VA_STATUS_ERROR_INVALID_PARAMETER);

    for(int i = 0; i < configList->size(); i++)
    {
        if((configList->at(i).profile == profile) && (configList->at(i).entrypoint == entrypoint))
        {
            uint32_t curConfigID = ADD_CONFIG_ID_VP_OFFSET(i);
            if(!mediaCtx->m_capsNext->m_capsTable->IsVpConfigId(curConfigID))
            {
                 DDI_VP_ASSERTMESSAGE("DDI: Invalid configID.");
                 return VA_STATUS_ERROR_INVALID_CONFIG;
            }

            *configId = curConfigID;
            return VA_STATUS_SUCCESS;
        }
    }

    return VA_STATUS_ERROR_ATTR_NOT_SUPPORTED;
}

VAStatus DdiVpFunctions::QueryVideoProcFilters(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  *filters,
    uint32_t          *filtersNum)
{
    uint32_t maxNumFilters       = DDI_VP_MAX_NUM_FILTERS;
    uint32_t i                   = 0;
    uint32_t supportedFiltersNum = 0;
    VAStatus vaStatus            = VA_STATUS_SUCCESS;

    DDI_VP_FUNC_ENTER;
    DDI_UNUSED(ctx);
    DDI_UNUSED(context);
    DDI_VP_CHK_NULL(filters,    "nullptr filters",    VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(filtersNum, "nullptr filtersNum", VA_STATUS_ERROR_INVALID_PARAMETER);

    // check if array size is less than VP_MAX_NUM_FILTERS
    if(*filtersNum < maxNumFilters)
    {
        DDI_VP_NORMALMESSAGE("filtersNum %d < maxNumFilters %d. Probably caused by Libva version upgrade!", *filtersNum, maxNumFilters);
    }

    // Set the filters
    while(supportedFiltersNum < *filtersNum && i < DDI_VP_MAX_NUM_FILTERS)
    {
        uint32_t filterCapsNum = 0;
        vaStatus = QueryVideoProcFilterCaps(ctx, context, m_vpSupportedFilters[i], nullptr, &filterCapsNum);
        if(vaStatus == VA_STATUS_SUCCESS && filterCapsNum != 0)
        {
            filters[supportedFiltersNum] = m_vpSupportedFilters[i];
            supportedFiltersNum++;
        }
        i++;
    }

    // Tell the app how many valid filters are filled in the array
    *filtersNum = supportedFiltersNum;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryVideoProcFilterCaps(
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  type,
    void              *filterCaps,
    uint32_t          *filterCapsNum)
{
    uint32_t                            queryCapsNum   = 0;   /* the filter caps number queried by app layer */
    uint32_t                            existCapsNum   = 0;   /* the actual number of filters in vp module */
    uint32_t                            queryFlag      = 0;   /* QUERY_CAPS_ATTRIBUTE: search caps attribute */
    PDDI_MEDIA_CONTEXT                  mediaDrvCtx    = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL (ctx,           "nullptr ctx.",           VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL (filterCapsNum, "nullptr filterCapsNum.", VA_STATUS_ERROR_INVALID_PARAMETER);

    mediaDrvCtx = GetMediaContext(ctx);
    DDI_VP_CHK_NULL (mediaDrvCtx,   "nullptr mediaDrvCtx.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (*filterCapsNum != 0)
    {
        /* return filter caps attribute  */
        queryFlag = QUERY_CAPS_ATTRIBUTE;
        /* check filter_caps pointer to fill in the filter_caps */
        DDI_VP_CHK_NULL (filterCaps, "nullptr filterCaps.", VA_STATUS_ERROR_INVALID_PARAMETER);
    }

    queryCapsNum = *filterCapsNum;

    switch (type)
    {
        /* Noise reduction filter */
        case VAProcFilterNoiseReduction:
        {
            existCapsNum   = 1;
            /* set input filter caps number to the actual number of filters in vp module */
            *filterCapsNum = existCapsNum;
            /* set the actual filter caps attribute in vp module */
            DDI_CHK_RET(QueryNoiseReductionCapsAttrib(queryFlag, queryCapsNum, existCapsNum, filterCaps), "Query Noise Reduction Caps Attribute Failed.")
            break;
        }
        /* HVS Noise reduction filter */
        case VAProcFilterHVSNoiseReduction:
        {
            existCapsNum   = 0;
            *filterCapsNum = existCapsNum;
            if (MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrHVSDenoise))
            {
                existCapsNum   = 4;
                *filterCapsNum = existCapsNum;
            }
            break;
        }
        /* Deinterlacing filter */
        case VAProcFilterDeinterlacing:
        {
            existCapsNum   = 3;
            /* set input filter caps number to the actual number of filters in vp module */
            *filterCapsNum = existCapsNum;
            /* set the actual filter caps attribute in vp module */
            DDI_CHK_RET(QueryDeinterlacingCapsAttrib(queryFlag, queryCapsNum, existCapsNum, filterCaps), "Query Deinterlacing filter Caps Attribute Failed.");
            break;
        }
        /* Sharpening filter. */
        case VAProcFilterSharpening:
        {
            existCapsNum   = 1;
            /* set input filter caps number to the actual number of filters in vp module */
            *filterCapsNum = existCapsNum;
            /* set the actual filter caps attribute in vp module */
            DDI_CHK_RET(QuerySharpeningCapsAttrib(queryFlag, queryCapsNum, existCapsNum, filterCaps), "Query Sharpening filter Cpas Attribute Failed.");
            break;
        }
        /* Color balance parameters. */
        case VAProcFilterColorBalance:
        {
            existCapsNum   = sizeof(m_vpColorBalCap)/sizeof(VAProcFilterCapColorBalance);
            /* set input filter caps number to the actual number of filters in vp module */
            *filterCapsNum = existCapsNum;
            /* set the actual filter caps attribute in vp module */
            DDI_CHK_RET(QueryColorBalanceCapsAttrib(queryFlag, queryCapsNum, existCapsNum, filterCaps), "Query Color Balance Caps Attribute Failed.");
            break;
        }
        case VAProcFilterSkinToneEnhancement:
        {
            existCapsNum   = 1;
            *filterCapsNum = existCapsNum;
            DDI_CHK_RET(QuerySkinToneEnhancementCapsAttrib(queryFlag, queryCapsNum, existCapsNum, filterCaps), "Query SkinTone Enhancement Caps Attribute Failed.");
            break;
        }
        case VAProcFilterTotalColorCorrection:
        {
            existCapsNum   = 6;
            *filterCapsNum = existCapsNum;
            DDI_CHK_RET(QueryTotalColorCorrectionCapsAttrib(queryFlag, queryCapsNum, existCapsNum, filterCaps), "Query Total Color Correction Caps Attribute Failed.");
            break;
        }
        case VAProcFilterHighDynamicRangeToneMapping:
        {
            existCapsNum   = 0;
            *filterCapsNum = existCapsNum;
            if (MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrHDR))
            {
                existCapsNum   = 1;
                *filterCapsNum = existCapsNum;
                DDI_CHK_RET(QueryHDRToneMappingCapsAttrib(queryFlag, queryCapsNum, existCapsNum, filterCaps), "Query High Dynamic Range ToneMapping Caps Attribute Failed.");
            }
            break;
        }
#if VA_CHECK_VERSION(1, 12, 0)
        case VAProcFilter3DLUT:
        {
            existCapsNum   = 0;
            *filterCapsNum = existCapsNum;
            /* 3DLUT is supported in VEBOX on Gen11+*/
            if (!MEDIA_IS_SKU(&mediaDrvCtx->SkuTable, FtrDisableVEBoxFeatures))
            {
                existCapsNum   = 3;
                *filterCapsNum = existCapsNum;
                DDI_CHK_RET(QueryLut3DCapsAttrib(queryFlag, queryCapsNum, existCapsNum, filterCaps), "Query 3Dlut Caps Attribute Failed.");
            }

            break;           
        }
#endif

        case VAProcFilterCount:
        case VAProcFilterNone:
            return VA_STATUS_ERROR_INVALID_VALUE;

        default:
            return VA_STATUS_ERROR_UNSUPPORTED_FILTER;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryVideoProcPipelineCaps(
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID          *filters,
    uint32_t            filtersNum,
    VAProcPipelineCaps  *pipelineCaps)
{
    PDDI_MEDIA_CONTEXT mediaCtx = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(ctx,          "nullptr ctx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(pipelineCaps, "nullptr pipelineCaps", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (filtersNum > 0)
    {
        DDI_VP_CHK_NULL(filters,   "nullptr filters",     VA_STATUS_ERROR_INVALID_PARAMETER);
    }
    mediaCtx   = GetMediaContext(ctx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

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

    //Capability of Gen9+ platform
    pipelineCaps->max_input_width            = VP_MAX_PIC_WIDTH;
    pipelineCaps->max_input_height           = VP_MAX_PIC_HEIGHT;
    pipelineCaps->max_output_width           = VP_MAX_PIC_WIDTH;
    pipelineCaps->max_output_height          = VP_MAX_PIC_HEIGHT;

    pipelineCaps->min_input_width            = VP_MIN_PIC_WIDTH;
    pipelineCaps->min_input_height           = VP_MIN_PIC_HEIGHT;
    pipelineCaps->min_output_width           = VP_MIN_PIC_WIDTH;
    pipelineCaps->min_output_height          = VP_MIN_PIC_WIDTH;

    
    for (int i = 0; i < filtersNum; i++) {
        void *pData;
        DdiMedia_MapBuffer(ctx, filters[i], &pData);
        DDI_CHK_NULL(pData, "nullptr pData", VA_STATUS_ERROR_INVALID_PARAMETER);
        VAProcFilterParameterBufferBase* base_param = (VAProcFilterParameterBufferBase*) pData;
        if (base_param->type == VAProcFilterDeinterlacing)
        {
            VAProcFilterParameterBufferDeinterlacing *di_param = (VAProcFilterParameterBufferDeinterlacing *)base_param;
            if (di_param->algorithm == VAProcDeinterlacingMotionAdaptive ||
                di_param->algorithm == VAProcDeinterlacingMotionCompensated)
            {
                pipelineCaps->num_forward_references = 1;
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiInitCtx(
    VADriverContextP vaDrvCtx,
    PDDI_VP_CONTEXT  vpCtx)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);
    VAStatus                 vaStatus          = VA_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT       mediaCtx          = nullptr;
    PVPHAL_RENDER_PARAMS     vpHalRenderParams = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vaDrvCtx, "nullptr vaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaCtx = GetMediaContext(vaDrvCtx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize VPHAL
    vpCtx->MosDrvCtx.bufmgr                = mediaCtx->pDrmBufMgr;
    vpCtx->MosDrvCtx.fd                    = mediaCtx->fd;
    vpCtx->MosDrvCtx.iDeviceId             = mediaCtx->iDeviceId;
    vpCtx->MosDrvCtx.m_skuTable            = mediaCtx->SkuTable;
    vpCtx->MosDrvCtx.m_waTable             = mediaCtx->WaTable;
    vpCtx->MosDrvCtx.m_gtSystemInfo        = *mediaCtx->pGtSystemInfo;
    vpCtx->MosDrvCtx.m_platform            = mediaCtx->platform;
    vpCtx->MosDrvCtx.m_auxTableMgr         = mediaCtx->m_auxTableMgr;
    vpCtx->MosDrvCtx.pGmmClientContext     = mediaCtx->pGmmClientContext;
    vpCtx->MosDrvCtx.ppMediaMemDecompState = &mediaCtx->pMediaMemDecompState;
    vpCtx->MosDrvCtx.pfnMediaMemoryCopy    = mediaCtx->pfnMediaMemoryCopy;
    vpCtx->MosDrvCtx.pfnMediaMemoryCopy2D  = mediaCtx->pfnMediaMemoryCopy2D;
    vpCtx->MosDrvCtx.pfnMemoryDecompress   = mediaCtx->pfnMemoryDecompress;
    vpCtx->MosDrvCtx.ppMediaCopyState      = &mediaCtx->pMediaCopyState;

    vpCtx->MosDrvCtx.m_osDeviceContext     = mediaCtx->m_osDeviceContext;
    vpCtx->MosDrvCtx.m_apoMosEnabled       = true;
    vpCtx->MosDrvCtx.m_userSettingPtr      = mediaCtx->m_userSettingPtr;

    vpCtx->MosDrvCtx.pPerfData = (PERF_DATA *)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));
    DDI_VP_CHK_NULL(vpCtx->MosDrvCtx.pPerfData, "nullptr vpCtx->MosDrvCtx.pPerfData", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // initialize DDI level cp interface
    vpCtx->pCpDdiInterfaceNext = CreateDdiCpNext(&vpCtx->MosDrvCtx);
    if (nullptr == vpCtx->pCpDdiInterfaceNext)
    {
        FreeVpHalRenderParams(vpCtx, vpHalRenderParams);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    DDI_CHK_RET(DdiInitVpHal(vpCtx), "Call DdiInitVpHal failed");

    // allocate vphal render param
    vpHalRenderParams = MOS_New(VPHAL_RENDER_PARAMS);
    if( nullptr == vpHalRenderParams)
    {
        FreeVpHalRenderParams(vpCtx, vpHalRenderParams);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    // initialize vphal render params
    for (int32_t surfIndex = 0; surfIndex < VPHAL_MAX_SOURCES; surfIndex++)
    {
        vpHalRenderParams->pSrc[surfIndex] = MOS_New(VPHAL_SURFACE);
        if( nullptr == vpHalRenderParams->pSrc[surfIndex])
        {
            FreeVpHalRenderParams(vpCtx, vpHalRenderParams);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }

    for (int32_t surfIndex = 0; surfIndex < VPHAL_MAX_TARGETS; surfIndex++)
    {
        vpHalRenderParams->pTarget[surfIndex] = MOS_New(VPHAL_SURFACE);
        if( nullptr == vpHalRenderParams->pTarget[surfIndex])
        {
            FreeVpHalRenderParams(vpCtx, vpHalRenderParams);
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }

    // background Colorfill
    vpHalRenderParams->pColorFillParams = MOS_New(VPHAL_COLORFILL_PARAMS);
    if( nullptr == vpHalRenderParams->pColorFillParams)
    {
        FreeVpHalRenderParams(vpCtx, vpHalRenderParams);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    // reset source surface count
    vpHalRenderParams->uSrcCount = 0;
    vpCtx->MosDrvCtx.wRevision   = 0;
    vpCtx->iPriSurfs             = 0;

    // Add the render param for calculating alpha value.
    // Because can not pass the alpha calculate flag from iVP,
    // need to hardcode here for both Android and Linux.
    vpHalRenderParams->bCalculatingAlpha = true;

    // put the render param in vp context
    vpCtx->pVpHalRenderParams    = vpHalRenderParams;

#if (_DEBUG || _RELEASE_INTERNAL)
    vaStatus = DdiVpTools::InitDumpConfig(vpCtx);
    DDI_CHK_RET(vaStatus, "Init Dump Config failed");
#endif //(_DEBUG || _RELEASE_INTERNAL)

    return VA_STATUS_SUCCESS;
}

void DdiVpFunctions::FreeVpHalRenderParams(
    PDDI_VP_CONTEXT      vpCtx,
    PVPHAL_RENDER_PARAMS vpHalRenderParams)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx", );

    if(vpHalRenderParams)
    {
        for (int32_t surfIndex = 0; surfIndex < VPHAL_MAX_SOURCES; surfIndex++)
        {
            MOS_Delete(vpHalRenderParams->pSrc[surfIndex]);
        }
        for (int32_t surfIndex = 0; surfIndex < VPHAL_MAX_TARGETS; surfIndex++)
        {
            MOS_Delete(vpHalRenderParams->pTarget[surfIndex]);
        }

        MOS_Delete(vpHalRenderParams->pColorFillParams);
        MOS_Delete(vpHalRenderParams);
    }

    if (vpCtx->pCpDdiInterfaceNext)
    {
        MOS_Delete(vpCtx->pCpDdiInterfaceNext);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    DdiVpTools::DestoryDumpConfig(vpCtx);
#endif //(_DEBUG || _RELEASE_INTERNAL)
    return;
}

VAStatus DdiVpFunctions::DdiInitVpHal(
    PDDI_VP_CONTEXT   vpCtx)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    VpBase           *vpHal        = nullptr;
    VpSettings       vpHalSettings = {};
    VAStatus         vaStatus      = VA_STATUS_ERROR_UNKNOWN;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // Create VpHal state
    MOS_STATUS status = MOS_STATUS_UNKNOWN;
    vpHal = VpBase::VphalStateFactory( nullptr, &(vpCtx->MosDrvCtx), &status);
    if (vpHal && MOS_FAILED(status))
    {
        MOS_Delete(vpHal);
        vpHal = nullptr;
    }
    if (!vpHal)
    {
        DDI_VP_ASSERTMESSAGE("Failed to create vphal.");
        MOS_Delete(vpCtx);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    vpHalSettings.maxPhases                = VP_SETTING_MAX_PHASES;
    vpHalSettings.mediaStates              = VP_SETTING_MEDIA_STATES;
    vpHalSettings.sameSampleThreshold      = VP_SETTING_SAME_SAMPLE_THRESHOLD;
    vpHalSettings.disableDnDi              = false;

    // Allocate resources (state heaps, resources, KDLL)
    if (MOS_FAILED(vpHal->Allocate(&vpHalSettings)))
    {
        DDI_VP_ASSERTMESSAGE("Failed to allocate resources for vphal.");
        MOS_Delete(vpHal);
        vpHal = nullptr;
        return vaStatus;
    }

    vpCtx->pVpHal  = vpHal;
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiDestroyVpHal(PDDI_VP_CONTEXT vpCtx)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    DdiDestroyRenderParams(vpCtx);

    // Destroy VPHAL context
    if (vpCtx)
    {
        vpCtx->MosDrvCtx.m_skuTable.reset();
        vpCtx->MosDrvCtx.m_waTable.reset();
        if (vpCtx->pVpHal)
        {
            MOS_Delete(vpCtx->pVpHal);
            vpCtx->pVpHal = nullptr;
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        DdiVpTools::DestoryDumpConfig(vpCtx);
#endif //(_DEBUG || _RELEASE_INTERNAL)
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiDestroyRenderParams(PDDI_VP_CONTEXT vpCtx)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    DdiDestroySrcParams(vpCtx);
    DdiDestroyTargetParams(vpCtx);

    if (vpCtx->pVpHalRenderParams)
    {
        MOS_Delete(vpCtx->pVpHalRenderParams->pSplitScreenDemoModeParams);
        MOS_Delete(vpCtx->pVpHalRenderParams->pCompAlpha);
        if (vpCtx->pVpHalRenderParams->pColorFillParams)
        {
            MOS_Delete(vpCtx->pVpHalRenderParams->pColorFillParams);
        }
        MOS_Delete(vpCtx->pVpHalRenderParams);
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiDestroySrcParams(PDDI_VP_CONTEXT vpCtx)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx->pVpHalRenderParams, "nullptr vpCtx->pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    for (uint32_t surfIndex = 0; surfIndex < VPHAL_MAX_SOURCES; surfIndex++)
    {
        if (nullptr != vpCtx->pVpHalRenderParams->pSrc[surfIndex])
        {
            MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pProcampParams);
            MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pDeinterlaceParams);
            MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pDenoiseParams);
            if (vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pIEFParams)
            {
                if (!vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pIEFParams->pExtParam)
                {
                    DDI_VP_ASSERTMESSAGE("vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pIEFParams->pExtParam is nullptr.");
                }
                MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pIEFParams);
            }
            MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pBlendingParams);
            MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pLumaKeyParams);
            MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pColorPipeParams);
            MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pHDRParams);
            if (vpCtx->pVpHalRenderParams->pSrc[surfIndex]->p3DLutParams)
            {
                MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->p3DLutParams->pExt3DLutSurface);
            }
            MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->p3DLutParams);
            DdiDestroyVpHalSurface(vpCtx->pVpHalRenderParams->pSrc[surfIndex]);
            vpCtx->pVpHalRenderParams->pSrc[surfIndex] = nullptr;
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiDestroyVpHalSurface(PVPHAL_SURFACE surf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(surf, "nullptr surf.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (surf->pFwdRef)
    {
        DdiDestroyVpHalSurface(surf->pFwdRef);
    }
    if (surf->pBwdRef)
    {
        DdiDestroyVpHalSurface(surf->pBwdRef);
    }

    MOS_Delete(surf);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiDestroyTargetParams(PDDI_VP_CONTEXT vpCtx)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       targetSurface     = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx->pVpHalRenderParams, "nullptr vpCtx->pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    vpHalRenderParams = vpCtx->pVpHalRenderParams;
    for (uint32_t targetIndex = 0; targetIndex < VPHAL_MAX_TARGETS; targetIndex++)
    {
        targetSurface = vpHalRenderParams->pTarget[targetIndex];
        if (targetSurface)
        {
            if (targetSurface->OsResource.bo)
            {
                targetSurface->OsResource.bo = nullptr;
            }
            if (targetSurface->pProcampParams)
            {
                MOS_Delete(targetSurface->pProcampParams);
            }
            if (targetSurface->pDeinterlaceParams)
            {
                MOS_Delete(targetSurface->pDeinterlaceParams);
            }
            if (targetSurface->pDenoiseParams)
            {
                MOS_Delete(targetSurface->pDenoiseParams);
            }
            if (targetSurface->pHDRParams)
            {
                MOS_Delete(targetSurface->pHDRParams);
            }

            MOS_Delete(vpHalRenderParams->pTarget[targetIndex]);
        }
    }
    // reset render target count
    vpHalRenderParams->uDstCount = 0;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryNoiseReductionCapsAttrib(
    uint32_t queryFlag,
    uint32_t queryCapsNum,
    uint32_t existCapsNum,
    void     *filterCaps)
{
    VAProcFilterCap *dnCap = nullptr;
    DDI_VP_FUNC_ENTER;

    if (queryFlag == QUERY_CAPS_ATTRIBUTE)
    {
        DDI_VP_CHK_NULL(filterCaps, "nullptr filterCaps", VA_STATUS_ERROR_INVALID_PARAMETER);
        if (queryCapsNum < existCapsNum)
        {
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }
        dnCap                      = (VAProcFilterCap *)filterCaps;
        dnCap->range.min_value     = NOISEREDUCTION_MIN;
        dnCap->range.max_value     = NOISEREDUCTION_MAX;
        dnCap->range.default_value = NOISEREDUCTION_DEFAULT;
        dnCap->range.step          = NOISEREDUCTION_STEP;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryDeinterlacingCapsAttrib(
    uint32_t queryFlag,
    uint32_t queryCapsNum,
    uint32_t existCapsNum,
    void     *filterCaps)
{
    VAProcFilterCapDeinterlacing *diCap = nullptr;
    DDI_VP_FUNC_ENTER;

    if (queryFlag == QUERY_CAPS_ATTRIBUTE)
    {
        DDI_VP_CHK_NULL(filterCaps, "nullptr filterCaps", VA_STATUS_ERROR_INVALID_PARAMETER);
        if (queryCapsNum < existCapsNum)
        {
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }
        diCap         = (VAProcFilterCapDeinterlacing*)filterCaps;
        diCap[0].type = VAProcDeinterlacingBob;
        diCap[1].type = VAProcDeinterlacingMotionAdaptive;
        diCap[2].type = VAProcDeinterlacingMotionCompensated;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QuerySharpeningCapsAttrib(
    uint32_t queryFlag,
    uint32_t queryCapsNum,
    uint32_t existCapsNum,
    void     *filterCaps)
{
    VAProcFilterCap *sharpeningCap = nullptr;
    DDI_VP_FUNC_ENTER;

    if (queryFlag == QUERY_CAPS_ATTRIBUTE)
    {
        DDI_VP_CHK_NULL(filterCaps, "nullptr filterCaps", VA_STATUS_ERROR_INVALID_PARAMETER);
        if (queryCapsNum < existCapsNum)
        {
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }

        sharpeningCap                      = (VAProcFilterCap*)filterCaps;
        sharpeningCap->range.min_value     = EDGEENHANCEMENT_MIN;
        sharpeningCap->range.max_value     = EDGEENHANCEMENT_MAX;
        sharpeningCap->range.default_value = EDGEENHANCEMENT_DEFAULT;
        sharpeningCap->range.step          = EDGEENHANCEMENT_STEP;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryColorBalanceCapsAttrib(
    uint32_t queryFlag,
    uint32_t queryCapsNum,
    uint32_t existCapsNum,
    void     *filterCaps)
{
    VAProcFilterCapColorBalance *colorBalCap = nullptr;
    DDI_VP_FUNC_ENTER;

    if (queryFlag == QUERY_CAPS_ATTRIBUTE)
    {
        DDI_VP_CHK_NULL(filterCaps, "nullptr filterCaps", VA_STATUS_ERROR_INVALID_PARAMETER);
        if (queryCapsNum < existCapsNum)
        {
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }

        for (uint32_t cnt = 0; cnt < queryCapsNum; cnt++)
        {
            colorBalCap                         = (VAProcFilterCapColorBalance*)filterCaps + cnt;
            colorBalCap->type                   = m_vpColorBalCap[cnt].type;
            colorBalCap->range.default_value    = m_vpColorBalCap[cnt].range.default_value;
            colorBalCap->range.max_value        = m_vpColorBalCap[cnt].range.max_value;
            colorBalCap->range.min_value        = m_vpColorBalCap[cnt].range.min_value;
            colorBalCap->range.step             = m_vpColorBalCap[cnt].range.step;
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QuerySkinToneEnhancementCapsAttrib(
    uint32_t queryFlag,
    uint32_t queryCapsNum,
    uint32_t existCapsNum,
    void     *filterCaps)
{
    VAProcFilterCap *steCap = nullptr;
    DDI_VP_FUNC_ENTER;

    if (queryFlag == QUERY_CAPS_ATTRIBUTE)
    {
        DDI_VP_CHK_NULL(filterCaps, "nullptr filterCaps", VA_STATUS_ERROR_INVALID_PARAMETER);
        if (queryCapsNum < existCapsNum)
        {
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }
        steCap                      = (VAProcFilterCap *)filterCaps;
        steCap->range.min_value     = STE_MIN;
        steCap->range.max_value     = STE_MAX;
        steCap->range.default_value = STE_DEFAULT;
        steCap->range.step          = STE_STEP;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryTotalColorCorrectionCapsAttrib(
    uint32_t queryFlag,
    uint32_t queryCapsNum,
    uint32_t existCapsNum,
    void     *filterCaps)
{
    VAProcFilterCapTotalColorCorrection *tccCap = nullptr;
    DDI_VP_FUNC_ENTER;

    if (queryFlag == QUERY_CAPS_ATTRIBUTE)
    {
        DDI_VP_CHK_NULL(filterCaps, "nullptr filterCaps", VA_STATUS_ERROR_INVALID_PARAMETER);
        if (queryCapsNum < existCapsNum)
        {
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }

        for (uint32_t cnt = 0; cnt < queryCapsNum; cnt++)
        {
            tccCap                      = (VAProcFilterCapTotalColorCorrection *)filterCaps + cnt;
            tccCap->type                = (VAProcTotalColorCorrectionType)((uint32_t)VAProcTotalColorCorrectionRed + cnt);
            tccCap->range.min_value     = TCC_MIN;
            tccCap->range.max_value     = TCC_MAX;
            tccCap->range.default_value = TCC_DEFAULT;
            tccCap->range.step          = TCC_STEP;
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryHDRToneMappingCapsAttrib(
    uint32_t            queryFlag,
    uint32_t            queryCapsNum,
    uint32_t            existCapsNum,
    void                *filterCaps)
{
    VAProcFilterCapHighDynamicRange *hdrTmCap    = nullptr;
    DDI_VP_FUNC_ENTER;

    if (queryFlag == QUERY_CAPS_ATTRIBUTE)
    {
        DDI_VP_CHK_NULL(filterCaps,    "nullptr filterCaps",    VA_STATUS_ERROR_INVALID_PARAMETER);
        if (queryCapsNum < existCapsNum)
        {
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }
        hdrTmCap                = (VAProcFilterCapHighDynamicRange *)filterCaps;
        hdrTmCap->metadata_type = VAProcHighDynamicRangeMetadataHDR10;
        hdrTmCap->caps_flag     = VA_TONE_MAPPING_HDR_TO_HDR | VA_TONE_MAPPING_HDR_TO_SDR | VA_TONE_MAPPING_HDR_TO_EDR;

    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::QueryLut3DCapsAttrib(
    uint32_t            queryFlag,
    uint32_t            queryCapsNum,
    uint32_t            existCapsNum,
    void                *filterCaps)
{
    uint32_t             channelMapping = 0;
    VAProcFilterCap3DLUT *lut3DCap      = nullptr;
    DDI_VP_FUNC_ENTER;

    /* set the actual filter caps attribute in vp module */
    if (queryFlag == QUERY_CAPS_ATTRIBUTE)
    {
        DDI_VP_CHK_NULL(filterCaps,    "nullptr filterCaps",    VA_STATUS_ERROR_INVALID_PARAMETER);
        if (queryCapsNum < existCapsNum)
        {
            return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        }

        channelMapping = VA_3DLUT_CHANNEL_RGB_RGB | VA_3DLUT_CHANNEL_YUV_RGB | VA_3DLUT_CHANNEL_VUY_RGB;
        /* 17^3 */
        lut3DCap                      = (VAProcFilterCap3DLUT *)filterCaps + 0;
        lut3DCap->lut_size            = 17;
        lut3DCap->lut_stride[0]       = 17;
        lut3DCap->lut_stride[1]       = 17;
        lut3DCap->lut_stride[2]       = 32;
        lut3DCap->bit_depth           = 16;
        lut3DCap->num_channel         = 4;
        lut3DCap->channel_mapping     = channelMapping;
        /* 33^3 */
        lut3DCap                      = (VAProcFilterCap3DLUT *)filterCaps + 1;
        lut3DCap->lut_size            = 33;
        lut3DCap->lut_stride[0]       = 33;
        lut3DCap->lut_stride[1]       = 33;
        lut3DCap->lut_stride[2]       = 64;
        lut3DCap->bit_depth           = 16;
        lut3DCap->num_channel         = 4;
        lut3DCap->channel_mapping     = channelMapping;
        /* 65^3 */
        lut3DCap                      = (VAProcFilterCap3DLUT *)filterCaps + 2;
        lut3DCap->lut_size            = 65;
        lut3DCap->lut_stride[0]       = 65;
        lut3DCap->lut_stride[1]       = 65;
        lut3DCap->lut_stride[2]       = 128;
        lut3DCap->bit_depth           = 16;
        lut3DCap->num_channel         = 4;
        lut3DCap->channel_mapping     = channelMapping;
    }
    return VA_STATUS_SUCCESS;
}

PVPHAL_RENDER_PARAMS DdiVpFunctions::VpGetRenderParams(PDDI_VP_CONTEXT vpCtx)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", nullptr);
    return vpCtx->pVpHalRenderParams;
}

VAStatus DdiVpFunctions::VpSetOsResource(
    PDDI_VP_CONTEXT     vpCtx,
    PDDI_MEDIA_SURFACE  boRt,
    uint32_t            targetIndex)
{
    PMOS_RESOURCE           osResource        = nullptr;
    PVPHAL_RENDER_PARAMS    vpHalRenderParams = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(boRt, "nullptr boRt.", VA_STATUS_ERROR_INVALID_SURFACE);

    // get vphal render parameters
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    // set render target os resource information according to target surface
    osResource              = &vpHalRenderParams->pTarget[targetIndex]->OsResource;
    DDI_VP_CHK_NULL(osResource, "nullptr osResource.", VA_STATUS_ERROR_INVALID_PARAMETER);

    osResource->bo          = boRt->bo;
    osResource->bMapped     = boRt->bMapped;
    osResource->Format      = MediaLibvaUtilNext::GetFormatFromMediaFormat(boRt->format);
    osResource->iWidth      = boRt->iWidth;
    osResource->iHeight     = boRt->iHeight;
    osResource->iPitch      = boRt->iPitch;
    osResource->iCount      = boRt->iRefCount;
    osResource->TileType    = MediaLibvaUtilNext::GetTileTypeFromMediaTileType(boRt->TileType);
    osResource->pGmmResInfo = boRt->pGmmResourceInfo;

    Mos_Solo_SetOsResource(boRt->pGmmResourceInfo, osResource);

    return VA_STATUS_SUCCESS;
}

void DdiVpFunctions::VpSetRenderParams(
    PDDI_MEDIA_SURFACE    mediaSurf,
    PVPHAL_RENDER_PARAMS  vpHalRenderParams,
    VASurfaceID           renderTarget)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(mediaSurf, "nullptr mediaSurf", );
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams", );

    // reset source surface count
    vpHalRenderParams->uSrcCount = 0;

    vpHalRenderParams->bReportStatus    = true;
    vpHalRenderParams->StatusFeedBackID = renderTarget;

    if (mediaSurf->pSurfDesc && (mediaSurf->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR))
    {
        vpHalRenderParams->pTarget[vpHalRenderParams->uDstCount]->b16UsrPtr = VpIs16UsrPtrPitch(mediaSurf->iPitch, mediaSurf->format);
    }
    else
    {
        vpHalRenderParams->pTarget[vpHalRenderParams->uDstCount]->b16UsrPtr = false;
    }

    // increase render target count
    vpHalRenderParams->uDstCount++;
    return;
}

bool DdiVpFunctions::VpIs16UsrPtrPitch(uint32_t pitch, DDI_MEDIA_FORMAT format)
{
    uint32_t pitchAligned = 64;
    bool     status       = false;
    DDI_VP_FUNC_ENTER;

    if (Media_Format_YV12 == format)
    {
        pitchAligned = 128;
    }

    if (!(pitch % 16) && (pitch % pitchAligned))
    {
        status = true;
    }
    else
    {
        status = false;
    }

    DDI_VP_NORMALMESSAGE("[VP] 16Usrptr check, surface pitch is %d, go to %s path.", pitch, status?"16Usrptr":"legacy");
    return status;
}

VAStatus DdiVpFunctions::VpReportFeatureMode(PDDI_VP_CONTEXT vpCtx)
{
    VP_CONFIG configValues = {};
    DDI_VP_FUNC_ENTER;

    DDI_VP_CHK_NULL(vpCtx,         "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx->pVpHal, "nullptr vpHal.", VA_STATUS_ERROR_INVALID_PARAMETER);

    VpConfigValuesInit(&configValues);
    VpHalDdiReportFeatureMode(vpCtx->pVpHal, &configValues);
    VpFeatureReport(&configValues, vpCtx);

    return VA_STATUS_SUCCESS;
}

void DdiVpFunctions::VpConfigValuesInit(
    PVP_CONFIG  configValues)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(configValues, "nullptr configValues.",);

    configValues->dwVpPath                   = 0;
    configValues->dwVpComponent              = 0;
    configValues->dwReportedDeinterlaceMode  = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwReportedScalingMode      = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwReportedOutputPipeMode   = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwReportedVEFeatureInUse   = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwVPMMCInUseReported       = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwRTCompressibleReported   = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwRTCompressModeReported   = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwCapturePipeInUseReported = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwReportedCompositionMode  = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwReportedHdrMode          = LIBVA_VP_CONFIG_NOT_REPORTED;

    configValues->dwFFDICompressibleReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwFFDICompressModeReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwFFDNCompressibleReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwFFDNCompressModeReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwSTMMCompressibleReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwSTMMCompressModeReported    = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwScalerCompressibleReported  = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwScalerCompressModeReported  = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwPrimaryCompressibleReported = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwPrimaryCompressModeReported = LIBVA_VP_CONFIG_NOT_REPORTED;

    configValues->dwReportedVeboxScalability    = LIBVA_VP_CONFIG_NOT_REPORTED;
    configValues->dwReportedVPApogeios          = LIBVA_VP_CONFIG_NOT_REPORTED;
    return;
}

void DdiVpFunctions::VpHalDdiReportFeatureMode(
    VpBase         *vpHalState,
    PVP_CONFIG     configValues)
{
    VphalFeatureReport *report = nullptr;
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalState, "nullptr vpHalState.", );

    // Get VPHAL feature reporting
    report = vpHalState->GetRenderFeatureReport();
    DDI_VP_CHK_NULL(report, "nullptr report", );

    report->SetConfigValues(configValues);
    return;
}

void DdiVpFunctions::VpFeatureReport(PVP_CONFIG config, PDDI_VP_CONTEXT vpCtx)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(config, "nullptr config.", );

    MediaUserSettingSharedPtr userSettingPtr = vpCtx ? vpCtx->MosDrvCtx.m_userSettingPtr : nullptr;

    ReportUserSetting(
        userSettingPtr,
        __VPHAL_VEBOX_OUTPUTPIPE_MODE,
        config->dwCurrentOutputPipeMode,
        MediaUserSetting::Group::Sequence);

    ReportUserSetting(
        userSettingPtr,
        __VPHAL_VEBOX_FEATURE_INUSE,
        config->dwCurrentVEFeatureInUse,
        MediaUserSetting::Group::Sequence);

#ifdef _MMC_SUPPORTED
    //VP Primary Surface Compress Mode Report
    ReportUserSetting(
        userSettingPtr,
        __VPHAL_PRIMARY_MMC_COMPRESSMODE,
        config->dwPrimaryCompressMode,
        MediaUserSetting::Group::Sequence);
    //VP RT Compress Mode
    ReportUserSetting(
        userSettingPtr,
        __VPHAL_RT_MMC_COMPRESSMODE,
        config->dwRTCompressMode,
        MediaUserSetting::Group::Sequence);
#endif
    //VP RT Cache Usage
    ReportUserSetting(
        userSettingPtr,
        __VPHAL_RT_Cache_Setting,
        config->dwRTCacheSetting,
        MediaUserSetting::Group::Sequence);

#if (_DEBUG || _RELEASE_INTERNAL)
    //VP RT Old Cache Usage
    ReportUserSetting(
        userSettingPtr,
        __VPHAL_RT_Old_Cache_Setting,
        config->dwRTOldCacheSetting,
        MediaUserSetting::Group::Sequence);

    ReportUserSettingForDebug(
        userSettingPtr,
        __VPHAL_VEBOX_HDR_MODE,
        config->dwCurrentHdrMode,
        MediaUserSetting::Group::Sequence);

#ifdef _MMC_SUPPORTED
    //VP MMC In Use
    ReportUserSettingForDebug(
        userSettingPtr,
        __VPHAL_MMC_ENABLE,
        config->dwVPMMCInUse,
        MediaUserSetting::Group::Sequence);

    //VP Primary Surface Compressible
    ReportUserSettingForDebug(
        userSettingPtr,
        __VPHAL_PRIMARY_MMC_COMPRESSIBLE,
        config->dwPrimaryCompressible,
        MediaUserSetting::Group::Sequence);

    //VP RT Compressible
    ReportUserSettingForDebug(
        userSettingPtr,
        __VPHAL_RT_MMC_COMPRESSIBLE,
        config->dwRTCompressible,
        MediaUserSetting::Group::Sequence);

#endif
#endif //(_DEBUG || _RELEASE_INTERNAL)

    if (config->dwCurrentVeboxScalability != config->dwReportedVeboxScalability)
    {
        ReportUserSetting(
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE,
            config->dwCurrentVeboxScalability,
            MediaUserSetting::Group::Device);

        config->dwReportedVeboxScalability = config->dwCurrentVeboxScalability;
    }

    if (config->dwCurrentVPApogeios != config->dwReportedVPApogeios)
    {
        ReportUserSetting(
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_VPP_APOGEIOS_ENABLE,
            config->dwCurrentVPApogeios,
            MediaUserSetting::Group::Sequence);

        config->dwReportedVPApogeios = config->dwCurrentVPApogeios;
    }

    return;
}

VAStatus DdiVpFunctions::DdiSetGpuPriority(
    PDDI_VP_CONTEXT     vpCtx,
    int32_t             priority)
{
    PMOS_INTERFACE osInterface = nullptr;
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    //Set the priority for Gpu
    if(vpCtx->pVpHal)
    {
        osInterface = vpCtx->pVpHal->GetOsInterface();
        DDI_VP_CHK_NULL(osInterface, "nullptr osInterface.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        osInterface->pfnSetGpuPriority(osInterface, priority);
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::VpSetColorStandardExplictly(
    PVPHAL_SURFACE          vpHalSurf,
    VAProcColorStandardType colorStandard,
    VAProcColorProperties   colorProperties)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSurf, "nullptr vpHalSurf.", VA_STATUS_ERROR_INVALID_SURFACE);
    DDI_VP_CHK_CONDITION((colorStandard != VAProcColorStandardExplicit), "Not Explict color standard, Exit!", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (IS_RGB_FORMAT(vpHalSurf->Format))
    {
        switch (colorProperties.colour_primaries)
        {
            case COLOUR_PRIMARY_BT2020:
                vpHalSurf->ColorSpace = (colorProperties.color_range & VA_SOURCE_RANGE_REDUCED) ? CSpace_BT2020_stRGB : CSpace_BT2020_RGB;
                break;
            case COLOUR_PRIMARY_BT709:
            case COLOUR_PRIMARY_BT601:
                vpHalSurf->ColorSpace = (colorProperties.color_range & VA_SOURCE_RANGE_REDUCED) ? CSpace_stRGB : CSpace_sRGB;
                break;
            default:
                vpHalSurf->ColorSpace = CSpace_sRGB;
                DDI_VP_ASSERTMESSAGE("unknown Color Standard for RGB format.");
                break;
        }
    }

    if (IS_YUV_FORMAT(vpHalSurf->Format) || IS_ALPHA_YUV_FORMAT(vpHalSurf->Format))
    {
        switch (colorProperties.colour_primaries)
        {
            case COLOUR_PRIMARY_BT2020:
                vpHalSurf->ColorSpace = (colorProperties.color_range & VA_SOURCE_RANGE_FULL) ? CSpace_BT2020_FullRange : CSpace_BT2020;
                break;
            case COLOUR_PRIMARY_BT709:
                vpHalSurf->ColorSpace = (colorProperties.color_range & VA_SOURCE_RANGE_FULL) ? CSpace_BT709_FullRange : CSpace_BT709;
                break;
            case COLOUR_PRIMARY_BT601:
                vpHalSurf->ColorSpace = (colorProperties.color_range & VA_SOURCE_RANGE_FULL) ? CSpace_BT601_FullRange : CSpace_BT601;
                break;
            default:
                vpHalSurf->ColorSpace = CSpace_BT601;
                DDI_VP_ASSERTMESSAGE("unknown Color Standard for YUV format.");
                break;
        }
    }

    switch (colorProperties.transfer_characteristics)
    {
        case TRANSFER_CHARACTERISTICS_ST2084:
            vpHalSurf->GammaType = VPHAL_GAMMA_SMPTE_ST2084;
            break;
        default:
            vpHalSurf->GammaType = VPHAL_GAMMA_TRADITIONAL_GAMMA;
            break;
    }

    return VA_STATUS_SUCCESS;
}

bool DdiVpFunctions::VpIsRenderTarget(
    VADriverContextP              vaDrvCtx,
    PDDI_VP_CONTEXT               vpCtx,
    VAProcPipelineParameterBuffer *pipelineParam)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PDDI_MEDIA_CONTEXT   mediaCtx          = nullptr;
    PDDI_MEDIA_SURFACE   mediaSrcSurf      = nullptr;
    PVPHAL_SURFACE       vpHalTgtSurf      = nullptr;
    bool                 isTarget          = false;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vaDrvCtx,      "nullptr vaDrvCtx.",      VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx,         "nullptr vpCtx.",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(pipelineParam, "nullptr pipelineParam.", VA_STATUS_ERROR_INVALID_BUFFER);

    mediaCtx = GetMediaContext(vaDrvCtx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    mediaSrcSurf = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, pipelineParam->surface);
    DDI_VP_CHK_NULL(mediaSrcSurf, "nullptr mediaSrcSurf.", VA_STATUS_ERROR_INVALID_BUFFER);
    vpHalRenderParams = vpCtx->pVpHalRenderParams;
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (pipelineParam->pipeline_flags == 0 && vpHalRenderParams->uDstCount >= 1)
    {
        vpHalTgtSurf = vpHalRenderParams->pTarget[vpHalRenderParams->uDstCount - 1];

        isTarget = ((vpHalTgtSurf->OsResource.bo != nullptr) && (vpHalTgtSurf->OsResource.bo == mediaSrcSurf->bo));
    }

    return isTarget;
}

VAStatus DdiVpFunctions::VpSetRenderTargetParams(
    VADriverContextP              vaDrvCtx,
    PDDI_VP_CONTEXT               vpCtx,
    VAProcPipelineParameterBuffer *pipelineParam)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PDDI_MEDIA_CONTEXT   mediaCtx          = nullptr;
    PDDI_MEDIA_SURFACE   mediaSrcSurf      = nullptr;
    PVPHAL_SURFACE       vpHalTgtSurf      = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vaDrvCtx,      "nullptr vaDrvCtx.",      VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx,         "nullptr vpCtx.",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(pipelineParam, "nullptr pipelineParam.", VA_STATUS_ERROR_INVALID_BUFFER);

    mediaCtx = GetMediaContext(vaDrvCtx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaSrcSurf = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, pipelineParam->surface);
    DDI_VP_CHK_NULL(mediaSrcSurf, "nullptr mediaSrcSurf.", VA_STATUS_ERROR_INVALID_BUFFER);
    vpHalRenderParams = vpCtx->pVpHalRenderParams;
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr pVpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if(vpHalRenderParams->uDstCount < 1)
    {
        DDI_VP_ASSERTMESSAGE("vpHalRenderParams->uDstCount is lower than 1");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    DDI_VP_CHK_NULL(vpHalRenderParams->pTarget, "nullptr vpHalRenderParams->pTarget.", VA_STATUS_ERROR_INVALID_BUFFER);
    vpHalTgtSurf = vpHalRenderParams->pTarget[vpHalRenderParams->uDstCount - 1];
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf.", VA_STATUS_ERROR_INVALID_BUFFER);

    // Set src rect
    SetSrcRect(pipelineParam->surface_region, vpHalTgtSurf, mediaSrcSurf);
    // Set dest rect
    SetDestRect(pipelineParam->output_region, vpHalTgtSurf, mediaSrcSurf);

    if (IsProcmpEnable(vpHalTgtSurf))
    {
        // correct the ChromaSitting location if Procamp is enabled.
#if (VA_MAJOR_VERSION < 1)
        pipelineParam->output_surface_flag = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
#else
        pipelineParam->output_color_properties.chroma_sample_location = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
#endif
    }

#if (VA_MAJOR_VERSION < 1)
    VpUpdateProcChromaSittingState(vpHalTgtSurf, (uint8_t)(pipelineParam->output_surface_flag & 0xff));
#else
    VpUpdateProcChromaSittingState(vpHalTgtSurf, pipelineParam->output_color_properties.chroma_sample_location);
#endif
    return VA_STATUS_SUCCESS;
}

void DdiVpFunctions::SetSrcRect(
    const VARectangle  *surfaceRegion,
    PVPHAL_SURFACE     vpHalTgtSurf,
    PDDI_MEDIA_SURFACE mediaSrcSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(surfaceRegion, "nullptr surfaceRegion", );
    DDI_VP_CHK_NULL(vpHalTgtSurf,  "nullptr vpHalTgtSurf", );
    DDI_VP_CHK_NULL(mediaSrcSurf,  "nullptr mediaSrcSurf", );

    vpHalTgtSurf->rcSrc.left    = surfaceRegion->x;
    vpHalTgtSurf->rcSrc.top     = surfaceRegion->y;
    vpHalTgtSurf->rcSrc.right   = surfaceRegion->x + surfaceRegion->width;
    vpHalTgtSurf->rcSrc.bottom  = surfaceRegion->y + surfaceRegion->height;

    if (vpHalTgtSurf->rcSrc.top < 0)
    {
        vpHalTgtSurf->rcSrc.top = 0;
    }

    if (vpHalTgtSurf->rcSrc.left < 0)
    {
        vpHalTgtSurf->rcSrc.left = 0;
    }

    if (vpHalTgtSurf->rcSrc.right > mediaSrcSurf->iWidth)
    {
        vpHalTgtSurf->rcSrc.right = mediaSrcSurf->iWidth;
    }

    if (vpHalTgtSurf->rcSrc.bottom > mediaSrcSurf->iHeight)
    {
        vpHalTgtSurf->rcSrc.bottom = mediaSrcSurf->iHeight;
    }

    return;
}

void DdiVpFunctions::SetDestRect(
    const VARectangle  *outputRegion,
    PVPHAL_SURFACE     vpHalTgtSurf,
    PDDI_MEDIA_SURFACE mediaSrcSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(outputRegion, "nullptr outputRegion", );
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf", );
    DDI_VP_CHK_NULL(mediaSrcSurf, "nullptr mediaSrcSurf", );

    vpHalTgtSurf->rcDst.left    = outputRegion->x;
    vpHalTgtSurf->rcDst.top     = outputRegion->y;
    vpHalTgtSurf->rcDst.right   = outputRegion->x + outputRegion->width;
    vpHalTgtSurf->rcDst.bottom  = outputRegion->y + outputRegion->height;
    if (vpHalTgtSurf->rcDst.top < 0)
    {
        vpHalTgtSurf->rcDst.top = 0;
    }

    if (vpHalTgtSurf->rcDst.left < 0)
    {
        vpHalTgtSurf->rcDst.left = 0;
    }

    if (vpHalTgtSurf->rcDst.right > mediaSrcSurf->iWidth)
    {
        vpHalTgtSurf->rcDst.right = mediaSrcSurf->iWidth;
    }

    if (vpHalTgtSurf->rcDst.bottom > mediaSrcSurf->iHeight)
    {
        vpHalTgtSurf->rcDst.bottom = mediaSrcSurf->iHeight;
    }

    return;
}

bool DdiVpFunctions::IsProcmpEnable(PVPHAL_SURFACE vpHalSrcSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if ((vpHalSrcSurf->pProcampParams && vpHalSrcSurf->pProcampParams->bEnabled) &&
        (vpHalSrcSurf->pProcampParams->fContrast == 1 && vpHalSrcSurf->pProcampParams->fHue == 0 && vpHalSrcSurf->pProcampParams->fSaturation == 1) &&
        !vpHalSrcSurf->pBlendingParams && !vpHalSrcSurf->pLumaKeyParams && (!vpHalSrcSurf->pIEFParams || !vpHalSrcSurf->pIEFParams->bEnabled) &&
        !vpHalSrcSurf->pDeinterlaceParams && (!vpHalSrcSurf->pDenoiseParams || (!vpHalSrcSurf->pDenoiseParams->bEnableChroma && !vpHalSrcSurf->pDenoiseParams->bEnableLuma)) &&
        (!vpHalSrcSurf->pColorPipeParams || (!vpHalSrcSurf->pColorPipeParams->bEnableACE && !vpHalSrcSurf->pColorPipeParams->bEnableSTE && !vpHalSrcSurf->pColorPipeParams->bEnableTCC)) &&
        !vpHalSrcSurf->pHDRParams)
    {
        return true;
    }

    return false;
}

VAStatus DdiVpFunctions::VpUpdateProcChromaSittingState(
    PVPHAL_SURFACE vpHalSurf,
    uint8_t        chromasitingState)
{
    uint32_t chromaSitingFlags = 0;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSurf, "nullptr vpHalSurf.", VA_STATUS_ERROR_INVALID_PARAMETER);

    // Chroma siting
    // The lower 4 bits are still used as chroma-siting flag for output/input_surface_flag
    // Set the vertical chroma siting info at bit 1:0
    chromaSitingFlags = chromasitingState & 0x3;

    switch (chromaSitingFlags)
    {
    case VA_CHROMA_SITING_VERTICAL_TOP:
        vpHalSurf->ChromaSiting = CHROMA_SITING_VERT_TOP;
        break;
    case VA_CHROMA_SITING_VERTICAL_CENTER:
        vpHalSurf->ChromaSiting = CHROMA_SITING_VERT_CENTER;
        break;
    case VA_CHROMA_SITING_VERTICAL_BOTTOM:
        vpHalSurf->ChromaSiting = CHROMA_SITING_VERT_BOTTOM;
        break;
    default:
        vpHalSurf->ChromaSiting = CHROMA_SITING_NONE;
        break;
    }

    if (vpHalSurf->ChromaSiting != CHROMA_SITING_NONE)
    {
        // Set the horizontal chroma siting info at bit 3:2
        chromaSitingFlags = chromasitingState & 0xc;

        switch (chromaSitingFlags)
        {
        case VA_CHROMA_SITING_HORIZONTAL_LEFT:
            vpHalSurf->ChromaSiting = vpHalSurf->ChromaSiting | CHROMA_SITING_HORZ_LEFT;
            break;
        case VA_CHROMA_SITING_HORIZONTAL_CENTER:
            vpHalSurf->ChromaSiting = vpHalSurf->ChromaSiting | CHROMA_SITING_HORZ_CENTER;
            break;
        default:
            vpHalSurf->ChromaSiting = CHROMA_SITING_NONE;
            break;
        }
    }

    return VA_STATUS_SUCCESS;
}

void DdiVpFunctions::SetSrcSurfaceRect(
    const VARectangle        *surfRegion,
    PVPHAL_SURFACE           vpHalSrcSurf,
    PDDI_MEDIA_SURFACE       mediaSrcSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf", );
    DDI_VP_CHK_NULL(mediaSrcSurf, "nullptr mediaSrcSurf", );

    if (surfRegion != nullptr)
    {
        vpHalSrcSurf->rcSrc.top    = surfRegion->y;
        vpHalSrcSurf->rcSrc.left   = surfRegion->x;
        vpHalSrcSurf->rcSrc.right  = surfRegion->x + surfRegion->width;
        vpHalSrcSurf->rcSrc.bottom = surfRegion->y + surfRegion->height;

        if (vpHalSrcSurf->rcSrc.top < 0)
        {
            vpHalSrcSurf->rcSrc.top = 0;
        }

        if (vpHalSrcSurf->rcSrc.left < 0)
        {
            vpHalSrcSurf->rcSrc.left = 0;
        }

        if (vpHalSrcSurf->rcSrc.right > mediaSrcSurf->iWidth)
        {
            vpHalSrcSurf->rcSrc.right = mediaSrcSurf->iWidth;
        }

        if (vpHalSrcSurf->rcSrc.bottom > mediaSrcSurf->iRealHeight)
        {
            vpHalSrcSurf->rcSrc.bottom = mediaSrcSurf->iRealHeight;
        }
    }
    else
    {
        // nullptr surface_region implies the whole surface
        vpHalSrcSurf->rcSrc.top    = 0;
        vpHalSrcSurf->rcSrc.left   = 0;
        vpHalSrcSurf->rcSrc.right  = mediaSrcSurf->iWidth;
        vpHalSrcSurf->rcSrc.bottom = mediaSrcSurf->iRealHeight;
    }
    return;
}

VAStatus DdiVpFunctions::SetDestSurfaceRect(
    const VARectangle    *outputRegion,
    PVPHAL_SURFACE       vpHalSrcSurf,
    PVPHAL_SURFACE       vpHalTgtSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf.", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (outputRegion != nullptr)
    {
        vpHalSrcSurf->rcDst.top    = outputRegion->y;
        vpHalSrcSurf->rcDst.left   = outputRegion->x;
        vpHalSrcSurf->rcDst.right  = outputRegion->x + outputRegion->width;
        vpHalSrcSurf->rcDst.bottom = outputRegion->y + outputRegion->height;
    }
    else
    {
        vpHalSrcSurf->rcDst.top    = 0;
        vpHalSrcSurf->rcDst.left   = 0;
        vpHalSrcSurf->rcDst.right  = vpHalTgtSurf->rcDst.right;
        vpHalSrcSurf->rcDst.bottom = vpHalTgtSurf->rcDst.bottom;
    }

    if ((vpHalTgtSurf->rcSrc.right < vpHalSrcSurf->rcDst.right) ||
        (vpHalTgtSurf->rcSrc.bottom < vpHalSrcSurf->rcDst.bottom))
    {
        DDI_VP_CHK_CONDITION(true, "Invalid color fill parameter!", VA_STATUS_ERROR_INVALID_PARAMETER);
    }
    return VA_STATUS_SUCCESS;
}

#if (VA_MAJOR_VERSION < 1)
VAStatus DdiVpFunctions::DdiGetColorSpace(PVPHAL_SURFACE vpHalSurf, VAProcColorStandardType colorStandard, uint32_t flag)
#else
VAStatus DdiVpFunctions::DdiGetColorSpace(PVPHAL_SURFACE vpHalSurf, VAProcColorStandardType colorStandard, VAProcColorProperties colorProperties)
#endif
{
    uint8_t colorRange = colorProperties.color_range;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSurf, "nullptr vpHalSurf", VA_STATUS_ERROR_INVALID_PARAMETER);

    vpHalSurf->ColorSpace = CSpace_None;

    // Convert VAProcColorStandardType to VPHAL_CSPACE
    if (IS_RGB_FORMAT(vpHalSurf->Format) || (vpHalSurf->Format == Format_P8))
    {
        switch (colorStandard)
        {
        case VAProcColorStandardBT2020:
#if (VA_MAJOR_VERSION < 1)
            if (flag & VA_SOURCE_RANGE_FULL)
#else
            if (colorRange == VA_SOURCE_RANGE_FULL)
#endif
            {
                vpHalSurf->ColorSpace = CSpace_BT2020_RGB;
            }
            else
            {
                vpHalSurf->ColorSpace = CSpace_BT2020_stRGB;
            }
            break;
        case VAProcColorStandardSTRGB:
            vpHalSurf->ColorSpace = CSpace_stRGB;
            break;
        case VAProcColorStandardExplicit:
            VpSetColorStandardExplictly(vpHalSurf, colorStandard, colorProperties);
            break;
        case VAProcColorStandardSRGB:
        default:
            vpHalSurf->ColorSpace = CSpace_sRGB;
            break;
        }
    }
    else
    {
        // Set colorspace by default to avoid application don't set ColorStandard
        if (colorStandard == 0)
        {
            VpSetColorSpaceByDefault(vpHalSurf);
        }
        else
        {
            VpSetColorSpaceByColorStandard(vpHalSurf, colorStandard, colorProperties, colorRange);
        }
    }
    DDI_VP_CHK_CONDITION((vpHalSurf->ColorSpace == CSpace_None), "Invalid color standard", VA_STATUS_ERROR_INVALID_PARAMETER);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::SetBackgroundColorfill(
    PVPHAL_RENDER_PARAMS vpHalRenderParams,
    uint32_t             outBackGroundcolor)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    if ((outBackGroundcolor >> 24) != 0)
    {
        if (vpHalRenderParams->pColorFillParams == nullptr)
        {
            vpHalRenderParams->pColorFillParams = MOS_New(VPHAL_COLORFILL_PARAMS);
        }

        DDI_VP_CHK_NULL(vpHalRenderParams->pColorFillParams, "nullptr pColorFillParams.", VA_STATUS_ERROR_UNKNOWN);

        // set background colorfill option
        vpHalRenderParams->pColorFillParams->Color   = outBackGroundcolor;
        vpHalRenderParams->pColorFillParams->bYCbCr  = false;
        vpHalRenderParams->pColorFillParams->CSpace  = CSpace_sRGB;

    }
    else
    {
        MOS_Delete(vpHalRenderParams->pColorFillParams);
    }
    return VA_STATUS_SUCCESS;
}

MOS_STATUS DdiVpFunctions::VpHalDdiSetupSplitScreenDemoMode(
    uint32_t                             splitDemoPosDdi,
    uint32_t                             splitDemoParaDdi,
    PVPHAL_SPLIT_SCREEN_DEMO_MODE_PARAMS *splitScreenDemoModeParams,
    bool                                 *disableDemoMode,
    PMOS_INTERFACE                       osInterface)
{
    MOS_STATUS eStatus                   = MOS_STATUS_SUCCESS;
    uint32_t   splitScreenDemoPosition   = splitDemoPosDdi;
    uint32_t   splitScreenDemoParameters = splitDemoParaDdi;
    uint32_t   ufSplitScreenDemoPosition   = 0;
    uint32_t   ufSplitScreenDemoParameters = 0;
    MediaUserSettingSharedPtr userSetting  = nullptr;
    DDI_VP_FUNC_ENTER;
    //--------------------------
    // Set Demo Mode Parameters
    //--------------------------
    if (*splitScreenDemoModeParams == nullptr)
    {
        *splitScreenDemoModeParams = (PVPHAL_SPLIT_SCREEN_DEMO_MODE_PARAMS)MOS_AllocAndZeroMemory(sizeof(VPHAL_SPLIT_SCREEN_DEMO_MODE_PARAMS));
        DDI_VP_CHK_NULL(*splitScreenDemoModeParams, "nullptr *splitScreenDemoModeParams!", MOS_STATUS_NULL_POINTER);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // If it is not enabled from DDI params, check if internal user feature key have settings
    if (splitScreenDemoPosition == SPLIT_SCREEN_DEMO_DISABLED && splitScreenDemoParameters == 0)
    {
        DDI_VP_CHK_NULL(osInterface, "nullptr osInterface", MOS_STATUS_NULL_POINTER);
        userSetting = osInterface->pfnGetUserSettingInstance(osInterface);
        ReadUserSettingForDebug(
            userSetting,
            ufSplitScreenDemoPosition,
            __MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_POSITION,
            MediaUserSetting::Group::Device);
        splitScreenDemoPosition = ufSplitScreenDemoPosition;

        ReadUserSettingForDebug(
            userSetting,
            ufSplitScreenDemoParameters,
            __MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_PARAMETERS,
            MediaUserSetting::Group::Device);
        splitScreenDemoParameters = ufSplitScreenDemoParameters;
    }
#endif

    if ((splitScreenDemoPosition > SPLIT_SCREEN_DEMO_DISABLED) && (splitScreenDemoPosition < SPLIT_SCREEN_DEMO_END_POS_LIST))
    {
        (*splitScreenDemoModeParams)->Position        = (VPHAL_SPLIT_SCREEN_DEMO_POSITION)(splitScreenDemoPosition);
        (*splitScreenDemoModeParams)->bDisableACE     = (bool)((splitScreenDemoParameters & 0x0001) > 0);
        (*splitScreenDemoModeParams)->bDisableAVS     = (bool)((splitScreenDemoParameters & 0x0002) > 0);
        (*splitScreenDemoModeParams)->bDisableDN      = (bool)((splitScreenDemoParameters & 0x0004) > 0);
        (*splitScreenDemoModeParams)->bDisableFMD     = (bool)((splitScreenDemoParameters & 0x0008) > 0);
        (*splitScreenDemoModeParams)->bDisableIEF     = (bool)((splitScreenDemoParameters & 0x0010) > 0);
        (*splitScreenDemoModeParams)->bDisableProcamp = (bool)((splitScreenDemoParameters & 0x0020) > 0);
        (*splitScreenDemoModeParams)->bDisableSTE     = (bool)((splitScreenDemoParameters & 0x0040) > 0);
        (*splitScreenDemoModeParams)->bDisableTCC     = (bool)((splitScreenDemoParameters & 0x0080) > 0);
        (*splitScreenDemoModeParams)->bDisableIS      = (bool)((splitScreenDemoParameters & 0x0100) > 0);
        (*splitScreenDemoModeParams)->bDisableDrDb    = (bool)((splitScreenDemoParameters & 0x0200) > 0);
        (*splitScreenDemoModeParams)->bDisableDNUV    = (bool)((splitScreenDemoParameters & 0x0400) > 0);
        (*splitScreenDemoModeParams)->bDisableFRC     = (bool)((splitScreenDemoParameters & 0x0800) > 0);
        (*splitScreenDemoModeParams)->bDisableLACE    = (bool)((splitScreenDemoParameters & 0x1000) > 0);
        *disableDemoMode                              = false;
    }
    else
    {
        *disableDemoMode = true;
    }

    return eStatus;
}

VAStatus DdiVpFunctions::DdiUpdateProcPipelineFutureReferenceFrames(
    PDDI_VP_CONTEXT               vpCtx,
    VADriverContextP              vaDrvCtx,
    PVPHAL_SURFACE                vpHalSrcSurf,
    VAProcPipelineParameterBuffer *pipelineParam)
{
    PVPHAL_SURFACE       surface        = nullptr;
    PDDI_MEDIA_SURFACE   refSurfBuffObj = nullptr;
    PDDI_MEDIA_CONTEXT   mediaCtx       = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx,         "nullptr vpCtx!",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vaDrvCtx,      "nullptr vaDrvCtx!",      VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpHalSrcSurf,  "nullptr vpHalSrcSurf!",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(pipelineParam, "nullptr pipelineParam!", VA_STATUS_ERROR_INVALID_PARAMETER);

    // initialize
    mediaCtx = GetMediaContext(vaDrvCtx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx!", VA_STATUS_ERROR_INVALID_CONTEXT);

    surface = vpHalSrcSurf;
    if (!pipelineParam->backward_references)
    {
        DDI_VP_NORMALMESSAGE("nullptr pipelineParam->forward_references");
        return VA_STATUS_SUCCESS;
    }

    for (uint32_t i = 0; i < pipelineParam->num_backward_references; i++)
    {
        if (surface->pFwdRef == nullptr)
        {
            surface->pFwdRef = MOS_New(VPHAL_SURFACE);
            DDI_VP_CHK_NULL(surface->pFwdRef, "nullptr surface->pFwdRef!", VA_STATUS_ERROR_ALLOCATION_FAILED);

            surface->pFwdRef->Format        = vpHalSrcSurf->Format;
            surface->pFwdRef->SurfType      = vpHalSrcSurf->SurfType;
            surface->pFwdRef->rcSrc         = vpHalSrcSurf->rcSrc;
            surface->pFwdRef->rcDst         = vpHalSrcSurf->rcDst;
            surface->pFwdRef->ColorSpace    = vpHalSrcSurf->ColorSpace;
            surface->pFwdRef->ExtendedGamut = vpHalSrcSurf->ExtendedGamut;
            surface->pFwdRef->SampleType    = vpHalSrcSurf->SampleType;
            surface->pFwdRef->ScalingMode   = vpHalSrcSurf->ScalingMode;
            surface->pFwdRef->OsResource    = vpHalSrcSurf->OsResource;
            surface->pFwdRef->dwWidth       = vpHalSrcSurf->dwWidth;
            surface->pFwdRef->dwHeight      = vpHalSrcSurf->dwHeight;
            surface->pFwdRef->dwPitch       = vpHalSrcSurf->dwPitch;
            surface->uFwdRefCount           = pipelineParam->num_backward_references - i;
        }
        refSurfBuffObj = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, pipelineParam->backward_references[i]);
        DDI_VP_CHK_NULL(refSurfBuffObj, "nullptr refSurfBuffObj!", VA_STATUS_ERROR_INVALID_SURFACE);

        surface->pFwdRef->OsResource.bo          = refSurfBuffObj->bo;
        surface->pFwdRef->OsResource.Format      = MediaLibvaUtilNext::GetFormatFromMediaFormat(refSurfBuffObj->format);
        surface->pFwdRef->OsResource.iWidth      = refSurfBuffObj->iWidth;
        surface->pFwdRef->OsResource.iHeight     = refSurfBuffObj->iHeight;
        surface->pFwdRef->OsResource.iPitch      = refSurfBuffObj->iPitch;
        surface->pFwdRef->OsResource.TileType    = MediaLibvaUtilNext::GetTileTypeFromMediaTileType(refSurfBuffObj->TileType);
        surface->pFwdRef->OsResource.pGmmResInfo = refSurfBuffObj->pGmmResourceInfo;

        Mos_Solo_SetOsResource(refSurfBuffObj->pGmmResourceInfo, &surface->OsResource);

        surface->pFwdRef->FrameID = refSurfBuffObj->frame_idx;
        surface = surface->pFwdRef;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiUpdateProcPipelinePastReferenceFrames(
    PDDI_VP_CONTEXT               vpCtx,
    VADriverContextP              vaDrvCtx,
    PVPHAL_SURFACE                vpHalSrcSurf,
    VAProcPipelineParameterBuffer *pipelineParam)
{
    PVPHAL_SURFACE       surface        = nullptr;
    PDDI_MEDIA_SURFACE   refSurfBuffObj = nullptr;
    PDDI_MEDIA_CONTEXT   mediaCtx       = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx,         "nullptr vpCtx!",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vaDrvCtx,      "nullptr vaDrvCtx!",      VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpHalSrcSurf,  "nullptr vpHalSrcSurf!",  VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(pipelineParam, "nullptr pipelineParam!", VA_STATUS_ERROR_INVALID_PARAMETER);

    // initialize
    mediaCtx = GetMediaContext(vaDrvCtx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx!", VA_STATUS_ERROR_INVALID_CONTEXT);

    surface = vpHalSrcSurf;
    if(!pipelineParam->forward_references)
    {
        DDI_VP_NORMALMESSAGE("nullptr pipelineParam->backward_references");
        return VA_STATUS_SUCCESS;
    }

    for (uint32_t i = 0; i < pipelineParam->num_forward_references; i++)
    {
        if (surface->pBwdRef == nullptr)
        {
            surface->pBwdRef = MOS_New(VPHAL_SURFACE);
            DDI_VP_CHK_NULL(surface->pBwdRef, "nullptr surface->pBwdRef!", VA_STATUS_ERROR_ALLOCATION_FAILED);

            surface->pBwdRef->Format        = vpHalSrcSurf->Format;
            surface->pBwdRef->SurfType      = vpHalSrcSurf->SurfType;
            surface->pBwdRef->rcSrc         = vpHalSrcSurf->rcSrc;
            surface->pBwdRef->rcDst         = vpHalSrcSurf->rcDst;
            surface->pBwdRef->ColorSpace    = vpHalSrcSurf->ColorSpace;
            surface->pBwdRef->ExtendedGamut = vpHalSrcSurf->ExtendedGamut;
            surface->pBwdRef->SampleType    = vpHalSrcSurf->SampleType;
            surface->pBwdRef->ScalingMode   = vpHalSrcSurf->ScalingMode;
            surface->pBwdRef->OsResource    = vpHalSrcSurf->OsResource;
            surface->pBwdRef->dwWidth       = vpHalSrcSurf->dwWidth;
            surface->pBwdRef->dwHeight      = vpHalSrcSurf->dwHeight;
            surface->pBwdRef->dwPitch       = vpHalSrcSurf->dwPitch;
            surface->uBwdRefCount           = pipelineParam->num_forward_references - i;
        }
        refSurfBuffObj = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, pipelineParam->forward_references[i]);
        DDI_VP_CHK_NULL(refSurfBuffObj, "nullptr refSurfBuffObj!", VA_STATUS_ERROR_INVALID_SURFACE);

        surface->pBwdRef->OsResource.bo          = refSurfBuffObj->bo;
        surface->pBwdRef->OsResource.Format      = MediaLibvaUtilNext::GetFormatFromMediaFormat(refSurfBuffObj->format);
        surface->pBwdRef->OsResource.iWidth      = refSurfBuffObj->iWidth;
        surface->pBwdRef->OsResource.iHeight     = refSurfBuffObj->iHeight;
        surface->pBwdRef->OsResource.iPitch      = refSurfBuffObj->iPitch;
        surface->pBwdRef->OsResource.TileType    = MediaLibvaUtilNext::GetTileTypeFromMediaTileType(refSurfBuffObj->TileType);
        surface->pBwdRef->OsResource.pGmmResInfo = refSurfBuffObj->pGmmResourceInfo;

        Mos_Solo_SetOsResource(refSurfBuffObj->pGmmResourceInfo, &surface->OsResource);

        surface->pBwdRef->FrameID = refSurfBuffObj->frame_idx;
        surface = surface->pBwdRef;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiUpdateFilterParamBuffer(
    VADriverContextP vaDrvCtx,
    PDDI_VP_CONTEXT  vpCtx,
    uint32_t         surfIndex,
    int32_t          filterType,
    void             *data,
    uint32_t         elementNum,
    DDI_VP_STATE     *vpStateFlags)
{
    VAStatus vaStatus = VA_STATUS_SUCCESS;
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpStateFlags, "nullptr vpStateFlags.", VA_STATUS_ERROR_INVALID_PARAMETER);

    switch (filterType)
    {
    case VAProcFilterDeinterlacing:
        vpStateFlags->bDeinterlaceEnable = true;
        vaStatus                         = DdiSetProcFilterDinterlaceParams(vpCtx, surfIndex, (VAProcFilterParameterBufferDeinterlacing *)data);
        break;
    case VAProcFilterNoiseReduction:
        vpStateFlags->bDenoiseEnable = true;
        vaStatus                     = DdiSetProcFilterDenoiseParams(vpCtx, surfIndex, (VAProcFilterParameterBuffer *)data);
        break;
    case VAProcFilterHVSNoiseReduction:
        vpStateFlags->bDenoiseEnable = true;
        vaStatus                     = DdiSetProcFilterHVSDenoiseParams(vpCtx, surfIndex, (VAProcFilterParameterBufferHVSNoiseReduction *)data);
        break;
    case VAProcFilterSharpening:
        vpStateFlags->bIEFEnable = true;
        vaStatus                 = DdiSetProcFilterSharpnessParams(vpCtx, surfIndex, (VAProcFilterParameterBuffer *)data);
        break;
    case VAProcFilterColorBalance:
        vpStateFlags->bProcampEnable = true;
        vaStatus                     = DdiSetProcFilterColorBalanceParams(vpCtx, surfIndex, (VAProcFilterParameterBufferColorBalance *)data, elementNum);
        break;
    case VAProcFilterSkinToneEnhancement:
        vaStatus = DdiSetProcFilterSkinToneEnhancementParams(vpCtx, surfIndex, (VAProcFilterParameterBuffer *)data);
        break;
    case VAProcFilterTotalColorCorrection:
        vaStatus = DdiSetProcFilterTotalColorCorrectionParams(vpCtx, surfIndex, (VAProcFilterParameterBufferTotalColorCorrection *)data, elementNum);
        break;
    case VAProcFilterHighDynamicRangeToneMapping:
        vaStatus = DdiSetProcFilterHdrTmParams(vpCtx, surfIndex, (VAProcFilterParameterBufferHDRToneMapping *)data);
        break;
#if VA_CHECK_VERSION(1, 12, 0)
    case VAProcFilter3DLUT:
        vaStatus = DdiSetProcFilter3DLutParams(vaDrvCtx, vpCtx, surfIndex, (VAProcFilterParameterBuffer3DLUT *)data);
        break;
#endif
    case VAProcFilterNone:
        vaStatus = VA_STATUS_ERROR_INVALID_PARAMETER;
        break;
    default:
        DDI_VP_ASSERTMESSAGE("VAProcFilterType is unknown.");
        vaStatus = VA_STATUS_ERROR_UNSUPPORTED_FILTER;
        break;
    }  // switch (type)

    return vaStatus;
}

VAStatus DdiVpFunctions::DdiSetProcFilterDinterlaceParams(
    PDDI_VP_CONTEXT                          vpCtx,
    uint32_t                                 surfIndex,
    VAProcFilterParameterBufferDeinterlacing *diParamBuff)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       targetSurf        = nullptr;
    PVPHAL_SURFACE       srcSurf           = nullptr;
    VPHAL_DI_MODE        diMode;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx,       "nullptr vpCtx.",       VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(diParamBuff, "nullptr diParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    srcSurf = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(srcSurf, "nullptr srcSurf.", VA_STATUS_ERROR_INVALID_SURFACE);
    targetSurf = vpHalRenderParams->pTarget[0];
    DDI_VP_CHK_NULL(targetSurf, "nullptr targetSurf.", VA_STATUS_ERROR_INVALID_SURFACE);

    switch (diParamBuff->algorithm)
    {
    case VAProcDeinterlacingBob:
        diMode = DI_MODE_BOB;
        break;
    case VAProcDeinterlacingMotionAdaptive:
    case VAProcDeinterlacingMotionCompensated:
        diMode = DI_MODE_ADI;
        break;
    case VAProcDeinterlacingWeave:
        srcSurf->bFieldWeaving = true;
        return VA_STATUS_SUCCESS;
    case VAProcDeinterlacingNone:
        return VA_STATUS_SUCCESS;
    default:
        DDI_VP_ASSERTMESSAGE("Deinterlacing type is unsupported.");
        return VA_STATUS_ERROR_UNIMPLEMENTED;
    }  // switch (diParamBuff->algorithm)

    if (nullptr == srcSurf->pDeinterlaceParams)
    {
        srcSurf->pDeinterlaceParams = MOS_New(VPHAL_DI_PARAMS);
        DDI_VP_CHK_NULL(srcSurf->pDeinterlaceParams, "srcSurf->pDeinterlaceParams is nullptr", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    if (nullptr == targetSurf->pDeinterlaceParams)
    {
        targetSurf->pDeinterlaceParams = MOS_New(VPHAL_DI_PARAMS);
        DDI_VP_CHK_NULL(targetSurf->pDeinterlaceParams, "targetSurf->pDeinterlaceParams is nullptr", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }
    //application detect scene change and then pass parameter to driver.
    if (diParamBuff->flags & VA_DEINTERLACING_SCD_ENABLE)
    {
        diMode                                  = DI_MODE_BOB;
        srcSurf->pDeinterlaceParams->bSCDEnable = true;
    }
    else
    {
        srcSurf->pDeinterlaceParams->bSCDEnable = false;
    }

    srcSurf->pDeinterlaceParams->DIMode       = diMode;
    srcSurf->pDeinterlaceParams->bSingleField = (diParamBuff->flags & VA_DEINTERLACING_ONE_FIELD) ? true : false;
    srcSurf->pDeinterlaceParams->bEnableFMD   = (diParamBuff->flags & VA_DEINTERLACING_FMD_ENABLE) ? true : false;

    //update sample type
    UpdateSampleType(srcSurf, diParamBuff->flags);

    if (srcSurf->pDeinterlaceParams->DIMode != DI_MODE_ADI)
    {
        return VA_STATUS_SUCCESS;
    }

    //When pBwdRef is not nullptr and uBwdRefCount is nonzero, ADI can use Bwd Ref frame.
    //Otherwise, ADI shouldn't use Bwd Ref frame.
    if (srcSurf->uBwdRefCount && srcSurf->pBwdRef != nullptr)
    {
        SetFrameID(vpCtx, srcSurf);
    }
    else
    {
        //ADI no reference frame driver only care EVEN/ODD
        if (diParamBuff->flags & VA_DEINTERLACING_BOTTOM_FIELD)
        {
            srcSurf->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
        }
        else
        {
            srcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
        }
    }

    //int32_t overflow process
    vpCtx->FrameIDTracer.uiFrameIndex = (vpCtx->FrameIDTracer.uiFrameIndex + 1 == INT_MAX) ? 1 : vpCtx->FrameIDTracer.uiFrameIndex + 1;

    return VA_STATUS_SUCCESS;
}

void DdiVpFunctions::VpSetColorSpaceByDefault(PVPHAL_SURFACE  vpHalSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSurf, "nullptr vpHalSurf.", );

    if ((vpHalSurf->rcSrc.right - vpHalSurf->rcSrc.left) <= 1280 && (vpHalSurf->rcSrc.bottom - vpHalSurf->rcSrc.top) <= 720)
    {
        vpHalSurf->ColorSpace = CSpace_BT601;
    }  //720p
    else if ((vpHalSurf->rcSrc.right - vpHalSurf->rcSrc.left) <= 1920 && (vpHalSurf->rcSrc.bottom - vpHalSurf->rcSrc.top) <= 1080)
    {
        vpHalSurf->ColorSpace = CSpace_BT709;
    }  //1080p
    else
    {
        if (vpHalSurf->Format == Format_P010 || vpHalSurf->Format == Format_P016)
        {
            vpHalSurf->ColorSpace = CSpace_BT2020;
        }
        else
        {
            vpHalSurf->ColorSpace = CSpace_BT709;
        }
    }  //4K
    return;
}

void DdiVpFunctions::VpSetColorSpaceByColorStandard(
    PVPHAL_SURFACE          vpHalSurf,
    VAProcColorStandardType colorStandard,
    VAProcColorProperties   colorProperties,
    uint8_t                 colorRange)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSurf, "nullptr vpHalSurf.", );

    switch (colorStandard)
            {
            case VAProcColorStandardBT709:
#if (VA_MAJOR_VERSION < 1)
                if (flag & VA_SOURCE_RANGE_FULL)
#else
                if (colorRange == VA_SOURCE_RANGE_FULL)
#endif
                {
                    vpHalSurf->ColorSpace = CSpace_BT709_FullRange;
                }
                else
                {
                    vpHalSurf->ColorSpace = CSpace_BT709;
                }
                break;
            case VAProcColorStandardBT601:
#if (VA_MAJOR_VERSION < 1)
                if (flag & VA_SOURCE_RANGE_FULL)
#else
                if (colorRange == VA_SOURCE_RANGE_FULL)
#endif
                {
                    vpHalSurf->ColorSpace = CSpace_BT601_FullRange;
                }
                else
                {
                    vpHalSurf->ColorSpace = CSpace_BT601;
                }
                break;
            case VAProcColorStandardBT2020:
#if (VA_MAJOR_VERSION < 1)
                if (flag & VA_SOURCE_RANGE_FULL)
#else
                if (colorRange == VA_SOURCE_RANGE_FULL)
#endif
                {
                    vpHalSurf->ColorSpace = CSpace_BT2020_FullRange;
                }
                else
                {
                    vpHalSurf->ColorSpace = CSpace_BT2020;
                }
                break;
            case VAProcColorStandardBT470M:
            case VAProcColorStandardBT470BG:
            case VAProcColorStandardSMPTE170M:
            case VAProcColorStandardSMPTE240M:
            case VAProcColorStandardGenericFilm:
            case VAProcColorStandardXVYCC601:
            case VAProcColorStandardXVYCC709:
                vpHalSurf->ColorSpace = CSpace_None;
                break;
            case VAProcColorStandardExplicit:
                VpSetColorStandardExplictly(vpHalSurf, colorStandard, colorProperties);
                break;
            default:
                vpHalSurf->ColorSpace = CSpace_BT601;
                break;
            }
    return;
}

void DdiVpFunctions::UpdateSampleType(
    PVPHAL_SURFACE srcSurf,
    uint32_t       flags)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(srcSurf, "nullptr srcSurf.", );
    if (flags & VA_DEINTERLACING_BOTTOM_FIELD_FIRST)
    {
        if (flags & VA_DEINTERLACING_BOTTOM_FIELD)
        {
            srcSurf->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
        }
        else
        {
            srcSurf->SampleType = SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD;
        }
    }
    else
    {
        if (flags & VA_DEINTERLACING_BOTTOM_FIELD)
        {
            srcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD;
        }
        else
        {
            srcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
        }
    }
    return;
}

void DdiVpFunctions::SetFrameID(PDDI_VP_CONTEXT vpCtx, PVPHAL_SURFACE srcSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", );
    DDI_VP_CHK_NULL(srcSurf, "nullptr srcSurf.", );

    srcSurf->uBwdRefCount = 1;

    //When the bo of the current frame's SRC and reference are same with previous frame's,
    //we should set the frame ID same as previous frame's setting.
    if (vpCtx->FrameIDTracer.pLastSrcSurfBo == srcSurf->OsResource.bo &&
        vpCtx->FrameIDTracer.pLastBwdSurfBo == srcSurf->pBwdRef->OsResource.bo &&
        vpCtx->FrameIDTracer.uiLastSampleType != srcSurf->SampleType)
    {
        srcSurf->FrameID          = vpCtx->FrameIDTracer.uiLastSrcSurfFrameID;
        srcSurf->pBwdRef->FrameID = vpCtx->FrameIDTracer.uiLastBwdSurfFrameID;
    }
    //Otherwise, we should update the values of frame ID and FrameID tracer.
    else
    {
        srcSurf->pBwdRef->FrameID = (VP_SETTING_SAME_SAMPLE_THRESHOLD + 1) * vpCtx->FrameIDTracer.uiFrameIndex;
        srcSurf->FrameID          = srcSurf->pBwdRef->FrameID + (VP_SETTING_SAME_SAMPLE_THRESHOLD + 1);

        vpCtx->FrameIDTracer.pLastSrcSurfBo = srcSurf->OsResource.bo;
        vpCtx->FrameIDTracer.pLastBwdSurfBo = srcSurf->pBwdRef->OsResource.bo;

        vpCtx->FrameIDTracer.uiLastSrcSurfFrameID = srcSurf->FrameID;
        vpCtx->FrameIDTracer.uiLastBwdSurfFrameID = srcSurf->pBwdRef->FrameID;
        vpCtx->FrameIDTracer.uiLastSampleType     = srcSurf->SampleType;
    }
    return;
}

VAStatus DdiVpFunctions::DdiSetProcFilterDenoiseParams(
    PDDI_VP_CONTEXT             vpCtx,
    uint32_t                    surfIndex,
    VAProcFilterParameterBuffer *dnParamBuff)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       src               = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    src = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(src, "nullptr src.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (nullptr == src->pDenoiseParams)
    {
        src->pDenoiseParams = MOS_New(VPHAL_DENOISE_PARAMS);
    }
    DDI_VP_CHK_NULL(src->pDenoiseParams, "MOS_New pDenoiseParams failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // denoise caps range is from 0 to 64, out of range parameter is treated as an error
    if (dnParamBuff->value < NOISEREDUCTION_MIN || dnParamBuff->value > NOISEREDUCTION_MAX)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    src->pDenoiseParams->fDenoiseFactor = dnParamBuff->value;

    // Luma and chroma denoise should be always enabled when noise reduction is needed
    src->pDenoiseParams->bEnableLuma   = true;
    src->pDenoiseParams->bEnableChroma = true;
    src->pDenoiseParams->bAutoDetect   = false;
    src->pDenoiseParams->NoiseLevel    = NOISELEVEL_DEFAULT;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiSetProcFilterHVSDenoiseParams(
    PDDI_VP_CONTEXT                              vpCtx,
    uint32_t                                     surfIndex,
    VAProcFilterParameterBufferHVSNoiseReduction *hvsDnParamBuff)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       src              = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx,          "nullptr vpCtx.",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(hvsDnParamBuff, "nullptr hvsDnParamBuff.", VA_STATUS_ERROR_INVALID_PARAMETER);

    // initialize
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    src = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(src, "nullptr src.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (nullptr == src->pDenoiseParams)
    {
        src->pDenoiseParams = MOS_New(VPHAL_DENOISE_PARAMS);
    }
    DDI_VP_CHK_NULL(src->pDenoiseParams, "MOS_New pDenoiseParams failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);

    // Luma and chroma denoise should be always enabled when noise reduction is needed
    src->pDenoiseParams->bEnableLuma        = true;
    src->pDenoiseParams->bEnableChroma      = true;
    src->pDenoiseParams->bEnableHVSDenoise  = true;

    SetHVSDnParams(hvsDnParamBuff, src->pDenoiseParams);

    DDI_VP_NORMALMESSAGE("HVS Denoise is enabled with qp %d, strength %d, mode %d!", src->pDenoiseParams->HVSDenoise.QP, src->pDenoiseParams->HVSDenoise.Strength, src->pDenoiseParams->HVSDenoise.Mode);

    return VA_STATUS_SUCCESS;
}

void DdiVpFunctions::SetHVSDnParams(
    VAProcFilterParameterBufferHVSNoiseReduction *hvsDnParamBuff,
    PVPHAL_DENOISE_PARAMS                        denoiseParams)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(hvsDnParamBuff, "nullptr vpCtx.",);
    DDI_VP_CHK_NULL(denoiseParams,  "nullptr hvsDnParamBuff.", );

    switch (hvsDnParamBuff->mode)
    {
    case VA_PROC_HVS_DENOISE_AUTO_SUBJECTIVE:
        denoiseParams->HVSDenoise.Mode = HVSDENOISE_AUTO_SUBJECTIVE;
        denoiseParams->bAutoDetect     = true;
        break;
    case VA_PROC_HVS_DENOISE_MANUAL:
        denoiseParams->HVSDenoise.Mode = HVSDENOISE_MANUAL;
        break;
    case VA_PROC_HVS_DENOISE_DEFAULT:
    case VA_PROC_HVS_DENOISE_AUTO_BDRATE:
    default:
        denoiseParams->HVSDenoise.Mode = HVSDENOISE_AUTO_BDRATE;
        denoiseParams->bAutoDetect     = true;
    }  // switch (hvsDnParamBuff->mode)

    if (denoiseParams->HVSDenoise.Mode == HVSDENOISE_AUTO_BDRATE)
    {
        denoiseParams->HVSDenoise.QP = hvsDnParamBuff->qp;
        if (denoiseParams->HVSDenoise.QP == 0)
        {
            //If didn't set value, HVS Auto Bdrate Mode default qp 27
            denoiseParams->HVSDenoise.QP = 27;
        }
    }
    else if (denoiseParams->HVSDenoise.Mode == HVSDENOISE_AUTO_SUBJECTIVE)
    {
        //HVS Subjective Mode default qp 32
        denoiseParams->HVSDenoise.QP = 32;
    }
    else
    {
        denoiseParams->HVSDenoise.QP       = 32;
        denoiseParams->HVSDenoise.Strength = hvsDnParamBuff->strength;
    }
    return;
}

MOS_STATUS DdiVpFunctions::VpHalDdiInitIEFParams(
    PVPHAL_IEF_PARAMS       iefParams)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(iefParams, "nullptr iefParams", MOS_STATUS_INVALID_PARAMETER);

    // Init default values for flows where user feature key is not available or used
    iefParams->bSkintoneTuned        = true;
    iefParams->bEmphasizeSkinDetail  = false;
    iefParams->bSmoothMode           = false;
    iefParams->StrongEdgeWeight      = IEF_STRONG_EDGE_WEIGHT;
    iefParams->RegularWeight         = IEF_REGULAR_WEIGHT;
    iefParams->StrongEdgeThreshold   = IEF_STRONG_EDGE_THRESHOLD;

    return MOS_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiSetProcFilterSharpnessParams(
    PDDI_VP_CONTEXT             vpCtx,
    uint32_t                    surfIndex,
    VAProcFilterParameterBuffer *sharpParamBuff)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       src               = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx,          "nullptr vpCtx.",          VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(sharpParamBuff, "nullptr sharpParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    src = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(src, "nullptr src.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (nullptr == src->pIEFParams)
    {
        src->pIEFParams = MOS_New(VPHAL_IEF_PARAMS);
        DDI_VP_CHK_NULL(src->pIEFParams, "MOS_New pIEFParams failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    // out of range parameter is treated as an error
    if (sharpParamBuff->value < EDGEENHANCEMENT_MIN || sharpParamBuff->value > EDGEENHANCEMENT_MAX)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    // sharpness option
    // setting with hard code in mplayer side.
    // refer to sharpening section of render_picture_vp() defined in vaapi.c.
    // change flag and factor for testing.
    VpHalDdiInitIEFParams(src->pIEFParams);
    src->bIEF                    = true;
    src->pIEFParams->bEnabled    = true;
    src->pIEFParams->fIEFFactor  = sharpParamBuff->value;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiSetProcFilterColorBalanceParams(
    PDDI_VP_CONTEXT                         vpCtx,
    uint32_t                                surfIndex,
    VAProcFilterParameterBufferColorBalance *colorBalanceParamBuff,
    uint32_t                                elementNum)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       src               = nullptr;
    bool                 procamp           = false;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx,                 "nullptr vpCtx.",                 VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(colorBalanceParamBuff, "nullptr colorBalanceParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    src = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(src, "nullptr src.", VA_STATUS_ERROR_INVALID_SURFACE);

    for (uint32_t i = 0; i < elementNum; i++)
    {
        if ((VAProcColorBalanceHue == colorBalanceParamBuff[i].attrib) ||
            (VAProcColorBalanceSaturation == colorBalanceParamBuff[i].attrib) ||
            (VAProcColorBalanceBrightness == colorBalanceParamBuff[i].attrib) ||
            (VAProcColorBalanceContrast == colorBalanceParamBuff[i].attrib))
        {
            procamp = true;
            break;
        }
    }

    if (nullptr == src->pProcampParams && true == procamp)
    {
        src->pProcampParams = MOS_New(VPHAL_PROCAMP_PARAMS);
        DDI_VP_CHK_NULL(src->pProcampParams, "MOS_New Source pProcampParams failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    if (nullptr == vpHalRenderParams->pTarget[0]->pProcampParams)
    {
        vpHalRenderParams->pTarget[0]->pProcampParams = MOS_New(VPHAL_PROCAMP_PARAMS);
        DDI_VP_CHK_NULL(vpHalRenderParams->pTarget[0]->pProcampParams, "MOS_New Target pProcampParams failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    // Needed for ACE
    if (nullptr == src->pColorPipeParams && SURF_IN_PRIMARY == src->SurfType)
    {
        src->pColorPipeParams = MOS_New(VPHAL_COLORPIPE_PARAMS);
        DDI_VP_CHK_NULL(src->pColorPipeParams, "MOS_New pColorPipeParams failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    // set default value
    if (nullptr != src->pProcampParams)
    {
        src->pProcampParams->fHue         = PROCAMP_HUE_DEFAULT;
        src->pProcampParams->fSaturation  = PROCAMP_SATURATION_DEFAULT;
        src->pProcampParams->fBrightness  = PROCAMP_BRIGHTNESS_DEFAULT;
        src->pProcampParams->fContrast    = PROCAMP_CONTRAST_DEFAULT;
    }

    for (uint32_t i = 0; i < elementNum; i++)
    {
        DDI_CHK_RET(SetColorBalanceParams(colorBalanceParamBuff, i, src, procamp), "Failed to Set ColorBalanceParams.");
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::SetColorBalanceParams(
    VAProcFilterParameterBufferColorBalance *colorBalanceParamBuff,
    uint32_t                                index,
    PVPHAL_SURFACE                          src,
    bool                                    procamp)
{
    DDI_VP_FUNC_ENTER;
    switch (colorBalanceParamBuff[index].attrib)
    {
        case VAProcColorBalanceHue:
        if (colorBalanceParamBuff[index].value < PROCAMP_HUE_MIN || colorBalanceParamBuff[index].value > PROCAMP_HUE_MAX)
        {
            DDI_VP_ASSERTMESSAGE("%d: Hue is out of bounds.", index);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (true == procamp)
        {
            src->pProcampParams->bEnabled = true;
            src->pProcampParams->fHue     = colorBalanceParamBuff[index].value;
        }
        break;
        case VAProcColorBalanceSaturation:
        if (colorBalanceParamBuff[index].value < PROCAMP_SATURATION_MIN || colorBalanceParamBuff[index].value > PROCAMP_SATURATION_MAX)
        {
            DDI_VP_ASSERTMESSAGE("%d: Saturation is out of bounds.", index);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (true == procamp)
        {
            src->pProcampParams->bEnabled     = true;
            src->pProcampParams->fSaturation = colorBalanceParamBuff[index].value;
        }
        break;
        case VAProcColorBalanceBrightness:
        if (colorBalanceParamBuff[index].value < PROCAMP_BRIGHTNESS_MIN || colorBalanceParamBuff[index].value > PROCAMP_BRIGHTNESS_MAX)
        {
            DDI_VP_ASSERTMESSAGE("%d: Brightness is out of bounds.", index);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (true == procamp)
        {
            src->pProcampParams->bEnabled     = true;
            src->pProcampParams->fBrightness = colorBalanceParamBuff[index].value;
        }
        break;
        case VAProcColorBalanceContrast:
        if (colorBalanceParamBuff[index].value < PROCAMP_CONTRAST_MIN || colorBalanceParamBuff[index].value > PROCAMP_CONTRAST_MAX)
        {
            DDI_VP_ASSERTMESSAGE("%d: Contrast is out of bounds.", index);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }
        if (true == procamp)
        {
            src->pProcampParams->bEnabled   = true;
            src->pProcampParams->fContrast = colorBalanceParamBuff[index].value;
        }
        break;
        case VAProcColorBalanceAutoContrast:
        if (SURF_IN_PRIMARY == src->SurfType)
        {
            src->pColorPipeParams->bEnableACE     = true;
            src->pColorPipeParams->dwAceLevel     = ACE_LEVEL_DEFAULT;
            src->pColorPipeParams->dwAceStrength  = ACE_STRENGTH_DEFAULT;
        }
        break;
        case VAProcColorBalanceAutoSaturation:
        case VAProcColorBalanceAutoBrightness:
            return VA_STATUS_ERROR_UNIMPLEMENTED;
        case VAProcColorBalanceNone:
        case VAProcColorBalanceCount:
        default:
            DDI_VP_ASSERTMESSAGE("colorBalanceParamBuff[%d].attrib is unknown.", index);
            return VA_STATUS_ERROR_INVALID_PARAMETER;
    }  // switch (attrib)
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiSetProcFilterSkinToneEnhancementParams(
    PDDI_VP_CONTEXT             vpCtx,
    uint32_t                    surfIndex,
    VAProcFilterParameterBuffer *steParamBuff)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       src               = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(steParamBuff, "Null steParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    src = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(src, "nullptr src.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (SURF_IN_PRIMARY == src->SurfType)
    {
        if (nullptr == src->pColorPipeParams)
        {
            src->pColorPipeParams = MOS_New(VPHAL_COLORPIPE_PARAMS);
            DDI_VP_CHK_NULL(src->pColorPipeParams, "MOS_New pColorPipeParams failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }

        // out of range parameter is treated as an error
        if (steParamBuff->value < STE_MIN || steParamBuff->value > STE_MAX)
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        src->pColorPipeParams->bEnableSTE            = true;
        src->pColorPipeParams->SteParams.dwSTEFactor = (uint32_t)steParamBuff->value;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiSetProcFilterTotalColorCorrectionParams(
    PDDI_VP_CONTEXT                                 vpCtx,
    uint32_t                                        surfIndex,
    VAProcFilterParameterBufferTotalColorCorrection *tccParamBuff,
    uint32_t                                        elementNum)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       src               = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(tccParamBuff, "nullptr tccParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    src = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(src, "nullptr src.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (SURF_IN_PRIMARY == src->SurfType)
    {
        if (nullptr == src->pColorPipeParams)
        {
            src->pColorPipeParams = MOS_New(VPHAL_COLORPIPE_PARAMS);
            DDI_VP_CHK_NULL(src->pColorPipeParams, "MOS_New pColorPipeParams failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }

        // set default values
        src->pColorPipeParams->TccParams.Red      = (uint8_t)TCC_DEFAULT;
        src->pColorPipeParams->TccParams.Green    = (uint8_t)TCC_DEFAULT;
        src->pColorPipeParams->TccParams.Blue     = (uint8_t)TCC_DEFAULT;
        src->pColorPipeParams->TccParams.Cyan     = (uint8_t)TCC_DEFAULT;
        src->pColorPipeParams->TccParams.Magenta  = (uint8_t)TCC_DEFAULT;
        src->pColorPipeParams->TccParams.Yellow   = (uint8_t)TCC_DEFAULT;

        for (uint32_t i = 0; i < elementNum; i++)
        {
            if (tccParamBuff[i].value < TCC_MIN || tccParamBuff[i].value > TCC_MAX)
            {
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }

            src->pColorPipeParams->bEnableTCC = true;

            switch (tccParamBuff[i].attrib)
            {
            case VAProcTotalColorCorrectionRed:
                src->pColorPipeParams->TccParams.Red = (uint8_t)tccParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionGreen:
                src->pColorPipeParams->TccParams.Green = (uint8_t)tccParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionBlue:
                src->pColorPipeParams->TccParams.Blue = (uint8_t)tccParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionCyan:
                src->pColorPipeParams->TccParams.Cyan = (uint8_t)tccParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionMagenta:
                src->pColorPipeParams->TccParams.Magenta = (uint8_t)tccParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionYellow:
                src->pColorPipeParams->TccParams.Yellow = (uint8_t)tccParamBuff[i].value;
                break;

            case VAProcTotalColorCorrectionNone:
            case VAProcTotalColorCorrectionCount:
            default:
                DDI_VP_ASSERTMESSAGE("tccParamBuff[%d].attrib is unknown.", i);
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiSetProcFilterHdrTmParams(
    PDDI_VP_CONTEXT                           vpCtx,
    uint32_t                                  surfIndex,
    VAProcFilterParameterBufferHDRToneMapping *hdrTmParamBuff)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       src               = nullptr;
    VAStatus             eStatus;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(hdrTmParamBuff, "nullptr hdrTmParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);

    // initialize
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    src = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(src, "nullptr src.", VA_STATUS_ERROR_INVALID_SURFACE);

    eStatus = VpUpdateProcHdrState(src, &hdrTmParamBuff->data);

    return eStatus;
}

VAStatus DdiVpFunctions::VpUpdateProcHdrState(
    const PVPHAL_SURFACE vpHalSurf,
    const VAHdrMetaData *hdrMetadata)
{
    VAHdrMetaDataHDR10 *hdr10MetaData = nullptr;
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSurf, "Null vpHalSurf.", VA_STATUS_ERROR_INVALID_BUFFER);

    // pass HDR metadata
    if ((hdrMetadata != nullptr) && (hdrMetadata->metadata_size != 0))
    {
        if (vpHalSurf->pHDRParams == nullptr)
        {
            vpHalSurf->pHDRParams = MOS_New(VPHAL_HDR_PARAMS);
            DDI_VP_CHK_NULL(vpHalSurf->pHDRParams, "MOS_New VPHAL_HDR_PARAMS failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }

        // HDR10 Meta Data
        if (hdrMetadata->metadata_type == VAProcHighDynamicRangeMetadataHDR10)
        {
            DDI_VP_NORMALMESSAGE("VpSetHdrParams HDR10 metadata.");
            hdr10MetaData = (VAHdrMetaDataHDR10 *)hdrMetadata->metadata;
            if (hdr10MetaData)
            {
                vpHalSurf->pHDRParams->white_point_x = hdr10MetaData->white_point_x;
                vpHalSurf->pHDRParams->white_point_y = hdr10MetaData->white_point_y;
                DDI_VP_NORMALMESSAGE("hdr10MetaData white_point_x %d, white_point_y %d.", hdr10MetaData->white_point_x, hdr10MetaData->white_point_y);
             
                // From VAAPI defintion which is following video spec, max/min_display_mastering_luminance are in units of 0.0001 candelas per square metre.
                uint32_t max_display_mastering_luminance = (hdr10MetaData->max_display_mastering_luminance > 655350000 ) ? 655350000 : hdr10MetaData->max_display_mastering_luminance;
                uint32_t min_display_mastering_luminance = (hdr10MetaData->min_display_mastering_luminance > 655350000 ) ? 655350000 : hdr10MetaData->min_display_mastering_luminance;

                vpHalSurf->pHDRParams->max_display_mastering_luminance = (uint16_t)(max_display_mastering_luminance / 10000);;
                vpHalSurf->pHDRParams->min_display_mastering_luminance = (uint16_t)(min_display_mastering_luminance / 10000);
                DDI_VP_NORMALMESSAGE("hdr10MetaData max_display_mastering_luminance %d, min_display_mastering_luminance %d.", hdr10MetaData->max_display_mastering_luminance, hdr10MetaData->min_display_mastering_luminance);

                vpHalSurf->pHDRParams->MaxCLL  = hdr10MetaData->max_content_light_level;
                vpHalSurf->pHDRParams->MaxFALL = hdr10MetaData->max_pic_average_light_level;
                DDI_VP_NORMALMESSAGE("hdr10MetaData MaxCLL %d, MaxFALL %d.", hdr10MetaData->max_content_light_level, hdr10MetaData->max_pic_average_light_level);

                vpHalSurf->pHDRParams->bAutoMode = false;
                vpHalSurf->pHDRParams->MaxCLL  = (vpHalSurf->pHDRParams->MaxCLL == 0) ? HDR_DEFAULT_MAXCLL : vpHalSurf->pHDRParams->MaxCLL;
                vpHalSurf->pHDRParams->MaxFALL = (vpHalSurf->pHDRParams->MaxFALL == 0) ? HDR_DEFAULT_MAXFALL : vpHalSurf->pHDRParams->MaxFALL;

                MOS_SecureMemcpy(vpHalSurf->pHDRParams->display_primaries_x, 3 * sizeof(uint16_t), hdr10MetaData->display_primaries_x, 3 * sizeof(uint16_t));
                MOS_SecureMemcpy(vpHalSurf->pHDRParams->display_primaries_y, 3 * sizeof(uint16_t), hdr10MetaData->display_primaries_y, 3 * sizeof(uint16_t));

                switch (vpHalSurf->GammaType)
                {
                case VPHAL_GAMMA_SMPTE_ST2084:
                    vpHalSurf->pHDRParams->EOTF = VPHAL_HDR_EOTF_SMPTE_ST2084;
                    break;
                case VPHAL_GAMMA_BT1886:
                    vpHalSurf->pHDRParams->EOTF = VPHAL_HDR_EOTF_BT1886;
                    break;
                default:
                    vpHalSurf->pHDRParams->EOTF = VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR;
                    break;
                }
                DDI_VP_NORMALMESSAGE("max_display_mastering_luminance %d.", vpHalSurf->pHDRParams->max_display_mastering_luminance);
                DDI_VP_NORMALMESSAGE("min_display_mastering_luminance %d.", vpHalSurf->pHDRParams->min_display_mastering_luminance);
                DDI_VP_NORMALMESSAGE("GammaType %d.", vpHalSurf->GammaType);
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

#if VA_CHECK_VERSION(1, 12, 0)
VAStatus DdiVpFunctions::DdiSetProcFilter3DLutParams(
    VADriverContextP                 vaDrvCtx,
    PDDI_VP_CONTEXT                  vpCtx,
    uint32_t                         surfIndex,
    VAProcFilterParameterBuffer3DLUT *lut3DParamBuff)
{
    VAStatus             eStatus           = VA_STATUS_SUCCESS;
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       src               = nullptr;
    PDDI_MEDIA_SURFACE   media3DLutSurf    = nullptr;
    PDDI_MEDIA_CONTEXT   mediaCtx          = nullptr;
    PMOS_RESOURCE        osResource        = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(lut3DParamBuff, "nullptr lut3DParamBuff.", VA_STATUS_ERROR_INVALID_BUFFER);
    DDI_VP_CHK_NULL(vaDrvCtx,       "nullptr vaDrvCtx.",       VA_STATUS_ERROR_INVALID_BUFFER);

    mediaCtx = GetMediaContext(vaDrvCtx);

    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    src = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(src, "nullptr src.", VA_STATUS_ERROR_INVALID_SURFACE);

    media3DLutSurf = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, lut3DParamBuff->lut_surface);
    DDI_VP_CHK_NULL(media3DLutSurf, "nullptr media3DLutSurf.", VA_STATUS_ERROR_INVALID_SURFACE);

    // only primary surface takes effect if 3DLUT filter
    if (SURF_IN_PRIMARY == src->SurfType)
    {
        if (nullptr == src->p3DLutParams)
        {
            src->p3DLutParams = MOS_New(VPHAL_3DLUT_PARAMS);
            DDI_VP_CHK_NULL(src->p3DLutParams, "MOS_New p3DLutParams failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }

        src->p3DLutParams->LutSize            = (uint32_t)lut3DParamBuff->lut_size;
        src->p3DLutParams->BitDepthPerChannel = (uint16_t)lut3DParamBuff->bit_depth;
        src->p3DLutParams->ByteCountPerEntry  = (uint16_t)(lut3DParamBuff->num_channel * (src->p3DLutParams->BitDepthPerChannel / 8));
        if (nullptr == src->p3DLutParams->pExt3DLutSurface)
        {
            src->p3DLutParams->pExt3DLutSurface = MOS_New(VPHAL_SURFACE);
            DDI_VP_CHK_NULL(src->p3DLutParams->pExt3DLutSurface, "MOS_New p3DLutParams failed.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }

        if (src->p3DLutParams->pExt3DLutSurface)
        {
            osResource               = &(src->p3DLutParams->pExt3DLutSurface->OsResource);
            DDI_VP_CHK_NULL(osResource, "nullptr  osResource.", VA_STATUS_ERROR_INVALID_PARAMETER);

            MediaLibvaCommonNext::MediaSurfaceToMosResource(media3DLutSurf, osResource);
            Mos_Solo_SetOsResource(media3DLutSurf->pGmmResourceInfo, osResource);
        }
    }

    return eStatus;
}
#endif


VAStatus DdiVpFunctions::DdiClearFilterParamBuffer(
    PDDI_VP_CONTEXT vpCtx,
    uint32_t        surfIndex,
    DDI_VP_STATE    vpStateFlags)
{
    DDI_VP_FUNC_ENTER;
    if (!vpStateFlags.bProcampEnable)
    {
        MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pProcampParams);
    }
    if (!vpStateFlags.bDeinterlaceEnable)
    {
        MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pDeinterlaceParams);
    }
    if (!vpStateFlags.bDenoiseEnable)
    {
        MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pDenoiseParams);
    }
    if (!vpStateFlags.bIEFEnable)
    {
        if (vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pIEFParams)
        {
            if (!vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pIEFParams->pExtParam)
            {
                DDI_VP_ASSERTMESSAGE("vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pIEFParams->pExtParam is nullptr.");
            }
            MOS_Delete(vpCtx->pVpHalRenderParams->pSrc[surfIndex]->pIEFParams);
        }
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::VpSetInterpolationParams(
    PVPHAL_SURFACE        surface,
    uint32_t              interpolationflags)
{
    DDI_VP_CHK_NULL(surface, "nullptr surface.", VA_STATUS_ERROR_INVALID_SURFACE);
    switch (interpolationflags)
    {
#if VA_CHECK_VERSION(1, 9, 0)
    case VA_FILTER_INTERPOLATION_NEAREST_NEIGHBOR:
        surface->ScalingMode       = VPHAL_SCALING_NEAREST;
        break;
    case VA_FILTER_INTERPOLATION_BILINEAR:
        surface->ScalingMode       = VPHAL_SCALING_BILINEAR;
        break;
    case VA_FILTER_INTERPOLATION_ADVANCED:
    case VA_FILTER_INTERPOLATION_DEFAULT:
#endif
    default:
        surface->ScalingMode       = VPHAL_SCALING_AVS;
        break;
    }

    return VA_STATUS_SUCCESS;
}

void DdiVpFunctions::VpUpdateWeaveDI(PVPHAL_SURFACE vpHalSrcSurf, uint32_t filterFlags)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf", );
    if (vpHalSrcSurf->pDeinterlaceParams == nullptr)
    {
        if ((filterFlags & 0x00000004) || ((filterFlags & VA_TOP_FIELD) && vpHalSrcSurf->bFieldWeaving))
        {
            vpHalSrcSurf->SampleType = SAMPLE_SINGLE_TOP_FIELD;
            if (vpHalSrcSurf->pBwdRef != nullptr)
            {
                vpHalSrcSurf->pBwdRef->SampleType = SAMPLE_SINGLE_BOTTOM_FIELD;
            }
            vpHalSrcSurf->ScalingMode   = VPHAL_SCALING_BILINEAR;
            vpHalSrcSurf->bFieldWeaving = true;
        }

        if ((filterFlags & 0x00000008) || ((filterFlags & VA_BOTTOM_FIELD) && vpHalSrcSurf->bFieldWeaving))
        {
            vpHalSrcSurf->SampleType = SAMPLE_SINGLE_BOTTOM_FIELD;
            if (vpHalSrcSurf->pBwdRef != nullptr)
            {
                vpHalSrcSurf->pBwdRef->SampleType = SAMPLE_SINGLE_TOP_FIELD;
            }
            vpHalSrcSurf->ScalingMode   = VPHAL_SCALING_BILINEAR;
            vpHalSrcSurf->bFieldWeaving = true;
        }
    }
    return;
}

VAStatus DdiVpFunctions::SetSurfaceParamsTopFieldFirst(uint32_t surfaceFlag, PVPHAL_SURFACE vpHalSrcSurf, PVPHAL_SURFACE vpHalTgtSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (surfaceFlag & VA_TOP_FIELD_FIRST)
    {
        vpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_INTERLEAVED;
        vpHalSrcSurf->SampleType            = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
        vpHalSrcSurf->bInterlacedScaling    = true;
        vpHalSrcSurf->bFieldWeaving         = false;
    }
    else if (surfaceFlag & VA_TOP_FIELD)
    {
        vpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_FIELD;
        vpHalSrcSurf->SampleType            = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
        vpHalTgtSurf->SampleType            = SAMPLE_SINGLE_TOP_FIELD;
        vpHalSrcSurf->bInterlacedScaling    = false;
        vpHalSrcSurf->bFieldWeaving         = false;
    }
    else if (surfaceFlag & VA_BOTTOM_FIELD)
    {
        vpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_FIELD;
        vpHalSrcSurf->SampleType            = SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD;
        vpHalTgtSurf->SampleType            = SAMPLE_SINGLE_BOTTOM_FIELD;
        vpHalSrcSurf->bInterlacedScaling    = false;
        vpHalSrcSurf->bFieldWeaving         = false;
    }
    else
    {
        DDI_VP_ASSERTMESSAGE("output_surface_flag need to be set for interlaced scaling.");
        vpHalSrcSurf->SampleType            = SAMPLE_PROGRESSIVE;
        vpHalSrcSurf->InterlacedScalingType = ISCALING_NONE;
        vpHalSrcSurf->bInterlacedScaling    = false;
        vpHalSrcSurf->bFieldWeaving         = false;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::SetSurfaceParamsBottomFieldFirst(uint32_t surfaceFlag, PVPHAL_SURFACE vpHalSrcSurf, PVPHAL_SURFACE vpHalTgtSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (surfaceFlag & VA_BOTTOM_FIELD_FIRST)
    {
        vpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_INTERLEAVED;
        vpHalSrcSurf->SampleType            = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
        vpHalSrcSurf->bInterlacedScaling    = true;
        vpHalSrcSurf->bFieldWeaving         = false;
    }
    else if (surfaceFlag & VA_TOP_FIELD)
    {
        vpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_FIELD;
        vpHalSrcSurf->SampleType            = SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD;
        vpHalTgtSurf->SampleType            = SAMPLE_SINGLE_TOP_FIELD;
        vpHalSrcSurf->bInterlacedScaling    = false;
        vpHalSrcSurf->bFieldWeaving         = false;
    }
    else if (surfaceFlag & VA_BOTTOM_FIELD)
    {
        vpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_FIELD;
        vpHalSrcSurf->SampleType            = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
        vpHalTgtSurf->SampleType            = SAMPLE_SINGLE_BOTTOM_FIELD;
        vpHalSrcSurf->bInterlacedScaling    = false;
        vpHalSrcSurf->bFieldWeaving         = false;
    }
    else
    {
        DDI_VP_ASSERTMESSAGE("output_surface_flag need to be set for interlaced scaling.");
        vpHalSrcSurf->SampleType            = SAMPLE_PROGRESSIVE;
        vpHalSrcSurf->InterlacedScalingType = ISCALING_NONE;
        vpHalSrcSurf->bInterlacedScaling    = false;
        vpHalSrcSurf->bFieldWeaving         = false;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::SetSurfaceParamsTopField(uint32_t surfaceFlag, PVPHAL_SURFACE vpHalSrcSurf, PVPHAL_SURFACE vpHalTgtSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (surfaceFlag & VA_TOP_FIELD_FIRST)
    {
        vpHalSrcSurf->InterlacedScalingType = ISCALING_FIELD_TO_INTERLEAVED;
        vpHalSrcSurf->SampleType            = SAMPLE_SINGLE_TOP_FIELD;
        vpHalTgtSurf->SampleType            = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;

        DDI_VP_CHK_NULL(vpHalSrcSurf->pBwdRef, "No Ref Field!", VA_STATUS_ERROR_UNIMPLEMENTED);
        vpHalSrcSurf->pBwdRef->InterlacedScalingType = ISCALING_FIELD_TO_INTERLEAVED;
        vpHalSrcSurf->pBwdRef->SampleType            = SAMPLE_SINGLE_BOTTOM_FIELD;

        vpHalSrcSurf->bInterlacedScaling = false;
        vpHalSrcSurf->bFieldWeaving      = true;
    }
    else if (surfaceFlag & VA_TOP_FIELD)
    {
        vpHalSrcSurf->InterlacedScalingType = ISCALING_FIELD_TO_FIELD;
        vpHalSrcSurf->SampleType            = SAMPLE_SINGLE_TOP_FIELD;
        vpHalSrcSurf->bInterlacedScaling    = false;
        vpHalSrcSurf->bFieldWeaving         = false;
    }
    else
    {
        vpHalSrcSurf->SampleType         = SAMPLE_PROGRESSIVE;
        vpHalSrcSurf->bInterlacedScaling = false;
        vpHalSrcSurf->bFieldWeaving      = false;
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::SetSurfaceParamsBottomField(uint32_t surfaceFlag, PVPHAL_SURFACE vpHalSrcSurf, PVPHAL_SURFACE vpHalTgtSurf)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf", VA_STATUS_ERROR_INVALID_PARAMETER);
    if (surfaceFlag & VA_BOTTOM_FIELD_FIRST)
    {
        vpHalSrcSurf->InterlacedScalingType = ISCALING_FIELD_TO_INTERLEAVED;
        vpHalSrcSurf->SampleType            = SAMPLE_SINGLE_BOTTOM_FIELD;
        vpHalTgtSurf->SampleType            = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;

        DDI_VP_CHK_NULL(vpHalSrcSurf->pBwdRef, "No Ref Field!", VA_STATUS_ERROR_UNIMPLEMENTED);
        vpHalSrcSurf->pBwdRef->InterlacedScalingType = ISCALING_FIELD_TO_INTERLEAVED;
        vpHalSrcSurf->pBwdRef->SampleType            = SAMPLE_SINGLE_TOP_FIELD;

        vpHalSrcSurf->bInterlacedScaling = false;
        vpHalSrcSurf->bFieldWeaving      = true;
    }
    else if (surfaceFlag & VA_BOTTOM_FIELD)
    {
        vpHalSrcSurf->InterlacedScalingType = ISCALING_FIELD_TO_FIELD;
        vpHalSrcSurf->SampleType            = SAMPLE_SINGLE_BOTTOM_FIELD;
        vpHalSrcSurf->bInterlacedScaling    = false;
        vpHalSrcSurf->bFieldWeaving         = false;
    }
    else
    {
        vpHalSrcSurf->SampleType         = SAMPLE_PROGRESSIVE;
        vpHalSrcSurf->bInterlacedScaling = false;
        vpHalSrcSurf->bFieldWeaving      = false;
    }
    return VA_STATUS_SUCCESS;
}

void DdiVpFunctions::SetLegacyInterlaceScalingParams(PVPHAL_SURFACE vpHalSrcSurf, uint32_t filterFlags)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf", );

    if (vpHalSrcSurf->pDeinterlaceParams == nullptr && vpHalSrcSurf->InterlacedScalingType == ISCALING_NONE)
    {
        if (filterFlags & VA_TOP_FIELD)
        {
            vpHalSrcSurf->SampleType            = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
            vpHalSrcSurf->ScalingMode           = VPHAL_SCALING_AVS;
            vpHalSrcSurf->bInterlacedScaling    = true;
            vpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_INTERLEAVED;
        }
        else if (filterFlags & VA_BOTTOM_FIELD)
        {
            vpHalSrcSurf->SampleType            = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
            vpHalSrcSurf->ScalingMode           = VPHAL_SCALING_AVS;
            vpHalSrcSurf->bInterlacedScaling    = true;
            vpHalSrcSurf->InterlacedScalingType = ISCALING_INTERLEAVED_TO_INTERLEAVED;
        }

        // Kernel does not support 3-plane interlaced AVS, so for 3-plane interlaced scaling, need to use bilinear.
        if (vpHalSrcSurf->bInterlacedScaling && IS_PL3_FORMAT(vpHalSrcSurf->Format))
        {
            vpHalSrcSurf->ScalingMode = VPHAL_SCALING_BILINEAR;
        }
    }
    return;
}

VAStatus DdiVpFunctions::VpUpdateProcRotateState(PVPHAL_SURFACE vpHalSrcSurf, uint32_t rotationState)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf.", VA_STATUS_ERROR_INVALID_PARAMETER);

    switch (rotationState)
    {
    case VA_ROTATION_NONE:
        vpHalSrcSurf->Rotation = VPHAL_ROTATION_IDENTITY;
        break;
    case VA_ROTATION_90:
        vpHalSrcSurf->Rotation = VPHAL_ROTATION_90;
        break;
    case VA_ROTATION_180:
        vpHalSrcSurf->Rotation = VPHAL_ROTATION_180;
        break;
    case VA_ROTATION_270:
        vpHalSrcSurf->Rotation = VPHAL_ROTATION_270;
        break;
    default:
        DDI_VP_ASSERTMESSAGE("VpUpdateProcRotateState rotationState = %d is out of range.", rotationState);
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::VpUpdateProcMirrorState(PVPHAL_SURFACE vpHalSrcSurf, uint32_t mirrorState)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpHalSrcSurf,  "nullptr vpHalSrcSurf.",  VA_STATUS_ERROR_INVALID_PARAMETER);

    if(mirrorState > VA_MIRROR_VERTICAL)
    {
        DDI_VP_ASSERTMESSAGE("VpUpdateProcMirrorState mirrorState = %d is out of range.", mirrorState);
        DDI_VP_ASSERTMESSAGE("VpUpdateProcMirrorState reset mirrorState to VA_MIRROR_NONE.");
        mirrorState = VA_MIRROR_NONE;
    }

    // Rotation must be a valid angle
    switch(vpHalSrcSurf->Rotation)
    {
        case VPHAL_ROTATION_IDENTITY:
            if(mirrorState == VA_MIRROR_HORIZONTAL)
            {
                vpHalSrcSurf->Rotation = VPHAL_MIRROR_HORIZONTAL;
            }
            else if(mirrorState == VA_MIRROR_VERTICAL)
            {
                vpHalSrcSurf->Rotation = VPHAL_MIRROR_VERTICAL;
            }
            break;
        case VPHAL_ROTATION_90:
            if(mirrorState == VA_MIRROR_HORIZONTAL)
            {
                vpHalSrcSurf->Rotation = VPHAL_ROTATE_90_MIRROR_HORIZONTAL;
            }
            else if(mirrorState == VA_MIRROR_VERTICAL)
            {
                vpHalSrcSurf->Rotation = VPHAL_ROTATE_90_MIRROR_VERTICAL;
            }
            break;
        case VPHAL_ROTATION_180:
            if(mirrorState == VA_MIRROR_HORIZONTAL)
            {
                vpHalSrcSurf->Rotation =  VPHAL_MIRROR_VERTICAL;
            }
            else if(mirrorState == VA_MIRROR_VERTICAL)
            {
                vpHalSrcSurf->Rotation =  VPHAL_MIRROR_HORIZONTAL;
            }
            break;
        case VPHAL_ROTATION_270:
            if(mirrorState == VA_MIRROR_HORIZONTAL)
            {
                vpHalSrcSurf->Rotation =  VPHAL_ROTATE_90_MIRROR_VERTICAL;
            }
            else if(mirrorState == VA_MIRROR_VERTICAL)
            {
                vpHalSrcSurf->Rotation =  VPHAL_ROTATE_90_MIRROR_HORIZONTAL;
            }
            break;
        default:
            DDI_VP_ASSERTMESSAGE("VpUpdateProcMirrorState Unexpected Invalid Rotation = %d.", vpHalSrcSurf->Rotation);
            return VA_STATUS_ERROR_INVALID_PARAMETER;

    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiSetProcPipelineBlendingParams(
    PDDI_VP_CONTEXT               vpCtx,
    uint32_t                      surfIndex,
    VAProcPipelineParameterBuffer *pipelineParam)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       src               = nullptr;
    PVPHAL_SURFACE       target            = nullptr;
    bool                 globalAlpha       = false;
    bool                 preMultAlpha      = false;
    const VABlendState   *blendState       = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx,         "nullptr vpCtx.",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(pipelineParam, "nullptr pipelineParam.", VA_STATUS_ERROR_INVALID_BUFFER);

    blendState = pipelineParam->blend_state;
    // initialize
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    src = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(src, "nullptr src.", VA_STATUS_ERROR_INVALID_SURFACE);

    if (nullptr == vpHalRenderParams->pCompAlpha)
    {
        vpHalRenderParams->pCompAlpha = MOS_New(VPHAL_ALPHA_PARAMS);
        DDI_VP_CHK_NULL(vpHalRenderParams->pCompAlpha, "nullptr pCompAlpha.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    // So far vaapi does not have alpha fill mode API.
    // And for blending support, we use VPHAL_ALPHA_FILL_MODE_NONE by default.
    target = vpHalRenderParams->pTarget[0];
    DDI_VP_CHK_NULL(target, "nullptr target.", VA_STATUS_ERROR_INVALID_SURFACE);

    //For surface with alpha, we need to bypass SFC that could change the output alpha.
    if (hasAlphaInSurface(src) && hasAlphaInSurface(target))
    {
        vpHalRenderParams->pCompAlpha->fAlpha    = 0.0f;
        vpHalRenderParams->pCompAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM;
    }
    else
    {
        vpHalRenderParams->pCompAlpha->fAlpha    = 1.0f;
        vpHalRenderParams->pCompAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_NONE;
    }

    // First, no Blending
    if (!blendState)
    {
        if (src->pBlendingParams)
        {
            src->pBlendingParams->BlendType = BLEND_NONE;
            src->pBlendingParams->fAlpha    = 1.0;
        }

        if (src->pLumaKeyParams)
        {
            src->pLumaKeyParams->LumaLow  = 0;
            src->pLumaKeyParams->LumaHigh = 0;
        }

        return VA_STATUS_SUCCESS;
    }

    // Then, process all blending types

    if (blendState->flags & VA_BLEND_GLOBAL_ALPHA)
    {
        globalAlpha = true;
    }

    if (blendState->flags & VA_BLEND_PREMULTIPLIED_ALPHA)
    {
        preMultAlpha = true;
    }

    if (nullptr == src->pBlendingParams)
    {
        src->pBlendingParams = MOS_New(VPHAL_BLENDING_PARAMS);
        DDI_VP_CHK_NULL(src->pBlendingParams, "nullptr pBlendingParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////
    // Surfaces contains alpha value, will be devide into premultiplied and non-premultiplied:
    //     For premultiplied surface, will support:
    //         a. per-pixel alpha blending;
    //         b. per-pixel plus per-plane alpha blending;
    //     For non-promultilied surface, will support:
    //         a. per-plane alpha blending;
    //         b. per-pixel alpha blending;
    //         c. NOT support per-pixel plus per-plane alpha blending;
    // Surfaces NOT contains alpha value, will not be devided into premultiplied or non-premultiplied:
    //     Will only support per-plane alpha blending.
    ///////////////////////////////////////////////////////////////////////////////////////////////////
    if (hasAlphaInSurface(src))
    {
        // For premultiplied surface, we just support four blending types until now
        SetBlendingTypes(preMultAlpha, globalAlpha, src->pBlendingParams, blendState->global_alpha);
    }
    else
    {
        if (globalAlpha)  // Do per-plane alpha blending for surface which not contain alpha value
        {
            if (blendState->global_alpha < 1.0f)
            {
                src->pBlendingParams->BlendType = BLEND_CONSTANT;
                src->pBlendingParams->fAlpha    = blendState->global_alpha;
                DDI_VP_ASSERTMESSAGE("BLEND_CONSTANT do not support alpha calculating.\n");
            }
            else
            {
                src->pBlendingParams->BlendType = BLEND_PARTIAL;
                DDI_VP_NORMALMESSAGE("Because BLEND_CONSTANT do not support alpha calculating, use BLEND_PARTIAL instead.\n");
            }

            // preMultAlpha should not be true for surfaces which do not contain alpha value,
            // but inorder to make the driver more usable, choose the most reasonable blending mode, but print out a message.
            if (preMultAlpha)
            {
                DDI_VP_NORMALMESSAGE("Should not set VA_BLEND_PREMULTIPLIED_ALPHA for surface which do not contain alpha value.\n");
            }
        }
        else
        {
            src->pBlendingParams->BlendType = BLEND_NONE;
            src->pBlendingParams->fAlpha    = 1.0;

            // preMultAlpha should not be true for surfaces which do not contain alpha value,
            // but inorder to make the driver more usable, choose the most reasonable blending mode, but print out a message.
            if (preMultAlpha)
            {
                DDI_VP_NORMALMESSAGE("Should not set VA_BLEND_PREMULTIPLIED_ALPHA for surface which do not contain alpha value.\n");
            }
        }
    }

    if (blendState->flags & VA_BLEND_LUMA_KEY)
    {
        if (nullptr == src->pLumaKeyParams)
        {
            src->pLumaKeyParams = MOS_New(VPHAL_LUMAKEY_PARAMS);
            DDI_VP_CHK_NULL(src->pLumaKeyParams, "nullptr pLumaKeyParams.", VA_STATUS_ERROR_ALLOCATION_FAILED);
        }
        src->pLumaKeyParams->LumaLow  = (int16_t)(pipelineParam->blend_state->min_luma * 255);
        src->pLumaKeyParams->LumaHigh = (int16_t)(pipelineParam->blend_state->max_luma * 255);
    }

    return VA_STATUS_SUCCESS;
}

void DdiVpFunctions::SetBlendingTypes(
    bool                   preMultAlpha,
    bool                   globalAlpha,
    PVPHAL_BLENDING_PARAMS blendingParams,
    float                  globalalpha)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(blendingParams, "nullptr blendingParams.", );
    if (preMultAlpha && !globalAlpha)  // Do per-pixel alpha blending for premultiplied surface.
        {
            blendingParams->BlendType = BLEND_PARTIAL;
        }
        else if (preMultAlpha && globalAlpha)  // Do per-pixel plus per-plane alpha blending For premultiplied surface.
        {
            if (globalalpha < 1.0f)
            {
                blendingParams->BlendType = BLEND_CONSTANT_PARTIAL;
            }
            else
            {
                // Optimization when per-plane alpha value is 1.0
                blendingParams->BlendType = BLEND_PARTIAL;
            }
            blendingParams->fAlpha = globalalpha;
        }
        else if (!preMultAlpha && globalAlpha)  // Do per-plane alpha blending for non-premultiplied surface, not do per-pixel blending
        {
            blendingParams->BlendType = BLEND_CONSTANT;
            blendingParams->fAlpha    = globalalpha;
            DDI_VP_ASSERTMESSAGE("BLEND_CONSTANT do not support alpha calculating.\n");
        }
        else if (!preMultAlpha && !globalAlpha)  // Do per-pixel alpha blending for non-premultiplied surface
        {
            blendingParams->BlendType = BLEND_SOURCE;
            DDI_VP_ASSERTMESSAGE("BLEND_SOURCE do not support alpha calculating.\n");
        }
        // Not support per-plane plus per-pixel alpha blending for non-premultiplied surface.
    return;
}

bool DdiVpFunctions::hasAlphaInSurface(PVPHAL_SURFACE surface)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(surface, "nullptr surface.No surface handle passed in.\n", false);

    switch (surface->Format)
    {
    case Format_P8:  //P8_UNORM: An 8-bit color index which is used to lookup a 32-bit ARGB value in the texture palette.
    case Format_AI44:
    case Format_IA44:
    case Format_A8R8G8B8:
    case Format_A8B8G8R8:
    case Format_A8P8:
    case Format_AYUV:
    case Format_Y410:
    case Format_Y416:
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
        return true;
    default:
        return false;
    }

    DDI_VP_ASSERTMESSAGE("No surface handle passed in.\n");
    return false;
}

VAStatus DdiVpFunctions::DdiUpdateVphalTargetSurfColorSpace(
    VADriverContextP              vaDrvCtx,
    PDDI_VP_CONTEXT               vpCtx,
    VAProcPipelineParameterBuffer *pipelineParam,
    uint32_t                      targetIndex)
{
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       vpHalSrcSurf      = nullptr;
    PVPHAL_SURFACE       vpHalTgtSurf      = nullptr;
    VAStatus             vaStatus          = VA_STATUS_SUCCESS;
    DDI_UNUSED(vaDrvCtx);

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    vpHalRenderParams = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);
    vpHalTgtSurf = vpHalRenderParams->pTarget[targetIndex];
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf.", VA_STATUS_ERROR_INVALID_SURFACE);

    // update target surface color space
#if (VA_MAJOR_VERSION < 1)
    vaStatus = DdiGetColorSpace(vpHalTgtSurf, pipelineParam->output_color_standard, pipelineParam->output_surface_flag);
#else
    vaStatus = DdiGetColorSpace(vpHalTgtSurf, pipelineParam->output_color_standard, pipelineParam->output_color_properties);
#endif

    vpHalSrcSurf = vpHalRenderParams->pSrc[0];
    // Not support BT601/BT709 -> BT2020 colorspace conversion, if colorspace is not set, will keep it same with input.
    if (vpHalSrcSurf != nullptr &&
        pipelineParam->output_color_standard == 0 &&
        IS_COLOR_SPACE_BT2020(vpHalTgtSurf->ColorSpace) &&
        !IS_COLOR_SPACE_BT2020(vpHalSrcSurf->ColorSpace))
    {
        vpHalTgtSurf->ColorSpace = vpHalSrcSurf->ColorSpace;
    }

    // extended gamut?
    vpHalRenderParams->pTarget[0]->ExtendedGamut = false;

    return vaStatus;
}

VAStatus DdiVpFunctions::DdiBeginPictureInt(
    VADriverContextP vaDrvCtx,
    PDDI_VP_CONTEXT  vpCtx,
    VASurfaceID      vaSurfID)
{
    PDDI_MEDIA_CONTEXT   mediaDrvCtx       = nullptr;
    uint32_t             ctxType           = 0;
    VAStatus             vaStatus          = VA_STATUS_SUCCESS;
    PVPHAL_RENDER_PARAMS vpHalRenderParams = nullptr;
    PVPHAL_SURFACE       vpHalTgtSurf      = nullptr;
    PDDI_MEDIA_SURFACE   mediaTgtSurf      = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vaDrvCtx, "nullptr vaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    // initialize
    mediaDrvCtx = GetMediaContext(vaDrvCtx);
    DDI_VP_CHK_NULL(mediaDrvCtx, "nullptr mediaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx,       "nullptr vpCtx.",       VA_STATUS_ERROR_INVALID_CONTEXT);
    vpCtx->TargetSurfID = vaSurfID;

    vpHalRenderParams   = VpGetRenderParams(vpCtx);
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    // uDstCount == 0 means no render target is set yet.
    // uDstCount == 1 means 1 render target has been set already.
    // uDstCount == 2 means 2 render targets have been set already.
    DDI_VP_CHK_LESS(vpHalRenderParams->uDstCount, VPHAL_MAX_TARGETS, "Too many render targets for VP.", VA_STATUS_ERROR_INVALID_PARAMETER);

    vpHalTgtSurf = vpHalRenderParams->pTarget[vpHalRenderParams->uDstCount];
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf.",  VA_STATUS_ERROR_INVALID_SURFACE);
    mediaTgtSurf = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaDrvCtx, vaSurfID);
    DDI_VP_CHK_NULL(mediaTgtSurf, "nullptr mediaTgtSurf.", VA_STATUS_ERROR_INVALID_SURFACE);

    mediaTgtSurf->pVpCtx = vpCtx;

    // Setup Target VpHal Surface
    vpHalTgtSurf->SurfType       = SURF_OUT_RENDERTARGET;
    vpHalTgtSurf->rcSrc.top      = 0;
    vpHalTgtSurf->rcSrc.left     = 0;
    vpHalTgtSurf->rcSrc.right    = mediaTgtSurf->iWidth;
    vpHalTgtSurf->rcSrc.bottom   = mediaTgtSurf->iRealHeight;
    vpHalTgtSurf->rcDst.top      = 0;
    vpHalTgtSurf->rcDst.left     = 0;
    vpHalTgtSurf->rcDst.right    = mediaTgtSurf->iWidth;
    vpHalTgtSurf->rcDst.bottom   = mediaTgtSurf->iRealHeight;
    vpHalTgtSurf->ExtendedGamut  = false;

    // Set os resource for VPHal render
    vaStatus = VpSetOsResource(vpCtx, mediaTgtSurf, vpHalRenderParams->uDstCount);
    DDI_CHK_RET(vaStatus, "Call VpSetOsResource failed");

    vpHalTgtSurf->Format = vpHalTgtSurf->OsResource.Format;

    vpHalRenderParams->bReportStatus    = true;
    vpHalRenderParams->StatusFeedBackID = vaSurfID;
    if (mediaTgtSurf->pSurfDesc && (mediaTgtSurf->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR))
    {
        vpHalRenderParams->pTarget[vpHalRenderParams->uDstCount]->b16UsrPtr = VpIs16UsrPtrPitch(mediaTgtSurf->iPitch, mediaTgtSurf->format);
    }
    else
    {
        vpHalRenderParams->pTarget[vpHalRenderParams->uDstCount]->b16UsrPtr = false;
    }
    // increase render target count
    vpHalRenderParams->uDstCount++;

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::DdiSetProcPipelineParams(
    VADriverContextP              vaDrvCtx,
    PDDI_VP_CONTEXT               vpCtx,
    VAProcPipelineParameterBuffer *pipelineParam)
{
    PVPHAL_RENDER_PARAMS             vpHalRenderParams  = nullptr;
    PDDI_MEDIA_CONTEXT               mediaCtx           = nullptr;
    PDDI_MEDIA_SURFACE               mediaSrcSurf       = nullptr;
    PDDI_MEDIA_BUFFER                filterBuf          = nullptr;
    void *                           data               = nullptr;
    uint32_t                         i                  = 0;
    PVPHAL_SURFACE                   vpHalSrcSurf       = nullptr;
    PVPHAL_SURFACE                   vpHalTgtSurf       = nullptr;
    uint32_t                         surfIndex          = 0;
    uint32_t                         scalingflags       = 0;
    uint32_t                         interpolationflags = 0;
    VAStatus                         vaStatus           = VA_STATUS_SUCCESS;
    MOS_STATUS                       eStatus            = MOS_STATUS_SUCCESS;
    DDI_VP_STATE                     vpStateFlags       = {};
    PMOS_INTERFACE                   osInterface        = nullptr;
    VAProcFilterParameterBufferBase  *filterParam       = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vaDrvCtx, "nullptr vaDrvCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(vpCtx->pVpHal, "nullptr vpCtx->pVpHal.", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(pipelineParam, "nullptr pipelineParam.", VA_STATUS_ERROR_INVALID_PARAMETER);

    // initialize
    mediaCtx = GetMediaContext(vaDrvCtx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx.", VA_STATUS_ERROR_INVALID_CONTEXT);

    mediaSrcSurf = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, pipelineParam->surface);
    DDI_VP_CHK_NULL(mediaSrcSurf, "nullptr mediaSrcSurf.", VA_STATUS_ERROR_INVALID_BUFFER);

    vpHalRenderParams = vpCtx->pVpHalRenderParams;
    DDI_VP_CHK_NULL(vpHalRenderParams, "nullptr vpHalRenderParams.", VA_STATUS_ERROR_INVALID_PARAMETER);

    osInterface = vpCtx->pVpHal->GetOsInterface();
    DDI_VP_CHK_NULL(osInterface, "nullptr osInterface.", VA_STATUS_ERROR_INVALID_BUFFER);

    // increment surface count
    vpHalRenderParams->uSrcCount++;

    // check if the surface count exceeded maximum VPHAL surfaces
    DDI_VP_CHK_CONDITION((vpHalRenderParams->uSrcCount > VPHAL_MAX_SOURCES),
        "Surface count exceeds maximum!",
        VA_STATUS_ERROR_MAX_NUM_EXCEEDED);

    // update surface surfIndex
    surfIndex = vpHalRenderParams->uSrcCount - 1;

    vpHalSrcSurf = vpHalRenderParams->pSrc[surfIndex];
    DDI_VP_CHK_NULL(vpHalSrcSurf, "nullptr vpHalSrcSurf.", VA_STATUS_ERROR_INVALID_PARAMETER);

    vpHalSrcSurf->Format   = MediaLibvaUtilNext::GetFormatFromMediaFormat(mediaSrcSurf->format);
    vpHalSrcSurf->TileType = MediaLibvaUtilNext::GetTileTypeFromMediaTileType(mediaSrcSurf->TileType);

    DDI_VP_CHK_CONDITION((Format_Invalid == vpHalSrcSurf->Format), "Invalid surface media format!", VA_STATUS_ERROR_INVALID_PARAMETER);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (vpCtx->pCurVpDumpDDIParam != nullptr && vpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[surfIndex] != nullptr)
    {
        MOS_SecureMemcpy(vpCtx->pCurVpDumpDDIParam->pPipelineParamBuffers[surfIndex], sizeof(VAProcPipelineParameterBuffer), pipelineParam, sizeof(VAProcPipelineParameterBuffer));
        vpCtx->pCurVpDumpDDIParam->SrcFormat[surfIndex] = vpHalSrcSurf->Format;
    }
#endif  //(_DEBUG || _RELEASE_INTERNAL)

    // Currently we only support 1 primary surface in VP
    if (vpCtx->iPriSurfs < VP_MAX_PRIMARY_SURFS)
    {
        vpHalSrcSurf->SurfType = SURF_IN_PRIMARY;
        vpCtx->iPriSurfs++;
    }
    else
    {
        vpHalSrcSurf->SurfType = SURF_IN_SUBSTREAM;
    }

    // Set workload path using pipeline_flags VA_PROC_PIPELINE_FAST flag
    if (pipelineParam->pipeline_flags & VA_PROC_PIPELINE_FAST)
    {
        vpHalRenderParams->bForceToRender = true;
    }

    // Set src rect
    SetSrcSurfaceRect(pipelineParam->surface_region, vpHalSrcSurf, mediaSrcSurf);

    vpHalTgtSurf = vpHalRenderParams->pTarget[0];
    DDI_VP_CHK_NULL(vpHalTgtSurf, "nullptr vpHalTgtSurf.", VA_STATUS_ERROR_UNKNOWN);
    // Set dest rect
    vaStatus = SetDestSurfaceRect(pipelineParam->output_region, vpHalSrcSurf, vpHalTgtSurf);
    DDI_CHK_RET(vaStatus, "Failed to Set dest rect!");

    //set the frame_id
    vpHalSrcSurf->FrameID = mediaSrcSurf->frame_idx;

    //In order to refresh the frame ID in DdiVp_UpdateFilterParamBuffer when run ADI case,
    //we need to set the OS resource here.
    vpHalSrcSurf->OsResource.bo = mediaSrcSurf->bo;

    // csc option
    //---------------------------------------
    // Set color space for src
#if (VA_MAJOR_VERSION < 1)
    vaStatus = DdiGetColorSpace(vpHalSrcSurf, pipelineParam->surface_color_standard, pipelineParam->input_surface_flag);
#else
    vaStatus = DdiGetColorSpace(vpHalSrcSurf, pipelineParam->surface_color_standard, pipelineParam->input_color_properties);
#endif
    DDI_CHK_RET(vaStatus, "Unsupport Color space!");

    if (mediaSrcSurf->format == Media_Format_400P)
    {
        vpHalSrcSurf->ColorSpace = CSpace_BT601Gray;
    }

    // extended gamut? RGB can't have extended gamut flag
    vpHalSrcSurf->ExtendedGamut = false;

    // Background Colorfill
    // According to libva  definition, if alpha in output background color is zero, then colorfill is not needed
    DDI_CHK_RET(SetBackgroundColorfill(vpHalRenderParams, pipelineParam->output_background_color), "Failed Set Background Colorfill");

    // Set Demo Mode option
    if (vpHalRenderParams->bDisableDemoMode == false)
    {
        eStatus = VpHalDdiSetupSplitScreenDemoMode(
            0,  // No DDI setting on Linux. Set it when Linux DDI supports it
            0,  // No DDI setting on Linux. Set it when Linux DDI supports it
            &vpHalRenderParams->pSplitScreenDemoModeParams,
            &vpHalRenderParams->bDisableDemoMode,
            osInterface);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            DDI_VP_ASSERTMESSAGE("Failed to setup Split-Screen Demo Mode.");
            MOS_FreeMemAndSetNull(vpHalRenderParams->pSplitScreenDemoModeParams);
        }
    }

    // Update fwd and bkward ref frames: Required for Advanced processing - will be supported in the future
    vpHalSrcSurf->uFwdRefCount = pipelineParam->num_backward_references;

    vaStatus = DdiUpdateProcPipelineFutureReferenceFrames(vpCtx, vaDrvCtx, vpHalSrcSurf, pipelineParam);
    DDI_CHK_RET(vaStatus, "Failed to update future reference frames!");

    vpHalSrcSurf->uBwdRefCount = pipelineParam->num_forward_references;

    vaStatus = DdiUpdateProcPipelinePastReferenceFrames(vpCtx, vaDrvCtx, vpHalSrcSurf, pipelineParam);
    DDI_CHK_RET(vaStatus, "Failed to update past reference frames!");

    // Check if filter values changed,if yes, then reset all filters for this surface

    // intialize the filter parameter
    // Parameters once set in the surface will be keep till video complete
    // so the parameter of rendered surface should be intialized every time
    if (vpHalRenderParams->bStereoMode)
    {
        vpHalSrcSurf->Rotation = VPHAL_ROTATION_IDENTITY;
    }

    // Progressive or interlaced - Check filter_flags
    // if the deinterlace parameters is not set, manipulate it as a progressive video.
    if (pipelineParam->filter_flags & VA_TOP_FIELD)
    {
        vpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
    }
    else if (pipelineParam->filter_flags & VA_BOTTOM_FIELD)
    {
        vpHalSrcSurf->SampleType = SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD;
    }
    else  // VA_FRAME_PICTURE
    {
        vpHalSrcSurf->SampleType = SAMPLE_PROGRESSIVE;

        //if the deinterlace parameters is set, clear it.
        if (vpHalSrcSurf->pDeinterlaceParams != nullptr)
        {
            MOS_Delete(vpHalSrcSurf->pDeinterlaceParams);
        }
    }

    for (i = 0; i < pipelineParam->num_filters; i++)
    {
        VABufferID filter = pipelineParam->filters[i];

        filterBuf = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, filter);
        DDI_VP_CHK_NULL(filterBuf, "nullptr filterBuf!", VA_STATUS_ERROR_INVALID_BUFFER);

        DDI_VP_CHK_CONDITION((VAProcFilterParameterBufferType != filterBuf->uiType),
            "Invalid parameter buffer type!",
            VA_STATUS_ERROR_INVALID_PARAMETER);

        // Map Buffer data to virtual addres space
        MediaLibvaInterfaceNext::MapBuffer(vaDrvCtx, filter, &data);

        filterParam = (VAProcFilterParameterBufferBase *)data;

        // HSBC can only be applied to the primary layer
        if (!((filterParam->type == VAProcFilterColorBalance) && vpHalSrcSurf->SurfType != SURF_IN_PRIMARY))
        {
            // Pass the filter type
            vaStatus = DdiUpdateFilterParamBuffer(vaDrvCtx, vpCtx, surfIndex, filterParam->type, data, filterBuf->uiNumElements, &vpStateFlags);
            DDI_CHK_RET(vaStatus, "Failed to update parameter buffer!");
        }
    }

    DdiClearFilterParamBuffer(vpCtx, surfIndex, vpStateFlags);

    // Use Render to do scaling for resolution larger than 8K
    if (MEDIA_IS_WA(&mediaCtx->WaTable, WaDisableVeboxFor8K))
    {
        vpHalRenderParams->bDisableVeboxFor8K = true;
    }

    // Scaling algorithm
    scalingflags                    = pipelineParam->filter_flags & VA_FILTER_SCALING_MASK;
    vpHalSrcSurf->ScalingPreference = VPHAL_SCALING_PREFER_SFC;  // default
    // Interpolation method
#if VA_CHECK_VERSION(1, 9, 0)
    interpolationflags = pipelineParam->filter_flags & VA_FILTER_INTERPOLATION_MASK;
#endif
    switch (scalingflags)
    {
    case VA_FILTER_SCALING_FAST:
        if (vpHalSrcSurf->SurfType == SURF_IN_PRIMARY)
        {
            VpSetInterpolationParams(vpHalSrcSurf, interpolationflags);
            vpHalSrcSurf->ScalingPreference = VPHAL_SCALING_PREFER_SFC_FOR_VEBOX;
        }
        else
        {
            vpHalSrcSurf->ScalingMode = VPHAL_SCALING_BILINEAR;
        }
        break;
    case VA_FILTER_SCALING_HQ:
        if (vpHalSrcSurf->SurfType == SURF_IN_PRIMARY)
        {
            VpSetInterpolationParams(vpHalSrcSurf, interpolationflags);
            vpHalSrcSurf->ScalingPreference = VPHAL_SCALING_PREFER_COMP;
        }
        else
        {
            vpHalSrcSurf->ScalingMode = VPHAL_SCALING_BILINEAR;
        }
        break;
    case VA_FILTER_SCALING_DEFAULT:
    case VA_FILTER_SCALING_NL_ANAMORPHIC:
    default:
        if (vpHalSrcSurf->SurfType == SURF_IN_PRIMARY)
        {
            VpSetInterpolationParams(vpHalSrcSurf, interpolationflags);
            vpHalSrcSurf->ScalingPreference = VPHAL_SCALING_PREFER_SFC;
        }
        else
        {
            vpHalSrcSurf->ScalingMode = VPHAL_SCALING_BILINEAR;
        }
        break;
    }
    //init interlace scaling flag
    vpHalSrcSurf->bInterlacedScaling = false;
    vpHalSrcSurf->bFieldWeaving      = false;

    if (vpHalSrcSurf->pDeinterlaceParams == nullptr)
    {
        if (pipelineParam->input_surface_flag & VA_TOP_FIELD_FIRST)
        {
            vaStatus = SetSurfaceParamsTopFieldFirst(pipelineParam->output_surface_flag, vpHalSrcSurf, vpHalTgtSurf);
            DDI_CHK_RET(vaStatus, "Failed to set params on TopFieldFirst");
        }
        else if (pipelineParam->input_surface_flag & VA_BOTTOM_FIELD_FIRST)
        {
            vaStatus = SetSurfaceParamsBottomFieldFirst(pipelineParam->output_surface_flag, vpHalSrcSurf, vpHalTgtSurf);
            DDI_CHK_RET(vaStatus, "Failed to set params on BottomFieldFirst");
        }
        else if (pipelineParam->input_surface_flag & VA_TOP_FIELD)
        {
            vaStatus = SetSurfaceParamsTopField(pipelineParam->output_surface_flag, vpHalSrcSurf, vpHalTgtSurf);
            DDI_CHK_RET(vaStatus, "Failed to set params on TopField");
        }
        else if (pipelineParam->input_surface_flag & VA_BOTTOM_FIELD)
        {
            vaStatus = SetSurfaceParamsBottomField(pipelineParam->output_surface_flag, vpHalSrcSurf, vpHalTgtSurf);
            DDI_CHK_RET(vaStatus, "Failed to set params on BottomField");
        }
        else
        {
            vpHalSrcSurf->SampleType            = SAMPLE_PROGRESSIVE;
            vpHalSrcSurf->InterlacedScalingType = ISCALING_NONE;
            vpHalSrcSurf->bInterlacedScaling    = false;
            vpHalSrcSurf->bFieldWeaving         = false;
        }
    }

    // For legacy interlace scaling
    SetLegacyInterlaceScalingParams(vpHalSrcSurf, pipelineParam->filter_flags);

    // For weave DI
    VpUpdateWeaveDI(vpHalSrcSurf, pipelineParam->filter_flags);

    // Rotation
    vaStatus = VpUpdateProcRotateState(vpHalSrcSurf, pipelineParam->rotation_state);
    DDI_CHK_RET(vaStatus, "Failed to update rotate state!");

    // Mirror
    vaStatus = VpUpdateProcMirrorState(vpHalSrcSurf, pipelineParam->mirror_state);
    DDI_CHK_RET(vaStatus, "Failed to update mirror state!");

    // Alpha blending
    // Note: the alpha blending region cannot overlay
    vaStatus = DdiSetProcPipelineBlendingParams(vpCtx, surfIndex, pipelineParam);
    DDI_CHK_RET(vaStatus, "Failed to update Alpha Blending parameter!");

    if (IsProcmpEnable(vpHalSrcSurf))
    {
        // correct the ChromaSitting location if Procamp is enabled.
#if (VA_MAJOR_VERSION < 1)
        pipelineParam->input_surface_flag   = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
        pipelineParam->output_surface_flag = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
#else
        pipelineParam->input_color_properties.chroma_sample_location = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
        pipelineParam->output_color_properties.chroma_sample_location = VA_CHROMA_SITING_HORIZONTAL_LEFT | VA_CHROMA_SITING_VERTICAL_TOP;
#endif
    }

#if (VA_MAJOR_VERSION < 1)
    VpUpdateProcChromaSittingState(vpHalSrcSurf, (uint8_t)(pipelineParam->input_surface_flag & 0xff));
    VpUpdateProcChromaSittingState(vpHalTgtSurf, (uint8_t)(pipelineParam->output_surface_flag & 0xff));
#else
    VpUpdateProcChromaSittingState(vpHalSrcSurf, pipelineParam->input_color_properties.chroma_sample_location);
    VpUpdateProcChromaSittingState(vpHalTgtSurf, pipelineParam->output_color_properties.chroma_sample_location);
#endif
    // update OsResource for src surface
    vpHalSrcSurf->OsResource.Format      = MediaLibvaUtilNext::GetFormatFromMediaFormat(mediaSrcSurf->format);
    vpHalSrcSurf->OsResource.iWidth      = mediaSrcSurf->iWidth;
    vpHalSrcSurf->OsResource.iHeight     = mediaSrcSurf->iHeight;
    vpHalSrcSurf->OsResource.iPitch      = mediaSrcSurf->iPitch;
    vpHalSrcSurf->OsResource.iCount      = mediaSrcSurf->iRefCount;
    vpHalSrcSurf->OsResource.bo          = mediaSrcSurf->bo;
    vpHalSrcSurf->OsResource.TileType    = MediaLibvaUtilNext::GetTileTypeFromMediaTileType(mediaSrcSurf->TileType);
    vpHalSrcSurf->OsResource.pGmmResInfo = mediaSrcSurf->pGmmResourceInfo;

    Mos_Solo_SetOsResource(mediaSrcSurf->pGmmResourceInfo, &vpHalSrcSurf->OsResource);

    //Set encryption bit for input surface
    //This setting only used for secure VPP test app which do secure VP and provide secure YUV as input
    //Since APP cannot set encryption flag, it will set input_surface_flag to ask driver add encryption bit to input surface
    if (osInterface->osCpInterface->IsHMEnabled() && (pipelineParam->input_surface_flag & VPHAL_SURFACE_ENCRYPTION_FLAG))
    {
        osInterface->osCpInterface->SetResourceEncryption(&(vpHalSrcSurf->OsResource), true);
    }

    // Update the Render Target params - this needs to be done once when Render Target is passed via BeginPicture
    vaStatus = DdiUpdateVphalTargetSurfColorSpace(vaDrvCtx, vpCtx, pipelineParam, 0);
    DDI_CHK_RET(vaStatus, "Failed to update vphal target surface color space!");
    // Update the Render Target HDR params - this needs to be done once when Render Target is passed via BeginPicture
    vaStatus = VpUpdateProcHdrState(vpHalTgtSurf, pipelineParam->output_hdr_metadata);
    DDI_CHK_RET(vaStatus, "Failed to update vphal target surface HDR metadata!");

    // Using additional_outputs processing as 1:N case.
    for (i = 0; i < pipelineParam->num_additional_outputs; i++)
    {
        vaStatus = DdiBeginPictureInt(vaDrvCtx, vpCtx, pipelineParam->additional_outputs[i]);
        DDI_CHK_RET(vaStatus, "Failed to update vphal target surface buffers!");
        vaStatus = DdiUpdateVphalTargetSurfColorSpace(vaDrvCtx, vpCtx, pipelineParam, i + 1);
        DDI_CHK_RET(vaStatus, "Failed to update vphal target surface color space!");
    }

    // add 16aligned UsrPtr mode support
    if (mediaSrcSurf->pSurfDesc && (mediaSrcSurf->pSurfDesc->uiVaMemType == VA_SURFACE_ATTRIB_MEM_TYPE_USER_PTR))
    {
        vpHalSrcSurf->b16UsrPtr = VpIs16UsrPtrPitch(mediaSrcSurf->iPitch, mediaSrcSurf->format); // VpIs16UsrPtrPitch  define in beginPicture
    }
    else
    {
        vpHalSrcSurf->b16UsrPtr = false;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::StatusCheck(
    PDDI_MEDIA_CONTEXT mediaCtx,
    DDI_MEDIA_SURFACE  *surface,
    VASurfaceID        surfaceId)
{
    PDDI_VP_CONTEXT         vpCtx        = nullptr;
    QUERY_STATUS_REPORT_APP tempVpReport = {};
    DDI_MEDIA_SURFACE       *tempSurface = nullptr;
    MOS_STATUS              eStatus      = MOS_STATUS_SUCCESS;
    DDI_VP_FUNC_ENTER;

    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(surface,  "nullptr surface",  VA_STATUS_ERROR_INVALID_CONTEXT);

    vpCtx = (PDDI_VP_CONTEXT)surface->pVpCtx;
    DDI_VP_CHK_NULL(vpCtx ,        "nullptr vpCtx",         VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(vpCtx->pVpHal ,"nullptr vpCtx->pVpHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    MOS_ZeroMemory(&tempVpReport, sizeof(QUERY_STATUS_REPORT_APP));

    // Get reported surfaces' count
    uint32_t tableLen = 0;
    eStatus = vpCtx->pVpHal->GetStatusReportEntryLength(&tableLen);
    if(eStatus != MOS_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }
    if (tableLen > 0 && surface->curStatusReportQueryState == DDI_MEDIA_STATUS_REPORT_QUERY_STATE_PENDING)
    {
        // Query the status for all of surfaces which have finished
        for(uint32_t i = 0; i < tableLen; i++)
        {
            MOS_ZeroMemory(&tempVpReport, sizeof(QUERY_STATUS_REPORT_APP));
            eStatus = vpCtx->pVpHal->GetStatusReport(&tempVpReport, 1);
            if(eStatus != MOS_STATUS_SUCCESS)
            {
                return VA_STATUS_ERROR_OPERATION_FAILED;
            }
            // StatusFeedBackID is last time submitted Target Surface ID which is set in BeginPicture,
            // So we can know the report is for which surface here.
            tempSurface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, tempVpReport.StatusFeedBackID);
            DDI_VP_CHK_NULL(tempSurface,  "nullptr tempSurface",  VA_STATUS_ERROR_OPERATION_FAILED);

            // Update the status of the surface which is reported.
            tempSurface->curStatusReport.vpp.status = (uint32_t)tempVpReport.dwStatus;
            tempSurface->curStatusReportQueryState  = DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED;

            if(tempVpReport.StatusFeedBackID == surfaceId)
            {
                break;
            }
        }
    }

    if (surface->curStatusReportQueryState != DDI_MEDIA_STATUS_REPORT_QUERY_STATE_COMPLETED)
    {
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

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
    return VA_STATUS_SUCCESS;
}

VAStatus DdiVpFunctions::ProcessPipeline(
    VADriverContextP    vaDrvCtx,
    VAContextID         ctxID,
    VASurfaceID         srcSurface,
    VARectangle         *srcRect,
    VASurfaceID         dstSurface,
    VARectangle         *dstRect)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_VP, PERF_LEVEL_DDI);

    VAStatus                      vaStatus            = VA_STATUS_SUCCESS;
    uint32_t                      ctxType             = DDI_MEDIA_CONTEXT_TYPE_NONE;
    PDDI_VP_CONTEXT               vpCtx               = nullptr;
    VAProcPipelineParameterBuffer *inputPipelineParam = nullptr;

    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(vaDrvCtx, "nullptr vaDrvCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    vpCtx = (PDDI_VP_CONTEXT)MediaLibvaCommonNext::GetContextFromContextID(vaDrvCtx, ctxID, &ctxType);
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    vaStatus = BeginPicture(vaDrvCtx, ctxID, dstSurface);
    DDI_CHK_RET(vaStatus, "VP BeginPicture failed");

    //Set parameters
    inputPipelineParam = (VAProcPipelineParameterBuffer*)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineParameterBuffer));
    DDI_VP_CHK_NULL(inputPipelineParam, "nullptr inputPipelineParam", VA_STATUS_ERROR_ALLOCATION_FAILED);

    inputPipelineParam->surface_region = srcRect;
    inputPipelineParam->output_region  = dstRect;
    inputPipelineParam->surface        = srcSurface;

    vaStatus = DdiSetProcPipelineParams(vaDrvCtx, vpCtx, inputPipelineParam);
    if(vaStatus != VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(inputPipelineParam);
        DDI_VP_ASSERTMESSAGE("VP SetProcPipelineParams failed.");
        return vaStatus;
    }

    vaStatus = EndPicture(vaDrvCtx, ctxID);
    if(vaStatus != VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(inputPipelineParam);
        DDI_VP_ASSERTMESSAGE("VP EndPicture failed.");
        return vaStatus;
    }

    MOS_FreeMemory(inputPipelineParam);
    return vaStatus;
}

VAStatus DdiVpFunctions::PutSurface(
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
    VARectangle      *cliprects,
    uint32_t         numberCliprects,
    uint32_t         flags)
{
    void               *vpCtx      = nullptr;
    PDDI_MEDIA_CONTEXT mediaDrvCtx = nullptr;
    uint32_t           ctxType     = DDI_MEDIA_CONTEXT_TYPE_NONE;
    DDI_VP_FUNC_ENTER;

    mediaDrvCtx = GetMediaContext(ctx);
    DDI_VP_CHK_NULL(mediaDrvCtx,               "nullptr mediaDrvCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(mediaDrvCtx->pSurfaceHeap, "nullptr mediaDrvCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(mediaDrvCtx->pVpCtxHeap,   "nullptr mediaDrvCtx->pVpCtxHeap",   VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_LESS((uint32_t)surface, mediaDrvCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surface", VA_STATUS_ERROR_INVALID_SURFACE);

    if (nullptr != mediaDrvCtx->pVpCtxHeap->pHeapBase)
    {
        vpCtx = MediaLibvaCommonNext::GetContextFromContextID(ctx, (VAContextID)(DDI_MEDIA_SOFTLET_VACONTEXTID_VP_OFFSET), &ctxType);
    }

#if defined(ANDROID) || !defined(X11_FOUND)
    return VA_STATUS_ERROR_UNIMPLEMENTED;
#else
    if(nullptr == vpCtx)
    {
        VAContextID context  = VA_INVALID_ID;
        VAStatus    vaStatus = CreateContext(ctx, 0, 0, 0, 0, 0, 0, &context);
        DDI_CHK_RET(vaStatus, "Create VP Context failed");
    }
    return PutSurfaceLinuxHW(ctx, surface, draw, srcx, srcy, srcw, srch, destx, desty, destw, desth, cliprects, numberCliprects, flags);
#endif

    return VA_STATUS_SUCCESS;
}

#if !defined(ANDROID) && defined(X11_FOUND)

void DdiVpFunctions::RectInit(
    RECT            *rect,
    int16_t         destx,
    int16_t         desty,
    uint16_t        destw,
    uint16_t        desth)
{
    DDI_VP_FUNC_ENTER;
    DDI_VP_CHK_NULL(rect, "nullptr rect", );

    rect->left   = destx;
    rect->top    = desty;
    rect->right  = destx + destw;
    rect->bottom = desty + desth;
    return;
}

VAStatus DdiVpFunctions::PutSurfaceLinuxHW(
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
    VARectangle      *cliprects,
    uint32_t         numberCliprects,
    uint32_t         flags)
{
    VpBase                 *vpHal        = nullptr;
    int32_t                ovRenderIndex = 0;
    VPHAL_SURFACE          surf          = {};
    VPHAL_SURFACE          target        = {};
    VPHAL_RENDER_PARAMS    renderParams  = {};
    VPHAL_COLORFILL_PARAMS colorFill     = {};
    MOS_STATUS             eStatus       = MOS_STATUS_INVALID_PARAMETER;
    RECT                   srcRect       = { 0, 0, 0, 0 };
    RECT                   dstRect       = { 0, 0, 0, 0 };
    PDDI_MEDIA_CONTEXT     mediaCtx      = nullptr;
    PDDI_MEDIA_SURFACE     bufferObject  = nullptr;
    uint32_t               width = 0,height = 0,pitch = 0;
    uint32_t               drawableTilingMode  = 0;
    uint32_t               drawableSwizzleMode = 0;
    MOS_TILE_TYPE          tileType            = MOS_TILE_INVALID;
    uint32_t               ctxType             = 0;
    PDDI_VP_CONTEXT        vpCtx               = nullptr;
    struct dri_drawable    *driDrawable        = nullptr;
    union dri_buffer       *buffer             = nullptr;
    GMM_RESCREATE_PARAMS   gmmParams           = {};

    DDI_VP_FUNC_ENTER;

    mediaCtx = GetMediaContext(ctx);
    DDI_VP_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_VP_CHK_NULL(mediaCtx->dri_output, "nullptr mediaDrvCtx->dri_output", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(mediaCtx->pSurfaceHeap, "nullptr mediaDrvCtx->pSurfaceHeap", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(mediaCtx->pGmmClientContext, "nullptr mediaCtx->pGmmClientContext", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(mediaCtx->pVpCtxHeap, "nullptr mediaCtx->pVpCtxHeap", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_VP_CHK_NULL(mediaCtx->pVpCtxHeap->pHeapBase, "nullptr mediaCtx->pVpCtxHeap->pHeapBase", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CHK_LESS((uint32_t)surface, mediaCtx->pSurfaceHeap->uiAllocatedHeapElements, "Invalid surfaceId", VA_STATUS_ERROR_INVALID_SURFACE);

    struct dri_vtable *const driVtable = &mediaCtx->dri_output->vtable;
    DDI_VP_CHK_NULL(driVtable, "nullptr driVtable", VA_STATUS_ERROR_INVALID_PARAMETER);

    driDrawable = driVtable->get_drawable(ctx, (Drawable)draw);
    DDI_VP_CHK_NULL(driDrawable, "nullptr driDrawable", VA_STATUS_ERROR_INVALID_PARAMETER);
    buffer = driVtable->get_rendering_buffer(ctx, driDrawable);
    DDI_VP_CHK_NULL(buffer, "nullptr buffer", VA_STATUS_ERROR_INVALID_PARAMETER);

    bufferObject = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, surface);
    DDI_VP_CHK_NULL(bufferObject, "nullptr bufferObject", VA_STATUS_ERROR_INVALID_SURFACE);
    MediaLibvaUtilNext::MediaPrintFps();
    pitch = bufferObject->iPitch;

    vpCtx = (PDDI_VP_CONTEXT)MediaLibvaCommonNext::GetContextFromContextID(ctx, (VAContextID)(DDI_MEDIA_SOFTLET_VACONTEXTID_VP_OFFSET), &ctxType);
    DDI_VP_CHK_NULL(vpCtx, "nullptr vpCtx", VA_STATUS_ERROR_INVALID_PARAMETER);
    vpHal = vpCtx->pVpHal;
    DDI_VP_CHK_NULL(vpHal, "nullptr vpHal", VA_STATUS_ERROR_INVALID_PARAMETER);

    renderParams.Component = COMPONENT_LibVA;

    //Init source rectangle
    RectInit(&srcRect, srcx, srcy, srcw, srch);
    RectInit(&dstRect, destx, desty, destw, desth);

    if (destx + destw > driDrawable->x + driDrawable->width)
    {
        dstRect.right = driDrawable->x + driDrawable->width - destx;
        if(dstRect.right <= 0)
        {
            return VA_STATUS_SUCCESS;
        }
    }
    if (desty + desth > driDrawable->y + driDrawable->height)
    {
        dstRect.bottom = driDrawable->y + driDrawable->height - desty;
        if(dstRect.bottom <= 0)
        {
            return VA_STATUS_SUCCESS;
        }
    }
    // Source Surface Information
    surf.Format                 = MediaLibvaUtilNext::GetFormatFromMediaFormat(bufferObject->format);  // Surface format
    surf.SurfType               = SURF_IN_PRIMARY;                                   // Surface type (context)
    surf.SampleType             = SAMPLE_PROGRESSIVE;
    surf.ScalingMode            = VPHAL_SCALING_AVS;

    surf.OsResource.Format      = MediaLibvaUtilNext::GetFormatFromMediaFormat(bufferObject->format);
    surf.OsResource.iWidth      = bufferObject->iWidth;
    surf.OsResource.iHeight     = bufferObject->iHeight;
    surf.OsResource.iPitch      = bufferObject->iPitch;
    surf.OsResource.iCount      = 0;
    surf.OsResource.TileType    = MediaLibvaUtilNext::GetTileTypeFromMediaTileType(bufferObject->TileType);
    surf.OsResource.bMapped     = bufferObject->bMapped;
    surf.OsResource.bo          = bufferObject->bo;
    surf.OsResource.pGmmResInfo = bufferObject->pGmmResourceInfo;

    surf.dwWidth                = bufferObject->iWidth;
    surf.dwHeight               = bufferObject->iHeight;
    surf.dwPitch                = bufferObject->iPitch;
    surf.TileType               = MediaLibvaUtilNext::GetTileTypeFromMediaTileType(bufferObject->TileType);
    surf.ColorSpace             = MediaLibvaUtilNext::GetColorSpaceFromMediaFormat(bufferObject->format);
    surf.ExtendedGamut          = false;
    surf.rcSrc                  = srcRect;
    surf.rcDst                  = dstRect;

    MOS_LINUX_BO *drawableBo = mos_bo_create_from_name(mediaCtx->pDrmBufMgr, "rendering buffer", buffer->dri2.name);
    DDI_VP_CHK_NULL(drawableBo, "nullptr drawableBo", VA_STATUS_ERROR_ALLOCATION_FAILED);

    if (!mos_bo_get_tiling(drawableBo, &drawableTilingMode, &drawableSwizzleMode))
    {
        switch (drawableTilingMode)
        {
            case TILING_Y:
                tileType = MOS_TILE_Y;
                break;
            case TILING_X:
                tileType = MOS_TILE_X;
                gmmParams.Flags.Info.TiledX    = true;
                break;
            case TILING_NONE:
               tileType = MOS_TILE_LINEAR;
               gmmParams.Flags.Info.Linear    = true;
               break;
            default:
                drawableTilingMode          = TILING_NONE;
                tileType = MOS_TILE_LINEAR;
                gmmParams.Flags.Info.Linear    = true;
                break;
        }
        target.OsResource.TileType = (MOS_TILE_TYPE)drawableTilingMode;
    }
    else
    {
        target.OsResource.TileType = (MOS_TILE_TYPE)TILING_NONE;
        tileType = MOS_TILE_LINEAR;
        gmmParams.Flags.Info.Linear    = true;
    }
    gmmParams.Flags.Info.LocalOnly = MEDIA_IS_SKU(&mediaCtx->SkuTable, FtrLocalMemory);

    target.Format                = Format_A8R8G8B8;
    target.SurfType              = SURF_OUT_RENDERTARGET;

    //init target retangle
    RectInit(&srcRect, driDrawable->x, driDrawable->y, driDrawable->width, driDrawable->height);
    RectInit(&dstRect, driDrawable->x, driDrawable->y, driDrawable->width, driDrawable->height);

    // Create GmmResourceInfo
    gmmParams.Flags.Gpu.Video       = true;
    gmmParams.BaseWidth             = driDrawable->width;
    gmmParams.BaseHeight            = driDrawable->height;
    gmmParams.ArraySize             = 1;
    gmmParams.Type                  = RESOURCE_2D;
    gmmParams.Format                = GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
    //gmmParams.Format                = GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
    target.OsResource.pGmmResInfo = mediaCtx->pGmmClientContext->CreateResInfoObject(&gmmParams);
    if (nullptr == target.OsResource.pGmmResInfo)
    {
        mos_bo_unreference(drawableBo);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    target.OsResource.iWidth     = driDrawable->width;
    target.OsResource.iHeight    = driDrawable->height;
    target.OsResource.iPitch     = buffer->dri2.pitch;
    target.OsResource.Format     = Format_A8R8G8B8;
    target.OsResource.iCount     = 0;
    target.OsResource.bo         = drawableBo;
    target.OsResource.pData      = (uint8_t *)drawableBo->virt;
    target.OsResource.TileType   = tileType;
    target.TileType              = tileType;
    target.dwWidth               = driDrawable->width;
    target.dwHeight              = driDrawable->height;
    target.dwPitch               = target.OsResource.iPitch;
    target.ColorSpace            = CSpace_sRGB;
    target.ExtendedGamut         = false;
    target.rcSrc                 = srcRect;
    target.rcDst                 = dstRect;

    renderParams.uSrcCount                  = 1;
    renderParams.uDstCount                  = 1;
    renderParams.pSrc[0]                    = &surf;
    renderParams.pTarget[0]                 = &target;
    renderParams.pColorFillParams           = &colorFill;
    renderParams.pColorFillParams->Color    = 0xFF000000;
    renderParams.pColorFillParams->bYCbCr   = false;
    renderParams.pColorFillParams->CSpace   = CSpace_sRGB;

    MosUtilities::MosLockMutex(&mediaCtx->PutSurfaceRenderMutex);
    eStatus = vpHal->Render(&renderParams);
    if (MOS_FAILED(eStatus))
    {
        MosUtilities::MosUnlockMutex(&mediaCtx->PutSurfaceRenderMutex);
        mos_bo_unreference(drawableBo);
        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    MosUtilities::MosUnlockMutex(&mediaCtx->PutSurfaceRenderMutex);
    mos_bo_unreference(drawableBo);
    target.OsResource.bo         = nullptr;
    MosUtilities::MosLockMutex(&mediaCtx->PutSurfaceSwapBufferMutex);
    driVtable->swap_buffer(ctx, driDrawable);
    MosUtilities::MosUnlockMutex(&mediaCtx->PutSurfaceSwapBufferMutex);

    mediaCtx->pGmmClientContext->DestroyResInfoObject(target.OsResource.pGmmResInfo);
    target.OsResource.pGmmResInfo = nullptr;

    return VA_STATUS_SUCCESS;
}
#endif // !defined(ANDROID) && defined(X11_FOUND)
