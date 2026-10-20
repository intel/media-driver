/*
* Copyright (c) 2021-2025, Intel Corporation
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
//! \file     encode_vp9_vdenc_feature_manager_xe3_lpm.cpp
//! \brief    Defines the common interface for vp9 vdenc feature manager
//!

#include "encode_vp9_vdenc_feature_manager_xe3_lpm.h"
#include "encode_vp9_basic_feature_xe3_lpm.h"
#include "encode_vp9_vdenc_const_settings.h"
#include "encode_vp9_brc.h"
#include "encode_vp9_segmentation.h"

#include "media_feature_manager.h"
#include "media_vp9_feature_defs.h"
#include "encode_vp9_hpu.h"
#include "encode_vp9_cqp.h"
#include "encode_vp9_pak.h"

namespace encode
{
MOS_STATUS EncodeVp9VdencFeatureManagerXe3_Lpm::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeVp9VdencConstSettingsXe3_Lpm);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeVp9VdencFeatureManagerXe3_Lpm::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    auto setting = static_cast<EncodeVp9VdencConstSettings *>(m_featureConstSettings);
    ENCODE_CHK_NULL_RETURN(setting);
    setting->SetOsInterface(m_hwInterface->GetOsInterface());

    EncodeBasicFeature *basicFeature = MOS_New(Vp9BasicFeatureXe3_Lpm, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Vp9FeatureIDs::basicFeature, basicFeature));

    Vp9EncodeHpu *hpuFeature = MOS_New(Vp9EncodeHpu, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Vp9FeatureIDs::vp9HpuFeature, hpuFeature));

    Vp9EncodeCqp *cqpFeature = MOS_New(Vp9EncodeCqp, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Vp9FeatureIDs::vp9CqpFeature, cqpFeature));

    Vp9EncodeTile *tileFeature = MOS_New(Vp9EncodeTile, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Vp9FeatureIDs::encodeTile, tileFeature));

    Vp9EncodeBrc *brcFeature = MOS_New(Vp9EncodeBrc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Vp9FeatureIDs::vp9BrcFeature, brcFeature));

    Vp9Segmentation *segmentFeature = MOS_New(Vp9Segmentation, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Vp9FeatureIDs::vp9Segmentation, segmentFeature));

    Vp9EncodePak *pakFeature = MOS_New(Vp9EncodePak, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Vp9FeatureIDs::vp9PakFeature, pakFeature));

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
