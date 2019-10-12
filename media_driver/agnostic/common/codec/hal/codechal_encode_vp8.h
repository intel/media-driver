/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_encode_vp8.h
//! \brief    This file defines the base C++ class/interface for VP8 DualPipe encoding
//!           to be used across CODECHAL components.
//!

#ifndef __CODECHAL_ENCODE_VP8_H__
#define __CODECHAL_ENCODE_VP8_H__

#include "codechal_encoder_base.h"

#define CODECHAL_NUM_VP8_ENC_SYNC_TAGS      36
#define CODECHAL_INIT_DSH_SIZE_VP8_ENC      MHW_PAGE_SIZE * 2

#define CODECHAL_ENCODE_VP8_INVALID_PIC_ID  CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8

#define VP8_NUM_COEFF_PLANES                4
#define VP8_NUM_COEFF_BANDS                 8
#define VP8_NUM_LOCAL_COMPLEXITIES          3
#define VP8_NUM_COEFF_NODES                 11
#define VP8_LAST_REF_FLAG                   0x1
#define VP8_GOLDEN_REF_FLAG                 0x2
#define VP8_ALT_REF_FLAG                    0x4

#define INTERMEDIATE_PARTITION0_SIZE        (64 * 1024)
#define TOKEN_STATISTICS_SIZE               (304 * sizeof(uint32_t))                                                                           //270 tokens + 34 DWs for partition and segment info
#define COEFFS_PROPABILITIES_SIZE           (VP8_NUM_COEFF_PLANES * VP8_NUM_COEFF_BANDS * VP8_NUM_LOCAL_COMPLEXITIES * VP8_NUM_COEFF_NODES) //1056
#define HISTOGRAM_SIZE                      (136 * sizeof(uint32_t))
#define MODE_PROPABILITIES_SIZE             96
#define HEADER_METADATA_SIZE                (32  * sizeof(uint32_t))
#define PICTURE_STATE_CMD_SIZE              (37  * sizeof(uint32_t))
#define PICTURE_STATE_SIZE                  (PICTURE_STATE_CMD_SIZE + HEADER_METADATA_SIZE + (16 * sizeof(uint32_t)))                            // + Extra dws for NOOP and BB_End
#define HEADER_METADATA_OFFSET              (PICTURE_STATE_CMD_SIZE + (3 * sizeof(uint32_t)))                                                   //Add one extra noop
#define MPU_BITSTREAM_SIZE                  128
#define TPU_BITSTREAM_SIZE                  1344
#define ENTROPY_COST_TABLE_SIZE             (256 * sizeof(uint32_t))
#define MPU_CURBE_SIZE                      (24 * sizeof(uint32_t))
#define TOKEN_BITS_DATA_SIZE                (16 * sizeof(uint32_t))
#define VP8_KERNEL_DUMP_SIZE                (600000 * sizeof(uint32_t))
#define VP8_MODE_COST_UPDATE_SURFACE_SIZE   64
#define REPAK_DECISION_BUF_SIZE             (4 * sizeof(uint32_t))

#define VP8_ALL_DC_BIAS_DEFAULT                                1500
#define CODECHAL_VP8_QP_MAX                                    127
#define CODECHAL_VP8_MODE_COST_SURFACE_SIZE                    64

#define BRC_CONSTANTSURFACE_VP8                                2880
#define CODECHAL_ENCODE_VP8_BRC_CONSTANTSURFACE_WIDTH          64
#define CODECHAL_ENCODE_VP8_BRC_CONSTANTSURFACE_HEIGHT         44

//*------------------------------------------------------------------------------
//* Codec Definitions
//*------------------------------------------------------------------------------

enum CodechalEncodeVp8MbencKernelStateIdx
{
    CODECHAL_ENCODE_VP8_MBENC_IDX_I_CHROMA = 0,
    CODECHAL_ENCODE_VP8_MBENC_IDX_I_LUMA,
    CODECHAL_ENCODE_VP8_MBENC_IDX_P,
    CODECHAL_ENCODE_VP8_MBENC_IDX_NUM
};

enum CodechalEncodeVp8BrcKernelStateIdx
{
    CODECHAL_ENCODE_VP8_BRC_IDX_IFRAMEDIST = 0,
    CODECHAL_ENCODE_VP8_BRC_IDX_INIT,
    CODECHAL_ENCODE_VP8_BRC_IDX_RESET,
    CODECHAL_ENCODE_VP8_BRC_IDX_UPDATE,
    CODECHAL_ENCODE_VP8_BRC_IDX_NUM,
};

enum CodechalEncodeVp8MbpakKernelStateIdx
{
    CODECHAL_ENCODE_VP8_MBPAK_IDX_PHASE1 = 0,
    CODECHAL_ENCODE_VP8_MBPAK_IDX_PHASE2,
    CODECHAL_ENCODE_VP8_MBPAK_IDX_NUM
};

static const uint8_t CodecHal_TargetUsageToMode_VP8[NUM_TARGET_USAGE_MODES] =
{
    encodeNormalMode,
    encodeQualityMode,
    encodeQualityMode,
    encodeNormalMode,
    encodeNormalMode,
    encodeNormalMode,
    encodePerformanceMode,
    encodePerformanceMode
};

struct CodechalVp8ModeCostUpdateSurface
{
    union
    {
        struct
        {
            uint32_t            : 16;
            uint32_t Intra16x16 : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW0;

    union
    {
        struct
        {
            uint32_t          : 16;
            uint32_t Intra4x4 : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW1;

    union
    {
        struct
        {
            uint32_t Inter16x8 : 16;
            uint32_t Inter8x8  : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW2;

    union
    {
        struct
        {
            uint32_t          : 16;
            uint32_t Inter4x4 : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW3;

    union
    {
        struct
        {
            uint32_t : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW4;

    union
    {
        struct
        {
            uint32_t : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW5;

    union
    {
        struct
        {
            uint32_t IntraNonDCPenalty16x16 : 16;
            uint32_t IntraNonDCPenalty4x4   : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW6;

    union
    {
        struct
        {
            uint32_t RefFrameCostIntra : 16;
            uint32_t RefFrameCostLast  : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW7;

