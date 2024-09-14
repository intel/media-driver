/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     encode_hevc_vdenc_const_settings_xe2_lpm_base_open.h
//! \brief    Defines the opensource hevc cmd2 const settings
//! \details  The encode feature manager is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#ifdef VDENC_CMD2_SETTINGS_OPEN
setting->vdencCmd2Settings.emplace_back(
VDENC_CMD2_LAMBDA()
{
    par.extSettings.emplace_back(
        [this, isLowDelay, &par](uint32_t *data) {

            uint32_t CodingTypeMinus1              = m_hevcPicParams->CodingType - 1;
            uint32_t tu                            = m_hevcSeqParams->TargetUsage;
            uint32_t numL0Minus1Is0                = m_hevcSliceParams->num_ref_idx_l0_active_minus1 == 0;
            uint32_t lowDelay                      = isLowDelay;
            uint32_t currPicRef                    = m_hevcPicParams->pps_curr_pic_ref_enabled_flag;
            uint32_t paletteMode                   = m_hevcSeqParams->palette_mode_enabled_flag;
            uint32_t depthMinus8                   = m_hevcSeqParams->bit_depth_luma_minus8;
            uint32_t rdoq                          = m_hevcRdoqEnabled;
            uint32_t numRef0                       = par.numRefL0;
            uint32_t numRef1                       = par.numRefL1;
            uint32_t LowDelayMode                  = m_hevcSeqParams->LowDelayMode;
            uint32_t chroma_format_idc             = m_hevcSeqParams->chroma_format_idc;

            static const uint32_t dw2Lut[3][2] = { { 0x3, 0x2,}, { 0x3, 0x3,}, { 0x3, 0x3,},};
            data[2] |= dw2Lut[CodingTypeMinus1][currPicRef];

            static const uint32_t dw5Lut[3] = { 0xc0a000, 0xc1a000, 0xc0a000,};
            data[5] |= dw5Lut[CodingTypeMinus1];

            static const uint32_t dw7Lut[3][2][2] = { { { 0x64003, 0x64003,}, { 0x64003, 0x64003,},}, { { 0x64003, 0x64003,}, { 0x64003, 0xe4003,},}, { { 0x64003, 0x64003,}, { 0x64003, 0xe4003,},},};
            data[7] |= dw7Lut[CodingTypeMinus1][numL0Minus1Is0][lowDelay];

            static const uint32_t dw8Lut[3][8][2][2][2] = { { { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x54555555, 0x54555555,}, { 0x90908090, 0x90908090,},}, { { 0, 0,}, { 0x90908090, 0x90908090,},},}, { { { 0x54555555, 0x54555555,}, { 0xfffdccaa, 0xfffdccaa,},}, { { 0, 0,}, { 0xfffdccaa, 0xfffdccaa,},},}, { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,},}, { { 0, 0,}, { 0x55550000, 0x55550000,},},}, { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,},}, { { 0, 0,}, { 0x55550000, 0x55550000,},},}, { { { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,},}, { { 0, 0,}, { 0x55550000, 0x55550000,},},},}, { { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x90908090, 0x90908090,}, { 0x90908090, 0x90908090,},}, { { 0x90908090, 0x90908090,}, { 0x90908090, 0x90908090,},},}, { { { 0xfffdccaa, 0xfffdccaa,}, { 0xfffdccaa, 0xfffdccaa,},}, { { 0xfffdccaa, 0xfffdccaa,}, { 0xfffdccaa, 0xfffdccaa,},},}, { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},}, { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},}, { { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},},}, { { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x90908090, 0x90908090,}, { 0x90908090, 0x90908090,},}, { { 0x90908090, 0x90908090,}, { 0x90908090, 0x90908090,},},}, { { { 0xfffdfcaa, 0xfffdccaa,}, { 0xfffdfcaa, 0xfffdccaa,},}, { { 0xfffdfcaa, 0xfffdccaa,}, { 0xfffdfcaa, 0xfffdccaa,},},}, { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},}, { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},}, { { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},},},};
            data[8] |= dw8Lut[CodingTypeMinus1][tu][lowDelay][currPicRef][LowDelayMode];

            static const uint32_t dw9Lut[3][8][2][2][2] = { { { { { 0xc45555, 0xc45555,}, { 0xc45555, 0xc45555,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},},}, { { { 0xc45555, 0xc45555,}, { 0xc461e4, 0xc461e4,},}, { { 0xc40000, 0xc40000,}, { 0xc461e4, 0xc461e4,},},}, { { { 0x845555, 0x845555,}, { 0x84ffff, 0x84ffff,},}, { { 0x840000, 0x840000,}, { 0x84ffff, 0x84ffff,},},}, { { { 0x835555, 0x835555,}, { 0x835555, 0x835555,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x835555, 0x835555,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x835555, 0x835555,}, { 0x835555, 0x835555,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x425555, 0x425555,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},}, { { { 0x325555, 0x325555,}, { 0x320000, 0x320000,},}, { { 0x320000, 0x320000,}, { 0x320000, 0x320000,},},},}, { { { { 0xc45555, 0xc45555,}, { 0xc45555, 0xc45555,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},},}, { { { 0xc461e4, 0xc461e4,}, { 0xc461e4, 0xc461e4,},}, { { 0xc461e4, 0xc461e4,}, { 0xc461e4, 0xc461e4,},},}, { { { 0x84ffff, 0x84ffff,}, { 0x84ffff, 0x84ffff,},}, { { 0x84ffff, 0x84ffff,}, { 0x84ffff, 0x84ffff,},},}, { { { 0x835555, 0x835555,}, { 0x835555, 0x835555,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x835555, 0x835555,}, { 0x835555, 0x835555,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},}, { { { 0x320000, 0x320000,}, { 0x320000, 0x320000,},}, { { 0x320000, 0x320000,}, { 0x320000, 0x320000,},},},}, { { { { 0xc45555, 0xc45555,}, { 0xc45555, 0xc45555,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},},}, { { { 0xc461e4, 0xc461e4,}, { 0xc461e4, 0xc461e4,},}, { { 0xc461e4, 0xc461e4,}, { 0xc461e4, 0xc461e4,},},}, { { { 0x84fcff, 0x84ffff,}, { 0x84fcff, 0x84ffff,},}, { { 0x84fcff, 0x84ffff,}, { 0x84fcff, 0x84ffff,},},}, { { { 0x835555, 0x835555,}, { 0x835555, 0x835555,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x835555, 0x835555,}, { 0x835555, 0x835555,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},}, { { { 0x320000, 0x320000,}, { 0x320000, 0x320000,},}, { { 0x320000, 0x320000,}, { 0x320000, 0x320000,},},},},};
            data[9] |= dw9Lut[CodingTypeMinus1][tu][lowDelay][currPicRef][LowDelayMode];

            static const uint32_t dw11Lut = 0x80000000;
            data[11] |= dw11Lut;

            static const uint32_t dw12Lut[8] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xce4014a0, 0xce4014a0, 0xce4014a0, 0x89800dc0, 0x89800dc0,};
            data[12] |= dw12Lut[tu];

            static const uint32_t dw16Lut = 0xf000000;
            data[16] |= dw16Lut;

            static const uint32_t dw19Lut = 0x98000000;
            data[19] |= dw19Lut;

            static const uint32_t dw23Lut = 0xcccc0000;
            data[23] |= dw23Lut;

            static const uint32_t dw28Lut = 0x7d00fa0;
            data[28] |= dw28Lut;

            static const uint32_t dw29Lut = 0x2bc0bb8;
            data[29] |= dw29Lut;

            static const uint32_t dw30Lut = 0x32003e8;
            data[30] |= dw30Lut;

            static const uint32_t dw31Lut = 0x1f4012c;
            data[31] |= dw31Lut;

            static const uint32_t dw32Lut = 0x190;
            data[32] |= dw32Lut;

            static const uint32_t dw35Lut = 0xecc;
            data[35] |= dw35Lut;

            static const uint32_t dw37Lut[2] = { 0, 0x40,};
            data[37] |= dw37Lut[currPicRef];

            static const uint32_t dw39Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0x8000fc, 0x10001f8, 0x20003f0, 0x40007e0, 0x8000fc0,},};
            data[39] |= dw39Lut[paletteMode][depthMinus8];

            static const uint32_t dw40Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0xb10080, 0x1620100, 0x2c40200, 0x5880400, 0xb100800,},};
            data[40] |= dw40Lut[paletteMode][depthMinus8];

            static const uint32_t dw41Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0x300aa, 0x60154, 0xc02a8, 0x180550, 0x300aa0,},};
            data[41] |= dw41Lut[paletteMode][depthMinus8];

            static const uint32_t dw42Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0xd30069, 0x1a600d2, 0x34c01a4, 0x6980348, 0xd300690,},};
            data[42] |= dw42Lut[paletteMode][depthMinus8];

            static const uint32_t dw43Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0xe000e9, 0x1c001d2, 0x38003a4, 0x7000748, 0xe000e90,},};
            data[43] |= dw43Lut[paletteMode][depthMinus8];

            static const uint32_t dw44Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0x940003, 0x1280006, 0x250000c, 0x4a00018, 0x9400030,},};
            data[44] |= dw44Lut[paletteMode][depthMinus8];

            static const uint32_t dw45Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0x56004d, 0xac009a, 0x1580134, 0x2b00268, 0x56004d0,},};
            data[45] |= dw45Lut[paletteMode][depthMinus8];

            static const uint32_t dw46Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0x9500fd, 0x12a01fa, 0x25403f4, 0x4a807e8, 0x9500fd0,},};
            data[46] |= dw46Lut[paletteMode][depthMinus8];

            static const uint32_t dw47Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0x17002d, 0x2e005a, 0x5c00b4, 0xb80168, 0x17002d0,},};
            data[47] |= dw47Lut[paletteMode][depthMinus8];

            static const uint32_t dw48Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0xfd001f, 0x1fa003e, 0x3f4007c, 0x7e800f8, 0xfd001f0,},};
            data[48] |= dw48Lut[paletteMode][depthMinus8];

            static const uint32_t dw49Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0x2006c, 0x400d8, 0x801b0, 0x100360, 0x2006c0,},};
            data[49] |= dw49Lut[paletteMode][depthMinus8];

            static const uint32_t dw50Lut[2][5] = { { 0, 0, 0, 0, 0,}, { 0x800080, 0x1000100, 0x2000200, 0x4000400, 0x8000800,},};
            data[50] |= dw50Lut[paletteMode][depthMinus8];

            static const uint32_t dw51Lut[8][2][2] = { { { 0x33430aa0, 0x33430a90,}, { 0x33430a90, 0x33430a90,},}, { { 0x33430952, 0x33430952,}, { 0x33430952, 0x33430952,},}, { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x12227146, 0x12227152,}, { 0x12227152, 0x12227152,},}, { { 0x12127006, 0x12127012,}, { 0x12127012, 0x12127012,},},};
            data[51] |= dw51Lut[tu][currPicRef][paletteMode];

            static const uint32_t dw52Lut[8] = { 0x77fdb5b, 0x77f5b5b, 0x77f5bdb, 0x72d5959, 0x72d5959, 0x72d5959, 0x9295a5a, 0x9294a5a,};
            data[52] |= dw52Lut[tu];

            static const uint32_t dw53Lut[8] = { 0xffff, 0xffff, 0xffffffff, 0xff000000, 0xff000000, 0xff000000, 0xffff0000, 0x8000,};
            data[53] |= dw53Lut[tu];

            static const uint32_t dw54Lut[8][2][2][4] = { { { { 0x10080c0, 0x10080c0, 0x1008000, 0x1008000,}, { 0x1008000, 0x1008000, 0x1008000, 0x1008000,},}, { { 0x1008000, 0x1008000, 0x1008000, 0x1008000,}, { 0x1008000, 0x1008000, 0x1008000, 0x1008000,},},}, { { { 0x10080c0, 0x10080c0, 0x1008000, 0x1008000,}, { 0x1008000, 0x1008000, 0x1008000, 0x1008000,},}, { { 0x1008000, 0x1008000, 0x1008000, 0x1008000,}, { 0x1008000, 0x1008000, 0x1008000, 0x1008000,},},}, { { { 0x1008000, 0x1008000, 0x1008000, 0x1008000,}, { 0x1008000, 0x1008000, 0x1008000, 0x1008000,},}, { { 0x1008000, 0x1008000, 0x1008000, 0x1008000,}, { 0x1008000, 0x1008000, 0x1008000, 0x1008000,},},}, { { { 0xc5008000, 0xc5008000, 0xc5008000, 0xc5008000,}, { 0xc5008000, 0xc5008000, 0xc5008000, 0xc5008000,},}, { { 0xc5008000, 0xc5008000, 0xc5008000, 0xc5008000,}, { 0xc5008000, 0xc5008000, 0xc5008000, 0xc5008000,},},}, { { { 0xc5008000, 0xc5008000, 0xc5008000, 0xc5008000,}, { 0xc5008000, 0xc5008000, 0xc5008000, 0xc5008000,},}, { { 0xc5008000, 0xc5008000, 0xc5008000, 0xc5008000,}, { 0xc5008000, 0xc5008000, 0xc5008000, 0xc5008000,},},}, { { { 0x85008000, 0x85008000, 0x85008000, 0x85008000,}, { 0x85008000, 0x85008000, 0x85008000, 0x85008000,},}, { { 0x85008000, 0x85008000, 0x85008000, 0x85008000,}, { 0x85008000, 0x85008000, 0x85008000, 0x85008000,},},}, { { { 0xbd00800c, 0xbd00800c, 0xbd00800c, 0xbd00800c,}, { 0xbd00800c, 0xbd00800c, 0xbd00800c, 0xbd00800c,},}, { { 0xbd00800c, 0xbd00800c, 0xbd00800c, 0xbd00800c,}, { 0xbd00800c, 0xbd00800c, 0xbd00800c, 0xbd00800c,},},}, { { { 0xbd00800c, 0xbd00800c, 0xbd00800c, 0xbd00800c,}, { 0xbd00800c, 0xbd00800c, 0xbd00800c, 0xbd00800c,},}, { { 0xbd00800c, 0xbd00800c, 0xbd00800c, 0xbd00800c,}, { 0xbd00800c, 0xbd00800c, 0xbd00800c, 0xbd00800c,},},},};
            data[54] |= dw54Lut[tu][currPicRef][paletteMode][chroma_format_idc];

            static const uint32_t dw55Lut[2] = { 0, 0xcdef0123,};
            data[55] |= dw55Lut[rdoq];

            static const uint32_t dw56Lut[3][2][2][5][4] = { { { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x300, 0, 0,}, { 0, 0, 0, 0x300,}, { 0, 0, 0, 0,},}, { { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0x30b, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0x30b,}, { 0xb, 0xb, 0xb, 0xb,},},}, { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0, 0, 0x300,},}, { { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0x30b,},},},}, { { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0x300, 0x300, 0x300, 0x300,}, { 0, 0, 0, 0,},}, { { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0x30b, 0x30b, 0x30b, 0x30b,}, { 0xb, 0xb, 0xb, 0xb,},},}, { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0x300, 0x300, 0x300, 0x300,},}, { { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0x30b, 0x30b, 0x30b, 0x30b,},},},}, { { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x300, 0, 0,}, { 0, 0, 0, 0x300,}, { 0, 0, 0, 0,},}, { { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0x30b, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0x30b,}, { 0xb, 0xb, 0xb, 0xb,},},}, { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0, 0, 0x300,},}, { { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0xb,}, { 0xb, 0xb, 0xb, 0x30b,},},},},};
            data[56] |= dw56Lut[CodingTypeMinus1][currPicRef][rdoq][numRef0][numRef1];

            static const uint32_t dw57Lut[2] = { 0, 0x508c23,};
            data[57] |= dw57Lut[rdoq];

            static const uint32_t dw58Lut[2] = { 0, 0x466419,};
            data[58] |= dw58Lut[rdoq];

            static const uint32_t dw59Lut[2] = { 0, 0x7d6c5c4b,};
            data[59] |= dw59Lut[rdoq];

            static const uint32_t dw60Lut[2] = { 0, 0xbfaf9e8e,};
            data[60] |= dw60Lut[rdoq];

            return MOS_STATUS_SUCCESS;
        });

    return MOS_STATUS_SUCCESS;
});

