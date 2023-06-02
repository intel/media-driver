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
//! \file     encode_hevc_vdenc_feature_manager_xe_hpm.cpp
//! \brief    Defines the common interface for hevc Xe_HPM encode feature manager
//!

#include "encode_hevc_vdenc_feature_manager_xe_hpm.h"
#include "encode_hevc_vdenc_scc.h"
namespace encode
{
MOS_STATUS EncodeHevcVdencFeatureManagerXe_Hpm::CreateFeatures(void *constSettings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodeHevcVdencFeatureManagerXe_Xpm_Base::CreateFeatures(constSettings));
    HevcVdencScc *hevcScc = MOS_New(HevcVdencScc, this, m_allocator, m_hwInterface, constSettings);
    ENCODE_CHK_STATUS_RETURN(RegisterFeatures(HevcFeatureIDs::hevcVdencSccFeature, hevcScc));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManagerXe_Hpm::CheckFeatures(void* params)
{
    ENCODE_FUNC_CALL();

    EncoderParams *encodeParams = (EncoderParams *)params;

    auto m_basicFeature = dynamic_cast<HevcBasicFeature *>(GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    auto waTable = m_basicFeature->GetWaTable();
    MHW_MI_CHK_NULL(waTable);

    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams =
        static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams =
        static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);

    if (m_basicFeature->m_422State)
    {
        ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_422State->Init(hevcSeqParams, hevcPicParams));
    }
    if (MEDIA_IS_WA(waTable, WaEnableOnlyASteppingFeatures))
    {
        ENCODE_CHK_STATUS_RETURN(ValidateASteppingNotSupportedFeatures(hevcSeqParams, hevcPicParams));
    }

    ENCODE_CHK_STATUS_RETURN(EncodeHevcVdencFeatureManager::CheckFeatures(params));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeHevcVdencFeatureManagerXe_Hpm::ValidateASteppingNotSupportedFeatures(
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);

    // A stepping not support RollingI/SCC/DSS.
    if (hevcSeqParams->palette_mode_enabled_flag ||
        hevcPicParams->pps_curr_pic_ref_enabled_flag ||
        hevcSeqParams->SliceSizeControl ||
        hevcPicParams->bEnableRollingIntraRefresh)
    {
        ENCODE_ASSERTMESSAGE("RollingI/SCC/DSS are not supported on A stepping.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}
}
