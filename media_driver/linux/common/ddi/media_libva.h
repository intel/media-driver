/*
* Copyright (c) 2009-2021, Intel Corporation
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
//! \file     media_libva.h
//! \brief    libva(and its extension) interface head file
//!

#ifndef __MEDIA_LIBVA_H__
#define __MEDIA_LIBVA_H__

#include <va/va.h>
#include <va/va_backend.h>
#include "va/va_dec_vp8.h"
#include <va/va_enc_h264.h>
#include <va/va_enc_mpeg2.h>
#include <va/va_enc_jpeg.h>
#include <va/va_dec_jpeg.h>
#include <va/va_enc_vp8.h>
#include <va/va_dec_vp9.h>
#include <va/va_enc_hevc.h>
#include <va/va_vpp.h>
#include <va/va_backend_vpp.h>
#if VA_CHECK_VERSION(1,11,0)
#include <va/va_backend_prot.h>
#endif
#ifdef ANDROID
#include <va/va_android.h>
#if VA_MAJOR_VERSION < 1
#include "va_internal_android.h"
#endif
#endif // ANDROID
#include <va/va_dec_hevc.h>
#include "codechal.h"
#include "codechal_decoder.h"
#include "codechal_encoder_base.h"
#include "media_libva_common.h"
#include "ddi_codec_def_specific.h"

#define DDI_CODEC_GEN_MAX_PROFILES                 31   //  the number of va profiles, some profiles in va_private.h
#define DDI_CODEC_GEN_MAX_ENTRYPOINTS              7    // VAEntrypointVLD, VAEntrypointEncSlice, VAEntrypointEncSliceLP, VAEntrypointVideoProc

#define DDI_CODEC_GEN_MAX_IMAGE_FORMATS            2    // NV12 and P010
#define DDI_CODEC_GEN_MAX_SUBPIC_FORMATS           4    // no sub-pic blending support, still set to 4 for further implementation
#if VA_MAJOR_VERSION < 1
#define DDI_MEDIA_GEN_MAX_DISPLAY_ATTRIBUTES       4
#else
#if VA_CHECK_VERSION(1, 15, 0)
#define DDI_MEDIA_GEN_MAX_DISPLAY_ATTRIBUTES       2
#else
#define DDI_MEDIA_GEN_MAX_DISPLAY_ATTRIBUTES       1
#endif
#endif
#define DDI_CODEC_GEN_MAX_ATTRIBS_TYPE             4    //VAConfigAttribRTFormat,    VAConfigAttribRateControl,    VAConfigAttribDecSliceMode,    VAConfigAttribEncPackedHeaders

#define DDI_CODEC_GEN_STR_VENDOR                   "Intel iHD driver for Intel(R) Gen Graphics - " MEDIA_VERSION " (" MEDIA_VERSION_DETAILS ")"

#define DDI_CODEC_GET_VTABLE(ctx)                  (ctx->vtable)
#define DDI_CODEC_GET_VTABLE_VPP(ctx)              (ctx->vtable_vpp)
#if VA_CHECK_VERSION(1,11,0)
#define DDI_CODEC_GET_VTABLE_PROT(ctx)             (ctx->vtable_prot)
#endif
#define DDI_CODEC_GET_VTABLE_TPI(ctx)              (ctx->vtable_tpi)

#define DDI_CODEC_BATCH_BUFFER_SIZE                0x80000

/* Number of supported input color formats */
#define DDI_VP_NUM_INPUT_COLOR_STD    6
/* Number of supported output color formats */
#define DDI_VP_NUM_OUT_COLOR_STD      6

#define DDI_CP_ENCRYPT_TYPES_NUM             5    // CP encryption types number

// Enable unlimited output buffer, delete this build option (remove multiple output buffer) when it is verified
#define ENABLE_ENC_UNLIMITED_OUTPUT

// Max timeout for i915 bo_wait
#define DDI_BO_MAX_TIMEOUT       (~(0x8000000000000000))
// Negative value for infinite timeout for i915 bo_wait
#define DDI_BO_INFINITE_TIMEOUT  (-1)

//!
//! \brief  Get Device FD
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [out] pDevicefd
//!         device fd
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_GetDeviceFD (
    VADriverContextP ctx,
    int32_t         *pDevicefd
);

