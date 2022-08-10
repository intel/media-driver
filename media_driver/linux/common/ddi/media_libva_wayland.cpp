/*
* Copyright (c) 2009-2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file      media_libva_wayland.cpp
//! \brief     libva get surface buffer wayland implementation
//!

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <va/va_backend.h>
#include <va/va_backend_wayland.h>
#include <wayland-client.h>
#include <wayland-drm-client-protocol.h>
#include "media_libva_wayland.h"
#include "media_libva.h"
#include "dso_utils.h"
/* We need mesa's libEGL, first try the soname of a glvnd enabled mesa build */
#define LIBEGL_NAME             "libEGL_mesa.so.0"
/* Then fallback to plain libEGL.so.1 (which might not be mesa) */
#define LIBEGL_NAME_FALLBACK    "libEGL.so.1"
#define LIBWAYLAND_CLIENT_NAME  "libwayland-client.so.0"
#ifndef VA_FOURCC_IMC1
#define VA_FOURCC_IMC1 VA_FOURCC('I','M','C','1')
#endif
typedef uint32_t (*wl_display_get_global_func)(struct wl_display *display,
                                               const char *interface, uint32_t version);
typedef struct wl_event_queue *(*wl_display_create_queue_func)(struct wl_display *display);
typedef void (*wl_display_roundtrip_queue_func)(struct wl_display *display,
                                                struct wl_event_queue *queue);
typedef void (*wl_event_queue_destroy_func)(struct wl_event_queue *queue);
typedef void *(*wl_proxy_create_wrapper_func)(struct wl_proxy *proxy);
typedef void(*wl_proxy_wrapper_destroy_func)(void *proxy);
typedef struct wl_proxy *(*wl_proxy_create_func)(struct wl_proxy *factory,
                                                 const struct wl_interface *interface);
typedef void (*wl_proxy_destroy_func)(struct wl_proxy *proxy);
typedef void (*wl_proxy_marshal_func)(struct wl_proxy *p, uint32_t opcode, ...);
typedef int (*wl_proxy_add_listener_func)(struct wl_proxy *proxy,
                                          void (**implementation)(void), void *data);
typedef void (*wl_proxy_set_queue_func)(struct wl_proxy *proxy, struct wl_event_queue *queue);

struct wl_vtable {
    const struct wl_interface        *buffer_interface;
    const struct wl_interface        *drm_interface;
    const struct wl_interface        *registry_interface;
    wl_display_create_queue_func      display_create_queue;
    wl_display_roundtrip_queue_func   display_roundtrip_queue;
    wl_event_queue_destroy_func       event_queue_destroy;
    wl_proxy_create_wrapper_func      proxy_create_wrapper;
    wl_proxy_wrapper_destroy_func     proxy_wrapper_destroy;
    wl_proxy_create_func              proxy_create;
    wl_proxy_destroy_func             proxy_destroy;
    wl_proxy_marshal_func             proxy_marshal;
    wl_proxy_add_listener_func        proxy_add_listener;
    wl_proxy_set_queue_func           proxy_set_queue;
};

struct va_wl_output {
    struct dso_handle     *libegl_handle;
    struct dso_handle     *libwl_client_handle;
    struct wl_vtable       vtable;
    struct wl_event_queue *queue;
    struct wl_drm         *wl_drm;
    uint32_t               wl_drm_name;
    struct wl_registry    *wl_registry;
};

/* These function are copied and adapted from the version inside
 * wayland-client-protocol.h
 */
static void *
registry_bind(
    struct wl_vtable          *wl_vtable,
    struct wl_registry        *wl_registry,
    uint32_t                   name,
    const struct wl_interface *interface,
    uint32_t                   version
)
{
    struct wl_proxy *id;

    id = wl_vtable->proxy_create((struct wl_proxy *) wl_registry,
                                 interface);
    if (!id)
        return NULL;

    wl_vtable->proxy_marshal((struct wl_proxy *) wl_registry,
                             WL_REGISTRY_BIND, name, interface->name,
                             version, id);

    return (void *) id;
}

