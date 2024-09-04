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

#ifdef HAVE_LIBGEN_H
#include <libgen.h>
#endif
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>
#include <pciaccess.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <termios.h>
#ifndef ETIME
#define ETIME ETIMEDOUT
#endif

#include <map>
#include <vector>
#include <queue>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <algorithm>

#ifdef HAVE_VALGRIND
#include <valgrind/valgrind.h>
#include <valgrind/memcheck.h>

#define VG(x) x
#else
#define VG(x) do {} while (0)
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "mos_bufmgr_api.h"
#include "mos_util_debug.h"
#include "intel_hwconfig_types.h"
#include "xf86drm.h"
#include "mos_vma.h"
#include "libdrm_lists.h"
#include "mos_bufmgr_xe.h"
#include "mos_synchronization_xe.h"
#include "mos_utilities.h"
#include "mos_bufmgr_util_debug.h"
#include "media_user_setting_value.h"
#include "linux_system_info.h"
#include "mos_oca_defs_specific.h"

//These two struct used by mos_bufmgr_priv.h
typedef struct MOS_OCA_EXEC_LIST_INFO mos_oca_exec_list_info;
//struct MEDIA_SYSTEM_INFO;

#include "mos_bufmgr_priv.h"

#define PAGE_SIZE_4K                   (1ull << 12)
#define MAX(a, b) ((a) > (b) ? (a) : (b))

//mos_xe_mem_class currently used as index of default_alignment
enum mos_xe_mem_class
{
    MOS_XE_MEM_CLASS_SYSMEM = 0,     //For DRM_XE_MEM_REGION_CLASS_SYSMEM
    MOS_XE_MEM_CLASS_VRAM,           //For DRM_XE_MEM_REGION_CLASS_VRAM
    MOS_XE_MEM_CLASS_MAX
};

struct mos_xe_context {
    struct mos_linux_context ctx;

    /**
     * Always keep the latest avaiable timeline index for
     * such execution's fence out point.
     */
    struct mos_xe_dep* timeline_dep;

    /**
     * The UMD's dummy exec_queue id for exec_queue ctx.
     */
    uint32_t dummy_exec_queue_id;

    /**
     * Indicate to the ctx width.
     */
    uint8_t ctx_width;
    /**
     * Indicate to num placements when creating exec_queue.
     */
    uint8_t num_placements;
    /**
     * Indicate to engine class used to create exec_queue.
     */
    uint16_t engine_class;
    /**
     * Indicate to engine capability of queried exec_queue.
     */
    uint64_t engine_caps;
    /**
     * Indicate to creation flags, current value should be always zero.
     */
    uint32_t flags;
    /**
     * Indicate whether it is protected ctx.
     */
    bool is_protected;

    /**
     * Indicate to exec_queue reset count on this context;
     * Note, this count depends on context restore, if uplayer tries to query
     * reset statue before context restore, this value may be incorrect.
     */
    uint32_t reset_count;
};

typedef struct mos_xe_device {
    /**
     * Note: we agree that hw_config[0] points to the number of hw config in total
     * And hw config data starts from hw_config[1]
     */
    uint32_t *hw_config = nullptr;
    struct drm_xe_query_config *config = nullptr;
    struct drm_xe_query_engines *engines = nullptr;
    struct drm_xe_query_mem_regions *mem_regions = nullptr;
    struct drm_xe_query_gt_list *gt_list = nullptr;

    /**
     * Note: we agree here that uc_versions[0] for guc version and uc_versions[1] for huc version
     */
    struct drm_xe_query_uc_fw_version uc_versions[UC_TYPE_MAX];
} mos_xe_device;

typedef struct mos_xe_bufmgr_gem {
    struct mos_bufmgr bufmgr;

    atomic_t ref_count;

    int fd;

    std::recursive_mutex m_lock;

    drmMMListHead managers;
    drmMMListHead named;

    mos_vma_heap vma_heap[MEMZONE_COUNT];

    bool object_capture_disabled; // Note: useless on xe and remove it in furture.

    #define MEM_PROFILER_BUFFER_SIZE 256
    char mem_profiler_buffer[MEM_PROFILER_BUFFER_SIZE];
    char* mem_profiler_path;
    int mem_profiler_fd;

    uint32_t gt_id;

    /**
     * This RW lock is used for avoid reading or writing the same sync obj in KMD.
     * Reading sync obj ioctl: exec and syncobj wait.
     * Writing sync obj ioctl: reset sync obj, destroy sync obj and create sync obj.
     */
    std::shared_timed_mutex sync_obj_rw_lock;

    /**
     * Save the pair of UMD dummy exec_queue id and ctx pointer.
     */
    std::map<uint32_t, struct mos_xe_context*> global_ctx_info;

    uint32_t vm_id;

    /**
     * Everything queried from kmd that indicates to hw infomation.
     */
    struct mos_xe_device xe_device;

    //Note: DON't put these fields in xe_device
    bool     has_vram;
    uint8_t  va_bits;
    /** bitmask of all memory regions */
    uint64_t mem_regions_mask;
    /** @default_alignment: safe alignment regardless region location */
    uint32_t default_alignment[MOS_XE_MEM_CLASS_MAX] = {PAGE_SIZE_4K, PAGE_SIZE_4K};
    //End of Note

    /**
     * Indicates whether gpu-gpu and cpu-gpu synchronization is disabled.
     * This is mainly for debug purpose, and synchronizarion should be always enabled by default.
     * It could be disabled by env INTEL_SYNCHRONIZATION_DISABLE.
     */
    bool is_disable_synchronization;

    /** indicate to exec_queue property of timeslice */
#define EXEC_QUEUE_TIMESLICE_DEFAULT    -1
#define EXEC_QUEUE_TIMESLICE_MAX        100000 //100ms
    int32_t exec_queue_timeslice;
} mos_xe_bufmgr_gem;

typedef struct mos_xe_exec_bo {
    /** indicate to real exec bo*/
    struct mos_linux_bo *bo;

    /**
     * Save read, write flags etc.
     * Two flags defined here: EXEC_OBJECT_READ_XE and EXEC_OBJECT_WRITE_XE.
     * Whether this bo needs exec sync depends on this flags.
     */
    uint32_t flags;
} mos_xe_exec_bo;

typedef struct mos_xe_bo_gem {
    /**
     * Maximun size for bo name
     */
#define MAX_NAME_SIZE 128

    struct mos_linux_bo bo;

    /**
     * Reference count
     */
    atomic_t ref_count;
    /**
     * Map count when map bo is called
     */
    atomic_t map_count;

    //Note7: unify gem_handle and bo.handle by deleting this one; Refine mos_linux_bo.handle to typt of uint32_t
    /**
     * Bo handle allocared from drm
     * Note: conbine with bo.handle to use same one.
     */
    uint32_t gem_handle;
    /**
     * Save bo name, this is for debug usage;
     * Suggest giving bo name when allocating bo.
     */
    char name[MAX_NAME_SIZE];

    /**
         *
         * List contains  prime fd'd objects
     */
    drmMMListHead name_list;

    /**
     * Mapped address for the buffer, saved across map/unmap cycles
     */
    void *mem_virtual;

    /**
     * Boolean of whether this buffer was allocated with userptr
     */
    bool is_userptr;

    /**
     * Memory region on created the surfaces for local/system memory;
     * This field only indicates to memory region type, it not memory region instance.
     */
    int mem_region;

    /**
     * We should always get the syncobj handle from the bo handle by bellow 4 steps in each time:
     * 1. get the prime_handle from bo.handle
     * 2. get syncfile fd from prime_fd
     * 3. get syncobj_handle from syncfile by
     * 4. close prime_fd and syncfile fd.
     *
     * If umd wants external process to sync between them, umd should always import its batch
     * syncobj handle into each external bo's dma sync buffer.
     *
     * Boolean of whether this buffer is imported from external
     */
    bool is_imported;
    /**
     * @cpu_caching: The CPU caching mode to select for this object. If
     * mmaping the object the mode selected here will also be used.
     *
     * Supported values:
     *
     * DRM_XE_GEM_CPU_CACHING_WB: Allocate the pages with write-back
     * caching.  On iGPU this can't be used for scanout surfaces. Currently
     * not allowed for objects placed in VRAM.
     *
     * DRM_XE_GEM_CPU_CACHING_WC: Allocate the pages as write-combined. This
     * is uncached. Scanout surfaces should likely use this. All objects
     * that can be placed in VRAM must use this.
     */
    uint16_t cpu_caching;

    /**
     * @pat_index: The platform defined @pat_index to use for this mapping.
     * The index basically maps to some predefined memory attributes,
     * including things like caching, coherency, compression etc.  The exact
     * meaning of the pat_index is platform specific. When the KMD sets up
     * the binding the index here is encoded into the ppGTT PTE.
     *
     * For coherency the @pat_index needs to be at least 1way coherent when
     * drm_xe_gem_create.cpu_caching is DRM_XE_GEM_CPU_CACHING_WB. The KMD
     * will extract the coherency mode from the @pat_index and reject if
     * there is a mismatch (see note below for pre-MTL platforms).
     *
     * Note: On pre-MTL platforms there is only a caching mode and no
     * explicit coherency mode, but on such hardware there is always a
     * shared-LLC (or is dgpu) so all GT memory accesses are coherent with
     * CPU caches even with the caching mode set as uncached.  It's only the
     * display engine that is incoherent (on dgpu it must be in VRAM which
     * is always mapped as WC on the CPU). However to keep the uapi somewhat
     * consistent with newer platforms the KMD groups the different cache
     * levels into the following coherency buckets on all pre-MTL platforms:
     *
     *  ppGTT UC -> COH_NONE
     *  ppGTT WC -> COH_NONE
     *  ppGTT WT -> COH_NONE
     *  ppGTT WB -> COH_AT_LEAST_1WAY
     *
     * In practice UC/WC/WT should only ever used for scanout surfaces on
     * such platforms (or perhaps in general for dma-buf if shared with
     * another device) since it is only the display engine that is actually
     * incoherent.  Everything else should typically use WB given that we
     * have a shared-LLC.  On MTL+ this completely changes and the HW
     * defines the coherency mode as part of the @pat_index, where
     * incoherent GT access is possible.
     *
     * Note: For userptr and externally imported dma-buf the kernel expects
     * either 1WAY or 2WAY for the @pat_index.
     */
    uint16_t pat_index;

    /**
     * Boolean of whether this buffer is exported to external
     */
    bool is_exported;

    /**
     * For cmd bo, it has an exec bo list which saves all exec bo in it.
     * Uplayer caller should alway update this list before exec submission and clear the list after exec submission.
     */
    std::map<uintptr_t, struct mos_xe_exec_bo> exec_list;

#define INVALID_EXEC_QUEUE_ID    -1
    /**
     * Save last dummy write exec_queue id.
     * Init this field as INVALID_EXEC_QUEUE_ID at begining.
     */
    uint32_t last_exec_write_exec_queue;

    /**
     * Save last dummy read exec_queue id.
     * Init this field as INVALID_EXEC_QUEUE_ID at begining.
     */
    uint32_t last_exec_read_exec_queue;

    /**
     * Read dependents, pair of dummy EXEC_QUEUE_ID and mos_xe_bo_dep
     * This map saves read deps of this bo on all exec exec_queue;
     * Exec will check opration flags to get the dep from the map to add into exec sync array and updated the map after exec.
     * Refer to exec call to get more details.
     */
    std::map<uint32_t, struct mos_xe_bo_dep> read_deps;

    /**
     * Write dependents, pair of dummy EXEC_QUEUE_ID and mos_xe_bo_dep
     * This map saves write deps of this bo on all exec exec_queue;
     * Exec will check opration flags to get the dep from the map to add into exec sync array and updated the map after exec.
     * Refer to exec call to get more details.
     */
    std::map<uint32_t, struct mos_xe_bo_dep> write_deps;

} mos_xe_bo_gem;

struct mos_xe_external_bo_info {
    /**
     * syncobj handle created by umd to import external bo syncfile
     */
    int syncobj_handle;
    /**
     * prime fd export from external bo handle
     */
    int prime_fd;
};

#define MOS_UNIMPLEMENT(param)    (void)(param)

static pthread_mutex_t bufmgr_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static drmMMListHead bufmgr_list = { &bufmgr_list, &bufmgr_list };

static void mos_bo_free_xe(struct mos_linux_bo *bo);
static int mos_query_engines_count_xe(struct mos_bufmgr *bufmgr, unsigned int *nengine);
int mos_query_engines_xe(struct mos_bufmgr *bufmgr,
                      __u16 engine_class,
                      __u64 caps,
                      unsigned int *nengine,
                      void *engine_map);
static void mos_gem_bo_wait_rendering_xe(struct mos_linux_bo *bo);

static struct mos_xe_bufmgr_gem *
mos_bufmgr_gem_find(int fd)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem;

    DRMLISTFOREACHENTRY(bufmgr_gem, &bufmgr_list, managers) {
        if (bufmgr_gem->fd == fd) {
            atomic_inc(&bufmgr_gem->ref_count);
            return bufmgr_gem;
        }
    }

    return nullptr;
}

#define MOS_DRM_CHK_XE_DEV(xe_dev, info, query_func, retval)                 \
    MOS_DRM_CHK_NULL_RETURN_VALUE(xe_dev, retval);                           \
    if (xe_dev->info == nullptr)                                             \
    {                                                                        \
        xe_dev->info = query_func(fd);                                       \
        MOS_DRM_CHK_NULL_RETURN_VALUE(xe_dev->info, retval);                 \
    }

static struct drm_xe_query_gt_list *
__mos_query_gt_list_xe(int fd)
{
    int ret = 0;
    struct drm_xe_query_gt_list *gt_list;
    struct drm_xe_device_query query;
    memclear(query);
    query.query = DRM_XE_DEVICE_QUERY_GT_LIST;

    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY,
                &query);
    if (ret || !query.size)
    {
        return nullptr;
    }

    gt_list = (drm_xe_query_gt_list *)calloc(1, query.size);
    MOS_DRM_CHK_NULL_RETURN_VALUE(gt_list, nullptr);

    query.data = (uintptr_t)(gt_list);
    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY,
                &query);
    if (ret || !query.size || 0 == gt_list->num_gt)
    {
        MOS_XE_SAFE_FREE(gt_list);
        return nullptr;
    }

    return gt_list;
}

static uint32_t __mos_query_mem_regions_instance_mask_xe(struct mos_bufmgr *bufmgr)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, 0)
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;
    int fd = bufmgr_gem->fd;
    uint64_t __memory_regions = 0;

    MOS_DRM_CHK_XE_DEV(dev, gt_list, __mos_query_gt_list_xe, 0)

    struct drm_xe_query_gt_list *gt_list = dev->gt_list;
    for (int i = 0; i < gt_list->num_gt; i++) {
        /**
         * Note: __memory_regions is the mem region instance mask on all tiles and gts
         */
        __memory_regions |= gt_list->gt_list[i].near_mem_regions |
            gt_list->gt_list[i].far_mem_regions;
    }

    bufmgr_gem->mem_regions_mask = __memory_regions;

    return __memory_regions;
}

