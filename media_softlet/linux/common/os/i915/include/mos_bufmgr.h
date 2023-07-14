/*
 * Copyright © 2008-2023 Intel Corporation
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
 * Public definitions of Intel i915 specific bufmgr functions.
 */

#ifndef MOS_BUFMGR_H
#define MOS_BUFMGR_H

#include "mos_bufmgr_api.h"
#include "i915_drm.h"

int mos_set_context_param_parallel(struct mos_linux_context *ctx,
                         struct i915_engine_class_instance *ci,
                         unsigned int count);

int mos_set_context_param_load_balance(struct mos_linux_context *ctx,
                         struct i915_engine_class_instance *ci,
                         unsigned int count);
int mos_set_context_param_bond(struct mos_linux_context *ctx,
                        struct i915_engine_class_instance master_ci,
                        struct i915_engine_class_instance *bond_ci,
                        unsigned int bond_count);

int mos_get_context_param_sseu(struct mos_linux_context *ctx,
                struct drm_i915_gem_context_param_sseu *sseu);
int mos_set_context_param_sseu(struct mos_linux_context *ctx,
                struct drm_i915_gem_context_param_sseu sseu);

#endif /* INTEL_BUFMGR_H */
