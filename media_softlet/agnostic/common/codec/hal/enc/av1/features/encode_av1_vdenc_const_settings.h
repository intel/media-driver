/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     encode_av1_vdenc_const_settings.h
//! \brief    Defines the common interface for av1 vdenc const settings
//! \details  The default setting is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_AV1_VDENC_CONST_SETTINGS_H__
#define __ENCODE_AV1_VDENC_CONST_SETTINGS_H__

#include "codec_def_common_encode.h"
#include "codec_def_encode_av1.h"
#include "encode_const_settings.h"
#include "media_class_trace.h"
#include "media_feature_const_settings.h"
#include "mos_defs.h"
#include "mos_os.h"
#include "mos_os_specific.h"
#include <stdint.h>
#if _ENCODE_RESERVED
#include "mhw_vdbox_vdenc_cmdpar_ext.h"
#endif // _ENCODE_RESERVED

namespace encode
{
struct Av1VdencBrcSettings
{
    int32_t        numInstRateThresholds;
    ConstTableSet  instRateThresholdP;
    ConstTableSet  instRateThresholdI;

    double     devStdFPS;
    double     bpsRatioLow;
    double     bpsRatioHigh;
    int32_t    postMultPB;
    int32_t    negMultPB;
    int32_t    posMultVBR;
    int32_t    negMultVBR;

    uint32_t      numDevThreshlds;
    ConstTableSet devThresholdFpNegI;
    ConstTableSet devThresholdFpPosI;
    ConstTableSet devThresholdFpNegPB;
    ConstTableSet devThresholdFpPosPB;
    ConstTableSet devThresholdVbrNeg;
    ConstTableSet devThresholdVbrPos;

