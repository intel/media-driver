/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codec_def_common_encode.h
//! \brief    Defines common types and macros shared by CodecHal, MHW, and DDI layer for encode.
//! \details  All codec_def_encode may include this file which should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_COMMON_ENCODE_H__
#define __CODEC_DEF_COMMON_ENCODE_H__

#include "mos_defs.h"

#define CODEC_NUM_REF_BUFFERS               (CODEC_MAX_NUM_REF_FRAME + 1) // Max 16 references (for AVC) + 1 for the current frame
#define CODEC_NUM_NON_REF_BUFFERS           3
#define CODEC_NUM_TRACKED_BUFFERS           (CODEC_NUM_REF_BUFFERS + CODEC_NUM_NON_REF_BUFFERS)
#define CODEC_CURR_TRACKED_BUFFER           CODEC_NUM_TRACKED_BUFFERS

//BRC
#define BRC_IMG_STATE_SIZE_PER_PASS         128
#define BRC_IMG_STATE_SIZE_PER_PASS_G10     144
#define BRC_IMG_STATE_SIZE_PER_PASS_G11     192

// Quality/Performance differentiators for HSW AVC Encode
#define NUM_TARGET_USAGE_MODES 8
#define NUM_VDENC_TARGET_USAGE_MODES 8

//weighted prediction
#define CODEC_NUM_WP_FRAME              8
#define CODEC_MAX_FORWARD_WP_FRAME      6
#define CODEC_MAX_BACKWARD_WP_FRAME     2
#define CODEC_WP_OUTPUT_L0_START        0
#define CODEC_WP_OUTPUT_L1_START        6

#define CODEC_MAX_PIC_WIDTH            1920
#define CODEC_MAX_PIC_HEIGHT           1920                // Tablet usage in portrait mode, image resolution = 1200x1920, so change MAX_HEIGHT to 1920

#define CODEC_2K_MAX_PIC_WIDTH         2048
#define CODEC_2K_MAX_PIC_HEIGHT        2048

#define CODEC_4K_MAX_PIC_WIDTH         4096
#define CODEC_4K_MAX_PIC_HEIGHT        4096

#define CODEC_8K_MAX_PIC_WIDTH    8192
#define CODEC_8K_MAX_PIC_HEIGHT   8192

#define CODEC_16K_MAX_PIC_WIDTH        16384
#define CODEC_16K_MAX_PIC_HEIGHT       16384

#define CODECHAL_MAD_BUFFER_SIZE                4 // buffer size is 4 bytes

// HME
#define SCALE_FACTOR_2x     2
#define SCALE_FACTOR_4x     4
#define SCALE_FACTOR_16x    16
#define SCALE_FACTOR_32x    32

#define CODECHAL_VP9_MB_CODE_SIZE                   204

typedef struct tagENCODE_RECT
{
    uint16_t  Top;    // [0..(FrameHeight+ M-1)/M -1]
    uint16_t  Bottom; // [0..(FrameHeight+ M-1)/M -1]
    uint16_t  Left;   // [0..(FrameWidth+15)/16-1]
    uint16_t  Right;  // [0..(FrameWidth+15)/16-1]
} ENCODE_RECT;

typedef struct tagMOVE_RECT
{
    uint32_t  SourcePointX;
    uint32_t  SourcePointY;
    uint32_t  DestRectTop;
    uint32_t  DestRectBottom;
    uint32_t  DestRectLeft;
    uint32_t  DestRectRight;
} MOVE_RECT;

