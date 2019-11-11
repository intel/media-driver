/**************************************************************************
 *
 * Copyright © 2007 Red Hat Inc.
 * Copyright © 2007-2018 Intel Corporation
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
#include "string.h"

#include "i915_drm.h"

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#define VG(x) x
#else
#define VG(x)
#endif

#define memclear(s) memset(&s, 0, sizeof(s))

#define MOS_DBG(...) do {                    \
    if (bufmgr_gem->bufmgr.debug)            \
        fprintf(stderr, __VA_ARGS__);        \
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
    struct mos_gem_bo_bucket cache_bucket[14 * 4];
    int num_buckets;
    time_t time;

    drmMMListHead managers;

    drmMMListHead named;
    drmMMListHead vma_cache;
    int vma_count, vma_open, vma_max;

    uint64_t gtt_size;
    int available_fences;
    int pci_device;
    unsigned int has_bsd : 1;
    unsigned int has_blt : 1;
    unsigned int has_relaxed_fencing : 1;
    unsigned int has_llc : 1;
    unsigned int has_wait_timeout : 1;
    unsigned int bo_reuse : 1;
    unsigned int no_exec : 1;
    unsigned int has_vebox : 1;
    unsigned int has_ext_mmap : 1;
    bool fenced_relocs;

    struct {
        void *ptr;
        uint32_t handle;
    } userptr_active;

    // manage address for softpin buffer object
    uint64_t head_offset;
} mos_bufmgr_gem;

#define DRM_INTEL_RELOC_FENCE (1<<0)

struct mos_reloc_target {
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
    struct mos_linux_bo **softpin_target;
    /** Number softpinned BOs that are referenced by this buffer */
    int softpin_target_count;
    /** Maximum amount of softpinned BOs that are referenced by this buffer */
    int softpin_target_size;

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
    drmMMListHead vma_list;

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
};

static unsigned int
mos_gem_estimate_batch_space(struct mos_linux_bo ** bo_array, int count);

static unsigned int
mos_gem_compute_batch_space(struct mos_linux_bo ** bo_array, int count);

static int
mos_gem_bo_get_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
                uint32_t * swizzle_mode);

static int
mos_gem_bo_set_tiling_internal(struct mos_linux_bo *bo,
                     uint32_t tiling_mode,
                     uint32_t stride);

static void mos_gem_bo_unreference_locked_timed(struct mos_linux_bo *bo,
                              time_t time);

static void mos_gem_bo_unreference(struct mos_linux_bo *bo);

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
            struct mos_linux_bo *target_bo = bo_gem->softpin_target[j];
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
            new_size = 5;

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

    if (bo_gem->validate_index != -1) {
        bufmgr_gem->exec2_objects[bo_gem->validate_index].flags |= flags;
        return;
    }

    /* Extend the array of validation entries as necessary. */
    if (bufmgr_gem->exec_count == bufmgr_gem->exec_size) {
        int new_size = bufmgr_gem->exec_size * 2;

        if (new_size == 0)
            new_size = 5;
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

drm_export struct mos_linux_bo *
mos_gem_bo_alloc_internal(struct mos_bufmgr *bufmgr,
                const char *name,
                unsigned long size,
                unsigned long flags,
                uint32_t tiling_mode,
                unsigned long stride,
                unsigned int alignment)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;
    struct mos_bo_gem *bo_gem;
    unsigned int page_size = getpagesize();
    int ret;
    struct mos_gem_bo_bucket *bucket;
    bool alloc_from_cache;
    unsigned long bo_size;
    bool for_render = false;

    if (flags & BO_ALLOC_FOR_RENDER)
        for_render = true;

    /* Round the allocated size up to a power of two number of pages. */
    bucket = mos_gem_bo_bucket_for_size(bufmgr_gem, size);

    /* If we don't have caching at this size, don't actually round the
     * allocation up.
     */
    if (bucket == nullptr) {
        bo_size = size;
        if (bo_size < page_size)
            bo_size = page_size;
    } else {
        bo_size = bucket->size;
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
            bo_gem->bo.align = alignment;
        } else {
            assert(alignment == 0);
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

            if (mos_gem_bo_set_tiling_internal(&bo_gem->bo,
                                 tiling_mode,
                                 stride)) {
                mos_gem_bo_free(&bo_gem->bo);
                goto retry;
            }
        }
    }
    pthread_mutex_unlock(&bufmgr_gem->lock);

    if (!alloc_from_cache) {
        struct drm_i915_gem_create create;

        bo_gem = (struct mos_bo_gem *)calloc(1, sizeof(*bo_gem));
        if (!bo_gem)
            return nullptr;

        bo_gem->bo.size = bo_size;

        memclear(create);
        create.size = bo_size;

        ret = drmIoctl(bufmgr_gem->fd,
                   DRM_IOCTL_I915_GEM_CREATE,
                   &create);
        bo_gem->gem_handle = create.handle;
        bo_gem->bo.handle = bo_gem->gem_handle;
        if (ret != 0) {
            free(bo_gem);
            return nullptr;
        }
        bo_gem->bo.bufmgr = bufmgr;
        bo_gem->bo.align = alignment;

        bo_gem->tiling_mode = I915_TILING_NONE;
        bo_gem->swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
        bo_gem->stride = 0;

        /* drm_intel_gem_bo_free calls DRMLISTDEL() for an uninitialized
           list (vma_list), so better set the list head here */
        DRMINITLISTHEAD(&bo_gem->name_list);
        DRMINITLISTHEAD(&bo_gem->vma_list);
        if (mos_gem_bo_set_tiling_internal(&bo_gem->bo,
                             tiling_mode,
                             stride)) {
            mos_gem_bo_free(&bo_gem->bo);
            return nullptr;
        }
    }

    bo_gem->name = name;
    atomic_set(&bo_gem->refcount, 1);
    bo_gem->validate_index = -1;
    bo_gem->reloc_tree_fences = 0;
    bo_gem->used_as_reloc_target = false;
    bo_gem->has_error = false;
    bo_gem->reusable = true;
    bo_gem->use_48b_address_range = bufmgr_gem->bufmgr.bo_use_48b_address_range ? true : false;

    mos_bo_gem_set_in_aperture_size(bufmgr_gem, bo_gem, alignment);

    MOS_DBG("bo_create: buf %d (%s) %ldb\n",
        bo_gem->gem_handle, bo_gem->name, size);

    return &bo_gem->bo;
}

static struct mos_linux_bo *
mos_gem_bo_alloc_for_render(struct mos_bufmgr *bufmgr,
                  const char *name,
                  unsigned long size,
                  unsigned int alignment)
{
    return mos_gem_bo_alloc_internal(bufmgr, name, size,
                           I915_TILING_NONE, 0,
                           BO_ALLOC_FOR_RENDER,
                           alignment);
}

