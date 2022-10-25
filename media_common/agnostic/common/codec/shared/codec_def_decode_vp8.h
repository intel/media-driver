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
//! \file     codec_def_decode_vp8.h
//! \brief    Defines decode VP8 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to VP8 decode only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_DECODE_VP8_H__
#define __CODEC_DEF_DECODE_VP8_H__

#include "codec_def_common.h"

#define CODEC_VP8_MVP_COUNT                     19
#define CODEC_VP8_MAX_PARTITION_NUMBER          9

// VP8 Picture Parameters
typedef struct _CODEC_VP8_PIC_PARAMS
{
    CODEC_PICTURE       CurrPic;
    uint16_t            wFrameWidthInMbsMinus1;
    uint16_t            wFrameHeightInMbsMinus1;
    uint32_t            uiFirstPartitionSize;
    uint8_t             ucCurrPicIndex;
    uint8_t             ucLastRefPicIndex;
    uint8_t             ucGoldenRefPicIndex;
    uint8_t             ucAltRefPicIndex;
    uint8_t             ucDeblockedPicIndex;
    uint8_t             ucReserved8Bits;

    union
    {
        uint16_t              wPicFlags;
        struct
        {
            uint16_t              key_frame                   : 1;
            uint16_t              version                     : 3;
            uint16_t              segmentation_enabled        : 1;
            uint16_t              update_mb_segmentation_map  : 1;
            uint16_t              update_segment_feature_data : 1;
            uint16_t              mb_segement_abs_delta       : 1;
            uint16_t              filter_type                 : 1;
            uint16_t              sign_bias_golden            : 1;
            uint16_t              sign_bias_alternate         : 1;
            uint16_t              mb_no_coeff_skip            : 1;
            uint16_t              mode_ref_lf_delta_update    : 1;
            uint16_t              CodedCoeffTokenPartition    : 2;
            uint16_t              LoopFilterDisable           : 1;
            uint16_t              loop_filter_adj_enable      : 1;
        };
    };

    uint8_t             ucFilterLevel;
    uint8_t             ucBaseQIndex;
    char                cY1DcDeltaQ;
    char                cY2DcDeltaQ;
    char                cY2AcDeltaQ;
    char                cUVDcDeltaQ;
    char                cUVAcDeltaQ;

    char                cSegmentFeatureData[2][4];
    uint8_t             ucLoopFilterLevel[4];
    char                cRefLfDelta[4];
    char                cModeLfDelta[4];
    uint8_t             ucSharpnessLevel;
    char                cMbSegmentTreeProbs[3];
    uint8_t             ucProbSkipFalse;
    uint8_t             ucProbIntra;
    uint8_t             ucProbLast;
    uint8_t             ucProbGolden;
    uint8_t             ucYModeProbs[4];
    uint8_t             ucUvModeProbs[3];
    uint8_t             ucReserved8Bits1;
    uint8_t             ucMvUpdateProb[2][CODEC_VP8_MVP_COUNT];      // 2 for decoding row and column components
    uint8_t             ucP0EntropyCount;
    uint8_t             ucP0EntropyValue;
    uint32_t            uiP0EntropyRange;
    uint32_t            uiFirstMbByteOffset;
    uint32_t            uiPartitionSize[CODEC_VP8_MAX_PARTITION_NUMBER];

    uint32_t            uiStatusReportFeedbackNumber;
} CODEC_VP8_PIC_PARAMS, *PCODEC_VP8_PIC_PARAMS;

// VP8 Slice Parameters
typedef struct _CODEC_VP8_SLICE_PARAMS {
    uint32_t                BSNALunitDataLocation;
    uint32_t                SliceBytesInBuffer;
} CODEC_VP8_SLICE_PARAMS, *PCODEC_VP8_SLICE_PARAMS;

// VP8 Inverse Quantization Matrix Buffer
typedef struct _CODEC_VP8_IQ_MATRIX_PARAMS
{
    uint16_t              quantization_values[4][6];
} CODEC_VP8_IQ_MATRIX_PARAMS, *PCODEC_VP8_IQ_MATRIX_PARAMS;

#endif

