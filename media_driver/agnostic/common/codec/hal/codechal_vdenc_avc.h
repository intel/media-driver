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
//! \file     codechal_vdenc_avc.h
//! \brief    This file defines the base C++ class/interface for AVC VDENC encoding
//!           to be used across CODECHAL components.
//!

#ifndef __CODECHAL_VDENC_AVC_H__
#define __CODECHAL_VDENC_AVC_H__

#include "codechal_encode_avc_base.h"
#define CODECHAL_VDENC_AVC_MMIO_MFX_LRA_0_VMC240    0xF5F0EF00
#define CODECHAL_VDENC_AVC_MMIO_MFX_LRA_1_VMC240    0xFFFBFAF6
#define CODECHAL_VDENC_AVC_MMIO_MFX_LRA_2_VMC240    0x000002D3
#define CODECHAL_ENCODE_AVC_BRC_MIN_QP                      1
#define CODECHAL_VDENC_AVC_MB_SLICE_TRHESHOLD               12
#define CODECHAL_VDENC_AVC_BRC_HUC_STATUS_REENCODE_MASK     (1<<31)

#define CODECHAL_VDENC_AVC_BRC_MIN_QP                       10

#define CODECHAL_VDENC_AVC_CQP_NUM_OF_PASSES                1    // No standalone PAK IPCM pass for VDENC

#define CODECHAL_VDENC_AVC_BRC_HISTORY_BUF_SIZE             0x1000
#define CODECHAL_VDENC_AVC_BRC_DEBUG_BUF_SIZE               0x1000
#define CODECHAL_VDENC_AVC_BRC_MB_BUDGET_SIZE               104

#define CODECHAL_VDENC_AVC_BRC_TOPQPDELTATHRFORADAPT2PASS   2
#define CODECHAL_VDENC_AVC_BRC_BOTQPDELTATHRFORADAPT2PASS   1
#define CODECHAL_VDENC_AVC_BRC_TOPFRMSZTHRFORADAPT2PASS     32
#define CODECHAL_VDENC_AVC_BRC_BOTFRMSZTHRFORADAPT2PASS     24

#define CODECHAL_VDENC_AVC_AVBR_TOPQPDELTATHRFORADAPT2PASS  2
#define CODECHAL_VDENC_AVC_AVBR_BOTQPDELTATHRFORADAPT2PASS  2
#define CODECHAL_VDENC_AVC_AVBR_TOPFRMSZTHRFORADAPT2PASS    48
#define CODECHAL_VDENC_AVC_AVBR_BOTFRMSZTHRFORADAPT2PASS    32

#define CODECHAL_VDENC_AVC_BRC_TOPQPDELTATHRFORADAPT2PASS_4K 5
#define CODECHAL_VDENC_AVC_BRC_BOTQPDELTATHRFORADAPT2PASS_4K 5
#define CODECHAL_VDENC_AVC_BRC_TOPFRMSZTHRFORADAPT2PASS_4K   80
#define CODECHAL_VDENC_AVC_BRC_BOTFRMSZTHRFORADAPT2PASS_4K   80

#define CODECHAL_VDENC_AVC_STATIC_FRAME_ZMV_PERCENT          80
#define CODECHAL_VDENC_AVC_STATIC_FRAME_INTRACOSTSCLRatioP   240

//dynamic deviation thresholds calculation
#define CODECHAL_VDENC_AVC_N_DEV_THRESHLDS                  8
#define CODECHAL_VDENC_AVC_DEV_STD_FPS                      30.
#define CODECHAL_VDENC_AVC_BPS_RATIO_LOW                    0.1
#define CODECHAL_VDENC_AVC_BPS_RATIO_HIGH                   3.5
#define CODECHAL_VDENC_AVC_POS_MULT_I                       50
#define CODECHAL_VDENC_AVC_NEG_MULT_I                       -50
#define CODECHAL_VDENC_AVC_POS_MULT_PB                      50
#define CODECHAL_VDENC_AVC_NEG_MULT_PB                      -50
#define CODECHAL_VDENC_AVC_POS_MULT_VBR                     100
#define CODECHAL_VDENC_AVC_NEG_MULT_VBR                     -50

#define __CODEGEN_BITFIELD(l, h) (h) - (l) + 1
//!
//! \brief CODECHAL_VDENC_STREAMIN_STATE
//! \details
//!
//!
struct CODECHAL_VDENC_STREAMIN_STATE
{
    union
    {
        //!< DWORD 0
        struct
        {
            uint32_t RegionOfInterestRoiSelection : __CODEGEN_BITFIELD(0, 7);  //!< Region of Interest (ROI) Selection
            uint32_t Forceintra : __CODEGEN_BITFIELD(8, 8);                    //!< FORCEINTRA
            uint32_t Forceskip : __CODEGEN_BITFIELD(9, 9);                     //!< FORCESKIP
            uint32_t Reserved10 : __CODEGEN_BITFIELD(10, 31);                  //!< Reserved
        };
        uint32_t Value;
    } DW0;
    union
    {
        //!< DWORD 1
        struct
        {
            uint32_t Qpprimey : __CODEGEN_BITFIELD(0, 7);           //!< QPPRIMEY
            uint32_t Targetsizeinword : __CODEGEN_BITFIELD(8, 15);  //!< TargetSizeInWord
            uint32_t Maxsizeinword : __CODEGEN_BITFIELD(16, 23);    //!< MaxSizeInWord
            uint32_t Reserved56 : __CODEGEN_BITFIELD(24, 31);       //!< Reserved
        };
        uint32_t Value;
    } DW1;
    union
    {
        //!< DWORD 2
        struct
        {
            uint32_t FwdPredictorX : __CODEGEN_BITFIELD(0, 15);   //!< Fwd Predictor.X
            uint32_t FwdPredictorY : __CODEGEN_BITFIELD(16, 31);  //!< Fwd Predictor.Y
        };
        uint32_t Value;
    } DW2;
    union
    {
        //!< DWORD 3
        struct
        {
            uint32_t BwdPredictorX : __CODEGEN_BITFIELD(0, 15);   //!< Bwd Predictor.X
            uint32_t BwdPredictorY : __CODEGEN_BITFIELD(16, 31);  //!< Bwd Predictor.Y
        };
        uint32_t Value;
    } DW3;
    union
    {
        //!< DWORD 4
        struct
        {
            uint32_t FwdRefid0 : __CODEGEN_BITFIELD(0, 3);     //!< Fwd RefID0
            uint32_t BwdRefid0 : __CODEGEN_BITFIELD(4, 7);     //!< Bwd RefID0
            uint32_t Reserved136 : __CODEGEN_BITFIELD(8, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW4;

    uint32_t Reserved160[11];  //!< Reserved

    //! \name Local enumerations

    //! \brief FORCEINTRA
    //! \details
    //!     This field specifies whether current macroblock should be coded as an
    //!     intra macroblock.
    //!                    It is illegal to enable both ForceSkip and ForceIntra for
    //!     the same macroblock.
    //!                    This should be disabled if Rolling-I is enabled in the
    //!     VDEnc Image State.
    enum FORCEINTRA
    {
        FORCEINTRA_DISABLE = 0,  //!< VDEnc determined macroblock type
        FORCEINTRA_ENABLE  = 1,  //!< Force to be coded as an intra macroblock
    };

    //! \brief FORCESKIP
    //! \details
    //!     This field specifies whether current macroblock should be coded as a
    //!     skipped macroblock.
    //!                    It is illegal to enable both ForceSkip and ForceIntra for
    //!     the same macroblock.
    //!                    This should be disabled if Rolling-I is enabled in the
    //!     VDEnc Image State.
    //!                      It is illegal to enable ForceSkip for I-Frames.
    enum FORCESKIP
    {
        FORCESKIP_DISABLE = 0,  //!< VDEnc determined macroblock type
        FORCESKIP_ENABLE  = 1,  //!< Force to be coded as a skipped macroblock
    };

    //! \brief QPPRIMEY
    //! \details
    //!     Quantization parameter for Y.
    enum QPPRIMEY
    {
        QPPRIMEY_UNNAMED0  = 0,   //!< No additional details
        QPPRIMEY_UNNAMED51 = 51,  //!< No additional details
    };

    CODECHAL_VDENC_STREAMIN_STATE()
    {
        DW0.Value      = 0;
        DW0.Forceintra = 0;
        DW0.Forceskip  = 0;
        DW1.Value      = 0;
        DW1.Qpprimey   = 0;
        DW2.Value      = 0;
        DW3.Value      = 0;
        DW4.Value      = 0;
        MOS_ZeroMemory(&Reserved160, sizeof(Reserved160));
    }

    static const size_t dwSize   = 16;
    static const size_t byteSize = 64;
};

typedef struct _AVCVdencBRCCostantData
{
    uint8_t     UPD_GlobalRateQPAdjTabI_U8[64];
    uint8_t     UPD_GlobalRateQPAdjTabP_U8[64];
    uint8_t     UPD_GlobalRateQPAdjTabB_U8[64];
    uint8_t     UPD_DistThreshldI_U8[10];
    uint8_t     UPD_DistThreshldP_U8[10];
    uint8_t     UPD_DistThreshldB_U8[10];
    uint8_t     UPD_DistQPAdjTabI_U8[81];
    uint8_t     UPD_DistQPAdjTabP_U8[81];
    uint8_t     UPD_DistQPAdjTabB_U8[81];
    int8_t      UPD_BufRateAdjTabI_S8[72];
    int8_t      UPD_BufRateAdjTabP_S8[72];
    int8_t      UPD_BufRateAdjTabB_S8[72];
    uint8_t     UPD_FrmSzMinTabP_U8[9];
    uint8_t     UPD_FrmSzMinTabB_U8[9];
    uint8_t     UPD_FrmSzMinTabI_U8[9];
    uint8_t     UPD_FrmSzMaxTabP_U8[9];
    uint8_t     UPD_FrmSzMaxTabB_U8[9];
    uint8_t     UPD_FrmSzMaxTabI_U8[9];
    uint8_t     UPD_FrmSzSCGTabP_U8[9];
    uint8_t     UPD_FrmSzSCGTabB_U8[9];
    uint8_t     UPD_FrmSzSCGTabI_U8[9];
    // Cost Table 14*42 = 588 bytes
    uint8_t     UPD_I_IntraNonPred[42];
    uint8_t     UPD_I_Intra16x16[42];
    uint8_t     UPD_I_Intra8x8[42];
    uint8_t     UPD_I_Intra4x4[42];
    uint8_t     UPD_I_IntraChroma[42];
    uint8_t     UPD_P_IntraNonPred[42];
    uint8_t     UPD_P_Intra16x16[42];
    uint8_t     UPD_P_Intra8x8[42];
    uint8_t     UPD_P_Intra4x4[42];
    uint8_t     UPD_P_IntraChroma[42];
    uint8_t     UPD_P_Inter16x8[42];
    uint8_t     UPD_P_Inter8x8[42];
    uint8_t     UPD_P_Inter16x16[42];
    uint8_t     UPD_P_RefId[42];
    uint8_t     UPD_HMEMVCost[8][42];
    uint8_t     RSVD[42];
} AVCVdencBRCCostantData, *PAVCVdencBRCCostantData;

//!
//! \class    CodechalVdencAvcState
//! \brief    Codechal Vdenc Avc state
//!
class CodechalVdencAvcState : public CodechalEncodeAvcBase
{
public:
    const bool m_perfModeEnabled[NUM_VDENC_TARGET_USAGE_MODES] =
        {
            0, 0, 0, 0, 0, 0, 1, 1};

