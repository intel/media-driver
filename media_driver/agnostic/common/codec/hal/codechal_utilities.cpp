/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_utilities.cpp
//! \brief    Implements the common functions for codec.
//! \details  This modules implements utilities which are shared by encoder and decoder 
//!

#include "codechal_hw.h"
#include "codeckrnheader.h"
#include "codechal_utilities.h"

MOS_STATUS CodecHalInitMediaObjectWalkerParams(
    CodechalHwInterface *hwInterface,
    PMHW_WALKER_PARAMS walkerParams,
    PCODECHAL_WALKER_CODEC_PARAMS walkerCodecParams)
{
    CODECHAL_PUBLIC_CHK_NULL_RETURN(hwInterface);
    CODECHAL_PUBLIC_CHK_NULL_RETURN(walkerParams);
    CODECHAL_PUBLIC_CHK_NULL_RETURN(walkerCodecParams);

    MOS_ZeroMemory(walkerParams, sizeof(MHW_WALKER_PARAMS));

    walkerParams->WalkerMode               =
        (MHW_WALKER_MODE)walkerCodecParams->WalkerMode;
    walkerParams->UseScoreboard            = walkerCodecParams->bUseScoreboard;

    walkerParams->BlockResolution.x        = walkerCodecParams->dwResolutionX;
    walkerParams->BlockResolution.y        = walkerCodecParams->dwResolutionY;

    walkerParams->GlobalResolution.x       = walkerCodecParams->dwResolutionX;
    walkerParams->GlobalResolution.y       = walkerCodecParams->dwResolutionY;

    walkerParams->GlobalOutlerLoopStride.x = walkerCodecParams->dwResolutionX;
    walkerParams->GlobalOutlerLoopStride.y = 0;

    walkerParams->GlobalInnerLoopUnit.x    = 0;
    walkerParams->GlobalInnerLoopUnit.y    = walkerCodecParams->dwResolutionY;

    // Gen7.5 & Gen8: 0x3FF, Gen9: 0x7FF
    walkerParams->dwLocalLoopExecCount     = 0xFFFF;  //MAX VALUE
    walkerParams->dwGlobalLoopExecCount    = 0xFFFF;  //MAX VALUE

    if (walkerCodecParams->bMbEncIFrameDistInUse || walkerCodecParams->bNoDependency)
    {
        walkerParams->ScoreboardMask         = 0;
        // Raster scan walking pattern
        walkerParams->LocalOutLoopStride.x   = 0;
        walkerParams->LocalOutLoopStride.y   = 1;
        walkerParams->LocalInnerLoopUnit.x   = 1;
        walkerParams->LocalInnerLoopUnit.y   = 0;
        walkerParams->LocalEnd.x             = walkerCodecParams->dwResolutionX - 1;
        walkerParams->LocalEnd.y             = 0;
    }
    else if (walkerCodecParams->bUseVerticalRasterScan)
    {
        walkerParams->ScoreboardMask         = 0x1;
        walkerParams->LocalOutLoopStride.x   = 1;
        walkerParams->LocalOutLoopStride.y   = 0;
        walkerParams->LocalInnerLoopUnit.x   = 0;
        walkerParams->LocalInnerLoopUnit.y   = 1;
        walkerParams->LocalEnd.x             = 0;
        walkerParams->LocalEnd.y             = walkerCodecParams->dwResolutionY - 1;
    }
    else
    {
        walkerParams->LocalEnd.x             = 0;
        walkerParams->LocalEnd.y             = 0;

        if (walkerCodecParams->WalkerDegree == CODECHAL_46_DEGREE)   // Gen6 only VP8 HybridPak2Pattern
        {
            // 46 degree walking pattern
            walkerParams->ScoreboardMask         = walkerCodecParams->ScoreboardMask;
            walkerParams->LocalOutLoopStride.x   = 1;
            walkerParams->LocalOutLoopStride.y   = 0;
            walkerParams->LocalInnerLoopUnit.x   = 0x3FF; // -1
            walkerParams->LocalInnerLoopUnit.y   = 1;
        }
        else if (walkerCodecParams->WalkerDegree == CODECHAL_45Z_DEGREE)
        {
            // 45z degree pattern
            walkerParams->ScoreboardMask = 0x0F;

            walkerParams->dwGlobalLoopExecCount = 0x3FF;
            walkerParams->dwLocalLoopExecCount = 0x3FF;

            walkerParams->GlobalResolution.x = uint32_t(walkerCodecParams->dwResolutionX / 2.f) + 1;
            walkerParams->GlobalResolution.y = 2 * walkerCodecParams->dwResolutionY;

            walkerParams->GlobalStart.x = 0;
            walkerParams->GlobalStart.y = 0;

            walkerParams->GlobalOutlerLoopStride.x = walkerParams->GlobalResolution.x;
            walkerParams->GlobalOutlerLoopStride.y = 0;

            walkerParams->GlobalInnerLoopUnit.x = 0;
            walkerParams->GlobalInnerLoopUnit.y = walkerParams->GlobalResolution.y;

            walkerParams->BlockResolution.x = walkerParams->GlobalResolution.x;
            walkerParams->BlockResolution.y = walkerParams->GlobalResolution.y;

            walkerParams->LocalStart.x = 0;
            walkerParams->LocalStart.y = 0;

            walkerParams->LocalOutLoopStride.x = 1;
            walkerParams->LocalOutLoopStride.y = 0;

            walkerParams->LocalInnerLoopUnit.x = MOS_BITFIELD_VALUE((uint32_t)-1, 16);
            walkerParams->LocalInnerLoopUnit.y = 4;

            walkerParams->MiddleLoopExtraSteps = 3;
            walkerParams->MidLoopUnitX = 0;
            walkerParams->MidLoopUnitY = 1;
        }
        else if ( walkerCodecParams->WalkerDegree == CODECHAL_45_DEGREE ||     // Gen8,9
                 ( ( walkerCodecParams->wPictureCodingType == I_TYPE || (walkerCodecParams->wPictureCodingType == B_TYPE && !walkerCodecParams->bDirectSpatialMVPredFlag) )     // CHECK B_TYPE ALWAYS
                   && walkerCodecParams->WalkerDegree != CODECHAL_26_DEGREE ) )
        {
            // 45 degree walking pattern
            walkerParams->ScoreboardMask         = 0x03;
            walkerParams->LocalOutLoopStride.x   = 1;
            walkerParams->LocalOutLoopStride.y   = 0;
            walkerParams->LocalInnerLoopUnit.x   = MOS_BITFIELD_VALUE((uint32_t)-1, 16);     // Gen9: 0xFFF Gen6,8: 0x3FF
            walkerParams->LocalInnerLoopUnit.y   = 1;
        }
        else if ( walkerCodecParams->WalkerDegree == CODECHAL_26Z_DEGREE )
        {
            // 26z degree walking pattern used for HEVC
            walkerParams->ScoreboardMask         = 0x7f;

            // z-order in the local loop
            walkerParams->LocalOutLoopStride.x   = 0;
            walkerParams->LocalOutLoopStride.y   = 1;
            walkerParams->LocalInnerLoopUnit.x   = 1;
            walkerParams->LocalInnerLoopUnit.y   = 0;

            // dispatch 4 threads together in one LCU
            walkerParams->BlockResolution.x        = 2;
            walkerParams->BlockResolution.y        = 2;

            // 26 degree in the global loop
            walkerParams->GlobalOutlerLoopStride.x = 2;
            walkerParams->GlobalOutlerLoopStride.y = 0;

            walkerParams->GlobalInnerLoopUnit.x    = 0xFFF -4 + 1; // -4 in 2's compliment format
            walkerParams->GlobalInnerLoopUnit.y    = 2;
        }
        else
        {
            // 26 degree walking pattern
            walkerParams->ScoreboardMask         = 0x0F;
            walkerParams->LocalOutLoopStride.x   = 1;
            walkerParams->LocalOutLoopStride.y   = 0;
            walkerParams->LocalInnerLoopUnit.x   = MOS_BITFIELD_VALUE((uint32_t)-2, 16);     // Gen9: 0xFFE Gen6,8: 0x3FE
            walkerParams->LocalInnerLoopUnit.y   = 1;
        }
    }

    if (walkerCodecParams->bMbaff)
    {
        walkerParams->ScoreboardMask              = 0xFF;
        walkerParams->MiddleLoopExtraSteps        = 1;
        walkerParams->MidLoopUnitY                = 1;
        walkerParams->LocalInnerLoopUnit.y        = 2;
    }

    // In case of multiple Slice scenarios, every slice can be processed parallelly
    // to enhance the performance. This is accomplished by  launching the slices concurrently,
    // providing the X and Y position of the thread along with Color bit. The AVC MBEnc kernel
    // uses the color bit sent in the header to identify the Slice and calculates the MBY index
    // accordingly.The color bit literally conveys the slice number of the MB.
    if (walkerCodecParams->bColorbitSupported && walkerCodecParams->dwNumSlices <= CODECHAL_MEDIA_WALKER_MAX_COLORS)
    {
        walkerParams->ColorCountMinusOne       = walkerCodecParams->dwNumSlices - 1;
        walkerParams->BlockResolution.y        = walkerCodecParams->usSliceHeight;
        walkerParams->GlobalResolution.y       = walkerCodecParams->usSliceHeight;
        walkerParams->GlobalInnerLoopUnit.y    = walkerCodecParams->usSliceHeight;
    }

    if(walkerCodecParams->bGroupIdSelectSupported)
    {
        walkerParams->GroupIdLoopSelect    = walkerCodecParams->ucGroupId;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalGetKernelBinaryAndSize(
    uint8_t*   kernelBase,
    uint32_t   kernelUID,
    uint8_t**  kernelBinary,
    uint32_t*  size)
{
#ifdef ENABLE_KERNELS

    CODECHAL_PUBLIC_CHK_NULL_RETURN(kernelBase);
    CODECHAL_PUBLIC_CHK_NULL_RETURN(kernelBinary);
    CODECHAL_PUBLIC_CHK_NULL_RETURN(size);

    if (kernelUID >= IDR_CODEC_TOTAL_NUM_KERNELS)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    auto kernelOffsetTable = (uint32_t*)kernelBase;
    auto binaryBase = (uint8_t*)(kernelOffsetTable + IDR_CODEC_TOTAL_NUM_KERNELS + 1);

    *size = kernelOffsetTable[kernelUID + 1] - kernelOffsetTable[kernelUID];
    *kernelBinary = (*size) > 0 ? binaryBase + kernelOffsetTable[kernelUID] : nullptr;
#else
    *size = 0;
    *kernelBinary = nullptr;
#endif

    return MOS_STATUS_SUCCESS;
}

void CodecHal_GetSurfaceWidthInBytes(
    PMOS_SURFACE  surface,
    uint32_t     *widthInBytes)
{
    uint32_t        width;

    switch (surface->Format)
    {
        case Format_IMC1:
        case Format_IMC3:
        case Format_IMC2:
        case Format_IMC4:
        case Format_NV12:
        case Format_YV12:
        case Format_I420:
        case Format_IYUV:
        case Format_400P:
        case Format_411P:
        case Format_422H:
        case Format_422V:
        case Format_444P:
        case Format_RGBP:
        case Format_BGRP:
            width = surface->dwWidth;
            break;
        case Format_YUY2:
        case Format_YUYV:
        case Format_YVYU:
        case Format_UYVY:
        case Format_VYUY:
        case Format_P010:
            width = surface->dwWidth << 1;
            break;
        case Format_Y210:
        case Format_Y216:
        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
        case Format_A8B8G8R8:
        case Format_R32U:
        case Format_R32F:
            width = surface->dwWidth << 2;
            break;
        default:
            width = surface->dwWidth;
            break;
    }

    *widthInBytes = width;
}

MOS_STATUS CodecHalSetRcsSurfaceState(
    CodechalHwInterface             *hwInterface,
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PCODECHAL_SURFACE_CODEC_PARAMS  surfaceCodecParams,
    PMHW_KERNEL_STATE               kernelState)
{
    CODECHAL_PUBLIC_CHK_NULL_RETURN(hwInterface);
    CODECHAL_PUBLIC_CHK_NULL_RETURN(surfaceCodecParams);
    CODECHAL_PUBLIC_CHK_NULL_RETURN(hwInterface->GetRenderInterface());
    CODECHAL_PUBLIC_CHK_NULL_RETURN(hwInterface->GetRenderInterface()->m_stateHeapInterface);
    CODECHAL_PUBLIC_CHK_NULL_RETURN(hwInterface->GetOsInterface());

    PMHW_STATE_HEAP_INTERFACE stateHeapInterface = hwInterface->GetRenderInterface()->m_stateHeapInterface;
    PMOS_INTERFACE osInterface = hwInterface->GetOsInterface();

    MHW_RCS_SURFACE_PARAMS surfaceRcsParams;
    MOS_ZeroMemory(&surfaceRcsParams, sizeof(MHW_RCS_SURFACE_PARAMS));

    // default initial values
    surfaceRcsParams.dwNumPlanes                      = 1;    // MHW_GENERIC_PLANE = MHW_Y_PLANE
    surfaceRcsParams.dwCacheabilityControl            = surfaceCodecParams->dwCacheabilityControl;
    surfaceRcsParams.bRenderTarget                    = surfaceCodecParams->bRenderTarget;
    surfaceRcsParams.bIsWritable                      = surfaceCodecParams->bIsWritable;
    surfaceRcsParams.dwBaseAddrOffset[MHW_Y_PLANE]    = surfaceCodecParams->dwOffset;

    uint32_t                        surfaceFormat;
    MOS_SURFACE                     bufferSurface;
    if (surfaceCodecParams->bIs2DSurface)  // 2D
    {
        if (surfaceCodecParams->bUse32UnormSurfaceFormat)
        {
            surfaceFormat = MHW_GFX3DSTATE_SURFACEFORMAT_R32_UNORM;
        }
        else if (surfaceCodecParams->bUse16UnormSurfaceFormat)
        {
            surfaceFormat = MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM;
        }
        else if (surfaceCodecParams->bUseARGB8Format)
        {
            // Ds+Copy kernel requires input surface set to ARGB8
            surfaceFormat = MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM;
        }
        else if (surfaceCodecParams->bUse32UINTSurfaceFormat)
        {
            surfaceFormat = MHW_GFX3DSTATE_SURFACEFORMAT_R32_UINT;
        }
        else if (surfaceCodecParams->psSurface->Format == Format_YUY2)
        {
            surfaceFormat = MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_NORMAL;
        }
        else if (surfaceCodecParams->psSurface->Format == Format_UYVY)
        {
            surfaceFormat = MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPY;
        }
        else if (surfaceCodecParams->psSurface->Format == Format_P010)
        {
            surfaceFormat = MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM;
        }
        else //NV12
        {
            surfaceFormat = MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM;
        }

        surfaceRcsParams.ForceSurfaceFormat[MHW_Y_PLANE]      = surfaceFormat;
        surfaceRcsParams.dwSurfaceType                        = GFX3DSTATE_SURFACETYPE_2D;
        // MMC info passed with psSurface->CompressionMode
        surfaceRcsParams.psSurface                            = surfaceCodecParams->psSurface;

        uint32_t widthInBytes;
        if( surfaceCodecParams->bMediaBlockRW )
        {
            CodecHal_GetSurfaceWidthInBytes(surfaceCodecParams->psSurface, &widthInBytes);
            surfaceRcsParams.dwWidthToUse[MHW_Y_PLANE]        = WIDTH_IN_DW(widthInBytes);
        }
        else
        {
            surfaceRcsParams.dwWidthToUse[MHW_Y_PLANE]        = surfaceCodecParams->psSurface->dwWidth;
        }

        surfaceRcsParams.dwHeightToUse[MHW_Y_PLANE]           = (surfaceCodecParams->dwHeightInUse == 0) ?
            ((surfaceCodecParams->bUseHalfHeight) ? (surfaceCodecParams->psSurface->dwHeight / 2) : surfaceCodecParams->psSurface->dwHeight)
            : surfaceCodecParams->dwHeightInUse;
        surfaceRcsParams.dwBindingTableOffset[MHW_Y_PLANE]    = surfaceCodecParams->dwBindingTableOffset;
        surfaceRcsParams.dwYOffset[MHW_Y_PLANE]               = surfaceCodecParams->psSurface->YPlaneOffset.iYOffset;
        surfaceRcsParams.bVertLineStride                      = surfaceCodecParams->dwVerticalLineStride;
        surfaceRcsParams.bVertLineStrideOffs                  = surfaceCodecParams->dwVerticalLineStrideOffset;
        surfaceRcsParams.psSurface->dwDepth                   = 1;  // depth needs to be 0 for codec 2D surface

        if (surfaceCodecParams->bUseUVPlane)  // UV
        {
            surfaceRcsParams.dwNumPlanes                          = 2;    // Y, UV
            if (surfaceCodecParams->bForceChromaFormat)
            {
                surfaceRcsParams.ForceSurfaceFormat[MHW_U_PLANE]  = surfaceCodecParams->ChromaType;
            }
            else
            {
                MOS_MEMCOMP_STATE mmcstate;

                CODECHAL_PUBLIC_CHK_STATUS_RETURN(osInterface->pfnGetMemoryCompressionMode(
                    osInterface, &surfaceRcsParams.psSurface->OsResource, &mmcstate));

                // MMC HW requires GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPUVY format for P010 UV planes.
                surfaceRcsParams.ForceSurfaceFormat[MHW_U_PLANE] = (surfaceCodecParams->psSurface->Format == Format_P010) ?
                    ((mmcstate != MOS_MEMCOMP_DISABLED) ? MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPUVY : MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_UNORM) :
                    MHW_GFX3DSTATE_SURFACEFORMAT_R16_UINT;
            }
            surfaceRcsParams.dwBindingTableOffset[MHW_U_PLANE]    = surfaceCodecParams->dwUVBindingTableOffset;

            widthInBytes = surfaceCodecParams->psSurface->dwWidth;
            if (surfaceCodecParams->psSurface->Format == Format_P010)
            {
                widthInBytes *= 2;
            }
            surfaceRcsParams.dwWidthToUse[MHW_U_PLANE]            = (surfaceCodecParams->bMediaBlockRW) ?
                WIDTH_IN_DW(widthInBytes) : (surfaceCodecParams->psSurface->dwWidth / 2);
            surfaceRcsParams.dwHeightToUse[MHW_U_PLANE]           = (surfaceCodecParams->bUseHalfHeight) ?
                (surfaceCodecParams->psSurface->dwHeight / 4) : (surfaceCodecParams->psSurface->dwHeight / 2);

            if (surfaceCodecParams->psSurface->Format == Format_YUY2V || surfaceCodecParams->psSurface->Format == Format_Y216V || surfaceCodecParams->psSurface->Format == Format_P208)
            {
                surfaceRcsParams.dwHeightToUse[MHW_U_PLANE] = surfaceRcsParams.dwHeightToUse[MHW_U_PLANE] * 2;
            }

            if (IS_Y_MAJOR_TILE_FORMAT(surfaceRcsParams.psSurface->TileType))
            {
                uint32_t tileHeightAlignment =
                    (MOS_TILE_YS == surfaceRcsParams.psSurface->TileType) ? MOS_YSTILE_H_ALIGNMENT : MOS_YTILE_H_ALIGNMENT;

                surfaceRcsParams.dwBaseAddrOffset[MHW_U_PLANE] =
                    surfaceCodecParams->psSurface->dwPitch *
                    MOS_ALIGN_FLOOR(surfaceCodecParams->psSurface->UPlaneOffset.iYOffset, tileHeightAlignment);
                surfaceRcsParams.dwYOffset[MHW_U_PLANE] =
                    (surfaceCodecParams->psSurface->UPlaneOffset.iYOffset % tileHeightAlignment);
            }
            else if( MOS_TILE_LINEAR == surfaceRcsParams.psSurface->TileType )
            {
                surfaceRcsParams.dwBaseAddrOffset[MHW_U_PLANE] =
                    surfaceCodecParams->psSurface->dwPitch * surfaceCodecParams->psSurface->UPlaneOffset.iYOffset;
                surfaceRcsParams.dwYOffset[MHW_U_PLANE] = 0;
            }
            else if( MOS_TILE_X == surfaceRcsParams.psSurface->TileType )
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("X_TILE surface not supported yet!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            else
            {
                CODECHAL_PUBLIC_ASSERTMESSAGE("Invalid surface TileType");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        if (surfaceCodecParams->bUseHalfHeight)
        {
            surfaceRcsParams.MediaBoundaryPixelMode       = GFX3DSTATE_BOUNDARY_INTERLACED_FRAME;
        }
    }
    else if (surfaceCodecParams->bUseAdvState)  // AdvState
    {
        surfaceRcsParams.bUseAdvState                       = surfaceCodecParams->bUseAdvState;
        // MMC info passed with psSurface->CompressionMode
        surfaceRcsParams.psSurface                          = surfaceCodecParams->psSurface;
        surfaceRcsParams.dwWidthToUse[MHW_Y_PLANE]            = surfaceCodecParams->dwWidthInUse;
        surfaceRcsParams.dwWidthToUse[MHW_U_PLANE]            = surfaceRcsParams.dwWidthToUse[MHW_V_PLANE] = surfaceCodecParams->dwWidthInUse / 2;
        surfaceRcsParams.dwHeightToUse[MHW_Y_PLANE]            = surfaceCodecParams->dwHeightInUse;
        surfaceRcsParams.dwHeightToUse[MHW_U_PLANE]         = surfaceRcsParams.dwHeightToUse[MHW_V_PLANE] =
            (surfaceCodecParams->psSurface->Format == Format_YUY2V || surfaceCodecParams->psSurface->Format == Format_Y216V) ?
            surfaceCodecParams->dwHeightInUse :
            surfaceCodecParams->dwHeightInUse / 2;

        surfaceRcsParams.ForceSurfaceFormat[MHW_Y_PLANE]    = MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_420_8;
        surfaceRcsParams.dwBindingTableOffset[MHW_Y_PLANE]  = surfaceCodecParams->dwBindingTableOffset;
        surfaceRcsParams.bInterleaveChroma                  = true;
        surfaceRcsParams.Direction                          = (MHW_CHROMA_SITING_VDIRECTION) surfaceCodecParams->ucVDirection;
        surfaceRcsParams.dwYOffset[MHW_U_PLANE]             = surfaceCodecParams->psSurface->UPlaneOffset.iYOffset;
    }
    else // 1D Buffer
    {
        MOS_ZeroMemory(&bufferSurface, sizeof(MOS_SURFACE));
        MOS_SecureMemcpy(&bufferSurface.OsResource, sizeof(MOS_RESOURCE), surfaceCodecParams->presBuffer, sizeof(MOS_RESOURCE));

        surfaceRcsParams.psSurface = &bufferSurface;

        surfaceRcsParams.ForceSurfaceFormat[MHW_Y_PLANE]      = surfaceCodecParams->bRawSurface ?
            MHW_GFX3DSTATE_SURFACEFORMAT_RAW : MHW_GFX3DSTATE_SURFACEFORMAT_R32_UINT;
        surfaceRcsParams.dwBindingTableOffset[MHW_Y_PLANE]    = surfaceCodecParams->dwBindingTableOffset;

        surfaceRcsParams.psSurface->Type            = MOS_GFXRES_BUFFER;
        surfaceRcsParams.psSurface->Format          = Format_Buffer;
        surfaceRcsParams.psSurface->TileType        = MOS_TILE_LINEAR;
        surfaceRcsParams.psSurface->dwSize          = surfaceCodecParams->dwSize;
        surfaceRcsParams.psSurface->dwWidth         = (surfaceCodecParams->dwSize - 1) & 0x7F;
        surfaceRcsParams.psSurface->dwHeight        = ((surfaceCodecParams->dwSize - 1) & 0x1FFF80) >> 7;
        surfaceRcsParams.psSurface->dwDepth         = ((surfaceCodecParams->dwSize - 1) & 0xFE00000) >> 21;
        // GMM doesn't provide pitch info from surface
        surfaceRcsParams.psSurface->dwPitch         = surfaceCodecParams->bRawSurface ? sizeof(uint8_t) : sizeof(uint32_t);
        surfaceRcsParams.psSurface->bArraySpacing   = true;
    }

    CODECHAL_PUBLIC_CHK_STATUS_RETURN(stateHeapInterface->pfnSetSurfaceState(
        stateHeapInterface,
        kernelState,
        cmdBuffer,
        1,
        &surfaceRcsParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodecHalGetResourceInfo(
    PMOS_INTERFACE osInterface,
    PMOS_SURFACE surface)
{
    CODECHAL_PUBLIC_CHK_NULL_RETURN(surface);

    MOS_SURFACE details;
    MOS_ZeroMemory(&details, sizeof(details));
    details.Format = Format_Invalid;

    CODECHAL_PUBLIC_CHK_STATUS_RETURN(osInterface->pfnGetResourceInfo(osInterface, &surface->OsResource, &details));

    surface->Format        = details.Format;
    surface->dwWidth       = details.dwWidth;
    surface->dwHeight      = details.dwHeight;
    surface->dwPitch       = details.dwPitch;
    surface->dwDepth       = details.dwDepth;
    surface->dwQPitch      = details.dwQPitch;
    surface->bArraySpacing = details.bArraySpacing;
    surface->TileType      = details.TileType;
    surface->TileModeGMM   = details.TileModeGMM;
    surface->bGMMTileEnabled = details.bGMMTileEnabled;
    surface->dwOffset      = details.RenderOffset.YUV.Y.BaseOffset;
    surface->YPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.Y.BaseOffset;
    surface->YPlaneOffset.iXOffset = details.RenderOffset.YUV.Y.XOffset;
    surface->YPlaneOffset.iYOffset =
        (surface->YPlaneOffset.iSurfaceOffset - surface->dwOffset) / surface->dwPitch +
        details.RenderOffset.YUV.Y.YOffset;
    surface->UPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.U.BaseOffset;
    surface->UPlaneOffset.iXOffset       = details.RenderOffset.YUV.U.XOffset;
    surface->UPlaneOffset.iYOffset       =
        (surface->UPlaneOffset.iSurfaceOffset - surface->dwOffset) / surface->dwPitch +
        details.RenderOffset.YUV.U.YOffset;
    surface->UPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.U;
    surface->VPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.V.BaseOffset;
    surface->VPlaneOffset.iXOffset       = details.RenderOffset.YUV.V.XOffset;
    surface->VPlaneOffset.iYOffset       =
        (surface->VPlaneOffset.iSurfaceOffset - surface->dwOffset) / surface->dwPitch +
        details.RenderOffset.YUV.V.YOffset;
    surface->VPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.V;
    surface->bCompressible     = details.bCompressible;
    surface->CompressionMode   = details.CompressionMode;
    surface->bIsCompressed     = details.bIsCompressed;

    return MOS_STATUS_SUCCESS;
}
