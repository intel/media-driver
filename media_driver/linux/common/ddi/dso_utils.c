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

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include "dso_utils.h"

struct dso_handle {
    void       *handle;
};

/* Opens the named shared library */
struct dso_handle *
dso_open(const char *path)
{
    struct dso_handle *h;

    h = (dso_handle *)calloc(1, sizeof(*h));
    if (!h)
        return NULL;

    if (path) {
        h->handle = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
        if (!h->handle)
            goto error;
    } else
        h->handle = RTLD_DEFAULT;
    return h;

error:
    dso_close(h);
    return NULL;
}

/* Closes and disposed any allocated data */
void
dso_close(struct dso_handle *h)
{
    if (!h)
        return;

    if (h->handle) {
        if (h->handle != RTLD_DEFAULT)
            dlclose(h->handle);
        h->handle = NULL;
    }
    free(h);
}

/* Load symbol into the supplied location */
static bool
get_symbol(struct dso_handle *h, void *func_vptr, const char *name)
{
    dso_generic_func func;
    dso_generic_func * const func_ptr = (dso_generic_func *) func_vptr;
    const char *error;

    dlerror();
    func = (dso_generic_func)dlsym(h->handle, name);
    error = dlerror();
    if (error) {
        fprintf(stderr, "error: failed to resolve %s(): %s\n", name, error);
        return false;
    }
    *func_ptr = func;
    return true;
}

/* Loads symbols into the supplied vtable */
bool
dso_get_symbols(
    struct dso_handle          *h,
    void                       *vtable,
    unsigned int                vtable_length,
    const struct dso_symbol    *symbols
)
{
    const struct dso_symbol *s;

    for (s = symbols; s->name != NULL; s++) {
        if (s->offset + sizeof(dso_generic_func) > vtable_length)
            return false;
        if (!get_symbol(h, ((char *)vtable) + s->offset, s->name))
            return false;
    }
    return true;
}
