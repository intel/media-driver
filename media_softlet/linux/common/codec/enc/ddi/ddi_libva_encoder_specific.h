/*
* Copyright (c) 2022, Intel Corporation
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
//! \file      ddi_libva_encoder_specific.h
//! \brief     libva(and its extension) encoder head file
//!

#ifndef  __DDI_LIBVA_ENCODER_SPECIFIC_H__
#define  __DDI_LIBVA_ENCODER_SPECIFIC_H__

#include "media_libva.h"
#include "ddi_cp_interface_next.h"
#include "media_libva_cp_interface.h"
#include <vector>
namespace encode
{
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
    DdiCpInterfaceNext               *pCpDdiInterfaceNext;

    uint32_t                          indexNALUnit;
    uint8_t                           PicParamId;
    MOS_SURFACE                       segMapBuffer;
    uint32_t                          segmentMapDataSize;
    uint8_t                          *pSegmentMap;
    uint16_t                          wPicWidthInMB;          // Picture Width in MB width count
    uint16_t                          wPicHeightInMB;         // Picture Height in MB height count
    uint16_t                          wOriPicWidthInMB;
    uint16_t                          wOriPicHeightInMB;
    uint16_t                          wContextPicWidthInMB;
    uint16_t                          wContextPicHeightInMB;
    uint32_t                          dworiFrameWidth;        // Original Frame width in luma samples
    uint32_t                          dworiFrameHeight;       // Original Frame height in luma samples
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

static __inline PDDI_ENCODE_CONTEXT GetEncContextFromPVOID (void *encCtx)
{
    return (PDDI_ENCODE_CONTEXT)encCtx;
}

}  // namespace encode
#endif // __DDI_LIBVA_ENCODER_SPECiFIC_H__
