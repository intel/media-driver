/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_vp9_vdenc_const_settings_xe_lpm_plus.cpp
//! \brief    Defines the common interface for vp9 vdenc const settings
//!

#include "encode_vp9_vdenc_const_settings_xe_lpm_plus.h"
#include "encode_utils.h"

namespace encode
{

const uint32_t EncodeVp9VdencConstSettingsXe_Lpm_Plus::m_numMergeCandidateCu64x64Xe_Lpm_Plus[] = {0, 3, 3, 3, 3, 3, 2, 2};
const uint32_t EncodeVp9VdencConstSettingsXe_Lpm_Plus::m_numMergeCandidateCu32x32Xe_Lpm_Plus[] = {0, 3, 3, 3, 3, 3, 2, 2};
const uint32_t EncodeVp9VdencConstSettingsXe_Lpm_Plus::m_numMergeCandidateCu16x16Xe_Lpm_Plus[] = {0, 3, 3, 2, 2, 2, 2, 2};
const uint32_t EncodeVp9VdencConstSettingsXe_Lpm_Plus::m_numMergeCandidateCu8x8Xe_Lpm_Plus[]   = {0, 3, 3, 2, 2, 2, 1, 1};
const uint8_t  EncodeVp9VdencConstSettingsXe_Lpm_Plus::m_numImePredictorsXe_Lpm_Plus[]         = {0, 12, 12, 8, 8, 8, 4, 4};


MOS_STATUS EncodeVp9VdencConstSettingsXe_Lpm_Plus::SetTUSettings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto setting = static_cast<Vp9VdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->NumMergeCandidateCu8x8   = (uint32_t *)m_numMergeCandidateCu8x8Xe_Lpm_Plus;
    setting->NumMergeCandidateCu16x16 = (uint32_t *)m_numMergeCandidateCu16x16Xe_Lpm_Plus;
    setting->NumMergeCandidateCu32x32 = (uint32_t *)m_numMergeCandidateCu32x32Xe_Lpm_Plus;
    setting->NumMergeCandidateCu64x64 = (uint32_t *)m_numMergeCandidateCu64x64Xe_Lpm_Plus;
    setting->NumImePredictors         = (uint8_t *)m_numImePredictorsXe_Lpm_Plus;

    return eStatus;
}

