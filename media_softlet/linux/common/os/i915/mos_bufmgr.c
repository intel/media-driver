/**************************************************************************
 *
 * Copyright © 2007 Red Hat Inc.
 * Copyright © 2007-2022 Intel Corporation
 * Copyright 2006 Tungsten Graphics, Inc., Bismarck, ND., USA
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 *
 **************************************************************************/
/*
 * Authors: Thomas Hellström <thomas-at-tungstengraphics-dot-com>
 *          Keith Whitwell <keithw-at-tungstengraphics-dot-com>
 *        Eric Anholt <eric@anholt.net>
 *        Dave Airlie <airlied@linux.ie>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86drm.h"
#include "xf86atomic.h"
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

#include "errno.h"
#ifndef ETIME
#define ETIME ETIMEDOUT
#endif
#include "libdrm_macros.h"
#include "libdrm_lists.h"
#include "mos_bufmgr.h"
#include "mos_bufmgr_priv.h"
#ifdef ENABLE_XE_KMD
#include "mos_bufmgr_xe.h"
#endif
#include "string.h"

#include "i915_drm.h"
#include "mos_vma.h"
#include "mos_util_debug.h"
#include "mos_oca_defs_specific.h"
#include "intel_hwconfig_types.h"
#include "mos_utilities.h"
#include "linux_system_info.h"
#include "mos_os_specific.h"

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#define VG(x) x
#else
#define VG(x)
#endif

#define memclear(s) memset(&s, 0, sizeof(s))

#define MOS_DBG(...) do {                                             \
    if (bufmgr_gem != nullptr && bufmgr_gem->bufmgr.debug)            \
        fprintf(stderr, __VA_ARGS__);                                 \
} while (0)

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define MAX2(A, B) ((A) > (B) ? (A) : (B))

/**
 * upper_32_bits - return bits 32-63 of a number
 * @n: the number we're accessing
 *
 * A basic shift-right of a 64- or 32-bit quantity.  Use this to suppress
 * the "right shift count >= width of type" warning when that quantity is
 * 32-bits.
 */
#define upper_32_bits(n) ((__u32)(((n) >> 16) >> 16))

/**
 * lower_32_bits - return bits 0-31 of a number
 * @n: the number we're accessing
 */
#define lower_32_bits(n) ((__u32)(n))

#define PCI_CHIP_I915_G         0x2582
#define PCI_CHIP_E7221_G        0x258A
#define PCI_CHIP_I915_GM        0x2592

#define IS_915(devid)        ((devid) == PCI_CHIP_I915_G || \
                 (devid) == PCI_CHIP_E7221_G || \
                 (devid) == PCI_CHIP_I915_GM)

#define INITIAL_SOFTPIN_TARGET_COUNT  1024

struct mos_gem_bo_bucket {
    drmMMListHead head;
    unsigned long size;
};

struct mos_bufmgr_gem {
    struct mos_bufmgr bufmgr;

    atomic_t refcount;

    int fd;

    int max_relocs;

    pthread_mutex_t lock;

    struct drm_i915_gem_exec_object *exec_objects;
    struct drm_i915_gem_exec_object2 *exec2_objects;
    struct mos_linux_bo **exec_bos;
    int exec_size;
    int exec_count;

    /** Array of lists of cached gem objects of power-of-two sizes */
    struct mos_gem_bo_bucket cache_bucket[64];
    int num_buckets;
    time_t time;

    drmMMListHead managers;

    drmMMListHead named;

    uint64_t gtt_size;
    int available_fences;
    int pci_device;
    unsigned int has_bsd : 1;
    unsigned int has_bsd2 : 1;
    unsigned int has_blt : 1;
    unsigned int has_relaxed_fencing : 1;
    unsigned int has_llc : 1;
    unsigned int has_wait_timeout : 1;
    unsigned int bo_reuse : 1;
    unsigned int no_exec : 1;
    unsigned int has_vebox : 1;
    unsigned int has_ext_mmap : 1;
    unsigned int has_fence_reg : 1;
    unsigned int has_lmem : 1;
    unsigned int has_mmap_offset : 1;
    bool fenced_relocs;

    struct {
        void *ptr;
        uint32_t handle;
    } userptr_active;

    // manage address for softpin buffer object
    mos_vma_heap vma_heap[MEMZONE_COUNT];
    bool use_softpin;
    bool softpin_va1Malign;

    bool object_capture_disabled;

    #define MEM_PROFILER_BUFFER_SIZE 256
    char mem_profiler_buffer[MEM_PROFILER_BUFFER_SIZE];
    char* mem_profiler_path;
    int mem_profiler_fd;

    int device_type;

    uint32_t ts_freq;
} mos_bufmgr_gem;

#define DRM_INTEL_RELOC_FENCE (1<<0)

struct mos_reloc_target {
    struct mos_linux_bo *bo;
    int flags;
};

struct mos_softpin_target {
    struct mos_linux_bo *bo;
    int flags;
};

struct mos_bo_gem {
    struct mos_linux_bo bo;

    atomic_t refcount;
    uint32_t gem_handle;
    const char *name;

    /**
     * Kenel-assigned global name for this object
         *
         * List contains both flink named and prime fd'd objects
     */
    unsigned int global_name;
    drmMMListHead name_list;

    /**
     * Index of the buffer within the validation list while preparing a
     * batchbuffer execution.
     */
    int validate_index;

    /**
     * Current tiling mode
     */
    uint32_t tiling_mode;
    uint32_t swizzle_mode;
    unsigned long stride;

    time_t free_time;

    /** Array passed to the DRM containing relocation information. */
    struct drm_i915_gem_relocation_entry *relocs;
    /**
     * Array of info structs corresponding to relocs[i].target_handle etc
     */
    struct mos_reloc_target *reloc_target_info;
    /** Number of entries in relocs */
    int reloc_count;
    /** Array of BOs that are referenced by this buffer and will be softpinned */
    struct mos_softpin_target *softpin_target;
    /** Number softpinned BOs that are referenced by this buffer */
    int softpin_target_count;
    /** Maximum amount of softpinned BOs that are referenced by this buffer */
    int max_softpin_target_count;

    /** Mapped address for the buffer, saved across map/unmap cycles */
    void *mem_virtual;
    /** Uncached Mapped address for the buffer, saved across map/unmap cycles */
    void *mem_wc_virtual;
    /** GTT virtual address for the buffer, saved across map/unmap cycles */
    void *gtt_virtual;
    /**
     * Virtual address of the buffer allocated by user, used for userptr
     * objects only.
     */
    void *user_virtual;
    int map_count;

    /** BO cache list */
    drmMMListHead head;

    /**
     * Boolean of whether this BO and its children have been included in
     * the current drm_intel_bufmgr_check_aperture_space() total.
     */
    bool included_in_check_aperture;

    /**
     * Boolean of whether this buffer has been used as a relocation
     * target and had its size accounted for, and thus can't have any
     * further relocations added to it.
     */
    bool used_as_reloc_target;

    /**
     * Boolean of whether we have encountered an error whilst building the relocation tree.
     */
    bool has_error;

    /**
     * Boolean of whether this buffer can be re-used
     */
    bool reusable;

    /**
     * Boolean of whether the GPU is definitely not accessing the buffer.
     *
     * This is only valid when reusable, since non-reusable
     * buffers are those that have been shared wth other
     * processes, so we don't know their state.
     */
    bool idle;

    /**
     * Boolean of whether this buffer was allocated with userptr
     */
    bool is_userptr;

    /**
     * Boolean of whether this buffer can be placed in the full 48-bit
     * address range on gen8+.
     *
     * By default, buffers will be keep in a 32-bit range, unless this
     * flag is explicitly set.
     */
    bool use_48b_address_range;

    /**
     * Whether this buffer is softpinned at offset specified by the user
     */
    bool is_softpin;

    /*
    * Whether to remove the dependency of this bo in exebuf.
    */
    bool exec_async;

    /*
    * Whether to remove the dependency of this bo in exebuf.
    */
    bool exec_capture;

    /**
     * Size in bytes of this buffer and its relocation descendents.
     *
     * Used to avoid costly tree walking in
     * drm_intel_bufmgr_check_aperture in the common case.
     */
    int reloc_tree_size;

    /**
     * Number of potential fence registers required by this buffer and its
     * relocations.
     */
    int reloc_tree_fences;

    /** Flags that we may need to do the SW_FINSIH ioctl on unmap. */
    bool mapped_cpu_write;

    /**
     * Size to pad the object to.
     *
     */
    uint64_t pad_to_size;

    /**
     * Memory Type on created the surfaces for local/system memory
     */
    int mem_region;

    /**
     * PAT Index
     */
    unsigned int pat_index;

    /**
     * Is cpu cacheable
     */
    bool cpu_cacheable;
};

struct mos_exec_info {
    /* save all exec2_objects for res*/
    struct drm_i915_gem_exec_object2* obj;
    /* save batch buffer*/
    struct drm_i915_gem_exec_object2* batch_obj;
    /* save previous ptr to void mem leak when free*/
    struct drm_i915_gem_exec_object2* pSavePreviousExec2Objects;
    /*bo resource count*/
    uint32_t obj_count;
    /*batch buffer bo count*/
    uint32_t batch_count;
    /*remain size of 'obj'*/
    uint32_t obj_remain_size;
#define      OBJ512_SIZE    512
};

static unsigned int
mos_gem_estimate_batch_space(struct mos_linux_bo ** bo_array, int count);

static unsigned int
mos_gem_compute_batch_space(struct mos_linux_bo ** bo_array, int count);

static int
mos_gem_bo_get_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
                uint32_t * swizzle_mode);

static int
mos_gem_bo_check_mem_region_internal(struct mos_linux_bo *bo,
                     int mem_type);

static int
mos_gem_bo_set_tiling_internal(struct mos_linux_bo *bo,
                     uint32_t tiling_mode,
                     uint32_t stride);

static void mos_gem_bo_unreference_locked_timed(struct mos_linux_bo *bo,
                              time_t time);

static void mos_gem_bo_unreference(struct mos_linux_bo *bo);
static bool mos_gem_bo_is_softpin(struct mos_linux_bo *bo);
static void mos_gem_bo_start_gtt_access(struct mos_linux_bo *bo, int write_enable);
static void mos_gem_bo_free(struct mos_linux_bo *bo);

static int mos_bufmgr_get_driver_info(struct mos_bufmgr *bufmgr, struct LinuxDriverInfo *drvInfo);

static inline struct mos_bo_gem *to_bo_gem(struct mos_linux_bo *bo)
{
        return (struct mos_bo_gem *)bo;
}

static unsigned long
mos_gem_bo_tile_size(struct mos_bufmgr_gem *bufmgr_gem, unsigned long size,
               uint32_t *tiling_mode)
{
    unsigned long min_size, max_size;
    unsigned long i;

    if (*tiling_mode == I915_TILING_NONE)
        return size;

    /* 965+ just need multiples of page size for tiling */
    return ROUND_UP_TO(size, 4096);

}

/*
 * Round a given pitch up to the minimum required for X tiling on a
 * given chip.  We use 512 as the minimum to allow for a later tiling
 * change.
 */
static unsigned long
mos_gem_bo_tile_pitch(struct mos_bufmgr_gem *bufmgr_gem,
                unsigned long pitch, uint32_t *tiling_mode)
{
    unsigned long tile_width;
    unsigned long i;

    /* If untiled, then just align it so that we can do rendering
     * to it with the 3D engine.
     */
    if (*tiling_mode == I915_TILING_NONE)
        return ALIGN(pitch, 64);

    if (*tiling_mode == I915_TILING_X
            || (IS_915(bufmgr_gem->pci_device)
                && *tiling_mode == I915_TILING_Y))
        tile_width = 512;
    else
        tile_width = 128;

    /* 965 is flexible */
    return ROUND_UP_TO(pitch, tile_width);
}

static struct mos_gem_bo_bucket *
mos_gem_bo_bucket_for_size(struct mos_bufmgr_gem *bufmgr_gem,
                 unsigned long size)
{
    int i;

    for (i = 0; i < bufmgr_gem->num_buckets; i++) {
        struct mos_gem_bo_bucket *bucket =
            &bufmgr_gem->cache_bucket[i];
        if (bucket->size >= size) {
            return bucket;
        }
    }

    return nullptr;
}

static void
mos_gem_dump_validation_list(struct mos_bufmgr_gem *bufmgr_gem)
{
    int i, j;

    for (i = 0; i < bufmgr_gem->exec_count; i++) {
        struct mos_linux_bo *bo = bufmgr_gem->exec_bos[i];
        struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

        if (bo_gem->relocs == nullptr || bo_gem->softpin_target == nullptr) {
            MOS_DBG("%2d: %d %s(%s)\n", i, bo_gem->gem_handle,
                bo_gem->is_softpin ? "*" : "",
                bo_gem->name);
            continue;
        }

        for (j = 0; j < bo_gem->reloc_count; j++) {
            struct mos_linux_bo *target_bo = bo_gem->reloc_target_info[j].bo;
            struct mos_bo_gem *target_gem =
                (struct mos_bo_gem *) target_bo;

            MOS_DBG("%2d: %d %s(%s)@0x%08x %08x -> "
                "%d (%s)@0x%08x %08x + 0x%08x\n",
                i,
                bo_gem->gem_handle,
                bo_gem->is_softpin ? "*" : "",
                bo_gem->name,
                upper_32_bits(bo_gem->relocs[j].offset),
                lower_32_bits(bo_gem->relocs[j].offset),
                target_gem->gem_handle,
                target_gem->name,
                upper_32_bits(target_bo->offset64),
                lower_32_bits(target_bo->offset64),
                bo_gem->relocs[j].delta);
        }

        for (j = 0; j < bo_gem->softpin_target_count; j++) {
            struct mos_linux_bo *target_bo = bo_gem->softpin_target[j].bo;
            struct mos_bo_gem *target_gem =
                (struct mos_bo_gem *) target_bo;
            MOS_DBG("%2d: %d %s(%s) -> "
                "%d *(%s)@0x%08x %08x\n",
                i,
                bo_gem->gem_handle,
                bo_gem->is_softpin ? "*" : "",
                bo_gem->name,
                target_gem->gem_handle,
                target_gem->name,
                upper_32_bits(target_bo->offset64),
                lower_32_bits(target_bo->offset64));
        }
    }
}

static inline void
mos_gem_bo_reference(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    atomic_inc(&bo_gem->refcount);
}

/**
 * Adds the given buffer to the list of buffers to be validated (moved into the
 * appropriate memory type) with the next batch submission.
 *
 * If a buffer is validated multiple times in a batch submission, it ends up
 * with the intersection of the memory type flags and the union of the
 * access flags.
 */
static void
mos_add_validate_buffer(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int index;
    struct drm_i915_gem_exec_object *exec_objects;
    struct mos_linux_bo **exec_bos;

    if (bo_gem->validate_index != -1)
        return;

    /* Extend the array of validation entries as necessary. */
    if (bufmgr_gem->exec_count == bufmgr_gem->exec_size) {
        int new_size = bufmgr_gem->exec_size * 2;

        if (new_size == 0)
            new_size = ARRAY_INIT_SIZE;

        exec_objects = (struct drm_i915_gem_exec_object *)realloc(bufmgr_gem->exec_objects,
                sizeof(*bufmgr_gem->exec_objects) * new_size);
        if (!exec_objects)
            return;

        bufmgr_gem->exec_objects = exec_objects;

        exec_bos = (struct mos_linux_bo **)realloc(bufmgr_gem->exec_bos,
                sizeof(*bufmgr_gem->exec_bos) * new_size);
        if (!exec_bos)
            return;

        bufmgr_gem->exec_bos = exec_bos;
        bufmgr_gem->exec_size = new_size;
    }

    index = bufmgr_gem->exec_count;
    bo_gem->validate_index = index;
    /* Fill in array entry */
    bufmgr_gem->exec_objects[index].handle = bo_gem->gem_handle;
    bufmgr_gem->exec_objects[index].relocation_count = bo_gem->reloc_count;
    bufmgr_gem->exec_objects[index].relocs_ptr = (uintptr_t) bo_gem->relocs;
    bufmgr_gem->exec_objects[index].alignment = bo->align;
    bufmgr_gem->exec_objects[index].offset = 0;
    bufmgr_gem->exec_bos[index] = bo;
    bufmgr_gem->exec_count++;
}

static void
mos_add_validate_buffer2(struct mos_linux_bo *bo, int need_fence)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *)bo;
    int index;
    struct drm_i915_gem_exec_object2 *exec2_objects;
    struct mos_linux_bo **exec_bos;
    int flags = 0;

    if (need_fence)
        flags |= EXEC_OBJECT_NEEDS_FENCE;
    if (bo_gem->pad_to_size)
        flags |= EXEC_OBJECT_PAD_TO_SIZE;
    if (bo_gem->use_48b_address_range)
        flags |= EXEC_OBJECT_SUPPORTS_48B_ADDRESS;
    if (bo_gem->is_softpin)
        flags |= EXEC_OBJECT_PINNED;
    if (bo_gem->exec_async)
        flags |= EXEC_OBJECT_ASYNC;
    if (bo_gem->exec_capture)
        flags |= EXEC_OBJECT_CAPTURE;

    if (bo_gem->validate_index != -1) {
        bufmgr_gem->exec2_objects[bo_gem->validate_index].flags |= flags;
        return;
    }

    /* Extend the array of validation entries as necessary. */
    if (bufmgr_gem->exec_count == bufmgr_gem->exec_size) {
        int new_size = bufmgr_gem->exec_size * 2;

        if (new_size == 0)
            new_size = ARRAY_INIT_SIZE;
        exec2_objects = (struct drm_i915_gem_exec_object2 *)
                realloc(bufmgr_gem->exec2_objects,
                    sizeof(*bufmgr_gem->exec2_objects) * new_size);
        if (!exec2_objects)
            return;

        bufmgr_gem->exec2_objects = exec2_objects;

        exec_bos = (struct mos_linux_bo **)realloc(bufmgr_gem->exec_bos,
                sizeof(*bufmgr_gem->exec_bos) * new_size);
        if (!exec_bos)
            return;

        bufmgr_gem->exec_bos = exec_bos;
        bufmgr_gem->exec_size = new_size;
    }

    index = bufmgr_gem->exec_count;
    bo_gem->validate_index = index;
    /* Fill in array entry */
    bufmgr_gem->exec2_objects[index].handle = bo_gem->gem_handle;
    bufmgr_gem->exec2_objects[index].relocation_count = bo_gem->reloc_count;
    bufmgr_gem->exec2_objects[index].relocs_ptr = (uintptr_t)bo_gem->relocs;
    bufmgr_gem->exec2_objects[index].alignment = bo->align;
    bufmgr_gem->exec2_objects[index].offset = bo_gem->is_softpin ?
        bo->offset64 : 0;
    bufmgr_gem->exec_bos[index] = bo;
    bufmgr_gem->exec2_objects[index].flags = flags;
    bufmgr_gem->exec2_objects[index].rsvd1 = 0;
    bufmgr_gem->exec2_objects[index].pad_to_size = bo_gem->pad_to_size;
    bufmgr_gem->exec2_objects[index].rsvd2 = 0;
    bufmgr_gem->exec_count++;
}

static void
mos_add_reloc_objects(struct mos_reloc_target reloc_target)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)reloc_target.bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *)reloc_target.bo;
    int index;
    struct drm_i915_gem_exec_object2 *exec2_objects;
    struct mos_linux_bo **exec_bos;

    if (bo_gem->validate_index != -1) {
        bufmgr_gem->exec2_objects[bo_gem->validate_index].flags |= reloc_target.flags;
        return;
    }

    /* Extend the array of validation entries as necessary. */
    if (bufmgr_gem->exec_count == bufmgr_gem->exec_size) {
        int new_size = bufmgr_gem->exec_size * 2;

        if (new_size == 0)
            new_size = ARRAY_INIT_SIZE;
        exec2_objects = (struct drm_i915_gem_exec_object2 *)
                realloc(bufmgr_gem->exec2_objects,
                    sizeof(*bufmgr_gem->exec2_objects) * new_size);
        if (!exec2_objects)
        {
            MOS_DBG("realloc exec2_objects failed!\n");
            return;
        }

        bufmgr_gem->exec2_objects = exec2_objects;

        exec_bos = (struct mos_linux_bo **)realloc(bufmgr_gem->exec_bos,
                sizeof(*bufmgr_gem->exec_bos) * new_size);
        if (!exec_bos)
        {
            MOS_DBG("realloc exec_bo failed!\n");
            return;
        }

        bufmgr_gem->exec_bos = exec_bos;
        bufmgr_gem->exec_size = new_size;
    }

    index = bufmgr_gem->exec_count;
    bo_gem->validate_index = index;
    /* Fill in array entry */
    bufmgr_gem->exec2_objects[index].handle           = bo_gem->gem_handle;
    bufmgr_gem->exec2_objects[index].relocation_count = bo_gem->reloc_count;
    bufmgr_gem->exec2_objects[index].relocs_ptr       = (uintptr_t)bo_gem->relocs;
    bufmgr_gem->exec2_objects[index].alignment        = reloc_target.bo->align;
    bufmgr_gem->exec2_objects[index].offset           = 0;
    bufmgr_gem->exec_bos[index]                       = reloc_target.bo;
    bufmgr_gem->exec2_objects[index].flags            = reloc_target.flags;
    bufmgr_gem->exec2_objects[index].rsvd1            = 0;
    bufmgr_gem->exec2_objects[index].pad_to_size      = bo_gem->pad_to_size;
    bufmgr_gem->exec2_objects[index].rsvd2            = 0;
    bufmgr_gem->exec_count++;
}

static void
mos_add_softpin_objects(struct mos_softpin_target softpin_target)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)softpin_target.bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *)softpin_target.bo;
    int index;
    struct drm_i915_gem_exec_object2 *exec2_objects;
    struct mos_linux_bo **exec_bos;

    if (bo_gem->validate_index != -1) {
        bufmgr_gem->exec2_objects[bo_gem->validate_index].flags |= softpin_target.flags;
        return;
    }

    /* Extend the array of validation entries as necessary. */
    if (bufmgr_gem->exec_count == bufmgr_gem->exec_size) {
        int new_size = bufmgr_gem->exec_size * 2;

        if (new_size == 0)
            new_size = ARRAY_INIT_SIZE;
        exec2_objects = (struct drm_i915_gem_exec_object2 *)
                realloc(bufmgr_gem->exec2_objects,
                    sizeof(*bufmgr_gem->exec2_objects) * new_size);
        if (!exec2_objects)
        {
            MOS_DBG("realloc exec2_objects failed!\n");
            return;
        }

        bufmgr_gem->exec2_objects = exec2_objects;

        exec_bos = (struct mos_linux_bo **)realloc(bufmgr_gem->exec_bos,
                sizeof(*bufmgr_gem->exec_bos) * new_size);
        if (!exec_bos)
        {
            MOS_DBG("realloc exec_bo failed!\n");
            return;
        }

        bufmgr_gem->exec_bos = exec_bos;
        bufmgr_gem->exec_size = new_size;
    }

    index = bufmgr_gem->exec_count;
    bo_gem->validate_index = index;
    /* Fill in array entry */
    bufmgr_gem->exec2_objects[index].handle           = bo_gem->gem_handle;
    bufmgr_gem->exec2_objects[index].relocation_count = bo_gem->reloc_count;
    bufmgr_gem->exec2_objects[index].relocs_ptr       = (uintptr_t)bo_gem->relocs;
    bufmgr_gem->exec2_objects[index].alignment        = softpin_target.bo->align;
    bufmgr_gem->exec2_objects[index].offset           = softpin_target.bo->offset64;
    bufmgr_gem->exec2_objects[index].flags            = softpin_target.flags;
    bufmgr_gem->exec2_objects[index].pad_to_size      = bo_gem->pad_to_size;
    bufmgr_gem->exec2_objects[index].rsvd1            = 0;
    bufmgr_gem->exec2_objects[index].rsvd2            = 0;
    bufmgr_gem->exec_bos[index]                       = softpin_target.bo;
    bufmgr_gem->exec_count++;
}

