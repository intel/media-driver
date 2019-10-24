/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file      media_libva_encoder.cpp
//! \brief     libva(and its extension) encoder implementation
//!
#include <unistd.h>
#include "media_libva_encoder.h"
#include "media_ddi_encode_base.h"
#include "media_libva_util.h"
#include "media_libva_caps.h"
#include "media_ddi_factory.h"

#include "hwinfo_linux.h"
#include "codechal_memdecomp.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"

typedef MediaDdiFactoryNoArg<DdiEncodeBase> DdiEncodeFactory;

PDDI_ENCODE_CONTEXT DdiEncode_GetEncContextFromContextID(VADriverContextP ctx, VAContextID vaCtxID)
{
    uint32_t  ctxType;
    return (PDDI_ENCODE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, vaCtxID, &ctxType);
}

VAStatus DdiEncode_RemoveFromStatusReportQueue(
    PDDI_ENCODE_CONTEXT encCtx,
    PDDI_MEDIA_BUFFER   buf)

{
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->RemoveFromStatusReportQueue(buf);

    return vaStatus;
}

VAStatus DdiEncode_RemoveFromEncStatusReportQueue(
    PDDI_ENCODE_CONTEXT            encCtx,
    PDDI_MEDIA_BUFFER              buf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->RemoveFromEncStatusReportQueue(buf, typeIdx);

    return vaStatus;
}

VAStatus DdiEncode_RemoveFromPreEncStatusReportQueue(
    PDDI_ENCODE_CONTEXT            encCtx,
    PDDI_MEDIA_BUFFER              buf,
    DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->RemoveFromPreEncStatusReportQueue(buf, typeIdx);

    return vaStatus;
}

bool DdiEncode_CodedBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT encCtx,
    PDDI_MEDIA_BUFFER   buf)
{
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    return encCtx->m_encode->CodedBufferExistInStatusReport(buf);
}

bool DdiEncode_EncBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT            encCtx,
    PDDI_MEDIA_BUFFER              buf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    return encCtx->m_encode->EncBufferExistInStatusReport(buf, typeIdx);
}

bool DdiEncode_PreEncBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT            encCtx,
    PDDI_MEDIA_BUFFER              buf,
    DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    return encCtx->m_encode->PreEncBufferExistInStatusReport(buf, typeIdx);
}

VAStatus DdiEncode_EncStatusReport(
    PDDI_ENCODE_CONTEXT encCtx,
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **buf)
{
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaBuf, "nullptr mediaBuf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->EncStatusReport(mediaBuf, buf);

    return vaStatus;
}

VAStatus DdiEncode_PreEncStatusReport(
    PDDI_ENCODE_CONTEXT encCtx,
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **buf)
{
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaBuf, "nullptr mediaBuf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->PreEncStatusReport(mediaBuf, buf);

    return vaStatus;
}

VAStatus DdiEncode_StatusReport(
    PDDI_ENCODE_CONTEXT encCtx,
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **buf)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_DDI);

    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(mediaBuf, "nullptr mediaBuf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->StatusReport(mediaBuf, buf);

    return vaStatus;
}

//!
//! \brief  Clean and free encode context structure
//!
//! \param  [in] encCtx
//!     Pointer to ddi encode context
//!
void DdiEncodeCleanUp(PDDI_ENCODE_CONTEXT encCtx)
{
    if (encCtx->m_encode)
    {
        MOS_Delete(encCtx->m_encode);
        encCtx->m_encode = nullptr;
    }

    if (encCtx->pCpDdiInterface)
    {
        Delete_DdiCpInterface(encCtx->pCpDdiInterface);
        encCtx->pCpDdiInterface = nullptr;
    }

    MOS_FreeMemory(encCtx);
    encCtx = nullptr;

    return;
}

