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
//! \file     ddi_encode_functions.cpp
//! \brief    ddi encode functions implementaion.
//!
#include "ddi_encode_functions.h"
#include "media_libva_common_next.h"
#include "ddi_encode_hevc_specific.h"
#include "ddi_register_components_specific.h"
#include "codechal_memdecomp.h"
#include "media_interfaces_codechal_next.h"
#include "media_interfaces_mmd.h"
#include "media_libva_interface_next.h"

VAStatus DdiEncodeFunctions::CreateConfig (
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib   *attribList,
    int32_t           numAttribs,
    VAConfigID       *configId)
{
    VAStatus status = VA_STATUS_SUCCESS;
    DDI_CODEC_CHK_NULL(configId,   "nullptr configId",   VA_STATUS_ERROR_INVALID_PARAMETER);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx,             "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_caps",   VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(mediaCtx->m_capsNext->m_capsTable, "nullptr m_capsTable",   VA_STATUS_ERROR_INVALID_PARAMETER);

    status = mediaCtx->m_capsNext->CreateConfig(profile, entrypoint, attribList, numAttribs, configId);
    DDI_CODEC_CHK_RET(status, "Create common config failed");

    uint32_t rcMode = VA_RC_CQP;
    uint32_t feiFunction = 0;

    if((entrypoint == VAEntrypointStats) || (entrypoint == VAEntrypointEncPicture))
    {
        rcMode = VA_RC_NONE;
    }

    for(int i = 0; i < numAttribs; i++)
    {
        if(attribList[i].type == VAConfigAttribFEIFunctionType)
        {
            feiFunction = attribList[i].value;
        }

        if(attribList[i].type == VAConfigAttribRateControl && attribList[i].value != VA_RC_MB)
        {
            rcMode = attribList[i].value;
        }
    }

    auto configList = mediaCtx->m_capsNext->GetConfigList();
    DDI_CODEC_CHK_NULL(configList, "Get configList failed", VA_STATUS_ERROR_INVALID_PARAMETER);

    for(int i = 0; i < configList->size(); i++)
    {
        if((configList->at(i).profile == profile)        &&
           (configList->at(i).entrypoint == entrypoint))
        {
            if((rcMode      == configList->at(i).componentData.data.rcMode)       &&
               (feiFunction == configList->at(i).componentData.data.feiFunction))
            {
                uint32_t curConfigID = ADD_CONFIG_ID_ENC_OFFSET(i);
                if(!mediaCtx->m_capsNext->m_capsTable->IsEncConfigId(curConfigID))
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

VAStatus DdiEncodeFunctions::CreateContext (
    VADriverContextP  ctx,
    VAConfigID        configId,
    int32_t           pictureWidth,
    int32_t           pictureHeight,
    int32_t           flag,
    VASurfaceID       *renderTargets,
    int32_t           renderTargetsNum,
    VAContextID       *context)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_DDI);

    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(ctx->pDriverData, "nullptr ctx->pDriverData", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx->m_capsNext, "nullptr m_capsNext", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaCtx->m_capsNext->m_capsTable, "nullptr m_capsTable", VA_STATUS_ERROR_INVALID_CONTEXT);

    ConfigLinux*  configItem = nullptr;
    configItem = mediaCtx->m_capsNext->m_capsTable->QueryConfigItemFromIndex(configId);
    DDI_CODEC_CHK_NULL(configItem, "Invalid config id!", VA_STATUS_ERROR_INVALID_PARAMETER);

    if (renderTargetsNum > DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    encode::DdiEncodeBase *ddiEncode = DdiEncodeFactory::Create(ComponentInfo{configItem->profile, configItem->entrypoint});
    DDI_CODEC_CHK_NULL(ddiEncode, "nullptr ddiEncode", VA_STATUS_ERROR_UNIMPLEMENTED);
    VAStatus vaStatus = ddiEncode->CheckEncodeResolution(
        mediaCtx->m_capsNext,
        configId,
        pictureWidth,
        pictureHeight);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
    }

    // first create encoder context
    ddiEncode->m_encodeCtx = (decltype(ddiEncode->m_encodeCtx))MOS_AllocAndZeroMemory(sizeof(encode::DDI_ENCODE_CONTEXT));
    DDI_CODEC_CHK_NULL(ddiEncode->m_encodeCtx, "nullptr ddiEncode->m_encodeCtx", VA_STATUS_ERROR_ALLOCATION_FAILED);
    DDI_CODEC_VERBOSEMESSAGE(" DdiEncodeFunctions::CreateContext encCtx %p ddiEncode %p", ddiEncode->m_encodeCtx, ddiEncode);

    encode::PDDI_ENCODE_CONTEXT encCtx = ddiEncode->m_encodeCtx;
    encCtx->m_encode           = ddiEncode;

    //initialize DDI level cp interface
    MOS_CONTEXT mosCtx      = {};
    encCtx->pCpDdiInterfaceNext = CreateDdiCpNext(&mosCtx);
    if (nullptr == encCtx->pCpDdiInterfaceNext)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        CleanUp(encCtx);
        return vaStatus;
    }

    // Get the buf manager for codechal create
    mosCtx.bufmgr          = mediaCtx->pDrmBufMgr;
    mosCtx.fd              = mediaCtx->fd;
    mosCtx.iDeviceId       = mediaCtx->iDeviceId;
    mosCtx.m_skuTable      = mediaCtx->SkuTable;
    mosCtx.m_waTable       = mediaCtx->WaTable;
    mosCtx.m_gtSystemInfo  = *mediaCtx->pGtSystemInfo;
    mosCtx.m_platform      = mediaCtx->platform;

    mosCtx.ppMediaMemDecompState = &mediaCtx->pMediaMemDecompState;
    mosCtx.pfnMemoryDecompress   = mediaCtx->pfnMemoryDecompress;
    mosCtx.pfnMediaMemoryCopy    = mediaCtx->pfnMediaMemoryCopy;
    mosCtx.pfnMediaMemoryCopy2D  = mediaCtx->pfnMediaMemoryCopy2D;
    mosCtx.ppMediaCopyState      = &mediaCtx->pMediaCopyState;
    mosCtx.m_gtSystemInfo        = *mediaCtx->pGtSystemInfo;
    mosCtx.m_auxTableMgr         = mediaCtx->m_auxTableMgr;
    mosCtx.pGmmClientContext     = mediaCtx->pGmmClientContext;

    mosCtx.m_osDeviceContext = mediaCtx->m_osDeviceContext;
    mosCtx.m_apoMosEnabled   = true;
    mosCtx.m_userSettingPtr = mediaCtx->m_userSettingPtr;

    mosCtx.pPerfData = (PERF_DATA *)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));
    if (nullptr == mosCtx.pPerfData)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        CleanUp(encCtx);
        return vaStatus;
    }

    if (configItem->entrypoint == VAEntrypointEncSlice)
    {
        encCtx->bVdencActive = true;
    }

    encCtx->vaEntrypoint  = configItem->entrypoint;
    encCtx->vaProfile     = configItem->profile;
    encCtx->uiRCMethod    = configItem->componentData.data.rcMode;
    encCtx->wModeType     = ddiEncode->GetEncodeCodecMode(configItem->profile, configItem->entrypoint);
    encCtx->codecFunction = ddiEncode->GetEncodeCodecFunction(configItem->profile, configItem->entrypoint, encCtx->bVdencActive);

    CODECHAL_STANDARD_INFO standardInfo;
    MOS_ZeroMemory(&standardInfo, sizeof(CODECHAL_STANDARD_INFO));
    standardInfo.CodecFunction = encCtx->codecFunction;
    standardInfo.Mode          = encCtx->wModeType;
    Codechal *pCodecHal        = CodechalDeviceNext::CreateFactory(
        nullptr,
        &mosCtx,
        &standardInfo,
        nullptr);
    if (pCodecHal == nullptr)
    {
        // add anything necessary here to free the resource
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        CleanUp(encCtx);
        return vaStatus;
    }

    encCtx->pCodecHal = pCodecHal;

    // Setup some initial data
    encCtx->dworiFrameWidth  = pictureWidth;
    encCtx->dworiFrameHeight = pictureHeight;
    encCtx->wPicWidthInMB    = (uint16_t)(DDI_CODEC_NUM_MACROBLOCKS_WIDTH(pictureWidth));
    encCtx->wPicHeightInMB   = (uint16_t)(DDI_CODEC_NUM_MACROBLOCKS_HEIGHT(pictureHeight));
    encCtx->dwFrameWidth     = encCtx->wPicWidthInMB * CODECHAL_MACROBLOCK_WIDTH;
    encCtx->dwFrameHeight    = encCtx->wPicHeightInMB * CODECHAL_MACROBLOCK_HEIGHT;
    //recoder old resolution for dynamic resolution  change
    encCtx->wContextPicWidthInMB  = encCtx->wPicWidthInMB;
    encCtx->wContextPicHeightInMB = encCtx->wPicHeightInMB;
    encCtx->wOriPicWidthInMB      = encCtx->wPicWidthInMB;
    encCtx->wOriPicHeightInMB     = encCtx->wPicHeightInMB;
    encCtx->targetUsage           = TARGETUSAGE_RT_SPEED;
    // Attach PMEDIDA_DRIVER_CONTEXT
    encCtx->pMediaCtx = mediaCtx;

    encCtx->pCpDdiInterfaceNext->SetCpFlags(flag);
    encCtx->pCpDdiInterfaceNext->SetCpParams(CP_TYPE_NONE, encCtx->m_encode->m_codechalSettings);

    vaStatus = encCtx->m_encode->ContextInitialize(encCtx->m_encode->m_codechalSettings);

    if (vaStatus != VA_STATUS_SUCCESS)
    {
        CleanUp(encCtx);
        return vaStatus;
    }

    MOS_STATUS eStatus = pCodecHal->Allocate(encCtx->m_encode->m_codechalSettings);

