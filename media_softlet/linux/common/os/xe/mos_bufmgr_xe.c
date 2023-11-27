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
    MOS_XE_MEM_CLASS_SYSMEM = 0,     //For XE_MEM_REGION_CLASS_SYSMEM
    MOS_XE_MEM_CLASS_VRAM,           //For XE_MEM_REGION_CLASS_VRAM
    MOS_XE_MEM_CLASS_MAX
};

struct mos_xe_context {
    struct mos_linux_context ctx;
    /**
     *1.If dep_queue_free not empty, then pop() it to use as fence out with status=STATUS_DEP_BUSY set and
     *  add it into exec syncs array then push() it to dep_queue_busy after exec submission and update it into bo read and write deps.
     *2.If dep_queue_busy.size() achieves maximun size of MAX_DEPS_SIZE, pop() it to use as fence out with status=STATUS_DEP_BUSY set
     *  and reset(wait) it before adding into exec syncs array.
     *  Otherwise, create new dep with STATUS_DEP_BUSY set and add it into exec syncs array.
     *3.If one dep in the dep_queue_busy is confirmed as signaled by any caller, then umd could move all ones whose
     *  timeline_index ahead of its from busy queue to free queue with status=STATUS_DEP_FREE set.
     */
    std::queue<struct mos_xe_dep*> dep_queue_free;
    std::queue<struct mos_xe_dep*> dep_queue_busy;

    /**
     * Free dep list in which free deps have not been reseted and could not reuse directly.
     */
    std::list<struct mos_xe_dep*> free_dep_tmp_list;

    /**
     * The UMD's dummy exec_queue id for exec_queue ctx.
     */
    uint32_t dummy_exec_queue_id;

    /**
     * Indicates to current timeline index in the queue.
     */
    uint64_t cur_timeline_index;
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
    bool     has_vram;

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

    uint32_t *hw_config;
    uint32_t config_len;
    struct drm_xe_query_config *config;
    struct drm_xe_engine_class_instance *hw_engines;
    uint32_t number_hw_engines;
    struct drm_xe_query_mem_usage *mem_usage;
    struct drm_xe_query_gt_list *gt_list;
    uint32_t number_gt;
    /** bitmask of all memory regions */
    uint64_t memory_regions;
    /** @default_alignment: safe alignment regardless region location */
    uint32_t default_alignment[MOS_XE_MEM_CLASS_MAX] = {PAGE_SIZE_4K, PAGE_SIZE_4K};

    /**
     * Indicates whether gpu-gpu and cpu-gpu synchronization is disabled.
     * This is mainly for debug purpose, and synchronizarion should be always enabled by default.
     * It could be disabled by env INTEL_SYNCHRONIZATION_DISABLE.
     */
    bool is_disable_synchronization;

    // Note: this is for test_va_api case because of high cost of sync vm bind and bo creation during initialization
    bool is_defer_creation_and_binding;

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

static pthread_mutex_t bufmgr_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static drmMMListHead bufmgr_list = { &bufmgr_list, &bufmgr_list };

static struct drm_xe_query_gt_list *__mos_query_gt_list_xe(int fd);
static void mos_bo_free_xe(struct mos_linux_bo *bo);
static int mos_query_engines_count_xe(struct mos_bufmgr *bufmgr, unsigned int *nengine);
int mos_query_engines_class_xe(struct mos_bufmgr *bufmgr,
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

static uint32_t __mos_query_memory_regions_xe(int fd)
{
    int ret = 0;
    uint64_t __memory_regions = 0;
    struct drm_xe_query_gt_list *gt_list;
    struct mos_xe_bufmgr_gem *bufmgr_gem = mos_bufmgr_gem_find(fd);

    if (bufmgr_gem && bufmgr_gem->gt_list)
    {
        gt_list = bufmgr_gem->gt_list;
    }
    else
    {
        gt_list = __mos_query_gt_list_xe(fd);
    }

    if(gt_list)
    {
        for (int i = 0; i < gt_list->num_gt; i++) {
            __memory_regions |= gt_list->gt_list[i].native_mem_regions |
                gt_list->gt_list[i].slow_mem_regions;
        }

        if (!bufmgr_gem)
        {
            MOS_XE_SAFE_FREE(gt_list)
        }
        else if (bufmgr_gem && !bufmgr_gem->gt_list)
        {
            bufmgr_gem->gt_list = gt_list;
        }
    }

    if (bufmgr_gem)
    {
        atomic_dec(&bufmgr_gem->ref_count, 1);
    }

    return __memory_regions;
}


static struct drm_xe_query_mem_usage *
__mos_query_mem_usage_xe(int fd)
{
    int ret = 0;
    struct drm_xe_query_mem_usage *mem_usage;
    struct drm_xe_device_query query;
    memclear(query);
    query.query = DRM_XE_DEVICE_QUERY_MEM_USAGE;

    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY,
                &query);
    if(ret || !query.size)
    {
        return nullptr;
    }

    mem_usage = (drm_xe_query_mem_usage *)malloc(query.size);
    if (mem_usage != nullptr)
    {
        memset(mem_usage, 0, query.size);
    }
    else
    {
        MOS_DRM_ASSERTMESSAGE("malloc mem_usage failed");
        return nullptr;
    }

    query.data = (uintptr_t)(mem_usage);
    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY,
                &query);
    if (ret || !query.size || 0 == mem_usage->num_regions)
    {
        MOS_XE_SAFE_FREE(mem_usage);
        return nullptr;
    }

    return mem_usage;
}

uint8_t __mos_query_vram_region_count_xe(int fd)
{
    struct drm_xe_query_mem_usage *mem_usage;
    uint8_t num_regions = 0;
    uint8_t vram_regions = 0;
    struct mos_xe_bufmgr_gem *bufmgr_gem = mos_bufmgr_gem_find(fd);

    if (bufmgr_gem && bufmgr_gem->mem_usage)
    {
        mem_usage = bufmgr_gem->mem_usage;
    }
    else
    {
        mem_usage = __mos_query_mem_usage_xe(fd);
    }

    if(mem_usage)
    {
        num_regions = mem_usage->num_regions;
        for(int i =0; i < num_regions; i++)
        {
            if(mem_usage->regions[i].mem_class == XE_MEM_REGION_CLASS_VRAM)
            {
                vram_regions++;
            }
        }

        if (!bufmgr_gem)
        {
            MOS_XE_SAFE_FREE(mem_usage)
        }
        else if (bufmgr_gem && !bufmgr_gem->mem_usage)
        {
            bufmgr_gem->mem_usage = mem_usage;
        }
    }

    if (bufmgr_gem)
    {
        atomic_dec(&bufmgr_gem->ref_count, 1);
    }

    return vram_regions;
}

bool __mos_has_vram_xe(int fd)
{
    return __mos_query_vram_region_count_xe(fd) > 0;
}

int mos_xe_force_gt_reset(int fd, int gt_id)
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
    if(ret || !query.size)
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
    if(ret || !query.size)
    {
        return nullptr;
    }

    gt_list = (drm_xe_query_gt_list *)malloc(query.size);
    if (gt_list != nullptr)
    {
        memset(gt_list, 0, query.size);
    }
    else
    {
        MOS_DRM_ASSERTMESSAGE("malloc gt_list failed");
        return nullptr;
    }

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

