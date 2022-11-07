/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     media_libva_interface.h
//! \brief    libva interface head file
//!

#ifndef __MEDIA_LIBVA_INTERFACE_H__
#define __MEDIA_LIBVA_INTERFACE_H__

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
#if VA_MAJOR_VERSION < 1
#include "va_internal_android.h"
#endif
#endif // ANDROID
#include <va/va_dec_hevc.h>

class MediaLibvaInterface
{
public:

    //!
    //! \brief  clean up all library internal resources
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus Terminate (VADriverContextP ctx);

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
    static VAStatus QueryConfigEntrypoints (
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
    static VAStatus QueryConfigProfiles (
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
    static VAStatus QueryConfigAttributes (
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
    static VAStatus CreateConfig (
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
    static VAStatus DestroyConfig (
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
    static VAStatus GetConfigAttributes (
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
    static VAStatus CreateSurfaces (
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
    static VAStatus DestroySurfaces (
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
    static VAStatus CreateSurfaces2 (
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
    static VAStatus CreateContext (
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
    static VAStatus DestroyContext (
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
    static VAStatus CreateBuffer (
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
    static VAStatus BufferSetNumElements (
        VADriverContextP  ctx,
        VABufferID        buf_id,
        uint32_t          num_elements
    );

    //!
    //! \brief  Map buffer
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] buf_id
    //!         VA buffer id
    //! \param  [in] pbuf
    //!         Address of buffer
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus MapBuffer (
        VADriverContextP  ctx,
        VABufferID        buf_id,
        void              **pbuf
    );

    //!
    //! \brief  Unmap buffer
    //! \details    After client making changes to a mapped data store, it needs to
    //!             "Unmap" it to let the server know that the data is ready to be
    //!             consumed by the server
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] buf_id
    //!         VA buffer id
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus UnmapBuffer (
        VADriverContextP  ctx,
        VABufferID        buf_id
    );

    //!
    //! \brief  Destroy buffer
    //! \details    After this call, the buffer is deleted and this buffer_id is no longer valid
    //!             Only call this if the buffer is not going to be passed to vaRenderBuffer
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] buf_id
    //!         VA buffer id
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DestroyBuffer (
        VADriverContextP  ctx,
        VABufferID        buffer_id
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
    static VAStatus BeginPicture (
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
    static VAStatus RenderPicture (
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
    static VAStatus EndPicture (
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
    static VAStatus SyncSurface (
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
    static VAStatus SyncSurface2 (
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
    static VAStatus SyncBuffer (
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
    static VAStatus QuerySurfaceStatus (
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
    static VAStatus QuerySurfaceError (
        VADriverContextP  ctx,
        VASurfaceID       render_target,
        VAStatus          error_status,
        void              **error_info
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
    static VAStatus QuerySurfaceAttributes (
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
    static VAStatus PutSurface (
        VADriverContextP  ctx,
        VASurfaceID       surface,
        void*             draw,
        int16_t           srcx,
        int16_t           srcy,
        uint16_t          srcw,
        uint16_t          srch,
        int16_t           destx,
        int16_t           desty,
        uint16_t          destw,
        uint16_t          desth,
        VARectangle       *cliprects,
        uint32_t          number_cliprects,
        uint32_t          flags
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
    static VAStatus QueryImageFormats (
        VADriverContextP  ctx,
        VAImageFormat     *format_list,
        int32_t           *num_formats
    );

    //!
    //! \brief  Create an image
    //!
    //! \param  [in] ctx
    //!         Driver context
    //! \param  [in] format
    //!         The format of image
    //! \param  [in] width
    //!         The width of the image
    //! \param  [in] height
    //!         The height of the image
    //! \param  [out] image
    //!         The generated image
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus CreateImage (
        VADriverContextP  ctx,
        VAImageFormat     *format,
        int32_t           width,
        int32_t           height,
        VAImage           *image
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
    static VAStatus DeriveImage (
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
    static VAStatus DestroyImage (
        VADriverContextP  ctx,
        VAImageID         image
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
    //!         The image ID of the source image
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus GetImage (
        VADriverContextP  ctx,
        VASurfaceID       surface,
        int32_t           x,
        int32_t           y,
        uint32_t          width,
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
    static VAStatus PutImage (
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
    static VAStatus QueryDisplayAttributes (
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
    static VAStatus GetDisplayAttributes (
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
    static VAStatus QueryProcessingRate (
        VADriverContextP           ctx,
        VAConfigID                 config_id,
        VAProcessingRateParameter  *proc_buf,
        uint32_t                   *processing_rate
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
    static VAStatus Copy (
        VADriverContextP  ctx,
        VACopyObject      *dst_obj,
        VACopyObject      *src_obj,
        VACopyOption      option
    );
#endif

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
    static VAStatus BufferInfo (
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
    static VAStatus LockSurface (
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
    static VAStatus UnlockSurface (
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
    static VAStatus QueryVideoProcFilters (
        VADriverContextP  ctx,
        VAContextID       context,
        VAProcFilterType  *filters,
        uint32_t          *num_filters
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
    static VAStatus QueryVideoProcFilterCaps (
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
    static VAStatus QueryVideoProcPipelineCaps (
        VADriverContextP    ctx,
        VAContextID         context,
        VABufferID          *filters,
        uint32_t            num_filters,
        VAProcPipelineCaps  *pipeline_caps
    );

#if VA_CHECK_VERSION(1,11,0)
    //!
    //! \brief   Create protected session
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] configId
    //!          VA configuration ID
    //! \param   [out] protected_session
    //!          VA protected session ID
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus CreateProtectedSession (
        VADriverContextP      ctx,
        VAConfigID            config_id,
        VAProtectedSessionID  *protected_session
    );

    //!
    //! \brief   Destroy protected session
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] protected_session
    //!          VA protected session ID
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DestroyProtectedSession (
        VADriverContextP      ctx,
        VAProtectedSessionID  protected_session
    );

    //!
    //! \brief   Attach protected session to display or context
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] context
    //!          VA context ID to be attached if not 0.
    //! \param   [in] protected_session
    //!          VA protected session ID
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus AttachProtectedSession (
        VADriverContextP      ctx,
        VAContextID           context,
        VAProtectedSessionID  protected_session
    );

    //!
    //! \brief   Detach protected session from display or context
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] context
    //!          VA context ID to be Detached if not 0.
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DetachProtectedSession (
        VADriverContextP  ctx,
        VAContextID       context
    );

    //!
    //! \brief   TEE execution for the particular protected session
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] protected_session
    //!          VA protected session ID
    //! \param   [in] data
    //!          VA buffer ID
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus ProtectedSessionExecute (
        VADriverContextP      ctx,
        VAProtectedSessionID  protected_session,
        VABufferID            data
    );
#endif

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
    static VAStatus GetSurfaceAttributes (
        VADriverContextP  ctx,
        VAConfigID        config,
        VASurfaceAttrib   *attrib_list,
        uint32_t          num_attribs
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
    static VAStatus AcquireBufferHandle (
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
    static VAStatus ReleaseBufferHandle (
        VADriverContextP  ctx,
        VABufferID        buf_id
    );

    //!
    //! \brief API for export surface handle to other component
    //!
    //! \param [in] dpy
    //!        VA display.
    //! \param [in] surface_id
    //!        Surface to export.
    //! \param [in] mem_type
    //!        Memory type to export to.
    //! \param [in] flags
    //!        Combination of flags to apply
    //! \param [out] descriptor
    //!        Pointer to the descriptor structure to fill
    //!        with the handle details.  The type of this structure depends on
    //!        the value of mem_type.
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus ExportSurfaceHandle (
        VADriverContextP  ctx,
        VASurfaceID       surface_id,
        uint32_t          mem_type,
        uint32_t          flags,
        void              *descriptor
    );

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
    static VAStatus CreateMfeContextInternal (
        VADriverContextP  ctx,
        VAMFContextID     *mfe_context
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
    static VAStatus AddContextInternal (
        VADriverContextP  ctx,
        VAContextID       context,
        VAMFContextID     mfe_context
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
    static VAStatus ReleaseContextInternal (
        VADriverContextP  ctx,
        VAContextID       context,
        VAMFContextID     mfe_context
    );

    //!
    //! \brief MFE submit
    //!
    //! \param [in] ctx
    //!        Pointer to VA driver context
    //! \param [in] mfe_context
    //!        VA MF context ID
    //! \param [in] contexts
    //!        VA context ID
    //! \param [in] num_contexts
    //!        Number of contexts
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus MfeSubmit (
        VADriverContextP  ctx,
        VAMFContextID     mfe_context,
        VAContextID       *contexts,
        int32_t           num_contexts
    );
#endif

    //!
    //! \brief  Load DDI function pointer
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus LoadFunction(VADriverContextP ctx);

    //!
    //! \brief  Initialize
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] devicefd
    //!         Devoce fd
    //! \param  [out] major_version
    //!         Major version
    //! \param  [out] minor_version
    //!         Minor version
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus InitMediaContext (
        VADriverContextP ctx,
        int32_t          devicefd,
        int32_t         *major_version,
        int32_t         *minor_version
    );

};

#endif //__MEDIA_LIBVA_INTERFACE_H__