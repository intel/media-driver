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
//! \file     vphal_render_vebox_base.h
//! \brief    Common interface and structure used in Vebox
//! \details  Common interface and structure used in Vebox which are platform independent
//!
#ifndef __VPHAL_RENDER_VEBOX_BASE_H__
#define __VPHAL_RENDER_VEBOX_BASE_H__

#include "mos_os.h"
#include "renderhal.h"
#include "mhw_vebox.h"
#include "vphal.h"
#include "vphal_render_renderstate.h"
#include "vphal_render_common.h"
#include "vphal_render_vebox_iecp.h"
#include "vphal_render_sfc_base.h"
#include "vphal_render_vebox_denoise.h"

#define VPHAL_MAX_NUM_FFDI_SURFACES     4                                       //!< 2 for ADI plus additional 2 for parallel execution on HSW+
#define VPHAL_NUM_FFDN_SURFACES         2                                       //!< Number of FFDN surfaces
#define VPHAL_NUM_STMM_SURFACES         2                                       //!< Number of STMM statistics surfaces
#define VPHAL_DNDI_BUFFERS_MAX          4                                       //!< Max DNDI buffers
#define VPHAL_NUM_KERNEL_VEBOX          8                                       //!< Max kernels called at Adv stage

#ifndef VEBOX_AUTO_DENOISE_SUPPORTED
#define VEBOX_AUTO_DENOISE_SUPPORTED    1
#endif

//!
//! \brief Denoise Range
//!
#define NOISEFACTOR_MAX                                 64                      //!< Max Slider value
#define NOISEFACTOR_MID                                 32                      //!< Mid Slider value, SKL+ only
#define NOISEFACTOR_MIN                                 0                       //!< Min Slider value

//!
//! \brief Temporal Denoise Definitions
//!
#define NOISE_HISTORY_DELTA_DEFAULT                     8
#define NOISE_HISTORY_MAX_DEFAULT                       192
#define NOISE_NUMMOTIONPIXELS_THRESHOLD_DEFAULT         0
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_DEFAULT    6
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_DEFAULT       12
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_DEFAULT      128

// Pixel Range Threshold Array Denoise Definitions for SKL+ 5x5 Bilateral Filter
#define NOISE_BLF_RANGE_THRESHOLD_ADP_NLVL          1
#define NOISE_BLF_RANGE_THRESHOLD_ADP_NLVL_MIN      65536
#define NOISE_BLF_RANGE_THRESHOLD_ADP_NLVL_MAX      393216
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI0          192
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI1          320
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI2          384
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI3          640
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI4          1024
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI5          1280
#define NOISE_BLF_RANGE_THRADPDYNR_MIN              512
#define NOISE_BLF_RANGE_THRADPDYNR_MAX              2048
#define NOISE_BLF_RANGE_THRDYNR_MIN                 256
#define NOISE_BLF_RANGE_THRESHOLD_S0_MIN            32
#define NOISE_BLF_RANGE_THRESHOLD_S0_MID            192
#define NOISE_BLF_RANGE_THRESHOLD_S0_MAX            384
#define NOISE_BLF_RANGE_THRESHOLD_S1_MIN            64
#define NOISE_BLF_RANGE_THRESHOLD_S1_MID            256
#define NOISE_BLF_RANGE_THRESHOLD_S1_MAX            576
#define NOISE_BLF_RANGE_THRESHOLD_S2_MIN            128
#define NOISE_BLF_RANGE_THRESHOLD_S2_MID            512
#define NOISE_BLF_RANGE_THRESHOLD_S2_MAX            896
#define NOISE_BLF_RANGE_THRESHOLD_S3_MIN            128
#define NOISE_BLF_RANGE_THRESHOLD_S3_MID            640
#define NOISE_BLF_RANGE_THRESHOLD_S3_MAX            1280
#define NOISE_BLF_RANGE_THRESHOLD_S4_MIN            128
#define NOISE_BLF_RANGE_THRESHOLD_S4_MID            896
#define NOISE_BLF_RANGE_THRESHOLD_S4_MAX            1920
#define NOISE_BLF_RANGE_THRESHOLD_S5_MIN            128
#define NOISE_BLF_RANGE_THRESHOLD_S5_MID            1280
#define NOISE_BLF_RANGE_THRESHOLD_S5_MAX            2560
#define NOISE_BLF_RANGE_THRESHOLD_S0_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S0_MID
#define NOISE_BLF_RANGE_THRESHOLD_S1_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S1_MID
#define NOISE_BLF_RANGE_THRESHOLD_S2_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S2_MID
#define NOISE_BLF_RANGE_THRESHOLD_S3_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S3_MID
#define NOISE_BLF_RANGE_THRESHOLD_S4_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S4_MID
#define NOISE_BLF_RANGE_THRESHOLD_S5_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S5_MID
#define NOISE_BLF_RANGE_THRESHOLD_S0_AUTO_DEFAULT   192
#define NOISE_BLF_RANGE_THRESHOLD_S1_AUTO_DEFAULT   320
#define NOISE_BLF_RANGE_THRESHOLD_S2_AUTO_DEFAULT   384
#define NOISE_BLF_RANGE_THRESHOLD_S3_AUTO_DEFAULT   640
#define NOISE_BLF_RANGE_THRESHOLD_S4_AUTO_DEFAULT   1024
#define NOISE_BLF_RANGE_THRESHOLD_S5_AUTO_DEFAULT   1280

// Pixel Range Weight Array Denoise Definitions for SKL+ 5x5 Bilateral Filter
#define NOISE_BLF_RANGE_WGTS0_MIN                   16
#define NOISE_BLF_RANGE_WGTS0_MID                   16
#define NOISE_BLF_RANGE_WGTS0_MAX                   16
#define NOISE_BLF_RANGE_WGTS1_MIN                   9
#define NOISE_BLF_RANGE_WGTS1_MID                   14
#define NOISE_BLF_RANGE_WGTS1_MAX                   15
#define NOISE_BLF_RANGE_WGTS2_MIN                   2
#define NOISE_BLF_RANGE_WGTS2_MID                   10
#define NOISE_BLF_RANGE_WGTS2_MAX                   13
#define NOISE_BLF_RANGE_WGTS3_MIN                   0
#define NOISE_BLF_RANGE_WGTS3_MID                   5
#define NOISE_BLF_RANGE_WGTS3_MAX                   10
#define NOISE_BLF_RANGE_WGTS4_MIN                   0
#define NOISE_BLF_RANGE_WGTS4_MID                   2
#define NOISE_BLF_RANGE_WGTS4_MAX                   7
#define NOISE_BLF_RANGE_WGTS5_MIN                   0
#define NOISE_BLF_RANGE_WGTS5_MID                   1
#define NOISE_BLF_RANGE_WGTS5_MAX                   4
#define NOISE_BLF_RANGE_WGTS0_DEFAULT               NOISE_BLF_RANGE_WGTS0_MID
#define NOISE_BLF_RANGE_WGTS1_DEFAULT               NOISE_BLF_RANGE_WGTS1_MID
#define NOISE_BLF_RANGE_WGTS2_DEFAULT               NOISE_BLF_RANGE_WGTS2_MID
#define NOISE_BLF_RANGE_WGTS3_DEFAULT               NOISE_BLF_RANGE_WGTS3_MID
#define NOISE_BLF_RANGE_WGTS4_DEFAULT               NOISE_BLF_RANGE_WGTS4_MID
#define NOISE_BLF_RANGE_WGTS5_DEFAULT               NOISE_BLF_RANGE_WGTS5_MID

// Distance Weight Matrix Denoise Definitions for SKL+ 5x5 Bilateral Filter
#define NOISE_BLF_DISTANCE_WGTS00_DEFAULT           12
#define NOISE_BLF_DISTANCE_WGTS01_DEFAULT           12
#define NOISE_BLF_DISTANCE_WGTS02_DEFAULT           10
#define NOISE_BLF_DISTANCE_WGTS10_DEFAULT           12
#define NOISE_BLF_DISTANCE_WGTS11_DEFAULT           11
#define NOISE_BLF_DISTANCE_WGTS12_DEFAULT           10
#define NOISE_BLF_DISTANCE_WGTS20_DEFAULT           10
#define NOISE_BLF_DISTANCE_WGTS21_DEFAULT           10
#define NOISE_BLF_DISTANCE_WGTS22_DEFAULT           8

