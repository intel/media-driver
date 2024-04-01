/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     codechal_vdenc_avc_xe_hpm.cpp
//! \brief    This file implements the C++ class/interface for Xe_HPM platform's AVC
//!           VDEnc encoding to be used CODECHAL components.
//!

#include "codechal_vdenc_avc_xe_hpm.h"
#include "codechal_mmc_encode_avc_xe_hpm.h"
#include "mos_solo_generic.h"
#include "mhw_mmio_g12.h"
#include "mhw_mi_g12_X.h"
#include "codechal_hw_g12_X.h"

static const uint32_t TrellisQuantizationRoundingXe_Hpm[NUM_VDENC_TARGET_USAGE_MODES] =
    {
        0, 3, 3, 3, 3, 3, 3, 3};

static const bool TrellisQuantizationEnableXe_Hpm[NUM_VDENC_TARGET_USAGE_MODES] =
    {
        0, 1, 1, 0, 0, 0, 0, 0};

const uint16_t CodechalVdencAvcStateXe_Hpm::SliceSizeThrsholdsP_Xe_Hpm[52] =  // slice size threshold delta for P frame targeted for 99% compliance
{
    850, 850, 850, 850, 850, 850, 850, 850, 850, 850,  //[ 0- 9]
    525, 525, 325, 325, 325, 325, 325, 325, 325, 325,  //[10-19]
    250, 250, 250, 250, 250, 250, 250, 250, 250, 250,  //[20-29]
    250, 250, 250, 250, 250, 125, 125, 125, 125, 125,  //[30-39]
    125, 125, 125, 125, 125, 125, 125, 125, 125, 125,  //[40-49]
    125, 125  //[50-51]
};

const uint16_t CodechalVdencAvcStateXe_Hpm::SliceSizeThrsholdsI_Xe_Hpm[52] =  // slice size threshold delta for I frame targeted for 99% compliance
{
    850, 850, 850, 850, 850, 850, 850, 850, 850, 850,  //[ 0- 9]
    525, 525, 325, 325, 325, 325, 325, 325, 325, 325,  //[10-19]
    250, 250, 250, 250, 250, 250, 250, 250, 250, 250,  //[20-29]
    250, 250, 250, 250, 250, 125, 125, 125, 125, 125,  //[30-39]
    125, 125, 125, 125, 125, 125, 125, 125, 125, 125,  //[40-49]
    125, 125  //[50-51]
};

const uint8_t CodechalVdencAvcStateXe_Hpm::G0_P_InterRounding[] =
{
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  //QP=[0~12]
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  //QP=[13~25]
    3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2,  //QP=[26~38]
    1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0   //QP=[39~51]
};

const uint8_t CodechalVdencAvcStateXe_Hpm::G0_P_IntraRounding[] =
{
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //QP=[0~12]
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //QP=[13~25]
    4, 4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3,  //QP=[26~38]
    2, 2, 2, 2, 1, 1, 1, 1, 0, 0, 0, 0, 0   //QP=[39~51]
};

const uint8_t CodechalVdencAvcStateXe_Hpm::G3_P_InterRounding[] =
{
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //QP=[0~12]
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 3,  //QP=[13~25]
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  //QP=[26~38]
    3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2   //QP=[39~51]
};

const uint8_t CodechalVdencAvcStateXe_Hpm::G3_P_IntraRounding[] =
{
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //QP=[0~12]
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //QP=[13~25]
    4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3,  //QP=[26~38]
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3   //QP=[39~51]
};

const uint8_t CodechalVdencAvcStateXe_Hpm::G3_rB_InterRounding[] =
{
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  //QP=[0~12]
    3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 2, 2,  //QP=[13~25]
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //QP=[26~38]
    1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0   //QP=[39~51]
};

const uint8_t CodechalVdencAvcStateXe_Hpm::G3_rB_IntraRounding[] =
{
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //QP=[0~12]
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //QP=[13~25]
    4, 4, 4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3,  //QP=[26~38]
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3   //QP=[39~51]
};

const uint8_t CodechalVdencAvcStateXe_Hpm::G3_B_InterRounding[] =
{
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  //QP=[0~12]
    3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 1, 1, 1,  //QP=[13~25]
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //QP=[26~38]
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0   //QP=[39~51]
};

const uint8_t CodechalVdencAvcStateXe_Hpm::G3_B_IntraRounding[] =
{
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //QP=[0~12]
    4, 4, 4, 4, 4, 4, 3, 3, 3, 3, 3, 3, 3,  //QP=[13~25]
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  //QP=[26~38]
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2   //QP=[39~51]
};

