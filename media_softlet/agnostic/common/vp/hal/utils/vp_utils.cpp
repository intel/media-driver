/*
* Copyright (c) 2021-2023, Intel Corporation
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

#include <cmath>
#include <array>
#include "vp_utils.h"
#include "vp_common.h"
#include "hal_kerneldll_next.h"
#include "mos_interface.h"

MOS_STATUS VpUtils::ReAllocateSurface(
    PMOS_INTERFACE        osInterface,
    PVPHAL_SURFACE        surface,
    PCCHAR                surfaceName,
    MOS_FORMAT            format,
    MOS_GFXRES_TYPE       defaultResType,
    MOS_TILE_TYPE         defaultTileType,
    uint32_t              dwWidth,
    uint32_t              dwHeight,
    bool                  bCompressible,
    MOS_RESOURCE_MMC_MODE compressionMode,
    bool                  *bAllocated,
    MOS_HW_RESOURCE_DEF   resUsageType,
    MOS_TILE_MODE_GMM     tileModeByForce,
    Mos_MemPool           memType,
    bool                  isNotLockable)
{
    MOS_STATUS              eStatus      = MOS_STATUS_SUCCESS;
    VPHAL_GET_SURFACE_INFO  info         = {};
    MOS_ALLOC_GFXRES_PARAMS allocParams  = {};
    MOS_GFXRES_FREE_FLAGS   resFreeFlags = {0};

    //---------------------------------
    VP_PUBLIC_ASSERT(osInterface);
    VP_PUBLIC_ASSERT(surface);
    //---------------------------------

    *bAllocated = false;

    // bCompressible should be compared with bCompressible since it is inited by bCompressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (!Mos_ResourceIsNull(&surface->OsResource) &&
        (surface->dwWidth == dwWidth) &&
        (surface->dwHeight == dwHeight) &&
        (surface->Format == format) &&
        (surface->bCompressible == bCompressible) &&
        (surface->CompressionMode == compressionMode) &&
        (surface->TileType == defaultTileType))
    {
        goto finish;
    }

    // reuse the allocated buffer if the allocated size was larger than request size when OptimizeCpuTiming is enabled
    if (osInterface->bOptimizeCpuTiming                             &&
        !Mos_ResourceIsNull(&surface->OsResource)                   &&
        (Format_Buffer                        == format)            &&
        (surface->dwWidth * surface->dwHeight >= dwWidth * dwHeight))
    {
        goto finish;
    }

    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    VpHal_AllocParamsInitType(&allocParams, surface, defaultResType, defaultTileType);

    allocParams.dwWidth            = dwWidth;
    allocParams.dwHeight           = dwHeight;
    allocParams.Format             = format;
    allocParams.bIsCompressible    = bCompressible;
    allocParams.CompressionMode    = compressionMode;
    allocParams.pBufName           = surfaceName;
    allocParams.dwArraySize        = 1;
    allocParams.ResUsageType       = resUsageType;
    allocParams.m_tileModeByForce  = tileModeByForce;
    allocParams.dwMemType          = memType;
    allocParams.Flags.bNotLockable = isNotLockable;

    // Delete resource if already allocated
    //if free the compressed surface, need set the sync dealloc flag as 1 for sync dealloc for aux table update
    if (IsSyncFreeNeededForMMCSurface(surface, osInterface))
    {
        resFreeFlags.SynchronousDestroy = 1;
        VP_PUBLIC_NORMALMESSAGE("Set SynchronousDestroy flag for compressed resource %s", surfaceName);
    }
    osInterface->pfnFreeResourceWithFlag(osInterface, &(surface->OsResource), resFreeFlags.Value);

    // Allocate surface
    VP_PUBLIC_CHK_STATUS(osInterface->pfnAllocateResource(
        osInterface,
        &allocParams,
        &surface->OsResource));

    // Get surface information
    MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));

    // Pre-set to get surface info
    surface->Format = format;

    VP_PUBLIC_CHK_STATUS(VpHal_GetSurfaceInfo(osInterface, &info, surface));

    *bAllocated = true;

    MT_LOG7(MT_VP_HAL_REALLOC_SURF, MT_NORMAL, MT_VP_HAL_INTER_SURF_TYPE, surfaceName ? *((int64_t *)surfaceName) : 0, MT_SURF_WIDTH, dwWidth, MT_SURF_HEIGHT, dwHeight, MT_SURF_MOS_FORMAT, format, MT_SURF_TILE_MODE, surface->TileModeGMM, MT_SURF_COMP_ABLE, surface->bCompressible, MT_SURF_COMP_MODE, surface->CompressionMode);

finish:
    VP_PUBLIC_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

bool VpUtils::IsVerticalRotation(VPHAL_ROTATION rotation) {
    return (rotation != VPHAL_ROTATION_IDENTITY &&
            rotation != VPHAL_ROTATION_180 &&
            rotation != VPHAL_MIRROR_VERTICAL &&
            rotation != VPHAL_MIRROR_HORIZONTAL);
}

bool VpUtils::IsSyncFreeNeededForMMCSurface(PVPHAL_SURFACE surface, PMOS_INTERFACE osInterface)
{
    if (nullptr == surface || nullptr == osInterface)
    {
        return false;
    }

    //Compressed surface aux table update is after resource dealloction, aux table update need wait the WLs complete
    //the sync deallocation flag will make sure deallocation API return after all surface related WL been completed and resource been destroyed by OS
    auto *pSkuTable = osInterface->pfnGetSkuTable(osInterface);
    if (pSkuTable &&
        MEDIA_IS_SKU(pSkuTable, FtrE2ECompression) &&                                    //Compression enabled platform
        !MEDIA_IS_SKU(pSkuTable, FtrFlatPhysCCS) &&                                      //NOT DGPU compression
        ((surface->bCompressible) && (surface->CompressionMode != MOS_MMC_DISABLED)))  //Compressed enabled surface
    {
        return true;
    }

    return false;
}

void VpUtils::GetCscMatrixForVeSfc8Bit(
    VPHAL_CSPACE srcCspace,      
    VPHAL_CSPACE dstCspace,      
    float        *fCscCoeff,     
    float        *fCscInOffset,  
    float        *fCscOutOffset)       
{
    float   fCscMatrix[12] = {0};
    int32_t i              =  0;

    KernelDll_GetCSCMatrix(
        srcCspace,
        dstCspace,
        fCscMatrix);

    // Copy [3x3] into Coeff
    for (i = 0; i < 3; i++)
    {
        MOS_SecureMemcpy(
            &fCscCoeff[i * 3],
            sizeof(float) * 3,
            &fCscMatrix[i * 4],
            sizeof(float) * 3);
    }

    // Get the input offsets
    switch (srcCspace)
    {
    CASE_YUV_CSPACE_LIMITEDRANGE:
        fCscInOffset[0] = -16.0F;
        fCscInOffset[1] = -128.0F;
        fCscInOffset[2] = -128.0F;
        break;

    CASE_YUV_CSPACE_FULLRANGE:
        fCscInOffset[0] = 0.0F;
        fCscInOffset[1] = -128.0F;
        fCscInOffset[2] = -128.0F;
        break;

    case CSpace_sRGB:
        fCscInOffset[0] = 0.0F;
        fCscInOffset[1] = 0.0F;
        fCscInOffset[2] = 0.0F;
        break;

    case CSpace_stRGB:
        fCscInOffset[0] = -16.0F;
        fCscInOffset[1] = -16.0F;
        fCscInOffset[2] = -16.0F;
        break;

    //BT2020 YUV->RGB
    case CSpace_BT2020:
        fCscInOffset[0] = -16.0F;
        fCscInOffset[1] = -128.0F;
        fCscInOffset[2] = -128.0F;
        break;

    case CSpace_BT2020_FullRange:
        fCscInOffset[0] = 0.0F;
        fCscInOffset[1] = -128.0F;
        fCscInOffset[2] = -128.0F;
        break;

    //BT2020 RGB->YUV
    case CSpace_BT2020_RGB:
        fCscInOffset[0] = 0.0F;
        fCscInOffset[1] = 0.0F;
        fCscInOffset[2] = 0.0F;
        break;

    //BT2020 RGB->YUV
    case CSpace_BT2020_stRGB:
        fCscInOffset[0] = -16.0F;
        fCscInOffset[1] = -16.0F;
        fCscInOffset[2] = -16.0F;
        break;

    default:
        VP_PUBLIC_NORMALMESSAGE("Unsupported Input ColorSpace for Vebox %d.", (uint32_t)srcCspace);
    }

    // Get the output offsets
    switch (dstCspace)
    {
    CASE_YUV_CSPACE_LIMITEDRANGE:
        fCscOutOffset[0] = 16.0F;
        fCscOutOffset[1] = 128.0F;
        fCscOutOffset[2] = 128.0F;
        break;

    CASE_YUV_CSPACE_FULLRANGE:
        fCscOutOffset[0] = 0.0F;
        fCscOutOffset[1] = 128.0F;
        fCscOutOffset[2] = 128.0F;
        break;

    case CSpace_sRGB:
        fCscOutOffset[0] = 0.0F;
        fCscOutOffset[1] = 0.0F;
        fCscOutOffset[2] = 0.0F;
        break;

    case CSpace_stRGB:
        fCscOutOffset[0] = 16.0F;
        fCscOutOffset[1] = 16.0F;
        fCscOutOffset[2] = 16.0F;
        break;

    //BT2020 RGB->YUV
    case CSpace_BT2020:
        fCscOutOffset[0] = 16.0F;
        fCscOutOffset[1] = 128.0F;
        fCscOutOffset[2] = 128.0F;
        break;

    case CSpace_BT2020_FullRange:
        fCscOutOffset[0] = 0.0F;
        fCscOutOffset[1] = 128.0F;
        fCscOutOffset[2] = 128.0F;
        break;

    case CSpace_BT2020_RGB:
        fCscOutOffset[0] = 0.0F;
        fCscOutOffset[1] = 0.0F;
        fCscOutOffset[2] = 0.0F;
        break;

    case CSpace_BT2020_stRGB:
        fCscOutOffset[0] = 16.0F;
        fCscOutOffset[1] = 16.0F;
        fCscOutOffset[2] = 16.0F;
        break;

    default:
        VP_PUBLIC_NORMALMESSAGE("Unsupported Output ColorSpace for Vebox %d.", (uint32_t)dstCspace);
    }
}

bool VpUtils::GetCscMatrixForRender8Bit(
    VPHAL_COLOR_SAMPLE_8 *output,
    VPHAL_COLOR_SAMPLE_8 *input,
    VPHAL_CSPACE          srcCspace,
    VPHAL_CSPACE          dstCspace)
{
    float   pfCscMatrix[12] = {0};
    int32_t iCscMatrix[12]  = {0};
    bool    bResult         = false;
    int32_t i               = 0;

    KernelDll_GetCSCMatrix(srcCspace, dstCspace, pfCscMatrix);

    // convert float to fixed point format for the 3x4 matrix
    for (i = 0; i < 12; i++)
    {
        // multiply by 2^20 and round up
        iCscMatrix[i] = (int32_t)((pfCscMatrix[i] * 1048576.0f) + 0.5f);
    }

    bResult = GetCscMatrixForRender8BitWithCoeff(output, input, srcCspace, dstCspace, iCscMatrix);

    return bResult;
}

bool VpUtils::GetCscMatrixForRender8BitWithCoeff(
    VPHAL_COLOR_SAMPLE_8 *output,
    VPHAL_COLOR_SAMPLE_8 *input,
    VPHAL_CSPACE          srcCspace,
    VPHAL_CSPACE          dstCspace,
    int32_t              *iCscMatrix)
{
    bool    bResult = true;
    int32_t a = 0, r = 0, g = 0, b = 0;
    int32_t y1 = 0, u1 = 0, v1 = 0;

    y1 = r = input->YY;
    u1 = g = input->Cb;
    v1 = b = input->Cr;
    a      = input->Alpha;

    if (srcCspace == dstCspace)
    {
        // no conversion needed
        if ((dstCspace == CSpace_sRGB) || (dstCspace == CSpace_stRGB) || IS_COLOR_SPACE_BT2020_RGB(dstCspace))
        {
            output->A = (uint8_t)a;
            output->R = (uint8_t)r;
            output->G = (uint8_t)g;
            output->B = (uint8_t)b;
        }
        else
        {
            output->a = (uint8_t)a;
            output->Y = (uint8_t)y1;
            output->U = (uint8_t)u1;
            output->V = (uint8_t)v1;
        }
    }
    else
    {
        // conversion needed
        r = (y1 * iCscMatrix[0] + u1 * iCscMatrix[1] +
                v1 * iCscMatrix[2] + iCscMatrix[3] + 0x00080000) >>
            20;
        g = (y1 * iCscMatrix[4] + u1 * iCscMatrix[5] +
                v1 * iCscMatrix[6] + iCscMatrix[7] + 0x00080000) >>
            20;
        b = (y1 * iCscMatrix[8] + u1 * iCscMatrix[9] +
                v1 * iCscMatrix[10] + iCscMatrix[11] + 0x00080000) >>
            20;

        switch (dstCspace)
        {
        case CSpace_sRGB:
            output->A = (uint8_t)a;
            output->R = MOS_MIN(MOS_MAX(0, r), 255);
            output->G = MOS_MIN(MOS_MAX(0, g), 255);
            output->B = MOS_MIN(MOS_MAX(0, b), 255);
            break;

        case CSpace_stRGB:
            output->A = (uint8_t)a;
            output->R = MOS_MIN(MOS_MAX(16, r), 235);
            output->G = MOS_MIN(MOS_MAX(16, g), 235);
            output->B = MOS_MIN(MOS_MAX(16, b), 235);
            break;

        case CSpace_BT601:
        case CSpace_BT709:
            output->a = (uint8_t)a;
            output->Y = MOS_MIN(MOS_MAX(16, r), 235);
            output->U = MOS_MIN(MOS_MAX(16, g), 240);
            output->V = MOS_MIN(MOS_MAX(16, b), 240);
            break;

        case CSpace_xvYCC601:
        case CSpace_xvYCC709:
        case CSpace_BT601_FullRange:
        case CSpace_BT709_FullRange:
            output->a = (uint8_t)a;
            output->Y = MOS_MIN(MOS_MAX(0, r), 255);
            output->U = MOS_MIN(MOS_MAX(0, g), 255);
            output->V = MOS_MIN(MOS_MAX(0, b), 255);
            break;

        default:
            VP_PUBLIC_NORMALMESSAGE("Unsupported Output ColorSpace %d.", (uint32_t)dstCspace);
            bResult = false;
            break;
        }
    }

    return bResult;
}

MOS_STATUS VpUtils::GetPixelWithCSCForColorFillFloat(
    float                 input[4],
    float                 output[4],
    VPHAL_CSPACE          srcCspace,
    VPHAL_CSPACE          dstCspace)
{
    if (srcCspace == dstCspace)
    {
        VP_PUBLIC_NORMALMESSAGE("Set input color to output color for fp16 usage.");
        // no conversion needed
        output[0] = input[0];
        output[1] = input[1];
        output[2] = input[2];
        output[3] = input[3];
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Not support this colorspace convert for Float Colorfill. Source Color Space: %d, Target Color Space: %d.",
            srcCspace,
            dstCspace);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpUtils::GetPixelWithCSCForColorFill(
    VPHAL_COLOR_SAMPLE_8 &input,
    float                 output[4],
    VPHAL_CSPACE          srcCspace,
    VPHAL_CSPACE          dstCspace)
{
    VPHAL_COLOR_SAMPLE_8 dstColor = {};
    if (IS_COLOR_SPACE_BT2020(srcCspace))
    {
        VP_PUBLIC_ASSERTMESSAGE("Not support color fill input color space BT2020. Because DDI struct only contains 8 bit, which cannot accommodate BT2020");
    }
    else if (IS_COLOR_SPACE_BT2020(dstCspace))
    {
        // Target is BT2020, which is not supported by legacy convert
        VP_PUBLIC_NORMALMESSAGE("Will do special convert to BT2020. Source Cspace %d. Target Cspace %d", srcCspace, dstCspace);
        float pCscMatrix[12]     = {};
        auto  SDRDegamma_sRGB_x1 = [](float c) -> float {
            if (c <= VPHAL_HDR_EOTF_COEFF1_TRADITIONNAL_GAMMA_SRGB)
            {
                return c * VPHAL_HDR_EOTF_COEFF2_TRADITIONNAL_GAMMA_SRGB;
            }
            else
            {
                return pow(VPHAL_HDR_EOTF_COEFF3_TRADITIONNAL_GAMMA_SRGB * c + VPHAL_HDR_EOTF_COEFF4_TRADITIONNAL_GAMMA_SRGB, VPHAL_HDR_EOTF_COEFF5_TRADITIONNAL_GAMMA_SRGB);
            }
        };
        auto HDRGamma_x1 = [](float c) -> float {
            if (c <= 0.0181)
            {
                return c * 4.5f;
            }
            else
            {
                return 1.0993f * pow(c, 0.45f) - 0.0993f;
            }
        };

        //Convert to sRGB
        VPHAL_COLOR_SAMPLE_8 interColorRGB = {};
        if (!GetCscMatrixForRender8Bit(&interColorRGB, &input, srcCspace, CSpace_sRGB))
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
        }
        //Degamma sRGB IEC 61966-2-1:1999
        float R   = SDRDegamma_sRGB_x1((float)interColorRGB.R / 255);
        float G   = SDRDegamma_sRGB_x1((float)interColorRGB.G / 255);
        float B   = SDRDegamma_sRGB_x1((float)interColorRGB.B / 255);
        output[3] = (float)interColorRGB.A / 255;

        //CCM ITU-R BT.2087-0
        float R2020 = 0.329282097415f * G + 0.043313797587f * B + 0.627404078626f * R;
        float G2020 = 0.919541035593f * G + 0.011361189924f * B + 0.069097233123f * R;
        float B2020 = 0.088013255546f * G + 0.895595009604f * B + 0.016391587664f * R;

        //Gamma BT2020 ITU-R BT.2020-2
        R2020 = HDRGamma_x1(MOS_CLAMP_MIN_MAX(R2020, 0.f, 1.f));
        G2020 = HDRGamma_x1(MOS_CLAMP_MIN_MAX(G2020, 0.f, 1.f));
        B2020 = HDRGamma_x1(MOS_CLAMP_MIN_MAX(B2020, 0.f, 1.f));

        KernelDll_GetCSCMatrix(CSpace_BT2020_RGB, dstCspace, pCscMatrix);
        output[0] = pCscMatrix[0] * R2020 + pCscMatrix[1] * G2020 + pCscMatrix[2] * B2020 + pCscMatrix[3] / 1023;
        output[1] = pCscMatrix[4] * R2020 + pCscMatrix[5] * G2020 + pCscMatrix[6] * B2020 + pCscMatrix[7] / 1023;
        output[2] = pCscMatrix[8] * R2020 + pCscMatrix[9] * G2020 + pCscMatrix[10] * B2020 + pCscMatrix[11] / 1023;

        //clamp
        output[0] = MOS_CLAMP_MIN_MAX(output[0], 0.f, 1.f);
        output[1] = MOS_CLAMP_MIN_MAX(output[1], 0.f, 1.f);
        output[2] = MOS_CLAMP_MIN_MAX(output[2], 0.f, 1.f);
    }
    else if (srcCspace == dstCspace)
    {
        // no conversion needed
        output[0] = (float)input.YY / 255;     //R or Y
        output[1] = (float)input.Cb / 255;     //G or U
        output[2] = (float)input.Cr / 255;     //B or V
        output[3] = (float)input.Alpha / 255;  //A       
    }
    else if (dstCspace == CSpace_BT601Gray || dstCspace == CSpace_BT601Gray_FullRange)
    {
        //Target is Gray Color Space, not supported by legacy convert
        VP_PUBLIC_NORMALMESSAGE("Will do special convert to Gray CSpace. Source Cspace %d. Target Cspace %d", srcCspace, dstColor);
        float        pCscMatrix[12] = {};
        int32_t      iCscMatrix[12] = {};
        VPHAL_CSPACE interCSpace    = (dstCspace == CSpace_BT601Gray_FullRange ? CSpace_BT601_FullRange : CSpace_BT601);
        KernelDll_GetCSCMatrix(srcCspace, interCSpace, pCscMatrix);

        // convert float to fixed point format for the 3x4 matrix
        for (int32_t i = 0; i < 12; ++i)
        {
            // multiply by 2^20 and round up
            iCscMatrix[i] = (int32_t)((pCscMatrix[i] * 1048576.0f) + 0.5f);
        }

        int32_t luma = (input.YY * iCscMatrix[0] + input.Cb * iCscMatrix[1] + input.Cr * iCscMatrix[2] + iCscMatrix[3] + 0x00080000) >> 20;
        output[0]    = (float)MOS_CLAMP_MIN_MAX(luma, 0, 255) / 255;
        output[1]    = 0;
        output[2]    = 0;
        output[3]    = (float)input.Alpha / 255;
    }
    else
    {
        //legacy convert
        if (VpUtils::GetCscMatrixForRender8Bit(&dstColor, &input, srcCspace, dstCspace))
        {
            if ((dstCspace == CSpace_sRGB) || (dstCspace == CSpace_stRGB) || IS_COLOR_SPACE_BT2020_RGB(dstCspace))
            {
                output[0] = (float)dstColor.R / 255;
                output[1] = (float)dstColor.G / 255;
                output[2] = (float)dstColor.B / 255;
                output[3] = (float)dstColor.A / 255;
            }
            else
            {
                output[0] = (float)dstColor.Y / 255;
                output[1] = (float)dstColor.U / 255;
                output[2] = (float)dstColor.V / 255;
                output[3] = (float)dstColor.a / 255;
            }
        }
        else
        {
            VP_PUBLIC_ASSERTMESSAGE("Not supported color fill cspace convert. Source Cspace %d. Target Cspace %d", srcCspace, dstColor);
        }
    } 

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpUtils::GetNormalizedCSCMatrix(
    MEDIA_CSPACE src,
    MEDIA_CSPACE dst,
    float        cscMatrix[12])
{
    VP_PUBLIC_CHK_NULL_RETURN(cscMatrix);

    if ((IS_COLOR_SPACE_BT2020(src) && !IS_COLOR_SPACE_BT2020(dst)) ||
        (!IS_COLOR_SPACE_BT2020(src) && IS_COLOR_SPACE_BT2020(dst)))
    {
        VP_PUBLIC_ASSERTMESSAGE("Not support hdr to sdr or sdr to hdr csc convert. Src CSpace %d, Dst CSpace %d", src, dst);
    }

    KernelDll_GetCSCMatrix(src, dst, cscMatrix);

    //for BT2020RGB convert to BT2020RGB, KernelDll_GetCSCMatrix use 1023 as max bias
    //for other cases, such as sRGB/BT709/BT601 and BT2020YUV convert BT2020RGB, KernelDll_GetCSCMatrix use 255 as max bias
    //so need to normalize w/ different value
    if ((src == CSpace_BT2020_stRGB && dst == CSpace_BT2020_RGB) ||
        (src == CSpace_BT2020_RGB && dst == CSpace_BT2020_stRGB))
    {
        cscMatrix[3] /= 1023.f;
        cscMatrix[7] /= 1023.f;
        cscMatrix[11] /= 1023.f;
    }
    else
    {
        cscMatrix[3] /= 255.f;
        cscMatrix[7] /= 255.f;
        cscMatrix[11] /= 255.f;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpUtils::GetRgbRangeAndOffset(
    VPHAL_CSPACE cspace,
    float& rgbOffset,
    float& rgbExcursion)
{
    switch (cspace)
    {
    case CSpace_sRGB:
    case CSpace_BT2020_RGB:
        rgbOffset    = 0.0f;
        rgbExcursion = 255.0f;
        break;

    case CSpace_stRGB:
    case CSpace_BT2020_stRGB:
        rgbOffset    = 16.0f;
        rgbExcursion = 219.0f;
        break;

    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    rgbOffset /= 255.0f;
    rgbExcursion /= 255.0f;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpUtils::GetYuvRangeAndOffset(
    VPHAL_CSPACE cspace,
    float& lumaOffset,
    float& lumaExcursion,
    float& chromaZero,
    float& chromaExcursion)
{
    switch (cspace)
    {
    case CSpace_BT601_FullRange:
    case CSpace_BT709_FullRange:
    case CSpace_BT601Gray_FullRange:
    case CSpace_BT2020_FullRange:
        lumaOffset      = 0.0f;
        lumaExcursion   = 255.0f;
        chromaZero      = 128.0f;
        chromaExcursion = 255.0f;
        break;

    case CSpace_BT601:
    case CSpace_BT709:
    case CSpace_xvYCC601:  // since matrix is the same as 601, use the same range
    case CSpace_xvYCC709:  // since matrix is the same as 709, use the same range
    case CSpace_BT601Gray:
    case CSpace_BT2020:
        lumaOffset      = 16.0f;
        lumaExcursion   = 219.0f;
        chromaZero      = 128.0f;
        chromaExcursion = 224.0f;
        break;

    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    lumaOffset /= 255.0f;
    lumaExcursion /= 255.0f;
    chromaZero /= 255.0f;
    chromaExcursion /= 255.0f;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpUtils::GetCscMatrixForHDR(
    VPHAL_CSPACE src,
    VPHAL_CSPACE dst,
    bool         swapChannel,
    float       *outMatrix)
{
    static auto TransferRgbToYuvMatrix = [](VPHAL_CSPACE src, VPHAL_CSPACE dst, std::array<float, 12> &transferMatrix, float *outMatrix) -> MOS_STATUS {
        float Y_o = 0.0f, Y_e = 0.0f, C_z = 0.0f, C_e = 0.0f;
        float R_o = 0.0f, R_e = 0.0f;
        VP_FUNC_CALL();
        VP_PUBLIC_CHK_NULL_RETURN(outMatrix);

        VP_PUBLIC_CHK_STATUS_RETURN(GetRgbRangeAndOffset(src, R_o, R_e));
        VP_PUBLIC_CHK_STATUS_RETURN(GetYuvRangeAndOffset(dst, Y_o, Y_e, C_z, C_e));

        if (R_e == 0.0f)
        {
            VP_RENDER_ASSERTMESSAGE("R_e = %f  should not be zero.", R_e);
            return MOS_STATUS_INVALID_PARAMETER;
        }
        // multiplication of + onwards
        outMatrix[0]  = transferMatrix[0] * Y_e / R_e;
        outMatrix[1]  = transferMatrix[1] * Y_e / R_e;
        outMatrix[2]  = transferMatrix[2] * Y_e / R_e;
        outMatrix[4]  = transferMatrix[4] * C_e / R_e;
        outMatrix[5]  = transferMatrix[5] * C_e / R_e;
        outMatrix[6]  = transferMatrix[6] * C_e / R_e;
        outMatrix[8]  = transferMatrix[8] * C_e / R_e;
        outMatrix[9]  = transferMatrix[9] * C_e / R_e;
        outMatrix[10] = transferMatrix[10] * C_e / R_e;

        outMatrix[7]  = Y_o - Y_e * R_o / R_e;
        outMatrix[3]  = C_z;
        outMatrix[11] = C_z;

        return MOS_STATUS_SUCCESS;
    };

    static auto TransferYuvToRgbMatrix = [](VPHAL_CSPACE src, VPHAL_CSPACE dst, std::array<float, 12> &transferMatrix, float *outMatrix) -> MOS_STATUS {
        float      Y_o = 0.0f, Y_e = 0.0f, C_z = 0.0f, C_e = 0.0f;
        float      R_o = 0.0f, R_e = 0.0f;

        VP_PUBLIC_CHK_NULL_RETURN(outMatrix);

        VP_PUBLIC_CHK_STATUS_RETURN(GetRgbRangeAndOffset(dst, R_o, R_e));
        VP_PUBLIC_CHK_STATUS_RETURN(GetYuvRangeAndOffset(src, Y_o, Y_e, C_z, C_e));

        // after + (3x3)(3x3)
        if (Y_e == 0.0f || C_e == 0.0f)
        {
            VP_RENDER_ASSERTMESSAGE("Y_e = %f and C_e = %f, should not be zero.", Y_e, C_e);
            return MOS_STATUS_INVALID_PARAMETER;
        }
        outMatrix[0]  = transferMatrix[0] * R_e / Y_e;
        outMatrix[4]  = transferMatrix[4] * R_e / Y_e;
        outMatrix[8]  = transferMatrix[8] * R_e / Y_e;
        outMatrix[1]  = transferMatrix[1] * R_e / C_e;
        outMatrix[5]  = transferMatrix[5] * R_e / C_e;
        outMatrix[9]  = transferMatrix[9] * R_e / C_e;
        outMatrix[2]  = transferMatrix[2] * R_e / C_e;
        outMatrix[6]  = transferMatrix[6] * R_e / C_e;
        outMatrix[10] = transferMatrix[10] * R_e / C_e;

        // (3x1) - (3x3)(3x3)(3x1)
        outMatrix[3]   = R_o - (outMatrix[0] * Y_o + outMatrix[1] * C_z + outMatrix[2] * C_z);
        outMatrix[7]   = R_o - (outMatrix[4] * Y_o + outMatrix[5] * C_z + outMatrix[6] * C_z);
        outMatrix[11]  = R_o - (outMatrix[8] * Y_o + outMatrix[9] * C_z + outMatrix[10] * C_z);

        return MOS_STATUS_SUCCESS;
    };

    std::array<float, 12> tempMatrix;
    if (dst == CSpace_sRGB)
    {
        if (src == CSpace_BT601)
        {
            tempMatrix = {1.000000f, 0.000000f, 1.402000f, 0.000000f, 1.000000f, -0.344136f, -0.714136f, 0.000000f, 1.000000f, 1.772000f, 0.000000f, 0.000000f};
        }
        else if (src == CSpace_BT709)
        {
            tempMatrix = {1.000000f, 0.000000f, 1.574800f, 0.000000f, 1.000000f, -0.187324f, -0.468124f, 0.000000f, 1.000000f, 1.855600f, 0.000000f, 0.000000f};
        }
        else if (src == CSpace_BT2020)
        {
            tempMatrix = {1.000000f, 0.000000f, 1.474600f, 0.000000f, 1.000000f, -0.164550f, -0.571350f, 0.000000f, 1.000000f, 1.881400f, 0.000000f, 0.000000f};
        }
        else
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        VP_PUBLIC_CHK_STATUS_RETURN(TransferYuvToRgbMatrix(src, dst, tempMatrix, outMatrix));
    }
    else if (src == CSpace_sRGB)
    {
        if (dst == CSpace_BT601)
        {
            if (swapChannel)
            {
                tempMatrix = {0.500000f, -0.418688f, -0.081312f, 0.000000f, 0.299000f, 0.587000f, 0.114000f, 0.000000f, -0.168736f, -0.331264f, 0.500000f, 0.000000f};
            }
            else
            {
                tempMatrix = {-0.331264f, -0.168736f, 0.500000f, 0.000000f, 0.587000f, 0.299000f, 0.114000f, 0.000000f, -0.418688f, 0.500000f, -0.081312f, 0.000000f};
            }
        }
        else if (dst == CSpace_BT709 || dst == CSpace_BT709_FullRange)
        {
            if (swapChannel)
            {
                tempMatrix = {0.500000f, -0.454153f, -0.045847f, 0.000000f, 0.212600f, 0.715200f, 0.072200f, 0.000000f, -0.114572f, -0.385428f, 0.500000f, 0.000000f};
            }
            else
            {
                tempMatrix = {-0.385428f, -0.114572f, 0.500000f, 0.000000f, 0.715200f, 0.212600f, 0.072200f, 0.000000f, -0.454153f, 0.500000f, -0.045847f, 0.000000f};
            }
        }
        else if (dst == CSpace_BT2020)
        {
            if (swapChannel)
            {
                tempMatrix = {0.500000f, -0.459786f, -0.040214f, 0.000000f, 0.262700f, 0.678000f, 0.059300f, 0.000000f, -0.139630f, -0.360370f, 0.500000f, 0.000000f};
            }
            else
            {
                tempMatrix = {-0.360370f, -0.139630f, 0.500000f, 0.000000f, 0.678000f, 0.262700f, 0.059300f, 0.000000f, -0.459786f, 0.500000f, -0.040214f, 0.000000f};
            }
        }
        else
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        VP_PUBLIC_CHK_STATUS_RETURN(TransferRgbToYuvMatrix(src, dst, tempMatrix, outMatrix));
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}