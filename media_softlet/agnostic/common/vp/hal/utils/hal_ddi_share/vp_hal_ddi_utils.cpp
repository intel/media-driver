/*
* Copyright (c) 2022-2023, Intel Corporation
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

#include "vp_hal_ddi_utils.h"
#include "vp_utils.h"
#include "hal_kerneldll_next.h"

MOS_SURFACE VpHalDDIUtils::ConvertVphalSurfaceToMosSurface(PVPHAL_SURFACE surface)
{
    MOS_SURFACE outSurface = {};
    MOS_ZeroMemory(&outSurface, sizeof(MOS_SURFACE));

    if (surface != nullptr)
    {
        outSurface.OsResource        = surface->OsResource;
        outSurface.Format            = surface->Format;
        outSurface.dwWidth           = surface->dwWidth;
        outSurface.dwHeight          = surface->dwHeight;
        outSurface.TileType          = surface->TileType;
        outSurface.TileModeGMM       = surface->TileModeGMM;
        outSurface.bGMMTileEnabled   = surface->bGMMTileEnabled;
        outSurface.dwDepth           = surface->dwDepth;
        outSurface.dwPitch           = surface->dwPitch;
        outSurface.dwSlicePitch      = surface->dwSlicePitch;
        outSurface.dwOffset          = surface->dwOffset;
        outSurface.bCompressible     = surface->bCompressible;
        outSurface.bIsCompressed     = surface->bIsCompressed;
        outSurface.CompressionMode   = surface->CompressionMode;
        outSurface.CompressionFormat = surface->CompressionFormat;
    }

    return outSurface;
}

VPHAL_COLORPACK VpHalDDIUtils::GetSurfaceColorPack(
    MOS_FORMAT format)
{
    VP_FUNC_CALL();
    VPHAL_COLORPACK colorPack = VPHAL_COLORPACK_UNKNOWN;

    static const std::map<const VPHAL_COLORPACK, const MosFormatArray> colorPackMap =
    {
        {VPHAL_COLORPACK_400, {Format_Y8,
                               Format_Y16S,
                               Format_Y16U,
                               Format_400P}},
        {VPHAL_COLORPACK_420, {Format_IMC1,
                               Format_IMC2,
                               Format_IMC3,
                               Format_IMC4,
                               Format_NV12,
                               Format_NV21,
                               Format_YV12,
                               Format_I420,
                               Format_IYUV,
                               Format_P010,
                               Format_P016}},
        {VPHAL_COLORPACK_422, {Format_YUY2,
                               Format_YUYV,
                               Format_YVYU,
                               Format_UYVY,
                               Format_VYUY,
                               Format_P208,
                               Format_422H,
                               Format_422V,
                               Format_Y210,
                               Format_Y216}},
        {VPHAL_COLORPACK_444, {Format_A8R8G8B8,
                               Format_X8R8G8B8,
                               Format_A8B8G8R8,
                               Format_X8B8G8R8,
                               Format_A16B16G16R16,
                               Format_A16R16G16B16,
                               Format_R5G6B5,
                               Format_R8G8B8,
                               Format_RGBP,
                               Format_BGRP,
                               Format_Y416,
                               Format_Y410,
                               Format_AYUV,
                               Format_AUYV,
                               Format_444P,
                               Format_R10G10B10A2,
                               Format_B10G10R10A2,
                               Format_A16B16G16R16F,
                               Format_A16R16G16B16F}},
        {VPHAL_COLORPACK_411, {Format_411P}}
    };
    for (auto mapIt = colorPackMap.begin(); mapIt != colorPackMap.end(); mapIt++)
    {
        auto &iFormatArray = mapIt->second;
        auto  iter         = std::find(iFormatArray.begin(), iFormatArray.end(), format);
        if (iter == iFormatArray.end())
        {
            continue;
        }
        return mapIt->first;
    }

    VP_PUBLIC_ASSERTMESSAGE("Input format color pack unknown.");
    return VPHAL_COLORPACK_UNKNOWN;
}

uint32_t VpHalDDIUtils::GetSurfaceBitDepth(
    MOS_FORMAT format)
{
    uint32_t bitDepth = 0;

    VP_FUNC_CALL();

    static const std::map<const uint32_t, const MosFormatArray> bitDepthMap =
    {
        {8, {Format_IMC1,
             Format_IMC2,
             Format_IMC3,
             Format_IMC4,
             Format_NV12,
             Format_NV21,
             Format_YV12,
             Format_I420,
             Format_IYUV,
             Format_YUY2,
             Format_YUYV,
             Format_YVYU,
             Format_UYVY,
             Format_VYUY,
             Format_P208,
             Format_422H,
             Format_422V,
             Format_R5G6B5,
             Format_R8G8B8,
             Format_A8R8G8B8,
             Format_X8R8G8B8,
             Format_A8B8G8R8,
             Format_X8B8G8R8,
             Format_444P,
             Format_AYUV,
             Format_AUYV,
             Format_RGBP,
             Format_BGRP}},
        {10, {Format_P010,
              Format_R10G10B10A2,
              Format_B10G10R10A2,
              Format_Y210,
              Format_Y410,
              Format_P210}},
        {16, {Format_A16B16G16R16,
              Format_A16R16G16B16,
              Format_A16B16G16R16F,
              Format_A16R16G16B16F,
              Format_P016,
              Format_Y416,
              Format_Y216,
              Format_P216}},
    };
    for (auto mapIt = bitDepthMap.begin(); mapIt != bitDepthMap.end(); mapIt++)
    {
        auto &iFormatArray = mapIt->second;
        auto  iter         = std::find(iFormatArray.begin(), iFormatArray.end(), format);
        if (iter == iFormatArray.end())
        {
            continue;
        }
        return mapIt->first;
    }

    VP_PUBLIC_ASSERTMESSAGE("Unknown Input format for bit depth.");
    return 0;
}