static struct wl_registry *
display_get_registry(
    struct wl_vtable  *wl_vtable,
    struct wl_display *wl_display
)
{
    struct wl_proxy *callback;

    callback = wl_vtable->proxy_create((struct wl_proxy *) wl_display,
                                       wl_vtable->registry_interface);
    if (!callback)
        return NULL;

    wl_vtable->proxy_marshal((struct wl_proxy *) wl_display,
                             WL_DISPLAY_GET_REGISTRY, callback);

    return (struct wl_registry *) callback;
}

static int
registry_add_listener(
    struct wl_vtable                  *wl_vtable,
    struct wl_registry                *wl_registry,
    const struct wl_registry_listener *listener,
    void                              *data
)
{
    return wl_vtable->proxy_add_listener((struct wl_proxy *) wl_registry,
                                         (void (**)(void)) listener, data);
}

static void
registry_handle_global(
    void               *data,
    struct wl_registry *registry,
    uint32_t            name,
    const char         *interface,
    uint32_t            version
)
{
    VADriverContextP ctx = (VADriverContextP)data;
    DDI_MEDIA_CONTEXT *mediaCtx = DdiMedia_GetMediaContext(ctx);
    struct va_wl_output * const wl_output = mediaCtx->wl_output;
    struct wl_vtable * const wl_vtable = &wl_output->vtable;

    if (strcmp(interface, "wl_drm") == 0) {
        wl_output->wl_drm_name = name;
        wl_output->wl_drm = (wl_drm*)registry_bind(wl_vtable, wl_output->wl_registry,
                                          name, wl_vtable->drm_interface,
                                          (version < 2) ? version : 2);
    }
}

static void
registry_handle_global_remove(
    void               *data,
    struct wl_registry *registry,
    uint32_t            name
)
{
    VADriverContextP ctx = (VADriverContextP)data;
    DDI_MEDIA_CONTEXT *mediaCtx = DdiMedia_GetMediaContext(ctx);
    struct va_wl_output * const wl_output = mediaCtx->wl_output;

    if (wl_output->wl_drm && name == wl_output->wl_drm_name) {
        wl_output->vtable.proxy_destroy((struct wl_proxy *)wl_output->wl_drm);
        wl_output->wl_drm = NULL;
    }
}

static const struct wl_registry_listener registry_listener = {
    registry_handle_global,
    registry_handle_global_remove
};

/* Ensure wl_drm instance is created */
static bool
ensure_wl_output(VADriverContextP ctx)
{
    DDI_MEDIA_CONTEXT *mediaCtx = DdiMedia_GetMediaContext(ctx);
    struct va_wl_output * const wl_output = mediaCtx->wl_output;
    struct wl_vtable * const wl_vtable = &wl_output->vtable;
    struct wl_display *display_wrapper;

    if (wl_output->wl_drm)
        return true;

    if (wl_output->queue) {
        wl_output->vtable.event_queue_destroy(wl_output->queue);
        wl_output->queue = NULL;
    }
    wl_output->queue = (wl_event_queue*)wl_vtable->display_create_queue((wl_display*)ctx->native_dpy);
    if (!wl_output->queue)
        return false;

    display_wrapper = (wl_display*)wl_vtable->proxy_create_wrapper((wl_proxy*)ctx->native_dpy);
    if (!display_wrapper)
        return false;
    wl_vtable->proxy_set_queue((struct wl_proxy *) display_wrapper, wl_output->queue);

    if (wl_output->wl_registry) {
        wl_output->vtable.proxy_destroy((struct wl_proxy *)wl_output->wl_registry);
        wl_output->wl_registry = NULL;
    }
    wl_output->wl_registry = display_get_registry(wl_vtable, display_wrapper);
    wl_vtable->proxy_wrapper_destroy(display_wrapper);
    registry_add_listener(wl_vtable, wl_output->wl_registry,
                          &registry_listener, ctx);
    wl_vtable->display_roundtrip_queue((wl_display*)ctx->native_dpy, wl_output->queue);
    if (!wl_output->wl_drm)
        return false;
    return true;
}

