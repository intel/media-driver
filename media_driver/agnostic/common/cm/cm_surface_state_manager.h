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
//! \file      cm_surface_state_manager.h
//! \brief     Contains Class CmSurfaceStateMgr  definitions 
//!
#pragma once

#include "cm_def.h"
#include "cm_hal.h"
#include "cm_device_rt.h"
#include <unordered_map>

class CmSurfaceState2Dor3D;
class CmSurfaceStateBuffer;
class CmSurfaceState;

#define UPDATE_IF_DIFFERENT(src, value) \
    if(src != value)                    \
    {                                   \
        src = value;                    \
        m_dirty = true;                 \
    }

class CmSurfaceState2Dor3DMgr
{
public:
    CmSurfaceState2Dor3DMgr(CM_HAL_STATE *cmhal, MOS_RESOURCE *resource);
    virtual ~CmSurfaceState2Dor3DMgr();
    
    CmSurfaceState* GetSurfaceState(int isAvs = 0, int isSampler = 0, CM_HAL_SURFACE2D_SURFACE_STATE_PARAM *param = nullptr);
    void SetRotationFlag(uint32_t rotation);
    void SetChromaSitting(uint8_t chromaSitting);
    inline void SetMemoryObjectControl(uint16_t moc) { UPDATE_IF_DIFFERENT(m_defaultMoc, moc); }
    inline void SetRenderTarget(bool flag) { UPDATE_IF_DIFFERENT(m_defaultRenderTarget, flag); }
    inline void SetFrameType(CM_FRAME_TYPE frameType) { UPDATE_IF_DIFFERENT(m_defaultFrameType, frameType); }
    inline void SetOrigFormat(MOS_FORMAT format) { UPDATE_IF_DIFFERENT(m_defaultFormat, format);}
    inline void SetOrigDimension(uint32_t width, uint32_t height, uint32_t depth)
    {
        UPDATE_IF_DIFFERENT(m_defaultWidth, width);
        UPDATE_IF_DIFFERENT(m_defaultHeight, height);
        UPDATE_IF_DIFFERENT(m_defaultDepth, depth);
    }
    inline MOS_RESOURCE* GetResource() 
    {
        if (m_resource == nullptr)
            return nullptr;
        m_resourceData = *m_resource;
        return &m_resourceData;
    }
protected:
    enum _SurfaceStateType
    {
        _RENDER_SURFACE = 0,
        _3D_SAMPLER_SURFACE = 1,
        _VME_SURFACE = 2,
        _AVS_SAMPLER_SURFACE = 3
    };
    uint32_t Hash(CM_HAL_SURFACE2D_SURFACE_STATE_PARAM *param);
    void clean();
    CmSurfaceState2Dor3D *m_defaultSurfState[4];
    std::map<uint32_t, CmSurfaceState2Dor3D *> m_surfStateMap[4];

    CM_HAL_STATE *m_cmhal;
    MOS_RESOURCE *m_resource;
    MOS_RESOURCE m_resourceData;

    // default configs
    uint16_t m_defaultMoc;
    bool m_defaultRenderTarget;
    CM_FRAME_TYPE m_defaultFrameType;
    MOS_FORMAT m_defaultFormat;
    uint32_t m_defaultWidth;
    uint32_t m_defaultHeight;
    uint32_t m_defaultDepth;

    // sampler settings
    uint32_t m_rotationFlag;
    uint8_t m_chromaSitting;

    // indicate whether default configs change
    bool m_dirty;
};

class CmSurfaceStateBufferMgr
{
public:
    CmSurfaceStateBufferMgr(CM_HAL_STATE *cmhal, MOS_RESOURCE *resource);
    ~CmSurfaceStateBufferMgr();
    CmSurfaceState* GetSurfaceState(CM_HAL_BUFFER_SURFACE_STATE_ENTRY *param = nullptr);
    inline void SetOrigSize(uint32_t size) {m_origSize = size; }
    inline void SetMemoryObjectControl(uint16_t moc) {UPDATE_IF_DIFFERENT(m_defaultMoc, moc); }
protected:
    uint32_t Hash(CM_HAL_BUFFER_SURFACE_STATE_ENTRY *param);
    void clean();
    CmSurfaceStateBuffer *m_defaultSurfState;
    std::map<uint32_t, CmSurfaceStateBuffer *> m_surfStateMap;

    CM_HAL_STATE *m_cmhal;
    MOS_RESOURCE *m_resource;

    // default configs
    uint32_t m_origSize;
    uint16_t m_defaultMoc;

    // indicate whether default configs change
    bool m_dirty;
};

class CmSurfaceState3DMgr: public CmSurfaceState2Dor3DMgr
{
public:
    CmSurfaceState3DMgr(CM_HAL_STATE *cmhal, MOS_RESOURCE *resource);
    ~CmSurfaceState3DMgr() {}
protected:
    MOS_RESOURCE m_resourceFor3d;
};

