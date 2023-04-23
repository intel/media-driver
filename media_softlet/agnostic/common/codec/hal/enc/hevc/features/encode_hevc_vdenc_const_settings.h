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
//! \file     encode_hevc_vdenc_const_settings.h
//! \brief    Defines the common interface for henvc vdenc const settings
//! \details  The default setting is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_HEVC_VDENC_CONST_SETTINGS_H__
#define __ENCODE_HEVC_VDENC_CONST_SETTINGS_H__

#include "codec_def_common_encode.h"
#include "codec_def_encode_hevc.h"
#include "encode_const_settings.h"
#include "media_class_trace.h"
#include "media_feature_const_settings.h"
#include "mos_defs.h"
#include <stdint.h>
#include <array>
#if _ENCODE_RESERVED
#include "encode_hevc_vdenc_const_settings_ext.h"
#endif // _ENCODE_RESERVED

#define HUC_MODE_COST_NUM   7
#define HUC_QP_RANGE        52

namespace encode
{
struct HevcVdencBrcSettings
{
    ConstTableSet HevcVdencBrcSettings_0;
    ConstTableSet HevcVdencBrcSettings_1;
    ConstTableSet estRateThreshP0;
    ConstTableSet estRateThreshB0;
    ConstTableSet estRateThreshI0;
    ConstTableSet instRateThreshP0;
    ConstTableSet instRateThreshB0;
    ConstTableSet instRateThreshI0;

    ConstTableSet devThreshIFPNEG;
    ConstTableSet devThreshIFPPOS;
    ConstTableSet devThreshPBFPNEG;
    ConstTableSet devThreshPBFPPOS;
    ConstTableSet devThreshVBRNEG;
    ConstTableSet devThreshVBRPOS;
    ConstTableSet lowdelayDevThreshPB;
    ConstTableSet lowdelayDevThreshVBR;
    ConstTableSet lowdelayDevThreshI;

    ConstTableSet startGAdjFrame;

    uint32_t    numDevThreshlds = 0;
    double      devStdFPS       = 0;
    double      bpsRatioLow     = 0;
    double      bpsRatioHigh    = 0;
    int32_t     postMultPB      = 0;
    int32_t     negMultPB       = 0;
    int32_t     posMultVBR      = 0;
    int32_t     negMultVBR      = 0;

    uint8_t topFrmSzThrForAdapt2Pass_U8 = 0;
    uint8_t botFrmSzThrForAdapt2Pass_U8 = 0;
    uint8_t topQPDeltaThrForAdapt2Pass_U8 = 0;
    uint8_t botQPDeltaThrForAdapt2Pass_U8 = 0;

    int8_t (*HevcVdencBrcSettings_4)[9][8] = nullptr;
    int8_t (*HevcVdencBrcSettings_5)[9][8] = nullptr;
    int8_t (*HevcVdencBrcSettings_6)[9][8] = nullptr;

    ConstTableSet hucConstantData;
    ConstTableSet HevcVdencBrcSettings_2;
    ConstTableSet HevcVdencBrcSettings_3;
    ConstTableSet HevcVdencBrcSettings_7;
    ConstTableSet HevcVdencBrcSettings_8;
    ConstTableSet rateRatioThreshold;
    ConstTableSet startGAdjMult;
    ConstTableSet startGAdjDiv;
    ConstTableSet rateRatioThresholdQP;

    int8_t deltaQPForSadZone0_S8 = 0;
    int8_t deltaQPForSadZone1_S8 = 0;
    int8_t deltaQPForSadZone2_S8 = 0;
    int8_t deltaQPForSadZone3_S8 = 0;
    int8_t deltaQPForMvZero_S8   = 0;
    int8_t deltaQPForMvZone0_S8  = 0;
    int8_t deltaQPForMvZone1_S8  = 0;
    int8_t deltaQPForMvZone2_S8  = 0;

