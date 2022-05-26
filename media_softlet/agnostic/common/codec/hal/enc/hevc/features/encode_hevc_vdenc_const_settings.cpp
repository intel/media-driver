/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_hevc_vdenc_const_settings.cpp
//! \brief    Defines the common interface for hevc vdenc const settings
//! \details  The encode feature manager is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_hevc_vdenc_const_settings.h"
#include "codec_def_common.h"
#include "codec_def_encode.h"
#include "encode_utils.h"
#include "mhw_vdbox_vdenc_cmdpar.h"
#include "mos_os.h"
#include "mos_solo_generic.h"
#include "mos_utilities.h"
#include "mos_utilities_common.h"
#include <math.h>
#include <stddef.h>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>
#if _ENCODE_RESERVED
#include "mhw_vdbox_vdenc_cmdpar_ext.h"
#endif // _ENCODE_RESERVED

namespace encode
{
// BRC Init/Rest related params
const uint16_t HevcVdencBrcConstSettings::HevcVdencBrcConstSettings_0[] = {
    0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0003, 0x0004, 0x0005,
    0x0006, 0x0008, 0x000A, 0x000C, 0x000F, 0x0013, 0x0018, 0x001E, 0x0026, 0x0030, 0x003D, 0x004D, 0x0061, 0x007A, 0x009A, 0x00C2,
    0x00F4, 0x0133, 0x0183, 0x01E8, 0x0266, 0x0306, 0x03CF, 0x04CD, 0x060C, 0x079F, 0x099A, 0x0C18, 0x0F3D, 0x1333, 0x1831, 0x1E7A,
    0x2666, 0x3062, 0x3CF5, 0x4CCD
};

const uint16_t HevcVdencBrcConstSettings::HevcVdencBrcConstSettings_1[] = {
    0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0004, 0x0005,
    0x0007, 0x0008, 0x000A, 0x000D, 0x0011, 0x0015, 0x001A, 0x0021, 0x002A, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00A6, 0x00D2,
    0x0108, 0x014D, 0x01A3, 0x0210, 0x029A, 0x0347, 0x0421, 0x0533, 0x068D, 0x0841, 0x0A66, 0x0D1A, 0x1082, 0x14CD, 0x1A35, 0x2105,
    0x299A, 0x346A, 0x4209, 0x5333
};

const uint8_t HevcVdencBrcConstSettings::m_estRateThreshP0[7] =
{
    4, 8, 12, 16, 20, 24, 28
};

const uint8_t HevcVdencBrcConstSettings::m_estRateThreshB0[7] =
{
    4, 8, 12, 16, 20, 24, 28
};

const uint8_t HevcVdencBrcConstSettings::m_estRateThreshI0[7] =
{
    4, 8, 12, 16, 20, 24, 28
};

const int8_t HevcVdencBrcConstSettings::m_instRateThreshP0[4] =
{
    40, 60, 80, 120
};

const int8_t HevcVdencBrcConstSettings::m_instRateThreshB0[4] =
{
    35, 60, 80, 120
};

const int8_t HevcVdencBrcConstSettings::m_instRateThreshI0[4] =
{
    40, 60, 90, 115
};

const int8_t HevcVdencBrcConstSettings::m_lowdelayDevThreshPB[] = {
    -45, -33, -23, -15, -8, 0, 15, 25,
};

const int8_t HevcVdencBrcConstSettings::m_lowdelayDevThreshVBR[] = {
    -45, -35, -25, -15, -8, 0, 20, 40,
};
const int8_t HevcVdencBrcConstSettings::m_lowdelayDevThreshI[] = {
    -40, -30, -17, -10, -5, 0, 10, 20,
};

const double HevcVdencBrcConstSettings::m_devThreshIFPNEG[] = {
    0.80, 0.60, 0.34, 0.2,
};

const double HevcVdencBrcConstSettings::m_devThreshIFPPOS[] = {
    0.2, 0.4 , 0.66, 0.9,
};

const double HevcVdencBrcConstSettings::m_devThreshPBFPNEG[] = {
    0.90, 0.66, 0.46, 0.3,
};

const double HevcVdencBrcConstSettings::m_devThreshPBFPPOS[] = {
    0.3, 0.46, 0.70, 0.90,
};

const double HevcVdencBrcConstSettings::m_devThreshVBRNEG[] = {
    0.90, 0.70, 0.50, 0.3,
};

const double HevcVdencBrcConstSettings::m_devThreshVBRPOS[] = {
    0.4, 0.5, 0.75, 0.90,
};

const uint16_t HevcVdencBrcConstSettings::m_startGAdjFrame[4] =
{
    10, 50, 100, 150
};

const uint32_t HevcVdencBrcConstSettings::m_hucConstantData[]  = {
    0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x012c012c, 0x012c012c, 0x012c012c,
    0x012c012c, 0x012c012c, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00640064,
    0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064,
    0x00640064, 0x00640064, 0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x012c012c,
    0x012c012c, 0x012c012c, 0x012c012c, 0x012c012c, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00c800c8,
    0x00c800c8, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064,
    0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x503c1e04, 0xffc88c78, 0x3c1e0400, 0xc88c7850,
    0x140200ff, 0xa0824628, 0x0000ffc8, 0x00000000, 0x04030302, 0x00000000, 0x03030200, 0x0000ff04,
    0x02020000, 0xffff0303, 0x01000000, 0xff020202, 0x0000ffff, 0x02020100, 0x00fffffe, 0x01010000,
    0xfffffe02, 0x010000ff, 0xfefe0201, 0x0000ffff, 0xfe010100, 0x00fffffe, 0x01010000, 0x00000000,
    0x03030200, 0x00000004, 0x03020000, 0x00ff0403, 0x02000000, 0xff030302, 0x000000ff, 0x02020201,
    0x00ffffff, 0x02010000, 0xfffffe02, 0x01000000, 0xfffe0201, 0x0000ffff, 0xfe020101, 0x00fffffe,
    0x01010000, 0xfffffefe, 0x01000000, 0x00000001, 0x03020000, 0x00000403, 0x02000000, 0xff040303,
    0x00000000, 0x03030202, 0x0000ffff, 0x02020100, 0xffffff02, 0x01000000, 0xfffe0202, 0x000000ff,
    0xfe020101, 0x00ffffff, 0x02010100, 0xfffffefe, 0x01000000, 0xfffefe01, 0x000000ff, 0xe0e00101,
    0xc0d0d0d0, 0xe0e0b0c0, 0xd0d0d0e0, 0xf0f0c0d0, 0xd0e0e0e0, 0x0408d0d0, 0xe8f0f800, 0x1820dce0,
    0xf8fc0210, 0x2024ecf0, 0x0008101c, 0x2428f8fc, 0x08101418, 0x2830f800, 0x0c14181c, 0x3040fc00,
    0x0c10141c, 0xe8f80408, 0xc8d0d4e0, 0xf0f8b0c0, 0xccd4d8e0, 0x0000c0c8, 0xd8dce4f0, 0x0408d0d4,
    0xf0f80000, 0x0808dce8, 0xf0f80004, 0x0810dce8, 0x00080808, 0x0810f8fc, 0x08080808, 0x1010f800,
    0x08080808, 0x1020fc00, 0x08080810, 0xfc000408, 0xe0e8f0f8, 0x0001d0d8, 0xe8f0f8fc, 0x0204d8e0,
    0xf8fdff00, 0x0408e8f0, 0xfcff0002, 0x1014f0f8, 0xfcff0004, 0x1418f0f8, 0x00040810, 0x181cf8fc,
    0x04081014, 0x1820f800, 0x04081014, 0x3040fc00, 0x0c10141c, 0x40300408, 0x80706050, 0x30a0a090,
    0x70605040, 0xa0a09080, 0x60504030, 0xa0908070, 0x040201a0, 0x18141008, 0x02012420, 0x0a080604,
    0x01101010, 0x0c080402, 0x10101010, 0x05030201, 0x02010106, 0x00000503, 0xff030201, 0x02010000,
    0x000000ff, 0xfffefe01, 0xfdfd0100, 0xfb00ffff, 0xfffffefd, 0xfefdfbfa, 0x030201ff, 0x01010605,
    0x00050302, 0x03020101, 0x010000ff, 0x0000ff02, 0xffff0100, 0xfe0100ff, 0x00ffffff, 0xfffffefc,
    0xfefcfb00, 0x0101ffff, 0x01050402, 0x04020101, 0x01010000, 0x0000ff02, 0x00ff0101, 0xff000000,
    0x0100ffff, 0xfffffffe, 0xfffefd00, 0xfcfb00ff, 0x1efffffe, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
    0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
    0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
    0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000,
    0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
    0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
    0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
    0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000,
    0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
    0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
    0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
    0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000,
    0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
    0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
    0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
    0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000,
    0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
    0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
    0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
    0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff
};

const uint16_t HevcVdencBrcConstSettings::HevcVdencBrcConstSettings_2[] = {
    0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0004, 0x0004,
    0x0005, 0x0006, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000C, 0x000E, 0x0010, 0x0012, 0x0014, 0x0016, 0x0019, 0x001C,
    0x001F, 0x0023, 0x0027, 0x002C, 0x0032, 0x0038, 0x003E, 0x0046, 0x004F, 0x0058, 0x0063, 0x006F, 0x007D, 0x008C, 0x009D, 0x00B1,
    0x00C6, 0x00DF, 0x00FA, 0x0118
};

const uint16_t HevcVdencBrcConstSettings::HevcVdencBrcConstSettings_3[] = {
    0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003, 0x0004, 0x0004, 0x0005,
    0x0005, 0x0006, 0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B, 0x000D, 0x000E, 0x0010, 0x0012, 0x0014, 0x0017, 0x001A, 0x001D,
    0x0021, 0x0024, 0x0029, 0x002E, 0x0034, 0x003A, 0x0041, 0x0049, 0x0052, 0x005C, 0x0067, 0x0074, 0x0082, 0x0092, 0x00A4, 0x00B8,
    0x00CE, 0x00E8, 0x0104, 0x0124
};

const uint32_t HevcVdencBrcConstSettings::HevcVdencBrcConstSettings_7[] = {
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

const uint32_t HevcVdencBrcConstSettings::HevcVdencBrcConstSettings_8[] = {
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

const int8_t HevcVdencBrcConstSettings::HevcVdencBrcConstSettings_4[9][8] = {
    { 0,  0, -8, -12, -16, -20, -28, -36 },
{ 0,  0, -4, -8, -12,  -16, -24, -32 },
{ 4,  2,  0, -1, -3,  -8, -16, -24 },
{ 8,  4,  2,  0, -1,  -4,  -8, -16 },
{ 20, 16,  4,  0, -1,  -4,  -8, -16 },
{ 24, 20, 16,  8,  4,   0,  -4, -8 },
{ 28, 24, 20, 16,  8,   4,  0, -8 },
{ 32, 24, 20, 16, 8,   4,   0, -4 },
{ 64, 48, 28, 20, 16,  12,  8,  4 },
};

const int8_t HevcVdencBrcConstSettings::HevcVdencBrcConstSettings_5[9][8] = {
    { -8,  -24, -32, -40, -44, -48, -52, -80 },
{ -8,  -16, -32, -40, -40,  -44, -44, -56 },
{ 0,    0,  -12, -20, -24,  -28, -32, -36 },
{ 8,   4,  0,   0,    -8,   -16,  -24, -32 },
{ 32,  16,  8, 4,    -4,   -8,  -16,  -20 },
{ 36,  24,  16, 8,    4,    -2,  -4, -8 },
{ 40, 36, 24,   20, 16,  8,  0, -8 },
{ 48, 40, 28,  24, 20,  12,  0, -4 },
{ 64, 48, 28, 20, 16,  12,  8,  4 },
};

const int8_t HevcVdencBrcConstSettings::HevcVdencBrcConstSettings_6[9][8] = {
    { 0, -4, -8, -16, -24, -32, -40, -48 },
{ 1,  0, -4, -8, -16,  -24, -32, -40 },
{ 4,  2,  0, -1, -3,  -8, -16, -24 },
{ 8,  4,  2,  0, -1,  -4,  -8, -16 },
{ 20, 16,  4,  0, -1,  -4,  -8, -16 },
{ 24, 20, 16,  8,  4,   0,  -4, -8 },
{ 28, 24, 20, 16,  8,   4,  0, -8 },
{ 32, 24, 20, 16, 8,   4,   0, -4 },
{ 64, 48, 28, 20, 16,  12,  8,  4 },
};

const uint8_t HevcVdencBrcConstSettings::m_rateRatioThreshold[7] =
{
    40, 75, 97, 103, 125, 160, 0
};
const uint8_t HevcVdencBrcConstSettings::m_startGAdjMult[5] =
{
    1, 1, 3, 2, 1
};

const uint8_t HevcVdencBrcConstSettings::m_startGAdjDiv[5] =
{
    40, 5, 5, 3, 1
};

const uint8_t HevcVdencBrcConstSettings::m_rateRatioThresholdQP[8] =
{
    253, 254, 255, 0, 1, 2, 3, 0
};

EncodeHevcVdencConstSettings::EncodeHevcVdencConstSettings()
{
    m_featureSetting = MOS_New(HevcVdencFeatureSettings);
}

EncodeHevcVdencConstSettings::~EncodeHevcVdencConstSettings()
{
    auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
    MOS_Delete(setting);
    m_featureSetting = nullptr;
}

MOS_STATUS EncodeHevcVdencConstSettings::PrepareConstSettings()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(SetTUSettings());
    ENCODE_CHK_STATUS_RETURN(SetCommonSettings());
    ENCODE_CHK_STATUS_RETURN(SetVdencStreaminStateSettings());
    ENCODE_CHK_STATUS_RETURN(SetVdencCmd1Settings());
    ENCODE_CHK_STATUS_RETURN(SetVdencCmd2Settings());
    ENCODE_CHK_STATUS_RETURN(SetBrcSettings());

    if (m_osItf != nullptr)
    {
        // To enable rounding precision here
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_ROUNDING_ENABLE_ID,
            &userFeatureData,
            m_osItf->pOsContext);
        m_hevcVdencRoundingPrecisionEnabled = userFeatureData.i32Data ? true : false;

#if (_DEBUG || _RELEASE_INTERNAL)
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_ROUNDING_ENABLE_ID, m_hevcVdencRoundingPrecisionEnabled, m_osItf->pOsContext);
#endif
/*
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_RDO_ENABLE_ID,
            &userFeatureData,
            m_osItf->pOsContext);
        m_hevcRdoqEnabled = (userFeatureData.i32Data) ? true : false;
#if (_DEBUG || _RELEASE_INTERNAL)
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_HEVC_RDO_ENABLE_ID, m_hevcRdoqEnabled, m_osItf->pOsContext);
#endif
*/
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencConstSettings::Update(void *params)
{
    ENCODE_FUNC_CALL();

    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams =
        static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    m_hevcSeqParams = hevcSeqParams;

    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams =
        static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);
    m_hevcPicParams = hevcPicParams;

    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSliceParams =
        static_cast<PCODEC_HEVC_ENCODE_SLICE_PARAMS>(encodeParams->pSliceParams);
    ENCODE_CHK_NULL_RETURN(hevcSliceParams);
    m_hevcSliceParams = hevcSliceParams;

    return MOS_STATUS_SUCCESS;
}

#define CLIP3(MIN_, MAX_, X) (((X) < (MIN_)) ? (MIN_) : (((X) > (MAX_)) ? (MAX_) : (X)))

MOS_STATUS EncodeHevcVdencConstSettings::SetTUSettings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->rdoqEnable         = {true, true, true, true, true, true, true, false};
    setting->acqpEnable         = {true, true, true, true, true, true, true, false};

