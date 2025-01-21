/*
* Copyright (c) 2018-2024, Intel Corporation
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
//! \file     vp_pipeline_common.h
//! \brief    Defines the common interface for vp pipeline
//!           this file is for the base interface which is shared by all features/vp Pipelines.
//!
#ifndef __VP_PIPELINE_COMMON_H__
#define __VP_PIPELINE_COMMON_H__

#include "mos_utilities.h"
#include "vp_common.h"
#include "renderhal.h"
#include "vp_base.h"

namespace vp
{
class VpPlatformInterface;
class VpUserFeatureControl;
}

using VP_PIPELINE_PARAMS   = VPHAL_RENDER_PARAMS;
using PVP_PIPELINE_PARAMS  = VPHAL_RENDER_PARAMS*;
using PCVP_PIPELINE_PARAMS = const VPHAL_RENDER_PARAMS*;

//!
//! \brief Flags for update/copy/FMD kernels
//!
#define VP_VEBOX_FLAG_ENABLE_KERNEL_COPY                   0x00000001
#define VP_VEBOX_FLAG_ENABLE_KERNEL_COPY_DEBUG             0x00000002
#define VP_VEBOX_FLAG_ENABLE_KERNEL_DN_UPDATE              0x00000004
#define VP_VEBOX_FLAG_ENABLE_KERNEL_DN_UPDATE_DEBUG        0x00000008
#define VP_VEBOX_FLAG_ENABLE_KERNEL_FMD_SUMMATION          0x00000010

#define RESOURCE_ASSIGNMENT_HINT_BITS_DI        \
    uint32_t    bDi                 : 1;        \
    uint32_t    b60fpsDi            : 1;

#define RESOURCE_ASSIGNMENT_HINT_BITS_SCALING   \
    uint32_t    isIScalingTypeNone  : 1;        \
    uint32_t    isFieldWeaving      : 1;

#define RESOURCE_ASSIGNMENT_HINT_BITS_HDR       \
    uint32_t    is3DLut2DNeeded     : 1;

#define RESOURCE_ASSIGNMENT_HINT_BITS_DENOISE \
    uint32_t isHVSTableNeeded : 1;

#define RESOURCE_ASSIGNMENT_HINT_BITS_STD_ALONE \
    uint32_t isSkinScoreDumpNeededForSTDonly : 1; \
    uint32_t isSkinScoreOutputNeededForSTDOnly : 1;

#define RESOURCE_ASSIGNMENT_HINT_BITS           \
        RESOURCE_ASSIGNMENT_HINT_BITS_DI        \
        RESOURCE_ASSIGNMENT_HINT_BITS_SCALING   \
        RESOURCE_ASSIGNMENT_HINT_BITS_HDR       \
        RESOURCE_ASSIGNMENT_HINT_BITS_DENOISE   \
        RESOURCE_ASSIGNMENT_HINT_BITS_STD_ALONE

#define RESOURCE_ASSIGNMENT_HINT_SIZE   4

//!
//! \brief Enumeration for the user feature key "Bypass Composition" values
//!
typedef enum _VPHAL_COMP_BYPASS_MODE
{
    VPHAL_COMP_BYPASS_NOT_SET  = 0xffffffff,
    VPHAL_COMP_BYPASS_DISABLED = 0x0,
    VPHAL_COMP_BYPASS_ENABLED  = 0x1,
    VPHAL_COMP_BYPASS_DEFAULT  = 0x2
} VPHAL_COMP_BYPASS_MODE, *PVPHAL_COMP_BYPASS_MODE;

//!
//! \brief VP CTRL enum
//!
typedef enum _VP_CTRL
{
    VP_CTRL_DISABLE = 0,
    VP_CTRL_ENABLE,
    VP_CTRL_DEFAULT
} VP_CTRL;

struct VP_SURFACE
{
    MOS_SURFACE                 *osSurface      = nullptr;         //!< mos surface
    bool                        isResourceOwner = false;           //!< true if the resource is owned by current instance.
    VPHAL_CSPACE                ColorSpace      = CSpace_None;     //!< Color Space
    uint32_t                    ChromaSiting    = 0;               //!< ChromaSiting

    bool                        bQueryVariance  = false;     //!< enable variance query. Not in use for internal surface
    int32_t                     FrameID         = 0;         //!< Not in use for internal surface
    bool                        ExtendedGamut   = false;     //!< Extended Gamut Flag. Not in use for internal surface
    VPHAL_PALETTE               Palette         = {};        //!< Palette data. Not in use for internal surface
    VPHAL_SURFACE_TYPE          SurfType        = SURF_NONE; //!< Surface type (context). Not in use for internal surface
    uint32_t                    uFwdRefCount    = 0;         //!< Not in use for internal surface
    uint32_t                    uBwdRefCount    = 0;         //!< Not in use for internal surface
    VPHAL_SURFACE               *pFwdRef        = nullptr;   //!< Use VP_SURFACE instead of VPHAL_SURFACE later. Not in use for internal surface.
    VPHAL_SURFACE               *pBwdRef        = nullptr;   //!< Use VP_SURFACE instead of VPHAL_SURFACE later. Not in use for internal surface.
    VPHAL_SAMPLE_TYPE            SampleType     = SAMPLE_PROGRESSIVE;  //!<  Interlaced/Progressive sample type.
    // Use index of m_InputSurfaces for layerID. No need iLayerID here anymore.
    RECT                        rcSrc               = {0, 0, 0, 0};    //!< Source rectangle
    RECT                        rcDst               = {0, 0, 0, 0};    //!< Destination rectangle
    RECT                        rcMaxSrc            = {0, 0, 0, 0};    //!< Max source rectangle
    bool                        bVEBOXCroppingUsed  = false;           //!<Vebox crop case need use rcSrc as vebox input.
    uint32_t                    bufferWidth         = 0;               //!< 1D buffer Width, n/a if 2D surface
    uint32_t                    bufferHeight        = 0;               //!< 1D buffer Height, n/a if 2D surface

    // Return true if no resource assigned to current vp surface.
    bool        IsEmpty();
    // Clean the vp surface to empty state. Only valid for false == isResourceOwner case.
    MOS_STATUS  Clean();

    // Get Allocation Handle of resource
    uint64_t    GetAllocationHandle(MOS_INTERFACE *osIntf);
};

struct _VP_SETTINGS
{
    // For validation purpose settings
    uint32_t               disableDnDi            = 0;                   //!< Disable DNDI(Vebox)
    uint32_t               kernelUpdate           = 0;                   //!< For VEBox Copy and Update kernels
    uint32_t               disableHdr             = 0;                   //!< Disable Hdr
    uint32_t               veboxParallelExecution = 0;                   //!< Control VEBox parallel execution with render engine
    bool                   clearVideoViewMode     = 0;                   //!< Perf Optimize for ClearVideoView DDI
};

using VP_SETTINGS = _VP_SETTINGS;

// To define the features enabling on different engines
struct _VP_EXECUTE_CAPS
{
    union {
        struct {
            uint64_t bVebox         : 1;   // Vebox needed
            uint64_t bSFC           : 1;   // SFC needed
            uint64_t bRender        : 1;   // Render Only needed
            uint64_t bSecureVebox   : 1;   // Vebox in Secure Mode
            uint64_t bRenderHdr     : 1;   // Render HDR in use

            uint64_t bOutputPipeFeatureInuse : 1; // Output surface of pipeline is in use.
            uint64_t bForceCscToRender : 1; // If true, force to use render for csc.
            uint64_t bForceProcampToRender : 1;   // If true, force to use render for procamp.
            uint64_t lastSubmission : 1;    // If true, it's the last submission of current DDI.
            uint64_t bTemperalInputInuse : 1; // If true, will use temperal input instead of input

            // Vebox Features
            uint64_t bDN            : 1;   // Vebox DN needed
            uint64_t bDI            : 1;   // Vebox DI enabled
            uint64_t bDiProcess2ndField : 1;   // Vebox DI enabled
            uint64_t bDIFmdKernel   : 1;   // Vebox FMD Kernel enabled
            uint64_t bIECP          : 1;   // Vebox IECP needed
            uint64_t bSTE           : 1;   // Vebox STE or Vebox STD_alone needed
            uint64_t bACE           : 1;   // Vebox ACE needed
            uint64_t bTCC           : 1;   // Vebox TCC needed
            uint64_t bCGC           : 1;   // Vebox CGC needed
            uint64_t bBt2020ToRGB   : 1;   // Vebox Bt2020 gamut compression to RGB format
            uint64_t bProcamp       : 1;   // Vebox Procamp needed
            uint64_t bBeCSC         : 1;   // Vebox back end CSC needed
            uint64_t bFeCSC         : 1;   // Vebox front end CSC needed
            uint64_t bLACE          : 1;   // Vebox LACE Needed
            uint64_t bQueryVariance : 1;
            uint64_t bRefValid      : 1;   // Vebox Ref is Valid
            uint64_t bSTD           : 1;   // Vebox LACE STD Needed
            uint64_t bDnKernelUpdate: 1;
            uint64_t bVeboxSecureCopy : 1;
            uint64_t bHDR3DLUT      : 1;  // Vebox 3DLUT needed
            uint64_t b1K1DLutInited : 1;
            uint64_t bDV            : 1;
            uint64_t b3DlutOutput   : 1;
            uint64_t bHdr33lutsize  : 1;
            uint64_t bCappipe       : 1;
            uint64_t bLgca          : 1;
            uint64_t bFDFB          : 1;
            uint64_t bColorBalance  : 1;
            uint64_t b1K1DLutInUse  : 1;
            uint64_t bCcmCsc        : 1;
            uint64_t bDemosaicInUse : 1;
            uint64_t enableSFCLinearOutputByTileConvert : 1;  //true if do tileconvert from tileF to linear for SFC linear output

            // SFC features
            uint64_t bSfcCsc        : 1;   // Sfc Csc enabled
            uint64_t bSfcRotMir     : 1;       // Sfc Rotation/Mirror needed
            uint64_t bSfcScaling    : 1;   // Sfc Scaling Needed
            uint64_t bSfcIef        : 1;   // Sfc Details Needed
            uint64_t b1stPassOfSfc2PassScaling : 1; // 1st pass of sfc 2pass scaling

            // Render Features
            uint64_t bComposite     : 1;
            uint64_t bSR            : 1;
            uint64_t b3DLutCalc     : 1;
            uint64_t bHVSCalc       : 1;
            uint64_t bSegmentation  : 1;
            uint64_t bHdr           : 1;
            uint64_t bFallbackLegacyFC : 1;     // only valid when vpUserFeatureControl->EnableOclFC() is true
            uint64_t forceBypassWorkload : 1;  // If true, force to bypass workload.
            uint64_t bAiPath        : 1;        // if ture, it will walk into ai common filter to execute a series of ai sub kernels
        };
        uint64_t value;
    };
};

C_ASSERT(sizeof(_VP_EXECUTE_CAPS) == sizeof(uint64_t));

typedef struct _VP_EngineEntry
{
    union
    {
        struct
        {
            // set by GetXxxExecuteCaps
            uint64_t bEnabled : 1;
            uint64_t SfcNeeded : 1;
            uint64_t VeboxNeeded : 1;
            uint64_t RenderNeeded : 1;
            uint64_t hdrKernelNeeded : 1;
            uint64_t fcSupported : 1;           // Supported by fast composition
            uint64_t hdrKernelSupported : 1;    // Supported by Hdr Kenrel
            uint64_t isolated : 1;              // Only support single feature.
            uint64_t bt2020ToRGB : 1;           // true if bt2020 to rgb
            uint64_t is1K1DLutSurfaceInUse : 1;  // 1K1DLut surface in use
            uint64_t isHdr33LutSizeEnabled : 1;
            uint64_t isBayerInputInUse : 1;
            uint64_t frontEndCscNeeded : 1;  // true if use vebox front end csc to do output csc feature instead of using backendcsc + sfc. Only using it when no scaling needed
            uint64_t forceLegacyFC : 1;          // true if OCL FC not support the format, fall back to legacy FC

            // set by GetXxxPipeEnginCaps
            uint64_t bypassIfVeboxSfcInUse : 1;  // Bypass the feature if vebox or sfc in use. In such case, VeboxNeeded and
                                                // SfcNeeded are 0 but it should not block vebox or sfc being selected. 
            uint64_t forceEnableForSfc : 1;  // Force enabled when sfc being selected.
            uint64_t forceEnableForFc : 1;   // Force enabled when fc being selected.
            uint64_t forceEnableForHdrKernel : 1;
            uint64_t nonFcFeatureExists : 1;     // The feature exists, which do not support fc
            uint64_t nonVeboxFeatureExists : 1;  // The feature exists, which do not support vebox
            uint64_t fcOnlyFeatureExists : 1;    // The feature exists, which only support render fc, and not support vebox/sfc.
            uint64_t multiPassNeeded : 1;        // If true, multi-pass for frame processing is needed.
            uint64_t VeboxARGBOut : 1;
            uint64_t VeboxARGB10bitOutput : 1;
            uint64_t VeboxIECPNeeded : 1;
            uint64_t bypassVeboxFeatures : 1;
            uint64_t diProcess2ndField : 1;
            uint64_t sfc2PassScalingNeededX : 1;
            uint64_t sfc2PassScalingNeededY : 1;
            uint64_t usedForNextPass : 1;       // true if current feature should be bypassed for current pass and be processed during next pass.
            uint64_t sfcNotSupported : 1;       // true if sfc cannot be selected.
            uint64_t veboxNotSupported : 1;     // true if vebox cannot be selected.
            uint64_t isOutputPipeNeeded : 1;    // true if the feature is used for parameter calculation.
            uint64_t sfcOnlyFeatureExists : 1;  // The feature exists, which only support sfc.
            uint64_t bTemperalInputInuse : 1;   // true if replace input
            uint64_t outputWithLumaKey : 1;
            uint64_t enableSFCLinearOutputByTileConvert : 1;  //true if do tileconvert from tileF to linear for SFC linear output
            uint64_t forceBypassWorkload : 1;   // If true, force to bypass workload.
        };
        uint64_t value;
    };
}VP_EngineEntry;

C_ASSERT(sizeof(_VP_EngineEntry) == sizeof(uint64_t));

enum _VP_PACKET_ENGINE
{
    VP_PACKET_COMP = 0,
    VP_PACKET_VEBOX,
};

enum VP_RGB_OUTPUT_OVERRIDE_ID
{
    VP_RGB_OUTPUT_OVERRIDE_ID_INVALID = 0,
    VP_RGB_OUTPUT_OVERRIDE_ID_RGBP_LINEAR,
    VP_RGB_OUTPUT_OVERRIDE_ID_RGBP_TILE,
    VP_RGB_OUTPUT_OVERRIDE_ID_RGB24LINEAR,
    VP_RGB_OUTPUT_OVERRIDE_ID_BGRP_LINEAR,
    VP_RGB_OUTPUT_OVERRIDE_ID_BGRP_TILE,
    VP_RGB_OUTPUT_OVERRIDE_ID_MAX,
};

typedef enum _VP_COMP_BYPASS_MODE
{
    VP_COMP_BYPASS_NOT_SET  = 0xffffffff,
    VP_COMP_BYPASS_DISABLED = 0x0,
    VP_COMP_BYPASS_ENABLED  = 0x1
} VP_COMP_BYPASS_MODE, * PVP_COMP_BYPASS_MODE;

// RESOURCE_ASSIGNMENT_HINT are feature parameters which will affect resource assignment.
// For caps existing in VP_EXECUTE_CAPS, they should not be added to RESOURCE_ASSIGNMENT_HINT.
union RESOURCE_ASSIGNMENT_HINT
{
    struct
    {
        RESOURCE_ASSIGNMENT_HINT_BITS;
    };
    uint32_t value[RESOURCE_ASSIGNMENT_HINT_SIZE];
};

using VP_EXECUTE_CAPS  = _VP_EXECUTE_CAPS;
using VP_PACKET_ENGINE = _VP_PACKET_ENGINE;
using PVP_SURFACE      = VP_SURFACE*;

inline bool IsVeboxFeatureInuse(VP_EXECUTE_CAPS &caps)
{
    return (caps.bVebox && (!caps.bSFC || caps.bDN || caps.bDI || caps.bIECP || caps.bSTE ||
            caps.bACE || caps.bTCC || caps.bBeCSC || caps.bQueryVariance || caps.bLACE || caps.bSTD ||
            caps.bHDR3DLUT || caps.bDV));
}

#endif