static struct drm_xe_query_mem_regions *
__mos_query_mem_regions_xe(int fd)
{
    int ret = 0;
    struct drm_xe_query_mem_regions *mem_regions;
    struct drm_xe_device_query query;
    memclear(query);
    query.query = DRM_XE_DEVICE_QUERY_MEM_REGIONS;

    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY,
                &query);
    if (ret || !query.size)
    {
        return nullptr;
    }

    mem_regions = (drm_xe_query_mem_regions *)calloc(1, query.size);
    MOS_DRM_CHK_NULL_RETURN_VALUE(mem_regions, nullptr);

    query.data = (uintptr_t)(mem_regions);
    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query);
    if (ret || !query.size || 0 == mem_regions->num_mem_regions)
    {
        MOS_XE_SAFE_FREE(mem_regions);
        return nullptr;
    }

    return mem_regions;
}

uint8_t __mos_query_vram_region_count_xe(struct mos_xe_device *dev, int fd)
{
    uint8_t vram_regions = 0;

    MOS_DRM_CHK_XE_DEV(dev, mem_regions, __mos_query_mem_regions_xe, 0)

    struct drm_xe_query_mem_regions *mem_regions = dev->mem_regions;
    for (int i =0; i < mem_regions->num_mem_regions; i++)
    {
        if (mem_regions->mem_regions[i].mem_class == DRM_XE_MEM_REGION_CLASS_VRAM)
        {
            vram_regions++;
        }
    }

    return vram_regions;
}

int mos_force_gt_reset_xe(int fd, int gt_id)
{
    char reset_string[128];

    sprintf(reset_string, "cat /sys/kernel/debug/dri/0/gt%d/force_reset", gt_id);
    return system(reset_string);
}

static struct drm_xe_query_config *
__mos_query_config_xe(int fd)
{
    struct drm_xe_query_config *config;
    struct drm_xe_device_query query;
    int ret = 0;

    memclear(query);
    query.query = DRM_XE_DEVICE_QUERY_CONFIG;
    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, (void *)&query);
    if (ret || !query.size)
    {
        return nullptr;
    }

    config = (drm_xe_query_config *) malloc(query.size);
    if (config != nullptr)
    {
        memset(config, 0, query.size);
    }
    else
    {
        MOS_DRM_ASSERTMESSAGE("malloc config failed");
        return nullptr;
    }

    query.data = (uintptr_t)config;
    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, (void *)&query);
    if (ret || !query.size || 0 == config->num_params)
    {
        MOS_XE_SAFE_FREE(config);
        return nullptr;
    }

    return config;
}

static int
__mos_get_default_alignment_xe(struct mos_bufmgr *bufmgr)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, -EINVAL)
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;
    int fd = bufmgr_gem->fd;
    MOS_DRM_CHK_XE_DEV(dev, mem_regions, __mos_query_mem_regions_xe, -ENODEV)
    struct drm_xe_query_mem_regions *mem_regions = dev->mem_regions;
    uint16_t mem_class;

    for (int i = 0; i < mem_regions->num_mem_regions; i++)
    {
        if (DRM_XE_MEM_REGION_CLASS_SYSMEM == mem_regions->mem_regions[i].mem_class)
        {
            mem_class = MOS_XE_MEM_CLASS_SYSMEM;
        }
        else if (DRM_XE_MEM_REGION_CLASS_VRAM == mem_regions->mem_regions[i].mem_class)
        {
            mem_class = MOS_XE_MEM_CLASS_VRAM;
        }
        else
        {
            MOS_DRM_ASSERTMESSAGE("Unsupported mem class");
            return -EINVAL;
        }

        if (bufmgr_gem->default_alignment[mem_class] < mem_regions->mem_regions[i].min_page_size)
        {
            bufmgr_gem->default_alignment[mem_class] = mem_regions->mem_regions[i].min_page_size;
        }
    }

    return 0;
}

/**
 * Note: Need to add this func to bufmgr api later
 */
static int
mos_query_uc_version_xe(struct mos_bufmgr *bufmgr, struct mos_drm_uc_version *version)
{
    int ret = 0;
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    int fd = bufmgr_gem->fd;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;

    if (bufmgr && version && version->uc_type < UC_TYPE_MAX)
    {
        /**
         * Note: query uc version from kmd if no historic data in bufmgr, otherwise using historic data.
         */
        if (dev->uc_versions[version->uc_type].uc_type != version->uc_type)
        {
            struct drm_xe_device_query query;
            memclear(query);
            query.size = sizeof(struct drm_xe_query_uc_fw_version);
            query.query = DRM_XE_DEVICE_QUERY_UC_FW_VERSION;
            memclear(dev->uc_versions[version->uc_type]);
            dev->uc_versions[version->uc_type].uc_type = version->uc_type;
            query.data = (uintptr_t)&dev->uc_versions[version->uc_type];

            ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_DEVICE_QUERY,
                        &query);
            if (ret)
            {
                memclear(dev->uc_versions[version->uc_type]);
                dev->uc_versions[version->uc_type].uc_type = UC_TYPE_INVALID;
                MOS_DRM_ASSERTMESSAGE("Failed to query UC version, uc type: %d, errno: %d", version->uc_type, ret);
                return ret;
            }
        }

        version->major_version = dev->uc_versions[version->uc_type].major_ver;
        version->minor_version = dev->uc_versions[version->uc_type].minor_ver;
    }

    return ret;
}

bool __mos_has_vram_xe(struct mos_bufmgr *bufmgr)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, 0)
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;
    int fd = bufmgr_gem->fd;
    MOS_DRM_CHK_XE_DEV(dev, config, __mos_query_config_xe, 0)
    struct drm_xe_query_config *config = dev->config;
    bool has_vram = ((config->info[DRM_XE_QUERY_CONFIG_FLAGS] & DRM_XE_QUERY_CONFIG_FLAG_HAS_VRAM) > 0);
    bufmgr_gem->has_vram = has_vram;
    return has_vram;
}

uint8_t __mos_query_va_bits_xe(struct mos_bufmgr *bufmgr)
{
    uint8_t va_bits = 48;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, va_bits)
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;
    int fd = bufmgr_gem->fd;
    bufmgr_gem->va_bits = va_bits;
    MOS_DRM_CHK_XE_DEV(dev, config, __mos_query_config_xe, va_bits)
    struct drm_xe_query_config *config = dev->config;
    va_bits = config->info[DRM_XE_QUERY_CONFIG_VA_BITS] & 0xff;
    bufmgr_gem->va_bits = va_bits;
    return va_bits;
}

static uint64_t
mos_get_platform_information_xe(struct mos_bufmgr *bufmgr)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, 0)
    return bufmgr->platform_information;
}

static void
mos_set_platform_information_xe(struct mos_bufmgr *bufmgr, uint64_t p)
{
    if (bufmgr)
        bufmgr->platform_information |= p;
}

static enum mos_memory_zone
__mos_bo_memzone_for_address_xe(uint64_t address)
{
    if (address >= MEMZONE_PRIME_START)
        return MEMZONE_PRIME;
    else if (address >= MEMZONE_DEVICE_START)
        return MEMZONE_DEVICE;
    else
        return MEMZONE_SYS;
}

static void
__mos_bo_vma_free_xe(struct mos_bufmgr *bufmgr,
         uint64_t address,
         uint64_t size)
{
    CHK_CONDITION(nullptr == bufmgr, "nullptr bufmgr.\n", );
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bufmgr;

    CHK_CONDITION(0ull == address, "invalid address.\n", );
    enum mos_memory_zone memzone = __mos_bo_memzone_for_address_xe(address);
    mos_vma_heap_free(&bufmgr_gem->vma_heap[memzone], address, size);
}

static void
__mos_bo_mark_mmaps_incoherent_xe(struct mos_linux_bo *bo)
{
#if HAVE_VALGRIND
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *) bo;

    if (bo_gem->mem_virtual)
        VALGRIND_MAKE_MEM_NOACCESS(bo_gem->mem_virtual, bo->size);
#endif
}

static inline void
mos_bo_reference_xe(struct mos_linux_bo *bo)
{
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *) bo;

    atomic_inc(&bo_gem->ref_count);
}

drm_export void mos_bo_unreference_xe(struct mos_linux_bo *bo)
{
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *) bo;

    if (atomic_read(&bo_gem->ref_count) <= 0)
        return;

    if (atomic_dec_and_test(&bo_gem->ref_count))
    {
        /* release memory associated with this object */
        /* Clear any left-over mappings */
        if (atomic_read(&bo_gem->map_count) > 0)
        {
            atomic_set(&bo_gem->map_count, 0);
            __mos_bo_mark_mmaps_incoherent_xe(bo);
        }

        DRMLISTDEL(&bo_gem->name_list);

        mos_bo_free_xe(bo);
    }
}

static uint32_t
__mos_vm_create_xe(struct mos_bufmgr *bufmgr)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct drm_xe_vm_create vm;
    int ret;

    memclear(vm);
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_VM_CREATE, &vm);
    if (ret != 0)
    {
        MOS_DRM_ASSERTMESSAGE("DRM_IOCTL_XE_VM_CREATE failed: %s",
            strerror(errno));
        return INVALID_VM;
    }

    return vm.vm_id;
}

static void
__mos_vm_destroy_xe(struct mos_bufmgr *bufmgr, uint32_t vm_id)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct drm_xe_vm_destroy vm_destroy;
    int ret;

    if (INVALID_VM == vm_id)
    {
        MOS_DRM_ASSERTMESSAGE("invalid vm_id");
        return;
    }

    memclear(vm_destroy);
    vm_destroy.vm_id = vm_id;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_VM_DESTROY, &vm_destroy);
    if (ret != 0)
    {
        MOS_DRM_ASSERTMESSAGE("DRM_IOCTL_XE_VM_DESTROY failed: %s",
            strerror(errno));
    }
}


static uint32_t
mos_vm_create_xe(struct mos_bufmgr *bufmgr)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;

    if (bufmgr_gem->vm_id != INVALID_VM)
    {
        return bufmgr_gem->vm_id;
    }
    else
    {
        return __mos_vm_create_xe(bufmgr);
    }
}

static void
mos_vm_destroy_xe(struct mos_bufmgr *bufmgr, uint32_t vm_id)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;

    if (vm_id != bufmgr_gem->vm_id)
    {
        __mos_vm_destroy_xe(bufmgr, vm_id);
    }
}

static struct mos_linux_context *
mos_context_create_shared_xe(
                            struct mos_bufmgr *bufmgr,
                            mos_linux_context* ctx,
                            __u32 flags,
                            bool bContextProtected,
                            void *engine_map,
                            uint8_t ctx_width,
                            uint8_t num_placements,
                            uint32_t ctx_type)
{
    MOS_UNUSED(ctx);
    MOS_UNUSED(ctx_type);
    MOS_UNUSED(bContextProtected);

    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, nullptr)
    MOS_DRM_CHK_NULL_RETURN_VALUE(engine_map, nullptr)

    static uint32_t dummy_exec_queue_id = 0;
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct mos_xe_context *context = nullptr;
    struct drm_xe_exec_queue_create create;
    int ret;
    uint16_t engine_class = ((struct drm_xe_engine_class_instance *)engine_map)[0].engine_class;

    memclear(create);
    create.width = ctx_width;
    create.num_placements = num_placements;
    create.vm_id = bufmgr_gem->vm_id;
    create.flags = flags;
    create.instances = (uintptr_t)engine_map;

    /**
     * Note: must use MOS_New to allocate buffer instead of malloc since mos_xe_context
     * contains std::vector and std::queue. Otherwise both will have no instance.
     */
    context = MOS_New(mos_xe_context);
    MOS_DRM_CHK_NULL_RETURN_VALUE(context, nullptr)

    /**
     * Set exec_queue timeslice for render/ compute only as WA to ensure exec sequence.
     * Note, this is caused by a potential issue in kmd since exec_queue preemption by plenty of WL w/ same priority.
     */
    if ((engine_class == DRM_XE_ENGINE_CLASS_RENDER
                || engine_class == DRM_XE_ENGINE_CLASS_COMPUTE)
                && (ctx_width * num_placements == 1)
                && bufmgr_gem->exec_queue_timeslice != EXEC_QUEUE_TIMESLICE_DEFAULT)
    {
        struct drm_xe_ext_set_property timeslice;
        memclear(timeslice);
        timeslice.property = DRM_XE_EXEC_QUEUE_SET_PROPERTY_TIMESLICE;
        /**
         * Note, this value indicates to maximum of time slice for WL instead of real waiting time.
         */
        timeslice.value = bufmgr_gem->exec_queue_timeslice;
        timeslice.base.name = DRM_XE_EXEC_QUEUE_EXTENSION_SET_PROPERTY;
        create.extensions = (uintptr_t)(&timeslice);
        MOS_DRM_NORMALMESSAGE("WA: exec_queue timeslice set by engine class(%d), value(%d)",
                    engine_class, bufmgr_gem->exec_queue_timeslice);
    }

    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_EXEC_QUEUE_CREATE, &create);

    MOS_DRM_CHK_STATUS_MESSAGE_RETURN_VALUE_WH_OP(ret, context, MOS_Delete, nullptr,
                "ioctl failed in DRM_IOCTL_XE_EXEC_QUEUE_CREATE, return error(%d)", ret);

    context->ctx.ctx_id = create.exec_queue_id;
    context->ctx_width = ctx_width;
    context->num_placements = num_placements;
    context->engine_class = ((struct drm_xe_engine_class_instance *)engine_map)[0].engine_class;
    context->is_protected = bContextProtected;
    context->flags = flags;
    context->ctx.bufmgr = bufmgr;
    context->ctx.vm_id = bufmgr_gem->vm_id;
    context->reset_count = 0;
    context->timeline_dep = nullptr;

    bufmgr_gem->m_lock.lock();
    context->dummy_exec_queue_id = ++dummy_exec_queue_id;
    bufmgr_gem->global_ctx_info[context->dummy_exec_queue_id] = context;
    bufmgr_gem->m_lock.unlock();
    return &context->ctx;
}

static struct mos_linux_context *
mos_context_create_xe(struct mos_bufmgr *bufmgr)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct mos_xe_context *context = nullptr;

    /**
     * Note: must use MOS_New to allocate buffer instead of malloc since mos_xe_context
     * contains std::queue. Otherwise queue will have no instance.
     */
    context = MOS_New(mos_xe_context);
    MOS_DRM_CHK_NULL_RETURN_VALUE(context, nullptr)

    context->ctx.ctx_id = INVALID_EXEC_QUEUE_ID;
    context->ctx_width = 0;
    context->ctx.bufmgr = bufmgr;
    context->ctx.vm_id = bufmgr_gem->vm_id;
    context->reset_count = 0;
    context->timeline_dep = nullptr;
    context->dummy_exec_queue_id = INVALID_EXEC_QUEUE_ID;
    return &context->ctx;
}

static struct mos_linux_context *
mos_context_create_ext_xe(
                            struct mos_bufmgr *bufmgr,
                            __u32 flags,
                            bool bContextProtected)
{
    MOS_UNUSED(flags);
    MOS_UNUSED(bContextProtected);

    return mos_context_create_xe(bufmgr);
}

