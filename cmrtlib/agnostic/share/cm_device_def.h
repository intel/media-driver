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
//! Declaration of types and data stuctures used by implementations of CmDevice on various operating systems.
//!

#ifndef CMRTLIB_AGNOSTIC_SHARE_CM_DEVICE_DEF_H_
#define CMRTLIB_AGNOSTIC_SHARE_CM_DEVICE_DEF_H_

#include <cstdint>
#include "cm_surface_properties.h"
#include "cm_l3_cache_config.h"
#include "cm_queue_base_hw.h"
#include "cm_def.h"

typedef enum _CM_DEVICE_CAP_NAME {
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
  CAP_MAX_BUFFER_SIZE,
  CAP_MAX_SUBDEV_COUNT //for app to retrieve the total count of sub devices
} CM_DEVICE_CAP_NAME;

// parameters used to set the surface state of the CmSurface
struct CM_VME_SURFACE_STATE_PARAM {
  uint32_t width;
  uint32_t height;
};

#define CM_DEFAULT_PRINT_BUFFER_SIZE  (1*1024*1024) // 1M print buffer size

#define CM_SAMPLER_ARRAY_SIZE 100
#define MAX_BUFFER_SIZE 256
#define CM_VME_VIDEO_TYPE_AVC  1      //default value for AVC
#define CM_VME_VIDEO_TYPE_HEVC 2

typedef enum _CM_SUB_MAJOR_STEPPING {
  A = 0,
  B = 1,
  C = 2,
  D = 3,
  E = 4,
  F = 5,
  G = 6,
  H = 7,
  I = 8,
  J = 9,
  K = 10,
  L = 11,
  M = 12,
  N = 13,
  O = 14,
  P = 15,
  Q = 16,
  R = 17,
  S = 18,
  T = 19,
  U = 20,
  V = 21,
  W = 22,
  X = 23,
  Y = 24,
  Z = 25
} CM_SUB_MAJOR_STEPPING;

typedef enum _CM_SUB_PLATFORM_USE_SKU {
  GT1 = 1,
  GT2 = 2,
  GT3 = 3,
  GT4 = 4,
  GT5 = 5,
  GTA = 5,
  GTC = 6,
  GTX = 7,
  GT1_5 = 8
} CM_SUB_PLATFORM_USE_SKU;

typedef int32_t  (__cdecl *ReleaseSurfaceCallback)(void *cmDevice, void *surface);

struct CmDeviceCreationParam
{
    uint32_t createOption;        // [in]  Dev create option
    ReleaseSurfaceCallback releaseSurfaceFunc;  // [in]  Function Pointer to free surface
    void *deviceHandleInUmd;      // [out] pointer to handle in driver
    uint32_t version;             // [out] the Cm version
    uint32_t driverStoreEnabled;  // [out] DriverStoreEnable flag
    int32_t returnValue;          // [out] the return value from CMRT@UMD
};

#define CM_MAX_SPILL_SIZE_IN_BYTE_PER_HW_THREAD 9216 // 9K

#define CM_MAX_SURFACE3D_FORMAT_COUNT 3

#define CM_RT_PLATFORM "CM_RT_PLATFORM"
#define CM_RT_SKU "CM_RT_SKU"
#define CM_RT_MAX_THREADS "CM_RT_MAX_THREADS"
#define CM_RT_AUBLOAD_OPTION "CM_RT_AUBLOAD_OPTION"
#define CM_RT_GRITS_OPTION "CM_RT_GRITS_OPTION"

#define CM_RT_MULTIPLE_FRAMES "CM_RT_MULTIPLE_FRAMES"
#define CM_RT_STEPPING "CM_RT_STEPPING"
#define GRITS_PATH "GRITS_PATH"

#define CM_DEVICE_CONFIG_FAST_PATH_OFFSET 30
#define CM_DEVICE_CONFIG_FAST_PATH_ENABLE (1 << CM_DEVICE_CONFIG_FAST_PATH_OFFSET)
// enable the fast path by default from cmrtlib
#define CM_DEVICE_CREATE_OPTION_DEFAULT   CM_DEVICE_CONFIG_FAST_PATH_ENABLE
#define IGFX_UNKNOWN_CORE 0

