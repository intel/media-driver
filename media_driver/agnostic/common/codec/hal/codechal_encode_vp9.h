/*
* Copyright (c) 2019-2020 Intel Corporation
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
//! \file     codechal_encode_vp9.h
//! \brief    This file defines the base C++ class/interface for VP8 DualPipe encoding
//!           to be used across CODECHAL components.
//!

#ifndef __CODECHAL_ENCODER_VP9_H__
#define __CODECHAL_ENCODER_VP9_H__

#include "codechal_encoder_base.h"
#include "codec_def_common_vp9.h"
#include "codec_def_vp9_probs.h"

#define CODECHAL_ENCODE_VP9_NUM_REF_MBCODE_BUFFERS              (CODEC_VP9_NUM_REF_FRAMES + 1)

// 4K is just an estimation
#define CODECHAL_ENCODE_VP9_FRAME_HEADER_SIZE                   4096
#define CODECHAL_ENCODE_VP9_MAX_NAL_UNIT_TYPE                   1   // only support one NAL unit for uncompressed header

#define CODECHAL_ENCODE_VP9_CQP_NUM_OF_PASSES                   2
#define CODECHAL_ENCODE_VP9_BRC_DEFAULT_NUM_OF_PASSES           2   // 2 Passes minimum so HuC is Run twice, second PAK is conditional.
#define CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES               4
#define CODECHAL_ENCODE_VP9_MIN_TILE_SIZE                       128

#define CODECHAL_ENCODE_VP9_NUM_SYNC_TAGS                       36
#define CODECHAL_ENCODE_VP9_INIT_DSH_SIZE                       (MHW_PAGE_SIZE * 3)

#define CODECHAL_ENCODE_VP9_INVALID_PIC_ID                      CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9
#define CODECHAL_ENCODE_VP9_REF_SEGMENT_DISABLED                0xFF

#define CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER		80
#define CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS      192 // 42 uint32_ts for Pic State one uint32_t for BB End + 5 uint32_tS reserved to make it aligned for kernel read
#define CODECHAL_ENCODE_VP9_SEGMENT_STATE_BUFFER_SIZE_PER_PASS  256

#define CODECHAL_ENCODE_VP9_HUC_DMEM_SIZE						1280
#define TEMP_CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE            32

#define TEMP_CODECHAL_ENCODE_VP9_BRC_HISTORY_BUFFER_SIZE             608
#define CODECHAL_ENCODE_VP9_BRC_CONSTANTSURFACE_SIZE            17792
#define CODECHAL_ENCODE_VP9_BRC_BITSTREAM_SIZE_BUFFER_SIZE      16
#define CODECHAL_ENCODE_VP9_BRC_MSDK_PAK_BUFFER_SIZE            64
#define TEMP_CODECHAL_ENCODE_VP9_BRC_SUPER_FRAME_BUFFER_SIZE         MOS_ALIGN_CEIL(3 + 2 * sizeof(uint32_t), sizeof(uint32_t));

//this Target Usage modes are specified in VP9 Encode DDI specification.
static const uint8_t CodecHal_TargetUsageToMode_VP9[NUM_TARGET_USAGE_MODES] =
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

enum CodechalEncodeVp9MbencKernelStateIdx
{
    CODECHAL_ENCODE_VP9_MBENC_IDX_INTRA_32x32        = 0,
    CODECHAL_ENCODE_VP9_MBENC_IDX_INTRA_16x16        = 1,
    CODECHAL_ENCODE_VP9_MBENC_IDX_INTER              = 2,
    CODECHAL_ENCODE_VP9_MBENC_IDX_TX                 = 3,
    CODECHAL_ENCODE_VP9_MBENC_IDX_NUM                = 4 
};

enum CodechalEncodeVp9BrcKernelStateIdx
{
    CODECHAL_ENCODE_VP9_BRC_IDX_INTRA_DIST           = 0,
    CODECHAL_ENCODE_VP9_BRC_IDX_INIT                 = 1,
    CODECHAL_ENCODE_VP9_BRC_IDX_RESET                = 2,
    CODECHAL_ENCODE_VP9_BRC_IDX_UPDATE               = 3,
    CODECHAL_ENCODE_VP9_BRC_IDX_NUM                  = 4    
};

// VP9 encode prediction mode
enum VP9_PRED_MODE
{
    VP9_PRED_MODE_SINGLE    = 0,
    VP9_PRED_MODE_COMPOUND  = 1,
    VP9_PRED_MODE_HYBRID    = 2
};

//*------------------------------------------------------------------------------
//* Codec Definitions
//*------------------------------------------------------------------------------

struct CodechalBindingTableVp9Me
{
    uint32_t   dwMEMVDataSurface;
    uint32_t   dw16xMEMVDataSurface;
    uint32_t   dwMeDist;
    uint32_t   dwMeBrcDist;
    uint32_t   dwMeCurrPicL0;
    uint32_t   dwMeCurrPicL1;
};

struct CodechalBindingTableVp9MbencI32x32
{
    uint32_t   dwMbEncCurrY;
    uint32_t   dwMbEncCurrUV;
    uint32_t   dwMbEncSegmentationMap;
    uint32_t   dwMbEncModeDecision;
};

struct CodechalBindingTableVp9MbencI16x16
{
    uint32_t   dwMbEncCurrY;
    uint32_t   dwMbEncCurrUV;
    uint32_t   dwMbEncCurrNV12;
    uint32_t   dwMbEncSegmentationMap;
    uint32_t   dwMbEncTxCurbe;
    uint32_t   dwMbEncModeDecision;
};

struct CodechalBindingTableVp9MbencP{
    uint32_t   dwMbEncCurrY;
    uint32_t   dwMbEncCurrUV;
    uint32_t   dwMbEncCurrNV12;
    uint32_t   dwMbEncLastRefPic;
    uint32_t   dwMbEncGoldenRefPic;
    uint32_t   dwMbEncAlternateRefPic;
    uint32_t   dwMbEncHmeMvData;
    uint32_t   dwMbEncHmeDistortion;
    uint32_t   dwMbEncSegmentationMap;
    uint32_t   dwMbEncTxCurbe;
    uint32_t   dwMbEncModeDecisionPrevious;
    uint32_t   dwMbEncModeDecision;
    uint32_t   dwMbEncOutputInterModes16x16;
};

struct CodechalBindingTableVp9MbencTx
{
    uint32_t   dwMbEncCurrY;
    uint32_t   dwMbEncCurrUV;
    uint32_t   dwMbEncSegmentationMap;
    uint32_t   dwMbEncModeDecision;
    uint32_t   dwMbEncCuRecords;
    uint32_t   dwMbEncPakData;
};

struct CodechalBindingTableVp9BrcIntraDist
{
    uint32_t   dwIntraDistSrcY4XSurface;
    uint32_t   dwBrcIntraDistVmeCoarseIntraSurf;
    uint32_t   dwBrcIntraDistDistortionBuffer;
};

struct CodechalBindingTableVp9BrcInitReset
{
    uint32_t   dwBrcHistoryBuffer;
    uint32_t   dwBrcDistortionBuffer;
};

struct CodechalBindingTableVp9BrcUpdate
{
    uint32_t   dwBrcHistoryBuffer;
    uint32_t   dwBrcConstantData;
    uint32_t   dwBrcDistortionBuffer;
    uint32_t   dwBrcMSDKPakSurfaceBuffer;
    uint32_t   dwBrcMbEncCurbeData;
    uint32_t   dwBrcMbPakIntraLumaReconCurbeData;
    uint32_t   dwBrcMbPakIntraChromaReconCurbeData;
    uint32_t   dwBrcMbPakLFMaskCurbeData;
    uint32_t   dwBrcMbPakInterLumaReconCurbeData;
    uint32_t   dwBrcMbPakInterChromaReconCurbeData;
    uint32_t   dwBrcMbPakInterLumaRecon32x32CurbeData;
    uint32_t   dwBrcMbEncCurbeInput;
    uint32_t   dwBrcMbEncCurbeOutput;
    uint32_t   dwBrcPicStateInput;
    uint32_t   dwBrcPicStateOutput;
    uint32_t   dwBrcSegmentStateInput;
    uint32_t   dwBrcSegmentStateOutput;
    uint32_t   dwBrcBitstreamSizeData;
    uint32_t   dwBrcHucData;
};

struct CodechalVp9MbencCurbeParams
{
    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS           pSeqParams;
    PCODEC_VP9_ENCODE_PIC_PARAMS                pPicParams;
    PCODEC_VP9_ENCODE_SEGMENT_PARAMS            pSegmentParams;
    PCODEC_REF_LIST                             *ppRefList;
    uint16_t                                    wPicWidthInMb;
    uint16_t                                    wFieldFrameHeightInMb;
    uint16_t                                    wPictureCodingType;
    bool                                        bHmeEnabled;
    uint8_t                                     ucRefFrameFlags;
    CODECHAL_MEDIA_STATE_TYPE                   mediaStateType;
    PMHW_KERNEL_STATE                           pKernelState;
    PMHW_KERNEL_STATE                           pKernelStateI32x32; 
    PMOS_SURFACE                                psLastRefPic;
    PMOS_SURFACE                                psGoldenRefPic;
    PMOS_SURFACE                                psAltRefPic;
    bool                                        bMbEncCurbeSetInBrcUpdate;
    bool                                        bMultiRefQpCheck;
};

struct CodechalVp9BrcCurbeParams
{
    CODEC_PICTURE                                   CurrPic;
    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS               pSeqParams;
    PCODEC_VP9_ENCODE_PIC_PARAMS                    pPicParams;
    PCODEC_VP9_ENCODE_SEGMENT_PARAMS                pSegmentParams;
    uint16_t                                        wPictureCodingType;
    double                                          *pdBrcInitCurrentTargetBufFullInBits;   // Passed back to Render Interface
    double                                          *pdBrcInitResetInputBitsPerFrame;       // Passed back to Render Interface
    uint32_t                                        *pdwBrcInitResetBufSizeInBits;           // Passed back to Render Interface
    uint32_t                                        dwFrameWidth;
    uint32_t                                        dwFrameHeight;
    uint32_t                                        dwFrameWidthInMB;
    uint32_t                                        dwFrameHeightInMB;
    uint8_t                                         ucRefFrameFlags;
    bool                                            bHmeEnabled;
    bool                                            bInitBrc;
    bool                                            bMbBrcEnabled;
    bool                                            bUsedAsRef;
    uint8_t                                         ucKernelMode;                           // Normal/Quality/Performance
    uint32_t                                        dwBrcNumPakPasses;
    uint32_t                                        dwHeaderBytesInserted;  // dwHeaderBytesInserted is for WAAVCSWHeaderInsertion and is 0 otherwise
    bool                                            bMultiRefQpCheck;
    uint16_t                                        sFrameNumber;
    uint32_t                                        uiFramerate;
    CODECHAL_MEDIA_STATE_TYPE                       mediaStateType;
    PMHW_KERNEL_STATE                               pKernelState;
};

struct CodechalVp9MeCurbeParams
{
    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS       pSeqParams;
    PCODEC_VP9_ENCODE_PIC_PARAMS            pPicParams;
    uint32_t                                dwFrameWidth;
    uint32_t                                dwFrameFieldHeight;
    uint32_t                                ucRefFrameFlags;
    bool                                    b16xME;
    bool                                    b16xMeEnabled;
    PMHW_KERNEL_STATE                       pKernelState;
};

struct CodechalEncodeVp9InitKernelStateParams
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

struct CodechalVp9MeSurfaceParams
{
    PCODEC_REF_LIST                         *ppRefList;
    PCODEC_PICTURE                          pCurrReconstructedPic;
    PCODEC_PICTURE                          pLastRefPic;
    PCODEC_PICTURE                          pGoldenRefPic;
    PCODEC_PICTURE                          pAlternateRefPic;    
    PMOS_SURFACE                            ps4xMeMvDataBuffer;
    PMOS_SURFACE                            ps16xMeMvDataBuffer;
    PMOS_SURFACE                            psMeDistortionBuffer;
    PMOS_SURFACE                            psMeBrcDistortionBuffer;
    uint32_t                                dwDownscaledWidthInMb;
    uint32_t                                dwDownscaledHeightInMb;
    uint32_t                                dwVerticalLineStride;
    uint32_t                                dwVerticalLineStrideOffset;
    uint32_t                                dwEncodeWidth;
    uint32_t                                dwEncodeHeight;
    bool                                    b16xMeInUse;
    bool                                    b16xMeEnabled;
    bool                                    bDysEnabled;
    struct CodechalBindingTableVp9Me*       pMeBindingTable;
    PMHW_KERNEL_STATE                       pKernelState;
};

struct CodechalVp9MbencSurfaceParams
{
    CODECHAL_MEDIA_STATE_TYPE           MediaStateType;
    PCODEC_REF_LIST                     *ppRefList;
    PMOS_SURFACE                        psLastRefPic;
    PMOS_SURFACE                        psGoldenRefPic;
    PMOS_SURFACE                        psAltRefPic;
    uint16_t                            wPictureCodingType;
    PMOS_SURFACE                        psCurrPicSurface;
    uint32_t                            dwCurrPicSurfaceOffset;
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
    bool                                bSegmentationEnabled;
    uint32_t                            dwMbDataOffset;
    PMOS_RESOURCE                       presMbCodeSurface;
    PMOS_SURFACE                        psSegmentationMap;
    PMOS_SURFACE                        psModeDecisionPrevious;
    PMOS_SURFACE                        psModeDecision;
    PMOS_RESOURCE                       presModeDecisionPrevious;
    PMOS_RESOURCE                       presModeDecision;
    PMOS_RESOURCE                       presMbEncCurbeBuffer;
    PMOS_SURFACE                        psOutput16x16InterModes;
    PMOS_SURFACE                        psModeDecision_I_32x32;
    void*                               pvBindingTable;
    PMHW_KERNEL_STATE                   pKernelState;
    PMHW_KERNEL_STATE                   pKernelStateTx;
    bool                                bUseEncryptedDsh;
};

struct CodechalVp9BrcIntraDistSurfaceParams
{
    PCODEC_REF_LIST                     *ppRefList;
    PCODEC_PICTURE                      pCurrOriginalPic;
    PCODEC_PICTURE                      pCurrReconstructedPic;
    PMOS_SURFACE                        psSrcY4xSurface;
    PMOS_SURFACE                        psVmeCoarseIntraSurface;
    PMOS_SURFACE                        psBrcDistortionBuffer;
    uint32_t                            dwDownscaledWidthInMb4x;
    uint32_t                            dwDownscaledFrameHeightInMb4x;
    void*                               pvBindingTable;
    PMHW_KERNEL_STATE                   pKernelState;
};

struct CodechalVp9BrcInitResetSurfaceParams
{
    PMOS_RESOURCE                       presBrcHistoryBuffer;
    PMOS_SURFACE                        psBrcDistortionBuffer;
    uint32_t                            dwDownscaledWidthInMb4x;
    uint32_t                            dwDownscaledFrameHeightInMb4x;
    uint32_t                            dwBrcHistoryBufferSize;
    void*                               pvBindingTable;
    PMHW_KERNEL_STATE                   pKernelState;
};

struct CodechalVp9BrcUpdateSurfaceParams
{
    CODECHAL_MEDIA_STATE_TYPE           MbEncMediaStateType;
    PMOS_RESOURCE                       presBrcHistoryBuffer;
    PMOS_RESOURCE                       presBrcConstantDataBuffer; 
    PMOS_SURFACE                        psBrcDistortionBuffer;
    PMOS_RESOURCE                       presBrcMsdkPakBuffer;
    PMOS_RESOURCE                       presMbCodeBuffer;
    PMOS_RESOURCE                       presMbEncCurbeWriteBuffer;
    PMOS_RESOURCE                       presPicStateReadBuffer;
    PMOS_RESOURCE                       presPicStateWriteBuffer;
    PMOS_RESOURCE                       presSegmentStateReadBuffer;
    PMOS_RESOURCE                       presSegmentStateWriteBuffer;
    PMOS_RESOURCE                       presBrcBitstreamSizeData;
    PMOS_RESOURCE                       presBrcHucData;
    uint16_t                            wPictureCodingType;
    CODECHAL_MEDIA_STATE_TYPE           EncFunctionType;
    uint32_t                            dwDownscaledWidthInMb4x;
    uint32_t                            dwDownscaledFrameFieldHeightInMb4x;
    uint32_t                            dwBrcHistoryBufferSize;
    void*                               pvBindingTable;
    PMHW_KERNEL_STATE                   pKernelState;
    PMHW_KERNEL_STATE                   pKernelStateMbEnc;
};

struct CodechalVp9InitBrcConstantBufferParams
{
    PMOS_INTERFACE                          pOsInterface;
    MOS_RESOURCE                            resBrcConstantDataBuffer;
    uint16_t                                wPictureCodingType;
};

struct CODECHAL_ENCODE_VP9_BRC_BITSTREAM_SIZE_BUFFER
{
    uint32_t dwHcpBitstreamByteCountFrame; 
    uint32_t dwHcpImageStatusControl;  
    uint32_t Reserved[2];
};

//!
//! \class   CodechalEncodeVp9
//! \brief   VP9 dual-pipe encoder base class
//! \details This class defines the base class for VP9 dual-pipe encoder, it includes
//!          common member fields, functions, interfaces etc shared by all GENs.
//!          Gen specific definitions, features should be put into their corresponding classes.
//!          To create a VP9 dual-pipe encoder instance, client needs to call CodechalEncodeVp9::CreateVp9State()
//!
class CodechalEncodeVp9 : public CodechalEncoderState
{
public:

    struct CompressedHeader
    {
        union {
            struct {
                uint8_t valid         : 1;  // valid =1, invalid = 0
                uint8_t bin_probdiff  : 1;  // 1= bin, 0 = prob diff
                uint8_t prob          : 1;  // 0 = 128, 1 = 252
                uint8_t bin           : 1;
  	        uint8_t b_valid          : 1;
                uint8_t b_probdiff_select: 1;
                uint8_t b_prob_select    : 1;
                uint8_t b_bin            : 1;
            } fields;
            uint8_t value;
        };
    };

    struct BrcBuffers
    {
       MOS_RESOURCE            resBrcHistoryBuffer;
       MOS_RESOURCE            resBrcConstantDataBuffer;
       MOS_RESOURCE            resBrcMsdkPakBuffer;
       MOS_RESOURCE            resBrcMbEncCurbeWriteBuffer;
       MOS_RESOURCE            resMbEncAdvancedDsh;
       MOS_RESOURCE            resPicStateBrcReadBuffer;
       MOS_RESOURCE            resPicStateBrcWriteHucReadBuffer;
       MOS_RESOURCE            resSegmentStateBrcReadBuffer;
       MOS_RESOURCE            resSegmentStateBrcWriteBuffer;
       MOS_RESOURCE            resBrcBitstreamSizeBuffer;
       MOS_RESOURCE            resBrcHucDataBuffer;
    };

    struct PrevFrameInfo
    {
       uint32_t               IntraOnly; 
       uint32_t               FrameWidth;
       uint32_t               FrameHeight;
       uint32_t               KeyFrame;
       uint32_t               ShowFrame;
    };
    
    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeVp9();

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
    //! \brief      Put data for compressed header
    //!
    //! \param      [in] compressedHdr
    //!             Compressed header
    //! \param      [in] bit
    //!             Bit
    //! \param      [in] prob
    //!             Prob
    //! \param      [in] binIdx
    //!             Bin index
    //!
    void PutDataForCompressedHdr(
        CompressedHeader* compressedHdr,
        uint32_t bit,
        uint32_t prob,
        uint32_t binIdx);

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

    //! \brief    Encode User Feature Key Report.
    //! \details  Report user feature values set by encode.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS UserFeatureKeyReport();

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
    MOS_STATUS ReadHcpStatus(PMOS_COMMAND_BUFFER cmdBuffer);

    virtual MOS_STATUS ConstructPicStateBatchBuf(
        PMOS_RESOURCE picStateBuffer);

    //!
    //! \brief    Construct Pak insert object batch buf
    //! \details
    //!
    //! \param    [in] pakInsertObjBuffer
    //!           Pointer to MOS resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ConstructPakInsertObjBatchBuf(
        PMOS_RESOURCE pakInsertObjBuffer);

    //!
    //! \brief     Refresh frame internal buffers
    //! \details
    //!
    //! \return    MOS_STATUS
    //!            MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RefreshFrameInternalBuffers();

    //!
    //! \brief    Init context buffer
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //!
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ContextBufferInit(
            uint8_t *ctxBuffer,
            bool setToKey);
    //!
    //! \brief    Populate prob values which are different between Key and Non-Key frame
    //! \details
    //! \param    [in,out] ctxBuffer
    //!           Pointer to context buffer
    //!
    //! \param    [in] setToKey
    //!           Specify if it's key frame
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CtxBufDiffInit(
            uint8_t *ctxBuffer,
            bool setToKey);

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

protected:

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeVp9(
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
    MOS_STATUS AllocateBrcResources ();

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
    //! \brief    Set Curbe for BRC Init or Reset
    //!
    //! \param    [in] params
    //!           Pointer to CodechalVp9BrcInitResetSurfaceParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetBrcCurbe(struct CodechalVp9BrcCurbeParams* params) = 0;

    //!
    //! \brief    Send Surface for BRC Init or Reset
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CodechalVp9BrcInitResetSurfaceParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendBrcInitResetSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp9BrcInitResetSurfaceParams* params) = 0;

    //!
    //! \brief    Send Surface for BRC Intra Dist
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CodechalVp9BrcIntraDistSurfaceParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendBrcIntraDistSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp9BrcIntraDistSurfaceParams* params) = 0;

    //!
    //! \brief    Send Surface for BRC Update
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CodechalVp9BrcUpdateSurfaceParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendBrcUpdateSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp9BrcUpdateSurfaceParams* params) = 0;

    //!
    //! \brief    Set Curbe for Mb Enc
    //!
    //! \param    [in] params
    //!           Pointer to CodechalVp9MbencCurbeParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMbEncCurbe(struct CodechalVp9MbencCurbeParams* params) = 0;

    //!
    //! \brief    Set Curbe for ME
    //!
    //! \param    [in] params
    //!           Pointer to CodechalVp9MeCurbeParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMeCurbe(struct CodechalVp9MeCurbeParams* params) = 0;

    //!
    //! \brief    BRC Constant Buffer Initialize
    //!
    //! \param    [in] params
    //!           Pointer to CodechalVp9InitBrcConstantBufferParams params
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitBrcConstantBuffer(struct CodechalVp9InitBrcConstantBufferParams* params) = 0;

    //!
    //! \brief    Send Surface for ME
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CodechalVp9MeSurfaceParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMeSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp9MeSurfaceParams* params) = 0;


    //!
    //! \brief    Send Surface for Mb Enc
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to CodechalVp9MbencSurfaceParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMbEncSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        struct CodechalVp9MbencSurfaceParams*  params,
	CODECHAL_MEDIA_STATE_TYPE EncFunctionType) = 0;

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
    //! \brief    Invoke BRC BrcIntraDistKernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS BrcIntraDistKernel();

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
    //! \param    [in] EncFunctionType
    //!           Encoder Function Type
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS MbEncKernel(CODECHAL_MEDIA_STATE_TYPE EncFunctionType);

    //!
    //! \brief    Invoke ME kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS MeKernel();

    // Parameters passed by application
    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS   m_vp9SeqParams     = nullptr;
    PCODEC_VP9_ENCODE_PIC_PARAMS        m_vp9PicParams     = nullptr;
    PCODEC_VP9_ENCODE_SEGMENT_PARAMS    m_vp9SegmentParams = nullptr;

    uint8_t*                            m_kernelBinary = nullptr; //!< Pointer to the kernel binary
    uint32_t                            m_combinedKernelSize = 0; //!< Combined kernel binary size

    CODEC_PIC_ID    m_picIdx[CODEC_VP9_NUM_REF_FRAMES];
    PCODEC_REF_LIST m_refList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9];
    
    PCODECHAL_NAL_UNIT_PARAMS                 *m_nalUnitParams = nullptr;
    uint32_t                                  m_numNalUnit = 0;

    uint32_t                                  m_dwMaxPicWidth = 0;
    uint32_t                                  m_dwMaxPicHeight = 0;
    uint32_t                                  m_dwPicWidthInSB = 0;
    uint32_t                                  m_dwPicHeightInSB = 0;
    uint32_t                                  m_dwPicSizeInSB = 0;

    uint8_t                                   m_ucTxMode = 0;
    bool                                      m_hmeEnabled;
    bool                                      m_b16xMeEnabled;
    bool                                      m_b16xMeDone;
    bool                                      m_brcEnabled;
    bool                                      m_encryptedDshInUse;
    bool                                      m_brcReset;
    bool                                      m_mbBrcEnabled;;
    bool                                      m_multiRefQPCheckEnabled;
    bool                                      m_bWaitForENC;
    bool                                      m_adaptiveRepakSupported;

    uint32_t                                  m_frameRate;
    uint8_t                                   m_refFrameFlags;
    uint8_t                                   m_numRefFrames;
    PMOS_SURFACE                              m_lastRefPic;
    PMOS_SURFACE                              m_goldenRefPic;
    PMOS_SURFACE                              m_altRefPic;
    bool                                      m_mbEncIFrameDistEnabled;
    bool                                      m_brcDistortionBufferSupported;
    uint32_t                                  m_currentModeDecisionIndex;

    union
    {
        struct
        {
            MOS_RESOURCE                                resDeblockingFilterLineBuffer;
            MOS_RESOURCE                                resDeblockingFilterTileLineBuffer;
            MOS_RESOURCE                                resDeblockingFilterTileColumnBuffer;
            MOS_RESOURCE                                resMetadataLineBuffer;
            MOS_RESOURCE                                resMetadataTileLineBuffer;
            MOS_RESOURCE                                resMetadataTileColumnBuffer;
            MOS_RESOURCE                                resMvTemporalBuffer[2];
            MOS_RESOURCE                                resProbBuffer[CODEC_VP9_NUM_CONTEXTS];
            MOS_RESOURCE                                resSegmentIdBuffer;
            MOS_RESOURCE                                resHvcLineRowstoreBuffer; // Handle of HVC Line Row Store surface
            MOS_RESOURCE                                resHvcTileRowstoreBuffer; // Handle of HVC Tile Row Store surface
            MOS_RESOURCE                                resProbabilityDeltaBuffer;
            MOS_RESOURCE                                resTileRecordStrmOutBuffer;
            MOS_RESOURCE                                resCuStatsStrmOutBuffer;
            MOS_RESOURCE                                resCompressedHeaderBuffer;
            MOS_RESOURCE                                resProbabilityCounterBuffer;
            MOS_RESOURCE                                resModeDecision[2];
            bool                                        bClearAllToKey[CODEC_VP9_NUM_CONTEXTS];
            bool                                        bPreCtx0InterProbSaved;
            uint8_t                                     ucPreCtx0InterProbSaved[CODECHAL_VP9_INTER_PROB_SIZE];
            PrevFrameInfo                               m_prevFrameInfo;

            uint8_t                                     ucContextFrameTypes[CODEC_VP9_NUM_CONTEXTS];
            uint32_t                                    dwCurrMvTemporalBufferIndex;

            MOS_RESOURCE                                resHucProbDmemBuffer[2];
            MOS_RESOURCE                                resHucDefaultProbBuffer;
            MOS_RESOURCE                                resHucProbOutputBuffer;
            MOS_RESOURCE                                resHucPakInsertUncompressedHeaderReadBuffer;
            MOS_RESOURCE                                resHucPakInsertUncompressedHeaderWriteBuffer;
            MOS_RESOURCE                                resHucPakMmioBuffer;
            MOS_RESOURCE                                resHucDebugOutputBuffer;
            MOS_RESOURCE                                resHucSuperFrameBuffer;
        }Hw;
    }PakMode;

    // ME
    MHW_KERNEL_STATE                            m_meKernelState;
    struct CodechalBindingTableVp9Me            m_meBindingTable;
    MOS_SURFACE                                 m_s4XMemvDataBuffer;
    MOS_SURFACE                                 m_s16XMemvDataBuffer;
    MOS_SURFACE                                 m_s4XMeDistortionBuffer;

    // BRC Params
    MHW_KERNEL_STATE                            m_brcKernelStates[CODECHAL_ENCODE_VP9_BRC_IDX_NUM];
    struct CodechalBindingTableVp9BrcIntraDist  m_brcIntraDistBindingTable;
    struct CodechalBindingTableVp9BrcInitReset  m_brcInitResetBindingTable;
    struct CodechalBindingTableVp9BrcUpdate     m_brcUpdateBindingTable;
    double                                      m_dBrcInitCurrentTargetBufFullInBits;
    double                                      m_dBrcInitResetInputBitsPerFrame;
    uint32_t                                    m_brcInitResetBufSizeInBits;
    BrcBuffers                                  m_brcBuffers;
    bool                                        m_brcConstantBufferSupported;

    // MB Enc
    MHW_KERNEL_STATE                            m_mbEncKernelStates[CODECHAL_ENCODE_VP9_MBENC_IDX_NUM];
    uint32_t                                    m_numMbEncEncKrnStates;
    uint32_t                                    m_mbEncIFrameDshSize;
    struct CodechalBindingTableVp9MbencI32x32   m_mbEncI32x32BindingTable;
    struct CodechalBindingTableVp9MbencI16x16   m_mbEncI16x16BindingTable;
    struct CodechalBindingTableVp9MbencP        m_mbEncPBindingTable;
    struct CodechalBindingTableVp9MbencTx       m_mbEncTxBindingTable;
    uint32_t                                    m_mbEncDshSize;
    MOS_SURFACE                                 m_mbSegmentMapSurface;
    MOS_SURFACE                                 m_output16x16InterModes;
    bool                                        m_mbEncCurbeSetInBrcUpdate;
};

#endif  // __CODECHAL_ENCODER_VP9_H__
