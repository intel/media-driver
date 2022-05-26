/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     vp_dumper.cpp
//! \brief    Implementation of functions for debugging
//! \details  This file contains the Implementation of functions for
//!           surface dumper and parameter dumper
//!

#if (_DEBUG || _RELEASE_INTERNAL)
#include <stdio.h>
#include "vphal.h"
#include "mos_os.h"
#include "vp_dumper.h"
#include "mos_context_next.h"
#include "vp_utils.h"

#define ALLOC_GRANULARITY                           5000000

//==<Dump Surface>==============================================================
#define VPHAL_SURF_DUMP_OUTFILE_KEY_NAME        "outfileLocation"
#define VPHAL_SURF_DUMP_LOCATION_KEY_NAME       "dumpLocations"
#define VPHAL_SURF_DUMP_START_FRAME_KEY_NAME    "startFrame"
#define VPHAL_SURF_DUMP_END_FRAME_KEY_NAME      "endFrame"
#define VPHAL_SURF_DUMP_MAX_DATA_LEN            200
#define VPHAL_SURF_DUMP_TYPE_BACKGROUND         "background"
#define VPHAL_SURF_DUMP_TYPE_PRIMARY            "primary"
#define VPHAL_SURF_DUMP_TYPE_SUBSTREAM          "substream"
#define VPHAL_SURF_DUMP_TYPE_REFERENCE          "reference"
#define VPHAL_SURF_DUMP_TYPE_RENDERTARGET       "rendertarget"
#define VPHAL_SURF_DUMP_LOC_PREALL              "preall"
#define VPHAL_SURF_DUMP_LOC_PREDNDI             "predndi"
#define VPHAL_SURF_DUMP_LOC_POSTDNDI            "postdndi"
#define VPHAL_SURF_DUMP_LOC_PRECOMP             "precomp"
#define VPHAL_SURF_DUMP_LOC_POSTCOMP            "postcomp"
#define VPHAL_SURF_DUMP_LOC_PREMEMDECOMP        "prememdecomp"
#define VPHAL_SURF_DUMP_LOC_POSTMEMDECOMP       "postmemdecomp"
#define VPHAL_SURF_DUMP_LOC_VEBOX_DRIVERHEAP    "veboxdriverheap"
#define VPHAL_SURF_DUMP_LOC_VEBOX_KERNELHEAP    "veboxkernelheap"
#define VPHAL_SURF_DUMP_LOC_POSTALL             "postall"

//==<Dump Parameters>====================================================
#define VPHAL_PARAMS_DUMP_OUTFILE_KEY_NAME      "outxmlLocation"
#define VPHAL_PARAMS_DUMP_START_FRAME_KEY_NAME  "startxmlFrame"
#define VPHAL_PARAMS_DUMP_END_FRAME_KEY_NAME    "endxmlFrame"

void VpDumperTool::GetOsFilePath(
    const char* pcFilePath,
    char*       pOsFilePath)
{
    VP_FUNC_CALL();

    MOS_SecureMemcpy(pOsFilePath, MAX_PATH, (void*)pcFilePath, strlen(pcFilePath));
}

MOS_STATUS VpSurfaceDumper::GetPlaneDefs(
    PVPHAL_SURFACE                    pSurface,
    VPHAL_SURF_DUMP_SURFACE_DEF      *pPlanes,
    uint32_t                         *pdwNumPlanes,
    uint32_t                         *pdwSize,
    bool                              auxEnable,
    bool                              isDeswizzled)
{
    VP_FUNC_CALL();

    MOS_STATUS      eStatus;
    uint32_t        i;
    bool            PaddingEnable = false;

    eStatus = MOS_STATUS_SUCCESS;

    // Caller should supply this much!
    MOS_ZeroMemory(pPlanes, sizeof(VPHAL_SURF_DUMP_SURFACE_DEF) * 3);

    switch (pSurface->Format)
    {
    case Format_AI44:
    case Format_IA44:
    case Format_A4L4:
    case Format_P8:
    case Format_L8:
    case Format_A8:
    case Format_Buffer:
    case Format_STMM:
    case Format_IRW4:
    case Format_IRW5:
    case Format_IRW6:
    case Format_IRW7:
    case Format_RAW:
    case Format_Y8:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;
        break;

    case Format_R5G6B5:
    case Format_A8P8:
    case Format_A8L8:
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_IRW0:
    case Format_IRW1:
    case Format_IRW2:
    case Format_IRW3:
    case Format_V8U8:
    case Format_R16F:
    case Format_Y16S:
    case Format_Y16U:
    case Format_R16UN:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth  = pSurface->dwWidth * 2;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;
        break;

    case Format_R32U:
    case Format_R32F:
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_AYUV:
    case Format_AUYV:
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
    case Format_Y410:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth  = pSurface->dwWidth * 4;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;
        break;
    case Format_R8G8B8:
        *pdwNumPlanes = 1;
        pPlanes[0].dwWidth  = pSurface->dwWidth * 3;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;
        break;

    case Format_Y416:
    case Format_A16B16G16R16:
    case Format_A16R16G16B16:
    case Format_A16B16G16R16F:
    case Format_A16R16G16B16F:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth  = pSurface->dwWidth * 8;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;
        break;

    case Format_NV12:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch  = pSurface->dwPitch;
        break;

    case Format_P010:
    case Format_P016:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth  = pSurface->dwWidth * 2;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch  = pSurface->dwPitch;
        break;

    case Format_IMC2:
    case Format_IMC4:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch  = pPlanes[0].dwPitch;
        break;

    case Format_YVU9:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth / 4;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 4;
        pPlanes[1].dwPitch  = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth  = pPlanes[0].dwWidth / 4;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 4;
        pPlanes[2].dwPitch  = pPlanes[0].dwPitch;
        break;

    case Format_IMC1:
    case Format_IMC3:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth / 2;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch  = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth  = pPlanes[0].dwWidth / 2;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[2].dwPitch  = pPlanes[0].dwPitch;
        break;

    case Format_I420:
    case Format_IYUV:
    case Format_YV12:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth / 2;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch  = pPlanes[0].dwPitch / 2;

        pPlanes[2].dwWidth  = pPlanes[0].dwWidth / 2;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[2].dwPitch  = pPlanes[0].dwPitch / 2;
        break;
    case Format_400P:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;
        break;

    case Format_411P:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth / 4;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight;
        pPlanes[1].dwPitch  = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth  = pPlanes[0].dwWidth / 4;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight;
        pPlanes[2].dwPitch  = pPlanes[0].dwPitch;
        break;

    case Format_411R:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 4;
        pPlanes[1].dwPitch  = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth  = pPlanes[0].dwWidth;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 4;
        pPlanes[2].dwPitch  = pPlanes[0].dwPitch;
        break;

    case Format_422H:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth / 2;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight;
        pPlanes[1].dwPitch  = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth  = pPlanes[0].dwWidth / 2;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight;
        pPlanes[2].dwPitch  = pPlanes[0].dwPitch;
        break;

    case Format_422V:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch  = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth  = pPlanes[0].dwWidth;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[2].dwPitch  = pPlanes[0].dwPitch;
        break;

    case Format_444P:
    case Format_RGBP:
    case Format_BGRP:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth  = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight;
        pPlanes[1].dwPitch  = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth  = pPlanes[0].dwWidth;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight;
        pPlanes[2].dwPitch  = pPlanes[0].dwPitch;
        break;

    case Format_Y210:
    case Format_Y216:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth  = pSurface->dwWidth * 4;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;
        break;

    case Format_P210:
    case Format_P216:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth  = pSurface->dwWidth * 2;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch  = pSurface->dwPitch;

        pPlanes[1].dwWidth  = pSurface->dwWidth * 2;
        pPlanes[1].dwHeight = pSurface->dwHeight;
        pPlanes[1].dwPitch  = pSurface->dwPitch;
        break;

    default:
        VPHAL_DEBUG_NORMALMESSAGE("Format '%d' not supported in current driver, using default 1 plane for dump", pSurface->Format);
        *pdwNumPlanes = 1;
        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;
        break;
    }

    if (MOS_TILE_LINEAR != pSurface->TileType && (Format_RGBP == pSurface->Format || Format_BGRP == pSurface->Format))
    {
        for (i = 0; i < *pdwNumPlanes; i++)
        {
            pPlanes[i].dwHeight = MOS_ALIGN_CEIL(pPlanes[i].dwHeight, 16);
        }
    }
    // For Deswizzled surfaces, Mos_Specific_LockResource() already do the de-padding between Y and UV surf.
    // so, don't use the info of U/V PlaneOffset, as the padding is already removed.
    for (i = 0; i < *pdwNumPlanes; i++)
    {
        switch (i)
        {
        case 0:
            pPlanes[i].dwOffset = pSurface->YPlaneOffset.iSurfaceOffset +
                (pSurface->YPlaneOffset.iYOffset * pSurface->dwPitch) +
                pSurface->YPlaneOffset.iXOffset;
            break;
        case 1:
            if (pSurface->Format == Format_YV12)
            {
                pPlanes[i].dwOffset = (isDeswizzled ? pPlanes[0].dwPitch * pPlanes[0].dwHeight + pPlanes[0].dwOffset : pSurface->VPlaneOffset.iSurfaceOffset) +
                    (pSurface->VPlaneOffset.iYOffset * pSurface->dwPitch) +
                    pSurface->VPlaneOffset.iXOffset;
            }
            else
            {
                pPlanes[i].dwOffset = (isDeswizzled ? pPlanes[0].dwPitch * pPlanes[0].dwHeight + pPlanes[0].dwOffset : pSurface->UPlaneOffset.iSurfaceOffset) +
                    (pSurface->UPlaneOffset.iYOffset * pSurface->dwPitch) +
                    pSurface->UPlaneOffset.iXOffset;
            }
            break;
        case 2:
            if (pSurface->Format == Format_YV12)
            {
                pPlanes[i].dwOffset = (isDeswizzled ? pPlanes[1].dwOffset + pPlanes[1].dwPitch * pPlanes[1].dwHeight : pSurface->UPlaneOffset.iSurfaceOffset) +
                    (pSurface->UPlaneOffset.iYOffset * pSurface->dwPitch) +
                    pSurface->UPlaneOffset.iXOffset;
            }
            else
            {
                pPlanes[i].dwOffset = (isDeswizzled ? pPlanes[1].dwOffset + pPlanes[1].dwPitch * pPlanes[1].dwHeight : pSurface->VPlaneOffset.iSurfaceOffset) +
                    (pSurface->VPlaneOffset.iYOffset * pSurface->dwPitch) +
                    pSurface->VPlaneOffset.iXOffset;
            }
            break;
        default:
            VPHAL_DEBUG_ASSERTMESSAGE("More than 3 planes not supported.");
        }
    }

    //compressed surface need 32 align on height
    if (auxEnable && !isDeswizzled)
    {
        for (i = 0; i < *pdwNumPlanes; i++)
        {
            pPlanes[i].dwHeight = MOS_ALIGN_CEIL(pPlanes[i].dwHeight, 32);
        }
    }

    // For uncompressed surface, padding data is not needed. For compressed surface, padding data is needed for offline check
    if (auxEnable)
    {
        *pdwSize = (pPlanes[0].dwPitch * pPlanes[0].dwHeight) +
                   (pPlanes[1].dwPitch * pPlanes[1].dwHeight) +
                   (pPlanes[2].dwPitch * pPlanes[2].dwHeight);
    }
    else
    {
        *pdwSize = (pPlanes[0].dwWidth * pPlanes[0].dwHeight) +
                   (pPlanes[1].dwWidth * pPlanes[1].dwHeight) +
                   (pPlanes[2].dwWidth * pPlanes[2].dwHeight);
    }

    return eStatus;
}

MOS_STATUS VpSurfaceDumper::GetPlaneDefs(
    PVP_SURFACE                    pSurface,
    VPHAL_SURF_DUMP_SURFACE_DEF* pPlanes,
    uint32_t* pdwNumPlanes,
    uint32_t* pdwSize,
    bool                              auxEnable,
    bool                              isDeswizzled)
{
    VP_FUNC_CALL();

    MOS_STATUS      eStatus;
    uint32_t        i;
    bool            PaddingEnable = false;

    eStatus = MOS_STATUS_SUCCESS;

    // Caller should supply this much!
    MOS_ZeroMemory(pPlanes, sizeof(VPHAL_SURF_DUMP_SURFACE_DEF) * 3);

    switch (pSurface->osSurface->Format)
    {
    case Format_AI44:
    case Format_IA44:
    case Format_A4L4:
    case Format_P8:
    case Format_L8:
    case Format_A8:
    case Format_Buffer:
    case Format_STMM:
    case Format_IRW4:
    case Format_IRW5:
    case Format_IRW6:
    case Format_IRW7:
    case Format_RAW:
    case Format_Y8:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;
        break;

    case Format_R5G6B5:
    case Format_A8P8:
    case Format_A8L8:
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_IRW0:
    case Format_IRW1:
    case Format_IRW2:
    case Format_IRW3:
    case Format_V8U8:
    case Format_R16F:
    case Format_Y16S:
    case Format_Y16U:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth * 2;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;
        break;

    case Format_R32U:
    case Format_R32F:
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_R8G8B8:
    case Format_AYUV:
    case Format_AUYV:
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
    case Format_Y410:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth * 4;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;
        break;

    case Format_Y416:
    case Format_A16B16G16R16:
    case Format_A16R16G16B16:
    case Format_A16B16G16R16F:
    case Format_A16R16G16B16F:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth * 8;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;
        break;

    case Format_NV12:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch = pSurface->osSurface->dwPitch;
        break;

    case Format_P010:
    case Format_P016:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth * 2;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch = pSurface->osSurface->dwPitch;
        break;

    case Format_IMC2:
    case Format_IMC4:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_YVU9:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth / 4;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 4;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth / 4;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 4;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_IMC1:
    case Format_IMC3:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth / 2;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth / 2;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_I420:
    case Format_IYUV:
    case Format_YV12:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth / 2;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch / 2;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth / 2;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch / 2;
        break;
    case Format_400P:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;
        break;

    case Format_411P:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth / 4;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth / 4;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_411R:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 4;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 4;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_422H:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth / 2;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth / 2;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_422V:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_444P:
    case Format_RGBP:
    case Format_BGRP:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_Y210:
    case Format_Y216:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth * 4;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;
        break;

    case Format_P210:
    case Format_P216:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth * 2;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;

        pPlanes[1].dwWidth = pSurface->osSurface->dwWidth * 2;
        pPlanes[1].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[1].dwPitch = pSurface->osSurface->dwPitch;
        break;

    default:
        VPHAL_DEBUG_NORMALMESSAGE("Format '%d' not supported in current driver, using default 1 plane for dump", pSurface->osSurface->Format);
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth = pSurface->osSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->osSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->osSurface->dwPitch;
        break;
    }

    // For Deswizzled surfaces, Mos_Specific_LockResource() already do the de-padding between Y and UV surf.
    // so, don't use the info of U/V PlaneOffset, as the padding is already removed.
    for (i = 0; i < *pdwNumPlanes; i++)
    {
        switch (i)
        {
        case 0:
            pPlanes[i].dwOffset = pSurface->osSurface->YPlaneOffset.iSurfaceOffset +
                (pSurface->osSurface->YPlaneOffset.iYOffset * pSurface->osSurface->dwPitch) +
                pSurface->osSurface->YPlaneOffset.iXOffset;
            break;
        case 1:
            if (pSurface->osSurface->Format == Format_YV12)
            {
                pPlanes[i].dwOffset = (isDeswizzled ? pPlanes[0].dwPitch * pPlanes[0].dwHeight + pPlanes[0].dwOffset : pSurface->osSurface->VPlaneOffset.iSurfaceOffset) +
                    (pSurface->osSurface->VPlaneOffset.iYOffset * pSurface->osSurface->dwPitch) +
                    pSurface->osSurface->VPlaneOffset.iXOffset;
            }
            else
            {
                pPlanes[i].dwOffset = (isDeswizzled ? pPlanes[0].dwPitch * pPlanes[0].dwHeight + pPlanes[0].dwOffset : pSurface->osSurface->UPlaneOffset.iSurfaceOffset) +
                    (pSurface->osSurface->UPlaneOffset.iYOffset * pSurface->osSurface->dwPitch) +
                    pSurface->osSurface->UPlaneOffset.iXOffset;
            }
            break;
        case 2:
            if (pSurface->osSurface->Format == Format_YV12)
            {
                pPlanes[i].dwOffset = (isDeswizzled ? pPlanes[1].dwOffset + pPlanes[1].dwPitch * pPlanes[1].dwHeight : pSurface->osSurface->UPlaneOffset.iSurfaceOffset) +
                    (pSurface->osSurface->UPlaneOffset.iYOffset * pSurface->osSurface->dwPitch) +
                    pSurface->osSurface->UPlaneOffset.iXOffset;
            }
            else
            {
                pPlanes[i].dwOffset = (isDeswizzled ? pPlanes[1].dwOffset + pPlanes[1].dwPitch * pPlanes[1].dwHeight : pSurface->osSurface->VPlaneOffset.iSurfaceOffset) +
                    (pSurface->osSurface->VPlaneOffset.iYOffset * pSurface->osSurface->dwPitch) +
                    pSurface->osSurface->VPlaneOffset.iXOffset;
            }
            break;
        default:
            VPHAL_DEBUG_ASSERTMESSAGE("More than 3 planes not supported.");
        }
    }

    //compressed surface need 32 align on height
    if (auxEnable && !isDeswizzled)
    {
        for (i = 0; i < *pdwNumPlanes; i++)
        {
            pPlanes[i].dwHeight = MOS_ALIGN_CEIL(pPlanes[i].dwHeight, 32);
        }
    }

    // For uncompressed surface, padding data is not needed. For compressed surface, padding data is needed for offline check
    if (auxEnable)
    {
        *pdwSize = (pPlanes[0].dwPitch * pPlanes[0].dwHeight) +
            (pPlanes[1].dwPitch * pPlanes[1].dwHeight) +
            (pPlanes[2].dwPitch * pPlanes[2].dwHeight);
    }
    else
    {
        *pdwSize = (pPlanes[0].dwWidth * pPlanes[0].dwHeight) +
            (pPlanes[1].dwWidth * pPlanes[1].dwHeight) +
            (pPlanes[2].dwWidth * pPlanes[2].dwHeight);
    }

    return eStatus;
}

