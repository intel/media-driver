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
//! \file     codec_def_common_avc.h
//! \brief    Defines basic AVC types and macros shared by CodecHal, MHW, and DDI layer
//! \details  This is the base header for all codec_def AVC files. All codec_def AVC files should include this file which should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_COMMON_AVC_H__
#define __CODEC_DEF_COMMON_AVC_H__

#include "codec_def_common.h"

#define CODEC_AVC_MAX_NUM_REF_FRAME         16
#define CODEC_AVC_NUM_REF_LISTS             2
#define CODEC_AVC_NUM_REF_DMV_BUFFERS       (CODEC_AVC_MAX_NUM_REF_FRAME + 1) // Max 16 references + 1 for the current frame
#define CODEC_AVC_NUM_DMV_BUFFERS           (CODEC_AVC_NUM_REF_DMV_BUFFERS + 1)  // 1 for non-reference
#define CODEC_AVC_NUM_INIT_DMV_BUFFERS      4

#define CODEC_MAX_NUM_REF_FIELD             (CODEC_MAX_NUM_REF_FRAME * CODEC_NUM_FIELDS_PER_FRAME)
#define CODEC_AVC_MAX_SPS_NUM                32
#define CODEC_AVC_MAX_PPS_NUM                255

#define CODEC_AVC_NUM_UNCOMPRESSED_SURFACE   128 // 7 bits

#define CODECHAL_ENCODE_VDENC_BRC_CONST_BUFFER_NUM (NUM_PIC_TYPES + 1)  //!< For each frame type + 1 for ref B

//!
//! \enum     CODEC_AVC_WEIGHT_SCALE_SIZE
//! \brief    Codec AVC weight scale size
//!
enum CODEC_AVC_WEIGHT_SCALE_SIZE
{
    CODEC_AVC_WEIGHT_SCALE_4x4 = 24,
    CODEC_AVC_WEIGHT_SCALE_8x8 = 32,
    CODEC_AVC_WEIGHT_SCALE     = 56
};

typedef enum
{
    CODEC_AVC_BASE_PROFILE               = 66,
    CODEC_AVC_MAIN_PROFILE               = 77,
    CODEC_AVC_EXTENDED_PROFILE           = 88,
    CODEC_AVC_HIGH_PROFILE               = 100,
    CODEC_AVC_HIGH10_PROFILE             = 110,
    CODEC_AVC_HIGH422_PROFILE            = 122,
    CODEC_AVC_HIGH444_PROFILE            = 244,
    CODEC_AVC_CAVLC444_INTRA_PROFILE     = 44,
    CODEC_AVC_SCALABLE_BASE_PROFILE      = 83,
    CODEC_AVC_SCALABLE_HIGH_PROFILE      = 86
} CODEC_AVC_PROFILE_IDC;

typedef enum
{
    CODEC_AVC_LEVEL_1                    = 10,
    CODEC_AVC_LEVEL_1b                   = 9,
    CODEC_AVC_LEVEL_11                   = 11,
    CODEC_AVC_LEVEL_12                   = 12,
    CODEC_AVC_LEVEL_13                   = 13,
    CODEC_AVC_LEVEL_2                    = 20,
    CODEC_AVC_LEVEL_21                   = 21,
    CODEC_AVC_LEVEL_22                   = 22,
    CODEC_AVC_LEVEL_3                    = 30,
    CODEC_AVC_LEVEL_31                   = 31,
    CODEC_AVC_LEVEL_32                   = 32,
    CODEC_AVC_LEVEL_4                    = 40,
    CODEC_AVC_LEVEL_41                   = 41,
    CODEC_AVC_LEVEL_42                   = 42,
    CODEC_AVC_LEVEL_5                    = 50,
    CODEC_AVC_LEVEL_51                   = 51,
    CODEC_AVC_LEVEL_52                   = 52
} CODEC_AVC_LEVEL_IDC;

// H.264 Inverse Quantization Matrix Buffer
typedef struct _CODEC_AVC_IQ_MATRIX_PARAMS
{
    uint8_t         ScalingList4x4[6][16];
    uint8_t         ScalingList8x8[2][64];
} CODEC_AVC_IQ_MATRIX_PARAMS, *PCODEC_AVC_IQ_MATRIX_PARAMS;

typedef struct _CODEC_AVC_FRAME_STORE_ID
{
    bool    inUse;
    bool    reUse;
} CODEC_AVC_FRAME_STORE_ID, *PCODEC_AVC_FRAME_STORE_ID;

const uint8_t CODEC_AVC_Qmatrix_scan_4x4[16] =
{
    0, 1, 4, 8, 5, 2, 3, 6, 9, 12, 13, 10, 7, 11, 14, 15
};

const uint8_t CODEC_AVC_Qmatrix_scan_8x8[64] =
{
    0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

const uint8_t CODEC_AVC_Default_4x4_Intra[16] =
{
    6, 13, 13, 20, 20, 20, 28, 28, 28, 28, 32, 32, 32, 37, 37, 42
};

const uint8_t CODEC_AVC_Default_4x4_Inter[16] =
{
    10, 14, 14, 20, 20, 20, 24, 24, 24, 24, 27, 27, 27, 30, 30, 34
};

const uint8_t CODEC_AVC_Default_8x8_Intra[64] =
{
    6, 10, 10, 13, 11, 13, 16, 16, 16, 16, 18, 18, 18, 18, 18, 23,
    23, 23, 23, 23, 23, 25, 25, 25, 25, 25, 25, 25, 27, 27, 27, 27,
    27, 27, 27, 27, 29, 29, 29, 29, 29, 29, 29, 31, 31, 31, 31, 31,
    31, 33, 33, 33, 33, 33, 36, 36, 36, 36, 38, 38, 38, 40, 40, 42
};

const uint8_t CODEC_AVC_Default_8x8_Inter[64] =
{
    9, 13, 13, 15, 13, 15, 17, 17, 17, 17, 19, 19, 19, 19, 19, 21,
    21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24,
    24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 27, 27, 27, 27, 27,
    27, 28, 28, 28, 28, 28, 30, 30, 30, 30, 32, 32, 32, 33, 33, 35
};
#endif  // __CODEC_DEF_COMMON_AVC_H__
