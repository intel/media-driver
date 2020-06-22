/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file     vp_dumper.h
//! \brief    Defines structures and functions of vp dumpers for debugging
//!           This file contains the definition of structures and functions for
//!           surface dumper and parameter dumper
//!

#ifndef __VP_DUMPER_H__
#define __VP_DUMPER_H__

#if (_DEBUG || _RELEASE_INTERNAL)

#include "renderhal.h"
#include "mhw_vebox.h"
#include "vphal_common.h"       // Common interfaces and structures

#if !defined(LINUX) && !defined(ANDROID)
#include "UmdStateSeparation.h"
#endif



#define MAX_NAME_LEN            100

#define VPHAL_DUMP_OUTPUT_FOLDER                "\\vphaldump\\"

#define VPHAL_SURF_DUMP_MANUAL_TRIGGER_DEFAULT_NOT_SET (-1)
#define VPHAL_SURF_DUMP_MANUAL_TRIGGER_STARTED (1)
#define VPHAL_SURF_DUMP_MANUAL_TRIGGER_STOPPED (0)

//------------------------------------------------------------------------------
// Dump macro.  Simply calls the dump function.  defined as null in production
//------------------------------------------------------------------------------
#define VPHAL_SURFACE_DUMP(dumper, surf, frameCntr, layerCntr, loc)         \
    VPHAL_DEBUG_CHK_STATUS(dumper->DumpSurface(surf, frameCntr, layerCntr, loc));

//------------------------------------------------------------------------------
// Dump array of surfaces
//------------------------------------------------------------------------------
#define VPHAL_SURFACE_PTRS_DUMP(                                            \
    dumper, surfs, maxCntr, numCntr, frameCntr, loc)                            \
    VPHAL_DEBUG_CHK_STATUS(dumper->DumpSurfaceArray(                            \
        surfs, maxCntr, numCntr, frameCntr, loc));

//------------------------------------------------------------------------------
// Create macro for dumper.  Allocates and initializes.
//    Potential leak if renderer not destroyed properly. However, cannot add a
//    free here since renderer is not initialized to null (0)
//------------------------------------------------------------------------------
#define VPHAL_SURF_DUMP_CREATE()                                            \
    m_surfaceDumper = MOS_New(VpSurfaceDumper, m_osInterface);              \
    if (m_surfaceDumper)                                                        \
        m_surfaceDumper->GetSurfaceDumpSpec();

//------------------------------------------------------------------------------
// Destroy macro for dumper.  Frees and sets to null.
//------------------------------------------------------------------------------
#define VPHAL_SURF_DUMP_DESTORY(surfaceDumper)                              \
    MOS_Delete(surfaceDumper);                                                  \
    surfaceDumper = nullptr;


//------------------------------------------------------------------------------
// Create macro for vphal parameters dumper.  Allocates and initializes.
//------------------------------------------------------------------------------
#define VPHAL_PARAMETERS_DUMPPER_CREATE()                                   \
    m_parameterDumper = MOS_New(VpParameterDumper, m_osInterface);          \
    if (m_parameterDumper)                                                      \
        m_parameterDumper->GetParametersDumpSpec();

//------------------------------------------------------------------------------
// Destroy macro for dumper.  Frees and sets to null.
//------------------------------------------------------------------------------
#define VPHAL_PARAMETERS_DUMPPER_DESTORY(parameterDumper)                 \
    MOS_Delete(parameterDumper);                                              \
    parameterDumper = nullptr;

//------------------------------------------------------------------------------
// Dump macro for dumper.  Dump vphal parameters.
//------------------------------------------------------------------------------
#define VPHAL_PARAMETERS_DUMPPER_DUMP_XML(pRenderParams)                    \
    m_parameterDumper->DumpToXML(                                               \
        uiFrameCounter,                                                         \
        m_surfaceDumper->m_dumpSpec.pcOutputPath,                               \
        pRenderParams);

