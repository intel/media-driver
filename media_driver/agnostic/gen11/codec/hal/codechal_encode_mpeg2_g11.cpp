/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     codechal_encode_mpeg2_g11.cpp
//! \brief    MPEG2 dual-pipe encoder for GEN11.
//!

#include "codechal_encode_mpeg2_g11.h"
#include "codechal_kernel_hme_g11.h"
#include "codechal_kernel_header_g11.h"
#ifndef _FULL_OPEN_SOURCE
#include "igcodeckrn_g11.h"
#include "igcodeckrn_g11_icllp.h"
#endif
#include "mos_util_user_interface.h"
#include "codeckrnheader.h"

struct KernelHeader
{
    uint32_t m_kernelCount;

    // MBEnc Norm/Perf/Quality moes for frame
    CODECHAL_KERNEL_HEADER m_mpeg2MbEncI;
    CODECHAL_KERNEL_HEADER m_mpeg2MbEncP;
    CODECHAL_KERNEL_HEADER m_mpeg2MbEncB;

    CODECHAL_KERNEL_HEADER m_mpeg2InitFrameBrc;
    CODECHAL_KERNEL_HEADER m_mpeg2FrameEncUpdate;
    CODECHAL_KERNEL_HEADER m_mpeg2BrcResetFrame;
};

enum BindingTableOffsetMbEncG11
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
    mbEncSwScoreboard               = 19,
    mbEncNumBindingTableEntries     = 20
};

//!
//! \enum     StartCode
//! \brief    Start code
//!
enum StartCode
{
    startCodePrefix = 0x000001, // bit string 0000 0000 0000 0000 0000 0001
    startCodePicture = 0x00,
    // slice_start_code = 0x01..0xAF, it is the slice_vertical_position for the slice
    // 0xB0, 0xB1, 0xB6 - Reserved
    startCodeUserData = 0xB2,
    startCodeSequenceHeader = 0xB3,
    startCodeSequenceError = 0xB4,
    startCodeExtension = 0xB5,
    startCodeSequenceEnd = 0xB7,
    startCodeGroupStart = 0xB8
    // system start codes = 0xB9..0xFF
};

class MbEncCurbeG11
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
                    uint32_t m_mv0Cost                            : MOS_BITFIELD_RANGE(0,7);
                    uint32_t m_mv1Cost                            : MOS_BITFIELD_RANGE(8,15);
                    uint32_t m_mv2Cost                            : MOS_BITFIELD_RANGE(16,23);
                    uint32_t m_mv3Cost                            : MOS_BITFIELD_RANGE(24,31);
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
                    uint32_t m_mv4Cost                            : MOS_BITFIELD_RANGE(0,7);
                    uint32_t m_mv5Cost                            : MOS_BITFIELD_RANGE(8,15);
                    uint32_t m_mv6Cost                            : MOS_BITFIELD_RANGE(16,23);
                    uint32_t m_mv7Cost                            : MOS_BITFIELD_RANGE(24,31);
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

        union
        {
            struct
            {
                uint32_t softwareScoreboard;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW56;

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
        } DW57;

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
        } DW58;

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
        } DW59;

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
        } DW60;

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
        } DW61;

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
        } DW62;

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
        } DW63;
    }m_curbeData;

    //!
    //! \brief    Constructor
    //!
    MbEncCurbeG11(uint8_t codingType);

    //!
    //! \brief    Destructor
    //!
    ~MbEncCurbeG11(){};

    static const size_t m_byteSize = sizeof(CurbeData);
};

MbEncCurbeG11::MbEncCurbeG11(uint8_t codingType)
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

    m_curbeData.DW3.m_value  = 0xFE0C0000;
    m_curbeData.MvCost.DW5.m_value  = 0x00000000;
    m_curbeData.MvCost.DW6.m_value  = 0x00000000;
    m_curbeData.DW11.m_value = 0x5A325300;
    m_curbeData.DW12.m_value = 0x0000FF00;
    m_curbeData.DW13.m_value = 0x00FF0000;

}

