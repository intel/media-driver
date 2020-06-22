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
//! \file      cm_def.h
//! \brief     Contains CM definitions
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMDEF_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMDEF_H_

#include "cm_def_os.h"
#include "cm_common.h"

//! Map CM_SURFACE_FORMAT to MOS_FORMAT
#define CM_SURFACE_FORMAT                       MOS_FORMAT
#define CM_SURFACE_FORMAT_INVALID               Format_Invalid
#define CM_SURFACE_FORMAT_A8R8G8B8              Format_A8R8G8B8
#define CM_SURFACE_FORMAT_X8R8G8B8              Format_X8R8G8B8
#define CM_SURFACE_FORMAT_A8B8G8R8              Format_A8B8G8R8
#define CM_SURFACE_FORMAT_A8                    Format_A8
#define CM_SURFACE_FORMAT_P8                    Format_P8
#define CM_SURFACE_FORMAT_R32F                  Format_R32F
#define CM_SURFACE_FORMAT_NV12                  Format_NV12
#define CM_SURFACE_FORMAT_P016                  Format_P016
#define CM_SURFACE_FORMAT_P010                  Format_P010
#define CM_SURFACE_FORMAT_P208                  Format_P208
#define CM_SURFACE_FORMAT_V8U8                  Format_V8U8
#define CM_SURFACE_FORMAT_A8L8                  Format_A8L8
#define CM_SURFACE_FORMAT_D16                   Format_D16
#define CM_SURFACE_FORMAT_A16B16G16R16F         Format_A16B16G16R16F
#define CM_SURFACE_FORMAT_R10G10B10A2           Format_R10G10B10A2
#define CM_SURFACE_FORMAT_A16B16G16R16          Format_A16B16G16R16
#define CM_SURFACE_FORMAT_IRW0                  Format_IRW0
#define CM_SURFACE_FORMAT_IRW1                  Format_IRW1
#define CM_SURFACE_FORMAT_IRW2                  Format_IRW2
#define CM_SURFACE_FORMAT_IRW3                  Format_IRW3
#define CM_SURFACE_FORMAT_R32_SINT              Format_R32S
#define CM_SURFACE_FORMAT_R16_FLOAT             Format_R16F
#define CM_SURFACE_FORMAT_A8P8                  Format_A8P8
#define CM_SURFACE_FORMAT_I420                  Format_I420
#define CM_SURFACE_FORMAT_IMC3                  Format_IMC3
#define CM_SURFACE_FORMAT_IA44                  Format_IA44
#define CM_SURFACE_FORMAT_AI44                  Format_AI44
#define CM_SURFACE_FORMAT_Y410                  Format_Y410
#define CM_SURFACE_FORMAT_Y416                  Format_Y416
#define CM_SURFACE_FORMAT_Y210                  Format_Y210
#define CM_SURFACE_FORMAT_Y216                  Format_Y216
#define CM_SURFACE_FORMAT_AYUV                  Format_AYUV
#define CM_SURFACE_FORMAT_YV12                  Format_YV12
#define CM_SURFACE_FORMAT_400P                  Format_400P
#define CM_SURFACE_FORMAT_411P                  Format_411P
#define CM_SURFACE_FORMAT_411R                  Format_411R
#define CM_SURFACE_FORMAT_422H                  Format_422H
#define CM_SURFACE_FORMAT_422V                  Format_422V
#define CM_SURFACE_FORMAT_444P                  Format_444P
#define CM_SURFACE_FORMAT_RGBP                  Format_RGBP
#define CM_SURFACE_FORMAT_BGRP                  Format_BGRP
#define CM_SURFACE_FORMAT_R8_UINT               Format_R8U
#define CM_SURFACE_FORMAT_R32_UINT              Format_R32U
#define CM_SURFACE_FORMAT_R16_SINT              Format_R16S
#define CM_SURFACE_FORMAT_R16_UNORM             Format_R16UN
#define CM_SURFACE_FORMAT_R8G8_UNORM            Format_R8G8UN
#define CM_SURFACE_FORMAT_R16_UINT              Format_R16U
#define CM_SURFACE_FORMAT_R16_TYPELESS          Format_R16
#define CM_SURFACE_FORMAT_R16G16_UNORM          Format_R16G16UN
#define CM_SURFACE_FORMAT_L16                   Format_L16
#define CM_SURFACE_FORMAT_YUY2                  Format_YUY2
#define CM_SURFACE_FORMAT_L8                    Format_L8
#define CM_SURFACE_FORMAT_UYVY                  Format_UYVY
#define CM_SURFACE_FORMAT_VYUY                  Format_VYUY
#define CM_SURFACE_FORMAT_R8G8_SNORM            Format_R8G8SN
#define CM_SURFACE_FORMAT_Y16_SNORM             Format_Y16S
#define CM_SURFACE_FORMAT_Y16_UNORM             Format_Y16U
#define CM_SURFACE_FORMAT_Y8_UNORM              Format_Y8
#define CM_SURFACE_FORMAT_BUFFER_2D             Format_Buffer_2D
#define CM_SURFACE_FORMAT_D32F                  Format_D32F
#define CM_SURFACE_FORMAT_D24_UNORM_S8_UINT     Format_D24S8UN
#define CM_SURFACE_FORMAT_D32F_S8X24_UINT       Format_D32S8X24_FLOAT
#define CM_SURFACE_FORMAT_R16G16_SINT           Format_R16G16S
#define CM_SURFACE_FORMAT_R24G8_TYPELESS        Format_R24G8
#define CM_SURFACE_FORMAT_R32_TYPELESS          Format_R32
#define CM_SURFACE_FORMAT_R32G8X24_TYPELESS     Format_R32G8X24
#define CM_SURFACE_FORMAT_R8_UNORM              Format_R8UN
#define CM_SURFACE_FORMAT_R32G32B32A32F         Format_R32G32B32A32F

