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
//! \file     codechal_encode_hevc_g10.h
//! \brief    HEVC dual-pipe encoder for GEN10 platform.
//!

#ifndef __CODECHAL_ENCODE_HEVC_G10_H__
#define __CODECHAL_ENCODE_HEVC_G10_H__

#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_encode_hevc.h"

struct DsConvCurbeDataG10
{
    DsConvCurbeDataG10()
    {
        DW0_InputBitDepthForChroma = 10;
        DW0_InputBitDepthForLuma = 10;
        DW0_OutputBitDepthForChroma = 8;
        DW0_OutputBitDepthForLuma = 8;
        DW0_RoundingEnable = 1;
        DW1_PictureFormat = 0;
        DW1_ConvertFlag = 0;
        DW1_DownscaleStage = dsDisabled;
        DW1_MbStatisticsDumpFlag = 0;
        DW1_Reserved_0 = 0;
        DW1_LcuSize = 0;
        DW1_JobQueueSize = 2656;
        DW2_OriginalPicWidthInSamples = 0;
        DW2_OriginalPicHeightInSamples = 0;
        DW3_BTI_InputConversionSurface = 0xffff;
        DW4_BTI_Value = 0xffff;
        DW5_BTI_4xDsSurface = 0xffff;
        DW6_BTI_MBStatsSurface = 0xffff;
        DW7_BTI_2xDsSurface = 0xffff;
        DW8_BTI_MB_Split_Surface = 0xffff;
        DW9_BTI_LCU32_JobQueueScratchBufferSurface = 0xffff;
        DW10_BTI_LCU64_CU32_JobQueueScratchBufferSurface = 0xffff;
        DW11_BTI_LCU64_CU32_64x64_DistortionSurface = 0xffff;
    }

    // DWORD 0
    uint32_t   DW0_InputBitDepthForChroma : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW0_InputBitDepthForLuma : MOS_BITFIELD_RANGE(8, 15);
    uint32_t   DW0_OutputBitDepthForChroma : MOS_BITFIELD_RANGE(16, 23);
    uint32_t   DW0_OutputBitDepthForLuma : MOS_BITFIELD_RANGE(24, 30);
    uint32_t   DW0_RoundingEnable : MOS_BITFIELD_BIT(31);

    // DWORD 1
    uint32_t   DW1_PictureFormat : MOS_BITFIELD_RANGE(0, 7);
    uint32_t   DW1_ConvertFlag : MOS_BITFIELD_BIT(8);
    uint32_t   DW1_DownscaleStage : MOS_BITFIELD_RANGE(9, 11);
    uint32_t   DW1_MbStatisticsDumpFlag : MOS_BITFIELD_BIT(12);
    uint32_t   DW1_Reserved_0 : MOS_BITFIELD_RANGE(13, 14);
    uint32_t   DW1_LcuSize : MOS_BITFIELD_BIT(15);
    uint32_t   DW1_JobQueueSize : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 2
    uint32_t   DW2_OriginalPicWidthInSamples : MOS_BITFIELD_RANGE(0, 15);
    uint32_t   DW2_OriginalPicHeightInSamples : MOS_BITFIELD_RANGE(16, 31);

    // DWORD 3
    uint32_t   DW3_BTI_InputConversionSurface : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 4
    union {
        uint32_t   DW4_BTI_OutputConversionSurface : MOS_BITFIELD_RANGE(0, 31);
        uint32_t   DW4_BTI_InputDsSurface : MOS_BITFIELD_RANGE(0, 31);
        uint32_t   DW4_BTI_Value : MOS_BITFIELD_RANGE(0, 31);
    };

    // DWORD 5
    uint32_t   DW5_BTI_4xDsSurface : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 6
    uint32_t   DW6_BTI_MBStatsSurface : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 7
    uint32_t   DW7_BTI_2xDsSurface : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 8
    uint32_t   DW8_BTI_MB_Split_Surface : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 9
    uint32_t   DW9_BTI_LCU32_JobQueueScratchBufferSurface : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 10
    uint32_t   DW10_BTI_LCU64_CU32_JobQueueScratchBufferSurface : MOS_BITFIELD_RANGE(0, 31);

    // DWORD 11
    uint32_t   DW11_BTI_LCU64_CU32_64x64_DistortionSurface : MOS_BITFIELD_RANGE(0, 31);

};

