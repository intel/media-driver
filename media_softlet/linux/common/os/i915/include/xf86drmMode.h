/*
 * \file xf86drmMode.h
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

#ifndef _XF86DRMMODE_H_
#define _XF86DRMMODE_H_

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#include <drm.h>

/*
 * This is the interface for modesetting for drm.
 *
 * In order to use this interface you must include either <stdint.h> or another
 * header defining uint32_t, int32_t and uint16_t.
 *
 * It aims to provide a randr1.2 compatible interface for modesettings in the
 * kernel, the interface is also ment to be used by libraries like EGL.
 *
 * More information can be found in randrproto.txt which can be found here:
 * http://gitweb.freedesktop.org/?p=xorg/proto/randrproto.git
 *
 * There are some major diffrences to be noted. Unlike the randr1.2 proto you
 * need to create the memory object of the framebuffer yourself with the ttm
 * buffer object interface. This object needs to be pinned.
 */

/*
 * If we pickup an old version of drm.h which doesn't include drm_mode.h
 * we should redefine defines. This is so that builds doesn't breaks with
 * new libdrm on old kernels.
 */
#ifndef _DRM_MODE_H

#define DRM_DISPLAY_INFO_LEN    32
#define DRM_CONNECTOR_NAME_LEN  32
#define DRM_DISPLAY_MODE_LEN    32
#define DRM_PROP_NAME_LEN       32

#define DRM_MODE_TYPE_BUILTIN   (1<<0)
#define DRM_MODE_TYPE_CLOCK_C   ((1<<1) | DRM_MODE_TYPE_BUILTIN)
#define DRM_MODE_TYPE_CRTC_C    ((1<<2) | DRM_MODE_TYPE_BUILTIN)
#define DRM_MODE_TYPE_PREFERRED (1<<3)
#define DRM_MODE_TYPE_DEFAULT   (1<<4)
#define DRM_MODE_TYPE_USERDEF   (1<<5)
#define DRM_MODE_TYPE_DRIVER    (1<<6)

/* Video mode flags */
/* bit compatible with the xorg definitions. */
#define DRM_MODE_FLAG_PHSYNC            (1<<0)
#define DRM_MODE_FLAG_NHSYNC            (1<<1)
#define DRM_MODE_FLAG_PVSYNC            (1<<2)
#define DRM_MODE_FLAG_NVSYNC            (1<<3)
#define DRM_MODE_FLAG_INTERLACE            (1<<4)
#define DRM_MODE_FLAG_DBLSCAN            (1<<5)
#define DRM_MODE_FLAG_CSYNC            (1<<6)
#define DRM_MODE_FLAG_PCSYNC            (1<<7)
#define DRM_MODE_FLAG_NCSYNC            (1<<8)
#define DRM_MODE_FLAG_HSKEW            (1<<9) /* hskew provided */
#define DRM_MODE_FLAG_BCAST            (1<<10)
#define DRM_MODE_FLAG_PIXMUX            (1<<11)
#define DRM_MODE_FLAG_DBLCLK            (1<<12)
#define DRM_MODE_FLAG_CLKDIV2            (1<<13)
#define DRM_MODE_FLAG_3D_MASK            (0x1f<<14)
#define  DRM_MODE_FLAG_3D_NONE            (0<<14)
#define  DRM_MODE_FLAG_3D_FRAME_PACKING        (1<<14)
#define  DRM_MODE_FLAG_3D_FIELD_ALTERNATIVE    (2<<14)
#define  DRM_MODE_FLAG_3D_LINE_ALTERNATIVE    (3<<14)
#define  DRM_MODE_FLAG_3D_SIDE_BY_SIDE_FULL    (4<<14)
#define  DRM_MODE_FLAG_3D_L_DEPTH        (5<<14)
#define  DRM_MODE_FLAG_3D_L_DEPTH_GFX_GFX_DEPTH    (6<<14)
#define  DRM_MODE_FLAG_3D_TOP_AND_BOTTOM    (7<<14)
#define  DRM_MODE_FLAG_3D_SIDE_BY_SIDE_HALF    (8<<14)

/* DPMS flags */
/* bit compatible with the xorg definitions. */
#define DRM_MODE_DPMS_ON        0
#define DRM_MODE_DPMS_STANDBY   1
#define DRM_MODE_DPMS_SUSPEND   2
#define DRM_MODE_DPMS_OFF       3

/* Scaling mode options */
#define DRM_MODE_SCALE_NON_GPU          0
#define DRM_MODE_SCALE_FULLSCREEN       1
#define DRM_MODE_SCALE_NO_SCALE         2
#define DRM_MODE_SCALE_ASPECT           3

/* Dithering mode options */
#define DRM_MODE_DITHERING_OFF  0
#define DRM_MODE_DITHERING_ON   1
#define DRM_MODE_DITHERING_AUTO 2

