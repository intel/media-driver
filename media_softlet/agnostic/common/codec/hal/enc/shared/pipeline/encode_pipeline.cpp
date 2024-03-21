/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     encode_pipeline.cpp
//! \brief    Defines the common interface for encode pipeline
//! \details  The encode pipeline interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!
#include "encode_pipeline.h"
#include "encode_utils.h"
#include "media_packet.h"

#include "codechal_setting.h"
#include "encode_status_report_defs.h"
#include "encode_status_report.h"
#include "mos_solo_generic.h"

namespace encode {
EncodePipeline::EncodePipeline(
    CodechalHwInterfaceNext *hwInterface,
    CodechalDebugInterface *debugInterface):
      MediaPipeline(hwInterface ? hwInterface->GetOsInterface() : nullptr),
      m_hwInterface(hwInterface)
{

}

MOS_STATUS EncodePipeline::Initialize(void *settings)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(InitUserSetting(m_userSettingPtr));
    ENCODE_CHK_STATUS_RETURN(MediaPipeline::InitPlatform());
    ENCODE_CHK_STATUS_RETURN(MediaPipeline::CreateMediaCopyWrapper());
    ENCODE_CHK_NULL_RETURN(m_mediaCopyWrapper);

    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    ENCODE_CHK_NULL_RETURN(m_hwInterface->GetOsInterface());

    m_osInterface = m_hwInterface->GetOsInterface();
    ENCODE_CHK_NULL_RETURN(m_osInterface);

    if (m_mediaCopyWrapper->MediaCopyStateIsNull())
    {
        m_mediaCopyWrapper->SetMediaCopyState(m_hwInterface->CreateMediaCopy(m_osInterface));
    }

    m_mediaContext = MOS_New(MediaContext, scalabilityEncoder, m_hwInterface, m_osInterface);
    ENCODE_CHK_NULL_RETURN(m_mediaContext);

    m_allocator = MOS_New(EncodeAllocator, m_osInterface);
    ENCODE_CHK_NULL_RETURN(m_allocator);

    m_trackedBuf = MOS_New(TrackedBuffer, m_allocator, (uint8_t)CODEC_NUM_REF_BUFFERS, (uint8_t)CODEC_NUM_NON_REF_BUFFERS);
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);

    m_recycleBuf = MOS_New(RecycleResource, m_allocator);
    ENCODE_CHK_NULL_RETURN(m_recycleBuf);

    CodechalSetting *codecSettings = (CodechalSetting*)settings;
    m_standard = codecSettings->standard;
    m_mode = codecSettings->mode;
    m_codecFunction = codecSettings->codecFunction;

    CODECHAL_DEBUG_TOOL(
        m_debugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_debugInterface);
        ENCODE_CHK_STATUS_RETURN(
            m_debugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopyWrapper)
        );

        m_statusReportDebugInterface = MOS_New(CodechalDebugInterface);
        ENCODE_CHK_NULL_RETURN(m_statusReportDebugInterface);
        ENCODE_CHK_STATUS_RETURN(
        m_statusReportDebugInterface->Initialize(m_hwInterface, m_codecFunction, m_mediaCopyWrapper));
    );

    MediaUserSetting::Value outValue;
    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "Single Task Phase Enable",
        MediaUserSetting::Group::Sequence);
    m_singleTaskPhaseSupported = outValue.Get<bool>();

    ENCODE_CHK_STATUS_RETURN(CreateFeatureManager());
    ENCODE_CHK_NULL_RETURN(m_featureManager);

    m_encodecp = MOS_New(EncodeCp, m_hwInterface);
    m_encodecp->RegisterParams(codecSettings);
    bool cpenable = m_encodecp->isCpEnabled();

    ENCODE_CHK_STATUS_RETURN(m_featureManager->Init(codecSettings));

    m_packetUtilities = MOS_New(PacketUtilities, m_hwInterface, m_featureManager);
    ENCODE_CHK_NULL_RETURN(m_packetUtilities);
    ENCODE_CHK_STATUS_RETURN(m_packetUtilities->Init());

    m_statusReport = MOS_New(EncoderStatusReport, m_allocator, m_osInterface, true, true, cpenable);
    ENCODE_CHK_NULL_RETURN(m_statusReport);
    ENCODE_CHK_STATUS_RETURN(m_statusReport->Create());

    m_encodecp->setStatusReport(m_statusReport);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePipeline::Uninitialize()
{
    ENCODE_FUNC_CALL();

    MOS_Delete(m_mediaContext);

    MOS_Delete(m_encodecp);

    MOS_Delete(m_statusReport);

    CODECHAL_DEBUG_TOOL(
        MOS_Delete(m_debugInterface);
        MOS_Delete(m_statusReportDebugInterface);
    );

    MOS_Delete(m_trackedBuf);

    MOS_Delete(m_recycleBuf);

    if (m_featureManager != nullptr)
    {
        m_featureManager->Destroy();
        MOS_Delete(m_featureManager);
    }

    // Allocator should not be destroyed until all resources released.
    if (m_allocator != nullptr)
    {
        m_allocator->DestroyAllResources();
        MOS_Delete(m_allocator);
    }

    MOS_Delete(m_packetUtilities);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePipeline::UserFeatureReport()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(MediaPipeline::UserFeatureReport());

    // Encode HW Walker Reporting
//    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_ENCODE_HW_WALKER_ID, m_hwWalker);
//    if (m_hwWalker)
//    {
//        // Encode HW Walker m_mode Reporting
//#if (_DEBUG || _RELEASE_INTERNAL)
//        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_ENCODE_HW_WALKER_MODE_ID, m_walkerMode);
//#endif // _DEBUG || _RELEASE_INTERNAL
//    }
//
//    if (MEDIA_IS_SKU(m_skuTable, FtrSliceShutdown))
//    {
//        // SliceShutdownEnable Reporting
//        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_ENABLE_ID, m_sliceShutdownEnable);
//    }
#if (_DEBUG || _RELEASE_INTERNAL)
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "Media Encode Used VDBOX Number",
        GetPipeNum(),
        MediaUserSetting::Group::Sequence);

    ReportUserSettingForDebug(
        m_userSettingPtr,
        "Media Encode DDI TargetUsage",
        GetDDITU(),
        MediaUserSetting::Group::Sequence);