/* Create planar/prime YUV buffer
 * Create a prime buffer if fd is not -1, otherwise a
 * planar buffer
 */
static struct wl_buffer *
create_prime_or_planar_buffer(
    struct va_wl_output *wl_output,
    uint32_t             name,
    int                  fd,
    int32_t              width,
    int32_t              height,
    uint32_t             format,
    int32_t              offsets[3],
    int32_t              pitches[3]
)
{
    struct wl_vtable * const wl_vtable = &wl_output->vtable;
    struct wl_proxy *id;

    id = wl_vtable->proxy_create(
             (struct wl_proxy *)wl_output->wl_drm,
             wl_vtable->buffer_interface
         );
    if (!id)
        return NULL;

    wl_vtable->proxy_marshal(
        (struct wl_proxy *)wl_output->wl_drm,
        (fd != -1) ? WL_DRM_CREATE_PRIME_BUFFER : WL_DRM_CREATE_PLANAR_BUFFER,
        id,
        (fd != -1) ? fd : name,
        width, height, format,
        offsets[0], pitches[0],
        offsets[1], pitches[1],
        offsets[2], pitches[2]
    );
    return (struct wl_buffer *)id;
}
extern int32_t DdiMedia_MediaFormatToOsFormat(DDI_MEDIA_FORMAT format);
/* Hook to return Wayland buffer associated with the VA surface */
static VAStatus
va_GetSurfaceBufferWl(
    struct VADriverContext *ctx,
    VASurfaceID             surface,
    unsigned int            flags,
    struct wl_buffer      **out_buffer
)
{
    struct VADriverVTableWayland * const vtable = ctx->vtable_wayland;
    DDI_MEDIA_CONTEXT *mediaCtx = DdiMedia_GetMediaContext(ctx);
    DDI_MEDIA_SURFACE* mediaSurface = DdiMedia_GetSurfaceFromVASurfaceID(mediaCtx, surface);
    struct wl_buffer *buffer;
    uint32_t  drm_format;
    int offsets[3], pitches[3];
    int fd = -1;

    if (!mediaSurface)
        return VA_STATUS_ERROR_INVALID_SURFACE;

    if (flags != VA_FRAME_PICTURE)
        return VA_STATUS_ERROR_FLAG_NOT_SUPPORTED;

    if (!out_buffer)
        return VA_STATUS_ERROR_INVALID_PARAMETER;

    if (!ensure_wl_output(ctx))
        return VA_STATUS_ERROR_INVALID_DISPLAY;

    if (!vtable->has_prime_sharing || (mos_bo_gem_export_to_prime(mediaSurface->bo, &fd) != 0)) {
        fd = -1;

        if (mos_bo_flink(mediaSurface->bo, &mediaSurface->name) != 0)
            return VA_STATUS_ERROR_INVALID_SURFACE;
    }
    offsets[0] = 0;
    int32_t fourcc = DdiMedia_MediaFormatToOsFormat(mediaSurface->format);
    switch (fourcc) {
    case VA_FOURCC_NV12:
        drm_format = WL_DRM_FORMAT_NV12;
        pitches[0] = mediaSurface->iPitch;
        offsets[1] = mediaSurface->iPitch * mediaSurface->iHeight;
        pitches[1] = mediaSurface->iPitch;
        offsets[2] = 0;
        pitches[2] = 0;
        break;
    case VA_FOURCC_YV12:
        drm_format = WL_DRM_FORMAT_YUV420;
        pitches[0]               = mediaSurface->iPitch;
        pitches[1]               =
        pitches[2]               = mediaSurface->iPitch / 2;
        offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        offsets[2]               = mediaSurface->iPitch * mediaSurface->iHeight * 5 / 4;
        break;
    case VA_FOURCC_I420:
        drm_format = WL_DRM_FORMAT_YUV420;
        pitches[0]               = mediaSurface->iPitch;
        pitches[1]               =
        pitches[2]               = mediaSurface->iPitch / 2;
        offsets[1]               = mediaSurface->iPitch * mediaSurface->iHeight * 5 / 4;
        offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch;
        break;
    case VA_FOURCC_IMC1:
    case VA_FOURCC_IMC3:
        drm_format = WL_DRM_FORMAT_YUV420;
        pitches[0]               =
        pitches[1]               =
        pitches[2]               = mediaSurface->iPitch;
        offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch * 3 / 2;
        break;
    case VA_FOURCC_422H:
        drm_format = WL_DRM_FORMAT_YUV422;
        pitches[0]               =
        pitches[1]               =
        pitches[2]               = mediaSurface->iPitch;
        offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch * 2;
        break;
    case VA_FOURCC_422V:
        drm_format = WL_DRM_FORMAT_YUV422;
        pitches[0]               =
        pitches[1]               =
        pitches[2]               = mediaSurface->iPitch;
        offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch * 3 / 2;
        break;
    case VA_FOURCC_411P:
        drm_format = WL_DRM_FORMAT_YUV411;
        pitches[0]               =
        pitches[1]               =
        pitches[2]               = mediaSurface->iPitch;
        offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch * 2;
        break;
    case VA_FOURCC_444P:
        drm_format = WL_DRM_FORMAT_YUV444;
        pitches[0]               =
        pitches[1]               =
        pitches[2]               = mediaSurface->iPitch;
        offsets[1]               = mediaSurface->iHeight * mediaSurface->iPitch;
        offsets[2]               = mediaSurface->iHeight * mediaSurface->iPitch * 2;
        break;
    default:
        return VA_STATUS_ERROR_INVALID_IMAGE_FORMAT;
    }

    buffer = create_prime_or_planar_buffer(
                 mediaCtx->wl_output,
                 mediaSurface->name,
                 fd,
                 mediaSurface->iWidth,
                 mediaSurface->iRealHeight,
                 drm_format,
                 offsets,
                 pitches
             );

    if (fd != -1)
        close(fd);

    if (!buffer)
        return VA_STATUS_ERROR_ALLOCATION_FAILED;

    *out_buffer = buffer;
    return VA_STATUS_SUCCESS;
}

