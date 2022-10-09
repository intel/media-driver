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
 * @file mos_bufmgr_prelim.h
 *
 * The implementation for Prelim version functions.
 */

#ifndef MOS_BUFMGR_PRELIM_H
#define MOS_BUFMGR_PRELIM_H

#ifdef __I915_PMU_OTHER
#undef __I915_PMU_OTHER
#endif

#include "i915_drm.h"
#include "i915_drm_prelim.h"
#include "media_class_trace.h"
class BufmgrPrelim
{
public:
    static BufmgrPrelim *CreatePrelim(int fd);
    static void DestroyPrelim(BufmgrPrelim *prelim);
    static bool IsPrelimSupported();
    static int QueryDeviceBlob(int fd, MEDIA_SYSTEM_INFO* gfxInfo);
    static int SetContextParamParallel(struct mos_linux_context *ctx,
                     struct i915_engine_class_instance *ci,
                     uint32_t count);

    int Init(struct mos_bufmgr &bufmgr, bool hasLmem);
    uint8_t GetLmemRegionCount();

    void InitVmaHeap(mos_vma_heap *heap);
    void UninitVmaHeap(mos_vma_heap *heap);

    int QueryEngines(bool hasLmem,
            uint16_t engine_class,
            uint64_t caps,
            uint32_t *nengine,
            struct i915_engine_class_instance *ci);

    int CheckMemRegion(int memRegion, int mem_type);

    void SetMmapOffset(int memRegion, struct drm_i915_gem_mmap_offset &mmap_arg);

    int CreateGem(uint64_t inSize,
        uint32_t &outHandle,
        uint64_t &outSize,
        uint32_t &outRegion);

    int QueryEnginesCount(uint32_t *nengine);

    mos_memory_zone GetMemzoneForAddress(uint64_t address);

    int GetSystemMemRegionId() const { return PRELIM_I915_MEMORY_CLASS_SYSTEM; }

    void WaDisableSingleTimeline(bool hasLmem, uint32_t &flags);

    int GetMemoryInfo(bool hasLmem, char *info, uint32_t length);

private:
    BufmgrPrelim(int fd);
    virtual ~BufmgrPrelim();

    struct prelim_drm_i915_query_memory_regions *QueryMemRegions();
    uint32_t GetTileId();
    bool IsEngineSatisfyCaps(struct prelim_drm_i915_engine_info *engine, uint64_t caps);
    int QueryEnginesInternal(uint16_t engine_class,
        uint64_t caps,
        uint32_t *nengine,
        struct i915_engine_class_instance *ci,
        std::map<uint16_t, uint16_t> &logical_map);
    int QueryEnginesDistance(uint32_t nengine,
        struct prelim_drm_i915_query_distance_info *distances);
    bool IsQueryEnginesDistanceSupported(struct prelim_drm_i915_query_distance_info *distances);

    static int QueryItems(int fd, struct drm_i915_query_item *items, uint32_t n_items);

private:
    uint32_t m_tileId = 0;
    int m_fd = -1;
    static bool m_prelimEnabled;

    static constexpr uint32_t TYPE_DECIMAL = 10;

MEDIA_CLASS_DEFINE_END(BufmgrPrelim)
};

#endif //! MOS_BUFMGR_PRELIM_H
