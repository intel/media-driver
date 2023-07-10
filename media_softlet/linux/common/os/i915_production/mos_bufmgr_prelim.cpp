/*
 * Copyright © 2022 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Xiaogang Li <xiaogang.li@intel.com>
 *
 */

/**
 * @file mos_bufmgr_prelim.cpp
 *
 * The implementation for Prelim version functions.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "errno.h"

#include "errno.h"
#ifndef ETIME
#define ETIME ETIMEDOUT
#endif
#include "libdrm_macros.h"
#include "libdrm_lists.h"

#include "xf86drm.h"
#include "xf86drm.h"
#include "xf86atomic.h"
#include "i915_drm.h"
#include "mos_bufmgr.h"
#include "mos_bufmgr_priv.h"
#include "mos_resource_defs.h"
#include "mos_util_debug.h"
#include "mos_vma.h"
#include "intel_hwconfig_types.h"
#include "mos_bufmgr_prelim.h"

#define memclear(s) memset(&s, 0, sizeof(s))

#define GOTO_FINI_IF_NOT_0(expr) \
    if (expr)                    \
    {                            \
        goto fini;               \
    }

#define GOTO_FINI_IF_MALLOC_FAIL(expr) \
{                                      \
    void *ptr = (expr);                \
    if (!ptr)                          \
    {                                  \
        assert(ptr);                   \
        ret = -ENOMEM;                 \
        goto fini;                     \
    }                                  \
}

bool BufmgrPrelim::m_prelimEnabled = false;

BufmgrPrelim *BufmgrPrelim::CreatePrelim(int fd)
{
    if (fd < 0) {
        return nullptr;
    }

    struct drm_i915_query_item item{};
    BufmgrPrelim *prelim = nullptr;

    memclear(item);
    item.query_id = PRELIM_DRM_I915_QUERY_MEMORY_REGIONS;

    struct drm_i915_query q;

    memclear(q);
    q.num_items = 1;
    q.items_ptr = (uintptr_t)&item;

    if (0 == drmIoctl(fd, DRM_IOCTL_I915_QUERY, &q) && item.length > 0) {
        prelim = new BufmgrPrelim(fd);
        if (nullptr != prelim) {
            m_prelimEnabled = true;
        }
    }

    return prelim;
}

bool BufmgrPrelim::IsPrelimSupported()
{
    return m_prelimEnabled;
}

void BufmgrPrelim::DestroyPrelim(BufmgrPrelim *prelim)
{
    if (nullptr != prelim) {
        delete prelim;
    }
}

BufmgrPrelim::BufmgrPrelim(int fd)
:m_fd(fd)
{
}

BufmgrPrelim::~BufmgrPrelim()
{
    m_fd = -1;
}

int BufmgrPrelim::Init(struct mos_bufmgr &bufmgr, bool hasLmem)
{
    uint32_t nengine = 8;
    struct i915_engine_class_instance uengines[nengine];
    memset(uengines, 0, sizeof(uengines));

    m_tileId = GetTileId();

    bufmgr.get_reserved = &m_tileId;
    int ret = QueryEngines(hasLmem, I915_ENGINE_CLASS_VIDEO, PRELIM_I915_VIDEO_CLASS_CAPABILITY_VDENC, &nengine, uengines);
    if(ret == 0)
    {
        if(nengine > 0)
        {
            bufmgr.has_full_vd = true;
        }
        else
        {
            bufmgr.has_full_vd = false;
        }
    }

    return ret;
}

void BufmgrPrelim::WaDisableSingleTimeline(bool hasLmem, uint32_t &flags)
{
    /*
        Since KMD has potential issue when submit cmd buffer to multi-tile GPU with context of single timeline;
        So add a WA here to remove this flag if not first tile in use on XeHP because context with single timeline can only work on first tile
    */
    if(hasLmem && m_tileId != 0)
    {
        flags &= ~(I915_CONTEXT_CREATE_FLAGS_SINGLE_TIMELINE);
    }
}

int BufmgrPrelim::QueryItems(int fd, struct drm_i915_query_item *items, uint32_t numItems)
{
    struct drm_i915_query q;

    memclear(q);
    q.num_items = numItems;
    q.items_ptr = (uintptr_t)items;

    return drmIoctl(fd, DRM_IOCTL_I915_QUERY, &q);
}

