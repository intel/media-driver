/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_av1_vdenc_const_settings_xe3p_lpm_base.cpp
//! \brief    Defines the common interface for av1 vdenc const settings
//! \details  The encode feature manager is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_av1_vdenc_const_settings_xe3p_lpm_base.h"
#include "encode_utils.h"

namespace encode
{


const double mergeScal              = 0.75;
const double skipScal               = 0.30;
const uint32_t numExtendedCandCosts = 24;

MOS_STATUS EncodeAv1VdencConstSettingsXe3P_Lpm_Base::SetVdencCmd1Settings()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Av1VdencFeatureSettings*>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencCmd1Settings = {

    VDENC_CMD1_LAMBDA()
    {
        return MOS_STATUS_SUCCESS;
    }

};
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencConstSettingsXe3P_Lpm_Base::SetVdencCmd2Settings()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Av1VdencFeatureSettings*>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencCmd2Settings = {
        VDENC_CMD2_LAMBDA()
        {
            return MOS_STATUS_SUCCESS;
        }
    };

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencConstSettingsXe3P_Lpm_Base::SetVdencStreaminStateSettings()
{
    ENCODE_FUNC_CALL();

    auto setting = static_cast<Av1VdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencStreaminStateSettings.emplace_back(
        VDENC_STREAMIN_STATE_LAMBDA() {
            static const std::array<
                std::array<
                    uint8_t,
                    NUM_TARGET_USAGE_MODES + 1>,
                4>
                numMergeCandidates = {{
                    {4, 4, 4, 2, 2, 2, 0, 0},
                    {4, 4, 4, 2, 2, 2, 2, 1},
                    {5, 5, 5, 2, 2, 2, 2, 2},
                    {5, 5, 5, 2, 2, 2, 2, 2},
                }};

            static const std::array<
                uint8_t,
                NUM_TARGET_USAGE_MODES + 1>
                numImePredictors =  {8, 8, 8, 6, 6, 6, 4, 3};

            par.maxTuSize                = 3;  //Maximum TU Size allowed, restriction to be set to 3
            par.maxCuSize                = (cu64Align) ? 3 : 2;
            par.numMergeCandidateCu64x64 = numMergeCandidates[3][m_av1SeqParams->TargetUsage];
            par.numMergeCandidateCu32x32 = numMergeCandidates[2][m_av1SeqParams->TargetUsage];
            par.numMergeCandidateCu16x16 = numMergeCandidates[1][m_av1SeqParams->TargetUsage];
            par.numMergeCandidateCu8x8   = numMergeCandidates[0][m_av1SeqParams->TargetUsage];
            par.numImePredictors         = numImePredictors[m_av1SeqParams->TargetUsage];

            return MOS_STATUS_SUCCESS;
        });

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