    // VDEnc BRC Flag in BRC Init Kernel
    typedef enum _BRCFLAG
    {
        BRCFLAG_ISICQ                        = 0x0000,
        BRCFLAG_ISCBR                        = 0x0010,
        BRCFLAG_ISVBR                        = 0x0020,
        BRCFLAG_ISVCM                        = 0x0040,
        BRCFLAG_ISLOWDELAY                    = 0x0080
    } BRCFLAG;

    typedef enum _LutMode
    {
        LutMode_INTRA_NONPRED        =    0x00,  // extra penalty for non-predicted modes
        LutMode_INTRA                =    0x01,
        LutMode_INTRA_16x16         =    0x01,
        LutMode_INTRA_8x8            =    0x02,
        LutMode_INTRA_4x4            =    0x03,
        LutMode_INTER_BWD            =    0x09,
        LutMode_REF_ID                =    0x0A,
        LutMode_INTRA_CHROMA        =    0x0B,
        LutMode_INTER                =    0x08,
        LutMode_INTER_16x16         =    0x08,
        LutMode_INTER_16x8            =    0x04,
        LutMode_INTER_8x16            =    0x04,
        LutMode_INTER_8x8q            =    0x05,
        LutMode_INTER_8x4q            =    0x06,
        LutMode_INTER_4x8q            =    0x06,
        LutMode_INTER_4x4q            =    0x07,
        LutMode_INTER_16x8_FIELD    =    0x06,
        LutMode_INTER_8x8_FIELD     =    0x07
    } LutMode;

    typedef struct _TLBAllocationParams
    {
        PMOS_RESOURCE                        presTlbMmioBuffer;
        uint32_t                            dwMmioMfxLra0Override;
        uint32_t                            dwMmioMfxLra1Override;
        uint32_t                            dwMmioMfxLra2Override;
    } TLBAllocationParams, *PTLBAllocationParams;

    //!
    //! \brief    Constructor
    //!
    CodechalVdencAvcState(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalVdencAvcState(const CodechalVdencAvcState&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalVdencAvcState& operator=(const CodechalVdencAvcState&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalVdencAvcState();

    virtual MOS_STATUS Initialize( CodechalSetting * settings);

    virtual MOS_STATUS InitializePicture(const EncoderParams& params);

    virtual MOS_STATUS ExecuteKernelFunctions();

    virtual MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        bool                        frameTracking,
        MHW_MI_MMIOREGISTERS       *mmioRegister = nullptr);

    virtual MOS_STATUS ExecutePictureLevel();

    virtual MOS_STATUS ExecuteSliceLevel();

    virtual MOS_STATUS UserFeatureKeyReport();

    //!
    //! \brief    Add VDENC_WALKER_STATE commands to command buffer
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencWalkerStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer);

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
    virtual MOS_STATUS ValidateNumReferences( PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS params);

    //!
    //! \brief    Get inter rounding value.
    //!
    //! \param    [in] sliceState
    //!           Pointer to MHW_VDBOX_AVC_SLICE_STATE
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetInterRounding( PMHW_VDBOX_AVC_SLICE_STATE sliceState);

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
        uint8_t  sliceQP,
        uint16_t gopRefDist,
        bool*    skipBiasAdjustmentEnable);

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
    //! \brief    Init SFD(still frame detection) kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateSFD();

    //!
    //! \brief    Get SFD kernel curbe data
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeSFD( PCODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS params);

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
    virtual MOS_STATUS SendSFDSurfaces(
        PMOS_COMMAND_BUFFER                     cmdBuffer,
        PCODECHAL_ENCODE_AVC_SFD_SURFACE_PARAMS params);

    //!
    //! \brief    Run SFD(still frame detection) kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SFDKernel();

    //!
    //! \brief    Set VDENC Dirty ROI StreamIn Surface state
    //!
    //! \param    [in] vdencStreamIn
    //!           StreamIn Surface Resource.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupDirtyROI( PMOS_RESOURCE vdencStreamIn);

    //!
    //! \brief    Set VDENC HuC Brc InitReset state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmemHuCBrcInitReset() = 0;

    //!
    //! \brief    Set VDENC HuC Brc Update state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmemHuCBrcUpdate() = 0;

    //!
    //! \brief    VDENC Load Mv Cost based on QP
    //!
    //! \param    [in] QP
    //!           QP value
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS LoadMvCost( uint8_t QP) = 0;

    //!
    //! \brief    VDENC Load HME Mv Cost based on QP
    //!
    //! \param    [in] QP
    //!           QP value
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS LoadHmeMvCost( uint8_t QP) = 0;

