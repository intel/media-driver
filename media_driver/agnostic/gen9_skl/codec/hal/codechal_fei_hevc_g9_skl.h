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
//! \file     codechal_fei_hevc_g9_skl.h
//! \brief    HEVC FEI dual-pipe encoder for GEN9 SKL.
//!

#ifndef __CODECHAL_FEI_HEVC__G9_SKL_H__
#define __CODECHAL_FEI_HEVC__G9_SKL_H__

#include "codechal_encode_hevc_g9.h"

#ifdef HEVC_FEI_ENABLE_CMRT
#include <map>
#include <string>
#include "CMRTKernel_header_file.h"
#endif

//! HEVC encoder FEI intra 32x32 PU mode decision kernel curbe for GEN9
struct CODECHAL_FEI_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9
{
    union {
        struct {
            uint32_t       FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);   
            uint32_t       FrameHeight                  : MOS_BITFIELD_RANGE(16, 31); 
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       SliceType                    : MOS_BITFIELD_RANGE(0, 1);   
            uint32_t       PuType                       : MOS_BITFIELD_RANGE(2, 3);
            uint32_t       EnableStatisticsDataDump     : MOS_BITFIELD_BIT(4);
            uint32_t       LCUType                      : MOS_BITFIELD_BIT(5); 
            uint32_t       Res_6_15                     : MOS_BITFIELD_RANGE(6, 15);   
            uint32_t       SliceQp                      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       BRCEnable                    : MOS_BITFIELD_BIT(24); 
            uint32_t       LCUBRCEnable                 : MOS_BITFIELD_BIT(25);
            uint32_t       ROIEnable                    : MOS_BITFIELD_BIT(26);
            uint32_t       FASTSurveillanceFlag         : MOS_BITFIELD_BIT(27);
            uint32_t       Res_28_29                    : MOS_BITFIELD_RANGE(28, 29);
            uint32_t       bQualityImprovementEnable    : MOS_BITFIELD_BIT(30);
            uint32_t       EnableDebugDump              : MOS_BITFIELD_BIT(31); 
        };
        uint32_t Value;
    } DW1;
    
    union {
        struct {
            uint32_t       Lambda;
        };
        uint32_t Value;
    } DW2;
    
    union {
        struct {
            uint32_t       ModeCost32x32;
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       EarlyExit;
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       NewLambdaForHaarTransform;
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       BTI_32x32PU_Output;
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       BTI_Src_Y;
        };
        uint32_t Value;
    } DW9;

    union {
        struct {
            uint32_t       BTI_Src_Y2x;
        };
        uint32_t Value;
    } DW10;

    union {
        struct {
            uint32_t       BTI_Slice_Map;
        };
        uint32_t Value;
    } DW11;

    union {
        struct {
            uint32_t       BTI_Src_Y2x_VME;   
        };
        uint32_t Value;
    } DW12;

    union {
        struct {
            uint32_t       BTI_Brc_Input;   
        };
        uint32_t Value;
    } DW13;

    union {
        struct {
            uint32_t       BTI_LCU_Qp_Surface;
        };
        uint32_t Value;
    } DW14;

    union {
        struct {
            uint32_t       BTI_Brc_Data;
        };
        uint32_t Value;
    } DW15;

    union {
        struct {
            uint32_t       BTI_Stats_Surface;
        };
        uint32_t Value;
    } DW16;

    union {
        struct {
            uint32_t       BTI_Kernel_Debug;   
        };
        uint32_t Value;
    } DW17;
};

using PCODECHAL_FEI_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9 = struct CODECHAL_FEI_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_FEI_HEVC_I_32x32_PU_MODE_DECISION_CURBE_G9)) == 18);