//!
//! \brief  Load DDI function
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_LoadFuncion (VADriverContextP ctx);

//!
//! \brief  Initialize
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [out] major_version
//!         Major version
//! \param  [out] minor_version
//!         Minor version
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia__Initialize (
    VADriverContextP ctx,
    int32_t         *major_version,     /* out */
    int32_t         *minor_version      /* out */
);

//!
//! \brief  Initialize Media Context
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] devicefd
//!         Devoce fd
//! \param  [out] major_version
//!         Major version
//! \param  [out] minor_version
//!         Minor version
//! \param  [in]  apoDdiEnabled
//!         If apo ddi enabled       
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_InitMediaContext (
    VADriverContextP ctx,
    int32_t          devicefd,
    int32_t          *major_version,     /* out */
    int32_t          *minor_version,     /* out */
    bool             &apoDdiEnabled
);

//!
//! \brief  clean up casp/caps next/complist/hwinfo
//!
//! \param  [in] mediaCtx
//!         Pointer to media context
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_CleanUp(PDDI_MEDIA_CONTEXT mediaCtx);

//!
//! \brief  clean up all library internal resources
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_Terminate(VADriverContextP ctx);

//!
//! \brief  Query supported entrypoints for a given profile
//! \details    The caller must provide an "entrypoint_list" array that can hold at
//!             least vaMaxNumEntrypoints() entries. The actual number of entrypoints
//!             returned in "entrypoint_list" is returned in "num_entrypoints".
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] profile
//!         VA profile
//! \param  [out] entrypoint_list
//!         VA entrypoints
//! \param  [out] num_entrypoints
//!         Number of entrypoints
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QueryConfigEntrypoints (
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      *entrypoint_list,
    int32_t           *num_entrypoints
);

//!
//! \brief  Query supported profiles
//! \details    The caller must provide a "profile_list" array that can hold at
//!             least vaMaxNumProfile() entries. The actual number of profiles
//!             returned in "profile_list" is returned in "num_profile".
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [out] profile_list
//!         VA profiles
//! \param  [out] num_profiles
//!         Number of profiles
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QueryConfigProfiles (
    VADriverContextP  ctx,
    VAProfile         *profile_list,
    int32_t           *num_profiles
);

//!
//! \brief  Query all attributes for a given configuration
//! \details    The profile of the configuration is returned in "profile"
//!             The entrypoint of the configuration is returned in "entrypoint"
//!             The caller must provide an "attrib_list" array that can hold at least
//!             vaMaxNumConfigAttributes() entries. The actual number of attributes
//!             returned in "attrib_list" is returned in "num_attribs"
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] config_id
//!         VA config id
//! \param  [out] profile
//!         VA profile of configuration
//! \param  [out] entrypoint
//!         VA entrypoint of configuration
//! \param  [out] attrib_list
//!         VA attrib list
//! \param  [out] num_attribs
//!         Number of attribs
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QueryConfigAttributes (
    VADriverContextP  ctx,
    VAConfigID        config_id,
    VAProfile         *profile,
    VAEntrypoint      *entrypoint,
    VAConfigAttrib    *attrib_list,
    int32_t           *num_attribs
);

//!
//! \brief  Create a configuration for the encode/decode/vp pipeline
//! \details    it passes in the attribute list that specifies the attributes it cares
//!             about, with the rest taking default values.
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] profile
//!         VA profile of configuration
//! \param  [in] entrypoint
//!         VA entrypoint of configuration
//! \param  [out] attrib_list
//!         VA attrib list
//! \param  [out] num_attribs
//!         Number of attribs
//! \param  [out] config_id
//!         VA config id
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_CreateConfig (
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attrib_list,
    int32_t           num_attribs,
    VAConfigID        *config_id
);

//!
//! \brief  Free resources associated with a given config
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] config_id
//!         VA config id
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_DestroyConfig (
    VADriverContextP  ctx,
    VAConfigID        config_id
);

