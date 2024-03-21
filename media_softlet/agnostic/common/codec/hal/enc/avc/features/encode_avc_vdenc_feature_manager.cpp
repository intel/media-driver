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
//! \file     encode_avc_vdenc_feature_manager.cpp
//! \brief    Defines the common interface for avc vdenc feature manager
//! \details  The encode feature manager is further sub-divided by codec type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_avc_vdenc_feature_manager.h"
#include "encode_avc_brc.h"
#include "media_avc_feature_defs.h"
#include "encode_avc_vdenc_const_settings.h"

namespace encode
{

#define VDENC_AVC_CQP_NUM_OF_PASSES                1    // No standalone PAK IPCM pass for VDENC

MOS_STATUS EncodeAvcVdencFeatureManager::CheckFeatures(void *params)
{
    ENCODE_FUNC_CALL();
    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams =
        static_cast<PCODEC_AVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(avcSeqParams);
    PCODEC_AVC_ENCODE_PIC_PARAMS avcPicParams =
        static_cast<PCODEC_AVC_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(avcPicParams);

    auto settings = static_cast<EncodeAvcVdencConstSettings *>(m_featureConstSettings);
    ENCODE_CHK_NULL_RETURN(settings);
    ENCODE_CHK_STATUS_RETURN(settings->Update(params));

    if (encodeParams->bNewSeq)
    {
        m_ddiTargetUsage = avcSeqParams->TargetUsage;
        ENCODE_CHK_STATUS_RETURN(MapTargetUsage(avcSeqParams->TargetUsage));
        m_targetUsage = avcSeqParams->TargetUsage;
    }

    ENCODE_CHK_STATUS_RETURN(ValidatePassNum(avcSeqParams, avcPicParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAvcVdencFeatureManager::MapTargetUsage(uint8_t &targetUsage)
{
    ENCODE_FUNC_CALL();

    switch (targetUsage)
    {
    case 1: case 2:
        targetUsage = 1;
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

MOS_STATUS EncodeAvcVdencFeatureManager::ValidatePassNum(
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams,
    PCODEC_AVC_ENCODE_PIC_PARAMS avcPicParams)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(avcSeqParams);
    ENCODE_CHK_NULL_RETURN(avcPicParams);

    auto brcFeature = dynamic_cast<AvcEncodeBRC *>(GetFeature(AvcFeatureIDs::avcBrcFeature));
    ENCODE_CHK_NULL_RETURN(brcFeature);

    if (brcFeature->IsVdencBrcSupported(avcSeqParams))
    {
        m_passNum = CODECHAL_VDENC_BRC_NUM_OF_PASSES;  // 2-pass VDEnc BRC

        if (avcPicParams->BRCPrecision == 1)
        {
            brcFeature->EnableVdencSinglePass();
            m_passNum = 1;
        }
    }
    else
    {
        // Single PAK pass
        m_passNum = VDENC_AVC_CQP_NUM_OF_PASSES;
    }

    return MOS_STATUS_SUCCESS;
}

}