setting->vdencCmd2Settings.emplace_back(
    VDENC_CMD2_LAMBDA()
    {
        par.extSettings.emplace_back(
            [this](uint32_t *data) {

                if (!m_hevcVdencRoundingPrecisionEnabled)
                {
                    return MOS_STATUS_SUCCESS;
                }

                uint8_t tmp0 = 0;
                uint8_t tmp1 = 0;

                if (m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingIntra)
                {
                    tmp0 = m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetIntra;
                }
                else
                {
                    if (m_hevcPicParams->CodingType == I_TYPE)
                    {
                        tmp0 = 10;
                    }
                    else if (m_hevcSeqParams->HierarchicalFlag && m_hevcPicParams->HierarchLevelPlus1 > 0)
                    {
                        //Hierachical GOP
                        if (m_hevcPicParams->HierarchLevelPlus1 == 1)
                        {
                            tmp0 = 10;
                        }
                        else if (m_hevcPicParams->HierarchLevelPlus1 == 2)
                        {
                            tmp0 = 9;
                        }
                        else
                        {
                            tmp0 = 8;
                        }
                    }
                    else
                    {
                        tmp0 = 10;
                    }
                }

                if (m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingInter)
                {
                    tmp1 = m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetInter;
                }
                else
                {
                    if (m_hevcPicParams->CodingType == I_TYPE)
                    {
                        tmp1 = 4;
                    }
                    else if (m_hevcSeqParams->HierarchicalFlag && m_hevcPicParams->HierarchLevelPlus1 > 0)
                    {
                        //Hierachical GOP
                        if (m_hevcPicParams->HierarchLevelPlus1 == 1)
                        {
                            tmp1 = 4;
                        }
                        else if (m_hevcPicParams->HierarchLevelPlus1 == 2)
                        {
                            tmp1 = 3;
                        }
                        else
                        {
                            tmp1 = 2;
                        }
                    }
                    else
                    {
                        tmp1 = 4;
                    }
                }
                tmp0 &= 0xf;
                tmp1 &= 0xf;

                data[32] |= (tmp1 << 16);
                data[32] |= (tmp1 << 20);
                data[32] |= (tmp0 << 24);
                data[32] |= (tmp0 << 28);

                data[33] |= tmp1;
                data[33] |= (tmp1 << 4);
                data[33] |= (tmp1 << 8);
                data[33] |= (tmp1 << 12);
                data[33] |= (tmp0 << 16);
                data[33] |= (tmp0 << 20);
                data[33] |= (tmp1 << 24);
                data[33] |= (tmp1 << 28);

                data[34] |= tmp1;
                data[34] |= (tmp1 << 4);
                data[34] |= (tmp0 << 8);
                data[34] |= (tmp0 << 12);
                data[34] |= (tmp1 << 16);
                data[34] |= (tmp1 << 20);

                return MOS_STATUS_SUCCESS;
            });

        return MOS_STATUS_SUCCESS;
});

#endif  // VDENC_CMD2_SETTINGS_OPEN