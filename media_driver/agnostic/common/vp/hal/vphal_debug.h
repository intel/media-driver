/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file      vphal_debug.h 
//! \brief 
//!
//!
//! \file     vphal_debug.h
//! \brief    Definition of structures and functions for debugging VPHAL
//! \details  This file contains the definition of structures and functions for
//!           surface dumper, hw state dumper, perf counter dumper, and render
//!           parameter dumper
//!
#ifndef __VPHAL_DEBUG_H__
#define __VPHAL_DEBUG_H__

#if (_DEBUG || _RELEASE_INTERNAL)

#include "renderhal.h"
#include "mhw_vebox.h"
#include "vphal_common.h"       // Common interfaces and structures

#if !defined(LINUX) && !defined(ANDROID)
#include "UmdStateSeparation.h"
#endif

//==<DEFINITIONS>===============================================================
#define MAX_NAME_LEN            100

#define VPHAL_DBG_DUMP_OUTPUT_FOLDER                "\\vphaldump\\"

#define VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_DEFAULT_NOT_SET (-1)
#define VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_STARTED (1)
#define VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_STOPPED (0)

//------------------------------------------------------------------------------
// Dump macro.  Simply calls the dump function.  defined as null in production
//------------------------------------------------------------------------------
#define VPHAL_DBG_SURFACE_DUMP(dumper, surf, frameCntr, layerCntr, loc)         \
    VPHAL_DEBUG_CHK_STATUS(dumper->DumpSurface(surf, frameCntr, layerCntr, loc));

//------------------------------------------------------------------------------
// Dump array of surfaces
//------------------------------------------------------------------------------
#define VPHAL_DBG_SURFACE_PTRS_DUMP(                                            \
    dumper, surfs, maxCntr, numCntr, frameCntr, loc)                            \
    VPHAL_DEBUG_CHK_STATUS(dumper->DumpSurfaceArray(                            \
        surfs, maxCntr, numCntr, frameCntr, loc));

//------------------------------------------------------------------------------
// Create macro for dumper.  Allocates and initializes.
//    Potential leak if renderer not destroyed properly. However, cannot add a
//    free here since renderer is not initialized to null (0)
//------------------------------------------------------------------------------
#define VPHAL_DBG_SURF_DUMP_CREATE()                                            \
    m_surfaceDumper = MOS_New(VphalSurfaceDumper, m_pOsInterface);              \
    if (m_surfaceDumper)                                                        \
        m_surfaceDumper->GetSurfaceDumpSpec();

//------------------------------------------------------------------------------
// Destroy macro for dumper.  Frees and sets to null.
//------------------------------------------------------------------------------
#define VPHAL_DBG_SURF_DUMP_DESTORY(surfaceDumper)                              \
    MOS_Delete(surfaceDumper);                                                  \
    surfaceDumper = nullptr;

#define VPHAL_DBG_STATE_DUMPPER_CREATE()                                        \
    pRenderHal->pStateDumper = MOS_New(VphalHwStateDumper, pRenderHal);         \
    if (pRenderHal->pStateDumper)                                               \
        ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                    \
            GetStateDumpSpec();

#define VPHAL_DBG_STATE_DUMPPER_DESTORY(pStateDumper)                           \
    VphalHwStateDumper::Delete(pStateDumper);                                   \
    pStateDumper = nullptr;

#define VPHAL_DBG_STATE_DUMP_SET_CURRENT_FRAME_COUNT(uiFrameCounter)            \
    ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                        \
        m_dumpSpec.uiCurrentFrame = uiFrameCounter;

#define VPHAL_DBG_STATE_DUMPPER_SET_CURRENT_STAGE(uiCurrentStage)               \
    ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                        \
        iDebugStage = uiCurrentStage;                                           \
    ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                        \
        iPhase = 0;

#define VPHAL_DBG_STATE_DUMPPER_SET_CURRENT_PHASE(uiCurrentPhase)               \
    ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                        \
        iPhase = uiCurrentPhase;

#define VPHAL_DBG_STATE_DUMPPER_DUMP_COMMAND_BUFFER(pRenderHal, pCmdBuffer)     \
    ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                        \
    DumpCommandBuffer(pCmdBuffer);

