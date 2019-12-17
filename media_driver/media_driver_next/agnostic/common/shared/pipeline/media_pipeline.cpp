/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_pipeline.cpp
//! \brief    Defines the common interface for media pipeline
//! \details  The media pipeline interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#include "media_pipeline.h"
#include "media_cmd_task.h"
#include "media_packet.h"
#include "media_perf_profiler.h"

MediaPipeline::MediaPipeline(PMOS_INTERFACE osInterface) : m_osInterface(osInterface)
{
    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();
    if (!perfProfiler)
    {
        MOS_OS_ASSERTMESSAGE("Initialize MediaPerfProfiler failed!");
    }
    else
    {
        perfProfiler->Initialize((void *)this, m_osInterface);
    }
}

MediaPipeline::~MediaPipeline()
{
    DeletePackets();
    DeleteTasks();

    CODECHAL_DEBUG_TOOL(MOS_Delete(m_debugInterface));

    MediaPerfProfiler *perfProfiler = MediaPerfProfiler::Instance();

    if (!perfProfiler)
    {
        MOS_OS_ASSERTMESSAGE("Destroy MediaPerfProfiler failed!");
    }
    else
    {
        MediaPerfProfiler::Destroy(perfProfiler, (void *)this, m_osInterface);
    }
}

MOS_STATUS MediaPipeline::DeletePackets()
{
    for (auto pair : m_packetList)
    {
        MOS_Delete(pair.second);
    }

    m_packetList.clear();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPipeline::DeleteTasks()
{
    for (auto pair : m_taskList)
    {
        MOS_Delete(pair.second);
    }

    m_taskList.clear();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPipeline::InitPlatform()
{
    m_osInterface->pfnGetPlatform(m_osInterface, &m_platform);
    m_skuTable     = m_osInterface->pfnGetSkuTable(m_osInterface);
    m_waTable      = m_osInterface->pfnGetWaTable(m_osInterface);
    m_gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPipeline::UserFeatureReport()
{
#if (_DEBUG || _RELEASE_INTERNAL)
    WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_ENABLE_ID, 1);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPipeline::RegisterPacket(uint32_t packetId, MediaPacket *packet)
{
    if (nullptr == packet)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    auto iter = m_packetList.find(packetId);
    if (iter != m_packetList.end())
    {
        m_packetList.erase(iter);
    }
    m_packetList.insert(std::make_pair(packetId, packet));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPipeline::ActivatePacket(uint32_t packetId, bool immediateSubmit, uint8_t pass, uint8_t pipe, uint8_t pipeNum, uint8_t subPass, uint8_t rowNum)
{
    auto iter = m_packetList.find(packetId);
    if (iter == m_packetList.end())
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    PacketProperty prop;
    prop.packetId        = iter->first;
    prop.packet          = iter->second;
    prop.immediateSubmit = immediateSubmit;

    prop.stateProperty.currentPass        = pass;
    prop.stateProperty.currentPipe        = pipe;
    prop.stateProperty.pipeIndexForSubmit = pipeNum;
    prop.stateProperty.currentSubPass     = subPass;
    prop.stateProperty.currentRow         = rowNum;

    m_activePacketList.push_back(prop);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaPipeline::ExecuteActivePackets()
{
    for (auto prop : m_activePacketList)
    {
        prop.stateProperty.statusReport = m_statusReport;

        MediaTask *task = prop.packet->GetActiveTask();

        if (task)
        {
            task->AddPacket(&prop);
            if (prop.immediateSubmit)
            {
                task->Submit(true, m_scalability, m_debugInterface);
            }
        }
    }

    m_activePacketList.clear();

    return MOS_STATUS_SUCCESS;
}

MediaTask *MediaPipeline::CreateTask(MediaTask::TaskType type)
{
    MediaTask *task = nullptr;
    switch (type)
    {
    case MediaTask::TaskType::cmdTask:
        task = MOS_New(CmdTask, m_osInterface);
        break;
    case MediaTask::TaskType::mdfTask:
        break;
    default:
        break;
    }
    if (nullptr != task)
    {
        m_taskList.insert(std::make_pair(type, task));
    }
    return task;
}

MediaTask *MediaPipeline::GetTask(MediaTask::TaskType type)
{
    auto iter = m_taskList.find(type);
    if (iter != m_taskList.end())
    {
        return iter->second;
    }
    else
    {
        MediaTask *task = CreateTask(type);
        return task;
    }
}

MOS_STATUS MediaPipeline::CreateFeatureManager()
{
    m_featureManager = MOS_New(MediaFeatureManager);
    if (nullptr != m_featureManager)
    {
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        return MOS_STATUS_UNKNOWN;
    }
}

bool MediaPipeline::IsFrameTrackingEnabled()
{
    if (nullptr == m_scalability)
        return false;

    return m_scalability->IsFrameTrackingEnabled();
}

