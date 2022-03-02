/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_hevc_vdenc_feature_manager_xe_xpm_base.cpp
//! \brief    Defines the common interface for hevc M12.5 Plus encode feature manager
//!

#include "encode_hevc_vdenc_feature_manager_xe_xpm_base.h"
#include "encode_vdenc_lpla_analysis.h"
#include "encode_hevc_vdenc_lpla_enc.h"

namespace encode
{

MOS_STATUS EncodeHevcVdencFeatureManagerXe_Xpm_Base::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeHevcVdencConstSettingsXe_Xpm_Base);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManagerXe_Xpm_Base::CheckPlatformCapability(
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

    uint16_t numLCUsInFrameWidth = MOS_ALIGN_CEIL((hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) * (1 << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), CODECHAL_HEVC_VDENC_LCU_SIZE) / CODECHAL_HEVC_VDENC_LCU_SIZE;
    if (hevcSliceParams->NumLCUsInSlice % numLCUsInFrameWidth != 0 && hevcPicParams->num_tile_columns_minus1 == 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        ENCODE_ASSERTMESSAGE("Row unaligned slice is not supported on HEVC VDEnc");
    }

    return eStatus;
}

MOS_STATUS EncodeHevcVdencFeatureManagerXe_Xpm_Base::CreateFeatures(void *constSettings)
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

}