struct prelim_drm_i915_query_memory_regions *BufmgrPrelim::QueryMemRegions()
{
    int ret = 0;
    struct drm_i915_query_item item{};
    struct prelim_drm_i915_query_memory_regions *queryInfo = nullptr;

    memclear(item);
    item.query_id = PRELIM_DRM_I915_QUERY_MEMORY_REGIONS;
    ret = QueryItems(m_fd, &item, 1);
    if (ret != 0 || item.length <= 0)
        return nullptr;

    queryInfo = (struct prelim_drm_i915_query_memory_regions *)calloc(1, item.length);

    item.data_ptr = (uintptr_t)queryInfo;
    ret = QueryItems(m_fd, &item, 1);
    if (ret != 0) {
        free(queryInfo);
        return nullptr;
    }

    return queryInfo;
}

uint8_t BufmgrPrelim::GetLmemRegionCount()
{
    struct prelim_drm_i915_query_memory_regions *queryInfo;
    uint8_t numRegions = 0;
    uint8_t lmemRegions = 0;

    queryInfo = QueryMemRegions();

    if(nullptr != queryInfo)
    {
        numRegions = queryInfo->num_regions;

        for (int i = 0; i < numRegions; i++) {
            if (queryInfo->regions[i].region.memory_class == PRELIM_I915_MEMORY_CLASS_DEVICE)
            {
                lmemRegions += 1;
            }
        }
        free(queryInfo);
    }

    return lmemRegions;
}

uint32_t BufmgrPrelim::GetTileId()
{
    uint8_t lmemRegions = 0;
    uint32_t tileId = 0;
    lmemRegions = GetLmemRegionCount();

    if (lmemRegions > 0)
    {
        // generate an instance index by random
        struct timespec currentTime;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        srand(currentTime.tv_nsec);
        uint8_t instance = rand() % lmemRegions;
        tileId = instance;

        // get tile setting from environment variable
        char *tileInstance = getenv("INTEL_TILE_INSTANCE");
        if (tileInstance != nullptr)
        {
            errno = 0;
            long int instance = strtol(tileInstance, nullptr, TYPE_DECIMAL);
            /* Check for various possible errors */
            if ((errno == ERANGE && (instance == LONG_MAX || instance == LONG_MIN)) ||
                (errno != 0 && instance == 0))
            {
                fprintf(stderr, "Invalid INTEL_TILE_INSTANCE setting.(%d)\n", errno);
            }
            else
            {
                // valid instance value should be 0, 1, ..., lmemRegions-1
                if (instance >= 0 && instance < lmemRegions)
                {
                    tileId = instance;
                }
                else
                {
                    fprintf(stderr, "Invalid tile instance provided by user, will use default tile.\n");
                }
            }
        }
    }

    return tileId;
}

void BufmgrPrelim::InitVmaHeap(mos_vma_heap *heap)
{
   assert(nullptr != heap);
   mos_vma_heap_init(heap, MEMZONE_PRIME_START, MEMZONE_PRIME_SIZE);
}

void BufmgrPrelim::UninitVmaHeap(mos_vma_heap *heap)
{
   assert(nullptr != heap);
   mos_vma_heap_finish(heap);
}

enum mos_memory_zone
BufmgrPrelim::GetMemzoneForAddress(uint64_t address)
{
    if (address >= MEMZONE_PRIME_START) {
        return MEMZONE_PRIME;
    } else if (address >= MEMZONE_DEVICE_START) {
        return MEMZONE_DEVICE;
    } else {
        return MEMZONE_SYS;
    }
}

int BufmgrPrelim::CreateGem(uint64_t inSize,
    uint32_t &outHandle,
    uint64_t &outSize,
    uint32_t &outRegion)
{
    int ret = 0;

    struct prelim_drm_i915_gem_memory_class_instance mem_regions;
    uint32_t num_regions = 1;
    memclear(mem_regions);
    mem_regions.memory_class = PRELIM_I915_MEMORY_CLASS_DEVICE;
    mem_regions.memory_instance = m_tileId;

    struct prelim_drm_i915_gem_object_param region_param;
    memclear(region_param);
    region_param.size = num_regions;
    region_param.data = (uintptr_t)(&mem_regions);
    region_param.param = PRELIM_I915_OBJECT_PARAM | PRELIM_I915_PARAM_MEMORY_REGIONS;

    struct prelim_drm_i915_gem_create_ext_setparam setparam_region;
    memclear(setparam_region);
    setparam_region.base.name = PRELIM_I915_GEM_CREATE_EXT_SETPARAM;
    setparam_region.param = region_param;

    struct prelim_drm_i915_gem_create_ext create;
    memclear(create);
    create.size = inSize;
    create.extensions = (uintptr_t)(&setparam_region);

    ret = drmIoctl(m_fd,
            PRELIM_DRM_IOCTL_I915_GEM_CREATE_EXT,
            &create);
    outHandle = create.handle;
    outSize = create.size;
    outRegion = PRELIM_I915_MEMORY_CLASS_DEVICE;

    return ret;
}

