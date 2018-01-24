/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file     vphal.h
//! \brief    vphal interface clarification
//! \details  vphal interface clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VPHAL_H__
#define __VPHAL_H__

#include "vphal_common.h"

#include "vphal_common_composite.h"
#include "vphal_common_vebox.h"
#include "vphal_common_tools.h"
#include "mos_utilities.h"
#include "mos_util_debug.h"

//*-----------------------------------------------------------------------------
//| DEFINITIONS
//*-----------------------------------------------------------------------------
// Incremental size for allocating/reallocating resource
#define VPHAL_BUFFER_SIZE_INCREMENT     128

#define VPHAL_MAX_SOURCES               17       //!< worst case: 16 sub-streams + 1 pri video
#define VPHAL_MAX_CHANNELS              2
#define VPHAL_MAX_TARGETS               2        //!< dual output support for Android
#define VPHAL_MAX_FUTURE_FRAMES         18       //!< maximum future frames supported in VPHAL

// YUV input ranges
#define YUV_RANGE_16_235                1
#define YUV_RANGE_0_255                 2
#define YUV_RANGE_FROM_DDI              4

// Media Features height
#define VPHAL_RNDR_2K_HEIGHT  1080
// The reason that the definition is not (VPHAL_RNDR_2K_HEIGHT*2) is because some 4K clips have 1200 height.
#define VPHAL_RNDR_4K_HEIGHT  1200
#define VPHAL_RNDR_6K_HEIGHT  (VPHAL_RNDR_2K_HEIGHT*3)
#define VPHAL_RNDR_8K_HEIGHT  (VPHAL_RNDR_2K_HEIGHT*4)
#define VPHAL_RNDR_10K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*5)
#define VPHAL_RNDR_12K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*6)
#define VPHAL_RNDR_14K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*7)
#define VPHAL_RNDR_16K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*8)
#define VPHAL_RNDR_18K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*9)
#define VPHAL_RNDR_20K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*10)
#define VPHAL_RNDR_22K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*11)
#define VPHAL_RNDR_24K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*12)
#define VPHAL_RNDR_26K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*13)
#define VPHAL_RNDR_28K_HEIGHT (VPHAL_RNDR_2K_HEIGHT*14)

//------------------------------------------------------------------------------
// Simplified macros for debug message, Assert, Null check and MOS eStatus check
// within VPhal without the need to explicitly pass comp and sub-comp name
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_HW sub-comp
//------------------------------------------------------------------------------
#define VPHAL_HW_ASSERT(_expr)                                                       \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _expr)

#define VPHAL_HW_ASSERTMESSAGE(_message, ...)                                        \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VPHAL_HW_NORMALMESSAGE(_message, ...)                                        \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VPHAL_HW_VERBOSEMESSAGE(_message, ...)                                       \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _message, ##__VA_ARGS__)

#define VPHAL_HW_FUNCTION_ENTER                                                      \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW)

#define VPHAL_HW_CHK_STATUS(_stmt)                                                   \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _stmt)

#define VPHAL_HW_CHK_NULL(_ptr)                                                      \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _ptr)

#define VPHAL_HW_CHK_NULL_NO_STATUS(_ptr)                                            \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_HW, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_PUBLIC sub-comp
//------------------------------------------------------------------------------
#define VPHAL_PUBLIC_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _expr)

#define VPHAL_PUBLIC_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VPHAL_PUBLIC_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VPHAL_PUBLIC_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define VPHAL_PUBLIC_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC)

#define VPHAL_PUBLIC_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _stmt)

#define VPHAL_PUBLIC_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _stmt)

#define VPHAL_PUBLIC_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VPHAL_PUBLIC_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

#define VPHAL_PUBLIC_CHK_NULL_NO_STATUS_RETURN(_ptr)                                 \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_PUBLIC, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_DEBUG sub-comp
//------------------------------------------------------------------------------
#define VPHAL_DEBUG_ASSERT(_expr)                                                    \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _expr)