bool VpSurfaceDumper::HasAuxSurf(
    PMOS_RESOURCE    osResource)
{
    VP_FUNC_CALL();

    bool    hasAuxSurf = false;
#if !EMUL
    GMM_RESOURCE_FLAG                   gmmFlags;
    MOS_ZeroMemory(&gmmFlags, sizeof(gmmFlags));
    gmmFlags    = osResource->pGmmResInfo->GetResFlags();
    hasAuxSurf  = (gmmFlags.Gpu.MMC && gmmFlags.Gpu.UnifiedAuxSurface) ||
                  (gmmFlags.Gpu.CCS && gmmFlags.Gpu.UnifiedAuxSurface && gmmFlags.Info.MediaCompressed);
#endif
    return hasAuxSurf;
}

MOS_STATUS VpSurfaceDumper::DumpSurfaceToFile(
    PMOS_INTERFACE          pOsInterface,
    PVPHAL_SURFACE          pSurface,
    const char              *psPathPrefix,
    uint64_t                iCounter,
    bool                    bLockSurface,
    bool                    bNoDecompWhenLock,
    uint8_t*                pData)
{
    VP_FUNC_CALL();

    MOS_STATUS                          eStatus;
    bool                                isSurfaceLocked;
    char                                sPath[MAX_PATH], sOsPath[MAX_PATH];
    uint8_t                             *pDst, *pTmpSrc, *pTmpDst;
    uint32_t                            dwNumPlanes, dwSize, j, i;
    VPHAL_SURF_DUMP_SURFACE_DEF         planes[3];
    uint32_t                            dstPlaneOffset[3] = {0};
    MOS_LOCK_PARAMS                     LockFlags;
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData;
    bool                                hasAuxSurf;
    bool                                enableAuxDump;
    bool                                enablePlaneDump = false;
    PMOS_RESOURCE                       pLockedResource = nullptr;

    VPHAL_DEBUG_ASSERT(pSurface);
    VPHAL_DEBUG_ASSERT(pOsInterface);
    VPHAL_DEBUG_ASSERT(psPathPrefix);

    eStatus         = MOS_STATUS_SUCCESS;
    isSurfaceLocked = false;
    hasAuxSurf      = false;
    pDst            = nullptr;
    enableAuxDump   = m_dumpSpec.enableAuxDump;
    MOS_ZeroMemory(sPath,   MAX_PATH);
    MOS_ZeroMemory(sOsPath, MAX_PATH);
    dwNumPlanes = 0;
    enablePlaneDump = m_dumpSpec.enablePlaneDump;

    if (pSurface->dwDepth == 0)
    {
        pSurface->dwDepth = 1;
    }

    hasAuxSurf = HasAuxSurf(&pSurface->OsResource);

    // get plane definitions
    VPHAL_DEBUG_CHK_STATUS(GetPlaneDefs(
        pSurface,
        planes,
        &dwNumPlanes,
        &dwSize,
        hasAuxSurf, //(hasAuxSurf && enableAuxDump),
        !enableAuxDump));// !(hasAuxSurf && enableAuxDump)));

    if (bLockSurface)
    {
        // Caller should not give pData when it expect the function to lock surf
        VPHAL_DEBUG_ASSERT(pData = nullptr);

        LockFlags.Value     = 0;
        LockFlags.ReadOnly  = 1;

        // If aux data exist and enable aux dump, no swizzle and no decompress
        if (hasAuxSurf && enableAuxDump)
        {
            LockFlags.TiledAsTiled = 1;
            LockFlags.NoDecompress = 1;
        }

        if (bNoDecompWhenLock)
        {
            LockFlags.NoDecompress = 1;
        }

        bool isPlanar = false;
        isPlanar      = (pSurface->Format == Format_NV12) || (pSurface->Format == Format_P010) || (pSurface->Format == Format_P016);

        VPHAL_DEBUG_CHK_NULL(pOsInterface);
        VPHAL_DEBUG_CHK_NULL(pOsInterface->pfnGetSkuTable);
        auto *skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);

        // RGBP and BGRP support tile output but should not transfer to linear surface due to height 16 align issue.
        if ((skuTable && MEDIA_IS_SKU(skuTable, FtrE2ECompression) || isPlanar) &&
            (pSurface->TileType != MOS_TILE_LINEAR) &&
            !(pSurface->Format == Format_RGBP || pSurface->Format == Format_BGRP))
        {
            bool bAllocated;

            PVPHAL_SURFACE m_temp2DSurfForCopy = (PVPHAL_SURFACE)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
            VPHAL_RENDER_CHK_STATUS(VpUtils::ReAllocateSurface(
                pOsInterface,
                m_temp2DSurfForCopy,
                "Temp2DSurfForSurfDumper",
                pSurface->Format,
                MOS_GFXRES_2D,
                MOS_TILE_LINEAR,
                pSurface->dwWidth,
                pSurface->dwHeight,
                false,
                MOS_MMC_DISABLED,
                &bAllocated,
                MOS_HW_RESOURCE_DEF_MAX,
                MOS_TILE_UNSET_GMM,
                MOS_MEMPOOL_SYSTEMMEMORY));

            m_osInterface->pfnDoubleBufferCopyResource(
                m_osInterface,
                &pSurface->OsResource,
                &m_temp2DSurfForCopy->OsResource,
                false);

            pData = (uint8_t *)pOsInterface->pfnLockResource(
                pOsInterface,
                &m_temp2DSurfForCopy->OsResource,
                &LockFlags);
            pLockedResource = &m_temp2DSurfForCopy->OsResource;

            // get plane definitions
            VPHAL_DEBUG_CHK_STATUS(GetPlaneDefs(
                m_temp2DSurfForCopy,
                planes,
                &dwNumPlanes,
                &dwSize,
                hasAuxSurf,        //(hasAuxSurf && enableAuxDump),
                !enableAuxDump));  // !(hasAuxSurf && enableAuxDump)));
        }
        else
        {
            pData = (uint8_t *)pOsInterface->pfnLockResource(
                pOsInterface,
                &pSurface->OsResource,
                &LockFlags);
            pLockedResource = &pSurface->OsResource;
        }
        VPHAL_DEBUG_CHK_NULL(pData);

        // Write error to user feauture key
        eStatus = ReportUserSettingForDebug(
            m_userSettingPtr,
            __VPHAL_DBG_SURF_DUMPER_RESOURCE_LOCK,
            1,
            MediaUserSetting::Group::Sequence);

        VPHAL_DEBUG_ASSERT(eStatus == MOS_STATUS_SUCCESS);
        isSurfaceLocked = true;
    }

    MOS_SecureStringPrint(
        sPath,
        MAX_PATH,
        sizeof(sPath),
        "%s_f[%04lld]_w[%d]_h[%d]_p[%d].%s",
        psPathPrefix,
        iCounter,
        pSurface->dwWidth,
        planes[0].dwHeight,
        pSurface->dwPitch,
        VpDumperTool::GetFormatStr(pSurface->Format));

    VpDumperTool::GetOsFilePath(sPath, sOsPath);

    pDst = (uint8_t*)MOS_AllocAndZeroMemory(dwSize);
    VPHAL_DEBUG_CHK_NULL(pDst);
    VPHAL_DEBUG_CHK_NULL(pData);
    pTmpSrc = pData;
    pTmpDst = pDst;

    for (j = 0; j < dwNumPlanes; j++)
    {
        pTmpSrc = pData + planes[j].dwOffset;
        for (i = 0; i < planes[j].dwHeight; i++)
        {
            if (hasAuxSurf && enableAuxDump)
            {
                MOS_SecureMemcpy(
                    pTmpDst,
                    planes[j].dwPitch,
                    pTmpSrc,
                    planes[j].dwPitch);

                pTmpSrc += planes[j].dwPitch;
                pTmpDst += planes[j].dwPitch;

                dstPlaneOffset[j+1] += planes[j].dwPitch;
            }
            else
            {
                // For RGB24 format, need to remove one invalid zero every 64 byte
                // which is designed to avoid splitting two cache lines
                if (pSurface->Format == Format_R8G8B8)
                {
                    int dummyBytesPerLine = planes[j].dwWidth / 63;
                    int resPixel          = planes[j].dwWidth - 63 * dummyBytesPerLine;
                    for (int p = 0; p < dummyBytesPerLine; p++)
                    {
                        MOS_SecureMemcpy(
                            &pTmpDst[p * 63],
                            63,
                            &pTmpSrc[p * 64],
                            63);
                    }
                    if (resPixel > 0)
                    {
                        MOS_SecureMemcpy(
                            &pTmpDst[dummyBytesPerLine * 63],
                            resPixel,
                            &pTmpSrc[dummyBytesPerLine * 64],
                            resPixel);
                    }
                }
                else
                {
                    MOS_SecureMemcpy(
                        pTmpDst,
                        planes[j].dwWidth,
                        pTmpSrc,
                        planes[j].dwWidth);

                }
                pTmpSrc += planes[j].dwPitch;
                pTmpDst += planes[j].dwWidth;

                dstPlaneOffset[j + 1] += planes[j].dwWidth;
            }
        }

        //if more than 1 plane, dump each plane's surface for offline analysis
        if (enablePlaneDump)
        {
            if ((dwNumPlanes > 1) && (dwNumPlanes <= 3))
            {
                char sPlanePath[MAX_PATH], sPlaneOsPath[MAX_PATH];
                MOS_ZeroMemory(sPlanePath, MAX_PATH);
                MOS_ZeroMemory(sPlaneOsPath, MAX_PATH);

                MOS_SecureStringPrint(
                    sPlanePath,
                    MAX_PATH,
                    sizeof(sPlanePath),
                    "%s_f[%04lld]_w[%d]_h[%d]_p[%d].%s",
                    psPathPrefix,
                    iCounter,
                    planes[j].dwWidth,
                    planes[j].dwHeight,
                    planes[j].dwPitch,
                    (j == 0 ? "Y" : ((dwNumPlanes == 2) ? "UV" : ((j == 1) ? "U" : "V"))));

                VpDumperTool::GetOsFilePath(sPlanePath, sPlaneOsPath);

                VPHAL_DEBUG_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(sPlaneOsPath, pDst + dstPlaneOffset[j], dstPlaneOffset[j + 1]));
            }
            else
            {
                VPHAL_DEBUG_ASSERTMESSAGE("More than 3 planes not supported during plane dump.");
            }
        }
    }

    VPHAL_DEBUG_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(sOsPath, pDst, dwSize));

#if !EMUL
    // Dump Aux surface data
    if (hasAuxSurf && enableAuxDump)
    {
        uint32_t    resourceIndex;
        bool        isPlanar;
        uint8_t    *auxDataY;
        uint8_t    *auxDataUV;
        uint32_t    auxSizeY;
        uint32_t    auxSizeUV;
        uint8_t    *pSurfaceBase;

        resourceIndex = 0;
        pSurfaceBase  = pData;
        isPlanar      = (pOsInterface->pfnGetGmmClientContext(pOsInterface)->IsPlanar(pSurface->OsResource.pGmmResInfo->GetResourceFormat()) != 0);
        auxDataY      = (uint8_t*)pSurfaceBase + pSurface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_Y_CCS);
        auxDataUV     = (uint8_t*)pSurfaceBase + pSurface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_UV_CCS);
        if (isPlanar)
        {
            auxSizeY = (uint32_t)(pSurface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_UV_CCS) -
                pSurface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_Y_CCS));
        }else
        {
            auxSizeY = (uint32_t)(pSurface->OsResource.pGmmResInfo->GetSizeAuxSurface(GMM_AUX_CCS));
        }

        auxSizeUV = (uint32_t)(pSurface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_COMP_STATE) -
            pSurface->OsResource.pGmmResInfo->GetPlanarAuxOffset(resourceIndex, GMM_AUX_UV_CCS));

        if (auxSizeUV == 0)
        {
            auxSizeUV = (uint32_t)(pSurface->OsResource.pGmmResInfo->GetSizeAuxSurface(GMM_AUX_SURF)) /
                (pSurface->OsResource.pGmmResInfo)->GetArraySize() - auxSizeY;
        }

        // Dump Y Aux data
        MOS_SecureStringPrint(
            sPath,
            MAX_PATH,
            sizeof(sPath),
            "%s_f[%04lld]_w[%d]_h[%d]_p[%d].Yaux",
            psPathPrefix,
            iCounter,
            pSurface->dwWidth,
            pSurface->dwHeight,
            pSurface->dwPitch);

        VpDumperTool::GetOsFilePath(sPath, sOsPath);

        uint8_t  *pDstAux = (uint8_t*)MOS_AllocAndZeroMemory(auxSizeY);
        VPHAL_DEBUG_CHK_NULL(pDstAux);

        MOS_SecureMemcpy(
            pDstAux,
            auxSizeY,
            auxDataY,
            auxSizeY);

        VPHAL_DEBUG_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(sOsPath, pDstAux, auxSizeY));
        MOS_SafeFreeMemory(pDstAux);

        if (auxSizeUV && isPlanar)
        {
            // Dump UV Aux data
            MOS_SecureStringPrint(
                sPath,
                MAX_PATH,
                sizeof(sPath),
                "%s_f[%04lld]_w[%d]_h[%d]_p[%d].UVaux",
                psPathPrefix,
                iCounter,
                pSurface->dwWidth,
                pSurface->dwHeight,
                pSurface->dwPitch);

            VpDumperTool::GetOsFilePath(sPath, sOsPath);

            uint8_t  *pDstUVAux = (uint8_t*)MOS_AllocAndZeroMemory(auxSizeUV);
            VPHAL_DEBUG_CHK_NULL(pDstUVAux);

            MOS_SecureMemcpy(
                pDstUVAux,
                auxSizeUV,
                auxDataUV,
                auxSizeUV);

            VPHAL_DEBUG_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(sOsPath, pDstUVAux, auxSizeUV));
            MOS_SafeFreeMemory(pDstUVAux);
        }
    }