    uint32_t      numQpThresholds;
    ConstTableSet QPThresholds;
    uint32_t      numGlobalRateRatioThreshlds;
    ConstTableSet startGlobalAdjustFrame;
    ConstTableSet globalRateRatioThreshold;
    ConstTableSet globalRateRatioThresholdQP;
    uint32_t      numStartGlobalAdjusts;
    ConstTableSet startGlobalAdjustMult;
    ConstTableSet startGlobalAdjustDiv;
    uint32_t      numDistortionThresholds;
    ConstTableSet distortionThresh;
    ConstTableSet distortionThreshB;
    ConstTableSet maxFrameMultI;
    ConstTableSet maxFrameMultP;
    ConstTableSet av1DeltaQpI;
    ConstTableSet av1DeltaQpP;
    ConstTableSet av1DistortionsDeltaQpI;
    ConstTableSet av1DistortionsDeltaQpP;
    ConstTableSet loopFilterLevelTabLuma;
    ConstTableSet loopFilterLevelTabChroma;
    ConstTableSet hucModeCostsIFrame;
    ConstTableSet hucModeCostsPFrame;
};

struct Av1VdencFeatureSettings : VdencFeatureSettings
{
    Av1VdencBrcSettings brcSettings = {};
    double              av1Table[239] = {};
};

struct Av1VdencTUConstSettings
{
    static const uint8_t  vdencCmd2Par39[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par38[NUM_TARGET_USAGE_MODES];
    static const uint32_t vdencCmd2Par85Table0[NUM_TARGET_USAGE_MODES];
    static const uint32_t vdencCmd2Par85Table1[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par86[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par87Table0[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par87Table1[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par87Table2[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par87Table3[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table0[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table1[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table2[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table3[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table4[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table5[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table6[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table7[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table8[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table9[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table10[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table11[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par89[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par94[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par95[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par98[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par97[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par100[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par96[NUM_TARGET_USAGE_MODES];
    static const uint16_t vdencCmd2Par93[NUM_TARGET_USAGE_MODES];
    static const uint16_t vdencCmd2Par92[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par23[NUM_TARGET_USAGE_MODES];
};

struct Av1VdencBrcConstSettings
{
    //BRC INIT related const data
    static const int32_t        numInstRateThresholds = 4;
    static const int8_t         instRateThresholdP[numInstRateThresholds];
    static const int8_t         instRateThresholdI[numInstRateThresholds];

    static constexpr double     devStdFPS = 30.0;
    static constexpr double     bpsRatioLow = 0.1f;
    static constexpr double     bpsRatioHigh = 3.5;
    static constexpr int32_t    postMultPB = 50;
    static constexpr int32_t    negMultPB = -50;
    static constexpr int32_t    posMultVBR = 100;
    static constexpr int32_t    negMultVBR = -50;

    static constexpr uint32_t   numDevThreshlds = 8;
    static const double         devThresholdFpNegI[numDevThreshlds / 2];
    static const double         devThresholdFpPosI[numDevThreshlds / 2];
    static const double         devThresholdFpNegPB[numDevThreshlds / 2];
    static const double         devThresholdFpPosPB[numDevThreshlds / 2];
    static const double         devThresholdVbrNeg[numDevThreshlds / 2];
    static const double         devThresholdVbrPos[numDevThreshlds / 2];

    // BRC UPDATE related const data
    static constexpr uint32_t   numQpThresholds = 4;
    static const uint8_t        QPThresholds[numQpThresholds];
    static constexpr uint32_t   numGlobalRateRatioThreshlds = 6;
    static const uint16_t       startGlobalAdjustFrame[4];
    static const uint8_t        globalRateRatioThreshold[numGlobalRateRatioThreshlds];
    static const int8_t         globalRateRatioThresholdQP[numGlobalRateRatioThreshlds + 1];
    static constexpr uint32_t   numStartGlobalAdjusts = 4;
    static const uint8_t        startGlobalAdjustMult[numStartGlobalAdjusts + 1];
    static const uint8_t        startGlobalAdjustDiv[numStartGlobalAdjusts + 1];
    static constexpr uint32_t   numDistortionThresholds = 8;
    static const uint8_t        distortionThresh[numDistortionThresholds + 1];
    static const uint8_t        distortionThreshB[numDistortionThresholds + 1];
    static const uint8_t        maxFrameMultI[numQpThresholds + 1];
    static const uint8_t        maxFrameMultP[numQpThresholds + 1];
    static const int8_t         av1DeltaQpI[][numQpThresholds + 1];
    static const int8_t         av1DeltaQpP[][numQpThresholds + 1];
    static const int8_t         av1DistortionsDeltaQpI[][numDistortionThresholds + 1];
    static const int8_t         av1DistortionsDeltaQpP[][numDistortionThresholds + 1];
    static const uint8_t        loopFilterLevelTabLuma[256];
    static const uint8_t        loopFilterLevelTabChroma[256];
    static const uint32_t       hucModeCostsIFrame[52*6];
    static const uint32_t       hucModeCostsPFrame[52*6];
};

class EncodeAv1VdencConstSettings : public VdencConstSettings
{
public:

    //!
    //! \brief  EncodeAv1VdencConstSettings constructor
    //!
    EncodeAv1VdencConstSettings(PMOS_INTERFACE osInterface);
    
    //!
    //! \brief  EncodeAv1VdencConstSettings deconstructor
    //!
    virtual ~EncodeAv1VdencConstSettings() {}

    //!
    //! \brief  Prepare const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareConstSettings() override;

    MOS_STATUS Update(void* params) override;

    virtual MOS_STATUS SetVdencCmd1Settings() override;

    virtual MOS_STATUS SetVdencCmd2Settings() override { return MOS_STATUS_SUCCESS; }

protected:

    //!
    //! \brief  Prepare BRC related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetBrcSettings() override;

    //!
    //! \brief  Prepare table related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetTable() {return MOS_STATUS_SUCCESS;};

    //!
    //! \brief  Prepare common settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCommonSettings() override;

protected:
    PMOS_INTERFACE m_osInterface = nullptr;
    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS m_av1SeqParams = nullptr;
    PCODEC_AV1_ENCODE_PICTURE_PARAMS  m_av1PicParams = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodeAv1VdencConstSettings)
};


}
#endif // !__ENCODE_AV1_VDENC_CONST_SETTINGS_H__
