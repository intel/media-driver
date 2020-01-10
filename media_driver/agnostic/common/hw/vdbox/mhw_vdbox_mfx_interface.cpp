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

//! \file     mhw_vdbox_mfx_interface.cpp
//! \brief    MHW interface for constructing MFX commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox MFX commands across all platforms 
//!

#include "mhw_vdbox_mfx_interface.h"

const uint8_t MhwVdboxMfxInterface::m_mpeg2QuantMatrixScan[64] =
{
    // Inverse Zig-Zag scan pattern
    0, 1, 5, 6, 14, 15, 27, 28,
    2, 4, 7, 13, 16, 26, 29, 42,
    3, 8, 12, 17, 25, 30, 41, 43,
    9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54,
    20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61,
    35, 36, 48, 49, 57, 58, 62, 63
};

const uint16_t MhwVdboxMfxInterface::m_mpeg2DefaultIntraQuantizerMatrix[64] =
{
    8, 16, 19, 22, 26, 27, 29, 34,
    16, 16, 22, 24, 27, 29, 34, 37,
    19, 22, 26, 27, 29, 34, 34, 38,
    22, 22, 26, 27, 29, 34, 37, 40,
    22, 26, 27, 29, 32, 35, 40, 48,
    26, 27, 29, 32, 35, 40, 48, 58,
    26, 27, 29, 34, 38, 46, 56, 69,
    27, 29, 35, 38, 46, 56, 69, 83
};

const uint16_t MhwVdboxMfxInterface::m_mpeg2DefaultNonIntraQuantizerMatrix[64] =
{
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16
};

const uint32_t MhwVdboxMfxInterface::m_mpeg2SliceDeltaQPMax[4] =
{
    0x02040608, 0x01020304, 0x01020304, 0x01010202
};

const uint32_t MhwVdboxMfxInterface::m_mpeg2InitSliceDeltaQPMin[4] =
{
    0x81828384, 0x81828384, 0x81828384, 0x81828384
};

const uint32_t MhwVdboxMfxInterface::m_mpeg2FrameBitrateMinMax[4] =
{
    0x00010000, 0x00010000, 0x00010000, 0x00010000
};

const uint32_t MhwVdboxMfxInterface::m_mpeg2FrameBitrateMinMaxDelta[4] =
{
    0x00010000, 0x00010000, 0x00010000, 0x00010000
};

const uint8_t MhwVdboxMfxInterface::m_columnScan4x4[16] =
{
    0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15
};

const uint8_t MhwVdboxMfxInterface::m_columnScan8x8[64] =
{
    0, 8, 16, 24, 32, 40, 48, 56, 1, 9, 17, 25, 33, 41, 49, 57,
    2, 10, 18, 26, 34, 42, 50, 58, 3, 11, 19, 27, 35, 43, 51, 59,
    4, 12, 20, 28, 36, 44, 52, 60, 5, 13, 21, 29, 37, 45, 53, 61,
    6, 14, 22, 30, 38, 46, 54, 62, 7, 15, 23, 31, 39, 47, 55, 63
};

const MhwVdboxMfxInterface::AvcSliceType MhwVdboxMfxInterface::m_AvcBsdSliceType[10] =
{
    MhwVdboxMfxInterface::avcSliceP,
    MhwVdboxMfxInterface::avcSliceB,
    MhwVdboxMfxInterface::avcSliceI,
    MhwVdboxMfxInterface::avcSliceP,
    MhwVdboxMfxInterface::avcSliceI,
    MhwVdboxMfxInterface::avcSliceP,
    MhwVdboxMfxInterface::avcSliceB,
    MhwVdboxMfxInterface::avcSliceI,
    MhwVdboxMfxInterface::avcSliceP,
    MhwVdboxMfxInterface::avcSliceI
};