static struct mos_linux_bo *
mos_gem_bo_alloc(struct mos_bufmgr *bufmgr,
               const char *name,
               unsigned long size,
               unsigned int alignment)
{
    return mos_gem_bo_alloc_internal(bufmgr, name, size, 0,
                           I915_TILING_NONE, 0, 0);
}

static struct mos_linux_bo *
mos_gem_bo_alloc_tiled(struct mos_bufmgr *bufmgr, const char *name,
                 int x, int y, int cpp, uint32_t *tiling_mode,
                 unsigned long *pitch, unsigned long flags)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    unsigned long size, stride;
    uint32_t tiling;

    do {
        unsigned long aligned_y, height_alignment;

        tiling = *tiling_mode;

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
        aligned_y = y;
        height_alignment = 2;

        if (tiling == I915_TILING_X
            || (IS_915(bufmgr_gem->pci_device)
                && tiling == I915_TILING_Y))
            height_alignment = 8;
        else if (tiling == I915_TILING_Y)
            height_alignment = 32;
        aligned_y = ALIGN(y, height_alignment);

        stride = x * cpp;
        stride = mos_gem_bo_tile_pitch(bufmgr_gem, stride, tiling_mode);
        size = stride * aligned_y;
        size = mos_gem_bo_tile_size(bufmgr_gem, size, tiling_mode);
    } while (*tiling_mode != tiling);
    *pitch = stride;

    if (tiling == I915_TILING_NONE)
        stride = 0;

    return mos_gem_bo_alloc_internal(bufmgr, name, size, flags,
                           tiling, stride, 0);
}

static struct mos_linux_bo *
mos_gem_bo_alloc_userptr(struct mos_bufmgr *bufmgr,
                const char *name,
                void *addr,
                uint32_t tiling_mode,
                uint32_t stride,
                unsigned long size,
                unsigned long flags)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;
    struct mos_bo_gem *bo_gem;
    int ret;
    struct drm_i915_gem_userptr userptr;

    /* Tiling with userptr surfaces is not supported
     * on all hardware so refuse it for time being.
     */
    if (tiling_mode != I915_TILING_NONE)
        return nullptr;

    bo_gem = (struct mos_bo_gem *)calloc(1, sizeof(*bo_gem));
    if (!bo_gem)
        return nullptr;

    bo_gem->bo.size = size;

    memclear(userptr);
    userptr.user_ptr = (__u64)((unsigned long)addr);
    userptr.user_size = size;
    userptr.flags = 0;

    ret = drmIoctl(bufmgr_gem->fd,
            DRM_IOCTL_I915_GEM_USERPTR,
            &userptr);
    if (ret != 0) {
        MOS_DBG("bo_create_userptr: "
            "ioctl failed with user ptr %p size 0x%lx, "
            "user flags 0x%lx\n", addr, size, flags);
        free(bo_gem);
        return nullptr;
    }

    bo_gem->gem_handle = userptr.handle;
    bo_gem->bo.handle = bo_gem->gem_handle;
    bo_gem->bo.bufmgr    = bufmgr;
    bo_gem->is_userptr   = true;
#ifdef __cplusplus
    bo_gem->bo.virt   = addr;
#else
    bo_gem->bo.virtual   = addr;
#endif
    /* Save the address provided by user */
    bo_gem->user_virtual = addr;
    bo_gem->tiling_mode  = I915_TILING_NONE;
    bo_gem->swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
    bo_gem->stride       = 0;

    DRMINITLISTHEAD(&bo_gem->name_list);
    DRMINITLISTHEAD(&bo_gem->vma_list);

    bo_gem->name = name;
    atomic_set(&bo_gem->refcount, 1);
    bo_gem->validate_index = -1;
    bo_gem->reloc_tree_fences = 0;
    bo_gem->used_as_reloc_target = false;
    bo_gem->has_error = false;
    bo_gem->reusable = false;
    bo_gem->use_48b_address_range = bufmgr_gem->bufmgr.bo_use_48b_address_range ? true : false;

    mos_bo_gem_set_in_aperture_size(bufmgr_gem, bo_gem, 0);

    MOS_DBG("bo_create_userptr: "
        "ptr %p buf %d (%s) size %ldb, stride 0x%x, tile mode %d\n",
        addr, bo_gem->gem_handle, bo_gem->name,
        size, stride, tiling_mode);

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
               const char *name,
               void *addr,
               uint32_t tiling_mode,
               uint32_t stride,
               unsigned long size,
               unsigned long flags)
{
    if (has_userptr((struct mos_bufmgr_gem *)bufmgr))
        bufmgr->bo_alloc_userptr = mos_gem_bo_alloc_userptr;
    else
        bufmgr->bo_alloc_userptr = nullptr;

    return mos_bo_alloc_userptr(bufmgr, name, addr,
                      tiling_mode, stride, size, flags);
}

/**
 * Returns a drm_intel_bo wrapping the given buffer object handle.
 *
 * This can be used when one application needs to pass a buffer object
 * to another.
 */
struct mos_linux_bo *
mos_bo_gem_create_from_name(struct mos_bufmgr *bufmgr,
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
    atomic_set(&bo_gem->refcount, 1);
    bo_gem->validate_index = -1;
    bo_gem->gem_handle = open_arg.handle;
    bo_gem->bo.handle = open_arg.handle;
    bo_gem->global_name = handle;
    bo_gem->reusable = false;
    bo_gem->use_48b_address_range = bufmgr_gem->bufmgr.bo_use_48b_address_range ? true : false;

    memclear(get_tiling);
    get_tiling.handle = bo_gem->gem_handle;
    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_GET_TILING,
               &get_tiling);
    if (ret != 0) {
        mos_gem_bo_unreference(&bo_gem->bo);
        pthread_mutex_unlock(&bufmgr_gem->lock);
        return nullptr;
    }
    bo_gem->tiling_mode = get_tiling.tiling_mode;
    bo_gem->swizzle_mode = get_tiling.swizzle_mode;
    /* XXX stride is unknown */

    mos_bo_gem_set_in_aperture_size(bufmgr_gem, bo_gem, 0);

    DRMINITLISTHEAD(&bo_gem->vma_list);
    DRMLISTADDTAIL(&bo_gem->name_list, &bufmgr_gem->named);
    pthread_mutex_unlock(&bufmgr_gem->lock);
    MOS_DBG("bo_create_from_handle: %d (%s)\n", handle, bo_gem->name);

    return &bo_gem->bo;
}