    union
    {
        struct
        {
            uint32_t RefFrameCostGold : 16;
            uint32_t RefFrameCostAlt  : 16;
        };
        struct
        {
            uint32_t Value;
        };
    } DW8;

};

struct CodechalBindingTableVp8BrcUpdate
{
    uint32_t   dwBrcHistoryBuffer;
    uint32_t   dwBrcPakStatisticsOutputBuffer;
    uint32_t   dwBrcMSDKPakSurfaceBuffer;
    uint32_t   dwBrcEncoderCfgReadBuffer;
    uint32_t   dwBrcEncoderCfgWriteBuffer;
    uint32_t   dwBrcMbPak1CurbeWriteBuffer;
    uint32_t   dwBrcMbPak2CurbeWriteBuffer;
    uint32_t   dwBrcMbPakInputMSDKBuffer;
    uint32_t   dwBrcMbPakTableData;
    uint32_t   dwBrcMbEncCurbeReadBuffer;
    uint32_t   dwBrcMbEncCurbeWriteData;
    uint32_t   dwBrcMpuCurbeReadBuffer;
    uint32_t   dwBrcMpuCurbeWriteData;
    uint32_t   dwBrcTpuCurbeReadBuffer;
    uint32_t   dwBrcTpuCurbeWriteData;
    uint32_t   dwBrcDistortionBuffer;
    uint32_t   dwBrcConstantData;
    uint32_t   dwVp8BrcSegmentationMap;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
};

struct CodechalBindingTableVp8Mbenc
{
    uint32_t   dwVp8MBEncMBOut;
    uint32_t   dwVp8MBEncCurrY;
    uint32_t   dwVp8MBEncCurrUV;
    uint32_t   dwVp8MBEncMVDataFromME;
    uint32_t   dwVp8MBEncIndMVData;
    uint32_t   dwVp8MBEncCurrPic;
    uint32_t   dwVp8MBEncLastRefPic;
    uint32_t   dwVp8MBEncGoldenRefPic;
    uint32_t   dwVp8MBEncAlternateRefPic;
    uint32_t   dwVp8MBEncRef1Pic;
    uint32_t   dwVp8MBEncRef2Pic;
    uint32_t   dwVp8MBEncRef3Pic;
    uint32_t   dwVp8MBEncMBModeCostLuma;
    uint32_t   dwVp8MBEncBlockModeCost;
    uint32_t   dwVp8MBEncChromaRecon;
    uint32_t   dwVp8MBEncPerMBQuantDataI;
    uint32_t   dwVp8MBEncPerMBQuantDataP;
    uint32_t   dwVp8MBEncRefMBCount;
    uint32_t   dwVp8MBEncVMEInterPred;
    uint32_t   dwVp8MBEncVMEDebugStreamoutI;
    uint32_t   dwVp8MBEncVMEDebugStreamoutP;
    uint32_t   dwVp8MBEncSegmentationMap;
    uint32_t   dwVp8MBEncSegmentationMapP;
    uint32_t   dwVp8MBEncHistogram;
    uint32_t   dwVp8MBEncHistogramP;
    uint32_t   dwVp8MBEncVME;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
    uint32_t   dwVp8InterPredDistortion;
    uint32_t   dwVp8PerMVDataSurface;
    uint32_t   dwVp8MBModeCostUpdateSurface;
    uint32_t   dwVp8MBEncBRCDist;
    uint32_t   dwVp8MBEncVMECoarseIntra;
    uint32_t   dwVp8MbEncCurrYDownscaled;
    uint32_t   dwVp8MbEncSwscoreboardI;
    uint32_t   dwVp8MbEncSwscoreboardP;
};

struct CodechalBindingTableVp8Me
{
    uint32_t   dwVp8MEMVDataSurface;
    uint32_t   dwVp816xMEMVDataSurface;
    uint32_t   dwVp8MeDist;
    uint32_t   dwVp8MeBrcDist;
    uint32_t   dwVp8MeCurrPic;
    uint32_t   dwVp8MeRef1Pic;
    uint32_t   dwVp8MeRef2Pic;
    uint32_t   dwVp8MeRef3Pic;
    uint32_t   dwVp8MeLastRefPic;
    uint32_t   dwVp8MeGoldenRefPic;
    uint32_t   dwVp8MeAlternateRefPic;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
};

struct CodechalBindingTableVp8Mbpak
{
    uint32_t   dwVp8MBPakPerMBOut;
    uint32_t   dwVp8MBPakCurrY;
    uint32_t   dwVp8MBPakCurrUV;
    uint32_t   dwVp8MBPakLastRefY;
    uint32_t   dwVp8MBPakLastRefUV;
    uint32_t   dwVp8MBPakGoldenRefY;
    uint32_t   dwVp8MBPakGoldenRefUV;
    uint32_t   dwVp8MBPakAlternateRefY;
    uint32_t   dwVp8MBPakAlternateRefUV;
    uint32_t   dwVp8MBPakIndMVData;
    uint32_t   dwVp8MBPakCurrReconY;
    uint32_t   dwVp8MBPakCurrReconUV;
    uint32_t   dwVp8MBPakRowBuffY;
    uint32_t   dwVp8MBPakRowBuffUV;
    uint32_t   dwVp8MBPakColBuffY;
    uint32_t   dwVp8MBPakColBuffUV;
    uint32_t   dwVp8MBPakDebugStreamout;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
};

struct CodechalBindingTableVp8Mpu
{
    uint32_t   dwVp8MpuHistogram;
    uint32_t   dwVp8MpuReferenceModeProbability;
    uint32_t   dwVp8MpuModeProbability;
    uint32_t   dwVp8MpuReferenceTokenProbability;
    uint32_t   dwVp8MpuTokenProbability;
    uint32_t   dwVp8MpuFrameHeaderBitstream;
    uint32_t   dwVp8MpuHeaderMetaData;
    uint32_t   dwVp8MpuPictureState;
    uint32_t   dwVp8MpuMpuBitstream;
    uint32_t   dwVp8MpuKernelDebugDump;
    uint32_t   dwVp8MpuEntropyCost;
    uint32_t   dwVp8MpuTokenBitsData;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
    uint32_t   dwVp8MpuModeCostUpdateSurface;
};

struct CodechalBindingTableVp8Tpu
{
    uint32_t   dwVp8TpuPakTokenStatistics;
    uint32_t   dwVp8TpuTokenUpdateFlags;
    uint32_t   dwVp8TpuEntropyCost;
    uint32_t   dwVp8TpuFrameHeaderBitstream;
    uint32_t   dwVp8TpuDefaultTokenProbability;
    uint32_t   dwVp8TpuPictureState;
    uint32_t   dwVp8TpuMpuCurbeData;
    uint32_t   dwVp8TpuHeaderMetaData;
    uint32_t   dwVp8TpuTokenProbability;
    uint32_t   dwVp8TpuPakHardwareTokenProbabilityPass1;
    uint32_t   dwVp8TpuKeyFrameTokenProbability;
    uint32_t   dwVp8TpuUpdatedTokenProbability;
    uint32_t   dwVp8TpuPakHardwareTokenProbabilityPass2;
    uint32_t   dwVp8TpuKernelDebugDump;
    uint32_t   dwVp8TpuRepakDecision;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
};

struct CodechalVp8BrcInitResetCurbeParams
{
    CODEC_PICTURE                           CurrPic;
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS       pSeqParams;
    PCODEC_VP8_ENCODE_PIC_PARAMS            pPicParams;
    uint32_t                                dwFrameWidth;
    uint32_t                                dwFrameHeight;
    uint32_t                                dwAVBRAccuracy;
    uint32_t                                dwAVBRConvergence;
    double                                  *pdBrcInitCurrentTargetBufFullInBits;   // Passed back to Render Interface
    double                                  *pdBrcInitResetInputBitsPerFrame;       // Passed back to Render Interface
    uint32_t                                *pdwBrcInitResetBufSizeInBits;           // Passed back to Render Interface
    bool                                    bInitBrc;
    bool                                    bMbBrcEnabled;
    uint32_t                                dwFramerate;
    PMHW_KERNEL_STATE                       pKernelState;
};

struct CodechalVp8BrcUpdateCurbeParams
{
    CODEC_PICTURE                           CurrPic;
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS       pSeqParams;
    PCODEC_VP8_ENCODE_PIC_PARAMS            pPicParams;
    PCODECHAL_VP8_HYBRIDPAK_FRAMEUPDATE     pSliceParams;
    PCODEC_VP8_ENCODE_QUANT_DATA            pVp8QuantData;
    uint16_t                                wPictureCodingType;
    uint32_t                                dwAVBRAccuracy;
    uint32_t                                dwAVBRConvergence;
    uint32_t                                dwFrameWidthInMB;
    uint32_t                                dwFrameHeightInMB;
    uint32_t                                dwBrcInitResetBufSizeInBits;
    double                                  dBrcInitResetInputBitsPerFrame;
    double                                  *pdBrcInitCurrentTargetBufFullInBits;    // Passed in and back
    bool                                    bHmeEnabled;
    bool                                    bInitBrc;
    bool                                    bUsedAsRef;
    uint8_t                                 ucKernelMode;                           // Normal/Quality/Performance
    uint32_t                                dwVp8BrcNumPakPasses;
    uint32_t                                dwHeaderBytesInserted;  // dwHeaderBytesInserted is for WAAVCSWHeaderInsertion and is 0 otherwise
    bool                                    bMultiRefQpEnabled;
    uint32_t                                wFrameNumber;
};

struct CodechalVp8MeCurbeParams
{
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS       pSeqParams;
    PCODEC_VP8_ENCODE_PIC_PARAMS            pPicParams;
    uint32_t                                dwFrameWidth;
    uint32_t                                dwFrameFieldHeight;
    uint16_t                                wPictureCodingType;
    bool                                    b16xME;
    bool                                    b16xMeEnabled;
    uint8_t                                 ucKernelMode;
};

struct CodechalVp8MbencCurbeParams
{
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS       pSeqParams;
    PCODEC_VP8_ENCODE_PIC_PARAMS            pPicParams;
    PCODEC_VP8_ENCODE_QUANT_DATA            pVp8QuantData;
    PCODECHAL_VP8_HYBRIDPAK_FRAMEUPDATE     pVp8SliceParams;
    PCODEC_REF_LIST                         *ppRefList;
    uint16_t                                wPicWidthInMb;
    uint16_t                                wFieldFrameHeightInMb;
    uint16_t                                wPictureCodingType;
    bool                                    bHmeEnabled;
    bool                                    bVmeKernelDump;
    bool                                    bMbEncIFrameDistEnabled;
    bool                                    bBrcEnabled;
    uint8_t                                 ucKernelMode;     // normal, performance, quality.
    bool                                    bMbEncIFrameDistInUse;
    PCODEC_PICTURE                          pCurrOriginalPic;
    PCODEC_PICTURE                          pLastRefPic;
    PCODEC_PICTURE                          pGoldenRefPic;
    PCODEC_PICTURE                          pAlternateRefPic;
    PMHW_KERNEL_STATE                       pKernelState;
};

struct CodechalVp8MbpakCurbeParams
{
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS       pSeqParams;
    PCODEC_VP8_ENCODE_PIC_PARAMS            pPicParams;
    PCODEC_VP8_ENCODE_QUANT_DATA            pVp8QuantData;
    PCODEC_REF_LIST                         *ppRefList;
    uint16_t                                wPicWidthInMb;
    uint16_t                                wFieldFrameHeightInMb;
    uint16_t                                wPictureCodingType;
    bool                                    bHmeEnabled;
    bool                                    bVmeKernelDump;
    uint8_t                                 ucKernelMode;     // normal, performance, quality.
    CODECHAL_MEDIA_STATE_TYPE               EncFunctionType;
    PMHW_KERNEL_STATE                       pKernelState;
};

struct CodechalVp8MpuCurbeParams
{
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS       pSeqParams;
    PCODEC_VP8_ENCODE_PIC_PARAMS            pPicParams;
    PCODEC_VP8_ENCODE_QUANT_DATA            pVp8QuantData;
    uint16_t                                wPictureCodingType;
    bool                                    bVmeKernelDump;
    uint8_t                                 ucKernelMode;     // normal, performance, quality.
    CODECHAL_MEDIA_STATE_TYPE               EncFunctionType;
};

struct CodechalVp8TpuCurbeParams
{
    PCODEC_VP8_ENCODE_SEQUENCE_PARAMS       pSeqParams;
    PCODEC_VP8_ENCODE_PIC_PARAMS            pPicParams;
    PCODEC_VP8_ENCODE_QUANT_DATA            pVp8QuantData;
    uint16_t                                wPictureCodingType;
    uint16_t                                wPicWidthInMb;
    uint16_t                                wFieldFrameHeightInMb;
    bool                                    bVmeKernelDump;
    uint8_t                                 ucKernelMode;     // normal, performance, quality.
    CODECHAL_MEDIA_STATE_TYPE               EncFunctionType;
    bool                                    bRebinarizationFrameHdr;
    bool                                    bAdaptiveRePak;
};

struct CodechalVp8BrcInitResetSurfaceParams
{
    PMOS_RESOURCE                       presBrcHistoryBuffer;
    PMOS_SURFACE                        psMeBrcDistortionBuffer;
    uint32_t                            dwMeBrcDistortionBottomFieldOffset;
    uint32_t                            dwDownscaledWidthInMb4x;
    uint32_t                            dwDownscaledFrameHeightInMb4x;
    PMHW_KERNEL_STATE                   pKernelState;
};

struct CodechalVp8BrcUpdateSurfaceParams
{
    PMHW_KERNEL_STATE                   pMbEncKernelState;
    PMOS_RESOURCE                       presBrcHistoryBuffer;                                                    // BRC history buffer
    PMOS_RESOURCE                       presBrcPakStatisticBuffer;                                               // BRC PAKStatistic buffer
    PMOS_RESOURCE                       presVp8BrcConstantDataBuffer;
    PMOS_RESOURCE                       presVp8EncoderCfgCommandReadBuffer;
    PMOS_RESOURCE                       presVp8EncoderCfgCommandWriteBuffer;
    PMOS_RESOURCE                       presVp8PakQPInputTable;
    PMOS_RESOURCE                       presMbCodeBuffer;
    PMOS_SURFACE                        ps4xMeDistortionBuffer;
    PMOS_SURFACE                        psMeBrcDistortionBuffer;
    MOS_RESOURCE                        resMbBrcConstDataBuffer;
    uint32_t                            dwDownscaledWidthInMb4x;
    uint32_t                            dwDownscaledFrameFieldHeightInMb4x;
    bool                                bMbBrcEnabled;
    PMOS_SURFACE                        psSegmentationMap;
    uint32_t                            dwEncoderCfgCommandOffset;
    PMHW_KERNEL_STATE                   pKernelState;
    uint16_t                           wPictureCodingType;     // I, P frame
    uint32_t                            dwBrcPakStatisticsSize;
};

struct CodechalVp8MeSurfaceParams
{
    PCODEC_REF_LIST                     *ppRefList;
    PCODEC_PICTURE                      pLastRefPic;
    PCODEC_PICTURE                      pGoldenRefPic;
    PCODEC_PICTURE                      pAlternateRefPic;
    PMOS_SURFACE                        ps4xMeMvDataBuffer;
    PMOS_SURFACE                        ps16xMeMvDataBuffer;
    PMOS_SURFACE                        psMeDistortionBuffer;
    PMOS_SURFACE                        psMeBrcDistortionBuffer;
    uint32_t                            dwDownscaledWidthInMb;
    uint32_t                            dwDownscaledHeightInMb;
    uint32_t                            dwVerticalLineStride;
    uint32_t                            dwVerticalLineStrideOffset;
    bool                                b16xMeInUse;
    bool                                b16xMeEnabled;
    uint32_t                            RefCtrl;
    struct CodechalBindingTableVp8Me*   pMeBindingTable;
    PMHW_KERNEL_STATE                   pKernelState;
};

struct CodechalVp8MbencSurfaceParams
{
    CODECHAL_MEDIA_STATE_TYPE           MediaStateType;
    PCODEC_REF_LIST                     *ppRefList;
    PCODEC_PICTURE                      pCurrReconstructedPic;
    PCODEC_PICTURE                      pLastRefPic;
    PCODEC_PICTURE                      pGoldenRefPic;
    PCODEC_PICTURE                      pAlternateRefPic;
    uint16_t                            wPictureCodingType;
    PMOS_SURFACE                        psCurrPicSurface;
    uint32_t                            dwCurrPicSurfaceOffset;
    uint32_t                            dwMvOffset;
    uint32_t                            dwHistogramSize;
    PMOS_SURFACE                        ps4xMeMvDataBuffer;
    PMOS_SURFACE                        ps4xMeDistortionBuffer;
    PMOS_SURFACE                        psMeBrcDistortionBuffer;
    uint32_t                            dwOriFrameWidth;
    uint32_t                            dwOriFrameHeight;
    uint32_t                            dwVerticalLineStride;
    uint32_t                            dwVerticalLineStrideOffset;
    uint32_t                            dwFrameWidthInMb;
    uint32_t                            dwFrameFieldHeightInMb;
    bool                                bHmeEnabled;
    bool                                bVMEKernelDump;
    bool                                bSegmentationEnabled;
    bool                                bMbEncIFrameDistInUse;
    uint32_t                            dwMbDataOffset;
    uint32_t                            uiRefCtrl;
    PMOS_RESOURCE                       presPerMB_MBCodeOpData;
    PMOS_SURFACE                        psMBModeCostLumaBuffer;
    PMOS_SURFACE                        psBlockModeCostBuffer;
    PMOS_RESOURCE                       psChromaReconBuffer; //for FF Vp8
    PMOS_SURFACE                        psPerMBQuantDataBuffer;
    PMOS_RESOURCE                       presRefMbCountSurface;
    PMOS_RESOURCE                       presVmeKernelDumpBuffer;
    PMOS_SURFACE                        psSegmentationMap;
    PMOS_RESOURCE                       presHistogram;
    PMOS_SURFACE                        psInterPredictionDistortionSurface;
    PMOS_RESOURCE                       presPerMVDataSurface;
    PMOS_RESOURCE                       presModeCostUpdateSurface;
    PCODECHAL_VP8_HYBRIDPAK_FRAMEUPDATE pVp8SliceParams;
    struct CodechalBindingTableVp8Mbenc* pMbEncBindingTable;
    PMHW_KERNEL_STATE                   pKernelState;
};

struct CodechalVp8MbpakSurfaceParams
{
    CODECHAL_MEDIA_STATE_TYPE           MediaStateType;
    PCODEC_REF_LIST                     *ppRefList;
    PCODEC_PICTURE                      pCurrReconstructedPic;
    PCODEC_PICTURE                      pLastRefPic;
    PCODEC_PICTURE                      pGoldenRefPic;
    PCODEC_PICTURE                      pAlternateRefPic;
    uint16_t                            wPictureCodingType;
    uint32_t                            dwCurrPicSurfaceOffset;
    uint32_t                            dwMvOffset;
    uint32_t                            dwOriFrameWidth;
    uint32_t                            dwOriFrameHeight;
    uint32_t                            dwVerticalLineStride;
    uint32_t                            dwVerticalLineStrideOffset;
    bool                                bVMEKernelDump;
    uint32_t                            dwMbDataOffset;
    PMOS_RESOURCE                       presPerMB_MBCodeOpData;
    PMOS_RESOURCE                       presRowBuffY;
    PMOS_RESOURCE                       presRowBuffUV;
    PMOS_RESOURCE                       presColBuffY;
    PMOS_RESOURCE                       presColBuffUV;
    PMOS_RESOURCE                       presVmeKernelDumpBuffer;
    struct CodechalBindingTableVp8Mbpak*  pMbPakBindingTable;
    PMHW_KERNEL_STATE                   pKernelState;
};

struct CodechalVp8MpuSurfaceParams
{
    CODECHAL_MEDIA_STATE_TYPE           MediaStateType;
    uint32_t                            dwHistogramSize;
    uint32_t                            dwModeProbabilitySize;
    uint32_t                            dwTokenProbabilitySize;
    uint32_t                            dwFrameHeaderSize;
    uint32_t                            dwPictureStateSize;
    uint32_t                            dwHeaderMetadataSize;
    uint32_t                            dwMpuBitstreamSize;
    uint32_t                            dwTpuBitstreamSize;
    uint32_t                            dwEntropyCostTableSize;
    uint32_t                            dwHeaderMetaDataOffset;
    uint32_t                            dwTokenBitsDataSize;
    uint32_t                            dwKernelDumpSize;
    bool                                bVMEKernelDump;
    PMOS_RESOURCE                       presHistogram;
    PMOS_RESOURCE                       presRefModeProbability;
    PMOS_RESOURCE                       presModeProbability;
    PMOS_RESOURCE                       presRefTokenProbability;
    PMOS_RESOURCE                       presTokenProbability;
    PMOS_RESOURCE                       presFrameHeader;
    PMOS_RESOURCE                       presHeaderMetadata;
    PMOS_RESOURCE                       presPictureState;
    PMOS_RESOURCE                       presMpuBitstream;
    PMOS_RESOURCE                       presTpuBitstream;
    PMOS_RESOURCE                       presVmeKernelDumpBuffer;
    PMOS_RESOURCE                       presEntropyCostTable;
    PMOS_RESOURCE                       presTokenBitsData;
    PMOS_RESOURCE                       presModeCostUpdateBuffer;
    PMHW_KERNEL_STATE                   pKernelState;
};

struct CodechalVp8TpuSurfaceParams
{
    CODECHAL_MEDIA_STATE_TYPE           MediaStateType;
    uint32_t                            dwPakTokenStatsSize;
    uint32_t                            dwTokenProbabilitySize;
    uint32_t                            dwEntropyCostTableSize;
    uint32_t                            dwFrameHeaderSize;
    uint32_t                            dwPictureStateSize;
    uint32_t                            dwMpuCurbeSize;
    uint32_t                            dwHeaderMetadataSize;
    uint32_t                            dwHeaderMetaDataOffset;
    uint32_t                            dwKernelDumpSize;
    uint32_t                            dwRepakDecision;
    bool                                bVMEKernelDump;
    PMOS_RESOURCE                       presPakTokenStatistics;
    PMOS_RESOURCE                       presPakTokenUpdateFlags;
    PMOS_RESOURCE                       presEntropyCostTable;
    PMOS_RESOURCE                       presFrameHeader;
    PMOS_RESOURCE                       presDefaultTokenProbability;
    PMOS_RESOURCE                       presPictureState;
    PMOS_RESOURCE                       presMpuCurbeData;
    PMOS_RESOURCE                       presHeaderMetadata;
    PMOS_RESOURCE                       presCurrFrameTokenProbability;
    PMOS_RESOURCE                       presHwTokenProbabilityPass1;
    PMOS_RESOURCE                       presKeyFrameTokenProbability;
    PMOS_RESOURCE                       presUpdatedFrameTokenProbability;
    PMOS_RESOURCE                       presHwTokenProbabilityPass2;
    PMOS_RESOURCE                       presRepakDecisionSurface;
    PMOS_RESOURCE                       presVmeKernelDumpBuffer;
    PMHW_KERNEL_STATE                   pKernelState;
};

struct CodechalResourcesBrcParams
{
    bool       bHWWalker;
    uint32_t   dwDownscaledWidthInMB4x;
    uint32_t   dwDownscaledHeightInMB4x;
    uint32_t   dwDownscaledFieldHeightInMB4x;
    uint32_t   dwFrameWidthInMB;
    uint32_t   dwFrameHeightInMB;
};

struct CodechalVp8InitBrcConstantBufferParams
{
    PMOS_INTERFACE                          pOsInterface;
    PCODEC_PIC_ID                           pVp8PicIdx;
    MOS_RESOURCE                            resBrcConstantDataBuffer; // sBrcConstantDataBuffer[uiCurrDSH]
    uint32_t                                dwMbEncBlockBasedSkipEn;
    PCODEC_VP8_ENCODE_PIC_PARAMS            pPicParams;             // pAvcPicParams[ucPPSIdx]
    uint16_t                                wPictureCodingType;
};

struct CodechalVp8InitMbencConstantBufferParams
{
    PMOS_INTERFACE                          pOsInterface;
    MOS_SURFACE                             sMBModeCostLumaBuffer;
    MOS_SURFACE                             sBlockModeCostBuffer;
    PMOS_RESOURCE                           presHistogram;
};

struct CodechalVp8InitPakBufferParams
{
    PMOS_INTERFACE                  pOsInterface;
    PMOS_RESOURCE                   presCoeffProbsBuffer;
    PMOS_RESOURCE                   presFrameHeaderBuffer;
    uint8_t                         *pHeader;
    uint32_t                        dwHeaderSize;
};

struct CodechalVp8UpdateMpuTpuBufferParams
{
    PMOS_INTERFACE                  pOsInterface;
    PMOS_RESOURCE                   presCurrFrameTokenProbability;
    PMOS_RESOURCE                   presHwTokenProbabilityPass1;
    PMOS_RESOURCE                   presKeyFrameTokenProbability;
    PMOS_RESOURCE                   presHwTokenProbabilityPass2;
    PMOS_RESOURCE                   presRepakDecisionSurface;
    uint32_t                        dwCoeffProbsSize;
};

struct CodechalVp8MpuTpuBuffers
{
    MOS_RESOURCE                       resRefModeProbs;
    MOS_RESOURCE                       resModeProbs;
    MOS_RESOURCE                       resRefCoeffProbs;
    MOS_RESOURCE                       resCoeffProbs;
    MOS_RESOURCE                       resPictureState;
    MOS_RESOURCE                       resMpuBitstream;
    MOS_RESOURCE                       resEntropyCostTable;
    MOS_RESOURCE                       resTokenBitsData;
    MOS_RESOURCE                       resTpuBitstream;
    MOS_RESOURCE                       resPakTokenStatistics;
    MOS_RESOURCE                       resPakTokenUpdateFlags;
    MOS_RESOURCE                       resDefaultTokenProbability;
    MOS_RESOURCE                       resKeyFrameTokenProbability;
    MOS_RESOURCE                       resUpdatedTokenProbability;
    MOS_RESOURCE                       resHwTokenProbabilityPass2;
    MOS_RESOURCE                       resRepakDecisionSurface;
};

struct CodecEncodeVp8DumpState
{
    MOS_SURFACE                         *sMbSegmentMapSurface;
    MOS_SURFACE                         *s4xMEDistortionBuffer;
    MHW_KERNEL_STATE                    *MpuKernelState;
    MHW_KERNEL_STATE                    *TpuKernelState;
    EncodeBrcBuffers                    *BrcBuffers;
    struct CodechalVp8MpuTpuBuffers     *MpuTpuBuffers;
    MOS_RESOURCE                        *resFrameHeader;
    bool                                *bBrcEnabled;
    MOS_RESOURCE                        *resModeCostUpdateSurface;
    MOS_RESOURCE                        *resHistogram;
};

struct CodechalEncodeVp8InitKernelStateParams
{
   PMHW_KERNEL_STATE               pKernelState;
   MhwRenderInterface             *pRenderEngineInterface;
   uint8_t*                        pui8Binary;
   EncOperation                    Operation;
   uint32_t                        dwKrnStateIdx;
   uint32_t                        dwCombinedKernelSize;
   int32_t                         iBtCount;
   int32_t                         iCurbeCount;
};


//!
//! \class   CodechalEncodeVp8
//! \brief   VP8 dual-pipe encoder base class
//! \details This class defines the base class for VP8 dual-pipe encoder, it includes
//!          common member fields, functions, interfaces etc shared by all GENs.
//!          Gen specific definitions, features should be put into their corresponding classes.
//!          To create a VP8 dual-pipe encoder instance, client needs to call CodechalEncodeVp8::CreateVp8State()
//!
class CodechalEncodeVp8 : public CodechalEncoderState
{
public:

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeVp8();

