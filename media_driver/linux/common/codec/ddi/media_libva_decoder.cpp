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
#include "media_libva_cp.h"
#include "media_libva_caps.h"
#include "codechal_memdecomp.h"
#include "mos_solo_generic.h"
#include "media_ddi_factory.h"
#include "media_ddi_decode_base.h"
#include "media_interfaces.h"
#include "media_ddi_decode_const.h"

#ifndef ANDROID
#include <X11/Xutil.h>
#endif

#include <linux/fb.h>

typedef MediaDdiFactory<DdiMediaDecode, DDI_DECODE_CONFIG_ATTR> DdiDecodeFactory;

static DdiMediaDecode *DecCreateContext(DDI_DECODE_CONFIG_ATTR *ddiAttr,
                                        const std::string &codecKey)
{
    DdiMediaDecode *decBase = DdiDecodeFactory::CreateCodec(codecKey, ddiAttr);

    if (decBase)
    {
        return decBase;
    }
    else
    {
        return nullptr;
    }
}
static int32_t DdiDecode_GetDisplayInfo(VADriverContextP ctx)
{
    PDDI_MEDIA_CONTEXT           pMediaDrvCtx;
    int32_t                      fd;
    struct fb_var_screeninfo     vsinfo;

    pMediaDrvCtx        = DdiMedia_GetMediaContext(ctx);
    fd                  = -1;
    vsinfo.xres         = 0;
    vsinfo.yres         = 0;

    fd = open("/dev/graphics/fb0",O_RDONLY);
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
    pMediaDrvCtx->uiDisplayWidth  = vsinfo.xres;
    pMediaDrvCtx->uiDisplayHeight = vsinfo.yres;

    DDI_NORMALMESSAGE("DDI:pMediaDrvCtx->uiDisplayWidth =%d", pMediaDrvCtx->uiDisplayWidth);
    DDI_NORMALMESSAGE("DDI:pMediaDrvCtx->uiDisplayHeight =%d",pMediaDrvCtx->uiDisplayHeight);

    return 0;
}

int32_t DdiDecode_GetBitstreamBufIndexFromBuffer(DDI_CODEC_COM_BUFFER_MGR *pBufMgr, DDI_MEDIA_BUFFER *pBuf)
{
    int32_t i;
    for(i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
    {
        if(pBufMgr->pBitStreamBuffObject[i]->bo == pBuf->bo)
        {
            return i;
        }
    }

    return DDI_CODEC_INVALID_BUFFER_INDEX;
}


static bool DdiDecode_AllocBpBuffer(
    DDI_CODEC_COM_BUFFER_MGR   *pBufMgr)
{
    if(pBufMgr->Codec_Param.Codec_Param_VC1.VC1BitPlane[pBufMgr->Codec_Param.Codec_Param_VC1.dwVC1BitPlaneIndex].bUsed)
    {
        // wait until decode complete
        mos_bo_wait_rendering(pBufMgr->Codec_Param.Codec_Param_VC1.pVC1BitPlaneBuffObject[pBufMgr->Codec_Param.Codec_Param_VC1.dwVC1BitPlaneIndex]->bo);
    }

    pBufMgr->Codec_Param.Codec_Param_VC1.VC1BitPlane[pBufMgr->Codec_Param.Codec_Param_VC1.dwVC1BitPlaneIndex].bUsed = true;

    return true;
}

static bool DdiDecode_AllocJPEGBsBuffer(
    DDI_CODEC_COM_BUFFER_MGR   *pBufMgr,
    DDI_MEDIA_BUFFER           *pBuf)
{
    // Allocate JPEG slice data memory from CPU.
    uint8_t                   *pBsAddr;
    int32_t                    index;

    index = pBufMgr->dwNumSliceData;

    /* the pSliceData needs to be reallocated in order to contain more SliceDataBuf */
    if (index >= pBufMgr->m_maxNumSliceData)
    {
        /* In theroy it can resize the m_maxNumSliceData one by one. But in order to
         * avoid calling realloc frequently, it will try to allocate 10 to  hold more
         * SliceDataBuf. This is only for the optimized purpose.
         */
        int32_t reallocSize = pBufMgr->m_maxNumSliceData + 10;

        pBufMgr->pSliceData = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)realloc(pBufMgr->pSliceData, sizeof(pBufMgr->pSliceData[0]) * reallocSize);

        if (pBufMgr->pSliceData == nullptr)
        {
            CODEC_DDI_ASSERTMESSAGE("fail to reallocate pSliceData for JPEG\n.");
            return false;
        }
        memset((void *)(pBufMgr->pSliceData + pBufMgr->m_maxNumSliceData), 0,
               sizeof(pBufMgr->pSliceData[0]) * 10);

        pBufMgr->m_maxNumSliceData += 10;
    }

    pBsAddr = (uint8_t*)MOS_AllocAndZeroMemory(pBuf->iSize);
    if(pBsAddr == 0)
    {
        return false;
    }

    pBuf->pData                             = pBsAddr;
    pBuf->format                            = Media_Format_CPU;
    pBuf->bCFlushReq                        = false;
    pBuf->uiOffset                          = 0;
    pBufMgr->pSliceData[index].uiLength     = pBuf->iSize;
    pBufMgr->pSliceData[index].uiOffset     = pBuf->uiOffset;
    pBufMgr->pSliceData[index].pBaseAddress = pBuf->pData;
    pBufMgr->dwNumSliceData ++;
    return true;
}