#endif

finish:
    MOS_SafeFreeMemory(pDst);

    if (isSurfaceLocked && pLockedResource != nullptr)
    {
        eStatus = (MOS_STATUS)pOsInterface->pfnUnlockResource(pOsInterface, pLockedResource);
        VPHAL_DEBUG_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    }

    return eStatus;
}


MOS_STATUS VpSurfaceDumper::DumpSurfaceToFile(
    PMOS_INTERFACE          pOsInterface,
    PVP_SURFACE          pSurface,
    const char              *psPathPrefix,
    uint64_t                iCounter,
    bool                    bLockSurface,
    bool                    bNoDecompWhenLock,
    uint8_t*                pData)
{
    VP_FUNC_CALL();

    MOS_STATUS                          eStatus;
    bool                                isSurfaceLocked;
    char                                sPath[MAX_PATH], sOsPath[MAX_PATH];
    uint8_t                             *pDst, *pTmpSrc, *pTmpDst;
    uint32_t                            dwNumPlanes, dwSize, j, i;
    VPHAL_SURF_DUMP_SURFACE_DEF         planes[3];
    uint32_t                            dstPlaneOffset[3] = {0};
    MOS_LOCK_PARAMS                     LockFlags;
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData;
    bool                                hasAuxSurf;
    bool                                enableAuxDump;
    bool                                enablePlaneDump = false;
    PMOS_RESOURCE                       pLockedResource = nullptr;

    VPHAL_DEBUG_ASSERT(pSurface);
    VPHAL_DEBUG_ASSERT(pOsInterface);
    VPHAL_DEBUG_ASSERT(psPathPrefix);

    eStatus         = MOS_STATUS_SUCCESS;
    isSurfaceLocked = false;
    hasAuxSurf      = false;
    pDst            = nullptr;
    enableAuxDump   = m_dumpSpec.enableAuxDump;
    MOS_ZeroMemory(sPath,   MAX_PATH);
    MOS_ZeroMemory(sOsPath, MAX_PATH);
    dwNumPlanes = 0;
    enablePlaneDump = m_dumpSpec.enablePlaneDump;

    if (pSurface->osSurface->dwDepth == 0)
    {
        pSurface->osSurface->dwDepth = 1;
    }

    hasAuxSurf = HasAuxSurf(&pSurface->osSurface->OsResource);

    // get plane definitions
    VPHAL_DEBUG_CHK_STATUS(GetPlaneDefs(
        pSurface,
        planes,
        &dwNumPlanes,
        &dwSize,
        hasAuxSurf, //(hasAuxSurf && enableAuxDump),
        !enableAuxDump));// !(hasAuxSurf && enableAuxDump)));

    if (bLockSurface)
    {
        // Caller should not give pData when it expect the function to lock surf
        VPHAL_DEBUG_ASSERT(pData = nullptr);

        LockFlags.Value     = 0;
        LockFlags.ReadOnly  = 1;

        // If aux data exist and enable aux dump, no swizzle and no decompress
        if (hasAuxSurf && enableAuxDump)
        {
            LockFlags.TiledAsTiled = 1;
            LockFlags.NoDecompress = 1;
        }

        if (bNoDecompWhenLock)
        {
            LockFlags.NoDecompress = 1;
        }

        bool isPlanar = false;
        isPlanar      = (pSurface->osSurface->OsResource.Format == Format_NV12) || (pSurface->osSurface->OsResource.Format == Format_P010) || (pSurface->osSurface->OsResource.Format == Format_P016);

        {
            pData = (uint8_t *)pOsInterface->pfnLockResource(
                pOsInterface,
                &pSurface->osSurface->OsResource,
                &LockFlags);
            pLockedResource = &pSurface->osSurface->OsResource;
        }
        VPHAL_DEBUG_CHK_NULL(pData);

        // Write error to user feauture key
        eStatus = ReportUserSettingForDebug(
            m_userSettingPtr,
            __VPHAL_DBG_SURF_DUMPER_RESOURCE_LOCK,
            1,
            MediaUserSetting::Group::Sequence);

        VPHAL_DEBUG_ASSERT(eStatus == MOS_STATUS_SUCCESS);
        isSurfaceLocked = true;
    }

    MOS_SecureStringPrint(
        sPath,
        MAX_PATH,
        sizeof(sPath),
        "%s_f[%04lld]_w[%d]_h[%d]_p[%d].%s",
        psPathPrefix,
        iCounter,
        pSurface->osSurface->dwWidth,
        pSurface->osSurface->dwHeight,
        pSurface->osSurface->dwPitch,
        VpDumperTool::GetFormatStr(pSurface->osSurface->Format));

    VpDumperTool::GetOsFilePath(sPath, sOsPath);

    pDst = (uint8_t*)MOS_AllocAndZeroMemory(dwSize);
    VPHAL_DEBUG_CHK_NULL(pDst);
    VPHAL_DEBUG_CHK_NULL(pData);
    pTmpSrc = pData;
    pTmpDst = pDst;

    for (j = 0; j < dwNumPlanes; j++)
    {
        for (i = 0; i < planes[j].dwHeight; i++)
        {
            if (hasAuxSurf && enableAuxDump)
            {
                MOS_SecureMemcpy(
                    pTmpDst,
                    planes[j].dwPitch,
                    pTmpSrc,
                    planes[j].dwPitch);

                pTmpSrc += planes[j].dwPitch;
                pTmpDst += planes[j].dwPitch;

                dstPlaneOffset[j+1] += planes[j].dwPitch;
            }
            else
            {
                MOS_SecureMemcpy(
                    pTmpDst,
                    planes[j].dwWidth,
                    pTmpSrc,
                    planes[j].dwWidth);

                pTmpSrc += planes[j].dwPitch;
                pTmpDst += planes[j].dwWidth;

                dstPlaneOffset[j+1] += planes[j].dwWidth;
            }
        }

        //if more than 1 plane, dump each plane's surface for offline analysis
        if (enablePlaneDump)
        {
            if ((dwNumPlanes > 1) && (dwNumPlanes <= 3))
            {
                char sPlanePath[MAX_PATH], sPlaneOsPath[MAX_PATH];
                MOS_ZeroMemory(sPlanePath, MAX_PATH);
                MOS_ZeroMemory(sPlaneOsPath, MAX_PATH);

                MOS_SecureStringPrint(
                    sPlanePath,
                    MAX_PATH,
                    sizeof(sPlanePath),
                    "%s_f[%04lld]_w[%d]_h[%d]_p[%d].%s",
                    psPathPrefix,
                    iCounter,
                    planes[j].dwWidth,
                    planes[j].dwHeight,
                    planes[j].dwPitch,
                    (j == 0 ? "Y" : ((dwNumPlanes == 2) ? "UV" : ((j == 1) ? "U" : "V"))));

                VpDumperTool::GetOsFilePath(sPlanePath, sPlaneOsPath);

                VPHAL_DEBUG_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(sPlaneOsPath, pDst + dstPlaneOffset[j], dstPlaneOffset[j + 1]));
            }
            else
            {
                VPHAL_DEBUG_ASSERTMESSAGE("More than 3 planes not supported during plane dump.");
            }
        }
    }

    VPHAL_DEBUG_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(sOsPath, pDst, dwSize));

finish:
    MOS_SafeFreeMemory(pDst);

    if (isSurfaceLocked && pLockedResource != nullptr)
    {
        eStatus = (MOS_STATUS)pOsInterface->pfnUnlockResource(pOsInterface, pLockedResource);
        VPHAL_DEBUG_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    }

    return eStatus;
}

char* VpSurfaceDumper::WhitespaceTrim(
    char*   ptr)
{
    VP_FUNC_CALL();

    char*    pcTemp;                             // pointer to temp string to remove spces

    VPHAL_DEBUG_ASSERT(ptr);

    if(strlen(ptr) == 0)
    {
        goto finish;
    }

    //remove left spaces
    while (*ptr == ' ' || *ptr == '\t')
    {
        ptr++;
    }

    //remove right spaces
    pcTemp = ptr + strlen(ptr)-1;
    while(*pcTemp == ' '|| *pcTemp == '\t')
    {
        pcTemp--;
    }
    *(++pcTemp) = '\0';

finish:
    return ptr;
}

void VpDumperTool::StringToLower(
    char* pcString)
{
    VP_FUNC_CALL();

    size_t stStrLen;                                                           // length of string
    size_t i;                                                                  // loop iterator

    stStrLen = strlen(pcString);
    for (i = 0; i < stStrLen; i++)
    {
        if (isupper(pcString[i]))
        {
            pcString[i] = (char)tolower(pcString[i]);
        }
    }
}

MOS_STATUS VpSurfaceDumper::LocStringToEnum(
    char*                           pcLocString,
    uint32_t                        *pLocation)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    VpDumperTool::StringToLower(pcLocString);
    if (strcmp(pcLocString,      VPHAL_SURF_DUMP_LOC_PREALL)   == 0)
    {
        *pLocation = VPHAL_DUMP_TYPE_PRE_ALL;
    }
    else if (strcmp(pcLocString, VPHAL_SURF_DUMP_LOC_PREDNDI)  == 0)
    {
        *pLocation = VPHAL_DUMP_TYPE_PRE_DNDI;
    }
    else if (strcmp(pcLocString, VPHAL_SURF_DUMP_LOC_POSTDNDI) == 0)
    {
        *pLocation = VPHAL_DUMP_TYPE_POST_DNDI;
    }
    else if (strcmp(pcLocString, VPHAL_SURF_DUMP_LOC_PRECOMP)  == 0)
    {
        *pLocation = VPHAL_DUMP_TYPE_PRE_COMP;
    }
    else if (strcmp(pcLocString, VPHAL_SURF_DUMP_LOC_POSTCOMP) == 0)
    {
        *pLocation = VPHAL_DUMP_TYPE_POST_COMP;
    }
    else if (strcmp(pcLocString, VPHAL_SURF_DUMP_LOC_PREMEMDECOMP) == 0)
    {
        *pLocation = VPHAL_DUMP_TYPE_PRE_MEMDECOMP;
    }
    else if (strcmp(pcLocString, VPHAL_SURF_DUMP_LOC_POSTMEMDECOMP) == 0)
    {
        *pLocation = VPHAL_DUMP_TYPE_POST_MEMDECOMP;
    }
    else if (strcmp(pcLocString, VPHAL_SURF_DUMP_LOC_POSTALL)  == 0)
    {
        *pLocation = VPHAL_DUMP_TYPE_POST_ALL;
    }
    else if (strcmp(pcLocString, VPHAL_SURF_DUMP_LOC_VEBOX_DRIVERHEAP) == 0)
    {
        *pLocation = VPHAL_DUMP_TYPE_VEBOX_DRIVERHEAP;
    }
    else if (strcmp(pcLocString, VPHAL_SURF_DUMP_LOC_VEBOX_KERNELHEAP) == 0)
    {
        *pLocation = VPHAL_DUMP_TYPE_VEBOX_KERNELHEAP;
    }
    else
    {
        VPHAL_DEBUG_NORMALMESSAGE("Unknown dump location \"%s\".", pcLocString);
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

finish:
    return eStatus;
}

MOS_STATUS VpSurfaceDumper::EnumToLocString(
    uint32_t                        Location,
    char*                           pcLocString)
{
    VP_FUNC_CALL();

    MOS_STATUS  eStatus;
    uint32_t    i;
    size_t      stStrLen;

    eStatus = MOS_STATUS_SUCCESS;

    if (pcLocString == nullptr)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    switch (Location)
    {
        case VPHAL_DUMP_TYPE_PRE_ALL:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_SURF_DUMP_LOC_PREALL);
            break;
        case VPHAL_DUMP_TYPE_PRE_DNDI:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_SURF_DUMP_LOC_PREDNDI);
            break;
        case VPHAL_DUMP_TYPE_POST_DNDI:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_SURF_DUMP_LOC_POSTDNDI);
            break;
        case VPHAL_DUMP_TYPE_PRE_COMP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_SURF_DUMP_LOC_PRECOMP);
            break;
        case VPHAL_DUMP_TYPE_POST_COMP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_SURF_DUMP_LOC_POSTCOMP);
            break;
        case VPHAL_DUMP_TYPE_PRE_MEMDECOMP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_SURF_DUMP_LOC_PREMEMDECOMP);
            break;
        case VPHAL_DUMP_TYPE_POST_MEMDECOMP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_SURF_DUMP_LOC_POSTMEMDECOMP);
            break;
        case VPHAL_DUMP_TYPE_POST_ALL:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_SURF_DUMP_LOC_POSTALL);
            break;
        case VPHAL_DUMP_TYPE_VEBOX_DRIVERHEAP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_SURF_DUMP_LOC_VEBOX_DRIVERHEAP);
            break;
        case VPHAL_DUMP_TYPE_VEBOX_KERNELHEAP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_SURF_DUMP_LOC_VEBOX_KERNELHEAP);
            break;
        default:
            VPHAL_DEBUG_ASSERTMESSAGE("Unknown dump location \"%d\".", Location);
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
    } // end switch

    stStrLen = strlen(pcLocString);
    VPHAL_DEBUG_ASSERT(stStrLen > 1);          // assert b/c invalid access if <= 1
    i = pcLocString[1] == 'r' ? 3 : 4; // If pre, start i at 3, else 4
    // Maybe should add error case in case macros get changed later?
    for (; i < stStrLen; i++)
    {
        pcLocString[i] = (char)toupper(pcLocString[i]);
    }

finish:
    return eStatus;
}


MOS_STATUS VpSurfaceDumper::SurfTypeStringToEnum(
    char*                         pcSurfType,
    VPHAL_SURFACE_TYPE            *pSurfType)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;
    VpDumperTool::StringToLower(pcSurfType);
    if (strcmp(pcSurfType,      VPHAL_SURF_DUMP_TYPE_BACKGROUND)   == 0)
    {
        *pSurfType = SURF_IN_BACKGROUND;
    }
    else if (strcmp(pcSurfType, VPHAL_SURF_DUMP_TYPE_PRIMARY)      == 0)
    {
        *pSurfType = SURF_IN_PRIMARY;
    }
    else if (strcmp(pcSurfType, VPHAL_SURF_DUMP_TYPE_SUBSTREAM)    == 0)
    {
        *pSurfType = SURF_IN_SUBSTREAM;
    }
    else if (strcmp(pcSurfType, VPHAL_SURF_DUMP_TYPE_REFERENCE)    == 0)
    {
        *pSurfType = SURF_IN_REFERENCE;
    }
    else if (strcmp(pcSurfType, VPHAL_SURF_DUMP_TYPE_RENDERTARGET) == 0)
    {
        *pSurfType = SURF_OUT_RENDERTARGET;
    }
    else
    {
        VPHAL_DEBUG_ASSERTMESSAGE("Unknown surface type \"%s\".", pcSurfType);
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

finish:
    return eStatus;
}


