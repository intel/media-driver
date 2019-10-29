/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file      cm_surface_state_manager.cpp 
//! \brief     Contains Class CmSurfaceState2Dor3DMgr  definitions 
//!

#include "cm_surface_state_manager.h"
#include "cm_surface_state.h"

CmSurfaceState2Dor3DMgr::CmSurfaceState2Dor3DMgr(CM_HAL_STATE *cmhal, MOS_RESOURCE *resource):
    m_cmhal (cmhal),
    m_resource (resource),
    m_defaultMoc(MOS_CM_RESOURCE_USAGE_SurfaceState << 8),
    m_defaultRenderTarget (true),
    m_defaultFrameType (CM_FRAME),
    m_defaultFormat(Format_Invalid),
    m_defaultWidth(0),
    m_defaultHeight(0),
    m_defaultDepth(0),
    m_rotationFlag(0),
    m_chromaSitting(0),
    m_dirty(false)
{
    m_resourceData = {0};
    MOS_ZeroMemory(m_defaultSurfState, sizeof(m_defaultSurfState));
    if (m_cmhal && m_cmhal->cmHalInterface)
    {
        m_defaultMoc = (m_cmhal->cmHalInterface->GetDefaultMOCS()) << 8;
    }
}

CmSurfaceState2Dor3DMgr::~CmSurfaceState2Dor3DMgr()
{
    clean();
}

void CmSurfaceState2Dor3DMgr::clean()
{
    for (int i = 0; i < 4; i++)
    {
        MOS_Delete(m_defaultSurfState[i]);
        for (auto ite = m_surfStateMap[i].begin(); ite != m_surfStateMap[i].end(); ite++)
        {
            MOS_Delete(ite->second);
        }
        m_surfStateMap[i].clear();
    }
}

CmSurfaceState* CmSurfaceState2Dor3DMgr::GetSurfaceState(int isAvs, int isSampler, CM_HAL_SURFACE2D_SURFACE_STATE_PARAM *param)
{
    int index = isAvs * 2 + isSampler;
    CM_ASSERT(index < 4);
    if (m_dirty)
    {
        clean();
        m_dirty = false;
    }
    if (param == nullptr) // get default
    {
        if (! m_defaultSurfState[index])
        {
            m_defaultSurfState[index] = MOS_New(CmSurfaceState2Dor3D, m_cmhal);
            if (m_defaultSurfState[index] == nullptr)
            {
                return nullptr;
            }
            m_defaultSurfState[index]->Initialize(m_resource, isAvs, isSampler);
            m_defaultSurfState[index]->SetFrameType(m_defaultFrameType);
            m_defaultSurfState[index]->SetMemoryObjectControl(m_defaultMoc);
            m_defaultSurfState[index]->SetRenderTarget(m_defaultRenderTarget);
            m_defaultSurfState[index]->SetFormat(m_defaultFormat);
            m_defaultSurfState[index]->SetUserDimension(m_defaultWidth, m_defaultHeight, m_defaultDepth);
            m_defaultSurfState[index]->SetRotationFlag(m_rotationFlag);
            m_defaultSurfState[index]->SetChromaSitting(m_chromaSitting);
            m_defaultSurfState[index]->GenerateSurfaceState();
        }
        return m_defaultSurfState[index];
    }
    else
    {
        uint32_t hashIdx = Hash(param);
        auto search = m_surfStateMap[index].find(hashIdx);
        if (search == m_surfStateMap[index].end())
        {
            CmSurfaceState2Dor3D *ss = MOS_New(CmSurfaceState2Dor3D, m_cmhal);
            if (ss == nullptr)
            {
                return nullptr;
            }
            ss->Initialize(m_resource, isAvs, isSampler);
            ss->SetFrameType(m_defaultFrameType);
            ss->SetMemoryObjectControl(m_defaultMoc);
            ss->SetRenderTarget(m_defaultRenderTarget);
            ss->SetFormat(m_defaultFormat);
            ss->SetUserDimension(m_defaultWidth, m_defaultHeight, m_defaultDepth);
            ss->SetRotationFlag(m_rotationFlag);
            ss->SetChromaSitting(m_chromaSitting);
            ss->GenerateSurfaceState(param);
            m_surfStateMap[index][hashIdx] = ss;
            return ss;
        }
        else
        {
            return search->second;
        }
    }
    return nullptr;
}

void CmSurfaceState2Dor3DMgr::SetRotationFlag(uint32_t rotation)
{
    if (m_rotationFlag != rotation)
    {
        m_rotationFlag = rotation;

        // clean the 3D sampler surface state
        int index = _3D_SAMPLER_SURFACE;
        MOS_Delete(m_defaultSurfState[index]);
        for (auto ite = m_surfStateMap[index].begin(); ite != m_surfStateMap[index].end(); ite++)
        {
            MOS_Delete(ite->second);
        }
        m_surfStateMap[index].clear();

        // clean the AVS sampler surface state

        index = _AVS_SAMPLER_SURFACE;
        MOS_Delete(m_defaultSurfState[index]);
        for (auto ite = m_surfStateMap[index].begin(); ite != m_surfStateMap[index].end(); ite++)
        {
            MOS_Delete(ite->second);
        }
        m_surfStateMap[index].clear();
    }
}

