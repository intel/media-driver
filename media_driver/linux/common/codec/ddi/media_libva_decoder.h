/*
* Copyright (c) 2009-2020, Intel Corporation
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
#include "media_libva_cp_interface.h"
#include "media_ddi_decode_base.h"

#define DDI_DECODE_SFC_MAX_WIDTH                    4096
#define DDI_DECODE_SFC_MAX_HEIGHT                   4096
#define DDI_DECODE_SFC_MIN_WIDTH                    128
#define DDI_DECODE_SFC_MIN_HEIGHT                   128
#define DDI_DECODE_HCP_SFC_MAX_WIDTH                (16*1024)
#define DDI_DECODE_HCP_SFC_MAX_HEIGHT               (16*1024)

#if MOS_EVENT_TRACE_DUMP_SUPPORTED

#define MACROBLOCK_HEIGHT   16
#define MACROBLOCK_WIDTH    16

typedef struct _DECODE_EVENTDATA_INFO_PICTURE
{
    uint32_t CodecFormat;
    uint32_t FrameType;
    uint32_t PicStruct;
    uint32_t Width;
    uint32_t Height;
    uint32_t Bitdepth;
    uint32_t ChromaFormat;
    bool     EnabledSCC;
    bool     EnabledSegment;
    bool     EnabledFilmGrain;
} DECODE_EVENTDATA_INFO_PICTURE;

typedef struct _DECODE_EVENTDATA_INFO_PICTUREVA
{
    uint32_t CodecFormat;
    uint32_t FrameType;
    uint32_t PicStruct;
    uint32_t Width;
    uint32_t Height;
    uint32_t Bitdepth;
    uint32_t ChromaFormat;
    bool     EnabledSCC;
    bool     EnabledSegment;
    bool     EnabledFilmGrain;
} DECODE_EVENTDATA_INFO_PICTUREVA;

typedef struct _DECODE_EVENTDATA_VA_DISPLAYINFO
{
    uint32_t uiDisplayWidth;
    uint32_t uiDisplayHeight;
} DECODE_EVENTDATA_VA_DISPLAYINFO;

typedef struct _DECODE_EVENTDATA_VA_CREATEBUFFER
{
    VABufferType type;
    uint32_t size;
    uint32_t numElements;
    VABufferID *bufId;
} DECODE_EVENTDATA_VA_CREATEBUFFER;

typedef struct _DECODE_EVENTDATA_VA_BEGINPICTURE_START
{
    uint32_t FrameIndex;
} DECODE_EVENTDATA_VA_BEGINPICTURE_START;

typedef struct _DECODE_EVENTDATA_VA_BEGINPICTURE
{
    uint32_t FrameIndex;
    uint32_t hRes;
} DECODE_EVENTDATA_VA_BEGINPICTURE;

typedef struct _DECODE_EVENTDATA_VA_ENDPICTURE_START
{
    uint32_t FrameIndex;
} DECODE_EVENTDATA_VA_ENDPICTURE_START;

typedef struct _DECODE_VA_EVENTDATA_ENDPICTURE
{
    uint32_t FrameIndex;
    uint32_t hRes;
} DECODE_EVENTDATA_VA_ENDPICTURE;


typedef struct _DECODE_EVENTDATA_VA_RENDERPICTURE_START
{
    VABufferID *buffers;
} DECODE_EVENTDATA_VA_RENDERPICTURE_START;

typedef struct _DECODE_EVENTDATA_VA_RENDERPICTURE
{
    VABufferID *buffers;
    uint32_t hRes;
    uint32_t numBuffers;
} DECODE_EVENTDATA_VA_RENDERPICTURE;

typedef struct _DECODE_EVENTDATA_VA_CREATECONTEXT_START
{
    VABufferID configId;
} DECODE_EVENTDATA_VA_CREATECONTEXT_START;

typedef struct _DECODE_EVENTDATA_VA_CREATECONTEXT
{
    VABufferID configId;
    uint32_t hRes;
} DECODE_EVENTDATA_VA_CREATECONTEXT;

typedef struct _DECODE_EVENTDATA_VA_DESTROYCONTEXT_START
{
    VABufferID context;
} DECODE_EVENTDATA_VA_DESTROYCONTEXT_START;

typedef struct _DECODE_EVENTDATA_VA_DESTROYCONTEXT
{
    VABufferID context;
} DECODE_EVENTDATA_VA_DESTROYCONTEXT;

typedef struct _DECODE_EVENTDATA_VA_GETDECCTX
{
    uint32_t bufferID;
} DECODE_EVENTDATA_VA_GETDECCTX;

typedef struct _DECODE_EVENTDATA_VA_FREEBUFFERHEAPELEMENTS
{
    uint32_t bufNums;
} DECODE_EVENTDATA_VA_FREEBUFFERHEAPELEMENTS;

typedef struct _DECODE_EVENTDATA_VA_FEATURE_REPORTMODE
{
    uint32_t wMode;
    uint32_t ValueID;
} DECODE_EVENTDATA_VA_FEATURE_REPORTMODE;
#endif

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
class DdiDecodeBase;
class DdiVpFunctions;

//!
//! \struct DDI_DECODE_CONTEXT
//! \brief  Ddi decode context
//!
struct DDI_DECODE_CONTEXT
{
    // Decoder data related with the specific codec
    // The instance of DdiDecodeXXX. For example: DdiDecodeAvc, DdiDecodeJPEG
    DdiMediaDecode                  *m_ddiDecode;
    DdiDecodeBase                   *m_ddiDecodeNext;
    // Decoder private data
    DdiCpInterface                  *pCpDdiInterface;
    DdiVpFunctions                  *pVpDdiInterface;
    // Parameters
    CodechalDecodeParams            DecodeParams;
    uint16_t                        wMode;                  // Get the info during hand shaking
    Codechal                        *pCodecHal;
    bool                            bShortFormatInUse;
    bool                            bDecodeModeReported;
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
//! \brief  Status report
//!
//! \param  [in] decoder
//!     CodechalDecode decoder
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_StatusReport(
    PDDI_MEDIA_CONTEXT mediaCtx,
    CodechalDecode *decoder,
    DDI_MEDIA_SURFACE *surface);

//!
//! \brief  Status report
//!
//! \param  [in] decoder
//!     DecodePipelineAdapter decoder
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiDecode_StatusReport(
    PDDI_MEDIA_CONTEXT mediaCtx,
    DecodePipelineAdapter *decoder,
    DDI_MEDIA_SURFACE *surface);

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
VAStatus DdiDecode_SetGpuPriority(
    VADriverContextP     ctx,
    PDDI_DECODE_CONTEXT  decCtx,
    int32_t              priority
);

#endif
