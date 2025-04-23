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
#include "codechal_setting.h"
#include "decode_vp9_phase_single.h"
#include "decode_vp9_phase_front_end.h"
#include "decode_vp9_phase_back_end.h"
#include "decode_vp9_feature_manager.h"
#include "decode_vp9_buffer_update.h"
#include "media_debug_fast_dump.h"

namespace decode
{
Vp9Pipeline::Vp9Pipeline(
    CodechalHwInterfaceNext *hwInterface,
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
    m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;

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
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "VT Decoded Count",
        m_vtFrameCount,
        MediaUserSetting::Group::Sequence);
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
                m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;
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

MOS_STATUS Vp9Pipeline::CreateFeatureManager()
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
    MOS_STATUS status = phase->Initialize(pass, pipe, activePipeNum);
    if (status != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(phase);
        return status;
    }
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

    if (picParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufPicParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)picParams,
                fileName,
                sizeof(CODEC_VP9_PIC_PARAMS),
                0,
                MediaDebugSerializer<CODEC_VP9_PIC_PARAMS>());
        }
        else
        {
            DumpDecodeVp9PicParams(picParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::DumpSliceParams(CODEC_VP9_SLICE_PARAMS *slcParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (slcParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufSlcParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)slcParams,
                fileName,
                sizeof(CODEC_VP9_SLICE_PARAMS),
                0,
                MediaDebugSerializer<CODEC_VP9_SLICE_PARAMS>());
        }
        else
        {
            DumpDecodeVp9SliceParams(slcParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9Pipeline::DumpSegmentParams(CODEC_VP9_SEGMENT_PARAMS *segmentParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (segmentParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSegmentParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufSegmentParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)segmentParams,
                fileName,
                sizeof(CODEC_VP9_SEGMENT_PARAMS),
                0,
                MediaDebugSerializer<CODEC_VP9_SEGMENT_PARAMS>());
        }
        else
        {
            DumpDecodeVp9SegmentParams(segmentParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}  // namespace decode
