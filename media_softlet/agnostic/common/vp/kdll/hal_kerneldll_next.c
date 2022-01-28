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
//! \file      hal_kerneldll_next.c
//! \brief         Kernel Dynamic Linking/Loading routines for FC
//!


#include "hal_kerneldll_next.h"
#include "vp_utils.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

//---------------------------------------------------------------------------------------
// KernelDll_StartKernelSearch_Next - Starts kernel search
//
// Parameters:
//    Kdll_State       *pState       - [in]     Dynamic Linking State
//    Kdll_FilterEntry *pFilter      - [in]     Search filter (array of search entries)
//    int               iFilterSize  - [in]     Search filter size
//    Kdll_SearchState *pSearchState - [in/out] Kernel search state
//
// Output: none
//---------------------------------------------------------------------------------------
void KernelDll_StartKernelSearch_Next(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState,
    Kdll_FilterEntry *pFilter,
    int32_t          iFilterSize,
    uint32_t         uiIs64BInstrEnabled)
{
    int32_t nLayer;

    VP_RENDER_FUNCTION_ENTER;

    // Reset all states
    MOS_ZeroMemory(pSearchState, sizeof(Kdll_SearchState));

    // Setup KDLL state
    pSearchState->pKdllState    = pState;     // KDLL state

    // Cleanup kernel table
    pSearchState->KernelCount   = 0;          // # of kernels

    // Cleanup patch data
    memset(pSearchState->Patches ,  0, sizeof(pSearchState->Patches));
    memset(pSearchState->PatchID , -1, sizeof(pSearchState->PatchID));
    pSearchState->PatchCount = 0;

    // Copy original filter; filter will be modified as part of the search
    if (pFilter && iFilterSize > 0)
    {
        MOS_SecureMemcpy(pSearchState->Filter, iFilterSize * sizeof(Kdll_FilterEntry), pFilter, iFilterSize * sizeof(Kdll_FilterEntry));
        pSearchState->pFilter      = pSearchState->Filter;
        pSearchState->iFilterSize  = iFilterSize;

        // Copy the render target format
        pSearchState->target_format = pSearchState->pFilter[iFilterSize - 1].format;

        // Copy the render target tile type
        pSearchState->target_tiletype = pSearchState->pFilter[iFilterSize - 1].tiletype;

        // Indicate whether to use 64B save kernel for render target surface
        if (uiIs64BInstrEnabled                               &&
            ((pSearchState->target_tiletype == MOS_TILE_X)    ||
            (pSearchState->target_tiletype  == MOS_TILE_LINEAR)))
        {
            pSearchState->b64BSaveEnabled = true;
        }
    }
}

void KernelDll_ModifyFunctionPointers_Next(Kdll_State *pState)
{
    pState->pfnStartKernelSearch = KernelDll_StartKernelSearch_Next;
}

