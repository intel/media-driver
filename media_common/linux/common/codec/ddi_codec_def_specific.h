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
//! \file     ddi_codec_def_specific.h
//! \brief    libva codec interface head file
//!

#ifndef __DDI_CODEC_DEF_SPECIFIC_H__
#define __DDI_CODEC_DEF_SPECIFIC_H__

#include "media_libva_common_next.h"

/* Number of forward references */
#define DDI_CODEC_NUM_FWD_REF         0
/* Number of backward references */
#define DDI_CODEC_NUM_BK_REF          0
/* Number of vp surface attributes */
#define DDI_CODEC_NUM_QUERY_ATTR_VP   9

#define DDI_CODEC_BITSTREAM_BUFFER_INDEX_BITS 4  //the bitstream buffer index is 4 bits length
#define DDI_CODEC_MAX_BITSTREAM_BUFFER_INDEX  0xF  // the maximum bitstream buffer index is 0xF
#define DDI_CODEC_INVALID_BUFFER_INDEX        -1
#define DDI_CODEC_MIN_VALUE_OF_MAX_BS_SIZE    10240

#define DDI_CODEC_FEI_MAX_INTERFACE_REVISION  1000
#define DDI_CODEC_FEI_CTB_CMD_SIZE_SKL        16
#define DDI_CODEC_FEI_CU_RECORD_SIZE_SKL_KBL  64
#define DDI_CODEC_STATS_MAX_NUM_PAST_REFS     1
#define DDI_CODEC_STATS_MAX_NUM_FUTURE_REFS   1
#define DDI_CODEC_STATS_MAX_NUM_OUTPUTS       3
#define DDI_CODEC_STATS_INTERLACED_SUPPORT    1

#define DDI_CODEC_NUM_MAX_REF_FRAME           16

#define DDI_CODEC_MAX_BITSTREAM_BUFFER        16
#define DDI_CODEC_MAX_BITSTREAM_BUFFER_MINUS1 (DDI_CODEC_MAX_BITSTREAM_BUFFER - 1)
#define DDI_CODEC_VP8_MAX_REF_FRAMES          5
#define DDI_CODEC_INVALID_FRAME_INDEX         0xffffffff

#define DDI_CODEC_NUM_MACROBLOCKS_WIDTH(dwWidth)     ((dwWidth + (CODECHAL_MACROBLOCK_WIDTH - 1)) / CODECHAL_MACROBLOCK_WIDTH)
#define DDI_CODEC_NUM_MACROBLOCKS_HEIGHT(dwHeight)   ((dwHeight + (CODECHAL_MACROBLOCK_HEIGHT - 1)) / CODECHAL_MACROBLOCK_HEIGHT)

typedef struct _DDI_CODEC_VC1BITPLANE_OBJECT
{
    uint8_t       *pBitPlaneBase;
    bool           bUsed;
} DDI_CODEC_VC1BITPLANE_OBJECT;

typedef struct _DDI_CODEC_RENDER_TARGET_TABLE
{
    int32_t                      iNumRenderTargets;
    DDI_MEDIA_SURFACE           *pCurrentRT;           // raw input for encode
    DDI_MEDIA_SURFACE           *pCurrentReconTarget;  // recon surface for encode
    DDI_MEDIA_SURFACE           *pRT[DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT];
    uint8_t                      ucRTFlag[DDI_MEDIA_MAX_SURFACE_NUMBER_CONTEXT];
} DDI_CODEC_RENDER_TARGET_TABLE, *PDDI_CODEC_RENDER_TARGET_TABLE;

typedef struct _DDI_CODEC_BITSTREAM_BUFFER_INFO
{
    uint8_t            *pBaseAddress; // For JPEG it is a memory address when allocate slice data from CPU.
    uint32_t            uiOffset;
    uint32_t            uiLength;
    VABufferID          vaBufferId;
    bool                bRendered; // whether this slice data will be rendered.
    PDDI_MEDIA_BUFFER   pMappedGPUBuffer; // the GPU mapping for this buffer.
    bool                bIsUseExtBuf;
    uint8_t            *pSliceBuf;
} DDI_CODEC_BITSTREAM_BUFFER_INFO;

