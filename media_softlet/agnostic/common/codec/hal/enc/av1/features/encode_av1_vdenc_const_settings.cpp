/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     encode_av1_vdenc_const_settings.cpp
//! \brief    Defines the common interface for av1 vdenc const settings
//! \details  The encode feature manager is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_av1_vdenc_const_settings.h"
#include "codec_def_common_av1.h"
#include "codec_def_encode.h"
#include "encode_utils.h"
#include "mhw_vdbox_vdenc_cmdpar.h"
#include "mos_utilities.h"
#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace encode
{
// 8 slots designed for {TU1,TU2,TU3,TU4,TU5,TU6,TU7,Potential_feature}
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par39[NUM_TARGET_USAGE_MODES] = { 12, 12, 12, 8, 8, 8, 4, 4 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par38[NUM_TARGET_USAGE_MODES] = { 4, 4, 4, 3, 3, 3, 2, 2 };
const uint32_t Av1VdencTUConstSettings::vdencCmd2Par85Table0[NUM_TARGET_USAGE_MODES] = { 1, 1, 1, 3, 3, 3, 3, 3 };
const uint32_t Av1VdencTUConstSettings::vdencCmd2Par85Table1[NUM_TARGET_USAGE_MODES] = { 1, 1, 1, 1, 1, 1, 0, 0 };
const bool     Av1VdencTUConstSettings::vdencCmd2Par86[NUM_TARGET_USAGE_MODES] = { false, false, false, false, false, false, true, true };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par87Table0[NUM_TARGET_USAGE_MODES] = { 3, 3, 3, 2, 2, 2, 2, 2 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par87Table1[NUM_TARGET_USAGE_MODES] = { 3, 3, 3, 2, 2, 2, 2, 2 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par87Table2[NUM_TARGET_USAGE_MODES] = { 3, 3, 3, 2, 2, 2, 2, 2 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par87Table3[NUM_TARGET_USAGE_MODES] = { 3, 3, 3, 2, 2, 2, 2, 2 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table0[NUM_TARGET_USAGE_MODES] = { 3, 3, 3, 1, 1, 1, 1, 1 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table1[NUM_TARGET_USAGE_MODES] = { 3, 3, 3, 3, 3, 3, 3, 3 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table2[NUM_TARGET_USAGE_MODES] = {3, 3, 3, 3, 3, 3, 2, 2 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table3[NUM_TARGET_USAGE_MODES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table4[NUM_TARGET_USAGE_MODES] = { 1, 1, 1, 1, 1, 1, 1, 1 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table5[NUM_TARGET_USAGE_MODES] = { 3, 3, 3, 3, 3, 3, 3, 3 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table6[NUM_TARGET_USAGE_MODES] = { 3, 3, 3, 1, 1, 1, 1, 1 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table7[NUM_TARGET_USAGE_MODES] = { 3, 3, 3, 1, 1, 1, 1, 1 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table8[NUM_TARGET_USAGE_MODES] = { 7, 7, 7, 7, 7, 7, 1, 1 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table9[NUM_TARGET_USAGE_MODES] = { 3, 3, 3, 1, 1, 1, 1, 1 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table10[NUM_TARGET_USAGE_MODES] = { 7, 7, 7, 3, 3, 3, 2, 2 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par88Table11[NUM_TARGET_USAGE_MODES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
const bool     Av1VdencTUConstSettings::vdencCmd2Par89[NUM_TARGET_USAGE_MODES] = { false, false, false, false, false, false, true, true };
const bool     Av1VdencTUConstSettings::vdencCmd2Par94[NUM_TARGET_USAGE_MODES] = { false, false, false, true, true, true, true, true };
const bool     Av1VdencTUConstSettings::vdencCmd2Par95[NUM_TARGET_USAGE_MODES] = { false, false, false, false, false, false, true, true };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par98[NUM_TARGET_USAGE_MODES] = { 0, 0, 0, 0, 0, 0, 1, 1 };
const bool     Av1VdencTUConstSettings::vdencCmd2Par97[NUM_TARGET_USAGE_MODES] = { false, false, false, true, true, false, false, false };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par100[NUM_TARGET_USAGE_MODES] = { 0, 0, 0, 0, 0, 0, 0, 0 };
const uint8_t  Av1VdencTUConstSettings::vdencCmd2Par96[NUM_TARGET_USAGE_MODES] = { 0, 0, 0, 0, 0, 0, 3, 3};
const uint16_t Av1VdencTUConstSettings::vdencCmd2Par93[NUM_TARGET_USAGE_MODES] = { 0xffff, 0xffff, 0xffff, 0xff00, 0xff00, 0xff00, 0x8000, 0x8000 };
const uint16_t Av1VdencTUConstSettings::vdencCmd2Par92[NUM_TARGET_USAGE_MODES] = { 0xffff, 0xffff, 0xffff, 0xff00, 0xff00, 0xff00, 0xfffc, 0xfffc };
const bool     Av1VdencTUConstSettings::vdencCmd2Par23[NUM_TARGET_USAGE_MODES] = { false, false, false, false, false, false, false, false };

constexpr int8_t Av1VdencBrcConstSettings::instRateThresholdI[4] = { 30, 50, 90, 115 };
constexpr int8_t Av1VdencBrcConstSettings::instRateThresholdP[4] = { 30, 50, 70, 120 };
constexpr double Av1VdencBrcConstSettings::devThresholdFpNegI[4] = { 0.80, 0.60, 0.34, 0.2 };
constexpr double Av1VdencBrcConstSettings::devThresholdFpPosI[4] = { 0.2, 0.4, 0.66, 0.9 };
constexpr double Av1VdencBrcConstSettings::devThresholdFpNegPB[4] = { 0.90, 0.66, 0.46, 0.3 };
constexpr double Av1VdencBrcConstSettings::devThresholdFpPosPB[4] = { 0.3, 0.46, 0.70, 0.90 };
constexpr double Av1VdencBrcConstSettings::devThresholdVbrNeg[4] = { 0.90, 0.70, 0.50, 0.3 };
constexpr double Av1VdencBrcConstSettings::devThresholdVbrPos[4] = { 0.4, 0.5, 0.75, 0.90 };
constexpr uint8_t Av1VdencBrcConstSettings::QPThresholds[4] = { 40, 80, 120, 180 };
constexpr uint16_t Av1VdencBrcConstSettings::startGlobalAdjustFrame[4] = {10,50,100,150};
constexpr uint8_t Av1VdencBrcConstSettings::globalRateRatioThreshold[6] = {40, 75, 97, 103, 125, 160};
constexpr int8_t Av1VdencBrcConstSettings::globalRateRatioThresholdQP[7] = {-6, -4, -2, 0, 2, 4, 6};
constexpr uint8_t Av1VdencBrcConstSettings::startGlobalAdjustMult[5] = {1, 1, 3, 2, 1};
constexpr uint8_t Av1VdencBrcConstSettings::startGlobalAdjustDiv[5] = {40, 5, 5, 3, 1};
constexpr uint8_t Av1VdencBrcConstSettings::distortionThresh[9] = { 4,30,60,80, 120,140,200,255, 0 };
constexpr uint8_t Av1VdencBrcConstSettings::distortionThreshB[9] = { 2,20,40,70, 130,160,200,255, 0 };
constexpr uint8_t Av1VdencBrcConstSettings::maxFrameMultI[5] = { 8, 9, 10, 11, 12 };
constexpr uint8_t Av1VdencBrcConstSettings::maxFrameMultP[5] = { 4, 5, 6, 6, 7 };
constexpr int8_t Av1VdencBrcConstSettings::av1DeltaQpI[][5] =
{
    {2, 6, 10, 14, 18, },
    {2, 4, 6, 10, 14, },
    {0, 0, 2, 4, 8, },
    {0, 0, 0, 2, 4, },
    {-2, 0, 0, 0, 2, },
    {-6, -4, -2, 0, 0, },
    {-10, -8, -4, -2, 0, },
    {-14, -12, -8, -4, -2, },
    {-18, -14, -10, -4, -2, },
};

constexpr int8_t Av1VdencBrcConstSettings::av1DeltaQpP[][5] =
{
    {2, 4, 10, 16, 20, },
    {2, 4, 8, 12, 16, },
    {0, 2, 4, 8, 12, },
    {0, 0, 0, 2, 4, },
    {-2, 0, 0, 0, 2, },
    {-4, -2, -2, 0, 0, },
    {-6, -4, -2, -2, 0, },
    {-10, -6, -4, -2, 0, },
    {-14, -12, -8, -4, -2, },
};

constexpr int8_t Av1VdencBrcConstSettings::av1DistortionsDeltaQpI[][9] =
{
    { 0,  0,  0,  0,  0,  8,  12, 16, 20 },
    { 0,  0,  0,  0,  0,  6,  10, 14, 18 },
    {-2,  0,  0,  0,  0,  6,  8,  12, 14 },
    {-4, -2,  0,  0,  0,  2,  4,  6,  10 },
    {-6, -4, -2,  0,  0,  0,  2,  6,  10 },
    {-8, -4, -2,  0,  0,  0,  2,  6,  10 },
    {-10,-6, -4, -2,  0,  0,  2,  6,  10 },
    {-12,-8, -4, -2,  0,  0,  2,  6,  10 },
    {-12,-8, -4, -2,  0,  0,  2,  6,  10 },
};

constexpr int8_t Av1VdencBrcConstSettings::av1DistortionsDeltaQpP[][9] =
{
    { 0,  0,  0,  0,  0,  6,  10, 14,  18 },
    { 0,  0,  0,  0,  0,  6,  10, 12,  16 },
    {-2,  0,  0,  0,  0,  6,  10, 14,  16 },
    {-4, -2,  0,  0,  0,  4,  8,  10,  12 },
    {-6, -4, -2,  0,  0,  0,  2,  8,   10 },
    {-8, -4, -2,  0,  0,  0,  2,  8,   10 },
    {-8, -4, -2, -2,  0,  0,  0,  8,   10 },
    {-8, -6, -4, -2,  0,  0,  0,  2,   10 },
    {-10,-8, -4, -2,  0,  0,  0,  2,   8 },
};

constexpr uint8_t Av1VdencBrcConstSettings::loopFilterLevelTabLuma[256] = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  2,
     2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,
     4,  4,  4,  4,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,
     6,  7,  7,  7,  8,  8,  8,  8,  9,  9,  9,  9, 10, 10, 10, 10,
    11, 11, 11, 11, 12, 12, 12, 12, 13, 13, 13, 14, 14, 14, 15, 15,
    15, 16, 16, 16, 17, 17, 17, 17, 18, 18, 18, 19, 19, 20, 20, 20,
    21, 21, 21, 22, 22, 22, 23, 23, 24, 24, 24, 25, 25, 25, 26, 26,
    27, 27, 27, 28, 28, 29, 29, 29, 30, 30, 31, 31, 31, 32, 32, 33,
    33, 34, 34, 34, 35, 35, 36, 36, 37, 37, 38, 38, 39, 39, 40, 41,
    41, 42, 42, 43, 44, 45, 45, 46, 47, 48, 49, 50, 51, 52, 53, 55,
    56, 58, 59, 61, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63,
    63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63, 63
};

constexpr uint8_t Av1VdencBrcConstSettings::loopFilterLevelTabChroma[256] = {
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,  1,  1,  1,  1,  2,
     2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  3,  3,  3,
     3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
     5,  5,  5,  5,  5,  5,  5,  5,  6,  6,  6,  6,  6,  6,  6,  6,
     6,  6,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  8,  8,
     8,  8,  8,  8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9,  9, 10,
    10, 10, 10, 10, 11, 11, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15,
    16, 17, 18, 19, 20, 21, 22, 24, 25, 26, 28, 30, 31, 31, 31, 31,
    31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
    31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
    31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
    31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
    31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31
};

const uint32_t Av1VdencBrcConstSettings::hucModeCostsIFrame[52 * 6] =
{
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t Av1VdencBrcConstSettings::hucModeCostsPFrame[52 * 6] =
{
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
        0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515,
        0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000,
        0x00000000, 0x00000000, 0x10102f1e, 0x001e1515, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

EncodeAv1VdencConstSettings::EncodeAv1VdencConstSettings(PMOS_INTERFACE osInterface) : VdencConstSettings(osInterface)
{
    m_osInterface = osInterface;
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);
    m_featureSetting = MOS_New(Av1VdencFeatureSettings);
}

MOS_STATUS EncodeAv1VdencConstSettings::Update(void *params)
{
    ENCODE_FUNC_CALL();

    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS av1SeqParams =
        static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(av1SeqParams);
    m_av1SeqParams = av1SeqParams;

    PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams =
        static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(av1PicParams);
    m_av1PicParams = av1PicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencConstSettings::PrepareConstSettings()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(SetTUSettings());
    ENCODE_CHK_STATUS_RETURN(SetTable());
    ENCODE_CHK_STATUS_RETURN(SetVdencCmd1Settings());
    ENCODE_CHK_STATUS_RETURN(SetVdencCmd2Settings());
    ENCODE_CHK_STATUS_RETURN(SetBrcSettings());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencConstSettings::SetCommonSettings()
{
    ENCODE_FUNC_CALL();

    //TBD

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencConstSettings::SetBrcSettings()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Av1VdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

#define SET_TABLE(X) setting->brcSettings.X.data = (void *)Av1VdencBrcConstSettings::X;\
                     setting->brcSettings.X.size = sizeof(Av1VdencBrcConstSettings::X);

        SET_TABLE(instRateThresholdP);
        SET_TABLE(instRateThresholdI);
        SET_TABLE(devThresholdFpNegI);
        SET_TABLE(devThresholdFpPosI);
        SET_TABLE(devThresholdFpNegPB);
        SET_TABLE(devThresholdFpPosPB);
        SET_TABLE(devThresholdVbrNeg);
        SET_TABLE(devThresholdVbrPos);

        SET_TABLE(QPThresholds);
        SET_TABLE(startGlobalAdjustFrame);
        SET_TABLE(globalRateRatioThreshold);
        SET_TABLE(globalRateRatioThresholdQP);
        SET_TABLE(startGlobalAdjustMult);
        SET_TABLE(startGlobalAdjustDiv);
        SET_TABLE(distortionThresh);
        SET_TABLE(distortionThreshB);
        SET_TABLE(maxFrameMultI);
        SET_TABLE(maxFrameMultP);
        SET_TABLE(av1DeltaQpI);
        SET_TABLE(av1DeltaQpP);
        SET_TABLE(av1DistortionsDeltaQpI);
        SET_TABLE(av1DistortionsDeltaQpP);
        SET_TABLE(loopFilterLevelTabLuma);
        SET_TABLE(loopFilterLevelTabChroma);
        SET_TABLE(hucModeCostsIFrame);
        SET_TABLE(hucModeCostsPFrame);

#undef SET_TABLE

#define SET_DATA(X) setting->brcSettings.X = Av1VdencBrcConstSettings::X;

        SET_DATA(numInstRateThresholds);
        SET_DATA(devStdFPS);
        SET_DATA(bpsRatioLow);
        SET_DATA(bpsRatioHigh);
        SET_DATA(postMultPB);
        SET_DATA(negMultPB);
        SET_DATA(posMultVBR);
        SET_DATA(negMultVBR);
        SET_DATA(numDevThreshlds);

        SET_DATA(numQpThresholds);
        SET_DATA(numGlobalRateRatioThreshlds);
        SET_DATA(numStartGlobalAdjusts);
        SET_DATA(numDistortionThresholds);

#undef SET_DATA

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencConstSettings::SetVdencCmd1Settings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Av1VdencFeatureSettings*>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencCmd1Settings = {
        VDENC_CMD1_LAMBDA()
        {
            auto qp = m_av1PicParams->base_qindex;
            bool isIntra = AV1_KEY_OR_INRA_FRAME(m_av1PicParams->PicFlags.fields.frame_type);
            static const std::array<
                    std::array<
                        uint16_t,
                        256>,
                    2>
                par1Array =
                    {{
                        {1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   4,   4,   4,
                         4,   4,   4,   4,   5,   5,   5,   5,   5,   5,   6,   6,   6,   6,   6,   6,   7,   7,   7,
                         7,   7,   7,   7,   8,   8,   8,   8,   8,   8,   9,   9,   9,   9,   9,   9,   9,  10,  10,
                         10,  10,  10,  10,  11,  11,  11,  11,  11,  11,  11,  12,  12,  12,  12,  12,  12,  13,  13,
                         13,  13,  13,  13,  13,  14,  14,  14,  14,  14,  14,  15,  15,  15,  15,  15,  15,  16,  16,
                         16,  16,  16,  17,  17,  17,  18,  18,  18,  19,  19,  19,  20,  20,  20,  20,  21,  21,  21,
                         22,  22,  22,  23,  23,  23,  24,  24,  24,  25,  25,  26,  26,  27,  27,  28,  28,  29,  29,
                         30,  30,  31,  31,  31,  32,  33,  33,  34,  35,  35,  36,  36,  37,  38,  38,  39,  40,  40,
                         41,  42,  43,  43,  44,  45,  46,  47,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,
                         58,  59,  60,  61,  62,  63,  64,  66,  67,  68,  69,  71,  72,  73,  75,  76,  78,  79,  81,
                         82,  84,  85,  87,  89,  90,  92,  94,  95,  97,  99, 101, 103, 105, 107, 109, 111, 113, 115,
                         117, 120, 122, 124, 126, 129, 131, 134, 137, 139, 142, 145, 147, 150, 153, 156, 159, 162, 165,
                         168, 172, 175, 178, 182, 185, 189, 193, 196, 200, 204, 208, 212, 216, 221, 225, 229, 234, 238,
                         243, 248, 252, 257, 262, 267, 273, 278, 283},
                        {1,   1,   1,   2,   2,   2,   2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   4,   4,
                         4,   4,   4,   4,   5,   5,   5,   5,   5,   5,   5,   6,   6,   6,   6,   6,   6,   6,   7,
                         7,   7,   7,   7,   7,   8,   8,   8,   8,   8,   8,   8,   9,   9,   9,   9,   9,   9,   9,
                         10,  10,  10,  10,  10,  10,  11,  11,  11,  11,  11,  11,  11,  12,  12,  12,  12,  12,  12,
                         12,  13,  13,  13,  13,  13,  13,  14,  14,  14,  14,  14,  14,  14,  15,  15,  15,  15,  15,
                         15,  16,  16,  16,  17,  17,  17,  17,  18,  18,  18,  19,  19,  19,  20,  20,  20,  20,  21,
                         21,  21,  22,  22,  22,  23,  23,  23,  24,  24,  25,  25,  26,  26,  26,  27,  27,  28,  28,
                         29,  29,  30,  30,  30,  31,  32,  32,  33,  33,  34,  35,  35,  36,  36,  37,  38,  38,  39,
                         40,  41,  41,  42,  43,  44,  44,  45,  46,  47,  48,  48,  49,  50,  51,  52,  53,  54,  55,
                         56,  57,  58,  59,  60,  61,  62,  64,  65,  66,  67,  68,  70,  71,  72,  74,  75,  77,  78,
                         80,  81,  83,  84,  86,  87,  89,  91,  92,  94,  96,  98,  99, 101, 103, 105, 107, 109, 111,
                         114, 116, 118, 120, 122, 125, 127, 130, 132, 135, 137, 140, 143, 145, 148, 151, 154, 157, 160,
                         163, 166, 169, 173, 176, 179, 183, 186, 190, 194, 198, 201, 205, 209, 213, 218, 222, 226, 231,
                         235, 240, 244, 249, 254, 259, 264, 269, 274},
                    }};
            par.vdencCmd1Par1 = par1Array[isIntra ? 0 : 1][qp];

              static const std::array<
                    std::array<
                        uint16_t,
                        256>,
                    2>
                par0Array =
                    {{
                        {0,   0,   0,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   3,   3,   3,   3,   4,
                         4,   4,   5,   5,   5,   6,   6,   7,   7,   7,   8,   8,   9,   9,  10,  10,  11,  11,  12,
                         12,  13,  13,  14,  14,  15,  16,  16,  17,  18,  18,  19,  20,  20,  21,  22,  22,  23,  24,
                         25,  25,  26,  27,  28,  29,  29,  30,  31,  32,  33,  34,  35,  36,  37,  37,  38,  39,  40,
                         41,  42,  43,  44,  45,  47,  48,  49,  50,  51,  52,  53,  54,  55,  57,  58,  59,  60,  61,
                         62,  65,  67,  70,  73,  75,  78,  81,  84,  86,  89,  92,  95,  98, 102, 105, 108, 111, 114,
                         118, 121, 125, 128, 132, 135, 139, 144, 150, 156, 162, 168, 174, 180, 186, 192, 199, 206, 212,
                         219, 226, 233, 240, 248, 257, 267, 278, 288, 299, 309, 320, 332, 343, 355, 366, 378, 391, 406,
                         422, 438, 454, 471, 488, 505, 523, 541, 559, 581, 604, 627, 650, 674, 698, 723, 748, 774, 805,
                         836, 867, 900, 932, 966, 1000, 1039, 1080, 1121, 1163, 1205, 1249, 1299, 1349, 1401, 1454, 1508,
                         1562, 1624, 1687, 1751, 1817, 1884, 1958, 2034, 2112, 2191, 2272, 2361, 2452, 2545, 2640, 2745,
                         2851, 2960, 3071, 3192, 3316, 3442, 3570, 3711, 3854, 3999, 4158, 4319, 4484, 4662, 4843, 5029,
                         5228, 5432, 5640, 5863, 6091, 6323, 6572, 6825, 7097, 7374, 7656, 7957, 8264, 8591, 8925, 9280,
                         9642, 10026, 10418, 10833, 11257, 11705, 12162, 12646, 13138, 13659, 14189, 14748, 15318, 15919,
                         16551, 17195, 17872, 18584, 19309, 20070},
                        {0,   0,   0,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,   2,   3,   3,   3,   4,   4,
                         4,   4,   5,   5,   5,   6,   6,   7,   7,   7,   8,   8,   9,   9,   9,  10,  10,  11,  11,  12,
                         12,  13,  14,  14,  15,  15,  16,  16,  17,  18,  18,  19,  20,  20,  21,  22,  22,  23,  24,  25,
                         25,  26,  27,  28,  28,  29,  30,  31,  32,  32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,
                         43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  59,  61,  63,  66,  68,
                         71,  73,  76,  78,  81,  84,  86,  89,  92,  95,  98, 101, 104, 107, 110, 113, 117, 120, 123, 127,
                         130, 135, 140, 146, 151, 157, 163, 168, 174, 180, 186, 193, 199, 205, 212, 218, 225, 232, 241, 250,
                         260, 270, 280, 290, 300, 311, 321, 332, 343, 354, 366, 380, 395, 410, 425, 441, 457, 473, 490, 506,
                         523, 544, 565, 587, 609, 631, 654, 677, 701, 725, 754, 783, 812, 842, 873, 905, 936, 973, 1011, 1050,
                         1089, 1129, 1170, 1216, 1264, 1312, 1362, 1412, 1463, 1521, 1580, 1640, 1702, 1764, 1834, 1905, 1978,
                         2052, 2128, 2211, 2297, 2384, 2473, 2570, 2670, 2772, 2876, 2989, 3105, 3223, 3344, 3475, 3609, 3745,
                         3894, 4045, 4199, 4366, 4536, 4709, 4897, 5087, 5282, 5491, 5704, 5921, 6154, 6392, 6646, 6906, 7170,
                         7452, 7740, 8046, 8359, 8691, 9030, 9390, 9757, 10146, 10542, 10962, 11390, 11843, 12304, 12792, 13288,
                         13812, 14346, 14908, 15500, 16104, 16738, 17404, 18084, 18796},
                    }};
            par.vdencCmd1Par0 = par0Array[isIntra ? 0 : 1][qp];

            static const std::array<
                    std::array<
                        uint8_t,
                        8>,
                    2>
                par2Array =
                    {{
                        {0, 2, 3, 5, 6, 8, 9, 11},
                        {0, 2, 3, 5, 6, 8, 9, 11},
                    }};
            static const std::array< uint8_t, 12> par3Array = {4, 14, 24, 34, 44, 54, 64, 74, 84, 94, 104, 114};
            static const std::array< uint8_t, 12> par4Array = { 3, 9, 14, 19, 24, 29, 34, 39, 44, 49, 54, 60};
            for (auto i = 0; i < 8; i++)
            {
                par.vdencCmd1Par2[i] = par2Array[isIntra? 0 : 1][i];
                par.vdencCmd1Par3[i] = par3Array[i];
                par.vdencCmd1Par4[i] = par4Array[i];
            }
            for (auto i = 8; i < 12; i++)
            {
                par.vdencCmd1Par3[i] = par3Array[i];
                par.vdencCmd1Par4[i] = par4Array[i];
            }

            return MOS_STATUS_SUCCESS;
        },

        VDENC_CMD1_LAMBDA()
        {

            par.vdencCmd1Par50 = 5;
            par.vdencCmd1Par49 = 5;
            par.vdencCmd1Par48 = 5;
            par.vdencCmd1Par47 = 5;
            par.vdencCmd1Par54 = 12;
            par.vdencCmd1Par53 = 12;
            par.vdencCmd1Par52 = 12;
            par.vdencCmd1Par51 = 12;

            par.vdencCmd1Par58 = 18;
            par.vdencCmd1Par57 = 18;
            par.vdencCmd1Par56 = 18;
            par.vdencCmd1Par55 = 18;
            par.vdencCmd1Par74 = 16;
            par.vdencCmd1Par73 = 16;
            par.vdencCmd1Par72 = 16;
            par.vdencCmd1Par71 = 16;

            par.vdencCmd1Par62 = 16;
            par.vdencCmd1Par61 = 16;
            par.vdencCmd1Par60 = 16;
            par.vdencCmd1Par59 = 16;
            par.vdencCmd1Par78 = 16;
            par.vdencCmd1Par77 = 16;
            par.vdencCmd1Par76 = 16;
            par.vdencCmd1Par75 = 16;

           par.vdencCmd1Par66 = 16;
           par.vdencCmd1Par65 = 16;
           par.vdencCmd1Par64 = 16;
           par.vdencCmd1Par63 = 16;
           par.vdencCmd1Par82 = 16;
           par.vdencCmd1Par81 = 16;
           par.vdencCmd1Par80 = 16;
           par.vdencCmd1Par79 = 16;

           par.vdencCmd1Par70 = 22;
           par.vdencCmd1Par69 = 22;
           par.vdencCmd1Par68 = 22;
           par.vdencCmd1Par67 = 22;
           par.vdencCmd1Par86 = 26;
           par.vdencCmd1Par85 = 26;
           par.vdencCmd1Par84 = 26;
           par.vdencCmd1Par83 = 26;

           return MOS_STATUS_SUCCESS;
       },

       VDENC_CMD1_LAMBDA()
       {
           bool     isIntra = AV1_KEY_OR_INRA_FRAME(m_av1PicParams->PicFlags.fields.frame_type);
           if (isIntra)
           {
               par.vdencCmd1Par23 = 42;
               par.vdencCmd1Par24 = 0;
               par.vdencCmd1Par25 = 0;
               par.vdencCmd1Par26 = 0;
               par.vdencCmd1Par27 = 0;
               par.vdencCmd1Par28 = 0;
               par.vdencCmd1Par29 = 0;
               par.vdencCmd1Par30 = 0;
               par.vdencCmd1Par31 = 0;
               par.vdencCmd1Par32 = 0;
               par.vdencCmd1Par33 = 0;
               par.vdencCmd1Par34 = 21;
               par.vdencCmd1Par36 = 21;
               par.vdencCmd1Par37 = 47;
               par.vdencCmd1Par38 = 16;
               par.vdencCmd1Par39 = 16;
               par.vdencCmd1Par42 = 58;
               par.vdencCmd1Par43 = 20;
               par.vdencCmd1Par40 = 30;
               par.vdencCmd1Par41 = 30;
               par.vdencCmd1Par44 = 0;
               par.vdencCmd1Par45 = 20;
               par.vdencCmd1Par46 = 0;
           }
           else if (isLowDelay)
           {
               par.vdencCmd1Par6 = 3;
               par.vdencCmd1Par5 = 6;
               par.vdencCmd1Par7 = 10;
               par.vdencCmd1Par8[0] = 5;
               par.vdencCmd1Par9[0] = 6;
               par.vdencCmd1Par12[0] = 23;
               par.vdencCmd1Par13[0] = 26;
               par.vdencCmd1Par10[0] = 5;
               par.vdencCmd1Par11[0] = 0;
               par.vdencCmd1Par14[0] = 21;
               par.vdencCmd1Par15[0] = 0;
               par.vdencCmd1Par16 = 92;
               par.vdencCmd1Par17 = 19;
               par.vdencCmd1Par18 = 92;
               par.vdencCmd1Par19 = 18;
               par.vdencCmd1Par23 = 54;
               par.vdencCmd1Par20 = 15;
               par.vdencCmd1Par21 = 4;
               par.vdencCmd1Par22 = 4;
               par.vdencCmd1Par24 = 0;
               par.vdencCmd1Par25 = 0;
               par.vdencCmd1Par26 = 0;
               par.vdencCmd1Par27 = 0;
               par.vdencCmd1Par28 = 0;
               par.vdencCmd1Par29 = 0;
               par.vdencCmd1Par30 = 0;
               par.vdencCmd1Par31 = 0;
               par.vdencCmd1Par32 = 0;
               par.vdencCmd1Par33 = 0;
               par.vdencCmd1Par34 = 21;
               par.vdencCmd1Par36 = 21;
               par.vdencCmd1Par37 = 23;
               par.vdencCmd1Par38 = 24;
               par.vdencCmd1Par39 = 27;
               par.vdencCmd1Par40 = 41;
               par.vdencCmd1Par41 = 68;
               par.vdencCmd1Par42 = 37;
               par.vdencCmd1Par43 = 37;
               par.vdencCmd1Par44 = 0;
               par.vdencCmd1Par45 = 12;
               par.vdencCmd1Par46 = 0;
               par.vdencCmd1Par87 = 20;
               par.vdencCmd1Par88 = 20;
               par.vdencCmd1Par89 = 20;
           }
           else
           {
               par.vdencCmd1Par6 = 3;
               par.vdencCmd1Par5 = 6;
               par.vdencCmd1Par7 = 10;
               par.vdencCmd1Par8[0] = 5;
               par.vdencCmd1Par9[0] = 6;
               par.vdencCmd1Par12[0] = 23;
               par.vdencCmd1Par13[0] = 26;
               par.vdencCmd1Par10[0] = 5;
               par.vdencCmd1Par11[0] = 0;
               par.vdencCmd1Par14[0] = 21;
               par.vdencCmd1Par15[0] = 0;
               par.vdencCmd1Par16 = 92;
               par.vdencCmd1Par17 = 19;
               par.vdencCmd1Par18 = 92;
               par.vdencCmd1Par19 = 18;
               par.vdencCmd1Par21 = 4;
               par.vdencCmd1Par22 = 4;
               par.vdencCmd1Par23 = 54;
               par.vdencCmd1Par20 = 15;
               par.vdencCmd1Par24 = 0;
               par.vdencCmd1Par25 = 0;
               par.vdencCmd1Par26 = 0;
               par.vdencCmd1Par27 = 0;
               par.vdencCmd1Par28 = 0;
               par.vdencCmd1Par29 = 0;
               par.vdencCmd1Par30 = 0;
               par.vdencCmd1Par31 = 0;
               par.vdencCmd1Par32 = 0;
               par.vdencCmd1Par33 = 0;
               par.vdencCmd1Par34 = 21;
               par.vdencCmd1Par36 = 21;
               par.vdencCmd1Par37 = 23;
               par.vdencCmd1Par38 = 24;
               par.vdencCmd1Par39 = 27;
               par.vdencCmd1Par40 = 41;
               par.vdencCmd1Par41 = 68;
               par.vdencCmd1Par42 = 37;
               par.vdencCmd1Par43 = 37;
               par.vdencCmd1Par44 = 3;
               par.vdencCmd1Par45 = 12;
               par.vdencCmd1Par46 = 12;
               par.vdencCmd1Par87 = 20;
               par.vdencCmd1Par88 = 20;
               par.vdencCmd1Par89 = 20;
           }

           return MOS_STATUS_SUCCESS;
        }
    };

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
