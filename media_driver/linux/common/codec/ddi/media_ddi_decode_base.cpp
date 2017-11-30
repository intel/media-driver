/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     media_ddi_decode_base.cpp
//! \brief    The class implementation of DdiDecodeBase  for all decoders
//!

#include "media_libva_decoder.h"
#include "media_libva_util.h"
#include "media_ddi_decode_base.h"
#include "codechal.h"
#include "codechal_memdecomp.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"

DdiMediaDecode::DdiMediaDecode(DDI_DECODE_CONFIG_ATTR *ddiDecodeAttr)
    : DdiMediaBase()
{
    m_ddiDecodeAttr = ddiDecodeAttr;
    m_ctxType       = DDI_MEDIA_CONTEXT_TYPE_DECODER;
    m_ddiDecodeCtx  = nullptr;
}

VAStatus DdiMediaDecode::BasicInit(
    DDI_DECODE_CONFIG_ATTR *ddiConfAttr)
{
    if (ddiConfAttr == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    m_ddiDecodeAttr = (DDI_DECODE_CONFIG_ATTR *)MOS_AllocAndZeroMemory(
        sizeof(DDI_DECODE_CONFIG_ATTR));
    if (m_ddiDecodeAttr && ddiConfAttr)
    {
        memcpy(m_ddiDecodeAttr, ddiConfAttr, sizeof(DDI_DECODE_CONFIG_ATTR));
    }

    m_ddiDecodeCtx = (DDI_DECODE_CONTEXT *)MOS_AllocAndZeroMemory(
        sizeof(DDI_DECODE_CONTEXT));

    if ((m_ddiDecodeAttr == nullptr) ||
        (m_ddiDecodeCtx == nullptr))
    {
        MOS_FreeMemory(m_ddiDecodeAttr);
        m_ddiDecodeAttr = nullptr;
        MOS_FreeMemory(m_ddiDecodeCtx);
        m_ddiDecodeCtx = nullptr;
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    return VA_STATUS_SUCCESS;
}

uint32_t DdiMediaDecode::GetBsBufOffset(int32_t sliceGroup)
{
    return m_ddiDecodeCtx->BufMgr.pSliceData[sliceGroup].uiOffset;
}

VAStatus DdiMediaDecode::ParseProcessingBuffer(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *bufAddr)
{
#ifdef _DECODE_PROCESSING_SUPPORTED
    VAProcPipelineParameterBuffer *procBuf =
        (VAProcPipelineParameterBuffer *)bufAddr;

    DDI_CHK_NULL(procBuf, "Null Processing buffer", VA_STATUS_ERROR_INVALID_PARAMETER)

    if (m_ddiDecodeCtx->uiDecProcessingType == VA_DEC_PROCESSING)
    {
        PCODECHAL_DECODE_PROCESSING_PARAMS decProcessingParams;
        PMOS_SURFACE                       decProcessingSurface;

        decProcessingParams = m_ddiDecodeCtx->DecodeParams.m_procParams;

        decProcessingSurface = decProcessingParams->pOutputSurface;

        memset(decProcessingSurface, 0, sizeof(MOS_SURFACE));

        PDDI_MEDIA_SURFACE surface =
            DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, procBuf->additional_outputs[0]);

        DDI_CHK_NULL(surface, "Null surface in Processing buffer", VA_STATUS_ERROR_INVALID_PARAMETER)

        DdiMedia_MediaSurfaceToMosResource(surface, &(decProcessingSurface->OsResource));

        decProcessingSurface->dwWidth  = decProcessingSurface->OsResource.iWidth;
        decProcessingSurface->dwHeight = decProcessingSurface->OsResource.iHeight;
        decProcessingSurface->dwPitch  = decProcessingSurface->OsResource.iPitch;
        decProcessingSurface->TileType = decProcessingSurface->OsResource.TileType;
        decProcessingSurface->Format   = decProcessingSurface->OsResource.Format;

        decProcessingParams->rcInputSurfaceRegion.X      = procBuf->surface_region->x;
        decProcessingParams->rcInputSurfaceRegion.Y      = procBuf->surface_region->y;
        decProcessingParams->rcInputSurfaceRegion.Width  = procBuf->surface_region->width;
        decProcessingParams->rcInputSurfaceRegion.Height = procBuf->surface_region->height;

        decProcessingParams->pOutputSurface               = decProcessingSurface;
        decProcessingParams->rcOutputSurfaceRegion.X      = procBuf->output_region->x;
        decProcessingParams->rcOutputSurfaceRegion.Y      = procBuf->output_region->y;
        decProcessingParams->rcOutputSurfaceRegion.Width  = procBuf->output_region->width;
        decProcessingParams->rcOutputSurfaceRegion.Height = procBuf->output_region->height;

        // Chroma siting
        // Set the vertical chroma siting info
        uint32_t chromaSitingFlags;
        chromaSitingFlags                       = procBuf->input_color_properties.chroma_sample_location & 0x3;
        decProcessingParams->uiChromaSitingType = CODECHAL_CHROMA_SITING_NONE;
        decProcessingParams->uiRotationState    = 0;
        decProcessingParams->uiBlendState       = 0;
        decProcessingParams->uiMirrorState      = 0;

        switch (chromaSitingFlags)
        {
        case VA_CHROMA_SITING_VERTICAL_TOP:
            decProcessingParams->uiChromaSitingType = CODECHAL_CHROMA_SITING_VERT_TOP;
            break;
        case VA_CHROMA_SITING_VERTICAL_CENTER:
            decProcessingParams->uiChromaSitingType = CODECHAL_CHROMA_SITING_VERT_CENTER;
            break;
        case VA_CHROMA_SITING_VERTICAL_BOTTOM:
            decProcessingParams->uiChromaSitingType = CODECHAL_CHROMA_SITING_VERT_BOTTOM;
            break;
        default:
            decProcessingParams->uiChromaSitingType = CODECHAL_CHROMA_SITING_NONE;
            break;
        }

        if (decProcessingParams->uiChromaSitingType != CODECHAL_CHROMA_SITING_NONE)
        {
            // Set the horizontal chroma siting info
            chromaSitingFlags = procBuf->input_color_properties.chroma_sample_location & 0xc;

            switch (chromaSitingFlags)
            {
            case VA_CHROMA_SITING_HORIZONTAL_LEFT:
                decProcessingParams->uiChromaSitingType |= CODECHAL_CHROMA_SITING_HORZ_LEFT;
                break;
            case VA_CHROMA_SITING_HORIZONTAL_CENTER:
                decProcessingParams->uiChromaSitingType |= CODECHAL_CHROMA_SITING_HORZ_CENTER;
                break;
            default:
                decProcessingParams->uiChromaSitingType = CODECHAL_CHROMA_SITING_NONE;
                break;
            }
        }
    }

    return VA_STATUS_SUCCESS;
#else
    return VA_STATUS_ERROR_INVALID_PARAMETER;
#endif
}

VAStatus DdiMediaDecode::BeginPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VASurfaceID      renderTarget)
{
    DDI_FUNCTION_ENTER();;

    PDDI_MEDIA_CONTEXT mediaCtx;

    /* As it is already checked in the upper caller, skip the check */
    mediaCtx = DdiMedia_GetMediaContext(ctx);

    DDI_MEDIA_SURFACE *curRT;
    curRT = (DDI_MEDIA_SURFACE *)DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, renderTarget);
    DDI_CHK_NULL(curRT, "Null pCurRT", VA_STATUS_ERROR_INVALID_SURFACE);
    curRT->pDecCtx = m_ddiDecodeCtx;

    DDI_CODEC_RENDER_TARGET_TABLE *RTtbl;
    RTtbl             = &(m_ddiDecodeCtx->RTtbl);
    RTtbl->pCurrentRT = curRT;

    m_ddiDecodeCtx->bStreamOutEnabled              = false;
    m_ddiDecodeCtx->DecodeParams.m_numSlices       = 0;
    m_ddiDecodeCtx->DecodeParams.m_dataSize        = 0;
    m_ddiDecodeCtx->DecodeParams.m_deblockDataSize = 0;
    m_ddiDecodeCtx->m_groupIndex                   = 0;

    // register render targets
    DDI_CHK_RET(RegisterRTSurfaces(&m_ddiDecodeCtx->RTtbl, curRT),"RegisterRTSurfaces failed!");

    if (nullptr == m_ddiDecodeCtx->pCodecHal)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    MOS_STATUS eStatus = m_ddiDecodeCtx->pCodecHal->BeginFrame();
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_DECODING_ERROR;
    }

    DDI_FUNCTION_EXIT(VA_STATUS_SUCCESS);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMediaDecode::DecodeCombineBitstream(DDI_MEDIA_CONTEXT *mediaCtx)
{
    DDI_CODEC_COM_BUFFER_MGR *bufMgr;
    /* As it is checked in previous caller, it is skipped. */
    bufMgr = &(m_ddiDecodeCtx->BufMgr);

    if (bufMgr && (bufMgr->bIsSliceOverSize == false))
    {
        return VA_STATUS_SUCCESS;
    }

    PDDI_MEDIA_BUFFER newBitstreamBuffer;
    //allocate a new bit stream buffer
    newBitstreamBuffer = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    if (newBitstreamBuffer == nullptr)
    {
        DDI_ASSERTMESSAGE("DDI:AllocAndZeroMem return failure.");
        return VA_STATUS_ERROR_DECODING_ERROR;
    }

    newBitstreamBuffer->iSize     = m_ddiDecodeCtx->DecodeParams.m_dataSize;
    newBitstreamBuffer->uiType    = VASliceDataBufferType;
    newBitstreamBuffer->format    = Media_Format_Buffer;
    newBitstreamBuffer->uiOffset  = 0;
    newBitstreamBuffer->pMediaCtx = mediaCtx;

    VAStatus vaStatus;
    vaStatus = DdiMediaUtil_CreateBuffer(newBitstreamBuffer,
        mediaCtx->pDrmBufMgr);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(newBitstreamBuffer);
        newBitstreamBuffer = nullptr;
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    uint8_t *newBitStreamBase;
    newBitStreamBase = (uint8_t *)DdiMediaUtil_LockBuffer(newBitstreamBuffer, MOS_LOCKFLAG_WRITEONLY);

    if (newBitStreamBase == nullptr)
    {
        DdiMediaUtil_FreeBuffer(newBitstreamBuffer);
        MOS_FreeMemory(newBitstreamBuffer);
        newBitstreamBuffer = nullptr;
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    int32_t slcInd;
    //copy data to new bit stream
    for (slcInd = 0; slcInd < bufMgr->dwNumSliceData; slcInd++)
    {
        if (bufMgr->pSliceData[slcInd].bIsUseExtBuf == true)
        {
            if (bufMgr->pSliceData[slcInd].pSliceBuf)
            {
                memcpy(newBitStreamBase + bufMgr->pSliceData[slcInd].uiOffset,
                    bufMgr->pSliceData[slcInd].pSliceBuf,
                    bufMgr->pSliceData[slcInd].uiLength);
                MOS_FreeMemory(bufMgr->pSliceData[slcInd].pSliceBuf);
                bufMgr->pSliceData[slcInd].pSliceBuf    = nullptr;
                bufMgr->pSliceData[slcInd].bIsUseExtBuf = false;
            }
        }
        else
        {
            memcpy(newBitStreamBase + bufMgr->pSliceData[slcInd].uiOffset,
                bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex] + bufMgr->pSliceData[slcInd].uiOffset,
                bufMgr->pSliceData[slcInd].uiLength);
        }
    }

    //free original buffers
    if (bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex])
    {
        DdiMediaUtil_UnlockBuffer(bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]);
        bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex] = nullptr;
    }

    if (bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex])
    {
        DdiMediaUtil_FreeBuffer(bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]);
        MOS_FreeMemory(bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]);
        bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex] = nullptr;
    }

    //set new bitstream buffer
    bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex] = newBitstreamBuffer;
    bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex]       = newBitStreamBase;
    DdiMedia_MediaBufferToMosResource(m_ddiDecodeCtx->BufMgr.pBitStreamBuffObject[bufMgr->dwBitstreamIndex], &m_ddiDecodeCtx->BufMgr.resBitstreamBuffer);

    return VA_STATUS_SUCCESS;
}

