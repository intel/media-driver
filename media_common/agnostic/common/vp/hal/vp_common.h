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
//! \file     vp_common.h
//! \brief    clarify common utilities for vphal
//! \details  clarify common utilities for vphal including:
//!           some marcro, enum, union, structure, function
//!
#ifndef __VP_COMMON_H__
#define __VP_COMMON_H__

#if EMUL || VPHAL_LIB

#include "support.h"

#endif  // EMUL || VPHAL_LIB

#include "mos_os.h"
#include "vp_common_hdr.h"
#include "media_common_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define VPHAL_MEMORY_OBJECT_CONTROL uint32_t

//!
//! \brief Vphal Palette Type Mode enum
//!
typedef enum _VPHAL_PALETTE_TYPE
{
    VPHAL_PALETTE_NONE = 0,
    VPHAL_PALETTE_YCbCr_8,  //!< 8-bit depth - AYUV palette
    VPHAL_PALETTE_ARGB_8,   //!< 8-bit depth - ARGB palette
    VPHAL_PALETTE_AVYU_8    //!< 8-bit depth - YUY2 palette
} VPHAL_PALETTE_TYPE;

//!
//! \brief Vphal Chroma Siting enum
//!
typedef enum _VPHAL_CHROMA_SITING
{
    CHROMA_SITING_NONE        = 0,
    CHROMA_SITING_HORZ_LEFT   = 1 << 0,
    CHROMA_SITING_HORZ_CENTER = 1 << 1,
    CHROMA_SITING_HORZ_RIGHT  = 1 << 2,
    CHROMA_SITING_VERT_TOP    = 1 << 4,
    CHROMA_SITING_VERT_CENTER = 1 << 5,
    CHROMA_SITING_VERT_BOTTOM = 1 << 6,
} VPHAL_CHROMA_SITING;

//!
//! \brief Vphal Rotation Mode enum
//!
typedef enum _VPHAL_ROTATION
{
    VPHAL_ROTATION_IDENTITY           = ROTATION_IDENTITY,             //!< Rotation 0 degrees
    VPHAL_ROTATION_90                 = ROTATION_90,                   //!< Rotation 90 degrees
    VPHAL_ROTATION_180                = ROTATION_180,                  //!< Rotation 180 degrees
    VPHAL_ROTATION_270                = ROTATION_270,                  //!< Rotation 270 degrees
    VPHAL_MIRROR_HORIZONTAL           = ROTATION_MIRROR_HORIZONTAL,    //!< Horizontal Mirror
    VPHAL_MIRROR_VERTICAL             = ROTATION_MIRROR_VERTICAL,      //!< Vertical Mirror
    VPHAL_ROTATE_90_MIRROR_VERTICAL   = ROTATION_90_MIRROR_VERTICAL,   //!< 90 + V Mirror
    VPHAL_ROTATE_90_MIRROR_HORIZONTAL = ROTATION_90_MIRROR_HORIZONTAL  //!< 90 + H Mirror
} VPHAL_ROTATION;

//!
//! \brief Color Spaces enum
//!
typedef MEDIA_CSPACE VPHAL_CSPACE;

//!
//! \brief Surface types enum
//!        IMPORTANT : SurfaceType_Layer[] must be updated to match this enum type
//!
typedef enum _VPHAL_SURFACE_TYPE
{
    SURF_NONE = 0,
    SURF_IN_BACKGROUND,
    SURF_IN_PRIMARY,
    SURF_IN_SUBSTREAM,
    SURF_IN_REFERENCE,
    SURF_OUT_RENDERTARGET,
    SURF_TYPE_COUNT  //!< Keep this line at the end
} VPHAL_SURFACE_TYPE;
C_ASSERT(SURF_TYPE_COUNT == 6);  //!< When adding, update assert & vphal_solo_scenario.cpp

//!
//! Structure VPHAL_GAMMA_TYPE
//! \brief GAMMA Function type
//!
typedef enum _VPHAL_GAMMA_TYPE
{
    VPHAL_GAMMA_NONE = 0,
    VPHAL_GAMMA_TRADITIONAL_GAMMA,
    VPHAL_GAMMA_SMPTE_ST2084,
    VPHAL_GAMMA_BT1886,
    VPHAL_GAMMA_SRGB,
    VPHAL_GAMMA_Count
} VPHAL_GAMMA_TYPE;
C_ASSERT(VPHAL_GAMMA_Count == 5);  //!< When adding, update assert