drm_export void
mos_gem_bo_free(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_gem_close close;
    int ret;

    DRMLISTDEL(&bo_gem->vma_list);
    if (bo_gem->mem_virtual) {
        VG(VALGRIND_FREELIKE_BLOCK(bo_gem->mem_virtual, 0));
        drm_munmap(bo_gem->mem_virtual, bo_gem->bo.size);
        bufmgr_gem->vma_count--;
    }
    if (bo_gem->gtt_virtual) {
        drm_munmap(bo_gem->gtt_virtual, bo_gem->bo.size);
        bufmgr_gem->vma_count--;
    }
    if (bo_gem->mem_wc_virtual) {
        VG(VALGRIND_FREELIKE_BLOCK(bo_gem->mem_wc_virtual, 0));
        drm_munmap(bo_gem->mem_wc_virtual, bo_gem->bo.size);
        bufmgr_gem->vma_count--;
    }

    /* Close this object */
    memclear(close);
    close.handle = bo_gem->gem_handle;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_GEM_CLOSE, &close);
    if (ret != 0) {
        MOS_DBG("DRM_IOCTL_GEM_CLOSE %d failed (%s): %s\n",
            bo_gem->gem_handle, bo_gem->name, strerror(errno));
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

static void mos_gem_bo_purge_vma_cache(struct mos_bufmgr_gem *bufmgr_gem)
{
    int limit;

    MOS_DBG("%s: cached=%d, open=%d, limit=%d\n", __FUNCTION__,
        bufmgr_gem->vma_count, bufmgr_gem->vma_open, bufmgr_gem->vma_max);

    if (bufmgr_gem->vma_max < 0)
        return;

    /* We may need to evict a few entries in order to create new mmaps */
    limit = bufmgr_gem->vma_max - 2*bufmgr_gem->vma_open;
    if (bufmgr_gem->has_ext_mmap)
        limit -= bufmgr_gem->vma_open;
    if (limit < 0)
        limit = 0;

    while (bufmgr_gem->vma_count > limit) {
        struct mos_bo_gem *bo_gem;

        bo_gem = DRMLISTENTRY(struct mos_bo_gem,
                      bufmgr_gem->vma_cache.next,
                      vma_list);
        assert(bo_gem->map_count == 0);
        DRMLISTDELINIT(&bo_gem->vma_list);

        if (bo_gem->mem_virtual) {
            drm_munmap(bo_gem->mem_virtual, bo_gem->bo.size);
            bo_gem->mem_virtual = nullptr;
            bufmgr_gem->vma_count--;
        }
        if (bo_gem->gtt_virtual) {
            drm_munmap(bo_gem->gtt_virtual, bo_gem->bo.size);
            bo_gem->gtt_virtual = nullptr;
            bufmgr_gem->vma_count--;
        }
        if (bo_gem->mem_wc_virtual) {
            drm_munmap(bo_gem->mem_wc_virtual, bo_gem->bo.size);
            bo_gem->mem_wc_virtual = nullptr;
            bufmgr_gem->vma_count--;
        }
    }
}

static void mos_gem_bo_close_vma(struct mos_bufmgr_gem *bufmgr_gem,
                     struct mos_bo_gem *bo_gem)
{
    bufmgr_gem->vma_open--;
    DRMLISTADDTAIL(&bo_gem->vma_list, &bufmgr_gem->vma_cache);
    if (bo_gem->mem_virtual)
        bufmgr_gem->vma_count++;
    if (bo_gem->gtt_virtual)
        bufmgr_gem->vma_count++;
    if (bo_gem->mem_wc_virtual)
        bufmgr_gem->vma_count++;
    mos_gem_bo_purge_vma_cache(bufmgr_gem);
}

static void mos_gem_bo_open_vma(struct mos_bufmgr_gem *bufmgr_gem,
                      struct mos_bo_gem *bo_gem)
{
    bufmgr_gem->vma_open++;
    DRMLISTDEL(&bo_gem->vma_list);
    if (bo_gem->mem_virtual)
        bufmgr_gem->vma_count--;
    if (bo_gem->gtt_virtual)
        bufmgr_gem->vma_count--;
    if (bo_gem->mem_wc_virtual)
        bufmgr_gem->vma_count--;
    mos_gem_bo_purge_vma_cache(bufmgr_gem);
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
        mos_gem_bo_unreference_locked_timed(bo_gem->softpin_target[i],
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
        bo_gem->softpin_target_size = 0;
    }

    /* Clear any left-over mappings */
    if (bo_gem->map_count) {
        MOS_DBG("bo freed with non-zero map-count %d\n", bo_gem->map_count);
        bo_gem->map_count = 0;
        mos_gem_bo_close_vma(bufmgr_gem, bo_gem);
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

    if (bo_gem->map_count++ == 0)
        mos_gem_bo_open_vma(bufmgr_gem, bo_gem);

    /* Get a mapping of the buffer if we haven't before. */
    if (bo_gem->mem_wc_virtual == nullptr) {
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
            if (--bo_gem->map_count == 0)
                mos_gem_bo_close_vma(bufmgr_gem, bo_gem);
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
    int ret;

    pthread_mutex_lock(&bufmgr_gem->lock);

    ret = map_wc(bo);
    if (ret) {
        pthread_mutex_unlock(&bufmgr_gem->lock);
        return ret;
    }

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
    mos_gem_bo_mark_mmaps_incoherent(bo);
    VG(VALGRIND_MAKE_MEM_DEFINED(bo_gem->mem_wc_virtual, bo->size));
    pthread_mutex_unlock(&bufmgr_gem->lock);

    return 0;
}

int
mos_gem_bo_map_wc_unsynchronized(struct mos_linux_bo *bo) {
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
#ifdef HAVE_VALGRIND
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
#endif
    int ret;

    pthread_mutex_lock(&bufmgr_gem->lock);

    ret = map_wc(bo);
    if (ret == 0) {
        mos_gem_bo_mark_mmaps_incoherent(bo);
        VG(VALGRIND_MAKE_MEM_DEFINED(bo_gem->mem_wc_virtual, bo->size));
    }

    pthread_mutex_unlock(&bufmgr_gem->lock);

    return ret;
}

drm_export int mos_gem_bo_map(struct mos_linux_bo *bo, int write_enable)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_set_domain set_domain;
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

    pthread_mutex_lock(&bufmgr_gem->lock);

    if (bo_gem->map_count++ == 0)
        mos_gem_bo_open_vma(bufmgr_gem, bo_gem);

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
            if (--bo_gem->map_count == 0)
                mos_gem_bo_close_vma(bufmgr_gem, bo_gem);
            pthread_mutex_unlock(&bufmgr_gem->lock);
            return ret;
        }
        VG(VALGRIND_MALLOCLIKE_BLOCK(mmap_arg.addr_ptr, mmap_arg.size, 0, 1));
        bo_gem->mem_virtual = (void *)(uintptr_t) mmap_arg.addr_ptr;
    }
    MOS_DBG("bo_map: %d (%s) -> %p\n", bo_gem->gem_handle, bo_gem->name,
        bo_gem->mem_virtual);
#ifdef __cplusplus
    bo->virt = bo_gem->mem_virtual;
#else
    bo->virtual = bo_gem->mem_virtual;
#endif

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

    if (bo_gem->map_count++ == 0)
        mos_gem_bo_open_vma(bufmgr_gem, bo_gem);

    /* Get a mapping of the buffer if we haven't before. */
    if (bo_gem->gtt_virtual == nullptr) {
        struct drm_i915_gem_mmap_gtt mmap_arg;

        MOS_DBG("bo_map_gtt: mmap %d (%s), map_count=%d\n",
            bo_gem->gem_handle, bo_gem->name, bo_gem->map_count);

        memclear(mmap_arg);
        mmap_arg.handle = bo_gem->gem_handle;

        /* Get the fake offset back... */
        ret = drmIoctl(bufmgr_gem->fd,
                   DRM_IOCTL_I915_GEM_MMAP_GTT,
                   &mmap_arg);
        if (ret != 0) {
            ret = -errno;
            MOS_DBG("%s:%d: Error preparing buffer map %d (%s): %s .\n",
                __FILE__, __LINE__,
                bo_gem->gem_handle, bo_gem->name,
                strerror(errno));
            if (--bo_gem->map_count == 0)
                mos_gem_bo_close_vma(bufmgr_gem, bo_gem);
            return ret;
        }

        /* and mmap it */
        bo_gem->gtt_virtual = drm_mmap(0, bo->size, PROT_READ | PROT_WRITE,
                           MAP_SHARED, bufmgr_gem->fd,
                           mmap_arg.offset);
        if (bo_gem->gtt_virtual == MAP_FAILED) {
            bo_gem->gtt_virtual = nullptr;
            ret = -errno;
            MOS_DBG("%s:%d: Error mapping buffer %d (%s): %s .\n",
                __FILE__, __LINE__,
                bo_gem->gem_handle, bo_gem->name,
                strerror(errno));
            if (--bo_gem->map_count == 0)
                mos_gem_bo_close_vma(bufmgr_gem, bo_gem);
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

int
mos_gem_bo_map_gtt(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_set_domain set_domain;
    int ret;

    pthread_mutex_lock(&bufmgr_gem->lock);

    ret = map_gtt(bo);
    if (ret) {
        pthread_mutex_unlock(&bufmgr_gem->lock);
        return ret;
    }

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

int
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

drm_export int mos_gem_bo_unmap(struct mos_linux_bo *bo)
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
        mos_gem_bo_close_vma(bufmgr_gem, bo_gem);
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

int
mos_gem_bo_unmap_wc(struct mos_linux_bo *bo)
{
    return mos_gem_bo_unmap(bo);
}

int
mos_gem_bo_unmap_gtt(struct mos_linux_bo *bo)
{
    return mos_gem_bo_unmap(bo);
}

int mos_gem_bo_get_fake_offset(struct mos_linux_bo *bo)
{
    int ret;
    struct drm_i915_gem_mmap_gtt mmap_arg;
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    memclear(mmap_arg);
    mmap_arg.handle = bo_gem->gem_handle;

    ret = drmIoctl(bufmgr_gem->fd,
            DRM_IOCTL_I915_GEM_MMAP_GTT,
            &mmap_arg);
    if (ret != 0) {
        ret = -errno;
        MOS_DBG("%s:%d: Error to get buffer fake offset %d (%s): %s .\n",
            __FILE__, __LINE__,
            bo_gem->gem_handle, bo_gem->name,
            strerror(errno));
    }
    else {
        bo->offset64 = mmap_arg.offset;
    }
    return ret;
}

drm_export int
mos_gem_bo_subdata(struct mos_linux_bo *bo, unsigned long offset,
             unsigned long size, const void *data)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_pwrite pwrite;
    int ret;

    if (bo_gem->is_userptr)
        return -EINVAL;

    memclear(pwrite);
    pwrite.handle = bo_gem->gem_handle;
    pwrite.offset = offset;
    pwrite.size = size;
    pwrite.data_ptr = (uint64_t) (uintptr_t) data;

    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_PWRITE,
               &pwrite);
    if (ret != 0) {
        ret = -errno;
        MOS_DBG("%s:%d: Error writing data to buffer %d: (%d %d) %s .\n",
            __FILE__, __LINE__, bo_gem->gem_handle, (int)offset,
            (int)size, strerror(errno));
    }

    return ret;
}

static int
mos_gem_get_pipe_from_crtc_id(struct mos_bufmgr *bufmgr, int crtc_id)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;
    struct drm_i915_get_pipe_from_crtc_id get_pipe_from_crtc_id;
    int ret;

    memclear(get_pipe_from_crtc_id);
    get_pipe_from_crtc_id.crtc_id = crtc_id;
    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GET_PIPE_FROM_CRTC_ID,
               &get_pipe_from_crtc_id);
    if (ret != 0) {
        /* We return -1 here to signal that we don't
         * know which pipe is associated with this crtc.
         * This lets the caller know that this information
         * isn't available; using the wrong pipe for
         * vblank waiting can cause the chipset to lock up
         */
        return -1;
    }

    return get_pipe_from_crtc_id.pipe;
}

