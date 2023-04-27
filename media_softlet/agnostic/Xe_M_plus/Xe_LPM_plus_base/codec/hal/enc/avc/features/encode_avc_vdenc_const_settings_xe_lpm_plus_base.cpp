/*
 * Copyright (c) 2022, Intel Corporation
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
//! \file     encode_avc_vdenc_const_settings_xe_lpm_plus_base.cpp
//! \brief    Defines the common interface for Xe_LPM_PLUS_BASE avc vdenc const settings
//!

#include "encode_avc_vdenc_const_settings_xe_lpm_plus_base.h"
#include "encode_utils.h"

namespace encode
{

const uint8_t  AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_BRC_UPD_global_rate_ratio_threshold_Xe_Lpm_Plus_Base[7]       = {40, 75, 97, 103, 125, 160, 0};
const uint8_t  AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_BRC_UPD_slwin_global_rate_ratio_threshold_Xe_Lpm_Plus_Base[7] = {40, 75, 97, 103, 125, 160, 0};
const int8_t   AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_BRC_UPD_global_rate_ratio_threshold_qp_Xe_Lpm_Plus_Base[8]    = {-3, -2, -1, 0, 1, 2, 3, 0};

const int8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_BRC_UPD_GlobalRateQPAdjTabI_U8_Xe_Lpm_Plus_Base[64] =
{
     1,  2,  3,  5,  6,
     1,  1,  2,  3,  5,
     0,  0,  1,  2,  3,
    -1,  0,  0,  1,  2,
    -1,  0,  0,  0,  1,
    -2, -2, -1,  0,  1,
    -3, -3, -1, -1,  0,
    -5, -3, -2, -1, -1,
    -6, -5, -3, -2, -1,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,
};

const int8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_BRC_UPD_GlobalRateQPAdjTabP_U8_Xe_Lpm_Plus_Base[64] =
{
     1,  2,  3,  5,  6,
     1,  1,  2,  3,  5,
     0,  1,  1,  2,  3,
    -1,  0,  0,  1,  2,
    -1,  0,  0,  0,  1,
    -1, -1, -1,  0,  1,
    -2, -1, -1, -1,  0,
    -4, -2, -1, -1,  0,
    -5, -4, -2, -1, -1,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,
};

// P picture global rate QP Adjustment table for sliding window BRC
const int8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_BRC_UPD_SlWinGlobalRateQPAdjTabP_U8_Xe_Lpm_Plus_Base[64] =
{
     1,  2,  3,  5,  6,
     1,  1,  2,  3,  5,
     0,  1,  1,  2,  3,
    -1,  0,  0,  1,  2,
    -1,  0,  0,  0,  1,
    -1, -1, -1,  0,  1,
    -2, -1, -1, -1,  0,
    -4, -2, -1, -1,  0,
    -5, -4, -2, -1, -1,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,
};

const int8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_BRC_UPD_GlobalRateQPAdjTabB_U8_Xe_Lpm_Plus_Base[64] =
{
     1,  1,  2,  4,  5,
     1,  1,  1,  2,  4,
     0,  0,  1,  1,  2,
    -1,  0,  0,  1,  1,
    -1,  0,  0,  0,  0,
    -1, -1, -1,  0,  1,
    -2, -1, -1, -1,  0,
    -3, -2, -1, -1,  0,
    -5, -4, -2, -1, -1,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,  0,
     0,  0,  0,  0,
};

const uint8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_BRC_UPD_DistThreshldI_U8_Xe_Lpm_Plus_Base[10] = {4, 30, 60, 80, 120, 140, 200, 255, 0, 0};
const uint8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_BRC_UPD_DistThreshldP_U8_Xe_Lpm_Plus_Base[10] = {4, 30, 60, 80, 120, 140, 200, 255, 0, 0};
const uint8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_BRC_UPD_DistThreshldB_U8_Xe_Lpm_Plus_Base[10] = {2, 20, 40, 70, 130, 160, 200, 255, 0, 0};

const int8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_CBR_UPD_DistQPAdjTabI_U8_Xe_Lpm_Plus_Base[81] =
{
     0,  0,  0,  0, 0, 2, 3, 3, 4,
     0,  0,  0,  0, 0, 2, 3, 3, 4,
    -1,  0,  0,  0, 0, 2, 2, 3, 3,
    -1, -1,  0,  0, 0, 1, 2, 2, 2,
    -1, -1, -1,  0, 0, 0, 1, 2, 2,
    -2, -1, -1,  0, 0, 0, 1, 1, 2,
    -2, -1, -1, -1, 0, 0, 1, 1, 2,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
};

const int8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_CBR_UPD_DistQPAdjTabP_U8_Xe_Lpm_Plus_Base[81] =
{
     0,  0,  0,  0, 0, 2, 3, 3, 4,
     0,  0,  0,  0, 0, 2, 3, 3, 4,
    -1,  0,  0,  0, 0, 2, 2, 3, 3,
    -1, -1,  0,  0, 0, 1, 2, 2, 2,
    -1, -1, -1,  0, 0, 0, 1, 2, 2,
    -2, -1, -1,  0, 0, 0, 1, 1, 2,
    -2, -1, -1, -1, 0, 0, 1, 1, 2,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
};

const int8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_CBR_UPD_DistQPAdjTabB_U8_Xe_Lpm_Plus_Base[81] =
{
     0,  0,  0,  0, 0, 2, 3, 3, 4,
     0,  0,  0,  0, 0, 2, 3, 3, 4,
    -1,  0,  0,  0, 0, 2, 2, 3, 3,
    -1, -1,  0,  0, 0, 1, 2, 2, 2,
    -1, -1, -1,  0, 0, 0, 1, 2, 2,
    -2, -1, -1,  0, 0, 0, 1, 1, 2,
    -2, -1, -1, -1, 0, 0, 1, 1, 2,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
};

const int8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_VBR_UPD_DistQPAdjTabI_U8_Xe_Lpm_Plus_Base[81] =
{
     0,  0,  0,  0, 0, 2, 3, 3, 4,
     0,  0,  0,  0, 0, 2, 3, 3, 4,
    -1,  0,  0,  0, 0, 2, 2, 3, 3,
    -1, -1,  0,  0, 0, 1, 2, 2, 2,
    -1, -1, -1,  0, 0, 0, 1, 2, 2,
    -2, -1, -1,  0, 0, 0, 1, 1, 2,
    -2, -1, -1, -1, 0, 0, 1, 1, 2,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
};

const int8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_VBR_UPD_DistQPAdjTabP_U8_Xe_Lpm_Plus_Base[81] =
{
     0,  0,  0,  0, 0, 2, 3, 3, 4,
     0,  0,  0,  0, 0, 2, 3, 3, 4,
    -1,  0,  0,  0, 0, 2, 2, 3, 3,
    -1, -1,  0,  0, 0, 1, 2, 2, 2,
    -1, -1, -1,  0, 0, 0, 1, 2, 2,
    -2, -1, -1,  0, 0, 0, 1, 1, 2,
    -2, -1, -1, -1, 0, 0, 1, 1, 2,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
};

const int8_t AvcVdencBrcConstSettingsXe_Lpm_Plus_Base::m_VBR_UPD_DistQPAdjTabB_U8_Xe_Lpm_Plus_Base[81] =
{
     0,  0,  0,  0, 0, 2, 3, 3, 4,
     0,  0,  0,  0, 0, 2, 3, 3, 4,
    -1,  0,  0,  0, 0, 2, 2, 3, 3,
    -1, -1,  0,  0, 0, 1, 2, 2, 2,
    -1, -1, -1,  0, 0, 0, 1, 2, 2,
    -2, -1, -1,  0, 0, 0, 1, 1, 2,
    -2, -1, -1, -1, 0, 0, 1, 1, 2,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
    -2, -2, -1, -1, 0, 0, 0, 1, 1,
};


MOS_STATUS EncodeAvcVdencConstSettingsXe_Lpm_Plus_Base::SetVdencAvcImgStateSettings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<AvcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

#if !(_MEDIA_RESERVED)
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

                    static const uint32_t dw1Lut = 0x301;
                    data[1] |= dw1Lut;

                    static const uint32_t dw2Lut[3][8][2] = { { { 0x70028000, 0x70028000,}, { 0x70029400, 0x70029400,}, { 0x70029400, 0x70029400,}, { 0x70028240, 0x70028240,}, { 0x70028240, 0x70028240,}, { 0x70028240, 0x70028240,}, { 0x70028160, 0x70028160,}, { 0x70028160, 0x70028160,},}, { { 0x70028000, 0x70028000,}, { 0x70029407, 0x70029407,}, { 0x70029407, 0x70029407,}, { 0x70028247, 0x70028247,}, { 0x70028247, 0x70028247,}, { 0x70028247, 0x70028247,}, { 0x70028164, 0x70028164,}, { 0x70028164, 0x70028164,},}, { { 0x70028000, 0x72028000,}, { 0x70029407, 0x72029207,}, { 0x70029407, 0x72029207,}, { 0x70028244, 0x72028244,}, { 0x70028243, 0x72028243,}, { 0x70028243, 0x72028243,}, { 0x70028162, 0x72028162,}, { 0x70028162, 0x72028162,},},};
                    data[2] |= dw2Lut[CodingTypeMinus1][tu][RefPicFlag];

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

                    static const uint32_t dw13Lut[3] = { 0x7d00000, 0x6400000, 0x4b00000,};
                    data[13] |= dw13Lut[CodingTypeMinus1];

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

                    return MOS_STATUS_SUCCESS;

                });

            return MOS_STATUS_SUCCESS;
        });
#else
#define VDENC_AVC_IMGSTATE_SETTINGS_EXT
    #include "encode_avc_vdenc_const_settings_xe_lpm_plus_base_ext.h"
#undef VDENC_AVC_IMGSTATE_SETTINGS_EXT
#endif  // !(_MEDIA_RESERVED)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAvcVdencConstSettingsXe_Lpm_Plus_Base::SetBrcSettings()
{
    ENCODE_FUNC_CALL();
    EncodeAvcVdencConstSettings::SetBrcSettings();

    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<AvcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->brcSettings.BRC_UPD_global_rate_ratio_threshold       = (uint8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_BRC_UPD_global_rate_ratio_threshold_Xe_Lpm_Plus_Base;
    setting->brcSettings.BRC_UPD_slwin_global_rate_ratio_threshold = (uint8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_BRC_UPD_slwin_global_rate_ratio_threshold_Xe_Lpm_Plus_Base;
    setting->brcSettings.BRC_UPD_global_rate_ratio_threshold_qp    = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_BRC_UPD_global_rate_ratio_threshold_qp_Xe_Lpm_Plus_Base;


    setting->brcSettings.BRC_UPD_GlobalRateQPAdjTabI_U8      = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_BRC_UPD_GlobalRateQPAdjTabI_U8_Xe_Lpm_Plus_Base;
    setting->brcSettings.BRC_UPD_GlobalRateQPAdjTabP_U8      = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_BRC_UPD_GlobalRateQPAdjTabP_U8_Xe_Lpm_Plus_Base;
    setting->brcSettings.BRC_UPD_SlWinGlobalRateQPAdjTabP_U8 = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_BRC_UPD_SlWinGlobalRateQPAdjTabP_U8_Xe_Lpm_Plus_Base;
    setting->brcSettings.BRC_UPD_GlobalRateQPAdjTabB_U8      = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_BRC_UPD_GlobalRateQPAdjTabB_U8_Xe_Lpm_Plus_Base;

    setting->brcSettings.BRC_UPD_DistThreshldI_U8            = (uint8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_BRC_UPD_DistThreshldI_U8_Xe_Lpm_Plus_Base;
    setting->brcSettings.BRC_UPD_DistThreshldP_U8            = (uint8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_BRC_UPD_DistThreshldP_U8_Xe_Lpm_Plus_Base;
    setting->brcSettings.BRC_UPD_DistThreshldB_U8            = (uint8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_BRC_UPD_DistThreshldB_U8_Xe_Lpm_Plus_Base;

    setting->brcSettings.CBR_UPD_DistQPAdjTabI_U8            = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_CBR_UPD_DistQPAdjTabI_U8_Xe_Lpm_Plus_Base;
    setting->brcSettings.CBR_UPD_DistQPAdjTabP_U8            = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_CBR_UPD_DistQPAdjTabP_U8_Xe_Lpm_Plus_Base;
    setting->brcSettings.CBR_UPD_DistQPAdjTabB_U8            = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_CBR_UPD_DistQPAdjTabB_U8_Xe_Lpm_Plus_Base;
    setting->brcSettings.VBR_UPD_DistQPAdjTabI_U8            = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_VBR_UPD_DistQPAdjTabI_U8_Xe_Lpm_Plus_Base;
    setting->brcSettings.VBR_UPD_DistQPAdjTabP_U8            = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_VBR_UPD_DistQPAdjTabP_U8_Xe_Lpm_Plus_Base;
    setting->brcSettings.VBR_UPD_DistQPAdjTabB_U8            = (int8_t*)m_brcSettings_Xe_Lpm_Plus_Base.m_VBR_UPD_DistQPAdjTabB_U8_Xe_Lpm_Plus_Base;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