MOS_STATUS VpSurfaceDumper::ProcessDumpLocations(
    char*                      pcDumpLocData)
{
    VP_FUNC_CALL();

    MOS_STATUS  eStatus;
    char*       pcCommaLoc;                                                        // pointer to next comma in dump location string
    char*       pcCurrToken;                                                       // pointer to current token in a string
    char*       pcColonLoc;                                                        // pointer to next colon in location string
    int32_t     iNumStrings;                                                       // number of dump locations
    int32_t     i;                                                                 // loop iteration counter
    VPHAL_SURF_DUMP_SPEC    *pDumpSpec = &m_dumpSpec;

    eStatus      = MOS_STATUS_SUCCESS;
    iNumStrings  = 0;

    if (strlen(pcDumpLocData) > 0)
    {
        // Count number of strings in key data
        // If non empty string, there is at least one value
        iNumStrings++;
        // Count the rest
        pcCommaLoc = pcDumpLocData;
        while ((pcCommaLoc = strchr(pcCommaLoc + 1, ',')) != nullptr)
        {
            iNumStrings++;
        }

        // Set number of strings and allocate space for them
        pDumpSpec->iNumDumpLocs = iNumStrings;
        pDumpSpec->pDumpLocations = (VPHAL_SURF_DUMP_LOC*)MOS_AllocAndZeroMemory(
                    sizeof(VPHAL_SURF_DUMP_LOC) * pDumpSpec->iNumDumpLocs);
        pcCurrToken = pcDumpLocData;

        // Determine each part of the string
        for (i = 0; i < pDumpSpec->iNumDumpLocs; i++)
        {
            pcCommaLoc = strchr(pcCurrToken, ',');
            if (pcCommaLoc != nullptr)
            {
                *pcCommaLoc = '\0';
            }

            // Handle surface type
            pcColonLoc = strchr(pcCurrToken, ':');
            if (pcColonLoc == nullptr)
            {
                // Dump all
                pDumpSpec->pDumpLocations[i].SurfType = SURF_NONE;
            }
            else
            {
                // Remove surface type from dump location
                *pcColonLoc = '\0';
                // Set surface type
                pcColonLoc++;

                pcColonLoc = WhitespaceTrim(pcColonLoc);
                VPHAL_DEBUG_CHK_STATUS(SurfTypeStringToEnum(pcColonLoc,
                                    &(pDumpSpec->pDumpLocations[i].SurfType)));
            }

            //trim the whitespaces from dump location
            pcCurrToken = WhitespaceTrim(pcCurrToken);

            VPHAL_DEBUG_CHK_STATUS(LocStringToEnum(pcCurrToken,
                                &(pDumpSpec->pDumpLocations[i].DumpLocation)));
            if (pcCommaLoc != nullptr)
            {
                pcCurrToken = pcCommaLoc + 1;
            }
        } // end for each part of the string
    } // if data length > 0
finish:
    return eStatus;
}

void VpSurfaceDumper::GetSurfaceDumpSpec()
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    char                            pcDumpLocData[VPHAL_SURF_DUMP_MAX_DATA_LEN];
    VPHAL_SURF_DUMP_SPEC           *pDumpSpec = &m_dumpSpec;
    MediaUserSetting::Value        outValue;

    pDumpSpec->uiStartFrame    = 0xFFFFFFFF;
    pDumpSpec->uiEndFrame      = 0;
    pDumpSpec->pcOutputPath[0] = '\0';
    pcDumpLocData[0]           = '\0';

    // Get start frame
    // if start frame is not got assign a default value of 0
    ReadUserSettingForDebug(
        m_userSettingPtr,
        pDumpSpec->uiStartFrame,
        __VPHAL_DBG_SURF_DUMP_START_FRAME_KEY_NAME,
        MediaUserSetting::Group::Sequence);

    // Get end frame
    // if end frame is not got assign a default value of max
    ReadUserSettingForDebug(
        m_userSettingPtr,
        pDumpSpec->uiEndFrame,
        __VPHAL_DBG_SURF_DUMP_END_FRAME_KEY_NAME,
        MediaUserSetting::Group::Sequence);

    // Get out file path
    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        __VPHAL_DBG_SURF_DUMP_OUTFILE_KEY_NAME,
        MediaUserSetting::Group::Sequence);

    if (outValue.ConstString().size() > 0 && outValue.ConstString().size() < MOS_USER_CONTROL_MAX_DATA_SIZE)
    {
        // Copy the Output path
        MOS_SecureMemcpy(
            pDumpSpec->pcOutputPath,
            MAX_PATH,
            outValue.ConstString().c_str(),
            outValue.ConstString().size());
    }
#if !defined(LINUX) && !defined(ANDROID)
    else
    {
        std::string vphalDumpFilePath;

        // Use state separation APIs to obtain appropriate storage location
        if (SUCCEEDED(GetDriverPersistentStorageLocation(vphalDumpFilePath)))
        {
            std::string m_outputFilePath;

            m_outputFilePath = vphalDumpFilePath.c_str();
            m_outputFilePath.append(VPHAL_DUMP_OUTPUT_FOLDER);

            // Copy the Output path
            MOS_SecureMemcpy(
                pDumpSpec->pcOutputPath,
                MAX_PATH,
                m_outputFilePath.c_str(),
                m_outputFilePath.size());

            ReportUserSettingForDebug(m_userSettingPtr, __VPHAL_DBG_DUMP_OUTPUT_DIRECTORY, m_outputFilePath, MediaUserSetting::Group::Sequence);
        }
    }
#endif

    // Get dump locations
    outValue = "";
    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        __VPHAL_DBG_SURF_DUMP_LOCATION_KEY_NAME,
        MediaUserSetting::Group::Sequence);
    if (outValue.ConstString().size() > 0 && outValue.ConstString().size() < MOS_USER_CONTROL_MAX_DATA_SIZE
        && pDumpSpec->pcOutputPath[0] != '\0')
    {
        VPHAL_DEBUG_CHK_STATUS(ProcessDumpLocations(const_cast<char *>(outValue.ConstString().c_str())));
    }

    // Get enableAuxDump
    ReadUserSettingForDebug(
        m_userSettingPtr,
        pDumpSpec->enableAuxDump,
        __VPHAL_DBG_SURF_DUMP_ENABLE_AUX_DUMP,
        MediaUserSetting::Group::Sequence);

    // Get plane dump enabled flag
    ReadUserSettingForDebug(
        m_userSettingPtr,
        pDumpSpec->enablePlaneDump,
        __VPHAL_DBG_SURF_DUMPER_ENABLE_PLANE_DUMP,
        MediaUserSetting::Group::Sequence);

finish:
    if ((eStatus != MOS_STATUS_SUCCESS) || (pDumpSpec->pcOutputPath[0] == '\0'))
    {
        pDumpSpec->uiStartFrame = 1;
        pDumpSpec->uiEndFrame   = 0;
    }
}

VpSurfaceDumper::VpSurfaceDumper(PMOS_INTERFACE pOsInterface) :
    m_dumpSpec(),
    m_osInterface(pOsInterface)
{
    if (m_osInterface)
    {
        m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    }
}

VpSurfaceDumper::~VpSurfaceDumper()
{
    MOS_SafeFreeMemory(m_dumpSpec.pDumpLocations);
}


MOS_STATUS VpSurfaceDumper::DumpSurface(
    PVPHAL_SURFACE                  pSurf,
    uint32_t                        uiFrameNumber,
    uint32_t                        uiCounter,
    uint32_t                        Location)
{
    VP_FUNC_CALL();

    int32_t VphalSurfDumpManualTrigger = VPHAL_SURF_DUMP_MANUAL_TRIGGER_DEFAULT_NOT_SET;

    MOS_STATUS  eStatus;
    int32_t     i;
    VPHAL_SURF_DUMP_SPEC*      pDumpSpec = &m_dumpSpec;
    bool                       isDumpFromDecomp;
    bool                       orgDumpAuxEnable;
    int32_t                    pid = MosUtilities::MosGetPid();
    uint64_t                   timeStamp = 0;

    MosUtilities::MosQueryPerformanceCounter(&timeStamp);
    eStatus = MOS_STATUS_SUCCESS;
    i       = 0;
    isDumpFromDecomp    = (Location == VPHAL_DUMP_TYPE_PRE_MEMDECOMP || Location == VPHAL_DUMP_TYPE_POST_MEMDECOMP);

    orgDumpAuxEnable    = m_dumpSpec.enableAuxDump;
    if (Location == VPHAL_DUMP_TYPE_PRE_MEMDECOMP)
    {
        // For PreMemDecomp, dump without aux is meaningless
        // and, if we don't turn on aux dump, the surface will be deswizzled,
        // while Mos_Specific_LockResource() will perform deswilling to temp buffer and then copy to locked buffer
        // This will break the compressed surface.
        // So, we cannot dump compressed surf with deswizzling under current implementation.
        m_dumpSpec.enableAuxDump = true; 
    }

    OsContextNext *osCtx = nullptr;

    if (m_osInterface && m_osInterface->osStreamState && m_osInterface->osStreamState->osDeviceContext)
    {
        osCtx = m_osInterface->osStreamState->osDeviceContext;
        if (osCtx && osCtx->GetDumpFrameNum() != 0xffffffff && isDumpFromDecomp)
        {
            // override the uiFrameNumer as it is during Vphal dumping its surface and already in lock and decomp phase
            uiFrameNumber = osCtx->GetDumpFrameNum();
        }

        if (osCtx && !isDumpFromDecomp)
        {
            osCtx->SetDumpFrameNum(uiFrameNumber);
        }

    }


    // Get if manual triggered build
    ReadUserSettingForDebug(
        m_userSettingPtr,
        VphalSurfDumpManualTrigger,
        __VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_KEY_NAME,
        MediaUserSetting::Group::Sequence);

    if (VphalSurfDumpManualTrigger != VPHAL_SURF_DUMP_MANUAL_TRIGGER_DEFAULT_NOT_SET)
    {
        if (VphalSurfDumpManualTrigger == VPHAL_SURF_DUMP_MANUAL_TRIGGER_STARTED)
        {
            VPHAL_DEBUG_NORMALMESSAGE("Dump manaul trigger enabled, dump started: %d \n", VphalSurfDumpManualTrigger);

            for (i = 0; i < pDumpSpec->iNumDumpLocs; i++)
            {
                if (pDumpSpec->pDumpLocations[i].DumpLocation == Location &&      // should dump at this pipeline location AND
                    (pDumpSpec->pDumpLocations[i].SurfType == pSurf->SurfType ||  // should dump for this surface type OR
                        pDumpSpec->pDumpLocations[i].SurfType == SURF_NONE))      // should dump for any surface type
                {
                    char *loc = nullptr;
                    if (osCtx)
                    {
                        loc = osCtx->GetdumpLoc();
                    }
                    VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, m_dumpLoc));
                    if (!isDumpFromDecomp && pSurf->bIsCompressed && loc)
                    {
                        EnumToLocString(Location, loc);
                    }

                    if (!isDumpFromDecomp || (loc && loc[0] == 0))
                    {
                        MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_pid%x_ts%llx_loc[%s]_lyr[%d]", pDumpSpec->pcOutputPath, pid, timeStamp, m_dumpLoc, uiCounter);
                    }
                    else
                    {
                        if (loc)
                        {
                            MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_pid%x_ts%llx_loc[%s_%s]_lyr[%d]", pDumpSpec->pcOutputPath, pid, timeStamp, loc, m_dumpLoc, uiCounter);
                        }
                    }
                    DumpSurfaceToFile(
                        m_osInterface,
                        pSurf,
                        m_dumpPrefix,
                        uiFrameNumber,
                        true,
                        isDumpFromDecomp,
                        nullptr);
                    break;
                }
            }
        }
        else if (VphalSurfDumpManualTrigger == VPHAL_SURF_DUMP_MANUAL_TRIGGER_STOPPED)
        {
            VPHAL_DEBUG_NORMALMESSAGE("Dump manaul trigger enabled, dump stopped: %d \n", VphalSurfDumpManualTrigger);
        }
        else
        {
            VPHAL_DEBUG_NORMALMESSAGE("Dump manaul trigger flag: %d \n", VphalSurfDumpManualTrigger);
        }
    }
    else if (pDumpSpec->uiStartFrame <= uiFrameNumber &&
             uiFrameNumber <= pDumpSpec->uiEndFrame)
    {
        for (i = 0; i < pDumpSpec->iNumDumpLocs; i++)
        {
            if (pDumpSpec->pDumpLocations[i].DumpLocation == Location &&        // should dump at this pipeline location AND
                (pDumpSpec->pDumpLocations[i].SurfType == pSurf->SurfType ||    // should dump for this surface type OR
                 pDumpSpec->pDumpLocations[i].SurfType == SURF_NONE))           // should dump for any surface type
            {
                char *loc = nullptr;
                if (osCtx)
                {
                    loc = osCtx->GetdumpLoc();
                }
                VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, m_dumpLoc));
                if (!isDumpFromDecomp && pSurf->bIsCompressed && loc)
                {
                    EnumToLocString(Location, loc);
                }

                if (!isDumpFromDecomp || (loc && loc[0] == 0))
                {
                    MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_pid%x_ts%llx_loc_loc[%s]_lyr[%d]",
                        pDumpSpec->pcOutputPath, pid, timeStamp, m_dumpLoc, uiCounter);
                }
                else
                {
                    if (loc)
                    {
                        MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_pid%x_ts%llx_loc_loc[%s_%s]_lyr[%d]",
                            pDumpSpec->pcOutputPath, pid, timeStamp, loc, m_dumpLoc, uiCounter);
                    }
                }

                DumpSurfaceToFile(
                    m_osInterface,
                    pSurf,
                    m_dumpPrefix,
                    uiFrameNumber,
                    true,
                    isDumpFromDecomp,
                    nullptr);
                break;
            }
        }
    }
    else
    {
        VPHAL_DEBUG_VERBOSEMESSAGE("No surface dumpped, VphalSurfDumpManualTrigger: %d, uiStartFrame: %d,  uiEndFrame: %d\n", VphalSurfDumpManualTrigger, pDumpSpec->uiStartFrame, pDumpSpec->uiEndFrame);
    }

finish:
    if (!isDumpFromDecomp && osCtx)
    {
        osCtx->ResetDumpFrameNum();
        osCtx->ResetdumpLoc();
    }
    m_dumpSpec.enableAuxDump = orgDumpAuxEnable;

    return eStatus;
}