const MhwVdboxMfxInterface::VDEncFrameDeltaTable MhwVdboxMfxInterface::m_vdEncFrameDelta100PercentTab[CODEC_AVC_NUM_QP] =
{
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1300, 1400 },
    { 1275, 1325 },
    { 1250, 1275 },
    { 1200, 1200 },
    { 1125, 1125 },
    { 1075, 1075 },
    { 1000, 1000 },
    { 800, 950 },
    { 725, 900 },
    { 650, 850 },
    { 600, 800 },
    { 575, 750 },
    { 500, 700 },
    { 425, 675 },
    { 375, 650 },
    { 350, 625 },
    { 325, 550 },
    { 275, 500 },
    { 250, 450 },
    { 225, 425 },
    { 200, 375 },
    { 180, 350 },
    { 170, 320 },
    { 160, 290 },
    { 150, 260 },
    { 140, 245 },
    { 125, 230 },
    { 110, 220 },
    { 110, 220 },
    { 110, 220 },
    { 110, 220 },
    { 110, 220 },
    { 110, 220 },
    { 110, 220 },
    { 110, 220 },
    { 110, 220 }
};

const MhwVdboxMfxInterface::VDEncFrameDeltaTable MhwVdboxMfxInterface::m_vdEncFrameDelta90PercentTab[CODEC_AVC_NUM_QP] =
{
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 1400, 1400 },
    { 960, 1400 },
    { 900, 1400 },
    { 840, 1300 },
    { 770, 1200 },
    { 710, 1100 },
    { 650, 1000 },
    { 590, 900 },
    { 560, 860 },
    { 530, 815 },
    { 500, 750 },
    { 470, 700 },
    { 430, 650 },
    { 400, 600 },
    { 380, 570 },
    { 350, 540 },
    { 320, 500 },
    { 285, 465 },
    { 270, 430 },
    { 250, 400 },
    { 235, 380 },
    { 215, 360 },
    { 200, 350 },
    { 180, 320 },
    { 160, 290 },
    { 140, 260 },
    { 130, 240 },
    { 115, 220 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 },
    { 100, 200 }
};

MhwVdboxMfxInterface::MhwVdboxMfxInterface(
    PMOS_INTERFACE osInterface,
    MhwMiInterface *miInterface,
    MhwCpInterface *cpInterface,
    bool decodeInUse)
{
    MHW_FUNCTION_ENTER;

    m_osInterface = osInterface;
    m_MiInterface = miInterface;
    m_cpInterface = cpInterface;
    m_decodeInUse = decodeInUse;

    MHW_ASSERT(m_osInterface);
    MHW_ASSERT(m_MiInterface);
    MHW_ASSERT(m_cpInterface);

    m_waTable = osInterface->pfnGetWaTable(osInterface);
    m_skuTable = osInterface->pfnGetSkuTable(osInterface);
    m_osInterface->pfnGetPlatform(m_osInterface, &m_platform);
    m_maxVdboxIndex = MEDIA_IS_SKU(m_skuTable, FtrVcs2) ? MHW_VDBOX_NODE_2 : MHW_VDBOX_NODE_1;

    if (m_osInterface->bUsesGfxAddress)
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else // bUsesPatchList
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }

    auto gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);

    if (gtSystemInfo != nullptr && (!MEDIA_IS_SKU(m_skuTable, FtrWithSlimVdbox)))
    {
        m_numVdbox = (uint8_t)(gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled);
    }
    else
    {
        m_numVdbox = 1;
    }
}

void MhwVdboxMfxInterface::CalcAvcImgStateMinMaxBitrate(
    MHW_VDBOX_AVC_IMG_BITRATE_PARAMS& params)
{
    // Set this to New mode (= 1) - FrameBitRateMaxUnit and FrameBitRateMinUnit are in new mode (32byte/4Kb)
    params.frameBitRateMaxUnitMode = params.frameBitRateMinUnitMode = 1;

    // Set this to Kilo Byte (= 1) - FrameBitRateMax and FrameBitRateMin are in units of 4KBytes when FrameBitrateMaxUnitMode and FrameBitRateMinUnitMode are set to 1
    params.frameBitRateMaxUnit = params.frameBitRateMinUnit = 1;

    // We have 14 bits available when FrameBitRateMaxUnitMode is set to 1, so set it to max value
    params.frameBitRateMax = (1 << 14) - 1;

    // Set this to min available value
    params.frameBitRateMin = 0;

    // Set frame bitrate max and min delta to 0
    params.frameBitRateMaxDelta = params.frameBitRateMinDelta = 0;

    return;
}

