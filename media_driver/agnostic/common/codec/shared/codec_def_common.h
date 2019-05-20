/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codec_def_common.h
//! \brief    Defines common types and macros shared by CodecHal, MHW, and DDI layer
//! \details  This is the base header for all codec_def files. All codec_def may include this file which should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_COMMON_H__
#define __CODEC_DEF_COMMON_H__

#include "mos_defs.h"
#include "mos_os.h"
#include <math.h>

#define CODEC_MAX_NUM_REF_FRAME             16
#define CODEC_MAX_NUM_REF_FRAME_NON_AVC     4
#define CODEC_NUM_FIELDS_PER_FRAME          2

#define CODEC_NUM_BLOCK_PER_MB              6  //!<  Block number per MB: 4Y + Cb +Cr

// picture coding type
#define I_TYPE          1
#define P_TYPE          2
#define B_TYPE          3
#define NUM_PIC_TYPES   3

#define CODECHAL_MACROBLOCK_HEIGHT          16
#define CODECHAL_MACROBLOCK_WIDTH           16

#define CODECHAL_STATUS_QUERY_SKIPPED    0x00
#define CODECHAL_STATUS_QUERY_START_FLAG 0x01
#define CODECHAL_STATUS_QUERY_END_FLAG   0xFF

#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1   128
#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2 128
#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_JPEG  1
#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8   128
#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC  127 // 7 bits, 0x7f is invalid one
#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9   128

#define WIDTH_IN_DW(w)  ((w + 0x3) >> 2)

#define ABS(a)          (((a) < 0) ? (-(a)) : (a))
#define SIGNED(code)    (2 * ABS(code) - ((code) > 0))

#define CODEC_SIZE_MFX_STREAMOUT_DATA (16 * sizeof(uint32_t))

#define ILDB_TYPE       5
#define OLP_TYPE        6
#define MIXED_TYPE      7
#define COPY_TYPE       7

// Pyramind B frame - used from DDI
#define B1_TYPE         8
#define B2_TYPE         9
#define INVALID_TYPE    10 // keep at end

#define CODECHAL_GET_ARRAY_LENGTH(a)           (sizeof(a) / sizeof(a[0]))

/*! \brief Flags for various picture properties.
*/
typedef enum _CODEC_PICTURE_FLAG
{
    PICTURE_TOP_FIELD               = 0x01,
    PICTURE_BOTTOM_FIELD            = 0x02,
    PICTURE_FRAME                   = 0x04,
    PICTURE_INTERLACED_FRAME        = 0x08,
    PICTURE_SHORT_TERM_REFERENCE    = 0x10,
    PICTURE_LONG_TERM_REFERENCE     = 0x20,
    PICTURE_INVALID                 = 0x80,
    PICTURE_RESIZE                  = 0xF0,
    PICTURE_MAX_7BITS               = 0xFF
} CODEC_PICTURE_FLAG;

/*! \brief Information pertaining to a frame's uncompressed surface
*
*   Both to identify and describe the surface.
*/
typedef struct _CODEC_PICTURE
{
    uint8_t                 FrameIdx;   //!< Index for the frame's uncompressed surface
    CODEC_PICTURE_FLAG      PicFlags;   //!< Flags describing picture properties
    uint8_t                 PicEntry;   //!< Unaltered DDI frame information (for debug purposes only)
} CODEC_PICTURE, *PCODEC_PICTURE;

// Forward Declarations
typedef struct _CODEC_REF_LIST CODEC_REF_LIST, *PCODEC_REF_LIST;

// ---------------------------
// Enums
// ---------------------------
typedef enum
{
    TOP_FIELD          = 1,
    BOTTOM_FIELD       = 2,
    FRAME_PICTURE      = 3
} PICTURE_STRUCTURE;

//!
//! \enum     REFLIST
//! \brief    Reference list
//!
enum REFLIST
{
    LIST_0 = 0,
    LIST_1 = 1
};