static bool DdiDecode_AllocBsBuffer(
    DDI_CODEC_COM_BUFFER_MGR    *pBufMgr,
    DDI_MEDIA_BUFFER            *pBuf,
    PDDI_MEDIA_CONTEXT          pMediaCtx)
{
    int32_t           index, i;
    VAStatus          vaStatus;
    uint8_t          *pSliceBuf;
    DDI_MEDIA_BUFFER *pBsBufObj = nullptr;
    uint8_t          *pBsBufBaseAddr = nullptr;
    bool              bCreateBsBuffer = false;

    if ( nullptr == pBufMgr || nullptr == pBuf || nullptr == pMediaCtx )
    {
        CODEC_DDI_ASSERTMESSAGE("invalidate input parameters.");
        return false;
    }

    index       = pBufMgr->dwNumSliceData;
    vaStatus    = VA_STATUS_SUCCESS;
    pSliceBuf   = nullptr;

    /* the pSliceData needs to be reallocated in order to contain more SliceDataBuf */
    if (index >= pBufMgr->m_maxNumSliceData)
    {
        /* In theroy it can resize the m_maxNumSliceData one by one. But in order to
         * avoid calling realloc frequently, it will try to allocate 10 to  hold more
         * SliceDataBuf. This is only for the optimized purpose.
         */
        int32_t reallocSize = pBufMgr->m_maxNumSliceData + 10;

        pBufMgr->pSliceData = (DDI_CODEC_BITSTREAM_BUFFER_INFO *)realloc(pBufMgr->pSliceData, sizeof(pBufMgr->pSliceData[0]) * reallocSize);

        if (pBufMgr->pSliceData == nullptr)
        {
            CODEC_DDI_ASSERTMESSAGE("fail to reallocate pSliceData\n.");
            return false;
        }
        memset(pBufMgr->pSliceData + pBufMgr->m_maxNumSliceData, 0,
               sizeof(pBufMgr->pSliceData[0]) * 10);

        pBufMgr->m_maxNumSliceData += 10;
    }

    if(index >= 1)
    {
        pBuf->uiOffset = pBufMgr->pSliceData[index-1].uiOffset + pBufMgr->pSliceData[index-1].uiLength;
        if((pBuf->uiOffset + pBuf->iSize) > pBufMgr->pBitStreamBuffObject[pBufMgr->dwBitstreamIndex]->iSize)
        {
            pSliceBuf = (uint8_t*)MOS_AllocAndZeroMemory(pBuf->iSize);
            if(pSliceBuf == nullptr)
            {
                CODEC_DDI_ASSERTMESSAGE("DDI:AllocAndZeroMem return failure.")
                return false;
            }
            pBufMgr->bIsSliceOverSize = true;
        }
        else
        {
            pBufMgr->bIsSliceOverSize = false;
        }
    }
    else
    {
        pBufMgr->bIsSliceOverSize = false;
        for (i = 0; i < DDI_CODEC_MAX_BITSTREAM_BUFFER; i++)
        {
            if (pBufMgr->pBitStreamBuffObject[i]->bo != nullptr)
            {
                if (!mos_bo_busy(pBufMgr->pBitStreamBuffObject[i]->bo))
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
            pBufMgr->dwBitstreamIndex = (pBufMgr->ui64BitstreamOrder >> (DDI_CODEC_BITSTREAM_BUFFER_INDEX_BITS * DDI_CODEC_MAX_BITSTREAM_BUFFER_MINUS1)) & DDI_CODEC_MAX_BITSTREAM_BUFFER_INDEX;
            // wait until decode complete
            mos_bo_wait_rendering(pBufMgr->pBitStreamBuffObject[pBufMgr->dwBitstreamIndex]->bo);
        }
        else
        {
            pBufMgr->dwBitstreamIndex = i;
        }
        pBufMgr->ui64BitstreamOrder = (pBufMgr->ui64BitstreamOrder << 4) + pBufMgr->dwBitstreamIndex;
        
        pBsBufObj                   = pBufMgr->pBitStreamBuffObject[pBufMgr->dwBitstreamIndex];
        pBsBufObj ->pMediaCtx       = pMediaCtx;
        pBsBufBaseAddr              = pBufMgr->pBitStreamBase[pBufMgr->dwBitstreamIndex];

        if(pBsBufBaseAddr == nullptr)
        {
            bCreateBsBuffer = true;
            if (pBuf->iSize > pBsBufObj->iSize)
            {
                pBsBufObj->iSize = pBuf->iSize;
            }
        }
        else if(pBuf->iSize > pBsBufObj->iSize)
        {
           //free bo
            DdiMediaUtil_UnlockBuffer(pBsBufObj);
            DdiMediaUtil_FreeBuffer(pBsBufObj);
            pBsBufBaseAddr = nullptr;

            bCreateBsBuffer = true;
            pBsBufObj->iSize = pBuf->iSize;
        }

        if (bCreateBsBuffer)
        {
            if(VA_STATUS_SUCCESS != DdiMediaUtil_CreateBuffer(pBsBufObj, pMediaCtx->pDrmBufMgr))
            {
               return false;
            }
            
            pBsBufBaseAddr = (uint8_t*)DdiMediaUtil_LockBuffer(pBsBufObj, MOS_LOCKFLAG_WRITEONLY);
            if(pBsBufBaseAddr == nullptr)
            {
                DdiMediaUtil_FreeBuffer(pBsBufObj);
                return false;
            }
            pBufMgr->pBitStreamBase[pBufMgr->dwBitstreamIndex] = pBsBufBaseAddr;
        }
    }
    
    if(pBufMgr->pBitStreamBase[pBufMgr->dwBitstreamIndex] == nullptr)
    {
        return false;
    }

    pBufMgr->pSliceData[index].uiLength = pBuf->iSize;
    pBufMgr->pSliceData[index].uiOffset = pBuf->uiOffset;

    if(pBufMgr->bIsSliceOverSize == true)
    {
        pBuf->pData                              = pSliceBuf;
        pBuf->uiOffset                           = 0;
        pBufMgr->pSliceData[index].bIsUseExtBuf  = true;
        pBufMgr->pSliceData[index].pSliceBuf     = pSliceBuf;
    }
    else
    {
        pBuf->pData                              = (uint8_t*)(pBufMgr->pBitStreamBase[pBufMgr->dwBitstreamIndex]);
        pBufMgr->pSliceData[index].bIsUseExtBuf  = false;
        pBufMgr->pSliceData[index].pSliceBuf     = nullptr;
    }

    pBufMgr->dwNumSliceData ++;
    pBuf->bo                            = pBufMgr->pBitStreamBuffObject[pBufMgr->dwBitstreamIndex]->bo;
    pBuf->bCFlushReq                    = true;

    return true;
}

static bool DdiDecode_AllocSliceParamContext(
   PDDI_DECODE_CONTEXT  pDecCtx,
   uint32_t             dwNumSlices
)
{
    uint32_t     uiBaseSize;
    DDI_CHK_NULL(pDecCtx, "Null pDecCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    switch ( pDecCtx->wMode)
    {
        case CODECHAL_DECODE_MODE_AVCVLD:
            uiBaseSize = sizeof(CODEC_AVC_SLICE_PARAMS);
            break;
        case CODECHAL_DECODE_MODE_MPEG2VLD:
            uiBaseSize = sizeof(CodecDecodeMpeg2SliceParams);
            break;
        case CODECHAL_DECODE_MODE_VC1VLD:
            uiBaseSize = sizeof(CODEC_VC1_SLICE_PARAMS);
            break;
        case CODECHAL_DECODE_MODE_JPEG:
            uiBaseSize = sizeof(CodecDecodeJpegScanParameter);
            break;
        case CODECHAL_DECODE_MODE_HEVCVLD:
            uiBaseSize = sizeof(CODEC_HEVC_SLICE_PARAMS);
            break;
        default:
            return false;
    }

    if(pDecCtx->dwSliceParamBufNum < (pDecCtx->DecodeParams.m_numSlices + dwNumSlices))
    {
        pDecCtx->DecodeParams.m_sliceParams = realloc(pDecCtx->DecodeParams.m_sliceParams, uiBaseSize * (pDecCtx->DecodeParams.m_numSlices + dwNumSlices));
        if(pDecCtx->DecodeParams.m_sliceParams == nullptr)
        {
            return false;
        }

        MOS_ZeroMemory((void*)((uint8_t*)pDecCtx->DecodeParams.m_sliceParams + uiBaseSize * pDecCtx->dwSliceParamBufNum), uiBaseSize * ((pDecCtx->DecodeParams.m_numSlices + dwNumSlices) - pDecCtx->dwSliceParamBufNum));
        pDecCtx->dwSliceParamBufNum = pDecCtx->DecodeParams.m_numSlices + dwNumSlices;
    }

    return true;
}

static bool DdiDecode_AllocSliceControlBuffer(
    PDDI_DECODE_CONTEXT     pDecCtx,
    DDI_MEDIA_BUFFER       *pBuf)
{
    DDI_CODEC_COM_BUFFER_MGR   *pBufMgr;
    uint32_t                    uiAvailSize;
    uint32_t                    uiNewSize;

    pBufMgr     = &(pDecCtx->BufMgr);
    uiAvailSize = pDecCtx->dwSliceCtrlBufNum - pBufMgr->dwNumSliceControl;
    switch (pDecCtx->wMode)
    {
        case CODECHAL_DECODE_MODE_AVCVLD:
            if(pDecCtx->bShortFormatInUse)
            {
                if(uiAvailSize < pBuf->iNumElements)
                {
                    uiNewSize   = sizeof(VASliceParameterBufferBase) * (pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements);
                    pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264Base = (VASliceParameterBufferBase *)realloc(pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264Base, uiNewSize);
                    if(pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264Base == nullptr)
                    {
                        return false;
                    }
                    MOS_ZeroMemory(pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264Base + pDecCtx->dwSliceCtrlBufNum, sizeof(VASliceParameterBufferBase) * (pBuf->iNumElements - uiAvailSize));
                    pDecCtx->dwSliceCtrlBufNum = pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements;
                }
                pBuf->pData      = (uint8_t*)pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264Base;
                pBuf->uiOffset   = pBufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferBase);
            }
            else
            {
                if(uiAvailSize < pBuf->iNumElements)
                {
                    uiNewSize   = sizeof(VASliceParameterBufferH264) * (pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements);
                    pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264 = (VASliceParameterBufferH264 *)realloc(pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264, uiNewSize);
                    if(pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264 == nullptr)
                    {
                        return false;
                    }
                    MOS_ZeroMemory(pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264 + pDecCtx->dwSliceCtrlBufNum, sizeof(VASliceParameterBufferH264) * (pBuf->iNumElements - uiAvailSize));
                    pDecCtx->dwSliceCtrlBufNum = pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements;
                }
                pBuf->pData      = (uint8_t*)pBufMgr->Codec_Param.Codec_Param_H264.pVASliceParaBufH264;
                pBuf->uiOffset   = pBufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferH264);
            }
            break;
        case CODECHAL_DECODE_MODE_MPEG2VLD:
            if(uiAvailSize < pBuf->iNumElements)
            {
                uiNewSize   = sizeof(VASliceParameterBufferMPEG2) * (pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements);
                pBufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 = (VASliceParameterBufferMPEG2 *)realloc(pBufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2, uiNewSize);
                if(pBufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 == nullptr)
                {
                    return false;
                }
                MOS_ZeroMemory(pBufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2 + pDecCtx->dwSliceCtrlBufNum, sizeof(VASliceParameterBufferMPEG2) * (pBuf->iNumElements - uiAvailSize));
                pDecCtx->dwSliceCtrlBufNum = pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements;
            }
            pBuf->pData      = (uint8_t*)pBufMgr->Codec_Param.Codec_Param_MPEG2.pVASliceParaBufMPEG2;
            pBuf->uiOffset   = sizeof(VASliceParameterBufferMPEG2) * pBufMgr->dwNumSliceControl;
            break;
        case CODECHAL_DECODE_MODE_VC1VLD:
            if(uiAvailSize < pBuf->iNumElements)
            {
                uiNewSize   = sizeof(VASliceParameterBufferVC1) * (pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements);
                pBufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 = (VASliceParameterBufferVC1 *)realloc(pBufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1, uiNewSize);
                if(pBufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 == nullptr)
                {
                    return false;
                }
                MOS_ZeroMemory(pBufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1 + pDecCtx->dwSliceCtrlBufNum, sizeof(VASliceParameterBufferVC1) * (pBuf->iNumElements - uiAvailSize));
                pDecCtx->dwSliceCtrlBufNum = pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements;
            }
            pBuf->pData      = (uint8_t*)pBufMgr->Codec_Param.Codec_Param_VC1.pVASliceParaBufVC1;
            pBuf->uiOffset   = sizeof(VASliceParameterBufferVC1) * pBufMgr->dwNumSliceControl;
            break;
         case CODECHAL_DECODE_MODE_JPEG:
            if(uiAvailSize < pBuf->iNumElements)
            {
                uiNewSize   = sizeof(VASliceParameterBufferJPEGBaseline) * (pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements);
                pBufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG = (VASliceParameterBufferJPEGBaseline *)realloc(pBufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG, uiNewSize);
                if(pBufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG == nullptr)
                {
                    return false;
                }
                MOS_ZeroMemory(pBufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG + pDecCtx->dwSliceCtrlBufNum, sizeof(VASliceParameterBufferJPEGBaseline) * (pBuf->iNumElements - uiAvailSize));
                pDecCtx->dwSliceCtrlBufNum = pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements;
            }
            pBuf->pData      = (uint8_t*)pBufMgr->Codec_Param.Codec_Param_JPEG.pVASliceParaBufJPEG;
            pBuf->uiOffset   = sizeof(VASliceParameterBufferJPEGBaseline) * pBufMgr->dwNumSliceControl;
            break;
        case CODECHAL_DECODE_MODE_VP8VLD:
            if(pBufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8 == nullptr)
            {
                return false;
            }
            pBuf->pData      = (uint8_t*)pBufMgr->Codec_Param.Codec_Param_VP8.pVASliceParaBufVP8;
            pBuf->uiOffset   = pBufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferVP8);
            break;
        case CODECHAL_DECODE_MODE_HEVCVLD:
            if(pDecCtx->bShortFormatInUse)
            {
                if(uiAvailSize < pBuf->iNumElements)
                {
                    uiNewSize   = sizeof(VASliceParameterBufferBase) * (pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements);
                    pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC = (VASliceParameterBufferBase *)realloc(pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC, uiNewSize);
                    if(pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC == nullptr)
                    {
                        return false;
                    }
                    MOS_ZeroMemory(pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC + pDecCtx->dwSliceCtrlBufNum, sizeof(VASliceParameterBufferBase) * (pBuf->iNumElements - uiAvailSize));
                    pDecCtx->dwSliceCtrlBufNum = pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements;
                }
                pBuf->pData      = (uint8_t*)pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufBaseHEVC;
                pBuf->uiOffset   = pBufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferBase);
            }
            else
            {
                if(uiAvailSize < pBuf->iNumElements)
                {
                    uiNewSize   = sizeof(VASliceParameterBufferHEVC) * (pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements);
                    pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC = (VASliceParameterBufferHEVC *)realloc(pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC, uiNewSize);
                    if(pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC == nullptr)
                    {
                        return false;
                    }
                    MOS_ZeroMemory(pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC + pDecCtx->dwSliceCtrlBufNum, sizeof(VASliceParameterBufferHEVC) * (pBuf->iNumElements - uiAvailSize));
                    pDecCtx->dwSliceCtrlBufNum = pDecCtx->dwSliceCtrlBufNum - uiAvailSize + pBuf->iNumElements;
                }
                pBuf->pData      = (uint8_t*)pBufMgr->Codec_Param.Codec_Param_HEVC.pVASliceParaBufHEVC;
                pBuf->uiOffset   = pBufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferHEVC);
            }
            break;
        case CODECHAL_DECODE_MODE_VP9VLD:
            if(pBufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9 == nullptr)
            {
                return false;
            }
            pBuf->pData    = (uint8_t*)pBufMgr->Codec_Param.Codec_Param_VP9.pVASliceParaBufVP9;
            pBuf->uiOffset = pBufMgr->dwNumSliceControl * sizeof(VASliceParameterBufferVP9);
            break;
        default:
                return false;
    }

    pBufMgr->dwNumSliceControl += pBuf->iNumElements;

    return true;
}


