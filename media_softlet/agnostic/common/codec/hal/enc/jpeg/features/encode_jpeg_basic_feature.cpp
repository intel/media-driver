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
//! \file     encode_jpeg_basic_feature.cpp
//! \brief    Defines the common interface for encode jpeg parameter
//!

#include "encode_jpeg_basic_feature.h"

namespace encode
{

MOS_STATUS JpegBasicFeature::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::Init(setting));

    ENCODE_CHK_STATUS_RETURN(InitRefFrames());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegBasicFeature::InitRefFrames()
{
    ENCODE_FUNC_CALL();

    /*There is no any reference frame for JPEG. It uses status report.*/
    m_ref = std::make_shared<JpegReferenceFrames>();
    ENCODE_CHK_NULL_RETURN(m_ref);

    ENCODE_CHK_STATUS_RETURN(m_ref->Init(this));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegBasicFeature::GetTrackedBuffers()
{
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);
    ENCODE_CHK_NULL_RETURN(m_allocator);

    auto currRefList = m_ref->GetCurrRefList();
    ENCODE_CHK_STATUS_RETURN(m_trackedBuf->Acquire(currRefList, false));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegBasicFeature::Update(void *params)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::Update(params));

    EncoderParams* encodeParams = (EncoderParams*)params;
    ENCODE_CHK_NULL_RETURN(encodeParams->pPicParams);

    m_jpegPicParams        = (CodecEncodeJpegPictureParams *)(encodeParams->pPicParams);
    m_jpegScanParams      = (CodecEncodeJpegScanHeader *)(encodeParams->pSliceParams);
    m_jpegQuantTables     = (CodecEncodeJpegQuantTable *)(encodeParams->pQuantizationTable);
    m_jpegHuffmanTable    = (CodecEncodeJpegHuffmanDataArray *)(encodeParams->pHuffmanTable);
    m_applicationData     = encodeParams->pApplicationData;
    m_appDataSize         = encodeParams->dwAppDataSize;
    m_jpegQuantMatrixSent = encodeParams->bJpegQuantMatrixSent;
    m_fullHeaderInAppData = encodeParams->fullHeaderInAppData;
    m_numHuffBuffers      = encodeParams->dwNumHuffBuffers;

    ENCODE_CHK_NULL_RETURN(m_jpegPicParams);
    ENCODE_CHK_NULL_RETURN(m_jpegScanParams);
    ENCODE_CHK_NULL_RETURN(m_jpegQuantTables);
    ENCODE_CHK_NULL_RETURN(m_jpegHuffmanTable);

    ENCODE_CHK_STATUS_RETURN(m_ref->UpdatePicture());

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_MODE_SELECT, JpegBasicFeature)
{
    params.standardSelect         = CodecHal_GetStandardFromMode(m_mode);
    params.codecSelect            = encoderCodec;
    params.decoderShortFormatMode = 1;

    return MOS_STATUS_SUCCESS;
}

static inline uint32_t GetHwTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
{
    uint32_t tileMode = 0;

    if (gmmTileEnabled)
    {
        return tileModeGMM;
    }

    switch (tileType)
    {
    case MOS_TILE_LINEAR:
        tileMode = 0;
        break;
    case MOS_TILE_YS:
        tileMode = 1;
        break;
    case MOS_TILE_X:
        tileMode = 2;
        break;
    default:
        tileMode = 3;
        break;
    }

    return tileMode;
}

static inline uint8_t MosToMediaStateFormat(MOS_FORMAT format)
{
    switch (format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
        return MHW_MEDIASTATE_SURFACEFORMAT_R8G8B8A8_UNORM;
    case Format_422H:
    case Format_422V:
        return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_422_8;
    case Format_AYUV:
    case Format_AUYV:
        return MHW_MEDIASTATE_SURFACEFORMAT_A8Y8U8V8_UNORM;
    case Format_NV12:
    case Format_NV11:
    case Format_P208:
    case Format_IMC1:
    case Format_IMC3:
        return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_420_8;
    case Format_400P:
    case Format_P8:
        return MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM;
    case Format_411P:
    case Format_411R:
        return MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_411_8;
    case Format_UYVY:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPY;
    case Format_YVYU:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUV;
    case Format_VYUY:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUVY;
    case Format_YUY2:
    case Format_YUYV:
    case Format_444P:
    case Format_IMC2:
    case Format_IMC4:
    default:
        return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL;
    }

    return MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL;
}