typedef unsigned char byte;

#define CM_RT_API
#define CMRT_UMD_API

#define CISA_MAGIC_NUMBER       0x41534943      //"CISA"
#define CM_MIN_SURF_WIDTH       1
#define CM_MIN_SURF_HEIGHT      1
#define CM_MIN_SURF_DEPTH       2

#define CM_MAX_1D_SURF_WIDTH    0x80000000 // 2^31, 2 GB

#define CM_PAGE_ALIGNMENT       0x1000
#define CM_PAGE_ALIGNMENT_MASK  0x0FFF

#define CM_MAX_3D_SURF_WIDTH            2048
#define CM_MAX_3D_SURF_HEIGHT           2048
#define CM_MAX_3D_SURF_DEPTH            2048

#define CM_INIT_PROGRAM_COUNT       16
#define CM_INIT_KERNEL_COUNT        64
#define CM_INIT_SAMPLER_COUNT       32
#define CM_INIT_TASK_COUNT              16
#define CM_INIT_THREADGROUPSPACE_COUNT  8
#define CM_INIT_SAMPLER_8X8_STATE_COUNT 8
#define CM_INIT_EVENT_COUNT             128
#define CM_INIT_THREADSPACE_COUNT       8
#define CM_INIT_VEBOX_COUNT             16

#define CM_NO_EVENT                     ((CmEvent *)(-1)) // Magic Number for invisible event.

#define _NAME(...) #__VA_ARGS__
// hard ceiling
#define CM_MAX_OPTION_SIZE_IN_BYTE          512
#define CM_MAX_KERNEL_NAME_SIZE_IN_BYTE     256
#define CM_MAX_ISA_FILE_NAME_SIZE_IN_BYTE   256
#define CM_MAX_KERNEL_STRING_IN_BYTE        512

//Time in seconds before kernel should timeout
#define CM_MAX_TIMEOUT                      2
//Time in milliseconds before kernel should timeout
#define CM_MAX_TIMEOUT_MS                   CM_MAX_TIMEOUT*1000

#define CM_INVALID_KERNEL_INDEX             0xFFFFFFFF

#define CM_VME_FORWARD_ARRAY_LENGTH     16
#define CM_VME_BACKWARD_ARRAY_LENGTH    16

#define CM_INVALID_VME_SURFACE          0xFFFFFFFF

#define CM_INVALID_GLOBAL_SURFACE       0xFFFFFFFF

