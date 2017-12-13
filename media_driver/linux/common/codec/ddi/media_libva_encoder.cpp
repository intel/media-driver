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
//! \file      media_libva_encoder.cpp 
//! \brief     libva(and its extension) encoder implementation  
//!
#include <unistd.h>
#include "media_libva_encoder.h"
#include "media_ddi_encode_base.h"
#include "media_libva_cp.h"
#include "media_libva_util.h"
#include "media_libva_caps.h"
#include "media_ddi_factory.h"

#include "hwinfo_linux.h"
#include "codechal_memdecomp.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"

// encoder capability
#define DDI_ENCODE_ENCODE_PIC_WIDTH_MIN 32
#define DDI_ENCODE_ENCODE_PIC_HEIGHT_MIN 32

#define DDI_ENCODE_ENCODE_JPEG_PIC_WIDTH_MIN 16
#define DDI_ENCODE_ENCODE_JPEG_PIC_HEIGHT_MIN 16

extern int32_t MosMemAllocCounter;
extern int32_t MosMemAllocCounterGfx;

typedef MediaDdiFactoryNoArg<DdiEncodeBase> DdiEncodeFactory;

PDDI_ENCODE_CONTEXT DdiEncode_GetEncContextFromContextID(VADriverContextP ctx, VAContextID vaCtxID)
{
    uint32_t  uiCtxType;
    return (PDDI_ENCODE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, vaCtxID, &uiCtxType);
}

VAStatus DdiEncode_RemoveFromStatusReportQueue(
    PDDI_ENCODE_CONTEXT pEncCtx,
    PDDI_MEDIA_BUFFER   pBuf)

{
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pBuf, "Null pbuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = pEncCtx->m_encode->RemoveFromStatusReportQueue(pBuf);

    return vaStatus;
}

VAStatus DdiEncode_RemoveFromEncStatusReportQueue(
    PDDI_ENCODE_CONTEXT            pEncCtx,
    PDDI_MEDIA_BUFFER              pBuf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE TypeIdx)
{
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pBuf, "Null pbuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = pEncCtx->m_encode->RemoveFromEncStatusReportQueue(pBuf, TypeIdx);

    return vaStatus;
}

VAStatus DdiEncode_RemoveFromPreEncStatusReportQueue(
    PDDI_ENCODE_CONTEXT            pEncCtx,
    PDDI_MEDIA_BUFFER              pBuf,
    DDI_ENCODE_PRE_ENC_BUFFER_TYPE TypeIdx)
{
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pBuf, "Null pbuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = pEncCtx->m_encode->RemoveFromPreEncStatusReportQueue(pBuf, TypeIdx);

    return vaStatus;
}

bool DdiEncode_CodedBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT pEncCtx,
    PDDI_MEDIA_BUFFER   pBuf)
{
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pBuf, "Null pbuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    return pEncCtx->m_encode->CodedBufferExistInStatusReport(pBuf);
}

bool DdiEncode_EncBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT            pEncCtx,
    PDDI_MEDIA_BUFFER              pBuf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE TypeIdx)
{
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pBuf, "Null pbuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    return pEncCtx->m_encode->EncBufferExistInStatusReport(pBuf, TypeIdx);
}

bool DdiEncode_PreEncBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT            pEncCtx,
    PDDI_MEDIA_BUFFER              pBuf,
    DDI_ENCODE_PRE_ENC_BUFFER_TYPE TypeIdx)
{
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pBuf, "Null pbuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    return pEncCtx->m_encode->PreEncBufferExistInStatusReport(pBuf, TypeIdx);
}

VAStatus DdiEncode_EncStatusReport(
    PDDI_ENCODE_CONTEXT pEncCtx,
    DDI_MEDIA_BUFFER *  pMediaBuf,
    void **             pbuf)
{
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaBuf, "Null pMediaBuf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pbuf, "Null pbuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = pEncCtx->m_encode->EncStatusReport(pMediaBuf, pbuf);

    return vaStatus;
}

VAStatus DdiEncode_PreEncStatusReport(
    PDDI_ENCODE_CONTEXT pEncCtx,
    DDI_MEDIA_BUFFER *  pMediaBuf,
    void **             pbuf)
{
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaBuf, "Null pMediaBuf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pbuf, "Null pbuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = pEncCtx->m_encode->PreEncStatusReport(pMediaBuf, pbuf);

    return vaStatus;
}

