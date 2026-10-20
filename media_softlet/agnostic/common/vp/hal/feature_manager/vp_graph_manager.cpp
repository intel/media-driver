/* Copyright (c) 2025, Intel Corporation
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
//! \file     vp_graphset.cpp
//! \brief    The header file of the base class of graph set
//! \details  The graph set will include graph generation from binary.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#include "vp_graph_manager.h"
#include "levelzero_npu_interface.h"
#include "vp_utils.h"

using namespace vp;

VpGraphManager::VpGraphManager(VpGraphSet *graphset, PMOS_INTERFACE osInterface, VpAllocator *allocator) : 
    m_graphset(graphset), m_osInterface(osInterface), m_allocator(allocator)
{
    if (osInterface)
    {
        m_npuInterface = m_osInterface->npuInterface;
    }
}

MOS_STATUS VpGraphManager::GetGraphPackage(std::vector<AI_SINGLE_NPU_GRAPH_SETTING> &graphSettings, SwFilterPipe &executingPipe, GraphPackage *&graphPackage)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_graphset);
    VP_PUBLIC_CHK_NULL_RETURN(m_npuInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_NULL_RETURN(m_npuInterface->pfnCreateFence);
    VP_PUBLIC_CHK_NULL_RETURN(m_npuInterface->pfnCreateCmdList);

    VpGraphIDList graphIDs = {};
    for (AI_SINGLE_NPU_GRAPH_SETTING &graphSetting : graphSettings)
    {
        graphIDs.push_back(graphSetting.id);
    }
    auto handle = m_graphPackages.find(graphIDs);
    if (handle == m_graphPackages.end())
    {
        //Graph Package cannot be copied for it contains non-copiable member, so use piecewise_construct to create it directly in m_graphPackages
        handle = m_graphPackages.emplace(std::piecewise_construct, std::forward_as_tuple(graphIDs), std::forward_as_tuple()).first;
        VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_graphPackages);
        GraphPackage &package = handle->second;

        VP_PUBLIC_CHK_STATUS_RETURN(m_npuInterface->pfnCreateFence(m_npuInterface, package.GetNpuWLContext().Fence()));
        for (AI_SINGLE_NPU_GRAPH_SETTING &graphSetting : graphSettings)
        {
            AI_NPU_BUFFER_ALLOCATION_MAP allocationMap = {};
            VP_PUBLIC_CHK_NULL_RETURN(graphSetting.pfnGetIntermediateSurfaceSetting);
            VP_PUBLIC_CHK_STATUS_RETURN(graphSetting.pfnGetIntermediateSurfaceSetting(executingPipe, allocationMap));
            for (const auto &pair : allocationMap)
            {
                PVP_SURFACE surface   = nullptr;
                bool        allocated = false;
                VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->ReAllocateNpuBuffer(surface, pair.second.surfaceName.c_str(), pair.second.size, allocated));
                package.IntermiedateSurfaces().insert(std::make_pair(pair.first, surface));
            }
        }

        InitGraphListCmdPackage cmdPackage(m_npuInterface, &package);
        for (AI_SINGLE_NPU_GRAPH_SETTING &graphSetting : graphSettings)
        {
            ze_command_list_handle_t cmdList = nullptr;
            VP_PUBLIC_CHK_STATUS_RETURN(m_npuInterface->pfnCreateCmdList(m_npuInterface, cmdList));
            package.GetNpuWLContext().CmdList().push_back(cmdList);

            GraphHandle *graphHandle = nullptr;
            VP_PUBLIC_CHK_STATUS_RETURN(m_graphset->GetGraphObject(graphSetting.id, graphHandle));
            VP_PUBLIC_CHK_NULL_RETURN(graphHandle);

            std::vector<void *> graphArgs = {};
            for (auto& arg : graphSetting.graphArgs)
            {
                auto argHandle = package.IntermiedateSurfaces().find(arg);
                VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &package.IntermiedateSurfaces());
                VP_PUBLIC_CHK_NULL_RETURN(argHandle->second);
                VP_PUBLIC_CHK_NULL_RETURN(argHandle->second->zeNpuHostMem);
                graphArgs.push_back(argHandle->second->zeNpuHostMem);
            }

            cmdPackage.AddSingleLayerGraphArgs(graphArgs);
            cmdPackage.AddGraphHandle(graphHandle);
        }
        // InitGraphListCmdPackage Submit() is doing GraphPackage initilization, the real graph execution submit is in NpuCmdPackageSpecific
        VP_PUBLIC_CHK_STATUS_RETURN(m_osInterface->pfnSubmitPackage(m_osInterface, cmdPackage))
    }
    graphPackage = &handle->second;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpGraphManager::Destroy()
{
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_NULL_RETURN(m_npuInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_npuInterface->pfnDestroyFence);
    VP_PUBLIC_CHK_NULL_RETURN(m_npuInterface->pfnDestroyCmdList);

    for (auto& handle : m_graphPackages)
    {
        {
            NpuWLContext               &npuWLContext = handle.second.GetNpuWLContext();
            std::lock_guard<std::mutex> lock(npuWLContext.Mutex());
            npuWLContext.Initialized() = false;
            if (npuWLContext.Thread().joinable())
            {
                npuWLContext.Thread().join();
            }
            if (MOS_STATUS_SUCCESS != m_npuInterface->pfnDestroyFence(m_npuInterface, npuWLContext.Fence()))
            {
                VP_PUBLIC_ASSERTMESSAGE("Destroy Fence fail");
            }
            for (ze_command_list_handle_t &cmdList : npuWLContext.CmdList())
            {
                if (MOS_STATUS_SUCCESS != m_npuInterface->pfnDestroyCmdList(m_npuInterface, cmdList))
                {
                    VP_PUBLIC_ASSERTMESSAGE("Destroy Npu Cmd List fail");
                }
            }
        }
        
        for (const auto &surfaceHandle : handle.second.IntermiedateSurfaces())
        {
            PVP_SURFACE surface = surfaceHandle.second;
            if (MOS_STATUS_SUCCESS != m_allocator->DestroyNpuBuffer(surface))
            {
                VP_PUBLIC_ASSERTMESSAGE("Destroy Npu Buffer fail");
            }
        }
    }
    m_graphPackages.clear();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS InitGraphListCmdPackage::Submit()
{
    VP_PUBLIC_CHK_NULL_RETURN(m_graphPackage);
    VP_PUBLIC_CHK_NULL_RETURN(m_npuInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_npuInterface->pfnAppendGraph);

    m_graphPackage->GetNpuWLContext().Thread() = std::thread([this]() {
        MOS_STATUS status = MOS_STATUS_SUCCESS;
        do 
        {
            //waiting for all graph handles created successfully from IR/BLOB
            for (auto &graphHandle : m_graphHandles)
            {
                if (graphHandle == nullptr)
                {
                    status = MOS_STATUS_NULL_POINTER;
                    VP_PUBLIC_ASSERTMESSAGE("Graph Handle is nullptr");
                    break;
                }
                std::unique_lock<std::mutex> graphHandleLock(graphHandle->Mutex());
                if (graphHandle->Condition().wait_for(graphHandleLock, std::chrono::milliseconds(graphHandle->GetTimeoutMs()), [=] { return graphHandle->Initialized() || graphHandle->Failed(); }))
                {
                    if (graphHandle->Failed())
                    {
                        status = MOS_STATUS_UNINITIALIZED;
                        VP_PUBLIC_ASSERTMESSAGE("Graph Handle initialize failed, cannot continue to initialize Graph Package");
                        break;
                    }
                }
                else
                {
                    status = MOS_STATUS_UNINITIALIZED;
                    VP_PUBLIC_ASSERTMESSAGE("Timeout %d ms reached, Graph Handle still not finish Initialization", graphHandle->GetTimeoutMs());
                    break;
                }
            }
            if (status != MOS_STATUS_SUCCESS)
            {
                break;
            }
            // binding resources and graph handles into cmd list
            for (uint32_t i = 0; i < m_graphPackage->GetNpuWLContext().CmdList().size(); ++i)
            {
                if (m_multiLayerGraphArgs.size() <= i)
                {
                    status = MOS_STATUS_INVALID_PARAMETER;
                    VP_PUBLIC_ASSERTMESSAGE("Graph Arguments in multiLayerGraphArgs is not enough");
                    break;
                }
                if (m_graphHandles.size() <= i)
                {
                    status = MOS_STATUS_INVALID_PARAMETER;
                    VP_PUBLIC_ASSERTMESSAGE("Graph Handle in graphHandles is not enough");
                    break;
                }
                status = m_npuInterface->pfnAppendGraph(m_npuInterface, m_graphPackage->GetNpuWLContext().CmdList().at(i), m_graphHandles.at(i)->Graph(), m_multiLayerGraphArgs.at(i));
                if (status != MOS_STATUS_SUCCESS)
                {
                    VP_PUBLIC_ASSERTMESSAGE("Initialize Graph Package failed %d", status);
                    break;
                }
            }
        } while (false);

        {
            std::lock_guard<std::mutex> lock(m_graphPackage->GetNpuWLContext().Mutex());
            if (status == MOS_STATUS_SUCCESS)
            {
                m_graphPackage->GetNpuWLContext().Initialized() = true;
                m_graphPackage->GetNpuWLContext().Failed()      = false;
            }
            else
            {
                m_graphPackage->GetNpuWLContext().Failed()      = true;
                m_graphPackage->GetNpuWLContext().Initialized() = false;
            }
        }
        m_graphPackage->GetNpuWLContext().Condition().notify_all();
        m_releaseable = true;
    });
    
    return MOS_STATUS_SUCCESS;
}

std::unique_ptr<CmdPackage> InitGraphListCmdPackage::Clone() const
{
    return std::make_unique<InitGraphListCmdPackage>(*this);
}