MOS_STATUS VpSurfaceDumper::DumpSurface(
    PVP_SURFACE                  pSurf,
    uint32_t                        uiFrameNumber,
    uint32_t                        uiCounter,
    uint32_t                        Location)
{
    VP_FUNC_CALL();

    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    int32_t VphalSurfDumpManualTrigger = VPHAL_SURF_DUMP_MANUAL_TRIGGER_DEFAULT_NOT_SET;

    MOS_STATUS  eStatus;
    int32_t     i;
    VPHAL_SURF_DUMP_SPEC*      pDumpSpec = &m_dumpSpec;
    bool                       isDumpFromDecomp;
    bool                       orgDumpAuxEnable;
    int32_t                    pid = MosUtilities::MosGetPid();
    uint64_t                   timeStamp = 0;

    MosUtilities::MosQueryPerformanceCounter(&timeStamp);
    eStatus = MOS_STATUS_SUCCESS;
    i       = 0;
    isDumpFromDecomp    = (Location == VPHAL_DUMP_TYPE_PRE_MEMDECOMP || Location == VPHAL_DUMP_TYPE_POST_MEMDECOMP);

    orgDumpAuxEnable    = m_dumpSpec.enableAuxDump;
    if (Location == VPHAL_DUMP_TYPE_PRE_MEMDECOMP)
    {
        // For PreMemDecomp, dump without aux is meaningless
        // and, if we don't turn on aux dump, the surface will be deswizzled,
        // while Mos_Specific_LockResource() will perform deswilling to temp buffer and then copy to locked buffer
        // This will break the compressed surface.
        // So, we cannot dump compressed surf with deswizzling under current implementation.
        m_dumpSpec.enableAuxDump = true; 
    }

    OsContextNext *osCtx = nullptr;

    if (m_osInterface && m_osInterface->osStreamState && m_osInterface->osStreamState->osDeviceContext)
    {
        osCtx = m_osInterface->osStreamState->osDeviceContext;
        if (osCtx && osCtx->GetDumpFrameNum() != 0xffffffff && isDumpFromDecomp)
        {
            // override the uiFrameNumer as it is during Vphal dumping its surface and already in lock and decomp phase
            uiFrameNumber = osCtx->GetDumpFrameNum();
        }

        if (osCtx && !isDumpFromDecomp)
        {
            osCtx->SetDumpFrameNum(uiFrameNumber);
        }

    }

    // Get if manual triggered build
    ReadUserSettingForDebug(
        m_userSettingPtr,
        VphalSurfDumpManualTrigger,
        __VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_KEY_NAME,
        MediaUserSetting::Group::Sequence);

    if (VphalSurfDumpManualTrigger != VPHAL_SURF_DUMP_MANUAL_TRIGGER_DEFAULT_NOT_SET)
    {
        if (VphalSurfDumpManualTrigger == VPHAL_SURF_DUMP_MANUAL_TRIGGER_STARTED)
        {
            VPHAL_DEBUG_NORMALMESSAGE("Dump manaul trigger enabled, dump started: %d \n", VphalSurfDumpManualTrigger);

            for (i = 0; i < pDumpSpec->iNumDumpLocs; i++)
            {
                if (pDumpSpec->pDumpLocations[i].DumpLocation == Location &&      // should dump at this pipeline location AND
                    (pDumpSpec->pDumpLocations[i].SurfType == pSurf->SurfType ||  // should dump for this surface type OR
                        pDumpSpec->pDumpLocations[i].SurfType == SURF_NONE))      // should dump for any surface type
                {
                    char *loc = nullptr;
                    if (osCtx)
                    {
                        loc = osCtx->GetdumpLoc();
                    }
                    VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, m_dumpLoc));
                    if (!isDumpFromDecomp && pSurf->osSurface->bIsCompressed && loc)
                    {
                        EnumToLocString(Location, loc);
                    }

                    if (!isDumpFromDecomp || (loc && loc[0] == 0))
                    {
                        MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_pid%x_ts%llx_loc[%s]_lyr[%d]", pDumpSpec->pcOutputPath, pid, timeStamp, m_dumpLoc, uiCounter);
                    }
                    else
                    {
                        if (loc)
                        {
                            MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_pid%x_ts%llx_loc[%s_%s]_lyr[%d]", pDumpSpec->pcOutputPath, pid, timeStamp, loc, m_dumpLoc, uiCounter);
                        }
                    }
                    DumpSurfaceToFile(
                        m_osInterface,
                        pSurf,
                        m_dumpPrefix,
                        uiFrameNumber,
                        true,
                        isDumpFromDecomp,
                        nullptr);
                    break;
                }
            }
        }
        else if (VphalSurfDumpManualTrigger == VPHAL_SURF_DUMP_MANUAL_TRIGGER_STOPPED)
        {
            VPHAL_DEBUG_NORMALMESSAGE("Dump manaul trigger enabled, dump stopped: %d \n", VphalSurfDumpManualTrigger);
        }
        else
        {
            VPHAL_DEBUG_NORMALMESSAGE("Dump manaul trigger flag: %d \n", VphalSurfDumpManualTrigger);
        }
    }
    else if (pDumpSpec->uiStartFrame <= uiFrameNumber &&
             uiFrameNumber <= pDumpSpec->uiEndFrame)
    {
        for (i = 0; i < pDumpSpec->iNumDumpLocs; i++)
        {
            if (pDumpSpec->pDumpLocations[i].DumpLocation == Location &&        // should dump at this pipeline location AND
                (pDumpSpec->pDumpLocations[i].SurfType == pSurf->SurfType ||    // should dump for this surface type OR
                 pDumpSpec->pDumpLocations[i].SurfType == SURF_NONE))           // should dump for any surface type
            {
                char *loc = nullptr;
                if (osCtx)
                {
                    loc = osCtx->GetdumpLoc();
                }
                VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, m_dumpLoc));
                if (!isDumpFromDecomp && pSurf->osSurface->bIsCompressed && loc)
                {
                    EnumToLocString(Location, loc);
                }

                if (!isDumpFromDecomp || (loc && loc[0] == 0))
                {
                    MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_pid%x_ts%llx_loc[%s]_lyr[%d]",
                        pDumpSpec->pcOutputPath, pid, timeStamp, m_dumpLoc, uiCounter);
                }
                else
                {
                    if (loc)
                    {
                        MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_pid%x_ts%llx_loc[%s_%s]_lyr[%d]",
                            pDumpSpec->pcOutputPath, pid, timeStamp, loc, m_dumpLoc, uiCounter);
                    }
                }

                DumpSurfaceToFile(
                    m_osInterface,
                    pSurf,
                    m_dumpPrefix,
                    uiFrameNumber,
                    true,
                    isDumpFromDecomp,
                    nullptr);
                break;
            }
        }
    }
    else
    {
        VPHAL_DEBUG_VERBOSEMESSAGE("No surface dumpped, VphalSurfDumpManualTrigger: %d, uiStartFrame: %d,  uiEndFrame: %d\n", VphalSurfDumpManualTrigger, pDumpSpec->uiStartFrame, pDumpSpec->uiEndFrame);
    }

finish:
    if (!isDumpFromDecomp && osCtx)
    {
        osCtx->ResetDumpFrameNum();
        osCtx->ResetdumpLoc();
    }
    m_dumpSpec.enableAuxDump = orgDumpAuxEnable;

    return eStatus;
}

MOS_STATUS VpSurfaceDumper::DumpSurfaceArray(
    PVPHAL_SURFACE                  *ppSurfaces,
    uint32_t                        uiMaxSurfaces,
    uint32_t                        uiNumSurfaces,
    uint32_t                        uiFrameNumber,
    uint32_t                        Location)
{
    VP_FUNC_CALL();

    MOS_STATUS      eStatus;
    uint32_t        uiIndex;
    uint32_t        uiLayer;

    //---------------------------------------------------
    VPHAL_DEBUG_ASSERT(ppSurfaces);
    //---------------------------------------------------

    eStatus = MOS_STATUS_SUCCESS;

    for (uiIndex = 0, uiLayer = 0;
         uiLayer < uiNumSurfaces && uiIndex < uiMaxSurfaces; uiIndex++)
    {
        if (ppSurfaces[uiIndex])
        {
            VPHAL_DEBUG_ASSERT(!Mos_ResourceIsNull(&ppSurfaces[uiIndex]->OsResource));

            VPHAL_DEBUG_CHK_STATUS(DumpSurface(
                ppSurfaces[uiIndex],
                uiFrameNumber,
                uiLayer,
                Location));

            uiLayer++;
        }
    }

finish:
    return eStatus;
}


void VpParameterDumper::GetParametersDumpSpec()
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    bool                            bDumpEnabled;
    VPHAL_PARAMS_DUMP_SPEC         *pDumpSpec = &m_dumpSpec;
    MediaUserSetting::Value        outValue;

    pDumpSpec->uiStartFrame       = 0xFFFFFFFF;
    pDumpSpec->uiEndFrame         = 0;
    pDumpSpec->outFileLocation[0] = '\0';
    bDumpEnabled                  = false;

    // Get start frame
    // if start frame is not got assign a default value of 0
    ReadUserSettingForDebug(
        m_userSettingPtr,
        pDumpSpec->uiStartFrame,
        __VPHAL_DBG_PARAM_DUMP_START_FRAME_KEY_NAME,
        MediaUserSetting::Group::Sequence);

    // Get end frame
    // if end frame is not got assign a default value of max
    ReadUserSettingForDebug(
        m_userSettingPtr,
        pDumpSpec->uiEndFrame,
        __VPHAL_DBG_PARAM_DUMP_END_FRAME_KEY_NAME,
        MediaUserSetting::Group::Sequence);

    // Get out file path
    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        __VPHAL_DBG_PARAM_DUMP_OUTFILE_KEY_NAME,
        MediaUserSetting::Group::Sequence);
    if (outValue.ConstString().size() > 0 && outValue.ConstString().size() < MOS_USER_CONTROL_MAX_DATA_SIZE)
    {
        // Copy the Output path
        MOS_SecureMemcpy(
            pDumpSpec->outFileLocation,
            MAX_PATH,
            outValue.ConstString().c_str(),
            outValue.ConstString().size());
        bDumpEnabled = true;
    }
#if !defined(LINUX) && !defined(ANDROID)
    else
    {
        std::string vphalDumpFilePath;

        // Use state separation APIs to obtain appropriate storage location
        if (SUCCEEDED(GetDriverPersistentStorageLocation(vphalDumpFilePath)))
        {
            std::string m_outputFilePath;

            m_outputFilePath = vphalDumpFilePath.c_str();
            m_outputFilePath.append(VPHAL_DUMP_OUTPUT_FOLDER);

            // Copy the Output path
            MOS_SecureMemcpy(
                pDumpSpec->outFileLocation,
                MAX_PATH,
                m_outputFilePath.c_str(),
                m_outputFilePath.size());

            ReportUserSettingForDebug(m_userSettingPtr, __VPHAL_DBG_DUMP_OUTPUT_DIRECTORY, m_outputFilePath, MediaUserSetting::Group::Sequence);

            bDumpEnabled = true;
        }
    }
#endif

    // Get enableSkuWaDump
    ReadUserSettingForDebug(
        m_userSettingPtr,
        pDumpSpec->enableSkuWaDump,
        __VPHAL_DBG_PARA_DUMP_ENABLE_SKUWA_DUMP,
        MediaUserSetting::Group::Sequence);

    if ((eStatus != MOS_STATUS_SUCCESS) || (!bDumpEnabled))
    {
        pDumpSpec->uiStartFrame = 1;
        pDumpSpec->uiEndFrame = 0;
    }
}

