/*
* Copyright (c) 2011-2019, Intel Corporation
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
//! \file      vphal_debug.c 
//! \brief 
//!
//!
//! \file     vphal_debug.c
//! \brief    Implementation of functions for debugging VPHAL
//! \details  This file contains the Implementation of functions for
//!           surface dumper, hw state dumper, perf counter dumper, and render
//!           parameter dumper
//!
#if (_DEBUG || _RELEASE_INTERNAL)
#include <stdio.h>
#include "vphal.h"
#include "mhw_vebox.h"
#include "mos_os.h"
#include "vphal_debug.h"

#include "vphal_render_vebox_base.h"

#include "renderhal_platform_interface.h"

#include "ctype.h"     // isupper, tolower, toupper

#define ALLOC_GRANULARITY                           5000000

//==<Dump Surface>==============================================================
#define VPHAL_DBG_SURF_DUMP_OUTFILE_KEY_NAME        "outfileLocation"
#define VPHAL_DBG_SURF_DUMP_LOCATION_KEY_NAME       "dumpLocations"
#define VPHAL_DBG_SURF_DUMP_START_FRAME_KEY_NAME    "startFrame"
#define VPHAL_DBG_SURF_DUMP_END_FRAME_KEY_NAME      "endFrame"
#define VPHAL_DBG_SURF_DUMP_MAX_DATA_LEN            200
#define VPHAL_DBG_SURF_DUMP_TYPE_BACKGROUND         "background"
#define VPHAL_DBG_SURF_DUMP_TYPE_PRIMARY            "primary"
#define VPHAL_DBG_SURF_DUMP_TYPE_SUBSTREAM          "substream"
#define VPHAL_DBG_SURF_DUMP_TYPE_REFERENCE          "reference"
#define VPHAL_DBG_SURF_DUMP_TYPE_RENDERTARGET       "rendertarget"
#define VPHAL_DBG_SURF_DUMP_LOC_PREALL              "preall"
#define VPHAL_DBG_SURF_DUMP_LOC_PREDNDI             "predndi"
#define VPHAL_DBG_SURF_DUMP_LOC_POSTDNDI            "postdndi"
#define VPHAL_DBG_SURF_DUMP_LOC_PRECOMP             "precomp"
#define VPHAL_DBG_SURF_DUMP_LOC_POSTCOMP            "postcomp"
#define VPHAL_DBG_SURF_DUMP_LOC_PREMEMDECOMP        "prememdecomp"
#define VPHAL_DBG_SURF_DUMP_LOC_POSTMEMDECOMP       "postmemdecomp"
#define VPHAL_DBG_SURF_DUMP_LOC_POSTALL             "postall"

#define VPHAL_DBG_STATE_DUMP_OUTFILE_KEY_NAME        "outfileLocation"
#define VPHAL_DBG_STATE_DUMP_LOCATION_KEY_NAME       "dumpLocations"
#define VPHAL_DBG_STATE_DUMP_START_FRAME_KEY_NAME    "startFrame"
#define VPHAL_DBG_STATE_DUMP_END_FRAME_KEY_NAME      "endFrame"

#define VPHAL_DBG_STATE_DUMP_LOC_DNDI                "dndi"
#define VPHAL_DBG_STATE_DUMP_LOC_VEBOX               "vebox"
#define VPHAL_DBG_STATE_DUMP_LOC_COMP                "comp"

#define VPHAL_DBG_GSH_DUMP_TYPE                      "GSH"
#define VPHAL_DBG_SSH_DUMP_TYPE                      "SSH"
#define VPHAL_DBG_BB_DUMP_TYPE                       "BB"
#define VPHAL_DBG_CB_DUMP_TYPE                       "CB"
#define VPHAL_DBG_VEBOXSTATE_DUMP_TYPE               "VeboxState"
#define VPHAL_DBG_STATISTICS_DUMP_TYPE               "Statistics"

#define ALLOCLIST_SIZE                              15
#define VPHAL_VEBOX_STATISTICS_SIZE_MAX             (288 * 4)        //!< Max Statistics size
#define MAX_DW_STRLEN                               7
#define KERNEL_FIELD_NAME                           "Kernel"
#define VPDEBUG_VERSION                             "1.0"
#define XMLHEADER                                   "<?xml version=\"1.0\"?>\r\n"

//==<Dump Render Parameters>====================================================
#define VPHAL_DBG_PARAMS_DUMP_OUTFILE_KEY_NAME      "outxmlLocation"
#define VPHAL_DBG_PARAMS_DUMP_START_FRAME_KEY_NAME  "startxmlFrame"
#define VPHAL_DBG_PARAMS_DUMP_END_FRAME_KEY_NAME    "endxmlFrame"

void VphalDumperTool::GetOsFilePath(
    const char* pcFilePath,
    char*       pOsFilePath)
{
    MOS_SecureMemcpy(pOsFilePath, MAX_PATH, (void*)pcFilePath, strlen(pcFilePath));
}

MOS_STATUS VphalSurfaceDumper::GetPlaneDefs(
    PVPHAL_SURFACE                    pSurface,
    VPHAL_DBG_SURF_DUMP_SURFACE_DEF   *pPlanes,
    uint32_t                          *pdwNumPlanes,
    uint32_t                          *pdwSize,
    bool                              auxEnable,
    bool                              isDeswizzled)
{
    MOS_STATUS      eStatus;
    uint32_t        i;
    bool            PaddingEnable = false;

    eStatus = MOS_STATUS_SUCCESS;

    // Caller should supply this much!
    MOS_ZeroMemory(pPlanes, sizeof(VPHAL_DBG_SURF_DUMP_SURFACE_DEF) * 3);

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
    case Format_R8G8B8:
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
        VPHAL_DEBUG_ASSERTMESSAGE("Format '%d' not supported.", pSurface->Format);
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
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

finish:
    return eStatus;
}

bool VphalSurfaceDumper::HasAuxSurf(
    PMOS_RESOURCE    osResource)
{
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

MOS_STATUS VphalSurfaceDumper::DumpSurfaceToFile(
    PMOS_INTERFACE          pOsInterface,
    PVPHAL_SURFACE          pSurface,
    const char              *psPathPrefix,
    uint64_t                iCounter,
    bool                    bLockSurface,
    bool                    bNoDecompWhenLock,
    uint8_t*                pData)
{
    MOS_STATUS                          eStatus;
    bool                                isSurfaceLocked;
    char                                sPath[MAX_PATH], sOsPath[MAX_PATH];
    uint8_t                             *pDst, *pTmpSrc, *pTmpDst;
    uint32_t                            dwNumPlanes, dwSize, j, i;
    VPHAL_DBG_SURF_DUMP_SURFACE_DEF     planes[3];
    uint32_t                            dstPlaneOffset[3] = {0};
    MOS_LOCK_PARAMS                     LockFlags;
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData;
    bool                                hasAuxSurf;
    bool                                enableAuxDump;
    bool                                enablePlaneDump = false;

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

        pData = (uint8_t*)pOsInterface->pfnLockResource(
            pOsInterface,
            &pSurface->OsResource,
            &LockFlags);
        VPHAL_DEBUG_CHK_NULL(pData);

        // Write error to user feauture key
        MOS_ZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
        UserFeatureWriteData.Value.u32Data  = 1;
        UserFeatureWriteData.ValueID        = __VPHAL_DBG_SURF_DUMPER_RESOURCE_LOCK_ID;

        eStatus = MOS_UserFeature_WriteValues_ID(
            nullptr,
            &UserFeatureWriteData,
            1);

        VPHAL_DEBUG_ASSERT(eStatus == MOS_STATUS_SUCCESS);
        isSurfaceLocked = true;
    }

    MOS_SecureStringPrint(
        sPath,
        MAX_PATH,
        sizeof(sPath),
        "%s_f[%03lld]_w[%d]_h[%d]_p[%d].%s",
        psPathPrefix,
        iCounter,
        pSurface->dwWidth,
        pSurface->dwHeight,
        pSurface->dwPitch,
        VphalDumperTool::GetFormatStr(pSurface->Format));

    VphalDumperTool::GetOsFilePath(sPath, sOsPath);

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
                    "%s_f[%03lld]_w[%d]_h[%d]_p[%d].%s",
                    psPathPrefix,
                    iCounter,
                    planes[j].dwWidth,
                    planes[j].dwHeight,
                    planes[j].dwPitch,
                    (j == 0 ? "Y" : ((dwNumPlanes == 2) ? "UV" : ((j == 1) ? "U" : "V"))));

                VphalDumperTool::GetOsFilePath(sPlanePath, sPlaneOsPath);

                VPHAL_DEBUG_CHK_STATUS(MOS_WriteFileFromPtr(sPlaneOsPath, pDst + dstPlaneOffset[j], dstPlaneOffset[j + 1]));
            }
            else
            {
                VPHAL_DEBUG_ASSERTMESSAGE("More than 3 planes not supported during plane dump.");
            }
        }
    }

    VPHAL_DEBUG_CHK_STATUS(MOS_WriteFileFromPtr(sOsPath, pDst, dwSize));

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
            "%s_f[%03lld]_w[%d]_h[%d]_p[%d].Yaux",
            psPathPrefix,
            iCounter,
            pSurface->dwWidth,
            pSurface->dwHeight,
            pSurface->dwPitch);

        VphalDumperTool::GetOsFilePath(sPath, sOsPath);

        uint8_t  *pDstAux = (uint8_t*)MOS_AllocAndZeroMemory(auxSizeY);
        VPHAL_DEBUG_CHK_NULL(pDstAux);

        MOS_SecureMemcpy(
            pDstAux,
            auxSizeY,
            auxDataY,
            auxSizeY);

        VPHAL_DEBUG_CHK_STATUS(MOS_WriteFileFromPtr(sOsPath, pDstAux, auxSizeY));
        MOS_SafeFreeMemory(pDstAux);

        if (auxSizeUV && isPlanar)
        {
            // Dump UV Aux data
            MOS_SecureStringPrint(
                sPath,
                MAX_PATH,
                sizeof(sPath),
                "%s_f[%03lld]_w[%d]_h[%d]_p[%d].UVaux",
                psPathPrefix,
                iCounter,
                pSurface->dwWidth,
                pSurface->dwHeight,
                pSurface->dwPitch);

            VphalDumperTool::GetOsFilePath(sPath, sOsPath);

            uint8_t  *pDstUVAux = (uint8_t*)MOS_AllocAndZeroMemory(auxSizeUV);
            VPHAL_DEBUG_CHK_NULL(pDstUVAux);

            MOS_SecureMemcpy(
                pDstUVAux,
                auxSizeUV,
                auxDataUV,
                auxSizeUV);

            VPHAL_DEBUG_CHK_STATUS(MOS_WriteFileFromPtr(sOsPath, pDstUVAux, auxSizeUV));
            MOS_SafeFreeMemory(pDstUVAux);
        }
    }
#endif

finish:
    MOS_SafeFreeMemory(pDst);

    if (isSurfaceLocked)
    {
        eStatus = (MOS_STATUS)pOsInterface->pfnUnlockResource(pOsInterface, &pSurface->OsResource);
        VPHAL_DEBUG_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    }

    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DefGsh(
    VPHAL_DBG_FIELD_LAYOUT*  *ppGshLayout,
    uint32_t*                puiNumGSHFields)
{
    MOS_STATUS                      eStatus;
    PRENDERHAL_STATE_HEAP           pStateHeap;
    PRENDERHAL_STATE_HEAP_SETTINGS  pStateHeapSettings;
    uint32_t                        i;
    uint32_t                        j;

    // MEDIA STATE LAYOUT
    VPHAL_DBG_FIELD_LAYOUT*         pmediaStateLayout;
    uint32_t                        uiNumMediaStateFields;

    // GSH SAMPLER STATE LAYOUT
    VPHAL_DBG_FIELD_LAYOUT*         psamplerStateLayout;
    uint32_t                        uiNumSamplerStateFields;

    eStatus             = MOS_STATUS_SUCCESS;
    pStateHeap          = m_stateHeap;
    pStateHeapSettings  = m_stateHeapSettings;

    // GSH LAYOUT
    *puiNumGSHFields = 4;
    (*ppGshLayout) = (VPHAL_DBG_FIELD_LAYOUT*)MOS_AllocAndZeroMemory(
                        sizeof(VPHAL_DBG_FIELD_LAYOUT) *
                                                            (*puiNumGSHFields));
    VPHAL_DEBUG_CHK_NULL((*ppGshLayout));
    i = 0;

    MOS_SecureStringPrint((*ppGshLayout)[i].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "Sync/Perf Tags (SY)");
    (*ppGshLayout)[i].dwOffset      = pStateHeap->dwOffsetSync;
    (*ppGshLayout)[i].dwSize        = pStateHeap->dwSizeSync;
    (*ppGshLayout)[i].uiNumber      = 1;
    (*ppGshLayout)[i].pChildLayout  = nullptr;
    (*ppGshLayout)[i].uiNumChildren = 0;
    MOS_SecureStringPrint((*ppGshLayout)[i++].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "");

    MOS_SecureStringPrint((*ppGshLayout)[i].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "Media State");
    (*ppGshLayout)[i].dwOffset      = pStateHeap->pMediaStates->dwOffset;
    (*ppGshLayout)[i].dwSize        = pStateHeap->dwOffsetMediaID +
                                      pStateHeapSettings->iMediaIDs *
                                      pStateHeap->dwSizeMediaID;
    (*ppGshLayout)[i].uiNumber      = pStateHeapSettings->iMediaStateHeaps;
    // MEDIA STATE LAYOUT -- Indented to make hierarchy clear
    {
        uiNumMediaStateFields = 6;
        pmediaStateLayout = (VPHAL_DBG_FIELD_LAYOUT*)MOS_AllocAndZeroMemory(
                                sizeof(VPHAL_DBG_FIELD_LAYOUT) *
                                                   uiNumMediaStateFields);
        VPHAL_DEBUG_CHK_NULL(pmediaStateLayout);
        j = 0;

        MOS_SecureStringPrint(pmediaStateLayout[j].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "CURBE Data");
        pmediaStateLayout[j].dwOffset      = pStateHeap->dwOffsetCurbe;
        pmediaStateLayout[j].dwSize        = pStateHeap->dwSizeCurbe;
        pmediaStateLayout[j].uiNumber      = 1;
        pmediaStateLayout[j].pChildLayout  = nullptr;
        pmediaStateLayout[j].uiNumChildren = 0;
        MOS_SecureStringPrint(pmediaStateLayout[j++].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "");

        MOS_SecureStringPrint(pmediaStateLayout[j].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "Sampler States for MID");
        pmediaStateLayout[j].dwOffset      = pStateHeap->dwOffsetSampler;
        pmediaStateLayout[j].dwSize        = m_hwSizes->dwSizeSamplerState *
                                             pStateHeapSettings->iSamplers;
        pmediaStateLayout[j].uiNumber      = pStateHeapSettings->iMediaIDs;
        // GSH SAMPLER STATE LAYOUT -- Indented to make hierarchy clear
        {
            uiNumSamplerStateFields = 1;
            psamplerStateLayout = (VPHAL_DBG_FIELD_LAYOUT*)MOS_AllocAndZeroMemory(
                               sizeof(VPHAL_DBG_FIELD_LAYOUT) * uiNumSamplerStateFields);
            VPHAL_DEBUG_CHK_NULL(psamplerStateLayout);

            MOS_SecureStringPrint(psamplerStateLayout[0].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "Sampler State");
            psamplerStateLayout[0].dwOffset      = 0;
            psamplerStateLayout[0].dwSize        = m_hwSizes->dwSizeSamplerState;
            psamplerStateLayout[0].uiNumber      = pStateHeapSettings->iSamplers;
            psamplerStateLayout[0].pChildLayout  = nullptr;
            psamplerStateLayout[0].uiNumChildren = 0;
            MOS_SecureStringPrint(psamplerStateLayout[0].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "_SAMPLER_STATE_");
        }

        pmediaStateLayout[j].pChildLayout  = psamplerStateLayout;
        pmediaStateLayout[j].uiNumChildren = uiNumSamplerStateFields;
        MOS_SecureStringPrint(pmediaStateLayout[j++].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "");

        MOS_SecureStringPrint(pmediaStateLayout[j].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "Sampler 8x8 Table");
        pmediaStateLayout[j].dwOffset      = pStateHeap->dwOffsetSamplerAVS;
        pmediaStateLayout[j].dwSize        = pStateHeap->dwSizeSamplerAVS;
        pmediaStateLayout[j].uiNumber      = pStateHeapSettings->iSamplersAVS;
        pmediaStateLayout[j].pChildLayout  = nullptr;
        pmediaStateLayout[j].uiNumChildren = 0;
        MOS_SecureStringPrint(pmediaStateLayout[j++].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "_SAMPLER_8x8_STATE_TABLE_");

        MOS_SecureStringPrint(pmediaStateLayout[j].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "Media ID");
        pmediaStateLayout[j].dwOffset      = pStateHeap->dwOffsetMediaID;
        pmediaStateLayout[j].dwSize        = pStateHeap->dwSizeMediaID;
        pmediaStateLayout[j].uiNumber      = pStateHeapSettings->iMediaIDs;
        pmediaStateLayout[j].pChildLayout  = nullptr;
        pmediaStateLayout[j].uiNumChildren = 0;
        MOS_SecureStringPrint(pmediaStateLayout[j++].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "_INTERFACE_DESCRIPTOR_DATA_");
    }

    (*ppGshLayout)[i].pChildLayout  = pmediaStateLayout;
    (*ppGshLayout)[i].uiNumChildren = uiNumMediaStateFields;
    MOS_SecureStringPrint((*ppGshLayout)[i++].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "");

    MOS_SecureStringPrint((*ppGshLayout)[i].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "Kernel");
    (*ppGshLayout)[i].dwOffset      = pStateHeap->dwKernelBase;
    (*ppGshLayout)[i].dwSize        = pStateHeap->iKernelSize / pStateHeapSettings->iKernelCount;
    (*ppGshLayout)[i].uiNumber      = pStateHeapSettings->iKernelCount;
    (*ppGshLayout)[i].pChildLayout  = nullptr;
    (*ppGshLayout)[i].uiNumChildren = 0;
    MOS_SecureStringPrint((*ppGshLayout)[i++].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "");

finish:
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DefSsh(
    VPHAL_DBG_FIELD_LAYOUT*  *ppSshLayout,
    uint32_t*                puiNumSSHFields)
{
    MOS_STATUS                      eStatus;
    PRENDERHAL_STATE_HEAP           pStateHeap;
    PRENDERHAL_STATE_HEAP_SETTINGS  pStateHeapSettings;
    uint32_t                        i;
    int32_t                         k;

    // BINDING TABLE LAYOUT
    VPHAL_DBG_FIELD_LAYOUT*         pbtLayout;
    uint32_t                        uiNumBTFields;

    eStatus   = MOS_STATUS_SUCCESS;
    pStateHeap          = m_stateHeap;
    pStateHeapSettings  = m_stateHeapSettings;

    // SSH LAYOUT
    (*puiNumSSHFields) = 1 + pStateHeapSettings->iSurfaceStates;
    (*ppSshLayout) = (VPHAL_DBG_FIELD_LAYOUT*)MOS_AllocAndZeroMemory(sizeof(VPHAL_DBG_FIELD_LAYOUT) *
                                                            (*puiNumSSHFields));
    VPHAL_DEBUG_CHK_NULL((*ppSshLayout));
    i = 0;

    MOS_SecureStringPrint((*ppSshLayout)[i].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "Binding Table");
    (*ppSshLayout)[i].dwOffset      = pStateHeap->iBindingTableOffset;
    (*ppSshLayout)[i].dwSize        = pStateHeap->iBindingTableSize;
    (*ppSshLayout)[i].uiNumber      = pStateHeapSettings->iBindingTables;
    // BINDING TABLE LAYOUT -- Indented to make hierarchy clear
    {
        pbtLayout = (VPHAL_DBG_FIELD_LAYOUT*)MOS_AllocAndZeroMemory(sizeof(VPHAL_DBG_FIELD_LAYOUT) * 1);
        VPHAL_DEBUG_CHK_NULL(pbtLayout);
        uiNumBTFields = 1;
        MOS_SecureStringPrint(pbtLayout[0].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "Binding Index");
        pbtLayout[0].dwOffset      = pStateHeap->iBindingTableOffset;
        pbtLayout[0].dwSize        = pStateHeap->iBindingTableSize / pStateHeapSettings->iSurfacesPerBT;
        pbtLayout[0].uiNumber      = pStateHeapSettings->iSurfacesPerBT;
        pbtLayout[0].pChildLayout  = nullptr;
        pbtLayout[0].uiNumChildren = 0;
        MOS_SecureStringPrint(pbtLayout[0].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "_BINDING_TABLE_STATE_");
    }

    (*ppSshLayout)[i].pChildLayout  = pbtLayout;
    (*ppSshLayout)[i].uiNumChildren = 1;
    MOS_SecureStringPrint((*ppSshLayout)[i++].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "");

    for (k = 0; k < pStateHeapSettings->iSurfaceStates; k++)
    {// Not indented to make hierarchy levels clear
    MOS_SecureStringPrint((*ppSshLayout)[i+k].pcName, MAX_NAME_LEN, MAX_NAME_LEN, "Surface State %d", k);
    (*ppSshLayout)[i+k].dwOffset      = pStateHeap->iSurfaceStateOffset +
                                        m_renderHal->pRenderHalPltInterface->GetSurfaceStateCmdSize() * k;
    (*ppSshLayout)[i+k].dwSize        = m_renderHal->pRenderHalPltInterface->GetSurfaceStateCmdSize();
    (*ppSshLayout)[i+k].uiNumber      = 1;
    (*ppSshLayout)[i+k].pChildLayout  = nullptr;
    (*ppSshLayout)[i+k].uiNumChildren = 0;

    switch(pStateHeap->pSurfaceEntry[k].Type)
    {
        case RENDERHAL_SURFACE_TYPE_G8:
            MOS_SecureStringPrint((*ppSshLayout)[i+k].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "_SURFACE_STATE_G8");
            break;
        case RENDERHAL_SURFACE_TYPE_G9:
            MOS_SecureStringPrint((*ppSshLayout)[i+k].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "_SURFACE_STATE_G9");
            break;
        case RENDERHAL_SURFACE_TYPE_G10:
            MOS_SecureStringPrint((*ppSshLayout)[i+k].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "_SURFACE_STATE_G10");
            break;
        case RENDERHAL_SURFACE_TYPE_ADV_G8:
            MOS_SecureStringPrint((*ppSshLayout)[i+k].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "_SURFACE_STATE_ADV_G8");
            break;
        case RENDERHAL_SURFACE_TYPE_ADV_G9:
            MOS_SecureStringPrint((*ppSshLayout)[i+k].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "_SURFACE_STATE_ADV_G9");
            break;
        case RENDERHAL_SURFACE_TYPE_ADV_G10:
            MOS_SecureStringPrint((*ppSshLayout)[i+k].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "_SURFACE_STATE_ADV_G10");
            break;
        case RENDERHAL_SURFACE_TYPE_INVALID:
            MOS_SecureStringPrint((*ppSshLayout)[i+k].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "No surface");
            break;
        default:
            // When type is zero, its un-initialized, so just show string of DW
            VPHAL_DEBUG_ASSERT(0);
            MOS_SecureStringPrint((*ppSshLayout)[i+k].pcStructName, MAX_NAME_LEN, MAX_NAME_LEN, "");
            break;
    } // Not indented to make hierarchy clear
    }

finish:
    return eStatus;
}

void VphalHwStateDumper::FreeLayout(
    VPHAL_DBG_FIELD_LAYOUT   *pLayout,
    uint32_t                 uiNumFields)
{
    uint32_t    i;

    for (i = 0; i < uiNumFields; i++)
    {
        if (pLayout[i].pChildLayout != nullptr)
        {
            FreeLayout(pLayout[i].pChildLayout, pLayout[i].uiNumChildren);
        }
    }
    MOS_SafeFreeMemory(pLayout);
}

MOS_STATUS VphalHwStateDumper::DumpXmlFieldHeader(
    char        **ppcOutContents,
    char        *pcName,
    uint32_t    ulLoc,
    uint32_t    ulSize,
    bool        bIsByte,
    bool        bHasCont,
    bool        bStruct)
{
    MOS_STATUS eStatus;

    MOS_UNUSED(bHasCont);

    eStatus = MOS_STATUS_SUCCESS;

    if (!ppcOutContents)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, ppcOutContents,
        "<FIELD%s><NAME>%s</NAME><LOC>0x%x</LOC><SIZE unit=\"%s\">0x%x</SIZE>\r\n",
        bStruct ? " struct=\"TRUE\"" : "", pcName, ulLoc,
        bIsByte ? "byte" : "bit", ulSize));

finish:
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpXmlFieldFooter(
    char   **ppcOutContents,
    bool   bHasCont)
{
    MOS_STATUS eStatus;

    MOS_UNUSED(bHasCont);

    eStatus = MOS_STATUS_SUCCESS;

    if (!ppcOutContents)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, ppcOutContents, "</FIELD>\r\n"));

finish:
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpDwords(
    char      **ppcOutContents,
    uint32_t  dwSizeField)
{
    MOS_STATUS  eStatus;
    char*       pcName;
    int32_t     iNumFields;
    int32_t     iDWSize;
    int32_t     iOffset;
    int32_t     i;

    eStatus    = MOS_STATUS_SUCCESS;
    pcName     = nullptr;
    iNumFields = 0;
    iDWSize    = 0;
    iOffset    = 0;
    i          = 0;

    if (!ppcOutContents)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    VPHAL_DEBUG_ASSERT(dwSizeField % sizeof(uint32_t) == 0);

    pcName = (char*)MOS_AllocAndZeroMemory(MAX_DW_STRLEN);
    VPHAL_DEBUG_CHK_NULL(pcName);

    iNumFields = MOS_BYTES_TO_DWORDS(dwSizeField);//Change macro to >> 2 if optimizer doesn't
    iDWSize    = sizeof(uint32_t);

    for (i = 0; i < iNumFields; i++)
    {
        MOS_SecureStringPrint(pcName, MAX_DW_STRLEN, MAX_DW_STRLEN, "DW%d", i);
        VPHAL_DEBUG_CHK_STATUS(DumpXmlFieldHeader(ppcOutContents, pcName, iOffset,
                                               iDWSize, true, false, false));
        VPHAL_DEBUG_CHK_STATUS(DumpXmlFieldFooter(ppcOutContents, false));;
        iOffset += iDWSize;
    }

finish:
    MOS_SafeFreeMemory(pcName);
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpFieldHeap(
    char                     **ppcOutContents,
    VPHAL_DBG_FIELD_LAYOUT   *parLayout)
{
    MOS_STATUS  eStatus;
    char*       pcName;
    int32_t     iOffset;
    uint32_t    i;

    eStatus = MOS_STATUS_SUCCESS;
    pcName  = nullptr;
    iOffset = 0;
    i       = 0;

    if (!ppcOutContents || !parLayout)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    pcName = (char*)MOS_AllocAndZeroMemory(MAX_NAME_LEN);
    VPHAL_DEBUG_CHK_NULL(pcName);

    // For each instance of this field type
    for (i = 0; i < parLayout->uiNumber; i++)
    {
        MOS_SecureStringPrint(pcName, MAX_NAME_LEN, MAX_NAME_LEN, "%s %d", parLayout->pcName, i);
        VPHAL_DEBUG_CHK_STATUS(DumpXmlFieldHeader(ppcOutContents, pcName, iOffset,
                                               parLayout->dwSize, true, true,
                                               false));
        VPHAL_DEBUG_CHK_STATUS(DumpSubfields(ppcOutContents, parLayout, parLayout->pChildLayout,
                                        parLayout->uiNumChildren));
        VPHAL_DEBUG_CHK_STATUS(DumpXmlFieldFooter(ppcOutContents, true));
        iOffset += parLayout->dwSize;
    }

finish:
    MOS_SafeFreeMemory(pcName);
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpSubfields(
    char                     **ppcOutContents,
    VPHAL_DBG_FIELD_LAYOUT*  pParLayout,
    VPHAL_DBG_FIELD_LAYOUT*  pChildLayout,
    uint32_t                 uiNumChild)
{
    MOS_STATUS  eStatus;
    uint32_t    i;

    eStatus = MOS_STATUS_SUCCESS;
    i       = 0;

    if (!ppcOutContents || (!pParLayout && uiNumChild == 0) ||
        (!pChildLayout && uiNumChild > 0))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    if (uiNumChild > 0)
    {
        for (i = 0; i < uiNumChild; i++)
        {
            if (pChildLayout[i].uiNumber > 0)
            {
                VPHAL_DEBUG_CHK_STATUS(DumpField(ppcOutContents, &(pChildLayout[i])));
            }// end if exists
        }// end for each field
    }// if has fields
    else if (pParLayout->pcStructName[0] != '\0')
    {
        // second last param is not zero in such cases as 8x8 table
        // (arrays, multi structs) -- this is now handled by indirect recursion

        VPHAL_DEBUG_CHK_STATUS(DumpXmlFieldHeader(ppcOutContents, pParLayout->pcStructName,
                                               0, pParLayout->dwSize, true,
                                               false, true));
        VPHAL_DEBUG_CHK_STATUS(DumpXmlFieldFooter(ppcOutContents, false));
    }
    else
    {
        // Skip uint32_t dumping for kernel section
        if (strcmp(pParLayout->pcName, KERNEL_FIELD_NAME) != 0)
        {
            VPHAL_DEBUG_CHK_STATUS(DumpDwords(ppcOutContents, pParLayout->dwSize));
        }
    }

finish:
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpField(
    char                     **ppcOutContents,
    VPHAL_DBG_FIELD_LAYOUT*  pLayout)
{
    MOS_STATUS  eStatus;
    char*       pcName;

    eStatus = MOS_STATUS_SUCCESS;
    pcName  = nullptr;

    if (!ppcOutContents || !pLayout)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    // Header
    pcName = (char*)MOS_AllocAndZeroMemory(MAX_NAME_LEN);
    VPHAL_DEBUG_CHK_NULL(pcName);

    if (pLayout->uiNumber > 1)
    {
        MOS_SecureStringPrint(pcName, MAX_NAME_LEN, MAX_NAME_LEN, "%s Heap", pLayout->pcName);
    }
    else
    {
        MOS_SecureStringPrint(pcName, MAX_NAME_LEN, MAX_NAME_LEN, "%s", pLayout->pcName);
    }
    VPHAL_DEBUG_CHK_STATUS(DumpXmlFieldHeader(ppcOutContents, pcName, pLayout->dwOffset,
                                           pLayout->dwSize * pLayout->uiNumber,
                                           true, true, false));

    // Contents
    if (pLayout->uiNumber > 1)
    {
        VPHAL_DEBUG_CHK_STATUS(DumpFieldHeap(ppcOutContents, pLayout));
    }
    else
    {
        VPHAL_DEBUG_CHK_STATUS(DumpSubfields(ppcOutContents, pLayout, pLayout->pChildLayout,
                                        pLayout->uiNumChildren));
    }

    // footer
    VPHAL_DEBUG_CHK_STATUS(DumpXmlFieldFooter(ppcOutContents, true));

finish:
    return eStatus;
}

void VphalHwStateDumper::GetLastFieldIndex(
    VPHAL_DBG_FIELD_LAYOUT  fl[],
    uint32_t                uiNumFields,
    uint32_t                *puiLastIndex)
{
    uint32_t    i;

    i  = 0;

    if (uiNumFields < 1)
    {
        puiLastIndex = nullptr;
    }
    else
    {
        *puiLastIndex = 0;
        for (i = 1; i < uiNumFields; i++)
        {
            if (fl[i].dwOffset > fl[*puiLastIndex].dwOffset)
            {
                *puiLastIndex = i;
            }
        }
    }
}

MOS_STATUS VphalHwStateDumper::CheckPath(
    char* *ppcPath)
{
    MOS_STATUS  eStatus;
    size_t      iStrLen;
    char*       pcTmpPtr;

    eStatus  = MOS_STATUS_SUCCESS;
    iStrLen  = 0;
    pcTmpPtr = nullptr;

    if (!ppcPath || !*ppcPath)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    iStrLen = strlen(*ppcPath);
    if (iStrLen > 1)
    {
        if (((*ppcPath)[iStrLen - 1] == '\\') || ((*ppcPath)[iStrLen - 1] == '/'))
        {
            (*ppcPath)[iStrLen - 1] = '\0';
        }
    }

finish:
    return eStatus;
}

char* VphalSurfaceDumper::WhitespaceTrim(
    char*   ptr)
{
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

//------------------------------------------------------------------------------
//
//    TOP LEVEL FUNCTIONS
// These are for dumping structures and binary data.
// Many of these will be called directly from the driver.
//
//------------------------------------------------------------------------------

MOS_STATUS VphalHwStateDumper::DumpBinaryStruct(
    VPHAL_DBG_FIELD_LAYOUT*  fields,
    uint32_t                 uiNumFields,
    void                     *pvStructToDump,
    char*                    pcPath,
    PCCHAR                   pcName,
    int32_t                  iID)
{
    MOS_STATUS  eStatus;
    uint32_t    lLastFieldOffset;
    uint32_t    lLastFieldSize;
    uint32_t    lSize;
    uint32_t    uiLastIndex;
    int32_t     iStrLenPath;
    int32_t     iStrLenName;
    char*       pcOutFileName;
    char*       pcOutContents;
    char        pcTargetFileName[MAX_PATH];

    eStatus          = MOS_STATUS_SUCCESS;
    lLastFieldOffset = 0;
    lLastFieldSize   = 0;
    lSize            = 0;
    uiLastIndex      = 0;
    iStrLenPath      = 0;
    iStrLenName      = 0;
    pcOutFileName    = nullptr;
    pcOutContents    = nullptr;
    memset(pcTargetFileName, '\0', MAX_PATH);

    VPHAL_DEBUG_CHK_STATUS(VphalHwStateDumper::CheckPath(&pcPath));

    pcOutFileName = (char*)MOS_AllocAndZeroMemory(sizeof(char) * MAX_PATH);
    VPHAL_DEBUG_CHK_NULL(pcOutFileName);
    MOS_SecureStringPrint(pcOutFileName, MAX_PATH, MAX_PATH, "%s%s_%d.bin", pcPath, pcName, iID);

    GetLastFieldIndex(fields, uiNumFields, &uiLastIndex);
    lLastFieldOffset = fields[uiLastIndex].dwOffset;
    lLastFieldSize   = fields[uiLastIndex].dwSize;
    lSize            = lLastFieldOffset + lLastFieldSize;

    VphalDumperTool::GetOsFilePath(pcOutFileName, pcTargetFileName);
    VPHAL_DEBUG_CHK_STATUS(MOS_WriteFileFromPtr(pcTargetFileName, pvStructToDump, lSize));

finish:
    MOS_SafeFreeMemory(pcOutFileName);
    MOS_SafeFreeMemory(pcOutContents);
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpBatchBufferBinary(
    PMHW_BATCH_BUFFER    pBatchBuffer,
    const char*          pcFileLoc,
    int32_t              iID)
{
    MOS_STATUS             eStatus;
    VPHAL_DBG_FIELD_LAYOUT field;
    char*                  pcPath;
    size_t                 iStrLen;
    bool                   bLockNeeded;

    eStatus     = MOS_STATUS_SUCCESS;
    pcPath      = nullptr;
    iStrLen     = 0;
    bLockNeeded = false;

    if (!pBatchBuffer->pData)
    {
        bLockNeeded = true;
        VPHAL_DEBUG_CHK_STATUS(m_renderHal->pfnLockBB(m_renderHal, pBatchBuffer));
    }

    iStrLen = strlen(pcFileLoc) + 1;

    field.dwOffset      = 0;
    field.uiNumChildren = 0;
    field.uiNumber      = 1;

    pcPath = (char*)MOS_AllocAndZeroMemory(sizeof(char) * iStrLen);
    VPHAL_DEBUG_CHK_NULL(pcPath);
    MOS_SecureStringPrint(pcPath, iStrLen, iStrLen, "%s", pcFileLoc);
    VPHAL_DEBUG_CHK_STATUS(VphalHwStateDumper::CheckPath(&pcPath));

    field.dwSize = pBatchBuffer->iSize;
    VPHAL_DEBUG_CHK_STATUS(DumpBinaryStruct(&field, 1,
                                        pBatchBuffer->pData,
                                        pcPath, "BB", iID));

    if (bLockNeeded)
    {
        VPHAL_DEBUG_CHK_STATUS(m_renderHal->pfnUnlockBB(m_renderHal, pBatchBuffer));
    }

finish:
    MOS_SafeFreeMemory(pcPath);
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpCommandBufferBinary(
    PMOS_COMMAND_BUFFER    pCmd_buff,
    const char*            pcFileLoc,
    int32_t                iID)
{
    /*
        MOS_COMMAND_BUFFER

    DW0,DW1,DW2,DW3,DW4,DW5,DW6,      ...
    /-------------------------------------------------------------------------\
    |   |   |   |   |   |   |                                                 |
    |   |   |   |   |   |   |         ....                                    |
    |   |   |   |   |   |   |                                                 |
    |   |   |   |   |   |   |                                                 |
    \-------------------------------------------------------------------------/
    |\_________________/|\___________________________________________________/
    |        |          |                           |
pCmdBase  iOffset    pCmdPtr                      iRemaining

    pCmdBase  : First command in the buffer
    pCmdPtr   : End of current command (place to put next command)
    iOffset   : Space being used by commands (pCmdPtr - pCmdBase)
    iRemaining: Space left for comments till end of buffer
    */

    MOS_STATUS             eStatus;
    VPHAL_DBG_FIELD_LAYOUT field;
    char*                  pcPath;
    size_t                 iStrLen;

    eStatus = MOS_STATUS_SUCCESS;
    pcPath  = nullptr;
    iStrLen = strlen(pcFileLoc) + 1;

    field.dwOffset      = 0;
    field.uiNumber      = 1;
    field.uiNumChildren = 0;
    field.dwSize        = pCmd_buff->iOffset;

    pcPath = (char*)MOS_AllocAndZeroMemory(sizeof(char) * iStrLen);
    VPHAL_DEBUG_CHK_NULL(pcPath);
    MOS_SecureStringPrint(pcPath, iStrLen, iStrLen, "%s", pcFileLoc);
    VPHAL_DEBUG_CHK_STATUS(VphalHwStateDumper::CheckPath(&pcPath));

    VPHAL_DEBUG_CHK_STATUS(DumpBinaryStruct(&field, 1,
                                        pCmd_buff->pCmdBase,
                                        pcPath, "CMB", iID));

