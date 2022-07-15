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

//!
//! \brief    Get the color pack type of a surface
//! \details  Map mos surface format to color pack format and return.
//!           For unknown format return VPHAL_COLORPACK_UNKNOWN
//! \param    [in] Format
//!           MOS_FORMAT of a surface
//! \return   VPHAL_COLORPACK
//!           Color pack type of the surface
//!
VPHAL_COLORPACK VpHal_GetSurfaceColorPack(
    MOS_FORMAT Format)
{
    VPHAL_COLORPACK ColorPack;

    ColorPack = VPHAL_COLORPACK_UNKNOWN;

    switch (Format)
    {
    case Format_Y8:
    case Format_Y16S:
    case Format_Y16U:
        ColorPack = VPHAL_COLORPACK_400;
        break;

    case Format_IMC1:
    case Format_IMC2:
    case Format_IMC3:
    case Format_IMC4:
    case Format_NV12:
    case Format_NV21:
    case Format_YV12:
    case Format_I420:
    case Format_IYUV:
    case Format_P010:
    case Format_P016:
        ColorPack = VPHAL_COLORPACK_420;
        break;

    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_P208:
    case Format_422H:
    case Format_422V:
    case Format_Y210:
    case Format_Y216:
        ColorPack = VPHAL_COLORPACK_422;
        break;

    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_A16B16G16R16:
    case Format_A16R16G16B16:
    case Format_R5G6B5:
    case Format_R8G8B8:
    case Format_RGBP:
    case Format_BGRP:
    case Format_Y416:
    case Format_Y410:
    case Format_AYUV:
    case Format_AUYV:
    case Format_444P:
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
    case Format_A16B16G16R16F:
    case Format_A16R16G16B16F:
        ColorPack = VPHAL_COLORPACK_444;
        break;

    case Format_400P:
        ColorPack = VPHAL_COLORPACK_400;
        break;

    case Format_411P:
        ColorPack = VPHAL_COLORPACK_411;
        break;

    default:
        VP_PUBLIC_ASSERTMESSAGE("Input format color pack unknown.");
        ColorPack = VPHAL_COLORPACK_UNKNOWN;
        break;
    }

    return ColorPack;
}