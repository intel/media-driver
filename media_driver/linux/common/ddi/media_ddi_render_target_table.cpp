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
//! \file     media_ddi_render_target_table.cpp
//! \brief    Defines the class for render target management.
//!

#include "media_ddi_render_target_table.h"
#include "media_libva.h"
#include "media_libva_util.h"

#include <algorithm>
#include <set>

//!
//! \brief    Init
//! \details  Initialize the render target table. Should be called prior to other interactions
//!           with the render target table. Since the input parameter is codec-specific,
//!           Init should most likely be called in the codec-specific media context initialization
//!           stage.
//!
//! \param    [in] max_num_entries
//!           Maximum number of render targets to track in the render target table. Should realistically
//!           not exceed the maximum number of the uncompressed surface buffer of the driver.
//!
void MediaDdiRenderTargetTable::Init(size_t max_num_entries)
{
    m_currentRtSurface = VA_INVALID_ID;
    m_currentReconTarget = VA_INVALID_ID;
    m_surfaceIdToIndexMap.clear();
    m_usageOrder.clear();
    m_freeIndices.clear();
    for (RTTableIdx i = 0; i < max_num_entries; i += 1)
    {
        m_freeIndices.push_back(i);
    }
}

//!
//! \brief    Register Render Target Surface
//! \details  Register surface in render target table
//!
//! \param    [in] id
//!           VASurfaceID for the render target to be registered. If there is no more place
//!           inside the render target table, one inactive render target will be evicted in order
//!           to put the new render target in its place, and an error status returned.
//!
//! \return   VAStatus
//!           VA_STATUS_SUCCESS if successful (either id has been registered, or had
//!           already been registered, or a surface eviction was
//!           required to register the new render target), 
//!           VA_STATUS_ERROR_NOT_ENOUGH_BUFFER if it is
//!           impossible to register the new render target because none are marked as inactive.
//!
//!
VAStatus MediaDdiRenderTargetTable::RegisterRTSurface(VASurfaceID id)
{
    if (id == VA_INVALID_ID)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (!IsRegistered(id))
    {
        if (m_freeIndices.empty())
        {
            if (UnregisterRTSurface(m_usageOrder.back()) != VA_STATUS_SUCCESS)
            {
                return VA_STATUS_ERROR_INVALID_PARAMETER;
            }
        }

        RTTableIdx index = m_freeIndices.back();
        m_freeIndices.pop_back();
        m_surfaceIdToIndexMap[id] = index;
    }

    auto it = std::find(m_usageOrder.begin(), m_usageOrder.end(), id);
    if (it != m_usageOrder.end())
    {
        m_usageOrder.erase(it);
    }
    m_usageOrder.insert(m_usageOrder.begin(), id);

    return VA_STATUS_SUCCESS;
}

//!
//! \brief    Unregister Render Target Surface
//! \details  Unregister surface in render target table
//!
//! \param    [in] id
//!           VASurfaceID for the render target to be unregistered
//!
//! \return   VAStatus
//!           VA_STATUS_SUCCESS if successful, VA_STATUS_ERROR_INVALID_PARAMETER if
//!           no such surface is registered
//!
VAStatus MediaDdiRenderTargetTable::UnregisterRTSurface(VASurfaceID id)
{
    if (!IsRegistered(id))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_freeIndices.push_back(m_surfaceIdToIndexMap[id]);
    m_surfaceIdToIndexMap.erase(id);
    auto it = std::find(m_usageOrder.begin(), m_usageOrder.end(), id);
    if (it != m_usageOrder.end())
    {
        m_usageOrder.erase(it);
    }

    return VA_STATUS_SUCCESS;
}

//!
//! \brief    IsRegistered
//! \details  Determines whether or not a surface is registered in the render target table
//!
//! \param    [in] id
//!           VASurfaceID of the surface in question
//! \return   bool
//!           true if registered, false otherwise
//!
bool MediaDdiRenderTargetTable::IsRegistered(VASurfaceID id) const
{
    return (m_surfaceIdToIndexMap.find(id) != m_surfaceIdToIndexMap.end());
}