//! 
//! Structure VPHAL_DBG_SURF_DUMP_SURFACE_DEF
//! \brief    Plane definition
//! \details  Plane information including offset, height, width, pitch
//!
struct VPHAL_SURF_DUMP_SURFACE_DEF
{
    uint32_t   dwOffset;                                                           //!< Offset from start of the plane
    uint32_t   dwHeight;                                                           //!< Height in rows
    uint32_t   dwWidth;                                                            //!< Width in bytes
    uint32_t   dwPitch;                                                            //!< Pitch in bytes
};

//! 
//! \brief Dump locations as enum
//!
enum VPHAL_SURF_DUMP_LOCATION
{
    VPHAL_DUMP_TYPE_PRE_ALL,
    VPHAL_DUMP_TYPE_PRE_DNDI,
    VPHAL_DUMP_TYPE_POST_DNDI,
    VPHAL_DUMP_TYPE_PRE_COMP,
    VPHAL_DUMP_TYPE_POST_COMP,
    VPHAL_DUMP_TYPE_PRE_MEMDECOMP,
    VPHAL_DUMP_TYPE_POST_MEMDECOMP,
    VPHAL_DUMP_TYPE_POST_ALL
};

//!
//! Structure VPHAL_SURF_DUMP_LOC
//! \brief    Specification for a single pipeline location dump
//! \details  Specification for a single pipeline location dump
//!
struct VPHAL_SURF_DUMP_LOC
{
    uint32_t                        DumpLocation;                               //!< Dump location
    VPHAL_SURFACE_TYPE              SurfType;                                   //!< Type of this surface
};

//!
//! Structure VPHAL_SURF_DUMP_SPEC
//! \brief    All information about a surface dump specification
//! \details  All information about a surface dump specification
//!
struct VPHAL_SURF_DUMP_SPEC
{
    char                          pcOutputPath[MAX_PATH];                       //!< Path where dumps are written
    VPHAL_SURF_DUMP_LOC       *pDumpLocations;                              //!< Locations in post-processing pipeline to dump at
    uint32_t                      uiStartFrame;                                 //!< Frame to start dumping at
    uint32_t                      uiEndFrame;                                   //!< Frame to stop dumping at
    int32_t                       iNumDumpLocs;                                 //!< Number of pipe stage dump locations
    bool                          enableAuxDump;                                //!< Enable aux data dump for compressed surface
    bool                          enablePlaneDump;                              //!< Enable surface dump by plane
};

//!
//! Structure PVPHAL_DBG_PFIELD_LAYOUT
//! \brief    Debug Field Structure
//! \details  Debug Field Structure
//!
struct VPHAL_FIELD_LAYOUT
{
    char                     pcName[MAX_NAME_LEN];
    uint32_t                 dwOffset;
    uint32_t                 dwSize;
    uint32_t                 uiNumber;
    char                     pcStructName[MAX_NAME_LEN];
    VPHAL_FIELD_LAYOUT   *pChildLayout;
    uint32_t                 uiNumChildren;

};


//-------------------------------------------------------------------------------
// All information about parameters output dump
//-------------------------------------------------------------------------------
struct VPHAL_PARAMS_DUMP_SPEC
{
    char                       outFileLocation[MAX_PATH];                         // Location where dump files need to be stored
    uint32_t                   uiStartFrame;                                      // Start frame for dumping
    uint32_t                   uiEndFrame;                                        // End Frame for dumping
};

//==<FUNCTIONS>=================================================================
//!
//! Class VpSurfaceDumper
//! \brief VP Surface Dumper definition
//!
class VpSurfaceDumper
{
public:
    //!
    //! \brief    Dump a surface into a file
    //! \details  Dump a surface into a file, called by VpHalDbg_SurfaceDumperDump
    //!           File name will be generated based on surface format, width,
    //!           height, pitch, iCounter and psPathPrefix. For example, if the
    //!           psPathPrefix is /dump/primary, then file will be created under
    //!           /dump/ and file name will start as primary_
    //! \param    [in] pOsInterface
    //!           The pointer to OS interface
    //! \param    [in] pSurface
    //!           The pointer to the surface to be dumped
    //! \param    [in] psPathPrefix
    //!           The prefix of the file path which the surface will dump to
    //! \param    [in] iCounter
    //!           The counter that tells which render this surface belongs to
    //! \param    [in] bLockSurface
    //!           True if need to lock the surface
    //! \param    [in] bNoDecompWhenLock
    //!           True if force not to do decompress when Lock
    //! \param    [in] pData
    //!           non-null if caller already locked the surface byitself
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS DumpSurfaceToFile(
        PMOS_INTERFACE              pOsInterface,
        PVPHAL_SURFACE              pSurface,
        const char                  *psPathPrefix,
        uint64_t                    iCounter,
        bool                        bLockSurface,
        bool                        bNoDecompWhenLock,
        uint8_t*                    pData);