static void
mos_context_destroy_xe(struct mos_linux_context *ctx)
{
    if (nullptr == ctx)
    {
        return;
    }

    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)(ctx->bufmgr);
    if (nullptr == bufmgr_gem)
    {
        return;
    }
    struct mos_xe_context *context = (struct mos_xe_context *)ctx;
    struct drm_xe_exec_queue_destroy exec_queue_destroy;
    int ret;
    bufmgr_gem->m_lock.lock();
    bufmgr_gem->sync_obj_rw_lock.lock();
    mos_sync_destroy_timeline_dep(bufmgr_gem->fd, context->timeline_dep);
    context->timeline_dep = nullptr;
    bufmgr_gem->global_ctx_info.erase(context->dummy_exec_queue_id);
    bufmgr_gem->sync_obj_rw_lock.unlock();
    bufmgr_gem->m_lock.unlock();

    if (INVALID_EXEC_QUEUE_ID == ctx->ctx_id)
    {
        MOS_Delete(context);
        return;
    }

    memclear(exec_queue_destroy);
    exec_queue_destroy.exec_queue_id = ctx->ctx_id;

    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_EXEC_QUEUE_DESTROY, &exec_queue_destroy);
    if (ret != 0)
        MOS_DRM_ASSERTMESSAGE("DRM_IOCTL_XE_EXEC_QUEUE_DESTROY failed: %s", strerror(errno));

    MOS_Delete(context);
}

/**
 * Restore banned exec_queue with newly created one
 * Note: this call is only for banned context restore, if using it
 * as other purpose, MUST pay attention to context->reset_count here.
 */
static int
__mos_context_restore_xe(struct mos_bufmgr *bufmgr,
            struct mos_linux_context *ctx)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, -EINVAL);
    MOS_DRM_CHK_NULL_RETURN_VALUE(ctx, -EINVAL);
    if (INVALID_EXEC_QUEUE_ID == ctx->ctx_id)
    {
        MOS_DRM_ASSERTMESSAGE("Unable to restore intel context, it is not supported");
        return -EINVAL;
    }
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct mos_xe_context *context = (struct mos_xe_context *)ctx;
    int ret;

    //query engine firstly
    uint32_t nengine = 0;
    ret = mos_query_engines_count_xe(bufmgr, &nengine);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN(ret,
                "query engine count of restore failed, return error(%d)", ret)
    struct drm_xe_engine_class_instance engine_map[nengine];
    ret = mos_query_engines_xe(bufmgr,
                context->engine_class,
                context->engine_caps,
                &nengine,
                (void*)engine_map);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN(ret,
                "query engine of restore failed, return error(%d)", ret)

    //create new exec queue
    struct drm_xe_exec_queue_create create;
    memclear(create);
    create.width = context->ctx_width;
    create.num_placements = context->num_placements;
    create.vm_id = context->ctx.vm_id;
    create.flags = context->flags;
    create.instances = (uintptr_t)engine_map;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_EXEC_QUEUE_CREATE, &create);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN(ret,
                "ioctl failed in DRM_IOCTL_XE_EXEC_QUEUE_CREATE of restore, return error(%d)", ret)

    //destroy old exec_queue
    struct drm_xe_exec_queue_destroy exec_queue_destroy;
    memclear(exec_queue_destroy);
    exec_queue_destroy.exec_queue_id = ctx->ctx_id;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_EXEC_QUEUE_DESTROY, &exec_queue_destroy);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN(ret,
                "ioctl failed in DRM_IOCTL_XE_EXEC_QUEUE_DESTROY of restore, return error(%d)", ret)

    //restore
    context->ctx.ctx_id = create.exec_queue_id;
    context->reset_count += 1;

    return MOS_XE_SUCCESS;
}

/**
 * Get the property of the ctx
 *
 * @ctx indicates to the context that to query
 * @property indicates to what property that to query
 * @value indicates to quired value with given property
 */
static int
__mos_get_context_property_xe(struct mos_bufmgr *bufmgr,
            struct mos_linux_context *ctx,
            uint32_t property,
            uint64_t &value)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, -EINVAL);
    MOS_DRM_CHK_NULL_RETURN_VALUE(ctx, -EINVAL);
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct drm_xe_exec_queue_get_property p;
    memclear(p);
    p.property = property;
    p.exec_queue_id = ctx->ctx_id;

    int ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_EXEC_QUEUE_GET_PROPERTY, &p);

    value = p.value;
    return ret;
}

/**
 * Allocate a section of virtual memory for a buffer, assigning an address.
 */
static uint64_t
__mos_bo_vma_alloc_xe(struct mos_bufmgr *bufmgr,
          enum mos_memory_zone memzone,
          uint64_t size,
          uint64_t alignment)
{
    CHK_CONDITION(nullptr == bufmgr, "nullptr bufmgr.\n", 0);
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bufmgr;
    /* Force alignment to be some number of pages */
    alignment = ALIGN(alignment, PAGE_SIZE);

    uint64_t addr = mos_vma_heap_alloc(&bufmgr_gem->vma_heap[memzone], size, alignment);

    // currently only support 48bit range address
    CHK_CONDITION((addr >> 48ull) != 0, "invalid address, over 48bit range.\n", 0);
    CHK_CONDITION((addr >> (MEMZONE_SYS == memzone ? 40ull : (MEMZONE_DEVICE == memzone  ? 41ull:42ull))) != 0, "invalid address, over memory zone range.\n", 0);
    CHK_CONDITION((addr % alignment) != 0, "invalid address, not meet aligment requirement.\n", 0);

    return addr;
}

static int
__mos_bo_set_offset_xe(MOS_LINUX_BO *bo)
{
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *) bo;
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo->bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bo_gem, -EINVAL)
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL)

    uint64_t offset = 0;
    uint64_t alignment = 0;

    if (0 == bo->offset64)
    {
        bufmgr_gem->m_lock.lock();

        /* On platforms where lmem only supports 64K pages, kmd requires us
         * to either align the va to 2M or seperate the lmem objects and smem
         * objects into different va zones to avoid mixing up lmem object and
         * smem object into same page table. For imported object, we don't know
         * if it's in lmem or smem. So, we need to align the va to 2M.
         */
        if (MEMZONE_PRIME == bo_gem->mem_region)
        {
            offset = __mos_bo_vma_alloc_xe(bo->bufmgr, (enum mos_memory_zone)bo_gem->mem_region, bo->size, PAGE_SIZE_2M);
        }
        else if (MEMZONE_DEVICE == bo_gem->mem_region)
        {
            alignment = MAX(bufmgr_gem->default_alignment[MOS_XE_MEM_CLASS_VRAM], PAGE_SIZE_64K);
            offset = __mos_bo_vma_alloc_xe(bo->bufmgr, (enum mos_memory_zone)bo_gem->mem_region, bo->size, PAGE_SIZE_64K);
        }
        else if (MEMZONE_SYS == bo_gem->mem_region)
        {
            alignment = MAX(bufmgr_gem->default_alignment[MOS_XE_MEM_CLASS_SYSMEM], PAGE_SIZE_64K);
            offset = __mos_bo_vma_alloc_xe(bo->bufmgr, (enum mos_memory_zone)bo_gem->mem_region, bo->size, PAGE_SIZE_64K);
        }
        else
        {
            MOS_DRM_ASSERTMESSAGE("Invalid mem_region:%d", bo_gem->mem_region);
        }

        bo->offset64 = offset;
        bo->offset = offset;

        bufmgr_gem->m_lock.unlock();
    }

    return 0;
}

static int __mos_vm_bind_xe(int fd, uint32_t vm_id, uint32_t exec_queue_id, uint32_t bo_handle,
          uint64_t offset, uint64_t addr, uint64_t size, uint16_t pat_index, uint32_t op, uint32_t flags,
          struct drm_xe_sync *sync, uint32_t num_syncs, uint64_t ext)
{
    int ret;

    struct drm_xe_vm_bind bind;
    memclear(bind);
    bind.extensions = ext;
    bind.vm_id = vm_id;
    bind.exec_queue_id = exec_queue_id;
    bind.num_binds = 1;
    bind.bind.obj = bo_handle;
    bind.bind.obj_offset = offset;
    bind.bind.range = size;
    bind.bind.pat_index = pat_index;
    bind.bind.addr = addr;
    bind.bind.op = op;
    bind.bind.flags = flags;
    bind.num_syncs = num_syncs;
    bind.syncs = (uintptr_t)sync;

    ret = drmIoctl(fd, DRM_IOCTL_XE_VM_BIND, &bind);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("Failed to bind vm, vm_id:%d, exec_queue_id:%d, op:0x%x, flags:0x%x, bo_handle:%d, offset:%lx, addr:0x%lx, size:%ld, pat_index:%d, errno(%d)",
            vm_id, exec_queue_id, op, flags, bo_handle, offset, addr, size, pat_index, -errno);
    }

    return ret;
}

static int mos_vm_bind_sync_xe(int fd, uint32_t vm_id, uint32_t bo, uint64_t offset,
        uint64_t addr, uint64_t size, uint16_t pat_index, uint32_t op)
{
    struct drm_xe_sync sync;

    memclear(sync);
    sync.flags = DRM_XE_SYNC_FLAG_SIGNAL;
    sync.type = DRM_XE_SYNC_TYPE_SYNCOBJ;
    sync.handle = mos_sync_syncobj_create(fd, 0);

    int ret = __mos_vm_bind_xe(fd, vm_id, 0, bo, offset, addr, size, pat_index,
                op, 0, &sync, 1,  0);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("ret:%d, error:%d", ret, -errno);
        mos_sync_syncobj_destroy(fd, sync.handle);
        return ret;
    }

    ret = mos_sync_syncobj_wait_err(fd, &sync.handle, 1, INT64_MAX, 0, NULL);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("syncobj_wait error:%d", -errno);
    }

    mos_sync_syncobj_destroy(fd, sync.handle);

    return ret;
}

static int mos_vm_bind_async_xe(int fd, uint32_t vm_id, uint32_t bo, uint64_t offset,
        uint64_t addr, uint64_t size, uint16_t pat_index, uint32_t op,
        struct drm_xe_sync *sync, uint32_t num_syncs)
{
    return __mos_vm_bind_xe(fd, vm_id, 0, bo, offset, addr, size, pat_index,
                op, 0, sync, num_syncs,    0);
}

drm_export struct mos_linux_bo *
mos_bo_alloc_xe(struct mos_bufmgr *bufmgr,
               struct mos_drm_bo_alloc *alloc)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bufmgr;
    struct mos_xe_bo_gem *bo_gem;
    struct drm_xe_gem_create create;
    uint32_t bo_align = alloc->alignment;
    int ret;

    /**
     * Note: must use MOS_New to allocate buffer instead of malloc since mos_xe_bo_gem
     * contains std::vector and std::map. Otherwise both will have no instance.
     */
    bo_gem = MOS_New(mos_xe_bo_gem);
    MOS_DRM_CHK_NULL_RETURN_VALUE(bo_gem, nullptr)
    memclear(bo_gem->bo);
    bo_gem->is_exported = false;
    bo_gem->is_imported = false;
    bo_gem->is_userptr = false;
    bo_gem->last_exec_read_exec_queue = INVALID_EXEC_QUEUE_ID;
    bo_gem->last_exec_write_exec_queue = INVALID_EXEC_QUEUE_ID;
    atomic_set(&bo_gem->map_count, 0);
    bo_gem->mem_virtual = nullptr;
    bo_gem->mem_region = MEMZONE_SYS;
    bo_align = MAX(alloc->alignment, bufmgr_gem->default_alignment[MOS_XE_MEM_CLASS_SYSMEM]);

    if (bufmgr_gem->has_vram &&
            (MOS_MEMPOOL_VIDEOMEMORY == alloc->ext.mem_type || MOS_MEMPOOL_DEVICEMEMORY == alloc->ext.mem_type))
    {
        bo_gem->mem_region = MEMZONE_DEVICE;
        bo_align = MAX(alloc->alignment, bufmgr_gem->default_alignment[MOS_XE_MEM_CLASS_VRAM]);
        alloc->ext.cpu_cacheable = false;
    }

    memclear(create);
    if (MEMZONE_DEVICE == bo_gem->mem_region)
    {
        //Note: memory_region is related to gt_id for multi-tiles gpu, take gt_id into consideration in case of multi-tiles
        create.placement = bufmgr_gem->mem_regions_mask & (~0x1);
    }
    else
    {
        create.placement = bufmgr_gem->mem_regions_mask & 0x1;
    }

    //Note: We suggest vm_id=0 here as default, otherwise this bo cannot be exported as prelim fd.
    create.vm_id = 0;
    create.size = ALIGN(alloc->size, bo_align);

    /**
     * Note: current, it only supports WB/ WC while UC and other cache are not allowed.
     */
    create.cpu_caching = alloc->ext.cpu_cacheable ? DRM_XE_GEM_CPU_CACHING_WB : DRM_XE_GEM_CPU_CACHING_WC;

    if ((strcmp(alloc->name, "MEDIA") == 0 || strcmp(alloc->name, "Media") == 0)
        && create.cpu_caching == DRM_XE_GEM_CPU_CACHING_WC)
            create.flags |= DRM_XE_GEM_CREATE_FLAG_SCANOUT;

    ret = drmIoctl(bufmgr_gem->fd,
        DRM_IOCTL_XE_GEM_CREATE,
        &create);
    MOS_DRM_CHK_STATUS_MESSAGE_RETURN_VALUE_WH_OP(ret, bo_gem, MOS_Delete, nullptr,
                "ioctl failed in DRM_IOCTL_XE_GEM_CREATE, return error(%d)", ret);

    bo_gem->gem_handle = create.handle;
    bo_gem->bo.handle = bo_gem->gem_handle;
    bo_gem->bo.size    = create.size;
    bo_gem->bo.vm_id = INVALID_VM;
    bo_gem->bo.bufmgr = bufmgr;
    bo_gem->bo.align = bo_align;
    bo_gem->cpu_caching = create.cpu_caching;
    /**
     * Note: Better to get a default pat_index to overwite invalid argv. Normally it should not happen.
     */
    bo_gem->pat_index = alloc->ext.pat_index == PAT_INDEX_INVALID ? 0 : alloc->ext.pat_index;

    if (bufmgr_gem->mem_profiler_fd != -1)
    {
        snprintf(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE, "GEM_CREATE, %d, %d, %lu, %d, %s\n",
                    getpid(), bo_gem->bo.handle, bo_gem->bo.size,bo_gem->mem_region, alloc->name);
        ret = write(bufmgr_gem->mem_profiler_fd,
                    bufmgr_gem->mem_profiler_buffer,
                    strnlen(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE));
        if (-1 == ret)
        {
            MOS_DRM_ASSERTMESSAGE("Failed to write to %s: %s",
                        bufmgr_gem->mem_profiler_path, strerror(errno));
        }
    }

    /* drm_intel_gem_bo_free calls DRMLISTDEL() for an uninitialized
       list (vma_list), so better set the list head here */
    DRMINITLISTHEAD(&bo_gem->name_list);

    memcpy(bo_gem->name, alloc->name, (strlen(alloc->name) + 1) > MAX_NAME_SIZE ? MAX_NAME_SIZE : (strlen(alloc->name) + 1));
    atomic_set(&bo_gem->ref_count, 1);

    MOS_DRM_NORMALMESSAGE("buf %d (%s) %ldb, bo:0x%lx",
        bo_gem->gem_handle, alloc->name, alloc->size, (uint64_t)&bo_gem->bo);

    __mos_bo_set_offset_xe(&bo_gem->bo);

    ret = mos_vm_bind_sync_xe(bufmgr_gem->fd,
                    bufmgr_gem->vm_id,
                    bo_gem->gem_handle,
                    0,
                    bo_gem->bo.offset64,
                    bo_gem->bo.size,
                    bo_gem->pat_index,
                    DRM_XE_VM_BIND_OP_MAP);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("mos_vm_bind_sync_xe ret: %d", ret);
        mos_bo_free_xe(&bo_gem->bo);
        return nullptr;
    }
    else
    {
        bo_gem->bo.vm_id = bufmgr_gem->vm_id;
    }

    return &bo_gem->bo;
}

