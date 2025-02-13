/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     encode_hevc_vdenc_feature_manager.cpp
//! \brief    Defines the common interface for hevc vdenc feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_hevc_vdenc_feature_manager.h"
#include "encode_hevc_reference_frames.h"
#include "encode_hevc_vdenc_const_settings.h"
#include "encode_hevc_vdenc_weighted_prediction.h"
#include "encode_hevc_brc.h"
#include "encode_vdenc_lpla_analysis.h"
#include "encode_hevc_vdenc_lpla_enc.h"

namespace encode
{
MOS_STATUS EncodeHevcVdencFeatureManager::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeHevcVdencConstSettings);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManager::CheckFeatures(void *params)
{
    ENCODE_FUNC_CALL();
    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams =
        static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams =
        static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);
    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSliceParams =
        static_cast<PCODEC_HEVC_ENCODE_SLICE_PARAMS>(encodeParams->pSliceParams);
    ENCODE_CHK_NULL_RETURN(hevcSliceParams);

    auto settings = static_cast<EncodeHevcVdencConstSettings *>(m_featureConstSettings);
    ENCODE_CHK_NULL_RETURN(settings);
    settings->Update(params);

    if (encodeParams->bNewSeq)
    {
        m_ddiTargetUsage = hevcSeqParams->TargetUsage;
        ENCODE_CHK_STATUS_RETURN(MapTargetUsage(hevcSeqParams->TargetUsage));
        m_targetUsage = hevcSeqParams->TargetUsage;
    }

    auto sliceParams = hevcSliceParams;
    ENCODE_CHK_NULL_RETURN(sliceParams);
    for (uint32_t s = 0; s < encodeParams->dwNumSlices; s++, sliceParams++)
    {
        ENCODE_CHK_STATUS_RETURN(ValidateRandomAccess(hevcSeqParams, hevcPicParams, sliceParams));
    }

    // Screen content flag will come in with PPS on Android, but in SPS on Android,
    // we will use screen content flag in PPS for kernel programming, and update
    // the PPS screen content flag based on the SPS screen content flag if enabled.
    hevcPicParams->bScreenContent |= hevcSeqParams->bScreenContent;

    ENCODE_CHK_STATUS_RETURN(ValidateSCC(hevcPicParams));

    ENCODE_CHK_STATUS_RETURN(ValidateACQP(hevcSeqParams, hevcPicParams));

    ENCODE_CHK_STATUS_RETURN(ValidatePassNum(hevcSeqParams, hevcPicParams));

    ENCODE_CHK_STATUS_RETURN(CheckPlatformCapability(hevcSeqParams, hevcPicParams, hevcSliceParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManager::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    auto setting = static_cast<EncodeHevcVdencConstSettings *>(m_featureConstSettings);
    ENCODE_CHK_NULL_RETURN(setting);
    setting->SetOsInterface(m_hwInterface->GetOsInterface());

    EncodeBasicFeature *encBasic = MOS_New(HevcBasicFeature, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::basicFeature, encBasic));

    HevcEncodeCqp *encCqp = MOS_New(HevcEncodeCqp, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcCqpFeature, encCqp));

    HevcEncodeTile *encTile = MOS_New(HevcEncodeTile, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::encodeTile, encTile));

    HEVCEncodeBRC *brc = MOS_New(HEVCEncodeBRC, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcBrcFeature, brc));

    HevcVdencRoi *hevcRoi = MOS_New(HevcVdencRoi, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencRoiFeature, hevcRoi));

    HevcVdencWeightedPred *hevcWeightedPred = MOS_New(HevcVdencWeightedPred, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencWpFeature, hevcWeightedPred));

    HevcEncodeDss *hevcDss = MOS_New(HevcEncodeDss, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencDssFeature, hevcDss));

    HevcVdencScc *hevcScc = MOS_New(HevcVdencScc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencSccFeature, hevcScc));

    VdencLplaAnalysis *lplaAnalysis = MOS_New(VdencLplaAnalysis, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::vdencLplaAnalysisFeature, lplaAnalysis));

    HEVCVdencLplaEnc *lplaEnc = MOS_New(HEVCVdencLplaEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencLplaEncFeature, lplaEnc));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManager::MapTargetUsage(uint8_t &targetUsage)
{
    ENCODE_FUNC_CALL();

    switch (targetUsage)
    {
    case 1: case 2:
        targetUsage = 2;
        break;
    case 3: case 4: case 5:
        targetUsage = 4;
        break;
    case 6: case 7:
        targetUsage = 7;
        break;
    default:
        targetUsage = 4;
        break;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManager::ValidateRandomAccess(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    slcParams)
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus        = MOS_STATUS_SUCCESS;
    bool       isRandomAccess = false;

    ENCODE_CHK_NULL_RETURN(slcParams);

    if (slcParams->slice_type == CODECHAL_HEVC_B_SLICE)
    {
        if (slcParams->num_ref_idx_l0_active_minus1 != slcParams->num_ref_idx_l1_active_minus1)
        {
            isRandomAccess = true;
        }

        for (auto j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; j++)
        {
            if (slcParams->RefPicList[0][j].PicEntry != slcParams->RefPicList[1][j].PicEntry)
            {
                isRandomAccess = true;
            }
        }
    }

    if (isRandomAccess)
    {
        ENCODE_CHK_NULL_RETURN(hevcPicParams);
        ENCODE_CHK_NULL_RETURN(hevcSeqParams);

        // SCC + RA B is not supported.
        auto sccFeature = dynamic_cast<HevcVdencScc *>(GetFeature(HevcFeatureIDs::hevcVdencSccFeature));
        if (sccFeature && sccFeature->IsSCCEnabled())
        {
            ENCODE_ASSERTMESSAGE("SCC is in conflict with RandomAccess.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }

        if (hevcPicParams->bEnableRollingIntraRefresh)
        {
            ENCODE_ASSERT(false);
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }
    }

    uint8_t maxNumRef0 = isRandomAccess ? 2 : HevcReferenceFrames::m_numMaxVdencL0Ref;
    uint8_t maxNumRef1 = isRandomAccess ? 1 : HevcReferenceFrames::m_numMaxVdencL1Ref;

    if (slcParams->num_ref_idx_l0_active_minus1 > maxNumRef0 - 1)
    {
        ENCODE_ASSERT(false);
        slcParams->num_ref_idx_l0_active_minus1 = maxNumRef0 - 1;
    }

    if (slcParams->num_ref_idx_l1_active_minus1 > maxNumRef1 - 1)
    {
        ENCODE_ASSERT(false);
        slcParams->num_ref_idx_l1_active_minus1 = maxNumRef1 - 1;
    }

    return eStatus;
}

MOS_STATUS EncodeHevcVdencFeatureManager::ValidateSCC(PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(hevcPicParams);

    if (hevcPicParams->tiles_enabled_flag)
    {
        auto sccFeature = dynamic_cast<HevcVdencScc *>(GetFeature(HevcFeatureIDs::hevcVdencSccFeature));
        ENCODE_CHK_NULL_RETURN(sccFeature);
        if (sccFeature->IsSCCEnabled() && hevcPicParams->pps_curr_pic_ref_enabled_flag)
        {
            for (int i = 0; i < hevcPicParams->num_tile_columns_minus1 + 1; i++)
            {
                if (hevcPicParams->tile_column_width[i] < 5)
                {
                    ENCODE_ASSERTMESSAGE("IBC can not be enabled if tile width is less than 320.");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManager::ValidateACQP(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);

    // ACQP is by default disabled, only enable it when WP/SSC/QpAdjust required.
    if (hevcSeqParams->SliceSizeControl == true ||
        hevcSeqParams->QpAdjustment == true ||
        ((hevcPicParams->weighted_pred_flag ||
        hevcPicParams->weighted_bipred_flag) &&
        hevcPicParams->bEnableGPUWeightedPrediction == true))
    {
        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(GetFeature(HevcFeatureIDs::hevcBrcFeature));

        ENCODE_CHK_NULL_RETURN(brcFeature);
        brcFeature->SetACQPStatus(true);
    }

    return MOS_STATUS_SUCCESS;
}
MOS_STATUS EncodeHevcVdencFeatureManager::ValidatePassNum(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);

    auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(GetFeature(HevcFeatureIDs::hevcBrcFeature));

    auto basicFeature = dynamic_cast<HevcBasicFeature *>(GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);
    if (basicFeature->m_422State && basicFeature->m_422State->GetFeature422Flag())
    {
        hevcPicParams->BRCPrecision = 1;
    }

    // dynamic slice size control and brc to be added later here
    if (((hevcPicParams->weighted_pred_flag ||
        hevcPicParams->weighted_bipred_flag) &&
        hevcPicParams->bEnableGPUWeightedPrediction == true) ||
        hevcSeqParams->SliceSizeControl || (brcFeature->IsRateControlBrc(hevcSeqParams->RateControlMethod) && hevcPicParams->BRCPrecision != 1))
    {
        m_passNum = 2;
    }
    else
    {
        // Currently no advance feature to combine with tile replay CQP
        // Also no extra frame level pass for tile replay BRC
        m_passNum = 1;
    }

    auto lplaFeature = dynamic_cast<VdencLplaAnalysis *>(GetFeature(HevcFeatureIDs::vdencLplaAnalysisFeature));
    if (lplaFeature && hevcSeqParams->LookaheadDepth > 0 && hevcSeqParams->bLookAheadPhase)
    {
        m_passNum = 1;

        if (hevcPicParams->CodingType != I_TYPE && lplaFeature->IsLplaAIdrEnabled())
        {
            m_passNum += 1;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManager::CheckPlatformCapability(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS    hevcSliceParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);
    ENCODE_CHK_NULL_RETURN(hevcSliceParams);

    return eStatus;
}

}  // namespace encode
