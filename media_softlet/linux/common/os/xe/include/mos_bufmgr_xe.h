/*
 * Copyright © 2023 Intel Corporation
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
 *      Xu, Zhengguo <zhengguo.xu@intel.com>
 */

#ifndef __MOS_BUFMGR_XE__
#define __MOS_BUFMGR_XE__

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <map>
#include <set>
#include <string>

#include "libdrm_macros.h"
#include "xf86atomic.h"
#include "drm.h"
#include "xe_drm.h"

#if defined(__cplusplus)
extern "C" {
#endif
extern drm_export int drmIoctl(int fd, unsigned long request, void *arg);

#if (_DEBUG || _RELEASE_INTERNAL)
/**
 * use env "export INTEL_XE_BUFMGR_DEBUG = #XE_DEBUG_* to enable different log level in mos bufmgr;
 * the log only available in debug or release internal version."
 */
static int64_t __xe_bufmgr_debug__;
#define XE_DEBUG_SYNCHRONIZATION   (1ull << 0)
#define __XE_TEST_DEBUG(flags) (__xe_bufmgr_debug__ & flags)
#endif

#define XE_DEFAULT_ALIGNMENT           0x1000
#define XE_DEFAULT_ALIGNMENT_64K       0x10000

#define EXEC_OBJECT_READ_XE  0x1
#define EXEC_OBJECT_WRITE_XE 0x2

#define INVALID_HANDLE    -1

#define memclear(s) memset(&s, 0, sizeof(s))

#define system_memory(__memory_regions)     (__memory_regions & 0x1)
#define vram_memory(__memory_regions, gt)     (__memory_regions & (0x2 << gt))
#define vram_if_possible(__memory_regions, gt)             \
    (vram_memory(__memory_regions, gt) ?                   \
     vram_memory(__memory_regions, gt) :                   \
     system_memory(__memory_regions))

#define MOS_XE_SUCCESS         MOS_STATUS_SUCCESS

#define MOS_XE_SAFE_FREE(ptr)                               \
    if(ptr != nullptr){                                     \
        free(ptr);                                          \
        ptr = nullptr;                                      \
    }

#define MOS_XE_GET_KEYS_FROM_MAP(map_datas, key_datas)                          \
    for (auto &it : map_datas)                                                  \
    {                                                                           \
        key_datas.insert(it.first);                                             \
    }

#define MOS_XE_GET_VALUES_FROM_MAP(map_datas, v_datas)                          \
    for (auto &it : map_datas)                                                  \
    {                                                                           \
        v_datas.push_back(it.second);                                           \
    }

enum ENV_KEYS {
    RESERVED_0 = 0,
    INTEL_TILE_INSTANCE = 1,
    INTEL_XE_BUFMGR_DEBUG = 2,
    RESERVED_3 = 3,
    INTEL_ENGINE_TIMESLICE=4,
    INTEL_ENV_COUNT,
};

static std::map<uint32_t, std::string> ENV_VARIABLE_TABLE = {
    {INTEL_TILE_INSTANCE, "INTEL_TILE_INSTANCE"},
    {INTEL_XE_BUFMGR_DEBUG, "INTEL_XE_BUFMGR_DEBUG"},
    {INTEL_ENGINE_TIMESLICE, "INTEL_ENGINE_TIMESLICE"}
};

/**
 * Initializes the GEM buffer manager, which uses the kernel to allocate, map,
 * and manage map buffer objections.
 *
 * \param fd File descriptor of the opened DRM device.
 */
struct mos_bufmgr *mos_bufmgr_gem_init_xe(int fd, int batch_size);

int mos_get_dev_id_xe(int fd, uint32_t *device_id);

#if defined(__cplusplus)
}
#endif
#endif  /* __MOS_BUFMGR_XE__ */
