/*
* Copyright (c) 2017-2018, Intel Corporation
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
#include "media_libva_vp.h"
#include "media_libva_util.h"
#include "media_ddi_decode_base.h"
#include "codechal.h"
#include "codechal_memdecomp.h"
#include "media_interfaces_codechal.h"
#include "media_interfaces_mmd.h"
#include "mos_solo_generic.h"

DdiMediaDecode::DdiMediaDecode(DDI_DECODE_CONFIG_ATTR *ddiDecodeAttr)
    : DdiMediaBase()
{
    m_ddiDecodeAttr = ddiDecodeAttr;
    m_ddiDecodeCtx  = nullptr;
    MOS_ZeroMemory(&m_destSurface, sizeof(m_destSurface));
    m_groupIndex = 0;
    m_picWidthInMB = 0;
    m_picHeightInMB = 0;
    m_decProcessingType = 0;
    m_width = 0;
    m_height = 0;
    m_streamOutEnabled = false;
    m_sliceParamBufNum = 0;
    m_sliceCtrlBufNum = 0;
    m_codechalSettings = CodechalSetting::CreateCodechalSetting();
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
        MOS_SecureMemcpy(m_ddiDecodeAttr, sizeof(DDI_DECODE_CONFIG_ATTR), ddiConfAttr, sizeof(DDI_DECODE_CONFIG_ATTR));
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

    DDI_CHK_NULL(procBuf, "nullptr Processing buffer", VA_STATUS_ERROR_INVALID_PARAMETER)

    if (m_decProcessingType == VA_DEC_PROCESSING)
    {
        if(m_procBuf == nullptr)
        {
            m_procBuf = (VAProcPipelineParameterBuffer*)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineParameterBuffer));
            DDI_CHK_NULL(m_procBuf, "nullptr m_procBuf", VA_STATUS_ERROR_ALLOCATION_FAILED);
            MOS_SecureMemcpy(m_procBuf, sizeof(VAProcPipelineParameterBuffer), procBuf, sizeof(VAProcPipelineParameterBuffer));
        }
        auto decProcessingParams =
            (PCODECHAL_DECODE_PROCESSING_PARAMS)m_ddiDecodeCtx->DecodeParams.m_procParams;

        auto decProcessingSurface = decProcessingParams->pOutputSurface;

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
    DDI_FUNCTION_ENTER();

    PDDI_MEDIA_CONTEXT mediaCtx;

    /* As it is already checked in the upper caller, skip the check */
    mediaCtx = DdiMedia_GetMediaContext(ctx);

#ifdef _DECODE_PROCESSING_SUPPORTED
    //renderTarget is decode output surface; set renderTarget as vp sfc input surface m_procBuf->surface = rederTarget
    if(m_procBuf)
    {
        m_procBuf->surface = renderTarget;
    }
#endif

    DDI_MEDIA_SURFACE *curRT;
    curRT = (DDI_MEDIA_SURFACE *)DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, renderTarget);
    DDI_CHK_NULL(curRT, "nullptr pCurRT", VA_STATUS_ERROR_INVALID_SURFACE);
    curRT->pDecCtx = m_ddiDecodeCtx;

    DDI_CODEC_RENDER_TARGET_TABLE *RTtbl;
    RTtbl             = &(m_ddiDecodeCtx->RTtbl);
    RTtbl->pCurrentRT = curRT;

    m_streamOutEnabled              = false;
    m_ddiDecodeCtx->DecodeParams.m_numSlices       = 0;
    m_ddiDecodeCtx->DecodeParams.m_dataSize        = 0;
    m_ddiDecodeCtx->DecodeParams.m_deblockDataSize = 0;
    m_ddiDecodeCtx->DecodeParams.m_executeCallIndex = 0;
    m_groupIndex                                   = 0;

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

    uint32_t slcInd;
    //copy data to new bit stream
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
        Delete_DdiCpInterface(m_ddiDecodeCtx->pCpDdiInterface);
        m_ddiDecodeCtx->pCpDdiInterface = nullptr;
    }

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer);
    m_ddiDecodeCtx->DecodeParams.m_iqMatrixBuffer = nullptr;

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_huffmanTable);
    m_ddiDecodeCtx->DecodeParams.m_huffmanTable = nullptr;

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_picParams);
    m_ddiDecodeCtx->DecodeParams.m_picParams = nullptr;

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_sliceParams);
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = nullptr;

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_extPicParams);
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = nullptr;

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_advPicParams);
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = nullptr;

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_extSliceParams);
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = nullptr;

    MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_subsetParams);
    m_ddiDecodeCtx->DecodeParams.m_sliceParams = nullptr;

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_ddiDecodeCtx->DecodeParams.m_procParams != nullptr)
    {
        auto procParams =
            (PCODECHAL_DECODE_PROCESSING_PARAMS)m_ddiDecodeCtx->DecodeParams.m_procParams;
        MOS_FreeMemory(procParams->pOutputSurface);
        procParams->pOutputSurface = nullptr;

        MOS_FreeMemory(m_ddiDecodeCtx->DecodeParams.m_procParams);
        m_ddiDecodeCtx->DecodeParams.m_procParams = nullptr;
    }
