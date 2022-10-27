/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     encode_vp9_vdenc_feature_manager_xe_lpm_plus.cpp
//! \brief    Defines the common interface for vp9 vdenc feature manager
//!

#include "encode_vp9_vdenc_feature_manager_xe_lpm_plus.h"

namespace encode
{

MOS_STATUS EncodeVp9VdencFeatureManagerXe_Lpm_Plus::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeVp9VdencConstSettingsXe_Lpm_Plus);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeVp9VdencFeatureManagerXe_Lpm_Plus::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    auto setting = static_cast<EncodeVp9VdencConstSettingsXe_Lpm_Plus *>(m_featureConstSettings);
    ENCODE_CHK_NULL_RETURN(setting);
    setting->SetOsInterface(m_hwInterface->GetOsInterface());

    ENCODE_CHK_STATUS_RETURN(EncodeVp9VdencFeatureManager::CreateFeatures(constSettings));

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
