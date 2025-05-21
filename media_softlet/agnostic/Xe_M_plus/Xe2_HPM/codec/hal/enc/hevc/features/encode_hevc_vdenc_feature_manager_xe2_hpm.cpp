/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     encode_hevc_vdenc_feature_manager_xe2_hpm.cpp
//! \brief    Defines the common interface for hevc xe2 hpm encode feature manager
//!

#include "encode_hevc_vdenc_feature_manager_xe2_hpm.h"
#include "encode_hevc_vdenc_scc_xe2_hpm.h"
#include "encode_hevc_aqm.h"
#include "encode_hevc_vdenc_fastpass_xe2_hpm_base.h"

namespace encode
{

MOS_STATUS EncodeHevcVdencFeatureManagerXe2_Hpm::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeHevcVdencConstSettingsXe2_Hpm);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManagerXe2_Hpm::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodeHevcVdencFeatureManagerXe_Lpm_Plus_Base::CreateFeatures(constSettings));
    HevcVdencSccXe2_Hpm *hevcScc = MOS_New(HevcVdencSccXe2_Hpm, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencSccFeature, hevcScc, {HevcPipeline::encodePreEncPacket}));
    HevcEncodeAqm *hevcAqm = MOS_New(HevcEncodeAqm, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcAqm, hevcAqm, {HevcPipeline::encodePreEncPacket}));
    HevcVdencFastPass_Xe2_Hpm_Base *hevcFastPass = MOS_New(HevcVdencFastPass_Xe2_Hpm_Base, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencFastPassFeature, hevcFastPass));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManagerXe2_Hpm::MapTargetUsage(uint8_t &targetUsage)
{
    ENCODE_FUNC_CALL();

    switch (targetUsage)
    {
    case 1:
        targetUsage = 1;
        break;
    case 2:
        targetUsage = 2;
        break;
    case 3:
    case 4:
    case 5:
        targetUsage = 4;
        break;
    case 6:
        targetUsage = 6;
        break;
    case 7:
        targetUsage = 7;
        break;
    default:
        targetUsage = 4;
        break;
    }
    return MOS_STATUS_SUCCESS;
}
}

