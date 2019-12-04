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
//! \file     codechal_encode_mpeg2_g12.cpp
//! \brief    MPEG2 dual-pipe encoder for GEN12.
//!

#include "codechal_encode_mpeg2_g12.h"
#include "codechal_kernel_hme_g12.h"
#include "codechal_kernel_header_g12.h"
#include "codechal_mmc_encode_mpeg2_g12.h"
#include "mhw_render_g12_X.h"
#include "igcodeckrn_g12.h"
#include "mos_util_user_interface_g12.h"
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

enum BindingTableOffsetMbEncG12
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

class MbEncCurbeG12
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
                    uint32_t m_mode0Cost                          : MOS_BITFIELD_RANGE(0,7);
                    uint32_t m_mode1Cost                          : MOS_BITFIELD_RANGE(8,15);
                    uint32_t m_mode2Cost                          : MOS_BITFIELD_RANGE(16,23);
                    uint32_t m_mode3Cost                          : MOS_BITFIELD_RANGE(24,31);
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
                    uint32_t m_mode4Cost                          : MOS_BITFIELD_RANGE(0,7);
                    uint32_t m_mode5Cost                          : MOS_BITFIELD_RANGE(8,15);
                    uint32_t m_mode6Cost                          : MOS_BITFIELD_RANGE(16,23);
                    uint32_t m_mode7Cost                          : MOS_BITFIELD_RANGE(24,31);
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
    MbEncCurbeG12(uint8_t codingType);

    //!
    //! \brief    Destructor
    //!
    ~MbEncCurbeG12(){};

    static const size_t m_byteSize = sizeof(CurbeData);
};

MbEncCurbeG12::MbEncCurbeG12(uint8_t codingType)
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

CodechalEncodeMpeg2G12::CodechalEncodeMpeg2G12(
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

    m_useHwScoreboard = false;
    m_useCommonKernel = true;

    MOS_STATUS eStatus = CodecHalGetKernelBinaryAndSize(
        (uint8_t *)IGCODECKRN_G12,
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

CodechalEncodeMpeg2G12::~CodechalEncodeMpeg2G12()
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

MOS_STATUS CodechalEncodeMpeg2G12::Initialize(CodechalSetting * codecHalSettings)
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

MOS_STATUS CodechalEncodeMpeg2G12::SetAndPopulateVEHintParams(
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

MOS_STATUS CodechalEncodeMpeg2G12::SubmitCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    int32_t             bNullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
     m_osInterface,
     cmdBuffer,
     bNullRendering));

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G12::GetKernelHeaderAndSize(
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

uint32_t CodechalEncodeMpeg2G12::GetMaxBtCount()
{
    uint32_t scalingBtCount = MOS_ALIGN_CEIL(
        m_scaling4xKernelStates[0].KernelParams.iBTCount,
        m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
    uint32_t meBtCount = MOS_ALIGN_CEIL(
        m_hmeKernel ? m_hmeKernel->GetBTCount() : 0,
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

MOS_STATUS CodechalEncodeMpeg2G12::InitKernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Init kernel state
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMbEnc());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateBrc());

    // Create SW scoreboard init kernel state
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_swScoreboardState = MOS_New(CodechalEncodeSwScoreboardG12, this));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_swScoreboardState->InitKernelState());

     // Create hme kernel
    m_hmeKernel = MOS_New(CodechalKernelHmeG12, this, true);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hmeKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Initialize(
        GetCommonKernelHeaderAndSizeG12,
        m_kernelBase,
        m_kuidCommon));

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G12::InitKernelStateMbEnc()
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
        kernelStatePtr->KernelParams.iCurbeLength = MbEncCurbeG12::m_byteSize;
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

