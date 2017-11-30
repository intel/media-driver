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

#define CONVERT_TO_K(x, k)      (((x) + k - 1) / k)

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

#define FEI_ENC_BUFFER_TYPE_CTB_CMD       FEI_ENC_BUFFER_TYPE_MVDATA
#define FEI_ENC_BUFFER_TYPE_CU_RECORD     FEI_ENC_BUFFER_TYPE_MBCODE

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
    unsigned long                          ulHeadPosition;
    unsigned long                          ulUpdatePosition;
} DDI_ENCODE_STATUS_REPORT_INFO_BUF;

class DdiEncodeBase;

typedef struct _DDI_ENCODE_CONTEXT
{    
    DdiEncodeBase                    *m_encode;
    Codechal                         *pCodecHal;
    CODECHAL_MODE                     wModeType;
    CODECHAL_FUNCTION                 codecFunction;
    CODECHAL_FUNCTION                 feiFunction;
    VAProfile                         vaProfile;
    void                             *pSeqParams;
    void                             *pVuiParams;
    void                             *pPicParams;
    void                             *pSliceParams;
    void                             *pEncodeStatusReport;
    void                             *pQmatrixParams;
    void                             *pHuffmanParams;
    void                             *pJpegAppData;
    void                             *pFeiPicParams;
    void                             *pAvcQCParams;
    void                             *pAvcRoundingParams;
    void                             *pVpxSegParams;
    bool                              bMbDisableSkipMapEnabled;
    MOS_RESOURCE                      resPerMBSkipMapBuffer;
    void                             *pPreEncParams;
    MOS_RESOURCE                      resFeiMvPredictorBuffer;
    MOS_RESOURCE                      resFeiMbQpBuffer;
    MOS_RESOURCE                      resFeiDistortionBuffer;
    DDI_ENCODE_STATUS_REPORT_INFO_BUF statusReportBuf;
    MOS_RESOURCE                      resBitstreamBuffer;
    MOS_RESOURCE                      resMbCodeBuffer;
    MOS_RESOURCE                      resProbCoeffBuffer;
    MOS_SURFACE                       sCoeffSurface;
    MOS_RESOURCE                      resMBQpBuffer;
    //CP related
    DdiCpInterface                   *pCpDdiInterface;

    uint8_t                           WeightScale4x4[6][16];
    uint8_t                           WeightScale8x8[2][64];
    uint8_t                           bScalingLists4x4[6][16];
    uint8_t                           bScalingLists8x8[2][64];
    uint32_t                          indexNALUnit;
    uint8_t                           PicParamId;
    MOS_SURFACE                       segMapBuffer;
    bool                              bExtSatusReportEnable;// For VP8-F encoder now, but it can be used for any encoder to enable a Status report  using single-linked list via VAEncCodedBufferType
    bool                              bFirstFrame;
    uint32_t                          dwStoreData;
    uint8_t                           ucCurrPAKSet;
    uint8_t                           ucCurrENCSet;
    uint16_t                          wPicWidthInMB;          // Picture Width in MB width count
    uint16_t                          wPicHeightInMB;         // Picture Height in MB height count
    uint16_t                          wOriPicWidthInMB;
    uint16_t                          wOriPicHeightInMB;
    uint16_t                          wContextPicWidthInMB;
    uint16_t                          wContextPicHeightInMB;
    bool                              bMultiCallsPerPic;      // apply only for ENC only mode; app indicate at creat device time
    uint16_t                          usNumCallsPerPic;       // valid only when bMultiCallsPerPic is set; app indicate at creat device time
    uint32_t                          dwFrameWidth;           // Frame width in luma samples
    uint32_t                          dwFrameHeight;          // Frame height in luma samples
    uint32_t                          dwAppDataSize;
    PBSBuffer                         pbsBuffer;
    uint32_t                          dwNumSlices;
    bool                              bNewSeq;
    bool                              bPicQuant;
    bool                              bNewQmatrixData;
    bool                              bNewVuiData;
    bool                              bNewTimecodeMPEG2;     // This flag will only be asserted every time when parsing a new Seq Params for MPEG2 encode
    uint32_t                          uTimecode;             // Time code is only used by MPEG2 encoder for packing GOP header.
    uint32_t                          uFrameRate;

    uint32_t                          dwPicNum;
    PCODECHAL_NAL_UNIT_PARAMS        *ppNALUnitParams;
    PCODEC_ENCODER_SLCDATA            pSliceHeaderData;
    uint32_t                          uiSliceHeaderCnt;
    // For square region based rolling I
    uint32_t                          uiIntraRefreshFrameCnt;
    uint32_t                          uiIntraRefreshMBx;
    uint32_t                          uiIntraRefreshMBy;

    // whether Quant table is supplied by the app for JPEG encoder
    bool                              bJPEGQuantSupplied;
    
    // whether packed slice headers are passed from application;
    bool                              bHavePackedSliceHdr;

    // whether the latest packed header type is for slice header
    bool                              bLastPackedHdrIsSlice;
    
    // SEI stream passed by app
    PCODECHAL_ENCODE_SEI_DATA         pSEIFromApp;
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
    
    uint32_t                          ulMVOffset;

    void                             *pMpeg2UserDataListHead; 
    void                             *pMpeg2UserDataListTail;

    bool                              bIs10Bit;

} DDI_ENCODE_CONTEXT, *PDDI_ENCODE_CONTEXT;

