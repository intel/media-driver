/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file      codechal_common_avc.h
//! \brief     This modules includes the common definitions for AVC codec, across CODECHAL components.
//!

#ifndef __CODECHAL_COMMON_AVC_H__
#define __CODECHAL_COMMON_AVC_H__

#include "codechal.h"

#define CODECHAL_AVC_MAX_SPS_NUM                32
#define CODECHAL_AVC_MAX_PPS_NUM                255

#define CODECHAL_AVC_NUM_UNCOMPRESSED_SURFACE   128 // 7 bits

const uint8_t CODECHAL_AVC_Qmatrix_scan_4x4[16] =
{
    0, 1, 4, 8, 5, 2, 3, 6, 9, 12, 13, 10, 7, 11, 14, 15
};

const uint8_t CODECHAL_AVC_Qmatrix_scan_8x8[64] =
{
    0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5,
    12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28,
    35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
    58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

const uint8_t CODECHAL_AVC_Default_4x4_Intra[16] =
{
    6, 13, 13, 20, 20, 20, 28, 28, 28, 28, 32, 32, 32, 37, 37, 42
};

const uint8_t CODECHAL_AVC_Default_4x4_Inter[16] =
{
    10, 14, 14, 20, 20, 20, 24, 24, 24, 24, 27, 27, 27, 30, 30, 34
};

const uint8_t CODECHAL_AVC_Default_8x8_Intra[64] =
{
    6, 10, 10, 13, 11, 13, 16, 16, 16, 16, 18, 18, 18, 18, 18, 23,
    23, 23, 23, 23, 23, 25, 25, 25, 25, 25, 25, 25, 27, 27, 27, 27,
    27, 27, 27, 27, 29, 29, 29, 29, 29, 29, 29, 31, 31, 31, 31, 31,
    31, 33, 33, 33, 33, 33, 36, 36, 36, 36, 38, 38, 38, 40, 40, 42
};

const uint8_t CODECHAL_AVC_Default_8x8_Inter[64] =
{
    9, 13, 13, 15, 13, 15, 17, 17, 17, 17, 19, 19, 19, 19, 19, 21,
    21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22, 22, 24, 24, 24, 24,
    24, 24, 24, 24, 25, 25, 25, 25, 25, 25, 25, 27, 27, 27, 27, 27,
    27, 28, 28, 28, 28, 28, 30, 30, 30, 30, 32, 32, 32, 33, 33, 35
};

typedef enum
{
    CODECHAL_AVC_BASE_PROFILE               = 66,
    CODECHAL_AVC_MAIN_PROFILE               = 77,
    CODECHAL_AVC_EXTENDED_PROFILE           = 88,
    CODECHAL_AVC_HIGH_PROFILE               = 100,
    CODECHAL_AVC_HIGH10_PROFILE             = 110,
    CODECHAL_AVC_HIGH422_PROFILE            = 122,
    CODECHAL_AVC_HIGH444_PROFILE            = 244,
    CODECHAL_AVC_CAVLC444_INTRA_PROFILE     = 44,
    CODECHAL_AVC_SCALABLE_BASE_PROFILE      = 83,
    CODECHAL_AVC_SCALABLE_HIGH_PROFILE      = 86
} CODECHAL_AVC_PROFILE_IDC;

typedef enum
{
    CODECHAL_AVC_LEVEL_1                    = 10,
    CODECHAL_AVC_LEVEL_11                   = 11,
    CODECHAL_AVC_LEVEL_12                   = 12,
    CODECHAL_AVC_LEVEL_13                   = 13,
    CODECHAL_AVC_LEVEL_2                    = 20,
    CODECHAL_AVC_LEVEL_21                   = 21,
    CODECHAL_AVC_LEVEL_22                   = 22,
    CODECHAL_AVC_LEVEL_3                    = 30,
    CODECHAL_AVC_LEVEL_31                   = 31,
    CODECHAL_AVC_LEVEL_32                   = 32,
    CODECHAL_AVC_LEVEL_4                    = 40,
    CODECHAL_AVC_LEVEL_41                   = 41,
    CODECHAL_AVC_LEVEL_42                   = 42,
    CODECHAL_AVC_LEVEL_5                    = 50,
    CODECHAL_AVC_LEVEL_51                   = 51,
    CODECHAL_AVC_LEVEL_52                   = 52
} CODECHAL_AVC_LEVEL_IDC;

// H.264 Inverse Quantization Matrix Buffer
typedef struct _CODECHAL_AVC_IQ_MATRIX_PARAMS
{
    uint8_t         ScalingList4x4[6][16];
    uint8_t         ScalingList8x8[2][64];
} CODECHAL_AVC_IQ_MATRIX_PARAMS, *PCODECHAL_AVC_IQ_MATRIX_PARAMS;

typedef struct _CODECHAL_AVC_FRAME_STORE_ID
{
    bool    bInUse;
    bool    bReUse;
} CODECHAL_AVC_FRAME_STORE_ID, *PCODECHAL_AVC_FRAME_STORE_ID;;

//!
//! \brief    Set frame store Id for avc codec.
//! \details
//! \param    avcFrameStoreID
//!           [in,out] avc frame store id
//! \param    avcRefList
//!           [in,out] avc reference list
//! \param    mode
//!           [in] codechal mode
//! \return   frameIdx
//!           [in] frame index
//! \return   void
//!
void CodecHalAvc_SetFrameStoreIds(
    PCODECHAL_AVC_FRAME_STORE_ID    avcFrameStoreID,
    PCODEC_REF_LIST                *avcRefList,
    uint32_t                        mode,
    uint8_t                         frameIdx);

#endif  // __CODECHAL_COMMON_AVC_H__
