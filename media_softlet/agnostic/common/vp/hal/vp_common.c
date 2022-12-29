/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_common.c
//! \brief    Definition common utilities for vphal
//! \details  Definition common utilities for vphal including:
//!           some macro, enum, union, structure, function
//!

#include "vp_common.h"
#include "hal_kerneldll_next.h"
#include "vp_utils.h"

//!
//! \brief    Get CSC matrix in a form usable by Vebox, SFC and IECP kernels
//! \param    [in] SrcCspace
//!           Source Cspace
//! \param    [in] DstCspace
//!           Destination Cspace
//! \param    [out] pfCscCoeff
//!           [3x3] Coefficients matrix
//! \param    [out] pfCscInOffset
//!           [3x1] Input Offset matrix
//! \param    [out] pfCscOutOffset
//!           [3x1] Output Offset matrix
//! \return   void
//!
void VpHal_GetCscMatrix(
    VPHAL_CSPACE SrcCspace,      // [in] Source Cspace
    VPHAL_CSPACE DstCspace,      // [in] Destination Cspace
    float *      pfCscCoeff,     // [out] [3x3] Coefficients matrix
    float *      pfCscInOffset,  // [out] [3x1] Input Offset matrix
    float *      pfCscOutOffset)       // [out] [3x1] Output Offset matrix
{
    float   fCscMatrix[12] = {0};
    int32_t i;

    KernelDll_GetCSCMatrix(
        SrcCspace,
        DstCspace,
        fCscMatrix);

    // Copy [3x3] into Coeff
    for (i = 0; i < 3; i++)
    {
        MOS_SecureMemcpy(
            &pfCscCoeff[i * 3],
            sizeof(float) * 3,
            &fCscMatrix[i * 4],
            sizeof(float) * 3);
    }

    // Get the input offsets
    switch (SrcCspace)
    {
    CASE_YUV_CSPACE_LIMITEDRANGE:
        pfCscInOffset[0] = -16.0F;
        pfCscInOffset[1] = -128.0F;
        pfCscInOffset[2] = -128.0F;
        break;

    CASE_YUV_CSPACE_FULLRANGE:
        pfCscInOffset[0] = 0.0F;
        pfCscInOffset[1] = -128.0F;
        pfCscInOffset[2] = -128.0F;
        break;

    case CSpace_sRGB:
        pfCscInOffset[0] = 0.0F;
        pfCscInOffset[1] = 0.0F;
        pfCscInOffset[2] = 0.0F;
        break;

    case CSpace_stRGB:
        pfCscInOffset[0] = -16.0F;
        pfCscInOffset[1] = -16.0F;
        pfCscInOffset[2] = -16.0F;
        break;

    //BT2020 YUV->RGB
    case CSpace_BT2020:
        pfCscInOffset[0] = -16.0F;
        pfCscInOffset[1] = -128.0F;
        pfCscInOffset[2] = -128.0F;
        break;

    case CSpace_BT2020_FullRange:
        pfCscInOffset[0] = 0.0F;
        pfCscInOffset[1] = -128.0F;
        pfCscInOffset[2] = -128.0F;
        break;

    //BT2020 RGB->YUV
    case CSpace_BT2020_RGB:
        pfCscInOffset[0] = 0.0F;
        pfCscInOffset[1] = 0.0F;
        pfCscInOffset[2] = 0.0F;
        break;

    //BT2020 RGB->YUV
    case CSpace_BT2020_stRGB:
        pfCscInOffset[0] = -16.0F;
        pfCscInOffset[1] = -16.0F;
        pfCscInOffset[2] = -16.0F;
        break;

    default:
        VP_PUBLIC_NORMALMESSAGE("Unsupported Input ColorSpace for Vebox %d.", (uint32_t)SrcCspace);
    }

    // Get the output offsets
    switch (DstCspace)
    {
    CASE_YUV_CSPACE_LIMITEDRANGE:
        pfCscOutOffset[0] = 16.0F;
        pfCscOutOffset[1] = 128.0F;
        pfCscOutOffset[2] = 128.0F;
        break;

    CASE_YUV_CSPACE_FULLRANGE:
        pfCscOutOffset[0] = 0.0F;
        pfCscOutOffset[1] = 128.0F;
        pfCscOutOffset[2] = 128.0F;
        break;

    case CSpace_sRGB:
        pfCscOutOffset[0] = 0.0F;
        pfCscOutOffset[1] = 0.0F;
        pfCscOutOffset[2] = 0.0F;
        break;

    case CSpace_stRGB:
        pfCscOutOffset[0] = 16.0F;
        pfCscOutOffset[1] = 16.0F;
        pfCscOutOffset[2] = 16.0F;
        break;

    //BT2020 RGB->YUV
    case CSpace_BT2020:
        pfCscOutOffset[0] = 16.0F;
        pfCscOutOffset[1] = 128.0F;
        pfCscOutOffset[2] = 128.0F;
        break;

    case CSpace_BT2020_FullRange:
        pfCscOutOffset[0] = 0.0F;
        pfCscOutOffset[1] = 128.0F;
        pfCscOutOffset[2] = 128.0F;
        break;

    case CSpace_BT2020_RGB:
        pfCscOutOffset[0] = 0.0F;
        pfCscOutOffset[1] = 0.0F;
        pfCscOutOffset[2] = 0.0F;
        break;

    case CSpace_BT2020_stRGB:
        pfCscOutOffset[0] = 16.0F;
        pfCscOutOffset[1] = 16.0F;
        pfCscOutOffset[2] = 16.0F;
        break;

    default:
        VP_PUBLIC_NORMALMESSAGE("Unsupported Output ColorSpace for Vebox %d.", (uint32_t)DstCspace);
    }
}

//! \brief    Transfer float type to half precision float type
//! \details  Transfer float type to half precision float (16bit) type
//! \param    [in] fInput
//!           input FP32 number
//! \return   uint16_t
//!           half precision float value in bit
//!
uint16_t VpHal_FloatToHalfFloat(
    float fInput)
{
    bool                       Sign;
    int32_t                    Exp;
    bool                       ExpSign;
    uint32_t                   Mantissa;
    uint32_t                   dwInput;
    VPHAL_HALF_PRECISION_FLOAT outFloat;

    dwInput  = *((uint32_t *)(&fInput));
    Sign     = (dwInput >> 31) & 0x01;
    Exp      = (dwInput >> 23) & 0x0FF;
    Mantissa = dwInput & 0x07FFFFF;

    outFloat.Sign     = Sign;
    outFloat.Mantissa = (Mantissa >> 13) & 0x03ff;  // truncate to zero

    if (Exp == 0)
    {
        outFloat.Exponent = 0;
    }
    else if (Exp == 0xff)
    {
        // There is one accuracy issue in this fuction.
        // If FP32 is 0x7C800001(NaN), FP16 should be 0x7C01(NaN), but this function returns 0x7C00 instead of 0x7C01.
        // VpHal_FloatToHalfFloatA fixes this accuracy issue.
        outFloat.Exponent = 31;
    }
    else
    {
        // Transfer 15-bit exponent to 4-bit exponent
        Exp -= 0x7f;
        Exp += 0xf;

        if (Exp < 1)
        {
            Exp = 1;
        }
        else if (Exp > 30)
        {
            Exp = 30;
        }

        outFloat.Exponent = Exp;
    }

    return outFloat.value;
}