//! HEVC encoder FEI intra 16x16 PU mode decision kernel curbe for GEN9
struct CODECHAL_FEI_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9
{
    union {
        struct {
            uint32_t       FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);   
            uint32_t       FrameHeight                  : MOS_BITFIELD_RANGE(16, 31); 
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       Log2MaxCUSize                : MOS_BITFIELD_RANGE(0, 7);   
            uint32_t       Log2MinCUSize                : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       Log2MinTUSize                : MOS_BITFIELD_RANGE(16, 23); 
            uint32_t       SliceQp                      : MOS_BITFIELD_RANGE(24, 31);   
        };
        uint32_t Value;
    } DW1;
    
    union {
        struct {
            uint32_t       FixedPoint_Lambda_PredMode;
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t       LambdaScalingFactor          : MOS_BITFIELD_RANGE(0, 7);   
            uint32_t       SliceType                    : MOS_BITFIELD_RANGE(8, 9);   
            uint32_t       Reserved_10_15               : MOS_BITFIELD_RANGE(10, 15);
            uint32_t       IntraRefreshEn               : MOS_BITFIELD_RANGE(16, 17);
            uint32_t       EnableRollingIntra           : MOS_BITFIELD_BIT(18);
            uint32_t       HalfUpdateMixedLCU           : MOS_BITFIELD_BIT(19);
            uint32_t       Reserved_20_23               : MOS_BITFIELD_RANGE(20, 23);
            uint32_t       EnableIntraEarlyExit         : MOS_BITFIELD_BIT(24);
            uint32_t       BRCEnable                    : MOS_BITFIELD_BIT(25);
            uint32_t       LCUBRCEnable                 : MOS_BITFIELD_BIT(26);
            uint32_t       ROIEnable                    : MOS_BITFIELD_BIT(27);
            uint32_t       FASTSurveillanceFlag         : MOS_BITFIELD_BIT(28);
            uint32_t       bQualityImprovementEnable    : MOS_BITFIELD_BIT(29);
            uint32_t       Reserved_30_31               : MOS_BITFIELD_RANGE(30, 31);
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       PenaltyForIntra8x8NonDCPredMode : MOS_BITFIELD_RANGE(0, 7);
            uint32_t       IntraComputeType             : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       AVCIntra8x8Mask              : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       IntraSadAdjust               : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       FixedPoint_Lambda_CU_Mode_for_Cost_Calculation;
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       ScreenContentFlag            : MOS_BITFIELD_BIT(0); 
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(1, 31);
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       ModeCostIntraNonPred         : MOS_BITFIELD_RANGE(0, 7);   
            uint32_t       ModeCostIntra16x16           : MOS_BITFIELD_RANGE(8, 15);
            uint32_t       ModeCostIntra8x8             : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       ModeCostIntra4x4             : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       FixedPoint_Lambda_CU_Mode_for_Luma;
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       IntraRefreshMBNum            : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       IntraRefreshUnitInMB         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       IntraRefreshQPDelta          : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW9;

    union {
        struct {
            // 0: for intra, no Haar
            // 1: for inter or enable statictics data dump, need Haar
            uint32_t       HaarTransformMode            : MOS_BITFIELD_RANGE(0, 1);
            uint32_t       SimplifiedFlagForInter       : MOS_BITFIELD_BIT(2); 
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(3, 31);
        };
        uint32_t Value;
    } DW10;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW11;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW12;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW13;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW14;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW15;

    union {
        struct {
            uint32_t       BTI_Src_Y;
        };
        uint32_t Value;
    } DW16;

    union {
        struct {
            uint32_t       BTI_Sad_16x16_PU;
        };
        uint32_t Value;
    } DW17;

    union {
        struct {
            uint32_t       BTI_PAK_Object;
        };
        uint32_t Value;
    } DW18;

    union {
        struct {
            uint32_t       BTI_SAD_32x32_PU_mode;
        };
        uint32_t Value;
    } DW19;

    union {
        struct {
            uint32_t       BTI_VME_Mode_8x8;
        };
        uint32_t Value;
    } DW20;

    union {
        struct {
            uint32_t       BTI_Slice_Map;
        };
        uint32_t Value;
    } DW21;

    union {
        struct {
            uint32_t       BTI_VME_Src;
        };
        uint32_t Value;
    } DW22;

    union {
        struct {
            uint32_t       BTI_BRC_Input;
        };
        uint32_t Value;
    } DW23;

    union {
        struct {
            uint32_t       BTI_Simplest_Intra;
        };
        uint32_t Value;
    } DW24;

    union {
        struct {
            uint32_t       BTI_LCU_Qp_Surface;
        };
        uint32_t Value;
    } DW25;

    union {
        struct {
            uint32_t       BTI_BRC_Data;
        };
        uint32_t Value;
    } DW26;

    union {
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW27;
};

using PCODECHAL_FEI_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9 = struct CODECHAL_FEI_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_FEI_HEVC_I_16x16_PU_MODEDECISION_CURBE_G9)) == 28);

//! HEVC encoder FEI intra 8x8 PU kernel curbe for GEN9
struct CODECHAL_FEI_HEVC_I_8x8_PU_CURBE_G9
{
    union {
        struct {
            uint32_t       FrameWidth                   : MOS_BITFIELD_RANGE(0, 15);   
            uint32_t       FrameHeight                  : MOS_BITFIELD_RANGE(16, 31); 
        };
        uint32_t Value;
    } DW0;

