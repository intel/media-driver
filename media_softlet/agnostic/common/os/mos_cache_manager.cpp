/* Copyright (c) 2024, Intel Corporation
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
//! \file     mos_cache_manager.cpp
//! \brief    Defines the common classes for cache management
//!           this file is for the base interface which is shared by all features.
//!

#include "mos_cache_manager.h"
#include "media_class_trace.h"
#include "mos_util_debug.h"

class MosCacheManager
{
public:
    MosCacheManager();
    virtual ~MosCacheManager();

    static bool GetCacheSetting(MOS_COMPONENT id, uint32_t feature, bool bOut, ENGINE_TYPE engineType, MOS_CACHE_ELEMENT &element, bool isHeapSurf);

    static bool Register(CACHE_COMPONENTS id, std::map<uint64_t, MOS_CACHE_ELEMENT> *cacheTablesPtr)
    {
        if (GetInst().m_dataSet[id] == nullptr)
        {
            GetInst().m_dataSet[id] = cacheTablesPtr;
        }
        return true;
    }

private:
    static MosCacheManager &GetInst()
    {
        static MosCacheManager inst;
        return inst;
    }

    std::map<uint64_t, MOS_CACHE_ELEMENT> *m_dataSet[MAX_CACHE_COMPONENTS] = {};

    MEDIA_CLASS_DEFINE_END(MosCacheManager)
};

MosCacheManager::MosCacheManager()
{
}

MosCacheManager::~MosCacheManager()
{
}

bool MosCacheManager::GetCacheSetting(MOS_COMPONENT id, uint32_t feature, bool bOut, ENGINE_TYPE engineType, MOS_CACHE_ELEMENT &element, bool isHeapSurf)
{
    auto &inst = GetInst();
    uint64_t usage = MOS_CACHE_OBJECT(id, feature, isHeapSurf, bOut, engineType).value;
    for (int i = 0; i < MAX_CACHE_COMPONENTS; i++)
    {
        if (inst.m_dataSet[i])
        {
            std::map<uint64_t, MOS_CACHE_ELEMENT> *cacheMap = static_cast<std::map<uint64_t, MOS_CACHE_ELEMENT> *>(GetInst().m_dataSet[i]);
            auto                        it       = cacheMap->find(usage);
            if (it != cacheMap->end())
            {
                element = it->second;
                return true;
            }
        }
    }
    MOS_OS_ASSERTMESSAGE("Cache Element was not found for component %d, feature %d, bOut %d, engineType %d, isHeapSurf %d!", id, feature, bOut, engineType, isHeapSurf);
    return false;
}

bool RegisterCacheSettings(CACHE_COMPONENTS id, std::map<uint64_t, MOS_CACHE_ELEMENT> *cacheTablesPtr)
{
    return MosCacheManager::Register(id, cacheTablesPtr);
}

bool LoadCacheSettings(MOS_COMPONENT id, uint32_t feature, bool bOut, ENGINE_TYPE engineType, MOS_CACHE_ELEMENT &element, bool isHeapSurf)
{
    return MosCacheManager::GetCacheSetting(id, feature, bOut, engineType, element, isHeapSurf);
}