MOS_STATUS CodechalEncodeMpeg2G12::EncodeMeKernel()
{
    CodechalKernelHme::CurbeParam curbeParam = {};
    curbeParam.subPelMode = 3;
    curbeParam.currOriginalPic = m_picParams->m_currOriginalPic;
    curbeParam.qpPrimeY = m_mvCostTableOffset;
    // This parameter is used to select correct mode mv cost
    // and search path from the predefined tables specifically
    // for Mpeg2 BRC encoding path
    curbeParam.targetUsage = 8;
    curbeParam.maxMvLen = 128; // refer to m_maxVmvr/4
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

MOS_STATUS CodechalEncodeMpeg2G12::SetGpuCtxCreatOption()
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

MOS_STATUS CodechalEncodeMpeg2G12::UserFeatureKeyReport()
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

MOS_STATUS CodechalEncodeMpeg2G12::SetCurbeMbEnc(
    bool mbEncIFrameDistEnabled,
    bool mbQpDataEnabled)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint16_t picWidthInMb = mbEncIFrameDistEnabled ?
            (uint16_t)m_downscaledWidthInMb4x : m_picWidthInMb;
    uint16_t fieldFrameHeightInMb   = mbEncIFrameDistEnabled ?
            (uint16_t)m_downscaledFrameFieldHeightInMb4x : m_frameFieldHeightInMb;

    MbEncCurbeG12 cmd(m_picParams->m_pictureCodingType);

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
        cmd.m_curbeData.VmeSPath0.DW30.m_value = 0x83;       // Setting mode 0 cost to 0x83 (131)
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

MOS_STATUS CodechalEncodeMpeg2G12::SendMbEncSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool mbEncIFrameDistEnabled)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    auto currPicSurface = mbEncIFrameDistEnabled ? m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER) : m_rawSurfaceToEnc;

    // forward reference
    if (m_picIdx[0].bValid)
    {
        uint8_t picIdx0 = m_picIdx[0].ucPicIdx;

        if (picIdx0 < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
        {
            CodecHalGetResourceInfo(m_osInterface, &m_refList[picIdx0]->sRefBuffer);
        }
    }

    // backward reference
    if (m_picIdx[1].bValid)
    {
        uint8_t picIdx1 = m_picIdx[1].ucPicIdx;
        if (picIdx1 < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
        {
            CodecHalGetResourceInfo(m_osInterface, &m_refList[picIdx1]->sRefBuffer);
        }
    }

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

    auto presMbCodeBuffer = &m_refList[m_currReconstructedPic.FrameIdx]->resRefMbCodeBuffer;
    auto presPrevMbCodeBuffer = &m_refList[m_prevMBCodeIdx]->resRefMbCodeBuffer;

    // Caution: if PAFF supports added, need to make sure each field get correct surface pointer
    // PAK Obj command buffer
    uint32_t pakSize = (uint32_t)m_picWidthInMb * m_frameFieldHeightInMb * 16 * 4;  // 12 DW for MB + 4 DW for MV
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;

    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = presMbCodeBuffer;
    surfaceCodecParams.dwSize = pakSize;
    surfaceCodecParams.dwOffset = (uint32_t)m_mbcodeBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl =
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncPakObj;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Prev PAK Obj command buffer
    pakSize = (uint32_t)m_picWidthInMb * m_frameFieldHeightInMb * 16 * 4;  // 12 DW for MB + 4 DW for MV

                                                                           // verify if the current frame is not the first frame
    if (!Mos_ResourceIsNull(presPrevMbCodeBuffer) &&
        !m_firstFrame)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = presPrevMbCodeBuffer;
        surfaceCodecParams.dwSize = pakSize;
        surfaceCodecParams.dwOffset = (uint32_t)m_mbcodeBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncPakObjPrev;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Current Picture Y
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.psSurface = currPicSurface;
    surfaceCodecParams.dwCacheabilityControl =
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncCurrentY;
    surfaceCodecParams.dwVerticalLineStride = m_verticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = m_verticalLineStrideOffset;

#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    bool currBottomField = CodecHal_PictureIsBottomField(m_currOriginalPic) ? 1 : 0;
    uint8_t vDirection = (CodecHal_PictureIsFrame(m_currOriginalPic)) ? CODECHAL_VDIRECTION_FRAME :
        (currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;

    // Current Picture
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = currPicSurface;
    surfaceCodecParams.dwCacheabilityControl =
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncCurrentPic;
    surfaceCodecParams.ucVDirection = vDirection;

#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    uint8_t picIdx0 = CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2;
    uint8_t picIdx1 = CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2;
    bool refL0BottomField = false;
    bool refL1BottomField = false;

    if (m_picIdx[0].bValid)
    {
        picIdx0 = m_picIdx[0].ucPicIdx;
        refL0BottomField = (CodecHal_PictureIsBottomField(m_currOriginalPic)) ? 1 : 0;
    }

    if (m_picIdx[1].bValid)
    {
        picIdx1 = m_picIdx[1].ucPicIdx;
        refL1BottomField = (CodecHal_PictureIsBottomField(m_currOriginalPic)) ? 1 : 0;
    }

    // forward reference
    if (picIdx0 < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        if (m_verticalLineStride == CODECHAL_VLINESTRIDE_FIELD)
        {
            vDirection = (refL0BottomField ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
        }

        // Picture Y
        CodecHalGetResourceInfo(m_osInterface, &m_refList[picIdx0]->sRefBuffer);

        // Picture Y VME
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.psSurface = &m_refList[picIdx0]->sRefBuffer;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncForwardPic;
        surfaceCodecParams.ucVDirection = vDirection;
        surfaceCodecParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

#ifdef _MMC_SUPPORTED
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncForwardPic + 1;
#ifdef _MMC_SUPPORTED
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

    }

    // backward reference
    if (picIdx1 < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        if (m_verticalLineStride == CODECHAL_VLINESTRIDE_FIELD)
        {
            vDirection = (refL1BottomField ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
        }

        CodecHalGetResourceInfo(m_osInterface, &m_refList[picIdx1]->sRefBuffer);

        // Picture Y VME
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.psSurface = &m_refList[picIdx1]->sRefBuffer;
        surfaceCodecParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncBackwardPic;
        surfaceCodecParams.ucVDirection = vDirection;

#ifdef _MMC_SUPPORTED
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncBackwardPic + 1;
#ifdef _MMC_SUPPORTED
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Interlace Frame
    if ((CodecHal_PictureIsFrame(m_picParams->m_currOriginalPic)) &&
        (m_picParams->m_fieldCodingFlag || m_picParams->m_fieldFrameCodingFlag))
    {
        // Current Picture Interlace
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.psSurface = currPicSurface;
        surfaceCodecParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncInterlaceFrameCurrentPic;
        surfaceCodecParams.ucVDirection = vDirection;

#ifdef _MMC_SUPPORTED
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        if (picIdx1 < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
        {
            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.psSurface = &m_refList[picIdx1]->sRefBuffer;
            surfaceCodecParams.dwCacheabilityControl =
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncInterlaceFrameBackwardPic;
            surfaceCodecParams.ucVDirection = vDirection;

#ifdef _MMC_SUPPORTED
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncInterlaceFrameBackwardPic + 1;

#ifdef _MMC_SUPPORTED
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    // BRC distortion data buffer for I frame
    if (mbEncIFrameDistEnabled)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &m_brcBuffers.sMeBrcDistortionBuffer;
        surfaceCodecParams.dwOffset = m_brcBuffers.dwMeBrcDistortionBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncBrcDistortionSurface;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // MB-control surface for MB level QP, SkipEnable and NonSkipEnable
    if (m_mbQpDataEnabled)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &m_mbQpDataSurface;
        surfaceCodecParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MB_QP_CODEC].Value;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncMbControl;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (!m_useHwScoreboard &&
        m_pictureCodingType != I_TYPE)
    {
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

MOS_STATUS CodechalEncodeMpeg2G12::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    bool                        frameTracking,
    MHW_MI_MMIOREGISTERS       *mmioRegister)
{
    if (MOS_VE_SUPPORTED(m_osInterface) && cmdBuffer->Attributes.pAttriVe)
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
                (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);
        attriExt->bUseVirtualEngineHint = true;
        attriExt->VEngineHintParams.NeedSyncWithPrevious = 1;
    }

    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl = true;
    forceWakeupParams.bMFXPowerWellControlMask = true;
    forceWakeupParams.bHEVCPowerWellControl = false;
    forceWakeupParams.bHEVCPowerWellControlMask = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiForceWakeupCmd(
         cmdBuffer,
         &forceWakeupParams));

    return CodechalEncoderState::SendPrologWithFrameTracking(cmdBuffer, frameTracking, mmioRegister);
}

MOS_STATUS CodechalEncodeMpeg2G12::InitMmcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeMpeg2G12, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeMpeg2G12::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
        m_rawSurfaceToEnc,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "SrcSurf")));

    m_firstTaskInPhase = true;
    m_lastTaskInPhase = !m_singleTaskPhaseSupported;
    m_lastEncPhase = false;

    m_setRequestedEUSlices = (m_brcEnabled         &&
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
            CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT)));
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

void CodechalEncodeMpeg2G12::ResizeOnResChange()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncoderState::ResizeOnResChange();

    // need to re-allocate surfaces according to resolution
    m_swScoreboardState->ReleaseResources();
}

MOS_STATUS CodechalEncodeMpeg2G12::UpdateCmdBufAttribute(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                renderEngineInUse)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // should not be there. Will remove it in the next change
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (MOS_VE_SUPPORTED(m_osInterface) && cmdBuffer->Attributes.pAttriVe)
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
            (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);

        memset(attriExt, 0, sizeof(MOS_CMD_BUF_ATTRI_VE));
        attriExt->bUseVirtualEngineHint =
            attriExt->VEngineHintParams.NeedSyncWithPrevious = !renderEngineInUse;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2G12::AddMediaVfeCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    SendKernelCmdsParams *params)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MHW_VFE_PARAMS_G12 vfeParams = {};
    vfeParams.pKernelState              = params->pKernelState;
    vfeParams.eVfeSliceDisable          = MHW_VFE_SLICE_ALL;
    vfeParams.dwMaximumNumberofThreads  = m_encodeVfeMaxThreads;
    vfeParams.bFusedEuDispatch          = false; // legacy mode

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaVfeCmd(cmdBuffer, &vfeParams));

    return MOS_STATUS_SUCCESS;
}