/* Dirty info options */
#define DRM_MODE_DIRTY_OFF      0
#define DRM_MODE_DIRTY_ON       1
#define DRM_MODE_DIRTY_ANNOTATE 2

#define DRM_MODE_ENCODER_NONE   0
#define DRM_MODE_ENCODER_DAC    1
#define DRM_MODE_ENCODER_TMDS   2
#define DRM_MODE_ENCODER_LVDS   3
#define DRM_MODE_ENCODER_TVDAC  4
#define DRM_MODE_ENCODER_VIRTUAL 5
#define DRM_MODE_ENCODER_DSI    6
#define DRM_MODE_ENCODER_DPMST    7

#define DRM_MODE_SUBCONNECTOR_Automatic 0
#define DRM_MODE_SUBCONNECTOR_Unknown   0
#define DRM_MODE_SUBCONNECTOR_DVID      3
#define DRM_MODE_SUBCONNECTOR_DVIA      4
#define DRM_MODE_SUBCONNECTOR_Composite 5
#define DRM_MODE_SUBCONNECTOR_SVIDEO    6
#define DRM_MODE_SUBCONNECTOR_Component 8
#define DRM_MODE_SUBCONNECTOR_SCART     9

#define DRM_MODE_CONNECTOR_Unknown      0
#define DRM_MODE_CONNECTOR_VGA          1
#define DRM_MODE_CONNECTOR_DVII         2
#define DRM_MODE_CONNECTOR_DVID         3
#define DRM_MODE_CONNECTOR_DVIA         4
#define DRM_MODE_CONNECTOR_Composite    5
#define DRM_MODE_CONNECTOR_SVIDEO       6
#define DRM_MODE_CONNECTOR_LVDS         7
#define DRM_MODE_CONNECTOR_Component    8
#define DRM_MODE_CONNECTOR_9PinDIN      9
#define DRM_MODE_CONNECTOR_DisplayPort  10
#define DRM_MODE_CONNECTOR_HDMIA        11
#define DRM_MODE_CONNECTOR_HDMIB        12
#define DRM_MODE_CONNECTOR_TV        13
#define DRM_MODE_CONNECTOR_eDP        14
#define DRM_MODE_CONNECTOR_VIRTUAL      15
#define DRM_MODE_CONNECTOR_DSI          16

#define DRM_MODE_PROP_PENDING   (1<<0)
#define DRM_MODE_PROP_RANGE     (1<<1)
#define DRM_MODE_PROP_IMMUTABLE (1<<2)
#define DRM_MODE_PROP_ENUM      (1<<3) /* enumerated type with text strings */
#define DRM_MODE_PROP_BLOB      (1<<4)
#define DRM_MODE_PROP_BITMASK    (1<<5) /* bitmask of enumerated types */

/* non-extended types: legacy bitmask, one bit per type: */
#define DRM_MODE_PROP_LEGACY_TYPE  ( \
        DRM_MODE_PROP_RANGE | \
        DRM_MODE_PROP_ENUM | \
        DRM_MODE_PROP_BLOB | \
        DRM_MODE_PROP_BITMASK)

/* extended-types: rather than continue to consume a bit per type,
 * grab a chunk of the bits to use as integer type id.
 */
#define DRM_MODE_PROP_EXTENDED_TYPE    0x0000ffc0
#define DRM_MODE_PROP_TYPE(n)        ((n) << 6)
#define DRM_MODE_PROP_OBJECT        DRM_MODE_PROP_TYPE(1)
#define DRM_MODE_PROP_SIGNED_RANGE    DRM_MODE_PROP_TYPE(2)

#define DRM_MODE_CURSOR_BO      (1<<0)
#define DRM_MODE_CURSOR_MOVE    (1<<1)
#define DRM_MODE_CURSOR_FLAGS    0x03

#endif /* _DRM_MODE_H */

/*
 * Feature defines
 *
 * Just because these are defined doesn't mean that the kernel
 * can do that feature, its just for new code vs old libdrm.
 */
#define DRM_MODE_FEATURE_KMS        1
#define DRM_MODE_FEATURE_DIRTYFB    1

typedef struct _drmModeRes {

    int count_fbs;
    uint32_t *fbs;

    int count_crtcs;
    uint32_t *crtcs;

    int count_connectors;
    uint32_t *connectors;

    int count_encoders;
    uint32_t *encoders;

    uint32_t min_width, max_width;
    uint32_t min_height, max_height;
} drmModeRes, *drmModeResPtr;

typedef struct _drmModeModeInfo {
    uint32_t clock;
    uint16_t hdisplay, hsync_start, hsync_end, htotal, hskew;
    uint16_t vdisplay, vsync_start, vsync_end, vtotal, vscan;

    uint32_t vrefresh;

    uint32_t flags;
    uint32_t type;
    char name[DRM_DISPLAY_MODE_LEN];
} drmModeModeInfo, *drmModeModeInfoPtr;