    //!
    //! \brief    VDENC Load HME Mv Cost based on QP
    //!
    //! \param    [in] seqParams
    //!           QP value
    //! \param    [in] HMEMVCostTable
    //!           HME MV Cost Table
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS LoadHmeMvCostTable(
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams,
        uint8_t                           HMEMVCostTable[8][42]) = 0;

    //!
    //! \brief    Set VDENC ROI StreamIn Surface state
    //!
    //! \param    [in] picParams
    //!           Pointer to CODEC_AVC_ENCODE_PIC_PARAMS.
    //! \param    [in] vdencStreamIn
    //!           StreamIn Surface Resource.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupROIStreamIn(
        PCODEC_AVC_ENCODE_PIC_PARAMS picParams,
        PMOS_RESOURCE                vdencStreamIn);

    //!
    //! \brief    Sort and set distinct delta QPs
    //!
    //! \return   bool
    //!           true if native ROI, otherwise false
    //!
    bool ProcessRoiDeltaQp();

    //!
    //! \brief    VDENC BRC InitReset HuC FW Cmd.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS HuCBrcInitReset();

    //!
    //! \brief    VDENC BRC Update HuC FW Cmd.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS HuCBrcUpdate();

    //!
    //! \brief    VDENC Loads Cost According To CodingType & QP.
    //!
    //! \param    [in] pictureCodingType
    //!           Picture encoding type.
    //! \param    [in] QP
    //!           QP value
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS LoadCosts(
        uint16_t  pictureCodingType,
        uint8_t   QP);

    //!
    //! \brief    VDENC using dummy stream object for HuC BRC FW.
    //!
    //! \param    [in] cmdBuffer
    //!           Command Buffer.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS HuCBrcDummyStreamObject( PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    VDENC Set const date to HuC for BRC Update FW..
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetConstDataHuCBrcUpdate();

    //!
    //! \brief    VDENC Compute BRC Init QP..
    //! \param    [in] seqParams
    //!           AVC VDENC encoder sequence params
    //! \param    [in] initQP
    //!           Pointer to Init QP
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ComputeBRCInitQP(
        PCODEC_AVC_ENCODE_SEQUENCE_PARAMS     seqParams,
        int32_t*                              initQP);

    //!
    //! \brief    VDENC Store HuC Status to Register..
    //! \param    [in] hwInterface
    //!           HW Interface
    //! \param    [in] cmdBuffer
    //!           cmd buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AvcVdencStoreHuCStatus2Register(
        CodechalHwInterface                    *hwInterface,
        PMOS_COMMAND_BUFFER                     cmdBuffer);

    //!
    //! \brief    VDENC Set TLB allocation..
    //! \param    [in] cmdBuffer
    //!           cmd buffer
    //! \param    [in] params
    //!           AVC VDENC TLB allocation params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetTLBAllocation(
        PMOS_COMMAND_BUFFER    cmdBuffer,
        PTLBAllocationParams   params);

    //!
    //! \brief    VDENC Restore TLB allocation..
    //! \param    [in] cmdBuffer
    //!           cmd buffer
    //! \param    [in] tlbMmioBuffer
    //!           PMOS resource params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RestoreTLBAllocation(
        PMOS_COMMAND_BUFFER                         cmdBuffer,
        PMOS_RESOURCE                               tlbMmioBuffer);

    //!
    //! \brief    Set VDENC HuC Brc InitReset state
    //! 
    //! \param    [in] hucVdencBrcInitDmem
    //!           Point to BrcInitDmem of different Gen-x Platforms.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    template <class CODECHAL_VDENC_AVC_BRC_INIT_DMEM>
    MOS_STATUS SetDmemHuCBrcInitResetImpl( CODECHAL_VDENC_AVC_BRC_INIT_DMEM* hucVdencBrcInitDmem);

    //!
    //! \brief    Set VDENC HuC Brc Update state
    //! 
    //! \param    [in] hucVDEncBrcDmem
    //!           Point to BRC DMEM of different Gen-x Platforms.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    template <class CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM>
    MOS_STATUS SetDmemHuCBrcUpdateImpl( CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM* hucVDEncBrcDmem);

    //!
    //! \brief    Functions to set parameter and execute ME kernel
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteMeKernel();

protected:
    // AvcGeneraicState functions
    //!
    //! \brief    AVC VDEnc State Initialization.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS Initialize();

    //!
    //! \brief    Allocate VDENC necessary resources.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief    Set Sequence Structures.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetSequenceStructs();

    //!
    //! \brief    Set Picture Structures
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetPictureStructs();

    //!
    //! \brief    Set slice Structs
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetSliceStructs();

    //!
    //! \brief    Initialize Encode ME kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitKernelStateMe();

    //!
    //! \brief    Set Encode ME kernel Curbe data.
    //!
    //! \param    [in] params
    //!           Pointer to the MeCurbeParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SetCurbeMe( MeCurbeParams* params);

    //!
    //! \brief    Set Encode ME kernel Surfaces
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to the MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to the CODECHAL_ME_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SendMeSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        MeSurfaceParams*    params);

    //!
    //! \brief    Set MFX_PIPE_BUF_ADDR_STATE parameter
    //!
    //! \param    [in] genericParam
    //!           Input parameters
    //! \param    [out] param
    //!           reference to MHW_VDBOX_PIPE_BUF_ADDR_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SetMfxPipeBufAddrStateParams(
        CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS genericParam,
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& param);

    //!
    //! \brief    Set MHW_VDBOX_VDENC_CQPT_STATE parameter
    //!
    //! \param    [out] param
    //!           reference to MHW_VDBOX_VDENC_CQPT_STATE_PARAMS
    //!
    //! \return   void
    //!
    virtual void SetVdencCqptStateParams(MHW_VDBOX_VDENC_CQPT_STATE_PARAMS& param);

    //!
    //! \brief    Set MFX_AVC_IMG_STATE parameter
    //!
    //! \param    [out] param
    //!           reference to MHW_VDBOX_AVC_IMG_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual void SetMfxAvcImgStateParams(MHW_VDBOX_AVC_IMG_PARAMS& param);

    //!
    //! \brief    Calculate Vdenc Picture State CommandSize 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS CalculateVdencPictureStateCommandSize();

    //!
    //! \brief    Create MHW_VDBOX_STATE_CMDSIZE_PARAMS
    //!
    //! \return   PMHW_VDBOX_STATE_CMDSIZE_PARAMS
    //!
    virtual PMHW_VDBOX_STATE_CMDSIZE_PARAMS CreateMhwVdboxStateCmdsizeParams();

    //!
    //! \brief    Create PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS.
    //!
    //! \return   PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS
    //!
    virtual PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS CreateMhwVdboxPipeModeSelectParams();

    //!
    //! \brief    Create PMHW_VDBOX_AVC_IMG_PARAMS.
    //!
    //! \return   PMHW_VDBOX_AVC_IMG_PARAMS
    //!
    virtual PMHW_VDBOX_AVC_IMG_PARAMS CreateMhwVdboxAvcImgParams();

    //!
    //! \brief    Create PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS.
    //!
    //! \return   PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS
    //!
    virtual PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS CreateMhwVdboxVdencWalkerStateParams();

    virtual uint32_t GetBRCCostantDataSize() { return sizeof(AVCVdencBRCCostantData); }

    virtual MOS_STATUS FillHucConstData(uint8_t *data);

