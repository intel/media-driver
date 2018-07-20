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
//! \file      cm_rt.h
//! \brief     Contains all exposed APIs and Definitions for CM
//!

#ifndef __CM_RT_H__
#define __CM_RT_H__

//**********************************************************************
// Version
//**********************************************************************
#ifndef CM_1_0
#define CM_1_0          100
#endif 
#ifndef CM_2_0
#define CM_2_0          200
#endif
#ifndef CM_2_1
#define CM_2_1          201
#endif
#ifndef CM_2_2
#define CM_2_2          202
#endif
#ifndef CM_2_3
#define CM_2_3          203
#endif
#ifndef CM_2_4
#define CM_2_4          204
#endif
#ifndef CM_3_0
#define CM_3_0          300
#endif
#ifndef CM_4_0
#define CM_4_0          400
#endif
#ifndef CM_5_0
#define CM_5_0          500
#endif
#ifndef CM_6_0
#define CM_6_0          600
#endif
#ifndef CM_7_0
#define CM_7_0          700
#endif
#ifndef CM_7_2
#define CM_7_2          702 //for MDFRT API refreshment.
#endif

//Legacy MACRO, will be removed later
#ifndef __INTEL_MDF 
#define __INTEL_MDF     (CM_7_2)
#endif

//New MACRO
#ifndef __INTEL_CM
#define __INTEL_CM      (CM_7_2)
#endif

//**********************************************************************
// external headers
//**********************************************************************
#include <stdint.h>

//**********************************************************************
// Macros
//**********************************************************************
#ifdef __cplusplus
#define EXTERN_C     extern "C"
#else
#define EXTERN_C
#endif
#define CM_RT_API
#define CM_KERNEL_FUNCTION(...) CM_KERNEL_FUNCTION2(__VA_ARGS__)

#define _NAME_MERGE_(x, y)                      x ## y
#define _NAME_LABEL_(name, id)                  _NAME_MERGE_(name, id)
#define __CODEGEN_UNIQUE(name)                  _NAME_LABEL_(name, __LINE__)
#define BITFIELD_RANGE( startbit, endbit )     ((endbit)-(startbit)+1)
#define BITFIELD_BIT(bit)                        1

#define CM_MIN_SURF_WIDTH   1
#define CM_MIN_SURF_HEIGHT  1
#define CM_MIN_SURF_DEPTH   2

#define CM_MAX_1D_SURF_WIDTH  0X40000000 // 2^30

#define CM_MAX_2D_SURF_WIDTH    16384
#define CM_MAX_2D_SURF_HEIGHT   16384

#define CM_MAX_3D_SURF_WIDTH    2048
#define CM_MAX_3D_SURF_HEIGHT   2048
#define CM_MAX_3D_SURF_DEPTH    2048

#define CM_MAX_OPTION_SIZE_IN_BYTE          512
#define CM_MAX_KERNEL_NAME_SIZE_IN_BYTE     256
#define CM_MAX_ISA_FILE_NAME_SIZE_IN_BYTE   256

#define CM_MAX_THREADSPACE_WIDTH_FOR_MW        511
#define CM_MAX_THREADSPACE_HEIGHT_FOR_MW       511
#define CM_MAX_THREADSPACE_WIDTH_FOR_MO        512
#define CM_MAX_THREADSPACE_HEIGHT_FOR_MO       512
#define CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW  2047
#define CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW 2047


//Time in seconds before kernel should timeout
#define CM_MAX_TIMEOUT 2
//Time in milliseconds before kernel should timeout
#define CM_MAX_TIMEOUT_MS CM_MAX_TIMEOUT*1000

#define CM_NO_EVENT  ((CmEvent *)(-1))  //NO Event

// Cm Device Create Option
#define CM_DEVICE_CREATE_OPTION_DEFAULT                     0
#define CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE       1
#define CM_DEVICE_CREATE_OPTION_TDR_DISABLE                 64

#define CM_DEVICE_CONFIG_DISABLE_TASKFLUSHEDSEMAPHORE_OFFSET  6
#define CM_DEVICE_CONFIG_DISABLE_TASKFLUSHEDSEMAPHORE_MASK   (1<<CM_DEVICE_CONFIG_DISABLE_TASKFLUSHEDSEMAPHORE_OFFSET)
#define CM_DEVICE_CREATE_OPTION_TASKFLUSHEDSEMAPHORE_DISABLE  1   //to disable the semaphore for task flushed
#define CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_OFFSET           22
#define CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_DISENABLE         (1 << CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_OFFSET)    
#define CM_DEVICE_CONFIG_KERNEL_DEBUG_OFFSET                  23
#define CM_DEVICE_CONFIG_KERNEL_DEBUG_ENABLE               (1 << CM_DEVICE_CONFIG_KERNEL_DEBUG_OFFSET)

#define CM_MAX_DEPENDENCY_COUNT         8
#define CM_NUM_DWORD_FOR_MW_PARAM       16

#define CM_NUM_COEFF_ROWS 17
#define CM_NUM_COEFF_ROWS_SKL 32

#define CM_NUM_CONVOLVE_ROWS 16
#define CM_NUM_CONVOLVE_ROWS_SKL 32

#define    CM_CHROMA_SITING_NONE           0,
#define    CM_CHROMA_SITING_HORZ_LEFT      1 << 0
#define    CM_CHROMA_SITING_HORZ_CENTER    1 << 1
#define    CM_CHROMA_SITING_HORZ_RIGHT     1 << 2
#define    CM_CHROMA_SITING_VERT_TOP       1 << 4
#define    CM_CHROMA_SITING_VERT_CENTER    1 << 5
#define    CM_CHROMA_SITING_VERT_BOTTOM    1 << 6

#define CM_NULL_SURFACE                     0xFFFF

#define  CM_FUSED_EU_DISABLE                 0
#define  CM_FUSED_EU_ENABLE                  1
#define  CM_FUSED_EU_DEFAULT                 CM_FUSED_EU_DISABLE

#define  CM_TURBO_BOOST_DISABLE               0
#define  CM_TURBO_BOOST_ENABLE                1
#define  CM_TURBO_BOOST_DEFAULT              CM_TURBO_BOOST_ENABLE

#define CM_CALLBACK __cdecl

// SVM buffer access flags definition
#define CM_SVM_ACCESS_FLAG_COARSE_GRAINED    (0)                                 //Coarse-grained SVM buffer, IA/GT cache coherency disabled
#define CM_SVM_ACCESS_FLAG_FINE_GRAINED      (1 << 0)                            //Fine-grained SVM buffer, IA/GT cache coherency enabled
#define CM_SVM_ACCESS_FLAG_ATOMICS           (1 << 1)                            //Crosse IA/GT atomics supported SVM buffer, need CM_SVM_ACCESS_FLAG_FINE_GRAINED flag is set as well
#define CM_SVM_ACCESS_FLAG_DEFAULT           CM_SVM_ACCESS_FLAG_COARSE_GRAINED   //default is coarse-grained SVM buffer

//**********************************************************************
// OS-specific includings and types
//**********************************************************************
#include "cm_rt_def_os.h"

