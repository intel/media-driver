/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     codechal_vdenc_hevc.cpp
//! \brief    Defines base class for HEVC VDEnc encoder.
//!

#include "codechal_vdenc_hevc.h"

//!< \cond SKIP_DOXYGEN
const uint8_t CodechalVdencHevcState::m_estRateThreshP0[7] =
{
    4, 8, 12, 16, 20, 24, 28
};

const uint8_t CodechalVdencHevcState::m_estRateThreshB0[7] =
{
    4, 8, 12, 16, 20, 24, 28
};

const uint8_t CodechalVdencHevcState::m_estRateThreshI0[7] =
{
    4, 8, 12, 16, 20, 24, 28
};

const int8_t CodechalVdencHevcState::m_instRateThreshP0[4] =
{
    40, 60, 80, 120
};

const int8_t CodechalVdencHevcState::m_instRateThreshB0[4] =
{
    35, 60, 80, 120
};

const int8_t CodechalVdencHevcState::m_instRateThreshI0[4] =
{
    40, 60, 90, 115
};

const uint16_t CodechalVdencHevcState::m_startGAdjFrame[4] =
{
    10, 50, 100, 150
};

const uint8_t CodechalVdencHevcState::m_startGAdjMult[5] =
{
    1, 1, 3, 2, 1
};

const uint8_t CodechalVdencHevcState::m_startGAdjDiv[5] =
{
    40, 5, 5, 3, 1
};

const uint8_t CodechalVdencHevcState::m_rateRatioThreshold[7] =
{
    40, 75, 97, 103, 125, 160, 0
};

const uint8_t CodechalVdencHevcState::m_rateRatioThresholdQP[8] =
{
    253, 254, 255, 0, 1, 2, 3, 0
};

const uint32_t CodechalVdencHevcState::m_hucModeCostsIFrame[] = {
    0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e,
    0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e,
    0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e,
    0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e,
    0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e,
    0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e,
    0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x0d0e101e, 0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e,
    0x00320707, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x0d0e101e, 0x00320707, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalVdencHevcState::m_hucModeCostsPbFrame[] = {
    0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e,
    0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707,
    0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314,
    0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15,
    0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d,
    0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333,
    0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b,
    0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e,
    0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707,
    0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314,
    0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15,
    0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d,
    0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333,
    0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b,
    0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e,
    0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707,
    0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314,
    0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15,
    0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d,
    0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333,
    0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b,
    0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e,
    0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707,
    0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314,
    0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15,
    0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d,
    0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333,
    0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b,
    0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e,
    0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707,
    0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314,
    0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15,
    0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d,
    0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333,
    0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b,
    0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e,
    0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707,
    0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314,
    0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15,
    0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d,
    0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333,
    0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b,
    0x0d0e101e, 0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e,
    0x44320707, 0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707,
    0x15232314, 0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b, 0x0d0e101e, 0x44320707, 0x15232314,
    0x6e4d3f15, 0x04476e4d, 0x1f232333, 0x0f13131b
};

const uint16_t CodechalVdencHevcState::m_sadQpLambdaI[] = {
    0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0004, 0x0004,
    0x0005, 0x0006, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000E, 0x0010, 0x0012, 0x0014, 0x0016, 0x0019, 0x001C,
    0x001F, 0x0023, 0x0027, 0x002C, 0x0032, 0x0038, 0x003E, 0x0046, 0x004F, 0x0058, 0x0063, 0x006F, 0x007D, 0x008C, 0x009D, 0x00B1,
    0x00C6, 0x00DF, 0x00FA, 0x0118
};

// new table for visual quality improvement
const uint16_t CodechalVdencHevcState::m_sadQpLambdaI_VQI[] = {
    0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0004, 0x0004,
    0x0005, 0x0006, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000D, 0x000F, 0x0011, 0x0014, 0x0017, 0x001A, 0x001E, 0x0022,
    0x0027, 0x002D, 0x0033, 0x003B, 0x0043, 0x004D, 0x0057, 0x0064, 0x0072, 0x0082, 0x0095, 0x00A7, 0x00BB, 0x00D2, 0x00EC, 0x0109,
    0x0129, 0x014E, 0x0177, 0x01A5
};

const uint16_t CodechalVdencHevcState::m_sadQpLambdaP[] = {
    0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0004, 0x0004, 0x0005,
    0x0005, 0x0006, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000D, 0x000E, 0x0010, 0x0012, 0x0014, 0x0017, 0x001A, 0x001D,
    0x0021, 0x0024, 0x0029, 0x002E, 0x0034, 0x003A, 0x0041, 0x0049, 0x0052, 0x005C, 0x0067, 0x0074, 0x0082, 0x0092, 0x00A4, 0x00B8,
    0x00CE, 0x00E8, 0x0104, 0x0124
};

const uint16_t CodechalVdencHevcState::m_rdQpLambdaI[] = {
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0003, 0x0004, 0x0005,
    0x0006, 0x0008, 0x000A, 0x000C, 0x000F, 0x0013, 0x0018, 0x001E, 0x0026, 0x0030, 0x003D, 0x004D, 0x0061, 0x007A, 0x009A, 0x00C2,
    0x00F4, 0x0133, 0x0183, 0x01E8, 0x0266, 0x0306, 0x03CF, 0x04CD, 0x060C, 0x079F, 0x099A, 0x0C18, 0x0F3D, 0x1333, 0x1831, 0x1E7A,
    0x2666, 0x3062, 0x3CF5, 0x4CCD
};

const uint16_t CodechalVdencHevcState::m_rdQpLambdaP[] = {
    0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0004, 0x0005,
    0x0007, 0x0008, 0x000A, 0x000D, 0x0011, 0x0015, 0x001A, 0x0021, 0x002A, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00A6, 0x00D2,
    0x0108, 0x014D, 0x01A3, 0x0210, 0x029A, 0x0347, 0x0421, 0x0533, 0x068D, 0x0841, 0x0A66, 0x0D1A, 0x1082, 0x14CD, 0x1A35, 0x2105,
    0x299A, 0x346A, 0x4209, 0x5333
};

// Originial CodechalVdencHevcState::m_penaltyForIntraNonDC32x32PredMode table
const uint8_t CodechalVdencHevcState::m_penaltyForIntraNonDC32x32PredMode[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};

// New table for visual quality improvement
const uint8_t CodechalVdencHevcState::m_penaltyForIntraNonDC32x32PredMode_VQI[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x0C, 0x12, 0x19, 0x1f, 0x25, 0x2C, 0x32, 0x38,
    0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F,
    0x3F, 0x3F, 0x3F, 0x3F
};
//! \endcond

uint32_t CodechalVdencHevcState::GetMaxAllowedSlices(uint8_t levelIdc)
{
    uint32_t maxAllowedNumSlices = 0;

    switch (levelIdc)
    {
    case 10:
    case 20:
        maxAllowedNumSlices = 16;
        break;
    case 21:
        maxAllowedNumSlices = 20;
        break;
    case 30:
        maxAllowedNumSlices = 30;
        break;
    case 31:
        maxAllowedNumSlices = 40;
        break;
    case 40:
    case 41:
        maxAllowedNumSlices = 75;
        break;
    case 50:
    case 51:
    case 52:
        maxAllowedNumSlices = 200;
        break;
    case 60:
    case 61:
    case 62:
        maxAllowedNumSlices = 600;
        break;
    default:
        maxAllowedNumSlices = 0;
        break;
    }

    return maxAllowedNumSlices;
}

void CodechalVdencHevcState::SetPakPassType()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // default: VDEnc+PAK pass
    m_pakOnlyPass = false;

    // BRC
    if (m_brcEnabled)
    {
        // BRC with SSC, BRC without SSC
        // BRC fast 2nd pass needed, but weighted prediction/SSC 2nd pass not needed
        // HuC will update PAK pass type to be VDEnc+PAK if WP/SSC 2nd pass is needed
        if (GetCurrentPass() == 1)
        {
            m_pakOnlyPass = true;
        }
    }

    // CQP, ACQP, BRC
    if (m_hevcSeqParams->SAO_enabled_flag)
    {
        // SAO 2nd pass is always PAK only pass
        if (m_b2NdSaoPassNeeded && (GetCurrentPass() == m_uc2NdSaoPass))
        {
            m_pakOnlyPass = true;
        }
    }

    return;
}

void CodechalVdencHevcState::ComputeVDEncInitQP(int32_t& initQPIP, int32_t& initQPB)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    const float x0 = 0, y0 = 1.19f, x1 = 1.75f, y1 = 1.75f;
    uint32_t frameSize = ((m_frameWidth * m_frameHeight * 3) >> 1);

    initQPIP = (int)(1. / 1.2 * pow(10.0, (log10(frameSize * 2. / 3. * ((float)m_hevcSeqParams->FrameRate.Numerator / ((float)m_hevcSeqParams->FrameRate.Denominator * (float)m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS))) - x0) * (y1 - y0) / (x1 - x0) + y0) + 0.5);

    initQPIP += 2;

    int32_t gopP    = (m_hevcSeqParams->GopRefDist) ? ((m_hevcSeqParams->GopPicSize - 1) / m_hevcSeqParams->GopRefDist) : 0;
    int32_t gopB    = m_hevcSeqParams->GopPicSize - 1 - gopP;
    int32_t gopB1 = 0;
    int32_t gopB2 = 0;
    int32_t gopSize = 1 + gopP + gopB + gopB1 + gopB2;

    if (gopSize == 1)
    {
        initQPIP += 12;
    }
    else if (gopSize < 15)
    {
        initQPIP += ((14 - gopSize) >> 1);
    }

    initQPIP = CodecHal_Clip3((int32_t)m_hevcPicParams->BRCMinQp, (int32_t)m_hevcPicParams->BRCMaxQp, initQPIP);
    initQPIP--;

    if (initQPIP < 0)
    {
        initQPIP = 1;
    }

    initQPB = ((initQPIP + initQPIP) * 563 >> 10) + 1;
    initQPB = CodecHal_Clip3((int32_t)m_hevcPicParams->BRCMinQp, (int32_t)m_hevcPicParams->BRCMaxQp, initQPB);

    if (gopSize > 300)  //if intra frame is not inserted frequently
    {
        initQPIP -= 8;
        initQPB -= 8;
    }
    else
    {
        initQPIP -= 2;
        initQPB -= 2;
    }

    initQPIP = CodecHal_Clip3((int32_t)m_hevcPicParams->BRCMinQp, (int32_t)m_hevcPicParams->BRCMaxQp, initQPIP);
    initQPB  = CodecHal_Clip3((int32_t)m_hevcPicParams->BRCMinQp, (int32_t)m_hevcPicParams->BRCMaxQp, initQPB);
}

MOS_STATUS CodechalVdencHevcState::StoreHuCStatus2Register(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    // Write HUC_STATUS2 mask - bit 6 - valid IMEM loaded
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = &m_resHucStatus2Buffer;
    storeDataParams.dwResourceOffset = 0;
    storeDataParams.dwValue = m_hucInterface->GetHucStatus2ImemLoadedMask();
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    // Store HUC_STATUS2 register
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_resHucStatus2Buffer;
    storeRegParams.dwOffset = sizeof(uint32_t);
    storeRegParams.dwRegister = m_hucInterface->GetMmioRegisters(m_vdboxIndex)->hucStatus2RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &storeRegParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::HuCBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if ((!m_singleTaskPhaseSupported || m_firstTaskInPhase) )
    {
        // Send command buffer header at the beginning (OS dependent)
        bool requestFrameTracking = m_singleTaskPhaseSupported ?
            m_firstTaskInPhase : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
    }

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_vdboxHucHevcBrcInitKernelDescriptor;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(&cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCBrcInitReset());

    // set HuC DMEM param
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_vdencBrcInitDmemBuffer[m_currRecycledBufIdx];
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(m_vdencBrcInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(&cmdBuffer, &dmemParams));

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    virtualAddrParams.regionParams[0].presRegion = &m_vdencBrcHistoryBuffer;
    virtualAddrParams.regionParams[0].isWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(&cmdBuffer, &virtualAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcDummyStreamObject(&cmdBuffer));

    // Store HUC_STATUS2 register bit 6 before HUC_Start command
    // BitField: VALID IMEM LOADED - This bit will be cleared by HW at the end of a HUC workload
    // (HUC_Start command with last start bit set).
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreHuCStatus2Register(&cmdBuffer));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    if (!m_singleTaskPhaseSupported && (m_osInterface->bNoParsingAssistanceInKmd))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported)
    {
        bool renderingFlags = m_videoContextUsesNullHw;

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN( m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_MEDIA_STATE_BRC_INIT_RESET,
            "ENC")));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderingFlags));
    }

    CODECHAL_DEBUG_TOOL(DumpHucBrcInit());
    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::SetupBRCROIStreamIn(PMOS_RESOURCE streamIn, PMOS_RESOURCE deltaQpBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(streamIn);
    CODECHAL_ENCODE_CHK_NULL_RETURN(deltaQpBuffer);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    PDeltaQpForROI deltaQpData = (PDeltaQpForROI)m_osInterface->pfnLockResource(
        m_osInterface,
        deltaQpBuffer,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(deltaQpData);

    MOS_ZeroMemory(deltaQpData, m_deltaQpRoiBufferSize);

    uint32_t streamInWidth = (MOS_ALIGN_CEIL(m_frameWidth, 64) / 32);
    uint32_t streamInHeight = (MOS_ALIGN_CEIL(m_frameHeight, 64) / 32);
    uint32_t deltaQpBufWidth = (MOS_ALIGN_CEIL(m_frameWidth, 32) / 32);
    uint32_t deltaQpBufHeight = (MOS_ALIGN_CEIL(m_frameHeight, 32) / 32);
    bool cu64Align = true;

    for (auto i = m_hevcPicParams->NumROI - 1; i >= 0; i--)
    {
        //Check if the region is with in the borders
        uint16_t top    = (uint16_t)CodecHal_Clip3(0, (deltaQpBufHeight - 1), m_hevcPicParams->ROI[i].Top);
        uint16_t bottom = (uint16_t)CodecHal_Clip3(0, deltaQpBufHeight, m_hevcPicParams->ROI[i].Bottom);
        uint16_t left   = (uint16_t)CodecHal_Clip3(0, (deltaQpBufWidth - 1), m_hevcPicParams->ROI[i].Left);
        uint16_t right  = (uint16_t)CodecHal_Clip3(0, deltaQpBufWidth, m_hevcPicParams->ROI[i].Right);

        //Check if all the sides of ROI regions are aligned to 64CU
        if ((top % 2 == 1) || (bottom % 2 == 1) || (left % 2 == 1) || (right % 2 == 1))
        {
            cu64Align = false;
        }

        SetBrcRoiDeltaQpMap(streamInWidth, top, bottom, left, right, (uint8_t)i, deltaQpData);
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        deltaQpBuffer);

    uint8_t* data = (uint8_t*) m_osInterface->pfnLockResource(
        m_osInterface,
        streamIn,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminDataParams;
    MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
    streaminDataParams.maxTuSize = 3;    //Maximum TU Size allowed, restriction to be set to 3
    streaminDataParams.maxCuSize = (cu64Align) ? 3 : 2;
    switch (m_hevcSeqParams->TargetUsage)
    {
    case 1:
    case 4:
        streaminDataParams.numMergeCandidateCu64x64 = 4;
        streaminDataParams.numMergeCandidateCu32x32 = 3;
        streaminDataParams.numMergeCandidateCu16x16 = 2;
        streaminDataParams.numMergeCandidateCu8x8   = 1;
        streaminDataParams.numImePredictors         = m_imgStateImePredictors;
        break;
    case 7:
        streaminDataParams.numMergeCandidateCu64x64 = 2;
        streaminDataParams.numMergeCandidateCu32x32 = 2;
        streaminDataParams.numMergeCandidateCu16x16 = 2;
        streaminDataParams.numMergeCandidateCu8x8   = 0;
        streaminDataParams.numImePredictors         = 4;
        break;
    }

    int32_t streamInNumCUs = streamInWidth * streamInHeight;
    for (auto i = 0; i < streamInNumCUs; i++)
    {
        SetStreaminDataPerLcu(&streaminDataParams, data+(i*64));
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        streamIn);

    return eStatus;
}

void CodechalVdencHevcState::SetBrcRoiDeltaQpMap(
    uint32_t streamInWidth,
    uint32_t top,
    uint32_t bottom,
    uint32_t left,
    uint32_t right,
    uint8_t regionId,
    PDeltaQpForROI deltaQpMap)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    for (auto y = top; y < bottom; y++)
    {
        for (auto x = left; x < right; x++)
        {
            uint32_t offset = 0, xyOffset = 0;
            StreaminZigZagToLinearMap(streamInWidth, x, y, &offset, &xyOffset);

            (deltaQpMap + (offset + xyOffset))->iDeltaQp = m_hevcPicParams->ROI[regionId].PriorityLevelOrDQp;
        }
    }
}

void CodechalVdencHevcState::ProcessRoiDeltaQp()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Intialize ROIDistinctDeltaQp to be min expected delta qp, setting to -128
    // Check if forceQp is needed or not
    // forceQp is enabled if there are greater than 3 distinct delta qps or if the deltaqp is beyond range (-8, 7)

    for (auto k = 0; k < m_maxNumROI; k++)
    {
        m_hevcPicParams->ROIDistinctDeltaQp[k] = -128;
    }

    int32_t numQp = 0;
    for (int32_t i = 0; i < m_hevcPicParams->NumROI; i++)
    {
        bool dqpNew = true;

        //Get distinct delta Qps among all ROI regions, index 0 having the lowest delta qp
        int32_t k = numQp - 1;
        for (; k >= 0; k--)
        {
            if (m_hevcPicParams->ROI[i].PriorityLevelOrDQp == m_hevcPicParams->ROIDistinctDeltaQp[k] || m_hevcPicParams->ROI[i].PriorityLevelOrDQp == 0)
            {
                dqpNew = false;
                break;
            }
            else if (m_hevcPicParams->ROI[i].PriorityLevelOrDQp < m_hevcPicParams->ROIDistinctDeltaQp[k])
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (dqpNew)
        {
            for (int32_t j = numQp - 1; (j >= k + 1 && j >= 0); j--)
            {
                m_hevcPicParams->ROIDistinctDeltaQp[j + 1] = m_hevcPicParams->ROIDistinctDeltaQp[j];
            }
            m_hevcPicParams->ROIDistinctDeltaQp[k + 1] = m_hevcPicParams->ROI[i].PriorityLevelOrDQp;
            numQp++;
        }
    }

    //Set the ROI DeltaQp to zero for remaining array elements
    for (auto k = numQp; k < m_maxNumROI; k++)
    {
        m_hevcPicParams->ROIDistinctDeltaQp[k] = 0;
    }

    m_vdencNativeROIEnabled = !(numQp > m_maxNumNativeROI || m_hevcPicParams->ROIDistinctDeltaQp[0] < -8 || m_hevcPicParams->ROIDistinctDeltaQp[numQp - 1] > 7);
}

MOS_STATUS CodechalVdencHevcState::SetupROIStreamIn(PMOS_RESOURCE streamIn)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(streamIn);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        streamIn,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    uint32_t streamInWidth = (MOS_ALIGN_CEIL(m_frameWidth, 64) / 32);
    uint32_t streamInHeight = (MOS_ALIGN_CEIL(m_frameHeight, 64) / 32);
    int32_t streamInNumCUs = streamInWidth * streamInHeight;

    MOS_ZeroMemory(data, streamInNumCUs * 64);

    MHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminDataParams;

    //ROI higher priority for smaller index.
    bool cu64Align = true;
    for (int32_t i = m_hevcPicParams->NumROI - 1; i >= 0; i--)
    {

        //Check if the region is with in the borders
        uint16_t top    = (uint16_t)CodecHal_Clip3(0, (streamInHeight - 1), m_hevcPicParams->ROI[i].Top);
        uint16_t bottom = (uint16_t)CodecHal_Clip3(0, streamInHeight, m_hevcPicParams->ROI[i].Bottom);
        uint16_t left   = (uint16_t)CodecHal_Clip3(0, (streamInWidth - 1), m_hevcPicParams->ROI[i].Left);
        uint16_t right  = (uint16_t)CodecHal_Clip3(0, streamInWidth, m_hevcPicParams->ROI[i].Right);

        //Check if all the sides of ROI regions are aligned to 64CU
        if ((top % 2 == 1) || (bottom % 2 == 1) || (left % 2 == 1) || (right % 2 == 1))
        {
            cu64Align = false;
        }

        // For native ROI, determine Region ID based on distinct delta Qps and set ROI control
        uint32_t roiCtrl = 0;
        for (auto j = 0; j < m_maxNumNativeROI; j++)
        {
            if (m_hevcPicParams->ROIDistinctDeltaQp[j] == m_hevcPicParams->ROI[i].PriorityLevelOrDQp)
            {
                //All four 16x16 blocks within the 32x32 blocks should share the same region ID j
                roiCtrl = j + 1;
                for (auto k = 0; k < 3; k++)
                {
                    roiCtrl = roiCtrl << 2;
                    roiCtrl = roiCtrl + j + 1;
                }
                break;
            }
        }
        // Calculate ForceQp
        int8_t forceQp = (int8_t)CodecHal_Clip3(10, 51, m_hevcPicParams->QpY + m_hevcPicParams->ROI[i].PriorityLevelOrDQp + m_hevcSliceParams->slice_qp_delta);

        MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
        streaminDataParams.setQpRoiCtrl = true;
        if (m_vdencNativeROIEnabled)
        {
            streaminDataParams.roiCtrl = (uint8_t)roiCtrl;
        }
        else
        {
            streaminDataParams.forceQp = forceQp;
        }

        SetStreaminDataPerRegion(streamInWidth, top, bottom, left, right, &streaminDataParams, data);
    }

    MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
    streaminDataParams.maxTuSize = 3;    //Maximum TU Size allowed, restriction to be set to 3
    streaminDataParams.maxCuSize = (cu64Align) ? 3 : 2;
    switch (m_hevcSeqParams->TargetUsage)
    {
    case 1:
    case 4:
        streaminDataParams.numMergeCandidateCu64x64 = 4;
        streaminDataParams.numMergeCandidateCu32x32 = 3;
        streaminDataParams.numMergeCandidateCu16x16 = 2;
        streaminDataParams.numMergeCandidateCu8x8   = 1;
        streaminDataParams.numImePredictors         = m_imgStateImePredictors;
        break;
    case 7:
        streaminDataParams.numMergeCandidateCu64x64 = 2;
        streaminDataParams.numMergeCandidateCu32x32 = 2;
        streaminDataParams.numMergeCandidateCu16x16 = 2;
        streaminDataParams.numMergeCandidateCu8x8   = 0;
        streaminDataParams.numImePredictors         = 4;
        break;
    }

    for (auto i = 0; i < streamInNumCUs; i++)
    {
        SetStreaminDataPerLcu(&streaminDataParams, data + (i * 64));
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        streamIn);

    return eStatus;
}