static const uint8_t tableHucConstData[5][630] = {
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  31,  31,  31,  31,  31,  31,  31,  31,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  31,  42,  45,  45,  45,  45,  45,  45,  45,  45,
     28,  28,  28,  32,  36,  44,  48,  26,  30,  32,  36,  26,  30,  30,  24,  28,  24,  24,  20,  20,  22,
     20,  20,  20,  18,  18,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  16,  14,  14,  14,  14,  14,
      2,   0,   2,   0,   2,   0,   3,   0,   4,   0,   5,   0,   6,   0,   8,   0,  10,   0,  13,   0,  16,
      0,  20,   0,  26,   0,  33,   0,  41,   0,  52,   0,  66,   0,  83,   0, 104,   0, 132,   0, 166,   0,
    209,   0,   8,   1,  76,   1, 163,   1,  16,   2, 153,   2,  70,   3,  32,   4,  51,   5, 141,   6,  65,
      8, 102,  10,  26,  13, 130,  16, 204,  20,  52,  26,   4,  33, 153,  41, 105,  52,   9,  66,  51,  83,
      2,   0,   2,   0,   2,   0,   2,   0,   3,   0,   3,   0,   4,   0,   4,   0,   5,   0,   5,   0,   6,
      0,   7,   0,   8,   0,   9,   0,  10,   0,  11,   0,  13,   0,  14,   0,  16,   0,  18,   0,  20,   0,
     23,   0,  26,   0,  29,   0,  33,   0,  37,   0,  41,   0,  46,   0,  52,   0,  58,   0,  66,   0,  74,
      0,  83,   0,  93,   0, 104,   0, 117,   0, 132,   0, 148,   0, 166,   0, 186,   0, 209,   0, 235,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  31,  31,  31,  31,  31,  31,  31,  31,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  31,  42,  45,  45,  45,  45,  45,  45,  45,  45,
     12,  12,  12,  14,  16,  18,  20,  10,  12,  14,  16,  12,  14,  14,  14,  14,  12,  14,  14,  12,  14,
     14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      2,   0,   2,   0,   2,   0,   2,   0,   3,   0,   4,   0,   5,   0,   6,   0,   8,   0,  10,   0,  12,
      0,  16,   0,  20,   0,  25,   0,  32,   0,  40,   0,  50,   0,  64,   0,  80,   0, 101,   0, 128,   0,
    161,   0, 203,   0,   0,   1,  66,   1, 150,   1,   0,   2, 133,   2,  44,   3,   0,   4,  10,   5,  89,
      6,   0,   8,  20,  10, 178,  12,   0,  16,  40,  20, 101,  25,   0,  32,  81,  40, 203,  50,   0,  64,
      3,   0,   3,   0,   3,   0,   3,   0,   3,   0,   4,   0,   4,   0,   5,   0,   6,   0,   6,   0,   7,
      0,   8,   0,   9,   0,  10,   0,  12,   0,  13,   0,  15,   0,  16,   0,  19,   0,  21,   0,  24,   0,
     26,   0,  30,   0,  33,   0,  38,   0,  42,   0,  48,   0,  53,   0,  60,   0,  67,   0,  76,   0,  85,
      0,  96,   0, 107,   0, 120,   0, 135,   0, 152,   0, 171,   0, 192,   0, 215,   0, 241,   0,  15,   1,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   2,   2,   2,   2,   2,   2,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  31,  31,  31,  31,  31,  31,  31,  31,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  31,  42,  45,  45,  45,  45,  45,  45,  45,  45,
     12,  12,  12,  14,  16,  18,  20,  10,  12,  14,  16,  12,  14,  14,  14,  14,  12,  14,  14,  12,  14,
     14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      3,   0,   3,   0,   3,   0,   4,   0,   5,   0,   6,   0,   8,   0,  10,   0,  12,   0,  16,   0,  20,
      0,  25,   0,  32,   0,  40,   0,  51,   0,  64,   0,  81,   0, 102,   0, 129,   0, 162,   0, 204,   0,
      2,   1,  69,   1, 153,   1,   4,   2, 138,   2,  51,   3,   8,   4,  20,   5, 102,   6,  16,   8,  40,
     10, 204,  12,  32,  16,  81,  20, 153,  25,  65,  32, 163,  40,  51,  51, 130,  64,  70,  81, 102, 102,
      3,   0,   3,   0,   3,   0,   4,   0,   4,   0,   5,   0,   5,   0,   6,   0,   7,   0,   8,   0,   9,
      0,  10,   0,  11,   0,  12,   0,  14,   0,  16,   0,  18,   0,  20,   0,  22,   0,  25,   0,  28,   0,
     32,   0,  36,   0,  40,   0,  45,   0,  51,   0,  57,   0,  64,   0,  72,   0,  81,   0,  91,   0, 102,
      0, 115,   0, 129,   0, 145,   0, 162,   0, 182,   0, 205,   0, 230,   0,   2,   1,  34,   1,  69,   1,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      2,   2,   2,   2,   2,   2,   2,   2,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   4,   3,   3,   3,   3,   2,   2,   2,   2,   1,   1,   1,   1,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  31,  31,  31,  31,  31,  31,  31,  31,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  31,  42,  45,  45,  45,  45,  45,  45,  45,  45,
     14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
     14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      4,   0,   4,   0,   4,   0,   6,   0,   7,   0,   9,   0,  12,   0,  15,   0,  19,   0,  24,   0,  30,
      0,  38,   0,  48,   0,  60,   0,  76,   0,  96,   0, 121,   0, 153,   0, 193,   0, 243,   0,  51,   1,
    131,   1, 231,   1, 102,   2,   6,   3, 207,   3, 204,   4,  12,   6, 158,   7, 153,   9,  24,  12,  61,
     15,  51,  19,  48,  24, 122,  30, 102,  38,  97,  48, 244,  60, 204,  76, 195,  96, 233, 121, 153, 153,
      4,   0,   4,   0,   4,   0,   4,   0,   5,   0,   6,   0,   6,   0,   7,   0,   8,   0,   9,   0,  11,
      0,  12,   0,  13,   0,  15,   0,  17,   0,  19,   0,  22,   0,  24,   0,  27,   0,  31,   0,  35,   0,
     39,   0,  44,   0,  49,   0,  55,   0,  62,   0,  70,   0,  79,   0,  88,   0,  99,   0, 111,   0, 125,
      0, 140,   0, 158,   0, 177,   0, 199,   0, 223,   0, 250,   0,  25,   1,  60,   1,  98,   1, 142,   1,
      3,   3,   3,   3,   3,   3,   3,   3,   3,   2,   2,   2,   2,   1,   1,   1,   1,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,
      3,   3,   3,   3,   3,   3,   3,   3,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
      5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,   5,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,  10,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  31,  31,  31,  31,  31,  31,  31,  31,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,
     26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  26,  31,  42,  45,  45,  45,  45,  45,  45,  45,  45,
     14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
     14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,  14,
      3,   0,   3,   0,   3,   0,   4,   0,   5,   0,   7,   0,   9,   0,  11,   0,  14,   0,  18,   0,  22,
      0,  28,   0,  36,   0,  45,   0,  57,   0,  72,   0,  91,   0, 115,   0, 145,   0, 182,   0, 230,   0,
     34,   1, 109,   1, 204,   1,  68,   2, 219,   2, 153,   3, 137,   4, 182,   5,  51,   7,  18,   9, 109,
     11, 102,  14,  36,  18, 219,  22, 204,  28,  73,  36, 183,  45, 153,  57, 146,  72, 111,  91,  51, 115,
      3,   0,   3,   0,   3,   0,   4,   0,   4,   0,   5,   0,   5,   0,   6,   0,   7,   0,   8,   0,   9,
      0,  10,   0,  11,   0,  12,   0,  14,   0,  16,   0,  18,   0,  20,   0,  22,   0,  25,   0,  28,   0,
     32,   0,  36,   0,  40,   0,  45,   0,  51,   0,  57,   0,  64,   0,  72,   0,  81,   0,  91,   0, 102,
      0, 115,   0, 129,   0, 145,   0, 162,   0, 182,   0, 205,   0, 230,   0,   2,   1,  34,   1,  69,   1,
      3,   3,   3,   3,   3,   3,   3,   3,   2,   2,   2,   2,   2,   2,   2,   2,   1,   1,   1,   1,   1,
      1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   0,   0,   0,   0,
      4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,   4,
      4,   4,   4,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3,   3
};

