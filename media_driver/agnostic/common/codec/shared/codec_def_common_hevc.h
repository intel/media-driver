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
//! \file     codec_def_common_hevc.h
//! \brief    Defines basic HEVC types and macros shared by CodecHal, MHW, and DDI layer
//! \details  This is the base header for all codec_def HEVC files. All codec_def HEVC files should include this file which should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_COMMON_HEVC_H__
#define __CODEC_DEF_COMMON_HEVC_H__

#include "codec_def_common.h"

#define CODEC_MAX_NUM_REF_FRAME_HEVC        15
#define CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC 8
#define CODEC_HEVC_VDENC_LCU_WIDTH          64
#define CODEC_HEVC_VDENC_LCU_HEIGHT         64

/*! \brief Quantization matrix data, which is sent on a per-picture basis.
 *
 *  The quantization matrix buffer is sent only when scaling_list_enabled_flag takes value 1. If 0, driver should assume “flat” scaling lists are present and all the entries takes value 16.
 */
typedef struct _CODECHAL_HEVC_IQ_MATRIX_PARAMS
{
    /*! \brief Scaling lists for the 4x4 scaling process.
     *
     *  Corresponding to ScalingList[ 0 ][ MatrixID ][ i ] in HEVC specification, where MatrixID is in the range of 0 to 5, inclusive, and i is in the range of 0 to 15, inclusive.
     */
    uint8_t               ucScalingLists0[6][16];

    /*! \brief Scaling lists for the 8x8 scaling process.
    *
    *  Corresponding to ScalingList[ 1 ][ MatrixID ][ i ] in the HEVC specification, where MatrixID is in the range of 0 to 5, inclusive, and i is in the range of 0 to 63, inclusive.
    */
    uint8_t               ucScalingLists1[6][64];

    /*! \brief Scaling lists for the 8x8 scaling process.
    *
    *  Corresponding to ScalingList[ 2 ][ MatrixID ][ i ] in HEVC specification, where MatrixID is in the range of 0 to 5, inclusive, and i is in the range of 0 to 63, inclusive.
    */
    uint8_t               ucScalingLists2[6][64];

    /*! \brief Scaling lists for the 8x8 scaling process.
    *
    *   Corresponding to ScalingList[ 3 ][ MatrixID ][ i ] in HEVC specification, where MatrixID is in the range of 0 to 1, inclusive, and i is in the range of 0 to 63, inclusive.
    */
    uint8_t               ucScalingLists3[2][64];

    /*! \brief DC value of the scaling list for 16x16 size.
    *
    *   With sizeID equal to 2 and corresponding to scaling_list_dc_coef_minus8[ sizeID − 2 ][ matrixID ] +8 with sizeID equal to 2 and matrixID in the range of 0 to 5, inclusive, in HEVC specification.
    */
    uint8_t               ucScalingListDCCoefSizeID2[6];

    /*! \brief DC value of the scaling list for 32x32 size.
    *
    *   With sizeID equal to 3, and corresponding to scaling_list_dc_coef_minus8[ sizeID − 2 ][ matrixID ] +8 with sizeID equal to 3 and matrixID in the range of 0 to 1, inclusive, in HEVC specification.
    */
    uint8_t               ucScalingListDCCoefSizeID3[2];
} CODECHAL_HEVC_IQ_MATRIX_PARAMS, *PCODECHAL_HEVC_IQ_MATRIX_PARAMS;

#endif  // __CODEC_DEF_COMMON_HEVC_H__
