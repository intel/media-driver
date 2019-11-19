/*
* Copyright (c) 2011-2019, Intel Corporation
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
//! \file     codechal_encode_avc_g11.cpp
//! \brief    This file implements the C++ class/interface for Gen11 platform's AVC
//!           DualPipe encoding to be used across CODECHAL components.
//!

#include "codechal_encode_avc.h"
#include "codechal_encode_avc_g11.h"
#include "codechal_encode_wp_g11.h"
#include "codechal_kernel_header_g11.h"
#include "codechal_kernel_hme_g11.h"
#include "hal_oca_interface.h"
#ifndef _FULL_OPEN_SOURCE
#include "igcodeckrn_g11.h"
#endif
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g11.h"
#include "mhw_vdbox_mfx_hwcmd_g11_X.h"
#include "mos_util_user_interface.h"
#endif

static const uint32_t sfdOutputBufferSizeCommon = 128;
static const uint32_t sfdCostTableBufferSizeCommon = 52;
static const uint32_t seiBufferSIZE = 10240; // 10K is just = estimation;
static const uint32_t refThreshold = 400;
static const uint32_t brcConstantsurfaceEarlySkipTableSize = 128;
static const uint32_t brcConstantsurfaceModeMvCostSize = 1664;
static const uint32_t brcConstantsurfaceRefcostSize = 128;
static const uint32_t brcConstantsurfaceQpList0 = 32;
static const uint32_t brcConstantsurfaceQpList0Reserved = 32;
static const uint32_t brcConstantsurfaceQpList1 = 32;
static const uint32_t brcConstantsurfaceQpList1Reserved = 160;
static const uint32_t brcConstantsurfaceIntracostScalingFactor = 64;
static const uint32_t brcHistoryBufferSize = 880;
static const uint32_t brcConstantsurfaceWidth = 64;
static const uint32_t brcConstantsurfaceHeight = 53;
static const uint32_t brcConstantsurfaceLambdaSize = 512;;
static const uint32_t brcConstantsurfaceFtq25Size = 64;
static const uint32_t defaultTrellisQuantIntraRounding = 5;
static const uint32_t maxLambda = 0xEFFF;
static const uint32_t mbencCurbeSizeInDword = 89;
static const uint32_t mbencNumTargetUsages = 3;
static const uint32_t mbencBrcBufferSize = 128;
static const uint32_t mbTextureThreshold = 1024;
static const uint32_t adaptiveTxDecisionThreshold = 128;
static const uint32_t brcHistoryBufferOffsetSceneChanged = 0x2FC;

static const uint32_t trellisQuantizationRounding[NUM_TARGET_USAGE_MODES] =
{
    0, 3, 0, 0, 0, 0, 0, 0
};

enum MbencBindingTableOffset
{
    mbencMfcAvcPakObj =  0,
    mbencIndMvData =  1,
    mbencBrcDistortion =  2,    // For BRC distortion for I
    mbencCurrY =  3,
    mbencCurrUv =  4,
    mbencMbSpecificData =  5,
    mbencAuxVmeOut =  6,
    mbencRefpicselectL0 =  7,
    mbencMvDataFromMe =  8,
    mbenc4xMeDistortion =  9,
    mbencSlicemapData = 10,
    mbencFwdMbData = 11,
    mbencFwdMvData = 12,
    mbencMbqp = 13,
    mbencMbbrcConstData = 14,
    mbencVmeInterPredCurrPicIdx0 = 15,
    mbencVmeInterPredFwdPicIDX0 = 16,
    mbencVmeInterPredBwdPicIDX00 = 17,
    mbencVmeInterPredFwdPicIDX1 = 18,
    mbencVmeInterPredBwdPicIDX10 = 19,
    mbencVmeInterPredFwdPicIDX2 = 20,
    mbencReserved0 = 21,
    mbencVmeInterPredFwdPicIDX3 = 22,
    mbencReserved1 = 23,
    mbencVmeInterPredFwdPicIDX4 = 24,
    mbencReserved2 = 25,
    mbencVmeInterPredFwdPicIDX5 = 26,
    mbencReserved3 = 27,
    mbencVmeInterPredFwdPicIDX6 = 28,
    mbencReserved4 = 29,
    mbencVmeInterPredFwdPicIDX7 = 30,
    mbencReserved5 = 31,
    mbencVmeInterPredCurrPicIdx1 = 32,
    mbencVmeInterPredBwdPicIDX01 = 33,
    mbencReserved6 = 34,
    mbencVmeInterPredBwdPicIDX11 = 35,
    mbencReserved7 = 36,
    mbencMbStats = 37,
    mbencMadData = 38,
    mbencBrcCurbeData = 39,
    mbencForceNonskipMbMap = 40,
    mbEncAdv = 41,
    mbencSfdCostTable = 42,
    mbencSwScoreboard = 43,
    mbencNumSurfaces = 44
};

enum MbBrcUpdateBindingTableOffset
{
    mbBrcUpdateHistory = 0,
    mbBrcUpdateMbQp = 1,
    mbBrcUpdateRoi = 2,
    mbBrcUpdateMbStat = 3,
    mbBrcUpdateNumSurfaces = 4
};

enum WpBindingTableOffset
{
    wpInputRefSurface = 0,
    wpOutputScaledSurface = 1,
    wpNumSurfaces = 2
};

enum MbencIdOffset
{
    mbencIOffset = 0,
    mbencPOffset = 1,
    mbencBOffset = 2,
    mbencFrameTypeNum = 3
};

enum BrcUpdateBindingTableOffset
{
    frameBrcUpdateHistory = 0,
    frameBrcUpdatePakStatisticsOutput = 1,
    frameBrcUpdateImageStateRead = 2,
    frameBrcUpdateImageStateWrite = 3,
    frameBrcUpdateMbencCurbeWrite = 4,
    frameBrcUpdateDistortion = 5,
    frameBrcUpdateConstantData = 6,
    frameBrcUpdateMbStat = 7,
    frameBrcUpdateMvStat = 8,
    frameBrcUpdateNumSurfaces = 9
};

enum SfdBindingTableOffset
{
    sfdVdencInputImageState = 0,
    sfdMvDataSurface = 1,
    sfdInterDistortionSurface = 2,
    sfdOutputDataSurface = 3,
    sfdVdencOutputImageState = 4,
    sfdNumSurfaces = 5
};

// CURBE for Static Frame Detection kernel
class SfdCurbe
{
public:
    union
    {
        struct
        {
            uint32_t   VDEncModeDisable : MOS_BITFIELD_BIT(0);
            uint32_t   BRCModeEnable : MOS_BITFIELD_BIT(1);
            uint32_t   SliceType : MOS_BITFIELD_RANGE(2, 3);
            uint32_t:  MOS_BITFIELD_BIT(4);
            uint32_t   StreamInType : MOS_BITFIELD_RANGE(5, 8);
            uint32_t   EnableAdaptiveMvStreamIn : MOS_BITFIELD_BIT(9);
            uint32_t:  MOS_BITFIELD_BIT(10);
            uint32_t   EnableIntraCostScalingForStaticFrame : MOS_BITFIELD_BIT(11);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(12, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw0;

    union
    {
        struct
        {
            uint32_t   QPValue : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   NumOfRefs : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   HMEStreamInRefCost : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw1;

    union
    {
        struct
        {
            uint32_t   FrameWidthInMBs : MOS_BITFIELD_RANGE(0, 15);     // round-up to 4-MB aligned
            uint32_t   FrameHeightInMBs : MOS_BITFIELD_RANGE(16, 31);     // round-up to 4-MB aligned
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw2;

    union
    {
        struct
        {
            uint32_t   LargeMvThresh;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw3;

    union
    {
        struct
        {
            uint32_t   TotalLargeMvThreshold;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw4;

    union
    {
        struct
        {
            uint32_t   ZMVThreshold;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw5;

    union
    {
        struct
        {
            uint32_t   TotalZMVThreshold;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw6;

    union
    {
        struct
        {
            uint32_t   MinDistThreshold;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw7;

    uint8_t m_costTable[52];

    union
    {
        struct
        {
            uint32_t   ActualWidthInMB : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   ActualHeightInMB : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw21;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw22;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw23;

    union
    {
        struct
        {
            uint32_t   VDEncInputImagStateIndex;      // used in VDEnc CQP mode
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw24;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw25;

    union
    {
        struct
        {
            uint32_t   MVDataSurfaceIndex;      // contains HME MV Data generated by HME kernel
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw26;

    union
    {
        struct
        {
            uint32_t   InterDistortionSurfaceIndex;      // contains HME Inter Distortion generated by HME kernel
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw27;

    union
    {
        struct
        {
            uint32_t   OutputDataSurfaceIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw28;

    union
    {
        struct
        {
            uint32_t   VDEncOutputImagStateIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw29;

    SfdCurbe()
    {
        m_dw0.Value  = 0;
        m_dw1.Value  = 0;
        m_dw2.Value  = 0;
        m_dw3.Value  = 0;
        m_dw4.Value  = 0;
        m_dw5.Value  = 0;
        m_dw6.Value  = 0;
        m_dw21.Value = 0;
        m_dw22.Value = 0;
        m_dw23.Value = 0;
        m_dw24.Value = 0;
        m_dw25.Value = 0;
        m_dw26.Value = 0;
        m_dw27.Value = 0;
        m_dw28.Value = 0;
        m_dw29.Value = 0;

        for (uint8_t i = 0; i < 52; i++)
            m_costTable[i] = 0;
    };
};

class MbencCurbe
{
public:
    // DW0
    union
    {
        struct
        {
            uint32_t   SkipModeEn                          : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                          : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                            : MOS_BITFIELD_BIT(       2 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn                   : MOS_BITFIELD_BIT(       5 );
            uint32_t                                       : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn                  : MOS_BITFIELD_BIT(       7 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                        : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw0;

    // DW1
    union
    {
        struct
        {
            uint32_t   MaxNumMVs                           : MOS_BITFIELD_RANGE(  0, 5 );
            uint32_t   ExtendedMvCostRange                 : MOS_BITFIELD_BIT(       6 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  7,15 );
            uint32_t   BiWeight                            : MOS_BITFIELD_RANGE( 16,21 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 22,27 );
            uint32_t   UniMixDisable                       : MOS_BITFIELD_BIT(      28 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 29,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw1;

    // DW2
    union
    {
        struct
        {
            uint32_t   LenSP                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   PicWidth                            : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw2;

    // DW3
    union
    {
        struct
        {
            uint32_t   SrcSize                             : MOS_BITFIELD_RANGE(  0, 1 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  2, 3 );
            uint32_t   MbTypeRemap                         : MOS_BITFIELD_RANGE(  4, 5 );
            uint32_t   SrcAccess                           : MOS_BITFIELD_BIT(       6 );
            uint32_t   RefAccess                           : MOS_BITFIELD_BIT(       7 );
            uint32_t   SearchCtrl                          : MOS_BITFIELD_RANGE(  8,10 );
            uint32_t   DualSearchPathOption                : MOS_BITFIELD_BIT(      11 );
            uint32_t   SubPelMode                          : MOS_BITFIELD_RANGE( 12,13 );
            uint32_t   SkipType                            : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableFieldCacheAlloc              : MOS_BITFIELD_BIT(      15 );
            uint32_t   InterChromaMode                     : MOS_BITFIELD_BIT(      16 );
            uint32_t   FTEnable                            : MOS_BITFIELD_BIT(      17 );
            uint32_t   BMEDisableFBR                       : MOS_BITFIELD_BIT(      18 );
            uint32_t   BlockBasedSkipEnable                : MOS_BITFIELD_BIT(      19 );
            uint32_t   InterSAD                            : MOS_BITFIELD_RANGE( 20,21 );
            uint32_t   IntraSAD                            : MOS_BITFIELD_RANGE( 22,23 );
            uint32_t   SubMbPartMask                       : MOS_BITFIELD_RANGE( 24,30 );
            uint32_t                                       : MOS_BITFIELD_BIT(      31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw3;

    // DW4
    union
    {
        struct
        {
            uint32_t   PicHeightMinus1                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MvRestrictionInSliceEnable          : MOS_BITFIELD_BIT(      16 );
            uint32_t   DeltaMvEnable                       : MOS_BITFIELD_BIT(      17 );
            uint32_t   TrueDistortionEnable                : MOS_BITFIELD_BIT(      18 );
            uint32_t   EnableWavefrontOptimization         : MOS_BITFIELD_BIT(      19 );
            uint32_t   EnableFBRBypass                     : MOS_BITFIELD_BIT(      20 );
            uint32_t   EnableIntraCostScalingForStaticFrame: MOS_BITFIELD_BIT(      21 );
            uint32_t   EnableIntraRefresh                  : MOS_BITFIELD_BIT(      22 );
            uint32_t   Reserved                            : MOS_BITFIELD_BIT(      23 );
            uint32_t   EnableDirtyRect                     : MOS_BITFIELD_BIT(      24 );
            uint32_t   bCurFldIDR                          : MOS_BITFIELD_BIT(      25 );
            uint32_t   ConstrainedIntraPredFlag            : MOS_BITFIELD_BIT(      26 );
            uint32_t   FieldParityFlag                     : MOS_BITFIELD_BIT(      27 );
            uint32_t   HMEEnable                           : MOS_BITFIELD_BIT(      28 );
            uint32_t   PictureType                         : MOS_BITFIELD_RANGE( 29,30 );
            uint32_t   UseActualRefQPValue                 : MOS_BITFIELD_BIT(      31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw4;

    // DW5
    union
    {
        struct
        {
            uint32_t   SliceMbHeight                       : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   RefWidth                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                           : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw5;

    // DW6
    union
    {
        struct
        {
            uint32_t   BatchBufferEnd;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw6;

    // DW7
    union
    {
        struct
        {
            uint32_t   IntraPartMask                       : MOS_BITFIELD_RANGE(  0, 4 );
            uint32_t   NonSkipZMvAdded                     : MOS_BITFIELD_BIT(       5 );
            uint32_t   NonSkipModeAdded                    : MOS_BITFIELD_BIT(       6 );
            uint32_t   LumaIntraSrcCornerSwap              : MOS_BITFIELD_BIT(       7 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MVCostScaleFactor                   : MOS_BITFIELD_RANGE( 16,17 );
            uint32_t   BilinearEnable                      : MOS_BITFIELD_BIT(      18 );
            uint32_t   SrcFieldPolarity                    : MOS_BITFIELD_BIT(      19 );
            uint32_t   WeightedSADHAAR                     : MOS_BITFIELD_BIT(      20 );
            uint32_t   AConlyHAAR                          : MOS_BITFIELD_BIT(      21 );
            uint32_t   RefIDCostMode                       : MOS_BITFIELD_BIT(      22 );
            uint32_t                                       : MOS_BITFIELD_BIT(      23 );
            uint32_t   SkipCenterMask                      : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw7;

    struct
    {
        // DW8
        union
        {
            struct
            {
                uint32_t   Mode0Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   Mode1Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   Mode2Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   Mode3Cost : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW8;

        // DW9
        union
        {
            struct
            {
                uint32_t   Mode4Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   Mode5Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   Mode6Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   Mode7Cost : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW9;

        // DW10
        union
        {
            struct
            {
                uint32_t   Mode8Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   Mode9Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   RefIDCost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   ChromaIntraModeCost : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW10;

        // DW11
        union
        {
            struct
            {
                uint32_t   MV0Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   MV1Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   MV2Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   MV3Cost : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW11;

        // DW12
        union
        {
            struct
            {
                uint32_t   MV4Cost : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   MV5Cost : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   MV6Cost : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   MV7Cost : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW12;

        // DW13
        union
        {
            struct
            {
                uint32_t   QpPrimeY : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   QpPrimeCb : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   QpPrimeCr : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   TargetSizeInWord : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW13;

        // DW14
        union
        {
            struct
            {
                uint32_t   SICFwdTransCoeffThreshold_0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   SICFwdTransCoeffThreshold_1 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   SICFwdTransCoeffThreshold_2 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW14;

        // DW15
        union
        {
            struct
            {
                uint32_t   SICFwdTransCoeffThreshold_3 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   SICFwdTransCoeffThreshold_4 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   SICFwdTransCoeffThreshold_5 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   SICFwdTransCoeffThreshold_6 : MOS_BITFIELD_RANGE(24, 31);    // Highest Freq
            };
            struct
            {
                uint32_t   Value;
            };
        } DW15;
    } m_modeMvCost;

    struct
    {
        // DW16
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_0;
                SearchPathDelta   SPDelta_1;
                SearchPathDelta   SPDelta_2;
                SearchPathDelta   SPDelta_3;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW16;

        // DW17
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_4;
                SearchPathDelta   SPDelta_5;
                SearchPathDelta   SPDelta_6;
                SearchPathDelta   SPDelta_7;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW17;

        // DW18
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_8;
                SearchPathDelta   SPDelta_9;
                SearchPathDelta   SPDelta_10;
                SearchPathDelta   SPDelta_11;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW18;

        // DW19
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_12;
                SearchPathDelta   SPDelta_13;
                SearchPathDelta   SPDelta_14;
                SearchPathDelta   SPDelta_15;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW19;

        // DW20
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_16;
                SearchPathDelta   SPDelta_17;
                SearchPathDelta   SPDelta_18;
                SearchPathDelta   SPDelta_19;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW20;

        // DW21
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_20;
                SearchPathDelta   SPDelta_21;
                SearchPathDelta   SPDelta_22;
                SearchPathDelta   SPDelta_23;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW21;

        // DW22
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_24;
                SearchPathDelta   SPDelta_25;
                SearchPathDelta   SPDelta_26;
                SearchPathDelta   SPDelta_27;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW22;

        // DW23
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_28;
                SearchPathDelta   SPDelta_29;
                SearchPathDelta   SPDelta_30;
                SearchPathDelta   SPDelta_31;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW23;

        // DW24
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_32;
                SearchPathDelta   SPDelta_33;
                SearchPathDelta   SPDelta_34;
                SearchPathDelta   SPDelta_35;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW24;

        // DW25
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_36;
                SearchPathDelta   SPDelta_37;
                SearchPathDelta   SPDelta_38;
                SearchPathDelta   SPDelta_39;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW25;

        // DW26
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_40;
                SearchPathDelta   SPDelta_41;
                SearchPathDelta   SPDelta_42;
                SearchPathDelta   SPDelta_43;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW26;

        // DW27
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_44;
                SearchPathDelta   SPDelta_45;
                SearchPathDelta   SPDelta_46;
                SearchPathDelta   SPDelta_47;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW27;

        // DW28
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_48;
                SearchPathDelta   SPDelta_49;
                SearchPathDelta   SPDelta_50;
                SearchPathDelta   SPDelta_51;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW28;

        // DW29
        union
        {
            struct
            {
                SearchPathDelta   SPDelta_52;
                SearchPathDelta   SPDelta_53;
                SearchPathDelta   SPDelta_54;
                SearchPathDelta   SPDelta_55;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW29;

        // DW30
        union
        {
            struct
            {
                uint32_t   Intra4x4ModeMask : MOS_BITFIELD_RANGE(0, 8);
            uint32_t: MOS_BITFIELD_RANGE(9, 15);
                uint32_t   Intra8x8ModeMask : MOS_BITFIELD_RANGE(16, 24);
            uint32_t: MOS_BITFIELD_RANGE(25, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW30;

        // DW31
        union
        {
            struct
            {
                uint32_t   Intra16x16ModeMask : MOS_BITFIELD_RANGE(0, 3);
                uint32_t   IntraChromaModeMask : MOS_BITFIELD_RANGE(4, 7);
                uint32_t   IntraComputeType : MOS_BITFIELD_RANGE(8, 9);
            uint32_t: MOS_BITFIELD_RANGE(10, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW31;
    } m_spDelta;

    // DW32
    union
    {
        struct
        {
            uint32_t   SkipVal                             : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   MultiPredL0Disable                  : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   MultiPredL1Disable                  : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw32;

    // DW33
    union
    {
        struct
        {
            uint32_t   Intra16x16NonDCPredPenalty          : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Intra8x8NonDCPredPenalty            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   Intra4x4NonDCPredPenalty            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw33;

    // DW34
    union
    {
        struct
        {
            uint32_t   List0RefID0FieldParity              : MOS_BITFIELD_BIT(       0 );
            uint32_t   List0RefID1FieldParity              : MOS_BITFIELD_BIT(       1 );
            uint32_t   List0RefID2FieldParity              : MOS_BITFIELD_BIT(       2 );
            uint32_t   List0RefID3FieldParity              : MOS_BITFIELD_BIT(       3 );
            uint32_t   List0RefID4FieldParity              : MOS_BITFIELD_BIT(       4 );
            uint32_t   List0RefID5FieldParity              : MOS_BITFIELD_BIT(       5 );
            uint32_t   List0RefID6FieldParity              : MOS_BITFIELD_BIT(       6 );
            uint32_t   List0RefID7FieldParity              : MOS_BITFIELD_BIT(       7 );
            uint32_t   List1RefID0FrameFieldFlag           : MOS_BITFIELD_BIT(       8 );
            uint32_t   List1RefID1FrameFieldFlag           : MOS_BITFIELD_BIT(       9 );
            uint32_t   IntraRefreshEn                      : MOS_BITFIELD_RANGE( 10,11 );
            uint32_t   ArbitraryNumMbsPerSlice             : MOS_BITFIELD_BIT(      12 );
            uint32_t   TQEnable                            : MOS_BITFIELD_BIT(      13 );
            uint32_t   ForceNonSkipMbEnable                : MOS_BITFIELD_BIT(      14 );
            uint32_t   DisableEncSkipCheck                 : MOS_BITFIELD_BIT(      15 );
            uint32_t   EnableDirectBiasAdjustment          : MOS_BITFIELD_BIT(      16 );
            uint32_t   bForceToSkip                        : MOS_BITFIELD_BIT(      17 );
            uint32_t   EnableGlobalMotionBiasAdjustment    : MOS_BITFIELD_BIT(      18 );
            uint32_t   EnableAdaptiveTxDecision            : MOS_BITFIELD_BIT(      19 );
            uint32_t   EnablePerMBStaticCheck              : MOS_BITFIELD_BIT(      20 );
            uint32_t   EnableAdaptiveSearchWindowSize      : MOS_BITFIELD_BIT(      21 );
            uint32_t   RemoveIntraRefreshOverlap           : MOS_BITFIELD_BIT(      22 );
            uint32_t   CQPFlag                             : MOS_BITFIELD_BIT(      23 );
            uint32_t   List1RefID0FieldParity              : MOS_BITFIELD_BIT(      24 );
            uint32_t   List1RefID1FieldParity              : MOS_BITFIELD_BIT(      25 );
            uint32_t   MADEnableFlag                       : MOS_BITFIELD_BIT(      26 );
            uint32_t   ROIEnableFlag                       : MOS_BITFIELD_BIT(      27 );
            uint32_t   EnableMBFlatnessChkOptimization     : MOS_BITFIELD_BIT(      28 );
            uint32_t   bDirectMode                         : MOS_BITFIELD_BIT(      29 );
            uint32_t   MBBrcEnable                         : MOS_BITFIELD_BIT(      30 );
            uint32_t   bOriginalBff                        : MOS_BITFIELD_BIT(      31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw34;

    // DW35
    union
    {
        struct
        {
            uint32_t   PanicModeMBThreshold                : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   SmallMbSizeInWord                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   LargeMbSizeInWord                   : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw35;

    // DW36
    union
    {
        struct
        {
            uint32_t   NumRefIdxL0MinusOne                 : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMECombinedExtraSUs                 : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   NumRefIdxL1MinusOne                 : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t                                       : MOS_BITFIELD_RANGE( 24,26 );
            uint32_t   MBInputEnable                       : MOS_BITFIELD_BIT(      27 );
            uint32_t   IsFwdFrameShortTermRef              : MOS_BITFIELD_BIT(      28 );
            uint32_t   CheckAllFractionalEnable            : MOS_BITFIELD_BIT(      29 );
            uint32_t   HMECombineOverlap                   : MOS_BITFIELD_RANGE( 30,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw36;

    // DW37
    union
    {
        struct
        {
            uint32_t   SkipModeEn                          : MOS_BITFIELD_BIT(       0 );
            uint32_t   AdaptiveEn                          : MOS_BITFIELD_BIT(       1 );
            uint32_t   BiMixDis                            : MOS_BITFIELD_BIT(       2 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  3, 4 );
            uint32_t   EarlyImeSuccessEn                   : MOS_BITFIELD_BIT(       5 );
            uint32_t                                       : MOS_BITFIELD_BIT(       6 );
            uint32_t   T8x8FlagForInterEn                  : MOS_BITFIELD_BIT(       7 );
            uint32_t                                       : MOS_BITFIELD_RANGE(  8,23 );
            uint32_t   EarlyImeStop                        : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw37;

    // DW38
    union
    {
        struct
        {
            uint32_t   LenSP                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MaxNumSU                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefThreshold                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw38;

    // DW39
    union
    {
        struct
        {
            uint32_t                                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   HMERefWindowsCombThreshold          : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefWidth                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RefHeight                           : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw39;

    // DW40
    union
    {
        struct
        {
            uint32_t   DistScaleFactorRefID0List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID1List0          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw40;

    // DW41
    union
    {
        struct
        {
            uint32_t   DistScaleFactorRefID2List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID3List0          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw41;

    // DW42
    union
    {
        struct
        {
            uint32_t   DistScaleFactorRefID4List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID5List0          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw42;

    // DW43
    union
    {
        struct
        {
            uint32_t   DistScaleFactorRefID6List0          : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   DistScaleFactorRefID7List0          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw43;

    // DW44
    union
    {
        struct
        {
            uint32_t   ActualQPValueForRefID0List0         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List0         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID2List0         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID3List0         : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw44;

    // DW45
    union
    {
        struct
        {
            uint32_t   ActualQPValueForRefID4List0         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID5List0         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ActualQPValueForRefID6List0         : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ActualQPValueForRefID7List0         : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw45;

    // DW46
    union
    {
        struct
        {
            uint32_t   ActualQPValueForRefID0List1         : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ActualQPValueForRefID1List1         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RefCost                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw46;

    // DW47
    union
    {
        struct
        {
            uint32_t   MbQpReadFactor                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IntraCostSF                         : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxVmvR                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw47;

    //DW48
    union
    {
        struct
        {
            uint32_t   IntraRefreshMBx                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IntraRefreshUnitInMBMinus1          : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IntraRefreshQPDelta                 : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw48;

    // DW49
    union
    {
        struct
        {
            uint32_t   ROI1_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw49;

    // DW50
    union
    {
        struct
        {
            uint32_t   ROI1_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI1_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw50;

    // DW51
    union
    {
        struct
        {
            uint32_t   ROI2_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw51;

    // DW52
    union
    {
        struct
        {
            uint32_t   ROI2_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI2_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw52;

    // DW53
    union
    {
        struct
        {
            uint32_t   ROI3_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw53;

    // DW54
    union
    {
        struct
        {
            uint32_t   ROI3_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI3_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw54;

    // DW55
    union
    {
        struct
        {
            uint32_t   ROI4_X_left                         : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_top                          : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw55;

    // DW56
    union
    {
        struct
        {
            uint32_t   ROI4_X_right                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   ROI4_Y_bottom                       : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw56;

    // DW57
    union
    {
        struct
        {
            uint32_t   ROI1_dQpPrimeY                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   ROI2_dQpPrimeY                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ROI3_dQpPrimeY                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   ROI4_dQpPrimeY                      : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw57;

    // DW58
    union
    {
        struct
        {
            uint32_t   Lambda_8x8Inter                     : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   Lambda_8x8Intra                     : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw58;

    // DW59
    union
    {
        struct
        {
            uint32_t   Lambda_Inter                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   Lambda_Intra                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw59;

    // DW60
    union
    {
        struct
        {
            uint32_t   MBTextureThreshold                  : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   TxDecisonThreshold                  : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw60;

    // DW61
    union
    {
        struct
        {
            uint32_t   HMEMVCostScalingFactor              : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IntraRefreshMBy                     : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw61;

    // DW62
    union
    {
        struct
        {
            uint32_t   IPCM_QP0                            : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   IPCM_QP1                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_QP2                            : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   IPCM_QP3                            : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw62;

    // DW63
    union
    {
        struct
        {
            uint32_t   IPCM_QP4                            : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                            : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   IPCM_Thresh0                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw63;

    // DW64
    union
    {
        struct
        {
            uint32_t   IPCM_Thresh1                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh2                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw64;

    // DW65
    union
    {
        struct
        {
            uint32_t   IPCM_Thresh3                        : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   IPCM_Thresh4                        : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw65;

    // DW66
    union
    {
        struct
        {
            uint32_t   MBDataSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw66;

    // DW67
    union
    {
        struct
        {
            uint32_t   MVDataSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw67;

    // DW68
    union
    {
        struct
        {
            uint32_t   IDistSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw68;

    // DW69
    union
    {
        struct
        {
            uint32_t   SrcYSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw69;

    // DW70
    union
    {
        struct
        {
            uint32_t   MBSpecificDataSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw70;

    // DW71
    union
    {
        struct
        {
            uint32_t   AuxVmeOutSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw71;

    // DW72
    union
    {
        struct
        {
            uint32_t   CurrRefPicSelSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw72;

    // DW73
    union
    {
        struct
        {
            uint32_t   HMEMVPredFwdBwdSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw73;

    // DW74
    union
    {
        struct
        {
            uint32_t   HMEDistSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw74;

    // DW75
    union
    {
        struct
        {
            uint32_t   SliceMapSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw75;

    // DW76
    union
    {
        struct
        {
            uint32_t   FwdFrmMBDataSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw76;

    // DW77
    union
    {
        struct
        {
            uint32_t   FwdFrmMVSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw77;

    // DW78
    union
    {
        struct
        {
            uint32_t   MBQPBuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw78;

    // DW79
    union
    {
        struct
        {
            uint32_t   MBBRCLut;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw79;

    // DW80
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw80;

    // DW81
    union
    {
        struct
        {
            uint32_t   VMEInterPredictionMRSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw81;

    // DW82
    union
    {
        struct
        {
            uint32_t   MbStatsSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw82;

    // DW83
    union
    {
        struct
        {
            uint32_t   MADSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw83;

    // DW84
    union
    {
        struct
        {
            uint32_t   BRCCurbeSurfIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw84;

    // DW85
    union
    {
        struct
        {
            uint32_t   ForceNonSkipMBmapSurface;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw85;

    // DW86
    union
    {
        struct
        {
            uint32_t   ReservedIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw86;

    // DW87
    union
    {
        struct
        {
            uint32_t   StaticDetectionCostTableIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw87;

    // DW88
    union
    {
        struct
        {
            uint32_t   SWScoreboardIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw88;

    enum MBEncCurbeInitType
    {
        IDist,
        IFrame,
        IField,
        PFrame,
        PField,
        BFrame,
        BField
    };

    MbencCurbe(MBEncCurbeInitType initType)
    {
        switch (initType)
        {
        case IDist:
            m_dw0.Value             = 0x00000082;
            m_dw1.Value             = 0x00200008;
            m_dw2.Value             = 0x001e3910;
            m_dw3.Value             = 0x00a83000;
            m_dw4.Value             = 0x90000000;
            m_dw5.Value             = 0x28300000;
            m_dw6.Value             = 0x00000000;
            m_dw7.Value             = 0x00000000;
            m_modeMvCost.DW8.Value  = 0x00000000;
            m_modeMvCost.DW9.Value  = 0x00000000;
            m_modeMvCost.DW10.Value = 0x00000000;
            m_modeMvCost.DW11.Value = 0x00000000;
            m_modeMvCost.DW12.Value = 0x00000000;
            m_modeMvCost.DW13.Value = 0xff000000;
            m_modeMvCost.DW14.Value = 0x00000000;
            m_modeMvCost.DW15.Value = 0x00000000;
            m_spDelta.DW16.Value    = 0x00000000;
            m_spDelta.DW17.Value    = 0x00000000;
            m_spDelta.DW18.Value    = 0x00000000;
            m_spDelta.DW19.Value    = 0x00000000;
            m_spDelta.DW20.Value    = 0x00000000;
            m_spDelta.DW21.Value    = 0x00000000;
            m_spDelta.DW22.Value    = 0x00000000;
            m_spDelta.DW23.Value    = 0x00000000;
            m_spDelta.DW24.Value    = 0x00000000;
            m_spDelta.DW25.Value    = 0x00000000;
            m_spDelta.DW26.Value    = 0x00000000;
            m_spDelta.DW27.Value    = 0x00000000;
            m_spDelta.DW28.Value    = 0x00000000;
            m_spDelta.DW29.Value    = 0x00000000;
            m_spDelta.DW30.Value    = 0x00000000;
            m_spDelta.DW31.Value    = 0x00000100;
            m_dw32.Value            = 0x80800000;
            m_dw33.Value            = 0x00000000;
            m_dw34.Value            = 0x00000800;
            m_dw35.Value            = 0xffff00ff;
            m_dw36.Value            = 0x40000000;
            m_dw37.Value            = 0x00000080;
            m_dw38.Value            = 0x00003900;
            m_dw39.Value            = 0x28300000;
            m_dw40.Value            = 0x00000000;
            m_dw41.Value            = 0x00000000;
            m_dw42.Value            = 0x00000000;
            m_dw43.Value            = 0x00000000;
            m_dw44.Value            = 0x00000000;
            m_dw45.Value            = 0x00000000;
            m_dw46.Value            = 0x00000000;
            m_dw47.Value            = 0x00000000;
            m_dw48.Value            = 0x00000000;
            m_dw49.Value            = 0x00000000;
            m_dw50.Value            = 0x00000000;
            m_dw51.Value            = 0x00000000;
            m_dw52.Value            = 0x00000000;
            m_dw53.Value            = 0x00000000;
            m_dw54.Value            = 0x00000000;
            m_dw55.Value            = 0x00000000;
            m_dw56.Value            = 0x00000000;
            m_dw57.Value            = 0x00000000;
            m_dw58.Value            = 0x00000000;
            m_dw59.Value            = 0x00000000;
            m_dw60.Value            = 0x00000000;
            m_dw61.Value            = 0xffffffff;
            m_dw62.Value            = 0xffffffff;
            m_dw63.Value            = 0xffffffff;
            m_dw64.Value            = 0xffffffff;
            m_dw65.Value            = 0xffffffff;
            m_dw66.Value            = 0xffffffff;
            m_dw67.Value            = 0xffffffff;
            m_dw68.Value            = 0xffffffff;
            m_dw69.Value            = 0xffffffff;
            m_dw70.Value            = 0xffffffff;
            m_dw71.Value            = 0xffffffff;
            m_dw72.Value            = 0xffffffff;
            m_dw73.Value            = 0xffffffff;
            m_dw74.Value            = 0xffffffff;
            m_dw75.Value            = 0xffffffff;
            m_dw76.Value            = 0xffffffff;
            m_dw77.Value            = 0xffffffff;
            m_dw78.Value            = 0xffffffff;
            m_dw79.Value            = 0xffffffff;
            m_dw80.Value            = 0x00000000;
            m_dw81.Value            = 0x00000000;
            m_dw82.Value            = 0x00000000;
            m_dw83.Value            = 0x00000000;
            m_dw84.Value            = 0x00000000;
            m_dw85.Value            = 0x00000000;
            m_dw86.Value            = 0x00000000;
            m_dw87.Value            = 0x00000000;
            m_dw88.Value            = 0xffffffff;
            break;
        case IFrame:
            m_dw0.Value             = 0x00000082;
            m_dw1.Value             = 0x00000000;
            m_dw2.Value             = 0x00003910;
            m_dw3.Value             = 0x00a83000;
            m_dw4.Value             = 0x00000000;
            m_dw5.Value             = 0x28300000;
            m_dw6.Value             = 0x05000000;
            m_dw7.Value             = 0x00000000;
            m_modeMvCost.DW8.Value  = 0x00000000;
            m_modeMvCost.DW9.Value  = 0x00000000;
            m_modeMvCost.DW10.Value = 0x00000000;
            m_modeMvCost.DW11.Value = 0x00000000;
            m_modeMvCost.DW12.Value = 0x00000000;
            m_modeMvCost.DW13.Value = 0x00000000;
            m_modeMvCost.DW14.Value = 0x00000000;
            m_modeMvCost.DW15.Value = 0x00000000;
            m_spDelta.DW16.Value    = 0x00000000;
            m_spDelta.DW17.Value    = 0x00000000;
            m_spDelta.DW18.Value    = 0x00000000;
            m_spDelta.DW19.Value    = 0x00000000;
            m_spDelta.DW20.Value    = 0x00000000;
            m_spDelta.DW21.Value    = 0x00000000;
            m_spDelta.DW22.Value    = 0x00000000;
            m_spDelta.DW23.Value    = 0x00000000;
            m_spDelta.DW24.Value    = 0x00000000;
            m_spDelta.DW25.Value    = 0x00000000;
            m_spDelta.DW26.Value    = 0x00000000;
            m_spDelta.DW27.Value    = 0x00000000;
            m_spDelta.DW28.Value    = 0x00000000;
            m_spDelta.DW29.Value    = 0x00000000;
            m_spDelta.DW30.Value    = 0x00000000;
            m_spDelta.DW31.Value    = 0x00000000;
            m_dw32.Value            = 0x80800000;
            m_dw33.Value            = 0x00040c24;
            m_dw34.Value            = 0x00000000;
            m_dw35.Value            = 0xffff00ff;
            m_dw36.Value            = 0x40000000;
            m_dw37.Value            = 0x00000080;
            m_dw38.Value            = 0x00003900;
            m_dw39.Value            = 0x28300000;
            m_dw40.Value            = 0x00000000;
            m_dw41.Value            = 0x00000000;
            m_dw42.Value            = 0x00000000;
            m_dw43.Value            = 0x00000000;
            m_dw44.Value            = 0x00000000;
            m_dw45.Value            = 0x00000000;
            m_dw46.Value            = 0x00000000;
            m_dw47.Value            = 0x00000002;
            m_dw48.Value            = 0x00000000;
            m_dw49.Value            = 0x00000000;
            m_dw50.Value            = 0x00000000;
            m_dw51.Value            = 0x00000000;
            m_dw52.Value            = 0x00000000;
            m_dw53.Value            = 0x00000000;
            m_dw54.Value            = 0x00000000;
            m_dw55.Value            = 0x00000000;
            m_dw56.Value            = 0x00000000;
            m_dw57.Value            = 0x00000000;
            m_dw58.Value            = 0x00000000;
            m_dw59.Value            = 0x00000000;
            m_dw60.Value            = 0x00000000;
            m_dw61.Value            = 0x00000000;
            m_dw62.Value            = 0x00000000;
            m_dw63.Value            = 0x00000000;
            m_dw64.Value            = 0x00000000;
            m_dw65.Value            = 0x00000000;
            m_dw66.Value            = 0xffffffff;
            m_dw67.Value            = 0xffffffff;
            m_dw68.Value            = 0xffffffff;
            m_dw69.Value            = 0xffffffff;
            m_dw70.Value            = 0xffffffff;
            m_dw71.Value            = 0xffffffff;
            m_dw72.Value            = 0x00000000;
            m_dw73.Value            = 0x00000000;
            m_dw74.Value            = 0x00000000;
            m_dw75.Value            = 0xffffffff;
            m_dw76.Value            = 0x00000000;
            m_dw77.Value            = 0x00000000;
            m_dw78.Value            = 0xffffffff;
            m_dw79.Value            = 0xffffffff;
            m_dw80.Value            = 0xffffffff;
            m_dw81.Value            = 0xffffffff;
            m_dw82.Value            = 0xffffffff;
            m_dw83.Value            = 0xffffffff;
            m_dw84.Value            = 0xffffffff;
            m_dw85.Value            = 0x00000000;
            m_dw86.Value            = 0x00000000;
            m_dw87.Value            = 0x00000000;
            m_dw88.Value            = 0xffffffff;
            break;
        case IField:
            m_dw0.Value             = 0x00000082;
            m_dw1.Value             = 0x00000000;
            m_dw2.Value             = 0x00003910;
            m_dw3.Value             = 0x00a830c0;
            m_dw4.Value             = 0x02000000;
            m_dw5.Value             = 0x28300000;
            m_dw6.Value             = 0x05000000;
            m_dw7.Value             = 0x00000000;
            m_modeMvCost.DW8.Value  = 0x00000000;
            m_modeMvCost.DW9.Value  = 0x00000000;
            m_modeMvCost.DW10.Value = 0x00000000;
            m_modeMvCost.DW11.Value = 0x00000000;
            m_modeMvCost.DW12.Value = 0x00000000;
            m_modeMvCost.DW13.Value = 0x00000000;
            m_modeMvCost.DW14.Value = 0x00000000;
            m_modeMvCost.DW15.Value = 0x00000000;
            m_spDelta.DW16.Value    = 0x00000000;
            m_spDelta.DW17.Value    = 0x00000000;
            m_spDelta.DW18.Value    = 0x00000000;
            m_spDelta.DW19.Value    = 0x00000000;
            m_spDelta.DW20.Value    = 0x00000000;
            m_spDelta.DW21.Value    = 0x00000000;
            m_spDelta.DW22.Value    = 0x00000000;
            m_spDelta.DW23.Value    = 0x00000000;
            m_spDelta.DW24.Value    = 0x00000000;
            m_spDelta.DW25.Value    = 0x00000000;
            m_spDelta.DW26.Value    = 0x00000000;
            m_spDelta.DW27.Value    = 0x00000000;
            m_spDelta.DW28.Value    = 0x00000000;
            m_spDelta.DW29.Value    = 0x00000000;
            m_spDelta.DW30.Value    = 0x00000000;
            m_spDelta.DW31.Value    = 0x00000000;
            m_dw32.Value            = 0x80800000;
            m_dw33.Value            = 0x00040c24;
            m_dw34.Value            = 0x00000000;
            m_dw35.Value            = 0xffff00ff;
            m_dw36.Value            = 0x40000000;
            m_dw37.Value            = 0x00000080;
            m_dw38.Value            = 0x00003900;
            m_dw39.Value            = 0x28300000;
            m_dw40.Value            = 0x00000000;
            m_dw41.Value            = 0x00000000;
            m_dw42.Value            = 0x00000000;
            m_dw43.Value            = 0x00000000;
            m_dw44.Value            = 0x00000000;
            m_dw45.Value            = 0x00000000;
            m_dw46.Value            = 0x00000000;
            m_dw47.Value            = 0x00000002;
            m_dw48.Value            = 0x00000000;
            m_dw49.Value            = 0x00000000;
            m_dw50.Value            = 0x00000000;
            m_dw51.Value            = 0x00000000;
            m_dw52.Value            = 0x00000000;
            m_dw53.Value            = 0x00000000;
            m_dw54.Value            = 0x00000000;
            m_dw55.Value            = 0x00000000;
            m_dw56.Value            = 0x00000000;
            m_dw57.Value            = 0x00000000;
            m_dw58.Value            = 0x00000000;
            m_dw59.Value            = 0x00000000;
            m_dw60.Value            = 0x00000000;
            m_dw61.Value            = 0x00000000;
            m_dw62.Value            = 0x00000000;
            m_dw63.Value            = 0x00000000;
            m_dw64.Value            = 0x00000000;
            m_dw65.Value            = 0x00000000;
            m_dw66.Value            = 0xffffffff;
            m_dw67.Value            = 0xffffffff;
            m_dw68.Value            = 0xffffffff;
            m_dw69.Value            = 0xffffffff;
            m_dw70.Value            = 0xffffffff;
            m_dw71.Value            = 0xffffffff;
            m_dw72.Value            = 0x00000000;
            m_dw73.Value            = 0x00000000;
            m_dw74.Value            = 0x00000000;
            m_dw75.Value            = 0xffffffff;
            m_dw76.Value            = 0x00000000;
            m_dw77.Value            = 0x00000000;
            m_dw78.Value            = 0xffffffff;
            m_dw79.Value            = 0xffffffff;
            m_dw80.Value            = 0xffffffff;
            m_dw81.Value            = 0xffffffff;
            m_dw82.Value            = 0xffffffff;
            m_dw83.Value            = 0xffffffff;
            m_dw84.Value            = 0xffffffff;
            m_dw85.Value            = 0x00000000;
            m_dw86.Value            = 0x00000000;
            m_dw87.Value            = 0x00000000;
            m_dw88.Value            = 0xffffffff;
            break;
        case PFrame:
            m_dw0.Value             = 0x000000a3;
            m_dw1.Value             = 0x00000008;
            m_dw2.Value             = 0x00003910;
            m_dw3.Value             = 0x00ae3000;
            m_dw4.Value             = 0x30000000;
            m_dw5.Value             = 0x28300000;
            m_dw6.Value             = 0x05000000;
            m_dw7.Value             = 0x01400060;
            m_modeMvCost.DW8.Value  = 0x00000000;
            m_modeMvCost.DW9.Value  = 0x00000000;
            m_modeMvCost.DW10.Value = 0x00000000;
            m_modeMvCost.DW11.Value = 0x00000000;
            m_modeMvCost.DW12.Value = 0x00000000;
            m_modeMvCost.DW13.Value = 0x00000000;
            m_modeMvCost.DW14.Value = 0x00000000;
            m_modeMvCost.DW15.Value = 0x00000000;
            m_spDelta.DW16.Value    = 0x00000000;
            m_spDelta.DW17.Value    = 0x00000000;
            m_spDelta.DW18.Value    = 0x00000000;
            m_spDelta.DW19.Value    = 0x00000000;
            m_spDelta.DW20.Value    = 0x00000000;
            m_spDelta.DW21.Value    = 0x00000000;
            m_spDelta.DW22.Value    = 0x00000000;
            m_spDelta.DW23.Value    = 0x00000000;
            m_spDelta.DW24.Value    = 0x00000000;
            m_spDelta.DW25.Value    = 0x00000000;
            m_spDelta.DW26.Value    = 0x00000000;
            m_spDelta.DW27.Value    = 0x00000000;
            m_spDelta.DW28.Value    = 0x00000000;
            m_spDelta.DW29.Value    = 0x00000000;
            m_spDelta.DW30.Value    = 0x00000000;
            m_spDelta.DW31.Value    = 0x00000000;
            m_dw32.Value            = 0x80010000;
            m_dw33.Value            = 0x00040c24;
            m_dw34.Value            = 0x00000000;
            m_dw35.Value            = 0xffff00ff;
            m_dw36.Value            = 0x60000000;
            m_dw37.Value            = 0x000000a1;
            m_dw38.Value            = 0x00003900;
            m_dw39.Value            = 0x28300000;
            m_dw40.Value            = 0x00000000;
            m_dw41.Value            = 0x00000000;
            m_dw42.Value            = 0x00000000;
            m_dw43.Value            = 0x00000000;
            m_dw44.Value            = 0x00000000;
            m_dw45.Value            = 0x00000000;
            m_dw46.Value            = 0x00000000;
            m_dw47.Value            = 0x08000002;
            m_dw48.Value            = 0x00000000;
            m_dw49.Value            = 0x00000000;
            m_dw50.Value            = 0x00000000;
            m_dw51.Value            = 0x00000000;
            m_dw52.Value            = 0x00000000;
            m_dw53.Value            = 0x00000000;
            m_dw54.Value            = 0x00000000;
            m_dw55.Value            = 0x00000000;
            m_dw56.Value            = 0x00000000;
            m_dw57.Value            = 0x00000000;
            m_dw58.Value            = 0x00000000;
            m_dw59.Value            = 0x00000000;
            m_dw60.Value            = 0x00000000;
            m_dw61.Value            = 0x00000000;
            m_dw62.Value            = 0x00000000;
            m_dw63.Value            = 0x00000000;
            m_dw64.Value            = 0x00000000;
            m_dw65.Value            = 0x00000000;
            m_dw66.Value            = 0xffffffff;
            m_dw67.Value            = 0xffffffff;
            m_dw68.Value            = 0xffffffff;
            m_dw69.Value            = 0xffffffff;
            m_dw70.Value            = 0xffffffff;
            m_dw71.Value            = 0xffffffff;
            m_dw72.Value            = 0x00000000;
            m_dw73.Value            = 0x00000000;
            m_dw74.Value            = 0x00000000;
            m_dw75.Value            = 0xffffffff;
            m_dw76.Value            = 0x00000000;
            m_dw77.Value            = 0x00000000;
            m_dw78.Value            = 0xffffffff;
            m_dw79.Value            = 0xffffffff;
            m_dw80.Value            = 0xffffffff;
            m_dw81.Value            = 0xffffffff;
            m_dw82.Value            = 0xffffffff;
            m_dw83.Value            = 0xffffffff;
            m_dw84.Value            = 0xffffffff;
            m_dw85.Value            = 0x00000000;
            m_dw86.Value            = 0x00000000;
            m_dw87.Value            = 0x00000000;
            m_dw88.Value            = 0xffffffff;
            break;
        case PField:
            m_dw0.Value             = 0x000000a3;
            m_dw1.Value             = 0x00000008;
            m_dw2.Value             = 0x00003910;
            m_dw3.Value             = 0x00ae30c0;
            m_dw4.Value             = 0x30000000;
            m_dw5.Value             = 0x28300000;
            m_dw6.Value             = 0x05000000;
            m_dw7.Value             = 0x01400060;
            m_modeMvCost.DW8.Value  = 0x00000000;
            m_modeMvCost.DW9.Value  = 0x00000000;
            m_modeMvCost.DW10.Value = 0x00000000;
            m_modeMvCost.DW11.Value = 0x00000000;
            m_modeMvCost.DW12.Value = 0x00000000;
            m_modeMvCost.DW13.Value = 0x00000000;
            m_modeMvCost.DW14.Value = 0x00000000;
            m_modeMvCost.DW15.Value = 0x00000000;
            m_spDelta.DW16.Value    = 0x00000000;
            m_spDelta.DW17.Value    = 0x00000000;
            m_spDelta.DW18.Value    = 0x00000000;
            m_spDelta.DW19.Value    = 0x00000000;
            m_spDelta.DW20.Value    = 0x00000000;
            m_spDelta.DW21.Value    = 0x00000000;
            m_spDelta.DW22.Value    = 0x00000000;
            m_spDelta.DW23.Value    = 0x00000000;
            m_spDelta.DW24.Value    = 0x00000000;
            m_spDelta.DW25.Value    = 0x00000000;
            m_spDelta.DW26.Value    = 0x00000000;
            m_spDelta.DW27.Value    = 0x00000000;
            m_spDelta.DW28.Value    = 0x00000000;
            m_spDelta.DW29.Value    = 0x00000000;
            m_spDelta.DW30.Value    = 0x00000000;
            m_spDelta.DW31.Value    = 0x00000000;
            m_dw32.Value            = 0x80010000;
            m_dw33.Value            = 0x00040c24;
            m_dw34.Value            = 0x00000000;
            m_dw35.Value            = 0xffff00ff;
            m_dw36.Value            = 0x40000000;
            m_dw37.Value            = 0x000000a1;
            m_dw38.Value            = 0x00003900;
            m_dw39.Value            = 0x28300000;
            m_dw40.Value            = 0x00000000;
            m_dw41.Value            = 0x00000000;
            m_dw42.Value            = 0x00000000;
            m_dw43.Value            = 0x00000000;
            m_dw44.Value            = 0x00000000;
            m_dw45.Value            = 0x00000000;
            m_dw46.Value            = 0x00000000;
            m_dw47.Value            = 0x04000002;
            m_dw48.Value            = 0x00000000;
            m_dw49.Value            = 0x00000000;
            m_dw50.Value            = 0x00000000;
            m_dw51.Value            = 0x00000000;
            m_dw52.Value            = 0x00000000;
            m_dw53.Value            = 0x00000000;
            m_dw54.Value            = 0x00000000;
            m_dw55.Value            = 0x00000000;
            m_dw56.Value            = 0x00000000;
            m_dw57.Value            = 0x00000000;
            m_dw58.Value            = 0x00000000;
            m_dw59.Value            = 0x00000000;
            m_dw60.Value            = 0x00000000;
            m_dw61.Value            = 0x00000000;
            m_dw62.Value            = 0x00000000;
            m_dw63.Value            = 0x00000000;
            m_dw64.Value            = 0x00000000;
            m_dw65.Value            = 0x00000000;
            m_dw66.Value            = 0xffffffff;
            m_dw67.Value            = 0xffffffff;
            m_dw68.Value            = 0xffffffff;
            m_dw69.Value            = 0xffffffff;
            m_dw70.Value            = 0xffffffff;
            m_dw71.Value            = 0xffffffff;
            m_dw72.Value            = 0x00000000;
            m_dw73.Value            = 0x00000000;
            m_dw74.Value            = 0x00000000;
            m_dw75.Value            = 0xffffffff;
            m_dw76.Value            = 0x00000000;
            m_dw77.Value            = 0x00000000;
            m_dw78.Value            = 0xffffffff;
            m_dw79.Value            = 0xffffffff;
            m_dw80.Value            = 0xffffffff;
            m_dw81.Value            = 0xffffffff;
            m_dw82.Value            = 0xffffffff;
            m_dw83.Value            = 0xffffffff;
            m_dw84.Value            = 0xffffffff;
            m_dw85.Value            = 0x00000000;
            m_dw86.Value            = 0x00000000;
            m_dw87.Value            = 0x00000000;
            m_dw88.Value            = 0xffffffff;
            break;
        case BFrame:
            m_dw0.Value             = 0x000000a3;
            m_dw1.Value             = 0x00200008;
            m_dw2.Value             = 0x00003910;
            m_dw3.Value             = 0x00aa7700;
            m_dw4.Value             = 0x50020000;
            m_dw5.Value             = 0x20200000;
            m_dw6.Value             = 0x05000000;
            m_dw7.Value             = 0xff400000;
            m_modeMvCost.DW8.Value  = 0x00000000;
            m_modeMvCost.DW9.Value  = 0x00000000;
            m_modeMvCost.DW10.Value = 0x00000000;
            m_modeMvCost.DW11.Value = 0x00000000;
            m_modeMvCost.DW12.Value = 0x00000000;
            m_modeMvCost.DW13.Value = 0x00000000;
            m_modeMvCost.DW14.Value = 0x00000000;
            m_modeMvCost.DW15.Value = 0x00000000;
            m_spDelta.DW16.Value    = 0x00000000;
            m_spDelta.DW17.Value    = 0x00000000;
            m_spDelta.DW18.Value    = 0x00000000;
            m_spDelta.DW19.Value    = 0x00000000;
            m_spDelta.DW20.Value    = 0x00000000;
            m_spDelta.DW21.Value    = 0x00000000;
            m_spDelta.DW22.Value    = 0x00000000;
            m_spDelta.DW23.Value    = 0x00000000;
            m_spDelta.DW24.Value    = 0x00000000;
            m_spDelta.DW25.Value    = 0x00000000;
            m_spDelta.DW26.Value    = 0x00000000;
            m_spDelta.DW27.Value    = 0x00000000;
            m_spDelta.DW28.Value    = 0x00000000;
            m_spDelta.DW29.Value    = 0x00000000;
            m_spDelta.DW30.Value    = 0x00000000;
            m_spDelta.DW31.Value    = 0x00000000;
            m_dw32.Value            = 0x01010000;
            m_dw33.Value            = 0x00040c24;
            m_dw34.Value            = 0x00000000;
            m_dw35.Value            = 0xffff00ff;
            m_dw36.Value            = 0x60000000;
            m_dw37.Value            = 0x000000a1;
            m_dw38.Value            = 0x00003900;
            m_dw39.Value            = 0x28300000;
            m_dw40.Value            = 0x00000000;
            m_dw41.Value            = 0x00000000;
            m_dw42.Value            = 0x00000000;
            m_dw43.Value            = 0x00000000;
            m_dw44.Value            = 0x00000000;
            m_dw45.Value            = 0x00000000;
            m_dw46.Value            = 0x00000000;
            m_dw47.Value            = 0x08000002;
            m_dw48.Value            = 0x00000000;
            m_dw49.Value            = 0x00000000;
            m_dw50.Value            = 0x00000000;
            m_dw51.Value            = 0x00000000;
            m_dw52.Value            = 0x00000000;
            m_dw53.Value            = 0x00000000;
            m_dw54.Value            = 0x00000000;
            m_dw55.Value            = 0x00000000;
            m_dw56.Value            = 0x00000000;
            m_dw57.Value            = 0x00000000;
            m_dw58.Value            = 0x00000000;
            m_dw59.Value            = 0x00000000;
            m_dw60.Value            = 0x00000000;
            m_dw61.Value            = 0x00000000;
            m_dw62.Value            = 0x00000000;
            m_dw63.Value            = 0x00000000;
            m_dw64.Value            = 0x00000000;
            m_dw65.Value            = 0x00000000;
            m_dw66.Value            = 0xffffffff;
            m_dw67.Value            = 0xffffffff;
            m_dw68.Value            = 0xffffffff;
            m_dw69.Value            = 0xffffffff;
            m_dw70.Value            = 0xffffffff;
            m_dw71.Value            = 0xffffffff;
            m_dw72.Value            = 0x00000000;
            m_dw73.Value            = 0x00000000;
            m_dw74.Value            = 0x00000000;
            m_dw75.Value            = 0xffffffff;
            m_dw76.Value            = 0x00000000;
            m_dw77.Value            = 0x00000000;
            m_dw78.Value            = 0xffffffff;
            m_dw79.Value            = 0xffffffff;
            m_dw80.Value            = 0xffffffff;
            m_dw81.Value            = 0xffffffff;
            m_dw82.Value            = 0xffffffff;
            m_dw83.Value            = 0xffffffff;
            m_dw84.Value            = 0xffffffff;
            m_dw85.Value            = 0x00000000;
            m_dw86.Value            = 0x00000000;
            m_dw87.Value            = 0x00000000;
            m_dw88.Value            = 0xffffffff;
            break;
        case BField:
            m_dw0.Value             = 0x000000a3;
            m_dw1.Value             = 0x00200008;
            m_dw2.Value             = 0x00003919;
            m_dw3.Value             = 0x00aa77c0;
            m_dw4.Value             = 0x50020000;
            m_dw5.Value             = 0x20200000;
            m_dw6.Value             = 0x05000000;
            m_dw7.Value             = 0xff400000;
            m_modeMvCost.DW8.Value  = 0x00000000;
            m_modeMvCost.DW9.Value  = 0x00000000;
            m_modeMvCost.DW10.Value = 0x00000000;
            m_modeMvCost.DW11.Value = 0x00000000;
            m_modeMvCost.DW12.Value = 0x00000000;
            m_modeMvCost.DW13.Value = 0x00000000;
            m_modeMvCost.DW14.Value = 0x00000000;
            m_modeMvCost.DW15.Value = 0x00000000;
            m_spDelta.DW16.Value    = 0x00000000;
            m_spDelta.DW17.Value    = 0x00000000;
            m_spDelta.DW18.Value    = 0x00000000;
            m_spDelta.DW19.Value    = 0x00000000;
            m_spDelta.DW20.Value    = 0x00000000;
            m_spDelta.DW21.Value    = 0x00000000;
            m_spDelta.DW22.Value    = 0x00000000;
            m_spDelta.DW23.Value    = 0x00000000;
            m_spDelta.DW24.Value    = 0x00000000;
            m_spDelta.DW25.Value    = 0x00000000;
            m_spDelta.DW26.Value    = 0x00000000;
            m_spDelta.DW27.Value    = 0x00000000;
            m_spDelta.DW28.Value    = 0x00000000;
            m_spDelta.DW29.Value    = 0x00000000;
            m_spDelta.DW30.Value    = 0x00000000;
            m_spDelta.DW31.Value    = 0x00000000;
            m_dw32.Value            = 0x01010000;
            m_dw33.Value            = 0x00040c24;
            m_dw34.Value            = 0x00000000;
            m_dw35.Value            = 0xffff00ff;
            m_dw36.Value            = 0x40000000;
            m_dw37.Value            = 0x000000a1;
            m_dw38.Value            = 0x00003900;
            m_dw39.Value            = 0x28300000;
            m_dw40.Value            = 0x00000000;
            m_dw41.Value            = 0x00000000;
            m_dw42.Value            = 0x00000000;
            m_dw43.Value            = 0x00000000;
            m_dw44.Value            = 0x00000000;
            m_dw45.Value            = 0x00000000;
            m_dw46.Value            = 0x00000000;
            m_dw47.Value            = 0x04000002;
            m_dw48.Value            = 0x00000000;
            m_dw49.Value            = 0x00000000;
            m_dw50.Value            = 0x00000000;
            m_dw51.Value            = 0x00000000;
            m_dw52.Value            = 0x00000000;
            m_dw53.Value            = 0x00000000;
            m_dw54.Value            = 0x00000000;
            m_dw55.Value            = 0x00000000;
            m_dw56.Value            = 0x00000000;
            m_dw57.Value            = 0x00000000;
            m_dw58.Value            = 0x00000000;
            m_dw59.Value            = 0x00000000;
            m_dw60.Value            = 0x00000000;
            m_dw61.Value            = 0x00000000;
            m_dw62.Value            = 0x00000000;
            m_dw63.Value            = 0x00000000;
            m_dw64.Value            = 0x00000000;
            m_dw65.Value            = 0x00000000;
            m_dw66.Value            = 0xffffffff;
            m_dw67.Value            = 0xffffffff;
            m_dw68.Value            = 0xffffffff;
            m_dw69.Value            = 0xffffffff;
            m_dw70.Value            = 0xffffffff;
            m_dw71.Value            = 0xffffffff;
            m_dw72.Value            = 0x00000000;
            m_dw73.Value            = 0x00000000;
            m_dw74.Value            = 0x00000000;
            m_dw75.Value            = 0xffffffff;
            m_dw76.Value            = 0x00000000;
            m_dw77.Value            = 0x00000000;
            m_dw78.Value            = 0xffffffff;
            m_dw79.Value            = 0xffffffff;
            m_dw80.Value            = 0xffffffff;
            m_dw81.Value            = 0xffffffff;
            m_dw82.Value            = 0xffffffff;
            m_dw83.Value            = 0xffffffff;
            m_dw84.Value            = 0xffffffff;
            m_dw85.Value            = 0x00000000;
            m_dw86.Value            = 0x00000000;
            m_dw87.Value            = 0x00000000;
            m_dw88.Value            = 0xffffffff;
            break;
        default:
            break;
        }
    };
};

class BrcBlockCopyCurbe
{
public:
    // DWORD 0
    union
    {
        struct
        {
            uint32_t   BlockHeight : 16;
            uint32_t   BufferOffset : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw0;

    // DWORD 1
    union
    {
        struct
        {
            uint32_t   SrcSurfaceIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw1;

    // DWORD 2
    union
    {
        struct
        {
            uint32_t  DstSurfaceIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw2;

    // QWORD PADDING
    struct
    {
        uint32_t  Reserved;
    } m_padding;

    BrcBlockCopyCurbe()
    {
        m_dw0.Value        = 0;
        m_dw1.Value        = 0;
        m_dw2.Value        = 0;
        m_padding.Reserved = 0;
    }
};

class BrcInitResetCurbeG11
{
public:
    union
    {
        struct
        {
            uint32_t   ProfileLevelMaxFrame;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw0;

    union
    {
        struct
        {
            uint32_t   InitBufFullInBits;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw1;

    union
    {
        struct
        {
            uint32_t   BufSizeInBits;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw2;

    union
    {
        struct
        {
            uint32_t   AverageBitRate;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw3;

    union
    {
        struct
        {
            uint32_t   MaxBitRate;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw4;

    union
    {
        struct
        {
            uint32_t   MinBitRate;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw5;

    union
    {
        struct
        {
            uint32_t   FrameRateM;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw6;

    union
    {
        struct
        {
            uint32_t   FrameRateD;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw7;

    union
    {
        struct
        {
            uint32_t   BRCFlag : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   GopP : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw8;

    union
    {
        struct
        {
            uint32_t   GopB : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   FrameWidthInBytes : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw9;

    union
    {
        struct
        {
            uint32_t   FrameHeightInBytes : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   AVBRAccuracy : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw10;

    union
    {
        struct
        {
            uint32_t   AVBRConvergence : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MinQP : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw11;

    union
    {
        struct
        {
            uint32_t   MaxQP : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   NoSlices : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw12;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForP : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForP : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForP : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForP : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw13;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForB : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForB : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForB : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForB : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw14;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForI : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForI : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForI : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForI : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw15;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForPandB : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold1ForPandB : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold2ForPandB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold3ForPandB : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw16;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForPandB : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold5ForPandB : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold6ForPandB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold7ForPandB : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw17;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForVBR : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold1ForVBR : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold2ForVBR : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold3ForVBR : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw18;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForVBR : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   DeviationThreshold5ForVBR : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   DeviationThreshold6ForVBR : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold7ForVBR : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw19;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForI : MOS_BITFIELD_RANGE(0, 7);        // Signed byte
            uint32_t   DeviationThreshold1ForI : MOS_BITFIELD_RANGE(8, 15);       // Signed byte
            uint32_t   DeviationThreshold2ForI : MOS_BITFIELD_RANGE(16, 23);      // Signed byte
            uint32_t   DeviationThreshold3ForI : MOS_BITFIELD_RANGE(24, 31);      // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw20;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForI : MOS_BITFIELD_RANGE(0, 7);      // Signed byte
            uint32_t   DeviationThreshold5ForI : MOS_BITFIELD_RANGE(8, 15);      // Signed byte
            uint32_t   DeviationThreshold6ForI : MOS_BITFIELD_RANGE(16, 23);      // Signed byte
            uint32_t   DeviationThreshold7ForI : MOS_BITFIELD_RANGE(24, 31);      // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw21;

    union
    {
        struct
        {
            uint32_t   InitialQPForI : MOS_BITFIELD_RANGE(0, 7);     // Signed byte
            uint32_t   InitialQPForP : MOS_BITFIELD_RANGE(8, 15);     // Signed byte
            uint32_t   InitialQPForB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   SlidingWindowSize : MOS_BITFIELD_RANGE(24, 31);     // unsigned byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw22;

    union
    {
        struct
        {
            uint32_t   ACQP;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw23;

    union
    {
        struct
        {
            uint32_t   LongTermInterval : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw24;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw25;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw26;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw27;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw28;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw29;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw30;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw31;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexhistorybuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw32;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexdistortionbuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw33;

    BrcInitResetCurbeG11()
    {
        m_dw0.Value                      = 0;
        m_dw1.Value                      = 0;
        m_dw2.Value                      = 0;
        m_dw3.Value                      = 0;
        m_dw4.Value                      = 0;
        m_dw5.Value                      = 0;
        m_dw6.Value                      = 0;
        m_dw7.Value                      = 0;
        m_dw8.Value                      = 0;
        m_dw9.Value                      = 0;
        m_dw10.Value                     = 0;
        m_dw11.AVBRConvergence           = 0;
        m_dw11.MinQP                     = 1;
        m_dw12.MaxQP                     = 51;
        m_dw12.NoSlices                  = 0;
        m_dw13.InstantRateThreshold0ForP = 40;
        m_dw13.InstantRateThreshold1ForP = 60;
        m_dw13.InstantRateThreshold2ForP = 80;
        m_dw13.InstantRateThreshold3ForP = 120;
        m_dw14.InstantRateThreshold0ForB = 35;
        m_dw14.InstantRateThreshold1ForB = 60;
        m_dw14.InstantRateThreshold2ForB = 80;
        m_dw14.InstantRateThreshold3ForB = 120;
        m_dw15.InstantRateThreshold0ForI = 40;
        m_dw15.InstantRateThreshold1ForI = 60;
        m_dw15.InstantRateThreshold2ForI = 90;
        m_dw15.InstantRateThreshold3ForI = 115;
        m_dw16.Value                     = 0;
        m_dw17.Value                     = 0;
        m_dw18.Value                     = 0;
        m_dw19.Value                     = 0;
        m_dw20.Value                     = 0;
        m_dw21.Value                     = 0;
        m_dw22.Value                     = 0;
        m_dw23.Value                     = 0;
        m_dw24.Value                     = 0;
        m_dw25.Value                     = 0;
        m_dw26.Value                     = 0;
        m_dw27.Value                     = 0;
        m_dw28.Value                     = 0;
        m_dw29.Value                     = 0;
        m_dw30.Value                     = 0;
        m_dw31.Value                     = 0;
        m_dw32.Value                     = 0;
        m_dw33.Value                     = 0;
    }
};

class MbBrcUpdateCurbe
{
public:
    union
    {
        struct
        {
            uint32_t   CurrFrameType                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   EnableROI                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   ROIRatio                                    : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   CQP_QPValue                                 : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw0;

    union
    {
        struct
        {
            uint32_t   EnableCQPMode                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  8,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw1;

    union
    {
        struct
        {
            uint32_t  Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw2;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw3;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw4;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw5;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw6;

    union
    {
        struct
        {
            uint32_t   Reserved;;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw7;

    union
    {
        struct
        {
            uint32_t   HistorybufferIndex;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw8;

    union
    {
        struct
        {
            uint32_t    MBQPbufferIndex;
        };
        struct
        {
            uint32_t    Value;
        };
    } m_dw9;

    union
    {
        struct
        {
            uint32_t    ROIbufferIndex;
        };
        struct
        {
            uint32_t    Value;
        };
    } m_dw10;

    union
    {
        struct
        {
            uint32_t    MBstatisticalbufferIndex;
        };
        struct
        {
            uint32_t    Value;
        };
    } m_dw11;

    MbBrcUpdateCurbe()
    {
        m_dw0.Value = 0;
        m_dw1.Value = 0;
        m_dw2.Value = 0;
        m_dw3.Value = 0;
        m_dw4.Value = 0;
        m_dw5.Value = 0;
        m_dw6.Value = 0;
        m_dw7.Value = 0;
        m_dw8.Value = 0;
        m_dw9.Value = 0;
        m_dw10.Value = 0;
        m_dw11.Value = 0;
    }
};

class FrameBrcUpdateCurbe
{
public:
    union
    {
        struct
        {
            uint32_t   TargetSize;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw0;

    union
    {
        struct
        {
            uint32_t   FrameNumber;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw1;

    union
    {
        struct
        {
            uint32_t   SizeofPicHeaders;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw2;

    union
    {
        struct
        {
            uint32_t   startGAdjFrame0                             : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   startGAdjFrame1                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw3;

    union
    {
        struct
        {
            uint32_t   startGAdjFrame2                             : MOS_BITFIELD_RANGE(  0,15 );
            uint32_t   startGAdjFrame3                             : MOS_BITFIELD_RANGE( 16,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw4;

    union
    {
        struct
        {
            uint32_t   TargetSizeFlag                              : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   BRCFlag                                     : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaxNumPAKs                                  : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   CurrFrameType                               : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw5;

    union
    {
        struct
        {
            uint32_t   NumSkipFrames                               : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   MinimumQP                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   MaximumQP                                   : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   EnableForceToSkip                           : MOS_BITFIELD_BIT(      24 );
            uint32_t   EnableSlidingWindow                         : MOS_BITFIELD_BIT(      25 );
            uint32_t   EnableExtremLowDelay                        : MOS_BITFIELD_BIT(      26 );
            uint32_t   DisableVarCompute                           : MOS_BITFIELD_BIT(      27 );
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE( 28,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw6;

    union
    {
        struct
        {
            uint32_t    SizeSkipFrames;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw7;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult0                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   StartGlobalAdjustMult1                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   StartGlobalAdjustMult2                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   StartGlobalAdjustMult3                      : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw8;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult4                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   StartGlobalAdjustDiv0                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   StartGlobalAdjustDiv1                       : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   StartGlobalAdjustDiv2                       : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw9;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustDiv3                       : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   StartGlobalAdjustDiv4                       : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   QPThreshold0                                : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   QPThreshold1                                : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw10;

    union
    {
        struct
        {
            uint32_t   QPThreshold2                                : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   QPThreshold3                                : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThreshold0                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   gRateRatioThreshold1                        : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw11;

    union
    {
        struct
        {
            uint32_t   gRateRatioThreshold2                        : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   gRateRatioThreshold3                        : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThreshold4                        : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   gRateRatioThreshold5                        : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw12;

    union
    {
        struct
        {
            uint32_t   gRateRatioThresholdQP0                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   gRateRatioThresholdQP1                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThresholdQP2                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   gRateRatioThresholdQP3                      : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw13;

    union
    {
        struct
        {
            uint32_t   gRateRatioThresholdQP4                      : MOS_BITFIELD_RANGE(  0, 7 );
            uint32_t   gRateRatioThresholdQP5                      : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   gRateRatioThresholdQP6                      : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   QPIndexOfCurPic                             : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw14;

    union
    {
        struct
        {
            uint32_t   Reserved                                    : MOS_BITFIELD_RANGE(  0,7  );
            uint32_t   EnableROI                                   : MOS_BITFIELD_RANGE(  8,15 );
            uint32_t   RoundingIntra                               : MOS_BITFIELD_RANGE( 16,23 );
            uint32_t   RoundingInter                               : MOS_BITFIELD_RANGE( 24,31 );
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw15;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw16;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw17;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw18;

    union
    {
        struct
        {
            uint32_t   UserMaxFrame;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw19;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw20;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw21;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw22;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw23;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexBRChistorybuffer;
        };
        struct
        {
            uint32_t    Value;
        };
    } m_dw24;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexPreciousPAKstatisticsoutputbuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw25;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexAVCIMGstateinputbuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw26;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexAVCIMGstateoutputbuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw27;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexAVC_Encbuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw28;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexAVCDISTORTIONbuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw29;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexBRCconstdatabuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw30;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexMBStatsBuffer;
        };
        struct
        {
            uint32_t    Value;
        };
    } m_dw31;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexMotionvectorbuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw32;

    FrameBrcUpdateCurbe()
    {
        m_dw0.Value           = 0;
        m_dw1.Value           = 0;
        m_dw2.Value           = 0;
        m_dw3.startGAdjFrame0 = 10;
        m_dw3.startGAdjFrame1 = 50;
        m_dw4.startGAdjFrame2 = 100;
        m_dw4.startGAdjFrame3 = 150;
        m_dw5.Value           = 0;
        m_dw6.Value           = 0;
        m_dw7.Value           = 0;

        m_dw8.StartGlobalAdjustMult0 = 1;
        m_dw8.StartGlobalAdjustMult1 = 1;
        m_dw8.StartGlobalAdjustMult2 = 3;
        m_dw8.StartGlobalAdjustMult3 = 2;

        m_dw9.StartGlobalAdjustMult4 = 1;
        m_dw9.StartGlobalAdjustDiv0  = 40;
        m_dw9.StartGlobalAdjustDiv1  = 5;
        m_dw9.StartGlobalAdjustDiv2  = 5;

        m_dw10.StartGlobalAdjustDiv3 = 3;
        m_dw10.StartGlobalAdjustDiv4 = 1;
        m_dw10.QPThreshold0          = 7;
        m_dw10.QPThreshold1          = 18;

        m_dw11.QPThreshold2         = 25;
        m_dw11.QPThreshold3         = 37;
        m_dw11.gRateRatioThreshold0 = 40;
        m_dw11.gRateRatioThreshold1 = 75;

        m_dw12.gRateRatioThreshold2 = 97;
        m_dw12.gRateRatioThreshold3 = 103;
        m_dw12.gRateRatioThreshold4 = 125;
        m_dw12.gRateRatioThreshold5 = 160;

        m_dw13.gRateRatioThresholdQP0 = MOS_BITFIELD_VALUE((uint32_t)-3, 8);
        m_dw13.gRateRatioThresholdQP1 = MOS_BITFIELD_VALUE((uint32_t)-2, 8);
        m_dw13.gRateRatioThresholdQP2 = MOS_BITFIELD_VALUE((uint32_t)-1, 8);
        m_dw13.gRateRatioThresholdQP3 = 0;

        m_dw14.gRateRatioThresholdQP4 = 1;
        m_dw14.gRateRatioThresholdQP5 = 2;
        m_dw14.gRateRatioThresholdQP6 = 3;
        m_dw14.QPIndexOfCurPic        = 0xff;

        m_dw15.Value = 0;
        m_dw16.Value = 0;
        m_dw17.Value = 0;
        m_dw18.Value = 0;
        m_dw19.Value = 0;
        m_dw20.Value = 0;
        m_dw21.Value = 0;
        m_dw22.Value = 0;
        m_dw23.Value = 0;
        m_dw24.Value = 0;
        m_dw25.Value = 0;
        m_dw26.Value = 0;
        m_dw27.Value = 0;
        m_dw28.Value = 0;
        m_dw29.Value = 0;
        m_dw30.Value = 0;
        m_dw31.Value = 0;
        m_dw32.Value = 0;
    };
};

class KernelHeader
{
public:
    int m_kernelCount;

    // Quality mode for Frame/Field
    CODECHAL_KERNEL_HEADER m_avcmbEncQltyI;
    CODECHAL_KERNEL_HEADER m_avcmbEncQltyP;
    CODECHAL_KERNEL_HEADER m_avcmbEncQltyB;
    // Normal mode for Frame/Field
    CODECHAL_KERNEL_HEADER m_avcmbEncNormI;
    CODECHAL_KERNEL_HEADER m_avcmbEncNormP;
    CODECHAL_KERNEL_HEADER m_avcmbEncNormB;
    // Performance modes for Frame/Field
    CODECHAL_KERNEL_HEADER m_avcmbEncPerfI;
    CODECHAL_KERNEL_HEADER m_avcmbEncPerfP;
    CODECHAL_KERNEL_HEADER m_avcmbEncPerfB;
    // Modes for Frame/Field
    CODECHAL_KERNEL_HEADER m_avcmbEncAdvI;
    CODECHAL_KERNEL_HEADER m_avcmbEncAdvP;
    CODECHAL_KERNEL_HEADER m_avcmbEncAdvB;

    // BRC Init frame
    CODECHAL_KERNEL_HEADER m_initFrameBrc;

    // FrameBRC Update
    CODECHAL_KERNEL_HEADER m_frameEncUpdate;

    // BRC Reset frame
    CODECHAL_KERNEL_HEADER m_brcResetFrame;

    // BRC I Frame Distortion
    CODECHAL_KERNEL_HEADER m_brcIFrameDist;

    // BRCBlockCopy
    CODECHAL_KERNEL_HEADER m_brcBlockCopy;

    // MbBRC Update
    CODECHAL_KERNEL_HEADER m_mbBrcUpdate;

    //Weighted Prediction Kernel
    CODECHAL_KERNEL_HEADER m_avcWeightedPrediction;

    // SW scoreboard initialization kernel
    CODECHAL_KERNEL_HEADER m_avcInitSwScoreboard;
};

// QP is from 0 - 51, pad it to 64 since BRC needs array size to be 64 bytes
const uint8_t CodechalEncodeAvcEncG11::m_QPAdjustmentDistThresholdMaxFrameThresholdIPB[576] =
{
    0x01, 0x02, 0x03, 0x05, 0x06, 0x01, 0x01, 0x02, 0x03, 0x05, 0x00, 0x00, 0x01, 0x02, 0x03, 0xff,
    0x00, 0x00, 0x01, 0x02, 0xff, 0x00, 0x00, 0x00, 0x01, 0xfe, 0xfe, 0xff, 0x00, 0x01, 0xfd, 0xfd,
    0xff, 0xff, 0x00, 0xfb, 0xfd, 0xfe, 0xff, 0xff, 0xfa, 0xfb, 0xfd, 0xfe, 0xff, 0x00, 0x04, 0x1e,
    0x3c, 0x50, 0x78, 0x8c, 0xc8, 0xff, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x05, 0x06, 0x01, 0x01, 0x02, 0x03, 0x05, 0x00, 0x01, 0x01, 0x02, 0x03, 0xff,
    0x00, 0x00, 0x01, 0x02, 0xff, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0x00, 0x01, 0xfe, 0xff,
    0xff, 0xff, 0x00, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0xfb, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0x04, 0x1e,
    0x3c, 0x50, 0x78, 0x8c, 0xc8, 0xff, 0x04, 0x05, 0x06, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x04, 0x05, 0x01, 0x01, 0x01, 0x02, 0x04, 0x00, 0x00, 0x01, 0x01, 0x02, 0xff,
    0x00, 0x00, 0x01, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x01, 0xfe, 0xff,
    0xff, 0xff, 0x00, 0xfd, 0xfe, 0xff, 0xff, 0x00, 0xfb, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0x02, 0x14,
    0x28, 0x46, 0x82, 0xa0, 0xc8, 0xff, 0x04, 0x04, 0x05, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03,
    0x03, 0x04, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0xfe, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe,
    0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfe, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03,
    0x03, 0x04, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0xfe, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe,
    0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfe, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03,
    0x03, 0x04, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0xfe, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe,
    0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfe, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const CODECHAL_ENCODE_AVC_IPCM_THRESHOLD CodechalEncodeAvcEncG11::m_IPCMThresholdTable[5] =
{
    { 2, 3000 },
    { 4, 3600 },
    { 6, 5000 },
    { 10, 7500 },
    { 18, 9000 },
};

const uint32_t CodechalEncodeAvcEncG11::m_IntraModeCostForHighTextureMB[CODEC_AVC_NUM_QP]
{
    0x00000303, 0x00000304, 0x00000404, 0x00000405, 0x00000505, 0x00000506, 0x00000607, 0x00000708,
    0x00000809, 0x0000090a, 0x00000a0b, 0x00000b0c, 0x00000c0e, 0x00000e18, 0x00001819, 0x00001918,
    0x00001a19, 0x00001b19, 0x00001d19, 0x00001e18, 0x00002818, 0x00002918, 0x00002a18, 0x00002b19,
    0x00002d18, 0x00002e18, 0x00003818, 0x00003918, 0x00003a18, 0x00003b0f, 0x00003d0e, 0x00003e0e,
    0x0000480e, 0x0000490e, 0x00004a0e, 0x00004b0d, 0x00004d0d, 0x00004e0d, 0x0000580e, 0x0000590e,
    0x00005a0e, 0x00005b0d, 0x00005d0c, 0x00005e0b, 0x0000680a, 0x00006908, 0x00006a09, 0x00006b0a,
    0x00006d0b, 0x00006e0d, 0x0000780e, 0x00007918
};

const int32_t CodechalEncodeAvcEncG11::m_brcBTCounts[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    frameBrcUpdateNumSurfaces,
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    mbencNumSurfaces,
    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_NUM_SURFACES,
    mbBrcUpdateNumSurfaces     // MbBRCUpdate kernel starting GEN9
};

const int32_t CodechalEncodeAvcEncG11::m_brcCurbeSize[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    (sizeof(BrcInitResetCurbeG11)),
    (sizeof(FrameBrcUpdateCurbe)),
    (sizeof(BrcInitResetCurbeG11)),
    (sizeof(MbencCurbe)),
    0,
    (sizeof(MbBrcUpdateCurbe))     // MbBRCUpdate kernel starting GEN9
};

const uint16_t CodechalEncodeAvcEncG11::m_LambdaData[256] = {
     9,     7,     9,     6,    12,     8,    12,     8,    15,    10,    15,     9,    19,    13,    19,    12,    24,
    17,    24,    15,    30,    21,    30,    19,    38,    27,    38,    24,    48,    34,    48,    31,    60,    43,
    60,    39,    76,    54,    76,    49,    96,    68,    96,    62,   121,    85,   121,    78,   153,   108,   153,
    99,   193,   135,   193,   125,   243,   171,   243,   157,   306,   215,   307,   199,   385,   271,   387,   251,
   485,   342,   488,   317,   612,   431,   616,   400,   771,   543,   777,   505,   971,   684,   981,   638,  1224,
   862,  1237,   806,  1542,  1086,  1562,  1018,  1991,  1402,  1971,  1287,  2534,  1785,  2488,  1626,  3077,  2167,
  3141,  2054,  3982,  2805,  3966,  2596,  4887,  3442,  5007,  3281,  6154,  4335,  6322,  4148,  7783,  5482,  7984,
  5243,  9774,  6885, 10082,  6629, 12489,  8797, 12733,  8382, 15566, 10965, 16082, 10599, 19729, 13897, 20313, 13404,
 24797, 17467, 25660, 16954, 31313, 22057, 32415, 21445, 39458, 27795, 40953, 27129, 49594, 34935, 51742, 34323, 61440,
 43987, 61440, 43428, 61440, 55462, 61440, 54954, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
 61440, 61440, 61440, 61440,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
     0,
};

const uint8_t CodechalEncodeAvcEncG11::m_FTQ25[64] = //27 value 4 dummy
{
    0,                                      //qp=0
    0, 0, 0, 0, 0, 0,                       //qp=1,2;3,4;5,6;
    1, 1, 3, 3, 6, 6, 8, 8, 11, 11,         //qp=7,8;9,10;11,12;13,14;15;16
    13, 13, 16, 16, 19, 19, 22, 22, 26, 26, //qp=17,18;19,20;21,22;23,24;25,26
    30, 30, 34, 34, 39, 39, 44, 44, 50, 50, //qp=27,28;29,30;31,32;33,34;35,36
    56, 56, 62, 62, 69, 69, 77, 77, 85, 85, //qp=37,38;39,40;41,42;43,44;45,46
    94, 94, 104, 104, 115, 115,             //qp=47,48;49,50;51
    0, 0, 0, 0, 0, 0, 0, 0                  //dummy
};

// AVC MBEnc RefCost tables, index [CodingType][QP]
// QP is from 0 - 51, pad it to 64 since BRC needs each subarray size to be 128bytes
const uint16_t CodechalEncodeAvcEncG11::m_refCostMultiRefQp[NUM_PIC_TYPES][64] =
{
// I-frame
{
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000
},
// P-slice
{
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000
},
//B-slice
{
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000
}
};

const uint32_t CodechalEncodeAvcEncG11::m_multiPred[NUM_TARGET_USAGE_MODES] =
{
0, 3, 3, 0, 0, 0, 0, 0
};

const uint32_t CodechalEncodeAvcEncG11::m_MRDisableQPCheck[NUM_TARGET_USAGE_MODES] =
{
0, 1, 0, 0, 0, 0, 0, 0
};

MOS_STATUS CodechalEncodeAvcEncG11::GetKernelHeaderAndSize(void *binary, EncOperation operation, uint32_t krnStateIdx, void *krnHeader, uint32_t *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    auto kernelHeaderTable = (KernelHeader *)binary;
    auto                    invalidEntry      = &(kernelHeaderTable->m_avcWeightedPrediction) + 1;
    auto nextKrnOffset = *krnSize;
    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;

    if (operation == ENC_BRC)
    {
        currKrnHeader = &kernelHeaderTable->m_initFrameBrc;
    }
    else if (operation == ENC_MBENC)
    {
        currKrnHeader = &kernelHeaderTable->m_avcmbEncQltyI;
    }
    else if (operation == ENC_MBENC_ADV)
    {
        currKrnHeader = &kernelHeaderTable->m_avcmbEncAdvI;
    }
    else if (operation == ENC_WP)
    {
        currKrnHeader = &kernelHeaderTable->m_avcWeightedPrediction;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    auto pNextKrnHeader = (currKrnHeader + 1);
    if (pNextKrnHeader < invalidEntry)
    {
        nextKrnOffset = pNextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

CodechalEncodeAvcEncG11::CodechalEncodeAvcEncG11(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcEnc(hwInterface, debugInterface, standardInfo),
    m_sinlgePipeVeState(nullptr)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_ASSERT(m_osInterface);

    // Virtual Engine is enabled in default.
    Mos_SetVirtualEngineSupported(m_osInterface, true);

    m_cmKernelEnable           = true;
    m_mbStatsSupported         = true;
    m_useCommonKernel          = true;
    bKernelTrellis             = true;
    bExtendedMvCostRange       = true;
    bBrcSplitEnable            = true;
    bDecoupleMbEncCurbeFromBRC = true;
    bHighTextureModeCostEnable = true;
    bMvDataNeededByBRC         = false;

#ifndef _FULL_OPEN_SOURCE
    //ICLHP currently uses LP kernel
    m_kernelBase = (uint8_t*)IGCODECKRN_G11;
#else
    m_kernelBase = nullptr;
#endif

    this->pfnGetKernelHeaderAndSize = this->GetKernelHeaderAndSize;

    m_vdboxOneDefaultUsed = true;

    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_encodeParState = MOS_New(CodechalDebugEncodeParG11, this));
        CreateAvcPar();
    )
}

CodechalEncodeAvcEncG11::~CodechalEncodeAvcEncG11()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_wpState)
    {
        MOS_Delete(m_wpState);
        m_wpState = nullptr;
    }
    MOS_Delete(m_intraDistKernel);

    if (m_swScoreboardState)
    {
        MOS_Delete(m_swScoreboardState);
        m_swScoreboardState = nullptr;
    }

    if (m_sinlgePipeVeState)
    {
        MOS_FreeMemAndSetNull(m_sinlgePipeVeState);
    }

    CODECHAL_DEBUG_TOOL(
        DestroyAvcPar();
        MOS_Delete(m_encodeParState);
    )
}

MOS_STATUS CodechalEncodeAvcEncG11::MbEncKernel(bool mbEncIFrameDistInUse)
{
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t ppsIdx          = m_avcSliceParams->pic_parameter_set_id;
    uint8_t spsIdx          = m_avcPicParams[ppsIdx]->seq_parameter_set_id;
    auto refList            = &m_refList[0];
    auto currRefList        = m_refList[m_currReconstructedPic.FrameIdx];
    bool use45DegreePattern = false;
    bool roiEnabled         = (m_avcPicParams[ppsIdx]->NumROI > 0) ? true : false;
    uint8_t refPicListIdx   = m_avcSliceParams[ppsIdx].RefPicList[0][0].FrameIdx;
    uint8_t refFrameListIdx = m_avcPicParam[ppsIdx].RefFrameList[refPicListIdx].FrameIdx;
    bool dirtyRoiEnabled    = (m_pictureCodingType == P_TYPE
        && m_avcPicParams[ppsIdx]->NumDirtyROI > 0
        && m_prevReconFrameIdx == refFrameListIdx);

    //  Two flags(bMbConstDataBufferNeeded, bMbQpBufferNeeded)
    //  would be used as there are two buffers and not all cases need both the buffers
    //  Constant Data buffer  needed for MBBRC, MBQP, ROI, RollingIntraRefresh
    //  Please note that this surface needs to be programmed for
    //  all usage cases(including CQP cases) because DWord13 includes mode cost for high texture MB?s cost.
    bool mbConstDataBufferInUse = bMbBrcEnabled || bMbQpDataEnabled || roiEnabled || dirtyRoiEnabled ||
        m_avcPicParam->EnableRollingIntraRefresh || bHighTextureModeCostEnable;

    bool mbQpBufferInUse = bMbBrcEnabled || bBrcRoiEnabled || bMbQpDataEnabled;

    if (m_feiEnable)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_avcFeiPicParams);
        mbConstDataBufferInUse |= m_avcFeiPicParams->bMBQp;
        mbQpBufferInUse |= m_avcFeiPicParams->bMBQp;
    }

    // MFE MBEnc kernel handles several frames from different streams in one submission.
    // All the streams use the same HW/OS/StateHeap interfaces during this submssion.
    // All the streams use the kernel state from the first stream.
    // The first stream allocates the DSH and SSH, send the binding table.
    // The last stream sets mfe curbe, prepare and submit the command buffer.
    // All the streams set their own curbe surfaces and surface states.
    CODECHAL_ENCODE_AVC_BINDING_TABLE_MBENC origMbEncBindingTable;
    if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
    {
        auto mfeEncodeSharedState = m_mfeEncodeSharedState;
        if (m_mfeFirstStream)
        {
            mfeEncodeSharedState->pHwInterface = m_hwInterface;
            mfeEncodeSharedState->pOsInterface = m_osInterface;
            m_hwInterface->GetRenderInterface()->m_stateHeapInterface = m_stateHeapInterface;
            m_osInterface->pfnResetOsStates(m_osInterface);
        }
        else
        {
            m_hwInterface = mfeEncodeSharedState->pHwInterface;
            m_osInterface = mfeEncodeSharedState->pOsInterface;
            m_stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

        }
        // Set maximum width/height, it is used for initializing media walker parameters
        // during submitting the command buffer at the last stream.
        if (m_picWidthInMb > mfeEncodeSharedState->dwPicWidthInMB)
        {
            mfeEncodeSharedState->dwPicWidthInMB = m_picWidthInMb;
        }
        if (m_frameFieldHeightInMb > mfeEncodeSharedState->dwPicHeightInMB)
        {
            mfeEncodeSharedState->dwPicHeightInMB = m_frameFieldHeightInMb;
        }
        if (m_sliceHeight > mfeEncodeSharedState->sliceHeight)
        {
            mfeEncodeSharedState->sliceHeight = m_sliceHeight;
        }

        m_osInterface->pfnSetGpuContext(m_osInterface, m_renderContext);
        CODECHAL_DEBUG_TOOL(
            m_debugInterface->m_osInterface = m_osInterface;)
        // bookkeeping the original binding table
        origMbEncBindingTable = MbEncBindingTable;
    }

    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = (mbEncIFrameDistInUse && !m_singleTaskPhaseSupported) ?
        CODECHAL_ENCODE_PERFTAG_CALL_INTRA_DIST : CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType;
    if (mbEncIFrameDistInUse)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST;
    }
    else if (bUseMbEncAdvKernel)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_ADV;
    }
    else if (m_kernelMode == encodeNormalMode)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_NORMAL;
    }
    else if (m_kernelMode == encodePerformanceMode)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_PERFORMANCE;
    }
    else
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_QUALITY;
    }

    // Initialize DSH kernel region
    PMHW_KERNEL_STATE kernelState;
    if (mbEncIFrameDistInUse)
    {
        kernelState = &BrcKernelStates[CODECHAL_ENCODE_BRC_IDX_IFRAMEDIST];
    }
    else if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
    {
        kernelState = &mfeMbEncKernelState;
    }
    else
    {
        CodechalEncodeIdOffsetParams idOffsetParams;
        MOS_ZeroMemory(&idOffsetParams, sizeof(idOffsetParams));
        idOffsetParams.Standard           = m_standard;
        idOffsetParams.EncFunctionType    = encFunctionType;
        idOffsetParams.wPictureCodingType = m_pictureCodingType;
        idOffsetParams.ucDmvPredFlag      = m_avcSliceParams->direct_spatial_mv_pred_flag;
        idOffsetParams.interlacedField   = CodecHal_PictureIsField(m_currOriginalPic);

        uint32_t krnStateIdx;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetMbEncKernelStateIdx(
            &idOffsetParams,
            &krnStateIdx));
        kernelState = &pMbEncKernelStates[krnStateIdx];
    }

    // All the streams use the kernel state from the first stream.
    if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
    {
        if (m_mfeFirstStream)
        {
            m_mfeEncodeSharedState->pMfeMbEncKernelState = kernelState;
        }
        else
        {
            kernelState = m_mfeEncodeSharedState->pMfeMbEncKernelState;
        }
    }

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase == true || !m_singleTaskPhaseSupported ||
        (IsMfeMbEncEnabled(mbEncIFrameDistInUse) && m_mfeFirstStream))
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    // Allocate DSH and SSH for the first stream, which will be passed to other streams through the shared kernel state.
    if ((IsMfeMbEncEnabled(mbEncIFrameDistInUse) && m_mfeFirstStream) ||
        (!IsMfeMbEncEnabled(mbEncIFrameDistInUse) && !bMbEncCurbeSetInBrcUpdate))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
            m_stateHeapInterface,
            kernelState,
            false,
            0,
            false,
            m_storeData));

        MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
        MOS_ZeroMemory(&idParams, sizeof(idParams));
        idParams.pKernelState = kernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
            m_stateHeapInterface,
            1,
            &idParams));
    }

    if (bMbEncCurbeSetInBrcUpdate)
    {
        if (!IsMfeMbEncEnabled(mbEncIFrameDistInUse))
        {
            // If BRC update was used to set up the DSH & SSH, SSH only needs to
            // be obtained if single task phase is enabled because the same SSH
            // could not be shared between BRC update and MbEnc
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
                m_stateHeapInterface,
                kernelState,
                true,
                0,
                m_singleTaskPhaseSupported,
                m_storeData));
        }
    }
    else
    {
        // Setup AVC Curbe
        CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS mbEncCurbeParams;
        MOS_ZeroMemory(&mbEncCurbeParams, sizeof(mbEncCurbeParams));
        mbEncCurbeParams.pPicParams              = m_avcPicParams[ppsIdx];
        mbEncCurbeParams.pSeqParams              = m_avcSeqParams[spsIdx];
        mbEncCurbeParams.pSlcParams              = m_avcSliceParams;
        mbEncCurbeParams.ppRefList               = &(m_refList[0]);
        mbEncCurbeParams.pPicIdx                 = &(m_picIdx[0]);
        mbEncCurbeParams.bRoiEnabled             = roiEnabled;
        mbEncCurbeParams.bDirtyRoiEnabled        = dirtyRoiEnabled;
        mbEncCurbeParams.bMbEncIFrameDistEnabled = mbEncIFrameDistInUse;
        mbEncCurbeParams.pdwBlockBasedSkipEn     = &dwMbEncBlockBasedSkipEn;
        if (mbEncIFrameDistInUse)
        {
            mbEncCurbeParams.bBrcEnabled           = false;
            mbEncCurbeParams.wPicWidthInMb         = (uint16_t)m_downscaledWidthInMb4x;
            mbEncCurbeParams.wFieldFrameHeightInMb = (uint16_t)m_downscaledFrameFieldHeightInMb4x;
            mbEncCurbeParams.usSliceHeight         = (m_sliceHeight + SCALE_FACTOR_4x - 1) / SCALE_FACTOR_4x;
        }
        else
        {
            mbEncCurbeParams.bBrcEnabled           = bBrcEnabled;
            mbEncCurbeParams.wPicWidthInMb         = m_picWidthInMb;
            mbEncCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
            mbEncCurbeParams.usSliceHeight         = (m_arbitraryNumMbsInSlice) ?
                m_frameFieldHeightInMb : m_sliceHeight;
            mbEncCurbeParams.bUseMbEncAdvKernel    = bUseMbEncAdvKernel;
        }
        mbEncCurbeParams.pKernelState                     = kernelState;
        mbEncCurbeParams.pAvcQCParams                     = m_avcQCParams;
        mbEncCurbeParams.bMbDisableSkipMapEnabled         = bMbDisableSkipMapEnabled;
        mbEncCurbeParams.bStaticFrameDetectionEnabled     = bStaticFrameDetectionEnable && m_hmeEnabled;
        mbEncCurbeParams.bApdatvieSearchWindowSizeEnabled = bApdatvieSearchWindowEnable;
        mbEncCurbeParams.bSquareRollingIEnabled           = bSquareRollingIEnabled;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeAvcMbEnc(
            &mbEncCurbeParams));
    }

    if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
    {
        // Set MFE specific curbe in the last stream
        // MFE MBEnc specific curbe is different from the normal MBEnc curbe which is passed
        // to MFE MBEnc kernel as a surface.
        if (m_mfeLastStream)
        {
            CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS mfeMbEncCurbeParams;
            MOS_ZeroMemory(&mfeMbEncCurbeParams, sizeof(mfeMbEncCurbeParams));
            mfeMbEncCurbeParams.submitNumber  = m_mfeEncodeParams.submitNumber;
            mfeMbEncCurbeParams.pKernelState  = kernelState;
            mfeMbEncCurbeParams.pBindingTable = &MbEncBindingTable;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeAvcMfeMbEnc(&mfeMbEncCurbeParams));
        }
        // Change the binding table according to the index during this submission
        UpdateMfeMbEncBindingTable(m_mfeEncodeParams.submitIndex);
    }

    CODECHAL_DEBUG_TOOL(
      CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));
    )

        for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        {
            if (m_picIdx[i].bValid)
            {
                uint8_t index = m_picIdx[i].ucPicIdx;
                refList[index]->sRefBuffer = m_userFlags.bUseRawPicForRef ?
                    refList[index]->sRefRawBuffer : refList[index]->sRefReconBuffer;

                CodecHalGetResourceInfo(m_osInterface, &refList[index]->sRefBuffer);
            }
        }

    MOS_COMMAND_BUFFER cmdBuffer;
    MOS_ZeroMemory(&cmdBuffer, sizeof(cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // For MFE, All the commands are sent in the last stream and can not be sent in different streams
    // since CmdBuffer is zeroed for each stream and cmd buffer pointer is reset.
    if (!IsMfeMbEncEnabled(mbEncIFrameDistInUse) || m_mfeLastStream)
    {
        SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
        sendKernelCmdsParams.EncFunctionType = encFunctionType;
        sendKernelCmdsParams.ucDmvPredFlag   =
            m_avcSliceParams->direct_spatial_mv_pred_flag;
        sendKernelCmdsParams.pKernelState    = kernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));
    }

    // Set up MB BRC Constant Data Buffer if there is QP change within a frame
    if (mbConstDataBufferInUse)
    {
        CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS initMbBrcConstantDataBufferParams;

        MOS_ZeroMemory(&initMbBrcConstantDataBufferParams, sizeof(initMbBrcConstantDataBufferParams));
        initMbBrcConstantDataBufferParams.pOsInterface                = m_osInterface;
        initMbBrcConstantDataBufferParams.presBrcConstantDataBuffer   =
            &BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
        initMbBrcConstantDataBufferParams.dwMbEncBlockBasedSkipEn     = dwMbEncBlockBasedSkipEn;
        initMbBrcConstantDataBufferParams.pPicParams                  = m_avcPicParams[ppsIdx];
        initMbBrcConstantDataBufferParams.wPictureCodingType          = m_pictureCodingType;
        initMbBrcConstantDataBufferParams.bSkipBiasAdjustmentEnable   = m_skipBiasAdjustmentEnable;
        initMbBrcConstantDataBufferParams.bAdaptiveIntraScalingEnable = bAdaptiveIntraScalingEnable;
        initMbBrcConstantDataBufferParams.bOldModeCostEnable          = bOldModeCostEnable;
        initMbBrcConstantDataBufferParams.pAvcQCParams                = m_avcQCParams;
        initMbBrcConstantDataBufferParams.bEnableKernelTrellis        = bKernelTrellis && m_trellisQuantParams.dwTqEnabled;

        // Kernel controlled Trellis Quantization
        if (bKernelTrellis && m_trellisQuantParams.dwTqEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CalcLambdaTable(
                m_pictureCodingType,
                &initMbBrcConstantDataBufferParams.Lambda[0][0]));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMbBrcConstantDataBuffer(&initMbBrcConstantDataBufferParams));

        // dump MbBrcLut
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            initMbBrcConstantDataBufferParams.presBrcConstantDataBuffer,
            CodechalDbgAttr::attrOutput,
            "MbBrcLut",
            16 * (CODEC_AVC_NUM_QP) * sizeof(uint32_t),
            0,
            CODECHAL_MEDIA_STATE_ENC_QUALITY)));
    }

    // Add binding table
    // For MFE first stream sends binding table since the function zeros the whole SSH.
    // If last stream sends binding table it will clean the surface states from other streams.
    if (!IsMfeMbEncEnabled(mbEncIFrameDistInUse) || m_mfeFirstStream)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
            m_stateHeapInterface,
            kernelState));
    }

    //Add surface states
    CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS mbEncSurfaceParams;
    MOS_ZeroMemory(&mbEncSurfaceParams, sizeof(mbEncSurfaceParams));
    mbEncSurfaceParams.MediaStateType        = encFunctionType;
    mbEncSurfaceParams.pAvcSlcParams         = m_avcSliceParams;
    mbEncSurfaceParams.ppRefList             = &m_refList[0];
    mbEncSurfaceParams.pAvcPicIdx            = &m_picIdx[0];
    mbEncSurfaceParams.pCurrOriginalPic      = &m_currOriginalPic;
    mbEncSurfaceParams.pCurrReconstructedPic = &m_currReconstructedPic;
    mbEncSurfaceParams.wPictureCodingType    = m_pictureCodingType;
    mbEncSurfaceParams.psCurrPicSurface      = mbEncIFrameDistInUse ? m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER) : m_rawSurfaceToEnc;
    if (mbEncIFrameDistInUse && CodecHal_PictureIsBottomField(m_currOriginalPic))
    {
        mbEncSurfaceParams.dwCurrPicSurfaceOffset = m_scaledBottomFieldOffset;
    }
    mbEncSurfaceParams.dwMbCodeBottomFieldOffset          = (uint32_t)m_mbcodeBottomFieldOffset;
    mbEncSurfaceParams.dwMvBottomFieldOffset              = (uint32_t)m_mvBottomFieldOffset;
    mbEncSurfaceParams.ps4xMeMvDataBuffer                 = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer);
    mbEncSurfaceParams.ps4xMeDistortionBuffer             = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer);
    mbEncSurfaceParams.dwMeMvBottomFieldOffset            = m_hmeKernel->Get4xMeMvBottomFieldOffset();
    mbEncSurfaceParams.dwMeDistortionBottomFieldOffset    = m_hmeKernel->GetDistortionBottomFieldOffset();
    mbEncSurfaceParams.psMeBrcDistortionBuffer            = &BrcBuffers.sMeBrcDistortionBuffer;
    mbEncSurfaceParams.dwMeBrcDistortionBottomFieldOffset = BrcBuffers.dwMeBrcDistortionBottomFieldOffset;
    mbEncSurfaceParams.dwRefPicSelectBottomFieldOffset    = (uint32_t)ulRefPicSelectBottomFieldOffset;
    mbEncSurfaceParams.dwFrameWidthInMb                   = (uint32_t)m_picWidthInMb;
    mbEncSurfaceParams.dwFrameFieldHeightInMb             = (uint32_t)m_frameFieldHeightInMb;
    mbEncSurfaceParams.dwFrameHeightInMb                  = (uint32_t)m_picHeightInMb;
    // Interleaved input surfaces
    mbEncSurfaceParams.dwVerticalLineStride               = m_verticalLineStride;
    mbEncSurfaceParams.dwVerticalLineStrideOffset         = m_verticalLineStrideOffset;
    // Vertical line stride is not used for the case of scaled surfaces saved as separate fields
    if (!m_fieldScalingOutputInterleaved && mbEncIFrameDistInUse)
    {
        mbEncSurfaceParams.dwVerticalLineStride           = 0;
        mbEncSurfaceParams.dwVerticalLineStrideOffset     = 0;
    }
    mbEncSurfaceParams.bHmeEnabled                        = m_hmeSupported;
    mbEncSurfaceParams.bMbEncIFrameDistInUse              = mbEncIFrameDistInUse;
    mbEncSurfaceParams.presMbBrcConstDataBuffer           = &BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
    mbEncSurfaceParams.psMbQpBuffer                       =
        bMbQpDataEnabled ? &sMbQpDataSurface : &BrcBuffers.sBrcMbQpBuffer;
    mbEncSurfaceParams.dwMbQpBottomFieldOffset            = bMbQpDataEnabled ? 0 : BrcBuffers.dwBrcMbQpBottomFieldOffset;
    mbEncSurfaceParams.bUsedAsRef                         =
        m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef;
    mbEncSurfaceParams.presMADDataBuffer                  = &m_resMadDataBuffer[m_currMadBufferIdx];
    mbEncSurfaceParams.bMbQpBufferInUse                   = mbQpBufferInUse;
    mbEncSurfaceParams.bMbSpecificDataEnabled             = bMbSpecificDataEnabled;
    mbEncSurfaceParams.presMbSpecificDataBuffer           = &resMbSpecificDataBuffer[m_currRecycledBufIdx];
    mbEncSurfaceParams.bMbConstDataBufferInUse            = mbConstDataBufferInUse;
    mbEncSurfaceParams.bMADEnabled                        = mbEncIFrameDistInUse ? false : m_madEnabled;
    mbEncSurfaceParams.bUseMbEncAdvKernel                 = mbEncIFrameDistInUse ? false : bUseMbEncAdvKernel;
    mbEncSurfaceParams.presMbEncCurbeBuffer               =
        (mbEncIFrameDistInUse && bUseMbEncAdvKernel) ? nullptr : &BrcBuffers.resMbEncAdvancedDsh;
    mbEncSurfaceParams.presMbEncBRCBuffer = &BrcBuffers.resMbEncBrcBuffer;

    if (IsMfeMbEncEnabled(mbEncIFrameDistInUse) || bDecoupleMbEncCurbeFromBRC)
    {
        mbEncSurfaceParams.dwMbEncBRCBufferSize           = m_mbencBrcBufferSize;
    }

    mbEncSurfaceParams.bUseAdvancedDsh                    = bAdvancedDshInUse;
    mbEncSurfaceParams.bBrcEnabled                        = bBrcEnabled;
    mbEncSurfaceParams.bArbitraryNumMbsInSlice            = m_arbitraryNumMbsInSlice;
    mbEncSurfaceParams.psSliceMapSurface                  = &m_sliceMapSurface[m_currRecycledBufIdx];
    mbEncSurfaceParams.dwSliceMapBottomFieldOffset        = (uint32_t)m_sliceMapBottomFieldOffset;
    mbEncSurfaceParams.pMbEncBindingTable                 = &MbEncBindingTable;
    mbEncSurfaceParams.pKernelState                       = kernelState;

    if (m_mbStatsSupported)
    {
        mbEncSurfaceParams.bMBVProcStatsEnabled            = (m_flatnessCheckEnabled || m_adaptiveTransformDecisionEnabled);
        mbEncSurfaceParams.presMBVProcStatsBuffer          = &m_resMbStatsBuffer;
        mbEncSurfaceParams.dwMBVProcStatsBottomFieldOffset = m_mbStatsBottomFieldOffset;
    }
    else
    {
        mbEncSurfaceParams.bFlatnessCheckEnabled            = m_flatnessCheckEnabled;
        mbEncSurfaceParams.psFlatnessCheckSurface           = &m_flatnessCheckSurface;
        mbEncSurfaceParams.dwFlatnessCheckBottomFieldOffset = (uint32_t)m_flatnessCheckBottomFieldOffset;
    }

    // Set up pFeiPicParams
    mbEncSurfaceParams.pFeiPicParams             = m_avcFeiPicParams;

    mbEncSurfaceParams.bMbDisableSkipMapEnabled  = bMbDisableSkipMapEnabled;
    mbEncSurfaceParams.psMbDisableSkipMapSurface = psMbDisableSkipMapSurface;

    if (bUseWeightedSurfaceForL0 || bUseWeightedSurfaceForL1)
    {
        if (!m_wpUseCommonKernel)
        {
            mbEncSurfaceParams.pWeightedPredOutputPicSelectList = &WeightedPredOutputPicSelectList[0];
        }
        mbEncSurfaceParams.bUseWeightedSurfaceForL0 = bUseWeightedSurfaceForL0;
        mbEncSurfaceParams.bUseWeightedSurfaceForL1 = bUseWeightedSurfaceForL1;
    }

    // Clear the MAD buffer -- the kernel requires it to be 0 as it accumulates the result
    if (mbEncSurfaceParams.bMADEnabled)
    {
        // set lock flag to WRITE_ONLY
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            mbEncSurfaceParams.presMADDataBuffer,
            &lockFlags);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, CODECHAL_MAD_BUFFER_SIZE);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            mbEncSurfaceParams.presMADDataBuffer);

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resMadDataBuffer[m_currMadBufferIdx],
            CodechalDbgAttr::attrOutput,
            "MADRead",
            CODECHAL_MAD_BUFFER_SIZE,
            0,
            encFunctionType)));
    }

    // static frame detection buffer
    mbEncSurfaceParams.bStaticFrameDetectionEnabled = bStaticFrameDetectionEnable && m_hmeEnabled;
    mbEncSurfaceParams.presSFDOutputBuffer = &resSFDOutputBuffer[0];
    if (m_pictureCodingType == P_TYPE)
    {
        mbEncSurfaceParams.presSFDCostTableBuffer = &resSFDCostTablePFrameBuffer;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        mbEncSurfaceParams.presSFDCostTableBuffer = &resSFDCostTableBFrameBuffer;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendAvcMbEncSurfaces(&cmdBuffer, &mbEncSurfaceParams));

    // For MFE, only one walker processes frame in parallel through color bits.
    if (!IsMfeMbEncEnabled(mbEncIFrameDistInUse) || m_mfeLastStream)
    {
        uint32_t resolutionX = mbEncIFrameDistInUse ?
            m_downscaledWidthInMb4x : (uint32_t)m_picWidthInMb;
        uint32_t resolutionY = mbEncIFrameDistInUse ?
            m_downscaledFrameFieldHeightInMb4x : (uint32_t)m_frameFieldHeightInMb;

        CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
        MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
        walkerCodecParams.WalkerMode               = m_walkerMode;
        walkerCodecParams.bUseScoreboard           = m_useHwScoreboard;
        walkerCodecParams.wPictureCodingType       = m_pictureCodingType;
        walkerCodecParams.bMbEncIFrameDistInUse    = mbEncIFrameDistInUse;
        walkerCodecParams.bMbaff                   = m_mbaffEnabled;
        walkerCodecParams.bDirectSpatialMVPredFlag = m_avcSliceParams->direct_spatial_mv_pred_flag;
        walkerCodecParams.bColorbitSupported       = (m_colorbitSupported && !m_arbitraryNumMbsInSlice) ? m_cmKernelEnable : false;

        if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
        {
            walkerCodecParams.dwNumSlices   = m_mfeEncodeParams.submitNumber;  // MFE use color bit to handle frames in parallel
            walkerCodecParams.WalkerDegree  = CODECHAL_26_DEGREE;                        // MFE use 26 degree dependency
            walkerCodecParams.dwResolutionX = m_mfeEncodeSharedState->dwPicWidthInMB;
            walkerCodecParams.dwResolutionY = m_mfeEncodeSharedState->dwPicHeightInMB;
            walkerCodecParams.usSliceHeight = m_mfeEncodeSharedState->sliceHeight;
        }
        else
        {
            walkerCodecParams.dwResolutionX = resolutionX;
            walkerCodecParams.dwResolutionY = resolutionY;
            walkerCodecParams.dwNumSlices   = m_numSlices;
            walkerCodecParams.usSliceHeight = m_sliceHeight;
        }
        walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
        walkerCodecParams.ucGroupId = m_groupId;

        MHW_WALKER_PARAMS walkerParams;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
            m_hwInterface,
            &walkerParams,
            &walkerCodecParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
            &cmdBuffer,
            &walkerParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

        // Add dump for MBEnc surface state heap here
        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
                encFunctionType,
                MHW_SSH_TYPE,
                kernelState));
        )

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
                m_stateHeapInterface,
                kernelState));
        if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
                m_stateHeapInterface));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
        }

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            encFunctionType,
            nullptr)));
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            encFunctionType,
            nullptr)));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        if ((!m_singleTaskPhaseSupported || m_lastTaskInPhase))
        {
            HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
            m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
            m_lastTaskInPhase = false;
        }
    }

    currRefList->ucMADBufferIdx = m_currMadBufferIdx;
    currRefList->bMADEnabled    = m_madEnabled;

    if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
    {
        m_stateHeapInterface    = m_origStateHeapInterface;
        m_hwInterface           = m_origHwInterface;
        m_osInterface           = m_origOsInterface;

        MbEncBindingTable       = origMbEncBindingTable;

        CODECHAL_DEBUG_TOOL(
            m_debugInterface->m_osInterface = m_osInterface;)
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::SetAndPopulateVEHintParams(
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

MOS_STATUS CodechalEncodeAvcEncG11::ExecuteSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    auto cpInterface = m_hwInterface->GetCpInterface();
    auto avcSlcParams = m_avcSliceParams;
    auto avcPicParams = m_avcPicParams[avcSlcParams->pic_parameter_set_id];
    auto avcSeqParams = m_avcSeqParams[avcPicParams->seq_parameter_set_id];
    auto slcData = m_slcData;

    // For use with the single task phase implementation
    if (m_sliceStructCaps != CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE)
    {
        uint32_t numSlc = (m_frameFieldHeightInMb + m_sliceHeight - 1) / m_sliceHeight;

        if (numSlc != m_numSlices)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    bool useBatchBufferForPakSlices = false;
    if (m_singleTaskPhaseSupported  && m_singleTaskPhaseSupportedInPak)
    {
        if (m_currPass == 0)
        {
            // The same buffer is used for all slices for all passes.
            uint32_t batchBufferForPakSlicesSize =
                (m_numPasses + 1) * m_numSlices * m_pakSliceSize;
            if (batchBufferForPakSlicesSize >
                (uint32_t)m_batchBufferForPakSlices[m_currRecycledBufIdx].iSize)
            {
                if (m_batchBufferForPakSlices[m_currRecycledBufIdx].iSize)
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReleaseBatchBufferForPakSlices(m_currRecycledBufIdx));
                }

                CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBatchBufferForPakSlices(
                    m_numSlices,
                    m_numPasses,
                    m_currRecycledBufIdx));
            }
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_LockBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_currRecycledBufIdx]));
        useBatchBufferForPakSlices = true;
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (m_osInterface->osCpInterface->IsCpEnabled())
    {
        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.bLastPass = (m_currPass == m_numPasses) ? true : false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->SetMfxProtectionState(false, &cmdBuffer, nullptr, &sliceInfoParam));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->UpdateParams(false));
    }

    avcSlcParams = m_avcSliceParams;

    CODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS  packSlcHeaderParams;
    packSlcHeaderParams.pBsBuffer = &m_bsBuffer;
    packSlcHeaderParams.pPicParams = avcPicParams;
    packSlcHeaderParams.pSeqParams = m_avcSeqParam;
    packSlcHeaderParams.ppRefList = &(m_refList[0]);
    packSlcHeaderParams.CurrPic = m_currOriginalPic;
    packSlcHeaderParams.CurrReconPic = m_currReconstructedPic;
    packSlcHeaderParams.UserFlags = m_userFlags;
    packSlcHeaderParams.NalUnitType = m_nalUnitType;
    packSlcHeaderParams.wPictureCodingType = m_pictureCodingType;
    packSlcHeaderParams.bVdencEnabled = false;

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.presDataBuffer = &m_resMbCodeSurface;
    sliceState.pAvcPicIdx = &(m_picIdx[0]);
    sliceState.pEncodeAvcSeqParams = m_avcSeqParam;
    sliceState.pEncodeAvcPicParams = avcPicParams;
    sliceState.pBsBuffer = &m_bsBuffer;
    sliceState.ppNalUnitParams = m_nalUnitParams;
    sliceState.bBrcEnabled = bBrcEnabled;
    // Disable Panic mode when min/max QP control is on. kernel may disable it, but disable in driver also.
    if (avcSeqParams->bForcePanicModeControl == 1) {
        sliceState.bRCPanicEnable = (!avcSeqParams->bPanicModeDisable) && (!bMinMaxQPControlEnabled);
    }
    else {
        sliceState.bRCPanicEnable = m_panicEnable && (!bMinMaxQPControlEnabled);
    }
    sliceState.bAcceleratorHeaderPackingCaps = m_encodeParams.bAcceleratorHeaderPackingCaps;
    sliceState.wFrameFieldHeightInMB = m_frameFieldHeightInMb;

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;
    for (uint16_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        if (m_currPass == 0)
        {
            packSlcHeaderParams.pAvcSliceParams = &avcSlcParams[slcCount];
            if (bAcceleratorHeaderPackingCaps)
            {
                slcData[slcCount].SliceOffset = m_bsBuffer.SliceOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_PackSliceHeader(&packSlcHeaderParams));
                slcData[slcCount].BitSize = m_bsBuffer.BitSize;
            }
            if (m_sliceStructCaps != CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE)
            {
                slcData[slcCount].CmdOffset = slcCount * m_sliceHeight * m_picWidthInMb * 16 * 4;
            }
            else
            {
                slcData[slcCount].CmdOffset = packSlcHeaderParams.pAvcSliceParams->first_mb_in_slice * 16 * 4;
            }
        }

        sliceState.pEncodeAvcSliceParams = &avcSlcParams[slcCount];
        sliceState.dwDataBufferOffset =
            m_slcData[slcCount].CmdOffset + m_mbcodeBottomFieldOffset;
        sliceState.dwOffset = slcData[slcCount].SliceOffset;
        sliceState.dwLength = slcData[slcCount].BitSize;
        sliceState.uiSkipEmulationCheckCount = slcData[slcCount].SkipEmulationByteCount;
        sliceState.dwSliceIndex = (uint32_t)slcCount;
        sliceState.bFirstPass = (m_currPass == 0);
        sliceState.bLastPass = (m_currPass == m_numPasses);
        sliceState.bInsertBeforeSliceHeaders = (slcCount == 0);
        sliceState.bVdencInUse = false;
        // App handles tail insertion for VDEnc dynamic slice in non-cp case
        sliceState.bVdencNoTailInsertion = false;

        uint32_t batchBufferForPakSlicesStartOffset =
            (uint32_t)m_batchBufferForPakSlices[m_currRecycledBufIdx].iCurrent;

        if (useBatchBufferForPakSlices)
        {
            sliceState.pBatchBufferForPakSlices =
                &m_batchBufferForPakSlices[m_currRecycledBufIdx];
            sliceState.bSingleTaskPhaseSupported = true;
            sliceState.dwBatchBufferForPakSlicesStartOffset = batchBufferForPakSlicesStartOffset;
        }

        if (m_avcRoundingParams != nullptr && m_avcRoundingParams->bEnableCustomRoudingIntra)
        {
            sliceState.dwRoundingIntraValue = m_avcRoundingParams->dwRoundingIntra;
        }
        else
        {
            sliceState.dwRoundingIntraValue = 5;
        }
        if (m_avcRoundingParams != nullptr && m_avcRoundingParams->bEnableCustomRoudingInter)
        {
            sliceState.bRoundingInterEnable = true;
            sliceState.dwRoundingValue = m_avcRoundingParams->dwRoundingInter;
        }
        else
        {
            sliceState.bRoundingInterEnable = bRoundingInterEnable;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendSlice(&cmdBuffer, &sliceState));

        // Add dumps for 2nd level batch buffer
        if (sliceState.bSingleTaskPhaseSupported)
        {
            CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState.pBatchBufferForPakSlices);

            CODECHAL_DEBUG_TOOL(
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
                    sliceState.pBatchBufferForPakSlices,
                    CODECHAL_MEDIA_STATE_ENC_NORMAL,
                    nullptr));
            )
        }
    }

    if (useBatchBufferForPakSlices)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_currRecycledBufIdx],
            m_lastTaskInPhase));
    }

    // Insert end of sequence/stream if set
    if (m_lastPicInStream || m_lastPicInSeq)
    {
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastPicInSeq = m_lastPicInSeq;
        pakInsertObjectParams.bLastPicInStream = m_lastPicInStream;
        pakInsertObjectParams.dwBitSize = 32;   // use dwBitSize for SrcDataEndingBitInclusion
        if (m_lastPicInSeq)
        {
            pakInsertObjectParams.dwLastPicInSeqData = (uint32_t)((1 << 16) | CODECHAL_ENCODE_AVC_NAL_UT_EOSEQ << 24);
        }
        if (m_lastPicInStream)
        {
            pakInsertObjectParams.dwLastPicInStreamData = (uint32_t)((1 << 16) | CODECHAL_ENCODE_AVC_NAL_UT_EOSTREAM << 24);
        }
        pakInsertObjectParams.bHeaderLengthExcludeFrmSize = true;
        if (pakInsertObjectParams.bEmulationByteBitsInsert)
        {
            //Does not matter here, but keeping for consistency
            CODECHAL_ENCODE_ASSERTMESSAGE("The emulation prevention bytes are not inserted by the app and are requested to be inserted by HW.");
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr, &pakInsertObjectParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadMfcStatus(&cmdBuffer));

    // BRC PAK statistics different for each pass
    if (bBrcEnabled)
    {
        uint32_t offset =
            (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
            m_encodeStatusBuf.dwNumPassesOffset +   // Num passes offset
            sizeof(uint32_t) * 2;                                                                  // pEncodeStatus is offset by 2 DWs in the resource

        EncodeReadBrcPakStatsParams   readBrcPakStatsParams;
        readBrcPakStatsParams.pHwInterface = m_hwInterface;
        readBrcPakStatsParams.presBrcPakStatisticBuffer = &BrcBuffers.resBrcPakStatisticBuffer[0];
        readBrcPakStatsParams.presStatusBuffer = &m_encodeStatusBuf.resStatusBuffer;
        readBrcPakStatsParams.dwStatusBufNumPassesOffset = offset;
        readBrcPakStatsParams.ucPass = m_currPass;
        readBrcPakStatsParams.VideoContext = m_videoContext;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStatistics(&cmdBuffer, &readBrcPakStatsParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    std::string pakPassName = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_currPass));
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            pakPassName.data()));
    //CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
    //    m_debugInterface,
    //    &cmdBuffer));
    )

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    // For VDEnc SHME and CSC need to wait workload finish on render engine
    if ((m_currPass == 0) && !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        auto syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    bool renderingFlags = m_videoContextUsesNullHw;

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(&cmdBuffer));
        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, renderingFlags));

        CODECHAL_DEBUG_TOOL(
            if (!m_mmcUserFeatureUpdated) {
                CODECHAL_UPDATE_ENCODE_MMC_USER_FEATURE(m_reconSurface);
                m_mmcUserFeatureUpdated = true;
            })

            if (m_sliceSizeStreamoutSupported)
            {
                CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_pakSliceSizeStreamoutBuffer,
                    CodechalDbgAttr::attrOutput,
                    "SliceSizeStreamout",
                    CODECHAL_ENCODE_SLICESIZE_BUF_SIZE,
                    0,
                    CODECHAL_NUM_MEDIA_STATES)));
            }

            if ((m_currPass == m_numPasses) &&
                m_signalEnc &&
                !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
            {
                // Check if the signal obj count exceeds max value
                if (m_semaphoreObjCount == MOS_MIN(m_semaphoreMaxCount, MOS_MAX_OBJECT_SIGNALED))
                {
                    auto syncParams = g_cInitSyncParams;
                    syncParams.GpuContext = m_renderContext;
                    syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
                    m_semaphoreObjCount--;
                }

                // signal semaphore
                auto syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_videoContext;
                syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
                m_semaphoreObjCount++;
            }
    }

    // Reset parameters for next PAK execution
    if (m_currPass == m_numPasses)
    {
        if (!m_singleTaskPhaseSupported)
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateSliceStateParam(
            bAdaptiveRoundingInterEnable,
            &sliceState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpFrameParFile());
    )

        return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    auto slcParams = m_avcSliceParams;
    auto sliceType = Slice_Type[slcParams->slice_type];

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_rawSurfaceToEnc,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf"));
    )

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_cscDsState);

    // Scaling, BRC Init/Reset, HME and SWSB Init are included in the same task phase
    m_lastEncPhase = false;
    m_firstTaskInPhase = true;

    // Check if SW scoreboard init is needed
    auto swScoreboardInitNeeded = true;
    CodechalEncodeSwScoreboard::KernelParams swScoreboardKernelParames;
    MOS_ZeroMemory(&swScoreboardKernelParames, sizeof(swScoreboardKernelParames));

    // Decide dependency pattern
    if (m_pictureCodingType == I_TYPE ||
        (m_pictureCodingType == B_TYPE && !m_avcSliceParams->direct_spatial_mv_pred_flag)) // I or B-temporal
    {
        swScoreboardKernelParames.surfaceIndex = dependencyWavefront45Degree;
        m_swScoreboardState->SetDependencyPattern(dependencyWavefront45Degree);
    }
    else //P or B-spatial
    {
        swScoreboardKernelParames.surfaceIndex = dependencyWavefront26Degree;
        m_swScoreboardState->SetDependencyPattern(dependencyWavefront26Degree);
    }

    m_swScoreboardState->SetCurSwScoreboardSurfaceIndex(swScoreboardKernelParames.surfaceIndex);

    // Only called if current pattern was not initialized before
    if (Mos_ResourceIsNull(&m_swScoreboardState->GetCurSwScoreboardSurface()->OsResource))
    {
        swScoreboardInitNeeded = true;
    }

    // BRC init/reset needs to be called before HME since it will reset the Brc Distortion surface
    if ((bBrcEnabled || m_avcPicParam->bEnableQpAdjustment) && (bBrcInit || bBrcReset))
    {
        bool cscEnabled = m_cscDsState->RequireCsc() && m_firstField;
        m_lastTaskInPhase = !(cscEnabled || m_scalingEnabled || m_16xMeSupported || m_hmeEnabled || swScoreboardInitNeeded);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcInitResetKernel());
    }

    UpdateSSDSliceCount();

    if (m_firstField)
    {
        // Csc, Downscaling, and/or 10-bit to 8-bit conversion
        CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
        MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
        cscScalingKernelParams.bLastTaskInPhaseCSC =
            cscScalingKernelParams.bLastTaskInPhase4xDS = !(m_16xMeSupported || m_hmeEnabled);
        cscScalingKernelParams.bLastTaskInPhase16xDS = !(m_32xMeSupported || m_hmeEnabled);
        cscScalingKernelParams.bLastTaskInPhase32xDS = !m_hmeEnabled;
        cscScalingKernelParams.inputColorSpace = m_avcSeqParam->InputColorSpace;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));
    }

    // SFD should be called only when HME enabled
    auto staticFrameDetectionInUse = bStaticFrameDetectionEnable && m_hmeEnabled;

    // temp for SKL, once perMB pulled-in to BDW/HSW this can be removed
    staticFrameDetectionInUse = !bPerMbSFD && staticFrameDetectionInUse;

    if (m_hmeKernel && m_hmeKernel->Is4xMeEnabled())
    {
        CodechalKernelHme::CurbeParam curbeParam = {};
        curbeParam.subPelMode = 3;
        curbeParam.currOriginalPic = m_avcPicParam->CurrOriginalPic;
        curbeParam.qpPrimeY = m_avcPicParam->pic_init_qp_minus26 + 26 + m_avcSliceParams->slice_qp_delta;
        curbeParam.targetUsage = m_avcSeqParam->TargetUsage;
        curbeParam.maxMvLen = CodecHalAvcEncode_GetMaxMvLen(m_avcSeqParam->Level);
        curbeParam.numRefIdxL0Minus1 = m_avcSliceParams->num_ref_idx_l0_active_minus1;
        curbeParam.numRefIdxL1Minus1 = m_avcSliceParams->num_ref_idx_l1_active_minus1;

        auto slcParams = m_avcSliceParams;
        curbeParam.list0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        curbeParam.list0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        curbeParam.list0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        curbeParam.list0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        curbeParam.list0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        curbeParam.list0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        curbeParam.list0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        curbeParam.list0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        curbeParam.list1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        curbeParam.list1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);

        CodechalKernelHme::SurfaceParams surfaceParam = {};
        surfaceParam.mbaffEnabled = m_mbaffEnabled;
        surfaceParam.numRefIdxL0ActiveMinus1 = m_avcSliceParams->num_ref_idx_l0_active_minus1;
        surfaceParam.numRefIdxL1ActiveMinus1 = m_avcSliceParams->num_ref_idx_l1_active_minus1;
        surfaceParam.verticalLineStride = m_verticalLineStride;
        surfaceParam.verticalLineStrideOffset = m_verticalLineStrideOffset;
        surfaceParam.meBrcDistortionBottomFieldOffset = BrcBuffers.dwMeBrcDistortionBottomFieldOffset;
        surfaceParam.refList = &m_refList[0];
        surfaceParam.picIdx = &m_picIdx[0];
        surfaceParam.currOriginalPic = &m_currOriginalPic;
        surfaceParam.refL0List = &(m_avcSliceParams->RefPicList[LIST_0][0]);
        surfaceParam.refL1List = &(m_avcSliceParams->RefPicList[LIST_1][0]);
        surfaceParam.meBrcDistortionBuffer = &BrcBuffers.sMeBrcDistortionBuffer;

        if (m_hmeKernel->Is16xMeEnabled())
        {
            m_lastTaskInPhase = false;
            if (m_hmeKernel->Is32xMeEnabled())
            {
                surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb32x;
                surfaceParam.downScaledHeightInMb = m_downscaledFrameFieldHeightInMb32x;
                surfaceParam.downScaledBottomFieldOffset = m_scaled32xBottomFieldOffset;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel32x));
            }
            surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb16x;
            surfaceParam.downScaledHeightInMb = m_downscaledFrameFieldHeightInMb16x;
            surfaceParam.downScaledBottomFieldOffset = m_scaled16xBottomFieldOffset;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel16x));
        }
        surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb4x;
        surfaceParam.downScaledHeightInMb = m_downscaledFrameFieldHeightInMb4x;
        surfaceParam.downScaledBottomFieldOffset = m_scaledBottomFieldOffset;

        m_lastTaskInPhase = !(staticFrameDetectionInUse || swScoreboardInitNeeded);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel4x));
    }

    // Initialize software scoreboard used by MBEnc kernel
    if (swScoreboardInitNeeded)
    {
        // Call SW scoreboard Init kernel
        m_lastTaskInPhase = true;
        swScoreboardKernelParames.scoreboardWidth           = m_picWidthInMb;
        swScoreboardKernelParames.scoreboardHeight          = m_frameFieldHeightInMb;
        swScoreboardKernelParames.swScoreboardSurfaceWidth  = swScoreboardKernelParames.scoreboardWidth * 4;
        swScoreboardKernelParames.swScoreboardSurfaceHeight = swScoreboardKernelParames.scoreboardHeight;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_swScoreboardState->Execute(&swScoreboardKernelParames));
    }

    // Scaling and HME are not dependent on the output from PAK
    if (m_waitForPak && m_semaphoreObjCount && !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
    {
        // Wait on PAK
        auto syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;
        syncParams.uiSemaphoreCount = m_semaphoreObjCount;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
        m_semaphoreObjCount = 0; //reset
    }

    // BRC and MbEnc are included in the same task phase
    m_lastEncPhase = true;
    m_firstTaskInPhase = true;

    if (bBrcEnabled)
    {
        if (bMbEncIFrameDistEnabled)
        {
            CodechalKernelIntraDist::CurbeParam curbeParam;
            curbeParam.downScaledWidthInMb4x  = m_downscaledWidthInMb4x;
            curbeParam.downScaledHeightInMb4x = m_downscaledFrameFieldHeightInMb4x;
            CodechalKernelIntraDist::SurfaceParams surfaceParam;
            surfaceParam.input4xDsSurface           =
            surfaceParam.input4xDsVmeSurface        = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            surfaceParam.intraDistSurface           = &BrcBuffers.sMeBrcDistortionBuffer;
            surfaceParam.intraDistBottomFieldOffset = BrcBuffers.dwMeBrcDistortionBottomFieldOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_intraDistKernel->Execute(curbeParam, surfaceParam));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcFrameUpdateKernel());
        if (bBrcSplitEnable && bMbBrcEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcMbUpdateKernel());
        }

        // Reset buffer ID used for BRC kernel performance reports
        m_osInterface->pfnResetPerfBufferID(m_osInterface);
    }
    else if (bMbBrcEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcMbUpdateKernel());

        // Reset buffer ID used for BRC kernel performance reports
        m_osInterface->pfnResetPerfBufferID(m_osInterface);
    }

    bUseWeightedSurfaceForL0 = false;
    bUseWeightedSurfaceForL1 = false;

    if (bWeightedPredictionSupported &&
        ((((sliceType == SLICE_P) || (sliceType == SLICE_SP)) && (m_avcPicParam->weighted_pred_flag)) ||
        (((sliceType == SLICE_B)) && (m_avcPicParam->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE))))
    {
        CodechalEncodeWP::SliceParams sliceWPParams;
        memset((void *)&sliceWPParams,0, sizeof(sliceWPParams));
        MOS_SecureMemcpy(&sliceWPParams.weights, sizeof(m_avcSliceParams->Weights), &m_avcSliceParams->Weights, sizeof(m_avcSliceParams->Weights));
        sliceWPParams.luma_log2_weight_denom = m_avcSliceParams->luma_log2_weight_denom;

        CodechalEncodeWP::KernelParams wpkernelParams;
        memset((void *)&wpkernelParams, 0, sizeof(wpkernelParams));
        wpkernelParams.useWeightedSurfaceForL0 = &bUseWeightedSurfaceForL0;
        wpkernelParams.useWeightedSurfaceForL1 = &bUseWeightedSurfaceForL1;
        wpkernelParams.slcWPParams             = &sliceWPParams;

        // Weighted Prediction to be applied for L0
        for (auto i = 0; i < (m_avcPicParam->num_ref_idx_l0_active_minus1 + 1); i++)
        {
            if((slcParams->luma_weight_flag[LIST_0] & (1 << i)) && (i < CODEC_AVC_MAX_FORWARD_WP_FRAME))
            {
                CODEC_PICTURE refPic   = slcParams->RefPicList[LIST_0][i];
                if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
                {
                    uint8_t frameIndex = m_picIdx[refPic.FrameIdx].ucPicIdx;
                    MOS_SURFACE refFrameInput = m_userFlags.bUseRawPicForRef?
                        m_refList[frameIndex]->sRefRawBuffer : m_refList[frameIndex]->sRefReconBuffer;

                    //Weighted Prediction for ith forward reference frame
                    wpkernelParams.useRefPicList1   = false;
                    wpkernelParams.wpIndex          = i;
                    wpkernelParams.refFrameInput    = &refFrameInput;
                    wpkernelParams.refIsBottomField = CodecHal_PictureIsBottomField(refPic);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_wpState->Execute(&wpkernelParams));
                }
            }
        }

        // Weighted Pred to be applied for L1
        if (((sliceType == SLICE_B)) &&
            (m_avcPicParam->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE))
        {
            for (auto i = 0; i < (m_avcPicParam->num_ref_idx_l1_active_minus1 + 1); i++)
            {
                if((slcParams->luma_weight_flag[LIST_1] & 1 << i) && (i < CODEC_AVC_MAX_BACKWARD_WP_FRAME))
                {
                    CODEC_PICTURE refPic   = slcParams->RefPicList[LIST_1][i];
                    if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
                    {
                        uint8_t frameIndex = m_picIdx[refPic.FrameIdx].ucPicIdx;
                        MOS_SURFACE refFrameInput = m_userFlags.bUseRawPicForRef?
                            m_refList[frameIndex]->sRefRawBuffer : m_refList[frameIndex]->sRefReconBuffer;

                        //Weighted Prediction for ith backward reference frame
                        wpkernelParams.useRefPicList1   = true;
                        wpkernelParams.wpIndex          = i;
                        wpkernelParams.refFrameInput    = &refFrameInput;
                        wpkernelParams.refIsBottomField = CodecHal_PictureIsBottomField(refPic);
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_wpState->Execute(&wpkernelParams));
                    }
                }
            }
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_WRITE_DATA   userFeatureWriteData;
    // Weighted prediction for L0 Reporting
    userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = bUseWeightedSurfaceForL0;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L0_IN_USE_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
    // Weighted prediction for L1 Reporting
    userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = bUseWeightedSurfaceForL1;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L1_IN_USE_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);
#endif // _DEBUG || _RELEASE_INTERNAL

    m_lastTaskInPhase = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel(false));

    // Reset buffer ID used for MbEnc kernel performance reports
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        auto syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
    }

    if (m_madEnabled)
    {
        m_currMadBufferIdx = (m_currMadBufferIdx + 1) % CODECHAL_ENCODE_MAX_NUM_MAD_BUFFERS;
    }

    // Reset after BRC Init has been processed
    bBrcInit = false;

    m_setRequestedEUSlices = false;

    CODECHAL_DEBUG_TOOL(
        if (m_hmeEnabled)
        {
            CODECHAL_ME_OUTPUT_PARAMS MeOutputParams;

            MOS_ZeroMemory(&MeOutputParams, sizeof(MeOutputParams));
            MeOutputParams.psMeMvBuffer = m_hmeKernel ? m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer) : &m_4xMeMvDataBuffer;
            MeOutputParams.psMeBrcDistortionBuffer =
                bBrcDistortionBufferSupported ? &BrcBuffers.sMeBrcDistortionBuffer : nullptr;
            MeOutputParams.psMeDistortionBuffer =
                m_4xMeDistortionBufferSupported ?
                (m_hmeKernel ? m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer) : &m_4xMeDistortionBuffer) : nullptr;
            MeOutputParams.b16xMeInUse = false;
            MeOutputParams.b32xMeInUse = false;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &MeOutputParams.psMeMvBuffer->OsResource,
                CodechalDbgAttr::attrOutput,
                "MvData",
                MeOutputParams.psMeMvBuffer->dwHeight *MeOutputParams.psMeMvBuffer->dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));

            if (MeOutputParams.psMeBrcDistortionBuffer)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &MeOutputParams.psMeBrcDistortionBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "BrcDist",
                    MeOutputParams.psMeBrcDistortionBuffer->dwHeight *MeOutputParams.psMeBrcDistortionBuffer->dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4), 8) : 0,
                    CODECHAL_MEDIA_STATE_4X_ME));
                if (MeOutputParams.psMeDistortionBuffer)
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                        &MeOutputParams.psMeDistortionBuffer->OsResource,
                        CodechalDbgAttr::attrOutput,
                        "MeDist",
                        MeOutputParams.psMeDistortionBuffer->dwHeight *MeOutputParams.psMeDistortionBuffer->dwPitch,
                        CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4 * 10), 8) : 0,
                        CODECHAL_MEDIA_STATE_4X_ME));
                }
            }

            if (m_16xMeEnabled)
            {
                MeOutputParams.psMeMvBuffer = m_hmeKernel ? m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me16xMvDataBuffer) : &m_16xMeMvDataBuffer;
                MeOutputParams.psMeBrcDistortionBuffer = nullptr;
                MeOutputParams.psMeDistortionBuffer = nullptr;
                MeOutputParams.b16xMeInUse = true;
                MeOutputParams.b32xMeInUse = false;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(
                    m_debugInterface->DumpBuffer(
                        &MeOutputParams.psMeMvBuffer->OsResource,
                        CodechalDbgAttr::attrOutput,
                        "MvData",
                        MeOutputParams.psMeMvBuffer->dwHeight *MeOutputParams.psMeMvBuffer->dwPitch,
                        CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) * (m_downscaledFrameFieldHeightInMb16x * 4) : 0,
                        CODECHAL_MEDIA_STATE_16X_ME));

                if (m_32xMeEnabled)
                {
                    MeOutputParams.psMeMvBuffer = m_hmeKernel ? m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me32xMvDataBuffer) : &m_32xMeMvDataBuffer;
                    MeOutputParams.psMeBrcDistortionBuffer = nullptr;
                    MeOutputParams.psMeDistortionBuffer = nullptr;
                    MeOutputParams.b16xMeInUse = false;
                    MeOutputParams.b32xMeInUse = true;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(
                        m_debugInterface->DumpBuffer(
                            &MeOutputParams.psMeMvBuffer->OsResource,
                            CodechalDbgAttr::attrOutput,
                            "MvData",
                            MeOutputParams.psMeMvBuffer->dwHeight *MeOutputParams.psMeMvBuffer->dwPitch,
                            CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64) * (m_downscaledFrameFieldHeightInMb32x * 4) : 0,
                            CODECHAL_MEDIA_STATE_32X_ME));
                }
            }
        }

        if (bBrcEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &BrcBuffers.resBrcImageStatesWriteBuffer,
                CodechalDbgAttr::attrOutput,
                "ImgStateWrite",
                BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses(),
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &BrcBuffers.resBrcHistoryBuffer,
                CodechalDbgAttr::attrOutput,
                "HistoryWrite",
                m_brcHistoryBufferSize,
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
            if (!Mos_ResourceIsNull(&BrcBuffers.sBrcMbQpBuffer.OsResource))
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &BrcBuffers.sBrcMbQpBuffer.OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MbQp",
                    BrcBuffers.sBrcMbQpBuffer.dwPitch*BrcBuffers.sBrcMbQpBuffer.dwHeight,
                    BrcBuffers.dwBrcMbQpBottomFieldOffset,
                    CODECHAL_MEDIA_STATE_MB_BRC_UPDATE));
            }
            if (BrcBuffers.pMbEncKernelStateInUse)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
                    CODECHAL_MEDIA_STATE_BRC_UPDATE,
                    BrcBuffers.pMbEncKernelStateInUse));
            }
            if (m_mbencBrcBufferSize > 0)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &BrcBuffers.resMbEncBrcBuffer,
                    CodechalDbgAttr::attrOutput,
                    "MbEncBRCWrite",
                    m_mbencBrcBufferSize,
                    0,
                    CODECHAL_MEDIA_STATE_BRC_UPDATE));
            }

            if (m_mbStatsSupported)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_resMbStatsBuffer,
                    CodechalDbgAttr::attrOutput,
                    "MBStatsSurf",
                    m_picWidthInMb * (((CodecHal_PictureIsField(m_currOriginalPic)) ? 2 : 4) * m_downscaledFrameFieldHeightInMb4x) * 16 * sizeof(uint32_t),
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? (m_picWidthInMb * 16 * sizeof(uint32_t) * (2 * m_downscaledFrameFieldHeightInMb4x)) : 0,
                    CODECHAL_MEDIA_STATE_4X_SCALING));
            }
            else if (m_flatnessCheckEnabled) {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_flatnessCheckSurface.OsResource,
                    CodechalDbgAttr::attrOutput,
                    "FlatnessChkSurf",
                    ((CodecHal_PictureIsField(m_currOriginalPic)) ? m_flatnessCheckSurface.dwHeight / 2 : m_flatnessCheckSurface.dwHeight) * m_flatnessCheckSurface.dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? (m_flatnessCheckSurface.dwPitch * m_flatnessCheckSurface.dwHeight >> 1) : 0,
                    CODECHAL_MEDIA_STATE_4X_SCALING));
            }
            if (bMbQpDataEnabled) {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &sMbQpDataSurface.OsResource,
                    CodechalDbgAttr::attrInput,
                    "MbQp",
                    sMbQpDataSurface.dwHeight*sMbQpDataSurface.dwPitch,
                    0,
                    CODECHAL_MEDIA_STATE_ENC_QUALITY));
            }
        }
        if (bMbSpecificDataEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &resMbSpecificDataBuffer[m_currRecycledBufIdx],
                CodechalDbgAttr::attrInput,
                "MbSpecificData",
                m_picWidthInMb*m_frameFieldHeightInMb*16,
                0,
                CODECHAL_MEDIA_STATE_ENC_QUALITY));
        }

        uint8_t         index;
        CODEC_PICTURE   refPic;
        if (bUseWeightedSurfaceForL0)
        {
            refPic = m_avcSliceParams->RefPicList[LIST_0][0];
            index = m_picIdx[refPic.FrameIdx].ucPicIdx;

            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_refList[index]->sRefBuffer,
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_In_L0")));

            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                m_wpState->GetWPOutputPicList(CODEC_WP_OUTPUT_L0_START + 0),
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_Out_L0")));
        }
        if (bUseWeightedSurfaceForL1)
        {
            refPic = m_avcSliceParams->RefPicList[LIST_1][0];
            index = m_picIdx[refPic.FrameIdx].ucPicIdx;

            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_refList[index]->sRefBuffer,
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_In_L1")));

            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                m_wpState->GetWPOutputPicList(CODEC_WP_OUTPUT_L1_START + 0),
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_Out_L1")));
        }

        if (m_feiEnable)
        {
             if(m_avcFeiPicParams->bMBQp){
                 CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                     &m_avcFeiPicParams->resMBQp,
                     CodechalDbgAttr::attrInput,
                     "MbQp",
                     m_picWidthInMb * m_frameFieldHeightInMb + 3,
                     0,
                     CODECHAL_MEDIA_STATE_ENC_QUALITY));
             }
             if (m_avcFeiPicParams->MVPredictorEnable){
                 CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_avcFeiPicParams->resMVPredictor,
                    CodechalDbgAttr::attrInput,
                    "MvPredictor",
                    m_picWidthInMb * m_frameFieldHeightInMb *40,
                    0,
                    CODECHAL_MEDIA_STATE_ENC_QUALITY));
             }
        }

        if (m_arbitraryNumMbsInSlice)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_sliceMapSurface[m_currRecycledBufIdx].OsResource,
                CodechalDbgAttr::attrInput,
                "SliceMapSurf",
                m_sliceMapSurface[m_currRecycledBufIdx].dwPitch * m_frameFieldHeightInMb,
                0,
                CODECHAL_MEDIA_STATE_ENC_QUALITY));
        }

        // Dump SW scoreboard surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &(m_swScoreboardState->GetCurSwScoreboardSurface())->OsResource,
            CodechalDbgAttr::attrOutput,
            "Out",
            (m_swScoreboardState->GetCurSwScoreboardSurface())->dwHeight * (m_swScoreboardState->GetCurSwScoreboardSurface())->dwPitch,
            0,
            CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT));
        )

    if (bBrcEnabled)
    {
        bMbEncCurbeSetInBrcUpdate = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::SceneChangeReport(PMOS_COMMAND_BUFFER    cmdBuffer, PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS params)
{

    MHW_MI_COPY_MEM_MEM_PARAMS                      copyMemMemParams;
    uint32_t offset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize)
        + (sizeof(uint32_t) * 2) + m_encodeStatusBuf.dwSceneChangedOffset;

    MOS_ZeroMemory(&copyMemMemParams, sizeof(copyMemMemParams));
    copyMemMemParams.presSrc = params->presBrcHistoryBuffer;
    copyMemMemParams.dwSrcOffset = brcHistoryBufferOffsetSceneChanged;
    copyMemMemParams.presDst = &m_encodeStatusBuf.resStatusBuffer;
    copyMemMemParams.dwDstOffset = offset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(
        cmdBuffer,
        &copyMemMemParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG11::InitializeState()
{
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::InitializeState());

    m_brcHistoryBufferSize = brcHistoryBufferSize;
    m_mbencBrcBufferSize = mbencBrcBufferSize;
    m_forceBrcMbStatsEnabled = true;
    m_useHwScoreboard = false;

    dwBrcConstantSurfaceWidth = brcConstantsurfaceWidth;
    dwBrcConstantSurfaceHeight = brcConstantsurfaceHeight;

    // Create weighted prediction kernel state
    m_wpUseCommonKernel = true;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_wpState = MOS_New(CodechalEncodeWPG11, this));
    m_wpState->SetKernelBase(m_kernelBase);
    // create intra distortion kernel
    m_intraDistKernel = MOS_New(CodechalKernelIntraDist, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_intraDistKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_intraDistKernel->Initialize(
        GetCommonKernelHeaderAndSizeG11,
        m_kernelBase,
        m_kuidCommon));

    // Create SW scoreboard init kernel state
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_swScoreboardState = MOS_New(CodechalEncodeSwScoreboardG11, this));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_swScoreboardState->InitKernelState());

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        m_sinlgePipeVeState = (PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE));
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_sinlgePipeVeState);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_InitInterface(m_hwInterface, m_sinlgePipeVeState));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG11::InitKernelStateBrc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_KERNEL_HEADER currKrnHeader;
    for (uint32_t krnStateIdx = 0; krnStateIdx < CODECHAL_ENCODE_BRC_IDX_NUM; krnStateIdx++)
    {
        auto kernelStatePtr = &BrcKernelStates[krnStateIdx];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            kernelBinary,
            ENC_BRC,
            krnStateIdx,
            (void*)&currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount = m_brcBTCounts[krnStateIdx];
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = m_brcCurbeSize[krnStateIdx];
        kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount = 1;

        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &BrcUpdateBindingTable;
    bindingTable->dwFrameBrcHistoryBuffer = frameBrcUpdateHistory;
    bindingTable->dwFrameBrcPakStatisticsOutputBuffer = frameBrcUpdatePakStatisticsOutput;
    bindingTable->dwFrameBrcImageStateReadBuffer = frameBrcUpdateImageStateRead;
    bindingTable->dwFrameBrcImageStateWriteBuffer = frameBrcUpdateImageStateWrite;

    bindingTable->dwFrameBrcMbEncCurbeWriteData = frameBrcUpdateMbencCurbeWrite;
    bindingTable->dwFrameBrcDistortionBuffer = frameBrcUpdateDistortion;
    bindingTable->dwFrameBrcConstantData = frameBrcUpdateConstantData;
    bindingTable->dwFrameBrcMbStatBuffer = frameBrcUpdateMbStat;
    bindingTable->dwFrameBrcMvDataBuffer = frameBrcUpdateMvStat;
    // starting GEN9 BRC kernel has split into a frame level update, and an MB level update.
    // above is BTI for frame level, below is BTI for MB level
    bindingTable->dwMbBrcHistoryBuffer = mbBrcUpdateHistory;
    bindingTable->dwMbBrcMbQpBuffer = mbBrcUpdateMbQp;
    bindingTable->dwMbBrcROISurface = mbBrcUpdateRoi;
    bindingTable->dwMbBrcMbStatBuffer = mbBrcUpdateMbStat;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS    params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS          trellisQuantParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(trellisQuantParams);

    trellisQuantParams->dwTqEnabled    = TrellisQuantizationEnable[params->ucTargetUsage];
    trellisQuantParams->dwTqRounding   =
    trellisQuantParams->dwTqEnabled ? trellisQuantizationRounding[params->ucTargetUsage]  : 0;

    // If AdaptiveTrellisQuantization is enabled then disable trellis quantization for
    // B-frames with QP > 26 only in CQP mode
    if(trellisQuantParams->dwTqEnabled
        && EnableAdaptiveTrellisQuantization[params->ucTargetUsage]
        && params->wPictureCodingType == B_TYPE
        && !params->bBrcEnabled && params->ucQP > 26)
    {
        trellisQuantParams->dwTqEnabled  = 0;
        trellisQuantParams->dwTqRounding = 0;
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::GetMbEncKernelStateIdx(CodechalEncodeIdOffsetParams* params, uint32_t* kernelOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelOffset);

    *kernelOffset = mbencIOffset;

    if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_ADV)
    {
        *kernelOffset +=
            mbencNumTargetUsages * mbencFrameTypeNum;
    }
    else
    {
        if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_NORMAL)
        {
            *kernelOffset += mbencFrameTypeNum;
        }
        else if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_PERFORMANCE)
        {
            *kernelOffset += mbencFrameTypeNum * 2;
        }
    }

    if (params->wPictureCodingType == P_TYPE)
    {
        *kernelOffset += mbencPOffset;
    }
    else if (params->wPictureCodingType == B_TYPE)
    {
        *kernelOffset += mbencBOffset;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    dwNumMbEncEncKrnStates =
         mbencNumTargetUsages * mbencFrameTypeNum;
    dwNumMbEncEncKrnStates += mbencFrameTypeNum;
    pMbEncKernelStates =
        MOS_NewArray(MHW_KERNEL_STATE, dwNumMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pMbEncKernelStates);

    auto kernelStatePtr = pMbEncKernelStates;
    CODECHAL_KERNEL_HEADER currKrnHeader;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    for (uint32_t krnStateIdx = 0; krnStateIdx < dwNumMbEncEncKrnStates; krnStateIdx++)
    {
        bool kernelState = (krnStateIdx >= mbencNumTargetUsages * mbencFrameTypeNum);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            kernelBinary,
            (kernelState ? ENC_MBENC_ADV : ENC_MBENC),
            (kernelState ? krnStateIdx - mbencNumTargetUsages * mbencFrameTypeNum : krnStateIdx),
            &currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount     = mbencNumSurfaces;
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = sizeof(MbencCurbe);
        kernelStatePtr->KernelParams.iBlockWidth  = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount     = 1;

        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

        kernelStatePtr++;
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &MbEncBindingTable;

    bindingTable->dwAvcMBEncMfcAvcPakObj             = mbencMfcAvcPakObj;
    bindingTable->dwAvcMBEncIndMVData                = mbencIndMvData;
    bindingTable->dwAvcMBEncBRCDist                  = mbencBrcDistortion;
    bindingTable->dwAvcMBEncCurrY                    = mbencCurrY;
    bindingTable->dwAvcMBEncCurrUV                   = mbencCurrUv;
    bindingTable->dwAvcMBEncMbSpecificData           = mbencMbSpecificData;

    bindingTable->dwAvcMBEncRefPicSelectL0           = mbencRefpicselectL0;
    bindingTable->dwAvcMBEncMVDataFromME             = mbencMvDataFromMe;
    bindingTable->dwAvcMBEncMEDist                   = mbenc4xMeDistortion;
    bindingTable->dwAvcMBEncSliceMapData             = mbencSlicemapData;
    bindingTable->dwAvcMBEncBwdRefMBData             = mbencFwdMbData;
    bindingTable->dwAvcMBEncBwdRefMVData             = mbencFwdMvData;
    bindingTable->dwAvcMBEncMbBrcConstData           = mbencMbbrcConstData;
    bindingTable->dwAvcMBEncMBStats                  = mbencMbStats;
    bindingTable->dwAvcMBEncMADData                  = mbencMadData;
    bindingTable->dwAvcMBEncMbNonSkipMap             = mbencForceNonskipMbMap;
    bindingTable->dwAvcMBEncAdv                      = mbEncAdv;
    bindingTable->dwAvcMbEncBRCCurbeData             = mbencBrcCurbeData;
    bindingTable->dwAvcMBEncStaticDetectionCostTable = mbencSfdCostTable;

    // Frame
    bindingTable->dwAvcMBEncMbQpFrame                = mbencMbqp;
    bindingTable->dwAvcMBEncCurrPicFrame[0]          = mbencVmeInterPredCurrPicIdx0;
    bindingTable->dwAvcMBEncFwdPicFrame[0]           = mbencVmeInterPredFwdPicIDX0;
    bindingTable->dwAvcMBEncBwdPicFrame[0]           = mbencVmeInterPredBwdPicIDX00;
    bindingTable->dwAvcMBEncFwdPicFrame[1]           = mbencVmeInterPredFwdPicIDX1;
    bindingTable->dwAvcMBEncBwdPicFrame[1]           = mbencVmeInterPredBwdPicIDX10;
    bindingTable->dwAvcMBEncFwdPicFrame[2]           = mbencVmeInterPredFwdPicIDX2;
    bindingTable->dwAvcMBEncFwdPicFrame[3]           = mbencVmeInterPredFwdPicIDX3;
    bindingTable->dwAvcMBEncFwdPicFrame[4]           = mbencVmeInterPredFwdPicIDX4;
    bindingTable->dwAvcMBEncFwdPicFrame[5]           = mbencVmeInterPredFwdPicIDX5;
    bindingTable->dwAvcMBEncFwdPicFrame[6]           = mbencVmeInterPredFwdPicIDX6;
    bindingTable->dwAvcMBEncFwdPicFrame[7]           = mbencVmeInterPredFwdPicIDX7;
    bindingTable->dwAvcMBEncCurrPicFrame[1]          = mbencVmeInterPredCurrPicIdx1;
    bindingTable->dwAvcMBEncBwdPicFrame[2]           = mbencVmeInterPredBwdPicIDX01;
    bindingTable->dwAvcMBEncBwdPicFrame[3]           = mbencVmeInterPredBwdPicIDX11;

    // Field
    bindingTable->dwAvcMBEncMbQpField                = mbencMbqp;
    bindingTable->dwAvcMBEncFieldCurrPic[0]          = mbencVmeInterPredCurrPicIdx0;
    bindingTable->dwAvcMBEncFwdPicTopField[0]        = mbencVmeInterPredFwdPicIDX0;
    bindingTable->dwAvcMBEncBwdPicTopField[0]        = mbencVmeInterPredBwdPicIDX00;
    bindingTable->dwAvcMBEncFwdPicBotField[0]        = mbencVmeInterPredFwdPicIDX0;
    bindingTable->dwAvcMBEncBwdPicBotField[0]        = mbencVmeInterPredBwdPicIDX00;
    bindingTable->dwAvcMBEncFwdPicTopField[1]        = mbencVmeInterPredFwdPicIDX1;
    bindingTable->dwAvcMBEncBwdPicTopField[1]        = mbencVmeInterPredBwdPicIDX10;
    bindingTable->dwAvcMBEncFwdPicBotField[1]        = mbencVmeInterPredFwdPicIDX1;
    bindingTable->dwAvcMBEncBwdPicBotField[1]        = mbencVmeInterPredBwdPicIDX10;
    bindingTable->dwAvcMBEncFwdPicTopField[2]        = mbencVmeInterPredFwdPicIDX2;
    bindingTable->dwAvcMBEncFwdPicBotField[2]        = mbencVmeInterPredFwdPicIDX2;
    bindingTable->dwAvcMBEncFwdPicTopField[3]        = mbencVmeInterPredFwdPicIDX3;
    bindingTable->dwAvcMBEncFwdPicBotField[3]        = mbencVmeInterPredFwdPicIDX3;
    bindingTable->dwAvcMBEncFwdPicTopField[4]        = mbencVmeInterPredFwdPicIDX4;
    bindingTable->dwAvcMBEncFwdPicBotField[4]        = mbencVmeInterPredFwdPicIDX4;
    bindingTable->dwAvcMBEncFwdPicTopField[5]        = mbencVmeInterPredFwdPicIDX5;
    bindingTable->dwAvcMBEncFwdPicBotField[5]        = mbencVmeInterPredFwdPicIDX5;
    bindingTable->dwAvcMBEncFwdPicTopField[6]        = mbencVmeInterPredFwdPicIDX6;
    bindingTable->dwAvcMBEncFwdPicBotField[6]        = mbencVmeInterPredFwdPicIDX6;
    bindingTable->dwAvcMBEncFwdPicTopField[7]        = mbencVmeInterPredFwdPicIDX7;
    bindingTable->dwAvcMBEncFwdPicBotField[7]        = mbencVmeInterPredFwdPicIDX7;
    bindingTable->dwAvcMBEncFieldCurrPic[1]          = mbencVmeInterPredCurrPicIdx1;
    bindingTable->dwAvcMBEncBwdPicTopField[2]        = mbencVmeInterPredBwdPicIDX01;
    bindingTable->dwAvcMBEncBwdPicBotField[2]        = mbencVmeInterPredBwdPicIDX01;
    bindingTable->dwAvcMBEncBwdPicTopField[3]        = mbencVmeInterPredBwdPicIDX11;
    bindingTable->dwAvcMBEncBwdPicBotField[3]        = mbencVmeInterPredBwdPicIDX11;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::InitMbBrcConstantDataBuffer(PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presBrcConstantDataBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::InitMbBrcConstantDataBuffer(params));

    if (params->wPictureCodingType == I_TYPE)
    {
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        uint32_t * data = (uint32_t *)params->pOsInterface->pfnLockResource(
            params->pOsInterface,
            params->presBrcConstantDataBuffer,
            &lockFlags);
        if (data == nullptr)
        {
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }

        // Update MbBrcConstantDataBuffer with high texture cost
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            // Writing to DW13 in each sub-array of 16 DWs
            *(data + 13) = (uint32_t)m_IntraModeCostForHighTextureMB[qp];
            // 16 DWs per QP value
            data += 16;
        }

        params->pOsInterface->pfnUnlockResource(
            params->pOsInterface,
            params->presBrcConstantDataBuffer);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::InitBrcConstantBuffer(PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS        params)
{
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);

    uint8_t tableIdx = params->wPictureCodingType - 1;
    bool blockBasedSkipEn = params->dwMbEncBlockBasedSkipEn ? true : false;
    bool transform_8x8_mode_flag = params->pPicParams->transform_8x8_mode_flag ? true : false;

    if (tableIdx >= 3)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid input parameter.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    MOS_LOCK_PARAMS     LockFlags;
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
    LockFlags.WriteOnly = 1;
    auto data = (uint8_t*)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        &params->sBrcConstantDataBuffer.OsResource,
        &LockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, params->sBrcConstantDataBuffer.dwWidth * params->sBrcConstantDataBuffer.dwHeight);

    // Fill surface with QP Adjustment table, Distortion threshold table, MaxFrame threshold table, Distortion QP Adjustment Table
    eStatus = MOS_SecureMemcpy(
        data,
        sizeof(m_QPAdjustmentDistThresholdMaxFrameThresholdIPB),
        (void*)m_QPAdjustmentDistThresholdMaxFrameThresholdIPB,
        sizeof(m_QPAdjustmentDistThresholdMaxFrameThresholdIPB));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    data += sizeof(m_QPAdjustmentDistThresholdMaxFrameThresholdIPB);

    // Fill surface with Skip Threshold Table
    switch (params->wPictureCodingType)
    {
    case P_TYPE:
        eStatus = MOS_SecureMemcpy(
            data,
            brcConstantsurfaceEarlySkipTableSize,
            (void*)&SkipVal_P_Common[blockBasedSkipEn][transform_8x8_mode_flag][0],
            brcConstantsurfaceEarlySkipTableSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        break;
    case B_TYPE:
        eStatus = MOS_SecureMemcpy(
            data,
            brcConstantsurfaceEarlySkipTableSize,
            (void*)&SkipVal_B_Common[blockBasedSkipEn][transform_8x8_mode_flag][0],
            brcConstantsurfaceEarlySkipTableSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        break;
    default:
        // do nothing for I TYPE
        break;
    }

    if ((params->wPictureCodingType != I_TYPE) && (params->pAvcQCParams != nullptr) && (params->pAvcQCParams->NonFTQSkipThresholdLUTInput))
    {
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            *(data + 1 + (qp * 2)) = (uint8_t)CalcSkipVal((params->dwMbEncBlockBasedSkipEn ? true : false),
                                                                             (params->pPicParams->transform_8x8_mode_flag ? true : false),
                                                                              params->pAvcQCParams->NonFTQSkipThresholdLUT[qp]);
        }
    }

    data += brcConstantsurfaceEarlySkipTableSize;

    // Fill surface with QP list

    // Initialize to -1 (0xff)
    MOS_FillMemory(data, brcConstantsurfaceQpList0, 0xff);
    MOS_FillMemory(data
        + brcConstantsurfaceQpList0
        + brcConstantsurfaceQpList0Reserved,
        brcConstantsurfaceQpList1, 0xff);

    switch (params->wPictureCodingType)
    {
    case B_TYPE:
        data += (brcConstantsurfaceQpList0 + brcConstantsurfaceQpList0Reserved);

        for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
            {
                *(data + refIdx) = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            }
        }
        data -= (brcConstantsurfaceQpList0 + brcConstantsurfaceQpList0Reserved);
        // break statement omitted intentionally
    case P_TYPE:
        for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l0_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
            {
                *(data + refIdx) = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            }
        }
        break;
    default:
        // do nothing for I type
        break;
    }

    data += (brcConstantsurfaceQpList0 + brcConstantsurfaceQpList0Reserved
        + brcConstantsurfaceQpList1 + brcConstantsurfaceQpList1Reserved);

    // Fill surface with Mode cost and MV cost
    eStatus = MOS_SecureMemcpy(
        data,
        brcConstantsurfaceModeMvCostSize,
        (void*)ModeMvCost_Cm[tableIdx],
        brcConstantsurfaceModeMvCostSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // If old mode cost is used the update the table
    if (params->wPictureCodingType == I_TYPE && params->bOldModeCostEnable)
    {
        auto dataTemp = (uint32_t *)data;
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            // Writing to DW0 in each sub-array of 16 DWs
            *dataTemp = (uint32_t)OldIntraModeCost_Cm_Common[qp];
            dataTemp += 16;
        }
    }

    if (params->pAvcQCParams)
    {
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            if (params->pAvcQCParams->FTQSkipThresholdLUTInput)
            {
                *(data + (qp * 32) + 24) =
                    *(data + (qp * 32) + 25) =
                    *(data + (qp * 32) + 27) =
                    *(data + (qp * 32) + 28) =
                    *(data + (qp * 32) + 29) =
                    *(data + (qp * 32) + 30) =
                    *(data + (qp * 32) + 31) = params->pAvcQCParams->FTQSkipThresholdLUT[qp];
            }
        }
    }

    data += brcConstantsurfaceModeMvCostSize;

    // Fill surface with Refcost
    eStatus = MOS_SecureMemcpy(
        data,
        brcConstantsurfaceRefcostSize,
        (void*)&m_refCostMultiRefQp[tableIdx][0],
        brcConstantsurfaceRefcostSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    data += brcConstantsurfaceRefcostSize;

    //Fill surface with Intra cost scaling Factor
    if (params->bAdaptiveIntraScalingEnable)
    {
        eStatus = MOS_SecureMemcpy(
            data,
            brcConstantsurfaceIntracostScalingFactor,
            (void*)&AdaptiveIntraScalingFactor_Cm_Common[0],
            brcConstantsurfaceIntracostScalingFactor);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }
    else
    {
        eStatus = MOS_SecureMemcpy(
            data,
            brcConstantsurfaceIntracostScalingFactor,
            (void*)&IntraScalingFactor_Cm_Common[0],
            brcConstantsurfaceIntracostScalingFactor);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    data += brcConstantsurfaceIntracostScalingFactor;

    eStatus = MOS_SecureMemcpy(
        data,
        brcConstantsurfaceLambdaSize,
        (void*)&m_LambdaData[0],
        brcConstantsurfaceLambdaSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    data += brcConstantsurfaceLambdaSize;

    eStatus = MOS_SecureMemcpy(
        data,
        brcConstantsurfaceFtq25Size,
        (void*)&m_FTQ25[0],
        brcConstantsurfaceFtq25Size);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        &params->sBrcConstantDataBuffer.OsResource);

    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Setup Curbe for AVC MbEnc Kernels
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncodeAvcEncG11::SetCurbeAvcMbEnc(PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSlcParams);

    auto picParams = params->pPicParams;
    auto seqParams = params->pSeqParams;
    auto slcParams = params->pSlcParams;

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.pEncodeAvcSeqParams = seqParams;
    sliceState.pEncodeAvcPicParams = picParams;
    sliceState.pEncodeAvcSliceParams = slcParams;

    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    uint8_t meMethod =
        (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[seqParams->TargetUsage] : m_meMethodGeneric[seqParams->TargetUsage];
    // set sliceQP to MAX_SLICE_QP for  MbEnc kernel, we can use it to verify whether QP is changed or not
    uint8_t sliceQP       = (params->bUseMbEncAdvKernel && params->bBrcEnabled) ? CODECHAL_ENCODE_AVC_MAX_SLICE_QP : picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta;
    bool framePicture = CodecHal_PictureIsFrame(picParams->CurrOriginalPic);
    bool topField     = CodecHal_PictureIsTopField(picParams->CurrOriginalPic);
    bool bottomField  = CodecHal_PictureIsBottomField(picParams->CurrOriginalPic);

    MbencCurbe::MBEncCurbeInitType curbeInitType;
    if (params->bMbEncIFrameDistEnabled)
    {
        curbeInitType = MbencCurbe::IDist;
    }
    else
    {
        switch (m_pictureCodingType)
        {
        case I_TYPE:
            if (framePicture)
                curbeInitType = MbencCurbe::IFrame;
            else
                curbeInitType = MbencCurbe::IField;
            break;

        case P_TYPE:
            if (framePicture)
                curbeInitType = MbencCurbe::PFrame;
            else
                curbeInitType = MbencCurbe::PField;
            break;
        case B_TYPE:
            if (framePicture)
                curbeInitType = MbencCurbe::BFrame;
            else
                curbeInitType = MbencCurbe::BField;
            break;
        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }
    }

    MbencCurbe cmd(curbeInitType);
    // r1
    cmd.m_dw0.AdaptiveEn =
        cmd.m_dw37.AdaptiveEn = EnableAdaptiveSearch[seqParams->TargetUsage];
    cmd.m_dw0.T8x8FlagForInterEn =
        cmd.m_dw37.T8x8FlagForInterEn = picParams->transform_8x8_mode_flag;
    cmd.m_dw2.LenSP                   = MaxLenSP[seqParams->TargetUsage];

    cmd.m_dw1.ExtendedMvCostRange = bExtendedMvCostRange;
    cmd.m_dw36.MBInputEnable = bMbSpecificDataEnabled;
    cmd.m_dw38.LenSP = 0;  // MBZ
    cmd.m_dw3.SrcAccess =
        cmd.m_dw3.RefAccess = framePicture ? 0 : 1;
    if (m_pictureCodingType != I_TYPE && bFTQEnable)
    {
        if (m_pictureCodingType == P_TYPE)
        {
            cmd.m_dw3.FTEnable = FTQBasedSkip[seqParams->TargetUsage] & 0x01;
        }
        else // B_TYPE
        {
            cmd.m_dw3.FTEnable = (FTQBasedSkip[seqParams->TargetUsage] >> 1) & 0x01;
        }
    }
    else
    {
        cmd.m_dw3.FTEnable = 0;
    }
    if (picParams->UserFlags.bDisableSubMBPartition)
    {
        cmd.m_dw3.SubMbPartMask = CODECHAL_ENCODE_AVC_DISABLE_4X4_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_4X8_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_8X4_SUB_MB_PARTITION;
    }
    cmd.m_dw2.PicWidth        = params->wPicWidthInMb;
    cmd.m_dw4.PicHeightMinus1 = params->wFieldFrameHeightInMb - 1;
    cmd.m_dw4.FieldParityFlag = cmd.m_dw7.SrcFieldPolarity = bottomField ? 1 : 0;
    cmd.m_dw4.EnableFBRBypass                              = bFBRBypassEnable;
    cmd.m_dw4.EnableIntraCostScalingForStaticFrame         = params->bStaticFrameDetectionEnabled;
    cmd.m_dw4.bCurFldIDR                                   = framePicture ? 0 : (picParams->bIdrPic || m_firstFieldIdrPic);
    cmd.m_dw4.ConstrainedIntraPredFlag                     = picParams->constrained_intra_pred_flag;
    cmd.m_dw4.HMEEnable                                    = m_hmeEnabled;
    cmd.m_dw4.PictureType                                  = m_pictureCodingType - 1;
    cmd.m_dw4.UseActualRefQPValue                          = m_hmeEnabled ? (m_MRDisableQPCheck[seqParams->TargetUsage] == 0) : false;
    cmd.m_dw5.SliceMbHeight                                = params->usSliceHeight;
    cmd.m_dw7.IntraPartMask                                = picParams->transform_8x8_mode_flag ? 0 : 0x2;  // Disable 8x8 if flag is not set

    // r2
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.m_dw6.BatchBufferEnd = 0;
    }
    else
    {
        uint8_t ucTableIdx = m_pictureCodingType - 1;
        eStatus            = MOS_SecureMemcpy(&(cmd.m_modeMvCost), 8 * sizeof(uint32_t), ModeMvCost_Cm[ucTableIdx][sliceQP], 8 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        if (m_pictureCodingType == I_TYPE && bOldModeCostEnable)
        {
            // Old intra mode cost needs to be used if bOldModeCostEnable is 1
            cmd.m_modeMvCost.DW8.Value = OldIntraModeCost_Cm_Common[sliceQP];
        }
        else if (m_skipBiasAdjustmentEnable)
        {
            // Load different MvCost for P picture when SkipBiasAdjustment is enabled
            // No need to check for P picture as the flag is only enabled for P picture
            cmd.m_modeMvCost.DW11.Value = MvCost_PSkipAdjustment_Cm_Common[sliceQP];
        }
    }

    uint8_t tableIdx;
    // r3 & r4
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.m_spDelta.DW31.IntraComputeType = 1;
    }
    else
    {
        tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
        eStatus  = MOS_SecureMemcpy(&(cmd.m_spDelta), 16 * sizeof(uint32_t), m_encodeSearchPath[tableIdx][meMethod], 16 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    // r5
    if (m_pictureCodingType == P_TYPE)
    {
        cmd.m_dw32.SkipVal = SkipVal_P_Common
            [cmd.m_dw3.BlockBasedSkipEnable]
            [picParams->transform_8x8_mode_flag]
            [sliceQP];
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        cmd.m_dw32.SkipVal = SkipVal_B_Common
            [cmd.m_dw3.BlockBasedSkipEnable]
            [picParams->transform_8x8_mode_flag]
            [sliceQP];
    }

    cmd.m_modeMvCost.DW13.QpPrimeY = sliceQP;
    // QpPrimeCb and QpPrimeCr are not used by Kernel. Following settings are for CModel matching.
    cmd.m_modeMvCost.DW13.QpPrimeCb        = sliceQP;
    cmd.m_modeMvCost.DW13.QpPrimeCr        = sliceQP;
    cmd.m_modeMvCost.DW13.TargetSizeInWord = 0xff;  // hardcoded for BRC disabled

    if (bMultiPredEnable && (m_pictureCodingType != I_TYPE))
    {
        // Based on "MultiPred to Kernel Mapping" in kernel
        switch (m_multiPred[seqParams->TargetUsage])
        {
        case 0: // Disable multipred for both P & B picture types
            cmd.m_dw32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.m_dw32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 1: // Enable multipred for P pictures only
            cmd.m_dw32.MultiPredL0Disable = (m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.m_dw32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 2: // Enable multipred for B pictures only
            cmd.m_dw32.MultiPredL0Disable = (m_pictureCodingType == B_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.m_dw32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 3: // Enable multipred for both P & B picture types
            cmd.m_dw32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE;
            cmd.m_dw32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;
        }
    }
    else
    {
        cmd.m_dw32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        cmd.m_dw32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
    }

    if (!framePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.m_dw34.List0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
            cmd.m_dw34.List0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
            cmd.m_dw34.List0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
            cmd.m_dw34.List0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
            cmd.m_dw34.List0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
            cmd.m_dw34.List0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
            cmd.m_dw34.List0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
            cmd.m_dw34.List0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
            cmd.m_dw34.List1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
            cmd.m_dw34.List1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    if (m_adaptiveTransformDecisionEnabled)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.m_dw34.EnableAdaptiveTxDecision = true;
        }
        cmd.m_dw60.TxDecisonThreshold = adaptiveTxDecisionThreshold;
    }

    if (m_adaptiveTransformDecisionEnabled || m_flatnessCheckEnabled)
    {
        cmd.m_dw60.MBTextureThreshold = mbTextureThreshold;
    }

    if (m_pictureCodingType == B_TYPE)
    {
        cmd.m_dw34.List1RefID0FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.m_dw34.List1RefID1FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        cmd.m_dw34.bDirectMode               = slcParams->direct_spatial_mv_pred_flag;
    }

    cmd.m_dw34.EnablePerMBStaticCheck         = params->bStaticFrameDetectionEnabled;
    cmd.m_dw34.EnableAdaptiveSearchWindowSize = params->bApdatvieSearchWindowSizeEnabled;
    cmd.m_dw34.RemoveIntraRefreshOverlap      = picParams->bDisableRollingIntraRefreshOverlap;
    cmd.m_dw34.bOriginalBff                    = framePicture ? 0 : ((m_firstField && (bottomField)) || (!m_firstField && (!bottomField)));
    cmd.m_dw34.EnableMBFlatnessChkOptimization = m_flatnessCheckEnabled;
    cmd.m_dw34.ROIEnableFlag                   = params->bRoiEnabled;
    cmd.m_dw34.MADEnableFlag                   = m_madEnabled;
    cmd.m_dw34.MBBrcEnable                     = bMbBrcEnabled || bMbQpDataEnabled;
    cmd.m_dw34.ArbitraryNumMbsPerSlice         = m_arbitraryNumMbsInSlice;
    cmd.m_dw34.TQEnable                        = m_trellisQuantParams.dwTqEnabled;  //Enabled for KBL
    cmd.m_dw34.ForceNonSkipMbEnable            = params->bMbDisableSkipMapEnabled;
    if (params->pAvcQCParams && !cmd.m_dw34.ForceNonSkipMbEnable)  // ignore DisableEncSkipCheck if Mb Disable Skip Map is available
    {
        cmd.m_dw34.DisableEncSkipCheck = params->pAvcQCParams->skipCheckDisable;
        }
        cmd.m_dw34.CQPFlag                    = !bBrcEnabled;  // 1 - Rate Control is CQP, 0 - Rate Control is BRC
        cmd.m_dw36.CheckAllFractionalEnable   = bCAFEnable;
        cmd.m_dw38.RefThreshold               = refThreshold;
        cmd.m_dw39.HMERefWindowsCombThreshold = (m_pictureCodingType == B_TYPE) ? HMEBCombineLen[seqParams->TargetUsage] : HMECombineLen[seqParams->TargetUsage];

        // Default:2 used for MBBRC (MB QP Surface width and height are 4x downscaled picture in MB unit * 4  bytes)
        // 0 used for MBQP data surface (MB QP Surface width and height are same as the input picture size in MB unit * 1bytes)
        // BRC use split kernel, MB QP surface is same size as input picture
        cmd.m_dw47.MbQpReadFactor = (bMbBrcEnabled || bMbQpDataEnabled) ? 0 : 2;

        // Those fields are not really used for I_dist kernel,
        // but set them to 0 to get bit-exact match with kernel
        if (params->bMbEncIFrameDistEnabled)
        {
            cmd.m_modeMvCost.DW13.QpPrimeY        = 0;
            cmd.m_modeMvCost.DW13.QpPrimeCb       = 0;
            cmd.m_modeMvCost.DW13.QpPrimeCr       = 0;
            cmd.m_dw33.Intra16x16NonDCPredPenalty = 0;
            cmd.m_dw33.Intra4x4NonDCPredPenalty   = 0;
            cmd.m_dw33.Intra8x8NonDCPredPenalty   = 0;
    }

    //r6
    if (cmd.m_dw4.UseActualRefQPValue)
    {
        cmd.m_dw44.ActualQPValueForRefID0List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        cmd.m_dw44.ActualQPValueForRefID1List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        cmd.m_dw44.ActualQPValueForRefID2List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        cmd.m_dw44.ActualQPValueForRefID3List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        cmd.m_dw45.ActualQPValueForRefID4List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        cmd.m_dw45.ActualQPValueForRefID5List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        cmd.m_dw45.ActualQPValueForRefID6List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        cmd.m_dw45.ActualQPValueForRefID7List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        cmd.m_dw46.ActualQPValueForRefID0List1 = AVCGetQPValueFromRefList(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.m_dw46.ActualQPValueForRefID1List1 = AVCGetQPValueFromRefList(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
    }

    tableIdx = m_pictureCodingType - 1;
    cmd.m_dw46.RefCost = m_refCostMultiRefQp[tableIdx][sliceQP];

    // Picture Coding Type dependent parameters
    if (m_pictureCodingType == I_TYPE)
    {
        cmd.m_dw0.SkipModeEn                  = 0;
        cmd.m_dw37.SkipModeEn                 = 0;
        cmd.m_dw36.HMECombineOverlap          = 0;
        cmd.m_dw47.IntraCostSF                = 16;  // This is not used but recommended to set this to 16 by Kernel team
        cmd.m_dw34.EnableDirectBiasAdjustment = 0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        cmd.m_dw1.MaxNumMVs        = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.m_dw3.BMEDisableFBR    = 1;
        cmd.m_dw5.RefWidth         = SearchX[seqParams->TargetUsage];
        cmd.m_dw5.RefHeight        = SearchY[seqParams->TargetUsage];
        cmd.m_dw7.NonSkipZMvAdded  = 1;
        cmd.m_dw7.NonSkipModeAdded = 1;
        cmd.m_dw7.SkipCenterMask   = 1;
        cmd.m_dw47.IntraCostSF =
            bAdaptiveIntraScalingEnable ? AdaptiveIntraScalingFactor_Cm_Common[sliceQP] : IntraScalingFactor_Cm_Common[sliceQP];
        cmd.m_dw47.MaxVmvR                    = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.m_dw36.HMECombineOverlap          = 1;
        cmd.m_dw36.NumRefIdxL0MinusOne        = bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.m_dw39.RefWidth                   = SearchX[seqParams->TargetUsage];
        cmd.m_dw39.RefHeight                  = SearchY[seqParams->TargetUsage];
        cmd.m_dw34.EnableDirectBiasAdjustment = 0;
        if (params->pAvcQCParams)
        {
            cmd.m_dw34.EnableGlobalMotionBiasAdjustment = params->pAvcQCParams->globalMotionBiasAdjustmentEnable;
        }
    }
    else
    {
        // B_TYPE
        cmd.m_dw1.MaxNumMVs      = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.m_dw1.BiWeight       = m_biWeight;
        cmd.m_dw3.SearchCtrl     = 7;
        cmd.m_dw3.SkipType       = 1;
        cmd.m_dw5.RefWidth       = BSearchX[seqParams->TargetUsage];
        cmd.m_dw5.RefHeight      = BSearchY[seqParams->TargetUsage];
        cmd.m_dw7.SkipCenterMask = 0xFF;
        cmd.m_dw47.IntraCostSF =
            bAdaptiveIntraScalingEnable ? AdaptiveIntraScalingFactor_Cm_Common[sliceQP] : IntraScalingFactor_Cm_Common[sliceQP];
        cmd.m_dw47.MaxVmvR           = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.m_dw36.HMECombineOverlap = 1;
        // Checking if the forward frame (List 1 index 0) is a short term reference
        {
            auto codecHalPic = params->pSlcParams->RefPicList[LIST_1][0];
            if (codecHalPic.PicFlags != PICTURE_INVALID &&
                codecHalPic.FrameIdx != CODECHAL_ENCODE_AVC_INVALID_PIC_ID &&
                params->pPicIdx[codecHalPic.FrameIdx].bValid)
            {
                // Although its name is FWD, it actually means the future frame or the backward reference frame
                cmd.m_dw36.IsFwdFrameShortTermRef = CodecHal_PictureIsShortTermRef(params->pPicParams->RefFrameList[codecHalPic.FrameIdx]);
            }
            else
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Invalid backward reference frame.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }
        cmd.m_dw36.NumRefIdxL0MinusOne        = bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.m_dw36.NumRefIdxL1MinusOne        = bMultiPredEnable ? slcParams->num_ref_idx_l1_active_minus1 : 0;
        cmd.m_dw39.RefWidth                   = BSearchX[seqParams->TargetUsage];
        cmd.m_dw39.RefHeight                  = BSearchY[seqParams->TargetUsage];
        cmd.m_dw40.DistScaleFactorRefID0List0 = m_distScaleFactorList0[0];
        cmd.m_dw40.DistScaleFactorRefID1List0 = m_distScaleFactorList0[1];
        cmd.m_dw41.DistScaleFactorRefID2List0 = m_distScaleFactorList0[2];
        cmd.m_dw41.DistScaleFactorRefID3List0 = m_distScaleFactorList0[3];
        cmd.m_dw42.DistScaleFactorRefID4List0 = m_distScaleFactorList0[4];
        cmd.m_dw42.DistScaleFactorRefID5List0 = m_distScaleFactorList0[5];
        cmd.m_dw43.DistScaleFactorRefID6List0 = m_distScaleFactorList0[6];
        cmd.m_dw43.DistScaleFactorRefID7List0 = m_distScaleFactorList0[7];
        if (params->pAvcQCParams)
        {
            cmd.m_dw34.EnableDirectBiasAdjustment = params->pAvcQCParams->directBiasAdjustmentEnable;
            if (cmd.m_dw34.EnableDirectBiasAdjustment)
            {
                cmd.m_dw7.NonSkipModeAdded = 1;
                cmd.m_dw7.NonSkipZMvAdded  = 1;
            }

            cmd.m_dw34.EnableGlobalMotionBiasAdjustment = params->pAvcQCParams->globalMotionBiasAdjustmentEnable;
        }
    }

    *params->pdwBlockBasedSkipEn = cmd.m_dw3.BlockBasedSkipEnable;

    if (picParams->EnableRollingIntraRefresh)
    {
        cmd.m_dw34.IntraRefreshEn = picParams->EnableRollingIntraRefresh;

        /* Multiple predictor should be completely disabled for the RollingI feature. This does not lead to much quality drop for P frames especially for TU as 1 */
        cmd.m_dw32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;

        /* Pass the same IntraRefreshUnit to the kernel w/o the adjustment by -1, so as to have an overlap of one MB row or column of Intra macroblocks
        across one P frame to another P frame, as needed by the RollingI algo */
        if (ROLLING_I_SQUARE == picParams->EnableRollingIntraRefresh && RATECONTROL_CQP != seqParams->RateControlMethod)
        {
            /*BRC update kernel updates these CURBE to MBEnc*/
            cmd.m_dw4.EnableIntraRefresh = false;
            cmd.m_dw34.IntraRefreshEn    = ROLLING_I_DISABLED;
            cmd.m_dw48.IntraRefreshMBx   = 0; /* MB column number */
            cmd.m_dw61.IntraRefreshMBy   = 0; /* MB row number */
        }
        else
        {
            cmd.m_dw4.EnableIntraRefresh = true;
            cmd.m_dw34.IntraRefreshEn    = picParams->EnableRollingIntraRefresh;
            cmd.m_dw48.IntraRefreshMBx   = picParams->IntraRefreshMBx; /* MB column number */
            cmd.m_dw61.IntraRefreshMBy   = picParams->IntraRefreshMBy; /* MB row number */
        }
        cmd.m_dw48.IntraRefreshUnitInMBMinus1 = picParams->IntraRefreshUnitinMB;
        cmd.m_dw48.IntraRefreshQPDelta        = picParams->IntraRefreshQPDelta;
    }
    else
    {
        cmd.m_dw34.IntraRefreshEn = 0;
    }

    if (params->bRoiEnabled)
    {
        cmd.m_dw49.ROI1_X_left   = picParams->ROI[0].Left;
        cmd.m_dw49.ROI1_Y_top    = picParams->ROI[0].Top;
        cmd.m_dw50.ROI1_X_right  = picParams->ROI[0].Right;
        cmd.m_dw50.ROI1_Y_bottom = picParams->ROI[0].Bottom;

        cmd.m_dw51.ROI2_X_left   = picParams->ROI[1].Left;
        cmd.m_dw51.ROI2_Y_top    = picParams->ROI[1].Top;
        cmd.m_dw52.ROI2_X_right  = picParams->ROI[1].Right;
        cmd.m_dw52.ROI2_Y_bottom = picParams->ROI[1].Bottom;

        cmd.m_dw53.ROI3_X_left   = picParams->ROI[2].Left;
        cmd.m_dw53.ROI3_Y_top    = picParams->ROI[2].Top;
        cmd.m_dw54.ROI3_X_right  = picParams->ROI[2].Right;
        cmd.m_dw54.ROI3_Y_bottom = picParams->ROI[2].Bottom;

        cmd.m_dw55.ROI4_X_left   = picParams->ROI[3].Left;
        cmd.m_dw55.ROI4_Y_top    = picParams->ROI[3].Top;
        cmd.m_dw56.ROI4_X_right  = picParams->ROI[3].Right;
        cmd.m_dw56.ROI4_Y_bottom = picParams->ROI[3].Bottom;

        if (bBrcEnabled == false)
        {
            uint16_t numROI = picParams->NumROI;
            char priorityLevelOrDQp[CODECHAL_ENCODE_AVC_MAX_ROI_NUMBER] = { 0 };

            // cqp case
            for (unsigned int i = 0; i < numROI; i += 1)
            {
                char dQpRoi = picParams->ROI[i].PriorityLevelOrDQp;

                // clip qp roi in order to have (qp + qpY) in range [0, 51]
                priorityLevelOrDQp[i] = (int8_t)CodecHal_Clip3(-sliceQP, CODECHAL_ENCODE_AVC_MAX_SLICE_QP - sliceQP, dQpRoi);
            }

            cmd.m_dw57.ROI1_dQpPrimeY = priorityLevelOrDQp[0];
            cmd.m_dw57.ROI2_dQpPrimeY = priorityLevelOrDQp[1];
            cmd.m_dw57.ROI3_dQpPrimeY = priorityLevelOrDQp[2];
            cmd.m_dw57.ROI4_dQpPrimeY = priorityLevelOrDQp[3];
        }
        else
        {
            // kernel does not support BRC case
            cmd.m_dw34.ROIEnableFlag = 0;
        }
    }
    else if (params->bDirtyRoiEnabled)
    {
        // enable Dirty Rect flag
        cmd.m_dw4.EnableDirtyRect = true;

        cmd.m_dw49.ROI1_X_left   = params->pPicParams->DirtyROI[0].Left;
        cmd.m_dw49.ROI1_Y_top    = params->pPicParams->DirtyROI[0].Top;
        cmd.m_dw50.ROI1_X_right  = params->pPicParams->DirtyROI[0].Right;
        cmd.m_dw50.ROI1_Y_bottom = params->pPicParams->DirtyROI[0].Bottom;

        cmd.m_dw51.ROI2_X_left   = params->pPicParams->DirtyROI[1].Left;
        cmd.m_dw51.ROI2_Y_top    = params->pPicParams->DirtyROI[1].Top;
        cmd.m_dw52.ROI2_X_right  = params->pPicParams->DirtyROI[1].Right;
        cmd.m_dw52.ROI2_Y_bottom = params->pPicParams->DirtyROI[1].Bottom;

        cmd.m_dw53.ROI3_X_left   = params->pPicParams->DirtyROI[2].Left;
        cmd.m_dw53.ROI3_Y_top    = params->pPicParams->DirtyROI[2].Top;
        cmd.m_dw54.ROI3_X_right  = params->pPicParams->DirtyROI[2].Right;
        cmd.m_dw54.ROI3_Y_bottom = params->pPicParams->DirtyROI[2].Bottom;

        cmd.m_dw55.ROI4_X_left   = params->pPicParams->DirtyROI[3].Left;
        cmd.m_dw55.ROI4_Y_top    = params->pPicParams->DirtyROI[3].Top;
        cmd.m_dw56.ROI4_X_right  = params->pPicParams->DirtyROI[3].Right;
        cmd.m_dw56.ROI4_Y_bottom = params->pPicParams->DirtyROI[3].Bottom;
    }

    if (m_trellisQuantParams.dwTqEnabled)
    {
        // Lambda values for TQ
        if (m_pictureCodingType == I_TYPE)
        {
            cmd.m_dw58.Value = TQ_LAMBDA_I_FRAME[sliceQP][0];
            cmd.m_dw59.Value = TQ_LAMBDA_I_FRAME[sliceQP][1];
        }
        else if (m_pictureCodingType == P_TYPE)
        {
            cmd.m_dw58.Value = TQ_LAMBDA_P_FRAME[sliceQP][0];
            cmd.m_dw59.Value = TQ_LAMBDA_P_FRAME[sliceQP][1];
        }
        else
        {
            cmd.m_dw58.Value = TQ_LAMBDA_B_FRAME[sliceQP][0];
            cmd.m_dw59.Value = TQ_LAMBDA_B_FRAME[sliceQP][1];
        }

        // check if Lambda is greater than max value
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));

        if (cmd.m_dw58.Lambda_8x8Inter > maxLambda)
        {
            cmd.m_dw58.Lambda_8x8Inter = 0xf000 + sliceState.dwRoundingValue;
        }

        if (cmd.m_dw58.Lambda_8x8Intra > maxLambda)
        {
            cmd.m_dw58.Lambda_8x8Intra = 0xf000 + defaultTrellisQuantIntraRounding;
        }

        // check if Lambda is greater than max value
        if (cmd.m_dw59.Lambda_Inter > maxLambda)
        {
            cmd.m_dw59.Lambda_Inter = 0xf000 + sliceState.dwRoundingValue;
        }

        if (cmd.m_dw59.Lambda_Intra > maxLambda)
        {
            cmd.m_dw59.Lambda_Intra = 0xf000 + defaultTrellisQuantIntraRounding;
        }
    }

    //IPCM QP and threshold
    cmd.m_dw62.IPCM_QP0 = m_IPCMThresholdTable[0].QP;
    cmd.m_dw62.IPCM_QP1 = m_IPCMThresholdTable[1].QP;
    cmd.m_dw62.IPCM_QP2 = m_IPCMThresholdTable[2].QP;
    cmd.m_dw62.IPCM_QP3 = m_IPCMThresholdTable[3].QP;
    cmd.m_dw63.IPCM_QP4 = m_IPCMThresholdTable[4].QP;

    cmd.m_dw63.IPCM_Thresh0 = m_IPCMThresholdTable[0].Threshold;
    cmd.m_dw64.IPCM_Thresh1 = m_IPCMThresholdTable[1].Threshold;
    cmd.m_dw64.IPCM_Thresh2 = m_IPCMThresholdTable[2].Threshold;
    cmd.m_dw65.IPCM_Thresh3 = m_IPCMThresholdTable[3].Threshold;
    cmd.m_dw65.IPCM_Thresh4 = m_IPCMThresholdTable[4].Threshold;

    cmd.m_dw66.MBDataSurfIndex               = mbencMfcAvcPakObj;
    cmd.m_dw67.MVDataSurfIndex               = mbencIndMvData;
    cmd.m_dw68.IDistSurfIndex                = mbencBrcDistortion;
    cmd.m_dw69.SrcYSurfIndex                 = mbencCurrY;
    cmd.m_dw70.MBSpecificDataSurfIndex       = mbencMbSpecificData;
    cmd.m_dw71.AuxVmeOutSurfIndex            = mbencAuxVmeOut;
    cmd.m_dw72.CurrRefPicSelSurfIndex        = mbencRefpicselectL0;
    cmd.m_dw73.HMEMVPredFwdBwdSurfIndex      = mbencMvDataFromMe;
    cmd.m_dw74.HMEDistSurfIndex              = mbenc4xMeDistortion;
    cmd.m_dw75.SliceMapSurfIndex             = mbencSlicemapData;
    cmd.m_dw76.FwdFrmMBDataSurfIndex         = mbencFwdMbData;
    cmd.m_dw77.FwdFrmMVSurfIndex             = mbencFwdMvData;
    cmd.m_dw78.MBQPBuffer                    = mbencMbqp;
    cmd.m_dw79.MBBRCLut                      = mbencMbbrcConstData;
    cmd.m_dw80.VMEInterPredictionSurfIndex   = mbencVmeInterPredCurrPicIdx0;
    cmd.m_dw81.VMEInterPredictionMRSurfIndex = mbencVmeInterPredCurrPicIdx1;
    cmd.m_dw82.MbStatsSurfIndex              = mbencMbStats;
    cmd.m_dw83.MADSurfIndex                  = mbencMadData;
    cmd.m_dw84.BRCCurbeSurfIndex             = mbencBrcCurbeData;
    cmd.m_dw85.ForceNonSkipMBmapSurface      = mbencForceNonskipMbMap;
    cmd.m_dw86.ReservedIndex                 = mbEncAdv;
    cmd.m_dw87.StaticDetectionCostTableIndex = mbencSfdCostTable;
    cmd.m_dw88.SWScoreboardIndex             = mbencSwScoreboard;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateEncParam(
            meMethod,
            &cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::SetCurbeAvcBrcInitReset(PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto picParams = m_avcPicParam;
    auto seqParams = m_avcSeqParam;
    auto vuiParams = m_avcVuiParams;
    uint32_t profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        (uint32_t*)&profileLevelMaxFrame));

    BrcInitResetCurbeG11 cmd;
    cmd.m_dw0.ProfileLevelMaxFrame = profileLevelMaxFrame;
    cmd.m_dw1.InitBufFullInBits    = seqParams->InitVBVBufferFullnessInBit;
    cmd.m_dw2.BufSizeInBits        = seqParams->VBVBufferSizeInBit;
    cmd.m_dw3.AverageBitRate       = seqParams->TargetBitRate;
    cmd.m_dw4.MaxBitRate           = seqParams->MaxBitRate;
    cmd.m_dw8.GopP =
        (seqParams->GopRefDist) ? ((seqParams->GopPicSize - 1) / seqParams->GopRefDist) : 0;
    cmd.m_dw9.GopB                = seqParams->GopPicSize - 1 - cmd.m_dw8.GopP;
    cmd.m_dw9.FrameWidthInBytes   = m_frameWidth;
    cmd.m_dw10.FrameHeightInBytes = m_frameHeight;
    cmd.m_dw12.NoSlices           = m_numSlices;

    cmd.m_dw32.SurfaceIndexhistorybuffer    = CODECHAL_ENCODE_AVC_BRC_INIT_RESET_HISTORY;
    cmd.m_dw33.SurfaceIndexdistortionbuffer = CODECHAL_ENCODE_AVC_BRC_INIT_RESET_DISTORTION;

    // if VUI present, VUI data has high priority
    if (seqParams->vui_parameters_present_flag && seqParams->RateControlMethod != RATECONTROL_AVBR)
    {
        cmd.m_dw4.MaxBitRate =
            ((vuiParams->bit_rate_value_minus1[0] + 1) << (6 + vuiParams->bit_rate_scale));

        if (seqParams->RateControlMethod == RATECONTROL_CBR)
        {
            cmd.m_dw3.AverageBitRate = cmd.m_dw4.MaxBitRate;
        }
    }

    cmd.m_dw6.FrameRateM = seqParams->FramesPer100Sec;
    cmd.m_dw7.FrameRateD = 100;
    cmd.m_dw8.BRCFlag    = (CodecHal_PictureIsFrame(m_currOriginalPic)) ? 0 : CODECHAL_ENCODE_BRCINIT_FIELD_PIC;
    // MBBRC should be skipped when BRC ROI is on
    cmd.m_dw8.BRCFlag |= (bMbBrcEnabled && !bBrcRoiEnabled) ? 0 : CODECHAL_ENCODE_BRCINIT_DISABLE_MBBRC;

    if (seqParams->RateControlMethod == RATECONTROL_CBR)
    {
        cmd.m_dw4.MaxBitRate = cmd.m_dw3.AverageBitRate;
        cmd.m_dw8.BRCFlag    = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISCBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (cmd.m_dw4.MaxBitRate < cmd.m_dw3.AverageBitRate)
        {
            cmd.m_dw3.AverageBitRate = cmd.m_dw4.MaxBitRate;  // Use max bit rate for HRD compliance
        }
        cmd.m_dw8.BRCFlag = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        cmd.m_dw8.BRCFlag = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        cmd.m_dw4.MaxBitRate = cmd.m_dw3.AverageBitRate;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        cmd.m_dw8.BRCFlag = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISICQ;
        cmd.m_dw23.ACQP   = seqParams->ICQQualityFactor;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VCM)
    {
        cmd.m_dw8.BRCFlag = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVCM;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (cmd.m_dw4.MaxBitRate < cmd.m_dw3.AverageBitRate)
        {
            cmd.m_dw3.AverageBitRate = cmd.m_dw4.MaxBitRate;  // Use max bit rate for HRD compliance
        }
        cmd.m_dw8.BRCFlag = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISQVBR;
        // use ICQQualityFactor to determine the larger Qp for each MB
        cmd.m_dw23.ACQP = seqParams->ICQQualityFactor;
    }

    cmd.m_dw10.AVBRAccuracy    = usAVBRAccuracy;
    cmd.m_dw11.AVBRConvergence = usAVBRConvergence;

    // Set dynamic thresholds
    double inputBitsPerFrame =
        ((double)(cmd.m_dw4.MaxBitRate) * (double)(cmd.m_dw7.FrameRateD) /
            (double)(cmd.m_dw6.FrameRateM));
    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        inputBitsPerFrame *= 0.5;
    }

    if (cmd.m_dw2.BufSizeInBits == 0)
    {
        cmd.m_dw2.BufSizeInBits = (uint32_t)inputBitsPerFrame * 4;
    }

    if (cmd.m_dw1.InitBufFullInBits == 0)
    {
        cmd.m_dw1.InitBufFullInBits = 7 * cmd.m_dw2.BufSizeInBits / 8;
    }
    if (cmd.m_dw1.InitBufFullInBits < (uint32_t)(inputBitsPerFrame * 2))
    {
        cmd.m_dw1.InitBufFullInBits = (uint32_t)(inputBitsPerFrame * 2);
    }
    if (cmd.m_dw1.InitBufFullInBits > cmd.m_dw2.BufSizeInBits)
    {
        cmd.m_dw1.InitBufFullInBits = cmd.m_dw2.BufSizeInBits;
    }

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * BufferSize
        cmd.m_dw2.BufSizeInBits     = 2 * seqParams->TargetBitRate;
        cmd.m_dw1.InitBufFullInBits = (uint32_t)(0.75 * cmd.m_dw2.BufSizeInBits);
    }

    double bpsRatio = inputBitsPerFrame / ((double)(cmd.m_dw2.BufSizeInBits) / 30);
    bpsRatio = (bpsRatio < 0.1) ? 0.1 : (bpsRatio > 3.5) ? 3.5 : bpsRatio;

    cmd.m_dw16.DeviationThreshold0ForPandB = (uint32_t)(-50 * pow(0.90, bpsRatio));
    cmd.m_dw16.DeviationThreshold1ForPandB = (uint32_t)(-50 * pow(0.66, bpsRatio));
    cmd.m_dw16.DeviationThreshold2ForPandB = (uint32_t)(-50 * pow(0.46, bpsRatio));
    cmd.m_dw16.DeviationThreshold3ForPandB = (uint32_t)(-50 * pow(0.3, bpsRatio));
    cmd.m_dw17.DeviationThreshold4ForPandB = (uint32_t)(50 * pow(0.3, bpsRatio));
    cmd.m_dw17.DeviationThreshold5ForPandB = (uint32_t)(50 * pow(0.46, bpsRatio));
    cmd.m_dw17.DeviationThreshold6ForPandB = (uint32_t)(50 * pow(0.7, bpsRatio));
    cmd.m_dw17.DeviationThreshold7ForPandB = (uint32_t)(50 * pow(0.9, bpsRatio));
    cmd.m_dw18.DeviationThreshold0ForVBR   = (uint32_t)(-50 * pow(0.9, bpsRatio));
    cmd.m_dw18.DeviationThreshold1ForVBR   = (uint32_t)(-50 * pow(0.7, bpsRatio));
    cmd.m_dw18.DeviationThreshold2ForVBR   = (uint32_t)(-50 * pow(0.5, bpsRatio));
    cmd.m_dw18.DeviationThreshold3ForVBR   = (uint32_t)(-50 * pow(0.3, bpsRatio));
    cmd.m_dw19.DeviationThreshold4ForVBR   = (uint32_t)(100 * pow(0.4, bpsRatio));
    cmd.m_dw19.DeviationThreshold5ForVBR   = (uint32_t)(100 * pow(0.5, bpsRatio));
    cmd.m_dw19.DeviationThreshold6ForVBR   = (uint32_t)(100 * pow(0.75, bpsRatio));
    cmd.m_dw19.DeviationThreshold7ForVBR   = (uint32_t)(100 * pow(0.9, bpsRatio));
    cmd.m_dw20.DeviationThreshold0ForI     = (uint32_t)(-50 * pow(0.8, bpsRatio));
    cmd.m_dw20.DeviationThreshold1ForI     = (uint32_t)(-50 * pow(0.6, bpsRatio));
    cmd.m_dw20.DeviationThreshold2ForI     = (uint32_t)(-50 * pow(0.34, bpsRatio));
    cmd.m_dw20.DeviationThreshold3ForI     = (uint32_t)(-50 * pow(0.2, bpsRatio));
    cmd.m_dw21.DeviationThreshold4ForI     = (uint32_t)(50 * pow(0.2, bpsRatio));
    cmd.m_dw21.DeviationThreshold5ForI     = (uint32_t)(50 * pow(0.4, bpsRatio));
    cmd.m_dw21.DeviationThreshold6ForI     = (uint32_t)(50 * pow(0.66, bpsRatio));
    cmd.m_dw21.DeviationThreshold7ForI     = (uint32_t)(50 * pow(0.9, bpsRatio));

    cmd.m_dw22.SlidingWindowSize = dwSlidingWindowSize;

    if (bBrcInit)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits = cmd.m_dw1.InitBufFullInBits;
    }

    *params->pdwBrcInitResetBufSizeInBits    = cmd.m_dw2.BufSizeInBits;
    *params->pdBrcInitResetInputBitsPerFrame = inputBitsPerFrame;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcInitParam(
            &cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::SetCurbeAvcFrameBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    auto seqParams = m_avcSeqParam;
    auto picParams = m_avcPicParam;
    auto slcParams = m_avcSliceParams;

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.pEncodeAvcSeqParams = seqParams;
    sliceState.pEncodeAvcPicParams = picParams;
    sliceState.pEncodeAvcSliceParams = slcParams;

    FrameBrcUpdateCurbe cmd;
    cmd.m_dw5.TargetSizeFlag = 0;
    if (*params->pdBrcInitCurrentTargetBufFullInBits > (double)dwBrcInitResetBufSizeInBits)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits -= (double)dwBrcInitResetBufSizeInBits;
        cmd.m_dw5.TargetSizeFlag = 1;
    }

    // skipped frame handling
    if (params->dwNumSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        cmd.m_dw6.NumSkipFrames  = params->dwNumSkipFrames;
        cmd.m_dw7.SizeSkipFrames = params->dwSizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame * params->dwNumSkipFrames;
    }

    cmd.m_dw0.TargetSize       = (uint32_t)(*params->pdBrcInitCurrentTargetBufFullInBits);
    cmd.m_dw1.FrameNumber      = m_storeData - 1;
    cmd.m_dw2.SizeofPicHeaders = m_headerBytesInserted << 3;  // kernel uses how many bits instead of bytes
    cmd.m_dw5.CurrFrameType =
        ((m_pictureCodingType - 2) < 0) ? 2 : (m_pictureCodingType - 2);
    cmd.m_dw5.BRCFlag =
        (CodecHal_PictureIsTopField(m_currOriginalPic)) ? brcUpdateIsField : ((CodecHal_PictureIsBottomField(m_currOriginalPic)) ? (brcUpdateIsField | brcUpdateIsBottomField) : 0);
    cmd.m_dw5.BRCFlag |= (m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) ? brcUpdateIsReference : 0;

    if (bMultiRefQpEnabled)
    {
        cmd.m_dw5.BRCFlag |= brcUpdateIsActualQp;
        cmd.m_dw14.QPIndexOfCurPic = m_currOriginalPic.FrameIdx;
    }

    cmd.m_dw5.BRCFlag |= seqParams->bAutoMaxPBFrameSizeForSceneChange ? brcUpdateAutoPbFrameSize : 0;

    cmd.m_dw5.MaxNumPAKs = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();

    cmd.m_dw6.MinimumQP            = params->ucMinQP;
    cmd.m_dw6.MaximumQP            = params->ucMaxQP;
    cmd.m_dw6.EnableForceToSkip    = (bForceToSkipEnable && !m_avcPicParam->bDisableFrameSkip);
    cmd.m_dw6.EnableSlidingWindow  = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);
    cmd.m_dw6.EnableExtremLowDelay = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW);
    cmd.m_dw6.DisableVarCompute    = bBRCVarCompuBypass;

    *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame;

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        cmd.m_dw3.startGAdjFrame0 = (uint32_t)((10 * usAVBRConvergence) / (double)150);
        cmd.m_dw3.startGAdjFrame1 = (uint32_t)((50 * usAVBRConvergence) / (double)150);
        cmd.m_dw4.startGAdjFrame2 = (uint32_t)((100 * usAVBRConvergence) / (double)150);
        cmd.m_dw4.startGAdjFrame3 = (uint32_t)((150 * usAVBRConvergence) / (double)150);
        cmd.m_dw11.gRateRatioThreshold0 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30) * (100 - 40)));
        cmd.m_dw11.gRateRatioThreshold1 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30) * (100 - 75)));
        cmd.m_dw12.gRateRatioThreshold2 = (uint32_t)((100 - (usAVBRAccuracy / (double)30) * (100 - 97)));
        cmd.m_dw12.gRateRatioThreshold3 = (uint32_t)((100 + (usAVBRAccuracy / (double)30) * (103 - 100)));
        cmd.m_dw12.gRateRatioThreshold4 = (uint32_t)((100 + (usAVBRAccuracy / (double)30) * (125 - 100)));
        cmd.m_dw12.gRateRatioThreshold5 = (uint32_t)((100 + (usAVBRAccuracy / (double)30) * (160 - 100)));
    }

    cmd.m_dw15.EnableROI = params->ucEnableROI;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));

    cmd.m_dw15.RoundingIntra = 5;
    cmd.m_dw15.RoundingInter = sliceState.dwRoundingValue;

    uint32_t profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        (uint32_t*)&profileLevelMaxFrame));

    cmd.m_dw19.UserMaxFrame = profileLevelMaxFrame;
    cmd.m_dw24.SurfaceIndexBRChistorybuffer                  = frameBrcUpdateHistory;
    cmd.m_dw25.SurfaceIndexPreciousPAKstatisticsoutputbuffer = frameBrcUpdatePakStatisticsOutput;
    cmd.m_dw26.SurfaceIndexAVCIMGstateinputbuffer            = frameBrcUpdateImageStateRead;
    cmd.m_dw27.SurfaceIndexAVCIMGstateoutputbuffer           = frameBrcUpdateImageStateWrite;
    cmd.m_dw28.SurfaceIndexAVC_Encbuffer                     = frameBrcUpdateMbencCurbeWrite;
    cmd.m_dw29.SurfaceIndexAVCDISTORTIONbuffer               = frameBrcUpdateDistortion;
    cmd.m_dw30.SurfaceIndexBRCconstdatabuffer                = frameBrcUpdateConstantData;
    cmd.m_dw31.SurfaceIndexMBStatsBuffer                     = frameBrcUpdateMbStat;
    cmd.m_dw32.SurfaceIndexMotionvectorbuffer                = frameBrcUpdateMvStat;
    auto pStateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pStateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcUpdateParam(
            &cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::SetCurbeAvcMbBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    MbBrcUpdateCurbe curbe;

    // BRC curbe requires: 2 for I-frame, 0 for P-frame, 1 for B-frame
    curbe.m_dw0.CurrFrameType = (m_pictureCodingType + 1) % 3;
    if( params->ucEnableROI )
    {
        if (bROIValueInDeltaQP)
        {
            curbe.m_dw0.EnableROI = 2;  // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled
            curbe.m_dw0.ROIRatio  = 0;
        }
        else
        {
            curbe.m_dw0.EnableROI = 1;  // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled

            uint32_t roisize = 0;
            uint32_t roiratio = 0;

            for (uint32_t i = 0 ; i < m_avcPicParam->NumROI ; ++i)
            {
                CODECHAL_ENCODE_VERBOSEMESSAGE("ROI[%d] = {%d, %d, %d, %d} {%d}, size = %d", i,
                    m_avcPicParam->ROI[i].Left, m_avcPicParam->ROI[i].Top,
                    m_avcPicParam->ROI[i].Bottom, m_avcPicParam->ROI[i].Right,
                    m_avcPicParam->ROI[i].PriorityLevelOrDQp,
                    (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                    (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left)));
                roisize += (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                                (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left));
            }

            if (roisize)
            {
                uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
                roiratio = 2 * (numMBs * 256 / roisize - 1);
                roiratio = MOS_MIN(51, roiratio); // clip QP from 0-51
            }
            CODECHAL_ENCODE_VERBOSEMESSAGE("ROIRatio = %d", roiratio);
            curbe.m_dw0.ROIRatio = roiratio;
        }
    }
    else
    {
        curbe.m_dw0.ROIRatio = 0;
    }

    if (m_avcPicParam->bEnableQpAdjustment)
    {
        curbe.m_dw0.CQP_QPValue = MOS_MIN(m_avcPicParam->QpY + m_avcSliceParams->slice_qp_delta, 51);
        curbe.m_dw1.EnableCQPMode = 1;
    }

    curbe.m_dw8.HistorybufferIndex        = mbBrcUpdateHistory;
    curbe.m_dw9.MBQPbufferIndex           = mbBrcUpdateMbQp;
    curbe.m_dw10.ROIbufferIndex           = mbBrcUpdateRoi;
    curbe.m_dw11.MBstatisticalbufferIndex = mbBrcUpdateMbStat;
    auto pStateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pStateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &curbe,
        params->pKernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::SetCurbeAvcBrcBlockCopy(PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS params)
{
    MOS_STATUS                                        eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    BrcBlockCopyCurbe cmd;
    cmd.m_dw0.BufferOffset    = params->dwBufferOffset;
    cmd.m_dw0.BlockHeight     = params->dwBlockHeight;
    cmd.m_dw1.SrcSurfaceIndex = 0x00;
    cmd.m_dw2.DstSurfaceIndex = 0x01;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::UserFeatureKeyReport());

#if (_DEBUG || _RELEASE_INTERNAL)

    // VE2.0 Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID, MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface));

#endif // _DEBUG || _RELEASE_INTERNAL
    return eStatus;
}
//------------------------------------------------------------------------------
//| Purpose:    Send surface for AVC MBEnc kernels
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncodeAvcEncG11::SendAvcMbEncSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params)
{
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSlcParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ppRefList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrReconstructedPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psCurrPicSurface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcPicIdx);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMbEncBindingTable);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    auto kernelState = params->pKernelState;
    auto avcMbEncBindingTable = params->pMbEncBindingTable;
    bool currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    bool currBottomField = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;
    auto currPicRefListEntry = params->ppRefList[params->pCurrReconstructedPic->FrameIdx];
    auto mbCodeBuffer = &currPicRefListEntry->resRefMbCodeBuffer;
    auto mvDataBuffer = &currPicRefListEntry->resRefMvDataBuffer;
    uint32_t refMbCodeBottomFieldOffset =
        params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * 64;
    uint32_t refMvBottomFieldOffset =
        MOS_ALIGN_CEIL(params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * (32 * 4), 0x1000);

    uint8_t vdirection, refVDirection;
    if (params->bMbEncIFrameDistInUse)
    {
        vdirection = CODECHAL_VDIRECTION_FRAME;
    }
    else
    {
        vdirection = (CodecHal_PictureIsFrame(*(params->pCurrOriginalPic))) ? CODECHAL_VDIRECTION_FRAME :
            (currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;
    }

    // PAK Obj command buffer
    uint32_t size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = mbCodeBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwMbCodeBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMfcAvcPakObj;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MV data buffer
    size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = mvDataBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwMvBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = avcMbEncBindingTable->dwAvcMBEncIndMVData;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Current Picture Y
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true; // Use media block RW for DP 2D surface access
    surfaceCodecParams.bUseUVPlane = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncCurrY;
    surfaceCodecParams.dwUVBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncCurrUV;
    surfaceCodecParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC_ME MV data buffer
    if (params->bHmeEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeMvDataBuffer;
        surfaceCodecParams.dwOffset = params->dwMeMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMVDataFromME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeDistortionBuffer);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeDistortionBuffer;
        surfaceCodecParams.dwOffset = params->dwMeDistortionBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMEDist;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMbConstDataBufferInUse)
    {
        // 16 DWs per QP value
        size = 16 * 52 * sizeof(uint32_t);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presMbBrcConstDataBuffer;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MB_BRC_CONST_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMbBrcConstData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMbQpBufferInUse)
    {
        // AVC MB BRC QP buffer
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMbQpBuffer;
        surfaceCodecParams.dwOffset = params->dwMbQpBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_MB_QP_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = currFieldPicture ? avcMbEncBindingTable->dwAvcMBEncMbQpField :
            avcMbEncBindingTable->dwAvcMBEncMbQpFrame;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMbSpecificDataEnabled)
    {
        size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * sizeof(CODECHAL_ENCODE_AVC_MB_SPECIFIC_PARAMS);
        CODECHAL_ENCODE_VERBOSEMESSAGE("Send MB specific surface, size = %d", size);
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMbSpecificDataBuffer;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMbSpecificData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Current Picture Y - VME
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ?
        avcMbEncBindingTable->dwAvcMBEncFieldCurrPic[0] : avcMbEncBindingTable->dwAvcMBEncCurrPicFrame[0];
    surfaceCodecParams.ucVDirection = vdirection;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ?
        avcMbEncBindingTable->dwAvcMBEncFieldCurrPic[1] : avcMbEncBindingTable->dwAvcMBEncCurrPicFrame[1];
    surfaceCodecParams.ucVDirection = vdirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Setup references 1...n
    // LIST 0 references
    uint8_t refIdx;
    for (refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        auto refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
        uint32_t refMbCodeBottomFieldOffsetUsed;
        uint32_t refMvBottomFieldOffsetUsed;
        uint32_t refBindingTableOffset;
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? true : false;
            // Program the surface based on current picture's field/frame mode
            if (currFieldPicture) // if current picture is field
            {
                if (refBottomField)
                {
                    refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicTopField[refIdx];
                }
            }
            else // if current picture is frame
            {
                refVDirection = CODECHAL_VDIRECTION_FRAME;
                refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicFrame[refIdx];
            }

            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            if((0 == refIdx) && (params->bUseWeightedSurfaceForL0))
            {
                surfaceCodecParams.psSurface = m_wpState->GetWPOutputPicList(CODEC_WP_OUTPUT_L0_START + refIdx);
            }
            else
            {
                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
            }
            surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
            surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;

            surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
            surfaceCodecParams.ucVDirection = refVDirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    // Setup references 1...n
    // LIST 1 references
    uint32_t curbeSize;
    for (refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        if (!currFieldPicture && refIdx > 0)
        {
            // Only 1 LIST 1 reference required here since only single ref is supported in frame case
            break;
        }

        auto refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
        uint32_t refMbCodeBottomFieldOffsetUsed;
        uint32_t refMvBottomFieldOffsetUsed;
        uint32_t refBindingTableOffset;
        bool     refBottomField;
        uint8_t  refPicIdx;
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            // Program the surface based on current picture's field/frame mode
            if (currFieldPicture) // if current picture is field
            {
                if (refBottomField)
                {
                    refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    refMbCodeBottomFieldOffsetUsed = refMbCodeBottomFieldOffset;
                    refMvBottomFieldOffsetUsed = refMvBottomFieldOffset;
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    refMbCodeBottomFieldOffsetUsed = 0;
                    refMvBottomFieldOffsetUsed = 0;
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx];
                }
            }
            else // if current picture is frame
            {
                refVDirection = CODECHAL_VDIRECTION_FRAME;
                refMbCodeBottomFieldOffsetUsed = 0;
                refMvBottomFieldOffsetUsed = 0;
                refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx];
            }

            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            if((0 == refIdx) && (params->bUseWeightedSurfaceForL1))
            {
                surfaceCodecParams.psSurface = m_wpState->GetWPOutputPicList(CODEC_WP_OUTPUT_L1_START + refIdx);
            }
            else
            {
                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
            }
            surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
            surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;

            surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
            surfaceCodecParams.ucVDirection = refVDirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            if (refIdx == 0)
            {
                // MB data buffer
                size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.dwSize = size;
                surfaceCodecParams.presBuffer = &params->ppRefList[refPicIdx]->resRefMbCodeBuffer;
                surfaceCodecParams.dwOffset = refMbCodeBottomFieldOffsetUsed;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdRefMBData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));

                // MV data buffer
                size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.dwSize = size;
                surfaceCodecParams.presBuffer = &params->ppRefList[refPicIdx]->resRefMvDataBuffer;
                surfaceCodecParams.dwOffset = refMvBottomFieldOffsetUsed;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdRefMVData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }

            if (refIdx < CODECHAL_ENCODE_NUM_MAX_VME_L1_REF)
            {
                if (currFieldPicture)
                {
                    // The binding table contains multiple entries for IDX0 backwards references
                    if (refBottomField)
                    {
                        refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                    else
                    {
                        refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                }
                else
                {
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                }

                // Picture Y VME
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.bUseAdvState = true;
                surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
                surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;
                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
                surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
                surfaceCodecParams.ucVDirection = refVDirection;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
        }
    }

    // BRC distortion data buffer for I frame
    if (params->bMbEncIFrameDistInUse)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMeBrcDistortionBuffer;
        surfaceCodecParams.dwOffset = params->dwMeBrcDistortionBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBRCDist;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // RefPicSelect of Current Picture
    if (params->bUsedAsRef)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &currPicRefListEntry->pRefPicSelectListEntry->sBuffer;
        surfaceCodecParams.psSurface->dwHeight = MOS_ALIGN_CEIL(surfaceCodecParams.psSurface->dwHeight, 8);
        surfaceCodecParams.dwOffset = params->dwRefPicSelectBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncRefPicSelectL0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if(params->bMBVProcStatsEnabled)
    {
        size = (currFieldPicture ? 1 : 2) * params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * sizeof(uint32_t);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMBVProcStatsBuffer;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwMBVProcStatsBottomFieldOffset : 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_MB_STATS_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMBStats;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else if (params->bFlatnessCheckEnabled)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psFlatnessCheckSurface;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwFlatnessCheckBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFlatnessChk;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE].Value;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMADEnabled)
    {
        size = CODECHAL_MAD_BUFFER_SIZE;

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bRawSurface = true;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMADDataBuffer;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMADData;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MAD_ENCODE].Value;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->dwMbEncBRCBufferSize > 0)
    {
        //Started from GEN95, separated Mbenc curbe from BRC update kernel. BRC update kernel will generate a 128 bytes surface for mbenc.
        //The new surface contains the updated data for mbenc. MBenc kernel has been changed to use the new BRC update output surface
        //to update its curbe internally.
        // MbEnc BRC buffer - write only
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presMbEncBRCBuffer;
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwMbEncBRCBufferSize);
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        if (params->bUseMbEncAdvKernel)
        {
            // For BRC the new BRC surface is used
            if (params->bUseAdvancedDsh)
            {
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.presBuffer = params->presMbEncCurbeBuffer;
                curbeSize = MOS_ALIGN_CEIL(
                    params->pKernelState->KernelParams.iCurbeLength,
                    m_hwInterface->GetRenderInterface()->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
                surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(curbeSize);
                surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
            else // For CQP the DSH CURBE is used
            {
                MOS_RESOURCE *dsh = nullptr;
                CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = params->pKernelState->m_dshRegion.GetResource());
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.presBuffer = dsh;
                surfaceCodecParams.dwOffset =
                    params->pKernelState->m_dshRegion.GetOffset() +
                    params->pKernelState->dwCurbeOffset;
                curbeSize = MOS_ALIGN_CEIL(
                    params->pKernelState->KernelParams.iCurbeLength,
                    m_hwInterface->GetRenderInterface()->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
                surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(curbeSize);
                surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
        }
    }

    if (params->bArbitraryNumMbsInSlice)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psSliceMapSurface;
        surfaceCodecParams.bRenderTarget = false;
        surfaceCodecParams.bIsWritable = false;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwSliceMapBottomFieldOffset : 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_SLICE_MAP_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncSliceMapData;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if(!params->bMbEncIFrameDistInUse)
    {
        if( params->bMbDisableSkipMapEnabled )
        {
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bIs2DSurface = true;
            surfaceCodecParams.bMediaBlockRW = true;
            surfaceCodecParams.psSurface = params->psMbDisableSkipMapSurface;
            surfaceCodecParams.dwOffset = 0;
            surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMbNonSkipMap;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_MBDISABLE_SKIPMAP_CODEC].Value;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        if( params->bStaticFrameDetectionEnabled )
        {
            // static frame cost table surface
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.presBuffer = params->presSFDCostTableBuffer;
            surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(sfdCostTableBufferSizeCommon );
            surfaceCodecParams.dwOffset = 0;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncStaticDetectionCostTable;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = m_swScoreboardState->GetCurSwScoreboardSurface();
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_SOFTWARE_SCOREBOARD_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = mbencSwScoreboard;
        surfaceCodecParams.bUse32UINTSurfaceFormat = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

}
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::SendAvcBrcFrameUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBrcBuffers);

    // BRC history buffer
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    auto kernelState = params->pKernelState;
    auto avcBrcUpdateBindingTable = params->pBrcUpdateBindingTable;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcHistoryBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcHistoryBufferSize);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_HISTORY_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcHistoryBuffer;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK Statistics buffer
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcPakStatisticBuffer[0];
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcPakStatisticsSize);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_PAK_STATS_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcPakStatisticsOutputBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - read only
    uint32_t size = MOS_BYTES_TO_DWORDS(BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses());
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer =
        &params->pBrcBuffers->resBrcImageStatesReadBuffer[params->ucCurrRecycledBufIdx];
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_PAK_IMAGESTATE_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcImageStateReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - write only
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcImageStatesWriteBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_PAK_IMAGESTATE_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcImageStateWriteBuffer;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (params->dwMbEncBRCBufferSize > 0)
    {
        //Started from GEN95, separated Mbenc curbe from BRC update kernel. BRC update kernel will generate a 128 bytes surface for mbenc.
        //The new surface contains the updated data for mbenc. MBenc kernel has been changed to use the new BRC update output surface
        //to update its curbe internally.
        // MbEnc BRC buffer - write only
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = &params->pBrcBuffers->resMbEncBrcBuffer;
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwMbEncBRCBufferSize);
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_BRC_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        PMHW_KERNEL_STATE pMbEncKernelState;
        CODECHAL_ENCODE_CHK_NULL_RETURN(pMbEncKernelState = params->pBrcBuffers->pMbEncKernelStateInUse);

        MOS_RESOURCE *dsh = nullptr;
        CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = pMbEncKernelState->m_dshRegion.GetResource());

        // BRC ENC CURBE Buffer - read only
        size = MOS_ALIGN_CEIL(
            pMbEncKernelState->KernelParams.iCurbeLength,
            m_renderEngineInterface->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = dsh;
        surfaceCodecParams.dwOffset =
            pMbEncKernelState->m_dshRegion.GetOffset() +
            pMbEncKernelState->dwCurbeOffset;
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(size);
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeReadBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // BRC ENC CURBE Buffer - write only
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        if (params->bUseAdvancedDsh)
        {
            surfaceCodecParams.presBuffer = params->presMbEncCurbeBuffer;
        }
        else
        {
            surfaceCodecParams.presBuffer = dsh;
            surfaceCodecParams.dwOffset =
                pMbEncKernelState->m_dshRegion.GetOffset() +
                pMbEncKernelState->dwCurbeOffset;
        }
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(size);
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // AVC_ME BRC Distortion data buffer - input/output
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface = &params->pBrcBuffers->sMeBrcDistortionBuffer;
    surfaceCodecParams.dwOffset = params->pBrcBuffers->dwMeBrcDistortionBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcDistortionBuffer;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Constant Data Surface
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface =
        &params->pBrcBuffers->sBrcConstantDataBuffer[params->ucCurrRecycledBufIdx];
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_CONSTANT_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcConstantData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MBStat buffer - input
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = params->presMbStatBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(m_hwInterface->m_avcMbStatBufferSize);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_MB_STATS_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbStatBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MV data buffer
    if (params->psMvDataBuffer)
    {
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMvDataBuffer;
        surfaceCodecParams.dwOffset = params->dwMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMvDataBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::SendAvcBrcMbUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBrcBuffers);

    // BRC history buffer
    auto kernelState = params->pKernelState;
    auto avcBrcUpdateBindingTable = params->pBrcUpdateBindingTable;
    CODECHAL_SURFACE_CODEC_PARAMS                   surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcHistoryBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcHistoryBufferSize);
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_HISTORY_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwMbBrcHistoryBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC MB QP data buffer
    if (params->bMbBrcEnabled)
    {
        params->pBrcBuffers->sBrcMbQpBuffer.dwHeight = MOS_ALIGN_CEIL((params->dwDownscaledFrameFieldHeightInMb4x << 2), 8);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.psSurface = &params->pBrcBuffers->sBrcMbQpBuffer;
        surfaceCodecParams.dwOffset = params->pBrcBuffers->dwBrcMbQpBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_MB_QP_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwMbBrcMbQpBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // BRC ROI feature
    if (params->bBrcRoiEnabled)
    {
        params->psRoiSurface->dwHeight = MOS_ALIGN_CEIL((params->dwDownscaledFrameFieldHeightInMb4x << 2), 8);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.bIsWritable = false;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.psSurface = params->psRoiSurface;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ROI_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwMbBrcROISurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // MBStat buffer
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = params->presMbStatBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(m_hwInterface->m_avcMbStatBufferSize);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_MB_STATS_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwMbBrcMbStatBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::SetGpuCtxCreatOption()
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

MOS_STATUS CodechalEncodeAvcEncG11::SetupROISurface()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS readOnly;
    MOS_ZeroMemory(&readOnly, sizeof(readOnly));
    readOnly.ReadOnly = 1;
    uint32_t * data = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &BrcBuffers.sBrcRoiSurface.OsResource, &readOnly);
    if (!data)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        return eStatus;
    }

    uint32_t bufferWidthInByte = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x << 4), 64);//(m_picWidthInMb * 4 + 63) & ~63;
    uint32_t bufferHeightInByte = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8);//(m_picHeightInMb + 7) & ~7;
    uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
    for (uint32_t uMB = 0; uMB <= numMBs; uMB++)
    {
        int32_t curMbY = uMB / m_picWidthInMb;
        int32_t curMbX = uMB - curMbY * m_picWidthInMb;

        uint32_t outdata = 0;
        for (int32_t roi = (m_avcPicParam->NumROI - 1); roi >= 0; roi--)
        {
            int32_t QPLevel;
            if (bROIValueInDeltaQP)
            {
                QPLevel = -m_avcPicParam->ROI[roi].PriorityLevelOrDQp;
            }
            else
            {
                // QP Level sent to ROI surface is (priority * 6)
                QPLevel = m_avcPicParam->ROI[roi].PriorityLevelOrDQp * 6;
            }

            if (QPLevel == 0)
            {
                continue;
            }

            if ((curMbX >= (int32_t)m_avcPicParam->ROI[roi].Left) && (curMbX < (int32_t)m_avcPicParam->ROI[roi].Right) &&
                (curMbY >= (int32_t)m_avcPicParam->ROI[roi].Top) && (curMbY < (int32_t)m_avcPicParam->ROI[roi].Bottom))
            {
                outdata = 15 | ((QPLevel & 0xFF) << 8);
            }
            else if (bROISmoothEnabled)
            {
                if ((curMbX >= (int32_t)m_avcPicParam->ROI[roi].Left - 1) && (curMbX < (int32_t)m_avcPicParam->ROI[roi].Right + 1) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[roi].Top - 1) && (curMbY < (int32_t)m_avcPicParam->ROI[roi].Bottom + 1))
                {
                    outdata = 14 | ((QPLevel & 0xFF) << 8);
                }
                else if ((curMbX >= (int32_t)m_avcPicParam->ROI[roi].Left - 2) && (curMbX < (int32_t)m_avcPicParam->ROI[roi].Right + 2) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[roi].Top - 2) && (curMbY < (int32_t)m_avcPicParam->ROI[roi].Bottom + 2))
                {
                    outdata = 13 | ((QPLevel & 0xFF) << 8);
                }
                else if ((curMbX >= (int32_t)m_avcPicParam->ROI[roi].Left - 3) && (curMbX < (int32_t)m_avcPicParam->ROI[roi].Right + 3) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[roi].Top - 3) && (curMbY < (int32_t)m_avcPicParam->ROI[roi].Bottom + 3))
                {
                    outdata = 12 | ((QPLevel & 0xFF) << 8);
                }
            }
        }
        data[(curMbY * (bufferWidthInByte>>2)) + curMbX] = outdata;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &BrcBuffers.sBrcRoiSurface.OsResource);

    uint32_t bufferSize = bufferWidthInByte * bufferHeightInByte;
    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &BrcBuffers.sBrcRoiSurface.OsResource,
        CodechalDbgAttr::attrInput,
        "ROI",
        bufferSize,
        0,
        CODECHAL_MEDIA_STATE_MB_BRC_UPDATE)));
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG11::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    bool                        frameTracking,
    MHW_MI_MMIOREGISTERS       *mmioRegister)
{
    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        if (cmdBuffer->Attributes.pAttriVe)
        {
            PMOS_CMD_BUF_ATTRI_VE attriExt =
                    (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);
            attriExt->bUseVirtualEngineHint = true;
            attriExt->VEngineHintParams.NeedSyncWithPrevious = 1;
        }
    }

    return CodechalEncodeAvcEnc::SendPrologWithFrameTracking(cmdBuffer, frameTracking, mmioRegister);
}