//GT-PIN
#define CM_MAX_ENTRY_FOR_A_SURFACE      6   //maxium planes(3)*dual state(2)
#define CM_GTPIN_BUFFER_NUM             3

#define CM_INIT_KERNEL_PER_PROGRAM              64  //

#define CM_MAX_SURFACE3D_FORMAT_COUNT   3

#define CM_RT_PLATFORM              "CM_RT_PLATFORM"
#define INCLUDE_GTENVVAR_NAME       "CM_DYNGT_INCLUDE"
#define CM_RT_SKU                   "CM_RT_SKU"
#define CM_RT_MAX_THREADS           "CM_RT_MAX_THREADS"
#define CM_RT_AUB_PARAM             "CM_RT_AUB_PARAM"
#define CM_RT_MUL_FRAME_FILE_BEGIN   0
#define CM_RT_MUL_FRAME_FILE_MIDDLE  1
#define CM_RT_MUL_FRAME_FILE_END     2

#define CM_RT_USER_FEATURE_FORCE_COHERENT_STATELESSBTI    "ForceCoherentStatelessBTI"

// need to sync with driver code
#define CM_HAL_LOCKFLAG_READONLY        0x00000001
#define CM_HAL_LOCKFLAG_WRITEONLY       0x00000002

#define CM_MAX_DEPENDENCY_COUNT                8
#define CM_MAX_THREADSPACE_WIDTH_FOR_MW        511
#define CM_MAX_THREADSPACE_HEIGHT_FOR_MW       511
#define CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW  2047
#define CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW 2047

#define MAX_SLM_SIZE_PER_GROUP_IN_1K        64 // 64KB PER Group on Gen7+
#define CM_MAX_THREAD_GROUP                 64

#define COMMON_ISA_NUM_PREDEFINED_SURF_VER_2    1
#define COMMON_ISA_NUM_PREDEFINED_SURF_VER_2_1  5
#define COMMON_ISA_NUM_PREDEFINED_SURF_VER_3_1  6

#define CM_FLAG_CURBE_ENABLED                   0x00000001  //bit 0
#define CM_FLAG_NONSTALLING_SCOREBOARD_ENABLED  0x00000002  //bit 1

#define GT_PIN_MSG_SIZE 1024

#define CM_GLOBAL_SURFACE_NUMBER      4
#define CM_GTPIN_SURFACE_NUMBER       3

#define GT_RESERVED_INDEX_START                                 250
#define GT_RESERVED_INDEX_START_GEN9_PLUS                       240
#define CM_GLOBAL_SURFACE_INDEX_START                           243
#define CM_GLOBAL_SURFACE_INDEX_START_GEN9_PLUS                 1
#define CM_NULL_SURFACE_BINDING_INDEX                           0                           //Reserve 0 for NULL surface

#define GTPIN_BINDING_TABLE_INDEX_BUFF0_GEN9_PLUS              (CM_GLOBAL_SURFACE_INDEX_START_GEN9_PLUS + CM_GLOBAL_SURFACE_NUMBER)
#define GTPIN_BINDING_TABLE_INDEX_BUFF1_GEN9_PLUS              (GTPIN_BINDING_TABLE_INDEX_BUFF0_GEN9_PLUS + 1)
#define GTPIN_BINDING_TABLE_INDEX_BUFF2_GEN9_PLUS              (GTPIN_BINDING_TABLE_INDEX_BUFF0_GEN9_PLUS + 2)

#define CM_NULL_SURFACE                     0xFFFF

#define R64_OFFSET                          32*64
#define CM_MOVE_INSTRUCTION_SIZE            16 // 16 bytes per move instruction

#define CM_SAMPLER_MAX_BINDING_INDEX        15

// For EnqueueWithHints using media objects
// hard code add instruction to adjust y coordinate
// just need to replace DW3 with constant value
// add (1) r0.3<1>:uw r0.3<0;1,0>:uw 0x0:w
#define CM_BDW_ADJUST_Y_SCOREBOARD_DW0          0x00000040
#define CM_BDW_ADJUST_Y_SCOREBOARD_DW1          0x20061248
#define CM_BDW_ADJUST_Y_SCOREBOARD_DW2          0x1e000006