static unsigned long
__mos_bo_tile_size_xe(struct mos_xe_bufmgr_gem *bufmgr_gem, unsigned long size,
               uint32_t *tiling_mode, uint32_t alignment)
{
    unsigned long min_size, max_size;
    unsigned long i;

    if (TILING_NONE == *tiling_mode)
        return size;

    /* 965+ just need multiples of page size for tiling */
    return ROUND_UP_TO(size, alignment);

}

/*
 * Round a given pitch up to the minimum required for X tiling on a
 * given chip.  We use 512 as the minimum to allow for a later tiling
 * change.
 */
static unsigned long
__mos_bo_tile_pitch_xe(struct mos_xe_bufmgr_gem *bufmgr_gem,
                unsigned long pitch, uint32_t *tiling_mode)
{
    unsigned long tile_width;
    unsigned long i;

    /* If untiled, then just align it so that we can do rendering
     * to it with the 3D engine.
     */
    if (TILING_NONE == *tiling_mode)
        return ALIGN(pitch, 64);

    if (TILING_X == *tiling_mode)
        tile_width = 512;
    else
        tile_width = 128;

    /* 965 is flexible */
    return ROUND_UP_TO(pitch, tile_width);
}

static struct mos_linux_bo *
mos_bo_alloc_tiled_xe(struct mos_bufmgr *bufmgr,
                 struct mos_drm_bo_alloc_tiled *alloc_tiled)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bufmgr;
    unsigned long size, stride;
    uint32_t tiling;

    uint32_t alignment = bufmgr_gem->default_alignment[MOS_XE_MEM_CLASS_SYSMEM];

    if (bufmgr_gem->has_vram &&
       (MOS_MEMPOOL_VIDEOMEMORY == alloc_tiled->ext.mem_type   || MOS_MEMPOOL_DEVICEMEMORY == alloc_tiled->ext.mem_type))
    {
        alignment = bufmgr_gem->default_alignment[MOS_XE_MEM_CLASS_VRAM];
    }

    do {
        unsigned long aligned_y, height_alignment;

        tiling = alloc_tiled->ext.tiling_mode;

        /* If we're tiled, our allocations are in 8 or 32-row blocks,
         * so failure to align our height means that we won't allocate
         * enough pages.
         *
         * If we're untiled, we still have to align to 2 rows high
         * because the data port accesses 2x2 blocks even if the
         * bottom row isn't to be rendered, so failure to align means
         * we could walk off the end of the GTT and fault.  This is
         * documented on 965, and may be the case on older chipsets
         * too so we try to be careful.
         */
        aligned_y = alloc_tiled->y;
        height_alignment = 2;

        if (TILING_X == tiling)
            height_alignment = 8;
        else if (TILING_Y == tiling)
            height_alignment = 32;
        aligned_y = ALIGN(alloc_tiled->y, height_alignment);

        stride = alloc_tiled->x * alloc_tiled->cpp;
        stride = __mos_bo_tile_pitch_xe(bufmgr_gem, stride, &alloc_tiled->ext.tiling_mode);
        size = stride * aligned_y;
        size = __mos_bo_tile_size_xe(bufmgr_gem, size, &alloc_tiled->ext.tiling_mode, alignment);
    } while (alloc_tiled->ext.tiling_mode != tiling);

    alloc_tiled->pitch = stride;

    struct mos_drm_bo_alloc alloc;
    alloc.name = alloc_tiled->name;
    alloc.size = size;
    alloc.alignment = alignment;
    alloc.ext = alloc_tiled->ext;

    return mos_bo_alloc_xe(bufmgr, &alloc);
}

drm_export struct mos_linux_bo *
mos_bo_alloc_userptr_xe(struct mos_bufmgr *bufmgr,
                struct mos_drm_bo_alloc_userptr *alloc_uptr)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bufmgr;
    struct mos_xe_bo_gem *bo_gem;
    int ret;

    /**
     * Note: must use MOS_New to allocate buffer instead of malloc since mos_xe_bo_gem
     * contains std::vector and std::map. Otherwise both will have no instance.
     */
    bo_gem = MOS_New(mos_xe_bo_gem);
    MOS_DRM_CHK_NULL_RETURN_VALUE(bo_gem, nullptr)
    memclear(bo_gem->bo);
    bo_gem->is_exported = false;
    bo_gem->is_imported = false;
    bo_gem->is_userptr = true;
    bo_gem->last_exec_read_exec_queue = INVALID_EXEC_QUEUE_ID;
    bo_gem->last_exec_write_exec_queue = INVALID_EXEC_QUEUE_ID;
    atomic_set(&bo_gem->map_count, 0);
    bo_gem->mem_virtual = alloc_uptr->addr;
    bo_gem->gem_handle = INVALID_HANDLE;
    bo_gem->bo.handle = INVALID_HANDLE;
    bo_gem->bo.size    = alloc_uptr->size;
    bo_gem->pat_index = alloc_uptr->pat_index == PAT_INDEX_INVALID ? 0 : alloc_uptr->pat_index;
    bo_gem->bo.bufmgr = bufmgr;
    bo_gem->bo.vm_id = INVALID_VM;
    bo_gem->mem_region = MEMZONE_SYS;

    /* Save the address provided by user */
#ifdef __cplusplus
    bo_gem->bo.virt   = alloc_uptr->addr;
#else
    bo_gem->bo.virtual   = alloc_uptr->addr;
#endif

    /* drm_intel_gem_bo_free calls DRMLISTDEL() for an uninitialized
       list (vma_list), so better set the list head here */
    DRMINITLISTHEAD(&bo_gem->name_list);

    memcpy(bo_gem->name, alloc_uptr->name, (strlen(alloc_uptr->name) + 1) > MAX_NAME_SIZE ? MAX_NAME_SIZE : (strlen(alloc_uptr->name) + 1));
    atomic_set(&bo_gem->ref_count, 1);

    __mos_bo_set_offset_xe(&bo_gem->bo);

    ret = mos_vm_bind_sync_xe(bufmgr_gem->fd,
                bufmgr_gem->vm_id,
                0,
                (uint64_t)alloc_uptr->addr,
                bo_gem->bo.offset64,
                bo_gem->bo.size,
                bo_gem->pat_index,
                DRM_XE_VM_BIND_OP_MAP_USERPTR);

    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("mos_xe_vm_bind_userptr_sync ret: %d", ret);
        mos_bo_free_xe(&bo_gem->bo);
        return nullptr;
    }
    else
    {
        bo_gem->bo.vm_id = bufmgr_gem->vm_id;
    }

    MOS_DRM_NORMALMESSAGE("mos_bo_alloc_userptr_xe: buf (%s) %ldb, bo:0x%lx",
        alloc_uptr->name, alloc_uptr->size, (uint64_t)&bo_gem->bo);


    return &bo_gem->bo;
}

static struct mos_linux_bo *
mos_bo_create_from_prime_xe(struct mos_bufmgr *bufmgr, struct mos_drm_bo_alloc_prime *alloc_prime)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bufmgr;
    int ret;
    uint32_t handle;
    struct mos_xe_bo_gem *bo_gem;
    int prime_fd = alloc_prime->prime_fd;
    int size = alloc_prime->size;
    uint16_t pat_index = alloc_prime->pat_index;
    drmMMListHead *list;

    bufmgr_gem->m_lock.lock();
    ret = drmPrimeFDToHandle(bufmgr_gem->fd, prime_fd, &handle);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("create_from_prime: failed to obtain handle from fd: %s", strerror(errno));
        bufmgr_gem->m_lock.unlock();
        return nullptr;
    }

    /*
     * See if the kernel has already returned this buffer to us. Just as
     * for named buffers, we must not create two bo's pointing at the same
     * kernel object
     */
    for (list = bufmgr_gem->named.next; list != &bufmgr_gem->named; list = list->next)
    {
        bo_gem = DRMLISTENTRY(struct mos_xe_bo_gem, list, name_list);
        if (bo_gem->gem_handle == handle)
        {
            mos_bo_reference_xe(&bo_gem->bo);
            bufmgr_gem->m_lock.unlock();
            return &bo_gem->bo;
        }
    }

    bo_gem = MOS_New(mos_xe_bo_gem);
    if (!bo_gem)
    {
        bufmgr_gem->m_lock.unlock();
        return nullptr;
    }

    memclear(bo_gem->bo);
    bo_gem->is_exported = false;
    bo_gem->is_imported = true;
    bo_gem->is_userptr = false;
    bo_gem->last_exec_read_exec_queue = INVALID_EXEC_QUEUE_ID;
    bo_gem->last_exec_write_exec_queue = INVALID_EXEC_QUEUE_ID;
    atomic_set(&bo_gem->map_count, 0);
    bo_gem->mem_virtual = nullptr;

    /* Determine size of bo.  The fd-to-handle ioctl really should
     * return the size, but it doesn't.  If we have kernel 3.12 or
     * later, we can lseek on the prime fd to get the size.  Older
     * kernels will just fail, in which case we fall back to the
     * provided (estimated or guess size). */
    ret = lseek(prime_fd, 0, SEEK_END);
    if (ret != -1)
        bo_gem->bo.size = ret;
    else
        bo_gem->bo.size = size;

    bo_gem->bo.handle = handle;
    /*
     * Note: Need to get the pat_index by the customer_gmminfo with 1way coherency at least.
     */
    bo_gem->pat_index = pat_index == PAT_INDEX_INVALID ? 0 : pat_index;
    bo_gem->bo.bufmgr = bufmgr;

    bo_gem->gem_handle = handle;
    atomic_set(&bo_gem->ref_count, 1);

    /**
     * change bo_gem->name to const char*
     */
    memcpy(bo_gem->name, alloc_prime->name, sizeof("prime"));
    bo_gem->mem_region = MEMZONE_PRIME;

    DRMLISTADDTAIL(&bo_gem->name_list, &bufmgr_gem->named);
    bufmgr_gem->m_lock.unlock();

    __mos_bo_set_offset_xe(&bo_gem->bo);

    ret = mos_vm_bind_sync_xe(bufmgr_gem->fd,
                bufmgr_gem->vm_id,
                bo_gem->gem_handle,
                0,
                bo_gem->bo.offset64,
                bo_gem->bo.size,
                bo_gem->pat_index,
                DRM_XE_VM_BIND_OP_MAP);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("mos_vm_bind_sync_xe ret: %d", ret);
        mos_bo_free_xe(&bo_gem->bo);
        return nullptr;
    }
    else
    {
        bo_gem->bo.vm_id = bufmgr_gem->vm_id;
    }

    return &bo_gem->bo;
}

static int
mos_bo_export_to_prime_xe(struct mos_linux_bo *bo, int *prime_fd)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo->bufmgr;
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *) bo;

    bufmgr_gem->m_lock.lock();
    if (DRMLISTEMPTY(&bo_gem->name_list))
        DRMLISTADDTAIL(&bo_gem->name_list, &bufmgr_gem->named);
    bufmgr_gem->m_lock.unlock();

    mos_gem_bo_wait_rendering_xe(bo);

    if (drmPrimeHandleToFD(bufmgr_gem->fd, bo_gem->gem_handle,
                   DRM_CLOEXEC, prime_fd) != 0)
        return -errno;

    bo_gem->is_exported = true;

    return 0;
}

/**
 * Update exec list for submission.
 *
 * @cmd_bo indicates to cmd bo for the exec submission.
 * @exec_bo indicates to the gpu resource for exec submission.
 * @write_flag indicates to whether exec bo's operation write on GPU.
 */
static int
mos_gem_bo_update_exec_list_xe(struct mos_linux_bo *cmd_bo, struct mos_linux_bo *exec_bo, bool write_flag)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(cmd_bo, -EINVAL)
    MOS_DRM_CHK_NULL_RETURN_VALUE(exec_bo, -EINVAL)
    struct mos_xe_bo_gem *cmd_bo_gem = (struct mos_xe_bo_gem *) cmd_bo;
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) cmd_bo->bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL)
    std::map<uintptr_t, struct mos_xe_exec_bo> &exec_list = cmd_bo_gem->exec_list;

    if (exec_bo->handle == cmd_bo->handle)
    {
        MOS_DRM_NORMALMESSAGE("cmd bo should not add into exec list, skip it");
        return MOS_XE_SUCCESS;
    }
    uintptr_t key = (uintptr_t)exec_bo;
    if (exec_list.count(key) > 0)
    {
        /**
         * This exec bo has added before, but need to update its exec flags.
         */

        // For all BOs with read and write usages, we could just assign write flag to reduce read deps size.
        if (write_flag || (exec_list[key].flags & EXEC_OBJECT_WRITE_XE))
        {
            exec_list[key].flags = EXEC_OBJECT_WRITE_XE;
        }
        else
        {
            // For BOs only with read usage, we should assign read flag.
            exec_list[key].flags |= EXEC_OBJECT_READ_XE;
        }
    }
    else
    {
        struct mos_xe_exec_bo target;
        target.bo = exec_bo;
        target.flags = write_flag ? EXEC_OBJECT_WRITE_XE : EXEC_OBJECT_READ_XE;
        exec_list[key] = target;
        mos_bo_reference_xe(exec_bo);
    }
    return MOS_XE_SUCCESS;
}

/**
 * Clear the exec bo from the list after submission.
 *
 * @cmd_bo indicates to cmd bo for the exec submission.
 * @start is unused.
 */
static void
mos_gem_bo_clear_exec_list_xe(struct mos_linux_bo *cmd_bo, int start)
{
    MOS_UNUSED(start);
    if (cmd_bo != nullptr && cmd_bo->bufmgr != nullptr)
    {
        struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) cmd_bo->bufmgr;
        struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *) cmd_bo;
        std::map<uintptr_t, struct mos_xe_exec_bo> &exec_list = bo_gem->exec_list;

        for (auto &it : exec_list) {
            mos_bo_unreference_xe(it.second.bo);
        }
        exec_list.clear();
    }
}

/**
 * This is to dump all pending execution timeline done on such bo
 */
int
__mos_dump_bo_wait_rendering_timeline_xe(uint32_t bo_handle,
            uint32_t *handles,
            uint64_t *points,
            uint32_t count,
            int64_t timeout_nsec,
            uint32_t wait_flags,
            uint32_t rw_flags)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if (__XE_TEST_DEBUG(XE_DEBUG_SYNCHRONIZATION))
    {
        MOS_DRM_CHK_NULL_RETURN_VALUE(handles, -EINVAL)
        char log_msg[MOS_MAX_MSG_BUF_SIZE] = { 0 };
        int offset = 0;
        offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                            MOS_MAX_MSG_BUF_SIZE - offset,
                            "\n\t\t\tdump bo wait rendering: bo handle = %d, timeout_nsec = %ld, wait_flags = %d, rw_flags = %d",
                            bo_handle,
                            timeout_nsec,
                            wait_flags,
                            rw_flags);

        for (int i = 0; i < count; i++)
        {
            offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                            MOS_MAX_MSG_BUF_SIZE - offset,
                            "\n\t\t\t-syncobj handle = %d, timeline = %ld",
                            handles[i],
                            points[i]);
        }

        offset > MOS_MAX_MSG_BUF_SIZE ?
            MOS_DRM_NORMALMESSAGE("imcomplete dump since log msg buffer overwrite %s", log_msg) : MOS_DRM_NORMALMESSAGE("%s", log_msg);
    }
