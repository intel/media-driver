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
//! \file      media_libva_decoder.h
//! \brief     libva(and its extension) decoder head file
//!

#ifndef __MEDIA_LIBVA_DECODER_H__
#define __MEDIA_LIBVA_DECODER_H__

#include "media_libva.h"
#include "media_libva_cp.h"

#define DDI_DECODE_SFC_MAX_WIDTH                    4096
#define DDI_DECODE_SFC_MAX_HEIGHT                   4096
#define DDI_DECODE_SFC_MIN_WIDTH                    128
#define DDI_DECODE_SFC_MIN_HEIGHT                   128

//!
//! \struct DDI_DECODE_CONFIG_ATTR
//! \brief  Ddi decode configuration attribute
//!
struct DDI_DECODE_CONFIG_ATTR
{
    VAProfile           profile;
    VAEntrypoint        entrypoint;
    uint32_t            uiDecSliceMode;
    uint32_t            uiEncryptionType;
    uint32_t            uiDecProcessingType;
};

typedef struct DDI_DECODE_CONFIG_ATTR *PDDI_DECODE_CONFIG_ATTR;

class DdiMediaDecode;

//!
//! \struct DDI_DECODE_CONTEXT
//! \brief  Ddi decode context
//!
struct DDI_DECODE_CONTEXT
{
    // Decoder data related with the specific codec
    // The instance of DdiDecodeXXX. For example: DdiDecodeAvc, DdiDecodeJPEG
    DdiMediaDecode                  *m_ddiDecode;
    // Decoder private data
    DdiCpInterface                  *pCpDdiInterface;
    // Parameters
    CodechalDecodeParams            DecodeParams;
    uint16_t                        wMode;                  // Get the info during hand shaking    
    Codechal                        *pCodecHal;
    bool                            bShortFormatInUse;
    VASurfaceDecodeMBErrors         vaSurfDecErrOutput[2];
    DDI_CODEC_RENDER_TARGET_TABLE   RTtbl;
    DDI_CODEC_COM_BUFFER_MGR        BufMgr;
    PDDI_MEDIA_CONTEXT              pMediaCtx;
    // Add a list to track DPB.
    VASurfaceID                     RecListSurfaceID[CODEC_AVC_NUM_UNCOMPRESSED_SURFACE];
    uint32_t                        dwSliceParamBufNum;
    uint32_t                        dwSliceCtrlBufNum;
    uint32_t                        uiDecProcessingType;
};

typedef struct DDI_DECODE_CONTEXT *PDDI_DECODE_CONTEXT;

static __inline PDDI_DECODE_CONTEXT DdiDecode_GetDecContextFromPVOID (void *decCtx)
{
    return (PDDI_DECODE_CONTEXT)decCtx;
}

//!
//! \brief  Create buffer
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] decCtx
//!     Pointer to ddi decode context
//! \param  [in] type
//!     VA buffer type
//! \param  [in] size
//!     Size
//! \param  [in] numElements
//!     Number of elements
//! \param  [in] data
//!     DAta
//! \param  [in] bufId
//!     VA buffer ID
//!     
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_CreateBuffer(
    VADriverContextP         ctx,
    PDDI_DECODE_CONTEXT      decCtx,
    VABufferType             type,
    uint32_t                 size,
    uint32_t                 numElements,
    void                    *data,
    VABufferID              *bufId
);

//!
//! \brief  Begin picture
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//! \param  [in] renderTarget
//!     VA surface ID
//!     
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         renderTarget
);

//!
//! \brief  End picture
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//!     
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_EndPicture (
    VADriverContextP    ctx,
    VAContextID         context
);

//!
//! \brief  Render picture
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//! \param  [in] buffers
//!     VA buffer ID
//! \param  [in] numBuffers
//!     Number of buffers
//!     
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_RenderPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID         *buffers,
    int32_t             numBuffers
);

//!
//! \brief  Create context
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] configId
//!     VA configuration ID
//! \param  [in] pictureWidth
//!     The width of picture
//! \param  [in] pictureHeight
//!     The height of picture
//! \param  [in] flag
//!     Flag
//! \param  [in] renderTargets
//!     VA surface ID
//! \param  [in] numRenderTargets
//!     Number of render targets
//! \param  [in] context
//!     VA context ID
//!     
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_CreateContext (
    VADriverContextP    ctx,
    VAConfigID          configId,
    int32_t             pictureWidth,
    int32_t             pictureHeight,
    int32_t             flag,
    VASurfaceID        *renderTargets,
    int32_t             numRenderTargets,
    VAContextID        *context
);

//!
//! \brief  Destroy context
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//!     
//! \return     VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_DestroyContext (
    VADriverContextP    ctx,
    VAContextID         context
);

#endif