//!
//! \brief Improved Deinterlacing for CNL+
//!
#define VPHAL_VEBOX_DI_CHROMA_TDM_WEIGHT_NATUAL                     0
#define VPHAL_VEBOX_DI_LUMA_TDM_WEIGHT_NATUAL                       4
#define VPHAL_VEBOX_DI_SHCM_DELTA_NATUAL                            5
#define VPHAL_VEBOX_DI_SHCM_THRESHOLD_NATUAL                        255
#define VPHAL_VEBOX_DI_SVCM_DELTA_NATUAL                            5
#define VPHAL_VEBOX_DI_SVCM_THRESHOLD_NATUAL                        255
#define VPHAL_VEBOX_DI_LUMA_TDM_CORING_THRESHOLD_NATUAL             0
#define VPHAL_VEBOX_DI_CHROMA_TDM_CORING_THRESHOLD_NATUAL           0
#define VPHAL_VEBOX_DI_DIRECTION_CHECK_THRESHOLD_NATUAL             3
#define VPHAL_VEBOX_DI_TEARING_LOW_THRESHOLD_NATUAL                 20
#define VPHAL_VEBOX_DI_TEARING_HIGH_THRESHOLD_NATUAL                100
#define VPHAL_VEBOX_DI_DIFF_CHECK_SLACK_THRESHOLD_NATUAL            15
#define VPHAL_VEBOX_DI_SAD_WT0_NATUAL                               0
#define VPHAL_VEBOX_DI_SAD_WT1_NATUAL                               63
#define VPHAL_VEBOX_DI_SAD_WT2_NATUAL                               76
#define VPHAL_VEBOX_DI_SAD_WT3_NATUAL                               89
#define VPHAL_VEBOX_DI_SAD_WT4_NATUAL                               114
#define VPHAL_VEBOX_DI_SAD_WT6_NATUAL                               217
#define VPHAL_VEBOX_DI_LPFWTLUT0_SD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT0_HD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT1_SD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT1_HD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT2_SD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT2_HD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT3_SD_NATUAL                          128
#define VPHAL_VEBOX_DI_LPFWTLUT3_HD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT4_SD_NATUAL                          128
#define VPHAL_VEBOX_DI_LPFWTLUT4_HD_NATUAL                          32
#define VPHAL_VEBOX_DI_LPFWTLUT5_SD_NATUAL                          128
#define VPHAL_VEBOX_DI_LPFWTLUT5_HD_NATUAL                          64
#define VPHAL_VEBOX_DI_LPFWTLUT6_SD_NATUAL                          255
#define VPHAL_VEBOX_DI_LPFWTLUT6_HD_NATUAL                          128
#define VPHAL_VEBOX_DI_LPFWTLUT7_SD_NATUAL                          255
#define VPHAL_VEBOX_DI_LPFWTLUT7_HD_NATUAL                          255

//!
//! \brief Chroma Downsampling and Upsampling for CNL+
//!
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE0_HORZ_OFFSET     0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE1_HORZ_OFFSET     1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE2_HORZ_OFFSET     0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE3_HORZ_OFFSET     1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE4_HORZ_OFFSET     0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE5_HORZ_OFFSET     1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE0_VERT_OFFSET     2
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE1_VERT_OFFSET     2
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE2_VERT_OFFSET     0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE3_VERT_OFFSET     0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE4_VERT_OFFSET     4
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE5_VERT_OFFSET     4
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE0_HORZ_OFFSET  0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE1_HORZ_OFFSET  1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE2_HORZ_OFFSET  0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE3_HORZ_OFFSET  1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE4_HORZ_OFFSET  0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE5_HORZ_OFFSET  1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE0_VERT_OFFSET  1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE1_VERT_OFFSET  1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE2_VERT_OFFSET  0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE3_VERT_OFFSET  0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE4_VERT_OFFSET  2
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE5_VERT_OFFSET  2
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_422_TYPE2_HORZ_OFFSET             0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_422_TYPE3_HORZ_OFFSET             1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_422_TYPE2_VERT_OFFSET             0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_422_TYPE3_VERT_OFFSET             0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE0_HORZ_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE1_HORZ_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE2_HORZ_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE3_HORZ_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE4_HORZ_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE5_HORZ_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE0_VERT_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE1_VERT_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE2_VERT_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE3_VERT_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE4_VERT_OFFSET           2
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE5_VERT_OFFSET           2
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE2_HORZ_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE3_HORZ_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE2_VERT_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE3_VERT_OFFSET           0
enum GFX_MEDIA_VEBOX_DI_OUTPUT_MODE
{
    MEDIA_VEBOX_DI_OUTPUT_BOTH            = 0,
    MEDIA_VEBOX_DI_OUTPUT_PREVIOUS        = 1,
    MEDIA_VEBOX_DI_OUTPUT_CURRENT         = 2
};

enum MEDIASTATE_DNDI_FIELDCOPY_SELECT
{
    MEDIASTATE_DNDI_DEINTERLACE     = 0,
    MEDIASTATE_DNDI_FIELDCOPY_PREV  = 1,
    MEDIASTATE_DNDI_FIELDCOPY_NEXT  = 2
};

struct VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION
{
    // DWORD 0
    union
    {
        // RangeThrStart0
        struct
        {
            uint32_t       RangeThrStart0;
        };

        uint32_t       Value;
    } DW00;

    // DWORD 1
    union
    {
        // RangeThrStart1
        struct
        {
            uint32_t       RangeThrStart1;
        };

        uint32_t       Value;
    } DW01;

    // DWORD 2
    union
    {
        // RangeThrStart2
        struct
        {
            uint32_t       RangeThrStart2;
        };

        uint32_t   Value;
    } DW02;

    // DWORD 3
    union
    {
        // RangeThrStart3
        struct
        {
            uint32_t       RangeThrStart3;
        };

        uint32_t   Value;
    } DW03;

    // DWORD 4
    union
    {
        // RangeThrStart4
        struct
        {
            uint32_t       RangeThrStart4;
        };

        uint32_t   Value;
    } DW04;

    // DWORD 5
    union
    {
        // RangeThrStart5
        struct
        {
            uint32_t       RangeThrStart5;
        };

        uint32_t   Value;
    } DW05;

    // DWORD 6
    union
    {
        // Reserved
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t   Value;
    } DW06;

    // DWORD 7
    union
    {
        // Reserved
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t   Value;
    } DW07;

    // DWORD 8
    union
    {
        // RangeWgt0
        struct
        {
            uint32_t       RangeWgt0;
        };

        uint32_t   Value;
    } DW08;

    // DWORD 9
    union
    {
        // RangeWgt1
        struct
        {
            uint32_t       RangeWgt1;
        };

        uint32_t   Value;
    } DW09;

    // DWORD 10
    union
    {
        // RangeWgt2
        struct
        {
            uint32_t       RangeWgt2;
        };

        uint32_t   Value;
    } DW10;

    // DWORD 11
    union
    {
        // RangeWgt3
        struct
        {
            uint32_t       RangeWgt3;
        };

        uint32_t   Value;
    } DW11;

    // DWORD 12
    union
    {
        // RangeWgt4
        struct
        {
            uint32_t       RangeWgt4;
        };

        uint32_t   Value;
    } DW12;

    // DWORD 13
    union
    {
        // RangeWgt5
        struct
        {
            uint32_t       RangeWgt5;
        };

        uint32_t   Value;
    } DW13;

    // DWORD 14
    union
    {
        // Reserved
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t   Value;
    } DW14;

    // DWORD 15
    union
    {
        // Reserved
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t   Value;
    } DW15;

    // DWORD 16 - 41: DistWgt[5][5]
    uint32_t DistWgt[5][5];

    // Padding for 32-byte alignment, VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION_G9 is 7 uint32_ts
    uint32_t dwPad[7];
};

//!
//! \brief Enumeration for the user feature key "Bypass Composition" values
//!
typedef enum _VPHAL_COMP_BYPASS_MODE
{
    VPHAL_COMP_BYPASS_NOT_SET  = 0xffffffff,
    VPHAL_COMP_BYPASS_DISABLED = 0x0,
    VPHAL_COMP_BYPASS_ENABLED  = 0x1
} VPHAL_COMP_BYPASS_MODE, *PVPHAL_COMP_BYPASS_MODE;

//!
//! \brief Kernel IDs
//!
typedef enum _VPHAL_VEBOX_KERNELID
{
    KERNEL_RESERVED = 0,
    KERNEL_UPDATEDNSTATE,
    KERNEL_VEBOX_BASE_MAX,
} VPHAL_VEBOX_KERNELID, *PVPHAL_VEBOX_KERNELID;

//!
//! \brief VPHAL Query Type for Vebox Statistics Surface
//!
typedef enum _VEBOX_STAT_QUERY_TYPE
{
    VEBOX_STAT_QUERY_FMD_OFFEST = 0,
    VEBOX_STAT_QUERY_WB_OFFEST,
    VEBOX_STAT_QUERY_GNE_OFFEST,
    VEBOX_STAT_QUERY_STD_OFFEST,
    VEBOX_STAT_QUERY_GCC_OFFEST,
    VEBOX_STAT_QUERY_PER_FRAME_SIZE
} VEBOX_STAT_QUERY_TYPE;

#define VPHAL_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE                (256 * 4)
#define VPHAL_VEBOX_ACE_HISTOGRAM_SIZE_PER_FRAME_PER_SLICE      (256 * 4)

//!
//! \brief Secure Block Copy kernel width
//!
#define SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH     64

//!
//! \brief Secure Block Copy kernel block height
//!
#define SECURE_BLOCK_COPY_KERNEL_BLOCK_HEIGHT   24