#define VPHAL_DBG_STATE_DUMPPER_DUMP_GSH(pRenderHal)                            \
    ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                        \
    DumpGSH();

#define VPHAL_DBG_STATE_DUMPPER_DUMP_SSH(pRenderHal)                            \
    ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                        \
    DumpSSH();

#define VPHAL_DBG_STATE_DUMPPER_DUMP_BATCH_BUFFER(pRenderHal, pBatchBuffer)     \
    ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                        \
    DumpBatchBuffer(pBatchBuffer);

#define VPHAL_DBG_STATE_DUMPPER_DUMP_VEBOX_STATES(pRenderHal, pVeboxState)      \
    ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                        \
    DumpVeboxState(pVeboxState);

#define VPHAL_DBG_STATE_DUMPPER_DUMP_VEBOX_STATISTICS(pRenderHal, pVeboxState, pStatSlice0Base, pStatSlice1Base)   \
    ((VphalHwStateDumper*)(pRenderHal->pStateDumper)) ->                        \
    DumpStatistics(pVeboxState, pStatSlice0Base, pStatSlice1Base);

//------------------------------------------------------------------------------
// Create macro for vphal parameters dumper.  Allocates and initializes.
//------------------------------------------------------------------------------
#define VPHAL_DBG_PARAMETERS_DUMPPER_CREATE()                                   \
    m_parameterDumper = MOS_New(VphalParameterDumper, m_pOsInterface);          \
    if (m_parameterDumper)                                                      \
        m_parameterDumper->GetParametersDumpSpec();

//------------------------------------------------------------------------------
// Destroy macro for dumper.  Frees and sets to null.
//------------------------------------------------------------------------------
#define VPHAL_DBG_PARAMETERS_DUMPPER_DESTORY(parameterDumper)                 \
    MOS_Delete(parameterDumper);                                              \
    parameterDumper = nullptr;

//------------------------------------------------------------------------------
// Dump macro for dumper.  Dump vphal parameters.
//------------------------------------------------------------------------------
#define VPHAL_DBG_PARAMETERS_DUMPPER_DUMP_XML(pRenderParams)                    \
    m_parameterDumper->DumpToXML(                                               \
        uiFrameCounter,                                                         \
        m_surfaceDumper->m_dumpSpec.pcOutputPath,                               \
        pRenderParams);

//! 
//! Structure VPHAL_DBG_SURF_DUMP_SURFACE_DEF
//! \brief    Plane definition
//! \details  Plane information including offset, height, width, pitch
//!
struct VPHAL_DBG_SURF_DUMP_SURFACE_DEF
{
    uint32_t   dwOffset;                                                           //!< Offset from start of the plane
    uint32_t   dwHeight;                                                           //!< Height in rows
    uint32_t   dwWidth;                                                            //!< Width in bytes
    uint32_t   dwPitch;                                                            //!< Pitch in bytes
};

//! 
//! \brief Dump locations as enum
//!
enum VPHAL_DBG_SURF_DUMP_LOCATION
{
    VPHAL_DBG_DUMP_TYPE_PRE_ALL,
    VPHAL_DBG_DUMP_TYPE_PRE_DNDI,
    VPHAL_DBG_DUMP_TYPE_POST_DNDI,
    VPHAL_DBG_DUMP_TYPE_PRE_COMP,
    VPHAL_DBG_DUMP_TYPE_POST_COMP,
    VPHAL_DBG_DUMP_TYPE_PRE_MEMDECOMP,
    VPHAL_DBG_DUMP_TYPE_POST_MEMDECOMP,
    VPHAL_DBG_DUMP_TYPE_POST_ALL
};

//!
//! Structure VPHAL_DBG_SURF_DUMP_LOC
//! \brief    Specification for a single pipeline location dump
//! \details  Specification for a single pipeline location dump
//!
struct VPHAL_DBG_SURF_DUMP_LOC
{
    uint32_t                        DumpLocation;                               //!< Dump location
    VPHAL_SURFACE_TYPE              SurfType;                                   //!< Type of this surface
};

