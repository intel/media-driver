/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_av1_vdenc_feature_manager_xe_hpm.cpp
//! \brief    Defines the common interface for xe hpm av1 vdenc feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_av1_vdenc_feature_manager_xe_hpm.h"
#include "encode_av1_basic_feature_xe_hpm.h"
#include "encode_av1_segmentation_xe_hpm.h"
#include "encode_av1_vdenc_lpla_enc.h"
#include "encode_av1_vdenc_const_settings_xe_hpm.h"

namespace encode
{

MOS_STATUS EncodeAv1VdencFeatureManagerXe_Hpm::CreateConstSettings()
{
    ENCODE_FUNC_CALL();
    m_featureConstSettings = MOS_New(EncodeAv1VdencConstSettingsXe_Hpm, m_hwInterface->GetOsInterface());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencFeatureManagerXe_Hpm::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    EncodeBasicFeature *encBasic = MOS_New(Av1BasicFeatureXe_Hpm, m_allocator, m_hwInterface, m_trackedBuf, m_recycleResource, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::basicFeature, encBasic));

    Av1EncodeTile *encTile = MOS_New(Av1EncodeTile, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::encodeTile, encTile));

    Av1SegmentationXe_Hpm *encSegmentation = MOS_New(Av1SegmentationXe_Hpm, this, m_allocator, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1Segmentation, encSegmentation));

    Av1Brc *encBrc = MOS_New(Av1Brc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1BrcFeature, encBrc));

    AV1VdencLplaEnc* lplaEnc = MOS_New(AV1VdencLplaEnc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(Av1FeatureIDs::av1LplaEncFeature, lplaEnc));

    return MOS_STATUS_SUCCESS;
}
}
