/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_av1_pipeline_g12.cpp
//! \brief    Defines the interface for av1 decode pipeline
//!
#include "decode_av1_pipeline_g12.h"
#include "decode_av1_packet_g12.h"
#include "decode_av1_picture_packet_g12.h"
#include "decode_av1_tile_packet_g12.h"
#include "decode_utils.h"
#include "codechal_debug.h"
#include "decode_av1_tile_coding_g12.h"
#include "decode_av1_feature_manager_g12.h"
#include "decode_mem_compression_g12.h"
#include "decode_av1_feature_defs_g12.h"
#include "decode_marker_packet_g12.h"
#include "decode_predication_packet_g12.h"

namespace decode
{
    Av1PipelineG12::Av1PipelineG12(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface)
        : Av1PipelineG12_Base(hwInterface, debugInterface)
    {

    }

    MOS_STATUS Av1PipelineG12::Init(void *settings)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(settings);
        DECODE_CHK_STATUS(Initialize(settings));

        if (MEDIA_IS_SKU(m_skuTable, FtrWithSlimVdbox))
        {
            m_numVdbox = 1;
        }

        m_av1DecodePkt = MOS_New(Av1DecodePktG12, this, m_task, m_hwInterface);
        DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, av1DecodePacketId), m_av1DecodePkt));
        DECODE_CHK_STATUS(m_av1DecodePkt->Init());

        if (m_numVdbox == 2)
        {
            m_allowVirtualNodeReassign = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineG12::InitContext()
    {
        DECODE_FUNC_CALL();

        auto basicFeature = dynamic_cast<Av1BasicFeatureG12*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(basicFeature);

        DecodeScalabilityPars scalPars;
        MOS_ZeroMemory(&scalPars, sizeof(scalPars));
        scalPars.disableScalability = true;
        scalPars.enableVE = MOS_VE_SUPPORTED(m_osInterface);
        if (MEDIA_IS_SKU(m_skuTable, FtrWithSlimVdbox))
        {
            scalPars.usingSlimVdbox = true;
        }
        else
        {
            scalPars.usingSlimVdbox = false;
        }
        scalPars.numVdbox = m_numVdbox;

        if (m_allowVirtualNodeReassign)
        {
            // reassign decoder virtual node at the first frame for each stream
            DECODE_CHK_STATUS(m_mediaContext->ReassignContextForDecoder(basicFeature->m_frameNum, &scalPars, &m_scalability));
            m_mediaContext->SetLatestDecoderVirtualNode();
        }
        else
        {
            DECODE_CHK_STATUS(m_mediaContext->SwitchContext(VdboxDecodeFunc, &scalPars, &m_scalability));
        }
        DECODE_CHK_NULL(m_scalability);

        m_decodeContext = m_osInterface->pfnGetGpuContext(m_osInterface);
        m_decodeContextHandle = m_osInterface->CurrentGpuContextHandle;
        DECODE_CHK_STATUS(basicFeature->m_tileCoding.CalcNumPass(*basicFeature->m_av1PicParams, basicFeature->m_av1TileParams));
        m_passNum = basicFeature->m_tileCoding.GetNumPass();
        m_scalability->SetPassNumber(m_passNum);

        if (scalPars.disableScalability)
            m_osInterface->pfnSetMultiEngineEnabled(m_osInterface, COMPONENT_Decode, false);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineG12::Prepare(void *params)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(params);
        DecodePipelineParams *pipelineParams = (DecodePipelineParams *)params;
        m_pipeMode = pipelineParams->m_pipeMode;

        PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

        auto basicFeature = dynamic_cast<Av1BasicFeatureG12*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(basicFeature);

        if (IsFirstProcessPipe(*pipelineParams))
        {
            DECODE_CHK_STATUS(Av1PipelineG12_Base::Prepare(params));
        }

        DECODE_CHK_STATUS(m_preSubPipeline->Prepare(*pipelineParams));
        DECODE_CHK_STATUS(m_postSubPipeline->Prepare(*pipelineParams));

        if (m_pipeMode == decodePipeModeProcess)
        {
            if (IsCompleteBitstream())
            {
                CODECHAL_DEBUG_TOOL(DECODE_CHK_STATUS(DumpParams(*basicFeature)));

                DecodeStatusParameters inputParameters = {};
                MOS_ZeroMemory(&inputParameters, sizeof(DecodeStatusParameters));
                inputParameters.statusReportFeedbackNumber = basicFeature->m_av1PicParams->m_statusReportFeedbackNumber;
                inputParameters.codecFunction              = basicFeature->m_codecFunction;
                inputParameters.picWidthInMb               = basicFeature->m_picWidthInMb;
                inputParameters.pictureCodingType          = basicFeature->m_pictureCodingType;
                inputParameters.currOriginalPic            = basicFeature->m_curRenderPic;
                inputParameters.currDecodedPicRes          = basicFeature->m_destSurface.OsResource;
                inputParameters.numUsedVdbox               = m_numVdbox;

                CODECHAL_DEBUG_TOOL(
                    if (m_streamout != nullptr)  
                    {  
                        DECODE_CHK_STATUS(m_streamout->InitStatusReportParam(inputParameters));  
                    }  
                );

#if (_DEBUG || _RELEASE_INTERNAL)
                auto filmGrainFeature = dynamic_cast<Av1DecodeFilmGrainG12*>(m_featureManager->GetFeature(
                        Av1FeatureIDs::av1SwFilmGrain));
                if (filmGrainFeature != nullptr && filmGrainFeature->m_filmGrainEnabled)
                {
                    auto frameIdx = basicFeature->m_curRenderPic.FrameIdx;
                    inputParameters.fgOutputPicRes = &filmGrainFeature->m_fgOutputSurfList[frameIdx].OsResource;
                }
#endif
                m_allocator->UpdateResoreceUsageType(&inputParameters.currDecodedPicRes, resourceOutputPicture);
                m_statusReport->Init(&inputParameters);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineG12::Execute()
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

        if (m_pipeMode == decodePipeModeBegin)
        {
            return MOS_STATUS_SUCCESS;
        }

        auto basicFeature = dynamic_cast<Av1BasicFeatureG12*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(basicFeature);
        DECODE_CHK_NULL(basicFeature->m_av1PicParams);
        if (basicFeature->m_av1PicParams->m_anchorFrameInsertion)
        {
            return MOS_STATUS_SUCCESS;
        }

        if (m_pipeMode == decodePipeModeProcess)
        {
            DECODE_CHK_STATUS(m_preSubPipeline->Execute());

            if (IsCompleteBitstream())
            {
                DECODE_CHK_STATUS(InitContext());
                DECODE_CHK_STATUS(ActivateDecodePackets());
                DECODE_CHK_STATUS(ExecuteActivePackets());

                DECODE_CHK_STATUS(m_postSubPipeline->Execute());

                CODECHAL_DEBUG_TOOL(
                    PMHW_BATCH_BUFFER batchBuffer = m_av1DecodePkt->GetSecondLvlBB();
                    if (batchBuffer != nullptr)
                    {
                        batchBuffer->iLastCurrent = batchBuffer->iSize;
                        batchBuffer->dwOffset = 0;
                        DECODE_CHK_STATUS(m_debugInterface->Dump2ndLvlBatch(
                            batchBuffer,
                            CODECHAL_NUM_MEDIA_STATES,
                            "AV1_DEC_Secondary"));
                    })

#if (_DEBUG || _RELEASE_INTERNAL)
                DECODE_CHK_STATUS(StatusCheck());
#endif
                // Only update user features for the first frame.
                if (basicFeature->m_frameNum == 0)
                {
                    DECODE_CHK_STATUS(UserFeatureReport());
                }

                DecodeFrameIndex++;
                basicFeature->m_frameNum = DecodeFrameIndex;

                DECODE_CHK_STATUS(m_statusReport->Reset());
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineG12::GetStatusReport(void *status, uint16_t numStatus)
    {
        DECODE_FUNC_CALL();

        m_statusReport->GetReport(numStatus, status);

        return MOS_STATUS_SUCCESS;
    }

    uint32_t Av1PipelineG12::GetCompletedReport()
    {
        DECODE_FUNC_CALL();

        uint32_t completedCount = m_statusReport->GetCompletedCount();
        uint32_t reportedCount = m_statusReport->GetReportedCount();

        if (reportedCount > completedCount)
        {
            DECODE_ASSERTMESSAGE("No report available at all");
            return 0;
        }
        else
        {
            uint32_t availableCount = completedCount - reportedCount;
            return availableCount;
        }
    }

    MOS_STATUS Av1PipelineG12::Destroy()
    {
        DECODE_FUNC_CALL();

        Uninitialize();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineG12::Initialize(void *settings)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(Av1PipelineG12_Base::Initialize(settings));
        DECODE_CHK_STATUS(InitMmcState());

        auto *codecSettings     = (CodechalSetting *)settings;
        m_fgCoordValSurfInitPipeline = MOS_New(FilmGrainSurfaceInit, this, m_task, m_numVdbox, m_hwInterface);
        DECODE_CHK_NULL(m_fgCoordValSurfInitPipeline);
        DECODE_CHK_STATUS(m_preSubPipeline->Register(*m_fgCoordValSurfInitPipeline));
        DECODE_CHK_STATUS(m_fgCoordValSurfInitPipeline->Init(*codecSettings));

        //pre subpipeline for generate noise
        m_fgGenNoiseSubPipeline = MOS_New(FilmGrainPreSubPipeline, this, m_task, m_numVdbox, m_hwInterface);
        DECODE_CHK_NULL(m_fgGenNoiseSubPipeline);
        DECODE_CHK_STATUS(m_preSubPipeline->Register(*m_fgGenNoiseSubPipeline));
        DECODE_CHK_STATUS(m_fgGenNoiseSubPipeline->Init(*codecSettings));

        //post subpipeline for apply noise
        m_fgAppNoiseSubPipeline = MOS_New(FilmGrainPostSubPipeline, this, m_task, m_numVdbox, m_hwInterface);
        DECODE_CHK_NULL(m_fgAppNoiseSubPipeline);
        DECODE_CHK_STATUS(m_postSubPipeline->Register(*m_fgAppNoiseSubPipeline));
        DECODE_CHK_STATUS(m_fgAppNoiseSubPipeline->Init(*codecSettings));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineG12::Uninitialize()
    {
        DECODE_FUNC_CALL();

        for (auto pair : m_packetList)
        {
            pair.second->Destroy();
        }

#ifdef _MMC_SUPPORTED
        if (m_mmcState != nullptr)
        {
            MOS_Delete(m_mmcState);
        }
#endif

        return Av1PipelineG12_Base::Uninitialize();
    }

    MOS_STATUS Av1PipelineG12::UserFeatureReport()
    {
        DECODE_FUNC_CALL();

        return Av1PipelineG12_Base::UserFeatureReport();
    }

    MOS_STATUS Av1PipelineG12::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
    {
        DecodePredicationPktG12 *predicationPkt = MOS_New(DecodePredicationPktG12, this, m_hwInterface);
        DECODE_CHK_NULL(predicationPkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
            DecodePacketId(this, predicationSubPacketId), *predicationPkt));

        DecodeMarkerPktG12 *markerPkt = MOS_New(DecodeMarkerPktG12, this, m_hwInterface);
        DECODE_CHK_NULL(markerPkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
            DecodePacketId(this, markerSubPacketId), *markerPkt));

        Av1DecodePicPktG12 *pictureDecodePkt = MOS_New(Av1DecodePicPktG12, this, m_hwInterface);
        DECODE_CHK_NULL(pictureDecodePkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
                            DecodePacketId(this, av1PictureSubPacketId), *pictureDecodePkt));

        Av1DecodeTilePktG12 *tileDecodePkt = MOS_New(Av1DecodeTilePktG12, this, m_hwInterface);
        DECODE_CHK_NULL(tileDecodePkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
                            DecodePacketId(this, av1TileSubPacketId), *tileDecodePkt));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineG12::InitMmcState()
    {
    #ifdef _MMC_SUPPORTED
        DECODE_CHK_NULL(m_hwInterface);
        m_mmcState = MOS_New(DecodeMemCompG12, m_hwInterface);
        DECODE_CHK_NULL(m_mmcState);

        Av1BasicFeatureG12 *basicFeature = dynamic_cast<Av1BasicFeatureG12*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(basicFeature);
        DECODE_CHK_STATUS(basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
    #endif
        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Av1PipelineG12::DumpOutput(const DecodeStatusReportData& reportData)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(DecodePipeline::DumpOutput(reportData));

        auto feature = dynamic_cast<Av1BasicFeatureG12*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(feature);
        auto filmGrainFeature = dynamic_cast<Av1DecodeFilmGrainG12*>(m_featureManager->GetFeature(
            Av1FeatureIDs::av1SwFilmGrain));
        if (filmGrainFeature != nullptr && filmGrainFeature->m_filmGrainEnabled)
        {
            if (reportData.currFgOutputPicRes != nullptr)
            {
                MOS_SURFACE fgOutputSurface;
                MOS_ZeroMemory(&fgOutputSurface, sizeof(fgOutputSurface));
                fgOutputSurface.Format     = Format_NV12;
                fgOutputSurface.OsResource = *reportData.currFgOutputPicRes;
                if (!Mos_ResourceIsNull(&fgOutputSurface.OsResource))
                {
                    DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&fgOutputSurface));
                    DECODE_CHK_STATUS(m_debugInterface->DumpYUVSurface(
                        &fgOutputSurface, CodechalDbgAttr::attrFilmGrain, "FilmGrain"));
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }
#endif

    MOS_STATUS Av1PipelineG12::CreateFeatureManager()  
    {
        DECODE_FUNC_CALL();
        m_featureManager = MOS_New(DecodeAv1FeatureManagerG12, m_allocator, m_hwInterface, m_osInterface);
        DECODE_CHK_NULL(m_featureManager);
        return MOS_STATUS_SUCCESS;
    }
}