static int
mos_gem_bo_get_subdata(struct mos_linux_bo *bo, unsigned long offset,
                 unsigned long size, void *data)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_pread pread;
    int ret;

    if (bo_gem->is_userptr)
        return -EINVAL;

    memclear(pread);
    pread.handle = bo_gem->gem_handle;
    pread.offset = offset;
    pread.size = size;
    pread.data_ptr = (uint64_t) (uintptr_t) data;
    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_PREAD,
               &pread);
    if (ret != 0) {
        ret = -errno;
        MOS_DBG("%s:%d: Error reading data from buffer %d: (%d %d) %s .\n",
            __FILE__, __LINE__, bo_gem->gem_handle, (int)offset,
            (int)size, strerror(errno));
    }

    return ret;
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
void
mos_gem_bo_start_gtt_access(struct mos_linux_bo *bo, int write_enable)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_set_domain set_domain;
    int ret;

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

static void
mos_bufmgr_gem_destroy(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;
    struct drm_gem_close close_bo;
    int i, ret;

    free(bufmgr_gem->exec2_objects);
    free(bufmgr_gem->exec_objects);
    free(bufmgr_gem->exec_bos);
    pthread_mutex_destroy(&bufmgr_gem->lock);

    /* Free any cached buffer objects we were going to reuse */
    for (i = 0; i < bufmgr_gem->num_buckets; i++) {
        struct mos_gem_bo_bucket *bucket =
            &bufmgr_gem->cache_bucket[i];
        struct mos_bo_gem *bo_gem;

        while (!DRMLISTEMPTY(&bucket->head)) {
            bo_gem = DRMLISTENTRY(struct mos_bo_gem,
                          bucket->head.next, head);
            DRMLISTDEL(&bo_gem->head);

            mos_gem_bo_free(&bo_gem->bo);
        }
    }

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
    free(bufmgr);
}