    VPHAL_SURF_DUMP_SPEC    m_dumpSpec;

    //!
    //! \brief    VpSurfaceDumper constuctor
    //!
    VpSurfaceDumper(PMOS_INTERFACE pOsInterface)
    :   m_dumpSpec(),
        m_osInterface(pOsInterface)
    {
    };

    //!
    //! \brief    VpSurfaceDumper destuctor
    //!
    virtual ~VpSurfaceDumper();

    //!
    //! \brief    Dump a surface
    //! \details  Dump a surface according to the surface dumper spec
    //! \param    [in] pSurf
    //!           The pointer to the surface to be dumped
    //! \param    [in] uiFrameNumber
    //!           The counter that tells which render this surface belongs to
    //! \param    [in] uiCounter
    //!           The counter that tells which layer this surface belongs to
    //! \param    [in] Location
    //!           The render stage, e.g. preall, predndi, postdndi ...
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS DumpSurface(
        PVPHAL_SURFACE                  pSurf,
        uint32_t                        uiFrameNumber,
        uint32_t                        uiCounter,
        uint32_t                        Location);

    // VpHalDbg_SurfaceDumperDumpPtrs
    //!
    //! \brief    Dump all surfaces in surface array
    //! \details  Dump all surfaces in surface array according to the surface
    //!           dumper spec
    //! \param    [in] ppSurfaces
    //!           The pointer to surface array to be dumped
    //! \param    [in] uiMaxSurfaces
    //!           The max number of surfaces supported in VPHAL
    //! \param    [in] uiNumSurfaces
    //!           The number of surfaces to be dumped
    //! \param    [in] uint32_t uiFrameNumber
    //!           The counter that tells which render this surface belongs to
    //! \param    [in] Location
    //!           The render stage, e.g. preall, predndi, postdndi ...
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpSurfaceArray(
        PVPHAL_SURFACE                  *ppSurfaces,
        uint32_t                        uiMaxSurfaces,
        uint32_t                        uiNumSurfaces,
        uint32_t                        uiFrameNumber,
        uint32_t                        Location);

    //!
    //! \brief    Query the register to get surface dump specification.
    //! \return   void
    //!
    void GetSurfaceDumpSpec();

protected:
    //!
    //! \brief    Convert a string to loc enum type
    //! \param    [in] pcLocString
    //!           Pointer to the string to be converted
    //! \param    [out] pLocation
    //!           Enum type
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS LocStringToEnum(
        char*                           pcLocString,
        uint32_t                        *pLocation);

    //!
    //! \brief    Converts an enum for loc type to a string
    //! \param    [in] Location
    //!           Enum type to be converted
    //! \param    [out] pcLocString
    //!           Location as a string -- must be allocated before sent in
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS EnumToLocString(
        uint32_t                        Location,
        char*                           pcLocString);

    //!
    //! \brief    Check if an osResource have aux surf
    //! \param    [in] osResource
    //!           Pointer to MOS Resource
    //! \return   bool
    //!           Return true if has aux surf, otherwise false
    //!
    bool HasAuxSurf(
        PMOS_RESOURCE                   osResource);

    PMOS_INTERFACE              m_osInterface;
    char                        m_dumpPrefix[MAX_PATH];     // Called frequently, so avoid repeated stack resizing with member data
    char                        m_dumpLoc[MAX_PATH];        // to avoid recursive call from diff owner but sharing the same buffer

private:

    //!
    //! \brief    Get plane information of a surface
    //! \details  Get plane information of a surface, e.g. offset of each plane,
    //!           number of planes, total size of the surface
    //! \param    [in] pSurface
    //!           The pointer to the surface
    //! \param    [out] pPlanes
    //!           The pointer to the plane information of the surface
    //! \param    [out] pdwNumPlanes
    //!           Number of planes of the surface
    //! \param    [out] pdwSize
    //!           The total size of the surface
    //! \param    [in] auxEnable
    //!           Whether aux dump is enabled
    //! \param    [in] isDeswizzled
    //!           Whether deswizzleing is considered. If yes, uv offset should remove paddings
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS GetPlaneDefs(
        PVPHAL_SURFACE                      pSurface,
        VPHAL_SURF_DUMP_SURFACE_DEF        *pPlanes,
        uint32_t*                           pdwNumPlanes,
        uint32_t*                           pdwSize,
        bool                                auxEnable,
        bool                                isDeswizzled);

    //!
    //! \brief    Parse dump location
    //! \details  Take dump location strings and break down into individual post-
    //!           processing pipeline locations and surface types.
    //! \param    [in] pcDumpLocData
    //!           String containing all dump locations
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS ProcessDumpLocations(
        char*                      pcDumpLocData);

    //!
    //! \brief    Convert a string to surf enum type
    //! \param    [in] pcSurfType
    //!           The pointer to string to be converted
    //! \param    [out] pSurfType
    //!           Enum version of string
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SurfTypeStringToEnum(
        char*                       pcSurfType,
        VPHAL_SURFACE_TYPE          *pSurfType);

    //!
    //! \brief    Removes white space from a C-string
    //! \param    [in] ptr
    //!           String to be trimmed
    //! \return   char*
    //!           String after trim
    //!
    char* WhitespaceTrim(
        char*                       ptr);
};


//!
//! Class VpPArameerDumper
//! \brief RenderParameter Dumper definition
//!
class VpParameterDumper
{
public:
    VPHAL_PARAMS_DUMP_SPEC  m_dumpSpec;

    //!
    //! \brief    VphalParameterDumper constuctor
    //!
    VpParameterDumper(PMOS_INTERFACE pOsInterface)
    :   m_dumpSpec(),
        m_osInterface(pOsInterface)
    {
    };

    //!
    //! \brief    Get VPHAL Parameters Dump Spec
    //! \param    [in] pDumpSpec
    //!           parameter dump spec
    //! \param    [in] pOsInterface
    //!           OS interface
    //! \return   void
    //!
    void GetParametersDumpSpec();

    //!
    //! \brief    VphalParameterDumper destuctor
    //!
    virtual ~VpParameterDumper();

    //!
    //! \brief    Dumps the Render Parameters to XML File
    //! \param    [in] uiFrameCounter
    //!           frame counter
    //! \param    [in] pcOutputPath
    //!           Surface dump output path
    //! \param    [in] pRenderParams
    //!           Render parameter to be dumped
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS DumpToXML(
        uint32_t                    uiFrameCounter,
        char                        *pcOutputPath,
        PVPHAL_RENDER_PARAMS        pRenderParams);

protected:
    //!
    //! \brief    Dumps the source Surface Parameters
    //! \param    [in] uiFrameCounter
    //!           frame counter
    //! \param    [in] pcOutputPath
    //!           Surface dump output path
    //! \param    [in] pSrc
    //!           source Surface to be dumped
    //! \param    [in] index
    //!           index of source Surface
    //! \param    [in/out] pcOutContents
    //!           output string buffer
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS DumpSourceSurface(
        uint32_t                        uiFrameCounter,
        char                            *pcOutputPath,
        PVPHAL_SURFACE                  pSrc,
        uint32_t                        index,
        char*                           &pcOutContents);

    //!
    //! \brief    Dumps the target Surface Parameters
    //! \param    [in] uiFrameCounter
    //!           frame counter
    //! \param    [in] pcOutputPath
    //!           Surface dump output path
    //! \param    [in] pSrc
    //!           target Surface to be dumped
    //! \param    [in] index
    //!           index of target Surface
    //! \param    [in/out] pcOutContents
    //!           output string buffer
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS DumpTargetSurface(
        uint32_t                        uiFrameCounter,
        char                            *pcOutputPath,
        PVPHAL_SURFACE                  pTarget,
        uint32_t                        index,
        char*                           &pcOutContents);