    //!
    //! \brief    Allocate resources for encoder instance
    //! \details  It is invoked when initializing encoder instance
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources();

    //!
    //! \brief    Free encoder resources
    //! \details  It is invoked when destorying encoder instance and it would call 
    //!           FreeEncResources(), FreeBrcResources() and FreePakResources()
    //!
    //! \return   void
    //!
    void FreeResources();

    //!
    //! \brief    Resize buffers due to resoluton change.
    //! \details  Resize buffers due to resoluton change.
    //!
    //! \return   void
    //!
    virtual void ResizeBuffer();

    //!
    //! \brief    Initialize encoder at picture level. Called by each frame.
    //!
    //! \param    [in] params
    //!           Picture encoding parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitializePicture(const EncoderParams& params);

    //!
    //! \brief    Encode kernel functions
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteKernelFunctions();

    //!
    //! \brief    Encode command at picture level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecutePictureLevel();

    //!
    //! \brief    Encode command at slice level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteSliceLevel();

    //!
    //! \brief    Initialize encoder instance
    //! \details  When GEN specific derived class implements this function to do its own initialization,
    //            it is required that the derived class calls #CodechalEncodeMpeg2::Initialize() first
    //            which would do common initialization for all GENs
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(CodechalSetting * codecHalSettings);