//!
//! Structure VPHAL_DBG_SURF_DUMP_SPEC
//! \brief    All information about a surface dump specification
//! \details  All information about a surface dump specification
//!
struct VPHAL_DBG_SURF_DUMP_SPEC
{
    char                          pcOutputPath[MAX_PATH];                       //!< Path where dumps are written
    VPHAL_DBG_SURF_DUMP_LOC       *pDumpLocations;                              //!< Locations in post-processing pipeline to dump at
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
struct VPHAL_DBG_FIELD_LAYOUT
{
    char                     pcName[MAX_NAME_LEN];
    uint32_t                 dwOffset;
    uint32_t                 dwSize;
    uint32_t                 uiNumber;
    char                     pcStructName[MAX_NAME_LEN];
    VPHAL_DBG_FIELD_LAYOUT   *pChildLayout;
    uint32_t                 uiNumChildren;

};

//! 
//! \brief Render stage in VPHAL
//!
enum  VPHAL_DEBUG_STAGE
{
    VPHAL_DBG_STAGE_NULL,
    VPHAL_DBG_STAGE_DNDI,
    VPHAL_DBG_STAGE_VEBOX,
    VPHAL_DBG_STAGE_COMP
};

//! 
//! \brief HW state buffer in VPHAL
//!
enum VPHAL_DBG_DUMP_TYPE
{
    VPHAL_DBG_DUMP_TYPE_GSH,
    VPHAL_DBG_DUMP_TYPE_SSH,
    VPHAL_DBG_DUMP_TYPE_BB,
    VPHAL_DBG_DUMP_TYPE_CB,
};

//!
//! Structure VPHAL_DBG_GSH_DUMP_LOC
//! \brief    Specification for GSH location dump
//! \details  Specification for GSH location dump
//!
struct VPHAL_DBG_GSH_DUMP_LOC
{
    uint32_t            DumpStage;            //!< Which stage the data should be dumped, used for GSH.
};

//!
//! Structure VPHAL_DBG_SSH_DUMP_LOC
//! \brief    Specification for SSH location dump
//! \details  Specification for SSH location dump
//!
struct VPHAL_DBG_SSH_DUMP_LOC
{
    uint32_t            DumpStage;            //!< Which stage the data should be dumped, used for SSH.
};

//!
//! Structure VPHAL_DBG_BB_DUMP_LOC
//! \brief    Specification for BB location dump
//! \details  Specification for BB location dump
//!
struct VPHAL_DBG_BB_DUMP_LOC
{
    uint32_t            DumpStage;            //!< Which stage the data should be dumped, used for batch buffer.
};

//!
//! Structure VPHAL_DBG_CB_DUMP_LOC
//! \brief    Specification for cmd buffer location dump
//! \details  Specification for cmd buffer location dump
//!
struct VPHAL_DBG_CB_DUMP_LOC
{
    uint32_t            DumpStage;            //!< Which stage the data should be dumped, used for Command buffer.
};

//!
//! Structure VPHAL_DBG_VEBOXSTATE_DUMP_LOC
//! \brief    Specification for Vebox state location dump
//! \details  Specification for Vebox state location dump
//!
struct VPHAL_DBG_VEBOXSTATE_DUMP_LOC
{
    uint32_t            DumpStage;            //!< Which stage the data should be dumped, used for Vebox state.
};

//!
//! Structure VPHAL_DBG_STATISTICS_DUMP_LOC
//! \brief    Specification for Statistics location dump
//! \details  Specification for Statistics location dump
//!
struct VPHAL_DBG_STATISTICS_DUMP_LOC
{
    uint32_t            DumpStage;            //!< Which stage the data should be dumped, used for Statistics.
};

//!
//! Structure VPHAL_DBG_GSH_DUMP_SPEC
//! \brief    All information about GSH dump specification
//! \details  All information about GSH dump specification
//!
struct VPHAL_DBG_GSH_DUMP_SPEC
{
    int32_t                         iNumDumpLocs;                                 //!< Number of pipe stage dump locations
    VPHAL_DBG_GSH_DUMP_LOC          *pDumpLocations;                              //!< Locations in post-processing pipeline to dump at
};

//!
//! Structure VPHAL_DBG_SSH_DUMP_SPEC
//! \brief    All information about SSH dump specification
//! \details  All information about SSH dump specification
//!
struct VPHAL_DBG_SSH_DUMP_SPEC
{
    int32_t                         iNumDumpLocs;                                 //!< Number of pipe stage dump locations
    VPHAL_DBG_SSH_DUMP_LOC          *pDumpLocations;                              //!< Locations in post-processing pipeline to dump at
};

//!
//! Structure VPHAL_DBG_BB_DUMP_SPEC
//! \brief    All information about batch buffer dump specification
//! \details  All information about batch buffer dump specification
//!
struct VPHAL_DBG_BB_DUMP_SPEC
{
    int32_t                         iNumDumpLocs;                                 //!< Number of pipe stage dump locations
    VPHAL_DBG_BB_DUMP_LOC           *pDumpLocations;                              //!< Locations in post-processing pipeline to dump at
};

//!
//! Structure PVPHAL_DBG_CB_DUMP_SPEC
//! \brief    All information about cmd buffer dump specification
//! \details  All information about cmd buffer dump specification
//!
struct VPHAL_DBG_CB_DUMP_SPEC
{
    int32_t                         iNumDumpLocs;                                 //!< Number of pipe stage dump locations
    VPHAL_DBG_CB_DUMP_LOC           *pDumpLocations;                              //!< Locations in post-processing pipeline to dump at
};

//!
//! Structure PVPHAL_DBG_VEBOXSTATE_DUMP_SPEC
//! \brief    All information about Vebox state dump specification
//! \details  All information about Vebox state dump specification
//!
struct VPHAL_DBG_VEBOXSTATE_DUMP_SPEC
{
    int32_t                             iNumDumpLocs;                             //!< Number of pipe stage dump locations
    VPHAL_DBG_VEBOXSTATE_DUMP_LOC       *pDumpLocations;                          //!< Locations in post-processing pipeline to dump at
};

//!
//! Structure PVPHAL_DBG_STATISTICS_DUMP_SPEC
//! \brief    All information about Statistics dump specification
//! \details  All information about Statistics dump specification
//!
struct VPHAL_DBG_STATISTICS_DUMP_SPEC
{
    int32_t                             iNumDumpLocs;                             //!< Number of pipe stage dump locations
    VPHAL_DBG_STATISTICS_DUMP_LOC       *pDumpLocations;                          //!< Locations in post-processing pipeline to dump at
};

//!
//! Structure VPHAL_DBG_DUMP_SPEC
//! \brief    All information about state dump specification
//! \details  All information about state dump specification
//!
struct VPHAL_DBG_DUMP_SPEC
{
    char                                   pcOutputPath[MAX_PATH];                //!< Path where dumps are written
    uint64_t                               uiStartFrame;                          //!< Frame to start dumping at
    uint64_t                               uiEndFrame;                            //!< Frame to stop dumping at
    uint32_t                               uiCurrentFrame;
    VPHAL_DBG_GSH_DUMP_SPEC                *pGSHDumpSpec;
    VPHAL_DBG_SSH_DUMP_SPEC                *pSSHDumpSpec;
    VPHAL_DBG_BB_DUMP_SPEC                 *pBBDumpSpec;
    VPHAL_DBG_CB_DUMP_SPEC                 *pCBDumpSpec;
    VPHAL_DBG_VEBOXSTATE_DUMP_SPEC         *pVeboxStateDumpSpec;
    VPHAL_DBG_STATISTICS_DUMP_SPEC         *pStatisticsDumpSpec;
};

//-------------------------------------------------------------------------------
// All information about parameters output dump
//-------------------------------------------------------------------------------
struct VPHAL_DBG_PARAMS_DUMP_SPEC
{
    char                       outFileLocation[MAX_PATH];                         // Location where dump files need to be stored
    uint32_t                   uiStartFrame;                                      // Start frame for dumping
    uint32_t                   uiEndFrame;                                        // End Frame for dumping
};

//==<FUNCTIONS>=================================================================
//!
//! Class VphalSurfaceDumper
//! \brief VPHAL Surface Dumper definition
//!
class VphalSurfaceDumper
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