uint32_t MhwVdboxMfxInterface::GetViewOrder(
    PMHW_VDBOX_AVC_DPB_PARAMS params,
    uint32_t currIdx,
    uint32_t list)
{
    MHW_CHK_NULL_RETURN(params);

    auto avcPicParams = params->pAvcPicParams;
    auto mvcExtPicParams = params->pMvcExtPicParams;
    auto avcRefList = params->ppAvcRefList;

    // No need to check if bottom field since only progressive is supported
    int32_t  currPOC = avcPicParams->CurrFieldOrderCnt[0];
    uint32_t numRefs = (list == LIST_0) ? mvcExtPicParams->NumInterViewRefsL0 : mvcExtPicParams->NumInterViewRefsL1;
    uint32_t viewOrder = 0xF;
    uint32_t currRef = params->pAvcPicIdx[currIdx].ucPicIdx;

    if (params->pAvcPicIdx[currIdx].bValid &&
        avcRefList[currRef]->bUsedAsInterViewRef &&
        (currPOC == avcRefList[currRef]->iFieldOrderCnt[0]))
    {
        for (uint32_t i = 0; i < numRefs; i++)
        {
            if (mvcExtPicParams->ViewIDList[currIdx] == mvcExtPicParams->InterViewRefList[list][i])
            {
                viewOrder = mvcExtPicParams->ViewIDList[currIdx];
                break;
            }
        }
    }

    return viewOrder;
}

bool MhwVdboxMfxInterface::IsVc1IPicture(
    const CODEC_PICTURE& picture,
    bool isFirstField,
    uint16_t picType)
{
    bool isI = false;

    if (CodecHal_PictureIsField(picture))
    {
        if (picType == vc1IIField)
        {
            // I/I frame
            isI = true;
        }
        else if (picType == vc1IPField)
        {
            // I/P frame
            isI = isFirstField;
        }
        else if (picType == vc1PIField)
        {
            // P/I frame
            isI = (!isFirstField);
        }
    }
    else
    {
        isI = (picType == vc1IFrame);
    }

    return isI;
}

bool MhwVdboxMfxInterface::IsVc1PPicture(
    const CODEC_PICTURE& picture,
    bool isFirstField,
    uint16_t picType)
{
    bool isP = false;

    if (CodecHal_PictureIsField(picture))
    {
        if (picType == vc1PPField)
        {
            // P/P frame
            isP = true;
        }
        else if (picType == vc1IPField)
        {
            // I/P frame
            isP = (!isFirstField);
        }
        else if (picType == vc1PIField)
        {
            // P/I frame
            isP = isFirstField;
        }
    }
    else
    {
        isP = (picType == vc1PFrame);
    }

    return isP;
}

bool MhwVdboxMfxInterface::IsVc1BPicture(
    const CODEC_PICTURE& picture,
    bool isFirstField,
    uint16_t picType)
{
    bool isB = false;

    if (CodecHal_PictureIsField(picture))
    {
        if (picType == vc1BBIField)
        {
            // B/BI frame
            isB = isFirstField;
        }
        else if (picType == vc1BIBField)
        {
            // BI/B frame
            isB = (!isFirstField);
        }
        else if (picType == vc1BBField)
        {
            // BI/BI frame
            isB = true;
        }
    }
    else
    {
        isB = (picType == vc1BFrame);
    }

    return isB;
}