#endif

    return;
}

int32_t DdiMediaDecode::GetBitstreamBufIndexFromBuffer(DDI_CODEC_COM_BUFFER_MGR *bufMgr, DDI_MEDIA_BUFFER *buf)
{
    int32_t i;
    for(i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        if(bufMgr->pBitStreamBuffObject[i]->bo == buf->bo)
        {
            return i;
        }
    }

    return DDI_CODEC_INVALID_BUFFER_INDEX;
}

VAStatus DdiMediaDecode::AllocBsBuffer(
    DDI_CODEC_COM_BUFFER_MGR    *bufMgr,
    DDI_MEDIA_BUFFER            *buf)
{
    uint32_t          index, i;
    VAStatus          vaStatus;
    uint8_t          *sliceBuf;
    DDI_MEDIA_BUFFER *bsBufObj = nullptr;
    uint8_t          *bsBufBaseAddr = nullptr;
    bool              createBsBuffer = false;

    if ( nullptr == bufMgr || nullptr == buf || nullptr == (m_ddiDecodeCtx->pMediaCtx) )
    {
        DDI_ASSERTMESSAGE("invalidate input parameters.");
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    index       = bufMgr->dwNumSliceData;
    vaStatus    = VA_STATUS_SUCCESS;
    sliceBuf   = nullptr;

    /* the pSliceData needs to be reallocated in order to contain more SliceDataBuf */
    if (index >= bufMgr->m_maxNumSliceData)
    {
        /* In theroy it can resize the m_maxNumSliceData one by one. But in order to
         * avoid calling realloc frequently, it will try to allocate 10 to  hold more
         * SliceDataBuf. This is only for the optimized purpose.
         */
        int32_t reallocSize = bufMgr->m_maxNumSliceData + 10;

        bufMgr->pSliceData = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)realloc(bufMgr->pSliceData, sizeof(bufMgr->pSliceData[0]) * reallocSize);

        if (bufMgr->pSliceData == nullptr)
        {
            DDI_ASSERTMESSAGE("fail to reallocate pSliceData\n.");
            return VA_STATUS_ERROR_ALLOCATION_FAILED;
        }
        memset(bufMgr->pSliceData + bufMgr->m_maxNumSliceData, 0,
               sizeof(bufMgr->pSliceData[0]) * 10);

        bufMgr->m_maxNumSliceData += 10;
    }

    if(index >= 1)
    {
        buf->uiOffset = bufMgr->pSliceData[index-1].uiOffset + bufMgr->pSliceData[index-1].uiLength;
        if((buf->uiOffset + buf->iSize) > bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]->iSize)
        {
            sliceBuf = (uint8_t*)MOS_AllocAndZeroMemory(buf->iSize);
            if(sliceBuf == nullptr)
            {
                DDI_ASSERTMESSAGE("DDI:AllocAndZeroMem return failure.")
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
                    //find a bitstream buffer whoes graphic memory is allocated but not used by HW now.
                    break;
                }
            }
            else
            {
                //find a new bitstream buffer whoes graphic memory is not allocated yet
                break;
            }
        }

        if (i == DDI_CODEC_MAX_BITSTREAM_BUFFER)
        {
            //find the oldest bistream buffer which is the most possible one to become free in the shortest time.
            bufMgr->dwBitstreamIndex = (bufMgr->ui64BitstreamOrder >> (DDI_CODEC_BITSTREAM_BUFFER_INDEX_BITS * DDI_CODEC_MAX_BITSTREAM_BUFFER_MINUS1)) & DDI_CODEC_MAX_BITSTREAM_BUFFER_INDEX;
            // wait until decode complete
            mos_bo_wait_rendering(bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]->bo);
        }
        else
        {
            bufMgr->dwBitstreamIndex = i;
        }
        bufMgr->ui64BitstreamOrder = (bufMgr->ui64BitstreamOrder << 4) + bufMgr->dwBitstreamIndex;

        bsBufObj                   = bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex];
        bsBufObj ->pMediaCtx       = m_ddiDecodeCtx->pMediaCtx;
        bsBufBaseAddr              = bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex];

        if(bsBufBaseAddr == nullptr)
        {
            createBsBuffer = true;
            if (buf->iSize > bsBufObj->iSize)
            {
                bsBufObj->iSize = buf->iSize;
            }
        }
        else if(buf->iSize > bsBufObj->iSize)
        {
           //free bo
            DdiMediaUtil_UnlockBuffer(bsBufObj);
            DdiMediaUtil_FreeBuffer(bsBufObj);
            bsBufBaseAddr = nullptr;

            createBsBuffer = true;
            bsBufObj->iSize = buf->iSize;
        }

        if (createBsBuffer)
        {
            if(VA_STATUS_SUCCESS != DdiMediaUtil_CreateBuffer(bsBufObj, m_ddiDecodeCtx->pMediaCtx->pDrmBufMgr))
            {
               return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }

            bsBufBaseAddr = (uint8_t*)DdiMediaUtil_LockBuffer(bsBufObj, MOS_LOCKFLAG_WRITEONLY);
            if(bsBufBaseAddr == nullptr)
            {
                DdiMediaUtil_FreeBuffer(bsBufObj);
                return VA_STATUS_ERROR_ALLOCATION_FAILED;
            }
            bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex] = bsBufBaseAddr;
        }
    }

    if(bufMgr->pBitStreamBase[bufMgr->dwBitstreamIndex] == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    bufMgr->pSliceData[index].uiLength = buf->iSize;
    bufMgr->pSliceData[index].uiOffset = buf->uiOffset;

    if(bufMgr->bIsSliceOverSize == true)
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
    buf->bo                            = bufMgr->pBitStreamBuffObject[bufMgr->dwBitstreamIndex]->bo;

    return VA_STATUS_SUCCESS;
}