#define RELOC_BUF_SIZE(x) ((I915_RELOC_HEADER + x * I915_RELOC0_STRIDE) * \
    sizeof(uint32_t))

static void
mos_bo_gem_set_in_aperture_size(struct mos_bufmgr_gem *bufmgr_gem,
                      struct mos_bo_gem *bo_gem,
                      unsigned int alignment)
{
    unsigned int size;

    assert(!bo_gem->used_as_reloc_target);

    /* The older chipsets are far-less flexible in terms of tiling,
     * and require tiled buffer to be size aligned in the aperture.
     * This means that in the worst possible case we will need a hole
     * twice as large as the object in order for it to fit into the
     * aperture. Optimal packing is for wimps.
     */
    size = bo_gem->bo.size;

    bo_gem->reloc_tree_size = size + alignment;
}

static int
mos_setup_reloc_list(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    unsigned int max_relocs = bufmgr_gem->max_relocs;

    if (bo->size / 4 < max_relocs)
        max_relocs = bo->size / 4;

    bo_gem->relocs = (struct drm_i915_gem_relocation_entry *)malloc(max_relocs *
                sizeof(struct drm_i915_gem_relocation_entry));
    bo_gem->reloc_target_info = (struct mos_reloc_target *)malloc(max_relocs *
                       sizeof(struct mos_reloc_target));
    if (bo_gem->relocs == nullptr || bo_gem->reloc_target_info == nullptr) {
        bo_gem->has_error = true;

        free (bo_gem->relocs);
        bo_gem->relocs = nullptr;

        free (bo_gem->reloc_target_info);
        bo_gem->reloc_target_info = nullptr;

        return 1;
    }

    return 0;
}

static int
mos_gem_bo_busy(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_busy busy;
    int ret;

    if (bo_gem->reusable && bo_gem->idle)
        return false;

    memclear(busy);
    busy.handle = bo_gem->gem_handle;

    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_BUSY, &busy);
    if (ret == 0) {
        bo_gem->idle = !busy.busy;
        return busy.busy;
    } else {
        return false;
    }
    return (ret == 0 && busy.busy);
}

static int
mos_gem_bo_madvise_internal(struct mos_bufmgr_gem *bufmgr_gem,
                 struct mos_bo_gem *bo_gem, int state)
{
    struct drm_i915_gem_madvise madv;

    memclear(madv);
    madv.handle = bo_gem->gem_handle;
    madv.madv = state;
    madv.retained = 1;
    drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_MADVISE, &madv);

    return madv.retained;
}

static int
mos_gem_bo_madvise(struct mos_linux_bo *bo, int madv)
{
    return mos_gem_bo_madvise_internal
        ((struct mos_bufmgr_gem *) bo->bufmgr,
         (struct mos_bo_gem *) bo,
         madv);
}

/* drop the oldest entries that have been purged by the kernel */
static void
mos_gem_bo_cache_purge_bucket(struct mos_bufmgr_gem *bufmgr_gem,
                    struct mos_gem_bo_bucket *bucket)
{
    while (!DRMLISTEMPTY(&bucket->head)) {
        struct mos_bo_gem *bo_gem;

        bo_gem = DRMLISTENTRY(struct mos_bo_gem,
                      bucket->head.next, head);
        if (mos_gem_bo_madvise_internal
            (bufmgr_gem, bo_gem, I915_MADV_DONTNEED))
            break;

        DRMLISTDEL(&bo_gem->head);
        mos_gem_bo_free(&bo_gem->bo);
    }
}

static int
mos_gem_query_items(int fd, struct drm_i915_query_item *items, uint32_t n_items)
{
    struct drm_i915_query q;

    memclear(q);
    q.num_items = n_items;
    q.items_ptr = (uintptr_t)items;

    return drmIoctl(fd, DRM_IOCTL_I915_QUERY, &q);
}

/**
 * query mechanism for memory regions.
 */
static struct drm_i915_query_memory_regions *mos_gem_get_query_memory_regions(int fd)
{
    int ret;
    struct drm_i915_query_item item;
    struct drm_i915_query_memory_regions *query_info;

    memclear(item);
    item.query_id = DRM_I915_QUERY_MEMORY_REGIONS;
    ret = mos_gem_query_items(fd, &item, 1);
    if (ret != 0 || item.length <= 0)
        return NULL;

    query_info = (drm_i915_query_memory_regions*)calloc(1, item.length);

    item.data_ptr = (uintptr_t)query_info;
    ret = mos_gem_query_items(fd, &item, 1);
    if (ret != 0) {
        free(query_info);
        return NULL;
    }

    return query_info;
}

/**
 * check how many lmem regions are available on device.
 */
static uint8_t mos_gem_get_lmem_region_count(int fd)
{
    struct drm_i915_query_memory_regions *query_info;
    uint8_t num_regions = 0;
    uint8_t lmem_regions = 0;

    query_info = mos_gem_get_query_memory_regions(fd);

    if(query_info)
    {
        num_regions = query_info->num_regions;

        for (int i = 0; i < num_regions; i++) {
            if (query_info->regions[i].region.memory_class == I915_MEMORY_CLASS_DEVICE)
            {
                lmem_regions += 1;
            }
        }
        free(query_info);
    }

    return lmem_regions;
}

/**
 * check if lmem is available on device.
 */
static bool mos_gem_has_lmem(int fd)
{
    return mos_gem_get_lmem_region_count(fd) > 0;
}

static enum mos_memory_zone
mos_gem_bo_memzone_for_address(uint64_t address)
{
    if (address >= MEMZONE_DEVICE_START)
        return MEMZONE_DEVICE;
    else 
        return MEMZONE_SYS;
}

/**
 * Allocate a section of virtual memory for a buffer, assigning an address.
 */
static uint64_t
mos_gem_bo_vma_alloc(struct mos_bufmgr *bufmgr,
          enum mos_memory_zone memzone,
          uint64_t size,
          uint64_t alignment)
{
    CHK_CONDITION(bufmgr == nullptr, "nullptr bufmgr.\n", 0);
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;
    /* Force alignment to be some number of pages */
    alignment = ALIGN(alignment, PAGE_SIZE);

    uint64_t addr = mos_vma_heap_alloc(&bufmgr_gem->vma_heap[memzone], size, alignment);

    // currently only support 48bit range address
    CHK_CONDITION((addr >> 48ull) != 0, "invalid address, over 48bit range.\n", 0);
    CHK_CONDITION((addr >> (memzone == MEMZONE_SYS ? 40ull : 41ull)) != 0, "invalid address, over memory zone range.\n", 0);
    CHK_CONDITION((addr % alignment) != 0, "invalid address, not meet aligment requirement.\n", 0);

    return addr;
}

static void
mos_gem_bo_vma_free(struct mos_bufmgr *bufmgr,
         uint64_t address,
         uint64_t size)
{
    CHK_CONDITION(bufmgr == nullptr, "nullptr bufmgr.\n", );
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;

    CHK_CONDITION(address == 0ull, "invalid address.\n", );
    enum mos_memory_zone memzone = mos_gem_bo_memzone_for_address(address);
    mos_vma_heap_free(&bufmgr_gem->vma_heap[memzone], address, size);
}

drm_export struct mos_linux_bo *
mos_gem_bo_alloc_internal(struct mos_bufmgr *bufmgr,
                struct mos_drm_bo_alloc *alloc)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;
    struct mos_bo_gem *bo_gem;
    unsigned int page_size = getpagesize();
    static bool support_pat_index = true;
    int ret;
    struct mos_gem_bo_bucket *bucket;
    bool alloc_from_cache;
    unsigned long bo_size;
    bool for_render = false;

    if (alloc->ext.flags & BO_ALLOC_FOR_RENDER)
        for_render = true;

    /* Round the allocated size up to a power of two number of pages. */
    bucket = mos_gem_bo_bucket_for_size(bufmgr_gem, alloc->size);

    /* If we don't have caching at this size, don't actually round the
     * allocation up.
     */
    if (bucket == nullptr) {
        bo_size = alloc->size;
        if (bo_size < page_size)
            bo_size = page_size;
    } else {
        bo_size = bucket->size;
    }
    if (!support_pat_index)
    {
        /* For old kernel without pat index support,
         * We need to reset pat_index for bo reuse policy
         */
        alloc->ext.pat_index = PAT_INDEX_INVALID;
    }
    pthread_mutex_lock(&bufmgr_gem->lock);
    /* Get a buffer out of the cache if available */
retry:
    alloc_from_cache = false;
    if (bucket != nullptr && !DRMLISTEMPTY(&bucket->head)) {
        if (for_render) {
            /* Allocate new render-target BOs from the tail (MRU)
             * of the list, as it will likely be hot in the GPU
             * cache and in the aperture for us.
             */
            bo_gem = DRMLISTENTRY(struct mos_bo_gem,
                          bucket->head.prev, head);
            DRMLISTDEL(&bo_gem->head);
            alloc_from_cache = true;
            bo_gem->bo.align = alloc->alignment;
        } else {
            assert(alloc->alignment == 0);
            /* For non-render-target BOs (where we're probably
             * going to map it first thing in order to fill it
             * with data), check if the last BO in the cache is
             * unbusy, and only reuse in that case. Otherwise,
             * allocating a new buffer is probably faster than
             * waiting for the GPU to finish.
             */
            bo_gem = DRMLISTENTRY(struct mos_bo_gem,
                          bucket->head.next, head);
            if (!mos_gem_bo_busy(&bo_gem->bo)) {
                alloc_from_cache = true;
                DRMLISTDEL(&bo_gem->head);
            }
        }

        if (alloc_from_cache) {
            if (!mos_gem_bo_madvise_internal
                (bufmgr_gem, bo_gem, I915_MADV_WILLNEED)) {
                mos_gem_bo_free(&bo_gem->bo);
                mos_gem_bo_cache_purge_bucket(bufmgr_gem,
                                    bucket);
                goto retry;
            }
            if (bo_gem->pat_index != alloc->ext.pat_index)
            {
                mos_gem_bo_free(&bo_gem->bo);
                goto retry;
            }
            if (mos_gem_bo_set_tiling_internal(&bo_gem->bo,
                                 alloc->ext.tiling_mode,
                                 alloc->stride)) {
                mos_gem_bo_free(&bo_gem->bo);
                goto retry;
            }
            if (bufmgr_gem->has_lmem && mos_gem_bo_check_mem_region_internal(&bo_gem->bo, alloc->ext.mem_type)) {
                mos_gem_bo_free(&bo_gem->bo);
                goto retry;
            }
        }
    }
    pthread_mutex_unlock(&bufmgr_gem->lock);

    if (!alloc_from_cache) {

        bo_gem = (struct mos_bo_gem *)calloc(1, sizeof(*bo_gem));
        if (!bo_gem)
            return nullptr;

        bo_gem->bo.size = bo_size;
        bo_gem->mem_region = I915_MEMORY_CLASS_SYSTEM;
        bo_gem->pat_index  = PAT_INDEX_INVALID;
        bo_gem->cpu_cacheable = true;

        if (bufmgr_gem->has_lmem &&
            (alloc->ext.mem_type == MOS_MEMPOOL_VIDEOMEMORY || alloc->ext.mem_type == MOS_MEMPOOL_DEVICEMEMORY)) {
            struct drm_i915_gem_memory_class_instance mem_region;
            memclear(mem_region);
            mem_region.memory_class = I915_MEMORY_CLASS_DEVICE;
            mem_region.memory_instance = 0;

            struct drm_i915_gem_create_ext_memory_regions regions;
            memclear(regions);
            regions.base.name = I915_GEM_CREATE_EXT_MEMORY_REGIONS;
            regions.num_regions = 1;
            regions.regions = (uintptr_t)(&mem_region);

            struct drm_i915_gem_create_ext create;
            memclear(create);
            create.size = bo_size;
            create.extensions = (uintptr_t)(&regions);

            ret = drmIoctl(bufmgr_gem->fd,
                    DRM_IOCTL_I915_GEM_CREATE_EXT,
                    &create);
            bo_gem->gem_handle = create.handle;
            bo_gem->bo.handle = bo_gem->gem_handle;
            bo_gem->mem_region = I915_MEMORY_CLASS_DEVICE;
        }
        else
        {
            ret = -EINVAL;
            if (support_pat_index && alloc->ext.pat_index != PAT_INDEX_INVALID)
            {
                struct drm_i915_gem_create_ext_set_pat set_pat_ext;
                memclear(set_pat_ext);
                set_pat_ext.base.name = I915_GEM_CREATE_EXT_SET_PAT;
                set_pat_ext.pat_index = alloc->ext.pat_index;

                struct drm_i915_gem_create_ext create;
                memclear(create);
                create.size = bo_size;
                create.extensions = (uintptr_t)(&set_pat_ext);
                ret = drmIoctl(bufmgr_gem->fd,
                        DRM_IOCTL_I915_GEM_CREATE_EXT,
                        &create);
                bo_gem->gem_handle = create.handle;
                bo_gem->bo.handle = bo_gem->gem_handle;
                bo_gem->pat_index = alloc->ext.pat_index;
                if (ret != 0)
                {
                    /* For old kernel without pat_index support,
                     * DRM_IOCTL_I915_GEM_CREATE_EXT with unknown
                     * set_pat_ext extension will return -EINVAL
                     * support_pat_index need to be set false.
                     */
                    support_pat_index = false;
                }
            }
            if (ret != 0)
            {
                struct drm_i915_gem_create create;
                memclear(create);
                create.size = bo_size;
                ret = drmIoctl(bufmgr_gem->fd,
                    DRM_IOCTL_I915_GEM_CREATE,
                    &create);
                bo_gem->gem_handle = create.handle;
                bo_gem->bo.handle = bo_gem->gem_handle;
                bo_gem->pat_index = PAT_INDEX_INVALID;
            }
        }
        if (ret != 0) {
            free(bo_gem);
            return nullptr;
        }
        bo_gem->bo.bufmgr = bufmgr;
        bo_gem->bo.align = alloc->alignment;

        bo_gem->tiling_mode = I915_TILING_NONE;
        bo_gem->swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
        bo_gem->stride = 0;
        if (bufmgr_gem->mem_profiler_fd != -1)
        {
            snprintf(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE, "GEM_CREATE, %d, %d, %lu, %d, %s\n", getpid(), bo_gem->bo.handle, bo_gem->bo.size,bo_gem->mem_region, alloc->name);
            ret = write(bufmgr_gem->mem_profiler_fd, bufmgr_gem->mem_profiler_buffer, strnlen(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE));
            if (ret == -1)
            {
                MOS_DBG("Failed to write to %s: %s\n", bufmgr_gem->mem_profiler_path, strerror(errno));
            }
        }

        /* drm_intel_gem_bo_free calls DRMLISTDEL() for an uninitialized
           list (vma_list), so better set the list head here */
        DRMINITLISTHEAD(&bo_gem->name_list);
        if (mos_gem_bo_set_tiling_internal(&bo_gem->bo,
                             alloc->ext.tiling_mode,
                             alloc->stride)) {
            mos_gem_bo_free(&bo_gem->bo);
            return nullptr;
        }
    }

    bo_gem->name = alloc->name;
    atomic_set(&bo_gem->refcount, 1);
    bo_gem->validate_index = -1;
    bo_gem->reloc_tree_fences = 0;
    bo_gem->used_as_reloc_target = false;
    bo_gem->has_error = false;
    bo_gem->reusable = true;
    bo_gem->use_48b_address_range = bufmgr_gem->bufmgr.bo_use_48b_address_range ? true : false;

    if (bo_gem->pat_index != PAT_INDEX_INVALID)
    {
        bo_gem->cpu_cacheable = alloc->ext.cpu_cacheable;
    }

    mos_bo_gem_set_in_aperture_size(bufmgr_gem, bo_gem, alloc->alignment);

    if (bufmgr_gem->use_softpin)
    {
        mos_bo_set_softpin(&bo_gem->bo);
    }

    MOS_DBG("bo_create: buf %d (%s) %ldb\n",
        bo_gem->gem_handle, bo_gem->name, alloc->size);

    return &bo_gem->bo;
}

static struct mos_linux_bo *
mos_gem_bo_alloc(struct mos_bufmgr *bufmgr,
               struct mos_drm_bo_alloc *alloc)
{
    alloc->ext.flags = 0;
    alloc->alignment = 0;
    alloc->stride = 0;
    return mos_gem_bo_alloc_internal(bufmgr, alloc);
}

static struct mos_linux_bo *
mos_gem_bo_alloc_tiled(struct mos_bufmgr *bufmgr,
            struct mos_drm_bo_alloc_tiled *alloc_tiled)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    unsigned long size, stride;
    uint32_t tiling;

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

        if (tiling == I915_TILING_X
            || (IS_915(bufmgr_gem->pci_device)
                && tiling == I915_TILING_Y))
            height_alignment = 8;
        else if (tiling == I915_TILING_Y)
            height_alignment = 32;
        aligned_y = ALIGN(alloc_tiled->y, height_alignment);

        stride = alloc_tiled->x * alloc_tiled->cpp;
        stride = mos_gem_bo_tile_pitch(bufmgr_gem, stride, &alloc_tiled->ext.tiling_mode);
        size = stride * aligned_y;
        size = mos_gem_bo_tile_size(bufmgr_gem, size, &alloc_tiled->ext.tiling_mode);
    } while (alloc_tiled->ext.tiling_mode != tiling);
    alloc_tiled->pitch = stride;

    if (tiling == I915_TILING_NONE)
        stride = 0;

    struct mos_drm_bo_alloc alloc;
    alloc.name = alloc_tiled->name;
    alloc.size = size;
    alloc.stride = stride;
    alloc.ext = alloc_tiled->ext;
    return mos_gem_bo_alloc_internal(bufmgr, &alloc);
}

static struct mos_linux_bo *
mos_gem_bo_alloc_userptr(struct mos_bufmgr *bufmgr,
                struct mos_drm_bo_alloc_userptr *alloc_uptr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;
    struct mos_bo_gem *bo_gem;
    int ret;
    struct drm_i915_gem_userptr userptr;

    /* Tiling with userptr surfaces is not supported
     * on all hardware so refuse it for time being.
     */
    if (alloc_uptr->tiling_mode != I915_TILING_NONE)
        return nullptr;

    bo_gem = (struct mos_bo_gem *)calloc(1, sizeof(*bo_gem));
    if (!bo_gem)
        return nullptr;

    bo_gem->bo.size = alloc_uptr->size;

    memclear(userptr);
    userptr.user_ptr = (__u64)((unsigned long)alloc_uptr->addr);
    userptr.user_size = alloc_uptr->size;
    userptr.flags = 0;

    ret = drmIoctl(bufmgr_gem->fd,
            DRM_IOCTL_I915_GEM_USERPTR,
            &userptr);
    if (ret != 0) {
        MOS_DBG("bo_create_userptr: "
            "ioctl failed with user ptr %p size 0x%lx, "
            "user flags 0x%lx\n", alloc_uptr->addr, alloc_uptr->size, alloc_uptr->flags);
        free(bo_gem);
        return nullptr;
    }

    bo_gem->gem_handle    = userptr.handle;
    bo_gem->bo.handle     = bo_gem->gem_handle;
    bo_gem->bo.bufmgr     = bufmgr;
    bo_gem->is_userptr    = true;
    bo_gem->pat_index     = PAT_INDEX_INVALID;
    bo_gem->cpu_cacheable = true;
#ifdef __cplusplus
    bo_gem->bo.virt   = alloc_uptr->addr;
#else
    bo_gem->bo.virtual   = alloc_uptr->addr;
#endif
    /* Save the address provided by user */
    bo_gem->user_virtual = alloc_uptr->addr;
    bo_gem->tiling_mode  = I915_TILING_NONE;
    bo_gem->swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
    bo_gem->stride       = 0;

    DRMINITLISTHEAD(&bo_gem->name_list);

    bo_gem->name = alloc_uptr->name;
    atomic_set(&bo_gem->refcount, 1);
    bo_gem->validate_index = -1;
    bo_gem->reloc_tree_fences = 0;
    bo_gem->used_as_reloc_target = false;
    bo_gem->has_error = false;
    bo_gem->reusable = false;
    bo_gem->use_48b_address_range = bufmgr_gem->bufmgr.bo_use_48b_address_range ? true : false;

    mos_bo_gem_set_in_aperture_size(bufmgr_gem, bo_gem, 0);

    if (bufmgr_gem->use_softpin)
    {
        mos_bo_set_softpin(&bo_gem->bo);
    }

    MOS_DBG("bo_create_userptr: "
        "ptr %p buf %d (%s) size %ldb, stride 0x%x, tile mode %d\n",
        alloc_uptr->addr, bo_gem->gem_handle, bo_gem->name,
        alloc_uptr->size, alloc_uptr->stride, alloc_uptr->tiling_mode);

    return &bo_gem->bo;
}

static bool
has_userptr(struct mos_bufmgr_gem *bufmgr_gem)
{
    int ret;
    void *ptr;
    long pgsz;
    struct drm_i915_gem_userptr userptr;

    pgsz = sysconf(_SC_PAGESIZE);
    assert(pgsz > 0);

    ret = posix_memalign(&ptr, pgsz, pgsz);
    if (ret) {
        MOS_DBG("Failed to get a page (%ld) for userptr detection!\n",
            pgsz);
        return false;
    }

    memclear(userptr);
    userptr.user_ptr = (__u64)(unsigned long)ptr;
    userptr.user_size = pgsz;

retry:
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_USERPTR, &userptr);
    if (ret) {
        if (errno == ENODEV && userptr.flags == 0) {
            userptr.flags = I915_USERPTR_UNSYNCHRONIZED;
            goto retry;
        }
        free(ptr);
        return false;
    }

    /* We don't release the userptr bo here as we want to keep the
     * kernel mm tracking alive for our lifetime. The first time we
     * create a userptr object the kernel has to install a mmu_notifer
     * which is a heavyweight operation (e.g. it requires taking all
     * mm_locks and stop_machine()).
     */

    bufmgr_gem->userptr_active.ptr = ptr;
    bufmgr_gem->userptr_active.handle = userptr.handle;
    return true;
}

