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
//! \file     media_debug_dumper.cpp
//! \brief    Implementation of functions for debugging
//! \details  This file contains the implementation of functions for
//!           surface dumper
//!

#if (_DEBUG || _RELEASE_INTERNAL)

#include <stdio.h>
#include "mos_os.h"
#include "mos_context_next.h"
#include "media_debug_dumper.h"

#define COMMON_SURF_DUMP_OUTFILE_KEY_NAME        "outfileLocation"
#define COMMON_SURF_DUMP_LOCATION_KEY_NAME       "dumpLocations"
#define COMMON_SURF_DUMP_START_FRAME_KEY_NAME    "startFrame"
#define COMMON_SURF_DUMP_END_FRAME_KEY_NAME      "endFrame"
#define COMMON_SURF_DUMP_MAX_DATA_LEN            200
#define COMMON_SURF_DUMP_TYPE_BACKGROUND         "background"
#define COMMON_SURF_DUMP_TYPE_PRIMARY            "primary"
#define COMMON_SURF_DUMP_TYPE_SUBSTREAM          "substream"
#define COMMON_SURF_DUMP_TYPE_REFERENCE          "reference"
#define COMMON_SURF_DUMP_TYPE_RENDERTARGET       "rendertarget"
#define COMMON_SURF_DUMP_LOC_PREALL              "preall"
#define COMMON_SURF_DUMP_LOC_PREDNDI             "predndi"
#define COMMON_SURF_DUMP_LOC_POSTDNDI            "postdndi"
#define COMMON_SURF_DUMP_LOC_PRECOMP             "precomp"
#define COMMON_SURF_DUMP_LOC_POSTCOMP            "postcomp"
#define COMMON_SURF_DUMP_LOC_PREMEMDECOMP        "prememdecomp"
#define COMMON_SURF_DUMP_LOC_POSTMEMDECOMP       "postmemdecomp"
#define COMMON_SURF_DUMP_LOC_VEBOX_DRIVERHEAP    "veboxdriverheap"
#define COMMON_SURF_DUMP_LOC_VEBOX_KERNELHEAP    "veboxkernelheap"
#define COMMON_SURF_DUMP_LOC_POSTALL             "postall"

CommonSurfaceDumper::CommonSurfaceDumper(PMOS_INTERFACE pOsInterface) :
    m_osInterface(pOsInterface),
    m_dumpSpec()
{
    if (m_osInterface)
    {
        m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    }
}

CommonSurfaceDumper::~CommonSurfaceDumper()
{
}

bool CommonSurfaceDumper::HasAuxSurf(
    PMOS_RESOURCE    osResource)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

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

