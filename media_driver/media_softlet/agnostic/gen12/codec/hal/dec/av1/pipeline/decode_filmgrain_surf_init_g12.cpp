/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_filmgrain_surf_init_g12.cpp
//! \brief    Defines the preSubpipeline for film grain surface init
//! \details  Defines the preSubpipeline for film grain surface init, initialize coordinate surface with 0 per kernel requirement.
//!
#include "decode_filmgrain_surf_init_g12.h"
#include "decode_av1_feature_defs_g12.h"
#include "decode_pipeline.h"
#include "decode_av1_filmgrain_feature_g12.h"

namespace decode {

FilmGrainSurfaceInit::FilmGrainSurfaceInit(DecodePipeline *pipeline, MediaTask *task, uint8_t numVdbox, CodechalHwInterface *hwInterface)
    : DecodeSubPipeline(pipeline, task, numVdbox)
{
    m_hwInterface = hwInterface;
}

FilmGrainSurfaceInit::~FilmGrainSurfaceInit()
{
    m_allocator->Destroy(m_tmpInitBuf);
}

MOS_STATUS FilmGrainSurfaceInit::Init(CodechalSetting &settings)
{
    DECODE_CHK_NULL(m_pipeline);

    CodechalHwInterface* hwInterface = m_hwInterface;
    DECODE_CHK_NULL(hwInterface);
    PMOS_INTERFACE osInterface = hwInterface->GetOsInterface();
    DECODE_CHK_NULL(osInterface);
    InitScalabilityPars(osInterface);

    m_allocator = m_pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    MediaFeatureManager* featureManager = m_pipeline->GetFeatureManager();
    DECODE_CHK_NULL(featureManager);
    m_filmGrainFeature = dynamic_cast<Av1DecodeFilmGrainG12 *>(featureManager->GetFeature(Av1FeatureIDs::av1SwFilmGrain));
    DECODE_CHK_NULL(m_filmGrainFeature);

    m_surfInitPkt             = MOS_New(HucCopyPktG12, m_pipeline, m_task, hwInterface);
    DECODE_CHK_NULL(m_surfInitPkt);
    Av1PipelineG12 *pipeline = dynamic_cast<Av1PipelineG12 *>(m_pipeline);
    DECODE_CHK_NULL(pipeline);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(pipeline, hucCopyPacketId), *m_surfInitPkt));
    DECODE_CHK_STATUS(m_surfInitPkt->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainSurfaceInit::Prepare(DecodePipelineParams &params)
{
    if (params.m_pipeMode == decodePipeModeBegin)
    {
        DECODE_CHK_STATUS(Begin());
    }
    else if (params.m_pipeMode == decodePipeModeProcess)
    {
        /*DON't use m_filmGrainFeature->m_filmGrainEnabled*/
        if (m_filmGrainFeature->m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain)
        {
            InitCoordinateSurface();
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainSurfaceInit::Begin()
{
    DECODE_CHK_STATUS(DecodeSubPipeline::Reset());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS FilmGrainSurfaceInit::InitCoordinateSurface()
{
    if (m_filmGrainFeature->m_coordinatesRandomValuesSurface)
    {
        uint32_t allocSize = m_filmGrainFeature->m_coordinateSurfaceSize;
        if (m_tmpInitBuf == nullptr)
        {
            m_tmpInitBuf = m_allocator->AllocateBuffer(
                allocSize, "tempInitializationBuffer", resourceInternalReadWriteCache, lockableVideoMem, true, 0);
            DECODE_CHK_NULL(m_tmpInitBuf);
        }
        else
        {
            DECODE_CHK_STATUS(m_allocator->Resize(m_tmpInitBuf, allocSize, lockableVideoMem, false, true));
        }

        HucCopyPktG12::HucCopyParams copyParams;
        copyParams.srcBuffer  = &(m_tmpInitBuf->OsResource);
        copyParams.srcOffset  = 0;
        copyParams.destBuffer = &(m_filmGrainFeature->m_coordinatesRandomValuesSurface->OsResource);
        copyParams.destOffset = 0;
        copyParams.copyLength = allocSize;
        m_surfInitPkt->PushCopyParams(copyParams);

        Av1PipelineG12 *pipeline = dynamic_cast<Av1PipelineG12 *>(m_pipeline);
        DECODE_CHK_NULL(pipeline);
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(pipeline, hucCopyPacketId), true, 0, 0));
    }

    return MOS_STATUS_SUCCESS;
}

MediaFunction FilmGrainSurfaceInit::GetMediaFunction()
{
    return VdboxDecodeWaFunc;
}

void FilmGrainSurfaceInit::InitScalabilityPars(PMOS_INTERFACE osInterface)
{
    m_decodeScalabilityPars.disableScalability = true;
    m_decodeScalabilityPars.disableRealTile    = true;
    m_decodeScalabilityPars.enableVE           = MOS_VE_SUPPORTED(osInterface);
    m_decodeScalabilityPars.numVdbox           = m_numVdbox;
}

}
