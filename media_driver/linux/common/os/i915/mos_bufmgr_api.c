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
#include <pciaccess.h>
#include "libdrm_macros.h"
#include "mos_bufmgr.h"
#include "mos_bufmgr_priv.h"
#include "xf86drm.h"

/** @file mos_bufmgr_api.c
 *
 * Convenience functions for buffer management methods.
 */

struct mos_linux_bo *
mos_bo_alloc(struct mos_bufmgr *bufmgr, const char *name,
           unsigned long size, unsigned int alignment)
{
    return bufmgr->bo_alloc(bufmgr, name, size, alignment);
}

void
mos_bo_set_exec_object_async(struct mos_linux_bo *bo)
{
    if(bo->bufmgr->set_exec_object_async)
        bo->bufmgr->set_exec_object_async(bo);
}

struct mos_linux_bo *
mos_bo_alloc_for_render(struct mos_bufmgr *bufmgr, const char *name,
                  unsigned long size, unsigned int alignment)
{
    return bufmgr->bo_alloc_for_render(bufmgr, name, size, alignment);
}

struct mos_linux_bo *
mos_bo_alloc_userptr(struct mos_bufmgr *bufmgr,
               const char *name, void *addr,
               uint32_t tiling_mode,
               uint32_t stride,
               unsigned long size,
               unsigned long flags)
{
    if (bufmgr->bo_alloc_userptr)
        return bufmgr->bo_alloc_userptr(bufmgr, name, addr, tiling_mode,
                        stride, size, flags);
    return nullptr;
}

struct mos_linux_bo *
mos_bo_alloc_tiled(struct mos_bufmgr *bufmgr, const char *name,
                        int x, int y, int cpp, uint32_t *tiling_mode,
                        unsigned long *pitch, unsigned long flags)
{
    return bufmgr->bo_alloc_tiled(bufmgr, name, x, y, cpp,
                      tiling_mode, pitch, flags);
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

int
mos_bo_subdata(struct mos_linux_bo *bo, unsigned long offset,
             unsigned long size, const void *data)
{
    return bo->bufmgr->bo_subdata(bo, offset, size, data);
}

int
mos_bo_get_subdata(struct mos_linux_bo *bo, unsigned long offset,
             unsigned long size, void *data)
{
    int ret;
    if (bo->bufmgr->bo_get_subdata)
        return bo->bufmgr->bo_get_subdata(bo, offset, size, data);

    if (size == 0 || data == nullptr)
        return 0;

    ret = mos_bo_map(bo, 0);
    if (ret)
        return ret;
#ifdef __cplusplus
    memcpy(data, (unsigned char *)bo->virt + offset, size);
#else
    memcpy(data, (unsigned char *)bo->virtual + offset, size);
#endif
    mos_bo_unmap(bo);
    return 0;
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
mos_bo_emit_reloc2(struct mos_linux_bo *bo, uint32_t offset,
                       struct mos_linux_bo *target_bo, uint32_t target_offset,
                       uint32_t read_domains, uint32_t write_domain,
                       uint64_t presumed_offset)
{
       return bo->bufmgr->bo_emit_reloc2(bo, offset,
                                        target_bo, target_offset,
                                        read_domains, write_domain,
                                        presumed_offset);
}

int
mos_bo_emit_reloc(struct mos_linux_bo *bo, uint32_t offset,
            struct mos_linux_bo *target_bo, uint32_t target_offset,
            uint32_t read_domains, uint32_t write_domain)
{
    return bo->bufmgr->bo_emit_reloc(bo, offset,
                     target_bo, target_offset,
                     read_domains, write_domain);
}

/* For fence registers, not GL fences */
int
mos_bo_emit_reloc_fence(struct mos_linux_bo *bo, uint32_t offset,
                  struct mos_linux_bo *target_bo, uint32_t target_offset,
                  uint32_t read_domains, uint32_t write_domain)
{
    return bo->bufmgr->bo_emit_reloc_fence(bo, offset,
                           target_bo, target_offset,
                           read_domains, write_domain);
}

int
mos_bo_pin(struct mos_linux_bo *bo, uint32_t alignment)
{
    if (bo->bufmgr->bo_pin)
        return bo->bufmgr->bo_pin(bo, alignment);

    return -ENODEV;
}

int
mos_bo_unpin(struct mos_linux_bo *bo)
{
    if (bo->bufmgr->bo_unpin)
        return bo->bufmgr->bo_unpin(bo);

    return -ENODEV;
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
mos_bo_set_softpin_offset(struct mos_linux_bo *bo, uint64_t offset)
{
    if (bo->bufmgr->bo_set_softpin_offset)
        return bo->bufmgr->bo_set_softpin_offset(bo, offset);

    return -ENODEV;
}

int
mos_bo_set_softpin(struct mos_linux_bo *bo)
{
    if (bo->bufmgr->bo_set_softpin)
        return bo->bufmgr->bo_set_softpin(bo);

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

int
mos_get_pipe_from_crtc_id(struct mos_bufmgr *bufmgr, int crtc_id)
{
    if (bufmgr->get_pipe_from_crtc_id)
        return bufmgr->get_pipe_from_crtc_id(bufmgr, crtc_id);
    return -1;
}

static size_t
mos_probe_agp_aperture_size(int fd)
{
    struct pci_device *pci_dev;
    size_t size = 0;
    int ret;

    ret = pci_system_init();
    if (ret)
        goto err;

    /* XXX handle multiple adaptors? */
    pci_dev = pci_device_find_by_slot(0, 0, 2, 0);
    if (pci_dev == nullptr)
        goto err;

    ret = pci_device_probe(pci_dev);
    if (ret)
        goto err;

    size = pci_dev->regions[2].size;
err:
    pci_system_cleanup ();
    return size;
}

int
mos_get_aperture_sizes(int fd, size_t *mappable, size_t *total)
{

    struct drm_i915_gem_get_aperture aperture;
    int ret;

    ret = drmIoctl(fd, DRM_IOCTL_I915_GEM_GET_APERTURE, &aperture);
    if (ret)
        return ret;

    *mappable = 0;
    /* XXX add a query for the kernel value? */
    if (*mappable == 0)
        *mappable = mos_probe_agp_aperture_size(fd);
    if (*mappable == 0)
        *mappable = 64 * 1024 * 1024; /* minimum possible value */
    *total = aperture.aper_size;
    return 0;
}