MOS_FORMAT DdiMediaDecode::GetFormat()
{
    return  Format_NV12;
}

VAStatus DdiMediaDecode::InitDecodeParams(
    VADriverContextP ctx,
    VAContextID      context)
{
    /* skip the mediaCtx check as it is checked in caller */
    PDDI_MEDIA_CONTEXT mediaCtx;
    mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_RET(DecodeCombineBitstream(mediaCtx),"DecodeCombineBitstream failed!");
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);
    bufMgr->dwNumSliceData    = 0;
    bufMgr->dwNumSliceControl = 0;
    memset(&m_destSurface, 0, sizeof(MOS_SURFACE));
    m_destSurface.dwOffset = 0;

    DDI_CODEC_RENDER_TARGET_TABLE *rtTbl = &(m_ddiDecodeCtx->RTtbl);

    if ((rtTbl == nullptr) || (rtTbl->pCurrentRT == nullptr))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }
    return VA_STATUS_SUCCESS;

}

VAStatus DdiMediaDecode::SetDecodeParams()
{
    DDI_CODEC_COM_BUFFER_MGR *bufMgr = &(m_ddiDecodeCtx->BufMgr);
    if ((&m_ddiDecodeCtx->DecodeParams)->m_numSlices == 0)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    MOS_FORMAT expectedFormat = GetFormat();
    m_destSurface.Format   = expectedFormat;
    DdiMedia_MediaSurfaceToMosResource((&(m_ddiDecodeCtx->RTtbl))->pCurrentRT, &(m_destSurface.OsResource));

    if (m_destSurface.OsResource.Format != expectedFormat)
    {
        DDI_NORMALMESSAGE("Surface fomrat of decoded surface is inconsistent with Codec bitstream\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_ddiDecodeCtx->DecodeParams.m_destSurface = &m_destSurface;
    m_ddiDecodeCtx->DecodeParams.m_deblockSurface = nullptr;

    m_ddiDecodeCtx->DecodeParams.m_dataBuffer    = &bufMgr->resBitstreamBuffer;
    m_ddiDecodeCtx->DecodeParams.m_bitStreamBufData = bufMgr->pBitstreamBuffer;

    m_ddiDecodeCtx->DecodeParams.m_bitplaneBuffer = nullptr;

    if (m_streamOutEnabled)
    {
        m_ddiDecodeCtx->DecodeParams.m_streamOutEnabled           = true;
        m_ddiDecodeCtx->DecodeParams.m_externalStreamOutBuffer    = &bufMgr->resExternalStreamOutBuffer;
    }
    else
    {
        m_ddiDecodeCtx->DecodeParams.m_streamOutEnabled           = false;
        m_ddiDecodeCtx->DecodeParams.m_externalStreamOutBuffer    = nullptr;
    }

    if (m_ddiDecodeCtx->pCpDdiInterface)
    {
        DDI_CHK_RET(m_ddiDecodeCtx->pCpDdiInterface->SetDecodeParams(&m_ddiDecodeCtx->DecodeParams),"SetDecodeParams failed!");
    }

    Mos_Solo_OverrideBufferSize(m_ddiDecodeCtx->DecodeParams.m_dataSize, m_ddiDecodeCtx->DecodeParams.m_dataBuffer);

    return VA_STATUS_SUCCESS;
}

VAStatus DdiMediaDecode::ExtraDownScaling(
        VADriverContextP         ctx,
        VAContextID              context)
{
#ifdef _DECODE_PROCESSING_SUPPORTED
    DDI_CHK_NULL(ctx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    VAStatus vaStatus = MOS_STATUS_SUCCESS;
    PDDI_MEDIA_CONTEXT mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(m_ddiDecodeCtx, "nullptr ctx", VA_STATUS_ERROR_INVALID_CONTEXT);
    CodechalDecode *decoder = dynamic_cast<CodechalDecode *>(m_ddiDecodeCtx->pCodecHal);
    DDI_CHK_NULL(decoder, "nullptr decoder", VA_STATUS_ERROR_INVALID_PARAMETER);
    if(m_ddiDecodeCtx->DecodeParams.m_procParams &&
        !decoder->IsVdSfcSupported())
    {
        //check vp context
        VAContextID vpCtxID = VA_INVALID_ID;
        if (mediaCtx->pVpCtxHeap != nullptr && mediaCtx->pVpCtxHeap->pHeapBase != nullptr)
        {
            //Get VP Context from heap.
            vpCtxID = (VAContextID)(0 + DDI_MEDIA_VACONTEXTID_OFFSET_VP);
        }
        else
        {
            //Create VP Context.
            vaStatus = DdiVp_CreateContext(ctx, 0, 0, 0, 0, 0, 0, &vpCtxID);
            DDI_CHK_RET(vaStatus, "Create VP Context failed.");
        }

        uint32_t        ctxType;
        PDDI_VP_CONTEXT pVpCtx = (PDDI_VP_CONTEXT)DdiMedia_GetContextFromContextID(ctx, vpCtxID, &ctxType);
        DDI_CHK_NULL(pVpCtx, "nullptr pVpCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

        //Set parameters
        VAProcPipelineParameterBuffer* pInputPipelineParam = m_procBuf;
        DDI_CHK_NULL(pInputPipelineParam, "nullptr pInputPipelineParam", VA_STATUS_ERROR_ALLOCATION_FAILED);

        vaStatus = DdiVp_BeginPicture(ctx, vpCtxID, pInputPipelineParam->additional_outputs[0]);
        DDI_CHK_RET(vaStatus, "VP BeginPicture failed");

        vaStatus = DdiVp_SetProcPipelineParams(ctx, pVpCtx, pInputPipelineParam);
        DDI_CHK_RET(vaStatus, "VP SetProcPipelineParams failed.");

        vaStatus = DdiVp_EndPicture(ctx, vpCtxID);
        DDI_CHK_RET(vaStatus, "VP EndPicture failed.");
    }
#endif
    return MOS_STATUS_SUCCESS;
}

VAStatus DdiMediaDecode::InitDummyReference(CodechalDecode& decoder)
{
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
        if (dummyReference->dwWidth < m_ddiDecodeCtx->DecodeParams.m_destSurface->dwWidth ||
            dummyReference->dwHeight < m_ddiDecodeCtx->DecodeParams.m_destSurface->dwHeight)
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
            //GetDummyReferenceFromDPB(m_ddiDecodeCtx);

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
        //GetDummyReferenceFromDPB(m_ddiDecodeCtx);
        //if (!Mos_ResourceIsNull(&dummyReference->OsResource))
        //{
        //    decoder->SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_DPB);
        //}
    }

    return VA_STATUS_SUCCESS;
}

VAStatus DdiMediaDecode::EndPicture(
    VADriverContextP ctx,
    VAContextID      context)
{
    DDI_FUNCTION_ENTER();

    if (m_ddiDecodeCtx->bDecodeModeReported == false)
    {
        ReportDecodeMode(m_ddiDecodeCtx->wMode);
        m_ddiDecodeCtx->bDecodeModeReported = true;
    }

    DDI_CHK_RET(InitDecodeParams(ctx,context),"InitDecodeParams failed!");

    DDI_CHK_RET(SetDecodeParams(), "SetDecodeParams failed!");
    DDI_CHK_RET(ClearRefList(&(m_ddiDecodeCtx->RTtbl), m_withDpb), "ClearRefList failed!");
    if (m_ddiDecodeCtx->pCodecHal == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }

    if (MEDIA_IS_WA(&m_ddiDecodeCtx->pMediaCtx->WaTable, WaDummyReference))
    {
        Mos_Specific_GetResourceInfo(
            m_ddiDecodeCtx->pCodecHal->GetOsInterface(), 
            &m_ddiDecodeCtx->DecodeParams.m_destSurface->OsResource,
            m_ddiDecodeCtx->DecodeParams.m_destSurface);

        CodechalDecode *decoder = dynamic_cast<CodechalDecode *>(m_ddiDecodeCtx->pCodecHal);
        DDI_CHK_NULL(decoder, "Null decoder", VA_STATUS_ERROR_INVALID_PARAMETER);
        DDI_CHK_RET(InitDummyReference(*decoder), "InitDummyReference failed!");
    }

    MOS_STATUS status = m_ddiDecodeCtx->pCodecHal->Execute((void *)(&m_ddiDecodeCtx->DecodeParams));
    if (status != MOS_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("DDI:DdiDecode_DecodeInCodecHal return failure.");
        return VA_STATUS_ERROR_DECODING_ERROR;
    }

    m_ddiDecodeCtx->DecodeParams.m_executeCallIndex++;

    (&(m_ddiDecodeCtx->RTtbl))->pCurrentRT = nullptr;

    status = m_ddiDecodeCtx->pCodecHal->EndFrame();
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

    DDI_FUNCTION_EXIT(VA_STATUS_SUCCESS);
    return VA_STATUS_SUCCESS;
}

VAStatus DdiMediaDecode::CreateBuffer(
    VABufferType             type,
    uint32_t                 size,
    uint32_t                 numElements,
    void                    *data,
    VABufferID              *bufId)
{
    DDI_MEDIA_BUFFER                *buf;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT   bufferHeapElement;
    uint16_t                         segMapWidth, segMapHeight;
    MOS_STATUS                       status = MOS_STATUS_SUCCESS;
    VAStatus                         va = VA_STATUS_SUCCESS;

    segMapWidth = m_picWidthInMB;
    segMapHeight= m_picHeightInMB;

    // only for VASliceParameterBufferType of buffer, the number of elements can be greater than 1
    if(type != VASliceParameterBufferType && numElements > 1)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    buf               = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
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
    buf->pMediaCtx     = m_ddiDecodeCtx->pMediaCtx;

    switch ((int32_t)type)
    {
        case VABitPlaneBufferType:
            buf->pData = (uint8_t*)((m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VC1.pBitPlaneBuffer));
            break;
        case VASliceDataBufferType:
        case VAProtectedSliceDataBufferType:
            va = AllocBsBuffer(&(m_ddiDecodeCtx->BufMgr), buf);
            if(va != VA_STATUS_SUCCESS)
            {
                goto CleanUpandReturn;
            }

            break;
        case VASliceParameterBufferType:
            va = AllocSliceControlBuffer(buf);
            if(va != VA_STATUS_SUCCESS)
            {
                goto CleanUpandReturn;
            }
            buf->format     = Media_Format_CPU;
            break;
        case VAPictureParameterBufferType:
            buf->pData      = GetPicParamBuf(&(m_ddiDecodeCtx->BufMgr));
            buf->format     = Media_Format_CPU;
            break;
        case VASubsetsParameterBufferType:
            //maximum entry point supported should not be more than 440
            if(numElements > 440)
            {
                va = VA_STATUS_ERROR_INVALID_PARAMETER;
                goto CleanUpandReturn;
            }
            buf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(size * numElements);
            buf->format     = Media_Format_CPU;
            break;
        case VAIQMatrixBufferType:
            buf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(size * numElements);
            buf->format     = Media_Format_CPU;
            break;
        case VAProbabilityBufferType:
            buf->pData      = (uint8_t*)(&(m_ddiDecodeCtx->BufMgr.Codec_Param.Codec_Param_VP8.ProbabilityDataVP8));
            break;
        case VAProcFilterParameterBufferType:
            buf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineCaps));
            buf->format     = Media_Format_CPU;
            break;
        case VAProcPipelineParameterBufferType:
            buf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineParameterBuffer));
            buf->format     = Media_Format_CPU;
            break;
        case VADecodeStreamoutBufferType:
        {
            segMapHeight = ((segMapHeight + 1) >> 1);   //uiSize must be equal and bigger than size for interlaced case

            if (size < MOS_ALIGN_CEIL(segMapHeight * segMapWidth * CODEC_SIZE_MFX_STREAMOUT_DATA, 64))
            {
                va = VA_STATUS_ERROR_INVALID_PARAMETER;
                goto CleanUpandReturn;
            }
            buf->iSize  = size * numElements;
            buf->format = Media_Format_Buffer;
            va = DdiMediaUtil_CreateBuffer(buf, m_ddiDecodeCtx->pMediaCtx->pDrmBufMgr);
            if(va != VA_STATUS_SUCCESS)
            {
                goto CleanUpandReturn;
            }
            break;
        }
        case VAHuffmanTableBufferType:
            buf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(size * numElements);
            buf->format     = Media_Format_CPU;
            break;
        default:
            va = m_ddiDecodeCtx->pCpDdiInterface->CreateBuffer(type, buf, size, numElements);
            if (va  == VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE)
            {
                MOS_FreeMemory(buf);
                return va;
            }
            break;
    }

    bufferHeapElement  = DdiMediaUtil_AllocPMediaBufferFromHeap(m_ddiDecodeCtx->pMediaCtx->pBufferHeap);
    if (nullptr == bufferHeapElement)
    {
        va = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        goto CleanUpandReturn;
    }
    bufferHeapElement->pBuffer      = buf;
    bufferHeapElement->pCtx         = (void*)m_ddiDecodeCtx;
    bufferHeapElement->uiCtxType    = DDI_MEDIA_CONTEXT_TYPE_DECODER;
    *bufId                          = bufferHeapElement->uiVaBufferID;

    // Keep record the VaBufferID of JPEG slice data buffer we allocated, in order to do buffer mapping when render this buffer. otherwise we
    // can not get correct buffer address when application create them disordered.
    if (type == VASliceDataBufferType && m_ddiDecodeCtx->wMode == CODECHAL_DECODE_MODE_JPEG)
    {
        // since the dwNumSliceData already +1 when allocate buffer, but here we need to track the VaBufferID before dwSliceData increased.
        m_ddiDecodeCtx->BufMgr.pSliceData[m_ddiDecodeCtx->BufMgr.dwNumSliceData - 1].vaBufferId = *bufId;
    }
    m_ddiDecodeCtx->pMediaCtx->uiNumBufs++;

    if(data == nullptr)
    {
        return va;
    }

    if( true == buf->bCFlushReq )
    {
        mos_bo_subdata(buf->bo, buf->uiOffset, size * numElements, data);
    }
    else
    {
        status = MOS_SecureMemcpy((void *)(buf->pData + buf->uiOffset), size * numElements, data, size * numElements);
        DDI_CHK_CONDITION((status != MOS_STATUS_SUCCESS), "DDI:Failed to copy buffer data!", VA_STATUS_ERROR_OPERATION_FAILED);
    }
    return va;

