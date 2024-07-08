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
#include "drm.h"
#include "i915_drm.h"
#include "libdrm_macros.h"
#include "mos_bufmgr.h"
#include "mos_bufmgr_priv.h"
#include "xf86drm.h"

/** @file mos_bufmgr_api_mock.c
 *
 * Convenience functions for buffer management methods.
 */

struct mos_linux_bo *
mos_bo_alloc(struct mos_bufmgr *bufmgr,
            struct mos_drm_bo_alloc *alloc)
{
    if (bufmgr && alloc && bufmgr->bo_alloc)
    {
        return bufmgr->bo_alloc(bufmgr, alloc);
    }
    return nullptr;
}

struct mos_linux_bo *
mos_bo_alloc_userptr(struct mos_bufmgr *bufmgr,
               struct mos_drm_bo_alloc_userptr *alloc_uptr)
{
    if (bufmgr && alloc_uptr && bufmgr->bo_alloc_userptr)
    {
        return bufmgr->bo_alloc_userptr(bufmgr, alloc_uptr);
    }
    return nullptr;
}

struct mos_linux_bo *
mos_bo_alloc_tiled(struct mos_bufmgr *bufmgr,
            struct mos_drm_bo_alloc_tiled *alloc_tiled)
{
    if (bufmgr && alloc_tiled && bufmgr->bo_alloc_tiled)
    {
        return bufmgr->bo_alloc_tiled(bufmgr, alloc_tiled);
    }
    return nullptr;
}

void
mos_bo_reference(struct mos_linux_bo *bo)
{
    bo->bufmgr->bo_reference(bo);
}

void
mos_bo_unreference(struct mos_linux_bo *bo)
{
    if (bo == nullptr)
        return;

    bo->bufmgr->bo_unreference(bo);
}

int
mos_bo_map(struct mos_linux_bo *buf, int write_enable)
{
    return buf->bufmgr->bo_map(buf, write_enable);
}

int
mos_bo_unmap(struct mos_linux_bo *buf)
{
    return buf->bufmgr->bo_unmap(buf);
}

void
mos_bo_wait_rendering(struct mos_linux_bo *bo)
{
    bo->bufmgr->bo_wait_rendering(bo);
}

void
mos_bufmgr_destroy(struct mos_bufmgr *bufmgr)
{
    bufmgr->destroy(bufmgr);
}

int
mos_bo_exec(struct mos_linux_bo *bo, int used,
          drm_clip_rect_t * cliprects, int num_cliprects, int DR4)
{
    return bo->bufmgr->bo_exec(bo, used, cliprects, num_cliprects, DR4);
}

int
mos_bo_mrb_exec(struct mos_linux_bo *bo, int used,
        drm_clip_rect_t *cliprects, int num_cliprects, int DR4,
        unsigned int rings)
{
    if (bo->bufmgr->bo_mrb_exec)
        return bo->bufmgr->bo_mrb_exec(bo, used,
                    cliprects, num_cliprects, DR4,
                    rings);

    switch (rings) {
    case I915_EXEC_DEFAULT:
    case I915_EXEC_RENDER:
        return bo->bufmgr->bo_exec(bo, used,
                       cliprects, num_cliprects, DR4);
    default:
        return -ENODEV;
    }
}

void
mos_bufmgr_set_debug(struct mos_bufmgr *bufmgr, int enable_debug)
{
    bufmgr->debug = enable_debug;
}

int
mos_bufmgr_check_aperture_space(struct mos_linux_bo ** bo_array, int count)
{
    return bo_array[0]->bufmgr->check_aperture_space(bo_array, count);
}

int
mos_bo_flink(struct mos_linux_bo *bo, uint32_t * name)
{
    if (bo->bufmgr->bo_flink)
        return bo->bufmgr->bo_flink(bo, name);

    return -ENODEV;
}

int
mos_bo_emit_reloc(struct mos_linux_bo *bo, uint32_t offset,
                       struct mos_linux_bo *target_bo, uint32_t target_offset,
                       uint32_t read_domains, uint32_t write_domain,
                       uint64_t presumed_offset)
{
       return bo->bufmgr->bo_emit_reloc(bo, offset,
                                        target_bo, target_offset,
                                        read_domains, write_domain,
                                        presumed_offset);
}