#ifdef _MMC_SUPPORTED
    PMOS_INTERFACE osInterface = pCodecHal->GetOsInterface();
    if (osInterface != nullptr &&
        !osInterface->apoMosEnabled &&
        MEDIA_IS_SKU(osInterface->pfnGetSkuTable(osInterface), FtrMemoryCompression) &&
        !mediaCtx->pMediaMemDecompState)
    {
        mediaCtx->pMediaMemDecompState =
            static_cast<MediaMemDecompState *>(MmdDevice::CreateFactory(&mosCtx));
    }
#endif

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        CleanUp(encCtx);
        return vaStatus;
    }

    vaStatus = encCtx->m_encode->InitCompBuffer();
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        CleanUp(encCtx);
        return vaStatus;
    }

    // register the render target surfaces for this encoder instance
    // This is a must as driver has the constraint, 127 surfaces per context
    for (int32_t i = 0; i < renderTargetsNum; i++)
    {
        DDI_MEDIA_SURFACE *surface;

        surface = MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, renderTargets[i]);
        if (nullptr == surface)
        {
            DDI_CODEC_ASSERTMESSAGE("DDI: invalid render target %d in vpgEncodeCreateContext.", i);
            vaStatus = VA_STATUS_ERROR_INVALID_SURFACE;
            CleanUp(encCtx);
            return vaStatus;
        }
        encCtx->RTtbl.pRT[i] = surface;
        encCtx->RTtbl.iNumRenderTargets++;
    }

    // convert PDDI_ENCODE_CONTEXT to VAContextID
    MosUtilities::MosLockMutex(&mediaCtx->EncoderMutex);
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT vaContextHeapElmt = MediaLibvaUtilNext::DdiAllocPVAContextFromHeap(mediaCtx->pEncoderCtxHeap);
    if (nullptr == vaContextHeapElmt)
    {
        MosUtilities::MosUnlockMutex(&mediaCtx->EncoderMutex);
        vaStatus = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        CleanUp(encCtx);
        return vaStatus;
    }

    vaContextHeapElmt->pVaContext = (void *)encCtx;
    mediaCtx->uiNumEncoders++;
    *context = (VAContextID)(vaContextHeapElmt->uiVaContextID + DDI_MEDIA_SOFTLET_VACONTEXTID_ENCODER_OFFSET);
    MosUtilities::MosUnlockMutex(&mediaCtx->EncoderMutex);
    DDI_CODEC_VERBOSEMESSAGE(" DdiEncodeFunctions::CreateContext ctx %p  *context %d", ctx, *context);
    return vaStatus;
}

