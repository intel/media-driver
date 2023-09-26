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
//! \file     media_ddi_decode_base_specific.cpp
//! \brief    The class implementation of DdiDecodeBase for all decoders
//!

#include "ddi_vp_functions.h"
#include "media_libva_util_next.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd_next.h"
#include "mos_solo_generic.h"
#include "ddi_decode_base_specific.h"
#include "media_libva_common_next.h"
#include "media_interfaces_codechal_next.h"
#include "ddi_decode_trace_specific.h"
#include "media_libva_caps_next.h"

namespace decode
{

DdiDecodeBase::DdiDecodeBase()
    : DdiCodecBase()
{
    DDI_CODEC_FUNC_ENTER;

    m_decodeCtx = nullptr;
    MOS_ZeroMemory(&m_destSurface, sizeof(m_destSurface));
    m_groupIndex    = 0;
    m_picWidthInMB  = 0;
    m_picHeightInMB = 0;
    m_decProcessingType = 0;
    m_width  = 0;
    m_height = 0;
    m_streamOutEnabled = false;
    m_sliceParamBufNum = 0;
    m_sliceCtrlBufNum  = 0;
    m_codechalSettings = CodechalSetting::CreateCodechalSetting();
}

VAStatus DdiDecodeBase::BasicInit(
    ConfigLinux *configItem)
{
    DDI_CODEC_FUNC_ENTER;

    if (configItem == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    m_ddiDecodeAttr = (ConfigLinux *)MOS_AllocAndZeroMemory(sizeof(ConfigLinux));
    if (m_ddiDecodeAttr && configItem)
    {
        MOS_SecureMemcpy(m_ddiDecodeAttr, sizeof(ConfigLinux), configItem, sizeof(ConfigLinux));
    }

    m_decodeCtx = (DDI_DECODE_CONTEXT *)MOS_AllocAndZeroMemory(sizeof(DDI_DECODE_CONTEXT));

    if ((m_ddiDecodeAttr == nullptr) ||
        (m_decodeCtx == nullptr))
    {
        MOS_FreeMemory(m_ddiDecodeAttr);
        m_ddiDecodeAttr = nullptr;
        MOS_FreeMemory(m_decodeCtx);
        m_decodeCtx  = nullptr;
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    return VA_STATUS_SUCCESS;
}

uint32_t DdiDecodeBase::GetBsBufOffset(int32_t sliceGroup)
{
    DDI_CODEC_FUNC_ENTER;

    return m_decodeCtx->BufMgr.pSliceData[sliceGroup].uiOffset;
}

VAStatus DdiDecodeBase::ParseProcessingBuffer(
    DDI_MEDIA_CONTEXT *mediaCtx,
    void              *bufAddr)
{
    DDI_CODEC_FUNC_ENTER;

#ifdef _DECODE_PROCESSING_SUPPORTED
    VAProcPipelineParameterBuffer *procBuf =
        (VAProcPipelineParameterBuffer *)bufAddr;

    DDI_CODEC_CHK_NULL(procBuf, "nullptr Processing buffer", VA_STATUS_ERROR_INVALID_PARAMETER)

    if (m_decProcessingType == VA_DEC_PROCESSING)
    {
        if (m_procBuf == nullptr)
        {
            m_procBuf = (VAProcPipelineParameterBuffer*)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineParameterBuffer));
            DDI_CODEC_CHK_NULL(m_procBuf, "nullptr m_procBuf", VA_STATUS_ERROR_ALLOCATION_FAILED);
            MOS_SecureMemcpy(m_procBuf, sizeof(VAProcPipelineParameterBuffer), procBuf, sizeof(VAProcPipelineParameterBuffer));
        }
        auto decProcessingParams =
            (DecodeProcessingParams *)m_decodeCtx->DecodeParams.m_procParams;

        auto decProcessingSurface = decProcessingParams->m_outputSurface;

        memset(decProcessingSurface, 0, sizeof(MOS_SURFACE));

        PDDI_MEDIA_SURFACE surface =
            MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, procBuf->additional_outputs[0]);

        DDI_CODEC_CHK_NULL(surface, "Null surface in Processing buffer", VA_STATUS_ERROR_INVALID_PARAMETER)

        MediaLibvaCommonNext::MediaSurfaceToMosResource(surface, &(decProcessingSurface->OsResource));

        decProcessingSurface->dwWidth  = decProcessingSurface->OsResource.iWidth;
        decProcessingSurface->dwHeight = decProcessingSurface->OsResource.iHeight;
        decProcessingSurface->dwPitch  = decProcessingSurface->OsResource.iPitch;
        decProcessingSurface->TileType = decProcessingSurface->OsResource.TileType;
        decProcessingSurface->Format   = decProcessingSurface->OsResource.Format;

        if (procBuf->surface_region != nullptr)
        {
            m_requireInputRegion                               = false;
            decProcessingParams->m_inputSurfaceRegion.m_x      = procBuf->surface_region->x;
            decProcessingParams->m_inputSurfaceRegion.m_y      = procBuf->surface_region->y;
            decProcessingParams->m_inputSurfaceRegion.m_width  = procBuf->surface_region->width;
            decProcessingParams->m_inputSurfaceRegion.m_height = procBuf->surface_region->height;
        }
        else
        {
            m_requireInputRegion = true;
        }

        decProcessingParams->m_outputSurface                = decProcessingSurface;
        if (procBuf->output_region != nullptr)
        {
            decProcessingParams->m_outputSurfaceRegion.m_x      = procBuf->output_region->x;
            decProcessingParams->m_outputSurfaceRegion.m_y      = procBuf->output_region->y;
            decProcessingParams->m_outputSurfaceRegion.m_width  = procBuf->output_region->width;
            decProcessingParams->m_outputSurfaceRegion.m_height = procBuf->output_region->height;
        }
        else
        {
            decProcessingParams->m_outputSurfaceRegion.m_x      = 0;
            decProcessingParams->m_outputSurfaceRegion.m_y      = 0;
            decProcessingParams->m_outputSurfaceRegion.m_width  = decProcessingSurface->dwWidth;
            decProcessingParams->m_outputSurfaceRegion.m_height = decProcessingSurface->dwHeight;
        }

        // Interpolation flags
        // Set the vdbox scaling mode
        uint32_t uInterpolationflags = 0;
#if VA_CHECK_VERSION(1, 9, 0)
        uInterpolationflags = procBuf->filter_flags & VA_FILTER_INTERPOLATION_MASK;
#endif

        switch (uInterpolationflags)
        {
#if VA_CHECK_VERSION(1, 9, 0)
        case VA_FILTER_INTERPOLATION_NEAREST_NEIGHBOR:
            decProcessingParams->m_scalingMode = CODECHAL_SCALING_NEAREST;
            break;
        case VA_FILTER_INTERPOLATION_BILINEAR:
            decProcessingParams->m_scalingMode = CODECHAL_SCALING_BILINEAR;
            break;
        case VA_FILTER_INTERPOLATION_ADVANCED:
        case VA_FILTER_INTERPOLATION_DEFAULT:
#endif
        default:
            decProcessingParams->m_scalingMode = CODECHAL_SCALING_AVS;
           break;
        }

        // Chroma siting
        // Set the vertical chroma siting info
        uint32_t chromaSitingFlags = 0;
        chromaSitingFlags                       = procBuf->input_color_properties.chroma_sample_location & 0x3;
        decProcessingParams->m_chromaSitingType = CODECHAL_CHROMA_SITING_NONE;
        decProcessingParams->m_rotationState    = 0;
        decProcessingParams->m_blendState       = 0;
        decProcessingParams->m_mirrorState      = 0;

        switch (chromaSitingFlags)
        {
        case VA_CHROMA_SITING_VERTICAL_TOP:
            decProcessingParams->m_chromaSitingType = CODECHAL_CHROMA_SITING_VERT_TOP;
            break;
        case VA_CHROMA_SITING_VERTICAL_CENTER:
            decProcessingParams->m_chromaSitingType = CODECHAL_CHROMA_SITING_VERT_CENTER;
            break;
        case VA_CHROMA_SITING_VERTICAL_BOTTOM:
            decProcessingParams->m_chromaSitingType = CODECHAL_CHROMA_SITING_VERT_BOTTOM;
            break;
        default:
            decProcessingParams->m_chromaSitingType = CODECHAL_CHROMA_SITING_NONE;
            break;
        }

        if (decProcessingParams->m_chromaSitingType != CODECHAL_CHROMA_SITING_NONE)
        {
            // Set the horizontal chroma siting info
            chromaSitingFlags = procBuf->input_color_properties.chroma_sample_location & 0xc;

            switch (chromaSitingFlags)
            {
            case VA_CHROMA_SITING_HORIZONTAL_LEFT:
                decProcessingParams->m_chromaSitingType |= CODECHAL_CHROMA_SITING_HORZ_LEFT;
                break;
            case VA_CHROMA_SITING_HORIZONTAL_CENTER:
                decProcessingParams->m_chromaSitingType |= CODECHAL_CHROMA_SITING_HORZ_CENTER;
                break;
            default:
                decProcessingParams->m_chromaSitingType = CODECHAL_CHROMA_SITING_NONE;
                break;
            }
        }
    }