static struct mos_linux_bo *
check_bo_alloc_userptr(struct mos_bufmgr *bufmgr,
               struct mos_drm_bo_alloc_userptr *alloc_uptr)
{
    if (has_userptr((struct mos_bufmgr_gem *)bufmgr))
        bufmgr->bo_alloc_userptr = mos_gem_bo_alloc_userptr;
    else
        bufmgr->bo_alloc_userptr = nullptr;

    return mos_bo_alloc_userptr(bufmgr, alloc_uptr);
}

/**
 * Returns a drm_intel_bo wrapping the given buffer object handle.
 *
 * This can be used when one application needs to pass a buffer object
 * to another.
 */
static struct mos_linux_bo *
mos_bufmgr_bo_gem_create_from_name(struct mos_bufmgr *bufmgr,
                  const char *name,
                  unsigned int handle)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;
    struct mos_bo_gem *bo_gem;
    int ret;
    struct drm_gem_open open_arg;
    struct drm_i915_gem_get_tiling get_tiling;
    drmMMListHead *list;

    /* At the moment most applications only have a few named bo.
     * For instance, in a DRI client only the render buffers passed
     * between X and the client are named. And since X returns the
     * alternating names for the front/back buffer a linear search
     * provides a sufficiently fast match.
     */
    pthread_mutex_lock(&bufmgr_gem->lock);
    for (list = bufmgr_gem->named.next;
         list != &bufmgr_gem->named;
         list = list->next) {
        bo_gem = DRMLISTENTRY(struct mos_bo_gem, list, name_list);
        if (bo_gem->global_name == handle) {
            mos_gem_bo_reference(&bo_gem->bo);
            pthread_mutex_unlock(&bufmgr_gem->lock);
            return &bo_gem->bo;
        }
    }

    memclear(open_arg);
    open_arg.name = handle;
    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_GEM_OPEN,
               &open_arg);
    if (ret != 0) {
        MOS_DBG("Couldn't reference %s handle 0x%08x: %s\n",
            name, handle, strerror(errno));
        pthread_mutex_unlock(&bufmgr_gem->lock);
        return nullptr;
    }
        /* Now see if someone has used a prime handle to get this
         * object from the kernel before by looking through the list
         * again for a matching gem_handle
         */
    for (list = bufmgr_gem->named.next;
         list != &bufmgr_gem->named;
         list = list->next) {
        bo_gem = DRMLISTENTRY(struct mos_bo_gem, list, name_list);
        if (bo_gem->gem_handle == open_arg.handle) {
            mos_gem_bo_reference(&bo_gem->bo);
            pthread_mutex_unlock(&bufmgr_gem->lock);
            return &bo_gem->bo;
        }
    }

    bo_gem = (struct mos_bo_gem *)calloc(1, sizeof(*bo_gem));
    if (!bo_gem) {
        pthread_mutex_unlock(&bufmgr_gem->lock);
        return nullptr;
    }

    bo_gem->bo.size = open_arg.size;
    bo_gem->bo.offset = 0;
    bo_gem->bo.offset64 = 0;
#if defined(__cplusplus)
    bo_gem->bo.virt = nullptr;
#else
    bo_gem->bo.virtual = nullptr;
#endif
    bo_gem->bo.bufmgr = bufmgr;
    bo_gem->name = name;
    bo_gem->pat_index = PAT_INDEX_INVALID;
    bo_gem->cpu_cacheable = true;
    atomic_set(&bo_gem->refcount, 1);
    bo_gem->validate_index = -1;
    bo_gem->gem_handle = open_arg.handle;
    bo_gem->bo.handle = open_arg.handle;
    bo_gem->global_name = handle;
    bo_gem->reusable = false;
    bo_gem->use_48b_address_range = bufmgr_gem->bufmgr.bo_use_48b_address_range ? true : false;

    memclear(get_tiling);
    if (bufmgr_gem->has_fence_reg) {
        get_tiling.handle = bo_gem->gem_handle;
        ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_GET_TILING,
               &get_tiling);
        if (ret != 0) {
            MOS_DBG("create_from_name: failed to get tiling: %s\n", strerror(errno));
            mos_gem_bo_unreference(&bo_gem->bo);
            pthread_mutex_unlock(&bufmgr_gem->lock);
            return nullptr;
        }
    }
    else
    {
        MOS_DBG("create_from_name: driver ignored to get tiling from kernel\n");
    }

    bo_gem->tiling_mode = get_tiling.tiling_mode;
    bo_gem->swizzle_mode = get_tiling.swizzle_mode;
    /* XXX stride is unknown */

    mos_bo_gem_set_in_aperture_size(bufmgr_gem, bo_gem, 0);

    DRMLISTADDTAIL(&bo_gem->name_list, &bufmgr_gem->named);
    pthread_mutex_unlock(&bufmgr_gem->lock);

    if (bufmgr_gem->use_softpin)
    {
        mos_bo_set_softpin(&bo_gem->bo);
    }

    MOS_DBG("bo_create_from_handle: %d (%s)\n", handle, bo_gem->name);

    return &bo_gem->bo;
}

static void
mos_gem_bo_free(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = nullptr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_gem_close close;
    int ret;

    CHK_CONDITION(bo_gem == nullptr, "bo_gem == nullptr\n", );

    bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;

    CHK_CONDITION(bufmgr_gem == nullptr, "bufmgr_gem == nullptr\n", );

    if (bo_gem->mem_virtual) {
        VG(VALGRIND_MAKE_MEM_NOACCESS(bo_gem->mem_virtual, 0));
        drm_munmap(bo_gem->mem_virtual, bo_gem->bo.size);
        bo_gem->mem_virtual = nullptr;
    }
    if (bo_gem->gtt_virtual) {
        VG(VALGRIND_MAKE_MEM_NOACCESS(bo_gem->gtt_virtual, 0));
        drm_munmap(bo_gem->gtt_virtual, bo_gem->bo.size);
        bo_gem->gtt_virtual = nullptr;
    }
    if (bo_gem->mem_wc_virtual) {
        VG(VALGRIND_MAKE_MEM_NOACCESS(bo_gem->mem_wc_virtual, 0));
        drm_munmap(bo_gem->mem_wc_virtual, bo_gem->bo.size);
        bo_gem->mem_wc_virtual = nullptr;
    }

    if(bufmgr_gem->bufmgr.bo_wait_rendering && mos_gem_bo_busy(bo))
    {
        bufmgr_gem->bufmgr.bo_wait_rendering(bo);
    }

    /* Close this object */
    memclear(close);
    close.handle = bo_gem->gem_handle;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_GEM_CLOSE, &close);
    if (ret != 0) {
        MOS_DBG("DRM_IOCTL_GEM_CLOSE %d failed (%s): %s\n",
            bo_gem->gem_handle, bo_gem->name, strerror(errno));
    }
    if (bufmgr_gem->mem_profiler_fd != -1)
    {
        snprintf(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE, "GEM_CLOSE, %d, %d, %lu, %d\n", getpid(), bo->handle,bo->size,bo_gem->mem_region);
        ret = write(bufmgr_gem->mem_profiler_fd, bufmgr_gem->mem_profiler_buffer, strnlen(bufmgr_gem->mem_profiler_buffer, MEM_PROFILER_BUFFER_SIZE));
        if (ret == -1)
        {
            MOS_DBG("Failed to write to %s: %s\n", bufmgr_gem->mem_profiler_path, strerror(errno));
        }
    }

    if (bufmgr_gem->use_softpin)
    {
        /* Return the VMA for reuse */
        mos_gem_bo_vma_free(bo->bufmgr, bo->offset64, bo->size);
    }

    free(bo);
}

static void
mos_gem_bo_mark_mmaps_incoherent(struct mos_linux_bo *bo)
{
#if HAVE_VALGRIND
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    if (bo_gem->mem_virtual)
        VALGRIND_MAKE_MEM_NOACCESS(bo_gem->mem_virtual, bo->size);

    if (bo_gem->gtt_virtual)
        VALGRIND_MAKE_MEM_NOACCESS(bo_gem->gtt_virtual, bo->size);

    if (bo_gem->mem_wc_virtual)
        VALGRIND_MAKE_MEM_NOACCESS(bo_gem->mem_wc_virtual, bo->size);
#endif
}

/** Frees all cached buffers significantly older than @time. */
static void
mos_gem_cleanup_bo_cache(struct mos_bufmgr_gem *bufmgr_gem, time_t time)
{
    int i;

    if (bufmgr_gem->time == time)
        return;

    for (i = 0; i < bufmgr_gem->num_buckets; i++) {
        struct mos_gem_bo_bucket *bucket =
            &bufmgr_gem->cache_bucket[i];

        while (!DRMLISTEMPTY(&bucket->head)) {
            struct mos_bo_gem *bo_gem;

            bo_gem = DRMLISTENTRY(struct mos_bo_gem,
                          bucket->head.next, head);
            if (time - bo_gem->free_time <= 1)
                break;

            DRMLISTDEL(&bo_gem->head);

            mos_gem_bo_free(&bo_gem->bo);
        }
    }

    bufmgr_gem->time = time;
}

drm_export void
mos_gem_bo_unreference_final(struct mos_linux_bo *bo, time_t time)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct mos_gem_bo_bucket *bucket;
    int i;

    /* Unreference all the target buffers */
    for (i = 0; i < bo_gem->reloc_count; i++) {
        if (bo_gem->reloc_target_info[i].bo != bo) {
            mos_gem_bo_unreference_locked_timed(bo_gem->
                                  reloc_target_info[i].bo,
                                  time);
        }
    }
    for (i = 0; i < bo_gem->softpin_target_count; i++)
        mos_gem_bo_unreference_locked_timed(bo_gem->softpin_target[i].bo,
                                  time);
    bo_gem->reloc_count = 0;
    bo_gem->used_as_reloc_target = false;
    bo_gem->softpin_target_count = 0;
    bo_gem->exec_async = false;

    MOS_DBG("bo_unreference final: %d (%s)\n",
        bo_gem->gem_handle, bo_gem->name);
    bo_gem->pad_to_size = 0;

    /* release memory associated with this object */
    if (bo_gem->reloc_target_info) {
        free(bo_gem->reloc_target_info);
        bo_gem->reloc_target_info = nullptr;
    }
    if (bo_gem->relocs) {
        free(bo_gem->relocs);
        bo_gem->relocs = nullptr;
    }
    if (bo_gem->softpin_target) {
        free(bo_gem->softpin_target);
        bo_gem->softpin_target = nullptr;
        bo_gem->max_softpin_target_count = 0;
    }

    /* Clear any left-over mappings */
    if (bo_gem->map_count) {
        MOS_DBG("bo freed with non-zero map-count %d\n", bo_gem->map_count);
        bo_gem->map_count = 0;
        mos_gem_bo_mark_mmaps_incoherent(bo);
    }

    DRMLISTDEL(&bo_gem->name_list);

    bucket = mos_gem_bo_bucket_for_size(bufmgr_gem, bo->size);
    /* Put the buffer into our internal cache for reuse if we can. */
    if (bufmgr_gem->bo_reuse && bo_gem->reusable && bucket != nullptr &&
        mos_gem_bo_madvise_internal(bufmgr_gem, bo_gem,
                          I915_MADV_DONTNEED)) {
        bo_gem->free_time = time;

        bo_gem->name = nullptr;
        bo_gem->validate_index = -1;

        DRMLISTADDTAIL(&bo_gem->head, &bucket->head);
    } else {
        mos_gem_bo_free(bo);
    }
}

static void mos_gem_bo_unreference_locked_timed(struct mos_linux_bo *bo,
                              time_t time)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    assert(atomic_read(&bo_gem->refcount) > 0);
    if (atomic_dec_and_test(&bo_gem->refcount))
        mos_gem_bo_unreference_final(bo, time);
}

static void mos_gem_bo_unreference(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    assert(atomic_read(&bo_gem->refcount) > 0);

    if (atomic_add_unless(&bo_gem->refcount, -1, 1)) {
        struct mos_bufmgr_gem *bufmgr_gem =
            (struct mos_bufmgr_gem *) bo->bufmgr;
        struct timespec time;

        clock_gettime(CLOCK_MONOTONIC, &time);

        pthread_mutex_lock(&bufmgr_gem->lock);

        if (atomic_dec_and_test(&bo_gem->refcount)) {
            mos_gem_bo_unreference_final(bo, time.tv_sec);
            mos_gem_cleanup_bo_cache(bufmgr_gem, time.tv_sec);
        }

        pthread_mutex_unlock(&bufmgr_gem->lock);
    }
}

static int
map_wc(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int ret;

    if (bo_gem->is_userptr)
        return -EINVAL;

    if (!bufmgr_gem->has_ext_mmap)
        return -EINVAL;

    /* Get a mapping of the buffer if we haven't before. */
    if (bo_gem->mem_wc_virtual == nullptr && bufmgr_gem->has_mmap_offset) {
        struct drm_i915_gem_mmap_offset mmap_arg;

        MOS_DBG("bo_map_wc: mmap_offset %d (%s), map_count=%d\n",
            bo_gem->gem_handle, bo_gem->name, bo_gem->map_count);

        memclear(mmap_arg);
        mmap_arg.handle = bo_gem->gem_handle;
        /* To indicate the uncached virtual mapping to KMD */
        if (bufmgr_gem->has_lmem)
        {
            mmap_arg.flags = I915_MMAP_OFFSET_FIXED;
        }
        else
        {
            mmap_arg.flags = I915_MMAP_OFFSET_WC;
        }
        ret = drmIoctl(bufmgr_gem->fd,
                   DRM_IOCTL_I915_GEM_MMAP_OFFSET,
                   &mmap_arg);
        if (ret != 0) {
            ret = -errno;
            MOS_DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
                __FILE__, __LINE__, bo_gem->gem_handle,
                bo_gem->name, strerror(errno));
            return ret;
        }

        /* and mmap it */
        bo_gem->mem_wc_virtual = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE,
                           MAP_SHARED, bufmgr_gem->fd,
                           mmap_arg.offset);
        if (bo_gem->mem_wc_virtual == MAP_FAILED) {
            bo_gem->mem_wc_virtual = nullptr;
            ret = -errno;
            MOS_DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
                __FILE__, __LINE__,
                bo_gem->gem_handle, bo_gem->name,
                strerror(errno));
        }
    }
    else if (bo_gem->mem_wc_virtual == nullptr) {
        struct drm_i915_gem_mmap mmap_arg;

        MOS_DBG("bo_map_wc: mmap %d (%s), map_count=%d\n",
            bo_gem->gem_handle, bo_gem->name, bo_gem->map_count);

        memclear(mmap_arg);
        mmap_arg.handle = bo_gem->gem_handle;
        /* To indicate the uncached virtual mapping to KMD */
        mmap_arg.flags = I915_MMAP_WC;
        mmap_arg.size = bo->size;
        ret = drmIoctl(bufmgr_gem->fd,
                   DRM_IOCTL_I915_GEM_MMAP,
                   &mmap_arg);
        if (ret != 0) {
            ret = -errno;
            MOS_DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
                __FILE__, __LINE__, bo_gem->gem_handle,
                bo_gem->name, strerror(errno));
            return ret;
        }
        VG(VALGRIND_MALLOCLIKE_BLOCK(mmap_arg.addr_ptr, mmap_arg.size, 0, 1));
        bo_gem->mem_wc_virtual = (void *)(uintptr_t) mmap_arg.addr_ptr;
    }
#ifdef __cplusplus
    bo->virt = bo_gem->mem_wc_virtual;
#else
    bo->virtual = bo_gem->mem_wc_virtual;
#endif

    MOS_DBG("bo_map_wc: %d (%s) -> %p\n", bo_gem->gem_handle, bo_gem->name,
        bo_gem->mem_wc_virtual);

    return 0;
}

/* To be used in a similar way to mmap_gtt */
drm_export int
mos_gem_bo_map_wc(struct mos_linux_bo *bo) {
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_set_domain set_domain;
    struct drm_i915_gem_wait wait;
    int ret;

    pthread_mutex_lock(&bufmgr_gem->lock);

    ret = map_wc(bo);
    if (ret) {
        pthread_mutex_unlock(&bufmgr_gem->lock);
        return ret;
    }

    if (bufmgr_gem->has_lmem) {
        assert(bufmgr_gem->has_wait_timeout);
        memclear(wait);
        wait.bo_handle = bo_gem->gem_handle;
        wait.timeout_ns = -1; // infinite wait
        ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_WAIT, &wait);
        if (ret == -1) {
            MOS_DBG("%s:%d: DRM_IOCTL_I915_GEM_WAIT failed (%d)\n",
                __FILE__, __LINE__, errno);
        }
    } else {
        /* Now move it to the GTT domain so that the GPU and CPU
         * caches are flushed and the GPU isn't actively using the
         * buffer.
         *
         * The domain change is done even for the objects which
         * are not bounded. For them first the pages are acquired,
         * before the domain change.
         */
        memclear(set_domain);
        set_domain.handle = bo_gem->gem_handle;
        set_domain.read_domains = I915_GEM_DOMAIN_GTT;
        set_domain.write_domain = I915_GEM_DOMAIN_GTT;
        ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_SET_DOMAIN,
               &set_domain);
        if (ret != 0) {
            MOS_DBG("%s:%d: Error setting domain %d: %s\n",
                __FILE__, __LINE__, bo_gem->gem_handle,
                strerror(errno));
        }
    }

    mos_gem_bo_mark_mmaps_incoherent(bo);
    VG(VALGRIND_MAKE_MEM_DEFINED(bo_gem->mem_wc_virtual, bo->size));
    pthread_mutex_unlock(&bufmgr_gem->lock);

    return 0;
}

drm_export int mos_gem_bo_map(struct mos_linux_bo *bo, int write_enable)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int ret;

    if (bo_gem->is_userptr) {
        /* Return the same user ptr */
#ifdef __cplusplus
        bo->virt = bo_gem->user_virtual;
#else
        bo->virtual = bo_gem->user_virtual;
#endif
        return 0;
    }
    /* If cpu cacheable is false, it means bo is Non-Coherent. */
    if (!bo_gem->cpu_cacheable)
    {
        return mos_gem_bo_map_wc(bo);
    }

    pthread_mutex_lock(&bufmgr_gem->lock);

    if (bufmgr_gem->has_mmap_offset) {
        struct drm_i915_gem_wait wait;

        if (!bo_gem->mem_virtual) {
            struct drm_i915_gem_mmap_offset mmap_arg;

            MOS_DBG("bo_map: %d (%s), map_count=%d\n",
                bo_gem->gem_handle, bo_gem->name, bo_gem->map_count);

            memclear(mmap_arg);
            mmap_arg.handle = bo_gem->gem_handle;
            if (bufmgr_gem->has_lmem)
            {
                mmap_arg.flags = I915_MMAP_OFFSET_FIXED;
            }
            else
            {
                mmap_arg.flags = I915_MMAP_OFFSET_WB;
            }
            ret = drmIoctl(bufmgr_gem->fd,
                   DRM_IOCTL_I915_GEM_MMAP_OFFSET,
                   &mmap_arg);
            if (ret != 0) {
                ret = -errno;
                MOS_DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
                    __FILE__, __LINE__, bo_gem->gem_handle,
                    bo_gem->name, strerror(errno));
                pthread_mutex_unlock(&bufmgr_gem->lock);
                return ret;
            }

            /* and mmap it */
            bo_gem->mem_virtual = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE,
                MAP_SHARED, bufmgr_gem->fd,
                mmap_arg.offset);
            if (bo_gem->mem_virtual == MAP_FAILED) {
                bo_gem->mem_virtual = nullptr;
                ret = -errno;
                MOS_DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
                    __FILE__, __LINE__,
                    bo_gem->gem_handle, bo_gem->name,
                    strerror(errno));
            }
        }

        assert(bufmgr_gem->has_wait_timeout);
        memclear(wait);
        wait.bo_handle = bo_gem->gem_handle;
        wait.timeout_ns = -1; // infinite wait
        ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_WAIT, &wait);
        if (ret == -1) {
            MOS_DBG("%s:%d: DRM_IOCTL_I915_GEM_WAIT failed (%d)\n",
                __FILE__, __LINE__, errno);
        }
    } else { /*!has_mmap_offset*/
        struct drm_i915_gem_set_domain set_domain;

        if (!bo_gem->mem_virtual) {
            struct drm_i915_gem_mmap mmap_arg;

            MOS_DBG("bo_map: %d (%s), map_count=%d\n",
                bo_gem->gem_handle, bo_gem->name, bo_gem->map_count);

            memclear(mmap_arg);
            mmap_arg.handle = bo_gem->gem_handle;
            mmap_arg.size = bo->size;
            ret = drmIoctl(bufmgr_gem->fd,
                DRM_IOCTL_I915_GEM_MMAP,
                &mmap_arg);
            if (ret != 0) {
                ret = -errno;
                MOS_DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
                    __FILE__, __LINE__, bo_gem->gem_handle,
                    bo_gem->name, strerror(errno));
                pthread_mutex_unlock(&bufmgr_gem->lock);
                return ret;
            }
            VG(VALGRIND_MALLOCLIKE_BLOCK(mmap_arg.addr_ptr, mmap_arg.size, 0, 1));
            bo_gem->mem_virtual = (void *)(uintptr_t) mmap_arg.addr_ptr;
        }

        memclear(set_domain);
        set_domain.handle = bo_gem->gem_handle;
        set_domain.read_domains = I915_GEM_DOMAIN_CPU;
        if (write_enable)
            set_domain.write_domain = I915_GEM_DOMAIN_CPU;
        else
            set_domain.write_domain = 0;
        ret = drmIoctl(bufmgr_gem->fd,
            DRM_IOCTL_I915_GEM_SET_DOMAIN,
            &set_domain);
        if (ret != 0) {
            MOS_DBG("%s:%d: Error setting to CPU domain %d: %s\n",
            __FILE__, __LINE__, bo_gem->gem_handle,
            strerror(errno));
        }
    }
    MOS_DBG("bo_map: %d (%s) -> %p\n", bo_gem->gem_handle, bo_gem->name,
        bo_gem->mem_virtual);
#ifdef __cplusplus
    bo->virt = bo_gem->mem_virtual;
#else
    bo->virtual = bo_gem->mem_virtual;
#endif

    if (write_enable)
        bo_gem->mapped_cpu_write = true;

    mos_gem_bo_mark_mmaps_incoherent(bo);
    VG(VALGRIND_MAKE_MEM_DEFINED(bo_gem->mem_virtual, bo->size));
    pthread_mutex_unlock(&bufmgr_gem->lock);

    return 0;
}

