/*
 * Copyright © 2020 Intel Corporation
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
 * @file mos_bufmgr_priv.h
 *
 * Private definitions of Intel-specific bufmgr functions and structures.
 */

#ifndef MOS_BUFMGR_PRIV_H
#define MOS_BUFMGR_PRIV_H

/**
 * Context for a buffer manager instance.
 *
 * Contains public methods followed by private storage for the buffer manager.
 */
struct mos_bufmgr {
    /**
     * Allocate a buffer object.
     *
     * Buffer objects are not necessarily initially mapped into CPU virtual
     * address space or graphics device aperture.  They must be mapped
     * using bo_map() or drm_intel_gem_bo_map_gtt() to be used by the CPU.
     */
    struct mos_linux_bo *(*bo_alloc) (struct mos_bufmgr *bufmgr,
                   struct mos_drm_bo_alloc *alloc) = nullptr;

    /**
     * Allocate a buffer object from an existing user accessible
     * address malloc'd with the provided size.
     * Alignment is used when mapping to the gtt.
     * Flags may be I915_VMAP_READ_ONLY or I915_USERPTR_UNSYNCHRONIZED
     */
    struct mos_linux_bo *(*bo_alloc_userptr)(struct mos_bufmgr *bufmgr,
                      struct mos_drm_bo_alloc_userptr *alloc_uptr) = nullptr;

    /**
     * Allocate a tiled buffer object.
     *
     * Alignment for tiled objects is set automatically; the 'flags'
     * argument provides a hint about how the object will be used initially.
     *
     * Valid tiling formats are:
     *  TILING_NONE
     *  TILING_X
     *  TILING_Y
     *
     * Note the tiling format may be rejected; callers should check the
     * 'tiling_mode' field on return, as well as the pitch value, which
     * may have been rounded up to accommodate for tiling restrictions.
     */
    struct mos_linux_bo *(*bo_alloc_tiled) (struct mos_bufmgr *bufmgr,
                     struct mos_drm_bo_alloc_tiled *alloc_tiled) = nullptr;

    /** Takes a reference on a buffer object */
    void (*bo_reference) (struct mos_linux_bo *bo) = nullptr;

    /**
     * Releases a reference on a buffer object, freeing the data if
     * no references remain.
     */
    void (*bo_unreference) (struct mos_linux_bo *bo) = nullptr;

    /**
     * Maps the buffer into userspace.
     *
     * This function will block waiting for any existing execution on the
     * buffer to complete, first.  The resulting mapping is available at
     * buf->virtual.
     */
    int (*bo_map) (struct mos_linux_bo *bo, int write_enable) = nullptr;

    /**
     * Reduces the refcount on the userspace mapping of the buffer
     * object.
     */
    int (*bo_unmap) (struct mos_linux_bo *bo) = nullptr;

    /**
     * Waits for rendering to an object by the GPU to have completed.
     *
     * This is not required for any access to the BO by bo_map,
     * bo_subdata, etc.  It is merely a way for the driver to implement
     * glFinish.
     */
    void (*bo_wait_rendering) (struct mos_linux_bo *bo) = nullptr;

    /**
     * Tears down the buffer manager instance.
     */
    void (*destroy) (struct mos_bufmgr *bufmgr) = nullptr;

    /**
     * Indicate if the buffer can be placed anywhere in the full ppgtt
     * address range (2^48).
     *
     * Any resource used with flat/heapless (0x00000000-0xfffff000)
     * General State Heap (GSH) or Intructions State Heap (ISH) must
     * be in a 32-bit range. 48-bit range will only be used when explicitly
     * requested.
     *
     * \param bo Buffer to set the use_48b_address_range flag.
     * \param enable The flag value.
     */
    void (*bo_use_48b_address_range) (struct mos_linux_bo *bo, uint32_t enable) = nullptr;

    /**
     * Sets buffer total padded size when buffer is used by the GPU.
     *
     * This enables dynamic padding to be added without using any backing
     * storage. For example handling GPU padding requirements for buffers
     * allocated by an entity unaware of the same.
     *
     * Set padded size remains active until reset (to zero or actual object
     * size).
     *
     * Returns 0 on success or an error code.
     *
     * \param bo Buffer to set total padded size for
     * \param pad_to_size Total size in bytes of object plus padding
     */
    int (*bo_pad_to_size) (struct mos_linux_bo *bo, uint64_t pad_to_size) = nullptr;