MOS_STATUS CodechalVdencAvcStateXe_Hpm::Initialize(CodechalSetting *settings)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcStateG12::Initialize(settings));

    m_staticFrameDetectionEnable = false;

    // for brc 2nd level BB usage
    m_mfxAvcImgStateSize    = m_mfxInterface->GetAvcImgStateSize();
    m_vdencCmd3Size         = m_vdencInterface->GetVdencCmd3Size();
    m_vdencAvcImgStateSize  = m_vdencInterface->GetVdencAvcImgStateSize();
    m_mfxAvcSlcStateSize    = m_mfxInterface->GetAvcSlcStateSize();
    m_vdencAvcSlcStateSize  = m_vdencInterface->GetVdencAvcSlcStateSize();
    m_miBatchBufferEndSize  = m_miInterface->GetMiBatchBufferEndCmdSize();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcStateG12::InitializeState());
    // disable HME after reg key reading, but before HME allocation
    m_hmeSupported = false;  // Xe_HPM don't support HME

    // disable CSC since only VDEnc natively supported formats are permitted
    if (m_cscDsState)
    {
        m_cscDsState->DisableCsc();
        m_cscDsState->EnableCopy();
        m_cscDsState->EnableMediaCopy();
    }

    return eStatus;
}

CodechalVdencAvcStateXe_Hpm::~CodechalVdencAvcStateXe_Hpm()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(m_osInterface, &m_hucAuthBuf);

    for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
    {
        MOS_STATUS eStatus = Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_2ndLevelBB[j], nullptr);
        ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    }
}

uint16_t CodechalVdencAvcStateXe_Hpm::GetAdaptiveRoundingNumSlices()
{
    return static_cast<uint16_t>(m_numSlices);
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::InitMmcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeAvcXe_Hpm, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::DeltaQPUpdate(uint8_t QpModulationStrength)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    uint8_t QpStrength = (uint8_t)(QpModulationStrength + (QpModulationStrength >> 1));
    if (!m_isFirstDeltaQP)
    {
        if (QpModulationStrength == 0)
        {
            m_qpModulationStrength = 0;
        }
        else
        {
            m_qpModulationStrength = (m_qpModulationStrength + QpStrength + 1) >> 1;
        }
    }
    else
    {
        m_qpModulationStrength = QpStrength;
        if (m_currPass == m_numPasses)
        {
            m_isFirstDeltaQP = false;
        }
    }

    return MOS_STATUS_SUCCESS;
}

void CodechalVdencAvcStateXe_Hpm::MotionEstimationDisableCheck()
{
    m_16xMeSupported = false;
    m_32xMeSupported = false;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::AllocateResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcStateG12::AllocateResources());

    if (!MEDIA_IS_WA(m_waTable, WaEnableOnlyASteppingFeatures))
    {
        // VDEnc colocated MV buffer size
        m_vdencMvTemporalBufferSize = (((uint32_t)m_picHeightInMb * m_picWidthInMb + 1) >> 1) * CODECHAL_CACHELINE_SIZE;

        // Allocate and initialize pre-generated colocated MV buffer for I frame
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_trackedBuf->AllocateMvTemporalBuffer(CODEC_NUM_REF_BUFFERS));

        PMOS_RESOURCE iBuf = m_trackedBuf->GetMvTemporalBuffer(CODEC_NUM_REF_BUFFERS);
        CODECHAL_ENCODE_CHK_NULL_RETURN(iBuf);

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        uint8_t *pData = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, iBuf, &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pData);
        uint32_t *pT = (uint32_t *)(pData) + 7;
        for (uint32_t j = 0; j < m_picHeightInMb; j++)
        {
            for (uint32_t i = 0; i < m_picWidthInMb; i++)
            {
                *pT = 0x4000;
                pT += 8;
            }
        }

        m_osInterface->pfnUnlockResource(m_osInterface, iBuf);
    }

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    // HUC STATUS 2 Buffer for HuC status check in COND_BB_END
    allocParamsForBufferLinear.dwBytes = sizeof(uint64_t);
    allocParamsForBufferLinear.pBufName = "Huc authentication status Buffer";
    allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_hucAuthBuf);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Huc authentication status Buffer.");
        return eStatus;
    }

    for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
    {
        // second level batch buffer
        MOS_ZeroMemory(&m_2ndLevelBB[j], sizeof(MHW_BATCH_BUFFER));
        m_2ndLevelBB[j].bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_hwInterface->GetOsInterface(),
            &m_2ndLevelBB[j],
            nullptr,
            CODECHAL_CACHELINE_SIZE));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::AllocateMDFResources()
{
    //There is no VME on Xe_HPM and there is no needed to allocate MDF resource.
    return MOS_STATUS_SUCCESS;
}

