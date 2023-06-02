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
//! \file     encode_hevc_vdenc_feature_manager_xe_lpm_plus.cpp
//! \brief    Defines the common interface for hevc Xe_Lpm_Plus encode feature manager
//!

#include "encode_hevc_vdenc_feature_manager_xe_lpm_plus.h"

namespace encode
{

MOS_STATUS EncodeHevcVdencFeatureManagerXe_Lpm_Plus::CheckFeatures(void* params)
{
    ENCODE_FUNC_CALL();

    EncoderParams *encodeParams = (EncoderParams *)params;

    auto m_basicFeature = dynamic_cast<HevcBasicFeature *>(GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

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

    ENCODE_CHK_STATUS_RETURN(EncodeHevcVdencFeatureManager::CheckFeatures(params));

    return MOS_STATUS_SUCCESS;
}
}

