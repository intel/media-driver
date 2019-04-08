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
#include <vector>

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

void DDI_CODEC_RENDER_TARGET_TABLE::Init(size_t max_num_entries)
{
    m_current_rt_surface = VA_INVALID_ID;
    m_current_recon_target = VA_INVALID_ID;
    m_free_index_pool.clear();
    m_va_to_rt_map.clear();

    for (RTTableIdx i = 0; i < max_num_entries; i++)
    {
        m_free_index_pool.push_back(i);
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
//!           required to register the new render target), VA_STATUS_ERROR_NOT_ENOUGH_BUFFER if it is
//!           impossible to register the new render target because none are marked as inactive.
//!
//!

VAStatus DDI_CODEC_RENDER_TARGET_TABLE::RegisterRTSurface(VASurfaceID id)
{
    if (id == VA_INVALID_ID)
    {
        DDI_ASSERTMESSAGE("Invalid VASurfaceID in RegisterRTSurfaces");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    if (IsRegistered(id))
    {
        m_va_to_rt_map[id].RTState = RT_STATE_ACTIVE_IN_CURFRAME;
        return VA_STATUS_SUCCESS;
    }

    if (!m_free_index_pool.empty())
    {
        RTTableIdx idx = m_free_index_pool.front();
        m_free_index_pool.pop_front();

        m_taken_index_history.push_back(idx);

        DDI_CODEC_RENDER_TARGET_INFO info;
        info.FrameIdx = idx;
        info.RTState = RT_STATE_ACTIVE_IN_CURFRAME;

        m_va_to_rt_map[id] = info;
        return VA_STATUS_SUCCESS;
    }
    else
    {
        using RTPair = std::pair<VASurfaceID, DDI_CODEC_RENDER_TARGET_INFO>;
        using RTPairIterator = std::map<VASurfaceID, DDI_CODEC_RENDER_TARGET_INFO>::iterator;
        using IdxListIterator = std::list<RTTableIdx>::iterator;


        // Evict the oldest inactive render target
        RTPairIterator rt_to_evict_iter = m_va_to_rt_map.end();
        for (auto it_list = m_taken_index_history.begin(); it_list != m_taken_index_history.end(); it_list++)
        {
            rt_to_evict_iter = std::find_if(m_va_to_rt_map.begin(), m_va_to_rt_map.end(),
                    [&] (const RTPair& pair)
                    { return (pair.second.FrameIdx == *it_list && pair.second.RTState == RT_STATE_INACTIVE);});
             if (rt_to_evict_iter != m_va_to_rt_map.end())
             {
                 DDI_VERBOSEMESSAGE("RegisterRTSurface: FrameIdx pool is empty, had to evict render target %d", rt_to_evict_iter->first);
                 RTTableIdx freed_idx = rt_to_evict_iter->second.FrameIdx;

                 m_taken_index_history.push_back(freed_idx);
                 m_taken_index_history.erase(it_list);

                 m_va_to_rt_map.erase(rt_to_evict_iter);
                 m_va_to_rt_map[id].FrameIdx = freed_idx;
                 m_va_to_rt_map[id].RTState = RT_STATE_ACTIVE_IN_CURFRAME;
                 return VA_STATUS_SUCCESS;
             }
        }

        DDI_ASSERTMESSAGE("RegisterRTSurface: FrameIdx pool is empty, and no render target can be evicted from the RT table!");
        return VA_STATUS_ERROR_NOT_ENOUGH_BUFFER;
    }
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
VAStatus DDI_CODEC_RENDER_TARGET_TABLE::UnRegisterRTSurface(VASurfaceID id)
{
    if (!IsRegistered(id))
    {
        DDI_VERBOSEMESSAGE("UnRegisterRTSurface: render target was not registered in the RTtbl!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_taken_index_history.erase(std::find(m_taken_index_history.begin(), m_taken_index_history.end(), m_va_to_rt_map[id].FrameIdx));
    m_free_index_pool.push_front(m_va_to_rt_map[id].FrameIdx);
    m_va_to_rt_map.erase(id);

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
bool DDI_CODEC_RENDER_TARGET_TABLE::IsRegistered(VASurfaceID id) const
{
    return (m_va_to_rt_map.find(id) != m_va_to_rt_map.end());
}


//!
//! \brief    Set Render Target State
//! \details  Sets the internal state variable for a registered render target
//!           to a specified value.
//!
//! \param    [in] id
//!           VASurfaceID of the registered render target
//! \param    [in] state
//!           RT_STATE to be set for the render target
//!
//! \return   VAStatus
//!           VA_STATUS_SUCCESS if successful, VA_STATUS_ERROR_INVALID_PARAMETER if
//!           no such surface is registered
//!
VAStatus DDI_CODEC_RENDER_TARGET_TABLE::SetRTState(VASurfaceID id, RT_STATE state)
{
    if (!IsRegistered(id))
    {
        DDI_VERBOSEMESSAGE("SetRTState: render target was not registered in the RTtbl!");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_va_to_rt_map[id].RTState = state;

    return VA_STATUS_SUCCESS;
}

//! \brief    Release DPB render targets
//! \details  Adjusts render target states in the render target table
//!           so that the render targets not belonging to current or previous frame's DPB
//!           may be evicted from the table if the table is completely filled up.
//!           In order to avoid RT table starvation, this should be called after each
//!           encode/decode/processing operation (usually at the end of EndPicture)
//!
void DDI_CODEC_RENDER_TARGET_TABLE::ReleaseDPBRenderTargets()
{
    for (auto& entry : m_va_to_rt_map)
    {
        if(entry.second.RTState == RT_STATE_ACTIVE_IN_LASTFRAME)
        {
            entry.second.RTState = RT_STATE_INACTIVE;
        }
        else if(entry.second.RTState == RT_STATE_ACTIVE_IN_CURFRAME)
        {
            entry.second.RTState = RT_STATE_ACTIVE_IN_LASTFRAME;
        }
    }
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

VAStatus DDI_CODEC_RENDER_TARGET_TABLE::SetCurrentRTSurface(VASurfaceID id)
{
    if (id != VA_INVALID_ID && !IsRegistered(id))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_current_rt_surface = id;

    return VA_STATUS_SUCCESS;
}

//!
//! \brief    Get Current Render Target Surface
//! \details  Returns a VASurfaceID of the "current" render target
//!
//! \return   VASurfaceID
//!
VASurfaceID DDI_CODEC_RENDER_TARGET_TABLE::GetCurrentRTSurface() const
{
    return m_current_rt_surface;
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

VAStatus DDI_CODEC_RENDER_TARGET_TABLE::SetCurrentReconTarget(VASurfaceID id)
{
    if (!IsRegistered(id))
    {
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_current_recon_target = id;

    return VA_STATUS_SUCCESS;
}

//!
//! \brief    Get Current Reconstructed Frame Render Target Surface
//! \details  Returns a VASurfaceID of the reconstructed frame render target
//!
//! \return   VASurfaceID
//!
VASurfaceID DDI_CODEC_RENDER_TARGET_TABLE::GetCurrentReconTarget() const
{
    return m_current_recon_target;
}

//!
//! \brief    Get Registered VA IDs
//! \details  Returns a vector of VASurfaceIDs currently registered in the render target table
//!
//! \return   VASurfaceID
//!
std::vector<VASurfaceID> DDI_CODEC_RENDER_TARGET_TABLE::GetRegisteredVAIDs() const
{
    std::vector<VASurfaceID> retval;
    retval.reserve(m_va_to_rt_map.size());
    for (const auto& pair : m_va_to_rt_map)
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
size_t DDI_CODEC_RENDER_TARGET_TABLE::GetNumRenderTargets() const
{
    return m_va_to_rt_map.size();
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
RTTableIdx DDI_CODEC_RENDER_TARGET_TABLE::GetFrameIdx(VASurfaceID id)
{
    if (id == VA_INVALID_ID || !IsRegistered(id))
    {
        return INVALID_RT_TABLE_INDEX;
    }

    return m_va_to_rt_map[id].FrameIdx;
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
VASurfaceID DDI_CODEC_RENDER_TARGET_TABLE::GetVAID(RTTableIdx FrameIdx) const
{
    using RTPair = std::pair<VASurfaceID, DDI_CODEC_RENDER_TARGET_INFO>;
    return std::find_if(m_va_to_rt_map.begin(), m_va_to_rt_map.end(), [=](const RTPair& pair)
                                    {return pair.second.FrameIdx == FrameIdx;})->first;
}