//!
//! \brief Secure Block Copy kernel inline data size
//!
#define SECURE_BLOCK_COPY_KERNEL_INLINE_SIZE    (1 * sizeof(uint32_t))

#define VPHAL_NUM_RGB_CHANNEL                   3
#define VPHAL_NUM_FRAME_PREVIOUS_CURRENT        2

//!
//! \brief Binding Table Index for Secure Block Copy kernel
//!
#define BI_SECURE_BLOCK_COPY_INPUT              0
#define BI_SECURE_BLOCK_COPY_OUTPUT             1

//!
//! \brief Binding Table index for DN Update kernel
//!
#define BI_DN_STATISTICS_SURFACE                       0
#define BI_DN_VEBOX_STATE_SURFACE                      1
#define BI_DN_TEMP_SURFACE                             2
#define BI_DN_SPATIAL_ATTRIBUTES_CONFIGURATION_SURFACE 3


//!
//! \brief  Judgement for Vebox surface height alignment 
//!         1. _a should be an integal of power(2,n)
//!
#define IS_VEBOX_SURFACE_HEIGHT_UNALIGNED(_pSrcSurface, _a)                                         \
    ((MOS_MIN((uint32_t)_pSrcSurface->dwHeight, (uint32_t)_pSrcSurface->rcMaxSrc.bottom))             &   \
    (uint32_t)((_a) - 1))

typedef struct VPHAL_VEBOX_STATE_PARAMS *PVPHAL_VEBOX_STATE_PARAMS;
typedef struct VPHAL_VEBOX_STATE_PARAMS_EXT *PVPHAL_VEBOX_STATE_PARAMS_EXT;
struct VPHAL_VEBOX_STATE_PARAMS
{
                                    VPHAL_VEBOX_STATE_PARAMS()
                                    {
                                        pVphalVeboxIecpParams = nullptr;
                                        pVphalVeboxDndiParams = nullptr;
                                    }
    virtual                         ~VPHAL_VEBOX_STATE_PARAMS()
                                    {
                                        pVphalVeboxIecpParams = nullptr;
                                        pVphalVeboxDndiParams = nullptr;
                                    }
    virtual void                    Init()
                                    {
                                        pVphalVeboxIecpParams = nullptr;
                                        pVphalVeboxDndiParams = nullptr;
                                    }
    virtual PVPHAL_VEBOX_STATE_PARAMS_EXT   GetExtParams()  {return nullptr;}

    PMHW_VEBOX_DNDI_PARAMS          pVphalVeboxDndiParams;
    PVPHAL_VEBOX_IECP_PARAMS        pVphalVeboxIecpParams;
};

//!
//! \brief  Chroma Denoise params
//!
typedef struct _VPHAL_DNUV_PARAMS
{
    uint32_t    dwHistoryInitUV;
    uint32_t    dwHistoryDeltaUV;
    uint32_t    dwHistoryMaxUV;
    uint32_t    dwSTADThresholdU;
    uint32_t    dwSTADThresholdV;
    uint32_t    dwLTDThresholdU;
    uint32_t    dwLTDThresholdV;
    uint32_t    dwTDThresholdU;
    uint32_t    dwTDThresholdV;
} VPHAL_DNUV_PARAMS, *PVPHAL_DNUV_PARAMS;

typedef struct _VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS
{
    PVPHAL_SURFACE                  pSurfInput;
    PVPHAL_SURFACE                  pSurfOutput;
    PVPHAL_SURFACE                  pSurfSTMM;
    PVPHAL_SURFACE                  pSurfDNOutput;
    PVPHAL_SURFACE                  pSurfSkinScoreOutput;
    bool                            bDIEnable;
} VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS, *PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS;

//!
//! \brief  Structure to handle DNDI sampler states
//!
typedef struct _VPHAL_SAMPLER_STATE_DNDI_PARAM
{
    uint32_t  dwDenoiseASDThreshold;
    uint32_t  dwDenoiseHistoryDelta;
    uint32_t  dwDenoiseMaximumHistory;
    uint32_t  dwDenoiseSTADThreshold;
    uint32_t  dwDenoiseSCMThreshold;
    uint32_t  dwDenoiseMPThreshold;
    uint32_t  dwLTDThreshold;
    uint32_t  dwTDThreshold;
    uint32_t  dwGoodNeighborThreshold;
    bool      bDNEnable;
    bool      bDIEnable;
    bool      bDNDITopFirst;
    bool      bProgressiveDN;
    uint32_t  dwFMDFirstFieldCurrFrame;
    uint32_t  dwFMDSecondFieldPrevFrame;
} VPHAL_SAMPLER_STATE_DNDI_PARAM, *PVPHAL_SAMPLER_STATE_DNDI_PARAM;

//!
//! \brief Transient Render data populated for every BLT call
//!
typedef struct VPHAL_VEBOX_RENDER_DATA *PVPHAL_VEBOX_RENDER_DATA;
typedef struct VPHAL_VEBOX_RENDER_DATA_EXT *PVPHAL_VEBOX_RENDER_DATA_EXT;
struct VPHAL_VEBOX_RENDER_DATA
{
public:
                                        VPHAL_VEBOX_RENDER_DATA()
                                        {
                                            // Flags
                                            bRefValid      = false;
                                            bSameSamples   = false;
                                            bProgressive   = false;
                                            bDenoise       = false;
#if VEBOX_AUTO_DENOISE_SUPPORTED
                                            bAutoDenoise   = false;
#endif
                                            bChromaDenoise = false;
                                            bOutOfBound    = false;
                                            bVDIWalker     = false;
                                            bIECP          = false;
                                            bColorPipe     = false;
                                            bProcamp       = false;
                                            // DNDI/Vebox
                                            bDeinterlace   = false;
                                            bSingleField   = false;
                                            bTFF           = false;
                                            bTopField      = false;
                                            bBeCsc         = false;
                                            bVeboxBypass   = false;
                                            b60fpsDi       = false;
                                            bQueryVariance = false;
                                            // Surface Information
                                            iFrame0        = 0;
                                            iFrame1        = 0;
                                            iCurDNIn       = 0;
                                            iCurDNOut      = 0;
                                            iCurHistIn     = 0;
                                            iCurHistOut    = 0;
                                            // Geometry
                                            iBlocksX       = 0;
                                            iBlocksY       = 0;
                                            iBindingTable  = 0;
                                            iMediaID0      = 0;
                                            iMediaID1      = 0;
                                            // Perf
                                            PerfTag        = VPHAL_NONE;
                                            // States
                                            pMediaState        = nullptr;
                                            pVeboxState        = nullptr;
                                            pRenderTarget      = nullptr;
                                            SamplerStateParams = { };
                                            VeboxDNDIParams    = { };
                                            pAlphaParams       = nullptr;
                                            // Batch Buffer rendering arguments
                                            BbArgs = { };
                                            // Vebox output parameters
                                            OutputPipe = VPHAL_OUTPUT_PIPE_MODE_COMP;
                                            // Kernel Information
                                            for (int i = 0; i < VPHAL_NUM_KERNEL_VEBOX; i++)
                                            {
                                                pKernelParam[i] = nullptr;
                                                KernelEntry[i]  = { };
                                            }
                                            pDNUVParams          = nullptr;
                                            iCurbeLength         = 0;
                                            iCurbeOffset         = 0;
                                            iInlineLength        = 0;
                                            // Debug parameters
                                            pKernelName          = nullptr;
                                            Component            = COMPONENT_UNKNOWN;
                                            // Memory compression flag
                                            bEnableMMC           = false;

                                            pOutputTempField     = nullptr;

                                            fScaleX              = 0.0f;
                                            fScaleY              = 0.0f;

                                            bHdr3DLut            = false;
                                            uiMaxDisplayLum      = 4000;
                                            uiMaxContentLevelLum = 1000;
                                            hdrMode              = VPHAL_HDR_MODE_NONE;

                                            m_pVeboxStateParams  = nullptr;
                                            m_pVeboxIecpParams   = nullptr;
                                        }
                                        VPHAL_VEBOX_RENDER_DATA(const VPHAL_VEBOX_RENDER_DATA&) = delete;
                                        VPHAL_VEBOX_RENDER_DATA& operator=(const VPHAL_VEBOX_RENDER_DATA&) = delete;
    virtual                             ~VPHAL_VEBOX_RENDER_DATA();
    virtual MOS_STATUS                  Init();
    PVPHAL_VEBOX_STATE_PARAMS           GetVeboxStateParams()   { return m_pVeboxStateParams;}
    PVPHAL_VEBOX_IECP_PARAMS            GetVeboxIECPParams()    { return m_pVeboxIecpParams; }
    virtual PVPHAL_VEBOX_RENDER_DATA_EXT GetExtData()           { return nullptr;}

    // Flags
    bool                                bRefValid;
    bool                                bSameSamples;
    bool                                bProgressive;
    bool                                bDenoise;
#if VEBOX_AUTO_DENOISE_SUPPORTED
    bool                                bAutoDenoise;
#endif
    bool                                bChromaDenoise;
    bool                                bOutOfBound;
    bool                                bVDIWalker;