typedef struct _DDI_CODEC_BUFFER_PARAM_H264
{
    // slice control buffer
    VASliceParameterBufferH264                  *pVASliceParaBufH264;
    VASliceParameterBufferBase                  *pVASliceParaBufH264Base;

    // one picture buffer
    VAPictureParameterBufferH264                 PicParam264;

    // one IQ buffer
    VAIQMatrixBufferH264                         IQm264;
} DDI_CODEC_BUFFER_PARAM_H264;

typedef struct _DDI_CODEC_BUFFER_PARAM_MPEG2
{
    // slice control buffer
    VASliceParameterBufferMPEG2                  *pVASliceParaBufMPEG2;

    // one picture buffer
    VAPictureParameterBufferMPEG2                 PicParamMPEG2;

    // one IQ buffer
    VAIQMatrixBufferMPEG2                         IQmMPEG2;
} DDI_CODEC_BUFFER_PARAM_MPEG2;

typedef struct _DDI_CODEC_BUFFER_PARAM_VC1
{
    // slice control buffer
    VASliceParameterBufferVC1                     *pVASliceParaBufVC1;

    // one picture buffer
    VAPictureParameterBufferVC1                   PicParamVC1;
    uint8_t                                      *pBitPlaneBuffer;
    DDI_MEDIA_BUFFER                             *pVC1BitPlaneBuffObject[DDI_CODEC_MAX_BITSTREAM_BUFFER];
    DDI_CODEC_VC1BITPLANE_OBJECT                  VC1BitPlane[DDI_CODEC_MAX_BITSTREAM_BUFFER];
    uint32_t                                      dwVC1BitPlaneIndex;
    MOS_RESOURCE                                  resBitPlaneBuffer;
} DDI_CODEC_BUFFER_PARAM_VC1;

typedef struct _DDI_CODEC_BUFFER_PARAM_JPEG
{
    // slice parameter buffer
    VASliceParameterBufferJPEGBaseline                    *pVASliceParaBufJPEG;

    // picture parameter buffer
    VAPictureParameterBufferJPEGBaseline                  PicParamJPEG;

    //IQ Matrix Buffer
    VAIQMatrixBufferJPEGBaseline                          IQmJPEG;
} DDI_CODEC_BUFFER_PARAM_JPEG;

typedef struct _DDI_CODEC_BUFFER_PARAM_VP8
{
    // slice control buffer
    VASliceParameterBufferVP8                   *pVASliceParaBufVP8;

    // one picture buffer
    VAPictureParameterBufferVP8                  PicParamVP8;

    // one IQ buffer
    VAIQMatrixBufferVP8                          IQmVP8;

    // Probability data
    DDI_MEDIA_BUFFER                            *pVP8ProbabilityDataBuffObject;
    MOS_RESOURCE                                 resProbabilityDataBuffer;
    uint8_t                                     *pProbabilityDataBase;
    VAProbabilityDataBufferVP8                   ProbabilityDataVP8;

    // Reference frames
    DDI_MEDIA_SURFACE                           *pReferenceFrames[DDI_CODEC_VP8_MAX_REF_FRAMES];
} DDI_CODEC_BUFFER_PARAM_VP8;

typedef struct _DDI_CODEC_BUFFER_PARAM_HEVC
{
    // slice control buffer
    VASliceParameterBufferHEVC                  *pVASliceParaBufHEVC;
    VASliceParameterBufferBase                  *pVASliceParaBufBaseHEVC;
    //slice control buffe for range extension
    VASliceParameterBufferHEVCExtension          *pVASliceParaBufHEVCRext;

    // one picture buffer
    VAPictureParameterBufferHEVC                 PicParamHEVC;

    //one picture buffer for range extension
    VAPictureParameterBufferHEVCExtension        PicParamHEVCRext;

    // one IQ buffer
    VAIQMatrixBufferHEVC                         IQmHEVC;
} DDI_CODEC_BUFFER_PARAM_HEVC;

