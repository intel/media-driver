/*
* Copyright (c) 2014-2019, Intel Corporation
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
//! \file     codechal_encode_sfc.cpp
//! \brief    Implements the encode interface extension for CSC via VEBox/SFC.
//! \details  Downsampling in this case is supported by the VEBox fixed function HW unit.
//!

#include "codechal_encode_sfc.h"
#include "codechal_encoder_base.h"
#include "hal_oca_interface.h"

#define CODECHAL_IS_BT601_CSPACE(format)                \
        ( (format == MHW_CSpace_BT601)               || \
          (format == MHW_CSpace_xvYCC601)            || \
          (format == MHW_CSpace_BT601Gray)           || \
          (format == MHW_CSpace_BT601_FullRange)     || \
          (format == MHW_CSpace_BT601Gray_FullRange) )

#define CODECHAL_IS_BT709_CSPACE(format)                \
        ( (format == MHW_CSpace_BT709)               || \
          (format == MHW_CSpace_xvYCC709)            || \
          (format == MHW_CSpace_BT709_FullRange) )

// Generic YUV to RGB conversion matrix from BT.601 standard
const float CODECHAL_CSC_BT601_YUV_RGB[9] =
{
    1.000000f,  0.000000f,  1.402000f,
    1.000000f, -0.344136f, -0.714136f,
    1.000000f,  1.772000f,  0.000000f
};

// Generic YUV to RGB conversion matrix from BT.709 standard
const float CODECHAL_CSC_BT709_YUV_RGB[9] =
{
    1.000000f,  0.000000f,  1.574800f,
    1.000000f, -0.187324f, -0.468124f,
    1.000000f,  1.855600f,  0.000000f
};

// Generic RGB to YUV conversion matrix from BT.601 standard
const float CODECHAL_CSC_BT601_RGB_YUV[9] =
{
    0.299000f,  0.587000f,  0.114000f,
   -0.168736f, -0.331264f,  0.500000f,
    0.500000f, -0.418688f, -0.081312f
};

// Generic RGB to YUV conversion matrix from BT.709 standard
const float CODECHAL_CSC_BT709_RGB_YUV[9] =
{
    0.212600f,  0.715200f,  0.072200f,
   -0.114572f, -0.385428f,  0.500000f,
    0.500000f, -0.454153f, -0.045847f
};

// BT2020 RGB to Non-constant YUV conversion matrix from R-REC-BT.2020-1-201406-I!!PDF-E.pdf
const float CODECHAL_CSC_BT2020_RGB_YUV[9] =
{
    0.262700f,  0.678000f,  0.059300f,     // Y
    -0.139630f, -0.360370f, 0.500000f,     // U
    0.500000f,  -0.459786f, -0.040214f     // V
};

// BT2020 Non-constant YUV to RGB conversion matrix from R-REC-BT.2020-1-201406-I!!PDF-E.pdf
const float CODECHAL_CSC_BT2020_YUV_RGB[9] =
{
    1.000000f, 0.000000f,  1.474600f,     //R
    1.000000f, -0.164553f, -0.571353f,    //G
    1.000000f, 1.881400f,  0.000000f      //B
};

CodecHalEncodeSfc::~CodecHalEncodeSfc()
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // pVeboxInterface->pfnDestroy() will be called in CodecHal_HwDestroy(), no need to destroy here

    FreeResources();
}

bool CodecHalEncodeSfc::IsCspace(MHW_CSPACE srcCspace, MHW_CSPACE dstCspace)
{
    switch (dstCspace)
    {
    case MHW_CSpace_RGB:
        return (srcCspace == MHW_CSpace_sRGB ||
            srcCspace == MHW_CSpace_stRGB);

    case MHW_CSpace_YUV:
        return (srcCspace == MHW_CSpace_BT709 ||
            srcCspace == MHW_CSpace_BT601 ||
            srcCspace == MHW_CSpace_BT601_FullRange ||
            srcCspace == MHW_CSpace_BT709_FullRange ||
            srcCspace == MHW_CSpace_xvYCC709 ||
            srcCspace == MHW_CSpace_xvYCC601);

    case MHW_CSpace_Gray:
        return (srcCspace == MHW_CSpace_BT601Gray ||
            srcCspace == MHW_CSpace_BT601Gray_FullRange);

    case MHW_CSpace_Any:
        return (srcCspace != MHW_CSpace_None);

    case MHW_CSpace_BT2020:
        return (srcCspace == MHW_CSpace_BT2020 ||
            srcCspace == MHW_CSpace_BT2020_FullRange);

    case MHW_CSpace_BT2020_RGB:
        return (srcCspace == MHW_CSpace_BT2020_RGB ||
            srcCspace == MHW_CSpace_BT2020_stRGB);

    default:
        return (srcCspace == dstCspace);
    }

    return false;
}

bool CodecHalEncodeSfc::GetRgbRangeAndOffset(
    MHW_CSPACE          srcCspace,
    float               *rgbOffset,
    float               *rgbExcursion)
{
    bool ret = true;

    switch (srcCspace)
    {
    case MHW_CSpace_sRGB:
    case MHW_CSpace_BT2020_RGB:
        *rgbOffset = 0.0f;
        *rgbExcursion = 255.0f;
        break;

    case MHW_CSpace_stRGB:
    case MHW_CSpace_BT2020_stRGB:
        *rgbOffset = 16.0f;
        *rgbExcursion = 219.0f;
        break;

    default:
        ret = false;
        break;
    }

    return ret;
}

bool CodecHalEncodeSfc::GetYuvRangeAndOffset(
    MHW_CSPACE          srcCspace,
    float               *lumaOffset,
    float               *lumaExcursion,
    float               *chromaZero,
    float               *chromaExcursion)
{
    bool ret = true;

    switch (srcCspace)
    {
    case MHW_CSpace_BT601_FullRange:
    case MHW_CSpace_BT709_FullRange:
    case MHW_CSpace_BT601Gray_FullRange:
    case MHW_CSpace_BT2020_FullRange:
        *lumaOffset = 0.0f;
        *lumaExcursion = 255.0f;
        *chromaZero = 128.0f;
        *chromaExcursion = 255.0f;
        break;

    case MHW_CSpace_BT601:
    case MHW_CSpace_BT709:
    case MHW_CSpace_xvYCC601: // since matrix is the same as 601, use the same range
    case MHW_CSpace_xvYCC709: // since matrix is the same as 709, use the same range
    case MHW_CSpace_BT601Gray:
    case MHW_CSpace_BT2020:
        *lumaOffset = 16.0f;
        *lumaExcursion = 219.0f;
        *chromaZero = 128.0f;
        *chromaExcursion = 224.0f;
        break;

    default:
        ret = false;
        break;
    }

    return ret;
}

bool CodecHalEncodeSfc::CalcYuvToRgbMatrix(
    MHW_CSPACE      srcCspace,                          // [in] YUV Color space
    MHW_CSPACE      dstCspace,                          // [in] RGB Color space
    float           *transferMatrix,                    // [in] Transfer matrix (3x3)
    float           *outMatrix)                         // [out] Conversion matrix (3x4)
{
    bool    ret = true;
    float   lumaOffset, lumaExcursion, chromaZero, chromaExcursion;
    float   rgbOffset, rgbExcursion;

    ret = GetRgbRangeAndOffset(dstCspace, &rgbOffset, &rgbExcursion);
    if (ret)
    {
        ret = GetYuvRangeAndOffset(srcCspace, &lumaOffset, &lumaExcursion, &chromaZero, &chromaExcursion);
    }
    if (ret)
    {

        // after + (3x3)(3x3)
        outMatrix[0] = transferMatrix[0] * rgbExcursion / lumaExcursion;
        outMatrix[4] = transferMatrix[3] * rgbExcursion / lumaExcursion;
        outMatrix[8] = transferMatrix[6] * rgbExcursion / lumaExcursion;
        outMatrix[1] = transferMatrix[1] * rgbExcursion / chromaExcursion;
        outMatrix[5] = transferMatrix[4] * rgbExcursion / chromaExcursion;
        outMatrix[9] = transferMatrix[7] * rgbExcursion / chromaExcursion;
        outMatrix[2] = transferMatrix[2] * rgbExcursion / chromaExcursion;
        outMatrix[6] = transferMatrix[5] * rgbExcursion / chromaExcursion;
        outMatrix[10] = transferMatrix[8] * rgbExcursion / chromaExcursion;

        // (3x1) - (3x3)(3x3)(3x1)
        outMatrix[3] = rgbOffset - (outMatrix[0] * lumaOffset + outMatrix[1] * chromaZero + outMatrix[2] * chromaZero);
        outMatrix[7] = rgbOffset - (outMatrix[4] * lumaOffset + outMatrix[5] * chromaZero + outMatrix[6] * chromaZero);
        outMatrix[11] = rgbOffset - (outMatrix[8] * lumaOffset + outMatrix[9] * chromaZero + outMatrix[10] * chromaZero);
    }
    return ret;
}

bool CodecHalEncodeSfc::CalcRgbToYuvMatrix(
    MHW_CSPACE      srcCspace,                      // [in] RGB Color space
    MHW_CSPACE      dstCspace,                      // [in] YUV Color space
    float           *transferMatrix,                // [in] Transfer matrix (3x3)
    float           *outMatrix)                     // [out] Conversion matrix (3x4)
{
    bool    ret = true;
    float   lumaOffset, lumaExcursion, chromaZero, chromaExcursion;
    float   rgbOffset, rgbExcursion;

    ret = GetRgbRangeAndOffset(srcCspace, &rgbOffset, &rgbExcursion);
    if (ret)
    {
        ret = GetYuvRangeAndOffset(dstCspace, &lumaOffset, &lumaExcursion, &chromaZero, &chromaExcursion);
    }
    if (ret)
    {
        // multiplication of + onwards
        outMatrix[0] = transferMatrix[0] * lumaExcursion / rgbExcursion;
        outMatrix[1] = transferMatrix[1] * lumaExcursion / rgbExcursion;
        outMatrix[2] = transferMatrix[2] * lumaExcursion / rgbExcursion;
        outMatrix[4] = transferMatrix[3] * chromaExcursion / rgbExcursion;
        outMatrix[5] = transferMatrix[4] * chromaExcursion / rgbExcursion;
        outMatrix[6] = transferMatrix[5] * chromaExcursion / rgbExcursion;
        outMatrix[8] = transferMatrix[6] * chromaExcursion / rgbExcursion;
        outMatrix[9] = transferMatrix[7] * chromaExcursion / rgbExcursion;
        outMatrix[10] = transferMatrix[8] * chromaExcursion / rgbExcursion;

        // before +
        outMatrix[3] = lumaOffset - lumaExcursion * rgbOffset / rgbExcursion;
        outMatrix[7] = chromaZero;
        outMatrix[11] = chromaZero;
    }
    return ret;
}

void CodecHalEncodeSfc::GetCSCMatrix(
    MHW_CSPACE          srcCspace,                      // [in] Source Color space
    MHW_CSPACE          dstCspace,                      // [in] Destination Color space
    float               *cscMatrix)                     // [out] CSC matrix to use
{
    int32_t         i;

    // BT601/709 YUV to sRGB/stRGB conversion
    if (IsCspace(srcCspace, MHW_CSpace_YUV))
    {
        if(IsCspace(dstCspace, MHW_CSpace_RGB))
        {
            if (CODECHAL_IS_BT601_CSPACE(srcCspace))
            {
                CalcYuvToRgbMatrix(srcCspace, dstCspace, (float *) CODECHAL_CSC_BT601_YUV_RGB, cscMatrix);
            }
            else // if (IS_BT709_CSPACE(srcCspace))
            {
                CalcYuvToRgbMatrix(srcCspace, dstCspace, (float *) CODECHAL_CSC_BT709_YUV_RGB, cscMatrix);
            }
        }
    }
    // sRGB/stRGB to BT601/709 YUV conversion
    else if (IsCspace(srcCspace, MHW_CSpace_RGB))
    {
        if (IsCspace(dstCspace, MHW_CSpace_YUV))
        {
            if (CODECHAL_IS_BT601_CSPACE(dstCspace))
            {
                CalcRgbToYuvMatrix(srcCspace, dstCspace, (float *)CODECHAL_CSC_BT601_RGB_YUV, cscMatrix);
            }
            else // if (IS_BT709_CSPACE(srcCspace))
            {
                CalcRgbToYuvMatrix(srcCspace, dstCspace, (float *)CODECHAL_CSC_BT709_RGB_YUV, cscMatrix);
            }
        }
    }
    // BT2020 YUV to RGB conversion
    else if (IsCspace(srcCspace, MHW_CSpace_BT2020))
    {
        if (IsCspace(dstCspace, MHW_CSpace_BT2020_RGB))
        {
            CalcYuvToRgbMatrix(srcCspace, dstCspace, (float *)CODECHAL_CSC_BT2020_YUV_RGB, cscMatrix);
        }
    }
    // BT2020 RGB to YUV conversion
    else if (IsCspace(srcCspace, MHW_CSpace_BT2020_RGB))
    {
        if (IsCspace(dstCspace, MHW_CSpace_BT2020))
        {
            CalcRgbToYuvMatrix(srcCspace, dstCspace, (float *)CODECHAL_CSC_BT2020_RGB_YUV, cscMatrix);
        }
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Not supported color space conversion(from %d to %d)", srcCspace, dstCspace);
    }

    CODECHAL_ENCODE_NORMALMESSAGE("");
    for(i = 0; i < 3; i++)
    {
        CODECHAL_ENCODE_NORMALMESSAGE("%f\t%f\t%f\t%f",
                            cscMatrix[4 * i],
                            cscMatrix[4 * i + 1],
                            cscMatrix[4 * i + 2],
                            cscMatrix[4 * i + 3]);
    }
}

void CodecHalEncodeSfc::GetCscMatrix(
    MHW_CSPACE             srcCspace,                                    // [in] Source Cspace
    MHW_CSPACE             dstCspace,                                    // [in] Destination Cspace
    float                  *cscCoeff,                                    // [out] [3x3] Coefficients matrix
    float                  *cscInOffset,                                 // [out] [3x1] Input Offset matrix
    float                  *cscOutOffset)                                // [out] [3x1] Output Offset matrix
{
    float   cscMatrix[12];
    int32_t i;

    GetCSCMatrix(
        srcCspace,
        dstCspace,
        cscMatrix);

    // Copy [3x3] into Coeff
    for (i = 0; i < 3; i++)
    {
        MOS_SecureMemcpy(
            &cscCoeff[i*3],
            sizeof(float) * 3,
            &cscMatrix[i*4],
            sizeof(float) * 3);
    }

    // Get the input offsets
    switch(srcCspace)
    {
        case MHW_CSpace_BT601:
        case MHW_CSpace_BT601Gray:
        case MHW_CSpace_xvYCC601:
        case MHW_CSpace_BT709:
        case MHW_CSpace_xvYCC709:
            cscInOffset[0] =  -16.0F;
            cscInOffset[1] = -128.0F;
            cscInOffset[2] = -128.0F;
            break;

        case MHW_CSpace_BT601_FullRange:
        case MHW_CSpace_BT601Gray_FullRange:
        case MHW_CSpace_BT709_FullRange:
            cscInOffset[0] =    0.0F;
            cscInOffset[1] = -128.0F;
            cscInOffset[2] = -128.0F;
            break;

        case MHW_CSpace_sRGB:
            cscInOffset[0] = 0.0F;
            cscInOffset[1] = 0.0F;
            cscInOffset[2] = 0.0F;
            break;

        case MHW_CSpace_stRGB:
            cscInOffset[0] = -16.0F;
            cscInOffset[1] = -16.0F;
            cscInOffset[2] = -16.0F;
            break;

        //BT2020 YUV->RGB
        case MHW_CSpace_BT2020:
            cscInOffset[0] = -16.0F;
            cscInOffset[1] = -128.0F;
            cscInOffset[2] = -128.0F;
            break;

        case MHW_CSpace_BT2020_FullRange:
            cscInOffset[0] =    0.0F;
            cscInOffset[1] = -128.0F;
            cscInOffset[2] = -128.0F;
            break;

        //BT2020 RGB->YUV
        case MHW_CSpace_BT2020_RGB:
            cscInOffset[0] = 0.0F;
            cscInOffset[1] = 0.0F;
            cscInOffset[2] = 0.0F;
            break;

        //BT2020 RGB->YUV
        case MHW_CSpace_BT2020_stRGB:
            cscInOffset[0] = -16.0F;
            cscInOffset[1] = -16.0F;
            cscInOffset[2] = -16.0F;
            break;

        default:
            CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported Input ColorSpace for Vebox.");
    }

    // Get the output offsets
    switch(dstCspace)
    {
        case MHW_CSpace_BT601:
        case MHW_CSpace_BT601Gray:
        case MHW_CSpace_xvYCC601:
        case MHW_CSpace_BT709:
        case MHW_CSpace_xvYCC709:
            cscOutOffset[0] =  16.0F;
            cscOutOffset[1] = 128.0F;
            cscOutOffset[2] = 128.0F;
            break;

        case MHW_CSpace_BT601_FullRange:
        case MHW_CSpace_BT601Gray_FullRange:
        case MHW_CSpace_BT709_FullRange:
            cscOutOffset[0] =   0.0F;
            cscOutOffset[1] = 128.0F;
            cscOutOffset[2] = 128.0F;
            break;

        case MHW_CSpace_sRGB:
            cscOutOffset[0] = 0.0F;
            cscOutOffset[1] = 0.0F;
            cscOutOffset[2] = 0.0F;
            break;

        case MHW_CSpace_stRGB:
            cscOutOffset[0] = 16.0F;
            cscOutOffset[1] = 16.0F;
            cscOutOffset[2] = 16.0F;
            break;

        //BT2020 RGB->YUV
        case MHW_CSpace_BT2020:
            cscOutOffset[0] = 16.0F;
            cscOutOffset[1] = 128.0F;
            cscOutOffset[2] = 128.0F;
            break;

        case MHW_CSpace_BT2020_FullRange:
            cscOutOffset[0] =   0.0F;
            cscOutOffset[1] = 128.0F;
            cscOutOffset[2] = 128.0F;
            break;

        case MHW_CSpace_BT2020_RGB:
            cscOutOffset[0] = 0.0F;
            cscOutOffset[1] = 0.0F;
            cscOutOffset[2] = 0.0F;
            break;

        case MHW_CSpace_BT2020_stRGB:
            cscOutOffset[0] = 16.0F;
            cscOutOffset[1] = 16.0F;
            cscOutOffset[2] = 16.0F;
            break;

        default:
            CODECHAL_PUBLIC_ASSERTMESSAGE("Unsupported Output ColorSpace for Vebox.");
    }
}

MOS_STATUS CodecHalEncodeSfc::AllocateResources()
{
    MOS_ALLOC_GFXRES_PARAMS    allocParamsForBufferLinear;
    uint32_t                   ycoeffTableSize;
    uint32_t                   uvcoeffTableSize;
    int32_t                    size;
    char*                      ptr;
    MOS_STATUS                 eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);

    // Allocate AVS line buffer
    if (Mos_ResourceIsNull(&m_resAvsLineBuffer))
    {
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = MOS_ROUNDUP_DIVIDE(m_inputSurface->dwHeight, 8) * 5 * MHW_SFC_CACHELINE_SIZE;

        allocParamsForBufferLinear.pBufName = "SfcAvsLineBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resAvsLineBuffer));
    }

    //Initialize AVS parameters, try to do once
    if (m_scaling && !m_avsParams.piYCoefsX)
    {
        m_avsParams.Format    = Format_None;
        m_avsParams.fScaleX   = 0.0F;
        m_avsParams.fScaleY   = 0.0F;
        m_avsParams.piYCoefsX = nullptr;

        ycoeffTableSize  = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9;
        uvcoeffTableSize = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9;

        size = (ycoeffTableSize + uvcoeffTableSize) * 2;

        ptr = (char*)MOS_AllocAndZeroMemory(size);
        CODECHAL_ENCODE_CHK_NULL_RETURN(ptr);

        m_avsParams.piYCoefsX  = (int32_t*)ptr;

        ptr += ycoeffTableSize;
        m_avsParams.piUVCoefsX = (int32_t*)ptr;

        ptr += uvcoeffTableSize;
        m_avsParams.piYCoefsY  = (int32_t*)ptr;

        ptr += ycoeffTableSize;
        m_avsParams.piUVCoefsY = (int32_t*)ptr;
    }

    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::FreeResources()
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);

    // Free AVS Line Buffer
    m_osInterface->pfnFreeResource(m_osInterface, &m_resAvsLineBuffer);

    // Free resLaceOrAceOrRgbHistogram
    m_osInterface->pfnFreeResource(m_osInterface, &m_resLaceOrAceOrRgbHistogram);

    // Free resStatisticsOutput
    m_osInterface->pfnFreeResource(m_osInterface, &m_resStatisticsOutput);

    // Free buffers in AVS parameters
    MOS_FreeMemory(m_avsParams.piYCoefsX);
    m_avsParams.piYCoefsX = nullptr;

    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::SetVeboxStateParams(
    PMHW_VEBOX_STATE_CMD_PARAMS         params)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    params->bNoUseVeboxHeap                         = 0;

    params->VeboxMode.ColorGamutExpansionEnable     = 0;
    params->VeboxMode.ColorGamutCompressionEnable   = 0;
    // On SKL, GlobalIECP must be enabled when the output pipe is Vebox or SFC
    params->VeboxMode.GlobalIECPEnable              = 1;
    params->VeboxMode.DNEnable                      = 0;
    params->VeboxMode.DIEnable                      = 0;
    params->VeboxMode.DNDIFirstFrame                = 0;
    params->VeboxMode.DIOutputFrames                = 0;
    params->VeboxMode.PipeSynchronizeDisable        = 0;
    params->VeboxMode.DemosaicEnable                = 0;
    params->VeboxMode.VignetteEnable                = 0;
    params->VeboxMode.AlphaPlaneEnable              = 0;
    params->VeboxMode.HotPixelFilteringEnable       = 0;
    // 0-both slices enabled   1-Slice 0 enabled   2-Slice 1 enabled
    // On SKL GT3 and GT4, there are 2 Veboxes. But only Vebox0 can be used,Vebox1 cannot be used
    params->VeboxMode.SingleSliceVeboxEnable        = 1;
    params->VeboxMode.LACECorrectionEnable          = 0;
    params->VeboxMode.DisableEncoderStatistics      = 1;
    params->VeboxMode.DisableTemporalDenoiseFilter  = 1;
    params->VeboxMode.SinglePipeIECPEnable          = 0;
    params->VeboxMode.SFCParallelWriteEnable        = 0;
    params->VeboxMode.ScalarMode                    = 0;
    params->VeboxMode.ForwardGammaCorrectionEnable  = 0;

    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::SetVeboxSurfaceStateParams(
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS         params)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    // Initialize SurfInput
    params->SurfInput.bActive                = true;
    params->SurfInput.Format                 = m_inputSurface->Format;
    params->SurfInput.dwWidth                = m_inputSurface->dwWidth;
    params->SurfInput.dwHeight               = m_inputSurface->dwHeight;
    params->SurfInput.dwPitch                = m_inputSurface->dwPitch;
    params->SurfInput.TileType               = m_inputSurface->TileType;
    params->SurfInput.TileModeGMM            = m_inputSurface->TileModeGMM;
    params->SurfInput.bGMMTileEnabled        = m_inputSurface->bGMMTileEnabled;
    params->SurfInput.dwYoffset              = m_inputSurface->YPlaneOffset.iYOffset;
    params->SurfInput.pOsResource            = &m_inputSurface->OsResource;
    params->SurfInput.rcMaxSrc.left          = m_inputSurfaceRegion.X;
    params->SurfInput.rcMaxSrc.top           = m_inputSurfaceRegion.Y;
    params->SurfInput.rcMaxSrc.right         = m_inputSurfaceRegion.X + m_inputSurfaceRegion.Width;
    params->SurfInput.rcMaxSrc.bottom        = m_inputSurfaceRegion.Y + m_inputSurfaceRegion.Height;

    // Initialize SurfSTMM
    params->SurfSTMM.dwPitch                 = m_inputSurface->dwPitch;

    params->bDIEnable                        = false;
    params->bOutputValid                     = (m_veboxOutputSurface != nullptr) ? true : false;

    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::SetVeboxDiIecpParams(
    PMHW_VEBOX_DI_IECP_CMD_PARAMS         params)
{
    uint32_t                    width;
    uint32_t                    height;
    MOS_ALLOC_GFXRES_PARAMS     allocParamsForBufferLinear;
    uint32_t                    size = 0, sizeLace = 0, sizeNoLace = 0;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_MEMCOMP_STATE           mmcMode = MOS_MEMCOMP_DISABLED;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    height = m_inputSurface->dwHeight;
    width  = m_inputSurface->dwWidth;

    params->dwStartingX             = 0;
    params->dwEndingX               = width - 1;
    params->dwCurrInputSurfOffset   = m_inputSurface->dwOffset;
    params->pOsResCurrInput         = &m_inputSurface->OsResource;
    params->CurrInputSurfCtrl.Value = 0;  //Keep it here untill VPHAL moving to new CMD definition and remove this parameter definition.

    CodecHalGetResourceInfo(
        m_osInterface,
        m_inputSurface);

    m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &m_inputSurface->OsResource, &mmcMode);
    if (mmcMode &&
        (m_inputSurface->TileType == MOS_TILE_Y ||
         m_inputSurface->TileType == MOS_TILE_YS))
    {
        m_inputSurface->bCompressible = true;
        m_inputSurface->CompressionMode = (MOS_RESOURCE_MMC_MODE)mmcMode;
    }
    else
    {
        m_inputSurface->CompressionMode = MOS_MMC_DISABLED;
    }

    params->CurInputSurfMMCState = (MOS_MEMCOMP_STATE)(m_inputSurface->CompressionMode);

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&m_resLaceOrAceOrRgbHistogram))
    {
        size = CODECHAL_SFC_VEBOX_RGB_HISTOGRAM_SIZE;

        sizeLace = MOS_ROUNDUP_DIVIDE(height, 64) *
            MOS_ROUNDUP_DIVIDE(width, 64)  *
            CODECHAL_SFC_VEBOX_LACE_HISTOGRAM_256_BIN_PER_BLOCK;

        sizeNoLace = CODECHAL_SFC_VEBOX_ACE_HISTOGRAM_SIZE_PER_FRAME_PER_SLICE *
            CODECHAL_SFC_NUM_FRAME_PREVIOUS_CURRENT                   *
            CODECHAL_SFC_VEBOX_MAX_SLICES;

        size += MOS_MAX(sizeLace, sizeNoLace);

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = size;
        allocParamsForBufferLinear.pBufName = "ResLaceOrAceOrRgbHistogram";

        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resLaceOrAceOrRgbHistogram);
    }

    params->pOsResLaceOrAceOrRgbHistogram = &m_resLaceOrAceOrRgbHistogram;

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&m_resStatisticsOutput))
    {
        width = MOS_ALIGN_CEIL(width, 64);
        height = MOS_ROUNDUP_DIVIDE(height, 4) + MOS_ROUNDUP_DIVIDE(CODECHAL_SFC_VEBOX_STATISTICS_SIZE * sizeof(uint32_t), width);
        size = width * height;

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = size;
        allocParamsForBufferLinear.pBufName = "ResStatisticsOutput";

        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resStatisticsOutput);
    }

    params->pOsResStatisticsOutput = &m_resStatisticsOutput;

    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::VeboxSetIecpParams(
    PMHW_VEBOX_IECP_PARAMS         mhwVeboxIecpParams)
{
    // Calculate matrix if not done so before. CSC is expensive!
    if ((m_cscInputCspace != m_inputSurfaceColorSpace) ||
        (m_cscOutputCspace != m_outputSurfaceColorSpace))
    {
        float       fTemp[3];

        // Get the matrix to use for conversion
        GetCscMatrix(
            m_inputSurfaceColorSpace,
            m_outputSurfaceColorSpace,
            m_cscCoeff,
            m_cscInOffset,
            m_cscOutOffset);

        // Vebox CSC converts RGB input to YUV for SFC
        // Vebox only supports A8B8G8R8 input, swap the 1st and 3rd
        // columns of the transfer matrix for A8R8G8B8 and X8R8G8B8
        // This only happens when SFC output is used
        if ((m_inputSurface->Format == Format_A8R8G8B8) ||
            (m_inputSurface->Format == Format_X8R8G8B8))
        {
            fTemp[0] = m_cscCoeff[0];
            fTemp[1] = m_cscCoeff[3];
            fTemp[2] = m_cscCoeff[6];

            m_cscCoeff[0] = m_cscCoeff[2];
            m_cscCoeff[3] = m_cscCoeff[5];
            m_cscCoeff[6] = m_cscCoeff[8];

            m_cscCoeff[2] = fTemp[0];
            m_cscCoeff[5] = fTemp[1];
            m_cscCoeff[8] = fTemp[2];
        }
    }
    // Store it for next BLT
    m_cscInputCspace   = m_inputSurfaceColorSpace;
    m_cscOutputCspace  = m_outputSurfaceColorSpace;
    CODECHAL_ENCODE_VERBOSEMESSAGE("Input color space: %d, output color space: %d",
                                        m_cscInputCspace, m_cscOutputCspace);

    // copy into MHW parameters

    mhwVeboxIecpParams->ColorSpace          = m_inputSurfaceColorSpace;
    mhwVeboxIecpParams->dstFormat           = m_sfcOutputSurface->Format;
    mhwVeboxIecpParams->srcFormat           = m_inputSurface->Format;
    mhwVeboxIecpParams->bCSCEnable          = m_veboxCsc;
    mhwVeboxIecpParams->pfCscCoeff          = m_cscCoeff;
    mhwVeboxIecpParams->pfCscInOffset       = m_cscInOffset;
    mhwVeboxIecpParams->pfCscOutOffset      = m_cscOutOffset;
    //mhwVeboxIecpParams->bAlphaEnable      = m_alphaEnable;    ??
    //mhwVeboxIecpParams->wAlphaValue       = m_alphaValue;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalEncodeSfc::SetSfcStateParams(
    PMHW_SFC_INTERFACE             sfcInterface,
    PMHW_SFC_STATE_PARAMS          params,
    PMHW_SFC_OUT_SURFACE_PARAMS    outSurfaceParams)
{
    uint16_t                    widthAlignUnit;
    uint16_t                    heightAlignUnit;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(sfcInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_inputSurface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_sfcOutputSurface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    params->sfcPipeMode                    = MEDIASTATE_SFC_PIPE_VE_TO_SFC;
    params->dwAVSFilterMode                = MEDIASTATE_SFC_AVS_FILTER_8x8;
    params->dwVDVEInputOrderingMode        = MEDIASTATE_SFC_INPUT_ORDERING_VE_4x8;

    params->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_444;

    // Adjust SFC input surface alignment.
    // As VEBOX doesn't do scaling, input size equals to output size
    // For the VEBOX output to SFC, width is multiple of 16 and height is multiple of 4
    widthAlignUnit                         = sfcInterface->m_veWidthAlignment;
    heightAlignUnit                        = sfcInterface->m_veHeightAlignment;

    params->dwInputFrameWidth              = MOS_ALIGN_CEIL(m_inputSurface->dwWidth, widthAlignUnit);
    params->dwInputFrameHeight             = MOS_ALIGN_CEIL(m_inputSurface->dwHeight, heightAlignUnit);

    params->dwChromaDownSamplingMode       = MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_444TO420;
    params->bAVSChromaUpsamplingEnable     = m_scaling;

    if ((params->fAVSXScalingRatio > 1.0F) || (params->fAVSYScalingRatio > 1.0F))
    {
        params->bBypassXAdaptiveFilter     = false;
        params->bBypassYAdaptiveFilter     = false;
    }
    else
    {
        params->bBypassXAdaptiveFilter     = true;
        params->bBypassYAdaptiveFilter     = true;
    }

    params->fChromaSubSamplingXSiteOffset  = 0.0F;
    params->fChromaSubSamplingYSiteOffset  = 0.0F;

    widthAlignUnit  = 1;
    heightAlignUnit = 1;

    switch(m_sfcOutputSurface->Format)
    {
        case Format_NV12:
        case Format_P010:
            widthAlignUnit     = 2;
            heightAlignUnit    = 2;
            break;
        case Format_YUY2:
        case Format_UYVY:
            widthAlignUnit     = 2;
            break;
        default:
            break;
    }

    // Default to Horizontal Left, Vertical Top
    params->dwChromaDownSamplingHorizontalCoef = (m_chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                  ((m_chromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 :
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);

    params->dwChromaDownSamplingVerticalCoef =   (m_chromaSiting  & MHW_CHROMA_SITING_VERT_CENTER) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                  ((m_chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 :
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);

    outSurfaceParams->dwWidth              = m_sfcOutputSurface->dwWidth;
    outSurfaceParams->dwHeight             = m_sfcOutputSurface->dwHeight;
    outSurfaceParams->dwPitch              = m_sfcOutputSurface->dwPitch;
    outSurfaceParams->TileType             = m_sfcOutputSurface->TileType;
    outSurfaceParams->TileModeGMM          = m_sfcOutputSurface->TileModeGMM;
    outSurfaceParams->bGMMTileEnabled      = m_sfcOutputSurface->bGMMTileEnabled;
    outSurfaceParams->ChromaSiting         = m_chromaSiting;
    outSurfaceParams->dwUYoffset           = m_sfcOutputSurface->UPlaneOffset.iYOffset;

    params->dwOutputFrameWidth             = MOS_ALIGN_CEIL(m_sfcOutputSurface->dwWidth, widthAlignUnit);
    params->dwOutputFrameHeight            = MOS_ALIGN_CEIL(m_sfcOutputSurface->dwHeight, heightAlignUnit);
    params->OutputFrameFormat              = m_sfcOutputSurface->Format;
    params->dwOutputSurfaceOffset          = m_sfcOutputSurface->dwOffset;
    params->pOsResOutputSurface            = &m_sfcOutputSurface->OsResource;
    params->pOsResAVSLineBuffer            = &m_resAvsLineBuffer;

    params->dwSourceRegionHeight           = MOS_ALIGN_FLOOR(m_inputSurfaceRegion.Height, heightAlignUnit);
    params->dwSourceRegionWidth            = MOS_ALIGN_FLOOR(m_inputSurfaceRegion.Width, widthAlignUnit);
    params->dwSourceRegionVerticalOffset   = MOS_ALIGN_CEIL(m_inputSurfaceRegion.Y, heightAlignUnit);
    params->dwSourceRegionHorizontalOffset = MOS_ALIGN_CEIL(m_inputSurfaceRegion.X, widthAlignUnit);
    params->dwScaledRegionHeight           = MOS_UF_ROUND(m_scaleY * params->dwSourceRegionHeight);
    params->dwScaledRegionWidth            = MOS_UF_ROUND(m_scaleX * params->dwSourceRegionWidth);
    params->dwScaledRegionVerticalOffset   = MOS_ALIGN_FLOOR(m_outputSurfaceRegion.Y, heightAlignUnit);
    params->dwScaledRegionHorizontalOffset = MOS_ALIGN_FLOOR(m_outputSurfaceRegion.X, widthAlignUnit);
    params->fAVSXScalingRatio              = m_scaleX;
    params->fAVSYScalingRatio              = m_scaleY;

    params->fAlphaPixel                    = 1.0F;
    params->bColorFillEnable               = m_colorFill;
    params->bCSCEnable                     = m_CSC;

    // ARGB8,ABGR10,A16B16G16R16,VYUY and YVYU output format need to enable swap
    if (m_sfcOutputSurface->Format == Format_X8R8G8B8     ||
        m_sfcOutputSurface->Format == Format_A8R8G8B8     ||
        m_sfcOutputSurface->Format == Format_R10G10B10A2  ||
        m_sfcOutputSurface->Format == Format_A16B16G16R16 ||
        m_sfcOutputSurface->Format == Format_VYUY         ||
        m_sfcOutputSurface->Format == Format_YVYU)
    {
        params->bRGBASwapEnable = true;
    }
    else
    {
        params->bRGBASwapEnable = false;
    }


    // CodecHal does not support SFC rotation
    params->RotationMode                   = MHW_ROTATION_IDENTITY;

    // For downsampling, expect output surface to be MMC disabled
    // For Jpeg, the only usage is CSC and the output surface format is RGB8, so also disable MMC
    params->bMMCEnable                     = false;
    params->MMCMode                        = MOS_MMC_DISABLED;

    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::SetSfcAvsStateParams(
    PMHW_SFC_INTERFACE             sfcInterface)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PMHW_SFC_AVS_STATE          mhwSfcAvsState;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(sfcInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_inputSurface);

    mhwSfcAvsState = &m_avsState;

    if (m_chromaSiting == MHW_CHROMA_SITING_NONE)
    {
        m_chromaSiting = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_CENTER;
    }

    mhwSfcAvsState->dwInputHorizontalSiting = (m_chromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                               ((m_chromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                               SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

    mhwSfcAvsState->dwInputVerticalSitting = (m_chromaSiting  & MHW_CHROMA_SITING_VERT_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                              ((m_chromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                              SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->SetSfcSamplerTable(
                                &m_lumaTable,
                                &m_chromaTable,
                                &m_avsParams,
                                m_inputSurface->Format,
                                m_scaleX,
                                m_scaleY,
                                m_chromaSiting,
                                true));
    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::SetSfcIefStateParams(
    PMHW_SFC_IEF_STATE_PARAMS         params)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    params->bIEFEnable     = false;
    params->bCSCEnable     = true;

    params->pfCscCoeff     = m_cscCoeff;
    params->pfCscInOffset  = m_cscInOffset;
    params->pfCscOutOffset = m_cscOutOffset;

    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::Initialize(
    CodechalHwInterface                *hwInterface,
    PMOS_INTERFACE                      osInterface)
{

    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hwInterface->GetVeboxInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(osInterface);

    m_hwInterface = hwInterface;
    m_osInterface = osInterface;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetVeboxInterface()->CreateHeap());

    // Create VEBOX Context
    MOS_GPUCTX_CREATOPTIONS createOption;
    //
    // VeboxgpuContext could be created from both VP and Codec.
    // If there is no such as a GPU context it will create a new one and set the GPU component ID. 
    // If there has been a valid GPU context it won’t create another one anymore and the component ID won’t be updated either.
    // Therefore if a codec veboxgpu context creation happens earlier than a vp veboxgpu context creation and set its component ID to MOS_GPU_COMPONENT_ENCODE,
    // VPBLT callstack would index a GpuAppTaskEvent of MOS_GPU_COMPONENT_ENCODE.
    //
    MOS_COMPONENT originalComponent = m_osInterface->Component;
    m_osInterface->Component        = COMPONENT_VPCommon;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        MOS_GPU_CONTEXT_VEBOX,
        MOS_GPU_NODE_VE,
        &createOption));

    m_osInterface->Component        = originalComponent;

    // Register Vebox GPU context with the Batch Buffer completion event
    // Ignore if creation fails
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        MOS_GPU_CONTEXT_VEBOX));

    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::SetParams(
    CODECHAL_ENCODE_SFC_PARAMS*         params)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pInputSurface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOutputSurface);

    m_inputSurface = params->pInputSurface;
    // Vebox o/p should not be written to memory for SFC, VeboxOutputSurface should be nullptr
    m_veboxOutputSurface = nullptr;
    m_sfcOutputSurface = params->pOutputSurface;

    // no scaling.
    m_scaling = false;
    m_colorFill = false;

    // No CSC for SFC pipe
    m_veboxCsc = true;

    m_scaleX = 1.0;
    m_scaleY = 1.0;
    m_chromaSiting = params->uiChromaSitingType;

    eStatus = MOS_SecureMemcpy(&m_inputSurfaceRegion,
        sizeof(m_inputSurfaceRegion),
        &params->rcInputSurfaceRegion,
        sizeof(params->rcInputSurfaceRegion));

    eStatus = MOS_SecureMemcpy(&m_outputSurfaceRegion,
        sizeof(m_outputSurfaceRegion),
        &params->rcOutputSurfaceRegion,
        sizeof(params->rcOutputSurfaceRegion));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources());

    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::AddSfcCommands(
    PMHW_SFC_INTERFACE              sfcInterface,
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MHW_SFC_LOCK_PARAMS            sfcLockParams;
    MHW_SFC_STATE_PARAMS           sfcStateParams;
    MHW_SFC_OUT_SURFACE_PARAMS     sfcOutSurfaceParams;
    MHW_SFC_IEF_STATE_PARAMS       sfcIefStateParams;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(sfcInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    MOS_ZeroMemory(&sfcLockParams, sizeof(sfcLockParams));

    sfcLockParams.sfcPipeMode     = MhwSfcInterface::SFC_PIPE_MODE_VEBOX;
    sfcLockParams.bOutputToMemory = false;

    MOS_ZeroMemory(&sfcStateParams, sizeof(sfcStateParams));
    MOS_ZeroMemory(&sfcOutSurfaceParams, sizeof(sfcOutSurfaceParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSfcStateParams(sfcInterface, &sfcStateParams, &sfcOutSurfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcLock(cmdBuffer, &sfcLockParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcState(cmdBuffer, &sfcStateParams, &sfcOutSurfaceParams));

    if (m_scaling)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSfcAvsStateParams(sfcInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcAvsState(cmdBuffer, &m_avsState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcAvsLumaTable(cmdBuffer, &m_lumaTable));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcAvsChromaTable(cmdBuffer, &m_chromaTable));
    }

    if (m_CSC)
    {
        MOS_ZeroMemory(&sfcIefStateParams, sizeof(sfcIefStateParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSfcIefStateParams(&sfcIefStateParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcIefState(cmdBuffer, &sfcIefStateParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(sfcInterface->AddSfcFrameStart(cmdBuffer, MhwSfcInterface::SFC_PIPE_MODE_VEBOX));

    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc::RenderStart(
    CodechalEncoderState*       encoder)
{
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  veboxSurfaceStateCmdParams;
    MHW_VEBOX_DI_IECP_CMD_PARAMS        veboxDiIecpCmdParams;
    MHW_VEBOX_IECP_PARAMS               veboxIecpParams;
    MHW_VEBOX_SURFACE_CNTL_PARAMS       veboxSurfCntlParams;
    MhwVeboxInterface                   *veboxInterface;
    PMHW_SFC_INTERFACE                  sfcInterface;
    MhwMiInterface                      *miInterface;
    MOS_COMMAND_BUFFER                  cmdBuffer;
    bool                                requestFrameTracking;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(encoder);
    CODECHAL_ENCODE_CHK_NULL_RETURN(sfcInterface = m_hwInterface->GetSfcInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(veboxInterface = m_hwInterface->GetVeboxInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(miInterface = m_hwInterface->GetMiInterface());

    // Switch GPU context to VEBOX
    m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VEBOX);
    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(m_osInterface);

    // Send command buffer header at the beginning
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // the first task?
    requestFrameTracking = false;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(encoder->SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));

    // If m_pollingSyncEnabled is set, insert HW semaphore to wait for external
    // raw surface processing to complete, before start CSC. Once the marker in
    // raw surface is overwritten by external operation, HW semaphore will be
    // signalled and CSC will start. This is to reduce SW latency between 
    // external raw surface processing and CSC, in usages like remote gaming.
    if (encoder->m_pollingSyncEnabled)
    {
        MHW_MI_SEMAPHORE_WAIT_PARAMS miSemaphoreWaitParams;
        MOS_ZeroMemory((&miSemaphoreWaitParams), sizeof(miSemaphoreWaitParams));
        miSemaphoreWaitParams.presSemaphoreMem = &m_inputSurface->OsResource;
        miSemaphoreWaitParams.dwResourceOffset = encoder->m_syncMarkerOffset;
        miSemaphoreWaitParams.bPollingWaitMode = true;
        miSemaphoreWaitParams.dwSemaphoreData  = encoder->m_syncMarkerValue;
        miSemaphoreWaitParams.CompareOperation = MHW_MI_SAD_NOT_EQUAL_SDD;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiSemaphoreWaitCmd(&cmdBuffer, &miSemaphoreWaitParams));
    }

    // Setup cmd prameters
    MOS_ZeroMemory(&veboxStateCmdParams, sizeof(veboxStateCmdParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetVeboxStateParams(&veboxStateCmdParams));

    MOS_ZeroMemory(&veboxSurfaceStateCmdParams, sizeof(veboxSurfaceStateCmdParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetVeboxSurfaceStateParams(&veboxSurfaceStateCmdParams));

    MOS_ZeroMemory(&veboxDiIecpCmdParams, sizeof(veboxDiIecpCmdParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetVeboxDiIecpParams(&veboxDiIecpCmdParams));

    MOS_ZeroMemory(&veboxSurfCntlParams, sizeof(veboxSurfCntlParams));
    veboxSurfCntlParams.bIsCompressed   = m_inputSurface->bIsCompressed;
    veboxSurfCntlParams.CompressionMode = m_inputSurface->CompressionMode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(veboxInterface->AddVeboxSurfaceControlBits(
        &veboxSurfCntlParams,
        (uint32_t *)&(veboxDiIecpCmdParams.CurrInputSurfCtrl.Value)));

    // get csc matrix
    MOS_ZeroMemory(&veboxIecpParams, sizeof(veboxIecpParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VeboxSetIecpParams(&veboxIecpParams));

    // send matrix into heap
    CODECHAL_ENCODE_CHK_STATUS_RETURN(veboxInterface->AddVeboxIecpState(
        &veboxIecpParams));

    // send Vebox and SFC cmds
    CODECHAL_ENCODE_CHK_STATUS_RETURN(veboxInterface->AddVeboxState(&cmdBuffer, &veboxStateCmdParams, 0));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(veboxInterface->AddVeboxSurfaces( &cmdBuffer, &veboxSurfaceStateCmdParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddSfcCommands(sfcInterface, &cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(veboxInterface->AddVeboxDiIecp(&cmdBuffer, &veboxDiIecpCmdParams));

    // If m_pollingSyncEnabled is set, write the marker to source surface for next MI_SEMAPHORE_WAIT to check.
    if (encoder->m_pollingSyncEnabled)
    {
        MHW_MI_STORE_DATA_PARAMS storeDataParams;
        storeDataParams.pOsResource      = &m_inputSurface->OsResource;
        storeDataParams.dwResourceOffset = encoder->m_syncMarkerOffset;
        storeDataParams.dwValue          = encoder->m_syncMarkerValue;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &storeDataParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(encoder->GetDebugInterface()->DumpCmdBuffer(
        &cmdBuffer,
        CODECHAL_MEDIA_STATE_CSC_DS_COPY,
        nullptr)));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        encoder->m_videoContextUsesNullHw));

    return eStatus;
}