CodechalEncodeMpeg2G11::CodechalEncodeMpeg2G11(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalEncodeMpeg2(hwInterface, debugInterface, standardInfo),
    m_sinlgePipeVeState(nullptr)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_ASSERT(m_osInterface);

    m_kuidCommon = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;

    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);

    pfnGetKernelHeaderAndSize = GetKernelHeaderAndSize;
    uint8_t *kernelBase = nullptr;
#ifndef _FULL_OPEN_SOURCE
    // ICL and ICLLP use different kernels
    if (GFX_IS_PRODUCT(m_hwInterface->GetPlatform(), IGFX_ICELAKE))
    {
        kernelBase = (uint8_t *)IGCODECKRN_G11;
    }
    else
    {
        kernelBase = (uint8_t *)IGCODECKRN_G11_ICLLP;
    }
#endif
    m_useHwScoreboard = false;
    m_useCommonKernel = true;

    MOS_STATUS eStatus = CodecHalGetKernelBinaryAndSize(
        kernelBase,
        m_kuid,
        &m_kernelBinary,
        &m_combinedKernelSize);

    // Virtual Engine is enabled in default
    Mos_SetVirtualEngineSupported(m_osInterface, true);

    CODECHAL_ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    m_hwInterface->GetStateHeapSettings()->dwIshSize +=
        MOS_ALIGN_CEIL(m_combinedKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));

    m_vdboxOneDefaultUsed = true;
}

CodechalEncodeMpeg2G11::~CodechalEncodeMpeg2G11()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_swScoreboardState)
    {
        MOS_Delete(m_swScoreboardState);
        m_swScoreboardState = nullptr;
    }

    if (m_sinlgePipeVeState)
    {
        MOS_FreeMemAndSetNull(m_sinlgePipeVeState);
    }
}

MOS_STATUS CodechalEncodeMpeg2G11::Initialize(CodechalSetting * codecHalSettings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeMpeg2::Initialize(codecHalSettings));

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        m_sinlgePipeVeState = (PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE));
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_sinlgePipeVeState);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_InitInterface(m_hwInterface, m_sinlgePipeVeState));
    }

    return eStatus;
}