PMHW_VDBOX_AVC_IMG_PARAMS CodechalVdencAvcStateXe_Hpm::CreateMhwVdboxAvcImgParams()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    PMHW_VDBOX_AVC_IMG_PARAMS avcImgParams = MOS_New(MHW_VDBOX_AVC_IMG_PARAMS_XE_XPM);

    return avcImgParams;
}

void CodechalVdencAvcStateXe_Hpm::SetMfxAvcImgStateParams(MHW_VDBOX_AVC_IMG_PARAMS& param)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CodechalVdencAvcStateG12::SetMfxAvcImgStateParams(param);
    param.biWeight        = m_biWeight;

    auto paramsXe_Xpm = static_cast<PMHW_VDBOX_AVC_IMG_PARAMS_XE_XPM>(&param);

    if (!MEDIA_IS_WA(m_waTable, WaEnableOnlyASteppingFeatures)) {
        if (m_currRefList && m_currRefList->bUsedAsRef && m_pictureCodingType != I_TYPE)
            paramsXe_Xpm->colMVWriteEnable = true;

        if (m_pictureCodingType == B_TYPE) {
            auto TopRefL1 = m_avcSliceParams->RefPicList[LIST_1][0];
            if (!CodecHal_PictureIsInvalid(TopRefL1) && m_picIdx[TopRefL1.FrameIdx].bValid)
                paramsXe_Xpm->colMVReadEnable = true;
        }
        paramsXe_Xpm->tuSettingsRevision = 1;
    }
}