    /**
     * Add relocation entry in reloc_buf, which will be updated with the
     * target buffer's real offset on on command submission.
     *
     * Relocations remain in place for the lifetime of the buffer object.
     *
     * \param bo Buffer to write the relocation into.
     * \param offset Byte offset within reloc_bo of the pointer to
     *            target_bo.
     * \param target_bo Buffer whose offset should be written into the
     *                  relocation entry.
     * \param target_offset Constant value to be added to target_bo's
     *            offset in relocation entry.
     * \param read_domains GEM read domains which the buffer will be
     *            read into by the command that this relocation
     *            is part of.
     * \param write_domains GEM read domains which the buffer will be
     *            dirtied in by the command that this
     *            relocation is part of.
     */
    int (*bo_emit_reloc) (struct mos_linux_bo *bo, uint32_t offset,
                  struct mos_linux_bo *target_bo, uint32_t target_offset,
                  uint32_t read_domains, uint32_t write_domain,
                  uint64_t presumed_offset) = nullptr;

    /** Executes the command buffer pointed to by bo. */
    int (*bo_exec) (struct mos_linux_bo *bo, int used,
            drm_clip_rect_t *cliprects, int num_cliprects,
            int DR4) = nullptr;
    /** Executes the command buffer pointed to by bo on the selected
     * ring buffer
     */
    int (*bo_mrb_exec) (struct mos_linux_bo *bo, int used,
                drm_clip_rect_t *cliprects, int num_cliprects,
                int DR4, unsigned flags) = nullptr;

    /**
     * Ask that the buffer be placed in tiling mode
     *
     * \param buf Buffer to set tiling mode for
     * \param tiling_mode desired, and returned tiling mode
     */
    int (*bo_set_tiling) (struct mos_linux_bo *bo, uint32_t * tiling_mode,
                  uint32_t stride) = nullptr;

    /**
     * Get the current tiling (and resulting swizzling) mode for the bo.
     *
     * \param buf Buffer to get tiling mode for
     * \param tiling_mode returned tiling mode
     * \param swizzle_mode returned swizzling mode
     */
    int (*bo_get_tiling) (struct mos_linux_bo *bo, uint32_t * tiling_mode,
                  uint32_t * swizzle_mode) = nullptr;

    /**
     * Softpin the buffer object 
     * \param bo Buffer to set the softpin
     */
    int (*bo_set_softpin) (struct mos_linux_bo *bo) = nullptr;

    /**
     * Add softpin object to softpin target list of the command buffer 
     * \param bo Command buffer which store the softpin target list
     * \param target_bo  Softpin target to be added
     * \param write_flag  Whether write flag is needed
     */
    int (*bo_add_softpin_target) (struct mos_linux_bo *bo, struct mos_linux_bo *target_bo, bool write_flag) = nullptr;

    /**
     * Create a visible name for a buffer which can be used by other apps
     *
     * \param buf Buffer to create a name for
     * \param name Returned name
     */
    int (*bo_flink) (struct mos_linux_bo *bo, uint32_t * name) = nullptr;

    /**
     * Returns 1 if mapping the buffer for write could cause the process
     * to block, due to the object being active in the GPU.
     */
    int (*bo_busy) (struct mos_linux_bo *bo) = nullptr;

    /**
     * Specify the volatility of the buffer.
     * \param bo Buffer to create a name for
     * \param madv The purgeable status
     *
     * Use I915_MADV_DONTNEED to mark the buffer as purgeable, and it will be
     * reclaimed under memory pressure. If you subsequently require the buffer,
     * then you must pass I915_MADV_WILLNEED to mark the buffer as required.
     *
     * Returns 1 if the buffer was retained, or 0 if it was discarded whilst
     * marked as I915_MADV_DONTNEED.
     */
    int (*bo_madvise) (struct mos_linux_bo *bo, int madv) = nullptr;

    int (*check_aperture_space) (struct mos_linux_bo ** bo_array, int count) = nullptr;

    /**
     * Disable buffer reuse for buffers which will be shared in some way,
     * as with scanout buffers. When the buffer reference count goes to
     * zero, it will be freed and not placed in the reuse list.
     *
     * \param bo Buffer to disable reuse for
     */
    int (*bo_disable_reuse) (struct mos_linux_bo *bo) = nullptr;