void CmSurfaceState2Dor3DMgr::SetChromaSitting(uint8_t chromaSitting)
{
    if (m_chromaSitting != chromaSitting)
    {
        m_chromaSitting = chromaSitting;
        // clean the AVS sampler surface state
        int index = _AVS_SAMPLER_SURFACE;
        MOS_Delete(m_defaultSurfState[index]);
        for (auto ite = m_surfStateMap[index].begin(); ite != m_surfStateMap[index].end(); ite++)
        {
            MOS_Delete(ite->second);
        }
        m_surfStateMap[index].clear();
    }
}


template <typename T>
inline void hash_combine(uint32_t &res, const T &field)
{
    res ^= field + 0x9e3779b9 + (res << 6) + (res >> 2);
}

uint32_t CmSurfaceState2Dor3DMgr::Hash(CM_HAL_SURFACE2D_SURFACE_STATE_PARAM *param)
{
    CM_HAL_SURFACE2D_SURFACE_STATE_PARAM temp;
    if (param == nullptr)
    {
        MOS_ZeroMemory(&temp, sizeof(temp));
        param = &temp;
    }
    uint32_t value = 0;
    hash_combine(value, param->depth);
    hash_combine(value, param->format);
    hash_combine(value, param->height);
    hash_combine(value, param->memoryObjectControl);
    hash_combine(value, param->pitch);
    hash_combine(value, param->surfaceXOffset);
    hash_combine(value, param->surfaceYOffset);
    hash_combine(value, param->width);

    return value;
}



CmSurfaceStateBufferMgr::CmSurfaceStateBufferMgr(CM_HAL_STATE *cmhal, MOS_RESOURCE *resource):
    m_defaultSurfState(nullptr),
    m_cmhal (cmhal),
    m_resource (resource),
    m_origSize(0),
    m_defaultMoc(MOS_CM_RESOURCE_USAGE_SurfaceState << 8),
    m_dirty(false)
{
    if (m_cmhal && m_cmhal->cmHalInterface)
    {
        m_defaultMoc = (m_cmhal->cmHalInterface->GetDefaultMOCS()) << 8;
    }
}

CmSurfaceStateBufferMgr::~CmSurfaceStateBufferMgr()
{
    clean();
}

void CmSurfaceStateBufferMgr::clean()
{
    MOS_Delete(m_defaultSurfState);
    for (auto ite = m_surfStateMap.begin(); ite != m_surfStateMap.end(); ite++)
    {
        MOS_Delete(ite->second);
    }
    m_surfStateMap.clear();
}
CmSurfaceState* CmSurfaceStateBufferMgr::GetSurfaceState(CM_HAL_BUFFER_SURFACE_STATE_ENTRY *param)
{
    if (m_dirty)
    {
        clean();
        m_dirty = false;
    }
    if (param == nullptr) // get default
    {
        if (! m_defaultSurfState)
        {
            m_defaultSurfState = MOS_New(CmSurfaceStateBuffer, m_cmhal);
            if (m_defaultSurfState == nullptr)
            {
                return nullptr;
            }
            m_defaultSurfState->Initialize(m_resource, m_origSize);
            m_defaultSurfState->SetMemoryObjectControl(m_defaultMoc);
            m_defaultSurfState->GenerateSurfaceState();
        }
        return m_defaultSurfState;
    }
    else
    {
        uint32_t hashIdx = Hash(param);
        auto search = m_surfStateMap.find(hashIdx);
        if (search == m_surfStateMap.end())
        {
            CmSurfaceStateBuffer *ss = MOS_New(CmSurfaceStateBuffer, m_cmhal);
            if (ss == nullptr)
            {
                return nullptr;
            }
            ss->Initialize(m_resource, m_origSize);
            ss->SetMemoryObjectControl(m_defaultMoc);
            ss->GenerateSurfaceState(param);
            m_surfStateMap[hashIdx] = ss;
            return ss;
        }
        else
        {
            return search->second;
        }
    }
    return nullptr;

}

uint32_t CmSurfaceStateBufferMgr::Hash(CM_HAL_BUFFER_SURFACE_STATE_ENTRY *param)
{
    CM_HAL_BUFFER_SURFACE_STATE_ENTRY temp;
    if (param == nullptr)
    {
        MOS_ZeroMemory(&temp, sizeof(temp));
        param = &temp;
    }
    uint32_t value = 0;
    hash_combine(value, param->surfaceStateSize);
    hash_combine(value, param->surfaceStateOffset);
    hash_combine(value, param->surfaceStateMOCS);

    return value;

}