drm_export int
map_gtt(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int ret;

    if (bo_gem->is_userptr)
        return -EINVAL;

    /* Get a mapping of the buffer if we haven't before. */
    if (bo_gem->gtt_virtual == nullptr) {
        __u64 offset = 0;
        if (bufmgr_gem->has_lmem) {
            struct drm_i915_gem_mmap_offset mmap_arg;

            MOS_DBG("map_gtt: mmap_offset %d (%s), map_count=%d\n",
                bo_gem->gem_handle, bo_gem->name, bo_gem->map_count);

            memclear(mmap_arg);
            mmap_arg.handle = bo_gem->gem_handle;
            mmap_arg.flags = I915_MMAP_OFFSET_FIXED;

            /* Get the fake offset back... */
            ret = drmIoctl(bufmgr_gem->fd,
                       DRM_IOCTL_I915_GEM_MMAP_OFFSET,
                       &mmap_arg);
            offset = mmap_arg.offset;
        }
        else
        {
            struct drm_i915_gem_mmap_gtt mmap_arg;

            MOS_DBG("bo_map_gtt: mmap %d (%s), map_count=%d\n",
                bo_gem->gem_handle, bo_gem->name, bo_gem->map_count);

            memclear(mmap_arg);
            mmap_arg.handle = bo_gem->gem_handle;

           /* Get the fake offset back... */
            ret = drmIoctl(bufmgr_gem->fd,
                   DRM_IOCTL_I915_GEM_MMAP_GTT,
                   &mmap_arg);
            offset = mmap_arg.offset;
        }
        if (ret != 0) {
            ret = -errno;
            MOS_DBG("%s:%d: Error preparing buffer map %d (%s): %s .\n",
                __FILE__, __LINE__,
                bo_gem->gem_handle, bo_gem->name,
                strerror(errno));
            return ret;
        }

        /* and mmap it */
        bo_gem->gtt_virtual = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE,
                           MAP_SHARED, bufmgr_gem->fd,
                           offset);
        if (bo_gem->gtt_virtual == MAP_FAILED) {
            bo_gem->gtt_virtual = nullptr;
            ret = -errno;
            MOS_DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
                __FILE__, __LINE__,
                bo_gem->gem_handle, bo_gem->name,
                strerror(errno));
            return ret;
        }
    }
#ifdef __cplusplus
    bo->virt = bo_gem->gtt_virtual;
#else
    bo->virtual = bo_gem->gtt_virtual;
#endif

    MOS_DBG("bo_map_gtt: %d (%s) -> %p\n", bo_gem->gem_handle, bo_gem->name,
        bo_gem->gtt_virtual);

    return 0;
}

static int
mos_gem_bo_map_gtt(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_set_domain set_domain;
    struct drm_i915_gem_wait wait;
    int ret;

    pthread_mutex_lock(&bufmgr_gem->lock);

    ret = map_gtt(bo);
    if (ret) {
        pthread_mutex_unlock(&bufmgr_gem->lock);
        return ret;
    }

    if (bufmgr_gem->has_lmem) {
        assert(bufmgr_gem->has_wait_timeout);
        memclear(wait);
        wait.bo_handle = bo_gem->gem_handle;
        wait.timeout_ns = -1; // infinite wait
        ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_WAIT, &wait);
        if (ret == -1) {
            MOS_DBG("%s:%d: DRM_IOCTL_I915_GEM_WAIT failed (%d)\n",
                __FILE__, __LINE__, errno);
        }
    } else {
        /* Now move it to the GTT domain so that the GPU and CPU
         * caches are flushed and the GPU isn't actively using the
         * buffer.
         *
         * The pagefault handler does this domain change for us when
         * it has unbound the BO from the GTT, but it's up to us to
         * tell it when we're about to use things if we had done
         * rendering and it still happens to be bound to the GTT.
         */
        memclear(set_domain);
        set_domain.handle = bo_gem->gem_handle;
        set_domain.read_domains = I915_GEM_DOMAIN_GTT;
        set_domain.write_domain = I915_GEM_DOMAIN_GTT;
        ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_SET_DOMAIN,
               &set_domain);
        if (ret != 0) {
            MOS_DBG("%s:%d: Error setting domain %d: %s\n",
                __FILE__, __LINE__, bo_gem->gem_handle,
                strerror(errno));
        }
    }
    mos_gem_bo_mark_mmaps_incoherent(bo);
    VG(VALGRIND_MAKE_MEM_DEFINED(bo_gem->gtt_virtual, bo->size));
    pthread_mutex_unlock(&bufmgr_gem->lock);

    return 0;
}

/**
 * Performs a mapping of the buffer object like the normal GTT
 * mapping, but avoids waiting for the GPU to be done reading from or
 * rendering to the buffer.
 *
 * This is used in the implementation of GL_ARB_map_buffer_range: The
 * user asks to create a buffer, then does a mapping, fills some
 * space, runs a drawing command, then asks to map it again without
 * synchronizing because it guarantees that it won't write over the
 * data that the GPU is busy using (or, more specifically, that if it
 * does write over the data, it acknowledges that rendering is
 * undefined).
 */

static int
mos_gem_bo_map_unsynchronized(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
#ifdef HAVE_VALGRIND
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
#endif
    int ret;

    /* If the CPU cache isn't coherent with the GTT, then use a
     * regular synchronized mapping.  The problem is that we don't
     * track where the buffer was last used on the CPU side in
     * terms of drm_intel_bo_map vs drm_intel_gem_bo_map_gtt, so
     * we would potentially corrupt the buffer even when the user
     * does reasonable things.
     */
    if (!bufmgr_gem->has_llc)
        return mos_gem_bo_map_gtt(bo);

    pthread_mutex_lock(&bufmgr_gem->lock);

    ret = map_gtt(bo);
    if (ret == 0) {
        mos_gem_bo_mark_mmaps_incoherent(bo);
        VG(VALGRIND_MAKE_MEM_DEFINED(bo_gem->gtt_virtual, bo->size));
    }

    pthread_mutex_unlock(&bufmgr_gem->lock);

    return ret;
}

static int mos_gem_bo_unmap(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int ret = 0;

    if (bo == nullptr)
        return 0;

    if (bo_gem->is_userptr)
        return 0;

    bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;

    pthread_mutex_lock(&bufmgr_gem->lock);

    if (bo_gem->map_count <= 0) {
        MOS_DBG("attempted to unmap an unmapped bo\n");
        pthread_mutex_unlock(&bufmgr_gem->lock);
        /* Preserve the old behaviour of just treating this as a
         * no-op rather than reporting the error.
         */
        return 0;
    }

    if (bo_gem->mapped_cpu_write) {
        struct drm_i915_gem_sw_finish sw_finish;

        /* Cause a flush to happen if the buffer's pinned for
         * scanout, so the results show up in a timely manner.
         * Unlike GTT set domains, this only does work if the
         * buffer should be scanout-related.
         */
        memclear(sw_finish);
        sw_finish.handle = bo_gem->gem_handle;
        ret = drmIoctl(bufmgr_gem->fd,
                   DRM_IOCTL_I915_GEM_SW_FINISH,
                   &sw_finish);
        ret = ret == -1 ? -errno : 0;

        bo_gem->mapped_cpu_write = false;
    }

    /* We need to unmap after every innovation as we cannot track
     * an open vma for every bo as that will exhaasut the system
     * limits and cause later failures.
     */
    if (--bo_gem->map_count == 0) {
        mos_gem_bo_mark_mmaps_incoherent(bo);
#ifdef __cplusplus
        bo->virt = nullptr;
#else
        bo->virtual = nullptr;
#endif
    }
    pthread_mutex_unlock(&bufmgr_gem->lock);

    return ret;
}

static int
mos_gem_bo_unmap_wc(struct mos_linux_bo *bo)
{
    return mos_gem_bo_unmap(bo);
}

static int
mos_gem_bo_unmap_gtt(struct mos_linux_bo *bo)
{
    return mos_gem_bo_unmap(bo);
}

/** Waits for all GPU rendering with the object to have completed. */
static void
mos_gem_bo_wait_rendering(struct mos_linux_bo *bo)
{
    mos_gem_bo_start_gtt_access(bo, 1);
}

/**
 * Waits on a BO for the given amount of time.
 *
 * @bo: buffer object to wait for
 * @timeout_ns: amount of time to wait in nanoseconds.
 *   If value is less than 0, an infinite wait will occur.
 *
 * Returns 0 if the wait was successful ie. the last batch referencing the
 * object has completed within the allotted time. Otherwise some negative return
 * value describes the error. Of particular interest is -ETIME when the wait has
 * failed to yield the desired result.
 *
 * Similar to drm_intel_gem_bo_wait_rendering except a timeout parameter allows
 * the operation to give up after a certain amount of time. Another subtle
 * difference is the internal locking semantics are different (this variant does
 * not hold the lock for the duration of the wait). This makes the wait subject
 * to a larger userspace race window.
 *
 * The implementation shall wait until the object is no longer actively
 * referenced within a batch buffer at the time of the call. The wait will
 * not guarantee that the buffer is re-issued via another thread, or an flinked
 * handle. Userspace must make sure this race does not occur if such precision
 * is important.
 *
 * Note that some kernels have broken the inifite wait for negative values
 * promise, upgrade to latest stable kernels if this is the case.
 */
drm_export int
mos_gem_bo_wait(struct mos_linux_bo *bo, int64_t timeout_ns)
{

    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_wait wait;
    int ret;

    if (!bufmgr_gem->has_wait_timeout) {
        MOS_DBG("%s:%d: Timed wait is not supported. Falling back to "
            "infinite wait\n", __FILE__, __LINE__);
        if (timeout_ns) {
            mos_gem_bo_wait_rendering(bo);
            return 0;
        } else {
            return mos_gem_bo_busy(bo) ? -ETIME : 0;
        }
    }

    memclear(wait);
    wait.bo_handle = bo_gem->gem_handle;
    wait.timeout_ns = timeout_ns;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_WAIT, &wait);
    if (ret == -1)
        return -errno;

    return ret;
}

/**
 * Sets the object to the GTT read and possibly write domain, used by the X
 * 2D driver in the absence of kernel support to do drm_intel_gem_bo_map_gtt().
 *
 * In combination with drm_intel_gem_bo_pin() and manual fence management, we
 * can do tiled pixmaps this way.
 */
static void
mos_gem_bo_start_gtt_access(struct mos_linux_bo *bo, int write_enable)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_set_domain set_domain;
    struct drm_i915_gem_wait wait;
    int ret;

    if (bufmgr_gem->has_lmem) {
        assert(bufmgr_gem->has_wait_timeout);
        memclear(wait);
        wait.bo_handle = bo_gem->gem_handle;
        wait.timeout_ns = -1; // infinite wait
        ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_WAIT, &wait);
        if (ret == -1) {
            MOS_DBG("%s:%d: DRM_IOCTL_I915_GEM_WAIT failed (%d)\n",
                __FILE__, __LINE__, errno);
        }
    } else {
        memclear(set_domain);
        set_domain.handle = bo_gem->gem_handle;
        set_domain.read_domains = I915_GEM_DOMAIN_GTT;
        set_domain.write_domain = write_enable ? I915_GEM_DOMAIN_GTT : 0;
        ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_SET_DOMAIN,
               &set_domain);
        if (ret != 0) {
            MOS_DBG("%s:%d: Error setting memory domains %d (%08x %08x): %s .\n",
                __FILE__, __LINE__, bo_gem->gem_handle,
                set_domain.read_domains, set_domain.write_domain,
                strerror(errno));
        }
    }
}

static void
mos_bufmgr_cleanup_cache(struct mos_bufmgr_gem *bufmgr_gem)
{
    for (int i = 0; i < bufmgr_gem->num_buckets; i++) {
        struct mos_gem_bo_bucket *bucket =
            &bufmgr_gem->cache_bucket[i];
        struct mos_bo_gem *bo_gem;

        while (!DRMLISTEMPTY(&bucket->head)) {
            bo_gem = DRMLISTENTRY(struct mos_bo_gem,
                          bucket->head.next, head);
            DRMLISTDEL(&bo_gem->head);

            mos_gem_bo_free(&bo_gem->bo);
        }
        bufmgr_gem->cache_bucket[i].size = 0;
    }
    bufmgr_gem->num_buckets = 0;
}

static void
mos_bufmgr_gem_destroy(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    struct drm_gem_close close_bo;
    int ret;

    free(bufmgr_gem->exec2_objects);
    free(bufmgr_gem->exec_objects);
    free(bufmgr_gem->exec_bos);
    pthread_mutex_destroy(&bufmgr_gem->lock);

    /* Free any cached buffer objects we were going to reuse */
    mos_bufmgr_cleanup_cache(bufmgr_gem);

    /* Release userptr bo kept hanging around for optimisation. */
    if (bufmgr_gem->userptr_active.ptr) {
        memclear(close_bo);
        close_bo.handle = bufmgr_gem->userptr_active.handle;
        ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_GEM_CLOSE, &close_bo);
        free(bufmgr_gem->userptr_active.ptr);
        if (ret)
            fprintf(stderr,
                "Failed to release test userptr object! (%d) "
                "i915 kernel driver may not be sane!\n", errno);
    }

    mos_vma_heap_finish(&bufmgr_gem->vma_heap[MEMZONE_SYS]);
    mos_vma_heap_finish(&bufmgr_gem->vma_heap[MEMZONE_DEVICE]);

    if (bufmgr_gem->mem_profiler_fd != -1)
    {
        close(bufmgr_gem->mem_profiler_fd);
    }

    free(bufmgr);
}

static int
do_bo_emit_reloc(struct mos_linux_bo *bo, uint32_t offset,
         struct mos_linux_bo *target_bo, uint32_t target_offset,
         uint32_t read_domains, uint32_t write_domain,
         bool need_fence, uint64_t presumed_offset)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *) target_bo;

    if (bo_gem->has_error)
        return -ENOMEM;

    if (target_bo_gem->has_error) {
        bo_gem->has_error = true;
        return -ENOMEM;
    }

    /* Create a new relocation list if needed */
    if (bo_gem->relocs == nullptr && mos_setup_reloc_list(bo))
        return -ENOMEM;

    /* Check overflow */
    assert(bo_gem->reloc_count < bufmgr_gem->max_relocs);

    /* Check args */
    assert(offset <= bo->size - 4);
    assert((write_domain & (write_domain - 1)) == 0);

    /* An object needing a fence is a tiled buffer, so it won't have
     * relocs to other buffers.
     */
    if (need_fence) {
        assert(target_bo_gem->reloc_count == 0);
        target_bo_gem->reloc_tree_fences = 1;
    }

    /* Make sure that we're not adding a reloc to something whose size has
     * already been accounted for.
     */
    assert(!bo_gem->used_as_reloc_target);
    if (target_bo_gem != bo_gem) {
        target_bo_gem->used_as_reloc_target = true;
        bo_gem->reloc_tree_size += target_bo_gem->reloc_tree_size;
        bo_gem->reloc_tree_fences += target_bo_gem->reloc_tree_fences;
    }

    int flags = 0;
    if (target_bo_gem->pad_to_size)
        flags |= EXEC_OBJECT_PAD_TO_SIZE;
    if (target_bo_gem->use_48b_address_range)
        flags |= EXEC_OBJECT_SUPPORTS_48B_ADDRESS;
    if (target_bo_gem->exec_async)
        flags |= EXEC_OBJECT_ASYNC;
    if (target_bo_gem->exec_capture)
        flags |= EXEC_OBJECT_CAPTURE;

    if (target_bo != bo)
        mos_gem_bo_reference(target_bo);

    bo_gem->reloc_target_info[bo_gem->reloc_count].bo = target_bo;
    bo_gem->reloc_target_info[bo_gem->reloc_count].flags = flags;
    bo_gem->relocs[bo_gem->reloc_count].offset = offset;
    bo_gem->relocs[bo_gem->reloc_count].delta = target_offset;
    bo_gem->relocs[bo_gem->reloc_count].target_handle =
        target_bo_gem->gem_handle;
    bo_gem->relocs[bo_gem->reloc_count].read_domains = read_domains;
    bo_gem->relocs[bo_gem->reloc_count].write_domain = write_domain;
    bo_gem->relocs[bo_gem->reloc_count].presumed_offset = presumed_offset;
    bo_gem->reloc_count++;

    return 0;
}

static void
mos_gem_bo_use_48b_address_range(struct mos_linux_bo *bo, uint32_t enable)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    bo_gem->use_48b_address_range = enable;
}

static void
mos_gem_bo_set_object_async(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *)bo;
    bo_gem->exec_async = true;
}

static void
mos_gem_bo_set_exec_object_async(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *) target_bo;
    int i;
    for (i = 0; i < bo_gem->reloc_count; i++)
    {
        if (bo_gem->reloc_target_info[i].bo == target_bo)
        {
            bo_gem->reloc_target_info[i].flags |= EXEC_OBJECT_ASYNC;
            break;
        }
    }

    for (i = 0; i < bo_gem->softpin_target_count; i++)
    {
        if (bo_gem->softpin_target[i].bo == target_bo)
        {
            bo_gem->softpin_target[i].flags |= EXEC_OBJECT_ASYNC;
            break;
        }
    }
}

static void
mos_gem_bo_set_object_capture(struct mos_linux_bo *bo)
{
    // Do nothing if bo is nullptr
    if (bo == nullptr)
    {
        return;
    }
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *)bo;
    if (bufmgr_gem != nullptr &&
        bo_gem != nullptr &&
        !bufmgr_gem->object_capture_disabled)
    {
        bo_gem->exec_capture = true;
    }
}

static int
mos_gem_bo_add_softpin_target(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo, bool write_flag)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *) target_bo;
    if (bo_gem->has_error)
        return -ENOMEM;

    if (target_bo_gem->has_error) {
        bo_gem->has_error = true;
        return -ENOMEM;
    }

    if (!target_bo_gem->is_softpin)
        return -EINVAL;
    if (target_bo_gem == bo_gem)
        return -EINVAL;

    if (bo_gem->softpin_target_count == bo_gem->max_softpin_target_count) {
        int max_softpin_target_count = bo_gem->max_softpin_target_count * 2;

        /* initial softpin target count*/
        if (max_softpin_target_count == 0){
            max_softpin_target_count = INITIAL_SOFTPIN_TARGET_COUNT;
        }

        bo_gem->softpin_target = (struct mos_softpin_target *)realloc(bo_gem->softpin_target, max_softpin_target_count *
                sizeof(struct mos_softpin_target));
        if (!bo_gem->softpin_target)
            return -ENOMEM;

        bo_gem->max_softpin_target_count = max_softpin_target_count;
    }

    int flags = EXEC_OBJECT_PINNED;
    if (target_bo_gem->pad_to_size)
        flags |= EXEC_OBJECT_PAD_TO_SIZE;
    if (target_bo_gem->use_48b_address_range)
        flags |= EXEC_OBJECT_SUPPORTS_48B_ADDRESS;
    if (target_bo_gem->exec_async)
        flags |= EXEC_OBJECT_ASYNC;
    if (target_bo_gem->exec_capture)
        flags |= EXEC_OBJECT_CAPTURE;
    if (write_flag)
        flags |= EXEC_OBJECT_WRITE;

    bo_gem->softpin_target[bo_gem->softpin_target_count].bo = target_bo;
    bo_gem->softpin_target[bo_gem->softpin_target_count].flags = flags;
    mos_gem_bo_reference(target_bo);
    bo_gem->softpin_target_count++;

    return 0;
}

static mos_oca_exec_list_info*
mos_bufmgr_bo_get_softpin_targets_info(struct mos_linux_bo *bo, int *count)
{
    if(bo == nullptr || count == nullptr)
    {
        return nullptr;
    }
    mos_oca_exec_list_info *info = nullptr;
    std::vector<int> bo_added;
    int counter = 0;
    int MAX_COUNT = 50;
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *)bo;
    int softpin_target_count = bo_gem->softpin_target_count;
    if(softpin_target_count == 0 || softpin_target_count > MAX_COUNT)
    {
        return info;
    }
    info = (mos_oca_exec_list_info *)malloc((softpin_target_count + 1) * sizeof(mos_oca_exec_list_info));
    if(info == nullptr)
    {
        return info;
    }

    for(int i = 0; i < softpin_target_count; i++)
    {
        /*note: set capture for each bo*/
        struct mos_softpin_target *target = (struct mos_softpin_target *)&bo_gem->softpin_target[i];
        struct mos_bo_gem *target_gem = (struct mos_bo_gem *)target->bo;
        if(std::find(bo_added.begin(), bo_added.end(), target->bo->handle) == bo_added.end())
        {
            info[counter].handle   = target->bo->handle;
            info[counter].size     = target->bo->size;
            info[counter].offset64 = target->bo->offset64;

            if (!bufmgr_gem->object_capture_disabled)
                target->flags   |= EXEC_OBJECT_CAPTURE;

            info[counter].flags    = target->flags;
            info[counter].mem_region = target_gem->mem_region;
            info[counter].is_batch = false;
            bo_added.push_back(target->bo->handle);
            counter++;
        }
    }

    /*note: bo is cmd bo, also need to be added*/
    int bb_flags = 0;
    if (bo_gem->pad_to_size)
        bb_flags |= EXEC_OBJECT_PAD_TO_SIZE;
    if (bo_gem->use_48b_address_range)
        bb_flags |= EXEC_OBJECT_SUPPORTS_48B_ADDRESS;
    if (bo_gem->is_softpin)
        bb_flags |= EXEC_OBJECT_PINNED;
    if (bo_gem->exec_async)
        bb_flags |= EXEC_OBJECT_ASYNC;
    if (bo_gem->exec_capture)
        bb_flags |= EXEC_OBJECT_CAPTURE;

    info[counter].handle   = bo->handle;
    info[counter].size     = bo->size;
    info[counter].offset64 = bo->offset64;
    info[counter].flags    = bb_flags;
    info[counter].mem_region = bo_gem->mem_region;
    info[counter].is_batch = true;
    counter++;

    *count = counter;

    return info;
}

static int
mos_gem_bo_pad_to_size(struct mos_linux_bo *bo, uint64_t pad_to_size)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    if (pad_to_size && pad_to_size < bo->size)
        return -EINVAL;

    bo_gem->pad_to_size = pad_to_size;
    return 0;
}

static int
mos_gem_bo_emit_reloc(struct mos_linux_bo *bo, uint32_t offset,
                struct mos_linux_bo *target_bo, uint32_t target_offset,
                uint32_t read_domains, uint32_t write_domain,
                uint64_t presumed_offset)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bo->bufmgr;

    return do_bo_emit_reloc(bo, offset, target_bo, target_offset,
                read_domains, write_domain,
                false,
                presumed_offset);
}

/**
 * Removes existing relocation entries in the BO after "start".
 *
 * This allows a user to avoid a two-step process for state setup with
 * counting up all the buffer objects and doing a
 * drm_intel_bufmgr_check_aperture_space() before emitting any of the
 * relocations for the state setup.  Instead, save the state of the
 * batchbuffer including drm_intel_gem_get_reloc_count(), emit all the
 * state, and then check if it still fits in the aperture.
 *
 * Any further drm_intel_bufmgr_check_aperture_space() queries
 * involving this buffer in the tree are undefined after this call.
 *
 * This also removes all softpinned targets being referenced by the BO.
 */
