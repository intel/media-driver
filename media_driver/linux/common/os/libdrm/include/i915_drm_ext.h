/*
 * Copyright © 2007-2018 Intel Corporation
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
 */

#ifndef _I915_DRM_EXT_H_
#define _I915_DRM_EXT_H_

#ifndef ANDROID

/** VDBOX balancing */
#define DRM_I915_LOAD_BALANCING_HINT    0x3f
#define DRM_IOCTL_I915_LOAD_BALANCING_HINT        DRM_IOWR(DRM_COMMAND_BASE + DRM_I915_LOAD_BALANCING_HINT, struct drm_i915_ring_load_query)

typedef struct drm_i915_ring_load_info
{
    /** ID of ring & load counter*/
    int ring_id;
    int load_cnt;
} drm_i915_ring_load_info;

typedef struct drm_i915_ring_load_query
{
    /** Number of rings, load counters of which we want to query & ptr to array
      * of load info structures */
    int query_size;
    drm_i915_ring_load_info *load_info;
} drm_i915_ring_load_query;

/** SSEU programming */
#define I915_CONTEXT_PARAM_SSEU     0x6

struct drm_i915_gem_context_param_sseu {
     __u64 flags;
     union {
        struct {
             __u8 slice_mask;
             __u8 subslice_mask;
             __u8 min_eu_per_subslice;
             __u8 max_eu_per_subslice;
        } packed;
        __u64 value;
    };
};

#endif

#endif /* _I915_DRM_EXT_H_ */
