/*
 * Copyright © 2007 Intel Corporation
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
 *    Eric Anholt <eric@anholt.net>
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <drm.h>
#include <i915_drm.h>
#include "libdrm_macros.h"
#include "mos_bufmgr.h"
#include "mos_bufmgr_priv.h"
#include "xf86drm.h"
#include "mos_util_debug.h"

/** @file mos_bufmgr_api.c
 *
 * Convenience functions for buffer management methods.
 */

struct mos_linux_bo *
mos_bo_alloc(struct mos_bufmgr *bufmgr,
            struct mos_drm_bo_alloc *alloc)
{
    if(!bufmgr || !alloc)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return nullptr;
    }

    if (bufmgr->bo_alloc)
    {
        return bufmgr->bo_alloc(bufmgr, alloc);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return nullptr;
    }
}

struct mos_linux_bo *
mos_bo_alloc_userptr(struct mos_bufmgr *bufmgr,
               struct mos_drm_bo_alloc_userptr *alloc_uptr)
{
    if(!bufmgr || !alloc_uptr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return nullptr;
    }

    if (bufmgr->bo_alloc_userptr)
    {
        return bufmgr->bo_alloc_userptr(bufmgr, alloc_uptr);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return nullptr;
    }
}

struct mos_linux_bo *
mos_bo_alloc_tiled(struct mos_bufmgr *bufmgr,
            struct mos_drm_bo_alloc_tiled *alloc_tiled)
{
    if(!bufmgr || !alloc_tiled)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return nullptr;
    }

    if (bufmgr->bo_alloc_tiled)
    {
        return bufmgr->bo_alloc_tiled(bufmgr, alloc_tiled);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return nullptr;
    }
}

void
mos_bo_reference(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bo->bufmgr && bo->bufmgr->bo_reference)
    {
        bo->bufmgr->bo_reference(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

void
mos_bo_unreference(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bo->bufmgr && bo->bufmgr->bo_unreference)
    {
        bo->bufmgr->bo_unreference(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

int
mos_bo_map(struct mos_linux_bo *bo, int write_enable)

{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_map)
    {
        return bo->bufmgr->bo_map(bo, write_enable);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_bo_unmap(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_unmap)
    {
        return bo->bufmgr->bo_unmap(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

void
mos_bo_wait_rendering(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bo->bufmgr && bo->bufmgr->bo_wait_rendering)
    {
        bo->bufmgr->bo_wait_rendering(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

int
mos_bo_exec(struct mos_linux_bo *bo, int used,
          drm_clip_rect_t * cliprects, int num_cliprects, int DR4)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_exec)
    {
        return bo->bufmgr->bo_exec(bo, used, cliprects, num_cliprects, DR4);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_bo_mrb_exec(struct mos_linux_bo *bo, int used,
        drm_clip_rect_t *cliprects, int num_cliprects, int DR4,
        unsigned int rings)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_mrb_exec)
    {
        return bo->bufmgr->bo_mrb_exec(bo, used,
                    cliprects, num_cliprects, DR4,
                    rings);
    }

    switch (rings) {
    case I915_EXEC_DEFAULT:
    case I915_EXEC_RENDER:
        if (bo->bufmgr && bo->bufmgr->bo_exec)
        {
            return bo->bufmgr->bo_exec(bo, used, cliprects, num_cliprects, DR4);
        }
        else
        {
            MOS_OS_CRITICALMESSAGE("Unsupported\n");
            return -EPERM;
        }
    break;
    default:
        return -ENODEV;
    }
}

int
mos_bo_emit_reloc(struct mos_linux_bo *bo, uint32_t offset,
                       struct mos_linux_bo *target_bo, uint32_t target_offset,
                       uint32_t read_domains, uint32_t write_domain,
                       uint64_t presumed_offset)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }
   
    if (bo->bufmgr && bo->bufmgr->bo_emit_reloc)
    {
        return bo->bufmgr->bo_emit_reloc(bo, offset,
                                    target_bo, target_offset,
                                    read_domains, write_domain,
                                    presumed_offset);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_bo_set_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
            uint32_t stride)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_set_tiling)
    {
        return bo->bufmgr->bo_set_tiling(bo, tiling_mode, stride);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }

    *tiling_mode = TILING_NONE;
    return -EPERM;
}

int
mos_bo_get_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
            uint32_t * swizzle_mode)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_get_tiling)
    {
        return bo->bufmgr->bo_get_tiling(bo, tiling_mode, swizzle_mode);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }

    *tiling_mode = TILING_NONE;
    *swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
    return -EPERM;
}

int
mos_bo_flink(struct mos_linux_bo *bo, uint32_t * name)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_flink)
    {
        return bo->bufmgr->bo_flink(bo, name);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -ENODEV;
    }
}

