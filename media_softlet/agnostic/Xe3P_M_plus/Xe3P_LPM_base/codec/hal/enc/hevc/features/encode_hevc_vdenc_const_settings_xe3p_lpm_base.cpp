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
//! \file     encode_hevc_vdenc_const_settings_xe3p_lpm_base.cpp
//! \brief    Defines the common interface for Xe3P_LPM_Base hevc vdenc const settings
//!

#include "codec_def_common.h"
#include "encode_hevc_vdenc_const_settings_xe3p_lpm_base.h"
#include "encode_utils.h"
#include "mos_solo_generic.h"

namespace encode
{
MOS_STATUS EncodeHevcVdencConstSettingsXe3P_Lpm_Base::SetTUSettings()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_featureSetting);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    auto setting = static_cast<HevcVdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->rdoqEnable = {true, true, true, true, true, true, true, false};
    setting->acqpEnable = {true, true, true, true, true, true, true, false};

    return eStatus;
}

 MOS_STATUS EncodeHevcVdencConstSettingsXe3P_Lpm_Base::SetVdencStreaminStateSettings()
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
                    {3, 3, 3, 2, 2, 2, 1, 0},
                    {3, 3, 3, 2, 2, 2, 2, 2},
                    {4, 4, 3, 2, 2, 2, 2, 2},
                    {3, 3, 3, 2, 2, 2, 2, 2},
                }};

            static const std::array<
                uint8_t,
                NUM_TARGET_USAGE_MODES + 1>
                numImePredictors = {12, 12, 8, 6, 6, 6, 4, 3};

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

MOS_STATUS EncodeHevcVdencConstSettingsXe3P_Lpm_Base::SetVdencCmd1Settings()
{
    ENCODE_FUNC_CALL();
    
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