void CodechalVdencHevcState::StreaminSetDirtyRectRegion(
    uint32_t streamInWidth,
    uint32_t top,
    uint32_t bottom,
    uint32_t left,
    uint32_t right,
    uint8_t  maxcu,
    void* streaminData)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminDataParams;
    MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
    streaminDataParams.maxTuSize = 3;
    streaminDataParams.maxCuSize = maxcu;
    streaminDataParams.puTypeCtrl = 0;

    switch (m_hevcSeqParams->TargetUsage)
    {
    case 1:
    case 4:
        streaminDataParams.numMergeCandidateCu64x64 = 4;
        streaminDataParams.numMergeCandidateCu32x32 = 3;
        streaminDataParams.numMergeCandidateCu16x16 = 2;
        streaminDataParams.numMergeCandidateCu8x8 = 1;
        streaminDataParams.numImePredictors = m_imgStateImePredictors;
        break;
    case 7:
        streaminDataParams.numMergeCandidateCu64x64 = 2;
        streaminDataParams.numMergeCandidateCu32x32 = 2;
        streaminDataParams.numMergeCandidateCu16x16 = 2;
        streaminDataParams.numMergeCandidateCu8x8 = 0;
        streaminDataParams.numImePredictors = 4;
        break;
    }

    SetStreaminDataPerRegion(streamInWidth, top, bottom, left, right, &streaminDataParams, streaminData);
}

void CodechalVdencHevcState::SetStreaminDataPerRegion(
    uint32_t streamInWidth,
    uint32_t top,
    uint32_t bottom,
    uint32_t left,
    uint32_t right,
    PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
    void* streaminData)
{
    uint8_t* data = (uint8_t*)streaminData;

    for (auto y = top; y < bottom; y++)
    {
        for (auto x = left; x < right; x++)
        {
            //Calculate X Y for the zig zag scan
            uint32_t offset = 0, xyOffset = 0;
            StreaminZigZagToLinearMap(streamInWidth, x, y, &offset, &xyOffset);

            SetStreaminDataPerLcu(streaminParams, data + (offset + xyOffset) * 64);
        }
    }
}

void CodechalVdencHevcState::StreaminZigZagToLinearMap(
    uint32_t streamInWidth,
    uint32_t x,
    uint32_t y,
    uint32_t* offset,
    uint32_t* xyOffset)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    *offset = streamInWidth * y;
    uint32_t yOffset = 0;
    uint32_t xOffset = 2 * x;

    //Calculate X Y Offset for the zig zag scan with in each 64x64 LCU
    //dwOffset gives the 64 LCU row
    if (y % 2)
    {
        *offset = streamInWidth * (y - 1);
        yOffset = 2;
    }

    if (x % 2)
    {
        xOffset = (2 * x) - 1;
    }

    *xyOffset = xOffset + yOffset;
}

void CodechalVdencHevcState::StreaminSetBorderNon64AlignStaticRegion(
    uint32_t streamInWidth,
    uint32_t top,
    uint32_t bottom,
    uint32_t left,
    uint32_t right,
    void* streaminData)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminDataParams;
    MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
    streaminDataParams.maxTuSize = 3;
    streaminDataParams.maxCuSize = 2;
    streaminDataParams.numMergeCandidateCu64x64 = 0; // MergeCand setting for Force MV
    streaminDataParams.numMergeCandidateCu32x32 = 1; // this is always set to 1
    streaminDataParams.numMergeCandidateCu16x16 = 0;
    streaminDataParams.numMergeCandidateCu8x8 = 0;
    streaminDataParams.numImePredictors = 0;
    streaminDataParams.puTypeCtrl = 0xff; //Force MV

    SetStreaminDataPerRegion(streamInWidth, top, bottom, left, right, &streaminDataParams, streaminData);
}

MOS_STATUS CodechalVdencHevcState::SetupDirtyRectStreamIn(PMOS_RESOURCE streamIn)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(streamIn);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        streamIn,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    uint32_t streamInWidth = (MOS_ALIGN_CEIL(m_frameWidth, 64) / 32);
    uint32_t streamInHeight = (MOS_ALIGN_CEIL(m_frameHeight, 64) / 32);
    int32_t streamInNumCUs = streamInWidth * streamInHeight;

    MOS_ZeroMemory(data, streamInNumCUs * 64);

    MHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminDataParams;
    MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
    streaminDataParams.maxTuSize = 3;
    streaminDataParams.maxCuSize = 3;
    streaminDataParams.numImePredictors = 0;
    streaminDataParams.puTypeCtrl = 0xff; //Force MV
    streaminDataParams.numMergeCandidateCu64x64 = 1; // MergeCand setting for Force MV
    streaminDataParams.numMergeCandidateCu32x32 = 0; // this is always set to 1
    streaminDataParams.numMergeCandidateCu16x16 = 0;
    streaminDataParams.numMergeCandidateCu8x8 = 0;

    for (auto i = 0; i < streamInNumCUs; i++)
    {
        SetStreaminDataPerLcu(&streaminDataParams, data + (i * 64));
    }

    uint32_t streamInWidthNo64Align  = (MOS_ALIGN_CEIL(m_frameWidth, 32) / 32);
    uint32_t streamInHeightNo64Align = (MOS_ALIGN_CEIL(m_frameHeight, 32) / 32);

    bool bActualWidth32Align  = (m_frameWidth % 32) == 0;
    bool bActualHeight32Align = (m_frameHeight % 32) == 0;

    // Set the static region when the width is not 64 CU aligned.
    if (streamInWidthNo64Align != streamInWidth || !bActualWidth32Align)
    {
        auto border_top    = 0;
        auto border_bottom = streamInHeight;
        auto border_left   = streamInWidthNo64Align - 1;
        auto border_right  = streamInWidth;

        if (!bActualWidth32Align)
        {
            StreaminSetDirtyRectRegion(streamInWidth, border_top, border_bottom, border_left, border_right, 3, data);
            if (streamInWidthNo64Align == streamInWidth)
            {
                StreaminSetBorderNon64AlignStaticRegion(streamInWidth, border_top, border_bottom, border_left-1, border_right-1, data);
            }
        }
        else
        {
            StreaminSetBorderNon64AlignStaticRegion(streamInWidth, border_top, border_bottom, border_left, border_right, data);
        }
    }

    // Set the static region when the height is not 64 CU aligned.
    if (streamInHeightNo64Align != streamInHeight || !bActualHeight32Align)
    {
        auto border_top    = streamInHeightNo64Align - 1;
        auto border_bottom = streamInHeight;
        auto border_left   = 0;
        auto border_right  = streamInWidth;

        if (!bActualHeight32Align)
        {
            StreaminSetDirtyRectRegion(streamInWidth, border_top, border_bottom, border_left, border_right, 3, data);
            if (streamInHeightNo64Align == streamInHeight)
            {
                StreaminSetBorderNon64AlignStaticRegion(streamInWidth, border_top - 1, border_bottom - 1, border_left, border_right, data);
            }                
        }
        else
        {
            StreaminSetBorderNon64AlignStaticRegion(streamInWidth, border_top, border_bottom, border_left, border_right, data);
        }
    }

    for (int i = m_hevcPicParams->NumDirtyRects - 1; i >= 0; i--)
    {
        //Check if the region is with in the borders
        uint16_t top    = (uint16_t)CodecHal_Clip3(0, (streamInHeight - 1), m_hevcPicParams->pDirtyRect[i].Top);
        uint16_t bottom = (uint16_t)CodecHal_Clip3(0, (streamInHeight - 1), m_hevcPicParams->pDirtyRect[i].Bottom) + 1;
        uint16_t left   = (uint16_t)CodecHal_Clip3(0, (streamInWidth - 1), m_hevcPicParams->pDirtyRect[i].Left);
        uint16_t right  = (uint16_t)CodecHal_Clip3(0, (streamInWidth - 1), m_hevcPicParams->pDirtyRect[i].Right) + 1;

        auto dirtyrect_top = top;
        auto dirtyrect_bottom = bottom;
        auto dirtyrect_left = left;
        auto dirtyrect_right = right;

        //If the border of the DirtyRect is not aligned with 64 CU, different setting in the border
        if (top % 2 != 0)
        {
            auto border_top = top;
            auto border_bottom = top + 1;
            auto border_left = left;
            auto border_right = right;

            StreaminSetDirtyRectRegion(streamInWidth, border_top, border_bottom, border_left, border_right, 2, data);

            border_top = top - 1;
            border_bottom = top;
            border_left = (left % 2 != 0) ? left - 1 : left;
            border_right = (right % 2 != 0) ? right + 1 : right;

            StreaminSetBorderNon64AlignStaticRegion(streamInWidth, border_top, border_bottom, border_left, border_right, data);

            dirtyrect_top = top + 1;
        }

        if (bottom % 2 != 0)
        {
            auto border_top = bottom - 1;
            auto border_bottom = bottom;
            auto border_left = left;
            auto border_right = right;

            StreaminSetDirtyRectRegion(streamInWidth, border_top, border_bottom, border_left, border_right, 2, data);

            border_top = bottom;
            border_bottom = bottom + 1;
            border_left = (left % 2 != 0) ? left - 1 : left;
            border_right = (right % 2 != 0) ? right + 1 : right;

            StreaminSetBorderNon64AlignStaticRegion(streamInWidth, border_top, border_bottom, border_left, border_right, data);

            dirtyrect_bottom = bottom - 1;
        }

        if (left % 2 != 0)
        {
            auto border_top = top;
            auto border_bottom = bottom;
            auto border_left = left;
            auto border_right = left + 1;

            StreaminSetDirtyRectRegion(streamInWidth, border_top, border_bottom, border_left, border_right, 2, data);

            border_top = (top % 2 != 0) ? top - 1 : top;
            border_bottom = (bottom % 2 != 0) ? bottom + 1 : bottom;
            border_left = left - 1;
            border_right = left;

            StreaminSetBorderNon64AlignStaticRegion(streamInWidth, border_top, border_bottom, border_left, border_right, data);

            dirtyrect_left = left + 1;
        }

        if (right % 2 != 0)
        {
            auto border_top = top;
            auto border_bottom = bottom;
            auto border_left = right - 1;
            auto border_right = right;

            StreaminSetDirtyRectRegion(streamInWidth, border_top, border_bottom, border_left, border_right, 2, data);

            border_top = (top % 2 != 0) ? top - 1 : top;
            border_bottom = (bottom % 2 != 0) ? bottom + 1 : bottom;
            border_left = right;
            border_right = right + 1;

            StreaminSetBorderNon64AlignStaticRegion(streamInWidth, border_top, border_bottom, border_left, border_right, data);
            dirtyrect_right = right - 1;
        }

        StreaminSetDirtyRectRegion(streamInWidth, dirtyrect_top, dirtyrect_bottom, dirtyrect_left, dirtyrect_right, 3, data);
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        streamIn);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::SetRegionsHuCBrcUpdate(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    int32_t currentPass = GetCurrentPass();
    if (currentPass < 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    // Add Virtual addr
    MOS_ZeroMemory(virtualAddrParams, sizeof(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS));
    virtualAddrParams->regionParams[0].presRegion = &m_vdencBrcHistoryBuffer;                // Region 0 - History Buffer (Input/Output)
    virtualAddrParams->regionParams[0].isWritable = true;
    virtualAddrParams->regionParams[1].presRegion =
        (MOS_RESOURCE*)m_allocator->GetResource(m_standard, vdencStats);                     // Region 1  VDEnc Statistics Buffer (Input) - VDENC_HEVC_VP9_FRAME_BASED_STATISTICS_STREAMOUT
    virtualAddrParams->regionParams[2].presRegion = &m_resFrameStatStreamOutBuffer;          // Region 2  PAK Statistics Buffer (Input) - MFX_PAK_FRAME_STATISTICS
    virtualAddrParams->regionParams[3].presRegion = &m_vdencReadBatchBuffer[m_currRecycledBufIdx][currentPass];    // Region 3 - Input SLB Buffer (Input)
    virtualAddrParams->regionParams[4].presRegion = &m_vdencBrcConstDataBuffer[m_currRecycledBufIdx];              // Region 4 - Constant Data (Input)
    virtualAddrParams->regionParams[5].presRegion = &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].OsResource;  // Region 5 - Output SLB Buffer (Output)
    virtualAddrParams->regionParams[5].isWritable = true;
    virtualAddrParams->regionParams[6].presRegion = &m_dataFromPicsBuffer;                   // Region 6 - Data Buffer of Current and Reference Pictures for Weighted Prediction (Input/Output)
    virtualAddrParams->regionParams[6].isWritable = true;
    virtualAddrParams->regionParams[7].presRegion = &m_resLcuBaseAddressBuffer;  // Region 7  Slice Stat Streamout (Input)
    virtualAddrParams->regionParams[8].presRegion =
        (MOS_RESOURCE*)m_allocator->GetResource(m_standard, pakInfo);                        // Region 8 - PAK Information (Input)
    virtualAddrParams->regionParams[9].presRegion = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];          // Region 9 – Streamin Buffer for ROI (Input)
    virtualAddrParams->regionParams[10].presRegion = &m_vdencDeltaQpBuffer[m_currRecycledBufIdx];                  // Region 10 – Delta QP Buffer for ROI (Input)
    virtualAddrParams->regionParams[11].presRegion = &m_vdencOutputROIStreaminBuffer;        // Region 11 – Streamin Buffer for ROI (Output)
    virtualAddrParams->regionParams[11].isWritable = true;

    // region 15 always in clear
    virtualAddrParams->regionParams[15].presRegion = &m_vdencBrcDbgBuffer;                   // Region 15 - Debug Buffer (Output)
    virtualAddrParams->regionParams[15].isWritable = true;

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::HuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported || (m_firstTaskInPhase && !m_brcInit))
    {
        // Send command buffer header at the beginning (OS dependent)
        bool requestFrameTracking = m_singleTaskPhaseSupported ?
            m_firstTaskInPhase : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
    }

    int32_t currentPass = GetCurrentPass();
    if (currentPass < 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ConstructBatchBufferHuCBRC(&m_vdencReadBatchBuffer[m_currRecycledBufIdx][currentPass]));

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));

    if (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)  // Low Delay BRC
    {
        imemParams.dwKernelDescriptor = m_vdboxHucHevcBrcLowdelayKernelDescriptor;
    }
    else
    {
        imemParams.dwKernelDescriptor = m_vdboxHucHevcBrcUpdateKernelDescriptor;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(&cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    // DMEM set
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCBrcUpdate());

    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &(m_vdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][currentPass]);
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(&cmdBuffer, &dmemParams));

    // Set Const Data buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetConstDataHuCBrcUpdate());

    // Add Virtual addr
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetRegionsHuCBrcUpdate(&m_virtualAddrParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(&cmdBuffer, &m_virtualAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcDummyStreamObject(&cmdBuffer));

    // Store HUC_STATUS2 register bit 6 before HUC_Start command
    // BitField: VALID IMEM LOADED - This bit will be cleared by HW at the end of a HUC workload
    // (HUC_Start command with last start bit set).
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreHuCStatus2Register(&cmdBuffer));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Write HUC_STATUS mask: DW1 (mask value)
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = &m_resPakMmioBuffer;
    storeDataParams.dwResourceOffset = sizeof(uint32_t);
    storeDataParams.dwValue = CODECHAL_VDENC_HEVC_BRC_HUC_STATUS_REENCODE_MASK;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &storeDataParams));

    // store HUC_STATUS register: DW0 (actual value)
    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hucInterface->GetMmioRegisters(m_vdboxIndex);
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_resPakMmioBuffer;
    storeRegParams.dwOffset = 0;
    storeRegParams.dwRegister = mmioRegisters->hucStatusRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &storeRegParams));

    // DW0 & DW1 will considered together for conditional batch buffer end cmd later
    if ((!m_singleTaskPhaseSupported) && (m_osInterface->bNoParsingAssistanceInKmd))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    // HuC Input
    CODECHAL_DEBUG_TOOL(DumpHucBrcUpdate(true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported)
    {
        bool renderingFlags = m_videoContextUsesNullHw;

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN( m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_MEDIA_STATE_BRC_UPDATE,
            "ENC")));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderingFlags));
    }

    CODECHAL_DEBUG_TOOL(DumpHucBrcUpdate(false));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::HuCBrcDummyStreamObject(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // pass dummy buffer by Ind Obj Addr command
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjParams;
    MOS_ZeroMemory(&indObjParams, sizeof(indObjParams));
    indObjParams.presDataBuffer = &m_vdencBrcDbgBuffer;
    indObjParams.dwDataSize = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucIndObjBaseAddrStateCmd(cmdBuffer, &indObjParams));

    MHW_VDBOX_HUC_STREAM_OBJ_PARAMS streamObjParams;
    MOS_ZeroMemory(&streamObjParams, sizeof(streamObjParams));
    streamObjParams.dwIndStreamInLength = 1;
    streamObjParams.bHucProcessing = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStreamObjectCmd(cmdBuffer, &streamObjParams));

    return eStatus;
}