//**********************************************************************
// Enumerations
//**********************************************************************
typedef enum _CM_RETURN_CODE
{
    CM_SUCCESS                                  = 0,
    /*
     * RANGE -1 ~ -9999 FOR EXTERNAL ERROR CODE
     */
    CM_FAILURE                                  = -1,
    CM_NOT_IMPLEMENTED                          = -2,
    CM_SURFACE_ALLOCATION_FAILURE               = -3,
    CM_OUT_OF_HOST_MEMORY                       = -4,
    CM_SURFACE_FORMAT_NOT_SUPPORTED             = -5,
    CM_EXCEED_SURFACE_AMOUNT                    = -6,
    CM_EXCEED_KERNEL_ARG_AMOUNT                 = -7,
    CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE           = -8,
    CM_INVALID_ARG_INDEX                        = -9,
    CM_INVALID_ARG_VALUE                        = -10,
    CM_INVALID_ARG_SIZE                         = -11,
    CM_INVALID_THREAD_INDEX                     = -12,
    CM_INVALID_WIDTH                            = -13,
    CM_INVALID_HEIGHT                           = -14,
    CM_INVALID_DEPTH                            = -15,
    CM_INVALID_COMMON_ISA                       = -16,
    CM_OPEN_VIDEO_DEVICE_FAILURE                = -17,
    CM_VIDEO_DEVICE_LOCKED                      = -18,  // Video device is already locked.
    CM_LOCK_VIDEO_DEVICE_FAILURE                = -19,  // Video device is not locked but can't be locked.
    CM_EXCEED_SAMPLER_AMOUNT                    = -20,
    CM_EXCEED_MAX_KERNEL_PER_ENQUEUE            = -21,
    CM_EXCEED_MAX_KERNEL_SIZE_IN_BYTE           = -22,
    CM_EXCEED_MAX_THREAD_AMOUNT_PER_ENQUEUE     = -23,
    CM_EXCEED_VME_STATE_G6_AMOUNT               = -24,
    CM_INVALID_THREAD_SPACE                     = -25,
    CM_EXCEED_MAX_TIMEOUT                       = -26,
    CM_JITDLL_LOAD_FAILURE                      = -27,
    CM_JIT_COMPILE_FAILURE                      = -28,
    CM_JIT_COMPILESIM_FAILURE                   = -29,
    CM_INVALID_THREAD_GROUP_SPACE               = -30,
    CM_THREAD_ARG_NOT_ALLOWED                   = -31,
    CM_INVALID_GLOBAL_BUFFER_INDEX              = -32,
    CM_INVALID_BUFFER_HANDLER                   = -33,
    CM_EXCEED_MAX_SLM_SIZE                      = -34,
    CM_JITDLL_OLDER_THAN_ISA                    = -35,
    CM_INVALID_HARDWARE_THREAD_NUMBER           = -36,
    CM_GTPIN_INVOKE_FAILURE                     = -37,
    CM_INVALIDE_L3_CONFIGURATION                = -38,
    CM_INVALID_TEXTURE2D_USAGE                  = -39,
    CM_INTEL_GFX_NOTFOUND                       = -40,
    CM_GPUCOPY_INVALID_SYSMEM                   = -41,
    CM_GPUCOPY_INVALID_WIDTH                    = -42,
    CM_GPUCOPY_INVALID_STRIDE                   = -43,
    CM_EVENT_DRIVEN_FAILURE                     = -44,
    CM_LOCK_SURFACE_FAIL                        = -45, // Lock surface failed
    CM_INVALID_GENX_BINARY                      = -46,
    CM_FEATURE_NOT_SUPPORTED_IN_DRIVER          = -47, // driver out-of-sync
    CM_QUERY_DLL_VERSION_FAILURE                = -48, //Fail in getting DLL file version
    CM_KERNELPAYLOAD_PERTHREADARG_MUTEX_FAIL    = -49,
    CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL    = -50,
    CM_KERNELPAYLOAD_SETTING_FAILURE            = -51,
    CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX    = -52, 
    CM_NOT_SET_KERNEL_ARGUMENT                  = -53,
    CM_GPUCOPY_INVALID_SURFACES                 = -54,
    CM_GPUCOPY_INVALID_SIZE                     = -55,
    CM_GPUCOPY_OUT_OF_RESOURCE                  = -56,
    CM_INVALID_VIDEO_DEVICE                     = -57,
    CM_SURFACE_DELAY_DESTROY                    = -58,
    CM_INVALID_VEBOX_STATE                      = -59,
    CM_INVALID_VEBOX_SURFACE                    = -60,
    CM_FEATURE_NOT_SUPPORTED_BY_HARDWARE        = -61,
    CM_RESOURCE_USAGE_NOT_SUPPORT_READWRITE     = -62,
    CM_MULTIPLE_MIPLEVELS_NOT_SUPPORTED         = -63,
    CM_INVALID_UMD_CONTEXT                      = -64,
    CM_INVALID_LIBVA_SURFACE                    = -65,
    CM_INVALID_LIBVA_INITIALIZE                 = -66,
    CM_KERNEL_THREADSPACE_NOT_SET               = -67,
    CM_INVALID_KERNEL_THREADSPACE               = -68,
    CM_KERNEL_THREADSPACE_THREADS_NOT_ASSOCIATED= -69,
    CM_KERNEL_THREADSPACE_INTEGRITY_FAILED      = -70,
    CM_INVALID_USERPROVIDED_GENBINARY           = -71,
    CM_INVALID_PRIVATE_DATA                     = -72,
    CM_INVALID_MOS_RESOURCE_HANDLE              = -73,
    CM_SURFACE_CACHED                           = -74,
    CM_SURFACE_IN_USE                           = -75,
    CM_INVALID_GPUCOPY_KERNEL                   = -76,
    CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN  = -77,
    CM_INVALID_MEDIA_WALKING_PATTERN            = -78,
    CM_FAILED_TO_ALLOCATE_SVM_BUFFER            = -79,
    CM_EXCEED_MAX_POWER_OPTION_FOR_PLATFORM     = -80,
    CM_INVALID_KERNEL_THREADGROUPSPACE          = -81,
    CM_INVALID_KERNEL_SPILL_CODE                = -82,
    CM_UMD_DRIVER_NOT_SUPPORTED                 = -83,
    CM_INVALID_GPU_FREQUENCY_VALUE              = -84,
    CM_SYSTEM_MEMORY_NOT_4KPAGE_ALIGNED         = -85,
    CM_KERNEL_ARG_SETTING_FAILED                = -86,
    CM_NO_AVAILABLE_SURFACE                     = -87,
    CM_VA_SURFACE_NOT_SUPPORTED                 = -88,
    CM_TOO_MUCH_THREADS                         = -89,
    CM_NULL_POINTER                             = -90,
    CM_EXCEED_MAX_NUM_2D_ALIASES                = -91,
    CM_INVALID_PARAM_SIZE                       = -92,
    CM_GT_UNSUPPORTED                           = -93,
    CM_GTPIN_FLAG_NO_LONGER_SUPPORTED           = -94,
    CM_PLATFORM_UNSUPPORTED_FOR_API             = -95,
    CM_TASK_MEDIA_RESET                         = -96,
    CM_KERNELPAYLOAD_SAMPLER_INVALID_BTINDEX    = -97,
    CM_EXCEED_MAX_NUM_BUFFER_ALIASES            = -98,
    CM_SYSTEM_MEMORY_NOT_4PIXELS_ALIGNED        = -99,
    CM_FAILED_TO_CREATE_CURBE_SURFACE           = -100,
    CM_INVALID_CAP_NAME                         = -101,
    CM_INVALID_PARAM_FOR_CREATE_QUEUE_EX        = -102,

    /*
     * RANGE <=-10000 FOR INTERNAL ERROR CODE
     */
    CM_INTERNAL_ERROR_CODE_OFFSET               = -10000,
} CM_RETURN_CODE;