//!
//! \enum     CODECHAL_STANDARD 
//! \brief    Codec standard
//!
enum CODECHAL_STANDARD
{
    // MFX/MFL pipeline
    CODECHAL_MPEG2      = 0,
    CODECHAL_VC1        = 1,
    CODECHAL_AVC        = 2,
    CODECHAL_JPEG       = 3,
    CODECHAL_RESERVED   = 4,    //formerly SVC
    CODECHAL_VP8        = 5,
    CODECHAL_UNDEFINED  = 9,

    // Cenc Decode
    CODECHAL_CENC       = 63,

    // HCP pipeline
    CODECHAL_HCP_BASE   = 64,
    CODECHAL_HEVC       = CODECHAL_HCP_BASE,
    CODECHAL_VP9        = CODECHAL_HCP_BASE + 1,
    CODECHAL_STANDARD_MAX
};

//!
//! \enum    CODECHAL_MODE
//! \brief   Mode requested (high level combination between CODEC_STANDARD and CODEC_FUCNTION).
//!          Note: These modes are may be used for performance tagging. Be sure to notify tool owners if changing the definitions.
//!
enum CODECHAL_MODE
{
    CODECHAL_DECODE_MODE_MPEG2IDCT          = 0,
    CODECHAL_DECODE_MODE_MPEG2VLD           = 1,
    CODECHAL_DECODE_MODE_VC1IT              = 2,
    CODECHAL_DECODE_MODE_VC1VLD             = 3,
    CODECHAL_DECODE_MODE_AVCVLD             = 4,
    CODECHAL_DECODE_MODE_JPEG               = 5,
    CODECHAL_DECODE_RESERVED_0              = 6,    // formerly SVC
    CODECHAL_DECODE_MODE_VP8VLD             = 7,
    CODECHAL_DECODE_MODE_HEVCVLD            = 8,
    CODECHAL_DECODE_MODE_HUC                = 9,
    CODECHAL_DECODE_RESERVED_2              = 10,   // formerly AVS
    CODECHAL_DECODE_MODE_MVCVLD             = 11,   // Needed for CP. Not in use by Codec HAL.
    CODECHAL_DECODE_MODE_VP9VLD             = 12,
    CODECHAL_DECODE_MODE_CENC               = 13,   // Only for getting HuC-based DRM command size. Not an actual mode.
    CODECHAL_NUM_DECODE_MODES               = 14,

    CODECHAL_ENCODE_MODE_AVC                = 16,   // Must be a power of 2 to match perf report expectations
    CODECHAL_ENCODE_MODE_MPEG2              = 18,
    CODECHAL_ENCODE_MODE_VP8                = 19,
    CODECHAL_ENCODE_MODE_JPEG               = 20,
    CODECHAL_ENCODE_MODE_HEVC               = 22,
    CODECHAL_ENCODE_MODE_VP9                = 23,
    CODECHAL_ENCODE_RESERVED_0              = 24,
    CODECHAL_NUM_ENCODE_MODES               = 8,

    CODECHAL_NUM_MODES                      = 25,   // Use the value for the last encode mode to determine this
    CODECHAL_UNSUPPORTED_MODE               = 25,
    CODECHAL_MODE_MAX                       = 25
};

// Slice group mask
typedef enum tagSLICE_GROUP_MASK
{
    SLICE_GROUP_START   = 0x1,
    SLICE_GROUP_END     = 0x2,
    SLICE_GROUP_LAST    = 0x4
} SLICE_GROUP_MASK;

typedef struct _CODEC_PIC_ID
{
    uint8_t   ucPicIdx;
    uint8_t   ucDMVOffset[CODEC_NUM_FIELDS_PER_FRAME];      // 0 - top field, 1 - bottom field
    bool      bValid;
} CODEC_PIC_ID, *PCODEC_PIC_ID;