//!
//! \def IS_COLOR_SPACE_BT2020_YUV(_a)
//! Check if the color space is BT2020 YUV
//!
#define IS_COLOR_SPACE_BT2020_YUV(_a) (_a == CSpace_BT2020 || \
                                       _a == CSpace_BT2020_FullRange)

//!
//! \def IS_COLOR_SPACE_BT2020_RGB(_a)
//! Check if the color space is BT2020 RGB
//!
#define IS_COLOR_SPACE_BT2020_RGB(_a) (_a == CSpace_BT2020_RGB || \
                                       _a == CSpace_BT2020_stRGB)

//!
//! \def IS_COLOR_SPACE_BT2020(_a)
//! Check if the color space is BT2020
//!
#define IS_COLOR_SPACE_BT2020(_a) (IS_COLOR_SPACE_BT2020_YUV(_a) || \
                                   IS_COLOR_SPACE_BT2020_RGB(_a))

//!
//! \brief Sample Type enum
//!
typedef enum _VPHAL_SAMPLE_TYPE
{
    SAMPLE_PROGRESSIVE,
    SAMPLE_SINGLE_TOP_FIELD,
    SAMPLE_SINGLE_BOTTOM_FIELD,
    SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD,
    SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD,
    SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD,
    SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD,
    SAMPLE_INVALID
} VPHAL_SAMPLE_TYPE;
C_ASSERT(SAMPLE_INVALID == 7);  //!< When adding, update assert & vphal_solo_scenario.cpp

//!
//! \brief Interlaced Scaling Mode enum
//!
typedef enum _VPHAL_ISCALING_TYPE
{
    ISCALING_NONE,
    ISCALING_INTERLEAVED_TO_INTERLEAVED,
    ISCALING_INTERLEAVED_TO_FIELD,
    ISCALING_FIELD_TO_INTERLEAVED,
    ISCALING_FIELD_TO_FIELD
} VPHAL_ISCALING_TYPE;
C_ASSERT(ISCALING_FIELD_TO_FIELD == 4);

//!
//! \ brief HVS DN Mode enum
//!
typedef enum _VPHAL_HVSDN_MODE
{
    HVSDENOISE_AUTO_BDRATE = 0,
    HVSDENOISE_AUTO_SUBJECTIVE,
    HVSDENOISE_MANUAL
} VPHAL_HVSDN_MODE;

//!
//! \brief DI Mode enum
//!
typedef enum _VPHAL_DI_MODE
{
    DI_MODE_BOB,
    DI_MODE_ADI
} VPHAL_DI_MODE;
C_ASSERT(DI_MODE_ADI == 1);  //!< When adding, update assert & vphal_solo_scenario.cpp

//!
//! \brief Blend Type enum
//!
typedef enum _VPHAL_BLEND_TYPE
{
    BLEND_NONE = 0,
    BLEND_SOURCE,
    BLEND_PARTIAL,
    BLEND_CONSTANT,
    BLEND_CONSTANT_SOURCE,
    BLEND_CONSTANT_PARTIAL,
    BLEND_XOR_MONO
} VPHAL_BLEND_TYPE;
C_ASSERT(BLEND_CONSTANT == 3);  //!< When adding, update assert & vphal_solo_scenario.cpp

//!
//! \brief Scaling Mode enum
//!
typedef enum _VPHAL_SCALING_MODE
{
    VPHAL_SCALING_NEAREST = 0,
    VPHAL_SCALING_BILINEAR,
    VPHAL_SCALING_AVS,
    VPHAL_SCALING_ADV_QUALITY  // !< Advance Perf mode
} VPHAL_SCALING_MODE;
C_ASSERT(VPHAL_SCALING_ADV_QUALITY == 3);  //!< When adding, update assert & vphal_solo_scenario.cpp

//!
//! \brief Composition Report Mode enum
//!
typedef enum _VPHAL_COMPOSITION_REPORT_MODE
{
    VPHAL_NO_COMPOSITION,       //!< No composition , Error Reporting cases
    VPHAL_INPLACE_COMPOSITION,  //!< Inplace Composition
    VPHAL_LEGACY_COMPOSITION    //!< Legacy Composition
} VPHAL_COMPOSITION_REPORT_MODE;

