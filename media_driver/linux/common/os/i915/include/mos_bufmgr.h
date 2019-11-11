/*
 * Copyright © 2008-2018 Intel Corporation
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

/**
 * @file mos_bufmgr.h
 *
 * Public definitions of Intel-specific bufmgr functions.
 */

#ifndef MOS_BUFMGR_H
#define MOS_BUFMGR_H

#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include "libdrm_macros.h"

struct drm_clip_rect;
struct mos_bufmgr;
struct mos_linux_context;

typedef struct mos_linux_bo MOS_LINUX_BO;
typedef struct mos_linux_context MOS_LINUX_CONTEXT;
typedef struct mos_bufmgr MOS_BUFMGR;

#include "mos_os_specific.h"
struct mos_linux_context {
    unsigned int ctx_id;
    struct mos_bufmgr *bufmgr;
    struct _MOS_OS_CONTEXT    *pOsContext;
    struct drm_i915_gem_vm_control* vm;
};

struct mos_linux_bo {
    /**
     * Size in bytes of the buffer object.
     *
     * The size may be larger than the size originally requested for the
     * allocation, such as being aligned to page size.
     */
    unsigned long size;

    /**
     * Alignment requirement for object
     *
     * Used for GTT mapping & pinning the object.
     */
    unsigned long align;

    /**
     * Deprecated field containing (possibly the low 32-bits of) the last
     * seen virtual card address.  Use offset64 instead.
     */
    unsigned long offset;

    /**
     * Virtual address for accessing the buffer data.  Only valid while
     * mapped.
     */
#ifdef __cplusplus
    void *virt;
#else
    void *virtual;
#endif

    /** Buffer manager context associated with this buffer object */
    struct mos_bufmgr *bufmgr;

    /**
     * MM-specific handle for accessing object
     */
    int handle;

    /**
     * Last seen card virtual address (offset from the beginning of the
     * aperture) for the object.  This should be used to fill relocation
     * entries when calling drm_intel_bo_emit_reloc()
     */
    uint64_t offset64;

    /**
     * indicate if the bo mapped into aux table
     */
    bool aux_mapped;
};

enum mos_aub_dump_bmp_format {
    MOS_AUB_DUMP_BMP_FORMAT_8BIT = 1,
    MOS_AUB_DUMP_BMP_FORMAT_ARGB_4444 = 4,
    MOS_AUB_DUMP_BMP_FORMAT_ARGB_0888 = 6,
    MOS_AUB_DUMP_BMP_FORMAT_ARGB_8888 = 7,
};

struct mos_aub_annotation {
    uint32_t type;
    uint32_t subtype;
    uint32_t ending_offset;
};

#define BO_ALLOC_FOR_RENDER (1<<0)

struct mos_linux_bo *mos_bo_alloc(struct mos_bufmgr *bufmgr, const char *name,
                 unsigned long size, unsigned int alignment);
struct mos_linux_bo *mos_bo_alloc_for_render(struct mos_bufmgr *bufmgr,
                        const char *name,
                        unsigned long size,
                        unsigned int alignment);
struct mos_linux_bo *mos_bo_alloc_userptr(struct mos_bufmgr *bufmgr,
                    const char *name,
                    void *addr, uint32_t tiling_mode,
                    uint32_t stride, unsigned long size,
                    unsigned long flags);
struct mos_linux_bo *mos_bo_alloc_tiled(struct mos_bufmgr *bufmgr,
                       const char *name,
                       int x, int y, int cpp,
                       uint32_t *tiling_mode,
                       unsigned long *pitch,
                       unsigned long flags);
void mos_bo_reference(struct mos_linux_bo *bo);
void mos_bo_unreference(struct mos_linux_bo *bo);
int mos_bo_map(struct mos_linux_bo *bo, int write_enable);
int mos_bo_unmap(struct mos_linux_bo *bo);

int mos_bo_subdata(struct mos_linux_bo *bo, unsigned long offset,
             unsigned long size, const void *data);