int BufmgrPrelim::CheckMemRegion(int memRegion, int mem_type)
{
    if (memRegion ==  PRELIM_I915_MEMORY_CLASS_SYSTEM &&
        (mem_type == MOS_MEMPOOL_VIDEOMEMORY || mem_type == MOS_MEMPOOL_DEVICEMEMORY))
        return -EINVAL;

    if (memRegion ==  PRELIM_I915_MEMORY_CLASS_DEVICE &&
        (mem_type == MOS_MEMPOOL_SYSTEMMEMORY))
        return -EINVAL;

    return 0;
}

void BufmgrPrelim::SetMmapOffset(int memRegion, struct drm_i915_gem_mmap_offset &mmap_arg)
{
    if(memRegion == PRELIM_I915_MEMORY_CLASS_SYSTEM)
    {
        mmap_arg.flags = I915_MMAP_OFFSET_WB;
    }
    else
    {
        mmap_arg.flags = I915_MMAP_OFFSET_WC;
    }
}

int BufmgrPrelim::QueryEnginesCount(unsigned int *nengine)
{
    assert(m_fd >= 0);
    assert(nullptr != nengine);

    struct drm_i915_query query;
    struct drm_i915_query_item queryItem;
    struct prelim_drm_i915_query_engine_info *engines = nullptr;
    int ret, len;

    memclear(queryItem);
    queryItem.query_id = PRELIM_DRM_I915_QUERY_ENGINE_INFO;
    queryItem.length = 0;

    ret = QueryItems(m_fd, &queryItem, 1);
    if (ret || queryItem.length <= 0)
    {
        *nengine = 0;
        return ret;
    }

    len = queryItem.length;
    engines = (prelim_drm_i915_query_engine_info *)malloc(len);
    if (engines == nullptr)
    {
        *nengine = 0;
        return ret;
    }

    memset(engines, 0, len);
    memclear(queryItem);
    queryItem.query_id = PRELIM_DRM_I915_QUERY_ENGINE_INFO;
    queryItem.length = len;
    queryItem.data_ptr = (uintptr_t)engines;
    ret = QueryItems(m_fd, &queryItem, 1);

    *nengine = ret ? 0 : engines->num_engines;

    if (engines)
    {
        free(engines);
    }
    return ret;
}

bool BufmgrPrelim::IsEngineSatisfyCaps(
    struct prelim_drm_i915_engine_info *engine,
    uint64_t caps)
{
    if (!(engine->flags & PRELIM_I915_ENGINE_INFO_HAS_KNOWN_CAPABILITIES) ||
        !(engine->known_capabilities & PRELIM_I915_VIDEO_CLASS_CAPABILITY_VDENC))
    caps &= ~PRELIM_I915_VIDEO_CLASS_CAPABILITY_VDENC;

    return (caps & engine->capabilities) == caps;
}