#define CM_MINIMUM_NUM_KERNELS_ENQWHINTS        2

#define CM_DEFAULT_PRINT_BUFFER_SIZE           (1*1024*1024) // 1M print buffer size
#define PRINT_BUFFER_HEADER_SIZE            32
#define CM_PRINTF_STATIC_BUFFER_ID          1

#define CM_INVALID_COLOR_COUNT              0

#define CM_INIT_GPUCOPY_KERNL_COUNT             16

#define CM_NUM_VME_HEVC_REFS                    4

#define PLATFORM_INTEL_UNKNOWN                  0

#define PLATFORM_INTEL_GT_UNKNOWN               0
#define PLATFORM_INTEL_GT1                      1
#define PLATFORM_INTEL_GT2                      2
#define PLATFORM_INTEL_GT3                      3
#define PLATFORM_INTEL_GT4                      4
#define PLATFORM_INTEL_GT1_5                    10

#define BDW_GT1_MAX_NUM_SLICES                  (1)
#define BDW_GT1_MAX_NUM_SUBSLICES               (2)
#define BDW_GT1_5_MAX_NUM_SLICES                (1)
#define BDW_GT1_5_MAX_NUM_SUBSLICES             (3)
#define BDW_GT2_MAX_NUM_SLICES                  (1)
#define BDW_GT2_MAX_NUM_SUBSLICES               (3)
#define BDW_GT3_MAX_NUM_SLICES                  (2)
#define BDW_GT3_MAX_NUM_SUBSLICES               (6)

#define SKL_GT1_MAX_NUM_SLICES                  (1)
#define SKL_GT1_MAX_NUM_SUBSLICES               (2)
#define SKL_GT1_5_MAX_NUM_SLICES                (1)
#define SKL_GT1_5_MAX_NUM_SUBSLICES             (3)
#define SKL_GT2_MAX_NUM_SLICES                  (1)
#define SKL_GT2_MAX_NUM_SUBSLICES               (3)
#define SKL_GT3_MAX_NUM_SLICES                  (2)
#define SKL_GT3_MAX_NUM_SUBSLICES               (6)
#define SKL_GT4_MAX_NUM_SLICES                  (3)
#define SKL_GT4_MAX_NUM_SUBSLICES               (9)

#define CNL_GT1_4X8_MAX_NUM_SLICES              (2)
#define CNL_GT1_4X8_MAX_NUM_SUBSLICES           (4)
#define CNL_GT2_7X8_MAX_NUM_SLICES              (3)
#define CNL_GT2_7X8_MAX_NUM_SUBSLICES           (7)
#define CNL_GT3_9X8_MAX_NUM_SLICES              (4)
#define CNL_GT3_9_8MAX_NUM_SUBSLICES            (9)

typedef enum _CM_DEVICE_CAP_NAME
{
    CAP_KERNEL_COUNT_PER_TASK,
    CAP_KERNEL_BINARY_SIZE,
    CAP_SAMPLER_COUNT ,
    CAP_SAMPLER_COUNT_PER_KERNEL,
    CAP_BUFFER_COUNT ,
    CAP_SURFACE2D_COUNT,
    CAP_SURFACE3D_COUNT,
    CAP_SURFACE_COUNT_PER_KERNEL,
    CAP_ARG_COUNT_PER_KERNEL,
    CAP_ARG_SIZE_PER_KERNEL ,
    CAP_USER_DEFINED_THREAD_COUNT_PER_TASK,
    CAP_HW_THREAD_COUNT,
    CAP_SURFACE2D_FORMAT_COUNT,
    CAP_SURFACE2D_FORMATS,
    CAP_SURFACE3D_FORMAT_COUNT,
    CAP_SURFACE3D_FORMATS,
    CAP_VME_STATE_COUNT,
    CAP_GPU_PLATFORM,
    CAP_GT_PLATFORM,
    CAP_MIN_FREQUENCY,
    CAP_MAX_FREQUENCY,
    CAP_L3_CONFIG,
    CAP_GPU_CURRENT_FREQUENCY,
    CAP_USER_DEFINED_THREAD_COUNT_PER_TASK_NO_THREAD_ARG,
    CAP_USER_DEFINED_THREAD_COUNT_PER_MEDIA_WALKER,
    CAP_USER_DEFINED_THREAD_COUNT_PER_THREAD_GROUP,
    CAP_SURFACE2DUP_COUNT,
    CAP_PLATFORM_INFO,
    CAP_MAX_BUFFER_SIZE
} CM_DEVICE_CAP_NAME;