typedef enum _CM_STATUS
{
    CM_STATUS_QUEUED         = 0,
    CM_STATUS_FLUSHED        = 1,
    CM_STATUS_FINISHED       = 2,
    CM_STATUS_STARTED        = 3,
    CM_STATUS_RESET          = 4

} CM_STATUS;

typedef enum _CM_PIXEL_TYPE
{
    CM_PIXEL_UINT,
    CM_PIXEL_SINT,
    CM_PIXEL_OTHER
} CM_PIXEL_TYPE;

enum GPU_PLATFORM{
    PLATFORM_INTEL_UNKNOWN     = 0,
    PLATFORM_INTEL_SNB         = 1,
    PLATFORM_INTEL_IVB         = 2,
    PLATFORM_INTEL_HSW         = 3,
    PLATFORM_INTEL_BDW         = 4,
    PLATFORM_INTEL_VLV         = 5,
    PLATFORM_INTEL_CHV         = 6,
    PLATFORM_INTEL_SKL         = 7,
    PLATFORM_INTEL_BXT         = 8,
    PLATFORM_INTEL_CNL         = 9,
    PLATFORM_INTEL_KBL         = 11,
    PLATFORM_INTEL_GLK         = 16,
    PLATFORM_INTEL_CFL         = 17,
};

enum GPU_GT_PLATFORM{
    PLATFORM_INTEL_GT_UNKNOWN  = 0,
    PLATFORM_INTEL_GT1         = 1,
    PLATFORM_INTEL_GT2         = 2,
    PLATFORM_INTEL_GT3         = 3,
    PLATFORM_INTEL_GT4         = 4,
    PLATFORM_INTEL_GT1_5       = 10
};

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

typedef enum _CM_FASTCOPY_OPTION
{
    CM_FASTCOPY_OPTION_NONBLOCKING  = 0x00,
    CM_FASTCOPY_OPTION_BLOCKING     = 0x01
} CM_FASTCOPY_OPTION;

typedef enum _CM_DEPENDENCY_PATTERN
{
    CM_NONE_DEPENDENCY          = 0,    //All threads run parallel, scanline dispatch
    CM_WAVEFRONT                = 1,
    CM_WAVEFRONT26              = 2,
    CM_VERTICAL_WAVE            = 3,
    CM_HORIZONTAL_WAVE          = 4,
    CM_WAVEFRONT26Z             = 5,
    CM_WAVEFRONT26X             = 6,
    CM_WAVEFRONT26ZIG           = 7,
    CM_WAVEFRONT26ZI            = 8
} CM_DEPENDENCY_PATTERN;

typedef enum _CM_WALKING_PATTERN
{
    CM_WALK_DEFAULT      = 0,
    CM_WALK_WAVEFRONT    = 1,
    CM_WALK_WAVEFRONT26  = 2,
    CM_WALK_VERTICAL     = 3,
    CM_WALK_HORIZONTAL   = 4,
    CM_WALK_WAVEFRONT26X = 5,
    CM_WALK_WAVEFRONT26ZIG = 6,
    CM_WALK_WAVEFRONT45D = 7,
    CM_WALK_WAVEFRONT45XD_2 = 8
} CM_WALKING_PATTERN;

typedef enum _CM_26ZI_DISPATCH_PATTERN
{
    VVERTICAL_HVERTICAL_26           = 0,
    VVERTICAL_HHORIZONTAL_26         = 1,
    VVERTICAL26_HHORIZONTAL26        = 2,
    VVERTICAL1X26_HHORIZONTAL1X26    = 3
} CM_26ZI_DISPATCH_PATTERN;

typedef enum _CM_MW_GROUP_SELECT
{
    CM_MW_GROUP_NONE        = 0,
    CM_MW_GROUP_COLORLOOP   = 1,
    CM_MW_GROUP_INNERLOCAL  = 2,
    CM_MW_GROUP_MIDLOCAL    = 3,
    CM_MW_GROUP_OUTERLOCAL  = 4,
    CM_MW_GROUP_INNERGLOBAL = 5,
} CM_MW_GROUP_SELECT;

/**************** L3/Cache ***************/
enum MEMORY_OBJECT_CONTROL
{
    MEMORY_OBJECT_CONTROL_SKL_DEFAULT = 0,
    MEMORY_OBJECT_CONTROL_SKL_NO_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC,
    MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_ELLC_L3,
    MEMORY_OBJECT_CONTROL_SKL_NO_CACHE,
    MEMORY_OBJECT_CONTROL_SKL_COUNT,

    MEMORY_OBJECT_CONTROL_CNL_DEFAULT = 0,
    MEMORY_OBJECT_CONTROL_CNL_NO_L3,
    MEMORY_OBJECT_CONTROL_CNL_NO_LLC_ELLC,
    MEMORY_OBJECT_CONTROL_CNL_NO_LLC,
    MEMORY_OBJECT_CONTROL_CNL_NO_ELLC,
    MEMORY_OBJECT_CONTROL_CNL_NO_LLC_L3,
    MEMORY_OBJECT_CONTROL_CNL_NO_ELLC_L3,
    MEMORY_OBJECT_CONTROL_CNL_NO_CACHE,
    MEMORY_OBJECT_CONTROL_CNL_COUNT,

    MEMORY_OBJECT_CONTROL_UNKNOWN = 0xff
};

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

typedef enum _L3_SUGGEST_CONFIG
{
   CM_L3_PLANE_DEFAULT = 0,
   CM_L3_PLANE_1,
   CM_L3_PLANE_2,
   CM_L3_PLANE_3,
   CM_L3_PLANE_4,
   CM_L3_PLANE_5,
   CM_L3_PLANE_6,
   CM_L3_PLANE_7,
   CM_L3_PLANE_8,
} L3_SUGGEST_CONFIG;

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

typedef enum _CM_MESSAGE_SEQUENCE_
{
    CM_MS_1x1   = 0,
    CM_MS_16x1  = 1,
    CM_MS_16x4  = 2,
    CM_MS_32x1  = 3,
    CM_MS_32x4  = 4,
    CM_MS_64x1  = 5,
    CM_MS_64x4  = 6
}CM_MESSAGE_SEQUENCE;

typedef enum _CM_MIN_MAX_FILTER_CONTROL_
{
    CM_MIN_FILTER   = 0,
    CM_MAX_FILTER   = 1,
    CM_BOTH_FILTER  = 3
}CM_MIN_MAX_FILTER_CONTROL;