VAStatus DdiEncode_StatusReport(
    PDDI_ENCODE_CONTEXT pEncCtx,
    DDI_MEDIA_BUFFER *  pMediaBuf,
    void **             pbuf)
{
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pMediaBuf, "Null pMediaBuf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(pbuf, "Null pbuf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = pEncCtx->m_encode->StatusReport(pMediaBuf, pbuf);

    return vaStatus;
}

void DdiEncodeCleanUp(PDDI_ENCODE_CONTEXT pEncCtx)
{
    if (pEncCtx->m_encode)
    {
        MOS_Delete(pEncCtx->m_encode);
        pEncCtx->m_encode = nullptr;
    }

    if (pEncCtx->pCpDdiInterface)
    {
        MOS_Delete(pEncCtx->pCpDdiInterface);
        pEncCtx->pCpDdiInterface = nullptr;
    }

    MOS_FreeMemory(pEncCtx);
    pEncCtx = nullptr;

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
    if (ctx == nullptr || ctx->pDriverData == nullptr)
    {
        return VA_STATUS_ERROR_INVALID_CONTEXT;
    }

    PDDI_MEDIA_CONTEXT pMediaDrvCtx = DdiMedia_GetMediaContext(ctx);
    uint32_t           dwMaxWidth   = ENCODE_MAX_PIC_WIDTH;
    uint32_t           dwMaxHeight  = ENCODE_MAX_PIC_HEIGHT;
    DDI_CHK_NULL(pMediaDrvCtx->m_caps, "Null m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);
    VAProfile profile;
    VAEntrypoint entrypoint;
    uint32_t rcMode;
    VAStatus vaStatus = pMediaDrvCtx->m_caps->GetEncConfigAttr(
		    config_id + DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE, 
		    &profile,
		    &entrypoint,
		    &rcMode);
    DDI_CHK_RET(vaStatus, "Invalide config_id!");

    vaStatus = pMediaDrvCtx->m_caps->CheckEncodeResolution(
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
    
    std::string    encodeKey = pMediaDrvCtx->m_caps->GetEncodeCodecKey(profile, entrypoint);
    DdiEncodeBase *ddiEncode = DdiEncodeFactory::CreateCodec(encodeKey);
    DDI_CHK_NULL(ddiEncode, "nullptr ddiEncode", VA_STATUS_ERROR_UNIMPLEMENTED);
    
    // first create encoder context
    ddiEncode->m_encodeCtx = (PDDI_ENCODE_CONTEXT)MOS_AllocAndZeroMemory(sizeof(DDI_ENCODE_CONTEXT));
    DDI_CHK_NULL(ddiEncode->m_encodeCtx, "nullptr ddiEncode->m_encodeCtx", VA_STATUS_ERROR_ALLOCATION_FAILED);

    PDDI_ENCODE_CONTEXT pEncCtx = ddiEncode->m_encodeCtx;
    pEncCtx->m_encode           = ddiEncode;

    //initialize DDI level cp interface
    MOS_CONTEXT MosCtx;
    pEncCtx->pCpDdiInterface = MOS_New(DdiCpInterface, MosCtx);
    if (nullptr == pEncCtx->pCpDdiInterface)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiEncodeCleanUp(pEncCtx);
        return vaStatus;
    }

    // Get the buf manager for codechal create
    MosCtx.bufmgr       = pMediaDrvCtx->pDrmBufMgr;
    MosCtx.fd           = pMediaDrvCtx->fd;
    MosCtx.iDeviceId    = pMediaDrvCtx->iDeviceId;
    MosCtx.SkuTable     = pMediaDrvCtx->SkuTable;
    MosCtx.WaTable      = pMediaDrvCtx->WaTable;
    MosCtx.gtSystemInfo = *pMediaDrvCtx->pGtSystemInfo;
    MosCtx.platform     = pMediaDrvCtx->platform;

    MosCtx.ppMediaMemDecompState = &pMediaDrvCtx->pMediaMemDecompState;
    MosCtx.pfnMemoryDecompress   = pMediaDrvCtx->pfnMemoryDecompress;
    MosCtx.pPerfData             = (PERF_DATA *)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));
    MosCtx.gtSystemInfo          = *pMediaDrvCtx->pGtSystemInfo;

    if (nullptr == MosCtx.pPerfData)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiEncodeCleanUp(pEncCtx);
        return vaStatus;
    }

    pEncCtx->vaProfile  = profile;
    pEncCtx->uiRCMethod = rcMode;
    pEncCtx->wModeType = pMediaDrvCtx->m_caps->GetEncodeCodecMode(profile, entrypoint);
    pEncCtx->codecFunction = pMediaDrvCtx->m_caps->GetEncodeCodecFunction(profile, entrypoint);

    if (entrypoint == VAEntrypointEncSliceLP)
    {
        pEncCtx->bVdencActive  = true;
    }

    //Both dual pipe and LP pipe should support 10bit for HEVCMain10 profile
    if (profile == VAProfileHEVCMain10)
    {
        pEncCtx->bIs10Bit = true;
    }

    CODECHAL_STANDARD_INFO StandardInfo;
    MOS_ZeroMemory(&StandardInfo, sizeof(CODECHAL_STANDARD_INFO));
    StandardInfo.CodecFunction = pEncCtx->codecFunction;
    StandardInfo.Mode          = pEncCtx->wModeType;
    Codechal *pCodecHal = CodechalDevice::CreateFactory(
        nullptr,
        &MosCtx,
        &StandardInfo,
        nullptr);
    if (pCodecHal == nullptr)
    {
        // add anything necessary here to free the resource
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiEncodeCleanUp(pEncCtx);
        return vaStatus;
    }

    pEncCtx->pCodecHal = pCodecHal;

    // Setup some initial data
    pEncCtx->dwFrameWidth      = picture_width;
    pEncCtx->dwFrameHeight     = picture_height;
    pEncCtx->bMultiCallsPerPic = 0;  // bit 31 for multi call indication
    pEncCtx->usNumCallsPerPic  = 1;  // bit 0-15 indicate number of calls.
    pEncCtx->wPicWidthInMB     = (uint16_t)(DDI_CODEC_NUM_MACROBLOCKS_WIDTH(picture_width));
    pEncCtx->wPicHeightInMB    = (uint16_t)(DDI_CODEC_NUM_MACROBLOCKS_HEIGHT(picture_height));
    //recoder old resolution for dynamic resolution  change
    pEncCtx->wContextPicWidthInMB  = pEncCtx->wPicWidthInMB;
    pEncCtx->wContextPicHeightInMB = pEncCtx->wPicHeightInMB;
    pEncCtx->wOriPicWidthInMB      = pEncCtx->wPicWidthInMB;
    pEncCtx->wOriPicHeightInMB     = pEncCtx->wPicHeightInMB;
    pEncCtx->ucCurrPAKSet          = 0;
    pEncCtx->ucCurrENCSet          = 0;
    pEncCtx->dwStoreData           = 1;
    pEncCtx->bFirstFrame           = 1;
    // Attach PMEDIDA_DRIVER_CONTEXT
    pEncCtx->pMediaCtx = pMediaDrvCtx;

    pEncCtx->pCpDdiInterface->SetHdcp2Enabled(flag);

    CODECHAL_SETTINGS CodecHalSettings;
    MOS_ZeroMemory(&CodecHalSettings, sizeof(CodecHalSettings));

    vaStatus = pEncCtx->m_encode->ContextInitialize(&CodecHalSettings);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        DdiEncodeCleanUp(pEncCtx);
        return vaStatus;
    }

    MOS_STATUS eStatus = pCodecHal->Allocate(&CodecHalSettings);

