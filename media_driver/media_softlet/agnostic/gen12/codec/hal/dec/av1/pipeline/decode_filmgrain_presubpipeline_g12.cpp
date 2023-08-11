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
//! \file     decode_filmgrain_presubpipeline_g12.cpp
//! \brief    Defines the preSubpipeline for film grain generate noise
//! \details  Defines the preSubpipeline for film grain generate noise, including getRandomValues, regressPhase1, regressPhase2 kernels.
//!
#include "decode_filmgrain_presubpipeline_g12.h"
#include "decode_basic_feature.h"
#include "decode_pipeline.h"
#include "decode_av1_filmgrain_feature_g12.h"
#include "decode_av1_feature_defs_g12.h"

namespace decode {

FilmGrainPreSubPipeline::FilmGrainPreSubPipeline(DecodePipeline *pipeline, MediaTask *task, uint8_t numVdbox, CodechalHwInterface* hwInterface)
    : DecodeSubPipeline(pipeline, task, numVdbox)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS FilmGrainPreSubPipeline::Init(CodechalSetting &settings)
{
    DECODE_CHK_NULL(m_pipeline);

    CodechalHwInterface *hwInterface = m_hwInterface;
    DECODE_CHK_NULL(hwInterface);
    PMOS_INTERFACE osInterface = hwInterface->GetOsInterface();
    DECODE_CHK_NULL(osInterface);
    InitScalabilityPars(osInterface);

    m_allocator = m_pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    MediaFeatureManager* featureManager = m_pipeline->GetFeatureManager();
    DECODE_CHK_NULL(featureManager);
    m_basicFeature = dynamic_cast<DecodeBasicFeature*>(featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    m_filmGrainFeature = dynamic_cast<Av1DecodeFilmGrainG12 *>(featureManager->GetFeature(Av1FeatureIDs::av1SwFilmGrain));
    DECODE_CHK_NULL(m_filmGrainFeature);

    //Create Packets
    m_filmGrainGrvPkt        = MOS_New(FilmGrainGrvPacket, m_pipeline, m_task, hwInterface);
    Av1PipelineG12 *pipeline = dynamic_cast<Av1PipelineG12 *>(m_pipeline);
    DECODE_CHK_NULL(pipeline);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(pipeline, av1FilmGrainGrvPacketId), *m_filmGrainGrvPkt));
    DECODE_CHK_STATUS(m_filmGrainGrvPkt->Init());

    m_filmGrainRp1Pkt = MOS_New(FilmGrainRp1Packet, m_pipeline, m_task, hwInterface);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(pipeline, av1FilmGrainRp1PacketId), *m_filmGrainRp1Pkt));
    DECODE_CHK_STATUS(m_filmGrainRp1Pkt->Init());

    m_filmGrainRp2Pkt = MOS_New(FilmGrainRp2Packet, m_pipeline, m_task, hwInterface);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(pipeline, av1FilmGrainRp2PacketId), *m_filmGrainRp2Pkt));
    DECODE_CHK_STATUS(m_filmGrainRp2Pkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainPreSubPipeline::Prepare(DecodePipelineParams &params)
{
    if (params.m_pipeMode == decodePipeModeBegin)
    {
        DECODE_CHK_STATUS(Begin());
    }
    else if (params.m_pipeMode == decodePipeModeProcess)
    {
        DECODE_CHK_NULL(params.m_params);
        CodechalDecodeParams *decodeParams = params.m_params;
        DECODE_CHK_STATUS(DoFilmGrainGenerateNoise(*decodeParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainPreSubPipeline::Begin()
{
    DECODE_CHK_STATUS(DecodeSubPipeline::Reset());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainPreSubPipeline::DoFilmGrainGenerateNoise(const CodechalDecodeParams &decodeParams)
{
    if (m_filmGrainFeature->m_filmGrainEnabled)
    {
        //Step1: Get Random Values
        DECODE_CHK_STATUS(GetRandomValuesKernel(decodeParams));

        //Step2: regressPhase1
        DECODE_CHK_STATUS(RegressPhase1Kernel(decodeParams));

        //Step3: regressPhase2
        DECODE_CHK_STATUS(RegressPhase2Kernel(decodeParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainPreSubPipeline::GetRandomValuesKernel(const CodechalDecodeParams &decodeParams)
{
    Av1PipelineG12 *pipeline = dynamic_cast<Av1PipelineG12 *>(m_pipeline);
    DECODE_CHK_NULL(pipeline);
    DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(pipeline, av1FilmGrainGrvPacketId), true, 0, 0));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainPreSubPipeline::RegressPhase1Kernel(const CodechalDecodeParams &decodeParams)
{
    Av1PipelineG12 *pipeline = dynamic_cast<Av1PipelineG12 *>(m_pipeline);
    DECODE_CHK_NULL(pipeline);
    DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(pipeline, av1FilmGrainRp1PacketId), true, 0, 0));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainPreSubPipeline::RegressPhase2Kernel(const CodechalDecodeParams &decodeParams)
{
    Av1PipelineG12 *pipeline = dynamic_cast<Av1PipelineG12 *>(m_pipeline);
    DECODE_CHK_NULL(pipeline);
    DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(pipeline, av1FilmGrainRp2PacketId), true, 0, 0));

    return MOS_STATUS_SUCCESS;
}

MediaFunction FilmGrainPreSubPipeline::GetMediaFunction()
{
    if(!MEDIA_IS_SKU(m_pipeline->GetSkuTable(), FtrCCSNode))
    {
        return RenderGenericFunc;
    }

    return ComputeVppFunc;
}

void FilmGrainPreSubPipeline::InitScalabilityPars(PMOS_INTERFACE osInterface)
{
    m_decodeScalabilityPars.disableScalability = true;
    m_decodeScalabilityPars.disableRealTile = true;
    m_decodeScalabilityPars.enableVE = MOS_VE_SUPPORTED(osInterface);
    m_decodeScalabilityPars.numVdbox = m_numVdbox;
}

}