    union {
        struct {
            uint32_t       SliceType                    : MOS_BITFIELD_RANGE(0, 1);   
            uint32_t       PuType                       : MOS_BITFIELD_RANGE(2, 3);
            uint32_t       DcFilterFlag                 : MOS_BITFIELD_BIT(4); 
            uint32_t       AngleRefineFlag              : MOS_BITFIELD_BIT(5);   
            uint32_t       LCUType                      : MOS_BITFIELD_BIT(6);   
            uint32_t       ScreenContentFlag            : MOS_BITFIELD_BIT(7);   
            uint32_t       IntraRefreshEn               : MOS_BITFIELD_RANGE(8, 9);
            uint32_t       EnableRollingIntra           : MOS_BITFIELD_BIT(10);
            uint32_t       HalfUpdateMixedLCU           : MOS_BITFIELD_BIT(11);
            uint32_t       Reserved_12_15               : MOS_BITFIELD_RANGE(12, 15);
            uint32_t       QPValue                      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       EnableIntraEarlyExit         : MOS_BITFIELD_BIT(24);   
            uint32_t       BRCEnable                    : MOS_BITFIELD_BIT(25);   
            uint32_t       LCUBRCEnable                 : MOS_BITFIELD_BIT(26);
            uint32_t       ROIEnable                    : MOS_BITFIELD_BIT(27);
            uint32_t       FASTSurveillanceFlag         : MOS_BITFIELD_BIT(28);
            uint32_t       Reserved_29                  : MOS_BITFIELD_BIT(29);
            uint32_t       bQualityImprovementEnable    : MOS_BITFIELD_BIT(30);
            uint32_t       EnableDebugDump              : MOS_BITFIELD_BIT(31);   
        };
        uint32_t Value;
    } DW1;
    
    union {
        struct {
            uint32_t       LumaLambda;
        };
        uint32_t Value;
    } DW2;

    union {
        struct {
            uint32_t       ChromaLambda;
        };
        uint32_t Value;
    } DW3;

    union {
        struct {
            uint32_t       HaarTransformFlag            : MOS_BITFIELD_RANGE(0, 1);   
            uint32_t       SimplifiedFlagForInter       : MOS_BITFIELD_BIT(2);
            uint32_t       Reserved                     : MOS_BITFIELD_RANGE(3, 31);   
        };
        uint32_t Value;
    } DW4;

    union {
        struct {
            uint32_t       IntraRefreshMBNum            : MOS_BITFIELD_RANGE(0, 15);
            uint32_t       IntraRefreshUnitInMB         : MOS_BITFIELD_RANGE(16, 23);
            uint32_t       IntraRefreshQPDelta          : MOS_BITFIELD_RANGE(24, 31);
        };
        uint32_t Value;
    } DW5;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW6;

    union {
        struct {
            uint32_t       Reserved;
        };
        uint32_t Value;
    } DW7;

    union {
        struct {
            uint32_t       BTI_Src_Y;
        };
        uint32_t Value;
    } DW8;

    union {
        struct {
            uint32_t       BTI_Slice_Map;
        };
        uint32_t Value;
    } DW9;

    union {
        struct {
            uint32_t       BTI_VME_8x8_Mode;
        };
        uint32_t Value;
    } DW10;

    union {
        struct {
            uint32_t       BTI_Intra_Mode;
        };
        uint32_t Value;
    } DW11;

    union {
        struct {
            uint32_t       BTI_BRC_Input;
        };
        uint32_t Value;
    } DW12;

    union {
        struct {
            uint32_t       BTI_Simplest_Intra;
        };
        uint32_t Value;
    } DW13;

    union {
        struct {
            uint32_t       BTI_LCU_Qp_Surface;
        };
        uint32_t Value;
    } DW14;

    union {
        struct {
            uint32_t       BTI_BRC_Data;
        };
        uint32_t Value;
    } DW15;

    union {
        struct {
            uint32_t       BTI_Debug;
        };
        uint32_t Value;
    } DW16;
};

using PCODECHAL_FEI_HEVC_I_8x8_PU_CURBE_G9 = struct CODECHAL_FEI_HEVC_I_8x8_PU_CURBE_G9*;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_FEI_HEVC_I_8x8_PU_CURBE_G9)) == 17);