//!
//! \brief    Surface params for DsConv kernel
//!
struct SurfaceParamsDsConv
{
    PMOS_SURFACE                        psInputSurface = nullptr;
    uint32_t                            dwInputFrameWidth = 0;
    uint32_t                            dwInputFrameHeight = 0;
    PMOS_SURFACE                        psOutputConvertedSurface = nullptr;
    uint32_t                            dwOutputConvertedFrameWidth = 0;
    uint32_t                            dwOutputConvertedFrameHeight = 0;
    PMOS_SURFACE                        psOutputScaledSurface4x = nullptr;
    uint32_t                            dwOutputScaledFrameWidth4x = 0;
    uint32_t                            dwOutputScaledFrameHeight4x = 0;
    PMOS_SURFACE                        psOutputScaledSurface2x = nullptr;
    uint32_t                            dwOutputScaledFrameWidth2x = 0;
    uint32_t                            dwOutputScaledFrameHeight2x = 0;
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC      pBindingTable = nullptr;
    PMHW_KERNEL_STATE                   pKernelState = nullptr;
    DsStage                             downScaleConversionType = dsDisabled;
};

//!  HEVC dual-pipe encoder class for GEN10
/*!
This class defines the member fields, functions for GEN10 platform
*/
class CodechalEncHevcStateG10 : public CodechalEncHevcState
{
public:

    //! TU based param list
    enum {
        EnableCu64CheckTuParam = 0,
        Cu64SkipCheckOnlyTuParam,
        EncQtDecisionModeTuParam,
        EncTuDecisionModeTuParam,
        EncTuDecisionForAllQtTuParam,
        EncRdDecisionModeForAllQtTuParam,
        CoefBitEstModeTuParam,
        MaxNumIMESearchCenterTuParam,
        EncSkipDecisionModeTuParam,
        EncTransformSimplifyTuParam,
        Log2TUMaxDepthInterTuParam,
        Log2TUMaxDepthIntraTuParam,
        TotalThreadNumPerLCUTuParam,
        SICDynamicRunPathMode,
        TotalThreadNumPerLCUTuParamFor4KOnly,
        NumRegionLCU64,
        TotalTuParams
    };

    //! Integer motion estimation ref window mode
    enum
    {
        IME_REF_WINDOW_MODE_RESERVED = 0,
        IME_REF_WINDOW_MODE_BIG,
        IME_REF_WINDOW_MODE_MEDIUM,
        IME_REF_WINDOW_MODE_SMALL,
        IME_REF_WINDOW_MODE_MAX
    };

    static const uint32_t  m_jobQueueSizeFor32x32Block      = 2656;     //!< Job queue size for 32x32 block
    static const uint32_t  m_brcLambdaModeCostTableSize     = 1664;     //!< Size in bytes for BRC lambda mode cost LUT
    static const uint32_t  m_encIConstantDataLutSize        = 24576;    //!< Constant data LUT size in ints for I-kernel
    static const uint32_t  m_encBConstantDataLutSize        = 582240;   //!< Constant data LUT size in ints for B-kernel
    static const uint32_t  m_encBConstantDataLutLcu64Size   = 1003520;  //!< Constant data LUT size in ints for LCU64 B-kernel
    static const uint32_t  m_maxThreadsPerLcuB              = 8;        //!< Maximum number of threads for LCU B
    static const uint32_t  m_minThreadsPerLcuB              = 3;        //!< Minimum number of threads for LCU B
    static const uint32_t  m_brcConstantSurfaceHeight       = 35;       //!< BRC constant surface height
    static const uint32_t  m_minScaledSurfaceSize           = 64;       //!< Minimal scaled surface size
    static const uint32_t  m_brcCombinedEncBufferSize       = 128;      //!< Brc Combined Enc buffer size

    static const uint32_t  m_brcLcu32x32LambdaModeCost[m_brcLambdaModeCostTableSize>>2];   //!< Lambda mode cost table for BRC LCU32x32
    static const uint32_t  m_brcLcu64x64LambdaModeCost[m_brcLambdaModeCostTableSize>>2];   //!< Lambda mode cost table for BRC LCU64x64
    static const uint32_t  m_encIConstantDataLut[m_encIConstantDataLutSize];               //!< Constant data table for I kernel
    static const uint32_t  m_encBConstantDataLut[m_encBConstantDataLutSize];               //!< Constant data table for B kernel
    static const uint32_t  m_encBConstantDataLutLcu64[m_encBConstantDataLutLcu64Size];     //!< Constant data table for LCU64 B kernel

