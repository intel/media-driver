/*
 * Copyright © 2008 Intel Corporation
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
    struct mos_linux_bo *(*bo_alloc) (struct mos_bufmgr *bufmgr, const char *name,
                   unsigned long size, unsigned int alignment);

#ifdef ANDROID
    /**
     * Allocate a buffer object, hinting where the object is supposed to be
     * placed (like backed by stolen memory area or by shmem) or it should
     * be pre-populated or not, by using flags parameter.
     */
    struct mos_linux_bo *(*bo_alloc2) (struct mos_bufmgr *bufmgr, const char *name,
                   unsigned long size, unsigned int alignment,
                   unsigned long flags);
#endif
    /**
     * Allocate a buffer object, hinting that it will be used as a
     * render target.
     *
     * This is otherwise the same as bo_alloc.
     */
    struct mos_linux_bo *(*bo_alloc_for_render) (struct mos_bufmgr *bufmgr,
                          const char *name,
                          unsigned long size,
                          unsigned int alignment);

    /**
     * Allocate a buffer object from an existing user accessible
     * address malloc'd with the provided size.
     * Alignment is used when mapping to the gtt.
     * Flags may be I915_VMAP_READ_ONLY or I915_USERPTR_UNSYNCHRONIZED
     */
    struct mos_linux_bo *(*bo_alloc_userptr)(struct mos_bufmgr *bufmgr,
                      const char *name, void *addr,
                      uint32_t tiling_mode, uint32_t stride,
                      unsigned long size,
                      unsigned long flags);

    /**
     * Allocate a tiled buffer object.
     *
     * Alignment for tiled objects is set automatically; the 'flags'
     * argument provides a hint about how the object will be used initially.
     *
     * Valid tiling formats are:
     *  I915_TILING_NONE
     *  I915_TILING_X
     *  I915_TILING_Y
     *
     * Note the tiling format may be rejected; callers should check the
     * 'tiling_mode' field on return, as well as the pitch value, which
     * may have been rounded up to accommodate for tiling restrictions.
     */
    struct mos_linux_bo *(*bo_alloc_tiled) (struct mos_bufmgr *bufmgr,
                     const char *name,
                     int x, int y, int cpp,
                     uint32_t *tiling_mode,
                     unsigned long *pitch,
                     unsigned long flags);

    /** Takes a reference on a buffer object */
    void (*bo_reference) (struct mos_linux_bo *bo);

    /**
     * Releases a reference on a buffer object, freeing the data if
     * no references remain.
     */
    void (*bo_unreference) (struct mos_linux_bo *bo);

    /**
     * Maps the buffer into userspace.
     *
     * This function will block waiting for any existing execution on the
     * buffer to complete, first.  The resulting mapping is available at
     * buf->virtual.
     */
    int (*bo_map) (struct mos_linux_bo *bo, int write_enable);

    /**
     * Reduces the refcount on the userspace mapping of the buffer
     * object.
     */
    int (*bo_unmap) (struct mos_linux_bo *bo);

    /**
     * Write data into an object.
     *
     * This is an optional function, if missing,
     * drm_intel_bo will map/memcpy/unmap.
     */
    int (*bo_subdata) (struct mos_linux_bo *bo, unsigned long offset,
               unsigned long size, const void *data);

    /**
     * Read data from an object
     *
     * This is an optional function, if missing,
     * drm_intel_bo will map/memcpy/unmap.
     */
    int (*bo_get_subdata) (struct mos_linux_bo *bo, unsigned long offset,
                   unsigned long size, void *data);

    /**
     * Waits for rendering to an object by the GPU to have completed.
     *
     * This is not required for any access to the BO by bo_map,
     * bo_subdata, etc.  It is merely a way for the driver to implement
     * glFinish.
     */
    void (*bo_wait_rendering) (struct mos_linux_bo *bo);

    /**
     * Tears down the buffer manager instance.
     */
    void (*destroy) (struct mos_bufmgr *bufmgr);

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
    void (*bo_use_48b_address_range) (struct mos_linux_bo *bo, uint32_t enable);

#ifdef ANDROID
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
    int (*bo_pad_to_size) (struct mos_linux_bo *bo, uint64_t pad_to_size);
#endif

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
                  uint32_t read_domains, uint32_t write_domain);
    int (*bo_emit_reloc2) (struct mos_linux_bo *bo, uint32_t offset,
                  struct mos_linux_bo *target_bo, uint32_t target_offset,
                  uint32_t read_domains, uint32_t write_domain,
                  uint64_t presumed_offset);
    int (*bo_emit_reloc_fence)(struct mos_linux_bo *bo, uint32_t offset,
                   struct mos_linux_bo *target_bo,
                   uint32_t target_offset,
                   uint32_t read_domains,
                   uint32_t write_domain);

    /** Executes the command buffer pointed to by bo. */
