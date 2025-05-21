/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_hevc_vdenc_const_settings_xe_xpm_base.cpp
//! \brief    Defines the common interface for M12.5 Plus hevc vdenc const settings
//!

#include "encode_hevc_vdenc_const_settings_xe_xpm_base.h"
#include "codec_def_common.h"
#include "codec_def_common_encode.h"
#include "codec_def_encode_hevc.h"
#include "encode_const_settings.h"
#include "encode_utils.h"
#include "mhw_vdbox_vdenc_cmdpar.h"
#include "mos_os.h"
#include "mos_solo_generic.h"
#include <stddef.h>
#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <vector>
#if _ENCODE_RESERVED
#include "mhw_vdbox_vdenc_cmdpar_ext.h"
#endif // _ENCODE_RESERVED

namespace encode
{
MOS_STATUS EncodeHevcVdencConstSettingsXe_Xpm_Base::SetTUSettings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->rdoqEnable = {true, true, true, true, true, true, true, true};
    setting->acqpEnable = {true, true, true, true, true, true, true, false};

    return eStatus;
}

 MOS_STATUS EncodeHevcVdencConstSettingsXe_Xpm_Base::SetVdencStreaminStateSettings()
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
                    {0, 3, 3, 2, 2, 2, 2, 2},
                    {0, 3, 3, 2, 2, 2, 2, 2},
                    {0, 3, 3, 2, 2, 2, 2, 2},
                    {0, 3, 3, 2, 2, 2, 2, 2},
                }};

            static const std::array<
                uint8_t,
                NUM_TARGET_USAGE_MODES + 1>
                numImePredictors = {0, 8, 8, 6, 6, 6, 4, 4};

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

