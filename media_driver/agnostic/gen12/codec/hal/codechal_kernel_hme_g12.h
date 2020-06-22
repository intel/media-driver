/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_kernel_hme_g11.h
//! \brief    Hme kernel implementation for Gen11 platform
//!
#ifndef __CODECHAL_KERNEL_HME_G12_H__
#define __CODECHAL_KERNEL_HME_G12_H__
#include "codechal_kernel_hme.h"

class CodechalKernelHmeG12 : public CodechalKernelHme
{
public:
    enum KernelIndex
    {
        hmeP                    = 0,
        hmeB                    = 1,
        hmeVDEncStreamIn        = 2,
        hmeVDEncHevcVp9StreamIn = 3
    };

    enum BindingTableOffset
    {
        meOutputMvDataSurface       = 0,
        meInputMvDataSurface        = 1,
        meDistortionSurface         = 2,
        meBrcDistortion             = 3,
        meCurrForFwdRef             = 5,
        meFwdRefIdx0                = 6,
        meFwdRefIdx1                = 8,
        meFwdRefIdx2                = 10,
        meFwdRefIdx3                = 12,
        meFwdRefIdx4                = 14,
        meFwdRefIdx5                = 16,
        meFwdRefIdx6                = 18,
        meFwdRefIdx7                = 20,
        meCurrForBwdRef             = 22,
        meBwdRefIdx0                = 23,
        meBwdRefIdx1                = 25,
        meVdencStreamInOutputBuffer = 26,
        meVdencStreamInInputBuffer  = 27,
        meSumMvandDistortionBuffer  = 28,
        meSurfaceNum                = 29
    };