/*! \brief Defines ROI settings.
*
*    {Top, Bottom, Left, Right} defines the ROI boundary. The values are in unit of blocks. The block size M should use LCU size (e.g. sif LCU size is 32x32, M is 32). And its range should be within the frame boundary, so that:
*        0 <= Top <= Bottom <= (FrameHeight+ M-1)/M -1
*        0 <= Left <= Right <= (FrameWidth+M-1)/M-1
*    If input range is out of frame boundary, driver should trim it.
*    ROI alignes with LCU based rectangular blocks and cannot have arbitrary pixel-based location.
*    Region overlapping is allowed. For MBs reside within more than one ROIs, parameters from ROI with smaller index rules. For example, when ROI[0] and ROI[1] overlap on a certain area, the QP value for the overlapped area will be determined by value of ROI[0]. The order of ROI[] reflects objects’ relative relationship of depth. Foreground objects should have ROI index smaller than background objects.
*/
typedef struct _CODEC_ROI
{
    uint16_t        Top;                //!< [0..(FrameHeight+15)/16-1]
    uint16_t        Bottom;             //!< [0..(FrameHeight+15)/16-1]
    uint16_t        Left;               //!< [0..(FrameWidth+15)/16-1]
    uint16_t        Right;              //!< [0..(FrameWidth+15)/16-1]
    /*! \brief For ROIValueInDeltaQP equals CQP case, this parameter gives explicit delta QP value of ROI regional QP vs. frame QP.
    *
    *    Value range [-51..51]. If regional QP PriorityLevelOfDQp + QpY is out of range of [0..51], driver should crop it. It could be applied on both CQP and BRC cases. For ROIValueInDeltaQP equals 0BRC cases, this parameter describes the priority level of the ROI region. Value range [-3..3]. The higher the absolute value, the bigger range of delta QP is allowed. And it is usually applies on BRC case. BRC will decide the actual delta QP value. Positive priority level means negative delta QP should be applied. And negative priority level means positive delta QP which implies the region should be intentionally blurred. In either case, value Priority level 0 means same as non-ROI region. It is suggested that application does not set value 0. But if it happens, driver will treat that ROI as part of non-ROI background.
    */
    char            PriorityLevelOrDQp;
} CODEC_ROI, *PCODEC_ROI;

/*! \brief Indicates the uncompressed input color space
*
*    Valid only when input is ARGB format.
*/
typedef enum _CODEC_INPUT_COLORSPACE
{
    ECOLORSPACE_P709 = 0,
    ECOLORSPACE_P601 = 1,
    ECOLORSPACE_P2020 = 2
} CODEC_INPUT_COLORSPACE, ENCODE_INPUT_COLORSPACE;

/*! \brief Indicates the tolerance the application has to variations in the frame size.
*
*    For example, wireless display scenarios may require very steady bitrate to reduce buffering time.  It affects the BRC algorithm used, but may or may not have an effect based on the combination of other BRC parameters.  Only valid when the driver reports support for FrameSizeToleranceSupport.
*/
typedef enum _CODEC_FRAMESIZE_TOLERANCE
{
    EFRAMESIZETOL_NORMAL        = 0,
    EFRAMESIZETOL_LOW           = 1,    //!< Maps to "sliding window"
    EFRAMESIZETOL_EXTREMELY_LOW = 2     //!< Maps to "low delay"
} CODEC_FRAMESIZE_TOLERANCE, ENCODE_FRAMESIZE_TOLERANCE;

/*! \brief Provides a hint to encoder about the scenario for the encoding session.
*
*    BRC algorithm may tune differently based on this info.
*/
typedef enum _CODEC_SCENARIO
{
    ESCENARIO_UNKNOWN           = 0,
    ESCENARIO_DISPLAYREMOTING   = 1,
    ESCENARIO_VIDEOCONFERENCE   = 2,
    ESCENARIO_ARCHIVE           = 3,
    ESCENARIO_LIVESTREAMING     = 4,
    ESCENARIO_VIDEOCAPTURE      = 5,
    ESCENARIO_VIDEOSURVEILLANCE = 6,
    ESCENARIO_GAMESTREAMING     = 7,
    ESCENARIO_REMOTEGAMING      = 8
} CODEC_SCENARIO, ENCODE_SCENARIO;

/*! \brief Provides a hint to encoder about the content for the encoding session.
*/
typedef enum _CODEC_CONTENT
{
    ECONTENT_UNKNOWN            = 0,
    ECONTENT_FULLSCREENVIDEO    = 1,
    ECONTENT_NONVIDEOSCREEN     = 2
} CODEC_CONTENT, ENCODE_CONTENT;

typedef enum
{
    RATECONTROL_CBR         = 1,
    RATECONTROL_VBR         = 2,
    RATECONTROL_CQP         = 3,
    RATECONTROL_AVBR        = 4,
    RATECONTROL_RESERVED0   = 8, // This is used by MSDK for Lookahead and hence not used here
    RATECONTROL_ICQ         = 9,
    RATECONTROL_VCM         = 10,
    RATECONTROL_QVBR        = 14,
    RATECONTROL_CQL         = 15,
    RATECONTROL_IWD_VBR     = 100
} RATE_CONTROL_METHOD;