typedef struct _DDI_ENCODE_MFE_CONTEXT
{
    std::vector<PDDI_ENCODE_CONTEXT> pDdiEncodeContexts;            // Container to keep sub contexts
    MEDIA_MUTEX_T                    encodeMfeMutex;
    uint32_t                         currentStreamId;               // Current allocated id, increased monotonically
    MfeSharedState                   *mfeEncodeSharedState;         // Keep shared state across sub contexts
}DDI_ENCODE_MFE_CONTEXT, *PDDI_ENCODE_MFE_CONTEXT;

static __inline PDDI_ENCODE_CONTEXT DdiEncode_GetEncContextFromPVOID (void *pEncCtx)
{
    return (PDDI_ENCODE_CONTEXT)pEncCtx;
}

PDDI_ENCODE_CONTEXT DdiEncode_GetEncContextFromContextID (VADriverContextP ctx, VAContextID vaCtxID);

VAStatus DdiEncode_RemoveFromStatusReportQueue(PDDI_ENCODE_CONTEXT  pEncCtx, PDDI_MEDIA_BUFFER pBuf);

VAStatus DdiEncode_RemoveFromEncStatusReportQueue(PDDI_ENCODE_CONTEXT pEncCtx, PDDI_MEDIA_BUFFER pBuf, DDI_ENCODE_FEI_ENC_BUFFER_TYPE idx);

VAStatus DdiEncode_RemoveFromPreEncStatusReportQueue(PDDI_ENCODE_CONTEXT pEncCtx, PDDI_MEDIA_BUFFER pBuf, DDI_ENCODE_PRE_ENC_BUFFER_TYPE idx);

PDDI_ENCODE_CONTEXT DdiEncode_GetEncContextFromContextID (VADriverContextP ctx, VAContextID vaCtxID);

bool DdiEncode_CodedBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT     pEncCtx,
    PDDI_MEDIA_BUFFER       pBuf);

bool DdiEncode_EncBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT             pEncCtx,
    PDDI_MEDIA_BUFFER               pBuf,
    DDI_ENCODE_FEI_ENC_BUFFER_TYPE  TypeIdx);

bool DdiEncode_PreEncBufferExistInStatusReport(
    PDDI_ENCODE_CONTEXT             pEncCtx,
    PDDI_MEDIA_BUFFER               pBuf,
    DDI_ENCODE_PRE_ENC_BUFFER_TYPE  TypeIdx);

VAStatus DdiEncode_EncStatusReport (
    PDDI_ENCODE_CONTEXT pEncCtx,
    DDI_MEDIA_BUFFER    *pMediaBuf,
    void                **pbuf
);

VAStatus DdiEncode_PreEncStatusReport (
    PDDI_ENCODE_CONTEXT pEncCtx,
    DDI_MEDIA_BUFFER    *pMediaBuf,
    void                **pbuf
);

VAStatus DdiEncode_StatusReport (
    PDDI_ENCODE_CONTEXT pEncCtx,
    DDI_MEDIA_BUFFER    *pMediaBuf,
    void                **pbuf
);

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

VAStatus DdiEncode_DestroyContext (
    VADriverContextP    ctx, 
    VAContextID         context);

VAStatus DdiEncode_CreateBuffer (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferType        type,
    uint32_t            size,
    uint32_t            num_elements,
    void               *data,
    VABufferID         *buf_id
);

VAStatus DdiEncode_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         render_target
);

VAStatus vDdiEncode_BeginPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VASurfaceID         render_target
);

VAStatus DdiEncode_RenderPicture (
    VADriverContextP    ctx,
    VAContextID         context,
    VABufferID         *buffers,
    int32_t             num_buffers
);

VAStatus DdiEncode_EndPicture(
    VADriverContextP    ctx,
    VAContextID         context);

VAStatus DdiEncode_MfeSubmit(
    VADriverContextP    ctx,
    VAMFContextID       mfe_context,
    VAContextID        *contexts,
    int32_t             num_contexts
);
#endif