    static const double   m_lambdaScaling[3][QP_NUM];                                   //!< Cost table weighting factor
    static const uint8_t  m_imeRefWindowSize[IME_REF_WINDOW_MODE_MAX][2];               //!< Table specifying the search window size for IME
    static const double   m_modeBits[2][3][15];                                         //!< Mode Bits table for cost computing
    static const uint8_t  m_tuSettings[TotalTuParams][3];                               //!< Table for TU based settings for different TU params

    static const struct CODECHAL_ENC_HEVC_ME_CURBE_G10 m_meCurbeInit;                          //!< Curbe initialization data for ME kernel
    static const struct CODECHAL_ENC_HEVC_BRC_INITRESET_CURBE_G10 m_brcInitResetCurbeInit;     //!< Curbe initialization data for BRC Init/Reset kernel
    static const struct CODECHAL_ENC_HEVC_BRC_UPDATE_CURBE_G10 m_brcUpdateCurbeInit;           //!< Curbe initialization data for BRC Update kernel
    static const struct CODECHAL_ENC_HEVC_MBENC_I_CURBE_G10 m_mbencICurbeInit;                 //!< Curbe initialization data for MBENC I kernel
    static const struct CODECHAL_ENC_HEVC_MBENC_B_CURBE_G10 m_mbencBCurbeInit;                 //!< Curbe initialization data for MBENC B kernel

    CODECHAL_ENCODE_BUFFER  m_kernelDebug;                             //!< Kernel Debug Surface for B-kernel
    MOS_SURFACE             m_intermediateCuRecordSurfaceLcu32;        //!< Intermediate CU Record Surface for I and B kernel
    MOS_SURFACE             m_secondIntermediateCuRecordSurfaceLcu32;  //!< Second Intermediate CU Record Surface for B kernel
    MOS_SURFACE             m_intermediateCuRecordSurfaceLcu64B;       //!< Intermediate CU Record Surface for Lcu64 B-kernel
    CODECHAL_ENCODE_BUFFER  m_encConstantTableForI;                    //!< Enc Constant Table for I
    CODECHAL_ENCODE_BUFFER  m_encConstantTableForB;                    //!< Enc constant table for B LCU32
    CODECHAL_ENCODE_BUFFER  m_encConstantTableForLcu64B;               //!< Enc constant table for B LCU64
    CODECHAL_ENCODE_BUFFER  m_lcuLevelInputData;                       //!< Lcu level input data
    CODECHAL_ENCODE_BUFFER  m_lcuEncodingScratchSurface;               //!< Lcu encoding scratch surface
    CODECHAL_ENCODE_BUFFER  m_lcuEncodingScratchSurfaceLcu64B;         //!< Lcu scratch surface
    CODECHAL_ENCODE_BUFFER  m_64x64DistortionSurface;                  //!< Distortion surface for 64x64
    MOS_SURFACE             m_scratchSurface;                          //!< Scartch Surface for I-kernel
    CODECHAL_ENCODE_BUFFER  m_concurrentThreadGroupData;               //!< Concurrent Thread Group Data Surface
    CODECHAL_ENCODE_BUFFER  m_jobQueueHeaderSurfaceForB;               //!< Job Queue Header buffer surface. When used by LCU64 kernel, it is the 1D header surface with smaller size
    CODECHAL_ENCODE_BUFFER  m_jobQueueHeaderSurfaceForBLcu64;          //!< Job Queue Header buffer surface
    MOS_SURFACE             m_jobQueueDataSurfaceForBLcu64Cu32;        //!< Job Queue Data Surface for LCU64 CU32
    MOS_SURFACE             m_jobQueueDataSurfaceForBLcu64;            //!< Job Queue Data Surface for LCU64
    MOS_SURFACE             m_cuSplitSurface;                          //!< Cu Split Surface
    MOS_SURFACE             m_mbStatisticsSurface;                     //!< MB statistics surface
    MOS_SURFACE             m_mbSplitSurface;                          //!< MB split surface
    MOS_SURFACE             m_residualDataScratchSurfaceForBLcu32;     //!< Residual Data Scratch Surface for LCU 32 B-kernel
    MOS_SURFACE             m_residualDataScratchSurfaceForBLcu64;     //!< Residual Data Scratch Surface for LCU 64 B-kernel
    CODECHAL_ENCODE_BUFFER  m_mvAndDistortionSumSurface;               //!< Mv and Distortion summation surface
    uint32_t                m_totalNumThreadsPerLcu = 0;               //!< Number of threads per LCU

