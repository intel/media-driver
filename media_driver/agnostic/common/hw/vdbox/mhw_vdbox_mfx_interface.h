/*
* Copyright (c) 2017-2018, Intel Corporation
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

//! \file     mhw_vdbox_mfx_interface.h
//! \brief    MHW interface for constructing MFX commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox MFX commands across all platforms
//!

#ifndef _MHW_VDBOX_MFX_INTERFACE_H_
#define _MHW_VDBOX_MFX_INTERFACE_H_

#include "mhw_vdbox.h"
#include "mhw_mi.h"
#include "codec_def_common_encode.h"
#include "codec_def_encode_vp9.h"
#include "codec_def_encode_vp8.h"
#include "codec_def_encode_hevc.h"
#include "codec_def_decode_mpeg2.h"
#include "codec_def_decode_vp8.h"
#include "codec_def_decode_jpeg.h"
#include "codec_def_vp8_probs.h"

#define MFX_PAK_STREAMOUT_DATA_BYTE_SIZE   4

typedef enum _MHW_VDBOX_AVC_DMV_OFFSET
{
    MHW_VDBOX_AVC_DMV_DEST_TOP = 32,
    MHW_VDBOX_AVC_DMV_DEST_BOTTOM = 33
} MHW_VDBOX_AVC_DMV_OFFSET;

typedef enum _MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_LUMA
{
    MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_LUMA_X0Y0 = 0x0f0c0300,
    MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_LUMA_X1Y0 = 0x0f0f0303,
    MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_LUMA_X0Y1 = 0x0f0c0f0c,
    MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_LUMA_X1Y1 = 0x0f0f0f0f,

} MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_LUMA;

typedef enum _MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_CHROMA
{
    MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_CHROMA_X0Y0 = 0x00000000,
    MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_CHROMA_X1Y0 = 0x00000303,
    MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_CHROMA_X0Y1 = 0x00000c0c,
    MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_CHROMA_X1Y1 = 0x00000f0f,

} MHW_VDBOX_DECODE_VC1_IT_ILDB_EDGE_CONTROL_CHROMA;

typedef enum _MHW_VDBOX_DECODE_JPEG_FORMAT_CODE
{
    MHW_VDBOX_DECODE_JPEG_FORMAT_SEPARATE_PLANE = 0, // formats of 3 separate plane for Y, U, and V respectively
    MHW_VDBOX_DECODE_JPEG_FORMAT_NV12 = 1,
    MHW_VDBOX_DECODE_JPEG_FORMAT_UYVY = 2,
    MHW_VDBOX_DECODE_JPEG_FORMAT_YUY2 = 3
} MHW_VDBOX_DECODE_JPEG_FORMAT_CODE;



typedef struct _MHW_VDBOX_MPEG2_SLICE_STATE
{
    PMOS_RESOURCE                   presDataBuffer;
    PMOS_RESOURCE                   presPicHeaderBBSurf; // BRC only
    uint32_t                        dwOffset;
    uint32_t                        dwLength;
    uint32_t                        dwSliceStartMbOffset;
    uint16_t                        wPicWidthInMb;
    uint16_t                        wPicHeightInMb;
    bool                            bLastSlice;

    // Decoding Only
    CodecDecodeMpeg2SliceParams    *pMpeg2SliceParams;

    // Encoding Only
    PCODEC_PIC_ID                   pMpeg2PicIdx;
    PCODEC_REF_LIST                *ppMpeg2RefList;
    CodecEncodeMpeg2SequenceParams *pEncodeMpeg2SeqParams;
    CodecEncodeMpeg2PictureParams  *pEncodeMpeg2PicParams;
    CodecEncodeMpeg2SliceParmas    *pEncodeMpeg2SliceParams;
    PBSBuffer                       pBsBuffer;
    PCODECHAL_NAL_UNIT_PARAMS      *ppNalUnitParams;
    PCODEC_ENCODER_SLCDATA          pSlcData;
    bool                            bFirstPass;
    bool                            bLastPass;
    uint32_t                        dwSliceIndex;
    uint32_t                        dwDataBufferOffset;
    bool                            bBrcEnabled;
    bool                            bRCPanicEnable;
    bool                            bInsertBeforeSliceHeaders;

} MHW_VDBOX_MPEG2_SLICE_STATE, *PMHW_VDBOX_MPEG2_SLICE_STATE;

typedef struct _MHW_VDBOX_MPEG2_MB_STATE
{
    CodecDecodeMpeg2MbParmas       *pMBParams;
    uint16_t                        wPicWidthInMb;
    uint16_t                        wPicHeightInMb;
    uint16_t                        wPicCodingType;
    uint32_t                        dwDCTLength;
    uint32_t                        dwITCoffDataAddrOffset;
    int16_t                         sPackedMVs0[4];
    int16_t                         sPackedMVs1[4];
} MHW_VDBOX_MPEG2_MB_STATE, *PMHW_VDBOX_MPEG2_MB_STATE;

typedef struct _MHW_VDBOX_VC1_PRED_PIPE_PARAMS
{
    PCODEC_VC1_PIC_PARAMS       pVc1PicParams;
    PCODEC_REF_LIST            *ppVc1RefList;
} MHW_VDBOX_VC1_PRED_PIPE_PARAMS, *PMHW_VDBOX_VC1_PRED_PIPE_PARAMS;

typedef struct _MHW_VDBOX_VC1_PIC_STATE
{
    PCODEC_VC1_PIC_PARAMS       pVc1PicParams;
    uint32_t                    Mode;
    PCODEC_REF_LIST            *ppVc1RefList;
    uint16_t                    wPrevAnchorPictureTFF;
    bool                        bPrevEvenAnchorPictureIsP;
    bool                        bPrevOddAnchorPictureIsP;
} MHW_VDBOX_VC1_PIC_STATE, *PMHW_VDBOX_VC1_PIC_STATE;

typedef struct _MHW_VDBOX_VC1_DIRECTMODE_PARAMS
{
    PMOS_RESOURCE                   presDmvReadBuffer;
    PMOS_RESOURCE                   presDmvWriteBuffer;
} MHW_VDBOX_VC1_DIRECTMODE_PARAMS, *PMHW_VDBOX_VC1_DIRECTMODE_PARAMS;

typedef struct _MHW_VDBOX_VC1_SLICE_STATE
{
    PCODEC_VC1_SLICE_PARAMS         pSlc;
    PMOS_RESOURCE                   presDataBuffer;
    uint32_t                        dwOffset;
    uint32_t                        dwLength;
    uint32_t                        dwNextVerticalPosition;
    bool                            bShortFormatInUse;
} MHW_VDBOX_VC1_SLICE_STATE, *PMHW_VDBOX_VC1_SLICE_STATE;

typedef struct _MHW_VDBOX_VC1_MB_STATE
{
    PCODEC_VC1_MB_PARAMS            pMb;
    PCODEC_VC1_PIC_PARAMS           pVc1PicParams;
    MEDIA_WA_TABLE                  *pWaTable;                               // WA table

    PMOS_RESOURCE                   presDataBuffer;
    uint8_t                        *pDeblockDataBuffer;
    uint32_t                        dwDataSize;
    uint32_t                        dwOffset;
    uint32_t                        dwLength;
    uint8_t                         bMbHorizOrigin;
    uint8_t                         bMbVertOrigin;
    uint8_t                         DeblockData[CODEC_NUM_BLOCK_PER_MB];
    uint32_t                        PackedLumaMvs[4];                       // packed motion vectors for Luma
    uint32_t                        PackedChromaMv;                         // packed motion vector for Chroma
    uint16_t                        wPicWidthInMb;                          // Picture Width in MB width count
    uint16_t                        wPicHeightInMb;                         // Picture Height in MB height count
    CODEC_PICTURE_FLAG              PicFlags;
    uint8_t                         bMotionSwitch;                          // VC1 MotionSwitch
    uint8_t                         bFieldPolarity;
    uint8_t                         bSkipped;
} MHW_VDBOX_VC1_MB_STATE, *PMHW_VDBOX_VC1_MB_STATE;

typedef struct _MHW_VDBOX_HUFF_TABLE_PARAMS
{
    uint32_t    HuffTableID;
    void       *pDCBits;
    void       *pDCValues;
    void       *pACBits;
    void       *pACValues;
} MHW_VDBOX_HUFF_TABLE_PARAMS, *PMHW_VDBOX_HUFF_TABLE_PARAMS;

typedef struct _MHW_VDBOX_ENCODE_HUFF_TABLE_PARAMS
{
    uint32_t    HuffTableID;
    uint8_t     pDCCodeLength[JPEG_NUM_HUFF_TABLE_DC_HUFFVAL]; // 12 values of 1 byte each
    uint16_t    pDCCodeValues[JPEG_NUM_HUFF_TABLE_DC_HUFFVAL]; // 12 values of 2 bytes each
    uint8_t     pACCodeLength[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL]; // 162 values of 1 byte each
    uint16_t    pACCodeValues[JPEG_NUM_HUFF_TABLE_AC_HUFFVAL]; // 162 values of 2 bytes each
} MHW_VDBOX_ENCODE_HUFF_TABLE_PARAMS, *PMHW_VDBOX_ENCODE_HUFF_TABLE_PARAMS;

typedef struct _MHW_VDBOX_JPEG_BSD_PARAMS
{
    uint32_t    dwIndirectDataLength;
    uint32_t    dwDataStartAddress;
    uint32_t    dwScanHorizontalPosition;
    uint32_t    dwScanVerticalPosition;
    bool        bInterleaved;
    int16_t     sScanComponent;
    uint32_t    dwMCUCount;
    uint32_t    dwRestartInterval;
} MHW_VDBOX_JPEG_BSD_PARAMS, *PMHW_VDBOX_JPEG_BSD_PARAMS;

typedef struct _MHW_VDBOX_JPEG_PIC_STATE
{
    CodecDecodeJpegPicParams  *pJpegPicParams;
    uint32_t                   Mode;
    uint32_t                   dwWidthInBlocks;
    uint32_t                   dwHeightInBlocks;
    uint32_t                   dwOutputFormat;
} MHW_VDBOX_JPEG_DECODE_PIC_STATE, *PMHW_VDBOX_JPEG_PIC_STATE;

//!
//! \struct    MhwVdboxJpegEncodePicState
//! \brief     MHW vdbox JPEG encode picture state
//!
struct MhwVdboxJpegEncodePicState
{
    CodecEncodeJpegPictureParams           *pJpegEncodePicParams;
    uint32_t                                mode;
};

//!
//! \struct    MhwVdboxJpegScanParams
//! \brief     MHW vdbox JPEG scan parameters
//!
struct MhwVdboxJpegScanParams
{
    CodecEncodeJpegScanHeader              *pJpegEncodeScanParams;
    CodecEncodeJpegInputSurfaceFormat       inputSurfaceFormat;
    uint32_t                                dwPicWidth;
    uint32_t                                dwPicHeight;
    uint32_t                                mode;
};

typedef struct _MHW_VDBOX_VP8_PIC_STATE
{
    PCODEC_VP8_PIC_PARAMS           pVp8PicParams;
    PCODEC_VP8_IQ_MATRIX_PARAMS     pVp8IqMatrixParams;
    PMOS_RESOURCE                   presCoefProbBuffer;
    PMOS_RESOURCE                   presSegmentationIdStreamBuffer;
    uint32_t                        dwCoefProbTableOffset;

    // Encoding Params
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS       pEncodeVP8SeqParams;
    PCODEC_VP8_ENCODE_PIC_PARAMS            pEncodeVP8PicParams;
    PCODEC_VP8_ENCODE_QUANT_DATA            pEncodeVP8QuantData;
    uint16_t                                wPicWidthInMb;
    uint16_t                                wPicHeightInMb;
    uint8_t                                 ucKernelMode;     // normal, performance, quality.

} MHW_VDBOX_VP8_PIC_STATE, *PMHW_VDBOX_VP8_PIC_STATE;

typedef struct _MHW_VDBOX_VP8_BSP_BUF_BASE_ADDR_PARAMS
{
    PMOS_RESOURCE               presFrameHeaderBuffer;
    PMOS_RESOURCE               presCoeffProbsBuffer;
    PMOS_RESOURCE               presPakIntermediateBuffer;
    PMOS_RESOURCE               presPakFinalFrameBuffer;
    PMOS_RESOURCE               presTokenStatisticsBuffer;
    PMOS_RESOURCE               presBsdMpcRowStoreScratchBuffer;
    uint32_t                    dwPakIntermediateTokenSize;
    uint32_t                    dwPakIntermediatePartition0Size;
    uint32_t                    dwPartitions;
} MHW_VDBOX_VP8_BSP_BUF_BASE_ADDR_PARAMS, *PMHW_VDBOX_VP8_BSP_BUF_BASE_ADDR_PARAMS;

typedef struct _MHW_VDBOX_VP8_ENCODER_CFG_PARAMS
{
    bool                                    bFirstPass;
    bool                                    bBRCEnabled;
    uint32_t                                dwCfgBufferSize;               // total buffer size
    uint32_t                                dwCfgCmdOffset;                // offset of the CFG command in
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS       pEncodeVP8SeqParams;
    PCODEC_VP8_ENCODE_PIC_PARAMS            pEncodeVP8PicParams;
    PCODEC_VP8_ENCODE_QUANT_DATA            pEncodeVP8QuantData;
} MHW_VDBOX_VP8_ENCODER_CFG_PARAMS, *PMHW_VDBOX_VP8_ENCODER_CFG_PARAMS;

typedef struct _MHW_VDBOX_VP8_BSD_PARAMS
{
    PCODEC_VP8_PIC_PARAMS       pVp8PicParams;
} MHW_VDBOX_VP8_BSD_PARAMS, *PMHW_VDBOX_VP8_BSD_PARAMS;

typedef struct _MHW_VDBOX_MPEG2_PIC_STATE
{
    uint32_t                    Mode;

    // Decoding Params
    CodecDecodeMpeg2PicParams  *pMpeg2PicParams;
    bool                        bDeblockingEnabled;
    uint32_t                    dwMPEG2ISliceConcealmentMode;           // Mpeg2 I slice concealment mode
    uint32_t                    dwMPEG2PBSliceConcealmentMode;          // Mpeg2 P/B slice concealment mode
    uint32_t                    dwMPEG2PBSlicePredBiDirMVTypeOverride;  // Mpeg2 P/B Slice Predicted BiDir Motion Type Override
    uint32_t                    dwMPEG2PBSlicePredMVOverride;           // Mpeg2 P/B Slice Predicted Motion Vector Override

                                                                        // Encoding Params
    CodecEncodeMpeg2SequenceParams         *pEncodeMpeg2SeqParams;
    CodecEncodeMpeg2PictureParams          *pEncodeMpeg2PicParams;
    PCODEC_REF_LIST                        *ppRefList;
    bool                                    bBrcEnabled;
    bool                                    bTrellisQuantEnable;
    uint16_t                                wPicWidthInMb;
    uint16_t                                wPicHeightInMb;
    uint8_t                                 ucKernelMode;     // normal, performance, quality.
    bool                                    bIPCMPass;

    uint32_t                                dwFrameBitRateMaxReportMask;
    uint32_t                                dwIntraMBMaxSize;
    uint32_t                                dwInterMBMaxSize;

    uint8_t                                 ucSliceDeltaQPMax0;
    uint8_t                                 ucSliceDeltaQPMax1;
    uint8_t                                 ucSliceDeltaQPMax2;
    uint8_t                                 ucSliceDeltaQPMax3;

    uint32_t                                dwFrameBitRateMax;
    uint32_t                                dwFrameBitRateMaxUnit;
} MHW_VDBOX_MPEG2_PIC_STATE, *PMHW_VDBOX_MPEG2_PIC_STATE;

typedef struct _MHW_VDBOX_AVC_DPB_PARAMS
{
    PCODEC_AVC_PIC_PARAMS                   pAvcPicParams;
    PCODEC_MVC_EXT_PIC_PARAMS               pMvcExtPicParams;
    PCODEC_REF_LIST                        *ppAvcRefList;
    PCODEC_PIC_ID                           pAvcPicIdx;
    bool                                    bPicIdRemappingInUse;
} MHW_VDBOX_AVC_DPB_PARAMS, *PMHW_VDBOX_AVC_DPB_PARAMS;

typedef struct _MHW_VDBOX_AVC_DIRECTMODE_PARAMS
{
    CODEC_PICTURE                   CurrPic;
    bool                            isEncode;
    uint32_t                        uiUsedForReferenceFlags;
    PMOS_RESOURCE                   presAvcDmvBuffers;
    uint8_t                         ucAvcDmvIdx;
    PCODEC_AVC_DMV_LIST             pAvcDmvList;
    PCODEC_PIC_ID                   pAvcPicIdx;
    void                            **avcRefList;
    bool                            bPicIdRemappingInUse;
    int32_t                         CurrFieldOrderCnt[2];
    bool                            bDisableDmvBuffers;
    PMOS_RESOURCE                   presMvcDummyDmvBuffer;
} MHW_VDBOX_AVC_DIRECTMODE_PARAMS, *PMHW_VDBOX_AVC_DIRECTMODE_PARAMS;

typedef struct _MHW_VDBOX_AVC_REF_IDX_PARAMS
{
    CODEC_PICTURE                   CurrPic               = {};
    uint32_t                        uiNumRefForList[2]    = {};
    CODEC_PICTURE                   RefPicList[2][32]     = {};
    PCODEC_PIC_ID                   pAvcPicIdx            = {};
    uint32_t                        uiList                = 0;
    void                            **avcRefList          = nullptr;
    bool                            isEncode              = false;
    bool                            bVdencInUse           = false;
    bool                            bIntelEntrypointInUse = false;
    bool                            bPicIdRemappingInUse  = false;
    bool                            oneOnOneMapping       = false;
    bool                            bDummyReference       = false;
} MHW_VDBOX_AVC_REF_IDX_PARAMS, *PMHW_VDBOX_AVC_REF_IDX_PARAMS;

typedef struct _MHW_VDBOX_PIC_ID_PARAMS
{
    bool                    bPicIdRemappingInUse;
    PCODEC_PIC_ID           pAvcPicIdx;
} MHW_VDBOX_PIC_ID_PARAMS, *PMHW_VDBOX_PIC_ID_PARAMS;

typedef struct _MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS
{
    PMOS_RESOURCE               presBsdMpcRowStoreScratchBuffer;                 // Handle of BSD/MPC Row Store Scratch data surface
    PMOS_RESOURCE               presMprRowStoreScratchBuffer;                    // Handle of MPR Row Store Scratch data surface
    PMOS_RESOURCE               presBitplaneBuffer;
} MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS, *PMHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS;

typedef struct _MHW_VDBOX_GPUNODE_LIMIT
{
    bool       bHcpInUse;
    bool       bHuCInUse;
    bool       bSfcInUse;
    uint32_t   dwGpuNodeToUse;
}MHW_VDBOX_GPUNODE_LIMIT, *PMHW_VDBOX_GPUNODE_LIMIT;

typedef struct _MHW_VDBOX_AVC_IMG_BITRATE_PARAMS
{
    uint32_t   frameBitRateMin : 14;
    uint32_t   frameBitRateMinUnitMode : 1;
    uint32_t   frameBitRateMinUnit : 1;
    uint32_t   frameBitRateMax : 14;
    uint32_t   frameBitRateMaxUnitMode : 1;
    uint32_t   frameBitRateMaxUnit : 1;
    uint32_t   frameBitRateMinDelta : 15;
    uint32_t   : 1;
    uint32_t   frameBitRateMaxDelta : 15;
    uint32_t   : 1;
} MHW_VDBOX_AVC_IMG_BITRATE_PARAMS, *PMHW_VDBOX_AVC_IMG_BITRATE_PARAMS;

//!
//! \class    MhwVdboxMfxInterface
//! \brief    MHW Vdbox Mfx interface
//!
/*!
This class defines the interfaces for constructing Vdbox Mfx commands across all platforms
*/
class MhwVdboxMfxInterface
{
protected:
    static const uint8_t  m_mpeg2QuantMatrixScan[64]; //!< MPEG2 quantization table scan order

