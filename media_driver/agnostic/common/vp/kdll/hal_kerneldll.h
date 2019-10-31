/*
* Copyright (c) 2008-2017, Intel Corporation
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
//! \file      hal_kerneldll.h 
//! \brief         Fast Compositing dynamic kernel linking/loading definitions 
//!
#ifndef __HAL_KERNELDLL_H__
#define __HAL_KERNELDLL_H__

#include "mos_defs.h"
#include "cm_fc_ld.h"
// Kernel IDs and Kernel Names
#include "vpkrnheader.h" // IDR_VP_TOTAL_NUM_KERNELS

#if EMUL

#include "support.h"

// Search callback codes
#define CB_REASON_SEARCH_FAILED -1
#define CB_REASON_UPDATE_FAILED -2
#define CB_REASON_BEGIN_SEARCH   0
#define CB_REASON_BEGIN_UPDATE   1
#define CB_REASON_END_SEARCH     2

#else // EMUL

#endif // EMUL

#include "vphal_common.h"

#define ROUND_FLOAT(n, factor) ( (n) * (factor) + (((n) > 0.0f) ? 0.5f : -0.5f) )

#define MIN_SHORT -32768.0f
#define MAX_SHORT  32767.0f
#define FLOAT_TO_SHORT(n)      (short)(MOS_MIN(MOS_MAX(MIN_SHORT, n), MAX_SHORT))

#define DL_MAX_SEARCH_FILTER_SIZE       10  // max number of entries to describe a compositing filter

#define DL_MAX_KERNELS           150 // Max component kernels to combine
#define DL_MAX_PATCH_DATA_SIZE   64  // Max size of a patch block
#define DL_MAX_PATCH_BLOCKS      8   // Max number of blocks to patch per patch data
#define DL_MAX_PATCHES           4   // Max patches to use

#define DL_MAX_SEARCH_NODES_PER_KERNEL  6        // max number of search nodes for a component kernel (max tree depth)
#define DL_MAX_COMPONENT_KERNELS        25       // max number of component kernels that can be combined
#define DL_MAX_EXPORT_COUNT             64       // size of the symbol export table
#define DL_DEFAULT_COMBINED_KERNELS     4        // Default number of kernels in cache
#define DL_MAX_COMBINED_KERNELS         64       // Max number of kernels in cache
#define DL_NEW_COMBINED_KERNELS         4        // The increased number of kernels in cache each time
#define DL_MAX_SYMBOLS                  100      // max number of import/export symbols in a combined kernels
#define DL_CACHE_BLOCK_SIZE             (128*1024)   // Kernel allocation block size
#define DL_MAX_KERNEL_SIZE              (128*1024)   // max output kernel size
#define DL_COMBINED_KERNEL_CACHE_SIZE   (DL_CACHE_BLOCK_SIZE*DL_NEW_COMBINED_KERNELS) // Combined kernel size

#define DL_PROCAMP_DISABLED             -1       // procamp is disabled
#define DL_PROCAMP_MAX                   1       // 1 Procamp entry

#define DL_CSC_DISABLED                 -1       // CSC is disabled
#define DL_CSC_MAX                       6       // 6 CSC matrices max

#define DL_CSC_MAX_G5                    2       // 2 CSC matrices max for Gen5

#define DL_CHROMASITING_DISABLE         -1       // Chromasiting is disabled

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum _MOS_FORMAT            Kdll_Format;
typedef enum _VPHAL_CSPACE          Kdll_CSpace;

// Layer definition
typedef enum _KDLL_LAYER
{
    Layer_Invalid      = -2,      // Invalid
    Layer_None         = -1,      // None
    Layer_Background   =  0,      // Background      (0)
    Layer_MainVideo    =  1,      // Main video      (1)
    Layer_SubVideo     =  2,      // Secondary video (2)
    Layer_SubPicture1  =  3,      // Sub-picture 1   (3)
    Layer_SubPicture2  =  4,      // Sub-picture 2   (4)
    Layer_SubPicture3  =  5,      // Sub-picture 3   (5) [Gen6]
    Layer_SubPicture4  =  6,      // Sub-picture 4   (6) [Gen6]
    Layer_Graphics     =  14,     // Graphics        (7)
    Layer_RenderTarget =  15      // Render Target
} Kdll_Layer;

// Sampling mode
// Please don't change the order in the enum is setup
typedef enum tagKdll_Sampling
{
    Sample_None                   = -2,
    Sample_Source                 = -1,  // Current source sampling
    Sample_Any                    =  0,
    Sample_Scaling_Any                ,  // Scaling (any scaling factor)
    Sample_Scaling                    ,  // Scaling (above or equal 0.34x)
    Sample_Scaling_034x               ,  // Scaling 0.34x (below 0.34x)
    Sample_iScaling                   ,  // Scaling (above or equal 0.34x)
    Sample_iScaling_034x              ,  // Scaling 0.34x (below 0.34x)
    Sample_iScaling_AVS                , //AVS Interlace Scaling on g75+
    Sample_Scaling_AVS                ,  // AVS Scaling on g575+
} Kdll_Sampling;

// Rotation Mode
typedef enum tagKdll_Rotation
{
    Rotate_Source
} Kdll_Rotation;

// Gen7+ : Shuffle data returned by Sample_8x8 message
typedef enum tagKdll_Shuffling
{
    Shuffle_None            = -1,
    Shuffle_Any             =  0,
    Shuffle_All_8x8_Layer   ,       // Mix of Sample_8x8 and others. Shuffle all 8x8 layer.
    Shuffle_RenderTarget            // All layers are Sample_8x8. Shuffle only in RenderTarget
} Kdll_Shuffling;

// Gen7+ : Rendering method
typedef enum tagKdll_RenderMethod
{
    RenderMethod_MediaObject        = 0,
    RenderMethod_MediaObjectWalker  = 1
} Kdll_RenderMethod;

// Gen7+ : Set CSC Coefficients method
typedef enum tagKdll_SetCSCCoeffMethod
{
    SetCSCCoeffMethod_Curbe       = 0,
    SetCSCCoeffMethod_Patch       = 1
} Kdll_SetCSCCoeffMethod;

#define ColorFill_Source        -1
#define ColorFill_False         0
#define ColorFill_True          1

#define LumaKey_Source          -1
#define LumaKey_False           0
#define LumaKey_True            1

#define Procamp_Source          -1

// Processing mode
typedef enum tagKdll_Processing
{
    Process_None        = -2,
    Process_Source      = -1,     // Current source processing
    Process_Any         =  0,
    Process_Composite   ,         // Composite 2 layers
    Process_PBlend      ,         // Partial Blend 2 layers  - 8-bits alpha
    Process_CBlend      ,         // Constant Blend 2 layers - 8-bits alpha
    Process_SBlend      ,         // Source Blend 2 layers   - 8-bits alpha
    Process_SBlend_4bits,         // Source Blend 2 layers   - 4-bits alpha
    Process_CSBlend     ,         // Constant multiply Source Blend 2 layers - 8-bits alpha (Source is NOT premultiplied)
    Process_CPBlend     ,         // Constant multiply Source Blend 2 layers - 8-bits alpha (Source is Premultiplied)
    Process_DI          ,         // Deinterlacing
    Process_DN          ,         // Denoise
    Process_DNDI                  // DNDI
} Kdll_Processing;

typedef enum tagKdll_CSCType
{
    CSC_YUV_RGB    = 0,          // YUV to RGB color space conversion
    CSC_RGB_YUV    = 1,          // RGB to YUV color space conversion
    CSC_YUV_YUV    = 2,          // YUV to YUV color space conversion
    CSC_RGB_RGB    = 3           // RGB to RGB color space conversion
} Kdll_CSCType;

// CSC Coefficients ID
// Worst case we could have any many CSCs are there are number of layers (xvYcc case). So, 6 CoeffIDs defined to support 6 layers max
typedef enum tagKdll_CoeffID
{
    CoeffID_Src0   = -5,   // Src0 coefficients
    CoeffID_Src1   = -4,   // Src1 coefficients
    CoeffID_Source = -3,   // Current source coefficients
    CoeffID_Any    = -2,   // Any matrix
    CoeffID_None   = -1,   // No matrix -> no CSC
    CoeffID_0      =  0,
    CoeffID_1      =  1,
    CoeffID_2      =  2,
    CoeffID_3      =  3,
    CoeffID_4      =  4,
    CoeffID_5      =  5
} Kdll_CoeffID;

// DL Parser State
typedef enum tagKdll_ParserState
{
    Parser_Invalid         = -1,    // invalid state
    Parser_Begin           =  0,    // start dynamic linking
    Parser_SetRenderMethod,         // MEDIA_OBJECT or MEDIA_OBJECT_WALKER rendering
    Parser_SetupLayer0     ,        // setup layer 0
    Parser_SetupLayer1     ,        // setup layer 1
    Parser_SetParamsLayer0 ,        // setup parameters for layer 0
    Parser_SetParamsLayer1 ,        // setup parameters for layer 1
    Parser_SetParamsTarget ,        // setup parameters for render target
    Parser_SampleLayer0    ,        // sample layer 0
    Parser_SampleLayer0Mix ,        // sample layer 0 need inter mix (interlaced scaling)
    Parser_SampleLayer0ColorFill,   // sample layer 0 colorfill
    Parser_RotateLayer0Check,       // check if layer 0 needs preComp rotation
    Parser_RotateLayer0    ,        // PreComp layer 0 rotate
    Parser_SampleLayer0Done,        // sample layer 0 is complete
    Parser_ShuffleLayer0   ,        // shuffle layer 0
    Parser_SampleLayer1    ,        // sample layer 1
    Parser_SampleLayer1Done,        // sample layer 1 is complete
    Parser_ShuffleLayer1   ,        // shuffle layer 1
    Parser_SampleLayer0SelectCSC,   // decided whether CSC be selected for Mix or jump to Parser_SampleLayer0Mix directly
    Parser_SetupCSC0       ,        // CSC parameters setup for layer 0
    Parser_ExecuteCSC0     ,        // CSC on layer 0
    Parser_ExecuteCSC0Done ,        // CSC on layer 0 is complete
    Parser_SetupCSC1       ,        // CSC parameter setup for layer 1
    Parser_ExecuteCSC1     ,        // CSC on layer 1
    Parser_ExecuteCSC1Done ,        // CSC on layer 1 is complete
    Parser_Lumakey         ,        // lumakey
    Parser_ProcessLayer    ,        // process layer 0 + layer 1
    Parser_ProcessLayerDone,        // processing is complete
    Parser_DualOutput      ,        // dual output
    Parser_Rotation        ,        // apply post composition rotation
    Parser_DestSurfIndex   ,        // destination surface index
    Parser_Colorfill       ,        // applying colorfill
    Parser_WriteOutput     ,        // write output
    Parser_End             ,        // end dynamic linking

    // This is used for custom states
    Parser_Custom          ,        // Custom state

    // Number of states - keep this at the end
    Parser_Count
} Kdll_ParserState;

// Dynamic Linking rule definitions
#define RID_IS_MATCH(rid)    ((rid & 0xFE00) == 0x0000)
#define RID_IS_SET(rid)      ((rid & 0xFE00) == 0x0200)
#define RID_IS_EXTENDED(rid) ((rid & 0xFD00) == 0x0100)

typedef enum tagKdll_RuleID
{
    // Parser/Patch Control
    RID_Op_EOF           = -2, // End of search table
    RID_Op_NewEntry      = -1, // New search entry

    // Simple Match Rules - 0x0000 to 0x0100
    RID_IsTargetCspace   = 0,  // Match target color space
    RID_IsLayerID        ,     // Match Layer ID
    RID_IsLayerFormat    ,     // Match Layer Format
    RID_IsParserState    ,     // Match Parser State
    RID_IsRenderMethod   ,     // Match rendering mode, media object or media walker
    RID_IsShuffling      ,     // Match Shuffling
    RID_IsDualOutput     ,     // Match Dual Output
    RID_IsLayerRotation  ,     // Match Rotation
    RID_IsRTRotate       ,     // Match if RT rotates
    RID_IsSrc0Format     ,     // Current Src0 source (surface) format
    RID_IsSrc0Sampling   ,     // Current Src0 sampling mode
    RID_IsSrc0Rotation ,       // Match Layer0 Rotation
    RID_IsSrc0ColorFill  ,     // Current Src0 Colorfill flag
    RID_IsSrc0LumaKey    ,     // Current Src0 LumaKey flag
    RID_IsSrc0Procamp    ,     // Match Src0 Procamp flag
    RID_IsSrc0Coeff      ,     // Current Src0 CSC coefficients
    RID_IsSrc0Processing ,     // Current Src0 processing mode
    RID_IsSrc0Chromasiting,    // Current Src0 Chromasiting mode
    RID_IsSrc1Format     ,     // Current Src1 source (surface) format
    RID_IsSrc1Sampling   ,     // Current Src1 sampling mode
    RID_IsSrc1LumaKey    ,     // Current Src1 LumaKey flag
    RID_IsSrc1SamplerLumaKey,  // Current Src1 Samper LumaKey flag
    RID_IsSrc1Procamp    ,     // Match Src1 Procamp flag
    RID_IsSrc1Coeff      ,     // Current Src1 CSC coefficients
    RID_IsSrc1Processing ,     // Current Src1 processing mode
    RID_IsSrc1Chromasiting,    // Current Src1 Chromasiting mode
    RID_IsLayerNumber    ,     // Current Layer number
    RID_IsQuadrant       ,     // Current Quadrant
    RID_IsCSCBeforeMix   ,     // CSC needed before Mix
    RID_IsTargetFormat   ,     // Render Target Format
    RID_Is64BSaveEnabled ,     // Indicate whether 64B save kernel could be used
    RID_IsTargetTileType ,     // Render Target Tile Type
    RID_IsProcampEnabled ,     // Match Procamp
    RID_IsSetCoeffMode   ,     // Set CSC coefficients mode
    RID_IsConstOutAlpha  ,     // Match alpha fill mode
    RID_IsDitherNeeded   ,     // Whether dithering needed

    // Extended Match Rules - 0x0100 to 0x01ff

    // Simple Set Rules - 0x0200 to 0x02ff
    RID_SetTargetCspace = 512, // Set target color space
    RID_SetParserState   ,     // Set Parser State
    RID_SetSrc0Format    ,     // Set Src0 source format
    RID_SetSrc0Sampling  ,     // Set Src0 sampling mode
    RID_SetSrc0Rotation  ,     // Set Src0 rotation
    RID_SetSrc0ColorFill ,     // Set Src0 Colorfill
    RID_SetSrc0LumaKey   ,     // Set Src0 LumaKey
    RID_SetSrc0Procamp   ,     // Set Src0 Procamp flag
    RID_SetSrc0Coeff     ,     // Set Src0 CSC coefficients
    RID_SetSrc0Processing,     // Set Src0 Processing mode
    RID_SetSrc1Format    ,     // Set Src1 source format
    RID_SetSrc1Sampling  ,     // Set Src1 sampling mode
    RID_SetSrc1Rotation  ,     // Set Src1 rotation
    RID_SetSrc1LumaKey   ,     // Set Src1 LumaKey
    RID_SetSrc1SamplerLumaKey, // Set Src1 Sampler LumaKey
    RID_SetSrc1Procamp   ,     // Set Src1 Procamp flag
    RID_SetSrc1Coeff     ,     // Set Src1 CSC coefficients
    RID_SetSrc1Processing,     // Set Src1 Processing mode
    RID_SetKernel        ,     // Set Kernel
    RID_SetNextLayer     ,     // Set Next Layer
    RID_SetPatchData     ,     // Set Patch Data to use
    RID_SetQuadrant      ,     // Set Quadrant
    RID_SetCSCBeforeMix  ,     // Set CSC flag before Mix

    // Extended Set Rules - 0x0300 to 0x03ff
    RID_SetPatch       = 768,  // Set Patch block
} Kdll_RuleID;

typedef enum tagKdll_Logic
{
    Kdll_None,
    Kdll_Or,
    Kdll_Not,
} Kdll_Logic;

// Parameters for RID_Op_NewEntry
#define RULE_DEFAULT        0
#define RULE_CUSTOM         1
#define RULE_NO_OVERRIDE    255

#define GROUP_DEFAULT       RULE_DEFAULT
#define GROUP_CUSTOM        RULE_CUSTOM
#define GROUP_NO_OVERRIDE   RULE_NO_OVERRIDE

//--------------------------------------------------------------
// Kernel DLL structures
//--------------------------------------------------------------

// Dynamic linking rule entry
typedef struct tagKdll_RuleEntry
{
    Kdll_RuleID     id    : 16;     // LSB
    int             value : 16;     // MSB
    Kdll_Logic      logic;
} Kdll_RuleEntry;

typedef struct tagKdll_RuleEntrySet
{
    const Kdll_RuleEntry *pRuleEntry;         // Pointer to the first meaningful rule of the set
    uint32_t              iGroup      :  8;   // Group (default, custom, non-overridable)
    uint32_t              iMatchCount : 12;   // Size of Match Rules (including variable length rules)
    uint32_t              iSetCount   : 12;   // Size of Set Rules (including variable length rules)
} Kdll_RuleEntrySet;

// Structure that defines a set of procamp parameters
typedef struct tagKdll_Procamp
{
    bool      bEnabled;         // Procamp Enabled/Disabled
    int       iProcampVersion;  // Procamp parameters version
    float     fBrightness;      // Brightness : range = -100.0 - 100.0; default = 0.0; step = 0.1
    float     fContrast;        // Contrast   : range =    0.0 -  10.0; default = 1.0; step = 0.01
    float     fHue;             // Hue        : range = -180.0 - 180.0; default = 0.0; step = 0.1
    float     fSaturation;      // Saturation : range =    0.0 -  10.0; default = 1.0; step = 0.01
} Kdll_Procamp;

// Structure that defines CSC+PA conversion matrix
typedef struct tagKdll_CSC_Matrix
{
    int           bInUse     : 1;   // Matrix is in use and valid (LSB)
    int                      : 3;
    Kdll_CoeffID  iCoeffID   : 4;   // Coeffient set
    VPHAL_CSPACE  SrcSpace   : 8;   // Source Color Space
    VPHAL_CSPACE  DstSpace   : 8;   // Destionation Color Space
    int           iProcampID : 8;   // Procamp parameter set (-1 if no Procamp) (MSB)
    int           iProcampVersion;  // Last procamp version (to recalculate matrix)
    short         Coeff[12];        // CSC kernel coeff: [Y'/R']   [0  1  2]   [Y/R]   [ 3]
                                    //                   [U'/G'] = [4  5  6] * [U/G] + [ 7]
                                    //                   [V'/B']   [8  9 10]   [V/B]   [11]
} Kdll_CSC_Matrix;

// Structure that defines a full set of CSC or CSC+PA parameters to be used by a combined kernel
typedef struct tagKdll_CSC_Params
{
    VPHAL_CSPACE      ColorSpace;                // Selected Color Space
    Kdll_CSC_Matrix   Matrix  [DL_CSC_MAX];      // CSC conversion matrix (3x3 + 1x3)
    uint8_t           MatrixID[DL_CSC_MAX];      // Coefficient allocation array
    uint8_t           PatchMatrixID[DL_CSC_MAX]; // CSC Matrix ID
    uint8_t           PatchMatrixNum;            // CSC Matrix Number
} Kdll_CSC_Params;

// Structure that defines a compositing layer
typedef struct tagKdll_FilterEntry
{
    // Current layer
    Kdll_Layer      layer;            // source layer       (Layer identification - Bg, Main, Sub, Gfx, ...)
    MOS_FORMAT      format;           // source format      (Pixel/Sampling Format - ARBG, NV12, YUY2, ...)
    VPHAL_CSPACE    cspace;           // source color space (BT709, BT601, xvYCC709, xvYCC601, sRGB, ...)
    Kdll_Sampling   sampler;          // sampling mode      (AVS, Scaling, ColorFill, Luma Keying, ...)
    int32_t         colorfill : 16;   // colorfill          (true/false)
    int32_t         lumakey   : 16;   // Luma key           (true/false)
    int32_t         samplerlumakey;   // Sampler Lumakey    (true/false)
    Kdll_Processing process;          // processing mode    (Compositing, Constant Blending, Source Blending, ...)
    int             procamp;          // index to procamp parameters (-1 of Procamp disabled)
    int             matrix;           // index to CSC matrix entry   (-1 if CSC not required)
    VPHAL_ROTATION  rotation;         // rotation angle
    MOS_TILE_TYPE   tiletype;         // Tiling Type
    bool            dualout;          // dual output mode
    bool            bWaEnableDscale;  // enable DScale kernels for sampler-unrom issue
    bool            bEnableDscale;    // always enable DScale Kernels
    int32_t         chromasiting;     // chromasiting        (-1 if Chromasiting is disabled)

    // This flag is used to select between kernels:
    // Save_RGB         or     Save_ARGB
    // Save_R10G10B10   or     Save_R10G10B10A2
    // Save_VUYA        or     Save_SrcVUYA
    bool            bFillOutputAlphaWithConstant;
    bool            bIsDitherNeeded;

    Kdll_RenderMethod      RenderMethod;
    Kdll_SetCSCCoeffMethod SetCSCCoeffMode;
} Kdll_FilterEntry, *PKdll_FilterEntry;

// Structure that defines a compositing filter
typedef Kdll_FilterEntry Kdll_FilterDesc[DL_MAX_SEARCH_FILTER_SIZE];

//------------------------------------------------------------
// Component kernel descriptors (equivalent to KDT)
//------------------------------------------------------------
// Component kernel linking information
typedef struct tagKdll_Linking
{
    int              iKUID;                 // Component Kernel Unique ID
    uint32_t         bExport         :  1;  // Export (1) / Import (0)
    uint32_t         bInline         :  1;  // Inline(1)  / Function (0)
    uint32_t                         :  2;  // - (MBZ)
    uint32_t         iLabelID        : 12;  // Label ID
    uint32_t         dwOffset        : 16;  // Instruction offset
} Kdll_Linking, *pKdll_Linking;

// Kernel patches
typedef enum tagKdll_PatchKind
{
    PatchKind_None           = 0,
    PatchKind_CSC_Coeff_Src0 = 1,
    PatchKind_CSC_Coeff_Src1 = 2,
} Kdll_PatchKind;

typedef struct tagKdll_PatchBlock
{
    uint32_t DstOffset : 16;
    uint32_t SrcOffset : 8;
    uint32_t BlockSize : 8;
} Kdll_PatchBlock;

// Kernel Patching data
typedef struct tagKdll_PatchData
{
    // Data for patching
    int             iPatchDataSize;                     // Size of Patch data block
    uint8_t         Data[DL_MAX_PATCH_DATA_SIZE];       // Patch data

    // Patches
    int             nPatches;
    Kdll_PatchBlock Patch[DL_MAX_PATCH_BLOCKS];
} Kdll_PatchData;

// Patch rule entry (rule extension)
typedef struct tagKdll_PatchRuleEntry
{
    uint32_t Dest   : 16 ;   // Patch destination in bytes (LSB)
    uint32_t Source : 8  ;   // Patch data source in bytes
    uint32_t Size   : 8  ;   // Patch size in bytes (MSB)
} Kdll_PatchRuleEntry;

extern const char  *g_cInit_ComponentNames[];

//------------------------------------------------------------
// KERNEL CACHE / LINK
//------------------------------------------------------------
// Import/export structure from kernel binary file
#pragma pack(4)
typedef struct tagKdll_LinkFileHeader
{
    uint32_t dwVersion;
    uint32_t dwSize;
    uint32_t dwImports;
    uint32_t dwExports;
} Kdll_LinkFileHeader;

typedef struct tagKdll_LinkData
{
    uint32_t iKUID         : 16;     // Kernel Unique ID
    uint32_t iLabelID      : 16;     // Label ID
    uint32_t bExport       : 1;      // 0 - import; 1 - export;
    uint32_t bResolved     : 1;      // 0 - unresolved; 1 - resolved;
    uint32_t dwOffset      : 20;     // Offset in DWORDs
    uint32_t bInline       : 1;      // 0 - function; 1 - inline;
    uint32_t              : 9;      // MBZ
} Kdll_LinkData;
#pragma pack()

typedef struct tagKdll_Symbol
{
    uint32_t       dwSize;
    uint32_t       dwCount;
    Kdll_LinkData *pLink;
} Kdll_Symbol;

typedef struct tagKdll_CacheEntry
{
    // Kernel binary
    uint8_t          *pBinary;           // kernel binary
    int               iSize;             // kernel size

    // Component kernel information
    int               iKUID;             // kernel unique id (static kernel)
    const char       *szName;            // kernel name
    int               nLink;             // Number of imports/exports
    Kdll_LinkData    *pLink;             // Kernel imports/exports

    // Combined kernel information
    uint16_t          wHashEntry;        // hash table entry
    int               iFilterSize;       // kernel filter size
    Kdll_FilterEntry *pFilter;           // kernel filter description
    Kdll_CSC_Params  *pCscParams;        // kernel CSC parameters
    VPHAL_CSPACE      colorfill_cspace;  // intermediate color space for colorfill

    // Cache control
    int               iKCID;             // kernel cache id (dynamically linked kernel)
    uint32_t          dwLoaded;          // kernel loaded flag
    uint32_t          dwRefresh;         // refresh counter (for expiration control)

    struct tagKdll_CacheEntry *pNextEntry;    // Next cache entry;
} Kdll_CacheEntry;

typedef struct tagKdll_KernelCache
{
    int              iCacheMaxEntries;  // Max number of entries
    int              iCacheEntries;     // Current number of cache entries
    int              iCacheSize;        // Cache buffer size
    int              iCacheFree;        // Cache buffer free
    int              iCacheID;          // Next kernel cache ID
    Kdll_CacheEntry *pCacheEntries;     // Array of kernel cache entries
    uint8_t         *pCache;            // Cache (binary data)
    int              nExports;          // Exports count
    Kdll_LinkData   *pExports;          // Exports table
} Kdll_KernelCache;

//--------------------------------------------------------------
// Kernel Hash table
//--------------------------------------------------------------
typedef struct tagKdll_KernelHashEntry
{
    uint16_t            next;              // Next entry with same 8-bit hash + 1 (0 is null)
    uint32_t            dwHash;            // 32-bit hash value (FNV-1a hash)
    int                 iFilter;           // Filter size
    Kdll_FilterEntry    *pFilter;          // Filter for matching
    Kdll_CacheEntry     *pCacheEntry;      // Pointer to kernel cache entry
} Kdll_KernelHashEntry;

typedef struct tagKdll_KernelHashTable
{
    uint16_t             wHashTable[256];   // 256 hashes (1 based index)
    uint16_t             pool;              // first in pool (1 based index)
    uint16_t             last;              // last in pool (for releasing)
    Kdll_KernelHashEntry HashEntry[DL_MAX_COMBINED_KERNELS]; // Hash table entries
} Kdll_KernelHashTable;

//--------------------------------------------------------------
// Dynamic linking state
//--------------------------------------------------------------
typedef struct tagKdll_State       *PKdll_State;
typedef struct tagKdll_SearchState *PKdll_SearchState;

typedef struct tagKdll_State
{
    int                     iSize;                  // Size of DL buffer
    uint32_t                dwRefresh;              // Refresh counter (for garbage collection)
    bool                    bEnableCMFC;            // Flag to enable CMFC

    // Default kernel component cache and rule table
    Kdll_KernelCache        ComponentKernelCache;   // Component kernels cache
    const Kdll_RuleEntry    *pRuleTableDefault;     // Default Dll rules (internal)

    // CMFC kernel fcpatch cache
    Kdll_KernelCache        CmFcPatchCache;         // CMFC kernel fcpatch cache

    // Custom kernel component cache and rule table
    Kdll_KernelCache        *pCustomKernelCache;    // Custom kernel cache
    const Kdll_RuleEntry    *pRuleTableCustom;      // Custom Dll rules (external)

    // Combined rule lookup table
    Kdll_RuleEntrySet       *pSortedRules;          // Sorted rule table

    Kdll_RuleEntrySet       *pDllRuleTable[Parser_Count]; // Rule acceleration table (one entry for each Parser State)
    int                     iDllRuleCount[Parser_Count]; // Rule count (number of entries for each Parser State)

    // Combined kernel cache and hash table
    Kdll_KernelCache        KernelCache;            // Output kernel cache
    Kdll_KernelHashTable    KernelHashTable;        // Hash table for resulting kernels

    Kdll_Procamp            *pProcamp;              // Array of Procamp parameters
    int32_t                 iProcampSize;           // Size of the array of Procamp parameters

    // Colorfill
    VPHAL_CSPACE            colorfill_cspace;       // Selected colorfill Color Space by Kdll

    // Start kernel search
    void                 (* pfnStartKernelSearch)(PKdll_State       pState,
                                                  PKdll_SearchState pSearchState,
                                                  Kdll_FilterEntry *pFilter,
                                                  int32_t           iFilterSize,
                                                  uint32_t          uiIs64BInstrEnabled);

    // Find best ColorSpace to use internally, allocate/calculate CSC matrices and arguments
    bool                 (* pfnSetupCSC)     (PKdll_State       pState,
                                              PKdll_SearchState pSearchState);

    // Find rule that matches the current search state
    bool                 (* pfnFindRule)     (PKdll_State       pState,
                                              PKdll_SearchState pSearchState);

    // Update state based on rule
    bool                 (* pfnUpdateState)  (PKdll_State       pState,
                                              PKdll_SearchState pSearchState);

    // Search Kernel based for a given
    bool                 (* pfnSearchKernel) (PKdll_State       pState,
                                              PKdll_SearchState pSearchState);

    // Build current best match kernel
    bool                 (* pfnBuildKernel)  (PKdll_State       pState,
                                              PKdll_SearchState pSearchState);

    // Map matrix to kernel CSC
    bool                 (* pfnMapCSCMatrix) (Kdll_CSCType      type,
                                              const float      *matrix,
                                              short            *coeff);
#if EMUL
    // Token to be passed back in Callbacks
    void *pToken;

    // Print Component Kernel
    void                 (* pfnCbListKernel) (void              *token,
                                              const char        *szKernel);

    // State update
    void                 (* pfnCbSearchSate) (void              *token,
                                              int32_t           reason,
                                              PKdll_SearchState pSearchState);
#endif
} Kdll_State;

typedef struct tagKdll_SearchState
{
    // Kernel DLL state
    Kdll_State          *pKdllState;                // Kernel DLL State

    // Filter and CSC parameters
    Kdll_FilterDesc      Filter;                    // Output Filter, with CSC data
    int                  iFilterSize;               // Size of the filter
    Kdll_CSC_Params      CscParams;                 // CSC parameters
    bool                 bCscBeforeMix;             // flag to identify if CSC needed before Mix
    Kdll_Shuffling       ShuffleSamplerData;        // Gen7+ Shuffle sampler output

    // RT Rotate
    bool                 bRTRotate;

    // Procamp
    bool                 bProcamp;

    // Search output
    Kdll_RuleEntrySet   *pMatchingRuleSet;          // Pointer to the matching rule set

    // Kernels
    int                  KernelCount;               // # of kernels
    int                  KernelID[DL_MAX_KERNELS];  // Array of kernel ids
    int                  KernelGrp[DL_MAX_KERNELS]; // Array of kernel groups
    int                  PatchID [DL_MAX_KERNELS];  // Array of patches

    // Kernel patches
    int                  PatchCount;                // Number of patches
    Kdll_PatchData       Patches[DL_MAX_PATCHES];   // Kernel patches

    // Current state
    Kdll_FilterEntry    *pFilter;                   // Current filter entry
    Kdll_ParserState     state;                     // Parser state
    VPHAL_CSPACE          cspace;                   // Destination color space
    int                  quadrant;                  // Current quadrant
    int                  layer_number;              // Current layer number

    // Src0 state
    MOS_FORMAT           src0_format;               // Src0 source format
    Kdll_Sampling        src0_sampling;             // Src0 sampling mode
    int32_t              src0_colorfill;            // Src0 colorfill flag
    int32_t              src0_lumakey;              // Src0 luma key
    int32_t              src0_procamp;              // Src0 procamp
    Kdll_CoeffID         src0_coeff;                // Src0 CSC coefficiants
    Kdll_Processing      src0_process;              // Src0 processing mode
    VPHAL_ROTATION       src0_rotation;             // Src0 Rotate

    // Src1 state
    MOS_FORMAT           src1_format;               // Src1 source format
    Kdll_Sampling        src1_sampling;             // Src1 sampling mode
    int32_t              src1_lumakey;              // Src1 luma key
    int32_t              src1_samplerlumakey;       // Src1 sampler luma key
    int32_t              src1_procamp;              // Src1 procamp
    Kdll_CoeffID         src1_coeff;                // Src1 CSC coefficients
    Kdll_Processing      src1_process;              // Src1 processing mode
    VPHAL_ROTATION       src1_rotation;             // Src1 Rotate

    // Render Target Format
    MOS_FORMAT           target_format;             // Render Target format

    bool                 b64BSaveEnabled;           // Whether to use 64B save kernel
    MOS_TILE_TYPE        target_tiletype;           // Render Target Tile Type

    // Dynamic linking
    int                  KernelSize;                // Kernel Size
    int                  KernelLeft;                // Remaining size
    Kdll_Symbol          KernelLink;                // DL symbols for linking
    Kdll_LinkData        LinkArray[DL_MAX_SYMBOLS]; // Import/Export symbols for dynamic linking
    uint8_t              Kernel[DL_MAX_KERNEL_SIZE];// Output Kernel
} Kdll_SearchState;

//---------------------------------
// Kernel DLL function prototypes
//---------------------------------

bool KernelDll_IsYUVFormat(MOS_FORMAT   format);

bool KernelDll_IsFormat(
    MOS_FORMAT      format,
    VPHAL_CSPACE     cspace,
    MOS_FORMAT      match);

bool KernelDll_IsCspace(
    VPHAL_CSPACE     cspace,
    VPHAL_CSPACE     match);

VPHAL_CSPACE KernelDll_TranslateCspace(VPHAL_CSPACE cspace);

void KernelDll_GetCSCMatrix(
    Kdll_CSpace     src,
    Kdll_CSpace     dst,
    float           *pCSC_Matrix);

bool KernelDll_MapCSCMatrix(
    Kdll_CSCType     type,
    const float      *matrix,
    short            *coeff);

// Kernel Rule Search / State Update
bool KernelDll_FindRule(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState);

bool KernelDll_UpdateState(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState);

// Simple Hash function
uint32_t KernelDll_SimpleHash(
    void            *pData,
    int             iSize);

//---------------------------------------------------------------------------------------
// KernelDll_SetupFunctionPointers - Setup Function pointers based on platform
//
// Parameters:
//    char  *pState    - [in] Kernel Dll state
//           platform  - [in] platform
//
// Output: true  - Function pointers are set
//         false - Failed to setup function pointers (invalid platform)
//-----------------------------------------------------------------------------------------
static bool KernelDll_SetupFunctionPointers(
    Kdll_State  *pState,
    void(*ModifyFunctionPointers)(PKdll_State));

// Allocate Kernel Dll State
Kdll_State *KernelDll_AllocateStates(
    void                 *pKernelCache,
    uint32_t             uKernelCacheSize,
    void                 *pFcPatchCache,
    uint32_t             uFcPatchCacheSize,
    const Kdll_RuleEntry *pInternalRules,
    void(*ModifyFunctionPointers)(PKdll_State));

// Release Kernel Dll State
void  KernelDll_ReleaseStates(Kdll_State *pState);

// Setup Kernel Dll Procamp Parameters
void KernelDll_SetupProcampParameters(Kdll_State    *pState,
                                      Kdll_Procamp  *pProcamp,
                                      int            iProcampSize);

// Update CSC coefficients
void KernelDll_UpdateCscCoefficients(Kdll_State      *pState,
                                     Kdll_CSC_Matrix *pMatrix);

// Find Kernel in hash table
Kdll_CacheEntry *
KernelDll_GetCombinedKernel(Kdll_State       *pState,
                            Kdll_FilterEntry *iFilter,
                            int               iFilterSize,
                            uint32_t          dwHash);

// Get component/static kernel
Kdll_CacheEntry *
KernelDll_GetComponentKernel(Kdll_State *pState,
                             int         iKUID);

// Allocate cache entry for a given size
Kdll_CacheEntry *
KernelDll_AllocateCacheEntry(Kdll_KernelCache *pCache,
                             int32_t           iSize);

// Allocate more kernel cache entries
Kdll_CacheEntry *
KernelDll_AllocateAdditionalCacheEntries(Kdll_KernelCache *pCache);

//Release the additional kernel cache entries
void KernelDll_ReleaseAdditionalCacheEntries(Kdll_KernelCache *pCache);

// Add kernel to cache and hash table
Kdll_CacheEntry *
KernelDll_AddKernel(Kdll_State       *pState,
                    Kdll_SearchState *pSearchState,
                    Kdll_FilterEntry *pFilter,
                    int               iFilterSize,
                    uint32_t          dwHash);

// Search kernel, output is in pSearchState
bool KernelDll_SearchKernel(
    Kdll_State          *pState,
    Kdll_SearchState    *pSearchState);

// Build kernel in SearchState
bool KernelDll_BuildKernel(Kdll_State *pState, Kdll_SearchState *pSearchState);

bool KernelDll_SetupCSC(
    Kdll_State       *pState,
    Kdll_SearchState *pSearchState);

bool KernelDll_IsSameFormatType(MOS_FORMAT   format1, MOS_FORMAT   format2);
void KernelDll_ReleaseHashEntry(Kdll_KernelHashTable *pHashTable, uint16_t entry);
void KernelDll_ReleaseCacheEntry(Kdll_KernelCache *pCache, Kdll_CacheEntry  *pEntry);

//---------------------------------------------------------------------------------------
// KernelDll_SetupFunctionPointers_Ext - Setup Extension Function pointers
//
// Parameters:
//    KdllState  *pState    - [in/out] Kernel Dll state
//
// Output: true  - Function pointers are set
//         false - Failed to setup function pointers (invalid platform)
//-----------------------------------------------------------------------------------------
bool KernelDll_SetupFunctionPointers_Ext(
    Kdll_State  *pState);

#if _DEBUG || EMUL

// Debugging strings for standalone application or debug driver
const char    *KernelDll_GetLayerString        (Kdll_Layer       layer);
const char    *KernelDll_GetFormatString       (MOS_FORMAT       format);
const char    *KernelDll_GetCSpaceString       (VPHAL_CSPACE     cspace);
const char    *KernelDll_GetSamplingString     (Kdll_Sampling    sampling);
const char    *KernelDll_GetRotationString     (VPHAL_ROTATION   rotation);
const char    *KernelDll_GetProcessString      (Kdll_Processing  process);
const char    *KernelDll_GetParserStateString  (Kdll_ParserState state);
const char    *KernelDll_GetRuleIDString       (Kdll_RuleID      RID);
const char    *KernelDll_GetCoeffIDString      (Kdll_CoeffID     CID);

int32_t KernelDll_PrintRule(
    char                    *szOut,
    int                     iSize,
    const Kdll_RuleEntry    *pEntry,
    Kdll_KernelCache        *pCache);

#endif // _DEBUG || EMUL

#ifdef __cplusplus
}
#endif

#endif // __HAL_KERNELDLL_H__