    //!
    //! \brief    Read Image Status
    //!
    //! \param    [out] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadImageStatus(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Read Mfc image Status
    //!
    //! \param    [out] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadMfcStatus(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Read Pak Statistics
    //!
    //! \param    [in]  params
    //!           Pointer to CODECHAL_ENCODE_READ_BRC_PAK_STATS_PARAMS
    //! \param    [out] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadBrcPakStatistics(
        PMOS_COMMAND_BUFFER cmdBuffer,
        EncodeReadBrcPakStatsParams* params);

    //!
    //! \brief    Get Status Report
    //!
    //! \param    [in]  encodeStatus
    //!           Pointer to CODECHAL_ENCODE_STATUS
    //! \param    [out] pEncodeStatusReport
    //!           Pointer to CODECHAL_ENCODE_STATUS_REPORT
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetStatusReport(
        EncodeStatus       *encodeStatus,
        EncodeStatusReport *pEncodeStatusReport);

    //!
    //! \brief    Initialize MMC state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitMmcState();

    CODEC_VP8_ENCODE_PIC_PARAMS *m_vp8PicParams = nullptr;  //<! Pointer to CodecVp8EncodePictureParams

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpMbEncPakOutput(PCODEC_REF_LIST currRefList, CodechalDebugInterface* debugInterface);
#endif // USE_CODECHAL_DEBUG_TOOL

protected:

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeVp8(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Allocate Resource of BRC
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_RESOURCES_BRC_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBrcResources (struct CodechalResourcesBrcParams*  params);

    //!
    //! \brief    Help function to allocate a 1D buffer
    //!
    //! \param    [in,out] buffer
    //!           Pointer to allocated buffer
    //! \param    [in] bufSize
    //!           Buffer size
    //! \param    [in] name
    //!           Buffer name
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBuffer(
        PMOS_RESOURCE   buffer,
        uint32_t        bufSize,
        PCCHAR          name);

    //!
    //! \brief    Help function to allocate a generic 2D surface
    //!
    //! \param    [in,out] surface
    //!           Pointer to allocated surface
    //! \param    [in] surfWidth
    //!           Surface width
    //! \param    [in] surfHeight
    //!           Surface height
    //! \param    [in] name
    //!           Surface name
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBuffer2D(
        PMOS_SURFACE    surface,
        uint32_t        surfWidth,
        uint32_t        surfHeight,
        PCCHAR          name);

    //!
    //! \brief    Help function to allocate a batch buffer
    //!
    //! \param    [in,out] batchBuffer
    //!           Pointer to allocated batch buffer
    //! \param    [in] bufSize
    //!           Buffer size
    //! \param    [in] name
    //!           Batch buffer name
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateBatchBuffer(
        PMHW_BATCH_BUFFER   batchBuffer,
        uint32_t            bufSize,
        PCCHAR              name);

    //!
    //! \brief    Free all Resources of BRC
    //!
    //! \return   void
    //!
    void FreeBrcResources();

    //!
    //! \brief    Initialize kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelState() = 0;

    //!
    //! \brief    Initialize Mpu Tpu Buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitMpuTpuBuffer() = 0;

    //!
    //! \brief    Set Curbe for BRC Init or Reset
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_BRC_INIT_RESET_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetBrcInitResetCurbe(struct CodechalVp8BrcInitResetCurbeParams* params) = 0;

    //!
    //! \brief    Send Surface for BRC Init or Reset
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_BRC_INIT_RESET_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendBrcInitResetSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp8BrcInitResetSurfaceParams* params) = 0;