    //!
    //! \enum     AvcSliceType
    //! \brief    Average slice type
    //!
    enum AvcSliceType
    {
        avcSliceP = 0,
        avcSliceB = 1,
        avcSliceI = 2
    };

    //!
    //! \enum     Mpeg2Vc1MacroblockIntratype
    //! \brief    MPEG2 VC1 macro block intra type
    //!
    enum Mpeg2Vc1MacroblockIntratype
    {
        mpeg2Vc1MacroblockNonintra  = 0,
        mpeg2Vc1MacroblockIntra     = 1
    };

    //!
    //! \enum     Mpeg2Vc1PictureStructure
    //! \brief    MPEG2 VC1 picture structure
    //!
    enum Mpeg2Vc1PictureStructure
    {
        mpeg2Vc1TopField = 1,
        mpeg2Vc1BottomField,
        mpeg2Vc1Frame
    };

    //!
    //! \enum     Vc1FrameCodingMode
    //! \brief    VC1 frame coding mode
    //!
    enum Vc1FrameCodingMode
    {
        vc1ProgressiveFrame = 0,
        vc1InterlacedFrame,
        vc1TffFrame,
        vc1BffFrame
    };

    //!
    //! \enum     Vc1CodedMode
    //! \brief    VC1 coded mode
    //!
    enum Vc1CodedMode
    {
        vc1NonrawMode = 0,
        vc1RawMode = 1
    };