int
mos_bo_busy(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_busy)
    {
        return bo->bufmgr->bo_busy(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }

    return 0;
}

int
mos_bo_madvise(struct mos_linux_bo *bo, int madv)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_madvise)
    {
        return bo->bufmgr->bo_madvise(bo, madv);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_bo_use_48b_address_range(struct mos_linux_bo *bo, uint32_t enable)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_use_48b_address_range)
    {
        bo->bufmgr->bo_use_48b_address_range(bo, enable);
        return 0;
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -ENODEV;
    }
}

void
mos_bo_set_object_async(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if(bo->bufmgr && bo->bufmgr->set_object_async)
    {
        bo->bufmgr->set_object_async(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

void
mos_bo_set_exec_object_async(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if(bo->bufmgr && bo->bufmgr->set_exec_object_async)
    {
        bo->bufmgr->set_exec_object_async(bo, target_bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

void
mos_bo_set_object_capture(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bo->bufmgr && bo->bufmgr->set_object_capture)
    {
        bo->bufmgr->set_object_capture(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

int
mos_bo_set_softpin(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_set_softpin)
    {
        return bo->bufmgr->bo_set_softpin(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -ENODEV;
    }
}

int
mos_bo_add_softpin_target(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo, bool write_flag)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_add_softpin_target)
    {
        return bo->bufmgr->bo_add_softpin_target(bo, target_bo, write_flag);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -ENODEV;
    }
}

mos_oca_exec_list_info* mos_bo_get_softpin_targets_info(struct mos_linux_bo *bo, int *count)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return nullptr;
    }

    if (bo->bufmgr && bo->bufmgr->bo_get_softpin_targets_info)
    {
        return bo->bufmgr->bo_get_softpin_targets_info(bo, count);

    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return nullptr;
    }
}

int
mos_bo_disable_reuse(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_disable_reuse)
    {
        return bo->bufmgr->bo_disable_reuse(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }

    return 0;
}

int
mos_bo_is_reusable(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_is_reusable)
    {
        return bo->bufmgr->bo_is_reusable(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }

    return 0;
}

int
mos_bo_references(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_references)
    {
        return bo->bufmgr->bo_references(bo, target_bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -ENODEV;
    }
}

int
mos_bo_pad_to_size(struct mos_linux_bo *bo, uint64_t pad_to_size)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }
       
    if (bo->bufmgr && bo->bufmgr->bo_pad_to_size)
    {
       return bo->bufmgr->bo_pad_to_size(bo, pad_to_size);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -ENODEV;
    }
}

struct mos_linux_bo *
mos_bo_create_from_name(struct mos_bufmgr *bufmgr,
                        const char *name,
                        unsigned int handle)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return nullptr;
    }

    if (bufmgr->bo_create_from_name)
    {
        return bufmgr->bo_create_from_name(bufmgr, name, handle);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return nullptr;
    }
}

int
mos_bo_map_unsynchronized(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_map_unsynchronized)
    {
    return bo->bufmgr->bo_map_unsynchronized(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }

}