static int
__mos_get_default_alignment_xe(struct mos_bufmgr *bufmgr, struct drm_xe_query_mem_usage *mem_usage)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, -EINVAL);
    MOS_DRM_CHK_NULL_RETURN_VALUE(mem_usage, -EINVAL);
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    uint16_t mem_class;

    for (int i = 0; i < mem_usage->num_regions; i++)
    {
        if (XE_MEM_REGION_CLASS_SYSMEM == mem_usage->regions[i].mem_class)
        {
            mem_class = MOS_XE_MEM_CLASS_SYSMEM;
        }
        else if (XE_MEM_REGION_CLASS_VRAM == mem_usage->regions[i].mem_class)
        {
            mem_class = MOS_XE_MEM_CLASS_VRAM;
        }
        else
        {
            MOS_DRM_ASSERTMESSAGE("Unsupported mem class");
            return -EINVAL;
        }

        if (bufmgr_gem->default_alignment[mem_class] < mem_usage->regions[i].min_page_size)
        {
            bufmgr_gem->default_alignment[mem_class] = mem_usage->regions[i].min_page_size;
        }
    }

    return 0;
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
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

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
    vm.flags = DRM_XE_VM_CREATE_ASYNC_DEFAULT;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_VM_CREATE, &vm);
    if (ret != 0) {
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
    if (ret != 0) {
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

/**
 * Set the property of the ctx
 *
 * @ctx indicates to the context that to set
 * @property indicates to what property that to set
 * @value indicates to value for given property
 */
static int
__mos_set_context_property(struct mos_bufmgr *bufmgr,
            struct mos_linux_context *ctx,
            uint32_t property,
            uint64_t value)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, -EINVAL);
    MOS_DRM_CHK_NULL_RETURN_VALUE(ctx, -EINVAL);
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct drm_xe_exec_queue_set_property p;
    memclear(p);
    p.property = property;
    p.exec_queue_id = ctx->ctx_id;
    p.value = value;

    int ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_EXEC_QUEUE_SET_PROPERTY, &p);

    return ret;
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
    if((engine_class == DRM_XE_ENGINE_CLASS_RENDER
                || engine_class == DRM_XE_ENGINE_CLASS_COMPUTE)
                && (ctx_width * num_placements == 1)
                && bufmgr_gem->exec_queue_timeslice != EXEC_QUEUE_TIMESLICE_DEFAULT)
    {
        struct drm_xe_ext_set_property timeslice;
        memclear(timeslice);
        timeslice.property = XE_EXEC_QUEUE_SET_PROPERTY_TIMESLICE;
        /**
         * Note, this value indicates to maximum of time slice for WL instead of real waiting time.
         */
        timeslice.value = bufmgr_gem->exec_queue_timeslice;
        timeslice.base.name = XE_EXEC_QUEUE_EXTENSION_SET_PROPERTY;
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
    context->cur_timeline_index = 0;
    context->ctx.bufmgr = bufmgr;
    context->ctx.vm_id = bufmgr_gem->vm_id;
    context->reset_count = 0;

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
    context->cur_timeline_index = 0;
    context->ctx.bufmgr = bufmgr;
    context->ctx.vm_id = bufmgr_gem->vm_id;
    context->reset_count = 0;
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
    if(nullptr == bufmgr_gem)
    {
        return;
    }
    struct mos_xe_context *context = (struct mos_xe_context *)ctx;
    struct drm_xe_exec_queue_destroy exec_queue_destroy;
    int ret;
    bufmgr_gem->m_lock.lock();
    bufmgr_gem->sync_obj_rw_lock.lock();
    mos_sync_clear_dep_queue(bufmgr_gem->fd, context->dep_queue_busy);
    mos_sync_clear_dep_queue(bufmgr_gem->fd, context->dep_queue_free);
    mos_sync_clear_dep_list(bufmgr_gem->fd, context->free_dep_tmp_list);
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
    if(ctx->ctx_id == INVALID_EXEC_QUEUE_ID)
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
    ret = mos_query_engines_class_xe(bufmgr,
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
__mos_get_context_property(struct mos_bufmgr *bufmgr,
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

    if(0 == bo->offset64)
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
        else if(MEMZONE_DEVICE == bo_gem->mem_region)
        {
            alignment = MAX(bufmgr_gem->default_alignment[MOS_XE_MEM_CLASS_VRAM], PAGE_SIZE_64K);
            offset = __mos_bo_vma_alloc_xe(bo->bufmgr, (enum mos_memory_zone)bo_gem->mem_region, bo->size, PAGE_SIZE_64K);
        }
        else if(MEMZONE_SYS == bo_gem->mem_region)
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
          uint64_t offset, uint64_t addr, uint64_t size, uint32_t op, uint32_t flags,
          struct drm_xe_sync *sync, uint32_t num_syncs, uint32_t region,
          uint64_t ext)
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
    bind.bind.addr = addr;
    bind.bind.op = op;
    bind.bind.flags = flags;
    bind.bind.region = region;
    bind.num_syncs = num_syncs;
    bind.syncs = (uintptr_t)sync;

    ret = drmIoctl(fd, DRM_IOCTL_XE_VM_BIND, &bind);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("Failed to bind vm, vm_id:%d, exec_queue_id:%d, op:0x%x, flags:0x%x, bo_handle:%d, offset:%lx, addr:0x%lx, size:%ld, errno(%d)",
            vm_id, exec_queue_id, op, bo_handle, offset, addr, size, -errno);
    }

    return ret;
}