struct CM_PLATFORM_INFO
{
    uint32_t numSlices;
    uint32_t numSubSlices;
    uint32_t numEUsPerSubSlice;
    uint32_t numHWThreadsPerEU;
    uint32_t numMaxEUsPerPool;
};

enum CM_QUERY_TYPE
{
    CM_QUERY_VERSION,
    CM_QUERY_REG_HANDLE,
    CM_QUERY_MAX_VALUES,
    CM_QUERY_GPU,
    CM_QUERY_GT,
    CM_QUERY_MIN_RENDER_FREQ,
    CM_QUERY_MAX_RENDER_FREQ,
    CM_QUERY_STEP,
    CM_QUERY_GPU_FREQ,
    CM_QUERY_MAX_VALUES_EX,
    CM_QUERY_SURFACE2D_FORMAT_COUNT,
    CM_QUERY_SURFACE2D_FORMATS,
    CM_QUERY_PLATFORM_INFO
};

struct CM_QUERY_CAPS
{
    CM_QUERY_TYPE type;
    union
    {
        int32_t version;
        HANDLE hRegistration;
        CM_HAL_MAX_VALUES maxValues;
        CM_HAL_MAX_VALUES_EX maxValuesEx;
        uint32_t maxVmeTableSize;
        uint32_t genCore;
        uint32_t genGT;
        uint32_t minRenderFreq;
        uint32_t maxRenderFreq;
        uint32_t genStepId;
        uint32_t gpuCurrentFreq;
        uint32_t surf2DCount;
        uint32_t *surf2DFormats;
        CM_PLATFORM_INFO platformInfo;
    };
};

// Dummy param for execute
struct CM_PARAMS
{
    uint32_t placeHolder;
};

enum CM_FUNCTION_ID
{
    CM_FN_RT_ULT                       = 0x900, // (This function code is only used to run ults for CM_RT@UMD)

    CM_FN_CREATECMDEVICE                   = 0x1000,
    CM_FN_DESTROYCMDEVICE                  = 0x1001,