typedef enum
{
    DEFAULT_WEIGHTED_INTER_PRED_MODE  =  0,
    EXPLICIT_WEIGHTED_INTER_PRED_MODE =  1,
    IMPLICIT_WEIGHTED_INTER_PRED_MODE =  2,
    INVALID_WEIGHTED_INTER_PRED_MODE  = -1
} WEIGHTED_INTER_PRED_MODE;

// used from MHW & DDI
typedef enum
{
    ROLLING_I_DISABLED  = 0,
    ROLLING_I_COLUMN    = 1,
    ROLLING_I_ROW       = 2,
    ROLLING_I_SQUARE    = 3
} ROLLING_I_SETTING;

typedef enum
{
    BRC_ROLLING_I_DISABLED  = 0,
    BRC_ROLLING_I_COLUMN    = 4,
    BRC_ROLLING_I_ROW       = 8,
    BRC_ROLLING_I_SQUARE    = 12,
    BRC_ROLLING_I_QP        = 13
}BRC_ROLLING_I_SETTING;

typedef enum _CODECHAL_MFX_SURFACE_ID
{
    CODECHAL_MFX_REF_SURFACE_ID     = 0,
    CODECHAL_MFX_SRC_SURFACE_ID     = 4,
    CODECHAL_MFX_DSRECON_SURFACE_ID = 5
} CODECHAL_MFX_SURFACE_ID;

typedef enum _CODECHAL_HCP_SURFACE_ID
{
    CODECHAL_HCP_DECODED_SURFACE_ID         = 0,
    CODECHAL_HCP_SRC_SURFACE_ID             = 1,    // Encode
    CODECHAL_HCP_LAST_SURFACE_ID            = 2,    // VP9
    CODECHAL_HCP_GOLDEN_SURFACE_ID          = 3,    // VP9
    CODECHAL_HCP_ALTREF_SURFACE_ID          = 4,    // VP9
    CODECHAL_HCP_REF_SURFACE_ID             = 5
} CODECHAL_HCP_SURFACE_ID;

// ---------------------------
// Structures
// ---------------------------
// used from MHW & DDI
typedef struct _BSBuffer
{
    uint8_t   *pBase;
    uint8_t   *pCurrent;
    uint32_t  SliceOffset;    // Slice offset, always byte aligned
    uint8_t   BitOffset;      // bit offset for pCurrent.
    uint32_t  BitSize;        // bit size per slice, first slice may include SPS & PPS
    uint32_t  BufferSize;     // buffer size
} BSBuffer, *PBSBuffer;

typedef struct _CODEC_ENCODER_SLCDATA
{
    uint32_t    SliceOffset;
    uint32_t    BitSize;
    uint32_t    CmdOffset;
    uint32_t    SkipEmulationByteCount;

    // MPEG2 only
    struct
    {
        uint8_t   SliceGroup;
        uint16_t  NextSgMbXCnt;
        uint16_t  NextSgMbYCnt;
    };
} CODEC_ENCODER_SLCDATA, *PCODEC_ENCODER_SLCDATA;

typedef struct _CODECHAL_NAL_UNIT_PARAMS
{
    uint32_t       uiNalUnitType;
    uint32_t       uiOffset;
    uint32_t       uiSize;
    bool           bInsertEmulationBytes;
    uint32_t       uiSkipEmulationCheckCount;
} CODECHAL_NAL_UNIT_PARAMS, *PCODECHAL_NAL_UNIT_PARAMS;

typedef struct tagFRAMERATE
{
    uint32_t    Numerator;
    uint32_t    Denominator;
} FRAMERATE;