//!
//! \brief  Get attributes for a given profile/entrypoint pair
//! \details    The caller must provide an "attrib_list" with all attributes to be
//!             retrieved.  Upon return, the attributes in "attrib_list" have been
//!             updated with their value.  Unknown attributes or attributes that are
//!             not supported for the given profile/entrypoint pair will have their
//!             value set to VA_ATTRIB_NOT_SUPPORTED
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] profile
//!         VA profile of configuration
//! \param  [in] entrypoint
//!         VA entrypoint of configuration
//! \param  [out] attrib_list
//!         VA attrib list
//! \param  [in] num_attribs
//!         Number of attribs
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_GetConfigAttributes (
    VADriverContextP  ctx,
    VAProfile         profile,
    VAEntrypoint      entrypoint,
    VAConfigAttrib    *attrib_list,
    int32_t           num_attribs
);

//!
//! \brief  Create surfaces
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] width
//!         Surface width
//! \param  [in] height
//!         Surface height
//! \param  [in] format
//!         Surface format
//! \param  [in] num_surfaces
//!         Number of surfaces
//! \param  [out] surfaces
//!         VA created surfaces
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_CreateSurfaces (
    VADriverContextP  ctx,
    int32_t           width,
    int32_t           height,
    int32_t           format,
    int32_t           num_surfaces,
    VASurfaceID       *surfaces
);

//!
//! \brief  Destroy resources associated with surfaces.
//! \details    Surfaces can only be destroyed after the context associated has been
//!             destroyed
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] surfaces
//!         VA array of surfaces to destroy
//! \param  [in] num_surfaces
//!         Number of surfaces in the array to be destroyed
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_DestroySurfaces (
    VADriverContextP  ctx,
    VASurfaceID       *surfaces,
    int32_t           num_surfaces
);

//!
//! \brief  Create surfaces2
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] format
//!         Surface format
//! \param  [in] width
//!         Surface width
//! \param  [in] height
//!         Surface height
//! \param  [out] surfaces
//!         VA created surfaces
//! \param  [in] num_surfaces
//!         Number of surfaces
//! \param  [out] attrib_list
//!         VA attrib list
//! \param  [in] num_attribs
//!         Number of attribs
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_CreateSurfaces2 (
    VADriverContextP  ctx,
    uint32_t          format,
    uint32_t          width,
    uint32_t          height,
    VASurfaceID       *surfaces,
    uint32_t          num_surfaces,
    VASurfaceAttrib   *attrib_list,
    uint32_t          num_attribs
);

//!
//! \brief  Create context
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] config_id
//!         VA config id
//! \param  [in] picture_width
//!         Picture width
//! \param  [in] picture_height
//!         Picture height
//! \param  [out] flag
//!         Create flag
//! \param  [in] render_targets
//!         VA render traget
//! \param  [in] num_render_targets
//!         Number of render targets
//! \param  [out] context
//!         VA created context
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_CreateContext (
    VADriverContextP  ctx,
    VAConfigID        config_id,
    int32_t           picture_width,
    int32_t           picture_height,
    int32_t           flag,
    VASurfaceID       *render_targets,
    int32_t           num_render_targets,
    VAContextID       *context
);

//!
//! \brief  Destroy context
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] context
//!         VA context to destroy
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_DestroyContext (
    VADriverContextP  ctx,
    VAContextID       context
);

//!
//! \brief  Create buffer
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] context
//!         VA context id
//! \param  [in] type
//!         VA buffer type
//! \param  [in] size
//!         Buffer size
//! \param  [out] num_elements
//!         Number of elements
//! \param  [in] data
//!         Buffer data
//! \param  [out] bufId
//!         VA buffer id
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_CreateBuffer (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferType      type,
    uint32_t          size,
    uint32_t          num_elements,
    void              *data,
    VABufferID        *bufId
);

//!
//! \brief  Convey to the server how many valid elements are in the buffer
//! \details    e.g. if multiple slice parameters are being held in a single buffer,
//!             this will communicate to the server the number of slice parameters
//!             that are valid in the buffer.
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] buf_id
//!         VA buffer id
//! \param  [in] num_elements
//!         Number of elements in buffer
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_BufferSetNumElements (
    VADriverContextP  ctx,
    VABufferID        buf_id,
    uint32_t          num_elements
);

//! \brief  Map buffer
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] buf_id
//!     VA buffer ID
//! \param  [out] pbuf
//!     Pointer to buffer
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_MapBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id,
    void                **pbuf
);

//! \brief  Unmap buffer
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] buf_id
//!     VA buffer ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_UnmapBuffer (
    VADriverContextP    ctx,
    VABufferID          buf_id
);

