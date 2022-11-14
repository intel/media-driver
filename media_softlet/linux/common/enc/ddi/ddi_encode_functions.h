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
//! \file     ddi_encode_functions.h
//! \brief    ddi encode functions head file
//!

#ifndef __DDI_ENCODE_FUNCTIONS_H__
#define __DDI_ENCODE_FUNCTIONS_H__

#include "ddi_media_functions.h"
#include "media_libva_caps_next.h"
#include "ddi_libva_encoder_specific.h"

class DdiEncodeFunctions :public DdiMediaFunctions
{
public:

    virtual ~DdiEncodeFunctions() override{};

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
        VADriverContextP  ctx,
        VAProfile         profile,
        VAEntrypoint      entrypoint,
        VAConfigAttrib   *attribList,
        int32_t           numAttribs,
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
    virtual VAStatus CreateContext (
        VADriverContextP  ctx,
        VAConfigID        configId,
        int32_t           pictureWidth,
        int32_t           pictureHeight,
        int32_t           flag,
        VASurfaceID       *renderTargets,
        int32_t           renderTargetsNum,
        VAContextID       *context
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
    virtual VAStatus DestroyContext (
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
    virtual VAStatus CreateBuffer (
        VADriverContextP  ctx,
        VAContextID       context,
        VABufferType      type,
        uint32_t          size,
        uint32_t          elementsNum,
        void              *data,
        VABufferID        *bufId
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
        DDI_MEDIA_CONTEXT   *mediaCtx,
        VABufferID          buf_id,
        void                **pbuf,
        uint32_t            flag
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
    virtual VAStatus UnmapBuffer (
        DDI_MEDIA_CONTEXT   *mediaCtx,
        VABufferID          buf_id
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
        DDI_MEDIA_CONTEXT  *mediaCtx,
        VABufferID         buffer_id
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
    virtual VAStatus BeginPicture (
        VADriverContextP  ctx,
        VAContextID       context,
        VASurfaceID       renderTarget
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
    virtual VAStatus RenderPicture (
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
    virtual VAStatus EndPicture (
        VADriverContextP  ctx,
        VAContextID       context
    ) override;

    //!
    //! \brief  Clean and free encode context structure
    //!
    //! \param  [in] encCtx
    //!     Pointer to ddi encode context
    //!
    void CleanUp(encode::PDDI_ENCODE_CONTEXT encCtx);

    //!
    //! \brief  Set Encode Gpu Priority
    //!
    //! \param  [in] encode context
    //!     Pointer to encode context
    //! \param  [in] priority
    //!     priority
    //! \return VAStatus
    //!
    VAStatus SetGpuPriority(
        encode::PDDI_ENCODE_CONTEXT encCtx,
        int32_t             priority
    );

    //!
    //! \brief  Coded buffer exist in status report
    //!
    //! \param  [in] encCtx
    //!     Pointer to ddi encode context
    //! \param  [in] buf
    //!     Pointer to ddi media buffer
    //!
    //! \return bool
    //!     true if call success, else false
    //!
    bool CodedBufferExistInStatusReport(
        encode::PDDI_ENCODE_CONTEXT     encCtx,
        PDDI_MEDIA_BUFFER       buf);

    //!
    //! \brief  Status report
    //!
    //! \param  [in] encCtx
    //!     Pointer to ddi encode context
    //! \param  [in] mediaBuf
    //!     Ddi media buffer
    //! \param  [out] buf
    //!     Pointer buffer
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus StatusReport (
        encode::PDDI_ENCODE_CONTEXT encCtx,
        DDI_MEDIA_BUFFER    *mediaBuf,
        void                **buf
    );

    //!
    //! \brief  Pre encode buffer exist in status report
    //!
    //! \param  [in] encCtx
    //!     Pointer to ddi encode context
    //! \param  [in] buf
    //!     Pointer to ddi media buffer
    //! \param  [in] typeIdx
    //!     Ddi encode PRE encode buffer type
    //!
    //! \return bool
    //!     true if call success, else false
    //!
    bool PreEncBufferExistInStatusReport(
        encode::PDDI_ENCODE_CONTEXT            encCtx,
        PDDI_MEDIA_BUFFER              buf,
        encode::DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx);

    //!
    //! \brief  Pre encode status report
    //!
    //! \param  [in] encCtx
    //!     Pointer to ddi encode context
    //! \param  [in] mediaBuf
    //!     Ddi media buffer
    //! \param  [out] pbuf
    //!     Pointer buffer
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus PreEncStatusReport(
        encode::PDDI_ENCODE_CONTEXT encCtx,
        DDI_MEDIA_BUFFER* mediaBuf,
        void** buf);

    //!
    //! \brief  Encode buffer exist in status report
    //!
    //! \param  [in] encCtx
    //!     Pointer to ddi encode context
    //! \param  [in] buf
    //!     Pointer to ddi media buffer
    //! \param  [in] typeIdx
    //!     Ddi encode FEI encode buffer type
    //!
    //! \return bool
    //!     true if call success, else false
    //!
    bool EncBufferExistInStatusReport(
        encode::PDDI_ENCODE_CONTEXT            encCtx,
        PDDI_MEDIA_BUFFER              buf,
        encode::DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx);

    //!
    //! \brief  Encode status report
    //!
    //! \param  [in] encCtx
    //!     Pointer to ddi encode context
    //! \param  [in] mediaBuf
    //!     Ddi media buffer
    //! \param  [out] pbuf
    //!     Pointer buffer
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus EncStatusReport (
        encode::PDDI_ENCODE_CONTEXT encCtx,
        DDI_MEDIA_BUFFER    *mediaBuf,
        void                **pbuf
    );

    //!
    //! \brief  Remove form preencode status report queue
    //!
    //! \param  [in] encCtx
    //!     Pointer to ddi encode context
    //! \param  [in] buf
    //!     Pointer to ddi media buffer
    //! \param  [in] idx
    //!     Ddi encode PRE encode buffer type
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus RemoveFromPreEncStatusReportQueue(
        encode::PDDI_ENCODE_CONTEXT            encCtx,
        PDDI_MEDIA_BUFFER              buf,
        encode::DDI_ENCODE_PRE_ENC_BUFFER_TYPE typeIdx);

    //!
    //! \brief  Remove form preencode status report queue
    //!
    //! \param  [in] encCtx
    //!     Pointer to ddi encode context
    //! \param  [in] buf
    //!     Pointer to ddi media buffer
    //! \param  [in] idx
    //!     Ddi encode PRE encode buffer type
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    VAStatus RemoveFromEncStatusReportQueue(
        encode::PDDI_ENCODE_CONTEXT            encCtx,
        PDDI_MEDIA_BUFFER              buf,
        encode::DDI_ENCODE_FEI_ENC_BUFFER_TYPE typeIdx);

MEDIA_CLASS_DEFINE_END(DdiEncodeFunctions)
};

#endif //__DDI_ENCODE_FUNCTIONS_H__