static void
mos_gem_bo_clear_relocs(struct mos_linux_bo *bo, int start)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int i;
    struct timespec time;

    clock_gettime(CLOCK_MONOTONIC, &time);

    assert(bo_gem->reloc_count >= start);

    /* Unreference the cleared target buffers */
    pthread_mutex_lock(&bufmgr_gem->lock);

    for (i = start; i < bo_gem->reloc_count; i++) {
        struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *) bo_gem->reloc_target_info[i].bo;
        if (&target_bo_gem->bo != bo) {
            bo_gem->reloc_tree_fences -= target_bo_gem->reloc_tree_fences;
            target_bo_gem->used_as_reloc_target = false;
            target_bo_gem->reloc_count = 0;
            mos_gem_bo_unreference_locked_timed(&target_bo_gem->bo,
                                  time.tv_sec);
        }
    }
    bo_gem->reloc_count = start;

    for (i = 0; i < bo_gem->softpin_target_count; i++) {
        struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *) bo_gem->softpin_target[i].bo;
        mos_gem_bo_unreference_locked_timed(&target_bo_gem->bo, time.tv_sec);
    }
    bo_gem->softpin_target_count = 0;

    pthread_mutex_unlock(&bufmgr_gem->lock);

}

/**
 * Walk the tree of relocations rooted at BO and accumulate the list of
 * validations to be performed and update the relocation buffers with
 * index values into the validation list.
 */
static void
mos_gem_bo_process_reloc(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int i;

    if (bo_gem->relocs == nullptr)
        return;

    for (i = 0; i < bo_gem->reloc_count; i++) {
        struct mos_linux_bo *target_bo = bo_gem->reloc_target_info[i].bo;

        if (target_bo == bo)
            continue;

        mos_gem_bo_mark_mmaps_incoherent(bo);

        /* Continue walking the tree depth-first. */
        mos_gem_bo_process_reloc(target_bo);

        /* Add the target to the validate list */
        mos_add_validate_buffer(target_bo);
    }
}

static void
mos_gem_bo_process_reloc2(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *)bo;
    int i;

    if (bo_gem->relocs == nullptr && bo_gem->softpin_target == nullptr)
        return;

    for (i = 0; i < bo_gem->reloc_count; i++) {
        struct mos_linux_bo *target_bo = bo_gem->reloc_target_info[i].bo;

        if (target_bo == bo)
            continue;

        mos_gem_bo_mark_mmaps_incoherent(bo);

        /* Continue walking the tree depth-first. */
        mos_gem_bo_process_reloc2(target_bo);

        /* Add the target to the validate list */
        mos_add_reloc_objects(bo_gem->reloc_target_info[i]);
    }

    for (i = 0; i < bo_gem->softpin_target_count; i++) {
        struct mos_linux_bo *target_bo = bo_gem->softpin_target[i].bo;

        if (target_bo == bo)
            continue;

        mos_gem_bo_mark_mmaps_incoherent(bo);
        mos_gem_bo_process_reloc2(target_bo);
        mos_add_softpin_objects(bo_gem->softpin_target[i]);
    }
}

static void
mos_update_buffer_offsets(struct mos_bufmgr_gem *bufmgr_gem)
{
    int i;

    for (i = 0; i < bufmgr_gem->exec_count; i++) {
        struct mos_linux_bo *bo = bufmgr_gem->exec_bos[i];
        struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

        /* Update the buffer offset */
        if (bufmgr_gem->exec_objects[i].offset != bo->offset64) {
            MOS_DBG("BO %d (%s) migrated: 0x%08x %08x -> 0x%08x %08x\n",
                bo_gem->gem_handle, bo_gem->name,
                upper_32_bits(bo->offset64),
                lower_32_bits(bo->offset64),
                upper_32_bits(bufmgr_gem->exec_objects[i].offset),
                lower_32_bits(bufmgr_gem->exec_objects[i].offset));
            bo->offset64 = bufmgr_gem->exec_objects[i].offset;
            bo->offset = bufmgr_gem->exec_objects[i].offset;
        }
    }
}

static void
mos_update_buffer_offsets2 (struct mos_bufmgr_gem *bufmgr_gem, mos_linux_context *ctx, mos_linux_bo *cmd_bo)
{
    int i;

    for (i = 0; i < bufmgr_gem->exec_count; i++) {
        struct mos_linux_bo *bo = bufmgr_gem->exec_bos[i];
        struct mos_bo_gem *bo_gem = (struct mos_bo_gem *)bo;

        /* Update the buffer offset */
        if (bufmgr_gem->exec2_objects[i].offset != bo->offset64) {
            /* If we're seeing softpinned object here it means that the kernel
             * has relocated our object... Indicating a programming error
             */
            assert(!bo_gem->is_softpin);
            MOS_DBG("BO %d (%s) migrated: 0x%08x %08x -> 0x%08x %08x\n",
                bo_gem->gem_handle, bo_gem->name,
                upper_32_bits(bo->offset64),
                lower_32_bits(bo->offset64),
                upper_32_bits(bufmgr_gem->exec2_objects[i].offset),
                lower_32_bits(bufmgr_gem->exec2_objects[i].offset));
            bo->offset64 = bufmgr_gem->exec2_objects[i].offset;
            bo->offset = bufmgr_gem->exec2_objects[i].offset;
        }

        if(!bufmgr_gem->use_softpin)
        {
            if (cmd_bo != bo) {
                auto item_ctx = ctx->pOsContext->contextOffsetList.begin();
                for (; item_ctx != ctx->pOsContext->contextOffsetList.end(); item_ctx++) {
                    if (item_ctx->intel_context == ctx && item_ctx->target_bo == bo) {
                        item_ctx->offset64 = bo->offset64;
                        break;
                    }
                }
                if ( item_ctx == ctx->pOsContext->contextOffsetList.end()) {
                    struct MOS_CONTEXT_OFFSET newContext = {ctx,
                                        bo,
                                        bo->offset64};
                    ctx->pOsContext->contextOffsetList.push_back(newContext);
                }
            }
        }
    }
}

drm_export int
do_exec2(struct mos_linux_bo *bo, int used, struct mos_linux_context *ctx,
     drm_clip_rect_t *cliprects, int num_cliprects, int DR4,
     unsigned int flags, int *fence
     )
{

    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bo->bufmgr;
    struct drm_i915_gem_execbuffer2 execbuf;
    int ret = 0;
    int i;

    if (to_bo_gem(bo)->has_error)
        return -ENOMEM;

    switch (flags & 0x7) {
    default:
        return -EINVAL;
    case I915_EXEC_BLT:
        if (!bufmgr_gem->has_blt)
            return -EINVAL;
        break;
    case I915_EXEC_BSD:
        if (!bufmgr_gem->has_bsd)
            return -EINVAL;
        break;
    case I915_EXEC_VEBOX:
        if (!bufmgr_gem->has_vebox)
            return -EINVAL;
        break;
    case I915_EXEC_RENDER:
    case I915_EXEC_DEFAULT:
        break;
    }

    pthread_mutex_lock(&bufmgr_gem->lock);
    /* Update indices and set up the validate list. */
    mos_gem_bo_process_reloc2(bo);

    /* Add the batch buffer to the validation list.  There are no relocations
     * pointing to it.
     */
    mos_add_validate_buffer2(bo, 0);

    memclear(execbuf);
    execbuf.buffers_ptr = (uintptr_t)bufmgr_gem->exec2_objects;
    execbuf.buffer_count = bufmgr_gem->exec_count;
    execbuf.batch_start_offset = 0;
    execbuf.batch_len = used;
    execbuf.cliprects_ptr = (uintptr_t)cliprects;
    execbuf.num_cliprects = num_cliprects;
    execbuf.DR1 = 0;
    execbuf.DR4 = DR4;
    execbuf.flags = flags;
    if (ctx == nullptr)
        i915_execbuffer2_set_context_id(execbuf, 0);
    else
        i915_execbuffer2_set_context_id(execbuf, ctx->ctx_id);
    execbuf.rsvd2 = 0;
    if(flags & I915_EXEC_FENCE_SUBMIT)
    {
        execbuf.rsvd2 = *fence;
    }
    if(flags & I915_EXEC_FENCE_OUT)
    {
        execbuf.rsvd2 = -1;
    }

    if (bufmgr_gem->no_exec)
        goto skip_execution;

    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_EXECBUFFER2_WR,
               &execbuf);
    if (ret != 0) {
        ret = -errno;
        if (ret == -ENOSPC) {
            MOS_DBG("Execbuffer fails to pin. "
                "Estimate: %u. Actual: %u. Available: %u\n",
                mos_gem_estimate_batch_space(bufmgr_gem->exec_bos,
                                   bufmgr_gem->exec_count),
                mos_gem_compute_batch_space(bufmgr_gem->exec_bos,
                                  bufmgr_gem->exec_count),
                (unsigned int) bufmgr_gem->gtt_size);
        }
    }

    if (ctx != nullptr)
    {
        mos_update_buffer_offsets2(bufmgr_gem, ctx, bo);
    }

    if(flags & I915_EXEC_FENCE_OUT)
    {
        *fence = execbuf.rsvd2 >> 32;
    }

skip_execution:
    if (bufmgr_gem->bufmgr.debug)
        mos_gem_dump_validation_list(bufmgr_gem);

    for (i = 0; i < bufmgr_gem->exec_count; i++) {
        struct mos_bo_gem *bo_gem = to_bo_gem(bufmgr_gem->exec_bos[i]);

        bo_gem->idle = false;

        /* Disconnect the buffer from the validate list */
        bo_gem->validate_index = -1;
        bufmgr_gem->exec_bos[i] = nullptr;
    }
    bufmgr_gem->exec_count = 0;
    pthread_mutex_unlock(&bufmgr_gem->lock);

    return ret;
}

static int
do_exec3(struct mos_linux_bo **bo, int _num_bo, struct mos_linux_context *ctx,
     drm_clip_rect_t *cliprects, int num_cliprects, int DR4,
     unsigned int _flags, int *fence
     )
{
    uint64_t flags = _flags;
    uint64_t num_bo = _num_bo;
    if((bo == nullptr) || (ctx == nullptr) || (num_bo == 0))
    {
        return -EINVAL;
    }

    struct mos_bufmgr_gem           *bufmgr_gem = (struct mos_bufmgr_gem *)bo[0]->bufmgr;
    struct drm_i915_gem_execbuffer2 execbuf;
    int                             ret = 0;
    int                             i;

    pthread_mutex_lock(&bufmgr_gem->lock);

    struct mos_exec_info exec_info;
    memset(static_cast<void*>(&exec_info), 0, sizeof(exec_info));
    exec_info.batch_obj = (struct drm_i915_gem_exec_object2 *) calloc (num_bo, sizeof(struct drm_i915_gem_exec_object2));
    if(exec_info.batch_obj == nullptr)
    {
        ret = -ENOMEM;
        goto skip_execution;
    }
    exec_info.obj_remain_size = OBJ512_SIZE;
    exec_info.obj = (struct drm_i915_gem_exec_object2 *) calloc (OBJ512_SIZE, sizeof(struct drm_i915_gem_exec_object2));
    if(exec_info.obj == nullptr)
    {
        ret = -ENOMEM;
        goto skip_execution;
    }

    for(i = 0; i < num_bo; i++)
    {
        if (to_bo_gem(bo[i])->has_error)
        {
            ret = -ENOMEM;
            goto skip_execution;
        }

        /* Update indices and set up the validate list. */
        mos_gem_bo_process_reloc2(bo[i]);

        /* Add the batch buffer to the validation list.  There are no relocations
         * pointing to it.
         */
        mos_add_validate_buffer2(bo[i], 0);

        if((bufmgr_gem->exec_count - 1 + num_bo) > exec_info.obj_remain_size)
        {
            // origin size + OBJ512_SIZE + obj_count + batch_count;
            uint32_t new_obj_size = exec_info.obj_count + exec_info.obj_remain_size + OBJ512_SIZE + bufmgr_gem->exec_count - 1 + num_bo;
            struct drm_i915_gem_exec_object2 *new_obj = (struct drm_i915_gem_exec_object2 *)realloc(exec_info.obj, new_obj_size * sizeof(struct drm_i915_gem_exec_object2));
            if(new_obj == nullptr)
            {
                ret = -ENOMEM;
                goto skip_execution;
            }
            exec_info.obj_remain_size = new_obj_size - exec_info.obj_count;
            exec_info.obj = new_obj;
        }
        if(0 == i)
        {
            uint32_t cp_size = (bufmgr_gem->exec_count - 1) * sizeof(struct drm_i915_gem_exec_object2);
            memcpy(exec_info.obj, bufmgr_gem->exec2_objects, cp_size);
            exec_info.obj_count += (bufmgr_gem->exec_count - 1);
            exec_info.obj_remain_size -= (bufmgr_gem->exec_count - 1);
        }
        else
        {
            for(int e2 = 0; e2 < bufmgr_gem->exec_count - 1; e2++)
            {
                int e1;
                for(e1 = 0; e1 < exec_info.obj_count; e1++)
                {
                    // skip the duplicated bo if it is already in the list of exec_info.obj
                    if(bufmgr_gem->exec2_objects[e2].handle == exec_info.obj[e1].handle)
                    {
                        break;
                    }
                }
                //if no duplicated bo found, add it into list of exec_info.obj
                if(e1 == exec_info.obj_count)
                {
                    exec_info.obj[exec_info.obj_count] = bufmgr_gem->exec2_objects[e2];
                    exec_info.obj_count++;
                    exec_info.obj_remain_size--;
                }
            }
        }
        memcpy(&exec_info.batch_obj[i], &bufmgr_gem->exec2_objects[bufmgr_gem->exec_count - 1], sizeof(struct drm_i915_gem_exec_object2));
        exec_info.batch_count++;
        uint32_t reloc_count = bufmgr_gem->exec2_objects[bufmgr_gem->exec_count - 1].relocation_count;
        uint32_t cp_size = (reloc_count * sizeof(struct drm_i915_gem_relocation_entry));
        
        struct drm_i915_gem_relocation_entry* ptr_reloc = (struct drm_i915_gem_relocation_entry *)calloc(reloc_count,sizeof(struct drm_i915_gem_relocation_entry));
        if(ptr_reloc == nullptr)
        {
            ret = -ENOMEM;
            goto skip_execution;
        }
        memcpy(ptr_reloc, (struct drm_i915_gem_relocation_entry *)bufmgr_gem->exec2_objects[bufmgr_gem->exec_count - 1].relocs_ptr, cp_size);

        exec_info.batch_obj[i].relocs_ptr = (uintptr_t)ptr_reloc;
        exec_info.batch_obj[i].relocation_count = reloc_count;

        //clear bo
        if (bufmgr_gem->bufmgr.debug)
        {
            mos_gem_dump_validation_list(bufmgr_gem);
        }

        for (int j = 0; j < bufmgr_gem->exec_count; j++) {
            struct mos_bo_gem *bo_gem = to_bo_gem(bufmgr_gem->exec_bos[j]);

            if(bo_gem)
            {
                bo_gem->idle = false;

                /* Disconnect the buffer from the validate list */
                bo_gem->validate_index = -1;
                bufmgr_gem->exec_bos[j] = nullptr;
            }
        }
        bufmgr_gem->exec_count = 0;
    }

    //add back batch obj to the last position
    for(i = 0; i < num_bo; i++)
    {
       exec_info.obj[exec_info.obj_count] = exec_info.batch_obj[i];
       exec_info.obj_count++;
       exec_info.obj_remain_size--;
    }

    //save previous ptr
    exec_info.pSavePreviousExec2Objects = bufmgr_gem->exec2_objects;
    bufmgr_gem->exec_count = exec_info.obj_count;
    bufmgr_gem->exec2_objects = exec_info.obj;

    memclear(execbuf);
    execbuf.buffers_ptr = (uintptr_t)bufmgr_gem->exec2_objects;
    execbuf.buffer_count = bufmgr_gem->exec_count;
    execbuf.batch_start_offset = 0;
    execbuf.cliprects_ptr = (uintptr_t)cliprects;
    execbuf.num_cliprects = num_cliprects;
    execbuf.DR1 = 0;
    execbuf.DR4 = DR4;
    execbuf.flags = flags;
    if (ctx == nullptr)
        i915_execbuffer2_set_context_id(execbuf, 0);
    else
        i915_execbuffer2_set_context_id(execbuf, ctx->ctx_id);
    execbuf.rsvd2 = 0;
    if((flags & I915_EXEC_FENCE_SUBMIT) || (flags & I915_EXEC_FENCE_IN))
    {
        execbuf.rsvd2 = *fence;
    }
    else if(flags & I915_EXEC_FENCE_OUT)
    {
        execbuf.rsvd2 = -1;
    }

    if (bufmgr_gem->no_exec)
        goto skip_execution;

   ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_EXECBUFFER2_WR,
               &execbuf);

    if (ret != 0) {
        ret = -errno;
        if (ret == -ENOSPC) {
            MOS_DBG("Execbuffer fails to pin. "
                "Estimate: %u. Actual: %u. Available: %u\n",
                mos_gem_estimate_batch_space(bufmgr_gem->exec_bos,
                                   bufmgr_gem->exec_count),
                mos_gem_compute_batch_space(bufmgr_gem->exec_bos,
                                  bufmgr_gem->exec_count),
                (unsigned int) bufmgr_gem->gtt_size);
        }
    }

    bufmgr_gem->exec2_objects = exec_info.pSavePreviousExec2Objects;

    if(flags & I915_EXEC_FENCE_OUT)
    {
        *fence = execbuf.rsvd2 >> 32;
    }

skip_execution:
    if (bufmgr_gem->bufmgr.debug)
        mos_gem_dump_validation_list(bufmgr_gem);

    bufmgr_gem->exec_count = 0;
    if(exec_info.batch_obj)
    {
        for(i = 0; i < num_bo; i++)
        {
            mos_safe_free((struct drm_i915_gem_relocation_entry *)exec_info.batch_obj[i].relocs_ptr);
        }
    }
    mos_safe_free(exec_info.obj);
    mos_safe_free(exec_info.batch_obj);
    pthread_mutex_unlock(&bufmgr_gem->lock);

    return ret;
}

static int
mos_gem_bo_exec2(struct mos_linux_bo *bo, int used,
               drm_clip_rect_t *cliprects, int num_cliprects,
               int DR4)
{
    return do_exec2(bo, used, nullptr, cliprects, num_cliprects, DR4,
            I915_EXEC_RENDER, nullptr);
}

static int
mos_gem_bo_mrb_exec2(struct mos_linux_bo *bo, int used,
            drm_clip_rect_t *cliprects, int num_cliprects, int DR4,
            unsigned int flags)
{
    return do_exec2(bo, used, nullptr, cliprects, num_cliprects, DR4,
            flags, nullptr);
}

static int
mos_gem_bo_context_exec2(struct mos_linux_bo *bo, int used, struct mos_linux_context *ctx,
                           drm_clip_rect_t *cliprects, int num_cliprects, int DR4,
                           unsigned int flags, int *fence)
{
    return do_exec2(bo, used, ctx, cliprects, num_cliprects, DR4,
                        flags, fence);
}

static int
mos_gem_bo_context_exec3(struct mos_linux_bo **bo, int num_bo, struct mos_linux_context *ctx,
                               struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                               unsigned int flags, int *fence)
{
    return do_exec3(bo, num_bo, ctx, cliprects, num_cliprects, DR4,
                        flags, fence);
}

static int
mos_gem_bo_check_mem_region_internal(struct mos_linux_bo *bo,
                     int mem_type)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    // when re-cycle the gem_bo, need to keep the VA space is keeping consistent on memory type
    if (bo_gem->mem_region ==  I915_MEMORY_CLASS_SYSTEM                            &&
        (mem_type == MOS_MEMPOOL_VIDEOMEMORY || mem_type == MOS_MEMPOOL_DEVICEMEMORY))
        return -EINVAL;

    if (bo_gem->mem_region ==  I915_MEMORY_CLASS_DEVICE                      &&
        (mem_type == MOS_MEMPOOL_SYSTEMMEMORY))
        return -EINVAL;

    return 0;
}

static int
mos_gem_bo_set_tiling_internal(struct mos_linux_bo *bo,
                     uint32_t tiling_mode,
                     uint32_t stride)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_set_tiling set_tiling;
    int ret;

    if (!bufmgr_gem->has_fence_reg)
        return 0;

    if (bo_gem->global_name == 0 &&
        tiling_mode == bo_gem->tiling_mode &&
        stride == bo_gem->stride)
        return 0;

    memset(&set_tiling, 0, sizeof(set_tiling));
    /* set_tiling is slightly broken and overwrites the
     * input on the error path, so we have to open code
     * rmIoctl.
     */
    set_tiling.handle = bo_gem->gem_handle;
    set_tiling.tiling_mode = tiling_mode;
    set_tiling.stride = stride;

    ret = drmIoctl(bufmgr_gem->fd,
            DRM_IOCTL_I915_GEM_SET_TILING,
            &set_tiling);
    if (ret == -1)
        return -errno;

    bo_gem->tiling_mode = set_tiling.tiling_mode;
    bo_gem->swizzle_mode = set_tiling.swizzle_mode;
    bo_gem->stride = set_tiling.stride;
    return 0;
}

static int
mos_gem_bo_set_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
                uint32_t stride)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int ret;

    /* Tiling with userptr surfaces is not supported
     * on all hardware so refuse it for time being.
     */
    if (bo_gem->is_userptr)
        return -EINVAL;

    /* Linear buffers have no stride. By ensuring that we only ever use
     * stride 0 with linear buffers, we simplify our code.
     */
    if (*tiling_mode == I915_TILING_NONE)
        stride = 0;

    ret = mos_gem_bo_set_tiling_internal(bo, *tiling_mode, stride);
    if (ret == 0)
        mos_bo_gem_set_in_aperture_size(bufmgr_gem, bo_gem, 0);
    *tiling_mode = bo_gem->tiling_mode;
    return ret;
}

static int
mos_gem_bo_get_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
                uint32_t * swizzle_mode)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    *tiling_mode = bo_gem->tiling_mode;
    *swizzle_mode = bo_gem->swizzle_mode;
    return 0;
}

static int
mos_gem_bo_set_softpin_offset(struct mos_linux_bo *bo, uint64_t offset)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    bo_gem->is_softpin = true;
    bo->offset64 = offset;
    bo->offset = offset;
    return 0;
}

static int
mos_gem_bo_set_softpin(MOS_LINUX_BO *bo)
{
    int ret = 0;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;

    pthread_mutex_lock(&bufmgr_gem->lock);
    if (!mos_gem_bo_is_softpin(bo))
    {
        uint64_t alignment = (bufmgr_gem->softpin_va1Malign) ? PAGE_SIZE_1M : PAGE_SIZE_64K;
        uint64_t offset = mos_gem_bo_vma_alloc(bo->bufmgr, (enum mos_memory_zone)bo_gem->mem_region, bo->size, alignment);
        ret = mos_gem_bo_set_softpin_offset(bo, offset);
    }
    pthread_mutex_unlock(&bufmgr_gem->lock);

    if (ret == 0)
    {
        ret = mos_bo_use_48b_address_range(bo, 1);
    }
    
    return ret;
}