MOS_STATUS CommonSurfaceDumper::GetPlaneDefs(
    PMOS_SURFACE                  pSurface,
    COMMON_SURF_DUMP_SURFACE_DEF* pPlanes,
    uint32_t*                     pdwNumPlanes,
    uint32_t*                     pdwSize,
    bool                          auxEnable,
    bool                          isDeswizzled)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MOS_STATUS      eStatus;
    uint32_t        i;
    bool            PaddingEnable = false;

    eStatus = MOS_STATUS_SUCCESS;

    // Caller should supply this much!
    MOS_ZeroMemory(pPlanes, sizeof(COMMON_SURF_DUMP_SURFACE_DEF) * 3);

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

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;
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

        pPlanes[0].dwWidth = pSurface->dwWidth * 2;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;
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

        pPlanes[0].dwWidth = pSurface->dwWidth * 4;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;
        break;

    case Format_Y416:
    case Format_A16B16G16R16:
    case Format_A16R16G16B16:
    case Format_A16B16G16R16F:
    case Format_A16R16G16B16F:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth = pSurface->dwWidth * 8;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;
        break;

    case Format_NV12:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch = pSurface->dwPitch;
        break;

    case Format_P010:
    case Format_P016:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth = pSurface->dwWidth * 2;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch = pSurface->dwPitch;
        break;

    case Format_IMC2:
    case Format_IMC4:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_YVU9:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

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

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

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

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth / 2;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch / 2;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth / 2;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 2;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch / 2;
        break;
    case Format_400P:
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;
        break;

    case Format_411P:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth / 4;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth / 4;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_411R:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight / 4;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight / 4;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_422H:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

        pPlanes[1].dwWidth = pPlanes[0].dwWidth / 2;
        pPlanes[1].dwHeight = pPlanes[0].dwHeight;
        pPlanes[1].dwPitch = pPlanes[0].dwPitch;

        pPlanes[2].dwWidth = pPlanes[0].dwWidth / 2;
        pPlanes[2].dwHeight = pPlanes[0].dwHeight;
        pPlanes[2].dwPitch = pPlanes[0].dwPitch;
        break;

    case Format_422V:
        *pdwNumPlanes = 3;

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

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

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

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

        pPlanes[0].dwWidth = pSurface->dwWidth * 4;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;
        break;

    case Format_P210:
    case Format_P216:
        *pdwNumPlanes = 2;

        pPlanes[0].dwWidth = pSurface->dwWidth * 2;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;

        pPlanes[1].dwWidth = pSurface->dwWidth * 2;
        pPlanes[1].dwHeight = pSurface->dwHeight;
        pPlanes[1].dwPitch = pSurface->dwPitch;
        break;

    default:
        MEDIA_DEBUG_NORMALMESSAGE("Format '%d' not supported in current driver, using default 1 plane for dump", pSurface->Format);
        *pdwNumPlanes = 1;

        pPlanes[0].dwWidth = pSurface->dwWidth;
        pPlanes[0].dwHeight = pSurface->dwHeight;
        pPlanes[0].dwPitch = pSurface->dwPitch;
        break;
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
            MEDIA_DEBUG_ASSERTMESSAGE("More than 3 planes not supported.");
            break;
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

char* CommonSurfaceDumper::WhitespaceTrim(
    char*   ptr)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    char*    pcTemp;                             // pointer to temp string to remove spces

    if (ptr == nullptr)
    {
        MEDIA_DEBUG_ASSERTMESSAGE("ptr value = \"%s\".", ptr);
        return nullptr;
    }

    if ((strlen(ptr) == 1) && (*ptr != '\0'))
    {
        MEDIA_DEBUG_ASSERTMESSAGE("Single character is not supported.");
        *ptr = '\0';
    }

    if (strlen(ptr) == 0)
    {
        return ptr;
    }

    // Remove left spaces
    while (*ptr == ' ' || *ptr == '\t')
    {
        ptr++;
    }

    // Remove right spaces
    pcTemp = ptr + strlen(ptr)-1;
    while (*pcTemp == ' ' || *pcTemp == '\t')
    {
        pcTemp--;
    }
    *(++pcTemp) = '\0';

    return ptr;
}