void CodechalVdencHevcState::SetVdencPipeModeSelectParams(
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& pipeModeSelectParams)
{
    pipeModeSelectParams.ucVdencBitDepthMinus8    = m_hevcSeqParams->bit_depth_luma_minus8;
    pipeModeSelectParams.bPakThresholdCheckEnable = m_hevcSeqParams->SliceSizeControl;
    pipeModeSelectParams.ChromaType               = m_hevcSeqParams->chroma_format_idc;
    pipeModeSelectParams.bTlbPrefetchEnable = true;
    pipeModeSelectParams.Format = m_rawSurfaceToPak->Format;

    // can be enabled by reg key (disabled by default)
    pipeModeSelectParams.bVdencPakObjCmdStreamOutEnable = m_vdencPakObjCmdStreamOutEnabled;

    int32_t currentPass = GetCurrentPass();

    // needs to be enabled for 1st pass in multi-pass case
    // This bit is ignored if PAK only second pass is enabled.
    if ((currentPass == 0) && (currentPass != m_numPasses))
    {
        pipeModeSelectParams.bVdencPakObjCmdStreamOutEnable = true;
    }
}

void CodechalVdencHevcState::SetVdencSurfaceStateParams(
    MHW_VDBOX_SURFACE_PARAMS& srcSurfaceParams,
    MHW_VDBOX_SURFACE_PARAMS& reconSurfaceParams,
    MHW_VDBOX_SURFACE_PARAMS& ds8xSurfaceParams,
    MHW_VDBOX_SURFACE_PARAMS& ds4xSurfaceParams)
{
    // VDENC_SRC_SURFACE_STATE parameters
    srcSurfaceParams.dwActualWidth        = ((m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
    srcSurfaceParams.dwActualHeight       = ((m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
    srcSurfaceParams.bColorSpaceSelection = (m_hevcSeqParams->InputColorSpace == ECOLORSPACE_P709) ? 1 : 0;

    // VDENC_REF_SURFACE_STATE parameters
    reconSurfaceParams.dwActualWidth = srcSurfaceParams.dwActualWidth;
    reconSurfaceParams.dwActualHeight = srcSurfaceParams.dwActualHeight;
    reconSurfaceParams.dwReconSurfHeight = m_rawSurfaceToPak->dwHeight;

    // VDENC_DS_REF_SURFACE_STATE parameters
    MOS_ZeroMemory(&ds8xSurfaceParams, sizeof(ds8xSurfaceParams));
    ds8xSurfaceParams.Mode = CODECHAL_ENCODE_MODE_HEVC;
    ds8xSurfaceParams.psSurface = m_trackedBuf->Get8xDsReconSurface(CODEC_CURR_TRACKED_BUFFER);
    ds8xSurfaceParams.ucSurfaceStateId = CODECHAL_MFX_DSRECON_SURFACE_ID;
    ds8xSurfaceParams.dwActualWidth = ds8xSurfaceParams.psSurface->dwWidth;
    ds8xSurfaceParams.dwActualHeight = ds8xSurfaceParams.psSurface->dwHeight;

    MOS_ZeroMemory(&ds4xSurfaceParams, sizeof(ds4xSurfaceParams));
    ds4xSurfaceParams.Mode = CODECHAL_ENCODE_MODE_HEVC;
    ds4xSurfaceParams.psSurface = m_trackedBuf->Get4xDsReconSurface(CODEC_CURR_TRACKED_BUFFER);
    ds4xSurfaceParams.ucSurfaceStateId = CODECHAL_MFX_DSRECON_SURFACE_ID;
    ds4xSurfaceParams.dwActualWidth = ds4xSurfaceParams.psSurface->dwWidth;
    ds4xSurfaceParams.dwActualHeight = ds4xSurfaceParams.psSurface->dwHeight;
}

void CodechalVdencHevcState::SetVdencPipeBufAddrParams(
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
{
    pipeBufAddrParams = {};
    pipeBufAddrParams.Mode = CODECHAL_ENCODE_MODE_HEVC;
    pipeBufAddrParams.psRawSurface = m_rawSurfaceToPak;
    pipeBufAddrParams.ps4xDsSurface = m_trackedBuf->Get4xDsReconSurface(CODEC_CURR_TRACKED_BUFFER);
    pipeBufAddrParams.ps8xDsSurface = m_trackedBuf->Get8xDsReconSurface(CODEC_CURR_TRACKED_BUFFER);
    pipeBufAddrParams.presVdencStreamOutBuffer = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, vdencStats);
    pipeBufAddrParams.dwVdencStatsStreamOutOffset =  0;
    pipeBufAddrParams.presVdencIntraRowStoreScratchBuffer = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, vdencIntraRowStoreScratch);
    pipeBufAddrParams.presVdencPakObjCmdStreamOutBuffer = m_resVdencPakObjCmdStreamOutBuffer = &m_resMbCodeSurface;
    pipeBufAddrParams.dwNumRefIdxL0ActiveMinus1                                              = m_hevcSliceParams->num_ref_idx_l0_active_minus1;
    pipeBufAddrParams.dwNumRefIdxL1ActiveMinus1                                              = m_hevcSliceParams->num_ref_idx_l1_active_minus1;

    if (m_vdencStreamInEnabled)
    {
        if (m_vdencHucUsed && m_hevcPicParams->NumROI && !m_vdencNativeROIEnabled)
        {
            pipeBufAddrParams.presVdencStreamInBuffer = &m_vdencOutputROIStreaminBuffer;
        }
        else
        {
            pipeBufAddrParams.presVdencStreamInBuffer = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
        }
    }

    PCODEC_PICTURE l0RefFrameList = m_hevcSliceParams->RefPicList[LIST_0];
    for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = l0RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
        {
            // L0 references
            uint8_t refPicIdx                             = m_picIdx[refPic.FrameIdx].ucPicIdx;
            pipeBufAddrParams.presVdencReferences[refIdx] = &m_refList[refPicIdx]->sRefReconBuffer.OsResource;

            // 4x/8x DS surface for VDEnc
            uint8_t scaledIdx                              = m_refList[refPicIdx]->ucScalingIdx;
            pipeBufAddrParams.presVdenc4xDsSurface[refIdx] = &(m_trackedBuf->Get4xDsReconSurface(scaledIdx))->OsResource;
            pipeBufAddrParams.presVdenc8xDsSurface[refIdx] = &(m_trackedBuf->Get8xDsReconSurface(scaledIdx))->OsResource;
        }
    }

    if (!m_lowDelay)
    {
        PCODEC_PICTURE l1RefFrameList = m_hevcSliceParams->RefPicList[LIST_1];
        for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l1_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = l1RefFrameList[refIdx];

            if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
            {
                // L1 references
                uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
                pipeBufAddrParams.presVdencReferences[refIdx + m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1] = 
                    &m_refList[refPicIdx]->sRefReconBuffer.OsResource;

                // 4x/8x DS surface for VDEnc
                uint8_t scaledIdx = m_refList[refPicIdx]->ucScalingIdx;
                pipeBufAddrParams.presVdenc4xDsSurface[refIdx + m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1] =
                    &(m_trackedBuf->Get4xDsReconSurface(scaledIdx))->OsResource;
                pipeBufAddrParams.presVdenc8xDsSurface[refIdx + m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1] = 
                    &(m_trackedBuf->Get8xDsReconSurface(scaledIdx))->OsResource;
            }
        }
    }

    uint8_t idxForTempMVP = 0xFF;

    if (m_hevcPicParams->CollocatedRefPicIndex != 0xFF && m_hevcPicParams->CollocatedRefPicIndex < CODEC_MAX_NUM_REF_FRAME_HEVC)
    {
        uint8_t frameIdx = m_hevcPicParams->RefFrameList[m_hevcPicParams->CollocatedRefPicIndex].FrameIdx;
        idxForTempMVP = m_refList[frameIdx]->ucScalingIdx;
    }

    if (idxForTempMVP == 0xFF && m_hevcSliceParams->slice_temporal_mvp_enable_flag)
    {
        // Temporal reference MV index is invalid and so disable the temporal MVP
        m_hevcSliceParams->slice_temporal_mvp_enable_flag = false;
    }
    else
    {
        pipeBufAddrParams.presColMvTempBuffer[0] = m_trackedBuf->GetMvTemporalBuffer(idxForTempMVP);
    }
}

void CodechalVdencHevcState::SetHcpSliceStateCommonParams(MHW_VDBOX_HEVC_SLICE_STATE& sliceStateParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncodeHevcBase::SetHcpSliceStateCommonParams(sliceStateParams);

    sliceStateParams.bVdencInUse = true;
    sliceStateParams.bVdencHucInUse     = m_hevcVdencAcqpEnabled || m_brcEnabled;
    sliceStateParams.bWeightedPredInUse = m_hevcVdencWeightedPredEnabled;
    sliceStateParams.pVdencBatchBuffer  = &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx];

    // This bit disables Top intra Reference pixel fetch in VDENC mode.
    // In PAK only second pass, this bit should be set to one.
    // "IntraRefFetchDisable" in HCP SLICE STATE should be set to 0 in first pass and 1 in subsequent passes.
    // For dynamic slice, 2nd pass is still VDEnc + PAK pass, not PAK only pass.
    sliceStateParams.bIntraRefFetchDisable = m_pakOnlyPass;
}

