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
//! \file     codec_def_encode_vp8.h
//! \brief    Defines encode VP8 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to VP8 encode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_ENCODE_VP8_H__
#define __CODEC_DEF_ENCODE_VP8_H__

#define ENCODE_VP8_BRC_HISTORY_BUFFER_SIZE      704
#define ENCODE_VP8_BRC_PAK_QP_TABLE_SIZE        2880
#define VP8_QINDEX_Y1_DC                        0
#define VP8_QINDEX_UV_DC                        1
#define VP8_QINDEX_UV_AC                        2
#define VP8_QINDEX_Y2_DC                        3
#define VP8_QINDEX_Y2_AC                        4

#define ENCODE_VP8_NUM_MAX_L0_REF     3    // used in LibVa

#define CODECHAL_VP8_MAX_QP                         128
#define CODECHAL_VP8_MB_CODE_SIZE                   204
#define CODECHAL_VP8_MB_MV_CODE_SIZE                64
#define CODECHAL_VP8_MB_MV_CODE_OFFSET_ALIGNMENT    4096
#define CODECHAL_VP8_MB_CODE_ALIGNMENT              32
#define CODECHAL_VP8_FRAME_HEADER_SIZE              4096
#define CODECHAL_VP8_MAX_SEGMENTS                   4
#define CODECHAL_VP8_ME_ME_DATA_SIZE_MULTIPLIER     3

//VP8 Sequence Params
typedef struct _CODEC_VP8_ENCODE_SEQUENCE_PARAMS
{
    uint16_t      FrameWidth         : 14;
    uint16_t      FrameWidthScale    : 2;
    uint16_t      FrameHeight        : 14;
    uint16_t      FrameHeightScale   : 2;

    uint16_t      GopPicSize;

    uint8_t       TargetUsage;
    uint8_t       RateControlMethod;
    uint32_t      TargetBitRate[256]; // One per temporal layer
    uint32_t      MaxBitRate;
    uint32_t      MinBitRate;
    uint32_t      InitVBVBufferFullnessInBit;
    uint32_t      VBVBufferSizeInBit;

    union
    {
        struct
        {
            uint32_t    ResetBRC                    : 1;
            uint32_t    NoFrameHeaderInsertion      : 1;
            uint32_t    UseRawReconRef              : 1;
            uint32_t    MBBRC                       : 4;
            uint32_t    bReserved                   : 25;
        };
        uint32_t    sFlags;
    };

    uint32_t      UserMaxFrameSize;
    uint16_t      AVBRAccuracy;
    uint16_t      AVBRConvergence;
    uint16_t      FramesPer100Sec[256]; // One per temporal layer
    uint8_t       NumTemporalLayersMinus1;
} CODEC_VP8_ENCODE_SEQUENCE_PARAMS, *PCODEC_VP8_ENCODE_SEQUENCE_PARAMS;
//Picture Params
typedef struct _CODEC_VP8_ENCODE_PIC_PARAMS
{

    CODEC_PICTURE       CurrOriginalPic;
    CODEC_PICTURE       CurrReconstructedPic;
    CODEC_PICTURE       LastRefPic;
    CODEC_PICTURE       GoldenRefPic;
    CODEC_PICTURE       AltRefPic;

    union
    {
        uint32_t        uPicFlags;

        struct
        {
            uint32_t    frame_type                  : 1;
            uint32_t    version                     : 3;
            uint32_t    show_frame                  : 1;
            uint32_t    color_space                 : 1;
            uint32_t    clamping_type               : 1;
            uint32_t    segmentation_enabled        : 1;
            uint32_t    update_mb_segmentation_map  : 1;
            uint32_t    update_segment_feature_data : 1;
            uint32_t    filter_type                 : 1;
            uint32_t    loop_filter_adj_enable      : 1;
            uint32_t    CodedCoeffTokenPartition    : 2;
            uint32_t    refresh_golden_frame        : 1;
            uint32_t    refresh_alternate_frame     : 1;
            uint32_t    copy_buffer_to_golden       : 2;
            uint32_t    copy_buffer_to_alternate    : 2;
            uint32_t    sign_bias_golden            : 1;
            uint32_t    sign_bias_alternate         : 1;
            uint32_t    refresh_entropy_probs       : 1;
            uint32_t    refresh_last                : 1;
            uint32_t    mb_no_coeff_skip            : 1;
            uint32_t    forced_lf_adjustment        : 1;
            uint32_t    ref_frame_ctrl              : 3;
            uint32_t                                : 3;
        };
    };

    char                loop_filter_level[4];
    char                ref_lf_delta[4];
    char                mode_lf_delta[4];
    uint8_t             sharpness_level;
    uint32_t            StatusReportFeedbackNumber;
    uint8_t             ClampQindexHigh;
    uint8_t             ClampQindexLow;
    uint8_t             temporal_id;
    uint8_t             first_ref;
    uint8_t             second_ref;

} CODEC_VP8_ENCODE_PIC_PARAMS, *PCODEC_VP8_ENCODE_PIC_PARAMS;

//Quant Data
typedef struct _CODEC_VP8_ENCODE_QUANT_DATA
{
    uint8_t     QIndex[4];
    char        QIndexDelta[5];
} CODEC_VP8_ENCODE_QUANT_DATA, *PCODEC_VP8_ENCODE_QUANT_DATA;

#endif
