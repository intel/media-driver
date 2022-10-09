/*
 * \file xf86drmMode.c
 * Header for DRM modesetting interface.
 *
 * \author Jakob Bornecrantz <wallbraker@gmail.com>
 *
 * \par Acknowledgements:
 * Feb 2007, Dave Airlie <airlied@linux.ie>
 */

/*
 * Copyright (c) 2007-2008 Tungsten Graphics, Inc., Cedar Park, Texas.
 * Copyright (c) 2007-2008 Dave Airlie <airlied@linux.ie>
 * Copyright (c) 2007-2008 Jakob Bornecrantz <wallbraker@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 */

#include <stdint.h>
#include <sys/ioctl.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xf86drmMode.h"
#include "xf86drm.h"
#include <drm.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#include <memcheck.h>
#define VG(x) x
#else
#define VG(x)
#endif

#define memclear(s) memset(&s, 0, sizeof(s))

#define U642VOID(x) ((void *)(unsigned long)(x))
#define VOID2U64(x) ((uint64_t)(unsigned long)(x))

/*
 * Util functions
 */

static void* drmAllocCpy(char *array, int count, int entry_size)
{
    char *r;
    int i;

    if (!count || !array || !entry_size)
        return 0;

    if (!(r = (char *)drmMalloc(count*entry_size)))
        return 0;

    for (i = 0; i < count; i++)
        memcpy(r+(entry_size*i), array+(entry_size*i), entry_size);

    return r;
}

void drmModeFreeResources(drmModeResPtr ptr)
{
    if (!ptr)
        return;

    drmFree(ptr->fbs);
    drmFree(ptr->crtcs);
    drmFree(ptr->connectors);
    drmFree(ptr->encoders);
    drmFree(ptr);

}

/*
 * ModeSetting functions.
 */

drmModeResPtr drmModeGetResources(int fd)
{
    struct drm_mode_card_res res, counts;
    drmModeResPtr r = 0;

retry:
    memclear(res);
    if (drmIoctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &res))
        return 0;

    counts = res;

    if (res.count_fbs) {
        res.fb_id_ptr = VOID2U64(drmMalloc(res.count_fbs*sizeof(uint32_t)));
        if (!res.fb_id_ptr)
            goto err_allocs;
    }
    if (res.count_crtcs) {
        res.crtc_id_ptr = VOID2U64(drmMalloc(res.count_crtcs*sizeof(uint32_t)));
        if (!res.crtc_id_ptr)
            goto err_allocs;
    }
    if (res.count_connectors) {
        res.connector_id_ptr = VOID2U64(drmMalloc(res.count_connectors*sizeof(uint32_t)));
        if (!res.connector_id_ptr)
            goto err_allocs;
    }
    if (res.count_encoders) {
        res.encoder_id_ptr = VOID2U64(drmMalloc(res.count_encoders*sizeof(uint32_t)));
        if (!res.encoder_id_ptr)
            goto err_allocs;
    }

    if (drmIoctl(fd, DRM_IOCTL_MODE_GETRESOURCES, &res))
        goto err_allocs;

    /* The number of available connectors and etc may have changed with a
     * hotplug event in between the ioctls, in which case the field is
     * silently ignored by the kernel.
     */
    if (counts.count_fbs < res.count_fbs ||
        counts.count_crtcs < res.count_crtcs ||
        counts.count_connectors < res.count_connectors ||
        counts.count_encoders < res.count_encoders)
    {
        drmFree(U642VOID(res.fb_id_ptr));
        drmFree(U642VOID(res.crtc_id_ptr));
        drmFree(U642VOID(res.connector_id_ptr));
        drmFree(U642VOID(res.encoder_id_ptr));

        goto retry;
    }

    /*
     * return
     */
    if (!(r = (drmModeResPtr)drmMalloc(sizeof(*r))))
        goto err_allocs;

    r->min_width     = res.min_width;
    r->max_width     = res.max_width;
    r->min_height    = res.min_height;
    r->max_height    = res.max_height;
    r->count_fbs     = res.count_fbs;
    r->count_crtcs   = res.count_crtcs;
    r->count_connectors = res.count_connectors;
    r->count_encoders = res.count_encoders;

    r->fbs        = (uint32_t*)drmAllocCpy((char *)U642VOID(res.fb_id_ptr), res.count_fbs, sizeof(uint32_t));
    r->crtcs      = (uint32_t*)drmAllocCpy((char *)U642VOID(res.crtc_id_ptr), res.count_crtcs, sizeof(uint32_t));
    r->connectors = (uint32_t*)drmAllocCpy((char *)U642VOID(res.connector_id_ptr), res.count_connectors, sizeof(uint32_t));
    r->encoders   = (uint32_t*)drmAllocCpy((char *)U642VOID(res.encoder_id_ptr), res.count_encoders, sizeof(uint32_t));
    if ((res.count_fbs && !r->fbs) ||
        (res.count_crtcs && !r->crtcs) ||
        (res.count_connectors && !r->connectors) ||
        (res.count_encoders && !r->encoders))
    {
        drmFree(r->fbs);
        drmFree(r->crtcs);
        drmFree(r->connectors);
        drmFree(r->encoders);
        drmFree(r);
        r = 0;
    }

err_allocs:
    drmFree(U642VOID(res.fb_id_ptr));
    drmFree(U642VOID(res.crtc_id_ptr));
    drmFree(U642VOID(res.connector_id_ptr));
    drmFree(U642VOID(res.encoder_id_ptr));

    return r;
}

