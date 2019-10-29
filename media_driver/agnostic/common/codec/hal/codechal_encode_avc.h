/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_encode_avc.h
//! \brief    This file defines the base C++ class/interface for AVC DualPipe encoding
//!           to be used across CODECHAL components.
//!

#ifndef __CODECHAL_ENCODE_AVC_H__
#define __CODECHAL_ENCODE_AVC_H__

#include "codechal_encode_avc_base.h"

#define CODECHAL_ENCODE_AVC_MAX_LAMBDA                                  0xEFFF

// BRC Block Copy
#define CODECHAL_ENCODE_AVC_BRC_COPY_NUM_ROWS_PER_VME_SEND_MSG          8
#define CODECHAL_ENCODE_AVC_BRC_COPY_NUM_SEND_MSGS_PER_KERNEL           3
#define CODECHAL_ENCODE_AVC_BRC_COPY_BLOCK_WIDTH                        64

// SubMbPartMask defined in CURBE for AVC ENC
#define CODECHAL_ENCODE_AVC_DISABLE_4X4_SUB_MB_PARTITION                0x40
#define CODECHAL_ENCODE_AVC_DISABLE_4X8_SUB_MB_PARTITION                0x20
#define CODECHAL_ENCODE_AVC_DISABLE_8X4_SUB_MB_PARTITION                0x10

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_BRC_INIT_RESET
{
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_HISTORY = 0,
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_DISTORTION,
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_BRC_INIT_RESET;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_BRC_BLOCK_COPY
{
    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_INPUT = 0,
    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_OUTPUT,
    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_NUM_SURFACES
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_BRC_BLOCK_COPY;

typedef struct _CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS
{
    PMHW_KERNEL_STATE                       pKernelState;
    uint32_t                                dwBufferOffset;
    uint32_t                                dwBlockHeight;
} CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS, *PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS
{
    PMOS_INTERFACE                              pOsInterface;
    PCODEC_AVC_ENCODE_SLICE_PARAMS              pAvcSlcParams;
    PCODEC_PIC_ID                               pAvcPicIdx;
    MOS_SURFACE                                 sBrcConstantDataBuffer; // sBrcConstantDataBuffer[uiCurrDSH]
    uint32_t                                    dwMbEncBlockBasedSkipEn;
    PCODEC_AVC_ENCODE_PIC_PARAMS                pPicParams;             // pAvcPicParams[ucPPSIdx]
    uint16_t                                    wPictureCodingType;
    bool                                        bSkipBiasAdjustmentEnable;
    bool                                        bAdaptiveIntraScalingEnable;
    bool                                        bOldModeCostEnable;
    PCODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS    pAvcQCParams;
} CODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS, *PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_BRC_INIT_RESET_SURFACE_PARAMS
{
    PMOS_RESOURCE                       presBrcHistoryBuffer;
    PMOS_SURFACE                        psMeBrcDistortionBuffer;
    uint32_t                            dwMeBrcDistortionBottomFieldOffset;
    uint32_t                            dwDownscaledWidthInMb4x;
    uint32_t                            dwDownscaledFrameFieldHeightInMb4x;
} CODECHAL_ENCODE_AVC_BRC_INIT_RESET_SURFACE_PARAMS, *PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_SURFACE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS
{
    double                                 *pdBrcInitCurrentTargetBufFullInBits;   // Passed back to Render Interface
    double                                 *pdBrcInitResetInputBitsPerFrame;       // Passed back to Render Interface
    uint32_t*                               pdwBrcInitResetBufSizeInBits;           // Passed back to Render Interface
    PMHW_KERNEL_STATE                       pKernelState;
} CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS, *PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS
{
    PMOS_INTERFACE                              pOsInterface;
    PMOS_RESOURCE                               presBrcConstantDataBuffer;
    uint32_t                                    dwMbEncBlockBasedSkipEn;
    PCODEC_AVC_ENCODE_PIC_PARAMS                pPicParams;             // pAvcPicParams[ucPPSIdx]
    uint16_t                                    wPictureCodingType;
    bool                                        bSkipBiasAdjustmentEnable;
    bool                                        bAdaptiveIntraScalingEnable;
    bool                                        bOldModeCostEnable;
    bool                                        bPreProcEnable;
    bool                                        bEnableKernelTrellis;
    PCODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS    pAvcQCParams;
    uint32_t                                    Lambda[52][2];
} CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS, *PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS
{
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS           pSeqParams;
    PCODEC_AVC_ENCODE_PIC_PARAMS                pPicParams;
    PCODEC_AVC_ENCODE_SLICE_PARAMS              pSlcParams;
    PCODEC_REF_LIST                             *ppRefList;
    PCODEC_PIC_ID                               pPicIdx;
    PCODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS    pAvcQCParams;
    uint16_t                                    wPicWidthInMb;
    uint16_t                                    wFieldFrameHeightInMb;
    uint32_t*                                   pdwBlockBasedSkipEn; // To be returned to render interface
    bool                                        bBrcEnabled;
    bool                                        bMbEncIFrameDistEnabled;
    bool                                        bRoiEnabled;
    bool                                        bDirtyRoiEnabled;
    bool                                        bUseMbEncAdvKernel;
    bool                                        bMbDisableSkipMapEnabled;
    bool                                        bStaticFrameDetectionEnabled;   // static frame detection enable or not
    bool                                        bApdatvieSearchWindowSizeEnabled;
    bool                                        bSquareRollingIEnabled;
    uint16_t                                    usSliceHeight;
    PMHW_KERNEL_STATE                           pKernelState;
    uint8_t*                                    pCurbeBinary;
} CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS, *PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS
{
    double                                  *pdBrcInitCurrentTargetBufFullInBits;    // Passed in and back
    uint32_t                                dwNumSkipFrames;
    uint32_t                                dwSizeSkipFrames;
    uint8_t                                 ucMinQP;  // Limit min QP that the kernel can choose, based on app setting
    uint8_t                                 ucMaxQP;  // Limit max QP that the kernel can choose, based on app setting
    uint8_t                                 ucEnableROI;                            // ROI feature for BRC
    uint32_t                                dwIntraRefreshQpThreshold;
    bool                                    bSquareRollingIEnabled;
    PMHW_KERNEL_STATE                       pKernelState;
} CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS, *PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS;
typedef struct _CODECHAL_ENCODE_AVC_BINDING_TABLE_BRC_UPDATE
{
    uint32_t   dwFrameBrcHistoryBuffer;
    uint32_t   dwFrameBrcPakStatisticsOutputBuffer;
    uint32_t   dwFrameBrcImageStateReadBuffer;
    uint32_t   dwFrameBrcImageStateWriteBuffer;
    uint32_t   dwFrameBrcMbEncCurbeReadBuffer;
    uint32_t   dwFrameBrcMbEncCurbeWriteData;
    uint32_t   dwFrameBrcDistortionBuffer;
    uint32_t   dwFrameBrcConstantData;
    uint32_t   dwFrameBrcMbStatBuffer;
    uint32_t   dwFrameBrcMvDataBuffer;
    uint32_t   dwMbBrcHistoryBuffer;
    uint32_t   dwMbBrcDistortionBuffer;
    uint32_t   dwMbBrcMbQpBuffer;
    uint32_t   dwMbBrcROISurface;
    uint32_t   dwMbBrcIntraDistortionPBFrameSurface;
    uint32_t   dwMbBrcMbStatBuffer;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
} CODECHAL_ENCODE_AVC_BINDING_TABLE_BRC_UPDATE, *PCODECHAL_ENCODE_AVC_BINDING_TABLE_BRC_UPDATE;

typedef struct _CODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS
{
    CODECHAL_MEDIA_STATE_TYPE                       MbEncMediaStateType;
    EncodeBrcBuffers*                               pBrcBuffers;
    uint32_t                                        dwDownscaledWidthInMb4x;
    uint32_t                                        dwDownscaledFrameFieldHeightInMb4x;
    bool                                            bMbBrcEnabled;
    bool                                            bUseAdvancedDsh;
    bool                                            bBrcRoiEnabled;
    PMOS_RESOURCE                                   presMbEncCurbeBuffer;
    PMOS_RESOURCE                                   presMbEncBRCBuffer; //extra surface on KBL BRC update
    PMOS_SURFACE                                    psRoiSurface;
    PMOS_RESOURCE                                   presMbStatBuffer;
    PMOS_SURFACE                                    psMvDataBuffer;
    uint32_t                                        dwBrcPakStatisticsSize;
    uint32_t                                        dwBrcHistoryBufferSize;
    uint32_t                                        dwMbEncBRCBufferSize;
    uint32_t                                        dwMvBottomFieldOffset;
    uint8_t                                         ucCurrRecycledBufIdx;
    PCODECHAL_ENCODE_AVC_BINDING_TABLE_BRC_UPDATE   pBrcUpdateBindingTable;
    PMHW_KERNEL_STATE                               pKernelState;
} CODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS, *PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_IPCM_THRESHOLD
{
    uint8_t   QP;
    uint16_t  Threshold;
} CODECHAL_ENCODE_AVC_IPCM_THRESHOLD;

// Force RepartitionCheck
typedef enum _CODECHAL_ENCODE_AVC_RPC
{
    CODECHAL_ENCODE_RPC_FOLLOW_DRIVER = 0,
    CODECHAL_ENCODE_RPC_FORCE_ENABLE,
    CODECHAL_ENCODE_RPC_FORCE_DISABLE
} CODECHAL_ENCODE_AVC_RPC;

typedef struct _CODECHAL_ENCODE_AVC_BINDING_TABLE_MBENC
{
    uint32_t   dwAvcMBEncMfcAvcPakObj;
    uint32_t   dwAvcMBEncIndMVData;
    uint32_t   dwAvcVPPStatistics;
    uint32_t   dwAvcMBEncCurrY;
    uint32_t   dwAvcMBEncCurrUV;
    uint32_t   dwAvcMBEncMbSpecificData;
    uint32_t   dwAvcMBEncFwdRefY;
    uint32_t   dwAvcMBEncBwdRefY;
    uint32_t   dwAvcMBEncFwdRefMVData;
    uint32_t   dwAvcMBEncBwdRefMBData;
    uint32_t   dwAvcMBEncBwdRefMVData;
    uint32_t   dwAvcMBEncMVDataFromME;
    uint32_t   dwAvcMBEncRefPicSelectL0;
    uint32_t   dwAvcMBEncRefPicSelectL1;
    uint32_t   dwAvcMBEncCurrPic;
    uint32_t   dwAvcMBEncFwdPic;
    uint32_t   dwAvcMBEncBwdPic;
    uint32_t   dwAvcMBEncMbBrcConstData;
    uint32_t   dwAvcMBEncMEDist;
    uint32_t   dwAvcMBEncBRCDist;
    uint32_t   dwAvcMBEncDebugScratch;
    uint32_t   dwAvcMBEncFlatnessChk;
    uint32_t   dwAvcMBEncMBStats;
    uint32_t   dwAvcMBEncMADData;
    uint32_t   dwAvcMBEncVMEDistortion;
    uint32_t   dwAvcMbEncBRCCurbeData;
    uint32_t   dwAvcMBEncSliceMapData;
    uint32_t   dwAvcMBEncMvPrediction;
    uint32_t   dwAvcMBEncMbNonSkipMap;
    uint32_t   dwAvcMBEncAdv;
    uint32_t   dwAvcMBEncStaticDetectionCostTable;

    // Frame Binding Table Entries
    uint32_t   dwAvcMBEncCurrPicFrame[CODEC_AVC_NUM_REF_LISTS];
    uint32_t   dwAvcMBEncFwdPicFrame[CODECHAL_ENCODE_NUM_MAX_VME_L0_REF];
    uint32_t   dwAvcMBEncBwdPicFrame[CODECHAL_ENCODE_NUM_MAX_VME_L1_REF * 2];   // Bwd ref IDX0 and IDX1 are repeated, so include two extra to account for this
    uint32_t   dwAvcMBEncMbQpFrame;
    uint32_t   dwAvcMbEncMADFrame;
    uint32_t   dwAvcMBEncSliceMapFrame;
    uint32_t   dwAvcMBEncMbNonSkipMapFrame;

    // Field Binding Table Entries
    uint32_t   dwAvcMBEncFieldCurrPic[CODEC_AVC_NUM_REF_LISTS];
    uint32_t   dwAvcMBEncFwdPicTopField[CODECHAL_ENCODE_NUM_MAX_VME_L0_REF];
    uint32_t   dwAvcMBEncFwdPicBotField[CODECHAL_ENCODE_NUM_MAX_VME_L0_REF];
    uint32_t   dwAvcMBEncBwdPicTopField[CODECHAL_ENCODE_NUM_MAX_VME_L1_REF * 2];
    uint32_t   dwAvcMBEncBwdPicBotField[CODECHAL_ENCODE_NUM_MAX_VME_L1_REF * 2];
    uint32_t   dwAvcMBEncMbQpField;
    uint32_t   dwAvcMBEncMADField;
    uint32_t   dwAvcMBEncSliceMapField;
    uint32_t   dwAvcMBEncMbNonSkipMapField;

    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
} CODECHAL_ENCODE_AVC_BINDING_TABLE_MBENC, *PCODECHAL_ENCODE_AVC_BINDING_TABLE_MBENC;

typedef struct _CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS
{
    CODECHAL_MEDIA_STATE_TYPE                   MediaStateType;
    PCODEC_AVC_ENCODE_SLICE_PARAMS              pAvcSlcParams;
    CodecEncodeAvcFeiPicParams                  *pFeiPicParams;
    PCODEC_REF_LIST                             *ppRefList;
    PCODEC_PIC_ID                               pAvcPicIdx;
    PCODEC_PICTURE                              pCurrOriginalPic;
    PCODEC_PICTURE                              pCurrReconstructedPic;
    uint16_t                                    wPictureCodingType;
    PMOS_SURFACE                                psCurrPicSurface;
    uint32_t                                    dwCurrPicSurfaceOffset;
    uint32_t                                    dwMbCodeBottomFieldOffset;
    uint32_t                                    dwMvBottomFieldOffset;
    PMOS_SURFACE                                ps4xMeMvDataBuffer;
    CmSurface2D                                 *ps4xMeMvDataCmBuffer;
    uint32_t                                    dwMeMvBottomFieldOffset;
    PMOS_SURFACE                                ps4xMeDistortionBuffer;
    CmSurface2D                                 *ps4xMeDistortionCmBuffer;
    uint32_t                                    dwMeDistortionBottomFieldOffset;
    uint32_t                                    dwRefPicSelectBottomFieldOffset;
    PMOS_SURFACE                                psMeBrcDistortionBuffer;
    uint32_t                                    dwMeBrcDistortionBottomFieldOffset;
    PMOS_RESOURCE                               presMbBrcConstDataBuffer;
    PMOS_RESOURCE                               presMbSpecificDataBuffer;
    PMOS_SURFACE                                psMbQpBuffer;
    uint32_t                                    dwMbQpBottomFieldOffset;
    bool                                        bFlatnessCheckEnabled;
    PMOS_SURFACE                                psFlatnessCheckSurface;
    uint32_t                                    dwFlatnessCheckBottomFieldOffset;
    bool                                        bMBVProcStatsEnabled;
    PMOS_RESOURCE                               presMBVProcStatsBuffer;
    uint32_t                                    dwMBVProcStatsBottomFieldOffset;
    PMOS_RESOURCE                               presMADDataBuffer;
    bool                                        bMADEnabled;
    uint32_t                                    dwFrameWidthInMb;
    uint32_t                                    dwFrameFieldHeightInMb;
    uint32_t                                    dwFrameHeightInMb;
    uint32_t                                    dwVerticalLineStride;
    uint32_t                                    dwVerticalLineStrideOffset;
    bool                                        bHmeEnabled;
    bool                                        bMbEncIFrameDistInUse;
    bool                                        bMbQpBufferInUse;
    bool                                        bMbSpecificDataEnabled;
    bool                                        bMbConstDataBufferInUse;
    bool                                        bUsedAsRef;
    PMOS_RESOURCE                               presMbEncCurbeBuffer;
    PMOS_RESOURCE                               presMbEncBRCBuffer;  //extra surface on KBL BRC update
    uint32_t                                    dwMbEncBRCBufferSize;
    bool                                        bUseMbEncAdvKernel;
    bool                                        bArbitraryNumMbsInSlice;
    PMOS_SURFACE                                psSliceMapSurface;           // Slice map for arbitrary number of mbs in slice feature
    uint32_t                                    dwSliceMapBottomFieldOffset;
    bool                                        bBrcEnabled;
    PMHW_KERNEL_STATE                           pKernelState;
    bool                                        bUseAdvancedDsh;
    PCODECHAL_ENCODE_AVC_BINDING_TABLE_MBENC    pMbEncBindingTable;
    bool                                        bMbDisableSkipMapEnabled;
    PMOS_SURFACE                                psMbDisableSkipMapSurface;
    bool                                        bStaticFrameDetectionEnabled;
    PMOS_RESOURCE                               presSFDOutputBuffer;
    PMOS_RESOURCE                               presSFDCostTableBuffer;
    PCODEC_AVC_REF_PIC_SELECT_LIST              pWeightedPredOutputPicSelectList;
    bool                                        bUseWeightedSurfaceForL0;
    bool                                        bUseWeightedSurfaceForL1;
} CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS, *PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS
{
    uint32_t                                    submitNumber;
    PMHW_KERNEL_STATE                           pKernelState;
    PCODECHAL_ENCODE_AVC_BINDING_TABLE_MBENC    pBindingTable;
} CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS, *PCODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS;

typedef enum _CODECHAL_ENCODE_AVC_MULTIPRED
{
    CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE            =   0x01,
    CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE           =   0x80
} CODECHAL_ENCODE_AVC_MULTIPRED;

typedef struct _CODECHAL_ENCODE_AVC_BINDING_TABLE_PREPROC
{
    uint32_t   dwAvcPreProcCurrY;
    uint32_t   dwAvcPreProcCurrUV;
    uint32_t   dwAvcPreProcMVDataFromHME;
    uint32_t   dwAvcPreProcMvPredictor;
    uint32_t   dwAvcPreProcMbQp;
    uint32_t   dwAvcPreProcMvDataOut;
    uint32_t   dwAvcPreProcMbStatsOut;

    uint32_t   dwAvcPreProcVMECurrPicFrame[CODEC_AVC_NUM_REF_LISTS];
    uint32_t   dwAvcPreProcVMEFwdPicFrame;
    uint32_t   dwAvcPreProcVMEBwdPicFrame[2];   // Bwd ref IDX0 and IDX1 are repeated, so include two to account for this
    uint32_t   dwAvcPreProcFtqLut;

    uint32_t   dwAvcPreProcVMECurrPicField[2];
    uint32_t   dwAvcPreProcVMEFwdPicField[2];
    uint32_t   dwAvcPreProcVMEBwdPicField[2];
    uint32_t   dwAvcPreProcFtqLutField;

    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
} CODECHAL_ENCODE_AVC_BINDING_TABLE_PREPROC, *PCODECHAL_ENCODE_AVC_BINDING_TABLE_PREPROC;

typedef struct _CODECHAL_ENCOCDE_AVC_PREPROC_SURFACE_PARAMS
{
    FeiPreEncParams                             *pPreEncParams;
    PCODEC_REF_LIST                             *ppRefList;
    PCODEC_PICTURE                              pCurrOriginalPic;
    PMOS_SURFACE                                psCurrPicSurface;
    PMOS_SURFACE                                ps4xMeMvDataBuffer;
    PMOS_RESOURCE                               presFtqLutBuffer;
    uint32_t                                    dwMeMvBottomFieldOffset;
    uint32_t                                    dwMBVProcStatsBottomFieldOffset;
    uint32_t                                    dwFrameWidthInMb;
    uint32_t                                    dwFrameFieldHeightInMb;
    uint32_t                                    dwVerticalLineStride;
    uint32_t                                    dwVerticalLineStrideOffset;
    bool                                        bHmeEnabled;
    PMHW_KERNEL_STATE                           pKernelState;
    PCODECHAL_ENCODE_AVC_BINDING_TABLE_PREPROC  pPreProcBindingTable;
} CODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS, *PCODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS
{
    FeiPreEncParams                         *pPreEncParams;
    uint16_t                                wPicWidthInMb;
    uint16_t                                wFieldFrameHeightInMb;
    PMHW_KERNEL_STATE                       pKernelState;
    uint8_t*                                pCurbeBinary;
} CODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS, *PCODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_WP_CURBE_PARAMS
{
    uint8_t   RefPicListIdx;
    uint32_t  WPIdx;
} CODECHAL_ENCODE_AVC_WP_CURBE_PARAMS, *PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS
{
    PMOS_SURFACE                       psInputRefBuffer;
    PMOS_SURFACE                       psOutputScaledBuffer;
    uint32_t                           dwVerticalLineStride;
    uint32_t                           dwVerticalLineStrideOffset;
    uint8_t                            ucVDirection;
    PMHW_KERNEL_STATE                  pKernelState;
} CODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS, *PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS;

//!
//! \struct    CodechalEncodeAvcEnc
//! \brief     Codechal encode Avc Encode
//!
struct CodechalEncodeAvcEnc : public CodechalEncodeAvcBase
{

    typedef struct _CODECHAL_ENCODE_AVC_MB_SPECIFIC_PARAMS
    {
        union
        {
            struct
            {
                uint32_t   ForceToIntra                                : MOS_BITFIELD_RANGE( 0, 0 );
                uint32_t   Reserved                                    : MOS_BITFIELD_RANGE( 1,31 );
            };
            struct
            {
                uint32_t   Value;
            };
        } DW0;

        union
        {
            struct
            {
                uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
            };
            struct
            {
                uint32_t   Value;
            };
        } DW1;

        union
        {
            struct
            {
                uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
            };
            struct
            {
                uint32_t   Value;
            };
        } DW2;

        union
        {
            struct
            {
                uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,31 );
            };
            struct
            {
                uint32_t   Value;
            };
        } DW3;
    } CODECHAL_ENCODE_AVC_MB_SPECIFIC_PARAMS, *PCODECHAL_ENCODE_AVC_MB_SPECIFIC_PARAMS;
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_MB_SPECIFIC_PARAMS)) == 4);

    // SEI
    CodechalEncodeSeiData               SeiData;                                            //!< Encode SEI data parameter.
    uint32_t                            dwSEIDataOffset;                                    //!< Encode SEI data offset.
    uint8_t*                            pSeiParamBuffer;                                    //!< Encode SEI data buffer.

    bool                                bMbEncCurbeSetInBrcUpdate;                          //!< bMbEncCurbeSetInBrcUpdate.
    bool                                bMbEncIFrameDistEnabled;                            //!< bMbEncIFrameDistEnabled.
    bool                                bBrcInit;                                           //!< BRC init enable flag.
    bool                                bBrcReset;                                          //!< BRC reset enable flag.
    bool                                bBrcEnabled;                                        //!< BRC enable flag.
    bool                                bMbBrcEnabled;                                      //!< MBBrc enable flag.
    bool                                bBrcRoiEnabled;                                     //!< BRC ROI feature enable flag.
    bool                                bROIValueInDeltaQP;                                 //!< ROI QP in delta QP flag.
    bool                                bROISmoothEnabled;                                  //!< ROI smooth area enable flag.
    bool                                bMbBrcUserFeatureKeyControl;                        //!< MBBRC user feature control enable flag.
    double                              dBrcTargetSize;                                     //!< BRC target size.
    uint32_t                            dwTrellis;                                          //!< Trellis Number.
    bool                                bAcceleratorHeaderPackingCaps;                      //!< Flag set by driver from driver caps.
    uint32_t                            dwIntraRefreshQpThreshold;                          //!< Intra Refresh QP Threshold.
    bool                                bSquareRollingIEnabled;                             //!< SquareRollingI enable flag.

    // VME Scratch Buffers
    MOS_RESOURCE                        resVMEScratchBuffer;                                //!< VME Scratch Buffer resource.
    bool                                bVMEScratchBuffer;                                  //!< VME ScratchBuffer enable flag.
    MOS_RESOURCE                        resVmeKernelDumpBuffer;                             //!< VME Kernel Dump resource.
    bool                                bVMEKernelDump;                                     //!< VME kernel dump flag.
    uint32_t                            ulVMEKernelDumpBottomFieldOffset;                   //!< VME Kernel Dump Bottom Field Offset

    // MbEnc
    PMHW_KERNEL_STATE                           pMbEncKernelStates;                                             //!< Pointer to MbEnc Kernel States.
    CODECHAL_ENCODE_AVC_BINDING_TABLE_MBENC     MbEncBindingTable;                                              //!< Pointer to MbEnc BindingTable.
    uint32_t                                    dwNumMbEncEncKrnStates;                                         //!< MbEncEncKrnStates Number.
    CODEC_AVC_REF_PIC_SELECT_LIST               RefPicSelectList[CODECHAL_ENCODE_AVC_REF_PIC_SELECT_ENTRIES];   //!< Array of RefPicSelect.
    uint8_t                                     ucCurrRefPicSelectIndex;                                        //!< Current RefPic Select Index
    uint32_t                                    ulRefPicSelectBottomFieldOffset;                                //!< RefPic Select BottomField Offset
    uint32_t                                    dwMbEncBlockBasedSkipEn;                                        //!< MbEnc Block Based Skip enable flag.
    bool                                        bKernelTrellis;                                                 //!< Kernel controlled Trellis Quantization.
    bool                                        bExtendedMvCostRange;                                           //!< Extended MV Cost Range for Gen10+.

    //MFE MbEnc
    MHW_KERNEL_STATE                            mfeMbEncKernelState;                                             //!< Mfe MbEnc Kernel State.

    // Intra Distortion
    PMHW_KERNEL_STATE                           pIntraDistortionKernelStates;                                   //!< Point to Intra Distortion Kernel States.

    // WP
    PMHW_KERNEL_STATE                   pWPKernelState;                                                         //!< Point to WP Kernel State.
    CODEC_AVC_REF_PIC_SELECT_LIST       WeightedPredOutputPicSelectList[CODEC_AVC_NUM_WP_FRAME];                //!< Array of WeightedPredOutputPicSelectList.

    // BRC Params
    MHW_KERNEL_STATE                                BrcKernelStates[CODECHAL_ENCODE_BRC_IDX_NUM];               //!< Array of BrcKernelStates.
    CODECHAL_ENCODE_AVC_BINDING_TABLE_BRC_UPDATE    BrcUpdateBindingTable;                                      //!< BrcUpdate BindingTable

    // PreProc
    MHW_KERNEL_STATE                                PreProcKernelState;                                         //!< PreProc KernelState
    CODECHAL_ENCODE_AVC_BINDING_TABLE_PREPROC       PreProcBindingTable;                                        //!< PreProc BindingTable

    EncodeBrcBuffers                    BrcBuffers;                                                     //!< BRC related buffers
    uint16_t                            usAVBRAccuracy;                                                 //!< AVBR Accuracy
    uint16_t                            usAVBRConvergence;                                              //!< AVBR Convergence
    double                              dBrcInitCurrentTargetBufFullInBits;                             //!< BRC init current target buffer full in bits
    double                              dBrcInitResetInputBitsPerFrame;                                 //!< BrcInitReset Input Bits Per Frame
    uint32_t                            dwBrcInitResetBufSizeInBits;                                    //!< BrcInitReset Buffer Size In Bits
    uint32_t                            dwBrcInitPreviousTargetBufFullInBits;                           //!< BRC Init Previous Target Buffer Full In Bits
    // Below values will be set if qp control params are sent by app
    bool                                bMinMaxQPControlEnabled;                                        //!< Flag to indicate if min/max QP feature is enabled or not.
    uint8_t                             ucIMinQP;                                                       //!< I frame Minimum QP.
    uint8_t                             ucIMaxQP;                                                       //!< I frame Maximum QP.
    uint8_t                             ucPMinQP;                                                       //!< P frame Minimum QP.
    uint8_t                             ucPMaxQP;                                                       //!< P frame Maximum QP.
    uint8_t                             ucBMinQP;                                                       //!< B frame Minimum QP.
    uint8_t                             ucBMaxQP;                                                       //!< B frame Maximum QP.
    bool                                bPFrameMinMaxQPControl;                                         //!< Indicates min/max QP values for P-frames are set separately or not.
    bool                                bBFrameMinMaxQPControl;                                         //!< Indicates min/max QP values for B-frames are set separately or not.

    uint32_t                            dwSkipFrameBufferSize;                                          //!< size of skip frame packed data.
    MOS_RESOURCE                        resSkipFrameBuffer;                                             //!< copy skip frame packed data from DDI.
    // Mb Disable Skip Map
    bool                                bMbDisableSkipMapEnabled;                                       //!< MbDisableSkipMap Flag.
    PMOS_SURFACE                        psMbDisableSkipMapSurface;                                      //!< Point to MbDisableSkipMap Surface.

    // Mb Qp Data
    bool                                bMbQpDataEnabled;                                               //!< Mb Qp Data Enable Flag.
    MOS_SURFACE                         sMbQpDataSurface;                                               //!< Pointer to MOS_SURFACE of Mb Qp data surface, provided by DDI.

        // Mb specific Data
    bool                                bMbSpecificDataEnabled;
    MOS_RESOURCE                        resMbSpecificDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];

    // Static frame detection
    bool                                bStaticFrameDetectionEnable;                                    //!< Static frame detection enable.
    bool                                bApdatvieSearchWindowEnable;                                    //!< allow search window size change when SFD enabled.
    bool                                bPerMbSFD;                                                      //!<
    MOS_RESOURCE                        resSFDOutputBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];        //!< Array of SFDOutputBuffer.
    MOS_RESOURCE                        resSFDCostTablePFrameBuffer;                                    //!< SFD CostTable of P Frame.
    MOS_RESOURCE                        resSFDCostTableBFrameBuffer;                                    //!< SFD CostTable of B Frame.
    PMHW_KERNEL_STATE                   pSFDKernelState;                                                //!< Point to SFD kernel state.

    // Generation Specific Support Flags & User Feature Key Reads
    bool                                bBrcDistortionBufferSupported;                                  //!< BRC DistortionBuffer Support Flag.
    bool                                bRefPicSelectListSupported;                                     //!< RefPicSelectList Support Flag.
    uint8_t                             ucMbBrcSupportCaps;                                             //!< MbBrcSupport Capability.
    bool                                bMultiPredEnable;                                               //!< MultiPredictor enable, 6 predictors
    bool                                bFTQEnable;                                                     //!< FTQEnable
    bool                                bCAFSupported;                                                  //!< CAFSupported
    bool                                bCAFEnable;                                                     //!< CAFEnable
    bool                                bCAFDisableHD;                                                  //!< Disable CAF for HD
    bool                                bSkipBiasAdjustmentSupported;                                   //!< SkipBiasAdjustment support for P frame
    bool                                bAdaptiveIntraScalingEnable;                                    //!< Enable AdaptiveIntraScaling
    bool                                bOldModeCostEnable;                                             //!< Enable Old Mode Cost (HSW cost table for BDW)
    bool                                bMultiRefQpEnabled;                                             //!< BDW MultiRef QP
    bool                                bAdvancedDshInUse;                                              //!< Use MbEnc Adv kernel
    bool                                bUseMbEncAdvKernel;                                             //!< Use MbEnc Adv kernel
    bool                                bUseWeightedSurfaceForL0;                                       //!< Use WP Surface for L0
    bool                                bUseWeightedSurfaceForL1;                                       //!< Use WP Surface for L1
    bool                                bWeightedPredictionSupported;                                   //!< Weighted prediction support
    bool                                bBrcSplitEnable;                                                //!< starting GEN9 BRC kernel has split into frame-level and MB-level update.
    bool                                bDecoupleMbEncCurbeFromBRC;                                     //!< starting GEN95 BRC kernel write to extra surface instead of MBEnc curbe.
    bool                                bSliceLevelReportSupported;                                     //!< Slice Level Report support
    bool                                bFBRBypassEnable;                                               //!< FBRBypassEnable
    bool                                bBrcRoiSupported;                                               //!< BRC Roi Support Flag.
    bool                                bMvDataNeededByBRC;                                             //!< starting G95, mv data buffer from HME is needed by BRC frame update kernel.
    bool                                bHighTextureModeCostEnable;                                     //!< HighTexture ModeCost Enable Flag.

    bool                                bRoundingInterEnable;                                           //!< RoundingInter Enable Flag.
    bool                                bAdaptiveRoundingInterEnable;                                   //!< Adaptive Rounding Inter Enable Flag.
    uint32_t                            dwRoundingInterP;                                               //!< Rounding Inter for P frame
    uint32_t                            dwRoundingInterB;                                               //!< Rounding Inter for B frame
    uint32_t                            dwRoundingInterBRef;                                            //!< Rounding Inter for BRef frame
    uint32_t                            dwBrcConstantSurfaceWidth;                                      //!< Brc Constant Surface Width
    uint32_t                            dwBrcConstantSurfaceHeight;                                     //!< Brc Constant Surface Height

    uint32_t                            dwSlidingWindowSize;                                            //!< Sliding Window Size
    bool                                bForceToSkipEnable;                                             //!< ForceToSkip Enable Flag
    bool                                bBRCVarCompuBypass;                                             //!< Bypass variance computation in BRC kernel

    static const uint32_t MaxLenSP[NUM_TARGET_USAGE_MODES];
    static const uint32_t EnableAdaptiveSearch[NUM_TARGET_USAGE_MODES];
    static const uint32_t FTQBasedSkip[NUM_TARGET_USAGE_MODES];
    static const uint32_t HMEBCombineLen[NUM_TARGET_USAGE_MODES];
    static const uint32_t HMECombineLen[NUM_TARGET_USAGE_MODES];
    static const uint32_t SearchX[NUM_TARGET_USAGE_MODES];
    static const uint32_t SearchY[NUM_TARGET_USAGE_MODES];
    static const uint32_t BSearchX[NUM_TARGET_USAGE_MODES];
    static const uint32_t BSearchY[NUM_TARGET_USAGE_MODES];

    static const uint32_t InterRoundingP_TQ[NUM_TARGET_USAGE_MODES];
    static const uint32_t InterRoundingBRef_TQ[NUM_TARGET_USAGE_MODES];
    static const uint32_t InterRoundingB_TQ[NUM_TARGET_USAGE_MODES];
    static const uint32_t TrellisQuantizationEnable[NUM_TARGET_USAGE_MODES];
    static const uint32_t EnableAdaptiveTrellisQuantization[NUM_TARGET_USAGE_MODES];
    static const uint32_t TQ_LAMBDA_I_FRAME[CODEC_AVC_NUM_QP][2];
    static const uint32_t TQ_LAMBDA_P_FRAME[CODEC_AVC_NUM_QP][2];
    static const uint32_t TQ_LAMBDA_B_FRAME[CODEC_AVC_NUM_QP][2];
    static const uint8_t  IntraScalingFactor_Cm_Common[64];
    static const uint8_t  AdaptiveIntraScalingFactor_Cm_Common[64];
    static const uint32_t OldIntraModeCost_Cm_Common[CODEC_AVC_NUM_QP];
    static const uint32_t MvCost_PSkipAdjustment_Cm_Common[CODEC_AVC_NUM_QP];
    static const uint16_t SkipVal_B_Common[2][2][64];
    static const uint16_t SkipVal_P_Common[2][2][64];
    static const uint32_t PreProcFtqLut_Cm_Common[CODEC_AVC_NUM_QP][16];
    static const uint32_t MBBrcConstantData_Cm_Common[3][CODEC_AVC_NUM_QP][16];

    static const uint32_t ModeMvCost_Common[3][CODEC_AVC_NUM_QP][8];
    static const uint16_t RefCost_Common[3][64];
    static const uint8_t  MaxRefIdx0_Progressive_4K[NUM_TARGET_USAGE_MODES];
    static const uint8_t  MaxRefIdx0[NUM_TARGET_USAGE_MODES];
    static const uint8_t  MaxBRefIdx0[NUM_TARGET_USAGE_MODES];
    static const uint8_t  MaxRefIdx1[NUM_TARGET_USAGE_MODES];
    static const uint32_t SuperHME[NUM_TARGET_USAGE_MODES];
    static const uint32_t UltraHME[NUM_TARGET_USAGE_MODES];

    static const uint32_t ModeMvCost_Cm[3][52][8];

    static const uint8_t  m_qpDistMaxFrameAdjustmentCm[576];
    static const uint32_t MultiPred[NUM_TARGET_USAGE_MODES];
    static const uint32_t MRDisableQPCheck[NUM_TARGET_USAGE_MODES];
    static const uint16_t RefCost_MultiRefQp[NUM_PIC_TYPES][64];
    static const uint32_t CODECHAL_ENCODE_AVC_AllFractional_Common[NUM_TARGET_USAGE_MODES];
    static const uint32_t CODECHAL_ENCODE_AVC_DisableAllFractionalCheckForHighRes_Common[NUM_TARGET_USAGE_MODES];

    static const uint32_t m_refThreshold = 400;
    static const uint32_t m_mbencNumTargetUsages = 3;
    static const uint32_t m_brcConstantSurfaceEarlySkipTableSize = 128;
    static const uint32_t m_brcConstantSurfaceRefCostSize = 128;
    static const uint32_t m_brcConstantSurfacModeMvCostSize = 1664;

    typedef enum _ID_OFFSET_MBENC_CM
    {
        MBENC_I_OFFSET_CM = 0,
        MBENC_P_OFFSET_CM = 1,
        MBENC_B_OFFSET_CM = 2,
        MBENC_TARGET_USAGE_CM = 3
    } ID_OFFSET_MBENC_CM;

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAvcEnc(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalEncodeAvcEnc(const CodechalEncodeAvcEnc&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalEncodeAvcEnc& operator=(const CodechalEncodeAvcEnc&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeAvcEnc();

    //Encode interface implementations
    //!
    //! \brief    Initialize standard related members.
    //! \details  Initialize members, set proper value
    //!           to involved variables, allocate resources.
    //!
    //! \param    [in] settings
    //!           Encode settings.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS Initialize(
        CodechalSetting * settings) override;

    //!
    //! \brief    Initialize encoder related members.
    //! \details  Initialize members, set proper value
    //!           to involved variables, allocate resources.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitializePicture(const EncoderParams& params) override;

    //!
    //! \brief    Call media kernel functions.
    //! \details  Call to related encode kernel functions.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS ExecuteKernelFunctions() override;

    //!
    //! \brief    Encode frame in picture level.
    //! \details  Call related encode functions to encode
    //!           one frame in picture level.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS ExecutePictureLevel() override;

    //!
    //! \brief    Encode frame in slice level.
    //! \details  Call related encode functions to encode
    //!           one frame in slice level.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS ExecuteSliceLevel() override;

    //!
    //! \brief    Encode User Feature Key Report.
    //! \details  Report user feature values set by encode.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS UserFeatureKeyReport() override;

    virtual MOS_STATUS ExecutePreEnc(EncoderParams* encodeParams) override;
    //!
    //! \brief    Slice map surface programming
    //! \details  Set slice map data.
    //! 
    //! \param    [in] data
    //!           Encode interface
    //! \param    [in] avcSliceParams
    //!           Slice map command params
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS EncodeGenerateSliceMap(
        uint8_t* data,
        PCODEC_AVC_ENCODE_SLICE_PARAMS avcSliceParams);

    //!
    //! \brief    Allocate necessary resources.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS AllocateResources() override;

    //!
    //! \brief    Allocate necessary resources for BRC case.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS AllocateResourcesBrc();

    //!
    //! \brief    Allocate necessary resources for MBBRC case.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS AllocateResourcesMbBrc();

    //!
    //! \brief    Release allocated resources for BRC case.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS ReleaseResourcesBrc();

    //!
    //! \brief    Release allocated resources for MBBRC case.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS ReleaseResourcesMbBrc();

    //!
    //! \brief    AVC Enc State Initialization.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS Initialize() override;

    //!
    //! \brief    Generic State Picture Level Encoding..
    //!
    //! \param    [in] params
    //!           Pointer to the CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS GenericEncodePictureLevel(
        PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS   params);

    //!
    //! \brief    Run Encode ME kernel
    //!
    //! \param    [in] brcBuffers
    //!           Pointer to the EncodeBrcBuffers
    //! \param    [in] hmeLevel
    //!           Hme level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS GenericEncodeMeKernel(EncodeBrcBuffers* brcBuffers, HmeLevel hmeLevel);

    //!
    //! \brief    Initialize Encode ME kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitKernelStateMe() override;

    //!
    //! \brief    Set Encode ME kernel Curbe data.
    //!
    //! \param    [in] params
    //!           Pointer to the MeCurbeParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SetCurbeMe (
        MeCurbeParams* params) override;

    //!
    //! \brief    Set Encode ME kernel Surfaces
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to the PMOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to the CODECHAL_ME_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SendMeSurfaces (
        PMOS_COMMAND_BUFFER cmdBuffer,
        MeSurfaceParams* params) override;

    // AvcState functions.
    //!
    //! \brief    Initialize data members of AVC encoder instance
    //! 
    //! \return   void
    //!
    virtual void InitializeDataMember();

    // state related funcs
    //!
    //! \brief    Initialize encode state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitializeState();

    //!
    //! \brief    Validate reference list L0 and L1.
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ValidateNumReferences(
        PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS               params);

    //!
    //! \brief    Initialize brc constant buffer
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitBrcConstantBuffer(
        PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS        params);

    //!
    //! \brief    Initialize mbbrc constant buffer
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitMbBrcConstantDataBuffer(
        PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params);

    //!
    //! \brief    Get inter rounding value.
    //!
    //! \param    [in] sliceState
    //!           Pointer to MHW_VDBOX_AVC_SLICE_STATE
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetInterRounding(
        PMHW_VDBOX_AVC_SLICE_STATE sliceState);

    //!
    //! \brief    Calculate lambda table.
    //!
    //! \param    [in] slice_type
    //!           Slice type.
    //! \param    [out] lambda
    //!           Lambda table.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CalcLambdaTable(
        uint16_t slice_type,
        uint32_t* lambda);

    //!
    //! \brief    Get Trellis Quantization mode/value enable or not.
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS.
    //! \param    [out] trellisQuantParams
    //!           Pointer to CODECHAL_ENCODE_AVC_TQ_PARAMS, mode & value setup.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS    params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS          trellisQuantParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get Skip Bias Adjustment.
    //!
    //! \param    [in] sliceQP
    //!           Slice QP.
    //! \param    [in] gopRefDist
    //!           GOP reference dist.
    //! \param    [in] skipBiasAdjustmentEnable
    //!           Adjustable or not.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetSkipBiasAdjustment(
        uint8_t sliceQP,
        uint16_t gopRefDist,
        bool* skipBiasAdjustmentEnable);

    //!
    //! \brief    Get Hme Supported Based On TU.
    //!
    //! \param    [in] hmeLevel
    //!           HME level
    //! \param    [out] supported
    //!           Supported or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHmeSupportedBasedOnTU(
        HmeLevel hmeLevel,
        bool *supported);

    //!
    //! \brief    Get MbBrc status.
    //!
    //! \param    [in] targetUsage
    //!           Target Usage.
    //! \param    [out] mbBrcEnabled
    //!           MBBRC enabled or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMbBrcEnabled(
        uint32_t                    targetUsage,
        bool                       *mbBrcEnabled);

    //!
    //! \brief    CAF enabled or not.
    //!
    //! \param    [out] cafEnable
    //!           CAF enabled or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetCAFEnabled(
        bool *cafEnable);

    //!
    //! \brief    ATD enabled or not.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetATDEnabled();

    //!
    //! \brief    Init BRC reset kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BrcInitResetKernel();

    //!
    //! \brief    Init MbEnc kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateMbEnc()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Init Weight Prediction kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateWP()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Init BRC kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateBrc()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Init FEI PreProc kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStatePreProc()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Init SFD(still frame detection) kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateSFD();

    //!
    //! \brief    Init kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelState();

    //!
    //! \brief    Insert RefPic Select List
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InsertInRefPicSelectList();

    //!
    //! \brief    Run MbEnc Kernel.
    //!
    //! \param    [in] mbEncIFrameDistInUse
    //!           MbEncIFrameDist in use or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS MbEncKernel(
        bool mbEncIFrameDistInUse);

    //!
    //! \brief    Run Brc Frame Update Kernel.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BrcFrameUpdateKernel();

    //!
    //! \brief    Run Brc Copy Kernel.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BrcCopyKernel();

    //!
    //! \brief    Run Brc MB update Kernel.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BrcMbUpdateKernel();

    //!
    //! \brief    Run MbEnc Kernel.
    //!
    //! \param    [in] useRefPicList1
    //!           Use RefPicList 1 or Not.
    //! \param    [in] index
    //!           Index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS WPKernel(
                    bool useRefPicList1,
                    uint32_t index);

    //!
    //! \brief    Run SFD(still frame detection) kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SFDKernel();

    //!
    //! \brief    Get MbEnc kernel state Idx
    //!
    //! \param    [in] params
    //!           Pointer to the CodechalEncodeIdOffsetParams
    //! \param    [in] kernelOffset
    //!           kernel offset 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMbEncKernelStateIdx(
        CodechalEncodeIdOffsetParams*          params,
        uint32_t*                              kernelOffset)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get MbEnc kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMbEnc(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set MFE MbEnc kernel curbe data
    //!
    //! \param    [in] mbEncIFrameDistInUse
    //!           MbEncIFrameDist in use or not
    //!
    //! \return   BOOL
    //!           true if MFE MbEnc is enabled, otherwise false
    //!
    virtual bool IsMfeMbEncEnabled(
        bool mbEncIFrameDistInUse)
    {
        return false;
    }

    //!
    //! \brief    Init Mfe MbEnc kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateMfeMbEnc()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Defer init MFE specific resoruces and flags.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitMfe()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set MFE MbEnc kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMfeMbEnc(
        PCODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Update binding table for MFE MbEnc kernel
    //!
    //! \param    [in] submitIndex
    //!           Index in this mfe submission call
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateMfeMbEncBindingTable(
        uint32_t submitIndex)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get Weighted Prediction kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_WP_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcWP(
        PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get FEI PreProc kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcPreProc(
        PCODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get BRC InitReset kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcBrcInitReset(
        PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get FrameBRCUpdate kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcFrameBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get MbBrcUpdate kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMbBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get Brc Block Copy kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcBrcBlockCopy(
        PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get SFD kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcSFD(
        PCODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS params);

    //!
    //! \brief    Set Sequence Structs
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetSequenceStructs() override;

    //!
    //! \brief    Set Sequence Structs
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetPictureStructs() override;

    //!
    //! \brief    Set slice Structs
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSliceStructs() override;

    //!
    //! \brief    Set BRC InitReset kernel Surface
    //!
    //! \param    [in] cmdBuffer
    //!           Cmd Buffer
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_INIT_RESET_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBrcInitResetSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_SURFACE_PARAMS params);

    //!
    //! \brief    Set MbEnc kernel Surface data
    //!
    //! \param    [in] cmdBuffer
    //!           Cmd Buffer
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcMbEncSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set Weighted Prediction kernel Surface state
    //!
    //! \param    [in] cmdBuffer
    //!           Cmd Buffer
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcWPSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set FEI PreProc kernel Surface state
    //!
    //! \param    [in] cmdBuffer
    //!           Cmd Buffer
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcPreProcSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set BrcFrameUpdate kernel Surface state
    //!
    //! \param    [in] cmdBuffer
    //!           Cmd Buffer
    //! \param    [in] params
    //!           pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcBrcFrameUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set BrcMbUpdate kernel Surface state
    //!
    //! \param    [in] cmdBuffer
    //!           Cmd Buffer
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcBrcMbUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set SFD kernel Surface state
    //!
    //! \param    [in] cmdBuffer
    //!           Cmd Buffer
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_SFD_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcSFDSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_SFD_SURFACE_PARAMS params);

    //!
    //! \brief    Set ROI kernel Surface state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupROISurface()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Inserts the generic prologue command for a command buffer
    //! \param  [in] cmdBuffer
    //!         Command buffer
    //! \param  [in] frameTracking
    //!         Indicate if frame tracking requested
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        bool                        frameTracking,
        MHW_MI_MMIOREGISTERS       *mmioRegister = nullptr) override;

    //!
    //! \brief  Realize the scene change report
    //! \param  [in] cmdBuffer
    //!         Command buffer
    //!         [in]  params
    //!           Pointer to the CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SceneChangeReport(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS  params)
    {
        return MOS_STATUS_SUCCESS;
    };

    //! \brief    Dump encode kernel output
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpEncodeKernelOutput();

    //!
    //! \brief    Calculate skip value
    //! \param    [in] encBlockBasedSkipEn
    //!           Indicate if encode block Based skip enabled
    //! \param    [in] transform8x8Flag
    //!           Indicate if transform8*8 makes effect
    //! \param    [in] skipVal
    //!           input Skip value
    //! \return   uint16_t
    //!           return the updated Skip value
    //!
    uint16_t CalcSkipVal(
        bool    encBlockBasedSkipEn,
        bool    transform8x8Flag,
        uint16_t  skipVal);

    //!
    //! \brief    Get Max MV value per 2 mbs based on LevelIdc
    //! \details  VDBOX private function to get max MV value per 2 mbs
    //! \param    [in] levelIdc
    //!           AVC level
    //! \return   uint32_t
    //!           return the max mv value per 2 mbs
    //!
    uint32_t GetMaxMvsPer2Mb(uint8_t levelIdc);

     //!
     //! \brief    Get QP value
     //! \param    [in] params 
     //!           AVC mbenc cubre params
     //! \param    [in] list
     //!           forword or backword reference
     //! \param    [in] index
     //!           reference frame index
     //! \return   uint8_t
     //!           return 0
    uint32_t GetRefPicFieldFlag(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS     params,
        uint32_t                                    list,
        uint32_t                                    index);

     //!
     //! \brief    Get QP value
     //! \param    [in] params 
     //!           AVC mbenc cubre params
     //! \param    [list] list
     //!           forword or backword reference
     //! \param    [in] index
     //!           reference frame index
     //! \return   uint8_t
     //!           return 0
    uint8_t AVCGetQPValueFromRefList(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS     params,
        uint32_t                                    list,
        uint32_t                                    index);

     //!
     //! \brief    Send surfaces for the AVC BRC Block Copy kernel
     //! \param    [in] hwInterface 
     //!           Hardware interface
     //! \param    [in] cmdBuffer
     //!           comand buffer
     //! \param    [in] mbEncKernelState 
     //!           MB encoder kernel state 
     //! \param    [in] kernelState 
     //!           Kernel State
     //! \param    [in] presAdvancedDsh
     //!           pmos resource
     //! \return   MOS_STATUS
     //!           MOS_STATUS_SUCCESS if success, else fail reason
     //!
    MOS_STATUS SendBrcBlockCopySurfaces(
        CodechalHwInterface    *hwInterface,
        PMOS_COMMAND_BUFFER     cmdBuffer,
        PMHW_KERNEL_STATE       mbEncKernelState,
        PMHW_KERNEL_STATE       kernelState,
        PMOS_RESOURCE           presAdvancedDsh);

#if USE_CODECHAL_DEBUG_TOOL
protected:
    virtual MOS_STATUS DumpSeqParFile()  override;
    virtual MOS_STATUS DumpFrameParFile() override;

    virtual MOS_STATUS PopulateHmeParam(
        bool    is16xMeEnabled,
        bool    is32xMeEnabled,
        uint8_t meMethod,
        void    *cmd) override;
#endif
};
#endif  // __CODECHAL_ENCODE_AVC_H__