void DdiMediaDecode::DestroyContext(VADriverContextP ctx)
{
    Codechal *codecHal;
    /* as they are already checked in caller, this is skipped */
    codecHal = m_ddiDecodeCtx->pCodecHal;

    if (codecHal != nullptr)
    {
        MOS_FreeMemory(codecHal->GetOsInterface()->pOsContext->pPerfData);
        codecHal->GetOsInterface()->pOsContext->pPerfData = nullptr;

        // destroy codechal
        codecHal->Destroy();
        MOS_Delete(codecHal);

        m_ddiDecodeCtx->pCodecHal = nullptr;
    }

    int32_t i;
    for (i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i++)
    {
        if ((m_ddiDecodeCtx->RTtbl.pRT[i] != nullptr) &&
            (m_ddiDecodeCtx->RTtbl.pRT[i]->pDecCtx == m_ddiDecodeCtx))
        {
            m_ddiDecodeCtx->RTtbl.pRT[i]->pDecCtx = nullptr;
        }
    }

    if (m_ddiDecodeCtx->pCpDdiInterface)
    {
        MOS_Delete(m_ddiDecodeCtx->pCpDdiInterface);
        m_ddiDecodeCtx->pCpDdiInterface = nullptr;
    }

    if (m_ddiDecodeCtx->pDecodeStatusReport)
    {
        MOS_DeleteArray(m_ddiDecodeCtx->pDecodeStatusReport);
        m_ddiDecodeCtx->pDecodeStatusReport = nullptr;
    }


    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer);
    m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer = nullptr;

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_huffmanTable);
    m_ddiDecodeCtx->DecodeParams.m_huffmanTable = nullptr;

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_picParams);
    m_ddiDecodeCtx->DecodeParams.m_picParams = nullptr;

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_sliceParams);
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = nullptr;

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_ddiDecodeCtx->DecodeParams.m_procParams != nullptr)
    {
        PCODECHAL_DECODE_PROCESSING_PARAMS procParams;

        procParams = m_ddiDecodeCtx->DecodeParams.m_procParams;
        MOS_FreeMemory(procParams->pOutputSurface);
        procParams->pOutputSurface = nullptr;

        MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_procParams);
        m_ddiDecodeCtx->DecodeParams.m_procParams = nullptr;
    }