#define VPHAL_DEBUG_ASSERTMESSAGE(_message, ...)                                     \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VPHAL_DEBUG_NORMALMESSAGE(_message, ...)                                     \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VPHAL_DEBUG_VERBOSEMESSAGE(_message, ...)                                    \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _message, ##__VA_ARGS__)

#define VPHAL_DEBUG_FUNCTION_ENTER                                                   \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG)

#define VPHAL_DEBUG_CHK_STATUS(_stmt)                                                \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _stmt)

#define VPHAL_DEBUG_CHK_NULL(_ptr)                                                   \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _ptr)

#define VPHAL_DEBUG_CHK_NULL_NO_STATUS(_ptr)                                         \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DEBUG, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_RENDER sub-comp
//------------------------------------------------------------------------------
#define VPHAL_RENDER_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _expr)

#define VPHAL_RENDER_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER)

#define VPHAL_RENDER_EXITMESSAGE(_message, ...)                                      \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_FUNCTION_EXIT, MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define VPHAL_RENDER_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt)

#define VPHAL_RENDER_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _stmt, _message, ##__VA_ARGS__)

#define VPHAL_RENDER_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define VPHAL_RENDER_CHK_NULL_RETURN(_ptr)                                           \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

#define VPHAL_RENDER_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_RENDER, _ptr)

//------------------------------------------------------------------------------
// Macros specific to MOS_VP_SUBCOMP_DDI sub-comp
//------------------------------------------------------------------------------
#define VP_DDI_ASSERT(_expr)                                                         \
    MOS_ASSERT(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _expr)

#define VP_DDI_ASSERTMESSAGE(_message, ...)                                          \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_NORMALMESSAGE(_message, ...)                                          \
    MOS_NORMALMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_VERBOSEMESSAGE(_message, ...)                                         \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define VP_DDI_FUNCTION_ENTER                                                        \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI)

#define VP_DDI_CHK_STATUS(_stmt)                                                     \
    MOS_CHK_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _stmt)

#define VP_DDI_CHK_STATUS_MESSAGE(_stmt, _message, ...)                              \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _stmt, _message, ##__VA_ARGS__)