MOS_STATUS EncodeVp9VdencConstSettingsXe_Lpm_Plus::SetVdencCmd1Settings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Vp9VdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencCmd1Settings = {
        VDENC_CMD1_LAMBDA(){
            uint8_t qp = m_vp9PicParams->LumaACQIndex;
            uint8_t frameType = m_vp9PicParams->PicFlags.fields.frame_type;

            double DLOCAL0 = (frameType == CODEC_VP9_KEY_FRAME) ? 0.31 : 0.33;
            double DLOCAL1 = DLOCAL0 * CODECHAL_VP9_QUANT_AC[qp] / 8;

            // DW22
            par.vdencCmd1Par0 = (uint16_t)(DLOCAL1 * DLOCAL1 * 4 + 0.5);
            par.vdencCmd1Par1 = (uint16_t)(DLOCAL1 * 4 + 0.5);

            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            uint8_t frameType = m_vp9PicParams->PicFlags.fields.frame_type;
            if (CODEC_VP9_KEY_FRAME == frameType)
            {
                // DW9
                par.vdencCmd1Par5 = 0;
                par.vdencCmd1Par6 = 0;
                par.vdencCmd1Par7 = 0;

                // DW10
                par.vdencCmd1Par13[0] = 0;
                par.vdencCmd1Par9[0]  = 0;
                par.vdencCmd1Par12[0] = 0;
                par.vdencCmd1Par8[0]  = 0;

                // DW11
                par.vdencCmd1Par15[0] = 0;
                par.vdencCmd1Par11[0] = 0;
                par.vdencCmd1Par14[0] = 0;
                par.vdencCmd1Par10[0] = 0;

                // DW12
                par.vdencCmd1Par19 = 0;
                par.vdencCmd1Par18 = 0;
                par.vdencCmd1Par17 = 0;
                par.vdencCmd1Par16 = 0;

                // DW13
                par.vdencCmd1Par20 = 0;
                par.vdencCmd1Par21 = 0;
                par.vdencCmd1Par22 = 0;
                par.vdencCmd1Par23 = 0x2a;

                // DW16
                par.vdencCmd1Par34 = 0x15;
                par.vdencCmd1Par35 = 0;

                // DW17
                par.vdencCmd1Par39 = 0x10;
                par.vdencCmd1Par38 = 0x10;
                par.vdencCmd1Par37 = 0x2f;
                par.vdencCmd1Par36 = 0x15;

                // DW18
                par.vdencCmd1Par43 = 0x14;
                par.vdencCmd1Par42 = 0x3a;
                par.vdencCmd1Par41 = 0x1e;
                par.vdencCmd1Par40 = 0x1e;

                // DW31
                par.vdencCmd1Par89 = 0;
                par.vdencCmd1Par88 = 0;
                par.vdencCmd1Par87 = 0;

            }
            else
            {
                // DW9
                par.vdencCmd1Par5 = 0x6;
                par.vdencCmd1Par6 = 0x3;
                par.vdencCmd1Par7 = 0x0a;

                // DW10
                par.vdencCmd1Par13[0] = 0x1a;
                par.vdencCmd1Par9[0]  = 0x06;
                par.vdencCmd1Par12[0] = 0x17;
                par.vdencCmd1Par8[0]  = 0x05;

                // DW11
                par.vdencCmd1Par15[0] = 0;
                par.vdencCmd1Par11[0] = 0;
                par.vdencCmd1Par14[0] = 0x15;
                par.vdencCmd1Par10[0] = 0x05;

                // DW12
                par.vdencCmd1Par19 = 0x12;
                par.vdencCmd1Par18 = 0x5c;
                par.vdencCmd1Par17 = 0x13;
                par.vdencCmd1Par16 = 0x5c;

                // DW13
                par.vdencCmd1Par20 = 0x0f;
                par.vdencCmd1Par21 = 0x04;
                par.vdencCmd1Par22 = 0x04;
                par.vdencCmd1Par23 = 0x36;

                // DW16
                par.vdencCmd1Par34 = 0x15;
                par.vdencCmd1Par35 = 0x04;

                // DW17
                par.vdencCmd1Par39 = 0x1b;
                par.vdencCmd1Par38 = 0x18;
                par.vdencCmd1Par37 = 0x17;
                par.vdencCmd1Par36 = 0x15;

                // DW18
                par.vdencCmd1Par43 = 0x19;
                par.vdencCmd1Par42 = 0x18;
                par.vdencCmd1Par41 = 0x44;
                par.vdencCmd1Par40 = 0x29;

                // DW31
                par.vdencCmd1Par89 = 0x14;
                par.vdencCmd1Par88 = 0x14;
                par.vdencCmd1Par87 = 0x14;
            }
            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
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
            static const std::array<uint8_t, 12> data3 = {
                4, 14, 24, 34, 44, 54, 64, 74, 84, 94, 104, 114};

            static const std::array<uint8_t, 12> data4 = {
                3, 9, 14, 19, 24, 29, 34, 39, 44, 49, 54, 60};

            for (size_t i = 0; i < data3.size(); i++)
            {
                par.vdencCmd1Par3[i] = data3[i];
                par.vdencCmd1Par4[i] = data4[i];
            }
            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            // DW14
            par.vdencCmd1Par24 = 0;
            par.vdencCmd1Par25 = 0;
            par.vdencCmd1Par26 = 0;
            par.vdencCmd1Par27 = 0;

            // DW15
            par.vdencCmd1Par28 = 0;
            par.vdencCmd1Par29 = 0;
            par.vdencCmd1Par30 = 0;
            par.vdencCmd1Par31 = 0;

            // DW16
            par.vdencCmd1Par32 = 0;
            par.vdencCmd1Par33 = 0;

            return MOS_STATUS_SUCCESS;
        },
        VDENC_CMD1_LAMBDA()
        {
            // DW19
            par.vdencCmd1Par44 = 0;
            par.vdencCmd1Par45 = 0;
            par.vdencCmd1Par46 = 0;

            // DW20
            par.vdencCmd1Par47 = 5;
            par.vdencCmd1Par48 = 5;
            par.vdencCmd1Par49 = 5;
            par.vdencCmd1Par50 = 5;

            // DW21
            par.vdencCmd1Par51 = 12;
            par.vdencCmd1Par52 = 12;
            par.vdencCmd1Par53 = 12;
            par.vdencCmd1Par54 = 12;

            // DW23
            par.vdencCmd1Par55 = 0x12;
            par.vdencCmd1Par56 = 0x12;
            par.vdencCmd1Par57 = 0x12;
            par.vdencCmd1Par58 = 0x12;

            // DW24
            par.vdencCmd1Par59 = 0x10;
            par.vdencCmd1Par60 = 0x10;
            par.vdencCmd1Par61 = 0x10;
            par.vdencCmd1Par62 = 0x10;

            // DW25
            par.vdencCmd1Par63 = 0x10;
            par.vdencCmd1Par64 = 0x10;
            par.vdencCmd1Par65 = 0x10;
            par.vdencCmd1Par66 = 0x10;

            // DW26
            par.vdencCmd1Par67 = 0x16;
            par.vdencCmd1Par68 = 0x16;
            par.vdencCmd1Par69 = 0x16;
            par.vdencCmd1Par70 = 0x16;

            // DW27
            par.vdencCmd1Par71 = 0x10;
            par.vdencCmd1Par72 = 0x10;
            par.vdencCmd1Par73 = 0x10;
            par.vdencCmd1Par74 = 0x10;

            // DW28
            par.vdencCmd1Par75 = 0x10;
            par.vdencCmd1Par76 = 0x10;
            par.vdencCmd1Par77 = 0x10;
            par.vdencCmd1Par78 = 0x10;

            // DW29
            par.vdencCmd1Par79 = 0x10;
            par.vdencCmd1Par80 = 0x10;
            par.vdencCmd1Par81 = 0x10;
            par.vdencCmd1Par82 = 0x10;

            // DW30
            par.vdencCmd1Par83 = 0x1a;
            par.vdencCmd1Par84 = 0x1a;
            par.vdencCmd1Par85 = 0x1a;
            par.vdencCmd1Par86 = 0x1a;

            return MOS_STATUS_SUCCESS;
        }
    };

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeVp9VdencConstSettingsXe_Lpm_Plus::SetVdencCmd2Settings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Vp9VdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