#ifdef _MMC_SUPPORTED
    PMOS_INTERFACE osInterface = pCodecHal->GetOsInterface();
    if (osInterface != nullptr &&
        MEDIA_IS_SKU(osInterface->pfnGetSkuTable(osInterface), FtrMemoryCompression) &&
        !pMediaDrvCtx->pMediaMemDecompState)
    {
        pMediaDrvCtx->pMediaMemDecompState =
            static_cast<MediaMemDecompState*>(MmdDevice::CreateFactory(&MosCtx));
    }
#endif

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiEncodeCleanUp(pEncCtx);
        return vaStatus;
    }

    vaStatus = pEncCtx->m_encode->InitCompBuffer();
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        DdiEncodeCleanUp(pEncCtx);
        return vaStatus;
    }

    // register the render target surfaces for this encoder instance
    // This is a must as driver has the constraint, 127 surfaces per context
    for (int32_t i = 0; i < num_render_targets; i++)
    {
        DDI_MEDIA_SURFACE *pSurface;

        pSurface = DdiMedia_GetSurfaceFromVASurfaceID(pMediaDrvCtx, render_targets[i]);
        if (nullptr == pSurface)
        {
            DDI_ASSERTMESSAGE("DDI: invalid render target %d in vpgEncodeCreateContext.", i);
            vaStatus = VA_STATUS_ERROR_INVALID_SURFACE;
            DdiEncodeCleanUp(pEncCtx);
            return vaStatus;
        }
        pEncCtx->RTtbl.pRT[i] = pSurface;
        pEncCtx->RTtbl.iNumRenderTargets++;
    }

    // convert PDDI_ENCODE_CONTEXT to VAContextID
    DdiMediaUtil_LockMutex(&pMediaDrvCtx->EncoderMutex);
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT pVaContextHeapElmt = DdiMediaUtil_AllocPVAContextFromHeap(pMediaDrvCtx->pEncoderCtxHeap);
    if (nullptr == pVaContextHeapElmt)
    {
        DdiMediaUtil_UnLockMutex(&pMediaDrvCtx->EncoderMutex);
        vaStatus = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        DdiEncodeCleanUp(pEncCtx);
        return vaStatus;
    }

    pVaContextHeapElmt->pVaContext = (void*)pEncCtx;
    pMediaDrvCtx->uiNumEncoders++;
    *context = (VAContextID)(pVaContextHeapElmt->uiVaContextID + DDI_MEDIA_VACONTEXTID_OFFSET_ENCODER);
    DdiMediaUtil_UnLockMutex(&pMediaDrvCtx->EncoderMutex);

    return vaStatus;
}