    bool                                bIECP;
    bool                                bColorPipe;
    bool                                bProcamp;

    // DNDI/Vebox
    bool                                bDeinterlace;
    bool                                bSingleField;
    bool                                bTFF;
    bool                                bTopField;
    bool                                bBeCsc;
    bool                                bVeboxBypass;
    bool                                b60fpsDi;
    bool                                bQueryVariance;

    // Surface Information
    int32_t                             iFrame0;
    int32_t                             iFrame1;
    int32_t                             iCurDNIn;
    int32_t                             iCurDNOut;
    int32_t                             iCurHistIn;
    int32_t                             iCurHistOut;

    // Geometry
    int32_t                             iBlocksX;
    int32_t                             iBlocksY;
    int32_t                             iBindingTable;
    int32_t                             iMediaID0;
    int32_t                             iMediaID1;

    // Perf
    VPHAL_PERFTAG                       PerfTag;

    // States
    PRENDERHAL_MEDIA_STATE              pMediaState;
    PMHW_VEBOX_HEAP_STATE               pVeboxState;
    PVPHAL_SURFACE                      pRenderTarget;

    MHW_SAMPLER_STATE_PARAM             SamplerStateParams;

    MHW_VEBOX_DNDI_PARAMS               VeboxDNDIParams;

    PVPHAL_ALPHA_PARAMS                 pAlphaParams;

    // Batch Buffer rendering arguments
    VPHAL_ADVPROC_BB_ARGS               BbArgs;

    // Vebox output parameters
    VPHAL_OUTPUT_PIPE_MODE              OutputPipe;

    // Kernel Information
    PRENDERHAL_KERNEL_PARAM             pKernelParam[VPHAL_NUM_KERNEL_VEBOX];
    Kdll_CacheEntry                     KernelEntry[VPHAL_NUM_KERNEL_VEBOX];
    PVPHAL_DNUV_PARAMS                  pDNUVParams;
    int32_t                             iCurbeLength;
    int32_t                             iCurbeOffset;
    int32_t                             iInlineLength;

    // Debug parameters
    char*                               pKernelName;                            //!< Kernel used for current rendering

    // Current component
    MOS_COMPONENT                       Component;

    // Memory compression flag
    bool                                bEnableMMC;                             //!< Enable memory compression flag

    // Temp surface for the field won't be output
    PVPHAL_SURFACE                      pOutputTempField;

    // Scaling ratio from source to render target
    // Scaling ratio is needed to determine if SFC or VEBOX is used
    float                               fScaleX;                                //!< X Scaling ratio
    float                               fScaleY;                                //!< Y Scaling ratio

    bool                                bHdr3DLut;                              //!< Enable 3DLut to process HDR
    uint32_t                            uiMaxDisplayLum;                        //!< Maximum Display Luminance
    uint32_t                            uiMaxContentLevelLum;                   //!< Maximum Content Level Luminance
    VPHAL_HDR_MODE                      hdrMode;

protected:
    // Vebox State Parameters
    PVPHAL_VEBOX_STATE_PARAMS           m_pVeboxStateParams;                    //!< auto allocated param instance for set/submit VEBOX cmd
    // Vebox IECP Parameters
    PVPHAL_VEBOX_IECP_PARAMS            m_pVeboxIecpParams;                     //!< auto allocated param instance for set/submit VEBOX IECP cmd
};

//!
//! \brief VPHAL VEBOX/IECP State
//!
typedef class VPHAL_VEBOX_STATE *PVPHAL_VEBOX_STATE;
class VPHAL_VEBOX_STATE : public RenderState
{
public:
                                        VPHAL_VEBOX_STATE(
                                            PMOS_INTERFACE                  pOsInterface,
                                            PMHW_VEBOX_INTERFACE            pVeboxInterface,
                                            PMHW_SFC_INTERFACE              pSfcInterface,
                                            PRENDERHAL_INTERFACE            pRenderHal,
                                            PVPHAL_VEBOX_EXEC_STATE         pVeboxExecState,
                                            PVPHAL_RNDR_PERF_DATA           pPerfData,
                                            const VPHAL_DNDI_CACHE_CNTL     &dndiCacheCntl,
                                            MOS_STATUS                      *peStatus);
                                        VPHAL_VEBOX_STATE(const VPHAL_VEBOX_STATE&) = delete;
                                        VPHAL_VEBOX_STATE& operator=(const VPHAL_VEBOX_STATE&) = delete;
    virtual                             ~VPHAL_VEBOX_STATE();

    virtual MOS_STATUS                  AllocateExecRenderData()
    {
        if (!m_pLastExecRenderData)
        {
            m_pLastExecRenderData = MOS_New(VPHAL_VEBOX_RENDER_DATA);
            if (!m_pLastExecRenderData)
            {
                return MOS_STATUS_NO_SPACE;
            }
            m_pLastExecRenderData->Init();
        }
        return MOS_STATUS_SUCCESS;
    }
    virtual PVPHAL_VEBOX_RENDER_DATA    GetLastExecRenderData()     { if (!m_pLastExecRenderData) { AllocateExecRenderData(); } return m_pLastExecRenderData; }

    //!
    //! \brief    copy Report data
    //! \details  copy Report data from this render
    //! \param    [out] pReporting 
    //!           pointer to the Report data to copy data to
    //!
    void CopyReporting(VphalFeatureReport *pReporting);

    //!
    //! \brief    copy Report data about features
    //! \details  copy Report data from this render
    //! \param    [out] pReporting 
    //!           pointer to the Report data to copy data to
    //!
    void CopyFeatureReporting(VphalFeatureReport *pReporting);

    //!
    //! \brief    copy Report data about resources
    //! \details  copy Report data from this render
    //! \param    [out] pReporting 
    //!           pointer to the Report data to copy data to
    //!
    void CopyResourceReporting(VphalFeatureReport *pReporting);

    //!
    //! \brief    Allocate sfc temp surface for Vebox output
    //! \details  Allocate sfc temp surface for Vebox output
    //! \param    VphalRenderer* pRenderer
    //!           [in,out] VPHAL renderer pointer
    //! \param    PCVPHAL_RENDER_PARAMS pcRenderParams
    //!           [in] Const pointer to VPHAL render parameter
    //! \param    PVPHAL_VEBOX_RENDER_DATA pRenderData
    //!           [in] pointer to VPHAL VEBOX render parameter
    //! \param    PVPHAL_SURFACE pInSurface
    //!           [in] Pointer to input surface
    //! \param    PVPHAL_SURFACE pOutSurface
    //!           [in] Pointer to output surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AllocateSfcTempSurfaces(
        VphalRenderer                  *pRenderer,
        PCVPHAL_RENDER_PARAMS           pcRenderParams,
        PVPHAL_VEBOX_RENDER_DATA        pRenderData,
        PVPHAL_SURFACE                  pInSurface,
        PVPHAL_SURFACE                  pOutSurface);

    //!
    //! \brief    Allocate sfc 2pass 2nd time surface for Vebox output, only for multi-lays.
    //! \details  Allocate sfc temp surface for Vebox output
    //! \param    VphalRenderer* pRenderer
    //!           [in,out] VPHAL renderer pointer
    //! \param    PCVPHAL_RENDER_PARAMS pcRenderParams
    //!           [in] Const pointer to VPHAL render parameter
    //! \param    PVPHAL_VEBOX_RENDER_DATA pRenderData
    //!           [in] pointer to VPHAL VEBOX render parameter
    //! \param    PVPHAL_SURFACE pInSurface
    //!           [in] Pointer to input surface
    //! \param    PVPHAL_SURFACE pOutSurface
    //!           [in] Pointer to output surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AllocateSfc2ndTempSurfaces(
      VphalRenderer                  *pRenderer,
      PCVPHAL_RENDER_PARAMS           pcRenderParams,
      PVPHAL_VEBOX_RENDER_DATA        pRenderData,
      PVPHAL_SURFACE                  pInSurface,
      PVPHAL_SURFACE                  pOutSurface);

    // External components
    PMHW_VEBOX_INTERFACE            m_pVeboxInterface;                            //!< Pointer to MHW Vebox Structure Interface
    PMHW_SFC_INTERFACE              m_pSfcInterface;                              //!< Pointer to SFC Structure Interface
    Kdll_State                      *m_pKernelDllState;                           //!< Kernel DLL state
    VpKernelID                      m_currKernelId;                               //!< Kernel ID

    VphalSfcState                   *m_sfcPipeState;                              //!< SFC state

    // Execution state
    PVPHAL_VEBOX_RENDER_DATA        m_pLastExecRenderData;                        //!< Cache last render operation info
    PVPHAL_VEBOX_EXEC_STATE         m_pVeboxExecState;                            //!< Vebox Execution State

    // CSC in VEBOX params
    VPHAL_CSPACE                    CscOutputCspace;                            //!< Cspace of Output Frame
    VPHAL_CSPACE                    CscInputCspace;                             //!< Cspace of Input frame
    float                           fCscCoeff[9];                               //!< [3x3] Coeff matrix for CSC
    float                           fCscInOffset[3];                            //!< [3x1] Input Offset matrix for CSC
    float                           fCscOutOffset[3];                           //!< [3x1] Output Offset matrix for CSC

    // Dynamic linking filter
    Kdll_FilterEntry                SearchFilter[2];

    // Threshold for discontinuity check
    int32_t                         iSameSampleThreshold;

    // Resources
    VPHAL_SURFACE                   *m_currentSurface;                          //!< Current frame
    VPHAL_SURFACE                   *m_previousSurface;                         //!< Previous frame
    RENDERHAL_SURFACE               RenderHalCurrentSurface;                    //!< Current frame for MHW
    RENDERHAL_SURFACE               RenderHalPreviousSurface;                   //!< Previous frame for MHW

    union
    {
        //  DNDI/VEBOX
        struct
        {
            VPHAL_SURFACE           *FFDISurfaces[VPHAL_MAX_NUM_FFDI_SURFACES];  //!< FFDI output surface structure
        };
    };
    VPHAL_SURFACE                   VeboxRGBHistogram = {};            //!< VEBOX RGB Histogram surface for Vebox Gen9+
    VPHAL_SURFACE                   VeboxStatisticsSurface;                     //!< Statistics Surface for VEBOX
    RENDERHAL_SURFACE               RenderHalVeboxStatisticsSurface;            //!< Statistics Surface for VEBOX for MHW
