/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     media_libva_interface_next.h
//! \brief    libva interface next head file
//!

#ifndef __MEDIA_LIBVA_INTERFACE_NEXT_H__
#define __MEDIA_LIBVA_INTERFACE_NEXT_H__

#include <va/va.h>
#include <va/va_backend.h>
#include <va/va_drmcommon.h>
#include "media_libva_common_next.h"
#include "ddi_media_functions.h"

class MediaLibvaInterfaceNext
{
public:

    //!
    //! \brief  Init component list
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus InitCompList(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Release component list
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context
    //!
    static void ReleaseCompList(PDDI_MEDIA_CONTEXT mediaCtx);

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
    static VAStatus Initialize (
        VADriverContextP ctx,
        int32_t          devicefd,
        int32_t          *major_version,     /* out */
        int32_t          *minor_version      /* out */
    );

    //!
    //! \brief  clean up all library internal resources
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus Terminate(VADriverContextP ctx);

    //!
    //! \brief  Create context
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] configId
    //!         VA config id
    //! \param  [in] pictureWidth
    //!         Picture width
    //! \param  [in] pictureHeight
    //!         Picture height
    //! \param  [out] flag
    //!         Create flag
    //! \param  [in] renderTargets
    //!         VA render traget
    //! \param  [in] renderTargetsNum
    //!         Number of render targets
    //! \param  [out] context
    //!         VA created context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus CreateContext (
        VADriverContextP  ctx,
        VAConfigID        configId,
        int32_t           pictureWidth,
        int32_t           pictureHeight,
        int32_t           flag,
        VASurfaceID       *renderTargets,
        int32_t           rendertargetsNum,
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
    //! \param  [out] elementsNum
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
        uint32_t          elementsNum,
        void              *data,
        VABufferID        *bufId
    );

    //!
    //! \brief  Destroy buffer
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] buf_id
    //!         VA buffer ID
    //!
    //! \return     VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DestroyBuffer(
        VADriverContextP    ctx,
        VABufferID          buf_id
    );

    //!
    //! \brief  Get ready to decode a picture to a target surface
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] context
    //!         VA context id
    //! \param  [in] renderTarget
    //!         VA render target surface
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus BeginPicture (
        VADriverContextP  ctx,
        VAContextID       context,
        VASurfaceID       renderTarget
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
    //! \param  [in] buffersNum
    //!         number of buffers
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus RenderPicture (
        VADriverContextP  ctx,
        VAContextID       context,
        VABufferID        *buffers,
        int32_t           buffersNum
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
    //! \brief    Query supported entrypoints for a given profile
    //!
    //! \param    [in] ctx
    //!          Pointer to VA driver context
    //! \param    [in] profile
    //!           VA profile
    //! \param    [in] entrypointList
    //!           Pointer to VAEntrypoint array that can hold at least vaMaxNumEntrypoints() entries
    //! \param    [out] numEntryPoints
    //!           It returns the actual number of supported VAEntrypoints.
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    static VAStatus QueryConfigEntrypoints(
        VADriverContextP ctx,
        VAProfile        profile,
        VAEntrypoint     *entrypointList,
        int32_t          *entrypointsNum);

    //!
    //! \brief    Query supported profiles
    //!
    //! \param    [in] ctx
    //!          Pointer to VA driver context
    //! \param    [in] profileList
    //!           Pointer to VAProfile array that can hold at least vaMaxNumProfile() entries
    //! \param    [out] numProfiles
    //!           Pointer to int32_t. It returns the actual number of supported profiles.
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    static VAStatus QueryConfigProfiles(
        VADriverContextP  ctx,
        VAProfile         *profileList,
        int32_t           *profilesNum);

    //!
    //! \brief    Query all attributes for a given configuration
    //!
    //! \param    [in] ctx
    //!          Pointer to VA driver context
    //! \param    [in] configId
    //!           VA configuration
    //! \param    [in,out] profile
    //!           Pointer to VAProfile of the configuration
    //! \param    [in,out] entrypoint
    //!           Pointer to VAEntrypoint of the configuration
    //! \param    [in,out] attribList
    //!           Pointer to VAConfigAttrib array that can hold at least
    //!           vaMaxNumConfigAttributes() entries.
    //! \param    [in,out] numAttribs
    //!           The actual number of VAConfigAttrib returned in the array attribList
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    static VAStatus QueryConfigAttributes(
        VADriverContextP  ctx,
        VAConfigID        configId,
        VAProfile         *profile,
        VAEntrypoint      *entrypoint,
        VAConfigAttrib    *attribList,
        int32_t           *numAttribs);

    //!
    //! \brief    Create a configuration
    //! \details  It passes in the attribute list that specifies the attributes it
    //!           cares about, with the rest taking default values.
    //!
    //! \param    [in] ctx
    //!          Pointer to VA driver context
    //! \param    [in] profile
    //!           VA profile
    //! \param    [in] entrypoint
    //!           VA entrypoint
    //! \param    [in] attribList
    //!           Pointer to VAConfigAttrib array that specifies the attributes
    //! \param    [in] numAttribs
    //!           Number of VAConfigAttrib in the array attribList
    //! \param    [out] configId
    //!           Pointer to returned VAConfigID if success
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    static VAStatus CreateConfig(
        VADriverContextP  ctx,
        VAProfile         profile,
        VAEntrypoint      entrypoint,
        VAConfigAttrib    *attribList,
        int32_t           numAttribs,
        VAConfigID        *configId);

    //!
    //! \brief    Destory the VAConfigID
    //!
    //! \param    [in] ctx
    //!          Pointer to VA driver context
    //! \param    [in] configId
    //!           Specify the VAConfigID
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if succeed
    //!           VA_STATUS_ERROR_INVALID_CONFIG if the conifgId is invalid
    //!
    static VAStatus DestroyConfig(
        VADriverContextP  ctx,
        VAConfigID        configId);

    //!
    //! \brief    Get attributes for a given profile/entrypoint pair
    //! \details  The caller must provide an "attribList" with all attributes to be
    //!           retrieved.  Upon return, the attributes in "attribList" have been
    //!           updated with their value.  Unknown attributes or attributes that are
    //!           not supported for the given profile/entrypoint pair will have their
    //!           value set to VA_ATTRIB_NOT_SUPPORTED.
    //!
    //! \param    [in] ctx
    //!          Pointer to VA driver context
    //! \param    [in] profile
    //!           VA profile
    //! \param    [in] entrypoint
    //!           VA entrypoint
    //! \param    [in,out] attribList
    //!           Pointer to VAConfigAttrib array. The attribute type is set by caller and
    //!           attribute value is set by this function.
    //! \param    [in] numAttribs
    //!           Number of VAConfigAttrib in the array attribList
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    static VAStatus GetConfigAttributes(
        VADriverContextP  ctx,
        VAProfile         profile,
        VAEntrypoint      entrypoint,
        VAConfigAttrib    *attribList,
        int32_t           numAttribs);

    //!
    //! \brief    Get surface attributes for a given config ID
    //!
    //! \param    [in] ctx
    //!          Pointer to VA driver context
    //! \param    [in] configId
    //!           VA configuration
    //! \param    [in,out] attribList
    //!           Pointer to VASurfaceAttrib array. It returns
    //!           the supported  surface attributes
    //! \param    [in,out] numAttribs
    //!           The number of elements allocated on input
    //!           Return the number of elements actually filled in output
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!           VA_STATUS_ERROR_MAX_NUM_EXCEEDED if size of attribList is too small
    //!
    static VAStatus QuerySurfaceAttributes(
        VADriverContextP ctx,
        VAConfigID       configId,
        VASurfaceAttrib  *attribList,
        uint32_t         *numAttribs);

    //!
    //! \brief    Query the suppported image formats
    //!
    //! \param    [in] ctx
    //!          Pointer to VA driver context
    //! \param    [in,out] formatList
    //!           Pointer to a VAImageFormat array. The array size shouldn't be less than vaMaxNumImageFormats
    //!           It will return the supported image formats.
    //! \param    [in,out] num_formats
    //!           Pointer to a integer that will return the real size of formatList.
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if succeed
    //!
    static VAStatus QueryImageFormats(
        VADriverContextP ctx,
        VAImageFormat    *formatList,
        int32_t          *numFormats);

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
    static VAStatus SetImagePalette(
        VADriverContextP ctx,
        VAImageID        image,
        unsigned char    *palette);

    //!
    //! \brief  Query subpicture formats
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] formatList
    //!         VA image format
    //! \param  [in] flags
    //!         Flags
    //! \param  [in] formatsNum
    //!         Number of formats
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus QuerySubpictureFormats(
        VADriverContextP ctx,
        VAImageFormat    *formatList,
        uint32_t         *flags,
        uint32_t         *formatsNum);
    
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
    static VAStatus CreateSubpicture(
        VADriverContextP ctx,
        VAImageID        image,
        VASubpictureID   *subpicture);
    
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
    static VAStatus DestroySubpicture(
        VADriverContextP ctx,
        VASubpictureID   subpicture);
    
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
    static VAStatus SetSubpictureImage(
        VADriverContextP ctx,
        VASubpictureID   subpicture,
        VAImageID        image);
    
    //!
    //! \brief  Set subpicture chrome key
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] subpicture
    //!         VA subpicture ID
    //! \param  [in] chromakeyMin
    //!         Minimum chroma key
    //! \param  [in] chromakeyMax
    //!         Maximum chroma key
    //! \param  [in] chromakeyMask
    //!         Chromakey mask
    //!
    //! \return VAStatus
    //!     VA_STATUS_ERROR_UNIMPLEMENTED
    //!
    static VAStatus SetSubpictureChromakey(
        VADriverContextP ctx,
        VASubpictureID   subpicture,
        uint32_t         chromakeyMin,
        uint32_t         chromakeyMax,
        uint32_t         chromakeyMask);
    
    //!
    //! \brief  set subpicture global alpha
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] subpicture
    //!         VA subpicture ID
    //! \param  [in] globalAlpha
    //!         Global alpha
    //!
    //! \return VAStatus
    //!     VA_STATUS_ERROR_UNIMPLEMENTED
    //!
    static VAStatus SetSubpictureGlobalAlpha(
        VADriverContextP ctx,
        VASubpictureID   subpicture,
        float            globalAlpha);
    
    //!
    //! \brief  Associate subpicture
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] subpicture
    //!         VA subpicture ID
    //! \param  [in] targetSurfaces
    //!         VA surface ID
    //! \param  [in] surfacesNum
    //!         Number of surfaces
    //! \param  [in] srcX
    //!         Source x of the region
    //! \param  [in] srcY
    //!         Source y of the region
    //! \param  [in] srcWidth
    //!         Source width of the region
    //! \param  [in] srcHeight
    //!         Source height of the region
    //! \param  [in] destX
    //!         Destination x
    //! \param  [in] destY
    //!         Destination y
    //! \param  [in] destWidth
    //!         Destination width
    //! \param  [in] destHeight
    //!         Destination height
    //! \param  [in] flags
    //!         Flags
    //!
    //! \return VAStatus
    //!     VA_STATUS_ERROR_UNIMPLEMENTED
    //!
    static VAStatus AssociateSubpicture(
        VADriverContextP ctx,
        VASubpictureID   subpicture,
        VASurfaceID      *targetSurfaces,
        int32_t          surfacesNum,
        int16_t          srcX,  /* upper left offset in subpicture */
        int16_t          srcY,
        uint16_t         srcWidth,
        uint16_t         srcHeight,
        int16_t          destX, /* upper left offset in surface */
        int16_t          destY,
        uint16_t         destWidth,
        uint16_t         destHeight,
        uint32_t         flags);
    
    //!
    //! \brief  Deassociate subpicture
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] subpicture
    //!         VA subpicture ID
    //! \param  [in] targetSurfaces
    //!         VA surface ID
    //! \param  [in] surfacesNum
    //!         Number of surfaces
    //!
    //! \return VAStatus
    //!     VA_STATUS_ERROR_UNIMPLEMENTED
    //!
    static VAStatus DeassociateSubpicture(
        VADriverContextP ctx,
        VASubpictureID   subpicture,
        VASurfaceID      *targetSurfaces,
        int32_t          surfacesNum);
    
    //!
    //! \brief  Set display attributes
    //! \details    Only attributes returned with VA_DISPLAY_ATTRIB_SETTABLE set in the "flags" field
    //!         from vaQueryDisplayAttributes() can be set.  If the attribute is not settable or
    //!         the value is out of range, the function returns VA_STATUS_ERROR_ATTR_NOT_SUPPORTED
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] attrList
    //!         VA display attribute
    //! \param  [in] attributesNum
    //!         Number of attributes
    //!
    //! \return VAStatus
    //!     VA_STATUS_ERROR_UNIMPLEMENTED
    //!
    static VAStatus SetDisplayAttributes(
        VADriverContextP    ctx,
        VADisplayAttribute  *attrList,
        int32_t             attributesNum);

    //!
    //! \brief  Query display attributes
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] attrList
    //!         VA display attribute
    //! \param  [in] attributesNum
    //!         Number of attributes
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus QueryDisplayAttributes(
        VADriverContextP    ctx,
        VADisplayAttribute  *attrList,
        int32_t             *attributesNum);

    //!
    //! \brief  Get display attributes
    //! \details    This function returns the current attribute values in "attr_list".
    //!         Only attributes returned with VA_DISPLAY_ATTRIB_GETTABLE set in the "flags" field
    //!         from vaQueryDisplayAttributes() can have their values retrieved.
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] attrList
    //!         VA display attribute
    //! \param  [in] attributesNum
    //!         Number of attributes
    //!
    //! \return VAStatus
    //!     VA_STATUS_ERROR_UNIMPLEMENTED
    //!
    static VAStatus GetDisplayAttributes(
        VADriverContextP    ctx,
        VADisplayAttribute  *attrList,
        int32_t             attributesNum);

    //!
    //! \brief  Convey to the server how many valid elements are in the buffer
    //! \details    e.g. if multiple slice parameters are being held in a single buffer,
    //!             this will communicate to the server the number of slice parameters
    //!             that are valid in the buffer.
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] bufId
    //!         VA buffer id
    //! \param  [in] elementsNum
    //!         Number of elements in buffer
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus BufferSetNumElements(
        VADriverContextP ctx,
        VABufferID       bufId,
        uint32_t         elementsNum);

    //!
    //! \brief    Get process rate for a given config ID
    //!
    //! \param    [in] ctx
    //!          Pointer to VA driver context
    //! \param    [in] configId
    //!           VA configuration
    //! \param    [in,out] procBuf
    //!           Pointer to VAProcessingRateParameter
    //! \param    [in,out] processingRate
    //!           Return the process rate
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    static VAStatus QueryProcessingRate(
        VADriverContextP           ctx,
        VAConfigID                 configId,
        VAProcessingRateParameter  *procBuf,
        uint32_t                   *processingRate);

    //!
    //! \brief  Query video proc filters
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] context
    //!         VA context ID
    //! \param  [in] filters
    //!         VA proc filter type
    //! \param  [in] filtersNum
    //!         Number of filters
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus QueryVideoProcFilters (
        VADriverContextP  ctx,
        VAContextID       context,
        VAProcFilterType  *filters,
        uint32_t          *filtersNum
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
    //! \param  [inout] filterCaps
    //!         FIlter caps
    //! \param  [inout] filterCapsNum
    //!         Number of filter caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus QueryVideoProcFilterCaps (
        VADriverContextP  ctx,
        VAContextID       context,
        VAProcFilterType  type,
        void              *filterCaps,
        uint32_t          *filterCapsNum
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
    //! \param  [in] filtersNum
    //!         Number of filters
    //! \param  [in] pipelineCaps
    //!         VA proc pipeline caps
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus QueryVideoProcPipelineCaps (
        VADriverContextP    ctx,
        VAContextID         context,
        VABufferID          *filters,
        uint32_t            filtersNum,
        VAProcPipelineCaps  *pipelineCaps
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
    //! \param  [in] surfacesNum
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
        int32_t           surfacesNum,
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
    //! \param  [in] surfacesNum
    //!         Number of surfaces in the array to be destroyed
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DestroySurfaces (
        VADriverContextP  ctx,
        VASurfaceID       *surfaces,
        int32_t           surfacesNum
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
    //! \param  [in] surfacesNum
    //!         Number of surfaces
    //! \param  [out] attribList
    //!         VA attrib list
    //! \param  [in] attribsNum
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
        uint32_t          surfacesNum,
        VASurfaceAttrib   *attribList,
        uint32_t          attribsNum
    );

#if VA_CHECK_VERSION(1, 9, 0)
    //!
    //! \brief  Sync Surface
    //! \details    Sync surface
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] surfaceId
    //!         VA surface id
    //! \param  [in] timeoutNs
    //!         time out period
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus SyncSurface2 (
        VADriverContextP    ctx,
        VASurfaceID         surfaceId,
        uint64_t            timeoutNs
    );

    //!
    //! \brief  Sync buffer
    //! \details    This function blocks until all pending operations on the render target
    //!             have been completed.  Upon return it is safe to use the render target for a
    //!             different picture
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] bufId
    //!         VA buffer id
    //! \param  [in] timeoutNs
    //!         time out period
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus SyncBuffer (
        VADriverContextP  ctx,
        VABufferID        bufId,
        uint64_t          timeoutNs
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
        VADriverContextP ctx,
        VABufferID       bufId,
        VABufferType     *type,
        uint32_t         *size,
        uint32_t         *elementsNum);

    //!
    //! \brief  Query surface status
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] renderTarget
    //!         VA surface ID
    //! \param  [out] status
    //!         VA surface status
    static VAStatus QuerySurfaceStatus (
        VADriverContextP  ctx,
        VASurfaceID       renderTarget,
        VASurfaceStatus   *status);

    //!
    //! \brief  Get surface attributes for the supplied config.
    //!
    //! This function retrieves the surface attributes matching the supplied
    //! config. The caller shall provide an \c attrib_list with all attributes
    //! to be retrieved. Upon successful return, the attributes in \c attrib_list
    //! are updated with the requested value. Unknown attributes or attributes
    //! that are not supported for the given config will have their \c flags
    //! field set to \c VA_SURFACE_ATTRIB_NOT_SUPPORTED.
    //!
    //! \param  [in]  ctx
    //!         VA display
    //! \param  [in]  config
    //!         the config identifying a codec or a video processing pipeline
    //! \param  [out] attribList        
    //!         the list of attributes on output, with at least \c type fields filled in, 
    //!         and possibly \c value fields whenever necessary.The updated list of attributes and flags on output
    //! \param  [in] attribsNum       
    //!         the number of attributes supplied in the \c attrib_list array
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus GetSurfaceAttributes (
        VADriverContextP   ctx,
        VAConfigID         config,
        VASurfaceAttrib    *attribList,
        uint32_t           attribsNum);

    //!
    //! \brief  Aquire buffer handle
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] bufId
    //!         VA buffer ID
    //! \param  [in] bufInfo
    //!         VA buffer Info
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus AcquireBufferHandle (
        VADriverContextP ctx,
        VABufferID       bufId,
        VABufferInfo     *bufInfo);

    //!
    //! \brief  Release buffer handle
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] bufId
    //!         VA bufferID
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus ReleaseBufferHandle(
        VADriverContextP ctx,
        VABufferID       bufId);
 
    //!
    //! \brief API for export surface handle to other component
    //!
    //! \param [in] dpy
    //!        VA display.
    //! \param [in] surfaceId
    //!        Surface to export.
    //! \param [in] memType
    //!        Memory type to export to.
    //! \param [in] flags
    //!        Combination of flags to apply
    //! \param [out] descriptor
    //!        Pointer to the descriptor structure to fill
    //!        with the handle details.  The type of this structure depends on
    //!        the value of mem_type.
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus ExportSurfaceHandle(
        VADriverContextP ctx,
        VASurfaceID      surfaceId,
        uint32_t         memType,
        uint32_t         flags,
        void             *descriptor);

    //!
    //! \brief  media copy internal
    //! 
    //! \param  [in] mosCtx
    //!         Pointer to mos context
    //! \param  [in] src
    //!         VA copy mos resource src.
    //! \param  [in] dst
    //!         VA copy mos resrouce dst.
    //! \param  [in] option
    //!         VA copy option, copy mode.
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus CopyInternal(
        PMOS_CONTEXT    mosCtx,
        PMOS_RESOURCE   src,
        PMOS_RESOURCE   dst,
        uint32_t        copy_mode
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

#if VA_CHECK_VERSION(1,11,0)

    //!
    //! \brief   Create protected session
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] configId
    //!          VA configuration ID
    //! \param   [out] protectedSession
    //!          VA protected session ID
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus CreateProtectedSession (
        VADriverContextP      ctx,
        VAConfigID            configId,
        VAProtectedSessionID  *protectedSession
    );

    //!
    //! \brief   Destroy protected session
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] protectedSession
    //!          VA protected session ID
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DestroyProtectedSession (
        VADriverContextP      ctx,
        VAProtectedSessionID  protectedSession
    );

    //!
    //! \brief   Attach protected session to display or context
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] context
    //!          VA context ID to be attached if not 0.
    //! \param   [in] protectedSession
    //!          VA protected session ID
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus AttachProtectedSession (
        VADriverContextP      ctx,
        VAContextID           context,
        VAProtectedSessionID  protectedSession
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
    //! \param   [in] protectedSession
    //!          VA protected session ID
    //! \param   [in] data
    //!          VA buffer ID
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus ProtectedSessionExecute (
        VADriverContextP      ctx,
        VAProtectedSessionID  protectedSession,
        VABufferID            data
    );
#endif

    //!
    //! \brief  Convert media format to OS format
    //!
    //! \param  [in] format
    //!         Ddi media format
    //!
    //! \return Os format if call sucesss,else
    //!     VA_STATUS_ERROR_UNSUPPORTED_RT_FORMAT if fail
    //!
    static int32_t MediaFormatToOsFormat(DDI_MEDIA_FORMAT format);

    //!
    //! \brief  Destroy buffer from VA buffer ID
    //!
    //! \param  [in] mediaCtx
    //!     Pointer to ddi media context
    //! \param  [in] bufferID
    //!     VA buffer ID
    //!
    //! \return     bool
    //!     true if destroy buffer from VA buffer ID, else false
    //!
    static bool DestroyBufFromVABufferID(
        PDDI_MEDIA_CONTEXT mediaCtx,
        VABufferID         bufferID);
    
    //!
    //! \brief  Sync Surface
    //! \details    Sync surface
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] surfaceId
    //!         VA surface id
    //! \param  [in] timeoutNs
    //!         time out period
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus SyncSurface(
        VADriverContextP    ctx,
        VASurfaceID         renderTarget);

    //!
    //! \brief   Query Surface Error
    //!
    //! \param   [in] ctx
    //!          Pointer to VA driver context
    //! \param   [in] renderTarget
    //!          VASurfaceID
    //! \param   [in] errorStatus
    //!          error Status
    //! \param   [in] errorInfo
    //!          error info
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus QuerySurfaceError(
        VADriverContextP ctx,
        VASurfaceID      renderTarget,
        VAStatus         errorStatus,
        void             **errorInfo);
    
    //!
    //! \brief  Copy data from a VAImage to a surface
    //! \details    Image must be in a format supported by the implementation
    //!
    //! \param  [in] ctx
    //!         Input driver context
    //! \param  [in] surface
    //!         Surface ID of destination
    //! \param  [in] draw
    //!         Drawable of window system
    //! \param  [in] image
    //!         The image ID of the destination image
    //! \param  [in] srcx
    //!         Source x offset of the image region
    //! \param  [in] srcy
    //!         Source y offset of the image region
    //! \param  [in] srcw
    //!         Source width offset of the image region
    //! \param  [in] srch
    //!         Source height offset of the image region
    //! \param  [in] destx
    //!         Destination x offset of the surface region
    //! \param  [in] desty
    //!         Destination y offset of the surface region
    //! \param  [in] destw
    //!         Destination width offset of the surface region
    //! \param  [in] desth
    //!         Destination height offset of the surface region
    //! \param  [in] cliprects
    //!         client supplied clip list
    //! \param  [in] numberCliprects
    //!         number of clip rects in the clip list
    //! \param  [in] flags
    //!         de-interlacing flags
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus PutSurface(
        VADriverContextP ctx,
        VASurfaceID      surface,
        void             *draw,
        int16_t          srcx,
        int16_t          srcy,
        uint16_t         srcw,
        uint16_t         srch,
        int16_t          destx,
        int16_t          desty,
        uint16_t         destw,
        uint16_t         desth,
        VARectangle      *cliprects,        /* client supplied clip list */
        uint32_t         numberCliprects, /* number of clip rects in the clip list */
        uint32_t         flags             /* de-interlacing flags */
    );

    //!
    //! \brief  Destroy Image
    //! \details    Destroy VA Image
    //!
    //! \param  [in] ctx
    //!         Input driver context
    //! \param  [in] image
    //!         VA image ID
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DestroyImage(
        VADriverContextP ctx,
        VAImageID        image);

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
    static VAStatus GetImage(
        VADriverContextP ctx,
        VASurfaceID      surface,
        int32_t          x,     /* coordinates of the upper left source pixel */
        int32_t          y,
        uint32_t         width, /* width and height of the region */
        uint32_t         height,
        VAImageID        image
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
    //! \param  [in] srcX
    //!         Source x offset of the image region
    //! \param  [in] srcY
    //!         Source y offset of the image region
    //! \param  [in] srcWidth
    //!         Source width offset of the image region
    //! \param  [in] srcHeight
    //!         Source height offset of the image region
    //! \param  [in] destX
    //!         Destination x offset of the surface region
    //! \param  [in] destY
    //!         Destination y offset of the surface region
    //! \param  [in] destWidth
    //!         Destination width offset of the surface region
    //! \param  [in] destHeight
    //!         Destination height offset of the surface region
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus PutImage(
        VADriverContextP ctx,
        VASurfaceID      surface,
        VAImageID        image,
        int32_t          srcX,
        int32_t          srcY,
        uint32_t         srcWidth,
        uint32_t         srcHeight,
        int32_t          destX,
        int32_t          destY,
        uint32_t         destWidth,
        uint32_t         destHeight
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
    //! \param  [out] lumaStride
    //!         Luma stride
    //! \param  [out] chromaUStride
    //!         Chroma U stride
    //! \param  [out] chromaVStride
    //!         Chroma V stride
    //! \param  [out] lumaOffset
    //!         Luma offset
    //! \param  [out] chromaUOffset
    //!         Chroma U offset
    //! \param  [out] chromaVOffset
    //!         Chroma V offset
    //! \param  [out] bufferName
    //!         Buffer name
    //! \param  [out] buffer
    //!         Buffer
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus LockSurface(
        VADriverContextP ctx,
        VASurfaceID      surface,
        uint32_t        *fourcc,
        uint32_t        *lumaStride,
        uint32_t        *chromaUStride,
        uint32_t        *chromaVStride,
        uint32_t        *lumaOffset,
        uint32_t        *chromaUOffset,
        uint32_t        *chromaVOffset,
        uint32_t        *bufferName,
        void           **buffer);
    
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
    static VAStatus UnlockSurface(
        VADriverContextP   ctx,
        VASurfaceID        surface);
private:

    //!
    //! \brief  Copy Surface To Image
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] surface
    //!         Input surface
    //! \param  [in] image
    //!         Output image
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus CopySurfaceToImage(
        VADriverContextP  ctx,
        DDI_MEDIA_SURFACE *surface,
        VAImage           *image);

    //!
    //! \brief  Copy plane from src to dst row by row when src and dst strides are different
    //!
    //! \param  [in] dst
    //!         Destination plane
    //! \param  [in] dstPitch
    //!         Destination plane pitch
    //! \param  [in] src
    //!         Source plane
    //! \param  [in] srcPitch
    //!         Source plane pitch
    //! \param  [in] height
    //!         Plane hight
    //!
    static void CopyPlane(
        uint8_t  *dst,
        uint32_t dstPitch,
        uint8_t  *src,
        uint32_t srcPitch,
        uint32_t height);

    //!
    //! \brief  Map CompType from entrypoint
    //! 
    //! \param  [in] entrypoint
    //!         VAEntrypoint
    //!
    //! \return CompType
    //!
    static CompType MapCompTypeFromEntrypoint(VAEntrypoint entrypoint);

    //!
    //! \brief  Map CompType from CtxType
    //! 
    //! \param  [in] ctxType
    //!         context type
    //!
    //! \return CompType
    //!
    static CompType MapComponentFromCtxType(uint32_t ctxType);

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
    //! \brief  Free for media context
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context
    //!
    static void FreeForMediaContext(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Free for media context
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context mutex
    //!
    static void DestroyMediaContextMutex(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Create media context
    //!
    //!
    static PDDI_MEDIA_CONTEXT CreateMediaDriverContext();

    //!
    //! \brief  Initialize
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to DDI media driver context
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus HeapInitialize(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Get Plane Num
    //!
    //! \param  [in] mediaSurface
    //!         Pointer to DDI media surface
    //! \param  [in] hasAuxPlane
    //!         if has Aus Plane
    //!
    //! \return uint32_t
    //!     Plane num
    //!
    static uint32_t GetPlaneNum(PDDI_MEDIA_SURFACE mediaSurface, bool hasAuxPlane);

    //!
    //! \brief  Get Drm Format Of Separate Plane
    //!
    //! \param  [in] fourcc
    //!         Format
    //! \param  [in] plane
    //!         Plane
    //!
    //! \return uint32_t
    //!     Drmformat
    //!
    static uint32_t GetDrmFormatOfSeparatePlane(uint32_t fourcc, int plane);

    //!
    //! \brief  Get Drm Format Of Separate Plane
    //!
    //! \param  [in] fourcc
    //!         Format
    //!
    //! \return uint32_t
    //!     Drmformat
    //!
    static uint32_t GetDrmFormatOfCompositeObject(uint32_t fourcc);

    //!
    //! \brief  Get Chroma Pitch and Height
    //!
    //! \param  [in] fourcc
    //!         Format
    //! \param  [in] pitch
    //!         Pitch
    //! \param  [in] height
    //!         Height
    //! \param  [out] chromaPitch
    //!         ChromaPitch
    //! \param  [out] chromaHeight
    //!         ChromaHeight
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus GetChromaPitchHeight(
        uint32_t fourcc,
        uint32_t pitch,
        uint32_t height,
        uint32_t *chromaPitch,
        uint32_t *chromaHeight);
    
    //!
    //! \brief  Get VA image from VA image ID
    //! 
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context
    //! \param  [in] imageID
    //!         VA image ID
    //!
    //! \return VAImage*
    //!     Pointer to VAImage
    //!
    static VAImage* GetVAImageFromVAImageID (PDDI_MEDIA_CONTEXT mediaCtx, VAImageID imageID);

    //!
    //! \brief  Destroy image from VA image ID 
    //! 
    //! \param  [in] mediaCtx
    //!         Pointer to ddi media context
    //! \param  [in] imageID
    //!     VA image ID
    //!
    //! \return bool
    //!     True if destroy image from VA image ID, else fail
    //!
    static bool DestroyImageFromVAImageID (PDDI_MEDIA_CONTEXT mediaCtx, VAImageID imageID);

#ifdef _MMC_SUPPORTED
    //!
    //! \brief  Decompress internal media memory 
    //! 
    //! \param  [in] mosCtx
    //!         Pointer to mos context
    //! \param  [in] osResource
    //!         Pointer mos resource
    //!
    static void MediaMemoryDecompressInternal(
        PMOS_CONTEXT  mosCtx,
        PMOS_RESOURCE osResource);
    
    //!
    //! \brief  copy internal media surface to another surface 
    //! 
    //! \param  [in] mosCtx
    //!         Pointer to mos context
    //! \param  [in] inputOsResource
    //!         Pointer input mos resource
    //! \param  [in] outputOsResource
    //!         Pointer output mos resource
    //! \param  [in] boutputcompressed
    //!         output can be compressed or not
    //!
    static void MediaMemoryCopyInternal(
        PMOS_CONTEXT  mosCtx, 
        PMOS_RESOURCE inputOsResource, 
        PMOS_RESOURCE outputOsResource, 
        bool          boutputcompressed);
    
    //!
    //! \brief  copy internal media surface/buffer to another surface/buffer 
    //! 
    //! \param  [in] mosCtx
    //!         Pointer to mos context
    //! \param  [in] inputOsResource
    //!         Pointer input mos resource
    //! \param  [in] outputOsResource
    //!         Pointer output mos resource
    //! \param  [in] boutputcompressed
    //!         output can be compressed or not
    //! \param  [in] copyWidth
    //!         The 2D surface Width
    //! \param  [in] copyHeight
    //!         The 2D surface height
    //! \param  [in] copyInputOffset
    //!         The offset of copied surface from
    //! \param  [in] copyOutputOffset
    //!         The offset of copied to
    //!
    static void MediaMemoryCopy2DInternal(
        PMOS_CONTEXT  mosCtx,
        PMOS_RESOURCE inputOsResource,
        PMOS_RESOURCE outputOsResource,
        uint32_t      copyWidth,
        uint32_t      copyHeight,
        uint32_t      copyInputOffset,
        uint32_t      copyOutputOffset,
        uint32_t      bpp,
        bool          boutputcompressed);

    //!
    //! \brief  Tile/Linear format conversion for media surface/buffer
    //!
    //! \param  [in] mosCtx
    //!         Pointer to mos context
    //! \param  [in] inputOsResource
    //!         Pointer input mos resource
    //! \param  [in] outputOsResource
    //!         Pointer output mos resource
    //! \param  [in] copyWidth
    //!         The 2D surface Width
    //! \param  [in] copyHeight
    //!         The 2D surface height
    //! \param  [in] copyInputOffset
    //!         The offset of copied surface from
    //! \param  [in] copyOutputOffset
    //!         The offset of copied to
    //! \param  [in] isTileToLinear
    //!         Convertion direction, true: tile->linear, false: linear->tile
    //! \param  [in] outputCompressed
    //!         output can be compressed or not
    //!
    static VAStatus MediaMemoryTileConvertInternal(
        PMOS_CONTEXT  mosCtx,
        PMOS_RESOURCE inputOsResource,
        PMOS_RESOURCE outputOsResource,
        uint32_t      copyWidth,
        uint32_t      copyHeight,
        uint32_t      copyInputOffset,
        uint32_t      copyOutputOffset,
        bool          isTileToLinear,
        bool          outputCompressed);

#endif

#if defined(X11_FOUND)
#define X11_LIB_NAME "libX11.so.6"
    //!
    //! \brief  Close opened libX11.so lib, free related function table.
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //!
    static void DestroyX11Connection(
        PDDI_MEDIA_CONTEXT mediaCtx);
   
    //!
    //! \brief  dlopen libX11.so, setup the function table
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //!
    static VAStatus ConnectX11(
        PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Output driver initialization
    //!
    //! \param  [in] ctx
    //!     Pointer to VA driver context
    //!
    //! \return bool
    //!     true if call success, else false
    //!
    static bool OutputDriInit(VADriverContextP ctx);

    //!
    //! \brief  Get dso symbols
    //!
    //! \param  [in] h
    //!     Pointer dso handle
    //! \param  [in] vtable
    //!     Pointer to VA driver table
    //! \param  [in] vtable_length
    //!     VA driver table length
    //! \param  [in] symbols
    //!     dso symbols
    //!
    //! \return bool
    //!     true if call success, else false
    //!
    static bool DsoGetSymbols(
        struct dso_handle          *h,
        void                       *vtable,
        uint32_t                   vtable_length,
        const struct dso_symbol    *symbols);

    //!
    //! \brief  Open dso
    //!
    //! \param  [in] path
    //!     dso path
    //!
    //! \return dso_handle
    //!     dso handle struct
    //!
    static struct dso_handle* DsoOpen(const char *path);

    //!
    //! \brief  Get symbol
    //!
    //! \param  [in] h
    //!     Pointer dso handle
    //! \param  [in] func_vptr
    //!     Pointer to function
    //! \param  [in] name
    //!     Functions name
    //!
    //! \return bool
    //!     true if call success, else false
    //!
    static bool GetSymbol(
        struct dso_handle *h,
        void              *func_vptr,
        const char        *name);
#endif

    //!
    //! \brief  Free allocated surfaceheap elements
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //!
    static void FreeSurfaceHeapElements(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Free allocated bufferheap elements
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //!
    static void FreeBufferHeapElements(VADriverContextP ctx);

    //!
    //! \brief  Free allocated Imageheap elements
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //!
    static void FreeImageHeapElements(VADriverContextP ctx);

    //!
    //! \brief  Free allocated contextheap elements
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //!
    static void FreeContextHeapElements(VADriverContextP ctx);

    //!
    //! \brief  Free allocated ContextCM elements
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //!
    static void FreeContextCMElements(VADriverContextP ctx);

    //!
    //! \brief  Free allocated heap elements
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus HeapDestroy(PDDI_MEDIA_CONTEXT mediaCtx);

    //!
    //! \brief  Execute free allocated bufferheap elements for FreeContextHeapElements function
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //! \param  [in] contextHeap
    //!         context heap
    //! \param  [in] vaContextOffset
    //!         context heap
    //! \param  [in] ctxNums
    //!         context numbers
    //!
    static void FreeContextHeap(
        VADriverContextP ctx,
        PDDI_MEDIA_HEAP  contextHeap,
        int32_t          vaContextOffset,
        int32_t          ctxNums);

    //!
    //! \brief  DestroyCMContext
    //!
    //! \param  [in] ctx
    //!         VA Driver Context
    //! \param  [in] ctxID
    //!         context ID
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus DestroyContextCM(
        VADriverContextP ctx,
        VAContextID      ctxID);

    //!
    //! \brief  Init SurfaceDescriptor Without AuxTableMgr
    //!
    //! \param  [out] desc
    //!         SurfaceDescriptor
    //! \param  [in] formats
    //!         DRM format
    //! \param  [in] compositeObject
    //!         compostie layer flag
    //! \param  [in] planesNum
    //!         planes num
    //! \param  [in] offsetY
    //!         Y channel offset
    //! \param  [in] offsetU
    //!         U channel offset
    //! \param  [in] offsetV
    //!         V channel offset
    //! \param  [in] pitch
    //!         surface pitch
    //! \param  [in] chromaPitch
    //!         chromaPitch pitch
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus InitSurfaceDescriptorWithoutAuxTableMgr(
        VADRMPRIMESurfaceDescriptor *desc,
        uint32_t                    *formats,
        int                         compositeObject,
        uint32_t                    planesNum,
        uint32_t                    offsetY,
        uint32_t                    offsetU,
        uint32_t                    offsetV,
        uint32_t                    pitch,
        uint32_t                    chromaPitch);

    //!
    //! \brief  Init SurfaceDescriptor With AuxTableMgr
    //!
    //! \param  [out] desc
    //!         SurfaceDescriptor
    //! \param  [in] formats
    //!         DRM format
    //! \param  [in] compositeObject
    //!         compostie layer flag
    //! \param  [in] planesNum
    //!         planes num
    //! \param  [in] offsetY
    //!         Y channel offset
    //! \param  [in] offsetU
    //!         U channel offset
    //! \param  [in] offsetv
    //!         U channel offset
    //! \param  [in] auxOffsetY
    //!         Y channel aux offset
    //! \param  [in] auxOffsetUV
    //!         UV channel aux offset
    //! \param  [in] pitch
    //!         surface pitch
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus InitSurfaceDescriptorWithAuxTableMgr(
        VADRMPRIMESurfaceDescriptor *desc,
        uint32_t                    *formats,
        int                         compositeObject,
        uint32_t                    planesNum,
        uint32_t                    offsetY,
        uint32_t                    offsetU,
        uint32_t                    offsetV,
        uint32_t                    auxOffsetY,
        uint32_t                    auxOffsetUV,
        int32_t                     pitch);

    //!
    //! \brief  Generate Vaimg From input Media format
    //!
    //! \param  [in] mediaSurface
    //!         Media surface
    //! \param  [in] mediaCtx
    //!         Media context
    //! \param  [out] vaimg
    //!         vaimg
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus GenerateVaImgFromMediaFormat(
        DDI_MEDIA_SURFACE  *mediaSurface,
        PDDI_MEDIA_CONTEXT mediaCtx,
        VAImage            *vaimg);

    //!
    //! \brief  Generate Vaimg From input OS format
    //!
    //! \param  [in] format
    //!         Os format type
    //! \param  [out] vaimg
    //!         vaimg
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus GenerateVaImgFromOsFormat(
        VAImageFormat      format,
        int32_t            width,
        int32_t            height,
        GMM_RESOURCE_INFO  *gmmResourceInfo,
        VAImage            *vaimg);

    //!
    //! \brief  Convert Rt format to OS format
    //!
    //! \param  [in] format
    //!         RT format type
    //! \param  [out] expectedFourcc
    //!         Os format type
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus RtFormatToOsFormat(uint32_t format, int32_t &expectedFourcc);

    //!
    //! \brief  Convert Os format to media format
    //!
    //! \param  [in] fourcc
    //!         FourCC
    //! \param  [in] rtformatType
    //!         Rt format type
    //!
    //! \return DDI_MEDIA_FORMAT
    //!     Ddi media format
    //!
    static DDI_MEDIA_FORMAT OsFormatToMediaFormat(int32_t fourcc, int32_t rtformatType);

    //!
    //! \brief  create render target
    //!
    //! \param  [in] mediaDrvCtx
    //!         Pointer to media context
    //! \param  [in] mediaFormat
    //!         media format
    //! \param  [in] width
    //!         width
    //! \param  [in] height
    //!         height
    //! \param  [in] surfDesc
    //!         media surface descriptor
    //! \param  [in] surfaceUsageHint
    //!         surface usage hint
    //! \param  [in] memType
    //!         memory type
    //! \return uint_32
    //!     surface ID
    //!
    static uint32_t CreateRenderTarget(
        PDDI_MEDIA_CONTEXT            mediaDrvCtx,
        DDI_MEDIA_FORMAT              mediaFormat,
        uint32_t                      width,
        uint32_t                      height,
        DDI_MEDIA_SURFACE_DESCRIPTOR  *surfDesc,
        uint32_t                      surfaceUsageHint,
        int                           memType);

public:
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
    static VAStatus MapBuffer(
        VADriverContextP  ctx,
        VABufferID        buf_id,
        void              **pbuf);

    //!
    //! \brief  Map data store of the buffer into the client's address space
    //!         vaCreateBuffer() needs to be called with "data" set to nullptr before calling vaMapBuffer()
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] bufId
    //!         VA buffer ID
    //! \param  [out] buf
    //!         Pointer to buffer
    //! \param  [in] flag
    //!         Flag
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus MapBufferInternal(
        VADriverContextP  ctx,
        VABufferID        bufId,
        void              **buf,
        uint32_t          flag);

    //! \brief  Unmap buffer
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] bufId
    //!         VA buffer ID
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus UnmapBuffer(
        VADriverContextP    ctx,
        VABufferID          bufId);

    //!
    //! \brief  Decompress a compressed surface.
    //!
    //! \param  [in]  mediaCtx
    //!         Pointer to ddi media context
    //! \param  [in]  mediaSurface
    //!         Ddi media surface
    //!
    //! \return     VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus MediaMemoryDecompress(
        PDDI_MEDIA_CONTEXT mediaCtx,
        DDI_MEDIA_SURFACE  *mediaSurface);

public:
    // Global mutex
    static MEDIA_MUTEX_T m_GlobalMutex;
MEDIA_CLASS_DEFINE_END(MediaLibvaInterfaceNext)
};

#endif //__MEDIA_LIBVA_INTERFACE_NEXT_H__