/**
 * Adds the target buffer to the validation list and adds the relocation
 * to the reloc_buffer's relocation list.
 *
 * The relocation entry at the given offset must already contain the
 * precomputed relocation value, because the kernel will optimize out
 * the relocation entry write when the buffer hasn't moved from the
 * last known offset in target_bo.
 */
static int
do_bo_emit_reloc(struct mos_linux_bo *bo, uint32_t offset,
         struct mos_linux_bo *target_bo, uint32_t target_offset,
         uint32_t read_domains, uint32_t write_domain,
         bool need_fence)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *) target_bo;
    bool fenced_command;

    if (bo_gem->has_error)
        return -ENOMEM;

    if (target_bo_gem->has_error) {
        bo_gem->has_error = true;
        return -ENOMEM;
    }

    /* We never use HW fences for rendering on 965+ */
    need_fence = false;

    fenced_command = need_fence;
    if (target_bo_gem->tiling_mode == I915_TILING_NONE)
        need_fence = false;

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

    bo_gem->reloc_target_info[bo_gem->reloc_count].bo = target_bo;
    if (target_bo != bo)
        mos_gem_bo_reference(target_bo);
    if (fenced_command)
        bo_gem->reloc_target_info[bo_gem->reloc_count].flags =
            DRM_INTEL_RELOC_FENCE;
    else
        bo_gem->reloc_target_info[bo_gem->reloc_count].flags = 0;

    bo_gem->relocs[bo_gem->reloc_count].offset = offset;
    bo_gem->relocs[bo_gem->reloc_count].delta = target_offset;
    bo_gem->relocs[bo_gem->reloc_count].target_handle =
        target_bo_gem->gem_handle;
    bo_gem->relocs[bo_gem->reloc_count].read_domains = read_domains;
    bo_gem->relocs[bo_gem->reloc_count].write_domain = write_domain;
    bo_gem->relocs[bo_gem->reloc_count].presumed_offset = target_bo->offset64;
    bo_gem->reloc_count++;

    return 0;
}

static int
do_bo_emit_reloc2(struct mos_linux_bo *bo, uint32_t offset,
         struct mos_linux_bo *target_bo, uint32_t target_offset,
         uint32_t read_domains, uint32_t write_domain,
         bool need_fence, uint64_t presumed_offset)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *) target_bo;
    bool fenced_command;

    if (bo_gem->has_error)
        return -ENOMEM;

    if (target_bo_gem->has_error) {
        bo_gem->has_error = true;
        return -ENOMEM;
    }

    /* We never use HW fences for rendering on 965+ */
    need_fence = false;

    fenced_command = need_fence;

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

    bo_gem->reloc_target_info[bo_gem->reloc_count].bo = target_bo;
    if (target_bo != bo)
        mos_gem_bo_reference(target_bo);
    if (fenced_command)
        bo_gem->reloc_target_info[bo_gem->reloc_count].flags =
            DRM_INTEL_RELOC_FENCE;
    else
        bo_gem->reloc_target_info[bo_gem->reloc_count].flags = 0;

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
mos_gem_bo_set_exec_object_async(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *)bo;
    bo_gem->exec_async = true;
}

static int
mos_gem_bo_add_softpin_target(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo)
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

    if (bo_gem->softpin_target_count == bo_gem->softpin_target_size) {
        int new_size = bo_gem->softpin_target_size * 2;
        if (new_size == 0)
            new_size = bufmgr_gem->max_relocs;

        bo_gem->softpin_target = (struct mos_linux_bo **)realloc(bo_gem->softpin_target, new_size *
                sizeof(struct mos_linux_bo *));
        if (!bo_gem->softpin_target)
            return -ENOMEM;

        bo_gem->softpin_target_size = new_size;
    }
    bo_gem->softpin_target[bo_gem->softpin_target_count] = target_bo;
    mos_gem_bo_reference(target_bo);
    bo_gem->softpin_target_count++;

    return 0;
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
                uint32_t read_domains, uint32_t write_domain)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bo->bufmgr;
    struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *)target_bo;

    if (target_bo_gem->is_softpin)
        return mos_gem_bo_add_softpin_target(bo, target_bo);
    else
        return do_bo_emit_reloc(bo, offset, target_bo, target_offset,
                    read_domains, write_domain,
                    !bufmgr_gem->fenced_relocs);
}

static int
mos_gem_bo_emit_reloc2(struct mos_linux_bo *bo, uint32_t offset,
                struct mos_linux_bo *target_bo, uint32_t target_offset,
                uint32_t read_domains, uint32_t write_domain,
                uint64_t presumed_offset)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bo->bufmgr;
    struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *)target_bo;

    if (target_bo_gem->is_softpin)
    {
        return mos_gem_bo_add_softpin_target(bo, target_bo);
    }
    else
    {
        return do_bo_emit_reloc2(bo, offset, target_bo, target_offset,
                    read_domains, write_domain,
                    !bufmgr_gem->fenced_relocs,
                    presumed_offset);
    }
}

static int
mos_gem_bo_emit_reloc_fence(struct mos_linux_bo *bo, uint32_t offset,
                  struct mos_linux_bo *target_bo,
                  uint32_t target_offset,
                  uint32_t read_domains, uint32_t write_domain)
{
    return do_bo_emit_reloc(bo, offset, target_bo, target_offset,
                read_domains, write_domain, true);
}