int mos_bo_get_subdata(struct mos_linux_bo *bo, unsigned long offset,
                 unsigned long size, void *data);
void mos_bo_wait_rendering(struct mos_linux_bo *bo);

void mos_bufmgr_set_debug(struct mos_bufmgr *bufmgr, int enable_debug);
void mos_bufmgr_destroy(struct mos_bufmgr *bufmgr);
int mos_bo_exec(struct mos_linux_bo *bo, int used,
              struct drm_clip_rect *cliprects, int num_cliprects, int DR4);
int mos_bo_mrb_exec(struct mos_linux_bo *bo, int used,
            struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
            unsigned int flags);
int mos_bufmgr_check_aperture_space(struct mos_linux_bo ** bo_array, int count);

int mos_bo_emit_reloc(struct mos_linux_bo *bo, uint32_t offset,
                struct mos_linux_bo *target_bo, uint32_t target_offset,
                uint32_t read_domains, uint32_t write_domain);
int mos_bo_emit_reloc2(struct mos_linux_bo *bo, uint32_t offset,
         struct mos_linux_bo *target_bo, uint32_t target_offset,
         uint32_t read_domains, uint32_t write_domain,
         uint64_t presumed_offset);
int mos_bo_emit_reloc_fence(struct mos_linux_bo *bo, uint32_t offset,
                  struct mos_linux_bo *target_bo,
                  uint32_t target_offset,
                  uint32_t read_domains, uint32_t write_domain);
int mos_bo_pin(struct mos_linux_bo *bo, uint32_t alignment);
int mos_bo_unpin(struct mos_linux_bo *bo);
int mos_bo_set_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
                uint32_t stride);
int mos_bo_get_tiling(struct mos_linux_bo *bo, uint32_t * tiling_mode,
                uint32_t * swizzle_mode);

int mos_bo_flink(struct mos_linux_bo *bo, uint32_t * name);
int mos_bo_busy(struct mos_linux_bo *bo);
int mos_bo_madvise(struct mos_linux_bo *bo, int madv);
int mos_bo_use_48b_address_range(struct mos_linux_bo *bo, uint32_t enable);
void mos_bo_set_exec_object_async(struct mos_linux_bo *bo);
int mos_bo_set_softpin_offset(struct mos_linux_bo *bo, uint64_t offset);
int mos_bo_set_softpin(struct mos_linux_bo *bo);

int mos_bo_disable_reuse(struct mos_linux_bo *bo);
int mos_bo_is_reusable(struct mos_linux_bo *bo);
int mos_bo_references(struct mos_linux_bo *bo, struct mos_linux_bo *target_bo);
int mos_bo_pad_to_size(struct mos_linux_bo *bo, uint64_t pad_to_size);

/* drm_intel_bufmgr_gem.c */
struct mos_bufmgr *mos_bufmgr_gem_init(int fd, int batch_size);
struct mos_linux_bo *mos_bo_gem_create_from_name(struct mos_bufmgr *bufmgr,
                        const char *name,
                        unsigned int handle);
void mos_bufmgr_gem_enable_reuse(struct mos_bufmgr *bufmgr);
void mos_bufmgr_gem_enable_fenced_relocs(struct mos_bufmgr *bufmgr);
void mos_bufmgr_gem_set_vma_cache_size(struct mos_bufmgr *bufmgr,
                         int limit);
int mos_gem_bo_map_unsynchronized(struct mos_linux_bo *bo);
int mos_gem_bo_map_gtt(struct mos_linux_bo *bo);
int mos_gem_bo_unmap_gtt(struct mos_linux_bo *bo);
int mos_gem_bo_map_wc_unsynchronized(struct mos_linux_bo *bo);
int mos_gem_bo_unmap_wc(struct mos_linux_bo *bo);

int mos_gem_bo_get_fake_offset(struct mos_linux_bo *bo);
int mos_gem_bo_get_reloc_count(struct mos_linux_bo *bo);
void mos_gem_bo_start_gtt_access(struct mos_linux_bo *bo, int write_enable);

