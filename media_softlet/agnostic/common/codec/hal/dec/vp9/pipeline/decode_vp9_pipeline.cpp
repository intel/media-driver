/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_vp9_pipeline.cpp
//! \brief    Defines the interface for vp9 decode pipeline
//!
#include "decode_vp9_pipeline.h"
#include "decode_utils.h"
#include "media_user_settings_mgr_g12_plus.h"
#include "codechal_setting.h"
#include "decode_vp9_phase_single.h"
#include "decode_vp9_phase_front_end.h"
#include "decode_vp9_phase_back_end.h"
#include "decode_vp9_feature_manager.h"
#include "decode_vp9_buffer_update.h"

namespace decode
{
Vp9Pipeline::Vp9Pipeline(
    CodechalHwInterface    *hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipeline(hwInterface, debugInterface)
{
    MOS_STATUS m_status = InitUserSetting(m_userSettingPtr);
}

MOS_STATUS Vp9Pipeline::Initialize(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));

    // Create basic GPU context
    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
    m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);

    m_basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    auto *codecSettings = (CodechalSetting *)settings;
    DECODE_CHK_NULL(codecSettings);

    auto *bufferUpdatePipeline = MOS_New(DecodeVp9BufferUpdate, this, m_task, m_numVdbox);
    DECODE_CHK_NULL(bufferUpdatePipeline);
    DECODE_CHK_STATUS(m_preSubPipeline->Register(*bufferUpdatePipeline));
    DECODE_CHK_STATUS(bufferUpdatePipeline->Init(*codecSettings));   

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::Prepare(void *params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    auto basicFeature = dynamic_cast<Vp9BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(basicFeature);
    DECODE_CHK_STATUS(DecodePipeline::Prepare(params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::Uninitialize()
{
    DECODE_FUNC_CALL();

#if (_DEBUG || _RELEASE_INTERNAL)
    // Report real tile frame count and virtual tile frame count
    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data                     = m_vtFrameCount;
    userFeatureWriteData.ValueID                           = __MEDIA_USER_FEATURE_VALUE_ENABLE_HEVC_DECODE_VT_FRAME_COUNT_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1, m_osInterface->pOsContext);
#endif

    DECODE_CHK_STATUS(DestoryPhaseList());
    return DecodePipeline::Uninitialize();
}

MOS_STATUS Vp9Pipeline::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());
#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_VP9D_ENABLE_ID, 1, m_osInterface->pOsContext);
#endif

#ifdef _MMC_SUPPORTED
    CODECHAL_DEBUG_TOOL(
        if (m_mmcState != nullptr) {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_destSurface));
        })
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::Execute()
{
    DECODE_FUNC_CALL();

#if (_DEBUG || _RELEASE_INTERNAL)
    if (GetDecodeMode() == virtualTileDecodeMode)
    {
        m_vtFrameCount++;
    }
#endif

    for (auto &phase : m_phaseList)
    {
        DECODE_ASSERT(phase != nullptr);
        if (phase->RequiresContextSwitch())
        {
            // switch context
            DecodeScalabilityOption *scalabOption = phase->GetDecodeScalabilityOption();
            DECODE_CHK_NULL(scalabOption);
            DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, *scalabOption, &m_scalability));
            if (scalabOption->IsScalabilityOptionMatched(m_scalabOption))
            {
                m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);
            }
        }

        StateParams stateProperty;
        stateProperty.currentPipe        = phase->GetPipe();
        stateProperty.currentPass        = phase->GetPass();
        stateProperty.pipeIndexForSubmit = phase->GetPipe() + 1;
        stateProperty.componentState     = phase;
        DECODE_CHK_STATUS(ActivatePacket(phase->GetPktId(), phase->ImmediateSubmit(), stateProperty));

        if (phase->ImmediateSubmit())
        {
            m_scalability->SetPassNumber(phase->GetPass() + 1);
            DECODE_CHK_STATUS(ExecuteActivePackets());
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::CreateFeatureManager()  // After HwNext rebase, Need override
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeVp9FeatureManager, m_allocator, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));

    return MOS_STATUS_SUCCESS;
}