int BufmgrPrelim::QueryEnginesInternal(uint16_t engine_class,
                      uint64_t caps,
                      unsigned int *nengine,
                      struct i915_engine_class_instance *ci,
                      std::map<uint16_t, uint16_t> &logicalMap)
{
    if(nengine == nullptr || ci == nullptr)
    {
        return -EINVAL;
    }

    assert(m_fd >= 0);

    struct drm_i915_query query;
    struct drm_i915_query_item item;
    struct prelim_drm_i915_query_engine_info *engines = nullptr;
    int ret, len;

    memclear(item);
    item.query_id = PRELIM_DRM_I915_QUERY_ENGINE_INFO;
    item.length = 0;

    GOTO_FINI_IF_NOT_0(ret = QueryItems(m_fd, &item, 1));
    len = item.length;
    if (len <= 0)
    {
        goto fini;
    }

    engines = (prelim_drm_i915_query_engine_info *)malloc(len);
    GOTO_FINI_IF_MALLOC_FAIL(engines);

    memset(engines,0,len);
    memclear(item);
    item.query_id = PRELIM_DRM_I915_QUERY_ENGINE_INFO;
    item.length = len;
    item.data_ptr = (uintptr_t)engines;
    GOTO_FINI_IF_NOT_0(ret = QueryItems(m_fd, &item, 1));

    int i, num;
    for (i = 0, num = 0; i < engines->num_engines; i++)
    {
        struct prelim_drm_i915_engine_info *engine = (struct prelim_drm_i915_engine_info *)&engines->engines[i];
        if (engine_class == engine->engine.engine_class && IsEngineSatisfyCaps(engine, caps))
        {
            ci->engine_class = engine_class;
            ci->engine_instance = engine->engine.engine_instance;

        /** Need to sort ci to logical order instead of uabi order
                |physica | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | assume 4 fused off
                -----------------------------------------
                |uabi    | 0 | 1 | 2 | 3 |   | 4 | 5 | 6 |
                -----------------------------------------
                |logical | 0 | 3 | 1 | 4 |   | 5 | 2 | 6 |
        */
            logicalMap[engine->engine.engine_instance] = engine->logical_instance;
            ci++;
            num++;
        }
        if (num > *nengine)
        {
            fprintf(stderr,"%s: Number of engine instances out of range, %d,%d\n",
                    __FUNCTION__, num, *nengine);
            goto fini;
        }
    }
    *nengine = num;

fini:
    if (engines)
        free(engines);
    return ret;
}

int BufmgrPrelim::QueryEngines(bool hasLmem,
        uint16_t engineClass,
        uint64_t caps,
        unsigned int *nengine,
        struct i915_engine_class_instance *ci)
{
    if((nengine == nullptr) || (ci == nullptr))
    {
        return -EINVAL;
    }

    uint32_t tileId  = m_tileId;
    struct i915_engine_class_instance   *ciTemp       = nullptr;
    struct prelim_drm_i915_query_distance_info *distances  = nullptr;
    std::map<uint16_t, uint16_t> logicalMap;
    logicalMap.clear();
    unsigned int engineNum;

    ciTemp = (struct i915_engine_class_instance *)malloc((*nengine) * sizeof(struct i915_engine_class_instance));
    if (!ciTemp)
    {
        assert(ciTemp);
        return -ENOMEM;
    }

    memset(ciTemp, 0, (*nengine) * sizeof(struct i915_engine_class_instance));

    int ret = QueryEnginesInternal(engineClass,
                      caps,
                      nengine,
                      ciTemp,
                      logicalMap);
    GOTO_FINI_IF_NOT_0(ret);

    distances = (struct prelim_drm_i915_query_distance_info *)malloc((*nengine) * sizeof(struct prelim_drm_i915_query_distance_info));
    GOTO_FINI_IF_MALLOC_FAIL(distances);

    memset(distances, 0, (*nengine) * sizeof(struct prelim_drm_i915_query_distance_info));
    for (int i = 0; i < *nengine; i++)
    {
        distances[i].engine = ciTemp[i];
        distances[i].region.memory_class = hasLmem ? PRELIM_I915_MEMORY_CLASS_DEVICE : PRELIM_I915_MEMORY_CLASS_SYSTEM;
        distances[i].region.memory_instance = tileId;
    }
    GOTO_FINI_IF_NOT_0(ret = QueryEnginesDistance(*nengine, distances));

    //select the engine with 0 distance
    int i, j;
    for(i = 0, j = 0; i < *nengine; i++)
    {
        if(0 == distances[i].distance)
        {
            ci[j] = distances[i].engine;
            j++;
        }
#if (_DEBUG || _RELEASE_INTERNAL)
        else if (0 > distances[i].distance)
        {
            MOS_OS_NORMALMESSAGE("Engine = {engine_class = %hu, engine_instance = %hu}; Distance (error code) == %d",
            distances[i].engine.engine_class, distances[i].engine.engine_instance, distances[i].distance);
        }
#endif
    }
    *nengine = j;

