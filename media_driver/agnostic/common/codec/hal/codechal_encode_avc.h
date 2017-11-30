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

typedef struct _CODECHAL_ENCODE_AVC_BRC_INIT_RESET_SURFACE_PARAMS
{
    PMOS_RESOURCE                       presBrcHistoryBuffer;
    PMOS_SURFACE                        psMeBrcDistortionBuffer;
    uint32_t                            dwMeBrcDistortionBottomFieldOffset;
    uint32_t                            dwDownscaledWidthInMb4x;
    uint32_t                            dwDownscaledFrameFieldHeightInMb4x;
} CODECHAL_ENCODE_AVC_BRC_INIT_RESET_SURFACE_PARAMS, *PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_SURFACE_PARAMS;

typedef struct _CODECHAL_ENCODE_AVC_IPCM_THRESHOLD
{
    uint8_t   QP;
    uint16_t  Threshold;
} CODECHAL_ENCODE_AVC_IPCM_THRESHOLD;

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
    CODECHAL_ENCODE_SEI_DATA            SeiData;                                            //!< Encode SEI data parameter.
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

    //for compatibility
    PCODECHAL_ENCODE_AVC_STATE          pOldAvcState;                                                   //!< For back compatibility, remove in future.
    PCODECHAL_ENCODE_AVC_GENERIC_STATE  m_avcGenericState = nullptr;                                    //!< For back compatibility, remove in future.

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
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeAvcEnc();

    //Encode interface implementations
    //!
    //! \brief    Initialize standard related members.
    //! \details  Initialize members, set proper value
    //!           to involved variables, allocate resources.
    //!
    //! \param    [in] pSettings
    //!           Encode settings.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS Initialize(
        PCODECHAL_SETTINGS pSettings) override;

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
    //! \brief    Resize buffers due to resoluton change.
    //! \details  Resize buffers due to resoluton change.
    //!
    //! \return   void
    //!
    virtual void ResizeBuffer();

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
    //! \param    [in] pbData
    //!           Encode interface
    //! \param    [in] pAvcSliceParams
    //!           Slice map command params
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS EncodeGenerateSliceMap(
        uint8_t* pbData,
        PCODEC_AVC_ENCODE_SLICE_PARAMS pAvcSliceParams);

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
    MOS_STATUS AllocateResourcesBrc();

    //!
    //! \brief    Allocate necessary resources for MBBRC case.
    //!           
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS AllocateResourcesMbBrc();

    //!
    //! \brief    Release allocated resources for BRC case.
    //!           
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS ReleaseResourcesBrc();

    //!
    //! \brief    Release allocated resources for MBBRC case.
    //!           
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS ReleaseResourcesMbBrc();

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
    //! \param    [in] pParams
    //!           Pointer to the CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS GenericEncodePictureLevel(
        PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS   pParams);

    //!
    //! \brief    Run Encode ME kernel
    //!
    //! \param    [in] pBrcBuffers
    //!           Pointer to the EncodeBrcBuffers
    //! \param    [in] hmeLevel
    //!           Hme level
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS GenericEncodeMeKernel(EncodeBrcBuffers* pBrcBuffers, HmeLevel hmeLevel);

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
    //! \param    [in] pParams
    //!           Pointer to the MeCurbeParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SetCurbeMe (
        MeCurbeParams* pParams) override;

    //!
    //! \brief    Set Encode ME kernel Surfaces
    //!
    //! \param    [in] pCmdBuffer
    //!           Pointer to the PMOS_COMMAND_BUFFER
    //! \param    [in] pParams
    //!           Pointer to the CODECHAL_ME_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SendMeSurfaces (
        PMOS_COMMAND_BUFFER pCmdBuffer,
        MeSurfaceParams* pParams) override;

    // AvcState functions.
    //!
    //! \brief    Initialize data members of AVC encoder instance
    //!          
    //! \return   void
    //!
    virtual void InitializeDataMember();

    //!
    //! \brief    Cast this to PCODECHAL_ENCODE_AVC_STATE.
    //!
    //! \return   PCODECHAL_ENCODE_AVC_STATE
    //!           Pointer to CODECHAL_ENCODE_AVC_STATE to meet legacy function requirement.
    //!
    PCODECHAL_ENCODE_AVC_STATE CodechalEncodeAvcStateCast();

    //!
    //! \brief    Cast this to PCODECHAL_ENCODE_AVC_GENERIC_STATE.
    //!
    //! \return   PCODECHAL_ENCODE_AVC_GENERIC_STATE
    //!           Pointer to PCODECHAL_ENCODE_AVC_GENERIC_STATE to meet legacy function requirement.
    //!
    PCODECHAL_ENCODE_AVC_GENERIC_STATE CodechalEncodeAvcGenericStateCast();

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
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ValidateNumReferences(
        PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS               pParams);

    //!
    //! \brief    Initialize brc constant buffer
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitBrcConstantBuffer(
        PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS        pParams);

    //!
    //! \brief    Initialize mbbrc constant buffer
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitMbBrcConstantDataBuffer(
        PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS pParams);

    //!
    //! \brief    Get inter rounding value.
    //!
    //! \param    [in] pSliceState
    //!           Pointer to MHW_VDBOX_AVC_SLICE_STATE
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetInterRounding(
        PMHW_VDBOX_AVC_SLICE_STATE pSliceState);

    //!
    //! \brief    Calculate lambda table.
    //!
    //! \param    [in] slice_type
    //!           Slice type.
    //! \param    [out] pLambda
    //!           Lambda table.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CalcLambdaTable(
        uint16_t slice_type,
        uint32_t* pLambda);

    //!
    //! \brief    Get Trellis Quantization mode/value enable or not.
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS.
    //! \param    [out] pTrellisQuantParams
    //!           Pointer to CODECHAL_ENCODE_AVC_TQ_PARAMS, mode & value setup.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS    pParams,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS          pTrellisQuantParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get Skip Bias Adjustment.
    //!
    //! \param    [in] ucSliceQP
    //!           Slice QP.
    //! \param    [in] GopRefDist
    //!           GOP reference dist.
    //! \param    [in] pbSkipBiasAdjustmentEnable
    //!           Adjustable or not.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetSkipBiasAdjustment(
        uint8_t ucSliceQP,
        uint16_t GopRefDist,
        bool* pbSkipBiasAdjustmentEnable);

    //!
    //! \brief    Get Hme Supported Based On TU.
    //!
    //! \param    [in] hmeLevel
    //!           HME level
    //! \param    [out] pbSupported
    //!           Supported or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHmeSupportedBasedOnTU(
        HmeLevel hmeLevel,
        bool *pbSupported);

    //!
    //! \brief    Get MbBrc status.
    //!
    //! \param    [in] dwTargetUsage
    //!           Target Usage.
    //! \param    [out] pbMbBrcEnabled
    //!           MBBRC enabled or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMbBrcEnabled(
        uint32_t                    dwTargetUsage,
        bool                       *pbMbBrcEnabled);

    //!
    //! \brief    CAF enabled or not.
    //!
    //! \param    [out] pbCAFEnable
    //!           CAF enabled or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetCAFEnabled(
        bool *pbCAFEnable);

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
    //! \brief    Insert RefPic Select List
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InsertInRefPicSelectList();

    //!
    //! \brief    Run MbEnc Kernel.
    //!
    //! \param    [in] bMbEncIFrameDistInUse
    //!           MbEncIFrameDist in use or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS MbEncKernel(
        bool bMbEncIFrameDistInUse);
 
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
    //! \param    [in] bUseRefPicList1
    //!           Use RefPicList 1 or Not.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS WPKernel(
                    bool bUseRefPicList1,
                    uint32_t Index);

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
    //! \param    [in] pParams
    //!           Pointer to the PCODECHAL_ENCODE_ID_OFFSET_PARAMS
    //! \param    [in] pdwKernelOffset
    //!           kernel offset 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMbEncKernelStateIdx(
        PCODECHAL_ENCODE_ID_OFFSET_PARAMS   pParams,
        uint32_t*                              pdwKernelOffset)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get MbEnc kernel curbe data
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMbEnc(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set MFE MbEnc kernel curbe data
    //!
    //! \param    [in] bMbEncIFrameDistInUse
    //!           MbEncIFrameDist in use or not
    //!
    //! \return   BOOL
    //!           true if MFE MbEnc is enabled, otherwise false
    //!
    virtual bool IsMfeMbEncEnabled(
        bool bMbEncIFrameDistInUse)
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
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMfeMbEnc(
        PCODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS pParams)
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
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_WP_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcWP(
        PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get FEI PreProc kernel curbe data
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcPreProc(
        PCODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get BRC InitReset kernel curbe data
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcBrcInitReset(
        PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get FrameBRCUpdate kernel curbe data
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcFrameBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get MbBrcUpdate kernel curbe data
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMbBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get Brc Block Copy kernel curbe data
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcBrcBlockCopy(
        PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get SFD kernel curbe data
    //!
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcSFD(
        PCODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS pParams);


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
    //! \param    [in] pCmdBuffer
    //!           Cmd Buffer
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_INIT_RESET_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBrcInitResetSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_SURFACE_PARAMS pParams);

    //!
    //! \brief    Set MbEnc kernel Surface data
    //!
    //! \param    [in] pCmdBuffer
    //!           Cmd Buffer
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcMbEncSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set Weighted Prediction kernel Surface state
    //!
    //! \param    [in] pCmdBuffer
    //!           Cmd Buffer
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcWPSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set FEI PreProc kernel Surface state
    //!
    //! \param    [in] pCmdBuffer
    //!           Cmd Buffer
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcPreProcSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PCODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set BrcFrameUpdate kernel Surface state
    //!
    //! \param    [in] pCmdBuffer
    //!           Cmd Buffer
    //! \param    [in] pParams
    //!           pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcBrcFrameUpdateSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set BrcMbUpdate kernel Surface state
    //!
    //! \param    [in] pCmdBuffer
    //!           Cmd Buffer
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcBrcMbUpdateSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS pParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set SFD kernel Surface state
    //!
    //! \param    [in] pCmdBuffer
    //!           Cmd Buffer
    //! \param    [in] pParams
    //!           Pointer to CODECHAL_ENCODE_AVC_SFD_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcSFDSurfaces(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PCODECHAL_ENCODE_AVC_SFD_SURFACE_PARAMS pParams);

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
    //!         [in] frameTracking
    //!         Indicate if frame tracking requested
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        bool                        frameTracking) override;

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
    //!           [in] transform8x8Flag
    //!           Indicate if transform8*8 makes effect
    //!           [in] SkipVal
    //!           input Skip value
    //! \return   uint16_t
    //!           return the updated Skip value
    //!
    uint16_t CalcSkipVal(
        bool    encBlockBasedSkipEn,
        bool    transform8x8Flag,
        uint16_t  SkipVal);

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
    //! \brief    Indicates if reference picture is a FIELD picture, if check ppRefList, the flags is always the second field flag;
    //! \param    [in] params
    //!           AVC mbenc cubre params
    //!           [list] list
    //!           forword or backword reference
    //!           [in] index
    //!           reference frame index
    //! \return   uint32_t
    //!           uiRefPicFieldFlag
    //!
    uint32_t GetRefPicFieldFlag(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS     params,
        uint32_t                                    list,
        uint32_t                                    index);

     //!
     //! \brief    Get QP value
     //! \param    [in] params 
     //!           AVC mbenc cubre params
     //!           [list] list
     //!           forword or backword reference
     //!           [in] index
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
     //!           [in] cmdBuffer
     //!           comand buffer
     //!           [in] mbEncKernelState 
     //!           MB encoder kernel state 
     //!           [in] kernelState 
     //!           Kernel State
     //!           [in] presAdvancedDsh
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

    //!
    //! \brief    Encoder pre enc look up buffer index
    //! \param    [in] encoder
    //!           codechal encoder pointer
    //!           [in] ucFrameIdx
    //!           ucFrame Index
    //!           [in] fristFrameinPreenc
    //!           Indicate if it's the first frame in Preenc
    //!           [in] inCache
    //!           Indicate if it's in cache
    //! \return   uint8_t
    //!           uiEmptyEntry
    //!
    uint8_t PreencLookUpBufIndex(
        CodechalEncoderState*       encoder,
        uint8_t                     ucFrameIdx,
        bool                        fristFrameinPreenc,
        bool                        *inCache);

#if USE_CODECHAL_DEBUG_TOOL
protected:
    virtual MOS_STATUS DumpParFile();

    virtual MOS_STATUS PopulateHmeParam(
        bool is16xMeEnabled,
        bool is32xMeEnabled,
        void *cmd);
#endif
};
#endif  // __CODECHAL_ENCODE_AVC_H__