/* Hook to return Wayland buffer associated with the VA image */
static VAStatus
va_GetImageBufferWl(
    struct VADriverContext *ctx,
    VAImageID               image,
    unsigned int            flags,
    struct wl_buffer      **out_buffer
)
{
    return VA_STATUS_ERROR_UNIMPLEMENTED;
}

bool
ensure_driver_vtable(VADriverContextP ctx)
{
    struct VADriverVTableWayland * const vtable = ctx->vtable_wayland;

    if (!vtable)
        return false;

    vtable->vaGetSurfaceBufferWl = va_GetSurfaceBufferWl;
    vtable->vaGetImageBufferWl   = va_GetImageBufferWl;
    return true;
}

bool
DdiMedia_wayland_init(VADriverContextP ctx)
{
    DDI_MEDIA_CONTEXT *mediaCtx = DdiMedia_GetMediaContext(ctx);
    struct dso_handle *dso_handle;
    struct wl_vtable *wl_vtable;

    static const struct dso_symbol libegl_symbols[] = {
        {
            "wl_drm_interface",
            offsetof(struct wl_vtable, drm_interface)
        },
        { NULL, }
    };

    static const struct dso_symbol libwl_client_symbols[] = {
        {
            "wl_buffer_interface",
            offsetof(struct wl_vtable, buffer_interface)
        },
        {
            "wl_registry_interface",
            offsetof(struct wl_vtable, registry_interface)
        },
        {
            "wl_display_create_queue",
            offsetof(struct wl_vtable, display_create_queue)
        },
        {
            "wl_display_roundtrip_queue",
            offsetof(struct wl_vtable, display_roundtrip_queue)
        },
        {
            "wl_event_queue_destroy",
            offsetof(struct wl_vtable, event_queue_destroy)
        },
        {
            "wl_proxy_create_wrapper",
            offsetof(struct wl_vtable, proxy_create_wrapper)
        },
        {
            "wl_proxy_wrapper_destroy",
            offsetof(struct wl_vtable, proxy_wrapper_destroy)
        },
        {
            "wl_proxy_create",
            offsetof(struct wl_vtable, proxy_create)
        },
        {
            "wl_proxy_destroy",
            offsetof(struct wl_vtable, proxy_destroy)
        },
        {
            "wl_proxy_marshal",
            offsetof(struct wl_vtable, proxy_marshal)
        },
        {
            "wl_proxy_add_listener",
            offsetof(struct wl_vtable, proxy_add_listener)
        },
        {
            "wl_proxy_set_queue",
            offsetof(struct wl_vtable, proxy_set_queue)
        },
        { NULL, }
    };

    if (ctx->display_type != VA_DISPLAY_WAYLAND)
        return false;

    mediaCtx->wl_output = (va_wl_output *)calloc(1, sizeof(struct va_wl_output));
    if (!mediaCtx->wl_output)
        goto error;

    mediaCtx->wl_output->libegl_handle = dso_open(LIBEGL_NAME);
    if (!mediaCtx->wl_output->libegl_handle) {
        mediaCtx->wl_output->libegl_handle = dso_open(LIBEGL_NAME_FALLBACK);
        if (!mediaCtx->wl_output->libegl_handle)
            goto error;
    }

    dso_handle = mediaCtx->wl_output->libegl_handle;
    wl_vtable  = &mediaCtx->wl_output->vtable;
    if (!dso_get_symbols(dso_handle, wl_vtable, sizeof(*wl_vtable),
                         libegl_symbols))
        goto error;

    mediaCtx->wl_output->libwl_client_handle = dso_open(LIBWAYLAND_CLIENT_NAME);
    if (!mediaCtx->wl_output->libwl_client_handle)
        goto error;

    dso_handle = mediaCtx->wl_output->libwl_client_handle;
    wl_vtable  = &mediaCtx->wl_output->vtable;
    if (!dso_get_symbols(dso_handle, wl_vtable, sizeof(*wl_vtable),
                         libwl_client_symbols))
        goto error;

    if (!ensure_driver_vtable(ctx))
        goto error;
    return true;

error:
    DdiMedia_wayland_terminate(ctx);
    return false;
}