    uint8_t m_modeCost[14] = {0};                                      //!< Mode Cost
    CODECHAL_ENCODE_HEVC_WALKINGPATTERN_PARAM m_walkingPatternParam;   //!< WalkingPattern parameter

    //!
    //! \brief    Constructor
    //!
    CodechalEncHevcStateG10(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncHevcStateG10() {};

    // inherited virtual functions
    MOS_STATUS Initialize(CodechalSetting * settings);
    MOS_STATUS InitKernelState();
    uint32_t GetMaxBtCount();
    MOS_STATUS EncodeKernelFunctions();
    MOS_STATUS AllocateEncResources();
    MOS_STATUS AllocateEncResourcesLCU64();
    MOS_STATUS FreeEncResources();
    MOS_STATUS AllocatePakResources();
    MOS_STATUS FreePakResources();
    MOS_STATUS SetSequenceStructs();
    MOS_STATUS CalcScaledDimensions();
    void GetMaxRefFrames(uint8_t& maxNumRef0, uint8_t& maxNumRef1);

    //!
    //! \brief    Allocate ME resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateMeResources();

    //!
    //! \brief    Free ME resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyMeResources();

    //!
    //! \brief    Get encoder kernel header and kernel size
    //!
    //! \param    [in] binary
    //!           Pointer to kernel binary
    //! \param    [in] operation
    //!           Enc kernel operation
    //! \param    [in] krnStateIdx
    //!           Kernel state index
    //! \param    [out] krnHeader
    //!           Pointer to kernel header
    //! \param    [out] krnSize
    //!           Pointer to kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS GetKernelHeaderAndSize(
        void                           *binary,
        EncOperation                   operation,
        uint32_t                       krnStateIdx,
        void                           *krnHeader,
        uint32_t                       *krnSize);