    //!
    //! \brief    Dumps the target Surface Parameters
    //! \param    [in] uiFrameCounter
    //!           frame counter
    //! \param    [in] pcOutputPath
    //!           Surface dump output path
    //! \param    [in] pRenderParams
    //!           Render parameter to be dumped
    //! \param    [in/out] pcOutContents
    //!           output string buffer
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS DumpRenderParameter(
        uint32_t                        uiFrameCounter,
        char                            *pcOutputPath,
        PVPHAL_RENDER_PARAMS            pRenderParams,
        char*                           &pcOutContents);

    //!
    //! \brief    Gets Debug Component String
    //! \param    [in] component
    //!           Mos component
    //! \return   const char *
    //!           String of the Component
    //!
    virtual const char * GetComponentStr(
        MOS_COMPONENT             component);

private:
    PMOS_INTERFACE  m_osInterface;

    //!
    //! \brief    Gets Debug Whole Format String
    //! \param    [in] format
    //!           Mos format
    //! \return   const char *
    //!           String of the whole format sucha as Format_NV12
    //!
    const char * GetWholeFormatStr(MOS_FORMAT format);

    //!
    //! \brief    Gets Debug Tile Type String
    //! \param    [in] format
    //!           Mos tile type
    //! \return   const char *
    //!           String of the tile type
    //!
    const char * GetTileTypeStr(MOS_TILE_TYPE tile_type);

    //!
    //! \brief    Gets Debug Surface Type String
    //! \param    [in] component
    //!           Mos component
    //! \return   const char *
    //!           String of the component
    //!
    const char * GetSurfaceTypeStr(VPHAL_SURFACE_TYPE surface_type);

    //!
    //! \brief    Gets Debug Sample Type String
    //! \param    [in] sample type
    //!           vphal sample type
    //! \return   const char *
    //!           String of the vphal sample type
    //!
    const char * GetSampleTypeStr(VPHAL_SAMPLE_TYPE sample_type);

    //!
    //! \brief    Gets Debug Color Type String
    //! \param    [in] Color type
    //!           vphal color type
    //! \return   const char *
    //!           String of the vphal color type
    //!
    const char * GetColorSpaceStr(VPHAL_CSPACE color_space);

    //!
    //! \brief    Gets Debug Blend Type String
    //! \param    [in] Blend type
    //!           vphal blend type
    //! \return   const char *
    //!           String of the vphal color type
    //!
    const char * GetBlendTypeStr(VPHAL_BLEND_TYPE blend_type);

    //!
    //! \brief    Gets Debug Palette Type String
    //! \param    [in] palette type
    //!           vphal palette type
    //! \return   const char *
    //!           String of vphal palette type
    //!
    const char * GetPaletteTypeStr(VPHAL_PALETTE_TYPE palette_type);

    //!
    //! \brief    Gets Debug Scaling Mode String
    //! \param    [in] scaling mode
    //!           vphal scaling mode
    //! \return   const char *
    //!           String of vphal scaling mode
    //!
    const char * GetScalingModeStr(VPHAL_SCALING_MODE scaling_mode);

    //!
    //! \brief    Gets Debug Rotation Mode String
    //! \param    [in] rotation mode
    //!           vphal rotation mode
    //! \return   const char *
    //!           String of vphal rotation mode
    //!
    const char * GetRotationModeStr(VPHAL_ROTATION rotation_mode);

    //!
    //! \brief    Gets Debug Deinterlace Mode String
    //! \param    [in] deinterlace mode
    //!           vphal deinterlace mode
    //! \return   const char *
    //!           String of vphal deinterlace mode
    //!
    const char * GetDIModeStr(VPHAL_DI_MODE di_mode);