    return VA_STATUS_SUCCESS;
#else
    return VA_STATUS_ERROR_INVALID_PARAMETER;
#endif
}

VAStatus DdiDecodeBase::BeginPicture(
    VADriverContextP ctx,
    VAContextID      context,
    VASurfaceID      renderTarget)
{
    DDI_CODEC_FUNC_ENTER;

    PDDI_MEDIA_CONTEXT mediaCtx;

    /* As it is already checked in the upper caller, skip the check */
    mediaCtx = GetMediaContext(ctx);

#ifdef _DECODE_PROCESSING_SUPPORTED
    // renderTarget is decode output surface; set renderTarget as vp sfc input surface m_procBuf->surface = rederTarget
    if (m_procBuf)
    {
        m_procBuf->surface = renderTarget;
    }
#endif

    DDI_MEDIA_SURFACE *curRT = nullptr;
    curRT = (DDI_MEDIA_SURFACE *)MediaLibvaCommonNext::GetSurfaceFromVASurfaceID(mediaCtx, renderTarget);
    DDI_CODEC_CHK_NULL(curRT, "nullptr pCurRT", VA_STATUS_ERROR_INVALID_SURFACE);
    curRT->pDecCtx = m_decodeCtx;

    DDI_CODEC_RENDER_TARGET_TABLE *RTtbl;
    RTtbl             = &(m_decodeCtx->RTtbl);
    RTtbl->pCurrentRT = curRT;

    m_streamOutEnabled                           = false;
    m_decodeCtx->DecodeParams.m_numSlices        = 0;
    m_decodeCtx->DecodeParams.m_dataSize         = 0;
    m_decodeCtx->DecodeParams.m_dataOffset       = 0;
    m_decodeCtx->DecodeParams.m_deblockDataSize  = 0;
    m_decodeCtx->DecodeParams.m_executeCallIndex = 0;
    m_decodeCtx->DecodeParams.m_cencBuf          = nullptr;
    m_groupIndex                                 = 0;

    // register render targets
    DDI_CHK_RET(RegisterRTSurfaces(&m_decodeCtx->RTtbl, curRT),"RegisterRTSurfaces failed!");

    if (nullptr == m_decodeCtx->pCodecHal)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    MOS_STATUS eStatus = m_decodeCtx->pCodecHal->BeginFrame();
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_DECODING_ERROR;
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeBase::DecodeCombineBitstream(DDI_MEDIA_CONTEXT *mediaCtx)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_COM_BUFFER_MGR *bufMgr = nullptr;
    /* As it is checked in previous caller, it is skipped. */
    bufMgr = &(m_decodeCtx->BufMgr);

    if (bufMgr && (bufMgr->bIsSliceOverSize == false))
    {
        return VA_STATUS_SUCCESS;
    }

    PDDI_MEDIA_BUFFER newBitstreamBuffer;
    // allocate a new bit stream buffer
    newBitstreamBuffer = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    if (newBitstreamBuffer == nullptr)
    {
        DDI_CODEC_ASSERTMESSAGE("DDI:AllocAndZeroMem return failure.");
        return VA_STATUS_ERROR_DECODING_ERROR;
    }

    newBitstreamBuffer->iSize     = m_decodeCtx->DecodeParams.m_dataSize;
    newBitstreamBuffer->uiType    = VASliceDataBufferType;
    newBitstreamBuffer->format    = Media_Format_Buffer;
    newBitstreamBuffer->uiOffset  = 0;
    newBitstreamBuffer->pMediaCtx = mediaCtx;

    VAStatus vaStatus;
    vaStatus = MediaLibvaUtilNext::CreateBuffer(newBitstreamBuffer,
        mediaCtx->pDrmBufMgr);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        MOS_FreeMemory(newBitstreamBuffer);
        newBitstreamBuffer = nullptr;
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    uint8_t *newBitStreamBase = nullptr;
    newBitStreamBase = (uint8_t *)MediaLibvaUtilNext::LockBuffer(newBitstreamBuffer, MOS_LOCKFLAG_WRITEONLY);

    if (newBitStreamBase == nullptr)
    {
        MediaLibvaUtilNext::FreeBuffer(newBitstreamBuffer);
        MOS_FreeMemory(newBitstreamBuffer);
        newBitstreamBuffer = nullptr;
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    uint32_t slcInd = 0;
    // copy data to new bit stream
    for (slcInd = 0; slcInd < bufMgr->dwNumSliceData; slcInd++)
    {
        if (bufMgr->pSliceData[slcInd].bIsUseExtBuf == true)
        {
            if (bufMgr->pSliceData[slcInd].pSliceBuf)
            {
                MOS_SecureMemcpy(newBitStreamBase + bufMgr->pSliceData[slcInd].uiOffset,
                    bufMgr->pSliceData[slcInd].uiLength,
                    bufMgr->pSliceData[slcInd].pSliceBuf,
                    bufMgr->pSliceData[slcInd].uiLength);
                MOS_FreeMemory(bufMgr->pSliceData[slcInd].pSliceBuf);
                bufMgr->pSliceData[slcInd].pSliceBuf    = nullptr;
                bufMgr->pSliceData[slcInd].bIsUseExtBuf = false;
            }
        }
        else
        {
            MOS_SecureMemcpy(newBitStreamBase + bufMgr->pSliceData[slcInd].uiOffset,
                bufMgr->pSliceData[slcInd].uiLength,
                bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex] + bufMgr->pSliceData[slcInd].uiOffset,
                bufMgr->pSliceData[slcInd].uiLength);
        }
    }

    // free original buffers
    if (bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex])
    {
        MediaLibvaUtilNext::UnlockBuffer(bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]);
        bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex] = nullptr;
    }

    if (bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex])
    {
        MediaLibvaUtilNext::FreeBuffer(bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]);
        MOS_FreeMemory(bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]);
        bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex] = nullptr;
    }

    // set new bitstream buffer
    bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex] = newBitstreamBuffer;
    bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex]       = newBitStreamBase;
    MediaLibvaCommonNext::MediaBufferToMosResource(m_decodeCtx->BufMgr.pBitStreamBuffObject[bufMgr->dwBitstreamIndex], &m_decodeCtx->BufMgr.resBitstreamBuffer);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeBase::CheckDecodeResolution(
    ConfigLinux       *configItem,
    uint32_t          width,
    uint32_t          height)
{
    DDI_CODEC_FUNC_ENTER;
    uint32_t maxWidth  = 0;
    uint32_t maxHeight = 0;

    DDI_CODEC_CHK_NULL(configItem, "nullptr configItem", VA_STATUS_ERROR_INVALID_CONFIG);
    VAConfigAttrib  *supportedAttribList = configItem->attribList;
    DDI_CODEC_CHK_NULL(supportedAttribList, "nullptr supportedAttribList", VA_STATUS_ERROR_INVALID_CONFIG);

    // parse supported width and height from capstable attributes
    for (uint32_t i = 0; i < configItem->numAttribs; i++)
    {
        if (supportedAttribList[i].type == VAConfigAttribMaxPictureWidth)
        {
            maxWidth = supportedAttribList[i].value;
        }
        else if (supportedAttribList[i].type == VAConfigAttribMaxPictureHeight)
        {
            maxHeight = supportedAttribList[i].value;
        }
        else
        {
            continue;
        }
    }

    if (width > maxWidth || height > maxHeight)
    {
        return VA_STATUS_ERROR_RESOLUTION_NOT_SUPPORTED;
    }
    else
    {
        return VA_STATUS_SUCCESS;
    }

}