MOS_STATUS CodechalVdencHevcState::AddHcpPakInsertSliceHeader(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcSliceParams);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    // Insert slice header
    MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
    MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
    pakInsertObjectParams.bLastHeader = true;
    pakInsertObjectParams.bEmulationByteBitsInsert = true;
    pakInsertObjectParams.pBatchBufferForPakSlices = batchBuffer;

    // App does the slice header packing, set the skip count passed by the app
    pakInsertObjectParams.uiSkipEmulationCheckCount = params->uiSkipEmulationCheckCount;
    pakInsertObjectParams.pBsBuffer = params->pBsBuffer;
    pakInsertObjectParams.dwBitSize = params->dwLength;
    pakInsertObjectParams.dwOffset = params->dwOffset;
    pakInsertObjectParams.bVdencInUse = params->bVdencInUse;

    // For HEVC VDEnc Dynamic Slice
    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams = params->pEncodeHevcSliceParams;
    if (m_hevcSeqParams->SliceSizeControl)
    {
        pakInsertObjectParams.bLastHeader = false;
        pakInsertObjectParams.bEmulationByteBitsInsert = false;
        pakInsertObjectParams.dwBitSize = hevcSlcParams->BitLengthSliceHeaderStartingPortion;
        pakInsertObjectParams.bResetBitstreamStartingPos = true;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(
        cmdBuffer,
        &pakInsertObjectParams));

    if (m_hevcSeqParams->SliceSizeControl)
    {
        // Send HCP_PAK_INSERT_OBJ command. For dynamic slice, we are skipping the beginning part of slice header.
        pakInsertObjectParams.bLastHeader = true;
        pakInsertObjectParams.dwBitSize = params->dwLength - hevcSlcParams->BitLengthSliceHeaderStartingPortion;
        pakInsertObjectParams.dwOffset += ((hevcSlcParams->BitLengthSliceHeaderStartingPortion + 7) / 8);   // Skips the first 5 bytes which is Start Code + Nal Unit Header
        pakInsertObjectParams.bResetBitstreamStartingPos = true;
        pakInsertObjectParams.bVdencInUse = params->bVdencInUse;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(
            cmdBuffer,
            &pakInsertObjectParams));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::AddHcpWeightOffsetStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hevcSlcParams);

    MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS hcpWeightOffsetParams;
    MOS_ZeroMemory(&hcpWeightOffsetParams, sizeof(hcpWeightOffsetParams));

    for (auto k = 0; k < 2; k++) // k=0: LIST_0, k=1: LIST_1
    {
        // Luma, Chroma offset
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            hcpWeightOffsetParams.LumaOffsets[k][i] = (int16_t)hevcSlcParams->luma_offset[k][i];
            // Cb, Cr
            for (auto j = 0; j < 2; j++)
            {
                hcpWeightOffsetParams.ChromaOffsets[k][i][j] = (int16_t)hevcSlcParams->chroma_offset[k][i][j];
            }
        }

        // Luma Weight
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &hcpWeightOffsetParams.LumaWeights[k],
            sizeof(hcpWeightOffsetParams.LumaWeights[k]),
            &hevcSlcParams->delta_luma_weight[k],
            sizeof(hevcSlcParams->delta_luma_weight[k])));

        // Chroma Weight
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &hcpWeightOffsetParams.ChromaWeights[k],
            sizeof(hcpWeightOffsetParams.ChromaWeights[k]),
            &hevcSlcParams->delta_chroma_weight[k],
            sizeof(hevcSlcParams->delta_chroma_weight[k])));
    }

    if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_P_SLICE || hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
    {
        hcpWeightOffsetParams.ucList = LIST_0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(cmdBuffer, nullptr, &hcpWeightOffsetParams));
    }

    if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
    {
        hcpWeightOffsetParams.ucList = LIST_1;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(cmdBuffer, nullptr, &hcpWeightOffsetParams));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::AddVdencWeightOffsetStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hevcSlcParams);

    MHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS vdencWeightOffsetParams;
    MOS_ZeroMemory(&vdencWeightOffsetParams, sizeof(vdencWeightOffsetParams));

    vdencWeightOffsetParams.bWeightedPredEnabled = m_hevcVdencWeightedPredEnabled;

    if (vdencWeightOffsetParams.bWeightedPredEnabled)
    {
        vdencWeightOffsetParams.dwDenom = 1 << (hevcSlcParams->luma_log2_weight_denom);

        // Luma offset
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            vdencWeightOffsetParams.LumaOffsets[0][i] = (int16_t)hevcSlcParams->luma_offset[0][i];
            vdencWeightOffsetParams.LumaOffsets[1][i] = (int16_t)hevcSlcParams->luma_offset[1][i];
        }

        // Luma Weight
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
            &vdencWeightOffsetParams.LumaWeights[0],
            sizeof(vdencWeightOffsetParams.LumaWeights[0]),
            &hevcSlcParams->delta_luma_weight[0],
            sizeof(hevcSlcParams->delta_luma_weight[0])),
            "Failed to copy luma weight 0 memory.");

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
            &vdencWeightOffsetParams.LumaWeights[1],
            sizeof(vdencWeightOffsetParams.LumaWeights[1]),
            &hevcSlcParams->delta_luma_weight[1],
            sizeof(hevcSlcParams->delta_luma_weight[1])),
            "Failed to copy luma weight 1 memory.");
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWeightsOffsetsStateCmd(
        cmdBuffer,
        nullptr,
        &vdencWeightOffsetParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::AddVdencWalkerStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MHW_VDBOX_VDENC_WALKER_STATE_PARAMS vdencWalkerStateParams;
    vdencWalkerStateParams.Mode = CODECHAL_ENCODE_MODE_HEVC;
    vdencWalkerStateParams.pHevcEncSeqParams = params->pEncodeHevcSeqParams;
    vdencWalkerStateParams.pHevcEncPicParams = params->pEncodeHevcPicParams;
    vdencWalkerStateParams.pEncodeHevcSliceParams = params->pEncodeHevcSliceParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWalkerStateCmd(cmdBuffer, &vdencWalkerStateParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::ReadBrcPakStats(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    uint32_t offset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
        m_encodeStatusBuf.dwNumPassesOffset +   // Num passes offset
        sizeof(uint32_t) * 2;                               // encodeStatus is offset by 2 DWs in the resource

    EncodeReadBrcPakStatsParams   readBrcPakStatsParams;
    readBrcPakStatsParams.pHwInterface = m_hwInterface;
    readBrcPakStatsParams.presBrcPakStatisticBuffer = &m_vdencBrcBuffers.resBrcPakStatisticBuffer[m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite];
    readBrcPakStatsParams.presStatusBuffer = &m_encodeStatusBuf.resStatusBuffer;
    readBrcPakStatsParams.dwStatusBufNumPassesOffset = offset;
    readBrcPakStatsParams.ucPass = (uint8_t) GetCurrentPass();
    readBrcPakStatsParams.VideoContext = m_videoContext;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStatistics(
        cmdBuffer,
        &readBrcPakStatsParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::StoreVdencStatistics(PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t offset = sizeof(CodechalVdencHevcLaStats) * m_currLaDataIdx;
    MHW_MI_COPY_MEM_MEM_PARAMS miCpyMemMemParams;
    MOS_ZeroMemory(&miCpyMemMemParams, sizeof(MHW_MI_COPY_MEM_MEM_PARAMS));
    miCpyMemMemParams.presSrc = m_resVdencStatsBuffer; // 8X8 Normalized intra CU count is in m_resVdencStatsBuffer DW1
    miCpyMemMemParams.dwSrcOffset = 4;
    miCpyMemMemParams.presDst = &m_vdencLaStatsBuffer;
    miCpyMemMemParams.dwDstOffset = offset + CODECHAL_OFFSETOF(CodechalVdencHevcLaStats, intraCuCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(cmdBuffer, &miCpyMemMemParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::StoreLookaheadStatistics(PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex())                                                                         \
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);

    uint32_t offset = sizeof(CodechalVdencHevcLaStats) * m_currLaDataIdx;

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = &m_vdencLaStatsBuffer;
    miStoreRegMemParams.dwOffset = offset + CODECHAL_OFFSETOF(CodechalVdencHevcLaStats, frameByteCount);
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    // Calculate header size including LCU header
    uint32_t headerBitSize = 0;
    for (uint32_t i = 0; i < HEVC_MAX_NAL_UNIT_TYPE; i++)
    {
        headerBitSize += m_nalUnitParams[i]->uiSize * 8;
    }
    for (uint32_t i = 0; i < m_numSlices; i++)
    {
        headerBitSize += m_slcData[i].BitSize;
    }

    // Store to headerBitCount in CodechalVdencHevcLaStats
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    storeDataParams.pOsResource      = &m_vdencLaStatsBuffer;
    storeDataParams.dwResourceOffset = offset + CODECHAL_OFFSETOF(CodechalVdencHevcLaStats, headerBitCount);
    storeDataParams.dwValue          = headerBitSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    auto                            mmioRegistersMfx = m_mfxInterface->GetMmioRegisters(m_vdboxIndex);
    MHW_MI_LOAD_REGISTER_MEM_PARAMS miLoadRegMemParams;
    MHW_MI_FLUSH_DW_PARAMS          flushDwParams;
    MHW_MI_ATOMIC_PARAMS            atomicParams;

    MOS_ZeroMemory(&miLoadRegMemParams, sizeof(miLoadRegMemParams));
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    MOS_ZeroMemory(&atomicParams, sizeof(atomicParams));
    // VCS_GPR0_Lo = LCUHdrBits
    miLoadRegMemParams.presStoreBuffer = &m_resFrameStatStreamOutBuffer;  // LCUHdrBits is in m_resFrameStatStreamOutBuffer DW4
    miLoadRegMemParams.dwOffset        = 4 * sizeof(uint32_t);
    miLoadRegMemParams.dwRegister      = mmioRegistersMfx->generalPurposeRegister0LoOffset;  // VCS_GPR0_Lo
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &miLoadRegMemParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        cmdBuffer,
        &flushDwParams));

    // frame headerBitCount += LCUHdrBits
    atomicParams.pOsResource      = &m_vdencLaStatsBuffer;
    atomicParams.dwResourceOffset = offset + CODECHAL_OFFSETOF(CodechalVdencHevcLaStats, headerBitCount);
    atomicParams.dwDataSize       = sizeof(uint32_t);
    atomicParams.Operation        = MHW_MI_ATOMIC_ADD;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiAtomicCmd(
        cmdBuffer,
        &atomicParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreVdencStatistics(cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::ReadSliceSize(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    uint32_t baseOffset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize + sizeof(uint32_t) * 2);  // encodeStatus is offset by 2 DWs in the resource

    // Report slice size to app only when dynamic slice is enabled
    if (!m_hevcSeqParams->SliceSizeControl)
    {
        // Clear slice size report in EncodeStatus buffer
        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_encodeStatusBuf.resStatusBuffer, &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);
        EncodeStatus* dataStatus = (EncodeStatus*)(data + baseOffset);
        MOS_ZeroMemory(&(dataStatus->sliceReport), sizeof(EncodeStatusSliceReport));
        m_osInterface->pfnUnlockResource(m_osInterface, &m_encodeStatusBuf.resStatusBuffer);

        return eStatus;
    }

    uint32_t sizeOfSliceSizesBuffer = MOS_ALIGN_CEIL(CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);

    if (IsFirstPass())
    {
        // Create/ Initialize slice report buffer once per frame, to be used across passes
        if (Mos_ResourceIsNull(&m_resSliceReport[m_encodeStatusBuf.wCurrIndex]))
        {
            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format = Format_Buffer;
            allocParamsForBufferLinear.dwBytes = sizeOfSliceSizesBuffer;

            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_resSliceReport[m_encodeStatusBuf.wCurrIndex]),
                "Failed to create HEVC VDEnc Slice Report Buffer ");
        }

        // Clear slice size structure to be sent in EncodeStatusReport buffer
        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_resSliceReport[m_encodeStatusBuf.wCurrIndex], &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);
        MOS_ZeroMemory(data, sizeOfSliceSizesBuffer);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resSliceReport[m_encodeStatusBuf.wCurrIndex]);

        // Set slice size pointer in slice size structure
        data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface,(&m_encodeStatusBuf.resStatusBuffer), &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);
        EncodeStatus* dataStatus = (EncodeStatus*)(data + baseOffset);
        (dataStatus)->sliceReport.pSliceSize             = &m_resSliceReport[m_encodeStatusBuf.wCurrIndex];
        m_osInterface->pfnUnlockResource(m_osInterface, &m_encodeStatusBuf.resStatusBuffer);
    }

    // Copy Slize size data buffer from PAK to be sent back to App
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CopyDataBlock(cmdBuffer, 
        &m_resLcuBaseAddressBuffer, 0, &m_resSliceReport[m_encodeStatusBuf.wCurrIndex], 0, sizeOfSliceSizesBuffer));

    MHW_MI_COPY_MEM_MEM_PARAMS miCpyMemMemParams;
    MOS_ZeroMemory(&miCpyMemMemParams, sizeof(MHW_MI_COPY_MEM_MEM_PARAMS));
    miCpyMemMemParams.presSrc       = &m_resFrameStatStreamOutBuffer; // Slice size overflow is in m_resFrameStatStreamOutBuffer DW0[16]
    miCpyMemMemParams.dwSrcOffset   = 0;
    miCpyMemMemParams.presDst       = &m_encodeStatusBuf.resStatusBuffer;
    miCpyMemMemParams.dwDstOffset   = baseOffset + m_encodeStatusBuf.dwSliceReportOffset;     // Slice size overflow is at DW0 EncodeStatusSliceReport
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(cmdBuffer, &miCpyMemMemParams));


    MOS_ZeroMemory(&miCpyMemMemParams, sizeof(MHW_MI_COPY_MEM_MEM_PARAMS));
    miCpyMemMemParams.presSrc       = m_resSliceCountBuffer; // Number of slice sizes are stored in this buffer. Updated at runtime
    miCpyMemMemParams.dwSrcOffset   = 0;
    miCpyMemMemParams.presDst       = &m_encodeStatusBuf.resStatusBuffer;
    miCpyMemMemParams.dwDstOffset   = baseOffset + m_encodeStatusBuf.dwSliceReportOffset + 1;     // Num slices is located at DW1 EncodeStatusSliceReport
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(cmdBuffer, &miCpyMemMemParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::CopyDataBlock(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMOS_RESOURCE sourceSurface,
    uint32_t sourceOffset,
    PMOS_RESOURCE destSurface,
    uint32_t destOffset,
    uint32_t copySize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalHucStreamoutParams hucStreamOutParams;
    MOS_ZeroMemory(&hucStreamOutParams, sizeof(hucStreamOutParams));

    // Ind Obj Addr command
    hucStreamOutParams.dataBuffer            = sourceSurface;
    hucStreamOutParams.dataSize              = copySize + sourceOffset;
    hucStreamOutParams.dataOffset            = MOS_ALIGN_FLOOR(sourceOffset, CODECHAL_PAGE_SIZE);
    hucStreamOutParams.streamOutObjectBuffer = destSurface;
    hucStreamOutParams.streamOutObjectSize   = copySize + destOffset;
    hucStreamOutParams.streamOutObjectOffset = MOS_ALIGN_FLOOR(destOffset, CODECHAL_PAGE_SIZE);

    // Stream object params
    hucStreamOutParams.indStreamInLength     = copySize;
    hucStreamOutParams.inputRelativeOffset   = sourceOffset - hucStreamOutParams.dataOffset;
    hucStreamOutParams.outputRelativeOffset  = destOffset - hucStreamOutParams.streamOutObjectOffset;


    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->PerformHucStreamOut(
        &hucStreamOutParams,
        cmdBuffer));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC       = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC    = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::ExecutePictureLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    if (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex())                                                                         \
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifyCommandBufferSize());

    if (!m_singleTaskPhaseSupportedInPak)
    {
        // Command buffer or patch list size are too small and so we cannot submit multiple pass of PAKs together
        m_firstTaskInPhase = true;
        m_lastTaskInPhase = true;
    }

    // PAK pass type for each pass: VDEnc+PAK vs. PAK-only
    SetPakPassType();

    bool pakOnlyMultipassEnable;

    pakOnlyMultipassEnable = m_pakOnlyPass;

    bool panicEnabled = (m_brcEnabled) && (m_panicEnable) && (GetCurrentPass() == 1) && !m_pakOnlyPass;

    uint32_t rollingILimit = (m_hevcPicParams->bEnableRollingIntraRefresh == ROLLING_I_ROW) ? MOS_ROUNDUP_DIVIDE(m_frameHeight, 32) : (m_hevcPicParams->bEnableRollingIntraRefresh == ROLLING_I_COLUMN) ? MOS_ROUNDUP_DIVIDE(m_frameWidth, 32) : 0;

    m_refList[m_currReconstructedPic.FrameIdx]->rollingIntraRefreshedPosition =
        CodecHal_Clip3(0, rollingILimit, m_hevcPicParams->IntraInsertionLocation + m_hevcPicParams->IntraInsertionSize);

    // For ACQP / BRC, update pic params rolling intra reference location here before cmd buffer is prepared.
    PCODEC_PICTURE l0RefFrameList = m_hevcSliceParams->RefPicList[LIST_0];
    for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = l0RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
            m_hevcPicParams->RollingIntraReferenceLocation[refIdx] = m_refList[refPicIdx]->rollingIntraRefreshedPosition;
        }
    }

    // clean-up per VDBOX semaphore memory
    int32_t currentPass = GetCurrentPass();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucCmdInitializer->CmdInitializerSetConstData(
        m_osInterface,
        m_miInterface,
        m_vdencInterface,
        m_hevcSeqParams,
        m_hevcPicParams,
        m_hevcSliceParams,
        m_pakOnlyPass,
        m_hevcVdencAcqpEnabled,
        m_brcEnabled,
        m_vdencStreamInEnabled,
        m_vdencNativeROIEnabled,
        m_hevcVdencRoundingEnabled,
        panicEnabled,
        currentPass));

    // Send HuC BRC Init/ Update only on first pipe.
    if (m_vdencHucUsed)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucCmdInitializer->CmdInitializerExecute(true, &m_vdencReadBatchBuffer[m_currRecycledBufIdx][currentPass]));

        if (!m_singleTaskPhaseSupported)
        {
            //Reset earlier set PAK perf tag
            m_osInterface->pfnResetPerfBufferID(m_osInterface);

            // STF: HuC+VDEnc+PAK single BB, non-STF: HuC Init/HuC Update/(VDEnc+PAK) in separate BBs
            perfTag.Value                = 0;
            perfTag.Mode                 = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
            perfTag.CallType             = CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET;
            perfTag.PictureCodingType    = m_pictureCodingType;
            m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
        }
        m_resVdencBrcUpdateDmemBufferPtr[0] = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, pakInfo);

        // Invoke BRC init/reset FW
        if (m_brcInit || m_brcReset)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcInitReset());
        }

        if (!m_singleTaskPhaseSupported)
        {
            //Reset performance buffer used for BRC init
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
            // STF: HuC+VDEnc+PAK single BB, non-STF: HuC Init/HuC Update/(VDEnc+PAK) in separate BBs
            perfTag.Value                = 0;
            perfTag.Mode                 = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
            perfTag.CallType             = CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE;
            perfTag.PictureCodingType    = m_pictureCodingType;
            m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
        }

        // Invoke BRC update FW
        CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcUpdate());
        m_brcInit = m_brcReset = false;
        if (!m_singleTaskPhaseSupported)
        {
            //reset performance buffer used for BRC update
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }
    }
    else
    {
        ConstructBatchBufferHuCCQP(&m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].OsResource);
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported)
    {
        //PAK Perf Tag
        perfTag.Value             = 0;
        perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
        perfTag.CallType          = CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE;
        perfTag.PictureCodingType = m_pictureCodingType;
        m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
    }

    if ((!m_singleTaskPhaseSupported || (m_firstTaskInPhase && !m_hevcVdencAcqpEnabled)) )
    {
        // Send command buffer header at the beginning (OS dependent)
        // frame tracking tag is only added in the last command buffer header
        bool requestFrameTracking = m_singleTaskPhaseSupported ?
            m_firstTaskInPhase :
            m_lastTaskInPhase;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
    }

    // ACQP + SSC, ACQP + WP, BRC, BRC + SSC, BRC + WP
    // 2nd pass for SSC, WP, BRC needs conditional batch buffer end cmd, which is decided by HUC_STATUS output from HuC
    if (currentPass && m_vdencHuCConditional2ndPass && (currentPass != m_uc2NdSaoPass))
    {
        MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS miConditionalBatchBufferEndParams;

        // Insert conditional batch buffer end
        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        // VDENC uses HuC FW generated semaphore for conditional 2nd pass
        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            &m_resPakMmioBuffer;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));

            auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);

            uint32_t baseOffset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) + sizeof(uint32_t) * 2;  // encodeStatus is offset by 2 DWs in the resource

            // Write back the HCP image control register for RC6 may clean it out
            MHW_MI_LOAD_REGISTER_MEM_PARAMS miLoadRegMemParams;
            MOS_ZeroMemory(&miLoadRegMemParams, sizeof(miLoadRegMemParams));
            miLoadRegMemParams.presStoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
            miLoadRegMemParams.dwOffset = baseOffset + m_encodeStatusBuf.dwImageStatusCtrlOffset;
            miLoadRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(&cmdBuffer, &miLoadRegMemParams));

            MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
            MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
            miStoreRegMemParams.presStoreBuffer = &m_vdencBrcBuffers.resBrcPakStatisticBuffer[m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite];
            miStoreRegMemParams.dwOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS);
            miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &miStoreRegMemParams));

            MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
            miStoreRegMemParams.presStoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
            miStoreRegMemParams.dwOffset = baseOffset + m_encodeStatusBuf.dwImageStatusCtrlOfLastBRCPassOffset;
            miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &miStoreRegMemParams));
    }

    if (!currentPass && m_osInterface->bTagResourceSync)
    {
        // This is a short term solution to solve the sync tag issue: the sync tag write for PAK is inserted at the end of 2nd pass PAK BB
        // which may be skipped in multi-pass PAK enabled case. The idea here is to insert the previous frame's tag at the beginning
        // of the BB and keep the current frame's tag at the end of the BB. There will be a delay for tag update but it should be fine
        // as long as Dec/VP/Enc won't depend on this PAK so soon.

        MOS_RESOURCE globalGpuContextSyncTagBuffer;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
            m_osInterface,
            &globalGpuContextSyncTagBuffer));

        MHW_MI_STORE_DATA_PARAMS params;
        params.pOsResource = &globalGpuContextSyncTagBuffer;
        params.dwResourceOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        uint32_t value = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        params.dwValue = (value > 0) ? (value - 1) : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &params));
    }

    if (!m_lookaheadUpdate)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));
    }

    MHW_VDBOX_SURFACE_PARAMS srcSurfaceParams;
    SetHcpSrcSurfaceParams(srcSurfaceParams);

    MHW_VDBOX_SURFACE_PARAMS reconSurfaceParams;
    SetHcpReconSurfaceParams(reconSurfaceParams);

    *m_pipeBufAddrParams = {};
    SetHcpPipeBufAddrParams(*m_pipeBufAddrParams);
    m_pipeBufAddrParams->pRawSurfParam = &srcSurfaceParams;
    m_pipeBufAddrParams->pDecodedReconParam = &reconSurfaceParams;
    m_mmcState->SetPipeBufAddr(m_pipeBufAddrParams, &cmdBuffer);

    SetHcpPipeModeSelectParams(*m_pipeModeSelectParams);

    // HuC modifies HCP pipe mode select command, when 2nd pass SAO is required
    if (m_vdencHucUsed && m_b2NdSaoPassNeeded)
    {
        // current location to add cmds in 2nd level batch buffer
        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].iCurrent = 0;
        // reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = 0;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx]));

        // save offset for next 2nd level batch buffer usage
        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;
    }
    else
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(&cmdBuffer, m_pipeModeSelectParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &srcSurfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &reconSurfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeBufAddrCmd(&cmdBuffer, m_pipeBufAddrParams));

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    SetHcpIndObjBaseAddrParams(indObjBaseAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    MHW_VDBOX_QM_PARAMS fqmParams, qmParams;
    SetHcpQmStateParams(fqmParams, qmParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpFqmStateCmd(&cmdBuffer, &fqmParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpQmStateCmd(&cmdBuffer, &qmParams));

    SetVdencPipeModeSelectParams(*m_pipeModeSelectParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeModeSelectCmd(&cmdBuffer, m_pipeModeSelectParams));

    MHW_VDBOX_SURFACE_PARAMS dsSurfaceParams[2];
    SetVdencSurfaceStateParams(srcSurfaceParams, reconSurfaceParams, dsSurfaceParams[0], dsSurfaceParams[1]);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencSrcSurfaceStateCmd(&cmdBuffer, &srcSurfaceParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencRefSurfaceStateCmd(&cmdBuffer, &reconSurfaceParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencDsRefSurfaceStateCmd(&cmdBuffer, &dsSurfaceParams[0], 2));

    SetVdencPipeBufAddrParams(*m_pipeBufAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeBufAddrCmd(&cmdBuffer, m_pipeBufAddrParams));

    MHW_VDBOX_HEVC_PIC_STATE picStateParams;
    SetHcpPicStateParams(picStateParams);

    if (m_vdencHucUsed)
    {
        // 2nd level batch buffer
        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx]));

        // save offset for next 2nd level batch buffer usage
        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset += m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
    }
    else
    {
        // current location to add cmds in 2nd level batch buffer
        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].iCurrent = 0;
        // reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = 0;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx]));
        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
    }

    // Send HEVC_VP9_RDOQ_STATE command
    if (m_hevcRdoqEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpHevcVp9RdoqStateCmd(&cmdBuffer, &picStateParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::SendHwSliceEncodeCommand(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pHevcPicIdx);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ppNalUnitParams);

    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = params->pEncodeHevcPicParams;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams = params->pEncodeHevcSliceParams;

    // VDENC does not use batch buffer for slice state
    // add HCP_REF_IDX command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpRefIdxCmd(cmdBuffer, nullptr, params));

    if (params->bVdencHucInUse)
    {
        // 2nd level batch buffer
        PMHW_BATCH_BUFFER secondLevelBatchBufferUsed = params->pVdencBatchBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(cmdBuffer, secondLevelBatchBufferUsed));
    }
    else
    {
        // Weighted Prediction
        // This slice level command is issued, if the weighted_pred_flag or weighted_bipred_flag equals one.
        // If zero, then this command is not issued.
        if (params->bWeightedPredInUse)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpWeightOffsetStateCmd(cmdBuffer, hevcSlcParams));
        }

        // add HEVC Slice state commands
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSliceStateCmd(cmdBuffer, params));

        // add HCP_PAK_INSERT_OBJECTS command
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPakInsertNALUs(cmdBuffer, params->pVdencBatchBuffer, params));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPakInsertSliceHeader(cmdBuffer, params->pVdencBatchBuffer, params));

        // Send VDENC_WEIGHT_OFFSETS_STATE command
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AddVdencWeightOffsetStateCmd(cmdBuffer, hevcSlcParams));
    }

    // Send VDENC_WALKER_STATE command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddVdencWalkerStateCmd(cmdBuffer, params));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::ExecuteSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBatchBufferForPakSlices());

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    SetHcpSliceStateCommonParams(*m_sliceStateParams);

    // starting location for executing slice level cmds
    m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;

    PCODEC_ENCODER_SLCDATA slcData = m_slcData;
    for (uint32_t startLcu = 0, slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        if (IsFirstPass())
        {
            slcData[slcCount].CmdOffset = startLcu * (m_hcpInterface->GetHcpPakObjSize()) * sizeof(uint32_t);
        }

        SetHcpSliceStateParams(*m_sliceStateParams, slcData, slcCount);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHwSliceEncodeCommand(&cmdBuffer, m_sliceStateParams));

        startLcu += m_hevcSliceParams[slcCount].NumLCUsInSlice;

        m_batchBufferForPakSlicesStartOffset = (uint32_t)m_batchBufferForPakSlices[m_currPakSliceIdx].iCurrent;

        if (m_hevcVdencAcqpEnabled || m_brcEnabled)
        {
            // save offset for next 2nd level batch buffer usage
            // This is because we don't know how many times HCP_WEIGHTOFFSET_STATE & HCP_PAK_INSERT_OBJECT will be inserted for each slice
            // dwVdencBatchBufferPerSliceConstSize: constant size for each slice
            // m_vdencBatchBufferPerSliceVarSize:   variable size for each slice
            m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset += m_hwInterface->m_vdencBatchBufferPerSliceConstSize + m_vdencBatchBufferPerSliceVarSize[slcCount];
        }

        // Send VD_PIPELINE_FLUSH command
        MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;
        MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
        vdPipelineFlushParams.Flags.bWaitDoneMFX = 1;
        vdPipelineFlushParams.Flags.bWaitDoneVDENC = 1;
        vdPipelineFlushParams.Flags.bFlushVDENC = 1;
        vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipelineFlushParams));
    }

    if (m_useBatchBufferForPakSlices)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_currPakSliceIdx],
            m_lastTaskInPhase));
    }

    // Insert end of sequence/stream if set
    if (m_lastPicInStream || m_lastPicInSeq)
    {
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastPicInSeq = m_lastPicInSeq;
        pakInsertObjectParams.bLastPicInStream = m_lastPicInStream;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(&cmdBuffer, &pakInsertObjectParams));
    }

    // Send MI_FLUSH command
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Send VD_PIPELINE_FLUSH command
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;
    MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
    vdPipelineFlushParams.Flags.bWaitDoneHEVC = 1;
    vdPipelineFlushParams.Flags.bFlushHEVC = 1;
    vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipelineFlushParams));

    // Send MI_FLUSH command
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(&cmdBuffer));

    // BRC PAK statistics different for each pass
    if (m_brcEnabled)
    {
        uint32_t offset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
            m_encodeStatusBuf.dwNumPassesOffset +   // Num passes offset
            sizeof(uint32_t) * 2;                               // encodeStatus is offset by 2 DWs in the resource

        EncodeReadBrcPakStatsParams readBrcPakStatsParams;
        readBrcPakStatsParams.pHwInterface = m_hwInterface;
        readBrcPakStatsParams.presBrcPakStatisticBuffer = &m_vdencBrcBuffers.resBrcPakStatisticBuffer[m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite];
        readBrcPakStatsParams.presStatusBuffer = &m_encodeStatusBuf.resStatusBuffer;
        readBrcPakStatsParams.dwStatusBufNumPassesOffset = offset;
        readBrcPakStatsParams.ucPass = (uint8_t) GetCurrentPass();
        readBrcPakStatsParams.VideoContext = m_videoContext;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStatistics(
            &cmdBuffer,
            &readBrcPakStatsParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadSseStatistics(&cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadSliceSize(&cmdBuffer));

    if (m_lookaheadPass)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreLookaheadStatistics(&cmdBuffer));
    }
