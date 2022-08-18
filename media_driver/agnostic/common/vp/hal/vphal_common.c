/*
* Copyright (c) 2009-2022, Intel Corporation
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
//! \file     vphal_common.c
//! \brief    Definition common utilities for vphal
//! \details  Definition common utilities for vphal including:
//!           some macro, enum, union, structure, function
//!
#include "vphal.h"
#include "hal_kerneldll.h"
#include "mos_os.h"

#include <math.h>

#if EMUL || VPHAL_LIB
#include "support.h"
#endif

union FP32
{
    uint32_t u;
    float f;
    struct
    {
        uint32_t Mantissa : 23;
        uint32_t Exponent : 8;
        uint32_t Sign : 1;
    };
};

union FP16
{
    uint16_t u;
    struct
    {
        uint16_t Mantissa : 10;
        uint16_t Exponent : 5;
        uint16_t Sign : 1;
    };
};
//!
//! \brief    Performs Color Space Convert for Sample 8 bit Using Specified Coeff Matrix
//! \details  Performs Color Space Convert from Src Color Spase to Dst Color Spase
//            Using Secified input CSC Coeff Matrix
//! \param    [out] pOutput
//!           Pointer to VPHAL_COLOR_SAMPLE_8
//! \param    [in] pInput
//!           Pointer to VPHAL_COLOR_SAMPLE_8
//! \param    [in] srcCspace
//!           Source Color Space
//! \param    [in] dstCspace
//!           Dest Color Space
//! \param    [in] piCscMatrix
//!           input CSC coeff Matrxi
//! \return   bool
//!           Return true if successful, otherwise false
//!
bool VpHal_CSC(
    VPHAL_COLOR_SAMPLE_8    *pOutput,
    VPHAL_COLOR_SAMPLE_8    *pInput,
    VPHAL_CSPACE            srcCspace,
    VPHAL_CSPACE            dstCspace,
    int32_t*                piCscMatrix)
{
    bool        bResult;
    int32_t     A, R, G, B;
    int32_t     Y1, U1, V1;

    bResult = true;

    Y1 = R = pInput->YY;
    U1 = G = pInput->Cb;
    V1 = B = pInput->Cr;
    A      = pInput->Alpha;

    if (srcCspace == dstCspace)
    {
        // no conversion needed
        if ((dstCspace == CSpace_sRGB) || (dstCspace == CSpace_stRGB) || IS_COLOR_SPACE_BT2020_RGB(dstCspace))
        {
            pOutput->A = (uint8_t)A;
            pOutput->R = (uint8_t)R;
            pOutput->G = (uint8_t)G;
            pOutput->B = (uint8_t)B;
        }
        else
        {
            pOutput->a = (uint8_t)A;
            pOutput->Y = (uint8_t)Y1;
            pOutput->U = (uint8_t)U1;
            pOutput->V = (uint8_t)V1;
        }
    }
    else
    {
        // conversion needed
        R = (Y1 * piCscMatrix[0]  + U1 * piCscMatrix[1]  +
             V1 * piCscMatrix[2]  +      piCscMatrix[3]  + 0x00080000) >> 20;
        G = (Y1 * piCscMatrix[4]  + U1 * piCscMatrix[5]  +
             V1 * piCscMatrix[6]  +      piCscMatrix[7]  + 0x00080000) >> 20;
        B = (Y1 * piCscMatrix[8]  + U1 * piCscMatrix[9]  +
             V1 * piCscMatrix[10] +      piCscMatrix[11] + 0x00080000) >> 20;

        switch (dstCspace)
        {
            case CSpace_sRGB:
                pOutput->A = (uint8_t)A;
                pOutput->R = MOS_MIN(MOS_MAX(0,R),255);
                pOutput->G = MOS_MIN(MOS_MAX(0,G),255);
                pOutput->B = MOS_MIN(MOS_MAX(0,B),255);
                break;

            case CSpace_stRGB:
                pOutput->A = (uint8_t)A;
                pOutput->R = MOS_MIN(MOS_MAX(16,R),235);
                pOutput->G = MOS_MIN(MOS_MAX(16,G),235);
                pOutput->B = MOS_MIN(MOS_MAX(16,B),235);
                break;

            case CSpace_BT601:
            case CSpace_BT709:
                pOutput->a = (uint8_t)A;
                pOutput->Y = MOS_MIN(MOS_MAX(16,R),235);
                pOutput->U = MOS_MIN(MOS_MAX(16,G),240);
                pOutput->V = MOS_MIN(MOS_MAX(16,B),240);
                break;

            case CSpace_xvYCC601:
            case CSpace_xvYCC709:
            case CSpace_BT601_FullRange:
            case CSpace_BT709_FullRange:
                pOutput->a = (uint8_t)A;
                pOutput->Y = MOS_MIN(MOS_MAX(0,R),255);
                pOutput->U = MOS_MIN(MOS_MAX(0,G),255);
                pOutput->V = MOS_MIN(MOS_MAX(0,B),255);
                break;

            default:
                VPHAL_PUBLIC_NORMALMESSAGE("Unsupported Output ColorSpace %d.", (uint32_t)dstCspace);
                bResult = false;
                break;
        }
    }

    return bResult;
}

//!
//! \brief    Performs Color Space Convert for Sample 8 bit
//! \details  Performs Color Space Convert from Src Color Spase to Dst Color Spase
//! \param    [out] pOutput
//!           Pointer to VPHAL_COLOR_SAMPLE_8
//! \param    [in] pInput
//!           Pointer to VPHAL_COLOR_SAMPLE_8
//! \param    [in] srcCspace
//!           Source Color Space
//! \param    [in] dstCspace
//!           Dest Color Space
//! \return   bool
//!           Return true if successful, otherwise false
//!
bool VpHal_CSC_8(
    VPHAL_COLOR_SAMPLE_8    *pOutput,
    VPHAL_COLOR_SAMPLE_8    *pInput,
    VPHAL_CSPACE            srcCspace,
    VPHAL_CSPACE            dstCspace)
{
    float    pfCscMatrix[12] = {0};
    int32_t piCscMatrix[12] = {0};
    bool    bResult;
    int32_t i;

    KernelDll_GetCSCMatrix(srcCspace, dstCspace, pfCscMatrix);

    // convert float to fixed point format for the 3x4 matrix
    for (i = 0; i < 12; i++)
    {
        // multiply by 2^20 and round up
        piCscMatrix[i] = (int32_t)((pfCscMatrix[i] * 1048576.0f) + 0.5f);
    }

    bResult = VpHal_CSC(pOutput, pInput, srcCspace, dstCspace, piCscMatrix);

    return bResult;
}

//!
//! \brief    sinc
//! \details  Calculate sinc(x)
//! \param    [in] x
//!           float
//! \return   float
//!           sinc(x)
//!
float VpHal_Sinc(float x)
{
    return (VPHAL_ABS(x) < 1e-9f) ? 1.0F : (float)(sin(x) / x);
}

//!
//! \brief    Lanczos
//! \details  Calculate lanczos(x)
//!           Basic formula is:  lanczos(x)= VpHal_Sinc(x) * VpHal_Sinc(x / fLanczosT)
//! \param    [in] x
//!           float
//! \param    [in] dwNumEntries
//!           dword
//! \param    [in] fLanczosT
//! 
//! \return   float
//!           lanczos(x)
//!
float VpHal_Lanczos(float x, uint32_t dwNumEntries, float fLanczosT)
{
    uint32_t dwNumHalfEntries;

    dwNumHalfEntries = dwNumEntries >> 1;
    if (fLanczosT < dwNumHalfEntries)
    {
        fLanczosT = (float)dwNumHalfEntries;
    }

    if (VPHAL_ABS(x) >= dwNumHalfEntries)
    {
        return 0.0;
    }

    x *= VPHAL_PI;

    return VpHal_Sinc(x) * VpHal_Sinc(x / fLanczosT);
}

bool IsSyncFreeNeededForMMCSurface(PVPHAL_SURFACE pSurface, PMOS_INTERFACE pOsInterface)
{
    if (nullptr == pSurface || nullptr == pOsInterface)
    {
        return false;
    }

    //Compressed surface aux table update is after resource dealloction, aux table update need wait the WLs complete
    //the sync deallocation flag will make sure deallocation API return after all surface related WL been completed and resource been destroyed by OS
    auto *pSkuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
    if (pSkuTable &&
        MEDIA_IS_SKU(pSkuTable, FtrE2ECompression) &&                                      //Compression enabled platform
        !MEDIA_IS_SKU(pSkuTable, FtrFlatPhysCCS) &&                                        //NOT DGPU compression
        ((pSurface->bCompressible) && (pSurface->CompressionMode != MOS_MMC_DISABLED)))    //Compressed enabled surface
    {
        return true;
    }

    return false;
}

//!
//! \brief    Allocates the Surface
//! \details  Allocates the Surface
//!           - if the surface is not already allocated OR
//!           - resource dimenisions OR format changed
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \param    [in,out] pSurface
//!           Pointer to VPHAL_SURFACE
//! \param    [in] pSurfaceName
//!           Pointer to surface name
//! \param    [in] Format
//!           Expected MOS_FORMAT
//! \param    [in] DefaultResType
//!           Expected Resource Type
//! \param    [in] DefaultTileType
//!           Expected Surface Tile Type
//! \param    [in] dwWidth
//!           Expected Surface Width
//! \param    [in] dwHeight
//!           Expected Surface Height
//! \param    [in] bCompressible
//!           Surface being compressible or not
//! \param    [in] CompressionMode
//!           Compression Mode
//! \param    [out] pbAllocated
//!           true if allocated, false for not
//! \param    [in] resUsageType
//!           resource usage type for caching
//! \param    [in] tileModeByForce
//!           Forced tile mode
//! \param    [in] memType
//!           vidoe memory location
//! \param    [in] isNotLockable
//!           true if surface not lockable
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_ReAllocateSurface(
    PMOS_INTERFACE          pOsInterface,
    PVPHAL_SURFACE          pSurface,
    PCCHAR                  pSurfaceName,
    MOS_FORMAT              Format,
    MOS_GFXRES_TYPE         DefaultResType,
    MOS_TILE_TYPE           DefaultTileType,
    uint32_t                dwWidth,
    uint32_t                dwHeight,
    bool                    bCompressible,
    MOS_RESOURCE_MMC_MODE   CompressionMode,
    bool*                   pbAllocated,
    MOS_HW_RESOURCE_DEF     resUsageType,
    MOS_TILE_MODE_GMM       tileModeByForce,
    Mos_MemPool             memType,
    bool                    isNotLockable)
{
    MOS_STATUS              eStatus;
    VPHAL_GET_SURFACE_INFO  Info;
    MOS_ALLOC_GFXRES_PARAMS AllocParams;
    MOS_GFXRES_FREE_FLAGS   resFreeFlags = {0};

    //---------------------------------
    VPHAL_PUBLIC_ASSERT(pOsInterface);
    VPHAL_PUBLIC_ASSERT(pSurface);
    //---------------------------------

    eStatus      = MOS_STATUS_SUCCESS;
    *pbAllocated = false;

    // bCompressible should be compared with bCompressible since it is inited by bCompressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (!Mos_ResourceIsNull(&pSurface->OsResource)        &&
        (pSurface->dwWidth         == dwWidth)            &&
        (pSurface->dwHeight        == dwHeight)           &&
        (pSurface->Format          == Format)             &&
        (pSurface->bCompressible   == bCompressible)      &&
        (pSurface->CompressionMode == CompressionMode)    &&
        (pSurface->TileType        == DefaultTileType     ||
        MOS_TILE_Y                 == DefaultTileType     &&
        IS_Y_MAJOR_TILE_FORMAT(pSurface->TileType)))
    {
        goto finish;
    }

    if (pOsInterface->bOptimizeCpuTiming                  &&
        (DefaultResType == MOS_GFXRES_BUFFER)             &&
        (pSurface->dwWidth  >= dwWidth))
    {
        goto finish;
    }

    MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    VpHal_AllocParamsInitType(&AllocParams, pSurface, DefaultResType, DefaultTileType);

    AllocParams.dwWidth         = dwWidth;
    AllocParams.dwHeight        = dwHeight;
    AllocParams.Format          = Format;
    AllocParams.bIsCompressible = bCompressible;
    AllocParams.CompressionMode = CompressionMode;
    AllocParams.pBufName        = pSurfaceName;
    AllocParams.dwArraySize     = 1;
    AllocParams.ResUsageType    = resUsageType;
    AllocParams.m_tileModeByForce = tileModeByForce;
    AllocParams.dwMemType       = memType;
    AllocParams.Flags.bNotLockable = isNotLockable;

    // Delete resource if already allocated
    //if free the compressed surface, need set the sync dealloc flag as 1 for sync dealloc for aux table update
    if (IsSyncFreeNeededForMMCSurface(pSurface, pOsInterface))
    {
        resFreeFlags.SynchronousDestroy = 1;
        VPHAL_PUBLIC_NORMALMESSAGE("Set SynchronousDestroy flag for compressed resource %s", pSurfaceName);
    }
    pOsInterface->pfnFreeResourceWithFlag(pOsInterface, &(pSurface->OsResource), resFreeFlags.Value);

    // Allocate surface
    VPHAL_PUBLIC_CHK_STATUS(pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParams,
        &pSurface->OsResource));

    // Get surface information
    MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));

    // Pre-set to get surface info
    pSurface->Format = Format;

    VPHAL_PUBLIC_CHK_STATUS(VpHal_GetSurfaceInfo(pOsInterface, &Info, pSurface));

    *pbAllocated     = true;

    MT_LOG7(MT_VP_HAL_REALLOC_SURF, MT_NORMAL, MT_VP_HAL_INTER_SURF_TYPE, pSurfaceName ? *((int64_t*)pSurfaceName) : 0,
        MT_SURF_WIDTH, dwWidth, MT_SURF_HEIGHT, dwHeight, MT_SURF_MOS_FORMAT, Format, MT_SURF_TILE_MODE, pSurface->TileModeGMM,
        MT_SURF_COMP_ABLE, pSurface->bCompressible, MT_SURF_COMP_MODE, pSurface->CompressionMode);

finish:
    VPHAL_PUBLIC_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    Reads the Surface contents and copy to the Dst Buffer
//! \details  Reads the Surface contents and copy to the Dst Buffer
//!           - 1 lock surface
//!           - 2 copy surface data to pDst
//!           - 3 unlock surface
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \param    [in] pSurface
//!           Pointer to VPHAL_SURFACE
//! \param    [in] uBpp
//!           bit per pixel of surface contents
//! \param    [out] pDst
//!           output buffer to store Surface contents
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_ReadSurface (
    PMOS_INTERFACE      pOsInterface,
    PVPHAL_SURFACE      pSurface,
    uint32_t            uBpp,
    uint8_t*            pDst)
{
    MOS_STATUS      eStatus;
    uint8_t*        pSrc;
    uint8_t*        pTempSrc;
    uint8_t*        pTempDst;
    uint32_t        uSize;
    uint32_t        uWidthInBytes;
    uint32_t        uY;
    uint32_t        uZ;
    MOS_LOCK_PARAMS LockFlags;

    //----------------------------------------------
    VPHAL_PUBLIC_ASSERT(pSurface);
    VPHAL_PUBLIC_ASSERT(pSurface->dwWidth   > 0);
    VPHAL_PUBLIC_ASSERT(pSurface->dwHeight  > 0);
    VPHAL_PUBLIC_ASSERT(pSurface->dwDepth   > 0);
    VPHAL_PUBLIC_ASSERT(pSurface->dwPitch >= pSurface->dwWidth);
    VPHAL_PUBLIC_ASSERT(uBpp > 0);
    VPHAL_PUBLIC_ASSERT(pDst);
    VPHAL_PUBLIC_ASSERT(!Mos_ResourceIsNull(&pSurface->OsResource));
    //----------------------------------------------

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.ReadOnly = 1;

    pSrc = (uint8_t*)pOsInterface->pfnLockResource(
                    pOsInterface,
                    &pSurface->OsResource,
                    &LockFlags);
    VPHAL_PUBLIC_CHK_NULL(pSrc);

    // Calculate Size in Bytes
    uSize = pSurface->dwWidth * pSurface->dwHeight * pSurface->dwDepth * uBpp/8;
    uWidthInBytes = pSurface->dwWidth * uBpp / 8;
    if (pSurface->dwPitch == uWidthInBytes)
    {
        MOS_SecureMemcpy(pDst, uSize, pSrc, uSize);
    }
    else
    {
        pTempSrc    = pSrc;
        pTempDst    = pDst;

        for (uZ = 0; uZ < pSurface->dwDepth; uZ++)
        {
            for (uY = 0; uY < pSurface->dwHeight; uY++)
            {
                MOS_SecureMemcpy(pTempDst, uWidthInBytes, pTempSrc, uWidthInBytes);
                pTempSrc += pSurface->dwPitch;
                pTempDst += uWidthInBytes;
            }
        }
    }

    VPHAL_PUBLIC_CHK_STATUS(pOsInterface->pfnUnlockResource(pOsInterface, &pSurface->OsResource));

finish:
    return eStatus;
}

//!
//! \brief    Copy Data from input Buffer to the Surface contents
//! \details  Copy Data from input Buffer to the Surface contents
//!           - 1 lock surface
//!           - 2 copy data from pSrc to Surface
//!           - 3 unlock surface
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \param    [out] pSurface
//!           Pointer to VPHAL_SURFACE
//! \param    [in] uBpp
//!           bit per pixel of input buffer
//! \param    [in] pSrc
//!           Input buffer to store Surface contents
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS VpHal_WriteSurface (
    PMOS_INTERFACE      pOsInterface,
    PVPHAL_SURFACE      pSurface,
    uint32_t            uBpp,
    const uint8_t*      pSrc)
{
    MOS_STATUS          eStatus;
    uint8_t*            pDst;
    uint8_t*            pTempSrc;
    uint8_t*            pTempDst;
    uint32_t            uWidthInBytes;
    uint32_t            uSize;
    uint32_t            uY;
    uint32_t            uZ;
    MOS_LOCK_PARAMS     LockFlags;

    //----------------------------------------------
    VPHAL_PUBLIC_ASSERT(pSurface);
    VPHAL_PUBLIC_ASSERT(pSurface->dwWidth   > 0);
    VPHAL_PUBLIC_ASSERT(pSurface->dwHeight  > 0);
    VPHAL_PUBLIC_ASSERT(pSurface->dwDepth   > 0);
    VPHAL_PUBLIC_ASSERT(pSurface->dwPitch >= pSurface->dwWidth);
    VPHAL_PUBLIC_ASSERT(uBpp > 0);
    VPHAL_PUBLIC_ASSERT(pSrc);
    VPHAL_PUBLIC_ASSERT(!Mos_ResourceIsNull(&pSurface->OsResource));
    //----------------------------------------------

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly = 1;

    pDst = (uint8_t*)pOsInterface->pfnLockResource(
                pOsInterface,
                &pSurface->OsResource,
                &LockFlags);
    VPHAL_PUBLIC_CHK_NULL(pDst);

    // Calculate Size in Bytes
    uSize = pSurface->dwWidth * pSurface->dwHeight * pSurface->dwDepth * uBpp/8;
    uWidthInBytes = pSurface->dwWidth * uBpp/8;

    if (pSurface->dwPitch == uWidthInBytes)
    {
        MOS_SecureMemcpy(pDst, uSize, pSrc, uSize);
    }
    else
    {
        pTempSrc    = (uint8_t*)pSrc;
        pTempDst    = pDst;

        for (uZ = 0; uZ < pSurface->dwDepth; uZ++)
        {
            for (uY = 0; uY < pSurface->dwHeight; uY++)
            {
                MOS_SecureMemcpy(pTempDst, uWidthInBytes, pTempSrc, uWidthInBytes);
                pTempSrc += uWidthInBytes;
                pTempDst += pSurface->dwPitch;
            }
        }
    }

    VPHAL_PUBLIC_CHK_STATUS(pOsInterface->pfnUnlockResource(pOsInterface, &pSurface->OsResource));

finish:
    return eStatus;
}

//!
//! \brief    Decide whether Chroma up sampling is needed
//! \param    [in] pSource
//!           Pointer to Source Surface
//! \param    [in] pTarget
//!           Pointer to Target Surface
//! \return   bool
//!           Return true if Chroma up sampling is needed, otherwise false
//!
bool VpHal_IsChromaUpSamplingNeeded(
    PVPHAL_SURFACE          pSource,
    PVPHAL_SURFACE          pTarget)
{
    bool                  bChromaUpSampling;
    VPHAL_COLORPACK       srcColorPack, dstColorPack;

    VPHAL_PUBLIC_ASSERT(pSource);
    VPHAL_PUBLIC_ASSERT(pTarget);

    bChromaUpSampling = false;

    srcColorPack = VpHal_GetSurfaceColorPack(pSource->Format);
    dstColorPack = VpHal_GetSurfaceColorPack(pTarget->Format);

    if ((srcColorPack == VPHAL_COLORPACK_420 &&
        (dstColorPack == VPHAL_COLORPACK_422 || dstColorPack == VPHAL_COLORPACK_444)) ||
        (srcColorPack == VPHAL_COLORPACK_422 && dstColorPack == VPHAL_COLORPACK_444))
    {
        bChromaUpSampling = true;
    }

    return bChromaUpSampling;
}

//!
//! \brief    Decide whether Chroma down sampling is needed
//! \param    [in] pSource
//!           Pointer to Source Surface
//! \param    [in] pTarget
//!           Pointer to Target Surface
//! \return   bool
//!           Return true if Chroma down sampling is needed, otherwise false
//!
bool VpHal_IsChromaDownSamplingNeeded(
    PVPHAL_SURFACE          pSource,
    PVPHAL_SURFACE          pTarget)
{
    bool                  bChromaDownSampling;
    VPHAL_COLORPACK       srcColorPack, dstColorPack;

    VPHAL_PUBLIC_ASSERT(pSource);
    VPHAL_PUBLIC_ASSERT(pTarget);

    bChromaDownSampling = false;

    srcColorPack = VpHal_GetSurfaceColorPack(pSource->Format);
    dstColorPack = VpHal_GetSurfaceColorPack(pTarget->Format);

    if ((srcColorPack == VPHAL_COLORPACK_444 &&
        (dstColorPack == VPHAL_COLORPACK_422 || dstColorPack == VPHAL_COLORPACK_420)) ||
        (srcColorPack == VPHAL_COLORPACK_422 && dstColorPack == VPHAL_COLORPACK_420))
    {
        bChromaDownSampling = true;
    }

    return bChromaDownSampling;
}

//! \brief    Get the bit depth of a surface
//! \details  Get bit depth of input mos surface format and return.
//!           For unknown format return 0
//! \param    [in] Format
//!           MOS_FORMAT of a surface
//! \return   uint32_t
//!           Bit depth of the surface
//!
uint32_t VpHal_GetSurfaceBitDepth (
    MOS_FORMAT      Format)
{
    uint32_t uBitDepth;

    uBitDepth = 0;

    switch (Format)
    {
        case Format_IMC1:
        case Format_IMC2:
        case Format_IMC3:
        case Format_IMC4:
        case Format_NV12:
        case Format_NV21:
        case Format_YV12:
        case Format_I420:
        case Format_IYUV:
        case Format_YUY2:
        case Format_YUYV:
        case Format_YVYU:
        case Format_UYVY:
        case Format_VYUY:
        case Format_P208:
        case Format_422H:
        case Format_422V:
        case Format_R5G6B5:
        case Format_R8G8B8:
        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
        case Format_444P:
        case Format_AYUV:
        case Format_AUYV:
        case Format_RGBP:
        case Format_BGRP:
            uBitDepth = 8;
            break;

        case Format_P010:
        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
        case Format_Y210:
        case Format_Y410:
        case Format_P210:
            uBitDepth = 10;
            break;

        case Format_A16B16G16R16:
        case Format_A16R16G16B16:
        case Format_A16B16G16R16F:
        case Format_A16R16G16B16F:
        case Format_P016:
        case Format_Y416:
        case Format_Y216:
        case Format_P216:
            uBitDepth = 16;
            break;

        default:
            VPHAL_PUBLIC_ASSERTMESSAGE("Input format color pack unknown.");
            uBitDepth = 0;
            break;
    }

    return uBitDepth;
}

//!
//! \brief      Get the scale ratio
//! \details    Get the scale ratio from input surface to output surface
//! \param      [in] pSource
//!             Pointer to input Surface
//! \param      [in] pTarget
//!             Pointer to output Surface
//! \param      [out] pfScaleX
//!             Pointer to scaling ratio x
//! \param      [out] pfScaleY
//!             Pointer to scaling ratio y
//! \return     void
//!
void VpHal_GetScalingRatio(
    PVPHAL_SURFACE              pSource,
    PVPHAL_SURFACE              pTarget,
    float*                      pfScaleX,
    float*                      pfScaleY)
{
    MOS_UNUSED(pTarget);

    VPHAL_PUBLIC_ASSERT(pSource);
    VPHAL_PUBLIC_ASSERT(pTarget);
    VPHAL_PUBLIC_ASSERT(pfScaleX);
    VPHAL_PUBLIC_ASSERT(pfScaleY);

    float fScaleX = 0.0;
    float fScaleY = 0.0;

    // Source rectangle is pre-rotated, destination rectangle is post-rotated.
    if (pSource->Rotation == VPHAL_ROTATION_IDENTITY ||
        pSource->Rotation == VPHAL_ROTATION_180 ||
        pSource->Rotation == VPHAL_MIRROR_HORIZONTAL ||
        pSource->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        fScaleX = (float)(pSource->rcDst.right - pSource->rcDst.left) /
                  (float)(pSource->rcSrc.right - pSource->rcSrc.left);
        fScaleY = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                  (float)(pSource->rcSrc.bottom - pSource->rcSrc.top);
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 ||
        // VPHAL_ROTATE_90_MIRROR_HORIZONTAL || VPHAL_ROTATE_90_MIRROR_VERTICAL
        fScaleX = (float)(pSource->rcDst.bottom - pSource->rcDst.top) /
                  (float)(pSource->rcSrc.right - pSource->rcSrc.left);
        fScaleY = (float)(pSource->rcDst.right - pSource->rcDst.left) /
                  (float)(pSource->rcSrc.bottom - pSource->rcSrc.top);
    }

    *pfScaleX = fScaleX;
    *pfScaleY = fScaleY;
}

//! \brief    Transfer float type to half precision float type
//! \details  Transfer float type to half precision float (16bit) type
//! \param    [in] fInput
//!           input FP32 number
//! \return   uint16_t
//!           half precision float value in bit
//!
uint16_t VpHal_FloatToHalfFloat(
    float     fInput)
{
    bool                        Sign;
    int32_t                     Exp;
    bool                        ExpSign;
    uint32_t                    Mantissa;
    uint32_t                    dwInput;
    VPHAL_HALF_PRECISION_FLOAT  outFloat;

    dwInput   = *((uint32_t *) (&fInput));
    Sign      = (dwInput >> 31) &  0x01;
    Exp       = (dwInput >> 23) &  0x0FF;
    Mantissa  = dwInput & 0x07FFFFF;

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

//! \brief    Transfer float type to half precision float type
//! \details  Transfer float type to half precision float (16bit) type
//! \param    [in] fInputA
//!           input FP32 number
//! \return   uint16_t
//!           half precision float value in bit
//!
uint16_t VpHal_FloatToHalfFloatA(float fInputA)
{

    FP32 fInput        = *(FP32*)(&fInputA);
    FP16 fOutput       = { 0 };

    // Based on ISPC reference code (with minor modifications)
    if (fInput.Exponent == 0) // Signed zero/denormal (which will underflow)
    {
        fOutput.Exponent = 0;
    }
    else if (fInput.Exponent == 255) // Inf or NaN (all exponent bits set)
    {
        fOutput.Exponent = 31;
        fOutput.Mantissa = fInput.Mantissa ? 0x200 : 0; // NaN->qNaN and Inf->Inf
    }
    else // Normalized number
    {
        // Exponent unbias the single, then bias the halfp
        int newexp = fInput.Exponent - 127 + 15;
        if (newexp >= 31) // Overflow, return signed infinity
        {
            fOutput.Exponent = 31;
        }
        else if (newexp <= 0) // Underflow
        {
            if ((14 - newexp) <= 24) // Mantissa might be non-zero
            {
                uint32_t mant = fInput.Mantissa | 0x800000; // Hidden 1 bit
                fOutput.Mantissa = mant >> (14 - newexp);
                if ((mant >> (13 - newexp)) & 1) // Check for rounding
                {
                    fOutput.u++; // Round, might overflow into exp bit, but this is OK
                }
            }
        }
        else
        {
            fOutput.Exponent = newexp;
            fOutput.Mantissa = fInput.Mantissa >> 13;
            if (fInput.Mantissa & 0x1000) // Check for rounding
            {
                fOutput.u++; // Round, might overflow to inf, this is OK
            }
        }
    }

    fOutput.Sign = fInput.Sign;
    uint16_t res = *(uint16_t*)(&fOutput);
    return res;
}

MOS_SURFACE VpHal_ConvertVphalSurfaceToMosSurface(PVPHAL_SURFACE pSurface)
{
    VPHAL_PUBLIC_ASSERT(pSurface);

    MOS_SURFACE outSurface;
    MOS_ZeroMemory(&outSurface, sizeof(MOS_SURFACE));
    outSurface.OsResource      = pSurface->OsResource;
    outSurface.Format          = pSurface->Format;
    outSurface.dwWidth         = pSurface->dwWidth;
    outSurface.dwHeight        = pSurface->dwHeight;
    outSurface.TileType        = pSurface->TileType;
    outSurface.TileModeGMM     = pSurface->TileModeGMM;
    outSurface.bGMMTileEnabled = pSurface->bGMMTileEnabled;
    outSurface.dwDepth         = pSurface->dwDepth;
    outSurface.dwPitch         = pSurface->dwPitch;
    outSurface.dwSlicePitch    = pSurface->dwSlicePitch;
    outSurface.dwOffset        = pSurface->dwOffset;
    outSurface.bCompressible   = pSurface->bCompressible;
    outSurface.bIsCompressed   = pSurface->bIsCompressed;
    outSurface.CompressionMode = pSurface->CompressionMode;
    outSurface.CompressionFormat = pSurface->CompressionFormat;

    return outSurface;
}