MOS_STATUS CommonSurfaceDumper::SurfTypeStringToEnum(
    char*                         pcSurfType,
    COMMON_SURFACE_TYPE            *pSurfType)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    CommonDumperTool::StringToLower(pcSurfType);
    if (strcmp(pcSurfType,      COMMON_SURF_DUMP_TYPE_BACKGROUND)   == 0)
    {
        *pSurfType = COMMON_SURF_IN_BACKGROUND;
    }
    else if (strcmp(pcSurfType, COMMON_SURF_DUMP_TYPE_PRIMARY)      == 0)
    {
        *pSurfType = COMMON_SURF_IN_PRIMARY;
    }
    else if (strcmp(pcSurfType, COMMON_SURF_DUMP_TYPE_SUBSTREAM)    == 0)
    {
        *pSurfType = COMMON_SURF_IN_SUBSTREAM;
    }
    else if (strcmp(pcSurfType, COMMON_SURF_DUMP_TYPE_REFERENCE)    == 0)
    {
        *pSurfType = COMMON_SURF_IN_REFERENCE;
    }
    else if (strcmp(pcSurfType, COMMON_SURF_DUMP_TYPE_RENDERTARGET) == 0)
    {
        *pSurfType = COMMON_SURF_OUT_RENDERTARGET;
    }
    else
    {
        MEDIA_DEBUG_ASSERTMESSAGE("Unknown surface type \"%s\".", pcSurfType);
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS CommonSurfaceDumper::LocStringToEnum(
    char*                           pcLocString,
    uint32_t                        *pLocation)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    CommonDumperTool::StringToLower(pcLocString);
    if (strcmp(pcLocString,      COMMON_SURF_DUMP_LOC_PREALL)   == 0)
    {
        *pLocation = COMMON_DUMP_TYPE_PRE_ALL;
    }
    else if (strcmp(pcLocString, COMMON_SURF_DUMP_LOC_PREDNDI)  == 0)
    {
        *pLocation = COMMON_DUMP_TYPE_PRE_DNDI;
    }
    else if (strcmp(pcLocString, COMMON_SURF_DUMP_LOC_POSTDNDI) == 0)
    {
        *pLocation = COMMON_DUMP_TYPE_POST_DNDI;
    }
    else if (strcmp(pcLocString, COMMON_SURF_DUMP_LOC_PRECOMP)  == 0)
    {
        *pLocation = COMMON_DUMP_TYPE_PRE_COMP;
    }
    else if (strcmp(pcLocString, COMMON_SURF_DUMP_LOC_POSTCOMP) == 0)
    {
        *pLocation = COMMON_DUMP_TYPE_POST_COMP;
    }
    else if (strcmp(pcLocString, COMMON_SURF_DUMP_LOC_PREMEMDECOMP) == 0)
    {
        *pLocation = COMMON_DUMP_TYPE_PRE_MEMDECOMP;
    }
    else if (strcmp(pcLocString, COMMON_SURF_DUMP_LOC_POSTMEMDECOMP) == 0)
    {
        *pLocation = COMMON_DUMP_TYPE_POST_MEMDECOMP;
    }
    else if (strcmp(pcLocString, COMMON_SURF_DUMP_LOC_POSTALL)  == 0)
    {
        *pLocation = COMMON_DUMP_TYPE_POST_ALL;
    }
    else if (strcmp(pcLocString, COMMON_SURF_DUMP_LOC_VEBOX_DRIVERHEAP) == 0)
    {
        *pLocation = COMMON_DUMP_TYPE_VEBOX_DRIVERHEAP;
    }
    else if (strcmp(pcLocString, COMMON_SURF_DUMP_LOC_VEBOX_KERNELHEAP) == 0)
    {
        *pLocation = COMMON_DUMP_TYPE_VEBOX_KERNELHEAP;
    }
    else
    {
        MEDIA_DEBUG_NORMALMESSAGE("Unknown dump location \"%s\".", pcLocString);
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS CommonSurfaceDumper::ProcessDumpLocations(
    char*                      pcDumpLocData)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MOS_STATUS  eStatus;
    char*       pcCommaLoc;                                                        // pointer to next comma in dump location string
    char*       pcCurrToken;                                                       // pointer to current token in a string
    char*       pcColonLoc;                                                        // pointer to next colon in location string
    int32_t     iNumStrings;                                                       // number of dump locations
    int32_t     i;                                                                 // loop iteration counter
    COMMON_SURF_DUMP_SPEC    *pDumpSpec = &m_dumpSpec;

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
        pDumpSpec->pDumpLocations = (COMMON_SURF_DUMP_LOC*)MOS_AllocAndZeroMemory(
                    sizeof(COMMON_SURF_DUMP_LOC) * pDumpSpec->iNumDumpLocs);
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
                pDumpSpec->pDumpLocations[i].SurfType = COMMON_SURF_NONE;
            }
            else
            {
                // Remove surface type from dump location
                *pcColonLoc = '\0';
                // Set surface type
                pcColonLoc++;

                pcColonLoc = WhitespaceTrim(pcColonLoc);
            }

            //trim the whitespaces from dump location
            pcCurrToken = WhitespaceTrim(pcCurrToken);
        } // end for each part of the string
    } // if data length > 0

    return eStatus;
}