    VPHAL_DBG_SURF_DUMP_SPEC    m_dumpSpec;

    //!
    //! \brief    VphalSurfaceDumper constuctor
    //!
    VphalSurfaceDumper(PMOS_INTERFACE pOsInterface)
    :   m_dumpSpec(),
        m_osInterface(pOsInterface)
    {
    };

    //!
    //! \brief    VphalSurfaceDumper destuctor
    //!
    virtual ~VphalSurfaceDumper();

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
    static uint32_t             m_frameNumInVp;             // For use when vp dump its compressed surface, override the frame number given from MediaVeboxDecompState
    static char                 m_dumpLocInVp[MAX_PATH];    // For use when vp dump its compressed surface, to distinguish each vp loc's pre/post decomp

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
        VPHAL_DBG_SURF_DUMP_SURFACE_DEF     *pPlanes,
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

typedef class VPHAL_VEBOX_STATE *PVPHAL_VEBOX_STATE;

//!
//! Class VphalHwStateDumper
//! \brief HW State Dumper definition
//!
class VphalHwStateDumper
{
public:
    VPHAL_DBG_DUMP_SPEC         m_dumpSpec;
    int32_t                     iDebugStage;        // dnuv, dndi, compostion ...
    int32_t                     iPhase;

    //!
    //! \brief    VphalHwStateDumper destuctor
    //!
    static void Delete(void * dumper)
    {
        VphalHwStateDumper * hwStateDumper = (VphalHwStateDumper*) dumper;
        MOS_Delete(hwStateDumper);
    };