    // clang-format off
    class Curbe
    {
    public:
        Curbe()
        {
            MOS_SecureMemcpy(&m_data, sizeof(m_data), &m_initCurbe, sizeof(m_initCurbe));
        }
        struct CurbeData
        {
            // DW0
            union
            {
                struct
                {
                    uint32_t SkipModeEn         : MOS_BITFIELD_BIT(0);
                    uint32_t AdaptiveEn         : MOS_BITFIELD_BIT(1);
                    uint32_t BiMixDis           : MOS_BITFIELD_BIT(2);
                    uint32_t reserved3          : MOS_BITFIELD_RANGE(3, 4);
                    uint32_t EarlyImeSuccessEn  : MOS_BITFIELD_BIT(5);
                    uint32_t reserved6          : MOS_BITFIELD_BIT(6);
                    uint32_t T8x8FlagForInterEn : MOS_BITFIELD_BIT(7);
                    uint32_t reserved8          : MOS_BITFIELD_RANGE(8, 23);
                    uint32_t EarlyImeStop       : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW0;
            // DW1
            union
            {
                struct
                {
                    uint32_t MaxNumMVs     : MOS_BITFIELD_RANGE(0, 5);
                    uint32_t reserved6     : MOS_BITFIELD_RANGE(6, 15);
                    uint32_t BiWeight      : MOS_BITFIELD_RANGE(16, 21);
                    uint32_t reserved22    : MOS_BITFIELD_RANGE(22, 27);
                    uint32_t UniMixDisable : MOS_BITFIELD_BIT(28);
                    uint32_t reserved29    : MOS_BITFIELD_RANGE(29, 31);
                };
                uint32_t Value;
            } DW1;
            // DW2
            union
            {
                struct
                {
                    uint32_t MaxLenSP   : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t MaxNumSU   : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t reserved16 : MOS_BITFIELD_RANGE(16, 31);
                };
                uint32_t Value;
            } DW2;
            // DW3
            union
            {
                struct
                {
                    uint32_t SrcSize                : MOS_BITFIELD_RANGE(0, 1);
                    uint32_t reserved2              : MOS_BITFIELD_RANGE(2, 3);
                    uint32_t MbTypeRemap            : MOS_BITFIELD_RANGE(4, 5);
                    uint32_t SrcAccess              : MOS_BITFIELD_BIT(6);
                    uint32_t RefAccess              : MOS_BITFIELD_BIT(7);
                    uint32_t SearchCtrl             : MOS_BITFIELD_RANGE(8, 10);
                    uint32_t DualSearchPathOption   : MOS_BITFIELD_BIT(11);
                    uint32_t SubPelMode             : MOS_BITFIELD_RANGE(12, 13);
                    uint32_t SkipType               : MOS_BITFIELD_BIT(14);
                    uint32_t DisableFieldCacheAlloc : MOS_BITFIELD_BIT(15);
                    uint32_t InterChromaMode        : MOS_BITFIELD_BIT(16);
                    uint32_t FTEnable               : MOS_BITFIELD_BIT(17);
                    uint32_t BMEDisableFBR          : MOS_BITFIELD_BIT(18);
                    uint32_t BlockBasedSkipEnable   : MOS_BITFIELD_BIT(19);
                    uint32_t InterSAD               : MOS_BITFIELD_RANGE(20, 21);
                    uint32_t IntraSAD               : MOS_BITFIELD_RANGE(22, 23);
                    uint32_t SubMbPartMask          : MOS_BITFIELD_RANGE(24, 30);
                    uint32_t reserved31             : MOS_BITFIELD_BIT(31);
                };
                uint32_t Value;
            } DW3;
            // DW4
            union
            {
                struct
                {
                    uint32_t reserved0           : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t PictureHeightMinus1 : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t PictureWidth        : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t reserved24          : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW4;
            // DW5
            union
            {
                struct
                {
                    uint32_t SumMVThreshold : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t QpPrimeY  : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t RefWidth  : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t RefHeight : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW5;
            // DW6
            union
            {
                struct
                {
                    uint32_t reserved0         : MOS_BITFIELD_BIT(0);
                    uint32_t InputStreamInEn   : MOS_BITFIELD_BIT(1);
                    uint32_t LCUSize           : MOS_BITFIELD_BIT(2);
                    uint32_t WriteDistortions  : MOS_BITFIELD_BIT(3);
                    uint32_t UseMvFromPrevStep : MOS_BITFIELD_BIT(4);
                    uint32_t BRCEnable         : MOS_BITFIELD_BIT(5);
                    uint32_t reserved5         : MOS_BITFIELD_RANGE(6, 7);
                    uint32_t SuperCombineDist  : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t MaxVmvR           : MOS_BITFIELD_RANGE(16, 31);
                };
                uint32_t Value;
            } DW6;
            // DW7
            union
            {
                struct
                {
                    uint32_t reserved0         : MOS_BITFIELD_RANGE(0, 15);
                    uint32_t MVCostScaleFactor : MOS_BITFIELD_RANGE(16, 17);
                    uint32_t BilinearEnable    : MOS_BITFIELD_BIT(18);
                    uint32_t SrcFieldPolarity  : MOS_BITFIELD_BIT(19);
                    uint32_t WeightedSADHAAR   : MOS_BITFIELD_BIT(20);
                    uint32_t AConlyHAAR        : MOS_BITFIELD_BIT(21);
                    uint32_t RefIDCostMode     : MOS_BITFIELD_BIT(22);
                    uint32_t reserved23        : MOS_BITFIELD_BIT(23);
                    uint32_t SkipCenterMask    : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW7;
            // DW8
            union
            {
                struct
                {
                    uint32_t Mode0Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t Mode1Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t Mode2Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t Mode3Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW8;
            // DW9
            union
            {
                struct
                {
                    uint32_t Mode4Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t Mode5Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t Mode6Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t Mode7Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW9;
            // DW10
            union
            {
                struct
                {
                    uint32_t Mode8Cost           : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t Mode9Cost           : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t RefIDCost           : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t ChromaIntraModeCost : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW10;
            // DW11
            union
            {
                struct
                {
                    uint32_t MV0Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t MV1Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t MV2Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t MV3Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW11;
            // DW12
            union
            {
                struct
                {
                    uint32_t MV4Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t MV5Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t MV6Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t MV7Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW12;
            // DW13
            union
            {
                struct
                {
                    uint32_t NumRefIdxL0MinusOne : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t NumRefIdxL1MinusOne : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t RefStreaminCost     : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t ROIEnable           : MOS_BITFIELD_RANGE(24, 26);
                    uint32_t reserved27          : MOS_BITFIELD_RANGE(27, 31);
                };
                uint32_t Value;
            } DW13;
            // DW14
            union
            {
                struct
                {
                    uint32_t List0RefID0FieldParity : MOS_BITFIELD_BIT(0);
                    uint32_t List0RefID1FieldParity : MOS_BITFIELD_BIT(1);
                    uint32_t List0RefID2FieldParity : MOS_BITFIELD_BIT(2);
                    uint32_t List0RefID3FieldParity : MOS_BITFIELD_BIT(3);
                    uint32_t List0RefID4FieldParity : MOS_BITFIELD_BIT(4);
                    uint32_t List0RefID5FieldParity : MOS_BITFIELD_BIT(5);
                    uint32_t List0RefID6FieldParity : MOS_BITFIELD_BIT(6);
                    uint32_t List0RefID7FieldParity : MOS_BITFIELD_BIT(7);
                    uint32_t List1RefID0FieldParity : MOS_BITFIELD_BIT(8);
                    uint32_t List1RefID1FieldParity : MOS_BITFIELD_BIT(9);
                    uint32_t reserved10             : MOS_BITFIELD_RANGE(10, 31);
                };
                uint32_t Value;
            } DW14;
            // DW15
            union
            {
                struct
                {
                    uint32_t PrevMvReadPosFactor : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t MvShiftFactor       : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t Reserved            : MOS_BITFIELD_RANGE(16, 31);
                };
                uint32_t Value;
            } DW15;

