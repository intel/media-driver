/*
 * Copyright (c) 2021, Intel Corporation
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
//! \file     encode_avc_vdenc_const_settings_xe2_lpm.cpp
//! \brief    Defines the common interface for Xe2_LPM avc vdenc const settings
//!

#include "encode_avc_vdenc_const_settings_xe2_lpm.h"
#include "encode_utils.h"

namespace encode
{

const uint8_t  AvcVdencBrcConstSettingsXe2_Lpm::m_BRC_UPD_global_rate_ratio_threshold_Xe2_Lpm[7]       = {40, 75, 97, 103, 125, 160, 0};
const uint8_t  AvcVdencBrcConstSettingsXe2_Lpm::m_BRC_UPD_slwin_global_rate_ratio_threshold_Xe2_Lpm[7] = {40, 75, 97, 103, 125, 160, 0};
const int8_t   AvcVdencBrcConstSettingsXe2_Lpm::m_BRC_UPD_global_rate_ratio_threshold_qp_Xe2_Lpm[8]    = {-3, -2, -1, 0, 1, 2, 3, 0};

const int8_t AvcVdencBrcConstSettingsXe2_Lpm::m_BRC_UPD_GlobalRateQPAdjTabI_U8_Xe2_Lpm[64] =
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

const int8_t AvcVdencBrcConstSettingsXe2_Lpm::m_BRC_UPD_GlobalRateQPAdjTabP_U8_Xe2_Lpm[64] =
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
const int8_t AvcVdencBrcConstSettingsXe2_Lpm::m_BRC_UPD_SlWinGlobalRateQPAdjTabP_U8_Xe2_Lpm[64] =
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

const int8_t AvcVdencBrcConstSettingsXe2_Lpm::m_BRC_UPD_GlobalRateQPAdjTabB_U8_Xe2_Lpm[64] =
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

const uint8_t AvcVdencBrcConstSettingsXe2_Lpm::m_BRC_UPD_DistThreshldI_U8_Xe2_Lpm[10] = {4, 30, 60, 80, 120, 140, 200, 255, 0, 0};
const uint8_t AvcVdencBrcConstSettingsXe2_Lpm::m_BRC_UPD_DistThreshldP_U8_Xe2_Lpm[10] = {4, 30, 60, 80, 120, 140, 200, 255, 0, 0};
const uint8_t AvcVdencBrcConstSettingsXe2_Lpm::m_BRC_UPD_DistThreshldB_U8_Xe2_Lpm[10] = {2, 20, 40, 70, 130, 160, 200, 255, 0, 0};

const int8_t AvcVdencBrcConstSettingsXe2_Lpm::m_CBR_UPD_DistQPAdjTabI_U8_Xe2_Lpm[81] =
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

const int8_t AvcVdencBrcConstSettingsXe2_Lpm::m_CBR_UPD_DistQPAdjTabP_U8_Xe2_Lpm[81] =
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

const int8_t AvcVdencBrcConstSettingsXe2_Lpm::m_CBR_UPD_DistQPAdjTabB_U8_Xe2_Lpm[81] =
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

const int8_t AvcVdencBrcConstSettingsXe2_Lpm::m_VBR_UPD_DistQPAdjTabI_U8_Xe2_Lpm[81] =
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

const int8_t AvcVdencBrcConstSettingsXe2_Lpm::m_VBR_UPD_DistQPAdjTabP_U8_Xe2_Lpm[81] =
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

const int8_t AvcVdencBrcConstSettingsXe2_Lpm::m_VBR_UPD_DistQPAdjTabB_U8_Xe2_Lpm[81] =
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


MOS_STATUS EncodeAvcVdencConstSettingsXe2_Lpm::SetVdencAvcImgStateSettings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<AvcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

#if _MEDIA_RESERVED
#define VDENC_AVC_IMGSTATE_SETTINGS_EXT
#include "encode_avc_vdenc_const_settings_xe2_lpm_ext.h"
#undef VDENC_AVC_IMGSTATE_SETTINGS_EXT
#else
#define VDENC_AVCIMGSTATE_SETTINGS_OPEN
#include "encode_avc_vdenc_const_settings_xe2_lpm_open.h"
#undef VDENC_AVCIMGSTATE_SETTINGS_OPEN
#endif  // !(_MEDIA_RESERVED)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAvcVdencConstSettingsXe2_Lpm::SetBrcSettings()
{
    ENCODE_FUNC_CALL();
    EncodeAvcVdencConstSettings::SetBrcSettings();

    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<AvcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->brcSettings.BRC_UPD_global_rate_ratio_threshold       = (uint8_t *)m_brcSettings_Xe2_Lpm.m_BRC_UPD_global_rate_ratio_threshold_Xe2_Lpm;
    setting->brcSettings.BRC_UPD_slwin_global_rate_ratio_threshold = (uint8_t *)m_brcSettings_Xe2_Lpm.m_BRC_UPD_slwin_global_rate_ratio_threshold_Xe2_Lpm;
    setting->brcSettings.BRC_UPD_global_rate_ratio_threshold_qp    = (int8_t *)m_brcSettings_Xe2_Lpm.m_BRC_UPD_global_rate_ratio_threshold_qp_Xe2_Lpm;

    setting->brcSettings.BRC_UPD_GlobalRateQPAdjTabI_U8      = (int8_t *)m_brcSettings_Xe2_Lpm.m_BRC_UPD_GlobalRateQPAdjTabI_U8_Xe2_Lpm;
    setting->brcSettings.BRC_UPD_GlobalRateQPAdjTabP_U8      = (int8_t *)m_brcSettings_Xe2_Lpm.m_BRC_UPD_GlobalRateQPAdjTabP_U8_Xe2_Lpm;
    setting->brcSettings.BRC_UPD_SlWinGlobalRateQPAdjTabP_U8 = (int8_t *)m_brcSettings_Xe2_Lpm.m_BRC_UPD_SlWinGlobalRateQPAdjTabP_U8_Xe2_Lpm;
    setting->brcSettings.BRC_UPD_GlobalRateQPAdjTabB_U8      = (int8_t *)m_brcSettings_Xe2_Lpm.m_BRC_UPD_GlobalRateQPAdjTabB_U8_Xe2_Lpm;

    setting->brcSettings.BRC_UPD_DistThreshldI_U8 = (uint8_t *)m_brcSettings_Xe2_Lpm.m_BRC_UPD_DistThreshldI_U8_Xe2_Lpm;
    setting->brcSettings.BRC_UPD_DistThreshldP_U8 = (uint8_t *)m_brcSettings_Xe2_Lpm.m_BRC_UPD_DistThreshldP_U8_Xe2_Lpm;
    setting->brcSettings.BRC_UPD_DistThreshldB_U8 = (uint8_t *)m_brcSettings_Xe2_Lpm.m_BRC_UPD_DistThreshldB_U8_Xe2_Lpm;

    setting->brcSettings.CBR_UPD_DistQPAdjTabI_U8 = (int8_t *)m_brcSettings_Xe2_Lpm.m_CBR_UPD_DistQPAdjTabI_U8_Xe2_Lpm;
    setting->brcSettings.CBR_UPD_DistQPAdjTabP_U8 = (int8_t *)m_brcSettings_Xe2_Lpm.m_CBR_UPD_DistQPAdjTabP_U8_Xe2_Lpm;
    setting->brcSettings.CBR_UPD_DistQPAdjTabB_U8 = (int8_t *)m_brcSettings_Xe2_Lpm.m_CBR_UPD_DistQPAdjTabB_U8_Xe2_Lpm;
    setting->brcSettings.VBR_UPD_DistQPAdjTabI_U8 = (int8_t *)m_brcSettings_Xe2_Lpm.m_VBR_UPD_DistQPAdjTabI_U8_Xe2_Lpm;
    setting->brcSettings.VBR_UPD_DistQPAdjTabP_U8 = (int8_t *)m_brcSettings_Xe2_Lpm.m_VBR_UPD_DistQPAdjTabP_U8_Xe2_Lpm;
    setting->brcSettings.VBR_UPD_DistQPAdjTabB_U8 = (int8_t *)m_brcSettings_Xe2_Lpm.m_VBR_UPD_DistQPAdjTabB_U8_Xe2_Lpm;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