    //!
    //! \brief    VphalHwStateDumper constuctor
    //!
    VphalHwStateDumper(PRENDERHAL_INTERFACE             pRenderHal)
        :   m_dumpSpec(),
            iDebugStage(0),
            iPhase(0),
            m_renderHal(pRenderHal),
            m_osInterface(pRenderHal->pOsInterface),
            m_hwSizes(pRenderHal->pHwSizes),
            m_stateHeap(pRenderHal->pStateHeap),
            m_stateHeapSettings(&pRenderHal->StateHeapSettings)
    {
    };

    //!
    //! \brief    VphalHwStateDumper destuctor
    //!
    virtual ~VphalHwStateDumper();

    //!
    //! \brief    Query the register to get dump state spec.
    //! \return   void
    //!
    void GetStateDumpSpec();

    //!
    //! \brief    Dumps GSH
    //! \return   void
    //!
    void DumpGSH();

    //!
    //! \brief    Dumps SSH
    //! \return   void
    //!
    void DumpSSH();

    //!
    //! \brief    Dump batch buffer
    //! \param    [in] pBatchBuffer
    //!           Pointer to batch buffer
    //! \return   void
    //!
    void DumpBatchBuffer(
        PMHW_BATCH_BUFFER           pBatchBuffer);

    //!
    //! \brief    Dumps Command buffer
    //! \param    [in] pCommandBuffer
    //!           Pointer to command buffer
    //! \return   void
    //!
    void DumpCommandBuffer(
        PMOS_COMMAND_BUFFER         pCommandBuffer);

    //!
    //! \brief    Dumps Vebox State
    //! \details  Dumps Vebox State
    //! \param    [in] pVeboxState
    //!           Pointer to Vebox state
    //! \return   void
    //!
    void DumpVeboxState(
        PVPHAL_VEBOX_STATE                       pVeboxState);

    //!
    //! \brief    Dumps Statistics
    //! \param    [in] pVeboxState
    //!           Pointer to DNDI state
    //! \param    [in] pStat0Base
    //!           Base address of Statistics 0
    //! \param    [in] pStat1Base
    //!           Base address of Statistics 1
    //! \return   void
    //!
    void DumpStatistics(
        void*                       pVeboxState,
        uint8_t*                    pStat0Base,
        uint8_t*                    pStat1Base);

protected:
    //!
    //! \brief    Converts an enum for stage type to a string
    //! \param    [in] Location
    //!           VPHAL debug stage
    //! \param    [out] pcLocString
    //!           Location as a string -- must be allocated before sent in
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS EnumToLocString(
        VPHAL_DEBUG_STAGE          Location,
        char*                      pcLocString);