    return eStatus;
}

MOS_STATUS EncodeHevcVdencConstSettings::SetCommonSettings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->transformSkipCoeffsTable = {{
        {{
            {{
                {{
                    {42, 37}, {32, 40}
                }},
                {{
                    {40, 40}, {32, 45}
                }}
            }},
            {{
                {{
                    {29, 48}, {26, 53}
                }},
                {{
                    {26, 56}, {24, 62}
                }}
            }}
        }},
        {{
            {{
                {{
                    {42, 40}, {32, 45}
                }},
                {{
                    {40, 46}, {32, 48}
                }}
            }},
            {{
                {{
                    {26, 53}, {24, 58}
                }},
                {{
                    {32, 53}, {26, 64}
                }}
            }}
        }},
        {{
            {{
                {{
                    {38, 42}, {32, 51}
                }},
                {{
                    {43, 43}, {35, 46}
                }}
            }},
            {{
                {{
                    {26, 56}, {24, 64}
                }},
                {{
                    {35, 50}, {32, 57}
                }}
            }}
        }},
        {{
            {{
                {{
                    {35, 46}, {32, 52}
                }},
                {{
                    {51, 42}, {38, 53}
                }}
            }},
            {{
                {{
                    {29, 56}, {29, 70}
                }},
                {{
                    {38, 47}, {37, 64}
                }}
            }}
        }},
    }};

    setting->transformSkipLambdaTable = {
    149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149,
    149, 149, 149, 149, 149, 149, 149, 149, 149, 162, 174, 186, 199, 211, 224, 236,
    249, 261, 273, 286, 298, 298, 298, 298, 298, 298, 298, 298, 298, 298, 298, 298,
    298, 298, 298, 298
    };

    setting->rdoqLamdas8bits = {{
    {{
        {{
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005,
                0x0007, 0x0008, 0x000a, 0x000d, 0x0011, 0x0015, 0x001a, 0x0021,
                0x002a, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00a6, 0x00d2,
                0x0108, 0x014d, 0x01a3, 0x0210, 0x029a, 0x0347, 0x0421, 0x0533,
                0x068d, 0x0841, 0x0a66, 0x0d1a, 0x1082, 0x14cd, 0x1a35, 0x2105,
                0x299a, 0x346a, 0x4209, 0x5333
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005,
                0x0007, 0x0008, 0x000a, 0x000d, 0x0011, 0x0015, 0x001a, 0x0021,
                0x002a, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00a6, 0x00d2,
                0x0108, 0x014d, 0x01a3, 0x0210, 0x029a, 0x0347, 0x0421, 0x0533,
                0x068d, 0x0841, 0x0a66, 0x0d1a, 0x1082, 0x14cd, 0x1a35, 0x2105,
                0x299a, 0x346a, 0x4209, 0x5333
            },
        }},
        {{
            {   //Inter Luma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }},
    {{
        {{
            {   //Intra Luma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Intra Chroma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
        {{
            {   //Inter Luma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }}
}};

    setting->rdoqLamdas10bits = {{
    {{
        {{
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002,
                0x0003, 0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000a, 0x000d,
                0x0011, 0x0015, 0x001a, 0x0021, 0x002a, 0x0034, 0x0042, 0x0053,
                0x0069, 0x0084, 0x00a6, 0x00d2, 0x0108, 0x014d, 0x01a3, 0x0210,
                0x029a, 0x0347, 0x0421, 0x0533, 0x068d, 0x0841, 0x0a66, 0x0d1a,
                0x1082, 0x14cd, 0x1a35, 0x2105, 0x299a, 0x346a, 0x4209, 0x5333
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002,
                0x0003, 0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000a, 0x000d,
                0x0011, 0x0015, 0x001a, 0x0021, 0x002a, 0x0034, 0x0042, 0x0053,
                0x0069, 0x0084, 0x00a6, 0x00d2, 0x0108, 0x014d, 0x01a3, 0x0210,
                0x029a, 0x0347, 0x0421, 0x0533, 0x068d, 0x0841, 0x0a66, 0x0d1a,
                0x1082, 0x14cd, 0x1a35, 0x2105, 0x299a, 0x346a, 0x4209, 0x5333
            },
        }},
        {{
            {   //Inter Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0006, 0x0007,
                0x0009, 0x000b, 0x000e, 0x0012, 0x0016, 0x001c, 0x0023, 0x002c,
                0x0038, 0x0046, 0x0059, 0x0075, 0x009b, 0x00cc, 0x010c, 0x0160,
                0x01cd, 0x025b, 0x0314, 0x0405, 0x053d, 0x06d2, 0x08df, 0x0b2d,
                0x0e14, 0x11bd, 0x165a, 0x1c29, 0x237b, 0x2cb4, 0x3852, 0x46f5,
                0x5967, 0x70a4, 0x8deb, 0xb2ce, 0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005, 0x0007,
                0x0008, 0x000b, 0x000d, 0x0011, 0x0015, 0x001b, 0x0021, 0x002a,
                0x0035, 0x0043, 0x0054, 0x006c, 0x008c, 0x00b4, 0x00e7, 0x0129,
                0x017d, 0x01ea, 0x0275, 0x0327, 0x040c, 0x0530, 0x06a7, 0x0862,
                0x0a8f, 0x0d4e, 0x10c3, 0x151f, 0x1a9c, 0x2187, 0x2a3d, 0x3538,
                0x430d, 0x547b, 0x6a70, 0x861b, 0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }},
    {{
        {{
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0006, 0x0007,
                0x0009, 0x000b, 0x000e, 0x0012, 0x0016, 0x001c, 0x0023, 0x002c,
                0x0038, 0x0046, 0x0059, 0x0075, 0x009b, 0x00cc, 0x010c, 0x0160,
                0x01cd, 0x025b, 0x0314, 0x0405, 0x053d, 0x06d2, 0x08df, 0x0b2d,
                0x0e14, 0x11bd, 0x165a, 0x1c29, 0x237b, 0x2cb4, 0x3852, 0x46f5,
                0x5967, 0x70a4, 0x8deb, 0xb2ce, 0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005, 0x0007,
                0x0008, 0x000b, 0x000d, 0x0011, 0x0015, 0x001b, 0x0021, 0x002a,
                0x0035, 0x0043, 0x0054, 0x006c, 0x008c, 0x00b4, 0x00e7, 0x0129,
                0x017d, 0x01ea, 0x0275, 0x0327, 0x040c, 0x0530, 0x06a7, 0x0862,
                0x0a8f, 0x0d4e, 0x10c3, 0x151f, 0x1a9c, 0x2187, 0x2a3d, 0x3538,
                0x430d, 0x547b, 0x6a70, 0x861b, 0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
        {{
            {   //Inter Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0006, 0x0007,
                0x0009, 0x000b, 0x000e, 0x0012, 0x0016, 0x001c, 0x0023, 0x002c,
                0x0038, 0x0046, 0x0059, 0x0075, 0x009b, 0x00cc, 0x010c, 0x0160,
                0x01cd, 0x025b, 0x0314, 0x0405, 0x053d, 0x06d2, 0x08df, 0x0b2d,
                0x0e14, 0x11bd, 0x165a, 0x1c29, 0x237b, 0x2cb4, 0x3852, 0x46f5,
                0x5967, 0x70a4, 0x8deb, 0xb2ce, 0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005, 0x0007,
                0x0008, 0x000b, 0x000d, 0x0011, 0x0015, 0x001b, 0x0021, 0x002a,
                0x0035, 0x0043, 0x0054, 0x006c, 0x008c, 0x00b4, 0x00e7, 0x0129,
                0x017d, 0x01ea, 0x0275, 0x0327, 0x040c, 0x0530, 0x06a7, 0x0862,
                0x0a8f, 0x0d4e, 0x10c3, 0x151f, 0x1a9c, 0x2187, 0x2a3d, 0x3538,
                0x430d, 0x547b, 0x6a70, 0x861b, 0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }}
}};

    setting->rdoqLamdas12bits = {{
    {{
        {{
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005,
                0x0007, 0x0008, 0x000a, 0x000d, 0x0011, 0x0015, 0x001a, 0x0021,
                0x002a, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00a6, 0x00d2,
                0x0108, 0x014d, 0x01a3, 0x0210, 0x029a, 0x0347, 0x0421, 0x0533,
                0x068d, 0x0841, 0x0a66, 0x0d1a, 0x1082, 0x14cd, 0x1a35, 0x2105,
                0x299a, 0x346a, 0x4209, 0x5333
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005,
                0x0007, 0x0008, 0x000a, 0x000d, 0x0011, 0x0015, 0x001a, 0x0021,
                0x002a, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00a6, 0x00d2,
                0x0108, 0x014d, 0x01a3, 0x0210, 0x029a, 0x0347, 0x0421, 0x0533,
                0x068d, 0x0841, 0x0a66, 0x0d1a, 0x1082, 0x14cd, 0x1a35, 0x2105,
                0x299a, 0x346a, 0x4209, 0x5333
            },
        }},
        {{
            {   //Inter Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }},
    {{
        {{
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
        {{
            {   //Inter Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }}
}};

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencConstSettings::SetVdencStreaminStateSettings()
{
    ENCODE_FUNC_CALL();

    auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencStreaminStateSettings.emplace_back(
        VDENC_STREAMIN_STATE_LAMBDA() {
            static const std::array<
                std::array<
                    uint8_t,
                    NUM_TARGET_USAGE_MODES + 1>,
                4>
                numMergeCandidates = {{
                    {0, 1, 1, 1, 1, 1, 2, 2},
                    {0, 2, 2, 2, 2, 2, 2, 2},
                    {0, 3, 3, 3, 3, 3, 2, 2},
                    {0, 4, 4, 4, 4, 4, 2, 2},
                }};

            static const std::array<
                uint8_t,
                NUM_TARGET_USAGE_MODES + 1>
                numImePredictors = {0, 8, 8, 8, 8, 8, 4, 4};

            par.maxTuSize                = 3;  //Maximum TU Size allowed, restriction to be set to 3
            par.maxCuSize                = (cu64Align) ? 3 : 2;
            par.numMergeCandidateCu64x64 = numMergeCandidates[3][m_hevcSeqParams->TargetUsage];
            par.numMergeCandidateCu32x32 = numMergeCandidates[2][m_hevcSeqParams->TargetUsage];
            par.numMergeCandidateCu16x16 = numMergeCandidates[1][m_hevcSeqParams->TargetUsage];
            par.numMergeCandidateCu8x8   = numMergeCandidates[0][m_hevcSeqParams->TargetUsage];
            par.numImePredictors         = numImePredictors[m_hevcSeqParams->TargetUsage];

            auto waTable = m_osItf == nullptr ? nullptr : m_osItf->pfnGetWaTable(m_osItf);
            if (waTable)
            {
                if (MEDIA_IS_WA(waTable, WaHEVCVDEncROINumMergeCandidateSetting) && m_hevcSeqParams->TargetUsage == 4)
                {
                    par.numMergeCandidateCu64x64 = 3;
                    par.numMergeCandidateCu32x32 = 3;
                    par.numMergeCandidateCu16x16 = 2;
                    par.numMergeCandidateCu8x8   = 1;
                }

                ENCODE_CHK_NULL_RETURN(m_osItf);
                if (MEDIA_IS_WA(waTable, Wa_22011549751) && m_hevcPicParams->CodingType == I_TYPE && !m_osItf->bSimIsActive && !Mos_Solo_Extension(m_osItf->pOsContext) && !m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
                {
                    par.numMergeCandidateCu64x64 = 0;
                    par.numMergeCandidateCu32x32 = 0;
                    par.numMergeCandidateCu16x16 = 0;
                    par.numMergeCandidateCu8x8   = 2;
                    par.numImePredictors         = 0;
                }
            }

            return MOS_STATUS_SUCCESS;
        });

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencConstSettings::SetVdencCmd1Settings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencCmd1Settings = {
        VDENC_CMD1_LAMBDA()
        {
            static constexpr std::array<std::array<double, 4>, 2> ConstTable1 =
            {{
                {0.68445, 1.03428, 1.17, 1.17},
                {0.7605, 0.9464, 0.9464, 1.04}
            }};
    
            static constexpr std::array<double, 13> LowDelayTable =
            {
                0.7048, 0.763533, 0.822267, 0.881, 0.939733, 0.998467,
                1.0572, 1.11593, 1.17467, 1.2334, 1.29213, 1.35087, 1.4096
            };

            static constexpr std::array<double, 52> ConstTable2 =
            {
                1.000000,  1.000000,  1.000000,  1.000000,  1.000000, 
                1.000000,  1.000000,  1.000000,  1.000000,  1.000000,
                1.000000,  1.000000,  1.259921,  1.587401,  2.000000, 
                2.519842,  3.174802,  4.000000,  5.039684,  6.349604, 
                8.000000,  10.079368, 12.699208, 16.000000, 20.158737,
                25.398417, 32.000000, 40.317474, 50.796834, 64.000000, 
                80.634947, 101.593667, 128.0000, 161.269894, 203.187335,
                256.00000, 322.539789, 406.374669, 512.0000, 645.079578,
                812.749339, 1024.0000, 1290.159155, 1625.498677, 2048.0, 
                2580.31831, 3250.997354, 4096.0000, 5160.636620, 6501.994709, 8192
            };
    
            double doubleNum0;

            uint32_t bGopSize = m_hevcSeqParams->GopRefDist;
            int32_t  depth    = m_hevcPicParams->HierarchLevelPlus1 ? m_hevcPicParams->HierarchLevelPlus1 - 1 : 0;
            uint8_t  qp       = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
    
            if (m_hevcSeqParams->LowDelayMode)
            {
                if (m_hevcPicParams->CodingType == I_TYPE)
                {
                    doubleNum0 = 0.4845;
                }
                else
                {
                    if (depth == 0)
                    {
                        doubleNum0 = 0.578;
                    }
                    else
                    {
                        int tmp = CLIP3(24, 36, qp);
                        doubleNum0 = LowDelayTable[tmp - 24];
                    }
                }
            }
            else
            {
                if (m_hevcPicParams->CodingType == I_TYPE)
                {
                    doubleNum0 = 0.60;
                }
                else if (m_hevcPicParams->CodingType == B_TYPE && bGopSize == 4)
                {
                    doubleNum0 = ConstTable1[0][depth];
                }
                else if (m_hevcPicParams->CodingType == B_TYPE && bGopSize == 8)
                {
                    doubleNum0 = ConstTable1[1][depth];
                }
                else
                {
                    doubleNum0 = 0.65;
                }
            }

            double doubleNum1 = doubleNum0 * ConstTable2[qp - 1];
            par.vdencCmd1Par0 = (uint16_t)(MOS_MIN(65535, doubleNum1 * 4 + 0.5));

            doubleNum1 = sqrt(doubleNum1);
            par.vdencCmd1Par1 = (uint16_t)(MOS_MIN(65535, doubleNum1 * 4 + 0.5));

            par.vdencCmd1Par2[0] = 0;
            par.vdencCmd1Par2[1] = 2;
            par.vdencCmd1Par2[2] = 3;
            par.vdencCmd1Par2[3] = 5;
            par.vdencCmd1Par2[4] = 6;
            par.vdencCmd1Par2[5] = 8;
            par.vdencCmd1Par2[6] = 9;
            par.vdencCmd1Par2[7] = 11;

            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            static const std::array<uint8_t, 12> data = {
                4, 12, 20, 28, 36, 44, 52, 60, 68, 76, 84, 92
            };

            for (size_t i = 0; i < data.size(); i++)
            {
                par.vdencCmd1Par3[i] = data[i];
                par.vdencCmd1Par4[i] = data[i];
            }

            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par22 = 4;

            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par24 = 0;
            par.vdencCmd1Par25 = 0;
            par.vdencCmd1Par26 = 0;
            par.vdencCmd1Par27 = 0;
            par.vdencCmd1Par28 = 0;
            par.vdencCmd1Par29 = 0;
            par.vdencCmd1Par30 = 0;
            par.vdencCmd1Par31 = 0;

            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par32 = 0;
            par.vdencCmd1Par33 = 0;

            if (m_hevcPicParams->CodingType == I_TYPE)
            {
                par.vdencCmd1Par34 = 21;
                par.vdencCmd1Par35 = 0;
            }
            else
            {
                par.vdencCmd1Par34 = 7;
                par.vdencCmd1Par35 = 4;
            }

            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par44 = 0;
            par.vdencCmd1Par45 = 20;
            par.vdencCmd1Par46 = 0;

            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par47 = 12;
            par.vdencCmd1Par48 = 12;
            par.vdencCmd1Par49 = 12;
            par.vdencCmd1Par50 = 12;
            par.vdencCmd1Par51 = 12;
            par.vdencCmd1Par52 = 12;
            par.vdencCmd1Par53 = 12;
            par.vdencCmd1Par54 = 12;

            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par55 = 0x10;
            par.vdencCmd1Par56 = 0x10;
            par.vdencCmd1Par57 = 0x10;
            par.vdencCmd1Par58 = 0x10;
            par.vdencCmd1Par59 = 0x10;
            par.vdencCmd1Par60 = 0x10;
            par.vdencCmd1Par61 = 0x10;
            par.vdencCmd1Par62 = 0x10;
            par.vdencCmd1Par63 = 0x10;
            par.vdencCmd1Par64 = 0x10;
            par.vdencCmd1Par65 = 0x10;
            par.vdencCmd1Par66 = 0x10;
            par.vdencCmd1Par67 = 0x10;
            par.vdencCmd1Par68 = 0x10;
            par.vdencCmd1Par69 = 0x10;
            par.vdencCmd1Par70 = 0x10;
            par.vdencCmd1Par71 = 0x10;
            par.vdencCmd1Par72 = 0x10;
            par.vdencCmd1Par73 = 0x10;
            par.vdencCmd1Par74 = 0x10;
            par.vdencCmd1Par75 = 0x10;
            par.vdencCmd1Par76 = 0x10;
            par.vdencCmd1Par77 = 0x10;
            par.vdencCmd1Par78 = 0x10;
            par.vdencCmd1Par79 = 0x10;
            par.vdencCmd1Par80 = 0x10;
            par.vdencCmd1Par81 = 0x10;
            par.vdencCmd1Par82 = 0x10;
            par.vdencCmd1Par83 = 0x10;
            par.vdencCmd1Par84 = 0x10;
            par.vdencCmd1Par85 = 0x10;
            par.vdencCmd1Par86 = 0x10;

            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            if (m_hevcPicParams->CodingType == I_TYPE)
            {
                par.vdencCmd1Par87 = 16;
                par.vdencCmd1Par88 = 16;
                par.vdencCmd1Par89 = 47;
            }
            else if (m_hevcPicParams->CodingType == B_TYPE)
            {
                par.vdencCmd1Par87 = 20;
                par.vdencCmd1Par88 = 20;
                par.vdencCmd1Par89 = 20;
            }

            return MOS_STATUS_SUCCESS;
        }
    };

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencConstSettings::SetBrcSettings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->brcSettings.HevcVdencBrcSettings_0.data = (void *)m_brcSettings.HevcVdencBrcConstSettings_0;
    setting->brcSettings.HevcVdencBrcSettings_0.size = sizeof(m_brcSettings.HevcVdencBrcConstSettings_0);
    setting->brcSettings.HevcVdencBrcSettings_1.data = (void *)m_brcSettings.HevcVdencBrcConstSettings_1;
    setting->brcSettings.HevcVdencBrcSettings_1.size = sizeof(m_brcSettings.HevcVdencBrcConstSettings_1);

    setting->brcSettings.estRateThreshP0.data = (void *)m_brcSettings.m_estRateThreshP0;
    setting->brcSettings.estRateThreshP0.size = sizeof(m_brcSettings.m_estRateThreshP0);

    setting->brcSettings.estRateThreshB0.data = (void *)m_brcSettings.m_estRateThreshB0;
    setting->brcSettings.estRateThreshB0.size = sizeof(m_brcSettings.m_estRateThreshB0);

    setting->brcSettings.estRateThreshI0.data = (void *)m_brcSettings.m_estRateThreshI0;
    setting->brcSettings.estRateThreshI0.size = sizeof(m_brcSettings.m_estRateThreshI0);

    setting->brcSettings.instRateThreshP0.data = (void *)m_brcSettings.m_instRateThreshP0;
    setting->brcSettings.instRateThreshP0.size = sizeof(m_brcSettings.m_instRateThreshP0);

    setting->brcSettings.instRateThreshB0.data = (void *)m_brcSettings.m_instRateThreshB0;
    setting->brcSettings.instRateThreshB0.size = sizeof(m_brcSettings.m_instRateThreshB0);

    setting->brcSettings.instRateThreshI0.data = (void *)m_brcSettings.m_instRateThreshI0;
    setting->brcSettings.instRateThreshI0.size = sizeof(m_brcSettings.m_instRateThreshI0);

    setting->brcSettings.devThreshIFPNEG.data = (void *)m_brcSettings.m_devThreshIFPNEG;
    setting->brcSettings.devThreshIFPNEG.size = sizeof(m_brcSettings.m_devThreshIFPNEG);

    setting->brcSettings.devThreshIFPPOS.data = (void *)m_brcSettings.m_devThreshIFPPOS;
    setting->brcSettings.devThreshIFPPOS.size = sizeof(m_brcSettings.m_devThreshIFPPOS);

    setting->brcSettings.devThreshPBFPNEG.data = (void *)m_brcSettings.m_devThreshPBFPNEG;
    setting->brcSettings.devThreshPBFPNEG.size = sizeof(m_brcSettings.m_devThreshPBFPNEG);

    setting->brcSettings.devThreshPBFPPOS.data = (void *)m_brcSettings.m_devThreshPBFPPOS;
    setting->brcSettings.devThreshPBFPPOS.size = sizeof(m_brcSettings.m_devThreshPBFPPOS);

    setting->brcSettings.devThreshVBRNEG.data = (void *)m_brcSettings.m_devThreshVBRNEG;
    setting->brcSettings.devThreshVBRNEG.size = sizeof(m_brcSettings.m_devThreshVBRNEG);

    setting->brcSettings.devThreshVBRPOS.data = (void *)m_brcSettings.m_devThreshVBRPOS;
    setting->brcSettings.devThreshVBRPOS.size = sizeof(m_brcSettings.m_devThreshVBRPOS);

    setting->brcSettings.lowdelayDevThreshPB.data = (void *)m_brcSettings.m_lowdelayDevThreshPB;
    setting->brcSettings.lowdelayDevThreshPB.size = sizeof(m_brcSettings.m_lowdelayDevThreshPB);

    setting->brcSettings.lowdelayDevThreshVBR.data = (void *)m_brcSettings.m_lowdelayDevThreshVBR;
    setting->brcSettings.lowdelayDevThreshVBR.size = sizeof(m_brcSettings.m_lowdelayDevThreshVBR);

    setting->brcSettings.lowdelayDevThreshI.data = (void *)m_brcSettings.m_lowdelayDevThreshI;
    setting->brcSettings.lowdelayDevThreshI.size = sizeof(m_brcSettings.m_lowdelayDevThreshI);

    setting->brcSettings.startGAdjFrame.data = (void *)m_brcSettings.m_startGAdjFrame;
    setting->brcSettings.startGAdjFrame.size = sizeof(m_brcSettings.m_startGAdjFrame);

    setting->brcSettings.numDevThreshlds = m_brcSettings.m_numDevThreshlds;
    setting->brcSettings.devStdFPS = m_brcSettings.m_devStdFPS;
    setting->brcSettings.bpsRatioLow = m_brcSettings.m_bpsRatioLow;
    setting->brcSettings.bpsRatioHigh = m_brcSettings.m_bpsRatioHigh;
    setting->brcSettings.postMultPB = m_brcSettings.m_postMultPB;
    setting->brcSettings.negMultPB = m_brcSettings.m_negMultPB;
    setting->brcSettings.posMultVBR = m_brcSettings.m_posMultVBR;
    setting->brcSettings.negMultVBR = m_brcSettings.m_negMultVBR;

    setting->brcSettings.hucConstantData.data = (void *)m_brcSettings.m_hucConstantData;
    setting->brcSettings.hucConstantData.size = sizeof(m_brcSettings.m_hucConstantData);

    setting->brcSettings.HevcVdencBrcSettings_2.data = (void *)m_brcSettings.HevcVdencBrcConstSettings_2;
    setting->brcSettings.HevcVdencBrcSettings_2.size = sizeof(m_brcSettings.HevcVdencBrcConstSettings_2);

    setting->brcSettings.HevcVdencBrcSettings_3.data = (void *)m_brcSettings.HevcVdencBrcConstSettings_3;
    setting->brcSettings.HevcVdencBrcSettings_3.size = sizeof(m_brcSettings.HevcVdencBrcConstSettings_3);

    setting->brcSettings.HevcVdencBrcSettings_7.data = (void *)m_brcSettings.HevcVdencBrcConstSettings_7;
    setting->brcSettings.HevcVdencBrcSettings_7.size = sizeof(m_brcSettings.HevcVdencBrcConstSettings_7);

    setting->brcSettings.HevcVdencBrcSettings_8.data = (void *)m_brcSettings.HevcVdencBrcConstSettings_8;
    setting->brcSettings.HevcVdencBrcSettings_8.size = sizeof(m_brcSettings.HevcVdencBrcConstSettings_8);

    setting->brcSettings.HevcVdencBrcSettings_4 = (int8_t(*)[9][8])m_brcSettings.HevcVdencBrcConstSettings_4;

    setting->brcSettings.HevcVdencBrcSettings_5 = (int8_t(*)[9][8])m_brcSettings.HevcVdencBrcConstSettings_5;

    setting->brcSettings.HevcVdencBrcSettings_6 = (int8_t(*)[9][8])m_brcSettings.HevcVdencBrcConstSettings_6;

    setting->brcSettings.rateRatioThreshold.data = (void *)m_brcSettings.m_rateRatioThreshold;
    setting->brcSettings.rateRatioThreshold.size = sizeof(m_brcSettings.m_rateRatioThreshold);

    setting->brcSettings.startGAdjMult.data = (void *)m_brcSettings.m_startGAdjMult;
    setting->brcSettings.startGAdjMult.size = sizeof(m_brcSettings.m_startGAdjMult);

    setting->brcSettings.startGAdjDiv.data = (void *)m_brcSettings.m_startGAdjDiv;
    setting->brcSettings.startGAdjDiv.size = sizeof(m_brcSettings.m_startGAdjDiv);

    setting->brcSettings.rateRatioThresholdQP.data = (void *)m_brcSettings.m_rateRatioThresholdQP;
    setting->brcSettings.rateRatioThresholdQP.size = sizeof(m_brcSettings.m_rateRatioThresholdQP);

    setting->brcSettings.topFrmSzThrForAdapt2Pass_U8 = m_brcSettings.m_topFrmSzThrForAdapt2Pass_U8;
    setting->brcSettings.botFrmSzThrForAdapt2Pass_U8 = m_brcSettings.m_botFrmSzThrForAdapt2Pass_U8;

    setting->brcSettings.topQPDeltaThrForAdapt2Pass_U8 = m_brcSettings.m_topQPDeltaThrForAdapt2Pass_U8;
    setting->brcSettings.botQPDeltaThrForAdapt2Pass_U8 = m_brcSettings.m_botQPDeltaThrForAdapt2Pass_U8;

    setting->brcSettings.deltaQPForSadZone0_S8 = m_brcSettings.m_deltaQPForSadZone0_S8;
    setting->brcSettings.deltaQPForSadZone1_S8 = m_brcSettings.m_deltaQPForSadZone1_S8;
    setting->brcSettings.deltaQPForSadZone2_S8 = m_brcSettings.m_deltaQPForSadZone2_S8;
    setting->brcSettings.deltaQPForSadZone3_S8 = m_brcSettings.m_deltaQPForSadZone3_S8;
    setting->brcSettings.deltaQPForMvZero_S8 = m_brcSettings.m_deltaQPForMvZero_S8;
    setting->brcSettings.deltaQPForMvZone0_S8 = m_brcSettings.m_deltaQPForMvZone0_S8;
    setting->brcSettings.deltaQPForMvZone1_S8 = m_brcSettings.m_deltaQPForMvZone1_S8;
    setting->brcSettings.deltaQPForMvZone2_S8 = m_brcSettings.m_deltaQPForMvZone2_S8;

    setting->brcSettings.reEncodePositiveQPDeltaThr_S8 = m_brcSettings.m_reEncodePositiveQPDeltaThr_S8;
    setting->brcSettings.reEncodeNegativeQPDeltaThr_S8 = m_brcSettings.m_reEncodeNegativeQPDeltaThr_S8;
    setting->brcSettings.sceneChgPrevIntraPctThreshold_U8 = m_brcSettings.m_sceneChgPrevIntraPctThreshold_U8;
    setting->brcSettings.sceneChgCurIntraPctThreshold_U8 = m_brcSettings.m_sceneChgCurIntraPctThreshold_U8;

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