finish:
    MOS_SafeFreeMemory(pcPath);
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpGshBinary(
    const char*          pcFileLoc,
    int32_t              iID)
{
    MOS_STATUS                      eStatus;
    uint32_t                        dwSizeMS;
    size_t                          iStrLen;
    char*                           pcPath;
    char*                           pcOutFileName;
    char*                           pcOutContents;
    char*                           pcTargetFileName;
    PRENDERHAL_STATE_HEAP           pStateHeap;
    PRENDERHAL_STATE_HEAP_SETTINGS  pStateHeapSettings;

    // GSH LAYOUT
    VPHAL_DBG_FIELD_LAYOUT* pGshLayout = nullptr;
    uint32_t                uiNumGSHFields = 0;

    eStatus             = MOS_STATUS_SUCCESS;
    dwSizeMS            = 0;
    iStrLen             = 0;
    pcPath              = nullptr;
    pcOutFileName       = nullptr;
    pcOutContents       = nullptr;
    pcTargetFileName    = nullptr;
    pStateHeap          = nullptr;
    pStateHeapSettings  = nullptr;

    pcTargetFileName = (char*)MOS_AllocAndZeroMemory(MAX_PATH);
    VPHAL_DEBUG_CHK_NULL(pcTargetFileName);
    memset(pcTargetFileName, '\0', MAX_PATH);

    pStateHeap          = m_stateHeap;
    pStateHeapSettings  = m_stateHeapSettings;

    iStrLen = strlen(pcFileLoc) + 1;
    pcPath = (char*)MOS_AllocAndZeroMemory(sizeof(char) * iStrLen);
    VPHAL_DEBUG_CHK_NULL(pcPath);
    MOS_SecureStringPrint(pcPath, iStrLen, iStrLen, "%s", pcFileLoc);
    VPHAL_DEBUG_CHK_STATUS(VphalHwStateDumper::CheckPath(&pcPath));

    pcOutFileName = (char*)MOS_AllocAndZeroMemory(sizeof(char) * MAX_PATH);
    VPHAL_DEBUG_CHK_NULL(pcOutFileName);
    MOS_SecureStringPrint(pcOutFileName, MAX_PATH, MAX_PATH, "%sGSH_%d.xml", pcPath, iID);

    VPHAL_DEBUG_CHK_STATUS(DefGsh(&pGshLayout, &uiNumGSHFields));

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(true, &pcOutContents, XMLHEADER));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents,
                                  "<GSH><NAME>GSH</NAME><LOC>0x%x</LOC><VERSION>%s</VERSION>\r\n",
                                  (uintptr_t)pStateHeap->pGshBuffer, VPDEBUG_VERSION));

    VPHAL_DEBUG_CHK_STATUS(DumpSubfields(&pcOutContents, nullptr, pGshLayout, uiNumGSHFields));

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "</GSH>\r\n"));
    VphalDumperTool::GetOsFilePath(pcOutFileName, pcTargetFileName);
    iStrLen = strlen(pcOutContents);
    if(iStrLen > ULONG_MAX)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }
    dwSizeMS = (uint32_t)iStrLen;
    VPHAL_DEBUG_CHK_STATUS(MOS_WriteFileFromPtr(pcTargetFileName,
                                                    pcOutContents,
                                                    dwSizeMS));

    VPHAL_DEBUG_CHK_STATUS(DumpBinaryStruct(pGshLayout,
                                        uiNumGSHFields, pStateHeap->pGshBuffer,
                                        pcPath, "GSH", iID));