protected:
    bool                                        m_vdencSinglePassEnable = false;   //!< Enable VDEnc single pass

    MOS_RESOURCE                                m_vdencIntraRowStoreScratchBuffer; //!< Handle of intra row store surface
    MOS_RESOURCE                                m_pakStatsBuffer;                  //!< Handle of PAK status buffer
    MOS_RESOURCE                                m_vdencStatsBuffer;                //!< Handle of VDEnc status buffer
    MOS_RESOURCE                                m_vdencColocatedMVBuffer;           //!< Handle of colocated MV buffer
    MOS_RESOURCE                                m_vdencTlbMmioBuffer;              //!< VDEnc TLB MMIO buffer

    uint32_t                                    m_mmioMfxLra0Override = 0;         //!< Override Register MFX_LRA_0
    uint32_t                                    m_mmioMfxLra1Override = 0;         //!< Override Register MFX_LRA_1
    uint32_t                                    m_mmioMfxLra2Override = 0;         //!< Override Register MFX_LRA_2

    uint8_t                                     *m_vdencModeCostTbl = nullptr;     //!< Pointer to VDEnc Mode Cost Table
    uint8_t                                     *m_vdencMvCostTbl = nullptr;       //!< Pointer to VDEnc MV Cost Table
    uint8_t                                     *m_vdencHmeMvCostTbl = nullptr;    //!< Pointer to VDEnc HME MV Cost Table

    // SEI
    CodechalEncodeSeiData m_seiData;         //!< Encode SEI data parameter.
    uint32_t              m_seiDataOffset;   //!< Encode SEI data offset.
    uint8_t *             m_seiParamBuffer;  //!< Encode SEI data buffer.

    bool     m_brcInit;                       //!< BRC init enable flag.
    bool     m_brcReset;                      //!< BRC reset enable flag.
    bool     m_mbBrcEnabled;                  //!< MBBrc enable flag.
    bool     m_mbBrcUserFeatureKeyControl;    //!< MBBRC user feature control enable flag.
    double   m_dBrcTargetSize;                //!< BRC target size.
    uint32_t m_trellis;                       //!< Trellis Number.
    bool     m_acceleratorHeaderPackingCaps;  //!< Flag set by driver from driver caps.

    double   m_dBrcInitCurrentTargetBufFullInBits;  //!< BRC init current target buffer full in bits
    double   m_dBrcInitResetInputBitsPerFrame;      //!< BrcInitReset Input Bits Per Frame
    uint32_t m_brcInitResetBufSizeInBits;           //!< BrcInitReset Buffer Size In Bits
    uint32_t m_brcInitPreviousTargetBufFullInBits;  //!< BRC Init Previous Target Buffer Full In Bits

    // Below values will be set if qp control params are sent by app
    bool    m_minMaxQpControlEnabled;  //!< Flag to indicate if min/max QP feature is enabled or not.
    uint8_t m_iMinQp;                  //!< I frame Minimum QP.
    uint8_t m_iMaxQp;                  //!< I frame Maximum QP.
    uint8_t m_pMinQp;                  //!< P frame Minimum QP.
    uint8_t m_pMaxQp;                  //!< P frame Maximum QP.
    bool    m_pFrameMinMaxQpControl;   //!< Indicates min/max QP values for P-frames are set separately or not.

    uint32_t     m_skipFrameBufferSize;  //!< size of skip frame packed data.
    MOS_RESOURCE m_resSkipFrameBuffer;   //!< copy skip frame packed data from DDI.

    // VDENC BRC Buffers
    MOS_RESOURCE m_resVdencBrcUpdateDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][CODECHAL_VDENC_BRC_NUM_OF_PASSES];  //!< Brc Update DMEM Buffer Array.
    MOS_RESOURCE m_resVdencBrcInitDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];                                      //!< Brc Init DMEM Buffer Array.
    MOS_RESOURCE m_resVdencBrcImageStatesReadBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];                               //!< Read-only VDENC+PAK IMG STATE buffer.
    MOS_RESOURCE m_resVdencBrcConstDataBuffer;                                                                          //!< BRC Const Data Buffer.
    MOS_RESOURCE m_resVdencBrcHistoryBuffer;                                                                            //!< BRC History Buffer.
    MOS_RESOURCE m_resVdencBrcDbgBuffer;                                                                                //!< BRC Debug Buffer.

    // Static frame detection
    bool              m_staticFrameDetectionEnable;                               //!< Static frame detection enable.
    MOS_RESOURCE      m_resSfdOutputBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];  //!< Array of SFDOutputBuffer.
    MOS_RESOURCE      m_resSfdCostTablePFrameBuffer;                              //!< SFD CostTable of P Frame.
    MOS_RESOURCE      m_resSfdCostTableBFrameBuffer;                              //!< SFD CostTable of B Frame.
    MOS_RESOURCE      m_resVdencSfdImageStateReadBuffer;                          //!< SFD ImageState Read Buffer.
    PMHW_KERNEL_STATE m_sfdKernelState;                                           //!< Point to SFD kernel state.

    // Generation Specific Support Flags & User Feature Key Reads
    uint8_t m_mbBrcSupportCaps;             //!< MbBrcSupport Capability.
    bool    m_ftqEnable;                    //!< FTQEnable
    bool    m_skipBiasAdjustmentSupported;  //!< SkipBiasAdjustment support for P frame
    bool    m_sliceLevelReportSupported;    //!< Slice Level Report support
    bool    m_brcRoiSupported;              //!< BRC Roi Support Flag.
    bool    m_brcMotionAdaptiveEnable;      //!< BRC motion adaptive optimization enabled. 

    bool     m_roundingInterEnable;          //!< RoundingInter Enable Flag.
    bool     m_adaptiveRoundingInterEnable;  //!< Adaptive Rounding Inter Enable Flag.
    uint32_t m_roundingInterP;               //!< Rounding Inter for P frame.

    uint8_t m_vdEncModeCost[12];  //!< VDEnc Mode Cost Table.
    uint8_t m_vdEncMvCost[8];     //!< VDEnc MV Cost Table.
    uint8_t m_vdEncHmeMvCost[8];  //!< VDEnc HME MV Cost Table.

    uint32_t                            m_slidingWindowSize;                                            //!< Slideing Window Size.
    bool                                m_forceToSkipEnable;                                            //!< Force to Skip Flag.
    uint32_t                            m_vdencBrcInitDmemBufferSize;                                   //!< Brc Init-Dmem Buffer Size.
    uint32_t                            m_vdencBrcUpdateDmemBufferSize;                                 //!< Brc Update-Dmem Buffer Size.
    uint32_t                            m_vdencColocatedMVBufferSize;                                     //!< Colocated MV Read / Write Buffer Size.
    bool                                m_vdencStaticFrame;                                             //!< Static Frame Indicator.
    uint32_t                            m_vdencStaticRegionPct;                                         //!< Ratio of Static Region in One Frame.
    bool                                m_oneOnOneMapping = false;                                      //!< Indicate if one on one ref index mapping is enabled

    static const uint32_t TrellisQuantizationRounding[NUM_VDENC_TARGET_USAGE_MODES];
    static const bool TrellisQuantizationEnable[NUM_TARGET_USAGE_MODES];

    static const uint32_t m_vdboxHucVdencBrcInitKernelDescriptor = 4;                                     //!< Huc Vdenc Brc init kernel descriptor
    static const uint32_t m_vdboxHucVdencBrcUpdateKernelDescriptor = 5;                                   //!< Huc Vdenc Brc update kernel descriptor

    static constexpr uint8_t m_maxNumRoi       = 16;  //!< VDEnc maximum number of ROI supported
    static constexpr uint8_t m_maxNumNativeRoi = 3;   //!< Number of native ROI supported by VDEnc HW