#if VEBOX_AUTO_DENOISE_SUPPORTED
    VPHAL_SURFACE                   VeboxTempSurface;                           //!< Temp Surface for Vebox State update kernels
    VPHAL_SURFACE                   VeboxSpatialAttributesConfigurationSurface; //!< Spatial Attributes Configuration Surface for DN kernel Gen9+
    RENDERHAL_SURFACE               RenderHalVeboxSpatialAttributesConfigurationSurface; //!< Spatial Attributes Configuration Surface for DN kernel Gen9+ for MHW
    VPHAL_SURFACE                   VeboxHeapResource;                          //!< Vebox Heap resource for DN kernel
    VPHAL_SURFACE                   tmpResource;                                //!< Temp resource for DN kernel
    RENDERHAL_SURFACE               RenderHalVeboxHeapResource;                 //!< Vebox Heap resource for DN kernel for MHW
    RENDERHAL_SURFACE               RenderHalTmpResource;                       //!< Temp resource for DN kernel for MHW
#endif
    union
    {
        // DNDI
        struct
        {
            VPHAL_SURFACE           *FFDNSurfaces[VPHAL_NUM_FFDN_SURFACES];     //!< Denoise output surfaces
            VPHAL_SURFACE           STMMSurfaces[VPHAL_NUM_STMM_SURFACES];      //!< Motion history (DI)
        };
    };

    // BNE system memory pointer
    uint8_t*                        pBNEData;                                   //!< System memory for GNE calculating
    uint32_t                        dwBNESize;                                  //!< System memory size for BNE surface

    // Statistics
    uint32_t                        dwVeboxPerBlockStatisticsWidth;             //!< Per block statistics width
    uint32_t                        dwVeboxPerBlockStatisticsHeight;            //!< Per block statistics height

    // Cache attributes
    VPHAL_DNDI_CACHE_CNTL           DnDiSurfMemObjCtl;                          //!< Surface memory object control

    // Batch Buffers
    int32_t                         iBatchBufferCount;                          //!< Number of batch buffers
    MHW_BATCH_BUFFER                BatchBuffer[VPHAL_DNDI_BUFFERS_MAX];        //!< Batch buffers
    VPHAL_BATCH_BUFFER_PARAMS       BufferParam[VPHAL_DNDI_BUFFERS_MAX];        //!< Batch buffer parameters

    // Denoise output control
    int32_t                         iCurDNIndex;                                //!< Current index of Denoise Output

    // DNDI
    struct
    {
        int32_t                     iNumFFDISurfaces;                           //!< Actual number of FFDISurfaces. Is <= VPHAL_NUM_FFDI_SURFACES
        int32_t                     iCurStmmIndex;                              //!< Current index of Motion History Buffer
        uint32_t                    dwGlobalNoiseLevel;                         //!< Global Noise Level
    };

    // Chroma DN
    struct
    {
        int32_t                     iCurHistIndex;                              //!< Current index of Chroma Denoise History Buffer
        uint32_t                    dwGlobalNoiseLevelU;                        //!< Global Noise Level for U
        uint32_t                    dwGlobalNoiseLevelV;                        //!< Global Noise Level for V
        bool                        bFirstFrame;                                //!< First frame case for Chroma DN
    };

    // timestamps for DI output control
    int32_t                         iCurFrameID;                                //!< Current Frame ID
    int32_t                         iPrvFrameID;                                //!< Previous Frame ID

    // for Pre-Processing
    bool                            bSameSamples;                               //!< True for second DI
    int32_t                         iCallID;                                    //!< Current render call ID;
    bool                            bDNEnabled;                                 //!< DN was enabled in the previous call
    bool                            bDIEnabled;                                 //!< DI was enabled in the previous call

    // Platform dependent states
    PRENDERHAL_KERNEL_PARAM         pKernelParamTable;                          //!< Kernel Parameter table

    // HW Params
    uint32_t                        dwKernelUpdate;                             //!< Enable/Disable kernel update

    uint32_t                        dwCompBypassMode;                           //!< Bypass Composition Optimization read from User feature keys

    // Debug parameters
    char*                           pKernelName;                                //!< Kernel Used for current rendering
    bool                            bNullHwRenderDnDi;                          //!< Null rendering for DnDi function

    bool                            bEnableMMC;                                 //!< Memory compression enbale flag - read from User feature keys
    bool                            bDisableTemporalDenoiseFilter;              //!< Temporal denoise filter disable flag - read from User feature keys
    bool                            bDisableTemporalDenoiseFilterUserKey;       //!< Backup temporal denoise filter disable flag - read from User feature keys

    // S3D channel
    uint32_t                         uiCurrentChannel;                           //!< 0=StereoLeft or nonStereo, 1=StereoRight. N/A in nonStereo

    MOS_GPU_CONTEXT                  RenderGpuContext;                           //!< Render GPU context

    VPHAL_SURFACE                    Vebox3DLookUpTables = {};
    VPHAL_SURFACE                    SfcTempSurface = {};
    VPHAL_SURFACE                    Sfc2ndTempSurface = {};

    VphalHVSDenoiser                 *m_hvsDenoiser;                             //!< Human Vision System Based Denoiser - Media Kernel to generate DN parameter
    uint8_t                          *m_hvsKernelBinary;                         //!< Human Vision System Based Denoiser - Pointer to HVS kernel Binary
    uint32_t                         m_hvsKernelBinarySize;                      //!< Human Vision System Based Denoiser - Size of HVS kernel Binary

    bool                             bPhasedSubmission;                          //!< Flag to indicate if secondary command buffers are submitted together (Win) or separately (Linux)

protected:
    PVPHAL_VEBOX_IECP_RENDERER      m_IECP;                                     //!< pointer to IECP Renderer module, which contains more filters like TCC, STE.

public:
    //!
    //! \brief    Initialize VEBOX state
    //! \param    [in] pSettings
    //!           Pointer to VPHAL settings
    //! \param    [in] pKernelDllState
    //!           Pointer to KDLL state
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize(
        const VphalSettings         *pSettings,
        Kdll_State                  *pKernelDllState);

    virtual void Destroy();

    //!
    //! \brief    RenderState Rendering
    //! \details  VpHal RenderState entry
    //! \param    [in] pcRenderParams
    //!           Pointer to Render parameters
    //! \param    [in,out] pRenderPassData
    //!           Pointer to Render data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Render(
        PCVPHAL_RENDER_PARAMS  pcRenderParams,
        RenderpassData         *pRenderPassData);

    //!
    //! \brief    Set DI output frame
    //! \details  Choose 2nd Field of Previous frame or 1st Field of Current frame
    //!           or both frames
    //! \param    [in] pRenderData
    //!           Pointer to Render data
    //! \param    [in] pVeboxState
    //!           Pointer to Vebox State
    //! \param    [in] pVeboxMode
    //!           Pointer to Vebox Mode
    //! \return   GFX_MEDIA_VEBOX_DI_OUTPUT_MODE
    //!           Return Previous/Current/Both frames
    //!
    virtual GFX_MEDIA_VEBOX_DI_OUTPUT_MODE SetDIOutputFrame(
        PVPHAL_VEBOX_RENDER_DATA pRenderData,
        PVPHAL_VEBOX_STATE       pVeboxState,
        PMHW_VEBOX_MODE          pVeboxMode);

    //!
    //! \brief    Judge if render is needed
    //! \details  Check Render parameter/data if render needed
    //! \param    [in] pcRenderParams
    //!           Pointer to Render parameters
    //! \param    [in,out] pRenderPassData
    //!           Pointer to Render data
    //! \return   bool
    //!           true if meeded. Else false
    //!
    virtual bool IsNeeded(
        PCVPHAL_RENDER_PARAMS  pcRenderParams,
        RenderpassData         *pRenderPassData) = 0;

    //!
    //! \brief    Judge if render support multiple stream rendering
    //! \details  Judge if render support multiple stream rendering
    //! \return   bool
    //!           true if supported. Else false
    //!
    virtual bool IsMultipleStreamSupported()
    {
        // VEBOX render is for single layer only.
        return false;
    }

    virtual MOS_STATUS AllocateResources() = 0;

    virtual void FreeResources() = 0;

    //!
    //! \brief    Setup kernels for Vebox auto mode features
    //! \details  Setup kernels that co-operate with Vebox auto mode features
    //! \param    [in] iKDTIndex
    //!           Index to Kernel Parameter Array (defined platform specific)
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupVeboxKernel(
        int32_t                     iKDTIndex) = 0;

    virtual MOS_STATUS SetupDiIecpState(
        bool                        bDiScdEnable,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS
                                    pVeboxDiIecpCmdParams) = 0;

    virtual void SetupSurfaceStates(
        bool                        bDiVarianceEnable,
        PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS
                                    pVeboxSurfaceStateCmdParams) = 0;

    virtual MOS_STATUS VeboxQueryStatLayout(
        VEBOX_STAT_QUERY_TYPE           QueryType,
        uint32_t*                       pQuery) = 0;
    //!
    //! \brief    Update RenderGpuContext
    //! \details  Update RenderGpuContext
    //! \param    [in] renderGpuContext
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS UpdateRenderGpuContext(
        MOS_GPU_CONTEXT renderGpuContext);

