/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     codechal_encode_sfc.c
//! \brief    Implements the encode interface extension for CSC via VEBox/SFC.
//! \details  Downsampling in this case is supported by the VEBox fixed function HW unit.
//!

#include "codechal_encode_sfc.h"
#include "codechal_encoder_base.h"

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

bool CodecHalSfc_IsCspace(MHW_CSPACE srcCspace, MHW_CSPACE dstCspace)
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

bool CodecHalSfc_GetRgbRangeAndOffset(
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

bool CodecHalSfc_GetYuvRangeAndOffset(
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

/*----------------------------------------------------------------------------
| Name      : KernelDll_CalcYuvToRgbMatrix
| Purpose   : Given the YUV->RGB transfer matrix, get the final matrix after
|             applying offsets and excursions.
|
| [R']     [R_o]                                 [R_e/Y_e    0       0   ]  [Y'  - Y_o]
| [G']  =  [R_o] + [YUVtoRGBCoeff (3x3 matrix)]. [   0    R_e/C_e    0   ]. [Cb' - C_z]
| [B']     [R_o]                                 [   0       0    R_e/C_e]. [Cr' - C_z]
|
| [R']  = [C0  C1   C2] [Y' ]   [C3]      {Out pMatrix}
| [G']  = [C4  C5   C6].[Cb'] + [C7]
| [B']  = [C8  C9  C10] [Cr'] + [C11]
|
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool CodecHalSfc_CalcYuvToRgbMatrix(
    MHW_CSPACE      SrcCspace,                          // [in] YUV Color space 
    MHW_CSPACE      DstCspace,                          // [in] RGB Color space
    float           *transferMatrix,                    // [in] Transfer matrix (3x3)
    float           *outMatrix)                         // [out] Conversion matrix (3x4)
{
    bool    ret;
    float   lumaOffset, lumaExcursion, chromaZero, chromaExcursion;
    float   rgbOffset, rgbExcursion;

    ret = true;

    ret = CodecHalSfc_GetRgbRangeAndOffset(DstCspace, &rgbOffset, &rgbExcursion);
    if (ret == false) 
    {
        goto finish;
    }

    ret = CodecHalSfc_GetYuvRangeAndOffset(SrcCspace, &lumaOffset, &lumaExcursion, &chromaZero, &chromaExcursion);
    if (ret == false) 
    {
        goto finish;
    }

    // after + (3x3)(3x3)
    outMatrix[0]  = transferMatrix[0] * rgbExcursion / lumaExcursion;
    outMatrix[4]  = transferMatrix[3] * rgbExcursion / lumaExcursion;
    outMatrix[8]  = transferMatrix[6] * rgbExcursion / lumaExcursion;
    outMatrix[1]  = transferMatrix[1] * rgbExcursion / chromaExcursion;
    outMatrix[5]  = transferMatrix[4] * rgbExcursion / chromaExcursion;
    outMatrix[9]  = transferMatrix[7] * rgbExcursion / chromaExcursion;
    outMatrix[2]  = transferMatrix[2] * rgbExcursion / chromaExcursion;
    outMatrix[6]  = transferMatrix[5] * rgbExcursion / chromaExcursion;
    outMatrix[10] = transferMatrix[8] * rgbExcursion / chromaExcursion;

    // (3x1) - (3x3)(3x3)(3x1)
    outMatrix[3]  = rgbOffset - (outMatrix[0] * lumaOffset + outMatrix[1] * chromaZero + outMatrix[2]  * chromaZero);
    outMatrix[7]  = rgbOffset - (outMatrix[4] * lumaOffset + outMatrix[5] * chromaZero + outMatrix[6]  * chromaZero);
    outMatrix[11] = rgbOffset - (outMatrix[8] * lumaOffset + outMatrix[9] * chromaZero + outMatrix[10] * chromaZero);

finish:
    return ret;
}

bool CodecHalSfc_CalcRgbToYuvMatrix(
    MHW_CSPACE      SrcCspace,                      // [in] RGB Color space 
    MHW_CSPACE      DstCspace,                      // [in] YUV Color space
    float           *transferMatrix,                // [in] Transfer matrix (3x3)
    float           *outMatrix)                     // [out] Conversion matrix (3x4)
{
    bool    ret;
    float   lumaOffset, lumaExcursion, chromaZero, chromaExcursion;
    float   rgbOffset, rgbExcursion;

    ret = true;

    ret = CodecHalSfc_GetRgbRangeAndOffset(SrcCspace, &rgbOffset, &rgbExcursion);
    if (ret == false)
    {
        goto finish;
    }

    ret = CodecHalSfc_GetYuvRangeAndOffset(DstCspace, &lumaOffset, &lumaExcursion, &chromaZero, &chromaExcursion);
    if (ret == false)
    {
        goto finish;
    }

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

finish:
    return ret;
}

void CodecHalSfc_GetCSCMatrix(
    MHW_CSPACE          SrcCspace,                      // [in] Source Color space
    MHW_CSPACE          DstCspace,                      // [in] Destination Color space
    float               *cscMatrix)                     // [out] CSC matrix to use
{
    int32_t         i;

    // BT601/709 YUV to sRGB/stRGB conversion
    if (CodecHalSfc_IsCspace(SrcCspace, MHW_CSpace_YUV))
    {
        if(CodecHalSfc_IsCspace(DstCspace, MHW_CSpace_RGB))
        {
            if (CODECHAL_IS_BT601_CSPACE(SrcCspace))
            {
                CodecHalSfc_CalcYuvToRgbMatrix(SrcCspace, DstCspace, (float *) CODECHAL_CSC_BT601_YUV_RGB, cscMatrix);
            }
            else // if (IS_BT709_CSPACE(SrcCspace))
            {
                CodecHalSfc_CalcYuvToRgbMatrix(SrcCspace, DstCspace, (float *) CODECHAL_CSC_BT709_YUV_RGB, cscMatrix);
            }
        }
    }
    // sRGB/stRGB to BT601/709 YUV conversion
    else if (CodecHalSfc_IsCspace(SrcCspace, MHW_CSpace_RGB))
    {
        if (CodecHalSfc_IsCspace(DstCspace, MHW_CSpace_YUV))
        {
            if (CODECHAL_IS_BT601_CSPACE(DstCspace))
            {
                CodecHalSfc_CalcRgbToYuvMatrix(SrcCspace, DstCspace, (float *)CODECHAL_CSC_BT601_RGB_YUV, cscMatrix);
            }
            else // if (IS_BT709_CSPACE(SrcCspace))
            {
                CodecHalSfc_CalcRgbToYuvMatrix(SrcCspace, DstCspace, (float *)CODECHAL_CSC_BT709_RGB_YUV, cscMatrix);
            }
        }        
    }
    // BT2020 YUV to RGB conversion
    else if (CodecHalSfc_IsCspace(SrcCspace, MHW_CSpace_BT2020))
    {
        if (CodecHalSfc_IsCspace(DstCspace, MHW_CSpace_BT2020_RGB))
        {
            CodecHalSfc_CalcYuvToRgbMatrix(SrcCspace, DstCspace, (float *)CODECHAL_CSC_BT2020_YUV_RGB, cscMatrix);
        }
    }
    // BT2020 RGB to YUV conversion
    else if (CodecHalSfc_IsCspace(SrcCspace, MHW_CSpace_BT2020_RGB))
    {
        if (CodecHalSfc_IsCspace(DstCspace, MHW_CSpace_BT2020))
        {
            CodecHalSfc_CalcRgbToYuvMatrix(SrcCspace, DstCspace, (float *)CODECHAL_CSC_BT2020_RGB_YUV, cscMatrix);
        }
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Not supported color space conversion(from %d to %d)", SrcCspace, DstCspace);
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

void CodecHal_GetCscMatrix(
    MHW_CSPACE             SrcCspace,                                    // [in] Source Cspace
    MHW_CSPACE             DstCspace,                                    // [in] Destination Cspace
    float                  *cscCoeff,                                    // [out] [3x3] Coefficients matrix
    float                  *cscInOffset,                                 // [out] [3x1] Input Offset matrix
    float                  *cscOutOffset)                                // [out] [3x1] Output Offset matrix
{
    float   cscMatrix[12];
    int32_t i;

    CodecHalSfc_GetCSCMatrix(
        SrcCspace,
        DstCspace,
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
    switch(SrcCspace)
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
    switch(DstCspace)
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

MOS_STATUS CodecHalEncodeSfc_AllocateResources(
    PMOS_INTERFACE                     osInterface,
    PCODECHAL_ENCODE_SFC_STATE         sfcState)
{
    MOS_ALLOC_GFXRES_PARAMS    allocParamsForBufferLinear;
    uint32_t                   ycoeffTableSize;
    uint32_t                   uvcoeffTableSize;
    int32_t                    size;
    char*                      ptr;
    MOS_STATUS                 eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(osInterface);
    CODECHAL_ENCODE_CHK_NULL(sfcState);

    // Allocate AVS line buffer
    if (Mos_ResourceIsNull(&sfcState->resAvsLineBuffer))
    {
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format   = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = MOS_ROUNDUP_DIVIDE(sfcState->pInputSurface->dwHeight, 8) * 5 * MHW_SFC_CACHELINE_SIZE;

        allocParamsForBufferLinear.pBufName = "SfcAvsLineBuffer";

        eStatus = (MOS_STATUS)osInterface->pfnAllocateResource(
            osInterface,
            &allocParamsForBufferLinear,
            &sfcState->resAvsLineBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("Failed to allocate Sfc Avs Line Buffer.");
            goto finish;
        }
    }

    //Initialize AVS parameters, try to do once
    if (sfcState->bScaling && !sfcState->AvsParams.piYCoefsX)
    {
        sfcState->AvsParams.Format    = Format_None;
        sfcState->AvsParams.fScaleX   = 0.0F;
        sfcState->AvsParams.fScaleY   = 0.0F;
        sfcState->AvsParams.piYCoefsX = nullptr;

        ycoeffTableSize  = POLYPHASE_Y_COEFFICIENT_TABLE_SIZE_G9;
        uvcoeffTableSize = POLYPHASE_UV_COEFFICIENT_TABLE_SIZE_G9;

        size = (ycoeffTableSize + uvcoeffTableSize) * 2;

        ptr = (char*)MOS_AllocAndZeroMemory(size);
        if (ptr == nullptr)
        {
            CODECHAL_PUBLIC_ASSERTMESSAGE("No memory to allocate AVS coefficient tables.");
            eStatus = MOS_STATUS_NO_SPACE;
            goto finish;
        }

        sfcState->AvsParams.piYCoefsX  = (int32_t*)ptr; 

        ptr += ycoeffTableSize;
        sfcState->AvsParams.piUVCoefsX = (int32_t*)ptr;

        ptr += uvcoeffTableSize;
        sfcState->AvsParams.piYCoefsY  = (int32_t*)ptr;

        ptr += ycoeffTableSize;
        sfcState->AvsParams.piUVCoefsY = (int32_t*)ptr;
    }

finish:
    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc_FreeResources(
    PMOS_INTERFACE                     osInterface,
    PCODECHAL_ENCODE_SFC_STATE         sfcState)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(osInterface);
    CODECHAL_ENCODE_CHK_NULL(sfcState);

    // Free AVS Line Buffer
    osInterface->pfnFreeResource(osInterface, &sfcState->resAvsLineBuffer);

    // Free resLaceOrAceOrRgbHistogram
    osInterface->pfnFreeResource(osInterface, &sfcState->resLaceOrAceOrRgbHistogram);

    // Free resStatisticsOutput
    osInterface->pfnFreeResource(osInterface, &sfcState->resStatisticsOutput);

    // Free buffers in AVS parameters
    MOS_FreeMemory(sfcState->AvsParams.piYCoefsX);
    sfcState->AvsParams.piYCoefsX = nullptr;

finish:
    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc_SetVeboxStateParams(
    PMHW_VEBOX_STATE_CMD_PARAMS         params)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(params);

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

finish:
    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc_SetVeboxSurfaceStateParams(
    PCODECHAL_ENCODE_SFC_STATE                  sfcState,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS         params)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(sfcState);
    CODECHAL_ENCODE_CHK_NULL(params);

    // Initialize SurfInput
    params->SurfInput.bActive                = true;
    params->SurfInput.Format                 = sfcState->pInputSurface->Format;
    params->SurfInput.dwWidth                = sfcState->pInputSurface->dwWidth;
    params->SurfInput.dwHeight               = sfcState->pInputSurface->dwHeight;
    params->SurfInput.dwPitch                = sfcState->pInputSurface->dwPitch;
    params->SurfInput.TileType               = sfcState->pInputSurface->TileType;
    params->SurfInput.pOsResource            = &sfcState->pInputSurface->OsResource;
    params->SurfInput.rcMaxSrc.left          = sfcState->rcInputSurfaceRegion.X;
    params->SurfInput.rcMaxSrc.top           = sfcState->rcInputSurfaceRegion.Y;
    params->SurfInput.rcMaxSrc.right         = sfcState->rcInputSurfaceRegion.X + sfcState->rcInputSurfaceRegion.Width;
    params->SurfInput.rcMaxSrc.bottom        = sfcState->rcInputSurfaceRegion.Y + sfcState->rcInputSurfaceRegion.Height;

    // Initialize SurfSTMM
    params->SurfSTMM.dwPitch                 = sfcState->pInputSurface->dwPitch;

    params->bDIEnable                        = false;
    params->bOutputValid                     = (sfcState->pVeboxOutputSurface != nullptr) ? true : false;

finish:
    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc_SetVeboxDiIecpParams(
    PMOS_INTERFACE                        osInterface,
    PCODECHAL_ENCODE_SFC_STATE            sfcState,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS         params)
{
    uint32_t                    width;
    uint32_t                    height;
    MOS_ALLOC_GFXRES_PARAMS     allocParamsForBufferLinear;
    uint32_t                    size = 0, sizeLace = 0, sizeNoLace = 0;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(sfcState);
    CODECHAL_ENCODE_CHK_NULL(params);

    height = sfcState->pInputSurface->dwHeight;
    width  = sfcState->pInputSurface->dwWidth;

    params->dwStartingX             = 0;
    params->dwEndingX               = width - 1;
    params->dwCurrInputSurfOffset   = sfcState->pInputSurface->dwOffset;
    params->pOsResCurrInput         = &sfcState->pInputSurface->OsResource;
    params->CurrInputSurfCtrl.Value = 0;  //Keep it here untill VPHAL moving to new CMD definition and remove this parameter definition.

    CodecHal_GetResourceInfo(
        osInterface,
        sfcState->pInputSurface);

    params->CurInputSurfMMCState = (MOS_MEMCOMP_STATE)(sfcState->pInputSurface->CompressionMode);

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&sfcState->resLaceOrAceOrRgbHistogram))
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

        osInterface->pfnAllocateResource(
            osInterface,
            &allocParamsForBufferLinear,
            &sfcState->resLaceOrAceOrRgbHistogram);
    }

    params->pOsResLaceOrAceOrRgbHistogram = &sfcState->resLaceOrAceOrRgbHistogram;

    // Allocate Resource to avoid Page Fault issue since HW will access it
    if (Mos_ResourceIsNull(&sfcState->resStatisticsOutput))
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

        osInterface->pfnAllocateResource(
            osInterface,
            &allocParamsForBufferLinear,
            &sfcState->resStatisticsOutput);
    }

    params->pOsResStatisticsOutput = &sfcState->resStatisticsOutput;


finish:
    return eStatus;
}


// input -> RGB (from app)
// output -> NV12 (raw surface?)
MOS_STATUS CodecHalEncodeSfc_VeboxSetIecpParams(
    PMOS_INTERFACE                 osInterface,
    PCODECHAL_ENCODE_SFC_STATE     sfcState,
    PMHW_VEBOX_IECP_PARAMS         mhwVeboxIecpParams)
{
    // Calculate matrix if not done so before. CSC is expensive!
    if ((sfcState->CscInputCspace != sfcState->InputSurfaceColorSpace) ||
        (sfcState->CscOutputCspace != sfcState->OutputSurfaceColorSpace))
    {
        float       fTemp[3];

        // Get the matrix to use for conversion
        CodecHal_GetCscMatrix(
            sfcState->InputSurfaceColorSpace,
            sfcState->OutputSurfaceColorSpace,
            sfcState->fCscCoeff,
            sfcState->fCscInOffset,
            sfcState->fCscOutOffset);

        // Vebox CSC converts RGB input to YUV for SFC
        // Vebox only supports A8B8G8R8 input, swap the 1st and 3rd
        // columns of the transfer matrix for A8R8G8B8 and X8R8G8B8
        // This only happens when SFC output is used
        if ((sfcState->pInputSurface->Format == Format_A8R8G8B8) ||
            (sfcState->pInputSurface->Format == Format_X8R8G8B8))
        {
            fTemp[0] = sfcState->fCscCoeff[0];
            fTemp[1] = sfcState->fCscCoeff[3];
            fTemp[2] = sfcState->fCscCoeff[6];

            sfcState->fCscCoeff[0] = sfcState->fCscCoeff[2];
            sfcState->fCscCoeff[3] = sfcState->fCscCoeff[5];
            sfcState->fCscCoeff[6] = sfcState->fCscCoeff[8];

            sfcState->fCscCoeff[2] = fTemp[0];
            sfcState->fCscCoeff[5] = fTemp[1];
            sfcState->fCscCoeff[8] = fTemp[2];
        }
    }
    // Store it for next BLT
    sfcState->CscInputCspace   = sfcState->InputSurfaceColorSpace;
    sfcState->CscOutputCspace  = sfcState->OutputSurfaceColorSpace;
    CODECHAL_ENCODE_VERBOSEMESSAGE("Input color space: %d, output color space: %d",
                                        sfcState->CscInputCspace, sfcState->CscOutputCspace);

    // copy into MHW parameters

    mhwVeboxIecpParams->ColorSpace          = sfcState->InputSurfaceColorSpace;
    mhwVeboxIecpParams->dstFormat           = sfcState->pSfcOutputSurface->Format;
    mhwVeboxIecpParams->srcFormat           = sfcState->pInputSurface->Format;
    mhwVeboxIecpParams->bCSCEnable          = sfcState->bVeboxCsc;
    mhwVeboxIecpParams->pfCscCoeff          = sfcState->fCscCoeff;
    mhwVeboxIecpParams->pfCscInOffset       = sfcState->fCscInOffset;
    mhwVeboxIecpParams->pfCscOutOffset      = sfcState->fCscOutOffset;
    //mhwVeboxIecpParams->bAlphaEnable        = sfcState->bAlphaEnable;    ??
    //mhwVeboxIecpParams->wAlphaValue         = sfcState->wAlphaValue;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalEncodeSfc_SetSfcStateParams(
    PMHW_SFC_INTERFACE             sfcInterface,
    PCODECHAL_ENCODE_SFC_STATE     sfcState,
    PMHW_SFC_STATE_PARAMS          params,
    PMHW_SFC_OUT_SURFACE_PARAMS    outSurfaceParams)
{
    uint16_t                    widthAlignUnit;
    uint16_t                    heightAlignUnit;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(sfcInterface);
    CODECHAL_ENCODE_CHK_NULL(sfcState);
    CODECHAL_ENCODE_CHK_NULL(sfcState->pInputSurface);
    CODECHAL_ENCODE_CHK_NULL(sfcState->pSfcOutputSurface);
    CODECHAL_ENCODE_CHK_NULL(params);

    params->sfcPipeMode                    = MEDIASTATE_SFC_PIPE_VE_TO_SFC;
    params->dwAVSFilterMode                = MEDIASTATE_SFC_AVS_FILTER_8x8;
    params->dwVDVEInputOrderingMode        = MEDIASTATE_SFC_INPUT_ORDERING_VE_4x8;

    params->dwInputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_444;

    // Adjust SFC input surface alignment.
    // As VEBOX doesn't do scaling, input size equals to output size
    // For the VEBOX output to SFC, width is multiple of 16 and height is multiple of 4
    widthAlignUnit                         = sfcInterface->m_veWidthAlignment;
    heightAlignUnit                        = sfcInterface->m_veHeightAlignment;

    params->dwInputFrameWidth              = MOS_ALIGN_CEIL(sfcState->pInputSurface->dwWidth, widthAlignUnit);
    params->dwInputFrameHeight             = MOS_ALIGN_CEIL(sfcState->pInputSurface->dwHeight, heightAlignUnit);

    params->dwChromaDownSamplingMode       = MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_444TO420;
    params->bAVSChromaUpsamplingEnable     = sfcState->bScaling;

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

    switch(sfcState->pSfcOutputSurface->Format)
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
    params->dwChromaDownSamplingHorizontalCoef = (sfcState->uiChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? 
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                  ((sfcState->uiChromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 :
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);

    params->dwChromaDownSamplingVerticalCoef =   (sfcState->uiChromaSiting  & MHW_CHROMA_SITING_VERT_CENTER) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 :
                                                  ((sfcState->uiChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ?
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_8_OVER_8 :
                                                  MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8);

    outSurfaceParams->dwWidth              = sfcState->pSfcOutputSurface->dwWidth;
    outSurfaceParams->dwHeight             = sfcState->pSfcOutputSurface->dwHeight;
    outSurfaceParams->dwPitch              = sfcState->pSfcOutputSurface->dwPitch;
    outSurfaceParams->TileType             = sfcState->pSfcOutputSurface->TileType;
    outSurfaceParams->ChromaSiting         = sfcState->uiChromaSiting;
    outSurfaceParams->dwUYoffset           = sfcState->pSfcOutputSurface->UPlaneOffset.iYOffset;

    params->dwOutputFrameWidth             = MOS_ALIGN_CEIL(sfcState->pSfcOutputSurface->dwWidth, widthAlignUnit);
    params->dwOutputFrameHeight            = MOS_ALIGN_CEIL(sfcState->pSfcOutputSurface->dwHeight, heightAlignUnit);
    params->OutputFrameFormat              = sfcState->pSfcOutputSurface->Format;
    params->dwOutputSurfaceOffset          = sfcState->pSfcOutputSurface->dwOffset;
    params->pOsResOutputSurface            = &sfcState->pSfcOutputSurface->OsResource;
    params->pOsResAVSLineBuffer            = &sfcState->resAvsLineBuffer;

    params->dwSourceRegionHeight           = MOS_ALIGN_FLOOR(sfcState->rcInputSurfaceRegion.Height, heightAlignUnit);
    params->dwSourceRegionWidth            = MOS_ALIGN_FLOOR(sfcState->rcInputSurfaceRegion.Width, widthAlignUnit);
    params->dwSourceRegionVerticalOffset   = MOS_ALIGN_CEIL(sfcState->rcInputSurfaceRegion.Y, heightAlignUnit);
    params->dwSourceRegionHorizontalOffset = MOS_ALIGN_CEIL(sfcState->rcInputSurfaceRegion.X, widthAlignUnit);
    params->dwScaledRegionHeight           = MOS_UF_ROUND(sfcState->fScaleY * params->dwSourceRegionHeight);
    params->dwScaledRegionWidth            = MOS_UF_ROUND(sfcState->fScaleX * params->dwSourceRegionWidth);
    params->dwScaledRegionVerticalOffset   = MOS_ALIGN_FLOOR(sfcState->rcOutputSurfaceRegion.Y, heightAlignUnit);
    params->dwScaledRegionHorizontalOffset = MOS_ALIGN_FLOOR(sfcState->rcOutputSurfaceRegion.X, widthAlignUnit);
    params->fAVSXScalingRatio              = sfcState->fScaleX;
    params->fAVSYScalingRatio              = sfcState->fScaleY;

    params->fAlphaPixel                    = 1.0F;
    params->bColorFillEnable               = sfcState->bColorFill;
    params->bCSCEnable                     = sfcState->bCSC;
    params->bRGBASwapEnable                = params->bCSCEnable;

    // CodecHal does not support SFC rotation
    params->RotationMode                   = MHW_ROTATION_IDENTITY; 

    // For downsampling, expect output surface to be MMC disabled
    // For Jpeg, the only usage is CSC and the output surface format is RGB8, so also disable MMC
    params->bMMCEnable                     = false; 
    params->MMCMode                        = MOS_MMC_DISABLED;

finish:
    return eStatus;
    }

MOS_STATUS CodecHalEncodeSfc_SetSfcAvsStateParams(
    PMHW_SFC_INTERFACE             sfcInterface,
    PCODECHAL_ENCODE_SFC_STATE     sfcState)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PMHW_SFC_AVS_STATE          mhwSfcAvsState;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(sfcInterface);
    CODECHAL_ENCODE_CHK_NULL(sfcState);
    CODECHAL_ENCODE_CHK_NULL(sfcState->pInputSurface);

    mhwSfcAvsState = &sfcState->AvsState;

    if (sfcState->uiChromaSiting == MHW_CHROMA_SITING_NONE)
    {
        sfcState->uiChromaSiting = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_CENTER;
    }

    mhwSfcAvsState->dwInputHorizontalSiting = (sfcState->uiChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                               ((sfcState->uiChromaSiting & MHW_CHROMA_SITING_HORZ_RIGHT) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                               SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

    mhwSfcAvsState->dwInputVerticalSitting = (sfcState->uiChromaSiting  & MHW_CHROMA_SITING_VERT_CENTER) ? SFC_AVS_INPUT_SITING_COEF_4_OVER_8 :
                                              ((sfcState->uiChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM) ? SFC_AVS_INPUT_SITING_COEF_8_OVER_8 :
                                              SFC_AVS_INPUT_SITING_COEF_0_OVER_8);

    CODECHAL_ENCODE_CHK_STATUS(sfcInterface->SetSfcSamplerTable(
                                &sfcState->LumaTable,
                                &sfcState->ChromaTable,
                                &sfcState->AvsParams,
                                sfcState->pInputSurface->Format,
                                sfcState->fScaleX,
                                sfcState->fScaleY,
                                sfcState->uiChromaSiting,
                                true));

finish:
    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc_SetSfcIefStateParams(
    PCODECHAL_ENCODE_SFC_STATE        sfcState,
    PMHW_SFC_IEF_STATE_PARAMS         params)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(sfcState);
    CODECHAL_ENCODE_CHK_NULL(params);

    params->bIEFEnable     = false;
    params->bCSCEnable     = true;

    params->pfCscCoeff     = sfcState->fCscCoeff;
    params->pfCscInOffset  = sfcState->fCscInOffset;
    params->pfCscOutOffset = sfcState->fCscOutOffset;

finish:
    return eStatus;
}


MOS_STATUS CodecHalEncodeSfc_Initialize(
    CodechalHwInterface                *hwInterface,
    PMOS_INTERFACE                      osInterface)
{

    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(hwInterface);
    CODECHAL_ENCODE_CHK_NULL(hwInterface->GetVeboxInterface());
    CODECHAL_ENCODE_CHK_NULL(osInterface);

    CODECHAL_ENCODE_CHK_STATUS(hwInterface->GetVeboxInterface()->CreateHeap());

    // Create VEBOX Context
    CODECHAL_ENCODE_CHK_STATUS(osInterface->pfnCreateGpuContext(
        osInterface,
        MOS_GPU_CONTEXT_VEBOX,
        MOS_GPU_NODE_VE,
        MOS_GPU_CONTEXT_CREATE_DEFAULT));

    // Register Vebox GPU context with the Batch Buffer completion event
    // Ignore if creation fails
    CODECHAL_ENCODE_CHK_STATUS(osInterface->pfnRegisterBBCompleteNotifyEvent(
        osInterface,
        MOS_GPU_CONTEXT_VEBOX));

finish:
    return eStatus;
}


MOS_STATUS CodecHalEncodeSfc_Destroy(
    CodechalHwInterface            *hwInterface,
    PMOS_INTERFACE                  osInterface,
    PCODECHAL_ENCODE_SFC_STATE      sfcState)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(hwInterface);
    CODECHAL_ENCODE_CHK_NULL(hwInterface->GetVeboxInterface());
    CODECHAL_ENCODE_CHK_NULL(osInterface);
    CODECHAL_ENCODE_CHK_NULL(sfcState);

    // pVeboxInterface->pfnDestroy() will be called in CodecHal_HwDestroy(), no need to destroy here

    eStatus = CodecHalEncodeSfc_FreeResources(osInterface, sfcState);

    MOS_ZeroMemory(sfcState, sizeof(*sfcState));

finish:
    return eStatus;
}

// call every frame.  get the input/output surface and  color space...
MOS_STATUS CodecHalEncodeSfc_SetParams(
    PMOS_INTERFACE                      osInterface,
    PCODECHAL_ENCODE_SFC_STATE          sfcState,
    CODECHAL_ENCODE_SFC_PARAMS*         params)
{
    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(osInterface);
    CODECHAL_ENCODE_CHK_NULL(sfcState);
    CODECHAL_ENCODE_CHK_NULL(params);
    CODECHAL_ENCODE_CHK_NULL(params->pInputSurface);
    CODECHAL_ENCODE_CHK_NULL(params->pOutputSurface);

    sfcState->pInputSurface = params->pInputSurface;
    // Vebox o/p should not be written to memory for SFC, VeboxOutputSurface should be nullptr
    sfcState->pVeboxOutputSurface = nullptr;
    sfcState->pSfcOutputSurface = params->pOutputSurface;

    // no scaling.
    sfcState->bScaling = false;
    sfcState->bColorFill = false;

    // No CSC for SFC pipe
    sfcState->bVeboxCsc = true;

    sfcState->fScaleX = 1.0;
    sfcState->fScaleY = 1.0;
    sfcState->uiChromaSiting = params->uiChromaSitingType;

    eStatus = MOS_SecureMemcpy(&sfcState->rcInputSurfaceRegion,
        sizeof(sfcState->rcInputSurfaceRegion),
        &params->rcInputSurfaceRegion,
        sizeof(params->rcInputSurfaceRegion));

    eStatus = MOS_SecureMemcpy(&sfcState->rcOutputSurfaceRegion,
        sizeof(sfcState->rcOutputSurfaceRegion),
        &params->rcOutputSurfaceRegion,
        sizeof(params->rcOutputSurfaceRegion));

    CODECHAL_ENCODE_CHK_STATUS(CodecHalEncodeSfc_AllocateResources(osInterface, sfcState));

finish:
    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc_AddSfcCommands(
    PMHW_SFC_INTERFACE              sfcInterface,
    PCODECHAL_ENCODE_SFC_STATE      sfcState,
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MHW_SFC_LOCK_PARAMS            sfcLockParams;
    MHW_SFC_STATE_PARAMS           sfcStateParams;
    MHW_SFC_OUT_SURFACE_PARAMS     sfcOutSurfaceParams;
    MHW_SFC_IEF_STATE_PARAMS       sfcIefStateParams;
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(sfcInterface);
    CODECHAL_ENCODE_CHK_NULL(sfcState);
    CODECHAL_ENCODE_CHK_NULL(cmdBuffer);

    MOS_ZeroMemory(&sfcLockParams, sizeof(sfcLockParams));

    sfcLockParams.sfcPipeMode     = MhwSfcInterface::SFC_PIPE_MODE_VEBOX;
    sfcLockParams.bOutputToMemory = false;

    MOS_ZeroMemory(&sfcStateParams, sizeof(sfcStateParams));
    MOS_ZeroMemory(&sfcOutSurfaceParams, sizeof(sfcOutSurfaceParams));
    CODECHAL_ENCODE_CHK_STATUS(CodecHalEncodeSfc_SetSfcStateParams(sfcInterface, sfcState, &sfcStateParams, &sfcOutSurfaceParams));

    CODECHAL_ENCODE_CHK_STATUS(sfcInterface->AddSfcLock(cmdBuffer, &sfcLockParams));
    CODECHAL_ENCODE_CHK_STATUS(sfcInterface->AddSfcState(cmdBuffer, &sfcStateParams, &sfcOutSurfaceParams));

    if (sfcState->bScaling)
    {
        CODECHAL_ENCODE_CHK_STATUS(CodecHalEncodeSfc_SetSfcAvsStateParams(sfcInterface, sfcState));
        CODECHAL_ENCODE_CHK_STATUS(sfcInterface->AddSfcAvsState(cmdBuffer, &sfcState->AvsState));
        CODECHAL_ENCODE_CHK_STATUS(sfcInterface->AddSfcAvsLumaTable(cmdBuffer, &sfcState->LumaTable));
        CODECHAL_ENCODE_CHK_STATUS(sfcInterface->AddSfcAvsChromaTable(cmdBuffer, &sfcState->ChromaTable));
    }

    if (sfcState->bCSC)
    {
        MOS_ZeroMemory(&sfcIefStateParams, sizeof(sfcIefStateParams));
        CODECHAL_ENCODE_CHK_STATUS(CodecHalEncodeSfc_SetSfcIefStateParams(sfcState, &sfcIefStateParams));
        CODECHAL_ENCODE_CHK_STATUS(sfcInterface->AddSfcIefState(cmdBuffer, &sfcIefStateParams));
    }

    CODECHAL_ENCODE_CHK_STATUS(sfcInterface->AddSfcFrameStart(cmdBuffer, MhwSfcInterface::SFC_PIPE_MODE_VEBOX));

finish:
    return eStatus;
}

MOS_STATUS CodecHalEncodeSfc_RenderStart(
    CodechalHwInterface        *hwInterface,
    PMOS_INTERFACE              osInterface,
    CodechalEncoderState*       encoder,
    PCODECHAL_ENCODE_SFC_STATE  sfcState)
{
    MHW_VEBOX_STATE_CMD_PARAMS          veboxStateCmdParams;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS  veboxSurfaceStateCmdParams;
    MHW_VEBOX_DI_IECP_CMD_PARAMS        veboxDiIecpCmdParams;
    MHW_VEBOX_IECP_PARAMS               veboxIecpParams;
    MhwVeboxInterface                   *veboxInterface;
    PMHW_SFC_INTERFACE                  sfcInterface;
    MOS_COMMAND_BUFFER                  cmdBuffer;
    bool                                requestFrameTracking;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL(hwInterface);
    CODECHAL_ENCODE_CHK_NULL(osInterface);
    CODECHAL_ENCODE_CHK_NULL(encoder);
    CODECHAL_ENCODE_CHK_NULL(sfcState);
    CODECHAL_ENCODE_CHK_NULL(sfcInterface = hwInterface->GetSfcInterface());
    CODECHAL_ENCODE_CHK_NULL(veboxInterface = hwInterface->GetVeboxInterface());

    // Switch GPU context to VEBOX
    osInterface->pfnSetGpuContext(osInterface, MOS_GPU_CONTEXT_VEBOX);
    // Reset allocation list and house keeping
    osInterface->pfnResetOsStates(osInterface);

    // Send command buffer header at the beginning
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    CODECHAL_ENCODE_CHK_STATUS(osInterface->pfnGetCommandBuffer(osInterface, &cmdBuffer, 0));

    // the first task?
    requestFrameTracking = false;
    CODECHAL_ENCODE_CHK_STATUS(encoder->SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));

    // Setup cmd prameters
    MOS_ZeroMemory(&veboxStateCmdParams, sizeof(veboxStateCmdParams));
    CODECHAL_ENCODE_CHK_STATUS(CodecHalEncodeSfc_SetVeboxStateParams(&veboxStateCmdParams));

    MOS_ZeroMemory(&veboxSurfaceStateCmdParams, sizeof(veboxSurfaceStateCmdParams));
    CODECHAL_ENCODE_CHK_STATUS(CodecHalEncodeSfc_SetVeboxSurfaceStateParams(sfcState, &veboxSurfaceStateCmdParams));

    MOS_ZeroMemory(&veboxDiIecpCmdParams, sizeof(veboxDiIecpCmdParams));
    CODECHAL_ENCODE_CHK_STATUS(CodecHalEncodeSfc_SetVeboxDiIecpParams(osInterface, sfcState, &veboxDiIecpCmdParams));

    // get csc matrix
    MOS_ZeroMemory(&veboxIecpParams, sizeof(veboxIecpParams));
    CODECHAL_ENCODE_CHK_STATUS(CodecHalEncodeSfc_VeboxSetIecpParams(osInterface, sfcState, &veboxIecpParams));

    // send matrix into heap
    CODECHAL_ENCODE_CHK_STATUS(veboxInterface->AddVeboxIecpState(
        &veboxIecpParams));

    // send Vebox and SFC cmds
    CODECHAL_ENCODE_CHK_STATUS(veboxInterface->AddVeboxState(&cmdBuffer, &veboxStateCmdParams, 0));

    CODECHAL_ENCODE_CHK_STATUS(veboxInterface->AddVeboxSurfaces( &cmdBuffer, &veboxSurfaceStateCmdParams));

    CODECHAL_ENCODE_CHK_STATUS(CodecHalEncodeSfc_AddSfcCommands(sfcInterface, sfcState, &cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS(veboxInterface->AddVeboxDiIecp(&cmdBuffer, &veboxDiIecpCmdParams));

    CODECHAL_ENCODE_CHK_STATUS(hwInterface->GetMiInterface()->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS(encoder->GetDebugInterface()->DumpCmdBuffer(
        &cmdBuffer,
        CODECHAL_MEDIA_STATE_CSC_DS_COPY,
        nullptr)));

    osInterface->pfnReturnCommandBuffer(osInterface, &cmdBuffer, 0);
    CODECHAL_ENCODE_CHK_STATUS(osInterface->pfnSubmitCommandBuffer(
        osInterface,
        &cmdBuffer,
        encoder->m_videoContextUsesNullHw));

finish:
    return eStatus;
}