// Get dump location
MOS_STATUS CommonSurfaceDumper::GetSurfaceDumpLocation(
    char* dumpLoc,
    MCPY_DIRECTION mcpyDirection)
{
    MediaUserSetting::Value        outValue;

    MEDIA_DEBUG_FUNCTION_ENTER;

    // Get dump location
    outValue = "";

    if (mcpyDirection == mcpy_in)
    {
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __COMMON_DBG_SURF_DUMP_LOCATION_KEY_NAME_IN,
            MediaUserSetting::Group::Device);
    }
    else
    {
        ReadUserSettingForDebug(
            m_userSettingPtr,
            outValue,
            __COMMON_DBG_SURF_DUMP_LOCATION_KEY_NAME_OUT,
            MediaUserSetting::Group::Device);
    }

#if !defined(LINUX) && !defined(ANDROID)
    strcpy_s(dumpLoc, MAX_PATH, outValue.ConstString().c_str());
#else
    strcpy(dumpLoc, outValue.ConstString().c_str());
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CommonSurfaceDumper::ReAllocateSurface(
    PMOS_INTERFACE  pOsInterface,
    PMOS_SURFACE    pSurface,
    PMOS_SURFACE    pSrcSurf,
    PCCHAR          pSurfaceName,
    MOS_GFXRES_TYPE defaultResType,
    bool            useLinearResource)
{
    MOS_ALLOC_GFXRES_PARAMS allocParams;

    MEDIA_DEBUG_CHK_NULL(pOsInterface);
    MEDIA_DEBUG_CHK_NULL(pSrcSurf);
    MEDIA_DEBUG_CHK_NULL(pSurface);
    MEDIA_DEBUG_CHK_NULL(&pSurface->OsResource);

    // bCompressible should be compared with bCompressible since it is inited by bCompressible in previous call
    // TileType of surface should be compared since we need to reallocate surface if TileType changes
    if (!Mos_ResourceIsNull(&pSurface->OsResource) &&
        (pSurface->dwWidth == pSrcSurf->dwWidth) &&
        (pSurface->dwHeight == pSrcSurf->dwHeight) &&
        (pSurface->Format == pSrcSurf->Format) &&
        ((pSurface->bCompressible == pSrcSurf->bCompressible) || useLinearResource) && // omit this check as linear surface is uncompressible
        (pSurface->CompressionMode == pSrcSurf->CompressionMode) &&
        ((pSurface->TileType == pSrcSurf->TileType) ||
         (pSurface->TileType == MOS_TILE_LINEAR && useLinearResource))) // when useLinearResource no reallocation needed
    {
        MEDIA_DEBUG_NORMALMESSAGE("Skip to reallocate temp surface.");
        return MOS_STATUS_SUCCESS;
    }
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

#if !EMUL
    //  Need to reallocate surface according to expected tiletype instead of tiletype of the surface what we have
    if ((pSurface->OsResource.pGmmResInfo != nullptr) &&
        (pSurface->TileType == pSrcSurf->TileType))
    {
        // Reallocate but use same tile type and resource type as current
        allocParams.TileType = pSurface->OsResource.TileType;
        allocParams.Type     = defaultResType;
    }
    else
#endif
    {
        // First time allocation. Caller must specify default params
        allocParams.TileType = pSrcSurf->TileType;
        allocParams.Type     = defaultResType;
    }

    // Force to use tile linear for reallocated resource
    if (useLinearResource)
    {
        allocParams.TileType = MOS_TILE_LINEAR;
        allocParams.Type = MOS_GFXRES_2D;
    }

    allocParams.dwWidth         = pSrcSurf->dwWidth;
    allocParams.dwHeight        = pSrcSurf->dwHeight;
    allocParams.Format          = pSrcSurf->Format;
    allocParams.bIsCompressible = pSrcSurf->bCompressible;
    allocParams.CompressionMode = pSrcSurf->CompressionMode;
    allocParams.pBufName        = pSurfaceName;
    allocParams.dwArraySize     = 1;

    // Delete resource if already allocated
    pOsInterface->pfnFreeResource(pOsInterface, &(pSurface->OsResource));

    // Allocate surface
    MEDIA_DEBUG_CHK_STATUS(pOsInterface->pfnAllocateResource(
        pOsInterface,
        &allocParams,
        &pSurface->OsResource));

    pSurface->dwWidth         = pSrcSurf->dwWidth;
    pSurface->dwHeight        = pSrcSurf->dwHeight;
    pSurface->dwPitch         = pSrcSurf->dwPitch;
    pSurface->dwDepth         = pSrcSurf->dwDepth;
    pSurface->dwQPitch        = pSrcSurf->dwQPitch;
    pSurface->bArraySpacing   = pSrcSurf->bArraySpacing;
    pSurface->bCompressible   = pSrcSurf->bCompressible;
    pSurface->CompressionMode = pSrcSurf->CompressionMode;
    pSurface->bIsCompressed   = pSrcSurf->bIsCompressed;

    if (!pOsInterface->apoMosEnabled && !pOsInterface->apoMosForLegacyRuntime)
    {
        MOS_SURFACE details;
        MOS_ZeroMemory(&details, sizeof(details));
        details.Format = Format_Invalid;

        MEDIA_DEBUG_CHK_STATUS(pOsInterface->pfnGetResourceInfo(pOsInterface, &pSurface->OsResource, &details));

        pSurface->Format                      = details.Format;
        pSurface->TileType                    = details.TileType;
        pSurface->dwOffset                    = details.RenderOffset.YUV.Y.BaseOffset;
        pSurface->YPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.Y.BaseOffset;
        pSurface->YPlaneOffset.iXOffset       = details.RenderOffset.YUV.Y.XOffset;
        pSurface->YPlaneOffset.iYOffset =
            (pSurface->YPlaneOffset.iSurfaceOffset - pSurface->dwOffset) / pSurface->dwPitch +
            details.RenderOffset.YUV.Y.YOffset;
        pSurface->UPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.U.BaseOffset;
        pSurface->UPlaneOffset.iXOffset       = details.RenderOffset.YUV.U.XOffset;
        pSurface->UPlaneOffset.iYOffset =
            (pSurface->UPlaneOffset.iSurfaceOffset - pSurface->dwOffset) / pSurface->dwPitch +
            details.RenderOffset.YUV.U.YOffset;
        pSurface->UPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.U;
        pSurface->VPlaneOffset.iSurfaceOffset     = details.RenderOffset.YUV.V.BaseOffset;
        pSurface->VPlaneOffset.iXOffset           = details.RenderOffset.YUV.V.XOffset;
        pSurface->VPlaneOffset.iYOffset =
            (pSurface->VPlaneOffset.iSurfaceOffset - pSurface->dwOffset) / pSurface->dwPitch +
            details.RenderOffset.YUV.V.YOffset;
        pSurface->VPlaneOffset.iLockSurfaceOffset = details.LockOffset.YUV.V;
    }
    else
    {
        pSurface->Format = Format_Invalid;
        MEDIA_DEBUG_CHK_STATUS(pOsInterface->pfnGetResourceInfo(pOsInterface, &pSurface->OsResource, pSurface));
    }

    return MOS_STATUS_SUCCESS;
}