            struct
            {
                // DW16
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_0;
                        SearchPathDelta SPDelta_1;
                        SearchPathDelta SPDelta_2;
                        SearchPathDelta SPDelta_3;
                    };
                    uint32_t Value;
                } DW16;
                // DW17
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_4;
                        SearchPathDelta SPDelta_5;
                        SearchPathDelta SPDelta_6;
                        SearchPathDelta SPDelta_7;
                    };
                    uint32_t Value;
                } DW17;
                // DW18
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_8;
                        SearchPathDelta SPDelta_9;
                        SearchPathDelta SPDelta_10;
                        SearchPathDelta SPDelta_11;
                    };
                    uint32_t Value;
                } DW18;
                // DW19
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_12;
                        SearchPathDelta SPDelta_13;
                        SearchPathDelta SPDelta_14;
                        SearchPathDelta SPDelta_15;
                    };
                    uint32_t Value;
                } DW19;
                // DW20
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_16;
                        SearchPathDelta SPDelta_17;
                        SearchPathDelta SPDelta_18;
                        SearchPathDelta SPDelta_19;
                    };
                    uint32_t Value;
                } DW20;
                // DW21
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_20;
                        SearchPathDelta SPDelta_21;
                        SearchPathDelta SPDelta_22;
                        SearchPathDelta SPDelta_23;
                    };
                    uint32_t Value;
                } DW21;
                // DW22
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_24;
                        SearchPathDelta SPDelta_25;
                        SearchPathDelta SPDelta_26;
                        SearchPathDelta SPDelta_27;
                    };
                    uint32_t Value;
                } DW22;
                // DW23
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_28;
                        SearchPathDelta SPDelta_29;
                        SearchPathDelta SPDelta_30;
                        SearchPathDelta SPDelta_31;
                    };
                    uint32_t Value;
                } DW23;
                // DW24
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_32;
                        SearchPathDelta SPDelta_33;
                        SearchPathDelta SPDelta_34;
                        SearchPathDelta SPDelta_35;
                    };
                    uint32_t Value;
                } DW24;
                // DW25
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_36;
                        SearchPathDelta SPDelta_37;
                        SearchPathDelta SPDelta_38;
                        SearchPathDelta SPDelta_39;
                    };
                    uint32_t Value;
                } DW25;
                // DW26
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_40;
                        SearchPathDelta SPDelta_41;
                        SearchPathDelta SPDelta_42;
                        SearchPathDelta SPDelta_43;
                    };
                    uint32_t Value;
                } DW26;
                // DW27
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_44;
                        SearchPathDelta SPDelta_45;
                        SearchPathDelta SPDelta_46;
                        SearchPathDelta SPDelta_47;
                    };
                    uint32_t Value;
                } DW27;
                // DW28
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_48;
                        SearchPathDelta SPDelta_49;
                        SearchPathDelta SPDelta_50;
                        SearchPathDelta SPDelta_51;
                    };
                    uint32_t Value;
                } DW28;
                // DW29
                union
                {
                    struct
                    {
                        SearchPathDelta SPDelta_52;
                        SearchPathDelta SPDelta_53;
                        SearchPathDelta SPDelta_54;
                        SearchPathDelta SPDelta_55;
                    };
                    uint32_t Value;
                } DW29;
            } SpDelta;

            // DW30
            union
            {
                struct
                {
                    uint32_t ActualMBWidth  : MOS_BITFIELD_RANGE(0, 15);
                    uint32_t ActualMBHeight : MOS_BITFIELD_RANGE(16, 31);
                };
                uint32_t Value;
            } DW30;
            // DW31
            union
            {
                struct
                {
                    uint32_t RoiCtrl          : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t MaxTuSize        : MOS_BITFIELD_RANGE(8, 9);
                    uint32_t MaxCuSize        : MOS_BITFIELD_RANGE(10, 11);
                    uint32_t NumImePredictors : MOS_BITFIELD_RANGE(12, 15);
                    uint32_t Reserved         : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t PuTypeCtrl       : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW31;
            // DW32
            union
            {
                struct
                {
                    uint32_t ForceMvx0 : MOS_BITFIELD_RANGE(0, 15);
                    uint32_t ForceMvy0 : MOS_BITFIELD_RANGE(16, 31);
                };
                uint32_t Value;
            } DW32;
            // DW33
            union
            {
                struct
                {
                    uint32_t ForceMvx1 : MOS_BITFIELD_RANGE(0, 15);
                    uint32_t ForceMvy1 : MOS_BITFIELD_RANGE(16, 31);
                };
                uint32_t Value;
            } DW33;
            // DW34
            union
            {
                struct
                {
                    uint32_t ForceMvx2 : MOS_BITFIELD_RANGE(0, 15);
                    uint32_t ForceMvy2 : MOS_BITFIELD_RANGE(16, 31);
                };
                uint32_t Value;
            } DW34;
            // DW35
            union
            {
                struct
                {
                    uint32_t ForceMvx3 : MOS_BITFIELD_RANGE(0, 15);
                    uint32_t ForceMvy3 : MOS_BITFIELD_RANGE(16, 31);
                };
                uint32_t Value;
            } DW35;
            // DW36
            union
            {
                struct
                {
                    uint32_t ForceRefIdx0        : MOS_BITFIELD_RANGE(0, 3);
                    uint32_t ForceRefIdx1        : MOS_BITFIELD_RANGE(4, 7);
                    uint32_t ForceRefIdx2        : MOS_BITFIELD_RANGE(8, 11);
                    uint32_t ForceRefIdx3        : MOS_BITFIELD_RANGE(12, 15);
                    uint32_t NumMergeCandCu8x8   : MOS_BITFIELD_RANGE(16, 19);
                    uint32_t NumMergeCandCu16x16 : MOS_BITFIELD_RANGE(20, 23);
                    uint32_t NumMergeCandCu32x32 : MOS_BITFIELD_RANGE(24, 27);
                    uint32_t NumMergeCandCu64x64 : MOS_BITFIELD_RANGE(28, 31);
                };
                uint32_t Value;
            } DW36;
            // DW37
            union
            {
                struct
                {
                    uint32_t SegID            : MOS_BITFIELD_RANGE(0, 15);
                    uint32_t QpEnable         : MOS_BITFIELD_RANGE(16, 19);
                    uint32_t SegIDEnable      : MOS_BITFIELD_BIT(20);
                    uint32_t Reserved         : MOS_BITFIELD_RANGE(21, 22);
                    uint32_t ForceRefIdEnable : MOS_BITFIELD_BIT(23);
                    uint32_t Reserved1        : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW37;
            // DW38
            union
            {
                struct
                {
                    uint32_t ForceQp0 : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t ForceQp1 : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t ForceQp2 : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t ForceQp3 : MOS_BITFIELD_RANGE(24, 31);
                };
                uint32_t Value;
            } DW38;
            // DW39
            union
            {
                struct
                {
                    uint32_t Reserved;
                };
                uint32_t Value;
            } DW39;
            // DW40
            union
            {
                struct
                {
                    uint32_t _4xMeMvOutputDataSurfIndex;
                };
                uint32_t Value;
            } DW40;
            // DW41
            union
            {
                struct
                {
                    uint32_t _16xOr32xMeMvInputDataSurfIndex;
                };
                uint32_t Value;
            } DW41;
            // DW42
            union
            {
                struct
                {
                    uint32_t _4xMeOutputDistSurfIndex;
                };
                uint32_t Value;
            } DW42;
            // DW43
            union
            {
                struct
                {
                    uint32_t _4xMeOutputBrcDistSurfIndex;
                };
                uint32_t Value;
            } DW43;
            // DW44
            union
            {
                struct
                {
                    uint32_t VMEFwdInterPredictionSurfIndex;
                };
                uint32_t Value;
            } DW44;
            // DW45
            union
            {
                struct
                {
                    uint32_t VMEBwdInterPredictionSurfIndex;
                };
                uint32_t Value;
            } DW45;
            // DW46
            union
            {
                struct
                {
                    uint32_t VDEncStreamInOutputSurfIndex;
                };
                uint32_t Value;
            } DW46;
            // DW47
            union
            {
                struct
                {
                    uint32_t VDEncStreamInInputSurfIndex;
                };
                uint32_t Value;
            } DW47;
            // DW48
            union
            {
                struct
                {
                    uint32_t SumMVandDistortionOutputSurfIndex;
                };
                uint32_t Value;
            } DW48;
        } m_data;

        static const uint32_t m_curbeSize = sizeof(CurbeData);
        static const uint32_t m_initCurbe[49];
    };
    // clang-format on

public:
    //!
    //! \brief Constructor
    //!
    //! \param  [in] me4xDistBufferSupported
    //!         flag to support 4x Distortion buffer
    //!
    CodechalKernelHmeG12(
        CodechalEncoderState *encoder,
        bool     me4xDistBufferSupported = true);

    uint32_t GetBTCount() override { return BindingTableOffset::meSurfaceNum; }

protected:
    uint32_t   GetCurbeSize() override { return Curbe::m_curbeSize; }
    MOS_STATUS SetCurbe(MHW_KERNEL_STATE *kernelState) override;
    MOS_STATUS SendSurfaces(PMOS_COMMAND_BUFFER cmd, MHW_KERNEL_STATE *kernelState) override;
    MHW_KERNEL_STATE *        GetActiveKernelState() override;
    CODECHAL_MEDIA_STATE_TYPE GetMediaStateType() override;
};

#endif /* __CODECHAL_KERNEL_HME_G12_H__ */
