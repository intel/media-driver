/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_hevc_vdenc_feature_manager_xe3p_lpm_base.cpp
//! \brief    Defines the common interface for hevc Xe3P_LPM_Base encode feature manager
//!

#include "encode_vdenc_lpla_analysis.h"
#include "encode_hevc_vdenc_lpla_enc.h"
#include "encode_hevc_vdenc_feature_manager_xe3p_lpm_base.h"
#include "encode_hevc_aqm.h"
#include "encode_vdenc_hevc_fastpass.h"
#include "encode_vdenc_hevc_height_padding.h"
#include "encode_hevc_basic_feature_xe3p_lpm_base.h"
#if _KERNEL_RESERVED
#include "encode_hevc_vdenc_saliency.h"
#include "encode_hevc_vdenc_saliency_xe3p_lpm.h"
#include "encode_hevc_tf_xe3p_lpm.h"
#endif

#include <vector>

namespace encode
{

MOS_STATUS EncodeHevcVdencFeatureManagerXe3P_Lpm_Base::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeHevcVdencConstSettingsXe3P_Lpm_Base);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManagerXe3P_Lpm_Base::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    auto setting = static_cast<EncodeHevcVdencConstSettings *>(m_featureConstSettings);
    ENCODE_CHK_NULL_RETURN(setting);
    setting->SetOsInterface(m_hwInterface->GetOsInterface());

    // Block list for the standard features. RegisterFeatures takes the list by rvalue
    // reference, so a new vector is produced on each call. The preEnc packet has always
    // been blocked; when the pre-encode filter kernel-pipeline packets exist
    // (_KERNEL_RESERVED), the reused hme-ds / me packets are blocked too so these features
    // do NOT leak their SETPAR into them (only the TF feature should drive those packets).
    auto stdPkts = []() -> std::vector<int> {
#if _KERNEL_RESERVED
        return {HevcPipeline::encodePreEncPacket, EncodePipeline::hmeDsPacket, EncodePipeline::mePacket};
#else
        return {HevcPipeline::encodePreEncPacket};
#endif
    };

    // Block list for features that previously had no list (allowed everywhere). Keep them
    // allowed on every packet except the pre-encode filter kernel packets, so the
    // preEnc/full-encode behavior is unchanged while still preventing the leak into
    // hme-ds / me.
    auto noTfPkts = []() -> std::vector<int> {
#if _KERNEL_RESERVED
        return {EncodePipeline::hmeDsPacket, EncodePipeline::mePacket};
#else
        return {};
#endif
    };

    EncodeBasicFeature *encBasic = MOS_New(HevcBasicFeatureXe3p_Lpm_Base, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::basicFeature, encBasic, stdPkts()));

    HevcEncodeCqp *encCqp = MOS_New(HevcEncodeCqp, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcCqpFeature, encCqp, stdPkts()));

    HevcEncodeTile *encTile = MOS_New(HevcEncodeTile, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::encodeTile, encTile, stdPkts()));

    HevcVdencSccXe3P_Lpm_Base *hevcScc = MOS_New(HevcVdencSccXe3P_Lpm_Base, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencSccFeature, hevcScc, stdPkts()));

    HevcVdencPreEnc *hevcPreenc = MOS_New(HevcVdencPreEnc, this, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(FeatureIDs::preEncFeature, hevcPreenc, {HevcPipeline::encodePreEncPacket}, LIST_TYPE::ALLOW_LIST));  // only encodePreEncPacket

    HevcVdencFullEnc *hevcFullenc = MOS_New(HevcVdencFullEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcFullEncFeature, hevcFullenc, stdPkts()));

    HEVCEncodeBRC *brc = MOS_New(HEVCEncodeBRC, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcBrcFeature, brc, stdPkts()));

    HevcVdencRoi *hevcRoi = MOS_New(HevcVdencRoi, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencRoiFeature, hevcRoi, stdPkts()));

    HevcVdencWeightedPred *hevcWeightedPred = MOS_New(HevcVdencWeightedPred, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencWpFeature, hevcWeightedPred, stdPkts()));

    HevcEncodeDss *hevcDss = MOS_New(HevcEncodeDss, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencDssFeature, hevcDss, stdPkts()));

    VdencLplaAnalysis *lplaAnalysis = MOS_New(VdencLplaAnalysis, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::vdencLplaAnalysisFeature, lplaAnalysis, stdPkts()));

    HEVCVdencLplaEnc *lplaEnc = MOS_New(HEVCVdencLplaEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencLplaEncFeature, lplaEnc, stdPkts()));

    HevcEncodeAqm *hevcAqm = MOS_New(HevcEncodeAqm, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcAqm, hevcAqm, stdPkts()));

    HevcVdencFastPass *hevcFastPass = MOS_New(HevcVdencFastPass, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencFastPassFeature, hevcFastPass, noTfPkts()));