VAStatus DdiEncodeFunctions::DestroyContext (
    VADriverContextP  ctx,
    VAContextID       context)
{
    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(ctx->pDriverData, "nullptr ctx->pDriverData", VA_STATUS_ERROR_INVALID_CONTEXT);

    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr mediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    uint32_t        ctxType                   = 0;
    encode::PDDI_ENCODE_CONTEXT encCtx  = (decltype(encCtx))MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->pCodecHal, "nullptr encCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

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

    if (codecHal->GetOsInterface() && codecHal->GetOsInterface()->pOsContext)
    {
        MOS_FreeMemory(codecHal->GetOsInterface()->pOsContext->pPerfData);
        codecHal->GetOsInterface()->pOsContext->pPerfData = nullptr;
    }

    // destroy codechal
    codecHal->Destroy();
    MOS_Delete(codecHal);

    if (encCtx->pCpDdiInterfaceNext)
    {
        MOS_Delete(encCtx->pCpDdiInterfaceNext);
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

    MosUtilities::MosLockMutex(&mediaCtx->EncoderMutex);
    MediaLibvaUtilNext::DdiReleasePVAContextFromHeap(mediaCtx->pEncoderCtxHeap, encIndex);
    mediaCtx->uiNumEncoders--;
    MosUtilities::MosUnlockMutex(&mediaCtx->EncoderMutex);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::CreateBuffer (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferType      type,
    uint32_t          size,
    uint32_t          elementsNum,
    void              *data,
    VABufferID        *bufId)
{
    DDI_CODEC_CHK_NULL(ctx, "nullptr context!", VA_STATUS_ERROR_INVALID_CONTEXT);

    uint32_t        ctxType                   = 0;
    encode::PDDI_ENCODE_CONTEXT encCtx = (decltype(encCtx))MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = encCtx->m_encode->CreateBuffer(ctx, type, size, elementsNum, data, bufId);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::MapBufferInternal(
    DDI_MEDIA_CONTEXT   *mediaCtx,
    VABufferID          buf_id,
    void              **pbuf,
    uint32_t            flag
)
{
    DDI_CODEC_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_START, &buf_id, sizeof(buf_id), &flag, sizeof(flag));

    DDI_CODEC_CHK_NULL(pbuf,  "nullptr pbuf",    VA_STATUS_ERROR_INVALID_PARAMETER);

    DDI_CODEC_CHK_NULL(mediaCtx,              "nullptr mediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CODEC_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_LESS((uint32_t)buf_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufferId", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_BUFFER   *buf     = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx, buf_id);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    // The context is nullptr when the buffer is created from DeriveImage
    // So doesn't need to check the context for all cases
    // Only check the context in dec/enc mode
    VAStatus                 vaStatus  = VA_STATUS_SUCCESS;
    uint32_t                 ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, buf_id);
    void                     *ctxPtr = nullptr;
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = nullptr;
    encode::PDDI_ENCODE_CONTEXT      encCtx  = nullptr;

    ctxPtr = MediaLibvaCommonNext::GetCtxFromVABufferID(mediaCtx, buf_id);
    DDI_CODEC_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);

    encCtx = encode::GetEncContextFromPVOID(ctxPtr);
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    bufMgr = &(encCtx->BufMgr);

    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_INFO, &ctxType, sizeof(ctxType), &buf->uiType, sizeof(uint32_t));
    switch ((int32_t)buf->uiType)
    {
        case VAEncCodedBufferType:
            DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

            if( CodedBufferExistInStatusReport( encCtx, buf ) )
            {
                vaStatus = StatusReport(encCtx, buf, pbuf);
                MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                return vaStatus;
            }
            // so far a coded buffer that has NOT been added into status report is skipped frame in non-CP case
            // but this can change in future if new usage models come up
            encCtx->BufMgr.pCodedBufferSegment->buf  = MediaLibvaUtilNext::LockBuffer(buf, flag);
            encCtx->BufMgr.pCodedBufferSegment->size = buf->iSize;
            *pbuf =  encCtx->BufMgr.pCodedBufferSegment;

            break;

        case VAStatsStatisticsBufferType:
        case VAStatsStatisticsBottomFieldBufferType:
        case VAStatsMVBufferType:
            {
                DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT)
                encode::DDI_ENCODE_PRE_ENC_BUFFER_TYPE idx = (buf->uiType == VAStatsMVBufferType) ? encode::PRE_ENC_BUFFER_TYPE_MVDATA :
                                                    ((buf->uiType == VAStatsStatisticsBufferType)   ? encode::PRE_ENC_BUFFER_TYPE_STATS
                                                                                                          : encode::PRE_ENC_BUFFER_TYPE_STATS_BOT);
                if((encCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC) && PreEncBufferExistInStatusReport( encCtx, buf, idx))
                {
                    vaStatus = PreEncStatusReport(encCtx, buf, pbuf);
                    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                    return vaStatus;
                }
                if(buf->bo)
                {
                    *pbuf = MediaLibvaUtilNext::LockBuffer(buf, flag);
                }
                break;
            }
        case VAStatsMVPredictorBufferType:
        case VAEncFEIMBControlBufferType:
        case VAEncFEIMVPredictorBufferType:
        case VAEncQPBufferType:
            if(buf->bo)
            {
                *pbuf = MediaLibvaUtilNext::LockBuffer(buf, flag);
            }
            break;
        case VAEncFEIMVBufferType:
        case VAEncFEIMBCodeBufferType:
        case VAEncFEICURecordBufferType:
        case VAEncFEICTBCmdBufferType:
        case VAEncFEIDistortionBufferType:
            {
                DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT)
                if(encCtx->wModeType == CODECHAL_ENCODE_MODE_AVC)
                {
                    CodecEncodeAvcFeiPicParams *feiPicParams = (CodecEncodeAvcFeiPicParams *)encCtx->pFeiPicParams;

                    encode::DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (buf->uiType == VAEncFEIMVBufferType) ? encode::FEI_ENC_BUFFER_TYPE_MVDATA :
                                       ((buf->uiType == VAEncFEIMBCodeBufferType) ? encode::FEI_ENC_BUFFER_TYPE_MBCODE :
                                                                                        encode::FEI_ENC_BUFFER_TYPE_DISTORTION);
                    if((feiPicParams != nullptr) && (encCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC) && EncBufferExistInStatusReport(encCtx, buf, idx))
                    {
                        vaStatus = EncStatusReport(encCtx, buf, pbuf);
                        MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                        return vaStatus;
                    }
                }
                else if(encCtx->wModeType == CODECHAL_ENCODE_MODE_HEVC)

                {
                    CodecEncodeHevcFeiPicParams *feiPicParams = (CodecEncodeHevcFeiPicParams *)encCtx->pFeiPicParams;
                    encode::DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (buf->uiType == VAEncFEICTBCmdBufferType) ? encode::FEI_ENC_BUFFER_TYPE_MVDATA   :
                                                      ((buf->uiType == VAEncFEICURecordBufferType) ? encode::FEI_ENC_BUFFER_TYPE_MBCODE :
                                                                                                      encode::FEI_ENC_BUFFER_TYPE_DISTORTION);
                    if((feiPicParams != nullptr) && (encCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC) && EncBufferExistInStatusReport( encCtx, buf, idx))
                    {
                        vaStatus = EncStatusReport(encCtx, buf, pbuf);
                        MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                        return vaStatus;
                    }
                }
                if(buf->bo)
                {
                    *pbuf = MediaLibvaUtilNext::LockBuffer(buf, flag);
                }
            }
            break;
        case VAStatsStatisticsParameterBufferType:
            *pbuf = (void *)(buf->pData + buf->uiOffset);
            break;
        case VAEncMacroblockMapBufferType:
            MosUtilities::MosLockMutex(&mediaCtx->BufferMutex);
            *pbuf = MediaLibvaUtilNext::LockBuffer(buf, flag);
            MosUtilities::MosUnlockMutex(&mediaCtx->BufferMutex);
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
                *pbuf = MediaLibvaUtilNext::LockBuffer(buf, flag);
            }
            break;

        case VAImageBufferType:
        default:
            vaStatus = DdiMediaFunctions::MapBufferInternal(mediaCtx, buf_id, pbuf, MOS_LOCKFLAG_READONLY | MOS_LOCKFLAG_WRITEONLY);
            break;
    }

    MOS_TraceEventExt(EVENT_VA_MAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return vaStatus;
}

