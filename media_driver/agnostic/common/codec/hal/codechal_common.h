/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_common.h
//! \brief    Defines common functions, macros, and types shared by all of CodecHal.
//! \details  Considered part of the public interface however is internally facing instead of externally.
//!

#ifndef __CODECHAL_COMMON_H__
#define __CODECHAL_COMMON_H__

#ifndef LOG_TAG
#define LOG_TAG "CODECHAL"
#endif

#include "mos_os.h"

class CodechalHwInterface;
typedef struct _CODECHAL_SETTINGS CODECHAL_SETTINGS, *PCODECHAL_SETTINGS;
typedef struct _MHW_WALKER_PARAMS MHW_WALKER_PARAMS, *PMHW_WALKER_PARAMS;
typedef struct _MHW_STATE_HEAP_INTERFACE MHW_STATE_HEAP_INTERFACE, *PMHW_STATE_HEAP_INTERFACE;
typedef struct MHW_KERNEL_STATE *PMHW_KERNEL_STATE;
typedef struct _MHW_RCS_SURFACE_PARAMS MHW_RCS_SURFACE_PARAMS, *PMHW_RCS_SURFACE_PARAMS;
typedef struct _CODECHAL_SURFACE_CODEC_PARAMS CODECHAL_SURFACE_CODEC_PARAMS, *PCODECHAL_SURFACE_CODEC_PARAMS;

#define CODECHAL_STATUS_QUERY_SKIPPED    0x00
#define CODECHAL_STATUS_QUERY_START_FLAG 0x01
#define CODECHAL_STATUS_QUERY_END_FLAG   0xFF

#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1   128
#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2 128
#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_JPEG  1
#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8   128
#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC  127 // 7 bits, 0x7f is invalid one
#define CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9   128

#define CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9      3

//!
//! \def CODECHAL_DECODE_VP9_MAX_NUM_REF_FRAME
//! Max reference frame number of vp9 decoder
//!
#define CODECHAL_DECODE_VP9_MAX_NUM_REF_FRAME   8


#define CODECHAL_MAD_BUFFER_SIZE                4 // buffer size is 4 bytes

#define CODECHAL_RESET_VLINE_STRIDE_BUFFER_WIDTH  16
#define CODECHAL_RESET_VLINE_STRIDE_BUFFER_HEIGHT 16


// VC-1/AVC decoding filter types, used only for performance capture
#define ILDB_TYPE       5
#define OLP_TYPE        6
#define MIXED_TYPE      7
#define COPY_TYPE       7

// Pyramind B frame - used from DDI
#define B1_TYPE         8
#define B2_TYPE         9
#define INVALID_TYPE    10 // keep at end

// HME
#define SCALE_FACTOR_2x     2
#define SCALE_FACTOR_4x     4
#define SCALE_FACTOR_16x    16
#define SCALE_FACTOR_32x    32

#define CODECHAL_VP8_NUM_MAX_VME_REF                3
#define CODECHAL_NUM_VP8_DMV_BUFFERS                (CODECHAL_VP8_NUM_MAX_VME_REF + 1) // DMV buffers for VP8 here means the Scaling buffers & not the Direct Mode buffers
#define CODECHAL_VP8_MAX_QP                         128
#define CODECHAL_VP8_MB_CODE_SIZE                   204
#define CODECHAL_VP8_MB_MV_CODE_SIZE                64
#define CODECHAL_VP8_MB_MV_CODE_OFFSET_ALIGNMENT    4096
#define CODECHAL_VP8_MB_CODE_ALIGNMENT              32
#define CODECHAL_VP8_FRAME_HEADER_SIZE              4096
#define CODECHAL_VP8_MAX_SEGMENTS                   4
#define CODECHAL_VP8_ME_ME_DATA_SIZE_MULTIPLIER     3

#define CODECHAL_VP9_MB_CODE_SIZE                   204

#define CODECHAL_HEVC_MAX_NUM_SLICES_LVL_5      200

#define CODECHAL_VP9_NUM_MV_BUFFERS             2

#define CODECHAL_GET_ARRAY_LENGTH(a)           (sizeof(a) / sizeof(a[0]))

// The current definition of the first encode function CODECHAL_FUNCTION_ENC should be used
// as a base for subsequent encode functions
#define CODECHAL_ENCODE_FUNCTION_BIT_OFFSET ((uint32_t)(log((double)CODECHAL_FUNCTION_ENC)/log(2.)))

