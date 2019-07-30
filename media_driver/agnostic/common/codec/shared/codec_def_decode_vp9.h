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
//! \file     codec_def_decode_vp9.h
//! \brief    Defines decode VP9 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to VP9 decode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_DECODE_VP9_H__
#define __CODEC_DEF_DECODE_VP9_H__

#include "codec_def_common.h"
#include "codec_def_common_vp9.h"

#define CODECHAL_MAX_CUR_NUM_REF_FRAME_VP9      3
#define CODECHAL_DECODE_VP9_MAX_NUM_REF_FRAME   8
#define CODECHAL_VP9_NUM_MV_BUFFERS             2
#define VP9_CENC_PRIMITIVE_CMD_OFFSET_IN_DW     16

//!
//! \enum CODECHAL_DECODE_VP9_SEG_LVL_FEATURES
//! VP9 decode segment level
//!
typedef enum
{
    CODECHAL_DECODE_VP9_SEG_LVL_ALT_Q = 0,          //!< Use alternate Quantizer
    CODECHAL_DECODE_VP9_SEG_LVL_ALT_LF = 1,         //!< Use alternate loop filter value
    CODECHAL_DECODE_VP9_SEG_LVL_REF_FRAME = 2,      //!< Optional Segment reference frame
    CODECHAL_DECODE_VP9_SEG_LVL_SKIP = 3,           //!< Optional Segment (0,0) + skip mode
    CODECHAL_DECODE_VP9_SEG_LVL_MAX = 4             //!< Number of features supported
} CODECHAL_DECODE_VP9_SEG_LVL_FEATURES;

//!
//! \enum CODECHAL_DECODE_VP9_MV_REFERENCE_FRAME
//! VP9 decode mv reference
//!
typedef enum
{
    CODECHAL_DECODE_VP9_NONE = -1,
    CODECHAL_DECODE_VP9_INTRA_FRAME = 0,
    CODECHAL_DECODE_VP9_LAST_FRAME = 1,
    CODECHAL_DECODE_VP9_GOLDEN_FRAME = 2,
    CODECHAL_DECODE_VP9_ALTREF_FRAME = 3,
    CODECHAL_DECODE_VP9_MAX_REF_FRAMES = 4
} CODECHAL_DECODE_VP9_MV_REFERENCE_FRAME;

// VP9 Decode Slice Parameter Buffer
typedef struct _CODEC_VP9_SLICE_PARAMS {
    uint32_t              BSNALunitDataLocation;
    uint32_t              SliceBytesInBuffer;
    uint16_t              wBadSliceChopping;
} CODEC_VP9_SLICE_PARAMS, *PCODEC_VP9_SLICE_PARAMS;

// VP9 Picture Parameters Buffer
typedef struct _CODEC_VP9_PIC_PARAMS
{
    uint16_t              FrameHeightMinus1;               // [0..65535]
    uint16_t              FrameWidthMinus1;                // [0..65535]

    union
    {
        struct
        {
            uint32_t        frame_type                      : 1;        // [0..1]
            uint32_t        show_frame                      : 1;        // [0..1]
            uint32_t        error_resilient_mode            : 1;        // [0..1]
            uint32_t        intra_only                      : 1;        // [0..1]
            uint32_t        LastRefIdx                      : 3;        // [0..7]
            uint32_t        LastRefSignBias                 : 1;        // [0..1]
            uint32_t        GoldenRefIdx                    : 3;        // [0..7]
            uint32_t        GoldenRefSignBias               : 1;        // [0..1]
            uint32_t        AltRefIdx                       : 3;        // [0..7]
            uint32_t        AltRefSignBias                  : 1;        // [0..1]
            uint32_t        allow_high_precision_mv         : 1;        // [0..1]
            uint32_t        mcomp_filter_type               : 3;        // [0..7]
            uint32_t        frame_parallel_decoding_mode    : 1;        // [0..1]
            uint32_t        segmentation_enabled            : 1;        // [0..1]
            uint32_t        segmentation_temporal_update    : 1;        // [0..1]
            uint32_t        segmentation_update_map         : 1;        // [0..1]
            uint32_t        reset_frame_context             : 2;        // [0..3]
            uint32_t        refresh_frame_context           : 1;        // [0..1]
            uint32_t        frame_context_idx               : 2;        // [0..3]
            uint32_t        LosslessFlag                    : 1;        // [0..1]
            uint32_t        ReservedField                   : 2;        // [0]
        } fields;
        uint32_t            value;
    } PicFlags;

    CODEC_PICTURE       RefFrameList[8];                            // [0..127, 0xFF]
    CODEC_PICTURE       CurrPic;                                    // [0..127]
    uint8_t             filter_level;                               // [0..63]
    uint8_t             sharpness_level;                            // [0..7]
    uint8_t             log2_tile_rows;                             // [0..2]
    uint8_t             log2_tile_columns;                          // [0..5]
    uint8_t             UncompressedHeaderLengthInBytes;            // [0..255]
    uint16_t            FirstPartitionSize;                         // [0..65535]
    uint8_t             SegTreeProbs[7];
    uint8_t             SegPredProbs[3];

    uint32_t            BSBytesInBuffer;

    uint32_t            StatusReportFeedbackNumber;

    uint8_t             profile;                        // [0..3]
    uint8_t             BitDepthMinus8;                 // [0, 2, 4]
    uint8_t             subsampling_x;                  // [0..1]
    uint8_t             subsampling_y;                  // [0..1]
} CODEC_VP9_PIC_PARAMS, *PCODEC_VP9_PIC_PARAMS;

typedef struct _CODEC_VP9_SEGMENT_PARAMS
{
    CODEC_VP9_SEG_PARAMS        SegData[8];
} CODEC_VP9_SEGMENT_PARAMS, *PCODEC_VP9_SEGMENT_PARAMS;

#endif
