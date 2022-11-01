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
//! \file     ddi_media_functions.h
//! \brief    ddi media functions head file
//!

#ifndef __DDI_MEDIA_FUNCTIONS_H__
#define __DDI_MEDIA_FUNCTIONS_H__

#include <stdint.h>
#include <va/va_version.h>
#include <va/va.h>
#include <va/va_backend.h>
#include "media_factory.h"
#include "media_class_trace.h"

#define DDI_VP_NUM_INPUT_COLOR_STD          6                 /* Number of supported input color formats */
#define DDI_VP_NUM_OUT_COLOR_STD            6                 /* Number of supported output color formats*/
#define DDI_CODEC_NUM_FWD_REF               0                 /* Number of forward references */
#define DDI_CODEC_NUM_BK_REF                0                 /* Number of backward references */
typedef struct DDI_MEDIA_CONTEXT *PDDI_MEDIA_CONTEXT;
typedef struct _DDI_MEDIA_SURFACE DDI_MEDIA_SURFACE;

class DdiMediaFunctions
{
public:

    virtual ~DdiMediaFunctions(){};

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
    virtual VAStatus CreateContext (
        VADriverContextP  ctx,
        VAConfigID        configId,
        int32_t           pictureWidth,
        int32_t           pictureHeight,
        int32_t           flag,
        VASurfaceID       *renderTargets,
        int32_t           renderTargetsNum,
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
    virtual VAStatus DestroyContext (
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
    virtual VAStatus CreateBuffer (
        VADriverContextP  ctx,
        VAContextID       context,
        VABufferType      type,
        uint32_t          size,
        uint32_t          elementsNum,
        void              *data,
        VABufferID        *bufId
    );

    //!
    //! \brief  Map data store of the buffer into the client's address space
    //!         vaCreateBuffer() needs to be called with "data" set to nullptr before
    //!         calling vaMapBuffer()
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
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
    virtual VAStatus MapBufferInternal(
        DDI_MEDIA_CONTEXT   *mediaCtx,
        VABufferID          bufId,
        void                **buf,
        uint32_t            flag
    );

    //! \brief  Unmap buffer
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //! \param  [in] bufId
    //!         VA buffer ID
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus UnmapBuffer (
        DDI_MEDIA_CONTEXT   *mediaCtx,
        VABufferID          bufId
    );

    //!
    //! \brief  Destroy buffer
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //! \param  [in] bufId
    //!         VA buffer ID
    //!
    //! \return     VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus DestroyBuffer(
        DDI_MEDIA_CONTEXT  *mediaCtx,
        VABufferID         bufId
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
    virtual VAStatus BeginPicture (
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
    virtual VAStatus RenderPicture (
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
    virtual VAStatus EndPicture (
        VADriverContextP  ctx,
        VAContextID       context
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
    //! \param  [out] attribList
    //!         VA attrib list
    //! \param  [out] attribsNum
    //!         Number of attribs
    //! \param  [out] configId
    //!         VA config id
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus CreateConfig (
        VADriverContextP  ctx,
        VAProfile         profile,
        VAEntrypoint      entrypoint,
        VAConfigAttrib    *attriblist,
        int32_t           attribsNum,
        VAConfigID        *configId
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
    //! \param  [in] filtersNum
    //!         Number of filters
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus QueryVideoProcFilters (
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
    virtual VAStatus QueryVideoProcFilterCaps (
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
    virtual VAStatus QueryVideoProcPipelineCaps (
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
    virtual VAStatus CreateProtectedSession (
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
    virtual VAStatus DestroyProtectedSession (
        VADriverContextP      ctx,
        VAProtectedSessionID  protecteSsession
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
    virtual VAStatus AttachProtectedSession (
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
    virtual VAStatus DetachProtectedSession (
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
    virtual VAStatus ProtectedSessionExecute (
        VADriverContextP      ctx,
        VAProtectedSessionID  protectedSession,
        VABufferID            data
    );

#endif

    //!
    //! \brief   Status check after SyncSurface2
    //!
    //! \param   [in] mediaCtx
    //!          Pointer to media driver context
    //! \param   [in] surface
    //!          DDI MEDIA SURFACE
    //! \param   [in] VASurfaceID
    //!          surface id
    //!
    //! \return  VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus StatusCheck(
        PDDI_MEDIA_CONTEXT mediaCtx,
        DDI_MEDIA_SURFACE  *surface,
        VASurfaceID        surfaceId
    );

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
    virtual VAStatus QuerySurfaceError(
        VADriverContextP ctx,
        VASurfaceID      renderTarget,
        VAStatus         errorStatus,
        void             **errorInfo
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
    //! \param  numberCliprects
    //!     Number of clip rects in the clip list
    //! \param  flags
    //!     De-interlacing flags
    //!
    //! \return     VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus PutSurface(
        VADriverContextP ctx,
        VASurfaceID      surface,
        void             *draw,             /* Drawable of window system */
        int16_t          srcx,
        int16_t          srcy,
        uint16_t         srcw,
        uint16_t         srch,
        int16_t          destx,
        int16_t          desty,
        uint16_t         destw,
        uint16_t         desth,
        VARectangle      *cliprects,        /* client supplied clip list */
        uint32_t         numberCliprects,  /* number of clip rects in the clip list */
        uint32_t         flags             /* de-interlacing flags */
    );
    
    //! \brief  Ddi process pipeline
    //!
    //! \param  pVaDrvCtx
    //!     Pointer to VA driver context
    //! \param  ctxID
    //!     VA context ID
    //! \param  srcSurface
    //!     VA src surface ID
    //! \param  srcRect
    //!     VA src rect
    //! \param  dstSurface
    //!     VA dst surface ID
    //! \param  dstRect
    //!     VA dst rect
    //!
    //! \return     VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus ProcessPipeline(
        VADriverContextP    vaDrvCtx,
        VAContextID         ctxID,
        VASurfaceID         srcSurface,
        VARectangle         *srcRect,
        VASurfaceID         dstSurface,
        VARectangle         *dstRect
    );

protected:
    static const VAProcColorStandardType   m_vpInputColorStd[DDI_VP_NUM_INPUT_COLOR_STD];
    static const VAProcColorStandardType   m_vpOutputColorStd[DDI_VP_NUM_OUT_COLOR_STD];

MEDIA_CLASS_DEFINE_END(DdiMediaFunctions)
};

enum CompType
{
    CompCommon = 0,
    CompCodec  = 1,
    CompEncode = 2,
    CompDecode = 3,
    CompVp     = 4,
    CompCp     = 5,
    CompCount
};

typedef MediaFactory<CompType, DdiMediaFunctions> FunctionsFactory;

#endif //__DDI_MEDIA_FUNCTIONS_H__