    //!
    //! \brief    Set Curbe for BRC Update
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_BRC_UPDATE_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetBrcUpdateCurbe(struct CodechalVp8BrcUpdateCurbeParams*   params) = 0;

    //!
    //! \brief    Send Surface for BRC Update
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_BRC_UPDATE_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBrcUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp8BrcUpdateSurfaceParams*     params);

    //!
    //! \brief    Set Curbe for Mb Enc
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_MBENC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMbEncCurbe(struct CodechalVp8MbencCurbeParams* params) = 0;

    //!
    //! \brief    Set Curbe for ME
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_ME_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMeCurbe(struct CodechalVp8MeCurbeParams* params) = 0;

    //!
    //! \brief    Set Curbe for Mpu
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_MPU_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMpuCurbe(struct CodechalVp8MpuCurbeParams* params) = 0;

    //!
    //! \brief    Set Curbe for Tpu
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_TPU_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetTpuCurbe(struct CodechalVp8TpuCurbeParams* params) = 0;

    //!
    //! \brief    BRC Constant Buffer Initialize
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_INIT_BRC_CONSTANT_BUFFER_PARAMS params
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitBrcConstantBuffer(struct CodechalVp8InitBrcConstantBufferParams* params) = 0;

    //!
    //! \brief    BRC Distortion Buffer Initialize
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitBrcDistortionBuffer() = 0;

