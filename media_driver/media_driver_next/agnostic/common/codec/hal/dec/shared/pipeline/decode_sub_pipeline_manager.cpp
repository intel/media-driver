/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_sub_pipeline_manager.cpp
//! \brief    Defines the common interface for decode sub pipeline manager
//! \details  Defines the common interface for decode sub pipeline manager
//!
#include "decode_sub_pipeline.h"
#include "decode_pipeline.h"
#include "decode_utils.h"
#include "media_packet.h"

namespace decode {

DecodeSubPipelineManager::DecodeSubPipelineManager(DecodePipeline& decodePipeline)
{
    m_decodePipeline = &decodePipeline;
}

DecodeSubPipelineManager::~DecodeSubPipelineManager()
{
    Destroy(m_subPipelineList);
}

MOS_STATUS DecodeSubPipelineManager::Register(DecodeSubPipeline& subPipeline)
{
    m_subPipelineList.push_back(&subPipeline);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeSubPipelineManager::Init(CodechalSetting& settings)
{
    for (auto subPipeline : m_subPipelineList)
    {
        DECODE_CHK_STATUS(subPipeline->Init(settings));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeSubPipelineManager::Prepare(DecodePipelineParams& params)
{
    for (auto subPipeline : m_subPipelineList)
    {
        DECODE_CHK_STATUS(subPipeline->Prepare(params));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeSubPipelineManager::ExecuteSubPipeline(DecodeSubPipeline &subPipeline)
{
    auto& subActivePackets = subPipeline.GetActivePackets();
    if (subActivePackets.size() == 0)
    {
        return MOS_STATUS_SUCCESS;
    }

    MediaContext *mediaContext = m_decodePipeline->GetMediaContext();
    DECODE_CHK_NULL(mediaContext);

    MediaFunction mediaFunction = subPipeline.GetMediaFunction();
    DecodeScalabilityPars& scalPars = subPipeline.GetScalabilityPars();
    auto& scalability = m_decodePipeline->GetMediaScalability();
    DECODE_CHK_STATUS(mediaContext->SwitchContext(mediaFunction, &scalPars, &scalability));

    auto& subPacketList = subPipeline.GetPacketList();
    std::swap(subPacketList, m_decodePipeline->m_packetList);
    std::swap(subActivePackets, m_decodePipeline->m_activePacketList);

    DECODE_CHK_STATUS(m_decodePipeline->ExecuteActivePackets());

    std::swap(subPacketList, m_decodePipeline->m_packetList);
    std::swap(subActivePackets, m_decodePipeline->m_activePacketList);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeSubPipelineManager::Execute()
{
    for (auto subPipeline : m_subPipelineList)
    {
        DECODE_CHK_STATUS(ExecuteSubPipeline(*subPipeline));
    }
    return MOS_STATUS_SUCCESS;
}

void DecodeSubPipelineManager::Destroy(std::vector<DecodeSubPipeline *> &subPipelineList)
{
    for (auto subPipeline : subPipelineList)
    {
        MOS_Delete(subPipeline);
    }
    subPipelineList.clear();
}

}