    //!
    //! \struct   RefBoundaryReplicationMode
    //! \brief    Reference boundary replication mode
    //!
    struct RefBoundaryReplicationMode
    {
        union
        {
            struct
            {
                uint8_t ref0            : 1;
                uint8_t ref1            : 1;
                uint8_t ref2            : 1;
                uint8_t ref3            : 1;
            };
            struct
            {
                uint8_t value;
            };
        } BY0;
    };

    //!
    //! \enum     Vc1FrameBoundaryType
    //! \brief    VC1 frame boundary type
    //!
    enum Vc1FrameBoundaryType
    {
        vc1ProgressiveBoundary = 0,
        vc1InterlacedBoundary  = 1
    };

    //!
    //! \struct   AvcRefListWrite
    //! \brief    Average reference list write
    //!
    struct AvcRefListWrite
    {
        union
        {
            struct
            {
                uint8_t bottomField     : 1;
                uint8_t frameStoreID    : 4;
                uint8_t fieldPicFlag    : 1;
                uint8_t longTermFlag    : 1;
                uint8_t nonExisting     : 1;
            };
            struct
            {
                uint8_t value;
            };
        } UC[32];
    };

    //!
    //! \enum     AvcQmTypes
    //! \brief    Average qm types
    //!
    enum AvcQmTypes
    {
        avcQmIntra4x4 = 0,
        avcQmInter4x4 = 1,
        avcQmIntra8x8 = 2,
        avcQmInter8x8 = 3
    };