#if defined (_AVC_ENCODE_VME_SUPPORTED) || defined (_AVC_ENCODE_VDENC_SUPPORTED)
struct _CODEC_AVC_REF_PIC_SELECT_LIST;
typedef struct _CODEC_AVC_REF_PIC_SELECT_LIST   *PCODEC_AVC_REF_PIC_SELECT_LIST;
#endif

typedef struct _CODEC_VC1_IC
{
    // Top filed
    uint16_t    wICCScale1;
    uint16_t    wICCShiftL1;
    uint16_t    wICCShiftC1;
    // Bottom field
    uint16_t    wICCScale2;
    uint16_t    wICCShiftL2;
    uint16_t    wICCShiftC2;
} CODEC_VC1_IC, *PCODEC_VC1_IC;

//!
//! \struct   _CODEC_REF_LIST
//! \brief    Codec reference list
//!
struct _CODEC_REF_LIST
{
    // Shared decoding parameters
    CODEC_PICTURE                       RefPic;
    MOS_RESOURCE                        resRefPic;      // Resource of RefPic
    uint8_t                             ucFrameId;
    int32_t                             iFieldOrderCnt[CODEC_NUM_FIELDS_PER_FRAME];
    uint8_t                             ucDMVIdx[CODEC_NUM_FIELDS_PER_FRAME];
    bool                                bUsedAsRef;
    uint8_t                             ucNumRef;
    uint8_t                             ucAvcPictureCodingType; // used for PAFF case, 0: frame, 1: tff field, 2: invalid, 3: bff field
    CODEC_PICTURE                       RefList[CODEC_MAX_NUM_REF_FRAME];
    int16_t                             sFrameNumber;
 
    // Shared encoding parameters
    uint8_t                             ucMbCodeIdx;
    uint8_t                             ucScalingIdx;
    uint8_t                             ucMADBufferIdx;
    bool                                bMADEnabled;
    bool                                b4xScalingUsed;
    bool                                b16xScalingUsed;
    bool                                b32xScalingUsed;
    bool                                b2xScalingUsed; // 2x scaling currently only used for HEVC CNL. Uses same surface as 32x since 32x is not supported.
    uint8_t                             ucInitialIdx[2][2];
    uint8_t                             ucFinalIdx[2][2];
    uint8_t                             ucQPValue[2];
    MOS_RESOURCE                        resBitstreamBuffer;
    MOS_RESOURCE                        resRefMbCodeBuffer;
    MOS_RESOURCE                        resRefMvDataBuffer;
    MOS_SURFACE                         sRefBuffer;
    MOS_SURFACE                         sRefReconBuffer;
    MOS_SURFACE                         sRefRawBuffer;

    // Codec specific parameters
    union
    {
        // AVC, MVC
        struct
        {
           uint16_t                             usNonExistingFrameFlags;
           bool                                 bUsedAsInterViewRef;
           uint32_t                             uiUsedForReferenceFlags;
#if defined (_AVC_ENCODE_VME_SUPPORTED) || defined (_AVC_ENCODE_VDENC_SUPPORTED)
           PCODEC_AVC_REF_PIC_SELECT_LIST       pRefPicSelectListEntry;
#endif
           MOS_RESOURCE                         resRefTopFieldMbCodeBuffer;
           MOS_RESOURCE                         resRefBotFieldMbCodeBuffer;
           MOS_RESOURCE                         resRefTopFieldMvDataBuffer;
           MOS_RESOURCE                         resRefBotFieldMvDataBuffer;
        };

        // VC1
        struct
        {
            uint32_t                            dwRefSurfaceFlags;
            CODEC_VC1_IC                        Vc1IcValues[CODEC_NUM_FIELDS_PER_FRAME];
            bool                                bUnequalFieldSurfaceValid;
            uint32_t                            dwUnequalFieldSurfaceIdx;
        };

        // HEVC
        struct
        {
            bool                                bIsIntra;
            bool                                bFormatConversionDone;
            uint32_t                            rollingIntraRefreshedPosition;      // in units of blocks
        };

