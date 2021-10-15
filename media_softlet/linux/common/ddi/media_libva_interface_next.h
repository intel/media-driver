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
#include "media_libva_common_next.h"

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
        VAProfile         *profile_list,
        int32_t           *num_profiles);

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

private:
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
    static VAStatus DestroyContextCM(
        VADriverContextP ctx,
        VAContextID      ctxID);

public:
    // Global mutex
    static MEDIA_MUTEX_T m_GlobalMutex;
};

#endif //__MEDIA_LIBVA_INTERFACE_NEXT_H__