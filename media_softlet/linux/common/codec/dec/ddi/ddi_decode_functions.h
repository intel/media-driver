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
//! \file     ddi_decode_functions.h
//! \brief    ddi decode functions head file
//!

#ifndef __DDI_DECODE_FUNCTIONS_H__
#define __DDI_DECODE_FUNCTIONS_H__

#include "ddi_media_functions.h"
#include "ddi_libva_decoder_specific.h"
#include "media_libva_caps_next.h"
#include "decode_pipeline_adapter.h"

using namespace decode;

class DdiDecodeFunctions :public DdiMediaFunctions
{
public:

    virtual ~DdiDecodeFunctions() override{};

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
    virtual VAStatus CreateConfig(
        VADriverContextP ctx,
        VAProfile        profile,
        VAEntrypoint     entrypoint,
        VAConfigAttrib   *attribList,
        int32_t          numAttribs,
        VAConfigID       *configId
    ) override;

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
    virtual VAStatus CreateContext(
        VADriverContextP ctx,
        VAConfigID       configId,
        int32_t          pictureWidth,
        int32_t          pictureHeight,
        int32_t          flag,
        VASurfaceID      *renderTargets,
        int32_t          renderTargetsNum,
        VAContextID      *context
    ) override;

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
    virtual VAStatus DestroyContext(
        VADriverContextP  ctx,
        VAContextID       context
    ) override;

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
    virtual VAStatus CreateBuffer(
        VADriverContextP  ctx,
        VAContextID       context,
        VABufferType      type,
        uint32_t          size,
        uint32_t          elementsNum,
        void              *data,
        VABufferID        *bufId
    ) override;

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
    virtual VAStatus BeginPicture(
        VADriverContextP ctx,
        VAContextID      context,
        VASurfaceID      renderTarget
    ) override;

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
    virtual VAStatus RenderPicture(
        VADriverContextP  ctx,
        VAContextID       context,
        VABufferID        *buffers,
        int32_t           buffersNum
    ) override;

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
    virtual VAStatus EndPicture(
        VADriverContextP ctx,
        VAContextID      context
    ) override;

    //!
    //! \brief  Clean and free decode context structure
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] decCtx
    //!     Pointer to ddi decode context
    //!
    void CleanUp(
        VADriverContextP    ctx,
        PDDI_DECODE_CONTEXT decCtx
    );

    //!
    //! \brief  Set Decode Gpu priority
    //!
    //! \param  [in] ctx
    //!     Pointer to VA driver context
    //! \param  [in] decode context
    //!     Pointer to decode context
    //! \param  [in] priority
    //!     priority
    //! \return VAStatus
    //!
    VAStatus SetGpuPriority(
        VADriverContextP    ctx,
        PDDI_DECODE_CONTEXT decCtx,
        int32_t             priority
    );

    //!
    //! \brief  Query video proc pipeline caps when decode + sfc
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
    VAStatus QueryVideoProcPipelineCaps(
        VADriverContextP   ctx,
        VAContextID        context,
        VABufferID         *filters,
        uint32_t           filtersNum,
        VAProcPipelineCaps *pipelineCaps
    ) override;

    //!
    //! \brief  Map data store of the buffer into the client's address space
    //!         vaCreateBuffer() needs to be called with "data" set to nullptr before
    //!         calling vaMapBuffer()
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //! \param  [in] buf_id
    //!         VA buffer ID
    //! \param  [out] pbuf
    //!         Pointer to buffer
    //! \param  [in] flag
    //!         Flag
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus MapBufferInternal(
        DDI_MEDIA_CONTEXT *mediaCtx,
        VABufferID        buf_id,
        void              **pbuf,
        uint32_t          flag
    ) override;

    //!
    //! \brief  Unmap buffer
    //!
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] buf_id
    //!         VA buffer ID
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus UnmapBuffer(
        DDI_MEDIA_CONTEXT *mediaCtx,
        VABufferID        buf_id
    ) override;

    //!
    //! \brief  Destroy buffer
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //! \param  [in] buffer_id
    //!         VA buffer ID
    //!
    //! \return     VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    virtual VAStatus DestroyBuffer(
        DDI_MEDIA_CONTEXT *mediaCtx,
        VABufferID        buffer_id
    ) override;

    //!
    //! \brief  Check status
    //!
    //! \param  [in] mediaCtx
    //!         Pointer to media context
    //! \param  [in] surface
    //!         Pointer to media surface
    //! \param  [in] surfaceId
    //!         Surface ID
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus StatusCheck(
        PDDI_MEDIA_CONTEXT mediaCtx,
        DDI_MEDIA_SURFACE  *surface,
        VASurfaceID        surfaceId
    ) override;

    //!
    //! \brief  Status report
    //!
    //! \param  [in] decoder
    //!     DecodePipelineAdapter decoder
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus StatusReport(
        PDDI_MEDIA_CONTEXT    mediaCtx,
        DecodePipelineAdapter *decoder,
        DDI_MEDIA_SURFACE     *surface);

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
    VAStatus QuerySurfaceError(
        VADriverContextP ctx,
        VASurfaceID      renderTarget,
        VAStatus         errorStatus,
        void             **errorInfo
    ) override;

private:
    int32_t GetDisplayInfo(VADriverContextP ctx);

    void FreeBufferHeapElements(VADriverContextP ctx, PDDI_DECODE_CONTEXT decCtx);

    bool ReleaseBsBuffer(DDI_CODEC_COM_BUFFER_MGR *bufMgr, DDI_MEDIA_BUFFER *buf);

    bool ReleaseBpBuffer(DDI_CODEC_COM_BUFFER_MGR *bufMgr, DDI_MEDIA_BUFFER *buf);

    bool ReleaseSliceControlBuffer(uint32_t ctxType, void *ctx, DDI_MEDIA_BUFFER *buf);

MEDIA_CLASS_DEFINE_END(DdiDecodeFunctions)
};

#endif //__DDI_DECODE_FUNCTIONS_H__
