/*
* Copyright (c) 2022 - 2023, Intel Corporation
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
//! \file     encode_av1_vdenc_feature_manager_xe3_lpm_base.cpp
//! \brief    Defines Xe3_LPM+ common class for av1 vdenc feature manager
//!

#include "encode_av1_vdenc_feature_manager_xe3_lpm_base.h"
#include "encode_av1_basic_feature_xe3_lpm_base.h"
#include "encode_av1_superres.h"
#include "encode_av1_aqm.h"
#include "encode_av1_vdenc_const_settings_xe3_lpm_base.h"
#include "encode_av1_scc_xe3_lpm_base.h"
#if _MEDIA_RESERVED
#include "encode_av1_rho_domain_xe3_lpm_base.h"
#endif
#include "encode_av1_tile_xe3_lpm_base.h"
#include "encode_av1_fastpass.h"
#include "encode_av1_vdenc_preenc.h"
#include "encode_av1_vdenc_fullenc.h"
#include "encode_av1_vdenc_lpla_enc.h"
#include "encode_av1_pipeline.h"
#if _MEDIA_RESERVED
#include "encode_av1_feature_xe3_lpm_base_ext.h"
#endif

namespace encode
{

MOS_STATUS EncodeAv1VdencFeatureManagerXe3_Lpm_Base::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeAv1VdencConstSettingsXe3_Lpm_Base, m_hwInterface->GetOsInterface());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencFeatureManagerXe3_Lpm_Base::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    Av1SuperRes *superRes = MOS_New(Av1SuperRes, this, m_allocator, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1SuperRes, superRes, { Av1Pipeline::encodePreEncPacket }));

    EncodeBasicFeature *encBasic = MOS_New(Av1BasicFeatureXe3_Lpm_Base, this, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::basicFeature, encBasic, { Av1Pipeline::encodePreEncPacket }));

    Av1EncodeTile_Xe3_LpmBase *encTile = MOS_New(Av1EncodeTile_Xe3_LpmBase, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::encodeTile, encTile, { Av1Pipeline::encodePreEncPacket }));

    Av1Segmentation *encSegmentation = MOS_New(Av1Segmentation, this, m_allocator, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1Segmentation, encSegmentation, { Av1Pipeline::encodePreEncPacket }));

    Av1Brc *encBrc = MOS_New(Av1Brc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1BrcFeature, encBrc, { Av1Pipeline::encodePreEncPacket }));

    Av1Scc *encSCC = MOS_New(Av1SccXe3_Lpm_Base, m_allocator, m_hwInterface, constSettings, this);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1Scc, encSCC, { Av1Pipeline::encodePreEncPacket }));

    Av1EncodeAqm *encAqm = MOS_New(Av1EncodeAqm, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1Aqm, encAqm, { Av1Pipeline::encodePreEncPacket }));

#if _MEDIA_RESERVED
    Av1ReservedFeature2_Xe3_Lpm_Base *av1ReservedFeature2 = MOS_New(Av1ReservedFeature2_Xe3_Lpm_Base, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1ReservedFeatureID3, av1ReservedFeature2, { Av1Pipeline::encodePreEncPacket }));
#endif

    Av1FastPass *encFastPass = MOS_New(Av1FastPass, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1FastPass, encFastPass, { Av1Pipeline::encodePreEncPacket }));

    Av1VdencPreEnc* av1Preenc = MOS_New(Av1VdencPreEnc, this, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(FeatureIDs::preEncFeature, av1Preenc, { Av1Pipeline::encodePreEncPacket }, LIST_TYPE::ALLOW_LIST));

    Av1VdencFullEnc* av1Fullenc = MOS_New(Av1VdencFullEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1FullEncFeature, av1Fullenc, { Av1Pipeline::encodePreEncPacket }));

    AV1VdencLplaEnc* lplaEnc = MOS_New(AV1VdencLplaEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1LplaEncFeature, lplaEnc, { Av1Pipeline::encodePreEncPacket }));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencFeatureManagerXe3_Lpm_Base::MapTargetUsage(uint8_t &targetUsage)
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
        {
            targetUsage      = 7;
            auto osInterface = m_hwInterface->GetOsInterface();
            if (osInterface)
            {
                //If Wa_15017562431 is set, map tu 7 to tu6
                MEDIA_WA_TABLE *waTable = osInterface->pfnGetWaTable(osInterface);
                if (waTable && MEDIA_IS_WA(waTable, Wa_15017562431))
                {
                    targetUsage = 6;
                }
            }
            break;
        }
    default:
        targetUsage = 4;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

}