/*
 *  vpgEncodeCreateContext - Create an encode context
 *  dpy: display
 *  config_id: configuration for the encode context
 *  picture_width: encode picture width
 *  picture_height: encode picture height
 *  flag: any combination of the following:
 *  VA_PROGRESSIVE (only progressive frame pictures in the sequence when set)
 *  render_targets: render targets (surfaces) tied to the context
 *  num_render_targets: number of render targets in the above array
 *  context: created context id upon return
 */
VAStatus DdiEncode_CreateContext(
    VADriverContextP ctx,
    VAConfigID       config_id,
    int32_t          picture_width,
    int32_t          picture_height,
    int32_t          flag,
    VASurfaceID     *render_targets,
    int32_t          num_render_targets,
    VAContextID     *context)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_DDI);

    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(ctx->pDriverData, "nullptr ctx->pDriverData", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaDrvCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaDrvCtx->m_caps, "nullptr m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAProfile profile;
    VAEntrypoint entrypoint;
    uint32_t rcMode = 0;
    uint32_t feiFunction = 0;
    VAStatus vaStatus = mediaDrvCtx->m_caps->GetEncConfigAttr(
            config_id + DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE,
            &profile,
            &entrypoint,
            &rcMode,
            &feiFunction);
    DDI_CHK_RET(vaStatus, "Invalide config_id!");

    vaStatus = mediaDrvCtx->m_caps->CheckEncodeResolution(
            profile,
            picture_width,
            picture_height);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
    }

    if (num_render_targets > DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    std::string    encodeKey = mediaDrvCtx->m_caps->GetEncodeCodecKey(profile, entrypoint, feiFunction);
    DdiEncodeBase *ddiEncode = DdiEncodeFactory::CreateCodec(encodeKey);
    DDI_CHK_NULL(ddiEncode, "nullptr ddiEncode", VA_STATUS_ERROR_UNIMPLEMENTED);

    // first create encoder context
    ddiEncode->m_encodeCtx = (PDDI_ENCODE_CONTEXT)MOS_AllocAndZeroMemory(sizeof(DDI_ENCODE_CONTEXT));
    DDI_CHK_NULL(ddiEncode->m_encodeCtx, "nullptr ddiEncode->m_encodeCtx", VA_STATUS_ERROR_ALLOCATION_FAILED);

    PDDI_ENCODE_CONTEXT encCtx = ddiEncode->m_encodeCtx;
    encCtx->m_encode           = ddiEncode;

    //initialize DDI level cp interface
    MOS_CONTEXT mosCtx = { };
    encCtx->pCpDdiInterface = Create_DdiCpInterface(mosCtx);
    if (nullptr == encCtx->pCpDdiInterface)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiEncodeCleanUp(encCtx);
        return vaStatus;
    }

    // Get the buf manager for codechal create
    mosCtx.bufmgr          = mediaDrvCtx->pDrmBufMgr;
    mosCtx.m_gpuContextMgr = mediaDrvCtx->m_gpuContextMgr;
    mosCtx.m_cmdBufMgr     = mediaDrvCtx->m_cmdBufMgr;
    mosCtx.fd              = mediaDrvCtx->fd;
    mosCtx.iDeviceId       = mediaDrvCtx->iDeviceId;
    mosCtx.SkuTable        = mediaDrvCtx->SkuTable;
    mosCtx.WaTable         = mediaDrvCtx->WaTable;
    mosCtx.gtSystemInfo    = *mediaDrvCtx->pGtSystemInfo;
    mosCtx.platform        = mediaDrvCtx->platform;

    mosCtx.ppMediaMemDecompState = &mediaDrvCtx->pMediaMemDecompState;
    mosCtx.pfnMemoryDecompress   = mediaDrvCtx->pfnMemoryDecompress;
    mosCtx.pPerfData             = (PERF_DATA *)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));
    mosCtx.gtSystemInfo          = *mediaDrvCtx->pGtSystemInfo;
    mosCtx.m_auxTableMgr         = mediaDrvCtx->m_auxTableMgr;
    mosCtx.pGmmClientContext     = mediaDrvCtx->pGmmClientContext;

    mosCtx.m_osDeviceContext     = mediaDrvCtx->m_osDeviceContext;

    if (nullptr == mosCtx.pPerfData)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiEncodeCleanUp(encCtx);
        return vaStatus;
    }

    encCtx->vaEntrypoint  = entrypoint;
    encCtx->vaProfile     = profile;
    encCtx->uiRCMethod    = rcMode;
    encCtx->wModeType     = mediaDrvCtx->m_caps->GetEncodeCodecMode(profile, entrypoint);
    encCtx->codecFunction = mediaDrvCtx->m_caps->GetEncodeCodecFunction(profile, entrypoint, feiFunction);

    if (entrypoint == VAEntrypointEncSliceLP)
    {
        encCtx->bVdencActive  = true;
    }

    //Both dual pipe and LP pipe should support 10bit for below profiles
    // - HEVCMain10 profile
    // - VAProfileVP9Profile2
    // - VAProfileVP9Profile3
    if (profile == VAProfileVP9Profile2 ||
        profile == VAProfileVP9Profile3)
    {
        encCtx->m_encode->m_is10Bit = true;
    }

    if (profile == VAProfileVP9Profile1 ||
        profile == VAProfileVP9Profile3)
    {
        encCtx->m_encode->m_chromaFormat = DdiEncodeBase::yuv444;
    }

    CODECHAL_STANDARD_INFO standardInfo;
    MOS_ZeroMemory(&standardInfo, sizeof(CODECHAL_STANDARD_INFO));
    standardInfo.CodecFunction = encCtx->codecFunction;
    standardInfo.Mode          = encCtx->wModeType;
    Codechal *pCodecHal = CodechalDevice::CreateFactory(
        nullptr,
        &mosCtx,
        &standardInfo,
        nullptr);
    if (pCodecHal == nullptr)
    {
        // add anything necessary here to free the resource
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiEncodeCleanUp(encCtx);
        return vaStatus;
    }

    encCtx->pCodecHal = pCodecHal;

    // Setup some initial data
    encCtx->dworiFrameWidth   = picture_width;
    encCtx->dworiFrameHeight  = picture_height;
    encCtx->wPicWidthInMB     = (uint16_t)(DDI_CODEC_NUM_MACROBLOCKS_WIDTH(picture_width));
    encCtx->wPicHeightInMB    = (uint16_t)(DDI_CODEC_NUM_MACROBLOCKS_HEIGHT(picture_height));
    encCtx->dwFrameWidth      = encCtx->wPicWidthInMB * CODECHAL_MACROBLOCK_WIDTH;
    encCtx->dwFrameHeight     = encCtx->wPicHeightInMB * CODECHAL_MACROBLOCK_HEIGHT;
    //recoder old resolution for dynamic resolution  change
    encCtx->wContextPicWidthInMB  = encCtx->wPicWidthInMB;
    encCtx->wContextPicHeightInMB = encCtx->wPicHeightInMB;
    encCtx->wOriPicWidthInMB      = encCtx->wPicWidthInMB;
    encCtx->wOriPicHeightInMB     = encCtx->wPicHeightInMB;
    encCtx->targetUsage           = TARGETUSAGE_RT_SPEED;
    // Attach PMEDIDA_DRIVER_CONTEXT
    encCtx->pMediaCtx = mediaDrvCtx;

    encCtx->pCpDdiInterface->SetHdcp2Enabled(flag);
    encCtx->pCpDdiInterface->SetCpParams(CP_TYPE_NONE, encCtx->m_encode->m_codechalSettings);

    vaStatus = encCtx->m_encode->ContextInitialize(encCtx->m_encode->m_codechalSettings);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        DdiEncodeCleanUp(encCtx);
        return vaStatus;
    }

    encCtx->m_encode->m_codechalSettings->enableCodecMmc = false;
    MOS_STATUS eStatus = pCodecHal->Allocate(encCtx->m_encode->m_codechalSettings);

