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
#pragma once
#include "cm_jitter_info.h"
#include "cm_debug.h"
#include "cm_csync.h"
#include "cm_log.h"
#include "cm_def_os.h"

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
#define CM_SURFACE_FORMAT_R16_TYPELESS          Format_D16
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

typedef unsigned char byte;

#define CM_RT_API 
#define CMRT_UMD_API

#define CISA_MAGIC_NUMBER       0x41534943      //"CISA"
#define CM_MIN_SURF_WIDTH       1
#define CM_MIN_SURF_HEIGHT      1
#define CM_MIN_SURF_DEPTH       2

#define CM_MAX_1D_SURF_WIDTH    0x40000000 // 2^30, 1024M

#define CM_PAGE_ALIGNMENT       0x1000
#define CM_PAGE_ALIGNMENT_MASK  0x0FFF

#define CM_MAX_2D_SURF_WIDTH  16384
#define CM_MAX_2D_SURF_HEIGHT 16384

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
#define CM_RT_JITTER_DEBUG_FLAG      "-debug"
#define CM_RT_JITTER_GTPIN_FLAG      "-gtpin"
#define CM_RT_JITTER_NCSTATELESS_FLAG      "-ncstateless"
#define CM_RT_JITTER_MAX_NUM_FLAGS      30
#define CM_RT_JITTER_NUM_RESERVED_FLAGS 3// one for gtpin;  two for hw stepping info
#define CM_RT_JITTER_MAX_NUM_USER_FLAGS (CM_RT_JITTER_MAX_NUM_FLAGS - CM_RT_JITTER_NUM_RESERVED_FLAGS)

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

#define CM_THREADSPACE_MAX_COLOR_COUNT      16
#define CM_INVALID_COLOR_COUNT              0

#define CM_KERNEL_DATA_CLEAN                    0           // kernel data clean
#define CM_KERNEL_DATA_KERNEL_ARG_DIRTY         1           // per kernel arg dirty
#define CM_KERNEL_DATA_THREAD_ARG_DIRTY         (1 << 1)    // per thread arg dirty
#define CM_KERNEL_DATA_PAYLOAD_DATA_DIRTY       (1 << 2)    // indirect payload data dirty
#define CM_KERNEL_DATA_PAYLOAD_DATA_SIZE_DIRTY  (1 << 3)    // indirect payload data size changes
#define CM_KERNEL_DATA_GLOBAL_SURFACE_DIRTY     (1 << 4)    // global surface dirty
#define CM_KERNEL_DATA_THREAD_COUNT_DIRTY       (1 << 5)    // thread count dirty, reset() be called
#define CM_KERNEL_DATA_SAMPLER_BTI_DIRTY        (1 << 6)    // sampler bti dirty

#define CM_INIT_GPUCOPY_KERNL_COUNT             16

#define SURFACE_FLAG_ASSUME_NOT_IN_USE          1
#define CM_NUM_VME_HEVC_REFS                    4

#define PLATFORM_INTEL_UNKNOWN                  0

#define PLATFORM_INTEL_GT_UNKNOWN               0
#define PLATFORM_INTEL_GT1                      1
#define PLATFORM_INTEL_GT2                      2
#define PLATFORM_INTEL_GT3                      3
#define PLATFORM_INTEL_GT4                      4
#define PLATFORM_INTEL_GT1_5                    10

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

typedef enum _SURFACE_DESTROY_KIND{
    APP_DESTROY         = 0,
    GC_DESTROY          = 1, 
    FORCE_DESTROY       = 2,
    DELAYED_DESTROY     = 3
} SURFACE_DESTROY_KIND;

typedef enum _CM_GPUCOPY_DIRECTION
{
    CM_FASTCOPY_GPU2CPU = 0,
    CM_FASTCOPY_CPU2GPU = 1,
    CM_FASTCOPY_GPU2GPU = 2,
    CM_FASTCOPY_CPU2CPU = 3
} CM_GPUCOPY_DIRECTION;