/*
 * vaDestroyContext - Destroy a context
 * dpy: display
 * context: context to be destroyed
 */
VAStatus DdiEncode_DestroyContext(VADriverContextP ctx, VAContextID context)
{
    PDDI_MEDIA_CONTEXT                pMediaCtx;
    PDDI_ENCODE_CONTEXT               pEncCtx;
    Codechal                          *pCodecHal;
    uint32_t                          uiEncIndex;
    uint32_t                          MemoryCounter = 0;
    MOS_USER_FEATURE_VALUE_DATA       UserFeatureData;
    MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData;

    pMediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    // assume the VAContextID is encoder ID
    pEncCtx = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->pCodecHal, "Null pEncCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    pCodecHal = pEncCtx->pCodecHal;

    if (nullptr != pEncCtx->m_encode)
    {
        pEncCtx->m_encode->FreeCompBuffer();
    }

    MOS_FreeMemory(pCodecHal->GetOsInterface()->pOsContext->pPerfData);
    pCodecHal->GetOsInterface()->pOsContext->pPerfData = nullptr;

    // destroy codechal
    pCodecHal->Destroy();
    MOS_Delete(pCodecHal);

    if (pEncCtx->pCpDdiInterface)
    {
        MOS_Delete(pEncCtx->pCpDdiInterface);
        pEncCtx->pCpDdiInterface = nullptr;
    }

    if (nullptr != pEncCtx->m_encode)
    {
        MOS_Delete(pEncCtx->m_encode);
        pEncCtx->m_encode = nullptr;
    }

    MOS_FreeMemory(pEncCtx);
    pEncCtx = nullptr;

    uiEncIndex = (uint32_t)context;
    uiEncIndex &= DDI_MEDIA_MASK_VACONTEXTID;

    DdiMediaUtil_LockMutex(&pMediaCtx->EncoderMutex);
    DdiMediaUtil_ReleasePVAContextFromHeap(pMediaCtx->pEncoderCtxHeap, uiEncIndex);
    pMediaCtx->uiNumEncoders--;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->EncoderMutex);

    /***********************************************************************************
    MemNinja Release feature allows test applications to detect memory leak based on
    driver counter value, sum of the global variables MosMemAllocCounter and MosMemAllocCounterGfx
    which increment by 1 whenever system memory/graphics memory is allocated and decrement by 1
    when the system memory/graphics memory is freed.
    
    MemNinja Release feature is described below.
    1. MemNinja Counter - Driver repots the internal counter MosMemAllocCounter value
    when test completes. Test application checks this value.
    If MemNinjaCounter != 0, test app can flag test as fail.
    **************************************************************************************/

    //MemNinja Counter intialization
    MosMemAllocCounter    = 0;
    MosMemAllocCounterGfx = 0;
    MemoryCounter         = MosMemAllocCounter + MosMemAllocCounterGfx;

    UserFeatureWriteData               = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    UserFeatureWriteData.Value.i32Data = MemoryCounter;
    UserFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_MEMNINJA_COUNTER_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1);

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

    DDI_ENCODE_CONTEXT *pEncCtx = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "Null pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = pEncCtx->m_encode->CreateBuffer(ctx, type, size, num_elements, data, buf_id);

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
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context in vpgEncodeBeginPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    PDDI_ENCODE_CONTEXT pEncCtx = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "Null pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = pEncCtx->m_encode->BeginPicture(ctx, context, render_target);
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
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context in vpgEncodeRenderPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    PDDI_ENCODE_CONTEXT pEncCtx = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "Null pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = pEncCtx->m_encode->RenderPicture(ctx, context, buffers, num_buffers);
    DDI_FUNCTION_EXIT(vaStatus);
    return vaStatus;
}