        // VP9
        struct
        {
            MOS_SURFACE                         sDysSurface;            // dynamic scaled surface (encoding resolution)
            MOS_SURFACE                         sDys4xScaledSurface;    // dynamic scaled surface (encoding resolution)
            MOS_SURFACE                         sDys16xScaledSurface;   // dynamic scaled surface (encoding resolution)
            uint32_t                            dwFrameWidth;           // in pixel
            uint32_t                            dwFrameHeight;          // in pixel
        };
    };
};

/*! \brief High level codec functionality
*/
typedef enum _CODECHAL_FUNCTION
{
    CODECHAL_FUNCTION_INVALID               = 0,
    CODECHAL_FUNCTION_DECODE                = 1,
    CODECHAL_FUNCTION_ENC                   = 2,        // Must be a power of 2 to match perf report expectations
    CODECHAL_FUNCTION_PAK                   = 4,        // Must be a power of 2 to match perf report expectations
    CODECHAL_FUNCTION_ENC_PAK               = 8,        // Must be a power of 2 to match perf report expectations
    CODECHAL_FUNCTION_HYBRIDPAK             = 16,
    CODECHAL_FUNCTION_ENC_VDENC_PAK         = 32,
    CODECHAL_FUNCTION_CENC_DECODE           = 64,
    CODECHAL_FUNCTION_DEMO_COPY             = 128,
    CODECHAL_FUNCTION_FEI_PRE_ENC           = 256,
    CODECHAL_FUNCTION_FEI_ENC               = 512,
    CODECHAL_FUNCTION_FEI_PAK               = 1024,
    CODECHAL_FUNCTION_FEI_ENC_PAK           = 2048
} CODECHAL_FUNCTION, *PCODECHAL_FUNCTION;

// TargetUsage from app - used from DDI
enum
{
    TARGETUSAGE_UNKNOWN         = 0,
    TARGETUSAGE_BEST_QUALITY    = 1,
    TARGETUSAGE_HI_QUALITY      = 2,
    TARGETUSAGE_OPT_QUALITY     = 3,
    TARGETUSAGE_OK_QUALITY      = 5,
    TARGETUSAGE_NO_SPEED        = 1,
    TARGETUSAGE_OPT_SPEED       = 3,
    TARGETUSAGE_RT_SPEED        = 4,
    TARGETUSAGE_HI_SPEED        = 6,
    TARGETUSAGE_BEST_SPEED      = 7,
    TARGETUSAGE_LOW_LATENCY     = 0x10,
    TARGETUSAGE_MULTIPASS       = 0x20
};

typedef enum _CODECHAL_LUMA_CHROMA_DEPTH
{
    CODECHAL_LUMA_CHROMA_DEPTH_INVALID      = 0x00,
    CODECHAL_LUMA_CHROMA_DEPTH_8_BITS       = 0x01,
    CODECHAL_LUMA_CHROMA_DEPTH_10_BITS      = 0x02,
    CODECHAL_LUMA_CHROMA_DEPTH_12_BITS      = 0x04
} CODECHAL_LUMA_CHROMA_DEPTH;

/*! \brief Status returned per picture executed by status reporting
*/
typedef enum _CODECHAL_STATUS
{
    /*! \brief  The picture in question was processed successfully by HW
    *
    *   All relevant parameters in the status reporting structure should be valid.
    */
    CODECHAL_STATUS_SUCCESSFUL  = 0,
    CODECHAL_STATUS_INCOMPLETE  = 1,    //!< The picture in question has not yet finished being processing on HW.
    /*! \brief  Indicates that an error occured during execution.
    *
    *   Only error reporting parameters in the status reporting structure will be valid. This status will be returned if the workload(s) for the picture in question resulted in a HW hang or HW status indicators indicate a failure.
    */
    CODECHAL_STATUS_ERROR       = 2,
    CODECHAL_STATUS_UNAVAILABLE = 3     //!< Indicates that the entry in the status reporting array was not used
} CODECHAL_STATUS, *PCODECHAL_STATUS;