#endif

    return;
}

void DdiMediaDecode::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    m_ddiDecodeCtx->dwWidth           = picWidth;
    m_ddiDecodeCtx->dwHeight          = picHeight;
    m_ddiDecodeCtx->wPicWidthInMB     = (int16_t)(DDI_CODEC_NUM_MACROBLOCKS_WIDTH(picWidth));
    m_ddiDecodeCtx->wPicHeightInMB    = (int16_t)(DDI_CODEC_NUM_MACROBLOCKS_HEIGHT(picHeight));
    m_ddiDecodeCtx->wMode             = CODECHAL_DECODE_MODE_AVCVLD;
    m_ddiDecodeCtx->bShortFormatInUse = false;
#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_ddiDecodeAttr->uiDecProcessingType == VA_DEC_PROCESSING)
    {
        DDI_NORMALMESSAGE("Decoding context has scaling/format conversion capabilities");
        m_ddiDecodeCtx->uiDecProcessingType = VA_DEC_PROCESSING;
    }
    else
#endif
    {
        DDI_NORMALMESSAGE("Decoding context DOESN'T have scaling/format conversion capabilities");
        m_ddiDecodeCtx->uiDecProcessingType = VA_DEC_PROCESSING_NONE;
    }
    m_ddiDecodeCtx->bStreamOutEnabled                  = false;
    m_ddiDecodeCtx->DecodeParams.m_picIdRemappingInUse = true;
}