CleanUpandReturn:
    if(buf)
    {
        MOS_FreeMemory(buf->pData);
        MOS_FreeMemory(buf);
    }
    return va;

}

void DdiMediaDecode::ContextInit(
    int32_t picWidth,
    int32_t picHeight)
{
    m_width           = picWidth;
    m_height          = picHeight;
    m_picWidthInMB     = (int16_t)(DDI_CODEC_NUM_MACROBLOCKS_WIDTH(picWidth));
    m_picHeightInMB    = (int16_t)(DDI_CODEC_NUM_MACROBLOCKS_HEIGHT(picHeight));
    m_ddiDecodeCtx->wMode             = CODECHAL_DECODE_MODE_AVCVLD;
    m_ddiDecodeCtx->bShortFormatInUse = false;
#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_ddiDecodeAttr->uiDecProcessingType == VA_DEC_PROCESSING)
    {
        DDI_NORMALMESSAGE("Decoding context has scaling/format conversion capabilities");
        m_decProcessingType = VA_DEC_PROCESSING;
    }
    else
#endif
    {
        DDI_NORMALMESSAGE("Decoding context DOESN'T have scaling/format conversion capabilities");
        m_decProcessingType = VA_DEC_PROCESSING_NONE;
    }
    m_streamOutEnabled                  = false;
    m_ddiDecodeCtx->DecodeParams.m_picIdRemappingInUse = true;
}

