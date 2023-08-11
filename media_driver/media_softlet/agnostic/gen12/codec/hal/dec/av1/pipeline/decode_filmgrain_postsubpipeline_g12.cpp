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
//! \file     decode_filmgrain_postsubpipeline_g12.cpp
//! \brief    Defines the postSubpipeline for film grain
//! \details  Defines the postSubpipline for film grain to execute apply noise kernel
//!

#include "decode_filmgrain_postsubpipeline_g12.h"
#include "decode_basic_feature.h"
#include "decode_pipeline.h"
#include "decode_av1_filmgrain_feature_g12.h"
#include "decode_av1_feature_defs_g12.h"

namespace decode {

FilmGrainPostSubPipeline::FilmGrainPostSubPipeline(DecodePipeline *pipeline, MediaTask *task, uint8_t numVdbox, CodechalHwInterface *hwInterface)
    : DecodeSubPipeline(pipeline, task, numVdbox)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS FilmGrainPostSubPipeline::Init(CodechalSetting &settings)
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
    m_filmGrainAppPkt = MOS_New(FilmGrainAppNoisePkt, m_pipeline, m_task, hwInterface);
    Av1PipelineG12 *pipeline = dynamic_cast<Av1PipelineG12 *>(m_pipeline);
    DECODE_CHK_NULL(pipeline);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(pipeline, av1FilmGrainAppPacketId), *m_filmGrainAppPkt));
    DECODE_CHK_STATUS(m_filmGrainAppPkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainPostSubPipeline::Prepare(DecodePipelineParams &params)
{
    if (params.m_pipeMode == decodePipeModeBegin)
    {
        DECODE_CHK_STATUS(Begin());
    }
    else if (params.m_pipeMode == decodePipeModeProcess)
    {
        DECODE_CHK_NULL(params.m_params);
        CodechalDecodeParams *decodeParams = params.m_params;
        DECODE_CHK_STATUS(DoFilmGrainApplyNoise(*decodeParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainPostSubPipeline::Begin()
{
    DECODE_CHK_STATUS(DecodeSubPipeline::Reset());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainPostSubPipeline::DoFilmGrainApplyNoise(const CodechalDecodeParams &decodeParams)
{
    /*DON't use m_filmGrainFeature->m_filmGrainEnabled*/
    if (m_filmGrainFeature->m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain)
    {
        Av1PipelineG12 *pipeline = dynamic_cast<Av1PipelineG12 *>(m_pipeline);
        DECODE_CHK_NULL(pipeline);
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(pipeline, av1FilmGrainAppPacketId), true, 0, 0));
        // For film grain frame, apply noise packet should update report global count, so need to set
        // frameTrackingRequested flag to true.
        m_activePacketList.back().frameTrackingRequested = true;
    }

    return MOS_STATUS_SUCCESS;
}

MediaFunction FilmGrainPostSubPipeline::GetMediaFunction()
{
    if(!MEDIA_IS_SKU(m_pipeline->GetSkuTable(), FtrCCSNode))
    {
        return RenderGenericFunc;
    }

    return ComputeVppFunc;
}

void FilmGrainPostSubPipeline::InitScalabilityPars(PMOS_INTERFACE osInterface)
{
    m_decodeScalabilityPars.disableScalability = true;
    m_decodeScalabilityPars.disableRealTile = true;
    m_decodeScalabilityPars.enableVE = MOS_VE_SUPPORTED(osInterface);
    m_decodeScalabilityPars.numVdbox = m_numVdbox;
}

}