typedef enum _CODECHAL_CHROMA_SITING_TYPE
{
    CODECHAL_CHROMA_SITING_NONE             = 0x00,
    CODECHAL_CHROMA_SITING_HORZ_LEFT        = 0x01,
    CODECHAL_CHROMA_SITING_HORZ_CENTER      = 0x02,
    CODECHAL_CHROMA_SITING_HORZ_RIGHT       = 0x04,
    CODECHAL_CHROMA_SITING_VERT_TOP         = 0x10,
    CODECHAL_CHROMA_SITING_VERT_CENTER      = 0x20,
    CODECHAL_CHROMA_SITING_VERT_BOTTOM      = 0x40
} CODECHAL_CHROMA_SITING_TYPE;

typedef enum _CODECHAL_CHROMA_SUBSAMPLING
{
    CODECHAL_CHROMA_SUBSAMPLING_TOP_CENTER    = 0,
    CODECHAL_CHROMA_SUBSAMPLING_CENTER_CENTER,
    CODECHAL_CHROMA_SUBSAMPLING_BOTTOM_CENTER,
    CODECHAL_CHROMA_SUBSAMPLING_TOP_LEFT,
    CODECHAL_CHROMA_SUBSAMPLING_CENTER_LEFT,
    CODECHAL_CHROMA_SUBSAMPLING_BOTTOM_LEFT
} CODECHAL_CHROMA_SUBSAMPLING;

#define CODECHAL_OFFSETOF(TYPE, ELEMENT)                    ((size_t)&(((TYPE *)0)->ELEMENT))
#define CODECHAL_OFFSETOF_IN_DW(TYPE, ELEMENT)              (CODECHAL_OFFSETOF(TYPE, ELEMENT) >> 2)
#define CODECHAL_ADDROF_TYPE(TYPE, ELEMENT, ELEMENT_ADDR)   ((TYPE *)((uint8_t *)(ELEMENT_ADDR) - CODECHAL_OFFSETOF(TYPE, ELEMENT)))

#define CODECHAL_GET_WIDTH_IN_MACROBLOCKS(dwWidth)               (((dwWidth) + (CODECHAL_MACROBLOCK_WIDTH - 1)) / CODECHAL_MACROBLOCK_WIDTH)
#define CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(dwHeight)             (((dwHeight) + (CODECHAL_MACROBLOCK_HEIGHT - 1)) / CODECHAL_MACROBLOCK_HEIGHT)
#define CODECHAL_GET_WIDTH_IN_BLOCKS(dwWidth, dwBlockSize)       (((dwWidth) + (dwBlockSize - 1)) / dwBlockSize)
#define CODECHAL_GET_HEIGHT_IN_BLOCKS(dwHeight, dwBlockSize)     (((dwHeight) + (dwBlockSize - 1)) / dwBlockSize)
#define CODECHAL_GET_4xDS_SIZE_32ALIGNED(dwSize)                 ((((dwSize >> 2) + 31) >> 5) << 5)
#define CODECHAL_GET_2xDS_SIZE_32ALIGNED(dwSize)                 ((((dwSize >> 1) + 31) >> 5) << 5)

// Macros to determin CODEC_PICTURE_FLAG meaning
#define CodecHal_PictureIsTopField(Pic)         (((Pic).PicFlags & PICTURE_TOP_FIELD) != 0)
#define CodecHal_PictureIsBottomField(Pic)      (((Pic).PicFlags & PICTURE_BOTTOM_FIELD) != 0)
#define CodecHal_PictureIsField(pic)            \
    ((CodecHal_PictureIsTopField((pic)) || CodecHal_PictureIsBottomField((pic))) != 0)