/*********************************************************************************\
    Constants for VDENC costing look-up-tables
\*********************************************************************************/
typedef enum _CODEC_VDENC_LUTMODE
{
    CODEC_VDENC_LUTMODE_INTRA_SADMPM               = 0x00,
    CODEC_VDENC_LUTMODE_INTRA_32x32                = 0x01,
    CODEC_VDENC_LUTMODE_INTRA_16x16                = 0x02,
    CODEC_VDENC_LUTMODE_INTRA_8x8                  = 0x03,
    CODEC_VDENC_LUTMODE_INTER_32x16                = 0x04,
    CODEC_VDENC_LUTMODE_INTER_16x32                = 0x04,
    CODEC_VDENC_LUTMODE_INTER_AMP                  = 0x04,  //All asymmetrical shapes
    CODEC_VDENC_LUTMODE_INTER_16x16                = 0x05,
    CODEC_VDENC_LUTMODE_INTER_16x8                 = 0x06,
    CODEC_VDENC_LUTMODE_INTER_8x16                 = 0x06,
    CODEC_VDENC_LUTMODE_INTER_8x8                  = 0x07,
    CODEC_VDENC_LUTMODE_INTER_32x32                = 0x08,
    CODEC_VDENC_LUTMODE_INTER_BIDIR                = 0x09,
    CODEC_VDENC_LUTMODE_REF_ID                     = 0x0A,
    CODEC_VDENC_LUTMODE_INTRA_CHROMA               = 0x0B,
    CODEC_VDENC_LUTMODE_INTRA_NxN                  = 0x0C,
    CODEC_VDENC_LUTMODE_INTRA_RDEMPM               = 0x0D,
    CODEC_VDENC_LUTMODE_MERGE_32X32                = 0x0E,
    CODEC_VDENC_LUTMODE_MERGE_16x16                = 0x0F,
    CODEC_VDENC_LUTMODE_MERGE_8x8                  = 0x10,
    CODEC_VDENC_LUTMODE_SKIP_32X32                 = 0x11,
    CODEC_VDENC_LUTMODE_SKIP_16x16                 = 0x12,
    CODEC_VDENC_LUTMODE_SKIP_8x8                   = 0x13,
    CODEC_VDENC_LUTMODE_INTRA_DC_32x32_SAD         = 0x14,
    CODEC_VDENC_LUTMODE_INTRA_DC_16x16_SAD         = 0x15,
    CODEC_VDENC_LUTMODE_INTRA_DC_8x8_SAD           = 0x16,
    CODEC_VDENC_LUTMODE_INTRA_DC_4x4_SAD           = 0x17,
    CODEC_VDENC_LUTMODE_INTRA_NONDC_32x32_SAD      = 0x18,
    CODEC_VDENC_LUTMODE_INTRA_NONDC_16x16_SAD      = 0x19,
    CODEC_VDENC_LUTMODE_INTRA_NONDC_8x8_SAD        = 0x1A,
    CODEC_VDENC_LUTMODE_INTRA_NONDC_4x4_SAD        = 0x1B,
    CODEC_VDENC_LUTMODE_INTRA_DC_32x32_RD          = 0x1C,
    CODEC_VDENC_LUTMODE_INTRA_DC_8x8_RD            = 0x1D,
    CODEC_VDENC_LUTMODE_INTRA_NONDC_32x32_RD       = 0x1E,
    CODEC_VDENC_LUTMODE_INTRA_NONDC_8x8_RD         = 0x1F,
    CODEC_VDENC_LUTMODE_INTRA_LEFT_BOUNDARY_SAD    = 0x20,
    CODEC_VDENC_LUTMODE_INTRA_TOP_BOUNDARY_SAD     = 0x21,
    CODEC_VDENC_LUTMODE_INTRA_TU_SPLIT             = 0x22,
    CODEC_VDENC_LUTMODE_INTER_TU_SPLIT             = 0x23,
    CODEC_VDENC_LUTMODE_TU_CBF_FLAG                = 0x24,
    CODEC_VDENC_LUTMODE_INTRA_TU_32_CBF_FLAG       = CODEC_VDENC_LUTMODE_TU_CBF_FLAG + 0,
    CODEC_VDENC_LUTMODE_INTRA_TU_16_CBF_FLAG       = CODEC_VDENC_LUTMODE_TU_CBF_FLAG + 1,
    CODEC_VDENC_LUTMODE_INTRA_TU_8_CBF_FLAG        = CODEC_VDENC_LUTMODE_TU_CBF_FLAG + 2,
    CODEC_VDENC_LUTMODE_INTRA_TU_4_CBF_FLAG        = CODEC_VDENC_LUTMODE_TU_CBF_FLAG + 3,
    CODEC_VDENC_LUTMODE_INTER_TU_32_CBF_FLAG       = CODEC_VDENC_LUTMODE_TU_CBF_FLAG + 4,
    CODEC_VDENC_LUTMODE_INTER_TU_16_CBF_FLAG       = CODEC_VDENC_LUTMODE_TU_CBF_FLAG + 5,
    CODEC_VDENC_LUTMODE_INTER_TU_8_CBF_FLAG        = CODEC_VDENC_LUTMODE_TU_CBF_FLAG + 6,
    CODEC_VDENC_LUTMODE_INTER_TU_4_CBF_FLAG        = CODEC_VDENC_LUTMODE_TU_CBF_FLAG + 7,

    CODEC_VDENC_LUTMODE_TU_COEF_EST                = 0x2C,
    CODEC_VDENC_LUTMODE_INTRA_TU_32_NZC            = CODEC_VDENC_LUTMODE_TU_COEF_EST + 0,
    CODEC_VDENC_LUTMODE_INTRA_TU_16_NZC            = CODEC_VDENC_LUTMODE_TU_COEF_EST + 1,
    CODEC_VDENC_LUTMODE_INTRA_TU_8_NZC             = CODEC_VDENC_LUTMODE_TU_COEF_EST + 2,
    CODEC_VDENC_LUTMODE_INTRA_TU_4_NZC             = CODEC_VDENC_LUTMODE_TU_COEF_EST + 3,
    CODEC_VDENC_LUTMODE_INTER_TU_32_NZC            = CODEC_VDENC_LUTMODE_TU_COEF_EST + 4,
    CODEC_VDENC_LUTMODE_INTER_TU_16_NZC            = CODEC_VDENC_LUTMODE_TU_COEF_EST + 5,
    CODEC_VDENC_LUTMODE_INTER_TU_8_NZC             = CODEC_VDENC_LUTMODE_TU_COEF_EST + 6,
    CODEC_VDENC_LUTMODE_INTER_TU_4_NZC             = CODEC_VDENC_LUTMODE_TU_COEF_EST + 7,

    CODEC_VDENC_LUTMODE_INTRA_TU_32_NSIGC          = CODEC_VDENC_LUTMODE_TU_COEF_EST + 8,
    CODEC_VDENC_LUTMODE_INTRA_TU_16_NSIGC          = CODEC_VDENC_LUTMODE_TU_COEF_EST + 9,
    CODEC_VDENC_LUTMODE_INTRA_TU_8_NSIGC           = CODEC_VDENC_LUTMODE_TU_COEF_EST + 10,
    CODEC_VDENC_LUTMODE_INTRA_TU_4_NSIGC           = CODEC_VDENC_LUTMODE_TU_COEF_EST + 11,
    CODEC_VDENC_LUTMODE_INTER_TU_32_NSIGC          = CODEC_VDENC_LUTMODE_TU_COEF_EST + 12,
    CODEC_VDENC_LUTMODE_INTER_TU_16_NSIGC          = CODEC_VDENC_LUTMODE_TU_COEF_EST + 13,
    CODEC_VDENC_LUTMODE_INTER_TU_8_NSIGC           = CODEC_VDENC_LUTMODE_TU_COEF_EST + 14,
    CODEC_VDENC_LUTMODE_INTER_TU_4_NSIGC           = CODEC_VDENC_LUTMODE_TU_COEF_EST + 15,

    CODEC_VDENC_LUTMODE_INTRA_TU_32_NSUBSETC       = CODEC_VDENC_LUTMODE_TU_COEF_EST + 16,
    CODEC_VDENC_LUTMODE_INTRA_TU_16_NSUBSETC       = CODEC_VDENC_LUTMODE_TU_COEF_EST + 17,
    CODEC_VDENC_LUTMODE_INTRA_TU_8_NSUBSETC        = CODEC_VDENC_LUTMODE_TU_COEF_EST + 18,
    CODEC_VDENC_LUTMODE_INTRA_TU_4_NSUBSETC        = CODEC_VDENC_LUTMODE_TU_COEF_EST + 19,
    CODEC_VDENC_LUTMODE_INTER_TU_32_NSUBSETC       = CODEC_VDENC_LUTMODE_TU_COEF_EST + 20,
    CODEC_VDENC_LUTMODE_INTER_TU_16_NSUBSETC       = CODEC_VDENC_LUTMODE_TU_COEF_EST + 21,
    CODEC_VDENC_LUTMODE_INTER_TU_8_NSUBSETC        = CODEC_VDENC_LUTMODE_TU_COEF_EST + 22,
    CODEC_VDENC_LUTMODE_INTER_TU_4_NSUBSETC        = CODEC_VDENC_LUTMODE_TU_COEF_EST + 23,

    CODEC_VDENC_LUTMODE_INTRA_TU_32_NLEVELC        = CODEC_VDENC_LUTMODE_TU_COEF_EST + 24,
    CODEC_VDENC_LUTMODE_INTRA_TU_16_NLEVELC        = CODEC_VDENC_LUTMODE_TU_COEF_EST + 25,
    CODEC_VDENC_LUTMODE_INTRA_TU_8_NLEVELC         = CODEC_VDENC_LUTMODE_TU_COEF_EST + 26,
    CODEC_VDENC_LUTMODE_INTRA_TU_4_NLEVELC         = CODEC_VDENC_LUTMODE_TU_COEF_EST + 27,
    CODEC_VDENC_LUTMODE_INTER_TU_32_NLEVELC        = CODEC_VDENC_LUTMODE_TU_COEF_EST + 28,
    CODEC_VDENC_LUTMODE_INTER_TU_16_NLEVELC        = CODEC_VDENC_LUTMODE_TU_COEF_EST + 29,
    CODEC_VDENC_LUTMODE_INTER_TU_8_NLEVELC         = CODEC_VDENC_LUTMODE_TU_COEF_EST + 30,
    CODEC_VDENC_LUTMODE_INTER_TU_4_NLEVELC         = CODEC_VDENC_LUTMODE_TU_COEF_EST + 31,

    // VP9 specific cost
    CODEC_VDENC_LUTMODE_INTRA_32x16                = 0x4C,
    CODEC_VDENC_LUTMODE_INTRA_16x8                 = 0x4D,
    CODEC_VDENC_LUTMODE_INTER_NEARESTMV            = 0x4E,
    CODEC_VDENC_LUTMODE_INTER_NEARMV               = 0x4F,
    CODEC_VDENC_LUTMODE_INTER_ZEROMV               = 0x50,
    CODEC_VDENC_LUTMODE_TU_DEPTH0                  = 0x51,
    CODEC_VDENC_LUTMODE_TU_DEPTH1                  = 0x52,
    CODEC_VDENC_LUTMODE_TU_DEPTH2                  = 0x53,

    CODEC_VDENC_LUTMODE_INTRA_64X64DC              = 0x54,
    CODEC_VDENC_LUTMODE_MERGE_64X64                = 0x55,
    CODEC_VDENC_LUTMODE_SKIP_64X64                 = 0x56,

    CODEC_VDENC_NUM_MODE_COST                      = 0x57
} CODEC_VDENC_LUTMODE;