static inline bool IsVPlanePresent(MOS_FORMAT format)
{
    switch (format)
    {
    case Format_NV12:
    case Format_NV11:
    case Format_P208:
    case Format_IMC1:
    case Format_IMC3:
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_422H:
    case Format_422V:
        // Adding RGB formats because RGB is treated like YUV for JPEG encode and decode
    case Format_RGBP:
    case Format_BGRP:
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
    case Format_411P:
    case Format_411R:
    case Format_444P:
    case Format_IMC2:
    case Format_IMC4:
        return true;
    default:
        return false;
    }
}

MHW_SETPAR_DECL_SRC(MFX_SURFACE_STATE, JpegBasicFeature)
{
    PMOS_SURFACE psSurface        = m_rawSurfaceToPak;
    uint32_t     uvPlaneAlignment = MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9;

    ENCODE_CHK_NULL_RETURN(psSurface);

    params.surfaceId        = CODECHAL_MFX_SRC_SURFACE_ID;
    params.height           = psSurface->dwHeight - 1;
    params.width            = psSurface->dwWidth - 1;
    params.tilemode         = GetHwTileType(psSurface->TileType, psSurface->TileModeGMM, psSurface->bGMMTileEnabled);
    params.surfacePitch     = psSurface->dwPitch - 1;
    params.interleaveChroma = psSurface->Format == Format_P8 ? 0 : 1;
    params.surfaceFormat    = MosToMediaStateFormat(psSurface->Format);

    params.yOffsetForUCb = params.yOffsetForVCr =
        MOS_ALIGN_CEIL((psSurface->UPlaneOffset.iSurfaceOffset - psSurface->dwOffset)/psSurface->dwPitch + psSurface->RenderOffset.YUV.U.YOffset, uvPlaneAlignment);
    if (IsVPlanePresent(psSurface->Format))
    {
        params.yOffsetForVCr =
            MOS_ALIGN_CEIL((psSurface->VPlaneOffset.iSurfaceOffset - psSurface->dwOffset)/psSurface->dwPitch + psSurface->RenderOffset.YUV.V.YOffset, uvPlaneAlignment);
    }

#ifdef _MMC_SUPPORTED
    if (m_mmcState && m_mmcState->IsMmcEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(psSurface, &params.compressionFormat));
    }
#endif
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_BUF_ADDR_STATE, JpegBasicFeature)
{
    params.decodeInUse  = false;
    params.psRawSurface = m_rawSurfaceToPak;

#ifdef _MMC_SUPPORTED
    ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(m_rawSurfaceToPak, &params.RawSurfMmcState));
#endif
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_IND_OBJ_BASE_ADDR_STATE, JpegBasicFeature)
{
    params.Mode                    = CODECHAL_ENCODE_MODE_AVC;
    params.presPakBaseObjectBuffer = const_cast<PMOS_RESOURCE>(&m_resBitstreamBuffer);
    params.dwPakBaseObjectSize     = m_bitstreamUpperBound;

    return MOS_STATUS_SUCCESS;
}

uint32_t JpegBasicFeature::GetJpegHorizontalSamplingFactorForY(CodecEncodeJpegInputSurfaceFormat format)const
{
    uint32_t horizontalSamplingFactor = 1;

    if (format == codechalJpegY8)
    {
        horizontalSamplingFactor = 1;
    }
    else if (format == codechalJpegNV12)
    {
        horizontalSamplingFactor = 2;
    }
    else if (format == codechalJpegUYVY || format == codechalJpegYUY2)
    {
        horizontalSamplingFactor = 2;
    }
    else if (format == codechalJpegRGB)
    {
        horizontalSamplingFactor = 1;
    }

    return horizontalSamplingFactor;
}

uint32_t JpegBasicFeature::GetJpegVerticalSamplingFactorForY(CodecEncodeJpegInputSurfaceFormat format)const
{
    uint32_t verticalSamplingFactor = 1;

    if (format == codechalJpegY8)
    {
        verticalSamplingFactor = 1;
    }
    else if (format == codechalJpegNV12)
    {
        verticalSamplingFactor = 2;
    }
    else if (format == codechalJpegRGB ||
        format == codechalJpegUYVY ||
        format == codechalJpegYUY2)
    {
        verticalSamplingFactor = 1;
    }

    return verticalSamplingFactor;
}

