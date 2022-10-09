/*
 * Copyright © 2009 Intel Corporation
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
 *    Chris Wilson <chris@chris-wilson.co.uk>
 *
 */

/**
 * @file xf86atomic.h
 *
 * Private definitions for atomic operations
 */

#ifndef LIBDRM_ATOMICS_H
#define LIBDRM_ATOMICS_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if 1//HAVE_LIBDRM_ATOMIC_PRIMITIVES

#define HAS_ATOMIC_OPS 1

typedef struct {
    int atomic;
} atomic_t;

# define atomic_read(x) ((x)->atomic)
# define atomic_set(x, val) ((x)->atomic = (val))
# define atomic_inc(x) ((void) __sync_fetch_and_add (&(x)->atomic, 1))
# define atomic_inc_return(x) (__sync_add_and_fetch (&(x)->atomic, 1))
# define atomic_dec_and_test(x) (__sync_add_and_fetch (&(x)->atomic, -1) == 0)
# define atomic_add(x, v) ((void) __sync_add_and_fetch(&(x)->atomic, (v)))
# define atomic_dec(x, v) ((void) __sync_sub_and_fetch(&(x)->atomic, (v)))
# define atomic_cmpxchg(x, oldv, newv) __sync_val_compare_and_swap (&(x)->atomic, oldv, newv)

#endif

static inline int atomic_add_unless(atomic_t *v, int add, int unless)
{
    int c, old;
    c = atomic_read(v);
    while (c != unless && (old = atomic_cmpxchg(v, c, c + add)) != c)
        c = old;
    return c == unless;
}

#endif