#endif
    return MOS_XE_SUCCESS;
}

/**
 * @bo indicates to bo object that need to wait
 * @timeout_nsec indicates to timeout in nanosecond:
 *     if timeout_nsec > 0, waiting for given time, if timeout, return -ETIME;
 *     if timeout_nsec ==0, check bo busy state, if busy, return -ETIME imediately;
 * @wait_flags indicates wait operation, it supports wait all, wait submit, wait available or wait any;
 *     refer drm syncobj to get more details in drm.h
 * @rw_flags indicates to read/write operation:
 *     if rw_flags & EXEC_OBJECT_WRITE_XE, means bo write. Otherwise it means bo read.
 * @first_signaled indicates to first signaled syncobj handle in the handls array.
 */
static int
__mos_gem_bo_wait_timeline_rendering_with_flags_xe(struct mos_linux_bo *bo,
            int64_t timeout_nsec,
            uint32_t wait_flags,
            uint32_t rw_flags,
            uint32_t *first_signaled)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bo, -EINVAL)

    mos_xe_bufmgr_gem *bufmgr_gem = (mos_xe_bufmgr_gem *)bo->bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL)

    int ret = MOS_XE_SUCCESS;
    uint32_t count = 0;
    mos_xe_bo_gem *bo_gem = (mos_xe_bo_gem *)bo;
    std::map<uint32_t, uint64_t> timeline_data; //pair(syncobj, point)
    std::vector<uint32_t> handles;
    std::vector<uint64_t> points;
    std::set<uint32_t> exec_queue_ids;
    bufmgr_gem->m_lock.lock();
    bufmgr_gem->sync_obj_rw_lock.lock_shared();
    MOS_XE_GET_KEYS_FROM_MAP(bufmgr_gem->global_ctx_info, exec_queue_ids);

    mos_sync_get_bo_wait_timeline_deps(exec_queue_ids,
                bo_gem->read_deps,
                bo_gem->write_deps,
                timeline_data,
                bo_gem->last_exec_write_exec_queue,
                rw_flags);
    bufmgr_gem->m_lock.unlock();

    for (auto it : timeline_data)
    {
        handles.push_back(it.first);
        points.push_back(it.second);
    }

    count = handles.size();
    if (count > 0)
    {
        ret = mos_sync_syncobj_timeline_wait(bufmgr_gem->fd,
                        handles.data(),
                        points.data(),
                        count,
                        timeout_nsec,
                        wait_flags,
                        first_signaled);

        __mos_dump_bo_wait_rendering_timeline_xe(bo_gem->gem_handle,
                        handles.data(),
                        points.data(),
                        count,
                        timeout_nsec,
                        wait_flags,
                        rw_flags);
    }
    bufmgr_gem->sync_obj_rw_lock.unlock_shared();

    return ret;
}

/**
 * Check if bo is still busy state.
 *
 * Check if read dep on all exec_queue and write dep on last write exec_queue are signaled.
 * If any one dep is not signaled, that means this bo is busy and return -ETIME immediately.
 * Otheriwise, move all dep on this bo from busy queue to free queue for reuse.
 */
static int
mos_gem_bo_busy_xe(struct mos_linux_bo *bo)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bo, -EINVAL);
    mos_xe_bufmgr_gem *bufmgr_gem = (mos_xe_bufmgr_gem *)bo->bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL)

    int64_t timeout_nsec = 0;
    uint32_t wait_flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
    uint32_t rw_flags = EXEC_OBJECT_READ_XE | EXEC_OBJECT_WRITE_XE;

    int ret =  __mos_gem_bo_wait_timeline_rendering_with_flags_xe(bo, timeout_nsec, wait_flags, rw_flags, nullptr);

    if (ret)
    {
        //busy
        if (errno != ETIME)
        {
            MOS_DRM_ASSERTMESSAGE("bo_busy_xe ret:%d, error:%d", ret, -errno);
        }
        return true;
    }
    else if (MOS_XE_SUCCESS == ret)
    {
        //free
        return false;
    }

    return false;
}

/**
 * Waits for all GPU rendering with the object to have completed.
 *
 * Wait read dep on all exec_queue and write dep on last write exec_queue are signaled.
 * And move all dep on this bo from busy queue to free queue for reuse after rendering completed.
 */
static void
mos_gem_bo_wait_rendering_xe(struct mos_linux_bo *bo)
{
    if (bo == nullptr || bo->bufmgr == nullptr)
    {
        MOS_DRM_ASSERTMESSAGE("ptr is null pointer");
        return;
    }
    mos_xe_bufmgr_gem *bufmgr_gem = (mos_xe_bufmgr_gem *)bo->bufmgr;

    int64_t timeout_nsec = INT64_MAX;
    uint32_t wait_flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
    uint32_t rw_flags = EXEC_OBJECT_READ_XE | EXEC_OBJECT_WRITE_XE;

    int ret =  __mos_gem_bo_wait_timeline_rendering_with_flags_xe(bo, timeout_nsec, wait_flags, rw_flags, nullptr);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("bo_wait_rendering_xe ret:%d, error:%d", ret, -errno);
    }
}

/**
 * @timeout_ns indicates to timeout for waiting, but it is fake timeout;
 *     it only indicates to wait bo rendering completed or check bo busy state.
 *     if timeout_ns != 0, wait bo rendering completed.
 *     if timeout_ns == 0. check bo busy state.
 */
static int
mos_gem_bo_wait_xe(struct mos_linux_bo *bo, int64_t timeout_ns)
{
    if (timeout_ns)
    {
        mos_gem_bo_wait_rendering_xe(bo);
        return 0;
    }
    else
    {
        return mos_gem_bo_busy_xe(bo) ? -ETIME : 0;
    }
    return 0;
}

/**
 * Map gpu resource for CPU read or write.
 *
 * 1. if map for write, it should wait read dep on all exec_queue and write dep on last write exec_queue signaled.
 * 2. if map for read, it should only wait write dep on last write exec_queue signaled.
 *
 * After bo rendering completed on GPU, then CPU could continue its read or write operation.
 */
static int
mos_bo_map_xe(struct mos_linux_bo *bo, int write_enable)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bo, -EINVAL)
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo->bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL)
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *) bo;
    int ret;

    int64_t timeout_nsec = INT64_MAX;
    uint32_t wait_flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
    uint32_t rw_flags = write_enable ? EXEC_OBJECT_WRITE_XE : EXEC_OBJECT_READ_XE;

    ret =  __mos_gem_bo_wait_timeline_rendering_with_flags_xe(bo, timeout_nsec, wait_flags, rw_flags, nullptr);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("bo wait rendering error(%d ns)", -errno);
    }

    if (bo_gem->is_userptr)
    {
        /* Return the same user ptr */
        return 0;
    }

    bufmgr_gem->m_lock.lock();
    if (nullptr == bo_gem->mem_virtual)
    {
        struct drm_xe_gem_mmap_offset mmo;
        memclear(mmo);
        mmo.handle = bo->handle;
        ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_GEM_MMAP_OFFSET, &mmo);
        if (ret)
        {
            bufmgr_gem->m_lock.unlock();
            return ret;
        }

        bo_gem->mem_virtual = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE,
            MAP_SHARED, bufmgr_gem->fd, mmo.offset);
        if (MAP_FAILED == bo_gem->mem_virtual)
        {
            bo_gem->mem_virtual = nullptr;
            ret = -errno;
            MOS_DRM_ASSERTMESSAGE("Error mapping buffer %d (%s): %s .",
                bo_gem->gem_handle, bo_gem->name,
                strerror(errno));
        }
    }

#ifdef __cplusplus
    bo->virt = bo_gem->mem_virtual;
#else
    bo->virtual = bo_gem->mem_virtual;
#endif

    atomic_inc(&bo_gem->map_count);

    __mos_bo_mark_mmaps_incoherent_xe(bo);
    VG(VALGRIND_MAKE_MEM_DEFINED(bo_gem->mem_virtual, bo->size));
    bufmgr_gem->m_lock.unlock();

    return 0;
}

static int
mos_bo_map_wc_xe(struct mos_linux_bo *bo)
{
    return mos_bo_map_xe(bo, false);
}

static int mos_bo_unmap_xe(struct mos_linux_bo *bo)
{
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *) bo;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bo_gem, 0)
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo->bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, 0)

    if (bo_gem->is_userptr)
        return 0;

    bufmgr_gem->m_lock.lock();

    if (atomic_dec_and_test(&bo_gem->map_count))
    {
       __mos_bo_mark_mmaps_incoherent_xe(bo);
#ifdef __cplusplus
        bo->virt = nullptr;
#else
        bo->virtual = nullptr;
#endif
    }
    bufmgr_gem->m_lock.unlock();

    return 0;
}

static int
mos_bo_unmap_wc_xe(struct mos_linux_bo *bo)
{
    return mos_bo_unmap_xe(bo);
}

/**
 *This aims to dump the sync info on such execution.
 *@syncs contains fence in from bo who has dependency on
 *currect execution and a fence out in @dep from current execution.
 */
int __mos_dump_syncs_array_xe(struct drm_xe_sync *syncs,
            uint32_t count,
            mos_xe_dep *dep)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if (__XE_TEST_DEBUG(XE_DEBUG_SYNCHRONIZATION))
    {
        MOS_DRM_CHK_NULL_RETURN_VALUE(syncs, -EINVAL)
        MOS_DRM_CHK_NULL_RETURN_VALUE(dep, -EINVAL)
        char log_msg[MOS_MAX_MSG_BUF_SIZE] = { 0 };
        int offset = 0;
        offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                    MOS_MAX_MSG_BUF_SIZE - offset,
                    "\n\t\t\tdump fence out syncobj: handle = %d, timeline = %ld",
                    dep->timeline_index);
        if (count > 0)
        {
            offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                    MOS_MAX_MSG_BUF_SIZE - offset,
                    "\n\t\t\tdump exec syncs array, num sync = %d",
                    count);
        }
        for (int i = 0; i < count; i++)
        {
            /**
             * Note: we assume all are timeline sync here, and change later when any other
             * types sync in use.
             */
            offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                    MOS_MAX_MSG_BUF_SIZE - offset,
                    "\n\t\t\t-syncobj_handle = %d, timeline = %ld, sync type = %d, sync flags = %d",
                    syncs[i].handle, syncs[i].timeline_value, syncs[i].type, syncs[i].flags);
        }
        offset > MOS_MAX_MSG_BUF_SIZE ?
            MOS_DRM_NORMALMESSAGE("imcomplete dump since log msg buffer overwrite %s", log_msg) : MOS_DRM_NORMALMESSAGE("%s", log_msg);
    }
#endif
    return MOS_XE_SUCCESS;
}

/**
 * This is to dump timeline for each exec bo on such execution,
 * pair of execed_queue_id & timeline_value will be dumped.
 */
int
__mos_dump_bo_deps_map_xe(struct mos_linux_bo **bo,
            int num_bo,
            std::vector<mos_xe_exec_bo> &exec_list,
            uint32_t curr_exec_queue_id,
            std::map<uint32_t, struct mos_xe_context*> ctx_infos)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if (__XE_TEST_DEBUG(XE_DEBUG_SYNCHRONIZATION))
    {
        MOS_DRM_CHK_NULL_RETURN_VALUE(bo, -EINVAL)
        uint32_t exec_list_size = exec_list.size();
        for (int i = 0; i < exec_list_size + num_bo; i++)
        {
            mos_xe_bo_gem *exec_bo_gem = nullptr;
            uint32_t exec_flags = 0;
            if (i < exec_list_size)
            {
                exec_bo_gem = (mos_xe_bo_gem *)exec_list[i].bo;
                exec_flags = exec_list[i].flags;
            }
            else
            {
                exec_bo_gem = (mos_xe_bo_gem *)bo[i - exec_list_size];
                exec_flags = EXEC_OBJECT_WRITE_XE; //use write flags for batch bo as default.
            }
            if (exec_bo_gem)
            {
                if (exec_bo_gem->is_imported || exec_bo_gem->is_exported)
                {
                    MOS_DRM_NORMALMESSAGE("\n\t\t\tdump external bo, handle=%d, without deps map, skip dump", exec_bo_gem->bo.handle);
                }
                else
                {
                    char log_msg[MOS_MAX_MSG_BUF_SIZE] = { 0 };
                    int offset = 0;
                    offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                                    MOS_MAX_MSG_BUF_SIZE - offset,
                                    "\n\t\t\tdump %s dep: bo handle=%d, curr_exec_queue_id=%d, curr_op_flags=%d",
                                    i >= exec_list_size ? "batch bo" : "exec bo",
                                    exec_bo_gem->bo.handle,
                                    curr_exec_queue_id,
                                    exec_flags);

                    auto it =  exec_bo_gem->read_deps.begin();
                    while (it != exec_bo_gem->read_deps.end())
                    {
                        if (ctx_infos.count(it->first) > 0)
                        {
                            offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                                            MOS_MAX_MSG_BUF_SIZE - offset,
                                            "\n\t\t\t-read deps: execed_exec_queue_id=%d, syncobj_handle=%d", "timeline = %ld",
                                            it->first,
                                            it->second.dep ? it->second.dep->syncobj_handle : INVALID_HANDLE,
                                            it->second.dep ? it->second.exec_timeline_index : INVALID_HANDLE);
                        }
                        it++;
                    }

                    it = exec_bo_gem->write_deps.begin();
                    while (it != exec_bo_gem->write_deps.end())
                    {
                        if (ctx_infos.count(it->first) > 0)
                        {
                            offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                                            MOS_MAX_MSG_BUF_SIZE - offset,
                                            "\n\t\t\t-write deps: execed_exec_queue_id=%d, syncobj_handle=%d", "timeline = %ld",
                                            it->first,
                                            it->second.dep ? it->second.dep->syncobj_handle : INVALID_HANDLE,
                                            it->second.dep ? it->second.exec_timeline_index : INVALID_HANDLE);
                        }
                        it++;
                    }
                    offset > MOS_MAX_MSG_BUF_SIZE ?
                        MOS_DRM_NORMALMESSAGE("imcomplete dump since log msg buffer overwrite %s", log_msg) : MOS_DRM_NORMALMESSAGE("%s", log_msg);
                }
            }
        }
    }
#endif
    return MOS_XE_SUCCESS;
}