typedef enum _CM_VA_FUNCTION_
{
    CM_VA_MINMAXFILTER  = 0,
    CM_VA_DILATE        = 1,
    CM_VA_ERODE         = 2
} CM_VA_FUNCTION;

typedef enum _CM_EVENT_PROFILING_INFO
{
    CM_EVENT_PROFILING_HWSTART,
    CM_EVENT_PROFILING_HWEND,
    CM_EVENT_PROFILING_SUBMIT,
    CM_EVENT_PROFILING_COMPLETE,
    CM_EVENT_PROFILING_KERNELCOUNT,
    CM_EVENT_PROFILING_KERNELNAMES,
    CM_EVENT_PROFILING_THREADSPACE
}CM_EVENT_PROFILING_INFO;

// CM Convolve type for SKL+
typedef enum _CM_CONVOLVE_SKL_TYPE
{
    CM_CONVOLVE_SKL_TYPE_2D = 0,
    CM_CONVOLVE_SKL_TYPE_1D = 1,
    CM_CONVOLVE_SKL_TYPE_1P = 2
} CM_CONVOLVE_SKL_TYPE;

typedef enum _CM_ROTATION
{
    CM_ROTATION_IDENTITY = 0,      //!< Rotation 0 degrees
    CM_ROTATION_90,                //!< Rotation 90 degrees
    CM_ROTATION_180,               //!< Rotation 180 degrees
    CM_ROTATION_270,               //!< Rotation 270 degrees
} CM_ROTATION;

// to define frame type for interlace frame support 
typedef enum _CM_FRAME_TYPE 
{ 
    CM_FRAME,     // singe frame, not interlaced 
    CM_TOP_FIELD,  
    CM_BOTTOM_FIELD, 
    MAX_FRAME_TYPE 
} CM_FRAME_TYPE; 

enum CM_QUEUE_TYPE
{
    CM_QUEUE_TYPE_NONE = 0,
    CM_QUEUE_TYPE_RENDER = 1,
    CM_QUEUE_TYPE_COMPUTE = 2,
    CM_QUEUE_TYPE_VEBOX = 3
};

//**********************************************************************
// Structures
//**********************************************************************
typedef struct _CM_SAMPLER_STATE
{
    CM_TEXTURE_FILTER_TYPE minFilterType;
    CM_TEXTURE_FILTER_TYPE magFilterType;   
    CM_TEXTURE_ADDRESS_TYPE addressU;   
    CM_TEXTURE_ADDRESS_TYPE addressV;   
    CM_TEXTURE_ADDRESS_TYPE addressW; 
} CM_SAMPLER_STATE;

typedef struct _CM_SAMPLER_STATE_EX
{
    CM_TEXTURE_FILTER_TYPE minFilterType;
    CM_TEXTURE_FILTER_TYPE magFilterType;   
    CM_TEXTURE_ADDRESS_TYPE addressU;   
    CM_TEXTURE_ADDRESS_TYPE addressV;   
    CM_TEXTURE_ADDRESS_TYPE addressW;

    CM_PIXEL_TYPE SurfaceFormat;
    union {
        DWORD BorderColorRedU;
        INT BorderColorRedS;
        FLOAT BorderColorRedF;
    };

    union {
        DWORD BorderColorGreenU;
        INT BorderColorGreenS;
        FLOAT BorderColorGreenF;
    };

    union {
        DWORD BorderColorBlueU;
        INT BorderColorBlueS;
        FLOAT BorderColorBlueF;
    };
    
    union {
        DWORD BorderColorAlphaU;
        INT BorderColorAlphaS;
        FLOAT BorderColorAlphaF;
    };
} CM_SAMPLER_STATE_EX;

typedef struct _CM_PLATFORM_INFO
{
    UINT numSlices;
    UINT numSubSlices;
    UINT numEUsPerSubSlice;
    UINT numHWThreadsPerEU;
    UINT numMaxEUsPerPool;
}CM_PLATFORM_INFO, *PCM_PLATFORM_INFO;

// CM RT DLL File Version
typedef struct _CM_DLL_FILE_VERSION
{
    WORD    wMANVERSION;
    WORD    wMANREVISION;
    WORD    wSUBREVISION;
    WORD    wBUILD_NUMBER; 
    //Version constructed as : "wMANVERSION.wMANREVISION.wSUBREVISION.wBUILD_NUMBER"
} CM_DLL_FILE_VERSION, *PCM_DLL_FILE_VERSION;

// parameters used to set the surface state of the CmSurface
struct CM_VME_SURFACE_STATE_PARAM
{
    UINT    width;
    UINT    height;
};

typedef struct _CM_WALKING_PARAMETERS
{
    DWORD Value[CM_NUM_DWORD_FOR_MW_PARAM];
} CM_WALKING_PARAMETERS, *PCM_WALKING_PARAMETERS;

typedef struct _CM_DEPENDENCY
{
    UINT    count;
    INT     deltaX[CM_MAX_DEPENDENCY_COUNT];
    INT     deltaY[CM_MAX_DEPENDENCY_COUNT];
}CM_DEPENDENCY;

typedef struct _CM_COORDINATE
{
    INT x;
    INT y;
} CM_COORDINATE, *PCM_COORDINATE;

typedef struct _CM_THREAD_PARAM
{
    CM_COORDINATE   scoreboardCoordinates;
    BYTE            scoreboardColor;
    BYTE            sliceDestinationSelect;
    BYTE            subSliceDestinationSelect;
} CM_THREAD_PARAM, *PCM_THREAD_PARAM;

struct L3ConfigRegisterValues
{
    unsigned int config_register0;
    unsigned int config_register1;
    unsigned int config_register2;
    unsigned int config_register3;
};

//GT-PIN
typedef struct _CM_SURFACE_DETAILS{
    UINT        width; 
    UINT        height; 
    UINT        depth; 
    CM_SURFACE_FORMAT   format; 
    UINT        planeIndex;
    UINT        pitch; 
    UINT        slicePitch;
    UINT        SurfaceBaseAddress;
    UINT8       TiledSurface;
    UINT8       TileWalk;
    UINT        XOffset;
    UINT        YOffset; 
    
}CM_SURFACE_DETAILS;

/*
 *  AVS SAMPLER8x8 STATE
 */
typedef struct _CM_AVS_COEFF_TABLE{
    float   FilterCoeff_0_0;
    float   FilterCoeff_0_1;
    float   FilterCoeff_0_2;
    float   FilterCoeff_0_3;
    float   FilterCoeff_0_4;
    float   FilterCoeff_0_5;
    float   FilterCoeff_0_6;
    float   FilterCoeff_0_7;
}CM_AVS_COEFF_TABLE;

typedef struct _CM_AVS_INTERNEL_COEFF_TABLE{
    BYTE   FilterCoeff_0_0;
    BYTE   FilterCoeff_0_1;
    BYTE   FilterCoeff_0_2;
    BYTE   FilterCoeff_0_3;
    BYTE   FilterCoeff_0_4;
    BYTE   FilterCoeff_0_5;
    BYTE   FilterCoeff_0_6;
    BYTE   FilterCoeff_0_7;
}CM_AVS_INTERNEL_COEFF_TABLE;

