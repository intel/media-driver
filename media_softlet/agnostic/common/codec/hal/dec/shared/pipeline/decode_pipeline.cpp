/*
* Copyright (c) 2018-2023, Intel Corporation
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
//! \file     decode_pipeline.cpp
//! \brief    Defines the common interface for decode pipeline
//! \details  The decode pipeline interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!
#include "decode_pipeline.h"
#include "decode_utils.h"
#include "decode_status_report.h"
#include "media_packet.h"
#include "decode_predication_packet.h"
#include "decode_marker_packet.h"
#include "decode_downsampling_packet.h"
#include "codechal_setting.h"
#include "decode_basic_feature.h"
#include "mos_solo_generic.h"
#include "decode_sfc_histogram_postsubpipeline.h"
#include "decode_common_feature_defs.h"
#include "decode_resource_auto_lock.h"

namespace decode {

DecodePipeline::DecodePipeline(
    CodechalHwInterfaceNext *hwInterface,
    CodechalDebugInterface *debugInterface):
    MediaPipeline(hwInterface ? hwInterface->GetOsInterface() : nullptr)
{
    DECODE_FUNC_CALL();

    DECODE_ASSERT(hwInterface != nullptr);
    m_hwInterface = hwInterface;
    MOS_STATUS m_status = (InitUserSetting(m_userSettingPtr));

    m_singleTaskPhaseSupported =
        ReadUserFeature(m_userSettingPtr, "Decode Single Task Phase Enable", MediaUserSetting::Group::Sequence).Get<bool>();

    m_pCodechalOcaDumper = MOS_New(CodechalOcaDumper);
    if (!m_pCodechalOcaDumper)
    {
        MOS_OS_ASSERTMESSAGE("Initialize CodechalOcaDumper failed!");
    }
}

MOS_STATUS DecodePipeline::CreateStatusReport()
{
    m_statusReport = MOS_New(DecodeStatusReport, m_allocator, true, m_osInterface);
    DECODE_CHK_NULL(m_statusReport);
    DECODE_CHK_STATUS(m_statusReport->Create());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePipeline::CreatePreSubPipeLines(DecodeSubPipelineManager &subPipelineManager)
{
    m_bitstream = MOS_New(DecodeInputBitstream, this, m_task, m_numVdbox);
    DECODE_CHK_NULL(m_bitstream);
    DECODE_CHK_STATUS(subPipelineManager.Register(*m_bitstream));

    m_streamout = MOS_New(DecodeStreamOut, this, m_task, m_numVdbox);
    DECODE_CHK_NULL(m_streamout);
    DECODE_CHK_STATUS(subPipelineManager.Register(*m_streamout));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePipeline::CreatePostSubPipeLines(DecodeSubPipelineManager &subPipelineManager)
{
    DECODE_FUNC_CALL();

#ifdef _DECODE_PROCESSING_SUPPORTED
    auto sfcHistogramPostSubPipeline = MOS_New(DecodeSfcHistogramSubPipeline, this, m_task, m_numVdbox);
    DECODE_CHK_NULL(sfcHistogramPostSubPipeline);
    DECODE_CHK_STATUS(m_postSubPipeline->Register(*sfcHistogramPostSubPipeline));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePipeline::CreateSubPipeLineManager(CodechalSetting* codecSettings)
{
    m_preSubPipeline = MOS_New(DecodeSubPipelineManager, *this);
    DECODE_CHK_NULL(m_preSubPipeline);
    DECODE_CHK_STATUS(CreatePreSubPipeLines(*m_preSubPipeline));
    DECODE_CHK_STATUS(m_preSubPipeline->Init(*codecSettings));

    m_postSubPipeline = MOS_New(DecodeSubPipelineManager, *this);
    DECODE_CHK_NULL(m_postSubPipeline);
    DECODE_CHK_STATUS(CreatePostSubPipeLines(*m_postSubPipeline));
    DECODE_CHK_STATUS(m_postSubPipeline->Init(*codecSettings));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePipeline::CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings)
{
    DecodePredicationPkt *predicationPkt = MOS_New(DecodePredicationPkt, this, m_hwInterface);
    DECODE_CHK_NULL(predicationPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, predicationSubPacketId), *predicationPkt));

    DecodeMarkerPkt *markerPkt = MOS_New(DecodeMarkerPkt, this, m_hwInterface);
    DECODE_CHK_NULL(markerPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, markerSubPacketId), *markerPkt));
    return MOS_STATUS_SUCCESS;
}

DecodeSubPacket* DecodePipeline::GetSubPacket(uint32_t subPacketId)
{
    return m_subPacketManager->GetSubPacket(subPacketId);
}

MOS_STATUS DecodePipeline::CreateSubPacketManager(CodechalSetting* codecSettings)
{
    DECODE_CHK_NULL(codecSettings);
    m_subPacketManager = MOS_New(DecodeSubPacketManager);
    DECODE_CHK_NULL(m_subPacketManager);
    DECODE_CHK_STATUS(CreateSubPackets(*m_subPacketManager, *codecSettings));
    DECODE_CHK_STATUS(m_subPacketManager->Init());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePipeline::Initialize(void *settings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(settings);

    DECODE_CHK_STATUS(MediaPipeline::InitPlatform());
    DECODE_CHK_STATUS(MediaPipeline::CreateMediaCopyWrapper());
    DECODE_CHK_NULL(m_mediaCopyWrapper);

    DECODE_CHK_NULL(m_waTable);

    auto *codecSettings = (CodechalSetting*)settings;
    DECODE_CHK_NULL(m_hwInterface);
    DECODE_CHK_STATUS(m_hwInterface->Initialize(codecSettings));

    if (m_mediaCopyWrapper->MediaCopyStateIsNull())
    {
        m_mediaCopyWrapper->SetMediaCopyState(m_hwInterface->CreateMediaCopy(m_osInterface));
    }

    CODECHAL_DEBUG_TOOL(
        m_debugInterface = MOS_New(CodechalDebugInterface);
        DECODE_CHK_NULL(m_debugInterface);
        DECODE_CHK_STATUS(
            m_debugInterface->Initialize(m_hwInterface, codecSettings->codecFunction, m_mediaCopyWrapper));
    );

    m_mediaContext = MOS_New(MediaContext, scalabilityDecoder, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_mediaContext);

    m_task = CreateTask(MediaTask::TaskType::cmdTask);
    DECODE_CHK_NULL(m_task);

    m_numVdbox = GetSystemVdboxNumber();

    bool limitedLMemBar = MEDIA_IS_SKU(m_skuTable, FtrLimitedLMemBar) ? true : false;
    m_allocator = MOS_New(DecodeAllocator, m_osInterface, limitedLMemBar);
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(CreateStatusReport());

    m_decodecp = Create_DecodeCpInterface(codecSettings, m_hwInterface->GetCpInterface(), m_hwInterface->GetOsInterface());
    if (m_decodecp)
    {
        DECODE_CHK_STATUS(m_decodecp->RegisterParams(codecSettings));
    }
    DECODE_CHK_STATUS(CreateFeatureManager());
    DECODE_CHK_STATUS(m_featureManager->Init(codecSettings));

    DECODE_CHK_STATUS(CreateSubPipeLineManager(codecSettings));
    DECODE_CHK_STATUS(CreateSubPacketManager(codecSettings));

#if (_DEBUG || _RELEASE_INTERNAL)
    m_delayMiliseconds = ReadUserFeature(m_userSettingPtr, "Delay Miliseconds", MediaUserSetting::Group::Sequence).Get<uint32_t>();
#endif

#if MHW_HWCMDPARSER_ENABLED
    mhw::HwcmdParser::InitInstance(m_osInterface, mhw::HwcmdParser::AddOnMode::NoAddOn);
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePipeline::Uninitialize()
{
    DECODE_FUNC_CALL();

    // Wait all cmd completion before delete resource.
    m_osInterface->pfnWaitAllCmdCompletion(m_osInterface);

    Delete_DecodeCpInterface(m_decodecp);
    m_decodecp = nullptr;

    MOS_Delete(m_mediaContext);

    MOS_Delete(m_statusReport);

    MOS_Delete(m_featureManager);

    MOS_Delete(m_preSubPipeline);
    MOS_Delete(m_postSubPipeline);
    MOS_Delete(m_subPacketManager);

    MOS_Delete(m_allocator);

    CODECHAL_DEBUG_TOOL(MOS_Delete(m_debugInterface););

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePipeline::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    return MediaPipeline::UserFeatureReport();
}

bool DecodePipeline::IsFirstProcessPipe(const DecodePipelineParams& pipelineParams)
{
    if (pipelineParams.m_pipeMode != decodePipeModeProcess)
    {
        return false;
    }

    CodechalDecodeParams *decodeParams = pipelineParams.m_params;
    if (decodeParams == nullptr)
    {
        return false;
    }

    return (decodeParams->m_executeCallIndex == 0);
}

uint8_t DecodePipeline::GetSystemVdboxNumber()
{
    uint8_t numVdbox = 1;

    MEDIA_ENGINE_INFO mediaSysInfo;
    MOS_ZeroMemory(&mediaSysInfo, sizeof(MEDIA_ENGINE_INFO));
    MOS_STATUS eStatus = m_osInterface->pfnGetMediaEngineInfo(m_osInterface, mediaSysInfo);
    if (eStatus == MOS_STATUS_SUCCESS)
    {
        numVdbox = (uint8_t)(mediaSysInfo.VDBoxInfo.NumberOfVDBoxEnabled);
    }
    else
    {
        DECODE_ASSERTMESSAGE("Failed to query media engine info!!");
    }

    return numVdbox;
}

MOS_STATUS DecodePipeline::Prepare(void *params)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(params);
    DecodePipelineParams *pipelineParams = (DecodePipelineParams *)params;
    CodechalDecodeParams *decodeParams = pipelineParams->m_params;
    DECODE_CHK_NULL(decodeParams);

    DECODE_CHK_STATUS(m_task->Clear());
    m_activePacketList.clear();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_STATUS(m_featureManager->CheckFeatures(decodeParams));
    DECODE_CHK_STATUS(m_featureManager->Update(decodeParams));
    if (m_decodecp)
    {
        DECODE_CHK_STATUS(m_decodecp->UpdateParams(true));
    }
    DECODE_CHK_STATUS(m_subPacketManager->Prepare());

    DECODE_CHK_STATUS(Mos_Solo_SetGpuAppTaskEvent(m_osInterface, decodeParams->m_gpuAppTaskEvent));

#if MHW_HWCMDPARSER_ENABLED
    auto instance = mhw::HwcmdParser::GetInstance();
    if (instance)
    {
        // use I as default frame type
        instance->Update('I', (void *)m_featureManager, mhw::HwcmdParser::WorkloadType::Decode);
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePipeline::ExecuteActivePackets()
{
    DECODE_FUNC_CALL();
    MOS_TraceEventExt(EVENT_PIPE_EXE, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    // Last element in m_activePacketList must be immediately submitted
    m_activePacketList.back().immediateSubmit = true;

    for (PacketProperty prop : m_activePacketList)
    {
        prop.stateProperty.singleTaskPhaseSupported = m_singleTaskPhaseSupported;
        prop.stateProperty.statusReport = m_statusReport;
        MOS_TraceEventExt(EVENT_PIPE_EXE, EVENT_TYPE_INFO, &prop.packetId, sizeof(uint32_t), nullptr, 0);

        MediaTask *task = prop.packet->GetActiveTask();
        DECODE_CHK_STATUS(task->AddPacket(&prop));
        if (prop.immediateSubmit)
        {
            DECODE_CHK_STATUS(task->Submit(true, m_scalability, m_debugInterface));
        }
    }

    m_activePacketList.clear();
    MOS_TraceEventExt(EVENT_PIPE_EXE, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return MOS_STATUS_SUCCESS;
}

bool DecodePipeline::IsCompleteBitstream()
{
    return (m_bitstream == nullptr) ? false : m_bitstream->IsComplete();
}

#ifdef _DECODE_PROCESSING_SUPPORTED
bool DecodePipeline::IsDownSamplingSupported()
{
    DECODE_ASSERT(m_subPacketManager != nullptr);

    DecodeDownSamplingPkt *downSamplingPkt = dynamic_cast<DecodeDownSamplingPkt *>(
        GetSubPacket(DecodePacketId(this, downSamplingSubPacketId)));
    if (downSamplingPkt == nullptr)
    {
        return false;
    }

    return downSamplingPkt->IsSupported();
}
#endif

MOS_SURFACE* DecodePipeline::GetDummyReference()
{
    auto* feature = dynamic_cast<DecodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    return (feature != nullptr) ? &(feature->m_dummyReference) : nullptr;
}

CODECHAL_DUMMY_REFERENCE_STATUS DecodePipeline::GetDummyReferenceStatus()
{
    auto* feature = dynamic_cast<DecodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    return (feature != nullptr) ? feature->m_dummyReferenceStatus : CODECHAL_DUMMY_REFERENCE_INVALID;
}

void DecodePipeline::SetDummyReferenceStatus(CODECHAL_DUMMY_REFERENCE_STATUS status)
{
    auto* feature = dynamic_cast<DecodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    if (feature != nullptr)
    {
        feature->m_dummyReferenceStatus = status;
    }
}

#if USE_CODECHAL_DEBUG_TOOL
#ifdef _DECODE_PROCESSING_SUPPORTED
MOS_STATUS DecodePipeline::DumpDownSamplingParams(DecodeDownSamplingFeature &downSamplingParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDecodeProcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    if(!downSamplingParams.IsEnabled())
    {
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(downSamplingParams.m_inputSurface);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "Input Surface Resolution: "
        << +downSamplingParams.m_inputSurface->dwWidth << " x " << +downSamplingParams.m_inputSurface->dwHeight << std::endl;
    oss << "Input Region Resolution: "
        << +downSamplingParams.m_inputSurfaceRegion.m_width << " x " << +downSamplingParams.m_inputSurfaceRegion.m_height << std::endl;
    oss << "Input Region Offset: ("
        << +downSamplingParams.m_inputSurfaceRegion.m_x << "," << +downSamplingParams.m_inputSurfaceRegion.m_y << ")" << std::endl;
    oss << "Input Surface Format: "
        << (downSamplingParams.m_inputSurface->Format == Format_NV12 ? "NV12" : "P010" )<< std::endl;
    oss << "Output Surface Resolution: "
        << +downSamplingParams.m_outputSurface.dwWidth << " x " << +downSamplingParams.m_outputSurface.dwHeight << std::endl;
    oss << "Output Region Resolution: "
        << +downSamplingParams.m_outputSurfaceRegion.m_width << " x " << +downSamplingParams.m_outputSurfaceRegion.m_height << std::endl;
    oss << "Output Region Offset: ("
        << +downSamplingParams.m_outputSurfaceRegion.m_x << ", " << +downSamplingParams.m_outputSurfaceRegion.m_y << ")" << std::endl;
    oss << "Output Surface Format: "
        << (downSamplingParams.m_outputSurface.Format == Format_NV12 ? "NV12" : "YUY2" )<< std::endl;

    const char* filePath = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufDecProcParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(filePath, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS DecodePipeline::DumpBitstream(PMOS_RESOURCE pBitstream, uint32_t size, uint32_t offset, const char* attrName)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(pBitstream);

    if(attrName == nullptr)
    {
        attrName = CodechalDbgAttr::attrDecodeBitstream;
    }

    DECODE_CHK_STATUS(m_debugInterface->DumpBuffer(
        pBitstream, 
        attrName,
        "_DEC", 
        size, 
        offset, 
        CODECHAL_NUM_MEDIA_STATES));

    if (MOS_TraceKeyEnabled(TR_KEY_DECODE_BITSTREAM_INFO))
    {
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly = 1;
        
        uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, pBitstream, &lockFlags);
        MEDIA_DEBUG_CHK_NULL(data);
        data += offset;

        DECODE_EVENTDATA_BITSTREAM eventData;
        eventData.BitstreamSize = size;
        for (uint32_t i = 0; i < MOS_MIN(BITSTREAM_INFO_SIZE, size); i++)
        {
            eventData.Data[i] = data[i];
        }
        MOS_TraceEvent(EVENT_DECODE_INFO_BITSTREAM, EVENT_TYPE_INFO, &eventData, sizeof(eventData));

        m_osInterface->pfnUnlockResource(m_osInterface, pBitstream);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePipeline::DelayForDumpOutput()
{
    DECODE_FUNC_CALL();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDelayForDumpOutput))
    {
        uint32_t completedCount = m_statusReport->GetCompletedCount();
        for (uint8_t i = 0; i < 20; i++)
        {
            if (completedCount <= m_statusCheckCount)
            {
                MosUtilities::MosSleep(5);
                completedCount = m_statusReport->GetCompletedCount();
            }
            else
            {
                break;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodePipeline::DumpOutput(const DecodeStatusReportData& reportData)
{
    DECODE_FUNC_CALL();

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDecodeOutputSurface))
    {
        MOS_SURFACE dstSurface;
        MOS_ZeroMemory(&dstSurface, sizeof(dstSurface));
        dstSurface.Format     = Format_NV12;
        dstSurface.OsResource = reportData.currDecodedPicRes;
        DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&dstSurface));

        DECODE_CHK_STATUS(m_debugInterface->DumpYUVSurface(
            &dstSurface, CodechalDbgAttr::attrDecodeOutputSurface, "DstSurf"));
    }

    if(m_streamout != nullptr)
    {
        m_streamout->DumpOutput(reportData);
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    DecodeDownSamplingFeature* downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature*>(
        m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
    if (downSamplingFeature != nullptr && downSamplingFeature->IsEnabled())
    {
        if (reportData.currSfcOutputSurface != nullptr &&
            m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSfcOutputSurface))
        {
            MOS_SURFACE &sfcDstSurface = *reportData.currSfcOutputSurface;

            if (!Mos_ResourceIsNull(&sfcDstSurface.OsResource))
            {
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&sfcDstSurface));

                //rgb format read from reg key
                uint32_t sfcOutputRgbFormatFlag =
                    ReadUserFeature(m_userSettingPtr, "Decode SFC RGB Format Output", MediaUserSetting::Group::Sequence).Get<uint32_t>();
                if (sfcOutputRgbFormatFlag)
                {
                    DECODE_CHK_STATUS(m_debugInterface->DumpRgbDataOnYUVSurface(
                        &sfcDstSurface, CodechalDbgAttr::attrSfcOutputSurface, "SfcDstRgbSurf"));
                }
                else
                {
                    DECODE_CHK_STATUS(m_debugInterface->DumpYUVSurface(
                        &sfcDstSurface, CodechalDbgAttr::attrSfcOutputSurface, "SfcDstSurf"));
                }
            }
        }

        if (reportData.currHistogramOutBuf != nullptr &&
            !Mos_ResourceIsNull(reportData.currHistogramOutBuf))
        {
            DECODE_CHK_STATUS(m_debugInterface->DumpBuffer(
                reportData.currHistogramOutBuf,
                CodechalDbgAttr::attrSfcHistogram,
                "_DEC",
                HISTOGRAM_BINCOUNT * downSamplingFeature->m_histogramBinWidth));
        }
    }
#endif

    return MOS_STATUS_SUCCESS;
}
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS DecodePipeline::ReportVdboxIds(const DecodeStatusMfx& status)
{
    DECODE_FUNC_CALL();

    // report the VDBOX IDs to user feature
    uint32_t vdboxIds = ReadUserFeature(m_userSettingPtr, "Used VDBOX ID", MediaUserSetting::Group::Sequence).Get<uint32_t>();
    for (auto i = 0; i < csInstanceIdMax; i++)
    {
        CsEngineId csEngineId;
        csEngineId.value = status.m_mmioCsEngineIdReg[i];
        if (csEngineId.value != 0)
        {
            DECODE_ASSERT(csEngineId.fields.classId == classIdVideoEngine);
            DECODE_ASSERT(csEngineId.fields.instanceId < csInstanceIdMax);
            vdboxIds |= 1 << ((csEngineId.fields.instanceId) << 2);
        }
    }

    if (vdboxIds != 0)
    {
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_VDBOX_ID_USED, vdboxIds, m_osInterface->pOsContext);
    }

    return MOS_STATUS_SUCCESS;
}

#ifdef _DECODE_PROCESSING_SUPPORTED
MOS_STATUS DecodePipeline::ReportSfcLinearSurfaceUsage(const DecodeStatusReportData& reportData)
{
    DECODE_FUNC_CALL();

    DecodeDownSamplingFeature *downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature *>(
        m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
    if (downSamplingFeature != nullptr && downSamplingFeature->IsEnabled())
    {
        if (reportData.currSfcOutputSurface != nullptr)
        {
            MOS_SURFACE sfcDstSurface = *reportData.currSfcOutputSurface;
            if (!Mos_ResourceIsNull(&sfcDstSurface.OsResource))
            {
                if (sfcDstSurface.TileType == MOS_TILE_LINEAR)
                {
                    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_SFC_LINEAR_OUTPUT_USED_ID, 1, m_osInterface->pOsContext);
                }
                else
                {
                    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_SFC_LINEAR_OUTPUT_USED_ID, 0, m_osInterface->pOsContext);
                }
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS DecodePipeline::StatusCheck()
{
    DECODE_FUNC_CALL();

#if MHW_HWCMDPARSER_ENABLED
    auto instance = mhw::HwcmdParser::GetInstance();
    if (instance)
    {
        // Json dump for each frame
        instance->DumpOutput();
    }
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_delayMiliseconds > 0)
    {
        MosUtilities::MosSleep(m_delayMiliseconds);
    }
#endif

    CODECHAL_DEBUG_TOOL(DECODE_CHK_STATUS(DelayForDumpOutput()));

    uint32_t completedCount = m_statusReport->GetCompletedCount();
    if (completedCount <= m_statusCheckCount)
    {
        DECODE_NORMALMESSAGE("Invalid status check count, completedCount = %d m_statusCheckCount =%d.", completedCount, m_statusCheckCount);
        return MOS_STATUS_SUCCESS;
    }

    DecodeStatusReport* statusReport = dynamic_cast<DecodeStatusReport*>(m_statusReport);
    DECODE_CHK_NULL(statusReport);

    while (m_statusCheckCount < completedCount)
    {
        const DecodeStatusMfx& status = statusReport->GetMfxStatus(m_statusCheckCount);
        if (status.status != DecodeStatusReport::queryEnd)
        {
            DECODE_NORMALMESSAGE("Media reset may have occured at frame %d, status is %d, completedCount is %d.",
                m_statusCheckCount, status.status, completedCount);
        }

        DECODE_NORMALMESSAGE("hucStatus2 is 0x%x at frame %d.", status.m_hucErrorStatus2, m_statusCheckCount);
        DECODE_NORMALMESSAGE("hucStatus is 0x%x at frame %d.", status.m_hucErrorStatus, m_statusCheckCount);

        DECODE_CHK_STATUS(HwStatusCheck(status));

        DECODE_CHK_STATUS(ReportVdboxIds(status));

#if USE_CODECHAL_DEBUG_TOOL
        const DecodeStatusReportData& reportData = statusReport->GetReportData(m_statusCheckCount);

        auto bufferDumpNumTemp = m_debugInterface->m_bufferDumpFrameNum;
        auto currPicTemp       = m_debugInterface->m_currPic;
        auto frameTypeTemp     = m_debugInterface->m_frameType;
        auto secondField       = m_debugInterface->m_secondField;

        m_debugInterface->m_bufferDumpFrameNum = DecodeOutputIndex;
        m_debugInterface->m_currPic            = reportData.currDecodedPic;
        m_debugInterface->m_frameType          = reportData.frameType;
        m_debugInterface->m_secondField        = reportData.secondField;

#ifdef _DECODE_PROCESSING_SUPPORTED
        ReportSfcLinearSurfaceUsage(reportData);
#endif

        DECODE_CHK_STATUS(DumpOutput(reportData));

        if ((CodecHal_PictureIsFrame(m_debugInterface->m_currPic)) ||
            (CodecHal_PictureIsField(m_debugInterface->m_currPic) && m_debugInterface->m_secondField) ||
            (!CodecHal_PictureIsFrame(m_debugInterface->m_currPic) && !CodecHal_PictureIsField(m_debugInterface->m_currPic)))
        {            
            DecodeOutputIndex++;
        }

        m_debugInterface->m_bufferDumpFrameNum = bufferDumpNumTemp;
        m_debugInterface->m_currPic            = currPicTemp;
        m_debugInterface->m_frameType          = frameTypeTemp;
        m_debugInterface->m_secondField        = secondField;
#endif

        m_statusCheckCount++;
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}