drmModePropertyPtr drmModeGetProperty(int fd, uint32_t property_id)
{
    struct drm_mode_get_property prop;
    drmModePropertyPtr r;

    memclear(prop);
    prop.prop_id = property_id;

    if (drmIoctl(fd, DRM_IOCTL_MODE_GETPROPERTY, &prop))
        return 0;

    if (prop.count_values)
        prop.values_ptr = VOID2U64(drmMalloc(prop.count_values * sizeof(uint64_t)));

    if (prop.count_enum_blobs && (prop.flags & (DRM_MODE_PROP_ENUM | DRM_MODE_PROP_BITMASK)))
        prop.enum_blob_ptr = VOID2U64(drmMalloc(prop.count_enum_blobs * sizeof(struct drm_mode_property_enum)));

    if (prop.count_enum_blobs && (prop.flags & DRM_MODE_PROP_BLOB)) {
        prop.values_ptr = VOID2U64(drmMalloc(prop.count_enum_blobs * sizeof(uint32_t)));
        prop.enum_blob_ptr = VOID2U64(drmMalloc(prop.count_enum_blobs * sizeof(uint32_t)));
    }

    if (drmIoctl(fd, DRM_IOCTL_MODE_GETPROPERTY, &prop)) {
        r = nullptr;
        goto err_allocs;
    }

    if (!(r = (drmModePropertyPtr)drmMalloc(sizeof(*r))))
        return nullptr;

    r->prop_id = prop.prop_id;
    r->count_values = prop.count_values;

    r->flags = prop.flags;
    if (prop.count_values)
        r->values = (uint64_t *)drmAllocCpy((char *)U642VOID(prop.values_ptr), prop.count_values, sizeof(uint64_t));
    if (prop.flags & (DRM_MODE_PROP_ENUM | DRM_MODE_PROP_BITMASK)) {
        r->count_enums = prop.count_enum_blobs;
        r->enums = (struct drm_mode_property_enum*)drmAllocCpy((char*)U642VOID(prop.enum_blob_ptr), prop.count_enum_blobs, sizeof(struct drm_mode_property_enum));
    } else if (prop.flags & DRM_MODE_PROP_BLOB) {
        r->values = (uint64_t *)drmAllocCpy((char *)U642VOID(prop.values_ptr), prop.count_enum_blobs, sizeof(uint32_t));
        r->blob_ids = (uint32_t *)drmAllocCpy((char *)U642VOID(prop.enum_blob_ptr), prop.count_enum_blobs, sizeof(uint32_t));
        r->count_blobs = prop.count_enum_blobs;
    }
    strncpy(r->name, prop.name, DRM_PROP_NAME_LEN);
    r->name[DRM_PROP_NAME_LEN-1] = 0;

err_allocs:
    drmFree(U642VOID(prop.values_ptr));
    drmFree(U642VOID(prop.enum_blob_ptr));

    return r;
}

void drmModeFreeProperty(drmModePropertyPtr ptr)
{
    if (!ptr)
        return;

    drmFree(ptr->values);
    drmFree(ptr->enums);
    drmFree(ptr);
}

drmModeObjectPropertiesPtr drmModeObjectGetProperties(int fd,
                              uint32_t object_id,
                              uint32_t object_type)
{
    struct drm_mode_obj_get_properties properties;
    drmModeObjectPropertiesPtr ret = nullptr;
    uint32_t count;

retry:
    memclear(properties);
    properties.obj_id = object_id;
    properties.obj_type = object_type;

    if (drmIoctl(fd, DRM_IOCTL_MODE_OBJ_GETPROPERTIES, &properties))
        return 0;

    count = properties.count_props;

    if (count) {
        properties.props_ptr = VOID2U64(drmMalloc(count *
                              sizeof(uint32_t)));
        if (!properties.props_ptr)
            goto err_allocs;
        properties.prop_values_ptr = VOID2U64(drmMalloc(count *
                              sizeof(uint64_t)));
        if (!properties.prop_values_ptr)
            goto err_allocs;
    }

    if (drmIoctl(fd, DRM_IOCTL_MODE_OBJ_GETPROPERTIES, &properties))
        goto err_allocs;

    if (count < properties.count_props) {
        drmFree(U642VOID(properties.props_ptr));
        drmFree(U642VOID(properties.prop_values_ptr));
        goto retry;
    }
    count = properties.count_props;

    ret = (drmModeObjectPropertiesPtr)drmMalloc(sizeof(*ret));
    if (!ret)
        goto err_allocs;

    ret->count_props = count;
    ret->props = (uint32_t *)drmAllocCpy((char *)U642VOID(properties.props_ptr),
                 count, sizeof(uint32_t));
    ret->prop_values = (uint64_t *)drmAllocCpy((char *)U642VOID(properties.prop_values_ptr),
                       count, sizeof(uint64_t));
    if (ret->count_props && (!ret->props || !ret->prop_values)) {
        drmFree(ret->props);
        drmFree(ret->prop_values);
        drmFree(ret);
        ret = nullptr;
    }

err_allocs:
    drmFree(U642VOID(properties.props_ptr));
    drmFree(U642VOID(properties.prop_values_ptr));
    return ret;
}

void drmModeFreeObjectProperties(drmModeObjectPropertiesPtr ptr)
{
    if (!ptr)
        return;
    drmFree(ptr->props);
    drmFree(ptr->prop_values);
    drmFree(ptr);
}
