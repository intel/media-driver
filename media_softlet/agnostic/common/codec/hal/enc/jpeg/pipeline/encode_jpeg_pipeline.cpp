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
//! \file     encode_jpeg_pipeline.cpp
//! \brief    Defines the interface for jpeg encode pipeline
//!

#include "encode_jpeg_pipeline.h"
#include "encode_jpeg_basic_feature.h"
#include "encode_scalability_defs.h"
#include "encode_status_report_defs.h"
#include "encode_jpeg_packet.h"
#include "encode_jpeg_feature_manager.h"

namespace encode {

JpegPipeline::JpegPipeline(
    CodechalHwInterfaceNext *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : EncodePipeline(hwInterface, debugInterface)
{
}

MOS_STATUS JpegPipeline::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();

    CodechalSetting *codecSettings = (CodechalSetting *)settings;
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(m_hwInterface->Initialize(codecSettings));
    ENCODE_CHK_STATUS_RETURN(InitMmcState());

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Initialize(settings));

    CODECHAL_DEBUG_TOOL(
        if (m_debugInterface != nullptr) {
            MOS_Delete(m_debugInterface);
        }
        m_debugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_debugInterface);
        ENCODE_CHK_NULL_RETURN(m_mediaCopyWrapper);
        ENCODE_CHK_STATUS_RETURN(
            m_debugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopyWrapper));

        if (m_statusReportDebugInterface != nullptr) {
            MOS_Delete(m_statusReportDebugInterface);
        }
        m_statusReportDebugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_statusReportDebugInterface);
        ENCODE_CHK_STATUS_RETURN(
            m_statusReportDebugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopyWrapper));
    );

    ENCODE_CHK_STATUS_RETURN(GetSystemVdboxNumber());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::Uninitialize()
{
    ENCODE_FUNC_CALL();
    if (m_mmcState != nullptr)
    {
        MOS_Delete(m_mmcState);
    }

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Uninitialize());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::UserFeatureReport()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::UserFeatureReport());

    ReportUserSetting(
        m_userSettingPtr,
        "JPEG Encode Mode",
        m_codecFunction,
        MediaUserSetting::Group::Sequence);

#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "Enable Encode VE CtxBasedScheduling",
        MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface),
        MediaUserSetting::Group::Sequence);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::Prepare(void *params)
{
    ENCODE_FUNC_CALL();

    EncoderParams *encodeParams = (EncoderParams *)params;
    ENCODE_CHK_NULL_RETURN(encodeParams);

    //TODO: should check with m_codecFunction
    if (encodeParams->ExecCodecFunction != CODECHAL_FUNCTION_ENC_VDENC_PAK && 
        encodeParams->ExecCodecFunction != CODECHAL_FUNCTION_PAK)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::Prepare(params));

    CodecEncodeJpegPictureParams *picParams = static_cast<CodecEncodeJpegPictureParams*>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(picParams);

    auto basicFeature = dynamic_cast<JpegBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    basicFeature->m_bitstreamUpperBound = encodeParams->dwBitstreamSize;

    EncodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(EncodeScalabilityPars));
    scalPars.enableVE           = MOS_VE_SUPPORTED(m_osInterface);
    scalPars.numVdbox           = m_numVdbox;
    scalPars.forceMultiPipe     = false;
    scalPars.outputChromaFormat = basicFeature->m_outputChromaFormat;
    scalPars.numTileRows        = 1;
    scalPars.numTileColumns     = 1;
    scalPars.IsPak              = true;

    ENCODE_CHK_STATUS_RETURN(m_mediaContext->SwitchContext(VdboxEncodeFunc, &scalPars, &m_scalability));

    EncoderStatusParameters inputParameters = {};
    MOS_ZeroMemory(&inputParameters, sizeof(EncoderStatusParameters));

    inputParameters.statusReportFeedbackNumber = picParams->m_statusReportFeedbackNumber;
    inputParameters.codecFunction              = encodeParams->ExecCodecFunction;
    inputParameters.currRefList                = basicFeature->m_ref->GetCurrRefList();
    inputParameters.picWidthInMb               = basicFeature->m_picWidthInMb;
    inputParameters.frameFieldHeightInMb       = basicFeature->m_frameFieldHeightInMb;
    inputParameters.currOriginalPic            = basicFeature->m_currOriginalPic;
    inputParameters.pictureCodingType          = basicFeature->m_pictureCodingType;
    inputParameters.numUsedVdbox               = m_numVdbox;
    inputParameters.hwWalker                   = false;
    inputParameters.maxNumSlicesAllowed        = 1;
    inputParameters.numberTilesInFrame         = 0;

    ENCODE_CHK_STATUS_RETURN(m_statusReport->Init(&inputParameters));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::Execute()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(ActivateVideoPackets());
    ENCODE_CHK_STATUS_RETURN(ExecuteActivePackets());
    ENCODE_CHK_STATUS_RETURN(ResetParams());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::GetStatusReport(void *status, uint16_t numStatus)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(m_statusReport->GetReport(numStatus, status));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::Destroy()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Uninitialize());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::Init(void *settings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(settings);

    ENCODE_CHK_STATUS_RETURN(Initialize(settings));

    MediaTask *task = CreateTask(MediaTask::TaskType::cmdTask);
    ENCODE_CHK_NULL_RETURN(task);

    JpegPkt *jpegPkt = MOS_New(JpegPkt, this, task, m_hwInterface);
    ENCODE_CHK_STATUS_RETURN(RegisterPacket(baseJpegPacket, jpegPkt));
    ENCODE_CHK_STATUS_RETURN(jpegPkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::ActivateVideoPackets()
{
    ENCODE_FUNC_CALL();

    bool immediateSubmit = !m_singleTaskPhaseSupported;

    ENCODE_CHK_STATUS_RETURN(ActivatePacket(baseJpegPacket, immediateSubmit, 0, 0));

    // Last element in m_activePacketList must be immediately submitted
    m_activePacketList.back().immediateSubmit = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::CreateBufferTracker()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::CreateStatusReport()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::CreateFeatureManager()
{
    ENCODE_FUNC_CALL();

    m_featureManager = MOS_New(EncodeJpegFeatureManager, m_allocator, m_hwInterface, m_trackedBuf, m_recycleBuf);
    ENCODE_CHK_NULL_RETURN(m_featureManager);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::ResetParams()
{
    ENCODE_FUNC_CALL();

    auto feature = dynamic_cast<EncodeBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(feature);

    // Only update user features for first frame.
    if (feature->m_frameNum == 0)
    {
        ENCODE_CHK_STATUS_RETURN(UserFeatureReport());
    }

    feature->m_frameNum++;

    ENCODE_CHK_STATUS_RETURN(m_statusReport->Reset());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegPipeline::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_mmcState = MOS_New(EncodeMemComp, m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

}