finish:
    MOS_SafeFreeMemory(pcPath);
    MOS_SafeFreeMemory(pcTargetFileName);
    MOS_SafeFreeMemory(pcOutFileName);
    MOS_SafeFreeMemory(pcOutContents);
    FreeLayout(pGshLayout, uiNumGSHFields);
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpSshBinary(
    const char*          pcFileLoc,
    int32_t              iID)
{
    MOS_STATUS                      eStatus;
    size_t                          iStrLen;
    int32_t                         i;
    char*                           pcPath;
    char*                           pcOutFileName;
    char*                           pcOutContents;
    char*                           pcTargetFileName;
    uint32_t                        dwSizeMS;
    PRENDERHAL_STATE_HEAP           pStateHeap;
    PRENDERHAL_STATE_HEAP_SETTINGS  pStateHeapSettings;

    // SSH LAYOUT
    uint32_t                        uiNumSSHFields = 0;
    VPHAL_DBG_FIELD_LAYOUT*         pSshLayout = nullptr;

    eStatus          = MOS_STATUS_SUCCESS;
    iStrLen          = 0;
    i                = 0;
    pcPath           = nullptr;
    pcOutFileName    = nullptr;
    pcOutContents    = nullptr;
    pcTargetFileName = nullptr;
    dwSizeMS         = 0;

    pcTargetFileName = (char*)MOS_AllocAndZeroMemory(MAX_PATH);
    VPHAL_DEBUG_CHK_NULL(pcTargetFileName);
    memset(pcTargetFileName, '\0', MAX_PATH);

    pStateHeap          = m_stateHeap;
    pStateHeapSettings  = m_stateHeapSettings;

    iStrLen = strlen(pcFileLoc) + 1;
    pcPath = (char*)MOS_AllocAndZeroMemory(sizeof(char) * iStrLen);
    VPHAL_DEBUG_CHK_NULL(pcPath);
    MOS_SecureStringPrint(pcPath, iStrLen, iStrLen, "%s", pcFileLoc);
    VPHAL_DEBUG_CHK_STATUS(VphalHwStateDumper::CheckPath(&pcPath));

    pcOutFileName = (char*)MOS_AllocAndZeroMemory(sizeof(char) * (iStrLen + 20));
    VPHAL_DEBUG_CHK_NULL(pcOutFileName);
    MOS_SecureStringPrint(pcOutFileName, iStrLen + 20, iStrLen + 20, "%sSSH_%d.xml", pcPath, iID);

    VPHAL_DEBUG_CHK_STATUS(DefSsh(&pSshLayout, &uiNumSSHFields));

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(true, &pcOutContents, XMLHEADER));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents,
                                  "<SSH><NAME>SSH</NAME><LOC>0x%x</LOC><VERSION>%s</VERSION>\r\n",
                                  (uintptr_t)pStateHeap->pSshBuffer, VPDEBUG_VERSION));

    VPHAL_DEBUG_CHK_STATUS(DumpSubfields(&pcOutContents, nullptr, pSshLayout,
                                    uiNumSSHFields));

    VphalDumperTool::AppendString(false, &pcOutContents, "</SSH>\r\n");

    VphalDumperTool::GetOsFilePath(pcOutFileName, pcTargetFileName);
    iStrLen = strlen(pcOutContents);
    if(iStrLen > ULONG_MAX)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }
    dwSizeMS = (uint32_t)iStrLen;
    VPHAL_DEBUG_CHK_STATUS(MOS_WriteFileFromPtr(pcTargetFileName,
                                                    pcOutContents,
                                                    dwSizeMS));

    VPHAL_DEBUG_CHK_STATUS(DumpBinaryStruct(pSshLayout,
                                        uiNumSSHFields, pStateHeap->pSshBuffer,
                                        pcPath, "SSH", iID));

