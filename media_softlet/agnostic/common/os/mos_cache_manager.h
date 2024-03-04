/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     mos_cache_manager.h
//! \brief    cache management, to be used across media components
//! \details  cache management, to be used across media components
//!
#ifndef __MOS_CACHE_MANAGER_H__
#define __MOS_CACHE_MANAGER_H__

#include "mos_defs.h"
#include "mos_os_hw.h"

enum ENGINE_TYPE
{
    RENDER_ENGINE = 0,
    SFC_ENGINE,
    VEBOX_ENGINE,
    VDBOX_ENGINE
};

enum SURFACE_TYPE_BASE
{
    VEBOX_HEAP = 0,
    ISH_HEAP,
    GSH_HEAP,
    HEAP_MAX
};

enum CACHE_COMPONENTS
{
    CACHE_COMPONENT_COMMON = 0,
    CACHE_COMPONENT_VP,
    MAX_CACHE_COMPONENTS
};

struct MOS_CACHE_ELEMENT
{
    MOS_HW_RESOURCE_DEF mocsUsageType;
    MOS_HW_RESOURCE_DEF patIndex;
    MOS_CACHE_ELEMENT(MOS_HW_RESOURCE_DEF _mocsUsageType, MOS_HW_RESOURCE_DEF _patIndex) : 
        mocsUsageType(_mocsUsageType), patIndex(_patIndex)
    {
    }
};

struct MOS_CACHE_OBJECT
{
    uint64_t value;
    MOS_CACHE_OBJECT(MOS_COMPONENT _componentId, uint32_t _surfaceType, bool _isHeap, bool _isOutput, ENGINE_TYPE _engineType) :
        value( (static_cast<uint64_t>(_componentId) << 60) | (static_cast<uint64_t>(_surfaceType + !_isHeap * HEAP_MAX ) << 28) | \
            (static_cast<uint64_t>(_isOutput) << 27) | (static_cast<uint64_t>(_engineType) << 23))
    {
    }
} ;

bool RegisterCacheSettings(CACHE_COMPONENTS id, std::map<uint64_t, MOS_CACHE_ELEMENT> *cacheTablesPtr);

bool LoadCacheSettings(MOS_COMPONENT id, uint32_t feature, bool bOut, ENGINE_TYPE engineType, MOS_CACHE_ELEMENT &element, bool isHeapSurf);

#endif