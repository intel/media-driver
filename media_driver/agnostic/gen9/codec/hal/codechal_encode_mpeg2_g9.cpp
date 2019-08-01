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
//! \file     codechal_encode_mpeg2_g9.cpp
//! \brief    MPEG2 dual-pipe encoder for GEN9.
//!

#include "codechal_encode_mpeg2_g9.h"
#include "igcodeckrn_g9.h"

struct KernelHeader
{
    uint32_t m_kernelCount;

    // MBEnc Norm/Perf/Quality moes for frame
    CODECHAL_KERNEL_HEADER m_mpeg2MbEncI;
    CODECHAL_KERNEL_HEADER m_mpeg2MbEncP;
    CODECHAL_KERNEL_HEADER m_mpeg2MbEncB;

    // ME Downscale
    CODECHAL_KERNEL_HEADER m_plyDscalePly;

    // AVC_ME kernels for distortion
    CODECHAL_KERNEL_HEADER m_mpeg2AvcMeP;
    CODECHAL_KERNEL_HEADER m_mpeg2AvcMeB;

    CODECHAL_KERNEL_HEADER m_mpeg2InitFrameBrc;
    CODECHAL_KERNEL_HEADER m_mpeg2FrameEncUpdate;
    CODECHAL_KERNEL_HEADER m_mpeg2BrcResetFrame;

    // BRCBlockCopy
    CODECHAL_KERNEL_HEADER m_mpeg2BrcBlockCopy;
};

enum BindingTableOffsetMeG9
{
    meMvDataSurface                 = 0,
    meDistortionSurface             = 2,
    meBrcDistortion                 = 3,
    meCurrForFwdRef                 = 5,
    meFwdRefIdx0                    = 6,
    meCurrForBwdRef                 = 22,
    meBwdRefIdx0                    = 23,
    meNumSurface                    = 27,
};

enum BindingTableOffsetMbEncG9
{
    mbEncPakObj                     = 0,
    mbEncPakObjPrev                 = 1,
    mbEncCurrentY                   = 3,
    mbEncBrcDistortionSurface       = 8,
    mbEncCurrentPic                 = 9,
    mbEncForwardPic                 = 10,
    mbEncBackwardPic                = 11,
    mbEncInterlaceFrameCurrentPic   = 14,
    mbEncInterlaceFrameBackwardPic  = 15,
    mbEncMbControl                  = 18,
    mbEncNumBindingTableEntries     = 19
};

class MeCurbeG9
{
public:
    struct CurbeData
    {
        // DW0
        union
        {
            struct
            {
                uint32_t m_skipModeEn                     : MOS_BITFIELD_BIT(0);
                uint32_t m_adaptiveEn                     : MOS_BITFIELD_BIT(1);
                uint32_t m_biMixDis                       : MOS_BITFIELD_BIT(2);
                uint32_t m_reserved3                      : MOS_BITFIELD_RANGE(3, 4);
                uint32_t m_earlyImeSuccessEn              : MOS_BITFIELD_BIT(5);
                uint32_t m_reserved6                      : MOS_BITFIELD_BIT(6);
                uint32_t m_t8x8FlagForInterEn             : MOS_BITFIELD_BIT(7);
                uint32_t m_reserved8                      : MOS_BITFIELD_RANGE(8, 23);
                uint32_t m_earlyImeStop                   : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW0;

