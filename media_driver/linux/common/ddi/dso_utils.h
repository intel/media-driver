/*
 * Copyright (C) 2012 Intel Corporation. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef DSO_UTILS_H
#define DSO_UTILS_H

#include <stdbool.h>

/** Generic pointer to function. */
typedef void (*dso_generic_func)(void);

/** Library handle (opaque). */
struct dso_handle;

/** Symbol lookup table. */
struct dso_symbol {
    /** Symbol name */
    const char  *name;
    /** Offset into the supplied vtable where symbol is to be loaded. */
    unsigned int offset;
};

/**
 * Opens the named shared library.
 *
 * @param[in]  path  the library name, or NULL to lookup into loaded libraries
 * @return the newly allocated library handle
 */
struct dso_handle *
dso_open(const char *path);

/** Closes and disposed any allocated data. */
void
dso_close(struct dso_handle *h);

/**
 * Loads symbols into the supplied vtable.
 *
 * @param[in]  handle           the DSO handle
 * @param[in]  vtable           the function table to fill in
 * @param[in]  vtable_length    the size (in bytes) of the function table
 * @param[in]  symbols          the NULL terminated array of symbols to lookup
 * @return true on success, false otherwise
 **/
bool
dso_get_symbols(
    struct dso_handle          *h,
    void                       *vtable,
    unsigned int                vtable_length,
    const struct dso_symbol    *symbols
);

#endif /* DSO_UTILS_H */
