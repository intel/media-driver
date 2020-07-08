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
//! \file     decode_av1_pipeline.cpp
//! \brief    Defines the interface for av1 decode pipeline
//!
#include "decode_av1_pipeline.h"
#include "decode_utils.h"
#include "media_user_settings_mgr_g12.h"
#include "codechal_setting.h"
#include "decode_av1_feature_manager.h"

namespace decode {

Av1Pipeline::Av1Pipeline(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipeline(hwInterface, debugInterface)
{

}

MOS_STATUS Av1Pipeline::Initialize(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));

    m_cdfCopyPkt = MOS_New(HucCopyPkt, this, m_task, m_hwInterface);
    DECODE_CHK_NULL(m_cdfCopyPkt);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, defaultCdfBufCopyPacketId), m_cdfCopyPkt));
    DECODE_CHK_STATUS(m_cdfCopyPkt->Init());

    auto *codecSettings = (CodechalSetting*)settings;
    DECODE_CHK_NULL(codecSettings);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::Prepare(void *params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(basicFeature);
    DECODE_CHK_STATUS(DecodePipeline::Prepare(params));

    if (basicFeature->m_frameNum == 0)
    {
        for (uint8_t i = 0; i < av1DefaultCdfTableNum; i++)
        {
            HucCopyPkt::HucCopyParams copyParams = {};
            copyParams.srcBuffer  = &(basicFeature->m_tmpCdfBuffers[i]->OsResource);
            copyParams.srcOffset  = 0;
            copyParams.destBuffer = &(basicFeature->m_defaultCdfBuffers[i]->OsResource);
            copyParams.destOffset = 0;
            copyParams.copyLength = basicFeature->m_cdfMaxNumBytes;
            m_cdfCopyPkt->PushCopyParams(copyParams);
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::Uninitialize()
{
    DECODE_FUNC_CALL();

    return DecodePipeline::Uninitialize();
}

MOS_STATUS Av1Pipeline::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());
#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_AV1D_ENABLE_ID_G12, 1, m_osInterface->pOsContext);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1Pipeline::ActivateDecodePackets()
{
    DECODE_FUNC_CALL();

    bool immediateSubmit = true; //Set to true when tile based decoding in use.

    if (m_isFirstTileInFrm)
    {
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, defaultCdfBufCopyPacketId), immediateSubmit, 0, 0));
        m_isFirstTileInFrm = false;
    }

    if (FrameBasedDecodingInUse())
    {
        immediateSubmit = false;
    }

    for (uint8_t curPass = 0; curPass < GetPassNum(); curPass++)
    {
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, av1DecodePacketId), immediateSubmit, curPass, 0));
    }

    return MOS_STATUS_SUCCESS;
}

bool Av1Pipeline::FrameBasedDecodingInUse()
{
    auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));

    bool isframeBasedDecodingUsed = false;
    if (basicFeature != nullptr)
    {
        isframeBasedDecodingUsed = (basicFeature->m_av1PicParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType &
                                   (basicFeature->m_av1PicParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType |
                                    basicFeature->m_av1PicParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType)) &&
                                    basicFeature->m_av1PicParams->m_picInfoFlags.m_fields.m_useSuperres &&
                                    MEDIA_IS_WA(GetWaTable(), Wa_1409820462);
    }
    return isframeBasedDecodingUsed;
}

MOS_STATUS Av1Pipeline::CreateFeatureManager()
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeAv1FeatureManager, m_allocator, m_hwInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

Av1Pipeline::Av1DecodeMode Av1Pipeline::GetDecodeMode()
{
    return m_decodeMode;
}

}