// ---------------------------
// Tables
// ---------------------------

// matrix required to read in the quantization matrix
static const uint8_t jpeg_qm_scan_8x8[64] =
{
    // Zig-Zag scan pattern
    0,   1,  8, 16,  9,  2,  3, 10,
    17, 24, 32, 25, 18, 11,  4,  5,
    12, 19, 26, 33, 40, 48, 41, 34,
    27, 20, 13,  6,  7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36,
    29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46,
    53, 60, 61, 54, 47, 55, 62, 63
};

static const uint8_t hevc_qm_scan_4x4[16] =
{
    // Up-right scan pattern
    0,  4,  1,   8,
    5,  2,  12,  9,
    6,  3,  13,  10,
    7,  14, 11,  15
};

static const uint8_t hevc_qm_scan_8x8[64] =
{
    // Up-right scan pattern
    0,  8,  1,  16, 9,  2,  24, 17,
    10, 3,  32, 25, 18, 11, 4,  40,
    33, 26, 19, 12, 5,  48, 41, 34,
    27, 20, 13, 6,  56, 49, 42, 35,
    28, 21, 14, 7,  57, 50, 43, 36,
    29, 22, 15, 58, 51, 44, 37, 30,
    23, 59, 52, 45, 38, 31, 60, 53,
    46, 39, 61, 54, 47, 62, 55, 63
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

// Batch buffer type
enum
{
    MB_ENC_Frame_BB    = 0,
    MB_ENC_Field_BB,
    //Add new buffer type here
    NUM_ENCODE_BB_TYPE
};

// used from DDI
typedef enum
{
    FRAME_NO_SKIP       = 0,        // encode as normal, no skip frames
    FRAME_SKIP_NORMAL   = 1         // one or more frames were skipped prior to curr frame. Encode curr frame as normal, update BRC
} FRAME_SKIP_FLAG;

typedef enum _CODECHAL_LUMA_CHROMA_DEPTH
{
    CODECHAL_LUMA_CHROMA_DEPTH_INVALID      = 0x00,
    CODECHAL_LUMA_CHROMA_DEPTH_8_BITS       = 0x01,
    CODECHAL_LUMA_CHROMA_DEPTH_10_BITS      = 0x02,
    CODECHAL_LUMA_CHROMA_DEPTH_12_BITS      = 0x04
} CODECHAL_LUMA_CHROMA_DEPTH;

// The current definition of the first encode mode CODECHAL_ENCODE_MODE_AVC should be used
// as a base for subsequent encode modes
#define CODECHAL_ENCODE_MODE_BIT_OFFSET     ((uint32_t)(log((double)CODECHAL_ENCODE_MODE_AVC)/log(2.)))
#define CODECHAL_ENCODE_MODE_BIT_MASK       (( 1L << CODECHAL_ENCODE_MODE_BIT_OFFSET) - 1 )

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

typedef enum _CODECHAL_MEDIA_STATE_TYPE
{
    CODECHAL_MEDIA_STATE_OLP                                = 0,
    CODECHAL_MEDIA_STATE_ENC_NORMAL                         = 1,
    CODECHAL_MEDIA_STATE_ENC_PERFORMANCE                    = 2,
    CODECHAL_MEDIA_STATE_ENC_QUALITY                        = 3,
    CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST                   = 4,
    CODECHAL_MEDIA_STATE_32X_SCALING                        = 5,
    CODECHAL_MEDIA_STATE_16X_SCALING                        = 6,
    CODECHAL_MEDIA_STATE_4X_SCALING                         = 7,
    CODECHAL_MEDIA_STATE_32X_ME                             = 8,
    CODECHAL_MEDIA_STATE_16X_ME                             = 9,
    CODECHAL_MEDIA_STATE_4X_ME                              = 10,
    CODECHAL_MEDIA_STATE_BRC_INIT_RESET                     = 11,
    CODECHAL_MEDIA_STATE_BRC_UPDATE                         = 12,
    CODECHAL_MEDIA_STATE_BRC_BLOCK_COPY                     = 13,
    CODECHAL_MEDIA_STATE_HYBRID_PAK_P1                      = 14,
    CODECHAL_MEDIA_STATE_HYBRID_PAK_P2                      = 15,
    CODECHAL_MEDIA_STATE_ENC_I_FRAME_CHROMA                 = 16,
    CODECHAL_MEDIA_STATE_ENC_I_FRAME_LUMA                   = 17,
    CODECHAL_MEDIA_STATE_MPU_FHB                            = 18,
    CODECHAL_MEDIA_STATE_TPU_FHB                            = 19,
    CODECHAL_MEDIA_STATE_PA_COPY                            = 20,
    CODECHAL_MEDIA_STATE_PL2_COPY                           = 21,
    CODECHAL_MEDIA_STATE_ENC_ADV                            = 22,
    CODECHAL_MEDIA_STATE_2X_SCALING                         = 23,
    CODECHAL_MEDIA_STATE_32x32_PU_MODE_DECISION             = 24,
    CODECHAL_MEDIA_STATE_16x16_PU_SAD                       = 25,
    CODECHAL_MEDIA_STATE_16x16_PU_MODE_DECISION             = 26,
    CODECHAL_MEDIA_STATE_8x8_PU                             = 27,
    CODECHAL_MEDIA_STATE_8x8_PU_FMODE                       = 28,
    CODECHAL_MEDIA_STATE_32x32_B_INTRA_CHECK                = 29,
    CODECHAL_MEDIA_STATE_HEVC_B_MBENC                       = 30,
    CODECHAL_MEDIA_STATE_RESET_VLINE_STRIDE                 = 31,
    CODECHAL_MEDIA_STATE_HEVC_B_PAK                         = 32,
    CODECHAL_MEDIA_STATE_HEVC_BRC_LCU_UPDATE                = 33,
    CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN                  = 34,
    CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32                    = 35,
    CODECHAL_MEDIA_STATE_VP9_ENC_I_16x16                    = 36,
    CODECHAL_MEDIA_STATE_VP9_ENC_P                          = 37,
    CODECHAL_MEDIA_STATE_VP9_ENC_TX                         = 38,
    CODECHAL_MEDIA_STATE_VP9_DYS                            = 39,
    CODECHAL_MEDIA_STATE_VP9_PAK_LUMA_RECON                 = 40,
    CODECHAL_MEDIA_STATE_VP9_PAK_CHROMA_RECON               = 41,
    CODECHAL_MEDIA_STATE_VP9_PAK_DEBLOCK_MASK               = 42,
    CODECHAL_MEDIA_STATE_VP9_PAK_LUMA_DEBLOCK               = 43,
    CODECHAL_MEDIA_STATE_VP9_PAK_CHROMA_DEBLOCK             = 44,
    CODECHAL_MEDIA_STATE_VP9_PAK_MC_PRED                    = 45,
    CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_LUMA_RECON         = 46,
    CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_LUMA_RECON_32x32   = 47,
    CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_CHROMA_RECON       = 48,
    CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_INTRA_LUMA_RECON   = 49,
    CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_INTRA_CHROMA_RECON = 50,
    CODECHAL_MEDIA_STATE_PREPROC                            = 51,
    CODECHAL_MEDIA_STATE_ENC_WP                             = 52,
    CODECHAL_MEDIA_STATE_HEVC_I_MBENC                       = 53,
    CODECHAL_MEDIA_STATE_CSC_DS_COPY                        = 54,
    CODECHAL_MEDIA_STATE_2X_4X_SCALING                      = 55,
    CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC                 = 56,
    CODECHAL_MEDIA_STATE_MB_BRC_UPDATE                      = 57,
    CODECHAL_MEDIA_STATE_STATIC_FRAME_DETECTION             = 58,
    CODECHAL_MEDIA_STATE_HEVC_ROI                           = 59,
    CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT                 = 60,
    CODECHAL_NUM_MEDIA_STATES                               = 61
} CODECHAL_MEDIA_STATE_TYPE;

C_ASSERT(CODECHAL_NUM_MEDIA_STATES == (CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT + 1)); //!< update this and add new entry in the default SSEU table for each platform()

// Decoder extension Function Codes
typedef enum _CODECHAL_ENCODE_FUNCTION_ID
{
    CODECHAL_ENCODE_ENC_ID           = 0x100,
    CODECHAL_ENCODE_PAK_ID           = 0x101,
    CODECHAL_ENCODE_ENC_PAK_ID       = 0x102,
    CODECHAL_ENCODE_VPP_ID           = 0x103,
    CODECHAL_ENCODE_FORMAT_COUNT_ID  = 0x104,
    CODECHAL_ENCODE_FORMATS_ID       = 0x105,
    CODECHAL_ENCODE_ENC_CTRL_CAPS_ID = 0x106,
    CODECHAL_ENCODE_ENC_CTRL_GET_ID  = 0x107,
    CODECHAL_ENCODE_ENC_CTRL_SET_ID  = 0x108,
    CODECHAL_ENCODE_MBDATA_LAYOUT_ID = 0x109,
    CODECHAL_ENCODE_FEI_PRE_ENC_ID   = 0x10A,
    CODECHAL_ENCODE_FEI_ENC_ID       = 0x10B,
    CODECHAL_ENCODE_FEI_PAK_ID       = 0x10C,
    CODECHAL_ENCODE_FEI_ENC_PAK_ID   = 0x10D,
    CODECHAL_ENCODE_QUERY_STATUS_ID  = 0x121
} CODECHAL_ENCODE_FUNCTION_ID;

typedef enum _CODECHAL_WALKER_DEGREE
{
    CODECHAL_NO_DEGREE,
    CODECHAL_45_DEGREE,
    CODECHAL_26_DEGREE,
    CODECHAL_46_DEGREE,    // VP8 HybridPak2Pattern
    CODECHAL_26Z_DEGREE,   // HEVC
    CODECHAL_45Z_DEGREE,   // VP9 MB ENC I 16x16, P
    CODECHAL_26X_DEGREE,   // HEVC
    CODECHAL_26ZX_DEGREE,  // HEVC
    CODECHAL_45D_DEGREE,   // HEVC 45 diamond walk pattern
    CODECHAL_45XD_DEGREE,  // HEVC 45X diamond walk pattern
    CODECHAL_45X_DEGREE_ALT //HEVC 45X ALT walk pattern
} CODECHAL_WALKER_DEGREE;

typedef enum _CODECHAL_SLICE_STATE
{
    CODECHAL_SLICE_SHUTDOWN_DEFAULT     = 0,
    CODECHAL_SLICE_SHUTDOWN_ONE_SLICE   = 1,
    CODECHAL_SLICE_SHUTDOWN_TWO_SLICES  = 2
} CODECHAL_SLICE_STATE;

typedef enum _CODEC_SLICE_STRUCTS
{
    CODECHAL_SLICE_STRUCT_ONESLICE           = 0,    // Once slice for the whole frame
    CODECHAL_SLICE_STRUCT_POW2ROWS           = 1,    // Slices are power of 2 number of rows, all slices the same
    CODECHAL_SLICE_STRUCT_ROWSLICE           = 2,    // Slices are any number of rows, all slices the same
    CODECHAL_SLICE_STRUCT_ARBITRARYROWSLICE  = 3,    // Slices are any number of rows, slices can be different
    CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE   = 4     // Slices are any number of MBs, slices can be different
    // 5 - 7 are Reserved
} CODEC_SLICE_STRUCTS;

// HEVC Decode Kernel ID
typedef enum _CODECHAL_HEVC_KERNEL_ID
{
    CODECHAL_HEVC_DECODE_DequantResidueLuma     = 0,
    CODECHAL_HEVC_DECODE_DequantResidueChroma   = 1,
    CODECHAL_HEVC_DECODE_IntraPredLuma          = 2,
    CODECHAL_HEVC_DECODE_IntraPredChroma        = 3,
    CODECHAL_HEVC_DECODE_InterPredLuma          = 4,
    CODECHAL_HEVC_DECODE_InterPredChroma        = 5,
    CODECHAL_HEVC_DECODE_DeblockLuma            = 6,
    CODECHAL_HEVC_DECODE_DeblockChroma          = 7,
    CODECHAL_HEVC_DECODE_SAOLuma                = 8,
    CODECHAL_HEVC_DECODE_SAOChroma              = 9
} CODECHAL_HEVC_KERNEL_ID;

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

typedef enum _CODECHAL_ENCODE_BRC_KERNEL_STATE_IDX
{
    CODECHAL_ENCODE_BRC_IDX_INIT = 0,
    CODECHAL_ENCODE_BRC_IDX_FrameBRC_UPDATE,
    CODECHAL_ENCODE_BRC_IDX_RESET,
    CODECHAL_ENCODE_BRC_IDX_IFRAMEDIST,
    CODECHAL_ENCODE_BRC_IDX_BLOCKCOPY,
    CODECHAL_ENCODE_BRC_IDX_MbBRC_UPDATE, // extra MbBRCUpdate kernel starting GEN9
    CODECHAL_ENCODE_BRC_IDX_NUM
} CODECHAL_ENCODE_BRC_KERNEL_STATE_IDX;

typedef enum _CODECHAL_HUC_PRODUCT_FAMILY
{
    HUC_UNKNOWN     = 0,
    HUC_SKYLAKE     = 2,
    HUC_BROXTON,
    HUC_KABYLAKE,
    HUC_CANNONLAKE,
} CODECHAL_HUC_PRODUCT_FAMILY;

typedef enum _CODECHAL_CS_ENGINE_ID_DEF
{
    // Instance ID
    CODECHAL_CS_INSTANCE_ID_VDBOX0 = 0,
    CODECHAL_CS_INSTANCE_ID_VDBOX1 = 1,
    CODECHAL_CS_INSTANCE_ID_VDBOX2 = 2,
    CODECHAL_CS_INSTANCE_ID_VDBOX3 = 3,
    CODECHAL_CS_INSTANCE_ID_VDBOX4 = 4,
    CODECHAL_CS_INSTANCE_ID_VDBOX5 = 5,
    CODECHAL_CS_INSTANCE_ID_VDBOX6 = 6,
    CODECHAL_CS_INSTANCE_ID_VDBOX7 = 7,
    CODECHAL_CS_INSTANCE_ID_MAX,
    // Class ID
    CODECHAL_CLASS_ID_VIDEO_ENGINE = 1,
} CODECHAL_CS_ENGINE_ID_DEF;

#define CodecHal_CombinePictureFlags(originalPic, newPic)   \
    ((CODEC_PICTURE_FLAG)((((uint32_t)(originalPic).PicFlags) & 0xF) | ((uint32_t)(newPic).PicFlags)))

// Determine which mode type is used
#define CodecHalIsDecodeMode(mode)                   \
    ((mode == CODECHAL_DECODE_MODE_AVCVLD)       ||  \
    (mode == CODECHAL_DECODE_MODE_VC1VLD)        ||  \
    (mode == CODECHAL_DECODE_MODE_VC1IT)         ||  \
    (mode == CODECHAL_DECODE_MODE_MPEG2VLD)      ||  \
    (mode == CODECHAL_DECODE_MODE_MPEG2IDCT)     ||  \
    (mode == CODECHAL_DECODE_MODE_JPEG)          ||  \
    (mode == CODECHAL_DECODE_MODE_VP8VLD)        ||  \
    (mode == CODECHAL_DECODE_MODE_HEVCVLD)       ||  \
    (mode == CODECHAL_DECODE_MODE_VP9VLD))

#define CodecHalIsEncodeMode(mode)                      \
    ((mode == CODECHAL_ENCODE_MODE_AVC)             ||  \
    (mode == CODECHAL_ENCODE_MODE_MPEG2)            ||  \
    (mode == CODECHAL_ENCODE_MODE_VP8)              ||  \
    (mode == CODECHAL_ENCODE_MODE_JPEG)             ||  \
    (mode == CODECHAL_ENCODE_MODE_HEVC)             ||  \
    (mode == CODECHAL_ENCODE_MODE_VP9))

#define CodecHalIsDecode(codecFunction)                 \
        (codecFunction == CODECHAL_FUNCTION_DECODE ||   \
         codecFunction == CODECHAL_FUNCTION_CENC_DECODE)

//FEI Encode Macros
#define CodecHalIsFeiEncode(codecFunction)              \
    ( codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC ||  \
      codecFunction == CODECHAL_FUNCTION_FEI_ENC ||  \
    codecFunction == CODECHAL_FUNCTION_FEI_PAK ||  \
    codecFunction == CODECHAL_FUNCTION_FEI_ENC_PAK)

//Encode Macros
#define CodecHalIsEncode(codecFunction)                 \
        (codecFunction == CODECHAL_FUNCTION_ENC ||      \
         codecFunction == CODECHAL_FUNCTION_PAK ||      \
         codecFunction == CODECHAL_FUNCTION_ENC_PAK ||  \
         codecFunction == CODECHAL_FUNCTION_ENC_VDENC_PAK ||\
         codecFunction == CODECHAL_FUNCTION_HYBRIDPAK) || \
         CodecHalIsFeiEncode(codecFunction)

#define CodecHalUsesVideoEngine(codecFunction)            \
        (codecFunction == CODECHAL_FUNCTION_PAK       ||  \
         codecFunction == CODECHAL_FUNCTION_ENC_PAK   ||  \
         codecFunction == CODECHAL_FUNCTION_ENC_VDENC_PAK || \
         codecFunction == CODECHAL_FUNCTION_FEI_PAK   ||  \
         codecFunction == CODECHAL_FUNCTION_FEI_ENC_PAK)

#define CodecHalUsesRenderEngine(codecFunction, standard)   \
    (codecFunction == CODECHAL_FUNCTION_ENC ||              \
    (codecFunction == CODECHAL_FUNCTION_ENC_PAK) ||           \
    codecFunction == CODECHAL_FUNCTION_HYBRIDPAK ||         \
    ((codecFunction == CODECHAL_FUNCTION_DECODE) && (standard == CODECHAL_VC1)) || \
    codecFunction == CODECHAL_FUNCTION_ENC_VDENC_PAK || \
    codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC || \
    codecFunction == CODECHAL_FUNCTION_FEI_ENC   ||  \
    codecFunction == CODECHAL_FUNCTION_FEI_ENC_PAK)

#define CodecHalUsesOnlyRenderEngine(codecFunction) \
    (codecFunction == CODECHAL_FUNCTION_ENC ||      \
     codecFunction == CODECHAL_FUNCTION_FEI_ENC ||      \
    codecFunction == CODECHAL_FUNCTION_HYBRIDPAK)

#define CodecHalUsesVdencEngine(codecFunction)   \
        (codecFunction == CODECHAL_FUNCTION_ENC_VDENC_PAK)

#define CodecHalIsRateControlBrc(rateControl, standard) (\
    (rateControl == RATECONTROL_CBR)                || \
    (rateControl == RATECONTROL_VBR)                || \
    (rateControl == RATECONTROL_AVBR)               || \
    (rateControl == RATECONTROL_CQL)                || \
    ((( rateControl == RATECONTROL_VCM)       || \
      ( rateControl == RATECONTROL_ICQ)       || \
      ( rateControl == RATECONTROL_QVBR)      || \
      ( rateControl == RATECONTROL_IWD_VBR))  && \
            ( standard == CODECHAL_AVC ))               )

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