uint32_t CodechalEncodeMpeg2G11::GetMaxBtCount()
{
    uint32_t scalingBtCount = MOS_ALIGN_CEIL(
        m_scaling4xKernelStates[0].KernelParams.iBTCount,
        m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
    uint32_t meBtCount = MOS_ALIGN_CEIL(
        m_hmeKernel ? m_hmeKernel->GetBTCount() : 0 ,
        m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
    uint32_t mbEncBtCount = MOS_ALIGN_CEIL(
        m_mbEncKernelStates[0].KernelParams.iBTCount,
        m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());

    uint32_t brcBtCount = 0;
    for (uint32_t i = 0; i < CODECHAL_ENCODE_BRC_IDX_NUM; i++)
    {
        brcBtCount += MOS_ALIGN_CEIL(
            m_brcKernelStates[i].KernelParams.iBTCount,
            m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
    }

    uint32_t swScoreboardBtCount = 0;
    if (!m_useHwScoreboard)
    {
        swScoreboardBtCount = MOS_ALIGN_CEIL(
            m_swScoreboardState->GetBTCount(),
            m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
    }

    return MOS_MAX(scalingBtCount + meBtCount, mbEncBtCount + brcBtCount + swScoreboardBtCount);
}

MOS_STATUS CodechalEncodeMpeg2G11::GetKernelHeaderAndSize(
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

    if (operation == ENC_BRC)
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

MOS_STATUS CodechalEncodeMpeg2G11::InitKernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Init kernel state
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMbEnc());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateBrc());

    // Create SW scoreboard init kernel state
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_swScoreboardState = MOS_New(CodechalEncodeSwScoreboardG11, this));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_swScoreboardState->InitKernelState());

    // Create hme kernel
    m_hmeKernel = MOS_New(CodechalKernelHmeG11, this, true);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hmeKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Initialize(
        GetCommonKernelHeaderAndSizeG11,
        m_kernelBase,
        m_kuidCommon));
    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G11::InitKernelStateMbEnc()
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
        kernelStatePtr->KernelParams.iCurbeLength = MbEncCurbeG11::m_byteSize;
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
    m_mbEncBindingTable.m_mbEncScoreboard                 = mbEncSwScoreboard;

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G11::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_SUPPORTED(m_osInterface))
    {
        return eStatus;
    }

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;
        MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
        vesetParams.bNeedSyncWithPrevious = true;
        vesetParams.bSFCInUse = false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_SetHintParams(m_sinlgePipeVeState, &vesetParams));
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_PopulateHintParams(m_sinlgePipeVeState, cmdBuffer, true));

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G11::ExecuteSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    auto cpInterface = m_hwInterface->GetCpInterface();

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (m_osInterface->osCpInterface->IsCpEnabled())
    {
        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.bLastPass = (m_currPass == m_numPasses) ? true : false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->SetMfxProtectionState(m_mfxInterface->IsDecodeInUse(), &cmdBuffer, nullptr, &sliceInfoParam));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->UpdateParams(false));
    }

    MHW_VDBOX_MPEG2_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.presDataBuffer = &m_resMbCodeSurface;
    sliceState.pMpeg2PicIdx = &(m_picIdx[0]);
    sliceState.ppMpeg2RefList = &(m_refList[0]);
    sliceState.pEncodeMpeg2SeqParams = m_seqParams;
    sliceState.pEncodeMpeg2PicParams = m_picParams;
    sliceState.pEncodeMpeg2SliceParams = m_sliceParams;
    sliceState.pBsBuffer = &m_bsBuffer;
    sliceState.bBrcEnabled = m_brcEnabled;
    if (m_seqParams->m_forcePanicModeControl == 1) {
        sliceState.bRCPanicEnable = !m_seqParams->m_panicModeDisable;
    }
    else {
        sliceState.bRCPanicEnable = m_panicEnable;
    }
    sliceState.presPicHeaderBBSurf = &m_brcBuffers.resBrcPicHeaderOutputBuffer;

    for (uint16_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        //we should not need to call pfnPackSliceHeader this it's done by hw
        PCODEC_ENCODER_SLCDATA  slcData = m_slcData;
        CODECHAL_ENCODE_CHK_NULL_RETURN(slcData);
        sliceState.dwDataBufferOffset =
            m_slcData[slcCount].CmdOffset + m_mbcodeBottomFieldOffset;
        sliceState.dwOffset = slcData[slcCount].SliceOffset;
        sliceState.dwLength = slcData[slcCount].BitSize;
        sliceState.dwSliceIndex = slcCount;
        sliceState.bFirstPass = true;
        sliceState.bLastPass = false;
        sliceState.pSlcData = &slcData[slcCount];
        sliceState.bFirstPass = (m_currPass == 0);
        sliceState.bLastPass = (m_currPass == m_numPasses);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendSliceParams(&cmdBuffer, &sliceState));
    }

    // Insert end of stream if set
    if (m_lastPicInStream)
    {
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastPicInStream = true;
        if (m_codecFunction == CODECHAL_FUNCTION_ENC_PAK)
        {
            pakInsertObjectParams.bSetLastPicInStreamData = true;
            pakInsertObjectParams.dwBitSize = 32;   // use dwBitSize for SrcDataEndingBitInclusion
            pakInsertObjectParams.dwLastPicInStreamData = (uint32_t)((1 << 16) | startCodeSequenceEnd << 24);
        }
        else
        {
            pakInsertObjectParams.bSetLastPicInStreamData = false;
            pakInsertObjectParams.dwBitSize = 8;    // use dwBitSize for SrcDataEndingBitInclusion
            pakInsertObjectParams.dwLastPicInStreamData = 0;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr, &pakInsertObjectParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadMfcStatus(&cmdBuffer));

    // BRC PAK statistics different for each pass
    if (m_brcEnabled)
    {
        uint32_t frameOffset =
            (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
            m_encodeStatusBuf.dwNumPassesOffset +   // Num passes offset
            sizeof(uint32_t) * 2;                                                          // pEncodeStatus is offset by 2 DWs in the resource

        EncodeReadBrcPakStatsParams   readBrcPakStatsParams;
        readBrcPakStatsParams.pHwInterface = m_hwInterface;
        readBrcPakStatsParams.presBrcPakStatisticBuffer = &m_brcBuffers.resBrcPakStatisticBuffer[0];
        readBrcPakStatsParams.presStatusBuffer = &m_encodeStatusBuf.resStatusBuffer;
        readBrcPakStatsParams.dwStatusBufNumPassesOffset = frameOffset;
        readBrcPakStatsParams.ucPass = m_currPass;
        readBrcPakStatsParams.VideoContext = m_videoContext;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStatistics(
            &cmdBuffer,
            &readBrcPakStatsParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(
        &cmdBuffer,
        CODECHAL_NUM_MEDIA_STATES));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    std::string Pak_pass = "PAK_PASS[" + std::to_string(static_cast<uint32_t>(m_currPass)) + "]";
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            Pak_pass.c_str()));

    //CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
    //    m_debugInterface,
    //    &cmdBuffer));
    )

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    MOS_SYNC_PARAMS syncParams;
    if ((m_currPass == 0) &&
        !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    if (!m_singleTaskPhaseSupported ||
        m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(&cmdBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
            m_osInterface,
            &cmdBuffer,
            m_videoContextUsesNullHw));

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState)
            {
                m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
            }
        )

            if ((m_currPass == m_numPasses) &&
                m_signalEnc &&
                !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
            {
                // Check if the signal obj count exceeds max m_value
                if (m_semaphoreObjCount == MOS_MIN(m_semaphoreMaxCount, MOS_MAX_OBJECT_SIGNALED))
                {
                    syncParams = g_cInitSyncParams;
                    syncParams.GpuContext = m_renderContext;
                    syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
                    m_semaphoreObjCount--;
                }

                // signal semaphore
                syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_videoContext;
                syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
                m_semaphoreObjCount++;
            }
    }

    // Reset parameters for next PAK execution
    if (m_currPass == m_numPasses)
    {
        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G11::EncodeMeKernel()
{
    CodechalKernelHme::CurbeParam curbeParam = {};
    curbeParam.subPelMode = 3;
    curbeParam.currOriginalPic = m_picParams->m_currOriginalPic;
    curbeParam.qpPrimeY = m_mvCostTableOffset;
    // This parameter is used to select correct mode mv cost
    // and search path from the predefined tables specifically
    // for Mpeg2 BRC encoding path
    curbeParam.targetUsage = 8;
    curbeParam.maxMvLen = 128;
    curbeParam.numRefIdxL0Minus1 = 0;
    curbeParam.numRefIdxL1Minus1 = 0;

    CODEC_PICTURE l0RefFrameList = m_picParams->m_refFrameList[0];
    // no need to modify index if picture is invalid
    if (l0RefFrameList.PicFlags != PICTURE_INVALID)
    {
        l0RefFrameList.FrameIdx = 0;
    }

    CODEC_PICTURE l1RefFrameList = m_picParams->m_refFrameList[1];
    // no need to modify index if picture is invalid
    if (l1RefFrameList.PicFlags != PICTURE_INVALID)
    {
        l1RefFrameList.FrameIdx = 1;
    }

    CodechalKernelHme::SurfaceParams surfaceParam = {};
    surfaceParam.numRefIdxL0ActiveMinus1 = 0;
    surfaceParam.numRefIdxL1ActiveMinus1 = 0;
    surfaceParam.refList = &m_refList[0];
    surfaceParam.picIdx = &m_picIdx[0];
    surfaceParam.currOriginalPic = &m_currOriginalPic;
    surfaceParam.refL0List = &l0RefFrameList;
    surfaceParam.refL1List = &l1RefFrameList;
    surfaceParam.meBrcDistortionBuffer = &m_brcBuffers.sMeBrcDistortionBuffer;
    surfaceParam.meBrcDistortionBottomFieldOffset = m_brcBuffers.dwMeBrcDistortionBottomFieldOffset;
    surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb4x;
    surfaceParam.downScaledHeightInMb = m_downscaledHeightInMb4x;
    surfaceParam.downScaledBottomFieldOffset = m_scaledBottomFieldOffset;
    surfaceParam.verticalLineStride = m_verticalLineStride;
    surfaceParam.verticalLineStrideOffset = m_verticalLineStrideOffset;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel4x));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeMpeg2G11::SetCurbeMbEnc(
    bool mbEncIFrameDistEnabled,
    bool mbQpDataEnabled)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint16_t picWidthInMb = mbEncIFrameDistEnabled ?
            (uint16_t)m_downscaledWidthInMb4x : m_picWidthInMb;
    uint16_t fieldFrameHeightInMb   = mbEncIFrameDistEnabled ?
            (uint16_t)m_downscaledFrameFieldHeightInMb4x : m_frameFieldHeightInMb;

    MbEncCurbeG11 cmd(m_picParams->m_pictureCodingType);

    // Set CURBE data
    cmd.m_curbeData.DW0.m_isInterlacedFrameFlag =
        (CodecHal_PictureIsFrame(m_picParams->m_currOriginalPic)) ? (m_picParams->m_fieldCodingFlag || m_picParams->m_fieldFrameCodingFlag) : 0;
    cmd.m_curbeData.DW0.m_isTopFieldFirst = cmd.m_curbeData.DW0.m_isInterlacedFrameFlag ? !m_picParams->m_interleavedFieldBFF : 0;
    cmd.m_curbeData.DW0.m_forceToSkip = 0;
    cmd.m_curbeData.DW4.m_pictureType = m_picParams->m_pictureCodingType;
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

    if (m_picParams->m_pictureCodingType == I_TYPE)
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
    else if (m_picParams->m_pictureCodingType == P_TYPE)
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

        // This calculation of m_distScaleFactor was taken directly from the algorithm.
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
    if (m_picParams->m_pictureCodingType != I_TYPE)
    {
        cmd.m_curbeData.VmeSPath0.DW30.m_value = 0x83; // Setting mode 0 cost to 0x83 (131)
        cmd.m_curbeData.VmeSPath0.DW31.m_value = 0x41414141; // Set mode 4, 5, 6, 7 costs to 0x41 (67)
        cmd.m_curbeData.DW8.m_mode8Cost = 0x41;
    }

    cmd.m_curbeData.DW48.m_value = 0x05000000; // BB-End Command
    cmd.m_curbeData.DW49.m_value = m_mbEncBindingTable.m_mbEncPakObj;
    cmd.m_curbeData.DW50.m_value = m_mbEncBindingTable.m_mbEncPakObjPrev;
    cmd.m_curbeData.DW51.m_value = m_mbEncBindingTable.m_mbEncCurrentY;
    cmd.m_curbeData.DW52.m_value = m_mbEncBindingTable.m_mbEncCurrentPic; // Also FWD REF & BWD REF too, +1, +2 respectively
    cmd.m_curbeData.DW53.m_value = m_mbEncBindingTable.m_mbEncInterlaceFrameCurrentPic; // Also Int BWD Ref  (+1)
    cmd.m_curbeData.DW54.m_value = m_mbEncBindingTable.m_mbEncBrcDistortionSurface;
    cmd.m_curbeData.DW55.m_value = m_mbEncBindingTable.m_mbEncMbControl;
    // P/B frames need software score board
    if (m_picParams->m_pictureCodingType != I_TYPE)
    {
        cmd.m_curbeData.DW56.m_value = m_mbEncBindingTable.m_mbEncScoreboard;
    }

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
        uint32_t krnStateIdx = m_pictureCodingType - 1;
        kernelState = &m_mbEncKernelStates[krnStateIdx];
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &cmd,
        kernelState->dwCurbeOffset,
        cmd.m_byteSize));

    return eStatus;

}

