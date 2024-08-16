/*
* Copyright (c) 2021 - 2024, Intel Corporation
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
//! \file     encode_av1_vdenc_feature_manager_xe_lpm_plus_base.cpp
//! \brief    Defines Xe_LPM_plus+ common class for av1 vdenc feature manager
//!

#include "encode_av1_vdenc_feature_manager_xe_lpm_plus_base.h"
#include "encode_av1_superres.h"
#include "encode_av1_basic_feature_xe_lpm_plus_base.h"
#include "encode_av1_vdenc_const_settings_xe_lpm_plus_base.h"
#include "encode_av1_pipeline.h"
#include "encode_av1_vdenc_lpla_enc.h"
#if _MEDIA_RESERVED
#include "encode_av1_aqm.h"
#endif  // !(_MEDIA_RESERVED)

namespace encode
{

MOS_STATUS EncodeAv1VdencFeatureManagerXe_Lpm_Plus_Base::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeAv1VdencConstSettingsXe_Lpm_Plus_Base, m_hwInterface->GetOsInterface());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencFeatureManagerXe_Lpm_Plus_Base::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    // packetIdListType indicate whether packet ID list is a block list or an allow list, by default it is a block list.

    Av1SuperRes *superRes = MOS_New(Av1SuperRes, this, m_allocator, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1SuperRes, superRes));

    EncodeBasicFeature *encBasic = MOS_New(Av1BasicFeatureXe_Lpm_Plus_Base, this, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::basicFeature, encBasic, {Av1Pipeline::encodePreEncPacket}));

    Av1EncodeTile *encTile = MOS_New(Av1EncodeTile, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::encodeTile, encTile, {Av1Pipeline::encodePreEncPacket}));

    Av1Segmentation *encSegmentation = MOS_New(Av1Segmentation, this, m_allocator, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1Segmentation, encSegmentation, {Av1Pipeline::encodePreEncPacket}));

    Av1Brc *encBrc = MOS_New(Av1Brc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1BrcFeature, encBrc, {Av1Pipeline::encodePreEncPacket}));

    // pre-enc: encodePreEncPacket add Av1VdencPreEnc in the allow list and block other features.
    // In pre-enc 1st pass, the encodePreEncPacket only needs its preEnc feature.
    // In pre-enc 2nd pass, Av1VdencFullEnc + Av1VdencPacket.
    // If needed, other features can be enabled in the 2nd pass.
    Av1VdencPreEnc *av1Preenc = MOS_New(Av1VdencPreEnc, this, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(FeatureIDs::preEncFeature, av1Preenc, {Av1Pipeline::encodePreEncPacket}, LIST_TYPE::ALLOW_LIST));

    Av1VdencFullEnc* av1Fullenc = MOS_New(Av1VdencFullEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1FullEncFeature, av1Fullenc, {Av1Pipeline::encodePreEncPacket}));

    AV1VdencLplaEnc* lplaEnc = MOS_New(AV1VdencLplaEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1LplaEncFeature, lplaEnc, {Av1Pipeline::encodePreEncPacket}));

#if _MEDIA_RESERVED
    Av1EncodeAqm *encAqm = MOS_New(Av1EncodeAqm, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1Aqm, encAqm, { Av1Pipeline::encodePreEncPacket }));
#endif  // !(_MEDIA_RESERVED)
    return MOS_STATUS_SUCCESS;
}

}