bool CommonSurfaceDumper::IsSyncFreeNeededForMMCSurface(
    PMOS_INTERFACE  osInterface, 
    PMOS_SURFACE    surface)
{
    if (nullptr == osInterface || nullptr == surface)
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

MOS_STATUS CommonSurfaceDumper::DumpSurfaceToFileEnd(
    PMOS_INTERFACE  pOsInterface,
    uint8_t        *pDst,
    bool            isSurfaceLocked,
    PMOS_RESOURCE   pLockedResource,
    PMOS_SURFACE    pSurface)
{
    MOS_STATUS eStatus;
    eStatus = MOS_STATUS_SUCCESS;

    MOS_SafeFreeMemory(pDst);

    if (isSurfaceLocked && pLockedResource != nullptr)
    {
        eStatus = (MOS_STATUS)pOsInterface->pfnUnlockResource(pOsInterface, pLockedResource);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            return eStatus;
        }
    }

    if (pSurface)
    {
        MOS_GFXRES_FREE_FLAGS resFreeFlags = {0};
        if (IsSyncFreeNeededForMMCSurface(pOsInterface, pSurface))
        {
            resFreeFlags.SynchronousDestroy = 1;
        }
        pOsInterface->pfnFreeResourceWithFlag(pOsInterface, &(pSurface->OsResource), resFreeFlags.Value);
    }
    MOS_SafeFreeMemory(pSurface);

    return eStatus;
}