static struct mos_linux_bo *
mos_gem_bo_create_from_prime(struct mos_bufmgr *bufmgr, struct mos_drm_bo_alloc_prime *alloc_prime)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;
    int ret;
    uint32_t handle;
    struct mos_bo_gem *bo_gem;
    struct drm_i915_gem_get_tiling get_tiling;
    int prime_fd = alloc_prime->prime_fd;
    int size = alloc_prime->size;
    drmMMListHead *list;

    pthread_mutex_lock(&bufmgr_gem->lock);
    ret = drmPrimeFDToHandle(bufmgr_gem->fd, prime_fd, &handle);
    if (ret) {
        MOS_DBG("create_from_prime: failed to obtain handle from fd: %s\n", strerror(errno));
        pthread_mutex_unlock(&bufmgr_gem->lock);
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
        bo_gem = DRMLISTENTRY(struct mos_bo_gem, list, name_list);
        if (bo_gem->gem_handle == handle) {
            mos_gem_bo_reference(&bo_gem->bo);
            pthread_mutex_unlock(&bufmgr_gem->lock);
            return &bo_gem->bo;
        }
    }

    bo_gem = (struct mos_bo_gem *)calloc(1, sizeof(*bo_gem));
    if (!bo_gem) {
        pthread_mutex_unlock(&bufmgr_gem->lock);
        return nullptr;
    }
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

    bo_gem->gem_handle    = handle;
    bo_gem->pat_index     = PAT_INDEX_INVALID;
    bo_gem->cpu_cacheable = true;
    atomic_set(&bo_gem->refcount, 1);

    bo_gem->name = alloc_prime->name;
    bo_gem->validate_index = -1;
    bo_gem->reloc_tree_fences = 0;
    bo_gem->used_as_reloc_target = false;
    bo_gem->has_error = false;
    bo_gem->reusable = false;
    bo_gem->use_48b_address_range = bufmgr_gem->bufmgr.bo_use_48b_address_range ? true : false;

    DRMLISTADDTAIL(&bo_gem->name_list, &bufmgr_gem->named);
    pthread_mutex_unlock(&bufmgr_gem->lock);

    memclear(get_tiling);
    if(bufmgr_gem->has_fence_reg) {
        get_tiling.handle = bo_gem->gem_handle;
        ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_GET_TILING,
               &get_tiling);
        if (ret != 0) {
            MOS_DBG("create_from_prime: failed to get tiling: %s\n", strerror(errno));
            mos_gem_bo_unreference(&bo_gem->bo);
            return nullptr;
        }
    }
    else
    {
        MOS_DBG("create_from_prime: driver ignored to get tiling from kernel\n");
    }

    bo_gem->tiling_mode = get_tiling.tiling_mode;
    bo_gem->swizzle_mode = get_tiling.swizzle_mode;
    /* XXX stride is unknown */
    mos_bo_gem_set_in_aperture_size(bufmgr_gem, bo_gem, 0);
    if (bufmgr_gem->use_softpin)
    {
        mos_bo_set_softpin(&bo_gem->bo);
    }

    return &bo_gem->bo;
}

static int
mos_gem_bo_export_to_prime(struct mos_linux_bo *bo, int *prime_fd)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    pthread_mutex_lock(&bufmgr_gem->lock);
        if (DRMLISTEMPTY(&bo_gem->name_list))
                DRMLISTADDTAIL(&bo_gem->name_list, &bufmgr_gem->named);
    pthread_mutex_unlock(&bufmgr_gem->lock);

    if (drmPrimeHandleToFD(bufmgr_gem->fd, bo_gem->gem_handle,
                   DRM_CLOEXEC, prime_fd) != 0)
        return -errno;

    bo_gem->reusable = false;

    return 0;
}

static int
mos_gem_bo_flink(struct mos_linux_bo *bo, uint32_t * name)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int ret;

    if (!bo_gem->global_name) {
        struct drm_gem_flink flink;

        memclear(flink);
        flink.handle = bo_gem->gem_handle;

        pthread_mutex_lock(&bufmgr_gem->lock);

        ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_GEM_FLINK, &flink);
        if (ret != 0) {
            pthread_mutex_unlock(&bufmgr_gem->lock);
            return -errno;
        }

        bo_gem->global_name = flink.name;
        bo_gem->reusable = false;

                if (DRMLISTEMPTY(&bo_gem->name_list))
                        DRMLISTADDTAIL(&bo_gem->name_list, &bufmgr_gem->named);
        pthread_mutex_unlock(&bufmgr_gem->lock);
    }

    *name = bo_gem->global_name;
    return 0;
}

/**
 * Enables unlimited caching of buffer objects for reuse.
 *
 * This is potentially very memory expensive, as the cache at each bucket
 * size is only bounded by how many buffers of that size we've managed to have
 * in flight at once.
 */
static void
mos_gem_enable_reuse(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;

    bufmgr_gem->bo_reuse = true;
}

/**
 * Return the additional aperture space required by the tree of buffer objects
 * rooted at bo.
 */
static int
mos_gem_bo_get_aperture_space(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int i;
    int total = 0;

    if (bo == nullptr || bo_gem->included_in_check_aperture)
        return 0;

    total += bo->size;
    bo_gem->included_in_check_aperture = true;

    for (i = 0; i < bo_gem->reloc_count; i++)
        total +=
            mos_gem_bo_get_aperture_space(bo_gem->
                            reloc_target_info[i].bo);

    return total;
}

/**
 * Count the number of buffers in this list that need a fence reg
 *
 * If the count is greater than the number of available regs, we'll have
 * to ask the caller to resubmit a batch with fewer tiled buffers.
 *
 * This function over-counts if the same buffer is used multiple times.
 */
static unsigned int
mos_gem_total_fences(struct mos_linux_bo ** bo_array, int count)
{
    int i;
    unsigned int total = 0;

    for (i = 0; i < count; i++) {
        struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo_array[i];

        if (bo_gem == nullptr)
            continue;

        total += bo_gem->reloc_tree_fences;
    }
    return total;
}

/**
 * Clear the flag set by drm_intel_gem_bo_get_aperture_space() so we're ready
 * for the next drm_intel_bufmgr_check_aperture_space() call.
 */
static void
mos_gem_bo_clear_aperture_space_flag(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int i;

    if (bo == nullptr || !bo_gem->included_in_check_aperture)
        return;

    bo_gem->included_in_check_aperture = false;

    for (i = 0; i < bo_gem->reloc_count; i++)
        mos_gem_bo_clear_aperture_space_flag(bo_gem->
                               reloc_target_info[i].bo);
}

/**
 * Return a conservative estimate for the amount of aperture required
 * for a collection of buffers. This may double-count some buffers.
 */
static unsigned int
mos_gem_estimate_batch_space(struct mos_linux_bo **bo_array, int count)
{
    int i;
    unsigned int total = 0;

    for (i = 0; i < count; i++) {
        struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo_array[i];
        if (bo_gem != nullptr)
            total += bo_gem->reloc_tree_size;
    }
    return total;
}

/**
 * Return the amount of aperture needed for a collection of buffers.
 * This avoids double counting any buffers, at the cost of looking
 * at every buffer in the set.
 */
static unsigned int
mos_gem_compute_batch_space(struct mos_linux_bo **bo_array, int count)
{
    int i;
    unsigned int total = 0;

    for (i = 0; i < count; i++) {
        total += mos_gem_bo_get_aperture_space(bo_array[i]);
        /* For the first buffer object in the array, we get an
         * accurate count back for its reloc_tree size (since nothing
         * had been flagged as being counted yet).  We can save that
         * value out as a more conservative reloc_tree_size that
         * avoids double-counting target buffers.  Since the first
         * buffer happens to usually be the batch buffer in our
         * callers, this can pull us back from doing the tree
         * walk on every new batch emit.
         */
        if (i == 0) {
            struct mos_bo_gem *bo_gem =
                (struct mos_bo_gem *) bo_array[i];
            bo_gem->reloc_tree_size = total;
        }
    }

    for (i = 0; i < count; i++)
        mos_gem_bo_clear_aperture_space_flag(bo_array[i]);
    return total;
}

/**
 * Return -1 if the batchbuffer should be flushed before attempting to
 * emit rendering referencing the buffers pointed to by bo_array.
 *
 * This is required because if we try to emit a batchbuffer with relocations
 * to a tree of buffers that won't simultaneously fit in the aperture,
 * the rendering will return an error at a point where the software is not
 * prepared to recover from it.
 *
 * However, we also want to emit the batchbuffer significantly before we reach
 * the limit, as a series of batchbuffers each of which references buffers
 * covering almost all of the aperture means that at each emit we end up
 * waiting to evict a buffer from the last rendering, and we get synchronous
 * performance.  By emitting smaller batchbuffers, we eat some CPU overhead to
 * get better parallelism.
 */
static int
mos_gem_check_aperture_space(struct mos_linux_bo **bo_array, int count)
{
    struct mos_bufmgr_gem *bufmgr_gem =
        (struct mos_bufmgr_gem *) bo_array[0]->bufmgr;
    unsigned int total = 0;
    unsigned int threshold = bufmgr_gem->gtt_size * 3 / 4;
    int total_fences;

    /* Check for fence reg constraints if necessary */
    if (bufmgr_gem->available_fences) {
        total_fences = mos_gem_total_fences(bo_array, count);
        if (total_fences > bufmgr_gem->available_fences)
            return -ENOSPC;
    }

    total = mos_gem_estimate_batch_space(bo_array, count);

    if (total > threshold)
        total = mos_gem_compute_batch_space(bo_array, count);

    if (total > threshold) {
        MOS_DBG("check_space: overflowed available aperture, "
            "%dkb vs %dkb\n",
            total / 1024, (int)bufmgr_gem->gtt_size / 1024);
        return -ENOSPC;
    } else {
        MOS_DBG("drm_check_space: total %dkb vs bufgr %dkb\n", total / 1024,
            (int)bufmgr_gem->gtt_size / 1024);
        return 0;
    }
}

/*
 * Disable buffer reuse for objects which are shared with the kernel
 * as scanout buffers
 */
static int
mos_gem_bo_disable_reuse(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    bo_gem->reusable = false;
    return 0;
}

static int
mos_gem_bo_is_reusable(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    return bo_gem->reusable;
}

static int
_mos_gem_bo_references(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    int i;

    for (i = 0; i < bo_gem->reloc_count; i++) {
        if (bo_gem->reloc_target_info[i].bo == target_bo)
            return 1;
        if (bo == bo_gem->reloc_target_info[i].bo)
            continue;
        if (_mos_gem_bo_references(bo_gem->reloc_target_info[i].bo,
                        target_bo))
            return 1;
    }

    for (i = 0; i< bo_gem->softpin_target_count; i++) {
        if (bo_gem->softpin_target[i].bo == target_bo)
            return 1;
        if (_mos_gem_bo_references(bo_gem->softpin_target[i].bo, target_bo))
            return 1;
    }

    return 0;
}

/** Return true if target_bo is referenced by bo's relocation tree. */
static int
mos_gem_bo_references(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo)
{
    struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *) target_bo;

    if (bo == nullptr || target_bo == nullptr)
        return 0;
    if (target_bo_gem->used_as_reloc_target)
        return _mos_gem_bo_references(bo, target_bo);
    return 0;
}

static void
add_bucket(struct mos_bufmgr_gem *bufmgr_gem, int size)
{
    unsigned int i = bufmgr_gem->num_buckets;

    assert(i < ARRAY_SIZE(bufmgr_gem->cache_bucket));

    DRMINITLISTHEAD(&bufmgr_gem->cache_bucket[i].head);
    bufmgr_gem->cache_bucket[i].size = size;
    bufmgr_gem->num_buckets++;
}

static void
init_cache_buckets(struct mos_bufmgr_gem *bufmgr_gem)
{
    unsigned long size, cache_max_size = 64 * 1024 * 1024;

    /* OK, so power of two buckets was too wasteful of memory.
     * Give 3 other sizes between each power of two, to hopefully
     * cover things accurately enough.  (The alternative is
     * probably to just go for exact matching of sizes, and assume
     * that for things like composited window resize the tiled
     * width/height alignment and rounding of sizes to pages will
     * get us useful cache hit rates anyway)
     */
    add_bucket(bufmgr_gem, 4096);
    add_bucket(bufmgr_gem, 4096 * 2);
    add_bucket(bufmgr_gem, 4096 * 3);

    /* Initialize the linked lists for BO reuse cache. */
    for (size = 4 * 4096; size <= cache_max_size; size *= 2) {
        add_bucket(bufmgr_gem, size);

        add_bucket(bufmgr_gem, size + size * 1 / 4);
        add_bucket(bufmgr_gem, size + size * 2 / 4);
        add_bucket(bufmgr_gem, size + size * 3 / 4);
    }
}

static void
mos_gem_realloc_cache(struct mos_bufmgr *bufmgr, uint8_t alloc_mode)
{
    unsigned long size, cache_max_size = 64 * 1024 * 1024, unit_size;
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;

    // Clean up the pre-allocated cache before re-allocating according
    // to alloc_mode
    mos_bufmgr_cleanup_cache(bufmgr_gem);

    /* OK, so power of two buckets was too wasteful of memory.
     * Give 3 other sizes between each power of two, to hopefully
     * cover things accurately enough.  (The alternative is
     * probably to just go for exact matching of sizes, and assume
     * that for things like composited window resize the tiled
     * width/height alignment and rounding of sizes to pages will
     * get us useful cache hit rates anyway)
     */
    /* alloc_mode 0 is default alloc_mode
     * alloc_mode 1 rounding up to 64K for all < 1M
     * alloc_mode 2 rounding up to 2M for size> 1M
     * alloc_mode 3 rounding up to 2M for size > 1M and 64K for size <= 1M */
    if( alloc_mode > 3 )
        alloc_mode = 0;

    if ( 0 == alloc_mode || 2 == alloc_mode)
    {
        // < 1M normal alloc_mode
        add_bucket(bufmgr_gem, 4096);
        add_bucket(bufmgr_gem, 4096 * 2);
        add_bucket(bufmgr_gem, 4096 * 3);
        /* Initialize the linked lists for BO reuse cache. */
        for (size = 4 * 4096; size < 1024 * 1024; size *= 2) {
            add_bucket(bufmgr_gem, size);
            add_bucket(bufmgr_gem, size + size * 1 / 4);
            add_bucket(bufmgr_gem, size + size * 2 / 4);
            add_bucket(bufmgr_gem, size + size * 3 / 4);
        }

        add_bucket(bufmgr_gem, 1024 * 1024);
    }
    if (1 == alloc_mode || 3 == alloc_mode)
    {
        // < 1M 64k alignment
        unit_size = 64 * 1024;
        for (size = unit_size; size <= 1024 * 1024; size += unit_size)
        {
            add_bucket(bufmgr_gem, size);
        }
    }
    if( 0 == alloc_mode || 1 == alloc_mode)
    {
       //> 1M is normal alloc_mode
        add_bucket(bufmgr_gem, 1280 * 1024);
        add_bucket(bufmgr_gem, 1536 * 1024);
        add_bucket(bufmgr_gem, 1792 * 1024);

        for (size = 2 * 1024 * 1024; size < cache_max_size; size *= 2) {
            add_bucket(bufmgr_gem, size);
            add_bucket(bufmgr_gem, size + size * 1 / 4);
            add_bucket(bufmgr_gem, size + size * 2 / 4);
            add_bucket(bufmgr_gem, size + size * 3 / 4);
        }
    }
    if( 2 == alloc_mode || 3 == alloc_mode)
    {
       //> 1M rolling to 2M
       unit_size = 2 * 1024 * 1024;
       add_bucket(bufmgr_gem, unit_size);
       add_bucket(bufmgr_gem, 3 * 1024 * 1024);

       for (size = 4 * 1024 * 1024; size <= cache_max_size; size += unit_size)
       {
           add_bucket(bufmgr_gem, size);
       }
    }
}

/**
 * Get the PCI ID for the device.  This can be overridden by setting the
 * INTEL_DEVID_OVERRIDE environment variable to the desired ID.
 */
static int
get_pci_device_id(struct mos_bufmgr_gem *bufmgr_gem)
{
    char *devid_override;
    int devid = 0;
    int ret;
    drm_i915_getparam_t gp;

    if (geteuid() == getuid()) {
        devid_override = getenv("INTEL_DEVID_OVERRIDE");
        if (devid_override) {
            bufmgr_gem->no_exec = true;
            return strtod(devid_override, nullptr);
        }
    }

    memclear(gp);
    gp.param = I915_PARAM_CHIPSET_ID;
    gp.value = &devid;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (ret) {
        fprintf(stderr, "get chip id failed: %d [%d]\n", ret, errno);
        fprintf(stderr, "param: %d, val: %d\n", gp.param, *gp.value);
    }
    return devid;
}

static int
mos_gem_get_devid(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;

    return bufmgr_gem->pci_device;
}

int mos_gem_ctx_set_user_ctx_params(struct mos_linux_context *context)
{
    /*
     * INTEL_I915_CTX_CONTROL=0, 1, 2, 3
     * 0: default, do nothing
     * 1: disable ctx recoverable
     * 2: disable ctx bannable
     * 3: disable both
     * */

    if (context == nullptr)
    {
        return -EINVAL;
    }

    int ret = 0;
    char *user_ctx_env = getenv("INTEL_I915_CTX_CONTROL");
    bool disable_recoverable = false;
    bool disable_bannable = false;
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)context->bufmgr;

    if (user_ctx_env != nullptr)
    {
        uint8_t  user_ctx_env_value = (uint8_t)atoi(user_ctx_env);
        if(user_ctx_env_value > 3)
        {
            MOS_DBG("INTEL_I915_CTX_CONTROL: invalid value %u setting\n",
                user_ctx_env_value);
        }
        else
        {
            if (user_ctx_env_value & 0x1)
            {
                disable_recoverable = true;
            }

            if(user_ctx_env_value & 0x2)
            {
                disable_bannable = true;
            }
        }
    }

    if(disable_recoverable)
    {
        ret = mos_set_context_param(context,
                    0,
                    I915_CONTEXT_PARAM_RECOVERABLE,
                    0);
        if(ret != 0) {
            MOS_DBG("I915_CONTEXT_PARAM_RECOVERABLE failed: %s\n",
                strerror(errno));
        }
        else
        {
            MOS_DBG("successfull to disable context recoverable\n");
        }
    }

    if(disable_bannable)
    {
        ret = mos_set_context_param(context,
                    0,
                    I915_CONTEXT_PARAM_BANNABLE,
                    0);
        if(ret != 0) {
            MOS_DBG("I915_CONTEXT_PARAM_BANNABLE failed: %s\n",
                strerror(errno));
        }
        else
        {
            MOS_DBG("successfull to disable context bannable\n");
        }
    }

    return ret;
}

static struct mos_linux_context *
mos_gem_context_create(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    struct drm_i915_gem_context_create create;
    struct mos_linux_context *context = nullptr;
    int ret;

    context = (struct mos_linux_context *)calloc(1, sizeof(*context));
    if (!context)
        return nullptr;

    memclear(create);
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_CONTEXT_CREATE, &create);
    if (ret != 0) {
        MOS_DBG("DRM_IOCTL_I915_GEM_CONTEXT_CREATE failed: %s\n",
            strerror(errno));
        free(context);
        return nullptr;
    }

    context->ctx_id = create.ctx_id;
    context->bufmgr = bufmgr;

    ret = mos_gem_ctx_set_user_ctx_params(context);

    return context;
}

static void
mos_gem_context_destroy(struct mos_linux_context *ctx)
{
    struct mos_bufmgr_gem *bufmgr_gem;
    struct drm_i915_gem_context_destroy destroy;
    int ret;

    if (ctx == nullptr)
        return;

    memclear(destroy);

    bufmgr_gem = (struct mos_bufmgr_gem *)ctx->bufmgr;
    destroy.ctx_id = ctx->ctx_id;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_CONTEXT_DESTROY,
               &destroy);
    if (ret != 0)
        fprintf(stderr, "DRM_IOCTL_I915_GEM_CONTEXT_DESTROY failed: %s\n",
            strerror(errno));

    free(ctx);
}

static int
mos_bufmg_get_reset_stats(struct mos_linux_context *ctx,
              uint32_t *reset_count,
              uint32_t *active,
              uint32_t *pending)
{
    struct mos_bufmgr_gem *bufmgr_gem;
    struct drm_i915_reset_stats stats;
    int ret;

    if (ctx == nullptr)
        return -EINVAL;

    memclear(stats);

    bufmgr_gem = (struct mos_bufmgr_gem *)ctx->bufmgr;
    stats.ctx_id = ctx->ctx_id;
    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GET_RESET_STATS,
               &stats);
    if (ret == 0) {
        if (reset_count != nullptr)
            *reset_count = stats.reset_count;

        if (active != nullptr)
            *active = stats.batch_active;

        if (pending != nullptr)
            *pending = stats.batch_pending;
    }

    return ret;
}

static unsigned int mos_bufmgr_hweight8(struct mos_linux_context *ctx, uint8_t w)
{
    uint32_t i, weight = 0;

    for (i=0; i<8; i++)
    {
        weight += !!((w) & (1UL << i));
    }
    return weight;
}

static uint8_t mos_bufmgr_switch_off_n_bits(struct mos_linux_context *ctx, uint8_t in_mask, int n)
{
    int i,count;
    uint8_t bi,out_mask;

    assert (n>0 && n<=8);

    out_mask = in_mask;
    count = n;
    for(i=0; i<8; i++)
    {
        bi = 1UL<<i;
        if (bi & in_mask)
        {
            out_mask &= ~bi;
            count--;
        }
        if (count==0)
        {
            break;
        }
    }
    return out_mask;
}

static int
mos_bufmgr_get_context_param_sseu(struct mos_linux_context *ctx,
                struct drm_i915_gem_context_param_sseu *sseu)
{
    struct mos_bufmgr_gem *bufmgr_gem;
    struct drm_i915_gem_context_param context_param;
    int ret;

    if (ctx == nullptr)
        return -EINVAL;

    bufmgr_gem = (struct mos_bufmgr_gem *)ctx->bufmgr;
    memset(&context_param, 0, sizeof(context_param));
    context_param.ctx_id = ctx->ctx_id;
    context_param.param = I915_CONTEXT_PARAM_SSEU;
    context_param.value = (uint64_t) sseu;
    context_param.size = sizeof(struct drm_i915_gem_context_param_sseu);

    ret = drmIoctl(bufmgr_gem->fd,
            DRM_IOCTL_I915_GEM_CONTEXT_GETPARAM,
            &context_param);

    return ret;
}

static int
mos_bufmgr_set_context_param_sseu(struct mos_linux_context *ctx,
                struct drm_i915_gem_context_param_sseu sseu)
{
    struct mos_bufmgr_gem *bufmgr_gem;
    struct drm_i915_gem_context_param context_param;
    int ret;

    if (ctx == nullptr)
        return -EINVAL;

    bufmgr_gem = (struct mos_bufmgr_gem *)ctx->bufmgr;
    memset(&context_param, 0, sizeof(context_param));
    context_param.ctx_id = ctx->ctx_id;
    context_param.param = I915_CONTEXT_PARAM_SSEU;
    context_param.value = (uint64_t) &sseu;
    context_param.size = sizeof(struct drm_i915_gem_context_param_sseu);

    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_CONTEXT_SETPARAM,
               &context_param);

    return ret;
}