    int8_t  reEncodePositiveQPDeltaThr_S8 = 0;
    int8_t  reEncodeNegativeQPDeltaThr_S8 = 0;
    uint8_t sceneChgPrevIntraPctThreshold_U8 = 0;
    uint8_t sceneChgCurIntraPctThreshold_U8  = 0;
};

struct HevcVdencArbSettings  // adaptive region boot settings
{
    const uint8_t m_roiCtrl   = 85;  // All four 16x16 blocks within the 32x32 blocks share the same region ID 1 (01010101)
    const uint8_t m_maxCuSize = 2;   // For ARB, currently supports 32x32 block
    const std::array<
        uint16_t,
        8>
        m_rowOffsetsForBoost = {{0, 3, 5, 2, 7, 4, 1, 6}};
};

struct HevcVdencFeatureSettings : VdencFeatureSettings
{
    std::vector<
        std::function<
            MOS_STATUS(mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD1) & par, bool isLowDelay)> >
        vdencLaCmd1Settings;

    std::vector<
        std::function<
            MOS_STATUS(mhw::vdbox::vdenc::_MHW_PAR_T(VDENC_CMD2) & par, bool isLowDelay)> >
        vdencLaCmd2Settings;

    std::array<bool, NUM_TARGET_USAGE_MODES + 1> rdoqEnable{};
    std::array<bool, NUM_TARGET_USAGE_MODES + 1> acqpEnable{};
    std::array<bool, NUM_TARGET_USAGE_MODES + 1> rdoqLaEnable{};
    std::array<bool, NUM_TARGET_USAGE_MODES + 1> acqpLaEnable{};

    std::array<
        std::array<
            std::array<
                std::array<
                    std::array<uint8_t,
                        2>,
                    2>,
                2>,
            2>,
        4>
        transformSkipCoeffsTable{};

    std::array<uint16_t, 52> transformSkipLambdaTable{};

    std::array<
        std::array<
            std::array<
                std::array<uint16_t,
                    52>,
                2>,
            2>,
        2>
        rdoqLamdas8bits{};

    std::array<
        std::array<
            std::array<
                std::array<uint16_t,
                    64>,
                2>,
            2>,
        2>
        rdoqLamdas10bits{};

    std::array<
        std::array<
            std::array<
                std::array<uint16_t,
                    76>,
                2>,
            2>,
        2>
        rdoqLamdas12bits{};

    HevcVdencBrcSettings brcSettings = {};
    HevcVdencArbSettings arbSettings = {};
};

struct HevcVdencBrcConstSettings
{
    static const uint16_t       HevcVdencBrcConstSettings_0[52];
    static const uint16_t       HevcVdencBrcConstSettings_1[52];
    static const uint8_t        m_estRateThreshP0[7];
    static const uint8_t        m_estRateThreshB0[7];
    static const uint8_t        m_estRateThreshI0[7];
    static const int8_t         m_instRateThreshP0[4];
    static const int8_t         m_instRateThreshB0[4];
    static const int8_t         m_instRateThreshI0[4];

    static constexpr uint32_t   m_numDevThreshlds = 8;
    static constexpr double     m_devStdFPS = 30.0;
    static constexpr double     m_bpsRatioLow = 0.1;
    static constexpr double     m_bpsRatioHigh = 3.5;
    static constexpr int32_t    m_postMultPB = 50;
    static constexpr int32_t    m_negMultPB = -50;
    static constexpr int32_t    m_posMultVBR = 100;
    static constexpr int32_t    m_negMultVBR = -50;

    static const double         m_devThreshIFPNEG[m_numDevThreshlds / 2];
    static const double         m_devThreshIFPPOS[m_numDevThreshlds / 2];
    static const double         m_devThreshPBFPNEG[m_numDevThreshlds / 2];
    static const double         m_devThreshPBFPPOS[m_numDevThreshlds / 2];
    static const double         m_devThreshVBRNEG[m_numDevThreshlds / 2];
    static const double         m_devThreshVBRPOS[m_numDevThreshlds / 2];
    static const int8_t         m_lowdelayDevThreshPB[m_numDevThreshlds];
    static const int8_t         m_lowdelayDevThreshVBR[m_numDevThreshlds];
    static const int8_t         m_lowdelayDevThreshI[m_numDevThreshlds];