//!
//! \brief  Destroy buffer
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] buffer_id
//!         VA buffer ID
//!
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_DestroyBuffer (
    VADriverContextP    ctx,
    VABufferID          buffer_id
);

//!
//! \brief  Get ready to decode a picture to a target surface
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] context
//!         VA context id
//! \param  [in] render_target
//!         VA render target surface
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_BeginPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VASurfaceID       render_target
);

//!
//! \brief  Send decode buffers to the server
//! \details    Buffers are automatically destroyed afterwards
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] context
//!         VA buffer id
//! \param  [in] buffer
//!         Pointer to VA buffer id
//! \param  [in] num_buffers
//!         number of buffers
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_RenderPicture (
    VADriverContextP  ctx,
    VAContextID       context,
    VABufferID        *buffers,
    int32_t           num_buffers
);

//!
//! \brief  Make the end of rendering for a picture
//! \details    The server should start processing all pending operations for this
//!             surface. This call is non-blocking. The client can start another
//!             Begin/Render/End sequence on a different render target
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] context
//!         VA buffer id
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_EndPicture (
    VADriverContextP  ctx,
    VAContextID       context
);

//!
//! \brief  Sync surface
//! \details    This function blocks until all pending operations on the render target
//!             have been completed.  Upon return it is safe to use the render target for a
//!             different picture
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] render_target
//!         VA render target surface id
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_SyncSurface (
    VADriverContextP  ctx,
    VASurfaceID       render_target
);

#if VA_CHECK_VERSION(1, 9, 0)
//!
//! \brief  Sync surface
//! \details    This function blocks until all pending operations on the render target
//!             have been completed.  Upon return it is safe to use the render target for a
//!             different picture
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] surface_id
//!         VA render target surface id
//! \param  [in] timeout_ns
//!         time out period
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_SyncSurface2 (
    VADriverContextP  ctx,
    VASurfaceID       surface_id,
    uint64_t          timeout_ns
);

//!
//! \brief  Sync buffer
//! \details    This function blocks until all pending operations on the render target
//!             have been completed.  Upon return it is safe to use the render target for a
//!             different picture
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] buf_id
//!         VA buffer id
//! \param  [in] timeout_ns
//!         time out period
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_SyncBuffer (
    VADriverContextP  ctx,
    VABufferID        buf_id,
    uint64_t          timeout_ns
);
#endif

//!
//! \brief  Query surface status
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] render_target
//!         VA surface ID
//! \param  [out] status
//!         VA surface status
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QuerySurfaceStatus (
    VADriverContextP  ctx,
    VASurfaceID       render_target,
    VASurfaceStatus   *status
);

//!
//! \brief  Report MB error info
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] render_target
//!         VA surface ID
//! \param  [in] error_status
//!         Error status
//! \param  [out] error_info
//!         Information on error
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QuerySurfaceError (
    VADriverContextP  ctx,
    VASurfaceID       render_target,
    VAStatus          error_status,
    void              **error_info /*out*/
);

//!
//! \brief  Query surface attributes for the supplied config
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] config_id
//!         VA config id
//! \param  [out] attrib_list
//!         VA surface attrib
//! \param  [out] num_attribs
//!         Number of attribs
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QuerySurfaceAttributes (
    VADriverContextP  ctx,
    VAConfigID        config_id,
    VASurfaceAttrib   *attrib_list,
    uint32_t          *num_attribs
);

//!
//! \brief  Put surface
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] surface
//!         VA surface ID
//! \param  [in] draw
//!         Drawable of window system
//! \param  [in] srcx
//!         X offset of src image
//! \param  [in] srcy
//!         Y offset of src image
//! \param  [in] srcw
//!         Width offset of src image
//! \param  [in] srch
//!         Height offset of src image
//! \param  [in] destx
//!         X offset of dst image
//! \param  [in] desty
//!         Y offset of dst image
//! \param  [in] destw
//!         Width offset of dst image
//! \param  [in] desth
//!         Height offset of dst image
//! \param  [in] cliprects
//!         Client supplied clip list
//! \param  [in] number_cliprects
//!         Number of clip rects in the clip list
//! \param  [in] flags
//!         de-interlacing flags
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_PutSurface (
    VADriverContextP  ctx,
    VASurfaceID       surface,
    void*             draw,             /* Drawable of window system */
    int16_t           srcx,
    int16_t           srcy,
    uint16_t          srcw,
    uint16_t          srch,
    int16_t           destx,
    int16_t           desty,
    uint16_t          destw,
    uint16_t          desth,
    VARectangle       *cliprects,        /* client supplied clip list */
    uint32_t          number_cliprects, /* number of clip rects in the clip list */
    uint32_t          flags             /* de-interlacing flags */
);