// Batch buffer type
enum
{
    MB_ENC_Frame_BB    = 0,
    MB_ENC_Field_BB,
    //Add new buffer type here
    NUM_ENCODE_BB_TYPE
};

typedef enum
{
    FRAME_NO_SKIP       = 0,        // encode as normal, no skip frames
    FRAME_SKIP_NORMAL   = 1         // one or more frames were skipped prior to curr frame. Encode curr frame as normal, update BRC
} FRAME_SKIP_FLAG;

typedef enum _CODEC_SLICE_STRUCTS
{
    CODECHAL_SLICE_STRUCT_ONESLICE           = 0,    // Once slice for the whole frame
    CODECHAL_SLICE_STRUCT_POW2ROWS           = 1,    // Slices are power of 2 number of rows, all slices the same
    CODECHAL_SLICE_STRUCT_ROWSLICE           = 2,    // Slices are any number of rows, all slices the same
    CODECHAL_SLICE_STRUCT_ARBITRARYROWSLICE  = 3,    // Slices are any number of rows, slices can be different
    CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE   = 4     // Slices are any number of MBs, slices can be different
    // 5 - 7 are Reserved
} CODEC_SLICE_STRUCTS;

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

#define CodecHalUsesPakEngine(codecFunction)   \
    (codecFunction == CODECHAL_FUNCTION_PAK       ||  \
     codecFunction == CODECHAL_FUNCTION_ENC_PAK)

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

// The current definition of the first encode mode CODECHAL_ENCODE_MODE_AVC should be used
// as a base for subsequent encode modes
#define CODECHAL_ENCODE_MODE_BIT_OFFSET     ((uint32_t)(log((double)CODECHAL_ENCODE_MODE_AVC)/log(2.)))
#define CODECHAL_ENCODE_MODE_BIT_MASK       (( 1L << CODECHAL_ENCODE_MODE_BIT_OFFSET) - 1 )

#endif  // __CODEC_DEF_COMMON_ENCODE_H__