void CodechalVdencAvcStateXe_Hpm::SetMfxPipeModeSelectParams(
    const CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS &genericParam,
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &param)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CodechalVdencAvcStateG12::SetMfxPipeModeSelectParams(genericParam, param);

    auto avcPicParams = m_avcPicParams[m_avcSliceParams->pic_parameter_set_id];
    auto avcSeqParams = m_avcSeqParams[avcPicParams->seq_parameter_set_id];
    auto paramGen12   = ((MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 *)&param);

    paramGen12->bIsRandomAccess = (avcPicParams->CodingType == B_TYPE);
    paramGen12->bBRCEnabled     = m_vdencBrcEnabled;

    if (!MEDIA_IS_WA(m_waTable, WaEnableOnlyASteppingFeatures)) {
        paramGen12->tuSettingsRevision = 1;
        paramGen12->tuMinus1 = avcSeqParams->TargetUsage - 1;
        paramGen12->ucQuantizationPrecision = 1;
    }
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::SetMfxPipeBufAddrStateParams(
    CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS genericParam,
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& param)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CodechalVdencAvcState::SetMfxPipeBufAddrStateParams(genericParam, param);

    auto l1RefFrameList = m_avcSliceParams->RefPicList[LIST_1];
    auto l0RefNum = m_avcSliceParams->num_ref_idx_l0_active_minus1 + 1;
    for (uint8_t refIdx = 0; refIdx <= m_avcSliceParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        auto refPic = l1RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
        {
            // L1 references
            auto refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
            param.presVdencReferences[l0RefNum + refIdx] = &m_refList[refPicIdx]->sRefReconBuffer.OsResource;
            param.presVdenc4xDsSurface[l0RefNum + refIdx] =
                &(m_trackedBuf->Get4xDsReconSurface(m_refList[refPicIdx]->ucScalingIdx))->OsResource;
        }
    }

    if (m_currRefList && m_currRefList->bUsedAsRef) {
        m_currRefList->bIsIntra = (m_pictureCodingType == I_TYPE);
        param.presVdencColocatedMVWriteBuffer = (m_currRefList->bIsIntra) ? nullptr : m_trackedBuf->GetMvTemporalBuffer(CODEC_CURR_TRACKED_BUFFER);
    } else {
        param.presVdencColocatedMVWriteBuffer = nullptr;
    }

    if (m_pictureCodingType == B_TYPE) {
        auto TopRefL1 = l1RefFrameList[0];
        if (!CodecHal_PictureIsInvalid(TopRefL1) && m_picIdx[TopRefL1.FrameIdx].bValid) {
            auto refList = m_refList[m_picIdx[TopRefL1.FrameIdx].ucPicIdx];
            param.presVdencColocatedMVReadBuffer = m_trackedBuf->GetMvTemporalBuffer((refList->bIsIntra) ? CODEC_NUM_REF_BUFFERS : refList->ucScalingIdx);
        } else
            param.presVdencColocatedMVReadBuffer = nullptr;
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceState(param.pDecodedReconParam));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceState(param.pRawSurfParam));
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::GetTrellisQuantization(PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params, PCODECHAL_ENCODE_AVC_TQ_PARAMS trellisQuantParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(trellisQuantParams);

    trellisQuantParams->dwTqEnabled  = TrellisQuantizationEnableXe_Hpm[params->ucTargetUsage];
    trellisQuantParams->dwTqRounding = trellisQuantParams->dwTqEnabled ? TrellisQuantizationRoundingXe_Hpm[params->ucTargetUsage] : 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::LoadHmeMvCostTable(PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams, uint8_t hmeMvCostTable[8][42])
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::FillHucConstData(uint8_t *data, uint8_t picType)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcState::FillHucConstData(data, picType));

    auto codingType = picType + 1;
    auto type = codingType == I_TYPE ? 0 :
                codingType == P_TYPE ? (m_avcSeqParam->GopRefDist == 1 ? 2 : 1) :
                codingType == B_TYPE ? 3 : 4;

    auto hucConstData = (PAVCVdencBRCCostantDataXe_Hpm)data;
    MOS_SecureMemcpy(hucConstData->Reserved, sizeof(hucConstData->Reserved), tableHucConstData[type], 630);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::SetRounding(PCODECHAL_ENCODE_AVC_ROUNDING_PARAMS param, PMHW_VDBOX_AVC_SLICE_STATE sliceState)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState->pEncodeAvcSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState->pEncodeAvcPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState->pEncodeAvcSliceParams);

    auto    avcSeqParams   = sliceState->pEncodeAvcSeqParams;
    auto    avcPicParams   = sliceState->pEncodeAvcPicParams;
    auto    avcSliceParams = sliceState->pEncodeAvcSliceParams;
    uint8_t sliceQP        = avcPicParams->pic_init_qp_minus26 + 26 + avcSliceParams->slice_qp_delta;

    sliceState->dwRoundingIntraValue = 5;
    sliceState->bRoundingInterEnable = m_roundingInterEnable;

    switch (Slice_Type[avcSliceParams->slice_type])
    {
    case SLICE_P:
        if (m_roundingInterP == CODECHAL_ENCODE_AVC_INVALID_ROUNDING)
        {
            if (m_adaptiveRoundingInterEnable && !m_vdencBrcEnabled)
            {
                if (avcSeqParams->GopRefDist == 1)
                {
                    sliceState->dwRoundingIntraValue = G0_P_IntraRounding[sliceQP];
                    sliceState->dwRoundingValue      = G0_P_InterRounding[sliceQP];
                }
                else
                {
                    sliceState->dwRoundingIntraValue = G3_P_IntraRounding[sliceQP];
                    sliceState->dwRoundingValue      = G3_P_InterRounding[sliceQP];
                }
            }
            else
                sliceState->dwRoundingValue = CodechalVdencAvcState::InterRoundingP[avcSeqParams->TargetUsage];
        }
        else
            sliceState->dwRoundingValue = m_roundingInterP;

        break;
    case SLICE_B:
        if (m_adaptiveRoundingInterEnable && !m_vdencBrcEnabled)
        {
            if (m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef)
            {
                sliceState->dwRoundingIntraValue = G3_rB_IntraRounding[sliceQP];
                sliceState->dwRoundingValue      = G3_rB_InterRounding[sliceQP];
            }
            else
            {
                sliceState->dwRoundingIntraValue = G3_B_IntraRounding[sliceQP];
                sliceState->dwRoundingValue      = G3_B_InterRounding[sliceQP];
            }
        }
        else
        {
            if (m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef)
                sliceState->dwRoundingValue = InterRoundingBRef[avcSeqParams->TargetUsage];
            else
                sliceState->dwRoundingValue = InterRoundingB[avcSeqParams->TargetUsage];
        }
        break;
    default:
        // do nothing
        break;
    }

    if (param != nullptr && param->bEnableCustomRoudingIntra)
    {
        sliceState->dwRoundingIntraValue = param->dwRoundingIntra;
    }

    if (param != nullptr && param->bEnableCustomRoudingInter)
    {
        sliceState->bRoundingInterEnable = true;
        sliceState->dwRoundingValue      = param->dwRoundingInter;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::SetupWalkerContext(
        MOS_COMMAND_BUFFER* cmdBuffer,
        SendKernelCmdsParams* params)
{
    //There is no VME on Xe_HPM

    return MOS_STATUS_SUCCESS;
}

void CodechalVdencAvcStateXe_Hpm::CopyMBQPDataToStreamIn(CODECHAL_VDENC_STREAMIN_STATE* pData, uint8_t* pInputData)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    for (uint32_t curY = 0; curY < m_picHeightInMb; curY++)
    {
        for (uint32_t curX = 0; curX < m_picWidthInMb; curX++)
        {
            uint8_t qpData = *(pInputData + m_encodeParams.psMbQpDataSurface->dwPitch * curY + curX);

            pData->DW0.RegionOfInterestRoiSelection = 0;
            pData->DW1.Qpprimey = m_avcPicParam->NumDeltaQpForNonRectROI == 0 ? qpData  // MBQP in ForceQP mode
                : (qpData == 0 ? 0 : m_avcPicParam->NonRectROIDeltaQpList[qpData - 1]); // MBQP in DeltaQP mode
            pData++;
        }
    }
}

