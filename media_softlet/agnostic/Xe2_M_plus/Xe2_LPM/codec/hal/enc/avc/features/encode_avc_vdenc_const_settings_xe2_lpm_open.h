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
//! \file     encode_avc_vdenc_const_settings_xe2_hpm_open.h
//! \brief    Defines the opensource avc const settings
//! \details  The encode feature manager is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#ifdef VDENC_AVCIMGSTATE_SETTINGS_OPEN

setting->vdencAvcImgStateSettings.emplace_back(
    VDENC_AVC_IMG_STATE_LAMBDA()
    {
        par.extSettings.emplace_back(
            [this, &par](uint32_t *data) {
                auto waTable = m_osItf->pfnGetWaTable(m_osItf);
                ENCODE_CHK_NULL_RETURN(waTable);

                uint32_t CodingTypeMinus1          = m_avcPicParams->CodingType - 1;
                uint32_t tu                        = m_avcSeqParams->TargetUsage;
                uint32_t EnableRollingIntraRefresh = m_avcPicParams->EnableRollingIntraRefresh;
                uint32_t Level                     = m_avcSeqParams->Level;
                uint32_t RefPicFlag                = m_avcPicParams->RefPicFlag;
                uint32_t Wa_18011246551            = MEDIA_IS_WA(waTable, Wa_18011246551);
                uint32_t LowDelayMode              = m_avcSeqParams->LowDelayMode;
                uint32_t EnableSliceLevelRateCtrl  = m_avcSeqParams->EnableSliceLevelRateCtrl;

                static const uint32_t dw1Lut = 0x301;
                data[1] |= dw1Lut;

                static const uint32_t dw2Lut[3][8][2][2] = { { { { 0x70028000, 0x70028000,}, { 0x70028000, 0x70028000,},}, { { 0xc0029400, 0xc0029400,}, { 0xc0029400, 0xc0029400,},}, { { 0x70029400, 0x70029400,}, { 0x70029400, 0x70029400,},}, { { 0x40028240, 0x40028240,}, { 0x40028240, 0x40028240,},}, { { 0x40028240, 0x40028240,}, { 0x40028240, 0x40028240,},}, { { 0x40028240, 0x40028240,}, { 0x40028240, 0x40028240,},}, { { 0x400281e0, 0x40028160,}, { 0x400281e0, 0x40028160,},}, { { 0x400281e0, 0x40028160,}, { 0x400281e0, 0x40028160,},},}, { { { 0x70028000, 0x70028000,}, { 0x70028000, 0x70028000,},}, { { 0xc0029407, 0xc0029407,}, { 0xc0029407, 0xc0029407,},}, { { 0x70029407, 0x70029407,}, { 0x70029407, 0x70029407,},}, { { 0x40028247, 0x40028247,}, { 0x40028247, 0x40028247,},}, { { 0x40028247, 0x40028247,}, { 0x40028247, 0x40028247,},}, { { 0x40028247, 0x40028247,}, { 0x40028247, 0x40028247,},}, { { 0x400281e4, 0x40028164,}, { 0x400281e4, 0x40028164,},}, { { 0x400281e4, 0x40028164,}, { 0x400281e4, 0x40028164,},},}, { { { 0x70028000, 0x70028000,}, { 0x70028000, 0x70028000,},}, { { 0xc0029407, 0xc0029407,}, { 0xc0029407, 0xc0029407,},}, { { 0x70029407, 0x70029407,}, { 0x72029207, 0x72029207,},}, { { 0x40028244, 0x40028244,}, { 0x40028244, 0x40028244,},}, { { 0x40028243, 0x40028243,}, { 0x40028243, 0x40028243,},}, { { 0x40028243, 0x40028243,}, { 0x40028243, 0x40028243,},}, { { 0x400281e2, 0x40028162,}, { 0x400281e2, 0x40028162,},}, { { 0x400281e2, 0x40028162,}, { 0x400281e2, 0x40028162,},},},};
                data[2] |= dw2Lut[CodingTypeMinus1][tu][RefPicFlag][EnableSliceLevelRateCtrl];
                
                static const uint32_t dw4Lut[3][8][4][2] = { { { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0x800, 0x800,}, { 0x800, 0x800,}, { 0x800, 0x800,}, { 0x800, 0x800,},}, { { 0x800, 0x800,}, { 0x800, 0x800,}, { 0x800, 0x800,}, { 0x800, 0x800,},},}, { { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0xbd0088,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0xbd0088,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0xbd0088,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0xbd0088,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0xbd0088,}, { 0, 0,}, { 0, 0,},}, { { 0x800, 0x800,}, { 0x800, 0xbd0888,}, { 0x800, 0x800,}, { 0x800, 0x800,},}, { { 0x800, 0x800,}, { 0x800, 0xbd0888,}, { 0x800, 0x800,}, { 0x800, 0x800,},},}, { { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0xbd0088,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0xbd0088,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0xbd0088,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0xbd0088,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0xbd0088,}, { 0, 0,}, { 0, 0,},}, { { 0x800, 0x800,}, { 0x800, 0xbd0888,}, { 0x800, 0x800,}, { 0x800, 0x800,},}, { { 0x800, 0x800,}, { 0x800, 0xbd0888,}, { 0x800, 0x800,}, { 0x800, 0x800,},},},};
                data[4] |= dw4Lut[CodingTypeMinus1][tu][EnableRollingIntraRefresh][Wa_18011246551];

                static const uint32_t dw5Lut[3][8] = { { 0, 0, 0, 0, 0, 0, 0, 0,}, { 0, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000,}, { 0, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000, 0x80000000,},};
                data[5] |= dw5Lut[CodingTypeMinus1][tu];

                static const uint32_t dw7Lut = 0xffff0000;
                data[7] |= dw7Lut;

                static const uint32_t dw8Lut[53] = { 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x1002000, 0x1002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x4002000, 0x4002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x4002000, 0x8002000, 0x8002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x8002000, 0x8002000, 0x8002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x2002000, 0x8002000, 0x8002000, 0x8002000,};
                data[8] |= dw8Lut[Level];

                static const uint32_t dw10Lut[3] = { 0x3e80000, 0x3200000, 0x2580000,};
                data[10] |= dw10Lut[CodingTypeMinus1];

                static const uint32_t dw11Lut[3] = { 0xbb807d0, 0x9600640, 0x70804b0,};
                data[11] |= dw11Lut[CodingTypeMinus1];

                static const uint32_t dw12Lut[3][8] = { { 0xf000000, 0xf000000, 0xf000000, 0xf000000, 0xf000000, 0xf000000, 0xf000000, 0xf000000,}, { 0xf000000, 0xf000000, 0xf000000, 0xf000000, 0xf000000, 0xf000000, 0xfff0000, 0xfff0000,}, { 0xf000000, 0xf000000, 0xf000000, 0xf000000, 0xf000000, 0xf000000, 0xfff0000, 0xfff0000,},};
                data[12] |= dw12Lut[CodingTypeMinus1][tu];

                static const uint32_t dw13Lut[3][8][2] = { { { 0x7d00000, 0x7d00000,}, { 0x7d00000, 0x7d00000,}, { 0x7d00000, 0x7d00000,}, { 0x7d00000, 0x7d00000,}, { 0x7d00000, 0x7d00000,}, { 0x7d00000, 0x7d00000,}, { 0x7d00000, 0x7d00000,}, { 0x7d00000, 0x7d00000,},}, { { 0x6400000, 0x6400000,}, { 0x6400000, 0x64000c0,}, { 0x6400000, 0x6400000,}, { 0x6400000, 0x6400000,}, { 0x6400000, 0x6400000,}, { 0x6400000, 0x6400000,}, { 0x6400000, 0x6400000,}, { 0x6400000, 0x6400000,},}, { { 0x4b00000, 0x4b00000,}, { 0x4b000c0, 0x4b000c0,}, { 0x4b00000, 0x4b00000,}, { 0x4b00000, 0x4b00000,}, { 0x4b00000, 0x4b00000,}, { 0x4b00000, 0x4b00000,}, { 0x4b00000, 0x4b00000,}, { 0x4b00000, 0x4b00000,},},};
                data[13] |= dw13Lut[CodingTypeMinus1][tu][LowDelayMode];

                static const uint32_t dw14Lut[3][8] = { { 0xff200000, 0xff200000, 0xff200000, 0xff200000, 0xff200000, 0xff200000, 0xff200000, 0xff200000,}, { 0xff200000, 0xff200000, 0xff200000, 0xff200000, 0xff200000, 0xff200000, 0xffe00000, 0xffe00000,}, { 0xff200000, 0xff200000, 0xff200000, 0xff200000, 0xff200000, 0xff200000, 0xffe00000, 0xffe00000,},};
                data[14] |= dw14Lut[CodingTypeMinus1][tu];

                static const uint32_t dw15Lut = 0xbb80002;
                data[15] |= dw15Lut;

                static const uint32_t dw16Lut = 0xe100004;
                data[16] |= dw16Lut;

                static const uint32_t dw17Lut = 0x13880006;
                data[17] |= dw17Lut;

                static const uint32_t dw18Lut = 0x1f40000a;
                data[18] |= dw18Lut;

                static const uint32_t dw19Lut = 0x23280012;
                data[19] |= dw19Lut;

                static const uint32_t dw22Lut = 0x33000000;
                data[22] |= dw22Lut;

                static const uint32_t dw25Lut[8] = { 0x6000c0, 0x6000c0, 0, 0x6000c0, 0x6000c0, 0x6000c0, 0x6000c0, 0x6000c0,};
                data[25] |= dw25Lut[tu];

                return MOS_STATUS_SUCCESS;

            });

        return MOS_STATUS_SUCCESS;
    });

#endif  // VDENC_AVCIMGSTATE_SETTINGS_OPEN