    //!
    //! \enum     Mpeg2QmTypes
    //! \brief    MPEG2 qm types
    //!
    enum Mpeg2QmTypes
    {
        mpeg2QmIntra = 0,
        mpeg2QmNonIntra,
    };

    //!
    //! \enum     AvcPicid
    //! \brief    Average picid
    //!
    enum AvcPicid
    {
        avcPicidDisabled = 0,
        avcPicidDefault = 0xFFFFFFFF
    };

    //!
    //! \enum     AvcImgStructure
    //! \brief    Average image structure
    //!
    enum AvcImgStructure
    {
        avcFrame        = 0,
        avcTopField     = 1,
        avcBottomField  = 3
    };

    //!
    //! \enum     CodecSelect
    //! \brief    Code select
    //!
    enum CodecSelect
    {
        decoderCodec    = 0,
        encoderCodec    = 1
    };

    //!
    //! \enum     MfxDecoderModeSelect
    //! \brief    MFX decoder mode select
    //!
    enum MfxDecoderModeSelect
    {
        mfxDecoderModeVld   = 0,
        mfxDecoderModeIt    = 1
    };

    //!
    //! \struct   VDEncFrameDeltaTable
    //! \brief    VD encode frame delta table
    //!
    struct VDEncFrameDeltaTable
    {
        uint32_t PFrameDelta;
        uint32_t IFrameDelta;
    };

    static const uint16_t m_mpeg2DefaultIntraQuantizerMatrix[64]; //!< MPEG2 default quantization matrix for intra macroblock
    static const uint16_t m_mpeg2DefaultNonIntraQuantizerMatrix[64]; //!< MPEG2 default quantization matrix for inter macroblock
    static const uint32_t m_mpeg2SliceDeltaQPMax[4]; //! MPEG2 slice delta QP max value
    static const uint32_t m_mpeg2InitSliceDeltaQPMin[4]; //!< MPEG2 slice delta QP min value
    static const uint32_t m_mpeg2FrameBitrateMinMax[4]; //!< MPEG2 frame bitrate Min/Max value
    static const uint32_t m_mpeg2FrameBitrateMinMaxDelta[4]; //!< MPEG2 frame bitrate Min/Max delta value

    static const uint8_t  m_columnScan4x4[16]; //! AVC column scan order for 4x4 block
    static const uint8_t  m_columnScan8x8[64]; //!< AVC column scan order for 8x8 block
    static const AvcSliceType m_AvcBsdSliceType[10]; //!< AVC BSD slice type

    static const uint8_t  m_vp8MaxNumPartitions = 8; //!< Maximal number of partitions for VP8

    static const uint32_t m_mfxErrorFlagsMask = 0xFBFF; //!< Mfx error flags mask
                                                        //!< Bit 10 of MFD_ERROR_STATUS register is set to a random value during RC6, so it is not used

    static const VDEncFrameDeltaTable m_vdEncFrameDelta100PercentTab[CODEC_AVC_NUM_QP];
    static const VDEncFrameDeltaTable m_vdEncFrameDelta90PercentTab[CODEC_AVC_NUM_QP];