typedef struct _CM_AVS_NONPIPLINED_STATE{
    bool BypassXAF;
    bool BypassYAF;
    BYTE DefaultSharpLvl;
    BYTE maxDerivative4Pixels;
    BYTE maxDerivative8Pixels;
    BYTE transitionArea4Pixels;
    BYTE transitionArea8Pixels;    
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
    BYTE DefaultSharpLvl;
    BYTE maxDerivative4Pixels;
    BYTE maxDerivative8Pixels;
    BYTE transitionArea4Pixels;
    BYTE transitionArea8Pixels;    
    CM_AVS_INTERNEL_COEFF_TABLE Tbl0X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_INTERNEL_COEFF_TABLE Tbl0Y[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_INTERNEL_COEFF_TABLE Tbl1X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_INTERNEL_COEFF_TABLE Tbl1Y[ CM_NUM_COEFF_ROWS_SKL ];
    bool bEnableRGBAdaptive;
    bool bAdaptiveFilterAllChannels;
}CM_AVS_INTERNEL_NONPIPLINED_STATE;

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

struct CM_AVS_STATE_MSG_EX {
  CM_AVS_STATE_MSG_EX();
  
  bool enable_all_channel_adaptive_filter;  // adaptive filter for all channels. validValues => [true..false]
  bool enable_rgb_adaptive_filter;          // adaptive filter for all channels. validValues => [true..false]
  bool enable_8_tap_adaptive_filter;        // enable 8-tap filter. validValues => [true..false]
  bool enable_uv_8_tap_filter;              // enable 8-tap filter on UV/RB channels. validValues => [true..false]
  bool writeback_format;                    // true sampleunorm, false standard. validValues => [true..false]
  bool writeback_mode;                      // true vsa, false ief. validValues => [true..false]
  BYTE state_selection;                     // 0=>first,1=>second scaler8x8 state. validValues => [0..1]

  // Image enhancement filter settings.
  bool enable_ief;        // image enhancement filter enable. validValues => [true..false]
  bool ief_type;          // true "basic" or false "advanced". validValues => [true..false]
  bool enable_ief_smooth; // true based on 3x3, false based on 5x5 validValues => [true..false]
  float r3c_coefficient;  // smoothing coeffient. Valid values => [0.0..0.96875]
  float r3x_coefficient;  // smoothing coeffient. Valid values => [0.0..0.96875]
  float r5c_coefficient;  // smoothing coeffient. validValues => [0.0..0.96875]
  float r5cx_coefficient; // smoothing coeffient. validValues => [0.0..0.96875]
  float r5x_coefficient;  // smoothing coeffient. validValues => [0.0..0.96875]

  // Edge processing settings.
  BYTE strong_edge_threshold; // validValues => [0..64]
  BYTE strong_edge_weight;    // Sharpening strength when a strong edge. validValues => [0..7]
  BYTE weak_edge_threshold;   // validValues => [0..64]
  BYTE regular_edge_weight;   // Sharpening strength when a weak edge. validValues => [0..7]
  BYTE non_edge_weight;       // Sharpening strength when no edge. validValues => [0..7]

  // Chroma key.
  bool enable_chroma_key; // Chroma keying be performed. validValues => [true..false]
  BYTE chroma_key_index;  // ChromaKey Table entry. validValues => [0..3]

  // Skin tone settings.
  bool enable_skin_tone;              // SkinToneTunedIEF_Enable. validValues => [true..false]
  bool enable_vy_skin_tone_detection; // Enables STD in the VY subspace. validValues => [true..false]
  bool skin_detail_factor;            // validValues => [true..false]
  BYTE skin_types_margin;             // validValues => [0..255]
  BYTE skin_types_threshold;          // validValues => [0..255]

  // Miscellaneous settings.
  BYTE gain_factor;             // validValues => [0..63]
  BYTE global_noise_estimation; // validValues => [0..255]
  bool mr_boost;                // validValues => [true..false]
  BYTE mr_smooth_threshold;     // validValues => [0..3]
  BYTE mr_threshold; 
  bool steepness_boost;         // validValues => [true..false]
  BYTE steepness_threshold;     // validValues => [0..15]
  bool texture_coordinate_mode; // true: clamp, false: mirror. validValues => [true..false]  
  BYTE max_hue;                 // Rectangle half width. validValued => [0..63]
  BYTE max_saturation;          // Rectangle half length. validValued => [0..63]
  int angles;                   // validValued => [0..360]
  BYTE diamond_margin ;         // validValues => [0..7]
  char diamond_du;              // Rhombus center shift in the sat-direction. validValues => [-64..63]
  char diamond_dv;              // Rhombus center shift in the hue-direction. validValues => [-64..63]
  float diamond_alpha;          // validValues => [0.0..4.0]
  BYTE diamond_threshold;       // validValues => [0..63]
  BYTE rectangle_margin;        // validValues => [0..7]
  BYTE rectangle_midpoint[2];   // validValues => [[0..255, 0..255]]
  float vy_inverse_margin[2];   // validValues => [[0.0..1.0, 0.0..1.0]]

  // Piecewise linear function settings.
  BYTE piecewise_linear_y_points[4];      // validValues => [[0..255, 0..255, 0..255, 0..255]]
  float piecewise_linear_y_slopes[2];     // validValues => [[-4.0...4.0, -4.0...4.0]]
  BYTE piecewise_linear_points_lower[4];  // validValues => [[0..255, 0..255, 0..255, 0..255]]
  BYTE piecewise_linear_points_upper[4];  // validValues => [[0..255, 0..255, 0..255, 0..255]]
  float piecewise_linear_slopes_lower[4]; // validValues => [[-4.0...4.0, -4.0...4.0, -4.0...4.0, -4.0...4.0]]
  float piecewise_linear_slopes_upper[4]; // validValues => [[-4.0...4.0, -4.0...4.0, -4.0...4.0, -4.0...4.0]]
  BYTE piecewise_linear_biases_lower[4];  // validValues => [[0..255, 0..255, 0..255, 0..255]]
  BYTE piecewise_linear_biases_upper[4];  // validValues => [[0..255, 0..255, 0..255, 0..255]]

  // AVS non-pipelined states.
  BYTE default_sharpness_level;   // default coefficient between smooth and sharp filtering. validValues => [0..255]
  bool enable_x_adaptive_filter;  // validValues => [true, false]
  bool enable_y_adaptive_filter;  // validValues => [true, false]
  BYTE max_derivative_4_pixels;   // lower boundary of the smooth 4 pixel area. validValues => [0..255]
  BYTE max_derivative_8_pixels;   // lower boundary of the smooth 8 pixel area. validValues => [0..255]
  BYTE transition_area_4_pixels;  // used in adaptive filtering to specify the width of the transition area for the 4 pixel calculation. validValues => [0..8]
  BYTE transition_area_8_pixels;  // Used in adaptive filtering to specify the width of the transition area for the 8 pixel calculation. validValues => [0..8]
  CM_AVS_COEFF_TABLE table0_x[CM_NUM_COEFF_ROWS_SKL];
  CM_AVS_COEFF_TABLE table0_y[CM_NUM_COEFF_ROWS_SKL];
  CM_AVS_COEFF_TABLE table1_x[CM_NUM_COEFF_ROWS_SKL];
  CM_AVS_COEFF_TABLE table1_y[CM_NUM_COEFF_ROWS_SKL];
};

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

typedef struct _CM_CONVOLVE_STATE_MSG{
    bool CoeffSize; //true 16-bit, false 8-bit
    byte SclDwnValue; //Scale down value
    byte Width; //Kernel Width
    byte Height; //Kernel Height   
    //SKL mode
    bool isVertical32Mode;
    bool isHorizontal32Mode;
    CM_CONVOLVE_SKL_TYPE nConvolveType;
    CM_CONVOLVE_COEFF_TABLE Table[CM_NUM_CONVOLVE_ROWS_SKL];
} CM_CONVOLVE_STATE_MSG;

/*
 *   MISC SAMPLER8x8 State
 */
typedef struct _CM_MISC_STATE {
    //DWORD 0
    union{
        struct{
            DWORD   Row0      : 16;
            DWORD   Reserved  : 8;
            DWORD   Width     : 4;
            DWORD   Height    : 4;
        };
        struct{
            DWORD value;
        };
    } DW0;

    //DWORD 1
    union{
        struct{
            DWORD   Row1      : 16;
            DWORD   Row2      : 16;
        };
        struct{
            DWORD value;
        };
    } DW1;

    //DWORD 2
    union{
        struct{
            DWORD   Row3      : 16;
            DWORD   Row4      : 16;
        };
        struct{
            DWORD value;
        };
    } DW2;

    //DWORD 3
    union{
        struct{
            DWORD   Row5      : 16;
            DWORD   Row6      : 16;
        };
        struct{
            DWORD value;
        };
    } DW3;

    //DWORD 4
    union{
        struct{
            DWORD   Row7      : 16;
            DWORD   Row8      : 16;
        };
        struct{
            DWORD value;
        };
    } DW4;

    //DWORD 5
    union{
        struct{
            DWORD   Row9      : 16;
            DWORD   Row10      : 16;
        };
        struct{
            DWORD value;
        };
    } DW5;

    //DWORD 6
    union{
        struct{
            DWORD   Row11      : 16;
            DWORD   Row12      : 16;
        };
        struct{
            DWORD value;
        };
    } DW6;

    //DWORD 7
    union{
        struct{
            DWORD   Row13      : 16;
            DWORD   Row14      : 16;
        };
        struct{
            DWORD value;
        };
    } DW7;
} CM_MISC_STATE;
 
typedef struct _CM_MISC_STATE_MSG{
    //DWORD 0
    union{
        struct{
            DWORD   Row0      : 16;
            DWORD   Reserved  : 8;
            DWORD   Width     : 4;
            DWORD   Height    : 4;
        };
        struct{
            DWORD value;
        };
    }DW0;

    //DWORD 1
    union{
        struct{
            DWORD   Row1      : 16;
            DWORD   Row2      : 16;
        };
        struct{
            DWORD value;
        };
    }DW1;

    //DWORD 2
    union{
        struct{
            DWORD   Row3      : 16;
            DWORD   Row4      : 16;
        };
        struct{
            DWORD value;
        };
    }DW2;

    //DWORD 3
    union{
        struct{
            DWORD   Row5      : 16;
            DWORD   Row6      : 16;
        };
        struct{
            DWORD value;
        };
    }DW3;

    //DWORD 4
    union{
        struct{
            DWORD   Row7      : 16;
            DWORD   Row8      : 16;
        };
        struct{
            DWORD value;
        };
    }DW4;

    //DWORD 5
    union{
        struct{
            DWORD   Row9      : 16;
            DWORD   Row10      : 16;
        };
        struct{
            DWORD value;
        };
    }DW5;

    //DWORD 6
    union{
        struct{
            DWORD   Row11      : 16;
            DWORD   Row12      : 16;
        };
        struct{
            DWORD value;
        };
    }DW6;

    //DWORD 7
    union{
        struct{
            DWORD   Row13      : 16;
            DWORD   Row14      : 16;
        };
        struct{
            DWORD value;
        };
    }DW7;
} CM_MISC_STATE_MSG;

enum CM_SAMPLER_STATE_TYPE {
  CM_SAMPLER8X8_AVS = 0,
  CM_SAMPLER8X8_CONV = 1,
  CM_SAMPLER8X8_MISC = 3,
  CM_SAMPLER8X8_CONV1DH = 4,
  CM_SAMPLER8X8_CONV1DV = 5,
  CM_SAMPLER8X8_AVS_EX = 6,
  CM_SAMPLER8X8_NONE
};

struct CM_SAMPLER_8X8_DESCR {
  CM_SAMPLER_STATE_TYPE stateType;
  union {
    CM_AVS_STATE_MSG *avs;
    CM_AVS_STATE_MSG_EX *avs_ex;
    CM_CONVOLVE_STATE_MSG *conv;
    CM_MISC_STATE_MSG *misc; //ERODE/DILATE/MINMAX
  };
};

typedef struct _CM_VEBOX_STATE
{

    DWORD       ColorGamutExpansionEnable : 1;
    DWORD       ColorGamutCompressionEnable : 1;
    DWORD       GlobalIECPEnable : 1;
    DWORD       DNEnable : 1;
    DWORD       DIEnable : 1;
    DWORD       DNDIFirstFrame : 1;
    DWORD       DownsampleMethod422to420 : 1;
    DWORD       DownsampleMethod444to422 : 1;
    DWORD       DIOutputFrames : 2;
    DWORD       DemosaicEnable : 1;
    DWORD       VignetteEnable : 1;
    DWORD       AlphaPlaneEnable : 1;
    DWORD       HotPixelFilteringEnable : 1;
    DWORD       SingleSliceVeboxEnable : 1;
    DWORD       LaceCorrectionEnable : BITFIELD_BIT(16);
    DWORD       DisableEncoderStatistics : BITFIELD_BIT(17);
    DWORD       DisableTemporalDenoiseFilter : BITFIELD_BIT(18);
    DWORD       SinglePipeEnable : BITFIELD_BIT(19);
    DWORD      __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(20);
    DWORD       ForwardGammaCorrectionEnable : BITFIELD_BIT(21);
    DWORD        __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 24);
    DWORD       StateSurfaceControlBits : BITFIELD_RANGE(25, 31);
}  CM_VEBOX_STATE, *PCM_VEBOX_STATE;