int
mos_bo_map_gtt(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_map_gtt)
    {
        return bo->bufmgr->bo_map_gtt(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_bo_unmap_gtt(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_unmap_gtt)
    {
        return bo->bufmgr->bo_unmap_gtt(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

drm_export int
mos_bo_map_wc(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_map_wc)
    {
        return bo->bufmgr->bo_map_wc(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_bo_unmap_wc(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_unmap_wc)
    {
        return bo->bufmgr->bo_unmap_wc(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

void
mos_bo_start_gtt_access(struct mos_linux_bo *bo, int write_enable)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bo->bufmgr && bo->bufmgr->bo_start_gtt_access)
    {
        bo->bufmgr->bo_start_gtt_access(bo, write_enable);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

int
mos_bo_context_exec2(struct mos_linux_bo *bo, int used, struct mos_linux_context *ctx,
                               struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                               unsigned int flags, int *fence)

{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_context_exec2)
    {
        return bo->bufmgr->bo_context_exec2(bo, used, ctx, cliprects, num_cliprects, DR4, flags, fence);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}
                               
int
mos_bo_context_exec3(struct mos_linux_bo **bo, int num_bo, struct mos_linux_context *ctx,
                               struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                               unsigned int flags, int *fence)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if ((*bo)->bufmgr && (*bo)->bufmgr->bo_context_exec3)
    {
        return (*bo)->bufmgr->bo_context_exec3(bo, num_bo, ctx, cliprects, num_cliprects, DR4, flags, fence);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

drm_export bool
mos_bo_is_exec_object_async(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return false;
    }

    if (bo->bufmgr && bo->bufmgr->bo_is_exec_object_async)
    {
        return bo->bufmgr->bo_is_exec_object_async(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return false;
    }
}

drm_export bool
mos_bo_is_softpin(struct mos_linux_bo *bo)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return false;
    }

    if (bo->bufmgr && bo->bufmgr->bo_is_softpin)
    {
        return bo->bufmgr->bo_is_softpin(bo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return false;
    }
}

drm_export int
mos_bo_wait(struct mos_linux_bo *bo, int64_t timeout_ns)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_wait)
    {
        return bo->bufmgr->bo_wait(bo, timeout_ns);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

drm_export void
mos_bo_clear_relocs(struct mos_linux_bo *bo, int start)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bo->bufmgr && bo->bufmgr->bo_clear_relocs)
    {
        bo->bufmgr->bo_clear_relocs(bo, start);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

int
mos_bo_export_to_prime(struct mos_linux_bo *bo, int *prime_fd)
{
    if(!bo)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo->bufmgr && bo->bufmgr->bo_export_to_prime)
    {
        return bo->bufmgr->bo_export_to_prime(bo, prime_fd);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

struct mos_linux_bo *
mos_bo_create_from_prime(struct mos_bufmgr *bufmgr,
                        struct mos_drm_bo_alloc_prime *alloc_prime)
{
    if(!bufmgr || !alloc_prime)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return nullptr;
    }

    if (bufmgr->bo_create_from_prime)
    {
        return bufmgr->bo_create_from_prime(bufmgr, alloc_prime);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return nullptr;
    }
}

int
mos_bufmgr_check_aperture_space(struct mos_linux_bo ** bo_array, int count)
{
    if(!bo_array)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bo_array[0]->bufmgr && bo_array[0]->bufmgr->check_aperture_space)
    {
        return bo_array[0]->bufmgr->check_aperture_space(bo_array, count);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

void
mos_bufmgr_destroy(struct mos_bufmgr *bufmgr)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bufmgr->destroy)
    {
        bufmgr->destroy(bufmgr);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

void
mos_bufmgr_set_debug(struct mos_bufmgr *bufmgr, int enable_debug)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    bufmgr->debug = enable_debug;
}

struct mos_linux_context *
mos_context_create(struct mos_bufmgr *bufmgr)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return nullptr;
    }

    if (bufmgr->context_create)
    {
        return bufmgr->context_create(bufmgr);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return nullptr;
    }
}

struct mos_linux_context *
mos_context_create_ext(
                            struct mos_bufmgr *bufmgr,
                            __u32 flags,
                            bool bContextProtected)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return nullptr;
    }

    if (bufmgr->context_create_ext)
    {
        return bufmgr->context_create_ext(bufmgr, flags, bContextProtected);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return nullptr;
    }

}

struct mos_linux_context *
mos_context_create_shared(
                            struct mos_bufmgr *bufmgr,
                            mos_linux_context* ctx,
                            __u32 flags,
                            bool bContextProtected,
                            void *engine_map,
                            uint8_t ctx_width,
                            uint8_t num_placements,
                            uint32_t ctx_type)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return nullptr;
    }

    if (bufmgr->context_create_shared)
    {
        return bufmgr->context_create_shared(bufmgr, ctx, flags, bContextProtected,
                    engine_map, ctx_width, num_placements, ctx_type);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return nullptr;
    }
}

__u32
mos_vm_create(struct mos_bufmgr *bufmgr)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return INVALID_VM;
    }

    if (bufmgr->vm_create)
    {    
        return bufmgr->vm_create(bufmgr);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return INVALID_VM;
    }
}