typedef struct _CODECHAL_VLD_SLICE_RECORD
{
    uint32_t   dwSkip;
    uint32_t   dwOffset;
    uint32_t   dwLength;
    uint32_t   dwSliceStartMbOffset;
    bool       bIsLastSlice;
} CODECHAL_VLD_SLICE_RECORD, *PCODECHAL_VLD_SLICE_RECORD;

typedef struct _CODECHAL_VC1_VLD_SLICE_RECORD
{
    uint32_t   dwSkip;
    uint32_t   dwOffset;
    uint32_t   dwLength;
    uint32_t   dwSliceYOffset;
    uint32_t   dwNextSliceYOffset;
} CODECHAL_VC1_VLD_SLICE_RECORD, *PCODECHAL_VC1_VLD_SLICE_RECORD;

typedef struct _CODECHAL_WALKER_CODEC_PARAMS
{
    uint32_t                WalkerMode;
    bool                    bUseScoreboard;
    uint32_t                dwResolutionX;
    uint32_t                dwResolutionY;
    bool                    bNoDependency;
    CODECHAL_WALKER_DEGREE  WalkerDegree;
    bool                    bUseVerticalRasterScan;
    uint32_t                wPictureCodingType;
    bool                    bMbEncIFrameDistInUse;
    bool                    bMbaff;
    uint32_t                bDirectSpatialMVPredFlag;
    bool                    bColorbitSupported;
    uint32_t                dwNumSlices;
    uint16_t                usSliceHeight;
    bool                    bGroupIdSelectSupported;
    uint8_t                 ucGroupId;
    uint32_t                ScoreboardMask;
    uint16_t                usTotalThreadNumPerLcu; //Used by Gen10 HEVC
} CODECHAL_WALKER_CODEC_PARAMS, *PCODECHAL_WALKER_CODEC_PARAMS;