finish:
    MOS_SafeFreeMemory(pcPath);
    MOS_SafeFreeMemory(pcOutFileName);
    MOS_SafeFreeMemory(pcOutContents);
    FreeLayout(pSshLayout, uiNumSSHFields);
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpVeboxStateBinary(
    PMHW_VEBOX_INTERFACE pVeboxInterface,
    const char*          pcFileLoc,
    int32_t              iID)
{
    MOS_STATUS             eStatus;
    VPHAL_DBG_FIELD_LAYOUT field;
    char*                  pcPath;
    size_t                 iStrLen;
    const MHW_VEBOX_HEAP   *pVeboxHeap = nullptr;

    eStatus      = MOS_STATUS_SUCCESS;
    pcPath      = nullptr;
    iStrLen     = strlen(pcFileLoc) + 1;

    VPHAL_DEBUG_CHK_STATUS(pVeboxInterface->GetVeboxHeapInfo(
                                &pVeboxHeap));
    VPHAL_DEBUG_CHK_NULL(pVeboxHeap);

    field.dwOffset      = 0;
    field.uiNumChildren = 0;
    field.uiNumber      = 1;

    pcPath = (char*)MOS_AllocAndZeroMemory(sizeof(char) * iStrLen);
    VPHAL_DEBUG_CHK_NULL(pcPath);
    MOS_SecureStringPrint(pcPath, iStrLen, iStrLen, "%s", pcFileLoc);

    field.dwSize = pVeboxHeap->uiInstanceSize;

    VPHAL_DEBUG_CHK_STATUS(DumpBinaryStruct(&field, 1,
                                        (pVeboxHeap->pLockedDriverResourceMem +pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize) ,
                                        pcPath, nullptr, iID));

finish:
    MOS_SafeFreeMemory(pcPath);
    return eStatus;
}

MOS_STATUS VphalHwStateDumper::DumpStatisticsBinary(
    PVPHAL_VEBOX_STATE      pVeboxState,
    uint8_t*                pStat0Base,
    uint8_t*                pStat1Base,
    const char*             pcFileLoc,
    int32_t                 iID)
{
    MOS_STATUS              eStatus;
    VPHAL_DBG_FIELD_LAYOUT  field;
    size_t                  iStrLen;
    char*                   pcOutFileName0;
    char*                   pcOutFileName1;
    int32_t                 dwWidth,dwHeight;
    void                    *pState0, *pState1;

    eStatus        = MOS_STATUS_SUCCESS;
    iStrLen        = 0;
    pcOutFileName0 = nullptr;
    pcOutFileName1 = nullptr;
    iStrLen        = strlen(pcFileLoc) + 4;
    pState0        = (void*)(pStat0Base);
    pState1        = (void*)(pStat1Base);

    pcOutFileName0 = (char*)MOS_AllocAndZeroMemory(sizeof(char) * iStrLen);
    VPHAL_DEBUG_CHK_NULL(pcOutFileName0);
    MOS_SecureStringPrint(pcOutFileName0, iStrLen, iStrLen, "%s[slice0]", pcFileLoc);

    pcOutFileName1 = (char*)MOS_AllocAndZeroMemory(sizeof(char) * iStrLen);
    VPHAL_DEBUG_CHK_NULL(pcOutFileName1);
    MOS_SecureStringPrint(pcOutFileName1, iStrLen, iStrLen, "%s[slice1]", pcFileLoc);

    field.dwOffset      = 0;
    field.uiNumChildren = 0;
    field.uiNumber      = 1;

    dwWidth      = MOS_ALIGN_CEIL(pVeboxState->m_currentSurface->dwWidth, 64);
    dwHeight     = MOS_ROUNDUP_DIVIDE(pVeboxState->m_currentSurface->dwHeight, 4) +
                   MOS_ROUNDUP_DIVIDE(VPHAL_VEBOX_STATISTICS_SIZE_MAX * sizeof(uint32_t), dwWidth);
    field.dwSize = dwWidth * dwHeight;

    VPHAL_DEBUG_CHK_STATUS(DumpBinaryStruct(&field, 1,
                                        pState0 , pcOutFileName0, nullptr, iID));
    VPHAL_DEBUG_CHK_STATUS(DumpBinaryStruct(&field, 1,
                                        pState1 , pcOutFileName1, nullptr, iID));

finish:
    MOS_SafeFreeMemory(pcOutFileName0);
    MOS_SafeFreeMemory(pcOutFileName1);

    return eStatus;
}

void VphalDumperTool::StringToLower(
    char* pcString)
{
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

MOS_STATUS VphalSurfaceDumper::LocStringToEnum(
    char*                           pcLocString,
    uint32_t                        *pLocation)
{
    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    VphalDumperTool::StringToLower(pcLocString);
    if (strcmp(pcLocString,      VPHAL_DBG_SURF_DUMP_LOC_PREALL)   == 0)
    {
        *pLocation = VPHAL_DBG_DUMP_TYPE_PRE_ALL;
    }
    else if (strcmp(pcLocString, VPHAL_DBG_SURF_DUMP_LOC_PREDNDI)  == 0)
    {
        *pLocation = VPHAL_DBG_DUMP_TYPE_PRE_DNDI;
    }
    else if (strcmp(pcLocString, VPHAL_DBG_SURF_DUMP_LOC_POSTDNDI) == 0)
    {
        *pLocation = VPHAL_DBG_DUMP_TYPE_POST_DNDI;
    }
    else if (strcmp(pcLocString, VPHAL_DBG_SURF_DUMP_LOC_PRECOMP)  == 0)
    {
        *pLocation = VPHAL_DBG_DUMP_TYPE_PRE_COMP;
    }
    else if (strcmp(pcLocString, VPHAL_DBG_SURF_DUMP_LOC_POSTCOMP) == 0)
    {
        *pLocation = VPHAL_DBG_DUMP_TYPE_POST_COMP;
    }
    else if (strcmp(pcLocString, VPHAL_DBG_SURF_DUMP_LOC_PREMEMDECOMP) == 0)
    {
        *pLocation = VPHAL_DBG_DUMP_TYPE_PRE_MEMDECOMP;
    }
    else if (strcmp(pcLocString, VPHAL_DBG_SURF_DUMP_LOC_POSTMEMDECOMP) == 0)
    {
        *pLocation = VPHAL_DBG_DUMP_TYPE_POST_MEMDECOMP;
    }
    else if (strcmp(pcLocString, VPHAL_DBG_SURF_DUMP_LOC_POSTALL)  == 0)
    {
        *pLocation = VPHAL_DBG_DUMP_TYPE_POST_ALL;
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

MOS_STATUS VphalSurfaceDumper::EnumToLocString(
    uint32_t                        Location,
    char*                           pcLocString)
{
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
        case VPHAL_DBG_DUMP_TYPE_PRE_ALL:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_SURF_DUMP_LOC_PREALL);
            break;
        case VPHAL_DBG_DUMP_TYPE_PRE_DNDI:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_SURF_DUMP_LOC_PREDNDI);
            break;
        case VPHAL_DBG_DUMP_TYPE_POST_DNDI:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_SURF_DUMP_LOC_POSTDNDI);
            break;
        case VPHAL_DBG_DUMP_TYPE_PRE_COMP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_SURF_DUMP_LOC_PRECOMP);
            break;
        case VPHAL_DBG_DUMP_TYPE_POST_COMP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_SURF_DUMP_LOC_POSTCOMP);
            break;
        case VPHAL_DBG_DUMP_TYPE_PRE_MEMDECOMP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_SURF_DUMP_LOC_PREMEMDECOMP);
            break;
        case VPHAL_DBG_DUMP_TYPE_POST_MEMDECOMP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_SURF_DUMP_LOC_POSTMEMDECOMP);
            break;
        case VPHAL_DBG_DUMP_TYPE_POST_ALL:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_SURF_DUMP_LOC_POSTALL);
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

MOS_STATUS VphalHwStateDumper::EnumToLocString(
    VPHAL_DEBUG_STAGE          Location,
    char*                      pcLocString)
{
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
        case VPHAL_DBG_STAGE_DNDI:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_STATE_DUMP_LOC_DNDI);
            break;
        case VPHAL_DBG_STAGE_VEBOX:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_STATE_DUMP_LOC_VEBOX);
            break;
        case VPHAL_DBG_STAGE_COMP:
            MOS_SecureStringPrint(pcLocString, MAX_PATH, MAX_PATH, VPHAL_DBG_STATE_DUMP_LOC_COMP);
            break;
        default:
            VPHAL_DEBUG_NORMALMESSAGE("Unknown dump location \"%d\".", Location);
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
    } // end switch

    stStrLen = strlen(pcLocString);
    for (i=0; i < stStrLen; i++)
    {
        pcLocString[i] = (char)toupper(pcLocString[i]);
    }

finish:
    return eStatus;
}

MOS_STATUS VphalSurfaceDumper::SurfTypeStringToEnum(
    char*                         pcSurfType,
    VPHAL_SURFACE_TYPE            *pSurfType)
{
    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;
    VphalDumperTool::StringToLower(pcSurfType);
    if (strcmp(pcSurfType,      VPHAL_DBG_SURF_DUMP_TYPE_BACKGROUND)   == 0)
    {
        *pSurfType = SURF_IN_BACKGROUND;
    }
    else if (strcmp(pcSurfType, VPHAL_DBG_SURF_DUMP_TYPE_PRIMARY)      == 0)
    {
        *pSurfType = SURF_IN_PRIMARY;
    }
    else if (strcmp(pcSurfType, VPHAL_DBG_SURF_DUMP_TYPE_SUBSTREAM)    == 0)
    {
        *pSurfType = SURF_IN_SUBSTREAM;
    }
    else if (strcmp(pcSurfType, VPHAL_DBG_SURF_DUMP_TYPE_REFERENCE)    == 0)
    {
        *pSurfType = SURF_IN_REFERENCE;
    }
    else if (strcmp(pcSurfType, VPHAL_DBG_SURF_DUMP_TYPE_RENDERTARGET) == 0)
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

MOS_STATUS VphalHwStateDumper::StateTypeStringToEnum(
    char*                         pcStateType,
    uint32_t                      *pStateType)
{
    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;
    VphalDumperTool::StringToLower(pcStateType);
    if (strcmp(pcStateType, VPHAL_DBG_STATE_DUMP_LOC_DNDI)   == 0)
    {
        *pStateType = VPHAL_DBG_STAGE_DNDI;
    }
    else if (strcmp(pcStateType, VPHAL_DBG_STATE_DUMP_LOC_VEBOX)  == 0)
    {
        *pStateType = VPHAL_DBG_STAGE_VEBOX;
    }
    else if (strcmp(pcStateType, VPHAL_DBG_STATE_DUMP_LOC_COMP)   == 0)
    {
        *pStateType = VPHAL_DBG_STAGE_COMP;
    }
    else
    {
        VPHAL_DEBUG_NORMALMESSAGE("Unknown surface type \"%s\".", pcStateType);
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

finish:
    return eStatus;
}

MOS_STATUS VphalSurfaceDumper::ProcessDumpLocations(
    char*                      pcDumpLocData)
{
    MOS_STATUS  eStatus;
    char*       pcCommaLoc;                                                        // pointer to next comma in dump location string
    char*       pcCurrToken;                                                       // pointer to current token in a string
    char*       pcColonLoc;                                                        // pointer to next colon in location string
    int32_t     iNumStrings;                                                       // number of dump locations
    int32_t     i;                                                                 // loop iteration counter
    VPHAL_DBG_SURF_DUMP_SPEC    *pDumpSpec = &m_dumpSpec;

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
        pDumpSpec->pDumpLocations = (VPHAL_DBG_SURF_DUMP_LOC*)MOS_AllocAndZeroMemory(
                    sizeof(VPHAL_DBG_SURF_DUMP_LOC) * pDumpSpec->iNumDumpLocs);
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

void VphalSurfaceDumper::GetSurfaceDumpSpec()
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;
    bool                            bDumpEnabled;
    char                            cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
    char                            pcDumpLocData[VPHAL_DBG_SURF_DUMP_MAX_DATA_LEN];
    VPHAL_DBG_SURF_DUMP_SPEC        *pDumpSpec = &m_dumpSpec;

    pDumpSpec->uiStartFrame    = 0xFFFFFFFF;
    pDumpSpec->uiEndFrame      = 0;
    pDumpSpec->pcOutputPath[0] = '\0';
    pcDumpLocData[0]           = '\0';
    bDumpEnabled               = false;

    // Get start frame
    // if start frame is not got assign a default value of 0
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_SURF_DUMP_START_FRAME_KEY_NAME_ID,
        &UserFeatureData));
    pDumpSpec->uiStartFrame = UserFeatureData.u32Data;

    // Get end frame
    // if end frame is not got assign a default value of max
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_SURF_DUMP_END_FRAME_KEY_NAME_ID,
        &UserFeatureData));
    pDumpSpec->uiEndFrame = UserFeatureData.u32Data;

    // Get out file path
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.StringData.pStringData = cStringData;
    UserFeatureData.StringData.uMaxSize    = MOS_USER_CONTROL_MAX_DATA_SIZE;
    UserFeatureData.StringData.uSize       = 0;    //set the default value. 0 is empty buffer.

    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_SURF_DUMP_OUTFILE_KEY_NAME_ID,
        &UserFeatureData));

    if (UserFeatureData.StringData.uSize > 0)
    {
        // Copy the Output path
        MOS_SecureMemcpy(
            pDumpSpec->pcOutputPath,
            MAX_PATH,
            UserFeatureData.StringData.pStringData,
            UserFeatureData.StringData.uSize);
    }