//!
//! \brief  Query supported image formats
//! \details    The caller must provide a "format_list" array that can hold at
//!             least vaMaxNumImageFormats() entries. The actual number of formats
//!             returned in "format_list" is returned in "num_formats"
//!
//! \param  [in] ctx
//!         Driver context
//! \param  [out] format_list
//!         The format of image
//! \param  [out] num_formats
//!         The number of the formats
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QueryImageFormats (
    VADriverContextP  ctx,
    VAImageFormat     *format_list,
    int32_t           *num_formats
);

//!
//! \brief  Create an image
//!
//! \param  [in] ctx
//!     Driver context
//! \param  [in] format
//!     The format of image
//! \param  [in] width
//!     The width of the image
//! \param  [in] height
//!     The height of the image
//! \param  [out] image
//!     The generated image
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_CreateImage (
    VADriverContextP  ctx,
    VAImageFormat     *format,
    int32_t           width,
    int32_t           height,
    VAImage           *image     /* out */
);

//!
//! \brief  Derive image
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] surface
//!         VA surface ID
//! \param  [in] image
//!         VA image
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_DeriveImage (
    VADriverContextP  ctx,
    VASurfaceID       surface,
    VAImage           *image
);

//!
//! \brief  Free allocated surfaceheap elements
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] image
//!         VA image ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_DestroyImage (
    VADriverContextP  ctx,
    VAImageID         image
);

//!
//! \brief  Set image palette
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] image
//!         VA image ID
//! \param  [in] palette
//!         Palette
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED if call success, else fail reason
//!
VAStatus DdiMedia_SetImagePalette (
    VADriverContextP  ctx,
    VAImageID         image,
    unsigned char     *palette
);

//!
//! \brief  Retrive surface data into a VAImage
//! \details    Image must be in a format supported by the implementation
//!
//! \param  [in] ctx
//!         Input driver context
//! \param  [in] surface
//!         Input surface ID of source
//! \param  [in] x
//!         X offset of the wanted region
//! \param  [in] y
//!         Y offset of the wanted region
//! \param  [in] width
//!         Width of the wanted region
//! \param  [in] height
//!         Height of the wanted region
//! \param  [in] image
//!     The image ID of the source image
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_GetImage (
    VADriverContextP  ctx,
    VASurfaceID       surface,
    int32_t           x,     /* coordinates of the upper left source pixel */
    int32_t           y,
    uint32_t          width, /* width and height of the region */
    uint32_t          height,
    VAImageID         image
);

//!
//! \brief  Copy data from a VAImage to a surface
//! \details    Image must be in a format supported by the implementation
//!
//! \param  [in] ctx
//!         Input driver context
//! \param  [in] surface
//!         Surface ID of destination
//! \param  [in] image
//!         The image ID of the destination image
//! \param  [in] src_x
//!         Source x offset of the image region
//! \param  [in] src_y
//!         Source y offset of the image region
//! \param  [in] src_width
//!         Source width offset of the image region
//! \param  [in] src_height
//!         Source height offset of the image region
//! \param  [in] dest_x
//!         Destination x offset of the surface region
//! \param  [in] dest_y
//!         Destination y offset of the surface region
//! \param  [in] dest_width
//!         Destination width offset of the surface region
//! \param  [in] dest_height
//!         Destination height offset of the surface region
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_PutImage (
    VADriverContextP  ctx,
    VASurfaceID       surface,
    VAImageID         image,
    int32_t           src_x,
    int32_t           src_y,
    uint32_t          src_width,
    uint32_t          src_height,
    int32_t           dest_x,
    int32_t           dest_y,
    uint32_t          dest_width,
    uint32_t          dest_height
);

//!
//! \brief  Query subpicture formats
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] format_list
//!         VA image format
//! \param  [in] flags
//!         Flags
//! \param  [in] num_formats
//!         Number of formats
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QuerySubpictureFormats (
    VADriverContextP  ctx,
    VAImageFormat     *format_list,
    uint32_t          *flags,
    uint32_t          *num_formats
);