typedef enum _VPHAL_DI_REPORT_MODE
{
    VPHAL_DI_REPORT_PROGRESSIVE,  //!< Progressive output
    VPHAL_DI_REPORT_BOB,          //!< True BOB output
    VPHAL_DI_REPORT_ADI_BOB,      //!< DNDI BOB output
    VPHAL_DI_REPORT_ADI,          //!< ADI output
    VPHAL_DI_REPORT_FMD           //!< FMD output
} VPHAL_DI_REPORT_MODE;

//!
//! \brief Colorpack enum
//!
typedef enum _VPHAL_COLORPACK
{
    VPHAL_COLORPACK_400 = 0,
    VPHAL_COLORPACK_420,
    VPHAL_COLORPACK_411,
    VPHAL_COLORPACK_422,
    VPHAL_COLORPACK_444,
    VPHAL_COLORPACK_UNKNOWN
} VPHAL_COLORPACK, *PVPHAL_COLORPACK;

//!
//! \brief   Output Pipe Mode enum
//! \details The output pipe that writes the RenderTarget surface
//!
typedef enum _VPHAL_OUTPUT_PIPE_MODE
{
    VPHAL_OUTPUT_PIPE_MODE_INVALID = -1,  //!< None output pipe selected. This is an invalid state
    VPHAL_OUTPUT_PIPE_MODE_COMP    = 0,   //!< Composition output pipe. RenderTarget will be written by Composition
    VPHAL_OUTPUT_PIPE_MODE_SFC     = 1,   //!< SFC output pipe. RenderTarget will be written by SFC
    VPHAL_OUTPUT_PIPE_MODE_VEBOX   = 2    //!< Vebox output pipe. RenderTarget will be written by Vebox
} VPHAL_OUTPUT_PIPE_MODE,
    *PVPHAL_OUTPUT_PIPE_MODE;

typedef enum _VPHAL_SCALING_PREFERENCE
{
    VPHAL_SCALING_PREFER_SFC = 0,       // SFC is default
    VPHAL_SCALING_PREFER_COMP,          // For Gen9/10, Composition AVS get better quality than SFC AVS
    VPHAL_SCALING_PREFER_SFC_FOR_VEBOX  // if VEBOX is required, use VEBOX + SFC, otherwise use Composistion
} VPHAL_SCALING_PREFERENCE;
C_ASSERT(VPHAL_SCALING_PREFER_SFC_FOR_VEBOX == 2);  //!< When adding, update assert & vphal_solo_scenario.cpp

//-----------------------------------------------------------------------------
// Forward declaration -
// IMPORTANT - DDI interfaces are NOT to access internal VPHAL states
//-----------------------------------------------------------------------------
typedef struct _RENDERHAL_INTERFACE *PRENDERHAL_INTERFACE;
typedef class MhwVeboxInterface *    PMHW_VEBOX_INTERFACE;
typedef class MhwSfcInterface *      PMHW_SFC_INTERFACE;
typedef struct VPHAL_SURFACE            *PVPHAL_SURFACE;

class VphalRenderer;

class MhwCpInterface;

//!
//! Union   VPHAL_COLOR_SAMPLE_8
//! \brief  Vphal Color Sample 8 bit
//!
typedef union _VPHAL_COLOR_SAMPLE_8
{
    // ARGB
    struct
    {
        uint8_t B;
        uint8_t G;
        uint8_t R;
        uint8_t A;
    };

    // AVYU (YUY2 samples)
    struct
    {
        uint8_t U;
        uint8_t Y;
        uint8_t V;
        uint8_t a;
    };

    // AYCbCr (NV12 samples)
    struct
    {
        uint8_t Cr;  //!< V
        uint8_t Cb;  //!< U
        uint8_t YY;  //!< Y
        uint8_t Alpha;
    };

    uint32_t dwValue;
} VPHAL_COLOR_SAMPLE_8, *PVPHAL_COLOR_SAMPLE_8;