void
mos_vm_destroy(struct mos_bufmgr *bufmgr, __u32 vm_id)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bufmgr->vm_destroy)
    {
        bufmgr->vm_destroy(bufmgr, vm_id);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

void 
mos_bufmgr_enable_reuse(struct mos_bufmgr *bufmgr)
{   
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bufmgr->enable_reuse)
    {
        bufmgr->enable_reuse(bufmgr);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

void
mos_bufmgr_enable_softpin(struct mos_bufmgr *bufmgr, bool va1m_align)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bufmgr->enable_softpin)
    {
        bufmgr->enable_softpin(bufmgr, va1m_align);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

void
mos_bufmgr_enable_vmbind(struct mos_bufmgr *bufmgr)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bufmgr->enable_vmbind)
    {
        bufmgr->enable_vmbind(bufmgr);
    }
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

void
mos_bufmgr_disable_object_capture(struct mos_bufmgr *bufmgr)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bufmgr->disable_object_capture)
    {
        bufmgr->disable_object_capture(bufmgr);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

int
mos_bufmgr_get_memory_info(struct mos_bufmgr *bufmgr, char *info, uint32_t length)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bufmgr->get_memory_info)
    {
        return bufmgr->get_memory_info(bufmgr, info, length);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_bufmgr_get_devid(struct mos_bufmgr *bufmgr)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bufmgr->get_devid)
    {
        return bufmgr->get_devid(bufmgr);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

void
mos_bufmgr_realloc_cache(struct mos_bufmgr *bufmgr, uint8_t alloc_mode)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bufmgr->realloc_cache)
    {
        return bufmgr->realloc_cache(bufmgr, alloc_mode);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

int
mos_query_engines_count(struct mos_bufmgr *bufmgr,
                      unsigned int *nengine)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bufmgr->query_engines_count)
    {
        return bufmgr->query_engines_count(bufmgr, nengine);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_query_engines(struct mos_bufmgr *bufmgr,
                      __u16 engine_class,
                      __u64 caps,
                      unsigned int *nengine,
                      void *ci)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bufmgr->query_engines)
    {
        return bufmgr->query_engines(bufmgr, engine_class, caps, nengine, ci);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported bufmgr:0x%x\n", bufmgr);
        return -EPERM;
    }
}

size_t
mos_get_engine_class_size(struct mos_bufmgr *bufmgr)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return 0;
    }

    if (bufmgr->get_engine_class_size)
    {
        return bufmgr->get_engine_class_size();
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return 0;
    }
}

int
mos_query_sys_engines(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bufmgr->query_sys_engines)
    {
        return bufmgr->query_sys_engines(bufmgr, gfx_info);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }

}

int
mos_query_device_blob(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bufmgr->query_device_blob)
    {
        return bufmgr->query_device_blob(bufmgr, gfx_info);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

void
mos_select_fixed_engine(struct mos_bufmgr *bufmgr,
            void *engine_map,
            uint32_t *nengine,
            uint32_t fixed_instance_mask)
{
    if (bufmgr && bufmgr->select_fixed_engine)
    {
        return bufmgr->select_fixed_engine(bufmgr,
                    engine_map,
                    nengine,
                    fixed_instance_mask);
    }

}

int
mos_get_driver_info(struct mos_bufmgr *bufmgr, struct LinuxDriverInfo *drvInfo)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bufmgr->get_driver_info)
    {
        return bufmgr->get_driver_info(bufmgr, drvInfo);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_query_hw_ip_version(struct mos_bufmgr *bufmgr, __u16 engine_class, void *ip_ver_info)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bufmgr->query_hw_ip_version)
    {
        return bufmgr->query_hw_ip_version(bufmgr, engine_class, ip_ver_info);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

uint64_t
mos_get_platform_information(struct mos_bufmgr *bufmgr)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return 0;
    }

    if (bufmgr->get_platform_information)
    {
        return bufmgr->get_platform_information(bufmgr);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }

    return 0;
}

void
mos_set_platform_information(struct mos_bufmgr *bufmgr, uint64_t p)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bufmgr->set_platform_information)
    {
        bufmgr->set_platform_information(bufmgr, p);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

bool
mos_has_bsd2(struct mos_bufmgr *bufmgr)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return false;
    }

    if (bufmgr->has_bsd2)
    {
        return bufmgr->has_bsd2(bufmgr);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return false;
    }
}

