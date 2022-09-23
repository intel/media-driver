/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_sfc_histogram_postsubpipeline_m12.cpp
//! \brief    Defines the postSubpipeline for decode SFC histogram
//! \details  Defines the postSubpipline for decode SFC histogram
//!

#include "decode_sfc_histogram_postsubpipeline_m12.h"
#include "decode_pipeline.h"
#include "decode_common_feature_defs.h"
#include "decode_huc_packet_creator_g12.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode {

DecodeSfcHistogramSubPipelineM12::DecodeSfcHistogramSubPipelineM12(DecodePipeline *pipeline,
    MediaTask *task,
    uint8_t numVdbox, 
    CodechalHwInterface *hwInterface):
    DecodeSfcHistogramSubPipeline(pipeline, task, numVdbox)
{
    m_hwInterface = hwInterface;
}

MOS_STATUS DecodeSfcHistogramSubPipelineM12::Init(CodechalSetting &settings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_pipeline);

    CodechalHwInterface *hwInterface = m_hwInterface;
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
    HucPacketCreatorG12 *hucPktCreator = dynamic_cast<HucPacketCreatorG12 *>(m_pipeline);
    DECODE_CHK_NULL(hucPktCreator);
    m_copyPkt = hucPktCreator->CreateHucCopyPkt(m_pipeline, m_task, hwInterface);
    DECODE_CHK_NULL(m_copyPkt);
    MediaPacket *packet = dynamic_cast<MediaPacket *>(m_copyPkt);
    DECODE_CHK_NULL(packet);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(m_pipeline, hucCopyPacketId), *packet));
    DECODE_CHK_STATUS(packet->Init());

    return MOS_STATUS_SUCCESS;
}

}

#endif