protected:

    static const uint32_t AVC_I_SLICE_SIZE_MINUS = 500;                                    //!< VDENC I SLICE threshold
    static const uint32_t AVC_P_SLICE_SIZE_MINUS = 500;                                    //!< VDENC P SLICE threshold
    static const uint32_t SFD_OUTPUT_BUFFER_SIZE = 128;                                    //!< SFD_OUTPUT_BUFFER_SIZE
    static const uint32_t AVC_BRC_STATS_BUF_SIZE = 80;                                     //!< VDENC BRC statistics buffer size
    static const uint32_t AVC_BRC_PAK_STATS_BUF_SIZE = 204;                                //!< VDENC BRC PAK statistics buffer size
    static const double   BRC_DevThreshI0_FP_NEG[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2];  //!< Negative BRC threshold for I frame
    static const double   BRC_DevThreshI0_FP_POS[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2];  //!< Positive BRC threshold for I frame
    static const double   BRC_DevThreshPB0_FP_NEG[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2]; //!< Negative BRC threshold for P/B frame
    static const double   BRC_DevThreshPB0_FP_POS[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2]; //!< Positive BRC threshold for P/B frame
    static const double   BRC_DevThreshVBR0_NEG[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2];   //!< Negative BRC threshold for VBR mode
    static const double   BRC_DevThreshVBR0_POS[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2];   //!< Positive BRC threshold for VBR mode
    static const int8_t   BRC_LowDelay_DevThreshPB0_S8[8];                                 //!< Low Delay BRC threshold for P/B frame
    static const int8_t   BRC_LowDelay_DevThreshI0_S8[8];                                  //!< Low Delay BRC threshold for I frame
    static const int8_t   BRC_LowDelay_DevThreshVBR0_S8[8];                                //!< Low Delay BRC threshold for VBR Mode
    static const int8_t   BRC_INIT_DistQPDelta_I8[4];                                      //!< Distortion QP Delta
    static const uint8_t  BRC_EstRateThreshP0_U8[7];                                       //!< Estimate Rate Thresh of P frame
    static const uint8_t  BRC_EstRateThreshI0_U8[7];                                       //!< Estimate Rate Thresh of I frame
    static const uint16_t BRC_UPD_start_global_adjust_frame[4];                            //!< Start Global Adjust Frame
    static const uint8_t  BRC_UPD_global_rate_ratio_threshold[7];                          //!< Global Rate Ratio Threshold
    static const uint8_t  BRC_UPD_slwin_global_rate_ratio_threshold[7];                    //!< Slide Window Global Rate Ratio Threshold
    static const uint8_t  BRC_UPD_start_global_adjust_mult[5];                             //!< Start Global Adjust Multiply
    static const uint8_t  BRC_UPD_start_global_adjust_div[5];                              //!< Start Global Adjust Division
    static const uint16_t BRC_UPD_SLCSZ_UPD_THRDELTAP_100Percent_U16[42];                  //!< Slice Size Threshold Delta for P frame.
    static const uint16_t BRC_UPD_SLCSZ_UPD_THRDELTAI_100Percent_U16[42];                  //!< Slice Size Threshold Delta for I frame.
    static const int8_t   BRC_UPD_global_rate_ratio_threshold_qp[8];                       //!< Global Rate Ratio QP Threshold
    static const uint32_t AVC_Mode_Cost[2][12][CODEC_AVC_NUM_QP];                          //!< Mode Cost Table.
    static const int8_t   BRC_UPD_GlobalRateQPAdjTabI_U8[64];                              //!< I Picture Global Rate QP Adjustment Table.
    static const int8_t   BRC_UPD_GlobalRateQPAdjTabP_U8[64];                              //!< P Picture Global Rate QP Adjustment Table.
    static const int8_t   BRC_UPD_SlWinGlobalRateQPAdjTabP_U8[64];                         //!< P picture Global Rate QP Adjustment Table for Sliding Window BRC
    static const int8_t   BRC_UPD_GlobalRateQPAdjTabB_U8[64];                              //!< B Picture Global Rate QP Adjustment Table.
    static const uint8_t  BRC_UPD_DistThreshldI_U8[10];                                    //!< I Picture Distortion THreshold.
    static const uint8_t  BRC_UPD_DistThreshldP_U8[10];                                    //!< P Picture Distortion THreshold.
    static const int8_t   CBR_UPD_DistQPAdjTabI_U8[81];                                    //!< I Picture Distortion QP Adjustment Table under CBR Mode.
    static const int8_t   CBR_UPD_DistQPAdjTabP_U8[81];                                    //!< P Picture Distortion QP Adjustment Table under CBR Mode.
    static const int8_t   CBR_UPD_DistQPAdjTabB_U8[81];                                    //!< B Picture Distortion QP Adjustment Table under CBR Mode.
    static const int8_t   VBR_UPD_DistQPAdjTabI_U8[81];                                    //!< I Picture Distortion QP Adjustment Table under VBR Mode.
    static const int8_t   VBR_UPD_DistQPAdjTabP_U8[81];                                    //!< P Picture Distortion QP Adjustment Table under VBR Mode.
    static const int8_t   VBR_UPD_DistQPAdjTabB_U8[81];                                    //!< B Picture Distortion QP Adjustment Table under VBR Mode.
    static const int8_t   CBR_UPD_FrmSzAdjTabI_S8[72];                                     //!< I Picture Frame Size Adjustment Table under CBR Mode.
    static const int8_t   CBR_UPD_FrmSzAdjTabP_S8[72];                                     //!< P Picture Frame Size Adjustment Table under CBR Mode.
    static const int8_t   CBR_UPD_FrmSzAdjTabB_S8[72];                                     //!< B Picture Frame Size Adjustment Table under CBR Mode.
    static const int8_t   VBR_UPD_FrmSzAdjTabI_S8[72];                                     //!< I Picture Frame Size Adjustment Table under VBR Mode.
    static const int8_t   VBR_UPD_FrmSzAdjTabP_S8[72];                                     //!< P Picture Frame Size Adjustment Table under VBR Mode.
    static const int8_t   VBR_UPD_FrmSzAdjTabB_S8[72];                                     //!< B Picture Frame Size Adjustment Table under VBR Mode.
    static const int8_t   QVBR_UPD_FrmSzAdjTabP_S8[72];                                    //!< P Picture Frame Size Adjustment Table under QVBR Mode.
    static const int8_t   LOW_DELAY_UPD_FrmSzAdjTabI_S8[72];                               //!< I Picture Frame Size Adjustment Table under Low Delay Mode.
    static const int8_t   LOW_DELAY_UPD_FrmSzAdjTabP_S8[72];                               //!< P Picture Frame Size Adjustment Table under Low Delay Mode.
    static const int8_t   LOW_DELAY_UPD_FrmSzAdjTabB_S8[72];                               //!< B Picture Frame Size Adjustment Table under Low Delay Mode.
    static const uint8_t  BRC_UPD_FrmSzMinTabP_U8[9];                                      //!< I Picture Minimum Frame Size Table.
    static const uint8_t  BRC_UPD_FrmSzMinTabI_U8[9];                                      //!< P Picture Minimum Frame Size Table.
    static const uint8_t  BRC_UPD_FrmSzMaxTabP_U8[9];                                      //!< I Picture Maximum Frame Size Table.
    static const uint8_t  BRC_UPD_FrmSzMaxTabI_U8[9];                                      //!< P Picture Maximum Frame Size Table.
    static const uint8_t  BRC_UPD_FrmSzSCGTabP_U8[9];                                      //!<
    static const uint8_t  BRC_UPD_FrmSzSCGTabI_U8[9];                                      //!<

                                                                                            ///< BRC Const Data.
    static const uint8_t  BRC_UPD_I_IntraNonPred[42];                                      //!< Cost Table for Intra Non-Prediction
    static const uint8_t  BRC_UPD_I_Intra8x8[42];                                          //!< Cost Table for Intra 8x8
    static const uint8_t  BRC_UPD_I_Intra4x4[42];                                          //!< Cost Table for Intra 4x4
    static const uint8_t  BRC_UPD_I_IntraChroma[42];                                       //!< Cost Table for Intra Chrome
    static const uint8_t  BRC_UPD_P_IntraNonPred[42];                                      //!< Cost Table for Intra Non-Prediction
    static const uint8_t  BRC_UPD_P_Intra16x16[42];                                        //!< Cost Table for Intra 16x16
    static const uint8_t  BRC_UPD_P_Intra8x8[42];                                          //!< Cost Table for Intra 8x8
    static const uint8_t  BRC_UPD_P_Intra4x4[42];                                          //!< Cost Table for Intra 4x4
    static const uint8_t  BRC_UPD_P_Inter16x8[42];                                         //!< Cost Table for Inter 16x8
    static const uint8_t  BRC_UPD_P_Inter8x8[42];                                          //!< Cost Table for Inter 8x8
    static const uint8_t  BRC_UPD_P_Inter16x16[42];                                        //!< Cost Table for Inter 16x16
    static const uint8_t  BRC_UPD_P_RefId[42];                                             //!< Cost Table for Reference Index

    static const bool     SHMEEnabled[NUM_VDENC_TARGET_USAGE_MODES];                       //!< SHME Enabled Query Table.
    static const bool     UHMEEnabled[NUM_VDENC_TARGET_USAGE_MODES];                       //!< HME Enabled Query Table.
    static const uint8_t  MaxRefIdx0[NUM_VDENC_TARGET_USAGE_MODES];                        //!< Max Reference Index Query Table.
    static const uint8_t  AdaptiveInterRoundingPWithoutB[CODEC_AVC_NUM_QP];                //!< InterRounding Table.
    static const uint8_t  AdaptiveInterRoundingP[CODEC_AVC_NUM_QP];                        //!< InterRounding Table.
    static const uint32_t InterRoundingP[NUM_TARGET_USAGE_MODES];                          //!< P Picture InterRounding Table.
    static const uint32_t InterRoundingB[NUM_TARGET_USAGE_MODES];                          //!< B Picture InterRounding Table.
    static const uint32_t InterRoundingBRef[NUM_TARGET_USAGE_MODES];                       //!< B Ref Picture InterRounding Table.
    static const uint8_t  AdaptiveInterRoundingB[CODEC_AVC_NUM_QP];                        //!< B Picture Adaptive InterRounding Table.

