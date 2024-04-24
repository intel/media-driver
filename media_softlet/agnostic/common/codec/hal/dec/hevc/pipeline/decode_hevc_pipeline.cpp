/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     decode_hevc_pipeline.cpp
//! \brief    Defines the interface for hevc decode pipeline
//!
#include "decode_hevc_pipeline.h"
#include "decode_utils.h"
#include "codechal_setting.h"
#include "decode_hevc_phase_s2l.h"
#include "decode_hevc_phase_long.h"
#include "decode_hevc_phase_front_end.h"
#include "decode_hevc_phase_back_end.h"
#include "decode_hevc_phase_real_tile.h"
#include "decode_hevc_feature_manager.h"
#include "decode_hevc_downsampling_packet.h"
#include "media_debug_fast_dump.h"

namespace decode {

HevcPipeline::HevcPipeline(CodechalHwInterfaceNext *hwInterface, CodechalDebugInterface *debugInterface)
    : DecodePipeline(hwInterface, debugInterface)
{
    MOS_STATUS m_status = InitUserSetting(m_userSettingPtr);
}

MOS_STATUS HevcPipeline::Initialize(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::Initialize(settings));

    auto *codecSettings = (CodechalSetting*)settings;
    DECODE_CHK_NULL(codecSettings);

    // Create basic GPU context
    DecodeScalabilityPars scalPars;
    MOS_ZeroMemory(&scalPars, sizeof(scalPars));
    DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
    m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);
    m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;

    m_basicFeature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::Uninitialize()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DestoryPhaseList());
    return DecodePipeline::Uninitialize();
}

MOS_STATUS HevcPipeline::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());

#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_HEVCD_ENABLE_ID, 1, m_osInterface->pOsContext);
#endif

#ifdef _MMC_SUPPORTED
    CODECHAL_DEBUG_TOOL(
        if (m_mmcState != nullptr)
        {
            m_mmcState->UpdateUserFeatureKey(&(m_basicFeature->m_destSurface));
        }
        );
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::Execute()
{
    DECODE_FUNC_CALL();

#if (_DEBUG || _RELEASE_INTERNAL)
    if (GetDecodeMode() == realTileDecodeMode)
    {
        m_rtFrameCount++;
    }
    else if (GetDecodeMode() == virtualTileDecodeMode)
    {
        m_vtFrameCount++;
    }
    else if (GetDecodeMode() == separateTileDecodeMode)
    {
        m_spFrameCount++;
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
            if (m_allowVirtualNodeReassign)
            {
                // reassign decoder virtual node at the first frame for each stream
                DECODE_CHK_STATUS(m_mediaContext->ReassignContextForDecoder(m_basicFeature->m_frameNum, *scalabOption, &m_scalability));
                m_mediaContext->SetLatestDecoderVirtualNode();
            }
            else
            {
                DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, *scalabOption, &m_scalability));
            }
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

MOS_STATUS HevcPipeline::CreateFeatureManager()
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeHevcFeatureManager, m_allocator, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(DecodePipeline::CreateSubPackets(subPacketManager, codecSettings));