static int mos_xe_vm_bind_sync(int fd, uint32_t vm_id, uint32_t bo, uint64_t offset,
        uint64_t addr, uint64_t size, uint32_t op, bool is_defer)
{
    if (is_defer)
    {
        return 0;
    }
    struct drm_xe_sync sync;

    memclear(sync);
    sync.flags = DRM_XE_SYNC_SYNCOBJ | DRM_XE_SYNC_SIGNAL;
    sync.handle = mos_sync_syncobj_create(fd, 0);

    int ret = __mos_vm_bind_xe(fd, vm_id, 0, bo, offset, addr, size,
                op, XE_VM_BIND_FLAG_ASYNC, &sync, 1, 0, 0);

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

static int mos_xe_vm_bind_async(int fd, uint32_t vm_id, uint32_t bo, uint64_t offset,
        uint64_t addr, uint64_t size, uint32_t op,
        struct drm_xe_sync *sync, uint32_t num_syncs)
{
    return __mos_vm_bind_xe(fd, vm_id, 0, bo, offset, addr, size,
                op, XE_VM_BIND_FLAG_ASYNC, sync, num_syncs, 0, 0);
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

    if(bufmgr_gem->has_vram &&
            (MOS_MEMPOOL_VIDEOMEMORY == alloc->ext.mem_type   || MOS_MEMPOOL_DEVICEMEMORY == alloc->ext.mem_type))
    {
        bo_gem->mem_region = MEMZONE_DEVICE;
        bo_align = MAX(alloc->alignment, bufmgr_gem->default_alignment[MOS_XE_MEM_CLASS_VRAM]);
    }

    memclear(create);
    if (MEMZONE_DEVICE == bo_gem->mem_region)
    {
        //Note: memory_region is related to gt_id for multi-tiles gpu, take gt_id into consideration in case of multi-tiles
        create.flags = bufmgr_gem->memory_regions    & (~0x1);
    }
    else
    {
        create.flags = bufmgr_gem->memory_regions    & 0x1;
    }

    //Note: We suggest vm_id=0 here as default, otherwise this bo cannot be exported as prelim fd.
    create.vm_id = 0;
    create.size = ALIGN(alloc->size, bo_align);
    if (bufmgr_gem->is_defer_creation_and_binding)
    {
        create.flags |= XE_GEM_CREATE_FLAG_DEFER_BACKING;
    }
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

    if (bufmgr_gem->mem_profiler_fd != -1)
    {
        snprintf(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE, "GEM_CREATE, %d, %d, %lu, %d, %s\n", getpid(), bo_gem->bo.handle, bo_gem->bo.size,bo_gem->mem_region, alloc->name);
        ret = write(bufmgr_gem->mem_profiler_fd, bufmgr_gem->mem_profiler_buffer, strnlen(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE));
        if (-1 == ret)
        {
            MOS_DRM_ASSERTMESSAGE("Failed to write to %s: %s", bufmgr_gem->mem_profiler_path, strerror(errno));
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

    ret = mos_xe_vm_bind_sync(bufmgr_gem->fd, bufmgr_gem->vm_id, bo_gem->gem_handle, 0, bo_gem->bo.offset64, bo_gem->bo.size, XE_VM_BIND_OP_MAP, bufmgr_gem->is_defer_creation_and_binding);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("mos_xe_vm_bind_sync ret: %d", ret);
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

    if(bufmgr_gem->has_vram &&
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

    ret = mos_xe_vm_bind_sync(bufmgr_gem->fd, bufmgr_gem->vm_id, 0, (uint64_t)alloc_uptr->addr, bo_gem->bo.offset64, bo_gem->bo.size, XE_VM_BIND_OP_MAP_USERPTR, bufmgr_gem->is_defer_creation_and_binding);
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
mos_bo_create_from_prime_xe(struct mos_bufmgr *bufmgr, int prime_fd, int size)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bufmgr;
    int ret;
    uint32_t handle;
    struct mos_xe_bo_gem *bo_gem;
    drmMMListHead *list;

    bufmgr_gem->m_lock.lock();
    ret = drmPrimeFDToHandle(bufmgr_gem->fd, prime_fd, &handle);
    if (ret) {
        MOS_DRM_ASSERTMESSAGE("create_from_prime: failed to obtain handle from fd: %s", strerror(errno));
        bufmgr_gem->m_lock.unlock();
        return nullptr;
    }

    /*
     * See if the kernel has already returned this buffer to us. Just as
     * for named buffers, we must not create two bo's pointing at the same
     * kernel object
     */
    for (list = bufmgr_gem->named.next;
         list != &bufmgr_gem->named;
         list = list->next) {
        bo_gem = DRMLISTENTRY(struct mos_xe_bo_gem, list, name_list);
        if (bo_gem->gem_handle == handle) {
            mos_bo_reference_xe(&bo_gem->bo);
            bufmgr_gem->m_lock.unlock();
            return &bo_gem->bo;
        }
    }

    bo_gem = MOS_New(mos_xe_bo_gem);
    if (!bo_gem) {
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
    bo_gem->bo.bufmgr = bufmgr;

    bo_gem->gem_handle = handle;
    atomic_set(&bo_gem->ref_count, 1);

    memcpy(bo_gem->name, "prime", sizeof("prime"));
    bo_gem->mem_region = MEMZONE_PRIME;

    DRMLISTADDTAIL(&bo_gem->name_list, &bufmgr_gem->named);
    bufmgr_gem->m_lock.unlock();

    __mos_bo_set_offset_xe(&bo_gem->bo);

    ret = mos_xe_vm_bind_sync(bufmgr_gem->fd, bufmgr_gem->vm_id, bo_gem->gem_handle, 0, bo_gem->bo.offset64, bo_gem->bo.size, XE_VM_BIND_OP_MAP, bufmgr_gem->is_defer_creation_and_binding);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("mos_xe_vm_bind_sync ret: %d", ret);
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
    if(cmd_bo != nullptr && cmd_bo->bufmgr != nullptr)
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
 * Update busy queue and free queue after wait for bo deps.
 * Move all free deps on read/write dep map from busy queue to free queue;
 * Clear all signaled deps in bo read/write dep map.
 */
static int
__mos_gem_bo_update_dep_queue_xe(struct mos_linux_bo *bo,
            std::map<uint32_t, uint64_t> &max_timeline_data)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bo, -EINVAL)

    mos_xe_bufmgr_gem *bufmgr_gem = (mos_xe_bufmgr_gem *)bo->bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL)
    bufmgr_gem->m_lock.lock();
    // Use max time line data to update dep queue in every ctx.
    for(auto it = max_timeline_data.begin(); it != max_timeline_data.end(); it++)
    {
        uint32_t exec_queue_id = it->first;
        if(bufmgr_gem->global_ctx_info.count(exec_queue_id) > 0)
        {
            mos_xe_context *ctx = bufmgr_gem->global_ctx_info[exec_queue_id];
            if(ctx)
            {
                mos_sync_update_dep_queue(bufmgr_gem->fd,
                            ctx->dep_queue_free,
                            ctx->dep_queue_busy,
                            ctx->free_dep_tmp_list,
                            it->second);
            }
        }
    }
    bufmgr_gem->m_lock.unlock();
    return MOS_XE_SUCCESS;
}

int
__mos_dump_bo_wait_rendering_syncobj_xe(uint32_t bo_handle,
            uint32_t *handles,
            uint32_t count,
            int64_t timeout_nsec,
            uint32_t wait_flags,
            uint32_t rw_flags)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if(__XE_TEST_DEBUG(XE_DEBUG_SYNCHRONIZATION))
    {
        MOS_DRM_CHK_NULL_RETURN_VALUE(handles, -EINVAL)
        char log_msg[MOS_MAX_MSG_BUF_SIZE] = { 0 };
        int offset = 0;
        offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                            MOS_MAX_MSG_BUF_SIZE - offset,
                            "\n\t\t\tdump bo(handle=%d) wait rendering syncobj:",
                            bo_handle);

        for(int i = 0; i < count; i++)
        {
            offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                            MOS_MAX_MSG_BUF_SIZE - offset,
                            "\n\t\t\t-syncobj handle = %d, timeout_nsec = %ld, wait_flags = %d, rw_flags = %d",
                            handles[i],
                            timeout_nsec,
                            wait_flags,
                            rw_flags);
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

    for(auto it : timeline_data)
    {
        handles.push_back(it.first);
        points.push_back(it.second);
    }

    count = handles.size();
    if(count > 0)
    {
        ret = mos_sync_syncobj_timeline_wait(bufmgr_gem->fd,
                        handles.data(),
                        points.data(),
                        count,
                        timeout_nsec,
                        wait_flags,
                        first_signaled);
    }
    bufmgr_gem->sync_obj_rw_lock.unlock_shared();

    return ret;
}

/**
 * @bo indicates to bo object that need to wait
 * @max_timeline_data max timeline data in every exec_queue context for this bo resource.
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
__mos_gem_bo_wait_rendering_with_flags_xe(struct mos_linux_bo *bo,
            std::map<uint32_t, uint64_t> &max_timeline_data,
            int64_t timeout_nsec,
            uint32_t wait_flags,
            uint32_t rw_flags,
            uint32_t *first_signaled)
{
    if(mos_sync_get_synchronization_mode() == MOS_SYNC_TIMELINE)
    {
        return __mos_gem_bo_wait_timeline_rendering_with_flags_xe(
                    bo,
                    timeout_nsec,
                    wait_flags,
                    rw_flags,
                    first_signaled);
    }

    MOS_DRM_CHK_NULL_RETURN_VALUE(bo, -EINVAL)

    mos_xe_bufmgr_gem *bufmgr_gem = (mos_xe_bufmgr_gem *)bo->bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL)

    int ret = MOS_XE_SUCCESS;
    uint32_t count = 0;
    mos_xe_bo_gem *bo_gem = (mos_xe_bo_gem *)bo;
    std::vector<uint32_t> handles;
    std::set<uint32_t> exec_queue_ids;
    bufmgr_gem->m_lock.lock();
    bufmgr_gem->sync_obj_rw_lock.lock_shared();
    MOS_XE_GET_KEYS_FROM_MAP(bufmgr_gem->global_ctx_info, exec_queue_ids);

    std::vector<struct mos_xe_dep*> used_deps;
    mos_sync_get_bo_wait_deps(exec_queue_ids,
                bo_gem->read_deps,
                bo_gem->write_deps,
                max_timeline_data,
                bo_gem->last_exec_write_exec_queue,
                handles,
                used_deps,
                rw_flags);
    bufmgr_gem->m_lock.unlock();
    count = handles.size();
    if(count > 0)
    {
        ret = mos_sync_syncobj_wait_err(bufmgr_gem->fd,
                        handles.data(),
                        count,
                        timeout_nsec,
                        wait_flags,
                        first_signaled);

        __mos_dump_bo_wait_rendering_syncobj_xe(bo->handle,
                handles.data(),
                count,
                timeout_nsec,
                wait_flags,
                rw_flags);
    }
    bufmgr_gem->sync_obj_rw_lock.unlock_shared();
    mos_sync_dec_reference_count_in_deps(used_deps);
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

    if(mos_sync_get_synchronization_mode() != MOS_SYNC_NONE)
    {
        int64_t timeout_nsec = 0;
        uint32_t wait_flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
        uint32_t rw_flags = EXEC_OBJECT_READ_XE | EXEC_OBJECT_WRITE_XE;
        std::map<uint32_t, uint64_t> max_timeline_data;
        int ret = __mos_gem_bo_wait_rendering_with_flags_xe(bo, max_timeline_data, timeout_nsec, wait_flags, rw_flags, nullptr);
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
            __mos_gem_bo_update_dep_queue_xe(bo, max_timeline_data);
            return false;
        }
    }
    else if (!bufmgr_gem->is_defer_creation_and_binding)
    {
        //Note: hard code here for non-synchronization and remove after switch done.
        usleep(5000);
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
    if(bo == nullptr || bo->bufmgr == nullptr)
    {
        MOS_DRM_ASSERTMESSAGE("ptr is null pointer");
        return;
    }
    mos_xe_bufmgr_gem *bufmgr_gem = (mos_xe_bufmgr_gem *)bo->bufmgr;

    if(mos_sync_get_synchronization_mode() != MOS_SYNC_NONE)
    {
        int64_t timeout_nsec = INT64_MAX;
        uint32_t wait_flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
        uint32_t rw_flags = EXEC_OBJECT_READ_XE | EXEC_OBJECT_WRITE_XE;
        std::map<uint32_t, uint64_t> max_timeline_data;
        int ret = __mos_gem_bo_wait_rendering_with_flags_xe(bo, max_timeline_data, timeout_nsec, wait_flags, rw_flags, nullptr);

        if (ret)
        {
            MOS_DRM_ASSERTMESSAGE("bo_wait_rendering_xe ret:%d, error:%d", ret, -errno);
        }
        else if (MOS_XE_SUCCESS == ret)
        {
            __mos_gem_bo_update_dep_queue_xe(bo, max_timeline_data);
        }
    }
    else if(!bufmgr_gem->is_defer_creation_and_binding)
    {
        //Note: hard code here for non-synchronization and remove after switch done.
        usleep(5000);
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
    if(timeout_ns)
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

    if(mos_sync_get_synchronization_mode() != MOS_SYNC_NONE)
    {
        int64_t timeout_nsec = INT64_MAX;
        uint32_t wait_flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
        uint32_t rw_flags = write_enable ? EXEC_OBJECT_WRITE_XE : EXEC_OBJECT_READ_XE;
        std::map<uint32_t, uint64_t> max_timeline_data;
        ret = __mos_gem_bo_wait_rendering_with_flags_xe(bo, max_timeline_data, timeout_nsec, wait_flags, rw_flags, nullptr);
        if(ret == MOS_XE_SUCCESS)
        {
            __mos_gem_bo_update_dep_queue_xe(bo, max_timeline_data);
        }
        else
        {
            MOS_DRM_ASSERTMESSAGE("bo wait rendering error(%d ns)", -errno);
        }
    }
    else if (!bufmgr_gem->is_defer_creation_and_binding)
    {
        //Note: hard code here for non-synchronization and remove after switch done.
        usleep(5000);
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

int __mos_dump_syncs_array_xe(struct drm_xe_sync *syncs,
            uint32_t count,
            mos_xe_dep *dep)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if(__XE_TEST_DEBUG(XE_DEBUG_SYNCHRONIZATION))
    {
        MOS_DRM_CHK_NULL_RETURN_VALUE(syncs, -EINVAL)
        MOS_DRM_CHK_NULL_RETURN_VALUE(dep, -EINVAL)
        char log_msg[MOS_MAX_MSG_BUF_SIZE] = { 0 };
        int offset = 0;
        offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                    MOS_MAX_MSG_BUF_SIZE - offset,
                    "\n\t\t\tdump fence out syncobj: handle = %d, flags = %d",
                    dep->sync.handle, dep->sync.flags);
        if(count > 0)
        {
            offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                    MOS_MAX_MSG_BUF_SIZE - offset,
                    "\n\t\t\tdump exec syncs array, num sync = %d",
                    count);
        }
        for(int i = 0; i < count; i++)
        {
            offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                    MOS_MAX_MSG_BUF_SIZE - offset,
                    "\n\t\t\t-syncobj_handle = %d, flags=%d",
                    syncs[i].handle, syncs[i].flags);
        }
        offset > MOS_MAX_MSG_BUF_SIZE ?
            MOS_DRM_NORMALMESSAGE("imcomplete dump since log msg buffer overwrite %s", log_msg) : MOS_DRM_NORMALMESSAGE("%s", log_msg);
    }
#endif
    return MOS_XE_SUCCESS;
}

int
__mos_dump_bo_deps_map_xe(struct mos_linux_bo **bo,
            int num_bo,
            std::vector<mos_xe_exec_bo> &exec_list,
            uint32_t curr_exec_queue_id,
            std::map<uint32_t, struct mos_xe_context*> ctx_infos)
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if(__XE_TEST_DEBUG(XE_DEBUG_SYNCHRONIZATION))
    {
        MOS_DRM_CHK_NULL_RETURN_VALUE(bo, -EINVAL)
        uint32_t exec_list_size = exec_list.size();
        for(int i = 0; i < exec_list_size + num_bo; i++)
        {
            mos_xe_bo_gem *exec_bo_gem = nullptr;
            uint32_t exec_flags = 0;
            if(i < exec_list_size)
            {
                exec_bo_gem = (mos_xe_bo_gem *)exec_list[i].bo;
                exec_flags = exec_list[i].flags;
            }
            else
            {
                exec_bo_gem = (mos_xe_bo_gem *)bo[i - exec_list_size];
                exec_flags = EXEC_OBJECT_WRITE_XE; //use write flags for batch bo as default.
            }
            if(exec_bo_gem)
            {
                if(exec_bo_gem->is_imported || exec_bo_gem->is_exported)
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
                    while(it != exec_bo_gem->read_deps.end())
                    {
                        if (ctx_infos.count(it->first) > 0)
                        {
                            offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                                            MOS_MAX_MSG_BUF_SIZE - offset,
                                            "\n\t\t\t-read deps: execed_exec_queue_id=%d, dep_status=%d, syncobj_handle=%d, sync_flags=%d",
                                            it->first,
                                            it->second.dep ? it->second.dep->status : -1,
                                            it->second.dep ? it->second.dep->sync.handle : INVALID_HANDLE,
                                            it->second.dep ? it->second.dep->sync.flags : -1);
                        }
                        it++;
                    }

                    it = exec_bo_gem->write_deps.begin();
                    while(it != exec_bo_gem->write_deps.end())
                    {
                        if (ctx_infos.count(it->first) > 0)
                        {
                            offset += MOS_SecureStringPrint(log_msg + offset, MOS_MAX_MSG_BUF_SIZE,
                                            MOS_MAX_MSG_BUF_SIZE - offset,
                                            "\n\t\t\t-write deps: execed_exec_queue_id=%d, dep_status=%d, syncobj_handle=%d, sync_flags=%d",
                                            it->first,
                                            it->second.dep ? it->second.dep->status : -1,
                                            it->second.dep ? it->second.dep->sync.handle : INVALID_HANDLE,
                                            it->second.dep ? it->second.dep->sync.flags : -1);
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
            std::vector<mos_xe_dep*> &used_internal_deps,
            std::vector<struct mos_xe_external_bo_info> &external_bos)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(ctx, -EINVAL);
    uint32_t curr_dummy_exec_queue_id = ctx->dummy_exec_queue_id;
    uint32_t exec_list_size = exec_list.size();
    int ret = 0;
    std::set<uint32_t> exec_queue_ids;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL);
    MOS_XE_GET_KEYS_FROM_MAP(bufmgr_gem->global_ctx_info, exec_queue_ids);

    for(int i = 0; i < exec_list_size + num_bo; i++)
    {
        mos_xe_bo_gem *exec_bo_gem = nullptr;
        uint32_t exec_flags = 0;
        if(i < exec_list_size)
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

        if(exec_bo_gem)
        {
            if(exec_flags == 0)
            {
                //Add an assert message here in case of potential thread safety issue.
                //Currently, exec bo's flags could only be in (0, EXEC_OBJECT_READ_XE | EXEC_OBJECT_WRITE_XE]
                MOS_DRM_ASSERTMESSAGE("Invalid op flags(0x0) for exec bo(handle=%d)", exec_bo_gem->bo.handle);
            }

            if(exec_bo_gem->is_imported || exec_bo_gem->is_exported)
            {
                //external bo, need to export its syncobj everytime.
                int prime_fd = INVALID_HANDLE;
                ret = mos_sync_update_exec_syncs_from_handle(
                            bufmgr_gem->fd,
                            exec_bo_gem->bo.handle,
                            exec_flags,
                            syncs,
                            prime_fd);
                if(ret == MOS_XE_SUCCESS)
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
                ret = mos_sync_update_exec_syncs_from_deps(
                            curr_dummy_exec_queue_id,
                            exec_bo_gem->last_exec_write_exec_queue,
                            exec_flags,
                            exec_queue_ids,
                            exec_bo_gem->read_deps,
                            exec_bo_gem->write_deps,
                            syncs,
                            used_internal_deps);
            }
        }
    }
    return MOS_XE_SUCCESS;
}

static struct mos_xe_dep*
__mos_context_exec_update_syncs_from_queue_xe(struct mos_xe_bufmgr_gem *bufmgr_gem,
            struct mos_xe_context *ctx,
            std::vector<struct drm_xe_sync> &syncs)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, nullptr);
    MOS_DRM_CHK_NULL_RETURN_VALUE(ctx, nullptr);
    /**
     * Get a new dep from queue and add it into syncs;
     * Note, this dep must return back to busy queue after exec submission.
     */
    bool need_wait = false;
    mos_sync_update_free_dep_tmp_list(bufmgr_gem->fd, ctx->free_dep_tmp_list, ctx->dep_queue_free);
    mos_xe_dep *dep = mos_sync_update_exec_syncs_from_queue(
                bufmgr_gem->fd,
                ctx->dep_queue_free,
                ctx->dep_queue_busy,
                ctx->free_dep_tmp_list,
                syncs,
                need_wait);
    if (dep && need_wait)
    {
        bufmgr_gem->sync_obj_rw_lock.lock();
        int ret = mos_sync_syncobj_reset(
                    bufmgr_gem->fd,
                    &(dep->sync.handle),
                    1);
        if(ret)
        {
            MOS_DRM_ASSERTMESSAGE("failed to reset syncobj(%d)", dep->sync.handle);
        }
        bufmgr_gem->sync_obj_rw_lock.unlock();
    }
    return dep;
}

static int
__mos_context_exec_update_bo_deps_xe(struct mos_linux_bo **bo,
            int num_bo,
            std::vector<mos_xe_exec_bo> &exec_list,
            uint32_t curr_exec_queue_id,
            struct mos_xe_dep *dep)
{
    uint32_t exec_list_size = exec_list.size();

    for(int i = 0; i < exec_list_size + num_bo; i++)
    {
        mos_xe_bo_gem *exec_bo_gem = nullptr;
        uint32_t exec_flags = 0;
        if(i < exec_list_size)
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
        if(exec_bo_gem)
        {
            mos_sync_update_bo_deps(curr_exec_queue_id, exec_flags, dep, exec_bo_gem->read_deps, exec_bo_gem->write_deps);
            if(exec_flags & EXEC_OBJECT_READ_XE)
            {
                exec_bo_gem->last_exec_read_exec_queue = curr_exec_queue_id;
            }
            if(exec_flags & EXEC_OBJECT_WRITE_XE)
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
    ret = __mos_get_context_property(bufmgr, ctx, XE_EXEC_QUEUE_GET_PROPERTY_BAN, property_value);

    /**
     * if exec_queue is banned, queried value is 1, otherwise it is zero;
     * if exec failure is not caused by exec_queue ban, umd could not help recover it.
     */
    if(ret || !property_value)
    {
        MOS_DRM_ASSERTMESSAGE("Failed to retore ctx(%d) with error(%d)",
                    curr_exec_queue_id, -EPERM);
        return -EPERM;
    }

    ret = __mos_context_restore_xe(bufmgr, ctx);

    if(ret == MOS_XE_SUCCESS)
    {
        curr_exec_queue_id = ctx->ctx_id;
        exec.exec_queue_id = curr_exec_queue_id;
        //try once again to submit
        ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_EXEC, &exec);
        if(ret)
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
 *  3. Get a new dep from the dep_queue_free and dep_queue_busy, and add it to syncs array;
 *      Note: if the new dep comes from dep_queue_busy, exec must wait and reset it.
 *  4. Exec submittion with batches and syncs.
 *  5. Update read_deps[ctx->dummy_exec_queue_id] and write_deps[ctx->dummy_exec_queue_id] with the new deps from the dep_queue;
 *  6. Return back the dep to dep_queue_busy.
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
    if(num_bo <= 0)
    {
        MOS_DRM_ASSERTMESSAGE("invalid batch bo num(%d)", num_bo);
        return -EINVAL;
    }

    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo[0]->bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr_gem, -EINVAL)

    uint64_t batch_addrs[num_bo];

    std::vector<mos_xe_exec_bo> exec_list;
    for(int i = 0; i < num_bo; i++)
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
    std::vector<mos_xe_dep*> used_internal_deps;
    uint64_t curr_timeline = 0;
    int ret = 0;

    uint32_t exec_list_size = exec_list.size();
    if(exec_list_size == 0)
    {
        MOS_DRM_NORMALMESSAGE("invalid exec list count(%d)", exec_list_size);
    }

    bufmgr_gem->m_lock.lock();
    struct mos_xe_dep *dep = __mos_context_exec_update_syncs_from_queue_xe(bufmgr_gem, context, syncs);
    if(dep == nullptr)
    {
        MOS_DRM_ASSERTMESSAGE("Failed to get dep from queue");
        bufmgr_gem->m_lock.unlock();
        return -EINVAL;
    }
    bufmgr_gem->sync_obj_rw_lock.lock_shared();
    //update exec syncs array by external and interbal bo dep
    __mos_context_exec_update_syncs_xe(
                bufmgr_gem,
                bo,
                num_bo,
                context,
                exec_list,
                syncs,
                used_internal_deps,
                external_bos);

    //exec submit
    uint32_t sync_count = syncs.size();
    struct drm_xe_sync *syncs_array = syncs.data();
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
    if(ret)
    {
        MOS_DRM_ASSERTMESSAGE("Failed to submission in DRM_IOCTL_XE_EXEC(errno:%d): exec_queue_id = %d, num_syncs = %d, num_bo = %d",
                    -errno, curr_exec_queue_id, sync_count, num_bo);

        //check if it caused by guilty exec_queue_id, if so, could restore the exec_queue_id/ queue here and re-try exec again.
        if(ret == -EPERM)
        {
            ret = __mos_bo_context_exec_retry_xe(&bufmgr_gem->bufmgr, ctx, exec, curr_exec_queue_id);
        }
    }
    curr_timeline = dep->sync.timeline_value;

    //dump fence in and fence out info
    __mos_dump_syncs_array_xe(syncs_array, sync_count, dep);
    //dump bo deps map
    __mos_dump_bo_deps_map_xe(bo, num_bo, exec_list, curr_exec_queue_id, bufmgr_gem->global_ctx_info);

    if(mos_sync_get_synchronization_mode() == MOS_SYNC_SYNCOBJ)
    {
        //return back dep to busy queue which is got from queue in previous.
        mos_sync_return_back_dep(context->dep_queue_busy, dep, context->cur_timeline_index);
    }

    //dep timeline index has been updated in mos_sync_return_back_dep, then could update read, write dep maps.
    __mos_context_exec_update_bo_deps_xe(bo, num_bo, exec_list, context->dummy_exec_queue_id, dep);

    if(mos_sync_get_synchronization_mode() == MOS_SYNC_TIMELINE)
    {
        //Update dep with latest available timeline
        mos_sync_update_timeline_dep(dep);
    }

    mos_sync_dec_reference_count_in_deps(used_internal_deps);

    bufmgr_gem->sync_obj_rw_lock.unlock_shared();
    bufmgr_gem->m_lock.unlock();

    //import batch syncobj or its point for external bos and close syncobj created for external bo before.
    uint32_t external_bo_count = external_bos.size();
    int sync_file_fd = INVALID_HANDLE;
    int temp_syncobj = INVALID_HANDLE;

    if(external_bo_count > 0)
    {
        if(mos_sync_get_synchronization_mode() == MOS_SYNC_SYNCOBJ)
        {
            sync_file_fd = mos_sync_syncobj_handle_to_syncfile_fd(bufmgr_gem->fd, dep->sync.handle);
        }
        else if(mos_sync_get_synchronization_mode() == MOS_SYNC_TIMELINE)
        {
            temp_syncobj = mos_sync_syncobj_create(bufmgr_gem->fd, 0);
            if(temp_syncobj > 0)
            {
                mos_sync_syncobj_timeline_to_binary(bufmgr_gem->fd, temp_syncobj, dep->sync.handle, curr_timeline, 0);
                sync_file_fd = mos_sync_syncobj_handle_to_syncfile_fd(bufmgr_gem->fd, temp_syncobj);
            }
        }
    }
    for(int i = 0; i < external_bo_count; i++)
    {
        //import syncobj for external bos
        if(sync_file_fd >= 0)
        {
            mos_sync_import_syncfile_to_external_bo(bufmgr_gem->fd, external_bos[i].prime_fd, sync_file_fd);
        }
        if(external_bos[i].prime_fd != INVALID_HANDLE)
        {
            close(external_bos[i].prime_fd);
        }
        mos_sync_syncobj_destroy(bufmgr_gem->fd, external_bos[i].syncobj_handle);
    }
    if(sync_file_fd >= 0)
    {
        close(sync_file_fd);
    }
    if(temp_syncobj > 0)
    {
        mos_sync_syncobj_destroy(bufmgr_gem->fd, temp_syncobj);
    }

    //Note: keep exec return value for final return value.
    return ret;
}

static int
mos_bo_context_exec_xe(struct mos_linux_bo **bo, int num_bo, struct mos_linux_context *ctx,
                               struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                               unsigned int flags, int *fence)
{
    if((nullptr == bo) || (nullptr == ctx) || num_bo <= 0)
    {
        return -EINVAL;
    }

    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo[0]->bufmgr;
    struct mos_xe_context *context = (struct mos_xe_context *) ctx;
    struct drm_xe_sync sync;
    struct drm_xe_exec exec;
    int ret = 0;
    uint64_t batch_addrs[num_bo];

    for(int i = 0; i < num_bo; i++)
    {
        MOS_DRM_CHK_NULL_RETURN_VALUE(bo[i], -EINVAL)
        batch_addrs[i] = bo[i]->offset64;
    }
    memclear(sync);
    sync.handle = mos_sync_syncobj_create(bufmgr_gem->fd, 0);
    sync.flags = DRM_XE_SYNC_SYNCOBJ | DRM_XE_SYNC_SIGNAL;

    memclear(exec);
    exec.extensions = 0;
    exec.exec_queue_id = ctx->ctx_id;
    exec.num_syncs = 1;
    exec.syncs = (uint64_t)&sync;
    exec.address = (num_bo == 1 ? batch_addrs[0] : (uint64_t)batch_addrs);
    exec.num_batch_buffer = num_bo;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_XE_EXEC, &exec);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("error:%d", -errno);
        mos_sync_syncobj_destroy(bufmgr_gem->fd, sync.handle);
        return ret;
    }

    ret = mos_sync_syncobj_wait_err(bufmgr_gem->fd, &sync.handle, 1, INT64_MAX, 0, NULL);
    if (ret)
    {
        MOS_DRM_ASSERTMESSAGE("syncobj_wait error:%d", -errno);
    }

    mos_sync_syncobj_destroy(bufmgr_gem->fd, sync.handle);

    return ret;
}