    engineNum = *nengine;
    if (engineClass == I915_ENGINE_CLASS_VIDEO && engineNum > 2)
    {
        std::map<uint16_t, uint16_t> reorderMap; //<logical id,  uabi id>

        for (i = 0; i < engineNum; i++)
        {
            uint16_t index = logicalMap[(ci + i)->engine_instance];
            reorderMap[index] = (ci + i)->engine_instance;
        }

        i = 0;
        for (auto iter = reorderMap.begin(); iter != reorderMap.end(); iter++)
        {
            (ci + i)->engine_instance = iter->second;
            i++;
        }
    }

fini:
    if(ciTemp)
    {
        free(ciTemp);
    }
    if(distances)
    {
        free(distances);
    }

    return ret;
}

int BufmgrPrelim::SetContextParamParallel(struct mos_linux_context *ctx,
                     struct i915_engine_class_instance *ci,
                     unsigned int count)
{
    if((ctx == nullptr) || (ci == nullptr) || (count <= 0))
    {
        return -EINVAL;
    }

    int      ret  = 0;
    uint32_t size = 0;
    int      n;
    struct i915_context_engines_parallel_submit* parallel_submit = nullptr;
    struct i915_context_param_engines* set_engines = nullptr;

    size = sizeof(struct i915_context_engines_parallel_submit) + count * sizeof(*ci);
    parallel_submit = (struct i915_context_engines_parallel_submit*)malloc(size);
    GOTO_FINI_IF_MALLOC_FAIL(parallel_submit);

    memset(parallel_submit, 0, size);
    parallel_submit->base.name = PRELIM_I915_CONTEXT_ENGINES_EXT_PARALLEL2_SUBMIT;
    parallel_submit->engine_index = 0;
    parallel_submit->width = count;
    parallel_submit->num_siblings = 1;
    for(int i = 0; i < count; i++)
    {
        parallel_submit->engines[i] = ci[i];
    }

    /* I915_DEFINE_CONTEXT_PARAM_ENGINES */
    size = sizeof(struct i915_context_param_engines) + sizeof(struct i915_engine_class_instance);
    set_engines = (struct i915_context_param_engines*) malloc(size);
    GOTO_FINI_IF_MALLOC_FAIL(set_engines);

    set_engines->extensions = (uintptr_t)(parallel_submit);
    set_engines->engines[0].engine_class = I915_ENGINE_CLASS_INVALID;
    set_engines->engines[0].engine_instance = I915_ENGINE_CLASS_INVALID_NONE;

    ret = mos_set_context_param(ctx,
                          size,
                          I915_CONTEXT_PARAM_ENGINES,
                          (uintptr_t)set_engines);
fini:
    if (set_engines)
        free(set_engines);
    if (parallel_submit)
        free(parallel_submit);
    return ret;
}

int BufmgrPrelim::QueryDeviceBlob(int fd, MEDIA_SYSTEM_INFO* gfxInfo)
{
    uint32_t *hwInfo;
    uint32_t  ulength= 0;
    int ret, i;
    struct drm_i915_query_item item;

    hwInfo = nullptr;

    memclear(item);
    item.length = 0;
    item.query_id = PRELIM_DRM_I915_QUERY_HWCONFIG_TABLE;
    ret = QueryItems(fd, &item, 1);
    if (ret != 0 || item.length <= 0)
    {
        return (ret != 0) ? ret : -1;
    }

    hwInfo = (uint32_t*) malloc(item.length);
    if (hwInfo != nullptr)
        memset(hwInfo, 0, item.length);
    else
        return -ENOMEM;
    item.data_ptr = (uintptr_t) hwInfo;
    ret = QueryItems(fd, &item, 1);
    if (ret != 0 || item.length <= 0)
    {
        return (ret != 0) ? ret : -1;
    }
    ulength = item.length / sizeof(uint32_t);
    i = 0;
    assert(gfxInfo);
    while (i < ulength) {
        /* Attribute ID starts with 1 */
        assert(hwInfo[i] > 0);

        switch (hwInfo[i])
        {
            case INTEL_HWCONFIG_MAX_SLICES_SUPPORTED:
            {
                assert(hwInfo[i+1] == 1);
                gfxInfo->SliceCount = hwInfo[i+2];
                gfxInfo->MaxSlicesSupported = hwInfo[i+2];
            }

            case INTEL_HWCONFIG_MAX_DUAL_SUBSLICES_SUPPORTED:
            {
                assert(hwInfo[i+1] == 1);
                gfxInfo->SubSliceCount = hwInfo[i+2];
                gfxInfo->MaxSubSlicesSupported = hwInfo[i+2];
            }

            case INTEL_HWCONFIG_MAX_NUM_EU_PER_DSS:
            {
                assert(hwInfo[i+1] == 1);
                gfxInfo->MaxEuPerSubSlice = hwInfo[i+2];
            }

            case INTEL_HWCONFIG_NUM_THREADS_PER_EU:
            {
                assert(hwInfo[i+1] == 1);
                gfxInfo->NumThreadsPerEu = hwInfo[i+2];
            }

            case INTEL_HWCONFIG_MAX_VECS:
            {
                assert(hwInfo[i+1] == 1);
                gfxInfo->MaxVECS = hwInfo[i+2];
            }

            default:
                break;
        }

        /* Advance to next key */
        i += hwInfo[i + 1];  // value size
        i += 2;// KL size
    }

    if (hwInfo != nullptr)
        free(hwInfo);

    return ret;
}