    CM_FN_CMDEVICE_CREATEBUFFER            = 0x1100,
    CM_FN_CMDEVICE_DESTROYBUFFER           = 0x1101,
    CM_FN_CMDEVICE_CREATEBUFFERUP          = 0x1102,
    CM_FN_CMDEVICE_DESTROYBUFFERUP         = 0x1103,
    CM_FN_CMDEVICE_CREATESURFACE2D         = 0x1104,
    CM_FN_CMDEVICE_DESTROYSURFACE2D        = 0x1105,
    CM_FN_CMDEVICE_CREATESURFACE2DUP       = 0x1106,
    CM_FN_CMDEVICE_DESTROYSURFACE2DUP      = 0x1107,
    CM_FN_CMDEVICE_GETSURFACE2DINFO        = 0x1108,
    CM_FN_CMDEVICE_CREATESURFACE3D         = 0x1109,
    CM_FN_CMDEVICE_DESTROYSURFACE3D        = 0x110A,
    CM_FN_CMDEVICE_CREATEQUEUE             = 0x110B,
    CM_FN_CMDEVICE_LOADPROGRAM             = 0x110C,
    CM_FN_CMDEVICE_DESTROYPROGRAM          = 0x110D,
    CM_FN_CMDEVICE_CREATEKERNEL            = 0x110E,
    CM_FN_CMDEVICE_DESTROYKERNEL           = 0x110F,
    CM_FN_CMDEVICE_CREATETASK              = 0x1110,
    CM_FN_CMDEVICE_DESTROYTASK             = 0x1111,
    CM_FN_CMDEVICE_GETCAPS                 = 0x1112,
    CM_FN_CMDEVICE_SETCAPS                 = 0x1113,
    CM_FN_CMDEVICE_CREATETHREADSPACE       = 0x1114,
    CM_FN_CMDEVICE_DESTROYTHREADSPACE      = 0x1115,
    CM_FN_CMDEVICE_CREATETHREADGROUPSPACE  = 0x1116,
    CM_FN_CMDEVICE_DESTROYTHREADGROUPSPACE = 0x1117,
    CM_FN_CMDEVICE_SETL3CONFIG             = 0x1118,
    CM_FN_CMDEVICE_SETSUGGESTEDL3CONFIG    = 0x1119,
    CM_FN_CMDEVICE_CREATESAMPLER           = 0x111A,
    CM_FN_CMDEVICE_DESTROYSAMPLER          = 0x111B,
    CM_FN_CMDEVICE_CREATESAMPLER8X8        = 0x111C,
    CM_FN_CMDEVICE_DESTROYSAMPLER8X8       = 0x111D,
    CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE = 0x111E,
    CM_FN_CMDEVICE_DESTROYSAMPLER8X8SURFACE= 0x111F,
    CM_FN_CMDEVICE_DESTROYVMESURFACE       = 0x1123,
    CM_FN_CMDEVICE_CREATEVMESURFACEG7_5    = 0x1124,
    CM_FN_CMDEVICE_DESTROYVMESURFACEG7_5   = 0x1125,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D  = 0x1126,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE3D  = 0x1127,
    CM_FN_CMDEVICE_DESTROYSAMPLERSURFACE   = 0x1128,
    CM_FN_CMDEVICE_ENABLE_GTPIN            = 0x112A,
    CM_FN_CMDEVICE_INIT_PRINT_BUFFER       = 0x112C,
    CM_FN_CMDEVICE_CREATEVEBOX             = 0x112D,
    CM_FN_CMDEVICE_DESTROYVEBOX            = 0x112E,
    CM_FN_CMDEVICE_CREATEBUFFERSVM          = 0x1131,
    CM_FN_CMDEVICE_DESTROYBUFFERSVM         = 0x1132,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE2DUP= 0x1133,
    CM_FN_CMDEVICE_REGISTER_GTPIN_MARKERS  = 0x1136,
    CM_FN_CMDEVICE_CLONEKERNEL             = 0x1137,
    CM_FN_CMDEVICE_CREATESURFACE2D_ALIAS   = 0x1138,
    CM_FN_CMDEVICE_CREATESAMPLER_EX        = 0x1139,
    CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE_EX = 0x113A,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D_EX = 0x113B,
    CM_FN_CMDEVICE_CREATESURFACE2D_EX      = 0x113C,
    CM_FN_CMDEVICE_CREATEBUFFER_ALIAS      = 0x113D,
    CM_FN_CMDEVICE_CONFIGVMESURFACEDIMENSION = 0x113E,
    CM_FN_CMDEVICE_CREATEHEVCVMESURFACEG10 = 0x113F,
    CM_FN_CMDEVICE_GETVISAVERSION          = 0x1140,
    CM_FN_CMDEVICE_CREATEQUEUEEX           = 0x1141,
    CM_FN_CMDEVICE_FLUSH_PRINT_BUFFER      = 0x1142,
    CM_FN_CMDEVICE_DESTROYBUFFERSTATELESS  = 0x1143,

    CM_FN_CMQUEUE_ENQUEUE                  = 0x1500,
    CM_FN_CMQUEUE_DESTROYEVENT             = 0x1501,
    CM_FN_CMQUEUE_ENQUEUECOPY              = 0x1502,
    CM_FN_CMQUEUE_ENQUEUEWITHGROUP         = 0x1504,
    CM_FN_CMQUEUE_ENQUEUESURF2DINIT        = 0x1505,
    CM_FN_CMQUEUE_ENQUEUECOPY_V2V          = 0x1506,
    CM_FN_CMQUEUE_ENQUEUECOPY_L2L          = 0x1507,
    CM_FN_CMQUEUE_ENQUEUEVEBOX             = 0x1508,
    CM_FN_CMQUEUE_ENQUEUEWITHHINTS         = 0x1509,
    CM_FN_CMQUEUE_ENQUEUEFAST              = 0x150a,
    CM_FN_CMQUEUE_DESTROYEVENTFAST         = 0x150b,
    CM_FN_CMQUEUE_ENQUEUEWITHGROUPFAST     = 0x150c,
    CM_FN_CMQUEUE_ENQUEUECOPY_BUFFER       = 0x150d,

};

#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CM_DEVICE_DEF_H_