static int
mos_gem_get_context_param(struct mos_linux_context *ctx,
                uint32_t size,
                uint64_t param,
                uint64_t *value)
{
    struct mos_bufmgr_gem *bufmgr_gem;
    struct drm_i915_gem_context_param context_param;
    int ret;

    if (ctx == nullptr)
        return -EINVAL;

    bufmgr_gem = (struct mos_bufmgr_gem *)ctx->bufmgr;
    context_param.ctx_id = ctx->ctx_id;
    context_param.size = size;
    context_param.param = param;
    context_param.value = 0;

    ret = drmIoctl(bufmgr_gem->fd,
            DRM_IOCTL_I915_GEM_CONTEXT_GETPARAM,
            &context_param);
    *value = context_param.value;

    return ret;
}

static int
mos_gem_set_context_param(struct mos_linux_context *ctx,
                uint32_t size,
                uint64_t param,
                uint64_t value)
{
    struct mos_bufmgr_gem *bufmgr_gem;
    struct drm_i915_gem_context_param context_param;
    int ret;

    if (ctx == nullptr)
        return -EINVAL;

    bufmgr_gem = (struct mos_bufmgr_gem *)ctx->bufmgr;
    context_param.ctx_id = ctx->ctx_id;
    context_param.size = size;
    context_param.param = param;
    context_param.value = value;

    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_CONTEXT_SETPARAM,
               &context_param);

    return ret;
}

static int
mos_bufmg_reg_read(struct mos_bufmgr *bufmgr,
           uint32_t offset,
           uint64_t *result)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    struct drm_i915_reg_read reg_read;
    int ret;

    memclear(reg_read);
    reg_read.offset = offset;

    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_REG_READ, &reg_read);

    *result = reg_read.val;
    return ret;
}

static pthread_mutex_t bufmgr_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static drmMMListHead bufmgr_list = { &bufmgr_list, &bufmgr_list };

static struct mos_bufmgr_gem *
mos_bufmgr_gem_find(int fd)
{
    struct mos_bufmgr_gem *bufmgr_gem;

    DRMLISTFOREACHENTRY(bufmgr_gem, &bufmgr_list, managers) {
        if (bufmgr_gem->fd == fd) {
            atomic_inc(&bufmgr_gem->refcount);
            return bufmgr_gem;
        }
    }

    return nullptr;
}

static void
mos_bufmgr_gem_unref(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;

    if (atomic_add_unless(&bufmgr_gem->refcount, -1, 1)) {
        pthread_mutex_lock(&bufmgr_list_mutex);

        if (atomic_dec_and_test(&bufmgr_gem->refcount)) {
            DRMLISTDEL(&bufmgr_gem->managers);
            mos_bufmgr_gem_destroy(bufmgr);
        }

        pthread_mutex_unlock(&bufmgr_list_mutex);
    }
}

static int
mos_gem_get_memory_info(struct mos_bufmgr *bufmgr, char *info, uint32_t length)
{
    return 0;
}

static void mos_gem_enable_softpin(struct mos_bufmgr *bufmgr, bool va1m_align)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    bufmgr_gem->use_softpin           = true;
    bufmgr_gem->softpin_va1Malign     = va1m_align;
}

static void mos_gem_enable_vmbind(struct mos_bufmgr *bufmgr)
{
}

static void mos_gem_disable_object_capture(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    if (bufmgr_gem != nullptr)
    {
        bufmgr_gem->object_capture_disabled = true;
    }
}

static struct mos_linux_context *
mos_gem_context_create_ext(struct mos_bufmgr *bufmgr, __u32 flags, bool bContextProtected)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    struct drm_i915_gem_context_create_ext create;
    struct mos_linux_context *context = nullptr;
    struct drm_i915_gem_context_create_ext_setparam p_protected;
    struct drm_i915_gem_context_create_ext_setparam p_norecover;
    int ret;

    context = (struct mos_linux_context *)calloc(1, sizeof(*context));
    if (!context)
        return nullptr;

    memclear(create);
    create.flags = flags;
    create.extensions = 0;

    if (bContextProtected)
    {
        memclear(p_protected);
        memclear(p_norecover);
        p_protected.base.next_extension = 0;
        p_protected.base.name           = I915_CONTEXT_CREATE_EXT_SETPARAM;
        p_protected.param.param         = I915_CONTEXT_PARAM_PROTECTED_CONTENT;
        p_protected.param.value         = 1;

        p_norecover.base.next_extension = (uintptr_t)&p_protected;
        p_norecover.base.name           = I915_CONTEXT_CREATE_EXT_SETPARAM;
        p_norecover.param.param         = I915_CONTEXT_PARAM_RECOVERABLE;
        p_norecover.param.value         = 0;

        create.flags = flags|I915_CONTEXT_CREATE_FLAGS_USE_EXTENSIONS;
        create.extensions = (uintptr_t)&p_norecover;
    }
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_CONTEXT_CREATE_EXT, &create);
    if (ret != 0) {
        MOS_DBG("DRM_IOCTL_I915_GEM_CONTEXT_CREATE failed: %s\n",
            strerror(errno));
        free(context);
        return nullptr;
    }

    context->ctx_id = create.ctx_id;
    context->bufmgr = bufmgr;

    ret = mos_gem_ctx_set_user_ctx_params(context);

    return context;
}

static __u32 mos_gem_vm_create(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    struct drm_i915_gem_vm_control *vm = nullptr;
    __u32 vm_id;
    int ret;

    vm = (struct drm_i915_gem_vm_control *)calloc(1, sizeof(struct drm_i915_gem_vm_control));
    if (nullptr == vm)
    {
        MOS_DBG("vm calloc failed\n" );
        return INVALID_VM;
    }

    memset(vm, 0, sizeof(*vm));

    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_VM_CREATE, vm);
    if (ret != 0) {
        MOS_DBG("DRM_IOCTL_I915_GEM_VM_CREATE failed: %s\n",
            strerror(errno));
        free(vm);
        return INVALID_VM;
    }

    vm_id = vm->vm_id;
    free(vm);

    return vm_id;
}

static void mos_gem_vm_destroy(struct mos_bufmgr *bufmgr, __u32 vm_id)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    struct drm_i915_gem_vm_control *vm = nullptr;
    int ret;

    if (vm_id == INVALID_VM)
    {
        MOS_DBG("input invalid param\n" );
        return;
    }

    vm = (struct drm_i915_gem_vm_control *)calloc(1, sizeof(struct drm_i915_gem_vm_control));

    if (nullptr == vm)
    {
        MOS_DBG("vm calloc failed\n" );
        return;
    }

    vm->vm_id = vm_id;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_VM_DESTROY, vm);
    if (ret != 0) {
        MOS_DBG("DRM_IOCTL_I915_GEM_VM_DESTROY failed: %s\n",
            strerror(errno));
    }

    free(vm);
}

static struct mos_linux_context *
mos_gem_context_create_shared(struct mos_bufmgr *bufmgr,
            mos_linux_context* ctx,
            __u32 flags,
            bool bContextProtected,
            void *engine_map,
            uint8_t ctx_width,
            uint8_t num_placements,
            uint32_t ctx_type)
{
    MOS_UNUSED(engine_map);
    MOS_UNUSED(ctx_width);
    MOS_UNUSED(num_placements);
    MOS_UNUSED(ctx_type);

    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    struct drm_i915_gem_context_create_ext create;
    struct mos_linux_context *context = nullptr;
    struct drm_i915_gem_context_create_ext_setparam p_protected;
    struct drm_i915_gem_context_create_ext_setparam p_norecover;
    int ret;

    if (ctx == nullptr || ctx->vm_id == INVALID_VM)
        return nullptr;

    context = (struct mos_linux_context *)calloc(1, sizeof(*context));
    if (!context)
        return nullptr;

    memclear(create);
    create.flags = flags;
    create.extensions = 0;
    if (bContextProtected)
    {
        memclear(p_protected);
        memclear(p_norecover);
        p_protected.base.next_extension = 0;
        p_protected.base.name           = I915_CONTEXT_CREATE_EXT_SETPARAM;
        p_protected.param.param         = I915_CONTEXT_PARAM_PROTECTED_CONTENT;
        p_protected.param.value         = 1;

        p_norecover.base.next_extension = (uintptr_t)&p_protected;
        p_norecover.base.name           = I915_CONTEXT_CREATE_EXT_SETPARAM;
        p_norecover.param.param         = I915_CONTEXT_PARAM_RECOVERABLE;
        p_norecover.param.value         = 0;

        create.flags = flags|I915_CONTEXT_CREATE_FLAGS_USE_EXTENSIONS;
        create.extensions = (uintptr_t)&p_norecover;
    }
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_CONTEXT_CREATE_EXT, &create);
    if (ret != 0) {
        MOS_DBG("DRM_IOCTL_I915_GEM_CONTEXT_CREATE failed: %s\n",
            strerror(errno));
        free(context);
        return nullptr;
    }

    context->ctx_id = create.ctx_id;
    context->bufmgr = bufmgr;

    ret = mos_set_context_param(context,
                0,
                I915_CONTEXT_PARAM_VM,
                ctx->vm_id);
    if(ret != 0) {
        MOS_DBG("I915_CONTEXT_PARAM_VM failed: %s\n",
            strerror(errno));
        free(context);
        return nullptr;
    }

    ret = mos_gem_ctx_set_user_ctx_params(context);

    return context;
}

static int mos_bufmgr_query_engines_count(struct mos_bufmgr *bufmgr,
                      unsigned int *nengine)
{
    assert(bufmgr);
    assert(nengine);
    int fd = ((struct mos_bufmgr_gem*)bufmgr)->fd;
    struct drm_i915_query query;
    struct drm_i915_query_item query_item;
    struct drm_i915_query_engine_info *engines = nullptr;
    int ret, len;

    memclear(query_item);
    query_item.query_id = DRM_I915_QUERY_ENGINE_INFO;
    query_item.length = 0;
    memclear(query);
    query.num_items = 1;
    query.items_ptr = (uintptr_t)&query_item;

    ret = drmIoctl(fd, DRM_IOCTL_I915_QUERY, &query);
    if (ret || query_item.length == 0)
    {
        *nengine = 0;
        return ret;
    }

    len = query_item.length;

    engines = (drm_i915_query_engine_info *)malloc(len);
    if (nullptr == engines)
    {
        *nengine = 0;
        ret = -ENOMEM;
        return ret;
    }

    memset(engines, 0, len);
    memclear(query_item);
    query_item.query_id = DRM_I915_QUERY_ENGINE_INFO;
    query_item.length = len;
    query_item.data_ptr = (uintptr_t)engines;
    memclear(query);
    query.num_items = 1;
    query.items_ptr = (uintptr_t)&query_item;

    ret = drmIoctl(fd, DRM_IOCTL_I915_QUERY, &query);

    *nengine = ret ? 0 : engines->num_engines;
    
    if(engines)
    {
        free(engines);
    }
    return ret;
}