int
mos_gem_bo_get_reloc_count(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;

    return bo_gem->reloc_count;
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
drm_export void
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
            mos_gem_bo_unreference_locked_timed(&target_bo_gem->bo,
                                  time.tv_sec);
        }
    }
    bo_gem->reloc_count = start;

    for (i = 0; i < bo_gem->softpin_target_count; i++) {
        struct mos_bo_gem *target_bo_gem = (struct mos_bo_gem *) bo_gem->softpin_target[i];
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
        int need_fence;

        if (target_bo == bo)
            continue;

        mos_gem_bo_mark_mmaps_incoherent(bo);

        /* Continue walking the tree depth-first. */
        mos_gem_bo_process_reloc2(target_bo);

        need_fence = (bo_gem->reloc_target_info[i].flags &
                  DRM_INTEL_RELOC_FENCE);

        /* Add the target to the validate list */
        mos_add_validate_buffer2(target_bo, need_fence);
    }

    for (i = 0; i < bo_gem->softpin_target_count; i++) {
        struct mos_linux_bo *target_bo = bo_gem->softpin_target[i];

        if (target_bo == bo)
            continue;

        mos_gem_bo_mark_mmaps_incoherent(bo);
        mos_gem_bo_process_reloc2(target_bo);
        mos_add_validate_buffer2(target_bo, false);
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

drm_export int
mos_gem_bo_exec(struct mos_linux_bo *bo, int used,
              drm_clip_rect_t * cliprects, int num_cliprects, int DR4)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct drm_i915_gem_execbuffer execbuf;
    int ret, i;

    if (to_bo_gem(bo)->has_error)
        return -ENOMEM;

    pthread_mutex_lock(&bufmgr_gem->lock);
    /* Update indices and set up the validate list. */
    mos_gem_bo_process_reloc(bo);

    /* Add the batch buffer to the validation list.  There are no
     * relocations pointing to it.
     */
    mos_add_validate_buffer(bo);

    memclear(execbuf);
    execbuf.buffers_ptr = (uintptr_t) bufmgr_gem->exec_objects;
    execbuf.buffer_count = bufmgr_gem->exec_count;
    execbuf.batch_start_offset = 0;
    execbuf.batch_len = used;
    execbuf.cliprects_ptr = (uintptr_t) cliprects;
    execbuf.num_cliprects = num_cliprects;
    execbuf.DR1 = 0;
    execbuf.DR4 = DR4;

    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_EXECBUFFER,
               &execbuf);
    if (ret != 0) {
        ret = -errno;
        if (errno == ENOSPC) {
            MOS_DBG("Execbuffer fails to pin. "
                "Estimate: %u. Actual: %u. Available: %u\n",
                mos_gem_estimate_batch_space(bufmgr_gem->exec_bos,
                                   bufmgr_gem->
                                   exec_count),
                mos_gem_compute_batch_space(bufmgr_gem->exec_bos,
                                  bufmgr_gem->
                                  exec_count),
                (unsigned int)bufmgr_gem->gtt_size);
        }
    }
    mos_update_buffer_offsets(bufmgr_gem);

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

int
mos_gem_bo_context_exec(struct mos_linux_bo *bo, struct mos_linux_context *ctx,
                  int used, unsigned int flags)
{
    return do_exec2(bo, used, ctx, nullptr, 0, 0, flags, nullptr);
}

int
mos_gem_bo_context_exec2(struct mos_linux_bo *bo, int used, struct mos_linux_context *ctx,
                           drm_clip_rect_t *cliprects, int num_cliprects, int DR4,
                           unsigned int flags, int *fence)
{
    return do_exec2(bo, used, ctx, cliprects, num_cliprects, DR4,
                        flags, fence);
}

static int
mos_gem_bo_pin(struct mos_linux_bo *bo, uint32_t alignment)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_pin pin;
    int ret;

    memclear(pin);
    pin.handle = bo_gem->gem_handle;
    pin.alignment = alignment;

    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_PIN,
               &pin);
    if (ret != 0)
        return -errno;

    bo->offset64 = pin.offset;
    bo->offset = pin.offset;
    return 0;
}

static int
mos_gem_bo_unpin(struct mos_linux_bo *bo)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    struct drm_i915_gem_unpin unpin;
    int ret;

    memclear(unpin);
    unpin.handle = bo_gem->gem_handle;

    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_UNPIN, &unpin);
    if (ret != 0)
        return -errno;

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

    if (bo_gem->global_name == 0 &&
        tiling_mode == bo_gem->tiling_mode &&
        stride == bo_gem->stride)
        return 0;

    memset(&set_tiling, 0, sizeof(set_tiling));
    do {
        /* set_tiling is slightly broken and overwrites the
         * input on the error path, so we have to open code
         * rmIoctl.
         */
        set_tiling.handle = bo_gem->gem_handle;
        set_tiling.tiling_mode = tiling_mode;
        set_tiling.stride = stride;

        ret = ioctl(bufmgr_gem->fd,
                DRM_IOCTL_I915_GEM_SET_TILING,
                &set_tiling);
    } while (ret == -1 && (errno == EINTR || errno == EAGAIN));
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
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bo->bufmgr;
    uint64_t offset = bufmgr_gem->head_offset;

    // if offset is over 48b address range, return error
    if (offset > 0xFFFFFFFFFFFF)
    {
        MOS_DBG("softpin failed: address over 48b range");
        return -EINVAL;
    }

    if (!mos_gem_bo_is_softpin(bo))
    {
        // update the head_offset, need to be 64K aligned
        bufmgr_gem->head_offset += MOS_ALIGN_CEIL(bo->size, 64*1024);

        // softpin the BO to the given offset
        ret = mos_gem_bo_set_softpin_offset(bo, offset);
        if (ret == 0)
        {
            ret = mos_bo_use_48b_address_range(bo, 1);
        }
        return ret;
    }

    return ret;
}

struct mos_linux_bo *
mos_bo_gem_create_from_prime(struct mos_bufmgr *bufmgr, int prime_fd, int size)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;
    int ret;
    uint32_t handle;
    struct mos_bo_gem *bo_gem;
    struct drm_i915_gem_get_tiling get_tiling;
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

    bo_gem->gem_handle = handle;

    atomic_set(&bo_gem->refcount, 1);

    bo_gem->name = "prime";
    bo_gem->validate_index = -1;
    bo_gem->reloc_tree_fences = 0;
    bo_gem->used_as_reloc_target = false;
    bo_gem->has_error = false;
    bo_gem->reusable = false;
    bo_gem->use_48b_address_range = bufmgr_gem->bufmgr.bo_use_48b_address_range ? true : false;

    DRMINITLISTHEAD(&bo_gem->vma_list);
    DRMLISTADDTAIL(&bo_gem->name_list, &bufmgr_gem->named);
    pthread_mutex_unlock(&bufmgr_gem->lock);

    memclear(get_tiling);
    get_tiling.handle = bo_gem->gem_handle;
    ret = drmIoctl(bufmgr_gem->fd,
               DRM_IOCTL_I915_GEM_GET_TILING,
               &get_tiling);
    if (ret != 0) {
        MOS_DBG("create_from_prime: failed to get tiling: %s\n", strerror(errno));
        mos_gem_bo_unreference(&bo_gem->bo);
        return nullptr;
    }
    bo_gem->tiling_mode = get_tiling.tiling_mode;
    bo_gem->swizzle_mode = get_tiling.swizzle_mode;
    /* XXX stride is unknown */
    mos_bo_gem_set_in_aperture_size(bufmgr_gem, bo_gem, 0);
    return &bo_gem->bo;
}

