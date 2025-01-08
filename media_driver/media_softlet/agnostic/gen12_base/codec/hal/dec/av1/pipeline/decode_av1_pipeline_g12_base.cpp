/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_av1_pipeline_g12_base.cpp
//! \brief    Defines the interface for av1 decode pipeline
//!
#include "decode_av1_pipeline_g12_base.h"
#include "decode_utils.h"
#include "codechal_setting.h"
#include "decode_av1_feature_manager_g12_base.h"
#include "decode_huc_packet_creator_g12.h"
#include "decode_sfc_histogram_postsubpipeline_m12.h"
#include "decode_input_bitstream_m12.h"
#include "decode_cp_bitstream_m12.h"
#include "decode_marker_packet_g12.h"
#include "decode_predication_packet_g12.h"
#include "media_debug_fast_dump.h"

namespace decode {

Av1PipelineG12_Base::Av1PipelineG12_Base(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface)
    : DecodePipeline(hwInterface ? hwInterface->m_hwInterfaceNext : nullptr, debugInterface)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS Av1PipelineG12_Base::Initialize(void *settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(MediaPipeline::InitPlatform());
    DECODE_CHK_STATUS(MediaPipeline::CreateMediaCopyWrapper());
    DECODE_CHK_NULL(m_mediaCopyWrapper);

    DECODE_CHK_NULL(m_waTable);

    auto *codecSettings = (CodechalSetting *)settings;
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
            m_debugInterface->Initialize(m_hwInterface, codecSettings->codecFunction, m_mediaCopyWrapper)););

    if (m_hwInterface->m_hwInterfaceNext)
    {
        m_hwInterface->m_hwInterfaceNext->legacyHwInterface = m_hwInterface;
    }
    m_mediaContext = MOS_New(MediaContext, scalabilityDecoder, m_hwInterface->m_hwInterfaceNext, m_osInterface);
    DECODE_CHK_NULL(m_mediaContext);

    m_task = CreateTask(MediaTask::TaskType::cmdTask);
    DECODE_CHK_NULL(m_task);

    m_numVdbox = GetSystemVdboxNumber();

    bool limitedLMemBar = MEDIA_IS_SKU(m_skuTable, FtrLimitedLMemBar) ? true : false;
    m_allocator         = MOS_New(DecodeAllocator, m_osInterface, limitedLMemBar);
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

    // Create basic GPU context
    if (m_osInterface->pfnIsMismatchOrderProgrammingSupported())
    {
        DecodeScalabilityPars scalPars;
        MOS_ZeroMemory(&scalPars, sizeof(scalPars));
        DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
        m_decodeContext       = m_osInterface->pfnGetGpuContext(m_osInterface);
        m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;
    }

    HucPacketCreatorG12 *hucPktCreator = dynamic_cast<HucPacketCreatorG12 *>(this);
    DECODE_CHK_NULL(hucPktCreator);

  bool forceTileBasedDecodingRead = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
    forceTileBasedDecodingRead = ReadUserFeature(m_userSettingPtr, "Force Av1 Tile Based Decode", MediaUserSetting::Group::Sequence).Get<bool>();
#endif
    m_forceTileBasedDecoding = forceTileBasedDecodingRead;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1PipelineG12_Base::Prepare(void *params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    auto basicFeature = dynamic_cast<Av1BasicFeatureG12*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(basicFeature);
    DECODE_CHK_STATUS(DecodePipeline::Prepare(params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1PipelineG12_Base::Uninitialize()
{
    DECODE_FUNC_CALL();

    return DecodePipeline::Uninitialize();
}

MOS_STATUS Av1PipelineG12_Base::UserFeatureReport()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::UserFeatureReport());
#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_AV1D_ENABLE_ID, 1, m_osInterface->pOsContext);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1PipelineG12_Base::ActivateDecodePackets()
{
    DECODE_FUNC_CALL();
    auto basicFeature    = dynamic_cast<Av1BasicFeatureG12 *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(basicFeature);
    bool immediateSubmit = true;

    if (m_isFirstTileInFrm)
    {
        m_isFirstTileInFrm = false;
    }

    if (!m_forceTileBasedDecoding)
    {
        immediateSubmit = false;
    }

    for (uint16_t curPass = 0; curPass < GetPassNum(); curPass++)
    {
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(this, av1DecodePacketId), immediateSubmit, curPass, 0));
        if (basicFeature->m_filmGrainEnabled)
        {
            m_activePacketList.back().frameTrackingRequested = false;
        }
    }

    return MOS_STATUS_SUCCESS;
}