void DdiDecodeBase::DestroyContext(VADriverContextP ctx)
{
    DDI_CODEC_FUNC_ENTER;

    Codechal *codecHal = nullptr;
    /* as they are already checked in caller, this is skipped */
    codecHal = m_decodeCtx->pCodecHal;

    if (codecHal != nullptr)
    {
        if (codecHal->GetOsInterface() && codecHal->GetOsInterface()->pOsContext)
        {
            MOS_FreeMemory(codecHal->GetOsInterface()->pOsContext->pPerfData);
            codecHal->GetOsInterface()->pOsContext->pPerfData = nullptr;
        }

        // destroy codechal
        codecHal->Destroy();
        MOS_Delete(codecHal);

        m_decodeCtx->pCodecHal = nullptr;
    }

    int32_t i = 0;
    for (i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i++)
    {
        if ((m_decodeCtx->RTtbl.pRT[i] != nullptr) &&
            (m_decodeCtx->RTtbl.pRT[i]->pDecCtx == m_decodeCtx))
        {
            m_decodeCtx->RTtbl.pRT[i]->pDecCtx = nullptr;
        }
    }

    if (m_decodeCtx->pCpDdiInterfaceNext)
    {
        MOS_Delete(m_decodeCtx->pCpDdiInterfaceNext);
    }

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_iqMatrixBuffer);
    m_decodeCtx->DecodeParams.m_iqMatrixBuffer = nullptr;

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_huffmanTable);
    m_decodeCtx->DecodeParams.m_huffmanTable = nullptr;

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_picParams);
    m_decodeCtx->DecodeParams.m_picParams = nullptr;

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_sliceParams);
    m_decodeCtx->DecodeParams.m_sliceParams = nullptr;

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_extPicParams);
    m_decodeCtx->DecodeParams.m_sliceParams = nullptr;

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_advPicParams);
    m_decodeCtx->DecodeParams.m_sliceParams = nullptr;

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_extSliceParams);
    m_decodeCtx->DecodeParams.m_sliceParams = nullptr;

    MOS_FreeMemory(m_decodeCtx->DecodeParams.m_subsetParams);
    m_decodeCtx->DecodeParams.m_sliceParams = nullptr;

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_decodeCtx->DecodeParams.m_procParams != nullptr)
    {
        auto procParams =
            (DecodeProcessingParams *)m_decodeCtx->DecodeParams.m_procParams;
        MOS_FreeMemory(procParams->m_outputSurface);
        procParams->m_outputSurface = nullptr;

        MOS_FreeMemory(m_decodeCtx->DecodeParams.m_procParams);
        m_decodeCtx->DecodeParams.m_procParams = nullptr;
    }
#endif

    return;
}

int32_t DdiDecodeBase::GetBitstreamBufIndexFromBuffer(DDI_CODEC_COM_BUFFER_MGR *bufMgr, DDI_MEDIA_BUFFER *buf)
{
    DDI_CODEC_FUNC_ENTER;

    int32_t i = 0;
    for(i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        if (bufMgr->pBitStreamBuffObject[i]->bo == buf->bo)
        {
            return i;
        }
    }

    return DDI_CODEC_INVALID_BUFFER_INDEX;
}