#endif // _DEBUG || _RELEASE_INTERNAL

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePipeline::Prepare(void *params)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_NULL_RETURN(m_hwInterface);

    ENCODE_CHK_NULL_RETURN(m_featureManager);
    ENCODE_CHK_STATUS_RETURN(m_featureManager->CheckFeatures(params));
    ENCODE_CHK_STATUS_RETURN(m_featureManager->Update(params));
    m_encodecp->UpdateParams(true);

    ENCODE_CHK_STATUS_RETURN(WaitForBatchBufferComplete());

    EncoderParams *encodeParams = (EncoderParams *)params;
    ENCODE_CHK_STATUS_RETURN(Mos_Solo_SetGpuAppTaskEvent(m_osInterface, encodeParams->gpuAppTaskEvent));

    m_osInterface->pfnIncPerfFrameID(m_osInterface);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePipeline::ContextSwitchBack()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_scalPars);

    m_scalPars->IsContextSwitchBack = true;
    m_mediaContext->SwitchContext(VdboxEncodeFunc, m_scalPars.get(), &m_scalability);
    m_scalPars->IsContextSwitchBack = false;

    ENCODE_CHK_NULL_RETURN(m_scalability);

    m_scalability->SetPassNumber(m_featureManager->GetNumPass());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePipeline::GetSystemVdboxNumber()
{
    MEDIA_SYSTEM_INFO *gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    MEDIA_ENGINE_INFO  mediaSysInfo;
    MOS_ZeroMemory(&mediaSysInfo, sizeof(MEDIA_ENGINE_INFO));
    MOS_STATUS eStatus = m_osInterface->pfnGetMediaEngineInfo(m_osInterface, mediaSysInfo);

    if (eStatus == MOS_STATUS_SUCCESS && (!MEDIA_IS_SKU(m_skuTable, FtrWithSlimVdbox)))
    {
        // Both VE mode and media solo mode should be able to get the VDBOX number via the same interface
        m_numVdbox = (uint8_t)(mediaSysInfo.VDBoxInfo.NumberOfVDBoxEnabled);
    }
    else
    {
        m_numVdbox = 1;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePipeline::WaitForBatchBufferComplete()
{
    ENCODE_CHK_NULL_RETURN(m_statusReport);

    const uint32_t completedFrames = m_statusReport->GetCompletedCount();

    if (!m_hwInterface->IsSimActive() &&
        m_recycledBufStatusNum[m_currRecycledBufIdx] > completedFrames)
    {
        uint32_t waitMs;

        // Wait for Batch Buffer complete event OR timeout
        for (waitMs = MHW_TIMEOUT_MS_DEFAULT; waitMs > 0; waitMs -= MHW_EVENT_TIMEOUT_MS)
        {
            if (m_recycledBufStatusNum[m_currRecycledBufIdx] <= completedFrames)
            {
                break;
            }

            MosUtilities::MosSleep(MHW_EVENT_TIMEOUT_MS);
        }

        ENCODE_VERBOSEMESSAGE("Waited for %d ms", (MHW_TIMEOUT_MS_DEFAULT - waitMs));

        if (m_recycledBufStatusNum[m_currRecycledBufIdx] > completedFrames)
        {
            ENCODE_ASSERTMESSAGE("No recycled buffers available, wait timed out at %d ms!", MHW_TIMEOUT_MS_DEFAULT);
            ENCODE_ASSERTMESSAGE("m_storeData = %d, m_recycledBufStatusNum[%d] = %d, data = %d",
                m_statusReport->GetSubmittedCount(),
                m_currRecycledBufIdx,
                m_recycledBufStatusNum[m_currRecycledBufIdx],
                completedFrames);
            return MOS_STATUS_CLIENT_AR_NO_SPACE;
        }
    }

    m_recycledBufStatusNum[m_currRecycledBufIdx] = m_statusReport->GetSubmittedCount();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePipeline::ExecuteActivePackets()
{
    ENCODE_FUNC_CALL();
    MOS_TraceEventExt(EVENT_PIPE_EXE, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    for (auto prop : m_activePacketList)
    {
        prop.stateProperty.singleTaskPhaseSupported = m_singleTaskPhaseSupported;
        prop.stateProperty.statusReport = m_statusReport;
        MOS_TraceEventExt(EVENT_PIPE_EXE, EVENT_TYPE_INFO, &prop.packetId, sizeof(uint32_t), nullptr, 0);

        MediaTask *task = prop.packet->GetActiveTask();
        ENCODE_CHK_STATUS_RETURN(task->AddPacket(&prop));
        if (prop.immediateSubmit)
        {
            ENCODE_CHK_STATUS_RETURN(task->Submit(true, m_scalability, m_debugInterface));
        }
    }

    m_activePacketList.clear();
    MOS_TraceEventExt(EVENT_PIPE_EXE, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePipeline::ExecuteResolveMetaData(PMOS_RESOURCE pInput, PMOS_RESOURCE pOutput)
{
    ENCODE_FUNC_CALL();
    MOS_COMMAND_BUFFER cmdBuffer;
    MOS_ZeroMemory(&cmdBuffer, sizeof(cmdBuffer));

    ENCODE_CHK_NULL_RETURN(m_scalability);
    ENCODE_CHK_STATUS_RETURN(m_scalability->GetCmdBuffer(&cmdBuffer));

    auto basicFeature = dynamic_cast<EncodeBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);
    uint32_t bufSize = basicFeature->m_metaDataOffset.dwMetaDataSize + basicFeature->m_numSlices * basicFeature->m_metaDataOffset.dwMetaDataSubRegionSize +
                       basicFeature->m_metaDataOffset.dwTilePartitionSize + basicFeature->m_metaDataOffset.dwPostFeatueSize;
    m_packetUtilities->AddMemCopyCmd(&cmdBuffer, pOutput, pInput, bufSize);
    ENCODE_CHK_STATUS_RETURN(m_scalability->ReturnCmdBuffer(&cmdBuffer));
    ENCODE_CHK_STATUS_RETURN(m_scalability->SubmitCmdBuffer(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePipeline::ReportErrorFlag(PMOS_RESOURCE pMetadataBuffer,
    uint32_t size, uint32_t offset, uint32_t flag)
{
    ENCODE_FUNC_CALL();
    MOS_COMMAND_BUFFER cmdBuffer;
    MOS_ZeroMemory(&cmdBuffer, sizeof(cmdBuffer));

    ENCODE_CHK_NULL_RETURN(m_scalability);
    ENCODE_CHK_STATUS_RETURN(m_scalability->GetCmdBuffer(&cmdBuffer));

    auto basicFeature = dynamic_cast<EncodeBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(basicFeature);

    basicFeature->m_metaDataOffset.dwMetaDataSize = size;
    m_packetUtilities->AddStoreDataImmCmd(&cmdBuffer, pMetadataBuffer, offset, flag);
    ENCODE_CHK_STATUS_RETURN(m_scalability->ReturnCmdBuffer(&cmdBuffer));
    ENCODE_CHK_STATUS_RETURN(m_scalability->SubmitCmdBuffer(&cmdBuffer));
    return MOS_STATUS_SUCCESS;
}

void EncodePipeline::SetFrameTrackingForMultiTaskPhase()
{
    if (!IsSingleTaskPhaseSupported())
    {
        for_each(m_activePacketList.begin(), m_activePacketList.end() - 1, [&](PacketProperty &blk) { blk.frameTrackingRequested = false; });
    }
}

}