VAStatus DdiEncodeFunctions::UnmapBuffer (
    DDI_MEDIA_CONTEXT   *mediaCtx,
    VABufferID          buf_id
)
{
    DDI_CODEC_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_UNMAP, EVENT_TYPE_START, &buf_id, sizeof(buf_id), nullptr, 0);

    DDI_CODEC_CHK_NULL(mediaCtx,               "nullptr mediaCtx",               VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CODEC_CHK_NULL( mediaCtx->pBufferHeap, "nullptr  mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_LESS((uint32_t)buf_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid buf_id", VA_STATUS_ERROR_INVALID_BUFFER);

    DDI_MEDIA_BUFFER   *buf     = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx,  buf_id);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    // The context is nullptr when the buffer is created from DeriveImage
    // So doesn't need to check the context for all cases
    // Only check the context in enc mode
    void     *ctxPtr = nullptr;
    uint32_t ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, buf_id);
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = nullptr;
    encode::PDDI_ENCODE_CONTEXT encCtx = nullptr;

    ctxPtr = MediaLibvaCommonNext::GetCtxFromVABufferID(mediaCtx, buf_id);
    DDI_CODEC_CHK_NULL(ctxPtr, "nullptr ctxPtr", VA_STATUS_ERROR_INVALID_CONTEXT);

    encCtx = encode::GetEncContextFromPVOID(ctxPtr);
    bufMgr = &(encCtx->BufMgr);

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
        case VAEncMacroblockDisableSkipMapBufferType:
            if(buf->bo)
            {
                MediaLibvaUtilNext::UnlockBuffer(buf);
            }
            break;

        default:
            DdiMediaFunctions::UnmapBuffer(mediaCtx, buf_id);
            break;
    }

    MOS_TraceEventExt(EVENT_VA_UNMAP, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::DestroyBuffer (
    DDI_MEDIA_CONTEXT  *mediaCtx,
    VABufferID          buffer_id
)
{
    DDI_CODEC_FUNC_ENTER;
    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_START, &buffer_id, sizeof(buffer_id), nullptr, 0);

    DDI_CODEC_CHK_NULL(mediaCtx,              "nullptr mediaCtx",              VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_CODEC_CHK_NULL(mediaCtx->pBufferHeap, "nullptr mediaCtx->pBufferHeap", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_LESS((uint32_t)buffer_id, mediaCtx->pBufferHeap->uiAllocatedHeapElements, "Invalid bufferId", VA_STATUS_ERROR_INVALID_CONTEXT);

    DDI_MEDIA_BUFFER   *buf     = MediaLibvaCommonNext::GetBufferFromVABufferID(mediaCtx,  buffer_id);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_BUFFER);

    void     *ctxPtr = MediaLibvaCommonNext::GetCtxFromVABufferID(mediaCtx,     buffer_id);
    uint32_t ctxType = MediaLibvaCommonNext::GetCtxTypeFromVABufferID(mediaCtx, buffer_id);

    DDI_CODEC_COM_BUFFER_MGR *bufMgr  = nullptr;
    encode::PDDI_ENCODE_CONTEXT encCtx  = nullptr;

    encCtx = encode::GetEncContextFromPVOID(ctxPtr);
    bufMgr = &(encCtx->BufMgr);

    switch ((int32_t)buf->uiType)
    {
        case VAImageBufferType:
            if(buf->format == Media_Format_CPU)
            {
                MOS_DeleteArray(buf->pData);
            }
            else
            {
                MediaLibvaUtilNext::UnRefBufObjInMediaBuffer(buf);

                if (buf->uiExportcount) {
                    buf->bPostponedBufFree = true;
                    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
                    return VA_STATUS_SUCCESS;
                }
            }
            break;
        case VAIQMatrixBufferType:
        case VAHuffmanTableBufferType:
        case VAEncSliceParameterBufferType:
        case VAEncPictureParameterBufferType:
        case VAEncSequenceParameterBufferType:
        case VAEncPackedHeaderDataBufferType:
        case VAEncPackedHeaderParameterBufferType:
            MOS_DeleteArray(buf->pData);
            break;
        case VAEncMacroblockMapBufferType:
            MediaLibvaUtilNext::FreeBuffer(buf);
            break;
#ifdef ENABLE_ENC_UNLIMITED_OUTPUT
        case VAEncCodedBufferType:
            if(nullptr == encCtx)
            {
                encCtx = encode::GetEncContextFromPVOID(ctxPtr);
                if(nullptr == encCtx)
                    return VA_STATUS_ERROR_INVALID_CONTEXT;
            }
            MediaLibvaUtilNext::FreeBuffer(buf);
            break;
#endif
        case VAStatsStatisticsParameterBufferType:
            MOS_DeleteArray(buf->pData);
            break;
        case VAStatsStatisticsBufferType:
        case VAStatsStatisticsBottomFieldBufferType:
        case VAStatsMVBufferType:
            {
                if(nullptr == encCtx)
                {
                    encCtx = encode::GetEncContextFromPVOID(ctxPtr);
                    if(nullptr == encCtx)
                        return VA_STATUS_ERROR_INVALID_CONTEXT;
                }
                if(encCtx->codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
                {
                    encode::DDI_ENCODE_PRE_ENC_BUFFER_TYPE idx = (buf->uiType == VAStatsMVBufferType) ? encode::PRE_ENC_BUFFER_TYPE_MVDATA :
                                                        ((buf->uiType == VAStatsStatisticsBufferType)   ? encode::PRE_ENC_BUFFER_TYPE_STATS
                                                                                                              : encode::PRE_ENC_BUFFER_TYPE_STATS_BOT);
                    RemoveFromPreEncStatusReportQueue(encCtx, buf, idx);
                }
            }
            MediaLibvaUtilNext::FreeBuffer(buf);
            break;
        case VAStatsMVPredictorBufferType:
        case VAEncFEIMBControlBufferType:
        case VAEncFEIMVPredictorBufferType:
        case VAEncQPBufferType:
        case VADecodeStreamoutBufferType:
            MediaLibvaUtilNext::FreeBuffer(buf);
            break;
        case VAEncFEIMVBufferType:
        case VAEncFEIMBCodeBufferType:
        case VAEncFEICURecordBufferType:
        case VAEncFEICTBCmdBufferType:
        case VAEncFEIDistortionBufferType:
            {
                if(nullptr == encCtx)
                {
                    encCtx = encode::GetEncContextFromPVOID(ctxPtr);
                    if(nullptr == encCtx)
                        return VA_STATUS_ERROR_INVALID_CONTEXT;
                }
                if(encCtx->wModeType == CODECHAL_ENCODE_MODE_AVC)
                {
                    CodecEncodeAvcFeiPicParams *feiPicParams;
                    feiPicParams = (CodecEncodeAvcFeiPicParams *)(encCtx->pFeiPicParams);
                    if((feiPicParams != nullptr) && (encCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC))
                    {
                        encode::DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (buf->uiType == VAEncFEIMVBufferType) ? encode::FEI_ENC_BUFFER_TYPE_MVDATA :
                                                        ((buf->uiType == VAEncFEIMBCodeBufferType) ? encode::FEI_ENC_BUFFER_TYPE_MBCODE :
                                                                                                      encode::FEI_ENC_BUFFER_TYPE_DISTORTION);
                        RemoveFromEncStatusReportQueue(encCtx, buf, idx);
                    }

                }
                else if(encCtx->wModeType == CODECHAL_ENCODE_MODE_HEVC)
                {
                    CodecEncodeHevcFeiPicParams *feiPicParams;
                    feiPicParams = (CodecEncodeHevcFeiPicParams *)(encCtx->pFeiPicParams);
                    if((feiPicParams != nullptr) && (encCtx->codecFunction == CODECHAL_FUNCTION_FEI_ENC))
                    {
                        encode::DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx = (buf->uiType == VAEncFEICTBCmdBufferType) ? encode::FEI_ENC_BUFFER_TYPE_MVDATA   :
                                                          ((buf->uiType == VAEncFEICURecordBufferType) ? encode::FEI_ENC_BUFFER_TYPE_MBCODE :
                                                                                                          encode::FEI_ENC_BUFFER_TYPE_DISTORTION);
                        RemoveFromEncStatusReportQueue(encCtx, buf, idx);
                    }
                }
            }
            MediaLibvaUtilNext::FreeBuffer(buf);
            break;
        default: // do not handle any un-listed buffer type
            MOS_DeleteArray(buf->pData);
            break;
            //return va_STATUS_SUCCESS;
    }
    MOS_Delete(buf);

    MediaLibvaInterfaceNext::DestroyBufFromVABufferID(mediaCtx, buffer_id);
    MOS_TraceEventExt(EVENT_VA_FREE_BUFFER, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiEncodeFunctions::BeginPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       renderTarget)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_DDI);
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_CHK_NULL(ctx, "nullptr context in vpgEncodeBeginPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    uint32_t        ctxType                   = 0;
    encode::PDDI_ENCODE_CONTEXT encCtx = (decltype(encCtx))MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = encCtx->m_encode->BeginPicture(ctx, context, renderTarget);
    return vaStatus;
}

VAStatus DdiEncodeFunctions::RenderPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           buffersNum)
{
    VAStatus        vaStatus                  = VA_STATUS_SUCCESS;
    int32_t         numOfBuffers              = buffersNum;
    int32_t         priority                  = 0;
    int32_t         priorityIndexInBuffers    = -1;
    bool            updatePriority            = false;
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_DDI);
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_CHK_NULL(ctx, "nullptr context in vpgEncodeRenderPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    uint32_t        ctxType                   = 0;
    encode::PDDI_ENCODE_CONTEXT encCtx = (decltype(encCtx))MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    priorityIndexInBuffers = MediaLibvaCommonNext::GetGpuPriority(ctx, buffers, numOfBuffers, &updatePriority, &priority);
    if (priorityIndexInBuffers != -1)
    {
        if(updatePriority)
        {
            vaStatus = SetGpuPriority(encCtx, priority);
            if(vaStatus != VA_STATUS_SUCCESS)
                return vaStatus;
        }
        MediaLibvaCommonNext::MovePriorityBufferIdToEnd(buffers, priorityIndexInBuffers, numOfBuffers);
        numOfBuffers--;
    }
    if (numOfBuffers == 0)
        return vaStatus;

    vaStatus = encCtx->m_encode->RenderPicture(ctx, context, buffers, numOfBuffers);
    return vaStatus;
}

VAStatus DdiEncodeFunctions::EndPicture (
    VADriverContextP  ctx,
    VAContextID       context)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_DDI);

    DDI_CODEC_FUNC_ENTER;
    DDI_CODEC_CHK_NULL(ctx, "nullptr context in vpgEncodeEndPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is encoder ID
    uint32_t        ctxType                   = 0;
    encode::PDDI_ENCODE_CONTEXT encCtx = (decltype(encCtx))MediaLibvaCommonNext::GetContextFromContextID(ctx, context, &ctxType);
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);

    VAStatus vaStatus = encCtx->m_encode->EndPicture(ctx, context);
    return vaStatus;
}