typedef struct _CM_POWER_OPTION
{
    USHORT nSlice;                      // set number of slice to use: 0(default number), 1, 2...
    USHORT nSubSlice;                   // set number of subslice to use: 0(default number), 1, 2... 
    USHORT nEU;                         // set number of EU to use: 0(default number), 1, 2...
} CM_POWER_OPTION, *PCM_POWER_OPTION;

// to support new flag with current API
// new flag/field could be add to the end of this structure
// 
struct CM_FLAG {
    CM_FLAG();
    CM_ROTATION rotationFlag;
    INT chromaSiting;
};

struct _CM_TASK_CONFIG {
    uint32_t turboBoostFlag;        //CM_TURBO_BOOST_DISABLE----disabled, CM_TURBO_BOOST_ENABLE--------enabled.
    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t reserved2;
};
#define CM_TASK_CONFIG _CM_TASK_CONFIG

// parameters used to set the surface state of the buffer
typedef struct _CM_SURFACE_MEM_OBJ_CTRL {
    MEMORY_OBJECT_CONTROL mem_ctrl;
    MEMORY_TYPE mem_type;
    INT age;
} CM_SURFACE_MEM_OBJ_CTRL;

typedef struct _CM_BUFFER_STATE_PARAM
{
    UINT                      uiSize;               // the new size of the buffer, if it is 0, set it as the (original width - offset)
    UINT                      uiBaseAddressOffset;  // offset should be 16-aligned
    CM_SURFACE_MEM_OBJ_CTRL   mocs;                 // if not set (all zeros), then the aliases mocs setting is the same as the origin
}CM_BUFFER_STATE_PARAM;