    static const uint32_t m_log2WeightDenomDefault = 5; //!< Default value of luma/chroma_log2_weight_denoms
    static const uint32_t m_mpeg2WeightScaleSize = 16; //!< Size of MPEG2 weight scale

    PMOS_INTERFACE              m_osInterface = nullptr; //!< Pointer to OS interface
    MhwMiInterface             *m_MiInterface = nullptr; //!< Pointer to MI interface
    MhwCpInterface             *m_cpInterface = nullptr; //!< Pointer to CP interface
    MEDIA_FEATURE_TABLE         *m_skuTable = nullptr; //!< Pointer to SKU table
    MEDIA_WA_TABLE              *m_waTable = nullptr; //!< Pointer to WA table
    bool                        m_decodeInUse = false; //!< Flag to indicate if the interface is for decoder or encoder use

    PLATFORM                    m_platform = {}; //!< Gen platform

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {}; //!< Cacheability settings

    uint32_t                    m_numBrcPakPasses = 4; //!< Number of BRC PAK passes
    bool                        m_rhoDomainStatsEnabled = false; //!< Flag to indicate if Rho domain stats is enabled
    bool                        m_rowstoreCachingSupported = false; //!< Flag to indicate if row store cache is supported

    MHW_VDBOX_ROWSTORE_CACHE    m_intraRowstoreCache = {}; //!< Intra rowstore cache
    MHW_VDBOX_ROWSTORE_CACHE    m_deblockingFilterRowstoreCache = {}; //!< Deblocking filter row store cache
    MHW_VDBOX_ROWSTORE_CACHE    m_bsdMpcRowstoreCache = {}; //!< BSD/MPC row store cache
    MHW_VDBOX_ROWSTORE_CACHE    m_mprRowstoreCache = {}; //!< MPR row store cache
    MHW_VDBOX_NODE_IND          m_maxVdboxIndex = MHW_VDBOX_NODE_1; //!< max vdbox index

    uint32_t                    m_avcImgStateSize = 0;  //!< size of avcImgState
    uint8_t                     m_numVdbox = 1; //!< vdbox num
    uint32_t                    m_brcNumPakPasses = 4; //!< Number of brc pak passes

    MmioRegistersMfx            m_mmioRegisters[MHW_VDBOX_NODE_MAX] = {};  //!< mfx mmio registers

    //!
    //! \brief    Constructor
    //!
    MhwVdboxMfxInterface(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse);

    //!
    //! \brief    Get Jpeg decode output surface format
    //! \details  VDBOX protected function to get jpeg decode output format
    //! \param    [in] format 
    //!           MOS type format
    //! \return   MHW_VDBOX_DECODE_JPEG_FORMAT_CODE
    //!           output surface format
    //!
    MHW_VDBOX_DECODE_JPEG_FORMAT_CODE GetJpegDecodeFormat(MOS_FORMAT format);

    //!
    //! \brief    Calculate Min/Max bitrate for AVC image state
    //!
    //! \param    [in, out] params
    //!           AVC bitrate parameters
    //!
    //! \return   void
    //!
    virtual void CalcAvcImgStateMinMaxBitrate(MHW_VDBOX_AVC_IMG_BITRATE_PARAMS& params);

    //!
    //! \brief    Add a resource to the command buffer
    //! \details  Internal function to add either a graphics address of a resource or
    //!           add the resource to the patch list for the requested buffer
    //!
    //! \param    [in] osInterface
    //!           OS interface
    //! \param    [in] cmdBuffer
    //!           Command buffer to which resource is added
    //! \param    [in] params
    //!           Parameters necessary to add the resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS(*AddResourceToCmd) (
        PMOS_INTERFACE osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_RESOURCE_PARAMS params);

    //!
    //! \brief    Helper function for getting view order in the case of MVC
    //!
    //! \param    [in] params
    //!           Pointer to AVC DPB params
    //! \param    [in] currIdx
    //!           Reference frame index
    //! \param    [in] list
    //!           forward (list0) or backword reference (list1)
    //!
    //! \return   uint32_t
    //!           View order
    //!
    uint32_t GetViewOrder(
        PMHW_VDBOX_AVC_DPB_PARAMS params,
        uint32_t currIdx,
        uint32_t list);

    //!
    //! \brief    Check if the input mos type format is VPlane Present
    //! \details  VDBOX protected function to check the input mos type format
    //!
    //! \param    [in]  format
    //!           MOS type format
    //! \return   bool
    //!           return true if the input format is VPlane present
    //!           otherwise return false
    //!
    bool IsVPlanePresent(
        MOS_FORMAT  format);

    //!
    //! \brief    Translate MOS type format to Mediastate surface format
    //! \details  VDBOX protected function to translate mos format to media state format
    //! \param    [in] format
    //!           MOS type format
    //! \return   uint32_t
    //!           media state surface format
    //!
    uint32_t MosToMediaStateFormat(MOS_FORMAT format);