int
mos_bo_gem_export_to_prime(struct mos_linux_bo *bo, int *prime_fd)
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
void
mos_bufmgr_gem_enable_reuse(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *) bufmgr;

    bufmgr_gem->bo_reuse = true;
}

/**
 * Enable use of fenced reloc type.
 *
 * New code should enable this to avoid unnecessary fence register
 * allocation.  If this option is not enabled, all relocs will have fence
 * register allocated.
 */
void
mos_bufmgr_gem_enable_fenced_relocs(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;

    if (bufmgr_gem->bufmgr.bo_exec == mos_gem_bo_exec2)
        bufmgr_gem->fenced_relocs = true;
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
        if (bo_gem->softpin_target[i] == target_bo)
            return 1;
        if (_mos_gem_bo_references(bo_gem->softpin_target[i], target_bo))
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

void
mos_bufmgr_gem_set_vma_cache_size(struct mos_bufmgr *bufmgr, int limit)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;

    bufmgr_gem->vma_max = limit;

    mos_gem_bo_purge_vma_cache(bufmgr_gem);
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

int
mos_bufmgr_gem_get_devid(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;

    return bufmgr_gem->pci_device;
}

/**
 * Sets up AUB dumping.
 *
 * This is a trace file format that can be used with the simulator.
 * Packets are emitted in a format somewhat like GPU command packets.
 * You can set up a GTT and upload your objects into the referenced
 * space, then send off batchbuffers and get BMPs out the other end.
 */
void
mos_bufmgr_gem_set_aub_dump(struct mos_bufmgr *bufmgr, int enable)
{
    fprintf(stderr, "libdrm aub dumping is deprecated.\n\n"
        "Use intel_aubdump from intel-gpu-tools instead.  Install intel-gpu-tools,\n"
        "then run (for example)\n\n"
        "\t$ intel_aubdump --output=trace.aub glxgears -geometry 500x500\n\n"
        "See the intel_aubdump man page for more details.\n");
}

struct mos_linux_context *
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

    return context;
}

void
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

