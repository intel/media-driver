/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     encode_vp9_vdenc_const_settings_xe3_lpm.cpp
//! \brief    Defines the common interface for vp9 vdenc const settings
//!

#include "encode_vp9_vdenc_const_settings_xe3_lpm.h"
#include "encode_utils.h"

namespace encode
{

const uint32_t EncodeVp9VdencConstSettingsXe3_Lpm::m_numMergeCandidateCu64x64Xe3_Lpm[] = {0, 3, 3, 3, 3, 3, 2, 2};
const uint32_t EncodeVp9VdencConstSettingsXe3_Lpm::m_numMergeCandidateCu32x32Xe3_Lpm[] = {0, 3, 3, 3, 3, 3, 2, 2};
const uint32_t EncodeVp9VdencConstSettingsXe3_Lpm::m_numMergeCandidateCu16x16Xe3_Lpm[] = {0, 3, 3, 2, 2, 2, 2, 2};
const uint32_t EncodeVp9VdencConstSettingsXe3_Lpm::m_numMergeCandidateCu8x8Xe3_Lpm[]   = {0, 3, 3, 2, 2, 2, 1, 1};
const uint8_t  EncodeVp9VdencConstSettingsXe3_Lpm::m_numImePredictorsXe3_Lpm[]         = {0, 12, 12, 8, 8, 8, 4, 4};


MOS_STATUS EncodeVp9VdencConstSettingsXe3_Lpm::SetTUSettings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto setting = static_cast<Vp9VdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->NumMergeCandidateCu8x8   = (uint32_t *)m_numMergeCandidateCu8x8Xe3_Lpm;
    setting->NumMergeCandidateCu16x16 = (uint32_t *)m_numMergeCandidateCu16x16Xe3_Lpm;
    setting->NumMergeCandidateCu32x32 = (uint32_t *)m_numMergeCandidateCu32x32Xe3_Lpm;
    setting->NumMergeCandidateCu64x64 = (uint32_t *)m_numMergeCandidateCu64x64Xe3_Lpm;
    setting->NumImePredictors         = (uint8_t *)m_numImePredictorsXe3_Lpm;

    return eStatus;
}

MOS_STATUS EncodeVp9VdencConstSettingsXe3_Lpm::SetVdencCmd2Settings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Vp9VdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

#if _MEDIA_RESERVED
#define VDENC_CMD2_SETTINGS_EXT
#include "encode_vp9_vdenc_const_settings_xe3_lpm_ext.h"
#undef VDENC_CMD2_SETTINGS_EXT
#else
    setting->vdencCmd2Settings.emplace_back(
        VDENC_CMD2_LAMBDA() {
            par.extSettings.emplace_back(
                [this, &par](uint32_t *data) {
                    auto waTable = m_osItf->pfnGetWaTable(m_osItf);
                    ENCODE_CHK_NULL_RETURN(waTable);
                    uint32_t TargetUsage      = m_vp9SeqParams->TargetUsage;
                    uint32_t l0RefNum         = par.numRefL0;
                    uint32_t isWa_15017562431 = MEDIA_IS_WA(waTable, Wa_15017562431);

                    static const uint32_t dw2Lut[8] = { 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x20000003, 0x20000003,};
                    data[2] |= dw2Lut[TargetUsage];

                    static const uint32_t dw5Lut = 0xc0a000;
                    data[5] |= dw5Lut;

                    static const uint32_t dw6Lut = 0x20080200;
                    data[6] |= dw6Lut;

                    static const uint32_t dw7Lut[4] = { 0xe2000, 0xe2000, 0x62000, 0x62000,};
                    data[7] |= dw7Lut[l0RefNum];

                    static const uint32_t dw9Lut[8] = { 0xc40000, 0xc40000, 0xc40000, 0x840000, 0x840000, 0x840000, 0x420000, 0x420000,};
                    data[9] |= dw9Lut[TargetUsage];

                    static const uint32_t dw12Lut = 0xffffffff;
                    data[12] |= dw12Lut;

                    static const uint32_t dw14Lut = 0x1f40000;
                    data[14] |= dw14Lut;

                    static const uint32_t dw15Lut = 0x138807d0;
                    data[15] |= dw15Lut;

                    static const uint32_t dw16Lut = 0xf000000;
                    data[16] |= dw16Lut;

                    static const uint32_t dw17Lut = 0x2710;
                    data[17] |= dw17Lut;

                    static const uint32_t dw18Lut = 0x80000;
                    data[18] |= dw18Lut;

                    static const uint32_t dw19Lut = 0x18000040;
                    data[19] |= dw19Lut;

                    static const uint32_t dw23Lut = 0x6a1a0000;
                    data[23] |= dw23Lut;

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

                    static const uint32_t dw35Lut = 0x800;
                    data[35] |= dw35Lut;

                    static const uint32_t dw51Lut[8] = { 0x33331502, 0x33331502, 0x33331502, 0x22333102, 0x22333102, 0x22333102, 0x12227256, 0x12227256,};
                    data[51] |= dw51Lut[TargetUsage];

                    static const uint32_t dw52Lut[8][2] = { { 0x77f5bdb, 0x77f5bdb,}, { 0x77f5bdb, 0x77f5bdb,}, { 0x77f5bdb, 0x77f5bdb,}, { 0x72d5949, 0x72d5949,}, { 0x72d5949, 0x472d5949,}, { 0x72d5949, 0x72d5949,}, { 0x9295a5a, 0x9295a5a,}, { 0x9295a5a, 0x9295a5a,},};
                    data[52] |= dw52Lut[TargetUsage][isWa_15017562431];

                    static const uint32_t dw53Lut[8] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xfff0ffff, 0xfff0ffff, 0xfff0ffff, 0xffffffff, 0xffffffff,};
                    data[53] |= dw53Lut[TargetUsage];

                    static const uint32_t dw54Lut[8] = { 0, 0, 0, 0xc4000000, 0xc4000000, 0xc4000000, 0xbc00000c, 0xbc00000c,};
                    data[54] |= dw54Lut[TargetUsage];

                    return MOS_STATUS_SUCCESS;
                });

            return MOS_STATUS_SUCCESS;
        });
#endif  // _MEDIA_RESERVED

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