#define CodecHal_PictureIsFrame(Pic)            (((Pic).PicFlags & PICTURE_FRAME) != 0)
#define CodecHal_PictureIsInterlacedFrame(Pic)  (((Pic).PicFlags & PICTURE_INTERLACED_FRAME) != 0)
#define CodecHal_PictureIsLongTermRef(Pic)      (((Pic).PicFlags & PICTURE_LONG_TERM_REFERENCE) != 0)
#define CodecHal_PictureIsShortTermRef(Pic)     (((Pic).PicFlags & PICTURE_SHORT_TERM_REFERENCE) != 0)
#define CodecHal_PictureIsInvalid(Pic)          (((Pic).PicFlags & PICTURE_INVALID) != 0)

#define CodecHal_GetFrameFieldHeightInMb(pic, heightInMb, adjustedHeightInMb)     \
{                                                                                   \
    if(CodecHal_PictureIsField((pic)))                                              \
    {                                                                               \
        adjustedHeightInMb = ((heightInMb) + 1) >> 1;                             \
    }                                                                               \
    else                                                                            \
    {                                                                               \
        adjustedHeightInMb = (heightInMb);                                        \
    }                                                                               \
}

#define CodecHalIsDecode(codecFunction)                 \
    (codecFunction == CODECHAL_FUNCTION_DECODE ||   \
     codecFunction == CODECHAL_FUNCTION_CENC_DECODE)

#define CodecHalIsDecodeModeVLD(mode)               \
    ((mode == CODECHAL_DECODE_MODE_MPEG2VLD)    ||  \
    (mode == CODECHAL_DECODE_MODE_VC1VLD)       ||  \
    (mode == CODECHAL_DECODE_MODE_AVCVLD)       ||  \
    (mode == CODECHAL_DECODE_MODE_JPEG)         ||  \
    (mode == CODECHAL_DECODE_MODE_VP8VLD)       ||  \
    (mode == CODECHAL_DECODE_MODE_HEVCVLD)      ||  \
    (mode == CODECHAL_DECODE_MODE_VP9VLD))

#define CodecHalIsDecodeModeIT(mode)                \
    ((mode == CODECHAL_DECODE_MODE_MPEG2IDCT)   ||  \
    (mode == CODECHAL_DECODE_MODE_VC1IT))

#define CodecHal_CombinePictureFlags(originalPic, newPic)   \
    ((CODEC_PICTURE_FLAG)((((uint32_t)(originalPic).PicFlags) & 0xF) | ((uint32_t)(newPic).PicFlags)))

#define CodecHalIsEnableFieldScaling(codecFunction, standard, hint) \
    ((codecFunction == CODECHAL_FUNCTION_DECODE)               &&    \
    (standard == CODECHAL_AVC)                                 &&    \
    (hint == true))

#define CodecHalDecodeMapGpuNodeToGpuContex(gpuNode, gpuCtxt, wactx)\
do                                                                                          \
{                                                                                           \
    switch (gpuNode)                                                                        \
    {                                                                                       \
        case MOS_GPU_NODE_VIDEO:                                                            \
            gpuCtxt = wactx ? MOS_GPU_CONTEXT_VIDEO2 : MOS_GPU_CONTEXT_VIDEO;              \
            break;                                                                          \
        case MOS_GPU_NODE_VIDEO2:                                                           \
            gpuCtxt = wactx ? MOS_GPU_CONTEXT_VDBOX2_VIDEO2 : MOS_GPU_CONTEXT_VDBOX2_VIDEO;\
            break;                                                                          \
        default:                                                                            \
            gpuCtxt = wactx ? MOS_GPU_CONTEXT_VIDEO2 : MOS_GPU_CONTEXT_VIDEO;              \
            CODECHAL_DECODE_ASSERTMESSAGE("invalid GPU NODE value.");                       \
            break;                                                                          \
    }                                                                                       \
}while (0)