bool Av1PipelineG12_Base::FrameBasedDecodingInUse()
{
    auto basicFeature = dynamic_cast<Av1BasicFeatureG12*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));

    bool isframeBasedDecodingUsed = false;

    if (basicFeature != nullptr)
    {
        isframeBasedDecodingUsed = ((basicFeature->m_av1PicParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType > 0) &
                                   ((basicFeature->m_av1PicParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType |
                                    basicFeature->m_av1PicParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType) > 0) &&
                                    basicFeature->m_av1PicParams->m_picInfoFlags.m_fields.m_useSuperres && MEDIA_IS_WA(GetWaTable(), Wa_1409820462)
                                    || !m_forceTileBasedDecoding);
    }
    return isframeBasedDecodingUsed;
}

MOS_STATUS Av1PipelineG12_Base::CreateFeatureManager()
{
    DECODE_FUNC_CALL();
    m_featureManager = MOS_New(DecodeAv1FeatureManagerG12_Base, m_allocator, m_hwInterface, m_osInterface);
    DECODE_CHK_NULL(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1PipelineG12_Base::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
{
    DECODE_FUNC_CALL();

    DecodePredicationPktG12 *predicationPkt = MOS_New(DecodePredicationPktG12, this, m_hwInterface);
    DECODE_CHK_NULL(predicationPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, predicationSubPacketId), *predicationPkt));

    DecodeMarkerPktG12 *markerPkt = MOS_New(DecodeMarkerPktG12, this, m_hwInterface);
    DECODE_CHK_NULL(markerPkt);
    DECODE_CHK_STATUS(subPacketManager.Register(
        DecodePacketId(this, markerSubPacketId), *markerPkt));

    return MOS_STATUS_SUCCESS;
}

Av1PipelineG12_Base::Av1DecodeMode Av1PipelineG12_Base::GetDecodeMode()
{
    return m_decodeMode;
}

MOS_STATUS Av1PipelineG12_Base::CreatePostSubPipeLines(DecodeSubPipelineManager &subPipelineManager)
{
    DECODE_FUNC_CALL();

#ifdef _DECODE_PROCESSING_SUPPORTED
    auto sfcHistogramPostSubPipeline = MOS_New(DecodeSfcHistogramSubPipelineM12, this, m_task, m_numVdbox, m_hwInterface);
    DECODE_CHK_NULL(sfcHistogramPostSubPipeline);
    DECODE_CHK_STATUS(m_postSubPipeline->Register(*sfcHistogramPostSubPipeline));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1PipelineG12_Base::CreatePreSubPipeLines(DecodeSubPipelineManager &subPipelineManager)
{
    m_bitstream = MOS_New(DecodeInputBitstreamM12, this, m_task, m_numVdbox, m_hwInterface);
    DECODE_CHK_NULL(m_bitstream);
    DECODE_CHK_STATUS(subPipelineManager.Register(*m_bitstream));

    m_streamout = MOS_New(DecodeStreamOutM12, this, m_task, m_numVdbox, m_hwInterface);
    DECODE_CHK_NULL(m_streamout);
    DECODE_CHK_STATUS(subPipelineManager.Register(*m_streamout));
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS Av1PipelineG12_Base::DumpParams(Av1BasicFeatureG12 &basicFeature)
{
    m_debugInterface->m_frameType          = basicFeature.m_av1PicParams->m_picInfoFlags.m_fields.m_frameType ? P_TYPE : I_TYPE;
    m_debugInterface->m_bufferDumpFrameNum = basicFeature.m_frameNum;

    DECODE_CHK_STATUS(DumpPicParams(basicFeature.m_av1PicParams));
    DECODE_CHK_STATUS(DumpTileParams(basicFeature.m_av1TileParams, basicFeature.m_tileCoding.m_numTiles));
    DECODE_CHK_STATUS(DumpBitstream(&basicFeature.m_resDataBuffer.OsResource, basicFeature.m_dataSize, 0));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1PipelineG12_Base::DumpPicParams(CodecAv1PicParams *picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (picParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "DEC",
            CodechalDbgBufferType::bufPicParams,
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)picParams,
                fileName,
                sizeof(CodecAv1PicParams),
                0,
                MediaDebugSerializer<CodecAv1PicParams>());
        }
        else
        {
            DumpDecodeAv1PicParams(picParams, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1PipelineG12_Base::DumpTileParams(CodecAv1TileParams *tileParams, uint32_t tileNum)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (tileParams == nullptr)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        const char *fileName = m_debugInterface->CreateFileName(
            "DEC",
            "TileParams",
            CodechalDbgExtType::txt);

        if (m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrEnableFastDump))
        {
            MediaDebugFastDump::Dump(
                (uint8_t *)tileParams,
                fileName,
                sizeof(CodecAv1TileParams) * tileNum,
                0,
                MediaDebugSerializer<CodecAv1TileParams>());
        }
        else
        {
            DumpDecodeAv1TileParams(tileParams, tileNum, fileName);
        }
    }

    return MOS_STATUS_SUCCESS;
}
#endif

}
