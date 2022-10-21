/*
* Copyright (c) 2017-2021, Intel Corporation
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
//! \file     codec_def_encode_vp9.h
//! \brief    Defines encode VP9 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to VP9 encode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_ENCODE_VP9_H__
#define __CODEC_DEF_ENCODE_VP9_H__

#include "codec_def_common.h"
#include "codec_def_common_encode.h"
#include "codec_def_common_vp9.h"

#define CODECHAL_ENCODE_VP9_MAX_NAL_UNIT_TYPE       1 // only support one NAL unit for uncompressed header
#define CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS 8
#define ENCODE_VP9_NUM_MAX_L0_REF                   3
#define VP9_MAX_COEFF_PARTITIONS                    4
#define VP9_HYBRIDPAK_PER_MB_DATA_SIZE              816
#define VP9_HYBRIDPAK_PER_MB_MV_DATA_SIZE           64
#define CODECHAL_ENCODE_VP9_FRAME_HEADER_SIZE       4096

typedef enum
{
    VP9_ENCODED_CHROMA_FORMAT_YUV420 = 0,
    VP9_ENCODED_CHROMA_FORMAT_YUV422 = 1,
    VP9_ENCODED_CHROMA_FORMAT_YUV444 = 2
} VP9_ENCODED_CHROMA_FORMAT;

typedef enum
{
    VP9_ENCODED_BIT_DEPTH_8 = 0,
    VP9_ENCODED_BIT_DEPTH_10 = 1,
    VP9_ENCODED_BIT_DEPTH_12 = 2
} VP9_ENCODED_BIT_DEPTH;

typedef struct _CODEC_VP9_ENCODE_SEG_PARAMS
{
    union
    {
        struct
        {
            uint8_t       SegmentReferenceEnabled : 1;
            uint8_t       SegmentReference        : 2;
            uint8_t       SegmentSkipped          : 1;
            uint8_t       ReservedField3          : 4;
        } fields;
        uint8_t           value;

    } SegmentFlags;

    char                SegmentLFLevelDelta;
    int16_t             SegmentQIndexDelta;

} CODEC_VP9_ENCODE_SEG_PARAMS, *PCODEC_VP9_ENCODE_SEG_PARAMS;

typedef struct _CODEC_VP9_ENCODE_SEGMENT_PARAMS
{
    CODEC_VP9_ENCODE_SEG_PARAMS SegData[8];

} CODEC_VP9_ENCODE_SEGMENT_PARAMS, *PCODEC_VP9_ENCODE_SEGMENT_PARAMS;

typedef struct _CODEC_VP9_ENCODE_SEQUENCE_PARAMS
{
    uint16_t       wMaxFrameWidth;
    uint16_t       wMaxFrameHeight;
    uint16_t       GopPicSize;
    uint8_t        TargetUsage;
    uint8_t        RateControlMethod;
    uint32_t       TargetBitRate[CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS];
    uint32_t       MaxBitRate;
    uint32_t       MinBitRate;
    uint32_t       InitVBVBufferFullnessInBit;
    uint32_t       VBVBufferSizeInBit;
    uint32_t       OptimalVBVBufferLevelInBit;
    uint32_t       UpperVBVBufferLevelThresholdInBit;
    uint32_t       LowerVBVBufferLevelThresholdInBit;

    union
    {
        struct
        {
            uint32_t bResetBRC                  : 1;
            uint32_t bNoFrameHeaderInsertion    : 1;
            uint32_t bUseRawReconRef            : 1;
            uint32_t MBBRC                      : 4; // This is not to be set for VP9 VDEnc (G10+), this is removed from DDI, only here to support legacy KBL DP
            uint32_t EnableDynamicScaling       : 1;
            uint32_t SourceFormat               : 2;
            uint32_t SourceBitDepth             : 2;
            uint32_t EncodedFormat              : 2;
            uint32_t EncodedBitDepth            : 2;
            uint32_t DisplayFormatSwizzle       : 1;
            uint32_t bReserved                  : 15;
        } fields;

        uint32_t value;
    } SeqFlags;

    uint32_t     UserMaxFrameSize;
    uint16_t     reserved2;
    uint16_t     reserved3;
    FRAME_RATE   FrameRate[CODECHAL_ENCODE_VP9_MAX_NUM_TEMPORAL_LAYERS];
    uint8_t      NumTemporalLayersMinus1;
    uint8_t      ICQQualityFactor;

    ENCODE_INPUT_COLORSPACE         InputColorSpace;
    ENCODE_SCENARIO                 ScenarioInfo;
    ENCODE_CONTENT                  ContentInfo;
    ENCODE_FRAMESIZE_TOLERANCE      FrameSizeTolerance;

} CODEC_VP9_ENCODE_SEQUENCE_PARAMS, *PCODEC_VP9_ENCODE_SEQUENCE_PARAMS;

typedef struct _CODEC_VP9_ENCODE_PIC_PARAMS
{
    uint16_t            SrcFrameHeightMinus1;
    uint16_t            SrcFrameWidthMinus1;
    uint16_t            DstFrameHeightMinus1;
    uint16_t            DstFrameWidthMinus1;

    CODEC_PICTURE       CurrOriginalPic;
    CODEC_PICTURE       CurrReconstructedPic;
    CODEC_PICTURE       RefFrameList[8];

    union
    {
        struct
        {
            uint32_t    frame_type                      : 1;
            uint32_t    show_frame                      : 1;
            uint32_t    error_resilient_mode            : 1;
            uint32_t    intra_only                      : 1;
            uint32_t    allow_high_precision_mv         : 1;
            uint32_t    mcomp_filter_type               : 3;
            uint32_t    frame_parallel_decoding_mode    : 1;
            uint32_t    segmentation_enabled            : 1;
            uint32_t    segmentation_temporal_update    : 1;
            uint32_t    segmentation_update_map         : 1;
            uint32_t    reset_frame_context             : 2;
            uint32_t    refresh_frame_context           : 1;
            uint32_t    frame_context_idx               : 2;
            uint32_t    LosslessFlag                    : 1;
            uint32_t    comp_prediction_mode            : 2;
            uint32_t    super_frame                     : 1;
            uint32_t    seg_id_block_size               : 2;
            uint32_t    seg_update_data                 : 1;
            uint32_t    reserved                        : 8;
        } fields;

        uint32_t value;
    } PicFlags;

    union
    {
        struct
        {
            uint32_t    LastRefIdx                      : 3;
            uint32_t    LastRefSignBias                 : 1;
            uint32_t    GoldenRefIdx                    : 3;
            uint32_t    GoldenRefSignBias               : 1;
            uint32_t    AltRefIdx                       : 3;
            uint32_t    AltRefSignBias                  : 1;

            uint32_t    ref_frame_ctrl_l0               : 3;
            uint32_t    ref_frame_ctrl_l1               : 3;

            uint32_t    refresh_frame_flags             : 8;
            uint32_t    reserved2                       : 6;
        } fields;

        uint32_t value;
    } RefFlags;

    uint8_t         LumaACQIndex;
    char            LumaDCQIndexDelta;
    char            ChromaACQIndexDelta;
    char            ChromaDCQIndexDelta;

    uint8_t         filter_level;    // This is not to be set for VP9 VDEnc (G10+), this is removed from DDI, only here to support legacy KBL DP
    uint8_t         sharpness_level; // This is not to be set for VP9 VDEnc (G10+), this is removed from DDI, only here to support legacy KBL DP

    char            LFRefDelta[4];   // This is not to be set for VP9 VDEnc (G10+), this is removed from DDI, only here to support legacy KBL DP
    char            LFModeDelta[2];  // This is not to be set for VP9 VDEnc (G10+), this is removed from DDI, only here to support legacy KBL DP

    uint16_t        BitOffsetForLFRefDelta;
    uint16_t        BitOffsetForLFModeDelta;
    uint16_t        BitOffsetForLFLevel;
    uint16_t        BitOffsetForQIndex;
    uint16_t        BitOffsetForFirstPartitionSize;
    uint16_t        BitOffsetForSegmentation;
    uint16_t        BitSizeForSegmentation;

    uint8_t         log2_tile_rows;
    uint8_t         log2_tile_columns;

    uint8_t         temporal_id;

    uint32_t        StatusReportFeedbackNumber;

    // Skip Frames
    uint8_t         SkipFrameFlag;    // [0..2]
    uint8_t         NumSkipFrames;
    uint32_t        SizeSkipFrames;
} CODEC_VP9_ENCODE_PIC_PARAMS, *PCODEC_VP9_ENCODE_PIC_PARAMS;
#endif