typedef enum _CM_FASTCOPY_OPTION
{
    CM_FASTCOPY_OPTION_NONBLOCKING = 0x00,
    CM_FASTCOPY_OPTION_BLOCKING = 0x01,
    CM_FASTCOPY_OPTION_DISABLE_TURBO_BOOST = 0x02
} CM_FASTCOPY_OPTION;

typedef enum _CM_STATUS
{
    CM_STATUS_QUEUED         = 0,
    CM_STATUS_FLUSHED        = 1,
    CM_STATUS_FINISHED       = 2,
    CM_STATUS_STARTED        = 3,
    CM_STATUS_RESET          = 4

}CM_STATUS;

typedef enum _CM_TS_FLAG
{
    WHITE = 0, 
    GRAY  = 1,
    BLACK = 2
} CM_TS_FLAG;

typedef struct _CM_COORDINATE
{
    int32_t x;
    int32_t y;
} CM_COORDINATE, *PCM_COORDINATE;

typedef struct _CM_THREAD_SPACE_UNIT 
{
    void            *pKernel;
    uint32_t        threadId;
    int32_t         numEdges; //For Emulation mode
    CM_COORDINATE   scoreboardCoordinates;
    uint8_t         dependencyMask;
    uint8_t         reset; 
    uint8_t         scoreboardColor;
    uint8_t         sliceDestinationSelect;
    uint8_t         subSliceDestinationSelect;
} CM_THREAD_SPACE_UNIT;

typedef struct _CM_THREAD_PARAM
{
    CM_COORDINATE   scoreboardCoordinates;     //[X, Y] terms of the scoreboard values of the current thread.
    uint8_t         scoreboardColor;           // dependency color the current thread.
    uint8_t         sliceDestinationSelect;    //select determines the slice of the current thread must be sent to.
    uint8_t         subSliceDestinationSelect;    //select determines the sub-slice of the current thread must be sent to.
}CM_THREAD_PARAM;

typedef enum _CM_THREAD_SPACE_DIRTY_STATUS
{
    CM_THREAD_SPACE_CLEAN                 = 0,
    CM_THREAD_SPACE_DEPENDENCY_MASK_DIRTY = 1,
    CM_THREAD_SPACE_DATA_DIRTY            = 2
}CM_THREAD_SPACE_DIRTY_STATUS, *PCM_THREAD_SPACE_DIRTY_STATUS;

typedef struct _CM_DEPENDENCY
{
    uint32_t count;
    int32_t deltaX[CM_MAX_DEPENDENCY_COUNT];
    int32_t deltaY[CM_MAX_DEPENDENCY_COUNT];
}CM_DEPENDENCY;

typedef enum _SAMPLER_SURFACE_TYPE
{
    SAMPLER_SURFACE_TYPE_2D,
    SAMPLER_SURFACE_TYPE_2DUP,
    SAMPLER_SURFACE_TYPE_3D
} SAMPLER_SURFACE_TYPE;

typedef enum _CM_INTERNAL_TASK_TYPE
{
    CM_INTERNAL_TASK_WITH_THREADSPACE,
    CM_INTERNAL_TASK_WITH_THREADGROUPSPACE,
    CM_INTERNAL_TASK_VEBOX,
    CM_INTERNAL_TASK_ENQUEUEWITHHINTS
} CM_INTERNAL_TASK_TYPE;

#define CM_TASK_TYPE_DEFAULT    CM_INTERNAL_TASK_WITH_THREADSPACE

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
    MEMORY_OBJECT_CONTROL_SKL_DEFAULT = 0,
    MEMORY_OBJECT_CONTROL_SKL_NO_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_CACHE,

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
    MEMORY_OBJECT_CONTROL mem_ctrl;
    MEMORY_TYPE mem_type;
    int32_t age;
} CM_SURFACE_MEM_OBJ_CTRL;

typedef enum _L3_ALLOC_KIND
{
    SLM_ALLOCATION = 0,
    URB_ALLOCATION,
    REST_ALLOCATION,
    DC_ASSIGNMENT,
    READ_ONLY_POOL,
    IS_ALLOCATION,
    CONSTANT_ALLICATION,
    TEXTURE_ALLICATION,
    SUM_ALLICATION
} L3_ALLOC_KIND;