Vp9Pipeline::Vp9DecodeMode Vp9Pipeline::GetDecodeMode()
{
    return m_decodeMode;
}

MOS_STATUS Vp9Pipeline::InitContexOption(Vp9BasicFeature &basicFeature)
{
    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));

    scalPars.usingHcp           = true;
    scalPars.enableVE           = MOS_VE_SUPPORTED(m_osInterface);
    scalPars.disableScalability = m_hwInterface->IsDisableScalability();
    scalPars.surfaceFormat      = basicFeature.m_destSurface.Format;
    scalPars.frameWidth         = basicFeature.m_frameWidthAlignedMinBlk;
    scalPars.frameHeight        = basicFeature.m_frameHeightAlignedMinBlk;
    scalPars.numVdbox           = m_numVdbox;
    if (m_osInterface->pfnIsMultipleCodecDevicesInUse(m_osInterface))
    {
        scalPars.disableScalability = true;
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface->bHcpDecScalabilityMode == MOS_SCALABILITY_ENABLE_MODE_FALSE)
    {
        scalPars.disableScalability = true;
    }
    else if (m_osInterface->bHcpDecScalabilityMode == MOS_SCALABILITY_ENABLE_MODE_USER_FORCE)
    {
        scalPars.disableScalability = false;
    }
    scalPars.modeSwithThreshold1 =
        ReadUserFeature(m_userSettingPtr, "HCP Decode Mode Switch TH1", MediaUserSetting::Group::Sequence).Get<uint32_t>();
    scalPars.modeSwithThreshold2 =
        ReadUserFeature(m_userSettingPtr, "HCP Decode Mode Switch TH2", MediaUserSetting::Group::Sequence).Get<uint32_t>();
    scalPars.forceMultiPipe =
        ReadUserFeature(m_userSettingPtr, "HCP Decode Always Frame Split", MediaUserSetting::Group::Sequence).Get<bool>();
    scalPars.userPipeNum =
        ReadUserFeature(m_userSettingPtr, "HCP Decode User Pipe Num", MediaUserSetting::Group::Sequence).Get<uint8_t>();
#endif