static int mos_bufmgr_query_engines(struct mos_bufmgr *bufmgr,
                      __u16 engine_class,
                      __u64 caps,
                      unsigned int *nengine,
                      void *engine_map)
{
    struct i915_engine_class_instance *ci = (struct i915_engine_class_instance *)engine_map;
    if((bufmgr == nullptr) || (nengine == nullptr) || (ci == nullptr))
    {
        return -EINVAL;
    }

    struct drm_i915_query query;
    struct drm_i915_query_item query_item;
    struct drm_i915_query_engine_info *engines = nullptr;
    int ret, len;
    int fd = ((struct mos_bufmgr_gem*)bufmgr)->fd;

    memclear(query_item);
    query_item.query_id = DRM_I915_QUERY_ENGINE_INFO;
    query_item.length = 0;
    memclear(query);
    query.num_items = 1;
    query.items_ptr = (uintptr_t)&query_item;

    ret = drmIoctl(fd, DRM_IOCTL_I915_QUERY, &query);
    if (ret)
    {
        goto fini;
    }

    len = query_item.length;
    if(len == 0)
    {
        goto fini;
    }

    engines = (drm_i915_query_engine_info *)malloc(len);
    if (nullptr == engines)
    {
        ret = -ENOMEM;
        goto fini;
    }
    memset(engines,0,len);
    memclear(query_item);
    query_item.query_id = DRM_I915_QUERY_ENGINE_INFO;
    query_item.length = len;
    query_item.data_ptr = (uintptr_t)engines;
    memclear(query);
    query.num_items = 1;
    query.items_ptr = (uintptr_t)&query_item;

    ret = drmIoctl(fd, DRM_IOCTL_I915_QUERY, &query);
    if (ret)
    {
        goto fini;
    }

    int i, num;
    for (i = 0, num = 0; i < engines->num_engines; i++) {
        struct drm_i915_engine_info *engine =
            (struct drm_i915_engine_info *)&engines->engines[i];
        if ( engine_class == engine->engine.engine_class
             && ((caps & engine->capabilities) == caps ))
        {
            ci->engine_class = engine_class;
            ci->engine_instance = engine->engine.engine_instance;
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

static size_t mos_bufmgr_get_engine_class_size()
{
    return sizeof(struct i915_engine_class_instance);
}

static int
mos_bufmgr_query_sys_engines(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    int ret;

    if (nullptr == gfx_info)
    {
        return -EINVAL;
    }

    unsigned int maxNengine = 0;
    if((gfx_info->VDBoxInfo.NumberOfVDBoxEnabled == 0)
        || (gfx_info->VEBoxInfo.NumberOfVEBoxEnabled == 0))
    {
        if (mos_query_engines_count(bufmgr, &maxNengine) || (maxNengine == 0))
        {
            MOS_OS_ASSERTMESSAGE("Failed to query engines count.\n");
            return -ENODEV;
        }
    }

    if (gfx_info->VDBoxInfo.NumberOfVDBoxEnabled == 0)
    {
        unsigned int nengine = maxNengine;
        struct i915_engine_class_instance *uengines = nullptr;
        uengines = (struct i915_engine_class_instance *)MOS_AllocAndZeroMemory(nengine * sizeof(struct i915_engine_class_instance));
        if (nullptr == uengines)
        {
            return -ENOMEM;
        }
        ret = mos_bufmgr_query_engines(bufmgr, I915_ENGINE_CLASS_VIDEO, 0, &nengine, (void *)uengines);
        if (ret)
        {
            MOS_OS_ASSERTMESSAGE("Failed to query vdbox engine\n");
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

    if (gfx_info->VEBoxInfo.NumberOfVEBoxEnabled == 0)
    {
        unsigned int nengine = maxNengine;
        struct i915_engine_class_instance *uengines = nullptr;
        uengines = (struct i915_engine_class_instance *)MOS_AllocAndZeroMemory(nengine * sizeof(struct i915_engine_class_instance));
        if (nullptr == uengines)
        {
            return -ENOMEM;
        }
        ret = mos_bufmgr_query_engines(bufmgr, I915_ENGINE_CLASS_VIDEO_ENHANCE, 0, &nengine, (void *)uengines);
        if (ret)
        {
            MOS_OS_ASSERTMESSAGE("Failed to query vebox engine\n");
            MOS_SafeFreeMemory(uengines);
            return -ENODEV;
        }
        else
        {
            MOS_OS_ASSERT(nengine <= maxNengine);
            gfx_info->VEBoxInfo.NumberOfVEBoxEnabled = nengine;
        }

        MOS_SafeFreeMemory(uengines);
    }

    return 0;
}

void mos_gem_select_fixed_engine(struct mos_bufmgr *bufmgr,
            void *engine_map,
            uint32_t *nengine,
            uint32_t fixed_instance_mask)
{
    MOS_UNUSED(bufmgr);
#if (DEBUG || _RELEASE_INTERNAL)
    if (fixed_instance_mask)
    {
        struct i915_engine_class_instance *_engine_map = (struct i915_engine_class_instance *)engine_map;
        auto unselect_index = 0;
        for(auto bit = 0; bit < *nengine; bit++)
        {
            if(((fixed_instance_mask >> bit) & 0x1) && (bit > unselect_index))
            {
                _engine_map[unselect_index].engine_class = _engine_map[bit].engine_class;
                _engine_map[unselect_index].engine_instance = _engine_map[bit].engine_instance;
                _engine_map[bit].engine_class = 0;
                _engine_map[bit].engine_instance = 0;
                unselect_index++;
            }
            else if(((fixed_instance_mask >> bit) & 0x1) && (bit == unselect_index))
            {
                unselect_index++;
            }
            else if(!((fixed_instance_mask >> bit) & 0x1))
            {
                _engine_map[bit].engine_class = 0;
                _engine_map[bit].engine_instance = 0;
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

static int mos_gem_set_context_param_parallel(struct mos_linux_context *ctx,
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
    if(parallel_submit == nullptr)
    {
        ret = -ENOMEM;
        goto fini;
    }
    memset(parallel_submit, 0, size);
    parallel_submit->base.name = I915_CONTEXT_ENGINES_EXT_PARALLEL_SUBMIT;
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
    if(set_engines == nullptr)
    {
        ret = -ENOMEM;
        goto fini;
    }
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

static int mos_gem_set_context_param_load_balance(struct mos_linux_context *ctx,
                     struct i915_engine_class_instance *ci,
                     unsigned int count)
{
    int ret;
    uint32_t size;
    struct i915_context_engines_load_balance* balancer = nullptr;
    struct i915_context_param_engines* set_engines = nullptr;

    MOS_OS_CHECK_CONDITION(ci == nullptr, "Invalid (nullptr) Pointer.", EINVAL);
    MOS_OS_CHECK_CONDITION(count == 0, "Invalid input parameter. Number of engines must be > 0.", EINVAL);

    /* I915_DEFINE_CONTEXT_ENGINES_LOAD_BALANCE */
    size = sizeof(struct i915_context_engines_load_balance) + count * sizeof(*ci);
    balancer = (struct i915_context_engines_load_balance*)malloc(size);
    if (NULL == balancer)
    {
        ret = -ENOMEM;
        goto fini;
    }
    memset(balancer, 0, size);
    balancer->base.name = I915_CONTEXT_ENGINES_EXT_LOAD_BALANCE;
    balancer->num_siblings = count;
    memcpy(balancer->engines, ci, count * sizeof(*ci));

    /* I915_DEFINE_CONTEXT_PARAM_ENGINES */
    size = sizeof(uint64_t) + sizeof(*ci);
    set_engines = (struct i915_context_param_engines*) malloc(size);
    if (NULL == set_engines)
    {
        ret = -ENOMEM;
        goto fini;
    }
    set_engines->extensions = (uintptr_t)(balancer);
    set_engines->engines[0].engine_class = I915_ENGINE_CLASS_INVALID;
    set_engines->engines[0].engine_instance = I915_ENGINE_CLASS_INVALID_NONE;

    ret = mos_set_context_param(ctx,
                          size,
                          I915_CONTEXT_PARAM_ENGINES,
                          (uintptr_t)set_engines);
fini:
    if (set_engines)
        free(set_engines);
    if (balancer)
        free(balancer);
    return ret;
}

static int mos_gem_set_context_param_bond(struct mos_linux_context *ctx,
                        struct i915_engine_class_instance master_ci,
                        struct i915_engine_class_instance *bond_ci,
                        unsigned int bond_count)
{
    int ret;
    uint32_t size;
    struct i915_context_engines_load_balance* balancer = nullptr;
    struct i915_context_engines_bond *bond = nullptr;
    struct i915_context_param_engines* set_engines = nullptr;

    assert(bond_ci);

    /* I915_DEFINE_CONTEXT_ENGINES_LOAD_BALANCE */
    size = sizeof(struct i915_context_engines_load_balance) + bond_count * sizeof(bond_ci);
    balancer = (struct i915_context_engines_load_balance*)malloc(size);
    if (NULL == balancer)
    {
        ret = -ENOMEM;
        goto fini;
    }
    memset(balancer, 0, size);
    balancer->base.name = I915_CONTEXT_ENGINES_EXT_LOAD_BALANCE;
    balancer->num_siblings = bond_count;
    memcpy(balancer->engines, bond_ci, bond_count * sizeof(*bond_ci));

    /* I915_DEFINE_CONTEXT_ENGINES_BOND */
    size = sizeof(struct i915_context_engines_bond) + bond_count * sizeof(*bond_ci);
    bond = (struct i915_context_engines_bond*)malloc(size);
    if (NULL == bond)
    {
        ret = -ENOMEM;
        goto fini;
    }
    memset(bond, 0, size);
    bond->base.name = I915_CONTEXT_ENGINES_EXT_BOND;
    bond->master = master_ci;
    bond->num_bonds = bond_count;
    memcpy(bond->engines, bond_ci, bond_count * sizeof(*bond_ci));

    /* I915_DEFINE_CONTEXT_PARAM_ENGINES */
    size = sizeof(uint64_t) + sizeof(struct i915_engine_class_instance);
    set_engines = (struct i915_context_param_engines*) malloc(size);
    if (NULL == set_engines)
    {
        ret = -ENOMEM;
        goto fini;
    }
    set_engines->extensions = (uintptr_t)(balancer);
    balancer->base.next_extension = (uintptr_t)(bond);
    set_engines->engines[0].engine_class = I915_ENGINE_CLASS_INVALID;
    set_engines->engines[0].engine_instance = I915_ENGINE_CLASS_INVALID_NONE;

    ret = mos_set_context_param(ctx,
                          size,
                          I915_CONTEXT_PARAM_ENGINES,
                          (uintptr_t)set_engines);
fini:
    if (set_engines)
        free(set_engines);
    if (bond)
        free(bond);
    if (balancer)
        free(balancer);
    return ret;
}

static bool mos_gem_bo_is_softpin(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    if (bo_gem == nullptr)
    {
        return false;
    }

    return bo_gem->is_softpin;
}

static bool
mos_gem_bo_is_exec_object_async(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *)bo;

    return bo_gem->exec_async;
}


static int mos_bufmgr_query_device_blob(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info)
{
    if ((bufmgr == nullptr) || (gfx_info == nullptr))
    {
        return -EINVAL;
    }

    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem*)bufmgr;
    if (bufmgr_gem == nullptr)
    {
        return -EINVAL;
    }
    int fd = bufmgr_gem->fd;
    uint32_t *hw_info = nullptr;
    uint32_t  ulength= 0;
    int ret, i;
    struct drm_i915_query_item query_item;

    memclear(query_item);
    query_item.length = 0;
    query_item.query_id = DRM_I915_QUERY_HWCONFIG_BLOB;
    ret = mos_gem_query_items(fd, &query_item, 1);
    if (ret != 0 || query_item.length <= 0)
    {
        mos_safe_free(hw_info);
        return (ret != 0) ? ret : -1;
    }

    hw_info = (uint32_t*) malloc(query_item.length);
    if (hw_info != nullptr)
    {
        memset(hw_info, 0, query_item.length);
    }
    else
    {
        mos_safe_free(hw_info);
        return -ENOMEM;
    }
    query_item.data_ptr = (uintptr_t) hw_info;
    ret = mos_gem_query_items(fd, &query_item, 1);
    if (ret != 0 || query_item.length <= 0)
    {
        mos_safe_free(hw_info);
        return (ret != 0) ? ret : -1;
    }
    ulength = query_item.length / sizeof(uint32_t);
    i = 0;
    while (i < ulength) {
        /* Attribute ID starts with 1 */
        if (hw_info[i] <= 0)
        {
            mos_safe_free(hw_info);
            return -EINVAL;
        }
        
        #if DEBUG_BLOB_QUERY
        fprintf("%s: %d\n", key_string[hw_info[i]], hw_info[i+2]);
        #endif
        if (INTEL_HWCONFIG_MAX_SLICES_SUPPORTED == hw_info[i])
        {
            if (hw_info[i+1] != 1)
            {
                mos_safe_free(hw_info);
                return -EINVAL;
            }
            gfx_info->SliceCount = hw_info[i+2];
            gfx_info->MaxSlicesSupported = hw_info[i+2];
        }

        if (INTEL_HWCONFIG_MAX_DUAL_SUBSLICES_SUPPORTED == hw_info[i])
        {
            if (hw_info[i+1] != 1)
            {
                mos_safe_free(hw_info);
                return -EINVAL;
            }
            gfx_info->SubSliceCount = hw_info[i+2];
            gfx_info->MaxSubSlicesSupported = hw_info[i+2];
        }

        if (INTEL_HWCONFIG_MAX_NUM_EU_PER_DSS == hw_info[i])
        {
            if (hw_info[i+1] != 1)
            {
                mos_safe_free(hw_info);
                return -EINVAL;
            }
            gfx_info->MaxEuPerSubSlice = hw_info[i+2];
        }

        if (INTEL_HWCONFIG_DEPRECATED_L3_CACHE_SIZE_IN_KB == hw_info[i])
        {
            if (hw_info[i+1] != 1)
            {
                mos_safe_free(hw_info);
                return -EINVAL;
            }
            gfx_info->L3CacheSizeInKb = hw_info[i+2];
        }

        if (INTEL_HWCONFIG_NUM_THREADS_PER_EU == hw_info[i])
        {
            if (hw_info[i+1] != 1)
            {
                mos_safe_free(hw_info);
                return -EINVAL;
            }
            gfx_info->NumThreadsPerEu = hw_info[i+2];
        }

        if (INTEL_HWCONFIG_MAX_VECS == hw_info[i])
        {
            if (hw_info[i+1] != 1)
            {
                mos_safe_free(hw_info);
                return -EINVAL;
            }
            gfx_info->MaxVECS = hw_info[i+2];
        }

        /* Advance to next key */
        i += hw_info[i + 1];  // value size
        i += 2;// KL size
    }

    if (hw_info != nullptr)
    {
        mos_safe_free(hw_info);
        hw_info = nullptr;
    }

    return ret;
}

static int mos_bufmgr_query_hw_ip_version(struct mos_bufmgr *bufmgr, __u16 engine_class, void *ip_ver_info)
{
    return -1;
}

static uint64_t mos_bufmgr_get_platform_information(struct mos_bufmgr *bufmgr)
{
    assert(bufmgr);
    return bufmgr->platform_information;
}

static void mos_bufmgr_set_platform_information(struct mos_bufmgr *bufmgr, uint64_t p)
{
    assert(bufmgr);
    bufmgr->platform_information |= p;
}

static int
mos_bufmgr_get_ts_frequency(struct mos_bufmgr *bufmgr, uint32_t *ts_freq)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem*)bufmgr;

    *ts_freq = bufmgr_gem->ts_freq;

    return 0;
}

static bool
mos_bufmgr_has_bsd2(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem*)bufmgr;

    return bufmgr_gem->has_bsd2;
}

#define I915_CONTEXT_PRIVATE_PARAM_BOOST 0x80000000
void
mos_bufmgr_enable_turbo_boost(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem*)bufmgr;
    struct drm_i915_gem_context_param ctxParam;
    int32_t retVal = 0;

    MOS_ZeroMemory( &ctxParam, sizeof( ctxParam ) );
    ctxParam.param = I915_CONTEXT_PRIVATE_PARAM_BOOST;
    ctxParam.value = 1;
    retVal = drmIoctl(bufmgr_gem->fd,
                      DRM_IOCTL_I915_GEM_CONTEXT_SETPARAM, &ctxParam );
}

/**
 * Initializes the GEM buffer manager, which uses the kernel to allocate, map,
 * and manage map buffer objections.
 *
 * \param fd File descriptor of the opened DRM device.
 */
static struct mos_bufmgr *
mos_bufmgr_gem_init_i915(int fd, int batch_size)
{
    struct mos_bufmgr_gem *bufmgr_gem;
    struct drm_i915_gem_get_aperture aperture;
    drm_i915_getparam_t gp;
    int ret, tmp;
    uint8_t alloc_mode;
    bool exec2 = false;

    pthread_mutex_lock(&bufmgr_list_mutex);

    bufmgr_gem = mos_bufmgr_gem_find(fd);
    if (bufmgr_gem)
        goto exit;

    bufmgr_gem = (struct mos_bufmgr_gem *)calloc(1, sizeof(*bufmgr_gem));
    if (bufmgr_gem == nullptr)
        goto exit;

    bufmgr_gem->fd = fd;
    atomic_set(&bufmgr_gem->refcount, 1);

    if (pthread_mutex_init(&bufmgr_gem->lock, nullptr) != 0) {
        free(bufmgr_gem);
        bufmgr_gem = nullptr;
        goto exit;
    }

    bufmgr_gem->bufmgr.bo_alloc = mos_gem_bo_alloc;
    bufmgr_gem->bufmgr.bo_alloc_tiled = mos_gem_bo_alloc_tiled;
    bufmgr_gem->bufmgr.bo_reference = mos_gem_bo_reference;
    bufmgr_gem->bufmgr.bo_unreference = mos_gem_bo_unreference;
    bufmgr_gem->bufmgr.bo_map = mos_gem_bo_map;
    bufmgr_gem->bufmgr.bo_unmap = mos_gem_bo_unmap;
    bufmgr_gem->bufmgr.bo_wait_rendering = mos_gem_bo_wait_rendering;
    bufmgr_gem->bufmgr.bo_pad_to_size = mos_gem_bo_pad_to_size;
    bufmgr_gem->bufmgr.bo_emit_reloc = mos_gem_bo_emit_reloc;
    bufmgr_gem->bufmgr.bo_get_tiling = mos_gem_bo_get_tiling;
    bufmgr_gem->bufmgr.bo_set_tiling = mos_gem_bo_set_tiling;
    bufmgr_gem->bufmgr.bo_flink = mos_gem_bo_flink;
    bufmgr_gem->bufmgr.bo_exec = mos_gem_bo_exec2;
    bufmgr_gem->bufmgr.bo_mrb_exec = mos_gem_bo_mrb_exec2;
    bufmgr_gem->bufmgr.bo_busy = mos_gem_bo_busy;
    bufmgr_gem->bufmgr.bo_madvise = mos_gem_bo_madvise;
    bufmgr_gem->bufmgr.destroy = mos_bufmgr_gem_unref;
    bufmgr_gem->bufmgr.debug = 0;
    bufmgr_gem->bufmgr.check_aperture_space =
        mos_gem_check_aperture_space;
    bufmgr_gem->bufmgr.bo_disable_reuse = mos_gem_bo_disable_reuse;
    bufmgr_gem->bufmgr.bo_is_reusable = mos_gem_bo_is_reusable;
    bufmgr_gem->bufmgr.bo_references = mos_gem_bo_references;

    bufmgr_gem->bufmgr.bo_wait = mos_gem_bo_wait;
    bufmgr_gem->bufmgr.bo_clear_relocs = mos_gem_bo_clear_relocs;
    bufmgr_gem->bufmgr.context_create = mos_gem_context_create;
    bufmgr_gem->bufmgr.context_create_ext = mos_gem_context_create_ext;
    bufmgr_gem->bufmgr.context_create_shared = mos_gem_context_create_shared;
    bufmgr_gem->bufmgr.context_destroy = mos_gem_context_destroy;
    bufmgr_gem->bufmgr.vm_create = mos_gem_vm_create;
    bufmgr_gem->bufmgr.vm_destroy = mos_gem_vm_destroy;
    bufmgr_gem->bufmgr.bo_context_exec2 = mos_gem_bo_context_exec2;
    bufmgr_gem->bufmgr.bo_context_exec3 = mos_gem_bo_context_exec3;
    bufmgr_gem->bufmgr.bo_is_exec_object_async = mos_gem_bo_is_exec_object_async;
    bufmgr_gem->bufmgr.bo_is_softpin = mos_gem_bo_is_softpin;
    bufmgr_gem->bufmgr.bo_map_gtt = mos_gem_bo_map_gtt;
    bufmgr_gem->bufmgr.bo_unmap_gtt = mos_gem_bo_unmap_gtt;
    bufmgr_gem->bufmgr.bo_map_wc = mos_gem_bo_map_wc;
    bufmgr_gem->bufmgr.bo_unmap_wc = mos_gem_bo_unmap_wc;
    bufmgr_gem->bufmgr.bo_map_unsynchronized = mos_gem_bo_map_unsynchronized;
    bufmgr_gem->bufmgr.bo_start_gtt_access = mos_gem_bo_start_gtt_access;
    bufmgr_gem->bufmgr.bo_get_softpin_targets_info = mos_bufmgr_bo_get_softpin_targets_info;
    bufmgr_gem->bufmgr.bo_create_from_name = mos_bufmgr_bo_gem_create_from_name;
    bufmgr_gem->bufmgr.enable_reuse = mos_gem_enable_reuse;
    bufmgr_gem->bufmgr.enable_softpin = mos_gem_enable_softpin;
    bufmgr_gem->bufmgr.enable_vmbind = mos_gem_enable_vmbind;
    bufmgr_gem->bufmgr.disable_object_capture = mos_gem_disable_object_capture;
    bufmgr_gem->bufmgr.get_memory_info = mos_gem_get_memory_info;
    bufmgr_gem->bufmgr.get_devid = mos_gem_get_devid;
    bufmgr_gem->bufmgr.realloc_cache = mos_gem_realloc_cache;
    bufmgr_gem->bufmgr.set_context_param = mos_gem_set_context_param;
    bufmgr_gem->bufmgr.set_context_param_parallel = mos_gem_set_context_param_parallel;
    bufmgr_gem->bufmgr.set_context_param_load_balance = mos_gem_set_context_param_load_balance;
    bufmgr_gem->bufmgr.set_context_param_bond = mos_gem_set_context_param_bond;
    bufmgr_gem->bufmgr.get_context_param = mos_gem_get_context_param;
    bufmgr_gem->bufmgr.bo_create_from_prime = mos_gem_bo_create_from_prime;
    bufmgr_gem->bufmgr.bo_export_to_prime = mos_gem_bo_export_to_prime;
    bufmgr_gem->bufmgr.reg_read = mos_bufmg_reg_read;
    bufmgr_gem->bufmgr.get_reset_stats = mos_bufmg_get_reset_stats;
    bufmgr_gem->bufmgr.get_context_param_sseu = mos_bufmgr_get_context_param_sseu;
    bufmgr_gem->bufmgr.set_context_param_sseu = mos_bufmgr_set_context_param_sseu;
    bufmgr_gem->bufmgr.query_sys_engines = mos_bufmgr_query_sys_engines;
    bufmgr_gem->bufmgr.query_device_blob = mos_bufmgr_query_device_blob;
    bufmgr_gem->bufmgr.get_driver_info = mos_bufmgr_get_driver_info;
    bufmgr_gem->bufmgr.query_hw_ip_version = mos_bufmgr_query_hw_ip_version;
    bufmgr_gem->bufmgr.get_platform_information = mos_bufmgr_get_platform_information;
    bufmgr_gem->bufmgr.set_platform_information = mos_bufmgr_set_platform_information;
    bufmgr_gem->bufmgr.query_engines_count = mos_bufmgr_query_engines_count;
    bufmgr_gem->bufmgr.query_engines = mos_bufmgr_query_engines;
    bufmgr_gem->bufmgr.get_engine_class_size = mos_bufmgr_get_engine_class_size;
    bufmgr_gem->bufmgr.select_fixed_engine = mos_gem_select_fixed_engine;
    bufmgr_gem->bufmgr.switch_off_n_bits = mos_bufmgr_switch_off_n_bits;
    bufmgr_gem->bufmgr.hweight8 = mos_bufmgr_hweight8;
    bufmgr_gem->bufmgr.get_ts_frequency = mos_bufmgr_get_ts_frequency;
    bufmgr_gem->bufmgr.has_bsd2 = mos_bufmgr_has_bsd2;
    bufmgr_gem->bufmgr.enable_turbo_boost = mos_bufmgr_enable_turbo_boost;

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

        if (ret == -1)
        {
            MOS_DBG("Failed to open %s: %s\n", bufmgr_gem->mem_profiler_path, strerror(errno));
        }
    }
    else
    {
        bufmgr_gem->mem_profiler_fd = -1;
    }

    memclear(aperture);
    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_GET_APERTURE,
               &aperture);

    if (ret == 0)
        bufmgr_gem->gtt_size = aperture.aper_available_size;
    else {
        fprintf(stderr, "DRM_IOCTL_I915_GEM_APERTURE failed: %s\n",
            strerror(errno));
        bufmgr_gem->gtt_size = 128 * 1024 * 1024;
        fprintf(stderr, "Assuming %dkB available aperture size.\n"
            "May lead to reduced performance or incorrect "
            "rendering.\n",
            (int)bufmgr_gem->gtt_size / 1024);
    }

    /* support Gen 8+ */
    bufmgr_gem->pci_device = get_pci_device_id(bufmgr_gem);

    if (bufmgr_gem->pci_device == 0) {
        pthread_mutex_destroy(&bufmgr_gem->lock);
        if (bufmgr_gem->mem_profiler_fd != -1)
        {
            close(bufmgr_gem->mem_profiler_fd);
        }
        free(bufmgr_gem);
        bufmgr_gem = nullptr;
        goto exit;
    }

    memclear(gp);
    gp.value = &tmp;

    gp.param = I915_PARAM_HAS_EXECBUF2;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (!ret)
        exec2 = true;

    gp.param = I915_PARAM_HAS_BSD;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->has_bsd = ret == 0;

    gp.param = I915_PARAM_HAS_BSD2;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->has_bsd2 = (ret == 0 && *gp.value != 0);

    gp.param = I915_PARAM_HAS_BLT;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->has_blt = ret == 0;

    gp.param = I915_PARAM_HAS_RELAXED_FENCING;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->has_relaxed_fencing = ret == 0;

    bufmgr_gem->bufmgr.bo_alloc_userptr = check_bo_alloc_userptr;

    gp.param = I915_PARAM_HAS_WAIT_TIMEOUT;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->has_wait_timeout = ret == 0;

    gp.param = I915_PARAM_HAS_LLC;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->has_llc = *gp.value;

    gp.param = I915_PARAM_HAS_VEBOX;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->has_vebox = (ret == 0) & (*gp.value > 0);

    gp.param = I915_PARAM_MMAP_VERSION;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->has_ext_mmap = (ret == 0) & (*gp.value > 0);

    gp.param = I915_PARAM_NUM_FENCES_AVAIL;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->has_fence_reg = (ret == 0) & (*gp.value > 0);

    gp.param = I915_PARAM_HAS_EXEC_SOFTPIN;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (ret == 0 && *gp.value > 0) {
        bufmgr_gem->bufmgr.bo_set_softpin        = mos_gem_bo_set_softpin;
        bufmgr_gem->bufmgr.bo_add_softpin_target = mos_gem_bo_add_softpin_target;
    }

    gp.param = I915_PARAM_HAS_EXEC_ASYNC;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (ret == 0 && *gp.value > 0) {
        bufmgr_gem->bufmgr.set_object_async      = mos_gem_bo_set_object_async;
        bufmgr_gem->bufmgr.set_exec_object_async = mos_gem_bo_set_exec_object_async;
    }

    gp.param = I915_PARAM_HAS_EXEC_CAPTURE;
    ret      = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (ret == 0 && *gp.value > 0)
    {
        bufmgr_gem->bufmgr.set_object_capture      = mos_gem_bo_set_object_capture;
    }
    else
    {
        bufmgr_gem->object_capture_disabled = true;
    }

    gp.param = I915_PARAM_MMAP_GTT_VERSION;
    ret =  drmIoctl(fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->has_mmap_offset  =  (ret == 0) && (*gp.value >= 4);

    gp.param = I915_PARAM_CS_TIMESTAMP_FREQUENCY;
    ret =  drmIoctl(fd, DRM_IOCTL_I915_GETPARAM, &gp);
    bufmgr_gem->ts_freq   =  (ret == 0) ? *gp.value : 0;

    struct drm_i915_gem_context_param context_param;
    memset(&context_param, 0, sizeof(context_param));
    context_param.param = I915_CONTEXT_PARAM_GTT_SIZE;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_CONTEXT_GETPARAM, &context_param);
    if (ret == 0){
        uint64_t gtt_size_4g = (uint64_t)1 << 32;
        if (context_param.value > gtt_size_4g)
        {
            bufmgr_gem->bufmgr.bo_use_48b_address_range = mos_gem_bo_use_48b_address_range;
        }
    }

    bufmgr_gem->device_type = DEVICE_TYPE_I915;
    bufmgr_gem->has_lmem = mos_gem_has_lmem(bufmgr_gem->fd);

    bufmgr_gem->bufmgr.has_full_vd = true;

    /* Let's go with one relocation per every 2 dwords (but round down a bit
     * since a power of two will mean an extra page allocation for the reloc
     * buffer).
     *
     * Every 4 was too few for the blender benchmark.
     */
    alloc_mode = (uint8_t)(batch_size & 0xff);
    batch_size &= 0xffffff00;
    bufmgr_gem->max_relocs = batch_size / sizeof(uint32_t) / 2 - 2;

    DRMINITLISTHEAD(&bufmgr_gem->named);
    init_cache_buckets(bufmgr_gem);

    DRMLISTADD(&bufmgr_gem->managers, &bufmgr_list);

    bufmgr_gem->use_softpin = false;
    mos_vma_heap_init(&bufmgr_gem->vma_heap[MEMZONE_SYS], MEMZONE_SYS_START, MEMZONE_SYS_SIZE);
    mos_vma_heap_init(&bufmgr_gem->vma_heap[MEMZONE_DEVICE], MEMZONE_DEVICE_START, MEMZONE_DEVICE_SIZE);

exit:
    pthread_mutex_unlock(&bufmgr_list_mutex);

    return bufmgr_gem != nullptr ? &bufmgr_gem->bufmgr : nullptr;
}

struct mos_bufmgr *
mos_bufmgr_gem_init(int fd, int batch_size, int *device_type)
{
    int type = mos_query_device_type(fd);
    if (device_type != nullptr)
    {
        *device_type = type;
    }
    if(DEVICE_TYPE_I915 == type)
    {
        return mos_bufmgr_gem_init_i915(fd, batch_size);
    }
#ifdef ENABLE_XE_KMD
    else if (DEVICE_TYPE_XE == type)
    {
        return mos_bufmgr_gem_init_xe(fd, batch_size);
    }
#endif

    return nullptr;
}

int mos_get_param(int fd, int32_t param, uint32_t *param_value)
{
    if((fd < 0) || (param_value == nullptr))
    {
        return -EINVAL;
    }

    struct drm_i915_getparam gp;
    gp.param = param;
    gp.value = (int32_t *)param_value;

    return drmIoctl(fd, DRM_IOCTL_I915_GETPARAM, &gp) == 0;
}

static int mos_bufmgr_get_driver_info(struct mos_bufmgr *bufmgr, struct LinuxDriverInfo *drvInfo)
{
    if (bufmgr == nullptr || drvInfo == nullptr)
    {
        return -EINVAL;
    }
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem*)bufmgr;
    int fd = bufmgr_gem->fd;
    if (fd < 0)
    {
        return -EINVAL;
    }
    uint32_t retValue = 0;

    drvInfo->hasBsd = 0;
    if (mos_get_param(fd, I915_PARAM_HAS_BSD, &retValue))
    {
        drvInfo->hasBsd = !!retValue;
    }

    drvInfo->hasBsd2 = 0;
    retValue = 0;
    if (mos_get_param(fd, I915_PARAM_HAS_BSD2, &retValue))
    {
        drvInfo->hasBsd2 = !!retValue;
    }

    drvInfo->hasVebox = 0;
    retValue = 0;
    if (mos_get_param(fd, I915_PARAM_HAS_VEBOX, &retValue))
    {
        drvInfo->hasVebox = !!retValue;
    }

    drvInfo->hasPpgtt = 1;
    retValue = 0;
    if (mos_get_param(fd, I915_PARAM_HAS_ALIASING_PPGTT, &retValue))
    {
        drvInfo->hasPpgtt = !!retValue;
    }

    drvInfo->hasHuc = 0;
    retValue = 0;
    if (mos_get_param(fd, I915_PARAM_HUC_STATUS, &retValue))
    {
        drvInfo->hasHuc = !!retValue;
        if (retValue == 1)
        {
            drvInfo->hasProtectedHuc = 1;
        }
    }

    drvInfo->devId = 0;
    retValue = 0;
    if (mos_get_param(fd, I915_PARAM_CHIPSET_ID, &retValue))
    {
        drvInfo->devId = retValue;
    }
    drvInfo->devRev = 0;
    retValue = 0;
    if (mos_get_param(fd, I915_PARAM_REVISION, &retValue))
    {
        drvInfo->devRev = retValue;
    }

    drvInfo->euCount = 0;
    retValue = 0;
    if (mos_get_param(fd, I915_PARAM_EU_TOTAL, &retValue))
    {
        drvInfo->euCount = retValue;
    }

    drvInfo->subSliceCount = 0;
    retValue = 0;
    if (mos_get_param(fd, I915_PARAM_SUBSLICE_TOTAL, &retValue))
    {
        drvInfo->subSliceCount = retValue;
    }

    // There is no interface to read total slice count from drm/i915, so we
    // will set the slice count in InitMediaSysInfo accordint to Device ID.
    drvInfo->sliceCount = 0;

    return 0;
}

static int mos_get_dev_id_i915(int fd, uint32_t *device_id)
{
    if (nullptr == device_id)
    {
        return -EINVAL;
    }
    uint32_t retValue = 0;
    if (mos_get_param(fd, I915_PARAM_CHIPSET_ID, &retValue))
    {
        *device_id = retValue;
    }

    return 0;
}

int mos_get_device_id(int fd, uint32_t *deviceId)
{
    int device_type = mos_query_device_type(fd);

    if (DEVICE_TYPE_I915 == device_type)
    {
        return mos_get_dev_id_i915(fd, deviceId);
    }
#ifdef ENABLE_XE_KMD
    else if (DEVICE_TYPE_XE == device_type)
    {
        return mos_get_dev_id_xe(fd, deviceId);
    }
#endif
    return -ENODEV;
}