    //!
    //! \brief    Convert a string to state enum type
    //! \param    [in] pcStateType
    //!           The pointer to string to be converted
    //! \param    [out] pStateType
    //!           Enum version of string
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS StateTypeStringToEnum(
        char*                         pcStateType,
        uint32_t                      *pStateType);

private:
    PRENDERHAL_INTERFACE            m_renderHal;
    PMOS_INTERFACE                  m_osInterface;
    PMHW_RENDER_STATE_SIZES         m_hwSizes;
    PRENDERHAL_STATE_HEAP           m_stateHeap;
    PRENDERHAL_STATE_HEAP_SETTINGS  m_stateHeapSettings;

    //!
    //! \brief    Take dump location strings and break down into individual 
    //!           post-processing pipeline locations and state types.
    //! \details  Take dump location strings and break down into individual 
    //!           post-processing pipeline locations and state types.
    //! \param    [in] pcDumpLocData
    //!           String containing all dump locations
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS ProcessDumpStateLocations(
        char*                      pcDumpLocData);

    //!
    //! \brief    Dumps binary contents for vebox Statistics
    //! \param    [in] pVeboxState
    //!           Pointer to DNDI state
    //! \param    [in] pStat0Base
    //!           Base address of Statistics 0
    //! \param    [in] pStat1Base
    //!           Base address of Statistics 1
    //! \param    [in] pcFileLoc
    //!           Structure name (becomes part of output file)
    //! \param    [in] iID
    //!           ID for dumping multiple of the same struct in the same place
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpStatisticsBinary(
        PVPHAL_VEBOX_STATE      pVeboxState,
        uint8_t*                pStat0Base,
        uint8_t*                pStat1Base,
        const char*             pcFileLoc,
        int32_t                 iID);

    //!
    //! \brief    Dumps binary contents and XML for GSH
    //! \param    [in] pcFileLoc
    //!           Structure name (becomes part of output file)
    //! \param    [in] iID
    //!           ID for dumping multiple of the same struct in the same place
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpGshBinary(
        const char*          pcFileLoc,
        int32_t              iID);

    //!
    //! \brief    Define GSH layout.
    //! \details  Define GSH layout according to GSH structure
    //! \param    [out] ppGshLayout
    //!           Layout of GSH
    //! \param    [out] puiNumGSHFields
    //!           Number of top-level fields in GSH
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DefGsh(
        VPHAL_DBG_FIELD_LAYOUT*  *ppGshLayout,
        uint32_t*                puiNumGSHFields);

    //!
    //! \brief    Dumps binary contents and XML for SSH
    //! \param    [in] pcFileLoc
    //!           Structure name (becomes part of output file)
    //! \param    [in] iID
    //!           ID for dumping multiple of the same struct in the same place
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpSshBinary(
        const char*          pcFileLoc,
        int32_t              iID);

    //!
    //! \brief    Define SSH layout.
    //! \details  Define SSH layout according to SSH structure
    //! \param    [out] ppSshLayout
    //!           Layout of SSH
    //! \param    [out] puiNumSSHFields
    //!           Number of top-level fields in SSH
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DefSsh(
        VPHAL_DBG_FIELD_LAYOUT*  *ppSshLayout,
        uint32_t*                puiNumSSHFields);

    //!
    //! \brief    Dumps binary contents and XML for BB
    //! \param    [in] pBatchBuffer
    //!           Pointer to buffer
    //! \param    [in] pcFileLoc
    //!           Structure name (becomes part of output file)
    //! \param    [in] iID
    //!           ID for dumping multiple of the same struct in the same place
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpBatchBufferBinary(
        PMHW_BATCH_BUFFER            pBatchBuffer,
        const char*                  pcFileLoc,
        int32_t                      iID);

    //!
    //! \brief    Dumps binary contents and XML for CMB
    //! \param    [in] pCmd_buff
    //!           Command buffer
    //! \param    [in] pcFileLoc
    //!           Structure name (becomes part of output file)
    //! \param    [in] iID
    //!           ID for dumping multiple of the same struct in the same place
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpCommandBufferBinary(
        PMOS_COMMAND_BUFFER          pCmd_buff,
        const char*                  pcFileLoc,
        int32_t                      iID);