//!
//! \brief  Clean and free encode context structure
//!
//! \param  [in] encCtx
//!     Pointer to ddi encode context
//!
void DdiEncodeFunctions::CleanUp(encode::PDDI_ENCODE_CONTEXT encCtx)
{
    if (encCtx->m_encode)
    {
        MOS_Delete(encCtx->m_encode);
        encCtx->m_encode = nullptr;
    }

    if (encCtx->pCpDdiInterfaceNext)
    {
        MOS_Delete(encCtx->pCpDdiInterfaceNext);
    }

    MOS_FreeMemory(encCtx);
    encCtx = nullptr;

    return;
}

VAStatus DdiEncodeFunctions::SetGpuPriority(
    encode::PDDI_ENCODE_CONTEXT encCtx,
    int32_t             priority
)
{
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    if(encCtx->pCodecHal != nullptr)
    {
        PMOS_INTERFACE osInterface = encCtx->pCodecHal->GetOsInterface();
        DDI_CODEC_CHK_NULL(osInterface, "nullptr osInterface.", VA_STATUS_ERROR_ALLOCATION_FAILED);

        //Set Gpu priority for encoder
        osInterface->pfnSetGpuPriority(osInterface, priority);
    }

    return VA_STATUS_SUCCESS;
}

bool DdiEncodeFunctions::CodedBufferExistInStatusReport(
    encode::PDDI_ENCODE_CONTEXT encCtx,
    PDDI_MEDIA_BUFFER   buf)
{
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    return encCtx->m_encode->CodedBufferExistInStatusReport(buf);
}