// BDW stepping sequence:        //  A0
// HSW stepping sequence:        //  A0, A1, B0, C0, D0
#define HW_GT_STEPPING_A0   "A0"
#define HW_GT_STEPPING_A1   "A1"
#define HW_GT_STEPPING_B0   "B0"
#define HW_GT_STEPPING_C0   "C0"
#define HW_GT_STEPPING_D0   "D0"

/**************** L3/Cache ***************/
typedef enum _MEMORY_OBJECT_CONTROL{
    // SNB
    MEMORY_OBJECT_CONTROL_USE_GTT_ENTRY,
    MEMORY_OBJECT_CONTROL_NEITHER_LLC_NOR_MLC,
    MEMORY_OBJECT_CONTROL_LLC_NOT_MLC,
    MEMORY_OBJECT_CONTROL_LLC_AND_MLC,

    // IVB
    MEMORY_OBJECT_CONTROL_FROM_GTT_ENTRY = MEMORY_OBJECT_CONTROL_USE_GTT_ENTRY,  // Caching dependent on pte
    MEMORY_OBJECT_CONTROL_L3,                                             // Cached in L3$
    MEMORY_OBJECT_CONTROL_LLC,                                            // Cached in LLC
    MEMORY_OBJECT_CONTROL_LLC_L3,                                         // Cached in LLC & L3$

    // HSW
    MEMORY_OBJECT_CONTROL_USE_PTE = MEMORY_OBJECT_CONTROL_FROM_GTT_ENTRY, // Caching dependent on pte
    MEMORY_OBJECT_CONTROL_L3_USE_PTE,
    MEMORY_OBJECT_CONTROL_UC,                                             // Uncached
    MEMORY_OBJECT_CONTROL_L3_UC,
    MEMORY_OBJECT_CONTROL_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_L3_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_ELLC,
    MEMORY_OBJECT_CONTROL_L3_ELLC,

    // BDW
    MEMORY_OBJECT_CONTROL_BDW_ELLC_ONLY = 0,
    MEMORY_OBJECT_CONTROL_BDW_LLC_ONLY,
    MEMORY_OBJECT_CONTROL_BDW_LLC_ELLC_ALLOWED,
    MEMORY_OBJECT_CONTROL_BDW_L3_LLC_ELLC_ALLOWED,

    // SKL
    // CNL
    // ICL
    MEMORY_OBJECT_CONTROL_SKL_DEFAULT = 0,
    MEMORY_OBJECT_CONTROL_SKL_NO_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_CACHE,

    // Unified memory object control type for SKL+
    MEMORY_OBJECT_CONTROL_DEFAULT = 0x0,
    MEMORY_OBJECT_CONTROL_NO_L3,
    MEMORY_OBJECT_CONTROL_NO_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_NO_LLC,
    MEMORY_OBJECT_CONTROL_NO_ELLC,
    MEMORY_OBJECT_CONTROL_NO_LLC_L3,
    MEMORY_OBJECT_CONTROL_NO_ELLC_L3,
    MEMORY_OBJECT_CONTROL_NO_CACHE,
    MEMORY_OBJECT_CONTROL_L1_ENABLED,

    MEMORY_OBJECT_CONTROL_TOTAL,
    //
    MEMORY_OBJECT_CONTROL_UNKNOW = 0xff
} MEMORY_OBJECT_CONTROL;

typedef enum _MEMORY_TYPE {
    CM_USE_PTE,
    CM_UN_CACHEABLE,
    CM_WRITE_THROUGH,
    CM_WRITE_BACK,

    // BDW
    MEMORY_TYPE_BDW_UC_WITH_FENCE = 0,
    MEMORY_TYPE_BDW_UC,
    MEMORY_TYPE_BDW_WT,
    MEMORY_TYPE_BDW_WB

} MEMORY_TYPE;