#if !defined(LINUX) && !defined(ANDROID)
    else
    {
        std::string vphalDumpFilePath;

        // Use state separation APIs to obtain appropriate storage location
        if (SUCCEEDED(GetDriverPersistentStorageLocation(vphalDumpFilePath)))
        {
            std::string m_outputFilePath;
            MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;

            m_outputFilePath = vphalDumpFilePath.c_str();
            m_outputFilePath.append(VPHAL_DBG_DUMP_OUTPUT_FOLDER);

            // Copy the Output path
            MOS_SecureMemcpy(
                pDumpSpec->pcOutputPath,
                MAX_PATH,
                m_outputFilePath.c_str(),
                m_outputFilePath.size());

            MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
            userFeatureWriteData.Value.StringData.pStringData = cStringData;
            userFeatureWriteData.Value.StringData.pStringData = const_cast<char *>(m_outputFilePath.c_str());
            userFeatureWriteData.Value.StringData.uSize       = m_outputFilePath.size();
            userFeatureWriteData.ValueID                      = __VPHAL_DBG_DUMP_OUTPUT_DIRECTORY_ID;
            MOS_UserFeature_WriteValues_ID(NULL, &userFeatureWriteData, 1);
        }
    }
#endif

    // Get dump locations
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.StringData.pStringData = cStringData;
    UserFeatureData.StringData.uMaxSize    = MOS_USER_CONTROL_MAX_DATA_SIZE;
    UserFeatureData.StringData.uSize       = 0;    //set the default value. 0 is empty buffer.

    MOS_CHK_STATUS_SAFE(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_SURF_DUMP_LOCATION_KEY_NAME_ID,
        &UserFeatureData));
    if (UserFeatureData.StringData.uSize > 0)
    {
        bDumpEnabled =  ((pDumpSpec->pcOutputPath[0] != '\0') &&
                         (UserFeatureData.StringData.pStringData[0] != '\0'));
    }

    if (bDumpEnabled)
    {
        VPHAL_DEBUG_CHK_STATUS(ProcessDumpLocations(
            UserFeatureData.StringData.pStringData));
    }

    // Get enableAuxDump
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_SURF_DUMP_ENABLE_AUX_DUMP_ID,
        &UserFeatureData));
    pDumpSpec->enableAuxDump = UserFeatureData.u32Data;

    // Get plane dump enabled flag
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_SURF_DUMPER_ENABLE_PLANE_DUMP,
        &UserFeatureData));
    pDumpSpec->enablePlaneDump = UserFeatureData.u32Data;

finish:
    if ((eStatus != MOS_STATUS_SUCCESS) || (!bDumpEnabled))
    {
        pDumpSpec->uiStartFrame = 1;
        pDumpSpec->uiEndFrame   = 0;
    }
}

MOS_STATUS VphalHwStateDumper::ProcessDumpStateLocations(
    char*                      pcDumpLocData)
{
    MOS_STATUS  eStatus;
    char*       pcCommaLoc;                                                        // pointer to next comma in dump location string
    char*       pcCurrToken;                                                       // pointer to current token in a string
    char*       pcRender;                                                          //pointer to video process type string
    int32_t     iNumStrings;                                                       // number of dump locations
    int32_t     iNumDumpLocs;
    int32_t     i = 0;
    VPHAL_DBG_DUMP_SPEC         *pDumpSpec = &m_dumpSpec;

    eStatus      = MOS_STATUS_SUCCESS;
    iNumStrings  = 0;

    if (strlen(pcDumpLocData) > 0)
    {
        // Count number of strings in key data
        // If non empty string, there is at least one value
        iNumStrings++;

        // Count the rest
        pcCommaLoc = pcDumpLocData;
        while ((pcCommaLoc = strchr(pcCommaLoc + 1, ';')) != nullptr)
        {
            iNumStrings++;
        }

        //calucate the number of GSH,SSH,CB,SB,VeboxState.
        pcCurrToken = pcDumpLocData;

        while(iNumStrings !=0)
        {
            pcCommaLoc = strchr(pcCurrToken, ';');
            if (pcCommaLoc != nullptr)
            {
                *pcCommaLoc = '\0';
            }

            pcRender = strchr(pcCurrToken, ':');

            if (pcRender != nullptr)
            {
                iNumDumpLocs = 1;
                pcRender = pcRender + 1;
                while ((pcRender = strchr(pcRender + 1, ',')) != nullptr)
                {
                    iNumDumpLocs++;
                }
            }
            else
            {
                iNumDumpLocs = 1;
            }

            if (!strncmp(pcCurrToken, "GSH", 3))
            {
                pDumpSpec->pGSHDumpSpec = (VPHAL_DBG_GSH_DUMP_SPEC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_GSH_DUMP_SPEC));
                pDumpSpec->pGSHDumpSpec->iNumDumpLocs=iNumDumpLocs;
                pDumpSpec->pGSHDumpSpec->pDumpLocations = (VPHAL_DBG_GSH_DUMP_LOC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_GSH_DUMP_LOC)*iNumDumpLocs);

                pcCurrToken = pcCurrToken+4;
                i           = 0;

                while(iNumDumpLocs!=0)
                {
                    pcRender = strchr(pcCurrToken, ',');
                    if (pcRender != nullptr)
                    {
                        *pcRender = '\0';
                    }

                    if(i<pDumpSpec->pGSHDumpSpec->iNumDumpLocs)
                    {
                        VPHAL_DEBUG_CHK_STATUS(StateTypeStringToEnum(pcCurrToken,
                                &(pDumpSpec->pGSHDumpSpec->pDumpLocations[i].DumpStage)));
                        i++;
                    }

                    if (pcRender != nullptr)
                    {
                        pcCurrToken = pcRender + 1;
                    }
                    iNumDumpLocs--;
                }
            }
            else if (!strncmp(pcCurrToken, "SSH", 3))
            {
                pDumpSpec->pSSHDumpSpec = (VPHAL_DBG_SSH_DUMP_SPEC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_SSH_DUMP_SPEC));
                pDumpSpec->pSSHDumpSpec->iNumDumpLocs=iNumDumpLocs;
                pDumpSpec->pSSHDumpSpec->pDumpLocations = (VPHAL_DBG_SSH_DUMP_LOC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_SSH_DUMP_LOC)*iNumDumpLocs);

                pcCurrToken = pcCurrToken+4;
                i           = 0;

                while(iNumDumpLocs!=0)
                {
                    pcRender = strchr(pcCurrToken, ',');
                    if (pcRender != nullptr)
                    {
                        *pcRender = '\0';
                    }

                    if(i<pDumpSpec->pSSHDumpSpec->iNumDumpLocs)
                    {
                        VPHAL_DEBUG_CHK_STATUS(StateTypeStringToEnum(pcCurrToken,
                                &(pDumpSpec->pSSHDumpSpec->pDumpLocations[i].DumpStage)));
                        i++;
                    }

                    if(pcRender != nullptr)
                    {
                        pcCurrToken = pcRender + 1;
                    }
                    iNumDumpLocs--;
                }
            }
            else if (!strncmp(pcCurrToken, "BB", 2))
            {
                pDumpSpec->pBBDumpSpec = (VPHAL_DBG_BB_DUMP_SPEC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_BB_DUMP_SPEC));
                pDumpSpec->pBBDumpSpec->iNumDumpLocs=iNumDumpLocs;
                pDumpSpec->pBBDumpSpec->pDumpLocations = (VPHAL_DBG_BB_DUMP_LOC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_BB_DUMP_LOC)*iNumDumpLocs);

                pcCurrToken = pcCurrToken+3;
                i           = 0;

                while(iNumDumpLocs!=0)
                {
                    pcRender = strchr(pcCurrToken, ',');
                    if (pcRender != nullptr)
                    {
                        *pcRender = '\0';
                    }

                    if(i<pDumpSpec->pBBDumpSpec->iNumDumpLocs)
                    {
                        VPHAL_DEBUG_CHK_STATUS(StateTypeStringToEnum(pcCurrToken,
                                &(pDumpSpec->pBBDumpSpec->pDumpLocations[i].DumpStage)));
                        i++;
                    }

                    if(pcRender != nullptr)
                    {
                        pcCurrToken = pcRender + 1;
                    }
                    iNumDumpLocs--;
                }
            }
            else if (!strncmp(pcCurrToken, "CB", 2))
            {
                pDumpSpec->pCBDumpSpec = (VPHAL_DBG_CB_DUMP_SPEC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_CB_DUMP_SPEC));
                pDumpSpec->pCBDumpSpec->iNumDumpLocs=iNumDumpLocs;
                pDumpSpec->pCBDumpSpec->pDumpLocations = (VPHAL_DBG_CB_DUMP_LOC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_CB_DUMP_LOC)*iNumDumpLocs);

                pcCurrToken = pcCurrToken+3;
                i           = 0;

                while(iNumDumpLocs!=0)
                {
                    pcRender = strchr(pcCurrToken, ',');
                    if (pcRender != nullptr)
                    {
                       *pcRender = '\0';
                    }

                    if(i<pDumpSpec->pCBDumpSpec->iNumDumpLocs)
                    {
                       VPHAL_DEBUG_CHK_STATUS(StateTypeStringToEnum(pcCurrToken,
                               &(pDumpSpec->pCBDumpSpec->pDumpLocations[i].DumpStage)));
                       i++;
                    }

                    if (pcRender != nullptr)
                    {
                       pcCurrToken = pcRender + 1;
                    }
                    iNumDumpLocs--;
                }
            }
            else if (!strncmp(pcCurrToken, "VeboxState", 10))
            {
                pDumpSpec->pVeboxStateDumpSpec = (VPHAL_DBG_VEBOXSTATE_DUMP_SPEC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_VEBOXSTATE_DUMP_SPEC));
                pDumpSpec->pVeboxStateDumpSpec->iNumDumpLocs=iNumDumpLocs;
                pDumpSpec->pVeboxStateDumpSpec->pDumpLocations = (VPHAL_DBG_VEBOXSTATE_DUMP_LOC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_VEBOXSTATE_DUMP_LOC)*iNumDumpLocs);

                pDumpSpec->pVeboxStateDumpSpec->pDumpLocations->DumpStage=VPHAL_DBG_STAGE_VEBOX;
                pcCurrToken = pcCurrToken+11;

            }
            else if (!strncmp(pcCurrToken, "Statistics", 10))
            {
                pDumpSpec->pStatisticsDumpSpec = (VPHAL_DBG_STATISTICS_DUMP_SPEC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_STATISTICS_DUMP_SPEC));
                pDumpSpec->pStatisticsDumpSpec->iNumDumpLocs=iNumDumpLocs;
                pDumpSpec->pStatisticsDumpSpec->pDumpLocations = (VPHAL_DBG_STATISTICS_DUMP_LOC*)MOS_AllocAndZeroMemory(
                sizeof(VPHAL_DBG_STATISTICS_DUMP_LOC)*iNumDumpLocs);

                pDumpSpec->pVeboxStateDumpSpec->pDumpLocations->DumpStage=VPHAL_DBG_STAGE_VEBOX;
                pcCurrToken = pcCurrToken+11;
            }

            if (pcCommaLoc != nullptr)
            {
                pcCurrToken = pcCommaLoc + 1;
            }
            iNumStrings--;

        }
    } // if data length > 0

finish:
    return eStatus;
}

void VphalHwStateDumper::GetStateDumpSpec()
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;
    PMOS_USER_FEATURE_VALUE_DATA    pUserFeatureData = &UserFeatureData;
    bool                            bDumpEnabled;
    char                            pcDumpLocData[VPHAL_DBG_SURF_DUMP_MAX_DATA_LEN];
    char                            cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
    VPHAL_DBG_DUMP_SPEC             *pDumpSpec = &m_dumpSpec;

    pDumpSpec->uiStartFrame    = 0xFFFFFFFF;
    pDumpSpec->uiEndFrame      = 0;
    pDumpSpec->pcOutputPath[0] = '\0';
    pcDumpLocData[0]           = '\0';
    bDumpEnabled               = false;

    // Get start frame
    // if start frame is not got assign a default value of 0
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_STATE_DUMP_START_FRAME_KEY_NAME_ID,
        &UserFeatureData));
    pDumpSpec->uiStartFrame = UserFeatureData.u32Data;

    // Get end frame
    // if end frame is not got assign a default value of max
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_STATE_DUMP_END_FRAME_KEY_NAME_ID,
        &UserFeatureData));
    pDumpSpec->uiEndFrame = UserFeatureData.u32Data;

    // Get out file path
    UserFeatureData.StringData.pStringData = cStringData;
    UserFeatureData.StringData.uMaxSize    = MOS_USER_CONTROL_MAX_DATA_SIZE;
    UserFeatureData.StringData.uSize       = 0;    // set the default value. 0 is empty buffer.

    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_STATE_DUMP_OUTFILE_KEY_NAME_ID,
        &UserFeatureData));

    if (pUserFeatureData->StringData.uSize > 0)
    {
        // Copy the Output path
        MOS_SecureMemcpy(
            pDumpSpec->pcOutputPath,
            MAX_PATH,
            pUserFeatureData->StringData.pStringData,
            pUserFeatureData->StringData.uSize);
    }
#if !defined(LINUX) && !defined(ANDROID)
    else
    {
        std::string vphalDumpFilePath;

        // Use state separation APIs to obtain appropriate storage location
        if (SUCCEEDED(GetDriverPersistentStorageLocation(vphalDumpFilePath)))
        {
            std::string m_outputFilePath;
            MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;

            m_outputFilePath = vphalDumpFilePath.c_str();
            m_outputFilePath.append(VPHAL_DBG_DUMP_OUTPUT_FOLDER);

            // Copy the Output path
            MOS_SecureMemcpy(
                pDumpSpec->pcOutputPath,
                MAX_PATH,
                m_outputFilePath.c_str(),
                m_outputFilePath.size());

            MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
            userFeatureWriteData.Value.StringData.pStringData = cStringData;
            userFeatureWriteData.Value.StringData.pStringData = const_cast<char *>(m_outputFilePath.c_str());
            userFeatureWriteData.Value.StringData.uSize       = m_outputFilePath.size();
            userFeatureWriteData.ValueID                      = __VPHAL_DBG_DUMP_OUTPUT_DIRECTORY_ID;
            MOS_UserFeature_WriteValues_ID(NULL, &userFeatureWriteData, 1);
        }
    }
#endif

    // Get dump locations
    UserFeatureData.StringData.pStringData = cStringData;
    UserFeatureData.StringData.uMaxSize    = MOS_USER_CONTROL_MAX_DATA_SIZE;
    UserFeatureData.StringData.uSize       = 0;    // set the default value. 0 is empty buffer.

    MOS_CHK_STATUS_SAFE(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_STATE_DUMP_LOCATION_KEY_NAME_ID,
        &UserFeatureData));

    if (pUserFeatureData->StringData.uSize > 0)
    {
        bDumpEnabled =  ((pDumpSpec->pcOutputPath[0] != '\0') &&
                         (pUserFeatureData->StringData.pStringData[0] != '\0'));
    }

    if (bDumpEnabled)
    {
        VPHAL_DEBUG_CHK_STATUS(ProcessDumpStateLocations(
            pUserFeatureData->StringData.pStringData));
    }

finish:
    if ((eStatus != MOS_STATUS_SUCCESS) || (!bDumpEnabled))
    {
        pDumpSpec->uiStartFrame = 1;
        pDumpSpec->uiEndFrame   = 0;
    }
}