public:
    //!
    //! \brief    Get Jpeg horizontal sampling factor by the given format
    //! \details  Get Jpeg horizontal sampling factor by the given format
    //! \param    [in] format
    //!           Jpeg input surface format
    //! \return   uint32_t
    //!           returns the horizontal sampling factor
    //!
    uint32_t GetJpegHorizontalSamplingFactorForY(
        CodecEncodeJpegInputSurfaceFormat format);

    //!
    //! \brief    Get Jpeg vetical sampling factor by the given format
    //! \details  Get Jpeg vetical sampling factor by the given format
    //! \param    [in] format
    //!           Jpeg input surface format
    //! \return   uint32_t
    //!           returns the vetical sampling factor
    //!
    uint32_t GetJpegVerticalSamplingFactorForY(
        CodecEncodeJpegInputSurfaceFormat format);

    //!
    //! \brief    Determines if the frame/field is of I type
    //!
    //! \param    [in] picture
    //!           CODECHAL_PICTURE structure, including frame index and picture flags
    //! \param    [in] isFirstField
    //!           True if the picture is first field, false otherwise
    //! \param    [in] picType
    //!           VC1 picture type
    //!
    //! \return   bool
    //!           True if it's I picture, otherwise return false
    //!
    bool IsVc1IPicture(
        const CODEC_PICTURE& picture,
        bool isFirstField,
        uint16_t picType);

    //!
    //! \brief    Determines if the frame/field is of P type
    //!
    //! \param    [in] picture
    //!           CODECHAL_PICTURE structure, including frame index and picture flags
    //! \param    [in] isFirstField
    //!           True if the picture is first field, false otherwise
    //! \param    [in] picType
    //!           VC1 picture type
    //!
    //! \return   bool
    //!           True if it's P picture, otherwise return false
    //!
    bool IsVc1PPicture(
        const CODEC_PICTURE& picture,
        bool isFirstField,
        uint16_t picType);

    //!
    //! \brief    Determines if the frame/field is of B type
    //!
    //! \param    [in] picture
    //!           CODECHAL_PICTURE structure, including frame index and picture flags
    //! \param    [in] isFirstField
    //!           True if the picture is first field, false otherwise
    //! \param    [in] picType
    //!           VC1 picture type
    //!
    //! \return   bool
    //!           True if it's B picture, otherwise return false
    //!
    bool IsVc1BPicture(
        const CODEC_PICTURE& picture,
        bool isFirstField,
        uint16_t picType);

    //!
    //! \brief    Determines if the frame/field is of BI type
    //!
    //! \param    [in] picture
    //!           CODECHAL_PICTURE structure, including frame index and picture flags
    //! \param    [in] isFirstField
    //!           True if the picture is first field, false otherwise
    //! \param    [in] picType
    //!           VC1 picture type
    //!
    //! \return   bool
    //!           True if it's BI picture, otherwise return false
    //!
    bool IsVc1BIPicture(
        const CODEC_PICTURE& picture,
        bool isFirstField,
        uint16_t picType);

    //!
    //! \brief    Determines if the slice is P slice
    //! \param    [in] sliceType
    //!           slice type
    //! \return   bool
    //!           True if it's P slice, otherwise return false
    //!
    bool IsAvcPSlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(m_AvcBsdSliceType)) ?
            (m_AvcBsdSliceType[sliceType] == avcSliceP) : false;
    }

    //!
    //! \brief    Determines if the slice is B slice
    //! \param    [in] sliceType
    //!           slice type
    //! \return   bool
    //!           True if it's B slice, otherwise return false
    //!
    bool IsAvcBSlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(m_AvcBsdSliceType)) ?
            (m_AvcBsdSliceType[sliceType] == avcSliceB) : false;
    }

    //!
    //! \brief    Determines if the slice is I slice
    //! \param    [in] sliceType
    //!           slice type
    //! \return   bool
    //!           True if it's I slice, otherwise return false
    //!
    bool IsAvcISlice(uint8_t sliceType)
    {
        return (sliceType < MHW_ARRAY_SIZE(m_AvcBsdSliceType)) ?
            (m_AvcBsdSliceType[sliceType] == avcSliceI) : false;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxMfxInterface() {}

    //!
    //! \brief    Judge if decode is in use
    //!
    //! \return   bool
    //!           true if decode in use, else false
    //!
    inline bool IsDecodeInUse()
    {
        return m_decodeInUse;
    }

    //!
    //! \brief    Get Mfx Error Flags Mask
    //!
    //! \return   [out] uint32_t
    //!           Mask got.
    //!
    inline uint32_t GetMfxErrorFlagsMask()
    {
        return m_mfxErrorFlagsMask;
    }

    //!
    //! \brief    Get the vdbox num
    //!
    //! \return   bool
    //!           vdbox num got
    //!
    inline uint8_t GetNumVdbox()
    {
        return m_numVdbox;
    }

    //!
    //! \brief    set the flag of decode in use
    //!
    //! \param    [in] decodeInUse
    //!           decodeInUse flag to set
    //!
    //! \return   void
    //!
    inline void SetDecodeInUse(bool decodeInUse)
    {
        m_decodeInUse = decodeInUse;
    }

    //!
    //! \brief    Judge if row store caching supported
    //!
    //! \return   bool
    //!           true if supported, else false
    //!
    inline bool IsRowStoreCachingSupported()
    {
        return m_rowstoreCachingSupported;
    }

    //!
    //! \brief    Judge if intra row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsIntraRowstoreCacheEnabled()
    {
        return m_intraRowstoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if deblocking filter row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsDeblockingFilterRowstoreCacheEnabled()
    {
        return m_deblockingFilterRowstoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if bsd mpc row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsBsdMpcRowstoreCacheEnabled()
    {
        return m_bsdMpcRowstoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if mpr row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsMprRowstoreCacheEnabled()
    {
        return m_mprRowstoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Get max vdbox index
    //!
    //! \return   MHW_VDBOX_NODE_IND
    //!           max vdbox index got
    //!
    inline MHW_VDBOX_NODE_IND GetMaxVdboxIndex()
    {
        return m_maxVdboxIndex;
    }

    //!
    //! \brief    Get mmio registers
    //!
    //! \param    [in] index
    //!           mmio registers index.
    //!
    //! \return   [out] MmioRegistersMfx*
    //!           mmio registers got.
    //!
    inline MmioRegistersMfx* GetMmioRegisters(MHW_VDBOX_NODE_IND index)
    {
        if (index < MHW_VDBOX_NODE_MAX)
        {
            return &m_mmioRegisters[index];
        }
        else
        {
            MHW_ASSERT("index is out of range!");
            return &m_mmioRegisters[MHW_VDBOX_NODE_1];
        }
    }

    //!
    //! \brief    Convert from Mfx mmio registers to MI mmio register
    //!
    //! \param    [in] index
    //!           mmio registers index.
    //! \param    [in] mmioRegister
    //!           reference to MHW_MI_MMIOREGISTERS.
    //!
    //! \return   [out] bool
    //!           return true if mmio register if found, otherwise return false
    //!
    inline bool ConvertToMiRegister(MHW_VDBOX_NODE_IND index, MHW_MI_MMIOREGISTERS &mmioRegister)
    {
        MmioRegistersMfx* mfxMmioReg = GetMmioRegisters(index);
        if (mfxMmioReg)
        {
            mmioRegister.generalPurposeRegister0LoOffset = mfxMmioReg->generalPurposeRegister0LoOffset;
            mmioRegister.generalPurposeRegister0HiOffset = mfxMmioReg->generalPurposeRegister0HiOffset;
            mmioRegister.generalPurposeRegister4LoOffset = mfxMmioReg->generalPurposeRegister4LoOffset;
            mmioRegister.generalPurposeRegister4HiOffset = mfxMmioReg->generalPurposeRegister4HiOffset;
            mmioRegister.generalPurposeRegister11LoOffset = mfxMmioReg->generalPurposeRegister11LoOffset;
            mmioRegister.generalPurposeRegister11HiOffset = mfxMmioReg->generalPurposeRegister11HiOffset;
            mmioRegister.generalPurposeRegister12LoOffset = mfxMmioReg->generalPurposeRegister12LoOffset;
            mmioRegister.generalPurposeRegister12HiOffset = mfxMmioReg->generalPurposeRegister12HiOffset;
            return true;
        }
        else
            return false;
    }

    //!
    //! \brief    Get brc pak passes num
    //! \details  Get brc pak passes num in mfx interface
    //!
    //! \return   [out] uint32_t
    //!           Brc pak passes num.
    //!
    inline uint32_t GetBrcNumPakPasses()
    {
        return m_brcNumPakPasses;
    }

    //!
    //! \brief    Set brc pak passes num
    //! \details  Set brc pak passes num in hcp interface
    //!
    //! \param    [in] brcNumPakPasses
    //!           Brc pak passes num to set.
    //!
    //! \return   void
    //!
    inline void SetBrcNumPakPasses(uint32_t brcNumPakPasses)
    {
        m_brcNumPakPasses = brcNumPakPasses;
    }

    //!
    //! \brief    get AVC img state size
    //!
    //! \return   uint32_t
    //!           AVC img state size got
    //!
    virtual uint32_t GetAvcImgStateSize() = 0;

    //!
    //! \brief    Decide Which GPU Node to use for Decode
    //! \details  Client facing function to create gpu context used by decoder
    //! \param    [in] gpuNodeLimit
    //!           GpuNode Limitation
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FindGpuNodeToUse(
        PMHW_VDBOX_GPUNODE_LIMIT       gpuNodeLimit) = 0;

    //!
    //! \brief    Set cacheability settings
    //!
    //! \param    [in] cacheabilitySettings
    //!           Cacheability settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCacheabilitySettings(
        MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC])
    {
        MHW_FUNCTION_ENTER;

        uint32_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);
        MOS_STATUS eStatus = MOS_SecureMemcpy(m_cacheabilitySettings, size,
            cacheabilitySettings, size);

        return eStatus;
    }

    //!
    //! \brief    Get average bsd slice type
    //!
    //! \param    [in] index
    //!           Index number
    //!
    //! \return   int32_t
    //!           Avc bsd slice type
    //!
    int32_t GetAvcBsdSliceType(uint32_t index)
    {
        return m_AvcBsdSliceType[index];
    }

    //!
    //! \brief    Calculates the maximum size for all picture level commands
    //! \details  Client facing function to calculate the maximum size for all picture level commands
    //!
    //! \param    [in] mode
    //!           codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] isShortFormat
    //!           True if short format, false long format
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMfxStateCommandsDataSize(
        uint32_t mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool isShortFormat) = 0;

    //!
    //! \brief    Calculates maximum size for all slice/MB level commands
    //! \details  Client facing function to calculate maximum size for all slice/MB level commands
    //!
    //! \param    [in] mode
    //!           Codec mode
    //! \param    [out] commandsSize
    //!           The maximum command buffer size
    //! \param    [out] patchListSize
    //!           The maximum command patch list size
    //! \param    [in] isModeSpecific
    //!           Indicate the long or short format for decoder or single take phase for encoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMfxPrimitiveCommandsDataSize(
        uint32_t mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool  isModeSpecific) = 0;

    //!
    //! \brief    Adds MFX AVC Image State command in command buffer
    //! \details  Client facing function to add MFX AVC Image State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddMfxAvcImgCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params);

    //!
    //! \brief    Adds MFX AVC weight offset command in command buffer
    //! \details  Client facing function to add MFX AVC weight offset command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddMfxAvcWeightOffset(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS params);

    //!
    //! \brief    Adds MFX AVC slice state command in command buffer
    //! \details  Client facing function to add MFX AVC slice state command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] avcSliceState
    //!           Pointer to AVC slice state which is used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddMfxAvcSlice(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState);

    //!
    //! \brief    Adds MPEG2 Image State command in command buffer
    //! \details  Client facing function to add MPEG2 Image State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddMfxMpeg2PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_MPEG2_PIC_STATE params);

    //!
    //! \brief    Adds MFX VP8 Picture State command in command buffer
    //! \details  Client facing function to add MFX VP8 Picture State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddMfxVp8PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_PIC_STATE params);

    //!
    //! \brief    Programs base address of rowstore scratch buffers
    //! \details  Internal function to get base address of rowstore scratch buffers
    //!
    //! \param    [in] rowstoreParams
    //!           Rowstore parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) = 0;

    //!
    //! \brief    Adds MFX pipe mode select command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX Surface State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxSurfaceCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX Pipe Buffer Address State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX Indirect Object Base Address State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX BSP Buffer Base Address State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxBspBufBaseAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX QM State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxQmCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_QM_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX FQM State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxFqmCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_QM_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX AVC Picture ID command in command buffer (decoder only)
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfdAvcPicidCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_PIC_ID_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX AVC Image State command for decoder in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Pointer to MHW batch buffer
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxDecodeAvcImgCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX AVC Image State command for encoder in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxEncodeAvcImgCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params) = 0;

    //!
    //! \brief    Adds AVC Directmode State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxAvcDirectmodeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_DIRECTMODE_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX AVC reference frame index command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxAvcRefIdx(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_REF_IDX_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX AVC weight offset command for decoder in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxDecodeAvcWeightOffset(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX AVC weight offset command for encoder in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxEncodeAvcWeightOffset(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX AVC slice state command for decoder in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] avcSliceState
    //!           Pointer to AVC slice state which is used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxDecodeAvcSlice(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState) = 0;

    //!
    //! \brief    Adds MFX AVC slice state command for encoder in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] avcSliceState
    //!           Pointer to AVC slice state which is used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxEncodeAvcSlice(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState) = 0;

    //!
    //! \brief    Adds MFX AVC DPB State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfdAvcDpbCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_DPB_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX AVC Slice Address command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] avcSliceState
    //!           Pointer to AVC slice state which is used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfdAvcSliceAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState) = 0;

    //!
    //! \brief    Adds AVC BSD Object command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] avcSliceState
    //!           Pointer to AVC slice state which is used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfdAvcBsdObjectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState) = 0;

    //!
    //! \brief    Adds AVC Image State command in command buffer
    //! \details  Client facing function to add AVC Image State command in command buffer
    //!
    //! \param    [in] brcImgBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxAvcImgBrcBuffer(
        PMOS_RESOURCE brcImgBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX Pak Insert Object command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxPakInsertObject(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_PAK_INSERT_PARAMS params) = 0;

    //!
    //! \brief    Adds MPEG2 Image State command for decoder in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxDecodeMpeg2PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_MPEG2_PIC_STATE params) = 0;

    //!
    //! \brief    Adds MPEG2 Image State command for encoder in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxEncodeMpeg2PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_MPEG2_PIC_STATE params) = 0;

    //!
    //! \brief    Adds MPEG2 BSD Object command in batch buffer or command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to add to VDBOX_BUFFER_START
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfdMpeg2BsdObject(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_MPEG2_SLICE_STATE params) = 0;

    //!
    //! \brief    Adds Mfd mpeg2 IT object command in batch buffer or command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to add to VDBOX_BUFFER_START
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfdMpeg2ITObject(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_MPEG2_MB_STATE params) = 0;

    //!
    //! \brief    Adds Mpeg2 Group Slice State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] mpeg2SliceState
    //!           Pointer to MPEG2 slice state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfcMpeg2SliceGroupCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_MPEG2_SLICE_STATE mpeg2SliceState) = 0;

    //!
    //! \brief    Adds mpeg2 Pak insert brc buffer command in command buffer
    //! \details  Client facing function to add mpeg2 Pak insert brc buffer command in command buffer
    //!
    //! \param    [in] brcPicHeaderInputBuffer
    //!           Picture header input buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfcMpeg2PakInsertBrcBuffer(
        PMOS_RESOURCE brcPicHeaderInputBuffer,
        PMHW_VDBOX_PAK_INSERT_PARAMS params) = 0;

    //!
    //! \brief    Adds Mpeg2 picture State command in command buffer
    //! \details  Client facing function to add Mpeg2 picture State command in command buffer
    //!
    //! \param    [in] brcImgBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Pointer to MPEG2 picture state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxMpeg2PicBrcBuffer(
        PMOS_RESOURCE brcImgBuffer,
        PMHW_VDBOX_MPEG2_PIC_STATE params) = 0;

    //!
    //! \brief    Adds VC1 Pred Pipe command in command buffer
    //! \details  Client facing function to add VC1 Pred Pipe command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxVc1PredPipeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VC1_PRED_PIPE_PARAMS params) = 0;

    //!
    //! \brief    Adds VC1 long picture state command in command buffer
    //! \details  Client facing function to add VC1 long picture state command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] vc1PicState
    //!           Pointer to VC1 picture state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxVc1LongPicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VC1_PIC_STATE vc1PicState) = 0;

    //!
    //! \brief    Adds VC1 short picture state command in command buffer
    //! \details  Client facing function to add VC1 short picture state command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] vc1PicState
    //!           Pointer to VC1 picture state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfdVc1ShortPicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VC1_PIC_STATE vc1PicState) = 0;

    //!
    //! \brief    Adds VC1 Direct Mode State command in command buffer
    //! \details  Client facing function to add VC1 Direct Mode State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxVc1DirectmodeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VC1_DIRECTMODE_PARAMS params) = 0;

    //!
    //! \brief    Adds VC1 BSD object command in command buffer
    //! \details  Client facing function to add VC1 BSD object command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] vc1SliceState
    //!           Pointer to VC1 slice state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfdVc1BsdObjectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VC1_SLICE_STATE vc1SliceState) = 0;

    //!
    //! \brief    Adds Mfd VC1 IT object command in batch buffer
    //! \details  Client facing function to add Mfd VC1 IT object command in batch buffer
    //!
    //! \param    [in] batchBuffer
    //!           Batch buffer to add to VDBOX_BUFFER_START
    //! \param    [in] vc1MbState
    //!           Pointer to VC1 MB state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfdVc1ItObjectCmd(
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_VC1_MB_STATE vc1MbState) = 0;

    //!
    //! \brief    Adds MFX JPEG Picture State command in command buffer
    //! \details  Client facing function to add MFX JPEG Picture State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxJpegPicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_JPEG_PIC_STATE params) = 0;

    //!
    //! \brief    Adds MFX JPEG Huffman Table State command in command buffer
    //! \details  Client facing function to add MFX JPEG Huffman Table State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxJpegHuffTableCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_HUFF_TABLE_PARAMS params) = 0;

    //!
    //! \brief    Adds MFX JPEG BSD object command in command buffer
    //! \details  Client facing function to add MFX JPEG BSD object State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxJpegBsdObjCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_JPEG_BSD_PARAMS params) = 0;

    //!
    //! \brief    Adds JPEG picture state command to the command buffer
    //! \details  Client facing function to add JPEG picture state command to the command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxJpegEncodePicStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        MhwVdboxJpegEncodePicState *params) = 0;

    //!
    //! \brief    Adds JPEG FQM command to the command buffer
    //! \details  Client facing function to add JPEG FQM command to the command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \param    [in] numQuantTables
    //!           Numbe of quant tables
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxJpegFqmCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_QM_PARAMS params,
        uint32_t numQuantTables) = 0;

    //!
    //! \brief    Adds JPEG Huffman Table state command to the command buffer
    //! \details  Client facing function to add JPEG Huffman Table state command to the command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfcJpegHuffTableStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_ENCODE_HUFF_TABLE_PARAMS params) = 0;

    //!
    //! \brief    Adds JPEG scan object command to the command buffer
    //! \details  Client facing function to add JPEG scan object command to the command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfcJpegScanObjCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        MhwVdboxJpegScanParams *params) = 0;

    //!
    //! \brief    Adds MFX VP8 Picture State command for decoder in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxDecodeVp8PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_PIC_STATE params) = 0;

    //!
    //! \brief    Adds MFX VP8 Picture State command for encoder in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxEncodeVp8PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_PIC_STATE params) = 0;

    //!
    //! \brief    Adds VP8 Decode BSD State command in command buffer
    //! \details  Client facing function to add VP8 Decode BSD State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfdVp8BsdObjectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_BSD_PARAMS params) = 0;

    //!
    //! \brief    Initializes the encoder cfg buffer with MFX_VP8_ENCODER_CFG_CMD
    //! \details  Client facing function to initialize the encoder cfg buffer with MFX_VP8_ENCODER_CFG_CMD
    //!           Headder offsets will be updated by kernel
    //!           The function will also add a batch buffer at the end.
    //!
    //! \param    [in] cfgCmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitMfxVp8EncoderCfgCmd(
        PMOS_RESOURCE cfgCmdBuffer,
        PMHW_VDBOX_VP8_ENCODER_CFG_PARAMS params) = 0;

    //!
    //! \brief    Adds VP8 MFX BSP Buffer Base Address State command in command buffer
    //! \details  Client facing function to add VP8 MFX BSP Buffer Base Address State command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMfxVp8BspBufBaseAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_BSP_BUF_BASE_ADDR_PARAMS params) = 0;
};

#endif