#if USE_CODECHAL_DEBUG_TOOL
protected:
    virtual MOS_STATUS DumpHucBrcInit();

    virtual MOS_STATUS DumpHucBrcUpdate(bool isInput);

    virtual MOS_STATUS DumpEncodeImgStats(
        PMOS_COMMAND_BUFFER        cmdbuffer);

    virtual MOS_STATUS DumpSeqParFile();
    virtual MOS_STATUS DumpFrameParFile();

    virtual MOS_STATUS PopulateHmeParam(
        bool    is16xMeEnabled,
        bool    is32xMeEnabled,
        uint8_t meMethod,
        void    *cmd);

    virtual MOS_STATUS PopulateEncParam(
        uint8_t meMethod,
        void    *cmd) { return MOS_STATUS_SUCCESS; }
#endif
};

// template functions for all gen-x platforms.
template <class CODECHAL_VDENC_AVC_BRC_INIT_DMEM>
MOS_STATUS CodechalVdencAvcState::SetDmemHuCBrcInitResetImpl(CODECHAL_VDENC_AVC_BRC_INIT_DMEM* hucVDEncBrcInitDmem)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto avcSeqParams = m_avcSeqParam;
    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) // Low Delay Mode
    {
        avcSeqParams->MaxBitRate = avcSeqParams->TargetBitRate;
    }

    m_dBrcInitResetInputBitsPerFrame =
        ((double)avcSeqParams->MaxBitRate * 100) / avcSeqParams->FramesPer100Sec;
    m_dBrcInitCurrentTargetBufFullInBits = m_dBrcInitResetInputBitsPerFrame;
    m_dBrcTargetSize                     = avcSeqParams->InitVBVBufferFullnessInBit;

    hucVDEncBrcInitDmem->BRCFunc_U8 = m_brcInit ? 0 : 2;  // 0 for init, 2 for reset

    hucVDEncBrcInitDmem->INIT_FrameWidth_U16 = (uint16_t)m_frameWidth;
    hucVDEncBrcInitDmem->INIT_FrameHeight_U16 = (uint16_t)m_frameHeight;

    hucVDEncBrcInitDmem->INIT_TargetBitrate_U32 = avcSeqParams->TargetBitRate;
    hucVDEncBrcInitDmem->INIT_MinRate_U32 = avcSeqParams->MinBitRate;
    hucVDEncBrcInitDmem->INIT_MaxRate_U32 = avcSeqParams->MaxBitRate;
    hucVDEncBrcInitDmem->INIT_BufSize_U32 = avcSeqParams->VBVBufferSizeInBit;
    hucVDEncBrcInitDmem->INIT_InitBufFull_U32 = avcSeqParams->InitVBVBufferFullnessInBit;

    if (hucVDEncBrcInitDmem->INIT_InitBufFull_U32 > avcSeqParams->VBVBufferSizeInBit)
        hucVDEncBrcInitDmem->INIT_InitBufFull_U32 = avcSeqParams->VBVBufferSizeInBit;

    switch (avcSeqParams->RateControlMethod)
    {
    case RATECONTROL_CBR:
        hucVDEncBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISCBR;
        break;
    case RATECONTROL_VBR:
        hucVDEncBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISVBR;
        break;
    case RATECONTROL_QVBR:
        // QVBR will use VBR BRCFlag, triggered when ICQQualityFactor > 10
        hucVDEncBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISVBR;
        break;
        // Temp solution using AVBR for low delay case, before the BRC flag is added to DDI
    case RATECONTROL_AVBR:
        hucVDEncBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISLOWDELAY;
        break;
    case RATECONTROL_ICQ:
        hucVDEncBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISICQ;
        break;
    case RATECONTROL_VCM:
        hucVDEncBrcInitDmem->INIT_BRCFlag_U16 |= BRCFLAG_ISVCM;
        break;
    default:
        break;
    }

    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) // Low Delay Mode
    {
        hucVDEncBrcInitDmem->INIT_BRCFlag_U16 = BRCFLAG_ISLOWDELAY;
        hucVDEncBrcInitDmem->INIT_LowDelayGoldenFrameBoost_U8 = 0; //get from ?
    }

    hucVDEncBrcInitDmem->INIT_FrameRateM_U32 = avcSeqParams->FramesPer100Sec;
    hucVDEncBrcInitDmem->INIT_FrameRateD_U32 = 100;

    uint32_t profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        avcSeqParams, this, &profileLevelMaxFrame));

    hucVDEncBrcInitDmem->INIT_ProfileLevelMaxFrame_U32 = profileLevelMaxFrame;
    if (avcSeqParams->GopRefDist && (avcSeqParams->GopPicSize > 0))
    {
        hucVDEncBrcInitDmem->INIT_GopP_U16 = (avcSeqParams->GopPicSize - 1) / avcSeqParams->GopRefDist;
    }

    if (m_minMaxQpControlEnabled)
    {
        // Convert range [1,51] to [10,51] for VDEnc due to HW limitation
        hucVDEncBrcInitDmem->INIT_MinQP_U16 = MOS_MAX(m_iMinQp, 10);
        hucVDEncBrcInitDmem->INIT_MaxQP_U16 = MOS_MAX(m_iMaxQp, 10);
    }
    else
    {
        hucVDEncBrcInitDmem->INIT_MinQP_U16 = CODECHAL_VDENC_AVC_BRC_MIN_QP;     // Setting values from arch spec
        hucVDEncBrcInitDmem->INIT_MaxQP_U16 = CODECHAL_ENCODE_AVC_MAX_SLICE_QP;  // Setting values from arch spec
    }

                                                                             //dynamic deviation thresholds
    double inputBitsPerFrame = ((double)avcSeqParams->MaxBitRate * (double)100) / (double)avcSeqParams->FramesPer100Sec;
    double bps_ratio = inputBitsPerFrame / ((double)avcSeqParams->VBVBufferSizeInBit * 100 / avcSeqParams->FramesPer100Sec/*DEV_STD_FPS*/);
    if (bps_ratio < CODECHAL_VDENC_AVC_BPS_RATIO_LOW) bps_ratio = CODECHAL_VDENC_AVC_BPS_RATIO_LOW;
    if (bps_ratio > CODECHAL_VDENC_AVC_BPS_RATIO_HIGH) bps_ratio = CODECHAL_VDENC_AVC_BPS_RATIO_HIGH;

    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) // Low Delay Mode
    {
        MOS_SecureMemcpy(hucVDEncBrcInitDmem->INIT_DevThreshPB0_S8, 8 * sizeof(int8_t), (void*)BRC_LowDelay_DevThreshPB0_S8, 8 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVDEncBrcInitDmem->INIT_DevThreshI0_S8, 8 * sizeof(int8_t), (void*)BRC_LowDelay_DevThreshI0_S8, 8 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVDEncBrcInitDmem->INIT_DevThreshVBR0_S8, 8 * sizeof(int8_t), (void*)BRC_LowDelay_DevThreshVBR0_S8, 8 * sizeof(int8_t));
    }
    else
    {
        //dynamic deviation thresholds
        for (int i = 0; i < CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2; i++)
        {
            hucVDEncBrcInitDmem->INIT_DevThreshPB0_S8[i] =
                (int8_t)(CODECHAL_VDENC_AVC_NEG_MULT_PB * pow(BRC_DevThreshPB0_FP_NEG[i], bps_ratio));
            hucVDEncBrcInitDmem->INIT_DevThreshPB0_S8[i + CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2] =
                (int8_t)(CODECHAL_VDENC_AVC_POS_MULT_PB * pow(BRC_DevThreshPB0_FP_POS[i], bps_ratio));

            hucVDEncBrcInitDmem->INIT_DevThreshI0_S8[i] =
                (int8_t)(CODECHAL_VDENC_AVC_NEG_MULT_I * pow(BRC_DevThreshI0_FP_NEG[i], bps_ratio));
            hucVDEncBrcInitDmem->INIT_DevThreshI0_S8[i + CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2] =
                (int8_t)(CODECHAL_VDENC_AVC_POS_MULT_I * pow(BRC_DevThreshI0_FP_POS[i], bps_ratio));

            hucVDEncBrcInitDmem->INIT_DevThreshVBR0_S8[i] =
                (int8_t)(CODECHAL_VDENC_AVC_NEG_MULT_VBR * pow(BRC_DevThreshVBR0_NEG[i], bps_ratio));
            hucVDEncBrcInitDmem->INIT_DevThreshVBR0_S8[i + CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2] =
                (int8_t)(CODECHAL_VDENC_AVC_POS_MULT_VBR * pow(BRC_DevThreshVBR0_POS[i], bps_ratio));
        }
    }

    int32_t initQP;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ComputeBRCInitQP(avcSeqParams, &initQP));

    hucVDEncBrcInitDmem->INIT_InitQPIP = (uint8_t)initQP;

    // MBBRC control
    if (m_mbBrcEnabled)
    {
        hucVDEncBrcInitDmem->INIT_MbQpCtrl_U8 = 1;
        MOS_SecureMemcpy(hucVDEncBrcInitDmem->INIT_DistQPDelta_I8, 4 * sizeof(int8_t), (void*)BRC_INIT_DistQPDelta_I8, 4 * sizeof(int8_t));
    }

    hucVDEncBrcInitDmem->INIT_SliceSizeCtrlEn_U8 = avcSeqParams->EnableSliceLevelRateCtrl; // Enable slice size control

    hucVDEncBrcInitDmem->INIT_OscillationQpDelta_U8 =
        ((avcSeqParams->RateControlMethod == RATECONTROL_VCM) || (avcSeqParams->RateControlMethod == RATECONTROL_QVBR)) ? 16 : 0;
    hucVDEncBrcInitDmem->INIT_HRDConformanceCheckDisable_U8 =
        ((avcSeqParams->RateControlMethod == RATECONTROL_VCM) || (avcSeqParams->RateControlMethod == RATECONTROL_AVBR)) ? 1 : 0;

    // Adaptive 2nd re-encode pass
    if (m_picWidthInMb * m_picHeightInMb >= ((3840 * 2160) >> 8)) // >= 4K
    {
        hucVDEncBrcInitDmem->INIT_TopQPDeltaThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_BRC_TOPQPDELTATHRFORADAPT2PASS_4K;
        hucVDEncBrcInitDmem->INIT_BotQPDeltaThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_BRC_BOTQPDELTATHRFORADAPT2PASS_4K;
        hucVDEncBrcInitDmem->INIT_TopFrmSzThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_BRC_TOPFRMSZTHRFORADAPT2PASS_4K;
        hucVDEncBrcInitDmem->INIT_BotFrmSzThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_BRC_BOTFRMSZTHRFORADAPT2PASS_4K;
    }
    else
    {
        if (avcSeqParams->RateControlMethod == RATECONTROL_AVBR)
        {
            hucVDEncBrcInitDmem->INIT_TopQPDeltaThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_AVBR_TOPQPDELTATHRFORADAPT2PASS;
            hucVDEncBrcInitDmem->INIT_BotQPDeltaThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_AVBR_BOTQPDELTATHRFORADAPT2PASS;
            hucVDEncBrcInitDmem->INIT_TopFrmSzThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_AVBR_TOPFRMSZTHRFORADAPT2PASS;
            hucVDEncBrcInitDmem->INIT_BotFrmSzThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_AVBR_BOTFRMSZTHRFORADAPT2PASS;
        }
        else
        {
            hucVDEncBrcInitDmem->INIT_TopQPDeltaThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_BRC_TOPQPDELTATHRFORADAPT2PASS;
            hucVDEncBrcInitDmem->INIT_BotQPDeltaThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_BRC_BOTQPDELTATHRFORADAPT2PASS;
            hucVDEncBrcInitDmem->INIT_TopFrmSzThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_BRC_TOPFRMSZTHRFORADAPT2PASS;
            hucVDEncBrcInitDmem->INIT_BotFrmSzThrForAdapt2Pass_U8 = CODECHAL_VDENC_AVC_BRC_BOTFRMSZTHRFORADAPT2PASS;
        }
    }

    hucVDEncBrcInitDmem->INIT_QPSelectForFirstPass_U8 = 1;
    hucVDEncBrcInitDmem->INIT_MBHeaderCompensation_U8 = 1;
    hucVDEncBrcInitDmem->INIT_DeltaQP_Adaptation_U8 = 1;
    hucVDEncBrcInitDmem->INIT_MaxCRFQualityFactor_U8 = CODECHAL_ENCODE_AVC_MAX_ICQ_QUALITYFACTOR + 1;

    if (RATECONTROL_QVBR == avcSeqParams->RateControlMethod || RATECONTROL_ICQ == avcSeqParams->RateControlMethod)
    {
        hucVDEncBrcInitDmem->INIT_CRFQualityFactor_U8 = (uint8_t)avcSeqParams->ICQQualityFactor;
        hucVDEncBrcInitDmem->INIT_ScenarioInfo_U8 = (RATECONTROL_QVBR == avcSeqParams->RateControlMethod) ? 1 : 0;
    }

    if (m_avcPicParam->NumDirtyROI)
    {
        hucVDEncBrcInitDmem->INIT_ScenarioInfo_U8 = 1; //DISPLAYREMOTING
    }

    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW) // Sliding Window BRC
    {
        hucVDEncBrcInitDmem->INIT_SlidingWidowRCEnable_U8 = 1;
        hucVDEncBrcInitDmem->INIT_SlidingWindowSize_U8 = (uint8_t)(avcSeqParams->FramesPer100Sec / 100);
        hucVDEncBrcInitDmem->INIT_SlidingWindowMaxRateRatio_U8 = 120;
    }

    MOS_SecureMemcpy(hucVDEncBrcInitDmem->INIT_EstRateThreshP0_U8, 7 * sizeof(uint8_t), (void*)BRC_EstRateThreshP0_U8, 7 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVDEncBrcInitDmem->INIT_EstRateThreshI0_U8, 7 * sizeof(uint8_t), (void*)BRC_EstRateThreshI0_U8, 7 * sizeof(uint8_t));

    return eStatus;
}