PDDI_DECODE_CONTEXT DdiDecode_GetDecContextFromContextID (VADriverContextP ctx, VAContextID vaCtxID)
{
    uint32_t  uiCtxType;
    void     *pCtx;

    pCtx = DdiMedia_GetContextFromContextID(ctx, vaCtxID, &uiCtxType);
    if(nullptr == pCtx)
        return nullptr;
    return (PDDI_DECODE_CONTEXT)pCtx;
}

VAStatus DdiDecode_CreateBuffer(
    VADriverContextP         ctx,
    PDDI_DECODE_CONTEXT      pDecCtx,
    VABufferType             type,
    uint32_t                 size,
    uint32_t                 num_elements,
    void                    *pData,
    VABufferID              *pBufId
)
{
    PDDI_MEDIA_CONTEXT               pMediaCtx;
    DDI_MEDIA_BUFFER                *pBuf;
    VAStatus                         va;
    Codechal                        *pCodecHal;
    PDDI_MEDIA_BUFFER_HEAP_ELEMENT   pBufferHeapElement;
    uint16_t                         segMapWidth, segMapHeight;
    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

    va          = VA_STATUS_SUCCESS;
    *pBufId     = VA_INVALID_ID;

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);

    pCodecHal   = pDecCtx->pCodecHal;
    DDI_CHK_NULL(pCodecHal, "Null pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    segMapWidth = pDecCtx->wPicWidthInMB;
    segMapHeight= pDecCtx->wPicHeightInMB;

    // only for VASliceParameterBufferType of buffer, the number of elements can be greater than 1
    if(type != VASliceParameterBufferType && num_elements > 1)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    pBuf               = (DDI_MEDIA_BUFFER *)MOS_AllocAndZeroMemory(sizeof(DDI_MEDIA_BUFFER));
    if (pBuf == nullptr)
    {
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    pBuf->iSize        = size * num_elements;
    pBuf->iNumElements = num_elements;
    pBuf->uiType       = type;
    pBuf->format       = Media_Format_Buffer;
    pBuf->uiOffset     = 0;
    pBuf->bCFlushReq   = false;
    pBuf->pMediaCtx    = pMediaCtx;

    switch ((int32_t)type)
    {
        case VABitPlaneBufferType:
            pBuf->pData = (uint8_t*)((pDecCtx->BufMgr.Codec_Param.Codec_Param_VC1.pBitPlaneBuffer));
            break;
        case VASliceDataBufferType:
        case VAProtectedSliceDataBufferType:
            // For JPEG we have a special way to allocate slice data buffer.
            if (pDecCtx->wMode == CODECHAL_DECODE_MODE_JPEG)
            {
                if(!DdiDecode_AllocJPEGBsBuffer(&(pDecCtx->BufMgr), pBuf))
                {
                    va = VA_STATUS_ERROR_ALLOCATION_FAILED;
                    goto CleanUpandReturn;
                }
            }
            else
            {
                if(!DdiDecode_AllocBsBuffer(&(pDecCtx->BufMgr), pBuf, pMediaCtx))
                {
                    va = VA_STATUS_ERROR_ALLOCATION_FAILED;
                    goto CleanUpandReturn;
                }
            }
            break;
        case VASliceParameterBufferType:
            if(!DdiDecode_AllocSliceControlBuffer(pDecCtx, pBuf))
            {
                va = VA_STATUS_ERROR_ALLOCATION_FAILED;
                goto CleanUpandReturn;
            }
            pBuf->format     = Media_Format_CPU;
            break;
        case VAPictureParameterBufferType:
            switch (pDecCtx->wMode)
            {
                case CODECHAL_DECODE_MODE_AVCVLD:
                    pBuf->pData = (uint8_t*)(&(pDecCtx->BufMgr.Codec_Param.Codec_Param_H264.PicParam264));
                    break;
                case CODECHAL_DECODE_MODE_MPEG2VLD:
                    pBuf->pData = (uint8_t*)(&(pDecCtx->BufMgr.Codec_Param.Codec_Param_MPEG2.PicParamMPEG2));
                    break;
                case CODECHAL_DECODE_MODE_VC1VLD:
                    pBuf->pData = (uint8_t*)(&(pDecCtx->BufMgr.Codec_Param.Codec_Param_VC1.PicParamVC1));
                    break;
                case CODECHAL_DECODE_MODE_JPEG:
                    pBuf->pData = (uint8_t*)(&(pDecCtx->BufMgr.Codec_Param.Codec_Param_JPEG.PicParamJPEG));
                    break;
                case CODECHAL_DECODE_MODE_VP8VLD:
                    pBuf->pData = (uint8_t*)(&(pDecCtx->BufMgr.Codec_Param.Codec_Param_VP8.PicParamVP8));
                    break;
                case CODECHAL_DECODE_MODE_HEVCVLD:
                    pBuf->pData = (uint8_t*)(&(pDecCtx->BufMgr.Codec_Param.Codec_Param_HEVC.PicParamHEVC));
                    break;
                case CODECHAL_DECODE_MODE_VP9VLD:
                    pBuf->pData = (uint8_t*)(&(pDecCtx->BufMgr.Codec_Param.Codec_Param_VP9.PicParamVP9));
                    break;
                default:
                    va = VA_STATUS_ERROR_INVALID_CONTEXT;
                    goto CleanUpandReturn;
                    break;
            }
            pBuf->format     = Media_Format_CPU;
            break;
        case VAIQMatrixBufferType:
            pBuf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(size * num_elements);
            pBuf->format     = Media_Format_CPU;
            break;
        case VAProbabilityBufferType:
            pBuf->pData      = (uint8_t*)(&(pDecCtx->BufMgr.Codec_Param.Codec_Param_VP8.ProbabilityDataVP8));
            break;
        case VAProcFilterParameterBufferType:
            pBuf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineCaps));
            pBuf->format     = Media_Format_CPU;
            break;
        case VAProcPipelineParameterBufferType:
            pBuf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(sizeof(VAProcPipelineParameterBuffer));
            pBuf->format     = Media_Format_CPU;
            break;
        case VADecodeStreamoutBufferType:
        {
            segMapHeight = ((segMapHeight + 1) >> 1);   //uiSize must be equal and bigger than size for interlaced case

            if (size < MOS_ALIGN_CEIL(segMapHeight * segMapWidth * CODEC_SIZE_MFX_STREAMOUT_DATA, 64))
            {
                va = VA_STATUS_ERROR_INVALID_PARAMETER;
                goto CleanUpandReturn;
            }
            pBuf->iSize  = size * num_elements;   
            pBuf->format = Media_Format_Buffer;
            va = DdiMediaUtil_CreateBuffer(pBuf, pMediaCtx->pDrmBufMgr);
            if(va != VA_STATUS_SUCCESS)
            {
                goto CleanUpandReturn;
            }
            break;
        }
        case VAHuffmanTableBufferType:
            pBuf->pData      = (uint8_t*)MOS_AllocAndZeroMemory(size * num_elements);
            pBuf->format     = Media_Format_CPU;
            break;
        default:
            va = pDecCtx->pCpDdiInterface->CreateBuffer(type, pBuf, size, num_elements);
            if (va  == VA_STATUS_ERROR_UNSUPPORTED_BUFFERTYPE)
            {
                MOS_FreeMemory(pBuf);
                return va;
            }
            break;
    }

    pBufferHeapElement  = DdiMediaUtil_AllocPMediaBufferFromHeap(pMediaCtx->pBufferHeap);
    if (nullptr == pBufferHeapElement)
    {
        va = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        goto CleanUpandReturn;
    }
    pBufferHeapElement->pBuffer      = pBuf;
    pBufferHeapElement->pCtx         = (void*)pDecCtx;
    pBufferHeapElement->uiCtxType    = DDI_MEDIA_CONTEXT_TYPE_DECODER;
    *pBufId                          = pBufferHeapElement->uiVaBufferID;


    // Keep record the VaBufferID of JPEG slice data buffer we allocated, in order to do buffer mapping when render this buffer. otherwise we
    // can not get correct buffer address when application create them disordered.
    if (type == VASliceDataBufferType && pDecCtx->wMode == CODECHAL_DECODE_MODE_JPEG)
    {
        // since the dwNumSliceData already +1 when allocate buffer, but here we need to track the VaBufferID before dwSliceData increased.
        pDecCtx->BufMgr.pSliceData[pDecCtx->BufMgr.dwNumSliceData - 1].vaBufferId = *pBufId;
    }
    pMediaCtx->uiNumBufs++;

    if(pData == nullptr)
    {
        return va;
    }

    if( true == pBuf->bCFlushReq )
    {
        mos_bo_subdata(pBuf->bo, pBuf->uiOffset, size * num_elements, pData);
    }
    else
    {
        eStatus = MOS_SecureMemcpy((void *)(pBuf->pData + pBuf->uiOffset), size * num_elements, pData, size * num_elements);
        DDI_CHK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "DDI:Failed to copy buffer data!", VA_STATUS_ERROR_OPERATION_FAILED);
    }
    return va;