//!
//! \brief  Create subpicture
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] image
//!         VA image ID
//! \param  [out] subpicture
//!         VA subpicture ID
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
VAStatus DdiMedia_CreateSubpicture (
    VADriverContextP  ctx,
    VAImageID         image,
    VASubpictureID    *subpicture   /* out */
);

//!
//! \brief  Destroy subpicture
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
VAStatus DdiMedia_DestroySubpicture (
    VADriverContextP  ctx,
    VASubpictureID    subpicture
);

//!
//! \brief  Set subpicture image
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//! \param  [in] image
//!         VA image ID
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
VAStatus DdiMedia_SetSubpictureImage (
    VADriverContextP  ctx,
    VASubpictureID    subpicture,
    VAImageID         image
);

//!
//! \brief  Set subpicture chrome key
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//! \param  [in] chromakey_min
//!         Minimum chroma key
//! \param  [in] chromakey_max
//!         Maximum chroma key
//! \param  [in] chromakey_mask
//!         Chromakey mask
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
VAStatus DdiMedia_SetSubpictureChromakey (
    VADriverContextP  ctx,
    VASubpictureID    subpicture,
    uint32_t          chromakey_min,
    uint32_t          chromakey_max,
    uint32_t          chromakey_mask
);

//!
//! \brief  set subpicture global alpha
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//! \param  [in] global_alpha
//!         Global alpha
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
VAStatus DdiMedia_SetSubpictureGlobalAlpha (
    VADriverContextP  ctx,
    VASubpictureID    subpicture,
    float             global_alpha
);

//!
//! \brief  Associate subpicture
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//! \param  [in] target_surfaces
//!         VA surface ID
//! \param  [in] num_surfaces
//!         Number of surfaces
//! \param  [in] src_x
//!         Source x of the region
//! \param  [in] src_y
//!         Source y of the region
//! \param  [in] src_width
//!         Source width of the region
//! \param  [in] src_height
//!         Source height of the region
//! \param  [in] dest_x
//!         Destination x
//! \param  [in] dest_y
//!         Destination y
//! \param  [in] dest_width
//!         Destination width
//! \param  [in] dest_height
//!         Destination height
//! \param  [in] flags
//!         Flags
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
VAStatus DdiMedia_AssociateSubpicture (
    VADriverContextP  ctx,
    VASubpictureID    subpicture,
    VASurfaceID       *target_surfaces,
    int32_t           num_surfaces,
    int16_t           src_x,  /* upper left offset in subpicture */
    int16_t           src_y,
    uint16_t          src_width,
    uint16_t          src_height,
    int16_t           dest_x, /* upper left offset in surface */
    int16_t           dest_y,
    uint16_t          dest_width,
    uint16_t          dest_height,
    /*
     * whether to enable chroma-keying or global-alpha
     * see VA_SUBPICTURE_XXX values
     */
    uint32_t          flags
);

//!
//! \brief  Deassociate subpicture
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] subpicture
//!         VA subpicture ID
//! \param  [in] target_surfaces
//!         VA surface ID
//! \param  [in] num_surfaces
//!         Number of surfaces
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
VAStatus DdiMedia_DeassociateSubpicture (
    VADriverContextP  ctx,
    VASubpictureID    subpicture,
    VASurfaceID       *target_surfaces,
    int32_t           num_surfaces
);

//!
//! \brief  Query display attributes
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] attr_list
//!         VA display attribute
//! \param  [in] num_attributes
//!         Number of attributes
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QueryDisplayAttributes (
    VADriverContextP    ctx,
    VADisplayAttribute  *attr_list,
    int32_t             *num_attributes
);

//!
//! \brief  Get display attributes
//! \details    This function returns the current attribute values in "attr_list".
//!         Only attributes returned with VA_DISPLAY_ATTRIB_GETTABLE set in the "flags" field
//!         from vaQueryDisplayAttributes() can have their values retrieved.
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] attr_list
//!         VA display attribute
//! \param  [in] num_attributes
//!         Number of attributes
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
VAStatus DdiMedia_GetDisplayAttributes (
    VADriverContextP    ctx,
    VADisplayAttribute  *attr_list,
    int32_t             num_attributes
);