MHW_SETPAR_DECL_SRC(MFX_JPEG_PIC_STATE, JpegBasicFeature)
{
    auto picParams = m_jpegPicParams;

    params.decodeInUse = false;
    params.inputSurfaceFormatYuv = (uint8_t)picParams->m_inputSurfaceFormat;

    if (picParams->m_inputSurfaceFormat == codechalJpegY8)
    {
        params.outputMcuStructure = jpegYUV400;
        params.pixelsInHorizontalLastMcu = picParams->m_picWidth % 8;
        params.pixelsInVerticalLastMcu = picParams->m_picHeight % 8;
    }
    else if (picParams->m_inputSurfaceFormat == codechalJpegNV12)
    {
        params.outputMcuStructure = jpegYUV420;

        if (picParams->m_picWidth % 2 == 0)
        {
            params.pixelsInHorizontalLastMcu = picParams->m_picWidth % 16;
        }
        else
        {
            params.pixelsInHorizontalLastMcu = (picParams->m_picWidth + 1) % 16;
        }

        if (picParams->m_picHeight % 2 == 0)
        {
            params.pixelsInVerticalLastMcu = picParams->m_picHeight % 16;
        }
        else
        {
            params.pixelsInVerticalLastMcu = (picParams->m_picHeight + 1) % 16;
        }
    }
    else if (picParams->m_inputSurfaceFormat == codechalJpegYUY2 ||
        picParams->m_inputSurfaceFormat == codechalJpegUYVY)
    {
        params.outputMcuStructure = jpegYUV422H2Y;

        if (picParams->m_picWidth % 2 == 0)
        {
            params.pixelsInHorizontalLastMcu = picParams->m_picWidth % 16;
        }
        else
        {
            params.pixelsInHorizontalLastMcu = (picParams->m_picWidth + 1) % 16;
        }

        params.pixelsInVerticalLastMcu = picParams->m_picHeight % 8;
    }
    else if (picParams->m_inputSurfaceFormat == codechalJpegRGB)
    {
        params.outputMcuStructure = jpegYUV444;
        params.pixelsInHorizontalLastMcu = picParams->m_picWidth % 8;
        params.pixelsInVerticalLastMcu = picParams->m_picHeight % 8;
    }

    uint32_t horizontalSamplingFactor = GetJpegHorizontalSamplingFactorForY((CodecEncodeJpegInputSurfaceFormat)picParams->m_inputSurfaceFormat);
    uint32_t verticalSamplingFactor = GetJpegVerticalSamplingFactorForY((CodecEncodeJpegInputSurfaceFormat)picParams->m_inputSurfaceFormat);
    params.frameWidthInBlocksMinus1 = (((picParams->m_picWidth + (horizontalSamplingFactor * 8 - 1)) / (horizontalSamplingFactor * 8)) * horizontalSamplingFactor) - 1;
    params.frameHeightInBlocksMinus1 = (((picParams->m_picHeight + (verticalSamplingFactor * 8 - 1)) / (verticalSamplingFactor * 8)) * verticalSamplingFactor) - 1;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFC_JPEG_SCAN_OBJECT, JpegBasicFeature)
{
    uint32_t horizontalSamplingFactor = GetJpegHorizontalSamplingFactorForY((CodecEncodeJpegInputSurfaceFormat)m_jpegPicParams->m_inputSurfaceFormat);
    uint32_t verticalSamplingFactor = GetJpegVerticalSamplingFactorForY((CodecEncodeJpegInputSurfaceFormat)m_jpegPicParams->m_inputSurfaceFormat);
    params.mcuCount =
        ((m_jpegPicParams->m_picWidth + (horizontalSamplingFactor * 8 - 1)) / (horizontalSamplingFactor * 8)) *
        ((m_jpegPicParams->m_picHeight + (verticalSamplingFactor * 8 - 1)) / (verticalSamplingFactor * 8));
    params.restartInterval = (uint16_t)m_jpegScanParams->m_restartInterval;

    for (auto i = 0; i < jpegNumComponent; i++)
    {
        params.huffmanDcTable |= (m_jpegScanParams->m_dcCodingTblSelector[i] << i);
        params.huffmanAcTable |= (m_jpegScanParams->m_acCodingTblSelector[i] << i);
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MI_FORCE_WAKEUP, JpegBasicFeature)
{
    params.bMFXPowerWellControl       = true;
    params.bMFXPowerWellControlMask   = true;
    params.bHEVCPowerWellControl      = false;
    params.bHEVCPowerWellControlMask  = true;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_WAIT, JpegBasicFeature)
{
    params.iStallVdboxPipeline = true;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