MOS_STATUS EncodeHevcVdencConstSettingsXe_Xpm_Base::SetVdencCmd1Settings()
{
    ENCODE_FUNC_CALL();

    EncodeHevcVdencConstSettings::SetVdencCmd1Settings();

    auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            static const std::array<uint8_t, 12> data = {
                3, 10, 16, 22, 29, 35, 42, 48, 54, 61, 67, 74
            };

            for (size_t i = 0; i < data.size(); i++)
            {
                par.vdencCmd1Par4[i] = data[i];
            }

            return MOS_STATUS_SUCCESS;
        });

    setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            par.vdencCmd1Par55 = 0x0E;
            par.vdencCmd1Par56 = 0x0E;
            par.vdencCmd1Par57 = 0x0C;
            par.vdencCmd1Par58 = 0x0B;
            par.vdencCmd1Par59 = 0x10;
            par.vdencCmd1Par60 = 0x10;
            par.vdencCmd1Par61 = 0x0F;
            par.vdencCmd1Par62 = 0x0F;
            par.vdencCmd1Par63 = 0x10;
            par.vdencCmd1Par64 = 0x10;
            par.vdencCmd1Par65 = 0x10;
            par.vdencCmd1Par66 = 0x10;
            par.vdencCmd1Par67 = 0x14;
            par.vdencCmd1Par68 = 0x10;
            par.vdencCmd1Par69 = 0x10;
            par.vdencCmd1Par70 = 0x10;
            par.vdencCmd1Par71 = 0x0C;
            par.vdencCmd1Par72 = 0x0C;
            par.vdencCmd1Par73 = 0x0A;
            par.vdencCmd1Par74 = 0x0A;
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
            par.vdencCmd1Par85 = 0x0E;
            par.vdencCmd1Par86 = 0x0F;

            return MOS_STATUS_SUCCESS;
        });

    setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            static const std::array<uint8_t, 16> data = {
                11, 0, 0, 0, 14, 0, 0, 0,
                11, 0, 0, 0, 0, 0, 0, 0
            };

            if (m_hevcPicParams->CodingType == I_TYPE)
            {
                return MOS_STATUS_SUCCESS;
            }

            for (size_t i = 0; i < 4; i++)
            {
                par.vdencCmd1Par8[i]  = data[i];
                par.vdencCmd1Par9[i]  = data[i + 4];
                par.vdencCmd1Par10[i] = data[i + 8];
                par.vdencCmd1Par11[i] = data[i + 12];
            }

            return MOS_STATUS_SUCCESS;
        });

    setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            static const std::array<uint8_t, 16> data = {
                23, 0, 0, 0, 26, 0, 0, 0,
                21, 0, 0, 0, 0, 0, 0, 0
            };

            if (m_hevcPicParams->CodingType == I_TYPE)
            {
                return MOS_STATUS_SUCCESS;
            }

            for (size_t i = 0; i < 4; i++)
            {
                par.vdencCmd1Par12[i] = data[i];
                par.vdencCmd1Par13[i] = data[i + 4];
                par.vdencCmd1Par14[i] = data[i + 8];
                par.vdencCmd1Par15[i] = data[i + 12];
            }

            return MOS_STATUS_SUCCESS;
        });

    setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            if (m_hevcPicParams->CodingType == P_TYPE)
            {
                par.vdencCmd1Par16 = 82;
                par.vdencCmd1Par17 = 20;
                par.vdencCmd1Par18 = 83;
                par.vdencCmd1Par19 = 17;
                par.vdencCmd1Par20 = 15;
                par.vdencCmd1Par21 = 0;
            }
            else if (m_hevcPicParams->CodingType == B_TYPE)
            {
                par.vdencCmd1Par16 = 99;
                par.vdencCmd1Par17 = 23;
                par.vdencCmd1Par18 = 99;
                par.vdencCmd1Par19 = 19;
                par.vdencCmd1Par20 = 17;
                par.vdencCmd1Par21 = 0;
            }

            return MOS_STATUS_SUCCESS;
        });

    setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            if (m_hevcPicParams->CodingType == I_TYPE)
            {
                par.vdencCmd1Par23 = 63;
            }
            else
            {
                par.vdencCmd1Par23 = 54;
            }

            return MOS_STATUS_SUCCESS;
        });

    setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            if (m_hevcPicParams->CodingType == I_TYPE)
            {
                par.vdencCmd1Par30 = 12;
            }

            return MOS_STATUS_SUCCESS;
        });

    setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            if (m_hevcPicParams->CodingType == I_TYPE)
            {
                par.vdencCmd1Par36 = 17;
                par.vdencCmd1Par37 = 47;
                par.vdencCmd1Par38 = 20;
                par.vdencCmd1Par39 = 9;
                par.vdencCmd1Par40 = 17;
                par.vdencCmd1Par41 = m_hevcPicParams->NumROI ? 0 : 30;
            }
            else
            {
                par.vdencCmd1Par36 = 7;
                par.vdencCmd1Par37 = 18;
                par.vdencCmd1Par38 = 18;
                par.vdencCmd1Par39 = 18;
                par.vdencCmd1Par40 = 27;
                par.vdencCmd1Par41 = m_hevcPicParams->NumROI ? 0 : 68;
            }

            return MOS_STATUS_SUCCESS;
        });

    setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            if (m_hevcPicParams->CodingType == P_TYPE)
            {
                par.vdencCmd1Par48 = 0;
                par.vdencCmd1Par49 = 32;
                par.vdencCmd1Par50 = 68;
            }

            return MOS_STATUS_SUCCESS;
        });

    setting->vdencCmd1Settings.emplace_back(
        VDENC_CMD1_LAMBDA() {
            static constexpr std::array<
                std::array<uint8_t,
                    3>,
                3>
                data = {{
                    {20, 35, 35},
                    {20, 35, 35},
                    {47, 16, 16}
                    }};

            if (m_hevcPicParams->CodingType == I_TYPE)
            {
                par.vdencCmd1Par87 = data[2][2];
                par.vdencCmd1Par88 = data[2][1];
                par.vdencCmd1Par89 = data[2][0];
            }
            else if (m_hevcPicParams->CodingType == P_TYPE)
            {
                par.vdencCmd1Par87 = data[1][2];
                par.vdencCmd1Par88 = data[1][1];
                par.vdencCmd1Par89 = data[1][0];
            }
            else if (m_hevcPicParams->CodingType == B_TYPE)
            {
                par.vdencCmd1Par87 = data[0][2];
                par.vdencCmd1Par88 = data[0][1];
                par.vdencCmd1Par89 = data[0][0];
            }

            return MOS_STATUS_SUCCESS;
        });

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencConstSettingsXe_Xpm_Base::SetVdencCmd2Settings()
{

    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

#if !(_MEDIA_RESERVED)
    setting->vdencCmd2Settings.emplace_back(
        VDENC_CMD2_LAMBDA()
        {
            par.extSettings.emplace_back(
                [this, isLowDelay, &par](uint32_t *data) {
                    auto waTable = m_osItf->pfnGetWaTable(m_osItf);
                    ENCODE_CHK_NULL_RETURN(waTable);

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
                    uint32_t Wa_22012463389                = MEDIA_IS_WA(waTable, Wa_22012463389);
                    uint32_t WaEnableOnlyASteppingFeatures = MEDIA_IS_WA(waTable, WaEnableOnlyASteppingFeatures);
                    uint32_t Wa_22011549751                = MEDIA_IS_WA(waTable, Wa_22011549751);
                    uint32_t Wa_14010476401                = MEDIA_IS_WA(waTable, Wa_14010476401);
                    uint32_t Wa_22011531258                = MEDIA_IS_WA(waTable, Wa_22011531258);

                    static const uint32_t dw2Lut[3][2] = { { 0x3, 0x2,}, { 0x3, 0x3,}, { 0x3, 0x3,},};
                    data[2] |= dw2Lut[CodingTypeMinus1][currPicRef];

                    static const uint32_t dw5Lut[3] = { 0xc0a000, 0xc1a000, 0xc0a000,};
                    data[5] |= dw5Lut[CodingTypeMinus1];

                    static const uint32_t dw7Lut[3][2][2][2][2] = { { { { { 0x64003, 0xe4003,}, { 0x64003, 0x64003,},}, { { 0x64003, 0xe4003,}, { 0x64003, 0x64003,},},}, { { { 0x64003, 0xe4003,}, { 0x64003, 0x64003,},}, { { 0x64003, 0xe4003,}, { 0x64003, 0x64003,},},},}, { { { { 0x64003, 0x64003,}, { 0x64003, 0x64003,},}, { { 0x64003, 0x64003,}, { 0x64003, 0x64003,},},}, { { { 0x64003, 0x64003,}, { 0x64003, 0x64003,},}, { { 0xe4003, 0xe4003,}, { 0xe4003, 0xe4003,},},},}, { { { { 0x64003, 0x64003,}, { 0x64003, 0x64003,},}, { { 0x64003, 0x64003,}, { 0x64003, 0x64003,},},}, { { { 0x64003, 0x64003,}, { 0x64003, 0x64003,},}, { { 0xe4003, 0xe4003,}, { 0xe4003, 0xe4003,},},},},};
                    data[7] |= dw7Lut[CodingTypeMinus1][numL0Minus1Is0][lowDelay][currPicRef][Wa_22011549751];

                    static const uint32_t dw8Lut[3][8][2] = { { { 0x54555555, 0,}, { 0x54555555, 0,}, { 0x54555555, 0,}, { 0x54555555, 0,}, { 0x54555555, 0,}, { 0x54555555, 0,}, { 0x54555555, 0,}, { 0x54555555, 0,},}, { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0xfffdccaa, 0xfffdccaa,}, { 0x55550000, 0x55550000,}, { 0xfffdccaa, 0xfffdccaa,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0xfffdfcaa, 0xfffdccaa,}, { 0x55550000, 0x55550000,}, { 0xfffdfcaa, 0xfffdccaa,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},};
                    data[8] |= dw8Lut[CodingTypeMinus1][tu][lowDelay];

                    static const uint32_t dw9Lut[3][8][2][2][2] = { { { { { 0x5555, 0x5555,}, { 0x5555, 0x5555,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x845555, 0x45555,}, { 0x845555, 0x845555,},}, { { 0x840000, 0x40000,}, { 0x840000, 0x840000,},},}, { { { 0x845555, 0x45555,}, { 0x845555, 0x845555,},}, { { 0x840000, 0x40000,}, { 0x840000, 0x840000,},},}, { { { 0x635555, 0x35555,}, { 0x635555, 0x635555,},}, { { 0x630000, 0x30000,}, { 0x630000, 0x630000,},},}, { { { 0x635555, 0x35555,}, { 0x635555, 0x635555,},}, { { 0x630000, 0x30000,}, { 0x630000, 0x630000,},},}, { { { 0x635555, 0x35555,}, { 0x635555, 0x635555,},}, { { 0x630000, 0x30000,}, { 0x630000, 0x630000,},},}, { { { 0x425555, 0x25555,}, { 0x425555, 0x425555,},}, { { 0x420000, 0x20000,}, { 0x420000, 0x420000,},},}, { { { 0x425555, 0x25555,}, { 0x425555, 0x425555,},}, { { 0x420000, 0x20000,}, { 0x420000, 0x420000,},},},}, { { { { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x840000, 0x840000,}, { 0x840000, 0x840000,},}, { { 0x840000, 0x840000,}, { 0x840000, 0x840000,},},}, { { { 0x84ffff, 0x84ffff,}, { 0x84ffff, 0x84ffff,},}, { { 0x84ffff, 0x84ffff,}, { 0x84ffff, 0x84ffff,},},}, { { { 0x630000, 0x630000,}, { 0x630000, 0x630000,},}, { { 0x630000, 0x630000,}, { 0x630000, 0x630000,},},}, { { { 0x63ffff, 0x63ffff,}, { 0x63ffff, 0x63ffff,},}, { { 0x63ffff, 0x63ffff,}, { 0x63ffff, 0x63ffff,},},}, { { { 0x630000, 0x630000,}, { 0x630000, 0x630000,},}, { { 0x630000, 0x630000,}, { 0x630000, 0x630000,},},}, { { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},}, { { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},},}, { { { { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,},},}, { { { 0x840000, 0x840000,}, { 0x840000, 0x840000,},}, { { 0x840000, 0x840000,}, { 0x840000, 0x840000,},},}, { { { 0x84fcff, 0x84fcff,}, { 0x84fcff, 0x84fcff,},}, { { 0x84ffff, 0x84ffff,}, { 0x84ffff, 0x84ffff,},},}, { { { 0x630000, 0x630000,}, { 0x630000, 0x630000,},}, { { 0x630000, 0x630000,}, { 0x630000, 0x630000,},},}, { { { 0x63fcff, 0x63fcff,}, { 0x63fcff, 0x63fcff,},}, { { 0x63ffff, 0x63ffff,}, { 0x63ffff, 0x63ffff,},},}, { { { 0x630000, 0x630000,}, { 0x630000, 0x630000,},}, { { 0x630000, 0x630000,}, { 0x630000, 0x630000,},},}, { { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},}, { { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},},},};
                    data[9] |= dw9Lut[CodingTypeMinus1][tu][lowDelay][currPicRef][Wa_22011549751];

                    static const uint32_t dw12Lut[8] = { 0, 0xffffffff, 0xffffffff, 0xce4014a0, 0xce4014a0, 0xce4014a0, 0x89800dc0, 0x89800dc0,};
                    data[12] |= dw12Lut[tu];

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

                    static const uint32_t dw51Lut[3][8][2][2][2][2] = { { { { { { 0, 0x20000000,}, { 0, 0x20000000,},}, { { 0x10, 0x20000010,}, { 0x10, 0x20000010,},},}, { { { 0x10, 0x10,}, { 0x10, 0x10,},}, { { 0x10, 0x10,}, { 0x10, 0x10,},},},}, { { { { 0x33331552, 0x20001552,}, { 0x33331552, 0x20001552,},}, { { 0x33331552, 0x20001552,}, { 0x33331552, 0x20001552,},},}, { { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},}, { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},},},}, { { { { 0x33331552, 0x20001552,}, { 0x33331552, 0x20001552,},}, { { 0x33331552, 0x20001552,}, { 0x33331552, 0x20001552,},},}, { { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},}, { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},},},}, { { { { 0x22223552, 0x20003552,}, { 0x22223552, 0x20003552,},}, { { 0x22223552, 0x20003552,}, { 0x22223552, 0x20003552,},},}, { { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},},},}, { { { { 0x22223552, 0x20003552,}, { 0x22223552, 0x20003552,},}, { { 0x22223552, 0x20003552,}, { 0x22223552, 0x20003552,},},}, { { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},},},}, { { { { 0x22223552, 0x20003552,}, { 0x22223552, 0x20003552,},}, { { 0x22223552, 0x20003552,}, { 0x22223552, 0x20003552,},},}, { { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},},},}, { { { { 0x22227152, 0x20007152,}, { 0x22227152, 0x20007152,},}, { { 0x22227152, 0x20007152,}, { 0x22227152, 0x20007152,},},}, { { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},}, { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},},},}, { { { { 0x22227152, 0x20007152,}, { 0x22227152, 0x20007152,},}, { { 0x22227152, 0x20007152,}, { 0x22227152, 0x20007152,},},}, { { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},}, { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},},},},}, { { { { { 0, 0,}, { 0x20000000, 0x20000000,},}, { { 0x10, 0x10,}, { 0x20000010, 0x20000010,},},}, { { { 0x10, 0x10,}, { 0x20000010, 0x20000010,},}, { { 0x10, 0x10,}, { 0x20000010, 0x20000010,},},},}, { { { { 0x33331552, 0x33331552,}, { 0x20001552, 0x20001552,},}, { { 0x33331552, 0x33331552,}, { 0x20001552, 0x20001552,},},}, { { { 0x33331552, 0x33331552,}, { 0x20001552, 0x20001552,},}, { { 0x33331552, 0x33331552,}, { 0x20001552, 0x20001552,},},},}, { { { { 0x33331552, 0x33331552,}, { 0x20001552, 0x20001552,},}, { { 0x33331552, 0x33331552,}, { 0x20001552, 0x20001552,},},}, { { { 0x33331552, 0x33331552,}, { 0x20001552, 0x20001552,},}, { { 0x33331552, 0x33331552,}, { 0x20001552, 0x20001552,},},},}, { { { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},}, { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},},}, { { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},}, { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},},},}, { { { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},}, { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},},}, { { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},}, { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},},},}, { { { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},}, { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},},}, { { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},}, { { 0x22223552, 0x22223552,}, { 0x20003552, 0x20003552,},},},}, { { { { 0x22227152, 0x22227152,}, { 0x20007152, 0x20007152,},}, { { 0x22227152, 0x22227152,}, { 0x20007152, 0x20007152,},},}, { { { 0x22227152, 0x22227152,}, { 0x20007152, 0x20007152,},}, { { 0x22227152, 0x22227152,}, { 0x20007152, 0x20007152,},},},}, { { { { 0x22227152, 0x22227152,}, { 0x20007152, 0x20007152,},}, { { 0x22227152, 0x22227152,}, { 0x20007152, 0x20007152,},},}, { { { 0x22227152, 0x22227152,}, { 0x20007152, 0x20007152,},}, { { 0x22227152, 0x22227152,}, { 0x20007152, 0x20007152,},},},},}, { { { { { 0, 0,}, { 0, 0,},}, { { 0x10, 0x10,}, { 0x10, 0x10,},},}, { { { 0x10, 0x10,}, { 0x10, 0x10,},}, { { 0x10, 0x10,}, { 0x10, 0x10,},},},}, { { { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},}, { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},},}, { { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},}, { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},},},}, { { { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},}, { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},},}, { { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},}, { { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},},},}, { { { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},},}, { { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},},},}, { { { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},},}, { { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},},},}, { { { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},},}, { { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},},},}, { { { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},}, { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},},}, { { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},}, { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},},},}, { { { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},}, { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},},}, { { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},}, { { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},},},},},};
                    data[51] |= dw51Lut[CodingTypeMinus1][tu][currPicRef][paletteMode][Wa_22012463389][Wa_22011549751];

                    static const uint32_t dw52Lut[8] = { 0, 0x77f5bdb, 0x77f5bdb, 0x72d5959, 0x72d5959, 0x72d5959, 0x929595a, 0x929595a,};
                    data[52] |= dw52Lut[tu];

                    static const uint32_t dw53Lut[3][8][2][2][2][2] = { { { { { { 0, 0,}, { 0x80000000, 0x80000000,},}, { { 0, 0,}, { 0x80000000, 0x80000000,},},}, { { { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,},},},}, { { { { 0xffffffff, 0xffffffff,}, { 0x80000000, 0x80000000,},}, { { 0xffffffff, 0xffffffff,}, { 0x80000000, 0x80000000,},},}, { { { 0xffffffff, 0xffffffff,}, { 0xffffffff, 0xffffffff,},}, { { 0xffffffff, 0xffffffff,}, { 0xffffffff, 0xffffffff,},},},}, { { { { 0xffffffff, 0xffffffff,}, { 0x80000000, 0x80000000,},}, { { 0xffffffff, 0xffffffff,}, { 0x80000000, 0x80000000,},},}, { { { 0xffffffff, 0xffffffff,}, { 0xffffffff, 0xffffffff,},}, { { 0xffffffff, 0xffffffff,}, { 0xffffffff, 0xffffffff,},},},}, { { { { 0xff000000, 0xff000000,}, { 0x80000000, 0x80000000,},}, { { 0xff000000, 0xff000000,}, { 0x80000000, 0x80000000,},},}, { { { 0xff000000, 0xff000000,}, { 0xff000000, 0xff000000,},}, { { 0xff000000, 0xff000000,}, { 0xff000000, 0xff000000,},},},}, { { { { 0xff000000, 0xff000000,}, { 0x80000000, 0x80000000,},}, { { 0xff000000, 0xff000000,}, { 0x80000000, 0x80000000,},},}, { { { 0xff000000, 0xff000000,}, { 0xff000000, 0xff000000,},}, { { 0xff000000, 0xff000000,}, { 0xff000000, 0xff000000,},},},}, { { { { 0xff000000, 0xff000000,}, { 0x80000000, 0x80000000,},}, { { 0xff000000, 0xff000000,}, { 0x80000000, 0x80000000,},},}, { { { 0xff000000, 0xff000000,}, { 0xff000000, 0xff000000,},}, { { 0xff000000, 0xff000000,}, { 0xff000000, 0xff000000,},},},}, { { { { 0xffff0000, 0xffff0000,}, { 0x80000000, 0x80000000,},}, { { 0xffff0000, 0xffff0000,}, { 0x80000000, 0x80000000,},},}, { { { 0xffff0000, 0xffff0000,}, { 0xffff0000, 0xffff0000,},}, { { 0xffff0000, 0xffff0000,}, { 0xffff0000, 0xffff0000,},},},}, { { { { 0xffff0000, 0xffff0000,}, { 0x80000000, 0x80000000,},}, { { 0xffff0000, 0xffff0000,}, { 0x80000000, 0x80000000,},},}, { { { 0xffff0000, 0xffff0000,}, { 0xffff0000, 0xffff0000,},}, { { 0xffff0000, 0xffff0000,}, { 0xffff0000, 0xffff0000,},},},},}, { { { { { 0, 0xfff0,}, { 0, 0xfff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},}, { { { 0, 0xfff0,}, { 0, 0xfff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},},}, { { { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},}, { { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},},}, { { { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},}, { { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},},}, { { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},}, { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},},}, { { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},}, { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},},}, { { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},}, { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},},}, { { { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},}, { { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},},}, { { { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},}, { { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0x80000000, 0x80000000,}, { 0x80000000, 0x80000000,},},},},}, { { { { { 0, 0xfff0,}, { 0, 0xfff0,},}, { { 0, 0xfff0,}, { 0, 0xfff0,},},}, { { { 0, 0xfff0,}, { 0, 0xfff0,},}, { { 0, 0xfff0,}, { 0, 0xfff0,},},},}, { { { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},}, { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},},}, { { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},}, { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},},},}, { { { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},}, { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},},}, { { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},}, { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},},},}, { { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},},}, { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},},},}, { { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},},}, { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},},},}, { { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},},}, { { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},},},}, { { { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},},}, { { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},},},}, { { { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},},}, { { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},},},},},};
                    data[53] |= dw53Lut[CodingTypeMinus1][tu][currPicRef][Wa_22012463389][Wa_22011549751][Wa_14010476401];

                    static const uint32_t dw54Lut[8][2] = { { 0, 0,}, { 0xc0, 0xc0,}, { 0, 0,}, { 0x44000000, 0x44000000,}, { 0x44000000, 0x44000000,}, { 0x4000000, 0x4000000,}, { 0xbc000000, 0x34000000,}, { 0xbc000000, 0x34000000,},};
                    data[54] |= dw54Lut[tu][Wa_22011531258];

                    static const uint32_t dw55Lut[2] = { 0, 0xcdef0123,};
                    data[55] |= dw55Lut[rdoq];

                    static const uint32_t dw56Lut[3][2][2][5][4][2] = { { { { { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0x300, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0x300, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},},}, { { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0x30b, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0x30b, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},},},}, { { { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0x300, 0,},},}, { { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0x30b, 0xb,},},},},}, { { { { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0x300, 0,}, { 0x300, 0,}, { 0x300, 0,}, { 0x300, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},},}, { { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0x30b, 0xb,}, { 0x30b, 0xb,}, { 0x30b, 0xb,}, { 0x30b, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},},},}, { { { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0x300, 0,}, { 0x300, 0,}, { 0x300, 0,}, { 0x300, 0,},},}, { { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0x30b, 0xb,}, { 0x30b, 0xb,}, { 0x30b, 0xb,}, { 0x30b, 0xb,},},},},}, { { { { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0x300, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0x300, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},},}, { { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0x30b, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0x30b, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},},},}, { { { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0x300, 0,},},}, { { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,},}, { { 0xb, 0xb,}, { 0xb, 0xb,}, { 0xb, 0xb,}, { 0x30b, 0xb,},},},},},};
                    data[56] |= dw56Lut[CodingTypeMinus1][currPicRef][rdoq][numRef0][numRef1][WaEnableOnlyASteppingFeatures];

                    static const uint32_t dw57Lut[2] = { 0, 0x508c23,};
                    data[57] |= dw57Lut[rdoq];

                    static const uint32_t dw58Lut[2] = { 0, 0x466419,};
                    data[58] |= dw58Lut[rdoq];

                    static const uint32_t dw59Lut[2] = { 0, 0x7d6c5c4b,};
                    data[59] |= dw59Lut[rdoq];

                    static const uint32_t dw60Lut[2] = { 0, 0xbfaf9e8e,};
                    data[60] |= dw60Lut[rdoq];

                    static const uint32_t dwsLut[] = {0x80000000, 0xf000000, 0x98000000, 0xcccc0000, 0x7d00fa0, 0x2bc0bb8, 0x32003e8, 0x1f4012c, 0x190, 0xecc};
                    data[11] |= dwsLut[0];
                    data[16] |= dwsLut[1];
                    data[19] |= dwsLut[2];
                    data[23] |= dwsLut[3];
                    data[28] |= dwsLut[4];
                    data[29] |= dwsLut[5];
                    data[30] |= dwsLut[6];
                    data[31] |= dwsLut[7];
                    data[32] |= dwsLut[8];
                    data[35] |= dwsLut[9];

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
#else
#define VDENC_CMD2_SETTINGS_EXT
    #include "encode_hevc_vdenc_const_settings_xe_xpm_base_ext.h"
#undef VDENC_CMD2_SETTINGS_EXT
#endif  // !(_MEDIA_RESERVED)

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
