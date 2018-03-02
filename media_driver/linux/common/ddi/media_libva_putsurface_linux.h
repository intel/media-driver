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
//! \file      media_libva_putsurface_linux.h 
//! \brief     libva(and its extension) putsurface linux implementaion head file 
//!
#ifndef __MEDIA_LIBVA_PUTSURFACE_LINUX_H__
#define __MEDIA_LIBVA_PUTSURFACE_LINUX_H__

#include <va/va.h>
#include <va/va_backend.h>
#include <va/va_dricommon.h>
#include "mos_defs.h"

#define LIBVA_X11_NAME "libva-x11.so.1"
typedef struct dri_drawable *(*dri_get_drawable_func)(
    VADriverContextP ctx, XID drawable);
typedef union dri_buffer *(*dri_get_rendering_buffer_func)(
    VADriverContextP ctx, struct dri_drawable *d);
typedef void (*dri_swap_buffer_func)(
    VADriverContextP ctx, struct dri_drawable *d);

//!
//! \struct dri_vtable
//! \brief  VA api driver table
//!
struct dri_vtable {
    dri_get_drawable_func               get_drawable;
    dri_get_rendering_buffer_func       get_rendering_buffer;
    dri_swap_buffer_func                swap_buffer;
};

//!
//! \struct va_dri_output
//! \brief  VA driver output
//!
struct va_dri_output {
    struct dso_handle  *handle;
    struct dri_vtable   vtable;
};

//!
//! \struct dso_symbol
//! \brief  Symbol lookup table
//!
struct dso_symbol {
    /** Symbol name */
    const char  *name;
    /** Offset into the supplied vtable where symbol is to be loaded. */
    uint32_t offset;
};

#include <dlfcn.h>
//#include "dso_utils.h"
/** Generic pointer to function. */
typedef void (*dso_generic_func)(void);

//!
//! \struct dso_handle
//! \brief  Dso handle
//!
struct dso_handle {
    void       *handle;
};

//!
//! \brief  Output driver initialization
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//!
//! \return bool
//!     true if call success, else false
//!
bool output_dri_init(VADriverContextP ctx);
//!
//! \brief  Closes and disposed any allocated data
//!
//! \param  [in] h
//!     Dso handle
//!
void dso_close(struct dso_handle *h);

//!
//! \brief  Rectangle initialization
//!
//! \param  [in] Rect
//!     Rectangle
//! \param  [in] destx
//!     Destination X
//! \param  [in] desty
//!     Destination Y
//! \param  [in] destw
//!     Destination W
//! \param  [in] desth
//!     Destination H
//!
void Rect_init(
    RECT            *Rect,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth
);

typedef GC (*TypeXCreateGC)(Display*, Drawable, unsigned long, XGCValues*);
typedef int32_t (*TypeXFreeGC)(Display*, GC);
typedef XImage* (*TypeXCreateImage)(Display*, Visual*, uint32_t, int32_t, int32_t,
                                    char*, uint32_t, uint32_t, int32_t, int32_t);
typedef int32_t (*TypeXDestroyImage)(XImage*);
typedef int32_t (*TypeXPutImage)(Display*, Drawable, GC, XImage*, int32_t, int32_t, int32_t, int32_t,
                             uint32_t, uint32_t);

//! \brief  Ddi codec put surface linux vphal ext
//!
//! \param  ctx
//!     Pointer to VA driver context
//! \param  surface
//!     VA surface ID
//! \param  draw
//!     Drawable of window system
//! \param  srcx
//!     Source X of the region
//! \param  srcy
//!     Source Y of the region
//! \param  srcw
//!     Source W of the region
//! \param  srch
//!     Source H of the region
//! \param  destx
//!     Destination X
//! \param  desty
//!     Destination Y
//! \param  destw
//!     Destination W
//! \param  desth
//!     Destination H
//! \param  cliprects
//!     Client-supplied clip list
//! \param  number_cliprects
//!     Number of clip rects in the clip list
//! \param  flags
//!     De-interlacing flags
//!
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiCodec_PutSurfaceLinuxVphalExt(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void            *draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,        /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
);

//! \brief  Ddi codec put surface linux hardware
//!
//! \param  ctx
//!     Pointer to VA driver context
//! \param  surface
//!     VA surface ID
//! \param  draw
//!     Drawable of window system
//! \param  srcx
//!     Source X of the region
//! \param  srcy
//!     Source Y of the region
//! \param  srcw
//!     Source W of the region
//! \param  srch
//!     Source H of the region
//! \param  destx
//!     Destination X
//! \param  desty
//!     Destination Y
//! \param  destw
//!     Destination W
//! \param  desth
//!     Destination H
//! \param  cliprects
//!     Client-supplied clip list
//! \param  number_cliprects
//!     Number of clip rects in the clip list
//! \param  flags
//!     De-interlacing flags
//!
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiCodec_PutSurfaceLinuxHW(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void*            draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,        /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
);

#ifndef ANDROID
//! \brief  Ddi codec put surface linux software
//!
//! \param  ctx
//!     Pointer to VA driver context
//! \param  surface
//!     VA surface ID
//! \param  draw
//!     Drawable of window system
//! \param  srcx
//!     Source X of the region
//! \param  srcy
//!     Source Y of the region
//! \param  srcw
//!     Source W of the region
//! \param  srch
//!     Source H of the region
//! \param  destx
//!     Destination X
//! \param  desty
//!     Destination Y
//! \param  destw
//!     Destination W
//! \param  desth
//!     Destination H
//! \param  cliprects
//!     Client-supplied clip list
//! \param  number_cliprects
//!     Number of clip rects in the clip list
//! \param  flags
//!     De-interlacing flags
//!
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_PutSurfaceLinuxSW(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void*            draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,        /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
);

//! \brief  Ddi codec put surface dummy
//!
//! \param  ctx
//!     Pointer to VA driver context
//! \param  surface
//!     VA surface ID
//! \param  draw
//!     Drawable of window system
//! \param  srcx
//!     Source X of the region
//! \param  srcy
//!     Source Y of the region
//! \param  srcw
//!     Source W of the region
//! \param  srch
//!     Source H of the region
//! \param  destx
//!     Destination X
//! \param  desty
//!     Destination Y
//! \param  destw
//!     Destination W
//! \param  desth
//!     Destination H
//! \param  cliprects
//!     Client-supplied clip list
//! \param  number_cliprects
//!     Number of clip rects in the clip list
//! \param  flags
//!     De-interlacing flags
//!
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_PutSurfaceDummy(
    VADriverContextP ctx,
    VASurfaceID      surface,
    void            *draw,             /* Drawable of window system */
    int16_t          srcx,
    int16_t          srcy,
    uint16_t         srcw,
    uint16_t         srch,
    int16_t          destx,
    int16_t          desty,
    uint16_t         destw,
    uint16_t         desth,
    VARectangle     *cliprects,        /* client supplied clip list */
    uint32_t         number_cliprects, /* number of clip rects in the clip list */
    uint32_t         flags             /* de-interlacing flags */
);
#endif

#endif