static int
__mos_context_exec_update_syncs_xe(struct mos_xe_bufmgr_gem *bufmgr_gem,
            struct mos_linux_bo **bo,
            int num_bo,
            struct mos_xe_context *ctx,
            std::vector<mos_xe_exec_bo> &exec_list,
            std::vector<struct drm_xe_sync> &syncs,
            std::vector<struct mos_xe_external_bo_info> &external_bos)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(ctx, -EINVAL);
    uint32_t curr_dummy_exec_queue_id = ctx->dummy_exec_queue_id;
    uint32_t exec_list_size = exec_list.size();
    int ret = 0;
    std::set<uint32_t> exec_queue_ids;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL);
    MOS_XE_GET_KEYS_FROM_MAP(bufmgr_gem->global_ctx_info, exec_queue_ids);

    for (int i = 0; i < exec_list_size + num_bo; i++)
    {
        mos_xe_bo_gem *exec_bo_gem = nullptr;
        uint32_t exec_flags = 0;
        if (i < exec_list_size)
        {
            //exec list bo
            exec_bo_gem = (mos_xe_bo_gem *)exec_list[i].bo;
            exec_flags = exec_list[i].flags;
        }
        else
        {
            //batch bo
            exec_bo_gem = (mos_xe_bo_gem *)bo[i - exec_list_size];
            exec_flags = EXEC_OBJECT_WRITE_XE; //use write flags for batch bo as default
        }

        if (exec_bo_gem)
        {
            if (exec_flags == 0)
            {
                //Add an assert message here in case of potential thread safety issue.
                //Currently, exec bo's flags could only be in (0, EXEC_OBJECT_READ_XE | EXEC_OBJECT_WRITE_XE]
                MOS_DRM_ASSERTMESSAGE("Invalid op flags(0x0) for exec bo(handle=%d)", exec_bo_gem->bo.handle);
            }

            if (exec_bo_gem->is_imported || exec_bo_gem->is_exported)
            {
                //external bo, need to export its syncobj everytime.
                int prime_fd = INVALID_HANDLE;
                ret = mos_sync_update_exec_syncs_from_handle(
                            bufmgr_gem->fd,
                            exec_bo_gem->bo.handle,
                            exec_flags,
                            syncs,
                            prime_fd);
                if (ret == MOS_XE_SUCCESS)
                {
                    /**
                     * Note, must import batch syncobj for each external bo
                     * and close the syncobj created for them after exec submission.
                    */
                    int count = syncs.size();
                    struct mos_xe_external_bo_info infos;
                    memclear(infos);
                    infos.syncobj_handle = syncs[count - 1].handle;
                    infos.prime_fd = prime_fd;
                    external_bos.push_back(infos);
                }
                else
                {
                    //Note: continue process even failed.
                    //This may only cause potential synchronization issue, DONT't crash umd here.
                    MOS_DRM_ASSERTMESSAGE("Failed to update syncobj for external bo(%d)",
                                exec_bo_gem->bo.handle);
                }
            }
            else
            {
                //internal bo
                ret = mos_sync_update_exec_syncs_from_timeline_deps(
                            curr_dummy_exec_queue_id,
                            exec_bo_gem->last_exec_write_exec_queue,
                            exec_flags,
                            exec_queue_ids,
                            exec_bo_gem->read_deps,
                            exec_bo_gem->write_deps,
                            syncs);
            }
        }
    }
    return MOS_XE_SUCCESS;
}

static int
__mos_context_exec_update_bo_deps_xe(struct mos_linux_bo **bo,
            int num_bo,
            std::vector<mos_xe_exec_bo> &exec_list,
            uint32_t curr_exec_queue_id,
            struct mos_xe_dep *dep)
{
    uint32_t exec_list_size = exec_list.size();

    for (int i = 0; i < exec_list_size + num_bo; i++)
    {
        mos_xe_bo_gem *exec_bo_gem = nullptr;
        uint32_t exec_flags = 0;
        if (i < exec_list_size)
        {
            //exec list bo
            exec_bo_gem = (mos_xe_bo_gem *)exec_list[i].bo;
            exec_flags = exec_list[i].flags;
        }
        else
        {
            //batch bo
            exec_bo_gem = (mos_xe_bo_gem *)bo[i - exec_list_size];
            exec_flags = EXEC_OBJECT_WRITE_XE; //use write flags for batch bo as default.
        }
        if (exec_bo_gem)
        {
            mos_sync_update_bo_deps(curr_exec_queue_id, exec_flags, dep, exec_bo_gem->read_deps, exec_bo_gem->write_deps);
            if (exec_flags & EXEC_OBJECT_READ_XE)
            {
                exec_bo_gem->last_exec_read_exec_queue = curr_exec_queue_id;
            }
            if (exec_flags & EXEC_OBJECT_WRITE_XE)
            {
                exec_bo_gem->last_exec_write_exec_queue = curr_exec_queue_id;
            }
        }
    }

    return MOS_XE_SUCCESS;
}

/**
 * @ctx indicates to guity ctx that needs to recover for re-submission
 * @exec indicates to exec data in previous failed submission to re-submit
 * @curr_exec_queue_id indicates to guilty exec_queue_id, it will be replaced by newly creating one
 */
static int
__mos_bo_context_exec_retry_xe(struct mos_bufmgr *bufmgr,
            struct mos_linux_context *ctx,
            struct drm_xe_exec &exec,
            uint32_t &curr_exec_queue_id)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, -EINVAL);
    MOS_DRM_CHK_NULL_RETURN_VALUE(ctx, -EINVAL);

    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    int ret = MOS_XE_SUCCESS;

    //query ctx property firstly to check if failure is caused by exec_queue ban
    uint64_t property_value = 0;
    ret = __mos_get_context_property_xe(bufmgr, ctx, DRM_XE_EXEC_QUEUE_GET_PROPERTY_BAN, property_value);

    /**
     * if exec_queue is banned, queried value is 1, otherwise it is zero;
     * if exec failure is not caused by exec_queue ban, umd could not help recover it.
     */
    if (ret || !property_value)
    {
        MOS_DRM_ASSERTMESSAGE("Failed to retore ctx(%d) with error(%d)",
                    curr_exec_queue_id, -EPERM);
        return -EPERM;
    }

    ret = __mos_context_restore_xe(bufmgr, ctx);

    if (ret == MOS_XE_SUCCESS)
    {
        curr_exec_queue_id = ctx->ctx_id;
        exec.exec_queue_id = curr_exec_queue_id;
        //try once again to submit
        ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_EXEC, &exec);
        if (ret)
        {
            MOS_DRM_ASSERTMESSAGE("Failed to re-submission in DRM_IOCTL_XE_EXEC(errno:%d): new exec_queue_id = %d",
                        ret, curr_exec_queue_id);
        }
    }
    else
    {
        MOS_DRM_ASSERTMESSAGE("Failed to retore context with error(%d), exec_queue_id = %d",
                    ret, curr_exec_queue_id);
    }
    return ret;
}

/**
 * @bo contains batch bo only.
 * @num_bo indicates to batch bo num.
 * @ctx indicates to the exec exec_queue.

 *GPU<->GPU synchronization:
 * Exec must ensure the synchronization between GPU->GPU with bellow 8 steps:
 * 1. Get the deps from read_deps and write_deps by checking bo's op flags and add it into syncs array;
 *     a) if flags & READ: get write_deps[last_write_exec_queue != ctx->dummy_exec_queue_id] & STATUS_DEP_BUSY only;
 *     b) if flags & WRITE: get read_deps[all_exec_queue exclude ctx->dummy_exec_queue_id] & STATUS_DEP_BUSY
 *        and write_deps[last_write_exec_queue != ctx->dummy_exec_queue_id] & STATUS_DEP_BUSY;
 *  2. Export a syncobj from external bo as dep and add it indo syncs array.
 *  3. Initial a new timeline dep object for exec queue if it doesn't have and add it to syncs array, otherwise add timeline
 *     dep from context->timeline_dep directly while it has latest avaiable timeline point in it;
 *  4. Exec submittion with batches and syncs.
 *  5. Update read_deps[ctx->dummy_exec_queue_id] and write_deps[ctx->dummy_exec_queue_id] with the new deps from the dep_queue;
 *  6. Update timeline dep's timeline index to be latest avaiable one for currect exec queue.
 *  7. Import syncobj from batch bo for each external bo's DMA buffer for external process to wait media process on demand.
 *  8. Close syncobj handle and syncobj fd for external bo to avoid leak.
 * GPU->CPU(optional):
 *     If bo->map_deps.dep exist:
 *         get it and add it to exec syncs array
 */
static int
mos_bo_context_exec_with_sync_xe(struct mos_linux_bo **bo, int num_bo, struct mos_linux_context *ctx,
                               struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                               unsigned int flags, int *fence)
{

    MOS_DRM_CHK_NULL_RETURN_VALUE(bo, -EINVAL)
    MOS_DRM_CHK_NULL_RETURN_VALUE(ctx, -EINVAL)
    if (num_bo <= 0)
    {
        MOS_DRM_ASSERTMESSAGE("invalid batch bo num(%d)", num_bo);
        return -EINVAL;
    }

    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo[0]->bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL)

    uint64_t batch_addrs[num_bo];

    std::vector<mos_xe_exec_bo> exec_list;
    for (int i = 0; i < num_bo; i++)
    {
        MOS_DRM_CHK_NULL_RETURN_VALUE(bo[i], -EINVAL)
        batch_addrs[i] = bo[i]->offset64;
        struct mos_xe_bo_gem *batch_bo_gem = (struct mos_xe_bo_gem *) bo[i];
        MOS_XE_GET_VALUES_FROM_MAP(batch_bo_gem->exec_list, exec_list);
    }

    struct mos_xe_context *context = (struct mos_xe_context *) ctx;
    uint32_t curr_exec_queue_id = context->ctx.ctx_id;
    std::vector<struct mos_xe_external_bo_info> external_bos;
    std::vector<struct drm_xe_sync> syncs;
    uint64_t curr_timeline = 0;
    int ret = 0;

    uint32_t exec_list_size = exec_list.size();
    if (exec_list_size == 0)
    {
        MOS_DRM_NORMALMESSAGE("invalid exec list count(%d)", exec_list_size);
    }

    bufmgr_gem->m_lock.lock();

    if (context->timeline_dep == nullptr)
    {
        context->timeline_dep = mos_sync_create_timeline_dep(bufmgr_gem->fd);

        if (context->timeline_dep == nullptr)
        {
            MOS_DRM_ASSERTMESSAGE("Failed to initial context timeline dep");
            bufmgr_gem->m_lock.unlock();
            return -ENOMEM;
        }
    }

    struct mos_xe_dep *dep = context->timeline_dep;
    //add latest avaiable timeline point(dep) into syncs as fence out point.
    mos_sync_update_exec_syncs_from_timeline_dep(
                          bufmgr_gem->fd,
                          dep,
                          syncs);

    bufmgr_gem->sync_obj_rw_lock.lock_shared();
    //update exec syncs array by external and interbal bo dep
    __mos_context_exec_update_syncs_xe(
                bufmgr_gem,
                bo,
                num_bo,
                context,
                exec_list,
                syncs,
                external_bos);

    //exec submit
    uint32_t sync_count = syncs.size();
    struct drm_xe_sync *syncs_array = syncs.data();

    //dump bo deps map
    __mos_dump_bo_deps_map_xe(bo, num_bo, exec_list, curr_exec_queue_id, bufmgr_gem->global_ctx_info);
    //dump fence in and fence out info
    __mos_dump_syncs_array_xe(syncs_array, sync_count, dep);

    struct drm_xe_exec exec;
    memclear(exec);
    exec.extensions = 0;
    exec.exec_queue_id = curr_exec_queue_id;
    exec.num_syncs = sync_count;
    exec.syncs = (uintptr_t)syncs_array;
    /**
     * exec.address only accepts batch->offset64 when num bo == 1;
     * and it only accepts batch array when num bo > 1
    */
    exec.address = (num_bo == 1 ? (uintptr_t)batch_addrs[0] : (uintptr_t)batch_addrs);
    exec.num_batch_buffer = num_bo;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_EXEC, &exec);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("Failed to submission in DRM_IOCTL_XE_EXEC(errno:%d): exec_queue_id = %d, num_syncs = %d, num_bo = %d",
                    -errno, curr_exec_queue_id, sync_count, num_bo);

        //check if it caused by guilty exec_queue_id, if so, could restore the exec_queue_id/ queue here and re-try exec again.
        if (ret == -EPERM)
        {
            ret = __mos_bo_context_exec_retry_xe(&bufmgr_gem->bufmgr, ctx, exec, curr_exec_queue_id);
        }
    }
    curr_timeline = dep->timeline_index;

    //update bos' read and write dep with new timeline
    __mos_context_exec_update_bo_deps_xe(bo, num_bo, exec_list, context->dummy_exec_queue_id, dep);

    //Update dep with latest available timeline
    mos_sync_update_timeline_dep(dep);

    bufmgr_gem->sync_obj_rw_lock.unlock_shared();
    bufmgr_gem->m_lock.unlock();

    //import batch syncobj or its point for external bos and close syncobj created for external bo before.
    uint32_t external_bo_count = external_bos.size();
    int sync_file_fd = INVALID_HANDLE;
    int temp_syncobj = INVALID_HANDLE;

    if (external_bo_count > 0)
    {
        temp_syncobj = mos_sync_syncobj_create(bufmgr_gem->fd, 0);
        if (temp_syncobj > 0)
        {
            mos_sync_syncobj_timeline_to_binary(bufmgr_gem->fd, temp_syncobj, dep->syncobj_handle, curr_timeline, 0);
            sync_file_fd = mos_sync_syncobj_handle_to_syncfile_fd(bufmgr_gem->fd, temp_syncobj);
        }
    }
    for (int i = 0; i < external_bo_count; i++)
    {
        //import syncobj for external bos
        if (sync_file_fd >= 0)
        {
            mos_sync_import_syncfile_to_external_bo(bufmgr_gem->fd, external_bos[i].prime_fd, sync_file_fd);
        }
        if (external_bos[i].prime_fd != INVALID_HANDLE)
        {
            close(external_bos[i].prime_fd);
        }
        mos_sync_syncobj_destroy(bufmgr_gem->fd, external_bos[i].syncobj_handle);
    }
    if (sync_file_fd >= 0)
    {
        close(sync_file_fd);
    }
    if (temp_syncobj > 0)
    {
        mos_sync_syncobj_destroy(bufmgr_gem->fd, temp_syncobj);
    }

    //Note: keep exec return value for final return value.
    return ret;
}

/**
 * Get the DEVICE ID for the device.  This can be overridden by setting the
 * INTEL_DEVID_OVERRIDE environment variable to the desired ID.
 */
static int
mos_get_devid_xe(struct mos_bufmgr *bufmgr)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    int fd = bufmgr_gem->fd;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;

    MOS_DRM_CHK_XE_DEV(dev, config, __mos_query_config_xe, 0)
    struct drm_xe_query_config *config = dev->config;

    return (config->info[DRM_XE_QUERY_CONFIG_REV_AND_DEVICE_ID] & 0xffff);
}

static struct drm_xe_query_engines *
__mos_query_engines_xe(int fd)
{
    if (fd < 0)
    {
        return nullptr;
    }

    struct drm_xe_device_query query;
    struct drm_xe_query_engines *engines;
    int ret;

    memclear(query);
    query.extensions = 0;
    query.query = DRM_XE_DEVICE_QUERY_ENGINES;
    query.size = 0;
    query.data = 0;

    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query);
    if (ret || !query.size)
    {
        MOS_DRM_ASSERTMESSAGE("ret:%d, length:%d", ret, query.size);
        return nullptr;
    }

    engines = (drm_xe_query_engines *)calloc(1, query.size);
    MOS_DRM_CHK_NULL_RETURN_VALUE(engines, nullptr)

    query.data = (uintptr_t)engines;
    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query);
    if (ret || !query.size)
    {
        MOS_DRM_ASSERTMESSAGE("ret:%d, length:%d", ret, query.size);
        MOS_XE_SAFE_FREE(engines);
        return nullptr;
    }

    return engines;
}

static int
mos_query_engines_count_xe(struct mos_bufmgr *bufmgr, unsigned int *nengine)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(nengine, -EINVAL);
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    int fd = bufmgr_gem->fd;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;

    MOS_DRM_CHK_XE_DEV(dev, engines, __mos_query_engines_xe, -ENODEV)
    *nengine = dev->engines->num_engines;

    return MOS_XE_SUCCESS;
}