bool CodechalVdencAvcStateXe_Hpm::CheckSupportedFormat(PMOS_SURFACE surface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    bool colorFormatSupported = false;
    if (IS_Y_MAJOR_TILE_FORMAT(surface->TileType) || (surface->TileType == MOS_TILE_LINEAR))
    {
        switch (surface->Format)
        {
        case Format_NV12:
        case Format_YUY2:
        case Format_AYUV:
        case Format_A8R8G8B8:
        case Format_A8B8G8R8:
        case Format_YUYV:
        case Format_YVYU:
        case Format_UYVY:
        case Format_VYUY:
            colorFormatSupported = true;
            break;
        default:
            break;
        }
    }

    return colorFormatSupported;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::SetPictureStructs()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_vdencStreamInEnabled = false;  // will be set for each frame

    MOS_STATUS status = CodechalVdencAvcStateG12::SetPictureStructs();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupThirdRef(
        &(m_resVdencStreamInBuffer[m_currRecycledBufIdx])));

    return status;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::SetupThirdRef(
    PMOS_RESOURCE vdencStreamIn)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_pictureCodingType == I_TYPE)
        return MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_avcPicParam->RefFrameList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(vdencStreamIn);

    auto ppsIdx = m_avcSliceParams->pic_parameter_set_id;

    // Only reference frames with predictors are utilized. Only 2 for VDEnc. One more predictor is added below via StreamIn
    constexpr int32_t numPredictorsVdenc = 2;
    const int32_t     listedRefs         = m_avcSliceParams[ppsIdx].num_ref_idx_l0_active_minus1 + 1 + (m_pictureCodingType == B_TYPE);

    if (listedRefs <= numPredictorsVdenc)  // all reference frames are utilized
        return MOS_STATUS_SUCCESS;

    int32_t toUtilizeIdx = numPredictorsVdenc - (m_pictureCodingType == B_TYPE);  // First from L0 to utilize: 3rd for P, 2nd for B

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    auto pData = (CODECHAL_VDENC_STREAMIN_STATE *)m_osInterface->pfnLockResource(
        m_osInterface,
        vdencStreamIn,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

    if (!m_vdencStreamInEnabled)  // check to allow streamIn sharing
    {
        uint32_t picSizeInMb = m_picHeightInMb * m_picWidthInMb;
        MOS_ZeroMemory(pData, picSizeInMb * CODECHAL_VDENC_STREAMIN_STATE::byteSize);
        m_vdencStreamInEnabled = true;
    }

    for (int32_t curMB = 0; curMB < m_picHeightInMb * m_picWidthInMb; curMB++)
    {
        pData[curMB].DW2.FwdPredictorX = 0;
        pData[curMB].DW2.FwdPredictorY = 0;
        pData[curMB].DW4.FwdRefid0     = toUtilizeIdx;
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        vdencStreamIn);

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->m_currPic            = m_avcPicParam->CurrOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = m_storeData;
        m_debugInterface->m_frameType          = m_pictureCodingType;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &(m_resVdencStreamInBuffer[m_currRecycledBufIdx]),
            CodechalDbgAttr::attrStreamIn,
            "_3rdRef",
            m_picWidthInMb * m_picHeightInMb * CODECHAL_CACHELINE_SIZE,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    )

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::ValidateNumReferences(PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS params)
{
    MOS_STATUS        eStatus             = MOS_STATUS_SUCCESS;
    constexpr uint8_t MaxNumRefPMinusOne  = 2;  // caps.MaxNum_Reference0-1 for all TUs
    constexpr uint8_t MaxNumRefB0MinusOne = 0;  // no corresponding caps
    constexpr uint8_t MaxNumRefB1MinusOne = 0;  // caps.MaxNum_Reference1-1 for all TUs

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);

    uint8_t numRefIdx0MinusOne = params->pAvcSliceParams->num_ref_idx_l0_active_minus1;
    uint8_t numRefIdx1MinusOne = params->pAvcSliceParams->num_ref_idx_l1_active_minus1;

    if (params->wPictureCodingType == P_TYPE)
    {
        if (numRefIdx0MinusOne > MaxNumRefPMinusOne)
        {
            CODECHAL_ENCODE_NORMALMESSAGE("Invalid active reference list size (P).");
            numRefIdx0MinusOne = MaxNumRefPMinusOne;
        }

        numRefIdx1MinusOne = 0;
    }
    else if (params->wPictureCodingType == B_TYPE)
    {
        if (numRefIdx0MinusOne > MaxNumRefB0MinusOne)
        {
            CODECHAL_ENCODE_NORMALMESSAGE("Invalid active reference list size (B0).");
            numRefIdx0MinusOne = MaxNumRefB0MinusOne;
        }

        if (numRefIdx1MinusOne > MaxNumRefB1MinusOne)
        {
            CODECHAL_ENCODE_NORMALMESSAGE("Invalid active reference list1 size (B1).");
            numRefIdx1MinusOne = MaxNumRefB1MinusOne;
        }
    }

    params->pAvcSliceParams->num_ref_idx_l0_active_minus1 = numRefIdx0MinusOne;
    params->pAvcSliceParams->num_ref_idx_l1_active_minus1 = numRefIdx1MinusOne;

    return eStatus;
}

