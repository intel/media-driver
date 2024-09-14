/*
* Copyright (c) 2018-2021, Intel Corporation
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
#include "vp_feature_manager.h"
#include "vp_platform_interface.h"

using namespace vp;

PacketFactory::PacketFactory(VpPlatformInterface *vpPlatformInterface) : m_vpPlatformInterface(vpPlatformInterface)
{
}

PacketFactory::~PacketFactory()
{
    ClearPacketPool(m_VeboxPacketPool);
    ClearPacketPool(m_RenderPacketPool);
}

void PacketFactory::ClearPacketPool(std::vector<VpCmdPacket *> &pool)
{
    VP_FUNC_CALL();

    while (!pool.empty())
    {
        VpCmdPacket *p = pool.back();
        pool.pop_back();
        MOS_Delete(p);
    }
}

MOS_STATUS PacketFactory::Initialize(MediaTask *pTask, PVP_MHWINTERFACE pHwInterface, PVpAllocator pAllocator, VPMediaMemComp *pMmc, VP_PACKET_SHARED_CONTEXT *packetSharedContext, VpKernelSet* vpKernels, 
void *debugInterface)
{
    VP_FUNC_CALL();

    m_pTask = pTask;
    m_pHwInterface = pHwInterface;
    m_pAllocator = pAllocator;
    m_pMmc = pMmc;
    m_packetSharedContext = packetSharedContext;
    m_kernelSet = vpKernels;

#if (_DEBUG || _RELEASE_INTERNAL)
    m_debugInterface = static_cast<VpDebugInterface*>(debugInterface);
#endif

    return MOS_STATUS_SUCCESS;
}

VpCmdPacket *PacketFactory::CreatePacket(EngineType type)
{
    VP_FUNC_CALL();

    switch(type)
    {
    case EngineTypeVebox:
    case EngineTypeVeboxSfc:
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
    VP_FUNC_CALL();

    if (nullptr == pPacket)
    {
        return;
    }
    PacketType type = pPacket->GetPacketId();
    switch (type)
    {
    case VP_PIPELINE_PACKET_VEBOX:
        m_VeboxPacketPool.push_back(pPacket);
        break;
    case VP_PIPELINE_PACKET_RENDER:
    case VP_PIPELINE_PACKET_COMPUTE:
        m_RenderPacketPool.push_back(pPacket);
        break;
    default:
        break;
    }
    pPacket = nullptr;
}

VpCmdPacket *PacketFactory::CreateVeboxPacket()
{
    VP_FUNC_CALL();

    VpCmdPacket *p = m_vpPlatformInterface ? m_vpPlatformInterface->CreateVeboxPacket(m_pTask, m_pHwInterface, m_pAllocator, m_pMmc) : nullptr;
    if (p)
    {
        p->SetPacketSharedContext(m_packetSharedContext);
    }
    return p;
}

VpCmdPacket *PacketFactory::CreateRenderPacket()
{
    VP_FUNC_CALL();

    VpCmdPacket *p = m_vpPlatformInterface ? m_vpPlatformInterface->CreateRenderPacket(m_pTask, m_pHwInterface, m_pAllocator, m_pMmc, m_kernelSet) : nullptr;
    if (p)
    {
        MOS_STATUS status = MOS_STATUS_SUCCESS;
        
        status = p->Init();
        
        if (MOS_STATUS_SUCCESS != status)
        {
            VP_PUBLIC_ASSERTMESSAGE("Render CMD Packet Init Fail");
        }

        p->SetPacketSharedContext(m_packetSharedContext);
    }
    return p;
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
    VP_FUNC_CALL();

    m_outputPipeMode = VPHAL_OUTPUT_PIPE_MODE_INVALID;
    m_veboxFeatureInuse = false;
    for (std::vector<VpCmdPacket *>::iterator it = m_Pipe.begin(); it != m_Pipe.end(); ++it)
    {
        m_PacketFactory.ReturnPacket(*it);
    }
    m_Pipe.clear();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PacketPipe::AddPacket(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    VpCmdPacket *pPacket = m_PacketFactory.CreatePacket(hwFilter.GetEngineType());
    VP_PUBLIC_CHK_NULL_RETURN(pPacket);
    MOS_STATUS status = hwFilter.SetPacketParams(*pPacket);
    if (MOS_FAILED(status))
    {
        MOS_OS_ASSERTMESSAGE("SetPacketParams failed!");
        m_PacketFactory.ReturnPacket(pPacket);
        return status;
    }
    m_Pipe.push_back(pPacket);
    if (hwFilter.GetRenderTargetType() == RenderTargetTypeSurface)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(SetOutputPipeMode(hwFilter.GetEngineType()));
    }

    m_veboxFeatureInuse |= hwFilter.IsVeboxFeatureInuse();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PacketPipe::SetOutputPipeMode(EngineType engineType)
{
    VP_FUNC_CALL();

    switch (engineType)
    {
    case EngineTypeVebox:
        m_outputPipeMode = VPHAL_OUTPUT_PIPE_MODE_VEBOX;
        break;
    case EngineTypeVeboxSfc:
        m_outputPipeMode = VPHAL_OUTPUT_PIPE_MODE_SFC;
        break;
    case EngineTypeRender:
        m_outputPipeMode = VPHAL_OUTPUT_PIPE_MODE_COMP;
        break;
    default:
        m_outputPipeMode = VPHAL_OUTPUT_PIPE_MODE_INVALID;
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PacketPipe::SwitchContext(PacketType type, MediaScalability *&scalability, MediaContext *mediaContext, bool bEnableVirtualEngine, uint8_t numVebox)
{
    VP_FUNC_CALL();

    ScalabilityPars scalPars = {};
    switch (type)
    {
    case VP_PIPELINE_PACKET_VEBOX:
        {
            VP_PUBLIC_NORMALMESSAGE("Switch to Vebox Context");

            scalPars.enableVE = bEnableVirtualEngine;
            scalPars.numVebox = numVebox;

            VP_PUBLIC_CHK_STATUS_RETURN(mediaContext->SwitchContext(VeboxVppFunc, &scalPars, &scalability));
            VP_PUBLIC_CHK_NULL_RETURN(scalability);
            break;
        }
    case VP_PIPELINE_PACKET_RENDER:
        {
            VP_PUBLIC_NORMALMESSAGE("Switch to Render Context");
            VP_PUBLIC_CHK_STATUS_RETURN(mediaContext->SwitchContext(RenderGenericFunc, &scalPars, &scalability));
            VP_PUBLIC_CHK_NULL_RETURN(scalability);
            break;
        }
    case VP_PIPELINE_PACKET_COMPUTE:
        {
            VP_PUBLIC_NORMALMESSAGE("Switch to Compute Context");
            VP_PUBLIC_CHK_STATUS_RETURN(mediaContext->SwitchContext(ComputeVppFunc, &scalPars, &scalability));
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
    VP_FUNC_CALL();

    VP_PUBLIC_NORMALMESSAGE("PacketPipe %p in execute.", this);

    // PrePare Packet in case any packet resources shared
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    for (std::vector<VpCmdPacket*>::reverse_iterator it = m_Pipe.rbegin(); it != m_Pipe.rend(); ++it)
    {
        VpCmdPacket* packet = *it;
        VP_PUBLIC_CHK_STATUS_RETURN(packet->PrepareState());
    }

    for (std::vector<VpCmdPacket *>::iterator it = m_Pipe.begin(); it != m_Pipe.end(); ++it)
    {
        VpCmdPacket *pPacket = *it;
        PacketProperty prop  = {};
        prop.packetId        = pPacket->GetPacketId();
        prop.packet          = pPacket;
        prop.immediateSubmit = true;
        prop.stateProperty.statusReport = statusReport;

        bool isSkip = false;
        // Checking if extra processing is needed.
        isSkip      = pPacket->ExtraProcessing();
        if (isSkip)
        {
            VP_PUBLIC_NORMALMESSAGE("Skip this packet.");
            continue;
        }

        MediaTask *pTask = pPacket->GetActiveTask();
        VP_PUBLIC_CHK_NULL_RETURN(pTask);

        VP_PUBLIC_CHK_STATUS_RETURN(SwitchContext(pPacket->GetPacketId(), scalability, mediaContext, bEnableVirtualEngine, numVebox));
        VP_PUBLIC_CHK_NULL_RETURN(scalability);
        pPacket->SetMediaScalability(scalability);

        VP_PUBLIC_CHK_STATUS_RETURN(pTask->AddPacket(&prop));
        if (prop.immediateSubmit)
        {
            VP_PUBLIC_NORMALMESSAGE("Execute Packet %p.", pPacket);
            VP_PUBLIC_CHK_STATUS_RETURN(pTask->Submit(true, scalability, nullptr));
        }

#if USE_MEDIA_DEBUG_TOOL
        for (auto& handle : pPacket->GetSurfSetting().surfGroup)
        {
            if(handle.first && handle.second)
            {
                VP_SURFACE_DUMP(m_PacketFactory.m_debugInterface,
                    handle.second,
                    0,
                    handle.first,
                    VPHAL_DUMP_TYPE_POST_COMP,
                    VPHAL_SURF_DUMP_DDI_VP_BLT);
            }
        }
#endif
    }

    return eStatus;
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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    if (nullptr == pPipe)
    {
        return;
    }
    pPipe->Clean();
    if (std::find(m_Pool.begin(), m_Pool.end(), pPipe) == m_Pool.end())
    {
        m_Pool.push_back(pPipe);
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("packetPipe %p is existing in m_Pool!, m_Pool size is %d.", pPipe, (uint32_t)m_Pool.size());
    }
    pPipe = nullptr;
}