#if VEBOX_AUTO_DENOISE_SUPPORTED
    //!
    //! \brief    Load update kernel curbe data
    //! \details  Loads the static data of update kernel to curbe
    //! \param    [out] iCurbeOffsetOutDN
    //!           Pointer to DN kernel curbe offset
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS LoadUpdateDenoiseKernelStaticData(
        int32_t*                        iCurbeOffsetOutDN) = 0;

    //!
    //! \brief    Setup surface states for Denoise
    //! \details  Setup Surface State for Vebox States Auto DN kernel
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupSurfaceStatesForDenoise() = 0;
#endif

    virtual MOS_STATUS PostCompRender(
        PVPHAL_VEBOX_EXEC_STATE         pVeboxExecState,
        PVPHAL_SURFACE                  pPriSurface);

    virtual bool IsFormatSupported(
        PVPHAL_SURFACE              pSrcSurface) = 0;

    virtual bool IsRTFormatSupported(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pRTSurface) = 0;

    virtual bool IsDnFormatSupported(
        PVPHAL_SURFACE                  pSrcSurface) = 0;

    virtual bool IsDiFormatSupported(
        PVPHAL_SURFACE              pSrcSurface) = 0;

    virtual bool UseKernelResource()=0;

    virtual void VeboxGetBeCSCMatrix(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pOutSurface) = 0;

    //!
    //! \brief    Check if the BeCSCMatrx need to be calculated
    //! \param    [in] pSrcSurface
    //!           Pointer to source surface
    //! \param    [in] pOutSurface
    //!           Pointer to output surface
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual bool VeboxIsBeCSCMatrixNeeded(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pOutSurface)
    {
        if (!pSrcSurface || !pOutSurface)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Null surface pointer");
            return false;
        }

        if ((CscInputCspace  != pSrcSurface->ColorSpace) ||
            (CscOutputCspace != pOutSurface->ColorSpace))
        {
            return true;
        }
        else
        {
            return false;

        }
    }

    virtual bool IsIECPEnabled()
    {
        return GetLastExecRenderData()->bIECP;
    }
    virtual bool IsQueryVarianceEnabled() {return false;}

    virtual MOS_STATUS VeboxSetPerfTag(MOS_FORMAT               srcFmt);
    virtual void VeboxClearFmdStates() { }