//!
//! \brief  Set display attributes
//! \details    Only attributes returned with VA_DISPLAY_ATTRIB_SETTABLE set in the "flags" field
//!         from vaQueryDisplayAttributes() can be set.  If the attribute is not settable or
//!         the value is out of range, the function returns VA_STATUS_ERROR_ATTR_NOT_SUPPORTED
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] attr_list
//!         VA display attribute
//! \param  [in] num_attributes
//!         Number of attributes
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
VAStatus DdiMedia_SetDisplayAttributes (
    VADriverContextP    ctx,
    VADisplayAttribute  *attr_list,
    int32_t             num_attributes
);

//!
//! \brief  Query processing rate
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] config_id
//!         VA configuration ID
//! \param  [in] proc_buf
//!         VA processing rate parameter
//! \param  [out] processing_rate
//!         Processing rate
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QueryProcessingRate (
      VADriverContextP           ctx,
      VAConfigID                 config_id,
      VAProcessingRateParameter  *proc_buf,
      uint32_t                   *processing_rate /* output parameter */
);

#if VA_CHECK_VERSION(1,10,0)
//!
//! \brief  media copy
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] dst_obj
//!         VA copy object dst.
//! \param  [in] src_obj
//!         VA copy object src.
//! \param  [in] option
//!         VA copy option, copy mode.
//! \param  [in] sync_handle
//!         VA copy sync handle
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_Copy (
    VADriverContextP  ctx,
    VACopyObject      *dst_obj,
    VACopyObject      *src_obj,
    VACopyOption      option
);
#endif //VA_CHECK_VERSION(1,10,0)

//!
//! \brief  Check for buffer info
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] buf_id
//!         VA buffer ID
//! \param  [out] type
//!         VA buffer type
//! \param  [out] size
//!         Size
//! \param  [out] num_elements
//!         Number of elements
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_BufferInfo (
    VADriverContextP  ctx,
    VABufferID        buf_id,
    VABufferType      *type,
    uint32_t          *size,
    uint32_t          *num_elements
);

//!
//! \brief  Lock surface
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] surface
//!         VA surface ID
//! \param  [out] fourcc
//!         FourCC
//! \param  [out] luma_stride
//!         Luma stride
//! \param  [out] chroma_u_stride
//!         Chroma U stride
//! \param  [out] chroma_v_stride
//!         Chroma V stride
//! \param  [out] luma_offset
//!         Luma offset
//! \param  [out] chroma_u_offset
//!         Chroma U offset
//! \param  [out] chroma_v_offset
//!         Chroma V offset
//! \param  [out] buffer_name
//!         Buffer name
//! \param  [out] buffer
//!         Buffer
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_LockSurface (
    VADriverContextP  ctx,
    VASurfaceID       surface,
    uint32_t          *fourcc,
    uint32_t          *luma_stride,
    uint32_t          *chroma_u_stride,
    uint32_t          *chroma_v_stride,
    uint32_t          *luma_offset,
    uint32_t          *chroma_u_offset,
    uint32_t          *chroma_v_offset,
    uint32_t          *buffer_name,
    void              **buffer
);

//!
//! \brief  Unlock surface
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] surface
//!         VA surface ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_UnlockSurface (
    VADriverContextP  ctx,
    VASurfaceID       surface
);

//!
//! \brief  Query video proc filters
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] context
//!         VA context ID
//! \param  [in] filters
//!         VA proc filter type
//! \param  [in] num_filters
//!         Number of filters
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QueryVideoProcFilters (
    VADriverContextP   ctx,
    VAContextID        context,
    VAProcFilterType   *filters,
    uint32_t           *num_filters
);

//!
//! \brief  Query video processing filter capabilities.
//!         The real implementation is in media_libva_vp.c, since it needs to use some definitions in vphal.h.
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] context
//!         VA context ID
//! \param  [in] type
//!         VA proc filter type
//! \param  [inout] filter_caps
//!         FIlter caps
//! \param  [inout] num_filter_caps
//!         Number of filter caps
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QueryVideoProcFilterCaps (
    VADriverContextP  ctx,
    VAContextID       context,
    VAProcFilterType  type,
    void              *filter_caps,
    uint32_t          *num_filter_caps
);