    //!
    //! \brief    Send Surface for ME
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_ME_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMeSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp8MeSurfaceParams*     params);

    //!
    //! \brief    Update MpuTpu Buffer with Key Frame
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_UPDATE_MPU_TPU_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS KeyFrameUpdateMpuTpuBuffer(struct CodechalVp8UpdateMpuTpuBufferParams* params) = 0;

    //!
    //! \brief    Send Surface for Mpu
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_MPU_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMpuSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp8MpuSurfaceParams*    params) = 0;

    //!
    //! \brief    Send Surface for Tpu
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_TPU_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendTpuSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp8TpuSurfaceParams*    params);

    //!
    //! \brief    MBEnc Constant Buffer initialize
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_INIT_MBENC_CONSTANT_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitMBEncConstantBuffer(struct CodechalVp8InitMbencConstantBufferParams*   params) = 0;

    //!
    //! \brief    Send Surface for Mb Enc
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CODECHAL_VP8_MBENC_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMbEncSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp8MbencSurfaceParams*  params);

    //!
    //! \brief    Set Pak Stats In Tpu Curbe
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetPakStatsInTpuCurbe(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup/configure encoder based on sequence parameter set
    //! \details  It is invoked when the encoder receives a new sequence parameter set and it would
    //!           set up and configure the encoder state that used for the sequence
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSequenceStructs();

    //!
    //! \brief    Setup/configure encoder based on picture parameter set
    //! \details  It is invoked for every picture and it would set up and configure the 
    //!           encoder state that used for current picture
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetPictureStructs();

    //!
    //! \brief    Invoke BRC Init/Reset kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BrcInitResetKernel();

    //!
    //! \brief    Invoke BRC update kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BrcUpdateKernel();

    //!
    //! \brief    Top level function for invoking MBenc kernel
    //!
    //! \param    [in]  isEncPhase1NotRun
    //!           Indicate if MbEnc Phase 1 is not enabled
    //! \param    [in]  isEncPhase2
    //!           Indicate if MbEnc Phase 2 is enabled
    //! \param    [in]  mbEncIFrameDistInUse
    //!           Indicate if MbEnc I-Frame distortion is enabled
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS MbEncKernel(
        bool    isEncPhase1NotRun,
        bool    isEncPhase2,
        bool    mbEncIFrameDistInUse);

    //!
    //! \brief    Invoke ME kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS MeKernel();

    //!
    //! \brief    Invoke MPU kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS MpuKernel();

    //!
    //! \brief    Invoke TPU kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS TpuKernel();

    //!
    //! \brief    Retrieves the MFC registers and stores them in the dump buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] resource
    //!           Pointer to MOS_RESOURCE
    //! \param    [in] baseOffset
    //!           Offset of base
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetPakStatsDebugBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE       resource,
        uint32_t            baseOffset);