#ifdef ANDROID
    int (*bo_exec) (struct mos_linux_bo *bo, int used,
            drm_clip_rect_t *cliprects, int num_cliprects,
            int DR4, int fence_in, int *fence_out);
#else
    int (*bo_exec) (struct mos_linux_bo *bo, int used,
            drm_clip_rect_t *cliprects, int num_cliprects,
            int DR4);
#endif
    /** Executes the command buffer pointed to by bo on the selected
     * ring buffer
     */
#ifdef ANDROID
    int (*bo_mrb_exec) (struct mos_linux_bo *bo, int used,
                drm_clip_rect_t *cliprects, int num_cliprects,
                int DR4, unsigned flags,
                int fence_in, int *fence_out);
#else
    int (*bo_mrb_exec) (struct mos_linux_bo *bo, int used,
                drm_clip_rect_t *cliprects, int num_cliprects,
                int DR4, unsigned flags);
#endif

    /**
     * Pin a buffer to the aperture and fix the offset until unpinned
     *
     * \param buf Buffer to pin
     * \param alignment Required alignment for aperture, in bytes
     */
    int (*bo_pin) (struct mos_linux_bo *bo, uint32_t alignment);

    /**
     * Unpin a buffer from the aperture, allowing it to be removed
     *
     * \param buf Buffer to unpin
     */
    int (*bo_unpin) (struct mos_linux_bo *bo);

    /**
     * Ask that the buffer be placed in tiling mode
     *
     * \param buf Buffer to set tiling mode for
     * \param tiling_mode desired, and returned tiling mode
     */
    int (*bo_set_tiling) (struct mos_linux_bo *bo, uint32_t * tiling_mode,
                  uint32_t stride);

    /**
     * Get the current tiling (and resulting swizzling) mode for the bo.
     *
     * \param buf Buffer to get tiling mode for
     * \param tiling_mode returned tiling mode
     * \param swizzle_mode returned swizzling mode
     */
    int (*bo_get_tiling) (struct mos_linux_bo *bo, uint32_t * tiling_mode,
                  uint32_t * swizzle_mode);

    /**
     * Set the offset at which this buffer will be softpinned
     * \param bo Buffer to set the softpin offset for
     * \param offset Softpin offset
     */
    int (*bo_set_softpin_offset) (struct mos_linux_bo *bo, uint64_t offset);

    /**
     * Create a visible name for a buffer which can be used by other apps
     *
     * \param buf Buffer to create a name for
     * \param name Returned name
     */
    int (*bo_flink) (struct mos_linux_bo *bo, uint32_t * name);

#ifdef ANDROID
    /**
     * Create a dma-buf prime fd for a buffer which can be used by other apps
     *
     * \param buf Buffer to create a prime fd for
     * \param prime_fd Returned prime fd
     */
    int (*bo_prime) (struct mos_linux_bo *bo, uint32_t * prime_fd);
#endif
    /**
     * Returns 1 if mapping the buffer for write could cause the process
     * to block, due to the object being active in the GPU.
     */
    int (*bo_busy) (struct mos_linux_bo *bo);

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
    int (*bo_madvise) (struct mos_linux_bo *bo, int madv);

    int (*check_aperture_space) (struct mos_linux_bo ** bo_array, int count);

    /**
     * Disable buffer reuse for buffers which will be shared in some way,
     * as with scanout buffers. When the buffer reference count goes to
     * zero, it will be freed and not placed in the reuse list.
     *
     * \param bo Buffer to disable reuse for
     */
    int (*bo_disable_reuse) (struct mos_linux_bo *bo);

    /**
     * Query whether a buffer is reusable.
     *
     * \param bo Buffer to query
     */
    int (*bo_is_reusable) (struct mos_linux_bo *bo);

    /**
     *
     * Return the pipe associated with a crtc_id so that vblank
     * synchronization can use the correct data in the request.
     * This is only supported for KMS and gem at this point, when
     * unsupported, this function returns -1 and leaves the decision
     * of what to do in that case to the caller
     *
     * \param bufmgr the associated buffer manager
     * \param crtc_id the crtc identifier
     */
    int (*get_pipe_from_crtc_id) (struct mos_bufmgr *bufmgr, int crtc_id);

    /** Returns true if target_bo is in the relocation tree rooted at bo. */
    int (*bo_references) (struct mos_linux_bo *bo, struct mos_linux_bo *target_bo);

    /**< Enables verbose debugging printouts */
    int debug;
};

#define ALIGN(value, alignment)    ((value + alignment - 1) & ~(alignment - 1))
#define ROUND_UP_TO(x, y)    (((x) + (y) - 1) / (y) * (y))
#define ROUND_UP_TO_MB(x)    ROUND_UP_TO((x), 1024*1024)

#endif /* INTEL_BUFMGR_PRIV_H */
