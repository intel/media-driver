/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_av1_pipeline_xe2_hpm.cpp
//! \brief    Defines the interface for av1 decode pipeline
//!
#include "decode_av1_pipeline_xe2_hpm.h"
#include "decode_av1_feature_manager_xe2_hpm.h"
#include "decode_av1_picture_packet_xe_lpm_plus_base.h"
#include "decode_av1_tile_packet_xe_lpm_plus_base.h"
#include "decode_av1_downsampling_packet_xe2_hpm.h"
#include "decode_mem_compression_xe2_hpm.h"

namespace decode
{
Av1PipelineXe2_Hpm::Av1PipelineXe2_Hpm(
        CodechalHwInterfaceNext *   hwInterface,
        CodechalDebugInterface *debugInterface)
    : Av1PipelineXe_Lpm_Plus_Base(hwInterface, debugInterface)
    {
        DECODE_FUNC_CALL()
    }

    MOS_STATUS Av1PipelineXe2_Hpm::Prepare(void *params)
    {
        DECODE_FUNC_CALL()

        DECODE_CHK_NULL(params);
        DecodePipelineParams *pipelineParams = (DecodePipelineParams *)params;
        m_pipeMode                           = pipelineParams->m_pipeMode;

        PERF_UTILITY_AUTO((__FUNCTION__ + std::to_string((int)m_pipeMode)).c_str(), PERF_DECODE, PERF_LEVEL_HAL);

        auto basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
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
#ifdef _DECODE_PROCESSING_SUPPORTED
                CODECHAL_DEBUG_TOOL(
                    DecodeDownSamplingFeature *downSamplingFeature = dynamic_cast<DecodeDownSamplingFeature *>(
                        m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
                    if (downSamplingFeature != nullptr) {
                        auto frameIdx                   = basicFeature->m_curRenderPic.FrameIdx;
                        inputParameters.sfcOutputSurface = &downSamplingFeature->m_outputSurfaceList[frameIdx];
                        if (downSamplingFeature->m_histogramBuffer != nullptr)
                        {
                            inputParameters.histogramOutputBuf = &downSamplingFeature->m_histogramBuffer->OsResource;
                        }
                        DumpDownSamplingParams(*downSamplingFeature);
                    });
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
                if (basicFeature->m_filmGrainEnabled)
                {
                    auto frameIdx                  = basicFeature->m_curRenderPic.FrameIdx;
                    inputParameters.fgOutputPicRes = &basicFeature->m_fgOutputSurfList[frameIdx].OsResource;
                }
#endif
                m_statusReport->Init(&inputParameters);
            }
        }

        return MOS_STATUS_SUCCESS;
        return MOS_STATUS();
    }

    MOS_STATUS Av1PipelineXe2_Hpm::CreateSubPackets(DecodeSubPacketManager & subPacketManager, CodechalSetting & codecSettings)
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
#ifdef _DECODE_PROCESSING_SUPPORTED
        Av1DownSamplingPktXe2_Hpm *downSamplingPkt = MOS_New(Av1DownSamplingPktXe2_Hpm, this, m_hwInterface);
        DECODE_CHK_NULL(downSamplingPkt);
        DECODE_CHK_STATUS(subPacketManager.Register(
            DecodePacketId(this, downSamplingSubPacketId), *downSamplingPkt));
#endif
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineXe2_Hpm::CreateFeatureManager()
    {
        DECODE_FUNC_CALL()
        m_featureManager = MOS_New(DecodeAv1FeatureManagerXe2_Hpm, m_allocator, m_hwInterface, m_osInterface);
        DECODE_CHK_NULL(m_featureManager);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1PipelineXe2_Hpm::InitMmcState()
    {
        DECODE_FUNC_CALL();

#ifdef _MMC_SUPPORTED
        DECODE_CHK_NULL(m_hwInterface);
        m_mmcState = MOS_New(DecodeMemCompXe2_Hpm, m_hwInterface);
        DECODE_CHK_NULL(m_mmcState);

        Av1BasicFeature *basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(basicFeature);
        DECODE_CHK_STATUS(basicFeature->SetMmcState(m_mmcState->IsMmcEnabled()));
#endif
        return MOS_STATUS_SUCCESS;
    }

}
