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

}  // namespace encode