#ifdef _MMC_SUPPORTED
    PMOS_INTERFACE osInterface = pCodecHal->GetOsInterface();
    if (osInterface != nullptr &&
        MEDIA_IS_SKU(osInterface->pfnGetSkuTable(osInterface), FtrMemoryCompression) &&
        !mediaDrvCtx->pMediaMemDecompState)
    {
        mediaDrvCtx->pMediaMemDecompState =
            static_cast<MediaMemDecompState*>(MmdDevice::CreateFactory(&mosCtx));
    }
#endif

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiEncodeCleanUp(encCtx);
        return vaStatus;
    }

    vaStatus = encCtx->m_encode->InitCompBuffer();
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiEncodeCleanUp(encCtx);
        return vaStatus;
    }

    // register the render target surfaces for this encoder instance
    // This is a must as driver has the constraint, 127 surfaces per context
    for (int32_t i = 0; i < num_render_targets; i++)
    {
        DDI_MEDIA_SURFACE *surface;

        surface = DdiMedia_GetSurfaceFromVASurfaceID(mediaDrvCtx, render_targets[i]);
        if (nullptr == surface)
        {
            DDI_ASSERTMESSAGE("DDI: invalid render target %d in vpgEncodeCreateContext.", i);
            vaStatus = VA_STATUS_ERROR_INVALID_SURFACE;
            DdiEncodeCleanUp(encCtx);
            return vaStatus;
        }
        encCtx->RTtbl.pRT[i] = surface;
        encCtx->RTtbl.iNumRenderTargets++;
    }

    // convert PDDI_ENCODE_CONTEXT to VAContextID
    DdiMediaUtil_LockMutex(&mediaDrvCtx->EncoderMutex);
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaContextHeapElmt = DdiMediaUtil_AllocPVAContextFromHeap(mediaDrvCtx->pEncoderCtxHeap);
    if (nullptr == vaContextHeapElmt)
    {
        DdiMediaUtil_UnLockMutex(&mediaDrvCtx->EncoderMutex);
        vaStatus = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        DdiEncodeCleanUp(encCtx);
        return vaStatus;
    }

    vaContextHeapElmt->pVaContext = (void*)encCtx;
    mediaDrvCtx->uiNumEncoders++;
    *context = (VAContextID)(vaContextHeapElmt->uiVaContextID + DDI_MEDIA_VACONTEXTID_OFFSET_ENCODER);
    DdiMediaUtil_UnLockMutex(&mediaDrvCtx->EncoderMutex);

    return vaStatus;
}