typedef struct _CM_SET_CAPS
{
    CM_SET_TYPE Type;
    union
    {
        uint32_t MaxValue;
        struct
        {
            uint32_t ConfigRegsiter0;
            uint32_t ConfigRegsiter1;
            uint32_t ConfigRegsiter2;
            uint32_t ConfigRegsiter3;
        };
    };
} CM_SET_CAPS, *PCM_SET_CAPS;

typedef struct _CM_HAL_EXEC_GROUPED_TASK_PARAM
{
    void    **pKernels;              // [in]  Array of Kernel data
    uint32_t *piKernelSizes;          // [in]  Parallel array of Kernel Size
    uint32_t iNumKernels;            // [in]  Number of Kernels in a task
    int32_t iTaskIdOut;             // [out] Task ID 
    uint32_t threadSpaceWidth;       // [in]  thread space width within group
    uint32_t threadSpaceHeight;      // [in]  thread space height within group
    uint32_t groupSpaceWidth;        // [in]  group space width
    uint32_t groupSpaceHeight;       // [in]  group space height
    uint32_t iSLMSize;               // [in]  SLM size per thread group in 1K unit
} CM_HAL_EXEC_GROUPED_TASK_PARAM, *PCM_HAL_EXEC_GROUPED_TASK_PARAM;

// Need to consistant with compiler
typedef enum _CM_ARG_KIND
{
    // compiler-defined kind
    ARG_KIND_GENERAL = 0x0,
    ARG_KIND_SAMPLER = 0x1,
    //ARG_KIND_SURFACE = 0x2, compiler value for surface 
    // runtime classify further surface to 1D/2D/3D
    ARG_KIND_SURFACE_2D = 0x2,
    ARG_KIND_SURFACE_1D = 0x3,
    ARG_KIND_SURFACE_3D = 0x4,
    ARG_KIND_SURFACE_VME = 0x5,
    ARG_KIND_VME_INDEX = 0x6,
    ARG_KIND_SURFACE_2D_UP = 0x7,
    ARG_KIND_SURFACE_SAMPLER8X8_AVS = 0x8,
    ARG_KIND_SURFACE_SAMPLER8X8_VA = 0x9, //get compiler update before checking this in
    ARG_KIND_SURFACE_SAMPLER = 0xb,
    ARG_KIND_SURFACE = 0xc,
    ARG_KIND_SURFACE2DUP_SAMPLER = 0xd,
    ARG_KIND_IMPLICT_LOCALSIZE = 0xe,
    ARG_KIND_IMPLICT_GROUPSIZE = 0xf,
    ARG_KIND_IMPLICIT_LOCALID = 0x10,
    ARG_KIND_STATE_BUFFER = 0x11,
    ARG_KIND_GENERAL_DEPVEC = 0x20,
    ARG_KIND_SURFACE_2D_SCOREBOARD = 0x2A  //used for SW scoreboarding
} CM_ARG_KIND;

typedef enum _SURFACE_KIND
{
    DATA_PORT_SURF,
    SAMPLER_SURF,
} SURFACE_KIND;

typedef struct _SURFACE_ARRAY_ARG
{
    uint16_t argKindForArray; //record each arg kind in array, used for surface array
    uint32_t addressModeForArray; // record each arg address control mode for media sampler in surface array

}SURFACE_ARRAY_ARG;