int
mos_bo_set_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
            uint32_t stride)
{
    if (bo->bufmgr->bo_set_tiling)
        return bo->bufmgr->bo_set_tiling(bo, tiling_mode, stride);

    *tiling_mode = I915_TILING_NONE;
    return 0;
}

int
mos_bo_get_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
            uint32_t * swizzle_mode)
{
    if (bo->bufmgr->bo_get_tiling)
        return bo->bufmgr->bo_get_tiling(bo, tiling_mode, swizzle_mode);

    *tiling_mode = I915_TILING_NONE;
    *swizzle_mode = I915_BIT_6_SWIZZLE_NONE;
    return 0;
}

int
mos_bo_set_softpin(struct mos_linux_bo *bo)
{
    if (bo->bufmgr->bo_set_softpin)
        return bo->bufmgr->bo_set_softpin(bo);

    return -ENODEV;
}

int
mos_bo_add_softpin_target(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo, bool write_flag)
{
    if (bo->bufmgr->bo_add_softpin_target)
        return bo->bufmgr->bo_add_softpin_target(bo, target_bo, write_flag);

    return -ENODEV;
}

int
mos_bo_disable_reuse(struct mos_linux_bo *bo)
{
    if (bo->bufmgr->bo_disable_reuse)
        return bo->bufmgr->bo_disable_reuse(bo);
    return 0;
}

int
mos_bo_is_reusable(struct mos_linux_bo *bo)
{
    if (bo->bufmgr->bo_is_reusable)
        return bo->bufmgr->bo_is_reusable(bo);
    return 0;
}

int
mos_bo_busy(struct mos_linux_bo *bo)
{
    if (bo->bufmgr->bo_busy)
        return bo->bufmgr->bo_busy(bo);
    return 0;
}

int
mos_bo_madvise(struct mos_linux_bo *bo, int madv)
{
    if (bo->bufmgr->bo_madvise)
        return bo->bufmgr->bo_madvise(bo, madv);
    return -1;
}

int
mos_bo_use_48b_address_range(struct mos_linux_bo *bo, uint32_t enable)
{
    if (bo->bufmgr->bo_use_48b_address_range) {
        bo->bufmgr->bo_use_48b_address_range(bo, enable);
        return 0;
    }

    return -ENODEV;
}

void
mos_bo_set_object_async(struct mos_linux_bo *bo)
{
    if( bo->bufmgr->set_object_async)
        bo->bufmgr->set_object_async(bo);
}

void
mos_bo_set_exec_object_async(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo)
{
    if( bo->bufmgr->set_exec_object_async)
        bo->bufmgr->set_exec_object_async(bo, target_bo);
}

int
mos_bo_pad_to_size(struct mos_linux_bo *bo, uint64_t pad_to_size)
{
       return bo->bufmgr->bo_pad_to_size(bo, pad_to_size);
}

int
mos_bo_references(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo)
{
    return bo->bufmgr->bo_references(bo, target_bo);
}

drm_export int
mos_bo_wait(struct mos_linux_bo *bo, int64_t timeout_ns)
{
    return bo->bufmgr->bo_wait(bo, timeout_ns);
}

drm_export void
mos_bo_clear_relocs(struct mos_linux_bo *bo, int start)
{
    bo->bufmgr->bo_clear_relocs(bo, start);
}

struct mos_linux_context *
mos_context_create(struct mos_bufmgr *bufmgr)
{
    return bufmgr->context_create(bufmgr);
}

struct mos_linux_context *
mos_context_create_ext(
                            struct mos_bufmgr *bufmgr,
                            __u32 flags,
                            bool bContextProtected)
{
    return bufmgr->context_create_ext(bufmgr, flags, bContextProtected);

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
    return bufmgr->context_create_shared(bufmgr, ctx, flags, bContextProtected,
                engine_map, ctx_width, num_placements, ctx_type);
}

void
mos_context_destroy(struct mos_linux_context *ctx)
{
    return ctx->bufmgr->context_destroy(ctx);
}

__u32
mos_vm_create(struct mos_bufmgr *bufmgr)
{
    return bufmgr->vm_create(bufmgr);
}

void
mos_vm_destroy(struct mos_bufmgr *bufmgr, __u32 vm_id)
{
    return bufmgr->vm_destroy(bufmgr, vm_id);
}

int
mos_bo_context_exec2(struct mos_linux_bo *bo, int used, struct mos_linux_context *ctx,
                               struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                               unsigned int flags, int *fence)

