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
//! \file     decode_sfc_histogram_postsubpipeline.cpp
//! \brief    Defines the postSubpipeline for decode SFC histogram
//! \details  Defines the postSubpipline for decode SFC histogram
//!

#include "decode_pipeline.h"
#include "decode_common_feature_defs.h"
#include "decode_sfc_histogram_postsubpipeline.h"
#include "decode_huc_packet_creator_base.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode {

DecodeSfcHistogramSubPipeline::DecodeSfcHistogramSubPipeline(DecodePipeline *pipeline,
    MediaTask *task,
    uint8_t numVdbox):
    DecodeSubPipeline(pipeline, task, numVdbox)
{}

MOS_STATUS DecodeSfcHistogramSubPipeline::Init(CodechalSetting &settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_pipeline);

    CodechalHwInterfaceNext *hwInterface = m_pipeline->GetHwInterface();
    DECODE_CHK_NULL(hwInterface);
    m_osInterface = hwInterface->GetOsInterface();
    DECODE_CHK_NULL(m_osInterface);
    InitScalabilityPars(m_osInterface);

    m_allocator = m_pipeline->GetDecodeAllocator();
    DECODE_CHK_NULL(m_allocator);

    auto featureManger = m_pipeline->GetFeatureManager();
    DECODE_CHK_NULL(featureManger);
    m_basicFeature = dynamic_cast<DecodeBasicFeature*>(featureManger->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    m_downsampFeature = dynamic_cast<DecodeDownSamplingFeature*>(featureManger->GetFeature(
        DecodeFeatureIDs::decodeDownSampling));

    //Create Packets
    HucPacketCreatorBase *hucPktCreator = dynamic_cast<HucPacketCreatorBase *>(m_pipeline);
    DECODE_CHK_NULL(hucPktCreator);
    m_copyPkt = hucPktCreator->CreateHucCopyPkt(m_pipeline, m_task, hwInterface);
    DECODE_CHK_NULL(m_copyPkt);
    MediaPacket *packet = dynamic_cast<MediaPacket *>(m_copyPkt);
    DECODE_CHK_NULL(packet);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(m_pipeline, hucCopyPacketId), *packet));
    DECODE_CHK_STATUS(packet->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeSfcHistogramSubPipeline::Prepare(DecodePipelineParams &params)
{
    DECODE_FUNC_CALL();
    if (params.m_pipeMode == decodePipeModeBegin)
    {
        DECODE_CHK_STATUS(Begin());
    }
    else if (params.m_pipeMode == decodePipeModeProcess &&
             m_downsampFeature != nullptr && m_downsampFeature->m_histogramBuffer != nullptr)  //m_downsampFeature could be null if downsampling is not enabled
    {
        DECODE_CHK_NULL(params.m_params);
        DECODE_CHK_NULL(m_basicFeature);
        DECODE_CHK_NULL(m_downsampFeature->m_histogramBuffer);

        CodechalDecodeParams*   decodeParams    = params.m_params;
        PMOS_RESOURCE           src             = &m_downsampFeature->m_histogramBuffer->OsResource;
        PMOS_RESOURCE           dest            = &decodeParams->m_histogramSurface.OsResource;
        uint32_t                destOffset      = decodeParams->m_histogramSurface.dwOffset;
        if (!m_allocator->ResourceIsNull(src) &&
            !m_allocator->ResourceIsNull(dest))
        {
            DECODE_CHK_STATUS(CopyHistogramToDestBuf(src, dest, destOffset));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeSfcHistogramSubPipeline::Begin()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodeSubPipeline::Reset());

    return MOS_STATUS_SUCCESS;
}

MediaFunction DecodeSfcHistogramSubPipeline::GetMediaFunction()
{
    DECODE_FUNC_CALL();
    return VdboxDecodeWaFunc;
}

void DecodeSfcHistogramSubPipeline::InitScalabilityPars(PMOS_INTERFACE osInterface)
{
    DECODE_FUNC_CALL();
    m_decodeScalabilityPars.disableScalability = true;
    m_decodeScalabilityPars.disableRealTile = true;
    m_decodeScalabilityPars.enableVE = MOS_VE_SUPPORTED(osInterface);
    m_decodeScalabilityPars.numVdbox = m_numVdbox;
}

MOS_STATUS DecodeSfcHistogramSubPipeline::CopyHistogramToDestBuf(MOS_RESOURCE* src,
    MOS_RESOURCE* dest,
    uint32_t destOffset)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(src);
    DECODE_CHK_NULL(dest);

    DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(m_pipeline, hucCopyPacketId), true, 0, 0));

    HucCopyPktItf::HucCopyParams copyParams;
    copyParams.srcBuffer    = src;
    copyParams.srcOffset    = 0;
    copyParams.destBuffer   = dest;
    copyParams.destOffset   = destOffset;
    copyParams.copyLength   = HISTOGRAM_BINCOUNT * m_downsampFeature->m_histogramBinWidth;
    DECODE_CHK_STATUS(m_copyPkt->PushCopyParams(copyParams));

    return MOS_STATUS_SUCCESS;
}


}

#endif