//!
//! Union   VPHAL_COLOR_SAMPLE_16
//! \brief  Vphal Color Sample 16 bit
//!
typedef union _VPHAL_COLOR_SAMPLE_16
{
    // ARGB
    struct
    {
        int16_t B;
        int16_t G;
        int16_t R;
        int16_t A;
    };

    // AVYU
    struct
    {
        int16_t U;
        int16_t Y;
        int16_t V;
        int16_t a;
    };

    // AYCbCr
    struct
    {
        int16_t Cr;  //!< V
        int16_t Cb;  //!< U
        int16_t YY;  //!< Y
        int16_t Alpha;
    };

    uint32_t Value;
} VPHAL_COLOR_SAMPLE_16, *PVPHAL_COLOR_SAMPLE_16;

//!
//! Structure VPHAL_PLANE_OFFSET
//! \brief  Vphal Plane OffSet
//!
typedef struct _VPHAL_PLANE_OFFSET
{
    int iLockSurfaceOffset;  //!< Locked surface offset
    int iSurfaceOffset;      //!< Plane surface offset
    int iXOffset;            //!< X offset - horizontal offset in pixels
    int iYOffset;            //!< Y offset - vertical offset in pixels
} VPHAL_PLANE_OFFSET, *PVPHAL_PLANE_OFFSET;

//!
//! Structure VPHAL_PALETTE
//! \brief    Vphal Palette
//! \details  Palette entries:
//!           - 0 : empty palette
//!           - -1 : palette allocated but not loaded
//!           - >0 : palette in use
//!
typedef struct _VPHAL_PALETTE
{
    VPHAL_PALETTE_TYPE PaletteType;    //!< Type of palette
    VPHAL_CSPACE       ColorSpace;     //!< ColorSpace of Palette
    bool               bHasAlpha;      //!< Alpha is valid
    int32_t            iTotalEntries;  //!< Palette entries allocated
    int32_t            iNumEntries;    //!< Palette entries in use
    union
    {
        PVPHAL_COLOR_SAMPLE_8  pPalette8;
        PVPHAL_COLOR_SAMPLE_16 pPalette16;
    };
} VPHAL_PALETTE, *PVPHAL_PALETTE;

//!
//! Structure VPHAL_BLENDING_PARAMS
//! \brief Layer Blending parameters
//!
typedef struct _VPHAL_BLENDING_PARAMS
{
    VPHAL_BLEND_TYPE BlendType;
    float            fAlpha;
} VPHAL_BLENDING_PARAMS, *PVPHAL_BLENDING_PARAMS;

//!
//! Structure VPHAL_LUMAKEY_PARAMS
//! \brief Luma Keying parameters
//!
typedef struct _VPHAL_LUMAKEY_PARAMS
{
    int16_t LumaLow;
    int16_t LumaHigh;
} VPHAL_LUMAKEY_PARAMS, *PVPHAL_LUMAKEY_PARAMS;

//!
//! Structure VPHAL_PROCAMP_PARAMS
//! \brief Procamp parameters
//!
typedef struct _VPHAL_PROCAMP_PARAMS
{
    bool  bEnabled;
    float fBrightness;
    float fContrast;
    float fHue;
    float fSaturation;
} VPHAL_PROCAMP_PARAMS, *PVPHAL_PROCAMP_PARAMS;

//!
//! Structure VPHAL_IEF_PARAMS
//! \brief IEF parameters - Image Enhancement (Detail) Filter
//!
typedef struct _VPHAL_IEF_PARAMS
{
    bool     bEnabled;
    bool     bSmoothMode;
    bool     bSkintoneTuned;
    bool     bEmphasizeSkinDetail;
    float    fIEFFactor;
    uint16_t StrongEdgeWeight;
    uint16_t RegularWeight;
    uint16_t StrongEdgeThreshold;
    void *   pExtParam;
} VPHAL_IEF_PARAMS, *PVPHAL_IEF_PARAMS;

//!
//! Structure VPHAL_DI_PARAMS
//! \brief Deinterlacing parameters
//!
typedef struct _VPHAL_DI_PARAMS
{
    VPHAL_DI_MODE DIMode;        //!< DeInterlacing mode
    bool          bEnableFMD;    //!< FMD
    bool          bSingleField;  //!< Used in frame Recon - if 30fps (one call per sample pair)
    bool          bSCDEnable;    //!< Scene change detection
} VPHAL_DI_PARAMS, *PVPHAL_DI_PARAMS;