MOS_STATUS CodechalEncodeAvcEncG11::InitKernelStateMe()
{
    m_hmeKernel = MOS_New(CodechalKernelHmeG11, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hmeKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Initialize(
        GetCommonKernelHeaderAndSizeG11,
        m_kernelBase,
        m_kuidCommon));
    return MOS_STATUS_SUCCESS;
}

void CodechalEncodeAvcEncG11::ResizeOnResChange()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncoderState::ResizeOnResChange();

    // need to re-allocate surfaces according to resolution
    m_swScoreboardState->ReleaseResources();
}

MOS_STATUS CodechalEncodeAvcEncG11::UpdateCmdBufAttribute(
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

        memset((void *)attriExt, 0, sizeof(MOS_CMD_BUF_ATTRI_VE));
        attriExt->bUseVirtualEngineHint =
            attriExt->VEngineHintParams.NeedSyncWithPrevious = !renderEngineInUse;
    }

    return eStatus;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncodeAvcEncG11::PopulateBrcInitParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    BrcInitResetCurbeG11 *curbe = (BrcInitResetCurbeG11 *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MBBRCEnable          = bMbBrcEnabled;
        m_avcPar->MBRC                 = bMbBrcEnabled;
        m_avcPar->BitRate              = curbe->m_dw3.AverageBitRate;
        m_avcPar->InitVbvFullnessInBit = curbe->m_dw1.InitBufFullInBits;
        m_avcPar->MaxBitRate           = curbe->m_dw4.MaxBitRate;
        m_avcPar->VbvSzInBit           = curbe->m_dw2.BufSizeInBits;
        m_avcPar->AvbrAccuracy         = curbe->m_dw10.AVBRAccuracy;
        m_avcPar->AvbrConvergence      = curbe->m_dw11.AVBRConvergence;
        m_avcPar->SlidingWindowSize    = curbe->m_dw22.SlidingWindowSize;
        m_avcPar->LongTermInterval     = curbe->m_dw24.LongTermInterval;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG11::PopulateBrcUpdateParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    FrameBrcUpdateCurbe *curbe = (FrameBrcUpdateCurbe *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->EnableMultipass     = (curbe->m_dw5.MaxNumPAKs > 0) ? 1 : 0;
        m_avcPar->MaxNumPakPasses     = curbe->m_dw5.MaxNumPAKs;
        m_avcPar->SlidingWindowEnable = curbe->m_dw6.EnableSlidingWindow;
        m_avcPar->FrameSkipEnable     = curbe->m_dw6.EnableForceToSkip;
        m_avcPar->UserMaxFrame        = curbe->m_dw19.UserMaxFrame;
    }
    else
    {
        m_avcPar->UserMaxFrameP = curbe->m_dw19.UserMaxFrame;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG11::PopulateEncParam(
    uint8_t meMethod,
    void    *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    MbencCurbe *curbe = (MbencCurbe *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MRDisableQPCheck = MRDisableQPCheck[m_targetUsage];
        m_avcPar->AllFractional =
            CODECHAL_ENCODE_AVC_AllFractional_Common[m_targetUsage & 0x7];
        m_avcPar->DisableAllFractionalCheckForHighRes =
            CODECHAL_ENCODE_AVC_DisableAllFractionalCheckForHighRes_Common[m_targetUsage & 0x7];
        m_avcPar->EnableAdaptiveSearch              = curbe->m_dw37.AdaptiveEn;
        m_avcPar->EnableFBRBypass                   = curbe->m_dw4.EnableFBRBypass;
        m_avcPar->BlockBasedSkip                    = curbe->m_dw3.BlockBasedSkipEnable;
        m_avcPar->MADEnableFlag                     = curbe->m_dw34.MADEnableFlag;
        m_avcPar->MBTextureThreshold                = curbe->m_dw60.MBTextureThreshold;
        m_avcPar->EnableMBFlatnessCheckOptimization = curbe->m_dw34.EnableMBFlatnessChkOptimization;
        m_avcPar->EnableArbitrarySliceSize          = curbe->m_dw34.ArbitraryNumMbsPerSlice;
        m_avcPar->RefThresh                         = curbe->m_dw38.RefThreshold;
        m_avcPar->EnableWavefrontOptimization       = curbe->m_dw4.EnableWavefrontOptimization;
        m_avcPar->MaxLenSP                          = curbe->m_dw2.LenSP;
        m_avcPar->DisableExtendedMvCostRange        = !curbe->m_dw1.ExtendedMvCostRange;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        m_avcPar->MEMethod                             = meMethod;
        m_avcPar->HMECombineLen                        = HMECombineLen[m_targetUsage];
        m_avcPar->FTQBasedSkip                         = FTQBasedSkip[m_targetUsage];
        m_avcPar->MultiplePred                         = MultiPred[m_targetUsage];
        m_avcPar->EnableAdaptiveIntraScaling           = bAdaptiveIntraScalingEnable;
        m_avcPar->StaticFrameIntraCostScalingRatioP    = 240;
        m_avcPar->SubPelMode                           = curbe->m_dw3.SubPelMode;
        m_avcPar->HMECombineOverlap                    = curbe->m_dw36.HMECombineOverlap;
        m_avcPar->SearchX                              = curbe->m_dw5.RefWidth;
        m_avcPar->SearchY                              = curbe->m_dw5.RefHeight;
        m_avcPar->SearchControl                        = curbe->m_dw3.SearchCtrl;
        m_avcPar->EnableAdaptiveTxDecision             = curbe->m_dw34.EnableAdaptiveTxDecision;
        m_avcPar->TxDecisionThr                        = curbe->m_dw60.TxDecisonThreshold;
        m_avcPar->EnablePerMBStaticCheck               = curbe->m_dw34.EnablePerMBStaticCheck;
        m_avcPar->EnableAdaptiveSearchWindowSize       = curbe->m_dw34.EnableAdaptiveSearchWindowSize;
        m_avcPar->EnableIntraCostScalingForStaticFrame = curbe->m_dw4.EnableIntraCostScalingForStaticFrame;
        m_avcPar->BiMixDisable                         = curbe->m_dw0.BiMixDis;
        m_avcPar->SurvivedSkipCost                     = (curbe->m_dw7.NonSkipZMvAdded << 1) + curbe->m_dw7.NonSkipModeAdded;
        m_avcPar->UniMixDisable                        = curbe->m_dw1.UniMixDisable;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        m_avcPar->BMEMethod                         = meMethod;
        m_avcPar->HMEBCombineLen                    = HMEBCombineLen[m_targetUsage];
        m_avcPar->StaticFrameIntraCostScalingRatioB = 200;
        m_avcPar->BSearchX                          = curbe->m_dw5.RefWidth;
        m_avcPar->BSearchY                          = curbe->m_dw5.RefHeight;
        m_avcPar->BSearchControl                    = curbe->m_dw3.SearchCtrl;
        m_avcPar->BSkipType                         = curbe->m_dw3.SkipType;
        m_avcPar->DirectMode                        = curbe->m_dw34.bDirectMode;
        m_avcPar->BiWeight                          = curbe->m_dw1.BiWeight;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG11::PopulatePakParam(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER   secondLevelBatchBuffer)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    uint8_t         *data = nullptr;
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    if (cmdBuffer != nullptr)
    {
        data = (uint8_t*)(cmdBuffer->pCmdPtr - (mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD::byteSize / sizeof(uint32_t)));
    }
    else if (secondLevelBatchBuffer != nullptr)
    {
        data = secondLevelBatchBuffer->pData;
    }
    else
    {
        data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &BrcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx], &lockFlags);
    }

    CODECHAL_DEBUG_CHK_NULL(data);

    mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD mfxCmd;
    mfxCmd = *(mhw_vdbox_mfx_g11_X::MFX_AVC_IMG_STATE_CMD *)(data);

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->TrellisQuantizationEnable         = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        m_avcPar->EnableAdaptiveTrellisQuantization = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        m_avcPar->TrellisQuantizationRounding       = mfxCmd.DW5.TrellisQuantizationRoundingTqr;
        m_avcPar->TrellisQuantizationChromaDisable  = mfxCmd.DW5.TrellisQuantizationChromaDisableTqchromadisable;
        m_avcPar->ExtendedRhoDomainEn               = mfxCmd.DW17.ExtendedRhodomainStatisticsEnable;
    }

    if (data && (cmdBuffer == nullptr) && (secondLevelBatchBuffer == nullptr))
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &BrcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx]);
    }

    return MOS_STATUS_SUCCESS;
}
#endif