/**
 * Get the DEVICE ID for the device.  This can be overridden by setting the
 * INTEL_DEVID_OVERRIDE environment variable to the desired ID.
 */
static int
mos_get_devid_xe(struct mos_bufmgr *bufmgr)
{
    if (nullptr == bufmgr)
    {
        MOS_DRM_ASSERTMESSAGE("bufmgr is nullptr");
        return 0;
    }
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct drm_xe_query_config *config;
    int devid = 0;

    if (nullptr == bufmgr_gem->config)
    {
        bufmgr_gem->config = __mos_query_config_xe(bufmgr_gem->fd);
    }

    config = bufmgr_gem->config;

    if (nullptr == config)
    {
        MOS_DRM_ASSERTMESSAGE("Get config failed");
        return devid;
    }

    devid = config->info[XE_QUERY_CONFIG_REV_AND_DEVICE_ID] & 0xffff;

    return devid;
}

static struct drm_xe_engine_class_instance *
__mos_query_engines_xe(int fd, unsigned int *nengine)
{
    if (fd < 0 || nullptr == nengine)
    {
        return nullptr;
    }

    struct drm_xe_device_query query;
    struct drm_xe_engine_class_instance *hw_engines;
    int ret;

    memclear(query);
    query.extensions = 0;
    query.query = DRM_XE_DEVICE_QUERY_ENGINES;
    query.size = 0;
    query.data = 0;

    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query);
    if(ret || !query.size)
    {
        MOS_DRM_ASSERTMESSAGE("ret:%d, length:%d", ret, query.size);
        *nengine = 0;
        return nullptr;
    }

    *nengine = query.size / sizeof(struct drm_xe_engine_class_instance);

    hw_engines = (drm_xe_engine_class_instance *)malloc(query.size);
    if (nullptr == hw_engines)
    {
        MOS_DRM_ASSERTMESSAGE("malloc hw_engines failed");
        return nullptr;
    }

    memset(hw_engines, 0, query.size);
    query.data = (uintptr_t)hw_engines;
    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query);
    if (ret || !query.size)
    {
        MOS_DRM_ASSERTMESSAGE("ret:%d, length:%d", ret, query.size);
        MOS_XE_SAFE_FREE(hw_engines);
        return nullptr;
    }

    //Note28: nengine and hw_engines are from all GTs, need to filter with gt_id

    return hw_engines;
}