template <class CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM>
MOS_STATUS CodechalVdencAvcState::SetDmemHuCBrcUpdateImpl(CODECHAL_VDENC_AVC_BRC_UPDATE_DMEM* hucVDEncBrcDmem)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto avcSeqParams = m_avcSeqParam;
    auto avcPicParams = m_avcPicParam;

    hucVDEncBrcDmem->BRCFunc_U8 = 1;   // Update:1

    if (!m_brcInit && (m_currPass == 0))
    {
        m_brcInitPreviousTargetBufFullInBits =
            (uint32_t)(m_dBrcInitCurrentTargetBufFullInBits + m_dBrcInitResetInputBitsPerFrame * m_numSkipFrames);
        m_dBrcInitCurrentTargetBufFullInBits += m_dBrcInitResetInputBitsPerFrame * (1 + m_numSkipFrames);
        m_dBrcTargetSize += m_dBrcInitResetInputBitsPerFrame * (1 + m_numSkipFrames);
    }

    if (m_dBrcTargetSize > avcSeqParams->VBVBufferSizeInBit)
    {
        m_dBrcTargetSize -= avcSeqParams->VBVBufferSizeInBit;
    }

    hucVDEncBrcDmem->UPD_FRAMENUM_U32           = m_avcSliceParams->frame_num;
    hucVDEncBrcDmem->UPD_TARGETSIZE_U32         = (uint32_t)(m_dBrcTargetSize);
    hucVDEncBrcDmem->UPD_PeakTxBitsPerFrame_U32 = (uint32_t)(m_dBrcInitCurrentTargetBufFullInBits - m_brcInitPreviousTargetBufFullInBits);

    //Dynamic slice size control
    if (avcSeqParams->EnableSliceLevelRateCtrl)
    {
        hucVDEncBrcDmem->UPD_SLCSZ_TARGETSLCSZ_U16 = (uint16_t)avcPicParams->SliceSizeInBytes; // target slice size
        hucVDEncBrcDmem->UPD_TargetSliceSize_U16 = (uint16_t)avcPicParams->SliceSizeInBytes; // set max slice size to be same as target slice size
        hucVDEncBrcDmem->UPD_MaxNumSliceAllowed_U16 = (uint16_t)m_maxNumSlicesAllowed;

        for (uint8_t k = 0; k < 42; k++)
        {
            hucVDEncBrcDmem->UPD_SLCSZ_UPD_THRDELTAI_U16[k] =
                MOS_MIN(avcPicParams->SliceSizeInBytes - 150, BRC_UPD_SLCSZ_UPD_THRDELTAI_100Percent_U16[k]);
            hucVDEncBrcDmem->UPD_SLCSZ_UPD_THRDELTAP_U16[k] =
                MOS_MIN(avcPicParams->SliceSizeInBytes - 150, BRC_UPD_SLCSZ_UPD_THRDELTAP_100Percent_U16[k]);
        }
    }
    else
    {
        hucVDEncBrcDmem->UPD_SLCSZ_TARGETSLCSZ_U16 = 0;
        hucVDEncBrcDmem->UPD_TargetSliceSize_U16 = 0;
        hucVDEncBrcDmem->UPD_MaxNumSliceAllowed_U16 = 0;

        for (uint8_t k = 0; k < 42; k++)
        {
            hucVDEncBrcDmem->UPD_SLCSZ_UPD_THRDELTAI_U16[k] = 0;
            hucVDEncBrcDmem->UPD_SLCSZ_UPD_THRDELTAP_U16[k] = 0;
        }
    }

    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW) // Sliding Window BRC
    {
        MOS_SecureMemcpy(hucVDEncBrcDmem->UPD_gRateRatioThreshold_U8, 7 * sizeof(uint8_t), (void*)BRC_UPD_slwin_global_rate_ratio_threshold, 7 * sizeof(uint8_t));
    }
    else
    {
        MOS_SecureMemcpy(hucVDEncBrcDmem->UPD_gRateRatioThreshold_U8, 7 * sizeof(uint8_t), (void*)BRC_UPD_global_rate_ratio_threshold, 7 * sizeof(uint8_t));
    }

    hucVDEncBrcDmem->UPD_CurrFrameType_U8 = (m_pictureCodingType + 1) % 3;   // I:1, P:2, B:0

    MOS_SecureMemcpy(hucVDEncBrcDmem->UPD_startGAdjFrame_U16, 4 * sizeof(uint16_t), (void*)BRC_UPD_start_global_adjust_frame, 4 * sizeof(uint16_t));

    MOS_SecureMemcpy(hucVDEncBrcDmem->UPD_startGAdjMult_U8, 5 * sizeof(uint8_t), (void*)BRC_UPD_start_global_adjust_mult, 5 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucVDEncBrcDmem->UPD_startGAdjDiv_U8, 5 * sizeof(uint8_t), (void*)BRC_UPD_start_global_adjust_div, 5 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucVDEncBrcDmem->UPD_gRateRatioThresholdQP_U8, 8 * sizeof(uint8_t), (void*)BRC_UPD_global_rate_ratio_threshold_qp, 8 * sizeof(uint8_t));

    hucVDEncBrcDmem->UPD_PAKPassNum_U8 = m_currPass;
    hucVDEncBrcDmem->UPD_MaxNumPass_U8 = m_numPasses + 1;

    uint32_t numP = 0;
    if (avcSeqParams->GopRefDist && (avcSeqParams->GopPicSize > 0))
    {
        numP = (avcSeqParams->GopPicSize - 1) / avcSeqParams->GopRefDist;
    }

    for (int32_t i = 0; i < 2; i++)
    {
        hucVDEncBrcDmem->UPD_SceneChgWidth_U8[i] = (uint8_t)MOS_MIN((numP + 1) / 5, 6);
    }

    hucVDEncBrcDmem->UPD_SceneChgDetectEn_U8 = 1;
    hucVDEncBrcDmem->UPD_SceneChgPrevIntraPctThreshold_U8 = 0x60;
    hucVDEncBrcDmem->UPD_SceneChgCurIntraPctThreshold_U8 = 0xc0;

    hucVDEncBrcDmem->UPD_IPAverageCoeff_U8 =
        (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) ? 0 : 0x80;

    hucVDEncBrcDmem->UPD_CQP_FracQp_U8 = 0;

    if (avcSeqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        hucVDEncBrcDmem->UPD_CQP_QpValue_U8 = 18;   //Cmodel suggested a few values to try: 18,20,26,30
    }
    else
    {
        hucVDEncBrcDmem->UPD_CQP_QpValue_U8 = 0;
    }

    if (m_staticFrameDetectionInUse)
    {
        hucVDEncBrcDmem->UPD_HMEDetectionEnable_U8 = 1;
    }
    else
    {
        hucVDEncBrcDmem->UPD_HMEDetectionEnable_U8 = 0;
    }

    // Skipped frame handling
    if (m_numSkipFrames)
    {
        // CP case: one or more frames with skip flag = 2 received and copied
        hucVDEncBrcDmem->UPD_SkipFrameSize_U16 = (uint16_t)m_sizeSkipFrames;
        hucVDEncBrcDmem->UPD_NumOfFramesSkipped_U16 = (uint16_t)m_numSkipFrames;
    }
    else if (FRAME_SKIP_NORMAL == m_skipFrameFlag)
    {
        // non-CP case: use the num/size of skipped frames passed in by MSDK
        hucVDEncBrcDmem->UPD_SkipFrameSize_U16 = (uint16_t)m_avcPicParam->SizeSkipFrames;
        hucVDEncBrcDmem->UPD_NumOfFramesSkipped_U16 = (uint16_t)m_avcPicParam->NumSkipFrames;
    }
    else
    {
        hucVDEncBrcDmem->UPD_SkipFrameSize_U16 = 0;
        hucVDEncBrcDmem->UPD_NumOfFramesSkipped_U16 = 0;
    }

    // HMECost enabled by default in CModel V11738+
    hucVDEncBrcDmem->UPD_HMECostEnable_U8 = 1;

    if (avcPicParams->NumDirtyROI)
    {
        hucVDEncBrcDmem->UPD_StaticRegionPct_U16 = (uint16_t)m_vdencStaticRegionPct;
        if (m_mbBrcEnabled)
        {
            hucVDEncBrcDmem->UPD_ROISource_U8 = 2;
        }
        else
        {
            hucVDEncBrcDmem->UPD_ROISource_U8 = 0;
        }
    }
    else
    {
        hucVDEncBrcDmem->UPD_StaticRegionPct_U16 = 0;
        hucVDEncBrcDmem->UPD_ROISource_U8 = 0;
    }

    hucVDEncBrcDmem->UPD_SLBB_Size_U16 = (uint16_t)m_hwInterface->m_vdencBrcImgStateBufferSize;

    // reset skip frame statistics
    m_numSkipFrames = 0;
    m_sizeSkipFrames = 0;

    return eStatus;
}

#endif  // __CODECHAL_VDENC_AVC_H__
