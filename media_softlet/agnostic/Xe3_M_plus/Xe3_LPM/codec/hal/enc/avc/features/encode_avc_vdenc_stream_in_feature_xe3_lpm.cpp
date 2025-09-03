/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_avc_vdenc_stream_in_xe3_lpm.cpp
//! \brief    Defines the interface for encode AVC VDEnc Stream-in on xe3_lpm
//!

#include "encode_avc_vdenc_stream_in_feature_xe3_lpm.h"
#include "encode_avc_brc_xe3_lpm.h"
#include "media_avc_feature_defs.h"

namespace encode
{
AvcVdencStreamInFeatureXe3_Lpm::AvcVdencStreamInFeatureXe3_Lpm(
    MediaFeatureManager     *featureManager,
    EncodeAllocator         *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void                    *constSettings) : AvcVdencStreamInFeature(featureManager, allocator, hwInterface, constSettings)
{
    m_featureManager = featureManager;
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_featureManager);

    m_basicFeature = dynamic_cast<AvcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_IMG_STATE, AvcVdencStreamInFeatureXe3_Lpm)
{
    auto picParams = m_basicFeature->m_picParam;

    auto brcFeature = dynamic_cast<AvcEncodeBRCXe3_Lpm*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    params.roiEnable             = m_enabled && picParams->EnableRollingIntraRefresh == ROLLING_I_DISABLED &&
                                    (picParams->NumDirtyROI && brcFeature->IsVdencBrcEnabled() ||
                                     picParams->NumROI && picParams->bNativeROI ||
                                     picParams->TargetFrameSize > 0 && !m_basicFeature->m_lookaheadDepth);  // TCBRC (for AdaptiveRegionBoost)

#if _MEDIA_RESERVED
    params.vdencAvcImgStatePar0 = m_enabled;
#else
    params.extSettings.emplace_back(
        [this](uint32_t *data) {
            data[13] |= m_enabled << 1;
            return MOS_STATUS_SUCCESS;
        });
#endif  // _MEDIA_RESERVED

    params.mbLevelQpEnable      = m_enabled && picParams->EnableRollingIntraRefresh == ROLLING_I_DISABLED &&
                                   (picParams->NumROI && !picParams->bNativeROI || m_basicFeature->m_mbQpDataEnabled);

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