void
mos_bufmgr_gem_set_aub_filename(struct mos_bufmgr *bufmgr,
                      const char *filename);
void mos_bufmgr_gem_set_aub_dump(struct mos_bufmgr *bufmgr, int enable);
void mos_gem_bo_aub_dump_bmp(struct mos_linux_bo *bo,
                   int x1, int y1, int width, int height,
                   enum mos_aub_dump_bmp_format format,
                   int pitch, int offset);
void
mos_bufmgr_gem_set_aub_annotations(struct mos_linux_bo *bo,
                     struct mos_aub_annotation *annotations,
                     unsigned count);

int mos_get_pipe_from_crtc_id(struct mos_bufmgr *bufmgr, int crtc_id);

int mos_get_aperture_sizes(int fd, size_t *mappable, size_t *total);
int mos_bufmgr_gem_get_devid(struct mos_bufmgr *bufmgr);

struct mos_linux_context *mos_gem_context_create(struct mos_bufmgr *bufmgr);
struct mos_linux_context *mos_gem_context_create_ext(
                            struct mos_bufmgr *bufmgr,
                            __u32 flags);
struct mos_linux_context *mos_gem_context_create_shared(
                            struct mos_bufmgr *bufmgr,
                            mos_linux_context* ctx,
                            __u32 flags);
struct drm_i915_gem_vm_control* mos_gem_vm_create(struct mos_bufmgr *bufmgr);
void mos_gem_vm_destroy(struct mos_bufmgr *bufmgr, struct drm_i915_gem_vm_control* vm);

#define MAX_ENGINE_INSTANCE_NUM 8

int mos_query_engines(int fd,
                      __u16 engine_class,
                      __u64 caps,
                      unsigned int *nengine,
                      struct i915_engine_class_instance *ci);
int mos_set_context_param_load_balance(struct mos_linux_context *ctx,
                         struct i915_engine_class_instance *ci,
                         unsigned int count);
int mos_set_context_param_bond(struct mos_linux_context *ctx,
                        struct i915_engine_class_instance master_ci,
                        struct i915_engine_class_instance *bond_ci,
                        unsigned int bond_count);

void mos_gem_context_destroy(struct mos_linux_context *ctx);
int mos_gem_bo_context_exec(struct mos_linux_bo *bo, struct mos_linux_context *ctx,
                  int used, unsigned int flags);
int
mos_gem_bo_context_exec2(struct mos_linux_bo *bo, int used, struct mos_linux_context *ctx,
                               struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                               unsigned int flags, int *fence);
int mos_bo_gem_export_to_prime(struct mos_linux_bo *bo, int *prime_fd);
struct mos_linux_bo *mos_bo_gem_create_from_prime(struct mos_bufmgr *bufmgr,
                        int prime_fd, int size);

/* drm_intel_bufmgr_fake.c */
struct mos_bufmgr *mos_bufmgr_fake_init(int fd,
                         unsigned long low_offset,
                         void *low_virtual,
                         unsigned long size,
                         volatile unsigned int
                         *last_dispatch);
void mos_bufmgr_fake_set_last_dispatch(struct mos_bufmgr *bufmgr,
                         volatile unsigned int
                         *last_dispatch);
void mos_bufmgr_fake_set_exec_callback(struct mos_bufmgr *bufmgr,
                         int (*exec) (struct mos_linux_bo *bo,
                              unsigned int used,
                              void *priv),
                         void *priv);
void mos_bufmgr_fake_set_fence_callback(struct mos_bufmgr *bufmgr,
                          unsigned int (*emit) (void *priv),
                          void (*wait) (unsigned int fence,
                                void *priv),
                          void *priv);
struct mos_linux__bo *mos_bo_fake_alloc_static(struct mos_bufmgr *bufmgr,
                         const char *name,
                         unsigned long offset,
                         unsigned long size, void *virt);