#if USE_CODECHAL_DEBUG_TOOL
    if (m_brcEnabled && m_enableFakeHrdSize)
    {
        uint32_t sizeInByte = (m_pictureCodingType == I_TYPE) ? m_fakeIFrameHrdSize : m_fakePBFrameHrdSize;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ModifyEncodedFrameSizeWithFakeHeaderSize(
            &cmdBuffer,
            sizeInByte,
            m_resVdencBrcUpdateDmemBufferPtr[0],
            0,
            &m_resFrameStatStreamOutBuffer,
            sizeof(uint32_t) * 4));
    }
#endif

    if (!m_lookaheadUpdate)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));
    }

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    std::string pakPassName = "PAK_PASS[" + std::to_string(GetCurrentPass())+"]";
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN( m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            pakPassName.data()));)

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        bool renderingFlags = m_videoContextUsesNullHw;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderingFlags));

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState)
            {
                m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
            }
        )

        if (IsLastPass() &&
            m_signalEnc &&
            m_currRefSync &&
            !Mos_ResourceIsNull(&m_currRefSync->resSyncObject))
        {
            // signal semaphore
            MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_videoContext;
            syncParams.presSyncResource = &m_currRefSync->resSyncObject;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            m_currRefSync->uiSemaphoreObjCount++;
            m_currRefSync->bInUsed = true;
        }
    }

    // HuC FW outputs are ready at this point if single task phase is enabled
    if (m_vdencHucUsed && m_singleTaskPhaseSupported)
    {
        // HuC Output STF=1
        CODECHAL_DEBUG_TOOL(DumpHucBrcUpdate(false));
    }

    // Reset parameters for next PAK execution
    if (IsLastPass())
    {
        if (!m_singleTaskPhaseSupported)
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_currPakSliceIdx = (m_currPakSliceIdx + 1) % CODECHAL_HEVC_NUM_PAK_SLICE_BATCH_BUFFERS;

        if (m_hevcSeqParams->ParallelBRC)
        {
            m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite =
                (m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
        }

        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
        m_frameNum++;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::ReadHcpStatus(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::ReadHcpStatus(cmdBuffer));

    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);
    // Slice Size Conformance
    if (m_hevcSeqParams->SliceSizeControl)
    {
        MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = m_resSliceCountBuffer;
        miStoreRegMemParams.dwOffset = 0;
        miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncSliceCountRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = m_resVdencModeTimerBuffer;
        miStoreRegMemParams.dwOffset = 0;
        miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncVdencModeTimerRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
    }

    if (m_vdencHucUsed)
    {
        // Store PAK frameSize MMIO to PakInfo buffer
        MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = m_resVdencBrcUpdateDmemBufferPtr[0];
        miStoreRegMemParams.dwOffset = 0;
        miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadImageStatus(cmdBuffer))

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::SetSequenceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::SetSequenceStructs());

    switch (m_hevcSeqParams->TargetUsage)
    {
        case 1: case 2:                         // Quality mode
            m_hevcSeqParams->TargetUsage = 1;
            break;
        case 3: case 4: case 5:                 // Normal mode
            m_hevcSeqParams->TargetUsage = 4;
            break;
        case 6: case 7:                         // Speed mode
            m_hevcSeqParams->TargetUsage = 7;
            break;
        default:
            m_hevcSeqParams->TargetUsage = 4;
            break;
    }

    m_targetUsage = (uint32_t)m_hevcSeqParams->TargetUsage;

    // enable motion adaptive under game streamming scenario for better quality
    if (m_hevcSeqParams->ScenarioInfo == ESCENARIO_GAMESTREAMING)
    {
        m_enableMotionAdaptive = true;
    }

    // ACQP is by default disabled, enable it when SSC/QpAdjust required.
    if (m_hevcSeqParams->SliceSizeControl == true ||
        m_hevcSeqParams->QpAdjustment == true)
    {
        m_hevcVdencAcqpEnabled = true;
    }

    // Get row store cache offset as all the needed information is got here
    if (m_vdencInterface->IsRowStoreCachingSupported())
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowStoreParams;
        rowStoreParams.Mode = m_mode;
        rowStoreParams.dwPicWidth = m_frameWidth;
        rowStoreParams.ucChromaFormat   = m_chromaFormat;
        rowStoreParams.ucBitDepthMinus8 = m_hevcSeqParams->bit_depth_luma_minus8;
        rowStoreParams.ucLCUSize        = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
        // VDEnc only support LCU64 for now
        CODECHAL_ENCODE_ASSERT(rowStoreParams.ucLCUSize == MAX_LCU_SIZE);
        m_hwInterface->SetRowstoreCachingOffsets(&rowStoreParams);
    }

    m_lookaheadDepth = m_hevcSeqParams->LookaheadDepth;
    m_lookaheadPass  = (m_lookaheadDepth > 0) && (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::SetPictureStructs());

    m_virtualEngineBbIndex = m_currOriginalPic.FrameIdx;

    //Enable only for TU1
    if (m_hevcSeqParams->TargetUsage != 1)
    {
        m_hmeEnabled = m_b16XMeEnabled = m_b32XMeEnabled = false;
        m_16xMeSupported = false;
    }

    // SSC can be satisfied in single VDEnc+PAK pass when required.
    // However it is not 100% guaranteed due to delay in HW.
    // When it happens, PAK would indicate SSC violation in MMIO register
    // and HuC would adjust SSC threshold and triggers another VDEnc+PAK pass.
    // SSC requires HuC for all target usages. (allow 1 pass SSC temporarily for testing purpose)
    if (m_hevcSeqParams->SliceSizeControl)
    {
        m_vdencHuCConditional2ndPass = true;
    }

    // Weighted Prediction is supported only with VDEnc, only applicable to P/B frames
    if (m_hevcPicParams->weighted_pred_flag || m_hevcPicParams->weighted_bipred_flag)
    {
        // with SAO, needs to increase total number of passes to 3 later (2 for SAO, 1 for WP)
        m_hevcVdencWeightedPredEnabled = true;
        m_vdencHuCConditional2ndPass = true;

        // Set ACQP enabled if GPU base WP is required.
        if(m_hevcPicParams->bEnableGPUWeightedPrediction)
        {
            m_hevcVdencAcqpEnabled = true;
        }
    }
    
    if (m_brcEnabled)  // VDEnc BRC supports maximum 2 PAK passes
    {
        if (m_hevcPicParams->BRCPrecision == 1)  // single-pass BRC, App requirment with first priority
        {
            m_numPasses = 0;
            // There is no need of additional pass for SSC, violation rate could be high but ok
        }
        else if (m_multipassBrcSupported)   // multi-pass BRC is supported
        {
            m_numPasses = CODECHAL_VDENC_BRC_NUM_OF_PASSES - 1;
            m_vdencHuCConditional2ndPass = true;
        }
        else
        {
            m_numPasses = 0;
        }

        m_vdencBrcEnabled = true;
        m_hevcVdencAcqpEnabled = false;  // when BRC is enabled, ACQP has to be turned off
    }
    else   // CQP, ACQP
    {
        m_numPasses = 0;

        // ACQP + SSC, ACQP + WP. CQP + SSC/WP donot need 2nd pass
        // driver programs 2nd pass, but it will be decided by conditional batch buffer end cmd to execute 2nd pass
        if (m_vdencHuCConditional2ndPass && m_hevcVdencAcqpEnabled)
        {
            m_numPasses += 1;
        }
    }

    CODECHAL_ENCODE_VERBOSEMESSAGE("m_numPasses = %d",m_numPasses);

    m_vdencHucUsed = m_hevcVdencAcqpEnabled || m_vdencBrcEnabled;

    // VDEnc always needs to enable due to pak fractional QP features
    // In VDENC mode, this field "Cu_Qp_Delta_Enabled_Flag" should always be set to 1.
    CODECHAL_ENCODE_ASSERT(m_hevcPicParams->cu_qp_delta_enabled_flag == 1);

    // Restriction: If RollingI is enabled, ROI needs to be disabled
    if (m_hevcPicParams->bEnableRollingIntraRefresh)
    {
        m_hevcPicParams->NumROI = 0;
    }

    //VDEnc StreamIn enabled if case of ROI (All frames), DirtyRect and SHME (ldB frames)
    m_vdencStreamInEnabled = (m_vdencEnabled) && (m_hevcPicParams->NumROI ||
                                                     (m_hevcPicParams->NumDirtyRects > 0 && (B_TYPE == m_hevcPicParams->CodingType)) || (m_b16XMeEnabled));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(PrepareVDEncStreamInData());

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::PrepareVDEncStreamInData()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_vdencStreamInEnabled && m_hevcPicParams->NumROI)
    {
        ProcessRoiDeltaQp();

        if (m_vdencHucUsed && !m_vdencNativeROIEnabled)
        {
            //ForceQp ROI in ACQP, BRC mode only
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupBRCROIStreamIn(&m_resVdencStreamInBuffer[m_currRecycledBufIdx], &m_vdencDeltaQpBuffer[m_currRecycledBufIdx]));
        }
        else
        {
            //Native ROI
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupROIStreamIn(&(m_resVdencStreamInBuffer[m_currRecycledBufIdx])));
        }
    }
    else if (m_vdencStreamInEnabled && (m_hevcPicParams->NumDirtyRects > 0 && (B_TYPE == m_hevcPicParams->CodingType)))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupDirtyRectStreamIn(&(m_resVdencStreamInBuffer[m_currRecycledBufIdx])));
    }
    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::CalcScaledDimensions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // HME Scaling WxH
    m_downscaledWidthInMb4x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x);
    m_downscaledHeightInMb4x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    m_downscaledWidth4x =
        m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x =
        m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT;

    // SuperHME Scaling WxH
    m_downscaledWidthInMb16x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_16x);
    m_downscaledHeightInMb16x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_16x);
    m_downscaledWidth16x =
        m_downscaledWidthInMb16x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight16x =
        m_downscaledHeightInMb16x * CODECHAL_MACROBLOCK_HEIGHT;

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::ValidateRefFrameData(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(slcParams);

    uint8_t maxNumRef0 = m_numMaxVdencL0Ref;
    uint8_t maxNumRef1 = m_numMaxVdencL1Ref;

    if (slcParams->num_ref_idx_l0_active_minus1 > maxNumRef0 - 1)
    {
        CODECHAL_ENCODE_ASSERT(false);
        slcParams->num_ref_idx_l0_active_minus1 = maxNumRef0 - 1;
    }

    if (slcParams->num_ref_idx_l1_active_minus1 > maxNumRef1 - 1)
    {
        CODECHAL_ENCODE_ASSERT(false);
        slcParams->num_ref_idx_l1_active_minus1 = maxNumRef1 - 1;
    }

    // For HEVC VDEnc, L0 and L1 must contain the same (number of) elements. If not, the input slc param is not good for VDEnc.
    if (slcParams->num_ref_idx_l0_active_minus1 != slcParams->num_ref_idx_l1_active_minus1)
    {
        CODECHAL_ENCODE_ASSERT(false);
        slcParams->num_ref_idx_l1_active_minus1 = slcParams->num_ref_idx_l0_active_minus1;
    }

    for (auto j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; j++)
    {
        if (slcParams->RefPicList[0][j].PicEntry != slcParams->RefPicList[1][j].PicEntry)
        {
            CODECHAL_ENCODE_ASSERT(false);
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::InitializePicture(const EncoderParams& params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::InitializePicture(params));

    m_resVdencStatsBuffer = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, vdencStats);
    m_resPakStatsBuffer = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, pakStats);
    m_resSliceCountBuffer = &m_sliceCountBuffer;
    m_resVdencModeTimerBuffer = &m_vdencModeTimerBuffer;

    CODECHAL_DEBUG_TOOL(
        if (m_newSeq) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateConstParam());
        }

        for (uint32_t i = 0; i < m_numSlices; i++) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateDdiParam(
                m_hevcSeqParams,
                m_hevcPicParams,
                &m_hevcSliceParams[i]));
    })
    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::UserFeatureKeyReport());