bool BufmgrPrelim::IsQueryEnginesDistanceSupported(struct prelim_drm_i915_query_distance_info *distances)
{
    assert(nullptr != distances);
    assert(m_fd >= 0);

    struct drm_i915_query_item item;
    memclear(item);
    item.query_id = PRELIM_DRM_I915_QUERY_DISTANCE_INFO;
    item.data_ptr = (uintptr_t)&distances[0];
    item.length = 0; //  When set to zero, this is filled with the size of the data

    if (QueryItems(m_fd, &item, 1))
    {
        return false;
    }
    return item.length > 0;
}

int BufmgrPrelim::QueryEnginesDistance(unsigned int nengine,
                      struct prelim_drm_i915_query_distance_info *distances)
{
    assert(nullptr != distances);
    assert(m_fd >= 0);

    int ret = 0;
    struct drm_i915_query_item *item = nullptr;

    item = (struct drm_i915_query_item *)malloc(nengine * sizeof(struct drm_i915_query_item));
    GOTO_FINI_IF_MALLOC_FAIL(item);

    GOTO_FINI_IF_NOT_0(!IsQueryEnginesDistanceSupported(distances));

    // Match memory, regions and distances to correct engines
    memset(item, 0, nengine * sizeof(struct drm_i915_query_item));
    for (int i = 0; i < nengine; i++)
    {
        item[i].query_id = PRELIM_DRM_I915_QUERY_DISTANCE_INFO;
        item[i].data_ptr = (uintptr_t)&distances[i];
        item[i].length = sizeof(struct prelim_drm_i915_query_distance_info);
    }

    GOTO_FINI_IF_NOT_0(ret = QueryItems(m_fd, item, nengine));
    for (int i = 0; i < nengine; i++)
    {
        if (item[i].length < 0) // The kernel sets this value to a negative value to signal an error
        {
            distances[i].distance = item[i].length; // Propagate the error up the call stack
        }
    }

fini:
    if (item)
        free(item);
    return ret;
}

int BufmgrPrelim::GetMemoryInfo(bool hasLmem, char *info, uint32_t length)
{
    if (hasLmem) {
        struct prelim_drm_i915_query_memory_regions *queryInfo = nullptr;
        queryInfo = QueryMemRegions();

        if (nullptr != queryInfo) {
            int offset = 0;
            for (int i = 0; i < queryInfo->num_regions; i++) {
                uint32_t memory_instance = queryInfo->regions[i].region.memory_instance;
                uint64_t probed_size = queryInfo->regions[i].probed_size;

                if (queryInfo->regions[i].region.memory_class == PRELIM_I915_MEMORY_CLASS_SYSTEM) {
                    offset += sprintf_s(info + offset, length - offset, "system(0x%x): 0x%lx ", memory_instance, probed_size);
                }
                else if (queryInfo->regions[i].region.memory_class == PRELIM_I915_MEMORY_CLASS_DEVICE) {
                    if (queryInfo->regions[i].region.memory_instance == m_tileId) {
                        offset += sprintf_s(info + offset, length - offset, "| *local%d: 0x%lx ", memory_instance, probed_size);
                    }
                    else {
                        offset += sprintf_s(info + offset, length - offset, " | local%d: 0x%lx ", memory_instance, probed_size);
                    }
                }
            }
            free(queryInfo);
            queryInfo = nullptr;
        }
    }

    return 0;
}