uint32_t CodechalVdencAvcStateXe_Hpm::GetCurrConstDataBufIdx()
{
    return m_avcPicParam->CodingType == B_TYPE && m_avcPicParam->RefPicFlag ? m_avcPicParam->CodingType // refB
                                                                            : m_avcPicParam->CodingType - 1;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::PackHucAuthCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Write HuC Load Info Mask
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    storeDataParams.pOsResource         = &m_hucAuthBuf;
    storeDataParams.dwResourceOffset    = 0;
    storeDataParams.dwValue             = HUC_LOAD_INFO_REG_MASK_G12;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &storeDataParams));

    // Store Huc Auth register
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_hucAuthBuf;
    storeRegParams.dwOffset        = sizeof(uint32_t);
    storeRegParams.dwRegister      = m_hucInterface->GetMmioRegisters(MHW_VDBOX_NODE_1)->hucLoadInfoOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &storeRegParams));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Check Huc auth: if equals to 0 continue chained BB until reset, otherwise send BB end cmd.
    uint32_t compareOperation = mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADEQUALIDD;
    auto hwInterface = dynamic_cast<CodechalHwInterfaceG12 *>(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hwInterface);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(hwInterface->SendCondBbEndCmd(
        &m_hucAuthBuf, 0, 0, false, true, compareOperation, &cmdBuffer));

    // Chained BB loop
    CODECHAL_ENCODE_CHK_STATUS_RETURN(static_cast<MhwMiInterfaceG12 *>(m_miInterface)->AddMiBatchBufferStartCmd(&cmdBuffer, m_batchBuf, true));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::CheckHucLoadStatus()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_COMMAND_BUFFER cmdBuffer = {};
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // add media reset check 100ms, which equals to 1080p WDT threshold
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->SetWatchdogTimerThreshold(1920, 1080, true));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(&cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStartCmd(&cmdBuffer));

    // program 2nd level chained BB for Huc auth
    m_batchBuf = &m_2ndLevelBB[m_currRecycledBufIdx];
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_batchBuf);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &(m_batchBuf->OsResource), &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER hucAuthCmdBuffer;
    MOS_ZeroMemory(&hucAuthCmdBuffer, sizeof(hucAuthCmdBuffer));
    hucAuthCmdBuffer.pCmdBase   = (uint32_t *)data;
    hucAuthCmdBuffer.pCmdPtr    = hucAuthCmdBuffer.pCmdBase;
    hucAuthCmdBuffer.iRemaining = m_batchBuf->iSize;
    hucAuthCmdBuffer.OsResource = m_batchBuf->OsResource;
    hucAuthCmdBuffer.cmdBuf1stLvl = &cmdBuffer;

    //pak check huc status command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(PackHucAuthCmds(hucAuthCmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(m_osInterface, &(m_batchBuf->OsResource)));

    // BB start for 2nd level BB
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, m_batchBuf));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::HuCBrcInitReset()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (MEDIA_IS_WA(m_waTable, WaCheckHucAuthenticationStatus))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CheckHucLoadStatus());
    }
    
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencAvcStateG12::HuCBrcInitReset());

    return MOS_STATUS_SUCCESS;
}

