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
//! \file      media_libva_encoder.h
//! \brief     libva(and its extension) encoder head file
//!

#ifndef  __MEDIA_LIBVA_ENCODER_H__
#define  __MEDIA_LIBVA_ENCODER_H__

#include "media_libva.h"
#include "media_libva_cp.h"
#include <vector>

// change to 0x1000 for memory optimization, double check when implement slice header packing in app
#define PACKED_HEADER_SIZE_PER_ROW      0x1000

#define DDI_ENCODE_MAX_STATUS_REPORT_BUFFER    CODECHAL_ENCODE_STATUS_NUM

typedef enum _DDI_ENCODE_FEI_ENC_BUFFER_TYPE
{
    FEI_ENC_BUFFER_TYPE_MVDATA     = 0,
    FEI_ENC_BUFFER_TYPE_MBCODE     = 1,
    FEI_ENC_BUFFER_TYPE_DISTORTION = 2,
    FEI_ENC_BUFFER_TYPE_MAX        = 3
} DDI_ENCODE_FEI_ENC_BUFFER_TYPE;

typedef enum _DDI_ENCODE_PRE_ENC_BUFFER_TYPE
{
    PRE_ENC_BUFFER_TYPE_MVDATA     = 0,
    PRE_ENC_BUFFER_TYPE_STATS      = 1,
    PRE_ENC_BUFFER_TYPE_STATS_BOT  = 2,
    PRE_ENC_BUFFER_TYPE_MAX        = 3
} DDI_ENCODE_PRE_ENC_BUFFER_TYPE;

typedef struct _DDI_ENCODE_STATUS_REPORT_INFO
{
    void           *pCodedBuf;              //encoded buffer address
    uint32_t        uiSize;                 //encoded frame size
    uint32_t        uiStatus;               // Encode frame status
    uint32_t        uiInputCtr[4];          // Counter for HDCP2 session
} DDI_ENCODE_STATUS_REPORT_INFO;

// ENC output buffer checking for FEI_ENC case only
typedef struct _DDI_ENCODE_STATUS_REPORT_ENC_INFO
{
    void           *pEncBuf[3];             // ENC buffers address for Mvdata, MbCode and Distortion
    uint32_t        uiBuffers;              // rendered ENC buffers
    uint32_t        uiStatus;               // ENC frame status
} DDI_ENCODE_STATUS_REPORT_ENC_INFO;

// PREENC output buffer checking
typedef struct _DDI_ENCODE_STATUS_REPORT_PREENC_INFO
{
    void           *pPreEncBuf[3];          // PREENC buffers address for Mvdata and Statistics, Statistics of Bottom Field
    uint32_t        uiBuffers;              // rendered ENC buffers
    uint32_t        uiStatus;               // PREENC frame status
} DDI_ENCODE_STATUS_REPORT_PREENC_INFO;

typedef struct _DDI_ENCODE_STATUS_REPORT_INFO_BUF
{
    DDI_ENCODE_STATUS_REPORT_INFO          infos[DDI_ENCODE_MAX_STATUS_REPORT_BUFFER];
    DDI_ENCODE_STATUS_REPORT_ENC_INFO      encInfos[DDI_ENCODE_MAX_STATUS_REPORT_BUFFER];
    DDI_ENCODE_STATUS_REPORT_PREENC_INFO   preencInfos[DDI_ENCODE_MAX_STATUS_REPORT_BUFFER];
    uint32_t                               ulHeadPosition;
    uint32_t                               ulUpdatePosition;
} DDI_ENCODE_STATUS_REPORT_INFO_BUF;

class DdiEncodeBase;