    //!
    //! \brief    Get encoder kernel header and kernel size
    //!
    //! \param    [in] encOperation
    //!           Specifies the media function type
    //! \param    [in] kernelParams
    //!           Pointer to kernel parameters
    //! \param    [in] idx
    //!           MbEnc/BRC kernel index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetKernelParams(
        EncOperation                    encOperation,
        PMHW_KERNEL_PARAM               kernelParams,
        uint32_t                        idx);

    //!
    //! \brief    Set Binding table for different kernelsge
    //!
    //! \param    [in] encOperation
    //!           Specifies the media function type
    //! \param    [in] bindingTable
    //!           Pointer to the binding table
    //! \param    [in] idx
    //!           MbEnc/BRC kernel index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetBindingTable(
        EncOperation                            encOperation,
        PCODECHAL_ENCODE_BINDING_TABLE_GENERIC  bindingTable,
        uint32_t                                idx);

    //!
    //! \brief    Send surfaces to the ME kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in]  hmeLevel
    //!           Level of HME
    //! \param    [in]  distType
    //!           Type of distortion surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMeSurfaces(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        HmeLevel                            hmeLevel,
        HEVC_ME_DIST_TYPE                   distType);

    //!
    //! \brief    Generate walking control region
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GenerateWalkingControlRegion();

    //!
    //! \brief    Prepare walker params for custom pattern thread dispatch
    //!
    //! \param    [in]  walkerParams
    //!           Pointer to HW walker params
    //! \param    [in]  scoreBoard
    //!           poitner to HW scoreboard
    //! \param    [in]  walkerCodecParams
    //!           Input params to program the HW walker
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetCustomDispatchPattern(
        PMHW_WALKER_PARAMS              walkerParams,
        PMHW_VFE_SCOREBOARD             scoreBoard,
        PCODECHAL_WALKER_CODEC_PARAMS   walkerCodecParams);

    //!
    //! \brief    Prepare the Curbe for ME kernel
    //!
    //! \param    [in]  hmeLevel
    //!           Level of HME
    //! \param    [in]  distType
    //!           Type of distortion surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeMe(
        HmeLevel                    hmeLevel,
        HEVC_ME_DIST_TYPE           distType);

    //!
    //! \brief    Check formats supported
    //!
    //! \param    [in]  surface
    //!           Surface used
    //!
    //! \return   bool
    //!
    bool CheckSupportedFormat(PMOS_SURFACE surface);

    //!
    //! \brief    Invoke HME kernel
    //!
    //! \param    [in]  hmeLevel
    //!           Level of HME
    //! \param    [in]  distType
    //!           Type of distortion surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeMeKernel(
        HmeLevel                        hmeLevel,
        HEVC_ME_DIST_TYPE               distType);

    //!
    //! \brief    Initialize kernel state for Scaling and Depth conversion
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateScalingAndConversion();

    //!
    //! \brief    Initialize HME kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMe();

    //!
    //! \brief    Initialize MbEnc kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateMbEnc();

    //!
    //! \brief    Initialize BRC kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelStateBrc();

    //!
    //! \brief    Send surfaces to the ScalingAndConversion kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in]  params
    //!           Input params for programming the surfaces
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendScalingAndConversionSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer, SurfaceParamsDsConv* params);

    //!
    //! \brief    Set Curbe for ScalingAndConversion kernel
    //!
    //! \param    [in]  params
    //!           Input curbe params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeScalingAndConversion(
        CodechalEncodeCscDs::CurbeParams* params);

    //!
    //! \brief    Invoke Downscaling and Conversion kernel
    //!
    //! \param    [in]  params
    //!           Input params to invoke the kernel 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeScalingAndConversionKernel(
        CodechalEncodeCscDs::KernelParams* params);

    //!
    //! \brief    Top level function for Scaling and Conversion 
    //! \details  ScalingAndConversionKernel is called inside this.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PerformScalingAndConversion();

    //!
    //! \brief    Invoke BRC Init/Reset kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeBrcInitResetKernel();

    //!
    //! \brief    Send surfaces BRC Init/Reset kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in]  krnIdx
    //!           Index of the BRC kernel for which surfaces are being sent
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBrcInitResetSurfaces(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        CODECHAL_HEVC_BRC_KRNIDX            krnIdx);

    //!
    //! \brief    Setup Curbe for BRC Init/Reset kernel
    //!
    //! \param    [in]  brcKrnIdx
    //!           Index of the BRC kernel for which Curbe is setup
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeBrcInitReset(CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx);

    //!
    //! \brief    Invoke frame level BRC update kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeBrcFrameUpdateKernel();

    //!
    //! \brief    Send surfaces for BRC Frame Update kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBrcFrameUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup Curbe for BRC Update kernel
    //!
    //! \param    [in]  brcKrnIdx
    //!           Index of the BRC update kernel(frame or LCU) for which Curbe is setup
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeBrcUpdate(CODECHAL_HEVC_BRC_KRNIDX brcKrnIdx);

    //!
    //! \brief    Invoke LCU level BRC update kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeBrcLcuUpdateKernel();

    //!
    //! \brief    Send surfaces for BRC LCU Update kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBrcLcuUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Top level function for invoking MBenc kernel
    //! \details  I, B or LCU64_B MBEnc kernel, based on encFunctionType
    //! \param    [in]  encFunctionType
    //!           Specifies the media state type
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodeMbEncKernel(CODECHAL_MEDIA_STATE_TYPE encFunctionType);

    //!
    //! \brief    Send Surfaces for MbEnc I kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMbEncSurfacesIKernel(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup Curbe for MbEnc I kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeMbEncIKernel();

    //!
    //! \brief    Send Surfaces for MbEnc B kernel
    //!
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendMbEncSurfacesBKernel(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup Curbe for MbEnc B LCU32 and LCU64_32 Kernels
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeMbEncBKernel();

    //!
    //! \brief    Generate LCU Level Data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GenerateLcuLevelData();

    //!
    //! \brief    Load cost table
    //!
    //! \param    [in]  sliceType
    //!           Slice Type
    //! \param    [in]  qp
    //!           QP value
    //! \param    [out]  lambdaMd
    //!           Intra SAD transform type
    //! \param    [out]  lambdaRd
    //!           Intra SAD transform type
    //! \param    [out]  tuSadThreshold
    //!           Intra SAD transform type
    //!
    //! \return   void
    //!
    void LoadCosts(uint8_t sliceType, uint8_t qp, uint16_t *lambdaMd, uint32_t *lambdaRd, uint32_t *tuSadThreshold);

    //!
    //! \brief    Setup BRC constant data 
    //!
    //! \param    [in, out]  brcConstantData
    //!           Pointer to BRC constant data surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupBrcConstantTable(PMOS_SURFACE brcConstantData);

};

//! \brief  typedef of class CodechalEncHevcStateG10*
using PCODECHAL_ENC_HEVC_STATE_G10 = class CodechalEncHevcStateG10*;

#endif  // __CODECHAL_ENCODE_HEVC_G10_H__