//!
//! \brief    Set Current Render Target Surface
//! \details  Sets a registered VASurfaceID as the one currently being processed ("current")
//!
//! \param    [in] id
//!           VASurfaceID of the registered render target to be set as "current"
//!
//! \return   VAStatus
//!           VA_STATUS_SUCCESS if successful, VA_STATUS_ERROR_INVALID_PARAMETER if
//!           no such surface is registered
VAStatus MediaDdiRenderTargetTable::SetCurrentRTSurface(VASurfaceID id)
{
    if (RegisterRTSurface(id) != VA_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_currentRtSurface = id;

    return VA_STATUS_SUCCESS;
}

//!
//! \brief    Get Current Render Target Surface
//! \details  Returns a VASurfaceID of the "current" render target
//!
//! \return   VASurfaceID
//!
VASurfaceID MediaDdiRenderTargetTable::GetCurrentRTSurface() const
{
    return m_currentRtSurface;
}

//!
//! \brief    Set Current Reconstructed Frame Render Target Surface
//! \details  Sets a registered VASurfaceID as the one that should contain the reconstructed frame
//!
//! \param    [in] id
//!           VASurfaceID of the registered render target to be set as containing the reconstructed frame
//!
//! \return   VAStatus
//!           VA_STATUS_SUCCESS if successful, VA_STATUS_ERROR_INVALID_PARAMETER if
//!           no such surface is registered
VAStatus MediaDdiRenderTargetTable::SetCurrentReconTarget(VASurfaceID id)
{
    if (RegisterRTSurface(id) != VA_STATUS_SUCCESS)
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_currentReconTarget = id;

    return VA_STATUS_SUCCESS;
}

//!
//! \brief    Get Current Reconstructed Frame Render Target Surface
//! \details  Returns a VASurfaceID of the reconstructed frame render target
//!
//! \return   VASurfaceID
//!
VASurfaceID MediaDdiRenderTargetTable::GetCurrentReconTarget() const
{
    return m_currentReconTarget;
}

//!
//! \brief    Get Registered VA IDs
//! \details  Returns a vector of VASurfaceIDs currently registered in the render target table
//!
//! \return   VASurfaceID
//!
std::vector<VASurfaceID> MediaDdiRenderTargetTable::GetRegisteredVAIDs() const
{
    std::vector<VASurfaceID> retval;
    retval.reserve(m_surfaceIdToIndexMap.size());
    for (const auto& pair : m_surfaceIdToIndexMap)
    {
        retval.push_back(pair.first);
    }
    return retval;
}

//!
//! \brief    Get Num Render Targets
//! \details  Returns the number of registered render targets
//!
//! \return   size_t
//!
size_t MediaDdiRenderTargetTable::GetNumRenderTargets() const
{
    return m_surfaceIdToIndexMap.size();
}

//!
//! \brief    Get Frame Index
//! \details  Returns a FrameIdx (driver internal surface management index)
//!           associated with the render target surface if it is registered in the table.
//!
//! \param    [in] id
//!           VASurfaceID of the registered render target
//!
//! \return   RTTableIdx
//!           FrameIdx of the render target with the VASurfaceID id if it is registered in the table,
//!           INVALID_RT_TABLE_INDEX if no such surface is registered
//!
RTTableIdx MediaDdiRenderTargetTable::GetFrameIdx(VASurfaceID id) const
{
    if (!IsRegistered(id))
    {
        return INVALID_RT_TABLE_INDEX;
    }

    return m_surfaceIdToIndexMap.at(id);
}

//!
//! \brief    Get VA ID
//! \details  Returns a VASurfaceID to which the FrameIdx (driver internal surface management index)
//!           is assigned.
//!
//! \param    [in] FrameIdx
//!           FrameIdx of the render target
//!
//! \return   VASurfaceID
//!           VASurfaceID of the render target with the FrameIdx if it is registered in the table,
//!           VA_INVALID_ID if no such surface is registered
//!
VASurfaceID MediaDdiRenderTargetTable::GetVAID(RTTableIdx FrameIdx) const
{
    using RTPair = std::pair<VASurfaceID, RTTableIdx>;
    VASurfaceID id = VA_INVALID_ID;

    auto it = std::find_if(m_surfaceIdToIndexMap.begin(), m_surfaceIdToIndexMap.end(),
        [=](const RTPair& pair){return pair.second == FrameIdx;});

    if (it != m_surfaceIdToIndexMap.end())
    {
        id = it->first;
    }

    return id;
}