//!
//! \brief Noise Level Enumeration
//!
typedef enum _VPHAL_NOISELEVEL
{
    NOISELEVEL_DEFAULT,
    NOISELEVEL_VC1_HD
} VPHAL_NOISELEVEL;
C_ASSERT(NOISELEVEL_VC1_HD == 1);  //!< When adding, update assert & vphal_solo_scenario.cpp

//!
//! Structure VPHAL_HVSDENOISE_PARAMS
//! \brief HVS Denoise Parameters - Human Vision System Based Denoise
//!
typedef struct _VPHAL_HVSDENOISE_PARAMS
{
    uint16_t         QP                  = 0;
    uint16_t         Strength            = 0;
    VPHAL_HVSDN_MODE Mode                = HVSDENOISE_AUTO_BDRATE;
    void *           pHVSDenoiseParam    = nullptr;
    uint32_t         dwDenoiseParamSize  = 0;
    uint32_t         dwGlobalNoiseLevel  = 0;  //!< Global Noise Level for Y
    uint32_t         dwGlobalNoiseLevelU = 0;  //!< Global Noise Level for U
    uint32_t         dwGlobalNoiseLevelV = 0;  //!< Global Noise Level for V
    uint16_t         TgneEnable          = 0;
    uint16_t         FirstFrame          = 0;
    uint16_t         TgneFirstFrame      = 0;
    uint16_t         Fallback            = 0;
    uint16_t         EnableChroma        = 0;
    uint16_t         EnableTemporalGNE   = 0;
} VPHAL_HVSDENOISE_PARAMS, *PVPHAL_HVSDENOISE_PARAMS;

//!
//! Structure VPHAL_SLIMIPU_DENOISE_PARAM
//! \brief SLIMIPU Denoise Parameters
//!
typedef struct _VPHAL_SLIMIPU_DENOISE_PARAM
{
    uint32_t MemSizeInBytes;
    void *   pSystemMem;
} VPHAL_SLIMIPU_DENOISE_PARAM, *PVPHAL_SLIMIPU_DENOISE_PARAM;

//!
//! Structure VPHAL_DENOISE_PARAMS
//! \brief Denoise parameters
//!
typedef struct _VPHAL_DENOISE_PARAMS
{
    bool                        bEnableChroma         = false;
    bool                        bEnableLuma           = false;
    bool                        bAutoDetect           = false;
    float                       fDenoiseFactor        = 0.0;
    VPHAL_NOISELEVEL            NoiseLevel            = NOISELEVEL_DEFAULT;
    bool                        bEnableHVSDenoise     = false;
    VPHAL_HVSDENOISE_PARAMS     HVSDenoise            = {};
    bool                        bEnableSlimIPUDenoise = false;
    VPHAL_SLIMIPU_DENOISE_PARAM SlimIPUDenoise        = {};
} VPHAL_DENOISE_PARAMS, *PVPHAL_DENOISE_PARAMS;

//!
//! Structure VPHAL_STE_PARAMS
//! \brief STE parameters - Skin Tone Enhancement
//!
typedef struct _VPHAL_STE_PARAMS
{
    uint32_t dwSTEFactor;
} VPHAL_STE_PARAMS, *PVPHAL_STE_PARAMS;

//!
//! Structure VPHAL_TCC_PARAMS
//! \brief TCC parameters - Total Color Control
//!
typedef struct _VPHAL_TCC_PARAMS
{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
    uint8_t Cyan;
    uint8_t Magenta;
    uint8_t Yellow;
} VPHAL_TCC_PARAMS, *PVPHAL_TCC_PARAMS;

//!
//! Structure VPHAL_COLORPIPE_PARAMS
//! \brief IECP Parameters - Color Pipe
//!
typedef struct _VPHAL_COLORPIPE_PARAMS
{
    bool             bEnableACE;
    bool             bEnableSTE;
    bool             bEnableTCC;
    bool             bAceLevelChanged;
    uint32_t         dwAceLevel;
    uint32_t         dwAceStrength;
    VPHAL_STE_PARAMS SteParams;
    VPHAL_TCC_PARAMS TccParams;
} VPHAL_COLORPIPE_PARAMS, *PVPHAL_COLORPIPE_PARAMS;