static int
mos_query_engines_count_xe(struct mos_bufmgr *bufmgr, unsigned int *nengine)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    MOS_DRM_CHK_NULL_RETURN_VALUE(nengine, -EINVAL);

    if (nullptr == bufmgr_gem->hw_engines)
    {
        bufmgr_gem->hw_engines = __mos_query_engines_xe(bufmgr_gem->fd, &bufmgr_gem->number_hw_engines);
        if (nullptr == bufmgr_gem->hw_engines)
        {
            return -ENODEV;
        }
    }

    *nengine = bufmgr_gem->number_hw_engines;

    return 0;
}

int
mos_query_engines_class_xe(struct mos_bufmgr *bufmgr,
                      __u16 engine_class,
                      __u64 caps,
                      unsigned int *nengine,
                      void *engine_map)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct drm_xe_engine_class_instance *ci = (struct drm_xe_engine_class_instance *)engine_map;
    struct drm_xe_engine_class_instance *hw_engines;

    MOS_DRM_CHK_NULL_RETURN_VALUE(nengine, -EINVAL);
    MOS_DRM_CHK_NULL_RETURN_VALUE(engine_map, -EINVAL);

    if (nullptr == bufmgr_gem->hw_engines)
    {
        bufmgr_gem->hw_engines = __mos_query_engines_xe(bufmgr_gem->fd, &bufmgr_gem->number_hw_engines);
        if (nullptr == bufmgr_gem->hw_engines)
        {
            return -ENODEV;
        }
    }

    hw_engines = bufmgr_gem->hw_engines;

    int i, num;
    for (i = 0, num = 0; i < bufmgr_gem->number_hw_engines; i++)
    {
        //Note29: need to filter engine with caps
        if (engine_class == hw_engines->engine_class)
        {
            ci->engine_class = engine_class;
            ci->engine_instance = hw_engines->engine_instance;
            ci->gt_id = hw_engines->gt_id;
            ci++;
            num++;
        }

        hw_engines++;
    }
    //Note30: need to confirm if engine_instance is ordered, otherwise re-order needed.

    if (num <= *nengine)
    {
        *nengine = num;
    }

    return 0;
}