    /**
     * Query whether a buffer is reusable.
     *
     * \param bo Buffer to query
     */
    int (*bo_is_reusable) (struct mos_linux_bo *bo) = nullptr;

    /** Returns true if target_bo is in the relocation tree rooted at bo. */
    int (*bo_references) (struct mos_linux_bo *bo, struct mos_linux_bo *target_bo) = nullptr;

    /**
     * Set async flag for a buffer object.
     *
     * \param bo Buffer to set async
     */
    void (*set_object_async) (struct mos_linux_bo *bo) = nullptr;

    /**
     * Set execution async flag for a buffer object.
     *
     * \param bo Command buffer
     * \param target_bo Buffer to set async
     */
    void (*set_exec_object_async) (struct mos_linux_bo *bo, struct mos_linux_bo *target_bo) = nullptr;

    /**
    * Set capture flag for a buffer object.
    *
    * \param bo Buffer to set capture
    */
    void (*set_object_capture)(struct mos_linux_bo *bo) = nullptr;

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
    int (*bo_wait)(struct mos_linux_bo *bo, int64_t timeout_ns) = nullptr;

    void (*bo_clear_relocs)(struct mos_linux_bo *bo, int start) = nullptr;
    struct mos_linux_context *(*context_create)(struct mos_bufmgr *bufmgr) = nullptr;
    struct mos_linux_context *(*context_create_ext)(
                                struct mos_bufmgr *bufmgr,
                                __u32 flags,
                                bool bContextProtected) = nullptr;
    struct mos_linux_context *(*context_create_shared)(
                                struct mos_bufmgr *bufmgr,
                                mos_linux_context* ctx,
                                __u32 flags,
                                bool bContextProtected,
                                void *engine_map,
                                uint8_t ctx_width,
                                uint8_t num_placements,
                                uint32_t ctx_type) = nullptr;
    void (*context_destroy)(struct mos_linux_context *ctx) = nullptr;
    __u32 (*vm_create)(struct mos_bufmgr *bufmgr) = nullptr;
    void (*vm_destroy)(struct mos_bufmgr *bufmgr, __u32 vm_id) = nullptr;
    int (*bo_context_exec2)(struct mos_linux_bo *bo, int used, struct mos_linux_context *ctx,
                                   struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                                   unsigned int flags, int *fence) = nullptr;
    
    int (*bo_context_exec3)(struct mos_linux_bo **bo, int num_bo, struct mos_linux_context *ctx,
                                   struct drm_clip_rect *cliprects, int num_cliprects, int DR4,
                                   unsigned int flags, int *fence) = nullptr;
    bool (*bo_is_exec_object_async)(struct mos_linux_bo *bo) = nullptr;
    bool (*bo_is_softpin)(struct mos_linux_bo *bo) = nullptr;
    int (*bo_map_gtt)(struct mos_linux_bo *bo) = nullptr;
    int (*bo_unmap_gtt)(struct mos_linux_bo *bo) = nullptr;
    int (*bo_map_wc)(struct mos_linux_bo *bo) = nullptr;
    int (*bo_unmap_wc)(struct mos_linux_bo *bo) = nullptr;
    int (*bo_map_unsynchronized)(struct mos_linux_bo *bo) = nullptr;
    void (*bo_start_gtt_access)(struct mos_linux_bo *bo, int write_enable) = nullptr;
    mos_oca_exec_list_info* (*bo_get_softpin_targets_info)(struct mos_linux_bo *bo, int *count) = nullptr;
    struct mos_linux_bo *(*bo_create_from_name)(struct mos_bufmgr *bufmgr,
                            const char *name,
                            unsigned int handle) = nullptr;
    void (*enable_reuse)(struct mos_bufmgr *bufmgr) = nullptr;
    void (*enable_softpin)(struct mos_bufmgr *bufmgr, bool va1m_align) = nullptr;
    void (*enable_vmbind)(struct mos_bufmgr *bufmgr) = nullptr;
    void (*disable_object_capture)(struct mos_bufmgr *bufmgr) = nullptr;
    int (*get_memory_info)(struct mos_bufmgr *bufmgr, char *info, uint32_t length) = nullptr;
    int (*get_devid)(struct mos_bufmgr *bufmgr) = nullptr;
    void (*realloc_cache)(struct mos_bufmgr *bufmgr, uint8_t alloc_mode) = nullptr;
    int (*query_engines_count)(struct mos_bufmgr *bufmgr,
                          unsigned int *nengine) = nullptr;
    