int
mos_get_reset_stats(struct mos_linux_context *ctx,
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

unsigned int mos_hweight8(uint8_t w)
{
    uint32_t i, weight = 0;

    for (i=0; i<8; i++)
    {
        weight += !!((w) & (1UL << i));
    }
    return weight;
}

uint8_t mos_switch_off_n_bits(uint8_t in_mask, int n)
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

int
mos_get_context_param_sseu(struct mos_linux_context *ctx,
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

int
mos_set_context_param_sseu(struct mos_linux_context *ctx,
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

int
mos_get_subslice_mask(int fd, unsigned int *subslice_mask)
{
    drm_i915_getparam_t gp;
    int ret;

    memclear(gp);
    gp.value = (int*)subslice_mask;
    gp.param = I915_PARAM_SUBSLICE_MASK;
    ret = drmIoctl(fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (ret)
        return -errno;

    return 0;
}

int
mos_get_slice_mask(int fd, unsigned int *slice_mask)
{
    drm_i915_getparam_t gp;
    int ret;

    memclear(gp);
    gp.value = (int*)slice_mask;
    gp.param = I915_PARAM_SLICE_MASK;
    ret = drmIoctl(fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (ret)
        return -errno;

    return 0;
}

int
mos_get_context_param(struct mos_linux_context *ctx,
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

int
mos_set_context_param(struct mos_linux_context *ctx,
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

int
mos_reg_read(struct mos_bufmgr *bufmgr,
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

int
mos_get_subslice_total(int fd, unsigned int *subslice_total)
{
    drm_i915_getparam_t gp;
    int ret;

    memclear(gp);
    gp.value = (int*)subslice_total;
    gp.param = I915_PARAM_SUBSLICE_TOTAL;
    ret = drmIoctl(fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (ret)
        return -errno;

    return 0;
}

int
mos_get_eu_total(int fd, unsigned int *eu_total)
{
    drm_i915_getparam_t gp;
    int ret;

    memclear(gp);
    gp.value = (int*)eu_total;
    gp.param = I915_PARAM_EU_TOTAL;
    ret = drmIoctl(fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (ret)
        return -errno;

    return 0;
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

/**
 * Initializes the GEM buffer manager, which uses the kernel to allocate, map,
 * and manage map buffer objections.
 *
 * \param fd File descriptor of the opened DRM device.
 */
struct mos_bufmgr *
mos_bufmgr_gem_init(int fd, int batch_size)
{
    struct mos_bufmgr_gem *bufmgr_gem;
    struct drm_i915_gem_get_aperture aperture;
    drm_i915_getparam_t gp;
    int ret, tmp;
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

    gp.param = I915_PARAM_HAS_EXEC_SOFTPIN;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (ret == 0 && *gp.value > 0)
    {
        bufmgr_gem->bufmgr.bo_set_softpin_offset = mos_gem_bo_set_softpin_offset;
        bufmgr_gem->bufmgr.bo_set_softpin        = mos_gem_bo_set_softpin;
    }

    gp.param = I915_PARAM_HAS_EXEC_ASYNC;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if (ret == 0 && *gp.value > 0)
        bufmgr_gem->bufmgr.set_exec_object_async = mos_gem_bo_set_exec_object_async;

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

    /* Let's go with one relocation per every 2 dwords (but round down a bit
     * since a power of two will mean an extra page allocation for the reloc
     * buffer).
     *
     * Every 4 was too few for the blender benchmark.
     */
    bufmgr_gem->max_relocs = batch_size / sizeof(uint32_t) / 2 - 2;

    bufmgr_gem->bufmgr.bo_alloc = mos_gem_bo_alloc;
    bufmgr_gem->bufmgr.bo_alloc_for_render =
        mos_gem_bo_alloc_for_render;
    bufmgr_gem->bufmgr.bo_alloc_tiled = mos_gem_bo_alloc_tiled;
    bufmgr_gem->bufmgr.bo_reference = mos_gem_bo_reference;
    bufmgr_gem->bufmgr.bo_unreference = mos_gem_bo_unreference;
    bufmgr_gem->bufmgr.bo_map = mos_gem_bo_map;
    bufmgr_gem->bufmgr.bo_unmap = mos_gem_bo_unmap;
    bufmgr_gem->bufmgr.bo_subdata = mos_gem_bo_subdata;
    bufmgr_gem->bufmgr.bo_get_subdata = mos_gem_bo_get_subdata;
    bufmgr_gem->bufmgr.bo_wait_rendering = mos_gem_bo_wait_rendering;
    bufmgr_gem->bufmgr.bo_pad_to_size = mos_gem_bo_pad_to_size;
    bufmgr_gem->bufmgr.bo_emit_reloc = mos_gem_bo_emit_reloc;
    bufmgr_gem->bufmgr.bo_emit_reloc2 = mos_gem_bo_emit_reloc2;
    bufmgr_gem->bufmgr.bo_emit_reloc_fence = mos_gem_bo_emit_reloc_fence;
    bufmgr_gem->bufmgr.bo_pin = mos_gem_bo_pin;
    bufmgr_gem->bufmgr.bo_unpin = mos_gem_bo_unpin;
    bufmgr_gem->bufmgr.bo_get_tiling = mos_gem_bo_get_tiling;
    bufmgr_gem->bufmgr.bo_set_tiling = mos_gem_bo_set_tiling;

    bufmgr_gem->bufmgr.bo_flink = mos_gem_bo_flink;
    /* Use the new one if available */
    if (exec2) {
        bufmgr_gem->bufmgr.bo_exec = mos_gem_bo_exec2;
        bufmgr_gem->bufmgr.bo_mrb_exec = mos_gem_bo_mrb_exec2;
    } else
        bufmgr_gem->bufmgr.bo_exec = mos_gem_bo_exec;
    bufmgr_gem->bufmgr.bo_busy = mos_gem_bo_busy;
    bufmgr_gem->bufmgr.bo_madvise = mos_gem_bo_madvise;
    bufmgr_gem->bufmgr.destroy = mos_bufmgr_gem_unref;
    bufmgr_gem->bufmgr.debug = 0;
    bufmgr_gem->bufmgr.check_aperture_space =
        mos_gem_check_aperture_space;
    bufmgr_gem->bufmgr.bo_disable_reuse = mos_gem_bo_disable_reuse;
    bufmgr_gem->bufmgr.bo_is_reusable = mos_gem_bo_is_reusable;
    bufmgr_gem->bufmgr.get_pipe_from_crtc_id =
        mos_gem_get_pipe_from_crtc_id;
    bufmgr_gem->bufmgr.bo_references = mos_gem_bo_references;

    DRMINITLISTHEAD(&bufmgr_gem->named);
    init_cache_buckets(bufmgr_gem);

    DRMINITLISTHEAD(&bufmgr_gem->vma_cache);
    bufmgr_gem->vma_max = -1; /* unlimited by default */

    DRMLISTADD(&bufmgr_gem->managers, &bufmgr_list);

    // init head_offset to 64K, since 0 will be regarded as nullptr in some condition
    bufmgr_gem->head_offset = 65536;

exit:
    pthread_mutex_unlock(&bufmgr_list_mutex);

    return bufmgr_gem != nullptr ? &bufmgr_gem->bufmgr : nullptr;
}

struct mos_linux_context *
mos_gem_context_create_ext(struct mos_bufmgr *bufmgr, __u32 flags)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    struct drm_i915_gem_context_create_ext create;
    struct mos_linux_context *context = nullptr;
    int ret;

    context = (struct mos_linux_context *)calloc(1, sizeof(*context));
    if (!context)
        return nullptr;

    memclear(create);
    create.flags = flags;
    create.extensions = 0;
    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_CONTEXT_CREATE_EXT, &create);
    if (ret != 0) {
        MOS_DBG("DRM_IOCTL_I915_GEM_CONTEXT_CREATE failed: %s\n",
            strerror(errno));
        free(context);
        return nullptr;
    }

    context->ctx_id = create.ctx_id;
    context->bufmgr = bufmgr;

    return context;
}

struct drm_i915_gem_vm_control* mos_gem_vm_create(struct mos_bufmgr *bufmgr)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    struct drm_i915_gem_vm_control *vm = nullptr;
    int ret;

    vm = (struct drm_i915_gem_vm_control *)calloc(1, sizeof(*vm));
    if (nullptr == vm)
    {
        return nullptr;
    }
    memset(vm, 0, sizeof(*vm));

    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_VM_CREATE, vm);
    if (ret != 0) {
        MOS_DBG("DRM_IOCTL_I915_GEM_VM_CREATE failed: %s\n",
            strerror(errno));
        free(vm);
        return nullptr;
    }

    return vm;
}

void mos_gem_vm_destroy(struct mos_bufmgr *bufmgr, struct drm_i915_gem_vm_control* vm)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    assert(vm);
    int ret;

    ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_I915_GEM_VM_DESTROY, vm);
    if (ret != 0) {
        MOS_DBG("DRM_IOCTL_I915_GEM_VM_DESTROY failed: %s\n",
            strerror(errno));
    }
    free(vm);
}

struct mos_linux_context *
mos_gem_context_create_shared(struct mos_bufmgr *bufmgr, mos_linux_context* ctx, __u32 flags)
{
    struct mos_bufmgr_gem *bufmgr_gem = (struct mos_bufmgr_gem *)bufmgr;
    struct drm_i915_gem_context_create_ext create;
    struct mos_linux_context *context = nullptr;
    int ret;

    if (ctx == nullptr || ctx->vm == nullptr)
        return nullptr;

    context = (struct mos_linux_context *)calloc(1, sizeof(*context));
    if (!context)
        return nullptr;

    memclear(create);
    create.flags = flags;
    create.extensions = 0;
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
                ctx->vm->vm_id);
    if(ret != 0) {
        MOS_DBG("I915_CONTEXT_PARAM_VM failed: %s\n",
            strerror(errno));
        free(context);
        return nullptr;
    }

    return context;
}

int mos_query_engines(int fd,
                      __u16 engine_class,
                      __u64 caps,
                      unsigned int *nengine,
                      struct i915_engine_class_instance *ci)
{
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
    if (ret)
    {
        goto fini;
    }
    len = query_item.length;

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

int mos_set_context_param_load_balance(struct mos_linux_context *ctx,
                     struct i915_engine_class_instance *ci,
                     unsigned int count)
{
    int ret;
    uint32_t size;
    struct i915_context_engines_load_balance* balancer = nullptr;
    struct i915_context_param_engines* set_engines = nullptr;

    assert(ci);

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

int mos_set_context_param_bond(struct mos_linux_context *ctx,
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

drm_export bool mos_gem_bo_is_softpin(struct mos_linux_bo *bo)
{
    struct mos_bo_gem *bo_gem = (struct mos_bo_gem *) bo;
    if (bo_gem == nullptr)
    {
        return false;
    }

    return bo_gem->is_softpin;
}