        // DW1
        union
        {
            struct
            {
                uint32_t m_maxNumMVs                      : MOS_BITFIELD_RANGE(0, 5);
                uint32_t m_reserved6                      : MOS_BITFIELD_RANGE(6, 15);
                uint32_t m_biWeight                       : MOS_BITFIELD_RANGE(16, 21);
                uint32_t m_reserved22                     : MOS_BITFIELD_RANGE(22, 27);
                uint32_t m_uniMixDisable                  : MOS_BITFIELD_BIT(28);
                uint32_t m_reserved29                     : MOS_BITFIELD_RANGE(29, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW1;

        // DW2
        union
        {
            struct
            {
                uint32_t m_maxLenSP                       : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_maxNumSU                       : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_reserved16                     : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW2;

        // DW3
        union
        {
            struct
            {
                uint32_t m_srcSize                        : MOS_BITFIELD_RANGE(0, 1);
                uint32_t m_reserved2                      : MOS_BITFIELD_RANGE(2, 3);
                uint32_t m_mbTypeRemap                    : MOS_BITFIELD_RANGE(4, 5);
                uint32_t m_srcAccess                      : MOS_BITFIELD_BIT(6);
                uint32_t m_refAccess                      : MOS_BITFIELD_BIT(7);
                uint32_t m_searchCtrl                     : MOS_BITFIELD_RANGE(8, 10);
                uint32_t m_dualSearchPathOption           : MOS_BITFIELD_BIT(11);
                uint32_t m_subPelMode                     : MOS_BITFIELD_RANGE(12, 13);
                uint32_t m_skipType                       : MOS_BITFIELD_BIT(14);
                uint32_t m_disableFieldCacheAlloc         : MOS_BITFIELD_BIT(15);
                uint32_t m_interChromaMode                : MOS_BITFIELD_BIT(16);
                uint32_t m_ftEnable                       : MOS_BITFIELD_BIT(17);
                uint32_t m_bmeDisableFBR                  : MOS_BITFIELD_BIT(18);
                uint32_t m_blockBasedSkipEnable           : MOS_BITFIELD_BIT(19);
                uint32_t m_interSAD                       : MOS_BITFIELD_RANGE(20, 21);
                uint32_t m_intraSAD                       : MOS_BITFIELD_RANGE(22, 23);
                uint32_t m_subMbPartMask                  : MOS_BITFIELD_RANGE(24, 30);
                uint32_t m_reserved31                     : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW3;

        // DW4
        union
        {
            struct
            {
                uint32_t m_reserved0                      : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_pictureHeightMinus1            : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_pictureWidth                   : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_reserved24                     : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW4;

        // DW5
        union
        {
            struct
            {
                uint32_t m_reserved0                      : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_qpPrimeY                       : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_refWidth                       : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_refHeight                      : MOS_BITFIELD_RANGE(24, 31);

            };
            struct
            {
                uint32_t m_value;
            };
        } DW5;

        // DW6
        union
        {
            struct
            {
                uint32_t m_reserved0                      : MOS_BITFIELD_RANGE(0, 2);
                uint32_t m_meModes                        : MOS_BITFIELD_RANGE(3, 4);
                uint32_t m_reserved5                      : MOS_BITFIELD_RANGE(5, 7);
                uint32_t m_superCombineDist               : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_maxVmvR                        : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW6;

        // DW7
        union
        {
            struct
            {
                uint32_t m_reserved0                      : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_mvCostScaleFactor              : MOS_BITFIELD_RANGE(16, 17);
                uint32_t m_bilinearEnable                 : MOS_BITFIELD_BIT(18);
                uint32_t m_srcFieldPolarity               : MOS_BITFIELD_BIT(19);
                uint32_t m_weightedSADHAAR                : MOS_BITFIELD_BIT(20);
                uint32_t m_acOnlyHAAR                     : MOS_BITFIELD_BIT(21);
                uint32_t m_refIDCostMode                  : MOS_BITFIELD_BIT(22);
                uint32_t m_reserved23                     : MOS_BITFIELD_BIT(23);
                uint32_t m_skipCenterMask                 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW7;

        // DW8
        union
        {
            struct
            {
                uint32_t m_mode0Cost                      : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_mode1Cost                      : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_mode2Cost                      : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_mode3Cost                      : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW8;

        // DW9
        union
        {
            struct
            {
                uint32_t m_mode4Cost                      : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_mode5Cost                      : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_mode6Cost                      : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_mode7Cost                      : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW9;

        // DW10
        union
        {
            struct
            {
                uint32_t m_mode8Cost                      : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_mode9Cost                      : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_refIDCost                      : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_chromaIntraModeCost            : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW10;

        // DW11
        union
        {
            struct
            {
                uint32_t m_mv0Cost                        : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_mv1Cost                        : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_mv2Cost                        : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_mv3Cost                        : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW11;

        // DW12
        union
        {
            struct
            {
                uint32_t m_mv4Cost                        : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_mv5Cost                        : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_mv6Cost                        : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_mv7Cost                        : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW12;

        // DW13
        union
        {
            struct
            {
                uint32_t m_numRefIdxL0MinusOne            : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_numRefIdxL1MinusOne            : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_actualMBWidth                  : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_actualMBHeight                 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW13;

        // DW14
        union
        {
            struct
            {
                uint32_t m_list0RefID0FieldParity         : MOS_BITFIELD_BIT(0);
                uint32_t m_list0RefID1FieldParity         : MOS_BITFIELD_BIT(1);
                uint32_t m_list0RefID2FieldParity         : MOS_BITFIELD_BIT(2);
                uint32_t m_list0RefID3FieldParity         : MOS_BITFIELD_BIT(3);
                uint32_t m_list0RefID4FieldParity         : MOS_BITFIELD_BIT(4);
                uint32_t m_list0RefID5FieldParity         : MOS_BITFIELD_BIT(5);
                uint32_t m_list0RefID6FieldParity         : MOS_BITFIELD_BIT(6);
                uint32_t m_list0RefID7FieldParity         : MOS_BITFIELD_BIT(7);
                uint32_t m_list1RefID0FieldParity         : MOS_BITFIELD_BIT(8);
                uint32_t m_list1RefID1FieldParity         : MOS_BITFIELD_BIT(9);
                uint32_t m_reserved10                     : MOS_BITFIELD_RANGE(10, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW14;

        // DW15
        union
        {
            struct
            {
                uint32_t m_prevMvReadPosFactor            : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_mvShiftFactor                  : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_reserved16                     : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW15;

        struct
        {
            // DW16
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_0;
                    SearchPathDelta   m_spDelta_1;
                    SearchPathDelta   m_spDelta_2;
                    SearchPathDelta   m_spDelta_3;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW16;

            // DW17
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_4;
                    SearchPathDelta   m_spDelta_5;
                    SearchPathDelta   m_spDelta_6;
                    SearchPathDelta   m_spDelta_7;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW17;

            // DW18
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_8;
                    SearchPathDelta   m_spDelta_9;
                    SearchPathDelta   m_spDelta_10;
                    SearchPathDelta   m_spDelta_11;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW18;

            // DW19
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_12;
                    SearchPathDelta   m_spDelta_13;
                    SearchPathDelta   m_spDelta_14;
                    SearchPathDelta   m_spDelta_15;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW19;

            // DW20
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_16;
                    SearchPathDelta   m_spDelta_17;
                    SearchPathDelta   m_spDelta_18;
                    SearchPathDelta   m_spDelta_19;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW20;

            // DW21
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_20;
                    SearchPathDelta   m_spDelta_21;
                    SearchPathDelta   m_spDelta_22;
                    SearchPathDelta   m_spDelta_23;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW21;

            // DW22
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_24;
                    SearchPathDelta   m_spDelta_25;
                    SearchPathDelta   m_spDelta_26;
                    SearchPathDelta   m_spDelta_27;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW22;

            // DW23
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_28;
                    SearchPathDelta   m_spDelta_29;
                    SearchPathDelta   m_spDelta_30;
                    SearchPathDelta   m_spDelta_31;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW23;

            // DW24
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_32;
                    SearchPathDelta   m_spDelta_33;
                    SearchPathDelta   m_spDelta_34;
                    SearchPathDelta   m_spDelta_35;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW24;

            // DW25
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_36;
                    SearchPathDelta   m_spDelta_37;
                    SearchPathDelta   m_spDelta_38;
                    SearchPathDelta   m_spDelta_39;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW25;

            // DW26
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_40;
                    SearchPathDelta   m_spDelta_41;
                    SearchPathDelta   m_spDelta_42;
                    SearchPathDelta   m_spDelta_43;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW26;

            // DW27
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_44;
                    SearchPathDelta   m_spDelta_45;
                    SearchPathDelta   m_spDelta_46;
                    SearchPathDelta   m_spDelta_47;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW27;

            // DW28
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_48;
                    SearchPathDelta   m_spDelta_49;
                    SearchPathDelta   m_spDelta_50;
                    SearchPathDelta   m_spDelta_51;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW28;

            // DW29
            union
            {
                struct
                {
                    SearchPathDelta   m_spDelta_52;
                    SearchPathDelta   m_spDelta_53;
                    SearchPathDelta   m_spDelta_54;
                    SearchPathDelta   m_spDelta_55;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW29;
        } SpDelta;

        // DW30
        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW30;

        // DW31
        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW31;

        // DW32
        union
        {
            struct
            {
                uint32_t m_4xMeMvOutputDataSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW32;

        // DW33
        union
        {
            struct
            {
                uint32_t m_16xMeMvInputDataSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW33;

        // DW34
        union
        {
            struct
            {
                uint32_t m_4xMeOutputDistSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW34;

        // DW35
        union
        {
            struct
            {
                uint32_t m_4xMeOutputBrcDistSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW35;

        // DW36
        union
        {
            struct
            {
                uint32_t m_vmeFwdInterPredictionSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW36;

        // DW37
        union
        {
            struct
            {
                uint32_t m_vmeBwdInterPredictionSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW37;

        // DW38
        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW38;
    }m_curbeData;

    //!
    //! \brief    Constructor
    //!
    MeCurbeG9();

    //!
    //! \brief    Destructor
    //!
    ~MeCurbeG9(){};

    static const size_t m_byteSize = sizeof(CurbeData);

} ;

class MbEncCurbeG9
{
public:
    struct CurbeData
    {
        union
        {
            struct
            {
                uint32_t m_skipModeEn                         : MOS_BITFIELD_BIT(0);
                uint32_t m_adaptiveEn                         : MOS_BITFIELD_BIT(1);
                uint32_t m_biMixDis                           : MOS_BITFIELD_BIT(2);
                uint32_t m_isInterlacedFrameFlag              : MOS_BITFIELD_BIT(3);
                uint32_t m_isTopFieldFirst                    : MOS_BITFIELD_BIT(4);
                uint32_t m_earlyImeSuccessEn                  : MOS_BITFIELD_BIT(5);
                uint32_t m_forceToSkip                        : MOS_BITFIELD_BIT(6);
                uint32_t m_t8x8FlagForInterEn                 : MOS_BITFIELD_BIT(7);
                uint32_t m_refPixOff                          : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_reserved1                          : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_earlyImeStop                       : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW0;

        union
        {
            struct
            {
                uint32_t m_maxNumMVs                          : MOS_BITFIELD_RANGE(0,5);
                uint32_t m_reserved0                          : MOS_BITFIELD_RANGE(6,7);
                uint32_t m_refIDPolBits                       : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_biWeight                           : MOS_BITFIELD_RANGE(16,21);
                uint32_t m_gxMask                             : MOS_BITFIELD_BIT(22);
                uint32_t m_gyMask                             : MOS_BITFIELD_BIT(23);
                uint32_t m_refPixShift                        : MOS_BITFIELD_RANGE(24,27);
                uint32_t m_uniMixDisable                      : MOS_BITFIELD_BIT(28);
                uint32_t m_refPixBiasEn                       : MOS_BITFIELD_BIT(29); // m_refPixBiasEn (MBZ)
                uint32_t m_idmShapeModeExt7x7                 : MOS_BITFIELD_BIT(30);
                uint32_t m_idmShapeModeExt5x5                 : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW1;

        union
        {
            struct
            {
                uint32_t m_maxLenSP                           : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_maxNumSU                           : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_start0X                            : MOS_BITFIELD_RANGE(16,19);
                uint32_t m_start0Y                            : MOS_BITFIELD_RANGE(20,23);
                uint32_t m_start1X                            : MOS_BITFIELD_RANGE(24,27);
                uint32_t m_start1Y                            : MOS_BITFIELD_RANGE(28,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW2;

        union
        {
            struct
            {
                uint32_t m_fieldBias                          : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_oppFieldBias                       : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_fieldSkipThr                       : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW3;

        union
        {
            struct
            {
                uint32_t m_reserved0                          : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_picHeightMinus1                    : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_picWidth                           : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_reserved24                         : MOS_BITFIELD_BIT(24); // WalkerType
                uint32_t m_motionSeeding                      : MOS_BITFIELD_BIT(25);
                uint32_t m_kernelMBModeDecision               : MOS_BITFIELD_BIT(26);
                uint32_t m_iFrameMBDistortionDumpEnable       : MOS_BITFIELD_BIT(27);
                uint32_t m_fieldFlag                          : MOS_BITFIELD_BIT(28);
                uint32_t m_pictureType                        : MOS_BITFIELD_RANGE(29,30);
                uint32_t m_reserved1                          : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW4;

        struct
        {
            union
            {
                struct
                {
                    uint32_t m_mv0Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_mv1Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_mv2Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_mv3Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW5;

            union
            {
                struct
                {
                    uint32_t m_mv4Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_mv5Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_mv6Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_mv7Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW6;
        } MvCost;

        union
        {
            struct
            {
                uint32_t m_intraPartMask                      : MOS_BITFIELD_RANGE(0,4);
                uint32_t m_nonSkipZMvAdded                    : MOS_BITFIELD_BIT(5);
                uint32_t m_nonSkipModeAdded                   : MOS_BITFIELD_BIT(6);
                uint32_t m_reserved7                          : MOS_BITFIELD_BIT(7);
                uint32_t m_reserved8                          : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_mvCostScaleFactor                  : MOS_BITFIELD_RANGE(16,17);
                uint32_t m_bilinearEnable                     : MOS_BITFIELD_BIT(18);
                uint32_t m_srcFieldPolarity                   : MOS_BITFIELD_BIT(19);
                uint32_t m_weightedSADHAAR                    : MOS_BITFIELD_BIT(20);
                uint32_t m_acOnlyHAAR                         : MOS_BITFIELD_BIT(21);
                uint32_t m_refIDCostMode                      : MOS_BITFIELD_BIT(22);
                uint32_t m_idmShapeMode                       : MOS_BITFIELD_BIT(23);
                uint32_t m_skipCenterMask                     : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW7;

        union
        {
            struct
            {
                uint32_t m_mode8Cost                          : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_mode9Cost                          : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_refIDCost                          : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_chromaIntraModeCost                : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW8;

        union
        {
            struct
            {
                uint32_t m_srcSize                            : MOS_BITFIELD_RANGE(0,1);
                uint32_t m_mbQPEnable                         : MOS_BITFIELD_BIT(2);
                uint32_t m_mbSkipEnable                       : MOS_BITFIELD_BIT(3);
                uint32_t m_mbNonSkipEnable                    : MOS_BITFIELD_BIT(4);
                uint32_t m_reserved5                          : MOS_BITFIELD_BIT(5);
                uint32_t m_srcAccess                          : MOS_BITFIELD_BIT(6);
                uint32_t m_refAccess                          : MOS_BITFIELD_BIT(7);
                uint32_t m_searchCtrl                         : MOS_BITFIELD_RANGE(8,10);
                uint32_t m_dualSearchOpt                      : MOS_BITFIELD_BIT(11);
                uint32_t m_subPelMode                         : MOS_BITFIELD_RANGE(12,13);
                uint32_t m_skipType                           : MOS_BITFIELD_BIT(14);
                uint32_t m_fieldCacheAllocationDis            : MOS_BITFIELD_BIT(15);
                uint32_t m_interChromaMode                    : MOS_BITFIELD_BIT(16);
                uint32_t m_ftEnable                           : MOS_BITFIELD_BIT(17);
                uint32_t m_bmeDisableFBR                      : MOS_BITFIELD_BIT(18);
                uint32_t m_reserved19                         : MOS_BITFIELD_BIT(19);
                uint32_t m_interSAD                           : MOS_BITFIELD_RANGE(20,21);
                uint32_t m_intraSAD                           : MOS_BITFIELD_RANGE(22,23);
                uint32_t m_subMbPartMask                      : MOS_BITFIELD_RANGE(24,30);
                uint32_t m_reserved31                         : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW9;

        union
        {
            struct
            {
                uint32_t m_dispatchID                         : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_largeMbSizeInWord                  : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_refWidth                           : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_refHeight                          : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW10;

        union
        {
            struct
            {
                uint32_t m_qpScaleCode                        : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_intraFactor                        : MOS_BITFIELD_RANGE(8,11);
                uint32_t m_intraMulFact                       : MOS_BITFIELD_RANGE(12,15);
                uint32_t m_intraBiasFF                        : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_intraBiasFrame                     : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW11;

        union
        {
            struct
            {
                uint32_t m_isFastMode                         : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_smallMbSizeInWord                  : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_distScaleFactor                    : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW12;

        union
        {
            struct
            {
                uint32_t m_panicModeMBThreshold               : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_targetSizeInWord                   : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_reserved14                         : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW13;

        union
        {
            struct
            {
                uint32_t m_forwardHorizontalSearchRange       : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_forwardVerticalSearchRange         : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t   m_value;
            };
        } DW14;

        union
        {
            struct
            {
                uint32_t m_backwardHorizontalSearchRange      : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_backwardVerticalSearchRange        : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW15;

        struct
        {
            union
            {
                struct
                {
                    uint32_t m_meDelta0to3;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW16;

            union
            {
                struct
                {
                    uint32_t m_meDelta4to7;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW17;

            union
            {
                struct
                {
                    uint32_t m_meDelta8to11;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW18;

            union
            {
                struct
                {
                    uint32_t m_meDelta12to15;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW19;

            union
            {
                struct
                {
                    uint32_t m_meDelta16to19;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW20;

            union
            {
                struct
                {
                    uint32_t m_meDelta20to23;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW21;

            union
            {
                struct
                {
                    uint32_t m_meDelta24to27;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW22;

            union
            {
                struct
                {
                    uint32_t m_meDelta28to31;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW23;

            union
            {
                struct
                {
                    uint32_t m_meDelta32to35;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW24;

            union
            {
                struct
                {
                    uint32_t m_meDelta36to39;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW25;

            union
            {
                struct
                {
                    uint32_t m_meDelta40to43;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW26;

            union
            {
                struct
                {
                    uint32_t m_meDelta44to47;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW27;

            union
            {
                struct
                {
                    uint32_t m_meDelta48to51;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW28;

            union
            {
                struct
                {
                    uint32_t m_meDelta52to55;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW29;

            union
            {
                struct
                {
                    uint32_t m_mode0Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_mode1Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_mode2Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_mode3Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW30;

            union
            {
                struct
                {
                    uint32_t m_mode4Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_mode5Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_mode6Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_mode7Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW31;
        } VmeSPath0;

        struct
        {
            union
            {
                struct
                {
                    uint32_t m_meDelta0to3;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW32;

            union
            {
                struct
                {
                    uint32_t m_meDelta4to7;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW33;

            union
            {
                struct
                {
                    uint32_t m_meDelta8to11;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW34;

            union
            {
                struct
                {
                    uint32_t m_meDelta12to15;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW35;

            union
            {
                struct
                {
                    uint32_t m_meDelta16to19;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW36;

            union
            {
                struct
                {
                    uint32_t m_meDelta20to23;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW37;

            union
            {
                struct
                {
                    uint32_t m_meDelta24to27;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW38;

            union
            {
                struct
                {
                    uint32_t m_meDelta28to31;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW39;

            union
            {
                struct
                {
                    uint32_t m_meDelta32to35;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW40;

            union
            {
                struct
                {
                    uint32_t m_meDelta36to39;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW41;

            union
            {
                struct
                {
                    uint32_t m_meDelta40to43;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW42;

            union
            {
                struct
                {
                    uint32_t m_meDelta44to47;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW43;

            union
            {
                struct
                {
                    uint32_t m_meDelta48to51;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW44;

            union
            {
                struct
                {
                    uint32_t m_meDelta52to55;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW45;

            struct
            {
                union
                {
                    struct
                    {
                        uint32_t m_mv0Cost_Interlaced : MOS_BITFIELD_RANGE(0, 7);
                        uint32_t m_mv1Cost_Interlaced : MOS_BITFIELD_RANGE(8, 15);
                        uint32_t m_mv2Cost_Interlaced : MOS_BITFIELD_RANGE(16, 23);
                        uint32_t m_mv3Cost_Interlaced : MOS_BITFIELD_RANGE(24, 31);
                    };
                    struct
                    {
                        uint32_t m_value;
                    };
                } DW46;

                union
                {
                    struct
                    {
                        uint32_t m_mv4Cost_Interlaced : MOS_BITFIELD_RANGE(0, 7);
                        uint32_t m_mv5Cost_Interlaced : MOS_BITFIELD_RANGE(8, 15);
                        uint32_t m_mv6Cost_Interlaced : MOS_BITFIELD_RANGE(16, 23);
                        uint32_t m_mv7Cost_Interlaced : MOS_BITFIELD_RANGE(24, 31);
                    };
                    struct
                    {
                        uint32_t m_value;
                    };
                } DW47;
            } MvCostInterlace;
        } VmeSPath1;

        union
        {
            struct
            {
                uint32_t m_batchBufferEnd;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW48;

        union
        {
            struct
            {
                uint32_t m_pakObjCmds;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW49;

        union
        {
            struct
            {
                uint32_t m_prevmPakObjCmds;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW50;

        union
        {
            struct
            {
                uint32_t m_currPicY;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW51;

        union
        {
            struct
            {
                uint32_t m_currFwdBwdRef;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW52;

        union
        {
            struct
            {
                uint32_t m_currBwdRef;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW53;

        union
        {
            struct
            {
                uint32_t m_distSurf4x;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW54;

        union
        {
            struct
            {
                uint32_t m_mbControl;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW55;
    }m_curbeData;

    //!
    //! \brief    Constructor
    //!
    MbEncCurbeG9(uint8_t codingType);

    //!
    //! \brief    Destructor
    //!
    ~MbEncCurbeG9(){};

    static const size_t m_byteSize = sizeof(CurbeData);
};

MeCurbeG9::MeCurbeG9()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_curbeData, m_byteSize);

    m_curbeData.DW0.m_value  = 0x00000000;
    m_curbeData.DW1.m_value  = 0x00200010;
    m_curbeData.DW2.m_value  = 0x00003939;
    m_curbeData.DW3.m_value  = 0x77a43000;
    m_curbeData.DW4.m_value  = 0x00000000;
    m_curbeData.DW5.m_value  = 0x28300000;

    m_curbeData.DW32.m_value = 0xffffffff;
    m_curbeData.DW33.m_value = 0xffffffff;
    m_curbeData.DW34.m_value = 0xffffffff;
    m_curbeData.DW35.m_value = 0xffffffff;
    m_curbeData.DW36.m_value = 0xffffffff;
    m_curbeData.DW37.m_value = 0xffffffff;
    m_curbeData.DW38.m_value = 0xffffffff;

}

MbEncCurbeG9::MbEncCurbeG9(uint8_t codingType)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_curbeData, m_byteSize);

    m_curbeData.DW0.m_value  = 0x00000023;
    switch (codingType)
    {
        case I_TYPE:
            m_curbeData.DW1.m_value  = 0x00200020;
            m_curbeData.DW2.m_value  = 0x00000000;
            m_curbeData.DW4.m_value  = 0x00000000;
            m_curbeData.DW7.m_value  = 0x00050066;
            m_curbeData.DW8.m_value  = 0x00000000;
            m_curbeData.DW9.m_value  = 0x7EA41000;
            m_curbeData.DW10.m_value = 0x0000FF00;
            break;
        case P_TYPE:
            m_curbeData.DW1.m_value  = 0x00200020;
            m_curbeData.DW2.m_value  = 0x00001009;
            m_curbeData.DW4.m_value  = 0x20000000;
            m_curbeData.DW7.m_value  = 0x00050066;
            m_curbeData.DW8.m_value  = 0x00000041;
            m_curbeData.DW9.m_value  = 0x7EA41000;
            m_curbeData.DW10.m_value = 0x2830FF00;
            break;
        case B_TYPE:
        default:
            m_curbeData.DW1.m_value  = 0x10200010;
            m_curbeData.DW2.m_value  = 0x00001005;
            m_curbeData.DW4.m_value  = 0x40000000;
            m_curbeData.DW7.m_value  = 0xFF050066;
            m_curbeData.DW8.m_value  = 0x00000041;
            m_curbeData.DW9.m_value  = 0x7EA01000;
            m_curbeData.DW10.m_value = 0x2020FF00;
            break;

    }

    m_curbeData.DW3.m_value         = 0xFE0C0000;
    m_curbeData.MvCost.DW5.m_value  = 0x00000000;
    m_curbeData.MvCost.DW6.m_value  = 0x00000000;
    m_curbeData.DW11.m_value        = 0x5A325300;
    m_curbeData.DW12.m_value        = 0x0000FF00;
    m_curbeData.DW13.m_value        = 0x00FF0000;

}

CodechalEncodeMpeg2G9::CodechalEncodeMpeg2G9(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncodeMpeg2(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    pfnGetKernelHeaderAndSize = GetKernelHeaderAndSize;

    MOS_STATUS eStatus = CodecHalGetKernelBinaryAndSize(
        (uint8_t *)IGCODECKRN_G9,
        m_kuid,
        &m_kernelBinary,
        &m_combinedKernelSize);

    CODECHAL_ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    m_hwInterface->GetStateHeapSettings()->dwIshSize +=
        MOS_ALIGN_CEIL(m_combinedKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));

    m_needCheckCpEnabled = true;
}

MOS_STATUS CodechalEncodeMpeg2G9::Initialize(CodechalSetting * codecHalSettings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeMpeg2::Initialize(codecHalSettings));

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G9::GetKernelHeaderAndSize(
    void                           *binary,
    EncOperation                   operation,
    uint32_t                       krnStateIdx,
    void                           *krnHeader,
    uint32_t                       *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    auto kernelHeaderTable = (KernelHeader *)binary;
    PCODECHAL_KERNEL_HEADER currKrnHeader;

    if (operation == ENC_SCALING4X)
    {
        currKrnHeader = &kernelHeaderTable->m_plyDscalePly;
    }
    else if (operation == ENC_ME)
    {
        currKrnHeader = &kernelHeaderTable->m_mpeg2AvcMeP;
    }
    else if (operation == ENC_BRC)
    {
        currKrnHeader = &kernelHeaderTable->m_mpeg2InitFrameBrc;
    }
    else if (operation == ENC_MBENC)
    {
        currKrnHeader = &kernelHeaderTable->m_mpeg2MbEncI;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    PCODECHAL_KERNEL_HEADER invalidEntry = &(kernelHeaderTable->m_mpeg2BrcResetFrame) + 1;
    PCODECHAL_KERNEL_HEADER nextKrnHeader = (currKrnHeader + 1);
    uint32_t nextKrnOffset = *krnSize;

    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G9::InitKernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Init kernel state
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMe());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMbEnc());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateBrc());

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G9::InitKernelStateMe()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    for (uint8_t krnStateIdx = 0; krnStateIdx < 2; krnStateIdx++)
    {
        auto kernelStatePtr = &m_meKernelStates[krnStateIdx];
        auto kernelSize = m_combinedKernelSize;

        CODECHAL_KERNEL_HEADER currKrnHeader;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            m_kernelBinary,
            ENC_ME,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount = meNumSurface;
        kernelStatePtr->KernelParams.iThreadCount = m_hwInterface->GetRenderInterface()->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = MeCurbeG9::m_byteSize;
        kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount = 1;

        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));
    }

    // Until a better way can be found, maintain old binding table structures
    auto meBindingTable = &m_meBindingTable;

    // Mpeg2 uses AVC ME Kernel for P/B distortion calculation
    meBindingTable->dwMEMVDataSurface    = meMvDataSurface;
    meBindingTable->dwMECurrForFwdRef    = meCurrForFwdRef;
    meBindingTable->dwMECurrForBwdRef    = meCurrForBwdRef;
    meBindingTable->dwMEDist             = meDistortionSurface;
    meBindingTable->dwMEBRCDist          = meBrcDistortion;
    meBindingTable->dwMEFwdRefPicIdx[0]  = meFwdRefIdx0;
    meBindingTable->dwMEFwdRefPicIdx[1]  = CODECHAL_INVALID_BINDING_TABLE_IDX;
    meBindingTable->dwMEFwdRefPicIdx[2]  = CODECHAL_INVALID_BINDING_TABLE_IDX;
    meBindingTable->dwMEFwdRefPicIdx[3]  = CODECHAL_INVALID_BINDING_TABLE_IDX;
    meBindingTable->dwMEFwdRefPicIdx[4]  = CODECHAL_INVALID_BINDING_TABLE_IDX;
    meBindingTable->dwMEFwdRefPicIdx[5]  = CODECHAL_INVALID_BINDING_TABLE_IDX;
    meBindingTable->dwMEFwdRefPicIdx[6]  = CODECHAL_INVALID_BINDING_TABLE_IDX;
    meBindingTable->dwMEFwdRefPicIdx[7]  = CODECHAL_INVALID_BINDING_TABLE_IDX;
    meBindingTable->dwMEBwdRefPicIdx[0]  = meBwdRefIdx0;
    meBindingTable->dwMEBwdRefPicIdx[1]  = CODECHAL_INVALID_BINDING_TABLE_IDX;

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G9::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    for (uint8_t krnStateIdx = 0; krnStateIdx < mbEncKernelIdxNum; krnStateIdx++)
    {
        auto kernelStatePtr = &m_mbEncKernelStates[krnStateIdx];
        auto kernelSize = m_combinedKernelSize;

        CODECHAL_KERNEL_HEADER currKrnHeader;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            m_kernelBinary,
            ENC_MBENC,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount = mbEncNumBindingTableEntries;
        kernelStatePtr->KernelParams.iThreadCount = m_hwInterface->GetRenderInterface()->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = MbEncCurbeG9::m_byteSize;
        kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount = 1;

        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));
    }

    m_mbEncBindingTable.m_mbEncPakObj                     = mbEncPakObj;
    m_mbEncBindingTable.m_mbEncPakObjPrev                 = mbEncPakObjPrev;
    m_mbEncBindingTable.m_mbEncCurrentY                   = mbEncCurrentY;
    m_mbEncBindingTable.m_mbEncBrcDistortionSurface       = mbEncBrcDistortionSurface;
    m_mbEncBindingTable.m_mbEncCurrentPic                 = mbEncCurrentPic;
    m_mbEncBindingTable.m_mbEncForwardPic                 = mbEncForwardPic;
    m_mbEncBindingTable.m_mbEncBackwardPic                = mbEncBackwardPic;
    m_mbEncBindingTable.m_mbEncInterlaceFrameCurrentPic   = mbEncInterlaceFrameCurrentPic;
    m_mbEncBindingTable.m_mbEncInterlaceFrameBackwardPic  = mbEncInterlaceFrameBackwardPic;
    m_mbEncBindingTable.m_mbEncMbControl                  = mbEncMbControl;

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G9::SetCurbeMe()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MeCurbeG9 cmd;

    // r1
    cmd.m_curbeData.DW3.m_subPelMode = 3;
    if (m_fieldScalingOutputInterleaved)
    {
        cmd.m_curbeData.DW3.m_srcAccess = cmd.m_curbeData.DW3.m_refAccess =
            CodecHal_PictureIsField(m_picParams->m_currOriginalPic) ? 1 : 0;
        cmd.m_curbeData.DW7.m_srcFieldPolarity =
            CodecHal_PictureIsBottomField(m_picParams->m_currOriginalPic) ? 1 : 0;
    }

    uint32_t scaleFactor = 4; // Scale factor always 4x, SHME/UHME not supported
    cmd.m_curbeData.DW4.m_pictureHeightMinus1 =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scaleFactor) - 1;
    cmd.m_curbeData.DW4.m_pictureWidth =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / scaleFactor);
    cmd.m_curbeData.DW5.m_qpPrimeY = m_mvCostTableOffset;
    cmd.m_curbeData.DW6.m_meModes = CODECHAL_ENCODE_ME4X_ONLY;
    cmd.m_curbeData.DW6.m_superCombineDist = m_superCombineDistGeneric[m_seqParams->m_targetUsage];
    cmd.m_curbeData.DW6.m_maxVmvR = m_maxVmvr;

    if (!CodecHal_PictureIsFrame(m_picParams->m_currOriginalPic))
    {
        cmd.m_curbeData.DW6.m_maxVmvR = cmd.m_curbeData.DW6.m_maxVmvR >> 1;
    }

    if (m_pictureCodingType == B_TYPE)
    {
        // This field is irrelevant since we are not using the bi-direct search.
        // set it to 32
        cmd.m_curbeData.DW1.m_biWeight = 32;
        cmd.m_curbeData.DW13.m_numRefIdxL1MinusOne = 0;   // always 0 for MPEG2
    }

    if (m_pictureCodingType == P_TYPE ||
        m_pictureCodingType == B_TYPE)
    {
        cmd.m_curbeData.DW13.m_numRefIdxL0MinusOne = 0;   // always 0 for MPEG2
    }

    // r3 & r4
    uint8_t meMethod = (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[m_seqParams->m_targetUsage] : m_meMethodGeneric[m_seqParams->m_targetUsage];
    uint8_t tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &(cmd.m_curbeData.SpDelta), 14 * sizeof(uint32_t),
        CodechalEncoderState::m_encodeSearchPath[tableIdx][meMethod],
        14 * sizeof(uint32_t)));

    // r5
    cmd.m_curbeData.DW32.m_4xMeMvOutputDataSurfIndex = m_meBindingTable.dwMEMVDataSurface ;
    cmd.m_curbeData.DW34.m_4xMeOutputDistSurfIndex = m_meBindingTable.dwMEDist;
    cmd.m_curbeData.DW35.m_4xMeOutputBrcDistSurfIndex = m_meBindingTable.dwMEBRCDist;
    cmd.m_curbeData.DW36.m_vmeFwdInterPredictionSurfIndex = m_meBindingTable.dwMECurrForFwdRef;
    cmd.m_curbeData.DW37.m_vmeBwdInterPredictionSurfIndex = m_meBindingTable.dwMECurrForBwdRef;

    uint32_t krnStateIdx =
        (m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_ME_IDX_P : CODECHAL_ENCODE_ME_IDX_B;

    if (m_pictureCodingType == B_TYPE && CodecHal_PictureIsInvalid(m_picParams->m_refFrameList[1]))
    {
        krnStateIdx = CODECHAL_ENCODE_ME_IDX_P;
    }
    auto kernelState = &m_meKernelStates[krnStateIdx];

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &cmd,
        kernelState->dwCurbeOffset,
        cmd.m_byteSize));
    return eStatus;

}

MOS_STATUS CodechalEncodeMpeg2G9::SendMeSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    auto meBindingTable = &m_meBindingTable;
    PMOS_SURFACE currScaledSurface = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
    PMOS_SURFACE meMvDataBuffer = &m_4xMEMVDataBuffer;

    // Reference height and width information should be taken from the current scaled surface rather
    // than from the reference scaled surface in the case of PAFF.
    auto bufWidth = MOS_ALIGN_CEIL(m_downscaledWidthInMb4x * 32, 64);
    auto bufHeight =
        m_downscaledHeightInMb4x * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER;

    // Force the values
    meMvDataBuffer->dwWidth = bufWidth;
    meMvDataBuffer->dwHeight = bufHeight;
    meMvDataBuffer->dwPitch = bufWidth;

    uint32_t krnStateIdx =
        (m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_ME_IDX_P : CODECHAL_ENCODE_ME_IDX_B;

    if (m_pictureCodingType == B_TYPE && CodecHal_PictureIsInvalid(m_picParams->m_refFrameList[1]))
    {
        krnStateIdx = CODECHAL_ENCODE_ME_IDX_P;
    }
    auto kernelState = &m_meKernelStates[krnStateIdx];

    CODECHAL_SURFACE_CODEC_PARAMS   surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = meMvDataBuffer;
    surfaceParams.dwOffset = m_memvBottomFieldOffset;
    surfaceParams.dwCacheabilityControl =
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceParams.dwBindingTableOffset = meBindingTable->dwMEMVDataSurface;
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        kernelState));

    // Insert Distortion buffers only for 4xMe case
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = &m_brcBuffers.sMeBrcDistortionBuffer;
    surfaceParams.dwOffset = m_brcBuffers.dwMeBrcDistortionBottomFieldOffset;
    surfaceParams.dwBindingTableOffset = meBindingTable->dwMEBRCDist;
    surfaceParams.dwCacheabilityControl =
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        kernelState));

    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = &m_4xMEDistortionBuffer;
    surfaceParams.dwOffset = m_meDistortionBottomFieldOffset;
    surfaceParams.dwBindingTableOffset = meBindingTable->dwMEDist;
    surfaceParams.dwCacheabilityControl =
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        kernelState));

    bool currFieldPicture = CodecHal_PictureIsField(m_currOriginalPic) ? 1 : 0;
    bool currBottomField = CodecHal_PictureIsBottomField(m_currOriginalPic) ? 1 : 0;
    uint8_t currVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME :
        ((currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
    uint32_t refScaledBottomFieldOffset = 0;
    auto refScaledSurface = *currScaledSurface;

    // Setup references 1...n
    // LIST 0 references
    CODEC_PICTURE refPic = m_picParams->m_refFrameList[0];
    // no need to modify index if picture is invalid
    if (refPic.PicFlags != PICTURE_INVALID)
    {
        refPic.FrameIdx = 0;
    }

    if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
    {
        // Current Picture Y - VME
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bUseAdvState = true;
        surfaceParams.psSurface = currScaledSurface;
        surfaceParams.dwOffset = currBottomField ? (uint32_t)m_scaledBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = meBindingTable->dwMECurrForFwdRef;
        surfaceParams.ucVDirection = currVDirection;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            kernelState));

        bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
        uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
        uint8_t scaledIdx = m_refList[refPicIdx]->ucScalingIdx;
        // for 4xMe
        MOS_SURFACE* p4xSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
        if (p4xSurface != nullptr)
        {
            refScaledSurface.OsResource = p4xSurface->OsResource;
        }
        else
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
        }
        refScaledBottomFieldOffset = refBottomField ? (uint32_t)m_scaledBottomFieldOffset : 0;

        // L0 Reference Picture Y - VME
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bUseAdvState = true;
        surfaceParams.psSurface = &refScaledSurface;
        surfaceParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = meBindingTable->dwMEFwdRefPicIdx[0];
        surfaceParams.ucVDirection = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME :
            ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            kernelState));
    }

    // Setup references 1...n
    // LIST 1 references
    refPic = m_picParams->m_refFrameList[1];
    // no need to modify index if picture is invalid
    if (refPic.PicFlags != PICTURE_INVALID)
    {
        refPic.FrameIdx = 1;
    }
    if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
    {
        // Current Picture Y - VME
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bUseAdvState = true;
        surfaceParams.psSurface = currScaledSurface;
        surfaceParams.dwOffset = currBottomField ? (uint32_t)m_scaledBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = meBindingTable->dwMECurrForBwdRef;
        surfaceParams.ucVDirection = currVDirection;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            kernelState));

        bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
        uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
        uint8_t scaledIdx = m_refList[refPicIdx]->ucScalingIdx;

        // for 4xMe
        MOS_SURFACE* p4xSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
        if (p4xSurface != nullptr)
        {
            refScaledSurface.OsResource = p4xSurface->OsResource;
        }
        else
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
        }
        refScaledBottomFieldOffset = refBottomField ? (uint32_t)m_scaledBottomFieldOffset : 0;

        // L1 Reference Picture Y - VME
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bUseAdvState = true;
        surfaceParams.psSurface = &refScaledSurface;
        surfaceParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceParams.dwBindingTableOffset = meBindingTable->dwMEBwdRefPicIdx[0];
        surfaceParams.ucVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME :
            ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            kernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G9::SetCurbeMbEnc(
    bool mbEncIFrameDistEnabled,
    bool mbQpDataEnabled)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint16_t picWidthInMb = mbEncIFrameDistEnabled ?
            (uint16_t)m_downscaledWidthInMb4x : m_picWidthInMb;
    uint16_t fieldFrameHeightInMb   = mbEncIFrameDistEnabled ?
            (uint16_t)m_downscaledFrameFieldHeightInMb4x : m_frameFieldHeightInMb;
    uint8_t codingType = m_mbEncForcePictureCodingType ?
        m_mbEncForcePictureCodingType : (uint8_t)m_pictureCodingType;

    MbEncCurbeG9 cmd(codingType);

    // Set CURBE data as per
    cmd.m_curbeData.DW0.m_isInterlacedFrameFlag =
        (CodecHal_PictureIsFrame(m_picParams->m_currOriginalPic)) ? (m_picParams->m_fieldCodingFlag || m_picParams->m_fieldFrameCodingFlag) : 0;
    cmd.m_curbeData.DW0.m_isTopFieldFirst = cmd.m_curbeData.DW0.m_isInterlacedFrameFlag ? !m_picParams->m_interleavedFieldBFF : 0;
    cmd.m_curbeData.DW0.m_forceToSkip = 0;
    cmd.m_curbeData.DW4.m_pictureType = codingType;
    cmd.m_curbeData.DW4.m_fieldFlag = (CodecHal_PictureIsFrame(m_picParams->m_currOriginalPic)) ? 0 : 1;
    cmd.m_curbeData.DW4.m_picWidth = picWidthInMb;
    cmd.m_curbeData.DW4.m_picHeightMinus1 = fieldFrameHeightInMb - 1;

    cmd.m_curbeData.DW7.m_bilinearEnable = 1;
    cmd.m_curbeData.DW7.m_mvCostScaleFactor = 1; // half-pel
    cmd.m_curbeData.DW7.m_idmShapeMode = 0;

    cmd.m_curbeData.DW9.m_srcAccess = cmd.m_curbeData.DW9.m_refAccess = 0;
    cmd.m_curbeData.DW9.m_fieldCacheAllocationDis = 0;

    // This is approximation using first slice
    // MPEG2 quantiser_scale_code legal range is [1, 31] inclusive
    if (m_sliceParams->m_quantiserScaleCode < 1)
    {
        m_sliceParams->m_quantiserScaleCode = 1;
    }
    else if (m_sliceParams->m_quantiserScaleCode > 31)
    {
        m_sliceParams->m_quantiserScaleCode = 31;
    }

    cmd.m_curbeData.DW9.m_mbQPEnable = mbQpDataEnabled;
    cmd.m_curbeData.DW11.m_qpScaleCode = m_sliceParams->m_quantiserScaleCode;
    cmd.m_curbeData.DW2.m_maxNumSU = 0x10;
    cmd.m_curbeData.DW12.m_isFastMode = 0;

    if (m_seqParams ->m_rateControlMethod == RATECONTROL_CQP)
    {
        cmd.m_curbeData.DW13.m_panicModeMBThreshold = 0xFFFF;
    }

    cmd.m_curbeData.DW14.m_forwardHorizontalSearchRange = 4 << m_picParams->m_fcode00;
    cmd.m_curbeData.DW14.m_forwardVerticalSearchRange = 4 << m_picParams->m_fcode01;
    cmd.m_curbeData.DW15.m_backwardHorizontalSearchRange = 4 << m_picParams->m_fcode10;
    cmd.m_curbeData.DW15.m_backwardVerticalSearchRange = 4 << m_picParams->m_fcode11;

    if (codingType == I_TYPE)
    {
        cmd.m_curbeData.DW2.m_value = 0;
        cmd.m_curbeData.DW4.m_pictureType = 0;
        cmd.m_curbeData.DW9.m_intraSAD = 0x2;

        // Modify CURBE for distortion calculation.
        if (mbEncIFrameDistEnabled)
        {
            cmd.m_curbeData.DW4.m_iFrameMBDistortionDumpEnable = true;
        }
        else
        {
            // Make sure the distortion dump flag is disabled for the normal MBEnc case.
            // No need to reset the height and width since they are set
            // correctly above this if-else and not modified after.
            cmd.m_curbeData.DW4.m_iFrameMBDistortionDumpEnable = false;
        }
    }
    else if (codingType == P_TYPE)
    {
        cmd.m_curbeData.DW1.m_uniMixDisable = 0;
        cmd.m_curbeData.DW1.m_biWeight = 32;
        cmd.m_curbeData.DW2.m_maxLenSP = 0x09;
        cmd.m_curbeData.DW4.m_pictureType = 1;
        cmd.m_curbeData.DW9.m_interSAD = 2;
        cmd.m_curbeData.DW9.m_intraSAD = 2;
        cmd.m_curbeData.DW9.m_searchCtrl = 0;
        cmd.m_curbeData.DW9.m_subPelMode = 1;
        cmd.m_curbeData.DW9.m_skipType = 0;
        cmd.m_curbeData.DW9.m_subMbPartMask = 0x7E;
        cmd.m_curbeData.DW10.m_refWidth = 48;
        cmd.m_curbeData.DW10.m_refHeight = 40;
        cmd.m_curbeData.DW12.m_distScaleFactor = 0;
        cmd.m_curbeData.DW7.m_skipCenterMask = 0x55;

        //Motion vector cost is taken from VME_LUTXY
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.m_curbeData.MvCost),
            sizeof(uint32_t)* 2,
            m_vmeLutXyP,
            sizeof(uint32_t)* 2));

        //VME_SEARCH_PATH
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.m_curbeData.VmeSPath0),
            sizeof(uint32_t)* 16,
            m_vmeSPathP0,
            sizeof(uint32_t)* 16));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.m_curbeData.VmeSPath1),
            sizeof(uint32_t)* 16,
            m_vmeSPathP1,
            sizeof(uint32_t)* 16));

        //Interlaced motion vector cost is the same as progressive P frame
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.m_curbeData.VmeSPath1.MvCostInterlace),
            sizeof(uint32_t)* 2,
            m_vmeLutXyP,
            sizeof(uint32_t)* 2));

        }
    else// B_TYPE
    {
        cmd.m_curbeData.DW1.m_biWeight = 32;
        cmd.m_curbeData.DW1.m_refPixShift = 0;
        cmd.m_curbeData.DW2.m_maxLenSP = 0x05;
        cmd.m_curbeData.DW4.m_pictureType = 2;
        cmd.m_curbeData.DW9.m_subMbPartMask = 0x7E;
        cmd.m_curbeData.DW9.m_subPelMode = 1;
        cmd.m_curbeData.DW9.m_skipType = 0;
        cmd.m_curbeData.DW9.m_searchCtrl = 7;
        cmd.m_curbeData.DW9.m_interSAD = 2;
        cmd.m_curbeData.DW9.m_intraSAD = 2;
        cmd.m_curbeData.DW10.m_refWidth = 32;
        cmd.m_curbeData.DW10.m_refHeight = 32;
        cmd.m_curbeData.DW7.m_skipCenterMask = 0xFF;

        switch (m_picParams->m_gopRefDist)
        {
        case 3:
            cmd.m_curbeData.DW12.m_distScaleFactor = (m_frameNumB > 1) ? 43 : 21;
            break;
        case 4:
            cmd.m_curbeData.DW12.m_distScaleFactor = (m_frameNumB << 4);
            break;
        default:
            cmd.m_curbeData.DW12.m_distScaleFactor = 32;
            break;
        }

        //Motion vector cost is taken from VME_LUTXY
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.m_curbeData.MvCost), sizeof(uint32_t)* 2,
            m_vmeLutXyB,
            sizeof(uint32_t)* 2));

        //VME_SEARCH_PATH
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.m_curbeData.VmeSPath0),
            sizeof(uint32_t)* 16,
            m_vmeSPathB0,
            sizeof(uint32_t)* 16));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.m_curbeData.VmeSPath1),
            sizeof(uint32_t)* 16,
            m_vmeSPathB1,
            sizeof(uint32_t)* 16));

        //Interlaced motion vector cost is the same as progressive P frame
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &(cmd.m_curbeData.VmeSPath1.MvCostInterlace),
            sizeof(uint32_t)* 2,
            m_vmeLutXyP,
            sizeof(uint32_t)* 2));

    }

    //ModeCost for P/B pictures
    if (codingType != I_TYPE)
    {
        cmd.m_curbeData.VmeSPath0.DW30.m_value = 0x83; // Setting mode 0 cost to 0x83 (131)
        cmd.m_curbeData.VmeSPath0.DW31.m_value = 0x41414141; // Set mode 4, 5, 6, 7 costs to 0x41 (67)
        cmd.m_curbeData.DW8.m_mode8Cost        = 0x41;
    }

    cmd.m_curbeData.DW48.m_value = 0x05000000; // BB-End Command
    cmd.m_curbeData.DW49.m_value = m_mbEncBindingTable.m_mbEncPakObj;
    cmd.m_curbeData.DW50.m_value = m_mbEncBindingTable.m_mbEncPakObjPrev;
    cmd.m_curbeData.DW51.m_value = m_mbEncBindingTable.m_mbEncCurrentY;
    cmd.m_curbeData.DW52.m_value = m_mbEncBindingTable.m_mbEncCurrentPic; // Also FWD REF & BWD REF too, +1, +2 respectively
    cmd.m_curbeData.DW53.m_value = m_mbEncBindingTable.m_mbEncInterlaceFrameCurrentPic; // Also Int BWD Ref  (+1)
    cmd.m_curbeData.DW54.m_value = m_mbEncBindingTable.m_mbEncBrcDistortionSurface;
    cmd.m_curbeData.DW55.m_value = m_mbEncBindingTable.m_mbEncMbControl;

    PMHW_KERNEL_STATE kernelState;
    // Initialize DSH kernel region
    if (mbEncIFrameDistEnabled)
    {
        kernelState = &m_brcKernelStates[CODECHAL_ENCODE_BRC_IDX_IFRAMEDIST];
    }
    else
    {
        // wPictureCodingType: I_TYPE = 1, P_TYPE = 2, B_TYPE = 3
        // KernelStates are I: 0, P: 1, B: 2
        // m_mbEncKernelStates: I: m_mbEncKernelStates[0], P: m_mbEncKernelStates[1], B: m_mbEncKernelStates[2]
        uint32_t krnStateIdx = codingType - 1;
        kernelState = &m_mbEncKernelStates[krnStateIdx];
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &cmd,
        kernelState->dwCurbeOffset,
        cmd.m_byteSize));

    return eStatus;

}

MOS_STATUS CodechalEncodeMpeg2G9::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_picParams);

    m_mbEncForcePictureCodingType = 0;

    // use P_TYPE in G9 MbEnc kernel for B with invalid backward reference
    if (m_pictureCodingType == B_TYPE && CodecHal_PictureIsInvalid(m_picParams->m_refFrameList[1]))
    {
        m_mbEncForcePictureCodingType = P_TYPE;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeMpeg2::ExecuteKernelFunctions());

    return eStatus;
}