typedef struct _DDI_CODEC_BUFFER_PARAM_VP9
{
    // one picture buffer
    VADecPictureParameterBufferVP9               PicParamVP9;

    // slice control buffer: 8 * sizeof(VASegmentParameterVP9)
    VASliceParameterBufferVP9                   *pVASliceParaBufVP9;
} DDI_CODEC_BUFFER_PARAM_VP9;

typedef struct _DDI_CODEC_COM_BUFFER_MGR
{
    // bitstream buffer
    DDI_MEDIA_BUFFER                            *pBitStreamBuffObject[DDI_CODEC_MAX_BITSTREAM_BUFFER];
    uint8_t                                     *pBitStreamBase[DDI_CODEC_MAX_BITSTREAM_BUFFER];
    uint32_t                                     dwBitstreamIndex;   //indicating which bitstream buffer is used now
    uint64_t                                     ui64BitstreamOrder; //save  bitstream buffer index used by previous 15 frames and current frame. the MSB is the oldest one, the LSB is current one.
    MOS_RESOURCE                                 resBitstreamBuffer;
    uint8_t                                     *pBitstreamBuffer;
    DDI_CODEC_BITSTREAM_BUFFER_INFO             *pSliceData;
    uint32_t                                     m_maxNumSliceData;
    uint32_t                                     dwNumSliceData;
    uint32_t                                     dwNumSliceControl;
    uint32_t                                     dwMaxBsSize;

    uint32_t                                     dwSizeOfRenderedSliceData; // Size of all the rendered slice data buffer
    uint32_t                                     dwNumOfRenderedSliceData; // how many slice data buffers will be rendered.
    uint32_t                                     dwNumOfRenderedSlicePara; // how many slice parameters buffers will be rendered.
    int32_t                                     *pNumOfRenderedSliceParaForOneBuffer; // how many slice headers in one slice parameter buffer.
    int32_t                                     *pRenderedOrder; // a array to keep record the sequence when slice data rendered.
    bool                                         bIsSliceOverSize;
    //decode parameters
    union
    {
        DDI_CODEC_BUFFER_PARAM_H264  Codec_Param_H264;
        DDI_CODEC_BUFFER_PARAM_MPEG2 Codec_Param_MPEG2;
        DDI_CODEC_BUFFER_PARAM_VC1   Codec_Param_VC1;
        DDI_CODEC_BUFFER_PARAM_JPEG  Codec_Param_JPEG;
        DDI_CODEC_BUFFER_PARAM_VP8   Codec_Param_VP8;
        DDI_CODEC_BUFFER_PARAM_HEVC  Codec_Param_HEVC;
        DDI_CODEC_BUFFER_PARAM_VP9   Codec_Param_VP9;
    } Codec_Param;

    uint32_t                                     dwEncodeNumSliceControl;

    void                                        *pHDCP2ParameterBuffer;

    VACodedBufferSegment                        *pCodedBufferSegment; // For bitstream output
    VAProcPipelineParameterBuffer                ProcPipelineParamBuffer;
    VAProcFilterParameterBuffer                  ProcFilterParamBuffer;
    VACodedBufferSegment                        *pCodedBufferSegmentForStatusReport; // for extended Status report such as long-term reference for VP8-F encode
    void                                        *pCodecParamReserved;
    void                                        *pCodecSlcParamReserved;

    // for External decode StreamOut Buffer
    MOS_RESOURCE                                 resExternalStreamOutBuffer;
} DDI_CODEC_COM_BUFFER_MGR;

#endif