VphalSurfaceDumper::~VphalSurfaceDumper()
{
    MOS_SafeFreeMemory(m_dumpSpec.pDumpLocations);
}

VphalHwStateDumper::~VphalHwStateDumper()
{
    VPHAL_DBG_DUMP_SPEC *pDumpSpec = &m_dumpSpec;

    if (pDumpSpec != nullptr)
    {
        if(pDumpSpec->pGSHDumpSpec != nullptr)
            MOS_SafeFreeMemory(pDumpSpec->pGSHDumpSpec->pDumpLocations);
        if(pDumpSpec->pSSHDumpSpec != nullptr)
            MOS_SafeFreeMemory(pDumpSpec->pSSHDumpSpec->pDumpLocations);
        if(pDumpSpec->pBBDumpSpec != nullptr)
            MOS_SafeFreeMemory(pDumpSpec->pBBDumpSpec->pDumpLocations);
        if(pDumpSpec->pCBDumpSpec != nullptr)
            MOS_SafeFreeMemory(pDumpSpec->pCBDumpSpec->pDumpLocations);
        if(pDumpSpec->pVeboxStateDumpSpec != nullptr)
            MOS_SafeFreeMemory(pDumpSpec->pVeboxStateDumpSpec->pDumpLocations);
        if(pDumpSpec->pStatisticsDumpSpec != nullptr)
            MOS_SafeFreeMemory(pDumpSpec->pStatisticsDumpSpec->pDumpLocations);

        MOS_SafeFreeMemory(pDumpSpec->pGSHDumpSpec);
        MOS_SafeFreeMemory(pDumpSpec->pSSHDumpSpec);
        MOS_SafeFreeMemory(pDumpSpec->pBBDumpSpec);
        MOS_SafeFreeMemory(pDumpSpec->pCBDumpSpec);
        MOS_SafeFreeMemory(pDumpSpec->pVeboxStateDumpSpec);
        MOS_SafeFreeMemory(pDumpSpec->pStatisticsDumpSpec);
    }
}

uint32_t VphalSurfaceDumper::m_frameNumInVp = 0xffffffff;
char     VphalSurfaceDumper::m_dumpLocInVp[MAX_PATH];

MOS_STATUS VphalSurfaceDumper::DumpSurface(
    PVPHAL_SURFACE                  pSurf,
    uint32_t                        uiFrameNumber,
    uint32_t                        uiCounter,
    uint32_t                        Location)
{
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    int32_t VphalSurfDumpManualTrigger = VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_DEFAULT_NOT_SET;

    MOS_STATUS                 eStatus;
    int32_t                    i;
    VPHAL_DBG_SURF_DUMP_SPEC*  pDumpSpec = &m_dumpSpec;
    bool                       isDumpFromDecomp;
    bool                       orgDumpAuxEnable;

    eStatus             = MOS_STATUS_SUCCESS;
    i                   = 0;
    isDumpFromDecomp    = (Location == VPHAL_DBG_DUMP_TYPE_PRE_MEMDECOMP || Location == VPHAL_DBG_DUMP_TYPE_POST_MEMDECOMP);

    orgDumpAuxEnable    = m_dumpSpec.enableAuxDump;
    if ((Location == VPHAL_DBG_DUMP_TYPE_PRE_MEMDECOMP) ||
        (Location == VPHAL_DBG_DUMP_TYPE_POST_MEMDECOMP && m_frameNumInVp != 0xffffffff))
    {
        // For PreMemDecomp, dump without aux is meaningless
        // and, if we don't turn on aux dump, the surface will be deswizzled,
        // while Mos_Specific_LockResource() will perform deswilling to temp buffer and then copy to locked buffer
        // This will break the compressed surface.
        // So, we cannot dump compressed surf with deswizzling under current implementation.
        // And, for PostMemDecomp, although aux surf should be zero, we can dump it out for comparison.
        // Besides, we don't need to deswizzle as the original dump from VP (preAll etc) will do the deswizzling
        // For pre/post mem decomp from Codec/3D, doing deswizzling might help to check the result visually
        m_dumpSpec.enableAuxDump = true; 
    }

    if (m_frameNumInVp != 0xffffffff && isDumpFromDecomp)
    {
        // override the uiFrameNumer as it is during Vphal dumping its surface and already in lock and decomp phase
        uiFrameNumber = m_frameNumInVp;
    }

    if (!isDumpFromDecomp)
    {
        m_frameNumInVp      = uiFrameNumber;
    }

    // Get if manual triggered build
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_KEY_NAME_ID,
        &UserFeatureData);
    VphalSurfDumpManualTrigger = UserFeatureData.u32Data;

    if (VphalSurfDumpManualTrigger != VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_DEFAULT_NOT_SET)
    {
        if (VphalSurfDumpManualTrigger == VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_STARTED)
        {
            VPHAL_DEBUG_NORMALMESSAGE("Dump manaul trigger enabled, dump started: %d \n", VphalSurfDumpManualTrigger);

            for (i = 0; i < pDumpSpec->iNumDumpLocs; i++)
            {
                if (pDumpSpec->pDumpLocations[i].DumpLocation == Location &&      // should dump at this pipeline location AND
                    (pDumpSpec->pDumpLocations[i].SurfType == pSurf->SurfType ||  // should dump for this surface type OR
                        pDumpSpec->pDumpLocations[i].SurfType == SURF_NONE))      // should dump for any surface type
                {
                    VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, m_dumpLoc));
                    if (!isDumpFromDecomp && pSurf->bIsCompressed)
                    {
                        EnumToLocString(Location, m_dumpLocInVp);
                    }

                    if (!isDumpFromDecomp || m_dumpLocInVp[0] == 0)
                    {
                        MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_loc[%s]_lyr[%d]", pDumpSpec->pcOutputPath, m_dumpLoc, uiCounter);
                    }
                    else
                    {
                        MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_loc[%s_%s]_lyr[%d]", pDumpSpec->pcOutputPath, m_dumpLocInVp, m_dumpLoc, uiCounter);
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
        else if (VphalSurfDumpManualTrigger == VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_STOPPED)
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
                VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, m_dumpLoc));
                if (!isDumpFromDecomp && pSurf->bIsCompressed)
                {
                    EnumToLocString(Location, m_dumpLocInVp);
                }

                if (!isDumpFromDecomp || m_dumpLocInVp[0] == 0)
                {
                    MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_loc[%s]_lyr[%d]",
                        pDumpSpec->pcOutputPath, m_dumpLoc, uiCounter);
                }
                else
                {
                    MOS_SecureStringPrint(m_dumpPrefix, MAX_PATH, MAX_PATH, "%s/surfdump_loc[%s_%s]_lyr[%d]",
                        pDumpSpec->pcOutputPath, m_dumpLocInVp, m_dumpLoc, uiCounter);
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
    if (!isDumpFromDecomp)
    {
        m_frameNumInVp      = 0xffffffff;
        m_dumpLocInVp[0]    = 0;
    }
    m_dumpSpec.enableAuxDump = orgDumpAuxEnable;

    return eStatus;
}

MOS_STATUS VphalSurfaceDumper::DumpSurfaceArray(
    PVPHAL_SURFACE                  *ppSurfaces,
    uint32_t                        uiMaxSurfaces,
    uint32_t                        uiNumSurfaces,
    uint32_t                        uiFrameNumber,
    uint32_t                        Location)
{
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

void VphalHwStateDumper::DumpGSH()
{
    MOS_STATUS            eStatus;
    int32_t               iIndex;
    uint64_t              uiPhase;
    VPHAL_DBG_DUMP_SPEC*  pDumpSpec;
    VPHAL_DEBUG_STAGE     Location;

    char pcDumpPrefix[MAX_PATH];
    char pcDumpLoc[MAX_PATH];

    Location  = (VPHAL_DEBUG_STAGE)iDebugStage;
    uiPhase   = iPhase;
    pDumpSpec = &m_dumpSpec;
    if ((pDumpSpec->pGSHDumpSpec == nullptr)                          ||
        (pDumpSpec->pGSHDumpSpec->pDumpLocations == nullptr)          ||
        (pDumpSpec->pGSHDumpSpec->pDumpLocations->DumpStage == 0))
    {
        goto finish;
    }
    MOS_ZeroMemory(pcDumpPrefix, MAX_PATH);
    MOS_ZeroMemory(pcDumpLoc,    MAX_PATH);

    if(pDumpSpec->uiStartFrame <= pDumpSpec->uiCurrentFrame &&
        pDumpSpec->uiCurrentFrame <= pDumpSpec->uiEndFrame)
    {
        for (iIndex= 0; iIndex< pDumpSpec->pGSHDumpSpec->iNumDumpLocs; iIndex++)
        {
            if (pDumpSpec->pGSHDumpSpec->pDumpLocations[iIndex].DumpStage == Location)
            {
                VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, pcDumpLoc));
                MOS_SecureStringPrint(pcDumpPrefix, MAX_PATH, MAX_PATH, "%s/GSH_dump_loc[%s]_[%lld]",
                    pDumpSpec->pcOutputPath, pcDumpLoc, uiPhase);

                DumpGshBinary(
                    pcDumpPrefix,
                    pDumpSpec->uiCurrentFrame);

              break;
            }
        }
    }

finish:
    return;

}

void VphalHwStateDumper::DumpSSH()
{
    MOS_STATUS            eStatus;
    int32_t               iIndex;
    uint64_t              uiPhase;
    VPHAL_DBG_DUMP_SPEC*  pDumpSpec;
    VPHAL_DEBUG_STAGE     Location;

    char  pcDumpPrefix[MAX_PATH];
    char  pcDumpLoc[MAX_PATH];

    Location  = (VPHAL_DEBUG_STAGE)iDebugStage;
    uiPhase   = iPhase;
    pDumpSpec = &m_dumpSpec;
    if ((pDumpSpec->pSSHDumpSpec == nullptr)                          ||
        (pDumpSpec->pSSHDumpSpec->pDumpLocations == nullptr)          ||
        (pDumpSpec->pSSHDumpSpec->pDumpLocations->DumpStage == 0))
    {
        goto finish;
    }

    MOS_ZeroMemory(pcDumpPrefix, MAX_PATH);
    MOS_ZeroMemory(pcDumpLoc,    MAX_PATH);

    if(pDumpSpec->uiStartFrame <= pDumpSpec->uiCurrentFrame &&
        pDumpSpec->uiCurrentFrame <= pDumpSpec->uiEndFrame)
    {
        for (iIndex= 0; iIndex< pDumpSpec->pSSHDumpSpec->iNumDumpLocs; iIndex++)
        {
            if (pDumpSpec->pSSHDumpSpec->pDumpLocations[iIndex].DumpStage == Location)
            {
                VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, pcDumpLoc));
                MOS_SecureStringPrint(pcDumpPrefix, MAX_PATH, MAX_PATH, "%s/SSH_dump_loc[%s]_[%lld]",
                    pDumpSpec->pcOutputPath, pcDumpLoc, uiPhase);

                DumpSshBinary(
                    pcDumpPrefix,
                    pDumpSpec->uiCurrentFrame);
                break;
            }
        }
    }

finish:
    return;

}

void VphalHwStateDumper::DumpBatchBuffer(
    PMHW_BATCH_BUFFER    pBatchBuffer)
{
    MOS_STATUS           eStatus;
    int32_t              iIndex;
    uint64_t             uiPhase;
    VPHAL_DBG_DUMP_SPEC* pDumpSpec;
    VPHAL_DEBUG_STAGE    Location;

    char  pcDumpPrefix[MAX_PATH];
    char  pcDumpLoc[MAX_PATH];

    Location  = (VPHAL_DEBUG_STAGE)iDebugStage;
    uiPhase   = iPhase;
    pDumpSpec = &m_dumpSpec;
    if ((pDumpSpec->pBBDumpSpec == nullptr)                          ||
        (pDumpSpec->pBBDumpSpec->pDumpLocations == nullptr)          ||
        (pDumpSpec->pBBDumpSpec->pDumpLocations->DumpStage == 0))
    {
        goto finish;
    }

    MOS_ZeroMemory(pcDumpPrefix, MAX_PATH);
    MOS_ZeroMemory(pcDumpLoc,    MAX_PATH);

    if(pDumpSpec->uiStartFrame <= pDumpSpec->uiCurrentFrame &&
        pDumpSpec->uiCurrentFrame <= pDumpSpec->uiEndFrame)
    {
        for (iIndex= 0; iIndex< pDumpSpec->pBBDumpSpec->iNumDumpLocs; iIndex++)
        {
            if (pDumpSpec->pBBDumpSpec->pDumpLocations[iIndex].DumpStage == Location)
            {
                VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, pcDumpLoc));
                MOS_SecureStringPrint(pcDumpPrefix, MAX_PATH, MAX_PATH, "%s/BB_dump_loc[%s]_[%lld]",
                    pDumpSpec->pcOutputPath, pcDumpLoc, uiPhase);

                DumpBatchBufferBinary(
                    pBatchBuffer,
                    pcDumpPrefix,
                    pDumpSpec->uiCurrentFrame);
                break;
            }
        }
    }

finish:
    return;
}

void VphalHwStateDumper::DumpCommandBuffer(
    PMOS_COMMAND_BUFFER       pCommandBuffer)
{
    MOS_STATUS           eStatus;
    int32_t              iIndex;
    uint64_t             uiPhase;
    VPHAL_DBG_DUMP_SPEC* pDumpSpec;
    VPHAL_DEBUG_STAGE    Location;

    char  pcDumpPrefix[MAX_PATH];
    char  pcDumpLoc[MAX_PATH];

    Location  = (VPHAL_DEBUG_STAGE)iDebugStage;
    uiPhase   = iPhase;
    pDumpSpec = &m_dumpSpec;
    if ((pDumpSpec->pCBDumpSpec == nullptr)                          ||
        (pDumpSpec->pCBDumpSpec->pDumpLocations == nullptr)          ||
        (pDumpSpec->pCBDumpSpec->pDumpLocations->DumpStage == 0))
    {
        goto finish;
    }

    MOS_ZeroMemory(pcDumpPrefix, MAX_PATH);
    MOS_ZeroMemory(pcDumpLoc,    MAX_PATH);

    if(pDumpSpec->uiStartFrame <= pDumpSpec->uiCurrentFrame &&
        pDumpSpec->uiCurrentFrame <= pDumpSpec->uiEndFrame)
    {
        for (iIndex= 0; iIndex< pDumpSpec->pCBDumpSpec->iNumDumpLocs; iIndex++)
        {
            if (pDumpSpec->pCBDumpSpec->pDumpLocations[iIndex].DumpStage == Location)
            {
                VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, pcDumpLoc));
                MOS_SecureStringPrint(pcDumpPrefix, MAX_PATH, MAX_PATH, "%s/CB_dump_loc[%s]_[%lld]",
                    pDumpSpec->pcOutputPath, pcDumpLoc,  uiPhase);

                DumpCommandBufferBinary(
                    pCommandBuffer,
                    pcDumpPrefix,
                    pDumpSpec->uiCurrentFrame);
                break;
            }
        }
    }

 finish:
    return;
}