VAStatus DdiEncodeFunctions::StatusReport (
        encode::PDDI_ENCODE_CONTEXT encCtx,
        DDI_MEDIA_BUFFER    *mediaBuf,
        void                **buf
    )
{
   PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_DDI);

    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaBuf, "nullptr mediaBuf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->StatusReport(mediaBuf, buf);

    return vaStatus;
}

bool DdiEncodeFunctions::PreEncBufferExistInStatusReport(
    encode::PDDI_ENCODE_CONTEXT            encCtx,
    PDDI_MEDIA_BUFFER              buf,
    encode:: DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    return encCtx->m_encode->PreEncBufferExistInStatusReport(buf, typeIdx);
}

VAStatus DdiEncodeFunctions::PreEncStatusReport(
    encode::PDDI_ENCODE_CONTEXT encCtx,
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **buf)
{
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaBuf, "nullptr mediaBuf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->PreEncStatusReport(mediaBuf, buf);

    return vaStatus;
}

bool DdiEncodeFunctions::EncBufferExistInStatusReport(
    encode::PDDI_ENCODE_CONTEXT            encCtx,
    PDDI_MEDIA_BUFFER              buf,
    encode::DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    return encCtx->m_encode->EncBufferExistInStatusReport(buf, typeIdx);
}

VAStatus DdiEncodeFunctions::EncStatusReport(
    encode::PDDI_ENCODE_CONTEXT encCtx,
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **buf)
{
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(mediaBuf, "nullptr mediaBuf", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->EncStatusReport(mediaBuf, buf);

    return vaStatus;
}

VAStatus DdiEncodeFunctions::RemoveFromPreEncStatusReportQueue(
    encode::PDDI_ENCODE_CONTEXT            encCtx,
    PDDI_MEDIA_BUFFER              buf,
    encode::DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->RemoveFromPreEncStatusReportQueue(buf, typeIdx);

    return vaStatus;
}

VAStatus DdiEncodeFunctions::RemoveFromEncStatusReportQueue(
    encode::PDDI_ENCODE_CONTEXT            encCtx,
    PDDI_MEDIA_BUFFER              buf,
    encode::DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx)
{
    DDI_CODEC_CHK_NULL(encCtx, "nullptr encCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(encCtx->m_encode, "nullptr encCtx->m_encode", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(buf, "nullptr buf", VA_STATUS_ERROR_INVALID_PARAMETER);

    VAStatus vaStatus = encCtx->m_encode->RemoveFromEncStatusReportQueue(buf, typeIdx);

    return vaStatus;
}