typedef struct _CODECHAL_SURFACE_CODEC_PARAMS
{
    uint32_t                    Mode;
    bool                        bIs2DSurface;
    bool                        bUseUVPlane;
    bool                        bUseAdvState;
    bool                        bMediaBlockRW;
    bool                        bUse16UnormSurfaceFormat;   // Force surface format to R16_UNORM
    bool                        bUse32UnormSurfaceFormat;   // Force surface format to R32_UNORM
    bool                        bUseARGB8Format;            // Force surface format to ARGB8 for Ds+Copy kernel
    bool                        bUse32UINTSurfaceFormat;
    bool                        bRenderTarget;
    bool                        bIsWritable;
    bool                        bUseHalfHeight;
    PMOS_SURFACE                psSurface;              // 2D surface parameters
    PMOS_RESOURCE               presBuffer;             // Buffer parameters
    uint32_t                    dwSize;                 // Buffer size
    uint32_t                    dwOffset;               // Buffer offset
    uint32_t                    dwBindingTableOffset;   // Binding table offset for given surface
    uint32_t                    dwUVBindingTableOffset; // Binding table offset for the UV plane
    uint32_t                    dwVerticalLineStride;
    uint32_t                    dwVerticalLineStrideOffset;
    uint8_t                     ucVDirection;
    uint32_t                    dwCacheabilityControl;
    bool                        bForceChromaFormat;
    uint32_t                    ChromaType;
    bool                        bRawSurface;
    uint8_t                     ucSurfaceStateId;

    uint32_t                    dwWidthInUse;
    uint32_t                    dwHeightInUse;

} CODECHAL_SURFACE_CODEC_PARAMS, *PCODECHAL_SURFACE_CODEC_PARAMS;