typedef struct _CM_SURFACE_MEM_OBJ_CTRL {
    int32_t mem_ctrl;
    MEMORY_TYPE mem_type;
    int32_t age;
} CM_SURFACE_MEM_OBJ_CTRL;

//GT-PIN
#define CM_MAX_ENTRY_FOR_A_SURFACE  6   //maxium planes(3)*dual state(2)
#define CM_GTPIN_BUFFER_NUM 3

typedef enum _CM_SAMPLER8x8_SURFACE_
{
    CM_AVS_SURFACE = 0,
    CM_VA_SURFACE = 1
}CM_SAMPLER8x8_SURFACE;

typedef enum _CM_SURFACE_ADDRESS_CONTROL_MODE_
{
    CM_SURFACE_CLAMP = 0,
    CM_SURFACE_MIRROR = 1
}CM_SURFACE_ADDRESS_CONTROL_MODE;

typedef struct _CM_SAMPLER_STATE
{
    CM_TEXTURE_FILTER_TYPE minFilterType;
    CM_TEXTURE_FILTER_TYPE magFilterType;
    CM_TEXTURE_ADDRESS_TYPE addressU;
    CM_TEXTURE_ADDRESS_TYPE addressV;
    CM_TEXTURE_ADDRESS_TYPE addressW;
} CM_SAMPLER_STATE;

typedef enum _CM_PIXEL_TYPE
{
    CM_PIXEL_UINT,
    CM_PIXEL_SINT,
    CM_PIXEL_OTHER
} CM_PIXEL_TYPE;

typedef struct _CM_SAMPLER_STATE_EX
{
    CM_TEXTURE_FILTER_TYPE minFilterType;
    CM_TEXTURE_FILTER_TYPE magFilterType;
    CM_TEXTURE_ADDRESS_TYPE addressU;
    CM_TEXTURE_ADDRESS_TYPE addressV;
    CM_TEXTURE_ADDRESS_TYPE addressW;

    CM_PIXEL_TYPE SurfaceFormat;
    union {
        uint32_t BorderColorRedU;
        int32_t BorderColorRedS;
        float BorderColorRedF;
    };

    union {
        uint32_t BorderColorGreenU;
        int32_t BorderColorGreenS;
        float BorderColorGreenF;
    };

    union {
        uint32_t BorderColorBlueU;
        int32_t BorderColorBlueS;
        float BorderColorBlueF;
    };

    union {
        uint32_t BorderColorAlphaU;
        int32_t BorderColorAlphaS;
        float BorderColorAlphaF;
    };
} CM_SAMPLER_STATE_EX;

typedef struct _CM_AVS_INTERNEL_COEFF_TABLE{
    float   FilterCoeff_0_0;
    float   FilterCoeff_0_1;
    float   FilterCoeff_0_2;
    float   FilterCoeff_0_3;
    float   FilterCoeff_0_4;
    float   FilterCoeff_0_5;
    float   FilterCoeff_0_6;
    float   FilterCoeff_0_7;
}CM_AVS_INTERNEL_COEFF_TABLE;

#define CM_NUM_COEFF_ROWS 17
#define CM_NUM_COEFF_ROWS_SKL 32