void VphalHwStateDumper::DumpVeboxState(PVPHAL_VEBOX_STATE pVeboxState)
{
    MOS_STATUS           eStatus;
    int32_t              iIndex;
    uint64_t             uiPhase;
    VPHAL_DBG_DUMP_SPEC* pDumpSpec;
    VPHAL_DEBUG_STAGE    Location;
    PVPHAL_VEBOX_STATE   pVeboxRndrState = (PVPHAL_VEBOX_STATE)pVeboxState;

    char  pcDumpPrefix[MAX_PATH];
    char  pcDumpLoc[MAX_PATH];

    Location  = (VPHAL_DEBUG_STAGE)iDebugStage;
    uiPhase   = iPhase;
    pDumpSpec = &m_dumpSpec;
    if ((pDumpSpec->pVeboxStateDumpSpec == nullptr)                          ||
        (pDumpSpec->pVeboxStateDumpSpec->pDumpLocations == nullptr)          ||
        (pDumpSpec->pVeboxStateDumpSpec->pDumpLocations->DumpStage == 0))
    {
        goto finish;
    }

    MOS_ZeroMemory(pcDumpPrefix, MAX_PATH);
    MOS_ZeroMemory(pcDumpLoc,    MAX_PATH);

    if(pDumpSpec->uiStartFrame <= pDumpSpec->uiCurrentFrame &&
        pDumpSpec->uiCurrentFrame <= pDumpSpec->uiEndFrame)
    {
        for (iIndex= 0; iIndex< pDumpSpec->pVeboxStateDumpSpec->iNumDumpLocs; iIndex++)
        {
            if (pDumpSpec->pVeboxStateDumpSpec->pDumpLocations[iIndex].DumpStage == Location)
            {
                VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, pcDumpLoc));
                MOS_SecureStringPrint(pcDumpPrefix, MAX_PATH, MAX_PATH, "%s/VeboxState_dump_loc[%s]_",
                    pDumpSpec->pcOutputPath,pcDumpLoc);

                DumpVeboxStateBinary(
                    pVeboxRndrState->m_pVeboxInterface,
                    pcDumpPrefix,
                    pDumpSpec->uiCurrentFrame);
                break;
            }
        }
    }

finish:
    return;
}

void VphalHwStateDumper::DumpStatistics(
    void*                       pVeboxState,
    uint8_t*                    pStat0Base,
    uint8_t*                    pStat1Base)
{
    MOS_STATUS           eStatus;
    int32_t              iIndex;
    uint64_t             uiPhase;
    VPHAL_DBG_DUMP_SPEC* pDumpSpec;
    VPHAL_DEBUG_STAGE    Location;

    char  pcDumpPrefix[MAX_PATH];
    char  pcDumpLoc[MAX_PATH];

    Location  = (VPHAL_DEBUG_STAGE)iDebugStage;
    uiPhase   = iPhase;
    pDumpSpec = &m_dumpSpec;
    if ((pDumpSpec->pStatisticsDumpSpec == nullptr)                          ||
        (pDumpSpec->pStatisticsDumpSpec->pDumpLocations == nullptr)          ||
        (pDumpSpec->pStatisticsDumpSpec->pDumpLocations->DumpStage == 0))
    {
        goto finish;
    }

    MOS_ZeroMemory(pcDumpPrefix, MAX_PATH);
    MOS_ZeroMemory(pcDumpLoc,    MAX_PATH);

    if(pDumpSpec->uiStartFrame <= pDumpSpec->uiCurrentFrame &&
        pDumpSpec->uiCurrentFrame <= pDumpSpec->uiEndFrame)
    {
        for (iIndex= 0; iIndex< pDumpSpec->pStatisticsDumpSpec->iNumDumpLocs; iIndex++)
        {
            if (pDumpSpec->pStatisticsDumpSpec->pDumpLocations[iIndex].DumpStage == Location)
            {
                VPHAL_DEBUG_CHK_STATUS(EnumToLocString(Location, pcDumpLoc));
                MOS_SecureStringPrint(pcDumpPrefix, MAX_PATH, MAX_PATH, "%s/Statistics_dump_loc[%s]_",
                    pDumpSpec->pcOutputPath,pcDumpLoc);

                DumpStatisticsBinary(
                    (PVPHAL_VEBOX_STATE)pVeboxState,
                    pStat0Base,
                    pStat1Base,
                    pcDumpPrefix,
                    pDumpSpec->uiCurrentFrame);
                break;
            }
        }
    }

finish:
    return;
}

void VphalParameterDumper::GetParametersDumpSpec()
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;
    bool                            bDumpEnabled;
    char                            cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
    VPHAL_DBG_PARAMS_DUMP_SPEC      *pDumpSpec = &m_dumpSpec;

    pDumpSpec->uiStartFrame       = 0xFFFFFFFF;
    pDumpSpec->uiEndFrame         = 0;
    pDumpSpec->outFileLocation[0] = '\0';
    cStringData[0]                = '\0';
    bDumpEnabled                  = false;

    // Get start frame
    // if start frame is not got assign a default value of 0
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_PARAM_DUMP_START_FRAME_KEY_NAME_ID,
        &UserFeatureData));
    pDumpSpec->uiStartFrame = UserFeatureData.u32Data;

    // Get end frame
    // if end frame is not got assign a default value of max
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_PARAM_DUMP_END_FRAME_KEY_NAME_ID,
        &UserFeatureData));
    pDumpSpec->uiEndFrame = UserFeatureData.u32Data;

    // Get out file path
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.StringData.pStringData = cStringData;
    UserFeatureData.StringData.uMaxSize = MOS_USER_CONTROL_MAX_DATA_SIZE;
    UserFeatureData.StringData.uSize = 0;    //set the default value. 0 is empty buffer.

    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_DBG_PARAM_DUMP_OUTFILE_KEY_NAME_ID,
        &UserFeatureData));
    if (UserFeatureData.StringData.uSize > 0)
    {
        // Copy the Output path
        MOS_SecureMemcpy(
            pDumpSpec->outFileLocation,
            MAX_PATH,
            UserFeatureData.StringData.pStringData,
            UserFeatureData.StringData.uSize);
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
            MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;

            m_outputFilePath = vphalDumpFilePath.c_str();
            m_outputFilePath.append(VPHAL_DBG_DUMP_OUTPUT_FOLDER);

            // Copy the Output path
            MOS_SecureMemcpy(
                pDumpSpec->outFileLocation,
                MAX_PATH,
                m_outputFilePath.c_str(),
                m_outputFilePath.size());

            MOS_ZeroMemory(&userFeatureWriteData, sizeof(userFeatureWriteData));
            userFeatureWriteData.Value.StringData.pStringData = cStringData;
            userFeatureWriteData.Value.StringData.pStringData = const_cast<char *>(m_outputFilePath.c_str());
            userFeatureWriteData.Value.StringData.uSize       = m_outputFilePath.size();
            userFeatureWriteData.ValueID                      = __VPHAL_DBG_DUMP_OUTPUT_DIRECTORY_ID;
            MOS_UserFeature_WriteValues_ID(NULL, &userFeatureWriteData, 1);

            bDumpEnabled = true;
        }
    }
#endif

    if ((eStatus != MOS_STATUS_SUCCESS) || (!bDumpEnabled))
    {
        pDumpSpec->uiStartFrame = 1;
        pDumpSpec->uiEndFrame = 0;
    }
}

MOS_STATUS VphalParameterDumper::DumpSourceSurface(
    uint32_t                        uiFrameCounter,
    char                            *pcOutputPath,
    PVPHAL_SURFACE                  pSrc,
    uint32_t                        index,
    char*                           &pcOutContents)
{
    MOS_STATUS                      eStatus;
    char                            sSurfaceFilePath[MAX_PATH] = { 0 }, sOsSurfaceFilePath[MAX_PATH] = { 0 };

    eStatus               = MOS_STATUS_SUCCESS;

    //Color Information
    {
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Color Information -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_CSPACE>%s</VPHAL_CSPACE>\n",             GetColorSpaceStr(pSrc->ColorSpace)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<EXTENDED_GAMUT></EXTENDED_GAMUT>\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<PALETTE_ALLOCATION>%d</PALETTE_ALLOCATION>\n", (pSrc->iPalette)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<PALETTE_DATA>\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<PALETTE_DATA_TYPE>%s</PALETTE_DATA_TYPE>\n", GetPaletteTypeStr(pSrc->Palette.PaletteType)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_CSPACE>%s</VPHAL_CSPACE>\n",           GetColorSpaceStr(pSrc->Palette.ColorSpace)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HAS_ALPHA>%d</HAS_ALPHA>\n",                 pSrc->Palette.bHasAlpha ? 1 : 0));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<TOTAL_ENTRIES>%d</TOTAL_ENTRIES>\n",         pSrc->Palette.iTotalEntries));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<NUM_ENTRIES>%d</NUM_ENTRIES>\n",             pSrc->Palette.iNumEntries));
        for (int nIndex = 0; nIndex < pSrc->Palette.iTotalEntries; nIndex++)
        {
            if (pSrc->Palette.pPalette8)
            {
                VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<PVPHAL_COLOR_SAMPLE>%08x</PVPHAL_COLOR_SAMPLE>\n", pSrc->Palette.pPalette8[nIndex].dwValue));
            }
        }
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</PALETTE_DATA>\n"));

        //Rendering parameters
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Rendering parameters -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_BLENDING_PARAMS>\n"));
        if (pSrc->pBlendingParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_BLEND_TYPE>%s</VPHAL_BLEND_TYPE>\n", GetBlendTypeStr(pSrc->pBlendingParams->BlendType)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ALPHA type=\"real\">%.3f</ALPHA>\n",       pSrc->pBlendingParams->fAlpha));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ALPHA type=\"real\"></ALPHA>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_BLENDING_PARAMS>\n"));

        //Luma key params
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_LUMAKEY_PARAMS>\n"));
        if (pSrc->pLumaKeyParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<LUMA_LOW>%d</LUMA_LOW>\n", pSrc->pLumaKeyParams->LumaLow));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<LUMA_HIGH>%d</LUMA_HIGH>\n", pSrc->pLumaKeyParams->LumaHigh));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<LUMA_LOW></LUMA_LOW>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<LUMA_HIGH></LUMA_HIGH>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_LUMAKEY_PARAMS>\n"));

        //Propcamp params
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_PROCAMP_PARAMS>\n"));
        if (pSrc->pProcampParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLED>%d</ENABLED>\n",                       (pSrc->pProcampParams->bEnabled ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<BRIGHTNESS type=\"real\">%.3f</BRIGHTNESS>\n", pSrc->pProcampParams->fBrightness));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<CONTRAST type=\"real\">%.3f</CONTRAST>\n",     pSrc->pProcampParams->fContrast));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HUE type=\"real\">%.3f</HUE>\n",               pSrc->pProcampParams->fHue));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<SATURATION type=\"real\">%.3f</SATURATION>\n", pSrc->pProcampParams->fSaturation));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLED></ENABLED>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<BRIGHTNESS type=\"real\"></BRIGHTNESS>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<CONTRAST type=\"real\"></CONTRAST>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HUE type=\"real\"></HUE>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<SATURATION type=\"real\"></SATURATION>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_PROCAMP_PARAMS>\n"));

        //IEF parameter
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_IEF_PARAMS>\n"));
        if (pSrc->pIEFParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<IEFFACTOR type = \"real\">%.3f</IEFFACTOR>\n", pSrc->pIEFParams->fIEFFactor));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<IEFFACTOR type = \"real\"></IEFFACTOR>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_IEF_PARAMS>\n"));

        //Advanced processing
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Advanced processing -->\n"));
        //DI
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_DI_PARAMS>\n"));
        if (pSrc->pDeinterlaceParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<DIMODE>%s</DIMODE>\n",             GetDIModeStr(pSrc->pDeinterlaceParams->DIMode)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<ENABLE_FMD>%d</ENABLE_FMD>\n",     (pSrc->pDeinterlaceParams->bEnableFMD ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<SINGLE_FIELD>%d</SINGLE_FIELD>\n", (pSrc->pDeinterlaceParams->bSingleField ? 1 : 0)));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<DIMODE></DIMODE>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<ENABLE_FMD></ENABLE_FMD>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<SINGLE_FIELD></SINGLE_FIELD>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_DI_PARAMS>\n"));
        //Denoise
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_DENOISE_PARAMS>\n"));
        if (pSrc->pDenoiseParams)
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<ENABLE_LUMA>%d</ENABLE_LUMA>\n",         (pSrc->pDenoiseParams->bEnableLuma ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<ENABLE_CHROMA>%d</ENABLE_CHROMA>\n",     (pSrc->pDenoiseParams->bEnableChroma ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<AUTO_DETECT>%d</AUTO_DETECT>\n",         (pSrc->pDenoiseParams->bAutoDetect ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<DENOISE_FACTOR>%.3f</DENOISE_FACTOR>\n", (pSrc->pDenoiseParams->fDenoiseFactor)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<DENOISE_FACTOR>%s</DENOISE_FACTOR>\n",    GetDenoiseModeStr(pSrc->pDenoiseParams->NoiseLevel)));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<ENABLE_LUMA></ENABLE_LUMA>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<ENABLE_CHROMA></ENABLE_CHROMA>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<AUTO_DETECT></AUTO_DETECT>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<DENOISE_FACTOR></DENOISE_FACTOR>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_DENOISE_PARAMS>\n"));
        //ColorPipe
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_COLORPIPE_PARAMS>\n"));
        if (pSrc->pColorPipeParams)
        {
            VPHAL_TCC_PARAMS TccParams = pSrc->pColorPipeParams->TccParams;
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_ACE>%d</ENABLE_ACE>\n",                (pSrc->pColorPipeParams->bEnableACE ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_STE>%d</ENABLE_STE>\n",                (pSrc->pColorPipeParams->bEnableSTE ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_TCC>%d</ENABLE_TCC>\n",                (pSrc->pColorPipeParams->bEnableTCC ? 1 : 0)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ACE_LEVEL>%d</ACE_LEVEL>\n",                  (pSrc->pColorPipeParams->dwAceLevel)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ACE_STRENGHTH>%d</ACE_STRENGHTH>\n",          (pSrc->pColorPipeParams->dwAceStrength)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<STE_PARAMS>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t\t<STE_FACTOR>%d</STE_FACTOR>\n",              (pSrc->pColorPipeParams->SteParams.dwSTEFactor)));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t</STE_PARAMS>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<TCC_PARAMS>%d,%d,%d,%d,%d,%d</TCC_PARAMS>\n", TccParams.Blue, TccParams.Cyan, TccParams.Green, TccParams.Magenta, TccParams.Red, TccParams.Yellow));
        }
        else
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_ACE></ENABLE_ACE>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_STE></ENABLE_STE>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ENABLE_TCC></ENABLE_TCC>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<ACE_LEVEL></ACE_LEVEL>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<STE_PARAMS></STE_PARAMS>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<TCC_PARAMS></TCC_PARAMS>\n"));
        }
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_COLORPIPE_PARAMS>\n"));
        //Gamut
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_GAMUT_PARAMS>\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<GCOMP_MODE></GCOMP_MODE>\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_GAMUT_PARAMS>\n"));

        //Sample information
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Sample information -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_FORMAT>%s</VPHAL_FORMAT>\n",             GetWholeFormatStr(pSrc->Format)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_SURFACE_TYPE>%s</VPHAL_SURFACE_TYPE>\n", GetSurfaceTypeStr(pSrc->SurfType)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_SAMPLE_TYPE>%s</VPHAL_SAMPLE_TYPE>\n",   GetSampleTypeStr(pSrc->SampleType)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_SCALING_MODE>%s</VPHAL_SCALING_MODE>\n", GetScalingModeStr(pSrc->ScalingMode)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_ROTATION_MODE>%s</VPHAL_ROTATION_MODE>\n", GetRotationModeStr(pSrc->Rotation)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<RCSRC>%d,%d,%d,%d</RCSRC>\n",                  pSrc->rcSrc.left, pSrc->rcSrc.top, pSrc->rcSrc.right, pSrc->rcSrc.bottom));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<RCDST>%d,%d,%d,%d</RCDST>\n",                  pSrc->rcDst.left, pSrc->rcDst.top, pSrc->rcDst.right, pSrc->rcDst.bottom));

        //Basic information
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Basic information -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_TILE_TYPE>%s</VPHAL_TILE_TYPE>\n", GetTileTypeStr(pSrc->TileType)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<WIDTH>%d</WIDTH>\n",                     pSrc->dwWidth));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<HEIGHT>%d</HEIGHT>\n",                   pSrc->dwHeight));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<PITCH>%d</PITCH>\n",                     pSrc->dwPitch));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<SIZE>%d</SIZE>\n",                       pSrc->dwPitch * pSrc->dwHeight));

        //Surface content initialization
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<!-- Surface content initialization -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<DATA>\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<DEFAULT_COLOR type=\"integer\">0x000000FF</DEFAULT_COLOR>\n"));
        if (pcOutputPath)
        {
            memset(sSurfaceFilePath, 0, MAX_PATH);
            memset(sOsSurfaceFilePath, 0, MAX_PATH);

            MOS_SecureStringPrint(sSurfaceFilePath, MAX_PATH, MAX_PATH, "%s%csurfdump_loc[preALL]_lyr[%d]_f[%03d]_w[%d]_h[%d]_p[%d].%s",
                pcOutputPath, MOS_DIR_SEPERATOR, index, uiFrameCounter, pSrc->dwWidth, pSrc->dwHeight, pSrc->dwPitch, VphalDumperTool::GetFormatStr(pSrc->Format));
            VphalDumperTool::GetOsFilePath(sSurfaceFilePath, sOsSurfaceFilePath);
        }
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<FILE>%s</FILE>\n", sOsSurfaceFilePath));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</DATA>\n"));
        // get backward reference
        if (pSrc->pBwdRef)
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<BACKREFDATA>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<DEFAULT_COLOR type=\"integer\">0x000000FF</DEFAULT_COLOR>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<Num>%d</Num>\n", pSrc->uBwdRefCount));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<FILE></FILE>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</BACKREFDATA>\n"));
        }
        // get forward reference
        if (pSrc->pFwdRef)
        {
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<FWDREFDATA>\n"));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<Num>%d</Num>\n", pSrc->uFwdRefCount));
            VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</FWDREFDATA>\n"));
        }
    }

    finish:
        return eStatus;
}