typedef struct _CM_ARG
{
    uint16_t unitKind; // value is of type CM_ARG_KIND
    uint16_t unitKindOrig; // used to restore unitKind when reset

    uint16_t index;
    SURFACE_KIND s_k;
    
    uint32_t unitCount; // 1 for for per kernel arg ; thread # for per thread arg

    uint16_t unitSize; // size of arg in byte
    uint16_t unitSizeOrig; // used to restore unitSize when reset

    uint16_t unitOffsetInPayload; // offset relative to R0 in payload
    uint16_t unitOffsetInPayloadOrig; // used to restore unitOffsetInPayload in adding move instruction for CURBE
    bool bIsDirty;      // used to indicate if its value be changed
    bool bIsSet;        // used to indicate if this argument is set correctly
    uint32_t nCustomValue;  // CM defined value for special argument kind

    uint32_t aliasIndex;    // CmSurface2D alias index
    bool bAliasCreated; // whether or not alias was created for this argument

    bool bIsNull;       // used to indicate if this is a null surface

    uint32_t unitVmeArraySize; // number of Vme surfaces in surface array 

    // pointer to the arg values. the size is unitCount * unitSize
    union
    {
        uint8_t *pValue; 
        int32_t *pIValue;
        uint32_t *pUIValue;
        float  *pFValue; 
    };

    uint16_t *surfIndex;
    SURFACE_ARRAY_ARG *pSurfArrayArg; // record each arg kind and address control mode for media sampler in surface array
    _CM_ARG()
    {
        unitKind = 0;
        unitCount = 0;
        unitSize = 0;
        unitOffsetInPayload = 0;
        pValue = nullptr;
        bIsDirty = false;
        bIsNull = false;
        unitVmeArraySize = 0;
    }
} CM_ARG ;

#define  CM_JIT_FLAG_SIZE                          256 
#define  CM_JIT_ERROR_MESSAGE_SIZE                 512  
#define  CM_JIT_PROF_INFO_SIZE                     4096

typedef struct {
    unsigned short name_index;
    unsigned char size;
    unsigned char* values;
    char *name;
} attribute_info_t;

typedef struct {
    unsigned short name_index;
    unsigned char bit_properties;
    unsigned short num_elements;
    unsigned short alias_index;
    unsigned short alias_offset;
    unsigned char attribute_count;
    attribute_info_t* attributes;
} gen_var_info_t;

typedef struct {
    unsigned short name_index;
    unsigned short num_elements;
    unsigned char attribute_count;
    attribute_info_t* attributes;
} spec_var_info_t;

typedef struct {
    unsigned short name_index;
    unsigned char kind;
    unsigned char attribute_count;
    attribute_info_t* attributes;
} label_info_t;


typedef struct _CM_KERNEL_INFO
{
    char kernelName[ CM_MAX_KERNEL_NAME_SIZE_IN_BYTE ];
    uint32_t inputCountOffset;

    //Used to store the input for the jitter from CISA
    uint32_t kernelIsaOffset;
    uint32_t kernelIsaSize;

    //Binary Size
    union
    {
        uint32_t jitBinarySize;
        uint32_t genxBinarySize;
    };

    union
    {
        void* jitBinaryCode;   //pointer to code created by jitter
        uint32_t genxBinaryOffset; //pointer to binary offset in CISA (use when jit is not enabled)
    };

    //Just a copy for original binary pointer and size (GTPin using only)
    void* pOrigBinary;
    uint32_t uiOrigBinarySize; 

    uint32_t globalStringCount;
    const char** globalStrings; 
    char kernelASMName[CM_MAX_KERNEL_NAME_SIZE_IN_BYTE + 1];        //The name of the Gen assembly file for this kernel (no extension)
    uint8_t kernelSLMSize;     //Size of the SLM used by each thread group
    bool blNoBarrier;       //Indicate if the barrier is used in kernel: true means no barrier used, false means barrier is used.

    FINALIZER_INFO *jitInfo;
    
    uint32_t variable_count;
    gen_var_info_t *variables;
    uint32_t address_count;
    spec_var_info_t *address;
    uint32_t predicte_count;
    spec_var_info_t *predictes;
    uint32_t label_count;
    label_info_t *label;
    uint32_t sampler_count;
    spec_var_info_t *sampler;
    uint32_t surface_count;
    spec_var_info_t *surface;
    uint32_t vme_count;
    spec_var_info_t *vme;

    uint32_t kernelInfoRefCount;    //reference counter for kernel info to reuse kernel info and jitbinary 
} CM_KERNEL_INFO ;


//GT-PIN
#define CM_MAX_ENTRY_FOR_A_SURFACE  6   //maxium planes(3)*dual state(2)
#define CM_GTPIN_BUFFER_NUM 3


typedef struct _CM_ARG_64
{
    void * pValue;
    int size;
}CM_ARG_64;

