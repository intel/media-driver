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
//! \file     encode_av1_vdenc_const_settings_xe3_lpm_base_open.h
//! \brief    Defines the opensource av1 cmd2 const settings
//! \details  The encode feature manager is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#ifdef VDENC_CMD2_SETTINGS_OPEN

setting->vdencCmd2Settings.emplace_back(
    VDENC_CMD2_LAMBDA()
    {

        par.extSettings.emplace_back(
            [this, isLowDelay, &par](uint32_t *data) {
                uint32_t TargetUsage        = m_av1SeqParams->TargetUsage;
                uint32_t lowDelay           = isLowDelay;
                uint32_t frameType          = m_av1PicParams->PicFlags.fields.frame_type;
                uint32_t l0RefNum           = par.numRefL0;
                uint32_t l1RefNum           = par.numRefL1;

                static const uint32_t dw2Lut = 0x3;
                data[2] |= dw2Lut;

                static const uint32_t dw5Lut = 0xc0a080;
                data[5] |= dw5Lut;

                static const uint32_t dw6Lut = 0xe0080200;
                data[6] |= dw6Lut;

                static const uint32_t dw7Lut[2][4] = { { 0x64003, 0x64003, 0x64003, 0x64003,}, { 0xe4003, 0xe4003, 0x64003, 0x64003,},};
                data[7] |= dw7Lut[lowDelay][l0RefNum];

                static const uint32_t dw8Lut[8] = { 0, 0, 0, 0, 0, 0, 0x55550000, 0x55550000,};
                data[8] |= dw8Lut[TargetUsage];

                static const uint32_t dw9Lut[8] = { 0x840000, 0x840000, 0x840000, 0x640000, 0x640000, 0x640000, 0x420000, 0x320000,};
                data[9] |= dw9Lut[TargetUsage];

                static const uint32_t dw11Lut[2][4][4] = { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x80000000, 0, 0,}, { 0x80000000, 0, 0, 0,},}, { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x80000000, 0, 0,}, { 0x80000000, 0, 0, 0x80000000,},},};
                data[11] |= dw11Lut[lowDelay][l0RefNum][l1RefNum];

                static const uint32_t dw12Lut = 0xffffffff;
                data[12] |= dw12Lut;

                static const uint32_t dw14Lut = 0x1f40000;
                data[14] |= dw14Lut;

                static const uint32_t dw15Lut = 0x13881388;
                data[15] |= dw15Lut;

                static const uint32_t dw16Lut = 0xf000000;
                data[16] |= dw16Lut;

                static const uint32_t dw17Lut = 0x3e8;
                data[17] |= dw17Lut;

                static const uint32_t dw18Lut = 0x80000;
                data[18] |= dw18Lut;

                static const uint32_t dw19Lut = 0x98000040;
                data[19] |= dw19Lut;

                static const uint32_t dw20Lut = 0xffff;
                data[20] |= dw20Lut;

                static const uint32_t dw27Lut = 0xffff0000;
                data[27] |= dw27Lut;

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

                static const uint32_t dw35Lut = 0x800;
                data[35] |= dw35Lut;

                static const uint32_t dw37Lut = 0x40;
                data[37] |= dw37Lut;

                static const uint32_t dw39Lut = 0x8000fc;
                data[39] |= dw39Lut;

                static const uint32_t dw40Lut = 0xb10080;
                data[40] |= dw40Lut;

                static const uint32_t dw41Lut = 0x300aa;
                data[41] |= dw41Lut;

                static const uint32_t dw42Lut = 0xd30069;
                data[42] |= dw42Lut;

                static const uint32_t dw43Lut = 0xe000e9;
                data[43] |= dw43Lut;

                static const uint32_t dw44Lut = 0x940003;
                data[44] |= dw44Lut;

                static const uint32_t dw45Lut = 0x56004d;
                data[45] |= dw45Lut;

                static const uint32_t dw46Lut = 0x9500fd;
                data[46] |= dw46Lut;

                static const uint32_t dw47Lut = 0x17002d;
                data[47] |= dw47Lut;

                static const uint32_t dw48Lut = 0xfd001f;
                data[48] |= dw48Lut;

                static const uint32_t dw49Lut = 0x2006c;
                data[49] |= dw49Lut;

                static const uint32_t dw50Lut = 0x800080;
                data[50] |= dw50Lut;

                static const uint32_t dw51Lut[8][4] = { { 0x44558800, 0x44558800, 0x44558800, 0x44558800,}, { 0x44558800, 0x44558800, 0x44558800, 0x44558800,}, { 0x44558800, 0x33358400, 0x44558800, 0x33358400,}, { 0x2222b408, 0x2222b408, 0x2222b408, 0x2222b408,}, { 0x2222b408, 0x2222b008, 0x2222b408, 0x2222b008,}, { 0x2222b408, 0x2222b408, 0x2222b408, 0x2222b408,}, { 0x222700e, 0x222700e, 0x222700e, 0x222700e,}, { 0x122700e, 0x122400e, 0x122700e, 0x122400e,},};
                data[51] |= dw51Lut[TargetUsage][frameType];

                static const uint32_t dw52Lut[8][4] = { { 0x5255949, 0x5255949, 0x5255949, 0x5255949,}, { 0x45255949, 0x45255949, 0x45255949, 0x45255949,}, { 0x45255949, 0x45255949, 0x45255949, 0x45255949,}, { 0x5255949, 0x5255949, 0x5255949, 0x5255949,}, { 0x5255949, 0x1254959, 0x5255949, 0x1254959,}, { 0x5255949, 0x5255949, 0x5255949, 0x5255949,}, { 0x9255959, 0x9255959, 0x9255959, 0x9255959,}, { 0x9254959, 0x125595a, 0x9254959, 0x125595a,},};
                data[52] |= dw52Lut[TargetUsage][frameType];

                static const uint32_t dw53Lut[8] = { 0xffff, 0xffff, 0xffff, 0xff000000, 0xff000000, 0xff000000, 0, 0,};
                data[53] |= dw53Lut[TargetUsage];

                static const uint32_t dw54Lut[8][4] = { { 0x20000480, 0x20000480, 0x20000480, 0x20000480,}, { 0x20000480, 0x20000480, 0x20000480, 0x20000480,}, { 0x20000480, 0x20000400, 0x20000480, 0x20000400,}, { 0x84000480, 0x84000480, 0x84000480, 0x84000480,}, { 0x84000480, 0x84000400, 0x84000480, 0x84000400,}, { 0x84000480, 0x84000480, 0x84000480, 0x84000480,}, { 0x9c00040c, 0x9c00040c, 0x9c00040c, 0x9c00040c,}, { 0xbc00040c, 0xbc00040c, 0xbc00040c, 0xbc00040c,},};
                data[54] |= dw54Lut[TargetUsage][frameType];

                static const uint32_t dw56Lut[2][4][4] = { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x300, 0, 0,}, { 0x300, 0, 0, 0,},}, { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x300, 0, 0,}, { 0x300, 0, 0, 0x300,},},};
                data[56] |= dw56Lut[lowDelay][l0RefNum][l1RefNum];

                static const uint32_t dw62Lut[8][4] = { { 0x77707777, 0x77707777, 0x77707777, 0x77707777,}, { 0x77707777, 0x77707777, 0x77707777, 0x77707777,}, { 0x77707777, 0x77304442, 0x77707777, 0x77304442,}, { 0x24403433, 0x24403433, 0x24403433, 0x24403433,}, { 0x24403433, 0x11103410, 0x24403433, 0x11103410,}, { 0x24403433, 0x24403433, 0x24403433, 0x24403433,}, { 0x11101410, 0x11101210, 0x11101410, 0x11101210,}, { 0x11101210, 0x11101111, 0x11101210, 0x11101111,},};
                data[62] |= dw62Lut[TargetUsage][frameType];

                static const uint32_t dw63Lut[8][4] = { { 0x54030303, 0x54030303, 0x54030303, 0x54030303,}, { 0x54030303, 0x54030303, 0x54030303, 0x54030303,}, { 0x54030303, 0x54030303, 0x54030303, 0x54030303,}, { 0x54010101, 0x54010101, 0x54010101, 0x54010101,}, { 0x54010101, 0x50010101, 0x54010101, 0x50010101,}, { 0x54010101, 0x54010101, 0x54010101, 0x54010101,}, { 0x10101, 0x10101, 0x10101, 0x10101,}, { 0x10101, 0x10101, 0x10101, 0x10101,},};
                data[63] |= dw63Lut[TargetUsage][frameType];

                static const uint32_t dw64Lut[8][4] = { { 0x30002, 0x2, 0x30002, 0x2,}, { 0x30002, 0x2, 0x30002, 0x2,}, { 0x30002, 0x10002, 0x30002, 0x10002,}, { 0x2, 0x2, 0x2, 0x2,}, { 0x2, 0x2, 0x2, 0x2,}, { 0x2, 0x2, 0x2, 0x2,}, { 0x1, 0x1, 0x1, 0x1,}, { 0x1, 0, 0x1, 0,},};

                data[64] |= dw64Lut[TargetUsage][frameType];

                return MOS_STATUS_SUCCESS;
            }
        );
        return MOS_STATUS_SUCCESS;
    }
);

#endif  // VDENC_CMD2_SETTINGS_OPEN