VAStatus DdiDecodeBase::AllocBsBuffer(
    DDI_CODEC_COM_BUFFER_MGR *bufMgr,
    DDI_MEDIA_BUFFER         *buf)
{
    DDI_CODEC_FUNC_ENTER;

    uint32_t         index = 0, i = 0;
    VAStatus         vaStatus  = VA_STATUS_SUCCESS;
    uint8_t          *sliceBuf = nullptr;
    DDI_MEDIA_BUFFER *bsBufObj = nullptr;
    uint8_t          *bsBufBaseAddr = nullptr;
    bool             createBsBuffer = false;

    if (nullptr == bufMgr || nullptr == buf || nullptr == (m_decodeCtx->pMediaCtx))
    {
        DDI_CODEC_ASSERTMESSAGE("invalidate input parameters.");
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    index = bufMgr->dwNumSliceData;

    /* the pSliceData needs to be reallocated in order to contain more SliceDataBuf */
    if (index >= bufMgr->m_maxNumSliceData)
    {
        /* In theroy it can resize the m_maxNumSliceData one by one. But in order to
         * avoid calling realloc frequently, it will try to allocate 10 to  hold more
         * SliceDataBuf. This is only for the optimized purpose.
         */
        int32_t reallocSize = bufMgr->m_maxNumSliceData + 10;

        bufMgr->pSliceData  = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)realloc(bufMgr->pSliceData, sizeof(bufMgr->pSliceData[0]) * reallocSize);

        if (bufMgr->pSliceData == nullptr)
        {
            DDI_CODEC_ASSERTMESSAGE("fail to reallocate pSliceData\n.");
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        memset(bufMgr->pSliceData + bufMgr->m_maxNumSliceData, 0,
               sizeof(bufMgr->pSliceData[0]) * 10);

        bufMgr->m_maxNumSliceData += 10;
    }

    if (index >= 1)
    {
        buf->uiOffset = bufMgr->pSliceData[index-1].uiOffset + bufMgr->pSliceData[index-1].uiLength;
        if ((buf->uiOffset + buf->iSize) > bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]->iSize)
        {
            sliceBuf = (uint8_t*)MOS_AllocAndZeroMemory(buf->iSize);
            if (sliceBuf == nullptr)
            {
                DDI_CODEC_ASSERTMESSAGE("DDI:AllocAndZeroMem return failure.")
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
            bufMgr->bIsSliceOverSize = true;
        }
        else
        {
            bufMgr->bIsSliceOverSize = false;
        }
    }
    else
    {
        bufMgr->bIsSliceOverSize = false;
        for (i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
        {
            if (bufMgr->pBitStreamBuffObject[i]->bo != nullptr)
            {
                if (!mos_bo_busy(bufMgr->pBitStreamBuffObject[i]->bo))
                {
                    // find a bitstream buffer whoes graphic memory is allocated but not used by HW now.
                    break;
                }
            }
            else
            {
                // find a new bitstream buffer whoes graphic memory is not allocated yet
                break;
            }
        }

        if (i == DDI_CODEC_MAX_BITSTREAM_BUFFER)
        {
            // find the oldest bistream buffer which is the most possible one to become free in the shortest time.
            bufMgr->dwBitstreamIndex = (bufMgr->ui64BitstreamOrder >> (DDI_CODEC_BITSTREAM_BUFFER_INDEX_BITS * DDI_CODEC_MAX_BITSTREAM_BUFFER_MINUS1)) & DDI_CODEC_MAX_BITSTREAM_BUFFER_INDEX;
            // wait until decode complete
            mos_bo_wait_rendering(bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]->bo);
        }
        else
        {
            bufMgr->dwBitstreamIndex = i;
        }
        bufMgr->ui64BitstreamOrder = (bufMgr->ui64BitstreamOrder << 4) + bufMgr->dwBitstreamIndex;

        bsBufObj            = bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex];
        bsBufObj->pMediaCtx = m_decodeCtx->pMediaCtx;
        bsBufBaseAddr       = bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex];

        if (bsBufBaseAddr == nullptr)
        {
            createBsBuffer = true;
            if (buf->iSize > bsBufObj->iSize)
            {
                bsBufObj->iSize = buf->iSize;
            }
        }
        else if (buf->iSize > bsBufObj->iSize)
        {
           // free bo
            MediaLibvaUtilNext::UnlockBuffer(bsBufObj);
            MediaLibvaUtilNext::FreeBuffer(bsBufObj);
            bsBufBaseAddr = nullptr;

            createBsBuffer  = true;
            bsBufObj->iSize = buf->iSize;
        }

        if (createBsBuffer)
        {
            if (VA_STATUS_SUCCESS != MediaLibvaUtilNext::CreateBuffer(bsBufObj, m_decodeCtx->pMediaCtx->pDrmBufMgr))
            {
               return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }

            bsBufBaseAddr = (uint8_t*)MediaLibvaUtilNext::LockBuffer(bsBufObj, MOS_LOCKFLAG_WRITEONLY);
            if (bsBufBaseAddr == nullptr)
            {
                MediaLibvaUtilNext::FreeBuffer(bsBufObj);
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
            bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex] = bsBufBaseAddr;
        }
    }

    if (bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex] == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    bufMgr->pSliceData[index].uiLength = buf->iSize;
    bufMgr->pSliceData[index].uiOffset = buf->uiOffset;

    if (bufMgr->bIsSliceOverSize == true)
    {
        buf->pData                              = sliceBuf;
        buf->uiOffset                           = 0;
        bufMgr->pSliceData[index].bIsUseExtBuf  = true;
        bufMgr->pSliceData[index].pSliceBuf     = sliceBuf;
        buf->bCFlushReq                         = false;
    }
    else
    {
        buf->pData                              = (uint8_t*)(bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex]);
        bufMgr->pSliceData[index].bIsUseExtBuf  = false;
        bufMgr->pSliceData[index].pSliceBuf     = nullptr;
        buf->bCFlushReq                         = true;
    }

    bufMgr->dwNumSliceData ++;
    buf->bo = bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]->bo;

    return VA_STATUS_SUCCESS;
}

MOS_FORMAT DdiDecodeBase::GetFormat()
{
    DDI_CODEC_FUNC_ENTER;

    return  Format_NV12;
}

