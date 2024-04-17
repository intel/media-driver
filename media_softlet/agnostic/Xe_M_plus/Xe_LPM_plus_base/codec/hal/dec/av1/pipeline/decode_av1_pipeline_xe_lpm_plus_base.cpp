/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     decode_av1_pipeline_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for av1 decode pipeline
//!
#include "decode_av1_pipeline_xe_lpm_plus_base.h"
#include "decode_av1_packet_xe_lpm_plus_base.h"
#include "decode_av1_picture_packet_xe_lpm_plus_base.h"
#include "decode_av1_tile_packet_xe_lpm_plus_base.h"
#include "decode_utils.h"
#include "codechal_debug.h"
#include "decode_av1_tile_coding.h"
#include "decode_av1_feature_manager_xe_lpm_plus_base.h"
#include "decode_mem_compression_xe_lpm_plus_base.h"
#include "decode_av1_status_report_xe_lpm_plus_base.h"

namespace decode
{
    Av1PipelineXe_Lpm_Plus_Base::Av1PipelineXe_Lpm_Plus_Base(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface)
        : Av1Pipeline(hwInterface, debugInterface)
    {
        DECODE_FUNC_CALL()
    }

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::Init(void *settings)
    {
        DECODE_FUNC_CALL()

        DECODE_CHK_NULL(settings);
        DECODE_CHK_STATUS(Initialize(settings));

        if (MEDIA_IS_SKU(m_skuTable, FtrWithSlimVdbox))
        {
            m_numVdbox = 1;
        }

        m_av1DecodePkt = MOS_New(Av1DecodePktXe_Lpm_Plus_Base, this, m_task, m_hwInterface);
        DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(this, av1DecodePacketId), m_av1DecodePkt));
        DECODE_CHK_STATUS(m_av1DecodePkt->Init());

        if (m_numVdbox == 2)
        {
            m_allowVirtualNodeReassign = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::InitContext()
    {
        DECODE_FUNC_CALL()

        auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(basicFeature);

        DecodeScalabilityPars scalPars;
        MOS_ZeroMemory(&scalPars, sizeof(ScalabilityPars));
        scalPars.disableRealTile = true;
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

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::Prepare(void *params)
    {
        DECODE_FUNC_CALL()

        DECODE_CHK_NULL(params);
        DecodePipelineParams *pipelineParams = (DecodePipelineParams *)params;
        m_pipeMode = pipelineParams->m_pipeMode;

        PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

        auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(basicFeature);

        if (IsFirstProcessPipe(*pipelineParams))
        {
            DECODE_CHK_STATUS(Av1Pipeline::Prepare(params));
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
#ifdef _DECODE_PROCESSING_SUPPORTED
                CODECHAL_DEBUG_TOOL(
                    DecodeDownSamplingFeature *downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature *>(
                        m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
                    if (downSamplingFeature != nullptr) {
                        auto frameIdx                   = basicFeature->m_curRenderPic.FrameIdx;
                        inputParameters.sfcOutputSurface = &downSamplingFeature->m_outputSurfaceList[frameIdx];
                        DumpDownSamplingParams(*downSamplingFeature);
                    });
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
                if (basicFeature->m_filmGrainEnabled)
                {
                    auto frameIdx = basicFeature->m_curRenderPic.FrameIdx;
                    inputParameters.fgOutputPicRes = &basicFeature->m_fgOutputSurfList[frameIdx].OsResource;
                }
#endif
                m_statusReport->Init(&inputParameters);
            }
        }
     
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::Execute()
    {
        DECODE_FUNC_CALL()

        PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

        if (m_pipeMode == decodePipeModeBegin)
        {
            return MOS_STATUS_SUCCESS;
        }

        auto basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
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
#ifdef _MMC_SUPPORTED
                if (m_mmcState != nullptr)
                {
                    m_mmcState->ReportSurfaceMmcMode(&(basicFeature->m_destSurface));
                }
#endif
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

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::GetStatusReport(void *status, uint16_t numStatus)
    {
        DECODE_FUNC_CALL()

        m_statusReport->GetReport(numStatus, status);

        return MOS_STATUS_SUCCESS;
    }

    uint32_t Av1PipelineXe_Lpm_Plus_Base::GetCompletedReport()
    {
        DECODE_FUNC_CALL()

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

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::Destroy()
    {
        DECODE_FUNC_CALL()

        Uninitialize();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::Initialize(void *settings)
    {
        DECODE_FUNC_CALL()

        DECODE_CHK_STATUS(Av1Pipeline::Initialize(settings));
        DECODE_CHK_STATUS(InitMmcState());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::Uninitialize()
    {
        DECODE_FUNC_CALL()

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

        return Av1Pipeline::Uninitialize();
    }

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::UserFeatureReport()
    {
        DECODE_FUNC_CALL()

        return Av1Pipeline::UserFeatureReport();
    }

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::CreateSubPackets(DecodeSubPacketManager &subPacketManager, CodechalSetting &codecSettings)
    {
        DECODE_CHK_STATUS(Av1Pipeline::CreateSubPackets(subPacketManager, codecSettings));

        Av1DecodePicPktXe_Lpm_Plus_Base *pictureDecodePkt = MOS_New(Av1DecodePicPktXe_Lpm_Plus_Base, this, m_hwInterface);
        DECODE_CHK_NULL(pictureDecodePkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
                            DecodePacketId(this, av1PictureSubPacketId), *pictureDecodePkt));

        Av1DecodeTilePktXe_Lpm_Plus_Base *tileDecodePkt = MOS_New(Av1DecodeTilePktXe_Lpm_Plus_Base, this, m_hwInterface);
        DECODE_CHK_NULL(tileDecodePkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
                            DecodePacketId(this, av1TileSubPacketId), *tileDecodePkt));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::InitMmcState()
    {
    #ifdef _MMC_SUPPORTED
        DECODE_CHK_NULL(m_hwInterface);
        m_mmcState = MOS_New(DecodeMemCompXe_Lpm_Plus_Base, m_hwInterface);
        DECODE_CHK_NULL(m_mmcState);

        Av1BasicFeature *basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(basicFeature);
        DECODE_CHK_STATUS(basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
    #endif
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::CreateStatusReport()
    {
        m_statusReport = MOS_New(DecodeAv1StatusReportXe_Lpm_Plus_Base, m_allocator, true, m_osInterface);
        DECODE_CHK_NULL(m_statusReport);
        DECODE_CHK_STATUS(m_statusReport->Create());

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::DumpOutput(const DecodeStatusReportData& reportData)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(DecodePipeline::DumpOutput(reportData));

        auto feature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(feature);
        m_debugInterface->m_bufferDumpFrameNum = feature->m_frameNum;
        auto basicFeature = dynamic_cast<Av1BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        if (basicFeature->m_filmGrainEnabled)
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

    MOS_STATUS Av1PipelineXe_Lpm_Plus_Base::CreateFeatureManager()
    {
        DECODE_FUNC_CALL()
        m_featureManager = MOS_New(DecodeAv1FeatureManagerXe_Lpm_Plus_Base, m_allocator, m_hwInterface, m_osInterface);
        DECODE_CHK_NULL(m_featureManager);
        return MOS_STATUS_SUCCESS;
    }

}