VAStatus DdiEncode_EndPicture(VADriverContextP ctx, VAContextID context)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx, "nullptr context in vpgEncodeEndPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    PDDI_ENCODE_CONTEXT pEncCtx = DdiEncode_GetEncContextFromContextID(ctx, context);
    DDI_CHK_NULL(pEncCtx, "Null pEncCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pEncCtx->m_encode, "Null pEncCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = pEncCtx->m_encode->EndPicture(ctx, context);
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
    PDDI_MEDIA_CONTEXT                  pMediaCtx;
    PDDI_ENCODE_MFE_CONTEXT             pEncodeMfeContext;
    PDDI_ENCODE_CONTEXT                 pEncodeContext;
    CodechalEncoderState                *encoder;
    uint32_t                            uiCtxType = DDI_MEDIA_CONTEXT_TYPE_NONE;
    int32_t                             i;
    std::vector<PDDI_ENCODE_CONTEXT>    pEncodeContexts;
    EncoderParams                       *pEncodeParams;
    MOS_STATUS                          status ;
    int32_t                             validContextNumber = 0;

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    pEncodeMfeContext  = (PDDI_ENCODE_MFE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, mfe_context, &uiCtxType);
    DDI_CHK_NULL(pEncodeMfeContext, "Null pEncodeMfeContext", VA_STATUS_ERROR_INVALID_CONTEXT);

    // Set mfe encoder params for this submission
    for (i = 0; i < num_contexts; i++)
    {
        pEncodeContext  = DdiEncode_GetEncContextFromContextID(ctx, contexts[i]);
        DDI_CHK_NULL(pEncodeContext, "Null pEncodeContext", VA_STATUS_ERROR_INVALID_CONTEXT);
        encoder = dynamic_cast<CodechalEncoderState *>(pEncodeContext->pCodecHal);
        DDI_CHK_NULL(encoder, "Null codechal encoder", VA_STATUS_ERROR_INVALID_CONTEXT);

        encoder->m_mfeEncodeParams.submitIndex = i;
        encoder->m_mfeEncodeParams.submitNumber = num_contexts;
        pEncodeContexts.push_back(pEncodeContext);
        validContextNumber++;
    }

    MOS_ZeroMemory(pEncodeMfeContext->mfeEncodeSharedState, sizeof(MfeSharedState));

    // Call Enc functions for all the sub contexts
    for (i = 0; i < validContextNumber; i++)
    {
        pEncodeContext  = pEncodeContexts[i];
        pEncodeContext->EncodeParams.ExecCodecFunction = CODECHAL_FUNCTION_ENC;
        status = pEncodeContext->pCodecHal->Execute(&pEncodeContext->EncodeParams);
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:Failed in Execute Enc!");
            return VA_STATUS_ERROR_ENCODING_ERROR;
        }
    }

    // Call Pak functions for all the sub contexts
    for (i = 0; i < validContextNumber; i++)
    {
        pEncodeContext  = pEncodeContexts[i];
        pEncodeContext->EncodeParams.ExecCodecFunction = CODECHAL_FUNCTION_PAK;
        status = pEncodeContext->pCodecHal->Execute(&pEncodeContext->EncodeParams);
        if (MOS_STATUS_SUCCESS != status)
        {
            DDI_ASSERTMESSAGE("DDI:Failed in Execute Pak!");
            return VA_STATUS_ERROR_ENCODING_ERROR;
        }
    }

    return VA_STATUS_SUCCESS;
}