typedef struct _drmModeFB {
    uint32_t fb_id;
    uint32_t width, height;
    uint32_t pitch;
    uint32_t bpp;
    uint32_t depth;
    /* driver specific handle */
    uint32_t handle;
} drmModeFB, *drmModeFBPtr;

typedef struct drm_clip_rect drmModeClip, *drmModeClipPtr;

typedef struct _drmModePropertyBlob {
    uint32_t id;
    uint32_t length;
    void *data;
} drmModePropertyBlobRes, *drmModePropertyBlobPtr;

typedef struct _drmModeProperty {
    uint32_t prop_id;
    uint32_t flags;
    char name[DRM_PROP_NAME_LEN];
    int count_values;
    uint64_t *values; /* store the blob lengths */
    int count_enums;
    struct drm_mode_property_enum *enums;
    int count_blobs;
    uint32_t *blob_ids; /* store the blob IDs */
} drmModePropertyRes, *drmModePropertyPtr;

static __inline int drm_property_type_is(drmModePropertyPtr property,
        uint32_t type)
{
    /* instanceof for props.. handles extended type vs original types: */
    if (property->flags & DRM_MODE_PROP_EXTENDED_TYPE)
        return (property->flags & DRM_MODE_PROP_EXTENDED_TYPE) == type;
    return property->flags & type;
}

typedef struct _drmModeCrtc {
    uint32_t crtc_id;
    uint32_t buffer_id; /**< FB id to connect to 0 = disconnect */

    uint32_t x, y; /**< Position on the framebuffer */
    uint32_t width, height;
    int mode_valid;
    drmModeModeInfo mode;

    int gamma_size; /**< Number of gamma stops */

} drmModeCrtc, *drmModeCrtcPtr;

typedef struct _drmModeEncoder {
    uint32_t encoder_id;
    uint32_t encoder_type;
    uint32_t crtc_id;
    uint32_t possible_crtcs;
    uint32_t possible_clones;
} drmModeEncoder, *drmModeEncoderPtr;

typedef enum {
    DRM_MODE_CONNECTED         = 1,
    DRM_MODE_DISCONNECTED      = 2,
    DRM_MODE_UNKNOWNCONNECTION = 3
} drmModeConnection;

typedef enum {
    DRM_MODE_SUBPIXEL_UNKNOWN        = 1,
    DRM_MODE_SUBPIXEL_HORIZONTAL_RGB = 2,
    DRM_MODE_SUBPIXEL_HORIZONTAL_BGR = 3,
    DRM_MODE_SUBPIXEL_VERTICAL_RGB   = 4,
    DRM_MODE_SUBPIXEL_VERTICAL_BGR   = 5,
    DRM_MODE_SUBPIXEL_NONE           = 6
} drmModeSubPixel;

typedef struct _drmModeConnector {
    uint32_t connector_id;
    uint32_t encoder_id; /**< Encoder currently connected to */
    uint32_t connector_type;
    uint32_t connector_type_id;
    drmModeConnection connection;
    uint32_t mmWidth, mmHeight; /**< HxW in millimeters */
    drmModeSubPixel subpixel;

    int count_modes;
    drmModeModeInfoPtr modes;

    int count_props;
    uint32_t *props; /**< List of property ids */
    uint64_t *prop_values; /**< List of property values */

    int count_encoders;
    uint32_t *encoders; /**< List of encoder ids */
} drmModeConnector, *drmModeConnectorPtr;

#define DRM_PLANE_TYPE_OVERLAY 0
#define DRM_PLANE_TYPE_PRIMARY 1
#define DRM_PLANE_TYPE_CURSOR  2

typedef struct _drmModeObjectProperties {
    uint32_t count_props;
    uint32_t *props;
    uint64_t *prop_values;
} drmModeObjectProperties, *drmModeObjectPropertiesPtr;

typedef struct _drmModePlane {
    uint32_t count_formats;
    uint32_t *formats;
    uint32_t plane_id;

    uint32_t crtc_id;
    uint32_t fb_id;

    uint32_t crtc_x, crtc_y;
    uint32_t x, y;

    uint32_t possible_crtcs;
    uint32_t gamma_size;
} drmModePlane, *drmModePlanePtr;

typedef struct _drmModePlaneRes {
    uint32_t count_planes;
    uint32_t *planes;
} drmModePlaneRes, *drmModePlaneResPtr;

extern void drmModeFreeResources( drmModeResPtr ptr );

/**
 * Retrives all of the resources associated with a card.
 */
extern drmModeResPtr drmModeGetResources(int fd);

extern drmModePropertyPtr drmModeGetProperty(int fd, uint32_t propertyId);
extern void drmModeFreeProperty(drmModePropertyPtr ptr);

extern drmModeObjectPropertiesPtr drmModeObjectGetProperties(int fd,
                            uint32_t object_id,
                            uint32_t object_type);
extern void drmModeFreeObjectProperties(drmModeObjectPropertiesPtr ptr);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