typedef struct _CODECHAL_KERNEL_HEADER
{
    union
    {
        struct
        {
            uint32_t                                       : 6;
            uint32_t       KernelStartPointer              : 26;   // GTT 31:6
        };
        struct
        {
            uint32_t       Value;
        };
    };
} CODECHAL_KERNEL_HEADER, *PCODECHAL_KERNEL_HEADER;

typedef union _CODECHAL_CS_ENGINE_ID
{
    struct
    {
        uint32_t       ClassId            : 3;    //[0...4]
        uint32_t       ReservedFiled1     : 1;    //[0]
        uint32_t       InstanceId         : 6;    //[0...7]
        uint32_t       ReservedField2     : 22;   //[0]
    } fields;
    uint32_t            value;
} CODECHAL_CS_ENGINE_ID, *PCODECHAL_CS_ENGINE_ID;

#define CodecHal_AllocateDataList(type, dataList, length)     \
{                                                               \
    type *ptr;                                                  \
    ptr = (type *)MOS_AllocAndZeroMemory(sizeof(type) * length);\
    if (ptr == nullptr)                                            \
    {                                                           \
        CODECHAL_PUBLIC_ASSERTMESSAGE("No memory to allocate CodecHal data list."); \
        eStatus = MOS_STATUS_NO_SPACE;                          \
        return eStatus;                                         \
    }                                                           \
    for (auto i = 0; i < length; i++)                           \
    {                                                           \
        dataList[i] = &(ptr[i]);                              \
    }                                                           \
}