#define VP_DDI_CHK_NULL(_ptr)                                                        \
    MOS_CHK_NULL(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VP_DDI_CHK_NULL_NO_STATUS(_ptr)                                              \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VPHAL_DDI_CHK_HR(_ptr)                                                       \
    MOS_CHK_HR(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

#define VPHAL_DDI_CHK_NULL_WITH_HR(_ptr)                                             \
    MOS_CHK_NULL_WITH_HR(MOS_COMPONENT_VP, MOS_VP_SUBCOMP_DDI, _ptr)

//!
//! \brief Base VP kernel list
//!
enum VpKernelID
{
    // FC
    kernelCombinedFc = 0,

    // 2 VEBOX KERNELS
    kernelVeboxSecureBlockCopy,
    kernelVeboxUpdateDnState,

    baseKernelMaxNumID
};

//!
//! \brief VPHAL SS/EU setting
//!
struct VphalSseuSetting
{
    uint8_t   numSlices;
    uint8_t   numSubSlices;
    uint8_t   numEUs;
    uint8_t   reserved;       // Place holder for frequency setting
};

//-----------------------------------------------------------------------------
// Forward declaration -
// IMPORTANT - DDI interfaces are NOT to access internal VPHAL states
//-----------------------------------------------------------------------------
typedef struct _RENDERHAL_INTERFACE     *PRENDERHAL_INTERFACE;
typedef class MhwVeboxInterface         *PMHW_VEBOX_INTERFACE;
typedef class MhwSfcInterface           *PMHW_SFC_INTERFACE;
class VphalRenderer;

class MhwCpInterface;

//-----------------------------------------------------------------------------
// VPHAL-DDI RENDERING INTERFACE
//
//      Params that may apply to more than one layer are part of VPHAL_SURFACE
//      DDI layers must set this interface before calling pfnRender
//-----------------------------------------------------------------------------
//!
//! Structure VphalSettings
//! \brief VPHAL Settings - controls allocation of internal resources in VPHAL
//!
struct VphalSettings
{
    //!
    //! \brief    VphalSettings Constructor
    //! \details  Creates instance of VphalSettings
    //!
    VphalSettings() :
        maxPhases(0),
        mediaStates(0),
        sameSampleThreshold(0),
        disableDnDi(0),
        kernelUpdate(0),
        veboxParallelExecution(0)
    {
    };

    int32_t                maxPhases;
    int32_t                mediaStates;
    int32_t                sameSampleThreshold;
    uint32_t               disableDnDi;                                          //!< For validation purpose
    uint32_t               kernelUpdate;                                         //!< For VEBox Copy and Update kernels
    uint32_t               veboxParallelExecution;                               //!< Control VEBox parallel execution with render engine
};

//!
//! Structure VphalFeatureReport
//! \brief    Vphal Feature Report Structure
//!
struct VphalFeatureReport
{
    //!
    //! \brief    VphalFeatureReport Constructor
    //! \details  Creates instance of VphalFeatureReport
    //!
    VphalFeatureReport()
    {
        // call InitReportValue() to initialize report value
        InitReportValue();
    };

    //!
    //! \brief    initialize VphalFeatureReport value
    //! \details  initialize VphalFeatureReport value, can use it to reset report value
    //!
    void InitReportValue();

    bool                            IECP;               //!< IECP enable/disable
    bool                            IEF;                //!< Enhancement filter
    bool                            Denoise;            //!< Denoise
    bool                            ChromaDenoise;      //!< Chroma Denoise
    VPHAL_DI_REPORT_MODE            DeinterlaceMode;    //!< Deinterlace mode
    VPHAL_SCALING_MODE              ScalingMode;        //!< Scaling mode
    VPHAL_OUTPUT_PIPE_MODE          OutputPipeMode;     //!< Output Pipe
    bool                            VPMMCInUse;         //!< MMC enable flag
    bool                            RTCompressible;     //!< RT MMC Compressible flag
    uint8_t                         RTCompressMode;     //!< RT MMC Compression mode
    bool                            FFDICompressible;   //!< FFDI MMC Compressible flag
    uint8_t                         FFDICompressMode;   //!< FFDI MMC Compression mode
    bool                            FFDNCompressible;   //!< FFDN MMC Compressible flag
    uint8_t                         FFDNCompressMode;   //!< FFDN MMC Compression mode
    bool                            STMMCompressible;   //!< STMM MMC Compressible flag
    uint8_t                         STMMCompressMode;   //!< STMM MMC Compression mode
    bool                            ScalerCompressible; //!< Scaler MMC Compressible flag for Gen10
    uint8_t                         ScalerCompressMode; //!< Scaler MMC Compression mode for Gen10
    bool                            PrimaryCompressible;//!< Input Primary Surface Compressible flag
    uint8_t                         PrimaryCompressMode;//!< Input Primary Surface Compression mode
    VPHAL_COMPOSITION_REPORT_MODE   CompositionMode;    //!< Inplace/Legacy Compostion flag
    bool                            VEFeatureInUse;     //!< If any VEBOX feature is in use, excluding pure bypass for SFC
};

//!
//! Structure VPHAL_SURFACE
//! \brief DDI-VPHAL surface definition
//!
struct VPHAL_SURFACE
{
    // Color Information
    VPHAL_CSPACE                ColorSpace;         //!<Color Space
    bool                        ExtendedGamut;      //!<Extended Gamut Flag
    int32_t                     iPalette;           //!<Palette Allocation
    VPHAL_PALETTE               Palette;            //!<Palette data

    // Rendering parameters
    RECT                        rcSrc;              //!< Source rectangle
    RECT                        rcDst;              //!< Destination rectangle
    RECT                        rcMaxSrc;           //!< Max source rectangle
    PVPHAL_BLENDING_PARAMS      pBlendingParams;    //!< Blending parameters
    PVPHAL_LUMAKEY_PARAMS       pLumaKeyParams;     //!< Luma keying parameters
    PVPHAL_PROCAMP_PARAMS       pProcampParams;     //!< Procamp parameters
    PVPHAL_IEF_PARAMS           pIEFParams;         //!< IEF parameters
    bool                        bCalculatingAlpha;  //!< Alpha calculation parameters
    bool                        bInterlacedScaling; //!< Interlaced scaling
    bool                        bFieldWeaving;      //!< Field Weaving
    bool                        bQueryVariance;     //!< enable variance query
    bool                        bDirectionalScalar; //!< Vebox Directional Scalar
    bool                        bFastColorFill;     //!< enable fast color fill without copy surface
    bool                        bMaxRectChanged;    //!< indicate rcMaxSrc been updated

    // Advanced Processing
    PVPHAL_DI_PARAMS            pDeinterlaceParams;
    PVPHAL_DENOISE_PARAMS       pDenoiseParams;     //!< Denoise
    PVPHAL_COLORPIPE_PARAMS     pColorPipeParams;   //!< ColorPipe

    // Frame ID and reference samples -> for advanced processing
    int32_t                     FrameID;
    uint32_t                    uFwdRefCount;
    uint32_t                    uBwdRefCount;
    PVPHAL_SURFACE              pFwdRef;
    PVPHAL_SURFACE              pBwdRef;

    // VPHAL_SURFACE Linked list
    PVPHAL_SURFACE              pNext;

    //--------------------------------------
    // FIELDS TO BE SETUP BY VPHAL int32_tERNALLY
    //--------------------------------------
    uint32_t                    dwWidth;            //!<  Surface width
    uint32_t                    dwHeight;           //!<  Surface height
    uint32_t                    dwPitch;            //!<  Surface pitch
    MOS_TILE_TYPE               TileType;           //!<  Tile Type
    bool                        bOverlay;           //!<  Overlay Surface
    bool                        bFlipChain;         //!<  FlipChain Surface
    VPHAL_PLANE_OFFSET          YPlaneOffset;       //!<  Y surface plane offset
    VPHAL_PLANE_OFFSET          UPlaneOffset;       //!<  U surface plane offset
    VPHAL_PLANE_OFFSET          VPlaneOffset;       //!<  V surface plane offset
    int32_t                     iLayerID;           //!<  Layer index (0-based index)
    VPHAL_SCALING_MODE          ScalingMode;        //!<  Scaling Mode
    VPHAL_SCALING_PREFERENCE    ScalingPreference;  //!<  Scaling preference
    bool                        bIEF;               //!<  IEF flag
    uint32_t                    dwSlicePitch;       //!<  SlicePitch of a 3D surface(GT-PIN support)

    //--------------------------------------
    // FIELDS TO BE PROVIDED BY DDI
    //--------------------------------------
    // Sample information
    MOS_FORMAT                  Format;             //!<  Surface format
    VPHAL_SURFACE_TYPE          SurfType;           //!<  Surface type (context)
    VPHAL_SAMPLE_TYPE           SampleType;         //!<  Interlaced/Progressive sample type
    uint32_t                    dwDepth;            //!<  Surface depth
    MOS_S3D_CHANNEL             Channel;            //!<  Channel
    uint32_t                    dwOffset;           //!<  Surface Offset (Y/Base)
    MOS_RESOURCE                OsResource;         //!<  Surface resource
    VPHAL_ROTATION              Rotation;           //!<  0: 0 degree, 1: 90 degree, 2: 180 degree, 3: 270 degreee

    // Chroma siting
    uint32_t                    ChromaSiting;
    bool                        bChromaSiting;      //!<  Chromasiting flag

    // Surface compression mode, enable flags
    bool                        bCompressible;      // The surface is compressible, means there are additional 128 bit for MMC no matter it is compressed or not
                                                    // The bIsCompressed in surface allocation structure should use this flag to initialize to allocate a compressible surface
    bool                        bIsCompressed;      // The surface is compressed, VEBox output can only support horizontal mode, but input can be horizontal / vertical
    MOS_RESOURCE_MMC_MODE       CompressionMode;
};

//!
//! Structure VPHAL_RENDER_PARAMS
//! \brief VPHAL Rendering Parameters
//!
struct VPHAL_RENDER_PARAMS
{
    // Input/output surfaces
    uint32_t                                uSrcCount;                  //!< Num sources
    VPHAL_SURFACE                           *pSrc[VPHAL_MAX_SOURCES];   //!< Source Samples
    uint32_t                                uDstCount;                  //!< Num Targets
    VPHAL_SURFACE                           *pTarget[VPHAL_MAX_TARGETS];//!< Render Target

    // Additional parameters not included in PVPHAL_SURFACE
    PRECT                                   pConstriction;              //!< Constriction rectangle
    PVPHAL_COLORFILL_PARAMS                 pColorFillParams;           //!< ColorFill - BG only
    bool                                    bTurboMode;                 //!< Enable Media Turbo Mode
    bool                                    bStereoMode;                //!< Stereo BLT mode
    PVPHAL_ALPHA_PARAMS                     pCompAlpha;                 //!< Alpha for composited surfaces
    bool                                    bDisableDemoMode;           //!< Enable/Disable demo mode function calls
    PVPHAL_SPLIT_SCREEN_DEMO_MODE_PARAMS    pSplitScreenDemoModeParams; //!< Split-screen demo mode for VP features
    bool                                    bIsDefaultStream;           //!< Identifier to differentiate default stream

    // Debugging parameters
    MOS_COMPONENT                           Component;                  //!< DDI component (for DEBUGGING only)

    // Status Report
    bool                                    bReportStatus;              //!< Report current media BB status (Pre-Processing)
    uint32_t                                StatusFeedBackID;           //!< Unique Staus ID;
#if (_DEBUG || _RELEASE_INTERNAL)
    bool                                    bTriggerGPUHang;            //!< Trigger GPU HANG
#endif

    bool                                    bCalculatingAlpha;          //!< Alpha calculation parameters

    // extension parameters
    void                                    *pExtensionData;            //!< Extension data

    VPHAL_RENDER_PARAMS():
        uSrcCount(0),
        pSrc(),
        uDstCount(0),
        pTarget(),
        pConstriction(nullptr),
        pColorFillParams(nullptr),
        bTurboMode(false),
        bStereoMode(false),
        pCompAlpha(nullptr),
        bDisableDemoMode(false),
        pSplitScreenDemoModeParams(nullptr),
        bIsDefaultStream(false),
        Component(),
        bReportStatus(false),
        StatusFeedBackID(0),
#if (_DEBUG || _RELEASE_INTERNAL)
        bTriggerGPUHang(false),
#endif
        bCalculatingAlpha(false),
        pExtensionData(nullptr)
    {
    }

};

typedef VPHAL_RENDER_PARAMS *PVPHAL_RENDER_PARAMS;

//!
//! Structure VPHAL_GET_SURFACE_INFO
//! \brief VPHAL Get Surface Infomation Parameters
//!
struct VPHAL_GET_SURFACE_INFO
{
    uint32_t          ArraySlice;
    uint32_t          MipSlice;
    MOS_S3D_CHANNEL   S3dChannel;
};

typedef const VPHAL_RENDER_PARAMS  *PCVPHAL_RENDER_PARAMS;

//!
//! Class VphalState
//! \brief VPHAL class definition
//!
class VphalState
{
public:
    // factory function
    static VphalState* VphalStateFactory(
        PMOS_INTERFACE          pOsInterface,
        PMOS_CONTEXT            pOsDriverContext,
        MOS_STATUS              *peStatus);

    //!
    //! \brief    VphalState Constructor
    //! \details  Creates instance of VphalState
    //!           - Caller must call Allocate to allocate all VPHAL states and objects.
    //! \param    [in] pOsInterface
    //!           OS interface, if provided externally - may be nullptr
    //! \param    [in] pOsDriverContext
    //!           OS driver context (UMD context, pShared, ...)
    //! \param    [in,out] peStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalState(
        PMOS_INTERFACE          pOsInterface,
        PMOS_CONTEXT            pOsDriverContext,
        MOS_STATUS              *peStatus);

    //!
    //! \brief    VphalState Destuctor
    //! \details  Destroys VPHAL and all internal states and objects
    //!
    virtual ~VphalState();

    //!
    //! \brief    Allocate VPHAL Resources
    //! \details  Allocate VPHAL Resources
    //!           - Allocate and initialize HW states
    //!           - Allocate and initialize renderer states
    //! \param    [in] pVpHalSettings
    //!           Pointer to VPHAL Settings
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS Allocate(
        const VphalSettings     *pVpHalSettings);

    //!
    //! \brief    Performs VP Rendering
    //! \details  Performs VP Rendering
    //!           - call default render of video
    //! \param    [in] pcRenderParams
    //!           Pointer to Render Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Render(
        PCVPHAL_RENDER_PARAMS   pcRenderParams);

    //!
    //! \brief    Get feature reporting from renderer
    //! \details  Get feature reporting from renderer
    //! \return   VphalFeatureReport*
    //!           Pointer to VPHAL_FEATURE_REPOR: rendering features reported
    //!
    VphalFeatureReport*       GetRenderFeatureReport();

    //!
    //! \brief    Get Status Report
    //! \details  Get Status Report, will return back to app indicating if related frame id is done by gpu
    //! \param    [out] pQueryReport
    //!           Pointer to pQueryReport, the status query report array.
    //! \param    [in] wStatusNum
    //!           The size of array pQueryReport.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    MOS_STATUS GetStatusReport(
        PQUERY_STATUS_REPORT_APP        pQueryReport,
        uint16_t                        numStatus);

    //!
    //! \brief    Get Status Report's entry length from head to tail
    //! \details  Get Status Report's entry length from head to tail
    //! \param    [out] puiLength
    //!           Pointer to the entry length
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS GetStatusReportEntryLength(
        uint32_t                         *puiLength);

    MEDIA_FEATURE_TABLE*          GetSkuTable()
    {
        return m_skuTable;
    };

    PMOS_INTERFACE              GetOsInterface()
    {
        return m_osInterface;
    };

    VphalRenderer*             GetRenderer()
    {
        return m_renderer;
    };

    void SetMhwVeboxInterface(MhwVeboxInterface* veboxInterface)
    {
        m_veboxInterface = veboxInterface;
    }

    void SetMhwSfcInterface(MhwSfcInterface* sfcInterface)
    {
        m_sfcInterface = sfcInterface;
    }
protected:
    // Internals
    PLATFORM                    m_platform;
    MEDIA_FEATURE_TABLE         *m_skuTable;
    MEDIA_WA_TABLE              *m_waTable;

    // States
    PMOS_INTERFACE              m_osInterface;
    PRENDERHAL_INTERFACE        m_renderHal;
    PMHW_VEBOX_INTERFACE        m_veboxInterface;
    MhwCpInterface              *m_cpInterface;
    PMHW_SFC_INTERFACE          m_sfcInterface;
    VphalRenderer               *m_renderer;

    // Render GPU context/node
    MOS_GPU_NODE                m_renderGpuNode;
    MOS_GPU_CONTEXT             m_renderGpuContext;

    //!
    //! \brief    Create instance of VphalRenderer
    //! \details  Create instance of VphalRenderer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CreateRenderer() = 0;
};

#endif  // __VPHAL_H__