//!
//! Structure VPHAL_3DLUT_PARAMS
//! \brief 3DLUT parameters - 3DLUT
//!
typedef struct _VPHAL_3DLUT_PARAMS
{
    PVPHAL_SURFACE pExt3DLutSurface;    // Pointer to the 3DLUT surface which app passes to driver.
    uint32_t       LutSize;             // Size of 3DLUT, i.e, how many entries LUT has.
    uint32_t       ChannelMapping;      // Channel Mapping for the 3DLUT input to 3DLUT output.
    uint16_t       BitDepthPerChannel;  // Bit Depth Per Channel(4 channels for 3DLUT).
    uint16_t       ByteCountPerEntry;   // Byte Count Per Entry including reserved bytes.
} VPHAL_3DLUT_PARAMS, *PVPHAL_3DLUT_PARAMS;

//!
//! Structure VPHAL_SURFACE
//! \brief DDI-VPHAL surface definition
//!
struct VPHAL_SURFACE
{
    // Color Information
    VPHAL_CSPACE  ColorSpace    = CSpace_None;  //!<Color Space
    bool          ExtendedGamut = false;        //!<Extended Gamut Flag
    int32_t       iPalette      = 0;            //!<Palette Allocation
    VPHAL_PALETTE Palette       = {};           //!<Palette data

    // Rendering parameters
    RECT                   rcSrc           = {0, 0, 0, 0};  //!< Source rectangle
    RECT                   rcDst           = {0, 0, 0, 0};  //!< Destination rectangle
    RECT                   rcMaxSrc        = {0, 0, 0, 0};  //!< Max source rectangle
    PVPHAL_BLENDING_PARAMS pBlendingParams = nullptr;       //!< Blending parameters
    PVPHAL_LUMAKEY_PARAMS  pLumaKeyParams  = nullptr;       //!< Luma keying parameters
    PVPHAL_PROCAMP_PARAMS  pProcampParams  = nullptr;
    ;                                                //!< Procamp parameters
    PVPHAL_IEF_PARAMS pIEFParams         = nullptr;  //!< IEF parameters
    bool              bCalculatingAlpha  = false;    //!< Alpha calculation parameters
    bool              bQueryVariance     = false;    //!< enable variance query
    bool              bDirectionalScalar = false;    //!< Vebox Directional Scalar
    bool              bFastColorFill     = false;    //!< enable fast color fill without copy surface
    bool              bMaxRectChanged    = false;    //!< indicate rcMaxSrc been updated
    bool              b16UsrPtr          = false;    //!< is 16 byte aligned system linear memory.
    bool              bVEBOXCroppingUsed = false;    //!< Vebox crop case need use rcSrc as vebox input.
    bool              bXORComp           = false;    //!< is mono-chroma composite mode.

    // Interlaced Scaling
    bool                bInterlacedScaling    = false;          //!< Interlaced scaling
    bool                bFieldWeaving         = false;          //!< Field Weaving
    VPHAL_ISCALING_TYPE InterlacedScalingType = ISCALING_NONE;  //!< Interlaced scaling type for new interlaced scaling mode

    // Advanced Processing
    PVPHAL_DI_PARAMS        pDeinterlaceParams = nullptr;
    PVPHAL_DENOISE_PARAMS   pDenoiseParams     = nullptr;  //!< Denoise
    PVPHAL_COLORPIPE_PARAMS pColorPipeParams   = nullptr;  //!< ColorPipe

    // Frame ID and reference samples -> for advanced processing
    int32_t        FrameID      = 0;
    uint32_t       uFwdRefCount = 0;
    uint32_t       uBwdRefCount = 0;
    PVPHAL_SURFACE pFwdRef      = nullptr;
    PVPHAL_SURFACE pBwdRef      = nullptr;

    // VPHAL_SURFACE Linked list
    PVPHAL_SURFACE pNext = nullptr;