#define CodecHal_FreeDataList(dataList, length)   \
{                                                   \
    void* ptr;                                      \
    ptr = (void*)(dataList[0]);                   \
    if (ptr)                                        \
    {                                               \
        MOS_FreeMemory(ptr);                        \
    }                                               \
    for (auto i = 0; i < length; i++)               \
    {                                               \
        dataList[i] = nullptr;                       \
    }                                               \
}

MOS_STATUS CodecHal_GetResourceInfo(
    PMOS_INTERFACE      osInterface,
    PMOS_SURFACE        surface);

uint32_t CodecHal_CeilLog2(uint32_t a);

MOS_STATUS CodecHal_InitMediaObjectWalkerParams(
    CodechalHwInterface             *hwInterface,
    PMHW_WALKER_PARAMS              walkerParams,
    PCODECHAL_WALKER_CODEC_PARAMS   walkerCodecParams);

//!
//! \brief    Loads kernel data into the ISH
//! \details  Uses the data described in the kernel state to assign an ISH block and load the kernel data into it
//! \param    stateHeapInterface
//!           [in] State heap interface
//! \param    kernelState
//!           [in] Kernel state describing the kernel data to be loaded
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHal_MhwInitISH(
    PMHW_STATE_HEAP_INTERFACE   stateHeapInterface,
    PMHW_KERNEL_STATE           kernelState);