protected:
    //!
    //! \brief    Vebox set up vebox state heap
    //! \details  Setup Vebox indirect states: DNDI and etc
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in] pOutSurface
    //!           Pointer to output surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSetupIndirectStates(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pOutSurface);

    //!
    //! \brief    Vebox Set VEBOX parameter
    //! \details  Set up the VEBOX parameter value
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSetDNDIParams(
        PVPHAL_SURFACE              pSrcSurface);

    //!
    //! \brief    Vebox Set FMD parameter
    //! \details  Set up the FMD parameters for DNDI State
    //! \param    [out] pLumaParams
    //!           Pointer to DNDI Param for set FMD parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSetFMDParams(
        PVPHAL_SAMPLER_STATE_DNDI_PARAM     pLumaParams);

    //!
    //! \brief    Vebox Populate DNDI parameters
    //! \details  Populate the VEBOX state DNDI parameters to VEBOX RenderData
    //! \param    [in] pLumaParams
    //!           Pointer to Luma DN and DI parameter
    //! \param    [in] pChromaParams
    //!           Pointer to Chroma DN parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxPopulateDNDIParams(
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
        PVPHAL_DNUV_PARAMS              pChromaParams);

    //!
    //! \brief    Flush command buffer for update kernels
    //! \details  Flush the command buffer for Update kernels
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxFlushUpdateStateCmdBuffer();

    //!
    //! \brief    Copy Vebox state heap
    //! \details  Call HW interface function,
    //!           use Secure_Block_Copy kernel,
    //!           copy Vebox state heap between different memory
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxCopyVeboxStates();

    //!
    //! \brief    Copy and update vebox state
    //! \details  Copy and update vebox state for input frame.
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxCopyAndUpdateVeboxState(
        PVPHAL_SURFACE           pSrcSurface);

    //!
    //! \brief    Vebox state heap update for auto mode features
    //! \details  Update Vebox indirect states for auto mode features
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxUpdateVeboxStates(
        PVPHAL_SURFACE              pSrcSurface);

    //!
    //! \brief    Doing prepare stage tasks for VeboxSendVeboxCmd
    //!           Parameters might remain unchanged in case
    //! \param    [out] CmdBuffer
    //!           reference to Cmd buffer control struct
    //! \param    [out] GenericPrologParams
    //!           Generic prolog params struct to be set
    //! \param    [out] GpuStatusBuffer
    //!           GpuStatusBuffer resource to be set
    //! \param    [out] iRemaining
    //!           integer showing initial cmd buffer usage
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSendVeboxCmd_Prepare(
        MOS_COMMAND_BUFFER                      &CmdBuffer,
        RENDERHAL_GENERIC_PROLOG_PARAMS         &GenericPrologParams,
        MOS_RESOURCE                            &GpuStatusBuffer,
        int32_t                                 &iRemaining);

    //!
    //! \brief    Check whether the Vebox command parameters are correct
    //! \param    [in] VeboxStateCmdParams
    //!           MHW vebox state cmd params
    //! \param    [in] VeboxDiIecpCmdParams
    //!           DiIecpCmd params struct
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxIsCmdParamsValid(
        const MHW_VEBOX_STATE_CMD_PARAMS        &VeboxStateCmdParams,
        const MHW_VEBOX_DI_IECP_CMD_PARAMS      &VeboxDiIecpCmdParams);

    //!
    //! \brief    Render the Vebox Cmd buffer for VeboxSendVeboxCmd
    //!           Parameters might remain unchanged in case
    //! \param    [in,out] CmdBuffer
    //!           reference to Cmd buffer control struct
    //! \param    [out] VeboxDiIecpCmdParams
    //!           DiIecpCmd params struct to be set
    //! \param    [out] VeboxSurfaceStateCmdParams
    //!           VPHAL surface state cmd to be set
    //! \param    [out] MhwVeboxSurfaceStateCmdParams
    //!           MHW surface state cmd to be set
    //! \param    [out] VeboxStateCmdParams
    //!           MHW vebox state cmd to be set
    //! \param    [out] FlushDwParams
    //!           MHW MI_FLUSH_DW cmd to be set
    //! \param    [in] pGenericPrologParams
    //!           pointer to Generic prolog params struct to send to cmd buffer header
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxRenderVeboxCmd(
        MOS_COMMAND_BUFFER                      &CmdBuffer,
        MHW_VEBOX_DI_IECP_CMD_PARAMS            &VeboxDiIecpCmdParams,
        VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
        MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
        MHW_VEBOX_STATE_CMD_PARAMS              &VeboxStateCmdParams,
        MHW_MI_FLUSH_DW_PARAMS                  &FlushDwParams,
        PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams);

    //!
    //! \brief    Render pipe cmds to Vebox Cmd buffer for VeboxSendVeboxCmd
    //! \details  Render pipe cmds to Vebox Cmd buffer for VeboxSendVeboxCmd
    //!           Parameters might remain unchanged in case
    //! \param    pVeboxInterface
    //!           [in] Pointers to Vebox Interface
    //! \param    pMhwMiInterface
    //!           [in] Pointers to MI HW Interface
    //! \param    pVeboxSurfaceParams
    //!           [in] Pointers to Vebox Surface Params Interface
    //! \param    pVeboxDiIecpCmdParams
    //!           [in] Pointers to DI/IECP CMD Params Interface
    //! \param    pCmdBuffer
    //!           [in,out] pointer to CMD buffer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxRenderMMCPipeCmd(
        PMHW_VEBOX_INTERFACE                pVeboxInterface,
        MhwMiInterface *                    pMhwMiInterface,
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pVeboxSurfaceParams,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS       pVeboxDiIecpCmdParams,
        PMOS_COMMAND_BUFFER                 pCmdBuffer)
    {
        MOS_UNUSED(*pCmdBuffer);
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Vebox send Vebox ring HW commands
    //! \details  Send Vebox ring Commands.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSendVeboxCmd();

    //!
    //! \brief    Sync for Indirect state Copy and Update Kernels
    //! \details  Sync for Indirect state Copy and Update Kernels before Send Vebox ring Commands.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSyncIndirectStateCmd();

    //!
    //! \brief    Set extra parameters before submit Vebox Cmd
    //! \details  Set extra parameters before submit Vebox Cmd
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSendVeboxCmdSetParamBeforeSubmit()
    {       return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Vebox set common rendering flag
    //! \details  Common flags should be set before other flags,
    //!           and it should be independent with other flags
    //! \param    [in] pSrc
    //!           Pointer to input surface of Vebox
    //! \param    [in] pRenderTarget
    //!           Pointer to Render targe surface of VPP BLT
    //! \return   void
    //!
    virtual void VeboxSetCommonRenderingFlags(
        PVPHAL_SURFACE              pSrc,
        PVPHAL_SURFACE              pRenderTarget);

    //!
    //! \brief    Vebox set Field related rendering flag
    //! \details  Set Field related flags for interlaced or related features
    //! \param    [in] pSrc
    //!           Pointer to input surface of Vebox
    //! \return   void
    //!
    virtual void VeboxSetFieldRenderingFlags(
        PVPHAL_SURFACE              pSrc);

    //!
    //! \brief    Vebox set rendering flag
    //! \details  Setup Rendering Flags due to different usage case - main entrance
    //! \param    [in] pSrc
    //!           Pointer to input surface of Vebox
    //! \param    [in] pRenderTarget
    //!           Pointer to Render targe surface of VPP BLT
    //! \return   void
    //!
    virtual void VeboxSetRenderingFlags(
        PVPHAL_SURFACE              pSrc,
        PVPHAL_SURFACE              pRenderTarget);

    //!
    //! \brief    Copy Dndi Surface Params
    //! \details  Copies surface params to output surface
    //!           based on src and temp surfaces
    //! \param    [in] pSrcSurface
    //!           Pointer to Source Surface
    //! \param    [in] pTempSurface
    //!           Pointer to Temporary Surface
    //! \param    [in,out] pOutSurface
    //!           Pointer to Out Surface
    //! \return   void
    //!
    virtual void VeboxCopySurfaceParams(
        const PVPHAL_SURFACE            pSrcSurface,
        const PVPHAL_SURFACE            pTempSurface,
        PVPHAL_SURFACE                  pOutSurface);

    //!
    //! \brief    Vebox initialize STMM History
    //! \details  Initialize STMM History surface
    //! Description:
    //!   This function is used by VEBox for initializing
    //!   the STMM surface.  The STMM / Denoise history is a custom surface used 
    //!   for both input and output. Each cache line contains data for 4 4x4s. 
    //!   The STMM for each 4x4 is 8 bytes, while the denoise history is 1 byte 
    //!   and the chroma denoise history is 1 byte for each U and V.
    //!   Byte    Data\n
    //!   0       STMM for 2 luma values at luma Y=0, X=0 to 1\n
    //!   1       STMM for 2 luma values at luma Y=0, X=2 to 3\n
    //!   2       Luma Denoise History for 4x4 at 0,0\n
    //!   3       Not Used\n
    //!   4-5     STMM for luma from X=4 to 7\n
    //!   6       Luma Denoise History for 4x4 at 0,4\n
    //!   7       Not Used\n
    //!   8-15    Repeat for 4x4s at 0,8 and 0,12\n
    //!   16      STMM for 2 luma values at luma Y=1,X=0 to 1\n
    //!   17      STMM for 2 luma values at luma Y=1, X=2 to 3\n
    //!   18      U Chroma Denoise History\n
    //!   19      Not Used\n
    //!   20-31   Repeat for 3 4x4s at 1,4, 1,8 and 1,12\n
    //!   32      STMM for 2 luma values at luma Y=2,X=0 to 1\n
    //!   33      STMM for 2 luma values at luma Y=2, X=2 to 3\n
    //!   34      V Chroma Denoise History\n
    //!   35      Not Used\n
    //!   36-47   Repeat for 3 4x4s at 2,4, 2,8 and 2,12\n
    //!   48      STMM for 2 luma values at luma Y=3,X=0 to 1\n
    //!   49      STMM for 2 luma values at luma Y=3, X=2 to 3\n
    //!   50-51   Not Used\n
    //!   36-47   Repeat for 3 4x4s at 3,4, 3,8 and 3,12\n
    //! \param    [in] iSurfaceIndex
    //!           Index of STMM surface array
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS VeboxInitSTMMHistory(
        int32_t                         iSurfaceIndex);

#if VEBOX_AUTO_DENOISE_SUPPORTED
    //!
    //! \brief    Vebox initialize Spatial Configuration Surface
    //! \details  Initialize Spatial Configuration History surface
    //! Description:
    //!   This function is used by VEBox for initializing
    //!   the Spatial Attributes Configuration surface. 
    //!   The GEN9+ DN kernel will use the init data in this surface and write back output data 
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS VeboxInitSpatialAttributesConfiguration();
#endif

    //!
    //! \brief    Update Vebox Execution State for Vebox/Render parallelism
    //! \details
    //!     Purpose   : Handle Vebox execution state machine transitions
    //!
    //!     Mode0:    Enter or stay in this state as long has (a) there are no future 
    //!               frames present or (b) FRC is active. Parallel execution is
    //!               handled different in FRC mode. (c) Vebox/SFC output path is
    //!               applied. Parallel execution is not needed when it is Vebox/SFC 
    //!               to output. Mode0 is considered the legacy serial vebox execution mode.
    //!     Mode0To2: Enter this state when a future frame becomes present. In this
    //!               state, perform a one time start up sequence in order to transistion
    //!               to Mode2 parallel execution state. 
    //!     Mode2:    Enter this state as long a future frame is present. This is the
    //!               steady parallel execution state where we process 1 frame ahead.
    //!               i.e. On BLT(N), we do vebox on the future frame N+1 and composite 
    //!               frame N in the same BLT(). 
    //!     Mode2To0: Enter this state when in Mode2 and no future frame is present. 
    //!               Transition back to Mode0.
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in] OutputPipe
    //!           The output path the driver uses to write the RenderTarget
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS UpdateVeboxExecutionState(
        PVPHAL_SURFACE          pSrcSurface,
        VPHAL_OUTPUT_PIPE_MODE  OutputPipe);

    //!
    //! \brief    Vebox render mode2
    //! \details  VEBOX/IECP Rendering for future frame
    //!           [Flow] 1. For future frame; send cmd.
    //!                  2. setup state for next vebox operation.
    //!                  3. Request "speculative" copy state, update state.
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in] pOutputSurface
    //!           Pointer to output surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxRenderMode2(
        PVPHAL_SURFACE           pSrcSurface,
        PVPHAL_SURFACE           pOutputSurface);

    //!
    //! \brief    Vebox render mode0to2
    //! \details  Purpose   : VEBOX/IECP Rendering for current and future frame
    //!           [Flow] 1. For current frame; setup state, copy state, update state, send cmd.
    //!                  2. For future frame;  setup state, copy state, update state, send cmd.
    //!                  3. setup state for next vebox operation.
    //!                  4. Request "speculative" copy state, update state.
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in] pOutputSurface
    //!           Pointer to output surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxRenderMode0To2(
        PVPHAL_SURFACE           pSrcSurface,
        PVPHAL_SURFACE           pOutputSurface);

    //!
    //! \brief    Vebox Render mode0
    //! \details  VEBOX/IECP Rendering for current frame
    //!           [Flow] 1. For current frame; setup state, copy state, update state, send cmd.
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in] pOutputSurface
    //!           Pointer to output surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxRenderMode0(
        PVPHAL_SURFACE           pSrcSurface,
        PVPHAL_SURFACE           pOutputSurface);

    //!
    //! \brief    Update output surface for FFDI Same sample case with SFC
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   output surface for the same sample case
    //!
    virtual PVPHAL_SURFACE GetOutputSurfForDiSameSampleWithSFC(
        PVPHAL_SURFACE pSrcSurface);

    //!
    //! \brief    Set DI output sample
    //! \details  Set DI sample to be used for compositing stage followed by VEBOX
    //!           feature reporting
    //! \param    [in] pSrcSurface
    //!           Pointer to Source Surface
    //! \param    [in,out] pOutputSurface
    //!           Pointer to Output Surface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if no error else MOS_STATUS_UNKNOWN
    //!
    virtual MOS_STATUS VeboxSetDiOutput(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pOutputSurface);

    //!
    //! \brief    Setup reference surfaces
    //! \details  Setup reference surfaces for app feeds reference case and
    //!           no reference frame case
    //! \param    [in] pSrcSurface
    //!           Pointer to Source Surface
    //! \return   PVPHAL_SURFACE
    //!           Pointer to Reference surface or nullptr if no reference
    //!
    PVPHAL_SURFACE VeboxSetReference(
        PVPHAL_SURFACE                  pSrcSurface);

    //!
    //! \brief    Add Extra kernels when VeboxFLushUpdateState
    //! \param    [in] CmdBuffer
    //!           reference to MOS_COMMAND_BUFFER
    //! \param    [out] MediaObjectParams
    //!           Reference to Media object params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxFlushUpdateStateAddExtraKernels(
        MOS_COMMAND_BUFFER&                 CmdBuffer,
        MHW_MEDIA_OBJECT_PARAMS&            MediaObjectParams)
    {
        MOS_UNUSED(CmdBuffer);
        MOS_UNUSED(MediaObjectParams);
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Check whether DN surface limitation is satisfied
    //! \param    [in] bDenoise
    //!           Flag to indicate whether DN is enabled
    //! \param    [in] CurrentSurface
    //!           Input surface of Vebox
    //! \param    [in] FFDNSurface
    //!           DN surface of Vebox
    //! \return   bool
    //!           Return true for limitation satisfied, otherwise false
    //!
    bool VeboxDNSurfaceLimitationSatisfied(
        bool                    bDenoise,
        VPHAL_SURFACE           *CurrentSurface,
        VPHAL_SURFACE           *FFDNSurface);

    //!
    //! \brief    Send Vecs Status Tag
    //! \details  Add MI Flush with write back into command buffer for GPU to write 
    //!           back GPU Tag. This should be the last command in 1st level batch.
    //!           This ensures sync tag will be written after rendering is complete.
    //! \param    [in] pMhwMiInterface
    //!           MHW MI interface
    //! \param    [in] pOsInterface
    //!           Pointer to OS Interface
    //! \param    [out] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \return   MOS_STATUS
    //!
    MOS_STATUS VeboxSendVecsStatusTag(
        PMHW_MI_INTERFACE                   pMhwMiInterface,
        PMOS_INTERFACE                      pOsInterface,
        PMOS_COMMAND_BUFFER                 pCmdBuffer);

    //!
    //! \brief    Calculate offsets of statistics surface address based on the
    //!           functions which were enabled in the previous call,
    //!           and store the width and height of the per-block statistics into DNDI_STATE
    //! \details
    //! Layout of Statistics surface when Temporal DI enabled
    //!     --------------------------------------------------------------\n
    //!     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
    //!     |-------------------------------------------------------------\n
    //!     | 16 bytes for x=0, Y=4       | ...\n
    //!     |------------------------------\n
    //!     | ...\n
    //!     |------------------------------\n
    //!     | 16 bytes for x=0, Y=height-4| ...\n
    //!     |-----------------------------------------------Pitch----------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 0 (Previous)| 17 DW Reserved         | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
    //!     |--------------------------------------------------------------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 0 (Current) | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
    //!     |--------------------------------------------------------------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 1 (Previous)| 17 DW Reserved         | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
    //!     |--------------------------------------------------------------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 1 (Current) | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
    //!     ---------------------------------------------------------------------------------------------------------------\n
    //!
    //! Layout of Statistics surface when DN or Spatial DI enabled (and Temporal DI disabled)
    //!     --------------------------------------------------------------\n
    //!     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
    //!     |-------------------------------------------------------------\n
    //!     | 16 bytes for x=0, Y=4       | ...\n
    //!     |------------------------------\n
    //!     | ...\n
    //!     |------------------------------\n
    //!     | 16 bytes for x=0, Y=height-4| ...\n
    //!     |-----------------------------------------------Pitch----------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 0 (Input)   | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
    //!     |--------------------------------------------------------------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 1 (Input)   | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
    //!     ---------------------------------------------------------------------------------------------------------------\n
    //!
    //! Layout of Statistics surface when both DN and DI are disabled
    //!     ------------------------------------------------Pitch----------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 0 (Input)   | 17 DW Reserved         | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
    //!     |--------------------------------------------------------------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 1 (Input)   | 17 DW Reserved         | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
    //!     ---------------------------------------------------------------------------------------------------------------\n
    //! \param    [out] pStatSlice0Offset
    //!           Statistics surface Slice 0 base pointer
    //! \param    [out] pStatSlice1Offset
    //!           Statistics surface Slice 1 base pointer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS VeboxGetStatisticsSurfaceOffsets(
        int32_t*                            pStatSlice0Offset,
        int32_t*                            pStatSlice1Offset);

    //!
    //! \brief    Check if 2 passes CSC are supported on the platform
    //!
    virtual bool Is2PassesCscPlatformSupported()
    {
        return false;
    }

    //!
    //! \brief    Check if 2 passes CSC are needed
    //! \param    [in] pSrc
    //!           Pointer to input surface of Vebox
    //! \param    [in] pRenderTarget
    //!           Pointer to Render targe surface of VPP BLT
    //! \return   bool
    //!           return true if 2 Passes CSC is needed, otherwise false
    //!
    bool VeboxIs2PassesCSCNeeded(
        PVPHAL_SURFACE              pSrc,
        PVPHAL_SURFACE              pRenderTarget);

    //!
    //! \brief    Copy Surface value
    //! \param    [in] pTargetSurface
    //!           Pointer to surface copy value to 
    //! \param    [in] pSourceSurface
    //!           Pointer to surface copy value from 
    //! \return   void
    //!
    virtual void CopySurfaceValue(
        PVPHAL_SURFACE              pTargetSurface,
        PVPHAL_SURFACE              pSourceSurface)
    {
        *pTargetSurface = *pSourceSurface;
    }

    //!
    //! \brief    Vebox get Luma default value
    //! \details  Initialize luma denoise paramters w/ default values.
    //! \param    [out] pLumaParams
    //!           Pointer to Luma DN parameter
    //! \return   void
    //!
    virtual void GetLumaDefaultValue(
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams) = 0;

    //!
    //! \brief    Vebox set DNDI parameter
    //! \details  Set denoise params for luma and chroma and deinterlace params
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in] pLumaParams
    //!           Pointer to Luma DN parameter
    //! \param    [in] pChromaParams
    //!           Pointer to Chroma DN parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetDNDIParams(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
        PVPHAL_DNUV_PARAMS              pChromaParams) = 0;

    //!
    //! \brief    Setup Vebox_State Command parameter
    //! \param    [in] bDiVarianceEnable
    //!           Is DI/Variances report enabled
    //! \param    [in,out] pVeboxStateCmdParams
    //!           Pointer to VEBOX_STATE command parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupVeboxState(
        bool                        bDiVarianceEnable,
        PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams) = 0;

    //!
    //! \brief    Create the platform specific sfc state
    //! \return   VphalSfcState*
    //!           Return created VphalSfcState pointer
    //!
    virtual VphalSfcState* CreateSfcState() = 0;

    //!
    //! \brief    Vebox Set Human Vision System based Denoise parameter
    //! \details  Vebox Set Human Vision System based Denoise parameter
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSetHVSDNParams(
        PVPHAL_SURFACE pSrcSurface);

    //!
    //! \brief Comp can be bypassed when the following conditions are all met
    //!
    bool IS_COMP_BYPASS_FEASIBLE(bool _bCompNeeded, PCVPHAL_RENDER_PARAMS _pcRenderParams, PVPHAL_SURFACE _pSrcSurface);

    //!
    //! \brief Vebox can be the output pipe when the following conditions are all met
    //!
    bool IS_OUTPUT_PIPE_VEBOX_FEASIBLE(PVPHAL_VEBOX_STATE _pVeboxState, PCVPHAL_RENDER_PARAMS _pcRenderParams, PVPHAL_SURFACE _pSrcSurface);

};

//!
//! \brief    Perform Rendering in VEBOX
//! \details  Check whether VEBOX Rendering is enabled. When it's enabled, perform VEBOX Rendering
//!           on the input surface and get the output surface
//! \param    [in,out] pRenderer
//!           VPHAL renderer pointer
//! \param    [in] pcRenderParams
//!           Const pointer to VPHAL render parameter
//! \param    [in,out] pRenderPassData
//!           Pointer to the VPHAL render pass data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_RndrRenderVebox(
    VphalRenderer           *pRenderer,
    PCVPHAL_RENDER_PARAMS   pcRenderParams,
    RenderpassData          *pRenderPassData);
#endif // __VPHAL_RENDER_VEBOX_BASE_H__
