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
#include "vp_graphset.h"
#include "levelzero_npu_interface.h"
#include "vp_utils.h"
#include "vp_platform_interface.h"

using namespace vp;

VpGraphSet::VpGraphSet(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator) :
    m_hwInterface(hwInterface),
    m_allocator(allocator)
{
    if (hwInterface)
    {
        m_osInterface = hwInterface->m_osInterface;
        if (hwInterface->m_vpPlatformInterface)
        {
            m_graphPool = &hwInterface->m_vpPlatformInterface->GetGraphPool();
        }
        
    }
}

MOS_STATUS VpGraphSet::Clean()
{
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface->npuInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface->npuInterface->pfnDestroyGraph);

    for (auto &handle : m_cachedGraphs)
    {
        std::lock_guard<std::mutex> lock(handle.second.Mutex());
        handle.second.Initialized() = false;
        if (handle.second.Thread().joinable())
        {
            handle.second.Thread().join();
        }
        ze_graph_handle_t &graph = handle.second.Graph();
        if (MOS_STATUS_SUCCESS != m_osInterface->npuInterface->pfnDestroyGraph(m_osInterface->npuInterface,graph))
        {
            VP_PUBLIC_ASSERTMESSAGE("Free graph fail");
        }
        handle.second.Initialized() = false;
        handle.second.Failed()      = false;
    }
    m_cachedGraphs.clear();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpGraphSet::GetGraphObject(
    VP_GRAPH_ID   graphId,
    GraphHandle *&graphHandle)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface->npuInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_graphPool);

    auto handle = m_cachedGraphs.find(graphId);
    if (handle == m_cachedGraphs.end())
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        VP_PUBLIC_CHK_STATUS_RETURN(LoadGraphForDebug());
#endif
        auto blobHandle = m_graphPool->find(graphId);
        VP_PUBLIC_CHK_NOT_FOUND_RETURN(blobHandle, m_graphPool);
        handle = m_cachedGraphs.emplace(std::piecewise_construct, std::forward_as_tuple(graphId), std::forward_as_tuple()).first;
        VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_cachedGraphs);
        InitGraphCmdPackage cmdPackage(blobHandle->second, m_osInterface->npuInterface, &handle->second);
        VP_PUBLIC_CHK_STATUS_RETURN(m_osInterface->pfnSubmitPackage(m_osInterface, cmdPackage));
    }
    graphHandle = &handle->second;
    return MOS_STATUS_SUCCESS;
}

std::unique_ptr<CmdPackage> InitGraphCmdPackage::Clone() const
{
    return std::make_unique<InitGraphCmdPackage>(*this);
}

MOS_STATUS InitGraphCmdPackage::Submit()
{
    VP_PUBLIC_CHK_NULL_RETURN(m_graphHandle);
    VP_PUBLIC_CHK_NULL_RETURN(m_npuInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_npuInterface->pfnBuildGraph);

    m_graphHandle->Thread() = std::thread([this]() {
        {
            MOS_STATUS                  status = m_npuInterface->pfnBuildGraph(m_npuInterface, m_graphBin.blob, m_graphBin.size, m_graphHandle->Graph());
            std::lock_guard<std::mutex> lock(m_graphHandle->Mutex());
            if (MOS_STATUS_SUCCESS == status)
            {
                m_graphHandle->Initialized() = true;
                m_graphHandle->Failed()      = false;
            }
            else
            {
                m_graphHandle->Failed()      = true;
                m_graphHandle->Initialized() = false;
            }
        }
        m_graphHandle->Condition().notify_all();
        m_releaseable = true;
    });
    return MOS_STATUS_SUCCESS;
}