VAStatus DdiDecodeBase::InitDecodeParams(
    VADriverContextP ctx,
    VAContextID      context)
{
    DDI_CODEC_FUNC_ENTER;

    /* skip the mediaCtx check as it is checked in caller */
    PDDI_MEDIA_CONTEXT mediaCtx;
    mediaCtx = GetMediaContext(ctx);
    DDI_CHK_RET(DecodeCombineBitstream(mediaCtx),"DecodeCombineBitstream failed!");
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_decodeCtx->BufMgr);
    bufMgr->dwNumSliceData    = 0;
    bufMgr->dwNumSliceControl = 0;
    memset(&m_destSurface, 0, sizeof(MOS_SURFACE));
    m_destSurface.dwOffset = 0;

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_decodeCtx->RTtbl);

    if ((rtTbl == nullptr) || (rtTbl->pCurrentRT == nullptr))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    return VA_STATUS_SUCCESS;

}

VAStatus DdiDecodeBase::SetDecodeParams()
{
    DDI_CODEC_FUNC_ENTER;

    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_decodeCtx->BufMgr);
    if ((&m_decodeCtx->DecodeParams)->m_numSlices == 0)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    MOS_FORMAT expectedFormat = GetFormat();
    m_destSurface.Format      = expectedFormat;
    MediaLibvaCommonNext::MediaSurfaceToMosResource((&(m_decodeCtx->RTtbl))->pCurrentRT, &(m_destSurface.OsResource));

    if (m_destSurface.OsResource.Format != expectedFormat)
    {
        DDI_CODEC_NORMALMESSAGE("Surface fomrat of decoded surface is inconsistent with Codec bitstream\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_decodeCtx->DecodeParams.m_destSurface    = &m_destSurface;
    m_decodeCtx->DecodeParams.m_deblockSurface = nullptr;

    m_decodeCtx->DecodeParams.m_dataBuffer       = &bufMgr->resBitstreamBuffer;
    m_decodeCtx->DecodeParams.m_bitStreamBufData = bufMgr->pBitstreamBuffer;

    m_decodeCtx->DecodeParams.m_bitplaneBuffer = nullptr;

    if (m_streamOutEnabled)
    {
        m_decodeCtx->DecodeParams.m_streamOutEnabled        = true;
        m_decodeCtx->DecodeParams.m_externalStreamOutBuffer = &bufMgr->resExternalStreamOutBuffer;
    }
    else
    {
        m_decodeCtx->DecodeParams.m_streamOutEnabled        = false;
        m_decodeCtx->DecodeParams.m_externalStreamOutBuffer = nullptr;
    }

    if (m_decodeCtx->pCpDdiInterfaceNext)
    {
        DDI_CHK_RET(m_decodeCtx->pCpDdiInterfaceNext->SetDecodeParams(m_decodeCtx, m_codechalSettings), "SetDecodeParams failed!");
    }

    Mos_Solo_OverrideBufferSize(m_decodeCtx->DecodeParams.m_dataSize, m_decodeCtx->DecodeParams.m_dataBuffer);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeBase::ExtraDownScaling(
    VADriverContextP ctx,
    VAContextID      context)
{
    DDI_CODEC_FUNC_ENTER;

#ifdef _DECODE_PROCESSING_SUPPORTED
    DDI_CODEC_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    VAStatus vaStatus = MOS_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx = GetMediaContext(ctx);
    DDI_CODEC_CHK_NULL(mediaCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CODEC_CHK_NULL(m_decodeCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    bool isDecodeDownScalingSupported = false;
    DecodePipelineAdapter *decoder = dynamic_cast<DecodePipelineAdapter *>(m_decodeCtx->pCodecHal);
    DDI_CODEC_CHK_NULL(decoder, "nullptr decoder", VA_STATUS_ERROR_INVALID_PARAMETER);
    isDecodeDownScalingSupported = decoder->IsDownSamplingSupported();

    if (m_decodeCtx->DecodeParams.m_procParams != nullptr &&
       m_procBuf &&
       !isDecodeDownScalingSupported)
    {
        // check vp context
        VAContextID vpCtxID = VA_INVALID_ID;
        if (mediaCtx->pVpCtxHeap != nullptr && mediaCtx->pVpCtxHeap->pHeapBase != nullptr)
        {
            // Get VP Context from heap.
            vpCtxID = (VAContextID)(0 + DDI_MEDIA_SOFTLET_VACONTEXTID_VP_OFFSET);
        }
        else
        {
            // Create VP Context.
            vaStatus = mediaCtx->m_compList[CompVp]->CreateContext(ctx, 0, 0, 0, 0, 0, 0, &vpCtxID);
            DDI_CHK_RET(vaStatus, "Create VP Context failed.");
        }

        uint32_t ctxType;
        PDDI_VP_CONTEXT pVpCtx = (PDDI_VP_CONTEXT)MediaLibvaCommonNext::GetContextFromContextID(ctx, vpCtxID, &ctxType);
        DDI_CODEC_CHK_NULL(pVpCtx, "nullptr pVpCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

        // Set parameters
        VAProcPipelineParameterBuffer* pInputPipelineParam = m_procBuf;
        DDI_CODEC_CHK_NULL(pInputPipelineParam, "nullptr pInputPipelineParam", VA_STATUS_ERROR_ALLOCATION_FAILED);

        vaStatus = mediaCtx->m_compList[CompVp]->BeginPicture(ctx, vpCtxID, pInputPipelineParam->additional_outputs[0]);
        DDI_CHK_RET(vaStatus, "VP BeginPicture failed");

        vaStatus = m_decodeCtx->pVpDdiInterface->DdiSetProcPipelineParams(ctx, pVpCtx, pInputPipelineParam);
        DDI_CHK_RET(vaStatus, "VP SetProcPipelineParams failed.");

        vaStatus = mediaCtx->m_compList[CompVp]->EndPicture(ctx, vpCtxID);
        DDI_CHK_RET(vaStatus, "VP EndPicture failed.");
    }
#endif
    return MOS_STATUS_SUCCESS;
}

VAStatus DdiDecodeBase::InitDummyReference(CodechalDecode& decoder)
{
    DDI_CODEC_FUNC_ENTER;

    PMOS_SURFACE dummyReference = decoder.GetDummyReference();

    // If dummy reference is from decode output surface, need to update frame by frame
    if (decoder.GetDummyReferenceStatus() == CODECHAL_DUMMY_REFERENCE_DEST_SURFACE)
    {
        MOS_ZeroMemory(dummyReference, sizeof(MOS_SURFACE));
        decoder.SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_INVALID);
    }

    if (!Mos_ResourceIsNull(&dummyReference->OsResource))
    {
        Mos_Specific_GetResourceInfo(decoder.GetOsInterface(), &dummyReference->OsResource, dummyReference);

        // Check if need to re-get dummy reference from DPB or re-allocated
        if (dummyReference->dwWidth  < m_decodeCtx->DecodeParams.m_destSurface->dwWidth ||
            dummyReference->dwHeight < m_decodeCtx->DecodeParams.m_destSurface->dwHeight)
        {
            // Check if the dummy reference needs to be re-allocated
            if (decoder.GetDummyReferenceStatus() == CODECHAL_DUMMY_REFERENCE_ALLOCATED)
            {
                decoder.GetOsInterface()->pfnFreeResource(decoder.GetOsInterface(), &dummyReference->OsResource);
            }

            // Reset dummy reference
            MOS_ZeroMemory(dummyReference, sizeof(MOS_SURFACE));
            decoder.SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_INVALID);

            // Considering potential risk, disable the dummy reference from DPB path temporarily
            //GetDummyReferenceFromDPB(m_decodeCtx);

            //if (!Mos_ResourceIsNull(&dummyReference->OsResource))
            //{
            //    decoder->SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_DPB);
            //}
        }
    }
    else
    {
        // Init dummy reference
        MOS_ZeroMemory(dummyReference, sizeof(MOS_SURFACE));
        decoder.SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_INVALID);

        // Considering potential risk, disable the dummy reference from DPB path temporarily
        //GetDummyReferenceFromDPB(m_decodeCtx);
        //if (!Mos_ResourceIsNull(&dummyReference->OsResource))
        //{
        //    decoder->SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_DPB);
        //}
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeBase::InitDummyReference(DecodePipelineAdapter &decoder)
{
    DDI_CODEC_FUNC_ENTER;

    PMOS_SURFACE dummyReference = decoder.GetDummyReference();

    // If dummy reference is from decode output surface, need to update frame by frame
    if (decoder.GetDummyReferenceStatus() == CODECHAL_DUMMY_REFERENCE_DEST_SURFACE)
    {
        MOS_ZeroMemory(dummyReference, sizeof(MOS_SURFACE));
        decoder.SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_INVALID);
    }

    if (!Mos_ResourceIsNull(&dummyReference->OsResource))
    {
        Mos_Specific_GetResourceInfo(decoder.GetOsInterface(), &dummyReference->OsResource, dummyReference);

        // Check if need to re-get dummy reference from DPB or re-allocated
        if (dummyReference->dwWidth  < m_decodeCtx->DecodeParams.m_destSurface->dwWidth ||
            dummyReference->dwHeight < m_decodeCtx->DecodeParams.m_destSurface->dwHeight)
        {
            // Check if the dummy reference needs to be re-allocated
            if (decoder.GetDummyReferenceStatus() == CODECHAL_DUMMY_REFERENCE_ALLOCATED)
            {
                decoder.GetOsInterface()->pfnFreeResource(decoder.GetOsInterface(), &dummyReference->OsResource);
            }

            // Reset dummy reference
            MOS_ZeroMemory(dummyReference, sizeof(MOS_SURFACE));
            decoder.SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_INVALID);

            // Considering potential risk, disable the dummy reference from DPB path temporarily
            //GetDummyReferenceFromDPB(m_decodeCtx);

            //if (!Mos_ResourceIsNull(&dummyReference->OsResource))
            //{
            //    decoder->SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_DPB);
            //}
        }
    }
    else
    {
        // Init dummy reference
        MOS_ZeroMemory(dummyReference, sizeof(MOS_SURFACE));
        decoder.SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_INVALID);

        // Considering potential risk, disable the dummy reference from DPB path temporarily
        //GetDummyReferenceFromDPB(m_decodeCtx);
        //if (!Mos_ResourceIsNull(&dummyReference->OsResource))
        //{
        //    decoder->SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_DPB);
        //}
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeBase::EndPicture(
    VADriverContextP ctx,
    VAContextID      context)
{
    DDI_CODEC_FUNC_ENTER;

    if (m_decodeCtx->bDecodeModeReported == false)
    {
        ReportDecodeMode(m_decodeCtx->wMode);
        m_decodeCtx->bDecodeModeReported = true;
    }

    DDI_CHK_RET(InitDecodeParams(ctx,context),"InitDecodeParams failed!");

    DDI_CHK_RET(SetDecodeParams(), "SetDecodeParams failed!");
    DDI_CHK_RET(ClearRefList(&(m_decodeCtx->RTtbl), m_withDpb), "ClearRefList failed!");
    if (m_decodeCtx->pCodecHal == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (MEDIA_IS_WA(&m_decodeCtx->pMediaCtx->WaTable, WaDummyReference))
    {
        Mos_Specific_GetResourceInfo(
            m_decodeCtx->pCodecHal->GetOsInterface(), 
            &m_decodeCtx->DecodeParams.m_destSurface->OsResource,
            m_decodeCtx->DecodeParams.m_destSurface);

        DecodePipelineAdapter *decoder = dynamic_cast<DecodePipelineAdapter *>(m_decodeCtx->pCodecHal);
        DDI_CODEC_CHK_NULL(decoder, "Null decoder", VA_STATUS_ERROR_INVALID_PARAMETER);
        DDI_CHK_RET(InitDummyReference(*decoder), "InitDummyReference failed!");
    }

    MOS_STATUS status = m_decodeCtx->pCodecHal->Execute((void *)(&m_decodeCtx->DecodeParams));
    if (status != MOS_STATUS_SUCCESS)
    {
        DDI_CODEC_ASSERTMESSAGE("DDI:DdiDecode_DecodeInCodecHal return failure.");
        return VA_STATUS_ERROR_DECODING_ERROR;
    }

    m_decodeCtx->DecodeParams.m_executeCallIndex++;

    (&(m_decodeCtx->RTtbl))->pCurrentRT = nullptr;

    status = m_decodeCtx->pCodecHal->EndFrame();
    if (status != MOS_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_DECODING_ERROR;
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (ExtraDownScaling(ctx,context) != VA_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_DECODING_ERROR;
    }
#endif

    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecodeBase::CreateBuffer(
    VABufferType type,
    uint32_t     size,
    uint32_t     numElements,
    void         *data,
    VABufferID   *bufId)
{
    DDI_CODEC_FUNC_ENTER;

    DDI_MEDIA_BUFFER                *buf = nullptr;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT  bufferHeapElement;
    uint16_t                        segMapWidth  = m_picWidthInMB;
    uint16_t                        segMapHeight = m_picHeightInMB;
    MOS_STATUS                      status = MOS_STATUS_SUCCESS;
    VAStatus                        va = VA_STATUS_SUCCESS;

    // only for VASliceParameterBufferType of buffer, the number of elements can be greater than 1
    if (type != VASliceParameterBufferType && numElements > 1)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    buf = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    if (buf == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    buf->iSize         = size * numElements;
    buf->uiNumElements = numElements;
    buf->uiType        = type;
    buf->format        = Media_Format_Buffer;
    buf->uiOffset      = 0;
    buf->bCFlushReq    = false;
    buf->pMediaCtx     = m_decodeCtx->pMediaCtx;

    switch ((int32_t)type)
    {
        case VABitPlaneBufferType:
            buf->pData = (uint8_t*)((m_decodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.pBitPlaneBuffer));
            break;
        case VASliceDataBufferType:
        case VAProtectedSliceDataBufferType:
            va = AllocBsBuffer(&(m_decodeCtx->BufMgr), buf);
            if (va != VA_STATUS_SUCCESS)
            {
                MOS_FreeMemory(buf);
                return va;
            }
            break;
        case VASliceParameterBufferType:
            va = AllocSliceControlBuffer(buf);
            if (va != VA_STATUS_SUCCESS)
            {
                MOS_FreeMemory(buf);
                return va;
            }
            buf->format = Media_Format_CPU;
            break;
        case VAPictureParameterBufferType:
            buf->pData  = GetPicParamBuf(&(m_decodeCtx->BufMgr));
            buf->format = Media_Format_CPU;
            break;
        case VASubsetsParameterBufferType:
            buf->pData  = (uint8_t*)MOS_AllocAndZeroMemory(size * numElements);
            buf->format = Media_Format_CPU;
            break;
        case VAIQMatrixBufferType:
            buf->pData  = (uint8_t*)MOS_AllocAndZeroMemory(size * numElements);
            buf->format = Media_Format_CPU;
            break;
        case VAProbabilityBufferType:
            buf->pData  = (uint8_t*)(&(m_decodeCtx->BufMgr.Codec_Param.Codec_Param_VP8.ProbabilityDataVP8));
            break;
        case VAProcFilterParameterBufferType:
            buf->pData  = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineCaps));
            buf->format = Media_Format_CPU;
            break;
        case VAProcPipelineParameterBufferType:
            buf->pData  = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineParameterBuffer));
            buf->format = Media_Format_CPU;
            break;
        case VADecodeStreamoutBufferType:
        {
            segMapHeight = ((segMapHeight + 1) >> 1);   // uiSize must be equal and bigger than size for interlaced case

            if (size < MOS_ALIGN_CEIL(segMapHeight * segMapWidth * CODEC_SIZE_MFX_STREAMOUT_DATA, 64))
            {
                va = VA_STATUS_ERROR_INVALID_PARAMETER;
                MOS_FreeMemory(buf);
                return va;
            }
            buf->iSize  = size * numElements;
            buf->format = Media_Format_Buffer;
            va = MediaLibvaUtilNext::CreateBuffer(buf, m_decodeCtx->pMediaCtx->pDrmBufMgr);
            if (va != VA_STATUS_SUCCESS)
            {
                MOS_FreeMemory(buf);
                return va;
            }
            break;
        }
        case VAHuffmanTableBufferType:
            buf->pData  = (uint8_t*)MOS_AllocAndZeroMemory(size * numElements);
            buf->format = Media_Format_CPU;
            break;
#if VA_CHECK_VERSION(1, 10, 0)
        case VAContextParameterUpdateBufferType:
        {
            buf->pData  = (uint8_t*)MOS_AllocAndZeroMemory(size * numElements);
            buf->format = Media_Format_CPU;
            break;
        }
#endif
        default:
            va = m_decodeCtx->pCpDdiInterfaceNext->CreateBuffer(type, buf, size, numElements);
            if (va == VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE)
            {
                DDI_CODEC_ASSERTMESSAGE("DDI:Decode CreateBuffer unsuppoted buffer type.");
                buf->pData  = (uint8_t*)MOS_AllocAndZeroMemory(size * numElements);
                buf->format = Media_Format_CPU;
                if (buf->pData != NULL)
                {
                    va = VA_STATUS_SUCCESS;
                }
            }
            break;
    }

    bufferHeapElement = MediaLibvaUtilNext::AllocPMediaBufferFromHeap(m_decodeCtx->pMediaCtx->pBufferHeap);
    if (nullptr == bufferHeapElement)
    {
        va = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        MOS_FreeMemory(buf);
        return va;
    }
    bufferHeapElement->pBuffer   = buf;
    bufferHeapElement->pCtx      = (void*)m_decodeCtx;
    bufferHeapElement->uiCtxType = DDI_MEDIA_CONTEXT_TYPE_DECODER;
    *bufId                       = bufferHeapElement->uiVaBufferID;

    // Keep record the VaBufferID of JPEG slice data buffer we allocated, in order to do buffer mapping when render this buffer. otherwise we
    // can not get correct buffer address when application create them disordered.
    if (type == VASliceDataBufferType && m_decodeCtx->wMode == CODECHAL_DECODE_MODE_JPEG)
    {
        // since the dwNumSliceData already +1 when allocate buffer, but here we need to track the VaBufferID before dwSliceData increased.
        m_decodeCtx->BufMgr.pSliceData[m_decodeCtx->BufMgr.dwNumSliceData - 1].vaBufferId = *bufId;
    }
    m_decodeCtx->pMediaCtx->uiNumBufs++;

    if (data == nullptr)
    {
        return va;
    }

    if (true == buf->bCFlushReq)
    {
        mos_bo_wait_rendering(buf->bo);
    }
    status = MOS_SecureMemcpy((void *)(buf->pData + buf->uiOffset), size * numElements, data, size * numElements);
    DDI_CHK_CONDITION((status != MOS_STATUS_SUCCESS), "DDI:Failed to copy buffer data!", VA_STATUS_ERROR_OPERATION_FAILED);

    return va;
}

void DdiDecodeBase::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    DDI_CODEC_FUNC_ENTER;

    m_width         = picWidth;
    m_height        = picHeight;
    m_picWidthInMB  = (int16_t)(DDI_CODEC_NUM_MACROBLOCKS_WIDTH(picWidth));
    m_picHeightInMB = (int16_t)(DDI_CODEC_NUM_MACROBLOCKS_HEIGHT(picHeight));
    m_decodeCtx->wMode = CODECHAL_DECODE_MODE_AVCVLD;
    m_decodeCtx->bShortFormatInUse = false;
#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_ddiDecodeAttr->componentData.data.processType == VA_DEC_PROCESSING)
    {
        DDI_CODEC_NORMALMESSAGE("Decoding context has scaling/format conversion capabilities");
        m_decProcessingType = VA_DEC_PROCESSING;
    }
    else
#endif
    {
        DDI_CODEC_NORMALMESSAGE("Decoding context DOESN'T have scaling/format conversion capabilities");
        m_decProcessingType = VA_DEC_PROCESSING_NONE;
    }
    m_streamOutEnabled = false;
    m_decodeCtx->DecodeParams.m_picIdRemappingInUse = true;
    return;
}

VAStatus DdiDecodeBase::CreateCodecHal(
    DDI_MEDIA_CONTEXT       *mediaCtx,
    void                    *ptr,
    _CODECHAL_STANDARD_INFO *standardInfo)
{
    DDI_CODEC_FUNC_ENTER;

    if ((mediaCtx == nullptr) ||
        (ptr == nullptr) ||
        (m_codechalSettings == nullptr) ||
        (standardInfo == nullptr))
    {
        DDI_CODEC_ASSERTMESSAGE("NULL pointer is passed for CreateCodecHal.\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    }
    MOS_CONTEXT *mosCtx  = (MOS_CONTEXT *)ptr;
    VAStatus    vaStatus = VA_STATUS_SUCCESS;

    Codechal *codecHal = CodechalDeviceNext::CreateFactory(
        nullptr,
        mosCtx,
        standardInfo,
        m_codechalSettings);

    if (nullptr == codecHal)
    {
        DDI_CODEC_ASSERTMESSAGE("Failure in CodecHal create.\n");
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    DecodePipelineAdapter *decoder = dynamic_cast<DecodePipelineAdapter *>(codecHal);
    if (nullptr == decoder)
    {
        DDI_CODEC_ASSERTMESSAGE("Failure in CodecHal create.\n");
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    m_decodeCtx->pCodecHal = codecHal;

    m_codechalSettings->sfcInUseHinted = true;

    if (m_ddiDecodeAttr && m_ddiDecodeAttr->componentData.data.encryptType)
    {
        m_codechalSettings->secureMode = true;
    }

    if (codecHal->Allocate(m_codechalSettings) != MOS_STATUS_SUCCESS)
    {
        DDI_CODEC_ASSERTMESSAGE("Failure in decode allocate.\n");
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    PMOS_INTERFACE osInterface = codecHal->GetOsInterface();
    if (osInterface == nullptr)
    {
        DDI_CODEC_ASSERTMESSAGE("Failure in decode allocate.\n");
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }

    m_decodeCtx->pCpDdiInterfaceNext->CreateCencDecode(codecHal->GetDebugInterface(), mosCtx, m_codechalSettings);

    return vaStatus;
}

void DdiDecodeBase::GetDummyReferenceFromDPB(
    DDI_DECODE_CONTEXT *decodeCtx)
{
    DDI_CODEC_FUNC_ENTER;

    MOS_SURFACE *destSurface = decodeCtx->DecodeParams.m_destSurface;
    MOS_SURFACE dummyReference;
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    uint32_t    i = 0;

    if (destSurface == nullptr)
    {
        DDI_CODEC_ASSERTMESSAGE("Decode output surface is NULL.\n");
        return;
    }

    for (i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i++)
    {
        if (decodeCtx->RTtbl.pRT[i] != nullptr && 
            decodeCtx->RTtbl.pRT[i] != decodeCtx->RTtbl.pCurrentRT)
        {
            MOS_ZeroMemory(&dummyReference, sizeof(MOS_SURFACE));
            MediaLibvaCommonNext::MediaSurfaceToMosResource(decodeCtx->RTtbl.pRT[i], &(dummyReference.OsResource));

            if (!Mos_ResourceIsNull(&dummyReference.OsResource))
            {
                eStatus = MOS_STATUS_SUCCESS;
                dummyReference.Format = Format_Invalid;
                eStatus = Mos_Specific_GetResourceInfo(decodeCtx->pCodecHal->GetOsInterface(), &dummyReference.OsResource, &dummyReference);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    continue;
                }

                if (dummyReference.Type   == destSurface->Type   && 
                    dummyReference.Format == destSurface->Format && 
                    dummyReference.bIsCompressed   == destSurface->bIsCompressed   && 
                    dummyReference.CompressionMode == destSurface->CompressionMode && 
                    dummyReference.TileType == destSurface->TileType && 
                    dummyReference.dwPitch  >= destSurface->dwPitch  && 
                    dummyReference.dwWidth  >= destSurface->dwWidth  && 
                    dummyReference.dwHeight >= destSurface->dwHeight)
                {
                    break;
                }
            }
        }
    }

    if (i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        DecodePipelineAdapter *decoder = dynamic_cast<DecodePipelineAdapter *>(decodeCtx->pCodecHal);
        if (decoder == nullptr)
        {
            DDI_CODEC_ASSERTMESSAGE("Codechal decode context is NULL.\n");
            return;
        }
        decoder->GetDummyReference()->OsResource = dummyReference.OsResource;
    }

    return;
}

void DdiDecodeBase::ReportDecodeMode(
    uint16_t wMode)
{
    DDI_CODEC_FUNC_ENTER;

    PMOS_INTERFACE     osInterface = m_decodeCtx->pCodecHal ? m_decodeCtx->pCodecHal->GetOsInterface() : nullptr;
    MOS_CONTEXT_HANDLE ctxHandle   = osInterface ? (MOS_CONTEXT_HANDLE)osInterface->pOsContext : nullptr;

    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
    MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
    userFeatureWriteData.Value.i32Data = wMode;
    switch (wMode)
    {
    case CODECHAL_DECODE_MODE_MPEG2IDCT:
    case CODECHAL_DECODE_MODE_MPEG2VLD:
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_MPEG2_MODE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, ctxHandle);
        break;
    case CODECHAL_DECODE_MODE_AVCVLD:
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_AVC_MODE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, ctxHandle);
        break;
    case CODECHAL_DECODE_MODE_JPEG:
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_JPEG_MODE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, ctxHandle);
        break;
    case CODECHAL_DECODE_MODE_VP8VLD:
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_VP8_MODE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, ctxHandle);
        break;
    case CODECHAL_DECODE_MODE_HEVCVLD:
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_HEVC_MODE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, ctxHandle);
        break;
    case CODECHAL_DECODE_MODE_VP9VLD:
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_VP9_MODE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, ctxHandle);
        break;
    case CODECHAL_DECODE_MODE_AV1VLD:
        userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_AV1_MODE_ID;
        MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, ctxHandle);
        break;
    default:
        break;
    }
#if MOS_EVENT_TRACE_DUMP_SUPPORTED
    {
        DECODE_EVENTDATA_VA_FEATURE_REPORTMODE eventData;
        eventData.wMode = (uint32_t)wMode;
        eventData.ValueID = userFeatureWriteData.ValueID;
        MOS_TraceEvent(EVENT_DECODE_FEATURE_DECODEMODE_REPORTVA, EVENT_TYPE_INFO, &eventData, sizeof(eventData), NULL, 0);
    }
#endif
    return;
}
} // namespace decode