    //!
    //! \brief    Encode BRC command at slice level
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeSliceLevelBrc(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Add Batch Buffer End to Picture State Command 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddBBEndToPicStateCmd();

    //!
    //! \brief    Get maximum BT count
    //!
    //! \return   uint32_t
    //!           Maximum BT count
    //!
    uint32_t GetMaxBtCount();

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpVp8EncodePicParams(PCODEC_VP8_ENCODE_PIC_PARAMS picParams);

    MOS_STATUS DumpVp8EncodeSeqParams(PCODEC_VP8_ENCODE_SEQUENCE_PARAMS seqParams);
#endif // USE_CODECHAL_DEBUG_TOOL

    MEDIA_FEATURE_TABLE *                   m_skuTable    = nullptr;                             // SKU table
    MEDIA_WA_TABLE *                        m_waTable     = nullptr;                             // SKU table

    // Parameters passed by application
    CODEC_VP8_ENCODE_SEQUENCE_PARAMS *  m_vp8SeqParams   = nullptr;
    CODEC_VP8_ENCODE_QUANT_DATA *       m_vp8QuantData   = nullptr;
    CODECHAL_VP8_HYBRIDPAK_FRAMEUPDATE *m_vp8SliceParams = nullptr;

    uint8_t*                               m_kernelBinary = nullptr;                            //!< Pointer to the kernel binary
    uint32_t                               m_combinedKernelSize = 0;                            //!< Combined kernel binary size

    CODEC_PIC_ID    m_picIdx[CODEC_MAX_NUM_REF_FRAME_NON_AVC];
    PCODEC_REF_LIST m_refList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8];