MOS_STATUS VpParameterDumper::DumpSourceSurface(
    uint32_t                        uiFrameCounter,
    char                            *pcOutputPath,
    PVPHAL_SURFACE                  pSrc,
    uint32_t                        index,
    char*                           &pcOutContents)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus;
    char                            sSurfaceFilePath[MAX_PATH] = { 0 }, sOsSurfaceFilePath[MAX_PATH] = { 0 };
    int32_t                         pid = MosUtilities::MosGetPid();
    uint64_t                        timeStamp = 0;

    MosUtilities::MosQueryPerformanceCounter(&timeStamp);
    eStatus               = MOS_STATUS_SUCCESS;

    //Color Information
    {
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Color Information -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_CSPACE>%s</VPHAL_CSPACE>\n",             GetColorSpaceStr(pSrc->ColorSpace)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<EXTENDED_GAMUT></EXTENDED_GAMUT>\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<PALETTE_ALLOCATION>%d</PALETTE_ALLOCATION>\n", (pSrc->iPalette)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<PALETTE_DATA>\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<PALETTE_DATA_TYPE>%s</PALETTE_DATA_TYPE>\n", GetPaletteTypeStr(pSrc->Palette.PaletteType)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_CSPACE>%s</VPHAL_CSPACE>\n",           GetColorSpaceStr(pSrc->Palette.ColorSpace)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HAS_ALPHA>%d</HAS_ALPHA>\n",                 pSrc->Palette.bHasAlpha ? 1 : 0));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<TOTAL_ENTRIES>%d</TOTAL_ENTRIES>\n",         pSrc->Palette.iTotalEntries));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<NUM_ENTRIES>%d</NUM_ENTRIES>\n",             pSrc->Palette.iNumEntries));
        for (int nIndex = 0; nIndex < pSrc->Palette.iTotalEntries; nIndex++)
        {
            if (pSrc->Palette.pPalette8)
            {
                VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<PVPHAL_COLOR_SAMPLE>%08x</PVPHAL_COLOR_SAMPLE>\n", pSrc->Palette.pPalette8[nIndex].dwValue));
            }
        }
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</PALETTE_DATA>\n"));

        //Rendering parameters
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Rendering parameters -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_BLENDING_PARAMS>\n"));
        if (pSrc->pBlendingParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_BLEND_TYPE>%s</VPHAL_BLEND_TYPE>\n", GetBlendTypeStr(pSrc->pBlendingParams->BlendType)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ALPHA type=\"real\">%.3f</ALPHA>\n",       pSrc->pBlendingParams->fAlpha));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ALPHA type=\"real\"></ALPHA>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_BLENDING_PARAMS>\n"));

        //Luma key params
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_LUMAKEY_PARAMS>\n"));
        if (pSrc->pLumaKeyParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<LUMA_LOW>%d</LUMA_LOW>\n", pSrc->pLumaKeyParams->LumaLow));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<LUMA_HIGH>%d</LUMA_HIGH>\n", pSrc->pLumaKeyParams->LumaHigh));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<LUMA_LOW></LUMA_LOW>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<LUMA_HIGH></LUMA_HIGH>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_LUMAKEY_PARAMS>\n"));

        //Propcamp params
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_PROCAMP_PARAMS>\n"));
        if (pSrc->pProcampParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLED>%d</ENABLED>\n",                       (pSrc->pProcampParams->bEnabled ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<BRIGHTNESS type=\"real\">%.3f</BRIGHTNESS>\n", pSrc->pProcampParams->fBrightness));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<CONTRAST type=\"real\">%.3f</CONTRAST>\n",     pSrc->pProcampParams->fContrast));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HUE type=\"real\">%.3f</HUE>\n",               pSrc->pProcampParams->fHue));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<SATURATION type=\"real\">%.3f</SATURATION>\n", pSrc->pProcampParams->fSaturation));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLED></ENABLED>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<BRIGHTNESS type=\"real\"></BRIGHTNESS>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<CONTRAST type=\"real\"></CONTRAST>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HUE type=\"real\"></HUE>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<SATURATION type=\"real\"></SATURATION>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_PROCAMP_PARAMS>\n"));

        //IEF parameter
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_IEF_PARAMS>\n"));
        if (pSrc->pIEFParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<IEFFACTOR type = \"real\">%.3f</IEFFACTOR>\n", pSrc->pIEFParams->fIEFFactor));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<IEFFACTOR type = \"real\"></IEFFACTOR>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_IEF_PARAMS>\n"));

        //Advanced processing
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Advanced processing -->\n"));
        //DI
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_DI_PARAMS>\n"));
        if (pSrc->pDeinterlaceParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<DIMODE>%s</DIMODE>\n",             GetDIModeStr(pSrc->pDeinterlaceParams->DIMode)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<ENABLE_FMD>%d</ENABLE_FMD>\n",     (pSrc->pDeinterlaceParams->bEnableFMD ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<SINGLE_FIELD>%d</SINGLE_FIELD>\n", (pSrc->pDeinterlaceParams->bSingleField ? 1 : 0)));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<DIMODE></DIMODE>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<ENABLE_FMD></ENABLE_FMD>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<SINGLE_FIELD></SINGLE_FIELD>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_DI_PARAMS>\n"));
        //Denoise
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_DENOISE_PARAMS>\n"));
        if (pSrc->pDenoiseParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_LUMA>%d</ENABLE_LUMA>\n",         (pSrc->pDenoiseParams->bEnableLuma ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_CHROMA>%d</ENABLE_CHROMA>\n",     (pSrc->pDenoiseParams->bEnableChroma ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<AUTO_DETECT>%d</AUTO_DETECT>\n",         (pSrc->pDenoiseParams->bAutoDetect ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<DENOISE_FACTOR>%.3f</DENOISE_FACTOR>\n", (pSrc->pDenoiseParams->fDenoiseFactor)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<NOISE_LEVEL>%s</NOISE_LEVEL>\n", GetDenoiseModeStr(pSrc->pDenoiseParams->NoiseLevel)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_HVS_DENOISE>%d</ENABLE_HVS_DENOISE>\n", (pSrc->pDenoiseParams->bEnableHVSDenoise ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HVS_DENOISE_QP>%d</HVS_DENOISE_QP>\n", (pSrc->pDenoiseParams->HVSDenoise.QP)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HVS_DENOISE_STRENGTH>%d</HVS_DENOISE_STRENGTH>\n", (pSrc->pDenoiseParams->HVSDenoise.Strength)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HVS_DENOISE_MODE>%s</HVS_DENOISE_MODE>\n", GetHVSDenoiseModeStr(pSrc->pDenoiseParams->HVSDenoise.Mode)));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_LUMA></ENABLE_LUMA>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_CHROMA></ENABLE_CHROMA>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<AUTO_DETECT></AUTO_DETECT>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<DENOISE_FACTOR></DENOISE_FACTOR>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_DENOISE_PARAMS>\n"));
        //ColorPipe
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_COLORPIPE_PARAMS>\n"));
        if (pSrc->pColorPipeParams)
        {
            VPHAL_TCC_PARAMS TccParams = pSrc->pColorPipeParams->TccParams;
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_ACE>%d</ENABLE_ACE>\n",                (pSrc->pColorPipeParams->bEnableACE ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_STE>%d</ENABLE_STE>\n",                (pSrc->pColorPipeParams->bEnableSTE ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_TCC>%d</ENABLE_TCC>\n",                (pSrc->pColorPipeParams->bEnableTCC ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ACE_LEVEL>%d</ACE_LEVEL>\n",                  (pSrc->pColorPipeParams->dwAceLevel)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ACE_STRENGHTH>%d</ACE_STRENGHTH>\n",          (pSrc->pColorPipeParams->dwAceStrength)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<STE_PARAMS>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t\t<STE_FACTOR>%d</STE_FACTOR>\n",              (pSrc->pColorPipeParams->SteParams.dwSTEFactor)));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t</STE_PARAMS>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<TCC_PARAMS>%d,%d,%d,%d,%d,%d</TCC_PARAMS>\n", TccParams.Blue, TccParams.Cyan, TccParams.Green, TccParams.Magenta, TccParams.Red, TccParams.Yellow));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_ACE></ENABLE_ACE>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_STE></ENABLE_STE>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_TCC></ENABLE_TCC>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ACE_LEVEL></ACE_LEVEL>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<STE_PARAMS></STE_PARAMS>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<TCC_PARAMS></TCC_PARAMS>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_COLORPIPE_PARAMS>\n"));
        //Gamut
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_GAMUT_PARAMS>\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<GCOMP_MODE></GCOMP_MODE>\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_GAMUT_PARAMS>\n"));

        //Sample information
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Sample information -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_FORMAT>%s</VPHAL_FORMAT>\n",             GetWholeFormatStr(pSrc->Format)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_SURFACE_TYPE>%s</VPHAL_SURFACE_TYPE>\n", GetSurfaceTypeStr(pSrc->SurfType)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_SAMPLE_TYPE>%s</VPHAL_SAMPLE_TYPE>\n",   GetSampleTypeStr(pSrc->SampleType)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_SCALING_MODE>%s</VPHAL_SCALING_MODE>\n", GetScalingModeStr(pSrc->ScalingMode)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_ROTATION_MODE>%s</VPHAL_ROTATION_MODE>\n", GetRotationModeStr(pSrc->Rotation)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<RCSRC>%d,%d,%d,%d</RCSRC>\n",                  pSrc->rcSrc.left, pSrc->rcSrc.top, pSrc->rcSrc.right, pSrc->rcSrc.bottom));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<RCDST>%d,%d,%d,%d</RCDST>\n",                  pSrc->rcDst.left, pSrc->rcDst.top, pSrc->rcDst.right, pSrc->rcDst.bottom));

        //Basic information
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Basic information -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_TILE_TYPE>%s</VPHAL_TILE_TYPE>\n", GetTileTypeStr(pSrc->TileType)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<WIDTH>%d</WIDTH>\n",                     pSrc->dwWidth));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<HEIGHT>%d</HEIGHT>\n",                   pSrc->dwHeight));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<PITCH>%d</PITCH>\n",                     pSrc->dwPitch));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<SIZE>%d</SIZE>\n",                       pSrc->dwPitch * pSrc->dwHeight));

        //Surface content initialization
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Surface content initialization -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<DATA>\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<DEFAULT_COLOR type=\"integer\">0x000000FF</DEFAULT_COLOR>\n"));
        if (pcOutputPath)
        {
            memset(sSurfaceFilePath, 0, MAX_PATH);
            memset(sOsSurfaceFilePath, 0, MAX_PATH);

            MOS_SecureStringPrint(sSurfaceFilePath, MAX_PATH, MAX_PATH, "%s%csurfdump_pid%x_ts%llx_loc[preALL]_lyr[%d]_f[%04d]_w[%d]_h[%d]_p[%d].%s",
                pcOutputPath, MOS_DIR_SEPERATOR, pid, timeStamp, index, uiFrameCounter, pSrc->dwWidth, pSrc->dwHeight, pSrc->dwPitch, VpDumperTool::GetFormatStr(pSrc->Format));
            VpDumperTool::GetOsFilePath(sSurfaceFilePath, sOsSurfaceFilePath);
        }
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<FILE>%s</FILE>\n", sOsSurfaceFilePath));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</DATA>\n"));
        // get backward reference
        if (pSrc->pBwdRef)
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<BACKREFDATA>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<DEFAULT_COLOR type=\"integer\">0x000000FF</DEFAULT_COLOR>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<Num>%d</Num>\n", pSrc->uBwdRefCount));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<FILE></FILE>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</BACKREFDATA>\n"));
        }
        // get forward reference
        if (pSrc->pFwdRef)
        {
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<FWDREFDATA>\n"));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<Num>%d</Num>\n", pSrc->uFwdRefCount));
            VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</FWDREFDATA>\n"));
        }
    }

    finish:
        return eStatus;
}

MOS_STATUS VpParameterDumper::DumpTargetSurface(
    uint32_t                        uiFrameCounter,
    char                            *pcOutputPath,
    PVPHAL_SURFACE                  pTarget,
    uint32_t                        index,
    char*                           &pcOutContents)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_FORMAT>%s</VPHAL_FORMAT>\n",             VpParameterDumper::GetWholeFormatStr(pTarget->Format)));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_SURFACE_TYPE>%s</VPHAL_SURFACE_TYPE>\n", VpParameterDumper::GetSurfaceTypeStr(pTarget->SurfType)));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_SAMPLE_TYPE>%s</VPHAL_SAMPLE_TYPE>\n",   GetSampleTypeStr(pTarget->SampleType)));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_CSPACE>%s</VPHAL_CSPACE>\n",             GetColorSpaceStr(pTarget->ColorSpace)));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_SCALING_MODE>%s</VPHAL_SCALING_MODE>\n", GetScalingModeStr(pTarget->ScalingMode)));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_TILE_TYPE>%s</VPHAL_TILE_TYPE>\n",       GetTileTypeStr(pTarget->TileType)));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<WIDTH>%d</WIDTH>\n",                           pTarget->dwWidth));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HEIGHT>%d</HEIGHT>\n",                         pTarget->dwHeight));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<PITCH>%d</PITCH>\n",                           pTarget->dwPitch));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<SIZE>%d</SIZE>\n",                             pTarget->dwPitch*pTarget->dwHeight));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<RCSRC>%d,%d,%d,%d</RCSRC>\n",                    pTarget->rcSrc.left, pTarget->rcSrc.top, pTarget->rcSrc.right, pTarget->rcSrc.bottom));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<RCDST>%d,%d,%d,%d</RCDST>\n",                    pTarget->rcDst.left, pTarget->rcDst.top, pTarget->rcDst.right, pTarget->rcDst.bottom));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<DATA>\n"));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t\t<DEFAULT_COLOR type=\"integer\">0x000000FF</DEFAULT_COLOR>\n"));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t\t<FILE></FILE>\n"));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t</DATA>\n"));

finish:
    return eStatus;
}

MOS_STATUS VpParameterDumper::DumpRenderParameter(
    uint32_t                        uiFrameCounter,
    char                            *pcOutputPath,
    PVPHAL_RENDER_PARAMS            pRenderParams,
    char*                           &pcOutContents)
{
    VP_FUNC_CALL();

    MOS_STATUS                      eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    if (pRenderParams->pTarget[0])
    {
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_SURFACE_TARGET>\n"));
        VPHAL_DEBUG_CHK_STATUS(DumpTargetSurface(
            uiFrameCounter,
            pcOutputPath,
            pRenderParams->pTarget[0],
            0,
            pcOutContents));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_SURFACE_TARGET>\n"));
    }
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<SRC_COUNT>%d</SRC_COUNT>\n", pRenderParams->uSrcCount));

    //Color fill
    if (pRenderParams->pColorFillParams)
    {
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_COLORFILL_PARAMS>\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<bYCbCr>%d</bYCbCr>\n", pRenderParams->pColorFillParams->bYCbCr));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<COLOR type=\"integer\">%08x</COLOR>\n", pRenderParams->pColorFillParams->Color));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t\t<CSPACE>%s</CSPACE>\n", GetColorSpaceStr(pRenderParams->pColorFillParams->CSpace)));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_COLORFILL_PARAMS>\n"));
    }

    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_COMPONENT>%s</VPHAL_COMPONENT>\n", GetComponentStr(pRenderParams->Component)));

finish:
    return eStatus;
}

MOS_STATUS VpParameterDumper::DumpToXML(
    uint32_t                        uiFrameCounter,
    char                            *pcOutputPath,
    PVPHAL_RENDER_PARAMS            pRenderParams)
{
    VP_FUNC_CALL();

    char                            sPath[MAX_PATH] = { 0 }, sOsPath[MAX_PATH] = { 0 };
    MOS_STATUS                      eStatus;
    char*                           pcOutContents;
    uint32_t                        dwStrLen = 0;
    FILE                            *fpOutXML;
    char*                           pCurFrameFileName;
    char*                           pBwdFrameFileName;
    VPHAL_PARAMS_DUMP_SPEC         *pParamsDumpSpec = &m_dumpSpec;
    int32_t                         pid = MosUtilities::MosGetPid();
    uint64_t                        timeStamp = 0;

    MosUtilities::MosQueryPerformanceCounter(&timeStamp);
    eStatus               = MOS_STATUS_SUCCESS;
    dwStrLen              = 0;
    pcOutContents         = nullptr;
    fpOutXML              = nullptr;
    pCurFrameFileName     = nullptr;
    pBwdFrameFileName     = nullptr;

    VPHAL_DEBUG_CHK_NULL(pRenderParams);
    VPHAL_DEBUG_CHK_NULL(pParamsDumpSpec);

    if ((pParamsDumpSpec->uiEndFrame < pParamsDumpSpec->uiStartFrame) || strlen(pParamsDumpSpec->outFileLocation) == 0)
        goto finish;

    // Create a processing instruction element.
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(true, &pcOutContents,  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\n"));
    // Create the root element.
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "<VPHAL_SCENARIO>\n"));
    // General infomation
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<ID>%d</ID>\n", MosUtilities::MosGetPid()));

    VPHAL_DEBUG_CHK_NULL(pRenderParams->pSrc[0]);
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<DESCRIPTION>%d</DESCRIPTION>\n", pRenderParams->pSrc[0]->FrameID));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<!-- Number of frames to render -->\n"));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<FRAME_COUNT type = \"integer\">1</FRAME_COUNT>\n"));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<!-- 60i or 30p BLT -->\n"));
    if ((pRenderParams->uSrcCount > 0) &&
        (pRenderParams->pSrc[0]->SampleType != SAMPLE_PROGRESSIVE))
    {
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<BLT_TYPE>60i</BLT_TYPE>\n"));
    }
    else
    {
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<BLT_TYPE>30p</BLT_TYPE>\n"));
    }

    // Surface
    for (uint32_t i = 0; i < pRenderParams->uSrcCount; i++)
    {
        if (pRenderParams->pSrc[i] == nullptr)
            continue;

        //surface infomation
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<!-- Input surface definitions -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<VPHAL_SURFACE>\n"));

        VPHAL_DEBUG_CHK_STATUS(DumpSourceSurface(
            uiFrameCounter,
            pcOutputPath,
            pRenderParams->pSrc[i],
            i,
            pcOutContents));
        VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t</VPHAL_SURFACE>\n"));
    }

    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<!-- Rendering parameters -->\n"));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t<VPHAL_RENDER_PARAMS>\n"));

    VPHAL_DEBUG_CHK_STATUS(DumpRenderParameter(
        uiFrameCounter,
        pcOutputPath,
        pRenderParams,
        pcOutContents));

    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "\t</VPHAL_RENDER_PARAMS>\n"));
    VPHAL_DEBUG_CHK_STATUS(VpDumperTool::AppendString(false, &pcOutContents, "</VPHAL_SCENARIO>\n"));

    MOS_SecureStringPrint(sPath, MAX_PATH, MAX_PATH, "%s%cparam_dump_pid%x_ts%llx[%04d].xml", pParamsDumpSpec->outFileLocation, MOS_DIR_SEPERATOR, pid, timeStamp, uiFrameCounter);

    VpDumperTool::GetOsFilePath(sPath, sOsPath);

    VPHAL_DEBUG_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(sOsPath, pcOutContents, strlen(pcOutContents)));
finish:
    if (pcOutContents)
    {
        MOS_FreeMemory(pcOutContents);
        pcOutContents = nullptr;
    }
    return eStatus;
}

VpParameterDumper::VpParameterDumper(PMOS_INTERFACE pOsInterface):
    m_dumpSpec(),
    m_osInterface(pOsInterface)
{
    if (m_osInterface)
    {
        m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    }
}

VpParameterDumper::~VpParameterDumper()
{
}