static size_t
mos_get_engine_class_size_xe()
{
    return sizeof(struct drm_xe_engine_class_instance);
}

//Note31: remove this code and restore it to previous version in HwInfoLinux.cpp
static int
mos_query_sys_engines_xe(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    int ret;

    if (nullptr == gfx_info)
    {
        return -EINVAL;
    }

    if (nullptr == bufmgr_gem->hw_engines)
    {
        bufmgr_gem->hw_engines = __mos_query_engines_xe(bufmgr_gem->fd, &bufmgr_gem->number_hw_engines);
        if (nullptr == bufmgr_gem->hw_engines)
        {
            return -ENODEV;
        }
    }

    if (0 == gfx_info->VDBoxInfo.NumberOfVDBoxEnabled)
    {
        unsigned int nengine = bufmgr_gem->number_hw_engines;
        struct drm_xe_engine_class_instance *uengines = nullptr;
        uengines = (struct drm_xe_engine_class_instance *)MOS_AllocAndZeroMemory(nengine * sizeof(struct drm_xe_engine_class_instance));
        if (nullptr == uengines)
        {
            return -ENOMEM;
        }
        ret = mos_query_engines_class_xe(bufmgr, DRM_XE_ENGINE_CLASS_VIDEO_DECODE, 0, &nengine, (void *)uengines);
        if (ret)
        {
            MOS_DRM_ASSERTMESSAGE("Failed to query vdbox engine");
            MOS_SafeFreeMemory(uengines);
            return -ENODEV;
        }
        else
        {
            gfx_info->VDBoxInfo.NumberOfVDBoxEnabled = nengine;
        }

        for (int i=0; i<nengine; i++)
        {
            gfx_info->VDBoxInfo.Instances.VDBoxEnableMask |= 1<<uengines[i].engine_instance;
        }

        MOS_SafeFreeMemory(uengines);
    }

    if (0 == gfx_info->VEBoxInfo.NumberOfVEBoxEnabled)
    {
        unsigned int nengine = bufmgr_gem->number_hw_engines;
        struct drm_xe_engine_class_instance *uengines = nullptr;
        uengines = (struct drm_xe_engine_class_instance *)MOS_AllocAndZeroMemory(nengine * sizeof(struct drm_xe_engine_class_instance));
        if (nullptr == uengines)
        {
            return -ENOMEM;
        }
        ret = mos_query_engines_class_xe(bufmgr, DRM_XE_ENGINE_CLASS_VIDEO_ENHANCE, 0, &nengine, (void *)uengines);
        if (ret)
        {
            MOS_DRM_ASSERTMESSAGE("Failed to query vebox engine");
            MOS_SafeFreeMemory(uengines);
            return -ENODEV;
        }
        else
        {
            MOS_OS_ASSERT(nengine <= bufmgr_gem->number_hw_engines);
            gfx_info->VEBoxInfo.NumberOfVEBoxEnabled = nengine;
        }

        MOS_SafeFreeMemory(uengines);
    }

    return 0;
}