#if (_DEBUG || _RELEASE_INTERNAL)
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_VDENC_IN_USE_ID, m_vdencEnabled);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_ACQP_ENABLE_ID, m_hevcVdencAcqpEnabled);
#endif

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::GetStatusReport(
    EncodeStatus *encodeStatus,
    EncodeStatusReport *encodeStatusReport)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::GetStatusReport(encodeStatus, encodeStatusReport));

    if (m_vdencHucUsed)
    {
        // Num of VDEn BRC pass is stored at PakMmio DW0
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = true;

        MOS_RESOURCE *pakInfoBuffer = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, pakInfo);
        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, pakInfoBuffer, &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);
        uint32_t* insertion = (uint32_t*)(data + sizeof(uint32_t));
        *insertion = encodeStatus->ImageStatusCtrl.hcpTotalPass << 24;
        m_osInterface->pfnUnlockResource(m_osInterface, pakInfoBuffer);
        pakInfoBuffer = nullptr;
    }


    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    uint32_t* sliceSize = nullptr;
    // pSliceSize is set/ allocated only when dynamic slice is enabled. Cannot use SSC flag here, as it is an asynchronous call
    if (encodeStatus->sliceReport.pSliceSize)
    {
        sliceSize = (uint32_t*)m_osInterface->pfnLockResource(m_osInterface, encodeStatus->sliceReport.pSliceSize, &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(sliceSize);

        encodeStatusReport->NumberSlices            = encodeStatus->sliceReport.NumberSlices;
        encodeStatusReport->SizeOfSliceSizesBuffer  = sizeof(uint16_t) * encodeStatus->sliceReport.NumberSlices;
        encodeStatusReport->SliceSizeOverflow       = (encodeStatus->sliceReport.SliceSizeOverflow >> 16) & 1;
        encodeStatusReport->pSliceSizes             = (uint16_t*)sliceSize;

        uint16_t prevCumulativeSliceSize = 0;
        // HW writes out a DW for each slice size. Copy in place the DW into 16bit fields expected by App
        for (auto sliceCount = 0; sliceCount < encodeStatus->sliceReport.NumberSlices; sliceCount++)
        {
            // PAK output the sliceSize at 16DW intervals. 
            CODECHAL_ENCODE_CHK_NULL_RETURN(&sliceSize[sliceCount * 16]);
            uint32_t CurrAccumulatedSliceSize           = sliceSize[sliceCount * 16];

            //convert cummulative slice size to individual, first slice may have PPS/SPS,
            encodeStatusReport->pSliceSizes[sliceCount] = CurrAccumulatedSliceSize - prevCumulativeSliceSize;
            prevCumulativeSliceSize += encodeStatusReport->pSliceSizes[sliceCount];
        }
        m_osInterface->pfnUnlockResource(m_osInterface, encodeStatus->sliceReport.pSliceSize);
    }

    encodeStatusReport->cqmHint = 0xFF;
    if (m_lookaheadPass && m_lookaheadUpdate)
    {
        encodeStatusReport->cqmHint = (uint8_t)(encodeStatus->lookaheadStatus & 0xFF);
        if (encodeStatusReport->cqmHint > 1)
        {
            // Currently only 0x00 and 0x01 are valid. Report invalid (0xFF) for other values.
            encodeStatusReport->cqmHint = 0xFF; 
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::AllocatePakResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::AllocatePakResources());

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    // Allocate Frame Statistics Streamout Data Destination Buffer. DW98-100 in HCP PipeBufAddr command
    uint32_t size                       = MOS_ALIGN_CEIL(m_sizeOfHcpPakFrameStats * m_maxTileNumber, CODECHAL_PAGE_SIZE);  //Each tile has 8 cache size bytes of data, Align to page is HuC requirement
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "FrameStatStreamOutBuffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                  m_osInterface,
                                                  &allocParamsForBufferLinear,
                                                  &m_resFrameStatStreamOutBuffer),
        "Failed to create VDENC FrameStatStreamOutBuffer Buffer");

    // PAK Statistics buffer
    size = MOS_ALIGN_CEIL(m_vdencBrcPakStatsBufferSize, CODECHAL_PAGE_SIZE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_allocator->AllocateResource(
        m_standard, size, 1, pakStats, "pakStats"));

    // Slice Count buffer 1 DW = 4 Bytes
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(4, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "Slice Count Buffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_sliceCountBuffer),
        "Failed to create VDENC Slice Count Buffer");

    // VDEncMode Timer buffer 1 DW = 4 Bytes
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(4, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDEncMode Timer Buffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencModeTimerBuffer),
        "Failed to create VDEncMode Timer Buffer");

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::FreePakResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(m_osInterface, &m_resFrameStatStreamOutBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_sliceCountBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencModeTimerBuffer);

    for (uint32_t i = 0; i < CODECHAL_ENCODE_STATUS_NUM; i++)
    {
        if (!Mos_ResourceIsNull(&m_resSliceReport[i]))
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_resSliceReport[i]);
        }
    }  

    return CodechalEncodeHevcBase::FreePakResources();
}