#if _MEDIA_RESERVED
#define VDENC_CMD2_SETTINGS_EXT
#include "encode_vp9_vdenc_const_settings_xe_lpm_plus_base_ext.h"
#undef VDENC_CMD2_SETTINGS_EXT
#else
    setting->vdencCmd2Settings.emplace_back(
        VDENC_CMD2_LAMBDA() {
            par.extSettings.emplace_back(
                [this, &par](uint32_t *data) {
                    uint32_t TargetUsagediv3 = m_vp9SeqParams->TargetUsage / 3;
                    uint32_t l0RefNum        = par.numRefL0;
                    uint32_t frameType       = m_vp9PicParams->PicFlags.fields.frame_type;
                    uint32_t temporalMvp     = par.temporalMvp;

                    static const uint32_t dw2Lut = 0x3;
                    data[2] |= dw2Lut;

                    static const uint32_t dw5Lut[2] = { 0xc0ac00, 0xf0c0ac00,};
                    data[5] |= dw5Lut[frameType];

                    static const uint32_t dw6Lut = 0x20080200;
                    data[6] |= dw6Lut;

                    static const uint32_t dw7Lut[4][2] = { { 0x84003, 0x84003,}, { 0x84003, 0x84003,}, { 0x84003, 0x4003,}, { 0x84003, 0x4003,},};
                    data[7] |= dw7Lut[l0RefNum][temporalMvp];

                    static const uint32_t dw8Lut[3] = { 0xfffdccaa, 0xfffdccaa, 0x55550000,};
                    data[8] |= dw8Lut[TargetUsagediv3];

                    static const uint32_t dw9Lut[3] = { 0x84ffff, 0x63ffff, 0x420000,};
                    data[9] |= dw9Lut[TargetUsagediv3];

                    static const uint32_t dw14Lut = 0x1f40000;
                    data[14] |= dw14Lut;

                    static const uint32_t dw15Lut = 0x138807d0;
                    data[15] |= dw15Lut;

                    static const uint32_t dw16Lut = 0xf00ff00;
                    data[16] |= dw16Lut;

                    static const uint32_t dw17Lut = 0x3e8;
                    data[17] |= dw17Lut;

                    static const uint32_t dw18Lut = 0x80000;
                    data[18] |= dw18Lut;

                    static const uint32_t dw19Lut = 0x18000040;
                    data[19] |= dw19Lut;

                    static const uint32_t dw28Lut = 0x7d00fa0;
                    data[28] |= dw28Lut;

                    static const uint32_t dw29Lut = 0x2bc0bb8;
                    data[29] |= dw29Lut;

                    static const uint32_t dw30Lut = 0x32003e8;
                    data[30] |= dw30Lut;

                    static const uint32_t dw31Lut = 0x1f4012c;
                    data[31] |= dw31Lut;

                    static const uint32_t dw32Lut = 0x55220190;
                    data[32] |= dw32Lut;

                    static const uint32_t dw33Lut = 0x22552222;
                    data[33] |= dw33Lut;

                    static const uint32_t dw34Lut = 0x225522;
                    data[34] |= dw34Lut;

                    static const uint32_t dw51Lut[3] = { 0x33331502, 0x22223502, 0x12227106,};
                    data[51] |= dw51Lut[TargetUsagediv3];

                    static const uint32_t dw52Lut[3] = { 0x77f5bdb, 0x72d595b, 0x9295a59,};
                    data[52] |= dw52Lut[TargetUsagediv3];

                    static const uint32_t dw53Lut[3] = { 0xffffffff, 0xff000000, 0xffff0000,};
                    data[53] |= dw53Lut[TargetUsagediv3];

                    static const uint32_t dw54Lut[3] = { 0, 0x44000000, 0x8c00000c,};
                    data[54] |= dw54Lut[TargetUsagediv3];

                    return MOS_STATUS_SUCCESS;
                });

            return MOS_STATUS_SUCCESS;
        });
#endif  // _MEDIA_RESERVED

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