void
mos_enable_turbo_boost(struct mos_bufmgr *bufmgr)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (bufmgr->enable_turbo_boost)
    {
        bufmgr->enable_turbo_boost(bufmgr);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

int
mos_get_ts_frequency(struct mos_bufmgr *bufmgr, uint32_t *ts_freq)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bufmgr->get_ts_frequency)
    {
        return bufmgr->get_ts_frequency(bufmgr, ts_freq);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}


int
mos_reg_read(struct mos_bufmgr *bufmgr,
               uint32_t offset,
               uint64_t *result)
{
    if(!bufmgr)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (bufmgr->reg_read)
    {
        return bufmgr->reg_read(bufmgr, offset, result);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_get_reset_stats(struct mos_linux_context *ctx,
                  uint32_t *reset_count,
                  uint32_t *active,
                  uint32_t *pending)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (ctx->bufmgr && ctx->bufmgr->get_reset_stats)
    {
        return ctx->bufmgr->get_reset_stats(ctx, reset_count, active, pending);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_get_context_param_sseu(struct mos_linux_context *ctx,
                struct drm_i915_gem_context_param_sseu *sseu)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (ctx->bufmgr && ctx->bufmgr->get_context_param_sseu)
    {
        return ctx->bufmgr->get_context_param_sseu(ctx, sseu);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_set_context_param_sseu(struct mos_linux_context *ctx,
                struct drm_i915_gem_context_param_sseu sseu)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (ctx->bufmgr && ctx->bufmgr->set_context_param_sseu)
    {
        return ctx->bufmgr->set_context_param_sseu(ctx, sseu);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_set_context_param(struct mos_linux_context *ctx,
                uint32_t size,
                uint64_t param,
                uint64_t value)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (ctx->bufmgr && ctx->bufmgr->set_context_param)
    {
        return ctx->bufmgr->set_context_param(ctx, size, param, value);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_set_context_param_parallel(struct mos_linux_context *ctx,
                         struct i915_engine_class_instance *ci,
                         unsigned int count)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (ctx->bufmgr && ctx->bufmgr->set_context_param_parallel)
    {
        return ctx->bufmgr->set_context_param_parallel(ctx, ci, count);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }

}

int
mos_set_context_param_load_balance(struct mos_linux_context *ctx,
                         struct i915_engine_class_instance *ci,
                         unsigned int count)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (ctx->bufmgr && ctx->bufmgr->set_context_param_load_balance)
    {
        return ctx->bufmgr->set_context_param_load_balance(ctx, ci, count);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_set_context_param_bond(struct mos_linux_context *ctx,
                        struct i915_engine_class_instance master_ci,
                        struct i915_engine_class_instance *bond_ci,
                        unsigned int bond_count)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (ctx->bufmgr && ctx->bufmgr->set_context_param_bond)
    {
        return ctx->bufmgr->set_context_param_bond(ctx, master_ci, bond_ci, bond_count);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

int
mos_get_context_param(struct mos_linux_context *ctx,
                           uint32_t size,
                           uint64_t param,
                           uint64_t *value)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return -EINVAL;
    }

    if (ctx->bufmgr && ctx->bufmgr->get_context_param)
    {
        return ctx->bufmgr->get_context_param(ctx, size, param, value);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
        return -EPERM;
    }
}

uint8_t
mos_switch_off_n_bits(struct mos_linux_context *ctx, uint8_t in_mask, int n)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return 0;
    }

    if (ctx->bufmgr && ctx->bufmgr->switch_off_n_bits)
    {
        return ctx->bufmgr->switch_off_n_bits(ctx, in_mask, n);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }

    return 0;
}

unsigned int
mos_hweight8(struct mos_linux_context *ctx, uint8_t w)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return 0;
    }

    if (ctx->bufmgr && ctx->bufmgr->hweight8)
    {
        return ctx->bufmgr->hweight8(ctx, w);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }

    return 0;
}

void
mos_context_destroy(struct mos_linux_context *ctx)
{
    if(!ctx)
    {
        MOS_OS_CRITICALMESSAGE("Input null ptr\n");
        return;
    }

    if (ctx->bufmgr && ctx->bufmgr->context_destroy)
    {
        ctx->bufmgr->context_destroy(ctx);
    }
    else
    {
        MOS_OS_CRITICALMESSAGE("Unsupported\n");
    }
}