//Sampler8x8 data structures

typedef enum _CM_MESSAGE_SEQUENCE_
{
    CM_MS_1x1 = 0,
    CM_MS_16x1 = 1,
    CM_MS_16x4 = 2,
    CM_MS_32x1 = 3,
    CM_MS_32x4 = 4,
    CM_MS_64x1 = 5,
    CM_MS_64x4 = 6
}CM_MESSAGE_SEQUENCE;

typedef enum _CM_MIN_MAX_FILTER_CONTROL_
{
    CM_MIN_FILTER = 0,
    CM_MAX_FILTER = 1,
    CM_BOTH_FILTER = 3
}CM_MIN_MAX_FILTER_CONTROL;

typedef enum _CM_VA_FUNCTION_
{
    CM_VA_MINMAXFILTER = 0,
    CM_VA_DILATE = 1,
    CM_VA_ERODE = 2
} CM_VA_FUNCTION;

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







//Function pointer definition for jitter compilation functions.
typedef int (__cdecl *pJITCompile)( const char* kernelName, 
                const void* kernelIsa, 
                uint32_t kernelIsaSize,
                void* &genBinary,
                uint32_t& genBinarySize,
                const char* platform,
                int majorVersion,
                int minorVersion,
                int numArgs,
                const char* args[],
                char* errorMsg,
                FINALIZER_INFO* jitInfo);
typedef void (__cdecl *pFreeBlock)(void* );
typedef void (__cdecl *pJITVersion)(unsigned int& majorV, 
                                    unsigned int& minorV);

#define JITCOMPILE_FUNCTION_STR   "JITCompile"
#define FREEBLOCK_FUNCTION_STR    "freeBlock"
#define JITVERSION_FUNCTION_STR     "getJITVersion"

#define USING_IGC_DLL             1
#ifdef USING_IGC_DLL
#define JITTER_DLL_NAME_64BIT     "igc64.dll"
#define JITTER_DLL_NAME_32BIT     "igc32.dll"
#else
#define JITTER_DLL_NAME_64BIT     "igfxcmjit64.dll"
#define JITTER_DLL_NAME_32BIT     "igfxcmjit32.dll"
#endif

typedef enum _JITDLL_FUNCTION_ORDINAL_
{
  JITDLL_ORDINAL_JITCOMPILE = 1, 
  JITDLL_ORDINAL_JITCOMPILESIM = 2,
  JITDLL_ORDINAL_FREEBLOCK = 3,
  JITDLL_ORDINAL_JITVERSION = 4
} JITDLL_FUNCTION_ORDINAL;

//non-depend on rtti::dynamic_cast
typedef enum _CM_ENUM_CLASS_TYPE
{
    CM_ENUM_CLASS_TYPE_CMBUFFER_RT          = 0,
    CM_ENUM_CLASS_TYPE_CMSURFACE2D          = 1,
    CM_ENUM_CLASS_TYPE_CMSURFACE2DUP        = 2, 
    CM_ENUM_CLASS_TYPE_CMSURFACE3D          = 3, 
    CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER     = 4,
    CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8  = 5, 
    CM_ENUM_CLASS_TYPE_CMSURFACEVME         = 6, 
    CM_ENUM_CLASS_TYPE_CMSAMPLER_RT         = 7,
    CM_ENUM_CLASS_TYPE_CMSAMPLER8X8STATE_RT = 8,
    CM_ENUM_CLASS_TYPE_CM_STATE_BUFFER      = 9
}CM_ENUM_CLASS_TYPE;

using namespace CMRT_UMD;

typedef enum _CM_KERNEL_INTERNAL_ARG_TYPE
{
    CM_KERNEL_INTERNEL_ARG_PERKERNEL = 0,
    CM_KERNEL_INTERNEL_ARG_PERTHREAD = 1
} CM_KERNEL_INTERNAL_ARG_TYPE, *PCM_KERNEL_INTERNAL_ARG_TYPE;

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
    uint32_t surface_x_offset;
    uint32_t surface_y_offset;
    uint32_t reserved[4]; // for future usage
} CM_SURFACE2D_STATE_PARAM;