//!
//! \brief    Assigns space in both DSH and SSH to the kernel state
//! \details  Uses input parameters to assign DSH/SSH regions to the kernel state
//! \param    stateHeapInterface
//!           [in] State heap interface
//! \param    kernelState
//!           [in] The kernel state to assign the new DSH/ISH regions
//! \param    noDshSpaceRequested
//!           [in] No DSH space should be assigned in this call
//! \param    forcedDshSize
//!           [in] The size of the DSH space required for this kernel state.
//!                If this value is 0, the size is calculated from the kernel state.
//! \param    noSshSpaceRequested
//!           [in] No SSH space should be assigned in this call
//! \param    currCmdBufId
//!           [in] Command buffer Id to keep track of the state heap resource
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHal_AssignDshAndSshSpace(
    PMHW_STATE_HEAP_INTERFACE   stateHeapInterface,
    PMHW_KERNEL_STATE           kernelState,
    bool                        noDshSpaceRequested,
    uint32_t                    forcedDshSize,
    bool                        noSshSpaceRequested,
    uint32_t                    currCmdBufId);

//!
//! \brief    Gets a kernel information for a specific KUID from the combined kernel
//! \details  Gets a kernel information for a specific KUID from the combined kernel
//! \param    kernelBase
//!           [in] The combined kernel
//! \param    kernelUID
//!           [in] The unique kernel identifier in the combined kernel
//! \param    kernelBinary
//!           [in,out] The binary of the kernel specified by dwKUID
//! \param    size
//!           [in,out] The size of the kernel specified by dwKUID
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHal_GetKernelBinaryAndSize(
    uint8_t*   kernelBase,
    uint32_t   kernelUID,
    uint8_t**  kernelBinary,
    uint32_t*  size);