int
mos_query_engines_xe(struct mos_bufmgr *bufmgr,
                      __u16 engine_class,
                      __u64 caps,
                      unsigned int *nengine,
                      void *engine_map)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(nengine, -EINVAL);
    MOS_DRM_CHK_NULL_RETURN_VALUE(engine_map, -EINVAL);

    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct drm_xe_engine_class_instance *ci = (struct drm_xe_engine_class_instance *)engine_map;
    int fd = bufmgr_gem->fd;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;

    MOS_DRM_CHK_XE_DEV(dev, engines, __mos_query_engines_xe, -ENODEV)
    struct drm_xe_query_engines *engines = dev->engines;

    int i, num;
    struct drm_xe_engine *engine;
    for (i = 0, num = 0; i < engines->num_engines; i++)
    {
        engine = (struct drm_xe_engine *)&engines->engines[i];
        if (engine_class == engine->instance.engine_class)
        {
            ci->engine_class = engine_class;
            ci->engine_instance = engine->instance.engine_instance;
            ci->gt_id = engine->instance.gt_id;
            ci++;
            num++;
        }

        if (num > *nengine)
        {
            MOS_DRM_ASSERTMESSAGE("Number of engine instances out of range, %d,%d", num, *nengine);
            return -1;
        }
    }

    //Note30: need to confirm if engine_instance is ordered, otherwise re-order needed.

    *nengine = num;

    return 0;
}

static size_t
mos_get_engine_class_size_xe()
{
    return sizeof(struct drm_xe_engine_class_instance);
}

static int
mos_query_sysinfo_xe(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, -EINVAL);
    MOS_DRM_CHK_NULL_RETURN_VALUE(gfx_info, -EINVAL);

    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    int fd = bufmgr_gem->fd;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;
    int ret;

    MOS_DRM_CHK_XE_DEV(dev, engines, __mos_query_engines_xe, -ENODEV)

    if (0 == gfx_info->VDBoxInfo.NumberOfVDBoxEnabled
                || 0 == gfx_info->VEBoxInfo.NumberOfVEBoxEnabled)
    {
        unsigned int num_vd = 0;
        unsigned int num_ve = 0;

        for (unsigned int i = 0; i < dev->engines->num_engines; i++)
        {
            if (0 == gfx_info->VDBoxInfo.NumberOfVDBoxEnabled
                        && dev->engines->engines[i].instance.engine_class == DRM_XE_ENGINE_CLASS_VIDEO_DECODE)
            {
                gfx_info->VDBoxInfo.Instances.VDBoxEnableMask |=
                    1 << dev->engines->engines[i].instance.engine_instance;
                num_vd++;
            }

            if (0 == gfx_info->VEBoxInfo.NumberOfVEBoxEnabled
                        && dev->engines->engines[i].instance.engine_class == DRM_XE_ENGINE_CLASS_VIDEO_ENHANCE)
            {
                num_ve++;
            }
        }

        if (num_vd > 0)
        {
            gfx_info->VDBoxInfo.NumberOfVDBoxEnabled = num_vd;
        }

        if (num_vd > 0)
        {
            gfx_info->VEBoxInfo.NumberOfVEBoxEnabled = num_ve;
        }
    }

    return 0;
}

void mos_select_fixed_engine_xe(struct mos_bufmgr *bufmgr,
            void *engine_map,
            uint32_t *nengine,
            uint32_t fixed_instance_mask)
{
    MOS_UNUSED(bufmgr);
#if (DEBUG || _RELEASE_INTERNAL)
    if (fixed_instance_mask)
    {
        struct drm_xe_engine_class_instance *_engine_map = (struct drm_xe_engine_class_instance *)engine_map;
        auto unselect_index = 0;
        for (auto bit = 0; bit < *nengine; bit++)
        {
            if (((fixed_instance_mask >> bit) & 0x1) && (bit > unselect_index))
            {
                _engine_map[unselect_index].engine_class = _engine_map[bit].engine_class;
                _engine_map[unselect_index].engine_instance = _engine_map[bit].engine_instance;
                _engine_map[unselect_index].gt_id = _engine_map[bit].gt_id;
                _engine_map[unselect_index].pad = _engine_map[bit].pad;
                _engine_map[bit].engine_class = 0;
                _engine_map[bit].engine_instance = 0;
                _engine_map[bit].gt_id = 0;
                _engine_map[bit].pad = 0;
                unselect_index++;
            }
            else if (((fixed_instance_mask >> bit) & 0x1) && (bit == unselect_index))
            {
                unselect_index++;
            }
            else if (!((fixed_instance_mask >> bit) & 0x1))
            {
                _engine_map[bit].engine_class = 0;
                _engine_map[bit].engine_instance = 0;
                _engine_map[bit].gt_id = 0;
                _engine_map[bit].pad = 0;
            }
        }
        *nengine = unselect_index;
    }
#else
    MOS_UNUSED(engine_map);
    MOS_UNUSED(nengine);
    MOS_UNUSED(fixed_instance_mask);
#endif

}


/**
 * Note: xe kmd doesn't support query blob before dg2.
 */
static uint32_t *
__mos_query_hw_config_xe(int fd)
{
    struct drm_xe_device_query query;
    uint32_t *hw_config;
    int ret;

    if (fd < 0)
    {
        return nullptr;
    }

    memclear(query);
    query.query = DRM_XE_DEVICE_QUERY_HWCONFIG;

    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query);
    if (ret || !query.size)
    {
        MOS_DRM_ASSERTMESSAGE("ret:%d, length:%d", ret, query.size);
        return nullptr;
    }

    hw_config = (uint32_t *)calloc(1, query.size + sizeof(uint32_t));
    MOS_DRM_CHK_NULL_RETURN_VALUE(hw_config, nullptr)

    query.data = (uintptr_t)&hw_config[1];
    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query);
    if (ret != 0 || query.size <= 0)
    {
        MOS_DRM_ASSERTMESSAGE("ret:%d, length:%d", ret, query.size);
        MOS_XE_SAFE_FREE(hw_config);
        return nullptr;
    }

    hw_config[0] = query.size / sizeof(uint32_t);

    return hw_config;
}

static int
mos_query_device_blob_xe(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(gfx_info, -EINVAL)

    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    int fd = bufmgr_gem->fd;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;

    MOS_DRM_CHK_XE_DEV(dev, hw_config, __mos_query_hw_config_xe, -ENODEV)

    uint32_t *hwconfig = &dev->hw_config[1];
    uint32_t num_config = dev->hw_config[0];

    int i = 0;
    while (i < num_config) {
        /* Attribute ID starts with 1 */
        assert(hwconfig[i] > 0);

    #if DEBUG_BLOB_QUERY
        MOS_DRM_NORMALMESSAGE("query blob: key=%s, value=%d", key_string[hwconfig[i]], hwconfig[i+2]);
    #endif
        if (INTEL_HWCONFIG_MAX_SLICES_SUPPORTED == hwconfig[i])
        {
            assert(hwconfig[i+1] == 1);
            gfx_info->SliceCount = hwconfig[i+2];
            gfx_info->MaxSlicesSupported = hwconfig[i+2];
        }

        if ((INTEL_HWCONFIG_MAX_DUAL_SUBSLICES_SUPPORTED == hwconfig[i])
            || (INTEL_HWCONFIG_MAX_SUBSLICE == hwconfig[i]))
        {
            assert(hwconfig[i+1] == 1);
            gfx_info->SubSliceCount = hwconfig[i+2];
            gfx_info->MaxSubSlicesSupported = hwconfig[i+2];
        }

        if ((INTEL_HWCONFIG_MAX_NUM_EU_PER_DSS == hwconfig[i])
            || (INTEL_HWCONFIG_MAX_EU_PER_SUBSLICE == hwconfig[i]))
        {
            assert(hwconfig[i+1] == 1);
            gfx_info->MaxEuPerSubSlice = hwconfig[i+2];
        }

        if (INTEL_HWCONFIG_DEPRECATED_L3_CACHE_SIZE_IN_KB == hwconfig[i])
        {
            assert(hwconfig[i+1] == 1);
            gfx_info->L3CacheSizeInKb = hwconfig[i+2];
        }

        if (INTEL_HWCONFIG_NUM_THREADS_PER_EU == hwconfig[i])
        {
            assert(hwconfig[i+1] == 1);
            gfx_info->NumThreadsPerEu = hwconfig[i+2];
        }

        if (INTEL_HWCONFIG_MAX_VECS == hwconfig[i])
        {
            assert(hwconfig[i+1] == 1);
            gfx_info->MaxVECS = hwconfig[i+2];
        }

        /* Advance to next key */
        i += hwconfig[i + 1];  // value size
        i += 2;// KL size
    }

    return 0;
}

static void
mos_enable_reuse_xe(struct mos_bufmgr *bufmgr)
{
    MOS_UNIMPLEMENT(bufmgr);
}

// The function is not supported on KMD
static int mos_query_hw_ip_version_xe(struct mos_bufmgr *bufmgr, __u16 engine_class, void *ip_ver_info)
{
    MOS_UNIMPLEMENT(bufmgr);
    MOS_UNIMPLEMENT(engine_class);
    MOS_UNIMPLEMENT(ip_ver_info);
    return 0;
}

static void
mos_bo_free_xe(struct mos_linux_bo *bo)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = nullptr;
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *) bo;
    struct drm_gem_close close_ioctl;
    int ret;

    if (nullptr == bo_gem)
    {
        MOS_DRM_ASSERTMESSAGE("bo == nullptr");
        return;
    }

    bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo->bufmgr;

    if (nullptr == bufmgr_gem)
    {
        MOS_DRM_ASSERTMESSAGE("bufmgr_gem == nullptr");
        return;
    }

    mos_gem_bo_wait_rendering_xe(bo);

    bufmgr_gem->m_lock.lock();

    if (!bo_gem->is_userptr)
    {
        if (bo_gem->mem_virtual)
        {
            VG(VALGRIND_MAKE_MEM_NOACCESS(bo_gem->mem_virtual, 0));
            drm_munmap(bo_gem->mem_virtual, bo_gem->bo.size);
            bo_gem->mem_virtual = nullptr;
        }
    }

    if (bo->vm_id != INVALID_VM)
    {
        ret = mos_vm_bind_sync_xe(bufmgr_gem->fd,
                    bo->vm_id,
                    0,
                    0,
                    bo->offset64,
                    bo->size,
                    bo_gem->pat_index,
                    DRM_XE_VM_BIND_OP_UNMAP);
        if (ret)
        {
            MOS_DRM_ASSERTMESSAGE("mos_gem_bo_free mos_vm_unbind ret error. bo:0x%lx, vm_id:%d\r",
                    (uint64_t)bo,
                    bo->vm_id);
        }
        else
        {
            bo->vm_id = INVALID_VM;
        }
    }

    if (!bo_gem->is_userptr)
    {
        /* Close this object */
         memclear(close_ioctl);
         close_ioctl.handle = bo_gem->gem_handle;
         ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_GEM_CLOSE, &close_ioctl);
         if (ret != 0)
         {
             MOS_DRM_ASSERTMESSAGE("DRM_IOCTL_GEM_CLOSE %d failed (%s): %s",
                 bo_gem->gem_handle, bo_gem->name, strerror(errno));
         }
    }

    if (bufmgr_gem->mem_profiler_fd != -1)
    {
        snprintf(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE, "GEM_CLOSE, %d, %d, %lu, %d\n", getpid(), bo->handle,bo->size,bo_gem->mem_region);
        ret = write(bufmgr_gem->mem_profiler_fd, bufmgr_gem->mem_profiler_buffer, strnlen(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE));
        if (-1 == ret)
        {
            snprintf(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE, "GEM_CLOSE, %d, %d, %lu, %d\n", getpid(), bo->handle,bo->size,bo_gem->mem_region);
            ret = write(bufmgr_gem->mem_profiler_fd, bufmgr_gem->mem_profiler_buffer, strnlen(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE));
            if (-1 == ret)
            {
                MOS_DRM_ASSERTMESSAGE("Failed to write to %s: %s", bufmgr_gem->mem_profiler_path, strerror(errno));
            }
        }
    }

    /* Return the VMA for reuse */
    __mos_bo_vma_free_xe(bo->bufmgr, bo->offset64, bo->size);
    bufmgr_gem->m_lock.unlock();

    MOS_Delete(bo_gem);
}

static int
mos_bo_set_softpin_xe(MOS_LINUX_BO *bo)
{
    MOS_UNIMPLEMENT(bo);
    return 0;
}

static void
mos_bufmgr_gem_destroy_xe(struct mos_bufmgr *bufmgr)
{
    if (nullptr == bufmgr)
        return;

    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bufmgr;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;
    int i, ret;

    /* Release userptr bo kept hanging around for optimisation. */

    mos_vma_heap_finish(&bufmgr_gem->vma_heap[MEMZONE_SYS]);
    mos_vma_heap_finish(&bufmgr_gem->vma_heap[MEMZONE_DEVICE]);
    mos_vma_heap_finish(&bufmgr_gem->vma_heap[MEMZONE_PRIME]);

    if (bufmgr_gem->vm_id != INVALID_VM)
    {
        __mos_vm_destroy_xe(bufmgr, bufmgr_gem->vm_id);
        bufmgr_gem->vm_id = INVALID_VM;
    }

    if (bufmgr_gem->mem_profiler_fd != -1)
    {
        close(bufmgr_gem->mem_profiler_fd);
    }

    MOS_XE_SAFE_FREE(dev->hw_config);
    dev->hw_config = nullptr;

    MOS_XE_SAFE_FREE(dev->config);
    dev->config = nullptr;

    MOS_XE_SAFE_FREE(dev->engines);
    dev->engines = nullptr;

    MOS_XE_SAFE_FREE(dev->mem_regions);
    dev->mem_regions = nullptr;

    MOS_XE_SAFE_FREE(dev->gt_list);
    dev->gt_list = nullptr;

    MOS_Delete(bufmgr_gem);
}

static void
mos_bufmgr_gem_unref_xe(struct mos_bufmgr *bufmgr)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;

    if (bufmgr_gem && atomic_add_unless(&bufmgr_gem->ref_count, -1, 1))
    {
        pthread_mutex_lock(&bufmgr_list_mutex);

        if (atomic_dec_and_test(&bufmgr_gem->ref_count))
        {
            DRMLISTDEL(&bufmgr_gem->managers);
            mos_bufmgr_gem_destroy_xe(bufmgr);
        }

        pthread_mutex_unlock(&bufmgr_list_mutex);
    }
}

static int
mo_get_context_param_xe(struct mos_linux_context *ctx,
                uint32_t size,
                uint64_t param,
                uint64_t *value)
{
    MOS_UNIMPLEMENT(ctx);
    MOS_UNIMPLEMENT(size);
    MOS_UNIMPLEMENT(param);
    MOS_UNIMPLEMENT(value);
    return 0;
}

static void mos_enable_softpin_xe(struct mos_bufmgr *bufmgr, bool va1m_align)
{
    MOS_UNIMPLEMENT(bufmgr);
    MOS_UNIMPLEMENT(va1m_align);
}

static int
mos_get_reset_stats_xe(struct mos_linux_context *ctx,
              uint32_t *reset_count,
              uint32_t *active,
              uint32_t *pending)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(ctx, -EINVAL);

    struct mos_xe_context *context = (struct mos_xe_context *)ctx;
    if (reset_count)
        *reset_count = context->reset_count;
    if (active)
        *active = 0;
    if (pending)
        *pending = 0;
    return 0;
}