typedef struct _DDI_ENCODE_CONTEXT
{
    DdiEncodeBase                    *m_encode;
    Codechal                         *pCodecHal;
    CODECHAL_MODE                     wModeType;
    CODECHAL_FUNCTION                 codecFunction;
    VAProfile                         vaProfile;
    VAEntrypoint                      vaEntrypoint;
    void                             *pSeqParams;
    void                             *pVuiParams;
    void                             *pPicParams;
    void                             *pSliceParams;
    void                             *pEncodeStatusReport;
    void                             *pQmatrixParams;
    void                             *pFeiPicParams;
    void                             *pVpxSegParams;
    bool                              bMbDisableSkipMapEnabled;
    MOS_RESOURCE                      resPerMBSkipMapBuffer;
    void                             *pPreEncParams;
    DDI_ENCODE_STATUS_REPORT_INFO_BUF statusReportBuf;
    MOS_RESOURCE                      resBitstreamBuffer;
    MOS_RESOURCE                      resMbCodeBuffer;
    MOS_RESOURCE                      resProbCoeffBuffer;
    MOS_SURFACE                       sCoeffSurface;
    MOS_RESOURCE                      resMBQpBuffer;
    //CP related
    DdiCpInterface                   *pCpDdiInterface;

    uint32_t                          indexNALUnit;
    uint8_t                           PicParamId;
    MOS_SURFACE                       segMapBuffer;
    uint16_t                          wPicWidthInMB;          // Picture Width in MB width count
    uint16_t                          wPicHeightInMB;         // Picture Height in MB height count
    uint16_t                          wOriPicWidthInMB;
    uint16_t                          wOriPicHeightInMB;
    uint16_t                          wContextPicWidthInMB;
    uint16_t                          wContextPicHeightInMB;
    uint32_t                          dwFrameWidth;           // Frame width in luma samples
    uint32_t                          dwFrameHeight;          // Frame height in luma samples
    PBSBuffer                         pbsBuffer;
    uint32_t                          dwNumSlices;
    bool                              bNewSeq;
    bool                              bPicQuant;
    bool                              bNewQmatrixData;
    bool                              bNewVuiData;
    uint32_t                          uFrameRate;

    PCODECHAL_NAL_UNIT_PARAMS        *ppNALUnitParams;
    PCODEC_ENCODER_SLCDATA            pSliceHeaderData;
    uint32_t                          uiSliceHeaderCnt;
    // For square region based rolling I
    uint32_t                          uiIntraRefreshFrameCnt;
    uint32_t                          uiIntraRefreshMBx;
    uint32_t                          uiIntraRefreshMBy;

    // whether packed slice headers are passed from application;
    bool                              bHavePackedSliceHdr;

    // whether the latest packed header type is for slice header
    bool                              bLastPackedHdrIsSlice;

    // SEI stream passed by app
    CodechalEncodeSeiData            *pSEIFromApp;
    uint32_t                          uiRCMethod;

    uint32_t                          uiTargetBitRate;
    uint32_t                          uiMaxBitRate;

    EncoderParams                     EncodeParams;

    //VDENC Enable flag
    bool                              bVdencActive;
    //VDENC Dynamic slice enabling
    bool                              EnableSliceLevelRateCtrl;
    //Per-MB Qp control
    bool                              bMBQpEnable;

    DDI_CODEC_RENDER_TARGET_TABLE     RTtbl;
    DDI_CODEC_COM_BUFFER_MGR          BufMgr;
    PDDI_MEDIA_CONTEXT                pMediaCtx;

    uint8_t                           targetUsage;

} DDI_ENCODE_CONTEXT, *PDDI_ENCODE_CONTEXT;

typedef struct _DDI_ENCODE_MFE_CONTEXT
{
    std::vector<PDDI_ENCODE_CONTEXT> pDdiEncodeContexts;            // Container to keep sub contexts
    MEDIA_MUTEX_T                    encodeMfeMutex;
    uint32_t                         currentStreamId;               // Current allocated id, increased monotonically
    MfeSharedState                   *mfeEncodeSharedState;         // Keep shared state across sub contexts
    bool                             isFEI;                         // Support legacy only or FEI only
}DDI_ENCODE_MFE_CONTEXT, *PDDI_ENCODE_MFE_CONTEXT;

static __inline PDDI_ENCODE_CONTEXT DdiEncode_GetEncContextFromPVOID (void *encCtx)
{
    return (PDDI_ENCODE_CONTEXT)encCtx;
}

//!
//! \brief  Get encode context from context ID
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] vaCtxID
//!     VA context ID
//!
//! \return PDDI_ENCODE_CONTEXT
//!     Pointer to ddi encode context
//!
PDDI_ENCODE_CONTEXT DdiEncode_GetEncContextFromContextID (VADriverContextP ctx, VAContextID vaCtxID);

//!
//! \brief  Remove from status report queue
//!
//! \param  [in] encCtx
//!     Pointer to ddi encode context
//! \param  [in] buf
//!     Pointer to ddi media buffer
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiEncode_RemoveFromStatusReportQueue(PDDI_ENCODE_CONTEXT  encCtx, PDDI_MEDIA_BUFFER buf);