//!
//! \brief    Get the surface width in bytes
//! \details  Get the suface width in bytes
//! \param    surface
//!           [in] Surface pointer
//! \param    widthInBytes
//!           [out] Output the surface width
//! \return   void
//!
void CodecHal_GetSurfaceWidthInBytes(
    PMOS_SURFACE                surface,
    uint32_t                   *widthInBytes);

MOS_STATUS CodecHal_SetRcsSurfaceState(
    CodechalHwInterface             *hwInterface,
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PCODECHAL_SURFACE_CODEC_PARAMS  surfaceCodecParams,
    PMHW_KERNEL_STATE               kernelState);

MOS_STATUS CodecHal_UserFeature_ReadValue(
    PMOS_USER_FEATURE_INTERFACE     osUserFeatureInterface,
    MOS_USER_FEATURE_VALUE_ID       valueID,
    PMOS_USER_FEATURE_VALUE_DATA    valueData);

MOS_STATUS CodecHal_UserFeature_WriteValue(
    PMOS_USER_FEATURE_INTERFACE             osUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      writeValues);

//!
//! \brief    Allocate and Initialize HUC resource gfx buffer
//! \details  Allocate a gfx buffer, and memcpy the on-demanding
//!           HuC kernel to it for later use.
//! \param    osInterface
//!           [in] OS Interface
//! \param    huCFwBuffer
//!           [in] OS resource structure for dst buffer.
//! \param    hucFwBinaryBase
//!           [in] memcpy src buffer address.
//! \param    hucFwBinarySize
//!           [in] memcpy data size
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, otherwise error code
//!
MOS_STATUS CodecHal_AllocateAndInitHucFwBufferResource(
    PMOS_INTERFACE          osInterface,
    const PMOS_RESOURCE     huCFwBuffer,
    const void             *hucFwBinaryBase,
    uint32_t                hucFwBinarySize);

#endif  // __CODECHAL_COMMON_H__