const char * VpDumperTool::GetFormatStr(MOS_FORMAT format)
{
    VP_FUNC_CALL();

    switch (format)
    {
        case Format_A8R8G8B8    : return _T("argb");
        case Format_X8R8G8B8    : return _T("xrgb");
        case Format_A8B8G8R8    : return _T("abgr");
        case Format_X8B8G8R8    : return _T("xbgr");
        case Format_A16R16G16B16: return _T("argb16");
        case Format_A16B16G16R16: return _T("abgr16");
        case Format_R5G6B5      : return _T("rgb16");
        case Format_R8G8B8      : return _T("rgb");
        case Format_R32U        : return _T("r32u");
        case Format_RGBP        : return _T("rgbp");
        case Format_BGRP        : return _T("bgrp");
        case Format_R32F        : return _T("r32f");
        case Format_YUY2        : return _T("yuy2");
        case Format_YUYV        : return _T("yuyv");
        case Format_YVYU        : return _T("yvyu");
        case Format_UYVY        : return _T("uyvy");
        case Format_VYUY        : return _T("vyuy");
        case Format_Y416        : return _T("y416");
        case Format_AYUV        : return _T("ayuv");
        case Format_AUYV        : return _T("auyv");
        case Format_NV12        : return _T("nv12");
        case Format_NV11        : return _T("nv11");
        case Format_P208        : return _T("p208");
        case Format_IMC1        : return _T("imc1");
        case Format_IMC2        : return _T("imc2");
        case Format_IMC3        : return _T("imc3");
        case Format_IMC4        : return _T("imc4");
        case Format_422H        : return _T("422h");
        case Format_422V        : return _T("422v");
        case Format_444P        : return _T("444p");
        case Format_411P        : return _T("411p");
        case Format_411R        : return _T("411r");
        case Format_400P        : return _T("400p");
        case Format_I420        : return _T("i420");
        case Format_IYUV        : return _T("iyuv");
        case Format_YV12        : return _T("yv12");
        case Format_YVU9        : return _T("yvu9");
        case Format_AI44        : return _T("ai44");
        case Format_IA44        : return _T("ia44");
        case Format_P8          : return _T("P8");
        case Format_STMM        : return _T("stmm");
        case Format_Buffer      : return _T("buff");
        case Format_RAW         : return _T("buff");
        case Format_PAL         : return _T("pal");
        case Format_A8P8        : return _T("a8p8");
        case Format_A8          : return _T("a8");
        case Format_L8          : return _T("l8");
        case Format_A4L4        : return _T("a4l4");
        case Format_A8L8        : return _T("a8l8");
        case Format_V8U8        : return _T("v8u8");
        case Format_IRW0        : return _T("irw0");
        case Format_IRW1        : return _T("irw1");
        case Format_IRW2        : return _T("irw2");
        case Format_IRW3        : return _T("irw3");
        case Format_R10G10B10A2 : return _T("r10g10b10a2");
        case Format_B10G10R10A2 : return _T("b10g10r10a2");
        case Format_P010        : return _T("p010");
        case Format_P016        : return _T("p016");
        case Format_R16F        : return _T("r16f");
        case Format_Y210        : return _T("y210");
        case Format_Y216        : return _T("y216");
        case Format_Y410        : return _T("y410");
        case Format_P210        : return _T("p210");
        case Format_P216        : return _T("p216");
        default                 : return _T("Err");
    }

    return nullptr;
}

MOS_STATUS VpDumperTool::GetSurfaceSize(
    PVPHAL_SURFACE          pSurface,
    uint32_t                iBpp,
    uint32_t*               piWidthInBytes,
    uint32_t*               piHeightInRows)
{
    VP_FUNC_CALL();

    MOS_STATUS  eStatus;
    uint32_t    iWidthInBytes;
    uint32_t    iHeightInRows;

    //-------------------------------------------
    VPHAL_DEBUG_ASSERT(pSurface->dwWidth >= 1);
    VPHAL_DEBUG_ASSERT(pSurface->dwHeight >= 1);
    VPHAL_DEBUG_ASSERT(pSurface->dwPitch >= 1);
    //-------------------------------------------

    eStatus = MOS_STATUS_SUCCESS;

    switch (pSurface->Format)
    {
        // Packed Formats
        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
        case Format_R5G6B5:
        case Format_R8G8B8:
        case Format_R32U:
        case Format_R32F:
        case Format_AYUV:
        case Format_YUY2:
        case Format_YUYV:
        case Format_YVYU:
        case Format_UYVY:
        case Format_VYUY:
        case Format_AI44:
        case Format_IA44:
        case Format_P8:
        case Format_A8P8:
        case Format_A8:
        case Format_L8:
        case Format_A4L4:
        case Format_A8L8:
        case Format_V8U8:
        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
        case Format_Y410:
        case Format_Y416:
        case Format_Y210:
        case Format_Y216:
            iWidthInBytes = pSurface->dwWidth * iBpp / 8;
            iHeightInRows = pSurface->dwHeight;
            break;

        // 4:2:0 (12-bits per pixel)
        // IMC1                           // IMC3
        // ----------------->             // ----------------->
        // ________________________       // ________________________
        //|Y0|Y1|                  |      //|Y0|Y1|                  |
        //|__|__|                  |      //|__|__|                  |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|________________________|      //|________________________|
        //|V0|V1|      |           |      //|U0|U1|      |           |
        //|__|__|      |           |      //|__|__|      |           |
        //|            |           |      //|            |           |
        //|____________|  PAD      |      //|____________|  PAD      |
        //|U0|U1|      |           |      //|V0|V1|      |           |
        //|__|__|      |           |      //|__|__|      |           |
        //|            |           |      //|            |           |
        //|____________|___________|      //|____________|___________|
        case Format_IMC1:
        case Format_IMC3:
            iWidthInBytes = pSurface->dwWidth;
            iHeightInRows = pSurface->dwHeight * 2;
            break;

        // 4:0:0 (8-bits per pixel)
        // 400P
        // ----------------->
        // ________________________
        //|Y0|Y1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        case Format_400P:
        case Format_Buffer:
        case Format_RAW:
            iWidthInBytes = pSurface->dwWidth;
            iHeightInRows = pSurface->dwHeight;
            break;

        // 4:1:1 (12-bits per pixel)      // 4:2:2 (16-bits per pixel)
        // 411P                           // 422H
        // ----------------->             // ----------------->
        // ________________________       // ________________________
        //|Y0|Y1|                  |      //|Y0|Y1|                  |
        //|__|__|                  |      //|__|__|                  |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|                        |      //|                        |
        //|________________________|      //|________________________|
        //|U0|U1||                 |      //|U0|U1|      |           |
        //|__|__||                 |      //|__|__|      |           |
        //|      |                 |      //|            |           |
        //|      |      PAD        |      //|            |    PAD    |
        //|      |                 |      //|            |           |
        //|      |                 |      //|            |           |
        //|      |                 |      //|            |           |
        //|______|_________________|      //|____________|___________|
        //|V0|V1||                 |      //|V0|V1|      |           |
        //|__|__||                 |      //|__|__|      |           |
        //|      |                 |      //|            |           |
        //|      |      PAD        |      //|            |    PAD    |
        //|      |                 |      //|            |           |
        //|      |                 |      //|            |           |
        //|      |                 |      //|            |           |
        //|______|_________________|      //|____________|___________|

        // 4:4:4 (24-bits per pixel)
        // 444P
        // ----------------->
        // ________________________
        //|Y0|Y1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|U0|U1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|V0|V1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|

        // 4:4:4 (24-bits per pixel)
        // RGBP
        // ----------------->
        // ________________________
        //|R0|R1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|G0|G1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|B0|B1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        case Format_RGBP:

        // 4:4:4 (24-bits per pixel)
        // BGRP
        // ----------------->
        // ________________________
        //|B0|B1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|G0|G1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|R0|R1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        case Format_BGRP:
        case Format_411P:
        case Format_422H:
        case Format_444P:
            iWidthInBytes = pSurface->dwWidth;
            iHeightInRows = pSurface->dwHeight * 3;
            break;

        // 4:1:1 (12-bits per pixel)
        // 411R
        // ----------------->
        // ________________________
        //|Y0|Y1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|U0|U1|                  |
        //|__|__|                  |
        //|________________________|
        //|V0|V1|                  |
        //|__|__|                  |
        //|________________________|
        case Format_411R:
            iWidthInBytes = pSurface->dwWidth;
            iHeightInRows = (pSurface->dwHeight * 3) / 2;
            break;

        // 4:2:2V (16-bits per pixel)
        // 422V
        // ----------------->
        // ________________________
        //|Y0|Y1|                  |
        //|__|__|                  |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|                        |
        //|________________________|
        //|U0|U1|                  |
        //|__|__|                  |
        //|                        |
        //|________________________|
        //|V0|V1|                  |
        //|__|__|                  |
        //|                        |
        //|________________________|
        case Format_422V:
            iWidthInBytes = pSurface->dwWidth;
            iHeightInRows = pSurface->dwHeight * 2;
            break;

        // 4:2:0 (12-bits per pixel)
        // IMC2                          // IMC4
        // ----------------->            // ----------------->
        // ________________________      // ________________________
        //|Y0|Y1|                  |     //|Y0|Y1|                  |
        //|__|__|                  |     //|__|__|                  |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|________________________|     //|________________________|
        //|V0|V1|      |U0|U1|     |     //|U0|U1|      |V0|V1|     |
        //|__|__|      |__|__|     |     //|__|__|      |__|__|     |
        //|            |           |     //|            |           |
        //|____________|___________|     //|____________|___________|

        // NV12                          // YV12
        // ----------------->            // ----------------->
        // ________________________      // ________________________
        //|Y0|Y1|                  |     //|Y0|Y1|                  |
        //|__|__|                  |     //|__|__|                  |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|________________________|     //|________________________|
        //|U0|V0|U1|V1|            |     //|V0|V1|                  |
        //|__|__|__|__|            |     //|__|__|__________________|
        //|                        |     //|U0|U1|                  |
        //|________________________|     //|__|__|__________________|

        case Format_IMC2:
        case Format_IMC4:
        case Format_NV12:
        case Format_YV12:
        case Format_I420:
        case Format_IYUV:
        case Format_YVU9:
            iWidthInBytes = pSurface->dwWidth;
            iHeightInRows = pSurface->dwHeight * iBpp / 8;
            break;

        case Format_P010:
        case Format_P016:
            iWidthInBytes = pSurface->dwWidth * 2;
            iHeightInRows = pSurface->dwHeight * 3 / 2;
            break;

        case Format_A16R16G16B16:
        case Format_A16B16G16R16:
            iWidthInBytes = pSurface->dwWidth * 8;
            iHeightInRows = pSurface->dwHeight;
            break;

        case Format_P210:
        case Format_P216:
            iWidthInBytes = pSurface->dwWidth * 2;
            iHeightInRows = pSurface->dwHeight * 2;

        default:
            VPHAL_DEBUG_ASSERTMESSAGE("Format %d not supported.", pSurface->Format);
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
    }

    *piWidthInBytes = iWidthInBytes;
    *piHeightInRows = iHeightInRows;

finish:
    return eStatus;
}