    bool     m_hmeEnabled = false;
    bool     m_b16XMeEnabled = false;
    bool     m_hmeDone = false;
    bool     m_b16XMeDone = false;
    bool     m_refCtrlOptimizationDone = false;
    bool     m_brcInit = false;
    bool     m_brcReset = false;
    bool     m_brcEnabled = false;
    bool     m_mbBrcEnabled = false;
    bool     m_mbEncIFrameDistEnabled = false;
    bool     m_brcDistortionBufferSupported = false;
    bool     m_initBrcDistortionBuffer = false;
    bool     m_brcConstantBufferSupported = false;
    bool     m_brcSegMapSupported = false;
    bool     m_mbEncCurbeSetInBrcUpdate;
    bool     m_mbPakCurbeSetInBrcUpdate = false;
    bool     m_mpuCurbeSetInBrcUpdate;
    bool     m_tpuCurbeSetInBrcUpdate;
    bool     m_mfxEncoderConfigCommandInitialized = false;
    bool     m_adaptiveRepakSupported = false;
    bool     m_repakSupported = false;
    uint16_t m_usMinPakPasses = 0;
    uint16_t m_usRepakPassIterVal = 0;  // n th pass when Repak is executed

    // MB Enc
    MHW_KERNEL_STATE                    m_mbEncKernelStates[CODECHAL_ENCODE_VP8_MBENC_IDX_NUM];
    uint32_t                            m_numMbEncEncKrnStates = 0;
    uint32_t                            m_mbEncIFrameDshSize = 0;
    struct CodechalBindingTableVp8Mbenc m_mbEncBindingTable;
    uint32_t                            m_mbEncBlockBasedSkipEn = 0;
    MOS_RESOURCE                        m_resRefMbCountSurface;
    MOS_SURFACE                         m_mbModeCostLumaBuffer = {};
    MOS_SURFACE                         m_blockModeCostBuffer = {};
    MOS_RESOURCE                        m_chromaReconBuffer;  // for fixed function VP8
    MOS_SURFACE                         m_perMbQuantDataBuffer = {};
    MOS_RESOURCE                        m_resPredMvDataSurface;
    MOS_RESOURCE                        m_resHistogram;
    MOS_RESOURCE                        m_resModeCostUpdateSurface;
    // MBRC = 1: internal segment map (sInSegmentMapSurface) is provided from BRC update kernel
    // MBRC = 0: external segment map (sMbSegmentMapSurface) is provided from the app, ignore internal segment map
    MOS_SURFACE m_inSegmentMapSurface = {};
    MOS_SURFACE m_mbSegmentMapSurface = {};  // var of type MOS_SURFACE of Mb segment map surface

    // MPU & TPU Buffers
    struct CodechalVp8MpuTpuBuffers m_mpuTpuBuffers;

    // TPU
    MHW_KERNEL_STATE                  m_tpuKernelState;
    struct CodechalBindingTableVp8Tpu m_tpuBindingTable = {};

    // MPU
    MHW_KERNEL_STATE                  m_mpuKernelState;
    struct CodechalBindingTableVp8Mpu m_mpuBindingTable = {};

    // VME Scratch Buffers
    MOS_RESOURCE m_resVmeKernelDumpBuffer;
    bool         m_vmeKernelDump;

    //HW pak
    MOS_RESOURCE m_resIntraRowStoreScratchBuffer;
    MOS_RESOURCE m_resFrameHeader;
    MOS_RESOURCE m_resPakIntermediateBuffer;

    // ME
    MHW_KERNEL_STATE                 m_meKernelState;
    struct CodechalBindingTableVp8Me m_meBindingTable = {};
    MOS_SURFACE                      m_s4XMemvDataBuffer;
    MOS_SURFACE                      m_s16XMemvDataBuffer = {};
    MOS_SURFACE                      m_s4XMeDistortionBuffer;

    uint32_t m_averageKeyFrameQp = 0;
    uint32_t m_averagePFrameQp = 0;
    uint32_t m_pFramePositionInGop = 0;
    // BRC Params, these parameters not used for BDW
    MHW_KERNEL_STATE                        m_brcKernelStates[CODECHAL_ENCODE_VP8_BRC_IDX_NUM];
    struct CodechalBindingTableVp8BrcUpdate m_brcUpdateBindingTable = {};
    EncodeBrcBuffers                        m_brcBuffers;
    uint16_t                                m_usAvbrAccuracy = 0;
    uint16_t                                m_usAvbrConvergence = 0;
    double                                  m_dBrcInitCurrentTargetBufFullInBits = 0;
    double                                  m_dBrcInitResetInputBitsPerFrame = 0;
    uint32_t                                m_brcInitResetBufSizeInBits = 0;
    uint32_t                                m_brcConstantSurfaceWidth = 0;
    uint32_t                                m_brcConstantSurfaceHeight = 0;

    uint32_t m_frameRate = 0;
};

#endif  // __CODECHAL_ENCODER_VP8_H__