typedef struct _CM_SURFACE2D_STATE_PARAM
{
    UINT format;
    UINT width;
    UINT height;
    UINT depth;
    UINT pitch;
    WORD memory_object_control;
    UINT surface_x_offset;
    UINT surface_y_offset;
    UINT reserved[4]; // for future usage
} CM_SURFACE2D_STATE_PARAM;

struct CM_QUEUE_CREATE_OPTION
{
    CM_QUEUE_TYPE QueueType : 3;
    bool RunAloneMode       : 1;
    unsigned int Reserved0  : 4;
    unsigned int Reserved1  : 8;
    unsigned int Reserved2  : 16;
};

typedef enum _CM_CONDITIONAL_END_OPERATOR_CODE {
    MAD_GREATER_THAN_IDD = 0,
    MAD_GREATER_THAN_OR_EQUAL_IDD,
    MAD_LESS_THAN_IDD,
    MAD_LESS_THAN_OR_EQUAL_IDD,
    MAD_EQUAL_IDD,
    MAD_NOT_EQUAL_IDD
} CM_CONDITIONAL_END_OPERATOR_CODE;

struct CM_CONDITIONAL_END_PARAM {
    DWORD opValue;
    CM_CONDITIONAL_END_OPERATOR_CODE  opCode;
    bool  opMask;
    bool  opLevel;
};

//**********************************************************************
// Constants
//**********************************************************************
const CM_QUEUE_CREATE_OPTION CM_DEFAULT_QUEUE_CREATE_OPTION = { CM_QUEUE_TYPE_RENDER, false, 0x0, 0x0, 0x0 };

//**********************************************************************
// Classes forward declarations
//**********************************************************************
class CmSampler8x8;
class CmEvent;
class CmThreadGroupSpace;
class CmKernel;
class CmTask;
class CmProgram;
class CmBuffer;
class CmBufferUP;
class CmBufferSVM;
class CmSurface2D;
class CmSurface2DUP;
class CmSurface3D;
class CmSampler;
class CmThreadSpace;
class CmVebox;
class CmQueue;
class SurfaceIndex;
class SamplerIndex;

//**********************************************************************
// Extended definitions if any
//**********************************************************************
#include "cm_rt_extension.h"

//**********************************************************************
// Classes
//**********************************************************************
class CmSampler8x8
{
public:
    CM_RT_API virtual INT GetIndex( SamplerIndex* & pIndex ) = 0 ;
protected:
    ~CmSampler8x8(){};
};

class CmEvent
{
public:
    CM_RT_API virtual INT GetStatus( CM_STATUS& status) = 0 ;
    CM_RT_API virtual INT GetExecutionTime(UINT64& time) = 0;
    CM_RT_API virtual INT WaitForTaskFinished(DWORD dwTimeOutMs = CM_MAX_TIMEOUT_MS) = 0 ;
    CM_RT_API virtual INT GetSurfaceDetails( UINT kernIndex, UINT surfBTI,CM_SURFACE_DETAILS& outDetails )=0;
    CM_RT_API virtual INT GetProfilingInfo(CM_EVENT_PROFILING_INFO infoType, size_t paramSize, PVOID pInputValue, PVOID pValue) = 0;
    CM_RT_API virtual INT GetExecutionTickTime(UINT64& ticks) = 0;
protected:
   ~CmEvent(){};
};

class CmKernel
{
public:       
    CM_RT_API virtual INT SetThreadCount(UINT count ) = 0;
    CM_RT_API virtual INT SetKernelArg(UINT index, size_t size, const void * pValue ) = 0;
    CM_RT_API virtual INT SetThreadArg(UINT threadId, UINT index, size_t size, const void * pValue ) = 0;
    CM_RT_API virtual INT SetStaticBuffer(UINT index, const void * pValue ) = 0;
    CM_RT_API virtual INT SetSurfaceBTI(SurfaceIndex* pSurface, UINT BTIndex) = 0;
    CM_RT_API virtual INT AssociateThreadSpace(CmThreadSpace* & pTS) = 0;
    CM_RT_API virtual INT AssociateThreadGroupSpace(CmThreadGroupSpace* & pTGS) = 0; 
    CM_RT_API virtual INT SetSamplerBTI(SamplerIndex* pSampler, UINT nIndex) = 0;
    CM_RT_API virtual INT DeAssociateThreadSpace(CmThreadSpace* & pTS) = 0;
    CM_RT_API virtual INT DeAssociateThreadGroupSpace(CmThreadGroupSpace* & pTGS) = 0;
    CM_RT_API virtual INT QuerySpillSize(unsigned int &spillSize) = 0;
    CM_RT_API virtual INT GetIndexForCurbeData( UINT curbe_data_size, SurfaceIndex *pSurface ) = 0;
protected:
   ~CmKernel(){};
};

class CmTask
{
public:       
    CM_RT_API virtual INT AddKernel(CmKernel *pKernel) = 0;
    CM_RT_API virtual INT Reset(void) = 0;
    CM_RT_API virtual INT AddSync(void) = 0;
    CM_RT_API virtual INT SetPowerOption( PCM_POWER_OPTION pCmPowerOption ) = 0;
    CM_RT_API virtual INT AddConditionalEnd(SurfaceIndex* pSurface, UINT offset, CM_CONDITIONAL_END_PARAM *pCondParam) = 0;
    CM_RT_API virtual INT SetProperty(const CM_TASK_CONFIG &taskConfig) = 0; 
protected:
   ~CmTask(){};
}; 

class CmBuffer
{
public:
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 
    CM_RT_API virtual INT ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT InitSurface(const DWORD initValue, CmEvent* pEvent) = 0;
    CM_RT_API virtual INT SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL option) = 0;
    CM_RT_API virtual INT SetSurfaceStateParam(SurfaceIndex *pSurfIndex, const CM_BUFFER_STATE_PARAM *pSSParam) = 0;
protected:
   ~CmBuffer(){};
};

class CmBufferUP
{
public:
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 
    CM_RT_API virtual INT SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL option) = 0;
protected:
   ~CmBufferUP(){};
};

class CmBufferSVM
{
public:
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 
    CM_RT_API virtual INT GetAddress( void * &pAddr) = 0;
protected:
    ~CmBufferSVM(){};
};