static uint32_t *
__mos_query_hw_config_xe(int fd, uint32_t* config_len)
{
    struct drm_xe_device_query query;
    uint32_t *hw_config;
    int ret;

    if (fd < 0 || nullptr == config_len)
    {
        return nullptr;
    }

    *config_len = 0;

    memclear(query);
    query.query = DRM_XE_DEVICE_QUERY_HWCONFIG;

    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query);
    if(ret || !query.size)
    {
        MOS_DRM_ASSERTMESSAGE("ret:%d, length:%d", ret, query.size);
        return nullptr;
    }

    hw_config = (uint32_t *)malloc(query.size);
    if (hw_config == nullptr)
    {
        MOS_DRM_ASSERTMESSAGE("malloc hw_config failed");
        return nullptr;
    }

    memset(hw_config, 0, query.size);

    query.data = (uintptr_t)hw_config;
    ret = drmIoctl(fd, DRM_IOCTL_XE_DEVICE_QUERY, &query);
    if (ret != 0 || query.size <= 0)
    {
        MOS_DRM_ASSERTMESSAGE("ret:%d, length:%d", ret, query.size);
        MOS_XE_SAFE_FREE(hw_config);
        return nullptr;
    }

    *config_len = query.size / sizeof(uint32_t);

    return hw_config;
}


static int
mos_query_device_blob_xe(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    struct drm_xe_device_query query;
    uint32_t *hwconfig;

    if (nullptr == gfx_info)
    {
        return -EINVAL;
    }

    if (nullptr == bufmgr_gem->hw_config)
    {
        bufmgr_gem->hw_config = __mos_query_hw_config_xe(bufmgr_gem->fd, &bufmgr_gem->config_len);
        if (nullptr == bufmgr_gem->hw_config)
        {
            return -ENODEV;
        }
    }

    hwconfig = bufmgr_gem->hw_config;

    int i = 0;
    while (i < bufmgr_gem->config_len) {
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

        if (INTEL_HWCONFIG_MAX_DUAL_SUBSLICES_SUPPORTED == hwconfig[i])
        {
            assert(hwconfig[i+1] == 1);
            gfx_info->SubSliceCount = hwconfig[i+2];
            gfx_info->MaxSubSlicesSupported = hwconfig[i+2];
        }

        if (INTEL_HWCONFIG_MAX_NUM_EU_PER_DSS == hwconfig[i])
        {
            assert(hwconfig[i+1] == 1);
            gfx_info->MaxEuPerSubSlice = hwconfig[i+2];
        }

        if (INTEL_HWCONFIG_L3_CACHE_SIZE_IN_KB == hwconfig[i])
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
    //Note33: define a macro to indicate unimplement func;
    //#define MOS_XE_UNIMPLEMENT(return) return;
    return;
}

static uint64_t
mos_get_platform_information_xe(struct mos_bufmgr *bufmgr)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(bufmgr, -EINVAL)
    return bufmgr->platform_information;
}

static void
mos_set_platform_information_xe(struct mos_bufmgr *bufmgr, uint64_t p)
{
    if(bufmgr)
        bufmgr->platform_information |= p;
}

// The function is not supported on KMD
static int mos_query_hw_ip_version_xe(struct mos_bufmgr *bufmgr, __u16 engine_class, void *ip_ver_info)
{
    return 0;
}

static void
mos_bo_free_xe(struct mos_linux_bo *bo)
{
    struct mos_xe_bufmgr_gem *bufmgr_gem = nullptr;
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *) bo;
    struct drm_gem_close close_ioctl;
    int ret;

    if(nullptr == bo_gem)
    {
        MOS_DRM_ASSERTMESSAGE("bo == nullptr");
        return;
    }

    bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo->bufmgr;

    if(nullptr == bufmgr_gem)
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
            VG(VALGRIND_FREELIKE_BLOCK(bo_gem->mem_virtual, 0));
            drm_munmap(bo_gem->mem_virtual, bo_gem->bo.size);
        }
    }

    if(bo->vm_id != INVALID_VM)
    {
        ret = mos_xe_vm_bind_sync(bufmgr_gem->fd, bo->vm_id, 0, 0, bo->offset64, bo->size, XE_VM_BIND_OP_UNMAP, bufmgr_gem->is_defer_creation_and_binding);
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
    //same as Note33
    MOS_UNUSED(bo);
    return 0;
}

static void
mos_bufmgr_gem_destroy_xe(struct mos_bufmgr *bufmgr)
{
    if (nullptr == bufmgr)
        return;

    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bufmgr;
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

    MOS_XE_SAFE_FREE(bufmgr_gem->hw_config)
    bufmgr_gem->hw_config = nullptr;

    MOS_XE_SAFE_FREE(bufmgr_gem->config);
    bufmgr_gem->config = nullptr;

    MOS_XE_SAFE_FREE(bufmgr_gem->hw_engines);
    bufmgr_gem->hw_engines = nullptr;

    MOS_XE_SAFE_FREE(bufmgr_gem->mem_usage);
    bufmgr_gem->mem_usage = nullptr;

    MOS_XE_SAFE_FREE(bufmgr_gem->gt_list);
    bufmgr_gem->gt_list = nullptr;

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
    //same as Note33
    MOS_UNUSED(ctx);
    MOS_UNUSED(size);
    MOS_UNUSED(param);
    MOS_UNUSED(value);
    return 0;
}

static void mos_enable_softpin_xe(struct mos_bufmgr *bufmgr, bool va1m_align)
{
    //same as Note33
    MOS_UNUSED(bufmgr);
    MOS_UNUSED(va1m_align);
}

static int
mos_get_reset_stats_xe(struct mos_linux_context *ctx,
              uint32_t *reset_count,
              uint32_t *active,
              uint32_t *pending)
{
    MOS_DRM_CHK_NULL_RETURN_VALUE(ctx, -EINVAL);

    struct mos_xe_context *context = (struct mos_xe_context *)ctx;
    if(reset_count)
        *reset_count = context->reset_count;
    if(active)
        *active = 0;
    if(pending)
        *pending = 0;
    return 0;
}

static mos_oca_exec_list_info*
mos_bo_get_oca_exec_list_info_xe(struct mos_linux_bo *bo, int *count)
{
    if(nullptr == bo  || nullptr == count)
    {
        return nullptr;
    }

    mos_oca_exec_list_info *info = nullptr;
    int counter = 0;
    int MAX_COUNT = 50;
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *) bo->bufmgr;
    struct mos_xe_bo_gem *bo_gem = (struct mos_xe_bo_gem *)bo;
    int exec_list_count = bo_gem->exec_list.size();

    if(exec_list_count == 0 || exec_list_count > MAX_COUNT)
    {
        return nullptr;
    }

    info = (mos_oca_exec_list_info *)malloc((exec_list_count + 1) * sizeof(mos_oca_exec_list_info));
    if(!info)
    {
        MOS_DRM_ASSERTMESSAGE("malloc mos_oca_exec_list_info failed");
        return info;
    }

    for(auto &it : bo_gem->exec_list)
    {
        /*note: set capture for each bo*/
        struct mos_xe_bo_gem *exec_bo_gem = (struct mos_xe_bo_gem *)it.second.bo;
        uint32_t exec_flags = it.second.flags;
        if(exec_bo_gem)
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
    //same as Note33
    MOS_UNUSED(bufmgr);
    return true;
}

static void
mos_bo_set_object_capture_xe(struct mos_linux_bo *bo)
{
    MOS_UNUSED(bo);
    // Do nothing, because object capture is not supported in xe kmd.
    MOS_DRM_NORMALMESSAGE("Object capture is not supported in xe kmd");
}

static void
mos_bo_set_object_async_xe(struct mos_linux_bo *bo)
{
    MOS_UNUSED(bo);
    // Do nothing, because object capture is not supported in xe kmd.
    MOS_DRM_NORMALMESSAGE("Object capture is not supported in xe kmd");
}