typedef struct _CM_AVS_NONPIPLINED_STATE{
    bool BypassXAF;
    bool BypassYAF;
    uint8_t DefaultSharpLvl;
    uint8_t maxDerivative4Pixels;
    uint8_t maxDerivative8Pixels;
    uint8_t transitionArea4Pixels;
    uint8_t transitionArea8Pixels;
    CM_AVS_COEFF_TABLE Tbl0X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_COEFF_TABLE Tbl0Y[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_COEFF_TABLE Tbl1X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_COEFF_TABLE Tbl1Y[ CM_NUM_COEFF_ROWS_SKL ];
    bool bEnableRGBAdaptive;
    bool bAdaptiveFilterAllChannels;
}CM_AVS_NONPIPLINED_STATE;

typedef struct _CM_AVS_INTERNEL_NONPIPLINED_STATE{
    bool BypassXAF;
    bool BypassYAF;
    uint8_t DefaultSharpLvl;
    uint8_t maxDerivative4Pixels;
    uint8_t maxDerivative8Pixels;
    uint8_t transitionArea4Pixels;
    uint8_t transitionArea8Pixels;
    CM_AVS_INTERNEL_COEFF_TABLE Tbl0X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_INTERNEL_COEFF_TABLE Tbl0Y[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_INTERNEL_COEFF_TABLE Tbl1X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_INTERNEL_COEFF_TABLE Tbl1Y[ CM_NUM_COEFF_ROWS_SKL ];
    bool bEnableRGBAdaptive;
    bool bAdaptiveFilterAllChannels;
}CM_AVS_INTERNEL_NONPIPLINED_STATE, *PCM_AVS_INTERNEL_NONPIPLINED_STATE;

typedef struct _CM_AVS_STATE_MSG{
    bool AVSTYPE; //true nearest, false adaptive
    bool EightTapAFEnable; //HSW+
    bool BypassIEF; //ignored for BWL, moved to sampler8x8 payload.
    bool ShuffleOutputWriteback; //SKL mode only to be set when AVS msg sequence is 4x4 or 8x4
    bool HDCDirectWriteEnable;
    unsigned short GainFactor;
    unsigned char GlobalNoiseEstm;
    unsigned char StrongEdgeThr;
    unsigned char WeakEdgeThr;
    unsigned char StrongEdgeWght;
    unsigned char RegularWght;
    unsigned char NonEdgeWght;
    unsigned short wR3xCoefficient;
    unsigned short wR3cCoefficient;
    unsigned short wR5xCoefficient;
    unsigned short wR5cxCoefficient;
    unsigned short wR5cCoefficient;
    //For Non-piplined states
    unsigned short stateID;
    CM_AVS_NONPIPLINED_STATE * AvsState;
} CM_AVS_STATE_MSG;

/*
*  CONVOLVE STATE DATA STRUCTURES
*/
typedef struct _CM_CONVOLVE_COEFF_TABLE{
    float   FilterCoeff_0_0;
    float   FilterCoeff_0_1;
    float   FilterCoeff_0_2;
    float   FilterCoeff_0_3;
    float   FilterCoeff_0_4;
    float   FilterCoeff_0_5;
    float   FilterCoeff_0_6;
    float   FilterCoeff_0_7;
    float   FilterCoeff_0_8;
    float   FilterCoeff_0_9;
    float   FilterCoeff_0_10;
    float   FilterCoeff_0_11;
    float   FilterCoeff_0_12;
    float   FilterCoeff_0_13;
    float   FilterCoeff_0_14;
    float   FilterCoeff_0_15;
    float   FilterCoeff_0_16;
    float   FilterCoeff_0_17;
    float   FilterCoeff_0_18;
    float   FilterCoeff_0_19;
    float   FilterCoeff_0_20;
    float   FilterCoeff_0_21;
    float   FilterCoeff_0_22;
    float   FilterCoeff_0_23;
    float   FilterCoeff_0_24;
    float   FilterCoeff_0_25;
    float   FilterCoeff_0_26;
    float   FilterCoeff_0_27;
    float   FilterCoeff_0_28;
    float   FilterCoeff_0_29;
    float   FilterCoeff_0_30;
    float   FilterCoeff_0_31;
}CM_CONVOLVE_COEFF_TABLE;

#define CM_NUM_CONVOLVE_ROWS_SKL 31
typedef struct _CM_CONVOLVE_STATE_MSG{
  bool CoeffSize; //true 16-bit, false 8-bit
  byte SclDwnValue; //Scale down value
  byte Width; //Kernel Width
  byte Height; //Kernel Height
  //SKL mode
  bool isVertical32Mode;
  bool isHorizontal32Mode;
  bool skl_mode;  // new added
  CM_CONVOLVE_SKL_TYPE nConvolveType;
  CM_CONVOLVE_COEFF_TABLE Table[ CM_NUM_CONVOLVE_ROWS_SKL ];
} CM_CONVOLVE_STATE_MSG;

/*
 *   MISC SAMPLER8x8 State
 */
typedef struct _CM_MISC_STATE {
    //uint32_t 0
    union{
        struct{
            uint32_t Row0      : 16;
            uint32_t Reserved  : 8;
            uint32_t Width     : 4;
            uint32_t Height    : 4;
        };
        struct{
            uint32_t value;
        };
    }DW0;

    //uint32_t 1
    union{
        struct{
            uint32_t Row1      : 16;
            uint32_t Row2      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW1;

    //uint32_t 2
    union{
        struct{
            uint32_t Row3      : 16;
            uint32_t Row4      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW2;

    //uint32_t 3
    union{
        struct{
            uint32_t Row5      : 16;
            uint32_t Row6      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW3;

    //uint32_t 4
    union{
        struct{
            uint32_t Row7      : 16;
            uint32_t Row8      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW4;

    //uint32_t 5
    union{
        struct{
            uint32_t Row9      : 16;
            uint32_t Row10      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW5;

    //uint32_t 6
    union{
        struct{
            uint32_t Row11      : 16;
            uint32_t Row12      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW6;

    //uint32_t 7
    union{
        struct{
            uint32_t Row13      : 16;
            uint32_t Row14      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW7;
} CM_MISC_STATE;

typedef struct _CM_MISC_STATE_MSG{
    //uint32_t 0
    union{
        struct{
            uint32_t Row0      : 16;
            uint32_t Reserved  : 8;
            uint32_t Width     : 4;
            uint32_t Height    : 4;
        };
        struct{
            uint32_t value;
        };
    }DW0;

    //uint32_t 1
    union{
        struct{
            uint32_t Row1      : 16;
            uint32_t Row2      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW1;

    //uint32_t 2
    union{
        struct{
            uint32_t Row3      : 16;
            uint32_t Row4      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW2;

    //uint32_t 3
    union{
        struct{
            uint32_t Row5      : 16;
            uint32_t Row6      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW3;

    //uint32_t 4
    union{
        struct{
            uint32_t Row7      : 16;
            uint32_t Row8      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW4;

    //uint32_t 5
    union{
        struct{
            uint32_t Row9      : 16;
            uint32_t Row10      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW5;

    //uint32_t 6
    union{
        struct{
            uint32_t Row11      : 16;
            uint32_t Row12      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW6;

    //uint32_t 7
    union{
        struct{
            uint32_t Row13      : 16;
            uint32_t Row14      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW7;
} CM_MISC_STATE_MSG;

typedef CM_HAL_SAMPLER_8X8_TYPE CM_SAMPLER_STATE_TYPE;

typedef struct _CM_SAMPLER_8X8_DESCR{
    CM_SAMPLER_STATE_TYPE stateType;
    union
    {
        CM_AVS_STATE_MSG * avs;
        CM_CONVOLVE_STATE_MSG * conv;
        CM_MISC_STATE_MSG * misc; //ERODE/DILATE/MINMAX
    };
} CM_SAMPLER_8X8_DESCR;

typedef enum _CM_ROTATION
{
    CM_ROTATION_IDENTITY = 0,      //!< Rotation 0 degrees
    CM_ROTATION_90,                //!< Rotation 90 degrees
    CM_ROTATION_180,               //!< Rotation 180 degrees
    CM_ROTATION_270,               //!< Rotation 270 degrees
} CM_ROTATION;

// to support new flag with current API
// new flag/field could be add to the end of this structure
//
struct CM_FLAG {
    CM_FLAG();
    CM_ROTATION rotationFlag;
    int32_t chromaSiting;
};

// parameters used to set the surface state of the CmSurface
struct CM_VME_SURFACE_STATE_PARAM
{
    uint32_t width;
    uint32_t height;
};

// parameters used to set the surface state of the CmSurface
typedef struct _CM_SURFACE2D_STATE_PARAM
{
    uint32_t format; //[IN] MOS_FORMAT
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t pitch;
    uint16_t memory_object_control;
    uint32_t surface_x_offset;  // Horizontal offset to the origin of the surface, in columns of pixels.
    uint32_t surface_y_offset;  // Vertical offset to the origin of the surface, in rows of pixels.
    uint32_t surface_offset;  // Offset to the origin of the surface, in bytes.
    uint32_t reserved[3]; // for future usage
} CM_SURFACE2D_STATE_PARAM;

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMDEF_H_