class CodechalFeiHevcStateG9Skl : public CodechalEncHevcStateG9
{
//    static constexpr uint32_t                   NUM_CONCURRENT_THREAD = 2;              //!< Number of concurrent threads
//    static constexpr uint32_t                   MAX_NUM_KERNEL_SPLIT  = 8;              //!< Maximal kernel split number


    static const uint32_t                       FEI_ENCB_B_CURBE_INIT[56];              //!< Initialization FEI ENC curbe for B frame
    static const uint32_t                       FEI_ENCB_P_CURBE_INIT[56];              //!< Initialization FEI ENC curbe for P frame
    static const uint32_t                       FEI_ENCB_I_CURBE_INIT[56];              //!< Initialization FEI ENC curbe for I frame
#ifdef HEVC_FEI_ENABLE_CMRT
typedef std::map<std::string, CMRTKernelBase *> CmKernelMapType;
    CmKernelMapType                             m_cmKernelMap;
    CmEvent                                     *m_cmEvent;
#endif
public:
    //!
    //! \brief    Constructor
    //!     
    CodechalFeiHevcStateG9Skl(CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalFeiHevcStateG9Skl() {};

private:
    // Resources for the render engine
    MOS_SURFACE                                 m_lcuQP;                                 //!< input LCU QP surface

    CodecEncodeHevcFeiPicParams *m_feiPicParams;

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

    // inherited virtual functions

    //!
    //! \brief    Set MbEnc kernel parameters
    //!
    //! \param    [in]  pKernelParams
    //!           Pointer to kernel parameters
    //! \param    [in]  idx
    //!           MbEnc kernel index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMbEncKernelParams(MHW_KERNEL_PARAM* pKernelParams, uint32_t idx);

    //!
    //! \brief    Set MbEnc kernel binding table
    //!
    //! \param    [in]  pBindingTable
    //!           Pointer to binding table
    //! \param    [in]  idx
    //!           MbEnc kernel index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMbEncBindingTable(PCODECHAL_ENCODE_BINDING_TABLE_GENERIC pBindingTable, uint32_t idx);

    //!
    //! \brief    End kernel function and submit command buffer
    //!
    //! \param    [in]  mediaStateType
    //!           Media state type
    //! \param    [in]  kernelState
    //!           Pointer to kernel state
    //! \param    [in]  cmdBuffer
    //!           Pointer to command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EndKernelCall(
        CODECHAL_MEDIA_STATE_TYPE       mediaStateType,
        PMHW_KERNEL_STATE               kernelState,
        PMOS_COMMAND_BUFFER             cmdBuffer);

    virtual MOS_STATUS InitKernelState();
    virtual MOS_STATUS Initialize(CodechalSetting * pSettings);
    virtual uint32_t GetMaxBtCount();
    virtual MOS_STATUS AllocateEncResources();
    virtual MOS_STATUS FreeEncResources();
    virtual MOS_STATUS InitSurfaceInfoTable();
    virtual MOS_STATUS SetSequenceStructs();

    //!
    //! \brief    Invoke 2x down scaling kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Encode2xScalingKernel();

    //!
    //! \brief    Invoke 32x32 PU mode decision kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Encode32x32PuModeDecisionKernel();

    //!
    //! \brief    Invoke 16x16 PU SAD computation kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Encode16x16SadPuComputationKernel(); 

    //!
    //! \brief    Invoke 16x16 PU mode decision kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Encode16x16PuModeDecisionKernel();

    //!
    //! \brief    Invoke 8x8 PU kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Encode8x8PUKernel();

    //!
    //! \brief    Invoke 8x8 PU FMode kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Encode8x8PUFMODEKernel();

    //!
    //! \brief    Invoke 32x32 B intra check kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Encode32X32BIntraCheckKernel();

    //!
    //! \brief    Invoke 8x8 B Pak kernel
    //!
    //! \param    [in]  pCurbe
    //!           Pointer to B_MB_ENC curbe structure
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Encode8x8BPakKernel(struct CODECHAL_FEI_HEVC_B_MB_ENC_CURBE_G9* pCurbe);

    //!
    //! \brief    Invoke 8x8 B MbEnc kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Encode8x8PBMbEncKernel();

    //!
    //! \brief    Encode kernel functions
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EncodeKernelFunctions();

    //!
    //! \brief    Initialize surface info
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
//    virtual MOS_STATUS InitSurfaceInfoTable();

    // Inherited virtual function
    bool UsePlatformControlFlag()
    {
        return false;
    }
};

#endif  // __CODECHAL_FEI_HEVC__G9_SKL_H__