VAStatus DdiMediaDecode::CreateCodecHal(
    DDI_MEDIA_CONTEXT       *mediaCtx,
    void                    *ptr,
    _CODECHAL_STANDARD_INFO *standardInfo)
{
    if ((mediaCtx == nullptr) ||
        (ptr == nullptr) ||
        (m_codechalSettings == nullptr) ||
        (standardInfo == nullptr))
    {
        DDI_ASSERTMESSAGE("NULL pointer is passed for CreateCodecHal.\n");
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    }
    MOS_CONTEXT    *mosCtx   = (MOS_CONTEXT *)ptr;
    VAStatus        vaStatus = VA_STATUS_SUCCESS;

    Codechal *codecHal = CodechalDevice::CreateFactory(
        nullptr,
        mosCtx,
        standardInfo,
        m_codechalSettings);
    CodechalDecode *decoder = dynamic_cast<CodechalDecode *>(codecHal);
    if (nullptr == codecHal || nullptr == decoder)
    {
        DDI_ASSERTMESSAGE("Failure in CodecHal create.\n");
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        return vaStatus;
    }
    m_ddiDecodeCtx->pCodecHal = codecHal;

    m_codechalSettings->enableCodecMmc = false;
    m_codechalSettings->sfcInUseHinted = true;
    if (codecHal->Allocate(m_codechalSettings) != MOS_STATUS_SUCCESS)
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

    m_ddiDecodeCtx->pCpDdiInterface->CreateCencDecode(decoder->GetDebugInterface(), mosCtx, m_codechalSettings);

    return vaStatus;
}