    //!
    //! \brief    Dumps binary contents  for VeboxState
    //! \details  Dumps binary contents  for VeboxState
    //! \param    [in] pVeboxInterface
    //!           Pointer to Vebox Interface
    //! \param    [in] pcFileLoc
    //!           Structure name (becomes part of output file)
    //! \param    [in] iID
    //!           ID for dumping multiple of the same struct in the same place
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpVeboxStateBinary(
        PMHW_VEBOX_INTERFACE         pVeboxInterface,
        const char*                  pcFileLoc,
        int32_t                      iID);

    //!
    //! \brief    Dumps binary contents of data structure to a file
    //! \param    [in] fields
    //!           Top level fields (for calculating size)
    //! \param    [in] uiNumFields
    //!           Size of previous array
    //! \param    [in] pvStructToDump
    //!           Pointer to structure in memory
    //! \param    [in] pcPath
    //!           Path of output file
    //! \param    [in] pcName
    //!           Structure name (becomes part of output file)
    //! \param    [in] iID
    //!           ID for dumping multiple of the same struct in the same place
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpBinaryStruct(
        VPHAL_DBG_FIELD_LAYOUT*      fields,
        uint32_t                     uiNumFields,
        void                         *pvStructToDump,
        char*                        pcPath,
        PCCHAR                       pcName,
        int32_t                      iID);

    //!
    //! \brief    Dump a field heap to the string
    //! \details  For num fields specified in parLayout, output XML header/footer
    //!           and call dump_subfields to handle lower hierarchy levels for each
    //!           e.g. N media states
    //! \param    [in,out] ppcOutContents
    //!           Pointer to string for structure descriptions
    //! \param    [in] parLayout
    //!           Pointer to parent layout description
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpFieldHeap(
        char                        **ppcOutContents,
        VPHAL_DBG_FIELD_LAYOUT*     parLayout);

    //!
    //! \brief    Dump sub field to the string
    //! \details  Dump next level in the hierarchy based on its type (struct, etc.)
    //! \param    [in,out] ppcOutContents
    //!           Pointer to string for structure descriptions
    //! \param    [in] pParLayout
    //!           Pointer to parent layout description
    //! \param    [in] pChildLayout
    //!           Pointer to current layout description (child of previous)
    //! \param    [in] uiNumChild
    //!           Number of children
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpSubfields(
        char                        **ppcOutContents,
        VPHAL_DBG_FIELD_LAYOUT*     pParLayout,
        VPHAL_DBG_FIELD_LAYOUT*     pChildLayout,
        uint32_t                    uiNumChild);

    //!
    //! \brief    Dump field to the string
    //! \details  Dump subfields as well as header and footer for pLayout
    //! \param    [in,out] ppcOutContents
    //!           Pointer to string for structure descriptions
    //! \param    [in] pLayout
    //!           Layout of this field
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpField(
        char                        **ppcOutContents,
        VPHAL_DBG_FIELD_LAYOUT*     pLayout);

    //!
    //! \brief    Print xml header
    //! \details  Print xml header based on parameters.
    //! \param    [in,out] ppcOutContents
    //!           Pointer to string for structure descriptions
    //! \param    [in] pcName
    //!           Name of field
    //! \param    [in] ulLoc
    //!           Offset of field from beginning of parent
    //! \param    [in] ulSize
    //!           Size of field in bytes
    //! \param    [in] bIsByte
    //!           True if unit is bytes (loc and size)
    //! \param    [in] bHasCont
    //!           True if has subfields
    //! \param    [in] bStruct
    //!           Has sub-fields/contents
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpXmlFieldHeader(
        char                        **ppcOutContents,
        char                        *pcName,
        uint32_t                    ulLoc,
        uint32_t                    ulSize,
        bool                        bIsByte,
        bool                        bHasCont,
        bool                        bStruct);

    //!
    //! \brief    Print xml footer.
    //! \param    [in,out] ppcOutContents
    //!           Pointer to string for structure descriptions
    //! \param    [in] bHasCont
    //!           Has sub-fields/contents
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpXmlFieldFooter(
        char                        **ppcOutContents,
        bool                        bHasCont);