#if _KERNEL_RESERVED
    if (m_hwInterface->GetPlatform().eRenderCoreFamily == IGFX_XE3P_CORE)
    {
        HevcVdencSaliencyXe3p_Lpm *hevcSaliency = MOS_New(HevcVdencSaliencyXe3p_Lpm, this, m_allocator, m_hwInterface, constSettings);
        ENCODE_CHK_STATUS_RETURN(RegisterFeatures(FeatureIDs::saliencyFeature, hevcSaliency, stdPkts()));
    }
    else
    {
        HevcVdencSaliency *hevcSaliency = MOS_New(HevcVdencSaliency, this, m_allocator, m_hwInterface, constSettings);
        ENCODE_CHK_STATUS_RETURN(RegisterFeatures(FeatureIDs::saliencyFeature, hevcSaliency, stdPkts()));
    }
#endif

    HevcVdencHeightPadding *hevcHeightPadding = MOS_New(HevcVdencHeightPadding, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencHeightPaddingFeature, hevcHeightPadding, noTfPkts()));

#if _KERNEL_RESERVED
    // Temporal filter (pre-encode): HEVC reuses the shared TF feature + the reused kernel
    // packets. It is registered under the shared FeatureIDs::tfFeature id so the reused
    // hme-ds / me / mc / tf packets (which look it up by that id) resolve it unchanged.
    TfFeature *tf = MOS_New(HevcTfFeatureXe3pLpm, this, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(FeatureIDs::tfFeature, tf,
                            {EncodePipeline::hmeDsPacket, EncodePipeline::mePacket},
                            LIST_TYPE::ALLOW_LIST));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManagerXe3P_Lpm_Base::MapTargetUsage(uint8_t &targetUsage)
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

MOS_STATUS EncodeHevcVdencFeatureManagerXe3P_Lpm_Base::CheckPlatformCapability(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams,
    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSliceParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);
    ENCODE_CHK_NULL_RETURN(hevcSliceParams);

    if (hevcSeqParams->bit_depth_luma_minus8 >= 4 || hevcSeqParams->bit_depth_chroma_minus8 >= 4 || hevcSeqParams->SourceBitDepth >= 2)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        ENCODE_ASSERTMESSAGE("12bit input or encoding is not supported on HEVC VDENC");
    }

    if (hevcSeqParams->ParallelBRC)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        ENCODE_ASSERTMESSAGE("Parallel BRC is not supported on VDENC");
    }

    if (hevcSeqParams->log2_max_coding_block_size_minus3 != 3)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        ENCODE_ASSERTMESSAGE("HEVC VDEnc only supports LCU64");
    }

    uint16_t numLCUsInFrameWidth = MOS_ALIGN_CEIL((hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) * (1 << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), CODECHAL_HEVC_MAX_LCU_SIZE_G10) / CODECHAL_HEVC_MAX_LCU_SIZE_G10;
    if (hevcSliceParams->NumLCUsInSlice % numLCUsInFrameWidth != 0 && hevcPicParams->num_tile_columns_minus1 == 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        ENCODE_ASSERTMESSAGE("Row unaligned slice is not supported on HEVC VDEnc");
    }

    return eStatus;
}

}
