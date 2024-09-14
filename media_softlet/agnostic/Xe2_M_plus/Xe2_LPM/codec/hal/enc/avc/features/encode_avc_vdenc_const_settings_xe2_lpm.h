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
//! \file     encode_avc_vdenc_const_settings_xe2_lpm.h
//! \brief    Defines the common interface for Xe2_LPM avc vdenc const settings
//!

#ifndef __ENCODE_AVC_VDENC_CONST_SETTINGS_XE2_LPM_H__
#define __ENCODE_AVC_VDENC_CONST_SETTINGS_XE2_LPM_H__

#include "encode_avc_vdenc_const_settings.h"

namespace encode
{
struct AvcVdencBrcConstSettingsXe2_Lpm
{
    static const uint8_t  m_BRC_UPD_global_rate_ratio_threshold_Xe2_Lpm[7];                  //!< Global Rate Ratio Threshold
    static const uint8_t  m_BRC_UPD_slwin_global_rate_ratio_threshold_Xe2_Lpm[7];            //!< Slide Window Global Rate Ratio Threshold
    static const int8_t   m_BRC_UPD_global_rate_ratio_threshold_qp_Xe2_Lpm[8];               //!< Global Rate Ratio QP Threshold

    static const int8_t   m_BRC_UPD_GlobalRateQPAdjTabI_U8_Xe2_Lpm[64];                      //!< I Picture Global Rate QP Adjustment Table.
    static const int8_t   m_BRC_UPD_GlobalRateQPAdjTabP_U8_Xe2_Lpm[64];                      //!< P Picture Global Rate QP Adjustment Table.
    static const int8_t   m_BRC_UPD_SlWinGlobalRateQPAdjTabP_U8_Xe2_Lpm[64];                 //!< P picture Global Rate QP Adjustment Table for Sliding Window BRC
    static const int8_t   m_BRC_UPD_GlobalRateQPAdjTabB_U8_Xe2_Lpm[64];                      //!< B Picture Global Rate QP Adjustment Table.

    static const uint8_t  m_BRC_UPD_DistThreshldI_U8_Xe2_Lpm[10];                            //!< I Picture Distortion THreshold.
    static const uint8_t  m_BRC_UPD_DistThreshldP_U8_Xe2_Lpm[10];                            //!< P Picture Distortion THreshold.
    static const uint8_t  m_BRC_UPD_DistThreshldB_U8_Xe2_Lpm[10];                            //!< P Picture Distortion THreshold.

    static const int8_t   m_CBR_UPD_DistQPAdjTabI_U8_Xe2_Lpm[81];                            //!< I Picture Distortion QP Adjustment Table under CBR Mode.
    static const int8_t   m_CBR_UPD_DistQPAdjTabP_U8_Xe2_Lpm[81];                            //!< P Picture Distortion QP Adjustment Table under CBR Mode.
    static const int8_t   m_CBR_UPD_DistQPAdjTabB_U8_Xe2_Lpm[81];                            //!< B Picture Distortion QP Adjustment Table under CBR Mode.
    static const int8_t   m_VBR_UPD_DistQPAdjTabI_U8_Xe2_Lpm[81];                            //!< I Picture Distortion QP Adjustment Table under VBR Mode.
    static const int8_t   m_VBR_UPD_DistQPAdjTabP_U8_Xe2_Lpm[81];                            //!< P Picture Distortion QP Adjustment Table under VBR Mode.
    static const int8_t   m_VBR_UPD_DistQPAdjTabB_U8_Xe2_Lpm[81];                            //!< B Picture Distortion QP Adjustment Table under VBR Mode.

};
class EncodeAvcVdencConstSettingsXe2_Lpm : public EncodeAvcVdencConstSettings
{
public:
    EncodeAvcVdencConstSettingsXe2_Lpm() = default;
    EncodeAvcVdencConstSettingsXe2_Lpm(PMOS_INTERFACE osInterface) :
        EncodeAvcVdencConstSettings (osInterface) {}
    virtual ~EncodeAvcVdencConstSettingsXe2_Lpm() {}

protected:
    MOS_STATUS SetVdencAvcImgStateSettings() override;
    virtual MOS_STATUS SetBrcSettings() override;

    AvcVdencBrcConstSettingsXe2_Lpm m_brcSettings_Xe2_Lpm;
MEDIA_CLASS_DEFINE_END(encode__EncodeAvcVdencConstSettingsXe2_Lpm)
};
}  // namespace encode
#endif  // !__ENCODE_AVC_VDENC_CONST_SETTINGS_XE2_LPM_H__