MOS_STATUS CodechalVdencHevcState::AllocateEncResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    // PAK stream-out buffer
    allocParamsForBufferLinear.dwBytes = CODECHAL_HEVC_PAK_STREAMOUT_SIZE;
    allocParamsForBufferLinear.pBufName = "Pak StreamOut Buffer";
    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resStreamOutBuffer[0]),
        "Failed to allocate Pak Stream Out Buffer.");

    // VDENC Intra Row Store Scratch buffer
    // 1 cacheline per MB
    uint32_t size = m_picWidthInMb * CODECHAL_CACHELINE_SIZE;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_allocator->AllocateResource(
        m_standard, size, 1, vdencIntraRowStoreScratch, "vdencIntraRowStoreScratch"));

    // VDENC Statistics buffer, only needed for BRC
    // The size is 19 CL for each tile, allocated with worst case, optimize later
    size = MOS_ALIGN_CEIL(m_vdencBrcStatsBufferSize * m_maxTileNumber, CODECHAL_PAGE_SIZE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_allocator->AllocateResource(
        m_standard, size, 1, vdencStats, "vdencStats"));

    if (m_hucCmdInitializer)
    {
        m_hucCmdInitializer->CmdInitializerAllocateResources(m_hwInterface);
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::FreeEncResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // PAK stream-out buffer de-allocated inside CodecHalEncodeReleaseResources()

    if (m_hucCmdInitializer)
    {
        m_hucCmdInitializer->CmdInitializerFreeResources();
    }
    MOS_Delete(m_hucCmdInitializer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencHevcState::AllocateBrcResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // initiate allocation paramters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    allocParamsForBufferLinear.dwBytes  = m_hevcBrcPakStatisticsSize;
    allocParamsForBufferLinear.pBufName = "BRC PAK Statistics Buffer";

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    uint8_t *data = nullptr;
    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_vdencBrcBuffers.resBrcPakStatisticBuffer[i]),
            "Failed to allocate BRC PAK Statistics Buffer.");

        CODECHAL_ENCODE_CHK_NULL_RETURN(data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &(m_vdencBrcBuffers.resBrcPakStatisticBuffer[i]),
            &lockFlagsWriteOnly));

        MOS_ZeroMemory(data, m_hevcBrcPakStatisticsSize);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_vdencBrcBuffers.resBrcPakStatisticBuffer[i]);
    }

    // PAK Info buffer
    uint32_t size = MOS_ALIGN_CEIL(sizeof(CodechalVdencHevcPakInfo), CODECHAL_PAGE_SIZE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_allocator->AllocateResource(
        m_standard, size, 1, pakInfo, "pakInfo"));

    // HuC FW Region 6: Data Buffer of Current Picture
    // Data (1024 bytes) for current
    // Data (1024 bytes) for ref0
    // Data (1024 bytes) for ref1
    // Data (1024 bytes) for ref2
    allocParamsForBufferLinear.dwBytes = CODECHAL_PAGE_SIZE * 4;
    allocParamsForBufferLinear.pBufName = "Data from Pictures Buffer for Weighted Prediction";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_dataFromPicsBuffer),
        "Failed to create Data from Pictures Buffer for Weighted Prediction");

    for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        // Delta QP for ROI Buffer
        // 1 byte for each 32x32 block, maximum region size is 8192 bytes for 4K/2K resolution, currently the allocation size is fixed
        allocParamsForBufferLinear.dwBytes = m_deltaQpRoiBufferSize;
        allocParamsForBufferLinear.pBufName = "Delta QP for ROI Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_vdencDeltaQpBuffer[k]),
            "Failed to create Delta QP for ROI Buffer");

        // BRC update DMEM
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BrcUpdate DmemBuffer";

        for (auto i = 0; i < CODECHAL_VDENC_BRC_NUM_OF_PASSES_FOR_TILE_REPLAY; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_vdencBrcUpdateDmemBuffer[k][i]),
                "Failed to create VDENC BrcUpdate DmemBuffer");

            CODECHAL_ENCODE_CHK_NULL_RETURN(data = (uint8_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &m_vdencBrcUpdateDmemBuffer[k][i],
                &lockFlagsWriteOnly));

            MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_vdencBrcUpdateDmemBuffer[k][i]);
        }

        // BRC init/reset DMEM
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BrcInit DmemBuffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_vdencBrcInitDmemBuffer[k]),
            "Failed to create VDENC BrcInit DmemBuffer");

        CODECHAL_ENCODE_CHK_NULL_RETURN(data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_vdencBrcInitDmemBuffer[k],
            &lockFlagsWriteOnly));

        MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_vdencBrcInitDmemBuffer[k]);

        // Const Data buffer
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcConstDataBufferSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BRC Const Data Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_vdencBrcConstDataBuffer[k]),
            "Failed to create VDENC BRC Const Data Buffer");

        // VDEnc read batch buffer (input for HuC FW)
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC Read Batch Buffer";

        for (auto i = 0; i < CODECHAL_VDENC_BRC_NUM_OF_PASSES; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_vdencReadBatchBuffer[k][i]),
                "Failed to allocate VDENC Read Batch Buffer");
        }
        
        // Lookahead Update DMEM
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencLaUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC Lookahead update Dmem Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_vdencLaUpdateDmemBuffer[k]),
            "Failed to create VDENC Lookahead Update Dmem Buffer");
    }

    for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
    {
        // VDENC uses second level batch buffer
        MOS_ZeroMemory(&m_vdenc2ndLevelBatchBuffer[j], sizeof(MHW_BATCH_BUFFER));
        m_vdenc2ndLevelBatchBuffer[j].bSecondLevel = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &m_vdenc2ndLevelBatchBuffer[j],
            nullptr,
            m_hwInterface->m_vdenc2ndLevelBatchBufferSize));
    }

    // BRC history buffer
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_brcHistoryBufSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC History Buffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencBrcHistoryBuffer),
        "Failed to create VDENC BRC History Buffer");
        
    // Lookahead Init DMEM
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencLaInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC Lookahead Init DmemBuffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencLaInitDmemBuffer),
        "Failed to create VDENC Lookahead Init DmemBuffer");

    // Lookahead history buffer
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_LaHistoryBufSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC Lookahead History Buffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencLaHistoryBuffer),
        "Failed to create VDENC Lookahead History Buffer");

    // Debug buffer
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_brcDebugBufSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC Debug Buffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencBrcDbgBuffer),
        "Failed to create VDENC BRC Debug Buffer");

    // Output ROI Streamin Buffer
    // 16 DWORDs (VDENC_HEVC_VP9_STREAMIN_STATE) for each 32x32 block, maximum region size is 65536 bytes for 8K/8K resolution, currently the allocation size is fixed
    allocParamsForBufferLinear.dwBytes = m_roiStreamInBufferSize;
    allocParamsForBufferLinear.pBufName = "Output ROI Streamin Buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencOutputROIStreaminBuffer));

    // Buffer to store VDEnc frame statistics for lookahead BRC
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_brcLooaheadStatsBufferSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC Lookahead Statistics Buffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencLaStatsBuffer),
        "Failed to create VDENC Lookahead Statistics Buffer");

    CodechalVdencHevcLaStats *lookaheadInfo = (CodechalVdencHevcLaStats *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_vdencLaStatsBuffer,
        &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(lookaheadInfo);
    MOS_ZeroMemory(lookaheadInfo, allocParamsForBufferLinear.dwBytes);
    m_osInterface->pfnUnlockResource(m_osInterface, &m_vdencLaStatsBuffer);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcState::FreeBrcResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_vdencBrcBuffers.resBrcPakStatisticBuffer[i]);
    }

    m_osInterface->pfnFreeResource(m_osInterface, &m_dataFromPicsBuffer);

    for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_vdencDeltaQpBuffer[k]);

        for (auto i = 0; i < CODECHAL_VDENC_BRC_NUM_OF_PASSES; i++)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_vdencReadBatchBuffer[k][i]);
        }

        for (auto i = 0; i < CODECHAL_VDENC_BRC_NUM_OF_PASSES_FOR_TILE_REPLAY; i++)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_vdencBrcUpdateDmemBuffer[k][i]);
        }

        m_osInterface->pfnFreeResource(m_osInterface, &m_vdencBrcInitDmemBuffer[k]);
        m_osInterface->pfnFreeResource(m_osInterface, &m_vdencBrcConstDataBuffer[k]);
        m_osInterface->pfnFreeResource(m_osInterface, &m_vdencLaUpdateDmemBuffer[k]);
    }

    for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
    {
        Mhw_FreeBb(m_osInterface, &m_vdenc2ndLevelBatchBuffer[j], nullptr);
    }

    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencBrcHistoryBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencBrcDbgBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencOutputROIStreaminBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencLaStatsBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencLaInitDmemBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencLaHistoryBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencHevcState::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeHevcBase::Initialize(settings));

    m_vdencBrcBuffers.uiCurrBrcPakStasIdxForRead = 0;
    //Reading buffer is with 2 frames late for BRC kernel uses the PAK statstic info of the frame before the previous frame
    m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite =
        (m_vdencBrcBuffers.uiCurrBrcPakStasIdxForRead + 2) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;

    uint32_t vdencPictureStatesSize = 0, vdencPicturePatchListSize = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetVdencStateCommandsDataSize(
        CODECHAL_ENCODE_MODE_HEVC,
        &vdencPictureStatesSize,
        &vdencPicturePatchListSize));

    //the following code used to calculate ulMBCodeSize:
    //pakObjCmdStreamOutDataSize = 2*BYTES_PER_DWORD*(numOfLcu*NUM_PAK_DWS_PER_LCU + numOfLcu*maxNumOfCUperLCU*NUM_DWS_PER_CU); // Multiply by 2 for sideband
    //const uint32_t maxNumOfCUperLCU = (64/8)*(64/8);
    // NUM_PAK_DWS_PER_LCU 5
    // NUM_DWS_PER_CU 8
    uint32_t numOfLCU = MOS_ROUNDUP_DIVIDE(m_frameWidth, MAX_LCU_SIZE) * MOS_ROUNDUP_DIVIDE(m_frameHeight, MAX_LCU_SIZE);
    m_mbCodeSize = MOS_ALIGN_CEIL(2 * sizeof(uint32_t) * numOfLCU * (5 + 64 * 8), CODECHAL_PAGE_SIZE);

    m_defaultPictureStatesSize += vdencPictureStatesSize;
    m_defaultPicturePatchListSize += vdencPicturePatchListSize;
    m_extraPictureStatesSize += m_hwInterface->m_hucCommandBufferSize;  // For slice size reporting, add the HuC copy commands

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
        &userFeatureData);
    m_singleTaskPhaseSupported = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_RDOQ_ENABLE_ID,
        &userFeatureData);
    m_hevcRdoqEnabled = userFeatureData.i32Data ? true : false;

    // Multi-Pass BRC
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MULTIPASS_BRC_ENABLE_ID,
        &userFeatureData);
    m_multipassBrcSupported = (userFeatureData.i32Data) ? true : false;

    if (m_codecFunction != CODECHAL_FUNCTION_PAK)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data = 1;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ME_ENABLE_ID,
            &userFeatureData);
        m_hmeSupported = (userFeatureData.i32Data) ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.i32Data = 1;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_16xME_ENABLE_ID,
            &userFeatureData);
        m_16xMeSupported = (userFeatureData.i32Data) ? true : false;
    }

    if (m_codecFunction == CODECHAL_FUNCTION_ENC_VDENC_PAK)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_ACQP_ENABLE_ID,
            &userFeatureData);
        m_hevcVdencAcqpEnabled = userFeatureData.i32Data ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_VQI_ENABLE_ID,
            &userFeatureData);
        m_hevcVisualQualityImprovement = userFeatureData.i32Data ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_ROUNDING_ENABLE_ID,
            &userFeatureData);
        m_hevcVdencRoundingEnabled = userFeatureData.i32Data ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_PAKOBJCMD_STREAMOUT_ENABLE_ID,
            &userFeatureData);
        m_vdencPakObjCmdStreamOutEnabled = userFeatureData.i32Data ? true : false;

#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENCODE_CQM_QP_THRESHOLD_ID,
            &userFeatureData);
        m_cqmQpThreshold = (uint8_t)userFeatureData.u32Data;
#endif
    }

    m_minScaledDimension = CODECHAL_ENCODE_MIN_SCALED_SURFACE_SIZE;
    m_minScaledDimensionInMb = (CODECHAL_ENCODE_MIN_SCALED_SURFACE_SIZE + 15) >> 4;

    if (m_frameWidth < 128 || m_frameHeight < 128)
    {
        m_16xMeSupported = false;
        m_32xMeSupported = false;
    }

    else if (m_frameWidth < 512 || m_frameHeight < 512)
    {
        m_16xMeSupported = true;
        m_32xMeSupported = false;
    }

    else
    {
        m_16xMeSupported = true;
        m_32xMeSupported = true;
    }

    if (m_16xMeSupported)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_16xME_ENABLE_ID,
            &userFeatureData);
        m_16xMeSupported = (userFeatureData.i32Data) ? true : false;
    }

    if (m_32xMeSupported)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_32xME_ENABLE_ID,
            &userFeatureData);
        m_32xMeSupported = (userFeatureData.i32Data) ? true : false;
    }

    return eStatus;
}

