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
//! \file     decode_input_bitstream.cpp
//! \brief    Defines the common interface for decode input bitstream
//! \details  Defines the interface to handle the decode input bitstream in
//!           both single execution call mode and multiple excution call mode.
//!
#include "decode_input_bitstream.h"
#include "decode_basic_feature.h"
#include "decode_pipeline.h"
#include "decode_huc_packet_creator_base.h"

namespace decode {

DecodeInputBitstream::DecodeInputBitstream(DecodePipeline* pipeline, MediaTask* task, uint8_t numVdbox)
    : DecodeSubPipeline(pipeline, task, numVdbox)
{}

DecodeInputBitstream::~DecodeInputBitstream()
{
    if (m_allocator)
    {
        m_allocator->Destroy(m_catenatedBuffer);
    }
}

MOS_STATUS DecodeInputBitstream::Init(CodechalSetting& settings)
{
    DECODE_CHK_NULL(m_pipeline);

    CodechalHwInterfaceNext *hwInterface = m_pipeline->GetHwInterface();
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

    HucPacketCreatorBase *hucPktCreator = dynamic_cast<HucPacketCreatorBase *>(m_pipeline);
    DECODE_CHK_NULL(hucPktCreator);
    m_concatPkt = hucPktCreator->CreateHucCopyPkt(m_pipeline, m_task, hwInterface);  
    DECODE_CHK_NULL(m_concatPkt);
    MediaPacket *packet = dynamic_cast<MediaPacket *>(m_concatPkt);
    DECODE_CHK_NULL(packet);
    DECODE_CHK_STATUS(RegisterPacket(DecodePacketId(m_pipeline, hucCopyPacketId), *packet));
    DECODE_CHK_STATUS(packet->Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeInputBitstream::Prepare(DecodePipelineParams& params)
{
    if (params.m_pipeMode == decodePipeModeBegin)
    {
        DECODE_CHK_STATUS(Begin());
    }
    else if (params.m_pipeMode == decodePipeModeProcess)
    {
        DECODE_CHK_NULL(params.m_params);
        CodechalDecodeParams *decodeParams = params.m_params;
        DECODE_CHK_STATUS(Append(*decodeParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeInputBitstream::Begin()
{
    DECODE_CHK_STATUS(DecodeSubPipeline::Reset());

    m_segmentsTotalSize = 0;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeInputBitstream::AllocateCatenatedBuffer()
{
    DECODE_CHK_NULL(m_allocator);

    uint32_t allocSize = MOS_ALIGN_CEIL(m_requiredSize, MHW_CACHELINE_SIZE);

    if (m_catenatedBuffer == nullptr)
    {
        m_catenatedBuffer = m_allocator->AllocateBuffer(
            allocSize, "bitstream", resourceInputBitstream, notLockableVideoMem);
        DECODE_CHK_NULL(m_catenatedBuffer);
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_STATUS(m_allocator->Resize(m_catenatedBuffer, allocSize, notLockableVideoMem));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeInputBitstream::Append(const CodechalDecodeParams &decodeParams)
{
    uint32_t segmentSize = decodeParams.m_dataSize;

    bool firstExecuteCall = (decodeParams.m_executeCallIndex == 0);
    if (firstExecuteCall)
    {
        m_requiredSize = m_basicFeature->m_dataSize;
        bool isIncompleteBitstream = (segmentSize < m_requiredSize);
        if (isIncompleteBitstream)
        {
            DECODE_CHK_STATUS(AllocateCatenatedBuffer());
            m_basicFeature->m_resDataBuffer = *m_catenatedBuffer;
            m_basicFeature->m_dataOffset = 0;
            DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(m_pipeline, hucCopyPacketId), true, 0, 0));
            AddNewSegment(*(decodeParams.m_dataBuffer), decodeParams.m_dataOffset, decodeParams.m_dataSize);
        }
    }
    else
    {
        if(m_segmentsTotalSize + segmentSize > m_requiredSize)
        {
            DECODE_ASSERTMESSAGE("Bitstream size exceeds allocated buffer size!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        DECODE_CHK_STATUS(ActivatePacket(DecodePacketId(m_pipeline, hucCopyPacketId), true, 0, 0));
        AddNewSegment(*(decodeParams.m_dataBuffer), decodeParams.m_dataOffset, decodeParams.m_dataSize);
    }

    m_segmentsTotalSize += MOS_ALIGN_CEIL(segmentSize, MHW_CACHELINE_SIZE);

    return MOS_STATUS_SUCCESS;
}

void DecodeInputBitstream::AddNewSegment(MOS_RESOURCE& resource, uint32_t offset, uint32_t size)
{
    HucCopyPktItf::HucCopyParams copyParams;
    copyParams.srcBuffer    = &resource;
    copyParams.srcOffset    = offset;
    copyParams.destBuffer   = &(m_catenatedBuffer->OsResource);
    copyParams.destOffset   = m_segmentsTotalSize;
    copyParams.copyLength   = size;
    m_concatPkt->PushCopyParams(copyParams);
}

bool DecodeInputBitstream::IsComplete()
{
    return (m_segmentsTotalSize >= m_requiredSize);
}

MediaFunction DecodeInputBitstream::GetMediaFunction()
{
    return VdboxDecodeWaFunc;
}

void DecodeInputBitstream::InitScalabilityPars(PMOS_INTERFACE osInterface)
{
    m_decodeScalabilityPars.disableScalability = true;
    m_decodeScalabilityPars.disableRealTile = true;
    m_decodeScalabilityPars.enableVE = MOS_VE_SUPPORTED(osInterface);
    m_decodeScalabilityPars.numVdbox = m_numVdbox;
}

}