void mos_bo_fake_disable_backing_store(struct mos_linux_bo *bo,
                     void (*invalidate_cb) (struct mos_linux_bo * bo,
                                    void *ptr),
                         void *ptr);

void mos_bufmgr_fake_contended_lock_take(struct mos_bufmgr *bufmgr);
void mos_bufmgr_fake_evict_all(struct mos_bufmgr *bufmgr);

struct mos_decode *mos_decode_context_alloc(uint32_t devid);
void mos_decode_context_free(struct mos_decode *ctx);
void mos_decode_set_batch_pointer(struct mos_decode *ctx,
                    void *data, uint32_t hw_offset,
                    int count);
void mos_decode_set_dump_past_end(struct mos_decode *ctx,
                    int dump_past_end);
void mos_decode_set_head_tail(struct mos_decode *ctx,
                    uint32_t head, uint32_t tail);
void mos_decode_set_output_file(struct mos_decode *ctx, FILE *out);
void mos_decode(struct mos_decode *ctx);

int mos_reg_read(struct mos_bufmgr *bufmgr,
               uint32_t offset,
               uint64_t *result);

int mos_get_reset_stats(struct mos_linux_context *ctx,
                  uint32_t *reset_count,
                  uint32_t *active,
                  uint32_t *pending);

int mos_get_context_param(struct mos_linux_context *ctx,
                           uint32_t size,
                           uint64_t param,
                           uint64_t *value);

int mos_set_context_param(struct mos_linux_context *ctx,
                uint32_t size,
                uint64_t param,
                uint64_t value);

int mos_get_subslice_total(int fd, unsigned int *subslice_total);
int mos_get_eu_total(int fd, unsigned int *eu_total);

int mos_get_context_param_sseu(struct mos_linux_context *ctx,
                struct drm_i915_gem_context_param_sseu *sseu);
int mos_set_context_param_sseu(struct mos_linux_context *ctx,
                struct drm_i915_gem_context_param_sseu sseu);
int mos_get_subslice_mask(int fd, unsigned int *subslice_mask);
int mos_get_slice_mask(int fd, unsigned int *slice_mask);
uint8_t mos_switch_off_n_bits(uint8_t in_mask, int n);
unsigned int mos_hweight8(uint8_t w);

#if defined(__cplusplus)
extern "C" {
#endif

drm_export int mos_gem_bo_map_wc(struct mos_linux_bo *bo);
drm_export void mos_gem_bo_clear_relocs(struct mos_linux_bo *bo, int start);
drm_export int mos_gem_bo_wait(struct mos_linux_bo *bo, int64_t timeout_ns);
drm_export struct mos_linux_bo *
mos_gem_bo_alloc_internal(struct mos_bufmgr *bufmgr,
                const char *name,
                unsigned long size,
                unsigned long flags,
                uint32_t tiling_mode,
                unsigned long stride,
                unsigned int alignment);
drm_export int
mos_gem_bo_exec(struct mos_linux_bo *bo, int used,
              drm_clip_rect_t * cliprects, int num_cliprects, int DR4);
drm_export int do_exec2(struct mos_linux_bo *bo, int used, struct mos_linux_context *ctx,
     drm_clip_rect_t *cliprects, int num_cliprects, int DR4,
     unsigned int flags, int *fence);

drm_export void mos_gem_bo_free(struct mos_linux_bo *bo);
drm_export void mos_gem_bo_unreference_final(struct mos_linux_bo *bo, time_t time);
drm_export int mos_gem_bo_map(struct mos_linux_bo *bo, int write_enable);
drm_export int map_gtt(struct mos_linux_bo *bo);
drm_export int mos_gem_bo_unmap(struct mos_linux_bo *bo);
drm_export int mos_gem_bo_subdata(struct mos_linux_bo *bo, unsigned long offset,
             unsigned long size, const void *data);

drm_export bool mos_gem_bo_is_softpin(struct mos_linux_bo *bo);
#if defined(__cplusplus)
}
#endif

#endif /* INTEL_BUFMGR_H */