CodechalVdencHevcState::CodechalVdencHevcState(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncodeHevcBase(hwInterface, debugInterface, standardInfo)
{
    m_fieldScalingOutputInterleaved = false;
    m_2xMeSupported = false;
    m_combinedDownScaleAndDepthConversion = false;
    m_vdencBrcStatsBufferSize = m_brcStatsBufSize;
    m_vdencBrcPakStatsBufferSize = m_brcPakStatsBufSize;
    m_vdencLaInitDmemBufferSize = sizeof(CodechalVdencHevcLaDmem);
    m_vdencLaUpdateDmemBufferSize = sizeof(CodechalVdencHevcLaDmem);

    MOS_ZeroMemory(&m_sliceCountBuffer, sizeof(m_sliceCountBuffer));
    MOS_ZeroMemory(&m_vdencModeTimerBuffer, sizeof(m_vdencModeTimerBuffer));

    MOS_ZeroMemory(&m_vdencBrcBuffers, sizeof(m_vdencBrcBuffers));
    MOS_ZeroMemory(&m_dataFromPicsBuffer, sizeof(m_dataFromPicsBuffer));
    MOS_ZeroMemory(&m_vdencDeltaQpBuffer, sizeof(m_vdencDeltaQpBuffer));
    MOS_ZeroMemory(&m_vdencOutputROIStreaminBuffer, sizeof(m_vdencOutputROIStreaminBuffer));
    MOS_ZeroMemory(m_vdencBrcUpdateDmemBuffer, sizeof(m_vdencBrcUpdateDmemBuffer));
    MOS_ZeroMemory(&m_vdencBrcInitDmemBuffer, sizeof(m_vdencBrcInitDmemBuffer));
    MOS_ZeroMemory(&m_vdencBrcConstDataBuffer, sizeof(m_vdencBrcConstDataBuffer));
    MOS_ZeroMemory(&m_vdencBrcHistoryBuffer, sizeof(m_vdencBrcHistoryBuffer));
    MOS_ZeroMemory(&m_vdencReadBatchBuffer, sizeof(m_vdencReadBatchBuffer));
    MOS_ZeroMemory(&m_vdencReadBatchBuffer, sizeof(m_vdencGroup3BatchBuffer));
    MOS_ZeroMemory(&m_vdencBrcDbgBuffer, sizeof(m_vdencBrcDbgBuffer));
    MOS_ZeroMemory(&m_vdenc2ndLevelBatchBuffer, sizeof(m_vdenc2ndLevelBatchBuffer));
    MOS_ZeroMemory(m_resSliceReport, sizeof(m_resSliceReport));
    MOS_ZeroMemory(&m_vdencLaStatsBuffer, sizeof(m_vdencLaStatsBuffer));

}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalVdencHevcState::DumpHucBrcInit()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    int32_t currentPass = GetCurrentPass();
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
        &m_vdencBrcInitDmemBuffer[m_currRecycledBufIdx],
        m_vdencBrcInitDmemBufferSize,
        currentPass,
        hucRegionDumpInit));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
        &m_vdencBrcHistoryBuffer,
        0,
        CODECHAL_VDENC_HEVC_BRC_HISTORY_BUF_SIZE,
        0,
        "_History",
        true,
        currentPass,
        hucRegionDumpInit));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencHevcState::DumpHucBrcUpdate(bool isInput)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    int32_t currentPass = GetCurrentPass();
    if (isInput)
    {
        //Dump HucBrcUpdate input buffers
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
            &m_vdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][currentPass],
            m_vdencBrcUpdateDmemBufferSize,
            currentPass,
            hucRegionDumpUpdate));

        // Region 1 - VDENC Statistics Buffer dump
        auto vdencStatusBuffer = m_virtualAddrParams.regionParams[1].presRegion;
        auto vdencStatusOffset = m_virtualAddrParams.regionParams[1].dwOffset;
        if (vdencStatusBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                vdencStatusBuffer,
                vdencStatusOffset,
                m_vdencBrcStatsBufferSize,
                1,
                "_VdencStats",
                true,
                currentPass,
                hucRegionDumpUpdate));
        }

        // Region 2 - PAK Statistics Buffer dump
        auto frameStatStreamOutBuffer = m_virtualAddrParams.regionParams[2].presRegion;
        auto frameStatStreamOutOffset = m_virtualAddrParams.regionParams[2].dwOffset;
        if (frameStatStreamOutBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                frameStatStreamOutBuffer,
                frameStatStreamOutOffset,
                m_vdencBrcPakStatsBufferSize,
                2,
                "_PakStats",
                true,
                currentPass,
                hucRegionDumpUpdate));
        }

        // Region 3 - Input SLB Buffer
        auto vdencReadBatchBuffer = m_virtualAddrParams.regionParams[3].presRegion;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
            vdencReadBatchBuffer,
            0,
            m_hwInterface->m_vdencReadBatchBufferSize,
            3,
            "_Slb",
            true,
            currentPass,
            hucRegionDumpUpdate));

        // Region 4 - Constant Data Buffer dump
        auto vdencBrcConstDataBuffer = m_virtualAddrParams.regionParams[4].presRegion;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
            vdencBrcConstDataBuffer,
            0,
            m_vdencBrcConstDataBufferSize,
            4,
            "_ConstData",
            true,
            currentPass,
            hucRegionDumpUpdate));

        // Region 7 - Slice Stat Streamout (Input)
        auto lucBasedAddressBuffer = m_virtualAddrParams.regionParams[7].presRegion;
        auto lucBasedAddressOffset = m_virtualAddrParams.regionParams[7].dwOffset;
        if (lucBasedAddressBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                lucBasedAddressBuffer,
                lucBasedAddressOffset,
                CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * CODECHAL_CACHELINE_SIZE,
                7,
                "_SliceStat",
                true,
                currentPass,
                hucRegionDumpUpdate));
        }

        // Region 8 - PAK MMIO Buffer dump
        auto pakInfoBufffer = m_virtualAddrParams.regionParams[8].presRegion;  
        if (pakInfoBufffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                pakInfoBufffer,
                0,
                sizeof(CodechalVdencHevcPakInfo),
                8,
                "_PakMmio",
                true,
                currentPass,
                hucRegionDumpUpdate));
        }

        // Region 9 - Streamin Buffer for ROI (Input)
        auto streamInBufferSize = (MOS_ALIGN_CEIL(m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;
        auto stramInBuffer = m_virtualAddrParams.regionParams[9].presRegion;
        if (stramInBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                stramInBuffer,
                0,
                streamInBufferSize,
                9,
                "_RoiStreamin",
                true,
                currentPass,
                hucRegionDumpUpdate));
        }

        // Region 10 - Delta QP for ROI Buffer
        auto vdencDeltaQpBuffer = m_virtualAddrParams.regionParams[10].presRegion;
        if (vdencDeltaQpBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                vdencDeltaQpBuffer,
                0,
                m_deltaQpRoiBufferSize,
                10,
                "_DeltaQp",
                true,
                currentPass,
                hucRegionDumpUpdate));
        }

        // Region 12 - Input SLB Buffer
        auto slbBuffer = m_virtualAddrParams.regionParams[12].presRegion;
        if (slbBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                slbBuffer,
                0,
                m_hwInterface->m_vdencGroup3BatchBufferSize,
                12,
                "_Slb",
                true,
                currentPass,
                hucRegionDumpUpdate));
        }
    }
    else
    {
        // Region 5 - Output SLB Buffer
        auto vdenc2ndLevelBatchBuffer = m_virtualAddrParams.regionParams[5].presRegion;
        if (vdenc2ndLevelBatchBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                vdenc2ndLevelBatchBuffer,
                0,
                m_hwInterface->m_vdenc2ndLevelBatchBufferSize,
                5,
                "_Slb",
                false,
                currentPass,
                hucRegionDumpUpdate));
        }

        // Region 11 - Output ROI Streamin Buffer
        auto vdencOutputROIStreaminBuffer = m_virtualAddrParams.regionParams[11].presRegion;
        if (vdencOutputROIStreaminBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                vdencOutputROIStreaminBuffer,
                0,
                m_roiStreamInBufferSize,
                11,
                "_RoiStreamin",
                false,
                currentPass,
                hucRegionDumpUpdate));
        }
    }

    // Region 0 - History Buffer dump (Input/Output)
    auto vdencBrcHistoryBuffer = m_virtualAddrParams.regionParams[0].presRegion;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
        vdencBrcHistoryBuffer,
        0,
        m_brcHistoryBufSize,
        0,
        "_History",
        isInput,
        currentPass,
        hucRegionDumpUpdate));

    // Region 6 - Data from Pictures for Weighted Prediction (Input/Output)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
        &m_dataFromPicsBuffer,
        0,
        CODECHAL_PAGE_SIZE * 4,
        6,
        "_PicsData",
        isInput,
        currentPass,
        hucRegionDumpUpdate));

    // Region 15 - Debug Output
    auto debugBuffer = m_virtualAddrParams.regionParams[15].presRegion;
    if (debugBuffer)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
            debugBuffer,
            0,
            0x1000,
            15,
            "_Debug",
            isInput,
            currentPass,
            hucRegionDumpUpdate));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencHevcState::DumpVdencOutputs()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Dump VDENC Stats Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        m_resVdencStatsBuffer,
        CodechalDbgAttr::attrVdencOutput,
        "_Stats",
        m_vdencBrcStatsBufferSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    // Dump PAK Stats Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        m_resPakStatsBuffer,
        CodechalDbgAttr::attrVdencOutput,
        "_PakStats",
        m_vdencBrcPakStatsBufferSize,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    // Dump PAK MMIO Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &m_resPakMmioBuffer,
        CodechalDbgKernel::kernelBrcUpdate,
        m_currPass ? "_MmioReg_Output_Pass1" : "_MmioReg_Output_Pass0",
        sizeof(VdencBrcPakMmio),
        0,
        CODECHAL_NUM_MEDIA_STATES));

    // Dump PAK Obj Cmd Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        m_resVdencPakObjCmdStreamOutBuffer,
        CodechalDbgAttr::attrVdencOutput,
        "_MbCode",
        m_mvOffset,
        0,
        CODECHAL_NUM_MEDIA_STATES));

    // Dump CU Record Cmd Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        m_resVdencPakObjCmdStreamOutBuffer,
        CodechalDbgAttr::attrVdencOutput,
        "_CURecord",
        m_mbCodeSize - m_mvOffset,
        m_mvOffset,
        CODECHAL_NUM_MEDIA_STATES));

    // Slice Size Conformance
    if (m_hevcSeqParams->SliceSizeControl)
    {
        uint32_t dwSize = CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6*CODECHAL_CACHELINE_SIZE;
        if (!m_hevcPicParams->tiles_enabled_flag)
        {
            // Slice Size StreamOut Surface
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resLcuBaseAddressBuffer,
                CodechalDbgAttr::attrVdencOutput,
                "_SliceSize",
                dwSize,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }

        dwSize = MOS_ALIGN_CEIL(4, CODECHAL_CACHELINE_SIZE);
        // Slice Count buffer 1 DW = 4 Bytes
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            m_resSliceCountBuffer,
            CodechalDbgAttr::attrVdencOutput,
            "_SliceCount",
            dwSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));

        // VDEncMode Timer buffer 1 DW = 4 Bytes
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            m_resVdencModeTimerBuffer,
            CodechalDbgAttr::attrVdencOutput,
            "_ModeTimer",
            dwSize,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencHevcState::DumpSeqParFile()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "ISliceQP                           = " << std::dec << m_hevcPar->ISliceQP << std::endl;
    oss << "PSliceQP                           = " << std::dec << m_hevcPar->PSliceQP << std::endl;
    oss << "BSliceQP                           = " << std::dec << m_hevcPar->BSliceQP << std::endl;
    oss << "StartFrameNum                      = " << std::dec << m_hevcPar->StartFrameNum << std::endl;
    oss << "ProfileIDC                         = " << std::dec << m_hevcPar->ProfileIDC << std::endl;
    oss << "LevelIDC                           = " << std::dec << m_hevcPar->LevelIDC << std::endl;
    oss << "NumP                               = " << std::dec << m_hevcPar->NumP << std::endl;
    oss << "NumB                               = " << std::dec << m_hevcPar->NumB << std::endl;
    oss << "NumSlices                          = " << std::dec << m_hevcPar->NumSlices << std::endl;
    oss << "SliceStartLCU                      = " << std::dec << m_hevcPar->SliceStartLCU << std::endl;
    oss << "Log2MinCUSize                      = " << std::dec << m_hevcPar->Log2MinCUSize << std::endl;
    oss << "Log2MaxCUSize                      = " << std::dec << m_hevcPar->Log2MaxCUSize << std::endl;
    oss << "Log2MinTUSize                      = " << std::dec << m_hevcPar->Log2MinTUSize << std::endl;
    oss << "Log2MaxTUSize                      = " << std::dec << m_hevcPar->Log2MaxTUSize << std::endl;
    oss << "InputBitDepthLuma                  = " << std::dec << m_hevcPar->InputBitDepthLuma << std::endl;
    oss << "InputBitDepthChroma                = " << std::dec << m_hevcPar->InputBitDepthChroma << std::endl;
    oss << "OutputBitDepthLuma                 = " << std::dec << m_hevcPar->OutputBitDepthLuma << std::endl;
    oss << "OutputBitDepthChroma               = " << std::dec << m_hevcPar->OutputBitDepthChroma << std::endl;
    oss << "InternalBitDepthLuma               = " << std::dec << m_hevcPar->InternalBitDepthLuma << std::endl;
    oss << "InternalBitDepthChroma             = " << std::dec << m_hevcPar->InternalBitDepthChroma << std::endl;
    oss << "Log2TUMaxDepthInter                = " << std::dec << m_hevcPar->Log2TUMaxDepthInter << std::endl;
    oss << "Log2TUMaxDepthIntra                = " << std::dec << m_hevcPar->Log2TUMaxDepthIntra << std::endl;
    oss << "Log2ParallelMergeLevel             = " << std::dec << m_hevcPar->Log2ParallelMergeLevel << std::endl;
    oss << "EnableTransquantBypass             = " << std::dec << m_hevcPar->TransquantBypassEnableFlag << std::endl;
    oss << "EnableTransformSkip                = " << std::dec << m_hevcPar->TransformSkipEnabledFlag << std::endl;
    oss << "TSDecisionEnabledFlag              = " << std::dec << m_hevcPar->TSDecisionEnabledFlag << std::endl;
    oss << "EnableTemporalMvp                  = " << std::dec << m_hevcPar->TemporalMvpEnableFlag << std::endl;
    oss << "CollocatedFromL0Flag               = " << std::dec << m_hevcPar->CollocatedFromL0Flag << std::endl;
    oss << "CollocatedRefIdx                   = " << std::dec << m_hevcPar->CollocatedRefIdx << std::endl;
    oss << "MvdL1ZeroFlag                      = " << std::dec << m_hevcPar->MvdL1ZeroFlag << std::endl;
    oss << "AmpEnabledFlag                     = " << std::dec << m_hevcPar->AmpEnabledFlag << std::endl;
    oss << "CuQpDeltaEnabledFlag               = " << std::dec << m_hevcPar->CuQpDeltaEnabledFlag << std::endl;
    oss << "DiffCuQpDeltaDepth                 = " << std::dec << m_hevcPar->DiffCuQpDeltaDepth << std::endl;
    oss << "ChromaCbQpOffset                   = " << std::dec << m_hevcPar->ChromaCbQpOffset << std::endl;
    oss << "ChromaCrQpOffset                   = " << std::dec << m_hevcPar->ChromaCrQpOffset << std::endl;
    oss << "DeblockingFilterTc                 = " << std::dec << m_hevcPar->DeblockingTc << std::endl;
    oss << "DeblockingFilterBeta               = " << std::dec << m_hevcPar->DeblockingTc << std::endl;
    oss << "DeblockingIDC                      = " << std::dec << m_hevcPar->DeblockingIDC << std::endl;
    oss << "LoopFilterAcrossSlicesEnabledFlag  = " << std::dec << m_hevcPar->LoopFilterAcrossSlicesEnabledFlag << std::endl;
    oss << "SignDataHidingFlag                 = " << std::dec << m_hevcPar->SignDataHidingFlag << std::endl;
    oss << "CabacInitFlag                      = " << std::dec << m_hevcPar->CabacInitFlag << std::endl;
    oss << "ConstrainedIntraPred               = " << std::dec << m_hevcPar->ConstrainedIntraPred << std::endl;
    oss << "LowDelay                           = " << std::dec << m_hevcPar->LowDelay << std::endl;
    oss << "EnableBAsRefs                      = " << std::dec << m_hevcPar->EnableBAsRefs << std::endl;
    oss << "BitRate                            = " << std::dec << m_hevcPar->BitRate << std::endl;
    oss << "MaxBitRate                         = " << std::dec << m_hevcPar->MaxBitRate << std::endl;
    oss << "VbvSzInBit                         = " << std::dec << m_hevcPar->VbvSzInBit << std::endl;
    oss << "InitVbvFullnessInBit               = " << std::dec << m_hevcPar->InitVbvFullnessInBit << std::endl;
    oss << "CuRC                               = " << std::dec << m_hevcPar->CuRC << std::endl;
    oss << "EnableMultipass                    = " << std::dec << m_hevcPar->EnableMultipass << std::endl;
    oss << "MaxNumPakPassesI                   = " << std::dec << m_hevcPar->MaxNumPakPassesI << std::endl;
    oss << "MaxNumPakPassesPB                  = " << std::dec << m_hevcPar->MaxNumPakPassesPB << std::endl;
    oss << "UserMaxIFrame                      = " << std::dec << m_hevcPar->UserMaxIFrame << std::endl;
    oss << "UserMaxPBFrame                     = " << std::dec << m_hevcPar->UserMaxPBFrame << std::endl;
    oss << "FrameRateM                         = " << std::dec << m_hevcPar->FrameRateM << std::endl;
    oss << "FrameRateD                         = " << std::dec << m_hevcPar->FrameRateD << std::endl;
    oss << "IntraRefreshEnable                 = " << std::dec << m_hevcPar->IntraRefreshEnable << std::endl;
    oss << "IntraRefreshMode                   = " << std::dec << m_hevcPar->IntraRefreshMode << std::endl;
    oss << "IntraRefreshSizeIn32x32            = " << std::dec << m_hevcPar->IntraRefreshSizeIn32x32 << std::endl;
    oss << "IntraRefreshDeltaQP                = " << std::dec << m_hevcPar->IntraRefreshDeltaQP << std::endl;
    oss << "HMECoarseRefPic                    = " << std::dec << m_hevcPar->HMECoarseRefPic << std::endl;
    oss << "FadeDetectionEnable                = " << std::dec << m_hevcPar->FadeDetectionEnable << std::endl;
    oss << "WeightedPred                       = " << std::dec << m_hevcPar->WeightedPred << std::endl;
    oss << "WeightedBiPred                     = " << std::dec << m_hevcPar->WeightedBiPred << std::endl;
    oss << "RefListModforWeightPred            = " << std::dec << m_hevcPar->RefListModforWeightPred << std::endl;
    oss << "EnableStatistics                   = " << std::dec << m_hevcPar->EnableStatistics << std::endl;
    oss << "OutputQualityType                  = " << std::dec << m_hevcPar->OutputQualityType << std::endl;
    oss << "SliceSizeCtrl                      = " << std::dec << m_hevcPar->SliceSizeCtrl << std::endl;
    oss << "SliceSizeThreshold                 = " << std::dec << m_hevcPar->SliceSizeThreshold << std::endl;
    oss << "MaxSliceSize                       = " << std::dec << m_hevcPar->MaxSliceSize << std::endl;
    oss << "VDEncMode                          = " << std::dec << m_hevcPar->VDEncMode << std::endl;
    oss << "DisableIntraLuma4x4Tu              = " << std::dec << m_hevcPar->DisableIntraLuma4x4Tu << std::endl;
    oss << "HMERef1Disable                     = " << std::dec << m_hevcPar->HMERef1Disable << std::endl;
    oss << "NumBetaPreditors                   = " << std::dec << m_hevcPar->NumBetaPreditors << std::endl;
    oss << "MaxNumImePredictor                 = " << std::dec << m_hevcPar->MaxNumImePredictor << std::endl;
    oss << "NumMergeCandidateCu8x8             = " << std::dec << m_hevcPar->NumMergeCandidateCu8x8 << std::endl;
    oss << "NumMergeCandidateCu16x16           = " << std::dec << m_hevcPar->NumMergeCandidateCu16x16 << std::endl;
    oss << "NumMergeCandidateCu32x32           = " << std::dec << m_hevcPar->NumMergeCandidateCu32x32 << std::endl;
    oss << "NumMergeCandidateCu64x64           = " << std::dec << m_hevcPar->NumMergeCandidateCu64x64 << std::endl;
    oss << "BRCMethod                          = " << std::dec << m_hevcPar->BRCMethod << std::endl;
    oss << "BRCType                            = " << std::dec << m_hevcPar->BRCType << std::endl;
    oss << "SAOEnabledFlag                     = " << std::dec << m_hevcPar->SAOEnabledFlag << std::endl;
    oss << "IntraFrameRDOQEnabledFlag          = " << std::dec << m_hevcPar->IntraFrameRDOQEnabledFlag << std::endl;
    oss << "InterFrameRDOQEnabledFlag          = " << std::dec << m_hevcPar->InterFrameRDOQEnabledFlag << std::endl;
    oss << "StreamInEnable                     = " << std::dec << m_hevcPar->StreamInEn << std::endl;
    oss << "StreamInMvPredictorRef             = " << std::dec << m_hevcPar->StreamInMvPredictorRef << std::endl;
    oss << "StaticFrameZMVPercent              = " << std::dec << m_hevcPar->StaticFrameZMVPercent << std::endl;
    oss << "HMEStreamInRefCost                 = " << std::dec << m_hevcPar->HMEStreamInRefCost << std::endl;
    oss << "IntraPeriod                        = " << std::dec << m_hevcPar->IntraPeriod << std::endl;
    oss << "BGOPSize                           = " << std::dec << m_hevcPar->BGOPSize << std::endl;
    oss << "MaxRefIdxL0                        = " << std::dec << m_hevcPar->MaxRefIdxL0 << std::endl;
    oss << "MaxRefIdxL1                        = " << std::dec << m_hevcPar->MaxRefIdxL1 << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "EncodeSequence",
        "EncodePar",
        CodechalDbgExtType::par);

    std::ofstream ofs(fileName, std::ios::app);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}
 
MOS_STATUS CodechalVdencHevcState::PopulateDdiParam(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    hevcSlcParams)
{
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        CodechalEncodeHevcBase::PopulateDdiParam(
            hevcSeqParams,
            hevcPicParams,
            hevcSlcParams));

    if (m_hevcVdencAcqpEnabled)
    {
        m_hevcPar->BRCMethod = 2;
        m_hevcPar->BRCType = 0;
        m_hevcPar->DisableCuQpAdj = 1;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencHevcState::ModifyEncodedFrameSizeWithFakeHeaderSize(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    uint32_t                            fakeHeaderSizeInByte,
    PMOS_RESOURCE                       resBrcUpdateCurbe,
    uint32_t                            targetSizePos,
    PMOS_RESOURCE                       resPakStat,
    uint32_t                            slcHrdSizePos
)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    //calculate slice headers size
    PCODEC_ENCODER_SLCDATA slcData = m_slcData;
    CODECHAL_ENCODE_CHK_NULL_RETURN(slcData);
    uint32_t totalSliceHeaderSize = 0;
    for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        totalSliceHeaderSize += (slcData->BitSize + 7) >> 3;
        slcData++;
    }

    uint32_t firstHdrSz = 0;
    for (uint32_t i = 0; i < m_encodeParams.uiNumNalUnits; i++)
    {
        firstHdrSz += m_encodeParams.ppNALUnitParams[i]->uiSize;
    }

    totalSliceHeaderSize += firstHdrSz;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddBufferWithIMMValue(
        cmdBuffer,
        resBrcUpdateCurbe,
        targetSizePos,
        fakeHeaderSizeInByte - totalSliceHeaderSize,
        true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddBufferWithIMMValue(
        cmdBuffer,
        resPakStat,
        slcHrdSizePos,
        fakeHeaderSizeInByte * 8,
        true));

    return eStatus;
}
#endif