    //!
    //! \brief    Gets Debug Denoise Mode String
    //! \param    [in] denoise level
    //!           vphal denoise level
    //! \return   const char *
    //!           String of vphal denoise level
    //!
    const char * GetDenoiseModeStr(VPHAL_NOISELEVEL noise_level);
};

//!
//! Class VpDumperTool
//! \brief Dumper Tool class definition
//!
class VpDumperTool
{
public:
    //!
    //! \brief    Get a file path in current OS
    //! \details  Covert a file path to a path that can be recognized in current OS.
    //!           File path may be different in different OS.
    //! \param    [in] pcFilePath
    //            Input string of the file path to be converted
    //! \param    [out] pOsFilePath
    //!           Converted file path
    //! \return   void
    //!
    static void GetOsFilePath(
        const char* pcFilePath,
        char*       pOsFilePath);

    //!
    //! \brief    Convert a string to an all lower case version
    //! \param    [in/out] pcString
    //!           The pointer to string to be converted
    //! \return   void
    //!
    static void StringToLower(
        char*                       pcString);

    //!
    //! \brief    Append to the string
    //! \details  Adds second arg to end of first arg.  Works like sprintf, but
    //!           optimized for this special use.
    //! \param    [in] bFirst
    //!           True if this is the first call to this function for next param.
    //!           Note that multiple strings cannot be appended at the same time.
    //!           They should be appended one by one, e.g. pcBigString2 can be appended
    //!           after you finish appending pcBigString1.
    //! \param    [in/out] ppcBigString
    //!           Pointer to string for that is appended to
    //! \param    [in] pcToAppendFmt
    //!           String format for appended part
    //! \param    [in] ...
    //!           Fields to fill in for appended part
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS AppendString(
        bool   bFirst,
        char   **ppcBigString,
        PCCHAR pcToAppendFmt,
        ...);

    //!
    //! \brief    Write Frame
    //! \details  Debug function to write intermediate output to C:\dump
    //! \param    [in] pOsInterface
    //!           Pointer to OS interface
    //! \param    [in] pSurface
    //!           Pointer to surface that will be read from
    //! \param    [in] fileName
    //!           File to be written to
    //! \param    [in] iCounter
    //!           Frame ID and cycle counter
    //! \return   void
    //!
    static void WriteFrame(
        PMOS_INTERFACE  pOsInterface,
        PVPHAL_SURFACE  pSurface,
        PCCHAR          fileName,
        uint64_t        iCounter);

    //!
    //! \brief    Returns the Size of the surface in bytes
    //! \param    [in] pSurface
    //!           The pointer to the surface
    //! \param    [int] iBpp
    //!           Number of bits per pixel
    //! \param    [out] piWidthInBytes
    //!           Width of the surface
    //! \param    [out] piHeightInRows
    //!           Height of the surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS GetSurfaceSize(
        PVPHAL_SURFACE              pSurface,
        uint32_t                    iBpp,
        uint32_t*                   piWidthInBytes,
        uint32_t*                   piHeightInRows);

    //!
    //! \brief    Gets Debug Format String
    //! \param    [in] format
    //!           Mos format
    //! \return   const char *
    //!           String of the format
    //!
    static const char * GetFormatStr(
        MOS_FORMAT                format);

};
#endif // (_DEBUG || _RELEASE_INTERNAL)

#if (!(_DEBUG || _RELEASE_INTERNAL))

#define VPHAL_SURFACE_DUMP(dumper, surf, frameCntr, layerCntr, loc)
#define VPHAL_SURFACE_PTRS_DUMP(                                            \
    dumper, surfs, maxCntr, numCntr, frameCntr, loc)
#define VPHAL_SURF_DUMP_CREATE()
#define VPHAL_SURF_DUMP_DESTORY(surfaceDumper)
#define VPHAL_PARAMETERS_DUMPPER_CREATE()
#define VPHAL_PARAMETERS_DUMPPER_DESTORY(pParametersDumpSpec)
#define VPHAL_PARAMETERS_DUMPPER_DUMP_XML(pRenderParams)

#endif // (!(_DEBUG || _RELEASE_INTERNAL) || EMUL)




#endif // __VP_DUMPER_H__