#define CodecHalDecode1stHcpDecPhaseInSinglePipeMode(hcpDecPhase, shortFormat)\
    ((hcpDecPhase == CodechalDecode::CodechalHcpDecodePhaseLegacyS2L && shortFormat)  || \
     (hcpDecPhase == CodechalDecode::CodechalHcpDecodePhaseLegacyLong && !shortFormat))

// ---------------------------
// Functions
// ---------------------------
static __inline uint16_t GetReciprocalScalingValue(uint8_t scaleValue)
{
    uint16_t reciprocalScaleValue;
    if (scaleValue < 2)
    {
        reciprocalScaleValue = 0xffff;
    }
    else
    {
        reciprocalScaleValue = (4096 * 16)/scaleValue;
    }

    return reciprocalScaleValue;
}

// this function covert the input value v to A|B format so v~=B*pow(2, A);
static __inline uint8_t Map44LutValue(uint32_t v, uint8_t max)
{
    uint32_t   maxCost;
    int     D;
    uint8_t   ret;

    if (v == 0)
    {
        return 0;
    }

    maxCost = ((max & 15) << (max >> 4));
    if (v >= maxCost)
    {
        return max;
    }

    D = (int)(log((double)v) / log(2.)) - 3;
    if (D < 0)
    {
        D = 0;
    }

    ret = (uint8_t)((D << 4) + (int)((v + (D == 0 ? 0 : (1 << (D - 1)))) >> D));
    ret = (ret & 0xf) == 0 ? (ret | 8) : ret;

    return ret;
}

static __inline uint8_t GetU62ModeCost(double mcost)
{
    return (uint8_t)(mcost * 4 + 0.5);
}

static __inline uint8_t GetU71ModeCost(double mcost)
{
    return (uint8_t)(mcost * 2 + 0.5);
}

static __inline uint8_t GetU44ModeCost(double mcost)
{
    return (uint8_t)(mcost * 16 + 0.5);
}

static __inline uint32_t CodecHal_GetStandardFromMode(uint32_t mode)
{
    uint32_t standard = CODECHAL_UNDEFINED;

    switch (mode)
    {
    case CODECHAL_DECODE_MODE_MPEG2IDCT:
    case CODECHAL_DECODE_MODE_MPEG2VLD:
    case CODECHAL_ENCODE_MODE_MPEG2:
        standard = CODECHAL_MPEG2;
        break;
    case CODECHAL_DECODE_MODE_VC1IT:
    case CODECHAL_DECODE_MODE_VC1VLD:
        standard = CODECHAL_VC1;
        break;
    case CODECHAL_DECODE_MODE_AVCVLD:
    case CODECHAL_ENCODE_MODE_AVC:
        standard = CODECHAL_AVC;
        break;
    case CODECHAL_DECODE_MODE_JPEG:
    case CODECHAL_ENCODE_MODE_JPEG:
        standard = CODECHAL_JPEG;
        break;
    case CODECHAL_DECODE_MODE_VP8VLD:
    case CODECHAL_ENCODE_MODE_VP8:
        standard = CODECHAL_VP8;
        break;
    case CODECHAL_DECODE_MODE_HEVCVLD:
    case CODECHAL_ENCODE_MODE_HEVC:
        standard = CODECHAL_HEVC;
        break;
    case CODECHAL_ENCODE_MODE_VP9:
    case CODECHAL_DECODE_MODE_VP9VLD:
        standard = CODECHAL_VP9;
        break;
    case CODECHAL_DECODE_MODE_CENC:
        standard = CODECHAL_CENC;
        break;
    default:
        standard = CODECHAL_UNDEFINED;
        break;
    }

    return standard;
}

static __inline int32_t CodecHal_Clip3(int x, int y, int z)
{
    int32_t ret = 0;

    if (z < x)
    {
        ret = x;
    }
    else if (z > y)
    {
        ret = y;
    }
    else
    {
        ret = z;
    }

    return ret;
}
#endif  // __CODEC_DEF_COMMON_H__