class CmSurface2DUP
{
public:    
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 
    CM_RT_API virtual INT SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL option) = 0;
    CM_RT_API virtual INT SetProperty(CM_FRAME_TYPE frameType) = 0; 
protected:
    ~CmSurface2DUP(){};
};

class CmSurface3D
{
public:    
    CM_RT_API virtual INT GetIndex( SurfaceIndex*& pIndex ) = 0; 
    CM_RT_API virtual INT ReadSurface( unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT WriteSurface( const unsigned char* pSysMem, CmEvent* pEvent, UINT64 sysMemSize = 0xFFFFFFFFFFFFFFFFULL ) = 0;
    CM_RT_API virtual INT InitSurface(const DWORD initValue, CmEvent* pEvent) = 0;
    CM_RT_API virtual INT SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL option) = 0;
protected:
   ~CmSurface3D(){};
};

class CmSampler
{
public:
    CM_RT_API virtual INT GetIndex( SamplerIndex* & pIndex ) = 0 ;
protected:
    ~CmSampler(){};
};

class CmThreadSpace
{
public:
    CM_RT_API virtual INT AssociateThread( UINT x, UINT y, CmKernel* pKernel , UINT threadId ) = 0;
    CM_RT_API virtual INT SelectThreadDependencyPattern ( CM_DEPENDENCY_PATTERN pattern ) = 0;
    CM_RT_API virtual INT AssociateThreadWithMask( UINT x, UINT y, CmKernel* pKernel , UINT threadId, BYTE nDependencyMask ) = 0;
    CM_RT_API virtual INT SetThreadSpaceColorCount( UINT colorCount ) = 0;
    CM_RT_API virtual INT SelectMediaWalkingPattern( CM_WALKING_PATTERN pattern ) = 0;
    CM_RT_API virtual INT Set26ZIDispatchPattern( CM_26ZI_DISPATCH_PATTERN pattern ) = 0;
    CM_RT_API virtual INT Set26ZIMacroBlockSize( UINT width, UINT height )  = 0;
    CM_RT_API virtual INT SetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT groupSelect) = 0;
    CM_RT_API virtual INT SelectMediaWalkingParameters( CM_WALKING_PARAMETERS paramaters ) = 0;
    CM_RT_API virtual INT SelectThreadDependencyVectors( CM_DEPENDENCY dependVectors ) = 0;
    CM_RT_API virtual INT SetThreadSpaceOrder(UINT threadCount, PCM_THREAD_PARAM pThreadSpaceOrder) = 0;
protected:
    ~CmThreadSpace(){};
};

class CmVebox
{
public:
    CM_RT_API virtual INT SetState(CM_VEBOX_STATE& VeBoxState) = 0;

    CM_RT_API virtual INT SetCurFrameInputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetCurFrameInputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetPrevFrameInputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetPrevFrameInputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetSTMMInputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetSTMMInputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetSTMMOutputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetSTMMOutputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetDenoisedCurFrameOutputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetDenoisedCurOutputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetCurFrameOutputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetCurFrameOutputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetPrevFrameOutputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetPrevFrameOutputSurfaceControlBits( const WORD ctrlBits ) = 0;

    CM_RT_API virtual INT SetStatisticsOutputSurface( CmSurface2D* pSurf ) = 0;
    CM_RT_API virtual INT SetStatisticsOutputSurfaceControlBits( const WORD ctrlBits ) = 0;
    CM_RT_API virtual INT SetParam(CmBufferUP *pParamBuffer) = 0;
protected:
   ~CmVebox(){};
};

class CmQueue
{
public:    
    CM_RT_API virtual INT Enqueue( CmTask* pTask, CmEvent* & pEvent, const CmThreadSpace* pTS = nullptr ) = 0;
    CM_RT_API virtual INT DestroyEvent( CmEvent* & pEvent ) = 0; 
    CM_RT_API virtual INT EnqueueWithGroup( CmTask* pTask, CmEvent* & pEvent, const CmThreadGroupSpace* pTGS = nullptr )=0;
    CM_RT_API virtual INT EnqueueCopyCPUToGPU( CmSurface2D* pSurface, const unsigned char* pSysMem, CmEvent* & pEvent ) = 0; 
    CM_RT_API virtual INT EnqueueCopyGPUToCPU( CmSurface2D* pSurface, unsigned char* pSysMem, CmEvent* & pEvent ) = 0;
    CM_RT_API virtual INT EnqueueInitSurface2D( CmSurface2D* pSurface, const DWORD initValue, CmEvent* &pEvent ) = 0;
    CM_RT_API virtual INT EnqueueCopyGPUToGPU( CmSurface2D* pOutputSurface, CmSurface2D* pInputSurface, UINT option, CmEvent* & pEvent ) = 0;
    CM_RT_API virtual INT EnqueueCopyCPUToCPU( unsigned char* pDstSysMem, unsigned char* pSrcSysMem, UINT size, UINT option, CmEvent* & pEvent ) = 0;

    CM_RT_API virtual INT EnqueueCopyCPUToGPUFullStride( CmSurface2D* pSurface, const unsigned char* pSysMem, const UINT widthStride, const UINT heightStride, const UINT option, CmEvent* & pEvent ) = 0;
    CM_RT_API virtual INT EnqueueCopyGPUToCPUFullStride( CmSurface2D* pSurface, unsigned char* pSysMem, const UINT widthStride, const UINT heightStride, const UINT option, CmEvent* & pEvent ) = 0;

    CM_RT_API virtual INT EnqueueCopyCPUToGPUFullStrideDup( CmSurface2D* pSurface, const unsigned char* pSysMem, const UINT widthStride, const UINT heightStride, const UINT option, CmEvent* & pEvent ) = 0;
    CM_RT_API virtual INT EnqueueCopyGPUToCPUFullStrideDup( CmSurface2D* pSurface, unsigned char* pSysMem, const UINT widthStride, const UINT heightStride, const UINT option, CmEvent* & pEvent ) = 0;
    
    CM_RT_API virtual INT EnqueueWithHints( CmTask* pTask, CmEvent* & pEvent, UINT hints = 0) = 0;
    CM_RT_API virtual INT EnqueueVebox( CmVebox* pVebox, CmEvent* & pEvent ) = 0;
protected:
    ~CmQueue(){};
};

//**********************************************************************
// Function pointer types
//**********************************************************************
typedef void (CM_CALLBACK *callback_function)(CmEvent*, void *);
typedef void (*IMG_WALKER_FUNTYPE)(void* img, void* arg);

//**********************************************************************
// OS-specific APIs and classes
//**********************************************************************
#include "cm_rt_api_os.h"

//**********************************************************************
// Functions declaration
//**********************************************************************
EXTERN_C CM_RT_API INT DestroyCmDevice(CmDevice* &device);
EXTERN_C CM_RT_API INT CMRT_Enqueue(CmQueue* queue, CmTask* task, CmEvent** event, const CmThreadSpace* threadSpace = nullptr);
EXTERN_C CM_RT_API const char* GetCmErrorString(int errCode);

//**********************************************************************
// Platfom specific definitions
//**********************************************************************
#include "cm_rt_g8.h"
#include "cm_rt_g9.h"
#include "cm_rt_g10.h"

#endif //__CM_RT_H__