//!
//! \brief  Remove from encode status report queue
//!
//! \param  [in] encCtx
//!     Pointer to ddi encode context
//! \param  [in] buf
//!     Pointer to ddi media buffer
//! \param  [in] idx
//!     Ddi encode FEI encode buffer type
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiEncode_RemoveFromEncStatusReportQueue(PDDI_ENCODE_CONTEXT encCtx, PDDI_MEDIA_BUFFER buf, DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx);

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
VAStatus DdiEncode_RemoveFromPreEncStatusReportQueue(PDDI_ENCODE_CONTEXT encCtx, PDDI_MEDIA_BUFFER buf, DDI_ENCODE_PRE_ENC_BUFFER_TYPE idx);

//!
//! \brief  Get encode context from context ID
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] vaCtxID
//!     VA context ID
//!
//! \return PDDI_ENCODE_CONTEXT
//!     Pointer to ddi encode context
//!
PDDI_ENCODE_CONTEXT DdiEncode_GetEncContextFromContextID (VADriverContextP ctx, VAContextID vaCtxID);

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
bool DdiEncode_CodedBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT     encCtx,
    PDDI_MEDIA_BUFFER       buf);

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
bool DdiEncode_EncBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT             encCtx,
    PDDI_MEDIA_BUFFER               buf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE  typeIdx);

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
bool DdiEncode_PreEncBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT             encCtx,
    PDDI_MEDIA_BUFFER               buf,
    DDI_ENCODE_PRE_ENC_BUFFER_TYPE  typeIdx);

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
VAStatus DdiEncode_EncStatusReport (
    PDDI_ENCODE_CONTEXT encCtx,
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **pbuf
);

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
VAStatus DdiEncode_PreEncStatusReport (
    PDDI_ENCODE_CONTEXT encCtx,
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **pbuf
);

//!
//! \brief  Status report
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
VAStatus DdiEncode_StatusReport (
    PDDI_ENCODE_CONTEXT encCtx,
    DDI_MEDIA_BUFFER    *mediaBuf,
    void                **pbuf
);

//!
//! \brief  Create context
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] config_id
//!     VA configuration ID
//! \param  [in] picture_width
//!     The width of picture
//! \param  [in] picture_height
//!     The height of picture
//! \param  [in] flag
//!     Flag
//! \param  [in] render_targets
//!     Render targets
//! \param  [in] num_render_targets
//!     Number of render targets
//! \param  [in] context
//!     VA context ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiEncode_CreateContext (
    VADriverContextP    ctx,
    VAConfigID          config_id,
    int32_t             picture_width,
    int32_t             picture_height,
    int32_t             flag,
    VASurfaceID        *render_targets,
    int32_t             num_render_targets,
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
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiEncode_DestroyContext (
    VADriverContextP    ctx,
    VAContextID         context);

//!
//! \brief  Create buffer
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//! \param  [in] type
//!     VA buffer type
//! \param  [in] size
//!     Size
//! \param  [in] num_elements
//!     Number of elements
//! \param  [in] data
//!     Data
//! \param  [in] buf_id
//!     VA buffer ID 
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiEncode_CreateBuffer (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferType        type,
    uint32_t            size,
    uint32_t            num_elements,
    void               *data,
    VABufferID         *buf_id
);

//!
//! \brief  Begin picture
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//! \param  [in] render_target
//!     VA surface ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiEncode_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         render_target
);

//!
//! \brief  Begin Picture
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//! \param  [in] render_target
//!     VA surface ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus vDdiEncode_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         render_target
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
//! \param  [in] num_buffers
//!     Number of buffers
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiEncode_RenderPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID         *buffers,
    int32_t             num_buffers
);

//!
//! \brief  End picture
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] context
//!     VA context ID
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiEncode_EndPicture(
    VADriverContextP    ctx,
    VAContextID         context);

//!
//! \brief  MFE submit
//!
//! \param  [in] ctx
//!     Pointer to VA driver context
//! \param  [in] mfe_context
//!     VA MF context ID
//! \param  [in] contexts
//!     VA context ID
//! \param  [in] num_contexts
//!     Number of contexts
//!
//! \return VAStatus
//!     VA_STATUS_SUCCESS if success, else fail reason
//!
VAStatus DdiEncode_MfeSubmit(
    VADriverContextP    ctx,
    VAMFContextID       mfe_context,
    VAContextID        *contexts,
    int32_t             num_contexts
);
#endif