MOS_STATUS CodechalEncodeMpeg2G11::SetGpuCtxCreatOption()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::SetGpuCtxCreatOption());
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
            m_sinlgePipeVeState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G11::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::UserFeatureKeyReport());

#if (_DEBUG || _RELEASE_INTERNAL)

    // VE2.0 Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID, MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface));

#endif // _DEBUG || _RELEASE_INTERNAL

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G11::SendMbEncSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool mbEncIFrameDistEnabled)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    auto currPicSurface = mbEncIFrameDistEnabled ? m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER) : m_rawSurfaceToEnc;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
    m_mmcState->GetSurfaceMmcState(currPicSurface);

    // forward reference
    if (m_picIdx[0].bValid)
    {
        uint8_t picIdx0 = m_picIdx[0].ucPicIdx;

        if (picIdx0 < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
        {
            CodecHalGetResourceInfo(m_osInterface, &m_refList[picIdx0]->sRefBuffer);
            m_mmcState->GetSurfaceMmcState(&m_refList[picIdx0]->sRefBuffer);
        }
    }

    // backward reference
    if (m_picIdx[1].bValid)
    {
        uint8_t picIdx1 = m_picIdx[1].ucPicIdx;
        if (picIdx1 < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
        {
            CodecHalGetResourceInfo(m_osInterface, &m_refList[picIdx1]->sRefBuffer);
            m_mmcState->GetSurfaceMmcState(&m_refList[picIdx1]->sRefBuffer);
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeMpeg2::SendMbEncSurfaces(
        cmdBuffer,
        mbEncIFrameDistEnabled));

    if (!m_useHwScoreboard &&
        m_pictureCodingType != I_TYPE)
    {
        PMHW_KERNEL_STATE kernelState;
        if (mbEncIFrameDistEnabled)
        {
            kernelState = &m_brcKernelStates[CODECHAL_ENCODE_BRC_IDX_IFRAMEDIST];
        }
        else
        {
            // wPictureCodingType: I_TYPE = 1, P_TYPE = 2, B_TYPE = 3
            // KernelStates are I: 0, P: 1, B: 2
            // m_mbEncKernelStates: I: m_mbEncKernelStates[0], P: m_mbEncKernelStates[1], B: m_mbEncKernelStates[2]
            uint32_t krnStateIdx = m_pictureCodingType - 1;

            kernelState = &m_mbEncKernelStates[krnStateIdx];
        }

        CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = m_swScoreboardState->GetCurSwScoreboardSurface();
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MB_QP_CODEC].Value;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncScoreboard;
        surfaceCodecParams.bUse32UINTSurfaceFormat = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G11::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    bool                        frameTracking,
    MHW_MI_MMIOREGISTERS       *mmioRegister)
{
    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
                (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);
        if (attriExt)
        {
            attriExt->bUseVirtualEngineHint = true;
            attriExt->VEngineHintParams.NeedSyncWithPrevious = 1;
        }
    }

    return CodechalEncoderState::SendPrologWithFrameTracking(cmdBuffer, frameTracking, mmioRegister);
}

MOS_STATUS CodechalEncodeMpeg2G11::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN( m_debugInterface->DumpYUVSurface(
        m_rawSurfaceToEnc,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "SrcSurf")));

    m_firstTaskInPhase = true;
    m_lastTaskInPhase = !m_singleTaskPhaseSupported;
    m_lastEncPhase = false;

    m_setRequestedEUSlices = (m_brcEnabled &&
        m_sliceStateEnable &&
        (m_frameHeight * m_frameWidth) >= m_hwInterface->m_mpeg2SSDResolutionThreshold) ? true : false;

    m_hwInterface->m_numRequestedEuSlices = (m_setRequestedEUSlices) ?
        m_sliceShutdownRequestState : m_sliceShutdownDefaultState;

    // Csc, Downscaling, and/or 10-bit to 8-bit conversion
    // Scaling is only used to calculate distortions in case of Mpeg2
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_cscDsState);

    CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
    MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
    cscScalingKernelParams.bLastTaskInPhaseCSC =
        cscScalingKernelParams.bLastTaskInPhase4xDS = m_pictureCodingType == I_TYPE;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));

    // P and B frames distortion calculations
    if (m_hmeSupported && (m_pictureCodingType != I_TYPE))
    {
        m_firstTaskInPhase = !m_singleTaskPhaseSupported;
        m_lastTaskInPhase = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel());
    }

    MOS_SYNC_PARAMS syncParams;

    // Scaling and HME are not dependent on the output from PAK
    if (m_waitForPak &&
        m_semaphoreObjCount &&
        !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
    {
        // Wait on PAK
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;
        syncParams.uiSemaphoreCount = m_semaphoreObjCount;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
        m_semaphoreObjCount = 0; //reset
    }

    m_firstTaskInPhase = true;
    if (m_brcEnabled)
    {
        if (m_pictureCodingType == I_TYPE)
        {
            // The reset/init is only valid for I frames
            if (m_brcInit || m_brcReset)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcInitResetKernel());
                m_firstTaskInPhase = !m_singleTaskPhaseSupported;
            }

            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMbEncKernel(true));
            m_firstTaskInPhase = !m_singleTaskPhaseSupported;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcUpdateKernel());
        m_firstTaskInPhase = !m_singleTaskPhaseSupported;
    }

    if (!m_useHwScoreboard && m_pictureCodingType != I_TYPE)
    {
        CodechalEncodeSwScoreboard::KernelParams swScoreboardKernelParames;
        MOS_ZeroMemory(&swScoreboardKernelParames, sizeof(swScoreboardKernelParames));

        m_swScoreboardState->SetDependencyPattern(m_pictureCodingType == P_TYPE ?
            dependencyWavefront45Degree : dependencyWavefrontHorizontal);
        swScoreboardKernelParames.surfaceIndex = m_swScoreboardState->GetDependencyPattern();

        m_swScoreboardState->SetCurSwScoreboardSurfaceIndex(swScoreboardKernelParames.surfaceIndex);

        if (Mos_ResourceIsNull(&m_swScoreboardState->GetCurSwScoreboardSurface()->OsResource))
        {
            swScoreboardKernelParames.scoreboardWidth           = m_picWidthInMb;
            swScoreboardKernelParames.scoreboardHeight          = m_frameFieldHeightInMb;
            swScoreboardKernelParames.swScoreboardSurfaceWidth  = swScoreboardKernelParames.scoreboardWidth * 4;
            swScoreboardKernelParames.swScoreboardSurfaceHeight = swScoreboardKernelParames.scoreboardHeight;

            m_swScoreboardState->Execute(&swScoreboardKernelParames);
        }

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &(m_swScoreboardState->GetCurSwScoreboardSurface())->OsResource,
            CodechalDbgAttr::attrOutput,
            "Out",
            (m_swScoreboardState->GetCurSwScoreboardSurface())->dwHeight * (m_swScoreboardState->GetCurSwScoreboardSurface())->dwPitch,
            0,
            CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT));
        )
    }

    m_lastTaskInPhase = true;
    m_lastEncPhase = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMbEncKernel(false));

    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
    }

    CODECHAL_DEBUG_TOOL(
        if (m_hmeEnabled && m_brcEnabled)
        {
            CODECHAL_ME_OUTPUT_PARAMS meOutputParams;
            MOS_ZeroMemory(&meOutputParams, sizeof(CODECHAL_ME_OUTPUT_PARAMS));
            meOutputParams.psMeMvBuffer = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer);
            meOutputParams.psMeBrcDistortionBuffer =
                m_brcDistortionBufferSupported ? &m_brcBuffers.sMeBrcDistortionBuffer : nullptr;
            meOutputParams.psMeDistortionBuffer = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer);
            meOutputParams.b16xMeInUse = false;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &meOutputParams.psMeMvBuffer->OsResource,
                CodechalDbgAttr::attrOutput,
                "MvData",
                meOutputParams.psMeMvBuffer->dwHeight *meOutputParams.psMeMvBuffer->dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));
            if (!m_vdencStreamInEnabled && meOutputParams.psMeBrcDistortionBuffer)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &meOutputParams.psMeBrcDistortionBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "BrcDist",
                    meOutputParams.psMeBrcDistortionBuffer->dwHeight *meOutputParams.psMeBrcDistortionBuffer->dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4), 8) : 0,
                    CODECHAL_MEDIA_STATE_4X_ME));
                if (meOutputParams.psMeDistortionBuffer)
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                        &meOutputParams.psMeDistortionBuffer->OsResource,
                        CodechalDbgAttr::attrOutput,
                        "MeDist",
                        meOutputParams.psMeDistortionBuffer->dwHeight *meOutputParams.psMeDistortionBuffer->dwPitch,
                        CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4 * 10), 8) : 0,
                        CODECHAL_MEDIA_STATE_4X_ME));
                }
            }
            // dump VDEncStreamin
            if (m_vdencStreamInEnabled)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
                    CodechalDbgAttr::attrOutput,
                    "MvData",
                    m_picWidthInMb * m_picHeightInMb* CODECHAL_CACHELINE_SIZE,
                    0,
                    CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN));
            }
        }
        if (m_mbQpDataEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_mbQpDataSurface.OsResource,
                CodechalDbgAttr::attrInput,
                "MbQp",
                m_mbQpDataSurface.dwHeight*m_mbQpDataSurface.dwPitch,
                0,
                CODECHAL_MEDIA_STATE_ENC_QUALITY));
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcImageStatesWriteBuffer,
            CodechalDbgAttr::attrOutput,
            "ImgStateWrite",
            BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses(),
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcHistoryBuffer,
            CodechalDbgAttr::attrOutput,
            "HistoryWrite",
            m_brcHistoryBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));
        if (m_brcBuffers.pMbEncKernelStateInUse)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
                CODECHAL_MEDIA_STATE_ENC_NORMAL,
                m_brcBuffers.pMbEncKernelStateInUse));
        }
        if (m_mbencBrcBufferSize>0)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcBuffers.resMbEncBrcBuffer,
                CodechalDbgAttr::attrOutput,
                "MbEncBRCWrite",
                m_mbencBrcBufferSize,
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcPicHeaderOutputBuffer,
            CodechalDbgAttr::attrOutput,
            "PicHeaderWrite",
            CODEC_ENCODE_MPEG2_BRC_PIC_HEADER_SURFACE_SIZE,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));
    )

        // Reset after BRC Init has been processed
        m_brcInit = false;

    m_setRequestedEUSlices = false;

    // Reset indices for next frame
    if (m_brcEnabled)
    {
        m_mbEncCurbeSetInBrcUpdate = false;
    }

    return eStatus;
}

void CodechalEncodeMpeg2G11::ResizeOnResChange()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncoderState::ResizeOnResChange();

    // need to re-allocate surfaces according to resolution
    m_swScoreboardState->ReleaseResources();
}

MOS_STATUS CodechalEncodeMpeg2G11::UpdateCmdBufAttribute(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                renderEngineInUse)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // should not be there. Will remove it in the next change
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
            (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);

        if (attriExt)
        {
            memset(attriExt, 0, sizeof(MOS_CMD_BUF_ATTRI_VE));
            attriExt->bUseVirtualEngineHint =
                attriExt->VEngineHintParams.NeedSyncWithPrevious = !renderEngineInUse;
        }
    }

    return eStatus;
}