static mos_oca_exec_list_info*
mos_bo_get_oca_exec_list_info_xe(struct mos_linux_bo *bo, int *count)
{
    if (nullptr == bo  || nullptr == count)
    {
        return nullptr;
    }

    mos_oca_exec_list_info *info = nullptr;
    int counter = 0;
    int MAX_COUNT = 50;
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo->bufmgr;
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *)bo;
    int exec_list_count = bo_gem->exec_list.size();

    if (exec_list_count == 0 || exec_list_count > MAX_COUNT)
    {
        return nullptr;
    }

    info = (mos_oca_exec_list_info *)malloc((exec_list_count + 1) * sizeof(mos_oca_exec_list_info));
    if (!info)
    {
        MOS_DRM_ASSERTMESSAGE("malloc mos_oca_exec_list_info failed");
        return info;
    }

    for (auto &it : bo_gem->exec_list)
    {
        /*note: set capture for each bo*/
        struct mos_xe_bo_gem *exec_bo_gem = (struct mos_xe_bo_gem *)it.second.bo;
        uint32_t exec_flags = it.second.flags;
        if (exec_bo_gem)
        {
            info[counter].handle   = exec_bo_gem->bo.handle;
            info[counter].size     = exec_bo_gem->bo.size;
            info[counter].offset64 = exec_bo_gem->bo.offset64;
            info[counter].flags    = exec_flags;
            info[counter].mem_region = exec_bo_gem->mem_region;
            info[counter].is_batch = false;
            counter++;
        }
    }

    /*note: bo is cmd bo, also need to be added*/
    info[counter].handle   = bo->handle;
    info[counter].size     = bo->size;
    info[counter].offset64 = bo->offset64;
    info[counter].flags    = EXEC_OBJECT_WRITE_XE; // use write flags for batch bo as default.
    info[counter].mem_region = bo_gem->mem_region;
    info[counter].is_batch = true;
    counter++;

    *count = counter;

    return info;
}

static bool
mos_has_bsd2_xe(struct mos_bufmgr *bufmgr)
{
    MOS_UNUSED(bufmgr);
    return true;
}

static void
mos_bo_set_object_capture_xe(struct mos_linux_bo *bo)
{
    MOS_UNIMPLEMENT(bo);
}

static void
mos_bo_set_object_async_xe(struct mos_linux_bo *bo)
{
    MOS_UNIMPLEMENT(bo);
}

static int
mos_get_driver_info_xe(struct mos_bufmgr *bufmgr, struct LinuxDriverInfo *drvInfo)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(drvInfo, -EINVAL)
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct mos_xe_device *dev = &bufmgr_gem->xe_device;
    int fd = bufmgr_gem->fd;

    uint32_t MaxEuPerSubSlice = 0;
    int i = 0;
    drvInfo->hasBsd = 1;
    drvInfo->hasBsd2 = 1;
    drvInfo->hasVebox = 1;

    //For XE driver always has ppgtt
    drvInfo->hasPpgtt = 1;

    /**
     * query blob
     * Note: xe kmd doesn't support query blob before dg2, so don't check null and return here.
     */
    if (dev->hw_config == nullptr)
    {
        dev->hw_config = __mos_query_hw_config_xe(fd);
    }

    if (dev->hw_config)
    {
        uint32_t *hw_config = &dev->hw_config[1];
        uint32_t num_config = dev->hw_config[0];

        while (i < num_config)
        {
            /* Attribute ID starts with 1 */
            assert(hw_config[i] > 0);

#if DEBUG_BLOB_QUERY
            MOS_DRM_NORMALMESSAGE("query blob: key=%s, value=%d", key_string[hw_config[i]], hw_config[i+2]);
#endif
            if (INTEL_HWCONFIG_MAX_SLICES_SUPPORTED == hw_config[i])
            {
                assert(hw_config[i+1] == 1);
                drvInfo->sliceCount = hw_config[i+2];
            }

            if ((INTEL_HWCONFIG_MAX_DUAL_SUBSLICES_SUPPORTED == hw_config[i])
                || (INTEL_HWCONFIG_MAX_SUBSLICE == hw_config[i]))
            {
                assert(hw_config[i+1] == 1);
                drvInfo->subSliceCount = hw_config[i+2];
            }

            if ((INTEL_HWCONFIG_MAX_NUM_EU_PER_DSS == hw_config[i])
                || (INTEL_HWCONFIG_MAX_EU_PER_SUBSLICE == hw_config[i]))
            {
                assert(hw_config[i+1] == 1);
                MaxEuPerSubSlice = hw_config[i+2];
            }

            /* Advance to next key */
            i += hw_config[i + 1];  // value size
            i += 2;// KL size
        }

        drvInfo->euCount = drvInfo->subSliceCount * MaxEuPerSubSlice;
    }
    else
    {
        drvInfo->euCount = 96;
        drvInfo->subSliceCount = 6;
        drvInfo->sliceCount = 1;
    }

    // query engines info
    MOS_DRM_CHK_XE_DEV(dev, engines, __mos_query_engines_xe, -ENODEV)
    struct drm_xe_query_engines *engines = dev->engines;
    int num_vd = 0;
    int num_ve = 0;
    for (i = 0; i < engines->num_engines; i++)
    {
        if (DRM_XE_ENGINE_CLASS_VIDEO_DECODE == engines->engines[i].instance.engine_class)
        {
            num_vd++;
        }
        else if (DRM_XE_ENGINE_CLASS_VIDEO_ENHANCE == engines->engines[i].instance.engine_class)
        {
            num_ve++;
        }
    }

    if (num_vd >= 1)
    {
        drvInfo->hasBsd = 1;
    }

    if (num_vd >= 2)
    {
        drvInfo->hasBsd2 = 1;
    }

    if (num_ve  >= 1)
    {
        drvInfo->hasVebox = 1;
    }

    drvInfo->hasHuc = 1;
    if (1 == drvInfo->hasHuc)
    {
        drvInfo->hasProtectedHuc = 1;
    }

    // query config
    MOS_DRM_CHK_XE_DEV(dev, config, __mos_query_config_xe, -ENODEV)
    struct drm_xe_query_config *config = dev->config;
    drvInfo->devId = config->info[DRM_XE_QUERY_CONFIG_REV_AND_DEVICE_ID] & 0xffff;
    drvInfo->devRev = config->info[DRM_XE_QUERY_CONFIG_REV_AND_DEVICE_ID] >> 16;

    return MOS_XE_SUCCESS;
}

/**
 * Initializes the GEM buffer manager, which uses the kernel to allocate, map,
 * and manage map buffer objections.
 *
 * \param fd File descriptor of the opened DRM device.
 */
struct mos_bufmgr *
mos_bufmgr_gem_init_xe(int fd, int batch_size)
{
    //Note: don't put this field in bufmgr in case of bufmgr inaccessable in some functions
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_READ_ENV_VARIABLE(INTEL_XE_BUFMGR_DEBUG, MOS_USER_FEATURE_VALUE_TYPE_INT64, __xe_bufmgr_debug__);
    if (__xe_bufmgr_debug__ < 0)
    {
        __xe_bufmgr_debug__ = 0;
    }
#endif

    struct mos_xe_bufmgr_gem *bufmgr_gem;
    int ret, tmp;
    struct mos_xe_device *dev = nullptr;

    pthread_mutex_lock(&bufmgr_list_mutex);

    bufmgr_gem = mos_bufmgr_gem_find(fd);
    if (bufmgr_gem)
        goto exit;

    bufmgr_gem = MOS_New(mos_xe_bufmgr_gem);
    if (nullptr == bufmgr_gem)
        goto exit;

    bufmgr_gem->bufmgr = {};
    bufmgr_gem->xe_device = {};
    dev = &bufmgr_gem->xe_device;

    bufmgr_gem->fd = fd;
    bufmgr_gem->vm_id = INVALID_VM;
    atomic_set(&bufmgr_gem->ref_count, 1);

    bufmgr_gem->bufmgr.vm_create = mos_vm_create_xe;
    bufmgr_gem->bufmgr.vm_destroy = mos_vm_destroy_xe;
    bufmgr_gem->bufmgr.context_create = mos_context_create_xe;
    bufmgr_gem->bufmgr.context_create_ext = mos_context_create_ext_xe;
    bufmgr_gem->bufmgr.context_create_shared = mos_context_create_shared_xe;
    bufmgr_gem->bufmgr.context_destroy = mos_context_destroy_xe;
    bufmgr_gem->bufmgr.bo_alloc = mos_bo_alloc_xe;
    bufmgr_gem->bufmgr.bo_add_softpin_target = mos_gem_bo_update_exec_list_xe;
    bufmgr_gem->bufmgr.bo_clear_relocs = mos_gem_bo_clear_exec_list_xe;
    bufmgr_gem->bufmgr.bo_alloc_userptr = mos_bo_alloc_userptr_xe;
    bufmgr_gem->bufmgr.bo_alloc_tiled = mos_bo_alloc_tiled_xe;
    bufmgr_gem->bufmgr.bo_map = mos_bo_map_xe;
    bufmgr_gem->bufmgr.bo_busy = mos_gem_bo_busy_xe;
    bufmgr_gem->bufmgr.bo_wait_rendering = mos_gem_bo_wait_rendering_xe;
    bufmgr_gem->bufmgr.bo_wait = mos_gem_bo_wait_xe;
    bufmgr_gem->bufmgr.bo_map_wc = mos_bo_map_wc_xe;
    bufmgr_gem->bufmgr.bo_unmap = mos_bo_unmap_xe;
    bufmgr_gem->bufmgr.bo_unmap_wc = mos_bo_unmap_wc_xe;
    bufmgr_gem->bufmgr.bo_create_from_prime = mos_bo_create_from_prime_xe;
    bufmgr_gem->bufmgr.bo_export_to_prime = mos_bo_export_to_prime_xe;
    bufmgr_gem->bufmgr.get_devid = mos_get_devid_xe;
    bufmgr_gem->bufmgr.query_engines_count = mos_query_engines_count_xe;
    bufmgr_gem->bufmgr.query_engines = mos_query_engines_xe;
    bufmgr_gem->bufmgr.get_engine_class_size = mos_get_engine_class_size_xe;
    bufmgr_gem->bufmgr.query_sys_engines = mos_query_sysinfo_xe;
    bufmgr_gem->bufmgr.select_fixed_engine = mos_select_fixed_engine_xe;
    bufmgr_gem->bufmgr.query_device_blob = mos_query_device_blob_xe;
    bufmgr_gem->bufmgr.get_driver_info = mos_get_driver_info_xe;
    bufmgr_gem->bufmgr.destroy = mos_bufmgr_gem_unref_xe;
    bufmgr_gem->bufmgr.query_hw_ip_version = mos_query_hw_ip_version_xe;
    bufmgr_gem->bufmgr.get_platform_information = mos_get_platform_information_xe;
    bufmgr_gem->bufmgr.set_platform_information = mos_set_platform_information_xe;
    bufmgr_gem->bufmgr.enable_reuse = mos_enable_reuse_xe;
    bufmgr_gem->bufmgr.bo_reference = mos_bo_reference_xe;
    bufmgr_gem->bufmgr.bo_unreference = mos_bo_unreference_xe;
    bufmgr_gem->bufmgr.bo_set_softpin = mos_bo_set_softpin_xe;
    bufmgr_gem->bufmgr.enable_softpin = mos_enable_softpin_xe;
    bufmgr_gem->bufmgr.get_context_param = mo_get_context_param_xe;
    bufmgr_gem->bufmgr.get_reset_stats = mos_get_reset_stats_xe;
    bufmgr_gem->bufmgr.bo_get_softpin_targets_info = mos_bo_get_oca_exec_list_info_xe;
    bufmgr_gem->bufmgr.has_bsd2= mos_has_bsd2_xe;
    bufmgr_gem->bufmgr.set_object_capture = mos_bo_set_object_capture_xe;
    bufmgr_gem->bufmgr.set_object_async = mos_bo_set_object_async_xe;
    bufmgr_gem->bufmgr.bo_context_exec3 = mos_bo_context_exec_with_sync_xe;

    bufmgr_gem->exec_queue_timeslice = EXEC_QUEUE_TIMESLICE_DEFAULT;
    MOS_READ_ENV_VARIABLE(INTEL_ENGINE_TIMESLICE, MOS_USER_FEATURE_VALUE_TYPE_INT32, bufmgr_gem->exec_queue_timeslice);
    if (bufmgr_gem->exec_queue_timeslice <= 0
            || bufmgr_gem->exec_queue_timeslice >= EXEC_QUEUE_TIMESLICE_MAX)
    {
        bufmgr_gem->exec_queue_timeslice = EXEC_QUEUE_TIMESLICE_DEFAULT;
    }

    bufmgr_gem->mem_profiler_fd = -1;
    bufmgr_gem->mem_profiler_path = getenv("MEDIA_MEMORY_PROFILER_LOG");
    if (bufmgr_gem->mem_profiler_path != nullptr)
    {
        if (strcmp(bufmgr_gem->mem_profiler_path, "/sys/kernel/debug/tracing/trace_marker") == 0)
        {
            ret = bufmgr_gem->mem_profiler_fd = open(bufmgr_gem->mem_profiler_path, O_WRONLY );
        }
        else
        {
            ret = bufmgr_gem->mem_profiler_fd = open(bufmgr_gem->mem_profiler_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        }

        if ( -1 == ret)
        {
            MOS_DRM_ASSERTMESSAGE("Failed to open %s: %s", bufmgr_gem->mem_profiler_path, strerror(errno));
        }
    }

    dev->uc_versions[UC_TYPE_GUC_SUBMISSION].uc_type = UC_TYPE_INVALID;
    dev->uc_versions[UC_TYPE_HUC].uc_type = UC_TYPE_INVALID;

    bufmgr_gem->vm_id = __mos_vm_create_xe(&bufmgr_gem->bufmgr);
    __mos_query_mem_regions_instance_mask_xe(&bufmgr_gem->bufmgr);
    __mos_has_vram_xe(&bufmgr_gem->bufmgr);
    __mos_get_default_alignment_xe(&bufmgr_gem->bufmgr);

    DRMLISTADD(&bufmgr_gem->managers, &bufmgr_list);
    DRMINITLISTHEAD(&bufmgr_gem->named);

    mos_vma_heap_init(&bufmgr_gem->vma_heap[MEMZONE_SYS], MEMZONE_SYS_START, MEMZONE_SYS_SIZE);
    mos_vma_heap_init(&bufmgr_gem->vma_heap[MEMZONE_DEVICE], MEMZONE_DEVICE_START, MEMZONE_DEVICE_SIZE);
    mos_vma_heap_init(&bufmgr_gem->vma_heap[MEMZONE_PRIME], MEMZONE_PRIME_START, MEMZONE_PRIME_SIZE);

exit:
    pthread_mutex_unlock(&bufmgr_list_mutex);

    return bufmgr_gem != nullptr ? &bufmgr_gem->bufmgr : nullptr;
}

int mos_get_dev_id_xe(int fd, uint32_t *device_id)
{
    if (fd < 0 || nullptr == device_id)
    {
        return -EINVAL;
    }
    struct drm_xe_query_config *config = __mos_query_config_xe(fd);
    MOS_DRM_CHK_NULL_RETURN_VALUE(config, -ENODEV)

    *device_id = config->info[DRM_XE_QUERY_CONFIG_REV_AND_DEVICE_ID] & 0xffff;
    MOS_XE_SAFE_FREE(config);

    return MOS_XE_SUCCESS;
}