    static const uint16_t       m_startGAdjFrame[4];

    static const uint32_t       m_hucConstantData[];
    static const uint16_t       HevcVdencBrcConstSettings_2[52];
    static const uint16_t       HevcVdencBrcConstSettings_3[52];
    static const uint32_t       HevcVdencBrcConstSettings_7[364];
    static const uint32_t       HevcVdencBrcConstSettings_8[364];
    static const int8_t         HevcVdencBrcConstSettings_4[9][8];
    static const int8_t         HevcVdencBrcConstSettings_5[9][8];
    static const int8_t         HevcVdencBrcConstSettings_6[9][8];
    static const uint8_t        m_rateRatioThreshold[7];
    static const uint8_t        m_startGAdjMult[5];
    static const uint8_t        m_startGAdjDiv[5];
    static const uint8_t        m_rateRatioThresholdQP[8];

    static constexpr uint8_t    m_topFrmSzThrForAdapt2Pass_U8 = 32;
    static constexpr uint8_t    m_botFrmSzThrForAdapt2Pass_U8 = 24;
    static constexpr uint8_t    m_topQPDeltaThrForAdapt2Pass_U8 = 2;
    static constexpr uint8_t    m_botQPDeltaThrForAdapt2Pass_U8 = 1;

    static const int8_t m_deltaQPForSadZone0_S8 = -1;
    static const int8_t m_deltaQPForSadZone1_S8 = 0;
    static const int8_t m_deltaQPForSadZone2_S8 = 1;
    static const int8_t m_deltaQPForSadZone3_S8 = 2;
    static const int8_t m_deltaQPForMvZero_S8 = 3;
    static const int8_t m_deltaQPForMvZone0_S8 = -2;
    static const int8_t m_deltaQPForMvZone1_S8 = 0;
    static const int8_t m_deltaQPForMvZone2_S8 = 2;

    static const int8_t m_reEncodePositiveQPDeltaThr_S8 = 4;
    static const int8_t m_reEncodeNegativeQPDeltaThr_S8 = -5;
    static const uint8_t m_sceneChgPrevIntraPctThreshold_U8 = 96;
    static const uint8_t m_sceneChgCurIntraPctThreshold_U8 = 192;
};

class EncodeHevcVdencConstSettings : public VdencConstSettings
{
public:

    //!
    //! \brief  EncodeHevcVdencConstSettings constructor
    //!
    EncodeHevcVdencConstSettings();

    //!
    //! \brief  EncodeHevcVdencConstSettings destructor
    //!
    ~EncodeHevcVdencConstSettings();

    MOS_STATUS PrepareConstSettings() override;

    MOS_STATUS Update(void *params) override;

protected:
    MOS_STATUS SetTUSettings() override;

    MOS_STATUS SetCommonSettings() override;

    MOS_STATUS SetVdencStreaminStateSettings() override;

    MOS_STATUS SetVdencCmd1Settings() override;

    MOS_STATUS SetVdencCmd2Settings() override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetLaTUSettings();
    MOS_STATUS SetVdencLaCmd1Settings();
    MOS_STATUS SetVdencLaCmd2Settings();

    MOS_STATUS SetBrcSettings() override;

    HevcVdencBrcConstSettings m_brcSettings;

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS m_hevcSeqParams   = nullptr;
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  m_hevcPicParams   = nullptr;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    m_hevcSliceParams = nullptr;

    bool m_hevcVdencRoundingPrecisionEnabled = true;  //!<  Roinding Precision enabled
    bool m_hevcRdoqEnabled                   = false;
    bool m_isLaSetting                       = false;

MEDIA_CLASS_DEFINE_END(encode__EncodeHevcVdencConstSettings)
};

}
#endif // !__ENCODE_HEVC_VDENC_CONST_SETTINGS_H__
