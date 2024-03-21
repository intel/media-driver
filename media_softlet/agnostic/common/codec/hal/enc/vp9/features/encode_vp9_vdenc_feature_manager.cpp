/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_vp9_vdenc_feature_manager.cpp
//! \brief    Defines the common interface for vp9 vdenc feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_vp9_vdenc_feature_manager.h"
#include "encode_vp9_vdenc_const_settings.h"
#include "codec_def_encode.h"
#include "encode_vp9_hpu.h"
#include "encode_vp9_brc.h"
#include "encode_vp9_cqp.h"
#include "encode_vp9_segmentation.h"
#include "encode_vp9_pak.h"

namespace encode
{
MOS_STATUS EncodeVp9VdencFeatureManager::CheckFeatures(void *params)
{
    ENCODE_FUNC_CALL();
    EncoderParams *encodeParams = (EncoderParams *)params;

    auto seqParams =
        static_cast<PCODEC_VP9_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(seqParams);
    auto picParams =
        static_cast<PCODEC_VP9_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(picParams);
    auto segmentParams =
        static_cast<PCODEC_VP9_ENCODE_SEGMENT_PARAMS>(encodeParams->pSegmentParams);

    auto settings = static_cast<EncodeVp9VdencConstSettings *>(m_featureConstSettings);
    ENCODE_CHK_NULL_RETURN(settings);
    ENCODE_CHK_STATUS_RETURN(settings->Update(params));

    if (encodeParams->bNewSeq)
    {
        // Store original target usage pass from app before it got modify.
        // CalculateRePakThresholds() is use original target usage to calculate
        // rePAK thresholds [See VP9 Codechal]
        auto basicFeature = static_cast<Vp9BasicFeature *>(GetFeature(Vp9FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(basicFeature);
        basicFeature->m_oriTargetUsage = seqParams->TargetUsage;

        m_ddiTargetUsage = seqParams->TargetUsage;
        ENCODE_CHK_STATUS_RETURN(MapTargetUsage(seqParams->TargetUsage));
        m_targetUsage = seqParams->TargetUsage;
    }

    ENCODE_CHK_STATUS_RETURN(ValidatePassNum(seqParams, picParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeVp9VdencFeatureManager::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeVp9VdencConstSettings);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeVp9VdencFeatureManager::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    EncodeBasicFeature *basicFeature = MOS_New(Vp9BasicFeature, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
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

MOS_STATUS EncodeVp9VdencFeatureManager::MapTargetUsage(uint8_t &targetUsage)
{
    ENCODE_FUNC_CALL();

    switch (targetUsage)
    {
    case 1:
    case 2:
        targetUsage = TargetUsage::QUALITY_2;
        break;
    case 3:
    case 4:
    case 5:
        targetUsage = TargetUsage::NORMAL;
        break;
    case 6:
    case 7:
        targetUsage = TargetUsage::PERFORMANCE;
        break;
    default:
        targetUsage = TargetUsage::NORMAL;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeVp9VdencFeatureManager::ValidatePassNum(PCODEC_VP9_ENCODE_SEQUENCE_PARAMS vp9SeqParams, PCODEC_VP9_ENCODE_PIC_PARAMS vp9PicParams)
{
    ENCODE_FUNC_CALL();

    auto basicFeature = static_cast<Vp9BasicFeature *>(GetFeature(Vp9FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);
    auto brcFeature = static_cast<Vp9EncodeBrc *>(GetFeature(Vp9FeatureIDs::vp9BrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    bool brcEnabled = CodecHalIsRateControlBrc(vp9SeqParams->RateControlMethod, CODECHAL_VP9);

    m_passNum = basicFeature->m_hucEnabled ? CODECHAL_ENCODE_VP9_CQP_NUM_OF_PASSES : 1;
    if (brcEnabled)
    {
        m_passNum = brcFeature->IsMultipassBrcSupported()
                        ? (CODECHAL_ENCODE_VP9_BRC_DEFAULT_NUM_OF_PASSES + 1)
                        : CODECHAL_ENCODE_VP9_BRC_DEFAULT_NUM_OF_PASSES;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