/*
 * vaDestroyContext - Destroy a context
 * dpy: display
 * context: context to be destroyed
 */
VAStatus DdiEncode_DestroyContext(VADriverContextP ctx, VAContextID context)
{
    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(ctx->pDriverData, "nullptr ctx->pDriverData", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    PDDI_ENCODE_CONTEXT encCtx  = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->pCodecHal, "nullptr encCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    Codechal *codecHal = encCtx->pCodecHal;

    if (nullptr != encCtx->m_encode)
    {
        encCtx->m_encode->FreeCompBuffer();
        if(nullptr != encCtx->m_encode->m_codechalSettings)
        {
            MOS_Delete(encCtx->m_encode->m_codechalSettings);
            encCtx->m_encode->m_codechalSettings = nullptr;
        }
    }

    MOS_FreeMemory(codecHal->GetOsInterface()->pOsContext->pPerfData);
    codecHal->GetOsInterface()->pOsContext->pPerfData = nullptr;

    // destroy codechal
    codecHal->Destroy();
    MOS_Delete(codecHal);

    if (encCtx->pCpDdiInterface)
    {
        Delete_DdiCpInterface(encCtx->pCpDdiInterface);
        encCtx->pCpDdiInterface = nullptr;
    }

    if (nullptr != encCtx->m_encode)
    {
        MOS_Delete(encCtx->m_encode);
        encCtx->m_encode = nullptr;
    }

    MOS_FreeMemory(encCtx);
    encCtx = nullptr;

    uint32_t encIndex = (uint32_t)context;
    encIndex &= DDI_MEDIA_MASK_VACONTEXTID;

    DdiMediaUtil_LockMutex(&mediaCtx->EncoderMutex);
    DdiMediaUtil_ReleasePVAContextFromHeap(mediaCtx->pEncoderCtxHeap, encIndex);
    mediaCtx->uiNumEncoders--;
    DdiMediaUtil_UnLockMutex(&mediaCtx->EncoderMutex);

    return VA_STATUS_SUCCESS;
}

/*
 * Creates a buffer for "num_elements" elements of "size" bytes and
 * initalize with "data".
 * if "data" is null, then the contents of the buffer data store
 * are undefined.
 * Basically there are two ways to get buffer data to the server side. One is
 * to call vaCreateBuffer() with a non-null "data", which results the data being
 * copied to the data store on the server side.  A different method that
 * eliminates this copy is to pass null as "data" when calling vaCreateBuffer(),
 * and then use vaMapBuffer() to map the data store from the server side to the
 * client address space for access.
 * Note: image buffers are created by the library, not the client. Please see
 * vaCreateImage on how image buffers are managed.
 */

VAStatus DdiEncode_CreateBuffer(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferType     type,
    uint32_t         size,
    uint32_t         num_elements,
    void            *data,
    VABufferID      *buf_id)
{
    DDI_CHK_NULL(ctx, "nullptr context!", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_ENCODE_CONTEXT *encCtx = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = encCtx->m_encode->CreateBuffer(ctx, type, size, num_elements, data, buf_id);

    return vaStatus;
}

/*
 * Get ready to encode a picture
 */
VAStatus DdiEncode_BeginPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VASurfaceID      render_target)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_DDI);

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context in vpgEncodeBeginPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    PDDI_ENCODE_CONTEXT encCtx = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = encCtx->m_encode->BeginPicture(ctx, context, render_target);
    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

