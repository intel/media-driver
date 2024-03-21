/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     encode_av1_vdenc_feature_manager.cpp
//! \brief    Defines the common interface for av1 vdenc feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_av1_vdenc_feature_manager.h"
#include "encode_av1_reference_frames.h"
#include "encode_av1_vdenc_const_settings.h"

namespace encode
{

MOS_STATUS EncodeAv1VdencFeatureManager::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeAv1VdencConstSettings, m_hwInterface->GetOsInterface());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencFeatureManager::CheckFeatures(void *params)
{
    ENCODE_FUNC_CALL();
    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS av1SeqParams =
        static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(av1SeqParams);

    auto settings = static_cast<EncodeAv1VdencConstSettings*>(m_featureConstSettings);
    ENCODE_CHK_NULL_RETURN(settings);
    settings->Update(params);

    if (encodeParams->bNewSeq)
    {
        m_ddiTargetUsage = av1SeqParams->TargetUsage;
        ENCODE_CHK_STATUS_RETURN(MapTargetUsage(av1SeqParams->TargetUsage));
        m_targetUsage = av1SeqParams->TargetUsage;
    }

    ENCODE_CHK_STATUS_RETURN(SetPassNum(av1SeqParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencFeatureManager::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    EncodeBasicFeature *encBasic = MOS_New(Av1BasicFeature, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::basicFeature, encBasic));

    Av1EncodeTile *encTile = MOS_New(Av1EncodeTile, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::encodeTile, encTile));

    Av1Segmentation *encSegmentation = MOS_New(Av1Segmentation, this, m_allocator, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1Segmentation, encSegmentation));

    Av1Brc *encBrc = MOS_New(Av1Brc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1BrcFeature, encBrc));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencFeatureManager::MapTargetUsage(uint8_t &targetUsage)
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
    case 7:
        targetUsage = 7;
        break;
    default:
        targetUsage = 4;
        break;
    }
    return MOS_STATUS_SUCCESS;
}


MOS_STATUS EncodeAv1VdencFeatureManager::SetPassNum(
    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS av1SeqParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(av1SeqParams);

    if (IsRateControlBrc(av1SeqParams->RateControlMethod))
    {
        m_passNum = 2;
    }
    else
    {
        m_passNum = 1;
    }

    return MOS_STATUS_SUCCESS;
}
}