//!
//! \brief  Query video proc pipeline caps
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] context
//!         VA context ID
//! \param  [in] filters
//!         VA buffer ID
//! \param  [in] num_filters
//!         Number of filters
//! \param  [in] pipeline_caps
//!         VA proc pipeline caps
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_QueryVideoProcPipelineCaps (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID          *filters,
    uint32_t            num_filters,
    VAProcPipelineCaps  *pipeline_caps
);

//!
//! \brief  Get surface attributes for the supplied config
//! \details    This function retrieves the surface attributes matching the supplied config.
//!
//! \param  [in] ctx
//!         VA display
//! \param  [in] config
//!         Config identifying a codec or a video processing pipeline
//! \param  [out] attrib_list
//!         List of attributes on output
//! \param  [in] num_attributes
//!         Number of attributes
//!
//! \return VAStatus
//!     VA_STATUS_ERROR_UNIMPLEMENTED
//!
VAStatus DdiMedia_GetSurfaceAttributes (
    VADriverContextP ctx,
    VAConfigID       config,
    VASurfaceAttrib  *attrib_list,
    uint32_t         num_attribs
);

//!
//! \brief  Aquire buffer handle
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] buf_id
//!         VA buffer ID
//! \param  [in] buf_info
//!         VA buffer Info
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_AcquireBufferHandle (
    VADriverContextP  ctx,
    VABufferID        buf_id,
    VABufferInfo      *buf_info
);

//!
//! \brief  Release buffer handle
//!
//! \param  [in] ctx
//!         Pointer to VA driver context
//! \param  [in] buf_id
//!         VA bufferID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_ReleaseBufferHandle (
    VADriverContextP  ctx,
    VABufferID        buf_id
);

//!
//! \brief   API for export surface handle to other component
//!
//! \param [in] dpy
//!          VA display.
//! \param [in] surface_id
//!          Surface to export.
//! \param [in] mem_type
//!          Memory type to export to.
//! \param [in] flags
//!          Combination of flags to apply
//!\param [out] descriptor
//!Pointer to the descriptor structure to fill
//!with the handle details.  The type of this structure depends on
//!the value of mem_type.
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_ExportSurfaceHandle (
    VADriverContextP  ctx,
    VASurfaceID       surface_id,
    uint32_t          mem_type,
    uint32_t          flags,
    void              *descriptor);

#ifndef ANDROID
//!
//! \brief Create Mfe Context
//!
//! \param [in] ctx
//!        Pointer to VA driver context
//! \param [out] mfe_context
//!        VA MF context ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_CreateMfeContextInternal (
    VADriverContextP    ctx,
    VAMFContextID      *mfe_context
);

//!
//! \brief Add context
//!
//! \param [in] ctx
//!        Pointer to VA driver context
//! \param [in] contexts
//!        VA context ID
//! \param [in] mfe_context
//!        VA MF context ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_AddContextInternal (
    VADriverContextP    ctx,
    VAContextID         context,
    VAMFContextID      mfe_context
);

//!
//! \brief Release context
//!
//! \param [in] ctx
//!        Pointer to VA driver context
//! \param [in] contexts
//!        VA context ID
//! \param [in] mfe_context
//!        VA MF context ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_ReleaseContextInternal (
    VADriverContextP    ctx,
    VAContextID         context,
    VAMFContextID      mfe_context
);

#endif // ANDROID

#ifdef __cplusplus
extern "C" {
#endif

//! \brief  Hybrid query buffer attributes
//!
//! \param  [in] dpy
//!     VA display
//! \param  [in] context
//!     VA context ID
//! \param  [in] bufferType
//!     VA buffer type
//! \param  [out] outputData
//!     Output data
//! \param  [out] outputDataLen
//!     Length of output data
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
MEDIAAPI_EXPORT VAStatus DdiMedia_HybridQueryBufferAttributes(
    VADisplay    dpy,
    VAContextID  context,
    VABufferType bufferType,
    void        *outputData,
    uint32_t    *outputDataLen);

//! \brief  Set frame ID
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] surface
//!     VA surface ID
//! \param  [in] frame_id
//!     Frame ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiMedia_SetFrameID(
    VADriverContextP    ctx,
    VASurfaceID         surface,
    uint32_t            frame_id);

#ifdef __cplusplus
}
#endif

#endif // __MEDIA_LIBVA_H__