void DdiMediaDecode::GetDummyReferenceFromDPB(
    DDI_DECODE_CONTEXT      *decodeCtx)
{
    MOS_SURFACE     *destSurface = decodeCtx->DecodeParams.m_destSurface;
    MOS_SURFACE     dummyReference;
    MOS_STATUS      eStatus;
    uint32_t        i;

    if (destSurface == nullptr)
    {
        DDI_ASSERTMESSAGE("Decode output surface is NULL.\n");
        return;
    }

    for (i = 0; i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT; i++)
    {
        if (decodeCtx->RTtbl.pRT[i] != nullptr && 
            decodeCtx->RTtbl.pRT[i] != decodeCtx->RTtbl.pCurrentRT)
        {
            MOS_ZeroMemory(&dummyReference, sizeof(MOS_SURFACE));
            DdiMedia_MediaSurfaceToMosResource(decodeCtx->RTtbl.pRT[i], &(dummyReference.OsResource));

            if (!Mos_ResourceIsNull(&dummyReference.OsResource))
            {
                eStatus = MOS_STATUS_SUCCESS;
                dummyReference.Format = Format_Invalid;
                eStatus = Mos_Specific_GetResourceInfo(decodeCtx->pCodecHal->GetOsInterface(), &dummyReference.OsResource, &dummyReference);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    continue;
                }

                if (dummyReference.Type == destSurface->Type && 
                    dummyReference.Format == destSurface->Format && 
                    dummyReference.bIsCompressed == destSurface->bIsCompressed && 
                    dummyReference.CompressionMode == destSurface->CompressionMode && 
                    dummyReference.TileType == destSurface->TileType && 
                    dummyReference.dwPitch >= destSurface->dwPitch && 
                    dummyReference.dwWidth >= destSurface->dwWidth && 
                    dummyReference.dwHeight >= destSurface->dwHeight)
                {
                    break;
                }
            }
        }
    }

    if (i < DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        CodechalDecode *decoder = dynamic_cast<CodechalDecode *>(decodeCtx->pCodecHal);
        if (decoder == nullptr)
        {
            DDI_ASSERTMESSAGE("Codechal decode context is NULL.\n");
            return;
        }
        decoder->GetDummyReference()->OsResource = dummyReference.OsResource;
    }
}

void DdiMediaDecode::ReportDecodeMode(
    uint16_t      wMode)
 {
    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;
    MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
    userFeatureWriteData.Value.i32Data = wMode;
    switch (wMode)
    {
        case CODECHAL_DECODE_MODE_MPEG2IDCT:
        case CODECHAL_DECODE_MODE_MPEG2VLD:
            userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_MPEG2_MODE_ID;
            MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
            break;
        case CODECHAL_DECODE_MODE_VC1IT:
        case CODECHAL_DECODE_MODE_VC1VLD:
            userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_VC1_MODE_ID;
            MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
            break;
        case CODECHAL_DECODE_MODE_AVCVLD:
            userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_AVC_MODE_ID;
            MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
            break;
        case CODECHAL_DECODE_MODE_JPEG:
            userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_JPEG_MODE_ID;
            MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
            break;
        case CODECHAL_DECODE_MODE_VP8VLD:
            userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_VP8_MODE_ID;
            MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
            break;
        case CODECHAL_DECODE_MODE_HEVCVLD:
            userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_HEVC_MODE_ID;
            MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
            break;
        case CODECHAL_DECODE_MODE_VP9VLD:
            userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_DECODE_VP9_MODE_ID;
            MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
            break;
        default:
            break;
    }
 }