#ifdef _DECODE_PROCESSING_SUPPORTED
    DecodeDownSamplingFeature *downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature *>(
        m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
    if (downSamplingFeature != nullptr && downSamplingFeature->IsEnabled())
    {
        scalPars.usingSfc = true;
        if (!MEDIA_IS_SKU(m_skuTable, FtrSfcScalability))
        {
            scalPars.disableScalability = true;
        }
    }
    //Disable Scalability when histogram is enabled
    if (downSamplingFeature != nullptr && (downSamplingFeature->m_histogramDestSurf || downSamplingFeature->m_histogramDebug))
    {
        scalPars.disableScalability = true;
    }
#endif

    if (MEDIA_IS_SKU(m_skuTable, FtrVirtualTileScalabilityDisable))
    {
        scalPars.disableScalability = true;
        scalPars.disableVirtualTile = true; 
    }    

    DECODE_CHK_STATUS(m_scalabOption.SetScalabilityOption(&scalPars));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::InitDecodeMode(ScalabilityMode scalabMode)
{
    if (scalabMode == scalabilityVirtualTileMode)
    {
        m_decodeMode = virtualTileDecodeMode;
    }
    else if (scalabMode == scalabilitySingleMode)
    {
        m_decodeMode = baseDecodeMode;
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

template <typename T>
MOS_STATUS Vp9Pipeline::CreatePhase(uint8_t pass, uint8_t pipe, uint8_t activePipeNum)
{
    DECODE_FUNC_CALL();
    T *phase = MOS_New(T, *this, m_scalabOption);
    DECODE_CHK_NULL(phase);
    DECODE_CHK_STATUS(phase->Initialize(pass, pipe, activePipeNum));
    m_phaseList.push_back(phase);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::CreatePhaseList(const ScalabilityMode scalabMode, const uint8_t numPipe)
{
    DECODE_FUNC_CALL();
    DECODE_ASSERT(m_phaseList.empty());

    if (scalabMode == scalabilityVirtualTileMode)
    {
        DECODE_CHK_STATUS(CreatePhase<Vp9PhaseFrontEnd>());
        for (uint8_t i = 0; i < numPipe; i++)
        {
            DECODE_CHK_STATUS(CreatePhase<Vp9PhaseBackEnd>(0, i, numPipe));
        }
    }
    else
    {
        DECODE_CHK_STATUS(CreatePhase<Vp9PhaseSingle>());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::DestoryPhaseList()
{
    for (auto &phase : m_phaseList)
    {
        MOS_Delete(phase);
    }
    m_phaseList.clear();
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Vp9Pipeline::DumpPicParams(CODEC_VP9_PIC_PARAMS *picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }
    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "CurrPic FrameIdx: " << std::hex << +picParams->CurrPic.FrameIdx << std::endl;
    oss << "CurrPic PicFlags: " << std::hex << +picParams->CurrPic.PicFlags << std::endl;

    for (uint8_t i = 0; i < 8; ++i)
    {
        oss << "RefFrameList[" << +i << "] FrameIdx:" << std::hex << +picParams->RefFrameList[i].FrameIdx << std::endl;
        oss << "RefFrameList[" << +i << "] PicFlags:" << std::hex << +picParams->RefFrameList[i].PicFlags << std::endl;
    }
    oss << "FrameWidthMinus1: " << std::hex << +picParams->FrameWidthMinus1 << std::endl;
    oss << "FrameHeightMinus1: " << std::hex << +picParams->FrameHeightMinus1 << std::endl;
    oss << "PicFlags value: " << std::hex << +picParams->PicFlags.value << std::endl;
    oss << "frame_type: " << std::hex << +picParams->PicFlags.fields.frame_type << std::endl;
    oss << "show_frame: " << std::hex << +picParams->PicFlags.fields.show_frame << std::endl;
    oss << "error_resilient_mode: " << std::hex << +picParams->PicFlags.fields.error_resilient_mode << std::endl;
    oss << "intra_only: " << std::hex << +picParams->PicFlags.fields.intra_only << std::endl;
    oss << "LastRefIdx: " << std::hex << +picParams->PicFlags.fields.LastRefIdx << std::endl;
    oss << "LastRefSignBias: " << std::hex << +picParams->PicFlags.fields.LastRefSignBias << std::endl;
    oss << "GoldenRefIdx: " << std::hex << +picParams->PicFlags.fields.GoldenRefIdx << std::endl;
    oss << "GoldenRefSignBias: " << std::hex << +picParams->PicFlags.fields.GoldenRefSignBias << std::endl;
    oss << "AltRefIdx: " << std::hex << +picParams->PicFlags.fields.AltRefIdx << std::endl;
    oss << "AltRefSignBias: " << std::hex << +picParams->PicFlags.fields.AltRefSignBias << std::endl;
    oss << "allow_high_precision_mv: " << std::hex << +picParams->PicFlags.fields.allow_high_precision_mv << std::endl;
    oss << "mcomp_filter_type: " << std::hex << +picParams->PicFlags.fields.mcomp_filter_type << std::endl;
    oss << "frame_parallel_decoding_mode: " << std::hex << +picParams->PicFlags.fields.frame_parallel_decoding_mode << std::endl;
    oss << "segmentation_enabled: " << std::hex << +picParams->PicFlags.fields.segmentation_enabled << std::endl;
    oss << "segmentation_temporal_update: " << std::hex << +picParams->PicFlags.fields.segmentation_temporal_update << std::endl;
    oss << "segmentation_update_map: " << std::hex << +picParams->PicFlags.fields.segmentation_update_map << std::endl;
    oss << "reset_frame_context: " << std::hex << +picParams->PicFlags.fields.reset_frame_context << std::endl;
    oss << "refresh_frame_context: " << std::hex << +picParams->PicFlags.fields.refresh_frame_context << std::endl;
    oss << "frame_context_idx: " << std::hex << +picParams->PicFlags.fields.frame_context_idx << std::endl;
    oss << "LosslessFlag: " << std::hex << +picParams->PicFlags.fields.LosslessFlag << std::endl;
    oss << "ReservedField: " << std::hex << +picParams->PicFlags.fields.ReservedField << std::endl;
    oss << "filter_level: " << std::hex << +picParams->filter_level << std::endl;
    oss << "sharpness_level: " << std::hex << +picParams->sharpness_level << std::endl;
    oss << "log2_tile_rows: " << std::hex << +picParams->log2_tile_rows << std::endl;
    oss << "log2_tile_columns: " << std::hex << +picParams->log2_tile_columns << std::endl;
    oss << "UncompressedHeaderLengthInBytes: " << std::hex << +picParams->UncompressedHeaderLengthInBytes << std::endl;
    oss << "FirstPartitionSize: " << std::hex << +picParams->FirstPartitionSize << std::endl;
    oss << "profile: " << std::hex << +picParams->profile << std::endl;
    oss << "BitDepthMinus8: " << std::hex << +picParams->BitDepthMinus8 << std::endl;
    oss << "subsampling_x: " << std::hex << +picParams->subsampling_x << std::endl;
    oss << "subsampling_y: " << std::hex << +picParams->subsampling_y << std::endl;

    for (uint8_t i = 0; i < 7; ++i)
    {
        oss << "SegTreeProbs[" << +i << "]: " << std::hex << +picParams->SegTreeProbs[i] << std::endl;
    }
    for (uint8_t i = 0; i < 3; ++i)
    {
        oss << "SegPredProbs[" << +i << "]: " << std::hex << +picParams->SegPredProbs[i] << std::endl;
    }
    oss << "BSBytesInBuffer: " << std::hex << +picParams->BSBytesInBuffer << std::endl;
    oss << "StatusReportFeedbackNumber: " << std::hex << +picParams->StatusReportFeedbackNumber << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::DumpSegmentParams(CODEC_VP9_SEGMENT_PARAMS *segmentParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSegmentParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(segmentParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for (uint8_t i = 0; i < 8; ++i)
    {
        oss << "SegData[" << +i << "] SegmentFlags value: " << std::hex << +segmentParams->SegData[i].SegmentFlags.value << std::endl;
        oss << "SegData[" << +i << "] SegmentReferenceEnabled: " << std::hex << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReferenceEnabled << std::endl;
        oss << "SegData[" << +i << "] SegmentReference: " << std::hex << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReference << std::endl;
        oss << "SegData[" << +i << "] SegmentReferenceSkipped: " << std::hex << +segmentParams->SegData[i].SegmentFlags.fields.SegmentReferenceSkipped << std::endl;
        oss << "SegData[" << +i << "] ReservedField3: " << std::hex << +segmentParams->SegData[i].SegmentFlags.fields.ReservedField3 << std::endl;

        for (uint8_t j = 0; j < 4; ++j)
        {
            oss << "SegData[" << +i << "] FilterLevel[" << +j << "]:";
            oss << std::hex << +segmentParams->SegData[i].FilterLevel[j][0] << " ";
            oss << std::hex << +segmentParams->SegData[i].FilterLevel[j][1] << std::endl;
        }
        oss << "SegData[" << +i << "] LumaACQuantScale: " << std::hex << +segmentParams->SegData[i].LumaACQuantScale << std::endl;
        oss << "SegData[" << +i << "] LumaDCQuantScale: " << std::hex << +segmentParams->SegData[i].LumaDCQuantScale << std::endl;
        oss << "SegData[" << +i << "] ChromaACQuantScale: " << std::hex << +segmentParams->SegData[i].ChromaACQuantScale << std::endl;
        oss << "SegData[" << +i << "] ChromaDCQuantScale: " << std::hex << +segmentParams->SegData[i].ChromaDCQuantScale << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufSegmentParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace decode