CleanUpandReturn:
    if(pBuf)
    {
        MOS_FreeMemory(pBuf->pData);
        MOS_FreeMemory(pBuf);
    }
    return va;

}

VAStatus DdiDecode_UnRegisterRTSurfaces(
    VADriverContextP    ctx,
    PDDI_MEDIA_SURFACE surface)
{
    DDI_CHK_NULL(ctx,"nullptr context!", VA_STATUS_ERROR_INVALID_CONTEXT);
    PDDI_MEDIA_CONTEXT mediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(mediaCtx,"nullptr mediaCtx!", VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(surface, "nullptr surface!", VA_STATUS_ERROR_INVALID_PARAMETER);
    
    //Look through all decode contexts to unregister the surface in each decode context's RTtable.
    if (mediaCtx->pDecoderCtxHeap != nullptr)
    {
        PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT pDecVACtxHeapBase;

        DdiMediaUtil_LockMutex(&mediaCtx->DecoderMutex);
        pDecVACtxHeapBase  = (PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT)mediaCtx->pDecoderCtxHeap->pHeapBase;
        for (int32_t j = 0; j < mediaCtx->pDecoderCtxHeap->uiAllocatedHeapElements; j++)
        {
            if (pDecVACtxHeapBase[j].pVaContext != nullptr)
            {
                PDDI_DECODE_CONTEXT  pDecCtx = (PDDI_DECODE_CONTEXT)pDecVACtxHeapBase[j].pVaContext;
                if (pDecCtx && pDecCtx->m_ddiDecode)
                {
                    //not check the return value since the surface may not be registered in the context. pay attention to LOGW.
                    pDecCtx->m_ddiDecode->UnRegisterRTSurfaces(&pDecCtx->RTtbl, surface);
                }
            }
        }
        DdiMediaUtil_UnLockMutex(&mediaCtx->DecoderMutex);
    }
    return VA_STATUS_SUCCESS;
}

VAStatus DdiDecode_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         render_target
)
{
    PDDI_MEDIA_CONTEXT               pMediaCtx;
    PDDI_DECODE_CONTEXT              pDecCtx;
    VAStatus                         vaStatus;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,                "nullptr context in vpgDecodeBeginPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);
    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,          "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pDecCtx     = DdiDecode_GetDecContextFromContextID(ctx, context);
    DDI_CHK_NULL(pDecCtx,            "Null pDecCtx",            VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pDecCtx->pCodecHal, "Null pDecCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    if (pDecCtx->m_ddiDecode)
    {
        vaStatus = pDecCtx->m_ddiDecode->BeginPicture(ctx, context, render_target);
        DDI_FUNCTION_EXIT(vaStatus);
        return vaStatus;
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
    VAStatus                            vaStatus;
    PDDI_MEDIA_CONTEXT                  pMediaCtx;
    PDDI_DECODE_CONTEXT                 pDecCtx;
    MOS_STATUS                          eStatus;
    uint32_t                            uiCtxType;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,                "nullptr context in vpgDecodeEndPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);
    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,          "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);
    // assume the VAContextID is decoder ID
    pDecCtx     = (PDDI_DECODE_CONTEXT)DdiMedia_GetContextFromContextID(ctx, context, &uiCtxType);
    DDI_CHK_NULL(pDecCtx,            "Null pDecCtx",            VA_STATUS_ERROR_INVALID_CONTEXT);

    if (pDecCtx->m_ddiDecode)
    {
        vaStatus = pDecCtx->m_ddiDecode->EndPicture(ctx, context);
        DDI_FUNCTION_EXIT(vaStatus);
        return vaStatus;
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
    int32_t             num_buffers
)
{
    PDDI_MEDIA_CONTEXT                  pMediaCtx;
    PDDI_DECODE_CONTEXT                 pDecCtx;
    VAStatus                            vaStatus = VA_STATUS_SUCCESS;

    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(ctx,                "nullptr context in vpgDecodeRenderPicture!", VA_STATUS_ERROR_INVALID_CONTEXT);
    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,          "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    // assume the VAContextID is decoder ID
    pDecCtx     = DdiDecode_GetDecContextFromContextID(ctx, context);
    DDI_CHK_NULL(pDecCtx,            "Null pDecCtx",            VA_STATUS_ERROR_INVALID_CONTEXT);

    if (pDecCtx->m_ddiDecode)
    {
        vaStatus = pDecCtx->m_ddiDecode->RenderPicture(ctx, context, buffers, num_buffers);
        DDI_FUNCTION_EXIT(vaStatus);
        return vaStatus;
    }

    DDI_FUNCTION_EXIT(VA_STATUS_ERROR_UNIMPLEMENTED);
    return VA_STATUS_ERROR_UNIMPLEMENTED;
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
    VAConfigID          config_id,
    int32_t             picture_width,
    int32_t             picture_height,
    int32_t             flag,
    VASurfaceID        *render_targets,
    int32_t             num_render_targets,
    VAContextID        *context
)
{
    VAStatus                          vaStatus;
    PDDI_DECODE_CONTEXT               pDecCtx;
    uint16_t                          wMode;
    PDDI_MEDIA_CONTEXT                pMediaCtx;
    MOS_CONTEXT                       MosCtx = {};
    int32_t                           i;
    PDDI_MEDIA_VACONTEXT_HEAP_ELEMENT pVaContextHeapElmt;
    uint32_t                          uiDecIndex;
    int32_t                           picture_height_aligned;
    std::string                       codecKey;
    DdiMediaDecode                   *ddiDecBase;
    DDI_DECODE_CONFIG_ATTR            decConfigAttr;

    DDI_UNUSED(flag);

    vaStatus            = VA_STATUS_SUCCESS;
    decConfigAttr.uiDecSliceMode = VA_DEC_SLICE_MODE_BASE;
    *context            = VA_INVALID_ID;

    wMode               = CODECHAL_DECODE_MODE_AVCVLD;

    DDI_CHK_NULL(ctx, "Null Ctx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pMediaCtx  = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx, "Null pMediaCtx", VA_STATUS_ERROR_INVALID_CONTEXT);

    pDecCtx = nullptr;
    if (num_render_targets > DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT)
    {
        return VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
    }

    codecKey = DECODE_ID_NONE;

    DDI_CHK_NULL(pMediaCtx->m_caps, "Null m_caps", VA_STATUS_ERROR_INVALID_CONTEXT);
    vaStatus = pMediaCtx->m_caps->GetDecConfigAttr(
            config_id + DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE,
            &decConfigAttr.profile,
            &decConfigAttr.entrypoint,
            &decConfigAttr.uiDecSliceMode,
            &decConfigAttr.uiEncryptionType,
            &decConfigAttr.uiDecProcessingType);
    DDI_CHK_RET(vaStatus, "Invalide config_id!");

    wMode = pMediaCtx->m_caps->GetDecodeCodecMode(decConfigAttr.profile);
    codecKey =  pMediaCtx->m_caps->GetDecodeCodecKey(decConfigAttr.profile);
    vaStatus = pMediaCtx->m_caps->CheckDecodeResolution(
            wMode,
            decConfigAttr.profile,
            picture_width,
            picture_height);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto CleanUpandReturn;
    }

    ddiDecBase = DecCreateContext(nullptr, codecKey);
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
    pDecCtx = (DDI_DECODE_CONTEXT *) (*ddiDecBase);

    if (nullptr == pDecCtx)
    {
        if (ddiDecBase)
            MOS_Delete(ddiDecBase);
        return VA_STATUS_ERROR_ALLOCATION_FAILED;
    }
    
    pDecCtx->pMediaCtx  = pMediaCtx;
    pDecCtx->m_ddiDecode = ddiDecBase;

    MosCtx.bufmgr                           = pMediaCtx->pDrmBufMgr;
    MosCtx.fd                               = pMediaCtx->fd;
    MosCtx.iDeviceId                        = pMediaCtx->iDeviceId;
    MosCtx.SkuTable                         = pMediaCtx->SkuTable;
    MosCtx.WaTable                          = pMediaCtx->WaTable;
    MosCtx.gtSystemInfo                     = *pMediaCtx->pGtSystemInfo;
    MosCtx.platform                         = pMediaCtx->platform;
    MosCtx.ppMediaMemDecompState            = &pMediaCtx->pMediaMemDecompState;
    MosCtx.pfnMemoryDecompress              = pMediaCtx->pfnMemoryDecompress;
    MosCtx.pPerfData                        = (PERF_DATA*)MOS_AllocAndZeroMemory(sizeof(PERF_DATA));
    MosCtx.pbHybridDecMultiThreadEnabled    = &pMediaCtx->bHybridDecMultiThreadEnabled;
    if (nullptr == MosCtx.pPerfData)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    ddiDecBase->ContextInit(picture_width, picture_height);

    //initialize DDI level CP interface
    pDecCtx->pCpDdiInterface = MOS_New(DdiCpInterface, MosCtx);
    if (nullptr == pDecCtx->pCpDdiInterface)
    {
        vaStatus = VA_STATUS_ERROR_ALLOCATION_FAILED;
        goto CleanUpandReturn;
    }

    /* the step three */
    vaStatus = ddiDecBase->CodecHalInit(pMediaCtx, &MosCtx);
    if (vaStatus != VA_STATUS_SUCCESS)
    {
        goto CleanUpandReturn;
    }

    DdiDecode_GetDisplayInfo(ctx);

    // register render targets
    for (i = 0; i < num_render_targets; i++)
    {
        DDI_MEDIA_SURFACE   *pSurface;

        pSurface   = DdiMedia_GetSurfaceFromVASurfaceID(pMediaCtx, render_targets[i]);
        if (nullptr == pSurface)
        {
            DDI_ASSERTMESSAGE("DDI: invalid render target %d in vpgCreateContext.",i);
            vaStatus = VA_STATUS_ERROR_INVALID_SURFACE;
            goto CleanUpandReturn;
        }
        if (VA_STATUS_SUCCESS != ddiDecBase->RegisterRTSurfaces(&pDecCtx->RTtbl, pSurface))
        {
            vaStatus = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;            
            goto CleanUpandReturn;
        }
    }

    DdiMediaUtil_LockMutex(&pMediaCtx->DecoderMutex);
    pVaContextHeapElmt = DdiMediaUtil_AllocPVAContextFromHeap(pMediaCtx->pDecoderCtxHeap);

    if (nullptr == pVaContextHeapElmt)
    {
        DdiMediaUtil_UnLockMutex(&pMediaCtx->DecoderMutex);
        vaStatus = VA_STATUS_ERROR_MAX_NUM_EXCEEDED;
        goto CleanUpandReturn;
    }

    pVaContextHeapElmt->pVaContext     = (void*)pDecCtx;
    pMediaCtx->uiNumDecoders++;
    *context                           = (VAContextID)(pVaContextHeapElmt->uiVaContextID + DDI_MEDIA_VACONTEXTID_OFFSET_DECODER);
    DdiMediaUtil_UnLockMutex(&pMediaCtx->DecoderMutex);

    // init the RecListSUrfaceID for checking DPB.
    for(i = 0; i < CODECHAL_AVC_NUM_UNCOMPRESSED_SURFACE; i++)
    {
        pDecCtx->RecListSurfaceID[i] = VA_INVALID_ID;
    }
    return vaStatus;

CleanUpandReturn:
    if (pDecCtx && ddiDecBase)
    {
        ddiDecBase->DestroyContext(ctx);
        MOS_Delete(ddiDecBase);
        MOS_FreeMemory(pDecCtx);
        pDecCtx = nullptr;
    }

    return vaStatus;
}

VAStatus DdiDecode_DestroyContext (
    VADriverContextP    ctx,
    VAContextID         context
)
{
    PDDI_MEDIA_CONTEXT       pMediaCtx;
    PDDI_DECODE_CONTEXT      pDecCtx;
    uint32_t                 uiDecIndex;

    pMediaCtx   = DdiMedia_GetMediaContext(ctx);
    DDI_CHK_NULL(pMediaCtx,          "Null pMediaCtx",          VA_STATUS_ERROR_INVALID_CONTEXT);
    pDecCtx     = DdiDecode_GetDecContextFromContextID(ctx, context);
    DDI_CHK_NULL(pDecCtx,            "Null pDecCtx",            VA_STATUS_ERROR_INVALID_CONTEXT);
    DDI_CHK_NULL(pDecCtx->pCodecHal, "Null pDecCtx->pCodecHal", VA_STATUS_ERROR_INVALID_CONTEXT);

    /* Free the context id from the context_heap earlier */
    uiDecIndex                          = (uint32_t)context;
    uiDecIndex                         &= DDI_MEDIA_MASK_VACONTEXTID;
    DdiMediaUtil_LockMutex(&pMediaCtx->DecoderMutex);
    DdiMediaUtil_ReleasePVAContextFromHeap(pMediaCtx->pDecoderCtxHeap, uiDecIndex);
    pMediaCtx->uiNumDecoders--;
    DdiMediaUtil_UnLockMutex(&pMediaCtx->DecoderMutex);

    if (pDecCtx->m_ddiDecode) {
        pDecCtx->m_ddiDecode->DestroyContext(ctx);
        MOS_Delete(pDecCtx->m_ddiDecode);
        pDecCtx->m_ddiDecode = nullptr;
        MOS_FreeMemory(pDecCtx);
        return VA_STATUS_SUCCESS;
    }

    return VA_STATUS_SUCCESS;
}
