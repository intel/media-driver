/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     encode_av1_vdenc_feature_manager_xe2_lpm_base.cpp
//! \brief    Defines Xe2_LPM+ common class for av1 vdenc feature manager
//!

#include "encode_av1_vdenc_feature_manager_xe2_lpm_base.h"
#include "encode_av1_basic_feature_xe2_lpm_base.h"
#include "encode_av1_superres.h"
#include "encode_av1_aqm.h"
#include "encode_av1_vdenc_const_settings_xe2_lpm_base.h"
#include "encode_av1_scc_xe2_lpm.h"
#include "encode_av1_vdenc_preenc.h"
#include "encode_av1_vdenc_fullenc.h"
#include "encode_av1_vdenc_lpla_enc.h"
#include "encode_av1_pipeline.h"
#include "encode_av1_tile_xe2_lpm.h"
#include "encode_av1_rounding_table.h"
#if _MEDIA_RESERVED
#include "encode_av1_feature_ext.h"
#endif  // !(_MEDIA_RESERVED)

namespace encode
{

MOS_STATUS EncodeAv1VdencFeatureManagerXe2_Lpm_Base::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeAv1VdencConstSettingsXe2_Lpm_Base, m_hwInterface->GetOsInterface());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencFeatureManagerXe2_Lpm_Base::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    Av1SuperRes *superRes = MOS_New(Av1SuperRes, this, m_allocator, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1SuperRes, superRes, { Av1Pipeline::encodePreEncPacket }));

    EncodeBasicFeature *encBasic = MOS_New(Av1BasicFeatureXe2_Lpm_Base, this, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::basicFeature, encBasic, { Av1Pipeline::encodePreEncPacket }));

    Av1EncodeTile_Xe2_Lpm *encTile = MOS_New(Av1EncodeTile_Xe2_Lpm, this, m_allocator, m_hwInterface, constSettings);
    MEDIA_CHK_NULL_RETURN(encTile);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::encodeTile, encTile, { Av1Pipeline::encodePreEncPacket }));

    Av1Segmentation *encSegmentation = MOS_New(Av1Segmentation, this, m_allocator, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1Segmentation, encSegmentation, { Av1Pipeline::encodePreEncPacket }));

    Av1Brc *encBrc = MOS_New(Av1Brc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1BrcFeature, encBrc, { Av1Pipeline::encodePreEncPacket }));

    Av1SccXe2_Lpm *encSCC = MOS_New(Av1SccXe2_Lpm, m_allocator, m_hwInterface, constSettings, this);
    ENCODE_CHK_NULL_RETURN(encSCC);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1Scc, encSCC, { Av1Pipeline::encodePreEncPacket }));

    Av1EncodeAqm *encAqm = MOS_New(Av1EncodeAqm, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1Aqm, encAqm, { Av1Pipeline::encodePreEncPacket }));

#if _MEDIA_RESERVED
    Av1ReservedFeature2* av1ReservedFeature2 = MOS_New(Av1ReservedFeature2, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1ReservedFeatureID3, av1ReservedFeature2, {Av1Pipeline::encodePreEncPacket}));
#endif  // _MEDIA_RESERVED

    Av1EncodeRoundingTable *encFeature = MOS_New(Av1EncodeRoundingTable, this, m_hwInterface, constSettings);
    MEDIA_CHK_NULL_RETURN(encFeature);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1LookupTableRounding, encFeature, { Av1Pipeline::encodePreEncPacket }));

    Av1VdencPreEnc* av1Preenc = MOS_New(Av1VdencPreEnc, this, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(FeatureIDs::preEncFeature, av1Preenc, { Av1Pipeline::encodePreEncPacket }, LIST_TYPE::ALLOW_LIST));

    Av1VdencFullEnc* av1Fullenc = MOS_New(Av1VdencFullEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1FullEncFeature, av1Fullenc, { Av1Pipeline::encodePreEncPacket }));

    AV1VdencLplaEnc* lplaEnc = MOS_New(AV1VdencLplaEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1LplaEncFeature, lplaEnc, { Av1Pipeline::encodePreEncPacket }));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencFeatureManagerXe2_Lpm_Base::MapTargetUsage(uint8_t &targetUsage)
{
    ENCODE_FUNC_CALL();

    switch (targetUsage)
    {
    case 1:
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