    //!
    //! \brief    Dump a dword to the string
    //! \details  For each uint32_t, i, specified by dwSizeField, output XML for DWi.
    //! \param    [in,out] ppcOutContents
    //!           Pointer to string for structure descriptions
    //! \param    [in] dwSizeField
    //!           Size of field in bytes
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DumpDwords(
        char                        **ppcOutContents,
        uint32_t                    dwSizeField);

    //!
    //! \brief    Frees layout memory
    //! \details  Frees layout memory as well as all subfields recursively
    //! \param    [in] pLayout
    //!           Layout to free
    //! \param    [in] uiNumFields
    //!           Number of fields in layout
    //! \return   void
    //!
    void FreeLayout(
        VPHAL_DBG_FIELD_LAYOUT*     pLayout,
        uint32_t                    uiNumFields);

    //!
    //! \brief    Checks for a properly formed path and fixes errors if it can
    //! \param    [in/out] ppcPath
    //!           Pointer to string of file path
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS CheckPath(
        char                        **ppcPath);

    //!
    //! \brief    Get last field index
    //! \details  Finds the index info fl[] that has the highest offset
    //! \param    [in] fl[]
    //!           Pointer to array of fields layouts
    //! \param    [in] uiNumFields
    //!           Number of fields
    //! \param    [out] puiLastIndex
    //!           Pointer to int which holds index of last field (last according to offset)
    //! \return   void
    //!
    void GetLastFieldIndex(
        VPHAL_DBG_FIELD_LAYOUT      fl[],
        uint32_t                    uiNumFields,
        uint32_t                    *uiLastIndex);

};

//!
//! Class VphalPArameerDumper
//! \brief RenderParameter Dumper definition
//!
class VphalParameterDumper
{
public:
    VPHAL_DBG_PARAMS_DUMP_SPEC  m_dumpSpec;

    //!
    //! \brief    VphalParameterDumper constuctor
    //!
    VphalParameterDumper(PMOS_INTERFACE pOsInterface)
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
    virtual ~VphalParameterDumper();

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
//! Class VphalDumperTool
//! \brief Dumper Tool class definition
//!
class VphalDumperTool
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

#define VPHAL_DBG_SURFACE_DUMP(dumper, surf, frameCntr, layerCntr, loc)
#define VPHAL_DBG_SURFACE_PTRS_DUMP(                                            \
    dumper, surfs, maxCntr, numCntr, frameCntr, loc)
#define VPHAL_DBG_SURF_DUMP_CREATE()
#define VPHAL_DBG_SURF_DUMP_DESTORY(surfaceDumper)
#define VPHAL_DBG_STATE_DUMPPER_CREATE()
#define VPHAL_DBG_STATE_DUMPPER_DESTORY(pStateDumper)
#define VPHAL_DBG_STATE_DUMP_SET_CURRENT_FRAME_COUNT(uiFrameCounter)
#define VPHAL_DBG_STATE_DUMPPER_SET_CURRENT_STAGE(uiCurrentStage)
#define VPHAL_DBG_STATE_DUMPPER_SET_CURRENT_PHASE(uiCurrentPhase)
#define VPHAL_DBG_STATE_DUMPPER_DUMP_COMMAND_BUFFER(pRenderHal, pCmdBuffer)
#define VPHAL_DBG_STATE_DUMPPER_DUMP_GSH(pRenderHal)
#define VPHAL_DBG_STATE_DUMPPER_DUMP_SSH(pRenderHal)
#define VPHAL_DBG_STATE_DUMPPER_DUMP_BATCH_BUFFER(pRenderHal, pBatchBuffer)
#define VPHAL_DBG_STATE_DUMPPER_DUMP_VEBOX_STATES(pRenderHal, pVeboxState)
#define VPHAL_DBG_STATE_DUMPPER_DUMP_VEBOX_STATISTICS(pRenderHal, pVeboxState, pStatSlice0Base, pStatSlice1Base)
#define VPHAL_DBG_PARAMETERS_DUMPPER_CREATE()
#define VPHAL_DBG_PARAMETERS_DUMPPER_DESTORY(pParametersDumpSpec)
#define VPHAL_DBG_PARAMETERS_DUMPPER_DUMP_XML(pRenderParams)

#endif // (!(_DEBUG || _RELEASE_INTERNAL) || EMUL)

#endif  // __VPHAL_DEBUG_H__