/*
 * Send encode buffers to the server.
 * Buffers are automatically destroyed afterwards
 */
VAStatus DdiEncode_RenderPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VABufferID      *buffers,
    int32_t          num_buffers)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_DDI);

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context in vpgEncodeRenderPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    PDDI_ENCODE_CONTEXT encCtx = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = encCtx->m_encode->RenderPicture(ctx, context, buffers, num_buffers);
    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

VAStatus DdiEncode_EndPicture(VADriverContextP ctx, VAContextID context)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_DDI);

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context in vpgEncodeEndPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    PDDI_ENCODE_CONTEXT encCtx = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = encCtx->m_encode->EndPicture(ctx, context);
    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

VAStatus DdiEncode_MfeSubmit(
    VADriverContextP    ctx,
    VAMFContextID      mfe_context,
    VAContextID        *contexts,
    int32_t             num_contexts
)
{
    PDDI_MEDIA_CONTEXT mediaCtx               = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t ctxType                          = DDI_MEDIA_CONTEXT_TYPE_NONE;
    PDDI_ENCODE_MFE_CONTEXT encodeMfeContext  = (PDDI_ENCODE_MFE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, mfe_context, &ctxType);
    DDI_CHK_NULL(encodeMfeContext, "nullptr encodeMfeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    std::vector<PDDI_ENCODE_CONTEXT>    encodeContexts;
    PDDI_ENCODE_CONTEXT encodeContext = nullptr;
    int32_t validContextNumber        = 0;
    // Set mfe encoder params for this submission
    for (int32_t i = 0; i < num_contexts; i++)
    {
        encodeContext                 = DdiEncode_GetEncContextFromContextID(ctx, contexts[i]);
        DDI_CHK_NULL(encodeContext, "nullptr encodeContext", VA_STATUS_ERROR_INVALID_CONTEXT);
        CodechalEncoderState *encoder = dynamic_cast<CodechalEncoderState *>(encodeContext->pCodecHal);
        DDI_CHK_NULL(encoder, "nullptr codechal encoder", VA_STATUS_ERROR_INVALID_CONTEXT);

        if (!encoder->m_mfeEnabled ||
            encoder->m_mfeEncodeSharedState != encodeMfeContext->mfeEncodeSharedState)
        {
            return VA_STATUS_ERROR_INVALID_CONTEXT;
        }

        // make sure the context has called BeginPicture&RenderPicture&EndPicture
        if (encodeContext->RTtbl.pRT[0] == nullptr
            || encodeContext->dwNumSlices <= 0
            || encodeContext->EncodeParams.pBSBuffer != encodeContext->pbsBuffer)
        {
            return VA_STATUS_ERROR_INVALID_PARAMETER;
        }

        encoder->m_mfeEncodeParams.submitIndex  = i;
        encoder->m_mfeEncodeParams.submitNumber = num_contexts;
        encodeContexts.push_back(encodeContext);
        validContextNumber++;
    }

    CmDevice *device = encodeMfeContext->mfeEncodeSharedState->pCmDev;
    CmTask   *task   = encodeMfeContext->mfeEncodeSharedState->pCmTask;
    CmQueue  *queue  = encodeMfeContext->mfeEncodeSharedState->pCmQueue;
    CodechalEncodeMdfKernelResource *resMbencKernel = encodeMfeContext->mfeEncodeSharedState->resMbencKernel;
    SurfaceIndex *vmeSurface    = encodeMfeContext->mfeEncodeSharedState->vmeSurface;
    SurfaceIndex *commonSurface = encodeMfeContext->mfeEncodeSharedState->commonSurface;


    MOS_ZeroMemory(encodeMfeContext->mfeEncodeSharedState, sizeof(MfeSharedState));

    encodeMfeContext->mfeEncodeSharedState->pCmDev   = device;
    encodeMfeContext->mfeEncodeSharedState->pCmTask  = task;
    encodeMfeContext->mfeEncodeSharedState->pCmQueue = queue;
    encodeMfeContext->mfeEncodeSharedState->resMbencKernel = resMbencKernel;
    encodeMfeContext->mfeEncodeSharedState->vmeSurface     = vmeSurface;
    encodeMfeContext->mfeEncodeSharedState->commonSurface  = commonSurface;

    encodeMfeContext->mfeEncodeSharedState->encoders.clear();

    // Call Enc functions for all the sub contexts
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    for (int32_t i = 0; i < validContextNumber; i++)
    {
        encodeContext  = encodeContexts[i];
        if (encodeContext->vaEntrypoint != VAEntrypointFEI )
        {
            encodeContext->EncodeParams.ExecCodecFunction = CODECHAL_FUNCTION_ENC;
        }
        else
        {
            encodeContext->EncodeParams.ExecCodecFunction = CODECHAL_FUNCTION_FEI_ENC;
        }

        CodechalEncoderState *encoder = dynamic_cast<CodechalEncoderState *>(encodeContext->pCodecHal);

        encodeMfeContext->mfeEncodeSharedState->encoders.push_back(encoder);

        status = encoder->Execute(&encodeContext->EncodeParams);
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:Failed in Execute Enc!");
            return VA_STATUS_ERROR_ENCODING_ERROR;
        }
    }

    // Call Pak functions for all the sub contexts
    for (int32_t i = 0; i < validContextNumber; i++)
    {
        encodeContext  = encodeContexts[i];
        if (encodeContext->vaEntrypoint != VAEntrypointFEI )
        {
            encodeContext->EncodeParams.ExecCodecFunction = CODECHAL_FUNCTION_PAK;
        }
        else
        {
            encodeContext->EncodeParams.ExecCodecFunction = CODECHAL_FUNCTION_FEI_PAK;
        }

        CodechalEncoderState *encoder = dynamic_cast<CodechalEncoderState *>(encodeContext->pCodecHal);
        status = encoder->Execute(&encodeContext->EncodeParams);
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:Failed in Execute Pak!");
            return VA_STATUS_ERROR_ENCODING_ERROR;
        }
    }

    return VA_STATUS_SUCCESS;
}