uint32_t CodechalVdencAvcStateXe_Hpm::GetVdencBRCImgStateBufferSize()
{
    return MOS_ALIGN_CEIL(MOS_ALIGN_CEIL(m_hwInterface->m_vdencBrcImgStateBufferSize, CODECHAL_CACHELINE_SIZE) + ENCODE_AVC_MAX_SLICES_SUPPORTED * (m_mfxInterface->GetAvcSlcStateSize() + m_vdencInterface->GetVdencAvcSlcStateSize() + m_miInterface->GetMiBatchBufferEndCmdSize()), CODECHAL_PAGE_SIZE);
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::AddVdencBrcImgBuffer(
    PMOS_RESOURCE             vdencBrcImgBuffer,
    PMHW_VDBOX_AVC_IMG_PARAMS params)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(vdencBrcImgBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mfxInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vdencInterface);

    uint8_t *       data = nullptr;
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, vdencBrcImgBuffer, &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(MOS_COMMAND_BUFFER));
    constructedCmdBuf.pCmdBase   = (uint32_t *)data;
    constructedCmdBuf.iRemaining = GetVdencBRCImgStateBufferSize();

    // Set MFX_IMAGE_STATE command
    constructedCmdBuf.pCmdPtr = (uint32_t *)data;
    constructedCmdBuf.iOffset = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcImgCmd(&constructedCmdBuf, nullptr, params));

    // Set VDENC_CMD3 command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencCmd3Cmd(&constructedCmdBuf, nullptr, params));

    // Set VDENC_IMAGE_STATE command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencImgStateCmd(&constructedCmdBuf, nullptr, params));

    m_miInterface->AddBatchBufferEndInsertionFlag(constructedCmdBuf);

    // AddBatchBufferEndInsertionFlag doesn't modify pCmdPtr + iOffset
    constructedCmdBuf.pCmdPtr += m_miBatchBufferEndSize / sizeof(uint32_t);
    constructedCmdBuf.iOffset += m_miBatchBufferEndSize;
    constructedCmdBuf.iRemaining -= m_miBatchBufferEndSize;

    // Add MI_NOOPs to align to CODECHAL_CACHELINE_SIZE
    uint32_t size = (MOS_ALIGN_CEIL(constructedCmdBuf.iOffset, CODECHAL_CACHELINE_SIZE) - constructedCmdBuf.iOffset) / sizeof(uint32_t);
    for (uint32_t i = 0; i < size; i++)
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiNoop(&constructedCmdBuf, nullptr));

    CODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS packSlcHeaderParams = {};
    MHW_VDBOX_AVC_SLICE_STATE                  sliceState          = {};
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCommonSliceState(packSlcHeaderParams, sliceState));

    for (uint16_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSliceState(packSlcHeaderParams, sliceState, slcCount));

        size = constructedCmdBuf.iOffset;

        // Set MFX_AVC_SLICE_STATE command
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcSlice(&constructedCmdBuf, nullptr, &sliceState));

        // Set VDENC_AVC_SLICE_STATE command
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencSliceStateCmd(&constructedCmdBuf, &sliceState));

        m_miInterface->AddBatchBufferEndInsertionFlag(constructedCmdBuf);
        constructedCmdBuf.pCmdPtr += m_miBatchBufferEndSize / sizeof(uint32_t);
        constructedCmdBuf.iOffset += m_miBatchBufferEndSize;
        constructedCmdBuf.iRemaining -= m_miBatchBufferEndSize;

        CODECHAL_ENCODE_ASSERT(constructedCmdBuf.iOffset - size < CODECHAL_CACHELINE_SIZE);
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        vdencBrcImgBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::AddMfxAvcSlice(
    PMOS_COMMAND_BUFFER        cmdBuffer,
    PMHW_BATCH_BUFFER          batchBuffer,
    PMHW_VDBOX_AVC_SLICE_STATE avcSliceState)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(avcSliceState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mfxInterface);

    if (m_vdencBrcEnabled)
    {
        // current location to add cmds in 2nd level batch buffer
        m_batchBufferForVdencImgStat[0].iCurrent = 0;
        // reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
        // base part of 2nd lvl BB must be aligned for CODECHAL_CACHELINE_SIZE
        m_batchBufferForVdencImgStat[0].dwOffset = MOS_ALIGN_CEIL(m_mfxAvcImgStateSize + m_vdencCmd3Size + m_vdencAvcImgStateSize + m_miBatchBufferEndSize, CODECHAL_CACHELINE_SIZE) +
                                                   avcSliceState->dwSliceIndex * (m_mfxAvcSlcStateSize + m_vdencAvcSlcStateSize + m_miBatchBufferEndSize);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(cmdBuffer, &m_batchBufferForVdencImgStat[0]));
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcSlice(cmdBuffer, batchBuffer, avcSliceState));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::AddVdencSliceStateCmd(
    PMOS_COMMAND_BUFFER        cmdBuffer,
    PMHW_VDBOX_AVC_SLICE_STATE params)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vdencInterface);

    // VDENC_AVC_SLICE_STATE was added in AddVdencBrcImgBuffer
    // execution will be triggered in AddMfxAvcSlice in 2nd lvl BB through MI_BATCH_BUFFER_START cmd
    if (!m_vdencBrcEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencSliceStateCmd(cmdBuffer, params));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::Execute(void *params)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_ENCODE, PERF_LEVEL_HAL);

    MOS_TraceEventExt(EVENT_CODECHAL_EXECUTE, EVENT_TYPE_START, &m_codecFunction, sizeof(m_codecFunction), nullptr, 0);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(Codechal::Execute(params));

    EncoderParams *encodeParams = (EncoderParams *)params;
    // MSDK event handling
    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mos_Solo_SetGpuAppTaskEvent(m_osInterface, encodeParams->gpuAppTaskEvent));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->SetWatchdogTimerThreshold(m_frameWidth, m_frameHeight));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SwitchContext());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ExecuteEnc(encodeParams));

    MOS_TraceEventExt(EVENT_CODECHAL_EXECUTE, EVENT_TYPE_END, nullptr, 0, nullptr, 0);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::SwitchContext()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (CodecHalUsesVideoEngine(m_codecFunction) && !m_isContextSwitched)
    {
        if (MEDIA_IS_SKU(m_skuTable, FtrVcs2) ||
            (MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface) && m_numVdbox > 1))
        {
            MOS_GPU_NODE encoderNode = m_osInterface->pfnGetLatestVirtualNode(m_osInterface, COMPONENT_Encode);
            MOS_GPU_NODE decoderNode = m_osInterface->pfnGetLatestVirtualNode(m_osInterface, COMPONENT_Decode);
            // switch encoder to different virtual node
            if ((encoderNode == m_videoGpuNode) || (decoderNode == m_videoGpuNode))
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(ChangeContext());
            }
            m_osInterface->pfnSetLatestVirtualNode(m_osInterface, m_videoGpuNode);
        }
    }
    // switch only once
    m_isContextSwitched = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcStateXe_Hpm::ChangeContext()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnDestroyVideoNodeAssociation(
        m_osInterface,
        m_videoGpuNode));
    MOS_GPU_NODE videoGpuNode = (m_videoGpuNode == MOS_GPU_NODE_VIDEO) ? MOS_GPU_NODE_VIDEO2 : MOS_GPU_NODE_VIDEO;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateVideoNodeAssociation(
        m_osInterface,
        true,
        &videoGpuNode));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnDestroyGpuContext(
        m_osInterface,
        m_videoContext));
    MOS_GPU_CONTEXT gpuContext = (videoGpuNode == MOS_GPU_NODE_VIDEO2) && !MOS_VE_MULTINODESCALING_SUPPORTED(m_osInterface) ? MOS_GPU_CONTEXT_VDBOX2_VIDEO3 : MOS_GPU_CONTEXT_VIDEO3;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        gpuContext,
        videoGpuNode,
        m_gpuCtxCreatOpt));
    m_videoGpuNode = videoGpuNode;
    m_videoContext = gpuContext;
    m_osInterface->pfnSetEncodePakContext(m_osInterface, m_videoContext);
    m_vdboxIndex = (m_videoGpuNode == MOS_GPU_NODE_VIDEO2) ? MHW_VDBOX_NODE_2 : MHW_VDBOX_NODE_1;

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalVdencAvcStateXe_Hpm::PopulateEncParam(uint8_t meMethod, void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

uint32_t CodechalVdencAvcStateXe_Hpm::GetPakVDEncPassDumpSize()
{
    return GetVdencBRCImgStateBufferSize();
}
#endif