void
DdiMedia_wayland_terminate(VADriverContextP ctx)
{
    DDI_MEDIA_CONTEXT *mediaCtx = DdiMedia_GetMediaContext(ctx);
    struct va_wl_output *wl_output;

    if (ctx->display_type != VA_DISPLAY_WAYLAND)
        return;

    wl_output = mediaCtx->wl_output;
    if (!wl_output)
        return;

    if (wl_output->wl_drm) {
        wl_output->vtable.proxy_destroy((struct wl_proxy *)wl_output->wl_drm);
        wl_output->wl_drm = NULL;
    }

    if (wl_output->wl_registry) {
        wl_output->vtable.proxy_destroy((struct wl_proxy *)wl_output->wl_registry);
        wl_output->wl_registry = NULL;
    }

    if (wl_output->queue) {
        wl_output->vtable.event_queue_destroy(wl_output->queue);
        wl_output->queue = NULL;
    }

    if (wl_output->libegl_handle) {
        dso_close(wl_output->libegl_handle);
        wl_output->libegl_handle = NULL;
    }

    if (wl_output->libwl_client_handle) {
        dso_close(wl_output->libwl_client_handle);
        wl_output->libwl_client_handle = NULL;
    }
    free(wl_output);
    mediaCtx->wl_output = NULL;
}