VAStatus DdiMediaDecode::CreateCodecHal(
    DDI_MEDIA_CONTEXT       *mediaCtx,
    void                    *ptr,
    _CODECHAL_SETTINGS      *codecHalSettings,
    _CODECHAL_STANDARD_INFO *standardInfo)
{
    if ((mediaCtx == nullptr) ||
        (ptr == nullptr) ||
        (codecHalSettings == nullptr) ||
        (standardInfo == nullptr))
    {
        DDI_ASSERTMESSAGE("NULL pointer is passed for CreateCodecHal.\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    }
    MOS_CONTEXT    *mosCtx   = (MOS_CONTEXT *)ptr;
    VAStatus        vaStatus = VA_STATUS_SUCCESS;

    codecHalSettings->pCpParams = m_ddiDecodeCtx->pCpDdiInterface->GetParams();

    Codechal *codecHal = CodechalDevice::CreateFactory(
        nullptr,
        mosCtx,
        standardInfo,
        codecHalSettings);
    CodechalDecode *decoder = dynamic_cast<CodechalDecode *>(codecHal);
    if (nullptr == codecHal || nullptr == decoder)
    {
        DDI_ASSERTMESSAGE("Failure in CodecHal create.\n");
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }
    m_ddiDecodeCtx->pCodecHal = codecHal;

    CodechalCencDecode *cencDecoder = nullptr;
    if (codecHalSettings->CodecFunction == CODECHAL_FUNCTION_CENC_DECODE)
    {
        CodechalCencDecode::CreateCencDecode((CODECHAL_STANDARD)codecHalSettings->Standard, &cencDecoder);
        if (nullptr == cencDecoder)
        {
            DDI_ASSERTMESSAGE("Failure in CreateCencDecode create.\n");
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            return vaStatus;
        }
        decoder->SetCencDecoder(cencDecoder);
    }

    if (codecHal->Allocate(codecHalSettings) != MOS_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Failure in decode allocate.\n");
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    PMOS_INTERFACE osInterface = codecHal->GetOsInterface();
    if (osInterface == nullptr)
    {
        DDI_ASSERTMESSAGE("Failure in decode allocate.\n");
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

#ifdef _MMC_SUPPORTED
    if (MEDIA_IS_SKU(osInterface->pfnGetSkuTable(osInterface), FtrMemoryCompression) &&
        !mediaCtx->pMediaMemDecompState)
    {
        mediaCtx->pMediaMemDecompState =
            static_cast<MediaMemDecompState *>(MmdDevice::CreateFactory(mosCtx));
    }
#endif

    if (codecHalSettings->CodecFunction == CODECHAL_FUNCTION_CENC_DECODE)
    {
        if (cencDecoder->Initialize(decoder, osInterface->pOsContext, codecHalSettings) != MOS_STATUS_SUCCESS)
        {
            DDI_ASSERTMESSAGE("Failure in CreateCencDecode create.\n");
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
            return vaStatus;
        }
    }

    if (decoder->IsStatusQueryReportingEnabled())
    {
        m_ddiDecodeCtx->pDecodeStatusReport =
            MOS_NewArray(CodechalDecodeStatusReport, CODECHAL_DECODE_STATUS_NUM);
        if (m_ddiDecodeCtx->pDecodeStatusReport == nullptr)
        {
            DDI_ASSERTMESSAGE("Failure to allocate the status buffer for decoding\n");
            vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
    }

    m_ddiDecodeCtx->wDecStatusReportNum = 0;

    return vaStatus;
}
