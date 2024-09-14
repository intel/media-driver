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
//! \file     encode_avc_vdenc_feature_manager_xe2_lpm.cpp
//! \brief    Defines the common interface for avc Xe2_LPM vdenc feature manager
//!

#include "encode_avc_vdenc_feature_manager_xe2_lpm.h"
#include "encode_avc_basic_feature_xe2_lpm.h"
#include "encode_avc_vdenc_const_settings_xe2_lpm.h"
#include "encode_avc_vdenc_stream_in_feature.h"
#include "encode_avc_vdenc_cqp_roi_feature.h"
#include "encode_avc_vdenc_brc_roi_feature.h"
#include "encode_avc_vdenc_weighted_prediction.h"
#include "encode_avc_brc.h"
#include "encode_avc_trellis.h"
#include "encode_avc_rounding.h"
#include "media_avc_feature_defs.h"
#include "encode_avc_vdenc_fullenc.h"
#include "encode_avc_aqm.h"
#include "encode_avc_vdenc_preenc.h"
#include "encode_avc_vdenc_pipeline.h"

namespace encode
{

MOS_STATUS EncodeAvcVdencFeatureManagerXe2_Lpm::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeAvcVdencConstSettingsXe2_Lpm, m_hwInterface->GetOsInterface());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAvcVdencFeatureManagerXe2_Lpm::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    auto setting = static_cast<EncodeAvcVdencConstSettings *>(m_featureConstSettings);
    ENCODE_CHK_NULL_RETURN(setting);
    setting->SetOsInterface(m_hwInterface->GetOsInterface());

    EncodeBasicFeature *encBasic = MOS_New(AvcBasicFeatureXe2_Lpm, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, m_mediaCopyWrapper, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(AvcFeatureIDs::basicFeature, encBasic, {AvcVdencPipeline::encodePreEncPacket}));

    AvcVdencStreamInFeature *streamInFeature = MOS_New(AvcVdencStreamInFeature, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(AvcFeatureIDs::avcVdencStreamInFeature, streamInFeature, {AvcVdencPipeline::encodePreEncPacket}));

    AvcEncodeTrellis *encTrellis = MOS_New(AvcEncodeTrellis, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(AvcFeatureIDs::avcTrellisFeature, encTrellis, {AvcVdencPipeline::encodePreEncPacket}));

    AvcEncodeRounding *rounding = MOS_New(AvcEncodeRounding, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(AvcFeatureIDs::avcRoundingFeature, rounding, {AvcVdencPipeline::encodePreEncPacket}));

    AvcEncodeBRC *brc = MOS_New(AvcEncodeBRC, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(AvcFeatureIDs::avcBrcFeature, brc, {AvcVdencPipeline::encodePreEncPacket}));

    AvcVdencRoiInterface::SupportedModes supportedRoiModes;
    supportedRoiModes.DirtyROI           = 1;
    supportedRoiModes.ROI_Native         = 1;
    supportedRoiModes.ROI_NonNative      = 1;
    supportedRoiModes.MBQP_ForceQP       = 1;
    supportedRoiModes.MBQP_DeltaQP       = 1;
    AvcVdencCqpRoiFeature *cqpRoiFeature = MOS_New(AvcVdencCqpRoiFeature, this, m_allocator, m_hwInterface, constSettings, supportedRoiModes);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(AvcFeatureIDs::avcCqpRoiFeature, cqpRoiFeature, {AvcVdencPipeline::encodePreEncPacket}));

    MOS_ZeroMemory(&supportedRoiModes, sizeof(supportedRoiModes));
    supportedRoiModes.DirtyROI           = 1;
    supportedRoiModes.ROI_Native         = 1;
    supportedRoiModes.ROI_NonNative      = 1;
    supportedRoiModes.ArbROI             = 1;
    AvcVdencBrcRoiFeature *brcRoiFeature = MOS_New(AvcVdencBrcRoiFeature, this, m_allocator, m_hwInterface, constSettings, supportedRoiModes);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(AvcFeatureIDs::avcBrcRoiFeature, brcRoiFeature, {AvcVdencPipeline::encodePreEncPacket}));

    AvcVdencWeightedPred *avcWeightedPred = MOS_New(AvcVdencWeightedPred, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(AvcFeatureIDs::avcVdencWpFeature, avcWeightedPred, {AvcVdencPipeline::encodePreEncPacket}));

    //for pre-enc
    AvcVdencPreEnc *avcPreenc = MOS_New(AvcVdencPreEnc, this, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(FeatureIDs::preEncFeature, avcPreenc, {AvcVdencPipeline::encodePreEncPacket}, LIST_TYPE::ALLOW_LIST));  // only encodePreEncPacket
    AvcVdencFullEnc *avcFullEnc = MOS_New(AvcVdencFullEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(AvcFeatureIDs::avcVdencFullEncFeature, avcFullEnc, {AvcVdencPipeline::encodePreEncPacket}));

    AvcEncodeAqm *encAqm = MOS_New(AvcEncodeAqm, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(AvcFeatureIDs::avcAqm, encAqm, {AvcVdencPipeline::encodePreEncPacket}));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAvcVdencFeatureManagerXe2_Lpm::MapTargetUsage(uint8_t &targetUsage)
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
    case 7:
        targetUsage = 7;
        break;
    default:
        targetUsage = 4;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