#ifdef _DECODE_PROCESSING_SUPPORTED
    HevcDownSamplingPkt *downSamplingPkt = MOS_New(HevcDownSamplingPkt, this, m_hwInterface);
    DECODE_CHK_NULL(downSamplingPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
                        DecodePacketId(this, downSamplingSubPacketId), *downSamplingPkt));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::InitContexOption(HevcScalabilityPars& scalPars)
{
    scalPars.usingHcp = true;
    scalPars.enableVE = MOS_VE_SUPPORTED(m_osInterface);
    scalPars.disableScalability = m_hwInterface->IsDisableScalability();
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
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::InitDecodeMode(ScalabilityMode scalabMode, HevcBasicFeature &basicFeature)
{
    if (scalabMode == scalabilityVirtualTileMode)
    {
        m_decodeMode = virtualTileDecodeMode;
    }
    else if (scalabMode == scalabilityRealTileMode)
    {
        m_decodeMode = realTileDecodeMode;
    }
    else
    {
        PCODEC_HEVC_PIC_PARAMS picParams = basicFeature.m_hevcPicParams;
        DECODE_CHK_NULL(picParams);

        if (picParams->tiles_enabled_flag &&
            (basicFeature.m_isSCCIBCMode || basicFeature.m_isSCCPLTMode || basicFeature.m_isWPPMode))
        {
            m_decodeMode = separateTileDecodeMode;
        }
        else
        {
            m_decodeMode = baseDecodeMode;
        }
    }

    return MOS_STATUS_SUCCESS;
}

template<typename T>
MOS_STATUS HevcPipeline::CreatePhase(uint8_t pass, uint8_t pipe, uint8_t activePipeNum)
{
    DECODE_FUNC_CALL();
    T *phase = MOS_New(T, *this, m_scalabOption);
    DECODE_CHK_NULL(phase);
    MOS_STATUS estatus = phase->Initialize(pass, pipe, activePipeNum);
    if (estatus != MOS_STATUS_SUCCESS)
    {
        MOS_Delete(phase);
        return estatus;
    }
    m_phaseList.push_back(phase);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::CreatePhaseList(HevcBasicFeature &basicFeature, const ScalabilityMode scalabMode, const uint8_t numPipe)
{
    DECODE_FUNC_CALL();
    DECODE_ASSERT(m_phaseList.empty());

    if (basicFeature.m_shortFormatInUse)
    {
        DECODE_CHK_STATUS(CreatePhase<HevcPhaseS2L>());
    }

    if (scalabMode == scalabilityVirtualTileMode)
    {
        DECODE_CHK_STATUS(CreatePhase<HevcPhaseFrontEnd>());
        for (uint8_t i = 0; i < numPipe; i++)
        {
            DECODE_CHK_STATUS(CreatePhase<HevcPhaseBackEnd>(0, i, numPipe));
        }
    }
    else if (scalabMode == scalabilityRealTileMode)
    {
        PCODEC_HEVC_PIC_PARAMS picParams = basicFeature.m_hevcPicParams;
        DECODE_CHK_NULL(picParams);
        uint8_t pass = 0;
        uint8_t pipe = 0;
        uint8_t activePipeNum = numPipe;
        uint32_t numTileCol = picParams->num_tile_columns_minus1 + 1;
        for (uint32_t i = 0; i < numTileCol; i++)
        {
            DECODE_CHK_STATUS(CreatePhase<HevcPhaseRealTile>(pass, pipe, activePipeNum));
            pipe++;
            if (pipe >= numPipe)
            {
                pipe = 0;
                pass++;
                uint32_t remainingCols = numTileCol - i - 1;
                activePipeNum = (remainingCols >= numPipe) ? numPipe : remainingCols;
            }
        }
    }
    else
    {
        DECODE_CHK_STATUS(CreatePhase<HevcPhaseLong>());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DestoryPhaseList()
{
    for (auto &phase : m_phaseList)
    {
        MOS_Delete(phase);
    }
    m_phaseList.clear();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::StoreDestToRefList(HevcBasicFeature &basicFeature)
{
    DECODE_CHK_NULL(basicFeature.m_hevcPicParams);
    DECODE_CHK_STATUS(basicFeature.m_refFrames.UpdateCurResource(
        *basicFeature.m_hevcPicParams, false));
    return MOS_STATUS_SUCCESS;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS HevcPipeline::HwStatusCheck(const DecodeStatusMfx &status)
{
    DECODE_FUNC_CALL();

    if (m_basicFeature->m_shortFormatInUse)
    {
        // Check HuC_status2 Imem loaded bit, if 0, return error
        if (((status.m_hucErrorStatus2 >> 32) && (m_hwInterface->GetHucInterfaceNext()->GetHucStatus2ImemLoadedMask())) == 0)
        {
            if (!m_reportHucStatus)
            {
                WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_HUC_LOAD_STATUS_ID, 1, m_osInterface->pOsContext);
                m_reportHucStatus = true;
            }
            DECODE_ASSERTMESSAGE("Huc IMEM Loaded fails");
            MT_ERR1(MT_DEC_HEVC, MT_DEC_HUC_ERROR_STATUS2, (status.m_hucErrorStatus2 >> 32));
        }

        // Check Huc_status None Critical Error bit, bit 15. If 0, return error.
        if (((status.m_hucErrorStatus >> 32) & m_hwInterface->GetHucInterfaceNext()->GetHucStatusHevcS2lFailureMask()) == 0)
        {
            if (!m_reportHucCriticalError)
            {
                WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_HUC_REPORT_CRITICAL_ERROR_ID, 1, m_osInterface->pOsContext);
                m_reportHucCriticalError = true;
            }
            DECODE_ASSERTMESSAGE("Huc Report Critical Error!");
            MT_ERR1(MT_DEC_HEVC, MT_DEC_HUC_STATUS_CRITICAL_ERROR, (status.m_hucErrorStatus >> 32));
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

bool HevcPipeline::IsShortFormat()
{
    return m_basicFeature->m_shortFormatInUse;
}

HevcPipeline::HevcDecodeMode HevcPipeline::GetDecodeMode()
{
    return m_decodeMode;
}

bool HevcPipeline::IsFESeparateSubmission()
{
    return m_scalabOption.IsFESeparateSubmission();
}

MHW_BATCH_BUFFER* HevcPipeline::GetSliceLvlCmdBuffer()
{
    if (m_secondLevelBBArray == nullptr)
    {
        return nullptr;
    }
    return m_secondLevelBBArray->Peek();
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS HevcPipeline::DumpPicParams(
    PCODEC_HEVC_PIC_PARAMS     picParams,
    PCODEC_HEVC_EXT_PIC_PARAMS extPicParams,
    PCODEC_HEVC_SCC_PIC_PARAMS sccPicParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (picParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        const char *picFileName = nullptr;
        const char *extFileName = nullptr;
        const char *sccFileName = nullptr;

        picFileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufPicParams,
            CodechalDbgExtType::txt);

        if (extPicParams)
        {
            extFileName = m_debugInterface->CreateFileName(
                "_DEC_EXT",
                CodechalDbgBufferType::bufPicParams,
                CodechalDbgExtType::txt);
        }

        if (sccPicParams)
        {
            sccFileName = m_debugInterface->CreateFileName(
                "_DEC_SCC",
                CodechalDbgBufferType::bufPicParams,
                CodechalDbgExtType::txt);
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)picParams,
                picFileName,
                sizeof(CODEC_HEVC_PIC_PARAMS),
                0,
                MediaDebugSerializer<CODEC_HEVC_PIC_PARAMS>());

            if (extPicParams)
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)extPicParams,
                    extFileName,
                    sizeof(CODEC_HEVC_EXT_PIC_PARAMS),
                    0,
                    MediaDebugSerializer<CODEC_HEVC_EXT_PIC_PARAMS>());
            }

            if (sccPicParams)
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)sccPicParams,
                    sccFileName,
                    sizeof(CODEC_HEVC_SCC_PIC_PARAMS),
                    0,
                    MediaDebugSerializer<CODEC_HEVC_SCC_PIC_PARAMS>());
            }
        }
        else
        {
            DumpDecodeHevcPicParams(picParams, picFileName);

            if (extPicParams)
            {
                DumpDecodeHevcExtPicParams(extPicParams, extFileName);
            }

            if (sccPicParams)
            {
                DumpDecodeHevcSccPicParams(sccPicParams, sccFileName);
            }
        }        
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DumpSliceParams(
    PCODEC_HEVC_SLICE_PARAMS     sliceParams,
    PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams,
    uint32_t                     numSlices,
    bool                         shortFormatInUse)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (sliceParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        const char *slcFileName       = nullptr;
        const char *extSlcFileName    = nullptr;
        bool        hasExtSliceParams = false;
            
        if (extSliceParams && !shortFormatInUse)
        {
            hasExtSliceParams = true;
        }

        slcFileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufSlcParams,
            CodechalDbgExtType::txt);

        if (hasExtSliceParams)
        {
            extSlcFileName = m_debugInterface->CreateFileName(
                "_DEC_EXT",
                CodechalDbgBufferType::bufSlcParams,
                CodechalDbgExtType::txt);
        }

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            if (shortFormatInUse)
            {
                PCODEC_HEVC_SF_SLICE_PARAMS slcParamsSF =
                    (PCODEC_HEVC_SF_SLICE_PARAMS)MOS_AllocMemory(sizeof(CODEC_HEVC_SF_SLICE_PARAMS) * numSlices);

                for (uint16_t i = 0; i < numSlices; i++)
                {
                    slcParamsSF[i] = reinterpret_cast<CODEC_HEVC_SF_SLICE_PARAMS &>(sliceParams[i]);
                }

                MediaDebugFastDump::Dump(
                    (uint8_t *)slcParamsSF,
                    slcFileName,
                    sizeof(CODEC_HEVC_SF_SLICE_PARAMS) * numSlices,
                    0,
                    MediaDebugSerializer<CODEC_HEVC_SF_SLICE_PARAMS>());
            }
            else
            {
                MediaDebugFastDump::Dump(
                    (uint8_t *)sliceParams,
                    slcFileName,
                    sizeof(CODEC_HEVC_SLICE_PARAMS) * numSlices,
                    0,
                    MediaDebugSerializer<CODEC_HEVC_SLICE_PARAMS>());

                if (extSliceParams)
                {
                    MediaDebugFastDump::Dump(
                        (uint8_t *)extSliceParams,
                        extSlcFileName,
                        sizeof(CODEC_HEVC_EXT_SLICE_PARAMS) * numSlices,
                        0,
                        MediaDebugSerializer<CODEC_HEVC_EXT_SLICE_PARAMS>());
                }
            }
        }
        else
        {
            DumpDecodeHevcSliceParams(sliceParams, numSlices, slcFileName, shortFormatInUse);

            if (hasExtSliceParams)
            {
                DumpDecodeHevcExtSliceParams(extSliceParams, numSlices, extSlcFileName);
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DumpIQParams(
    PCODECHAL_HEVC_IQ_MATRIX_PARAMS iqParams)
{
    DECODE_FUNC_CALL();

    if (iqParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufIqParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)iqParams,
                fileName,
                sizeof(CODECHAL_HEVC_IQ_MATRIX_PARAMS),
                0,
                MediaDebugSerializer<CODECHAL_HEVC_IQ_MATRIX_PARAMS>());
        }
        else
        {
            DumpDecodeHevcIQParams(iqParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DumpSubsetsParams(
    PCODEC_HEVC_SUBSET_PARAMS subsetsParams)
{
    DECODE_FUNC_CALL();

    if (subsetsParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSubsetsParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "_DEC",
            CodechalDbgBufferType::bufSubsetsParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)subsetsParams,
                fileName,
                sizeof(CODEC_HEVC_SUBSET_PARAMS),
                0,
                MediaDebugSerializer<CODEC_HEVC_SUBSET_PARAMS>());
        }
        else
        {
            DumpDecodeHevcSubsetParams(subsetsParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcPipeline::DumpSecondLevelBatchBuffer()
{
    DECODE_FUNC_CALL();

    PMHW_BATCH_BUFFER batchBuffer = GetSliceLvlCmdBuffer();
    DECODE_CHK_NULL(batchBuffer);

    batchBuffer->iLastCurrent = batchBuffer->iSize * batchBuffer->count;
    batchBuffer->dwOffset     = 0;
    DECODE_CHK_STATUS(m_debugInterface->Dump2ndLvlBatch(
                        batchBuffer,
                        CODECHAL_NUM_MEDIA_STATES,
                        "_DEC"));

    return MOS_STATUS_SUCCESS;
}
#endif

}