bool KernelDll_IsCspace(VPHAL_CSPACE cspace, VPHAL_CSPACE match)
{
    switch (match)
    {
    case CSpace_RGB:
        return (cspace == CSpace_sRGB ||
                cspace == CSpace_stRGB);

    case CSpace_YUV:
        return (cspace == CSpace_BT709 ||
                cspace == CSpace_BT601 ||
                cspace == CSpace_BT601_FullRange ||
                cspace == CSpace_BT709_FullRange ||
                cspace == CSpace_xvYCC709 ||
                cspace == CSpace_xvYCC601);

    case CSpace_Gray:
        return (cspace == CSpace_BT601Gray ||
                cspace == CSpace_BT601Gray_FullRange);

    case CSpace_Any:
        return (cspace != CSpace_None);

    case CSpace_BT2020:
        return (cspace == CSpace_BT2020 ||
                cspace == CSpace_BT2020_FullRange);

    case CSpace_BT2020_RGB:
        return (cspace == CSpace_BT2020_RGB ||
                cspace == CSpace_BT2020_stRGB);

    default:
        return (cspace == match);
    }

    return false;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_GetYuvRangeAndOffset
| Purpose   : Get the YUV offset and excursion for the input color space
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_GetYuvRangeAndOffset(
    Kdll_CSpace cspace,
    float *     pLumaOffset,
    float *     pLumaExcursion,
    float *     pChromaZero,
    float *     pChromaExcursion)
{
    bool res = true;

    switch (cspace)
    {
    case CSpace_BT601_FullRange:
    case CSpace_BT709_FullRange:
    case CSpace_BT601Gray_FullRange:
    case CSpace_BT2020_FullRange:
        *pLumaOffset      = 0.0f;
        *pLumaExcursion   = 255.0f;
        *pChromaZero      = 128.0f;
        *pChromaExcursion = 255.0f;
        break;

    case CSpace_BT601:
    case CSpace_BT709:
    case CSpace_xvYCC601:  // since matrix is the same as 601, use the same range
    case CSpace_xvYCC709:  // since matrix is the same as 709, use the same range
    case CSpace_BT601Gray:
    case CSpace_BT2020:
        *pLumaOffset      = 16.0f;
        *pLumaExcursion   = 219.0f;
        *pChromaZero      = 128.0f;
        *pChromaExcursion = 224.0f;
        break;

    default:
        res = false;
        break;
    }

    return res;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_GetRgbRangeAndOffset
| Purpose   : Get the RGB offset and excursion for the input color space
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_GetRgbRangeAndOffset(
    Kdll_CSpace cspace,
    float *     pRgbOffset,
    float *     pRgbExcursion)
{
    bool res = true;

    switch (cspace)
    {
    case CSpace_sRGB:
    case CSpace_BT2020_RGB:
        *pRgbOffset    = 0.0f;
        *pRgbExcursion = 255.0f;
        break;

    case CSpace_stRGB:
    case CSpace_BT2020_stRGB:
        *pRgbOffset    = 16.0f;
        *pRgbExcursion = 219.0f;
        break;

    default:
        res = false;
        break;
    }

    return res;
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
bool KernelDll_CalcYuvToRgbMatrix(
    Kdll_CSpace src,              // [in] YUV Color space
    Kdll_CSpace dst,              // [in] RGB Color space
    float *     pTransferMatrix,  // [in] Transfer matrix (3x3)
    float *     pOutMatrix)            // [out] Conversion matrix (3x4)
{
    bool  res;
    float Y_o, Y_e, C_z, C_e;
    float R_o, R_e;

    res = true;

    res = KernelDll_GetRgbRangeAndOffset(dst, &R_o, &R_e);
    if (res == false)
    {
        goto finish;
    }

    res = KernelDll_GetYuvRangeAndOffset(src, &Y_o, &Y_e, &C_z, &C_e);
    if (res == false)
    {
        goto finish;
    }

    // after + (3x3)(3x3)
    pOutMatrix[0]  = pTransferMatrix[0] * R_e / Y_e;
    pOutMatrix[4]  = pTransferMatrix[3] * R_e / Y_e;
    pOutMatrix[8]  = pTransferMatrix[6] * R_e / Y_e;
    pOutMatrix[1]  = pTransferMatrix[1] * R_e / C_e;
    pOutMatrix[5]  = pTransferMatrix[4] * R_e / C_e;
    pOutMatrix[9]  = pTransferMatrix[7] * R_e / C_e;
    pOutMatrix[2]  = pTransferMatrix[2] * R_e / C_e;
    pOutMatrix[6]  = pTransferMatrix[5] * R_e / C_e;
    pOutMatrix[10] = pTransferMatrix[8] * R_e / C_e;

    // (3x1) - (3x3)(3x3)(3x1)
    pOutMatrix[3]  = R_o - (pOutMatrix[0] * Y_o + pOutMatrix[1] * C_z + pOutMatrix[2] * C_z);
    pOutMatrix[7]  = R_o - (pOutMatrix[4] * Y_o + pOutMatrix[5] * C_z + pOutMatrix[6] * C_z);
    pOutMatrix[11] = R_o - (pOutMatrix[8] * Y_o + pOutMatrix[9] * C_z + pOutMatrix[10] * C_z);

finish:
    return res;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_CalcRgbToYuvMatrix
| Purpose   : Given the RGB->YUV transfer matrix, get the final matrix after
|             applying offsets and excursions.
|
| [Y' ]     [Y_o - Y_e.R_o/R_e]   [Y_e/R_e    0       0   ]  [   RGB to YUV  ]  [R']
| [Cb']  =  [C_z]               + [   0    C_e/R_e    0   ]. [Transfer matrix]. [G']
| [Cr']     [C_z]                 [   0       0    C_e/R_e]  [   3x3 matrix  ]  [B']
|
| [Y' ]  = [C0  C1   C2] [R']   [C3]      {Out pMatrix}
| [Cb']  = [C4  C5   C6].[G'] + [C7]
| [Cr']  = [C8  C9  C10] [B'] + [C11]
|
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_CalcRgbToYuvMatrix(
    Kdll_CSpace src,              // [in] RGB Color space
    Kdll_CSpace dst,              // [in] YUV Color space
    float *     pTransferMatrix,  // [in] Transfer matrix (3x3)
    float *     pOutMatrix)            // [out] Conversion matrix (3x4)
{
    bool  res;
    float Y_o, Y_e, C_z, C_e;
    float R_o, R_e;

    res = true;

    res = KernelDll_GetRgbRangeAndOffset(src, &R_o, &R_e);
    if (res == false)
    {
        goto finish;
    }

    res = KernelDll_GetYuvRangeAndOffset(dst, &Y_o, &Y_e, &C_z, &C_e);
    if (res == false)
    {
        goto finish;
    }

    // multiplication of + onwards
    pOutMatrix[0]  = pTransferMatrix[0] * Y_e / R_e;
    pOutMatrix[1]  = pTransferMatrix[1] * Y_e / R_e;
    pOutMatrix[2]  = pTransferMatrix[2] * Y_e / R_e;
    pOutMatrix[4]  = pTransferMatrix[3] * C_e / R_e;
    pOutMatrix[5]  = pTransferMatrix[4] * C_e / R_e;
    pOutMatrix[6]  = pTransferMatrix[5] * C_e / R_e;
    pOutMatrix[8]  = pTransferMatrix[6] * C_e / R_e;
    pOutMatrix[9]  = pTransferMatrix[7] * C_e / R_e;
    pOutMatrix[10] = pTransferMatrix[8] * C_e / R_e;

    // before +
    pOutMatrix[3]  = Y_o - Y_e * R_o / R_e;
    pOutMatrix[7]  = C_z;
    pOutMatrix[11] = C_z;

finish:
    return res;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_CalcGrayCoeffs
| Purpose   : Given CSC matrix, calculate the new matrix making Chroma zero.
|             Chroma will be read from the surface, but we need to factor in C_z
|             by adjusting this in the constant.
|
| [R']  = [C0  C1   C2] [Y' ]   [C3]      {Out pMatrix}
| [G']  = [C4  C5   C6].[C_z] + [C7]
| [B']  = [C8  C9  C10] [C_z]   [C11]
|
| New C3 = C1 * C_z + C2 * C_z + C3
|
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_CalcGrayCoeffs(
    Kdll_CSpace src,  // [in] YUV source Color space
    float *     pMatrix)   // [in/out] Conversion matrix (3x4)
{
    float Y_o, Y_e, C_z, C_e;
    bool  res;

    res = true;

    res = KernelDll_GetYuvRangeAndOffset(src, &Y_o, &Y_e, &C_z, &C_e);
    if (res == false)
    {
        goto finish;
    }

    // Calculate the constant offset by factoring in C_z
    pMatrix[3]  = pMatrix[1] * C_z + pMatrix[2] * C_z + pMatrix[3];
    pMatrix[7]  = pMatrix[5] * C_z + pMatrix[6] * C_z + pMatrix[7];
    pMatrix[11] = pMatrix[9] * C_z + pMatrix[10] * C_z + pMatrix[11];

    // Nullify the effect of chroma read
    pMatrix[1] = pMatrix[2] = 0;
    pMatrix[5] = pMatrix[6] = 0;
    pMatrix[9] = pMatrix[10] = 0;

finish:
    return res;
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_3x3MatrixProduct
| Purpose   : Given two [3x4] input matrices, calculate [3x3]x[3x3] ignoring
|             the last column in both inputs
| Return    : none
\---------------------------------------------------------------------------*/
void KernelDll_3x3MatrixProduct(
    float *      dest,
    const float *m1,
    const float *m2)
{
    dest[0] = m1[0] * m2[0] + m1[1] * m2[4] + m1[2] * m2[8];
    dest[1] = m1[0] * m2[1] + m1[1] * m2[5] + m1[2] * m2[9];
    dest[2] = m1[0] * m2[2] + m1[1] * m2[6] + m1[2] * m2[10];

    dest[4] = m1[4] * m2[0] + m1[5] * m2[4] + m1[6] * m2[8];
    dest[5] = m1[4] * m2[1] + m1[5] * m2[5] + m1[6] * m2[9];
    dest[6] = m1[4] * m2[2] + m1[5] * m2[6] + m1[6] * m2[10];

    dest[8]  = m1[8] * m2[0] + m1[9] * m2[4] + m1[10] * m2[8];
    dest[9]  = m1[8] * m2[1] + m1[9] * m2[5] + m1[10] * m2[9];
    dest[10] = m1[8] * m2[2] + m1[9] * m2[6] + m1[10] * m2[10];
}

/*----------------------------------------------------------------------------
| Name      : KernelDll_CalcYuvToYuvMatrix
| Purpose   : Calculate the matrix equation for converting b/w YUV color spaces.
|             1. Get conversion matrix from Source YUV to sRGB
|             2. Get conversion matrix from sRGB to Destination YUV
|             3. Apply the transformation below to get the final matrix
|
| [Y'dst]  = [C0  C1   C2] [C0  C1   C2][Y'src] [C0  C1   C2] [C3]    [C3]
| [U']     = [C4  C5   C6].[C4  C5   C6][C_z] + [C4  C5   C6].[C7]  + [C7]
| [V']     = [C8  C9  C10] [C8  C9  C10][C_z]   [C8  C9  C10] [C11]   [C11]
|             dst matrix    src matrix           dst matrix    src     dst
|
| [Y'dst]  = [C0  C1   C2] [Y'src]   [C3]      {Out pMatrix}
| [U']     = [C4  C5   C6].[C_z] +   [C7]
| [V']     = [C8  C9  C10] [C_z]     [C11]
|
| Return    : true if success else false
\---------------------------------------------------------------------------*/
bool KernelDll_CalcYuvToYuvMatrix(
    Kdll_CSpace src,    // [in] RGB Color space
    Kdll_CSpace dst,    // [in] YUV Color space
    float *     pOutMatrix)  // [out] Conversion matrix (3x4)
{
    float fYuvToRgb[12] = {0};
    float fRgbToYuv[12] = {0};
    bool  res;

    res = true;

    // 1. Get conversion matrix from Source YUV to sRGB
    if (IS_BT601_CSPACE(src))
    {
        res = KernelDll_CalcYuvToRgbMatrix(src, CSpace_sRGB, (float *)g_cCSC_BT601_YUV_RGB, fYuvToRgb);
    }
    else
    {
        res = KernelDll_CalcYuvToRgbMatrix(src, CSpace_sRGB, (float *)g_cCSC_BT709_YUV_RGB, fYuvToRgb);
    }
    if (res == false)
    {
        goto finish;
    }

    // 2. Get conversion matrix from sRGB to Destination YUV
    if (IS_BT601_CSPACE(dst))
    {
        res = KernelDll_CalcRgbToYuvMatrix(CSpace_sRGB, dst, (float *)g_cCSC_BT601_RGB_YUV, fRgbToYuv);
    }
    else
    {
        res = KernelDll_CalcRgbToYuvMatrix(CSpace_sRGB, dst, (float *)g_cCSC_BT709_RGB_YUV, fRgbToYuv);
    }
    if (res == false)
    {
        goto finish;
    }

    // 3. Multiply the 2 matrices above
    KernelDll_3x3MatrixProduct(pOutMatrix, fRgbToYuv, fYuvToRgb);

    // Perform [3x3][3x1] matrix multiply + [3x1] matrix
    pOutMatrix[3] = fRgbToYuv[0] * fYuvToRgb[3] + fRgbToYuv[1] * fYuvToRgb[7] +
                    fRgbToYuv[2] * fYuvToRgb[11] + fRgbToYuv[3];
    pOutMatrix[7] = fRgbToYuv[4] * fYuvToRgb[3] + fRgbToYuv[5] * fYuvToRgb[7] +
                    fRgbToYuv[6] * fYuvToRgb[11] + fRgbToYuv[7];
    pOutMatrix[11] = fRgbToYuv[8] * fYuvToRgb[3] + fRgbToYuv[9] * fYuvToRgb[7] +
                     fRgbToYuv[10] * fYuvToRgb[11] + fRgbToYuv[11];

finish:
    return res;
}


/*----------------------------------------------------------------------------
| Name      : KernelDll_GetCSCMatrix
| Purpose   : Get the required matrix for the given CSC conversion
| Return    :
\---------------------------------------------------------------------------*/
void KernelDll_GetCSCMatrix(
    Kdll_CSpace src,     // [in] Source Color space
    Kdll_CSpace dst,     // [in] Destination Color space
    float *     pCSC_Matrix)  // [out] CSC matrix to use
{
    bool        bMatrix;
    bool        bSrcGray;
    Kdll_CSpace temp;
    int32_t     i;

    bMatrix  = false;
    bSrcGray = KernelDll_IsCspace(src, CSpace_Gray);

    // convert gray color spaces to its equivalent non-gray cpsace
    switch (src)
    {
    case CSpace_BT601Gray:
        temp = CSpace_BT601;
        break;
    case CSpace_BT601Gray_FullRange:
        temp = CSpace_BT601_FullRange;
        break;
    default:
        temp = src;
        break;
    }

    // BT601/709 YUV to sRGB/stRGB conversion
    if (KernelDll_IsCspace(temp, CSpace_YUV) || KernelDll_IsCspace(temp, CSpace_Gray))
    {
        if (KernelDll_IsCspace(dst, CSpace_RGB))
        {
            if (IS_BT601_CSPACE(temp))
            {
                KernelDll_CalcYuvToRgbMatrix(temp, dst, (float *)g_cCSC_BT601_YUV_RGB, pCSC_Matrix);
                bMatrix = true;
            }
            else  // if (IS_BT709_CSPACE(temp))
            {
                KernelDll_CalcYuvToRgbMatrix(temp, dst, (float *)g_cCSC_BT709_YUV_RGB, pCSC_Matrix);
                bMatrix = true;
            }
        }
    }
    // sRGB/stRGB to BT601/709 YUV conversion
    else if (KernelDll_IsCspace(temp, CSpace_RGB))
    {
        if (KernelDll_IsCspace(dst, CSpace_YUV))
        {
            if (IS_BT601_CSPACE(dst))
            {
                KernelDll_CalcRgbToYuvMatrix(temp, dst, (float *)g_cCSC_BT601_RGB_YUV, pCSC_Matrix);
                bMatrix = true;
            }
            else  // if (IS_BT709_CSPACE(temp))
            {
                KernelDll_CalcRgbToYuvMatrix(temp, dst, (float *)g_cCSC_BT709_RGB_YUV, pCSC_Matrix);
                bMatrix = true;
            }
        }
    }
    // BT2020 YUV to RGB conversion
    else if (KernelDll_IsCspace(temp, CSpace_BT2020))
    {
        if (KernelDll_IsCspace(dst, CSpace_BT2020_RGB))
        {
            KernelDll_CalcYuvToRgbMatrix(temp, dst, (float *)g_cCSC_BT2020_YUV_RGB, pCSC_Matrix);
            bMatrix = true;
        }
    }
    // BT2020 RGB to YUV conversion
    else if (KernelDll_IsCspace(temp, CSpace_BT2020_RGB))
    {
        if (KernelDll_IsCspace(dst, CSpace_BT2020))
        {
            KernelDll_CalcRgbToYuvMatrix(temp, dst, (float *)g_cCSC_BT2020_RGB_YUV, pCSC_Matrix);
            bMatrix = true;
        }
    }

    // If matrix has not been derived yet, its one of the below special cases
    if (!bMatrix)
    {
        if (temp == dst)  // Check if its identity matrix
        {
            MOS_SecureMemcpy(pCSC_Matrix, sizeof(g_cCSC_Identity), (void *)g_cCSC_Identity, sizeof(g_cCSC_Identity));
        }
        else if (KernelDll_IsCspace(temp, CSpace_RGB))  // sRGB to stRGB inter-conversions
        {
            if (temp == CSpace_sRGB)
            {
                MOS_SecureMemcpy(pCSC_Matrix, sizeof(g_cCSC_sRGB_stRGB), (void *)g_cCSC_sRGB_stRGB, sizeof(g_cCSC_sRGB_stRGB));
            }
            else  //temp == CSpace_stRGB
            {
                MOS_SecureMemcpy(pCSC_Matrix, sizeof(g_cCSC_stRGB_sRGB), (void *)g_cCSC_stRGB_sRGB, sizeof(g_cCSC_stRGB_sRGB));
            }
        }
        else if (KernelDll_IsCspace(temp, CSpace_YUV))  // 601 to 709 inter-conversions
        {
            KernelDll_CalcYuvToYuvMatrix(temp, dst, pCSC_Matrix);
        }
        else
        {
            VP_RENDER_ASSERTMESSAGE("Not supported color space conversion(from %d to %d)", src, dst);
            MT_ERR2(MT_VP_KERNEL_CSC, MT_VP_KERNEL_CSPACE, src, MT_VP_KERNEL_CSPACE, dst);
        }
    }

    // Calculate the Gray transformation matrix now
    if (bSrcGray)
    {
        KernelDll_CalcGrayCoeffs(src, pCSC_Matrix);
    }

    VP_RENDER_NORMALMESSAGE("");
    for (i = 0; i < 3; i++)
    {
        VP_RENDER_NORMALMESSAGE("%f\t%f\t%f\t%f",
            pCSC_Matrix[4 * i],
            pCSC_Matrix[4 * i + 1],
            pCSC_Matrix[4 * i + 2],
            pCSC_Matrix[4 * i + 3]);
    }
}

#ifdef __cplusplus
}
#endif  // __cplusplus