MOS_STATUS CommonSurfaceDumper::DumpSurfaceToFile(
    PMOS_INTERFACE              pOsInterface,
    PMOS_SURFACE                pSurface,
    char                       *psPathPrefix,
    uint64_t                    iCounter,
    bool                        bLockSurface,
    bool                        bNoDecompWhenLock,
    uint8_t*                    pData)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MOS_STATUS                          eStatus;
    bool                                isSurfaceLocked;
    char                                sPath[MAX_PATH], sOsPath[MAX_PATH];
    uint8_t                            *pDst, *pTmpSrc, *pTmpDst;
    uint32_t                            dwNumPlanes, dwSize, j, i;
    COMMON_SURF_DUMP_SURFACE_DEF        planes[3];
    uint32_t                            dstPlaneOffset[3] = {0};
    MOS_LOCK_PARAMS                     LockFlags;
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData;
    bool                                hasAuxSurf;
    bool                                enableAuxDump;
    bool                                enablePlaneDump   = false;
    PMOS_RESOURCE                       pLockedResource   = nullptr;
    PMOS_SURFACE                        temp2DSurfForCopy = nullptr;

    MEDIA_DEBUG_CHK_NULL(pSurface);
    MEDIA_DEBUG_CHK_NULL(pOsInterface);
    MEDIA_DEBUG_CHK_NULL(psPathPrefix);

    eStatus         = MOS_STATUS_SUCCESS;
    isSurfaceLocked = false;
    hasAuxSurf      = false;
    pDst            = nullptr;
    enableAuxDump   = m_dumpSpec.enableAuxDump;
    MOS_ZeroMemory(sPath,   MAX_PATH);
    MOS_ZeroMemory(sOsPath, MAX_PATH);
    dwNumPlanes     = 0;
    enablePlaneDump = m_dumpSpec.enablePlaneDump;

    if (pSurface->dwDepth == 0)
    {
        pSurface->dwDepth = 1;
    }

    hasAuxSurf = HasAuxSurf(&pSurface->OsResource);

    // get plane definitions
    MEDIA_DEBUG_CHK_STATUS(GetPlaneDefs(
        pSurface,
        planes,
        &dwNumPlanes,
        &dwSize,
        hasAuxSurf, //(hasAuxSurf && enableAuxDump),
        !enableAuxDump));// !(hasAuxSurf && enableAuxDump)));

    if (bLockSurface)
    {
        // Caller should not give pData when it expect the function to lock surf
        pData = nullptr;

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
        if (isPlanar && pSurface->TileType != MOS_TILE_LINEAR)
        {
            temp2DSurfForCopy = (PMOS_SURFACE)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE));
            MEDIA_DEBUG_CHK_NULL(temp2DSurfForCopy);
           
            MEDIA_DEBUG_CHK_STATUS(ReAllocateSurface(
                m_osInterface,
                temp2DSurfForCopy,
                pSurface,
                "Temp2DSurfForSurfDumper",
                MOS_GFXRES_2D,
                1));

            m_osInterface->pfnDoubleBufferCopyResource(
                m_osInterface,
                &pSurface->OsResource,
                &temp2DSurfForCopy->OsResource,
                false);

            pData = (uint8_t *)pOsInterface->pfnLockResource(
                pOsInterface,
                &temp2DSurfForCopy->OsResource,
                &LockFlags);

            pLockedResource = &temp2DSurfForCopy->OsResource;

            // get plane definitions
            eStatus = GetPlaneDefs(
                temp2DSurfForCopy,
                planes,
                &dwNumPlanes,
                &dwSize,
                hasAuxSurf,        //(hasAuxSurf && enableAuxDump),
                !enableAuxDump);   // !(hasAuxSurf && enableAuxDump)));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                return(DumpSurfaceToFileEnd(
                    pOsInterface,
                    pDst,
                    isSurfaceLocked,
                    pLockedResource,
                    temp2DSurfForCopy));
            }
        }
        else
        {
            pData = (uint8_t *)pOsInterface->pfnLockResource(
                pOsInterface,
                &pSurface->OsResource,
                &LockFlags);
            pLockedResource = &pSurface->OsResource;
        }

        if (nullptr == pData)
        {
            return(DumpSurfaceToFileEnd(
                pOsInterface,
                pDst,
                isSurfaceLocked,
                pLockedResource,
                temp2DSurfForCopy));
        }

        isSurfaceLocked = true;

        // Write error to user feauture key
        eStatus = ReportUserSettingForDebug(
            m_userSettingPtr,
            __COMMON_DBG_SURF_DUMPER_RESOURCE_LOCK,
            1,
            MediaUserSetting::Group::Device);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            return(DumpSurfaceToFileEnd(
                pOsInterface,
                pDst,
                isSurfaceLocked,
                pLockedResource,
                temp2DSurfForCopy));
        }
    }

    MOS_SecureStringPrint(
        sPath,
        MAX_PATH,
        sizeof(sPath),
        "%s_f[%04lld]_w[%d]_h[%d]_p[%d].%s",
        psPathPrefix,
        iCounter,
        pSurface->dwWidth,
        pSurface->dwHeight,
        pSurface->dwPitch,
        CommonDumperTool::GetFormatStr(pSurface->Format));

    CommonDumperTool::GetOsFilePath(sPath, sOsPath);

    pDst = (uint8_t*)MOS_AllocAndZeroMemory(dwSize);

    if ((nullptr == pDst) || (nullptr == pData))
    {
        return(DumpSurfaceToFileEnd(
            pOsInterface,
            pDst,
            isSurfaceLocked,
            pLockedResource,
            temp2DSurfForCopy));
    }
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

                CommonDumperTool::GetOsFilePath(sPlanePath, sPlaneOsPath);

                eStatus = MosUtilities::MosWriteFileFromPtr(sPlaneOsPath, pDst + dstPlaneOffset[j], dstPlaneOffset[j + 1]);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    return(DumpSurfaceToFileEnd(
                    pOsInterface,
                    pDst,
                    isSurfaceLocked,
                    pLockedResource,
                    temp2DSurfForCopy));
                }
            }
            else
            {
                MEDIA_DEBUG_ASSERTMESSAGE("More than 3 planes not supported during plane dump.");
            }
        }
    }

    MosUtilities::MosWriteFileFromPtr(sOsPath, pDst, dwSize);

    return(DumpSurfaceToFileEnd(
        pOsInterface,
        pDst,
        isSurfaceLocked,
        pLockedResource,
        temp2DSurfForCopy));
}

const char * CommonDumperTool::GetFormatStr(MOS_FORMAT format)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

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

void CommonDumperTool::StringToLower(
    char* pcString)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

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

void CommonDumperTool::GetOsFilePath(
    const char* pcFilePath,
    char*       pOsFilePath)
{
    MEDIA_DEBUG_FUNCTION_ENTER;

    MOS_SecureMemcpy(pOsFilePath, MAX_PATH, (void*)pcFilePath, strlen(pcFilePath));
}

#endif // (_DEBUG || _RELEASE_INTERNAL)