{
    return bo->bufmgr->bo_context_exec2(bo, used, ctx, cliprects, num_cliprects, DR4, flags, fence);
}
                               
int
mos_bo_context_exec3(struct mos_linux_bo **bo, int num_bo, struct mos_linux_context *ctx,
                               struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                               unsigned int flags, int *fence)
{
    return (*bo)->bufmgr->bo_context_exec3(bo, num_bo, ctx, cliprects, num_cliprects, DR4, flags, fence);
}

drm_export bool
mos_bo_is_softpin(struct mos_linux_bo *bo)
{
    return bo->bufmgr->bo_is_softpin(bo);
}

int
mos_bo_map_gtt(struct mos_linux_bo *bo)
{
    return bo->bufmgr->bo_map_gtt(bo);
}
int
mos_bo_unmap_gtt(struct mos_linux_bo *bo)
{
    return bo->bufmgr->bo_unmap_gtt(bo);
}

drm_export int
mos_bo_map_wc(struct mos_linux_bo *bo)
{
    return bo->bufmgr->bo_map_wc(bo);
}

int
mos_bo_unmap_wc(struct mos_linux_bo *bo)
{
    return bo->bufmgr->bo_unmap_wc(bo);
}

int
mos_bo_map_unsynchronized(struct mos_linux_bo *bo)
{
    return bo->bufmgr->bo_map_unsynchronized(bo);
}

void
mos_bo_start_gtt_access(struct mos_linux_bo *bo, int write_enable)
{
    return bo->bufmgr->bo_start_gtt_access(bo, write_enable);
}

int
mos_set_context_param(struct mos_linux_context *ctx,
                uint32_t size,
                uint64_t param,
                uint64_t value)
{
    return ctx->bufmgr->set_context_param(ctx, size, param, value);
}

int
mos_set_context_param_load_balance(struct mos_linux_context *ctx,
                         struct i915_engine_class_instance *ci,
                         unsigned int count)
{
    return ctx->bufmgr->set_context_param_load_balance(ctx, ci, count);
}

int
mos_get_context_param(struct mos_linux_context *ctx,
                           uint32_t size,
                           uint64_t param,
                           uint64_t *value)
{
    return ctx->bufmgr->get_context_param(ctx, size, param, value);
}

struct mos_linux_bo *
mos_bo_create_from_prime(struct mos_bufmgr *bufmgr,
                        struct mos_drm_bo_alloc_prime *alloc_prime)
{
    return bufmgr->bo_create_from_prime(bufmgr, alloc_prime);
}

int
mos_bo_export_to_prime(struct mos_linux_bo *bo, int *prime_fd)
{
    return bo->bufmgr->bo_export_to_prime(bo, prime_fd);
}

int
mos_reg_read(struct mos_bufmgr *bufmgr,
               uint32_t offset,
               uint64_t *result)
{
    return bufmgr->reg_read(bufmgr, offset, result);
}

int
mos_get_reset_stats(struct mos_linux_context *ctx,
                  uint32_t *reset_count,
                  uint32_t *active,
                  uint32_t *pending)
{
    return ctx->bufmgr->get_reset_stats(ctx, reset_count, active, pending);
}

int
mos_get_context_param_sseu(struct mos_linux_context *ctx,
                struct drm_i915_gem_context_param_sseu *sseu)
{
    return ctx->bufmgr->get_context_param_sseu(ctx, sseu);
}

int
mos_set_context_param_sseu(struct mos_linux_context *ctx,
                struct drm_i915_gem_context_param_sseu sseu)
{
    return ctx->bufmgr->set_context_param_sseu(ctx, sseu);
}
                
uint64_t
mos_get_platform_information(struct mos_bufmgr *bufmgr)
{
    return bufmgr->get_platform_information(bufmgr);
}

void
mos_set_platform_information(struct mos_bufmgr *bufmgr, uint64_t p)
{
    return bufmgr->set_platform_information(bufmgr, p);
}

uint8_t
mos_switch_off_n_bits(struct mos_linux_context *ctx, uint8_t in_mask, int n)
{
    return ctx->bufmgr->switch_off_n_bits(ctx, in_mask, n);
}

unsigned int
mos_hweight8(struct mos_linux_context *ctx, uint8_t w)
{
    return ctx->bufmgr->hweight8(ctx, w);
}

