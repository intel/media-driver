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
#include "vp_cmd_packet.h"
#include "vp_utils.h"
#include "vp_packet_pipe.h"
#include "media_task.h"
#include "media_context.h"

using namespace vp;

PacketFactory::PacketFactory()
{
}

PacketFactory::~PacketFactory()
{
    ClearPacketPool(m_VeboxPacketPool);
    ClearPacketPool(m_RenderPacketPool);
}

void PacketFactory::ClearPacketPool(std::vector<VpCmdPacket *> &pool)
{
    while (!pool.empty())
    {
        VpCmdPacket *p = pool.back();
        pool.pop_back();
        MOS_Delete(p);
    }
}

MOS_STATUS PacketFactory::Initialize(MediaTask *pTask, PVP_MHWINTERFACE pHwInterface, PVpAllocator pAllocator, VPMediaMemComp *pMmc)
{
    m_pTask = pTask;
    m_pHwInterface = pHwInterface;
    m_pAllocator = pAllocator;
    m_pMmc = pMmc;

    return MOS_STATUS_SUCCESS;
}

VpCmdPacket *PacketFactory::CreatePacket(EngineType type)
{
    switch(type)
    {
    case EngineTypeVebox:
    case EngineTypeSfc:
        if (!m_VeboxPacketPool.empty())
        {
            VpCmdPacket *p = m_VeboxPacketPool.back();
            m_VeboxPacketPool.pop_back();
            return p;
        }
        return CreateVeboxPacket();
    case EngineTypeRender:
        if (!m_RenderPacketPool.empty())
        {
            VpCmdPacket *p = m_RenderPacketPool.back();
            m_RenderPacketPool.pop_back();
            return p;
        }
        return CreateRenderPacket();
    default:
        return nullptr;
    }
}

void PacketFactory::ReturnPacket(VpCmdPacket *&pPacket)
{
    if (nullptr == pPacket)
    {
        return;
    }
    PacketType type = pPacket->GetPacketId();
    switch (type)
    {
    case VP_PIPELINE_PACKET_FF:
        m_VeboxPacketPool.push_back(pPacket);
        break;
    case VP_PIPELINE_PACKET_COMP:
        m_RenderPacketPool.push_back(pPacket);
        break;
    default:
        break;
    }
    pPacket = nullptr;
}

PacketPipe::PacketPipe(PacketFactory &packetFactory) : m_PacketFactory(packetFactory)
{
}

PacketPipe::~PacketPipe()
{
    Clean();
}

MOS_STATUS PacketPipe::Clean()
{
    for (std::vector<VpCmdPacket *>::iterator it = m_Pipe.begin(); it != m_Pipe.end(); ++it)
    {
        m_PacketFactory.ReturnPacket(*it);
    }
    m_Pipe.clear();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PacketPipe::AddPacket(HwFilter &hwFilter)
{
    VpCmdPacket *pPacket = m_PacketFactory.CreatePacket(hwFilter.GetEngineType());
    VP_PUBLIC_CHK_NULL_RETURN(pPacket);

    if (MOS_FAILED(hwFilter.SetPacketParams(*pPacket)))
    {
        MOS_OS_ASSERTMESSAGE("SetPacketParams failed!");
        m_PacketFactory.ReturnPacket(pPacket);
        return MOS_STATUS_UNKNOWN;
    }
    m_Pipe.push_back(pPacket);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PacketPipe::SwitchContext(PacketType type, MediaScalability *&scalability, MediaContext *mediaContext, bool bEnableVirtualEngine, uint8_t numVebox)
{
    ScalabilityPars scalPars = {};
    switch (type)
    {
    case VP_PIPELINE_PACKET_FF:
        {
            VP_PUBLIC_NORMALMESSAGE("Switch to Vebox Context");

            scalPars.enableVE = bEnableVirtualEngine;
            scalPars.numVebox = numVebox;

            VP_PUBLIC_CHK_STATUS_RETURN(mediaContext->SwitchContext(VeboxVppFunc, &scalPars, &scalability));
            VP_PUBLIC_CHK_NULL_RETURN(scalability);
            break;
        }
    case VP_PIPELINE_PACKET_COMP:
        {
            VP_PUBLIC_NORMALMESSAGE("Switch to Render Context");
            VP_PUBLIC_CHK_STATUS_RETURN(mediaContext->SwitchContext(RenderGenericFunc, &scalPars, &scalability));
            VP_PUBLIC_CHK_NULL_RETURN(scalability);
            break;
        }
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PacketPipe::Execute(MediaStatusReport *statusReport, MediaScalability *&scalability, MediaContext *mediaContext, bool bEnableVirtualEngine, uint8_t numVebox)
{
    for (std::vector<VpCmdPacket *>::iterator it = m_Pipe.begin(); it != m_Pipe.end(); ++it)
    {
        VpCmdPacket *pPacket = *it;
        PacketProperty prop  = {};
        prop.packetId        = pPacket->GetPacketId();
        prop.packet          = pPacket;
        prop.immediateSubmit = true;
        prop.stateProperty.statusReport = statusReport;

        MediaTask *pTask = pPacket->GetActiveTask();
        VP_PUBLIC_CHK_NULL_RETURN(pTask);

        VP_PUBLIC_CHK_STATUS_RETURN(SwitchContext(pPacket->GetPacketId(), scalability, mediaContext, bEnableVirtualEngine, numVebox));
        VP_PUBLIC_CHK_NULL_RETURN(scalability);

        VP_PUBLIC_CHK_STATUS_RETURN(pTask->AddPacket(&prop));
        if (prop.immediateSubmit)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(pTask->Submit(true, scalability, nullptr));
        }
    }
    return MOS_STATUS_SUCCESS;
}

VpCmdPacket *PacketPipe::CreatePacket(EngineType type)
{
    return m_PacketFactory.CreatePacket(type);
}


PacketPipeFactory::PacketPipeFactory(PacketFactory &pPacketFactory) : m_pPacketFactory(pPacketFactory)
{
}

PacketPipeFactory::~PacketPipeFactory()
{
    while (!m_Pool.empty())
    {
        PacketPipe *p = m_Pool.back();
        m_Pool.pop_back();
        MOS_Delete(p);
    }
}

PacketPipe *PacketPipeFactory::CreatePacketPipe()
{
    if (!m_Pool.empty())
    {
        PacketPipe *p = m_Pool.back();
        m_Pool.pop_back();
        p->Clean();
        return p;
    }
    return MOS_New(PacketPipe, m_pPacketFactory);
}

void PacketPipeFactory::ReturnPacketPipe(PacketPipe *&pPipe)
{
    if (nullptr == pPipe)
    {
        return;
    }
    pPipe->Clean();
    m_Pool.push_back(pPipe);
    pPipe = nullptr;
}