MOS_STATUS VphalParameterDumper::DumpTargetSurface(
    uint32_t                        uiFrameCounter,
    char                            *pcOutputPath,
    PVPHAL_SURFACE                  pTarget,
    uint32_t                        index,
    char*                           &pcOutContents)
{
    MOS_STATUS                      eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_FORMAT>%s</VPHAL_FORMAT>\n",             VphalParameterDumper::GetWholeFormatStr(pTarget->Format)));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_SURFACE_TYPE>%s</VPHAL_SURFACE_TYPE>\n", VphalParameterDumper::GetSurfaceTypeStr(pTarget->SurfType)));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_SAMPLE_TYPE>%s</VPHAL_SAMPLE_TYPE>\n",   GetSampleTypeStr(pTarget->SampleType)));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_CSPACE>%s</VPHAL_CSPACE>\n",             GetColorSpaceStr(pTarget->ColorSpace)));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_SCALING_MODE>%s</VPHAL_SCALING_MODE>\n", GetScalingModeStr(pTarget->ScalingMode)));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<VPHAL_TILE_TYPE>%s</VPHAL_TILE_TYPE>\n",       GetTileTypeStr(pTarget->TileType)));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<WIDTH>%d</WIDTH>\n",                           pTarget->dwWidth));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<HEIGHT>%d</HEIGHT>\n",                         pTarget->dwHeight));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<PITCH>%d</PITCH>\n",                           pTarget->dwPitch));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<SIZE>%d</SIZE>\n",                             pTarget->dwPitch*pTarget->dwHeight));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<RCSRC>%d,%d,%d,%d</RCSRC>\n",                    pTarget->rcSrc.left, pTarget->rcSrc.top, pTarget->rcSrc.right, pTarget->rcSrc.bottom));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<RCDST>%d,%d,%d,%d</RCDST>\n",                    pTarget->rcDst.left, pTarget->rcDst.top, pTarget->rcDst.right, pTarget->rcDst.bottom));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<DATA>\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t\t<DEFAULT_COLOR type=\"integer\">0x000000FF</DEFAULT_COLOR>\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t\t<FILE></FILE>\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t</DATA>\n"));

finish:
    return eStatus;
}

MOS_STATUS VphalParameterDumper::DumpRenderParameter(
    uint32_t                        uiFrameCounter,
    char                            *pcOutputPath,
    PVPHAL_RENDER_PARAMS            pRenderParams,
    char*                           &pcOutContents)
{
    MOS_STATUS                      eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    if (pRenderParams->pTarget[0])
    {
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_SURFACE_TARGET>\n"));
        VPHAL_DEBUG_CHK_STATUS(DumpTargetSurface(
            uiFrameCounter,
            pcOutputPath,
            pRenderParams->pTarget[0],
            0,
            pcOutContents));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_SURFACE_TARGET>\n"));
    }
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<SRC_COUNT>%d</SRC_COUNT>\n", pRenderParams->uSrcCount));

    //Color fill
    if (pRenderParams->pColorFillParams)
    {
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_COLORFILL_PARAMS>\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<bYCbCr>%d</bYCbCr>\n", pRenderParams->pColorFillParams->bYCbCr));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<COLOR type=\"integer\">%08x</COLOR>\n", pRenderParams->pColorFillParams->Color));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t\t<CSPACE>%s</CSPACE>\n", GetColorSpaceStr(pRenderParams->pColorFillParams->CSpace)));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t</VPHAL_COLORFILL_PARAMS>\n"));
    }

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<VPHAL_COMPONENT>%s</VPHAL_COMPONENT>\n", GetComponentStr(pRenderParams->Component)));

finish:
    return eStatus;
}

MOS_STATUS VphalParameterDumper::DumpToXML(
    uint32_t                        uiFrameCounter,
    char                            *pcOutputPath,
    PVPHAL_RENDER_PARAMS            pRenderParams)
{
    char                            sPath[MAX_PATH] = { 0 }, sOsPath[MAX_PATH] = { 0 };
    MOS_STATUS                      eStatus;
    char*                           pcOutContents;
    uint32_t                        dwStrLen = 0;
    FILE                            *fpOutXML;
    char*                           pCurFrameFileName;
    char*                           pBwdFrameFileName;
    VPHAL_DBG_PARAMS_DUMP_SPEC      *pParamsDumpSpec = &m_dumpSpec;

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
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(true, &pcOutContents,  "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\n"));
    // Create the root element.
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "<VPHAL_SCENARIO>\n"));
    // General infomation
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<ID>%d</ID>\n", MOS_GetPid()));

    VPHAL_DEBUG_CHK_NULL(pRenderParams->pSrc[0]);
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<DESCRIPTION>%d</DESCRIPTION>\n", pRenderParams->pSrc[0]->FrameID));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<!-- Number of frames to render -->\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<FRAME_COUNT type = \"integer\">1</FRAME_COUNT>\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<!-- 60i or 30p BLT -->\n"));
    if ((pRenderParams->uSrcCount > 0) &&
        (pRenderParams->pSrc[0]->SampleType != SAMPLE_PROGRESSIVE))
    {
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<BLT_TYPE>60i</BLT_TYPE>\n"));
    }
    else
    {
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<BLT_TYPE>30p</BLT_TYPE>\n"));
    }

    // Surface
    for (uint32_t i = 0; i < pRenderParams->uSrcCount; i++)
    {
        if (pRenderParams->pSrc[i] == nullptr)
            continue;

        //surface infomation
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<!-- Input surface definitions -->\n"));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<VPHAL_SURFACE>\n"));

        VPHAL_DEBUG_CHK_STATUS(DumpSourceSurface(
            uiFrameCounter,
            pcOutputPath,
            pRenderParams->pSrc[i],
            i,
            pcOutContents));
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t</VPHAL_SURFACE>\n"));
    }

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<!-- Rendering parameters -->\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<VPHAL_RENDER_PARAMS>\n"));

    VPHAL_DEBUG_CHK_STATUS(DumpRenderParameter(
        uiFrameCounter,
        pcOutputPath,
        pRenderParams,
        pcOutContents));

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t</VPHAL_RENDER_PARAMS>\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "</VPHAL_SCENARIO>\n"));

    MOS_SecureStringPrint(sPath, MAX_PATH, MAX_PATH, "%s%cparam_dump[%d].xml", pParamsDumpSpec->outFileLocation, MOS_DIR_SEPERATOR, uiFrameCounter);

    VphalDumperTool::GetOsFilePath(sPath, sOsPath);

    VPHAL_DEBUG_CHK_STATUS(MOS_WriteFileFromPtr(sOsPath, pcOutContents, strlen(pcOutContents)));
finish:
    if (pcOutContents)
    {
        MOS_FreeMemory(pcOutContents);
        pcOutContents = nullptr;
    }
    return eStatus;
}

VphalParameterDumper::~VphalParameterDumper()
{
}

const char * VphalDumperTool::GetFormatStr(MOS_FORMAT format)
{
    switch (format)
    {
        case Format_A8R8G8B8    : return _T("argb");
        case Format_X8R8G8B8    : return _T("xrgb");
        case Format_A8B8G8R8    : return _T("abgr");
        case Format_X8B8G8R8    : return _T("xbgr");
        case Format_A16R16G16B16: return _T("argb16");
        case Format_A16B16G16R16: return _T("abgr16");
        case Format_R5G6B5      : return _T("rgb16");
        case Format_R8G8B8      : return _T("rgb24");
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

MOS_STATUS VphalDumperTool::GetSurfaceSize(
    PVPHAL_SURFACE          pSurface,
    uint32_t                iBpp,
    uint32_t*               piWidthInBytes,
    uint32_t*               piHeightInRows)
{
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
        case Format_Y210:
        case Format_Y216:
        case Format_Y410:
        case Format_Y416:
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

MOS_STATUS VphalDumperTool::AppendString(
    bool   bFirst,
    char*  *ppcBigString,
    PCCHAR pcToAppendFmt,
    ...)
{
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
    MOS_SecureVStringPrint(pcToAppend, ALLOC_GRANULARITY, ALLOC_GRANULARITY, pcToAppendFmt, argList);
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

void VphalDumperTool::WriteFrame(
    PMOS_INTERFACE          pOsInterface,
    PVPHAL_SURFACE          pSurface,
    PCCHAR                  fileName,
    uint64_t                iCounter)
{
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

    VphalDumperTool::GetSurfaceSize(
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
        VphalDumperTool::GetFormatStr(pSurface->Format));

    MOS_SecureMemcpy(sOsPath, MAX_PATH, sPath, strlen(sPath));

    // Write the data to file
    if (pSurface->dwPitch == iWidthInBytes)
    {
        MOS_WriteFileFromPtr((const char *)sOsPath, pData, iSize);
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

        MOS_WriteFileFromPtr((const char *)sOsPath, pDst, iSize);
    }

    if (pDst)
    {
        MOS_FreeMemory(pDst);
    }

    pOsInterface->pfnUnlockResource(
        pOsInterface,
        &pSurface->OsResource);
}

const char * VphalParameterDumper::GetComponentStr(MOS_COMPONENT component)
{
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

const char * VphalParameterDumper::GetWholeFormatStr(MOS_FORMAT format)
{
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
    case Format_Y216:               return _T("Fomrat_Y216");
    case Format_Y410:               return _T("Format_Y410");
    case Format_YV12_Planar:        return _T("Format_YV12_Planar");
    case Format_P210:               return _T("Format_P210");
    case Format_P216:               return _T("Format_P216");
    case Format_Count:              return _T("Format_Count");
    default:                        return _T("Err");
    }

    return nullptr;
}

const char * VphalParameterDumper::GetTileTypeStr(MOS_TILE_TYPE tile_type)
{
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

const char * VphalParameterDumper::GetSurfaceTypeStr(VPHAL_SURFACE_TYPE surface_type)
{
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

const char * VphalParameterDumper::GetSampleTypeStr(VPHAL_SAMPLE_TYPE sample_type)
{
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

const char * VphalParameterDumper::GetColorSpaceStr(VPHAL_CSPACE color_space)
{
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

const char * VphalParameterDumper::GetBlendTypeStr(VPHAL_BLEND_TYPE blend_type)
{
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

const char * VphalParameterDumper::GetPaletteTypeStr(VPHAL_PALETTE_TYPE palette_type)
{
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

const char * VphalParameterDumper::GetScalingModeStr(VPHAL_SCALING_MODE scaling_mode)
{
    switch (scaling_mode)
    {
    case VPHAL_SCALING_NEAREST:         return _T("VPHAL_SCALING_NEAREST");
    case VPHAL_SCALING_BILINEAR:        return _T("VPHAL_SCALING_BILINEAR");
    case VPHAL_SCALING_AVS:             return _T("VPHAL_SCALING_AVS");
    default:                            return _T("Err");
    }

    return nullptr;
}

const char * VphalParameterDumper::GetRotationModeStr(VPHAL_ROTATION rotation_mode)
{
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

const char * VphalParameterDumper::GetDIModeStr(VPHAL_DI_MODE di_mode)
{
    switch (di_mode)
    {
    case DI_MODE_BOB:         return _T("DI_MODE_BOB");
    case DI_MODE_ADI:         return _T("DI_MODE_ADI");
    default:                  return _T("Err");
    }

    return nullptr;
}

const char * VphalParameterDumper::GetDenoiseModeStr(VPHAL_NOISELEVEL noise_level)
{
    switch (noise_level)
    {
    case NOISELEVEL_DEFAULT:         return _T("NOISELEVEL_DEFAULT");
    case NOISELEVEL_VC1_HD:          return _T("NOISELEVEL_VC1_HD");
    default:                         return _T("Err");
    }

    return nullptr;
}
#endif // (_DEBUG || _RELEASE_INTERNAL)