static int
mos_get_driver_info_xe(struct mos_bufmgr *bufmgr, struct LinuxDriverInfo *drvInfo)
{
    if (nullptr == bufmgr || nullptr == drvInfo)
    {
        return -EINVAL;
    }
    uint32_t *hw_config = nullptr;
    uint32_t MaxEuPerSubSlice = 0;
    int i = 0;
    struct drm_xe_engine_class_instance *hw_engines = nullptr;
    struct drm_xe_query_config *config = nullptr;
    struct mos_xe_bufmgr_gem *bufmgr_gem = (struct mos_xe_bufmgr_gem *)bufmgr;
    drvInfo->hasBsd = 1;
    drvInfo->hasBsd2 = 1;
    drvInfo->hasVebox = 1;

    //For XE driver always has ppgtt
    drvInfo->hasPpgtt = 1;

    // Step1: Get hw_config
    if (nullptr == bufmgr_gem->hw_config)
    {
        bufmgr_gem->hw_config = __mos_query_hw_config_xe(bufmgr_gem->fd, &bufmgr_gem->config_len);
    }
    hw_config = bufmgr_gem->hw_config;
    if (hw_config)
    {
        while (i < bufmgr_gem->config_len)
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

            if (INTEL_HWCONFIG_MAX_DUAL_SUBSLICES_SUPPORTED == hw_config[i])
            {
                assert(hw_config[i+1] == 1);
                drvInfo->subSliceCount = hw_config[i+2];
            }

            if (INTEL_HWCONFIG_MAX_NUM_EU_PER_DSS == hw_config[i])
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

    // Step2: Get hw_engines
    if (nullptr == bufmgr_gem->hw_engines)
    {
        bufmgr_gem->hw_engines = __mos_query_engines_xe(bufmgr_gem->fd, &bufmgr_gem->number_hw_engines);
        if (nullptr == bufmgr_gem->hw_engines)
        {
            MOS_DRM_ASSERTMESSAGE("get hw_engines failed");
            return -ENODEV;
        }
    }
    hw_engines = bufmgr_gem->hw_engines;
    int engine_class_num;
    for (i = 0, engine_class_num = 0; i < bufmgr_gem->number_hw_engines; i++)
    {
        if (DRM_XE_ENGINE_CLASS_VIDEO_DECODE == hw_engines[i].engine_class)
        {
            engine_class_num++;
        }
    }
    if (engine_class_num >= 1)
    {
        drvInfo->hasBsd = 1;
    }
    if (engine_class_num >= 2)
    {
        drvInfo->hasBsd2 = 1;
    }

    for (i = 0, engine_class_num = 0; i < bufmgr_gem->number_hw_engines; i++)
    {
        if (DRM_XE_ENGINE_CLASS_VIDEO_DECODE == hw_engines[i].engine_class)
        {
            engine_class_num++;
        }
    }
    if (engine_class_num  >= 1)
    {
        drvInfo->hasVebox = 1;
    }

    // Step3: Get config
    if (nullptr == bufmgr_gem->config)
    {
        bufmgr_gem->config = __mos_query_config_xe(bufmgr_gem->fd);
        if (nullptr == bufmgr_gem->config)
        {
            MOS_DRM_ASSERTMESSAGE("get config failed");
            return -ENODEV;
        }
    }
    config = bufmgr_gem->config;

    drvInfo->hasHuc = 1;
    if (1 == drvInfo->hasHuc)
    {
        drvInfo->hasProtectedHuc = 1;
    }

    drvInfo->devId = config->info[XE_QUERY_CONFIG_REV_AND_DEVICE_ID] & 0xffff;
    drvInfo->devRev = config->info[XE_QUERY_CONFIG_REV_AND_DEVICE_ID] >> 16;

    return 0;
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
    if(__xe_bufmgr_debug__ < 0)
    {
        __xe_bufmgr_debug__ = 0;
    }
#endif

    struct mos_xe_bufmgr_gem *bufmgr_gem;
    int ret, tmp;
    int32_t sync_mode = MOS_SYNC_TIMELINE; //current default mode

    pthread_mutex_lock(&bufmgr_list_mutex);

    bufmgr_gem = mos_bufmgr_gem_find(fd);
    if (bufmgr_gem)
        goto exit;

    bufmgr_gem = MOS_New(mos_xe_bufmgr_gem);
    if (nullptr == bufmgr_gem)
        goto exit;

    bufmgr_gem->bufmgr = {};

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
    bufmgr_gem->bufmgr.query_engines = mos_query_engines_class_xe;
    bufmgr_gem->bufmgr.get_engine_class_size = mos_get_engine_class_size_xe;
    bufmgr_gem->bufmgr.query_sys_engines = mos_query_sys_engines_xe;
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

    MOS_READ_ENV_VARIABLE(INTEL_SYNCHRONIZATION_MODE, MOS_USER_FEATURE_VALUE_TYPE_INT32, sync_mode);

    if(sync_mode != MOS_SYNC_NONE)
    {
        if(sync_mode != MOS_SYNC_SYNCOBJ)
        {
            sync_mode = MOS_SYNC_TIMELINE; //invalid setting, use default mode
        }
        bufmgr_gem->bufmgr.bo_context_exec3 = mos_bo_context_exec_with_sync_xe;
    }
    else
    {
        bufmgr_gem->bufmgr.bo_context_exec3 = mos_bo_context_exec_xe;
    }
    mos_sync_set_synchronization_mode(sync_mode);
    MOS_DRM_NORMALMESSAGE("exec with synchronization mode: %d", sync_mode);
    bufmgr_gem->is_defer_creation_and_binding = false;
    MOS_READ_ENV_VARIABLE(INTEL_DEFER_CREATION_AND_BINDING, MOS_USER_FEATURE_VALUE_TYPE_BOOL, bufmgr_gem->is_defer_creation_and_binding);

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

    bufmgr_gem->vm_id = __mos_vm_create_xe(&bufmgr_gem->bufmgr);
    bufmgr_gem->config = __mos_query_config_xe(fd);
    bufmgr_gem->gt_list = __mos_query_gt_list_xe(fd);
    bufmgr_gem->memory_regions = __mos_query_memory_regions_xe(fd);
    bufmgr_gem->mem_usage = __mos_query_mem_usage_xe(fd);
    bufmgr_gem->has_vram = __mos_has_vram_xe(fd);
    bufmgr_gem->hw_config = __mos_query_hw_config_xe(fd, &bufmgr_gem->config_len);

    if (bufmgr_gem->mem_usage != nullptr)
    {
        __mos_get_default_alignment_xe(&bufmgr_gem->bufmgr, bufmgr_gem->mem_usage);
    }

    if (bufmgr_gem->gt_list != nullptr)
    {
        bufmgr_gem->number_gt = bufmgr_gem->gt_list->num_gt;
    }

    bufmgr_gem->hw_engines = __mos_query_engines_xe(fd, &bufmgr_gem->number_hw_engines);
    if (0 == bufmgr_gem->number_hw_engines || nullptr == bufmgr_gem->hw_engines)
    {
        MOS_DRM_ASSERTMESSAGE("Failed to query engines");

        if (bufmgr_gem->mem_profiler_fd != -1)
        {
            close(bufmgr_gem->mem_profiler_fd);
        }

        MOS_XE_SAFE_FREE(bufmgr_gem->hw_config)
        bufmgr_gem->hw_config = nullptr;

        MOS_XE_SAFE_FREE(bufmgr_gem->config);
        bufmgr_gem->config = nullptr;

        MOS_XE_SAFE_FREE(bufmgr_gem->mem_usage);
        bufmgr_gem->mem_usage = nullptr;

        MOS_XE_SAFE_FREE(bufmgr_gem->gt_list);
        bufmgr_gem->gt_list = nullptr;

        MOS_Delete(bufmgr_gem);
        goto exit;
    }

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
    struct drm_xe_query_config *config = nullptr;
    bool needFreeConfig = false;
    pthread_mutex_lock(&bufmgr_list_mutex);
    struct mos_xe_bufmgr_gem *bufmgr_gem = mos_bufmgr_gem_find(fd);
    pthread_mutex_unlock(&bufmgr_list_mutex);
    if (bufmgr_gem != nullptr)
    {
        if (nullptr == bufmgr_gem->config)
        {
            bufmgr_gem->config = __mos_query_config_xe(fd);
        }
        config = bufmgr_gem->config;
        needFreeConfig = false;
    }
    else
    {
        config = __mos_query_config_xe(fd);
        needFreeConfig = true;
    }

    if (nullptr == config)
    {
        MOS_DRM_ASSERTMESSAGE("get config failed\n");
        mos_bufmgr_gem_unref_xe((struct mos_bufmgr *)bufmgr_gem);
        return -ENODEV;
    }
    *device_id = config->info[XE_QUERY_CONFIG_REV_AND_DEVICE_ID] & 0xffff;

    mos_bufmgr_gem_unref_xe((struct mos_bufmgr *)bufmgr_gem);
    if (needFreeConfig)
    {
        MOS_XE_SAFE_FREE(config);
    }
    return 0;
}