bool MhwVdboxMfxInterface::IsVc1BIPicture(
    const CODEC_PICTURE& picture,
    bool isFirstField,
    uint16_t picType)
{
    bool isBI = false;

    if (CodecHal_PictureIsField(picture))
    {
        if (picType == vc1BBIField)
        {
            // B/BI frame
            isBI = (!isFirstField);
        }
        else if (picType == vc1BIBField)
        {
            // BI/B frame
            isBI = isFirstField;
        }
        else if (picType == vc1BIBIField)
        {
            // BI/BI frame
            isBI = true;
        }
    }
    else
    {
        isBI = (picType == vc1BIFrame);
    }

    return isBI;
}

MOS_STATUS MhwVdboxMfxInterface::AddMfxAvcImgCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_AVC_IMG_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (m_decodeInUse)
    {
        MHW_MI_CHK_STATUS(AddMfxDecodeAvcImgCmd(cmdBuffer, batchBuffer, params));
    }
    else
    {
        MHW_MI_CHK_STATUS(AddMfxEncodeAvcImgCmd(cmdBuffer, batchBuffer, params));
    }

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterface::AddMfxAvcWeightOffset(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (m_decodeInUse)
    {
        MHW_MI_CHK_STATUS(AddMfxDecodeAvcWeightOffset(cmdBuffer, batchBuffer, params));
    }
    else
    {
        MHW_MI_CHK_STATUS(AddMfxEncodeAvcWeightOffset(cmdBuffer, batchBuffer, params));
    }

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterface::AddMfxAvcSlice(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_AVC_SLICE_STATE avcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (m_decodeInUse)
    {
        MHW_MI_CHK_STATUS(AddMfxDecodeAvcSlice(cmdBuffer, batchBuffer, avcSliceState));
    }
    else
    {
        MHW_MI_CHK_STATUS(AddMfxEncodeAvcSlice(cmdBuffer, batchBuffer, avcSliceState));
    }

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterface::AddMfxMpeg2PicCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_MPEG2_PIC_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (m_decodeInUse)
    {
        MHW_MI_CHK_STATUS(AddMfxDecodeMpeg2PicCmd(cmdBuffer, params));
    }
    else
    {
        MHW_MI_CHK_STATUS(AddMfxEncodeMpeg2PicCmd(cmdBuffer, params));
    }

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterface::AddMfxVp8PicCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_VP8_PIC_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (m_decodeInUse)
    {
        MHW_MI_CHK_STATUS(AddMfxDecodeVp8PicCmd(cmdBuffer, params));
    }
    else
    {
        MHW_MI_CHK_STATUS(AddMfxEncodeVp8PicCmd(cmdBuffer, params));
    }

    return eStatus;
}

MHW_VDBOX_DECODE_JPEG_FORMAT_CODE MhwVdboxMfxInterface::GetJpegDecodeFormat(MOS_FORMAT format)
{
    switch (format)
    {
    case Format_NV12:
        return MHW_VDBOX_DECODE_JPEG_FORMAT_NV12;
    case Format_UYVY:
        return MHW_VDBOX_DECODE_JPEG_FORMAT_UYVY;
    case Format_YUY2:
        return MHW_VDBOX_DECODE_JPEG_FORMAT_YUY2;
    default:
        return MHW_VDBOX_DECODE_JPEG_FORMAT_SEPARATE_PLANE;
    }

    return MHW_VDBOX_DECODE_JPEG_FORMAT_SEPARATE_PLANE;
}

bool MhwVdboxMfxInterface::IsVPlanePresent(MOS_FORMAT format)
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

uint32_t MhwVdboxMfxInterface::MosToMediaStateFormat(MOS_FORMAT format)
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

uint32_t MhwVdboxMfxInterface::GetJpegHorizontalSamplingFactorForY(
    CodecEncodeJpegInputSurfaceFormat format)
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

uint32_t MhwVdboxMfxInterface::GetJpegVerticalSamplingFactorForY(
    CodecEncodeJpegInputSurfaceFormat format)
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