    int (*query_engines)(struct mos_bufmgr *bufmgr,
                          __u16 engine_class,
                          __u64 caps,
                          unsigned int *nengine,
                          void *ci) = nullptr;

    size_t (*get_engine_class_size)() = nullptr;

    void (*select_fixed_engine)(struct mos_bufmgr *bufmgr,
            void *engine_map,
            uint32_t *nengine,
            uint32_t fixed_instance_mask) = nullptr;

    int (*set_context_param)(struct mos_linux_context *ctx,
                    uint32_t size,
                    uint64_t param,
                    uint64_t value) = nullptr;
    int (*set_context_param_parallel)(struct mos_linux_context *ctx,
                             struct i915_engine_class_instance *ci,
                             unsigned int count) = nullptr;
    int (*set_context_param_load_balance)(struct mos_linux_context *ctx,
                             struct i915_engine_class_instance *ci,
                             unsigned int count) = nullptr;
    int (*set_context_param_bond)(struct mos_linux_context *ctx,
                            struct i915_engine_class_instance master_ci,
                            struct i915_engine_class_instance *bond_ci,
                            unsigned int bond_count) = nullptr;
    int (*get_context_param)(struct mos_linux_context *ctx,
                               uint32_t size,
                               uint64_t param,
                               uint64_t *value) = nullptr;
    struct mos_linux_bo *(*bo_create_from_prime)(struct mos_bufmgr *bufmgr,
                            struct mos_drm_bo_alloc_prime *alloc_prime) = nullptr;
    int (*bo_export_to_prime)(struct mos_linux_bo *bo, int *prime_fd) = nullptr;
    int (*reg_read)(struct mos_bufmgr *bufmgr,
                   uint32_t offset,
                   uint64_t *result) = nullptr;
    int (*get_reset_stats)(struct mos_linux_context *ctx,
                      uint32_t *reset_count,
                      uint32_t *active,
                      uint32_t *pending) = nullptr;
    int (*get_context_param_sseu)(struct mos_linux_context *ctx,
                    struct drm_i915_gem_context_param_sseu *sseu) = nullptr;
    int (*set_context_param_sseu)(struct mos_linux_context *ctx,
                    struct drm_i915_gem_context_param_sseu sseu) = nullptr;
    int (*query_sys_engines)(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info) = nullptr;
    int (*query_device_blob)(struct mos_bufmgr *bufmgr, MEDIA_SYSTEM_INFO* gfx_info) = nullptr;
    int (*get_driver_info)(struct mos_bufmgr *bufmgr, struct LinuxDriverInfo *drvInfo) = nullptr;
    int (*query_hw_ip_version)(struct mos_bufmgr *bufmgr, __u16 engine_class, void *ip_ver_info) = nullptr;
    uint64_t (*get_platform_information)(struct mos_bufmgr *bufmgr) = nullptr;
    void (*set_platform_information)(struct mos_bufmgr *bufmgr, uint64_t p) = nullptr;
    int (*get_ts_frequency)(struct mos_bufmgr *bufmgr, uint32_t *ts_freq) = nullptr;
    bool (*has_bsd2)(struct mos_bufmgr *bufmgr) = nullptr;
    void (*enable_turbo_boost)(struct mos_bufmgr *bufmgr) = nullptr;
    uint8_t (*switch_off_n_bits)(struct mos_linux_context *ctx, uint8_t in_mask, int n) = nullptr;
    unsigned int (*hweight8)(struct mos_linux_context *ctx, uint8_t w) = nullptr;

    /**< Enables verbose debugging printouts */
    int debug = 0;
    /** used for reserved info*/
    uint32_t *get_reserved = nullptr;
    uint32_t tile_id = 0;
    bool     has_full_vd = true;
    uint64_t platform_information = 0;
};

#define ALIGN(value, alignment)    ((value + alignment - 1) & ~(alignment - 1))
#define ROUND_UP_TO(x, y)    (((x) + (y) - 1) / (y) * (y))
#define ROUND_UP_TO_MB(x)    ROUND_UP_TO((x), 1024*1024)

#endif /* INTEL_BUFMGR_PRIV_H */
