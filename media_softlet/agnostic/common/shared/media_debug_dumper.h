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
//! \file     media_debug_dumper.h
//! \brief    Defines structures and functions of common dumpers for debugging
//!           This file contains the definition of structures and functions for
//!           surface dumper
//!

#ifndef __MEDIA_DEBUG_DUMPER_H__
#define __MEDIA_DEBUG_DUMPER_H__

#if (_DEBUG || _RELEASE_INTERNAL)

#include "renderhal.h"
#include "mhw_vebox.h"
#include "mos_os.h"
#include "media_common_defs.h"

#define USE_MEDIA_DEBUG_TOOL 1
#include "media_debug_utils.h"

#if !defined(LINUX) && !defined(ANDROID)
#include "UmdStateSeparation.h"
#endif

#define COMMON_DUMP_OUTPUT_FOLDER                           "\\Commondump\\"

struct COMMON_SURF_DUMP_SURFACE_DEF
{
    uint32_t   dwOffset;                                    //!< Offset from start of the plane
    uint32_t   dwHeight;                                    //!< Height in rows
    uint32_t   dwWidth;                                     //!< Width in bytes
    uint32_t   dwPitch;                                     //!< Pitch in bytes
};

enum COMMON_SURF_DUMP_LOCATION
{
    COMMON_DUMP_TYPE_PRE_ALL,
    COMMON_DUMP_TYPE_PRE_DNDI,
    COMMON_DUMP_TYPE_POST_DNDI,
    COMMON_DUMP_TYPE_PRE_COMP,
    COMMON_DUMP_TYPE_POST_COMP,
    COMMON_DUMP_TYPE_PRE_MEMDECOMP,
    COMMON_DUMP_TYPE_POST_MEMDECOMP,
    COMMON_DUMP_TYPE_VEBOX_DRIVERHEAP,
    COMMON_DUMP_TYPE_VEBOX_KERNELHEAP,
    COMMON_DUMP_TYPE_POST_ALL,
    COMMON_DUMP_TYPE_INTERNAL
};

typedef enum _MCPY_DIRECTION
{
    mcpy_in = 1,
    mcpy_out
} MCPY_DIRECTION;

typedef enum _COMMON_SURFACE_TYPE
{
    COMMON_SURF_NONE = 0,
    COMMON_SURF_IN_BACKGROUND,
    COMMON_SURF_IN_PRIMARY,
    COMMON_SURF_IN_SUBSTREAM,
    COMMON_SURF_IN_REFERENCE,
    COMMON_SURF_OUT_RENDERTARGET,
    COMMON_SURF_TYPE_COUNT  //!< Keep this line at the end
} COMMON_SURFACE_TYPE;
C_ASSERT(COMMON_SURF_TYPE_COUNT == 6);  //!< When adding, update assert & vphal_solo_scenario.cpp

struct COMMON_SURF_DUMP_LOC
{
    uint32_t                        DumpLocation;           //!< Dump location
    COMMON_SURFACE_TYPE             SurfType;               //!< Type of this surface
};

struct COMMON_SURF_DUMP_SPEC
{
    char                        pcOutputPath[MAX_PATH];     //!< Path where dumps are written
    COMMON_SURF_DUMP_LOC       *pDumpLocations;
    uint32_t                    uiStartFrame;               //!< Frame to start dumping at
    uint32_t                    uiEndFrame;                 //!< Frame to stop dumping at
    int32_t                     iNumDumpLocs;               //!< Number of pipe stage dump locations
    bool                        enableAuxDump;              //!< Enable aux data dump for compressed surface
    bool                        enablePlaneDump;            //!< Enable surface dump by plane
};

class CommonSurfaceDumper
{
public:
    CommonSurfaceDumper(PMOS_INTERFACE pOsInterface);

    virtual ~CommonSurfaceDumper();

    virtual MOS_STATUS DumpSurfaceToFile(
        PMOS_INTERFACE              pOsInterface,
        PMOS_SURFACE                pSurface,
        char                       *psPathPrefix,
        uint64_t                    iCounter,
        bool                        bLockSurface,
        bool                        bNoDecompWhenLock,
        uint8_t*                    pData);

    MOS_STATUS GetPlaneDefs(
        PMOS_SURFACE                        pSurface,
        COMMON_SURF_DUMP_SURFACE_DEF*       pPlanes,
        uint32_t*                           pdwNumPlanes,
        uint32_t*                           pdwSize,
        bool                                auxEnable,
        bool                                isDeswizzled);

    bool HasAuxSurf(
        PMOS_RESOURCE               osResource);

    MOS_STATUS GetSurfaceDumpLocation(
        char*          dumpLoc,
        MCPY_DIRECTION mcpyDirection);

    char* WhitespaceTrim(
        char*          ptr);

    MOS_STATUS SurfTypeStringToEnum(
        char*                       pcSurfType,
        COMMON_SURFACE_TYPE          *pSurfType);

    MOS_STATUS LocStringToEnum(
        char*                       pcLocString,
        uint32_t                    *pLocation);

    MOS_STATUS ProcessDumpLocations(
        char*                      pcDumpLocData);

    MOS_STATUS ReAllocateSurface(
        PMOS_INTERFACE  pOsInterface,
        PMOS_SURFACE    pSurface,
        PMOS_SURFACE    pSrcSurf,
        PCCHAR          pSurfaceName,
        MOS_GFXRES_TYPE defaultResType,
        bool            useLinearResource);

    bool IsSyncFreeNeededForMMCSurface(
        PMOS_INTERFACE  osInterface, 
        PMOS_SURFACE    surface);

    MOS_STATUS DumpSurfaceToFileEnd(
        PMOS_INTERFACE  pOsInterface,
        uint8_t        *pDst,
        bool            isSurfaceLocked,
        PMOS_RESOURCE   pLockedResource,
        PMOS_SURFACE    pSurface);

    PMOS_INTERFACE                  m_osInterface;
    COMMON_SURF_DUMP_SPEC           m_dumpSpec;
    MediaUserSettingSharedPtr       m_userSettingPtr = nullptr;       // userSettingInstance
    int32_t                         m_frameNum = 0;

MEDIA_CLASS_DEFINE_END(CommonSurfaceDumper)
};

class CommonDumperTool
{
public:
    static void GetOsFilePath(
        const char*                 pcFilePath,
        char*                       pOsFilePath);

    static void StringToLower(
        char*                       pcString);

    static const char * GetFormatStr(
        MOS_FORMAT                  format);

MEDIA_CLASS_DEFINE_END(CommonDumperTool)
};

#endif // (!(_DEBUG || _RELEASE_INTERNAL) || EMUL)

#endif // __MEDIA_DEBUG_DUMPER_H__