    //--------------------------------------
    // FIELDS TO BE SETUP BY VPHAL int32_tERNALLY
    //--------------------------------------
    uint32_t                 dwWidth           = 0;                         //!<  Surface width
    uint32_t                 dwHeight          = 0;                         //!<  Surface height
    uint32_t                 dwPitch           = 0;                         //!<  Surface pitch
    MOS_TILE_TYPE            TileType          = MOS_TILE_X;                //!<  Tile Type
    MOS_TILE_MODE_GMM        TileModeGMM       = MOS_TILE_LINEAR_GMM;       //!<  Tile Mode from GMM Definition
    bool                     bGMMTileEnabled   = false;                     //!<  GMM Tile Mode Flag
    bool                     bOverlay          = false;                     //!<  Overlay Surface
    bool                     bFlipChain        = false;                     //!<  FlipChain Surface
    VPHAL_PLANE_OFFSET       YPlaneOffset      = {0, 0, 0, 0};              //!<  Y surface plane offset
    VPHAL_PLANE_OFFSET       UPlaneOffset      = {0, 0, 0, 0};              //!<  U surface plane offset
    VPHAL_PLANE_OFFSET       VPlaneOffset      = {0, 0, 0, 0};              //!<  V surface plane offset
    int32_t                  iLayerID          = 0;                         //!<  Layer index (0-based index)
    VPHAL_SCALING_MODE       ScalingMode       = VPHAL_SCALING_NEAREST;     //!<  Scaling Mode
    VPHAL_SCALING_PREFERENCE ScalingPreference = VPHAL_SCALING_PREFER_SFC;  //!<  Scaling preference
    bool                     bIEF              = false;                     //!<  IEF flag
    uint32_t                 dwSlicePitch      = 0;                         //!<  SlicePitch of a 3D surface(GT-PIN support)

    //--------------------------------------
    // FIELDS TO BE PROVIDED BY DDI
    //--------------------------------------
    // Sample information
    MOS_FORMAT         Format     = Format_None;              //!<  Surface format
    VPHAL_SURFACE_TYPE SurfType   = SURF_NONE;                //!<  Surface type (context)
    VPHAL_SAMPLE_TYPE  SampleType = SAMPLE_PROGRESSIVE;       //!<  Interlaced/Progressive sample type
    uint32_t           dwDepth    = 0;                        //!<  Surface depth
    MOS_S3D_CHANNEL    Channel    = MOS_S3D_NONE;             //!<  Channel
    uint32_t           dwOffset   = 0;                        //!<  Surface Offset (Y/Base)
    MOS_RESOURCE       OsResource = {};                       //!<  Surface resource
    VPHAL_ROTATION     Rotation   = VPHAL_ROTATION_IDENTITY;  //!<  0: 0 degree, 1: 90 degree, 2: 180 degree, 3: 270 degreee

    // Chroma siting
    uint32_t ChromaSiting  = CHROMA_SITING_NONE;
    bool     bChromaSiting = false;  //!<  Chromasiting flag

    // Surface compression mode, enable flags
    bool bCompressible = false;  // The surface is compressible, means there are additional 128 bit for MMC no matter it is compressed or not
    // The bIsCompressed in surface allocation structure should use this flag to initialize to allocate a compressible surface
    bool                  bIsCompressed     = false;  // The surface is compressed, VEBox output can only support horizontal mode, but input can be horizontal / vertical
    MOS_RESOURCE_MMC_MODE CompressionMode   = MOS_MMC_DISABLED;
    uint32_t              CompressionFormat = 0;

    bool bUseSampleUnorm    = false;  //!<  true: sample unorm is used, false: DScaler or AVS is used.
    bool bUseSamplerLumakey = false;  //!<  true: sampler lumakey is used, false: lumakey is disabled or EU computed lumakey is used.
    //------------------------------------------
    // HDR related parameters, provided by DDI
    //------------------------------------------
    PVPHAL_HDR_PARAMS pHDRParams            = nullptr;
    VPHAL_GAMMA_TYPE  GammaType             = VPHAL_GAMMA_NONE;  //!<Gamma Type
    bool              bPreAPGWorkloadEnable = false;             //!< Previous Surface Execution Path

    // 3DLUT parameters
    PVPHAL_3DLUT_PARAMS p3DLutParams = nullptr;  //!< 3DLut Mapping Params
};

//!
//! Structure VPHAL_GET_SURFACE_INFO
//! \brief VPHAL Get Surface Infomation Parameters
//!

typedef struct VPHAL_GET_SURFACE_INFO *PVPHAL_GET_SURFACE_INFO;
struct VPHAL_GET_SURFACE_INFO
{
    uint32_t        ArraySlice;
    uint32_t        MipSlice;
    MOS_S3D_CHANNEL S3dChannel;
};

#ifdef __cplusplus
}
#endif

#endif  // __VP_COMMON_H__