MOS_STATUS VpDumperTool::AppendString(
    bool   bFirst,
    char*  *ppcBigString,
    PCCHAR pcToAppendFmt,
    ...)
{
    VP_FUNC_CALL();

    static size_t stTotalStrLen = 0;
    static size_t stTotalAlloc  = 0;

    MOS_STATUS  eStatus;
    size_t      stStrLenOld;
    size_t      stStrLenToAppend;
    char*       pcToAppend;
    char*       pcTmpPtr;
    va_list     argList;

    eStatus          = MOS_STATUS_SUCCESS;
    stStrLenToAppend = 0;
    stStrLenOld      = 0;
    pcTmpPtr         = nullptr;
    pcToAppend       = (char*)MOS_AllocAndZeroMemory(sizeof(char) * ALLOC_GRANULARITY);

    if (bFirst)
    {
        stTotalStrLen = 0;
        stTotalAlloc  = 0;
    }

    stStrLenOld = stTotalStrLen;

    if (ppcBigString == nullptr || pcToAppendFmt == nullptr || (int)strlen(pcToAppendFmt) < 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    va_start(argList, pcToAppendFmt);
    MosUtilities::MosSecureVStringPrint(pcToAppend, ALLOC_GRANULARITY, ALLOC_GRANULARITY, pcToAppendFmt, argList);
    va_end(argList);

    stStrLenToAppend = strlen(pcToAppend);

    if (*ppcBigString == nullptr)
    {
        stTotalStrLen = stStrLenToAppend + 1;
        stTotalAlloc  = MOS_ALIGN_CEIL(stStrLenToAppend, ALLOC_GRANULARITY);
        *ppcBigString = (char*)MOS_AllocAndZeroMemory(stTotalAlloc);
        VPHAL_DEBUG_CHK_NULL(*ppcBigString);
        *ppcBigString[0] = '\0';
        stStrLenOld++;
    }
    else
    {
        stTotalStrLen += stStrLenToAppend;
        if (stTotalStrLen >= stTotalAlloc)
        {
            stTotalAlloc = MOS_ALIGN_CEIL(stTotalStrLen, ALLOC_GRANULARITY);
            // Below should be equivalent to *ppcBigString = (char*)realloc(*ppcBigString, stTotalAlloc);
            pcTmpPtr = *ppcBigString;
            *ppcBigString = (char*)MOS_AllocAndZeroMemory(stTotalAlloc);
            VPHAL_DEBUG_CHK_NULL(*ppcBigString);
            MOS_SecureStringPrint(*ppcBigString, stTotalAlloc, stTotalAlloc, "%s", pcTmpPtr);
            MOS_FreeMemory(pcTmpPtr);
        }
    }
    MOS_SecureMemcpy(
        (char*)(((uintptr_t)(*ppcBigString)) + stStrLenOld - 1),
        stStrLenToAppend + 1,
        pcToAppend,
        stStrLenToAppend + 1);

finish:
    MOS_FreeMemory(pcToAppend);
    return eStatus;
}

void VpDumperTool::WriteFrame(
    PMOS_INTERFACE          pOsInterface,
    PVPHAL_SURFACE          pSurface,
    PCCHAR                  fileName,
    uint64_t                iCounter)
{
    VP_FUNC_CALL();

    uint8_t*                pData;
    char                    sPath[MAX_PATH];
    char                    sOsPath[MAX_PATH];
    uint8_t*                pDst;
    uint8_t*                pTmpDst;
    uint8_t*                pTmpSrc;
    uint32_t                iWidthInBytes;
    uint32_t                iHeightInRows;
    uint32_t                iBpp;
    uint32_t                iSize;
    uint32_t                iY;
    MOS_LOCK_PARAMS         LockFlags;

    pDst = nullptr;
    MOS_ZeroMemory(sPath, MAX_PATH);
    MOS_ZeroMemory(sOsPath, MAX_PATH);

    // get bits per pixel for the format
    pOsInterface->pfnGetBitsPerPixel(pOsInterface, pSurface->Format, &iBpp);

    VpDumperTool::GetSurfaceSize(
        pSurface,
        iBpp,
        &iWidthInBytes,
        &iHeightInRows);

    iSize = iWidthInBytes * iHeightInRows;

    // Write original image to file
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.ReadOnly = 1;

    pData = (uint8_t*)pOsInterface->pfnLockResource(
        pOsInterface,
        &pSurface->OsResource,
        &LockFlags);

    if(pData == nullptr)
    {
        VPHAL_DEBUG_ASSERTMESSAGE("pData == nullptr.");

        return;
    }
    MOS_SecureStringPrint(
        sPath,
        MAX_PATH,
        sizeof(sPath),
        "c:\\dump\\f[%08I64x]_%s_w[%d]_h[%d]_p[%d].%s",
        iCounter,
        fileName,
        pSurface->dwWidth,
        pSurface->dwHeight,
        pSurface->dwPitch,
        VpDumperTool::GetFormatStr(pSurface->Format));

    MOS_SecureMemcpy(sOsPath, MAX_PATH, sPath, strlen(sPath));

    // Write the data to file
    if (pSurface->dwPitch == iWidthInBytes)
    {
        MosUtilities::MosWriteFileFromPtr((const char *)sOsPath, pData, iSize);
    }
    else
    {
        pDst = (uint8_t*)MOS_AllocAndZeroMemory(iSize);
        pTmpSrc = pData;
        pTmpDst = pDst;

        for (iY = 0; iY < iHeightInRows; iY++)
        {
            MOS_SecureMemcpy(pTmpDst, iSize, pTmpSrc, iWidthInBytes);
            pTmpSrc += pSurface->dwPitch;
            pTmpDst += iWidthInBytes;
        }

        MosUtilities::MosWriteFileFromPtr((const char *)sOsPath, pDst, iSize);
    }

    if (pDst)
    {
        MOS_FreeMemory(pDst);
    }

    pOsInterface->pfnUnlockResource(
        pOsInterface,
        &pSurface->OsResource);
}

const char * VpParameterDumper::GetComponentStr(MOS_COMPONENT component)
{
    VP_FUNC_CALL();

    switch (component)
    {
    case COMPONENT_UNKNOWN:         return _T("COMPONENT_UNKNOWN");
    case COMPONENT_LibVA:           return _T("COMPONENT_LibVA");
    case COMPONENT_VPreP:           return _T("COMPONENT_VPreP");
    case COMPONENT_EMULATION:       return _T("COMPONENT_EMULATION");
    case COMPONENT_CM:              return _T("COMPONENT_CM");
    case COMPONENT_Encode:          return _T("COMPONENT_Encode");
    case COMPONENT_Decode:          return _T("COMPONENT_Decode");
    default:                        return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetWholeFormatStr(MOS_FORMAT format)
{
    VP_FUNC_CALL();

    switch (format)
    {
    case Format_Invalid:            return _T("Format_Invalid");
    case Format_Source:             return _T("Format_Source");
    case Format_420O:               return _T("Format_420O");
    case Format_RGB_Swap:           return _T("Format_RGB_Swap");
    case Format_RGB_No_Swap:        return _T("Format_RGB_No_Swap");
    case Format_RGB:                return _T("Format_RGB");
    case Format_RGB32:              return _T("Format_RGB32");
    case Format_PA:                 return _T("Format_PA");
    case Format_PL2:                return _T("Format_PL2");
    case Format_PL2_UnAligned:      return _T("Format_PL2_UnAligned");
    case Format_PL3:                return _T("Format_PL3");
    case Format_PL3_RGB:            return _T("Format_PL3_RGB");
    case Format_PAL:                return _T("Format_PA_T(");

    case Format_None:               return _T("Format_None");
    case Format_Any:                return _T("Format_Any");
    case Format_A8R8G8B8:           return _T("Format_A8R8G8B8");
    case Format_X8R8G8B8:           return _T("Format_X8R8G8B8");
    case Format_A8B8G8R8:           return _T("Format_A8B8G8R8");
    case Format_X8B8G8R8:           return _T("Format_X8B8G8R8");
    case Format_A16B16G16R16:       return _T("Format_A16B16G16R16");
    case Format_A16R16G16B16:       return _T("Format_A16R16G16B16");
    case Format_R5G6B5:             return _T("Format_R5G6B5");
    case Format_R8G8B8:             return _T("Format_R8G8B8");
    case Format_R32U:               return _T("Format_R32U");
    case Format_R32F:               return _T("Format_R32F");
    case Format_RGBP:               return _T("Format_RGBP");
    case Format_BGRP:               return _T("Format_BGRP");

    case Format_YUY2:               return _T("Format_YUY2");
    case Format_YUYV:               return _T("Format_YUYV");
    case Format_YVYU:               return _T("Format_YVYU");
    case Format_UYVY:               return _T("Format_UYVY");
    case Format_VYUY:               return _T("Format_VYUY");

    case Format_Y416:               return _T("Format_Y416");
    case Format_AYUV:               return _T("Format_AYUV");
    case Format_AUYV:               return _T("Format_AUYV");
    case Format_400P:               return _T("Format_400P");
    case Format_NV12:               return _T("Format_NV12");
    case Format_NV12_UnAligned:     return _T("Format_NV12_UnAligned");
    case Format_NV21:               return _T("Format_NV21");
    case Format_NV11:               return _T("Format_NV11");
    case Format_NV11_UnAligned:     return _T("Format_NV11_UnAligned");
    case Format_P208:               return _T("Format_P208");
    case Format_P208_UnAligned:     return _T("Format_P208_UnAligned");
    case Format_IMC1:               return _T("Format_IMC1");
    case Format_IMC2:               return _T("Format_IMC2");
    case Format_IMC3:               return _T("Format_IMC3");
    case Format_IMC4:               return _T("Format_IMC4");
    case Format_422H:               return _T("Format_422H");
    case Format_422V:               return _T("Format_422V");
    case Format_444P:               return _T("Format_444P");
    case Format_411P:               return _T("Format_411P");
    case Format_411R:               return _T("Format_411R");
    case Format_I420:               return _T("Format_I420");
    case Format_IYUV:               return _T("Format_IYUV");
    case Format_YV12:               return _T("Format_YV12");
    case Format_YVU9:               return _T("Format_YVU9");
    case Format_AI44:               return _T("Format_AI44");
    case Format_IA44:               return _T("Format_IA44");
    case Format_P8:                 return _T("Format_P8");
    case Format_A8P8:               return _T("Format_A8P8");
    case Format_A8:                 return _T("Format_A8");
    case Format_L8:                 return _T("Format_L8");
    case Format_A4L4:               return _T("Format_A4L4");
    case Format_A8L8:               return _T("Format_A8L8");
    case Format_IRW0:               return _T("Format_IRW0");
    case Format_IRW1:               return _T("Format_IRW1");
    case Format_IRW2:               return _T("Format_IRW2");
    case Format_IRW3:               return _T("Format_IRW3");
    case Format_IRW4:               return _T("Format_IRW4");
    case Format_IRW5:               return _T("Format_IRW5");
    case Format_IRW6:               return _T("Format_IRW6");
    case Format_IRW7:               return _T("Format_IRW7");
    case Format_STMM:               return _T("Format_STMM");
    case Format_Buffer:             return _T("Format_Buffer");
    case Format_Buffer_2D:          return _T("Format_Buffer_2D");
    case Format_V8U8:               return _T("Format_V8U8");
    case Format_R32S:               return _T("Format_R32S");
    case Format_R8U:                return _T("Format_R8U");
    case Format_R8G8UN:             return _T("Format_R8G8UN");
    case Format_R8G8SN:             return _T("Format_R8G8SN");
    case Format_G8R8_G8B8:          return _T("Format_G8R8_G8B8");
    case Format_R16U:               return _T("Format_R16U");
    case Format_R16S:               return _T("Format_R16S");
    case Format_R16UN:              return _T("Format_R16UN");
    case Format_RAW:                return _T("Format_RAW");
    case Format_Y8:                 return _T("Format_Y8");
    case Format_Y1:                 return _T("Format_Y1");
    case Format_Y16U:               return _T("Format_Y16U");
    case Format_Y16S:               return _T("Format_Y16S");
    case Format_L16:                return _T("Format_L16");
    case Format_D16:                return _T("Format_D16");
    case Format_R10G10B10A2:        return _T("Format_R10G10B10A2");
    case Format_B10G10R10A2:        return _T("Format_B10G10R10A2");
    case Format_P016:               return _T("Format_P016");
    case Format_P010:               return _T("Format_P010");
    case Format_Y210:               return _T("Format_Y210");
    case Format_Y216:               return _T("Format_Y216");
    case Format_Y410:               return _T("Format_Y410");
    case Format_P210:               return _T("Format_P210");
    case Format_P216:               return _T("Format_P216");
    case Format_YV12_Planar:        return _T("Format_YV12_Planar");
    case Format_Count:              return _T("Format_Count");
    default:                        return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetTileTypeStr(MOS_TILE_TYPE tile_type)
{
    VP_FUNC_CALL();

    switch (tile_type)
    {
    case MOS_TILE_X:            return _T("MOS_TILE_X");
    case MOS_TILE_Y:            return _T("MOS_TILE_Y");
    case MOS_TILE_LINEAR:       return _T("MOS_TILE_LINEAR");
    case MOS_TILE_INVALID:      return _T("MOS_TILE_INVALID");
    default:                    return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetSurfaceTypeStr(VPHAL_SURFACE_TYPE surface_type)
{
    VP_FUNC_CALL();

    switch (surface_type)
    {
    case SURF_NONE:             return _T("SURF_NONE");
    case SURF_IN_BACKGROUND:    return _T("SURF_IN_BACKGROUND");
    case SURF_IN_PRIMARY:       return _T("SURF_IN_PRIMARY");
    case SURF_IN_SUBSTREAM:     return _T("SURF_IN_SUBSTREAM");
    case SURF_IN_REFERENCE:     return _T("SURF_IN_REFERENCE");
    case SURF_OUT_RENDERTARGET: return _T("SURF_OUT_RENDERTARGET");
    case SURF_TYPE_COUNT:       return _T("SURF_TYPE_COUNT");
    default: return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetSampleTypeStr(VPHAL_SAMPLE_TYPE sample_type)
{
    VP_FUNC_CALL();

    switch (sample_type)
    {
    case SAMPLE_PROGRESSIVE:                                 return _T("SAMPLE_PROGRESSIVE");
    case SAMPLE_SINGLE_TOP_FIELD:                            return _T("SAMPLE_SINGLE_TOP_FIELD");
    case SAMPLE_SINGLE_BOTTOM_FIELD:                         return _T("SAMPLE_SINGLE_BOTTOM_FIELD");
    case SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD:            return _T("SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD");
    case SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD:         return _T("SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD");
    case SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD:             return _T("SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD");
    case SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD:          return _T("SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD");
    case SAMPLE_INVALID:                                     return _T("SAMPLE_INVALID");
    default:                                                 return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetColorSpaceStr(VPHAL_CSPACE color_space)
{
    VP_FUNC_CALL();

    switch (color_space)
    {
    case CSpace_None:                      return _T("CSpace_None");
    case CSpace_Source:                    return _T("CSpace_Source");
    case CSpace_RGB:                       return _T("CSpace_RGB");
    case CSpace_YUV:                       return _T("CSpace_YUV");
    case CSpace_Gray:                      return _T("CSpace_Gray");
    case CSpace_Any:                       return _T("CSpace_Any");
    case CSpace_sRGB:                      return _T("CSpace_sRGB");
    case CSpace_stRGB:                     return _T("CSpace_stRGB");
    case CSpace_BT601:                     return _T("CSpace_BT601");
    case CSpace_BT601_FullRange:           return _T("CSpace_BT601_FullRange");
    case CSpace_BT709:                     return _T("CSpace_BT709");
    case CSpace_BT709_FullRange:           return _T("CSpace_BT709_FullRange");
    case CSpace_xvYCC601:                  return _T("CSpace_xvYCC601");
    case CSpace_xvYCC709:                  return _T("CSpace_xvYCC709");
    case CSpace_BT601Gray:                 return _T("CSpace_BT601Gray");
    case CSpace_BT601Gray_FullRange:       return _T("CSpace_BT601Gray_FullRange");
    case CSpace_BT2020:                    return _T("CSpace_BT2020");
    case CSpace_BT2020_FullRange:          return _T("CSpace_BT2020_FullRange");
    case CSpace_BT2020_RGB:                return _T("CSpace_BT2020_RGB");
    case CSpace_BT2020_stRGB:              return _T("CSpace_BT2020_stRGB");
    case CSpace_Count:                     return _T("CSpace_Count");
    default:                               return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetBlendTypeStr(VPHAL_BLEND_TYPE blend_type)
{
    VP_FUNC_CALL();

    switch (blend_type)
    {
    case BLEND_NONE:               return _T("BLEND_NONE");
    case BLEND_SOURCE:             return _T("BLEND_SOURCE");
    case BLEND_PARTIAL:            return _T("BLEND_PARTIAL");
    case BLEND_CONSTANT:           return _T("BLEND_CONSTANT");
    case BLEND_CONSTANT_SOURCE:    return _T("BLEND_CONSTANT_SOURCE");
    case BLEND_CONSTANT_PARTIAL:   return _T("BLEND_CONSTANT_PARTIAL");
    default:                       return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetPaletteTypeStr(VPHAL_PALETTE_TYPE palette_type)
{
    VP_FUNC_CALL();

    switch (palette_type)
    {
    case VPHAL_PALETTE_NONE:          return _T("VPHAL_PALETTE_NONE");
    case VPHAL_PALETTE_YCbCr_8:       return _T("VPHAL_PALETTE_YCbCr_8");
    case VPHAL_PALETTE_ARGB_8:        return _T("VPHAL_PALETTE_ARGB_8");
    case VPHAL_PALETTE_AVYU_8:        return _T("VPHAL_PALETTE_AVYU_8");
    default:                          return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetScalingModeStr(VPHAL_SCALING_MODE scaling_mode)
{
    VP_FUNC_CALL();

    switch (scaling_mode)
    {
    case VPHAL_SCALING_NEAREST:         return _T("VPHAL_SCALING_NEAREST");
    case VPHAL_SCALING_BILINEAR:        return _T("VPHAL_SCALING_BILINEAR");
    case VPHAL_SCALING_AVS:             return _T("VPHAL_SCALING_AVS");
    default:                            return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetRotationModeStr(VPHAL_ROTATION rotation_mode)
{
    VP_FUNC_CALL();

    switch (rotation_mode)
    {
    case VPHAL_ROTATION_IDENTITY:               return _T("VPHAL_ROTATION_IDENTITY");
    case VPHAL_ROTATION_90:                     return _T("VPHAL_ROTATION_90");
    case VPHAL_ROTATION_180:                    return _T("VPHAL_ROTATION_180");
    case VPHAL_ROTATION_270:                    return _T("VPHAL_ROTATION_270");
    case VPHAL_MIRROR_HORIZONTAL:               return _T("VPHAL_MIRROR_HORIZONTAL");
    case VPHAL_MIRROR_VERTICAL:                 return _T("VPHAL_MIRROR_VERTICAL");
    case VPHAL_ROTATE_90_MIRROR_VERTICAL:       return _T("VPHAL_ROTATE_90_MIRROR_VERTICAL");
    case VPHAL_ROTATE_90_MIRROR_HORIZONTAL:     return _T("VPHAL_ROTATE_90_MIRROR_HORIZONTAL");

    default:                                    return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetDIModeStr(VPHAL_DI_MODE di_mode)
{
    VP_FUNC_CALL();

    switch (di_mode)
    {
    case DI_MODE_BOB:         return _T("DI_MODE_BOB");
    case DI_MODE_ADI:         return _T("DI_MODE_ADI");
    default:                  return _T("Err");
    }

    return nullptr;
}

const char * VpParameterDumper::GetDenoiseModeStr(VPHAL_NOISELEVEL noise_level)
{
    VP_FUNC_CALL();

    switch (noise_level)
    {
    case NOISELEVEL_DEFAULT:         return _T("NOISELEVEL_DEFAULT");
    case NOISELEVEL_VC1_HD:          return _T("NOISELEVEL_VC1_HD");
    default:                         return _T("Err");
    }

    return nullptr;
}

const char *VpParameterDumper::GetHVSDenoiseModeStr(VPHAL_HVSDN_MODE hvs_dn_mode)
{
    switch (hvs_dn_mode)
    {
    case HVSDENOISE_AUTO_BDRATE:
        return _T("HVSDENOISE_AUTO_BDRATE");
    case HVSDENOISE_AUTO_SUBJECTIVE:
        return _T("HVSDENOISE_AUTO_SUBJECTIVE");
    case HVSDENOISE_MANUAL:
        return _T("HVSDENOISE_MANUAL");
    default:
        return _T("Err");
    }

    return nullptr;
}
#endif // (_DEBUG || _RELEASE_INTERNAL)